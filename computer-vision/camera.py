import cv2
from pathlib import Path 
from datetime import datetime

# access data folder
DATA_DIR = Path(__file__).resolve().parent / "data"

#GStreamer pipeline
pipeline = (
    "nvarguscamerasrc "
    "exposuretimerange='5000000 5000000' "
    "gainrange='1 1' "
    "ispdigitalgainrange='1 1' ! "
    "video/x-raw(memory:NVMM), width=4056, height=3040, framerate=10/1 ! "
    "nvvidconv ! video/x-raw, format=BGRx ! "
    "videoconvert ! video/x-raw, format=BGR ! "
    "appsink drop=1 max-buffers=1 sync=false"
)
# access the nvidia camera system
# covert video to BGRx
# convert BGRx to BGR
# send frames to python as numpy arrays

cap = cv2.VideoCapture(pipeline, cv2.CAP_GSTREAMER)    
    
    

def close():
    cap.release()


def take_picture():

    if not cap.isOpened():
        raise RuntimeError("Could not open camera")
    
    
    ok, frame = cap.read()

    if not ok:
        raise RuntimeError("Could not capture image")
    
    return frame
    

def save_image(frame):
    # create unique filename
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S_%f")
    filename = DATA_DIR / f"image_{timestamp}.jpg"

    # attempt to save image
    if not cv2.imwrite(str(filename), frame):
        raise RuntimeError(f"Failed to save image: {filename}")
    
    return filename


