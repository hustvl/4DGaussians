workdir=$1
python scripts/extractimages.py multipleview/$workdir
colmap feature_extractor --database_path ./colmap_tmp/database.db --image_path ./colmap_tmp/images  --SiftExtraction.max_image_size 4096 --SiftExtraction.max_num_features 16384 --SiftExtraction.estimate_affine_shape 1 --SiftExtraction.domain_size_pooling 1
colmap exhaustive_matcher --database_path ./colmap_tmp/database.db
mkdir ./colmap_tmp/sparse
colmap mapper --database_path ./colmap_tmp/database.db --image_path ./colmap_tmp/images --output_path ./colmap_tmp/sparse
mkdir ./data/multipleview/$workdir/sparse_
cp -r ./colmap_tmp/sparse/0/* ./data/multipleview/$workdir/sparse_

mkdir ./colmap_tmp/dense
colmap image_undistorter --image_path ./colmap_tmp/images --input_path ./colmap_tmp/sparse/0 --output_path ./colmap_tmp/dense --output_type COLMAP
colmap patch_match_stereo --workspace_path ./colmap_tmp/dense --workspace_format COLMAP --PatchMatchStereo.geom_consistency true
colmap stereo_fusion --workspace_path ./colmap_tmp/dense --workspace_format COLMAP --input_type geometric --output_path ./colmap_tmp/dense/fused.ply

python scripts/downsample_point.py ./colmap_tmp/dense/fused.ply ./data/multipleview/$workdir/points3D_multipleview.ply

git clone https://github.com/Fyusion/LLFF.git
pip install scikit-image
python LLFF/imgs2poses.py ./colmap_tmp/

cp ./colmap_tmp/poses_bounds.npy ./data/multipleview/$workdir/poses_bounds_multipleview.npy

rm -rf ./colmap_tmp
rm -rf ./LLFF



