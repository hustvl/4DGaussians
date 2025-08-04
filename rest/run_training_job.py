# run_training_job.py
import argparse
import subprocess
import os


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--scene_path", required=True, help="Path to the dataset scene")
    parser.add_argument("--config", required=True, help="Path to config file")
    parser.add_argument("--expname", required=True, help="Experiment name")
    parser.add_argument("--output_dir", default="output", help="Where to store outputs")
    parser.add_argument("--checkpoint", default=None, help="Optional: checkpoint path")
    args = parser.parse_args()

    os.makedirs(args.output_dir, exist_ok=True)

    cmd = [
        "python",
        "rest/train_test.py",
        "-s",
        args.scene_path,
        "--port",
        "6017",
        "--expname",
        args.expname,
        "--configs",
        args.config,
    ]
    if args.checkpoint:
        cmd += ["--start_checkpoint", args.checkpoint]

    print(f"Running command: {' '.join(cmd)}")
    subprocess.run(cmd, check=True)


if __name__ == "__main__":
    main()
