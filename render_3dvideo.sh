exp_name1="main_coarse2fine_timegrid4_pointgrow"

export CUDA_VISIBLE_DEVICES=2&&python render.py --m "output/cut_roasted_beef/$exp_name1/"  --skip_train 
# export CUDA_VISIBLE_DEVICES=3&&python metrics.py --m "output/cut_roasted_beef/$exp_name1" 
# export CUDA_VISIBLE_DEVICES=3&&python render.py --m "output/$exp_name/bouncingball" 
# echo "Done"