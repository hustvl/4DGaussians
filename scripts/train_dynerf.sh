exp_name=$1
export CUDA_VISIBLE_DEVICES=1&&python train.py -s data/dynerf/flame_salmon_1 --port 6468 --expname "$exp_name/flame_salmon_1" --configs arguments/$exp_name/flame_salmon_1.py &
export CUDA_VISIBLE_DEVICES=3&&python train.py -s data/dynerf/coffee_martini --port 6472 --expname "$exp_name/coffee_martini" --configs arguments/$exp_name/coffee_martini.py  &
wait
export CUDA_VISIBLE_DEVICES=2&&python train.py -s data/dynerf/cook_spinach --port 6436 --expname "$exp_name/cook_spinach" --configs arguments/$exp_name/cook_spinach.py &
# wait
export CUDA_VISIBLE_DEVICES=3&&python train.py -s data/dynerf/cut_roasted_beef --port 6470 --expname "$exp_name/cut_roasted_beef" --configs arguments/$exp_name/cut_roasted_beef.py 
wait 
export CUDA_VISIBLE_DEVICES=1&&python train.py -s data/dynerf/flame_steak      --port 6471 --expname "$exp_name/flame_steak" --configs arguments/$exp_name/flame_steak.py &
export CUDA_VISIBLE_DEVICES=2&&python train.py -s data/dynerf/sear_steak       --port 6569 --expname "$exp_name/sear_steak" --configs arguments/$exp_name/sear_steak.py  
wait

export CUDA_VISIBLE_DEVICES=2&&python render.py --model_path output/$exp_name/cut_roasted_beef --configs arguments/$exp_name/cut_roasted_beef.py --skip_train &
export CUDA_VISIBLE_DEVICES=3&&python render.py --model_path output/$exp_name/sear_steak --configs arguments/$exp_name/sear_steak.py --skip_train 
wait
export CUDA_VISIBLE_DEVICES=2&&python render.py --model_path output/$exp_name/flame_steak --configs arguments/$exp_name/flame_steak.py --skip_train &
export CUDA_VISIBLE_DEVICES=3&&python render.py --model_path output/$exp_name/flame_salmon_1 --configs arguments/$exp_name/flame_salmon_1.py --skip_train 
wait
export CUDA_VISIBLE_DEVICES=2&&python render.py --model_path output/$exp_name/cook_spinach  --configs arguments/$exp_name/cook_spinach.py --skip_train  &
export CUDA_VISIBLE_DEVICES=3&&python render.py --model_path output/$exp_name/coffee_martini --configs arguments/$exp_name/coffee_martini.py --skip_train &
wait
export CUDA_VISIBLE_DEVICES=2&&python metrics.py --model_path "output/$exp_name/cut_roasted_beef/"  &
export CUDA_VISIBLE_DEVICES=3&&python metrics.py --model_path "output/$exp_name/cook_spinach/" 
wait
export CUDA_VISIBLE_DEVICES=3&&python metrics.py --model_path "output/$exp_name/sear_steak/" &
export CUDA_VISIBLE_DEVICES=2&&python metrics.py --model_path "output/$exp_name/flame_salmon_1/"  
wait
export CUDA_VISIBLE_DEVICES=2&&python metrics.py --model_path "output/$exp_name/flame_steak/" &
export CUDA_VISIBLE_DEVICES=3&&python metrics.py --model_path "output/$exp_name/coffee_martini/" 
echo "Done"