import os
import torch
from anomalib.models import Patchcore
from anomalib.engine import Engine
from anomalib.data import Folder 
from anomalib.data.utils import TestSplitMode 

# for recursion error, all this does is disable progress bars
os.environ["TQDM_DISABLE"] = "1"
os.environ["TERM"] = "dumb"

# check if CUDA is available
print("CUDA availabe:", torch.cuda.is_available())

# build model
model = Patchcore(
    backbone="resnet18", # using pretrained resnet-18
    layers=["layer2", "layer3"],
    coreset_sampling_ratio=0.1
)

# compile data
datamodule = Folder(
    name="screws",
    root="../data",
    normal_dir="train",
    test_split_mode=TestSplitMode.SYNTHETIC,
    train_batch_size = 8,
    eval_batch_size=8,
    num_workers=0
)

engine = Engine()

# fit the model 
engine.fit(
    model=model,
    datamodule=datamodule
)