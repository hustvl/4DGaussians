import cv2
import os
import re
def sorted_alphanumeric(data):
    """
    对给定的数据进行字母数字排序（考虑数字的数值大小）
    """
    convert = lambda text: int(text) if text.isdigit() else text.lower()
    alphanum_key = lambda key: [convert(c) for c in re.split('([0-9]+)', key)]
    return sorted(data, key=alphanum_key)
def create_video_from_images(folder_path, output_file, frame_rate=30, img_size=None):
    images = [img for img in os.listdir(folder_path) if img.endswith(".jpg") or img.endswith(".png")]
    images = sorted_alphanumeric(images)  # 使用自定义的排序函数

    # 获取第一张图片的尺寸
    frame = cv2.imread(os.path.join(folder_path, images[0]))
    height, width, layers = frame.shape

    # 如果指定了img_size，则调整尺寸
    if img_size is not None:
        width, height = img_size

    # 定义视频编码和创建VideoWriter对象
    fourcc = cv2.VideoWriter_fourcc(*'mp4v')  # 可以更改为其他编码器
    video = cv2.VideoWriter(output_file, fourcc, frame_rate, (width, height))

    for image in images:
        img = cv2.imread(os.path.join(folder_path, image))
        if img_size is not None:
            img = cv2.resize(img, img_size)
        video.write(img)

    cv2.destroyAllWindows()
    video.release()

# 使用示例
folder_path = 'output/editing_render'  # 替换为您的图片文件夹路径
output_file = 'output_video.mp4'  # 输出视频文件名
create_video_from_images(folder_path, output_file)
