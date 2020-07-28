const FEATURE_NVRAM_BSS_T BSS_%04d = {
	//basic setting
	.bss_ver                = 3,
	.scale_factor           = 8,
	.clip_th0               = 10,
	.clip_th1               = 20,
	.clip_th2               = 10,
	.clip_th3               = 20,
	.zero_gmv               = 12,
	.adf_th                 = 18,
	.sdf_th                 = 80,
	
	//YPF
	.ypf_en                 = 1,
	.ypf_fac                = 50,
	.ypf_adj_th             = 12,
	.ypf_dfmed0             = 20,
	.ypf_dfmed1             = 32,
	.ypf_th                 = {102,104,98,96,96,96,96,96},
	
	//FD
	.fd_en                  = 1,
	.fd_fac                 = 2,
	.fd_fnum                = 1,
	
	//EYE blinking
	.eye_en                 = 1,
	.eye_cfth               = 60,
	.eye_ratio0             = 60,
	.eye_ratio1             = 40,
	.eye_fac                = 50,
	
	// BSS 3.0
	.FaceCVTh               = 25,
	.GradThL                = 0,
	.GradThH                = 128,
	.FaceAreaThL            = {0, 0},
	.FaceAreaThH            = {16384, 32768},
	.APLDeltaTh             = {20, 20, 20, 20, 20, 30, 40, 50, 60, 70, 80, 95,110,130,150,170,190,210,230,260,290,320,320,320,320,320,320,320,320,320,320,320,320},

	.GradRatioTh            = {5000, 4000, 3000, 2500, 2000, 1000, 1000, 1000},
	.EyeDistThL             = 0,
	.EyeDistThH             = 35,
	.EyeMinWeight           = 5000,
	
    // reserved space
	.ext_setting            = 0,
    .reserved               = {0},
};