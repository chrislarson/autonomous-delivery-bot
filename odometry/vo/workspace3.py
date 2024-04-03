from pathlib import Path
import cv2
import depthai as dai
import time
import numpy as np

# Create pipeline
pipeline = dai.Pipeline()

monoLeft = pipeline.create(dai.node.MonoCamera)
monoRight = pipeline.create(dai.node.MonoCamera)
depth = pipeline.create(dai.node.StereoDepth)
xoutL = pipeline.create(dai.node.XLinkOut)
xoutR = pipeline.create(dai.node.XLinkOut)
xout = pipeline.create(dai.node.XLinkOut)
xoutL.setStreamName("rectL")
xoutR.setStreamName("rectR")
xout.setStreamName("disparity")
camRgb = pipeline.create(dai.node.ColorCamera)
videoEnc = pipeline.create(dai.node.VideoEncoder)
xoutJpeg = pipeline.create(dai.node.XLinkOut)
xoutRgb = pipeline.create(dai.node.XLinkOut)

xoutJpeg.setStreamName("jpeg")
xoutRgb.setStreamName("rgb")

# Properties
monoLeft.setResolution(dai.MonoCameraProperties.SensorResolution.THE_720_P)
monoLeft.setBoardSocket(dai.CameraBoardSocket.LEFT)
monoRight.setResolution(dai.MonoCameraProperties.SensorResolution.THE_720_P)
monoRight.setBoardSocket(dai.CameraBoardSocket.RIGHT)
camRgb.setBoardSocket(dai.CameraBoardSocket.RGB)
camRgb.setResolution(dai.ColorCameraProperties.SensorResolution.THE_1080_P)
videoEnc.setDefaultProfilePreset(
    camRgb.getFps(), dai.VideoEncoderProperties.Profile.MJPEG
)

# Linking
monoLeft.out.link(depth.left)
monoRight.out.link(depth.right)
depth.rectifiedLeft.link(xoutL.input)
depth.rectifiedRight.link(xoutR.input)

camRgb.video.link(xoutRgb.input)
camRgb.video.link(videoEnc.input)
videoEnc.bitstream.link(xoutJpeg.input)

frameno = 0

# Connect to device and start pipeline
with dai.Device(pipeline) as device:

    qRight = device.getOutputQueue(name="rectR", maxSize=4, blocking=False)
    qLeft = device.getOutputQueue(name="rectL", maxSize=4, blocking=False)

    qRgb = device.getOutputQueue(name="rgb", maxSize=30, blocking=False)
    qJpeg = device.getOutputQueue(name="jpeg", maxSize=30, blocking=True)
    q = device.getOutputQueue(name="disparity", maxSize=4, blocking=False)

    dirNameL = "leftimage"
    Path(dirNameL).mkdir(parents=True, exist_ok=True)
    dirNameR = "rightimage"
    Path(dirNameR).mkdir(parents=True, exist_ok=True)
    dirName = "rgb_data"
    Path(dirName).mkdir(parents=True, exist_ok=True)

    dirNameL = "disparity"
    Path(dirNameL).mkdir(parents=True, exist_ok=True)

    while True:
        inRight = qRight.get()  # Blocking call, will wait until a new data has arrived
        inLeft = qLeft.get()
        inRgb = qRgb.tryGet()
        inDisparity = q.get()  # blocking call, will wait until a new data has arrived
        frame = inDisparity.getFrame()

        cv2.imshow("right", inRight.getCvFrame())
        cv2.imshow("left", inLeft.getCvFrame())
        if inRgb is not None:
            cv2.imshow("rgb", inRgb.getCvFrame())

        frame = (frame * (255 / depth.initialConfig.getMaxDisparity())).astype(np.uint8)
        cv2.imwrite(f"{dirNameL}/{frameno}.png", frame)
        cv2.imshow("disparity", inDisparity.getFrame())

        cv2.imwrite(f"{dirNameR}/{frameno}.png", inRight.getFrame())
        cv2.imwrite(f"{dirNameL}/{frameno}.png", inLeft.getFrame())
        frameno += 1
        for encFrame in qJpeg.tryGetAll():
            with open(f"{dirName}/{frameno}.jpeg", "wb") as f:
                f.write(bytearray(encFrame.getData()))

        if cv2.waitKey(1) == ord("q"):
            break
