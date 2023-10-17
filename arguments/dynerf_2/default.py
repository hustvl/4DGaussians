ModelHiddenParams = dict(
    kplanes_config = {
     'grid_dimensions': 2,
     'input_coordinate_dim': 4,
     'output_coordinate_dim': 16,
     'resolution': [64, 64, 64, 150]
    },
    multires = [1,2,4,8],
    defor_depth = 1,
    net_width = 256,
    plane_tv_weight = 0.0002,
    time_smoothness_weight = 0.001,
    l1_time_planes =  0.001,
    no_do=False

)
OptimizationParams = dict(
    dataloader=True,
    iterations = 20_000,
    batch_size=8,    
    coarse_iterations = 3000,
    densify_until_iter = 20_000,
    opacity_reset_interval = 3000,

    opacity_threshold_coarse = 0.05,
    opacity_threshold_fine_init = 0.05,
    opacity_threshold_fine_after = 0.05,
    # pruning_interval = 2000
)