from anomalib.models import Patchcore
import torch
import cv2
from preprocess import preprocess

threshold = 10

# load in the model
device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

model = Patchcore.load_from_checkpoint(
    "models/model.ckpt",
    map_location=device
)

model.eval()


def detect_ind(image):
    # normalize image and make compatible with model
    rgb = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
    tensor = torch.from_numpy(rgb).permute(2,0,1).float() / 255.0

    # apply model's preprocessing to tensor
    tensor = model.pre_processor.transform(tensor)

    tensor = tensor.unsqueeze(0).to(device)

    with torch.no_grad():
        prediction = model.model(tensor)
    
    score = prediction.pred_score.item()

    return score

def detect_mult(images):
    scores = []

    for image in images:
        processed_image = preprocess(image)
        scores.append(detect_ind(processed_image))
    
    return max(scores) > threshold
