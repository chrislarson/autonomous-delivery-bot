import os

import cv2
import numpy as np

from feature_detector import FeatureDetector, DetectorType
from feature_matcher import FeatureMatcher, MatcherType
from utils import visualize_paths
from vo.stereo_visual_odometry import (
    load_image_sequence,
    load_gt_poses,
    load_calibration,
    form_transform,
)


def main():
    detector = FeatureDetector(DetectorType.AKAZE)
    matcher = FeatureMatcher(MatcherType.BruteForce, DetectorType.AKAZE)
    images = load_image_sequence("seq1", "left")
    gt_poses = load_gt_poses("seq1")
    K, P = load_calibration("seq1", "left")
    K_homog = np.concatenate((K, np.zeros((3, 1))), axis=1)
    poses = [gt_poses[0]]
    gt_path = []
    estimated_path = []

    images_keypoints = []
    images_descriptors = []

    # Detect features for all images
    for i, image in enumerate(images):
        keypoints, descriptors = detector.detect_and_compute(image, None)
        images_keypoints.append(keypoints)
        images_descriptors.append(descriptors)

    for i in range(1, len(images)):
        prev_keypoints = images_keypoints[i - 1]
        prev_descriptors = images_descriptors[i - 1]

        curr_keypoints = images_keypoints[i]
        curr_descriptors = images_descriptors[i]

        first_pass_matches = matcher.match_knn(prev_descriptors, curr_descriptors, k=2)
        good_matches = []
        good_points_query = []
        good_points_train = []

        # Lowe's Ratio Test
        for match in first_pass_matches:
            match1 = match[0]
            match2 = match[1]
            if match1.distance < 0.8 * match2.distance:
                good_matches.append([match1])
                good_points_query.append(prev_keypoints[match1.queryIdx].pt)
                good_points_train.append(curr_keypoints[match1.trainIdx].pt)

        good_points_query = np.array(good_points_query, dtype=np.float32)
        good_points_train = np.array(good_points_train, dtype=np.float32)

        # # noinspection PyTypeChecker
        matched_img = cv2.drawMatchesKnn(
            images[i - 1],
            prev_keypoints,
            images[i],
            curr_keypoints,
            good_matches,
            outImg=None,
            flags=2,
        )

        cv2.imshow("matched_image", matched_img)
        cv2.waitKey(100)

        # Find Essential Matrix
        E, _ = cv2.findEssentialMat(good_points_query, good_points_train, K)

        R1, R2, t = cv2.decomposeEssentialMat(E)
        t = np.squeeze(t)

        possible_solutions = [[R1, t], [R1, -t], [R2, t], [R2, -t]]

        z_sums = []
        relative_scales = []
        for solution in possible_solutions:
            R_in = solution[0]
            t_in = solution[1]
            H_in = form_transform(R_in, t_in)

            P_in = K_homog @ H_in

            prev_points_hom = cv2.triangulatePoints(
                P,
                P_in,
                good_points_query.T,
                good_points_train.T,
            )

            curr_points_hom = H_in @ prev_points_hom

            # Un-homogenize
            prev_points = prev_points_hom[:3, :] / prev_points_hom[3, :]
            curr_points = curr_points_hom[:3, :] / curr_points_hom[3, :]

            # Find the number of points there has positive z coordinate in both images
            sum_of_pos_z_prev = sum(prev_points[2, :] > 0)
            sum_of_pos_z_curr = sum(curr_points[2, :] > 0)

            # TODO: Explore relative scale methodologies
            # print(prev_points.T[:-1])
            # relative_scale = np.mean(
            #     np.linalg.norm(curr_points.T[:-1] - curr_points.T[1:])
            #     / np.linalg.norm(prev_points.T[:-1] - prev_points.T[1:])
            # )

            # print(relative_scale)
            z_sums.append(sum_of_pos_z_prev + sum_of_pos_z_curr)
            relative_scales.append(1)

        # Find the correct solution
        correct_solution_idx = np.argmax(z_sums)
        correct_solution = possible_solutions[correct_solution_idx]
        # relative_scale = relative_scales[correct_solution_idx]
        R1, t = correct_solution
        # t = t * relative_scale
        # print(relative_scale)

        prev_pose = poses[i - 1]
        transform = form_transform(R1, t)
        curr_pose = prev_pose @ np.linalg.inv(transform)

        poses.append(curr_pose)

    for i, est_pose in enumerate(poses):
        gt_pose = gt_poses[i]
        gt_path.append((gt_pose[0, 3], gt_pose[2, 3]))
        estimated_path.append((est_pose[0, 3], est_pose[2, 3]))

    visualize_paths(
        gt_path,
        estimated_path,
        "Monocular Visual Odometry",
        file_out=os.path.basename(__file__) + ".html",
    )


if __name__ == "__main__":
    main()
