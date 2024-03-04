exp_name1=$1
export CUDA_VISIBLE_DEVICES=2&&python train.py -s data/dycheck/spin --port 6084 --expname $exp_name1/spin/ --configs arguments/$exp_name1/default.py &
export CUDA_VISIBLE_DEVICES=3&&python train.py -s data/dycheck/space-out --port 6083 --expname $exp_name1/space-out/ --configs arguments/$exp_name1/default.py &
wait
export CUDA_VISIBLE_DEVICES=2&&python render.py --model_path output/$exp_name1/space-out/   --configs arguments/$exp_name1/default.py &
export CUDA_VISIBLE_DEVICES=3&&python render.py --model_path output/$exp_name1/spin/   --configs arguments/$exp_name1/default.py 
wait
export CUDA_VISIBLE_DEVICES=2&&python train.py -s data/dycheck/teddy/ --port 6081 --expname $exp_name1/teddy/ --configs arguments/$exp_name1/default.py &
export CUDA_VISIBLE_DEVICES=3&&python train.py -s data/dycheck/apple/ --port 6082 --expname $exp_name1/apple/ --configs arguments/$exp_name1/default.py 

wait
export CUDA_VISIBLE_DEVICES=2&&python render.py --model_path output/$exp_name1/teddy/  --skip_train --configs arguments/$exp_name1/default.py &
export CUDA_VISIBLE_DEVICES=3&&python render.py --model_path output/$exp_name1/apple/  --skip_train --configs arguments/$exp_name1/default.py 


wait
export CUDA_VISIBLE_DEVICES=2&&python metrics.py --model_path output/$exp_name1/apple/  &
export CUDA_VISIBLE_DEVICES=3&&python metrics.py --model_path output/$exp_name1/teddy/  &
export CUDA_VISIBLE_DEVICES=2&&python metrics.py --model_path output/$exp_name1/space-out/  &
export CUDA_VISIBLE_DEVICES=3&&python metrics.py --model_path output/$exp_name1/spin/  
echo "Done"