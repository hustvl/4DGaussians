
import os
import numpy as np
import glob
import sys
import json
from PIL import Image
from tqdm import tqdm
import shutil
import math
def fov2focal(fov, pixels):
    return pixels / (2 * math.tan(fov / 2))
def rotmat2qvec(R):
    Rxx, Ryx, Rzx, Rxy, Ryy, Rzy, Rxz, Ryz, Rzz = R.flat
    K = np.array([
        [Rxx - Ryy - Rzz, 0, 0, 0],
        [Ryx + Rxy, Ryy - Rxx - Rzz, 0, 0],
        [Rzx + Rxz, Rzy + Ryz, Rzz - Rxx - Ryy, 0],
        [Ryz - Rzy, Rzx - Rxz, Rxy - Ryx, Rxx + Ryy + Rzz]]) / 3.0
    eigvals, eigvecs = np.linalg.eigh(K)
    qvec = eigvecs[[3, 0, 1, 2], np.argmax(eigvals)]
    if qvec[0] < 0:
        qvec *= -1
    return qvec

root_dir = sys.argv[1]
colmap_dir = os.path.join(root_dir,"sparse_")
if not os.path.exists(colmap_dir):
    os.makedirs(colmap_dir)
imagecolmap_dir = os.path.join(root_dir,"image_colmap")
if not os.path.exists(imagecolmap_dir):
    os.makedirs(imagecolmap_dir)

image_dir = os.path.join(root_dir)
images = os.listdir(image_dir)
images.sort()
camera_json = os.path.join(root_dir,"transforms_train.json")


with open (camera_json) as f:
    meta = json.load(f)
try:
    image_size = meta['w'], meta['h']
    focal = [meta['fl_x'],meta['fl_y']]
except:
    try:
        image_size = meta['frames'][0]['w'], meta['frames'][0]['h']
        focal = [meta['frames'][0]['fl_x'],meta['frames'][0]['fl_y']]
    except:
        image_size = 800,800
        focal = fov2focal(meta['camera_angle_x'], 800)
        focal = [focal,focal]
# size = image.size
# breakpoint()
object_images_file = open(os.path.join(colmap_dir,"images.txt"),"w")
object_cameras_file = open(os.path.join(colmap_dir,"cameras.txt"),"w")

idx=0
sizes=1
cnt=0
while len(meta['frames'])//sizes > 200:
    sizes += 1
for frame in meta['frames']:
    cnt+=1
    if cnt %  sizes != 0:
        continue
    matrix = np.linalg.inv(np.array(frame["transform_matrix"]))
    R = -np.transpose(matrix[:3,:3])
    R[:,0] = -R[:,0]
    T = -matrix[:3, 3]
    T = -np.matmul(R,T)
    T = [str(i) for i in T]
    qevc = [str(i) for i in rotmat2qvec(np.transpose(R))]
    print(idx+1," ".join(qevc)," ".join(T),1,frame['file_path'].split('/')[-1]+".png","\n",file=object_images_file)

    print(idx,"SIMPLE_PINHOLE",image_size[0],image_size[1],focal[0],image_size[0]/2,image_size[1]/2,file=object_cameras_file)
    idx+=1
    # breakpoint()
    print(os.path.join(image_dir,frame['file_path']),os.path.join(imagecolmap_dir,frame['file_path'].split('/')[-1]+".png"))
    shutil.copy(os.path.join(image_dir,frame['file_path']+".png"),os.path.join(imagecolmap_dir,frame['file_path'].split('/')[-1]+".png"))
# write camera infomation.
# print(1,"SIMPLE_PINHOLE",image_size[0],image_size[1],focal[0],image_sizep0/2,image_size[1]/2,file=object_cameras_file)
object_point_file = open(os.path.join(colmap_dir,"points3D.txt"),"w")

object_cameras_file.close()
object_images_file.close()
object_point_file.close()

