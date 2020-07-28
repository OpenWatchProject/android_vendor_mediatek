#ifndef __TILE_ISP_REG_H__
#define __TILE_ISP_REG_H__

/* error enum */
#define ISP_TILE_ERROR_MESSAGE_ENUM(n, CMD) \
	/* Raw check */\
    CMD(n, ISP_MESSAGE_INVALID_SRC_SIZE_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_INVALID_TILE_RAW_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_INVALID_VIPI_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_INVALID_SRC_STRIDE_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_INVALID_WPE_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_INVALID_MFB_DL_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
	/* LSC check */\
    CMD(n, ISP_MESSAGE_LSC_ZERO_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
	/* LTM check */\
    CMD(n, ISP_MESSAGE_LTM_INVALID_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
	/* IMGBI check */\
    CMD(n, ISP_MESSAGE_IMGBI_FORMAT_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* IMGCI check */\
    CMD(n, ISP_MESSAGE_IMGCI_FORMAT_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* IMG2O size check */\
    CMD(n, ISP_MESSAGE_IMG2O_XSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_IMG2O_YSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_IMG2BO_XSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_IMG2BO_YSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* IMG3O size check */\
    CMD(n, ISP_MESSAGE_IMG3O_CROP_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_IMG3O_XSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_IMG3O_YSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* IMG3BO size check */\
    CMD(n, ISP_MESSAGE_IMG3BO_CROP_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_IMG3BO_XSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_IMG3BO_YSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* IMG3CO size check */\
    CMD(n, ISP_MESSAGE_IMG3CO_CROP_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_IMG3CO_XSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_IMG3CO_YSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* RSP CROP DISABLE error check */\
    CMD(n, ISP_MESSAGE_RSP_XS_BACK_FOR_DIFF_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_RSP_YS_BACK_FOR_DIFF_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_RSP_XE_BACK_SMALLER_THAN_FOR_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_RSP_YE_BACK_SMALLER_THAN_FOR_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* MFB FE check */\
    CMD(n, ISP_MESSAGE_UNKNOWN_DFE_MODE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TOO_SMALL_DFE_INPUT_XSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TOO_SMALL_DFE_INPUT_YSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TOO_SMALL_TILE_WIDTH_FOR_DFE_OUT_XE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TOO_SMALL_TILE_HEIGHT_FOR_DFE_OUT_YE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_NOT_SUPPORT_DFE_IP_WIDTH_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_NOT_SUPPORT_DFE_IP_HEIGHT_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_CONFIG_DFE_INPUT_SIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* ISP MUX check */\
    CMD(n, ISP_MESSAGE_ILLEGAL_RAW_PATH_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_ILLEGAL_YUV_PATH_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_ILLEGAL_RGB_PATH_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_ILLEGAL_MIX1_SEL_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_ILLEGAL_CRZ_SEL_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_ILLEGAL_SRZ1_SEL_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_ILLEGAL_TIMGO_SEL_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_ILLEGAL_IMGI_FMT_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
	/* 3DNR configuration check */\
    CMD(n, ISP_MESSAGE_ILLEGAL_3DNR_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_ILLEGAL_3DNR_VALID_WINDOW_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_ILLEGAL_VIPI_CROP_XSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_ILLEGAL_VIPI_CROP_YSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_ILLEGAL_VIPBI_CROP_XSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_ILLEGAL_VIPBI_CROP_YSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_ILLEGAL_VIPCI_CROP_XSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_ILLEGAL_VIPCI_CROP_YSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
	/* SRZ check */\
    CMD(n, ISP_MESSAGE_NOT_SUPPORT_OFFSET_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_NOT_SUPPORT_VFLIP_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
	/* C02 check */\
    CMD(n, ISP_MESSAGE_INVALID_C02_EN_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_INVALID_C02D2_EN_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* resizer coeff check */\
    CMD(n, ISP_MESSAGE_RESIZER_UNMATCH_INPUT_SIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* TIMGO size check */\
    CMD(n, ISP_MESSAGE_TIMGO_XSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TIMGO_YSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TIMGO_XCROP_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TIMGO_YCROP_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* DCESO size check */\
    CMD(n, ISP_MESSAGE_DCESO_XSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
	/* min size constraints */\
    CMD(n, ISP_MESSAGE_UNDER_MIN_XSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_UNDER_MIN_YSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
	/* WPE size check */\
    CMD(n, ISP_MESSAGE_WPEO_XSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_INVALID_CACHI_FMT_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
	/* ADL config check */\
    CMD(n, ISP_MESSAGE_INVALID_ADL_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
	/* FEO config check */\
    CMD(n, ISP_MESSAGE_INVALID_FEO_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
	/* SMT config check */\
    CMD(n, ISP_MESSAGE_INVALID_SMT_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_INVALID_SMT_OFFSET_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_INVALID_SMT_SMALL_DMA_SIZE_PARTIAL_DUMP_WARNING, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_INVALID_SMT_SMALL_DMA_SIZE_DISABLE_DUMP_WARNING, ISP_TPIPE_MESSAGE_FAIL)\
	/* MFB config check */\
    CMD(n, ISP_MESSAGE_INVALID_MFB_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* MFBI check */\
    CMD(n, ISP_MESSAGE_MFBI_XSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_MFBI_YSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* MFBO size check */\
    CMD(n, ISP_MESSAGE_MFBO_CROP_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_MFBO_XSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_MFBO_B_CROP_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* DMA align check */\
    CMD(n, ISP_MESSAGE_WRONG_IMGI_STRIDE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_WRONG_IMGBI_STRIDE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_WRONG_IMGCI_STRIDE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_WRONG_IMG3O_STRIDE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_WRONG_IMG3BO_STRIDE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_WRONG_IMG3CO_STRIDE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_WRONG_MFBI_STRIDE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_WRONG_MFBI_B_STRIDE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_WRONG_MFBO_STRIDE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_WRONG_MFBO_B_STRIDE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* OFT R2B size check */\
    CMD(n, ISP_MESSAGE_OFT_R2B_XSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_OFT_R2B_YSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_OFT_R2B_XCROP_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_OFT_R2B_YCROP_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_ILLEGAL_OFT_R2B_BPP_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_ILLEGAL_OFT_R2B_DEPTH_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* OFTL R2B size check */\
    CMD(n, ISP_MESSAGE_OFTL_R2B_XSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_OFTL_R2B_YSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_OFTL_R2B_XCROP_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_OFTL_R2B_YCROP_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_ILLEGAL_OFTL_R2B_BPP_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\

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
/* direct-link param 0: must be equal, 1: replaced by MDP, 2: don't care */
#define ISP_TILE_SW_REG_LUT(CMD, a, b, c, d, e) \
	/* common part */\
    /* 0: stop final, 1: stop per line, 2: stop per tile*/\
    CMD(a, b, c, d, e, int, TILE_IRQ_MODE, sw.tpipe_irq_mode, last_irq_mode,, true,,, 1, TILE_IRQ_MODE)\
    CMD(a, b, c, d, e, int, TILE_SEL_CAL, sw.tpipe_sel_mode, tile_sel_mode,, true,,, 1, TILE_SEL_CAL)\

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
/* direct-link param 0: must be equal, 1: replaced by MDP, 2: don't care */
#define DIP_TILE_SW_REG_LUT(CMD, a, b, c, d, e) \
    /* IMGI input width & height */\
    CMD(a, b, c, d, e, int, CTL_IMGI_XSIZE_PIX_DIP_A, sw.src_width, isp_tile_src_width,, true,,, 0, CTL_IMGI_XSIZE_PIX_DIP_B)\
    CMD(a, b, c, d, e, int, CTL_IMGI_YSIZE_PIX_DIP_A, sw.src_height, isp_tile_src_height,, true,,, 0, CTL_IMGI_YSIZE_PIX_DIP_B)\
    CMD(a, b, c, d, e, int, TILE_WIDTH_DIP_A, sw.tpipe_width, isp_tile_width,, true,,, 1, TILE_WIDTH_DIP_B)\
    CMD(a, b, c, d, e, int, TILE_HEIGHT_DIP_A, sw.tpipe_height, isp_tile_height,, true,,, 1, TILE_HEIGHT_DIP_B)\
    CMD(a, b, c, d, e, int, LCE_D1A_FULL_XOFF, lce.lce_full_xoff, lce_full_xoff,, true,,, 1, LCE_D1B_FULL_XOFF)\
    CMD(a, b, c, d, e, int, LCE_D1A_FULL_YOFF, lce.lce_full_yoff, lce_full_yoff,, true,,, 1, LCE_D1B_FULL_YOFF)\
    CMD(a, b, c, d, e, int, LCE_D1A_FULL_SLM_WD, lce.lce_full_slm_width, lce_full_slm_width,, true,,, 1, LCE_D1B_FULL_SLM_WD)\
    CMD(a, b, c, d, e, int, LCE_D1A_FULL_SLM_HT, lce.lce_full_slm_height, lce_full_slm_height,, true,,, 1, LCE_D1B_FULL_SLM_HT)\
    CMD(a, b, c, d, e, int, LCE_D1A_FULL_OUT_HT, lce.lce_full_out_height, lce_full_out_height,, true,,, 1, LCE_D1B_FULL_OUT_HT)\
    CMD(a, b, c, d, e, int, TILE_DUAL_EN, sw.tpipe_dual_en, tile_dual_en,, true,,, 1, TILE_DUAL_EN)\

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
/* direct-link param 0: must be equal, 1: replaced by MDP, 2: don't care */
#define WPE_TILE_SW_REG_LUT(CMD, a, b, c, d, e) \
    /* IMGI input width & height */\
    CMD(a, b, c, d, e, int, CTL_IMGI_XSIZE_PIX_WPE_A, sw.src_width_wpe, isp_tile_src_width_wpe,, true,,, 0, CTL_IMGI_XSIZE_PIX_WPE_B)\
    CMD(a, b, c, d, e, int, CTL_IMGI_YSIZE_PIX_WPE_A, sw.src_height_wpe, isp_tile_src_height_wpe,, true,,, 0, CTL_IMGI_YSIZE_PIX_WPE_B)\
    CMD(a, b, c, d, e, int, TILE_WIDTH_WPE_A, sw.tpipe_width_wpe, isp_tile_width_wpe,, true,,, 1, TILE_WIDTH_WPE_B)\
    CMD(a, b, c, d, e, int, TILE_HEIGHT_WPE_A, sw.tpipe_height_wpe, isp_tile_height_wpe,, true,,, 1, TILE_HEIGHT_WPE_B)\

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
/* direct-link param 0: must be equal, 1: replaced by MDP, 2: don't care */
#define MFB_TILE_SW_REG_LUT(CMD, a, b, c, d, e) \
    /* IMGI input width & height */\
    CMD(a, b, c, d, e, int, CTL_MFBI_XSIZE_PIX_MFB_A, sw.src_width_mfb, isp_tile_src_width_mfb,, true,,, 0, CTL_MFBI_XSIZE_PIX_MFB_A)\
    CMD(a, b, c, d, e, int, CTL_MFBI_YSIZE_PIX_MFB_A, sw.src_height_mfb, isp_tile_src_height_mfb,, true,,, 0, CTL_MFBI_YSIZE_PIX_MFB_A)\
    CMD(a, b, c, d, e, int, TILE_WIDTH_MFB_A, sw.tpipe_width_mfb, isp_tile_width_mfb,, true,,, 1, TILE_WIDTH_MFB_A)\
    CMD(a, b, c, d, e, int, TILE_HEIGHT_MFB_A, sw.tpipe_height_mfb, isp_tile_height_mfb,, true,,, 1, TILE_HEIGHT_MFB_A)\

/* register table (Cmodel, platform, tile driver) for HW parameters */
/* a, b, c, d, e reserved */
/* data type */
/* register name of current c model */
/* register name of HW ISP & platform parameters */
/* internal variable name of tile */
/* array bracket [xx] */
/* valid condition by tdr_en to print platform log with must string, default: false */
/* isp_reg.h reg name */
/* isp_reg.h field name */
/* direct-link param 0: must be equal, 1: replaced by MDP, 2: don't care, 4: shold compare isp_reg and reg map in program */
#define DIP_TILE_HW_REG_LUT(CMD, a, b, c, d, e) \
    /* Common */\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_PIX_ID, top.pixel_id, PIX_ID,, true, DIPCTL_D1A_DIPCTL_MISC_SEL, DIPCTL_PIX_ID, 4, DIPCTL_D1B_PIX_ID)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_IMGI_D1_FMT, top.cam_in_fmt, CAM_IN_FMT,, true, DIPCTL_D1A_DIPCTL_FMT_SEL1, DIPCTL_IMGI_D1_FMT, 4, DIPCTL_D1B_IMGI_D1_FMT)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_TDR_CTL_EXT_EN, top.ctl_extension_en, CTL_EXTENSION_EN,, true, DIPCTL_D1A_DIPCTL_TDR_SEL, DIPCTL_TDR_CTL_EXT_EN, 4, CTL_EXTENSION_B_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_FG_MODE, top.fg_mode, FG_MODE,, true, DIPCTL_D1A_DIPCTL_MISC_SEL, DIPCTL_FG_MODE, 4, DIPCTL_D1B_FG_MODE, DIPCTL_D1B_DIPCTL_MISC_SEL, DIPCTL_FG_MODE)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_PAKG_D1_FG_OUT, top.pakg_fg_out, PAKG_FG_OUT,, true, DIPCTL_D1A_DIPCTL_MISC_SEL, DIPCTL_PAKG_D1_FG_OUT, 4, DIPCTL_D1B_PAKG_D1_FG_OUT)\
    CMD(a, b, c, d, e, int, DIPDMATOP_D1A_UFO_IMGI_D1_EN, top.ufo_imgi_en, UFO_IMGI_EN,, true, DIPCTL_D1A_DIPCTL_MISC_SEL, DIPCTL_PAKG_D1_FG_OUT, 4, DIPDMATOP_D1B_UFO_IMGI_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_UFDI_D1_FMT, top.ufdi_fmt, UFDI_FMT,, true, DIPCTL_D1A_DIPCTL_FMT_SEL1, DIPCTL_UFDI_D1_FMT, 4, DIPCTL_D1B_UFDI_D1_FMT)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_VIPI_D1_FMT, top.vipi_fmt, VIPI_FMT,, true, DIPCTL_D1A_DIPCTL_FMT_SEL1, DIPCTL_VIPI_D1_FMT, 4, DIPCTL_D1B_VIPI_D1_FMT)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_IMG3O_D1_FMT, top.img3o_fmt, IMG3O_FMT,, true, DIPCTL_D1A_DIPCTL_FMT_SEL2, DIPCTL_IMG3O_D1_FMT, 4, DIPCTL_D1B_IMG3O_D1_FMT)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_CRZO_D1_FMT, top.crzo_fmt, CRZO_FMT,, true, DIPCTL_D1A_DIPCTL_FMT_SEL2, DIPCTL_CRZO_D1_FMT, 4, DIPCTL_D1B_CRZO_D1_FMT)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_TIMGO_D1_FMT, top.timgo_fmt, TIMGO_FMT,, true, DIPCTL_D1A_DIPCTL_FMT_SEL2, DIPCTL_TIMGO_D1_FMT, 4, DIPCTL_D1B_TIMGO_D1_FMT)\
    /* module enable register */\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_IMGI_D1_EN, top.imgi_en, IMGI_EN,, true, DIPCTL_D1A_DIPCTL_DMA_EN1, DIPCTL_IMGI_D1_EN, 4, DIPCTL_D1B_IMGI_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_IMGBI_D1_EN, top.imgbi_en, IMGBI_EN,, true, DIPCTL_D1A_DIPCTL_DMA_EN1, DIPCTL_IMGBI_D1_EN, 4, DIPCTL_D1A_IMGBI_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_IMGCI_D1_EN, top.imgci_en, IMGCI_EN,, true, DIPCTL_D1A_DIPCTL_DMA_EN1, DIPCTL_IMGCI_D1_EN, 4, DIPCTL_D1A_IMGCI_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_VIPI_D1_EN, top.vipi_en, VIPI_EN,, true, DIPCTL_D1A_DIPCTL_DMA_EN2, DIPCTL_VIPI_D1_EN, 4, DIPCTL_D1B_VIPI_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_VIPBI_D1_EN, top.vipbi_en, VIPBI_EN,, true, DIPCTL_D1A_DIPCTL_DMA_EN2, DIPCTL_VIPBI_D1_EN, 4, DIPCTL_D1B_VIPBI_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_VIPCI_D1_EN, top.vipci_en, VIPCI_EN,, true, DIPCTL_D1A_DIPCTL_DMA_EN2, DIPCTL_VIPCI_D1_EN, 4, DIPCTL_D1B_VIPCI_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_UFDI_D1_EN, top.ufdi_en, UFDI_EN,, true, DIPCTL_D1A_DIPCTL_DMA_EN1, DIPCTL_UFDI_D1_EN, 4, DIPCTL_D1B_UFDI_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_DMGI_D1_EN, top.dmgi_en, DMGI_EN,, true, DIPCTL_D1A_DIPCTL_DMA_EN1, DIPCTL_DMGI_D1_EN, 4, DIPCTL_D1B_DMGI_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_DEPI_D1_EN, top.depi_en, DEPI_EN,, true, DIPCTL_D1A_DIPCTL_DMA_EN1, DIPCTL_DEPI_D1_EN, 4, DIPCTL_D1B_UFDI_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_LCEI_D1_EN, top.lcei_en, LCEI_EN,, true, DIPCTL_D1A_DIPCTL_DMA_EN1, DIPCTL_LCEI_D1_EN, 4, DIPCTL_D1B_LCEI_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_SMTI_D1_EN, top.smt1i_en, SMT1I_EN,, true, DIPCTL_D1A_DIPCTL_DMA_EN1, DIPCTL_SMTI_D1_EN, 4, DIPCTL_D1B_SMTI_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_SMTI_D2_EN, top.smt2i_en, SMT2I_EN,, true, DIPCTL_D1A_DIPCTL_DMA_EN1, DIPCTL_SMTI_D1_EN, 4, DIPCTL_D1B_SMTI_D2_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_SMTI_D3_EN, top.smt3i_en, SMT3I_EN,, true, DIPCTL_D1A_DIPCTL_DMA_EN1, DIPCTL_SMTI_D1_EN, 4, DIPCTL_D1B_SMTI_D3_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_SMTI_D4_EN, top.smt4i_en, SMT4I_EN,, true, DIPCTL_D1A_DIPCTL_DMA_EN1, DIPCTL_SMTI_D1_EN, 4, DIPCTL_D1B_SMTI_D4_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_SMTO_D1_EN, top.smt1o_en, SMT1O_EN,, true, DIPCTL_D1A_DIPCTL_DMA_EN1, DIPCTL_SMTO_D1_EN, 4, DIPCTL_D1B_SMTO_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_SMTO_D2_EN, top.smt2o_en, SMT2O_EN,, true, DIPCTL_D1A_DIPCTL_DMA_EN1, DIPCTL_SMTO_D2_EN, 4, DIPCTL_D1B_SMTO_D2_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_SMTO_D3_EN, top.smt3o_en, SMT3O_EN,, true, DIPCTL_D1A_DIPCTL_DMA_EN1, DIPCTL_SMTO_D3_EN, 4, DIPCTL_D1B_SMTO_D3_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_SMTO_D4_EN, top.smt4o_en, SMT4O_EN,, true, DIPCTL_D1A_DIPCTL_DMA_EN1, DIPCTL_SMTO_D4_EN, 4, DIPCTL_D1B_SMTO_D4_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_SMT_D1_EN, top.smt1_en, SMT1_EN,, true, DIPCTL_D1A_DIPCTL_RGB_EN1, DIPCTL_SMT_D1_EN, 4, DIPCTL_D1B_SMT_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_SMT_D2_EN, top.smt2_en, SMT2_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN1, DIPCTL_SMT_D2_EN, 4, DIPCTL_D1B_SMT_D2_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_SMT_D3_EN, top.smt3_en, SMT3_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN1, DIPCTL_SMT_D3_EN, 4, DIPCTL_D1B_SMT_D3_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_SMT_D4_EN, top.smt4_en, SMT4_EN,, true, DIPCTL_D1A_DIPCTL_RGB_EN2, DIPCTL_SMT_D4_EN, 4, DIPCTL_D1B_SMT_D4_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_UNP_D1_EN, top.unp_en, UNP_EN,, true, DIPCTL_D1A_DIPCTL_RGB_EN1, DIPCTL_UNP_D1_EN, 4, DIPCTL_D1B_UNP_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_UNP_D2_EN, top.unp2_en, UNP2_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN1, DIPCTL_UNP_D2_EN, 4, DIPCTL_D1B_UNP_D2_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_UNP_D3_EN, top.unp3_en, UNP3_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN1, DIPCTL_UNP_D3_EN, 4, DIPCTL_D1B_UNP_D3_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_UNP_D4_EN, top.unp4_en, UNP4_EN,, true, DIPCTL_D1A_DIPCTL_RGB_EN2, DIPCTL_UNP_D4_EN, 4, DIPCTL_D1B_UNP_D4_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_UNP_D6_EN, top.unp6_en, UNP6_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN2, DIPCTL_UNP_D6_EN, 4, DIPCTL_D1B_UNP_D6_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_UNP_D7_EN, top.unp7_en, UNP7_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN2, DIPCTL_UNP_D7_EN, 4, DIPCTL_D1B_UNP_D7_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_UNP_D8_EN, top.unp8_en, UNP8_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN2, DIPCTL_UNP_D8_EN, 4, DIPCTL_D1B_UNP_D8_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_UNP_D9_EN, top.unp9_en, UNP9_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN2, DIPCTL_UNP_D9_EN, 4, DIPCTL_D1B_UNP_D9_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_UNP_D10_EN, top.unp10_en, UNP10_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN2, DIPCTL_UNP_D10_EN, 4, DIPCTL_D1B_UNP_D10_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_UNP_D11_EN, top.unp11_en, UNP11_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN2, DIPCTL_UNP_D11_EN, 4, DIPCTL_D1B_UNP_D11_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_UFD_D1_EN, top.ufd_en, UFD_EN,, true, DIPCTL_D1A_DIPCTL_RGB_EN1, DIPCTL_UFD_D1_EN, 4, DIPCTL_D1B_UFD_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_OBC_D1_EN, top.obc_en, OBC_EN,, true, DIPCTL_D1A_DIPCTL_RGB_EN1, DIPCTL_OBC_D1_EN, 4, DIPCTL_D1B_OBC_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_BPC_D1_EN, top.bpc_en, BPC_EN,, true, DIPCTL_D1A_DIPCTL_RGB_EN1, DIPCTL_BPC_D1_EN, 4, DIPCTL_D1B_BPC_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_ZFUS_D1_EN, top.zfus_en, ZFUS_EN,, true, DIPCTL_D1A_DIPCTL_RGB_EN1, DIPCTL_ZFUS_D1_EN, 4, DIPCTL_D1B_ZFUS_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_DGN_D1_EN, top.dgn_en, DGN_EN,, true, DIPCTL_D1A_DIPCTL_RGB_EN1, DIPCTL_DGN_D1_EN, 4, DIPCTL_D1B_DGN_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_LSC_D1_EN, top.lsc_en, LSC_EN,, true, DIPCTL_D1A_DIPCTL_RGB_EN1, DIPCTL_LSC_D1_EN, 4, DIPCTL_D1B_LSC_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_WB_D1_EN, top.wb_en, WB_EN,, true, DIPCTL_D1A_DIPCTL_RGB_EN1, DIPCTL_WB_D1_EN, 4, DIPCTL_D1B_WB_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_HLR_D1_EN, top.hlr_en, HLR_EN,, true, DIPCTL_D1A_DIPCTL_RGB_EN1, DIPCTL_HLR_D1_EN, 4, DIPCTL_D1B_HLR_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_LTM_D1_EN, top.ltm_en, LTM_EN,, true, DIPCTL_D1A_DIPCTL_RGB_EN1, DIPCTL_LTM_D1_EN, 4, DIPCTL_D1B_LTM_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_DM_D1_EN, top.dm_en, DM_EN,, true, DIPCTL_D1A_DIPCTL_RGB_EN1, DIPCTL_UDM_D1_EN, 4, DIPCTL_D1B_DM_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_GDR_D1_EN, top.gdr_en, GDR_EN,, true, DIPCTL_D1A_DIPCTL_RGB_EN1, DIPCTL_GDR_D1_EN, 4, DIPCTL_D1B_GDR_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_ALIGN_D1_EN, top.align_en, ALIGN_EN,, true, DIPCTL_D1A_DIPCTL_RGB_EN1, DIPCTL_ALIGN_D1_EN, 4, DIPCTL_D1B_ALIGN_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_LDNR_D1_EN, top.ldnr_en, LDNR_EN,, true, DIPCTL_D1A_DIPCTL_RGB_EN1, DIPCTL_LDNR_D1_EN, 4, DIPCTL_D1B_LDNR_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_FLC_D1_EN, top.flc_en, FLC_EN,, true, DIPCTL_D1A_DIPCTL_RGB_EN2, DIPCTL_FLC_D1_EN, 4, DIPCTL_D1B_FLC_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_CCM_D1_EN, top.ccm_en, CCM_EN,, true, DIPCTL_D1A_DIPCTL_RGB_EN2, DIPCTL_CCM_D1_EN, 4, DIPCTL_D1B_CCM_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_CCM_D2_EN, top.ccm2_en, CCM2_EN,, true, DIPCTL_D1A_DIPCTL_RGB_EN2, DIPCTL_CCM_D2_EN, 4, DIPCTL_D1B_CCM_D2_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_GGM_D1_EN, top.ggm_en, GGM_EN,, true, DIPCTL_D1A_DIPCTL_RGB_EN2, DIPCTL_GGM1_D1_EN, 4, DIPCTL_D1B_GGM_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_GGM_D2_EN, top.ggm2_en, GGM2_EN,, true, DIPCTL_D1A_DIPCTL_RGB_EN2, DIPCTL_GGM_D2_EN, 4, DIPCTL_D1B_GGM_D2_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_GGM_D4_EN, top.ggm4_en, GGM4_EN,, true, DIPCTL_D1A_DIPCTL_RGB_EN2, DIPCTL_GGM_D4_EN, 4, DIPCTL_D1B_GGM_D4_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_G2C_D4_EN, top.g2c4_en, G2C4_EN,, true, DIPCTL_D1A_DIPCTL_RGB_EN2, DIPCTL_G2C_D4_EN, 4, DIPCTL_D1B_G2C_D4_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_C42_D4_EN, top.c42d4_en, C42D4_EN,, true, DIPCTL_D1A_DIPCTL_RGB_EN2, DIPCTL_C42_D4_EN, 4, DIPCTL_D1B_C42_D4_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_CRP_D4_EN, top.crp4_en, CRP4_EN,, true, DIPCTL_D1A_DIPCTL_RGB_EN2, DIPCTL_CRP_D4_EN, 4, DIPCTL_D1B_CRP_D4_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_WSYNC_D1_EN, top.wsync_en, WSYNC_EN,, true, DIPCTL_D1A_DIPCTL_RGB_EN2, DIPCTL_WSYNC_D1_EN, 4, DIPCTL_D1B_WSYNC_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_G2CX_D1_EN, top.g2cx_en, G2CX_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN1, DIPCTL_G2C_D2_EN, 4, DIPCTL_D1B_G2CX_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_C42_D1_EN, top.c42_en, C42_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN1, DIPCTL_C42_D1_EN, 4, DIPCTL_D1B_C42_D1_EN)\
	CMD(a, b, c, d, e, int, DIPCTL_D1A_SLK_D1_EN, top.slk1_en, SLK1_EN,, true, DIPCTL_D1A_DIPCTL_RGB_EN1, DIPCTL_SLK_D1_EN, 4, DIPCTL_D1B_SLK_D1_EN)\
	CMD(a, b, c, d, e, int, DIPCTL_D1A_SLK_D2_EN, top.slk2_en, SLK2_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN2, DIPCTL_SLK_D2_EN, 4, DIPCTL_D1B_SLK_D2_EN)\
	CMD(a, b, c, d, e, int, DIPCTL_D1A_SLK_D3_EN, top.slk3_en, SLK3_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN2, DIPCTL_SLK_D3_EN, 4, DIPCTL_D1B_SLK_D3_EN)\
	CMD(a, b, c, d, e, int, DIPCTL_D1A_SLK_D4_EN, top.slk4_en, SLK4_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN2, DIPCTL_SLK_D4_EN, 4, DIPCTL_D1B_SLK_D4_EN)\
	CMD(a, b, c, d, e, int, DIPCTL_D1A_SLK_D5_EN, top.slk5_en, SLK5_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN2, DIPCTL_SLK_D5_EN, 4, DIPCTL_D1B_SLK_D5_EN)\
	CMD(a, b, c, d, e, int, DIPCTL_D1A_SLK_D6_EN, top.slk6_en, SLK6_EN,, true, DIPCTL_D1A_DIPCTL_RGB_EN1, DIPCTL_SLK_D6_EN, 4, DIPCTL_D1B_SLK_D6_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_NDG_D1_EN, top.ndg_en, NDG_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN1, DIPCTL_NDG_D1_EN, 4, DIPCTL_D1B_NDG_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_NDG_D2_EN, top.ndg2_en, NDG2_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN1, DIPCTL_NDG_D2_EN, 4, DIPCTL_D1B_NDG_D2_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_YNR_D1_EN, top.ynr_en, YNR_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN1, DIPCTL_NBC_D1_EN, 4, DIPCTL_D1B_YNR_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_C24_D1_EN, top.c24_en, C24_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN2, DIPCTL_C24_D1_EN, 4, DIPCTL_D1B_C24_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_C24_D2_EN, top.c24d2_en, C24D2_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN2, DIPCTL_C24_D2_EN, 4, DIPCTL_D1B_C24_D2_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_C24_D3_EN, top.c24d3_en, C24D3_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN1, DIPCTL_C24_D3_EN, 4, DIPCTL_D1B_C24_D3_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_C2G_D1_EN, top.c2g_en, C2G_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN1, DIPCTL_C2G_D1_EN, 4, DIPCTL_D1B_C2G_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_IGGM_D1_EN, top.iggm_en, IGGM_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN1, DIPCTL_IGGM1_D1_EN, 4, DIPCTL_D1B_IGGM_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_CCM_D3_EN, top.ccm3_en, CCM3_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN1, DIPCTL_IGGM1_D1_EN, 4, DIPCTL_D1B_CCM_D3_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_LCE_D1_EN, top.lce_en, LCE_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN1, DIPCTL_LCE_D1_EN, 4, DIPCTL_D1B_LCE_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_GGM_D3_EN, top.ggm3_en, GGM3_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN1, DIPCTL_GGM_D3_EN, 4, DIPCTL_D1B_GGM_D3_EN)\
	CMD(a, b, c, d, e, int, DIPCTL_D1A_DCE_D1_EN, top.dce_en, DCE_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN1, DIPCTL_DGGM_D1_EN, 4, DIPCTL_D1B_DCE_D1_EN)\
	CMD(a, b, c, d, e, int, DIPCTL_D1A_DCES_D1_EN, top.dces_en, DCES_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN1, DIPCTL_DGGMS_D1_EN, 4, DIPCTL_D1B_DCES_D1_EN)\
	CMD(a, b, c, d, e, int, DIPCTL_D1A_DCESO_D1_EN, top.dceso_en, DCESO_EN,, true, DIPCTL_D1A_DIPCTL_DMA_EN1, DIPCTL_DGGMSO_D1_EN, 4, DIPCTL_D1B_DCESO_D1_EN)\
	CMD(a, b, c, d, e, int, DIPCTL_D1A_G2C_D1_EN, top.g2c_en, G2C_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN1, DIPCTL_G2C_D1_EN, 4, DIPCTL_D1B_G2C_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_C42_D2_EN, top.c42d2_en, C42D2_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN1, DIPCTL_C42_D2_EN, 4, DIPCTL_D1B_C42_D2_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_EE_D1_EN, top.ee_en, EE_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN1, DIPCTL_EE_D1_EN, 4, DIPCTL_D1B_EE_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_CNR_D1_EN, top.cnr_en, CNR_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN1, DIPCTL_ABF_D1_EN, 4, DIPCTL_D1B_CNR_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_COLOR_D1_EN, top.color_en, COLOR_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN1, DIPCTL_COLOR_D1_EN, 4, DIPCTL_D1B_COLOR_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_NR3D_D1_EN, top.nr3d_en, NR3D_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN1, DIPCTL_COLOR_D1_EN, 4, DIPCTL_D1B_NR3D_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_CRP_D1_EN, top.rcp_en, CRP_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN1, DIPCTL_CRP_D1_EN, 4, DIPCTL_D1B_CRP_D1_EN)\
	CMD(a, b, c, d, e, int, DIPCTL_D1A_CRP_D2_EN, top.rcp2_en, CRP2_EN,, true, DIPCTL_D1A_DIPCTL_RGB_EN1, DIPCTL_CRP_D2_EN, 4, DIPCTL_D1B_CRP_D2_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_PAK_D1_EN, top.pak_en, PAK_EN,, true, DIPCTL_D1A_DIPCTL_RGB_EN1, DIPCTL_PAK_D1_EN, 4, DIPCTL_D1B_PAK_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_PAK_D6_EN, top.pak6_en, PAK6_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN2, DIPCTL_PAK_D6_EN, 4, DIPCTL_D1A_PAK_D6_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_PAK_D7_EN, top.pak7_en, PAK7_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN2, DIPCTL_PAK_D7_EN, 4, DIPCTL_D1A_PAK_D7_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_PAK_D8_EN, top.pak8_en, PAK8_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN2, DIPCTL_PAK_D8_EN, 4, DIPCTL_D1A_PAK_D8_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_TIMGO_D1_EN, top.timgo_en, TIMGO_EN,, true, DIPCTL_D1A_DIPCTL_DMA_EN1, DIPCTL_TIMGO_D1_EN, 4, DIPCTL_D1B_TIMGO_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_IMG3O_D1_EN, top.img3o_en, IMG3O_EN,, true, DIPCTL_D1A_DIPCTL_DMA_EN2, DIPCTL_IMG3O_D1_EN, 4, DIPCTL_D1B_IMG3O_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_IMG3BO_D1_EN, top.img3bo_en, IMG3BO_EN,, true, DIPCTL_D1A_DIPCTL_DMA_EN2, DIPCTL_IMG3BO_D1_EN, 4, DIPCTL_D1B_IMG3BO_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_IMG3CO_D1_EN, top.img3co_en, IMG3CO_EN,, true, DIPCTL_D1A_DIPCTL_DMA_EN2, DIPCTL_IMG3CO_D1_EN, 4, DIPCTL_D1B_IMG3CO_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_MCRP_D1_EN, top.mcrp_en, MCRP_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN2, DIPCTL_MCRP_D1_EN, 4, DIPCTL_D1B_MCRP_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_MCRP_D2_EN, top.mcrp2_en, MCRP2_EN,, true, DIPCTL_D1A_DIPCTL_RGB_EN2, DIPCTL_MCRP_D2_EN, 4, DIPCTL_D1B_MCRP_D2_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_MIX_D1_EN, top.mix1_en, MIX1_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN1, DIPCTL_MIX_D1_EN, 4, DIPCTL_D1B_MIX_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_MIX_D2_EN, top.mix2_en, MIX2_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN1, DIPCTL_MIX_D2_EN, 4, DIPCTL_D1B_MIX_D2_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_MIX_D3_EN, top.mix3_en, MIX3_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN1, DIPCTL_MIX_D3_EN, 4, DIPCTL_D1B_MIX_D3_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_MIX_D4_EN, top.mix4_en, MIX4_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN1, DIPCTL_MIX_D4_EN, 4, DIPCTL_D1B_MIX_D4_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_CRZ_D1_EN, top.crz_en, CRZ_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN2, DIPCTL_CRZ_D1_EN, 4, DIPCTL_D1B_CRZ_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_CRZO_D1_EN, top.crzo_en, CRZO_EN,, true, DIPCTL_D1A_DIPCTL_DMA_EN1, DIPCTL_CRZO_D1_EN, 4, DIPCTL_D1B_CRZO_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_CRZBO_D1_EN, top.crzbo_en, CRZBO_EN,, true, DIPCTL_D1A_DIPCTL_DMA_EN1, DIPCTL_CRZBO_D1_EN, 4, DIPCTL_D1B_CRZBO_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_SRZ_D1_EN, top.srz1_en, SRZ1_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN2, DIPCTL_SRZ_D1_EN, 4, DIPCTL_D1B_SRZ_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_SRZ_D3_EN, top.srz3_en, SRZ3_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN2, DIPCTL_SRZ_D3_EN, 4, DIPCTL_D1B_SRZ_D3_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_SRZ_D4_EN, top.srz4_en, SRZ4_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN2, DIPCTL_SRZ_D4_EN, 4, DIPCTL_D1B_SRZ_D4_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_DFE_D1_EN, top.dfe_en, DFE_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN2, DIPCTL_DFE_D1_EN, 4, DIPCTL_D1B_DFE_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_FEO_D1_EN, top.feo_en, FEO_EN,, true, DIPCTL_D1A_DIPCTL_DMA_EN1, DIPCTL_FEO_D1_EN, 4, DIPCTL_D1B_FEO_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_C02_D1_EN, top.c02_en, C02_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN2, DIPCTL_C02_D1_EN, 4, DIPCTL_D1B_C02_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_C02_D2_EN, top.c02d2_en, C02D2_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN2, DIPCTL_C02_D2_EN, 4, DIPCTL_D1B_C02_D2_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_CRSP_D1_EN, top.crsp_en, CRSP_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN2, DIPCTL_CRSP_D1_EN, 4, DIPCTL_D1B_CRSP_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_PLNR_D1_EN, top.plnr1_en, PLNR1_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN2, DIPCTL_PLNR_D1_EN, 4, DIPCTL_D1B_PLNR_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_PLNR_D2_EN, top.plnr2_en, PLNR2_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN2, DIPCTL_PLNR_D2_EN, 4, DIPCTL_D1B_PLNR_D2_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_PLNW_D1_EN, top.plnw1_en, PLNW1_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN2, DIPCTL_PLNW_D1_EN, 4, DIPCTL_D1B_PLNW_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_PLNW_D2_EN, top.plnw2_en, PLNW2_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN2, DIPCTL_PLNW_D2_EN, 4, DIPCTL_D1B_PLNW_D2_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_ADL_D1_EN, top.adl_en, ADL_EN,, true, DIPCTL_D1A_DIPCTL_DMA_EN1, DIPCTL_ADL_D1_EN, 4, DIPCTL_D1B_ADL_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_BS_D1_EN, top.bs_en, BS_EN,, true, DIPCTL_D1A_DIPCTL_YUV_EN2, DIPCTL_BS_D1_EN, 4, DIPCTL_D1A_BS_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_WIF_D1_EN, top.wif_en, WIF_EN,, true, DIPCTL_D1A_DIPCTL_RGB_EN1, DIPCTL_WIF_D1_EN, 4, DIPCTL_D1B_WIF_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_WIF_D2_EN, top.wif2_en, WIF2_EN,, true, DIPCTL_D1A_DIPCTL_RGB_EN1, DIPCTL_WIF_D2_EN, 4, DIPCTL_D1B_WIF_D2_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_WIF_D3_EN, top.wif3_en, WIF3_EN,, true, DIPCTL_D1A_DIPCTL_RGB_EN1, DIPCTL_WIF_D3_EN, 4, DIPCTL_D1B_WIF_D3_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_OFT_D1_EN, top.oft_en, OFT_EN,, true, MDPS_D1A_OFT_OFT_MODULE_EN, OFT_R2B_EN, 4, DIPCTL_D1B_OFT_D1_EN)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_OFTL_D1_EN, top.oftl_en, OFTL_EN,, true, MDPV_D1A_OFTL_OFT_MODULE_EN, OFTL_R2B_EN, 4, DIPCTL_D1B_OFTL_D1_EN)\
	/* SEL MUX */\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_TRAW_SEL, top.traw_sel, TRAW_SEL,, true, DIPCTL_D1A_DIPCTL_MUX_SEL1,  DIPCTL_TRAW_SEL, 4, DIPCTL_D1B_TRAW_SEL)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_FLC_D1_SEL, top.flc_sel, FLC_SEL,, true, DIPCTL_D1A_DIPCTL_MUX_SEL1, DIPCTL_FLC_D1_SEL, 4, DIPCTL_D1B_FLC_D1_SEL)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_DGN_D1_SEL, top.dgn_sel, DGN_SEL,, true, DIPCTL_D1A_DIPCTL_MUX_SEL1, DIPCTL_DGN_D1_SEL, 4, DIPCTL_D1B_DGN_D1_SEL)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_CRP_D2_SEL, top.crp2_sel, CRP2_SEL,, true, DIPCTL_D1A_DIPCTL_MUX_SEL1, DIPCTL_CRP_D2_SEL, 4, DIPCTL_D1B_CRP_D2_SEL)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_G2CX_D1_SEL, top.g2cx_sel, G2CX_SEL,, true, DIPCTL_D1A_DIPCTL_MUX_SEL1, DIPCTL_G2C_D2_SEL, 4, DIPCTL_D1B_G2CX_D1_SEL)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_YNR_D1_SEL, top.ynr_sel, YNR_SEL,, true, DIPCTL_D1A_DIPCTL_MUX_SEL1, DIPCTL_NBC_D1_SEL, 4, DIPCTL_D1B_YNR_D1_SEL)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_TIMGO_D1_SEL, top.timgo_sel, TIMGO_SEL,, true, DIPCTL_D1A_DIPCTL_MUX_SEL1, DIPCTL_TIMGO_D1_SEL, 4, DIPCTL_D1B_TIMGO_D1_SEL)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_MIX_D1_SEL, top.mix1_sel, MIX1_SEL,, true, DIPCTL_D1A_DIPCTL_MUX_SEL1, DIPCTL_MIX_D1_SEL, 4, DIPCTL_D1B_MIX_D1_SEL)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_C24_D1_SEL, top.c24d1_sel, C24D1_SEL,, true, DIPCTL_D1A_DIPCTL_MUX_SEL2, DIPCTL_C24_D1_SEL, 4, DIPCTL_D1B_C24_D1_SEL)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_CRZ_D1_SEL, top.crz_sel, CRZ_SEL,, true, DIPCTL_D1A_DIPCTL_MUX_SEL2, DIPCTL_CRZ_D1_SEL, 4, DIPCTL_D1B_CRZ_D1_SEL)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_FE_D1_SEL, top.fe_sel, FE_SEL,, true, DIPCTL_D1A_DIPCTL_MUX_SEL2, DIPCTL_FE_D1_SEL, 4, DIPCTL_D1B_FE_D1_SEL)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_SRZ_D1_SEL, top.srz_sel, SRZ1_SEL,, true, DIPCTL_D1A_DIPCTL_MUX_SEL2, DIPCTL_SRZ_D1_SEL, 4, DIPCTL_D1B_SRZ_D1_SEL)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_FEO_D1_SEL, top.feo_sel, FEO_SEL,, true, DIPCTL_D1A_DIPCTL_MUX_SEL2, DIPCTL_FEO_D1_SEL, 4, DIPCTL_D1B_FEO_D1_SEL)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_CRSP_D1_SEL, top.crsp_sel, CRSP_SEL,, true, DIPCTL_D1A_DIPCTL_MUX_SEL1, DIPCTL_CRSP_D1_SEL, 4, DIPCTL_D1B_CRSP_D1_SEL)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_NR3D_PRE_I_D1_SEL, top.nr3d_pre_i_sel, NR3D_PRE_I_SEL,, true, DIPCTL_D1A_DIPCTL_MUX_SEL2, DIPCTL_MIX_D3_W_SEL, 4, DIPCTL_D1B_NR3D_PRE_I_D1_SEL)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_MIX_D3_W_SEL, top.mix3_w_sel, MIX3_W_SEL,, true, DIPCTL_D1A_DIPCTL_MUX_SEL2, DIPCTL_MIX_D3_W_SEL, 4, DIPCTL_D1B_MIX_D3_W_SEL)\
    CMD(a, b, c, d, e, int, SMT_D1A_SMTO_SEL, smt1.smt1o_sel, SMT1O_SEL,, REG_CHECK_EN(c, SMT1_EN), DIPCTL_D1A_DIPCTL_MUX_SEL2, DIPCTL_MIX_D3_W_SEL, 4, SMT_D1B_SMTO_SEL)\
    CMD(a, b, c, d, e, int, SMT_D2A_SMTO_SEL, smt2.smt2o_sel, SMT2O_SEL,, REG_CHECK_EN(c, SMT2_EN), DIPCTL_D1A_DIPCTL_MUX_SEL2, DIPCTL_MIX_D3_W_SEL, 4, SMT_D2B_SMTO_SEL)\
    CMD(a, b, c, d, e, int, SMT_D3A_SMTO_SEL, smt3.smt3o_sel, SMT3O_SEL,, REG_CHECK_EN(c, SMT3_EN), DIPCTL_D1A_DIPCTL_MUX_SEL2, DIPCTL_MIX_D3_W_SEL, 4, SMT_D3B_SMTO_SEL)\
    CMD(a, b, c, d, e, int, SMT_D4A_SMTO_SEL, smt4.smt4o_sel, SMT4O_SEL,, REG_CHECK_EN(c, SMT4_EN), DIPCTL_D1A_DIPCTL_MUX_SEL2, DIPCTL_MIX_D3_W_SEL, 4, SMT_D4B_SMTO_SEL)\
    CMD(a, b, c, d, e, int, DIPCTL_D1A_OFT_D1_SEL, top.oft_sel, OFT_SEL,, true, MDPS_D1A_OFT_OFT_FORMAT, OFT_TOP_IN_DEPTH, 4, DIPCTL_D1B_OFT_D1_SEL)\
    /* IMGI_D1 */\
    CMD(a, b, c, d, e, int, DIPDMATOP_D1A_IMGI_D1_V_FLIP_EN, imgi.imgi_v_flip_en, IMGI_V_FLIP_EN,, REG_CMP_EQ(c, IMGI_EN, 1), DIPDMATOP_D1A_DIPDMATOP_VERTICAL_FLIP_EN, DIPDMATOP_IMGI_D1_V_FLIP_EN, 4, DIPDMATOP_D1B_IMGI_D1_V_FLIP_EN)\
    CMD(a, b, c, d, e, int, IMGI_D1A_STRIDE, imgi.imgi_stride, IMGI_STRIDE,, REG_CMP_EQ(c, IMGI_EN, 1), IMGI_D1A_IMGI_STRIDE, IMGI_STRIDE, 4, IMGI_D1B_STRIDE)\
    /* IMGBI_D1 */\
    CMD(a, b, c, d, e, int, IMGBI_D1A_OFST_ADDR, imgbi.imgbi_offset, IMGBI_OFFSET_ADDR,, REG_CMP_EQ(c, LOG_IMGBI_EN, 1), IMGI_D1A_IMGI_OFST_ADDR, IMGI_OFST_ADDR, 0, IMGBI_D1B_OFST_ADDR)\
    CMD(a, b, c, d, e, int, IMGBI_D1A_XSIZE, imgbi.imgbi_xsize, IMGBI_XSIZE,, REG_CMP_EQ(c, LOG_IMGBI_EN, 1), IMGI_D1A_IMGI_XSIZE, IMGI_XSIZE, 0, IMGBI_D1B_XSIZE)\
    CMD(a, b, c, d, e, int, IMGBI_D1A_YSIZE, imgbi.imgbi_ysize, IMGBI_YSIZE,, REG_CMP_EQ(c, LOG_IMGBI_EN, 1), IMGI_D1A_IMGI_YSIZE, IMGI_YSIZE, 0, IMGBI_D1B_YSIZE)\
    CMD(a, b, c, d, e, int, IMGBI_D1A_STRIDE, imgbi.imgbi_stride, IMGBI_STRIDE,, REG_CMP_EQ(c, IMGBI_EN, 1), IMGI_D1A_IMGI_STRIDE, IMGI_STRIDE, 4, IMGBI_D1B_STRIDE)\
    /* IMGCI_D1 */\
    CMD(a, b, c, d, e, int, DIPDMATOP_D1A_IMGCI_D1_V_FLIP_EN, imgci.imgci_v_flip_en, IMGCI_V_FLIP_EN,, REG_CMP_EQ(c, LOG_IMGBI_EN, 1), DIPDMATOP_D1A_DIPDMATOP_VERTICAL_FLIP_EN, DIPDMATOP_IMGI_D1_V_FLIP_EN, 4, DIPDMATOP_D1B_IMGCI_D1_V_FLIP_EN)\
    CMD(a, b, c, d, e, int, IMGCI_D1A_STRIDE, imgci.imgci_stride, IMGCI_STRIDE,, REG_CMP_EQ(c, LOG_IMGBI_EN, 1), IMGI_D1A_IMGI_STRIDE, IMGI_STRIDE, 4, IMGCI_D1B_STRIDE)\
    /* VIPI_D1 */\
    CMD(a, b, c, d, e, int, DIPDMATOP_D1A_VIPI_D1_V_FLIP_EN, vipi.vipi_v_flip_en, VIPI_V_FLIP_EN,, REG_CMP_EQ(c, LOG_VIPI_EN, 1), DIPDMATOP_D1A_DIPDMATOP_VERTICAL_FLIP_EN, DIPDMATOP_VIPI_D1_V_FLIP_EN, 4, DIPDMATOP_D1B_VIPI_D1_V_FLIP_EN)\
    CMD(a, b, c, d, e, int, VIPI_D1A_XSIZE, vipi.vipi_xsize, VIPI_XSIZE,, REG_CMP_EQ(c, LOG_VIPI_EN, 1), IMGI_D1A_IMGI_XSIZE, IMGI_XSIZE, 0, VIPI_D1B_XSIZE)\
    CMD(a, b, c, d, e, int, VIPI_D1A_YSIZE, vipi.vipi_ysize, VIPI_YSIZE,, REG_CMP_EQ(c, LOG_VIPI_EN, 1), IMGI_D1A_IMGI_YSIZE, IMGI_YSIZE, 0, VIPI_D1B_YSIZE)\
    CMD(a, b, c, d, e, int, VIPI_D1A_STRIDE, vipi.vipi_stride, VIPI_STRIDE,, REG_CMP_EQ(c, VIPI_EN, 1), IMGI_D1A_IMGI_STRIDE, IMGI_STRIDE, 4, VIPI_D1B_STRIDE)\
    /* VIPBI_D1 */\
    CMD(a, b, c, d, e, int, DIPDMATOP_D1A_VIPBI_D1_V_FLIP_EN, vipbi.vipbi_v_flip_en, VIPBI_V_FLIP_EN,, REG_CMP_EQ(c, LOG_VIPBI_EN, 1), DIPDMATOP_D1A_DIPDMATOP_VERTICAL_FLIP_EN, DIPDMATOP_VIPBI_D1_V_FLIP_EN, 4, DIPDMATOP_D1B_VIPBI_D1_V_FLIP_EN)\
    CMD(a, b, c, d, e, int, VIPBI_D1A_XSIZE, vipbi.vipbi_xsize, VIPBI_XSIZE,, REG_CMP_EQ(c, LOG_VIPBI_EN, 1), IMGI_D1A_IMGI_XSIZE, IMGI_XSIZE, 0, VIPBI_D1B_XSIZE)\
    CMD(a, b, c, d, e, int, VIPBI_D1A_YSIZE, vipbi.vipbi_ysize, VIPBI_YSIZE,, REG_CMP_EQ(c, LOG_VIPBI_EN, 1), IMGI_D1A_IMGI_YSIZE, IMGI_YSIZE, 0, VIPBI_D1B_YSIZE)\
    CMD(a, b, c, d, e, int, VIPBI_D1A_STRIDE, vipbi.vipbi_stride, VIPBI_STRIDE,, REG_CMP_EQ(c, VIPBI_EN, 1), IMGI_D1A_IMGI_STRIDE, IMGI_STRIDE, 4, VIPBI_D1B_STRIDE)\
    /* VIPCI_D1 */\
    CMD(a, b, c, d, e, int, DIPDMATOP_D1A_VIPCI_D1_V_FLIP_EN, vipci.vipci_v_flip_en, VIPCI_V_FLIP_EN,, REG_CMP_EQ(c, LOG_VIPCI_EN, 1), DIPDMATOP_D1A_DIPDMATOP_VERTICAL_FLIP_EN, DIPDMATOP_VIPCI_D1_V_FLIP_EN, 4, DIPDMATOP_D1B_VIPCI_D1_V_FLIP_EN)\
    CMD(a, b, c, d, e, int, VIPCI_D1A_XSIZE, vipci.vipci_xsize, VIPCI_XSIZE,, REG_CMP_EQ(c, LOG_VIPI_EN, 1), IMGI_D1A_IMGI_XSIZE, IMGI_XSIZE, 0, VIPCI_D1B_XSIZE)\
    CMD(a, b, c, d, e, int, VIPCI_D1A_YSIZE, vipci.vipci_ysize, VIPCI_YSIZE,, REG_CMP_EQ(c, LOG_VIPI_EN, 1), IMGI_D1A_IMGI_YSIZE, IMGI_YSIZE, 0, VIPCI_D1B_YSIZE)\
    CMD(a, b, c, d, e, int, VIPCI_D1A_STRIDE, vipci.vipci_stride, VIPCI_STRIDE,, REG_CMP_EQ(c, VIPI_EN, 1), IMGI_D1A_IMGI_STRIDE, IMGI_STRIDE, 4, VIPCI_D1B_STRIDE)\
    /* UFDI_D1 */\
    CMD(a, b, c, d, e, int, DIPDMATOP_D1A_UFDI_D1_V_FLIP_EN, ufdi.ufdi_v_flip_en, UFDI_V_FLIP_EN,, REG_CMP_EQ(c, LOG_UFDI_EN, 1), DIPDMATOP_D1A_DIPDMATOP_VERTICAL_FLIP_EN, DIPDMATOP_VIPCI_D1_V_FLIP_EN, 4, DIPDMATOP_D1B_UFDI_D1_V_FLIP_EN)\
    CMD(a, b, c, d, e, int, UFDI_D1A_XSIZE, ufdi.ufdi_xsize, UFDI_XSIZE,, REG_CMP_EQ(c, LOG_UFDI_EN, 1), IMGI_D1A_IMGI_XSIZE, IMGI_XSIZE, 0, UFDI_D1B_XSIZE)\
    CMD(a, b, c, d, e, int, UFDI_D1A_YSIZE, ufdi.ufdi_ysize, UFDI_YSIZE,, REG_CMP_EQ(c, LOG_UFDI_EN, 1), IMGI_D1A_IMGI_YSIZE, IMGI_YSIZE, 0, UFDI_D1B_YSIZE)\
    CMD(a, b, c, d, e, int, UFDI_D1A_STRIDE, ufdi.ufdi_stride, UFDI_STRIDE,, REG_CMP_EQ(c, LOG_UFDI_EN, 1), IMGI_D1A_IMGI_STRIDE, IMGI_STRIDE, 4, UFDI_D1B_STRIDE)\
    /* DMGI_D1 */\
    CMD(a, b, c, d, e, int, DIPDMATOP_D1A_DMGI_D1_V_FLIP_EN, dmgi.dmgi_v_flip_en, DMGI_V_FLIP_EN,, REG_CMP_EQ(c, LOG_DMGI_EN, 1), DIPDMATOP_D1A_DIPDMATOP_VERTICAL_FLIP_EN, DIPDMATOP_DMGI_D1_V_FLIP_EN, 4, DIPDMATOP_D1B_DMGI_D1_V_FLIP_EN)\
    CMD(a, b, c, d, e, int, DMGI_D1A_XSIZE, dmgi.dmgi_xsize, DMGI_XSIZE,, REG_CMP_EQ(c, LOG_DMGI_EN, 1), DMGI_D1A_DMGI_XSIZE, DMGI_XSIZE, 0, DMGI_D1B_XSIZE)\
    CMD(a, b, c, d, e, int, DMGI_D1A_YSIZE, dmgi.dmgi_ysize, DMGI_YSIZE,, REG_CMP_EQ(c, LOG_DMGI_EN, 1), DMGI_D1A_DMGI_YSIZE, DMGI_YSIZE, 0, DMGI_D1B_YSIZE)\
    CMD(a, b, c, d, e, int, DMGI_D1A_STRIDE, dmgi.dmgi_stride, DMGI_STRIDE,, REG_CMP_EQ(c, LOG_DMGI_EN, 1), DMGI_D1A_DMGI_STRIDE, DMGI_STRIDE, 4, DMGI_D1B_STRIDE)\
    /* DEPI_D1 */\
    CMD(a, b, c, d, e, int, DIPDMATOP_D1A_DEPI_D1_V_FLIP_EN, depi.depi_v_flip_en, DEPI_V_FLIP_EN,, REG_CMP_EQ(c, LOG_DEPI_EN, 1), DIPDMATOP_D1A_DIPDMATOP_VERTICAL_FLIP_EN, DIPDMATOP_DEPI_D1_V_FLIP_EN, 4, DIPDMATOP_D1B_DEPI_D1_V_FLIP_EN)\
    CMD(a, b, c, d, e, int, DEPI_D1A_XSIZE, depi.depi_xsize, DEPI_XSIZE,, REG_CMP_EQ(c, LOG_DEPI_EN, 1), DEPI_D1A_DEPI_XSIZE, DEPI_XSIZE, 1, DEPI_D1B_XSIZE)\
    CMD(a, b, c, d, e, int, DEPI_D1A_YSIZE, depi.depi_ysize, DEPI_YSIZE,, REG_CMP_EQ(c, LOG_DEPI_EN, 1), DEPI_D1A_DEPI_YSIZE, DEPI_YSIZE,1, DEPI_D1B_YSIZE)\
    CMD(a, b, c, d, e, int, DEPI_D1A_STRIDE, depi.depi_stride, DEPI_STRIDE,, REG_CMP_EQ(c, LOG_DEPI_EN, 1), DEPI_D1A_DEPI_STRIDE, DEPI_STRIDE, 4, DEPI_D1B_STRIDE)\
    /* LCEI_D1 */\
    CMD(a, b, c, d, e, int, DIPDMATOP_D1A_LCEI_D1_V_FLIP_EN, lcei.lcei_v_flip_en, LCEI_V_FLIP_EN,, REG_CMP_EQ(c, LOG_LCEI_EN, 1), DIPDMATOP_D1A_DIPDMATOP_VERTICAL_FLIP_EN, DIPDMATOP_LCEI_D1_V_FLIP_EN, 4, DIPDMATOP_D1B_LCEI_D1_V_FLIP_EN)\
    CMD(a, b, c, d, e, int, LCEI_D1A_XSIZE, lcei.lcei_xsize, LCEI_XSIZE,, REG_CMP_EQ(c, LOG_LCEI_EN, 1), LCEI_D1A_LCEI_XSIZE, LCEI_XSIZE, 0, LCEI_D1B_XSIZE)\
    CMD(a, b, c, d, e, int, LCEI_D1A_YSIZE, lcei.lcei_ysize, LCEI_YSIZE,, REG_CMP_EQ(c, LOG_LCEI_EN, 1), LCEI_D1A_LCEI_YSIZE, LCEI_YSIZE, 0, LCEI_D1B_YSIZE)\
    CMD(a, b, c, d, e, int, LCEI_D1A_STRIDE, lcei.lcei_stride, LCEI_STRIDE,, REG_CMP_EQ(c, LOG_LCEI_EN, 1), LCEI_D1A_LCEI_STRIDE, LCEI_STRIDE, 4, LCEI_D1B_STRIDE)\
    /* LCE_D1 */\
    CMD(a, b, c, d, e, int, LCE_D1A_LC_TONE, lce.lce_lc_tone, LCE_LC_TONE,, REG_CMP_EQ(c, LOG_LCE_EN, 1), LCE_D1A_LCE_GLOBAL, LCE_LC_TONE, 4, LCE_D1B_LC_TONE)\
    CMD(a, b, c, d, e, int, LCE_D1A_SLM_WD, lce.lce_slm_width, LCE_SLM_WIDTH,, REG_CMP_EQ(c, LOG_LCE_EN, 1), LCE_D1A_LCE_SLM_SIZE, LCE_SLM_WD, 0, LCE_D1B_SLM_WD)\
    CMD(a, b, c, d, e, int, LCE_D1A_SLM_HT, lce.lce_slm_height, LCE_SLM_HEIGHT,, REG_CMP_EQ(c, LOG_LCE_EN, 1), LCE_D1A_LCE_SLM_SIZE, LCE_SLM_HT, 0, LCE_D1B_SLM_HT)\
    CMD(a, b, c, d, e, int, LCE_D1A_BCMK_X, lce.lce_bc_mag_kubnx, LCE_BCMK_X,, REG_CMP_EQ(c, LOG_LCE_EN, 1), LCE_D1A_LCE_ZR, LCE_BCMK_X, 4, LCE_D1B_BCMK_X)\
    CMD(a, b, c, d, e, int, LCE_D1A_BCMK_Y, lce.lce_bc_mag_kubny, LCE_BCMK_Y,, REG_CMP_EQ(c, LOG_LCE_EN, 1), LCE_D1A_LCE_ZR, LCE_BCMK_Y, 4, LCE_D1B_BCMK_Y)\
	/* UNP_D2 */\
	CMD(a, b, c, d, e, int, UNP_D2A_YUV_BIT, unp2.yuv_bit, UNP2_YUV_BIT,, REG_CMP_EQ(c, LOG_UNP2_EN, 1), UNP_D1A_UNP_CONT, UNP_YUV_BIT, 4, UNP_D2B_YUV_BIT)\
	CMD(a, b, c, d, e, int, UNP_D2A_YUV_DNG, unp2.yuv_dng, UNP2_YUV_DNG,, REG_CMP_EQ(c, LOG_UNP2_EN, 1), UNP_D1A_UNP_CONT, UNP_YUV_DNG, 4, UNP_D2B_YUV_DNG)\
	/* UNP_D3 */\
	CMD(a, b, c, d, e, int, UNP_D3A_YUV_BIT, unp3.yuv_bit, UNP3_YUV_BIT,, REG_CMP_EQ(c, LOG_UNP3_EN, 1), UNP_D1A_UNP_CONT, UNP_YUV_BIT, 4, UNP_D3B_YUV_BIT)\
	CMD(a, b, c, d, e, int, UNP_D3A_YUV_DNG, unp3.yuv_dng, UNP3_YUV_DNG,, REG_CMP_EQ(c, LOG_UNP3_EN, 1), UNP_D1A_UNP_CONT, UNP_YUV_DNG, 4, UNP_D3B_YUV_DNG)\
	/* UNP_D4 */\
	CMD(a, b, c, d, e, int, UNP_D4A_YUV_BIT, unp4.yuv_bit, UNP4_YUV_BIT,, REG_CMP_EQ(c, LOG_UNP4_EN, 1), UNP_D1A_UNP_CONT, UNP_YUV_BIT, 4, UNP_D4B_YUV_BIT)\
	CMD(a, b, c, d, e, int, UNP_D4A_YUV_DNG, unp4.yuv_dng, UNP4_YUV_DNG,, REG_CMP_EQ(c, LOG_UNP4_EN, 1), UNP_D1A_UNP_CONT, UNP_YUV_DNG, 4, UNP_D4B_YUV_DNG)\
	/* UNP_D6 */\
	CMD(a, b, c, d, e, int, UNP_D6A_YUV_BIT, unp6.yuv_bit, UNP6_YUV_BIT,, REG_CMP_EQ(c, LOG_UNP6_EN, 1), UNP_D1A_UNP_CONT, UNP_YUV_BIT, 4, UNP_D6B_YUV_BIT)\
	CMD(a, b, c, d, e, int, UNP_D6A_YUV_DNG, unp6.yuv_dng, UNP6_YUV_DNG,, REG_CMP_EQ(c, LOG_UNP6_EN, 1), UNP_D1A_UNP_CONT, UNP_YUV_DNG, 4, UNP_D6B_YUV_DNG)\
	/* UNP_D7 */\
	CMD(a, b, c, d, e, int, UNP_D7A_YUV_BIT, unp7.yuv_bit, UNP7_YUV_BIT,, REG_CMP_EQ(c, LOG_UNP7_EN, 1), UNP_D1A_UNP_CONT, UNP_YUV_BIT, 4, UNP_D7B_YUV_BIT)\
	CMD(a, b, c, d, e, int, UNP_D7A_YUV_DNG, unp7.yuv_dng, UNP7_YUV_DNG,, REG_CMP_EQ(c, LOG_UNP7_EN, 1), UNP_D1A_UNP_CONT, UNP_YUV_DNG, 4, UNP_D7B_YUV_DNG)\
	/* UNP_D8 */\
	CMD(a, b, c, d, e, int, UNP_D8A_YUV_BIT, unp8.yuv_bit, UNP8_YUV_BIT,, REG_CMP_EQ(c, LOG_UNP8_EN, 1), UNP_D1A_UNP_CONT, UNP_YUV_BIT, 4, UNP_D8B_YUV_BIT)\
	CMD(a, b, c, d, e, int, UNP_D8A_YUV_DNG, unp8.yuv_dng, UNP8_YUV_DNG,, REG_CMP_EQ(c, LOG_UNP8_EN, 1), UNP_D1A_UNP_CONT, UNP_YUV_DNG, 4, UNP_D8B_YUV_DNG)\
	/* UNP_D9 */\
	CMD(a, b, c, d, e, int, UNP_D9A_YUV_BIT, unp9.yuv_bit, UNP9_YUV_BIT,, REG_CMP_EQ(c, LOG_UNP9_EN, 1), UNP_D1A_UNP_CONT, UNP_YUV_BIT, 4, UNP_D9B_YUV_BIT)\
	CMD(a, b, c, d, e, int, UNP_D9A_YUV_DNG, unp9.yuv_dng, UNP9_YUV_DNG,, REG_CMP_EQ(c, LOG_UNP9_EN, 1), UNP_D1A_UNP_CONT, UNP_YUV_DNG, 4, UNP_D9B_YUV_DNG)\
	/* UNP_D10 */\
	CMD(a, b, c, d, e, int, UNP_D10A_YUV_BIT, unp10.yuv_bit, UNP10_YUV_BIT,, REG_CMP_EQ(c, LOG_UNP10_EN, 1), UNP_D1A_UNP_CONT, UNP_YUV_BIT, 4, UNP_D10B_YUV_BIT)\
	CMD(a, b, c, d, e, int, UNP_D10A_YUV_DNG, unp10.yuv_dng, UNP10_YUV_DNG,, REG_CMP_EQ(c, LOG_UNP10_EN, 1), UNP_D1A_UNP_CONT, UNP_YUV_DNG, 4, UNP_D10B_YUV_DNG)\
	/* UNP_D11 */\
	CMD(a, b, c, d, e, int, UNP_D11A_YUV_BIT, unp11.yuv_bit, UNP11_YUV_BIT,, REG_CMP_EQ(c, LOG_UNP11_EN, 1), UNP_D1A_UNP_CONT, UNP_YUV_BIT, 4, UNP_D11B_YUV_BIT)\
	CMD(a, b, c, d, e, int, UNP_D11A_YUV_DNG, unp11.yuv_dng, UNP11_YUV_DNG,, REG_CMP_EQ(c, LOG_UNP11_EN, 1), UNP_D1A_UNP_CONT, UNP_YUV_DNG, 4, UNP_D11B_YUV_DNG)\
	/* PAK_D2 */\
	CMD(a, b, c, d, e, int, PAK_D2A_YUV_BIT, pak2.yuv_bit, PAK2_YUV_BIT,, REG_CMP_EQ(c, LOG_PAK2_EN, 1), PAK_D1A_PAK_CONT, PAK_YUV_BIT, 4, PAK_D2B_YUV_BIT)\
	CMD(a, b, c, d, e, int, PAK_D2A_YUV_DNG, pak2.yuv_dng, PAK2_YUV_DNG,, REG_CMP_EQ(c, LOG_PAK2_EN, 1), PAK_D1A_PAK_CONT, PAK_YUV_DNG, 4, PAK_D2B_YUV_DNG)\
	/* PAK_D3 */\
	CMD(a, b, c, d, e, int, PAK_D3A_YUV_BIT, pak3.yuv_bit, PAK3_YUV_BIT,, REG_CMP_EQ(c, LOG_PAK3_EN, 1), PAK_D1A_PAK_CONT, PAK_YUV_BIT, 4, PAK_D3B_YUV_BIT)\
	CMD(a, b, c, d, e, int, PAK_D3A_YUV_DNG, pak3.yuv_dng, PAK3_YUV_DNG,, REG_CMP_EQ(c, LOG_PAK3_EN, 1), PAK_D1A_PAK_CONT, PAK_YUV_DNG, 4, PAK_D3B_YUV_DNG)\
	/* PAK_D4 */\
	CMD(a, b, c, d, e, int, PAK_D4A_YUV_BIT, pak4.yuv_bit, PAK4_YUV_BIT,, REG_CMP_EQ(c, LOG_PAK4_EN, 1), PAK_D1A_PAK_CONT, PAK_YUV_BIT, 4, PAK_D4B_YUV_BIT)\
	CMD(a, b, c, d, e, int, PAK_D4A_YUV_DNG, pak4.yuv_dng, PAK4_YUV_DNG,, REG_CMP_EQ(c, LOG_PAK4_EN, 1), PAK_D1A_PAK_CONT, PAK_YUV_DNG, 4, PAK_D4B_YUV_DNG)\
	/* PAK_D6 */\
	CMD(a, b, c, d, e, int, PAK_D6A_YUV_BIT, pak6.yuv_bit, PAK6_YUV_BIT,, REG_CMP_EQ(c, LOG_PAK6_EN, 1), PAK_D6A_PAK_CONT, PAK_YUV_BIT, 4, PAK_D6B_YUV_BIT)\
	CMD(a, b, c, d, e, int, PAK_D6A_YUV_DNG, pak6.yuv_dng, PAK6_YUV_DNG,, REG_CMP_EQ(c, LOG_PAK6_EN, 1), PAK_D6A_PAK_CONT, PAK_YUV_DNG, 4, PAK_D6B_YUV_DNG)\
	/* PAK_D7 */\
	CMD(a, b, c, d, e, int, PAK_D7A_YUV_BIT, pak7.yuv_bit, PAK7_YUV_BIT,, REG_CMP_EQ(c, LOG_PAK7_EN, 1), PAK_D7A_PAK_CONT, PAK_YUV_BIT, 4, PAK_D7B_YUV_BIT)\
	CMD(a, b, c, d, e, int, PAK_D7A_YUV_DNG, pak7.yuv_dng, PAK7_YUV_DNG,, REG_CMP_EQ(c, LOG_PAK7_EN, 1), PAK_D7A_PAK_CONT, PAK_YUV_DNG, 4, PAK_D7B_YUV_DNG)\
	/* PAK_D8 */\
	CMD(a, b, c, d, e, int, PAK_D8A_YUV_BIT, pak8.yuv_bit, PAK8_YUV_BIT,, REG_CMP_EQ(c, LOG_PAK8_EN, 1), PAK_D8A_PAK_CONT, PAK_YUV_BIT, 4, PAK_D8B_YUV_BIT)\
	CMD(a, b, c, d, e, int, PAK_D8A_YUV_DNG, pak8.yuv_dng, PAK8_YUV_DNG,, REG_CMP_EQ(c, LOG_PAK8_EN, 1), PAK_D8A_PAK_CONT, PAK_YUV_DNG, 4, PAK_D8B_YUV_DNG)\
	/* DM_D1 */\
	CMD(a, b, c, d, e, int, DM_D1A_BYP, dm.dm_byp, DM_BYP,, REG_CMP_EQ(c, LOG_DM_EN, 1), UDM_D1A_DM_INTP_CRS, DM_BYP, 4, DM_D1B_BYP)\
	/* YNR_D1 */\
	CMD(a, b, c, d, e, int, YNR_D1A_ENY, ynr.eny, YNR_ENY,, REG_CMP_EQ(c, LOG_YNR_EN, 1), NBC_D1A_YNR_CON1, YNR_ENY, 4, YNR_D1B_ENY)\
    CMD(a, b, c, d, e, int, YNR_D1A_ENC, ynr.enc, YNR_ENC,, REG_CMP_EQ(c, LOG_YNR_EN, 1), NBC_D1A_YNR_CON1, YNR_ENC, 4, YNR_D1B_ENC)\
	CMD(a, b, c, d, e, int, YNR_D1A_VIDEO_MODE, ynr.video_mode, YNR_VIDEO_MODE,, REG_CMP_EQ(c, LOG_YNR_EN, 1), NBC_D1A_YNR_CON1, YNR_VIDEO_MODE, 4, YNR_D1B_VIDEO_MODE)\
	CMD(a, b, c, d, e, int, YNR_D1A_LCE_LINK, ynr.lce_link, YNR_LCE_LINK,, REG_CMP_EQ(c, LOG_YNR_EN, 1), NBC_D1A_YNR_CON1, YNR_LCE_LINK, 4, YNR_D1B_LCE_LINK)\
	CMD(a, b, c, d, e, int, YNR_D1A_SKIN_LINK, ynr.skin_link, YNR_SKIN_LINK,, REG_CMP_EQ(c, LOG_YNR_EN, 1), NBC_D1A_YNR_SKIN_CON, YNR_SKIN_LINK, 4, YNR_D1B_SKIN_LINK)\
	/* EE_D1 */\
	CMD(a, b, c, d, e, int, EE_D1A_OUT_EDGE_SEL, ee.ee_out_edge_sel, EE_OUT_EDGE_SEL,, REG_CMP_EQ(c, LOG_YNR_EN, 1), EE_D1A_EE_TOP_CTRL, EE_OUT_EDGE_SEL, 4, EE_D1B_OUT_EDGE_SEL)\
	/* CNR_D1 */\
	CMD(a, b, c, d, e, int, CNR_D1A_BPC_EN, cnr.bpc_en, CNR_BPC_EN,, REG_CMP_EQ(c, LOG_CNR_EN, 1), ABF_D1A_CNR_CNR_CON1, CNR_BPC_EN, 4, CNR_D1B_BPC_EN)\
	CMD(a, b, c, d, e, int, CNR_D1A_CNR_ENC, cnr.enc, CNR_ENC,, REG_CMP_EQ(c, LOG_CNR_EN, 1), ABF_D1A_CNR_CNR_CON1, CNR_CNR_ENC, 4, CNR_D1B_CNR_ENC)\
	CMD(a, b, c, d, e, int, CNR_D1A_VIDEO_MODE, cnr.video_mode, CNR_VIDEO_MODE,, REG_CMP_EQ(c, LOG_CNR_EN, 1), ABF_D1A_CNR_CNR_CON1, CNR_VIDEO_MODE, 4, CNR_D1B_VIDEO_MODE)\
	CMD(a, b, c, d, e, int, CNR_D1A_MODE, cnr.mode, CNR_MODE,, REG_CMP_EQ(c, LOG_CNR_EN, 1), ABF_D1A_CNR_CNR_CON1, CNR_MODE, 4, CNR_D1B_MODE)\
	CMD(a, b, c, d, e, int, CNR_D1A_CNR_SCALE_MODE, cnr.scale_mode, CNR_SCALE_MODE,, REG_CMP_EQ(c, LOG_CNR_EN, 1), ABF_D1A_CNR_CNR_CON1, CNR_CNR_SCALE_MODE, 4, CNR_D1B_CNR_SCALE_MODE)\
	CMD(a, b, c, d, e, int, CNR_D1A_ABF_EN, cnr.abf_en, CNR_ABF_EN,, REG_CMP_EQ(c, LOG_CNR_EN, 1), ABF_D1A_CNR_ABF_CON1, CNR_ABF_EN, 4, CNR_D1B_ABF_EN)\
	/* TIMGO_D1 */\
    CMD(a, b, c, d, e, int, TIMGO_D1A_STRIDE, timgo.timgo_stride, TIMGO_STRIDE,, REG_CMP_EQ(c, LOG_TIMGO_EN, 1), TIMGO_D1A_TIMGO_STRIDE, TIMGO_STRIDE, 4, TIMGO_D1B_STRIDE)\
    CMD(a, b, c, d, e, int, TIMGO_D1A_XOFFSET, timgo.timgo_xoffset, TIMGO_XOFFSET,, REG_CMP_EQ(c, LOG_TIMGO_EN, 1), TIMGO_D1A_TIMGO_CROP, TIMGO_XOFFSET, 0, TIMGO_D1B_XOFFSET)\
    CMD(a, b, c, d, e, int, TIMGO_D1A_YOFFSET, timgo.timgo_yoffset, TIMGO_YOFFSET,, REG_CMP_EQ(c, LOG_TIMGO_EN, 1), TIMGO_D1A_TIMGO_CROP, TIMGO_YOFFSET, 0, TIMGO_D1B_YOFFSET)\
    CMD(a, b, c, d, e, int, TIMGO_D1A_XSIZE, timgo.timgo_xsize, TIMGO_XSIZE,, REG_CMP_EQ(c, LOG_TIMGO_EN, 1), TIMGO_D1A_TIMGO_XSIZE, TIMGO_XSIZE, 0, TIMGO_D1B_XSIZE)\
    CMD(a, b, c, d, e, int, TIMGO_D1A_YSIZE, timgo.timgo_ysize, TIMGO_YSIZE,, REG_CMP_EQ(c, LOG_TIMGO_EN, 1), TIMGO_D1A_TIMGO_YSIZE, TIMGO_YSIZE, 0, TIMGO_D1B_YSIZE)\
	/* IMG3O_D1 */\
    CMD(a, b, c, d, e, int, IMG3O_D1A_STRIDE, img3o.img3o_stride, IMG3O_STRIDE,, REG_CMP_EQ(c, LOG_IMG3O_EN, 1), IMG3O_D1A_IMG3O_STRIDE, IMG3O_STRIDE, 4, IMG3O_D1B_STRIDE)\
    CMD(a, b, c, d, e, int, IMG3O_D1A_XOFFSET, img3o.img3o_xoffset, IMG3O_XOFFSET,, REG_CMP_EQ(c, LOG_IMG3O_EN, 1), IMG3O_D1A_IMG3O_CROP, IMG3O_XOFFSET, 0, IMG3O_D1B_XOFFSET)\
    CMD(a, b, c, d, e, int, IMG3O_D1A_YOFFSET, img3o.img3o_yoffset, IMG3O_YOFFSET,, REG_CMP_EQ(c, LOG_IMG3O_EN, 1), IMG3O_D1A_IMG3O_CROP, IMG3O_YOFFSET, 0, IMG3O_D1B_YOFFSET)\
    CMD(a, b, c, d, e, int, IMG3O_D1A_XSIZE, img3o.img3o_xsize, IMG3O_XSIZE,, REG_CMP_EQ(c, LOG_IMG3O_EN, 1), IMG3O_D1A_IMG3O_XSIZE, IMG3O_XSIZE, 0, IMG3O_D1B_XSIZE)\
    CMD(a, b, c, d, e, int, IMG3O_D1A_YSIZE, img3o.img3o_ysize, IMG3O_YSIZE,, REG_CMP_EQ(c, LOG_IMG3O_EN, 1), IMG3O_D1A_IMG3O_YSIZE, IMG3O_YSIZE, 0, IMG3O_D1B_YSIZE)\
	/* IMG3BO_D1 */\
    CMD(a, b, c, d, e, int, IMG3BO_D1A_STRIDE, img3bo.img3bo_stride, IMG3BO_STRIDE,, REG_CMP_EQ(c, LOG_IMG3BO_EN, 1), IMG3BO_D1A_IMG3BO_STRIDE, IMG3BO_STRIDE, 4, IMG3BO_D1B_STRIDE)\
    CMD(a, b, c, d, e, int, IMG3BO_D1A_XSIZE, img3bo.img3bo_xsize, IMG3BO_XSIZE,, REG_CMP_EQ(c, LOG_IMG3BO_EN, 1), IMG3BO_D1A_IMG3BO_XSIZE, IMG3BO_XSIZE, 0, IMG3BO_D1B_XSIZE)\
    CMD(a, b, c, d, e, int, IMG3BO_D1A_YSIZE, img3bo.img3bo_ysize, IMG3BO_YSIZE,, REG_CMP_EQ(c, LOG_IMG3BO_EN, 1), IMG3BO_D1A_IMG3BO_YSIZE, IMG3BO_YSIZE, 0, IMG3BO_D1B_YSIZE)\
	/* IMG3CO_D1 */\
    CMD(a, b, c, d, e, int, IMG3CO_D1A_STRIDE, img3co.img3co_stride, IMG3CO_STRIDE,, REG_CMP_EQ(c, LOG_IMG3CO_EN, 1), IMG3CO_D1A_IMG3CO_STRIDE, IMG3CO_STRIDE, 4, IMG3CO_D1B_STRIDE)\
    CMD(a, b, c, d, e, int, IMG3CO_D1A_XSIZE, img3co.img3co_xsize, IMG3CO_XSIZE,, REG_CMP_EQ(c, LOG_IMG3CO_EN, 1), IMG3CO_D1A_IMG3CO_XSIZE, IMG3CO_XSIZE, 0, IMG3CO_D1B_XSIZE)\
    CMD(a, b, c, d, e, int, IMG3CO_D1A_YSIZE, img3co.img3co_ysize, IMG3CO_YSIZE,, REG_CMP_EQ(c, LOG_IMG3CO_EN, 1), IMG3CO_D1A_IMG3CO_YSIZE, IMG3CO_YSIZE, 0, IMG3CO_D1B_YSIZE)\
	/* CRZO_D1 */\
    CMD(a, b, c, d, e, int, CRZO_D1A_STRIDE, crzo.crzo_stride, CRZO_STRIDE,, REG_CMP_EQ(c, LOG_CRZO_EN, 1), IMG3O_D1A_IMG3O_STRIDE, IMG3O_STRIDE, 4, CRZO_D1B_STRIDE)\
    CMD(a, b, c, d, e, int, CRZO_D1A_XOFFSET, crzo.crzo_xoffset, CRZO_XOFFSET,, REG_CMP_EQ(c, LOG_CRZO_EN, 1), IMG3O_D1A_IMG3O_CROP, IMG3O_XOFFSET, 0, CRZO_D1B_XOFFSET)\
    CMD(a, b, c, d, e, int, CRZO_D1A_YOFFSET, crzo.crzo_yoffset, CRZO_YOFFSET,, REG_CMP_EQ(c, LOG_CRZO_EN, 1), IMG3O_D1A_IMG3O_CROP, IMG3O_YOFFSET, 0, CRZO_D1B_YOFFSET)\
    CMD(a, b, c, d, e, int, CRZO_D1A_XSIZE, crzo.crzo_xsize, CRZO_XSIZE,, REG_CMP_EQ(c, LOG_CRZO_EN, 1), IMG3O_D1A_IMG3O_XSIZE, IMG3O_XSIZE, 0, CRZO_D1B_XSIZE)\
    CMD(a, b, c, d, e, int, CRZO_D1A_YSIZE, crzo.crzo_ysize, CRZO_YSIZE,, REG_CMP_EQ(c, LOG_CRZO_EN, 1), IMG3O_D1A_IMG3O_YSIZE, IMG3O_YSIZE, 0, CRZO_D1B_YSIZE)\
	/* CRZBO_D1 */\
    CMD(a, b, c, d, e, int, CRZBO_D1A_STRIDE, crzbo.crzbo_stride, CRZBO_STRIDE,, REG_CMP_EQ(c, LOG_CRZBO_EN, 1), IMG3O_D1A_IMG3O_STRIDE, IMG3O_STRIDE, 4, CRZBO_D1B_STRIDE)\
    CMD(a, b, c, d, e, int, CRZBO_D1A_XSIZE, crzbo.crzbo_xsize, CRZBO_XSIZE,, REG_CMP_EQ(c, LOG_CRZBO_EN, 1), IMG3O_D1A_IMG3O_XSIZE, IMG3O_XSIZE, 0, CRZBO_D1B_XSIZE)\
    CMD(a, b, c, d, e, int, CRZBO_D1A_YSIZE, crzbo.crzbo_ysize, CRZBO_YSIZE,, REG_CMP_EQ(c, LOG_CRZBO_EN, 1), IMG3O_D1A_IMG3O_YSIZE, IMG3O_YSIZE, 0, CRZBO_D1B_YSIZE)\
	/* UFD_D1 */\
    CMD(a, b, c, d, e, int, UFD_D1A_SEL, ufd.ufd_sel, ufd_sel,, true, IMG3O_D1A_IMG3O_STRIDE, IMG3O_STRIDE, 4, UFD_D1B_SEL)\
    CMD(a, b, c, d, e, int, UFD_D1A_BOND_MODE, ufd.ufd_bond_mode, ufd_bond_mode,, true, IMG3O_D1A_IMG3O_STRIDE, IMG3O_STRIDE, 0, UFD_D1B_BOND_MODE)\
    CMD(a, b, c, d, e, int, UFD_D1A_BS2_AU_START, ufd.ufd_bs2_au_start, ufd_bs2_au_start,, true, IMG3O_D1A_IMG3O_STRIDE, IMG3O_STRIDE, 0, UFD_D1B_BS2_AU_START)\
    CMD(a, b, c, d, e, int, UFD_D1A_BOND2_MODE, ufd.ufd_bond2_mode, ufd_bond2_mode,, true, IMG3O_D1A_IMG3O_STRIDE, IMG3O_STRIDE, 0, UFD_D1B_BOND2_MODE)\
    CMD(a, b, c, d, e, int, UFD_D1A_BS3_AU_START, ufd.ufd_bs3_au_start, ufd_bs3_au_start,, true, IMG3O_D1A_IMG3O_STRIDE, IMG3O_STRIDE, 0, UFD_D1B_BS3_AU_START)\
    /* SLK_D1 */\
    CMD(a, b, c, d, e, int, SLK_D1A_HRZ_COMP, slk1.slk_hrz_comp, SLK1_HRZ_COMP,, REG_CMP_EQ(c, LOG_SLK1_EN, 1), SLK_D1A_SLK_RZ, SLK_HRZ_COMP, 4, SLK_D1B_HRZ_COMP)\
    CMD(a, b, c, d, e, int, SLK_D1A_VRZ_COMP, slk1.slk_vrz_comp, SLK1_VRZ_COMP,, REG_CMP_EQ(c, LOG_SLK1_EN, 1), SLK_D1A_SLK_RZ, SLK_VRZ_COMP, 4, SLK_D1B_VRZ_COMP)\
    /* SLK_D2 */\
    CMD(a, b, c, d, e, int, SLK_D2A_HRZ_COMP, slk2.slk_hrz_comp, SLK2_HRZ_COMP,, REG_CMP_EQ(c, LOG_SLK2_EN, 1), SLK_D2A_SLK_RZ, SLK_HRZ_COMP, 4, SLK_D2B_HRZ_COMP)\
    CMD(a, b, c, d, e, int, SLK_D2A_VRZ_COMP, slk2.slk_vrz_comp, SLK2_VRZ_COMP,, REG_CMP_EQ(c, LOG_SLK2_EN, 1), SLK_D2A_SLK_RZ, SLK_VRZ_COMP, 4, SLK_D2B_VRZ_COMP)\
    /* SLK_D3 */\
    CMD(a, b, c, d, e, int, SLK_D3A_HRZ_COMP, slk3.slk_hrz_comp, SLK3_HRZ_COMP,, REG_CMP_EQ(c, LOG_SLK3_EN, 1), SLK_D3A_SLK_RZ, SLK_HRZ_COMP, 4, SLK_D3B_HRZ_COMP)\
    CMD(a, b, c, d, e, int, SLK_D3A_VRZ_COMP, slk3.slk_vrz_comp, SLK3_VRZ_COMP,, REG_CMP_EQ(c, LOG_SLK3_EN, 1), SLK_D3A_SLK_RZ, SLK_VRZ_COMP, 4, SLK_D3B_VRZ_COMP)\
    /* SLK_D4 */\
    CMD(a, b, c, d, e, int, SLK_D4A_HRZ_COMP, slk4.slk_hrz_comp, SLK4_HRZ_COMP,, REG_CMP_EQ(c, LOG_SLK4_EN, 1), SLK_D4A_SLK_RZ, SLK_HRZ_COMP, 4, SLK_D4B_HRZ_COMP)\
    CMD(a, b, c, d, e, int, SLK_D4A_VRZ_COMP, slk4.slk_vrz_comp, SLK4_VRZ_COMP,, REG_CMP_EQ(c, LOG_SLK4_EN, 1), SLK_D4A_SLK_RZ, SLK_VRZ_COMP, 4, SLK_D4B_VRZ_COMP)\
    /* SLK_D5 */\
    CMD(a, b, c, d, e, int, SLK_D5A_HRZ_COMP, slk5.slk_hrz_comp, SLK5_HRZ_COMP,, REG_CMP_EQ(c, LOG_SLK5_EN, 1), SLK_D5A_SLK_RZ, SLK_HRZ_COMP, 4, SLK_D5B_HRZ_COMP)\
    CMD(a, b, c, d, e, int, SLK_D5A_VRZ_COMP, slk5.slk_vrz_comp, SLK5_VRZ_COMP,, REG_CMP_EQ(c, LOG_SLK5_EN, 1), SLK_D5A_SLK_RZ, SLK_VRZ_COMP, 4, SLK_D5B_VRZ_COMP)\
    /* SLK_D6 */\
    CMD(a, b, c, d, e, int, SLK_D6A_HRZ_COMP, slk6.slk_hrz_comp, SLK6_HRZ_COMP,, REG_CMP_EQ(c, LOG_SLK6_EN, 1), SLK_D5A_SLK_RZ, SLK_HRZ_COMP, 4, SLK_D6B_HRZ_COMP)\
    CMD(a, b, c, d, e, int, SLK_D6A_VRZ_COMP, slk6.slk_vrz_comp, SLK6_VRZ_COMP,, REG_CMP_EQ(c, LOG_SLK6_EN, 1), SLK_D5A_SLK_RZ, SLK_VRZ_COMP, 4, SLK_D6B_VRZ_COMP)\
    /* LSC_D1 */\
    CMD(a, b, c, d, e, int, LSC_D1A_EXTEND_COEF_MODE, lsc.extend_coef_mode, EXTEND_COEF_MODE,, REG_CMP_EQ(c, LOG_LSC_EN, 1), SLK_D5A_SLK_RZ, SLK_HRZ_COMP, 4, LSC_D1B_EXTEND_COEF_MODE)\
    CMD(a, b, c, d, e, int, LSC_D1A_SDBLK_XNUM, lsc.sdblk_xnum, SDBLK_XNUM,, REG_CMP_EQ(c, LOG_LSC_EN, 1), SLK_D5A_SLK_RZ, SLK_HRZ_COMP, 0, LSC_D1B_SDBLK_XNUM)\
    CMD(a, b, c, d, e, int, LSC_D1A_SDBLK_YNUM, lsc.sdblk_ynum, SDBLK_YNUM,, REG_CMP_EQ(c, LOG_LSC_EN, 1), SLK_D5A_SLK_RZ, SLK_HRZ_COMP, 0, LSC_D1B_SDBLK_YNUM)\
    CMD(a, b, c, d, e, int, LSC_D1A_SDBLK_WIDTH, lsc.sdblk_width, SDBLK_WIDTH,, REG_CMP_EQ(c, LOG_LSC_EN, 1), SLK_D5A_SLK_RZ, SLK_HRZ_COMP, 0, LSC_D1B_SDBLK_WIDTH)\
    CMD(a, b, c, d, e, int, LSC_D1A_SDBLK_HEIGHT, lsc.sdblk_height, SDBLK_HEIGHT,, REG_CMP_EQ(c, LOG_LSC_EN, 1), SLK_D5A_SLK_RZ, SLK_HRZ_COMP, 0, LSC_D1B_SDBLK_HEIGHT)\
    CMD(a, b, c, d, e, int, LSC_D1A_SDBLK_lWIDTH, lsc.sdblk_last_width, SDBLK_LWIDTH,, REG_CMP_EQ(c, LOG_LSC_EN, 1), SLK_D5A_SLK_RZ, SLK_HRZ_COMP, 0, LSC_D1B_SDBLK_lWIDTH)\
    CMD(a, b, c, d, e, int, LSC_D1A_SDBLK_lHEIGHT, lsc.sdblk_last_height, SDBLK_LHEIGHT,, REG_CMP_EQ(c, LOG_LSC_EN, 1), SLK_D5A_SLK_RZ, SLK_HRZ_COMP, 0, LSC_D1B_SDBLK_lHEIGHT)\
    /* LTM_D1 */\
    CMD(a, b, c, d, e, int, LTM_D1A_BLK_X_NUM, ltm.blk_x_num, LTMBLK_XNUM,, REG_CMP_EQ(c, LOG_LTM_EN, 1), SLK_D5A_SLK_RZ, SLK_HRZ_COMP, 4, LTM_D1B_BLK_X_NUM)\
    CMD(a, b, c, d, e, int, LTM_D1A_BLK_Y_NUM, ltm.blk_y_num, LTMBLK_YNUM,, REG_CMP_EQ(c, LOG_LTM_EN, 1), SLK_D5A_SLK_RZ, SLK_HRZ_COMP, 4, LTM_D1B_BLK_Y_NUM)\
    CMD(a, b, c, d, e, int, LTM_D1A_BLK_WIDTH, ltm.blk_width, LTMBLK_WIDTH,, REG_CMP_EQ(c, LOG_LTM_EN, 1), SLK_D5A_SLK_RZ, SLK_HRZ_COMP, 4, LTM_D1B_BLK_WIDTH)\
    CMD(a, b, c, d, e, int, LTM_D1A_BLK_HEIGHT, ltm.blk_height, LTMBLK_HEIGHT,, REG_CMP_EQ(c, LOG_LTM_EN, 1), SLK_D5A_SLK_RZ, SLK_HRZ_COMP, 4, LTM_D1B_BLK_HEIGHT)\
   /* G2CX_D1 */\
    CMD(a, b, c, d, e, int, G2CX_D1A_SHADE_EN, g2cx.g2cx_shade_en, G2CX_SHADE_EN,, REG_CMP_EQ(c, LOG_G2CX_EN, 1), G2C_D2A_G2CX_SHADE_CON_1, G2CX_SHADE_EN, 4, G2CX_D1B_SHADE_EN)\
    CMD(a, b, c, d, e, int, G2CX_D1A_SHADE_XMID, g2cx.g2cx_shade_xmid, G2CX_SHADE_XMID,, REG_CMP_EQ(c, LOG_G2CX_EN, 1), G2C_D2A_G2CX_SHADE_TAR, G2CX_SHADE_XMID, 0, G2CX_D1B_SHADE_XMID)\
    CMD(a, b, c, d, e, int, G2CX_D1A_SHADE_YMID, g2cx.g2cx_shade_ymid, G2CX_SHADE_YMID,, REG_CMP_EQ(c, LOG_G2CX_EN, 1), G2C_D2A_G2CX_SHADE_TAR, G2CX_SHADE_YMID, 0, G2CX_D1B_SHADE_YMID)\
    CMD(a, b, c, d, e, int, G2CX_D1A_SHADE_VAR, g2cx.g2cx_shade_var, G2CX_SHADE_VAR,, REG_CMP_EQ(c, LOG_G2CX_EN, 1), G2C_D2A_G2CX_SHADE_CON_1, G2CX_SHADE_VAR, 0, G2CX_D1B_SHADE_VAR)\
	/* DCES_D1 */\
    CMD(a, b, c, d, e, int, DCES_D1A_CROP_XSTART, dces.dces_crop_xstart, DCES_CROP_XSTART,, REG_CMP_EQ(c, LOG_DCES_EN, 1), DGGMS_D1A_DCES_CROP_X, DCES_CROP_XSTART, 4, DCES_D1B_CROP_XSTART)\
    CMD(a, b, c, d, e, int, DCES_D1A_CROP_XEND, dces.dces_crop_xend, DCES_CROP_XEND,, REG_CMP_EQ(c, LOG_DCES_EN, 1), DGGMS_D1A_DCES_CROP_X, DCES_CROP_XEND, 4, DCES_D1B_CROP_XEND)\
    CMD(a, b, c, d, e, int, DCES_D1A_CROP_YSTART, dces.dces_crop_ystart, DCES_CROP_YSTART,, REG_CMP_EQ(c, LOG_DCES_EN, 1), DGGMS_D1A_DCES_CROP_Y, DCES_CROP_YSTART, 4, DCES_D1B_CROP_YSTART)\
    CMD(a, b, c, d, e, int, DCES_D1A_CROP_YEND, dces.dces_crop_yend, DCES_CROP_YEND,, REG_CMP_EQ(c, LOG_DCES_EN, 1), DGGMS_D1A_DCES_CROP_Y, DCES_CROP_YEND, 4, DCES_D1B_CROP_YEND)\
	/* DCESO_D1 */\
    CMD(a, b, c, d, e, int, DCESO_D1A_XSIZE, dceso.dceso_xsize, DCESO_XSIZE,, REG_CMP_EQ(c, LOG_DCESO_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, DCESO_D1B_XSIZE)\
    CMD(a, b, c, d, e, int, DCESO_D1A_YSIZE, dceso.dceso_ysize, DCESO_YSIZE,, REG_CMP_EQ(c, LOG_DCESO_EN, 1), DGGMSO_D1A_DGGMSO_YSIZE, DGGMSO_YSIZE, 0, DCESO_D1B_YSIZE)\
	/* SRZ_D1 */\
    CMD(a, b, c, d, e, int, SRZ_D1A_IN_CROP_WD, srz1.srz_input_crop_width, SRZ1_Input_Image_W,, REG_CMP_EQ(c, LOG_SRZ1_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, SRZ_D1B_IN_CROP_WD)\
    CMD(a, b, c, d, e, int, SRZ_D1A_IN_CROP_HT, srz1.srz_input_crop_height, SRZ1_Input_Image_H,, REG_CMP_EQ(c, LOG_SRZ1_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, SRZ_D1B_IN_CROP_HT)\
    CMD(a, b, c, d, e, int, SRZ_D1A_OUT_WD, srz1.srz_output_width, SRZ1_Output_Image_W,, REG_CMP_EQ(c, LOG_SRZ1_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, SRZ_D1B_OUT_WD)\
    CMD(a, b, c, d, e, int, SRZ_D1A_OUT_HT, srz1.srz_output_height, SRZ1_Output_Image_H,, REG_CMP_EQ(c, LOG_SRZ1_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, SRZ_D1B_OUT_HT)\
    CMD(a, b, c, d, e, int, SRZ_D1A_HORI_INT_OFST, srz1.srz_luma_horizontal_integer_offset, SRZ1_Luma_Horizontal_Integer_Offset,, REG_CMP_EQ(c, LOG_SRZ1_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, SRZ_D1B_HORI_INT_OFST)\
    CMD(a, b, c, d, e, int, SRZ_D1A_HORI_SUB_OFST, srz1.srz_luma_horizontal_subpixel_offset, SRZ1_Luma_Horizontal_Subpixel_Offset,, REG_CMP_EQ(c, LOG_SRZ1_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, SRZ_D1B_HORI_SUB_OFST)\
    CMD(a, b, c, d, e, int, SRZ_D1A_VERT_INT_OFST, srz1.srz_luma_vertical_integer_offset, SRZ1_Luma_Vertical_Integer_Offset,, REG_CMP_EQ(c, LOG_SRZ1_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, SRZ_D1B_VERT_INT_OFST)\
    CMD(a, b, c, d, e, int, SRZ_D1A_VERT_SUB_OFST, srz1.srz_luma_vertical_subpixel_offset, SRZ1_Luma_Vertical_Subpixel_Offset,, REG_CMP_EQ(c, LOG_SRZ1_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, SRZ_D1B_VERT_SUB_OFST)\
    CMD(a, b, c, d, e, int, SRZ_D1A_HORI_STEP, srz1.srz_horizontal_coeff_step, SRZ1_Horizontal_Coeff_Step,, REG_CMP_EQ(c, LOG_SRZ1_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 4, SRZ_D1B_HORI_STEP)\
    CMD(a, b, c, d, e, int, SRZ_D1A_VERT_STEP, srz1.srz_vertical_coeff_step, SRZ1_Vertical_Coeff_Step,, REG_CMP_EQ(c, LOG_SRZ1_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 4, SRZ_D1B_VERT_STEP)\
	/* SRZ_D3 */\
    CMD(a, b, c, d, e, int, SRZ_D3A_IN_CROP_WD, srz3.srz_input_crop_width, SRZ3_Input_Image_W,, REG_CMP_EQ(c, LOG_SRZ3_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, SRZ_D3B_IN_CROP_WD)\
    CMD(a, b, c, d, e, int, SRZ_D3A_IN_CROP_HT, srz3.srz_input_crop_height, SRZ3_Input_Image_H,, REG_CMP_EQ(c, LOG_SRZ3_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, SRZ_D3B_IN_CROP_HT)\
    CMD(a, b, c, d, e, int, SRZ_D3A_OUT_WD, srz3.srz_output_width, SRZ3_Output_Image_W,, REG_CMP_EQ(c, LOG_SRZ3_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, SRZ_D3B_OUT_WD)\
    CMD(a, b, c, d, e, int, SRZ_D3A_OUT_HT, srz3.srz_output_height, SRZ3_Output_Image_H,, REG_CMP_EQ(c, LOG_SRZ3_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, SRZ_D3B_OUT_HT)\
    CMD(a, b, c, d, e, int, SRZ_D3A_HORI_INT_OFST, srz3.srz_luma_horizontal_integer_offset, SRZ3_Luma_Horizontal_Integer_Offset,, REG_CMP_EQ(c, LOG_SRZ3_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, SRZ_D3B_HORI_INT_OFST)\
    CMD(a, b, c, d, e, int, SRZ_D3A_HORI_SUB_OFST, srz3.srz_luma_horizontal_subpixel_offset, SRZ3_Luma_Horizontal_Subpixel_Offset,, REG_CMP_EQ(c, LOG_SRZ3_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, SRZ_D3B_HORI_SUB_OFST)\
    CMD(a, b, c, d, e, int, SRZ_D3A_VERT_INT_OFST, srz3.srz_luma_vertical_integer_offset, SRZ3_Luma_Vertical_Integer_Offset,, REG_CMP_EQ(c, LOG_SRZ3_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, SRZ_D3B_VERT_INT_OFST)\
    CMD(a, b, c, d, e, int, SRZ_D3A_VERT_SUB_OFST, srz3.srz_luma_vertical_subpixel_offset, SRZ3_Luma_Vertical_Subpixel_Offset,, REG_CMP_EQ(c, LOG_SRZ3_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, SRZ_D3B_VERT_SUB_OFST)\
    CMD(a, b, c, d, e, int, SRZ_D3A_HORI_STEP, srz3.srz_horizontal_coeff_step, SRZ3_Horizontal_Coeff_Step,, REG_CMP_EQ(c, LOG_SRZ3_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 4, SRZ_D3B_HORI_STEP)\
    CMD(a, b, c, d, e, int, SRZ_D3A_VERT_STEP, srz3.srz_vertical_coeff_step, SRZ3_Vertical_Coeff_Step,, REG_CMP_EQ(c, LOG_SRZ3_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 4, SRZ_D3B_VERT_STEP)\
	/* SRZ_D4 */\
    CMD(a, b, c, d, e, int, SRZ_D4A_IN_CROP_WD, srz4.srz_input_crop_width, SRZ4_Input_Image_W,, REG_CMP_EQ(c, LOG_SRZ4_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, SRZ_D4B_IN_CROP_WD)\
    CMD(a, b, c, d, e, int, SRZ_D4A_IN_CROP_HT, srz4.srz_input_crop_height, SRZ4_Input_Image_H,, REG_CMP_EQ(c, LOG_SRZ4_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, SRZ_D4B_IN_CROP_HT)\
    CMD(a, b, c, d, e, int, SRZ_D4A_OUT_WD, srz4.srz_output_width, SRZ4_Output_Image_W,, REG_CMP_EQ(c, LOG_SRZ4_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, SRZ_D4B_OUT_WD)\
    CMD(a, b, c, d, e, int, SRZ_D4A_OUT_HT, srz4.srz_output_height, SRZ4_Output_Image_H,, REG_CMP_EQ(c, LOG_SRZ4_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, SRZ_D4B_OUT_HT)\
    CMD(a, b, c, d, e, int, SRZ_D4A_HORI_INT_OFST, srz4.srz_luma_horizontal_integer_offset, SRZ4_Luma_Horizontal_Integer_Offset,, REG_CMP_EQ(c, LOG_SRZ4_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, SRZ_D4B_HORI_INT_OFST)\
    CMD(a, b, c, d, e, int, SRZ_D4A_HORI_SUB_OFST, srz4.srz_luma_horizontal_subpixel_offset, SRZ4_Luma_Horizontal_Subpixel_Offset,, REG_CMP_EQ(c, LOG_SRZ4_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, SRZ_D4B_HORI_SUB_OFST)\
    CMD(a, b, c, d, e, int, SRZ_D4A_VERT_INT_OFST, srz4.srz_luma_vertical_integer_offset, SRZ4_Luma_Vertical_Integer_Offset,, REG_CMP_EQ(c, LOG_SRZ4_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, SRZ_D4B_VERT_INT_OFST)\
    CMD(a, b, c, d, e, int, SRZ_D4A_VERT_SUB_OFST, srz4.srz_luma_vertical_subpixel_offset, SRZ4_Luma_Vertical_Subpixel_Offset,, REG_CMP_EQ(c, LOG_SRZ4_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, SRZ_D4B_VERT_SUB_OFST)\
    CMD(a, b, c, d, e, int, SRZ_D4A_HORI_STEP, srz4.srz_horizontal_coeff_step, SRZ4_Horizontal_Coeff_Step,, REG_CMP_EQ(c, LOG_SRZ4_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 4, SRZ_D4B_HORI_STEP)\
    CMD(a, b, c, d, e, int, SRZ_D4A_VERT_STEP, srz4.srz_vertical_coeff_step, SRZ4_Vertical_Coeff_Step,, REG_CMP_EQ(c, LOG_SRZ4_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 4, SRZ_D4B_VERT_STEP)\
    /* CRZ_D1 */\
    CMD(a, b, c, d, e, int, CRZ_D1A_crop_size_x_d, crz.crz_input_crop_width, CRZ_Input_Image_W,, REG_CMP_EQ(c, LOG_CRZ_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, CRZ_D1B_crop_size_x_d)\
    CMD(a, b, c, d, e, int, CRZ_D1A_crop_size_y_d, crz.crz_input_crop_height, CRZ_Input_Image_H,, REG_CMP_EQ(c, LOG_CRZ_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, CRZ_D1B_crop_size_y_d)\
    CMD(a, b, c, d, e, int, CRZ_D1A_OUT_WD, crz.crz_output_width, CRZ_Output_Image_W,, REG_CMP_EQ(c, LOG_CRZ_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, CRZ_D1B_OUT_WD)\
    CMD(a, b, c, d, e, int, CRZ_D1A_OUT_HT, crz.crz_output_height, CRZ_Output_Image_H,, REG_CMP_EQ(c, LOG_CRZ_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, CRZ_D1B_OUT_HT)\
    CMD(a, b, c, d, e, int, CRZ_D1A_LUMA_HORI_INT_OFST, crz.crz_luma_horizontal_integer_offset, CRZ_Luma_Horizontal_Integer_Offset,, REG_CMP_EQ(c, LOG_CRZ_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, CRZ_D1B_LUMA_HORI_INT_OFST)\
    CMD(a, b, c, d, e, int, CRZ_D1A_LUMA_HORI_SUB_OFST, crz.crz_luma_horizontal_subpixel_offset, CRZ_Luma_Horizontal_Subpixel_Offset,, REG_CMP_EQ(c, LOG_CRZ_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, CRZ_D1B_LUMA_HORI_SUB_OFST)\
    CMD(a, b, c, d, e, int, CRZ_D1A_LUMA_VERT_INT_OFST, crz.crz_luma_vertical_integer_offset, CRZ_Luma_Vertical_Integer_Offset,, REG_CMP_EQ(c, LOG_CRZ_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, CRZ_D1B_LUMA_VERT_INT_OFST)\
    CMD(a, b, c, d, e, int, CRZ_D1A_LUMA_VERT_SUB_OFST, crz.crz_luma_vertical_subpixel_offset, CRZ_Luma_Vertical_Subpixel_Offset,, REG_CMP_EQ(c, LOG_CRZ_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, CRZ_D1B_LUMA_VERT_SUB_OFST)\
    CMD(a, b, c, d, e, int, CRZ_D1A_HORI_ALGO, crz.crz_horizontal_luma_algorithm, CRZ_Horizontal_Luma_Algorithm,, REG_CMP_EQ(c, LOG_CRZ_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 4, CRZ_D1B_HORI_ALGO)\
    CMD(a, b, c, d, e, int, CRZ_D1A_VERT_ALGO, crz.crz_vertical_luma_algorithm, CRZ_Vertical_Luma_Algorithm,, REG_CMP_EQ(c, LOG_CRZ_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 4, CRZ_D1B_VERT_ALGO)\
    CMD(a, b, c, d, e, int, CRZ_D1A_HORI_STEP, crz.crz_horizontal_coeff_step, CRZ_Horizontal_Coeff_Step,, REG_CMP_EQ(c, LOG_CRZ_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 4, CRZ_D1B_HORI_STEP)\
    CMD(a, b, c, d, e, int, CRZ_D1A_VERT_STEP, crz.crz_vertical_coeff_step, CRZ_Vertical_Coeff_Step,, REG_CMP_EQ(c, LOG_CRZ_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 4, CRZ_D1B_VERT_STEP)\
    /* DFE_D1 */\
    CMD(a, b, c, d, e, int, DFE_D1A_MODE, dfe.dfe_mode, DFE_MODE,, REG_CMP_EQ(c, LOG_DFE_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 4, DFE_D1B_MODE)\
    /* FEO_D1 */\
    CMD(a, b, c, d, e, int, FEO_D1A_STRIDE, feo.feo_stride, FEO_STRIDE,, REG_CMP_EQ(c, LOG_FEO_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 4, FEO_D1B_STRIDE)\
    /* NR3D_D1 */\
    CMD(a, b, c, d, e, int, NR3D_D1A_snr_en, nr3d.nr3d_snr_en, NR3D_SNR_EN,, REG_CMP_EQ(c, LOG_NR3D_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 4, NR3D_D1B_snr_en)\
    CMD(a, b, c, d, e, int, NR3D_D1A_on_en, nr3d.nr3d_on_en, NR3D_ON_EN,, REG_CMP_EQ(c, LOG_NR3D_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 4, NR3D_D1B_on_en)\
    CMD(a, b, c, d, e, int, NR3D_D1A_nr3d_on_ofst_x, nr3d.nr3d_on_xoffset, NR3D_ON_OFST_X,, REG_CMP_EQ(c, LOG_NR3D_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, NR3D_D1B_nr3d_on_ofst_x)\
    CMD(a, b, c, d, e, int, NR3D_D1A_nr3d_on_ofst_y, nr3d.nr3d_on_yoffset, NR3D_ON_OFST_Y,, REG_CMP_EQ(c, LOG_NR3D_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, NR3D_D1B_nr3d_on_ofst_y)\
    CMD(a, b, c, d, e, int, NR3D_D1A_nr3d_on_wd, nr3d.nr3d_on_width, NR3D_ON_WD,, REG_CMP_EQ(c, LOG_NR3D_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, NR3D_D1B_nr3d_on_wd)\
    CMD(a, b, c, d, e, int, NR3D_D1A_nr3d_on_ht, nr3d.nr3d_on_height, NR3D_ON_HT,, REG_CMP_EQ(c, LOG_NR3D_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, NR3D_D1B_nr3d_on_ht)\
	/* CRSP_D1 */\
    CMD(a, b, c, d, e, int, CRSP_D1A_cstep_y, crsp.crsp_ystep, CRSP_STEP_Y,, REG_CMP_EQ(c, LOG_CRSP_EN, 1), CRSP_D1A_CRSP_STEP_OFST, CRSP_STEP_Y, 4, CRSP_D1B_cstep_y)\
    CMD(a, b, c, d, e, int, CRSP_D1A_offset_x, crsp.crsp_xoffset, CRSP_OFST_X,, REG_CMP_EQ(c, LOG_CRSP_EN, 1), CRSP_D1A_CRSP_STEP_OFST, CRSP_OFST_X, 0, CRSP_D1B_offset_x)\
    CMD(a, b, c, d, e, int, CRSP_D1A_offset_y, crsp.crsp_yoffset, CRSP_OFST_Y,, REG_CMP_EQ(c, LOG_CRSP_EN, 1), CRSP_D1A_CRSP_STEP_OFST, CRSP_OFST_Y, 0, CRSP_D1B_offset_y)\
	/* ADL_D1 */\
    CMD(a, b, c, d, e, int, ADL_D1A_CTL_EN, adl.adl_ctl_en, ADL_CTL_EN,, REG_CMP_EQ(c, ADL_EN, 1), DIPCTL_D1A_DIPCTL_DMA_EN1, DIPCTL_ADL_D1_EN, 4, ADL_D1B_CTL_EN)\
    /* SMTI_D1 */\
    CMD(a, b, c, d, e, int, DIPDMATOP_D1A_SMTI_D1_V_FLIP_EN, smt1i.smt1i_v_flip_en, SMT1I_V_FLIP_EN,, REG_CMP_EQ(c, SMT1I_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 4, DIPDMATOP_D1B_SMTI_D1_V_FLIP_EN)\
    CMD(a, b, c, d, e, int, SMTI_D1A_XSIZE, smt1i.smt1i_xsize, SMT1I_XSIZE,, REG_CMP_EQ(c, SMT1I_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, SMTI_D1B_XSIZE)\
    CMD(a, b, c, d, e, int, SMTI_D1A_YSIZE, smt1i.smt1i_ysize, SMT1I_YSIZE,, REG_CMP_EQ(c, SMT1I_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, SMTI_D1B_YSIZE)\
    /* SMTO_D1 */\
    CMD(a, b, c, d, e, int, SMTO_D1A_XSIZE, smt1o.smt1o_xsize, SMT1O_XSIZE,, REG_CMP_EQ(c, SMT1O_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, SMTO_D1B_XSIZE)\
    CMD(a, b, c, d, e, int, SMTO_D1A_YSIZE, smt1o.smt1o_ysize, SMT1O_YSIZE,, REG_CMP_EQ(c, SMT1O_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, SMTO_D1B_YSIZE)\
    /* SMTI_D2 */\
    CMD(a, b, c, d, e, int, DIPDMATOP_D1A_SMTI_D2_V_FLIP_EN, smt2i.smt2i_v_flip_en, SMT2I_V_FLIP_EN,, REG_CMP_EQ(c, SMT2I_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 4, DIPDMATOP_D1B_SMTI_D2_V_FLIP_EN)\
    CMD(a, b, c, d, e, int, SMTI_D2A_XSIZE, smt2i.smt2i_xsize, SMT2I_XSIZE,, REG_CMP_EQ(c, SMT2I_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, SMTI_D2B_XSIZE)\
    CMD(a, b, c, d, e, int, SMTI_D2A_YSIZE, smt2i.smt2i_ysize, SMT2I_YSIZE,, REG_CMP_EQ(c, SMT2I_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, SMTI_D2B_YSIZE)\
    /* SMTO_D2 */\
    CMD(a, b, c, d, e, int, SMTO_D2A_XSIZE, smt2o.smt2o_xsize, SMT2O_XSIZE,, REG_CMP_EQ(c, SMT2O_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, SMTO_D2B_XSIZE)\
    CMD(a, b, c, d, e, int, SMTO_D2A_YSIZE, smt2o.smt2o_ysize, SMT2O_YSIZE,, REG_CMP_EQ(c, SMT2O_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, SMTO_D2B_YSIZE)\
    /* SMTI_D3 */\
    CMD(a, b, c, d, e, int, DIPDMATOP_D1A_SMTI_D3_V_FLIP_EN, smt3i.smt3i_v_flip_en, SMT3I_V_FLIP_EN,, REG_CMP_EQ(c, SMT3I_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 4, DIPDMATOP_D1B_SMTI_D3_V_FLIP_EN)\
    CMD(a, b, c, d, e, int, SMTI_D3A_XSIZE, smt3i.smt3i_xsize, SMT3I_XSIZE,, REG_CMP_EQ(c, SMT3I_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, SMTI_D3B_XSIZE)\
    CMD(a, b, c, d, e, int, SMTI_D3A_YSIZE, smt3i.smt3i_ysize, SMT3I_YSIZE,, REG_CMP_EQ(c, SMT3I_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, SMTI_D3B_YSIZE)\
    /* SMTO_D3 */\
    CMD(a, b, c, d, e, int, SMTO_D3A_XSIZE, smt3o.smt3o_xsize, SMT3O_XSIZE,, REG_CMP_EQ(c, SMT3O_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, SMTO_D3B_XSIZE)\
    CMD(a, b, c, d, e, int, SMTO_D3A_YSIZE, smt3o.smt3o_ysize, SMT3O_YSIZE,, REG_CMP_EQ(c, SMT3O_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, SMTO_D3B_YSIZE)\
    /* SMTI_D4 */\
    CMD(a, b, c, d, e, int, DIPDMATOP_D1A_SMTI_D4_V_FLIP_EN, smt4i.smt4i_v_flip_en, SMT4I_V_FLIP_EN,, REG_CMP_EQ(c, SMT4I_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 4, DIPDMATOP_D1B_SMTI_D4_V_FLIP_EN)\
    CMD(a, b, c, d, e, int, SMTI_D4A_XSIZE, smt4i.smt4i_xsize, SMT4I_XSIZE,, REG_CMP_EQ(c, SMT4I_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, SMTI_D4B_XSIZE)\
    CMD(a, b, c, d, e, int, SMTI_D4A_YSIZE, smt4i.smt4i_ysize, SMT4I_YSIZE,, REG_CMP_EQ(c, SMT4I_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, SMTI_D4B_YSIZE)\
    /* SMTO_D4 */\
    CMD(a, b, c, d, e, int, SMTO_D4A_XSIZE, smt4o.smt4o_xsize, SMT4O_XSIZE,, REG_CMP_EQ(c, SMT4O_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, SMTO_D4B_XSIZE)\
    CMD(a, b, c, d, e, int, SMTO_D4A_YSIZE, smt4o.smt4o_ysize, SMT4O_YSIZE,, REG_CMP_EQ(c, SMT4O_EN, 1), DGGMSO_D1A_DGGMSO_XSIZE, DGGMSO_XSIZE, 0, SMTO_D4B_YSIZE)\
    /* OFT */\
    CMD(a, b, c, d, e, int, OFT_D1A_CROP_EN, oft.oft_crop_en, OFT_CROP_EN,, true, MDPS_D1A_OFT_OFT_MODULE_EN, OFT_CROP_EN, 4, OFT_D1B_CROP_EN)\
    CMD(a, b, c, d, e, int, OFT_D1A_CRZ_EN, oft.oft_crz_en, OFT_CRZ_EN,, true, MDPS_D1A_OFT_OFT_MODULE_EN, OFT_CRZ_EN, 4, OFT_D1B_CRZ_EN)\
    CMD(a, b, c, d, e, int, OFT_D1A_DTH_EN, oft.oft_dth_en, OFT_DTH_EN,, true, MDPS_D1A_OFT_OFT_MODULE_EN, OFT_DTH_EN, 4, OFT_D1B_DTH_EN)\
    CMD(a, b, c, d, e, int, OFT_D1A_BCVT_EN, oft.oft_bcvt_en, OFT_BCVT_EN,, true, MDPS_D1A_OFT_OFT_MODULE_EN, OFT_BCVT_EN, 4, OFT_D1B_BCVT_EN)\
    CMD(a, b, c, d, e, int, OFT_D1A_C24_EN, oft.oft_c24_en, OFT_C24_EN,, true, MDPS_D1A_OFT_OFT_MODULE_EN, OFT_C24_EN, 4, OFT_D1B_C24_EN)\
    CMD(a, b, c, d, e, int, OFT_D1A_C2G_EN, oft.oft_c2g_en, OFT_C2G_EN,, true, MDPS_D1A_OFT_OFT_MODULE_EN, OFT_C2G_EN, 4, OFT_D1B_C2G_EN)\
    CMD(a, b, c, d, e, int, OFT_D1A_IGGM_EN, oft.oft_iggm_en, OFT_IGGM_EN,, true, MDPS_D1A_OFT_OFT_MODULE_EN, OFT_IGGM_EN, 4, OFT_D1B_IGGM_EN)\
    CMD(a, b, c, d, e, int, OFT_D1A_CCM_EN, oft.oft_ccm_en, OFT_CCM_EN,, true, MDPS_D1A_OFT_OFT_MODULE_EN, OFT_CCM_EN, 4, OFT_D1B_CCM_EN)\
    CMD(a, b, c, d, e, int, OFT_D1A_GCE_EN, oft.oft_gce_en, OFT_GCE_EN,, true, MDPS_D1A_OFT_OFT_MODULE_EN, OFT_PGGM_EN, 4, OFT_D1B_GCE_EN)\
    CMD(a, b, c, d, e, int, OFT_D1A_G2C_EN, oft.oft_g2c_en, OFT_G2C_EN,, true, MDPS_D1A_OFT_OFT_MODULE_EN, OFT_G2C_EN, 4, OFT_D1B_G2C_EN)\
    CMD(a, b, c, d, e, int, OFT_D1A_C42_EN, oft.oft_c42_en, OFT_C42_EN,, true, MDPS_D1A_OFT_OFT_MODULE_EN, OFT_C42_EN, 4, OFT_D1B_C42_EN)\
    CMD(a, b, c, d, e, int, OFT_D1A_C20_EN, oft.oft_c20_en, OFT_C20_EN,, true, MDPS_D1A_OFT_OFT_MODULE_EN, OFT_C20_EN, 4, OFT_D1B_C20_EN)\
    CMD(a, b, c, d, e, int, OFT_D1A_R2B_EN, oft.oft_r2b_en, OFT_R2B_EN,, true, MDPS_D1A_OFT_OFT_MODULE_EN, OFT_R2B_EN, 4, OFT_D1B_R2B_EN)\
    CMD(a, b, c, d, e, int, OFT_D1A_HSV_EN, oft.oft_hsv_en, OFT_HSV_EN,, true, MDPS_D1A_OFT_OFT_MODULE_EN, OFT_HSV_EN, 4, OFT_D1B_HSV_EN)\
    CMD(a, b, c, d, e, int, OFT_D1A_LAB_EN, oft.oft_lab_en, OFT_LAB_EN,, true, MDPS_D1A_OFT_OFT_MODULE_EN, OFT_LAB_EN, 4, OFT_D1B_LAB_EN)\
    /* OFT CRZ */\
    CMD(a, b, c, d, e, int, OFT_D1A_CDRZ_crop_size_x_d, oft.oft_crz_input_crop_width, OFT_CRZ_Input_Image_W,, REG_CMP_EQ(c, LOG_OFT_CRZ_EN, 1), MDPS_D1A_OFT_CRZ_IN_IMG, OFT_CDRZ_INPUT_IMAGE_W, 0, OFT_D1B_CDRZ_crop_size_x_d)\
    CMD(a, b, c, d, e, int, OFT_D1A_CDRZ_crop_size_y_d, oft.oft_crz_input_crop_height, OFT_CRZ_Input_Image_H,, REG_CMP_EQ(c, LOG_OFT_CRZ_EN, 1), MDPS_D1A_OFT_CRZ_IN_IMG, OFT_CDRZ_INPUT_IMAGE_H, 0, OFT_D1B_CDRZ_crop_size_y_d)\
    CMD(a, b, c, d, e, int, OFT_D1A_CDRZ_OUTPUT_IMAGE_W, oft.oft_crz_output_width, OFT_CRZ_Output_Image_W,, REG_CMP_EQ(c, LOG_OFT_CRZ_EN, 1), MDPS_D1A_OFT_CRZ_OUT_IMG, OFT_CDRZ_OUTPUT_IMAGE_W, 0, OFT_D1B_CDRZ_OUTPUT_IMAGE_W)\
    CMD(a, b, c, d, e, int, OFT_D1A_CDRZ_OUTPUT_IMAGE_H, oft.oft_crz_output_height, OFT_CRZ_Output_Image_H,, REG_CMP_EQ(c, LOG_OFT_CRZ_EN, 1), MDPS_D1A_OFT_CRZ_OUT_IMG, OFT_CDRZ_OUTPUT_IMAGE_H, 0, OFT_D1B_CDRZ_OUTPUT_IMAGE_H)\
    CMD(a, b, c, d, e, int, OFT_D1A_CDRZ_LUMA_HORIZONTAL_INTEGER_OFFSET, oft.oft_crz_luma_horizontal_integer_offset, OFT_CRZ_Luma_Horizontal_Integer_Offset,, REG_CMP_EQ(c, LOG_OFT_CRZ_EN, 1), MDPS_D1A_OFT_CRZ_LUMA_HORI_INT_OFST, OFT_CDRZ_LUMA_HORIZONTAL_INTEGER_OFFSET, 0, OFT_D1B_CDRZ_LUMA_HORIZONTAL_INTEGER_OFFSET)\
    CMD(a, b, c, d, e, int, OFT_D1A_CDRZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET, oft.oft_crz_luma_horizontal_subpixel_offset, OFT_CRZ_Luma_Horizontal_Subpixel_Offset,, REG_CMP_EQ(c, LOG_OFT_CRZ_EN, 1), MDPS_D1A_OFT_CRZ_LUMA_HORI_SUB_OFST, OFT_CDRZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET, 0, OFT_D1B_CDRZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET)\
    CMD(a, b, c, d, e, int, OFT_D1A_CDRZ_LUMA_VERTICAL_INTEGER_OFFSET, oft.oft_crz_luma_vertical_integer_offset, OFT_CRZ_Luma_Vertical_Integer_Offset,, REG_CMP_EQ(c, LOG_OFT_CRZ_EN, 1), MDPS_D1A_OFT_CRZ_LUMA_VERT_INT_OFST, OFT_CDRZ_LUMA_VERTICAL_INTEGER_OFFSET, 0, OFT_D1B_CDRZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET)\
    CMD(a, b, c, d, e, int, OFT_D1A_CDRZ_LUMA_VERTICAL_SUBPIXEL_OFFSET, oft.oft_crz_luma_vertical_subpixel_offset, OFT_CRZ_Luma_Vertical_Subpixel_Offset,, REG_CMP_EQ(c, LOG_OFT_CRZ_EN, 1), MDPS_D1A_OFT_CRZ_LUMA_VERT_SUB_OFST, OFT_CDRZ_LUMA_VERTICAL_SUBPIXEL_OFFSET, 0, OFT_D1B_CDRZ_LUMA_VERTICAL_SUBPIXEL_OFFSET)\
    CMD(a, b, c, d, e, int, OFT_D1A_CDRZ_HORIZONTAL_LUMA_ALGORITHM, oft.oft_crz_horizontal_luma_algorithm, OFT_CRZ_Horizontal_Luma_Algorithm,, REG_CMP_EQ(c, LOG_OFT_CRZ_EN, 1), MDPS_D1A_OFT_CRZ_CONTROL, OFT_CDRZ_HORIZONTAL_LUMA_ALGORITHM, 4, OFT_D1B_CDRZ_HORIZONTAL_LUMA_ALGORITHM)\
    CMD(a, b, c, d, e, int, OFT_D1A_CDRZ_VERTICAL_LUMA_ALGORITHM, oft.oft_crz_vertical_luma_algorithm, OFT_CRZ_Vertical_Luma_Algorithm,, REG_CMP_EQ(c, LOG_OFT_CRZ_EN, 1), MDPS_D1A_OFT_CRZ_CONTROL, OFT_CDRZ_VERTICAL_LUMA_ALGORITHM, 4, OFT_D1B_CDRZ_VERTICAL_LUMA_ALGORITHM)\
    CMD(a, b, c, d, e, int, OFT_D1A_CDRZ_HORIZONTAL_COEFF_STEP, oft.oft_crz_horizontal_coeff_step, OFT_CRZ_Horizontal_Coeff_Step,, REG_CMP_EQ(c, LOG_OFT_CRZ_EN, 1), MDPS_D1A_OFT_CRZ_HORI_STEP, OFT_CDRZ_HORIZONTAL_COEFF_STEP, 4, OFT_D1B_CDRZ_HORIZONTAL_COEFF_STEP)\
    CMD(a, b, c, d, e, int, OFT_D1A_CDRZ_VERTICAL_COEFF_STEP, oft.oft_crz_vertical_coeff_step, OFT_CRZ_Vertical_Coeff_Step,, REG_CMP_EQ(c, LOG_OFT_CRZ_EN, 1), MDPS_D1A_OFT_CRZ_VERT_STEP, OFT_CDRZ_VERTICAL_COEFF_STEP, 4, OFT_D1B_CDRZ_VERTICAL_COEFF_STEP)\
	/* OFT_C20 */\
    CMD(a, b, c, d, e, int, OFT_D1A_C20_OFST_X, oft.oft_c20_xoffset, OFT_C20_OFST_X,, REG_CMP_EQ(c, LOG_OFT_C20_EN, 1), MDPS_D1A_OFT_C20_CTRL, OFT_C20_OFST_X, 0, OFT_D1B_C20_OFST_X)\
    CMD(a, b, c, d, e, int, OFT_D1A_C20_OFST_Y, oft.oft_c20_yoffset, OFT_C20_OFST_Y,, REG_CMP_EQ(c, LOG_OFT_C20_EN, 1), MDPS_D1A_OFT_C20_CTRL, OFT_C20_OFST_Y, 0, OFT_D1B_C20_OFST_Y)\
    /* OFT R2B */\
    CMD(a, b, c, d, e, int, OFT_D1A_R2B_CH0_PITCH, oft.oft_r2b_ch0_stride, OFT_R2B_CH0_STRIDE,, REG_CMP_EQ(c, LOG_OFT_R2B_EN, 1), MDPS_D1A_OFT_R2B_CH0_PITCH, OFT_R2B_CH0_PITCH, 4, OFT_D1B_R2B_CH0_PITCH)\
    CMD(a, b, c, d, e, int, OFT_D1A_R2B_CH1_PITCH, oft.oft_r2b_ch1_stride, OFT_R2B_CH1_STRIDE,, REG_CMP_EQ(c, LOG_OFT_R2B_EN, 1), MDPS_D1A_OFT_R2B_CH1_PITCH, OFT_R2B_CH1_PITCH, 4, OFT_D1B_R2B_CH1_PITCH)\
    CMD(a, b, c, d, e, int, OFT_D1A_R2B_CH2_PITCH, oft.oft_r2b_ch2_stride, OFT_R2B_CH2_STRIDE,, REG_CMP_EQ(c, LOG_OFT_R2B_EN, 1), MDPS_D1A_OFT_R2B_CH2_PITCH, OFT_R2B_CH2_PITCH, 4, OFT_D1B_R2B_CH2_PITCH)\
    CMD(a, b, c, d, e, int, OFT_D1A_R2B_CROP_XSTART, oft.oft_r2b_xoffset, OFT_R2B_XOFFSET,, REG_CMP_EQ(c, LOG_OFT_R2B_EN, 1), MDPS_D1A_OFT_R2B_CROP_X, OFT_R2B_CROP_XSTART, 0, OFT_D1B_R2B_CROP_XSTART)\
    CMD(a, b, c, d, e, int, OFT_D1A_R2B_CROP_YSTART, oft.oft_r2b_yoffset, OFT_R2B_YOFFSET,, REG_CMP_EQ(c, LOG_OFT_R2B_EN, 1), MDPS_D1A_OFT_R2B_CROP_Y, OFT_R2B_CROP_YEND, 0, OFT_D1B_R2B_CROP_YSTART)\
    CMD(a, b, c, d, e, int, OFT_D1A_R2B_CH0_XSIZE, oft.oft_r2b_xsize, OFT_R2B_XSIZE,, REG_CMP_EQ(c, LOG_OFT_R2B_EN, 1), MDPS_D1A_OFT_R2B_CH0_FRM_SIZE, OFT_R2B_CH0_XSIZE, 0, OFT_D1B_R2B_CH0_XSIZE)\
    CMD(a, b, c, d, e, int, OFT_D1A_R2B_CH0_YSIZE, oft.oft_r2b_ysize, OFT_R2B_YSIZE,, REG_CMP_EQ(c, LOG_OFT_R2B_EN, 1), MDPS_D1A_OFT_R2B_CH0_FRM_SIZE, OFT_R2B_CH0_YSIZE, 0, OFT_D1B_R2B_CH0_YSIZE)\
    CMD(a, b, c, d, e, int, OFT_D1A_TOP_OUT_FORMAT, oft.oft_r2b_format, OFT_R2B_FMT,, true, MDPS_D1A_OFT_OFT_FORMAT, OFT_TOP_OUT_FORMAT, 4, OFT_D1B_TOP_OUT_FORMAT)\
    CMD(a, b, c, d, e, int, OFT_D1A_TOP_OUT_PLANE, oft.oft_r2b_plane, OFT_R2B_PLANE,, true, MDPS_D1A_OFT_OFT_FORMAT, OFT_TOP_OUT_PLANE, 4, OFT_D1B_TOP_OUT_PLANE)\
    CMD(a, b, c, d, e, int, OFT_D1A_TOP_OUT_COMP_MODE, oft.oft_r2b_comp_mode, OFT_R2B_COMP_MODE,, true, MDPS_D1A_OFT_OFT_FORMAT, OFT_TOP_OUT_COMP_MODE, 4, OFT_D1B_TOP_OUT_COMP_MODE)\
    CMD(a, b, c, d, e, int, OFT_D1A_TOP_OUT_COMP_EN, oft.oft_r2b_comp_en, OFT_R2B_COMP_EN,, true, MDPS_D1A_OFT_OFT_FORMAT, OFT_TOP_OUT_COMP_EN, 4, OFT_D1B_TOP_OUT_COMP_EN)\
    CMD(a, b, c, d, e, int, OFT_D1A_TOP_OUT_DEPTH, oft.oft_r2b_out_depth, OFT_R2B_DEPTH,, true, MDPS_D1A_OFT_OFT_FORMAT, OFT_TOP_OUT_DEPTH, 4, OFT_D1B_TOP_OUT_DEPTH)\
    CMD(a, b, c, d, e, int, OFT_D1A_EXT_ISP_FORMAT, oft.oft_ext_isp_format, OFT_EXT_FORMAT,, true, MDPS_D1A_OFT_OFT_EXT_ISP, OFT_EXT_ISP_FORMAT, 4, OFT_D1B_EXT_ISP_FORMAT)\
    /* OFTL */\
    CMD(a, b, c, d, e, int, OFTL_D1A_CROP_EN, oftl.oftl_crop_en, OFTL_CROP_EN,, REG_CMP_EQ(c, LOG_OFTL_CROP_EN, 1), MDPV_D1A_OFTL_OFT_MODULE_EN, OFTL_CROP_EN, 4, OFTL_D1B_CROP_EN)\
    CMD(a, b, c, d, e, int, OFTL_D1A_CRZ_EN, oftl.oftl_crz_en, OFTL_CRZ_EN,, REG_CMP_EQ(c, LOG_OFTL_CRZ_EN, 1), MDPV_D1A_OFTL_OFT_MODULE_EN, OFTL_CRZ_EN, 4, OFTL_D1B_CRZ_EN)\
    CMD(a, b, c, d, e, int, OFTL_D1A_DTH_EN, oftl.oftl_dth_en, OFTL_DTH_EN,, REG_CMP_EQ(c, LOG_OFTL_DTH_EN, 1), MDPV_D1A_OFTL_OFT_MODULE_EN, OFTL_DTH_EN, 4, OFTL_D1B_DTH_EN)\
    CMD(a, b, c, d, e, int, OFTL_D1A_BCVT_EN, oftl.oftl_bcvt_en, OFTL_BCVT_EN,, true, MDPV_D1A_OFTL_OFT_MODULE_EN, OFTL_BCVT_EN, 4, OFTL_D1A_BCVT_EN)\
    CMD(a, b, c, d, e, int, OFTL_D1A_C24_EN, oftl.oftl_c24_en, OFTL_C24_EN,, REG_CMP_EQ(c, LOG_OFTL_C24_EN, 1), MDPV_D1A_OFTL_OFT_MODULE_EN, OFTL_C24_EN, 4, OFTL_D1B_C24_EN)\
    CMD(a, b, c, d, e, int, OFTL_D1A_C2G_EN, oftl.oftl_c2g_en, OFTL_C2G_EN,, REG_CMP_EQ(c, LOG_OFTL_C2G_EN, 1), MDPV_D1A_OFTL_OFT_MODULE_EN, OFTL_C2G_EN, 4, OFTL_D1B_C2G_EN)\
    CMD(a, b, c, d, e, int, OFTL_D1A_IGGM_EN, oftl.oftl_iggm_en, OFTL_IGGM_EN,, REG_CMP_EQ(c, LOG_OFTL_IGGM_EN, 1), MDPV_D1A_OFTL_OFT_MODULE_EN, OFTL_IGGM_EN, 1, OFTL_D1B_IGGM_EN)\
    CMD(a, b, c, d, e, int, OFTL_D1A_CCM_EN, oftl.oftl_ccm_en, OFTL_CCM_EN,, REG_CMP_EQ(c, LOG_OFTL_CCM_EN, 1), MDPV_D1A_OFTL_OFT_MODULE_EN, OFTL_CCM_EN, 4, OFTL_D1B_CCM_EN)\
    CMD(a, b, c, d, e, int, OFTL_D1A_GCE_EN, oftl.oftl_gce_en, OFTL_GCE_EN,, REG_CMP_EQ(c, LOG_OFTL_GCE_EN, 1), MDPV_D1A_OFTL_OFT_MODULE_EN, OFTL_PGGM_EN, 4, OFTL_D1B_GCE_EN)\
    CMD(a, b, c, d, e, int, OFTL_D1A_G2C_EN, oftl.oftl_g2c_en, OFTL_G2C_EN,, REG_CMP_EQ(c, LOG_OFTL_G2C_EN, 1), MDPV_D1A_OFTL_OFT_MODULE_EN, OFTL_G2C_EN, 4, OFTL_D1B_G2C_EN)\
    CMD(a, b, c, d, e, int, OFTL_D1A_C42_EN, oftl.oftl_c42_en, OFTL_C42_EN,, REG_CMP_EQ(c, LOG_OFTL_C42_EN, 1), MDPV_D1A_OFTL_OFT_MODULE_EN, OFTL_C42_EN, 4, OFTL_D1B_C42_EN)\
    CMD(a, b, c, d, e, int, OFTL_D1A_C20_EN, oftl.oftl_c20_en, OFTL_C20_EN,, REG_CMP_EQ(c, LOG_OFTL_C20_EN, 1), MDPV_D1A_OFTL_OFT_MODULE_EN, OFTL_C20_EN, 4, OFTL_D1B_C20_EN)\
    CMD(a, b, c, d, e, int, OFTL_D1A_R2B_EN, oftl.oftl_r2b_en, OFTL_R2B_EN,, REG_CMP_EQ(c, LOG_OFTL_R2B_EN, 1), MDPV_D1A_OFTL_OFT_MODULE_EN, OFTL_R2B_EN, 4, OFTL_D1B_R2B_EN)\
    /* OFTL CRZ */\
    CMD(a, b, c, d, e, int, OFTL_D1A_CDRZ_crop_size_x_d, oftl.oftl_crz_input_crop_width, OFTL_CRZ_Input_Image_W,, REG_CMP_EQ(c, LOG_OFTL_CRZ_EN, 1), MDPV_D1A_OFTL_CRZ_IN_IMG, OFTL_CDRZ_INPUT_IMAGE_W, 0, OFTL_D1B_CDRZ_crop_size_x_d)\
    CMD(a, b, c, d, e, int, OFTL_D1A_CDRZ_crop_size_y_d, oftl.oftl_crz_input_crop_height, OFTL_CRZ_Input_Image_H,, REG_CMP_EQ(c, LOG_OFTL_CRZ_EN, 1), MDPV_D1A_OFTL_CRZ_IN_IMG, OFTL_CDRZ_INPUT_IMAGE_H, 0, OFTL_D1B_CDRZ_crop_size_y_d)\
    CMD(a, b, c, d, e, int, OFTL_D1A_CDRZ_OUTPUT_IMAGE_W, oftl.oftl_crz_output_width, OFTL_CRZ_Output_Image_W,, REG_CMP_EQ(c, LOG_OFTL_CRZ_EN, 1), MDPV_D1A_OFTL_CRZ_OUT_IMG, OFTL_CDRZ_OUTPUT_IMAGE_W, 0, OFTL_D1B_CDRZ_OUTPUT_IMAGE_W)\
    CMD(a, b, c, d, e, int, OFTL_D1A_CDRZ_OUTPUT_IMAGE_H, oftl.oftl_crz_output_height, OFTL_CRZ_Output_Image_H,, REG_CMP_EQ(c, LOG_OFTL_CRZ_EN, 1), MDPV_D1A_OFTL_CRZ_OUT_IMG, OFTL_CDRZ_OUTPUT_IMAGE_H, 0, OFTL_D1B_CDRZ_OUTPUT_IMAGE_H)\
    CMD(a, b, c, d, e, int, OFTL_D1A_CDRZ_LUMA_HORIZONTAL_INTEGER_OFFSET, oftl.oftl_crz_luma_horizontal_integer_offset, OFTL_CRZ_Luma_Horizontal_Integer_Offset,, REG_CMP_EQ(c, LOG_OFTL_CRZ_EN, 1), MDPV_D1A_OFTL_CRZ_LUMA_HORI_INT_OFST, OFTL_CDRZ_LUMA_HORIZONTAL_INTEGER_OFFSET, 0, OFTL_D1B_CDRZ_LUMA_HORIZONTAL_INTEGER_OFFSET)\
    CMD(a, b, c, d, e, int, OFTL_D1A_CDRZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET, oftl.oftl_crz_luma_horizontal_subpixel_offset, OFTL_CRZ_Luma_Horizontal_Subpixel_Offset,, REG_CMP_EQ(c, LOG_OFTL_CRZ_EN, 1), MDPV_D1A_OFTL_CRZ_LUMA_HORI_SUB_OFST, OFTL_CDRZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET, 0, OFTL_D1B_CDRZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET)\
    CMD(a, b, c, d, e, int, OFTL_D1A_CDRZ_LUMA_VERTICAL_INTEGER_OFFSET, oftl.oftl_crz_luma_vertical_integer_offset, OFTL_CRZ_Luma_Vertical_Integer_Offset,, REG_CMP_EQ(c, LOG_OFTL_CRZ_EN, 1), MDPV_D1A_OFTL_CRZ_LUMA_VERT_INT_OFST, OFTL_CDRZ_LUMA_VERTICAL_INTEGER_OFFSET, 0, OFTL_D1B_CDRZ_LUMA_VERTICAL_INTEGER_OFFSET)\
    CMD(a, b, c, d, e, int, OFTL_D1A_CDRZ_LUMA_VERTICAL_SUBPIXEL_OFFSET, oftl.oftl_crz_luma_vertical_subpixel_offset, OFTL_CRZ_Luma_Vertical_Subpixel_Offset,, REG_CMP_EQ(c, LOG_OFTL_CRZ_EN, 1), MDPV_D1A_OFTL_CRZ_LUMA_VERT_SUB_OFST, OFTL_CDRZ_LUMA_VERTICAL_SUBPIXEL_OFFSET, 0, OFTL_D1B_CDRZ_LUMA_VERTICAL_SUBPIXEL_OFFSET)\
    CMD(a, b, c, d, e, int, OFTL_D1A_CDRZ_HORIZONTAL_LUMA_ALGORITHM, oftl.oftl_crz_horizontal_luma_algorithm, OFTL_CRZ_Horizontal_Luma_Algorithm,, REG_CMP_EQ(c, LOG_OFTL_CRZ_EN, 1), MDPV_D1A_OFTL_CRZ_CONTROL, OFTL_CDRZ_HORIZONTAL_LUMA_ALGORITHM, 4, OFTL_D1B_CDRZ_HORIZONTAL_LUMA_ALGORITHM)\
    CMD(a, b, c, d, e, int, OFTL_D1A_CDRZ_VERTICAL_LUMA_ALGORITHM, oftl.oftl_crz_vertical_luma_algorithm, OFTL_CRZ_Vertical_Luma_Algorithm,, REG_CMP_EQ(c, LOG_OFTL_CRZ_EN, 1), MDPV_D1A_OFTL_CRZ_CONTROL, OFTL_CDRZ_VERTICAL_LUMA_ALGORITHM, 4, OFTL_D1B_CDRZ_VERTICAL_LUMA_ALGORITHM)\
    CMD(a, b, c, d, e, int, OFTL_D1A_CDRZ_HORIZONTAL_COEFF_STEP, oftl.oftl_crz_horizontal_coeff_step, OFTL_CRZ_Horizontal_Coeff_Step,, REG_CMP_EQ(c, LOG_OFTL_CRZ_EN, 1), MDPV_D1A_OFTL_CRZ_HORI_STEP, OFTL_CDRZ_HORIZONTAL_COEFF_STEP, 4, OFTL_D1B_CDRZ_HORIZONTAL_COEFF_STEP)\
    CMD(a, b, c, d, e, int, OFTL_D1A_CDRZ_VERTICAL_COEFF_STEP, oftl.oftl_crz_vertical_coeff_step, OFTL_CRZ_Vertical_Coeff_Step,, REG_CMP_EQ(c, LOG_OFTL_CRZ_EN, 1), MDPV_D1A_OFTL_CRZ_VERT_STEP, OFTL_CDRZ_VERTICAL_COEFF_STEP, 4, OFTL_D1B_CDRZ_VERTICAL_COEFF_STEP)\
	/* OFTL_C20 */\
    CMD(a, b, c, d, e, int, OFTL_D1A_C20_OFST_X, oftl.oftl_c20_xoffset, OFTL_C20_OFST_X,, REG_CMP_EQ(c, LOG_OFT_C20_EN, 1), MDPV_D1A_OFTL_C20_CTRL, OFTL_C20_OFST_X, 0, OFT_D1B_C20_OFST_X)\
    CMD(a, b, c, d, e, int, OFTL_D1A_C20_OFST_Y, oftl.oftl_c20_yoffset, OFTL_C20_OFST_Y,, REG_CMP_EQ(c, LOG_OFT_C20_EN, 1), MDPV_D1A_OFTL_C20_CTRL, OFTL_C20_OFST_Y, 0, OFT_D1B_C20_OFST_Y)\
    /* OFTL R2B */\
    CMD(a, b, c, d, e, int, OFTL_D1A_R2B_CH0_PITCH, oftl.oftl_r2b_ch0_stride, OFTL_R2B_CH0_STRIDE,, REG_CMP_EQ(c, LOG_OFTL_R2B_EN, 1), MDPV_D1A_OFTL_R2B_CH0_PITCH, OFTL_R2B_CH0_PITCH, 4, OFTL_D1B_R2B_CH0_PITCH)\
    CMD(a, b, c, d, e, int, OFTL_D1A_R2B_CH1_PITCH, oftl.oftl_r2b_ch1_stride, OFTL_R2B_CH1_STRIDE,, REG_CMP_EQ(c, LOG_OFTL_R2B_EN, 1), MDPV_D1A_OFTL_R2B_CH1_PITCH, OFTL_R2B_CH1_PITCH, 4, OFTL_D1B_R2B_CH1_PITCH)\
    CMD(a, b, c, d, e, int, OFTL_D1A_R2B_CROP_XSTART, oftl.oftl_r2b_xoffset, OFTL_R2B_XOFFSET,, REG_CMP_EQ(c, LOG_OFTL_R2B_EN, 1), MDPV_D1A_OFTL_R2B_CROP_X, OFTL_R2B_CROP_XSTART, 0, OFTL_D1B_R2B_CROP_XSTART)\
    CMD(a, b, c, d, e, int, OFTL_D1A_R2B_CROP_YSTART, oftl.oftl_r2b_yoffset, OFTL_R2B_YOFFSET,, REG_CMP_EQ(c, LOG_OFTL_R2B_EN, 1), MDPV_D1A_OFTL_R2B_CROP_Y, OFTL_R2B_CROP_YSTART, 0, OFTL_D1B_R2B_CROP_YSTART)\
    CMD(a, b, c, d, e, int, OFTL_D1A_R2B_CH0_XSIZE, oftl.oftl_r2b_xsize, OFTL_R2B_XSIZE,, REG_CMP_EQ(c, LOG_OFTL_R2B_EN, 1), MDPV_D1A_OFTL_R2B_CH0_FRM_SIZE, OFTL_R2B_CH0_XSIZE, 0, OFTL_D1B_R2B_CH0_XSIZE)\
    CMD(a, b, c, d, e, int, OFTL_D1A_R2B_CH0_YSIZE, oftl.oftl_r2b_ysize, OFTL_R2B_YSIZE,, REG_CMP_EQ(c, LOG_OFTL_R2B_EN, 1), MDPV_D1A_OFTL_R2B_CH0_FRM_SIZE, OFTL_R2B_CH0_YSIZE, 0, OFTL_D1B_R2B_CH0_YSIZE)\
    CMD(a, b, c, d, e, int, OFTL_D1A_TOP_OUT_FORMAT, oftl.oftl_r2b_format, OFTL_R2B_FMT,, true, MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_FORMAT, 4, OFTL_D1B_TOP_OUT_FORMAT)\
    CMD(a, b, c, d, e, int, OFTL_D1A_TOP_OUT_PLANE, oftl.oftl_r2b_plane, OFTL_R2B_PLANE,, true, MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_PLANE, 4, OFTL_D1B_TOP_OUT_PLANE)\
    CMD(a, b, c, d, e, int, OFTL_D1A_TOP_OUT_COMP_MODE, oftl.oftl_r2b_comp_mode, OFTL_R2B_COMP_MODE,, true, MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_COMP_MODE, 4, OFTL_D1B_TOP_OUT_COMP_MODE)\
    CMD(a, b, c, d, e, int, OFTL_D1A_TOP_OUT_COMP_EN, oftl.oftl_r2b_comp_en, OFTL_R2B_COMP_EN,, true, MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_COMP_EN, 4, OFTL_D1B_TOP_OUT_COMP_EN)\
    CMD(a, b, c, d, e, int, OFTL_D1A_TOP_OUT_DEPTH, oftl.oftl_r2b_out_depth, OFTL_R2B_DEPTH,, true, MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_DEPTH, 4, OFTL_D1B_TOP_OUT_DEPTH)\

/* register table (Cmodel, platform, tile driver) for HW parameters */
/* a, b, c, d, e reserved */
/* data type */
/* register name of current c model */
/* register name of HW ISP & platform parameters */
/* internal variable name of tile */
/* array bracket [xx] */
/* valid condition by tdr_en to print platform log with must string, default: false */
/* isp_reg.h reg name */
/* isp_reg.h field name */
/* direct-link param 0: must be equal, 1: replaced by MDP, 2: don't care, 4: shold compare isp_reg and reg map in program */
#define WPE_TILE_HW_REG_LUT(CMD, a, b, c, d, e) \

/* register table (Cmodel, platform, tile driver) for HW parameters */
/* a, b, c, d, e reserved */
/* data type */
/* register name of current c model */
/* register name of HW ISP & platform parameters */
/* internal variable name of tile */
/* array bracket [xx] */
/* valid condition by tdr_en to print platform log with must string, default: false */
/* isp_reg.h reg name */
/* isp_reg.h field name */
/* direct-link param 0: must be equal, 1: replaced by MDP, 2: don't care, 4: shold compare isp_reg and reg map in program */
#define MFB_TILE_HW_REG_LUT(CMD, a, b, c, d, e) \
    /* Common */\
	CMD(a, b, c, d, e, int, MFB_D1A_MFB_MFBI_EN, top.mfbi_en, MFBI_EN,, true, MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_DEPTH, 4, MFB_D1B_MFB_MFBI_EN)\
	CMD(a, b, c, d, e, int, MFB_D1A_MFB_MFBI_B_EN, top.mfbi_b_en, MFBI_B_EN,, true, MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_DEPTH, 4, MFB_D1B_MFB_MFBI_B_EN)\
	CMD(a, b, c, d, e, int, MFB_D1A_MFB_MFB3I_EN, top.mfb3i_en, MFB3I_EN,, true, MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_DEPTH, 4, MFB_D1B_MFB_MFB3I_EN)\
	CMD(a, b, c, d, e, int, MFB_D1A_MFB_MFB4I_EN, top.mfb4i_en, MFB4I_EN,, true, MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_DEPTH, 4, MFB_D1B_MFB_MFB4I_EN)\
	CMD(a, b, c, d, e, int, MFB_D1A_MFB_EN, top.mfb_en, MFB_EN,, true, MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_DEPTH, 4, MFB_D1B_MFB_EN)\
	CMD(a, b, c, d, e, int, MFB_D1A_MFB_MFBO_EN, top.mfbo_en, MFBO_EN,, true, MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_DEPTH, 4, MFB_D1B_MFB_MFBO_EN)\
	CMD(a, b, c, d, e, int, MFB_D1A_MFB_MFBO_B_EN, top.mfbo_b_en, MFBO_B_EN,, true, MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_DEPTH, 4, MFB_D1B_MFB_MFBO_B_EN)\
	CMD(a, b, c, d, e, int, MFB_D1A_MFB_MFB2O_EN, top.mfb2o_en, MFB2O_EN,, true, MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_DEPTH, 4, MFB_D1B_MFB_MFB2O_EN)\
    CMD(a, b, c, d, e, int, MFB_D1A_MFB_SRZ_EN, top.mfb_srz_en, MFB_SRZ_EN,, true, MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_DEPTH, 4, MFB_D1B_MFB_SRZ_EN)\
	/* MFB_DMA - MFBI */\
    CMD(a, b, c, d, e, int, MFBDMA_D1A_MFBI_V_FLIP_EN, mfbi.mfbi_v_flip_en, MFBI_V_FLIP_EN,, REG_CMP_EQ(c, MFBI_EN, 1), MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_DEPTH, 4, MFBDMA_D1B_MFBI_V_FLIP_EN)\
    CMD(a, b, c, d, e, int, MFBDMA_D1A_MFBI_STRIDE, mfbi.mfbi_stride, MFBI_STRIDE,, REG_CMP_EQ(c, MFBI_EN, 1), MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_DEPTH, 4, MFBDMA_D1B_MFBI_STRIDE)\
    CMD(a, b, c, d, e, int, MFBDMA_D1A_MFBI_B_V_FLIP_EN, mfbi_b.mfbi_b_v_flip_en, MFBI_B_V_FLIP_EN,, REG_CMP_EQ(c, MFBI_B_EN, 1), MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_DEPTH, 4, MFBDMA_D1B_MFBI_B_V_FLIP_EN)\
    CMD(a, b, c, d, e, int, MFBDMA_D1A_MFBI_B_STRIDE, mfbi_b.mfbi_b_stride, MFBI_B_STRIDE,, REG_CMP_EQ(c, MFBI_B_EN, 1), MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_DEPTH, 4, MFBDMA_D1B_MFBI_B_STRIDE)\
    CMD(a, b, c, d, e, int, MFBDMA_D1A_MFB3I_V_FLIP_EN, mfb3i.mfb3i_v_flip_en, MFB3I_V_FLIP_EN,, REG_CMP_EQ(c, MFB3I_EN, 1), MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_DEPTH, 4, MFBDMA_D1B_MFB3I_V_FLIP_EN)\
    CMD(a, b, c, d, e, int, MFBDMA_D1A_MFB3I_STRIDE, mfb3i.mfb3i_stride, MFB3I_STRIDE,, REG_CMP_EQ(c, MFB3I_EN, 1), MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_DEPTH, 4, MFBDMA_D1B_MFB3I_STRIDE)\
    CMD(a, b, c, d, e, int, MFBDMA_D1A_MFB4I_V_FLIP_EN, mfb4i.mfb4i_v_flip_en, MFB4I_V_FLIP_EN,, REG_CMP_EQ(c, MFB4I_EN, 1), MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_DEPTH, 4, MFBDMA_D1B_MFB4I_V_FLIP_EN)\
    CMD(a, b, c, d, e, int, MFBDMA_D1A_MFB4I_XSIZE, mfb4i.mfb4i_xsize, MFB4I_XSIZE,, REG_CMP_EQ(c, MFB4I_EN, 1), MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_DEPTH, 0, MFBDMA_D1B_MFB4I_XSIZE)\
    CMD(a, b, c, d, e, int, MFBDMA_D1A_MFB4I_YSIZE, mfb4i.mfb4i_ysize, MFB4I_YSIZE,, REG_CMP_EQ(c, MFB4I_EN, 1), MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_DEPTH, 0, MFBDMA_D1B_MFB4I_YSIZE)\
    CMD(a, b, c, d, e, int, MFBDMA_D1A_MFB4I_STRIDE, mfb4i.mfb4i_stride, MFB4I_STRIDE,, REG_CMP_EQ(c, MFB4I_EN, 1), MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_DEPTH, 4, MFBDMA_D1B_MFB4I_STRIDE)\
	/* MFB_SRZ */\
    CMD(a, b, c, d, e, int, MFB_D1A_SRZ_IN_CROP_WD, mfb_srz.srz_input_crop_width, MFB_SRZ_Input_Image_W,, REG_CMP_EQ(c, MFB_SRZ_EN, 1), MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_DEPTH, 0, MFB_SRZ_A_IN_CROP_WD)\
    CMD(a, b, c, d, e, int, MFB_D1A_SRZ_IN_CROP_HT, mfb_srz.srz_input_crop_height, MFB_SRZ_Input_Image_H,, REG_CMP_EQ(c, MFB_SRZ_EN, 1), MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_DEPTH, 0, MFB_SRZ_A_IN_CROP_HT)\
    CMD(a, b, c, d, e, int, MFB_D1A_SRZ_OUT_WD, mfb_srz.srz_output_width, MFB_SRZ_Output_Image_W,, REG_CMP_EQ(c, MFB_SRZ_EN, 1), MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_DEPTH, 0, MFB_SRZ_A_OUT_WD)\
    CMD(a, b, c, d, e, int, MFB_D1A_SRZ_OUT_HT, mfb_srz.srz_output_height, MFB_SRZ_Output_Image_H,, REG_CMP_EQ(c, MFB_SRZ_EN, 1), MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_DEPTH, 0, MFB_SRZ_A_OUT_HT)\
    CMD(a, b, c, d, e, int, MFB_D1A_SRZ_HORI_INT_OFST, mfb_srz.srz_luma_horizontal_integer_offset, MFB_SRZ_Luma_Horizontal_Integer_Offset,, REG_CMP_EQ(c, MFB_SRZ_EN, 1), MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_DEPTH, 0, MFB_SRZ_A_HORI_INT_OFST)\
    CMD(a, b, c, d, e, int, MFB_D1A_SRZ_HORI_SUB_OFST, mfb_srz.srz_luma_horizontal_subpixel_offset, MFB_SRZ_Luma_Horizontal_Subpixel_Offset,, REG_CMP_EQ(c, MFB_SRZ_EN, 1), MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_DEPTH, 0, MFB_SRZ_A_HORI_SUB_OFST)\
    CMD(a, b, c, d, e, int, MFB_D1A_SRZ_VERT_INT_OFST, mfb_srz.srz_luma_vertical_integer_offset, MFB_SRZ_Luma_Vertical_Integer_Offset,, REG_CMP_EQ(c, MFB_SRZ_EN, 1), MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_DEPTH, 0, MFB_SRZ_A_VERT_INT_OFST)\
    CMD(a, b, c, d, e, int, MFB_D1A_SRZ_VERT_SUB_OFST, mfb_srz.srz_luma_vertical_subpixel_offset, MFB_SRZ_Luma_Vertical_Subpixel_Offset,, REG_CMP_EQ(c, MFB_SRZ_EN, 1), MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_DEPTH, 0, MFB_SRZ_A_VERT_SUB_OFST)\
    CMD(a, b, c, d, e, int, MFB_D1A_SRZ_HORI_STEP, mfb_srz.srz_horizontal_coeff_step, MFB_SRZ_Horizontal_Coeff_Step,, REG_CMP_EQ(c, MFB_SRZ_EN, 1), MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_DEPTH, 4, MFB_SRZ_A_HORI_STEP)\
    CMD(a, b, c, d, e, int, MFB_D1A_SRZ_VERT_STEP, mfb_srz.srz_vertical_coeff_step, MFB_SRZ_Vertical_Coeff_Step,, REG_CMP_EQ(c, MFB_SRZ_EN, 1), MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_DEPTH, 4, MFB_SRZ_A_VERT_STEP)\
    /* MFB_DMA - MFBO */\
    CMD(a, b, c, d, e, int, MFBDMA_D1A_MFBO_STRIDE, mfbo.mfbo_stride, MFBO_STRIDE,, REG_CMP_EQ(c, MFBO_EN, 1), MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_DEPTH, 4, MFBDMA_D1B_MFBO_STRIDE)\
    /* MFB_DMA - MFBO_B */\
    CMD(a, b, c, d, e, int, MFBDMA_D1A_MFBO_B_STRIDE, mfbo_b.mfbo_b_stride, MFBO_B_STRIDE,, REG_CMP_EQ(c, MFBO_B_EN, 1), MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_DEPTH, 4, MFBDMA_D1B_MFBO_B_STRIDE)\
    /* MFB_DMA - MFB2O */\
    CMD(a, b, c, d, e, int, MFBDMA_D1A_MFB2O_STRIDE, mfb2o.mfb2o_stride, MFB2O_STRIDE,, REG_CMP_EQ(c, MFB2O_EN, 1), MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_DEPTH, 4, MFBDMA_D1B_MFB2O_STRIDE)\
	/* MFB_CRSP */\
    CMD(a, b, c, d, e, int, MFB_D1A_CRSP_STEP_Y, mfb_crsp.crsp_ystep, MFB_CRSP_STEP_Y,, REG_CMP_EQ(c, MFB_EN, 1), MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_DEPTH, 4, MFB_D1B_CRSP_STEP_Y)\
    CMD(a, b, c, d, e, int, MFB_D1A_CRSP_OFST_X, mfb_crsp.crsp_xoffset, MFB_CRSP_OFST_X,, REG_CMP_EQ(c, MFB_EN, 1), MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_DEPTH, 0, MFB_D1B_CRSP_OFST_X)\
    CMD(a, b, c, d, e, int, MFB_D1A_CRSP_OFST_Y, mfb_crsp.crsp_yoffset, MFB_CRSP_OFST_Y,, REG_CMP_EQ(c, MFB_EN, 1), MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_DEPTH, 0, MFB_D1B_CRSP_OFST_Y)\
	/* MFB_Y_UNP */\
	CMD(a, b, c, d, e, int, MFB_Y_UNP_YUV_BIT, mfb_y_unp.yuv_bit, MFB_Y_UNP_YUV_BIT,, REG_CMP_EQ(c, MFBI_EN, 1), MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_DEPTH, 4, MFB_Y_UNP_YUV_BIT)\
	CMD(a, b, c, d, e, int, MFB_Y_UNP_YUV_DNG, mfb_y_unp.yuv_dng, MFB_Y_UNP_YUV_DNG,, REG_CMP_EQ(c, MFBI_EN, 1), MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_DEPTH, 4, MFB_Y_UNP_YUV_DNG)\
	/* MFB_C_UNP */\
	CMD(a, b, c, d, e, int, MFB_C_UNP_YUV_BIT, mfb_c_unp.yuv_bit, MFB_C_UNP_YUV_BIT,, REG_CMP_EQ(c, MFBI_B_EN, 1), MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_DEPTH, 4, MFB_C_UNP_YUV_BIT)\
	CMD(a, b, c, d, e, int, MFB_C_UNP_YUV_DNG, mfb_c_unp.yuv_dng, MFB_C_UNP_YUV_DNG,, REG_CMP_EQ(c, MFBI_B_EN, 1), MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_DEPTH, 4, MFB_C_UNP_YUV_DNG)\
	/* MFB_Y_PAK */\
	CMD(a, b, c, d, e, int, MFB_Y_PAK_YUV_BIT, mfb_y_pak.yuv_bit, MFB_Y_PAK_YUV_BIT,, REG_CMP_EQ(c, MFBO_EN, 1), MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_DEPTH, 4, MFB_Y_PAK_YUV_BIT)\
	CMD(a, b, c, d, e, int, MFB_Y_PAK_YUV_DNG, mfb_y_pak.yuv_dng, MFB_Y_PAK_YUV_DNG,, REG_CMP_EQ(c, MFBO_EN, 1), MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_DEPTH, 4, MFB_Y_PAK_YUV_DNG)\
	/* MFB_C_PAK */\
	CMD(a, b, c, d, e, int, MFB_C_PAK_YUV_BIT, mfb_c_pak.yuv_bit, MFB_C_PAK_YUV_BIT,, REG_CMP_EQ(c, MFBO_B_EN, 1), MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_DEPTH, 4, MFB_C_PAK_YUV_BIT)\
	CMD(a, b, c, d, e, int, MFB_C_PAK_YUV_DNG, mfb_c_pak.yuv_dng, MFB_C_PAK_YUV_DNG,, REG_CMP_EQ(c, MFBO_B_EN, 1), MDPV_D1A_OFTL_OFT_FORMAT, OFTL_TOP_OUT_DEPTH, 4, MFB_C_PAK_YUV_DNG)\

/* register table (Cmodel, , tile driver) for Cmodel only parameters */
/* a, b, c, d, e reserved */
/* data type */
/* register name of current c model */
/* register name of HW ISP & platform parameters */
/* internal variable name of tile */
/* array bracket [xx] */
/* valid condition by tdr_en to print platform log with must string, default: false */
/* isp_reg.h reg name */
/* isp_reg.h field name */
/* direct-link param 0: must be equal, 1: replaced by MDP, 2: don't care */
#define ISP_TILE_CMODEL_PATH_LUT(CMD, a, b, c, d, e) \
    /* to add register only support by c model */\
    CMD(a, b, c, d, e, char *, ptr_tcm_dir_name,, data_path_ptr,,,,,,)\

/* register table (Cmodel, , tile driver) for Cmodel only parameters */
/* a, b, c, d, e reserved */
/* data type */
/* register name of current c model */
/* register name of HW ISP & platform parameters */
/* internal variable name of tile */
/* array bracket [xx] */
/* valid condition by tdr_en to print platform log with must string, default: false */
/* isp_reg.h reg name */
/* isp_reg.h field name */
/* direct-link param 0: must be equal, 1: replaced by MDP, 2: don't care */
#define ISP_TILE_CMODEL_REG_LUT(CMD, a, b, c, d, e) \
    /* to add register only support by c model */\
    CMD(a, b, c, d, e, int, TDRI_A_BASE_ADDR,, TDRI_A_BASE_ADDR,,,,,,)\
    CMD(a, b, c, d, e, int, TDRI_B_BASE_ADDR,, TDRI_B_BASE_ADDR,,,,,,)\
    CMD(a, b, c, d, e, int, WPE_TDRI_A_BASE_ADDR,, WPE_TDRI_A_BASE_ADDR,,,,,,)\
    CMD(a, b, c, d, e, int, WPE_TDRI_B_BASE_ADDR,, WPE_TDRI_B_BASE_ADDR,,,,,,)\
    CMD(a, b, c, d, e, int, MFB_TDRI_A_BASE_ADDR,, MFB_TDRI_A_BASE_ADDR,,,,,,)\
	/* FCSIM */\
    CMD(a, b, c, d, e, int, FCSIM_DL_FUNC_NUM,, FCSIM_DL_FUNC_NUM,,,,,,)\
    CMD(a, b, c, d, e, int, FCSIM_DL_FRAME_WIDTH,, FCSIM_DL_FRAME_WIDTH,,,,,,)\
    CMD(a, b, c, d, e, int, FCSIM_DL_FRAME_HEIGHT,, FCSIM_DL_FRAME_HEIGHT,,,,,,)\
    CMD(a, b, c, d, e, int, FCSIM_DL_TILE_H_NUM,, FCSIM_DL_TILE_H_NUM,,,,,,)\
    CMD(a, b, c, d, e, int, FCSIM_DL_FIRST_TILE_WIDTH,, FCSIM_DL_FIRST_TILE_WIDTH,,,,,,)\
    CMD(a, b, c, d, e, int, FCSIM_DL_TILE_WIDTH,, FCSIM_DL_TILE_WIDTH,,,,,,)\
    CMD(a, b, c, d, e, int, FCSIM_DL_LAST_TILE_WIDTH,, FCSIM_DL_LAST_TILE_WIDTH,,,,,,)\
    CMD(a, b, c, d, e, int, FCSIM_DL_TILE_V_NUM,, FCSIM_DL_TILE_V_NUM,,,,,,)\
    CMD(a, b, c, d, e, int, FCSIM_DL_FIRST_TILE_HEIGHT,, FCSIM_DL_FIRST_TILE_HEIGHT,,,,,,)\
    CMD(a, b, c, d, e, int, FCSIM_DL_TILE_HEIGHT,, FCSIM_DL_TILE_HEIGHT,,,,,,)\
    CMD(a, b, c, d, e, int, FCSIM_DL_LAST_TILE_HEIGHT,, FCSIM_DL_LAST_TILE_HEIGHT,,,,,,)\

/* register table ( , platform, tile driver) for Platform only parameters */
/* a, b, c, d, e reserved */
/* data type */
/* register name of current c model */
/* register name of HW ISP & platform parameters */
/* internal variable name of tile */
/* array bracket [xx] */
/* valid condition by tdr_en to print platform log with must string, default: false */
/* isp_reg.h reg name */
/* isp_reg.h field name */
/* direct-link param 0: must be equal, 1: replaced by MDP, 2: don't care */
#define ISP_TILE_PLATFORM_REG_LUT(CMD, a, b, c, d, e) \
    /* to add register only support by platform */\
    CMD(a, b, c, d, e, int,, sw.log_en, platform_log_en,, true,,,,)\

/* register table ( , , tile driver) for tile driver only parameters */
/* a, b, c, d, e reserved */
/* data type */
/* register name of current c model */
/* register name of HW ISP & platform parameters */
/* internal variable name of tile */
/* array bracket [xx] */
/* valid condition by tdr_en to print platform log with must string, default: false */
/* isp_reg.h reg name */
/* isp_reg.h field name */
/* direct-link param 0: must be equal, 1: replaced by MDP, 2: don't care */
#define ISP_TILE_PLATFORM_DEBUG_REG_LUT(CMD, a, b, c, d, e) \
    CMD(a, b, c, d, e, int,,, platform_buffer_size,,,,,,)\
    CMD(a, b, c, d, e, int,,, platform_max_tpipe_no,,,,,,)\
    CMD(a, b, c, d, e, int,,, platform_isp_hex_no_per_tpipe,,,,,,)\
    CMD(a, b, c, d, e, int,,, platform_isp_hex_no_per_tpipe_wpe,,,,,,)\
    CMD(a, b, c, d, e, int,,, platform_isp_hex_no_per_tpipe_mfb,,,,,,)\
    CMD(a, b, c, d, e, int,,, platform_error_no,,,,,,)\

/* register table ( , , tile driver) for tile driver only parameters */
/* a, b, c, d, e reserved */
/* data type */
/* register name of current c model */
/* register name of HW ISP & platform parameters */
/* internal variable name of tile */
/* array bracket [xx] */
/* valid condition by tdr_en to print platform log with must string, default: false */
/* isp_reg.h reg name */
/* isp_reg.h field name */
/* direct-link param 0: must be equal, 1: replaced by MDP, 2: don't care */
#define ISP_TILE_INTERNAL_TDR_REG_LUT(CMD, a, b, c, d, e) \
    /* tdr used only */\
    /* IMGI_D1 */\
    CMD(a, b, c, d, e, int,,, IMGI_TILE_OFFSET_ADDR,,,,,,)\
    CMD(a, b, c, d, e, int,,, IMGI_TILE_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, IMGI_TILE_YSIZE,,,,,,)\
    /* IMGBI_D1 */\
    CMD(a, b, c, d, e, int,,, IMGBI_TILE_OFFSET_ADDR,,,,,,)\
    CMD(a, b, c, d, e, int,,, IMGBI_TILE_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, IMGBI_TILE_YSIZE,,,,,,)\
    /* IMGCI_D1 */\
    CMD(a, b, c, d, e, int,,, IMGCI_TILE_OFFSET_ADDR,,,,,,)\
    CMD(a, b, c, d, e, int,,, IMGCI_TILE_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, IMGCI_TILE_YSIZE,,,,,,)\
    /* VIPI_D1 */\
    CMD(a, b, c, d, e, int,,, VIPI_TILE_OFFSET_ADDR,,,,,,)\
    CMD(a, b, c, d, e, int,,, VIPI_TILE_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, VIPI_TILE_YSIZE,,,,,,)\
    /* VIPBI_D1 */\
    CMD(a, b, c, d, e, int,,, VIPBI_TILE_OFFSET_ADDR,,,,,,)\
    CMD(a, b, c, d, e, int,,, VIPBI_TILE_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, VIPBI_TILE_YSIZE,,,,,,)\
    /* VIPCI_D1 */\
    CMD(a, b, c, d, e, int,,, VIPCI_TILE_OFFSET_ADDR,,,,,,)\
    CMD(a, b, c, d, e, int,,, VIPCI_TILE_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, VIPCI_TILE_YSIZE,,,,,,)\
    /* UFDI_D1 */\
    CMD(a, b, c, d, e, int,,, UFDI_TILE_OFFSET_ADDR,,,,,,)\
    CMD(a, b, c, d, e, int,,, UFDI_TILE_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, UFDI_TILE_YSIZE,,,,,,)\
    /* UNP_D1 */\
    CMD(a, b, c, d, e, int,,, UNP_OFST_STB,,,,,,)\
    CMD(a, b, c, d, e, int,,, UNP_OFST_EDB,,,,,,)\
    /* UNP_D2 */\
    CMD(a, b, c, d, e, int,,, UNP2_OFST_STB,,,,,,)\
    CMD(a, b, c, d, e, int,,, UNP2_OFST_EDB,,,,,,)\
    /* UNP_D3 */\
    CMD(a, b, c, d, e, int,,, UNP3_OFST_STB,,,,,,)\
    CMD(a, b, c, d, e, int,,, UNP3_OFST_EDB,,,,,,)\
    /* UNP_D4 */\
    CMD(a, b, c, d, e, int,,, UNP4_OFST_STB,,,,,,)\
    CMD(a, b, c, d, e, int,,, UNP4_OFST_EDB,,,,,,)\
    /* UNP_D6 */\
    CMD(a, b, c, d, e, int,,, UNP6_OFST_STB,,,,,,)\
    CMD(a, b, c, d, e, int,,, UNP6_OFST_EDB,,,,,,)\
    /* UNP_D7 */\
    CMD(a, b, c, d, e, int,,, UNP7_OFST_STB,,,,,,)\
    CMD(a, b, c, d, e, int,,, UNP7_OFST_EDB,,,,,,)\
    /* UNP_D8 */\
    CMD(a, b, c, d, e, int,,, UNP8_OFST_STB,,,,,,)\
    CMD(a, b, c, d, e, int,,, UNP8_OFST_EDB,,,,,,)\
    /* UNP_D9 */\
    CMD(a, b, c, d, e, int,,, UNP9_OFST_STB,,,,,,)\
    CMD(a, b, c, d, e, int,,, UNP9_OFST_EDB,,,,,,)\
    /* UNP_D10 */\
    CMD(a, b, c, d, e, int,,, UNP10_OFST_STB,,,,,,)\
    CMD(a, b, c, d, e, int,,, UNP10_OFST_EDB,,,,,,)\
    /* UNP_D11 */\
    CMD(a, b, c, d, e, int,,, UNP11_OFST_STB,,,,,,)\
    CMD(a, b, c, d, e, int,,, UNP11_OFST_EDB,,,,,,)\
    /* UFD_D1 */\
    CMD(a, b, c, d, e, int,,, UFD_WD,,,,,,)\
    CMD(a, b, c, d, e, int,,, UFD_HT,,,,,,)\
    CMD(a, b, c, d, e, int,,, UFD_X_START,,,,,,)\
    CMD(a, b, c, d, e, int,,, UFD_X_END,,,,,,)\
    CMD(a, b, c, d, e, int,,, UFD_Y_START,,,,,,)\
    CMD(a, b, c, d, e, int,,, UFD_Y_END,,,,,,)\
    CMD(a, b, c, d, e, int,,, UFD_AU_SIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, UFD_AU_OFST,,,,,,)\
    CMD(a, b, c, d, e, int,,, UFD_AU2_SIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, UFD_AU2_OFST,,,,,,)\
    CMD(a, b, c, d, e, int,,, UFD_AU3_SIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, UFD_AU3_OFST,,,,,,)\
    CMD(a, b, c, d, e, int,,, UFD_TILE_BOND_MODE,,,,,,)\
    CMD(a, b, c, d, e, int,,, UFD_TILE_BOND2_MODE,,,,,,)\
    /* HLR_D1 */\
    CMD(a, b, c, d, e, int,,, HLR_TILE_EDGE,,,,,,)\
    CMD(a, b, c, d, e, int,,, HLR_TILE_IN_WD,,,,,,)\
    CMD(a, b, c, d, e, int,,, HLR_TILE_IN_HT,,,,,,)\
    /* LTM_D1 */\
    CMD(a, b, c, d, e, int,,, LTM_TILE_EDGE,,,,,,)\
    CMD(a, b, c, d, e, int,,, LTM_TILE_IN_WIDTH,,,,,,)\
    CMD(a, b, c, d, e, int,,, LTM_TILE_IN_HEIGHT,,,,,,)\
    CMD(a, b, c, d, e, int,,, LTM_TILE_BLK_XNUM_STR,,,,,,)\
    CMD(a, b, c, d, e, int,,, LTM_TILE_BLK_XNUM_END,,,,,,)\
    CMD(a, b, c, d, e, int,,, LTM_TILE_BLK_YNUM_STR,,,,,,)\
    CMD(a, b, c, d, e, int,,, LTM_TILE_BLK_YNUM_END,,,,,,)\
    CMD(a, b, c, d, e, int,,, LTM_TILE_BLK_XCNT_STR,,,,,,)\
    CMD(a, b, c, d, e, int,,, LTM_TILE_BLK_XCNT_END,,,,,,)\
    CMD(a, b, c, d, e, int,,, LTM_TILE_BLK_YCNT_STR,,,,,,)\
    CMD(a, b, c, d, e, int,,, LTM_TILE_BLK_YCNT_END,,,,,,)\
    /* DM_D1 */\
    CMD(a, b, c, d, e, int,,, DM_TILE_EDGE,,,,,,)\
    /* LDNR_D1 */\
    CMD(a, b, c, d, e, int,,, LDNR_TILE_EDGE,,,,,,)\
    /* LSC_D1 */\
    CMD(a, b, c, d, e, int,,, LSC_TILE_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, LSC_TILE_YSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, LSC_TILE_XOFST,,,,,,)\
    CMD(a, b, c, d, e, int,,, LSC_TILE_YOFST,,,,,,)\
    CMD(a, b, c, d, e, int,,, LSC_XNUM,,,,,,)\
    CMD(a, b, c, d, e, int,,, LSC_YNUM,,,,,,)\
    CMD(a, b, c, d, e, int,,, LSC_LWIDTH,,,,,,)\
    CMD(a, b, c, d, e, int,,, LSC_LHEIGHT,,,,,,)\
	/* DMGI_D1 */\
    CMD(a, b, c, d, e, int,,, DMGI_TILE_OFFSET_ADDR,,,,,,)\
    CMD(a, b, c, d, e, int,,, DMGI_TILE_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, DMGI_TILE_YSIZE,,,,,,)\
	/* DEPI_D1 */\
    CMD(a, b, c, d, e, int,,, DEPI_TILE_OFFSET_ADDR,,,,,,)\
    CMD(a, b, c, d, e, int,,, DEPI_TILE_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, DEPI_TILE_YSIZE,,,,,,)\
    /* TIMGO_D1 */\
    CMD(a, b, c, d, e, int,,, TIMGO_TILE_OFFSET_ADDR,,,,,,)\
    CMD(a, b, c, d, e, int,,, TIMGO_TILE_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, TIMGO_TILE_YSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, TIMGO_TILE_XOFFSET,,,,,,)\
    CMD(a, b, c, d, e, int,,, TIMGO_TILE_YOFFSET,,,,,,)\
    CMD(a, b, c, d, e, int,,, TIMGO_TILE_FULL_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, TIMGO_TILE_FULL_YSIZE,,,,,,)\
    /* G2CX_D1 */\
    CMD(a, b, c, d, e, int,,, G2CX_TILE_SHADE_XMID,,,,,,)\
    CMD(a, b, c, d, e, int,,, G2CX_TILE_SHADE_YMID,,,,,,)\
    CMD(a, b, c, d, e, int,,, G2CX_TILE_SHADE_XSP,,,,,,)\
    CMD(a, b, c, d, e, int,,, G2CX_TILE_SHADE_YSP,,,,,,)\
    CMD(a, b, c, d, e, int,,, G2CX_TILE_SHADE_VAR,,,,,,)\
	/* NDG_D1 */\
    CMD(a, b, c, d, e, int,,, NDG_TILE_TOP_LOSS,,,,,,)\
    CMD(a, b, c, d, e, int,,, NDG_TILE_BOTTOM_LOSS,,,,,,)\
    CMD(a, b, c, d, e, int,,, NDG_TILE_LEFT_LOSS,,,,,,)\
    CMD(a, b, c, d, e, int,,, NDG_TILE_RIGHT_LOSS,,,,,,)\
	/* NDG2_D1 */\
    CMD(a, b, c, d, e, int,,, NDG2_TILE_TOP_LOSS,,,,,,)\
    CMD(a, b, c, d, e, int,,, NDG2_TILE_BOTTOM_LOSS,,,,,,)\
    CMD(a, b, c, d, e, int,,, NDG2_TILE_LEFT_LOSS,,,,,,)\
    CMD(a, b, c, d, e, int,,, NDG2_TILE_RIGHT_LOSS,,,,,,)\
    /* SLK_D1 */\
    CMD(a, b, c, d, e, int,,, SLK1_IN_TILE_XOFF,,,,,,)\
    CMD(a, b, c, d, e, int,,, SLK1_IN_TILE_YOFF,,,,,,)\
    CMD(a, b, c, d, e, int,,, SLK1_IN_TILE_WIDTH,,,,,,)\
    CMD(a, b, c, d, e, int,,, SLK1_IN_TILE_HEIGHT,,,,,,)\
    CMD(a, b, c, d, e, int,,, SLK1_OUT_TILE_XOFF,,,,,,)\
    CMD(a, b, c, d, e, int,,, SLK1_OUT_TILE_YOFF,,,,,,)\
    CMD(a, b, c, d, e, int,,, SLK1_OUT_TILE_WIDTH,,,,,,)\
    CMD(a, b, c, d, e, int,,, SLK1_OUT_TILE_HEIGHT,,,,,,)\
	/* SLK_D2 */\
    CMD(a, b, c, d, e, int,,, SLK2_IN_TILE_XOFF,,,,,,)\
    CMD(a, b, c, d, e, int,,, SLK2_IN_TILE_YOFF,,,,,,)\
    CMD(a, b, c, d, e, int,,, SLK2_IN_TILE_WIDTH,,,,,,)\
    CMD(a, b, c, d, e, int,,, SLK2_IN_TILE_HEIGHT,,,,,,)\
    CMD(a, b, c, d, e, int,,, SLK2_OUT_TILE_XOFF,,,,,,)\
    CMD(a, b, c, d, e, int,,, SLK2_OUT_TILE_YOFF,,,,,,)\
    CMD(a, b, c, d, e, int,,, SLK2_OUT_TILE_WIDTH,,,,,,)\
    CMD(a, b, c, d, e, int,,, SLK2_OUT_TILE_HEIGHT,,,,,,)\
    /* SLK_D3 */\
    CMD(a, b, c, d, e, int,,, SLK3_IN_TILE_XOFF,,,,,,)\
    CMD(a, b, c, d, e, int,,, SLK3_IN_TILE_YOFF,,,,,,)\
    CMD(a, b, c, d, e, int,,, SLK3_IN_TILE_WIDTH,,,,,,)\
    CMD(a, b, c, d, e, int,,, SLK3_IN_TILE_HEIGHT,,,,,,)\
    CMD(a, b, c, d, e, int,,, SLK3_OUT_TILE_XOFF,,,,,,)\
    CMD(a, b, c, d, e, int,,, SLK3_OUT_TILE_YOFF,,,,,,)\
    CMD(a, b, c, d, e, int,,, SLK3_OUT_TILE_WIDTH,,,,,,)\
    CMD(a, b, c, d, e, int,,, SLK3_OUT_TILE_HEIGHT,,,,,,)\
    /* SLK_D4 */\
    CMD(a, b, c, d, e, int,,, SLK4_IN_TILE_XOFF,,,,,,)\
    CMD(a, b, c, d, e, int,,, SLK4_IN_TILE_YOFF,,,,,,)\
    CMD(a, b, c, d, e, int,,, SLK4_IN_TILE_WIDTH,,,,,,)\
    CMD(a, b, c, d, e, int,,, SLK4_IN_TILE_HEIGHT,,,,,,)\
    CMD(a, b, c, d, e, int,,, SLK4_OUT_TILE_XOFF,,,,,,)\
    CMD(a, b, c, d, e, int,,, SLK4_OUT_TILE_YOFF,,,,,,)\
    CMD(a, b, c, d, e, int,,, SLK4_OUT_TILE_WIDTH,,,,,,)\
    CMD(a, b, c, d, e, int,,, SLK4_OUT_TILE_HEIGHT,,,,,,)\
    /* SLK_D5 */\
    CMD(a, b, c, d, e, int,,, SLK5_IN_TILE_XOFF,,,,,,)\
    CMD(a, b, c, d, e, int,,, SLK5_IN_TILE_YOFF,,,,,,)\
    CMD(a, b, c, d, e, int,,, SLK5_IN_TILE_WIDTH,,,,,,)\
    CMD(a, b, c, d, e, int,,, SLK5_IN_TILE_HEIGHT,,,,,,)\
    CMD(a, b, c, d, e, int,,, SLK5_OUT_TILE_XOFF,,,,,,)\
    CMD(a, b, c, d, e, int,,, SLK5_OUT_TILE_YOFF,,,,,,)\
    CMD(a, b, c, d, e, int,,, SLK5_OUT_TILE_WIDTH,,,,,,)\
    CMD(a, b, c, d, e, int,,, SLK5_OUT_TILE_HEIGHT,,,,,,)\
    /* SLK_D6 */\
    CMD(a, b, c, d, e, int,,, SLK6_IN_TILE_XOFF,,,,,,)\
    CMD(a, b, c, d, e, int,,, SLK6_IN_TILE_YOFF,,,,,,)\
    CMD(a, b, c, d, e, int,,, SLK6_IN_TILE_WIDTH,,,,,,)\
    CMD(a, b, c, d, e, int,,, SLK6_IN_TILE_HEIGHT,,,,,,)\
    CMD(a, b, c, d, e, int,,, SLK6_OUT_TILE_XOFF,,,,,,)\
    CMD(a, b, c, d, e, int,,, SLK6_OUT_TILE_YOFF,,,,,,)\
    CMD(a, b, c, d, e, int,,, SLK6_OUT_TILE_WIDTH,,,,,,)\
    CMD(a, b, c, d, e, int,,, SLK6_OUT_TILE_HEIGHT,,,,,,)\
    /* NDG_D1 */\
    CMD(a, b, c, d, e, int,,, NDG_TILE_XOFFSET,,,,,,)\
    CMD(a, b, c, d, e, int,,, NDG_TILE_YOFFSET,,,,,,)\
    CMD(a, b, c, d, e, int,,, NDG_TILE_WD,,,,,,)\
    CMD(a, b, c, d, e, int,,, NDG_TILE_HT,,,,,,)\
    CMD(a, b, c, d, e, int,,, NDG_TILE_XSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, NDG_TILE_XEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, NDG_TILE_YSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, NDG_TILE_YEND,,,,,,)\
    /* NDG_D2 */\
    CMD(a, b, c, d, e, int,,, NDG2_TILE_XOFFSET,,,,,,)\
    CMD(a, b, c, d, e, int,,, NDG2_TILE_YOFFSET,,,,,,)\
    CMD(a, b, c, d, e, int,,, NDG2_TILE_WD,,,,,,)\
    CMD(a, b, c, d, e, int,,, NDG2_TILE_HT,,,,,,)\
    CMD(a, b, c, d, e, int,,, NDG2_TILE_XSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, NDG2_TILE_XEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, NDG2_TILE_YSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, NDG2_TILE_YEND,,,,,,)\
    /* LCEI_D1 */\
    CMD(a, b, c, d, e, int,,, LCEI_TILE_OFFSET_ADDR,,,,,,)\
    CMD(a, b, c, d, e, int,,, LCEI_TILE_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, LCEI_TILE_YSIZE,,,,,,)\
    /* LCE_D1 */\
    CMD(a, b, c, d, e, int,,, LCE_TILE_OFFSET_X,,,,,,)\
    CMD(a, b, c, d, e, int,,, LCE_TILE_OFFSET_Y,,,,,,)\
    CMD(a, b, c, d, e, int,,, LCE_TILE_BIAS_X,,,,,,)\
    CMD(a, b, c, d, e, int,,, LCE_TILE_BIAS_Y,,,,,,)\
    CMD(a, b, c, d, e, int,,, LCE_TILE_IMAGE_WD,,,,,,)\
    CMD(a, b, c, d, e, int,,, LCE_TILE_IMAGE_HT,,,,,,)\
    CMD(a, b, c, d, e, int,,, LCE_TILE_SLM_WD,,,,,,)\
    CMD(a, b, c, d, e, int,,, LCE_TILE_SLM_HT,,,,,,)\
    CMD(a, b, c, d, e, int,,, LCE_TILE_EDGE,,,,,,)\
    /* CRZ_D1 */\
    CMD(a, b, c, d, e, int,,, CRZ_Tile_Input_Image_W,,,,,,)\
    CMD(a, b, c, d, e, int,,, CRZ_Tile_Input_Image_H,,,,,,)\
    CMD(a, b, c, d, e, int,,, CRZ_Tile_Output_Image_W,,,,,,)\
    CMD(a, b, c, d, e, int,,, CRZ_Tile_Output_Image_H,,,,,,)\
    CMD(a, b, c, d, e, int,,, CRZ_Tile_Luma_Horizontal_Integer_Offset,,,,,,)\
    CMD(a, b, c, d, e, int,,, CRZ_Tile_Luma_Vertical_Integer_Offset,,,,,,)\
    CMD(a, b, c, d, e, int,,, CRZ_Tile_Chroma_Horizontal_Integer_Offset,,,,,,)\
    CMD(a, b, c, d, e, int,,, CRZ_Tile_Chroma_Vertical_Integer_Offset,,,,,,)\
    CMD(a, b, c, d, e, int,,, CRZ_Tile_Luma_Horizontal_Subpixel_Offset,,,,,,)\
    CMD(a, b, c, d, e, int,,, CRZ_Tile_Luma_Vertical_Subpixel_Offset,,,,,,)\
    CMD(a, b, c, d, e, int,,, CRZ_Tile_Chroma_Horizontal_Subpixel_Offset,,,,,,)\
    CMD(a, b, c, d, e, int,,, CRZ_Tile_Chroma_Vertical_Subpixel_Offset,,,,,,)\
    /* CRZO_D1 */\
    CMD(a, b, c, d, e, int,,, CRZO_TILE_OFFSET_ADDR,,,,,,)\
    CMD(a, b, c, d, e, int,,, CRZO_TILE_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, CRZO_TILE_YSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, CRZO_TILE_XOFFSET,,,,,,)\
    CMD(a, b, c, d, e, int,,, CRZO_TILE_YOFFSET,,,,,,)\
    CMD(a, b, c, d, e, int,,, CRZO_TILE_FULL_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, CRZO_TILE_FULL_YSIZE,,,,,,)\
    /* CRZBO_D1 */\
    CMD(a, b, c, d, e, int,,, CRZBO_TILE_OFFSET_ADDR,,,,,,)\
    CMD(a, b, c, d, e, int,,, CRZBO_TILE_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, CRZBO_TILE_YSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, CRZBO_TILE_XOFFSET,,,,,,)\
    CMD(a, b, c, d, e, int,,, CRZBO_TILE_YOFFSET,,,,,,)\
    CMD(a, b, c, d, e, int,,, CRZBO_TILE_FULL_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, CRZBO_TILE_FULL_YSIZE,,,,,,)\
	/* EE_D1 */\
    CMD(a, b, c, d, e, int,,, EE_TILE_EDGE,,,,,,)\
    /* NR3D_D1 */\
    CMD(a, b, c, d, e, int,,, NR3D_TILE_ON_OFST_X,,,,,,)\
    CMD(a, b, c, d, e, int,,, NR3D_TILE_ON_OFST_Y,,,,,,)\
    CMD(a, b, c, d, e, int,,, NR3D_TILE_ON_WD,,,,,,)\
    CMD(a, b, c, d, e, int,,, NR3D_TILE_ON_HT,,,,,,)\
    CMD(a, b, c, d, e, int,,, NR3D_TILE_WD,,,,,,)\
    CMD(a, b, c, d, e, int,,, NR3D_TILE_HT,,,,,,)\
    CMD(a, b, c, d, e, int,,, NR3D_TILE_EDGE,,,,,,)\
    CMD(a, b, c, d, e, int,,, NR3D_TILE_X,,,,,,)\
    CMD(a, b, c, d, e, int,,, NR3D_TILE_Y,,,,,,)\
	/* CRSP_D1 */\
    CMD(a, b, c, d, e, int,,, CRSP_TILE_WD,,,,,,)\
    CMD(a, b, c, d, e, int,,, CRSP_TILE_OFST_X,,,,,,)\
    CMD(a, b, c, d, e, int,,, CRSP_TILE_HT,,,,,,)\
    CMD(a, b, c, d, e, int,,, CRSP_TILE_OFST_Y,,,,,,)\
    CMD(a, b, c, d, e, int,,, CRSP_TILE_CROP_XSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, CRSP_TILE_CROP_XEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, CRSP_TILE_CROP_YSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, CRSP_TILE_CROP_YEND,,,,,,)\
	/* CRSP_D2 */\
    CMD(a, b, c, d, e, int,,, CRSP2_TILE_WD,,,,,,)\
    CMD(a, b, c, d, e, int,,, CRSP2_TILE_OFST_X,,,,,,)\
    CMD(a, b, c, d, e, int,,, CRSP2_TILE_HT,,,,,,)\
    CMD(a, b, c, d, e, int,,, CRSP2_TILE_OFST_Y,,,,,,)\
    CMD(a, b, c, d, e, int,,, CRSP2_TILE_CROP_XSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, CRSP2_TILE_CROP_XEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, CRSP2_TILE_CROP_YSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, CRSP2_TILE_CROP_YEND,,,,,,)\
	/* DCE_D1 */\
    CMD(a, b, c, d, e, int,,, DCES_TILE_CROP_XSTART,,,,,,)\
	CMD(a, b, c, d, e, int,,, DCES_TILE_CROP_XEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, DCES_TILE_CROP_YSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, DCES_TILE_CROP_YEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, DCES_HSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, DCES_VSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, DCES_CLEAR,,,,,,)\
    /* DCESO_D1 */\
    CMD(a, b, c, d, e, int,,, DCESO_TILE_OFFSET_ADDR,,,,,,)\
    CMD(a, b, c, d, e, int,,, DCESO_TILE_XSIZE,,,,,,)\
    /* IMG3O_D1 */\
    CMD(a, b, c, d, e, int,,, IMG3O_TILE_OFFSET_ADDR,,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG3O_TILE_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG3O_TILE_YSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG3O_TILE_XOFFSET,,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG3O_TILE_YOFFSET,,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG3O_TILE_FULL_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG3O_TILE_FULL_YSIZE,,,,,,)\
    /* IMG3BO_D1 */\
    CMD(a, b, c, d, e, int,,, IMG3BO_TILE_OFFSET_ADDR,,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG3BO_TILE_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG3BO_TILE_YSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG3BO_TILE_XOFFSET,,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG3BO_TILE_YOFFSET,,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG3BO_TILE_FULL_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG3BO_TILE_FULL_YSIZE,,,,,,)\
    /* IMG3CO_D1 */\
    CMD(a, b, c, d, e, int,,, IMG3CO_TILE_OFFSET_ADDR,,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG3CO_TILE_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG3CO_TILE_YSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG3CO_TILE_XOFFSET,,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG3CO_TILE_YOFFSET,,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG3CO_TILE_FULL_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG3CO_TILE_FULL_YSIZE,,,,,,)\
    /* SRZ_D1 */\
    CMD(a, b, c, d, e, int,,, SRZ1_Tile_Input_Image_W,,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ1_Tile_Input_Image_H,,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ1_Tile_Output_Image_W,,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ1_Tile_Output_Image_H,,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ1_Tile_Luma_Horizontal_Integer_Offset,,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ1_Tile_Luma_Horizontal_Subpixel_Offset,,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ1_Tile_Luma_Vertical_Integer_Offset,,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ1_Tile_Luma_Vertical_Subpixel_Offset,,,,,,)\
    /* SRZ_D3 */\
    CMD(a, b, c, d, e, int,,, SRZ3_Tile_Input_Image_W,,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ3_Tile_Input_Image_H,,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ3_Tile_Output_Image_W,,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ3_Tile_Output_Image_H,,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ3_Tile_Luma_Horizontal_Integer_Offset,,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ3_Tile_Luma_Horizontal_Subpixel_Offset,,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ3_Tile_Luma_Vertical_Integer_Offset,,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ3_Tile_Luma_Vertical_Subpixel_Offset,,,,,,)\
    /* SRZ_D4 */\
    CMD(a, b, c, d, e, int,,, SRZ4_Tile_Input_Image_W,,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ4_Tile_Input_Image_H,,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ4_Tile_Output_Image_W,,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ4_Tile_Output_Image_H,,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ4_Tile_Luma_Horizontal_Integer_Offset,,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ4_Tile_Luma_Horizontal_Subpixel_Offset,,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ4_Tile_Luma_Vertical_Integer_Offset,,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ4_Tile_Luma_Vertical_Subpixel_Offset,,,,,,)\
    /* DFE_D1 */\
    CMD(a, b, c, d, e, int,,, DFE_TILE_EDGE,,,,,,)\
    CMD(a, b, c, d, e, int,,, DFE_TILE_XIDX,,,,,,)\
    CMD(a, b, c, d, e, int,,, DFE_TILE_YIDX,,,,,,)\
    CMD(a, b, c, d, e, int,,, DFE_TILE_START_X,,,,,,)\
    CMD(a, b, c, d, e, int,,, DFE_TILE_START_Y,,,,,,)\
    CMD(a, b, c, d, e, int,,, DFE_TILE_IN_WIDTH,,,,,,)\
    CMD(a, b, c, d, e, int,,, DFE_TILE_IN_HEIGHT,,,,,,)\
    /* FEO_D1 */\
    CMD(a, b, c, d, e, int,,, FEO_TILE_OFFSET_ADDR,,,,,,)\
    CMD(a, b, c, d, e, int,,, FEO_TILE_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, FEO_TILE_YSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, FEO_TILE_FULL_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, FEO_TILE_FULL_YSIZE,,,,,,)\
    /* C02_D1 */\
    CMD(a, b, c, d, e, int,,, C02_TILE_EDGE,,,,,,)\
    CMD(a, b, c, d, e, int,,, C02_TILE_CROP_XSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, C02_TILE_CROP_XEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, C02_TILE_CROP_YSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, C02_TILE_CROP_YEND,,,,,,)\
    /* C02_D2 */\
    CMD(a, b, c, d, e, int,,, C02D2_TILE_EDGE,,,,,,)\
    CMD(a, b, c, d, e, int,,, C02D2_TILE_CROP_XSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, C02D2_TILE_CROP_XEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, C02D2_TILE_CROP_YSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, C02D2_TILE_CROP_YEND,,,,,,)\
	/* C24_D1 */\
    CMD(a, b, c, d, e, int,,, C24_TILE_EDGE,,,,,,)\
	/* C24_D2 */\
    CMD(a, b, c, d, e, int,,, C24D2_TILE_EDGE,,,,,,)\
	/* C24_D3 */\
    CMD(a, b, c, d, e, int,,, C24D3_TILE_EDGE,,,,,,)\
	/* C42_D1 */\
    CMD(a, b, c, d, e, int,,, C42_TILE_EDGE,,,,,,)\
	/* C42_D2 */\
    CMD(a, b, c, d, e, int,,, C42D2_TILE_EDGE,,,,,,)\
	/* C42_D4 */\
    CMD(a, b, c, d, e, int,,, C42D4_TILE_EDGE,,,,,,)\
	/* YNR_D1 */\
    CMD(a, b, c, d, e, int,,, YNR_TILE_EDGE,,,,,,)\
    CMD(a, b, c, d, e, int,,, YNR_LTM_TOP_LOSS,,,,,,)\
    CMD(a, b, c, d, e, int,,, YNR_LTM_BOTTOM_LOSS,,,,,,)\
    CMD(a, b, c, d, e, int,,, YNR_LTM_LEFT_LOSS,,,,,,)\
    CMD(a, b, c, d, e, int,,, YNR_LTM_RIGHT_LOSS,,,,,,)\
	/* CNR_D1 */\
    CMD(a, b, c, d, e, int,,, CNR_TILE_EDGE,,,,,,)\
    CMD(a, b, c, d, e, int,,, CNR_BOK_TOP_LOSS,,,,,,)\
    CMD(a, b, c, d, e, int,,, CNR_BOK_BOTTOM_LOSS,,,,,,)\
    CMD(a, b, c, d, e, int,,, CNR_BOK_LEFT_LOSS,,,,,,)\
    CMD(a, b, c, d, e, int,,, CNR_BOK_RIGHT_LOSS,,,,,,)\
	/* COLOR_D1 */\
    CMD(a, b, c, d, e, int,,, COLOR_TILE_EDGE,,,,,,)\
    CMD(a, b, c, d, e, int,,, COLOR_CROP_H,,,,,,)\
    CMD(a, b, c, d, e, int,,, COLOR_CROP_V,,,,,,)\
	/* OBC_D1 */\
    CMD(a, b, c, d, e, int,,, OBC_TILE_EDGE,,,,,,)\
    CMD(a, b, c, d, e, int,,, OBC_TILE_LE_INV_CTL,,,,,,)\
	/* BPC_D1 */\
    CMD(a, b, c, d, e, int,,, BPC_TILE_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, BPC_TILE_YSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, BPC_TILE_XOFFSET,,,,,,)\
    CMD(a, b, c, d, e, int,,, BPC_TILE_YOFFSET,,,,,,)\
    CMD(a, b, c, d, e, int,,, BPC_TILE_EDGE,,,,,,)\
    CMD(a, b, c, d, e, int,,, BPC_TILE_LE_INV_CTL,,,,,,)\
	/* ZFUS_D1 */\
    CMD(a, b, c, d, e, int,,, ZFUS_TILE_EDGE,,,,,,)\
    CMD(a, b, c, d, e, int,,, ZFUS_TILE_LE_INV_CTL,,,,,,)\
    /* SMTI_D1 */\
    CMD(a, b, c, d, e, int,,, SMT1I_TILE_OFFSET_ADDR,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT1I_TILE_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT1I_TILE_YSIZE,,,,,,)\
    /* SMTO_D1 */\
    CMD(a, b, c, d, e, int,,, SMT1O_TILE_OFFSET_ADDR,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT1O_TILE_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT1O_TILE_YSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT1O_TILE_XOFFSET,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT1O_TILE_YOFFSET,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT1O_TILE_FULL_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT1O_TILE_FULL_YSIZE,,,,,,)\
    /* SMTI_D2 */\
    CMD(a, b, c, d, e, int,,, SMT2I_TILE_OFFSET_ADDR,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT2I_TILE_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT2I_TILE_YSIZE,,,,,,)\
    /* SMTO_D2 */\
    CMD(a, b, c, d, e, int,,, SMT2O_TILE_OFFSET_ADDR,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT2O_TILE_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT2O_TILE_YSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT2O_TILE_XOFFSET,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT2O_TILE_YOFFSET,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT2O_TILE_FULL_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT2O_TILE_FULL_YSIZE,,,,,,)\
    /* SMTI_D3 */\
    CMD(a, b, c, d, e, int,,, SMT3I_TILE_OFFSET_ADDR,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT3I_TILE_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT3I_TILE_YSIZE,,,,,,)\
    /* SMTO_D3 */\
    CMD(a, b, c, d, e, int,,, SMT3O_TILE_OFFSET_ADDR,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT3O_TILE_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT3O_TILE_YSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT3O_TILE_XOFFSET,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT3O_TILE_YOFFSET,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT3O_TILE_FULL_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT3O_TILE_FULL_YSIZE,,,,,,)\
    /* SMTI_D4 */\
    CMD(a, b, c, d, e, int,,, SMT4I_TILE_OFFSET_ADDR,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT4I_TILE_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT4I_TILE_YSIZE,,,,,,)\
    /* SMTO_D4 */\
    CMD(a, b, c, d, e, int,,, SMT4O_TILE_OFFSET_ADDR,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT4O_TILE_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT4O_TILE_YSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT4O_TILE_XOFFSET,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT4O_TILE_YOFFSET,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT4O_TILE_FULL_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT4O_TILE_FULL_YSIZE,,,,,,)\
    /* MFBI */\
    CMD(a, b, c, d, e, int,,, MFBI_TILE_OFFSET_ADDR,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFBI_TILE_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFBI_TILE_YSIZE,,,,,,)\
    /* MFBI_B */\
    CMD(a, b, c, d, e, int,,, MFBI_B_TILE_OFFSET_ADDR,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFBI_B_TILE_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFBI_B_TILE_YSIZE,,,,,,)\
    /* MFB3I */\
    CMD(a, b, c, d, e, int,,, MFB3I_TILE_OFFSET_ADDR,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB3I_TILE_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB3I_TILE_YSIZE,,,,,,)\
    /* MFB4I */\
    CMD(a, b, c, d, e, int,,, MFB4I_TILE_OFFSET_ADDR,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB4I_TILE_XSIZE,,,,,,)\
	CMD(a, b, c, d, e, int,,, MFB4I_TILE_YSIZE,,,,,,)\
	/* MFB_C02 */\
    CMD(a, b, c, d, e, int,,, MFB_C02_TILE_EDGE,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_C02_TILE_CROP_XSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_C02_TILE_CROP_XEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_C02_TILE_CROP_YSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_C02_TILE_CROP_YEND,,,,,,)\
	/* MFB_CRSP */\
    CMD(a, b, c, d, e, int,,, MFB_CRSP_TILE_WD,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_CRSP_TILE_HT,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_CRSP_TILE_OFST_X,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_CRSP_TILE_OFST_Y,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_CRSP_TILE_CROP_XSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_CRSP_TILE_CROP_XEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_CRSP_TILE_CROP_YSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_CRSP_TILE_CROP_YEND,,,,,,)\
	/* OMC */\
    CMD(a, b, c, d, e, int,,, OMC_TILE_OFST_X,,,,,,)\
    CMD(a, b, c, d, e, int,,, OMC_TILE_OFST_Y,,,,,,)\
    CMD(a, b, c, d, e, int,,, OMC_TILE_WD,,,,,,)\
    CMD(a, b, c, d, e, int,,, OMC_TILE_HT,,,,,,)\
    CMD(a, b, c, d, e, int,,, OMC_TILE_EDGE,,,,,,)\
    CMD(a, b, c, d, e, int,,, OMC_TILE_CROP_XSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, OMC_TILE_CROP_XEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, OMC_TILE_CROP_YSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, OMC_TILE_CROP_YEND,,,,,,)\
	/* MFB_Y_UNP */\
    CMD(a, b, c, d, e, int,,, MFB_Y_UNP_OFST_STB,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_Y_UNP_OFST_EDB,,,,,,)\
	/* MFB_C_UNP */\
    CMD(a, b, c, d, e, int,,, MFB_C_UNP_OFST_STB,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_C_UNP_OFST_EDB,,,,,,)\
    /* MFB */\
    CMD(a, b, c, d, e, int,,, MFB_TILE_EDGE,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_TILE_TOP_LOSS,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_TILE_BOTTOM_LOSS,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_TILE_LEFT_LOSS,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_TILE_RIGHT_LOSS,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_CONF_TOP_LOSS,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_CONF_BOTTOM_LOSS,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_CONF_LEFT_LOSS,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_CONF_RIGHT_LOSS,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_WT_TOP_LOSS,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_WT_BOTTOM_LOSS,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_WT_LEFT_LOSS,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_WT_RIGHT_LOSS,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_FRAME_WIDTH,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_FRAME_HEIGHT,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_END_XSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_END_XEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_END_YSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_END_YEND,,,,,,)\
    /* MFB_SRZ */\
    CMD(a, b, c, d, e, int,,, MFB_SRZ_Tile_Input_Image_W,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_SRZ_Tile_Input_Image_H,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_SRZ_Tile_Output_Image_W,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_SRZ_Tile_Output_Image_H,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_SRZ_Tile_Luma_Horizontal_Integer_Offset,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_SRZ_Tile_Luma_Horizontal_Subpixel_Offset,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_SRZ_Tile_Luma_Vertical_Integer_Offset,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_SRZ_Tile_Luma_Vertical_Subpixel_Offset,,,,,,)\
    /* MFBO */\
    CMD(a, b, c, d, e, int,,, MFBO_TILE_OFFSET_ADDR,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFBO_TILE_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFBO_TILE_YSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFBO_TILE_XOFFSET,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFBO_TILE_YOFFSET,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFBO_TILE_FULL_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFBO_TILE_FULL_YSIZE,,,,,,)\
    /* MFBO_B */\
    CMD(a, b, c, d, e, int,,, MFBO_B_TILE_OFFSET_ADDR,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFBO_B_TILE_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFBO_B_TILE_YSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFBO_B_TILE_XOFFSET,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFBO_B_TILE_YOFFSET,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFBO_B_TILE_FULL_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFBO_B_TILE_FULL_YSIZE,,,,,,)\
    /* MFB2O */\
    CMD(a, b, c, d, e, int,,, MFB2O_TILE_OFFSET_ADDR,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB2O_TILE_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB2O_TILE_YSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB2O_TILE_XOFFSET,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB2O_TILE_YOFFSET,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB2O_TILE_FULL_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB2O_TILE_FULL_YSIZE,,,,,,)\
    /* Internal */\
    CMD(a, b, c, d, e, int,,, CTRL_IMGI_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_IMGBI_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_IMGCI_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_VIPI_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_VIPBI_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_VIPCI_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_DEPI_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_DMGI_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_LCEI_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_IMG3O_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_IMG3BO_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_IMG3CO_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_UNP2_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_UNP3_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_UNP4_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_UNP6_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_UNP7_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_UNP8_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_UNP9_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_UNP10_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_UNP11_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_CNR_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_COLOR_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_NR3D_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SLK1_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SLK2_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SLK3_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SLK4_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SLK5_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SLK6_EN,,,,,,)\
	CMD(a, b, c, d, e, int,,, CTRL_NDG_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_NDG2_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_YCNR_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_TIMGO_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_G2CX_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_C42_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_YNR_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_PCA_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_EE_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_CRP2_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_PAK_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_PAK2_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_PAK3_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_PAK4_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_PAK6_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_PAK7_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_PAK8_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_PAKG_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_UFDI_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_UNP_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_LSC_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_WB_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_HLR_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_LTM_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_DM_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_LDNR_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_FLC_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_CCM_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_CCM2_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_GGM_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_GGM2_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_GGM4_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_G2C4_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_C42D4_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_CRP_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_CRP4_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_C24D2_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_C24D3_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_C2G_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_IGGM_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_CCM3_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_GGM3_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_DCE_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_DCES_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_DCESO_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_G2C_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_C42D2_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_LCE_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_MCRP_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_MCRP2_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_CRZO_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_CRZBO_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_PLNR1_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_PLNR2_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_PLNW1_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_PLNW2_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_DFE_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_FEO_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SRZ1_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SRZ3_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SRZ4_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_C02_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_CRSP_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_MIX1_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_MIX2_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_MIX3_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_MIX4_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMT1I_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMT2I_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMT3I_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMT4I_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMT1O_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMT2O_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMT3O_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMT4O_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMT1_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMT2_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMT3_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMT4_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMT1_TRD_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMT1_TRU_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMT1_CRPINL_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMT1_CRPINR_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMT1_CRPOUT_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMT2_TRD_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMT2_TRU_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMT2_CRPINL_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMT2_CRPINR_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMT2_CRPOUT_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMT3_TRD_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMT3_TRU_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMT3_CRPINL_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMT3_CRPINR_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMT3_CRPOUT_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMT4_TRD_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMT4_TRU_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMT4_CRPINL_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMT4_CRPINR_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMT4_CRPOUT_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_CRZ_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_ADL_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_BS_EN,,,,,,)\
	/* OFT */\
    CMD(a, b, c, d, e, int,,, CTRL_OFT_CRZ_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_OFT_DTH_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_OFT_C24_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_OFT_C42_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_OFT_R2B_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_OFT_C20_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_OFT_CROP_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_OFT_C2G_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_OFT_CCM_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_OFT_GCE_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_OFT_G2C_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_OFT_IGGM_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_OFT_HSV_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_OFT_LAB_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_OFTL_CRZ_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_OFTL_DTH_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_OFTL_C24_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_OFTL_C42_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_OFTL_R2B_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_OFTL_C20_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_OFTL_CROP_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_OFTL_C2G_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_OFTL_CCM_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_OFTL_GCE_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_OFTL_G2C_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_OFTL_IGGM_EN,,,,,,)\
	/* SMT_D1 */\
    CMD(a, b, c, d, e, int,,, SMT1_CRPINL_XSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT1_CRPINL_XEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT1_CRPINL_YSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT1_CRPINL_YEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT1_CRPINR_XSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT1_CRPINR_XEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT1_CRPINR_YSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT1_CRPINR_YEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT1_CRPOUT_XSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT1_CRPOUT_XEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT1_CRPOUT_YSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT1_CRPOUT_YEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT1_LEFT_DISABLE,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT1_RIGHT_DISABLE,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT1_TRU_IN_HT,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT1_TRU_IN_WD,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT1O_TDR_OFFSET,,,,,,)\
	/* SMT_D2 */\
    CMD(a, b, c, d, e, int,,, SMT2_CRPINL_XSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT2_CRPINL_XEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT2_CRPINL_YSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT2_CRPINL_YEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT2_CRPINR_XSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT2_CRPINR_XEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT2_CRPINR_YSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT2_CRPINR_YEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT2_CRPOUT_XSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT2_CRPOUT_XEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT2_CRPOUT_YSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT2_CRPOUT_YEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT2_LEFT_DISABLE,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT2_RIGHT_DISABLE,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT2_TRU_IN_HT,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT2_TRU_IN_WD,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT2O_TDR_OFFSET,,,,,,)\
	/* SMT_D3 */\
    CMD(a, b, c, d, e, int,,, SMT3_CRPINL_XSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT3_CRPINL_XEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT3_CRPINL_YSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT3_CRPINL_YEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT3_CRPINR_XSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT3_CRPINR_XEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT3_CRPINR_YSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT3_CRPINR_YEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT3_CRPOUT_XSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT3_CRPOUT_XEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT3_CRPOUT_YSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT3_CRPOUT_YEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT3_LEFT_DISABLE,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT3_RIGHT_DISABLE,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT3_TRU_IN_HT,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT3_TRU_IN_WD,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT3O_TDR_OFFSET,,,,,,)\
	/* SMT_D4 */\
    CMD(a, b, c, d, e, int,,, SMT4_CRPINL_XSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT4_CRPINL_XEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT4_CRPINL_YSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT4_CRPINL_YEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT4_CRPINR_XSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT4_CRPINR_XEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT4_CRPINR_YSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT4_CRPINR_YEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT4_CRPOUT_XSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT4_CRPOUT_XEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT4_CRPOUT_YSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT4_CRPOUT_YEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT4_LEFT_DISABLE,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT4_RIGHT_DISABLE,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT4_TRU_IN_HT,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT4_TRU_IN_WD,,,,,,)\
    CMD(a, b, c, d, e, int,,, SMT4O_TDR_OFFSET,,,,,,)\
    /* MCRP_D1 */\
    CMD(a, b, c, d, e, int,,, CTRL_MCRP_XSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_MCRP_XEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_MCRP_YSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_MCRP_YEND,,,,,,)\
    /* MCRP_D2 */\
    CMD(a, b, c, d, e, int,,, CTRL_MCRP2_XSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_MCRP2_XEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_MCRP2_YSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_MCRP2_YEND,,,,,,)\
    /* CRP_D1 */\
    CMD(a, b, c, d, e, int,,, CTRL_CRP_XSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_CRP_XEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_CRP_YSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_CRP_YEND,,,,,,)\
    /* CRP_D2 */\
    CMD(a, b, c, d, e, int,,, CTRL_CRP2_XSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_CRP2_XEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_CRP2_YSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_CRP2_YEND,,,,,,)\
    /* CRP_D4 */\
    CMD(a, b, c, d, e, int,,, CTRL_CRP4_XSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_CRP4_XEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_CRP4_YSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_CRP4_YEND,,,,,,)\
	/* ADL */\
    CMD(a, b, c, d, e, int,,, ADLI_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, ADLI_YSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, ADLO_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, ADLO_YSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_IN_CROP_XSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_IN_CROP_XEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_IN_CROP_YSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_IN_CROP_YEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_OUT_CROP_XSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_OUT_CROP_XEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_OUT_CROP_YSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_OUT_CROP_YEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_0,,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_1,,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_2,,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_3,,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_4,,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_5,,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_6,,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_7,,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_8,,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_9,,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_10,,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_11,,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_12,,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_13,,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_14,,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_15,,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_16,,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_17,,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_18,,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_19,,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_20,,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_21,,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_22,,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_23,,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_24,,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_25,,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_26,,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_27,,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_28,,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_29,,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_30,,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_31,,,,,,)\
    /* OFT CRZ */\
    CMD(a, b, c, d, e, int,,, OFT_CRZ_Tile_Input_Image_W,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFT_CRZ_Tile_Input_Image_H,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFT_CRZ_Tile_Output_Image_W,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFT_CRZ_Tile_Output_Image_H,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFT_CRZ_Tile_Luma_Horizontal_Integer_Offset,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFT_CRZ_Tile_Luma_Vertical_Integer_Offset,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFT_CRZ_Tile_Chroma_Horizontal_Integer_Offset,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFT_CRZ_Tile_Chroma_Vertical_Integer_Offset,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFT_CRZ_Tile_Luma_Horizontal_Subpixel_Offset,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFT_CRZ_Tile_Luma_Vertical_Subpixel_Offset,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFT_CRZ_Tile_Chroma_Horizontal_Subpixel_Offset,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFT_CRZ_Tile_Chroma_Vertical_Subpixel_Offset,,,,,,)\
	/* OFT DTH*/\
    CMD(a, b, c, d, e, int,,, OFT_DTH_TILE_TOP_LOSS,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFT_DTH_TILE_BOTTOM_LOSS,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFT_DTH_TILE_LEFT_LOSS,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFT_DTH_TILE_RIGHT_LOSS,,,,,,)\
	/* OFT */\
    CMD(a, b, c, d, e, int,,, OFT_C24_TILE_EDGE,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFT_C42_TILE_EDGE,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFT_DTH_TILE_XOFFSET,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFT_DTH_TILE_YOFFSET,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFT_DTH_TILE_WD,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFT_DTH_TILE_HT,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFT_DTH_TILE_XSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFT_DTH_TILE_XEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFT_DTH_TILE_YSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFT_DTH_TILE_YEND,,,,,,)\
    /* OFT R2B */\
    CMD(a, b, c, d, e, int,,, OFT_R2B_TILE_CROP_XSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFT_R2B_TILE_CROP_XEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFT_R2B_TILE_CROP_YSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFT_R2B_TILE_CROP_YEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFT_R2B_TILE_WIDTH,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFT_R2B_TILE_HEIGHT,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFT_R2B_TILE_CH0_ADDR_OFSET,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFT_R2B_TILE_CH1_ADDR_OFSET,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFT_R2B_TILE_CH2_ADDR_OFSET,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFT_R2B_TILE_FULL_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFT_R2B_TILE_FULL_YSIZE,,,,,,)\
    /* OFTL CRZ */\
    CMD(a, b, c, d, e, int,,, OFTL_CRZ_Tile_Input_Image_W,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFTL_CRZ_Tile_Input_Image_H,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFTL_CRZ_Tile_Output_Image_W,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFTL_CRZ_Tile_Output_Image_H,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFTL_CRZ_Tile_Luma_Horizontal_Integer_Offset,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFTL_CRZ_Tile_Luma_Vertical_Integer_Offset,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFTL_CRZ_Tile_Chroma_Horizontal_Integer_Offset,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFTL_CRZ_Tile_Chroma_Vertical_Integer_Offset,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFTL_CRZ_Tile_Luma_Horizontal_Subpixel_Offset,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFTL_CRZ_Tile_Luma_Vertical_Subpixel_Offset,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFTL_CRZ_Tile_Chroma_Horizontal_Subpixel_Offset,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFTL_CRZ_Tile_Chroma_Vertical_Subpixel_Offset,,,,,,)\
	/* OFTL DTH*/\
    CMD(a, b, c, d, e, int,,, OFTL_DTH_TILE_TOP_LOSS,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFTL_DTH_TILE_BOTTOM_LOSS,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFTL_DTH_TILE_LEFT_LOSS,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFTL_DTH_TILE_RIGHT_LOSS,,,,,,)\
	/* OFTL */\
    CMD(a, b, c, d, e, int,,, OFTL_C24_TILE_EDGE,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFTL_C42_TILE_EDGE,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFTL_DTH_TILE_XOFFSET,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFTL_DTH_TILE_YOFFSET,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFTL_DTH_TILE_WD,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFTL_DTH_TILE_HT,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFTL_DTH_TILE_XSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFTL_DTH_TILE_XEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFTL_DTH_TILE_YSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFTL_DTH_TILE_YEND,,,,,,)\
    /* OFTL R2B */\
    CMD(a, b, c, d, e, int,,, OFTL_R2B_TILE_CROP_XSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFTL_R2B_TILE_CROP_XEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFTL_R2B_TILE_CROP_YSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFTL_R2B_TILE_CROP_YEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFTL_R2B_TILE_WIDTH,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFTL_R2B_TILE_HEIGHT,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFTL_R2B_TILE_CH0_ADDR_OFSET,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFTL_R2B_TILE_CH1_ADDR_OFSET,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFTL_R2B_TILE_CH2_ADDR_OFSET,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFTL_R2B_TILE_FULL_XSIZE,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFTL_R2B_TILE_FULL_YSIZE,,,,,,)\
	/* OFT_C20_D1 */\
    CMD(a, b, c, d, e, int,,, OFT_C20_TILE_WD,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFT_C20_TILE_OFST_X,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFT_C20_TILE_HT,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFT_C20_TILE_OFST_Y,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFT_C20_TILE_CROP_XSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFT_C20_TILE_CROP_XEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFT_C20_TILE_CROP_YSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFT_C20_TILE_CROP_YEND,,,,,,)\
	/* OFTL_C20_D1 */\
    CMD(a, b, c, d, e, int,,, OFTL_C20_TILE_WD,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFTL_C20_TILE_OFST_X,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFTL_C20_TILE_HT,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFTL_C20_TILE_OFST_Y,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFTL_C20_TILE_CROP_XSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFTL_C20_TILE_CROP_XEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFTL_C20_TILE_CROP_YSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, OFTL_C20_TILE_CROP_YEND,,,,,,)\
    /* OFT_CROP_D1 */\
    CMD(a, b, c, d, e, int,,, CTRL_OFT_CROP_XSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_OFT_CROP_XEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_OFT_CROP_YSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_OFT_CROP_YEND,,,,,,)\
    /* OFTL_CROP_D1 */\
    CMD(a, b, c, d, e, int,,, CTRL_OFTL_CROP_XSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_OFTL_CROP_XEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_OFTL_CROP_YSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_OFTL_CROP_YEND,,,,,,)\
	/* WPE_DL */\
    CMD(a, b, c, d, e, int,,, WPE_FRAME_WIDTH,,,,,,)\
    CMD(a, b, c, d, e, int,,, WPE_FRAME_HEIGHT,,,,,,)\
    CMD(a, b, c, d, e, int,,, WPE_END_XSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, WPE_END_XEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, WPE_END_YSTART,,,,,,)\
    CMD(a, b, c, d, e, int,,, WPE_END_YEND,,,,,,)\
    CMD(a, b, c, d, e, int,,, WPE_DL_LAST_TILE,,,,,,)\

/* register table ( , , tile driver) for tile driver only parameters */
/* a, b, c, d, e reserved */
/* data type */
/* register name of current c model */
/* register name of HW ISP & platform parameters */
/* internal variable name of tile */
/* array bracket [xx] */
/* valid condition by tdr_en to print platform log with must string, default: false */
/* isp_reg.h reg name */
/* isp_reg.h field name */
/* direct-link param 0: must be equal, 1: replaced by MDP, 2: don't care */
#define ISP_TILE_INTERNAL_REG_LUT(CMD, a, b, c, d, e) \
    /* tdr_control_en */\
    CMD(a, b, c, d, e, int,,, LOG_IMGI_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_IMGBI_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_IMGCI_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_VIPI_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_VIPBI_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_VIPCI_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_UFDI_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_UNP_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_UNP2_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_UNP3_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_UNP4_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_UNP6_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_UNP7_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_UNP8_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_UNP9_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_UNP10_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_UNP11_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_UFD_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_OBC_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_PAK_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_PAK2_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_PAK3_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_PAK4_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_PAK6_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_PAK7_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_PAK8_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_CRP2_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_BPC_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_ZFUS_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_LSC_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_WB_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_HLR_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_LTM_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_DM_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_LDNR_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_FLC_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_CCM_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_CCM2_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_GGM_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_GGM2_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_GGM4_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_G2C4_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_C42D4_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_CRP_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_CRP4_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_C42_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_G2CX_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_TIMGO_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_YNR_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_CNR_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_COLOR_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_NR3D_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_SLK1_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_SLK2_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_SLK3_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_SLK4_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_SLK5_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_SLK6_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_NDG_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_NDG2_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_C24_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_C24D2_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_C24D3_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_C2G_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_IGGM_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_CCM3_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_GGM3_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_DCE_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_DCES_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_DCESO_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_G2C_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_C42D2_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_EE_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_IMG3O_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_IMG3BO_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_IMG3CO_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_CRZO_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_CRZBO_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_DEPI_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_DMGI_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_LCEI_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_LCE_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_SRZ1_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_SRZ3_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_SRZ4_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_CRZ_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_PLNR1_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_PLNW1_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_PLNW2_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_C02_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_C02D2_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_CRSP_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_MCP_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_MCP2_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_DFE_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_FEO_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_SMT1_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_SMT2_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_SMT3_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_SMT4_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_MIX1_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_MIX2_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_MIX3_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_MIX4_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_OFT_CRZ_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_OFT_C20_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_OFT_R2B_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_OFTL_CROP_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_OFTL_CRZ_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_OFTL_DTH_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_OFTL_C24_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_OFTL_C2G_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_OFTL_IGGM_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_OFTL_CCM_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_OFTL_GCE_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_OFTL_G2C_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_OFTL_C42_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_OFTL_C20_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_OFTL_R2B_EN,,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_WPE_DL_EN,,,,,,)\

#endif
