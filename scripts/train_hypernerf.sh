exp_name1="train"
export CUDA_VISIBLE_DEVICES=0&&python train.py -s data/hypernerf/virg/3dprinter --port 6021 --expname "3dprinter/$exp_name1/" --configs arguments/hypernerf/default.py &
export CUDA_VISIBLE_DEVICES=1&&python train.py -s data/hypernerf/virg/broom2 --port 6026 --expname "broom2/$exp_name1/"     --configs arguments/hypernerf/default.py &
export CUDA_VISIBLE_DEVICES=2&&python train.py -s data/hypernerf/virg/chicken --port 6029 --expname "chicken/$exp_name1/" --configs arguments/hypernerf/default.py &
export CUDA_VISIBLE_DEVICES=3&&python train.py -s data/hypernerf/virg/peel-banana --port 6028 --expname "peel-banana/$exp_name1/" --configs arguments/hypernerf/default.py &
wait
export CUDA_VISIBLE_DEVICES=0&&python render.py --model_path "output/3dprinter/$exp_name1/"  --skip_train --configs arguments/hypernerf/default.py &
export CUDA_VISIBLE_DEVICES=1&&python render.py --model_path "output/broom2/$exp_name1/"  --skip_train  --configs arguments/hypernerf/default.py &
export CUDA_VISIBLE_DEVICES=2&&python render.py --model_path "output/chicken/$exp_name1"  --skip_train  --configs arguments/hypernerf/default.py &
export CUDA_VISIBLE_DEVICES=3&&python render.py --model_path "output/peel-banana/$exp_name1"  --skip_train  --configs arguments/hypernerf/default.py &
wait
export CUDA_VISIBLE_DEVICES=0&&python metrics.py --model_path "output/3dprinter/$exp_name1/"  &
export CUDA_VISIBLE_DEVICES=1&&python metrics.py --model_path "output/broom2/$exp_name1/"  &
export CUDA_VISIBLE_DEVICES=2&&python metrics.py --model_path "output/chicken/$exp_name1/"  &
export CUDA_VISIBLE_DEVICES=3&&python metrics.py --model_path "output/peel-banana/$exp_name1/"  &


echo "Done"
