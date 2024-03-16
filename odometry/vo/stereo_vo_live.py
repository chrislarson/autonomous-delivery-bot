import os
import time
from datetime import datetime

import cv2
import math
import numpy as np
import serial

from feature_detector import FeatureDetector, DetectorType
from feature_matcher import FeatureMatcher, MatcherType, filter_matches
from utils import visualize_paths
from vo.calibration_loader import CalibrationLoader, CalibrationType

from image_loader import ImageLoader, ImageSource
from vo.stereo_matcher import StereoMatcherType, StereoMatcher, calc_depth_map


def form_transform(r, t):
    T = np.eye(4, dtype=np.float64)
    T[:3, :3] = r
    T[:3, 3] = t
    return T


L_src = 1
R_src = 0
arduino = True
MODEM = "/dev/cu.usbmodem1201"
BAUD = 9600


def main():
    images_disparity = []
    images_left = []
    images_right = []
    images_unfiltered = []
    images_filtered = []
    ser = None

    if arduino:
        ser = serial.Serial(MODEM, BAUD, timeout=1)
        ser.reset_input_buffer()
        instruction = f"<D,0.0>"
        instruction = bytes(instruction, "utf-8")
        ser.write(instruction)
        print("Serial connected.")

    # Pose
    est_poses = []
    H_tot = np.eye(4)

    calib = CalibrationLoader(CalibrationType.Local)
    k_left = calib.k_left
    t_left = calib.t_left
    t_right = calib.t_right

    image_loader = ImageLoader(ImageSource.Local, L_src, R_src)

    # Algorithm(s) Configuration
    max_depth = 3000
    detector = FeatureDetector(DetectorType.SIFT)
    matcher = FeatureMatcher(MatcherType.BruteForce, DetectorType.SIFT)
    stereo_matcher = StereoMatcher(StereoMatcherType.SGBM)

    image_loader.get_frame()
    time.sleep(0.25)
    image_loader.get_frame()
    time.sleep(0.25)
    image_loader.get_frame()
    time.sleep(0.25)

    cv_file = cv2.FileStorage()
    cv_file.open("../../calibration/stereoMap.xml", cv2.FileStorage_READ)
    stereoMapL_x = cv_file.getNode("stereoMapL_x").mat()
    stereoMapL_y = cv_file.getNode("stereoMapL_y").mat()
    stereoMapR_x = cv_file.getNode("stereoMapR_x").mat()
    stereoMapR_y = cv_file.getNode("stereoMapR_y").mat()
    start = datetime.now()

    # Algorithm
    for _ in range(18):
        image_loader.get_frame()

        image_right_prev = cv2.cvtColor(image_loader.right_prev, cv2.COLOR_BGR2GRAY)
        image_left_prev = cv2.cvtColor(image_loader.left_prev, cv2.COLOR_BGR2GRAY)
        image_left_curr = cv2.cvtColor(image_loader.left_curr, cv2.COLOR_BGR2GRAY)

        image_right_prev = cv2.remap(
            image_right_prev, stereoMapR_x, stereoMapR_y, cv2.INTER_LANCZOS4, cv2.BORDER_CONSTANT, 0
        )

        image_left_prev = cv2.remap(
            image_left_prev, stereoMapL_x, stereoMapL_y, cv2.INTER_LANCZOS4, cv2.BORDER_CONSTANT, 0
        )
        image_left_curr = cv2.remap(
            image_left_curr, stereoMapL_x, stereoMapL_y, cv2.INTER_LANCZOS4, cv2.BORDER_CONSTANT, 0
        )

        disp_map = stereo_matcher.compute_disparity(image_left_prev, image_right_prev)
        depth_map = calc_depth_map(disp_map, k_left, t_left, t_right, rectified=True)

        norm_image = cv2.normalize(
            disp_map, None, alpha=0, beta=1, norm_type=cv2.NORM_MINMAX, dtype=cv2.CV_32F
        )

        # TODO: apply mask over stereo blind spot
        kp_0, des_0 = detector.detect_and_compute(image_left_prev, None)
        kp_1, des_1 = detector.detect_and_compute(image_left_curr, None)

        matches = matcher.match_knn(des_0, des_1, k=2)
        filtered_matches = filter_matches(matches, dist_ratio=0.5)

        unfiltered_matched_img = cv2.drawMatchesKnn(
            image_left_prev,
            kp_0,
            image_left_curr,
            kp_1,
            matches,
            outImg=None,
            flags=2,
        )

        filtered_matched_img = cv2.drawMatchesKnn(
            image_left_prev,
            kp_0,
            image_left_curr,
            kp_1,
            [filtered_matches],
            outImg=None,
            flags=2,
        )

        images_disparity.append(norm_image)
        images_filtered.append(filtered_matched_img)
        images_unfiltered.append(unfiltered_matched_img)
        images_left.append(image_left_prev)
        images_right.append(image_right_prev)

        img_kp_0 = np.array([kp_0[m.queryIdx].pt for m in filtered_matches], dtype=np.float32)
        img_kp_1 = np.array([kp_1[m.trainIdx].pt for m in filtered_matches], dtype=np.float32)

        cx = k_left[0, 2]
        cy = k_left[1, 2]
        fx = k_left[0, 0]
        fy = k_left[1, 1]
        object_points = np.zeros((0, 3))
        delete = []
        for k, (u, v) in enumerate(img_kp_0):
            z = depth_map[int(v), int(u)]
            if z > max_depth:
                delete.append(k)
                continue

            # Use arithmetic to extract x and y (faster than using inverse of k)
            x = z * (u - cx) / fx
            y = z * (v - cy) / fy
            object_points = np.vstack([object_points, np.array([x, y, z])])

        img_kp_1 = np.delete(img_kp_1, delete, 0)

        # Use PnP algorithm with RANSAC for robustness to outliers
        try:
            _, rvec, tvec, inliers = cv2.solvePnPRansac(
                object_points, img_kp_1, k_left, np.array([])
            )
        except cv2.error:
            print("couldnt do features, continuing to next iteration")
            continue

        rmat = cv2.Rodrigues(rvec)[0]

        H = form_transform(rmat, tvec.T)
        H_tot = H_tot @ np.linalg.inv(H)
        est_poses.append(H_tot)

        theta = math.atan2(H_tot[0][2], H_tot[2][2])
        deg = theta * (180 / math.pi)
        instruction = f"<D,{deg}>"
        if ser is not None:
            instruction = bytes(instruction, "utf-8")
            ser.write(instruction)
        else:
            print(instruction)

    end = datetime.now()
    dur = end - start
    num_imgs = len(images_left)
    print(f"{num_imgs} frames in {dur} seconds")

    image_loader.close()

    est_path = []

    instruction = f"<D,999.9>"
    if ser is not None:
        instruction = bytes(instruction, "utf-8")
        ser.write(instruction)
    else:
        print(instruction)

    for idx, est_pose in enumerate(est_poses):
        est_path.append((est_pose[0, 3], est_pose[2, 3]))

    visualize_paths(
        est_path,
        est_path,
        "Stereo Visual Odometry",
        file_out=os.path.basename(__file__) + ".html",
    )

    for i in range(len(images_left)):
        cv2.imshow("Left camera", images_left[i])
        cv2.imwrite(
            "/Users/chris/development/csci507-project/data_us/seq_hall2/L" + str(i) + ".png",
            images_left[i],
        )

        cv2.imshow("Right camera", images_right[i])
        cv2.imwrite(
            "/Users/chris/development/csci507-project/data_us/seq_hall2/R" + str(i) + ".png",
            images_right[i],
        )

        cv2.imshow("Disparity Map", images_disparity[i])
        cv2.imwrite(
            "/Users/chris/development/csci507-project/data_us/seq_hall2/D" + str(i) + ".png",
            images_disparity[i],
        )

        cv2.imshow("Unfiltered Matches", images_unfiltered[i])
        cv2.imwrite(
            "/Users/chris/development/csci507-project/data_us/seq_hall2/U" + str(i) + ".png",
            images_unfiltered[i],
        )

        cv2.imshow("Filtered Matches", images_filtered[i])
        cv2.imwrite(
            "/Users/chris/development/csci507-project/data_us/seq_hall2/F" + str(i) + ".png",
            images_filtered[i],
        )

        cv2.waitKey(1000)


if __name__ == "__main__":
    main()
    cv2.destroyAllWindows()
