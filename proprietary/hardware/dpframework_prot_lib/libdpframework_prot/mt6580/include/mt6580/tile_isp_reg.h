#ifndef __TILE_ISP_REG_H__
#define __TILE_ISP_REG_H__

/* error enum */
#define ISP_TILE_ERROR_MESSAGE_ENUM(n, CMD) \
	/* IMGO & IMG2O size check */\
    CMD(n, ISP_MESSAGE_IMGO_XSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_IMGO_YSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_IMG2O_XSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_IMG2O_YSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
	/* RSP CROP DISABLE error check */\
    CMD(n, ISP_MESSAGE_RSP_XS_BACK_FOR_DIFF_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_RSP_YS_BACK_FOR_DIFF_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_RSP_XE_BACK_SMALLER_THAN_FOR_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_RSP_YE_BACK_SMALLER_THAN_FOR_ERROR, ISP_TPIPE_MESSAGE_FAIL)\

/* register table (Cmodel, platform, tile driver) for SW parameters */
/* a, b, c, d, e reserved */
/* data type */
/* register name of current c model */
/* register name of HW ISP & platform parameters */
/* internal variable name of tile */
/* array bracket [xx] */
/* valid condition by tdr_en to print platform log with must string, default: false */
/* isp_reg.h reg name */
/* isp_reg.h field name */
#define ISP_TILE_SW_REG_LUT(CMD, a, b, c, d, e) \
    /* IMGI input width & height */\
    CMD(a, b, c, d, e, int, isp_tile_src_width, sw.src_width, isp_tile_src_width,, true,,)\
    CMD(a, b, c, d, e, int, isp_tile_src_height, sw.src_height, isp_tile_src_height,, true,,)\
    CMD(a, b, c, d, e, int, tile_width, sw.tpipe_width, isp_tile_width,, true,,)\
    CMD(a, b, c, d, e, int, tile_height, sw.tpipe_height, isp_tile_height,, true,,)\
    /* 0: stop final, 1: stop per line, 2: stop per tile*/\
    CMD(a, b, c, d, e, int, tile_irq_mode, sw.tpipe_irq_mode, last_irq_mode,, true,,)\

/* register table (Cmodel, platform, tile driver) for HW parameters */
/* a, b, c, d, e reserved */
/* data type */
/* register name of current c model */
/* register name of HW ISP & platform parameters */
/* internal variable name of tile */
/* array bracket [xx] */
/* valid condition by tdr_en to print platform log with must string, default: false */
#define ISP_TILE_HW_REG_LUT(CMD, a, b, c, d, e) \
    /* Common */\
    CMD(a, b, c, d, e, int, PIX_ID, top.pixel_id, PIX_ID,, true, CAM_CTL_PIX_ID, PIX_ID)\
    CMD(a, b, c, d, e, int, CSR_IN_FMT, top.cam_in_fmt, CAM_IN_FMT,, true, CAM_CTL_FMT_SEL, CAM_IN_FMT)\
    CMD(a, b, c, d, e, int, MODE, top.mode, MODE,, true, CAM_CTL_FMT_SEL, SUB_MODE)\
    CMD(a, b, c, d, e, int, SCENARIO, top.scenario, SCENARIO,, true, CAM_CTL_FMT_SEL, SCENARIO)\
    CMD(a, b, c, d, e, int, RGBW_SEL, top.debug_sel, DBG_SEL,, true, CAM_CTL_SEL, DBG_SEL)\
    /* module enable register */\
    CMD(a, b, c, d, e, int, TILE_IMGI_EN, top.imgi_en, IMGI_EN,, REG_CMP_EQ(c, LOG_IMGI_EN, 1), CAM_CTL_DMA_EN, IMGI_EN)\
    /* 2D BPC */\
    CMD(a, b, c, d, e, int, TILE_BPC_2D_EN, bnr.bpc_en, BPC_ENABLE,, REG_CMP_EQ(c, LOG_BNR_EN, 1), CAM_BPC_CON, BPC_ENABLE)\
    CMD(a, b, c, d, e, int, TILE_UNP_EN, top.unp_en, UNP_EN,, REG_CMP_EQ(c, LOG_UNP_EN, 1), CAM_CTL_EN1, UNP_EN)\
    CMD(a, b, c, d, e, int, TILE_BNR_EN, top.bnr_en, BNR_ENABLE,, REG_CMP_EQ(c, LOG_BNR_EN, 1), CAM_CTL_EN1, BNR_EN)\
    CMD(a, b, c, d, e, int, TILE_LSC_EN, top.lsc_en, LSC_EN,, REG_CMP_EQ(c, LOG_LSC_EN, 1), CAM_CTL_EN1, LSC_EN)\
    CMD(a, b, c, d, e, int, TILE_LSCI_EN, top.lsci_en, LSCI_EN,, REG_CMP_EQ(c, LOG_LSCI_EN, 1), CAM_CTL_DMA_EN, LSCI_EN)\
    CMD(a, b, c, d, e, int, TILE_SL2_EN, top.sl2_en, SL2_EN,, REG_CMP_EQ(c, LOG_SL2_EN, 1), CAM_CTL_EN1, SL2_EN)\
    CMD(a, b, c, d, e, int, TILE_422_444_EN, top.c24_en, C24_EN,, REG_CMP_EQ(c, LOG_C24_EN, 1), CAM_CTL_EN1, C24_EN)\
    CMD(a, b, c, d, e, int, TILE_CFA_EN, top.cfa_en, CFA_EN,, REG_CMP_EQ(c, LOG_CFA_EN, 1), CAM_CTL_EN1, CFA_EN)\
    CMD(a, b, c, d, e, int, TILE_LCE_EN, top.lce_en, LCE_EN,, REG_CMP_EQ(c, LOG_LCE_EN, 1), CAM_CTL_EN1, LCE_EN)\
    CMD(a, b, c, d, e, int, TILE_LCEI_EN, top.lcei_en, LCEI_EN,, REG_CMP_EQ(c, LOG_LCEI_EN, 1), CAM_CTL_DMA_EN, LCEI_EN)\
    CMD(a, b, c, d, e, int, TILE_444_422_EN, top.c42_en, C42_EN,, REG_CMP_EQ(c, LOG_C42_EN, 1), CAM_CTL_EN2, C42_EN)\
    CMD(a, b, c, d, e, int, TILE_NBC_EN, top.nbc_en, NBC_EN,, REG_CMP_EQ(c, LOG_NBC_EN, 1), CAM_CTL_EN2, NBC_EN)\
    CMD(a, b, c, d, e, int, TILE_SEEE_EN, top.seee_en, SEEE_EN,, REG_CMP_EQ(c, LOG_SEEE_EN, 1), CAM_CTL_EN2, SEEE_EN)\
    CMD(a, b, c, d, e, int, TILE_CDRZ_EN, top.cdrz_en, CDRZ_EN,, REG_CMP_EQ(c, LOG_CDRZ_EN, 1), CAM_CTL_EN2, CDRZ_EN)\
    CMD(a, b, c, d, e, int, TILE_IMGO_EN, top.imgo_en, IMGO_EN,, REG_CMP_EQ(c, LOG_IMGO_EN, 1), CAM_CTL_DMA_EN, IMGO_EN)\
    CMD(a, b, c, d, e, int, TILE_IMG2O_EN, top.img2o_en, IMG2O_EN,, REG_CMP_EQ(c, LOG_IMG2O_EN, 1), CAM_CTL_DMA_EN, IMG2O_EN)\
    CMD(a, b, c, d, e, int, TCM_LOAD_EN, top.tcm_load_en, TCM_LOAD_EN,, false, CAM_CTL_TCM_EN, TCM_LOAD_EN)\
    CMD(a, b, c, d, e, int, CTL_EXTENSION_EN, top.ctl_extension_en, CTL_EXTENSION_EN,, true, CAM_CTL_PIX_ID, CTL_EXTENSION_EN)\
    CMD(a, b, c, d, e, int, TILE_RSP_EN, top.rsp_en, RSP_EN,, true, CAM_CTL_EN2, UV_CRSA_EN)\
    CMD(a, b, c, d, e, int, TILE_MDP_CROP_EN, top.mdp_crop_en, MDP_CROP_EN,, true, CAM_CTL_CROP_X, MDP_CROP_EN)\
    CMD(a, b, c, d, e, int, MDP_SEL, top.mdp_sel, MDP_SEL,, true, CAM_CTL_PIX_ID, MDP_SEL)\
    /* CDRZ & IMG2O */\
    /* IMGI */\
    CMD(a, b, c, d, e, int, IMGI_STRIDE, imgi.imgi_stride, IMGI_STRIDE,, REG_CMP_EQ(c, LOG_IMGI_EN, 1), CAM_IMGI_STRIDE, STRIDE)\
	/* CFA */\
    CMD(a, b, c, d, e, int, BAYER_BYPASS, cfa.bayer_bypass, BAYER_BYPASS,, REG_CMP_EQ(c, LOG_CFA_EN, 1), CAM_CFA_BYPASS, BAYER_BYPASS)\
    /* LSC */\
    CMD(a, b, c, d, e, int, ShadingBlk_Xnum, lsc.sdblk_xnum, SDBLK_XNUM,, REG_CMP_EQ(c, LOG_LSC_EN, 1), CAM_LSC_CTL2, SDBLK_XNUM)\
    CMD(a, b, c, d, e, int, ShadingBlk_Ynum, lsc.sdblk_ynum, SDBLK_YNUM,, REG_CMP_EQ(c, LOG_LSC_EN, 1), CAM_LSC_CTL3, SDBLK_YNUM)\
    CMD(a, b, c, d, e, int, ShadingBlk_Width, lsc.sdblk_width, SDBLK_WIDTH,, REG_CMP_EQ(c, LOG_LSC_EN, 1), CAM_LSC_CTL2, SDBLK_WIDTH)\
    CMD(a, b, c, d, e, int, ShadingBlk_Height, lsc.sdblk_height, SDBLK_HEIGHT,, REG_CMP_EQ(c, LOG_LSC_EN, 1), CAM_LSC_CTL3, SDBLK_HEIGHT)\
    CMD(a, b, c, d, e, int, ShadingBlk_LastWidth, lsc.sdblk_last_width, SDBLK_lWIDTH,, REG_CMP_EQ(c, LOG_LSC_EN, 1), CAM_LSC_LBLOCK, SDBLK_lWIDTH)\
    CMD(a, b, c, d, e, int, ShadingBlk_LastHeight, lsc.sdblk_last_height, SDBLK_IHEIGHT,, REG_CMP_EQ(c, LOG_LSC_EN, 1), CAM_LSC_LBLOCK, SDBLK_lHEIGHT)\
    /* LSCI */\
    CMD(a, b, c, d, e, int, LSCI_STRIDE, lsci.lsci_stride, LSCI_STRIDE,, REG_CMP_EQ(c, LOG_LSCI_EN, 1), CAM_LSCI_STRIDE, STRIDE)\
    /* LCE */\
    CMD(a, b, c, d, e, int, LCE_SLM_WIDTH, lce.lce_slm_width, SLM_WIDTH,, REG_CMP_EQ(c, LOG_LCE_EN, 1), CAM_LCE_SLM_SIZE, LCE_SLM_WIDTH)\
    CMD(a, b, c, d, e, int, LCE_SLM_HEIGHT, lce.lce_slm_height, SLM_HEIGHT,, REG_CMP_EQ(c, LOG_LCE_EN, 1), CAM_LCE_SLM_SIZE, LCE_SLM_HEIGHT)\
    CMD(a, b, c, d, e, int, LCE_BC_MAG_KUBNX, lce.lce_bc_mag_kubnx, BC_MAG_KUBNX,, REG_CMP_EQ(c, LOG_LCE_EN, 1), CAM_LCE_ZR, LCE_BC_MAG_KUBNX)\
    CMD(a, b, c, d, e, int, LCE_BC_MAG_KUBNY, lce.lce_bc_mag_kubny, BC_MAG_KUBNY,, REG_CMP_EQ(c, LOG_LCE_EN, 1), CAM_LCE_ZR, LCE_BC_MAG_KUBNY)\
    /* LCEI */\
    CMD(a, b, c, d, e, int, LCEI_STRIDE, lcei.lcei_stride, LCEI_STRIDE,, REG_CMP_EQ(c, LOG_LCEI_EN, 1), CAM_LCEI_STRIDE, STRIDE)\
    /* IMGO */\
    CMD(a, b, c, d, e, int, IMGO_STRIDE, imgo.imgo_stride, IMGO_STRIDE,, REG_CMP_EQ(c, LOG_IMGO_EN, 1), CAM_IMGO_STRIDE, STRIDE)\
    CMD(a, b, c, d, e, int, IMGO_CROP_EN, imgo.imgo_crop_en, IMGO_CROP_EN,, REG_CMP_EQ(c, LOG_IMGO_EN, 1), CAM_CTL_TILE, IMGO_CROP_EN)\
    CMD(a, b, c, d, e, int, IMGO_XOFFSET, imgo.imgo_xoffset, IMGO_XOFFSET,, REG_CMP_EQ(c, LOG_IMGO_EN, 1), CAM_IMGO_CROP, XOFFSET)\
    CMD(a, b, c, d, e, int, IMGO_YOFFSET, imgo.imgo_yoffset, IMGO_YOFFSET,, REG_CMP_EQ(c, LOG_IMGO_EN, 1), CAM_IMGO_CROP, YOFFSET)\
    CMD(a, b, c, d, e, int, IMGO_XSIZE, imgo.imgo_xsize, IMGO_XSIZE,, REG_CMP_EQ(c, LOG_IMGO_EN, 1), CAM_IMGO_XSIZE, XSIZE)\
	CMD(a, b, c, d, e, int, IMGO_YSIZE, imgo.imgo_ysize, IMGO_YSIZE,, REG_CMP_EQ(c, LOG_IMGO_EN, 1), CAM_IMGO_YSIZE, YSIZE)\
	CMD(a, b, c, d, e, int, IMGO_MUX_EN, imgo.imgo_mux_en, IMGO_MUX_EN,, REG_CMP_EQ(c, LOG_IMGO_EN, 1), CAM_CTL_MUX_SEL2, IMGO_MUX_EN)\
    CMD(a, b, c, d, e, int, IMGO_MUX, imgo.imgo_mux, IMGO_MUX,, REG_CMP_EQ(c, LOG_IMGO_EN, 1), CAM_CTL_MUX_SEL2, IMGO_MUX)\
    /* IMG2O */\
    CMD(a, b, c, d, e, int, IMG2O_CROP_EN, img2o.img2o_crop_en, IMG2O_CROP_EN,, REG_CMP_EQ(c, LOG_IMG2O_EN, 1), CAM_CTL_TILE, IMG2O_CROP_EN)\
    CMD(a, b, c, d, e, int, IMG2O_STRIDE, img2o.img2o_stride, IMG2O_STRIDE,, REG_CMP_EQ(c, LOG_IMG2O_EN, 1), CAM_IMG2O_STRIDE, STRIDE)\
    CMD(a, b, c, d, e, int, IMG2O_XOFFSET, img2o.img2o_xoffset, IMG2O_XOFFSET,, REG_CMP_EQ(c, LOG_IMG2O_EN, 1), CAM_IMG2O_CROP, XOFFSET)\
    CMD(a, b, c, d, e, int, IMG2O_YOFFSET, img2o.img2o_yoffset, IMG2O_YOFFSET,, REG_CMP_EQ(c, LOG_IMG2O_EN, 1), CAM_IMG2O_CROP, YOFFSET)\
    CMD(a, b, c, d, e, int, IMG2O_XSIZE, img2o.img2o_xsize, IMG2O_XSIZE,, REG_CMP_EQ(c, LOG_IMG2O_EN, 1), CAM_IMG2O_XSIZE, XSIZE)\
    CMD(a, b, c, d, e, int, IMG2O_YSIZE, img2o.img2o_ysize, IMG2O_YSIZE,, REG_CMP_EQ(c, LOG_IMG2O_EN, 1), CAM_IMG2O_YSIZE, YSIZE)\
    CMD(a, b, c, d, e, int, IMG2O_MUX_EN, img2o.img2o_mux_en, IMG2O_MUX_EN,, REG_CMP_EQ(c, LOG_IMG2O_EN, 1), CAM_CTL_MUX_SEL2, IMG2O_MUX_EN)\
    CMD(a, b, c, d, e, int, IMG2O_MUX, img2o.img2o_mux, IMG2O_MUX,, REG_CMP_EQ(c, LOG_IMG2O_EN, 1), CAM_CTL_MUX_SEL2, IMG2O_MUX)\
    /* Common enable */\
    CMD(a, b, c, d, e, int, ANR_ENY, nbc.anr_eny, ANR_ENY,, REG_CMP_EQ(c, LOG_NBC_EN, 1), CAM_ANR_CON1, ANR_ENY)\
    CMD(a, b, c, d, e, int, ANR_ENC, nbc.anr_enc, ANR_ENC,, REG_CMP_EQ(c, LOG_NBC_EN, 1), CAM_ANR_CON1, ANR_ENC)\
    CMD(a, b, c, d, e, int, ANR_IIR_MODE, nbc.anr_iir_mode, ANR_IIR_MODE,, REG_CMP_EQ(c, LOG_NBC_EN, 1), CAM_ANR_CON1, ANR_IIR_MODE)\
    CMD(a, b, c, d, e, int, ANR_SCALE_MODE, nbc.anr_scale_mode, ANR_SCALE_MODE,, REG_CMP_EQ(c, LOG_NBC_EN, 1), CAM_ANR_CON1, ANR_SCALE_MODE)\
    CMD(a, b, c, d, e, int, SE_EDGE, seee.se_edge, SE_EDGE,, REG_CMP_EQ(c, LOG_SEEE_EN, 1), CAM_SEEE_EDGE_CTRL, SE_EDGE)\
    /* CDRZ reg */\
    CMD(a, b, c, d, e, int, CRZ_crop_size_x_d, cdrz.cdrz_input_crop_width, CDRZ_Input_Image_W,, REG_CMP_EQ(c, LOG_CDRZ_EN, 1), CAM_CDRZ_INPUT_IMAGE, CDRZ_Input_Image_W)\
    CMD(a, b, c, d, e, int, CRZ_crop_size_y_d, cdrz.cdrz_input_crop_height, CDRZ_Input_Image_H,, REG_CMP_EQ(c, LOG_CDRZ_EN, 1), CAM_CDRZ_INPUT_IMAGE, CDRZ_Input_Image_H)\
    CMD(a, b, c, d, e, int, CDRZ_Output_Image_W, cdrz.cdrz_output_width, CDRZ_Output_Image_W,, REG_CMP_EQ(c, LOG_CDRZ_EN, 1), CAM_CDRZ_OUTPUT_IMAGE, CDRZ_Output_Image_W)\
    CMD(a, b, c, d, e, int, CDRZ_Output_Image_H, cdrz.cdrz_output_height, CDRZ_Output_Image_H,, REG_CMP_EQ(c, LOG_CDRZ_EN, 1), CAM_CDRZ_OUTPUT_IMAGE, CDRZ_Output_Image_H)\
    CMD(a, b, c, d, e, int, CRZ_Bias_x_d, cdrz.cdrz_luma_horizontal_integer_offset, CDRZ_Luma_Horizontal_Integer_Offset,, REG_CMP_EQ(c, LOG_CDRZ_EN, 1), CAM_CDRZ_LUMA_HORIZONTAL_INTEGER_OFFSET, CDRZ_Luma_Horizontal_Integer_Offset)\
    CMD(a, b, c, d, e, int, CRZ_offset_x_d, cdrz.cdrz_luma_horizontal_subpixel_offset, CDRZ_Luma_Horizontal_Subpixel_Offset,, REG_CMP_EQ(c, LOG_CDRZ_EN, 1), CAM_CDRZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET, CDRZ_Luma_Horizontal_Subpixel_Offset)\
    CMD(a, b, c, d, e, int, CRZ_Bias_y_d, cdrz.cdrz_luma_vertical_integer_offset, CDRZ_Luma_Vertical_Integer_Offset,, REG_CMP_EQ(c, LOG_CDRZ_EN, 1), CAM_CDRZ_LUMA_VERTICAL_INTEGER_OFFSET, CDRZ_Luma_Vertical_Integer_Offset)\
    CMD(a, b, c, d, e, int, CRZ_offset_y_d, cdrz.cdrz_luma_vertical_subpixel_offset, CDRZ_Luma_Vertical_Subpixel_Offset,, REG_CMP_EQ(c, LOG_CDRZ_EN, 1), CAM_CDRZ_LUMA_VERTICAL_SUBPIXEL_OFFSET, CDRZ_Luma_Vertical_Subpixel_Offset)\
    CMD(a, b, c, d, e, int, CDRZ_Horizontal_Luma_Algorithm, cdrz.cdrz_horizontal_luma_algorithm, CDRZ_Horizontal_Luma_Algorithm,, REG_CMP_EQ(c, LOG_CDRZ_EN, 1), CAM_CDRZ_CONTROL, CDRZ_Horizontal_Algorithm)\
    CMD(a, b, c, d, e, int, CDRZ_Vertical_Luma_Algorithm, cdrz.cdrz_vertical_luma_algorithm, CDRZ_Vertical_Luma_Algorithm,, REG_CMP_EQ(c, LOG_CDRZ_EN, 1), CAM_CDRZ_CONTROL, CDRZ_Vertical_Algorithm)\
    CMD(a, b, c, d, e, int, CDRZ_Horizontal_Coeff_Step, cdrz.cdrz_horizontal_coeff_step, CDRZ_Horizontal_Coeff_Step,, REG_CMP_EQ(c, LOG_CDRZ_EN, 1), CAM_CDRZ_HORIZONTAL_COEFF_STEP, CDRZ_Horizontal_Coeff_Step)\
    CMD(a, b, c, d, e, int, CDRZ_Vertical_Coeff_Step, cdrz.cdrz_vertical_coeff_step, CDRZ_Vertical_Coeff_Step,, REG_CMP_EQ(c, LOG_CDRZ_EN, 1), CAM_CDRZ_VERTICAL_COEFF_STEP, CDRZ_Vertical_Coeff_Step)\
	/* SL2 */\
    CMD(a, b, c, d, e, int, SL2_HRZ_COMP, sl2.sl2_hrz_comp, SL2_HRZ_COMP,, REG_CMP_EQ(c, LOG_SL2_EN, 1), CAM_SL2_HRZ_COMP, SL2_HRZ_COMP)\
    /* interface mode */\
	CMD(a, b, c, d, e, int, INTERLACE_MODE, top.interlace_mode, INTERLACE_MODE,, true, CAM_IMGI_SLOW_DOWN, INTERLACE_MODE)\

/* register table (Cmodel, , tile driver) for Cmodel only parameters */
/* a, b, c, d, e reserved */
/* data type */
/* register name of current c model */
/* register name of HW ISP & platform parameters */
/* internal variable name of tile */
/* array bracket [xx] */
/* reserved */
#define ISP_TILE_CMODEL_PATH_LUT(CMD, a, b, c, d, e) \
    /* to add register only support by c model */\
    CMD(a, b, c, d, e, char *, ptr_tcm_dir_name,, data_path_ptr,,,,)\

/* register table (Cmodel, , tile driver) for Cmodel only parameters */
/* a, b, c, d, e reserved */
/* data type */
/* register name of current c model */
/* register name of HW ISP & platform parameters */
/* internal variable name of tile */
/* array bracket [xx] */
/* reserved */
#define ISP_TILE_CMODEL_REG_LUT(CMD, a, b, c, d, e) \
    /* to add register only support by c model */\
    CMD(a, b, c, d, e, int, TDRI_BASE_ADDR,, TDRI_BASE_ADDR,,,,)\

/* register table ( , platform, tile driver) for Platform only parameters */
/* a, b, c, e reserved */
/* data type */
/* register name of current c model */
/* register name of HW ISP & platform parameters */
/* internal variable name of tile */
/* array bracket [xx] */
/* valid condition by tdr_en to print platform log with must string, default: false */
#define ISP_TILE_PLATFORM_REG_LUT(CMD, a, b, c, d, e) \
    /* to add register only support by platform */\
    CMD(a, b, c, d, e, int,, sw.log_en, platform_log_en,, true,,)\

/* register table ( , , tile driver) for tile driver only parameters */
/* a, b, c, d, e reserved */
/* data type */
/* register name of current c model */
/* register name of HW ISP & platform parameters */
/* internal variable name of tile */
/* array bracket [xx] */
/* reserved */
#define ISP_TILE_PLATFORM_DEBUG_REG_LUT(CMD, a, b, c, d, e) \
    CMD(a, b, c, d, e, int,,, platform_buffer_size,,,,)\
    CMD(a, b, c, d, e, int,,, platform_max_tpipe_no,,,,)\
    CMD(a, b, c, d, e, int,,, platform_isp_hex_no_per_tpipe,,,,)\
    CMD(a, b, c, d, e, int,,, platform_error_no,,,,)\
	/* match id for reg & config */\
    CMD(a, b, c, d, e, unsigned int,,, tpipe_id,,,,)\

/* register table ( , , tile driver) for tile driver only parameters */
/* a, b, c, d, e reserved */
/* data type */
/* register name of current c model */
/* register name of HW ISP & platform parameters */
/* internal variable name of tile */
/* array bracket [xx] */
/* reserved */
#define ISP_TILE_INTERNAL_TDR_REG_LUT(CMD, a, b, c, d, e) \
    /* tdr used only */\
   /* CRZ Tile */\
    CMD(a, b, c, d, e, int,,, CDRZ_Tile_Input_Image_W,,,,)\
    CMD(a, b, c, d, e, int,,, CDRZ_Tile_Input_Image_H,,,,)\
    CMD(a, b, c, d, e, int,,, CDRZ_Tile_Output_Image_W,,,,)\
    CMD(a, b, c, d, e, int,,, CDRZ_Tile_Output_Image_H,,,,)\
    CMD(a, b, c, d, e, int,,, CDRZ_Tile_Luma_Horizontal_Integer_Offset,,,,)\
    CMD(a, b, c, d, e, int,,, CDRZ_Tile_Luma_Vertical_Integer_Offset,,,,)\
    CMD(a, b, c, d, e, int,,, CDRZ_Tile_Chroma_Horizontal_Integer_Offset,,,,)\
    CMD(a, b, c, d, e, int,,, CDRZ_Tile_Chroma_Vertical_Integer_Offset,,,,)\
    CMD(a, b, c, d, e, int,,, CDRZ_Tile_Luma_Horizontal_Subpixel_Offset,,,,)\
    CMD(a, b, c, d, e, int,,, CDRZ_Tile_Luma_Vertical_Subpixel_Offset,,,,)\
    CMD(a, b, c, d, e, int,,, CDRZ_Tile_Chroma_Horizontal_Subpixel_Offset,,,,)\
    CMD(a, b, c, d, e, int,,, CDRZ_Tile_Chroma_Vertical_Subpixel_Offset,,,,)\
    /* IMGI */\
    CMD(a, b, c, d, e, int,,, IMGI_TILE_OFFSET_ADDR,,,,)\
    CMD(a, b, c, d, e, int,,, IMGI_TILE_XSIZE,,,,)\
    CMD(a, b, c, d, e, int,,, IMGI_TILE_YSIZE,,,,)\
    /* IMGO */\
    CMD(a, b, c, d, e, int,,, IMGO_TILE_OFFSET_ADDR,,,,)\
    CMD(a, b, c, d, e, int,,, IMGO_TILE_XSIZE,,,,)\
    CMD(a, b, c, d, e, int,,, IMGO_TILE_YSIZE,,,,)\
    CMD(a, b, c, d, e, int,,, IMGO_TILE_XOFFSET,,,,)\
    CMD(a, b, c, d, e, int,,, IMGO_TILE_YOFFSET,,,,)\
    /* IMG2O */\
    CMD(a, b, c, d, e, int,,, IMG2O_TILE_OFFSET_ADDR,,,,)\
    CMD(a, b, c, d, e, int,,, IMG2O_TILE_XSIZE,,,,)\
    CMD(a, b, c, d, e, int,,, IMG2O_TILE_YSIZE,,,,)\
    CMD(a, b, c, d, e, int,,, IMG2O_TILE_XOFFSET,,,,)\
    CMD(a, b, c, d, e, int,,, IMG2O_TILE_YOFFSET,,,,)\
    /* LCEI */\
    CMD(a, b, c, d, e, int,,, LCEI_TILE_OFFSET_ADDR,,,,)\
    CMD(a, b, c, d, e, int,,, LCEI_TILE_XSIZE,,,,)\
    CMD(a, b, c, d, e, int,,, LCEI_TILE_YSIZE,,,,)\
    /* LCE */\
    CMD(a, b, c, d, e, int,,, LCE_TILE_OFFSET_X,,,,)\
    CMD(a, b, c, d, e, int,,, LCE_TILE_OFFSET_Y,,,,)\
    CMD(a, b, c, d, e, int,,, LCE_TILE_BIAS_X,,,,)\
    CMD(a, b, c, d, e, int,,, LCE_TILE_BIAS_Y,,,,)\
    CMD(a, b, c, d, e, int,,, lce_xsize,,,,)\
    CMD(a, b, c, d, e, int,,, lce_ysize,,,,)\
    CMD(a, b, c, d, e, int,,, lce_output_xsize,,,,)\
    CMD(a, b, c, d, e, int,,, lce_output_ysize,,,,)\
	/* UNP */\
    CMD(a, b, c, d, e, int,,, UNP_STARTBIT_OFFSET,,,,)\
    CMD(a, b, c, d, e, int,,, UNP_ENDTBIT_OFFSET,,,,)\
    /* LSCI */\
    CMD(a, b, c, d, e, int,,, LSCI_TILE_OFFSET_ADDR,,,,)\
    CMD(a, b, c, d, e, int,,, LSCI_TILE_XSIZE,,,,)\
    CMD(a, b, c, d, e, int,,, LSCI_TILE_YSIZE,,,,)\
    /* LSC tile */\
    CMD(a, b, c, d, e, int,,, LSC_TILE_XSIZE,,,,)\
    CMD(a, b, c, d, e, int,,, LSC_TILE_YSIZE,,,,)\
    CMD(a, b, c, d, e, int,,, LSC_TILE_XOFST,,,,)\
    CMD(a, b, c, d, e, int,,, LSC_TILE_YOFST,,,,)\
    CMD(a, b, c, d, e, int,,, LSC_XNUM,,,,)\
    CMD(a, b, c, d, e, int,,, LSC_YNUM,,,,)\
    CMD(a, b, c, d, e, int,,, LSC_LWIDTH,,,,)\
    CMD(a, b, c, d, e, int,,, LSC_LHEIGHT,,,,)\
    /* SL2 */\
    CMD(a, b, c, d, e, int,,, SL2_TILE_XOFF,,,,)\
    CMD(a, b, c, d, e, int,,, SL2_TILE_YOFF,,,,)\
	/* MDP_CROP_EN */\
    CMD(a, b, c, d, e, int,,, CTRL_MDP_XSTART,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_MDP_XEND,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_MDP_YSTART,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_MDP_YEND,,,,)\
    /* Internal */\
    CMD(a, b, c, d, e, int,,, CTRL_CDRZ_EN,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_RSP_EN,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_IMGO_EN,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_IMG2O_EN,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_MDP_CROP_EN,,,,)\

/* register table ( , , tile driver) for tile driver only parameters */
/* a, b, c, d, e reserved */
/* data type */
/* register name of current c model */
/* register name of HW ISP & platform parameters */
/* internal variable name of tile */
/* array bracket [xx] */
/* reserved */
#define ISP_TILE_INTERNAL_REG_LUT(CMD, a, b, c, d, e) \
    /* TCM_ENBALE updated by tile_xxx_tdr() */\
    CMD(a, b, c, d, e, int,,, TCM_IMGI_EN,,,,)\
    CMD(a, b, c, d, e, int,,, TCM_LSCI_EN,,,,)\
    CMD(a, b, c, d, e, int,,, TCM_LSC_EN,,,,)\
    CMD(a, b, c, d, e, int,,, TCM_IMGO_EN,,,,)\
    CMD(a, b, c, d, e, int,,, TCM_IMGO_CROP_EN,,,,)\
    CMD(a, b, c, d, e, int,,, TCM_IMG2O_EN,,,,)\
    CMD(a, b, c, d, e, int,,, TCM_IMG2O_CROP_EN,,,,)\
    CMD(a, b, c, d, e, int,,, TCM_LCEI_EN,,,,)\
    CMD(a, b, c, d, e, int,,, TCM_UNP_EN,,,,)\
    CMD(a, b, c, d, e, int,,, TCM_LCE_EN,,,,)\
    CMD(a, b, c, d, e, int,,, TCM_CDRZ_EN,,,,)\
    CMD(a, b, c, d, e, int,,, TCM_SL2_EN,,,,)\
    CMD(a, b, c, d, e, int,,, TCM_MDP_CROP_EN,,,,)\
    /* WDMA output size */\
    CMD(a, b, c, d, e, int,,, LSC_TILE_FULL_XSIZE,,,,)\
    CMD(a, b, c, d, e, int,,, LSC_TILE_FULL_YSIZE,,,,)\
    CMD(a, b, c, d, e, int,,, IMGO_TILE_FULL_XSIZE,,,,)\
    CMD(a, b, c, d, e, int,,, IMGO_TILE_FULL_YSIZE,,,,)\
    CMD(a, b, c, d, e, int,,, IMG2O_TILE_FULL_XSIZE,,,,)\
    CMD(a, b, c, d, e, int,,, IMG2O_TILE_FULL_YSIZE,,,,)\
    /* TCM_ENBALE updated by tile_xxx_tdr() */\
    CMD(a, b, c, d, e, int,,, LOG_IMGI_EN,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_BNR_EN,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_LSCI_EN,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_LSC_EN,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_CFA_EN,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_UNP_EN,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_LCEI_EN,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_LCE_EN,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_NBC_EN,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_SEEE_EN,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_CDRZ_EN,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_IMGO_EN,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_IMG2O_EN,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_SL2_EN,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_C24_EN,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_C42_EN,,,,)\

#endif
