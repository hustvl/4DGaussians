

OptimizationParams = dict(

    coarse_iterations = 3000,
    deformation_lr_init = 0.00016,
    deformation_lr_final = 0.0000016,
    deformation_lr_delay_mult = 0.01,
    grid_lr_init = 0.0016,
    grid_lr_final = 0.000016,
    iterations = 20000,
    pruning_interval = 8000,
    percent_dense = 0.01,
    render_process=True,
    # no_do=False,
    # no_dshs=False
    
    # opacity_reset_interval=30000

)

ModelHiddenParams = dict(

    multires = [1, 2],
    defor_depth = 0,
    net_width = 64,
    plane_tv_weight = 0.0001,
    time_smoothness_weight = 0.01,
    l1_time_planes =  0.0001,
    weight_decay_iteration=0,
    bounds=1.6
)
