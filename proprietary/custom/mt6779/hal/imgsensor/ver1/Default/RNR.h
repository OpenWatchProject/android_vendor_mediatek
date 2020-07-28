
const ISP_NVRAM_RNR_T RNR_%04d = {
    .con1    ={.bits={.RNR_RAT=256, .RNR_EPS=1, .rsv_17=0}},
    .vc0     ={.bits={.RNR_VC0_R=0, .RNR_VC0_G=0, .RNR_VC0_B=0, .rsv_27=0}},
    .vc1     ={.bits={.RNR_VC1_R=0, .RNR_VC1_G=0, .RNR_VC1_B=0, .rsv_27=0}},
    .rng     ={.bits={.RNR_RNG1=4, .RNR_RNG2=14, .RNR_RNG3=25, .RNR_RNG4=36, .rsv_24=0}},
    .con2    ={.bits={.RNR_MED_RAT=0, .RNR_PRF_RAT=0, .RNR_RAD=3, .RNR_CTHR=43, .RNR_CTHL=36, .rsv_29=0}},
    .con3    ={.bits={.RNR_FL_MODE=0, .RNR_GD_RAT=0, .RNR_LLP_RAT=0, .RNR_INT_OFST=0, .rsv_23=0}},
    .sl      ={.bits={.RNR_SL_EN=0, .RNR_SL_Y2=128, .RNR_SL_Y1=128, .rsv_21=0}},
    .ssl_sth ={.bits={.RNR_SSL_C2=0, .RNR_STH_C2=0, .RNR_SSL_C1=0, .RNR_STH_C1=0}},
};