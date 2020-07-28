const ISP_NVRAM_FUS_T FUS_%04d = {
    .ratio1    ={.bits ={.FUS_HDR_RATIO_0=16, .rsv_10=0, .FUS_HDR_RATIO_1=16, .rsv_22=0, .FUS_MOSE_RATIO_0=7, .FUS_MOSE_RATIO_1=7}},
    .ratio2    ={.bits ={.FUS_HDR_RATIO_2=16, .rsv_10=0, .FUS_MOSE_RATIO_2=7, .FUS_ALIGN_LP_TH_0=0, .rsv_19=0, .FUS_ALIGN_LP_TH_1=0, .rsv_23=0, .FUS_ALIGN_LP_TH_2=0, .rsv_27=0, .FUS_ALIGN_LP_TH_3=0, .rsv_31=0}},
    .cfg       ={.bits ={.FUS_OSC_TH=4080, .FUS_OSC_COUNT=16, .FUS_MO_EN=1, .FUS_INPUT_MODE=1, .FUS_MO_TH=2, .FUS_OSC_BLDWD=8, .FUS_MO_BLDWD=9}},
};