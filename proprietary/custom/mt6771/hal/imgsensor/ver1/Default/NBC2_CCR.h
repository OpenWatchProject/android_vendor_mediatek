
const ISP_NVRAM_CCR_T NBC2_CCR_%04d = {
    .con     ={.bits={.NBC2_CCR_EN=0, .NBC2_CCR_SL2_LINK=1, .NBC2_CCR_SL2_MODE=0, .rsv_4=0, .NBC2_CCR_OR_MODE=0, .rsv_6=0, .NBC2_CCR_UV_GAIN_MODE=1, .rsv_9=0, .NBC2_CCR_UV_GAIN2=0, .rsv_23=0, .NBC2_CCR_Y_CPX3=255}},
    .ylut    ={.bits={.NBC2_CCR_Y_CPX1=32, .NBC2_CCR_Y_CPX2=96, .NBC2_CCR_Y_SP1=24, .rsv_23=0, .NBC2_CCR_Y_CPY1=16, .rsv_31=0}},
    .uvlut   ={.bits={.NBC2_CCR_UV_X1=5, .NBC2_CCR_UV_X2=37, .NBC2_CCR_UV_X3=255, .NBC2_CCR_UV_GAIN1=64, .rsv_31=0}},
    .ylut2   ={.bits={.NBC2_CCR_Y_SP0=16, .rsv_7=0, .NBC2_CCR_Y_SP2=0, .rsv_15=0, .NBC2_CCR_Y_CPY0=0, .rsv_23=0, .NBC2_CCR_Y_CPY2=64, .rsv_31=0}},
    .sat_ctrl={.bits={.NBC2_CCR_MODE=1, .rsv_1=0, .NBC2_CCR_CEN_U=0, .rsv_10=0, .NBC2_CCR_CEN_V=0, .rsv_18=0}},
    .uvlut_sp={.bits={.NBC2_CCR_UV_GAIN_SP1=64, .rsv_10=0, .NBC2_CCR_UV_GAIN_SP2=0, .rsv_26=0}},
    .hue1    ={.bits={.NBC2_CCR_HUE_X1=0, .rsv_9=0, .NBC2_CCR_HUE_X2=16, .rsv_25=0}},
    .hue2    ={.bits={.NBC2_CCR_HUE_X3=270, .rsv_9=0, .NBC2_CCR_HUE_X4=286, .rsv_25=0}},
    .hue3    ={.bits={.NBC2_CCR_HUE_SP1=128, .NBC2_CCR_HUE_SP2=127, .NBC2_CCR_HUE_GAIN1=64, .rsv_23=0, .NBC2_CCR_HUE_GAIN2=0, .rsv_31=0}},
    .l4lut1  ={.bits={.NBC2_CCR_SL2_X1=64, .NBC2_CCR_SL2_X2=128, .NBC2_CCR_SL2_X3=192, .rsv_24=0}},
    .l4lut2  ={.bits={.NBC2_CCR_SL2_GAIN0=16, .rsv_6=0, .NBC2_CCR_SL2_GAIN1=20, .rsv_14=0, .NBC2_CCR_SL2_GAIN2=24, .rsv_22=0, .NBC2_CCR_SL2_GAIN3=32, .rsv_30=0}},
    .l4lut3  ={.bits={.NBC2_CCR_SL2_SP0=8, .rsv_6=0, .NBC2_CCR_SL2_SP1=8, .rsv_14=0, .NBC2_CCR_SL2_SP2=16, .rsv_22=0, .NBC2_CCR_SL2_SP3=16, .rsv_30=0}},
};