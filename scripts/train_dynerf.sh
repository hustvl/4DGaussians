exp_name=$1
export CUDA_VISIBLE_DEVICES=0&&python train.py -s data/dynerf/cut_roasted_beef --port 6068 --expname "$exp_name/cut_roasted_beef" --configs arguments/$exp_name/default.py &
export CUDA_VISIBLE_DEVICES=1&&python train.py -s data/dynerf/cook_spinach --port 6066 --expname "$exp_name/cook_spinach" --configs arguments/$exp_name/default.py &
export CUDA_VISIBLE_DEVICES=2&&python train.py -s data/dynerf/sear_steak --port 6069 --expname "$exp_name/sear_steak" --configs arguments/$exp_name/default.py  &
wait
export CUDA_VISIBLE_DEVICES=0&&python train.py -s data/dynerf/flame_salmon_1 --port 6070 --expname "$exp_name/flame_salmon_1" --configs arguments/$exp_name/default.py &
export CUDA_VISIBLE_DEVICES=1&&python train.py -s data/dynerf/flame_steak --port 6071 --expname "$exp_name/flame_steak" --configs arguments/$exp_name/default.py &
export CUDA_VISIBLE_DEVICES=2&&python train.py -s data/dynerf/coffee_martini --port 6071 --expname "$exp_name/coffee_martini" --configs arguments/$exp_name/default.py  &
wait
export CUDA_VISIBLE_DEVICES=0&&python render.py --model_path output/$exp_name/cut_roasted_beef --configs arguments/$exp_name/default.py --skip_train &
export CUDA_VISIBLE_DEVICES=1&&python render.py --model_path output/$exp_name/cook_spinach  --configs arguments/$exp_name/default.py --skip_train  &
export CUDA_VISIBLE_DEVICES=2&&python render.py --model_path output/$exp_name/sear_steak --configs arguments/$exp_name/default.py --skip_train &
# export CUDA_VISIBLE_DEVICES=3&&python render.py --model_path output/$exp_name/hand1-dense-v2 --configs arguments/$exp_name/hand1-dense-v2.py --skip_train
wait
export CUDA_VISIBLE_DEVICES=0&&python render.py --model_path output/$exp_name/flame_salmon_1 --configs arguments/$exp_name/default.py --skip_train &
export CUDA_VISIBLE_DEVICES=1&&python render.py --model_path output/$exp_name/flame_steak --configs arguments/$exp_name/default.py --skip_train &
export CUDA_VISIBLE_DEVICES=2&&python render.py --model_path output/$exp_name/coffee_martini --configs arguments/$exp_name/default.py --skip_train &
wait
export CUDA_VISIBLE_DEVICES=0&&python metrics.py --model_path "output/$exp_name/cut_roasted_beef/"  &
export CUDA_VISIBLE_DEVICES=1&&python metrics.py --model_path "output/$exp_name/cook_spinach/" &
export CUDA_VISIBLE_DEVICES=2&&python metrics.py --model_path "output/$exp_name/sear_steak/" &
# export CUDA_VISIBLE_DEVICES=3&&python metrics.py --model_path "output/$exp_name/hand1-dense-v2/" 
wait
export CUDA_VISIBLE_DEVICES=0&&python metrics.py --model_path "output/$exp_name/flame_salmon_1/"  &
export CUDA_VISIBLE_DEVICES=1&&python metrics.py --model_path "output/$exp_name/flame_steak/" &
export CUDA_VISIBLE_DEVICES=2&&python metrics.py --model_path "output/$exp_name/coffee_martini/" &
echo "Done"