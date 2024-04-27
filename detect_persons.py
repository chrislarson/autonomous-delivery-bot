
from pathlib import Path
import sys
from typing import List
import cv2
import depthai as dai
import numpy as np
import time
# from roboticstoolbox import mstraj
import matplotlib.pyplot as plt
from person import Person


def add_person_bounding_box(
    frame, person_detected, x1: int, x2: int, y1: int, y2: int
):
    cv2.putText(
        frame,
        str("Person"),
        (x1 + 10, y1 + 20),
        cv2.FONT_HERSHEY_TRIPLEX,
        0.5,
        255,
    )
    cv2.putText(
        frame,
        "{:.2f}".format(person_detected.confidence * 100),
        (x1 + 10, y1 + 35),
        cv2.FONT_HERSHEY_TRIPLEX,
        0.5,
        255,
    )
    cv2.putText(
        frame,
        f"X: {int(person_detected.spatialCoordinates.x)} mm",
        (x1 + 10, y1 + 50),
        cv2.FONT_HERSHEY_TRIPLEX,
        0.5,
        255,
    )
    cv2.putText(
        frame,
        f"Y: {int(person_detected.spatialCoordinates.y)} mm",
        (x1 + 10, y1 + 65),
        cv2.FONT_HERSHEY_TRIPLEX,
        0.5,
        255,
    )
    cv2.putText(
        frame,
        f"Z: {int(person_detected.spatialCoordinates.z)} mm",
        (x1 + 10, y1 + 80),
        cv2.FONT_HERSHEY_TRIPLEX,
        0.5,
        255,
    )
    cv2.rectangle(frame, (x1, y1), (x2, y2), (255, 0, 0), cv2.FONT_HERSHEY_SIMPLEX)


