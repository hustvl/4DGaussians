exp_name=$1
export CUDA_VISIBLE_DEVICES=0&&python train.py -s data/hypernerf/virg/broom2 --port 6068 --expname "$exp_name/broom2" --configs arguments/$exp_name/broom2.py &
export CUDA_VISIBLE_DEVICES=2&&python train.py -s data/hypernerf/virg/vrig-3dprinter --port 6066 --expname "$exp_name/3dprinter" --configs arguments/$exp_name/3dprinter.py &
export CUDA_VISIBLE_DEVICES=2&&python train.py -s data/hypernerf/virg/peel-banana --port 6069 --expname "$exp_name/peel-banana" --configs arguments/$exp_name/banana.py  &
export CUDA_VISIBLE_DEVICES=0&&python train.py -s data/hypernerf/virg/vrig-chicken --port 6070 --expname "$exp_name/vrig-chicken" --configs arguments/$exp_name/chicken.py 
wait
export CUDA_VISIBLE_DEVICES=0&&python render.py --model_path output/$exp_name/broom2 --configs arguments/$exp_name/broom2.py --skip_train  --skip_test &
export CUDA_VISIBLE_DEVICES=2&&python render.py --model_path output/$exp_name/3dprinter  --configs arguments/$exp_name/3dprinter.py --skip_train  --skip_test &
export CUDA_VISIBLE_DEVICES=2&&python render.py --model_path output/$exp_name/peel-banana --configs arguments/$exp_name/banana.py --skip_train --skip_test &
export CUDA_VISIBLE_DEVICES=0&&python render.py --model_path output/$exp_name/vrig-chicken  --configs arguments/$exp_name/chicken.py --skip_train --skip_test &
wait
export CUDA_VISIBLE_DEVICES=0&&python metrics.py --model_path "output/$exp_name/broom2/"  &
export CUDA_VISIBLE_DEVICES=2&&python metrics.py --model_path "output/$exp_name/3dprinter/" &
export CUDA_VISIBLE_DEVICES=2&&python metrics.py --model_path "output/$exp_name/peel-banana/" &
export CUDA_VISIBLE_DEVICES=0&&python metrics.py --model_path "output/$exp_name/vrig-chicken/" &
wait
echo "Done"