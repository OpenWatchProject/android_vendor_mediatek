
const ISP_NVRAM_BNR_BPC_T BNR_BPC_%04d = {
    .con     ={.bits={.BPC_EN=0, .rsv_1=0, .BPC_LUT_EN=0, .BPC_TABLE_END_MODE=1, .rsv_6=0, .BPC_AVG_MODE=0, .rsv_9=0, .BPC_DTC_MODE=0, .BPC_CS_MODE=2, .BPC_CRC_MODE=0, .BPC_EXC=0, .BPC_BLD_MODE=0, .BNR_LE_INV_CTL=0, .BNR_OSC_COUNT=5, .BNR_EDGE=15}},
    .th1     ={.bits={.BPC_TH_LWB=100, .BPC_TH_Y=302, .BPC_BLD_SLP0=5}},
    .th2     ={.bits={.BPC_TH_UPB=500, .rsv_12=0, .BPC_BLD0=0, .rsv_23=0, .BPC_BLD1=64, .rsv_31=0}},
    .th3     ={.bits={.BPC_TH_XA=160, .BPC_TH_XB=700, .BPC_TH_SLA=1, .BPC_TH_SLB=3}},
    .th4     ={.bits={.BPC_DK_TH_XA=160, .BPC_DK_TH_XB=1002, .BPC_DK_TH_SLA=9, .BPC_DK_TH_SLB=3}},
    .dtc     ={.bits={.BPC_RNG=1, .BPC_CS_RNG=3, .rsv_7=0, .BPC_CT_LV=0, .rsv_12=0, .BPC_TH_MUL=10, .rsv_20=0, .BPC_NO_LV=3, .rsv_27=0}},
    .cor     ={.bits={.BPC_DIR_MAX=25, .BPC_DIR_TH=5, .BPC_RANK_IDXR=1, .BPC_RANK_IDXG=1, .rsv_22=0, .BPC_DIR_TH2=5}},
    .th1_c   ={.bits={.BPC_C_TH_LWB=100, .BPC_C_TH_Y=302, .rsv_24=0}},
    .th2_c   ={.bits={.BPC_C_TH_UPB=500, .rsv_12=0, .BPC_RANK_IDXB=1, .BPC_BLD_LWB=200, .rsv_28=0}},
    .th3_c   ={.bits={.BPC_C_TH_XA=160, .BPC_C_TH_XB=700, .BPC_C_TH_SLA=12, .BPC_C_TH_SLB=3}},
};