class DetectPersons:
    _DEPTH_STREAM_NAME = "depth"
    _DETECTIONS_STREAM_NAME = "detections"
    _RGB_STREAM_NAME = "rgb"
    _MOBILENET_PERSON_LABEL = 15
    _TARGET_LOCK_THRESHOLD = 15
    _nnBlobPath: str

    def __init__(self):
        # Get argument first
        self._nnBlobPath = str(
            (Path(__file__).parent / Path("models/mobilenet-ssd_openvino_2021.4_6shave.blob"))
            .resolve()
            .absolute()
        )
        if not Path(self._nnBlobPath).exists():
            raise FileNotFoundError(f"Required NN model file/s not found.")


    def detect(self, data_dir: str, timeout=15, show_preview=False):
        syncNN = True

        # Create pipeline.
        pipeline = dai.Pipeline()

        # Define sources and outputs.
        camRgb = pipeline.create(dai.node.ColorCamera)
        spatialDetectionNetwork = pipeline.create(dai.node.MobileNetSpatialDetectionNetwork)
        monoLeft = pipeline.create(dai.node.MonoCamera)
        monoRight = pipeline.create(dai.node.MonoCamera)
        stereo = pipeline.create(dai.node.StereoDepth)

        xoutRgb = pipeline.create(dai.node.XLinkOut)
        xoutNN = pipeline.create(dai.node.XLinkOut)
        xoutDepth = pipeline.create(dai.node.XLinkOut)

        xoutRgb.setStreamName(self._RGB_STREAM_NAME)
        xoutNN.setStreamName(self._DETECTIONS_STREAM_NAME)
        xoutDepth.setStreamName(self._DEPTH_STREAM_NAME)

        # Properties
        camRgb.setPreviewSize(300, height=300)
        camRgb.setResolution(dai.ColorCameraProperties.SensorResolution.THE_1080_P)
        camRgb.setInterleaved(False)
        camRgb.setColorOrder(dai.ColorCameraProperties.ColorOrder.BGR)

        monoLeft.setResolution(dai.MonoCameraProperties.SensorResolution.THE_480_P)
        monoLeft.setCamera("left")
        monoRight.setResolution(dai.MonoCameraProperties.SensorResolution.THE_480_P)
        monoRight.setCamera("right")

        # Set node configurations.
        stereo.setDefaultProfilePreset(dai.node.StereoDepth.PresetMode.HIGH_DENSITY)
        # Align depth map to the perspective of RGB camera, on which inference is done.
        stereo.setDepthAlign(dai.CameraBoardSocket.CAM_A)
        stereo.setSubpixel(True)
        stereo.setOutputSize(monoLeft.getResolutionWidth(), monoLeft.getResolutionHeight())

        spatialDetectionNetwork.setBlobPath(Path(self._nnBlobPath))
        spatialDetectionNetwork.setConfidenceThreshold(0.7)
        spatialDetectionNetwork.input.setBlocking(False)
        spatialDetectionNetwork.setBoundingBoxScaleFactor(0.5)
        spatialDetectionNetwork.setDepthLowerThreshold(100)
        spatialDetectionNetwork.setDepthUpperThreshold(10000)

        # Linking.
        monoLeft.out.link(stereo.left)
        monoRight.out.link(stereo.right)

        camRgb.preview.link(spatialDetectionNetwork.input)
        if syncNN:
            spatialDetectionNetwork.passthrough.link(xoutRgb.input)
        else:
            camRgb.preview.link(xoutRgb.input)

        spatialDetectionNetwork.out.link(xoutNN.input)

        stereo.depth.link(spatialDetectionNetwork.inputDepth)
        spatialDetectionNetwork.passthroughDepth.link(xoutDepth.input)


        # Connect to device and start pipeline.
        with dai.Device(pipeline) as device:

            # Output queues will be used to get the rgb frames and nn data from the outputs defined above device.
            preview_queue: dai.DataOutputQueue = device.getOutputQueue(
                name=self._RGB_STREAM_NAME, maxSize=4, blocking=False
            )
            detection_queue: dai.DataOutputQueue = device.getOutputQueue(
                name=self._DETECTIONS_STREAM_NAME, maxSize=4, blocking=False
            )
            depth_queue: dai.DataOutputQueue = device.getOutputQueue(
                name=self._DEPTH_STREAM_NAME, maxSize=4, blocking=False
            )

            startTime = time.monotonic()
            frame_counter = 0
            fps = 0
            color = (255, 255, 255)

            is_new_tracking_session = True
            tracking_in_progress = True
            missed_frames = 0
            missed_match_frames = 0
            persons_tracked: List[Person] = []

            while tracking_in_progress:
                in_preview = preview_queue.get()
                in_detection = detection_queue.get()
                in_depth = depth_queue.get()

                frame_counter += 1
                current_time = time.monotonic()
                if (current_time - startTime) > 1:
                    fps = frame_counter / (current_time - startTime)
                    frame_counter = 0
                    startTime = current_time

                frame = in_preview.getCvFrame()
                depth_frame = in_depth.getFrame()  # depth_frame values are in millimeters

                depth_downscaled = depth_frame[::4]
                if np.all(depth_downscaled == 0):
                    min_depth = (
                        0  # Set a default minimum depth value when all elements are zero
                    )
                else:
                    min_depth = np.percentile(depth_downscaled[depth_downscaled != 0], 1)
                max_depth = np.percentile(depth_downscaled, 99)

                detections = in_detection.detections

                # If the frame is available, draw bounding boxes on it and show the frame
                height = frame.shape[0]
                width = frame.shape[1]

                # Filter detections to PERSONS with CONFIDENCE > 70%.
                persons_detected = []
                for detection in detections:
                    if detection.label == self._MOBILENET_PERSON_LABEL:
                        if (
                            detection.spatialCoordinates.x
                            + detection.spatialCoordinates.y
                            + detection.spatialCoordinates.z
                        ) > 100:
                            persons_detected.append(detection)

                # Check whether we have the same number of persons as we are tracking.
                if len(persons_tracked) == len(persons_detected):
                    is_new_tracking_session = False
                    missed_frames = 0
                else:
                    missed_frames = missed_frames + 1
                    if missed_frames > 5:
                        print(
                            "Number of tracked targets != detected targets for 5 consecutive frames. Restarting tracking session."
                        )
                        is_new_tracking_session = True
                        persons_tracked = []
                        missed_frames = 0

                iter_matches_cnt = 0

                for pdi, person_detected in enumerate(persons_detected):

                    roi_data = person_detected.boundingBoxMapping
                    roi = roi_data.roi
                    top_left = roi.topLeft()
                    bottom_right = roi.bottomRight()
                    xmin = int(top_left.x)
                    ymin = int(top_left.y)
                    xmax = int(bottom_right.x)
                    ymax = int(bottom_right.y)

                    # Denormalize bounding box
                    x1 = int(person_detected.xmin * width)
                    x2 = int(person_detected.xmax * width)
                    y1 = int(person_detected.ymin * height)
                    y2 = int(person_detected.ymax * height)

                    person_coord = np.array(
                        [
                            person_detected.spatialCoordinates.x,
                            person_detected.spatialCoordinates.y,
                            person_detected.spatialCoordinates.z,
                        ]
                    )
                    person = Person(person_coord)

                    if is_new_tracking_session:
                        persons_tracked.append(person)
                    else:
                        for pti, person_tr in enumerate(persons_tracked):
                            is_match = person_tr.check_match(person_coord)
                            if is_match:
                                iter_matches_cnt = iter_matches_cnt + 1
                                person_tr.add_match(person_coord)
                                break  # break when finding match
                            else:
                                pass

                    add_person_bounding_box(frame, person_detected, x1, x2, y1, y2)

                cv2.putText(
                    frame,
                    "NN fps: {:.2f}".format(fps),
                    (2, frame.shape[0] - 4),
                    cv2.FONT_HERSHEY_TRIPLEX,
                    0.4,
                    (255, 255, 255),
                )

                cv2.imshow("preview", frame)

                if cv2.waitKey(1) == ord("q"):
                    break

                # If we're in a tracking session and we found no matches, restart.
                if iter_matches_cnt != len(persons_detected):
                    missed_match_frames = missed_match_frames + 1
                    if missed_match_frames > 4:
                        print(
                            "Number of target matches != number persons detected for 5 consecutive frames. Restarting tracking session."
                        )
                        is_new_tracking_session = True
                        persons_tracked = []
                        missed_match_frames = 0
                else:
                    missed_match_frames = 0
                    if len(persons_tracked) > 0:
                        all_locked = all(
                            p.match_count > self._TARGET_LOCK_THRESHOLD for p in persons_tracked
                        )
                        if all_locked:
                            tracking_in_progress = False
                            print("Target(s) locked. Generating waypoints.")
                            # Done tracking. Time to generate trajectory.

                iter_matches_cnt = 0

            # Done tracking. Generate trajectory.
            waypoint_coords = np.array([[0, 0], [0, 250]])
            for person_tr in persons_tracked:
                person_x = person_tr.ma_coord[0, 0]
                person_z = person_tr.ma_coord[0, 2]
                waypoint_coords = np.append(waypoint_coords, [[person_x, person_z]], axis=0)

            # waypoint_coords = np.append(waypoint_coords, [[0, 0]], axis=0)
            print("Waypoint coordinates:\n", waypoint_coords)


if __name__ == "__main__":
    detector = DetectPersons()
    detector.detect("")
