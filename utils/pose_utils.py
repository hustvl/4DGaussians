import numpy as np
from scipy.spatial.transform import Rotation as R
from scene.utils import Camera
from copy import deepcopy
def rotation_matrix_to_quaternion(rotation_matrix):
    return R.from_matrix(rotation_matrix).as_quat()

def quaternion_to_rotation_matrix(quat):
    return R.from_quat(quat).as_matrix()

def quaternion_slerp(q1, q2, t):
    # 计算两个四元数之间的点积
    dot = np.dot(q1, q2)

    # 如果点积为负，取反一个四元数以保证最短路径插值
    if dot < 0.0:
        q1 = -q1
        dot = -dot

    # 防止数值误差导致的问题
    dot = np.clip(dot, -1.0, 1.0)

    # 计算插值参数
    theta = np.arccos(dot) * t
    q3 = q2 - q1 * dot
    q3 = q3 / np.linalg.norm(q3)

    # 计算插值结果
    return np.cos(theta) * q1 + np.sin(theta) * q3

def bezier_interpolation(p1, p2, t):
    return (1 - t) * p1 + t * p2
def linear_interpolation(v1, v2, t):
    return (1 - t) * v1 + t * v2
def smooth_camera_poses(cameras, num_interpolations=5):
    smoothed_cameras = []
    smoothed_times = []
    total_poses = len(cameras) - 1 + (len(cameras) - 1) * num_interpolations
    time_increment = 10 / total_poses

    for i in range(len(cameras) - 1):
        cam1 = cameras[i]
        cam2 = cameras[i + 1]

        quat1 = rotation_matrix_to_quaternion(cam1.orientation)
        quat2 = rotation_matrix_to_quaternion(cam2.orientation)

        for j in range(num_interpolations + 1):
            t = j / (num_interpolations + 1)

            interp_orientation_quat = quaternion_slerp(quat1, quat2, t)
            interp_orientation_matrix = quaternion_to_rotation_matrix(interp_orientation_quat)

            interp_position = linear_interpolation(cam1.position, cam2.position, t)

            interp_time = i*10 / (len(cameras) - 1) + time_increment * j

            newcam = deepcopy(cam1)
            newcam.orientation = interp_orientation_matrix
            newcam.position = interp_position
            smoothed_cameras.append(newcam)
            smoothed_times.append(interp_time)
    smoothed_cameras.append(cameras[-1])
    smoothed_times.append(1.0)
    print(smoothed_times)
    return smoothed_cameras, smoothed_times

