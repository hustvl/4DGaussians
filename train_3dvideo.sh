exp_name1="main_batchsize_4_tset"
export CUDA_VISIBLE_DEVICES=3&&python train.py -s data/3Dvideo/cut_roasted_beef --port 6021 --expname "cut_roasted_beef/$exp_name1/" &
export CUDA_VISIBLE_DEVICES=2&&python train.py -s data/3Dvideo/cook_spinach --port 6022 --expname "cook_spinach/$exp_name1/" 
export CUDA_VISIBLE_DEVICES=3&&python train.py -s data/3Dvideo/sear_steak --port 6023 --expname "sear_steak/$exp_name1/" &
export CUDA_VISIBLE_DEVICES=2&&python train.py -s data/3Dvideo/coffee_martini --port 6024 --expname "coffee_martini/$exp_name1/" 
export CUDA_VISIBLE_DEVICES=3&&python train.py -s data/3Dvideo/flame_salmon_1 --port 6025 --expname "flame_salmon_1/$exp_name1/" 

export CUDA_VISIBLE_DEVICES=3&&python render.py --m "output/cut_roasted_beef/$exp_name1/"  --skip_train
export CUDA_VISIBLE_DEVICES=3&&python render.py --m "output/cook_spinach/$exp_name1/"  --skip_train
export CUDA_VISIBLE_DEVICES=3&&python render.py --m "output/sear_steak/$exp_name1/"  --skip_train
export CUDA_VISIBLE_DEVICES=3&&python render.py --m "output/coffee_martini/$exp_name1/"  --skip_train
export CUDA_VISIBLE_DEVICES=3&&python render.py --m "output/flame_salmon_1/$exp_name1"  --skip_train

export CUDA_VISIBLE_DEVICES=3&&python metrics.py --m "output/cut_roasted_beef/$exp_name1/"  
export CUDA_VISIBLE_DEVICES=3&&python metrics.py --m "output/cook_spinach/$exp_name1/"  
export CUDA_VISIBLE_DEVICES=3&&python metrics.py --m "output/sear_steak/$exp_name1/"  
export CUDA_VISIBLE_DEVICES=3&&python metrics.py --m "output/coffee_martini/$exp_name1/"  
export CUDA_VISIBLE_DEVICES=3&&python metrics.py --m "output/flame_salmon_1/$exp_name1/"  

echo "Done"