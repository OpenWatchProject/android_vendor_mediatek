
const ISP_NVRAM_HFG_T HFG_%04d = {
    .con_0   ={.bits={.DIP_HFC_STD=6, .HFC_LCE_LINK_EN=1, .rsv_9=0}},
    .luma_0  ={.bits={.HFC_LUMA_CPX1=64, .HFC_LUMA_CPX2=128, .HFC_LUMA_CPX3=192, .rsv_24=0}},
    .luma_1  ={.bits={.HFC_LUMA_CPY0=32, .rsv_6=0, .HFC_LUMA_CPY1=28, .rsv_14=0, .HFC_LUMA_CPY2=24, .rsv_22=0, .HFC_LUMA_CPY3=20, .rsv_30=0}},
    .luma_2  ={.bits={.HFC_LUMA_SP0=28, .rsv_5=0, .HFC_LUMA_SP1=28, .rsv_13=0, .HFC_LUMA_SP2=28, .rsv_21=0, .HFC_LUMA_SP3=28, .rsv_29=0}},
    .lce_0   ={.bits={.HFC_LCE_CPX1=64, .HFC_LCE_CPX2=128, .HFC_LCE_CPX3=192, .rsv_24=0}},
    .lce_1   ={.bits={.HFC_LCE_CPY0=16, .rsv_6=0, .HFC_LCE_CPY1=20, .rsv_14=0, .HFC_LCE_CPY2=24, .rsv_22=0, .HFC_LCE_CPY3=28, .rsv_30=0}},
    .lce_2   ={.bits={.HFC_LCE_SP0=4, .rsv_5=0, .HFC_LCE_SP1=4, .rsv_13=0, .HFC_LCE_SP2=4, .rsv_21=0, .HFC_LCE_SP3=4, .rsv_29=0}},
};