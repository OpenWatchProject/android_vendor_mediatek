const ISP_NVRAM_CNR_ABF_T CNR_ABF_%04d = {
    .con1      ={.bits ={.CNR_ABF_EN=1, .rsv_1=0, .CNR_ABF_BIL_IDX=2, .CNR_ABF_NSR_IDX=1, .rsv_12=0}},
    .con2      ={.bits ={.CNR_ABF_BF_U_OFST=20, .rsv_6=0}},
    .rcon      ={.bits ={.CNR_ABF_R1=190, .rsv_9=0, .CNR_ABF_R2=322, .rsv_25=0}},
    .ylut      ={.bits ={.CNR_ABF_Y0=0, .CNR_ABF_Y1=1, .CNR_ABF_Y2=150, .CNR_ABF_Y3=186}},
    .cxlut     ={.bits ={.CNR_ABF_CX0=126, .CNR_ABF_CX1=138, .CNR_ABF_CX2=198, .CNR_ABF_CX3=210}},
    .cylut     ={.bits ={.CNR_ABF_CY0=132, .CNR_ABF_CY1=140, .CNR_ABF_CY2=184, .CNR_ABF_CY3=205}},
    .ysp       ={.bits ={.CNR_ABF_Y_SP0=511, .rsv_10=0, .CNR_ABF_Y_SP1=1010, .rsv_26=0}},
    .cxsp      ={.bits ={.CNR_ABF_CX_SP0=56, .rsv_10=0, .CNR_ABF_CX_SP1=988, .rsv_26=0}},
    .cysp      ={.bits ={.CNR_ABF_CY_SP0=64, .rsv_10=0, .CNR_ABF_CY_SP1=1000, .rsv_26=0}},
    .clp       ={.bits ={.CNR_ABF_STHRE_R=250, .CNR_ABF_STHRE_G=250, .CNR_ABF_STHRE_B=250, .rsv_24=0}},
};
