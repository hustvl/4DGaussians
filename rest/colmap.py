import os
import subprocess
import argparse


def run_colmap_command(cmd, verbose=True):
    """Run a COLMAP command."""
    if verbose:
        print("Running:", " ".join(cmd))
    result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if result.returncode != 0:
        print("Error:", result.stderr.decode())
        raise RuntimeError(f"COLMAP command failed: {' '.join(cmd)}")
    return result.stdout.decode()


def extract_features(image_dir, database_path):
    run_colmap_command(
        [
            "colmap",
            "feature_extractor",
            "--database_path",
            database_path,
            "--image_path",
            image_dir,
        ]
    )


def match_features(database_path):
    run_colmap_command(
        ["colmap", "exhaustive_matcher", "--database_path", database_path]
    )


def sparse_reconstruction(project_path, database_path, image_dir):
    sparse_dir = os.path.join(project_path, "sparse")
    os.makedirs(sparse_dir, exist_ok=True)

    run_colmap_command(
        [
            "colmap",
            "mapper",
            "--database_path",
            database_path,
            "--image_path",
            image_dir,
            "--output_path",
            sparse_dir,
        ]
    )
    return sparse_dir


def dense_reconstruction(project_path, image_dir):
    sparse_model_path = os.path.join(project_path, "sparse", "0")
    dense_dir = os.path.join(project_path, "dense")
    os.makedirs(dense_dir, exist_ok=True)

    run_colmap_command(
        [
            "colmap",
            "image_undistorter",
            "--image_path",
            image_dir,
            "--input_path",
            sparse_model_path,
            "--output_path",
            dense_dir,
            "--output_type",
            "COLMAP",
        ]
    )

    run_colmap_command(
        [
            "colmap",
            "patch_match_stereo",
            "--workspace_path",
            dense_dir,
            "--workspace_format",
            "COLMAP",
            "--PatchMatchStereo.geom_consistency",
            "true",
        ]
    )

    run_colmap_command(
        [
            "colmap",
            "stereo_fusion",
            "--workspace_path",
            dense_dir,
            "--workspace_format",
            "COLMAP",
            "--input_type",
            "geometric",
            "--output_path",
            os.path.join(dense_dir, "fused.ply"),
        ]
    )


def main(image_dir, project_dir):
    database_path = os.path.join(project_dir, "database.db")
    os.makedirs(project_dir, exist_ok=True)

    print(">>> Step 1: Feature extraction")
    extract_features(image_dir, database_path)

    print(">>> Step 2: Feature matching")
    match_features(database_path)

    print(">>> Step 3: Sparse reconstruction")
    sparse_reconstruction(project_dir, database_path, image_dir)

    print(">>> Step 4: Dense reconstruction")
    dense_reconstruction(project_dir, image_dir)

    print("SfM pipeline completed successfully.")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Run sparse and dense SfM using COLMAP."
    )
    parser.add_argument(
        "--images", required=True, help="Path to folder with input images"
    )
    parser.add_argument(
        "--project", required=True, help="Path to output project directory"
    )

    args = parser.parse_args()
    main(args.images, args.project)
