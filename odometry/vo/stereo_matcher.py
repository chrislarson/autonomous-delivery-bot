from enum import Enum

import cv2
import numpy as np
from typing import Any


class StereoMatcherType(Enum):
    SGBM = 1
    BM = 2


def calc_depth_map(disp_left, k_left, t_left, t_right, rectified=True):
    # Get focal length of x-axis for left camera
    f = k_left[0][0]

    # Calculate baseline of stereo pair
    if rectified:
        b = t_right[0] - t_left[0]
    else:
        b = t_left[0] - t_right[0]

    # Avoid instability and division by zero
    disp_left[disp_left == 0.0] = 0.01
    disp_left[disp_left == -1.0] = 0.01

    depth_map = f * b / disp_left
    return depth_map


class StereoMatcher:
    # TODO: If we have CUDA, use cuda.StereoSGM
    def __init__(self, kind: StereoMatcherType):
        sad_window = 6
        num_disparities = sad_window * 16
        # num_disparities = 32
        block_size = 11
        # cv2.cuda.ste
        if kind == StereoMatcherType.SGBM:
            self._matcher = cv2.StereoSGBM.create(
                numDisparities=num_disparities,
                minDisparity=0,
                blockSize=block_size,
                # uniquenessRatio=10,
                P1=8 * 3 * sad_window**2,
                # P1=block_size * block_size * 8,
                P2=32 * 3 * sad_window**2,
                # P2=block_size * block_size * 32,
                # mode=cv2.STEREO_SGBM_MODE_SGBM_3WAY,
                mode=cv2.STEREO_SGBM_MODE_HH,
            )
        elif kind == StereoMatcherType.BM:
            self._matcher = cv2.StereoBM.create(
                # texture_threshold=10, numDisparities=num_disparities, blockSize=block_size
                # textureThreshold=10,
                numDisparities=16,
                blockSize=15,
            )

    def compute_disparity(self, img_left: Any, img_right: Any):
        disp = self._matcher.compute(img_left, img_right).astype(np.float32) / 16
        return disp

    # ret, sol = cv2.solve(coeff, z, flags=cv2.DECOMP_QR)
