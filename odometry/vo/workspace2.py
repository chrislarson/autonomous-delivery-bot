import cv2
import numpy as np

from odometry.vo.stereo_matcher import StereoMatcher, StereoMatcherType
from odometry.vo.feature_matcher import FeatureMatcher, MatcherType, filter_matches
from odometry.vo.feature_detector import FeatureDetector, DetectorType


def main():
    stereo_matcher = StereoMatcher(StereoMatcherType.SGBM)
    detector = FeatureDetector(DetectorType.SIFT)
    matcher = FeatureMatcher(MatcherType.BruteForce, DetectorType.SIFT)

    left_cap = cv2.VideoCapture(
        "/workspaces/megn540-project/sample_data/run_2/left.mp4"
    )
    right_cap = cv2.VideoCapture(
        "/workspaces/megn540-project/sample_data/run_2/right.mp4"
    )

    if not left_cap.isOpened():
        print("Cannot open left camera.")

    if not right_cap.isOpened():
        print("Cannot open right camera.")

    l_frames = []
    r_frames = []

    frame_idx = 0
    while True:

        l_ret, l_frame = left_cap.read()
        r_ret, r_frame = right_cap.read()

        if not l_ret:
            print("Can't receive left frame (stream end?). Exiting ...")
            break

        if not r_ret:
            print("Can't receive right frame (stream end?). Exiting ...")
            break

        l_frames.append(l_frame)
        r_frames.append(r_frame)

        num_frames = len(l_frames)

        # Ensure we pre-load a frame
        if num_frames < 2:
            continue

        kp_0, des_0 = detector.detect_and_compute(l_frames[num_frames - 1], None)
        kp_1, des_1 = detector.detect_and_compute(l_frames[num_frames - 2], None)
        matches = matcher.match_knn(des_0, des_1, k=2)
        filtered_matches = filter_matches(matches, dist_ratio=0.4)
        disp_map = stereo_matcher.compute_disparity(l_frame, r_frame)

        # ltr = np.hstack((l_frame, r_frame))
        # cv2.imshow("LTR", ltr)

        unfiltered_matched_img = cv2.drawMatchesKnn(
            l_frames[num_frames - 2],
            kp_0,
            l_frames[num_frames - 1],
            kp_1,
            matches,
            outImg=None,
            flags=2,
        )

        filtered_matched_img = cv2.drawMatchesKnn(
            l_frames[num_frames - 2],
            kp_0,
            l_frames[num_frames - 1],
            kp_1,
            [filtered_matches],
            outImg=None,
            flags=2,
        )

        left_left_img = np.hstack((l_frames[num_frames - 2], l_frames[num_frames - 1]))

        norm_image = cv2.normalize(
            disp_map, None, alpha=0, beta=1, norm_type=cv2.NORM_MINMAX, dtype=cv2.CV_32F
        )

        cv2.imshow("Left Camera Frames, times t=i and t=i+1", left_left_img)
        cv2.imshow("Disparity Map", norm_image)
        cv2.imshow("Unfiltered Matches", unfiltered_matched_img)
        cv2.imshow("Filtered Matches", filtered_matched_img)
        cv2.waitKey(1)

        # fps = 30
        # ms_between = int(1000 / fps)

        # cv2.waitKey(1)

    left_cap.release()
    right_cap.release()


if __name__ == "__main__":
    main()
    cv2.destroyAllWindows()
