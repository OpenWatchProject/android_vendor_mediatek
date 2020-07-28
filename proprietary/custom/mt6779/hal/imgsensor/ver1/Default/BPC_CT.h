const ISP_NVRAM_BPC_CT_T BPC_CT_%04d = {
    .con1      ={.bits ={.rsv_0=0, .BPC_CT_EN=1, .rsv_5=0}},
    .con2      ={.bits ={.BPC_CT_MD=0, .BPC_CT_MD2=1, .BPC_CT_THRD=0, .rsv_14=0, .BPC_CT_MBND=128, .rsv_26=0, .BPC_CT_SLOPE=1, .BPC_CT_DIV=2}},
    .bld1      ={.bits ={.BPC_CT_UPB=0, .BPC_CT_USP=1, .rsv_17=0}},
    .bld2      ={.bits ={.BPC_CT_LWB=0, .BPC_CT_LSP=1, .rsv_17=0}},
};
