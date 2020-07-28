#ifndef __TILE_ISP_REG_H__
#define __TILE_ISP_REG_H__

/* error enum */
#define ISP_TILE_ERROR_MESSAGE_ENUM(n, CMD) \
	/* Raw check */\
    CMD(n, ISP_MESSAGE_INVALID_SRC_SIZE_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_INVALID_RAW_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_INVALID_SRC_STRIDE_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
	/* LSC check */\
    CMD(n, ISP_MESSAGE_LSC_ZERO_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* LCEI check */\
    CMD(n, ISP_MESSAGE_LCEI_FORMAT_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* IMGBI check */\
    CMD(n, ISP_MESSAGE_IMGBI_FORMAT_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* IMGCI check */\
    CMD(n, ISP_MESSAGE_IMGCI_FORMAT_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* VIPI check */\
    CMD(n, ISP_MESSAGE_VIPI_FORMAT_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_VIPI_XSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_VIPI_YSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* VIP2I check */\
    CMD(n, ISP_MESSAGE_VIP2I_FORMAT_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_VIP2I_XSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_VIP2I_YSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* VIP3I check */\
    CMD(n, ISP_MESSAGE_VIP3I_FORMAT_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_VIP3I_YSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* IMG2O size check */\
    CMD(n, ISP_MESSAGE_IMG2O_XSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_IMG2O_YSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_IMG2BO_XSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_IMG2BO_YSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* IMG3O size check */\
    CMD(n, ISP_MESSAGE_IMG3O_XSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_IMG3O_YSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* IMG3BO size check */\
    CMD(n, ISP_MESSAGE_IMG3BO_XSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_IMG3BO_YSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* IMG3CO size check */\
    CMD(n, ISP_MESSAGE_IMG3CO_XSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_IMG3CO_YSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* RSP CROP DISABLE error check */\
    CMD(n, ISP_MESSAGE_RSP_XS_BACK_FOR_DIFF_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_RSP_YS_BACK_FOR_DIFF_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_RSP_XE_BACK_SMALLER_THAN_FOR_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_RSP_YE_BACK_SMALLER_THAN_FOR_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* MFB FE check */\
    CMD(n, ISP_MESSAGE_UNKNOWN_FE_MODE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TOO_SMALL_FE_INPUT_XSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TOO_SMALL_FE_INPUT_YSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TOO_SMALL_TILE_WIDTH_FOR_FE_OUT_XE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TOO_SMALL_TILE_HEIGHT_FOR_FE_OUT_YE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_NOT_SUPPORT_FE_IP_WIDTH_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_NOT_SUPPORT_FE_IP_HEIGHT_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_CONFIG_FE_INPUT_SIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* ISP MUX check */\
    CMD(n, ISP_MESSAGE_ILLEGAL_PGN_SEL_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_ILLEGAL_G2G_SEL_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_ILLEGAL_G2C_SEL_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_ILLEGAL_MIX1_SEL_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_ILLEGAL_CRZ_SEL_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_ILLEGAL_SRZ1_SEL_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_ILLEGAL_NR3D_SEL_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_ILLEGAL_PAK2O_SEL_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_ILLEGAL_IMGI_FMT_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
	/* 3DNR configuration check */\
    CMD(n, ISP_MESSAGE_ILLEGAL_3DNR_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_ILLEGAL_3DNR_VALID_WINDOW_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_ILLEGAL_VIPI_CROP_XSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_ILLEGAL_VIPI_CROP_YSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_ILLEGAL_VIP2I_CROP_XSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_ILLEGAL_VIP2I_CROP_YSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_ILLEGAL_VIP3I_CROP_XSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_ILLEGAL_VIP3I_CROP_YSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
	/* SRZ check */\
    CMD(n, ISP_MESSAGE_NOT_SUPPORT_OFFSET_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_NOT_SUPPORT_VFLIP_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
	/* C02 check */\
    CMD(n, ISP_MESSAGE_INVALID_C02_EN_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_INVALID_C02B_EN_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_C02_MIN_X_IN_SIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_C02_MIN_Y_IN_SIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_C02B_MIN_X_IN_SIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_C02B_MIN_Y_IN_SIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* resizer coeff check */\
    CMD(n, ISP_MESSAGE_RESIZER_UNMATCH_INPUT_SIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* PAK2O size check */\
    CMD(n, ISP_MESSAGE_PAK2O_XSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_PAK2O_YSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_NOT_SUPPORT_PAK2O_PATH_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
	/* min size constraints */\
    CMD(n, ISP_MESSAGE_UNDER_MIN_XSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_UNDER_MIN_YSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* EAFO size check */\
    CMD(n, ISP_MESSAGE_EAFO_FOUT_XSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_EAFO_FOUT_YSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_INVALID_EAF_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
	/* WPE size check */\
    CMD(n, ISP_MESSAGE_WPEO_XSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_INVALID_CACHI_FMT_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
	/* ADL config check */\
    CMD(n, ISP_MESSAGE_INVALID_ADL_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
	/* FEO config check */\
    CMD(n, ISP_MESSAGE_INVALID_FEO_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
	/* SMX config check */\
    CMD(n, ISP_MESSAGE_INVALID_SMX_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_INVALID_SMX_OFFSET_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_INVALID_SMX_SMALL_DMA_SIZE_PARTIAL_DUMP_WARNING, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_INVALID_SMX_SMALL_DMA_SIZE_DISABLE_DUMP_WARNING, ISP_TPIPE_MESSAGE_FAIL)\
	/* BLD config check */\
    CMD(n, ISP_MESSAGE_INVALID_BLD_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* BLDI check */\
    CMD(n, ISP_MESSAGE_BLDI_XSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_BLDI_YSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\

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
    CMD(a, b, c, d, e, int, TILE_IRQ_MODE, sw.tpipe_irq_mode, last_irq_mode,, true,,, 1, TILE_IRQ_MODE,,)\
    CMD(a, b, c, d, e, int, TILE_SEL_CAL, sw.tpipe_sel_mode, tile_sel_mode,, true,,, 1, TILE_SEL_CAL,,)\

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
    CMD(a, b, c, d, e, int, CTL_IMGI_XSIZE_PIX_DIP_A, sw.src_width, isp_tile_src_width,, true,,, 0, CTL_IMGI_XSIZE_PIX_DIP_B,,)\
    CMD(a, b, c, d, e, int, CTL_IMGI_YSIZE_PIX_DIP_A, sw.src_height, isp_tile_src_height,, true,,, 0, CTL_IMGI_YSIZE_PIX_DIP_B,,)\
    CMD(a, b, c, d, e, int, TILE_WIDTH_DIP_A, sw.tpipe_width, isp_tile_width,, true,,, 1, TILE_WIDTH_DIP_B,,)\
    CMD(a, b, c, d, e, int, TILE_HEIGHT_DIP_A, sw.tpipe_height, isp_tile_height,, true,,, 1, TILE_HEIGHT_DIP_B,,)\
    CMD(a, b, c, d, e, int, LCE_A_FULL_XOFF, lce.lce_full_xoff, lce_full_xoff,, true,,, 1, LCE_B_FULL_XOFF,,)\
    CMD(a, b, c, d, e, int, LCE_A_FULL_YOFF, lce.lce_full_yoff, lce_full_yoff,, true,,, 1, LCE_B_FULL_YOFF,,)\
    CMD(a, b, c, d, e, int, LCE_A_FULL_SLM_WD, lce.lce_full_slm_width, lce_full_slm_width,, true,,, 1, LCE_B_FULL_SLM_WD,,)\
    CMD(a, b, c, d, e, int, LCE_A_FULL_SLM_HT, lce.lce_full_slm_height, lce_full_slm_height,, true,,, 1, LCE_B_FULL_SLM_HT,,)\
    CMD(a, b, c, d, e, int, LCE_A_FULL_OUT_HT, lce.lce_full_out_height, lce_full_out_height,, true,,, 1, LCE_B_FULL_OUT_HT,,)\
    CMD(a, b, c, d, e, int, UFDG_A_BOND2_MODE, ufd.ufd_bond2_mode, ufd_bond2_mode,, true,,, 1, UFDG_A_BOND2_MODE,,)\
    CMD(a, b, c, d, e, int, UFDG_A_BS3_AU_START, ufd.ufd_bs3_au_start, ufd_bs3_au_start,, true,,, 1, UFDG_A_BS3_AU_START,,)\

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
    CMD(a, b, c, d, e, int, CTL_IMGI_XSIZE_PIX_WPE_A, sw.src_width_wpe, isp_tile_src_width_wpe,, true,,, 0, CTL_IMGI_XSIZE_PIX_WPE_B,,)\
    CMD(a, b, c, d, e, int, CTL_IMGI_YSIZE_PIX_WPE_A, sw.src_height_wpe, isp_tile_src_height_wpe,, true,,, 0, CTL_IMGI_YSIZE_PIX_WPE_B,,)\
    CMD(a, b, c, d, e, int, TILE_WIDTH_WPE_A, sw.tpipe_width_wpe, isp_tile_width_wpe,, true,,, 1, TILE_WIDTH_WPE_B,,)\
    CMD(a, b, c, d, e, int, TILE_HEIGHT_WPE_A, sw.tpipe_height_wpe, isp_tile_height_wpe,, true,,, 1, TILE_HEIGHT_WPE_B,,)\
	CMD(a, b, c, d, e, int, WARP_A_EN, top.wpe_en, WPE_EN,, true,,, 0, WARP_B_EN,,)\
	CMD(a, b, c, d, e, int, WARP_B_EN, top.wpe2_en, WPE2_EN,, true,,, 0, WARP_B_EN,,)\

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
#define EAF_TILE_SW_REG_LUT(CMD, a, b, c, d, e) \
    /* IMGI input width & height */\
    CMD(a, b, c, d, e, int, CTL_IMGI_XSIZE_PIX_EAF_A, sw.src_width_eaf, isp_tile_src_width_eaf,, true,,, 0, CTL_IMGI_XSIZE_PIX_EAF_B,,)\
    CMD(a, b, c, d, e, int, CTL_IMGI_YSIZE_PIX_EAF_A, sw.src_height_eaf, isp_tile_src_height_eaf,, true,,, 0, CTL_IMGI_YSIZE_PIX_EAF_B,,)\
    CMD(a, b, c, d, e, int, TILE_WIDTH_EAF_A, sw.tpipe_width_eaf, isp_tile_width_eaf,, true,,, 1, TILE_WIDTH_EAF_B,,)\
    CMD(a, b, c, d, e, int, TILE_HEIGHT_EAF_A, sw.tpipe_height_eaf, isp_tile_height_eaf,, true,,, 1, TILE_HEIGHT_EAF_B,,)\
    CMD(a, b, c, d, e, int, EAF_CTL_EXTENSION_A_EN, top.eaf_ctl_extension_en, EAF_CTL_EXTENSION_EN,, true,,, 1, EAF_CTL_EXTENSION_A_EN,,)\
	CMD(a, b, c, d, e, int, EAF_A_PHASE, eaf.eaf_phase, EAF_PHASE,, true,,, 0, EAF_A_PHASE,,)\

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
#define BLD_TILE_SW_REG_LUT(CMD, a, b, c, d, e) \
    /* IMGI input width & height */\
    CMD(a, b, c, d, e, int, CTL_BLDI_XSIZE_PIX_BLD_A, sw.src_width_bld, isp_tile_src_width_bld,, true,,, 0, CTL_BLDI_XSIZE_PIX_BLD_A,,)\
    CMD(a, b, c, d, e, int, CTL_BLDI_YSIZE_PIX_BLD_A, sw.src_height_bld, isp_tile_src_height_bld,, true,,, 0, CTL_BLDI_YSIZE_PIX_BLD_A,,)\
    CMD(a, b, c, d, e, int, TILE_WIDTH_BLD_A, sw.tpipe_width_bld, isp_tile_width_bld,, true,,, 1, TILE_WIDTH_BLD_A,,)\
    CMD(a, b, c, d, e, int, TILE_HEIGHT_BLD_A, sw.tpipe_height_bld, isp_tile_height_bld,, true,,, 1, TILE_HEIGHT_BLD_A,,)\
    CMD(a, b, c, d, e, int, BLD_CTL_EXTENSION_A_EN, top.bld_ctl_extension_en, BLD_CTL_EXTENSION_EN,, true,,, 1, BLD_CTL_EXTENSION_A_EN,,)\

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
    CMD(a, b, c, d, e, int, DIP_PIX_ID_A, top.pixel_id, PIX_ID,, true, DIP_A_CTL_FMT_SEL, PIX_ID, 4, DIP_PIX_ID_B, DIP_A_CTL_FMT_SEL, PIX_ID)\
    CMD(a, b, c, d, e, int, IMGI_FMT_A, top.cam_in_fmt, CAM_IN_FMT,, true, DIP_A_CTL_FMT_SEL, IMGI_FMT, 4, IMGI_FMT_B, DIP_A_CTL_FMT_SEL, IMGI_FMT)\
    CMD(a, b, c, d, e, int, CTL_EXTENSION_A_EN, top.ctl_extension_en, CTL_EXTENSION_EN,, true, DIP_A_CTL_TDR_CTL, CTL_EXTENSION_EN, 4, CTL_EXTENSION_B_EN, DIP_A_CTL_TDR_CTL, CTL_EXTENSION_EN)\
    CMD(a, b, c, d, e, int, FG_MODE_A, top.fg_mode, FG_MODE,, true, DIP_A_CTL_FMT_SEL, FG_MODE, 4, FG_MODE_B, DIP_A_CTL_FMT_SEL, FG_MODE)\
    CMD(a, b, c, d, e, int, UFO_IMGI_A_EN, top.ufo_imgi_en, UFO_IMGI_EN,, true, DIP_A_SPECIAL_FUN_EN, UFO_IMGI_EN, 4, UFO_IMGI_B_EN, DIP_A_SPECIAL_FUN_EN, UFO_IMGI_EN)\
    CMD(a, b, c, d, e, int, UFDI_FMT_A, top.ufdi_fmt, UFDI_FMT,, true, DIP_A_CTL_FMT_SEL, UFDI_FMT, 4, UFDI_FMT_B, DIP_A_CTL_FMT_SEL, UFDI_FMT)\
    CMD(a, b, c, d, e, int, VIPI_FMT_A, top.vipi_fmt, VIPI_FMT,, true, DIP_A_CTL_FMT_SEL, VIPI_FMT, 4, VIPI_FMT_B, DIP_A_CTL_FMT_SEL, VIPI_FMT)\
    CMD(a, b, c, d, e, int, IMG3O_FMT_A, top.img3o_fmt, IMG3O_FMT,, true, DIP_A_CTL_FMT_SEL, IMG3O_FMT, 4, IMG3O_FMT_B, DIP_A_CTL_FMT_SEL, IMG3O_FMT)\
    CMD(a, b, c, d, e, int, IMG2O_FMT_A, top.img2o_fmt, IMG2O_FMT,, true, DIP_A_CTL_FMT_SEL, IMG2O_FMT, 4, IMG2O_FMT_B, DIP_A_CTL_FMT_SEL, IMG2O_FMT)\
    CMD(a, b, c, d, e, int, PAK2_FMT_A, top.pak2_fmt, PAK2_FMT,, true, DIP_A_CTL_MISC_SEL, PAK2_FMT, 4, PAK2_FMT_B, DIP_A_CTL_MISC_SEL, PAK2_FMT)\
    /* module enable register */\
    CMD(a, b, c, d, e, int, ADL2_A_EN, top.adl_en, ADL_EN,, true, DIP_A_CTL_DMA_EN, ADL2_EN, 4, ADL_B_EN, DIP_A_CTL_DMA_EN, ADL2_EN)\
    CMD(a, b, c, d, e, int, IMGI_A_EN, top.imgi_en, IMGI_EN,, true, DIP_A_CTL_DMA_EN, IMGI_EN, 4, IMGI_B_EN, DIP_A_CTL_DMA_EN, IMGI_EN)\
    CMD(a, b, c, d, e, int, IMGBI_A_EN, top.imgbi_en, IMGBI_EN,, true, DIP_A_CTL_DMA_EN, IMGBI_EN, 4, IMGBI_B_EN, DIP_A_CTL_DMA_EN, IMGBI_EN)\
    CMD(a, b, c, d, e, int, IMGCI_A_EN, top.imgci_en, IMGCI_EN,, true, DIP_A_CTL_DMA_EN, IMGCI_EN, 4, IMGCI_B_EN, DIP_A_CTL_DMA_EN, IMGCI_EN)\
    CMD(a, b, c, d, e, int, UFDI_A_EN, top.ufdi_en, UFDI_EN,, true, DIP_A_CTL_DMA_EN, UFDI_EN, 4, UFDI_B_EN, DIP_A_CTL_DMA_EN, UFDI_EN)\
    CMD(a, b, c, d, e, int, UNP_A_EN, top.unp_en, UNP_EN,, true, DIP_A_CTL_RGB_EN, UNP_EN, 4, UNP_B_EN, DIP_A_CTL_RGB_EN, UNP_EN)\
    CMD(a, b, c, d, e, int, UFDG_A_EN, top.ufd_en, UFD_EN,, true, DIP_A_CTL_RGB_EN, UFD_EN, 4, UFDG_B_EN, DIP_A_CTL_RGB_EN, UFD_EN)\
    CMD(a, b, c, d, e, int, DCPN2_A_EN, top.dcpn2_en, DCPN2_EN,, true, DIP_A_CTL_RGB2_EN, DCPN2_EN, 4, LSC2_B_EN, DIP_A_CTL_RGB2_EN, DCPN2_EN)\
    CMD(a, b, c, d, e, int, LSC2_A_EN, top.lsc2_en, LSC2_EN,, true, DIP_A_CTL_RGB_EN, LSC2_EN, 4, LSC2_B_EN, DIP_A_CTL_RGB_EN, LSC2_EN)\
    CMD(a, b, c, d, e, int, CPN2_A_EN, top.cpn2_en, CPN2_EN,, true, DIP_A_CTL_RGB2_EN, CPN2_EN, 4, LSC2_B_EN, DIP_A_CTL_RGB2_EN, CPN2_EN)\
    CMD(a, b, c, d, e, int, SL2_A_EN, top.sl2_en, SL2_EN,, true, DIP_A_CTL_RGB_EN, SL2_EN, 4, SL2_B_EN, DIP_A_CTL_RGB_EN, SL2_EN)\
    CMD(a, b, c, d, e, int, RNR_A_EN, top.rnr_en, RNR_EN,, true, DIP_A_CTL_RGB_EN, RNR_EN, 4, RNR_B_EN, DIP_A_CTL_RGB_EN, RNR_EN)\
    CMD(a, b, c, d, e, int, UDM_A_EN, top.udm_en, UDM_EN,, true, DIP_A_CTL_RGB_EN, UDM_EN, 4, UDM_B_EN, DIP_A_CTL_RGB_EN, UDM_EN)\
    CMD(a, b, c, d, e, int, C24_A_EN, top.c24_en, C24_EN,, true, DIP_A_CTL_YUV_EN, C24_EN, 4, C24_B_EN, DIP_A_CTL_YUV_EN, C24_EN)\
    CMD(a, b, c, d, e, int, VIPI_A_EN, top.vipi_en, VIPI_EN,, true, DIP_A_CTL_DMA_EN, VIPI_EN, 4, VIPI_B_EN, DIP_A_CTL_DMA_EN, VIPI_EN)\
    CMD(a, b, c, d, e, int, VIP2I_A_EN, top.vip2i_en, VIP2I_EN,, true, DIP_A_CTL_DMA_EN, VIP2I_EN, 4, VIP2I_B_EN, DIP_A_CTL_DMA_EN, VIP2I_EN)\
    CMD(a, b, c, d, e, int, VIP3I_A_EN, top.vip3i_en, VIP3I_EN,, true, DIP_A_CTL_DMA_EN, VIP3I_EN, 4, VIP3I_B_EN, DIP_A_CTL_DMA_EN, VIP3I_EN)\
    CMD(a, b, c, d, e, int, PAK2O_A_EN, top.pak2o_en, PAK2O_EN,, true, DIP_A_CTL_DMA_EN, PAK2O_EN, 4, PAK2O_B_EN, DIP_A_CTL_DMA_EN, PAK2O_EN)\
    CMD(a, b, c, d, e, int, G2C_A_EN, top.g2c_en, G2C_EN,, true, DIP_A_CTL_YUV_EN, G2C_EN, 4, G2C_B_EN, DIP_A_CTL_YUV_EN, G2C_EN)\
    CMD(a, b, c, d, e, int, C42_A_EN, top.c42_en, C42_EN,, true, DIP_A_CTL_YUV_EN, C42_EN, 4, C42_B_EN, DIP_A_CTL_YUV_EN, C42_EN)\
    CMD(a, b, c, d, e, int, SL2B_A_EN, top.sl2b_en, SL2B_EN,, true, DIP_A_CTL_YUV_EN, SL2B_EN, 4, SL2B_B_EN, DIP_A_CTL_YUV_EN, SL2B_EN)\
    CMD(a, b, c, d, e, int, NBC_A_EN, top.nbc_en, NBC_EN,, true, DIP_A_CTL_YUV_EN, NBC_EN, 4, NBC_B_EN, DIP_A_CTL_YUV_EN, NBC_EN)\
    CMD(a, b, c, d, e, int, NBC2_A_EN, top.nbc2_en, NBC2_EN,, true, DIP_A_CTL_YUV_EN, NBC2_EN, 4, NBC2_B_EN, DIP_A_CTL_YUV_EN, NBC2_EN)\
    CMD(a, b, c, d, e, int, MIX1_A_EN, top.mix1_en, MIX1_EN,, true, DIP_A_CTL_YUV_EN, MIX1_EN, 4, MIX1_B_EN, DIP_A_CTL_YUV_EN, MIX1_EN)\
    CMD(a, b, c, d, e, int, MIX2_A_EN, top.mix2_en, MIX2_EN,, true, DIP_A_CTL_YUV_EN, MIX2_EN, 4, MIX2_B_EN, DIP_A_CTL_YUV_EN, MIX2_EN)\
    CMD(a, b, c, d, e, int, PCA_A_EN, top.pca_en, PCA_EN,, true, DIP_A_CTL_YUV_EN, PCA_EN, 4, PCA_B_EN, DIP_A_CTL_YUV_EN, PCA_EN)\
    CMD(a, b, c, d, e, int, SL2C_A_EN, top.sl2c_en, SL2C_EN,, true, DIP_A_CTL_YUV_EN, SL2C_EN, 4, SL2C_B_EN, DIP_A_CTL_YUV_EN, SL2C_EN)\
    CMD(a, b, c, d, e, int, SL2D_A_EN, top.sl2d_en, SL2D_EN,, true, DIP_A_CTL_YUV_EN, SL2D_EN, 4, SL2D_B_EN, DIP_A_CTL_YUV_EN, SL2D_EN)\
    CMD(a, b, c, d, e, int, SL2E_A_EN, top.sl2e_en, SL2E_EN,, true, DIP_A_CTL_YUV_EN, SL2E_EN, 4, SL2E_B_EN, DIP_A_CTL_YUV_EN, SL2E_EN)\
    CMD(a, b, c, d, e, int, SL2G_A_EN, top.sl2g_en, SL2G_EN,, true, DIP_A_CTL_RGB_EN, SL2G_EN, 4, SL2G_B_EN, DIP_A_CTL_RGB_EN, SL2G_EN)\
    CMD(a, b, c, d, e, int, SL2H_A_EN, top.sl2h_en, SL2H_EN,, true, DIP_A_CTL_RGB_EN, SL2H_EN, 4, SL2H_B_EN, DIP_A_CTL_RGB_EN, SL2H_EN)\
    CMD(a, b, c, d, e, int, SL2I_A_EN, top.sl2i_en, SL2I_EN,, true, DIP_A_CTL_YUV2_EN, SL2I_EN, 4, SL2I_B_EN, DIP_A_CTL_YUV2_EN, SL2I_EN)\
    CMD(a, b, c, d, e, int, HFG_A_EN, top.hfg_en, HFG_EN,, true, DIP_A_CTL_YUV2_EN, HFG_EN, 4, HFG_B_EN, DIP_A_CTL_YUV2_EN, HFG_EN)\
	CMD(a, b, c, d, e, int, SEEE_A_EN, top.seee_en, SEEE_EN,, true, DIP_A_CTL_YUV_EN, SEEE_EN, 4, SEEE_B_EN, DIP_A_CTL_YUV_EN, SEEE_EN)\
    CMD(a, b, c, d, e, int, LCEI_A_EN, top.lcei_en, LCEI_EN,, true, DIP_A_CTL_DMA_EN, LCEI_EN, 4, LCEI_B_EN, DIP_A_CTL_DMA_EN, LCEI_EN)\
    CMD(a, b, c, d, e, int, LCE_A_EN, top.lce_en, LCE_EN,, true, DIP_A_CTL_RGB_EN, LCE_EN, 4, LCE_B_EN, DIP_A_CTL_RGB_EN, LCE_EN)\
    CMD(a, b, c, d, e, int, MIX3_A_EN, top.mix3_en, MIX3_EN,, true, DIP_A_CTL_YUV_EN, MIX3_EN, 4, MIX3_B_EN, DIP_A_CTL_YUV_EN, MIX3_EN)\
    CMD(a, b, c, d, e, int, MIX4_A_EN, top.mix4_en, MIX4_EN,, true, DIP_A_CTL_YUV2_EN, MIX4_EN, 4, MIX4_B_EN, DIP_A_CTL_YUV2_EN, MIX4_EN)\
    CMD(a, b, c, d, e, int, CRZ_A_EN, top.crz_en, CDRZ_EN,, true, DIP_A_CTL_YUV_EN, CRZ_EN, 4, CRZ_B_EN, DIP_A_CTL_YUV_EN, CRZ_EN)\
    CMD(a, b, c, d, e, int, IMG2O_A_EN, top.img2o_en, IMG2O_EN,, true, DIP_A_CTL_DMA_EN, IMG2O_EN, 4, IMG2O_B_EN, DIP_A_CTL_DMA_EN, IMG2O_EN)\
    CMD(a, b, c, d, e, int, IMG2BO_A_EN, top.img2bo_en, IMG2BO_EN,, true, DIP_A_CTL_DMA_EN, IMG2BO_EN, 4, IMG2BO_B_EN, DIP_A_CTL_DMA_EN, IMG2BO_EN)\
    CMD(a, b, c, d, e, int, SRZ1_A_EN, top.srz1_en, SRZ1_EN,, true, DIP_A_CTL_YUV_EN, SRZ1_EN, 4, SRZ1_B_EN, DIP_A_CTL_YUV_EN, SRZ1_EN)\
    CMD(a, b, c, d, e, int, FE_A_EN, top.fe_en, FE_EN,, true, DIP_A_CTL_YUV_EN, FE_EN, 4, FE_B_EN, DIP_A_CTL_YUV_EN, FE_EN)\
    CMD(a, b, c, d, e, int, FEO_A_EN, top.feo_en, FEO_EN,, true, DIP_A_CTL_DMA_EN, FEO_EN, 4, FEO_B_EN, DIP_A_CTL_DMA_EN, FEO_EN)\
    CMD(a, b, c, d, e, int, C02_A_EN, top.c02_en, C02_EN,, true, DIP_A_CTL_YUV_EN, C02_EN, 4, C02_B_EN, DIP_A_CTL_YUV_EN, C02_EN)\
    CMD(a, b, c, d, e, int, C02B_A_EN, top.c02b_en, C02B_EN,, true, DIP_A_CTL_YUV_EN, C02B_EN, 4, C02B_B_EN, DIP_A_CTL_YUV_EN, C02B_EN)\
    CMD(a, b, c, d, e, int, NR3D_A_EN, top.nr3d_en, NR3D_EN,, true, DIP_A_CTL_YUV_EN, NR3D_EN, 4, NR3D_B_EN, DIP_A_CTL_YUV_EN, NR3D_EN)\
    CMD(a, b, c, d, e, int, CAMCOLOR_A_EN, top.color_en, COLOR_EN,, true, DIP_A_CTL_YUV_EN, COLOR_EN, 4, CAMCOLOR_B_EN, DIP_A_CTL_YUV_EN, COLOR_EN)\
    CMD(a, b, c, d, e, int, CRSP_A_EN, top.crsp_en, CRSP_EN,, true, DIP_A_CTL_YUV_EN, CRSP_EN, 4, CRSP_B_EN, DIP_A_CTL_YUV_EN, CRSP_EN)\
    CMD(a, b, c, d, e, int, IMG3O_A_EN, top.img3o_en, IMG3O_EN,, true, DIP_A_CTL_DMA_EN, IMG3O_EN, 4, IMG3O_B_EN, DIP_A_CTL_DMA_EN, IMG3O_EN)\
    CMD(a, b, c, d, e, int, IMG3BO_A_EN, top.img3bo_en, IMG3BO_EN,, true, DIP_A_CTL_DMA_EN, IMG3BO_EN, 4, IMG3BO_B_EN, DIP_A_CTL_DMA_EN, IMG3BO_EN)\
    CMD(a, b, c, d, e, int, IMG3CO_A_EN, top.img3co_en, IMG3CO_EN,, true, DIP_A_CTL_DMA_EN, IMG3CO_EN, 4, IMG3CO_B_EN, DIP_A_CTL_DMA_EN, IMG3CO_EN)\
    CMD(a, b, c, d, e, int, C24B_A_EN, top.c24b_en, C24B_EN,, true, DIP_A_CTL_YUV_EN, C24B_EN, 4, C24B_B_EN, DIP_A_CTL_YUV_EN, C24B_EN)\
    CMD(a, b, c, d, e, int, MDP_CROP_A_EN, top.mdp_crop_en, MDP_CROP_EN,, true, DIP_A_CTL_YUV_EN, MDPCROP_EN, 4, MDP_CROP_B_EN, DIP_A_CTL_YUV_EN, MDPCROP_EN)\
    CMD(a, b, c, d, e, int, MDP_CROP2_A_EN, top.mdp_crop2_en, MDP_CROP2_EN,, true, DIP_A_CTL_RGB_EN, MDPCROP2_EN, 4, MDP_CROP_B_EN, DIP_A_CTL_RGB_EN, MDPCROP2_EN)\
    CMD(a, b, c, d, e, int, SRZ2_A_EN, top.srz2_en, SRZ2_EN,, true, DIP_A_CTL_YUV_EN, SRZ2_EN, 4, SRZ2_B_EN, DIP_A_CTL_YUV_EN, SRZ2_EN)\
    CMD(a, b, c, d, e, int, PGN_A_EN, top.pgn_en, PGN_EN,, true, DIP_A_CTL_RGB_EN, PGN_EN, 4, PGN_B_EN, DIP_A_CTL_RGB_EN, PGN_EN)\
    CMD(a, b, c, d, e, int, G2G_A_EN, top.g2g_en, G2G_EN,, true, DIP_A_CTL_RGB_EN, G2G_EN, 4, G2G_B_EN, DIP_A_CTL_RGB_EN, G2G_EN)\
    CMD(a, b, c, d, e, int, FLC_A_EN, top.flc_en, FLC_EN,, true, DIP_A_CTL_RGB_EN, FLC_EN, 4, FLC_A_EN, DIP_A_CTL_RGB_EN, FLC_EN)\
    CMD(a, b, c, d, e, int, FLC2_A_EN, top.flc2_en, FLC2_EN,, true, DIP_A_CTL_RGB_EN, FLC2_EN, 4, FLC2_A_EN, DIP_A_CTL_RGB_EN, FLC2_EN)\
    CMD(a, b, c, d, e, int, GGM_A_EN, top.ggm_en, GGM_EN,, true, DIP_A_CTL_RGB_EN, GGM_EN, 4, GGM_B_EN, DIP_A_CTL_RGB_EN, GGM_EN)\
	CMD(a, b, c, d, e, int, DMGI_A_EN, top.dmgi_en, DMGI_EN,, true, DIP_A_CTL_DMA_EN, DMGI_EN, 4, DMGI_B_EN, DIP_A_CTL_DMA_EN, DMGI_EN)\
	CMD(a, b, c, d, e, int, DEPI_A_EN, top.depi_en, DEPI_EN,, true, DIP_A_CTL_DMA_EN, DEPI_EN, 4, DEPI_B_EN, DIP_A_CTL_DMA_EN, DEPI_EN)\
	CMD(a, b, c, d, e, int, PLNR1_A_EN, top.plnr1_en, PLNR1_EN,, true, DIP_A_CTL_YUV_EN, PLNR1_EN, 4, PLNR1_B_EN, DIP_A_CTL_YUV_EN, PLNR1_EN)\
	CMD(a, b, c, d, e, int, PLNR2_A_EN, top.plnr2_en, PLNR2_EN,, true, DIP_A_CTL_YUV_EN, PLNR2_EN, 4, PLNR2_B_EN, DIP_A_CTL_YUV_EN, PLNR2_EN)\
	CMD(a, b, c, d, e, int, PLNW1_A_EN, top.plnw1_en, PLNW1_EN,, true, DIP_A_CTL_YUV_EN, PLNW1_EN, 4, PLNW1_B_EN, DIP_A_CTL_YUV_EN, PLNW1_EN)\
	CMD(a, b, c, d, e, int, PLNW2_A_EN, top.plnw2_en, PLNW2_EN,, true, DIP_A_CTL_YUV_EN, PLNW2_EN, 4, PLNW2_B_EN, DIP_A_CTL_YUV_EN, PLNW2_EN)\
	CMD(a, b, c, d, e, int, DBS2_A_EN, top.dbs2_en, DBS2_EN,, true, DIP_A_CTL_RGB_EN, DBS2_EN, 4, DBS2_B_EN, DIP_A_CTL_RGB_EN, DBS2_EN)\
	CMD(a, b, c, d, e, int, ADBS2_A_EN, top.adbs2_en, ADBS2_EN,, true, DIP_A_CTL_RGB2_EN, ADBS2_EN, 4, ADBS2_B_EN, DIP_A_CTL_RGB2_EN, ADBS2_EN)\
	CMD(a, b, c, d, e, int, BNR2_A_EN, top.bnr2_en, BNR2_EN,, true, DIP_A_CTL_RGB_EN, BNR2_EN, 4, BNR2_B_EN, DIP_A_CTL_RGB_EN, BNR2_EN)\
	CMD(a, b, c, d, e, int, FM_A_EN, top.fm_en, FM_EN,, true, DIP_A_CTL_YUV2_EN, FM_EN, 2, FM_B_EN, DIP_A_CTL_YUV2_EN, FM_EN)\
	CMD(a, b, c, d, e, int, GDR1_A_EN, top.gdr1_en, GDR1_EN,, true, DIP_A_CTL_RGB_EN, GDR1_EN, 4, GDR1_B_EN, DIP_A_CTL_RGB_EN, GDR1_EN)\
	CMD(a, b, c, d, e, int, GDR2_A_EN, top.gdr2_en, GDR2_EN,, true, DIP_A_CTL_RGB_EN, GDR2_EN, 4, GDR2_B_EN, DIP_A_CTL_RGB_EN, GDR2_EN)\
	CMD(a, b, c, d, e, int, OBC2_A_EN, top.obc2_en, OBC2_EN,, true, DIP_A_CTL_RGB_EN, OBC2_EN, 4, OBC2_B_EN, DIP_A_CTL_RGB_EN, OBC2_EN)\
	CMD(a, b, c, d, e, int, RMG2_A_EN, top.rmg2_en, RMG2_EN,, true, DIP_A_CTL_RGB_EN, RMG2_EN, 4, RMG2_B_EN, DIP_A_CTL_RGB_EN, RMG2_EN)\
	CMD(a, b, c, d, e, int, RMM2_A_EN, top.rmm2_en, RMM2_EN,,true, DIP_A_CTL_RGB_EN, RMM2_EN, 4, RMM2_B_EN, DIP_A_CTL_RGB_EN, RMM2_EN)\
    CMD(a, b, c, d, e, int, SRZ3_A_EN, top.srz3_en, SRZ3_EN,, true, DIP_A_CTL_YUV2_EN, SRZ3_EN, 4, SRZ3_B_EN, DIP_A_CTL_YUV2_EN, SRZ3_EN)\
    CMD(a, b, c, d, e, int, SRZ4_A_EN, top.srz4_en, SRZ4_EN,, true, DIP_A_CTL_YUV2_EN, SRZ4_EN, 4, SRZ4_B_EN, DIP_A_CTL_YUV2_EN, SRZ4_EN)\
    CMD(a, b, c, d, e, int, RCP2_A_EN, top.rcp2_en, RCP2_EN,, true, DIP_A_CTL_RGB_EN, RCP2_EN, 4, RCP2_B_EN, DIP_A_CTL_RGB_EN, RCP2_EN)\
    CMD(a, b, c, d, e, int, PAK2_A_EN, top.pak2_en, PAK2_EN,, true, DIP_A_CTL_RGB_EN, PAK2_EN, 4, PAK2_B_EN, DIP_A_CTL_RGB_EN, PAK2_EN)\
    CMD(a, b, c, d, e, int, NDG_A_EN, top.ndg_en, NDG_EN,, true, DIP_A_CTL_YUV2_EN, NDG_EN, 4, NDG_B_EN, DIP_A_CTL_YUV2_EN, NDG_EN)\
    CMD(a, b, c, d, e, int, NDG2_A_EN, top.ndg2_en, NDG2_EN,, true, DIP_A_CTL_YUV2_EN, NDG2_EN, 4, NDG2_B_EN, DIP_A_CTL_YUV2_EN, NDG2_EN)\
    CMD(a, b, c, d, e, int, G2G2_A_EN, top.g2g2_en, G2G2_EN,, true, DIP_A_CTL_RGB_EN, G2G2_EN, 4, G2G2_B_EN, DIP_A_CTL_RGB_EN, G2G2_EN)\
    CMD(a, b, c, d, e, int, GGM2_A_EN, top.ggm2_en, GGM2_EN,, true, DIP_A_CTL_RGB_EN, GGM2_EN, 4, GGM2_B_EN, DIP_A_CTL_RGB_EN, GGM2_EN)\
    CMD(a, b, c, d, e, int, WSHIFT_A_EN, top.wshift_en, WSHIFT_EN,, true, DIP_A_CTL_RGB_EN, WSHIFT_EN, 4, WSHIFT_B_EN, DIP_A_CTL_RGB_EN, WSHIFT_EN)\
    CMD(a, b, c, d, e, int, WSYNC_A_EN, top.wsync_en, WSYNC_EN,, true, DIP_A_CTL_RGB_EN, WSYNC_EN, 4, WSYNC_B_EN, DIP_A_CTL_RGB_EN, WSYNC_EN)\
    CMD(a, b, c, d, e, int, UFDG_A_BOND_MODE, ufd.ufd_bond_mode, ufd_bond_mode,, true, DIP_A_UFDG_CON, UFDG_BOND_MODE, 1, UFDG_A_BOND_MODE, DIP_A_UFDG_CON, UFDG_BOND_MODE)\
    CMD(a, b, c, d, e, int, UFDG_A_BS2_AU_START, ufd.ufd_bs2_au_start, ufd_bs2_au_start,, true, DIP_A_UFDG_BS_AU_CON, UFDG_BS_AU_START, 1, UFDG_A_BS2_AU_START, DIP_A_UFDG_BS_AU_CON, UFDG_BS_AU_START)\
    CMD(a, b, c, d, e, int, UFD_A_SEL, ufd.ufd_sel, ufd_sel,, true, DIP_A_UFDG_CON, UFOD_SEL, 1, UFD_B_SEL, DIP_A_UFDG_CON, UFOD_SEL)\
	/* SMX1 */\
	CMD(a, b, c, d, e, int, SMX1_A_EN, top.smx1_en, SMX1_EN,, true, DIP_A_CTL_RGB_EN, SMX1_EN, 4, SMX1_B_EN, DIP_A_CTL_RGB_EN, SMX1_EN)\
    CMD(a, b, c, d, e, int, PAKG2_A_EN, top.pakg2_en, PAKG2_EN,, true, DIP_A_CTL_RGB_EN, PAKG2_EN, 1, PAKG2_B_EN, DIP_A_CTL_RGB_EN, PAKG2_EN)\
	CMD(a, b, c, d, e, int, SMX2_A_EN, top.smx2_en, SMX2_EN,, true, DIP_A_CTL_YUV2_EN, SMX2_EN, 4, SMX2_B_EN, DIP_A_CTL_YUV2_EN, SMX2_EN)\
	CMD(a, b, c, d, e, int, SMX3_A_EN, top.smx3_en, SMX3_EN,, true, DIP_A_CTL_YUV2_EN, SMX3_EN, 4, SMX3_B_EN, DIP_A_CTL_YUV2_EN, SMX3_EN)\
	CMD(a, b, c, d, e, int, SMX4_A_EN, top.smx4_en, SMX4_EN,, true, DIP_A_CTL_RGB2_EN, SMX4_EN, 4, SMX4_B_EN, DIP_A_CTL_RGB2_EN, SMX4_EN)\
	CMD(a, b, c, d, e, int, SMX1I_A_EN, top.smx1i_en, SMX1I_EN,, true, DIP_A_CTL_DMA_EN, SMX1I_EN, 1, SMX1I_B_EN, DIP_A_CTL_DMA_EN, SMX1I_EN)\
	CMD(a, b, c, d, e, int, SMX1O_A_EN, top.smx1o_en, SMX1O_EN,, true, DIP_A_CTL_DMA_EN, SMX1O_EN, 1, SMX1O_B_EN, DIP_A_CTL_DMA_EN, SMX1O_EN)\
	CMD(a, b, c, d, e, int, SMX2I_A_EN, top.smx2i_en, SMX2I_EN,, true, DIP_A_CTL_DMA_EN, SMX2I_EN, 1, SMX2I_B_EN, DIP_A_CTL_DMA_EN, SMX2I_EN)\
	CMD(a, b, c, d, e, int, SMX2O_A_EN, top.smx2o_en, SMX2O_EN,, true, DIP_A_CTL_DMA_EN, SMX2O_EN, 1, SMX2O_B_EN, DIP_A_CTL_DMA_EN, SMX2O_EN)\
	CMD(a, b, c, d, e, int, SMX3I_A_EN, top.smx3i_en, SMX3I_EN,, true, DIP_A_CTL_DMA_EN, SMX3I_EN, 1, SMX3I_B_EN, DIP_A_CTL_DMA_EN, SMX3I_EN)\
	CMD(a, b, c, d, e, int, SMX3O_A_EN, top.smx3o_en, SMX3O_EN,, true, DIP_A_CTL_DMA_EN, SMX3O_EN, 1, SMX3O_B_EN, DIP_A_CTL_DMA_EN, SMX3O_EN)\
	CMD(a, b, c, d, e, int, SMX4I_A_EN, top.smx4i_en, SMX4I_EN,, true, DIP_A_CTL_DMA_EN, SMX4I_EN, 1, SMX4I_B_EN, DIP_A_CTL_DMA_EN, SMX4I_EN)\
	CMD(a, b, c, d, e, int, SMX4O_A_EN, top.smx4o_en, SMX4O_EN,, true, DIP_A_CTL_DMA_EN, SMX4O_EN, 1, SMX4O_B_EN, DIP_A_CTL_DMA_EN, SMX4O_EN)\
	/* MUX */\
    CMD(a, b, c, d, e, int, PGN_A_SEL, top.pgn_sel, PGN_SEL,, true, DIP_A_CTL_PATH_SEL, PGN_SEL, 4, PGN_B_SEL, DIP_A_CTL_PATH_SEL, PGN_SEL)\
    CMD(a, b, c, d, e, int, RCP2_A_SEL, top.rcp2_sel, RCP2_SEL,, true, DIP_A_CTL_PATH_SEL, RCP2_SEL, 4, RCP2_B_SEL, DIP_A_CTL_PATH_SEL, RCP2_SEL)\
    CMD(a, b, c, d, e, int, G2G_A_SEL, top.g2g_sel, G2G_SEL,, true, DIP_A_CTL_PATH_SEL, G2G_SEL, 4, G2G_B_SEL, DIP_A_CTL_PATH_SEL, G2G_SEL)\
    CMD(a, b, c, d, e, int, G2C_A_SEL, top.g2c_sel, G2C_SEL,, true, DIP_A_CTL_PATH_SEL, G2C_SEL, 4, G2C_B_SEL, DIP_A_CTL_PATH_SEL, G2C_SEL)\
    CMD(a, b, c, d, e, int, SRZ1_A_SEL, top.srz1_sel, SRZ1_SEL,, true, DIP_A_CTL_PATH_SEL, SRZ1_SEL, 4, SRZ1_B_SEL, DIP_A_CTL_PATH_SEL, SRZ1_SEL)\
    CMD(a, b, c, d, e, int, MIX1_A_SEL, top.mix1_sel, MIX1_SEL,, true, DIP_A_CTL_PATH_SEL, MIX1_SEL, 4, MIX1_B_SEL, DIP_A_CTL_PATH_SEL, MIX1_SEL)\
    CMD(a, b, c, d, e, int, CRZ_A_SEL, top.crz_sel, CRZ_SEL,, true, DIP_A_CTL_PATH_SEL, CRZ_SEL, 4, CRZ_B_SEL, DIP_A_CTL_PATH_SEL, CRZ_SEL)\
    CMD(a, b, c, d, e, int, NR3D_A_SEL, top.nr3d_sel, NR3D_SEL,, true, DIP_A_CTL_PATH_SEL, NR3D_SEL, 4, NR3D_B_SEL, DIP_A_CTL_PATH_SEL, NR3D_SEL)\
    CMD(a, b, c, d, e, int, FE_A_SEL, top.fe_sel, FE_SEL,, true, DIP_A_CTL_PATH_SEL, FE_SEL, 4, FE_B_SEL, DIP_A_CTL_PATH_SEL, FE_SEL)\
    CMD(a, b, c, d, e, int, MDP_A_SEL, top.mdp_sel, MDP_SEL,, true, DIP_A_CTL_PATH_SEL, MDP_SEL, 4, MDP_B_SEL, DIP_A_CTL_PATH_SEL, MDP_SEL)\
    CMD(a, b, c, d, e, int, NBC_A_SEL, top.nbc_sel, NBC_SEL,, true, DIP_A_CTL_PATH_SEL, NBC_SEL, 4, NBC_B_SEL, DIP_A_CTL_PATH_SEL, NBC_SEL)\
    CMD(a, b, c, d, e, int, PAK2O_A_SEL, top.pak2o_sel, PAK2O_SEL,, true, DIP_A_CTL_MISC_SEL, PAK2O_SEL, 4, PAK2O_B_SEL, DIP_A_CTL_MISC_SEL, PAK2O_SEL)\
    CMD(a, b, c, d, e, int, CRSP_A_SEL, top.crsp_sel, CRSP_SEL,, true, DIP_A_CTL_PATH_SEL, CRSP_SEL, 4, CRSP_B_SEL, DIP_A_CTL_PATH_SEL, CRSP_SEL)\
    CMD(a, b, c, d, e, int, IMGI_A_SEL, top.imgi_sel, IMGI_SEL,, true, DIP_A_CTL_PATH_SEL, IMGI_SEL, 4, IMGI_B_SEL, DIP_A_CTL_PATH_SEL, IMGI_SEL)\
    CMD(a, b, c, d, e, int, GGM_A_SEL, top.ggm_sel, GGM_SEL,, true, DIP_A_CTL_PATH_SEL, GGM_SEL, 4, GGM_B_SEL, DIP_A_CTL_PATH_SEL, GGM_SEL)\
    CMD(a, b, c, d, e, int, SRC_A_SEL, top.src_sel, SRC_SEL,, true, DIP_A_ADL_CTL, SRC_SEL, 4, SRC_B_SEL, DIP_A_ADL_CTL, SRC_SEL)\
    CMD(a, b, c, d, e, int, DST_A_SEL, top.dst_sel, DST_SEL,, true, DIP_A_ADL_CTL, DST_SEL, 4, DST_B_SEL, DIP_A_ADL_CTL, DST_SEL)\
    CMD(a, b, c, d, e, int, WPE_A_SEL, top.wpe_sel, WPE_SEL,, true, DIP_A_CTL_PATH_SEL, WPE_SEL, 4, WPE_B_SEL, DIP_A_CTL_PATH_SEL, WPE_SEL)\
    CMD(a, b, c, d, e, int, FEO_A_SEL, top.feo_sel, FEO_SEL,, true, DIP_A_CTL_PATH_SEL, FEO_SEL, 4, FEO_B_SEL, DIP_A_CTL_PATH_SEL, FEO_SEL)\
	CMD(a, b, c, d, e, int, G2G2_A_SEL, top.g2g2_sel, G2G2_SEL,, true, DIP_A_CTL_PATH_SEL, G2G2_SEL, 4, G2G2_B_SEL, DIP_A_CTL_PATH_SEL, G2G2_SEL)\
    CMD(a, b, c, d, e, int, NBC_GMAP_LTM_MODE_A, top.nbc_gmap_ltm_mode, NBC_GMAP_LTM_MODE,, true, DIP_A_CTL_MISC_SEL, NBC_GMAP_LTM_MODE, 4, NBC_GMAP_LTM_MODE_B, DIP_A_CTL_MISC_SEL, NBC_GMAP_LTM_MODE)\
	CMD(a, b, c, d, e, int, WUV_MODE_A, top.wuv_mode, WUV_MODE,, true, DIP_A_CTL_MISC_SEL, WUV_MODE, 4, WUV_MODE_B, DIP_A_CTL_MISC_SEL, WUV_MODE)\
	/* interlace mode */\
    CMD(a, b, c, d, e, int, INTERLACE_MODE_A, top.interlace_mode, INTERLACE_MODE,, true, DIP_A_SPECIAL_FUN_EN, INTERLACE_MODE, 4, INTERLACE_MODE_B, DIP_A_SPECIAL_FUN_EN, INTERLACE_MODE)\
	/* SMX1 */\
    CMD(a, b, c, d, e, int, SMX1O_A_SEL, smx1.smx1o_sel, SMX1O_SEL,, REG_CHECK_EN(c, SMX1_EN), DIP_A_SMX1_CTL, SMXO_SEL, 4, SMX1O_B_SEL, DIP_A_SMX1_CTL, SMXO_SEL)\
    CMD(a, b, c, d, e, int, SMX2O_A_SEL, smx2.smx2o_sel, SMX2O_SEL,, REG_CHECK_EN(c, SMX2_EN), DIP_A_SMX2_CTL, SMXO_SEL, 4, SMX2O_B_SEL, DIP_A_SMX2_CTL, SMXO_SEL)\
    CMD(a, b, c, d, e, int, SMX3O_A_SEL, smx3.smx3o_sel, SMX3O_SEL,, REG_CHECK_EN(c, SMX3_EN), DIP_A_SMX3_CTL, SMXO_SEL, 4, SMX3O_B_SEL, DIP_A_SMX3_CTL, SMXO_SEL)\
    CMD(a, b, c, d, e, int, SMX4O_A_SEL, smx4.smx4o_sel, SMX4O_SEL,, REG_CHECK_EN(c, SMX4_EN), DIP_A_SMX4_CTL, SMXO_SEL, 4, SMX4O_B_SEL, DIP_A_SMX3_CTL, SMXO_SEL)\
	/* ADL */\
    CMD(a, b, c, d, e, int, ADL2_CTL_EN, adl.adl_ctl_en, ADL_CTL_EN,, REG_CMP_EQ(c, ADL_EN, 1), DIP_A_ADL_CTL, ENABLE, 4, ADL2_CTL_EN, DIP_A_ADL_CTL, ENABLE)\
	CMD(a, b, c, d, e, int, ADL_A_IPUI_STRIDE, adl.ipui_stride, IPUI_STRIDE,, REG_CMP_EQ(c, ADL_EN, 1), DIP_A_ADL_DMA_A_IPUI_STRIDE, STRIDE, 4, ADL_A_IPUI_STRIDE, DIP_A_ADL_DMA_A_IPUI_STRIDE, STRIDE)\
    CMD(a, b, c, d, e, int, ADL_A_IPUO_STRIDE, adl.ipuo_stride, IPUO_STRIDE,, REG_CMP_EQ(c, ADL_EN, 1), DIP_A_ADL_DMA_A_IPUO_STRIDE, STRIDE, 4, ADL_A_IPUO_STRIDE, DIP_A_ADL_DMA_A_IPUO_STRIDE, STRIDE)\
    /* IMGI */\
    CMD(a, b, c, d, e, int, IMGI_A_V_FLIP_EN, imgi.imgi_v_flip_en, IMGI_V_FLIP_EN,, REG_CMP_EQ(c, IMGI_EN, 1), DIP_A_VERTICAL_FLIP_EN, IMGI_V_FLIP_EN, 4, IMGI_B_V_FLIP_EN, DIP_A_VERTICAL_FLIP_EN, IMGI_V_FLIP_EN)\
    CMD(a, b, c, d, e, int, IMGI_A_STRIDE, imgi.imgi_stride, IMGI_STRIDE,, REG_CMP_EQ(c, IMGI_EN, 1), DIP_A_IMGI_STRIDE, STRIDE, 4, IMGI_B_STRIDE, DIP_A_IMGI_STRIDE, STRIDE)\
	/* IMGBI */\
	CMD(a, b, c, d, e, int, IMGBI_A_OFFSET_ADDR, imgbi.imgbi_offset, IMGBI_OFFSET_ADDR,, REG_CMP_EQ(c, LOG_IMGBI_EN, 1), DIP_A_IMGBI_OFST_ADDR, OFFSET_ADDR, 1, IMGBI_B_OFFSET_ADDR, DIP_A_IMGBI_OFST_ADDR, OFFSET_ADDR)\
    CMD(a, b, c, d, e, int, IMGBI_A_V_FLIP_EN, imgbi.imgbi_v_flip_en, IMGBI_V_FLIP_EN,, REG_CMP_EQ(c, LOG_IMGBI_EN, 1), DIP_A_VERTICAL_FLIP_EN, IMGBI_V_FLIP_EN, 4, IMGBI_B_V_FLIP_EN, DIP_A_VERTICAL_FLIP_EN, IMGBI_V_FLIP_EN)\
    CMD(a, b, c, d, e, int, IMGBI_A_XSIZE, imgbi.imgbi_xsize, IMGBI_XSIZE,, REG_CMP_EQ(c, LOG_IMGBI_EN, 1), DIP_A_IMGBI_XSIZE, XSIZE, 1, IMGBI_B_XSIZE, DIP_A_IMGBI_XSIZE, XSIZE)\
    CMD(a, b, c, d, e, int, IMGBI_A_YSIZE, imgbi.imgbi_ysize, IMGBI_YSIZE,, REG_CMP_EQ(c, LOG_IMGBI_EN, 1), DIP_A_IMGBI_YSIZE, YSIZE, 1, IMGBI_B_YSIZE, DIP_A_IMGBI_YSIZE, YSIZE)\
    CMD(a, b, c, d, e, int, IMGBI_A_STRIDE, imgbi.imgbi_stride, IMGBI_STRIDE,, REG_CMP_EQ(c, LOG_IMGBI_EN, 1), DIP_A_IMGBI_STRIDE, STRIDE, 4, IMGBI_B_STRIDE, DIP_A_IMGBI_STRIDE, STRIDE)\
	/* IMGCI */\
    CMD(a, b, c, d, e, int, IMGCI_A_V_FLIP_EN, imgci.imgci_v_flip_en, IMGCI_V_FLIP_EN,, REG_CMP_EQ(c, IMGCI_EN, 1), DIP_A_VERTICAL_FLIP_EN, IMGCI_V_FLIP_EN, 4, IMGCI_B_V_FLIP_EN, DIP_A_VERTICAL_FLIP_EN, IMGCI_V_FLIP_EN)\
    CMD(a, b, c, d, e, int, IMGCI_A_STRIDE, imgci.imgci_stride, IMGCI_STRIDE,, REG_CMP_EQ(c, LOG_IMGCI_EN, 1), DIP_A_IMGCI_STRIDE, STRIDE, 4, IMGCI_B_STRIDE, DIP_A_IMGCI_STRIDE, STRIDE)\
    /* UFDI */\
    CMD(a, b, c, d, e, int, UFDI_A_V_FLIP_EN, ufdi.ufdi_v_flip_en, UFDI_V_FLIP_EN,, REG_CMP_EQ(c, LOG_UFDI_EN, 1), DIP_A_VERTICAL_FLIP_EN, UFDI_V_FLIP_EN, 4, UFDI_B_V_FLIP_EN, DIP_A_VERTICAL_FLIP_EN, UFDI_V_FLIP_EN)\
    CMD(a, b, c, d, e, int, UFDI_A_XSIZE, ufdi.ufdi_xsize, UFDI_XSIZE,, REG_CMP_EQ(c, LOG_UFDI_EN, 1), DIP_A_UFDI_XSIZE, XSIZE, 0, UFDI_B_XSIZE, DIP_A_UFDI_XSIZE, XSIZE)\
    CMD(a, b, c, d, e, int, UFDI_A_YSIZE, ufdi.ufdi_ysize, UFDI_YSIZE,, REG_CMP_EQ(c, LOG_UFDI_EN, 1), DIP_A_UFDI_YSIZE, YSIZE, 0, UFDI_B_YSIZE, DIP_A_UFDI_YSIZE, YSIZE)\
    CMD(a, b, c, d, e, int, UFDI_A_STRIDE, ufdi.ufdi_stride, UFDI_STRIDE,, REG_CMP_EQ(c, LOG_UFDI_EN, 1), DIP_A_UFDI_STRIDE, STRIDE, 4, UFDI_B_STRIDE, DIP_A_UFDI_STRIDE, STRIDE)\
    /* 2D BPC */\
    CMD(a, b, c, d, e, int, BPC_tmp5_EN, bnr.bpc_en, BPC_ENABLE,, REG_CMP_EQ(c, LOG_BNR_EN, 1), DIP_A_BNR2_BPC_CON, BPC_EN, 4, BPC_tmp6_EN, DIP_A_BNR2_BPC_CON, BPC_EN)\
    CMD(a, b, c, d, e, int, BPC_tmp5_LUT_EN, bnr.bpc_tbl_en, BPC_TBL_EN,, REG_CMP_EQ(c, LOG_BNR_EN, 1), DIP_A_BNR2_BPC_CON, BPC_LUT_EN, 4, BPC_tmp6_LUT_EN, DIP_A_BNR2_BPC_CON, BPC_LUT_EN)\
	/* RMG */\
    CMD(a, b, c, d, e, int, RMG_tmp5_IHDR_EN, rmg.rmg_ihdr_en, RMG_IHDR_EN,, REG_CMP_EQ(c, LOG_RMG2_EN, 1), DIP_A_RMG2_HDR_CFG, RMG_IHDR_EN, 4, RMG_tmp6_IHDR_EN, DIP_A_RMG2_HDR_CFG, RMG_IHDR_EN)\
    CMD(a, b, c, d, e, int, RMG_tmp5_ZHDR_EN, rmg.rmg_zhdr_en, RMG_ZHDR_EN,, REG_CMP_EQ(c, LOG_RMG2_EN, 1), DIP_A_RMG2_HDR_CFG, RMG_ZHDR_EN, 4, RMG_tmp6_ZHDR_EN, DIP_A_RMG2_HDR_CFG, RMG_ZHDR_EN)\
    /* LSC2 */\
    CMD(a, b, c, d, e, int, LSC2_tmp1_EXTEND_COEF_MODE, lsc2.extend_coef_mode, EXTEND_COEF_MODE,, REG_CMP_EQ(c, LOG_LSC2_EN, 1), DIP_A_LSC2_CTL1, LSC_EXTEND_COEF_MODE, 0, LSC2_tmp2_EXTEND_COEF_MODE, DIP_A_LSC2_CTL1, LSC_EXTEND_COEF_MODE)\
    CMD(a, b, c, d, e, int, LSC2_tmp1_SDBLK_XNUM, lsc2.sdblk_xnum, SDBLK_XNUM,, REG_CMP_EQ(c, LOG_LSC2_EN, 1), DIP_A_LSC2_CTL2, LSC_SDBLK_XNUM, 0, LSC2_tmp2_SDBLK_XNUM, DIP_A_LSC2_CTL2, LSC_SDBLK_XNUM)\
    CMD(a, b, c, d, e, int, LSC2_tmp1_SDBLK_YNUM, lsc2.sdblk_ynum, SDBLK_YNUM,, REG_CMP_EQ(c, LOG_LSC2_EN, 1), DIP_A_LSC2_CTL3, LSC_SDBLK_YNUM, 0, LSC2_tmp2_SDBLK_YNUM, DIP_A_LSC2_CTL3, LSC_SDBLK_YNUM)\
    CMD(a, b, c, d, e, int, LSC2_tmp1_SDBLK_WIDTH, lsc2.sdblk_width, SDBLK_WIDTH,, REG_CMP_EQ(c, LOG_LSC2_EN, 1), DIP_A_LSC2_CTL2, LSC_SDBLK_WIDTH, 0, LSC2_tmp2_SDBLK_WIDTH, DIP_A_LSC2_CTL2, LSC_SDBLK_WIDTH)\
    CMD(a, b, c, d, e, int, LSC2_tmp1_SDBLK_HEIGHT, lsc2.sdblk_height, SDBLK_HEIGHT,, REG_CMP_EQ(c, LOG_LSC2_EN, 1), DIP_A_LSC2_CTL3, LSC_SDBLK_HEIGHT, 0, LSC2_tmp2_SDBLK_HEIGHT, DIP_A_LSC2_CTL3, LSC_SDBLK_HEIGHT)\
    CMD(a, b, c, d, e, int, LSC2_tmp1_SDBLK_lWIDTH, lsc2.sdblk_last_width, SDBLK_LWIDTH,, REG_CMP_EQ(c, LOG_LSC2_EN, 1), DIP_A_LSC2_LBLOCK, LSC_SDBLK_lWIDTH, 0, LSC2_tmp2_SDBLK_lWIDTH, DIP_A_LSC2_LBLOCK, LSC_SDBLK_lWIDTH)\
    CMD(a, b, c, d, e, int, LSC2_tmp1_SDBLK_lHEIGHT, lsc2.sdblk_last_height, SDBLK_LHEIGHT,, REG_CMP_EQ(c, LOG_LSC2_EN, 1), DIP_A_LSC2_LBLOCK, LSC_SDBLK_lHEIGHT, 0, LSC2_tmp2_SDBLK_lHEIGHT, DIP_A_LSC2_LBLOCK, LSC_SDBLK_lHEIGHT)\
    /* SL2 */\
    CMD(a, b, c, d, e, int, SL2_A_HRZ_COMP, sl2.sl2_hrz_comp, SL2_HRZ_COMP,, REG_CMP_EQ(c, LOG_SL2_EN, 1), DIP_A_SL2_RZ, SL2_HRZ_COMP, 4, SL2_B_HRZ_COMP, DIP_A_SL2_RZ, SL2_HRZ_COMP)\
    CMD(a, b, c, d, e, int, SL2_A_VRZ_COMP, sl2.sl2_vrz_comp, SL2_VRZ_COMP,, REG_CMP_EQ(c, LOG_SL2_EN, 1), DIP_A_SL2_RZ, SL2_VRZ_COMP, 4, SL2_B_VRZ_COMP, DIP_A_SL2_RZ, SL2_VRZ_COMP)\
	/* UDM */\
    CMD(a, b, c, d, e, int, UDM_tmp1_BYP, udm.bayer_bypass, BAYER_BYPASS,, REG_CMP_EQ(c, LOG_UDM_EN, 1), DIP_A_UDM_INTP_CRS, UDM_BYP, 4, UDM_tmp2_BYP, DIP_A_UDM_INTP_CRS, UDM_BYP)\
    /* VIPI */\
    CMD(a, b, c, d, e, int, VIPI_A_V_FLIP_EN, vipi.vipi_v_flip_en, VIPI_V_FLIP_EN,, REG_CMP_EQ(c, LOG_VIPI_EN, 1), DIP_A_VERTICAL_FLIP_EN, VIPI_V_FLIP_EN, 4, VIPI_B_V_FLIP_EN, DIP_A_VERTICAL_FLIP_EN, VIPI_V_FLIP_EN)\
    CMD(a, b, c, d, e, int, VIPI_A_XSIZE, vipi.vipi_xsize, VIPI_XSIZE,, REG_CMP_EQ(c, LOG_VIPI_EN, 1), DIP_A_VIPI_XSIZE, XSIZE, 0, VIPI_B_XSIZE, DIP_A_VIPI_XSIZE, XSIZE)\
    CMD(a, b, c, d, e, int, VIPI_A_YSIZE, vipi.vipi_ysize, VIPI_YSIZE,, REG_CMP_EQ(c, LOG_VIPI_EN, 1), DIP_A_VIPI_YSIZE, YSIZE, 0, VIPI_B_YSIZE, DIP_A_VIPI_YSIZE, YSIZE)\
    CMD(a, b, c, d, e, int, VIPI_A_STRIDE, vipi.vipi_stride, VIPI_STRIDE,, REG_CMP_EQ(c, LOG_VIPI_EN, 1), DIP_A_VIPI_STRIDE, STRIDE, 4, VIPI_B_STRIDE, DIP_A_VIPI_STRIDE, STRIDE)\
    /* VIP2I */\
    CMD(a, b, c, d, e, int, VIP2I_A_V_FLIP_EN, vip2i.vip2i_v_flip_en, VIP2I_V_FLIP_EN,, REG_CMP_EQ(c, LOG_VIP2I_EN, 1), DIP_A_VERTICAL_FLIP_EN, VIP2I_V_FLIP_EN, 4, VIP2I_B_V_FLIP_EN, DIP_A_VERTICAL_FLIP_EN, VIP2I_V_FLIP_EN)\
    CMD(a, b, c, d, e, int, VIP2I_A_XSIZE, vip2i.vip2i_xsize, VIP2I_XSIZE,, REG_CMP_EQ(c, LOG_VIP2I_EN, 1), DIP_A_VIP2I_XSIZE, XSIZE, 0, VIP2I_B_XSIZE, DIP_A_VIP2I_XSIZE, XSIZE)\
    CMD(a, b, c, d, e, int, VIP2I_A_YSIZE, vip2i.vip2i_ysize, VIP2I_YSIZE,, REG_CMP_EQ(c, LOG_VIP2I_EN, 1), DIP_A_VIP2I_YSIZE, YSIZE, 0, VIP2I_B_YSIZE, DIP_A_VIP2I_YSIZE, YSIZE)\
    CMD(a, b, c, d, e, int, VIP2I_A_STRIDE, vip2i.vip2i_stride, VIP2I_STRIDE,, REG_CMP_EQ(c, LOG_VIP2I_EN, 1), DIP_A_VIP2I_STRIDE, STRIDE, 4, VIP2I_B_STRIDE, DIP_A_VIP2I_STRIDE, STRIDE)\
    /* VIP3I */\
    CMD(a, b, c, d, e, int, VIP3I_A_V_FLIP_EN, vip3i.vip3i_v_flip_en, VIP3I_V_FLIP_EN,, REG_CMP_EQ(c, LOG_VIP3I_EN, 1), DIP_A_VERTICAL_FLIP_EN, VIP3I_V_FLIP_EN, 4, VIP3I_B_V_FLIP_EN, DIP_A_VERTICAL_FLIP_EN, VIP3I_V_FLIP_EN)\
    CMD(a, b, c, d, e, int, VIP3I_A_XSIZE, vip3i.vip3i_xsize, VIP3I_XSIZE,, REG_CMP_EQ(c, LOG_VIP3I_EN, 1), DIP_A_VIP3I_XSIZE, XSIZE, 0, VIP3I_B_XSIZE, DIP_A_VIP3I_XSIZE, XSIZE)\
    CMD(a, b, c, d, e, int, VIP3I_A_YSIZE, vip3i.vip3i_ysize, VIP3I_YSIZE,, REG_CMP_EQ(c, LOG_VIP3I_EN, 1), DIP_A_VIP3I_YSIZE, YSIZE, 0, VIP3I_B_YSIZE, DIP_A_VIP3I_YSIZE, YSIZE)\
    CMD(a, b, c, d, e, int, VIP3I_A_STRIDE, vip3i.vip3i_stride, VIP3I_STRIDE,, REG_CMP_EQ(c, LOG_VIP3I_EN, 1), DIP_A_VIP3I_STRIDE, STRIDE, 4, VIP3I_B_STRIDE, DIP_A_VIP3I_STRIDE, STRIDE)\
    /* PAK2O */\
    CMD(a, b, c, d, e, int, PAK2O_A_STRIDE, pak2o.pak2o_stride, PAK2O_STRIDE,, REG_CMP_EQ(c, LOG_PAK2O_EN, 1), DIP_A_PAK2O_STRIDE, STRIDE, 4, PAK2O_B_STRIDE, DIP_A_PAK2O_STRIDE, STRIDE)\
    CMD(a, b, c, d, e, int, PAK2O_A_XOFFSET, pak2o.pak2o_xoffset, PAK2O_XOFFSET,, REG_CMP_EQ(c, LOG_PAK2O_EN, 1), DIP_A_PAK2O_CROP, XOFFSET, 0, PAK2O_B_XOFFSET, DIP_A_PAK2O_CROP, XOFFSET)\
    CMD(a, b, c, d, e, int, PAK2O_A_YOFFSET, pak2o.pak2o_yoffset, PAK2O_YOFFSET,, REG_CMP_EQ(c, LOG_PAK2O_EN, 1), DIP_A_PAK2O_CROP, YOFFSET, 0, PAK2O_B_YOFFSET, DIP_A_PAK2O_CROP, YOFFSET)\
    CMD(a, b, c, d, e, int, PAK2O_A_XSIZE, pak2o.pak2o_xsize, PAK2O_XSIZE,, REG_CMP_EQ(c, LOG_PAK2O_EN, 1), DIP_A_PAK2O_XSIZE, XSIZE, 0, PAK2O_B_XSIZE, DIP_A_PAK2O_XSIZE, XSIZE)\
    CMD(a, b, c, d, e, int, PAK2O_A_YSIZE, pak2o.pak2o_ysize, PAK2O_YSIZE,, REG_CMP_EQ(c, LOG_PAK2O_EN, 1), DIP_A_PAK2O_YSIZE, YSIZE, 0, PAK2O_B_YSIZE, DIP_A_PAK2O_YSIZE, YSIZE)\
    /* MFB */\
    CMD(a, b, c, d, e, int, BLD_A_LL_DB_EN, mfb.bld_deblock_en, BLD_DEBLOCK_EN,, REG_CMP_EQ(c, LOG_MFB_EN, 1), DIP_A_MFB_CON, BLD_LL_DB_EN, 4, BLD_B_LL_DB_EN, DIP_A_MFB_CON, BLD_LL_DB_EN)\
    CMD(a, b, c, d, e, int, BLD_A_LL_BRZ_EN, mfb.bld_brz_en, BLD_LL_BRZ_EN,, REG_CMP_EQ(c, LOG_MFB_EN, 1), DIP_A_MFB_CON, BLD_LL_BRZ_EN, 4, BLD_B_LL_BRZ_EN, DIP_A_MFB_CON, BLD_LL_BRZ_EN)\
    CMD(a, b, c, d, e, int, BLD_A_MBD_WT_EN, mfb.bld_mbd_wt_en, BLD_MBD_WT_EN,, REG_CMP_EQ(c, LOG_MFB_EN, 1), DIP_A_MFB_CON, BLD_MBD_WT_EN, 4, BLD_B_MBD_WT_EN, DIP_A_MFB_CON, BLD_MBD_WT_EN)\
   /* G2C */\
    CMD(a, b, c, d, e, int, G2C_A_SHADE_EN, g2c.g2c_shade_en, G2C_SHADE_EN,, REG_CMP_EQ(c, LOG_G2C_EN, 1), DIP_A_G2C_SHADE_CON_1, G2C_SHADE_EN, 4, G2C_B_SHADE_EN, DIP_A_G2C_SHADE_CON_1, G2C_SHADE_EN)\
    CMD(a, b, c, d, e, int, G2C_A_SHADE_XMID, g2c.g2c_shade_xmid, G2C_SHADE_XMID,, REG_CMP_EQ(c, LOG_G2C_EN, 1), DIP_A_G2C_SHADE_TAR, G2C_SHADE_XMID, 4, G2C_B_SHADE_XMID, DIP_A_G2C_SHADE_TAR, G2C_SHADE_XMID)\
    CMD(a, b, c, d, e, int, G2C_A_SHADE_YMID, g2c.g2c_shade_ymid, G2C_SHADE_YMID,, REG_CMP_EQ(c, LOG_G2C_EN, 1), DIP_A_G2C_SHADE_TAR, G2C_SHADE_YMID, 4, G2C_B_SHADE_YMID, DIP_A_G2C_SHADE_TAR, G2C_SHADE_YMID)\
    CMD(a, b, c, d, e, int, G2C_A_SHADE_VAR, g2c.g2c_shade_var, G2C_SHADE_VAR,, REG_CMP_EQ(c, LOG_G2C_EN, 1), DIP_A_G2C_SHADE_CON_1, G2C_SHADE_VAR, 4, G2C_B_SHADE_VAR, DIP_A_G2C_SHADE_CON_1, G2C_SHADE_VAR)\
    /* SL2B */\
    CMD(a, b, c, d, e, int, SL2B_A_HRZ_COMP, sl2b.sl2b_hrz_comp, SL2B_HRZ_COMP,, REG_CMP_EQ(c, LOG_SL2B_EN, 1), DIP_A_SL2B_RZ, SL2_HRZ_COMP, 4, SL2B_B_HRZ_COMP, DIP_A_SL2B_RZ, SL2_HRZ_COMP)\
    CMD(a, b, c, d, e, int, SL2B_A_VRZ_COMP, sl2b.sl2b_vrz_comp, SL2B_VRZ_COMP,, REG_CMP_EQ(c, LOG_SL2B_EN, 1), DIP_A_SL2B_RZ, SL2_VRZ_COMP, 4, SL2B_B_VRZ_COMP, DIP_A_SL2B_RZ, SL2_VRZ_COMP)\
	/* NBC */\
	CMD(a, b, c, d, e, int, NBC_A_ANR_ENY, nbc.anr_eny, ANR_ENY,, REG_CMP_EQ(c, LOG_NBC_EN, 1), DIP_A_NBC_ANR_CON1, NBC_ANR_ENY, 4, NBC_B_ANR_ENY, DIP_A_NBC_ANR_CON1, NBC_ANR_ENY)\
    CMD(a, b, c, d, e, int, NBC_A_ANR_ENC, nbc.anr_enc, ANR_ENC,, REG_CMP_EQ(c, LOG_NBC_EN, 1), DIP_A_NBC_ANR_CON1, NBC_ANR_ENC, 4, NBC_B_ANR_ENC, DIP_A_NBC_ANR_CON1, NBC_ANR_ENC)\
    CMD(a, b, c, d, e, int, NBC_A_ANR_LTM_LINK, nbc.anr_ltm_link, ANR_LTM_LINK,, REG_CMP_EQ(c, LOG_NBC_EN, 1), DIP_A_NBC_ANR_CON1, NBC_ANR_LTM_LINK, 4, NBC_B_ANR_LTM_LINK, DIP_A_NBC_ANR_CON1, NBC_ANR_LTM_LINK)\
	/* NBC2 */\
	CMD(a, b, c, d, e, int, NBC2_A_ANR2_ENY, nbc2.anr2_eny, ANR2_ENY,, REG_CMP_EQ(c, LOG_NBC2_EN, 1), DIP_A_NBC2_ANR2_CON1, NBC2_ANR2_ENY, 4, NBC2_B_ANR2_ENY, DIP_A_NBC2_ANR2_CON1, NBC2_ANR2_ENY)\
    CMD(a, b, c, d, e, int, NBC2_A_ANR2_ENC, nbc2.anr2_enc, ANR2_ENC,, REG_CMP_EQ(c, LOG_NBC2_EN, 1), DIP_A_NBC2_ANR2_CON1, NBC2_ANR2_ENC, 4, NBC2_B_ANR2_ENC, DIP_A_NBC2_ANR2_CON1, NBC2_ANR2_ENC)\
    CMD(a, b, c, d, e, int, NBC2_A_ANR2_SCALE_MODE, nbc2.anr2_scale_mode, ANR2_SCALE_MODE,, REG_CMP_EQ(c, LOG_NBC2_EN, 1), DIP_A_NBC2_ANR2_CON1, NBC2_ANR2_SCALE_MODE, 4, NBC2_B_ANR2_SCALE_MODE, DIP_A_NBC2_ANR2_CON1, NBC2_ANR2_SCALE_MODE)\
    CMD(a, b, c, d, e, int, NBC2_A_ANR2_MODE, nbc2.anr2_mode, ANR2_MODE,, REG_CMP_EQ(c, LOG_NBC2_EN, 1), DIP_A_NBC2_ANR2_CON1, NBC2_ANR2_MODE, 4, NBC2_B_ANR2_MODE, DIP_A_NBC2_ANR2_CON1, NBC2_ANR2_MODE)\
    CMD(a, b, c, d, e, int, NBC2_A_BOK_MODE, nbc2.anr2_bok_mode, ANR2_BOK_MODE,, REG_CMP_EQ(c, LOG_NBC2_EN, 1), DIP_A_NBC2_BOK_CON, NBC2_BOK_MODE, 4, NBC2_B_BOK_MODE, DIP_A_NBC2_BOK_CON, NBC2_BOK_MODE)\
    CMD(a, b, c, d, e, int, NBC2_A_BOK_PF_EN, nbc2.anr2_bok_pf_en, ANR2_BOK_PF_EN,, REG_CMP_EQ(c, LOG_NBC2_EN, 1), DIP_A_NBC2_BOK_CON, NBC2_BOK_PF_EN, 4, NBC2_B_BOK_PF_EN, DIP_A_NBC2_BOK_CON, NBC2_BOK_PF_EN)\
	CMD(a, b, c, d, e, int, NBC2_A_ABF_ENC, nbc2.abf_en, ABF_EN,, REG_CMP_EQ(c, LOG_NBC2_EN, 1), DIP_A_NBC2_ABF_CON1, NBC2_ABF_EN, 4, NBC2_B_ABF_ENC, DIP_A_NBC2_ABF_CON1, NBC2_ABF_EN)\
    /* SL2C */\
    CMD(a, b, c, d, e, int, SL2C_A_HRZ_COMP, sl2c.sl2c_hrz_comp, SL2C_HRZ_COMP,, REG_CMP_EQ(c, LOG_SL2C_EN, 1), DIP_A_SL2C_RZ, SL2_HRZ_COMP, 4, SL2C_B_HRZ_COMP, DIP_A_SL2C_RZ, SL2_HRZ_COMP)\
    CMD(a, b, c, d, e, int, SL2C_A_VRZ_COMP, sl2c.sl2c_vrz_comp, SL2C_VRZ_COMP,, REG_CMP_EQ(c, LOG_SL2C_EN, 1), DIP_A_SL2C_RZ, SL2_VRZ_COMP, 4, SL2C_B_VRZ_COMP, DIP_A_SL2C_RZ, SL2_VRZ_COMP)\
	/* SL2D */\
    CMD(a, b, c, d, e, int, SL2D_A_HRZ_COMP, sl2d.sl2d_hrz_comp, SL2D_HRZ_COMP,, REG_CMP_EQ(c, LOG_SL2D_EN, 1), DIP_A_SL2D_RZ, SL2_HRZ_COMP, 4, SL2D_B_HRZ_COMP, DIP_A_SL2D_RZ, SL2_HRZ_COMP)\
    CMD(a, b, c, d, e, int, SL2D_A_VRZ_COMP, sl2d.sl2d_vrz_comp, SL2D_VRZ_COMP,, REG_CMP_EQ(c, LOG_SL2D_EN, 1), DIP_A_SL2D_RZ, SL2_VRZ_COMP, 4, SL2D_B_VRZ_COMP, DIP_A_SL2D_RZ, SL2_VRZ_COMP)\
	/* SL2E */\
    CMD(a, b, c, d, e, int, SL2E_A_HRZ_COMP, sl2e.sl2e_hrz_comp, SL2E_HRZ_COMP,, REG_CMP_EQ(c, LOG_SL2E_EN, 1), DIP_A_SL2E_RZ, SL2_HRZ_COMP, 4, SL2E_B_HRZ_COMP, DIP_A_SL2E_RZ, SL2_HRZ_COMP)\
    CMD(a, b, c, d, e, int, SL2E_A_VRZ_COMP, sl2e.sl2e_vrz_comp, SL2E_VRZ_COMP,, REG_CMP_EQ(c, LOG_SL2E_EN, 1), DIP_A_SL2E_RZ, SL2_VRZ_COMP, 4, SL2E_B_VRZ_COMP, DIP_A_SL2E_RZ, SL2_VRZ_COMP)\
	/* SL2G */\
    CMD(a, b, c, d, e, int, SL2G_A_HRZ_COMP, sl2g.sl2g_hrz_comp, SL2G_HRZ_COMP,, REG_CMP_EQ(c, LOG_SL2G_EN, 1), DIP_A_SL2G_RZ, SL2_HRZ_COMP, 4, SL2G_B_HRZ_COMP, DIP_A_SL2G_RZ, SL2_HRZ_COMP)\
    CMD(a, b, c, d, e, int, SL2G_A_VRZ_COMP, sl2g.sl2g_vrz_comp, SL2G_VRZ_COMP,, REG_CMP_EQ(c, LOG_SL2G_EN, 1), DIP_A_SL2G_RZ, SL2_VRZ_COMP, 4, SL2G_B_VRZ_COMP, DIP_A_SL2G_RZ, SL2_VRZ_COMP)\
	/* SL2H */\
    CMD(a, b, c, d, e, int, SL2H_A_HRZ_COMP, sl2h.sl2h_hrz_comp, SL2H_HRZ_COMP,, REG_CMP_EQ(c, LOG_SL2H_EN, 1), DIP_A_SL2H_RZ, SL2_HRZ_COMP, 4, SL2H_B_HRZ_COMP, DIP_A_SL2H_RZ, SL2_HRZ_COMP)\
    CMD(a, b, c, d, e, int, SL2H_A_VRZ_COMP, sl2h.sl2h_vrz_comp, SL2H_VRZ_COMP,, REG_CMP_EQ(c, LOG_SL2H_EN, 1), DIP_A_SL2H_RZ, SL2_VRZ_COMP, 4, SL2H_B_VRZ_COMP, DIP_A_SL2H_RZ, SL2_VRZ_COMP)\
	/* SEEE */\
    CMD(a, b, c, d, e, int, SEEE_A_OUT_EDGE_SEL, seee.se_edge, SE_EDGE,, REG_CMP_EQ(c, LOG_SEEE_EN, 1), DIP_A_SEEE_TOP_CTRL, SEEE_OUT_EDGE_SEL, 4, SEEE_B_OUT_EDGE_SEL, DIP_A_SEEE_TOP_CTRL, SEEE_OUT_EDGE_SEL)\
    /* LCEI */\
    CMD(a, b, c, d, e, int, LCEI_A_V_FLIP_EN, lcei.lcei_v_flip_en, LCEI_V_FLIP_EN,, REG_CMP_EQ(c, LOG_LCEI_EN, 1), DIP_A_VERTICAL_FLIP_EN, LCEI_V_FLIP_EN, 4, LCEI_B_V_FLIP_EN, DIP_A_VERTICAL_FLIP_EN, LCEI_V_FLIP_EN)\
    CMD(a, b, c, d, e, int, LCEI_A_XSIZE, lcei.lcei_xsize, LCEI_XSIZE,, REG_CMP_EQ(c, LOG_LCEI_EN, 1), DIP_A_LCEI_XSIZE, XSIZE, 0, LCEI_B_XSIZE, DIP_A_LCEI_XSIZE, XSIZE)\
    CMD(a, b, c, d, e, int, LCEI_A_YSIZE, lcei.lcei_ysize, LCEI_YSIZE,, REG_CMP_EQ(c, LOG_LCEI_EN, 1), DIP_A_LCEI_YSIZE, YSIZE, 0, LCEI_B_YSIZE, DIP_A_LCEI_YSIZE, YSIZE)\
    CMD(a, b, c, d, e, int, LCEI_A_STRIDE, lcei.lcei_stride, LCEI_STRIDE,, REG_CMP_EQ(c, LOG_LCEI_EN, 1), DIP_A_LCEI_STRIDE, STRIDE, 4, LCEI_B_STRIDE, DIP_A_LCEI_STRIDE, STRIDE)\
    /* LCE */\
    CMD(a, b, c, d, e, int, LCE_A_LC_TONE, lce.lce_lc_tone, LCE_LC_TONE,, REG_CMP_EQ(c, LOG_LCE_EN, 1), DIP_A_LCE25_TM_PARA0, LCE_LC_TONE, 0, LCE_B_LC_TONE, DIP_A_LCE25_TM_PARA0, LCE_LC_TONE)\
    CMD(a, b, c, d, e, int, LCE_A_SLM_WD, lce.lce_slm_width, SLM_WIDTH,, REG_CMP_EQ(c, LOG_LCE_EN, 1), DIP_A_LCE25_SLM_SIZE, LCE_SLM_WD, 0, LCE_B_SLM_WD, DIP_A_LCE25_SLM_SIZE, LCE_SLM_WD)\
    CMD(a, b, c, d, e, int, LCE_A_SLM_HT, lce.lce_slm_height, SLM_HEIGHT,, REG_CMP_EQ(c, LOG_LCE_EN, 1), DIP_A_LCE25_SLM_SIZE, LCE_SLM_HT, 0, LCE_B_SLM_HT, DIP_A_LCE25_SLM_SIZE, LCE_SLM_HT)\
    CMD(a, b, c, d, e, int, LCE_A_BCMK_X, lce.lce_bc_mag_kubnx, BC_MAG_KUBNX,, REG_CMP_EQ(c, LOG_LCE_EN, 1), DIP_A_LCE25_ZR, LCE_BCMK_X, 4, LCE_B_BCMK_X, DIP_A_LCE25_ZR, LCE_BCMK_X)\
    CMD(a, b, c, d, e, int, LCE_A_BCMK_Y, lce.lce_bc_mag_kubny, BC_MAG_KUBNY,, REG_CMP_EQ(c, LOG_LCE_EN, 1), DIP_A_LCE25_ZR, LCE_BCMK_Y, 4, LCE_B_BCMK_Y, DIP_A_LCE25_ZR, LCE_BCMK_Y)\
    /* CDRZ */\
    CMD(a, b, c, d, e, int, CRZ_A_crop_size_x_d, cdrz.cdrz_input_crop_width, CDRZ_Input_Image_W,, REG_CMP_EQ(c, LOG_CDRZ_EN, 1), DIP_A_CRZ_IN_IMG, CRZ_IN_WD, 0, CRZ_B_crop_size_x_d, DIP_A_CRZ_IN_IMG, CRZ_IN_WD)\
    CMD(a, b, c, d, e, int, CRZ_A_crop_size_y_d, cdrz.cdrz_input_crop_height, CDRZ_Input_Image_H,, REG_CMP_EQ(c, LOG_CDRZ_EN, 1), DIP_A_CRZ_IN_IMG, CRZ_IN_HT, 0, CRZ_B_crop_size_y_d, DIP_A_CRZ_IN_IMG, CRZ_IN_HT)\
    CMD(a, b, c, d, e, int, CDRZ_A_Output_Image_W, cdrz.cdrz_output_width, CDRZ_Output_Image_W,, REG_CMP_EQ(c, LOG_CDRZ_EN, 1), DIP_A_CRZ_OUT_IMG, CRZ_OUT_WD, 0, CDRZ_B_Output_Image_W, DIP_A_CRZ_OUT_IMG, CRZ_OUT_WD)\
    CMD(a, b, c, d, e, int, CDRZ_A_Output_Image_H, cdrz.cdrz_output_height, CDRZ_Output_Image_H,, REG_CMP_EQ(c, LOG_CDRZ_EN, 1), DIP_A_CRZ_OUT_IMG, CRZ_OUT_HT, 0, CDRZ_B_Output_Image_H, DIP_A_CRZ_OUT_IMG, CRZ_OUT_HT)\
    CMD(a, b, c, d, e, int, CDRZ_A_Luma_Horizontal_Integer_Offset, cdrz.cdrz_luma_horizontal_integer_offset, CDRZ_Luma_Horizontal_Integer_Offset,, REG_CMP_EQ(c, LOG_CDRZ_EN, 1), DIP_A_CRZ_LUMA_HORI_INT_OFST, CRZ_LUMA_HORI_INT_OFST, 0, CDRZ_B_Luma_Horizontal_Integer_Offset, DIP_A_CRZ_LUMA_HORI_INT_OFST, CRZ_LUMA_HORI_INT_OFST)\
    CMD(a, b, c, d, e, int, CDRZ_A_Luma_Horizontal_Subpixel_Offset, cdrz.cdrz_luma_horizontal_subpixel_offset, CDRZ_Luma_Horizontal_Subpixel_Offset,, REG_CMP_EQ(c, LOG_CDRZ_EN, 1), DIP_A_CRZ_LUMA_HORI_SUB_OFST, CRZ_LUMA_HORI_SUB_OFST, 0, CDRZ_B_Luma_Horizontal_Subpixel_Offset, DIP_A_CRZ_LUMA_HORI_SUB_OFST, CRZ_LUMA_HORI_SUB_OFST)\
    CMD(a, b, c, d, e, int, CDRZ_A_Luma_Vertical_Integer_Offset, cdrz.cdrz_luma_vertical_integer_offset, CDRZ_Luma_Vertical_Integer_Offset,, REG_CMP_EQ(c, LOG_CDRZ_EN, 1), DIP_A_CRZ_LUMA_VERT_INT_OFST, CRZ_LUMA_VERT_INT_OFST, 0, CDRZ_B_Luma_Vertical_Integer_Offset, DIP_A_CRZ_LUMA_VERT_INT_OFST, CRZ_LUMA_VERT_INT_OFST)\
    CMD(a, b, c, d, e, int, CDRZ_A_Luma_Vertical_Subpixel_Offset, cdrz.cdrz_luma_vertical_subpixel_offset, CDRZ_Luma_Vertical_Subpixel_Offset,, REG_CMP_EQ(c, LOG_CDRZ_EN, 1), DIP_A_CRZ_LUMA_VERT_SUB_OFST, CRZ_LUMA_VERT_SUB_OFST, 0, CDRZ_B_Luma_Vertical_Subpixel_Offset, DIP_A_CRZ_LUMA_VERT_SUB_OFST, CRZ_LUMA_VERT_SUB_OFST)\
    CMD(a, b, c, d, e, int, CDRZ_A_Horizontal_Luma_Algorithm, cdrz.cdrz_horizontal_luma_algorithm, CDRZ_Horizontal_Luma_Algorithm,, REG_CMP_EQ(c, LOG_CDRZ_EN, 1), DIP_A_CRZ_CONTROL, CRZ_HORI_ALGO, 4, CDRZ_B_Horizontal_Luma_Algorithm, DIP_A_CRZ_CONTROL, CRZ_HORI_ALGO)\
    CMD(a, b, c, d, e, int, CDRZ_A_Vertical_Luma_Algorithm, cdrz.cdrz_vertical_luma_algorithm, CDRZ_Vertical_Luma_Algorithm,, REG_CMP_EQ(c, LOG_CDRZ_EN, 1), DIP_A_CRZ_CONTROL, CRZ_VERT_ALGO, 4, CDRZ_B_Vertical_Luma_Algorithm, DIP_A_CRZ_CONTROL, CRZ_VERT_ALGO)\
    CMD(a, b, c, d, e, int, CDRZ_A_Horizontal_Coeff_Step, cdrz.cdrz_horizontal_coeff_step, CDRZ_Horizontal_Coeff_Step,, REG_CMP_EQ(c, LOG_CDRZ_EN, 1), DIP_A_CRZ_HORI_STEP, CRZ_HORI_STEP, 4, CDRZ_B_Horizontal_Coeff_Step, DIP_A_CRZ_HORI_STEP, CRZ_HORI_STEP)\
    CMD(a, b, c, d, e, int, CDRZ_A_Vertical_Coeff_Step, cdrz.cdrz_vertical_coeff_step, CDRZ_Vertical_Coeff_Step,, REG_CMP_EQ(c, LOG_CDRZ_EN, 1), DIP_A_CRZ_VERT_STEP, CRZ_VERT_STEP, 4, CDRZ_B_Vertical_Coeff_Step, DIP_A_CRZ_VERT_STEP, CRZ_VERT_STEP)\
    /* IMG2O */\
    CMD(a, b, c, d, e, int, IMG2O_A_STRIDE, img2o.img2o_stride, IMG2O_STRIDE,, REG_CMP_EQ(c, LOG_IMG2O_EN, 1), DIP_A_IMG2O_STRIDE, STRIDE, 4, IMG2O_B_STRIDE, DIP_A_IMG2O_STRIDE, STRIDE)\
    CMD(a, b, c, d, e, int, IMG2O_A_XOFFSET, img2o.img2o_xoffset, IMG2O_XOFFSET,, REG_CMP_EQ(c, LOG_IMG2O_EN, 1), DIP_A_IMG2O_CROP, XOFFSET, 0, IMG2O_B_XOFFSET, DIP_A_IMG2O_CROP, XOFFSET)\
    CMD(a, b, c, d, e, int, IMG2O_A_YOFFSET, img2o.img2o_yoffset, IMG2O_YOFFSET,, REG_CMP_EQ(c, LOG_IMG2O_EN, 1), DIP_A_IMG2O_CROP, YOFFSET, 0, IMG2O_B_YOFFSET, DIP_A_IMG2O_CROP, YOFFSET)\
    CMD(a, b, c, d, e, int, IMG2O_A_XSIZE, img2o.img2o_xsize, IMG2O_XSIZE,, REG_CMP_EQ(c, LOG_IMG2O_EN, 1), DIP_A_IMG2O_XSIZE, XSIZE, 0, IMG2O_B_XSIZE, DIP_A_IMG2O_XSIZE, XSIZE)\
    CMD(a, b, c, d, e, int, IMG2O_A_YSIZE, img2o.img2o_ysize, IMG2O_YSIZE,, REG_CMP_EQ(c, LOG_IMG2O_EN, 1), DIP_A_IMG2O_YSIZE, YSIZE, 0, IMG2O_B_YSIZE, DIP_A_IMG2O_YSIZE, YSIZE)\
    /* IMG2BO */\
    CMD(a, b, c, d, e, int, IMG2BO_A_STRIDE, img2bo.img2bo_stride, IMG2BO_STRIDE,, REG_CMP_EQ(c, LOG_IMG2BO_EN, 1), DIP_A_IMG2BO_STRIDE, STRIDE, 4, IMG2BO_B_STRIDE, DIP_A_IMG2BO_STRIDE, STRIDE)\
    CMD(a, b, c, d, e, int, IMG2BO_A_XOFFSET, img2bo.img2bo_xoffset, IMG2BO_XOFFSET,, REG_CMP_EQ(c, LOG_IMG2BO_EN, 1), DIP_A_IMG2BO_CROP, XOFFSET, 0, IMG2BO_B_XOFFSET, DIP_A_IMG2BO_CROP, XOFFSET)\
    CMD(a, b, c, d, e, int, IMG2BO_A_YOFFSET, img2bo.img2bo_yoffset, IMG2BO_YOFFSET,, REG_CMP_EQ(c, LOG_IMG2BO_EN, 1), DIP_A_IMG2BO_CROP, YOFFSET, 0, IMG2BO_B_YOFFSET, DIP_A_IMG2BO_CROP, YOFFSET)\
    CMD(a, b, c, d, e, int, IMG2BO_A_XSIZE, img2bo.img2bo_xsize, IMG2BO_XSIZE,, REG_CMP_EQ(c, LOG_IMG2BO_EN, 1), DIP_A_IMG2BO_XSIZE, XSIZE, 0, IMG2BO_B_XSIZE, DIP_A_IMG2BO_XSIZE, XSIZE)\
    CMD(a, b, c, d, e, int, IMG2BO_A_YSIZE, img2bo.img2bo_ysize, IMG2BO_YSIZE,, REG_CMP_EQ(c, LOG_IMG2BO_EN, 1), DIP_A_IMG2BO_YSIZE, YSIZE, 0, IMG2BO_B_YSIZE, DIP_A_IMG2BO_YSIZE, YSIZE)\
	/* SRZ1 */\
    CMD(a, b, c, d, e, int, SRZ1_A_IN_CROP_WD, srz1.srz_input_crop_width, SRZ1_Input_Image_W,, REG_CMP_EQ(c, LOG_SRZ1_EN, 1), DIP_A_SRZ1_IN_IMG, SRZ_IN_WD, 0, SRZ1_B_IN_CROP_WD, DIP_A_SRZ1_IN_IMG, SRZ_IN_WD)\
    CMD(a, b, c, d, e, int, SRZ1_A_IN_CROP_HT, srz1.srz_input_crop_height, SRZ1_Input_Image_H,, REG_CMP_EQ(c, LOG_SRZ1_EN, 1), DIP_A_SRZ1_IN_IMG, SRZ_IN_HT, 0, SRZ1_B_IN_CROP_HT, DIP_A_SRZ1_IN_IMG, SRZ_IN_HT)\
    CMD(a, b, c, d, e, int, SRZ1_A_OUT_WD, srz1.srz_output_width, SRZ1_Output_Image_W,, REG_CMP_EQ(c, LOG_SRZ1_EN, 1), DIP_A_SRZ1_OUT_IMG, SRZ_OUT_WD, 0, SRZ1_B_OUT_WD, DIP_A_SRZ1_OUT_IMG, SRZ_OUT_WD)\
    CMD(a, b, c, d, e, int, SRZ1_A_OUT_HT, srz1.srz_output_height, SRZ1_Output_Image_H,, REG_CMP_EQ(c, LOG_SRZ1_EN, 1), DIP_A_SRZ1_OUT_IMG, SRZ_OUT_HT, 0, SRZ1_B_OUT_HT, DIP_A_SRZ1_OUT_IMG, SRZ_OUT_HT)\
    CMD(a, b, c, d, e, int, SRZ1_A_HORI_INT_OFST, srz1.srz_luma_horizontal_integer_offset, SRZ1_Luma_Horizontal_Integer_Offset,, REG_CMP_EQ(c, LOG_SRZ1_EN, 1), DIP_A_SRZ1_HORI_INT_OFST, SRZ_HORI_INT_OFST, 0, SRZ1_B_HORI_INT_OFST, DIP_A_SRZ1_HORI_INT_OFST, SRZ_HORI_INT_OFST)\
    CMD(a, b, c, d, e, int, SRZ1_A_HORI_SUB_OFST, srz1.srz_luma_horizontal_subpixel_offset, SRZ1_Luma_Horizontal_Subpixel_Offset,, REG_CMP_EQ(c, LOG_SRZ1_EN, 1), DIP_A_SRZ1_HORI_SUB_OFST, SRZ_HORI_SUB_OFST, 0, SRZ1_B_HORI_SUB_OFST, DIP_A_SRZ1_HORI_SUB_OFST, SRZ_HORI_SUB_OFST)\
    CMD(a, b, c, d, e, int, SRZ1_A_VERT_INT_OFST, srz1.srz_luma_vertical_integer_offset, SRZ1_Luma_Vertical_Integer_Offset,, REG_CMP_EQ(c, LOG_SRZ1_EN, 1), DIP_A_SRZ1_VERT_INT_OFST, SRZ_VERT_INT_OFST, 0, SRZ1_B_VERT_INT_OFST, DIP_A_SRZ1_VERT_INT_OFST, SRZ_VERT_INT_OFST)\
    CMD(a, b, c, d, e, int, SRZ1_A_VERT_SUB_OFST, srz1.srz_luma_vertical_subpixel_offset, SRZ1_Luma_Vertical_Subpixel_Offset,, REG_CMP_EQ(c, LOG_SRZ1_EN, 1), DIP_A_SRZ1_VERT_SUB_OFST, SRZ_VERT_SUB_OFST, 0, SRZ1_B_VERT_SUB_OFST, DIP_A_SRZ1_VERT_SUB_OFST, SRZ_VERT_SUB_OFST)\
    CMD(a, b, c, d, e, int, SRZ1_A_HORI_STEP, srz1.srz_horizontal_coeff_step, SRZ1_Horizontal_Coeff_Step,, REG_CMP_EQ(c, LOG_SRZ1_EN, 1), DIP_A_SRZ1_HORI_STEP, SRZ_HORI_STEP, 4, SRZ1_B_HORI_STEP, DIP_A_SRZ1_HORI_STEP, SRZ_HORI_STEP)\
    CMD(a, b, c, d, e, int, SRZ1_A_VERT_STEP, srz1.srz_vertical_coeff_step, SRZ1_Vertical_Coeff_Step,, REG_CMP_EQ(c, LOG_SRZ1_EN, 1), DIP_A_SRZ1_VERT_STEP, SRZ_VERT_STEP, 4, SRZ1_B_VERT_STEP, DIP_A_SRZ1_VERT_STEP, SRZ_VERT_STEP)\
    /* SRZ2 */\
    CMD(a, b, c, d, e, int, SRZ2_A_IN_CROP_WD, srz2.srz_input_crop_width, SRZ2_Input_Image_W,, REG_CMP_EQ(c, LOG_SRZ2_EN, 1), DIP_A_SRZ2_IN_IMG, SRZ_IN_WD, 0, SRZ2_B_IN_CROP_WD, DIP_A_SRZ2_IN_IMG, SRZ_IN_WD)\
    CMD(a, b, c, d, e, int, SRZ2_A_IN_CROP_HT, srz2.srz_input_crop_height, SRZ2_Input_Image_H,, REG_CMP_EQ(c, LOG_SRZ2_EN, 1), DIP_A_SRZ2_IN_IMG, SRZ_IN_HT, 0, SRZ2_B_IN_CROP_HT, DIP_A_SRZ2_IN_IMG, SRZ_IN_HT)\
    CMD(a, b, c, d, e, int, SRZ2_A_OUT_WD, srz2.srz_output_width, SRZ2_Output_Image_W,, REG_CMP_EQ(c, LOG_SRZ2_EN, 1), DIP_A_SRZ2_OUT_IMG, SRZ_OUT_WD, 0, SRZ2_B_OUT_WD, DIP_A_SRZ2_OUT_IMG, SRZ_OUT_WD)\
    CMD(a, b, c, d, e, int, SRZ2_A_OUT_HT, srz2.srz_output_height, SRZ2_Output_Image_H,, REG_CMP_EQ(c, LOG_SRZ2_EN, 1), DIP_A_SRZ2_OUT_IMG, SRZ_OUT_HT, 0, SRZ2_B_OUT_HT, DIP_A_SRZ2_OUT_IMG, SRZ_OUT_HT)\
    CMD(a, b, c, d, e, int, SRZ2_A_HORI_INT_OFST, srz2.srz_luma_horizontal_integer_offset, SRZ2_Luma_Horizontal_Integer_Offset,, REG_CMP_EQ(c, LOG_SRZ2_EN, 1), DIP_A_SRZ2_HORI_INT_OFST, SRZ_HORI_INT_OFST, 0, SRZ2_B_HORI_INT_OFST, DIP_A_SRZ2_HORI_INT_OFST, SRZ_HORI_INT_OFST)\
    CMD(a, b, c, d, e, int, SRZ2_A_HORI_SUB_OFST, srz2.srz_luma_horizontal_subpixel_offset, SRZ2_Luma_Horizontal_Subpixel_Offset,, REG_CMP_EQ(c, LOG_SRZ2_EN, 1), DIP_A_SRZ2_HORI_SUB_OFST, SRZ_HORI_SUB_OFST, 0, SRZ2_B_HORI_SUB_OFST, DIP_A_SRZ2_HORI_SUB_OFST, SRZ_HORI_SUB_OFST)\
    CMD(a, b, c, d, e, int, SRZ2_A_VERT_INT_OFST, srz2.srz_luma_vertical_integer_offset, SRZ2_Luma_Vertical_Integer_Offset,, REG_CMP_EQ(c, LOG_SRZ2_EN, 1), DIP_A_SRZ2_VERT_INT_OFST, SRZ_VERT_INT_OFST, 0, SRZ2_B_VERT_INT_OFST, DIP_A_SRZ2_VERT_INT_OFST, SRZ_VERT_INT_OFST)\
    CMD(a, b, c, d, e, int, SRZ2_A_VERT_SUB_OFST, srz2.srz_luma_vertical_subpixel_offset, SRZ2_Luma_Vertical_Subpixel_Offset,, REG_CMP_EQ(c, LOG_SRZ2_EN, 1), DIP_A_SRZ2_VERT_SUB_OFST, SRZ_VERT_SUB_OFST, 0, SRZ2_B_VERT_SUB_OFST, DIP_A_SRZ2_VERT_SUB_OFST, SRZ_VERT_SUB_OFST)\
    CMD(a, b, c, d, e, int, SRZ2_A_HORI_STEP, srz2.srz_horizontal_coeff_step, SRZ2_Horizontal_Coeff_Step,, REG_CMP_EQ(c, LOG_SRZ2_EN, 1), DIP_A_SRZ2_HORI_STEP, SRZ_HORI_STEP, 4, SRZ2_B_HORI_STEP, DIP_A_SRZ2_HORI_STEP, SRZ_HORI_STEP)\
    CMD(a, b, c, d, e, int, SRZ2_A_VERT_STEP, srz2.srz_vertical_coeff_step, SRZ2_Vertical_Coeff_Step,, REG_CMP_EQ(c, LOG_SRZ2_EN, 1), DIP_A_SRZ2_VERT_STEP, SRZ_VERT_STEP, 4, SRZ2_B_VERT_STEP, DIP_A_SRZ2_VERT_STEP, SRZ_VERT_STEP)\
    /* FE */\
    CMD(a, b, c, d, e, int, FE_A_MODE, fe.fe_mode, FE_MODE,, REG_CMP_EQ(c, LOG_FE_EN, 1), DIP_A_FE_CTRL1, FE_MODE, 4, FE_B_MODE, DIP_A_FE_CTRL1, FE_MODE)\
    /* FEO */\
    CMD(a, b, c, d, e, int, FEO_A_STRIDE, feo.feo_stride, FEO_STRIDE,, REG_CMP_EQ(c, LOG_FEO_EN, 1), DIP_A_FEO_STRIDE, STRIDE, 4, FEO_B_STRIDE, DIP_A_FEO_STRIDE, STRIDE)\
    /* NR3D */\
    CMD(a, b, c, d, e, int, NR3D_A_ON_EN, nr3d.nr3d_on_en, NR3D_ON_EN,, REG_CMP_EQ(c, LOG_NR3D_EN, 1), DIP_A_NR3D_ON_CON, NR3D_ON_EN, 4, NR3D_B_ON_EN, DIP_A_NR3D_ON_CON, NR3D_ON_EN)\
    CMD(a, b, c, d, e, int, NR3D_A_ON_OFST_X, nr3d.nr3d_on_xoffset, NR3D_ON_OFST_X,, REG_CMP_EQ(c, LOG_NR3D_EN, 1), DIP_A_NR3D_ON_OFF, NR3D_ON_OFST_X, 0, NR3D_B_ON_OFST_X, DIP_A_NR3D_ON_OFF, NR3D_ON_OFST_X)\
    CMD(a, b, c, d, e, int, NR3D_A_ON_OFST_Y, nr3d.nr3d_on_yoffset, NR3D_ON_OFST_Y,, REG_CMP_EQ(c, LOG_NR3D_EN, 1), DIP_A_NR3D_ON_OFF, NR3D_ON_OFST_Y, 0, NR3D_B_ON_OFST_Y, DIP_A_NR3D_ON_OFF, NR3D_ON_OFST_Y)\
    CMD(a, b, c, d, e, int, NR3D_A_ON_WD, nr3d.nr3d_on_width, NR3D_ON_WD,, REG_CMP_EQ(c, LOG_NR3D_EN, 1), DIP_A_NR3D_ON_SIZ, NR3D_ON_WD, 0, NR3D_B_ON_WD, DIP_A_NR3D_ON_SIZ, NR3D_ON_WD)\
    CMD(a, b, c, d, e, int, NR3D_A_ON_HT, nr3d.nr3d_on_height, NR3D_ON_HT,, REG_CMP_EQ(c, LOG_NR3D_EN, 1), DIP_A_NR3D_ON_SIZ, NR3D_ON_HT, 0, NR3D_B_ON_HT, DIP_A_NR3D_ON_SIZ, NR3D_ON_HT)\
	/* CRSP */\
    CMD(a, b, c, d, e, int, CRSP_A_cstep_y, crsp.crsp_ystep, CRSP_STEP_Y,, REG_CMP_EQ(c, LOG_CRSP_EN, 1), DIP_A_CRSP_STEP_OFST, CRSP_STEP_Y, 0, CRSP_B_cstep_y, DIP_A_CRSP_STEP_OFST, CRSP_STEP_Y)\
    CMD(a, b, c, d, e, int, CRSP_A_offset_x, crsp.crsp_xoffset, CRSP_OFST_X,, REG_CMP_EQ(c, LOG_CRSP_EN, 1), DIP_A_CRSP_STEP_OFST, CRSP_OFST_X, 0, CRSP_B_offset_x, DIP_A_CRSP_STEP_OFST, CRSP_OFST_X)\
    CMD(a, b, c, d, e, int, CRSP_A_offset_y, crsp.crsp_yoffset, CRSP_OFST_Y,, REG_CMP_EQ(c, LOG_CRSP_EN, 1), DIP_A_CRSP_STEP_OFST, CRSP_OFST_Y, 0, CRSP_B_offset_y, DIP_A_CRSP_STEP_OFST, CRSP_OFST_Y)\
    /* IMG3O */\
    CMD(a, b, c, d, e, int, IMG3O_A_STRIDE, img3o.img3o_stride, IMG3O_STRIDE,, REG_CMP_EQ(c, LOG_IMG3O_EN, 1), DIP_A_IMG3O_STRIDE, STRIDE, 4, IMG3O_B_STRIDE, DIP_A_IMG3O_STRIDE, STRIDE)\
    CMD(a, b, c, d, e, int, IMG3O_A_XOFFSET, img3o.img3o_xoffset, IMG3O_XOFFSET,, REG_CMP_EQ(c, LOG_IMG3O_EN, 1), DIP_A_IMG3O_CROP, XOFFSET, 0, IMG3O_B_XOFFSET, DIP_A_IMG3O_CROP, XOFFSET)\
    CMD(a, b, c, d, e, int, IMG3O_A_YOFFSET, img3o.img3o_yoffset, IMG3O_YOFFSET,, REG_CMP_EQ(c, LOG_IMG3O_EN, 1), DIP_A_IMG3O_CROP, YOFFSET, 0, IMG3O_B_YOFFSET, DIP_A_IMG3O_CROP, YOFFSET)\
    CMD(a, b, c, d, e, int, IMG3O_A_XSIZE, img3o.img3o_xsize, IMG3O_XSIZE,, REG_CMP_EQ(c, LOG_IMG3O_EN, 1), DIP_A_IMG3O_XSIZE, XSIZE, 0, IMG3O_B_XSIZE, DIP_A_IMG3O_XSIZE, XSIZE)\
    CMD(a, b, c, d, e, int, IMG3O_A_YSIZE, img3o.img3o_ysize, IMG3O_YSIZE,, REG_CMP_EQ(c, LOG_IMG3O_EN, 1), DIP_A_IMG3O_YSIZE, YSIZE, 0, IMG3O_B_YSIZE, DIP_A_IMG3O_YSIZE, YSIZE)\
    /* IMG3BO */\
    CMD(a, b, c, d, e, int, IMG3BO_A_STRIDE, img3bo.img3bo_stride, IMG3BO_STRIDE,, REG_CMP_EQ(c, LOG_IMG3BO_EN, 1), DIP_A_IMG3BO_STRIDE, STRIDE, 4, IMG3BO_B_STRIDE, DIP_A_IMG3BO_STRIDE, STRIDE)\
    CMD(a, b, c, d, e, int, IMG3BO_A_XSIZE, img3bo.img3bo_xsize, IMG3BO_XSIZE,, REG_CMP_EQ(c, LOG_IMG3BO_EN, 1), DIP_A_IMG3BO_XSIZE, XSIZE, 0, IMG3BO_B_XSIZE, DIP_A_IMG3BO_XSIZE, XSIZE)\
    CMD(a, b, c, d, e, int, IMG3BO_A_YSIZE, img3bo.img3bo_ysize, IMG3BO_YSIZE,, REG_CMP_EQ(c, LOG_IMG3BO_EN, 1), DIP_A_IMG3BO_YSIZE, YSIZE, 0, IMG3BO_B_YSIZE, DIP_A_IMG3BO_YSIZE, YSIZE)\
    /* IMG3CO */\
    CMD(a, b, c, d, e, int, IMG3CO_A_STRIDE, img3co.img3co_stride, IMG3CO_STRIDE,, REG_CMP_EQ(c, LOG_IMG3CO_EN, 1), DIP_A_IMG3CO_STRIDE, STRIDE, 4, IMG3CO_B_STRIDE, DIP_A_IMG3CO_STRIDE, STRIDE)\
    CMD(a, b, c, d, e, int, IMG3CO_A_XSIZE, img3co.img3co_xsize, IMG3CO_XSIZE,, REG_CMP_EQ(c, LOG_IMG3CO_EN, 1), DIP_A_IMG3CO_XSIZE, XSIZE, 0, IMG3CO_B_XSIZE, DIP_A_IMG3CO_XSIZE, XSIZE)\
    CMD(a, b, c, d, e, int, IMG3CO_A_YSIZE, img3co.img3co_ysize, IMG3CO_YSIZE,, REG_CMP_EQ(c, LOG_IMG3CO_EN, 1), DIP_A_IMG3CO_YSIZE, YSIZE, 0, IMG3CO_B_YSIZE, DIP_A_IMG3CO_YSIZE, YSIZE)\
    /* DMGI */\
    CMD(a, b, c, d, e, int, DMGI_A_V_FLIP_EN, dmgi.dmgi_v_flip_en, DMGI_V_FLIP_EN,, REG_CMP_EQ(c, LOG_DMGI_EN, 1), DIP_A_VERTICAL_FLIP_EN, DMGI_V_FLIP_EN, 4, DMGI_B_V_FLIP_EN, DIP_A_VERTICAL_FLIP_EN, DMGI_V_FLIP_EN)\
    CMD(a, b, c, d, e, int, DMGI_A_OFFSET_ADDR, dmgi.dmgi_offset, DMGI_OFFSET_ADDR,, REG_CMP_EQ(c, LOG_DMGI_EN, 1), DIP_A_DMGI_OFST_ADDR, OFFSET_ADDR, 1, DMGI_B_OFFSET_ADDR, DIP_A_DMGI_OFST_ADDR, OFFSET_ADDR)\
    CMD(a, b, c, d, e, int, DMGI_A_XSIZE, dmgi.dmgi_xsize, DMGI_XSIZE,, REG_CMP_EQ(c, LOG_DMGI_EN, 1), DIP_A_DMGI_XSIZE, XSIZE, 1, DMGI_B_XSIZE, DIP_A_DMGI_XSIZE, XSIZE)\
    CMD(a, b, c, d, e, int, DMGI_A_YSIZE, dmgi.dmgi_ysize, DMGI_YSIZE,, REG_CMP_EQ(c, LOG_DMGI_EN, 1), DIP_A_DMGI_YSIZE, YSIZE, 1, DMGI_B_YSIZE, DIP_A_DMGI_YSIZE, YSIZE)\
    CMD(a, b, c, d, e, int, DMGI_A_STRIDE, dmgi.dmgi_stride, DMGI_STRIDE,, REG_CMP_EQ(c, LOG_DMGI_EN, 1), DIP_A_DMGI_STRIDE, STRIDE, 4, DMGI_B_STRIDE, DIP_A_DMGI_STRIDE, STRIDE)\
    /* DEPI */\
    CMD(a, b, c, d, e, int, DEPI_A_V_FLIP_EN, depi.depi_v_flip_en, DEPI_V_FLIP_EN,, REG_CMP_EQ(c, LOG_DEPI_EN, 1), DIP_A_VERTICAL_FLIP_EN, DEPI_V_FLIP_EN, 4, DEPI_B_V_FLIP_EN, DIP_A_VERTICAL_FLIP_EN, DEPI_V_FLIP_EN)\
    CMD(a, b, c, d, e, int, DEPI_A_XSIZE, depi.depi_xsize, DEPI_XSIZE,, REG_CMP_EQ(c, LOG_DMGI_EN, 1), DIP_A_DEPI_XSIZE, XSIZE, 1, DEPI_B_XSIZE, DIP_A_DEPI_XSIZE, XSIZE)\
    CMD(a, b, c, d, e, int, DEPI_A_YSIZE, depi.depi_ysize, DEPI_YSIZE,, REG_CMP_EQ(c, LOG_DMGI_EN, 1), DIP_A_DEPI_YSIZE, YSIZE, 1, DEPI_B_YSIZE, DIP_A_DEPI_YSIZE, YSIZE)\
    CMD(a, b, c, d, e, int, DEPI_A_STRIDE, depi.depi_stride, DEPI_STRIDE,, REG_CMP_EQ(c, LOG_DEPI_EN, 1), DIP_A_DEPI_STRIDE, STRIDE, 4, DEPI_B_STRIDE, DIP_A_DEPI_STRIDE, STRIDE)\
	/* PCA */\
    CMD(a, b, c, d, e, int, PCA_A_CFC_EN, pca.pca_cfc_en, PCA_CFC_EN,, REG_CMP_EQ(c, LOG_PCA_EN, 1), DIP_A_PCA_CON1, PCA_CFC_EN, 4, PCA_B_CFC_EN, DIP_A_PCA_CON1, PCA_CFC_EN)\
    CMD(a, b, c, d, e, int, PCA_A_CNV_EN, pca.pca_cnv_en, PCA_CNV_EN,, REG_CMP_EQ(c, LOG_PCA_EN, 1), DIP_A_PCA_CON1, PCA_CNV_EN, 4, PCA_B_CNV_EN, DIP_A_PCA_CON1, PCA_CNV_EN)\
	/* SRZ3 */\
    CMD(a, b, c, d, e, int, SRZ3_A_IN_CROP_WD, srz3.srz_input_crop_width, SRZ3_Input_Image_W,, REG_CMP_EQ(c, LOG_SRZ3_EN, 1), DIP_A_SRZ3_IN_IMG, SRZ_IN_WD, 0, SRZ3_B_IN_CROP_WD, DIP_A_SRZ3_IN_IMG, SRZ_IN_WD)\
    CMD(a, b, c, d, e, int, SRZ3_A_IN_CROP_HT, srz3.srz_input_crop_height, SRZ3_Input_Image_H,, REG_CMP_EQ(c, LOG_SRZ3_EN, 1), DIP_A_SRZ3_IN_IMG, SRZ_IN_HT, 0, SRZ3_B_IN_CROP_HT, DIP_A_SRZ3_IN_IMG, SRZ_IN_HT)\
    CMD(a, b, c, d, e, int, SRZ3_A_OUT_WD, srz3.srz_output_width, SRZ3_Output_Image_W,, REG_CMP_EQ(c, LOG_SRZ3_EN, 1), DIP_A_SRZ3_OUT_IMG, SRZ_OUT_WD, 0, SRZ3_B_OUT_WD, DIP_A_SRZ3_OUT_IMG, SRZ_OUT_WD)\
    CMD(a, b, c, d, e, int, SRZ3_A_OUT_HT, srz3.srz_output_height, SRZ3_Output_Image_H,, REG_CMP_EQ(c, LOG_SRZ3_EN, 1), DIP_A_SRZ3_OUT_IMG, SRZ_OUT_HT, 0, SRZ3_B_OUT_HT, DIP_A_SRZ3_OUT_IMG, SRZ_OUT_HT)\
    CMD(a, b, c, d, e, int, SRZ3_A_HORI_INT_OFST, srz3.srz_luma_horizontal_integer_offset, SRZ3_Luma_Horizontal_Integer_Offset,, REG_CMP_EQ(c, LOG_SRZ3_EN, 1), DIP_A_SRZ3_HORI_INT_OFST, SRZ_HORI_INT_OFST, 0, SRZ3_B_HORI_INT_OFST, DIP_A_SRZ3_HORI_INT_OFST, SRZ_HORI_INT_OFST)\
    CMD(a, b, c, d, e, int, SRZ3_A_HORI_SUB_OFST, srz3.srz_luma_horizontal_subpixel_offset, SRZ3_Luma_Horizontal_Subpixel_Offset,, REG_CMP_EQ(c, LOG_SRZ3_EN, 1), DIP_A_SRZ3_HORI_SUB_OFST, SRZ_HORI_SUB_OFST, 0, SRZ3_B_HORI_SUB_OFST, DIP_A_SRZ3_HORI_SUB_OFST, SRZ_HORI_SUB_OFST)\
    CMD(a, b, c, d, e, int, SRZ3_A_VERT_INT_OFST, srz3.srz_luma_vertical_integer_offset, SRZ3_Luma_Vertical_Integer_Offset,, REG_CMP_EQ(c, LOG_SRZ3_EN, 1), DIP_A_SRZ3_VERT_INT_OFST, SRZ_VERT_INT_OFST, 0, SRZ3_B_VERT_INT_OFST, DIP_A_SRZ3_VERT_INT_OFST, SRZ_VERT_INT_OFST)\
    CMD(a, b, c, d, e, int, SRZ3_A_VERT_SUB_OFST, srz3.srz_luma_vertical_subpixel_offset, SRZ3_Luma_Vertical_Subpixel_Offset,, REG_CMP_EQ(c, LOG_SRZ3_EN, 1), DIP_A_SRZ3_VERT_SUB_OFST, SRZ_VERT_SUB_OFST, 0, SRZ3_B_VERT_SUB_OFST, DIP_A_SRZ3_VERT_SUB_OFST, SRZ_VERT_SUB_OFST)\
    CMD(a, b, c, d, e, int, SRZ3_A_HORI_STEP, srz3.srz_horizontal_coeff_step, SRZ3_Horizontal_Coeff_Step,, REG_CMP_EQ(c, LOG_SRZ3_EN, 1), DIP_A_SRZ3_HORI_STEP, SRZ_HORI_STEP, 4, SRZ3_B_HORI_STEP, DIP_A_SRZ3_HORI_STEP, SRZ_HORI_STEP)\
    CMD(a, b, c, d, e, int, SRZ3_A_VERT_STEP, srz3.srz_vertical_coeff_step, SRZ3_Vertical_Coeff_Step,, REG_CMP_EQ(c, LOG_SRZ3_EN, 1), DIP_A_SRZ3_VERT_STEP, SRZ_VERT_STEP, 4, SRZ3_B_VERT_STEP, DIP_A_SRZ3_VERT_STEP, SRZ_VERT_STEP)\
    /* SRZ4 */\
    CMD(a, b, c, d, e, int, SRZ4_A_IN_CROP_WD, srz4.srz_input_crop_width, SRZ4_Input_Image_W,, REG_CMP_EQ(c, LOG_SRZ4_EN, 1), DIP_A_SRZ4_IN_IMG, SRZ_IN_WD, 0, SRZ4_B_IN_CROP_WD, DIP_A_SRZ4_IN_IMG, SRZ_IN_WD)\
    CMD(a, b, c, d, e, int, SRZ4_A_IN_CROP_HT, srz4.srz_input_crop_height, SRZ4_Input_Image_H,, REG_CMP_EQ(c, LOG_SRZ4_EN, 1), DIP_A_SRZ4_IN_IMG, SRZ_IN_HT, 0, SRZ4_B_IN_CROP_HT, DIP_A_SRZ4_IN_IMG, SRZ_IN_HT)\
    CMD(a, b, c, d, e, int, SRZ4_A_OUT_WD, srz4.srz_output_width, SRZ4_Output_Image_W,, REG_CMP_EQ(c, LOG_SRZ4_EN, 1), DIP_A_SRZ4_OUT_IMG, SRZ_OUT_WD, 0, SRZ4_B_OUT_WD, DIP_A_SRZ4_OUT_IMG, SRZ_OUT_WD)\
    CMD(a, b, c, d, e, int, SRZ4_A_OUT_HT, srz4.srz_output_height, SRZ4_Output_Image_H,, REG_CMP_EQ(c, LOG_SRZ4_EN, 1), DIP_A_SRZ4_OUT_IMG, SRZ_OUT_HT, 0, SRZ4_B_OUT_HT, DIP_A_SRZ4_OUT_IMG, SRZ_OUT_HT)\
    CMD(a, b, c, d, e, int, SRZ4_A_HORI_INT_OFST, srz4.srz_luma_horizontal_integer_offset, SRZ4_Luma_Horizontal_Integer_Offset,, REG_CMP_EQ(c, LOG_SRZ4_EN, 1), DIP_A_SRZ4_HORI_INT_OFST, SRZ_HORI_INT_OFST, 0, SRZ4_B_HORI_INT_OFST, DIP_A_SRZ4_HORI_INT_OFST, SRZ_HORI_INT_OFST)\
    CMD(a, b, c, d, e, int, SRZ4_A_HORI_SUB_OFST, srz4.srz_luma_horizontal_subpixel_offset, SRZ4_Luma_Horizontal_Subpixel_Offset,, REG_CMP_EQ(c, LOG_SRZ4_EN, 1), DIP_A_SRZ4_HORI_SUB_OFST, SRZ_HORI_SUB_OFST, 0, SRZ4_B_HORI_SUB_OFST, DIP_A_SRZ4_HORI_SUB_OFST, SRZ_HORI_SUB_OFST)\
    CMD(a, b, c, d, e, int, SRZ4_A_VERT_INT_OFST, srz4.srz_luma_vertical_integer_offset, SRZ4_Luma_Vertical_Integer_Offset,, REG_CMP_EQ(c, LOG_SRZ4_EN, 1), DIP_A_SRZ4_VERT_INT_OFST, SRZ_VERT_INT_OFST, 0, SRZ4_B_VERT_INT_OFST, DIP_A_SRZ4_VERT_INT_OFST, SRZ_VERT_INT_OFST)\
    CMD(a, b, c, d, e, int, SRZ4_A_VERT_SUB_OFST, srz4.srz_luma_vertical_subpixel_offset, SRZ4_Luma_Vertical_Subpixel_Offset,, REG_CMP_EQ(c, LOG_SRZ4_EN, 1), DIP_A_SRZ4_VERT_SUB_OFST, SRZ_VERT_SUB_OFST, 0, SRZ4_B_VERT_SUB_OFST, DIP_A_SRZ4_VERT_SUB_OFST, SRZ_VERT_SUB_OFST)\
    CMD(a, b, c, d, e, int, SRZ4_A_HORI_STEP, srz4.srz_horizontal_coeff_step, SRZ4_Horizontal_Coeff_Step,, REG_CMP_EQ(c, LOG_SRZ4_EN, 1), DIP_A_SRZ4_HORI_STEP, SRZ_HORI_STEP, 4, SRZ4_B_HORI_STEP, DIP_A_SRZ4_HORI_STEP, SRZ_HORI_STEP)\
    CMD(a, b, c, d, e, int, SRZ4_A_VERT_STEP, srz4.srz_vertical_coeff_step, SRZ4_Vertical_Coeff_Step,, REG_CMP_EQ(c, LOG_SRZ4_EN, 1), DIP_A_SRZ4_VERT_STEP, SRZ_VERT_STEP, 4, SRZ4_B_VERT_STEP, DIP_A_SRZ4_VERT_STEP, SRZ_VERT_STEP)\
	/* SL2I */\
    CMD(a, b, c, d, e, int, SL2I_A_HRZ_COMP, sl2i.sl2i_hrz_comp, SL2I_HRZ_COMP,, REG_CMP_EQ(c, LOG_SL2I_EN, 1), DIP_A_SL2I_RZ, SL2_HRZ_COMP, 4, SL2I_B_HRZ_COMP, DIP_A_SL2I_RZ, SL2_HRZ_COMP)\
    CMD(a, b, c, d, e, int, SL2I_A_VRZ_COMP, sl2i.sl2i_vrz_comp, SL2I_VRZ_COMP,, REG_CMP_EQ(c, LOG_SL2I_EN, 1), DIP_A_SL2I_RZ, SL2_VRZ_COMP, 4, SL2I_B_VRZ_COMP, DIP_A_SL2I_RZ, SL2_VRZ_COMP)\
    /* SMX1I */\
    CMD(a, b, c, d, e, int, SMX1I_A_V_FLIP_EN, smx1i.smx1i_v_flip_en, SMX1I_V_FLIP_EN,, REG_CMP_EQ(c, SMX1I_EN, 1), DIP_A_VERTICAL_FLIP_EN, SMX1I_V_FLIP_EN, 1, SMX1I_B_V_FLIP_EN, DIP_A_VERTICAL_FLIP_EN, SMX1I_V_FLIP_EN)\
    CMD(a, b, c, d, e, int, SMX1I_A_XSIZE, smx1i.smx1i_xsize, SMX1I_XSIZE,, REG_CMP_EQ(c, SMX1I_EN, 1), DIP_A_SMX1I_XSIZE, XSIZE, 1, SMX1I_B_XSIZE, DIP_A_SMX1I_XSIZE, XSIZE)\
    CMD(a, b, c, d, e, int, SMX1I_A_YSIZE, smx1i.smx1i_ysize, SMX1I_YSIZE,, REG_CMP_EQ(c, SMX1I_EN, 1), DIP_A_SMX1I_YSIZE, YSIZE, 1, SMX1I_B_YSIZE, DIP_A_SMX1I_YSIZE, YSIZE)\
    /* SMX1O */\
    CMD(a, b, c, d, e, int, SMX1O_A_XSIZE, smx1o.smx1o_xsize, SMX1O_XSIZE,, REG_CMP_EQ(c, SMX1O_EN, 1), DIP_A_SMX1O_XSIZE, XSIZE, 1, SMX1O_B_XSIZE, DIP_A_SMX1O_XSIZE, XSIZE)\
    CMD(a, b, c, d, e, int, SMX1O_A_YSIZE, smx1o.smx1o_ysize, SMX1O_YSIZE,, REG_CMP_EQ(c, SMX1O_EN, 1), DIP_A_SMX1O_YSIZE, YSIZE, 1, SMX1O_B_YSIZE, DIP_A_SMX1O_YSIZE, YSIZE)\
    /* SMX2I */\
    CMD(a, b, c, d, e, int, SMX2I_A_V_FLIP_EN, smx2i.smx2i_v_flip_en, SMX2I_V_FLIP_EN,, REG_CMP_EQ(c, SMX2I_EN, 1), DIP_A_VERTICAL_FLIP_EN, SMX2I_V_FLIP_EN, 1, SMX2I_B_V_FLIP_EN, DIP_A_VERTICAL_FLIP_EN, SMX2I_V_FLIP_EN)\
    CMD(a, b, c, d, e, int, SMX2I_A_XSIZE, smx2i.smx2i_xsize, SMX2I_XSIZE,, REG_CMP_EQ(c, SMX2I_EN, 1), DIP_A_SMX2I_XSIZE, XSIZE, 1, SMX2I_B_XSIZE, DIP_A_SMX2I_XSIZE, XSIZE)\
    CMD(a, b, c, d, e, int, SMX2I_A_YSIZE, smx2i.smx2i_ysize, SMX2I_YSIZE,, REG_CMP_EQ(c, SMX2I_EN, 1), DIP_A_SMX2I_YSIZE, YSIZE, 1, SMX2I_B_YSIZE, DIP_A_SMX2I_YSIZE, YSIZE)\
    /* SMX2O */\
    CMD(a, b, c, d, e, int, SMX2O_A_XSIZE, smx2o.smx2o_xsize, SMX2O_XSIZE,, REG_CMP_EQ(c, SMX2O_EN, 1), DIP_A_SMX2O_XSIZE, XSIZE, 1, SMX2O_B_XSIZE, DIP_A_SMX2O_XSIZE, XSIZE)\
    CMD(a, b, c, d, e, int, SMX2O_A_YSIZE, smx2o.smx2o_ysize, SMX2O_YSIZE,, REG_CMP_EQ(c, SMX2O_EN, 1), DIP_A_SMX2O_YSIZE, YSIZE, 1, SMX2O_B_YSIZE, DIP_A_SMX2O_YSIZE, YSIZE)\
    /* SMX3I */\
    CMD(a, b, c, d, e, int, SMX3I_A_V_FLIP_EN, smx3i.smx3i_v_flip_en, SMX3I_V_FLIP_EN,, REG_CMP_EQ(c, SMX3I_EN, 1), DIP_A_VERTICAL_FLIP_EN, SMX3I_V_FLIP_EN, 1, SMX3I_B_V_FLIP_EN, DIP_A_VERTICAL_FLIP_EN, SMX3I_V_FLIP_EN)\
    CMD(a, b, c, d, e, int, SMX3I_A_XSIZE, smx3i.smx3i_xsize, SMX3I_XSIZE,, REG_CMP_EQ(c, SMX3I_EN, 1), DIP_A_SMX3I_XSIZE, XSIZE, 1, SMX3I_B_XSIZE, DIP_A_SMX3I_XSIZE, XSIZE)\
    CMD(a, b, c, d, e, int, SMX3I_A_YSIZE, smx3i.smx3i_ysize, SMX3I_YSIZE,, REG_CMP_EQ(c, SMX3I_EN, 1), DIP_A_SMX3I_YSIZE, YSIZE, 1, SMX3I_B_YSIZE, DIP_A_SMX3I_YSIZE, YSIZE)\
    /* SMX3O */\
    CMD(a, b, c, d, e, int, SMX3O_A_XSIZE, smx3o.smx3o_xsize, SMX3O_XSIZE,, REG_CMP_EQ(c, SMX2O_EN, 1), DIP_A_SMX3O_XSIZE, XSIZE, 1, SMX3O_B_XSIZE, DIP_A_SMX3O_XSIZE, XSIZE)\
    CMD(a, b, c, d, e, int, SMX3O_A_YSIZE, smx3o.smx3o_ysize, SMX3O_YSIZE,, REG_CMP_EQ(c, SMX2O_EN, 1), DIP_A_SMX3O_YSIZE, YSIZE, 1, SMX3O_B_YSIZE, DIP_A_SMX3O_YSIZE, YSIZE)\
    /* SMX4I */\
    CMD(a, b, c, d, e, int, SMX4I_A_V_FLIP_EN, smx4i.smx4i_v_flip_en, SMX4I_V_FLIP_EN,, REG_CMP_EQ(c, SMX4I_EN, 1), DIP_A_VERTICAL_FLIP_EN, SMX4I_V_FLIP_EN, 1, SMX4I_B_V_FLIP_EN, DIP_A_VERTICAL_FLIP_EN, SMX4I_V_FLIP_EN)\
    CMD(a, b, c, d, e, int, SMX4I_A_XSIZE, smx4i.smx4i_xsize, SMX4I_XSIZE,, REG_CMP_EQ(c, SMX4I_EN, 1), DIP_A_SMX4I_XSIZE, XSIZE, 1, SMX4I_B_XSIZE, DIP_A_SMX4I_XSIZE, XSIZE)\
    CMD(a, b, c, d, e, int, SMX4I_A_YSIZE, smx4i.smx4i_ysize, SMX4I_YSIZE,, REG_CMP_EQ(c, SMX4I_EN, 1), DIP_A_SMX4I_YSIZE, YSIZE, 1, SMX4I_B_YSIZE, DIP_A_SMX4I_YSIZE, YSIZE)\
    /* SMX4O */\
    CMD(a, b, c, d, e, int, SMX4O_A_XSIZE, smx4o.smx4o_xsize, SMX4O_XSIZE,, REG_CMP_EQ(c, SMX4O_EN, 1), DIP_A_SMX4O_XSIZE, XSIZE, 1, SMX4O_B_XSIZE, DIP_A_SMX4O_XSIZE, XSIZE)\
    CMD(a, b, c, d, e, int, SMX4O_A_YSIZE, smx4o.smx4o_ysize, SMX4O_YSIZE,, REG_CMP_EQ(c, SMX4O_EN, 1), DIP_A_SMX4O_YSIZE, YSIZE, 1, SMX4O_B_YSIZE, DIP_A_SMX4O_YSIZE, YSIZE)\

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
    /* Common */\
    CMD(a, b, c, d, e, int, WPE_CTL_EXTENSION_A_EN, top.wpe_ctl_extension_en, WPE_CTL_EXTENSION_EN,, true, WPE_A_CTL_MOD_EN, CTL_EXTENSION, 4, WPE_CTL_EXTENSION_B_EN, WPE_A_CTL_MOD_EN, CTL_EXTENSION)\
	CMD(a, b, c, d, e, int, WPE_A_CACHI_EN, top.cachi_en, CACHI_EN,, true, WPE_A_CTL_DMA_EN, CACHI_EN, 4, WPE_B_CACHI_EN, WPE_A_CTL_DMA_EN, CACHI_EN)\
	CMD(a, b, c, d, e, int, WPE_A_VECI_EN, top.veci_en, VECI_EN,, true, WPE_A_CTL_DMA_EN, VECI_EN, 4, WPE_B_VECI_EN, WPE_A_CTL_DMA_EN, VECI_EN)\
	CMD(a, b, c, d, e, int, WPE_A_VEC2I_EN, top.vec2i_en, VEC2I_EN,, true, WPE_A_CTL_DMA_EN, VEC2I_EN, 4, WPE_B_VEC2I_EN, WPE_A_CTL_DMA_EN, VEC2I_EN)\
	CMD(a, b, c, d, e, int, WPE_A_VEC3I_EN, top.vec3i_en, VEC3I_EN,, true, WPE_A_CTL_DMA_EN, VEC3I_EN, 4, WPE_B_VEC3I_EN, WPE_A_CTL_DMA_EN, VEC3I_EN)\
	CMD(a, b, c, d, e, int, WPE_A_WPEO_EN, top.wpeo_en, WPEO_EN,, true, WPE_A_CTL_DMA_EN, WPEO_EN, 4, WPE_B_WPEO_EN, WPE_A_CTL_DMA_EN, WPEO_EN)\
	CMD(a, b, c, d, e, int, WPE_A_MSKO_EN, top.msko_en, MSKO_EN,, true, WPE_A_CTL_DMA_EN, MSKO_EN, 4, WPE_B_MSKO_EN, WPE_A_CTL_DMA_EN, MSKO_EN)\
	CMD(a, b, c, d, e, int, WPE_A_ISP_CROP_EN, top.wpe_ispcrop_en, WPE_ISP_CROP_EN,, true, WPE_A_CTL_MOD_EN, ISP_CROP_EN, 4, WPE_B_ISP_CROP_EN, WPE_A_CTL_MOD_EN, ISP_CROP_EN)\
	CMD(a, b, c, d, e, int, WPE_A_MDP_CROP_EN, top.wpe_mdpcrop_en, WPE_MDP_CROP_EN,, true, WPE_A_CTL_MOD_EN, MDP_CROP_EN, 4, WPE_B_MDP_CROP_EN, WPE_A_CTL_MOD_EN, MDP_CROP_EN)\
	CMD(a, b, c, d, e, int, WPE_A_C24_EN, top.wpe_c24_en, WPE_C24_EN,, true, WPE_A_CTL_MOD_EN, C24_EN, 4, WPE_B_C24_EN, WPE_A_CTL_MOD_EN, C24_EN)\
	CMD(a, b, c, d, e, int, WPE_A_VGEN_EN, top.vgen_en, VGEN_EN,, true, WPE_A_CTL_MOD_EN, VGEN_EN, 4, WPE_B_VGEN_EN, WPE_A_CTL_MOD_EN, VGEN_EN)\
	CMD(a, b, c, d, e, int, WPE_A_SYNC_EN, top.sync_en, SYNC_EN,, true, WPE_A_CTL_MOD_EN, SYNC_EN, 4, WPE_B_SYNC_EN, WPE_A_CTL_MOD_EN, SYNC_EN)\
	/* CAHCI */\
	CMD(a, b, c, d, e, int, WPE_A_CACHI_FMT, top.wpe_cachi_fmt, WPE_CACHI_FMT,, true, WPE_A_CTL_FMT_SEL, CACHI_FMT, 4, WPE_A_CACHI_FMT, WPE_A_CTL_FMT_SEL, CACHI_FMT)\
    /* VECI */\
    CMD(a, b, c, d, e, int, VECI_A_V_FLIP_EN, veci.veci_v_flip_en, VECI_V_FLIP_EN,, REG_CMP_EQ(c, LOG_VECI_EN, 1), WPE_A_VERTICAL_FLIP_EN, VECI_V_FLIP_EN, 4, VECI_B_V_FLIP_EN, WPE_A_VERTICAL_FLIP_EN, VECI_V_FLIP_EN)\
    CMD(a, b, c, d, e, int, VECI_A_XSIZE, veci.veci_xsize, VECI_XSIZE,, REG_CMP_EQ(c, LOG_VECI_EN, 1), WPE_A_VECI_XSIZE, XSIZE, 0, VECI_B_XSIZE, WPE_A_VECI_XSIZE, XSIZE)\
    CMD(a, b, c, d, e, int, VECI_A_YSIZE, veci.veci_ysize, VECI_YSIZE,, REG_CMP_EQ(c, LOG_VECI_EN, 1), WPE_A_VECI_YSIZE, YSIZE, 0, VECI_B_YSIZE, WPE_A_VECI_YSIZE, YSIZE)\
    CMD(a, b, c, d, e, int, VECI_A_STRIDE, veci.veci_stride, VECI_STRIDE,, REG_CMP_EQ(c, LOG_VECI_EN, 1), WPE_A_VECI_STRIDE, STRIDE, 4, VECI_B_STRIDE, WPE_A_VECI_STRIDE, STRIDE)\
    /* VEC2I */\
    CMD(a, b, c, d, e, int, VEC2I_A_V_FLIP_EN, vec2i.vec2i_v_flip_en, VEC2I_V_FLIP_EN,, REG_CMP_EQ(c, LOG_VEC2I_EN, 1), WPE_A_VERTICAL_FLIP_EN, VEC2I_V_FLIP_EN, 4, VEC2I_B_V_FLIP_EN, WPE_A_VERTICAL_FLIP_EN, VEC2I_V_FLIP_EN)\
    CMD(a, b, c, d, e, int, VEC2I_A_XSIZE, vec2i.vec2i_xsize, VEC2I_XSIZE,, REG_CMP_EQ(c, LOG_VEC2I_EN, 1), WPE_A_VEC2I_XSIZE, XSIZE, 0, VEC2I_B_XSIZE, WPE_A_VEC2I_XSIZE, XSIZE)\
    CMD(a, b, c, d, e, int, VEC2I_A_YSIZE, vec2i.vec2i_ysize, VEC2I_YSIZE,, REG_CMP_EQ(c, LOG_VEC2I_EN, 1), WPE_A_VEC2I_YSIZE, YSIZE, 0, VEC2I_B_YSIZE, WPE_A_VEC2I_YSIZE, YSIZE)\
    CMD(a, b, c, d, e, int, VEC2I_A_STRIDE, vec2i.vec2i_stride, VEC2I_STRIDE,, REG_CMP_EQ(c, LOG_VEC2I_EN, 1), WPE_A_VEC2I_STRIDE, STRIDE, 4, VEC2I_B_STRIDE, WPE_A_VEC2I_STRIDE, STRIDE)\
    /* VEC3I */\
    CMD(a, b, c, d, e, int, VEC3I_A_V_FLIP_EN, vec3i.vec3i_v_flip_en, VEC3I_V_FLIP_EN,, REG_CMP_EQ(c, LOG_VEC3I_EN, 1), WPE_A_VERTICAL_FLIP_EN, VEC3I_V_FLIP_EN, 4, VEC3I_B_V_FLIP_EN, WPE_A_VERTICAL_FLIP_EN, VEC3I_V_FLIP_EN)\
    CMD(a, b, c, d, e, int, VEC3I_A_XSIZE, vec3i.vec3i_xsize, VEC3I_XSIZE,, REG_CMP_EQ(c, LOG_VEC3I_EN, 1), WPE_A_VEC3I_XSIZE, XSIZE, 0, VEC3I_B_XSIZE, WPE_A_VEC3I_XSIZE, XSIZE)\
    CMD(a, b, c, d, e, int, VEC3I_A_YSIZE, vec3i.vec3i_ysize, VEC3I_YSIZE,, REG_CMP_EQ(c, LOG_VEC3I_EN, 1), WPE_A_VEC3I_YSIZE, YSIZE, 0, VEC3I_B_YSIZE, WPE_A_VEC3I_YSIZE, YSIZE)\
    CMD(a, b, c, d, e, int, VEC3I_A_STRIDE, vec3i.vec3i_stride, VEC3I_STRIDE,, REG_CMP_EQ(c, LOG_VEC3I_EN, 1), WPE_A_VEC3I_STRIDE, STRIDE, 4, VEC3I_B_STRIDE, WPE_A_VEC3I_STRIDE, STRIDE)\
    /* VGEN */\
    CMD(a, b, c, d, e, int, WPE_A_VGEN_crop_size_x_d, wpe.vgen_input_crop_width, VGEN_Input_Image_W,, REG_CMP_EQ(c, LOG_WPE_EN, 1), WPE_A_VGEN_IN_IMG, VGEN_IN_WD, 0, WPE_B_VGEN_crop_size_x_d, WPE_A_VGEN_IN_IMG, VGEN_IN_WD)\
    CMD(a, b, c, d, e, int, WPE_A_VGEN_crop_size_y_d, wpe.vgen_input_crop_height, VGEN_Input_Image_H,, REG_CMP_EQ(c, LOG_WPE_EN, 1), WPE_A_VGEN_IN_IMG, VGEN_IN_HT, 0, WPE_B_VGEN_crop_size_y_d, WPE_A_VGEN_IN_IMG, VGEN_IN_HT)\
    CMD(a, b, c, d, e, int, WPE_A_VGEN_OUT_WD, wpe.vgen_output_width, VGEN_Output_Image_W,, REG_CMP_EQ(c, LOG_WPE_EN, 1), WPE_A_VGEN_OUT_IMG, VGEN_OUT_WD, 0, WPE_B_VGEN_OUT_WD, WPE_A_VGEN_OUT_IMG, VGEN_OUT_WD)\
    CMD(a, b, c, d, e, int, WPE_A_VGEN_OUT_HT, wpe.vgen_output_height, VGEN_Output_Image_H,, REG_CMP_EQ(c, LOG_WPE_EN, 1), WPE_A_VGEN_OUT_IMG, VGEN_OUT_HT, 0, WPE_B_VGEN_OUT_HT, WPE_A_VGEN_OUT_IMG, VGEN_OUT_HT)\
    CMD(a, b, c, d, e, int, WPE_A_VGEN_HORI_INT_OFST, wpe.vgen_luma_horizontal_integer_offset, VGEN_Luma_Horizontal_Integer_Offset,, REG_CMP_EQ(c, LOG_WPE_EN, 1), WPE_A_VGEN_HORI_INT_OFST, VGEN_HORI_INT_OFST, 0, WPE_B_VGEN_HORI_INT_OFST, WPE_A_VGEN_HORI_INT_OFST, VGEN_HORI_INT_OFST)\
    CMD(a, b, c, d, e, int, WPE_A_VGEN_HORI_SUB_OFST, wpe.vgen_luma_horizontal_subpixel_offset, VGEN_Luma_Horizontal_Subpixel_Offset,, REG_CMP_EQ(c, LOG_WPE_EN, 1), WPE_A_VGEN_HORI_SUB_OFST, VGEN_HORI_SUB_OFST, 0, WPE_B_VGEN_HORI_SUB_OFST, WPE_A_VGEN_HORI_SUB_OFST, VGEN_HORI_SUB_OFST)\
    CMD(a, b, c, d, e, int, WPE_A_VGEN_VERT_INT_OFST, wpe.vgen_luma_vertical_integer_offset, VGEN_Luma_Vertical_Integer_Offset,, REG_CMP_EQ(c, LOG_WPE_EN, 1), WPE_A_VGEN_VERT_INT_OFST, VGEN_VERT_INT_OFST, 0, WPE_B_VGEN_VERT_INT_OFST, WPE_A_VGEN_VERT_INT_OFST, VGEN_VERT_INT_OFST )\
    CMD(a, b, c, d, e, int, WPE_A_VGEN_VERT_SUB_OFST, wpe.vgen_luma_vertical_subpixel_offset, VGEN_Luma_Vertical_Subpixel_Offset,, REG_CMP_EQ(c, LOG_WPE_EN, 1), WPE_A_VGEN_VERT_SUB_OFST, VGEN_VERT_SUB_OFST, 0, WPE_B_VGEN_VERT_SUB_OFST, WPE_A_VGEN_VERT_SUB_OFST, VGEN_VERT_SUB_OFST)\
    CMD(a, b, c, d, e, int, WPE_A_VGEN_HORI_STEP, wpe.vgen_horizontal_coeff_step, VGEN_Horizontal_Coeff_Step,, REG_CMP_EQ(c, LOG_WPE_EN, 1), WPE_A_VGEN_HORI_STEP, VGEN_HORI_STEP, 4, WPE_B_VGEN_HORI_STEP, WPE_A_VGEN_HORI_STEP, VGEN_HORI_STEP)\
    CMD(a, b, c, d, e, int, WPE_A_VGEN_VERT_STEP, wpe.vgen_vertical_coeff_step, VGEN_Vertical_Coeff_Step,, REG_CMP_EQ(c, LOG_WPE_EN, 1), WPE_A_VGEN_VERT_STEP, VGEN_VERT_STEP, 4, WPE_B_VGEN_VERT_STEP, WPE_A_VGEN_VERT_STEP, VGEN_VERT_STEP)\
    /* WPEO */\
    CMD(a, b, c, d, e, int, WPEO_A_STRIDE, wpeo.wpeo_stride, WPEO_STRIDE,, REG_CMP_EQ(c, LOG_WPEO_EN, 1), WPE_A_WPEO_STRIDE, STRIDE, 4, WPEO_B_STRIDE, WPE_A_WPEO_STRIDE, STRIDE)\
    CMD(a, b, c, d, e, int, WPEO_A_XOFFSET, wpeo.wpeo_xoffset, WPEO_XOFFSET,, REG_CMP_EQ(c, LOG_WPEO_EN, 1), WPE_A_WPEO_CROP, XOFFSET, 0, WPEO_B_XOFFSET, WPE_A_WPEO_CROP, XOFFSET)\
    CMD(a, b, c, d, e, int, WPEO_A_YOFFSET, wpeo.wpeo_yoffset, WPEO_YOFFSET,, REG_CMP_EQ(c, LOG_WPEO_EN, 1), WPE_A_WPEO_CROP, YOFFSET, 0, WPEO_B_YOFFSET, WPE_A_WPEO_CROP, YOFFSET)\
    CMD(a, b, c, d, e, int, WPEO_A_XSIZE, wpeo.wpeo_xsize, WPEO_XSIZE,, REG_CMP_EQ(c, LOG_WPEO_EN, 1), WPE_A_WPEO_XSIZE, XSIZE, 0, WPEO_B_XSIZE, WPE_A_WPEO_XSIZE, XSIZE)\
    CMD(a, b, c, d, e, int, WPEO_A_YSIZE, wpeo.wpeo_ysize, WPEO_YSIZE,, REG_CMP_EQ(c, LOG_WPEO_EN, 1), WPE_A_WPEO_YSIZE, YSIZE, 0, WPEO_B_YSIZE, WPE_A_WPEO_YSIZE, YSIZE)\
    /* MSKO */\
    CMD(a, b, c, d, e, int, MSKO_A_STRIDE, msko.msko_stride, MSKO_STRIDE,, REG_CMP_EQ(c, LOG_MSKO_EN, 1), WPE_A_MSKO_STRIDE, STRIDE, 4, MSKO_B_STRIDE, WPE_A_MSKO_STRIDE, STRIDE)\
    CMD(a, b, c, d, e, int, MSKO_A_XOFFSET, msko.msko_xoffset, MSKO_XOFFSET,, REG_CMP_EQ(c, LOG_MSKO_EN, 1), WPE_A_MSKO_CROP, XOFFSET, 0, MSKO_B_XOFFSET, WPE_A_MSKO_CROP, XOFFSET)\
    CMD(a, b, c, d, e, int, MSKO_A_YOFFSET, msko.msko_yoffset, MSKO_YOFFSET,, REG_CMP_EQ(c, LOG_MSKO_EN, 1), WPE_A_MSKO_CROP, YOFFSET, 0, MSKO_B_YOFFSET, WPE_A_MSKO_CROP, YOFFSET)\
    CMD(a, b, c, d, e, int, MSKO_A_XSIZE, msko.msko_xsize, MSKO_XSIZE,, REG_CMP_EQ(c, LOG_MSKO_EN, 1), WPE_A_MSKO_XSIZE, XSIZE, 0, MSKO_B_XSIZE, WPE_A_MSKO_XSIZE, XSIZE)\
    CMD(a, b, c, d, e, int, MSKO_A_YSIZE, msko.msko_ysize, MSKO_YSIZE,, REG_CMP_EQ(c, LOG_MSKO_EN, 1), WPE_A_MSKO_YSIZE, YSIZE, 0, MSKO_B_YSIZE, WPE_A_MSKO_YSIZE, YSIZE)\

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
#define WPE2_TILE_HW_REG_LUT(CMD, a, b, c, d, e) \
    /* Common */\

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
#define EAF_TILE_HW_REG_LUT(CMD, a, b, c, d, e) \
    /* Common */\
	CMD(a, b, c, d, e, int, EAF_A_EN, top.eaf_en, EAF_EN,, true, EAF_A_MAIN_CFG0, EAF_EN, 4, EAF_A_EN, EAF_A_MAIN_CFG0, EAF_EN)\
	CMD(a, b, c, d, e, int, EAFI_A_MASK_EN, top.eafi_mask_en, EAFI_MASK_EN,, true, EAF_A_MAIN_CFG1, EAFI_MASK_EN, 4, EAFI_A_MASK_EN, EAF_A_MAIN_CFG1, EAFI_MASK_EN)\
	CMD(a, b, c, d, e, int, EAFI_A_CUR_Y_EN, top.eafi_cur_y_en, EAFI_CUR_Y_EN,, true, EAF_A_MAIN_CFG1, EAFI_CUR_Y_EN, 4, EAFI_A_CUR_Y_EN, EAF_A_MAIN_CFG1, EAFI_CUR_Y_EN)\
	CMD(a, b, c, d, e, int, EAFI_A_CUR_UV_EN, top.eafi_cur_uv_en, EAFI_CUR_UV_EN,, true, EAF_A_MAIN_CFG1, EAFI_CUR_UV_EN, 42, EAFI_A_CUR_UV_EN, EAF_A_MAIN_CFG1, EAFI_CUR_UV_EN)\
	CMD(a, b, c, d, e, int, EAFI_A_PRE_Y_EN, top.eafi_pre_y_en, EAFI_PRE_Y_EN,, true, EAF_A_MAIN_CFG1, EAFI_PRE_Y_EN, 4, EAFI_A_PRE_Y_EN, EAF_A_MAIN_CFG1, EAFI_PRE_Y_EN)\
	CMD(a, b, c, d, e, int, EAFI_A_PRE_UV_EN, top.eafi_pre_uv_en, EAFI_PRE_UV_EN,, true, EAF_A_MAIN_CFG1, EAFI_PRE_UV_EN, 4, EAFI_A_PRE_UV_EN, EAF_A_MAIN_CFG1, EAFI_PRE_UV_EN)\
	CMD(a, b, c, d, e, int, EAFI_A_LKH_WMAP_EN, top.eafi_lkh_wmap_en, EAFI_LKH_WMAP_EN,, true, EAF_A_MAIN_CFG1, EAFI_LKH_WMAP_EN, 4, EAFI_A_LKH_WMAP_EN, EAF_A_MAIN_CFG1, EAFI_LKH_WMAP_EN)\
	CMD(a, b, c, d, e, int, EAFI_A_LKH_EMAP_EN, top.eafi_lkh_emap_en, EAFI_LKH_EMAP_EN,, true, EAF_A_MAIN_CFG1, EAFI_LKH_EMAP_EN, 4, EAFI_A_LKH_EMAP_EN, EAF_A_MAIN_CFG1, EAFI_LKH_EMAP_EN)\
	CMD(a, b, c, d, e, int, EAFI_A_DEPTH_EN, top.eafi_depth_en, EAFI_DEPTH_EN,, true, EAF_A_MAIN_CFG1, EAFI_DEP_EN, 4, EAFI_A_DEPTH_EN, EAF_A_MAIN_CFG1, EAFI_DEP_EN)\
	CMD(a, b, c, d, e, int, SRZ6_A_EN, top.srz6_en, SRZ6_EN,, true, EAF_A_MAIN_CFG0, EAF_SRZ_EN, 4, SRZ6_A_EN, EAF_A_MAIN_CFG0, EAF_SRZ_EN)\
	CMD(a, b, c, d, e, int, EAFO_A_FOUT_EN, top.eafo_fout_en, EAFO_FOUT_EN,, true, EAF_A_MAIN_CFG1, EAFO_FOUT_EN, 4, EAFO_A_FOUT_EN, EAF_A_MAIN_CFG1, EAFO_FOUT_EN)\
	CMD(a, b, c, d, e, int, EAF_A_JBFR_REF_SEL, eaf.jbfr_ref_sel, EAF_JBFR_REF_SEL,, true, EAF_A_MAIN_CFG0, EAF_JBFR_REF_SEL, 4, EAF_A_JBFR_REF_SEL, EAF_A_MAIN_CFG0, EAF_JBFR_REF_SEL)\
	CMD(a, b, c, d, e, int, EAF_A_JBFR_KER_MODE, eaf.jbfr_ker_mode, EAF_JBFR_KER_MODE,, true, EAF_A_JBFR_CFG0, EAF_JBFR_KER_MODE, 4, EAF_A_JBFR_KER_MODE, EAF_A_JBFR_CFG0, EAF_JBFR_KER_MODE)\
    /* EAFI */\
    CMD(a, b, c, d, e, int, EAFI_A_MASK_XSIZE, eafi.mask_xsize, EAFI_MASK_XSIZE,, REG_CMP_EQ(c, LOG_EAFI_MASK_EN, 1), EAF_A_EAFI_MASK3, EAFI_MASK_XSIZE, 0, EAFI_A_MASK_XSIZE, EAF_A_EAFI_MASK3, EAFI_MASK_XSIZE)\
    CMD(a, b, c, d, e, int, EAFI_A_MASK_YSIZE, eafi.mask_ysize, EAFI_MASK_YSIZE,, REG_CMP_EQ(c, LOG_EAFI_MASK_EN, 1), EAF_A_EAFI_MASK3, EAFI_MASK_YSIZE, 0, EAFI_A_MASK_YSIZE, EAF_A_EAFI_MASK3, EAFI_MASK_YSIZE)\
    CMD(a, b, c, d, e, int, EAFI_A_MASK_STRIDE, eafi.mask_stride, EAFI_MASK_STRIDE,, REG_CMP_EQ(c, LOG_EAFI_MASK_EN, 1), EAF_A_EAFI_MASK2, EAFI_MASK_STRIDE, 4, EAFI_A_MASK_STRIDE, EAF_A_EAFI_MASK2, EAFI_MASK_STRIDE)\
    CMD(a, b, c, d, e, int, EAFI_A_CUR_Y_XSIZE, eafi.cur_y_xsize, EAFI_CUR_Y_XSIZE,, REG_CMP_EQ(c, LOG_EAFI_CUR_Y_EN, 1), EAF_A_EAFI_CUR_Y3, EAFI_CUR_Y_XSIZE, 0, EAFI_A_CUR_Y_XSIZE, EAF_A_EAFI_CUR_Y3, EAFI_CUR_Y_XSIZE)\
    CMD(a, b, c, d, e, int, EAFI_A_CUR_Y_YSIZE, eafi.cur_y_ysize, EAFI_CUR_Y_YSIZE,, REG_CMP_EQ(c, LOG_EAFI_CUR_Y_EN, 1), EAF_A_EAFI_CUR_Y3, EAFI_CUR_Y_YSIZE, 0, EAFI_A_CUR_Y_YSIZE, EAF_A_EAFI_CUR_Y3, EAFI_CUR_Y_YSIZE)\
    CMD(a, b, c, d, e, int, EAFI_A_CUR_Y_STRIDE, eafi.cur_y_stride, EAFI_CUR_Y_STRIDE,, REG_CMP_EQ(c, LOG_EAFI_CUR_Y_EN, 1), EAF_A_EAFI_CUR_Y2, EAFI_CUR_Y_STRIDE, 4, EAFI_A_CUR_Y_STRIDE, EAF_A_EAFI_CUR_Y2, EAFI_CUR_Y_STRIDE)\
    CMD(a, b, c, d, e, int, EAFI_A_CUR_UV_STRIDE, eafi.cur_uv_stride, EAFI_CUR_UV_STRIDE,, REG_CMP_EQ(c, LOG_EAFI_CUR_UV_EN, 1), EAF_A_EAFI_CUR_UV2, EAFI_CUR_UV_STRIDE, 4, EAFI_A_CUR_UV_STRIDE, EAF_A_EAFI_CUR_UV2, EAFI_CUR_UV_STRIDE)\
	CMD(a, b, c, d, e, int, EAFI_A_LKH_WMAP_STRIDE, eafi.lkh_wmap_stride, EAFI_LKH_WMAP_STRIDE,, REG_CMP_EQ(c, LOG_EAFI_LKH_WMAP_EN, 1), EAF_A_EAFI_LKH_WMAP2, EAFI_LKH_WMAP_STRIDE, 4, EAFI_A_LKH_WMAP_STRIDE, EAF_A_EAFI_LKH_WMAP2, EAFI_LKH_WMAP_STRIDE)\
    CMD(a, b, c, d, e, int, EAFI_A_LKH_EMAP_STRIDE, eafi.lkh_emap_stride, EAFI_LKH_EMAP_STRIDE,, REG_CMP_EQ(c, LOG_EAFI_LKH_EMAP_EN, 1), EAF_A_EAFI_LKH_EMAP2, EAFI_LKH_EMAP_STRIDE, 4, EAFI_A_LKH_EMAP_STRIDE, EAF_A_EAFI_LKH_EMAP2, EAFI_LKH_EMAP_STRIDE)\
    CMD(a, b, c, d, e, int, EAFI_A_DEPTH_STRIDE, eafi.depth_stride, EAFI_DEPTH_STRIDE,, REG_CMP_EQ(c, LOG_EAFI_DEPTH_EN, 1), EAF_A_EAFI_DEP2, EAFI_DEP_STRIDE, 4, EAFI_A_DEPTH_STRIDE, EAF_A_EAFI_DEP2, EAFI_DEP_STRIDE)\
    CMD(a, b, c, d, e, int, EAFI_A_PRE_Y_STRIDE, eafi.pre_y_stride, EAFI_PRE_Y_STRIDE,, REG_CMP_EQ(c, LOG_EAFI_PRE_Y_EN, 1), EAF_A_EAFI_PRE_Y2, EAFI_PRE_Y_STRIDE, 4, EAFI_A_PRE_Y_STRIDE, EAF_A_EAFI_PRE_Y2, EAFI_PRE_Y_STRIDE)\
    CMD(a, b, c, d, e, int, EAFI_A_PRE_UV_STRIDE, eafi.pre_uv_stride, EAFI_PRE_UV_STRIDE,, REG_CMP_EQ(c, LOG_EAFI_PRE_UV_EN, 1), EAF_A_EAFI_PRE_UV2, EAFI_PRE_UV_STRIDE, 4, EAFI_A_PRE_UV_STRIDE, EAF_A_EAFI_PRE_UV2, EAFI_PRE_UV_STRIDE)\
    /* EAFO */\
    CMD(a, b, c, d, e, int, EAFO_A_FOUT_STRIDE, eafo.fout_stride, EAFO_FOUT_STRIDE,, REG_CMP_EQ(c, LOG_EAFO_FOUT_EN, 1), EAF_A_EAFO_FOUT2, EAFO_FOUT_STRIDE, 4, EAFO_A_FOUT_STRIDE, EAF_A_EAFO_FOUT2, EAFO_FOUT_STRIDE)\
    CMD(a, b, c, d, e, int, EAFO_A_FOUT_XOFFSET, eafo.fout_xoffset, EAFO_FOUT_XOFFSET,, REG_CMP_EQ(c, LOG_EAFO_FOUT_EN, 1), EAF_A_EAFO_FOUT4, EAFO_FOUT_XOFST, 0, EAFO_A_FOUT_XOFFSET, EAF_A_EAFO_FOUT4, EAFO_FOUT_XOFST)\
    CMD(a, b, c, d, e, int, EAFO_A_FOUT_YOFFSET, eafo.fout_yoffset, EAFO_FOUT_YOFFSET,, REG_CMP_EQ(c, LOG_EAFO_FOUT_EN, 1), EAF_A_EAFO_FOUT4, EAFO_FOUT_YOFST, 0, EAFO_A_FOUT_YOFFSET, EAF_A_EAFO_FOUT4, EAFO_FOUT_YOFST)\
    CMD(a, b, c, d, e, int, EAFO_A_FOUT_XSIZE, eafo.fout_xsize, EAFO_FOUT_XSIZE,, REG_CMP_EQ(c, LOG_EAFO_FOUT_EN, 1), EAF_A_EAFO_FOUT3, EAFO_FOUT_XSIZE, 0, EAFO_A_FOUT_XSIZE, EAF_A_EAFO_FOUT3, EAFO_FOUT_XSIZE)\
    CMD(a, b, c, d, e, int, EAFO_A_FOUT_YSIZE, eafo.fout_ysize, EAFO_FOUT_YSIZE,, REG_CMP_EQ(c, LOG_EAFO_FOUT_EN, 1), EAF_A_EAFO_FOUT3, EAFO_FOUT_YSIZE, 0, EAFO_A_FOUT_YSIZE, EAF_A_EAFO_FOUT3, EAFO_FOUT_YSIZE)\
	/* SRZ6 */\
    CMD(a, b, c, d, e, int, SRZ6_A_IN_CROP_WD, srz6.srz_input_crop_width, SRZ6_Input_Image_W,, REG_CMP_EQ(c, LOG_SRZ6_EN, 1), EAF_A_SRZ6_IN_IMG, SRZ_IN_WD, 0, SRZ6_A_IN_CROP_WD, EAF_A_SRZ6_IN_IMG, SRZ_IN_WD)\
    CMD(a, b, c, d, e, int, SRZ6_A_IN_CROP_HT, srz6.srz_input_crop_height, SRZ6_Input_Image_H,, REG_CMP_EQ(c, LOG_SRZ6_EN, 1), EAF_A_SRZ6_IN_IMG, SRZ_IN_HT, 0, SRZ6_A_IN_CROP_HT, EAF_A_SRZ6_IN_IMG, SRZ_IN_HT)\
    CMD(a, b, c, d, e, int, SRZ6_A_OUT_WD, srz6.srz_output_width, SRZ6_Output_Image_W,, REG_CMP_EQ(c, LOG_SRZ6_EN, 1), EAF_A_SRZ6_OUT_IMG, SRZ_OUT_WD, 0, SRZ6_A_OUT_WD, EAF_A_SRZ6_OUT_IMG, SRZ_OUT_WD)\
    CMD(a, b, c, d, e, int, SRZ6_A_OUT_HT, srz6.srz_output_height, SRZ6_Output_Image_H,, REG_CMP_EQ(c, LOG_SRZ6_EN, 1), EAF_A_SRZ6_OUT_IMG, SRZ_OUT_HT, 0, SRZ6_A_OUT_HT, EAF_A_SRZ6_OUT_IMG, SRZ_OUT_HT)\
    CMD(a, b, c, d, e, int, SRZ6_A_HORI_INT_OFST, srz6.srz_luma_horizontal_integer_offset, SRZ6_Luma_Horizontal_Integer_Offset,, REG_CMP_EQ(c, LOG_SRZ6_EN, 1), EAF_A_SRZ6_HORI_INT_OFST, SRZ_HORI_INT_OFST, 0, SRZ6_A_HORI_INT_OFST, EAF_A_SRZ6_HORI_INT_OFST, SRZ_HORI_INT_OFST)\
    CMD(a, b, c, d, e, int, SRZ6_A_HORI_SUB_OFST, srz6.srz_luma_horizontal_subpixel_offset, SRZ6_Luma_Horizontal_Subpixel_Offset,, REG_CMP_EQ(c, LOG_SRZ6_EN, 1), EAF_A_SRZ6_HORI_SUB_OFST, SRZ_HORI_SUB_OFST, 0, SRZ6_A_HORI_SUB_OFST, EAF_A_SRZ6_HORI_SUB_OFST, SRZ_HORI_SUB_OFST)\
    CMD(a, b, c, d, e, int, SRZ6_A_VERT_INT_OFST, srz6.srz_luma_vertical_integer_offset, SRZ6_Luma_Vertical_Integer_Offset,, REG_CMP_EQ(c, LOG_SRZ6_EN, 1), EAF_A_SRZ6_VERT_INT_OFST, SRZ_VERT_INT_OFST, 0, SRZ6_A_VERT_INT_OFST, EAF_A_SRZ6_VERT_INT_OFST, SRZ_VERT_INT_OFST)\
    CMD(a, b, c, d, e, int, SRZ6_A_VERT_SUB_OFST, srz6.srz_luma_vertical_subpixel_offset, SRZ6_Luma_Vertical_Subpixel_Offset,, REG_CMP_EQ(c, LOG_SRZ6_EN, 1), EAF_A_SRZ6_VERT_SUB_OFST, SRZ_VERT_SUB_OFST, 0, SRZ6_A_VERT_SUB_OFST, EAF_A_SRZ6_VERT_SUB_OFST, SRZ_VERT_SUB_OFST)\
    CMD(a, b, c, d, e, int, SRZ6_A_HORI_STEP, srz6.srz_horizontal_coeff_step, SRZ6_Horizontal_Coeff_Step,, REG_CMP_EQ(c, LOG_SRZ6_EN, 1), EAF_A_SRZ6_HORI_STEP, SRZ_HORI_STEP, 4, SRZ6_A_HORI_STEP, EAF_A_SRZ6_HORI_STEP, SRZ_HORI_STEP)\
    CMD(a, b, c, d, e, int, SRZ6_A_VERT_STEP, srz6.srz_vertical_coeff_step, SRZ6_Vertical_Coeff_Step,, REG_CMP_EQ(c, LOG_SRZ6_EN, 1), EAF_A_SRZ6_VERT_STEP, SRZ_VERT_STEP, 4, SRZ6_A_VERT_STEP, EAF_A_SRZ6_VERT_STEP, SRZ_VERT_STEP)\

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
#define BLD_TILE_HW_REG_LUT(CMD, a, b, c, d, e) \
    /* Common */\
    CMD(a, b, c, d, e, int, BLDI_A_EN, top.bldi_en, BLDI_EN,, true, MFB_A_MFB_TOP_CFG2, MFB_MFBI_EN, 4, BLDI_A_EN, MFB_A_MFB_TOP_CFG2, MFB_MFBI_EN)\
    CMD(a, b, c, d, e, int, BLD2I_A_EN, top.bld2i_en, BLD2I_EN,, true, MFB_A_MFB_TOP_CFG2, MFB_MFB2I_EN, 4, BLD2I_A_EN, MFB_A_MFB_TOP_CFG2, MFB_MFB2I_EN)\
    CMD(a, b, c, d, e, int, BLD3I_A_EN, top.bld3i_en, BLD3I_EN,, true, MFB_A_MFB_TOP_CFG2, MFB_MFB3I_EN, 4, BLD3I_A_EN, MFB_A_MFB_TOP_CFG2, MFB_MFB3I_EN)\
    CMD(a, b, c, d, e, int, BLD4I_A_EN, top.bld4i_en, BLD4I_EN,, true, MFB_A_MFB_TOP_CFG2, MFB_MFB4I_EN, 4, BLD4I_A_EN, MFB_A_MFB_TOP_CFG2, MFB_MFB4I_EN)\
    CMD(a, b, c, d, e, int, MFB_S_EN, top.bld_en, BLD_EN,, true, MFB_A_MFB_TOP_CFG0, MFB_EN, 4, MFB_S_EN, MFB_A_MFB_TOP_CFG0, MFB_EN)\
    CMD(a, b, c, d, e, int, BLDO_A_EN, top.bldo_en, BLDO_EN,, true, MFB_A_MFB_TOP_CFG2, MFB_MFBO_EN, 4, BLDO_A_EN, MFB_A_MFB_TOP_CFG2, MFB_MFBO_EN)\
    CMD(a, b, c, d, e, int, BLD2O_A_EN, top.bld2o_en, BLD2O_EN,, true, MFB_A_MFB_TOP_CFG2, MFB_MFB2O_EN, 4, BLD2O_A_EN, MFB_A_MFB_TOP_CFG2, MFB_MFB2O_EN)\
    CMD(a, b, c, d, e, int, MFB_SRZ_A_EN, top.mfb_srz_en, MFB_SRZ_EN,, true, MFB_A_MFB_TOP_CFG0, MFB_SRZ_EN, 4, MFB_SRZ_A_EN, MFB_A_MFB_TOP_CFG0, MFB_SRZ_EN)\
    CMD(a, b, c, d, e, int, BLDI_FMT_A, top.bldi_fmt, BLDI_FMT,, true, MFB_A_MFBI_STRIDE, FORMAT, 4, BLDI_FMT_A, MFB_A_MFBI_STRIDE, FORMAT)\
    /* BLDI */\
    CMD(a, b, c, d, e, int, BLDI_A_V_FLIP_EN, bldi.bldi_v_flip_en, BLDI_V_FLIP_EN,, REG_CMP_EQ(c, LOG_BLDI_EN, 1), MFB_A_VERTICAL_FLIP_EN, MFBI_V_FLIP_EN, 4, BLDI_A_V_FLIP_EN, MFB_A_VERTICAL_FLIP_EN, MFBI_V_FLIP_EN)\
    CMD(a, b, c, d, e, int, BLDI_A_STRIDE, bldi.bldi_stride, BLDI_STRIDE,, REG_CMP_EQ(c, LOG_BLDI_EN, 1), MFB_A_MFBI_STRIDE, STRIDE, 4, BLDI_A_STRIDE, MFB_A_MFBI_STRIDE, STRIDE)\
    CMD(a, b, c, d, e, int, BLDBI_A_V_FLIP_EN, bldbi.bldbi_v_flip_en, BLDBI_V_FLIP_EN,, REG_CMP_EQ(c, LOG_BLDBI_EN, 1), MFB_A_VERTICAL_FLIP_EN, MFBI_B_V_FLIP_EN, 4, BLDBI_A_V_FLIP_EN, MFB_A_VERTICAL_FLIP_EN, MFBI_B_V_FLIP_EN)\
    CMD(a, b, c, d, e, int, BLDBI_A_STRIDE, bldbi.bldbi_stride, BLDBI_STRIDE,, REG_CMP_EQ(c, LOG_BLDBI_EN, 1), MFB_A_MFBI_B_STRIDE, STRIDE, 4, BLDBI_A_STRIDE, MFB_A_MFBI_B_STRIDE, STRIDE)\
    CMD(a, b, c, d, e, int, BLD2I_A_V_FLIP_EN, bld2i.bld2i_v_flip_en, BLD2I_V_FLIP_EN,, REG_CMP_EQ(c, LOG_BLD2I_EN, 1), MFB_A_VERTICAL_FLIP_EN, MFB2I_V_FLIP_EN, 4, BLD2I_A_V_FLIP_EN, MFB_A_VERTICAL_FLIP_EN, MFB2I_V_FLIP_EN)\
    CMD(a, b, c, d, e, int, BLD2I_A_STRIDE, bld2i.bld2i_stride, BLD2I_STRIDE,, REG_CMP_EQ(c, LOG_BLD2I_EN, 1), MFB_A_MFB2I_STRIDE, STRIDE, 4, BLD2I_A_STRIDE, MFB_A_MFB2I_STRIDE, STRIDE)\
    CMD(a, b, c, d, e, int, BLD2BI_A_V_FLIP_EN, bld2bi.bld2bi_v_flip_en, BLD2BI_V_FLIP_EN,, REG_CMP_EQ(c, LOG_BLD2BI_EN, 1), MFB_A_VERTICAL_FLIP_EN, MFB2I_B_V_FLIP_EN, 4, BLD2BI_A_V_FLIP_EN, MFB_A_VERTICAL_FLIP_EN, MFB2I_B_V_FLIP_EN)\
    CMD(a, b, c, d, e, int, BLD2BI_A_STRIDE, bld2bi.bld2bi_stride, BLD2BI_STRIDE,, REG_CMP_EQ(c, LOG_BLD2BI_EN, 1), MFB_A_MFB2I_B_STRIDE, STRIDE, 4, BLD2BI_A_STRIDE, MFB_A_MFB2I_B_STRIDE, STRIDE)\
    CMD(a, b, c, d, e, int, BLD3I_A_V_FLIP_EN, bld3i.bld3i_v_flip_en, BLD3I_V_FLIP_EN,, REG_CMP_EQ(c, LOG_BLD3I_EN, 1), MFB_A_VERTICAL_FLIP_EN, MFB3I_V_FLIP_EN, 4, BLD3I_A_V_FLIP_EN, MFB_A_VERTICAL_FLIP_EN, MFB3I_V_FLIP_EN)\
    CMD(a, b, c, d, e, int, BLD3I_A_STRIDE, bld3i.bld3i_stride, BLD3I_STRIDE,, REG_CMP_EQ(c, LOG_BLD3I_EN, 1), MFB_A_MFB3I_STRIDE, STRIDE, 4, BLD3I_A_STRIDE, MFB_A_MFB3I_STRIDE, STRIDE)\
    CMD(a, b, c, d, e, int, BLD4I_A_V_FLIP_EN, bld4i.bld4i_v_flip_en, BLD4I_V_FLIP_EN,, REG_CMP_EQ(c, LOG_BLD4I_EN, 1), MFB_A_VERTICAL_FLIP_EN, MFB4I_V_FLIP_EN, 4, BLD4I_A_V_FLIP_EN, MFB_A_VERTICAL_FLIP_EN, MFB4I_V_FLIP_EN)\
    CMD(a, b, c, d, e, int, BLD4I_A_XSIZE, bld4i.bld4i_xsize, BLD4I_XSIZE,, REG_CMP_EQ(c, LOG_BLD4I_EN, 1), MFB_A_MFB4I_XSIZE, XSIZE, 0, BLD4I_A_XSIZE, MFB_A_MFB4I_XSIZE, XSIZE)\
    CMD(a, b, c, d, e, int, BLD4I_A_YSIZE, bld4i.bld4i_ysize, BLD4I_YSIZE,, REG_CMP_EQ(c, LOG_BLD4I_EN, 1), MFB_A_MFB4I_YSIZE, YSIZE, 0, BLD4I_A_YSIZE, MFB_A_MFB4I_YSIZE, YSIZE)\
    CMD(a, b, c, d, e, int, BLD4I_A_STRIDE, bld4i.bld4i_stride, BLD4I_STRIDE,, REG_CMP_EQ(c, LOG_BLD4I_EN, 1), MFB_A_MFB4I_STRIDE, STRIDE, 4, BLD4I_A_STRIDE, MFB_A_MFB4I_STRIDE, STRIDE)\
    /* BLD */\
    CMD(a, b, c, d, e, int, BLD_S_LL_DB_EN, bld.bld_deblock_en, BLD_S_DEBLOCK_EN,, REG_CMP_EQ(c, LOG_BLD_EN, 1), MFB_A_MFB_CON, BLD_LL_DB_EN, 4, BLD_S_LL_DB_EN, MFB_A_MFB_CON, BLD_LL_DB_EN)\
    CMD(a, b, c, d, e, int, BLD_S_LL_BRZ_EN, bld.bld_brz_en, BLD_S_LL_BRZ_EN,, REG_CMP_EQ(c, LOG_BLD_EN, 1), MFB_A_MFB_CON, BLD_LL_BRZ_EN, 4, BLD_S_LL_BRZ_EN, MFB_A_MFB_CON, BLD_LL_BRZ_EN)\
    CMD(a, b, c, d, e, int, BLD_S_MBD_WT_EN, bld.bld_mbd_wt_en, BLD_S_MBD_WT_EN,, REG_CMP_EQ(c, LOG_BLD_EN, 1), MFB_A_MFB_CON, BLD_MBD_WT_EN, 4, BLD_S_MBD_WT_EN, MFB_A_MFB_CON, BLD_MBD_WT_EN)\
    /* MFB_SRZ */\
    CMD(a, b, c, d, e, int, MFB_SRZ_A_IN_CROP_WD, mfb_srz.srz_input_crop_width, MFB_SRZ_Input_Image_W,, REG_CMP_EQ(c, LOG_MFB_SRZ_EN, 1), MFB_A_SRZ_IN_IMG, SRZ_IN_WD, 0, MFB_SRZ_A_IN_CROP_WD, MFB_A_SRZ_IN_IMG, SRZ_IN_WD)\
    CMD(a, b, c, d, e, int, MFB_SRZ_A_IN_CROP_HT, mfb_srz.srz_input_crop_height, MFB_SRZ_Input_Image_H,, REG_CMP_EQ(c, LOG_MFB_SRZ_EN, 1), MFB_A_SRZ_IN_IMG, SRZ_IN_HT, 0, MFB_SRZ_A_IN_CROP_HT, MFB_A_SRZ_IN_IMG, SRZ_IN_HT)\
    CMD(a, b, c, d, e, int, MFB_SRZ_A_OUT_WD, mfb_srz.srz_output_width, MFB_SRZ_Output_Image_W,, REG_CMP_EQ(c, LOG_MFB_SRZ_EN, 1), MFB_A_SRZ_OUT_IMG, SRZ_OUT_WD, 0, MFB_SRZ_A_OUT_WD, MFB_A_SRZ_OUT_IMG, SRZ_OUT_WD)\
    CMD(a, b, c, d, e, int, MFB_SRZ_A_OUT_HT, mfb_srz.srz_output_height, MFB_SRZ_Output_Image_H,, REG_CMP_EQ(c, LOG_MFB_SRZ_EN, 1), MFB_A_SRZ_OUT_IMG, SRZ_OUT_HT, 0, MFB_SRZ_A_OUT_HT, MFB_A_SRZ_OUT_IMG, SRZ_OUT_HT)\
    CMD(a, b, c, d, e, int, MFB_SRZ_A_HORI_INT_OFST, mfb_srz.srz_luma_horizontal_integer_offset, MFB_SRZ_Luma_Horizontal_Integer_Offset,, REG_CMP_EQ(c, LOG_MFB_SRZ_EN, 1), MFB_A_SRZ_HORI_INT_OFST, SRZ_HORI_INT_OFST, 0, MFB_SRZ_A_HORI_INT_OFST, MFB_A_SRZ_HORI_INT_OFST, SRZ_HORI_INT_OFST)\
    CMD(a, b, c, d, e, int, MFB_SRZ_A_HORI_SUB_OFST, mfb_srz.srz_luma_horizontal_subpixel_offset, MFB_SRZ_Luma_Horizontal_Subpixel_Offset,, REG_CMP_EQ(c, LOG_MFB_SRZ_EN, 1), MFB_A_SRZ_HORI_SUB_OFST, SRZ_HORI_SUB_OFST, 0, MFB_SRZ_A_HORI_SUB_OFST, MFB_A_SRZ_HORI_SUB_OFST, SRZ_HORI_SUB_OFST)\
    CMD(a, b, c, d, e, int, MFB_SRZ_A_VERT_INT_OFST, mfb_srz.srz_luma_vertical_integer_offset, MFB_SRZ_Luma_Vertical_Integer_Offset,, REG_CMP_EQ(c, LOG_MFB_SRZ_EN, 1), MFB_A_SRZ_VERT_INT_OFST, SRZ_VERT_INT_OFST, 0, MFB_SRZ_A_VERT_INT_OFST, MFB_A_SRZ_VERT_INT_OFST, SRZ_VERT_INT_OFST)\
    CMD(a, b, c, d, e, int, MFB_SRZ_A_VERT_SUB_OFST, mfb_srz.srz_luma_vertical_subpixel_offset, MFB_SRZ_Luma_Vertical_Subpixel_Offset,, REG_CMP_EQ(c, LOG_MFB_SRZ_EN, 1), MFB_A_SRZ_VERT_SUB_OFST, SRZ_VERT_SUB_OFST, 0, MFB_SRZ_A_VERT_SUB_OFST, MFB_A_SRZ_VERT_SUB_OFST, SRZ_VERT_SUB_OFST)\
    CMD(a, b, c, d, e, int, MFB_SRZ_A_HORI_STEP, mfb_srz.srz_horizontal_coeff_step, MFB_SRZ_Horizontal_Coeff_Step,, REG_CMP_EQ(c, LOG_MFB_SRZ_EN, 1), MFB_A_SRZ_HORI_STEP, SRZ_HORI_STEP, 4, MFB_SRZ_A_HORI_STEP, MFB_A_SRZ_HORI_STEP, SRZ_HORI_STEP)\
    CMD(a, b, c, d, e, int, MFB_SRZ_A_VERT_STEP, mfb_srz.srz_vertical_coeff_step, MFB_SRZ_Vertical_Coeff_Step,, REG_CMP_EQ(c, LOG_MFB_SRZ_EN, 1), MFB_A_SRZ_VERT_STEP, SRZ_VERT_STEP, 4, MFB_SRZ_A_VERT_STEP, MFB_A_SRZ_VERT_STEP, SRZ_VERT_STEP)\
    /* BLDO */\
    CMD(a, b, c, d, e, int, BLDO_A_STRIDE, bldo.bldo_stride, BLDO_STRIDE,, REG_CMP_EQ(c, LOG_BLDO_EN, 1), MFB_A_MFBO_STRIDE, STRIDE, 4, BLDO_A_STRIDE, MFB_A_MFBO_STRIDE, STRIDE)\
    CMD(a, b, c, d, e, int, BLDO_A_XOFFSET, bldo.bldo_xoffset, BLDO_XOFFSET,, REG_CMP_EQ(c, LOG_BLDO_EN, 1), MFB_A_MFBO_CROP, XOFFSET, 0, BLDO_A_XOFFSET, MFB_A_MFBO_CROP, XOFFSET)\
    CMD(a, b, c, d, e, int, BLDO_A_YOFFSET, bldo.bldo_yoffset, BLDO_YOFFSET,, REG_CMP_EQ(c, LOG_BLDO_EN, 1), MFB_A_MFBO_CROP, YOFFSET, 0, BLDO_A_YOFFSET, MFB_A_MFBO_CROP, YOFFSET)\
    CMD(a, b, c, d, e, int, BLDO_A_XSIZE, bldo.bldo_xsize, BLDO_XSIZE,, REG_CMP_EQ(c, LOG_BLDO_EN, 1), MFB_A_MFBO_XSIZE, XSIZE, 0, BLDO_A_XSIZE, MFB_A_MFBO_XSIZE, XSIZE)\
    CMD(a, b, c, d, e, int, BLDO_A_YSIZE, bldo.bldo_ysize, BLDO_YSIZE,, REG_CMP_EQ(c, LOG_BLDO_EN, 1), MFB_A_MFBO_YSIZE, YSIZE, 0, BLDO_A_YSIZE, MFB_A_MFBO_YSIZE, YSIZE)\
    /* BLDBO */\
    CMD(a, b, c, d, e, int, BLDBO_A_STRIDE, bldbo.bldbo_stride, BLDBO_STRIDE,, REG_CMP_EQ(c, LOG_BLDBO_EN, 1), MFB_A_MFBO_B_STRIDE, STRIDE, 4, BLDBO_A_STRIDE, MFB_A_MFBO_B_STRIDE, STRIDE)\
    /* BLD2O */\
    CMD(a, b, c, d, e, int, BLD2O_A_STRIDE, bld2o.bld2o_stride, BLD2O_STRIDE,, REG_CMP_EQ(c, LOG_BLD2O_EN, 1), MFB_A_MFB2O_STRIDE, STRIDE, 4, BLD2O_A_STRIDE, MFB_A_MFB2O_STRIDE, STRIDE)\
    CMD(a, b, c, d, e, int, BLD2O_A_XOFFSET, bld2o.bld2o_xoffset, BLD2O_XOFFSET,, REG_CMP_EQ(c, LOG_BLD2O_EN, 1), MFB_A_MFB2O_CROP, XOFFSET, 0, BLD2O_A_XOFFSET, MFB_A_MFB2O_CROP, XOFFSET)\
    CMD(a, b, c, d, e, int, BLD2O_A_YOFFSET, bld2o.bld2o_yoffset, BLD2O_YOFFSET,, REG_CMP_EQ(c, LOG_BLD2O_EN, 1), MFB_A_MFB2O_CROP, YOFFSET, 0, BLD2O_A_YOFFSET, MFB_A_MFB2O_CROP, YOFFSET)\
    CMD(a, b, c, d, e, int, BLD2O_A_XSIZE, bld2o.bld2o_xsize, BLD2O_XSIZE,, REG_CMP_EQ(c, LOG_BLD2O_EN, 1), MFB_A_MFB2O_XSIZE, XSIZE, 0, BLD2O_A_XSIZE, MFB_A_MFB2O_XSIZE, XSIZE)\
    CMD(a, b, c, d, e, int, BLD2O_A_YSIZE, bld2o.bld2o_ysize, BLD2O_YSIZE,, REG_CMP_EQ(c, LOG_BLD2O_EN, 1), MFB_A_MFB2O_YSIZE, YSIZE, 0, BLD2O_A_YSIZE, MFB_A_MFB2O_YSIZE, YSIZE)\
    /* CRSPB */\
    CMD(a, b, c, d, e, int, CRSPB_A_cstep_y, crspb.crspb_ystep, CRSPB_STEP_Y,, REG_CMP_EQ(c, LOG_CRSPB_EN, 1), MFB_A_CRSP_STEP_OFST, CRSP_STEP_Y, 0, CRSPB_A_cstep_y, MFB_A_CRSP_STEP_OFST, CRSP_STEP_Y)\
    CMD(a, b, c, d, e, int, CRSPB_A_offset_x, crspb.crspb_xoffset, CRSPB_OFST_X,, REG_CMP_EQ(c, LOG_CRSPB_EN, 1), MFB_A_CRSP_STEP_OFST, CRSP_OFST_X, 0, CRSPB_A_offset_x, MFB_A_CRSP_STEP_OFST, CRSP_OFST_X)\
    CMD(a, b, c, d, e, int, CRSPB_A_offset_y, crspb.crspb_yoffset, CRSPB_OFST_Y,, REG_CMP_EQ(c, LOG_CRSPB_EN, 1), MFB_A_CRSP_STEP_OFST, CRSP_OFST_Y, 0, CRSPB_A_offset_y, MFB_A_CRSP_STEP_OFST, CRSP_OFST_Y)\

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
    CMD(a, b, c, d, e, char *, ptr_tcm_dir_name,, data_path_ptr,,,,,)\

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
    CMD(a, b, c, d, e, int, TDRI_A_BASE_ADDR,, TDRI_A_BASE_ADDR,,,,,)\
    CMD(a, b, c, d, e, int, TDRI_B_BASE_ADDR,, TDRI_B_BASE_ADDR,,,,,)\
    CMD(a, b, c, d, e, int, WPE_TDRI_A_BASE_ADDR,, WPE_TDRI_A_BASE_ADDR,,,,,)\
    CMD(a, b, c, d, e, int, WPE_TDRI_B_BASE_ADDR,, WPE_TDRI_B_BASE_ADDR,,,,,)\
    CMD(a, b, c, d, e, int, EAF_TDRI_A_BASE_ADDR,, EAF_TDRI_A_BASE_ADDR,,,,,)\
    CMD(a, b, c, d, e, int, BLD_TDRI_A_BASE_ADDR,, BLD_TDRI_A_BASE_ADDR,,,,,)\
	/* FCSIM */\
    CMD(a, b, c, d, e, int, FCSIM_DL_FUNC_NUM,, FCSIM_DL_FUNC_NUM,,,,,)\
    CMD(a, b, c, d, e, int, FCSIM_DL_FRAME_WIDTH,, FCSIM_DL_FRAME_WIDTH,,,,,)\
    CMD(a, b, c, d, e, int, FCSIM_DL_FRAME_HEIGHT,, FCSIM_DL_FRAME_HEIGHT,,,,,)\
    CMD(a, b, c, d, e, int, FCSIM_DL_TILE_H_NUM,, FCSIM_DL_TILE_H_NUM,,,,,)\
    CMD(a, b, c, d, e, int, FCSIM_DL_FIRST_TILE_WIDTH,, FCSIM_DL_FIRST_TILE_WIDTH,,,,,)\
    CMD(a, b, c, d, e, int, FCSIM_DL_TILE_WIDTH,, FCSIM_DL_TILE_WIDTH,,,,,)\
    CMD(a, b, c, d, e, int, FCSIM_DL_LAST_TILE_WIDTH,, FCSIM_DL_LAST_TILE_WIDTH,,,,,)\
    CMD(a, b, c, d, e, int, FCSIM_DL_TILE_V_NUM,, FCSIM_DL_TILE_V_NUM,,,,,)\
    CMD(a, b, c, d, e, int, FCSIM_DL_FIRST_TILE_HEIGHT,, FCSIM_DL_FIRST_TILE_HEIGHT,,,,,)\
    CMD(a, b, c, d, e, int, FCSIM_DL_TILE_HEIGHT,, FCSIM_DL_TILE_HEIGHT,,,,,)\
    CMD(a, b, c, d, e, int, FCSIM_DL_LAST_TILE_HEIGHT,, FCSIM_DL_LAST_TILE_HEIGHT,,,,,)\
	/* */\
    CMD(a, b, c, d, e, int, YCNR_A_EN,, YCNR_EN,,,,,)\

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
    CMD(a, b, c, d, e, int,, sw.log_en, platform_log_en,, true,,,)\

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
    CMD(a, b, c, d, e, int,,, platform_buffer_size,,,,,)\
    CMD(a, b, c, d, e, int,,, platform_max_tpipe_no,,,,,)\
    CMD(a, b, c, d, e, int,,, platform_isp_hex_no_per_tpipe,,,,,)\
    CMD(a, b, c, d, e, int,,, platform_isp_hex_no_per_tpipe_wpe,,,,,)\
    CMD(a, b, c, d, e, int,,, platform_isp_hex_no_per_tpipe_wpe2,,,,,)\
    CMD(a, b, c, d, e, int,,, platform_isp_hex_no_per_tpipe_eaf,,,,,)\
    CMD(a, b, c, d, e, int,,, platform_isp_hex_no_per_tpipe_bld,,,,,)\
    CMD(a, b, c, d, e, int,,, platform_error_no,,,,,)\
    /* match id for reg & config */\
    CMD(a, b, c, d, e, unsigned int,,, tpipe_id,,,,,)\

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
    /* IMGI */\
    CMD(a, b, c, d, e, int,,, IMGI_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, IMGI_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, IMGI_TILE_YSIZE,,,,,)\
    /* IMGBI */\
    CMD(a, b, c, d, e, int,,, IMGBI_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, IMGBI_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, IMGBI_TILE_YSIZE,,,,,)\
    /* IMGCI */\
    CMD(a, b, c, d, e, int,,, IMGCI_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, IMGCI_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, IMGCI_TILE_YSIZE,,,,,)\
    /* UFDI */\
    CMD(a, b, c, d, e, int,,, UFDI_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, UFDI_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, UFDI_TILE_YSIZE,,,,,)\
    /* UNP */\
    CMD(a, b, c, d, e, int,,, UNP_OFST_STB,,,,,)\
    CMD(a, b, c, d, e, int,,, UNP_OFST_EDB,,,,,)\
    /* UFD */\
    CMD(a, b, c, d, e, int,,, UFD_WD,,,,,)\
    CMD(a, b, c, d, e, int,,, UFD_HT,,,,,)\
    CMD(a, b, c, d, e, int,,, UFD_X_START,,,,,)\
    CMD(a, b, c, d, e, int,,, UFD_X_END,,,,,)\
    CMD(a, b, c, d, e, int,,, UFD_Y_START,,,,,)\
    CMD(a, b, c, d, e, int,,, UFD_Y_END,,,,,)\
    CMD(a, b, c, d, e, int,,, UFD_AU_SIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, UFD_AU_OFST,,,,,)\
    CMD(a, b, c, d, e, int,,, UFD_AU2_SIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, UFD_AU2_OFST,,,,,)\
    CMD(a, b, c, d, e, int,,, UFD_AU3_SIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, UFD_AU3_OFST,,,,,)\
    CMD(a, b, c, d, e, int,,, UFD_TILE_BOND_MODE,,,,,)\
    CMD(a, b, c, d, e, int,,, UFD_TILE_BOND2_MODE,,,,,)\
    /* RNR */\
    CMD(a, b, c, d, e, int,,, RNR_TILE_EDGE,,,,,)\
    /* UDM */\
    CMD(a, b, c, d, e, int,,, UDM_TILE_EDGE,,,,,)\
	/* CPN2 */\
	CMD(a, b, c, d, e, int,,, CPN2_IN_WD,,,,,)\
	CMD(a, b, c, d, e, int,,, CPN2_IN_HT,,,,,)\
	CMD(a, b, c, d, e, int,,, CPN2_TILE_EDGE,,,,,)\
	/* DCPN2 */\
	CMD(a, b, c, d, e, int,,, DCPN2_IN_WD,,,,,)\
	CMD(a, b, c, d, e, int,,, DCPN2_IN_HT,,,,,)\
    CMD(a, b, c, d, e, int,,, DCPN2_TILE_EDGE,,,,,)\
    /* LSC */\
    CMD(a, b, c, d, e, int,,, LSC2_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, LSC2_TILE_YSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, LSC2_TILE_XOFST,,,,,)\
    CMD(a, b, c, d, e, int,,, LSC2_TILE_YOFST,,,,,)\
    CMD(a, b, c, d, e, int,,, LSC2_XNUM,,,,,)\
    CMD(a, b, c, d, e, int,,, LSC2_YNUM,,,,,)\
    CMD(a, b, c, d, e, int,,, LSC2_LWIDTH,,,,,)\
    CMD(a, b, c, d, e, int,,, LSC2_LHEIGHT,,,,,)\
    /* SL2 */\
    CMD(a, b, c, d, e, int,,, SL2_IN_TILE_XOFF,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2_IN_TILE_YOFF,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2_IN_TILE_WIDTH,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2_IN_TILE_HEIGHT,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2_OUT_TILE_XOFF,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2_OUT_TILE_YOFF,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2_OUT_TILE_WIDTH,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2_OUT_TILE_HEIGHT,,,,,)\
	/* DMGI */\
    CMD(a, b, c, d, e, int,,, DMGI_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, DMGI_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, DMGI_TILE_YSIZE,,,,,)\
	/* DEPI */\
    CMD(a, b, c, d, e, int,,, DEPI_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, DEPI_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, DEPI_TILE_YSIZE,,,,,)\
    /* VIPI */\
    CMD(a, b, c, d, e, int,,, VIPI_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, VIPI_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, VIPI_TILE_YSIZE,,,,,)\
    /* VIP2I */\
    CMD(a, b, c, d, e, int,,, VIP2I_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, VIP2I_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, VIP2I_TILE_YSIZE,,,,,)\
    /* VIP3I */\
    CMD(a, b, c, d, e, int,,, VIP3I_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, VIP3I_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, VIP3I_TILE_YSIZE,,,,,)\
    /* PAK2O */\
    CMD(a, b, c, d, e, int,,, PAK2O_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, PAK2O_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, PAK2O_TILE_YSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, PAK2O_TILE_XOFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, PAK2O_TILE_YOFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, PAK2O_TILE_FULL_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, PAK2O_TILE_FULL_YSIZE,,,,,)\
    /* G2C */\
    CMD(a, b, c, d, e, int,,, G2C_TILE_SHADE_XMID,,,,,)\
    CMD(a, b, c, d, e, int,,, G2C_TILE_SHADE_YMID,,,,,)\
    CMD(a, b, c, d, e, int,,, G2C_TILE_SHADE_XSP,,,,,)\
    CMD(a, b, c, d, e, int,,, G2C_TILE_SHADE_YSP,,,,,)\
    CMD(a, b, c, d, e, int,,, G2C_TILE_SHADE_VAR,,,,,)\
    /* SL2B */\
    CMD(a, b, c, d, e, int,,, SL2B_OUT_TILE_XOFF,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2B_OUT_TILE_YOFF,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2B_OUT_TILE_WIDTH,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2B_OUT_TILE_HEIGHT,,,,,)\
	/* NDG */\
    CMD(a, b, c, d, e, int,,, NDG_TILE_TOP_LOSS,,,,,)\
    CMD(a, b, c, d, e, int,,, NDG_TILE_BOTTOM_LOSS,,,,,)\
    CMD(a, b, c, d, e, int,,, NDG_TILE_LEFT_LOSS,,,,,)\
    CMD(a, b, c, d, e, int,,, NDG_TILE_RIGHT_LOSS,,,,,)\
	/* NDG2 */\
    CMD(a, b, c, d, e, int,,, NDG2_TILE_TOP_LOSS,,,,,)\
    CMD(a, b, c, d, e, int,,, NDG2_TILE_BOTTOM_LOSS,,,,,)\
    CMD(a, b, c, d, e, int,,, NDG2_TILE_LEFT_LOSS,,,,,)\
    CMD(a, b, c, d, e, int,,, NDG2_TILE_RIGHT_LOSS,,,,,)\
    /* SL2C */\
    CMD(a, b, c, d, e, int,,, SL2C_IN_TILE_XOFF,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2C_IN_TILE_YOFF,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2C_IN_TILE_WIDTH,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2C_IN_TILE_HEIGHT,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2C_OUT_TILE_XOFF,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2C_OUT_TILE_YOFF,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2C_OUT_TILE_WIDTH,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2C_OUT_TILE_HEIGHT,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2C_TILE_TOP_LOSS,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2C_TILE_BOTTOM_LOSS,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2C_TILE_LEFT_LOSS,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2C_TILE_RIGHT_LOSS,,,,,)\
    /* SL2D */\
    CMD(a, b, c, d, e, int,,, SL2D_IN_TILE_XOFF,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2D_IN_TILE_YOFF,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2D_IN_TILE_WIDTH,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2D_IN_TILE_HEIGHT,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2D_OUT_TILE_XOFF,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2D_OUT_TILE_YOFF,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2D_OUT_TILE_WIDTH,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2D_OUT_TILE_HEIGHT,,,,,)\
    /* SL2E */\
    CMD(a, b, c, d, e, int,,, SL2E_IN_TILE_XOFF,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2E_IN_TILE_YOFF,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2E_IN_TILE_WIDTH,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2E_IN_TILE_HEIGHT,,,,,)\
    /* SL2G */\
    CMD(a, b, c, d, e, int,,, SL2G_IN_TILE_XOFF,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2G_IN_TILE_YOFF,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2G_IN_TILE_WIDTH,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2G_IN_TILE_HEIGHT,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2G_OUT_TILE_XOFF,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2G_OUT_TILE_YOFF,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2G_OUT_TILE_WIDTH,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2G_OUT_TILE_HEIGHT,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2G_TILE_TOP_LOSS,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2G_TILE_BOTTOM_LOSS,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2G_TILE_LEFT_LOSS,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2G_TILE_RIGHT_LOSS,,,,,)\
    /* SL2H */\
    CMD(a, b, c, d, e, int,,, SL2H_IN_TILE_XOFF,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2H_IN_TILE_YOFF,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2H_IN_TILE_WIDTH,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2H_IN_TILE_HEIGHT,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2H_OUT_TILE_XOFF,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2H_OUT_TILE_YOFF,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2H_OUT_TILE_WIDTH,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2H_OUT_TILE_HEIGHT,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2H_TILE_TOP_LOSS,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2H_TILE_BOTTOM_LOSS,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2H_TILE_LEFT_LOSS,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2H_TILE_RIGHT_LOSS,,,,,)\
    /* SL2I */\
    CMD(a, b, c, d, e, int,,, SL2I_IN_TILE_XOFF,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2I_IN_TILE_YOFF,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2I_IN_TILE_WIDTH,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2I_IN_TILE_HEIGHT,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2I_OUT_TILE_XOFF,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2I_OUT_TILE_YOFF,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2I_OUT_TILE_WIDTH,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2I_OUT_TILE_HEIGHT,,,,,)\
    /* HFG */\
    CMD(a, b, c, d, e, int,,, HFG_TILE_XOFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, HFG_TILE_YOFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, HFG_TILE_WD,,,,,)\
    CMD(a, b, c, d, e, int,,, HFG_TILE_HT,,,,,)\
    /* NDG */\
    CMD(a, b, c, d, e, int,,, NDG_TILE_XOFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, NDG_TILE_YOFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, NDG_TILE_WD,,,,,)\
    CMD(a, b, c, d, e, int,,, NDG_TILE_HT,,,,,)\
    /* NDG2 */\
    CMD(a, b, c, d, e, int,,, NDG2_TILE_XOFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, NDG2_TILE_YOFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, NDG2_TILE_WD,,,,,)\
    CMD(a, b, c, d, e, int,,, NDG2_TILE_HT,,,,,)\
    /* BNR */\
    CMD(a, b, c, d, e, int,,, BPC_TILE_XOFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, BPC_TILE_YOFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, BPC_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, BPC_TILE_YSIZE,,,,,)\
    /* LCEI */\
    CMD(a, b, c, d, e, int,,, LCEI_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, LCEI_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, LCEI_TILE_YSIZE,,,,,)\
    /* LCE */\
    CMD(a, b, c, d, e, int,,, LCE_TILE_OFFSET_X,,,,,)\
    CMD(a, b, c, d, e, int,,, LCE_TILE_OFFSET_Y,,,,,)\
    CMD(a, b, c, d, e, int,,, LCE_TILE_BIAS_X,,,,,)\
    CMD(a, b, c, d, e, int,,, LCE_TILE_BIAS_Y,,,,,)\
    CMD(a, b, c, d, e, int,,, lce_xsize,,,,,)\
    CMD(a, b, c, d, e, int,,, lce_ysize,,,,,)\
    CMD(a, b, c, d, e, int,,, lce_output_xsize,,,,,)\
    CMD(a, b, c, d, e, int,,, lce_output_ysize,,,,,)\
    /* CRZ */\
    CMD(a, b, c, d, e, int,,, CDRZ_Tile_Input_Image_W,,,,,)\
    CMD(a, b, c, d, e, int,,, CDRZ_Tile_Input_Image_H,,,,,)\
    CMD(a, b, c, d, e, int,,, CDRZ_Tile_Output_Image_W,,,,,)\
    CMD(a, b, c, d, e, int,,, CDRZ_Tile_Output_Image_H,,,,,)\
    CMD(a, b, c, d, e, int,,, CDRZ_Tile_Luma_Horizontal_Integer_Offset,,,,,)\
    CMD(a, b, c, d, e, int,,, CDRZ_Tile_Luma_Vertical_Integer_Offset,,,,,)\
    CMD(a, b, c, d, e, int,,, CDRZ_Tile_Chroma_Horizontal_Integer_Offset,,,,,)\
    CMD(a, b, c, d, e, int,,, CDRZ_Tile_Chroma_Vertical_Integer_Offset,,,,,)\
    CMD(a, b, c, d, e, int,,, CDRZ_Tile_Luma_Horizontal_Subpixel_Offset,,,,,)\
    CMD(a, b, c, d, e, int,,, CDRZ_Tile_Luma_Vertical_Subpixel_Offset,,,,,)\
    CMD(a, b, c, d, e, int,,, CDRZ_Tile_Chroma_Horizontal_Subpixel_Offset,,,,,)\
    CMD(a, b, c, d, e, int,,, CDRZ_Tile_Chroma_Vertical_Subpixel_Offset,,,,,)\
    /* IMG2O */\
    CMD(a, b, c, d, e, int,,, IMG2O_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG2O_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG2O_TILE_YSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG2O_TILE_XOFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG2O_TILE_YOFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG2O_TILE_FULL_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG2O_TILE_FULL_YSIZE,,,,,)\
    /* IMG2BO */\
    CMD(a, b, c, d, e, int,,, IMG2BO_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG2BO_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG2BO_TILE_YSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG2BO_TILE_XOFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG2BO_TILE_YOFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG2BO_TILE_FULL_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG2BO_TILE_FULL_YSIZE,,,,,)\
	/* SEEE */\
    CMD(a, b, c, d, e, int,,, SEEE_TILE_EDGE,,,,,)\
    /* NR3D */\
    CMD(a, b, c, d, e, int,,, NR3D_TILE_FBCNT_XOFF,,,,,)\
    CMD(a, b, c, d, e, int,,, NR3D_TILE_FBCNT_YOFF,,,,,)\
    CMD(a, b, c, d, e, int,,, NR3D_TILE_FBCNT_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, NR3D_TILE_FBCNT_YSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, NR3D_TILE_ON_OFST_X,,,,,)\
    CMD(a, b, c, d, e, int,,, NR3D_TILE_ON_OFST_Y,,,,,)\
    CMD(a, b, c, d, e, int,,, NR3D_TILE_ON_WD,,,,,)\
    CMD(a, b, c, d, e, int,,, NR3D_TILE_ON_HT,,,,,)\
    CMD(a, b, c, d, e, int,,, NR3D_TILE_WD,,,,,)\
    CMD(a, b, c, d, e, int,,, NR3D_TILE_HT,,,,,)\
    CMD(a, b, c, d, e, int,,, NR3D_TILE_EDGE,,,,,)\
	/* COLOR */\
    CMD(a, b, c, d, e, int,,, COLOR_TILE_EDGE,,,,,)\
	/* CRSP */\
    CMD(a, b, c, d, e, int,,, CRSP_TILE_WD,,,,,)\
    CMD(a, b, c, d, e, int,,, CRSP_TILE_OFST_X,,,,,)\
    CMD(a, b, c, d, e, int,,, CRSP_TILE_HT,,,,,)\
    CMD(a, b, c, d, e, int,,, CRSP_TILE_OFST_Y,,,,,)\
    CMD(a, b, c, d, e, int,,, CRSP_TILE_CROP_XSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, CRSP_TILE_CROP_XEND,,,,,)\
    CMD(a, b, c, d, e, int,,, CRSP_TILE_CROP_YSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, CRSP_TILE_CROP_YEND,,,,,)\
	/* CRSPB */\
    CMD(a, b, c, d, e, int,,, CRSPB_TILE_WD,,,,,)\
    CMD(a, b, c, d, e, int,,, CRSPB_TILE_OFST_X,,,,,)\
    CMD(a, b, c, d, e, int,,, CRSPB_TILE_HT,,,,,)\
    CMD(a, b, c, d, e, int,,, CRSPB_TILE_OFST_Y,,,,,)\
    CMD(a, b, c, d, e, int,,, CRSPB_TILE_CROP_XSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, CRSPB_TILE_CROP_XEND,,,,,)\
    CMD(a, b, c, d, e, int,,, CRSPB_TILE_CROP_YSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, CRSPB_TILE_CROP_YEND,,,,,)\
    /* IMG3O */\
    CMD(a, b, c, d, e, int,,, IMG3O_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG3O_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG3O_TILE_YSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG3O_TILE_XOFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG3O_TILE_YOFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG3O_TILE_FULL_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG3O_TILE_FULL_YSIZE,,,,,)\
    /* IMG3BO */\
    CMD(a, b, c, d, e, int,,, IMG3BO_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG3BO_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG3BO_TILE_YSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG3BO_TILE_XOFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG3BO_TILE_YOFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG3BO_TILE_FULL_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG3BO_TILE_FULL_YSIZE,,,,,)\
    /* IMG3CO */\
    CMD(a, b, c, d, e, int,,, IMG3CO_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG3CO_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG3CO_TILE_YSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG3CO_TILE_XOFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG3CO_TILE_YOFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG3CO_TILE_FULL_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, IMG3CO_TILE_FULL_YSIZE,,,,,)\
    /* SRZ1 */\
    CMD(a, b, c, d, e, int,,, SRZ1_Tile_Input_Image_W,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ1_Tile_Input_Image_H,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ1_Tile_Output_Image_W,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ1_Tile_Output_Image_H,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ1_Tile_Luma_Horizontal_Integer_Offset,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ1_Tile_Luma_Horizontal_Subpixel_Offset,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ1_Tile_Luma_Vertical_Integer_Offset,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ1_Tile_Luma_Vertical_Subpixel_Offset,,,,,)\
    /* SRZ2 */\
    CMD(a, b, c, d, e, int,,, SRZ2_Tile_Input_Image_W,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ2_Tile_Input_Image_H,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ2_Tile_Output_Image_W,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ2_Tile_Output_Image_H,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ2_Tile_Luma_Horizontal_Integer_Offset,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ2_Tile_Luma_Horizontal_Subpixel_Offset,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ2_Tile_Luma_Vertical_Integer_Offset,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ2_Tile_Luma_Vertical_Subpixel_Offset,,,,,)\
    /* SRZ3 */\
    CMD(a, b, c, d, e, int,,, SRZ3_Tile_Input_Image_W,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ3_Tile_Input_Image_H,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ3_Tile_Output_Image_W,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ3_Tile_Output_Image_H,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ3_Tile_Luma_Horizontal_Integer_Offset,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ3_Tile_Luma_Horizontal_Subpixel_Offset,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ3_Tile_Luma_Vertical_Integer_Offset,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ3_Tile_Luma_Vertical_Subpixel_Offset,,,,,)\
    /* SRZ4 */\
    CMD(a, b, c, d, e, int,,, SRZ4_Tile_Input_Image_W,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ4_Tile_Input_Image_H,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ4_Tile_Output_Image_W,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ4_Tile_Output_Image_H,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ4_Tile_Luma_Horizontal_Integer_Offset,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ4_Tile_Luma_Horizontal_Subpixel_Offset,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ4_Tile_Luma_Vertical_Integer_Offset,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ4_Tile_Luma_Vertical_Subpixel_Offset,,,,,)\
    /* FE */\
    CMD(a, b, c, d, e, int,,, FE_TILE_EDGE,,,,,)\
    CMD(a, b, c, d, e, int,,, FE_TILE_XIDX,,,,,)\
    CMD(a, b, c, d, e, int,,, FE_TILE_YIDX,,,,,)\
    CMD(a, b, c, d, e, int,,, FE_TILE_START_X,,,,,)\
    CMD(a, b, c, d, e, int,,, FE_TILE_START_Y,,,,,)\
    CMD(a, b, c, d, e, int,,, FE_TILE_IN_WIDTH,,,,,)\
    CMD(a, b, c, d, e, int,,, FE_TILE_IN_HEIGHT,,,,,)\
    /* FEO */\
    CMD(a, b, c, d, e, int,,, FEO_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, FEO_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, FEO_TILE_YSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, FEO_TILE_FULL_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, FEO_TILE_FULL_YSIZE,,,,,)\
    /* C02 */\
    CMD(a, b, c, d, e, int,,, C02_TILE_EDGE,,,,,)\
    CMD(a, b, c, d, e, int,,, C02_TILE_CROP_XSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, C02_TILE_CROP_XEND,,,,,)\
    CMD(a, b, c, d, e, int,,, C02_TILE_CROP_YSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, C02_TILE_CROP_YEND,,,,,)\
    /* C02B */\
    CMD(a, b, c, d, e, int,,, C02B_TILE_EDGE,,,,,)\
    CMD(a, b, c, d, e, int,,, C02B_TILE_CROP_XSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, C02B_TILE_CROP_XEND,,,,,)\
    CMD(a, b, c, d, e, int,,, C02B_TILE_CROP_YSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, C02B_TILE_CROP_YEND,,,,,)\
    /* C02C */\
    CMD(a, b, c, d, e, int,,, C02C_TILE_EDGE,,,,,)\
    CMD(a, b, c, d, e, int,,, C02C_TILE_CROP_XSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, C02C_TILE_CROP_XEND,,,,,)\
    CMD(a, b, c, d, e, int,,, C02C_TILE_CROP_YSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, C02C_TILE_CROP_YEND,,,,,)\
    /* C02D */\
    CMD(a, b, c, d, e, int,,, C02D_TILE_EDGE,,,,,)\
    CMD(a, b, c, d, e, int,,, C02D_TILE_CROP_XSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, C02D_TILE_CROP_XEND,,,,,)\
    CMD(a, b, c, d, e, int,,, C02D_TILE_CROP_YSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, C02D_TILE_CROP_YEND,,,,,)\
	/* C24 */\
    CMD(a, b, c, d, e, int,,, C24_TILE_EDGE,,,,,)\
	/* C24B */\
    CMD(a, b, c, d, e, int,,, C24B_TILE_EDGE,,,,,)\
	/* C42 */\
    CMD(a, b, c, d, e, int,,, C42_TILE_EDGE,,,,,)\
	/* PCA */\
    CMD(a, b, c, d, e, int,,, PCA_TILE_EDGE,,,,,)\
	/* NBC */\
    CMD(a, b, c, d, e, int,,, NBC_TILE_EDGE,,,,,)\
    CMD(a, b, c, d, e, int,,, NBC_LTM_TOP_LOSS,,,,,)\
    CMD(a, b, c, d, e, int,,, NBC_LTM_BOTTOM_LOSS,,,,,)\
    CMD(a, b, c, d, e, int,,, NBC_LTM_LEFT_LOSS,,,,,)\
    CMD(a, b, c, d, e, int,,, NBC_LTM_RIGHT_LOSS,,,,,)\
	/* NBC2 */\
    CMD(a, b, c, d, e, int,,, NBC2_TILE_BOK_XOFF,,,,,)\
    CMD(a, b, c, d, e, int,,, NBC2_TILE_BOK_YOFF,,,,,)\
    CMD(a, b, c, d, e, int,,, NBC2_TILE_EDGE,,,,,)\
	/* LCE */\
    CMD(a, b, c, d, e, int,,, LCE_TILE_EDGE,,,,,)\
	/* ADBS2 */\
    CMD(a, b, c, d, e, int,,, ADBS2_TILE_EDGE,,,,,)\
    CMD(a, b, c, d, e, int,,, ADBS2_TILE_LE_INV_CTL,,,,,)\
	/* DBS2 */\
    CMD(a, b, c, d, e, int,,, DBS2_TILE_EDGE,,,,,)\
    CMD(a, b, c, d, e, int,,, DBS2_TILE_LE_INV_CTL,,,,,)\
	/* RMG2 */\
    CMD(a, b, c, d, e, int,,, RMG2_TILE_LE_INV_CTL,,,,,)\
	/* BNR2 */\
    CMD(a, b, c, d, e, int,,, BNR2_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, BNR2_TILE_YSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, BNR2_TILE_XOFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, BNR2_TILE_YOFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, BNR2_TILE_EDGE,,,,,)\
    CMD(a, b, c, d, e, int,,, BNR2_TILE_LE_INV_CTL,,,,,)\
	/* RMM2 */\
    CMD(a, b, c, d, e, int,,, RMM2_TILE_EDGE,,,,,)\
    CMD(a, b, c, d, e, int,,, RMM2_TILE_LE_INV_CTL,,,,,)\
    /* SMXI1 */\
    CMD(a, b, c, d, e, int,,, SMX1I_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX1I_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX1I_TILE_YSIZE,,,,,)\
    /* SMX1O */\
    CMD(a, b, c, d, e, int,,, SMX1O_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX1O_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX1O_TILE_YSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX1O_TILE_XOFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX1O_TILE_YOFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX1O_TILE_FULL_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX1O_TILE_FULL_YSIZE,,,,,)\
    /* SMXI2 */\
    CMD(a, b, c, d, e, int,,, SMX2I_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX2I_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX2I_TILE_YSIZE,,,,,)\
    /* SMX2O */\
    CMD(a, b, c, d, e, int,,, SMX2O_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX2O_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX2O_TILE_YSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX2O_TILE_XOFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX2O_TILE_YOFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX2O_TILE_FULL_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX2O_TILE_FULL_YSIZE,,,,,)\
    /* SMXI3 */\
    CMD(a, b, c, d, e, int,,, SMX3I_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX3I_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX3I_TILE_YSIZE,,,,,)\
    /* SMX3O */\
    CMD(a, b, c, d, e, int,,, SMX3O_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX3O_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX3O_TILE_YSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX3O_TILE_XOFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX3O_TILE_YOFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX3O_TILE_FULL_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX3O_TILE_FULL_YSIZE,,,,,)\
    /* SMXI4 */\
    CMD(a, b, c, d, e, int,,, SMX4I_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX4I_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX4I_TILE_YSIZE,,,,,)\
    /* SMX4O */\
    CMD(a, b, c, d, e, int,,, SMX4O_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX4O_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX4O_TILE_YSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX4O_TILE_XOFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX4O_TILE_YOFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX4O_TILE_FULL_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX4O_TILE_FULL_YSIZE,,,,,)\
    /* SRZ6 */\
    CMD(a, b, c, d, e, int,,, SRZ6_Tile_Input_Image_W,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ6_Tile_Input_Image_H,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ6_Tile_Output_Image_W,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ6_Tile_Output_Image_H,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ6_Tile_Luma_Horizontal_Integer_Offset,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ6_Tile_Luma_Horizontal_Subpixel_Offset,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ6_Tile_Luma_Vertical_Integer_Offset,,,,,)\
    CMD(a, b, c, d, e, int,,, SRZ6_Tile_Luma_Vertical_Subpixel_Offset,,,,,)\
    /* EAF */\
    CMD(a, b, c, d, e, int,,, EAF_TILE_EDGE,,,,,)\
	CMD(a, b, c, d, e, int,,, EAFI_MASK_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, EAFI_MASK_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, EAFI_MASK_TILE_YSIZE,,,,,)\
	CMD(a, b, c, d, e, int,,, EAFI_CUR_Y_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, EAFI_CUR_Y_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, EAFI_CUR_Y_TILE_YSIZE,,,,,)\
	CMD(a, b, c, d, e, int,,, EAFI_CUR_UV_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, EAFI_CUR_UV_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, EAFI_CUR_UV_TILE_YSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, EAFI_CUR_TILE_LEFT_LOSS,,,,,)\
    CMD(a, b, c, d, e, int,,, EAFI_CUR_TILE_RIGHT_LOSS,,,,,)\
    CMD(a, b, c, d, e, int,,, EAFI_CUR_TILE_TOP_LOSS,,,,,)\
    CMD(a, b, c, d, e, int,,, EAFI_CUR_TILE_BOTTOM_LOSS,,,,,)\
	CMD(a, b, c, d, e, int,,, EAFI_PRE_Y_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, EAFI_PRE_Y_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, EAFI_PRE_Y_TILE_YSIZE,,,,,)\
	CMD(a, b, c, d, e, int,,, EAFI_PRE_UV_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, EAFI_PRE_UV_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, EAFI_PRE_UV_TILE_YSIZE,,,,,)\
	CMD(a, b, c, d, e, int,,, EAFI_LKH_WMAP_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, EAFI_LKH_WMAP_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, EAFI_LKH_WMAP_TILE_YSIZE,,,,,)\
	CMD(a, b, c, d, e, int,,, EAFI_LKH_EMAP_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, EAFI_LKH_EMAP_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, EAFI_LKH_EMAP_TILE_YSIZE,,,,,)\
	CMD(a, b, c, d, e, int,,, EAFI_DEPTH_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, EAFI_DEPTH_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, EAFI_DEPTH_TILE_YSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, EAFO_FOUT_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, EAFO_FOUT_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, EAFO_FOUT_TILE_YSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, EAFO_FOUT_TILE_XOFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, EAFO_FOUT_TILE_YOFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, EAFO_FOUT_TILE_FULL_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, EAFO_FOUT_TILE_FULL_YSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, EAF_TILE_IN_WD,,,,,)\
    CMD(a, b, c, d, e, int,,, EAF_TILE_IN_HT,,,,,)\
    CMD(a, b, c, d, e, int,,, EAF_UV_TILE_IN_HT,,,,,)\
    /* VECI */\
    CMD(a, b, c, d, e, int,,, VECI_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, VECI_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, VECI_TILE_YSIZE,,,,,)\
    /* VEC2I */\
    CMD(a, b, c, d, e, int,,, VEC2I_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, VEC2I_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, VEC2I_TILE_YSIZE,,,,,)\
    /* VEC3I */\
    CMD(a, b, c, d, e, int,,, VEC3I_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, VEC3I_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, VEC3I_TILE_YSIZE,,,,,)\
    /* WPEO */\
    CMD(a, b, c, d, e, int,,, WPEO_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, WPEO_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, WPEO_TILE_YSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, WPEO_TILE_XOFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, WPEO_TILE_YOFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, WPEO_TILE_FULL_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, WPEO_TILE_FULL_YSIZE,,,,,)\
    /* MSKO */\
    CMD(a, b, c, d, e, int,,, MSKO_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, MSKO_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, MSKO_TILE_YSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, MSKO_TILE_XOFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, MSKO_TILE_YOFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, MSKO_TILE_FULL_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, MSKO_TILE_FULL_YSIZE,,,,,)\
    /* BLDI */\
    CMD(a, b, c, d, e, int,,, BLDI_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, BLDI_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, BLDI_TILE_YSIZE,,,,,)\
    /* BLDBI */\
    CMD(a, b, c, d, e, int,,, BLDBI_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, BLDBI_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, BLDBI_TILE_YSIZE,,,,,)\
    /* BLD2I */\
    CMD(a, b, c, d, e, int,,, BLD2I_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, BLD2I_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, BLD2I_TILE_YSIZE,,,,,)\
    /* BLD2BI */\
    CMD(a, b, c, d, e, int,,, BLD2BI_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, BLD2BI_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, BLD2BI_TILE_YSIZE,,,,,)\
    /* BLD3I */\
    CMD(a, b, c, d, e, int,,, BLD3I_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, BLD3I_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, BLD3I_TILE_YSIZE,,,,,)\
    /* BLD4I */\
    CMD(a, b, c, d, e, int,,, BLD4I_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, BLD4I_TILE_XSIZE,,,,,)\
	CMD(a, b, c, d, e, int,,, BLD4I_TILE_YSIZE,,,,,)\
    /* BLD */\
    CMD(a, b, c, d, e, int,,, BLD_TILE_XDIST,,,,,)\
    CMD(a, b, c, d, e, int,,, BLD_TILE_YDIST,,,,,)\
    CMD(a, b, c, d, e, int,,, BLD_TILE_OFFSET_X,,,,,)\
    CMD(a, b, c, d, e, int,,, BLD_TILE_OUTPUT_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, BLD_TILE_OUTPUT_YSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, BLD_TILE_EDGE,,,,,)\
    CMD(a, b, c, d, e, int,,, BLD_TILE_TOP_LOSS,,,,,)\
    CMD(a, b, c, d, e, int,,, BLD_TILE_BOTTOM_LOSS,,,,,)\
    CMD(a, b, c, d, e, int,,, BLD_TILE_LEFT_LOSS,,,,,)\
    CMD(a, b, c, d, e, int,,, BLD_TILE_RIGHT_LOSS,,,,,)\
    CMD(a, b, c, d, e, int,,, BLD_CONF_TOP_LOSS,,,,,)\
    CMD(a, b, c, d, e, int,,, BLD_CONF_BOTTOM_LOSS,,,,,)\
    CMD(a, b, c, d, e, int,,, BLD_CONF_LEFT_LOSS,,,,,)\
    CMD(a, b, c, d, e, int,,, BLD_CONF_RIGHT_LOSS,,,,,)\
    CMD(a, b, c, d, e, int,,, BLD_WT_TOP_LOSS,,,,,)\
    CMD(a, b, c, d, e, int,,, BLD_WT_BOTTOM_LOSS,,,,,)\
    CMD(a, b, c, d, e, int,,, BLD_WT_LEFT_LOSS,,,,,)\
    CMD(a, b, c, d, e, int,,, BLD_WT_RIGHT_LOSS,,,,,)\
    /* MFB_SRZ */\
    CMD(a, b, c, d, e, int,,, MFB_SRZ_Tile_Input_Image_W,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_SRZ_Tile_Input_Image_H,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_SRZ_Tile_Output_Image_W,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_SRZ_Tile_Output_Image_H,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_SRZ_Tile_Luma_Horizontal_Integer_Offset,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_SRZ_Tile_Luma_Horizontal_Subpixel_Offset,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_SRZ_Tile_Luma_Vertical_Integer_Offset,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_SRZ_Tile_Luma_Vertical_Subpixel_Offset,,,,,)\
    /* BLDO */\
    CMD(a, b, c, d, e, int,,, BLDO_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, BLDO_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, BLDO_TILE_YSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, BLDO_TILE_XOFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, BLDO_TILE_YOFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, BLDO_TILE_FULL_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, BLDO_TILE_FULL_YSIZE,,,,,)\
    /* BLDBO */\
    CMD(a, b, c, d, e, int,,, BLDBO_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, BLDBO_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, BLDBO_TILE_YSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, BLDBO_TILE_XOFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, BLDBO_TILE_YOFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, BLDBO_TILE_FULL_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, BLDBO_TILE_FULL_YSIZE,,,,,)\
    /* BLD2O */\
    CMD(a, b, c, d, e, int,,, BLD2O_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, BLD2O_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, BLD2O_TILE_YSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, BLD2O_TILE_XOFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, BLD2O_TILE_YOFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, BLD2O_TILE_FULL_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, BLD2O_TILE_FULL_YSIZE,,,,,)\
    /* VGEN */\
    CMD(a, b, c, d, e, int,,, VGEN_Tile_Input_Image_W,,,,,)\
    CMD(a, b, c, d, e, int,,, VGEN_Tile_Input_Image_H,,,,,)\
    CMD(a, b, c, d, e, int,,, VGEN_Tile_Output_Image_W,,,,,)\
    CMD(a, b, c, d, e, int,,, VGEN_Tile_Output_Image_H,,,,,)\
    CMD(a, b, c, d, e, int,,, VGEN_Tile_Luma_Horizontal_Integer_Offset,,,,,)\
    CMD(a, b, c, d, e, int,,, VGEN_Tile_Luma_Horizontal_Subpixel_Offset,,,,,)\
    CMD(a, b, c, d, e, int,,, VGEN_Tile_Luma_Vertical_Integer_Offset,,,,,)\
    CMD(a, b, c, d, e, int,,, VGEN_Tile_Luma_Vertical_Subpixel_Offset,,,,,)\
	/* WPE_C24 */\
    CMD(a, b, c, d, e, int,,, WPE_C24_TILE_EDGE,,,,,)\
	/* ADL2 */\
    CMD(a, b, c, d, e, int,,, ADL_TILE_EDGE,,,,,)\
    /* Internal */\
    CMD(a, b, c, d, e, int,,, CTRL_VIPI_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_VIP2I_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_VIP3I_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_FEO_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_IMG3O_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_IMG3BO_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_IMG3CO_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_IMG2O_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_IMG2BO_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_FE_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_CDRZ_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_MDP_CROP_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_MDP_CROP2_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_WSYNC_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_WSHIFT_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_NBC2_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SRZ1_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SL2C_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SL2E_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SL2G_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SL2H_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SL2I_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_HFG_EN,,,,,)\
	CMD(a, b, c, d, e, int,,, CTRL_NDG_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_NDG2_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_NR3D_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_COLOR_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_YCNR_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_CRSP_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_C24B_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_C02_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_PAK2O_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_DEPI_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_DMGI_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_G2G_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_G2G2_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_FLC_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_FLC2_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_LCE_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_LCEI_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_GGM_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_GGM2_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_C24_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_G2C_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_C42_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_NBC_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_PCA_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SEEE_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SL2B_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SL2D_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SRZ2_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_IMGBI_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_IMGCI_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_PLNR1_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_PLNW1_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_PLNW2_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SRZ3_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SRZ4_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_RCP2_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_PAK2_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_PAKG2_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_PGN_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SL2_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_LSC2_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_CPN2_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_DCPN2_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_RNR_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_UDM_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_MIX1_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_MIX2_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_MIX3_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_MIX4_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_UFDI_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_GDR2_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_ADL_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_UNP_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMX1I_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMX1O_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMX1_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMX1_TRD_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMX1_TRU_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMX1_CRPINL_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMX1_CRPINR_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMX1_CRPOUT_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMX2I_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMX2O_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMX2_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMX2_TRD_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMX2_TRU_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMX2_CRPINL_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMX2_CRPINR_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMX2_CRPOUT_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMX3I_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMX3O_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMX3_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMX3_TRD_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMX3_TRU_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMX3_CRPINL_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMX3_CRPINR_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMX3_CRPOUT_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMX4I_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMX4O_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMX4_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMX4_TRD_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMX4_TRU_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMX4_CRPINL_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMX4_CRPINR_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SMX4_CRPOUT_EN,,,,,)\
	/* SMX */\
    CMD(a, b, c, d, e, int,,, SMX1_CRPINL_XSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX1_CRPINL_XEND,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX1_CRPINL_YSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX1_CRPINL_YEND,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX1_CRPINR_XSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX1_CRPINR_XEND,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX1_CRPINR_YSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX1_CRPINR_YEND,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX1_CRPOUT_XSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX1_CRPOUT_XEND,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX1_CRPOUT_YSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX1_CRPOUT_YEND,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX1_LEFT_DISABLE,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX1_RIGHT_DISABLE,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX1_TRU_IN_HT,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX1_TRU_IN_WD,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX1O_TDR_OFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX2_CRPINL_XSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX2_CRPINL_XEND,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX2_CRPINL_YSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX2_CRPINL_YEND,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX2_CRPINR_XSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX2_CRPINR_XEND,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX2_CRPINR_YSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX2_CRPINR_YEND,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX2_CRPOUT_XSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX2_CRPOUT_XEND,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX2_CRPOUT_YSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX2_CRPOUT_YEND,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX2_LEFT_DISABLE,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX2_RIGHT_DISABLE,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX2_TRU_IN_HT,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX2_TRU_IN_WD,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX2O_TDR_OFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX3_CRPINL_XSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX3_CRPINL_XEND,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX3_CRPINL_YSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX3_CRPINL_YEND,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX3_CRPINR_XSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX3_CRPINR_XEND,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX3_CRPINR_YSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX3_CRPINR_YEND,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX3_CRPOUT_XSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX3_CRPOUT_XEND,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX3_CRPOUT_YSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX3_CRPOUT_YEND,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX3_LEFT_DISABLE,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX3_RIGHT_DISABLE,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX3_TRU_IN_HT,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX3_TRU_IN_WD,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX3O_TDR_OFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX4_CRPINL_XSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX4_CRPINL_XEND,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX4_CRPINL_YSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX4_CRPINL_YEND,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX4_CRPINR_XSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX4_CRPINR_XEND,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX4_CRPINR_YSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX4_CRPINR_YEND,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX4_CRPOUT_XSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX4_CRPOUT_XEND,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX4_CRPOUT_YSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX4_CRPOUT_YEND,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX4_LEFT_DISABLE,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX4_RIGHT_DISABLE,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX4_TRU_IN_HT,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX4_TRU_IN_WD,,,,,)\
    CMD(a, b, c, d, e, int,,, SMX4O_TDR_OFFSET,,,,,)\
    /* MDP_CROP_EN */\
    CMD(a, b, c, d, e, int,,, CTRL_MDP_XSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_MDP_XEND,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_MDP_YSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_MDP_YEND,,,,,)\
    /* MDP_CROP2_EN */\
    CMD(a, b, c, d, e, int,,, CTRL_MDP2_XSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_MDP2_XEND,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_MDP2_YSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_MDP2_YEND,,,,,)\
    /* WPE_ISP_CROP_EN */\
    CMD(a, b, c, d, e, int,,, CTRL_WPE_ISP_XSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_WPE_ISP_XEND,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_WPE_ISP_YSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_WPE_ISP_YEND,,,,,)\
    /* WPE_MDP_CROP_EN */\
    CMD(a, b, c, d, e, int,,, CTRL_WPE_MDP_XSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_WPE_MDP_XEND,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_WPE_MDP_YSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_WPE_MDP_YEND,,,,,)\
	/* WPEO & MSKO */\
    CMD(a, b, c, d, e, int,,, CTRL_WPEO_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_MSKO_EN,,,,,)\
	/* WPE_C24, MDP_CROP, ISP_CROP */\
    CMD(a, b, c, d, e, int,,, CTRL_WPE_C24_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_WPE_MDP_CROP_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_WPE_ISP_CROP_EN,,,,,)\
    /* RCP2 */\
    CMD(a, b, c, d, e, int,,, CTRL_RCP2_XSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_RCP2_XEND,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_RCP2_YSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_RCP2_YEND,,,,,)\
    /* HFG_CROP_EN */\
    CMD(a, b, c, d, e, int,,, CTRL_HFG_XSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_HFG_XEND,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_HFG_YSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_HFG_YEND,,,,,)\
    /* NDG_CROP_EN */\
    CMD(a, b, c, d, e, int,,, CTRL_NDG_XSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_NDG_XEND,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_NDG_YSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_NDG_YEND,,,,,)\
    /* NDG2_CROP_EN */\
    CMD(a, b, c, d, e, int,,, CTRL_NDG2_XSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_NDG2_XEND,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_NDG2_YSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_NDG2_YEND,,,,,)\
	/* ADL */\
    CMD(a, b, c, d, e, int,,, ADLI_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, ADLI_YSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, ADLO_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, ADLO_YSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_IN_CROP_XSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_IN_CROP_XEND,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_IN_CROP_YSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_IN_CROP_YEND,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_OUT_CROP_XSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_OUT_CROP_XEND,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_OUT_CROP_YSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_OUT_CROP_YEND,,,,,)\
    CMD(a, b, c, d, e, int,,, WR_RING_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, WR_RING_XOFF,,,,,)\
    CMD(a, b, c, d, e, int,,, WR_RING_YOFF,,,,,)\
    CMD(a, b, c, d, e, int,,, WR_RING_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, WR_RING_YSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, WR_RING_STRIDE,,,,,)\
    CMD(a, b, c, d, e, int,,, RD_RING_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, RD_RING_XOFF,,,,,)\
    CMD(a, b, c, d, e, int,,, RD_RING_YOFF,,,,,)\
    CMD(a, b, c, d, e, int,,, RD_RING_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, RD_RING_YSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, RD_RING_STRIDE,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_4,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_5,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_6,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_7,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_8,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_9,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_10,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_11,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_12,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_13,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_14,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_15,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_16,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_17,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_18,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_19,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_20,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_21,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_22,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_23,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_24,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_25,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_26,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_27,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_28,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_29,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_30,,,,,)\
    CMD(a, b, c, d, e, int,,, ADL_INFO_31,,,,,)\

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
    CMD(a, b, c, d, e, int,,, nr3d_edge_flag,,,,,)/* buffer to record nr3d tile boundary*/\
	/* SMX1 */\
    CMD(a, b, c, d, e, bool,,, smx1_enable_flag,,,,,)\
    CMD(a, b, c, d, e, int,,, smx1o_xs,,,,,)\
    CMD(a, b, c, d, e, int,,, smx1o_xe,,,,,)\
    CMD(a, b, c, d, e, int,,, smx1io_offset,,,,,)\
	/* SMX2 */\
    CMD(a, b, c, d, e, bool,,, smx2_enable_flag,,,,,)\
    CMD(a, b, c, d, e, int,,, smx2o_xs,,,,,)\
    CMD(a, b, c, d, e, int,,, smx2o_xe,,,,,)\
    CMD(a, b, c, d, e, int,,, smx2io_offset,,,,,)\
	/* SMX3 */\
    CMD(a, b, c, d, e, bool,,, smx3_enable_flag,,,,,)\
    CMD(a, b, c, d, e, int,,, smx3o_xs,,,,,)\
    CMD(a, b, c, d, e, int,,, smx3o_xe,,,,,)\
    CMD(a, b, c, d, e, int,,, smx3io_offset,,,,,)\
	/* SMX4 */\
    CMD(a, b, c, d, e, bool,,, smx4_enable_flag,,,,,)\
    CMD(a, b, c, d, e, int,,, smx4o_xs,,,,,)\
    CMD(a, b, c, d, e, int,,, smx4o_xe,,,,,)\
    CMD(a, b, c, d, e, int,,, smx4io_offset,,,,,)\
    /* TCM_ENBALE updated by tile_xxx_tdr() */\
    CMD(a, b, c, d, e, int,,, LOG_IMGI_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_IMGBI_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_IMGCI_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_UFDI_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_UNP_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_UFD_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_BNR_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_LSC2_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_RNR_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_UDM_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_C24_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_VIPI_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_VIP2I_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_VIP3I_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_MFB_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_PAK2O_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_CDRZ_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_IMG2O_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_IMG2BO_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_C42_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_G2C_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_NBC_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_NBC2_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_MIX1_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_MIX2_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_SL2_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_SL2B_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_SL2C_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_SL2D_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_SL2E_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_SL2G_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_SL2H_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_SL2I_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_HFG_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_NDG_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_NDG2_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_SEEE_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_SMX1_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_SMX2_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_SMX3_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_SMX4_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_LCEI_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_G2G_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_FLC_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_FLC2_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_GGM_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_WUV_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_WSYNC_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_LCE_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_MIX3_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_MIX4_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_SRZ1_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_FE_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_FEO_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_C02_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_C02B_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_NR3D_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_COLOR_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_CRSP_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_IMG3O_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_IMG3BO_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_IMG3CO_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_SRZ2_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_C24B_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_DEPI_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_DMGI_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_PCA_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_ADBS2_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_DBS2_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_BNR2_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_CPN2_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_DCPN2_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_RMG2_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_RMM2_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_SRZ3_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_SRZ4_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_SRZ6_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_EAFI_MASK_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_EAFI_CUR_Y_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_EAFI_CUR_UV_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_EAFI_PRE_Y_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_EAFI_PRE_UV_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_EAFI_DEPTH_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_EAFI_LKH_WMAP_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_EAFI_LKH_EMAP_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_EAFO_FOUT_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_EAF_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_VECI_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_VEC2I_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_VEC3I_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_WPE_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_WPEO_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_MSKO_EN,,,,,)\
   CMD(a, b, c, d, e, int,,, LOG_ADL_EN,,,,,)\
   CMD(a, b, c, d, e, int,,, LOG_WPE_C24_EN,,,,,)\
   CMD(a, b, c, d, e, int,,, LOG_WPE_ISP_CROP_EN,,,,,)\
   CMD(a, b, c, d, e, int,,, LOG_WPE_MDP_CROP_EN,,,,,)\
	CMD(a, b, c, d, e, int,,, LOG_BLDI_EN,,,,,)\
	CMD(a, b, c, d, e, int,,, LOG_BLDBI_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_BLD2I_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_BLD2BI_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_BLD3I_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_BLD4I_EN,,,,,)\
	CMD(a, b, c, d, e, int,,, LOG_BLD_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_MFB_SRZ_EN,,,,,)\
	CMD(a, b, c, d, e, int,,, LOG_BLDO_EN,,,,,)\
	CMD(a, b, c, d, e, int,,, LOG_BLDBO_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_BLD2O_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_CRSPB_EN,,,,,)\

#endif
