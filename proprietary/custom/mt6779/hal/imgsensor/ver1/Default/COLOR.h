const ISP_NVRAM_SCOLOR_PARAM_T COLOR_%04d = {
.FD_COLOR = {
    0,//FD_CM_EN
    0,//Priority
    
    0,//Range_W1_Y
    0,//Range_W1_S
    0,//Range_W1_H
    0,//Offset_W1_Y
    0,//Offset_W1_S
    0,//Offset_W1_H
    
    0,//Range_W2_Y
    0,//Range_W2_S
    0,//Range_W2_H
    0,//Offset_W2_Y
    0,//Offset_W2_S
    0,//Offset_W2_H
    
    0,//Range_W3_Y
    0,//Range_W3_S
    0,//Range_W3_H
    0,//Offset_W3_Y
    0,//Offset_W3_S
    0,//Offset_W3_H
    
    0,//Backup_0
    0,//Backup_1
    0,//Backup_2
    0,//Backup_3
    0,//Backup_4
    0,//Backup_5
    0,//Backup_6
    0,//Backup_7
    0,//Backup_8
    0,//Backup_9
    0,//Backup_10
    0,//Backup_11
    0,//Backup_12
    0,//Backup_13
    0,//Backup_14
    0,//Backup_15
    0,//Backup_16
    0,//Backup_17
    0,//Backup_18
    0,//Backup_19
    0,//Backup_20
    0,//Backup_21
    0,//Backup_22
    0 //Backup_23
},

.COLOR_TBL = {
    .cfg_main  ={.bits ={.COLOR_c2p_bypass=0, .COLOR_p2c_bypass=0, .COLOR_yeng_bypass=0, .COLOR_seng_bypass=0, .COLOR_heng_bypass=0, .rsv_5=0, .COLOR_all_bypass=0, .COLOR_wide_gamut_en=0, .rsv_9=0, .COLOR_seq_sel=0, .rsv_14=0, .COLOR_s_g_y_en=1, .COLOR_c_pp_cm_dbg_sel=0, .COLOR_lsp_en=1, .COLOR_lsp_sat_limit=1, .COLOR_lsp_ink_en=0, .COLOR_lsp_sat_src=1, .rsv_24=0}},
    .c_boost_main={.bits ={.COLOR_c_boost_gain=128, .rsv_8=0, .COLOR_c_new_boost_en=0, .rsv_14=0, .COLOR_c_new_boost_lmt_l=64, .COLOR_c_new_boost_lmt_u=255}},
    .c_boost_main_2={.bits ={.COLOR_color_cboost_yoffset=0, .rsv_8=0, .COLOR_color_cboost_yoffset_sel=0, .rsv_18=0, .COLOR_color_cboost_yconst=128}},
    .luma_adj  ={.bits ={.COLOR_y_level_adj=64, .rsv_7=0, .COLOR_y_slope_lmt=255, .rsv_16=0}},
    .g_pic_adj_main_1={.bits ={.COLOR_g_contrast=128, .rsv_10=0, .COLOR_g_brightness=1024, .rsv_27=0}},
    .g_pic_adj_main_2={.bits ={.COLOR_g_saturation=128, .rsv_10=0}},
    .y_slope_1_0_main={.bits ={.COLOR_y_slope_0=128, .rsv_8=0, .COLOR_y_slope_1=128, .rsv_24=0}},
    .y_slope_3_2_main={.bits ={.COLOR_y_slope_2=128, .rsv_8=0, .COLOR_y_slope_3=128, .rsv_24=0}},
    .y_slope_5_4_main={.bits ={.COLOR_y_slope_4=128, .rsv_8=0, .COLOR_y_slope_5=128, .rsv_24=0}},
    .y_slope_7_6_main={.bits ={.COLOR_y_slope_6=128, .rsv_8=0, .COLOR_y_slope_7=128, .rsv_24=0}},
    .y_slope_9_8_main={.bits ={.COLOR_y_slope_8=128, .rsv_8=0, .COLOR_y_slope_9=128, .rsv_24=0}},
    .y_slope_11_10_main={.bits ={.COLOR_y_slope_10=128, .rsv_8=0, .COLOR_y_slope_11=128, .rsv_24=0}},
    .y_slope_13_12_main={.bits ={.COLOR_y_slope_12=128, .rsv_8=0, .COLOR_y_slope_13=128, .rsv_24=0}},
    .y_slope_15_14_main={.bits ={.COLOR_y_slope_14=128, .rsv_8=0, .COLOR_y_slope_15=128, .rsv_24=0}},
    .local_hue_cd_0={.bits ={.COLOR_hue_to_hue_0=128, .COLOR_hue_to_hue_1=128, .COLOR_hue_to_hue_2=128, .COLOR_hue_to_hue_3=128}},
    .local_hue_cd_1={.bits ={.COLOR_hue_to_hue_4=128, .COLOR_hue_to_hue_5=128, .COLOR_hue_to_hue_6=128, .COLOR_hue_to_hue_7=128}},
    .local_hue_cd_2={.bits ={.COLOR_hue_to_hue_8=128, .COLOR_hue_to_hue_9=128, .COLOR_hue_to_hue_10=128, .COLOR_hue_to_hue_11=128}},
    .local_hue_cd_3={.bits ={.COLOR_hue_to_hue_12=128, .COLOR_hue_to_hue_13=128, .COLOR_hue_to_hue_14=128, .COLOR_hue_to_hue_15=128}},
    .local_hue_cd_4={.bits ={.COLOR_hue_to_hue_16=128, .COLOR_hue_to_hue_17=128, .COLOR_hue_to_hue_18=128, .COLOR_hue_to_hue_19=128}},
    .partial_sat_gain1_0={.bits ={.COLOR_sat_gain1_hue_0=128, .COLOR_sat_gain1_hue_1=128, .COLOR_sat_gain1_hue_2=128, .COLOR_sat_gain1_hue_3=128}},
    .partial_sat_gain1_1={.bits ={.COLOR_sat_gain1_hue_4=128, .COLOR_sat_gain1_hue_5=128, .COLOR_sat_gain1_hue_6=128, .COLOR_sat_gain1_hue_7=128}},
    .partial_sat_gain1_2={.bits ={.COLOR_sat_gain1_hue_8=128, .COLOR_sat_gain1_hue_9=128, .COLOR_sat_gain1_hue_10=128, .COLOR_sat_gain1_hue_11=128}},
    .partial_sat_gain1_3={.bits ={.COLOR_sat_gain1_hue_12=128, .COLOR_sat_gain1_hue_13=128, .COLOR_sat_gain1_hue_14=128, .COLOR_sat_gain1_hue_15=128}},
    .partial_sat_gain1_4={.bits ={.COLOR_sat_gain1_hue_16=128, .COLOR_sat_gain1_hue_17=128, .COLOR_sat_gain1_hue_18=128, .COLOR_sat_gain1_hue_19=128}},
    .partial_sat_gain2_0={.bits ={.COLOR_sat_gain2_hue_0=128, .COLOR_sat_gain2_hue_1=128, .COLOR_sat_gain2_hue_2=128, .COLOR_sat_gain2_hue_3=128}},
    .partial_sat_gain2_1={.bits ={.COLOR_sat_gain2_hue_4=128, .COLOR_sat_gain2_hue_5=128, .COLOR_sat_gain2_hue_6=128, .COLOR_sat_gain2_hue_7=128}},
    .partial_sat_gain2_2={.bits ={.COLOR_sat_gain2_hue_8=128, .COLOR_sat_gain2_hue_9=128, .COLOR_sat_gain2_hue_10=128, .COLOR_sat_gain2_hue_11=128}},
    .partial_sat_gain2_3={.bits ={.COLOR_sat_gain2_hue_12=128, .COLOR_sat_gain2_hue_13=128, .COLOR_sat_gain2_hue_14=128, .COLOR_sat_gain2_hue_15=128}},
    .partial_sat_gain2_4={.bits ={.COLOR_sat_gain2_hue_16=128, .COLOR_sat_gain2_hue_17=128, .COLOR_sat_gain2_hue_18=128, .COLOR_sat_gain2_hue_19=128}},
    .partial_sat_gain3_0={.bits ={.COLOR_sat_gain3_hue_0=128, .COLOR_sat_gain3_hue_1=128, .COLOR_sat_gain3_hue_2=128, .COLOR_sat_gain3_hue_3=128}},
    .partial_sat_gain3_1={.bits ={.COLOR_sat_gain3_hue_4=128, .COLOR_sat_gain3_hue_5=128, .COLOR_sat_gain3_hue_6=128, .COLOR_sat_gain3_hue_7=128}},
    .partial_sat_gain3_2={.bits ={.COLOR_sat_gain3_hue_8=128, .COLOR_sat_gain3_hue_9=128, .COLOR_sat_gain3_hue_10=128, .COLOR_sat_gain3_hue_11=128}},
    .partial_sat_gain3_3={.bits ={.COLOR_sat_gain3_hue_12=128, .COLOR_sat_gain3_hue_13=128, .COLOR_sat_gain3_hue_14=128, .COLOR_sat_gain3_hue_15=128}},
    .partial_sat_gain3_4={.bits ={.COLOR_sat_gain3_hue_16=128, .COLOR_sat_gain3_hue_17=128, .COLOR_sat_gain3_hue_18=128, .COLOR_sat_gain3_hue_19=128}},
    .partial_sat_point1_0={.bits ={.COLOR_sat_point1_hue_0=20, .COLOR_sat_point1_hue_1=20, .COLOR_sat_point1_hue_2=20, .COLOR_sat_point1_hue_3=20}},
    .partial_sat_point1_1={.bits ={.COLOR_sat_point1_hue_4=20, .COLOR_sat_point1_hue_5=20, .COLOR_sat_point1_hue_6=20, .COLOR_sat_point1_hue_7=20}},
    .partial_sat_point1_2={.bits ={.COLOR_sat_point1_hue_8=20, .COLOR_sat_point1_hue_9=20, .COLOR_sat_point1_hue_10=20, .COLOR_sat_point1_hue_11=20}},
    .partial_sat_point1_3={.bits ={.COLOR_sat_point1_hue_12=20, .COLOR_sat_point1_hue_13=20, .COLOR_sat_point1_hue_14=20, .COLOR_sat_point1_hue_15=20}},
    .partial_sat_point1_4={.bits ={.COLOR_sat_point1_hue_16=20, .COLOR_sat_point1_hue_17=20, .COLOR_sat_point1_hue_18=20, .COLOR_sat_point1_hue_19=20}},
    .partial_sat_point2_0={.bits ={.COLOR_sat_point2_hue_0=60, .COLOR_sat_point2_hue_1=60, .COLOR_sat_point2_hue_2=60, .COLOR_sat_point2_hue_3=60}},
    .partial_sat_point2_1={.bits ={.COLOR_sat_point2_hue_4=60, .COLOR_sat_point2_hue_5=60, .COLOR_sat_point2_hue_6=60, .COLOR_sat_point2_hue_7=60}},
    .partial_sat_point2_2={.bits ={.COLOR_sat_point2_hue_8=60, .COLOR_sat_point2_hue_9=60, .COLOR_sat_point2_hue_10=60, .COLOR_sat_point2_hue_11=60}},
    .partial_sat_point2_3={.bits ={.COLOR_sat_point2_hue_12=60, .COLOR_sat_point2_hue_13=60, .COLOR_sat_point2_hue_14=60, .COLOR_sat_point2_hue_15=60}},
    .partial_sat_point2_4={.bits ={.COLOR_sat_point2_hue_16=60, .COLOR_sat_point2_hue_17=60, .COLOR_sat_point2_hue_18=60, .COLOR_sat_point2_hue_19=60}},
    .s_gain_by_y0_0={.bits ={.COLOR_s_gain_y0_hue_0=128, .COLOR_s_gain_y0_hue_1=128, .COLOR_s_gain_y0_hue_2=128, .COLOR_s_gain_y0_hue_3=128}},
    .s_gain_by_y0_1={.bits ={.COLOR_s_gain_y0_hue_4=128, .COLOR_s_gain_y0_hue_5=128, .COLOR_s_gain_y0_hue_6=128, .COLOR_s_gain_y0_hue_7=128}},
    .s_gain_by_y0_2={.bits ={.COLOR_s_gain_y0_hue_8=128, .COLOR_s_gain_y0_hue_9=128, .COLOR_s_gain_y0_hue_10=128, .COLOR_s_gain_y0_hue_11=128}},
    .s_gain_by_y0_3={.bits ={.COLOR_s_gain_y0_hue_12=128, .COLOR_s_gain_y0_hue_13=128, .COLOR_s_gain_y0_hue_14=128, .COLOR_s_gain_y0_hue_15=128}},
    .s_gain_by_y0_4={.bits ={.COLOR_s_gain_y0_hue_16=128, .COLOR_s_gain_y0_hue_17=128, .COLOR_s_gain_y0_hue_18=128, .COLOR_s_gain_y0_hue_19=128}},
    .s_gain_by_y64_0={.bits ={.COLOR_s_gain_y64_hue_0=128, .COLOR_s_gain_y64_hue_1=128, .COLOR_s_gain_y64_hue_2=128, .COLOR_s_gain_y64_hue_3=128}},
    .s_gain_by_y64_1={.bits ={.COLOR_s_gain_y64_hue_4=128, .COLOR_s_gain_y64_hue_5=128, .COLOR_s_gain_y64_hue_6=128, .COLOR_s_gain_y64_hue_7=128}},
    .s_gain_by_y64_2={.bits ={.COLOR_s_gain_y64_hue_8=128, .COLOR_s_gain_y64_hue_9=128, .COLOR_s_gain_y64_hue_10=128, .COLOR_s_gain_y64_hue_11=128}},
    .s_gain_by_y64_3={.bits ={.COLOR_s_gain_y64_hue_12=128, .COLOR_s_gain_y64_hue_13=128, .COLOR_s_gain_y64_hue_14=128, .COLOR_s_gain_y64_hue_15=128}},
    .s_gain_by_y64_4={.bits ={.COLOR_s_gain_y64_hue_16=128, .COLOR_s_gain_y64_hue_17=128, .COLOR_s_gain_y64_hue_18=128, .COLOR_s_gain_y64_hue_19=128}},
    .s_gain_by_y128_0={.bits ={.COLOR_s_gain_y128_hue_0=128, .COLOR_s_gain_y128_hue_1=128, .COLOR_s_gain_y128_hue_2=128, .COLOR_s_gain_y128_hue_3=128}},
    .s_gain_by_y128_1={.bits ={.COLOR_s_gain_y128_hue_4=128, .COLOR_s_gain_y128_hue_5=128, .COLOR_s_gain_y128_hue_6=128, .COLOR_s_gain_y128_hue_7=128}},
    .s_gain_by_y128_2={.bits ={.COLOR_s_gain_y128_hue_8=128, .COLOR_s_gain_y128_hue_9=128, .COLOR_s_gain_y128_hue_10=128, .COLOR_s_gain_y128_hue_11=128}},
    .s_gain_by_y128_3={.bits ={.COLOR_s_gain_y128_hue_12=128, .COLOR_s_gain_y128_hue_13=128, .COLOR_s_gain_y128_hue_14=128, .COLOR_s_gain_y128_hue_15=128}},
    .s_gain_by_y128_4={.bits ={.COLOR_s_gain_y128_hue_16=128, .COLOR_s_gain_y128_hue_17=128, .COLOR_s_gain_y128_hue_18=128, .COLOR_s_gain_y128_hue_19=128}},
    .s_gain_by_y192_0={.bits ={.COLOR_s_gain_y192_hue_0=128, .COLOR_s_gain_y192_hue_1=128, .COLOR_s_gain_y192_hue_2=128, .COLOR_s_gain_y192_hue_3=128}},
    .s_gain_by_y192_1={.bits ={.COLOR_s_gain_y192_hue_4=128, .COLOR_s_gain_y192_hue_5=128, .COLOR_s_gain_y192_hue_6=128, .COLOR_s_gain_y192_hue_7=128}},
    .s_gain_by_y192_2={.bits ={.COLOR_s_gain_y192_hue_8=128, .COLOR_s_gain_y192_hue_9=128, .COLOR_s_gain_y192_hue_10=128, .COLOR_s_gain_y192_hue_11=128}},
    .s_gain_by_y192_3={.bits ={.COLOR_s_gain_y192_hue_12=128, .COLOR_s_gain_y192_hue_13=128, .COLOR_s_gain_y192_hue_14=128, .COLOR_s_gain_y192_hue_15=128}},
    .s_gain_by_y192_4={.bits ={.COLOR_s_gain_y192_hue_16=128, .COLOR_s_gain_y192_hue_17=128, .COLOR_s_gain_y192_hue_18=128, .COLOR_s_gain_y192_hue_19=128}},
    .s_gain_by_y256_0={.bits ={.COLOR_s_gain_y256_hue_0=128, .COLOR_s_gain_y256_hue_1=128, .COLOR_s_gain_y256_hue_2=128, .COLOR_s_gain_y256_hue_3=128}},
    .s_gain_by_y256_1={.bits ={.COLOR_s_gain_y256_hue_4=128, .COLOR_s_gain_y256_hue_5=128, .COLOR_s_gain_y256_hue_6=128, .COLOR_s_gain_y256_hue_7=128}},
    .s_gain_by_y256_2={.bits ={.COLOR_s_gain_y256_hue_8=128, .COLOR_s_gain_y256_hue_9=128, .COLOR_s_gain_y256_hue_10=128, .COLOR_s_gain_y256_hue_11=128}},
    .s_gain_by_y256_3={.bits ={.COLOR_s_gain_y256_hue_12=128, .COLOR_s_gain_y256_hue_13=128, .COLOR_s_gain_y256_hue_14=128, .COLOR_s_gain_y256_hue_15=128}},
    .s_gain_by_y256_4={.bits ={.COLOR_s_gain_y256_hue_16=128, .COLOR_s_gain_y256_hue_17=128, .COLOR_s_gain_y256_hue_18=128, .COLOR_s_gain_y256_hue_19=128}},
    .lsp_1     ={.bits ={.COLOR_lsp_lire_yslope=0, .COLOR_lsp_lire_sslope=127, .COLOR_lsp_lire_yth=0, .COLOR_lsp_lire_sth=0, .rsv_29=0}},
    .lsp_2     ={.bits ={.COLOR_lsp_lire_gain=127, .rsv_7=0, .COLOR_lsp_lsat_gain=127, .rsv_15=0, .COLOR_lsp_lsat_sth=0, .COLOR_lsp_lsat_sslope=127, .rsv_30=0}},
    .cm_control={.bits ={.COLOR_cm_bypass=0, .COLOR_cm_w1_en=0, .COLOR_cm_w1_wgt_en=1, .COLOR_cm_w1_ink_en=0, .COLOR_cm_w2_en=0, .COLOR_cm_w2_wgt_en=1, .COLOR_cm_w2_ink_en=0, .COLOR_cm_w3_en=0, .COLOR_cm_w3_wgt_en=1, .COLOR_cm_w3_ink_en=0, .COLOR_cm_ink_delta_mode=0, .rsv_11=0, .COLOR_cm_ink_ch=0, .rsv_14=0}},
    .cm_w1_hue_0={.bits={.COLOR_cm_h_w1_l=320, .COLOR_cm_h_w1_u=416, .COLOR_cm_h_w1_point0=336, .rsv_30=0}},
    .cm_w1_hue_1={.bits={.COLOR_cm_h_w1_point1=352, .COLOR_cm_h_w1_point2=368, .COLOR_cm_h_w1_point3=384, .rsv_30=0}},
    .cm_w1_hue_2={.bits={.COLOR_cm_h_w1_point4=400, .COLOR_cm_h_w1_slope0=128, .rsv_18=0, .COLOR_cm_h_w1_slope1=128, .rsv_28=0}},
    .cm_w1_hue_3={.bits={.COLOR_cm_h_w1_slope2=128, .COLOR_cm_h_w1_slope3=128, .COLOR_cm_h_w1_slope4=128, .COLOR_cm_h_w1_slope5=128}},
    .cm_w1_hue_4={.bits={.COLOR_cm_h_w1_wgt_lslope=21, .rsv_10=0, .COLOR_cm_h_w1_wgt_uslope=21, .rsv_26=0}},
    .cm_w1_luma_0={.bits={.COLOR_cm_y_w1_l=32, .COLOR_cm_y_w1_u=992, .COLOR_cm_y_w1_point0=192, .rsv_30=0}},
    .cm_w1_luma_1={.bits={.COLOR_cm_y_w1_point1=352, .COLOR_cm_y_w1_point2=512, .COLOR_cm_y_w1_point3=672, .rsv_30=0}},
    .cm_w1_luma_2={.bits={.COLOR_cm_y_w1_point4=832, .COLOR_cm_y_w1_slope0=128, .rsv_18=0, .COLOR_cm_y_w1_slope1=128, .rsv_28=0}},
    .cm_w1_luma_3={.bits={.COLOR_cm_y_w1_slope2=128, .COLOR_cm_y_w1_slope3=128, .COLOR_cm_y_w1_slope4=128, .COLOR_cm_y_w1_slope5=128}},
    .cm_w1_luma_4={.bits={.COLOR_cm_y_w1_wgt_lslope=17, .rsv_10=0, .COLOR_cm_y_w1_wgt_uslope=17, .rsv_26=0}},
    .cm_w1_sat_0={.bits={.COLOR_cm_s_w1_l=8, .COLOR_cm_s_w1_u=56, .COLOR_cm_s_w1_point0=16, .rsv_30=0}},
    .cm_w1_sat_1={.bits={.COLOR_cm_s_w1_point1=24, .COLOR_cm_s_w1_point2=32, .COLOR_cm_s_w1_point3=40, .rsv_30=0}},
    .cm_w1_sat_2={.bits={.COLOR_cm_s_w1_point4=48, .COLOR_cm_s_w1_slope0=128, .rsv_18=0, .COLOR_cm_s_w1_slope1=128, .rsv_28=0}},
    .cm_w1_sat_3={.bits={.COLOR_cm_s_w1_slope2=128, .COLOR_cm_s_w1_slope3=128, .COLOR_cm_s_w1_slope4=128, .COLOR_cm_s_w1_slope5=128}},
    .cm_w1_sat_4={.bits={.COLOR_cm_s_w1_wgt_lslope=85, .rsv_10=0, .COLOR_cm_s_w1_wgt_uslope=85, .rsv_26=0}},        
    .cm_w2_hue_0={.bits={.COLOR_cm_h_w2_l=544, .COLOR_cm_h_w2_u=736, .COLOR_cm_h_w2_point0=576, .rsv_30=0}},
    .cm_w2_hue_1={.bits={.COLOR_cm_h_w2_point1=608, .COLOR_cm_h_w2_point2=640, .COLOR_cm_h_w2_point3=672, .rsv_30=0}},
    .cm_w2_hue_2={.bits={.COLOR_cm_h_w2_point4=704, .COLOR_cm_h_w2_slope0=128, .rsv_18=0, .COLOR_cm_h_w2_slope1=128, .rsv_28=0}},
    .cm_w2_hue_3={.bits={.COLOR_cm_h_w2_slope2=128, .COLOR_cm_h_w2_slope3=128, .COLOR_cm_h_w2_slope4=128, .COLOR_cm_h_w2_slope5=128}},
    .cm_w2_hue_4={.bits={.COLOR_cm_h_w2_wgt_lslope=11, .rsv_10=0, .COLOR_cm_h_w2_wgt_uslope=11, .rsv_26=0}},
    .cm_w2_luma_0={.bits={.COLOR_cm_y_w2_l=32, .COLOR_cm_y_w2_u=992, .COLOR_cm_y_w2_point0=192, .rsv_30=0}},
    .cm_w2_luma_1={.bits={.COLOR_cm_y_w2_point1=352, .COLOR_cm_y_w2_point2=512, .COLOR_cm_y_w2_point3=672, .rsv_30=0}},
    .cm_w2_luma_2={.bits={.COLOR_cm_y_w2_point4=832, .COLOR_cm_y_w2_slope0=128, .rsv_18=0, .COLOR_cm_y_w2_slope1=128, .rsv_28=0}},
    .cm_w2_luma_3={.bits={.COLOR_cm_y_w2_slope2=128, .COLOR_cm_y_w2_slope3=128, .COLOR_cm_y_w2_slope4=128, .COLOR_cm_y_w2_slope5=128}},
    .cm_w2_luma_4={.bits={.COLOR_cm_y_w2_wgt_lslope=17, .rsv_10=0, .COLOR_cm_y_w2_wgt_uslope=17, .rsv_26=0}},
    .cm_w2_sat_0={.bits={.COLOR_cm_s_w2_l=8, .COLOR_cm_s_w2_u=116, .COLOR_cm_s_w2_point0=26, .rsv_30=0}},
    .cm_w2_sat_1={.bits={.COLOR_cm_s_w2_point1=44, .COLOR_cm_s_w2_point2=62, .COLOR_cm_s_w2_point3=80, .rsv_30=0}},
    .cm_w2_sat_2={.bits={.COLOR_cm_s_w2_point4=98, .COLOR_cm_s_w2_slope0=128, .rsv_18=0, .COLOR_cm_s_w2_slope1=128, .rsv_28=0}},
    .cm_w2_sat_3={.bits={.COLOR_cm_s_w2_slope2=128, .COLOR_cm_s_w2_slope3=128, .COLOR_cm_s_w2_slope4=128, .COLOR_cm_s_w2_slope5=128}},
    .cm_w2_sat_4={.bits={.COLOR_cm_s_w2_wgt_lslope=38, .rsv_10=0, .COLOR_cm_s_w2_wgt_uslope=38, .rsv_26=0}},
    .cm_w3_hue_0={.bits ={.COLOR_cm_h_w3_l=800, .COLOR_cm_h_w3_u=992, .COLOR_cm_h_w3_point0=832, .rsv_30=0}},
    .cm_w3_hue_1={.bits ={.COLOR_cm_h_w3_point1=864, .COLOR_cm_h_w3_point2=892, .COLOR_cm_h_w3_point3=928, .rsv_30=0}},
    .cm_w3_hue_2={.bits ={.COLOR_cm_h_w3_point4=960, .COLOR_cm_h_w3_slope0=128, .rsv_18=0, .COLOR_cm_h_w3_slope1=128, .rsv_28=0}},
    .cm_w3_hue_3={.bits ={.COLOR_cm_h_w3_slope2=128, .COLOR_cm_h_w3_slope3=128, .COLOR_cm_h_w3_slope4=128, .COLOR_cm_h_w3_slope5=128}},
    .cm_w3_hue_4={.bits ={.COLOR_cm_h_w3_wgt_lslope=11, .rsv_10=0, .COLOR_cm_h_w3_wgt_uslope=11, .rsv_26=0}},
    .cm_w3_luma_0={.bits ={.COLOR_cm_y_w3_l=423, .COLOR_cm_y_w3_u=1023, .COLOR_cm_y_w3_point0=523, .rsv_30=0}},
    .cm_w3_luma_1={.bits ={.COLOR_cm_y_w3_point1=623, .COLOR_cm_y_w3_point2=723, .COLOR_cm_y_w3_point3=823, .rsv_30=0}},
    .cm_w3_luma_2={.bits ={.COLOR_cm_y_w3_point4=923, .COLOR_cm_y_w3_slope0=128, .rsv_18=0, .COLOR_cm_y_w3_slope1=128, .rsv_28=0}},
    .cm_w3_luma_3={.bits ={.COLOR_cm_y_w3_slope2=128, .COLOR_cm_y_w3_slope3=128, .COLOR_cm_y_w3_slope4=128, .COLOR_cm_y_w3_slope5=128}},
    .cm_w3_luma_4={.bits ={.COLOR_cm_y_w3_wgt_lslope=27, .rsv_10=0, .COLOR_cm_y_w3_wgt_uslope=27, .rsv_26=0}},
    .cm_w3_sat_0={.bits ={.COLOR_cm_s_w3_l=8, .COLOR_cm_s_w3_u=110, .COLOR_cm_s_w3_point0=25, .rsv_30=0}},
    .cm_w3_sat_1={.bits ={.COLOR_cm_s_w3_point1=42, .COLOR_cm_s_w3_point2=59, .COLOR_cm_s_w3_point3=76, .rsv_30=0}},
    .cm_w3_sat_2={.bits ={.COLOR_cm_s_w3_point4=93, .COLOR_cm_s_w3_slope0=128, .rsv_18=0, .COLOR_cm_s_w3_slope1=128, .rsv_28=0}},
    .cm_w3_sat_3={.bits ={.COLOR_cm_s_w3_slope2=128, .COLOR_cm_s_w3_slope3=128, .COLOR_cm_s_w3_slope4=128, .COLOR_cm_s_w3_slope5=128}},
    .cm_w3_sat_4={.bits ={.COLOR_cm_s_w3_wgt_lslope=40, .rsv_10=0, .COLOR_cm_s_w3_wgt_uslope=40, .rsv_26=0}},
},
};