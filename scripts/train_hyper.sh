exp_name=$1
export CUDA_VISIBLE_DEVICES=0&&python train.py -s data/hypernerf/misc/split-cookie --port 6068 --expname "$exp_name/split-cookie" --configs arguments/$exp_name/default.py &
export CUDA_VISIBLE_DEVICES=1&&python train.py -s data/hypernerf/virg/vrig-3dprinter --port 6066 --expname "$exp_name/3dprinter" --configs arguments/$exp_name/default.py &
export CUDA_VISIBLE_DEVICES=2&&python train.py -s data/hypernerf/interp/chickchicken --port 6069 --expname "$exp_name/interp-chicken" --configs arguments/$exp_name/default.py  &
export CUDA_VISIBLE_DEVICES=3&&python train.py -s data/hypernerf/interp/cut-lemon1 --port 6070 --expname "$exp_name/cut-lemon1" --configs arguments/$exp_name/cut-lemon1.py &
# export CUDA_VISIBLE_DEVICES=3&&python train.py -s data/hypernerf/interp/hand1-dense-v2 --port 6071 --expname "$exp_name/hand1-dense-v2" --configs arguments/$exp_name/hand1-dense-v2.py 
wait
export CUDA_VISIBLE_DEVICES=2&&python render.py --model_path output/$exp_name/split-cookie --configs arguments/$exp_name/default.py --skip_train --skip_test &
export CUDA_VISIBLE_DEVICES=3&&python render.py --model_path output/$exp_name/3dprinter  --configs arguments/$exp_name/default.py --skip_train --skip_test &
export CUDA_VISIBLE_DEVICES=2&&python render.py --model_path output/$exp_name/interp-chicken --configs arguments/$exp_name/default.py --skip_train --skip_test&
export CUDA_VISIBLE_DEVICES=3&&python render.py --model_path output/$exp_name/cut-lemon1  --configs arguments/$exp_name/cut-lemon1.py --skip_train --skip_test&
# export CUDA_VISIBLE_DEVICES=3&&python render.py --model_path output/$exp_name/hand1-dense-v2 --configs arguments/$exp_name/hand1-dense-v2.py --skip_train
wait
export CUDA_VISIBLE_DEVICES=0&&python metrics.py --model_path "output/$exp_name/split-cookie/"  &
export CUDA_VISIBLE_DEVICES=1&&python metrics.py --model_path "output/$exp_name/3dprinter/" &
export CUDA_VISIBLE_DEVICES=2&&python metrics.py --model_path "output/$exp_name/interp-chicken/" &
export CUDA_VISIBLE_DEVICES=3&&python metrics.py --model_path "output/$exp_name/cut-lemon1/" &
# export CUDA_VISIBLE_DEVICES=3&&python metrics.py --model_path "output/$exp_name/hand1-dense-v2/" 
wait
echo "Done"