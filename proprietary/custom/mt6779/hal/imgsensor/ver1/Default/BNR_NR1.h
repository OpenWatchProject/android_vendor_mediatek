
const ISP_NVRAM_BNR_NR1_T BNR_NR1_%04d = {
    .con     ={.bits={.rsv_0=0, .NR1_CT_EN=0, .rsv_5=0}},/*0x00000010*/
    .ct_con  ={.bits={.NR1_CT_MD=0, .NR1_CT_MD2=1, .NR1_CT_THRD=0, .rsv_14=0, .NR1_MBND=128, .rsv_26=0, .NR1_CT_SLOPE=1, .NR1_CT_DIV=2}},/*0x10a00004*/
    .ct_con2   ={.bits ={.NR1_CT_LWB=0, .rsv_12=0, .NR1_CT_UPB=0, .rsv_28=0}},
    .ct_con3   ={.bits ={.NR1_CT_LSP=1, .rsv_5=0, .NR1_CT_USP=1, .rsv_13=0}},
};