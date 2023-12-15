import numpy as np
import cv2
import os
import shutil
from tqdm import tqdm
def resort(frames):
    newframes = {}
    min_frameid = 10000000
    for frame in frames:
        frameid = int(frame["file_path"].split('/')[1].split('.')[0])
        # print()
        if frameid < min_frameid:min_frameid = frameid
        newframes[frameid] = frame
    return [newframes[i+min_frameid] for i in range(len(frames))]
inputpath = "data/custom/wave-ns/"
outputpath = "data/custom/wave-train/"
testskip = 10
if not os.path.exists(outputpath):
    os.makedirs(outputpath)
image_path = os.listdir(os.path.join(inputpath,"images"))
import json
with open(os.path.join(inputpath,"transforms.json"),"r") as f:
    
    meta = json.load(f)

cnt = 0
train_json = {
    "w": meta["w"],
    "h": meta["h"],
    "fl_x": meta["fl_x"],
    "fl_y": meta["fl_y"],
    "cx": meta["cx"],
    "cy": meta["cy"],

    "camera_model" : meta["camera_model"],
    "frames":[]
}
test_json = {
    "w": meta["w"],
    "h": meta["h"],
    "fl_x": meta["fl_x"],
    "fl_y": meta["fl_y"],
    "cx": meta["cx"],
    "cy": meta["cy"],
    "camera_model" : meta["camera_model"],
    "frames":[]
}
train_image_path = os.path.join(outputpath,"train")
os.makedirs(train_image_path)
test_image_path = os.path.join(outputpath,"test")
os.makedirs(test_image_path)
# meta["frames"] = resort(meta["frames"])
totallen = len(meta["frames"])
for index, frame in tqdm(enumerate(meta["frames"])):
    image_path = os.path.join(inputpath,frame["file_path"])

    frame["time"] = index/totallen
    if index % testskip == 0:
        frame["file_path"] = "test/" + frame["file_path"].split("/")[-1]
        test_json["frames"].append(frame)
        shutil.copy(image_path, test_image_path)
    else:
        frame["file_path"] = "train/" + frame["file_path"].split("/")[-1]
        train_json["frames"].append(frame)
        shutil.copy(image_path, train_image_path)
with open(os.path.join(outputpath,"transforms_train.json"),"w") as f:
    json.dump(train_json, f)
with open(os.path.join(outputpath,"transforms_test.json"),"w") as f:
    json.dump(test_json, f)
print("done")