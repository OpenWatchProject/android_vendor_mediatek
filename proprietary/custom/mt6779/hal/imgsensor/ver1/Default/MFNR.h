const NVRAM_CAMERA_FEATURE_MFLL_STRUCT MFNR_%04d = {
    // MFLL
    .capture_M                      = 6,
    .blend_N                        = 6,
	
	.memc_bad_mv_range              = 255,
    .memc_bad_mv_rate_th            = 12707,

	.me_noise_lv                    = 1600,
	.conf_noise_lv                  = 0,
	
    //DSDN related settings
    .dsdn_ratio                     = 16,
    .memc_dsus_mode                 = 0,
	
	//AE variation compensation
	.aevc_ae_en                      = 1,
	.aevc_lcso_en                    = 1,
	
	//Post refine
	.post_refine_en                  = 1,
	.post_me_refine_en               = 1,

    //Global drop frm    
    .post_me_refine_mv_ratio        = 128, 
	.post_me_refine_face_ratio_ThL  = 1000,
	.post_me_refine_face_ratio_ThH  = 3200, 
	.post_me_refine_full_ratio_ThL  = 1000,
	.post_me_refine_full_ratio_ThH  = 3200, 
	.post_me_refine_edge_ccl_Th     = {600, 600}, 
	.post_me_refine_edge_cclnum_Th  = 3,
    .post_me_refine_edge_FDAreaThL  = 0,
	.post_me_refine_edge_FDAreaThH  = 16384,

    //Local decrease 
    .lcl_deconf_en                  = 0,
    .lcl_deconf_noise_lv            = 0,
    .lcl_deconf_bg_bss_ratio        = 256,
    .lcl_deconf_fd_bss_ratio        = 243,
    .lcl_deconf_dltvar_en           = 0,
	
    // reserved space
	.ext_setting                    = 0,
    .reserved                       = {0},
};