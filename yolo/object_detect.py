import cv2
import numpy as np
from typing import List,Tuple
from cv2.dnn import DNN_BACKEND_OPENCV, DNN_BACKEND_CUDA, DNN_TARGET_CPU, DNN_TARGET_CUDA

RectType = Tuple[float, float, float, float]
# namess from coco training dataset
names = [ 'person', 'bicycle', 'car', 'motorcycle', 'airplane', 'bus', 'train', 'truck', 'boat', 'traffic light',
         'fire hydrant', 'stop sign', 'parking meter', 'bench', 'bird', 'cat', 'dog', 'horse', 'sheep', 'cow',
         'elephant', 'bear', 'zebra', 'giraffe', 'backpack', 'umbrella', 'handbag', 'tie', 'suitcase', 'frisbee',
         'skis', 'snowboard', 'sports ball', 'kite', 'baseball bat', 'baseball glove', 'skateboard', 'surfboard',
         'tennis racket', 'bottle', 'wine glass', 'cup', 'fork', 'knife', 'spoon', 'bowl', 'banana', 'apple',
         'sandwich', 'orange', 'broccoli', 'carrot', 'hot dog', 'pizza', 'donut', 'cake', 'chair', 'couch',
         'potted plant', 'bed', 'dining table', 'toilet', 'tv', 'laptop', 'mouse', 'remote', 'keyboard', 'cell phone',
         'microwave', 'oven', 'toaster', 'sink', 'refrigerator', 'book', 'clock', 'vase', 'scissors', 'teddy bear',
         'hair drier', 'toothbrush' ]

# function that sets up yolov7 model 
def setup_model(
    config_path: str, weights_path: str, use_cuda: bool
) -> Tuple[cv2.dnn_Net, List[str]]:
    """
    Initializes the given DARKNET model


    :param config_path: The DARKNET configuration path
    :param weights_path: The DARKNET weights path
    :param use_cuda:  Whether to use CUDA for YOLO execution
    :return: The model network object and the output layer names
    :rtype: Tuple[cv2.dnn_Net, List[str]]
    """

    backend = DNN_BACKEND_CUDA if use_cuda else DNN_BACKEND_OPENCV
    target = DNN_TARGET_CUDA if use_cuda else DNN_TARGET_CPU

    model = cv2.dnn.readNetFromDarknet(config_path, weights_path)

    model.setPreferableBackend(backend)
    model.setPreferableTarget(target)

    layer_name = model.getLayerNames()
    layer_name = [layer_name[i - 1] for i in model.getUnconnectedOutLayers()]

    return model, layer_name
    
# function that detects objects in a frame
def detect_frame(
    bgr_frame: np.ndarray,
    model: cv2.dnn_Net,
    layers: List[str],
    min_conf: float,
    NMS_THRESHOLD: float = 0.3,
) -> List[Tuple[int, float, RectType]]:
    """
    Detects the objects in a BGR frame using YOLO


    :param bgr_frame: Frame to detect
    :param model: The YOLO model to use (opencv Net)
    :param layers: Network output layers
    :param min_conf: Confidence threshold
    :param NMS_THRESHOLD: Non-maxima suppression threshold
    :return List of detected (classID, confidence, RectType) tuples
    """
    h, w = bgr_frame.shape[:2]

    blob = cv2.dnn.blobFromImage(bgr_frame, 1 / 255.0, (416, 416), swapRB=True, crop=False)
    model.setInput(blob)
    layer_out = model.forward(layers)

    return_list: List[Tuple[int, float, RectType]] = []

    for output in layer_out:
        for detection in output:

            scores = detection[5:]
            classID: int = np.argmax(scores)
            confidence: float = scores[classID]

            # filter for people when detecting
            if confidence <= min_conf or classID != 0:
                continue

            box = detection[0:4] * np.array([w, h, w, h])
            center_x, center_y, width, height = box.astype("int")

            x = center_x - (width / 2)
            y = center_y - (height / 2)

            return_list.append((classID, confidence, (x, y, width, height)))

    boxes = [box for _, _, box in return_list]
    confidences = [conf for _, conf, _ in return_list]

    # apply non-maxima suppression to suppress weak, overlapping
    # bounding boxes
    indices = cv2.dnn.NMSBoxes(boxes, confidences, min_conf, NMS_THRESHOLD)

    if len(indices) <= 0:
        return []

    return_list = [return_list[i] for i in indices.flatten()]
    return return_list

# if filtering detections after all objects & bbxs have been detected
def filter_detection( name, det_list):
    id = names.index(name)
    filt_list = []
    for d in det_list:
        if d[0] == id:
            filt_list.append(d)
    return filt_list

def get_bbox_corners(video_path, model, layers):
    cap = cv2.VideoCapture(video_path)

    if (cap.isOpened() == False):
        print("Error opening video")

    count = 0
    corners = []
    while cap.isOpened():
        ret, frame = cap.read()
        
        if ret == True:
            # cv2.imshow("Frame", frame)
            # cv2.waitKey(1)
            detected = detect_frame(frame, model, layers, 0.5)
            
            if len(detected) > 0:
                for d in detected:
                    # extract bounding box corners
                    pt1 = (int(d[2][0]),int(d[2][1]))
                    pt2 = ((int(d[2][0])+int(d[2][2])),(int(d[2][1])+int(d[2][3])))
                    corners.append((pt1,pt2))
                    # draw bounding box and label on image
                    cv2.rectangle(frame,pt1,pt2,(0,0,255),3 ,cv2.LINE_AA)
                    cv2.putText(frame,str(names[d[0]]),(int(d[2][0]),int(d[2][1]+25)),4,1.0,(255,0,0))
                    cv2.putText(frame,'Frame '+ str(count), (0,25), 4, 1, (0,255,0))

            cv2.imshow("Frame", frame)
            count += 1
            cv2.waitKey(1)
            
            # Q on keyboard exits
            if 0xFF == ord('q'):
                break

        else:
            break
    cap.release()
    cv2.destroyAllWindows()
    
    

def main(args=None):
    run1 = './sample_data/run_1/left.mp4'
    run2 = './sample_data/run_2/left.mp4'

    weights = './yolov7_deps/yolov7.weights'
    config = './yolov7_deps/yolov7.cfg'
    model, layers = setup_model(config, weights, False)

    get_bbox_corners(run2, model, layers)

    



if __name__ == '__main__':
    main()