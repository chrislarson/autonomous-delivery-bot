from enum import Enum
from typing import Any, Union

import numpy as np

from video_capture_threaded import VideoCaptureThreaded


class ImageSource(Enum):
    Luxonis = 1
    KITTI = 2
    Video = 3


class ImageLoader:
    frame_count = 0
    capture: VideoCaptureThreaded
    right_prev: Any
    right_curr: Any
    left_prev: Any
    left_curr: Any

    # https://pyimagesearch.com/2015/12/21/increasing-webcam-fps-with-python-and-opencv/
    def __init__(
        self, image_source: ImageSource, L_src: Union[int, str], R_src: Union[int, str]
    ):
        self.capture: VideoCaptureThreaded
        self.right_curr = np.zeros((720, 1280), dtype=np.uint8)
        self.left_curr = np.zeros((720, 1280), dtype=np.uint8)
        self.right_prev = np.zeros((720, 1280), dtype=np.uint8)
        self.left_prev = np.zeros((720, 1280), dtype=np.uint8)

        if image_source == ImageSource.Luxonis:
            self.capture = VideoCaptureThreaded(L_src, R_src, (1280, 960), (1280, 720))
            self.capture.start()
            print("Initialized Luxonis Image Loader")

        elif image_source == ImageSource.Video:
            self.capture = VideoCaptureThreaded(L_src, R_src, (640, 480), (640, 480))
            self.capture.start()
            print("Initialized Video Image Loader")

    def get_frame(self):
        left_frame, right_frame = self.capture.read()
        # left_frame = left_frame[110:830, :]
        self.left_prev = self.left_curr.copy()
        self.right_prev = self.right_curr.copy()
        self.left_curr = left_frame.copy()
        self.right_curr = right_frame.copy()
        self.frame_count = self.frame_count + 1

    def close(self):
        self.capture.stop()
