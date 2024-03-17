
import os
import numpy as np
import glob
import sys
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
def normalize(v):
    """Normalize a vector."""
    return v / np.linalg.norm(v)

def average_poses(poses):
    """
    Calculate the average pose, which is then used to center all poses
    using @center_poses. Its computation is as follows:
    1. Compute the center: the average of pose centers.
    2. Compute the z axis: the normalized average z axis.
    3. Compute axis y': the average y axis.
    4. Compute x' = y' cross product z, then normalize it as the x axis.
    5. Compute the y axis: z cross product x.

    Note that at step 3, we cannot directly use y' as y axis since it's
    not necessarily orthogonal to z axis. We need to pass from x to y.
    Inputs:
        poses: (N_images, 3, 4)
    Outputs:
        pose_avg: (3, 4) the average pose
    """
    # 1. Compute the center
    center = poses[..., 3].mean(0)  # (3)

    # 2. Compute the z axis
    z = normalize(poses[..., 2].mean(0))  # (3)

    # 3. Compute axis y' (no need to normalize as it's not the final output)
    y_ = poses[..., 1].mean(0)  # (3)

    # 4. Compute the x axis
    x = normalize(np.cross(z, y_))  # (3)

    # 5. Compute the y axis (as z and x are normalized, y is already of norm 1)
    y = np.cross(x, z)  # (3)

    pose_avg = np.stack([x, y, z, center], 1)  # (3, 4)

    return pose_avg

blender2opencv = np.eye(4)
def center_poses(poses, blender2opencv):
    """
    Center the poses so that we can use NDC.
    See https://github.com/bmild/nerf/issues/34
    Inputs:
        poses: (N_images, 3, 4)
    Outputs:
        poses_centered: (N_images, 3, 4) the centered poses
        pose_avg: (3, 4) the average pose
    """
    poses = poses @ blender2opencv
    pose_avg = average_poses(poses)  # (3, 4)
    pose_avg_homo = np.eye(4)
    pose_avg_homo[
        :3
    ] = pose_avg  # convert to homogeneous coordinate for faster computation
    pose_avg_homo = pose_avg_homo
    # by simply adding 0, 0, 0, 1 as the last row
    last_row = np.tile(np.array([0, 0, 0, 1]), (len(poses), 1, 1))  # (N_images, 1, 4)
    poses_homo = np.concatenate(
        [poses, last_row], 1
    )  # (N_images, 4, 4) homogeneous coordinate

    poses_centered = np.linalg.inv(pose_avg_homo) @ poses_homo  # (N_images, 4, 4)
    #     poses_centered = poses_centered  @ blender2opencv
    poses_centered = poses_centered[:, :3]  # (N_images, 3, 4)

    return poses_centered, pose_avg_homo
root_dir = sys.argv[1]
colmap_dir = os.path.join(root_dir,"sparse_")
if not os.path.exists(colmap_dir):
    os.makedirs(colmap_dir)
poses_arr = np.load(os.path.join(root_dir, "poses_bounds.npy"))
poses = poses_arr[:, :-2].reshape([-1, 3, 5])  # (N_cams, 3, 5)
near_fars = poses_arr[:, -2:]
videos = glob.glob(os.path.join(root_dir, "cam[0-9][0-9]"))
videos = sorted(videos)
assert len(videos) == poses_arr.shape[0]
H, W, focal = poses[0, :, -1]
focal = focal/2
focal = [focal, focal]
poses = np.concatenate([poses[..., 1:2], -poses[..., :1], poses[..., 2:4]], -1)
# poses, _ = center_poses(
#     poses, blender2opencv
# )  # Re-center poses so that the average is near the center.
# near_original = near_fars.min()
# scale_factor = near_original * 0.75
# near_fars /= (
#     scale_factor  # rescale nearest plane so that it is at z = 4/3.
# )
# poses[..., 3] /= scale_factor
# Sample N_views poses for validation - NeRF-like camera trajectory.
# val_poses = directions
videos = glob.glob(os.path.join(root_dir, "cam[0-9][0-9]"))
videos = sorted(videos)
image_paths = []
for index, video_path in enumerate(videos):
    image_path = os.path.join(video_path,"images","0000.png")
    image_paths.append(image_path)
print(image_paths)
goal_dir = os.path.join(root_dir,"image_colmap")
if not os.path.exists(goal_dir):
    os.makedirs(goal_dir)
import shutil
image_name_list =[]
for index, image in enumerate(image_paths):
    image_name = image.split("/")[-1].split('.')
    image_name[0] = "r_%03d" % index
    print(image_name)
    # breakpoint()
    image_name = ".".join(image_name)
    image_name_list.append(image_name)
    goal_path = os.path.join(goal_dir,image_name)
    shutil.copy(image,goal_path)

print(poses)
# breakpoint()

# write image information.
object_images_file = open(os.path.join(colmap_dir,"images.txt"),"w")
for idx, pose in enumerate(poses):
    # pose_44 = np.eye(4)

    R = pose[:3,:3]
    R = -R
    R[:,0] = -R[:,0]
    T = pose[:3,3]
    
    R = np.linalg.inv(R)
    T = -np.matmul(R,T)
    T = [str(i) for i in T]
    # T = ["%.3f"%i for i in pose[:3,3]]
    qevc = [str(i) for i in rotmat2qvec(R)]
    # breakpoint()
    print(idx+1," ".join(qevc)," ".join(T),1,image_name_list[idx],"\n",file=object_images_file)
# breakpoint()

# write camera infomation.
object_cameras_file = open(os.path.join(colmap_dir,"cameras.txt"),"w")
print(1,"SIMPLE_PINHOLE",1352,1014,focal[0],1352/2,1014/2,file=object_cameras_file)
object_point_file = open(os.path.join(colmap_dir,"points3D.txt"),"w")

object_cameras_file.close()
object_images_file.close()
object_point_file.close()
