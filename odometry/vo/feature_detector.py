from enum import Enum
from typing import Any, Union

import cv2 as cv


class DetectorType(Enum):
    SIFT = 1
    ORB = 2
    AKAZE = 3

    def is_binary(self):
        return self in (DetectorType.ORB, DetectorType.AKAZE)


class FeatureDetector:
    _detector: Union[cv.AKAZE, cv.SIFT, cv.ORB]

    # if we have CUDA ..
    def __init__(self, kind: DetectorType):
        if kind == kind.SIFT:
            self._detector = cv.SIFT.create()
        elif kind == kind.ORB:
            self._detector = cv.ORB.create(nfeatures=2000)
        elif kind == kind.AKAZE:
            self._detector = cv.AKAZE.create()

    def detect_and_compute(self, image: Any, mask: Any):
        return self._detector.detectAndCompute(image, mask)
