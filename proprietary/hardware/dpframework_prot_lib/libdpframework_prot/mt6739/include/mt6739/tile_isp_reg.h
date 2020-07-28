#ifndef __TILE_ISP_REG_H__
#define __TILE_ISP_REG_H__

/* input datat type enum */
typedef enum ISP_TILE_IN_DATA_TYPE_ENUM
{
    ISP_TILE_IN_DATA_RAW,
    ISP_TILE_IN_DATA_YUV,
    ISP_TILE_IN_DATA_RGB,
    ISP_TILE_IN_DATA_NUM,
} ISP_TILE_IN_DATA_TYPE_ENUM;

/* error enum */
#define ISP_TILE_ERROR_MESSAGE_ENUM(n, CMD) \
    /* VIPI check */\
    CMD(n, ISP_MESSAGE_VIPI_FORMAT_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_VIPI_XSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_VIPI_YSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* VIP2I check */\
    CMD(n, ISP_MESSAGE_VIP2I_FORMAT_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_VIP2I_XSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_VIP2I_YSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* VIP3I check */\
    CMD(n, ISP_MESSAGE_ILLEGAL_BLD_MODE_FOR_VIP3I_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_VIP3I_FORMAT_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_VIP3I_XSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_VIP3I_YSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* IMG2O size check */\
    CMD(n, ISP_MESSAGE_IMG2O_XSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_IMG2O_YSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
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
    CMD(n, ISP_MESSAGE_ILLEGAL_CCL_MUX_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_ILLEGAL_G2G_MUX_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_ILLEGAL_C24_MUX_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_ILLEGAL_SRZ1_MUX_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_ILLEGAL_MIX1_MUX_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_ILLEGAL_PCA_MUX_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_ILLEGAL_CRZ_MUX_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_ILLEGAL_NR3D_MUX_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
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
	/* C02 check */\
    CMD(n, ISP_MESSAGE_C02_MIN_X_IN_SIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_C02_MIN_Y_IN_SIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* resizer coeff check */\
    CMD(n, ISP_MESSAGE_RESIZER_UNMATCH_INPUT_SIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
	/* UDM & NBC size constraints */\
    CMD(n, ISP_MESSAGE_UNDER_NBC_MIN_XSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_UNDER_NBC_MIN_YSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_UNDER_SEEE_MIN_YSIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\

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
    /* IMGI input width & height */\
    CMD(a, b, c, d, e, int, isp_tile_src_width, sw.src_width, isp_tile_src_width,, true,,, 0)\
    CMD(a, b, c, d, e, int, isp_tile_src_height, sw.src_height, isp_tile_src_height,, true,,, 0)\
    CMD(a, b, c, d, e, int, tile_width, sw.tpipe_width, isp_tile_width,, true,,, 1)\
    CMD(a, b, c, d, e, int, tile_height, sw.tpipe_height, isp_tile_height,, true,,, 1)\
    /* 0: stop final, 1: stop per line, 2: stop per tile*/\
    CMD(a, b, c, d, e, int, tile_irq_mode, sw.tpipe_irq_mode, last_irq_mode,, true,,, 1)\
	/* dummy reg */\
    CMD(a, b, c, d, e, int, TILE_C02_EN, top.c02_en, C02_EN,, true, CAM_CTL_EN_P2, C02_EN, 4)\
    /* UFDI */\
    CMD(a, b, c, d, e, int, UFDI_STRIDE, ufdi.ufdi_stride, UFDI_STRIDE,, REG_CMP_EQ(c, LOG_UFDI_EN, 1), CAM_UFDI_STRIDE, STRIDE, 4)\
    CMD(a, b, c, d, e, int, LCEI_V_FLIP_EN, top.lcei_v_flip_en, LCEI_V_FLIP_EN,, true, CAM_VERTICAL_FLIP_EN, LCEI_V_FLIP_EN, 4)\
    CMD(a, b, c, d, e, int, UFDI_V_FLIP_EN, top.ufdi_v_flip_en, UFDI_V_FLIP_EN,, true, CAM_VERTICAL_FLIP_EN, UFDI_V_FLIP_EN, 4)\
    /* VIPI */\
    CMD(a, b, c, d, e, int, VIPI_XSIZE, vipi.vipi_xsize, VIPI_XSIZE,, REG_CMP_EQ(c, LOG_VIPI_EN, 1), CAM_VIPI_XSIZE, XSIZE, 0)\
    CMD(a, b, c, d, e, int, VIPI_YSIZE, vipi.vipi_ysize, VIPI_YSIZE,, REG_CMP_EQ(c, LOG_VIPI_EN, 1), CAM_VIPI_YSIZE, YSIZE, 0)\
    CMD(a, b, c, d, e, int, VIPI_STRIDE, vipi.vipi_stride, VIPI_STRIDE,, REG_CMP_EQ(c, LOG_VIPI_EN, 1), CAM_VIPI_STRIDE, STRIDE, 4)\
    /* VIP2I */\
    CMD(a, b, c, d, e, int, VIP2I_XSIZE, vip2i.vip2i_xsize, VIP2I_XSIZE,, REG_CMP_EQ(c, LOG_VIP2I_EN, 1), CAM_VIP2I_XSIZE, XSIZE, 0)\
    CMD(a, b, c, d, e, int, VIP2I_YSIZE, vip2i.vip2i_ysize, VIP2I_YSIZE,, REG_CMP_EQ(c, LOG_VIP2I_EN, 1), CAM_VIP2I_YSIZE, YSIZE, 0)\
    CMD(a, b, c, d, e, int, VIP2I_STRIDE, vip2i.vip2i_stride, VIP2I_STRIDE,, REG_CMP_EQ(c, LOG_VIP2I_EN, 1), CAM_VIP2I_STRIDE, STRIDE, 4)\
    /* VIP3I */\
    CMD(a, b, c, d, e, int, VIP3I_XSIZE, vip3i.vip3i_xsize, VIP3I_XSIZE,, REG_CMP_EQ(c, LOG_VIP3I_EN, 1), CAM_VIP3I_XSIZE, XSIZE, 0)\
    CMD(a, b, c, d, e, int, VIP3I_YSIZE, vip3i.vip3i_ysize, VIP3I_YSIZE,, REG_CMP_EQ(c, LOG_VIP3I_EN, 1), CAM_VIP3I_YSIZE, YSIZE, 0)\
    CMD(a, b, c, d, e, int, VIP3I_STRIDE, vip3i.vip3i_stride, VIP3I_STRIDE,, REG_CMP_EQ(c, LOG_VIP3I_EN, 1), CAM_VIP3I_STRIDE, STRIDE, 4)\
    /* MFB */\
    CMD(a, b, c, d, e, int, BLD_MODE, mfb.bld_mode, BLD_MODE,, REG_CMP_EQ(c, LOG_MFB_EN, 1), CAM_MFB_CON, BLD_MODE, 4)\
    CMD(a, b, c, d, e, int, BLD_LL_DB_EN, mfb.bld_deblock_en, BLD_DEBLOCK_EN,, REG_CMP_EQ(c, LOG_MFB_EN, 1), CAM_MFB_LL_CON3, BLD_LL_DB_EN, 4)\
    CMD(a, b, c, d, e, int, BLD_LL_BRZ_EN, mfb.bld_brz_en, BLD_LL_BRZ_EN,, REG_CMP_EQ(c, LOG_MFB_EN, 1), CAM_MFB_LL_CON3, BLD_LL_BRZ_EN, 4)\
    /* MFBO */\
    CMD(a, b, c, d, e, int, MFBO_STRIDE, mfbo.mfbo_stride, MFBO_STRIDE,, REG_CMP_EQ(c, LOG_MFBO_EN, 1), CAM_MFBO_STRIDE, STRIDE, 4)\
    CMD(a, b, c, d, e, int, MFBO_XOFFSET, mfbo.mfbo_xoffset, MFBO_XOFFSET,, REG_CMP_EQ(c, LOG_MFBO_EN, 1), CAM_MFBO_CROP, XOFFSET, 0)\
    CMD(a, b, c, d, e, int, MFBO_YOFFSET, mfbo.mfbo_yoffset, MFBO_YOFFSET,, REG_CMP_EQ(c, LOG_MFBO_EN, 1), CAM_MFBO_CROP, YOFFSET, 0)\
    CMD(a, b, c, d, e, int, MFBO_XSIZE, mfbo.mfbo_xsize, MFBO_XSIZE,, REG_CMP_EQ(c, LOG_MFBO_EN, 1), CAM_MFBO_XSIZE, XSIZE, 0)\
    CMD(a, b, c, d, e, int, MFBO_YSIZE, mfbo.mfbo_ysize, MFBO_YSIZE,, REG_CMP_EQ(c, LOG_MFBO_EN, 1), CAM_MFBO_YSIZE, YSIZE, 0)\
    /* LCEI */\
    CMD(a, b, c, d, e, int, LCEI_STRIDE, lcei.lcei_stride, LCEI_STRIDE,, REG_CMP_EQ(c, LOG_LCEI_EN, 1), CAM_LCEI_STRIDE, STRIDE, 4)\
    /* LCE */\
    CMD(a, b, c, d, e, int, LCE_SLM_WD, lce.lce_slm_width, SLM_WIDTH,, REG_CMP_EQ(c, LOG_LCE_EN, 1), CAM_LCE_SLM_SIZE, LCE_SLM_WD, 0)\
    CMD(a, b, c, d, e, int, LCE_SLM_HT, lce.lce_slm_height, SLM_HEIGHT,, REG_CMP_EQ(c, LOG_LCE_EN, 1), CAM_LCE_SLM_SIZE, LCE_SLM_HT, 0)\
    CMD(a, b, c, d, e, int, LCE_BCMK_X, lce.lce_bc_mag_kubnx, BC_MAG_KUBNX,, REG_CMP_EQ(c, LOG_LCE_EN, 1), CAM_LCE_ZR, LCE_BCMK_X, 4)\
    CMD(a, b, c, d, e, int, LCE_BCMK_Y, lce.lce_bc_mag_kubny, BC_MAG_KUBNY,, REG_CMP_EQ(c, LOG_LCE_EN, 1), CAM_LCE_ZR, LCE_BCMK_Y, 4)\
    /* SRZ1 */\
    CMD(a, b, c, d, e, int, SRZ1_IN_WD, srz1.srz_input_crop_width, SRZ1_Input_Image_W,, REG_CMP_EQ(c, LOG_SRZ1_EN, 1), CAM_SRZ1_IN_IMG, SRZ1_IN_WD, 0)\
    CMD(a, b, c, d, e, int, SRZ1_IN_HT, srz1.srz_input_crop_height, SRZ1_Input_Image_H,, REG_CMP_EQ(c, LOG_SRZ1_EN, 1), CAM_SRZ1_IN_IMG, SRZ1_IN_HT, 0)\
    CMD(a, b, c, d, e, int, SRZ1_OUT_WD, srz1.srz_output_width, SRZ1_Output_Image_W,, REG_CMP_EQ(c, LOG_SRZ1_EN, 1), CAM_SRZ1_OUT_IMG, SRZ1_OUT_WD, 0)\
    CMD(a, b, c, d, e, int, SRZ1_OUT_HT, srz1.srz_output_height, SRZ1_Output_Image_H,, REG_CMP_EQ(c, LOG_SRZ1_EN, 1), CAM_SRZ1_OUT_IMG, SRZ1_OUT_HT, 0)\
    CMD(a, b, c, d, e, int, SRZ1_HORI_INT_OFST, srz1.srz_luma_horizontal_integer_offset, SRZ1_Luma_Horizontal_Integer_Offset,, REG_CMP_EQ(c, LOG_SRZ1_EN, 1), CAM_SRZ1_HORI_INT_OFST, SRZ1_HORI_INT_OFST, 0)\
    CMD(a, b, c, d, e, int, SRZ1_HORI_SUB_OFST, srz1.srz_luma_horizontal_subpixel_offset, SRZ1_Luma_Horizontal_Subpixel_Offset,, REG_CMP_EQ(c, LOG_SRZ1_EN, 1), CAM_SRZ1_HORI_SUB_OFST, SRZ1_HORI_SUB_OFST, 0)\
    CMD(a, b, c, d, e, int, SRZ1_VERT_INT_OFST, srz1.srz_luma_vertical_integer_offset, SRZ1_Luma_Vertical_Integer_Offset,, REG_CMP_EQ(c, LOG_SRZ1_EN, 1), CAM_SRZ1_VERT_INT_OFST, SRZ1_VERT_INT_OFST, 0)\
    CMD(a, b, c, d, e, int, SRZ1_VERT_SUB_OFST, srz1.srz_luma_vertical_subpixel_offset, SRZ1_Luma_Vertical_Subpixel_Offset,, REG_CMP_EQ(c, LOG_SRZ1_EN, 1), CAM_SRZ1_VERT_SUB_OFST, SRZ1_VERT_SUB_OFST, 0)\
    CMD(a, b, c, d, e, int, SRZ1_HORI_STEP, srz1.srz_horizontal_coeff_step, SRZ1_Horizontal_Coeff_Step,, REG_CMP_EQ(c, LOG_SRZ1_EN, 1), CAM_SRZ1_HORI_STEP, SRZ1_HORI_STEP, 4)\
    CMD(a, b, c, d, e, int, SRZ1_VERT_STEP, srz1.srz_vertical_coeff_step, SRZ1_Vertical_Coeff_Step,, REG_CMP_EQ(c, LOG_SRZ1_EN, 1), CAM_SRZ1_VERT_STEP, SRZ1_VERT_STEP, 4)\
    /* SRZ2 */\
    CMD(a, b, c, d, e, int, SRZ2_IN_WD, srz2.srz_input_crop_width, SRZ2_Input_Image_W,, REG_CMP_EQ(c, LOG_SRZ2_EN, 1), CAM_SRZ2_IN_IMG, SRZ2_IN_WD, 0)\
    CMD(a, b, c, d, e, int, SRZ2_IN_HT, srz2.srz_input_crop_height, SRZ2_Input_Image_H,, REG_CMP_EQ(c, LOG_SRZ2_EN, 1), CAM_SRZ2_IN_IMG, SRZ2_IN_HT, 0)\
    CMD(a, b, c, d, e, int, SRZ2_OUT_WD, srz2.srz_output_width, SRZ2_Output_Image_W,, REG_CMP_EQ(c, LOG_SRZ2_EN, 1), CAM_SRZ2_OUT_IMG, SRZ2_OUT_WD, 0)\
    CMD(a, b, c, d, e, int, SRZ2_OUT_HT, srz2.srz_output_height, SRZ2_Output_Image_H,, REG_CMP_EQ(c, LOG_SRZ2_EN, 1), CAM_SRZ2_OUT_IMG, SRZ2_OUT_HT, 0)\
    CMD(a, b, c, d, e, int, SRZ2_HORI_INT_OFST, srz2.srz_luma_horizontal_integer_offset, SRZ2_Luma_Horizontal_Integer_Offset,, REG_CMP_EQ(c, LOG_SRZ2_EN, 1), CAM_SRZ2_HORI_INT_OFST, SRZ2_HORI_INT_OFST, 0)\
    CMD(a, b, c, d, e, int, SRZ2_HORI_SUB_OFST, srz2.srz_luma_horizontal_subpixel_offset, SRZ2_Luma_Horizontal_Subpixel_Offset,, REG_CMP_EQ(c, LOG_SRZ2_EN, 1), CAM_SRZ2_HORI_SUB_OFST, SRZ2_HORI_SUB_OFST, 0)\
    CMD(a, b, c, d, e, int, SRZ2_VERT_INT_OFST, srz2.srz_luma_vertical_integer_offset, SRZ2_Luma_Vertical_Integer_Offset,, REG_CMP_EQ(c, LOG_SRZ2_EN, 1), CAM_SRZ2_VERT_INT_OFST, SRZ2_VERT_INT_OFST, 0)\
    CMD(a, b, c, d, e, int, SRZ2_VERT_SUB_OFST, srz2.srz_luma_vertical_subpixel_offset, SRZ2_Luma_Vertical_Subpixel_Offset,, REG_CMP_EQ(c, LOG_SRZ2_EN, 1), CAM_SRZ2_VERT_SUB_OFST, SRZ2_VERT_SUB_OFST, 0)\
    CMD(a, b, c, d, e, int, SRZ2_HORI_STEP, srz2.srz_horizontal_coeff_step, SRZ2_Horizontal_Coeff_Step,, REG_CMP_EQ(c, LOG_SRZ2_EN, 1), CAM_SRZ2_HORI_STEP, SRZ2_HORI_STEP, 4)\
    CMD(a, b, c, d, e, int, SRZ2_VERT_STEP, srz2.srz_vertical_coeff_step, SRZ2_Vertical_Coeff_Step,, REG_CMP_EQ(c, LOG_SRZ2_EN, 1), CAM_SRZ2_VERT_STEP, SRZ2_VERT_STEP, 4)\
    /* FE */\
    CMD(a, b, c, d, e, int, FE_MODE, fe.fe_mode, FE_MODE,, REG_CMP_EQ(c, LOG_FE_EN, 1), CAM_FE_CTRL, FE_MODE, 4)\
    /* FEO */\
    CMD(a, b, c, d, e, int, FEO_STRIDE, feo.feo_stride, FEO_STRIDE,, REG_CMP_EQ(c, LOG_FEO_EN, 1), CAM_FEO_STRIDE, STRIDE, 4)\
    /* NR3D */\
    CMD(a, b, c, d, e, int, NR3D_ON_EN, nr3d.nr3d_on_en, NR3D_ON_EN,, REG_CMP_EQ(c, LOG_NR3D_EN, 1), CAM_NR3D_CTRL, NR3D_ON_EN, 4)\
    CMD(a, b, c, d, e, int, NR3D_ON_OFST_X, nr3d.nr3d_on_xoffset, NR3D_ON_OFST_X,, REG_CMP_EQ(c, LOG_NR3D_EN, 1), CAM_NR3D_ON_OFF, NR3D_ON_OFST_X, 0)\
    CMD(a, b, c, d, e, int, NR3D_ON_OFST_Y, nr3d.nr3d_on_yoffset, NR3D_ON_OFST_Y,, REG_CMP_EQ(c, LOG_NR3D_EN, 1), CAM_NR3D_ON_OFF, NR3D_ON_OFST_Y, 0)\
    CMD(a, b, c, d, e, int, NR3D_ON_WD, nr3d.nr3d_on_width, NR3D_ON_WD,, REG_CMP_EQ(c, LOG_NR3D_EN, 1), CAM_NR3D_ON_SIZ, NR3D_ON_WD, 0)\
    CMD(a, b, c, d, e, int, NR3D_ON_HT, nr3d.nr3d_on_height, NR3D_ON_HT,, REG_CMP_EQ(c, LOG_NR3D_EN, 1), CAM_NR3D_ON_SIZ, NR3D_ON_HT, 0)\
	/* CRSP */\
    CMD(a, b, c, d, e, int, CRSP_cstep_y, crsp.crsp_ystep, CRSP_STEP_Y,, REG_CMP_EQ(c, LOG_CRSP_EN, 1), CAM_CRSP_STEP_OFST, CRSP_STEP_Y, 0)\
    CMD(a, b, c, d, e, int, CRSP_offset_x, crsp.crsp_xoffset, CRSP_OFST_X,, REG_CMP_EQ(c, LOG_CRSP_EN, 1), CAM_CRSP_STEP_OFST, CRSP_OFST_X, 0)\
    CMD(a, b, c, d, e, int, CRSP_offset_y, crsp.crsp_yoffset, CRSP_OFST_Y,, REG_CMP_EQ(c, LOG_CRSP_EN, 1), CAM_CRSP_STEP_OFST, CRSP_OFST_Y, 0)\
    /* IMG3O */\
    CMD(a, b, c, d, e, int, IMG3O_STRIDE, img3o.img3o_stride, IMG3O_STRIDE,, REG_CMP_EQ(c, LOG_IMG3O_EN, 1), CAM_IMG3O_STRIDE, STRIDE, 4)\
    CMD(a, b, c, d, e, int, IMG3O_XOFFSET, img3o.img3o_xoffset, IMG3O_XOFFSET,, REG_CMP_EQ(c, LOG_IMG3O_EN, 1), CAM_IMG3O_CROP, XOFFSET, 0)\
    CMD(a, b, c, d, e, int, IMG3O_YOFFSET, img3o.img3o_yoffset, IMG3O_YOFFSET,, REG_CMP_EQ(c, LOG_IMG3O_EN, 1), CAM_IMG3O_CROP, YOFFSET, 0)\
    CMD(a, b, c, d, e, int, IMG3O_XSIZE, img3o.img3o_xsize, IMG3O_XSIZE,, REG_CMP_EQ(c, LOG_IMG3O_EN, 1), CAM_IMG3O_XSIZE, XSIZE, 0)\
    CMD(a, b, c, d, e, int, IMG3O_YSIZE, img3o.img3o_ysize, IMG3O_YSIZE,, REG_CMP_EQ(c, LOG_IMG3O_EN, 1), CAM_IMG3O_YSIZE, YSIZE, 0)\
    /* IMG3BO */\
    CMD(a, b, c, d, e, int, IMG3BO_STRIDE, img3bo.img3bo_stride, IMG3BO_STRIDE,, REG_CMP_EQ(c, LOG_IMG3BO_EN, 1), CAM_IMG3BO_STRIDE, STRIDE, 4)\
    CMD(a, b, c, d, e, int, IMG3BO_XSIZE, img3bo.img3bo_xsize, IMG3BO_XSIZE,, REG_CMP_EQ(c, LOG_IMG3BO_EN, 1), CAM_IMG3BO_XSIZE, XSIZE, 0)\
    CMD(a, b, c, d, e, int, IMG3BO_YSIZE, img3bo.img3bo_ysize, IMG3BO_YSIZE,, REG_CMP_EQ(c, LOG_IMG3BO_EN, 1), CAM_IMG3BO_YSIZE, YSIZE, 0)\
    /* IMG3CO */\
    CMD(a, b, c, d, e, int, IMG3CO_STRIDE, img3co.img3co_stride, IMG3CO_STRIDE,, REG_CMP_EQ(c, LOG_IMG3CO_EN, 1), CAM_IMG3CO_STRIDE, STRIDE, 4)\
    CMD(a, b, c, d, e, int, IMG3CO_XSIZE, img3co.img3co_xsize, IMG3CO_XSIZE,, REG_CMP_EQ(c, LOG_IMG3CO_EN, 1), CAM_IMG3CO_XSIZE, XSIZE, 0)\
    CMD(a, b, c, d, e, int, IMG3CO_YSIZE, img3co.img3co_ysize, IMG3CO_YSIZE,, REG_CMP_EQ(c, LOG_IMG3CO_EN, 1), CAM_IMG3CO_YSIZE, YSIZE, 0)\

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
#define ISP_TILE_HW_REG_LUT(CMD, a, b, c, d, e) \
    /* Common */\
    CMD(a, b, c, d, e, int, SCENARIO, top.scenario, SCENARIO,, true, CAM_CTL_SCENARIO, SCENARIO, 4)\
    CMD(a, b, c, d, e, int, MODE, top.mode, MODE,, true, CAM_CTL_SCENARIO, SUB_MODE, 4)\
    CMD(a, b, c, d, e, int, PIX_ID, top.pixel_id, PIX_ID,, true, CAM_CTL_FMT_SEL_P1, PIX_ID, 4)\
    CMD(a, b, c, d, e, int, CSR_IN_FMT, top.cam_in_fmt, CAM_IN_FMT,, true, CAM_CTL_FMT_SEL_P2, IN_FMT, 4)\
    CMD(a, b, c, d, e, int, CTL_EXTENSION_EN, top.ctl_extension_en, CTL_EXTENSION_EN,, true, CAM_CTL_TILE, CTL_EXTENSION_EN, 4)\
    CMD(a, b, c, d, e, int, FG_MODE, top.fg_mode, FG_MODE,, true, CAM_CTL_FMT_SEL_P2, FG_MODE, 4)\
    CMD(a, b, c, d, e, int, UFDI_FMT, top.ufdi_fmt, UFDI_FMT,, true, CAM_CTL_FMT_SEL_P2, UFDI_FMT, 4)\
    CMD(a, b, c, d, e, int, VIPI_FMT, top.vipi_fmt, VIPI_FMT,, true, CAM_CTL_FMT_SEL_P2, VIPI_FMT, 4)\
    CMD(a, b, c, d, e, int, IMG3O_FMT, top.img3o_fmt, IMG3O_FMT,, true, CAM_CTL_FMT_SEL_P2, IMG3O_FMT, 4)\
    /* module enable register */\
    CMD(a, b, c, d, e, int, TILE_IMGI_EN, top.imgi_en, IMGI_EN,, true, CAM_CTL_EN_P2_DMA, IMGI_EN, 4)\
    CMD(a, b, c, d, e, int, TILE_DBS_EN, top.dbs_en, DBS_EN,, true, CAM_CTL_EN_P1, DBS_EN, 4)\
    CMD(a, b, c, d, e, int, TILE_UFDI_EN, top.ufdi_en, UFDI_EN,, true, CAM_CTL_EN_P2_DMA, UFDI_EN, 4)\
    CMD(a, b, c, d, e, int, TILE_UNP_EN, top.unp_en, UNP_EN,, true, CAM_CTL_EN_P2, UNP_EN, 4)\
    CMD(a, b, c, d, e, int, TILE_UFD_EN, top.ufd_en, UFD_EN,, true, CAM_CTL_EN_P2, UFD_EN, 4)\
    CMD(a, b, c, d, e, int, TILE_BNR_EN, top.bnr_en, BNR_ENABLE,, MODE_CMP_EQ(c, TPIPE_SCENARIO_IP, TPIPE_SUBMODE_IP_RAW), CAM_CTL_EN_P1, BNR_EN, 4)\
    CMD(a, b, c, d, e, int, TILE_LSCI_EN, top.lsci_en, LSCI_EN,, MODE_CMP_EQ(c, TPIPE_SCENARIO_IP, TPIPE_SUBMODE_IP_RAW), CAM_CTL_EN_P1_DMA, LSCI_EN, 4)\
    CMD(a, b, c, d, e, int, TILE_LSC_EN, top.lsc_en, LSC_EN,, MODE_CMP_EQ(c, TPIPE_SCENARIO_IP, TPIPE_SUBMODE_IP_RAW), CAM_CTL_EN_P1, LSC_EN, 4)\
    CMD(a, b, c, d, e, int, TILE_SL2_EN, top.sl2_en, SL2_EN,, true, CAM_CTL_EN_P2, SL2_EN, 4)\
    CMD(a, b, c, d, e, int, TILE_CFA_EN, top.cfa_en, CFA_EN,, true, CAM_CTL_EN_P2, CFA_EN, 4)\
    CMD(a, b, c, d, e, int, TILE_C24_EN, top.c24_en, C24_EN,, true, CAM_CTL_EN_P2, C24_EN, 4)\
    CMD(a, b, c, d, e, int, TILE_VIPI_EN, top.vipi_en, VIPI_EN,, true, CAM_CTL_EN_P2_DMA, VIPI_EN, 4)\
    CMD(a, b, c, d, e, int, TILE_VIP2I_EN, top.vip2i_en, VIP2I_EN,, true, CAM_CTL_EN_P2_DMA, VIP2I_EN, 4)\
    CMD(a, b, c, d, e, int, TILE_VIP3I_EN, top.vip3i_en, VIP3I_EN,, true, CAM_CTL_EN_P2_DMA, VIP3I_EN, 4)\
    CMD(a, b, c, d, e, int, TILE_MFB_EN, top.mfb_en, MFB_EN,, true, CAM_CTL_EN_P2, MFB_EN, 4)\
    CMD(a, b, c, d, e, int, TILE_MFBO_EN, top.mfbo_en, MFBO_EN,, true, CAM_CTL_EN_P2_DMA, MFBO_EN, 4)\
    CMD(a, b, c, d, e, int, TILE_G2C_EN, top.g2c_en, G2C_EN,, true, CAM_CTL_EN_P2, G2C_EN, 4)\
    CMD(a, b, c, d, e, int, TILE_C42_EN, top.c42_en, C42_EN,, true, CAM_CTL_EN_P2, C42_EN, 4)\
    CMD(a, b, c, d, e, int, TILE_NBC_EN, top.nbc_en, NBC_EN,, true, CAM_CTL_EN_P2, NBC_EN, 4)\
    CMD(a, b, c, d, e, int, TILE_MIX1_EN, top.mix1_en, MIX1_EN,, true, CAM_CTL_EN_P2, MIX1_EN, 4)\
    CMD(a, b, c, d, e, int, TILE_MIX2_EN, top.mix2_en, MIX2_EN,, true, CAM_CTL_EN_P2, MIX2_EN, 4)\
    CMD(a, b, c, d, e, int, TILE_PCA_EN, top.pca_en, PCA_EN,, true, CAM_CTL_EN_P2, PCA_EN, 0)\
    CMD(a, b, c, d, e, int, TILE_SL2C_EN, top.sl2c_en, SL2C_EN,, true, CAM_CTL_EN_P2, SL2C_EN, 4)\
    CMD(a, b, c, d, e, int, TILE_SEEE_EN, top.seee_en, SEEE_EN,, true, CAM_CTL_EN_P2, SEEE_EN, 4)\
    CMD(a, b, c, d, e, int, TILE_LCEI_EN, top.lcei_en, LCEI_EN,, true, CAM_CTL_EN_P2_DMA, LCEI_EN, 4)\
    CMD(a, b, c, d, e, int, TILE_LCE_EN, top.lce_en, LCE_EN,, true, CAM_CTL_EN_P2, LCE_EN, 4)\
    CMD(a, b, c, d, e, int, TILE_MIX3_EN, top.mix3_en, MIX3_EN,, true, CAM_CTL_EN_P2, MIX3_EN, 4)\
    CMD(a, b, c, d, e, int, TILE_CDRZ_EN, top.crz_en, CDRZ_EN,, true, CAM_CTL_EN_P2, CRZ_EN, 4)\
    CMD(a, b, c, d, e, int, TILE_IMG2O_EN, top.img2o_en, IMG2O_EN,, true, CAM_CTL_EN_P2_DMA, IMG2O_EN, 4)\
    CMD(a, b, c, d, e, int, TILE_SRZ1_EN, top.srz1_en, SRZ1_EN,, true, CAM_CTL_EN_P2, SRZ1_EN, 4)\
    CMD(a, b, c, d, e, int, TILE_FE_EN, top.fe_en, FE_EN,, true, CAM_CTL_EN_P2, FE_EN, 4)\
    CMD(a, b, c, d, e, int, TILE_FEO_EN, top.feo_en, FEO_EN,, true, CAM_CTL_EN_P2_DMA, FEO_EN, 4)\
    CMD(a, b, c, d, e, int, TILE_NR3D_EN, top.nr3d_en, NR3D_EN,, true, CAM_CTL_EN_P2, NR3D_EN, 4)\
    CMD(a, b, c, d, e, int, TILE_CRSP_EN, top.crsp_en, CRSP_EN,, true, CAM_CTL_EN_P2, CRSP_EN, 4)\
    CMD(a, b, c, d, e, int, TILE_IMG3O_EN, top.img3o_en, IMG3O_EN,, true, CAM_CTL_EN_P2_DMA, IMG3O_EN, 4)\
    CMD(a, b, c, d, e, int, TILE_IMG3BO_EN, top.img3bo_en, IMG3BO_EN,, true, CAM_CTL_EN_P2_DMA, IMG3BO_EN, 4)\
    CMD(a, b, c, d, e, int, TILE_IMG3CO_EN, top.img3co_en, IMG3CO_EN,, true, CAM_CTL_EN_P2_DMA, IMG3CO_EN, 4)\
    CMD(a, b, c, d, e, int, TILE_C24B_EN, top.c24b_en, C24B_EN,, true, CAM_CTL_EN_P2, C24B_EN, 4)\
    CMD(a, b, c, d, e, int, TILE_MDP_CROP_EN, top.mdp_crop_en, MDP_CROP_EN,, true, CAM_CTL_EN_P2, MDPCROP_EN, 4)\
    CMD(a, b, c, d, e, int, TILE_SRZ2_EN, top.srz2_en, SRZ2_EN,, true, CAM_CTL_EN_P2, SRZ2_EN, 4)\
    CMD(a, b, c, d, e, int, TILE_NSL2A_EN, top.nsl2a_en, NSL2A_EN,, true, CAM_CTL_EN_P2, NSL2A_EN, 4)\
    /* RDMA Vertical Flip Enable */\
    CMD(a, b, c, d, e, int, IMGI_V_FLIP_EN, top.imgi_v_flip_en, IMGI_V_FLIP_EN,, true, CAM_VERTICAL_FLIP_EN, IMGI_V_FLIP_EN, 4)\
    /* MUX */\
    CMD(a, b, c, d, e, int, UFD_SEL, top.ufd_sel, UFD_SEL,, true, CAM_CTL_SEL_P2, UFD_SEL, 4)\
    CMD(a, b, c, d, e, int, CCL_SEL, top.ccl_sel, CCL_SEL,, true, CAM_CTL_SEL_P2, CCL_SEL, 4)\
    CMD(a, b, c, d, e, int, CCL_SEL_EN, top.ccl_sel_en, CCL_SEL_EN,, true, CAM_CTL_SEL_P2, CCL_SEL_EN, 4)\
    CMD(a, b, c, d, e, int, G2G_SEL, top.g2g_sel, G2G_SEL,, true, CAM_CTL_SEL_P2, G2G_SEL, 4)\
    CMD(a, b, c, d, e, int, G2G_SEL_EN, top.g2g_sel_en, G2G_SEL_EN,, true, CAM_CTL_SEL_P2, G2G_SEL_EN, 4)\
    CMD(a, b, c, d, e, int, C24_SEL, top.c24_sel, C24_SEL,, true, CAM_CTL_SEL_P2, C24_SEL, 4)\
    CMD(a, b, c, d, e, int, SRZ1_SEL, top.srz1_sel, SRZ1_SEL,, true, CAM_CTL_SEL_P2, SRZ1_SEL, 4)\
    CMD(a, b, c, d, e, int, MIX1_SEL, top.mix1_sel, MIX1_SEL,, true, CAM_CTL_SEL_P2, MIX1_SEL, 4)\
    CMD(a, b, c, d, e, int, CRZ_SEL, top.crz_sel, CRZ_SEL,, true, CAM_CTL_SEL_P2, CRZ_SEL, 4)\
    CMD(a, b, c, d, e, int, NR3D_SEL, top.nr3d_sel, NR3D_SEL,, true, CAM_CTL_SEL_P2, NR3D_SEL, 4)\
    CMD(a, b, c, d, e, int, FE_SEL, top.fe_sel, FE_SEL,, true, CAM_CTL_SEL_P2, FE_SEL, 4)\
    CMD(a, b, c, d, e, int, MDP_SEL, top.mdp_sel, MDP_SEL,, true, CAM_CTL_SEL_P2, MDP_SEL, 4)\
    CMD(a, b, c, d, e, int, PCA_SEL, top.pca_sel, PCA_SEL,, true, CAM_CTL_SEL_P2, PCA_SEL, 4)\
    /* interlace mode */\
    CMD(a, b, c, d, e, int, INTERLACE_MODE, top.interlace_mode, INTERLACE_MODE,, true, CAM_IMGI_SLOW_DOWN, INTERLACE_MODE, 4)\
    /* IMGI */\
    CMD(a, b, c, d, e, int, IMGI_STRIDE, imgi.imgi_stride, IMGI_STRIDE,, REG_CMP_EQ(c, LOG_IMGI_EN, 1), CAM_IMGI_STRIDE, STRIDE, 4)\
    /* 2D BPC */\
    CMD(a, b, c, d, e, int, TILE_BPC_2D_EN, bnr.bpc_en, BPC_ENABLE,, REG_CMP_EQ(c, LOG_BNR_EN, 1), CAM_BNR_BPC_CON, BPC_EN, 4)\
    CMD(a, b, c, d, e, int, BPC_LUT_EN, bnr.bpc_tbl_en, BPC_TBL_EN,, REG_CMP_EQ(c, LOG_BNR_EN, 1), CAM_BNR_BPC_CON, BPC_LUT_EN, 4)\
    /* LSCI */\
    CMD(a, b, c, d, e, int, LSCI_STRIDE, lsci.lsci_stride, LSCI_STRIDE,, REG_CMP_EQ(c, LOG_LSCI_EN, 1), CAM_LSCI_STRIDE, STRIDE, 4)\
    /* LSC */\
    CMD(a, b, c, d, e, int, ShadingBlk_Xnum, lsc.sdblk_xnum, SDBLK_XNUM,, REG_CMP_EQ(c, LOG_LSC_EN, 1), CAM_LSC_CTL2, LSC_SDBLK_XNUM, 0)\
    CMD(a, b, c, d, e, int, ShadingBlk_Ynum, lsc.sdblk_ynum, SDBLK_YNUM,, REG_CMP_EQ(c, LOG_LSC_EN, 1), CAM_LSC_CTL3, LSC_SDBLK_YNUM, 0)\
    CMD(a, b, c, d, e, int, ShadingBlk_Width, lsc.sdblk_width, SDBLK_WIDTH,, REG_CMP_EQ(c, LOG_LSC_EN, 1), CAM_LSC_CTL2, LSC_SDBLK_WIDTH, 0)\
    CMD(a, b, c, d, e, int, ShadingBlk_Height, lsc.sdblk_height, SDBLK_HEIGHT,, REG_CMP_EQ(c, LOG_LSC_EN, 1), CAM_LSC_CTL3, LSC_SDBLK_HEIGHT, 0)\
    CMD(a, b, c, d, e, int, ShadingBlk_LastWidth, lsc.sdblk_last_width, SDBLK_LWIDTH,, REG_CMP_EQ(c, LOG_LSC_EN, 1), CAM_LSC_LBLOCK, LSC_SDBLK_lWIDTH, 0)\
    CMD(a, b, c, d, e, int, ShadingBlk_LastHeight, lsc.sdblk_last_height, SDBLK_LHEIGHT,, REG_CMP_EQ(c, LOG_LSC_EN, 1), CAM_LSC_LBLOCK, LSC_SDBLK_lHEIGHT, 0)\
    /* SL2 */\
    CMD(a, b, c, d, e, int, SL2_HRZ_COMP, sl2.sl2_hrz_comp, SL2_HRZ_COMP,, REG_CMP_EQ(c, LOG_SL2_EN, 1), CAM_SL2_HRZ, SL2_HRZ_COMP, 4)\
    /* CFA */\
    CMD(a, b, c, d, e, int, DM_BYP, cfa.bayer_bypass, BAYER_BYPASS,, REG_CMP_EQ(c, LOG_CFA_EN, 1), CAM_DM_O_BYP, DM_BYP, 4)\
    CMD(a, b, c, d, e, int, DM_FG_MODE, cfa.dm_fg_mode, DM_FG_MODE,, REG_CMP_EQ(c, LOG_CFA_EN, 1), CAM_DM_O_BYP, DM_FG_MODE, 4)\
    /* G2C */\
    CMD(a, b, c, d, e, int, G2C_SHADE_EN, g2c.g2c_shade_en, G2C_SHADE_EN,, REG_CMP_EQ(c, LOG_G2C_EN, 1), CAM_G2C_SHADE_CON_1, G2C_SHADE_EN, 4)\
    /* NSL2A */\
    CMD(a, b, c, d, e, int, NSL2A_HRZ_COMP, nsl2a.nsl2a_hrz_comp, NSL2A_HRZ_COMP,, REG_CMP_EQ(c, LOG_NSL2A_EN, 1), CAM_NSL2A_RZ, SL2_HRZ_COMP, 4)\
    CMD(a, b, c, d, e, int, NSL2A_VRZ_COMP, nsl2a.nsl2a_vrz_comp, NSL2A_VRZ_COMP,, REG_CMP_EQ(c, LOG_NSL2A_EN, 1), CAM_NSL2A_RZ, SL2_VRZ_COMP, 4)\
    /* NBC */\
    CMD(a, b, c, d, e, int, ANR1_ENY, nbc.anr_eny, ANR_ENY,, REG_CMP_EQ(c, LOG_NBC_EN, 1), CAM_ANR_CON1, ANR1_ENY, 4)\
    CMD(a, b, c, d, e, int, ANR1_ENC, nbc.anr_enc, ANR_ENC,, REG_CMP_EQ(c, LOG_NBC_EN, 1), CAM_ANR_CON1, ANR1_ENC, 4)\
    CMD(a, b, c, d, e, int, ANR1_SCALE_MODE, nbc.anr_scale_mode, ANR_SCALE_MODE,, REG_CMP_EQ(c, LOG_NBC_EN, 1), CAM_ANR_CON1, ANR1_SCALE_MODE, 4)\
    /* SL2C */\
    CMD(a, b, c, d, e, int, SL2C_HRZ_COMP, sl2c.sl2c_hrz_comp, SL2C_HRZ_COMP,, REG_CMP_EQ(c, LOG_SL2C_EN, 1), CAM_SL2C_HRZ, SL2C_HRZ_COMP, 4)\
    /* SEEE */\
    CMD(a, b, c, d, e, int, SEEE_OUT_EDGE_SEL, seee.se_edge, SE_EDGE,, REG_CMP_EQ(c, LOG_SEEE_EN, 1), CAM_SEEE_OUT_EDGE_CTRL, SEEE_OUT_EDGE_SEL, 4)\
    /* CDRZ */\
    CMD(a, b, c, d, e, int, CRZ_crop_size_x_d, cdrz.cdrz_input_crop_width, CDRZ_Input_Image_W,, REG_CMP_EQ(c, LOG_CDRZ_EN, 1), CAM_CRZ_IN_IMG, CRZ_IN_WD, 0)\
    CMD(a, b, c, d, e, int, CRZ_crop_size_y_d, cdrz.cdrz_input_crop_height, CDRZ_Input_Image_H,, REG_CMP_EQ(c, LOG_CDRZ_EN, 1), CAM_CRZ_IN_IMG, CRZ_IN_HT, 0)\
    CMD(a, b, c, d, e, int, CDRZ_Output_Image_W, cdrz.cdrz_output_width, CDRZ_Output_Image_W,, REG_CMP_EQ(c, LOG_CDRZ_EN, 1), CAM_CRZ_OUT_IMG, CRZ_OUT_WD, 0)\
    CMD(a, b, c, d, e, int, CDRZ_Output_Image_H, cdrz.cdrz_output_height, CDRZ_Output_Image_H,, REG_CMP_EQ(c, LOG_CDRZ_EN, 1), CAM_CRZ_OUT_IMG, CRZ_OUT_HT, 0)\
    CMD(a, b, c, d, e, int, CRZ_Bias_x_d, cdrz.cdrz_luma_horizontal_integer_offset, CDRZ_Luma_Horizontal_Integer_Offset,, REG_CMP_EQ(c, LOG_CDRZ_EN, 1), CAM_CRZ_LUMA_HORI_INT_OFST, CRZ_LUMA_HORI_INT_OFST, 0)\
    CMD(a, b, c, d, e, int, CRZ_offset_x_d, cdrz.cdrz_luma_horizontal_subpixel_offset, CDRZ_Luma_Horizontal_Subpixel_Offset,, REG_CMP_EQ(c, LOG_CDRZ_EN, 1), CAM_CRZ_LUMA_HORI_SUB_OFST, CRZ_LUMA_HORI_SUB_OFST, 0)\
    CMD(a, b, c, d, e, int, CRZ_Bias_y_d, cdrz.cdrz_luma_vertical_integer_offset, CDRZ_Luma_Vertical_Integer_Offset,, REG_CMP_EQ(c, LOG_CDRZ_EN, 1), CAM_CRZ_LUMA_VERT_INT_OFST, CRZ_LUMA_VERT_INT_OFST, 0)\
    CMD(a, b, c, d, e, int, CRZ_offset_y_d, cdrz.cdrz_luma_vertical_subpixel_offset, CDRZ_Luma_Vertical_Subpixel_Offset,, REG_CMP_EQ(c, LOG_CDRZ_EN, 1), CAM_CRZ_LUMA_VERT_SUB_OFST, CRZ_LUMA_VERT_SUB_OFST, 0)\
    CMD(a, b, c, d, e, int, CDRZ_Horizontal_Luma_Algorithm, cdrz.cdrz_horizontal_luma_algorithm, CDRZ_Horizontal_Luma_Algorithm,, REG_CMP_EQ(c, LOG_CDRZ_EN, 1), CAM_CRZ_CONTROL, CRZ_HORI_ALGO, 4)\
    CMD(a, b, c, d, e, int, CDRZ_Vertical_Luma_Algorithm, cdrz.cdrz_vertical_luma_algorithm, CDRZ_Vertical_Luma_Algorithm,, REG_CMP_EQ(c, LOG_CDRZ_EN, 1), CAM_CRZ_CONTROL, CRZ_VERT_ALGO, 4)\
    CMD(a, b, c, d, e, int, CDRZ_Horizontal_Coeff_Step, cdrz.cdrz_horizontal_coeff_step, CDRZ_Horizontal_Coeff_Step,, REG_CMP_EQ(c, LOG_CDRZ_EN, 1), CAM_CRZ_HORI_STEP, CRZ_HORI_STEP, 4)\
    CMD(a, b, c, d, e, int, CDRZ_Vertical_Coeff_Step, cdrz.cdrz_vertical_coeff_step, CDRZ_Vertical_Coeff_Step,, REG_CMP_EQ(c, LOG_CDRZ_EN, 1), CAM_CRZ_VERT_STEP, CRZ_VERT_STEP, 4)\
    /* IMG2O */\
    CMD(a, b, c, d, e, int, IMG2O_STRIDE, img2o.img2o_stride, IMG2O_STRIDE,, REG_CMP_EQ(c, LOG_IMG2O_EN, 1), CAM_IMG2O_STRIDE, STRIDE, 4)\
    CMD(a, b, c, d, e, int, IMG2O_XOFFSET, img2o.img2o_xoffset, IMG2O_XOFFSET,, REG_CMP_EQ(c, LOG_IMG2O_EN, 1), CAM_IMG2O_CROP, XOFFSET, 0)\
    CMD(a, b, c, d, e, int, IMG2O_YOFFSET, img2o.img2o_yoffset, IMG2O_YOFFSET,, REG_CMP_EQ(c, LOG_IMG2O_EN, 1), CAM_IMG2O_CROP, YOFFSET, 0)\
    CMD(a, b, c, d, e, int, IMG2O_XSIZE, img2o.img2o_xsize, IMG2O_XSIZE,, REG_CMP_EQ(c, LOG_IMG2O_EN, 1), CAM_IMG2O_XSIZE, XSIZE, 0)\
    CMD(a, b, c, d, e, int, IMG2O_YSIZE, img2o.img2o_ysize, IMG2O_YSIZE,, REG_CMP_EQ(c, LOG_IMG2O_EN, 1), CAM_IMG2O_YSIZE, YSIZE, 0)\

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
    CMD(a, b, c, d, e, int, TDRI_BASE_ADDR,, TDRI_BASE_ADDR,,,,,)\

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
    /* LSCI */\
    CMD(a, b, c, d, e, int,,, LSCI_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, LSCI_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, LSCI_TILE_YSIZE,,,,,)\
    /* LSC */\
    CMD(a, b, c, d, e, int,,, LSC_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, LSC_TILE_YSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, LSC_TILE_XOFST,,,,,)\
    CMD(a, b, c, d, e, int,,, LSC_TILE_YOFST,,,,,)\
    CMD(a, b, c, d, e, int,,, LSC_XNUM,,,,,)\
    CMD(a, b, c, d, e, int,,, LSC_YNUM,,,,,)\
    CMD(a, b, c, d, e, int,,, LSC_LWIDTH,,,,,)\
    CMD(a, b, c, d, e, int,,, LSC_LHEIGHT,,,,,)\
    CMD(a, b, c, d, e, int,,, LSC_TILE_FULL_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, LSC_TILE_FULL_YSIZE,,,,,)\
    /* SL2 */\
    CMD(a, b, c, d, e, int,,, SL2_TILE_XOFF,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2_TILE_YOFF,,,,,)\
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
    /* MFB */\
    CMD(a, b, c, d, e, int,,, MFB_TILE_XDIST,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_TILE_YDIST,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_TILE_OFFSET_X,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_TILE_OUTPUT_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, MFB_TILE_OUTPUT_YSIZE,,,,,)\
    /* MFBO */\
    CMD(a, b, c, d, e, int,,, MFBO_TILE_OFFSET_ADDR,,,,,)\
    CMD(a, b, c, d, e, int,,, MFBO_TILE_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, MFBO_TILE_YSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, MFBO_TILE_XOFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, MFBO_TILE_YOFFSET,,,,,)\
    CMD(a, b, c, d, e, int,,, MFBO_TILE_FULL_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, MFBO_TILE_FULL_YSIZE,,,,,)\
    /* G2C */\
    CMD(a, b, c, d, e, int,,, G2C_TILE_SHADE_XMID,,,,,)\
    CMD(a, b, c, d, e, int,,, G2C_TILE_SHADE_YMID,,,,,)\
    CMD(a, b, c, d, e, int,,, G2C_TILE_SHADE_XSP,,,,,)\
    CMD(a, b, c, d, e, int,,, G2C_TILE_SHADE_YSP,,,,,)\
    CMD(a, b, c, d, e, int,,, G2C_TILE_SHADE_VAR,,,,,)\
    /* NSL2A */\
    CMD(a, b, c, d, e, int,,, NSL2A_TILE_IN_XOFF,,,,,)\
    CMD(a, b, c, d, e, int,,, NSL2A_TILE_IN_YOFF,,,,,)\
    CMD(a, b, c, d, e, int,,, NSL2A_TILE_IN_WD,,,,,)\
    CMD(a, b, c, d, e, int,,, NSL2A_TILE_IN_HT,,,,,)\
    CMD(a, b, c, d, e, int,,, NSL2A_TILE_OUT_XOFF,,,,,)\
    CMD(a, b, c, d, e, int,,, NSL2A_TILE_OUT_YOFF,,,,,)\
    CMD(a, b, c, d, e, int,,, NSL2A_TILE_OUT_WD,,,,,)\
    CMD(a, b, c, d, e, int,,, NSL2A_TILE_OUT_HT,,,,,)\
    /* SL2C */\
    CMD(a, b, c, d, e, int,,, SL2C_TILE_XOFF,,,,,)\
    CMD(a, b, c, d, e, int,,, SL2C_TILE_YOFF,,,,,)\
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
    /* NR3D */\
    CMD(a, b, c, d, e, int,,, NR3D_TILE_FBCNT_XOFF,,,,,)\
    CMD(a, b, c, d, e, int,,, NR3D_TILE_FBCNT_YOFF,,,,,)\
    CMD(a, b, c, d, e, int,,, NR3D_TILE_FBCNT_XSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, NR3D_TILE_FBCNT_YSIZE,,,,,)\
    CMD(a, b, c, d, e, int,,, NR3D_TILE_ON_OFST_X,,,,,)\
    CMD(a, b, c, d, e, int,,, NR3D_TILE_ON_OFST_Y,,,,,)\
    CMD(a, b, c, d, e, int,,, NR3D_TILE_ON_WD,,,,,)\
    CMD(a, b, c, d, e, int,,, NR3D_TILE_ON_HT,,,,,)\
	/* CRSP */\
    CMD(a, b, c, d, e, int,,, CRSP_TILE_WD,,,,,)\
    CMD(a, b, c, d, e, int,,, CRSP_TILE_OFST_X,,,,,)\
    CMD(a, b, c, d, e, int,,, CRSP_TILE_HT,,,,,)\
    CMD(a, b, c, d, e, int,,, CRSP_TILE_OFST_Y,,,,,)\
    CMD(a, b, c, d, e, int,,, CRSP_TILE_CROP_XSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, CRSP_TILE_CROP_XEND,,,,,)\
    CMD(a, b, c, d, e, int,,, CRSP_TILE_CROP_YSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, CRSP_TILE_CROP_YEND,,,,,)\
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
    /* FE */\
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
    /* Internal */\
    CMD(a, b, c, d, e, int,,, CTRL_LCEI_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_UFDI_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_VIPI_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_VIP2I_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_VIP3I_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_FEO_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_IMG3O_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_IMG3BO_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_IMG3CO_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_IMG2O_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_FE_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_CDRZ_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_MDP_CROP_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_LCE_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SRZ1_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SRZ2_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_MIX1_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_PCA_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_MIX2_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SL2C_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_SEEE_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_MIX3_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_NR3D_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_CRSP_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_C24B_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_C02_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_MFBO_EN,,,,,)\
    /* MDP_CROP_EN */\
    CMD(a, b, c, d, e, int,,, CTRL_MDP_XSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_MDP_XEND,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_MDP_YSTART,,,,,)\
    CMD(a, b, c, d, e, int,,, CTRL_MDP_YEND,,,,,)\

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
    CMD(a, b, c, d, e, bool,,, nr3d_skip_pixel,,,,,)/* check skip internal pixel*/\
    CMD(a, b, c, d, e, bool,,, nr3d_skip_tile,,,,,)/* check skip boundary pixel */\
    CMD(a, b, c, d, e, int,,, nr3d_edge_flag,,,,,)/* buffer to record nr3d tile boundary*/\
    /* Input Data Type */\
    CMD(a, b, c, d, e, ISP_TILE_IN_DATA_TYPE_ENUM,,, IN_DATA_TYPE,,,,,)\
    /* TCM_ENBALE updated by tile_xxx_tdr() */\
    CMD(a, b, c, d, e, int,,, TCM_IMGI_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, TCM_UFDI_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, TCM_UNP_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, TCM_UFD_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, TCM_LSCI_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, TCM_LSC_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, TCM_SL2_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, TCM_VIPI_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, TCM_VIP2I_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, TCM_VIP3I_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, TCM_MFB_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, TCM_MFBO_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, TCM_G2C_SHADE_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, TCM_NSL2A_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, TCM_NBC_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, TCM_SL2C_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, TCM_BNR_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, TCM_CDRZ_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, TCM_IMG2O_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, TCM_LCEI_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, TCM_LCE_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, TCM_NR3D_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, TCM_CRSP_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, TCM_IMG3O_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, TCM_IMG3BO_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, TCM_IMG3CO_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, TCM_SRZ1_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, TCM_SRZ2_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, TCM_FE_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, TCM_FEO_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, TCM_C02_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, TCM_C02_CROP_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, TCM_MDP_CROP_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_IMGI_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_UFDI_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_UNP_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_UFD_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_BNR_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_DBS_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_LSCI_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_LSC_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_SL2_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_CFA_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_C24_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_VIPI_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_VIP2I_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_VIP3I_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_MFB_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_MFBO_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_CDRZ_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_IMG2O_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_C42_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_G2C_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_NSL2A_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_NBC_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_MIX1_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_PCA_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_MIX2_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_SL2C_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_SEEE_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_LCEI_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_LCE_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_MIX3_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_SRZ1_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_FE_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_FEO_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_C02_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_NR3D_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_CRSP_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_IMG3O_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_IMG3BO_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_IMG3CO_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_SRZ2_EN,,,,,)\
    CMD(a, b, c, d, e, int,,, LOG_C24B_EN,,,,,)\

#endif
