
import os
import numpy as np
import glob
import sys
import json
from PIL import Image
from tqdm import tqdm
import shutil
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

image_dir = os.path.join(root_dir,"rgb","2x")
images = os.listdir(image_dir)
images.sort()
camera_dir = os.path.join(root_dir,"camera")
cameras = os.listdir(camera_dir)
cameras.sort()
cams = []
for jsonfile in tqdm(cameras):
    with open (os.path.join(camera_dir,jsonfile)) as f:
        cams.append(json.load(f))
image_size = cams[0]['image_size']
image = Image.open(os.path.join(image_dir,images[0]))
size = image.size
# breakpoint()
object_images_file = open(os.path.join(colmap_dir,"images.txt"),"w")
object_cameras_file = open(os.path.join(colmap_dir,"cameras.txt"),"w")

idx=0
cnt=0
sizes=2
while len(cams)//sizes > 200:
    sizes += 1
# breakpoint()
for cam, image in zip(cams, images):
    cnt+=1

    # print(image)
    # breakpoint()
    if cnt %  sizes != 0:
        continue
    # print("begin to write")
    R = np.array(cam['orientation']).T
    # breakpoint()
    T = -np.array(cam['position'])@R 
    # T = -np.matmul(R,T)
    
    T = [str(i) for i in T]
    qevc = [str(i) for i in rotmat2qvec(R.T)]
    print(idx+1," ".join(qevc)," ".join(T),1,image,"\n",file=object_images_file)

    print(idx,"SIMPLE_PINHOLE",image_size[0]/2,image_size[1]/2,cam['focal_length']/2,cam['principal_point'][0]/2,cam['principal_point'][1]/2,file=object_cameras_file)
    idx+=1
    shutil.copy(os.path.join(image_dir,image),os.path.join(imagecolmap_dir,image))
print(idx)
# write camera infomation.
# print(1,"SIMPLE_PINHOLE",image_size[0],image_size[1],focal[0],image_sizep0/2,image_size[1]/2,file=object_cameras_file)
object_point_file = open(os.path.join(colmap_dir,"points3D.txt"),"w")

object_cameras_file.close()
object_images_file.close()
object_point_file.close()
