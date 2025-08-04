# train.py
import argparse
import os
import torch
from PIL import Image
import numpy as np


def mock_train(scene_path, output_dir):
    print(f"Mock training on scene: {scene_path}")

    # Check for CUDA
    cuda_available = torch.cuda.is_available()
    print("CUDA available:", cuda_available)

    # Perform dummy GPU computation
    if cuda_available:
        a = torch.rand(1000, 1000).cuda()
        b = torch.mm(a, a)
        print("Performed matrix multiplication on GPU. Result sum:", b.sum().item())
    else:
        print("GPU not available, using CPU")

    # Generate mock output image
    img = Image.new("RGB", (256, 256), color=(200, 50, 50))  # red square
    os.makedirs(output_dir, exist_ok=True)
    output_path = os.path.join(output_dir, "result_mock.png")
    img.save(output_path)
    print(f"Mock output image saved to {output_path}")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-s", "--scene_path", required=True, help="Path to dataset scene"
    )
    parser.add_argument("--port", default="6017", help="Unused dummy arg")
    parser.add_argument("--expname", required=True, help="Experiment name")
    parser.add_argument("--configs", required=True, help="Config path (unused)")
    parser.add_argument("--start_checkpoint", help="Optional: checkpoint path")

    args = parser.parse_args()

    output_dir = os.path.join("output", args.expname)
    mock_train(args.scene_path, output_dir)


if __name__ == "__main__":
    main()
