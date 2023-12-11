exp_name=$1
export CUDA_VISIBLE_DEVICES=0&&python train.py -s data/hypernerf/interp/aleks-teapot --port 6568 --expname "$exp_name/interp/aleks-teapot" --configs arguments/$exp_name/default.py &
export CUDA_VISIBLE_DEVICES=2&&python train.py -s data/hypernerf/interp/slice-banana --port 6566 --expname "$exp_name/interp/slice-banana" --configs arguments/$exp_name/default.py &
export CUDA_VISIBLE_DEVICES=2&&python train.py -s data/hypernerf/interp/chickchicken --port 6569 --expname "$exp_name/interp/interp-chicken" --configs arguments/$exp_name/default.py & 

wait
export CUDA_VISIBLE_DEVICES=0&&python train.py -s data/hypernerf/interp/cut-lemon1 --port 6670 --expname $exp_name/interp/cut-lemon1 --configs arguments/$exp_name/default.py &
export CUDA_VISIBLE_DEVICES=0&&python train.py -s data/hypernerf/interp/hand1-dense-v2 --port 6671 --expname $exp_name/interp/hand1-dense-v2 --configs arguments/$exp_name/default.py &
export CUDA_VISIBLE_DEVICES=2&&python train.py -s data/hypernerf/interp/torchocolate --port 6672 --expname $exp_name/interp/torchocolate --configs arguments/$exp_name/default.py &
wait
export CUDA_VISIBLE_DEVICES=0&&python render.py --model_path output/$exp_name/interp/aleks-teapot --configs arguments/$exp_name/default.py --skip_train &
export CUDA_VISIBLE_DEVICES=2&&python render.py --model_path output/$exp_name/interp/slice-banana  --configs arguments/$exp_name/default.py --skip_train &
export CUDA_VISIBLE_DEVICES=0&&python render.py --model_path output/$exp_name/interp/interp-chicken --configs arguments/$exp_name/default.py --skip_train &
wait
export CUDA_VISIBLE_DEVICES=0&&python render.py --model_path output/$exp_name/interp/cut-lemon1  --configs arguments/$exp_name/default.py --skip_train &
export CUDA_VISIBLE_DEVICES=2&&python render.py --model_path output/$exp_name/interp/hand1-dense-v2  --configs arguments/$exp_name/default.py --skip_train&
export CUDA_VISIBLE_DEVICES=0&&python render.py --model_path output/$exp_name/interp/torchocolate --configs arguments/$exp_name/default.py --skip_train &

wait
export CUDA_VISIBLE_DEVICES=0&&python metrics.py --model_path "output/$exp_name/interp/aleks-teapot/"  &
export CUDA_VISIBLE_DEVICES=2&&python metrics.py --model_path "output/$exp_name/interp/slice-banana/" &
export CUDA_VISIBLE_DEVICES=0&&python metrics.py --model_path "output/$exp_name/interp/interp-chicken/" 
export CUDA_VISIBLE_DEVICES=0&&python metrics.py --model_path "output/$exp_name/interp/cut-lemon1/" &
export CUDA_VISIBLE_DEVICES=2&&python metrics.py --model_path "output/$exp_name/interp/hand1-dense-v2/" &
export CUDA_VISIBLE_DEVICES=0&&python metrics.py --model_path "output/$exp_name/interp/torchocolate/" 
wait
echo "Done"