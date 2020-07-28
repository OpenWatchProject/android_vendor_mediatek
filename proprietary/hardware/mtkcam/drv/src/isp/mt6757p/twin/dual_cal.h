#ifndef __DUAL_CAL_H__
#define __DUAL_CAL_H__

#define DUAL_PLATFORM_DRIVER /* to change define with platform unique */

#if defined(_MSC_VER)
#include <conio.h>
#define dual_kbhit() _kbhit()
#define dual_stricmp                                             _stricmp
#define DUAL_FOLDER_SYMBOL_STR                                       "\\"
#define DUAL_FOLDER_SYMBOL_CHAR                                      '\\'
#define dual_sprintf(dst_ptr, size_dst, format, ...)             sprintf_s(dst_ptr, size_dst, format, __VA_ARGS__)
#define dual_fopen(file_ptr, filename_ptr, mode)                 fopen_s(&file_ptr, filename_ptr, mode)
#define dual_fscanf                                              fscanf_s
#define DUAL_MOVE_CMD                                                "move"
#define DUAL_COPY_CMD                                                "copy"
#define DUAL_CMP_CMD                                                 "fc"
#define DUAL_DEL_CMD                                                 "del"
#define dual_sscanf_1(...)                               sscanf_s(__VA_ARGS__)
#define dual_sscanf_2(...)							sscanf_s(__VA_ARGS__)
#define dual_sscanf_8(...)							sscanf_s(__VA_ARGS__)
#define DUAL_DEFAULT_PATH ".\\dual_conf"
#elif defined(linux) || defined(__linux) || defined(__arm__)
#define dual_kbhit() true
#define dual_stricmp                                             strcasecmp
#define DUAL_FOLDER_SYMBOL_STR                                       "/"
#define DUAL_FOLDER_SYMBOL_CHAR                                      '/'
#define dual_sprintf(dst_ptr, size_dst, ...)                     sprintf(dst_ptr, __VA_ARGS__)
#define dual_fopen(file_ptr, filename_ptr, mode)                 file_ptr = fopen(filename_ptr, mode)
#define dual_fscanf                                              fscanf
#define DUAL_MOVE_CMD                                                "mv"
#define DUAL_COPY_CMD                                                "cp"
#define DUAL_CMP_CMD                                                 "cmp"
#define DUAL_DEL_CMD                                                 "rm"
#define dual_sscanf_1(x, y, a, b)                               sscanf(x, y, a)
#define dual_sscanf_2(x, y, a, b, c, d)		sscanf(x, y, a, b)
#define dual_sscanf_8(x, y, a, b, c, d, e, f, g, h,...)		sscanf(x, y, a, b, c, d, e, f, g, h)
#define O_BINARY (0)
#define DUAL_DEFAULT_PATH "./dual_conf"
#else
#error("Non-supported c compiler environment\n")
#endif

#ifdef DUAL_PLATFORM_DRIVER
#if defined(_MSC_VER)           // VC
#define dual_driver_printf dual_printf_prefix
#define dual_driver_printf_no_prefix printf
#define uart_printf printf
#elif defined(linux) || defined(__linux) //Linux
#if defined(__arm__) //arm
#define dual_driver_printf(...) {ALOGE( "[%s][%d]",__FUNCTION__,__LINE__);ALOGD( __VA_ARGS__);}
#define dual_driver_printf_no_prefix(...) {ALOGE( "[%s][%d]",__FUNCTION__,__LINE__);ALOGD( __VA_ARGS__);}
#define NEW_LINE_CHAR   "\n"
#if defined(USING_MTK_LDVT)     // LDVT
#pragma message("LDVT environment")
#include "uvvf.h"
#define uart_printf(fmt, arg...) VV_MSG(LOG_TAG "[%s] " fmt NEW_LINE_CHAR, __func__, ##arg)
#else //not LDVT
#pragma message("FPGA/EVB/Phone environment")
#include <cutils/log.h>        // Android
#define uart_printf(fmt, arg...) ALOGE("[%s]" fmt"\n", __FUNCTION__, ##arg)
#endif //END LDVT
#else //not arm
#define dual_driver_printf dual_printf_prefix
#define dual_driver_printf_no_prefix printf
#define uart_printf printf
#endif //END platform
#endif //END VC
#else
#define dual_driver_printf dual_printf_prefix
#define dual_driver_printf_no_prefix printf
#endif

/* common define */
#ifndef __cplusplus
#ifndef bool
#define bool unsigned char
#endif
#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif
#endif

#define TWIN_SCENARIO_NORMAL (0x0)
#define TWIN_SCENARIO_AF_FAST_P1 (0x1)
#define TWIN_SCENARIO_AF_FAST_P2 (0x2)

#define DUAL_MAX_COMMAND_LENGTH (512)
#define DUAL_MAX_FILENAME_LENGTH (128)

#define DUAL_RAL_KEY_HEAD1  "RAL:"
#define DUAL_RAL_KEY_HEAD2  "# bits"
#define DUAL_VAR_KEY_HEAD  "VAR:"
#define DUAL_KNOBS_CMP_FILENAME "dual_knobs_cmp.txt"
#define DUAL_ISP_REG_CMP_FILENAME "dual_isp_reg_cmp.txt"
#define DUAL_IN_CONFIG_FILENAME "dual_in_config_"
#define DUAL_IN_CONFIG_EXTNAME ".txt"
#define DUAL_ENGINE_CONFIG_FILENAME "dual_eng_config_"
#define DUAL_ENIGNE_CONFIG_EXTNAME ".txt"
#define DUAL_OUT_CONFIG_FILENAME "dual_out_config_"
#define DUAL_OUT_CONFIG_EXTNAME ".txt"
#define DUAL_REG_DUMP_FILENAME "dual_reg_dump.txt"
#define DUAL_REG_CMP_FILENAME "dual_reg_diff.txt"

#define DUAL_MODE_CAL(dual_sel) (((dual_sel & 0x1)?1:0) + ((dual_sel & 0x2)?1:0) + ((dual_sel & 0x4)?1:0) + ((dual_sel & 0x8)?1:0))
#define DUAL_MOD(num, denom) (((denom)==1)?0:(((denom)==2)?((num)&0x1):(((denom)==4)?\
	((num)&0x3):(((denom)==8)?((num)&0x7):((num)%(denom))))))
#define DUAL_INT_DIV(num, denom) (((denom)==1)?(num):(((denom)==2)?((unsigned int)(num)>>0x1):\
	(((denom)==4)?((unsigned int)(num)>>0x2):(((denom)==8)?((unsigned int)(num)>>0x3):((num)/(denom))))))

#define DUAL_RAW_WD (6804)
#define DUAL_RAW_WD_D (6804)
#define DUAL_RRZ_WD (4608)
#define DUAL_RRZ_WD_D (4608)
#define DUAL_AF_TAPS (12) /* must be even */
#define DUAL_AF_MAX_BLOCK_WIDTH (80) /* must be even */
#define DUAL_AF_FAST_MARGIN (128) /* must be even */
#define DUAL_AF_FAST_MARGIN_L (32) /* must be even */

/* macro for debug print to file, #include <stdio.h> */
#define dual_printf_prefix(...) {\
    const char *ptr_char = get_current_file_name(__FILE__);\
    printf("[%s][%s][%d] ", ptr_char,  __FUNCTION__, __LINE__);\
    printf(__VA_ARGS__);\
}

#define DUAL_MAX_NAME_SIZE (128)
#define DUAL_MAX_REG_NO (40000)
#define DUAL_MAX_VAR_NO (2048)
#define DUAL_STR_SIZE (256)
#define DUAL_MAX_DUMP_COLUMN_LENGTH (512)
#define DUAL_SPACE_EQUAL_SYMBOL_STR " = "

#define DUAL_ERROR_MESSAGE_DATA(n, CMD) \
    CMD(n, DUAL_MESSAGE_OK)\
    CMD(n, DUAL_MESSAGE_FILE_OPEN_ERROR)\
    CMD(n, DUAL_MESSAGE_NULL_PTR_ERROR)\
    CMD(n, DUAL_MESSAGE_MISSING_READ_REG_ERROR)\
    CMD(n, DUAL_MESSAGE_INVALID_CONFIG_ERROR)\
    CMD(n, DUAL_MESSAGE_RRZ_SCALING_UP_ERROR)\
	CMD(n, DUAL_MESSAGE_RRZ_XS_XE_CAL_ERROR)\
	CMD(n, DUAL_MESSAGE_RAL_NO_OVER_SIZE_ERROR)\
	CMD(n, DUAL_MESSAGE_VAR_NO_OVER_SIZE_ERROR)\
	CMD(n, DUAL_MESSAGE_RAL_MASK_CHECK_ERROR)\
    CMD(n, DUAL_MESSAGE_INVALID_RRZ_IN_CROP_HT_ERROR)\
    CMD(n, DUAL_MESSAGE_INVALID_DUAL_SEL_ERROR)\
    CMD(n, DUAL_MESSAGE_INVALID_LE_INV_CTL_ERROR)\
    CMD(n, DUAL_MESSAGE_INVALID_AF_BLK_XSIZE_ERROR)\
    CMD(n, DUAL_MESSAGE_INVALID_AF_BLK_NUM_ERROR)\
    CMD(n, DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR)\
    CMD(n, DUAL_MESSAGE_INVALID_AF_SGG_CONFIG_ERROR)\
    CMD(n, DUAL_MESSAGE_INVALID_PIX_BUS_DMXI_ERROR)\
    CMD(n, DUAL_MESSAGE_INVALID_PIX_BUS_DMXO_ERROR)\
    CMD(n, DUAL_MESSAGE_INVALID_LSCI_STRIDE_ERROR)\
    CMD(n, DUAL_MESSAGE_DMX_IN_ALIGN_ERROR)\
    CMD(n, DUAL_MESSAGE_DMX_OUT_ALIGN_ERROR)\
    CMD(n, DUAL_MESSAGE_BIN_OUT_ALIGN_ERROR)\
    CMD(n, DUAL_MESSAGE_BMX_OUT_ALIGN_ERROR)\
    CMD(n, DUAL_MESSAGE_RMX_OUT_ALIGN_ERROR)\
    CMD(n, DUAL_MESSAGE_DMX_SIZE_CAL_ERROR)\
    CMD(n, DUAL_MESSAGE_BMX_SIZE_CAL_ERROR)\
    CMD(n, DUAL_MESSAGE_RCP3_SIZE_CAL_ERROR)\
    CMD(n, DUAL_MESSAGE_RMX_SIZE_CAL_ERROR)\
    CMD(n, DUAL_MESSAGE_ZERO_SL_HVRZ_COMP_ERROR)\
    CMD(n, DUAL_MESSAGE_AF_FAST_MODE_CONFIG_ERROR)\

#define DUAL_ENUM_STRING(n, a) if ((a) == (n)) return #a;
#define DUAL_ENUM_DECLARE(n, a) a,

/* to prevent from directly calling macro */
#define TWIN_GET_ERROR_NAME(n) \
    if (0 == (n)) return "DUAL_MESSAGE_UNKNOWN";\
    DUAL_ERROR_MESSAGE_DATA(n, DUAL_ENUM_STRING)\
    return "";

/* error enum */
typedef enum DUAL_MESSAGE_ENUM
{
    DUAL_MESSAGE_UNKNOWN=0,
    DUAL_ERROR_MESSAGE_DATA(,DUAL_ENUM_DECLARE)
}DUAL_MESSAGE_ENUM;

#define DUAL_RRZ_PREC (32768)
/* tile loss */
#define DUAL_TILE_LOSS_DBS (2)
#define DUAL_TILE_LOSS_BNR (4)
#define DUAL_TILE_LOSS_RMM (2)
#define DUAL_TILE_LOSS_CAC (6)
#define DUAL_TILE_LOSS_ALL (DUAL_TILE_LOSS_DBS + DUAL_TILE_LOSS_BNR + DUAL_TILE_LOSS_RMM + DUAL_TILE_LOSS_CAC)
#define DUAL_TILE_LOSS_ALL_L (DUAL_TILE_LOSS_DBS + DUAL_TILE_LOSS_BNR + DUAL_TILE_LOSS_RMM + DUAL_TILE_LOSS_CAC)
#define DUAL_TILE_LOSS_RRZ (8)
#define DUAL_TILE_LOSS_RRZ_L (6)
#define DUAL_TILE_LOSS_RRZ_TAP (5) /* 6 tp: 4, 8 tp: 5 */
#define DUAL_TILE_LOSS_RRZ_TAP_L (3) /* 6 tp: 2, 8 tp: 3 */
#define DUAL_ALIGN_PIXEL_MODE(n) (1<<(n))
#define DUAL_ZHDR_ALIGN_PIXEL (0x4)
#define DUAL_IHDR_LE_INV (0x1)
#define DUAL_ZHDR_LE_INV_R (0x2)
#define DUAL_ZHDR_LE_INV_G (0x4)
#define DUAL_ZHDR_LE_INV_B (0x8)
#define DUAL_RRZ_ALIGN(n) (2<<(n))
#define DUAL_RRZ_MASK(n) ((2<<(n)) - 1)

/* RAW format */
#define DUAL_OUT_FMT_BAYER_8      (8)
#define DUAL_OUT_FMT_BAYER_10     (9)
#define DUAL_OUT_FMT_BAYER_12     (10)
#define DUAL_OUT_FMT_BAYER_14	    (11)
#define DUAL_OUT_FMT_BAYER_8_2	    (12)
#define DUAL_OUT_FMT_BAYER_10_2	    (13)
#define DUAL_OUT_FMT_BAYER_12_2	    (14)
#define DUAL_OUT_FMT_BAYER_14_2	    (15)
#define DUAL_OUT_FMT_MIPI_10	    (16)
#define DUAL_OUT_FMT_FG_8      (0)
#define DUAL_OUT_FMT_FG_10     (1)
#define DUAL_OUT_FMT_FG_12     (2)
#define DUAL_OUT_FMT_FG_14     (3)

/* PLATFORM parameters */
/* a, b, c, d, e, f reserved */
/* support mode */
#define DUAL_SEL_SUPPORT_LIST(CMD, a, b, c, d, e, f) \
	/* LSC */\
	CMD(a, b, c, d, e, f, 0x3)/* LSB:110|0:MSB */\

/* PLATFORM parameters */
/* a, b, c, d, e, f reserved */
/* struct name */
/* variable name */
/* R/W type: R, RW, W */
#define DUAL_DEBUG_LIST(CMD, a, b, c, d, e, f) \
	DUAL_DEBUG_LIST_R(CMD, a, b, c, d, e, f) \

#define DUAL_DEBUG_LIST_R(CMD, a, b, c, d, e, f) \
	CMD(a, b, c, d, e, f, DEBUG, DUAL_LOG_EN)\
	CMD(a, b, c, d, e, f, DEBUG, DUAL_LOG_ID)\

/* PLATFORM parameters */
/* SW.AAA: dual config, BBB: knobs VAR, CCC: c model variable */
/* R/W type: R, RW, W */
#define DUAL_SW_LIST(CMD, a, b, c, d, e, f) \
	CMD(a, b, c, d, e, f, SW, DUAL_SEL, DUAL_SEL, DUAL_SEL)\
	/* LSC */\
	CMD(a, b, c, d, e, f, SW, TWIN_MODE_SDBLK_XNUM_ALL, TWIN_MODE_SDBLK_XNUM_ALL, TWIN_MODE_SDBLK_XNUM_ALL)\
	CMD(a, b, c, d, e, f, SW, TWIN_MODE_SDBLK_lWIDTH_ALL, TWIN_MODE_SDBLK_lWIDTH_ALL, TWIN_MODE_SDBLK_lWIDTH_ALL)\
	/* RRZ DZ */\
	CMD(a, b, c, d, e, f, SW, TWIN_RRZ_IN_CROP_HT, TWIN_RRZ_IN_CROP_HT, TWIN_RRZ_IN_CROP_HT)\
	CMD(a, b, c, d, e, f, SW, TWIN_RRZ_HORI_INT_OFST, TWIN_RRZ_HORI_INT_OFST, TWIN_RRZ_HORI_INT_OFST)\
	CMD(a, b, c, d, e, f, SW, TWIN_RRZ_HORI_INT_OFST_LAST, TWIN_RRZ_HORI_INT_OFST_LAST, TWIN_RRZ_HORI_INT_OFST_LAST)\
	CMD(a, b, c, d, e, f, SW, TWIN_RRZ_HORI_SUB_OFST, TWIN_RRZ_HORI_SUB_OFST, TWIN_RRZ_HORI_SUB_OFST)\
	CMD(a, b, c, d, e, f, SW, TWIN_RRZ_OUT_WD, cam_a_rrz_h_size, TWIN_RRZ_OUT_WD)\
	/* AF */\
	CMD(a, b, c, d, e, f, SW, TWIN_AF_OFFSET, TWIN_AF_OFFSET, TWIN_AF_OFFSET)\
	CMD(a, b, c, d, e, f, SW, TWIN_AF_BLOCK_XNUM, TWIN_AF_BLOCK_XNUM, TWIN_AF_BLOCK_XNUM)\
	/* RAWI */\
	CMD(a, b, c, d, e, f, SW, TWIN_RAWI_XSIZE, cam_a_rawi_h_size, TWIN_RAWI_XSIZE)\
	/* SCRNARIO */\
	CMD(a, b, c, d, e, f, SW, TWIN_SCENARIO, TWIN_SCENARIO, TWIN_SCENARIO)\

/* PLATFORM parameters */
/* AAA: reg, BBB: field, CCC: c model variable */
/* R/W type: R, RW, W */
#define DUAL_HW_UNI_LIST(CMD, a, b, c, d, e, f) \
	CMD(a, b, c, d, e, f, CAM_UNI_RAWI_YSIZE, YSIZE, RAWI_A_YSIZE, U)\
	CMD(a, b, c, d, e, f, CAM_UNI_TOP_FMT_SEL, PIX_BUS_RAWI, PIX_BUS_RAWI_A, U)\
	CMD(a, b, c, d, e, f, CAM_UNI_TOP_MOD_EN, UNP2_A_EN, UNP2_A_EN, U)\

/* PLATFORM parameters */
/* a, b, c, d, e, f reserved */
/* struct name */
/* variable name */
/* R/W type: R, RW, W */
#define DUAL_HW_A_LIST(CMD, a, b, c, d, e, f) \
	/* SEL */\
	CMD(a, b, c, d, e, f, CAM_A_CTL_SEL, DMX_SEL, DMX_A_SEL, U)\
	CMD(a, b, c, d, e, f, CAM_A_CTL_SEL, PMX_SEL, PMX_A_SEL, U)\
	CMD(a, b, c, d, e, f, CAM_A_CTL_SEL, SGG_SEL, SGG_A_SEL, U)\
	/* TG */\
	CMD(a, b, c, d, e, f, CAM_A_TG_SEN_GRAB_PXL, PXL_S, PIX_S, U)\
	CMD(a, b, c, d, e, f, CAM_A_TG_SEN_GRAB_PXL, PXL_E, PIX_E, U)\
	CMD(a, b, c, d, e, f, CAM_A_TG_SEN_GRAB_LIN, LIN_S, LIN_S, U)\
	CMD(a, b, c, d, e, f, CAM_A_TG_SEN_GRAB_LIN, LIN_E, LIN_E, U)\
	/* PIXEL MODE */\
    CMD(a, b, c, d, e, f, CAM_A_CTL_FMT_SEL, PIX_BUS_RMXO, PIX_BUS_RMXO_A, U)\
    CMD(a, b, c, d, e, f, CAM_A_CTL_FMT_SEL, PIX_BUS_BMXO, PIX_BUS_BMXO_A, U)\
    CMD(a, b, c, d, e, f, CAM_A_CTL_FMT_SEL, PIX_BUS_DMXO, PIX_BUS_DMXO_A, U)\
    CMD(a, b, c, d, e, f, CAM_A_CTL_FMT_SEL, PIX_BUS_DMXI, PIX_BUS_DMXI_A, U)\
	/* RCP3_A */\
    CMD(a, b, c, d, e, f, CAM_A_CTL_EN, RCP3_EN, RCP3_A_EN, U)\
    CMD(a, b, c, d, e, f, CAM_A_CTL_SEL, RCP3_SEL, RCP3_A_SEL, U)\
    CMD(a, b, c, d, e, f, CAM_A_RCP3_CROP_CON1, RCP_STR_X, RCP_tmp7_STR_X, U)\
    CMD(a, b, c, d, e, f, CAM_A_RCP3_CROP_CON1, RCP_END_X, RCP_tmp7_END_X, U)\
    CMD(a, b, c, d, e, f, CAM_A_RCP3_CROP_CON2, RCP_STR_Y, RCP_tmp7_STR_Y, U)\
    CMD(a, b, c, d, e, f, CAM_A_RCP3_CROP_CON2, RCP_END_Y, RCP_tmp7_END_Y, U)\
	/* IMGO */\
    CMD(a, b, c, d, e, f, CAM_A_CTL_EN, PAK_EN, PAK_A_EN, U)\
    CMD(a, b, c, d, e, f, CAM_A_CTL_EN, CPG_EN, CPG_A_EN, U)\
    CMD(a, b, c, d, e, f, CAM_A_CTL_SEL, IMG_SEL, IMG_A_SEL, U)\
    CMD(a, b, c, d, e, f, CAM_A_CTL_DMA_EN, IMGO_EN, IMGO_A_EN, U)\
    CMD(a, b, c, d, e, f, CAM_A_CTL_FMT_SEL, IMGO_FMT, IMGO_FMT_A, U)\
    CMD(a, b, c, d, e, f, CAM_A_IMGO_BASE_ADDR, BASE_ADDR, IMGO_A_BASE_ADDR, U)\
    CMD(a, b, c, d, e, f, CAM_A_IMGO_OFST_ADDR, OFFSET_ADDR, IMGO_A_OFFSET_ADDR, U)\
    CMD(a, b, c, d, e, f, CAM_A_IMGO_XSIZE, XSIZE, IMGO_A_XSIZE, U)\
    CMD(a, b, c, d, e, f, CAM_A_IMGO_YSIZE, YSIZE, IMGO_A_YSIZE, U)\
    CMD(a, b, c, d, e, f, CAM_A_IMGO_STRIDE, STRIDE, IMGO_A_STRIDE, U)\
    CMD(a, b, c, d, e, f, CAM_A_IMGO_CROP, XOFFSET, IMGO_A_XOFFSET, U)\
    CMD(a, b, c, d, e, f, CAM_A_IMGO_CROP, YOFFSET, IMGO_A_YOFFSET, U)\
	/* UFE */\
    CMD(a, b, c, d, e, f, CAM_A_CTL_SEL, UFE_SEL, UFE_A_SEL, U)\
	/* DMX */\
    CMD(a, b, c, d, e, f, CAM_A_CTL_EN, DMX_EN, DMX_A_EN, U)\
    CMD(a, b, c, d, e, f, CAM_A_DMX_CTL, DMX_EDGE, DMX_A_EDGE, U)\
    CMD(a, b, c, d, e, f, CAM_A_DMX_CROP, DMX_STR_X, DMX_A_STR_X, U)\
    CMD(a, b, c, d, e, f, CAM_A_DMX_CROP, DMX_END_X, DMX_A_END_X, U)\
    CMD(a, b, c, d, e, f, CAM_A_DMX_VSIZE, DMX_HT, DMX_A_HT, U)\
	/* BIN */\
    CMD(a, b, c, d, e, f, CAM_A_CTL_EN, BIN_EN, BIN_A_EN, U)\
	/* SL2F */\
    CMD(a, b, c, d, e, f, CAM_A_CTL_DMA_EN, SL2F_EN, SL2F_A_EN, U)\
    CMD(a, b, c, d, e, f, CAM_A_SL2F_RZ, SL2_HRZ_COMP, SL2F_A_HRZ_COMP, U)\
    CMD(a, b, c, d, e, f, CAM_A_SL2F_RZ, SL2_VRZ_COMP, SL2F_A_VRZ_COMP, U)\
    CMD(a, b, c, d, e, f, CAM_A_SL2F_XOFF, SL2_X_OFST, SL2F_A_X_OFST, U)\
    CMD(a, b, c, d, e, f, CAM_A_SL2F_YOFF, SL2_Y_OFST, SL2F_A_Y_OFST, U)\
    CMD(a, b, c, d, e, f, CAM_A_SL2F_SIZE, SL2_TPIPE_WD, SL2F_A_TPIPE_WD, U)\
    CMD(a, b, c, d, e, f, CAM_A_SL2F_SIZE, SL2_TPIPE_HT, SL2F_A_TPIPE_HT, U)\
	/* VBN */\
    CMD(a, b, c, d, e, f, CAM_A_CTL_DMA_EN, VBN_EN, VBN_A_EN, U)\
    CMD(a, b, c, d, e, f, CAM_A_VBN_TYPE, VBN_TYPE, VBN_tmp1_TYPE, U)\
    CMD(a, b, c, d, e, f, CAM_A_VBN_TYPE, VBN_DIAG_SEL_EN, VBN_tmp1_DIAG_SEL_EN, U)\
	/* SCM */\
    CMD(a, b, c, d, e, f, CAM_A_CTL_DMA_EN, SCM_EN, SCM_A_EN, U)\
    CMD(a, b, c, d, e, f, CAM_A_SCM_CFG1, SCM_LE_INV_CTL, SCM_tmp1_LE_INV_CTL, U)\
	/* DBN */\
    CMD(a, b, c, d, e, f, CAM_A_CTL_EN, DBN_EN, DBN_A_EN, U)\
	/* PBN */\
    CMD(a, b, c, d, e, f, CAM_A_CTL_EN, PBN_EN, PBN_A_EN, U)\
    CMD(a, b, c, d, e, f, CAM_A_PBN_TYPE, PBN_SEP, PBN_tmp1_SEP, U)\
	/* DBS */\
    CMD(a, b, c, d, e, f, CAM_A_CTL_EN, DBS_EN, DBS_A_EN, U)\
	/* LE_INV_CTL */\
    CMD(a, b, c, d, e, f, CAM_A_RMG_HDR_CFG, RMG_IHDR_EN, RMG_tmp1_IHDR_EN, U)\
    CMD(a, b, c, d, e, f, CAM_A_RMG_HDR_CFG, RMG_ZHDR_EN, RMG_tmp1_ZHDR_EN, U)\
    CMD(a, b, c, d, e, f, CAM_A_RMG_HDR_CFG2, RMG_HDR_THK, RMG_tmp1_THK, U)\
    CMD(a, b, c, d, e, f, CAM_A_BIN_CTL, BIN_LE_INV_CTL, BIN_tmp1_LE_INV_CTL, U)\
    CMD(a, b, c, d, e, f, CAM_A_RMG_HDR_GAIN, RMG_LE_INV_CTL, RMG_tmp1_LE_INV_CTL, U)\
    CMD(a, b, c, d, e, f, CAM_A_RMM_OSC, RMM_LE_INV_CTL, RMM_tmp1_LE_INV_CTL, U)\
    CMD(a, b, c, d, e, f, CAM_A_BNR_BPC_CON, BNR_LE_INV_CTL, BNR_tmp1_LE_INV_CTL, U)\
    CMD(a, b, c, d, e, f, CAM_A_DBS_CTL, DBS_LE_INV_CTL, DBS_tmp1_LE_INV_CTL, U)\
	/* SGM */\
    CMD(a, b, c, d, e, f, CAM_A_CTL_EN, SGM_EN, SGM_A_EN, U)\
    /*CMD(a, b, c, d, e, f, CAM_A_SGM_LUT_R[0], SGM_R, SGM_R_A, U)*/\
	/* OBC */\
    CMD(a, b, c, d, e, f, CAM_A_CTL_EN, OBC_EN, OBC_A_EN, U)\
	/* RMG */\
    CMD(a, b, c, d, e, f, CAM_A_CTL_EN, RMG_EN, RMG_A_EN, U)\
	/* BPCI */\
    CMD(a, b, c, d, e, f, CAM_A_BPCI_XSIZE, XSIZE, BPCI_A_XSIZE, U)\
    CMD(a, b, c, d, e, f, CAM_A_BPCI_YSIZE, YSIZE, BPCI_A_YSIZE, U)\
    CMD(a, b, c, d, e, f, CAM_A_BPCI_STRIDE, STRIDE, BPCI_A_STRIDE, U)\
	/* BNR */\
    CMD(a, b, c, d, e, f, CAM_A_CTL_EN, BNR_EN, BNR_A_EN, U)\
    CMD(a, b, c, d, e, f, CAM_A_BNR_BPC_TBLI1, BPC_XOFFSET, BPC_tmp1_XOFFSET, U)\
    CMD(a, b, c, d, e, f, CAM_A_BNR_BPC_TBLI1, BPC_YOFFSET, BPC_tmp1_YOFFSET, U)\
    CMD(a, b, c, d, e, f, CAM_A_BNR_BPC_TBLI2, BPC_XSIZE, BPC_tmp1_XSIZE, U)\
    CMD(a, b, c, d, e, f, CAM_A_BNR_BPC_TBLI2, BPC_YSIZE, BPC_tmp1_YSIZE, U)\
    CMD(a, b, c, d, e, f, CAM_A_BNR_PDC_POS, PDC_XCENTER, BPC_tmp1_XCENTER, U)\
    CMD(a, b, c, d, e, f, CAM_A_BNR_PDC_POS, PDC_YCENTER, BPC_tmp1_YCENTER, U)\
    CMD(a, b, c, d, e, f, CAM_A_BNR_PDC_CON, PDC_OUT, PDC_tmp1_OUT, U)\
    CMD(a, b, c, d, e, f, CAM_A_BNR_PDC_CON, PDC_EN, PDC_tmp1_EN, U)\
	/* RMM */\
    CMD(a, b, c, d, e, f, CAM_A_CTL_EN, RMM_EN, RMM_A_EN, U)\
	/* CAC */\
    CMD(a, b, c, d, e, f, CAM_A_CTL_EN, CAC_EN, CAC_A_EN, U)\
	/* CACI */\
    CMD(a, b, c, d, e, f, CAM_A_CACI_STRIDE, STRIDE, CACI_A_EN, U)\
    CMD(a, b, c, d, e, f, CAM_A_CACI_OFST_ADDR, OFFSET_ADDR, CACI_A_OFFSET_ADDR, U)\
    CMD(a, b, c, d, e, f, CAM_A_CACI_XSIZE, XSIZE, CACI_A_XSIZE, U)\
	/* CAC */\
    CMD(a, b, c, d, e, f, CAM_A_CAC_TILE_OFFSET, CAC_OFFSET_X, CAC_tmp1_TPIPE_OFST_X, U)\
    CMD(a, b, c, d, e, f, CAM_A_CAC_TILE_SIZE, CAC_TILE_WIDTH, CAC_tmp1_FULL_SIZE_X, U)\
	/* LSC */\
    CMD(a, b, c, d, e, f, CAM_A_CTL_EN, LSC_EN, LSC_A_EN, U)\
    CMD(a, b, c, d, e, f, CAM_A_LSC_CTL1, LSC_EXTEND_COEF_MODE, LSC_tmp1_EXTEND_COEF_MODE, U)\
    CMD(a, b, c, d, e, f, CAM_A_LSC_CTL2, LSC_SDBLK_XNUM, LSC_tmp1_SDBLK_XNUM, U)\
    CMD(a, b, c, d, e, f, CAM_A_LSC_CTL2, LSC_SDBLK_WIDTH, LSC_tmp1_SDBLK_WIDTH, U)\
    CMD(a, b, c, d, e, f, CAM_A_LSC_LBLOCK, LSC_SDBLK_lWIDTH, LSC_tmp1_SDBLK_lWIDTH, U)\
    CMD(a, b, c, d, e, f, CAM_A_LSC_TPIPE_OFST, LSC_TPIPE_OFST_X, LSC_tmp1_TPIPE_OFST_X, U)\
    CMD(a, b, c, d, e, f, CAM_A_LSC_TPIPE_SIZE, LSC_TPIPE_SIZE_X, LSC_tmp1_TPIPE_SIZE_X, U)\
	/* LSCI */\
    CMD(a, b, c, d, e, f, CAM_A_LSCI_OFST_ADDR, OFFSET_ADDR, LSCI_A_OFFSET_ADDR, U)\
    CMD(a, b, c, d, e, f, CAM_A_LSCI_STRIDE, STRIDE, LSCI_A_STRIDE, U)\
    CMD(a, b, c, d, e, f, CAM_A_LSCI_XSIZE, XSIZE, LSCI_A_XSIZE, U)\
	/* RCR */\
    CMD(a, b, c, d, e, f, CAM_A_CTL_EN, RCP_EN, RCP_A_EN, U)\
    CMD(a, b, c, d, e, f, CAM_A_RCP_CROP_CON1, RCP_STR_X, RCP_A_STR_X, U)\
    CMD(a, b, c, d, e, f, CAM_A_RCP_CROP_CON1, RCP_END_X, RCP_A_END_X, U)\
    CMD(a, b, c, d, e, f, CAM_A_RCP_CROP_CON2, RCP_STR_Y, RCP_A_STR_Y, U)\
    CMD(a, b, c, d, e, f, CAM_A_RCP_CROP_CON2, RCP_END_Y, RCP_A_END_Y, U)\
	/* BMX */\
    CMD(a, b, c, d, e, f, CAM_A_CTL_EN, BMX_EN, BMX_A_EN, U)\
    CMD(a, b, c, d, e, f, CAM_A_BMX_CTL, BMX_SINGLE_MODE_1, BMX_A_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, CAM_A_BMX_CTL, BMX_SINGLE_MODE_2, BMX_A_SIG_MODE2, U)\
    CMD(a, b, c, d, e, f, CAM_A_BMX_CTL, BMX_EDGE, BMX_A_EDGE, U)\
    CMD(a, b, c, d, e, f, CAM_A_BMX_CROP, BMX_STR_X, BMX_A_STR_X, U)\
    CMD(a, b, c, d, e, f, CAM_A_BMX_CROP, BMX_END_X, BMX_A_END_X, U)\
    CMD(a, b, c, d, e, f, CAM_A_BMX_VSIZE, BMX_HT, BMX_A_HT, U)\
	/* AMX */\
    CMD(a, b, c, d, e, f, CAM_A_CTL_DMA_EN, AMX_EN, AMX_A_EN, U)\
    CMD(a, b, c, d, e, f, CAM_A_AMX_CTL, AMX_SINGLE_MODE_1, AMX_A_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, CAM_A_AMX_CTL, AMX_SINGLE_MODE_2, AMX_A_SIG_MODE2, U)\
    CMD(a, b, c, d, e, f, CAM_A_AMX_CTL, AMX_EDGE, AMX_A_EDGE, U)\
    CMD(a, b, c, d, e, f, CAM_A_AMX_CROP, AMX_STR_X, AMX_A_STR_X, U)\
    CMD(a, b, c, d, e, f, CAM_A_AMX_CROP, AMX_END_X, AMX_A_END_X, U)\
    CMD(a, b, c, d, e, f, CAM_A_AMX_VSIZE, AMX_HT, AMX_A_HT, U)\
	/* RPG */\
    CMD(a, b, c, d, e, f, CAM_A_CTL_EN, RPG_EN, RPG_A_EN, U)\
	/* PMX */\
    CMD(a, b, c, d, e, f, CAM_A_CTL_EN, PMX_EN, PMX_A_EN, U)\
    CMD(a, b, c, d, e, f, CAM_A_PMX_CTL, PMX_SINGLE_MODE_1, PMX_tmp1_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, CAM_A_PMX_CTL, PMX_SINGLE_MODE_2, PMX_tmp1_SIG_MODE2, U)\
    CMD(a, b, c, d, e, f, CAM_A_PMX_CTL, PMX_EDGE, PMX_tmp1_EDGE, U)\
    CMD(a, b, c, d, e, f, CAM_A_PMX_CROP, PMX_STR_X, PMX_tmp1_STR_X, U)\
    CMD(a, b, c, d, e, f, CAM_A_PMX_CROP, PMX_END_X, PMX_tmp1_END_X, U)\
    CMD(a, b, c, d, e, f, CAM_A_PMX_VSIZE, PMX_HT, PMX_tmp1_HT, U)\
	/* RRZ */\
    CMD(a, b, c, d, e, f, CAM_A_CTL_EN, RRZ_EN, RRZ_A_EN, U)\
    CMD(a, b, c, d, e, f, CAM_A_RRZ_VERT_STEP, RRZ_VERT_STEP, RRZ_tmp1_VERT_STEP, U)\
    CMD(a, b, c, d, e, f, CAM_A_RRZ_VERT_INT_OFST, RRZ_VERT_INT_OFST, RRZ_tmp1_VERT_INT_OFST, U)\
    CMD(a, b, c, d, e, f, CAM_A_RRZ_VERT_SUB_OFST, RRZ_VERT_SUB_OFST, RRZ_tmp1_VERT_SUB_OFST, U)\
    CMD(a, b, c, d, e, f, CAM_A_RRZ_OUT_IMG, RRZ_OUT_HT, RRZ_tmp1_OUT_HT, U)\
	CMD(a, b, c, d, e, f, CAM_A_RRZ_OUT_IMG, RRZ_OUT_WD, RRZ_tmp1_OUT_WD, U)\
    CMD(a, b, c, d, e, f, CAM_A_RRZ_HORI_STEP, RRZ_HORI_STEP, RRZ_tmp1_HORI_STEP, U)\
    CMD(a, b, c, d, e, f, CAM_A_RRZ_HORI_INT_OFST, RRZ_HORI_INT_OFST, RRZ_tmp1_HORI_INT_OFST, U)\
    CMD(a, b, c, d, e, f, CAM_A_RRZ_HORI_SUB_OFST, RRZ_HORI_SUB_OFST, RRZ_tmp1_HORI_SUB_OFST, U)\
	/* SGG1 */\
    CMD(a, b, c, d, e, f, CAM_A_CTL_EN, SGG1_EN, SGG1_A_EN, U)\
	/* SGG5 */\
    CMD(a, b, c, d, e, f, CAM_A_AF_CON, AF_EXT_STAT_EN, af_a_ext_stat_en, U)\
    CMD(a, b, c, d, e, f, CAM_A_CTL_DMA_EN, SGG5_EN, SGG5_A_EN, U)\
	/* AF */\
    CMD(a, b, c, d, e, f, CAM_A_CTL_EN, AF_EN, AF_A_EN, U)\
    CMD(a, b, c, d, e, f, CAM_A_AF_BLK_0, AF_BLK_XSIZE, af_a_blk_xsize, U)\
    CMD(a, b, c, d, e, f, CAM_A_AF_BLK_0, AF_BLK_YSIZE, af_a_blk_ysize, U)\
    CMD(a, b, c, d, e, f, CAM_A_AF_BLK_1, AF_BLK_XNUM, af_a_blk_xnum, U)\
    CMD(a, b, c, d, e, f, CAM_A_AF_VLD, AF_VLD_XSTART, af_a_vld_xstart, U)\
    CMD(a, b, c, d, e, f, CAM_A_AF_BLK_1, AF_BLK_YNUM, af_a_blk_ynum, U)\
    CMD(a, b, c, d, e, f, CAM_A_AF_VLD, AF_VLD_YSTART, af_a_vld_ystart, U)\
	/* AFO */\
    CMD(a, b, c, d, e, f, CAM_A_CTL_DMA_EN, AFO_EN, AFO_A_EN, U)\
	CMD(a, b, c, d, e, f, CAM_A_AFO_BASE_ADDR, BASE_ADDR, AFO_A_BASE_ADDR, U)\
	CMD(a, b, c, d, e, f, CAM_A_AFO_OFST_ADDR, OFFSET_ADDR, AFO_A_OFFSET_ADDR, U)\
	CMD(a, b, c, d, e, f, CAM_A_AFO_XSIZE, XSIZE, AFO_A_XSIZE, U)\
	CMD(a, b, c, d, e, f, CAM_A_AFO_YSIZE, YSIZE, AFO_A_YSIZE, U)\
	CMD(a, b, c, d, e, f, CAM_A_AFO_STRIDE, STRIDE, AFO_A_STRIDE, U)\
	/* RMX */\
    CMD(a, b, c, d, e, f, CAM_A_CTL_EN, RMX_EN, RMX_A_EN, U)\
    CMD(a, b, c, d, e, f, CAM_A_RMX_CTL, RMX_SINGLE_MODE_1, RMX_A_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, CAM_A_RMX_CTL, RMX_SINGLE_MODE_2, RMX_A_SIG_MODE2, U)\
    CMD(a, b, c, d, e, f, CAM_A_RMX_CTL, RMX_EDGE, RMX_A_EDGE, U)\
    CMD(a, b, c, d, e, f, CAM_A_RMX_CROP, RMX_STR_X, RMX_A_STR_X, U)\
    CMD(a, b, c, d, e, f, CAM_A_RMX_CROP, RMX_END_X, RMX_A_END_X, U)\
    CMD(a, b, c, d, e, f, CAM_A_RMX_VSIZE, RMX_HT, RMX_A_HT, U)\

/* PLATFORM parameters */
/* a, b, c, d, e, f reserved */
/* struct name */
/* variable name */
/* R/W type: R, RW, W */
#define DUAL_HW_B_LIST(CMD, a, b, c, d, e, f) \
	/* SEL_D */\
	CMD(a, b, c, d, e, f, CAM_B_CTL_SEL, DMX_SEL, DMX_B_SEL, U)\
	CMD(a, b, c, d, e, f, CAM_B_CTL_SEL, PMX_SEL, PMX_B_SEL, U)\
	CMD(a, b, c, d, e, f, CAM_B_CTL_SEL, SGG_SEL, SGG_B_SEL, U)\
	/* PIXEL MODE_D */\
    CMD(a, b, c, d, e, f, CAM_B_CTL_FMT_SEL, PIX_BUS_RMXO, PIX_BUS_RMXO_B, U)\
    CMD(a, b, c, d, e, f, CAM_B_CTL_FMT_SEL, PIX_BUS_BMXO, PIX_BUS_BMXO_B, U)\
    CMD(a, b, c, d, e, f, CAM_B_CTL_FMT_SEL, PIX_BUS_DMXO, PIX_BUS_DMXO_B, U)\
    CMD(a, b, c, d, e, f, CAM_B_CTL_FMT_SEL, PIX_BUS_DMXI, PIX_BUS_DMXI_B, U)\
	/* RCP3_D */\
    CMD(a, b, c, d, e, f, CAM_B_CTL_EN, RCP3_EN, RCP3_B_EN, U)\
    CMD(a, b, c, d, e, f, CAM_B_RCP3_CROP_CON1, RCP_STR_X, RCP_tmp8_STR_X, U)\
    CMD(a, b, c, d, e, f, CAM_B_RCP3_CROP_CON1, RCP_END_X, RCP_tmp8_END_X, U)\
    CMD(a, b, c, d, e, f, CAM_B_RCP3_CROP_CON2, RCP_STR_Y, RCP_tmp8_STR_Y, U)\
    CMD(a, b, c, d, e, f, CAM_B_RCP3_CROP_CON2, RCP_END_Y, RCP_tmp8_END_Y, U)\
	/* IMGO_D */\
    CMD(a, b, c, d, e, f, CAM_B_CTL_EN, PAK_EN, PAK_B_EN, U)\
    CMD(a, b, c, d, e, f, CAM_B_CTL_EN, CPG_EN, CPG_B_EN, U)\
    CMD(a, b, c, d, e, f, CAM_B_CTL_SEL, IMG_SEL, IMG_B_SEL, U)\
    CMD(a, b, c, d, e, f, CAM_B_CTL_DMA_EN, IMGO_EN, IMGO_B_EN, U)\
    CMD(a, b, c, d, e, f, CAM_B_CTL_FMT_SEL, IMGO_FMT, IMGO_FMT_B, U)\
    CMD(a, b, c, d, e, f, CAM_B_IMGO_BASE_ADDR, BASE_ADDR, IMGO_B_BASE_ADDR, U)\
    CMD(a, b, c, d, e, f, CAM_B_IMGO_OFST_ADDR, OFFSET_ADDR, IMGO_B_OFFSET_ADDR, U)\
    CMD(a, b, c, d, e, f, CAM_B_IMGO_XSIZE, XSIZE, IMGO_B_XSIZE, U)\
    CMD(a, b, c, d, e, f, CAM_B_IMGO_YSIZE, YSIZE, IMGO_B_YSIZE, U)\
    CMD(a, b, c, d, e, f, CAM_B_IMGO_STRIDE, STRIDE, IMGO_B_STRIDE, U)\
    CMD(a, b, c, d, e, f, CAM_B_IMGO_CROP, XOFFSET, IMGO_B_XOFFSET, U)\
    CMD(a, b, c, d, e, f, CAM_B_IMGO_CROP, YOFFSET, IMGO_B_YOFFSET, U)\
	/* DMX_D */\
    CMD(a, b, c, d, e, f, CAM_B_CTL_EN, DMX_EN, DMX_B_EN, U)\
    CMD(a, b, c, d, e, f, CAM_B_DMX_CTL, DMX_EDGE, DMX_B_EDGE, U)\
    CMD(a, b, c, d, e, f, CAM_B_DMX_CROP, DMX_STR_X, DMX_B_STR_X, U)\
    CMD(a, b, c, d, e, f, CAM_B_DMX_CROP, DMX_END_X, DMX_B_END_X, U)\
    CMD(a, b, c, d, e, f, CAM_B_DMX_VSIZE, DMX_HT, DMX_B_HT, U)\
	/* BIN_D */\
    CMD(a, b, c, d, e, f, CAM_B_CTL_EN, BIN_EN, BIN_B_EN, U)\
	/* VBN_D */\
    CMD(a, b, c, d, e, f, CAM_B_CTL_DMA_EN, VBN_EN, VBN_B_EN, U)\
    CMD(a, b, c, d, e, f, CAM_B_VBN_TYPE, VBN_TYPE, VBN_tmp2_TYPE, U)\
    CMD(a, b, c, d, e, f, CAM_B_VBN_TYPE, VBN_DIAG_SEL_EN, VBN_tmp2_DIAG_SEL_EN, U)\
	/* SCM_D */\
    CMD(a, b, c, d, e, f, CAM_B_CTL_DMA_EN, SCM_EN, SCM_B_EN, U)\
    CMD(a, b, c, d, e, f, CAM_B_SCM_CFG1, SCM_LE_INV_CTL, SCM_tmp2_LE_INV_CTL, U)\
	/* DBN_D */\
    CMD(a, b, c, d, e, f, CAM_B_CTL_EN, DBN_EN, DBN_B_EN, U)\
	/* PBN_D */\
    CMD(a, b, c, d, e, f, CAM_B_CTL_EN, PBN_EN, PBN_B_EN, U)\
    CMD(a, b, c, d, e, f, CAM_B_PBN_TYPE, PBN_SEP, PBN_tmp2_SEP, U)\
	/* SL2F_D */\
    CMD(a, b, c, d, e, f, CAM_B_CTL_DMA_EN, SL2F_EN, SL2F_B_EN, U)\
    CMD(a, b, c, d, e, f, CAM_B_SL2F_XOFF, SL2_X_OFST, SL2F_B_X_OFST, U)\
    CMD(a, b, c, d, e, f, CAM_B_SL2F_YOFF, SL2_Y_OFST, SL2F_B_Y_OFST, U)\
    CMD(a, b, c, d, e, f, CAM_B_SL2F_SIZE, SL2_TPIPE_WD, SL2F_B_TPIPE_WD, U)\
    CMD(a, b, c, d, e, f, CAM_B_SL2F_SIZE, SL2_TPIPE_HT, SL2F_B_TPIPE_HT, U)\
	/* DBS_D */\
    CMD(a, b, c, d, e, f, CAM_B_CTL_EN, DBS_EN, DBS_B_EN, U)\
	/* LE_INV_CTL_D */\
    CMD(a, b, c, d, e, f, CAM_B_RMG_HDR_CFG, RMG_IHDR_EN, RMG_tmp2_IHDR_EN, U)\
    CMD(a, b, c, d, e, f, CAM_B_RMG_HDR_CFG, RMG_ZHDR_EN, RMG_tmp2_ZHDR_EN, U)\
    CMD(a, b, c, d, e, f, CAM_B_RMG_HDR_CFG2, RMG_HDR_THK, RMG_tmp2_THK, U)\
    CMD(a, b, c, d, e, f, CAM_B_BIN_CTL, BIN_LE_INV_CTL, BIN_tmp2_LE_INV_CTL, U)\
    CMD(a, b, c, d, e, f, CAM_B_RMG_HDR_GAIN, RMG_LE_INV_CTL, RMG_tmp2_LE_INV_CTL, U)\
    CMD(a, b, c, d, e, f, CAM_B_RMM_OSC, RMM_LE_INV_CTL, RMM_tmp2_LE_INV_CTL, U)\
    CMD(a, b, c, d, e, f, CAM_B_BNR_BPC_CON, BNR_LE_INV_CTL, BNR_tmp2_LE_INV_CTL, U)\
    CMD(a, b, c, d, e, f, CAM_B_DBS_CTL, DBS_LE_INV_CTL, DBS_tmp2_LE_INV_CTL, U)\
	/* SGM_D */\
    CMD(a, b, c, d, e, f, CAM_B_CTL_EN, SGM_EN, SGM_B_EN, U)\
    /*CMD(a, b, c, d, e, f, CAM_B_SGM_LUT_R[0], SGM_R, SGM_R_B, U)*/\
	/* OBC_D */\
    CMD(a, b, c, d, e, f, CAM_B_CTL_EN, OBC_EN, OBC_B_EN, U)\
	/* RMG_D */\
    CMD(a, b, c, d, e, f, CAM_B_CTL_EN, RMG_EN, RMG_B_EN, U)\
	/* BPCI_D */\
    CMD(a, b, c, d, e, f, CAM_B_BPCI_XSIZE, XSIZE, BPCI_B_XSIZE, U)\
    CMD(a, b, c, d, e, f, CAM_B_BPCI_YSIZE, YSIZE, BPCI_B_YSIZE, U)\
    CMD(a, b, c, d, e, f, CAM_B_BPCI_STRIDE, STRIDE, BPCI_B_STRIDE, U)\
	/* BNR_D */\
    CMD(a, b, c, d, e, f, CAM_B_CTL_EN, BNR_EN, BNR_B_EN, U)\
    CMD(a, b, c, d, e, f, CAM_B_BNR_BPC_TBLI1, BPC_XOFFSET, BPC_tmp2_XOFFSET, U)\
    CMD(a, b, c, d, e, f, CAM_B_BNR_BPC_TBLI1, BPC_YOFFSET, BPC_tmp2_YOFFSET, U)\
    CMD(a, b, c, d, e, f, CAM_B_BNR_BPC_TBLI2, BPC_XSIZE, BPC_tmp2_XSIZE, U)\
    CMD(a, b, c, d, e, f, CAM_B_BNR_BPC_TBLI2, BPC_YSIZE, BPC_tmp2_YSIZE, U)\
    CMD(a, b, c, d, e, f, CAM_B_BNR_PDC_POS, PDC_XCENTER, BPC_tmp2_XCENTER, U)\
    CMD(a, b, c, d, e, f, CAM_B_BNR_PDC_POS, PDC_YCENTER, BPC_tmp2_YCENTER, U)\
    CMD(a, b, c, d, e, f, CAM_B_BNR_PDC_CON, PDC_OUT, PDC_tmp2_OUT, U)\
    CMD(a, b, c, d, e, f, CAM_B_BNR_PDC_CON, PDC_EN, PDC_tmp2_EN, U)\
	/* RMM_D */\
    CMD(a, b, c, d, e, f, CAM_B_CTL_EN, RMM_EN, RMM_B_EN, U)\
	/* CAC_D */\
    CMD(a, b, c, d, e, f, CAM_B_CTL_EN, CAC_EN, CAC_B_EN, U)\
	/* CACI_D */\
    CMD(a, b, c, d, e, f, CAM_B_CACI_STRIDE, STRIDE, CACI_B_EN, U)\
    CMD(a, b, c, d, e, f, CAM_B_CACI_OFST_ADDR, OFFSET_ADDR, CACI_B_OFFSET_ADDR, U)\
    CMD(a, b, c, d, e, f, CAM_B_CACI_XSIZE, XSIZE, CACI_B_XSIZE, U)\
	/* CAC_D */\
    CMD(a, b, c, d, e, f, CAM_B_CAC_TILE_OFFSET, CAC_OFFSET_X, CAC_tmp2_TPIPE_OFST_X, U)\
    CMD(a, b, c, d, e, f, CAM_B_CAC_TILE_SIZE, CAC_TILE_WIDTH, CAC_tmp2_FULL_SIZE_X, U)\
	/* LSC_D */\
    CMD(a, b, c, d, e, f, CAM_B_CTL_EN, LSC_EN, LSC_B_EN, U)\
    CMD(a, b, c, d, e, f, CAM_B_LSC_CTL1, LSC_EXTEND_COEF_MODE, LSC_tmp2_EXTEND_COEF_MODE, U)\
    CMD(a, b, c, d, e, f, CAM_B_LSC_CTL2, LSC_SDBLK_XNUM, LSC_tmp2_SDBLK_XNUM, U)\
    CMD(a, b, c, d, e, f, CAM_B_LSC_CTL2, LSC_SDBLK_WIDTH, LSC_tmp2_SDBLK_WIDTH, U)\
    CMD(a, b, c, d, e, f, CAM_B_LSC_LBLOCK, LSC_SDBLK_lWIDTH, LSC_tmp2_SDBLK_lWIDTH, U)\
    CMD(a, b, c, d, e, f, CAM_B_LSC_TPIPE_OFST, LSC_TPIPE_OFST_X, LSC_tmp2_TPIPE_OFST_X, U)\
    CMD(a, b, c, d, e, f, CAM_B_LSC_TPIPE_SIZE, LSC_TPIPE_SIZE_X, LSC_tmp2_TPIPE_SIZE_X, U)\
	/* LSCI_D */\
    CMD(a, b, c, d, e, f, CAM_B_LSCI_OFST_ADDR, OFFSET_ADDR, LSCI_B_OFFSET_ADDR, U)\
    CMD(a, b, c, d, e, f, CAM_B_LSCI_STRIDE, STRIDE, LSCI_B_STRIDE, U)\
    CMD(a, b, c, d, e, f, CAM_B_LSCI_XSIZE, XSIZE, LSCI_B_XSIZE, U)\
	/* RCR_D */\
    CMD(a, b, c, d, e, f, CAM_B_CTL_EN, RCP_EN, RCP_B_EN, U)\
    CMD(a, b, c, d, e, f, CAM_B_RCP_CROP_CON1, RCP_STR_X, RCP_B_STR_X, U)\
    CMD(a, b, c, d, e, f, CAM_B_RCP_CROP_CON1, RCP_END_X, RCP_B_END_X, U)\
    CMD(a, b, c, d, e, f, CAM_B_RCP_CROP_CON2, RCP_STR_Y, RCP_B_STR_Y, U)\
    CMD(a, b, c, d, e, f, CAM_B_RCP_CROP_CON2, RCP_END_Y, RCP_B_END_Y, U)\
	/* BMX_D */\
    CMD(a, b, c, d, e, f, CAM_B_CTL_EN, BMX_EN, BMX_B_EN, U)\
    CMD(a, b, c, d, e, f, CAM_B_BMX_CTL, BMX_SINGLE_MODE_1, BMX_B_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, CAM_B_BMX_CTL, BMX_SINGLE_MODE_2, BMX_B_SIG_MODE2, U)\
    CMD(a, b, c, d, e, f, CAM_B_BMX_CTL, BMX_EDGE, BMX_B_EDGE, U)\
    CMD(a, b, c, d, e, f, CAM_B_BMX_CROP, BMX_STR_X, BMX_B_STR_X, U)\
    CMD(a, b, c, d, e, f, CAM_B_BMX_CROP, BMX_END_X, BMX_B_END_X, U)\
    CMD(a, b, c, d, e, f, CAM_B_BMX_VSIZE, BMX_HT, BMX_B_HT, U)\
	/* AMX_D */\
    CMD(a, b, c, d, e, f, CAM_B_CTL_DMA_EN, AMX_EN, AMX_B_EN, U)\
    CMD(a, b, c, d, e, f, CAM_B_AMX_CTL, AMX_SINGLE_MODE_1, AMX_B_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, CAM_B_AMX_CTL, AMX_SINGLE_MODE_2, AMX_B_SIG_MODE2, U)\
    CMD(a, b, c, d, e, f, CAM_B_AMX_CTL, AMX_EDGE, AMX_B_EDGE, U)\
    CMD(a, b, c, d, e, f, CAM_B_AMX_CROP, AMX_STR_X, AMX_B_STR_X, U)\
    CMD(a, b, c, d, e, f, CAM_B_AMX_CROP, AMX_END_X, AMX_B_END_X, U)\
    CMD(a, b, c, d, e, f, CAM_B_AMX_VSIZE, AMX_HT, AMX_B_HT, U)\
	/* RPG_D */\
    CMD(a, b, c, d, e, f, CAM_B_CTL_EN, RPG_EN, RPG_B_EN, U)\
	/* PMX_D */\
    CMD(a, b, c, d, e, f, CAM_B_CTL_EN, PMX_EN, PMX_B_EN, U)\
    CMD(a, b, c, d, e, f, CAM_B_PMX_CTL, PMX_SINGLE_MODE_1, PMX_tmp2_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, CAM_B_PMX_CTL, PMX_SINGLE_MODE_2, PMX_tmp2_SIG_MODE2, U)\
    CMD(a, b, c, d, e, f, CAM_B_PMX_CTL, PMX_EDGE, PMX_tmp2_EDGE, U)\
    CMD(a, b, c, d, e, f, CAM_B_PMX_CROP, PMX_STR_X, PMX_tmp2_STR_X, U)\
    CMD(a, b, c, d, e, f, CAM_B_PMX_CROP, PMX_END_X, PMX_tmp2_END_X, U)\
    CMD(a, b, c, d, e, f, CAM_B_PMX_VSIZE, PMX_HT, PMX_tmp2_HT, U)\
	/* RRZ_D */\
    CMD(a, b, c, d, e, f, CAM_B_CTL_EN, RRZ_EN, RRZ_B_EN, U)\
    CMD(a, b, c, d, e, f, CAM_B_RRZ_VERT_STEP, RRZ_VERT_STEP, RRZ_tmp2_VERT_STEP, U)\
    CMD(a, b, c, d, e, f, CAM_B_RRZ_VERT_INT_OFST, RRZ_VERT_INT_OFST, RRZ_tmp2_VERT_INT_OFST, U)\
    CMD(a, b, c, d, e, f, CAM_B_RRZ_VERT_SUB_OFST, RRZ_VERT_SUB_OFST, RRZ_tmp2_VERT_SUB_OFST, U)\
    CMD(a, b, c, d, e, f, CAM_B_RRZ_OUT_IMG, RRZ_OUT_HT, RRZ_tmp2_OUT_HT, U)\
	CMD(a, b, c, d, e, f, CAM_B_RRZ_OUT_IMG, RRZ_OUT_WD, RRZ_tmp2_OUT_WD, U)\
    CMD(a, b, c, d, e, f, CAM_B_RRZ_HORI_STEP, RRZ_HORI_STEP, RRZ_tmp2_HORI_STEP, U)\
    CMD(a, b, c, d, e, f, CAM_B_RRZ_HORI_INT_OFST, RRZ_HORI_INT_OFST, RRZ_tmp2_HORI_INT_OFST, U)\
    CMD(a, b, c, d, e, f, CAM_B_RRZ_HORI_SUB_OFST, RRZ_HORI_SUB_OFST, RRZ_tmp2_HORI_SUB_OFST, U)\
	/* SGG1_D */\
    CMD(a, b, c, d, e, f, CAM_B_CTL_EN, SGG1_EN, SGG1_B_EN, U)\
	/* SGG5_D */\
    CMD(a, b, c, d, e, f, CAM_B_AF_CON, AF_EXT_STAT_EN, af_b_ext_stat_en, U)\
    CMD(a, b, c, d, e, f, CAM_B_CTL_DMA_EN, SGG5_EN, SGG5_B_EN, U)\
	/* AF_D */\
    CMD(a, b, c, d, e, f, CAM_B_CTL_EN, AF_EN, AF_B_EN, U)\
    CMD(a, b, c, d, e, f, CAM_B_AF_BLK_0, AF_BLK_XSIZE, af_b_blk_xsize, U)\
    CMD(a, b, c, d, e, f, CAM_B_AF_BLK_0, AF_BLK_YSIZE, af_b_blk_ysize, U)\
    CMD(a, b, c, d, e, f, CAM_B_AF_BLK_1, AF_BLK_XNUM, af_b_blk_xnum, U)\
    CMD(a, b, c, d, e, f, CAM_B_AF_VLD, AF_VLD_XSTART, af_b_vld_xstart, U)\
    CMD(a, b, c, d, e, f, CAM_B_AF_BLK_1, AF_BLK_YNUM, af_b_blk_ynum, U)\
    CMD(a, b, c, d, e, f, CAM_B_AF_VLD, AF_VLD_YSTART, af_b_vld_ystart, U)\
	/* AFO_D */\
    CMD(a, b, c, d, e, f, CAM_B_CTL_DMA_EN, AFO_EN, AFO_B_EN, U)\
	CMD(a, b, c, d, e, f, CAM_B_AFO_BASE_ADDR, BASE_ADDR, AFO_B_BASE_ADDR, U)\
	CMD(a, b, c, d, e, f, CAM_B_AFO_OFST_ADDR, OFFSET_ADDR, AFO_B_OFFSET_ADDR, U)\
	CMD(a, b, c, d, e, f, CAM_B_AFO_XSIZE, XSIZE, AFO_B_XSIZE, U)\
	CMD(a, b, c, d, e, f, CAM_B_AFO_YSIZE, YSIZE, AFO_B_YSIZE, U)\
	CMD(a, b, c, d, e, f, CAM_B_AFO_STRIDE, STRIDE, AFO_B_STRIDE, U)\
	/* RMX_D */\
    CMD(a, b, c, d, e, f, CAM_B_CTL_EN, RMX_EN, RMX_B_EN, U)\
    CMD(a, b, c, d, e, f, CAM_B_RMX_CTL, RMX_SINGLE_MODE_1, RMX_B_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, CAM_B_RMX_CTL, RMX_SINGLE_MODE_2, RMX_B_SIG_MODE2, U)\
    CMD(a, b, c, d, e, f, CAM_B_RMX_CTL, RMX_EDGE, RMX_B_EDGE, U)\
    CMD(a, b, c, d, e, f, CAM_B_RMX_CROP, RMX_STR_X, RMX_B_STR_X, U)\
    CMD(a, b, c, d, e, f, CAM_B_RMX_CROP, RMX_END_X, RMX_B_END_X, U)\
    CMD(a, b, c, d, e, f, CAM_B_RMX_VSIZE, RMX_HT, RMX_B_HT, U)\

#define DUAL_SEL_CHECK(a, b, c, d, e, f, g,...) \
	if (false == (b))\
	{\
		if ((a) == g)\
		{\
			(b) = true;\
		}\
	}\

/* data search & update reg mapper */
/* a: ptr_reg */
/* b: cmp str */
/* c: value to fill */
/* d: found flag */
#define DUAL_READ_LOG_FILL_VAL(a, b, c, d, e, f, g, h,...) \
    if (false == (d))\
    {\
		if (0 == memcmp(b, #g, sizeof(#g) - 1))\
		{\
			if (0 == memcmp(b + sizeof(#g), #h, sizeof(#h)))\
			{\
				(a)->g.h = (c);\
				(d) = true;\
				(e)->g.h = 1;\
			}\
		}\
    }\

/* data search & update reg mapper */
/* a: ptr_reg */
/* b: cmp str */
/* c: value to fill */
/* d: found flag */
#define DUAL_READ_LOG_FILL_VAL_SW(a, b, c, d, e, f, g, h, i, j,...) \
    if (false == (d))\
    {\
		if (0 == memcmp(b, #g, sizeof(#g) - 1))\
		{\
			if (0 == memcmp(b + sizeof(#g), #h, sizeof(#h)))\
			{\
				(a)->g.h = (c);\
				(d) = true;\
				(e)->g.h = 1;\
			}\
		}\
    }\

/* data search & update reg mapper */
/* a: ptr_reg */
/* b: cmp str */
/* c: value to fill */
/* d: found flag */
#define DUAL_READ_LOG_FILL_VAL_ENGINE(a, b, c, d, e, f, g, h, i, j, k,...) \
    if (false == (d))\
    {\
		if (0 == memcmp(b, #h, sizeof(#h)))\
		{\
			(a)->h.Raw = (c);\
			(d) = true;\
			(e)->h.Raw = 0xFFFFFFFF;\
		}\
    }\

/* data search & update reg mapper */
/* a: ptr_reg */
/* b: cmp str */
/* c: value to fill */
/* d: found flag */
#define DUAL_READ_LOG_FILL_VAL_ENGINE_D(a, b, c, d, e, f, g, h, i, j, k,...) \
    if (false == (d))\
    {\
		if (0 == memcmp(b, #i, sizeof(#i)))\
		{\
			(a)->i.Raw = (c);\
			(d) = true;\
			(e)->i.Raw = 0xFFFFFFFF;\
		}\
    }\

/* data search & update reg mapper */
/* a: ptr_reg */
/* b: cmp str */
/* c: value to fill */
/* d: found flag */
#define DUAL_READ_LOG_FILL_VAL_ARRAY(a, b, c, d, e, f, g, h, i, j, k, l,...) \
	for (int temp_i=0;temp_i<l;temp_i++)\
	{\
		if (false == (d))\
		{\
			char array_name[DUAL_MAX_DUMP_COLUMN_LENGTH];\
			memset(array_name, 0x0, DUAL_MAX_DUMP_COLUMN_LENGTH);\
			dual_sprintf(array_name, DUAL_MAX_DUMP_COLUMN_LENGTH, "%s[%d]", #h, temp_i);\
			if (0 == memcmp(b, array_name, DUAL_MAX_DUMP_COLUMN_LENGTH))\
			{\
				(a)->h[temp_i].Raw = (c);\
				(d) = true;\
				(e)->h[temp_i].Raw = 0xFFFFFFFF;\
				break;\
			}\
		}\
	}\

/* data search & update reg mapper */
/* a: ptr_reg */
/* b: cmp str */
/* c: value to fill */
/* d: found flag */
#define DUAL_READ_LOG_FILL_VAL_ARRAY_D(a, b, c, d, e, f, g, h, i, j, k, l,...) \
	for (int temp_i=0;temp_i<l;temp_i++)\
	{\
	    if (false == (d))\
		{\
			char array_name[DUAL_MAX_DUMP_COLUMN_LENGTH];\
			memset(array_name, 0x0, DUAL_MAX_DUMP_COLUMN_LENGTH);\
			dual_sprintf(array_name, DUAL_MAX_DUMP_COLUMN_LENGTH, "%s[%d]", #i, temp_i);\
			if (0 == memcmp(b, array_name, DUAL_MAX_DUMP_COLUMN_LENGTH))\
			{\
				(a)->i[temp_i].Raw = (c);\
				(d) = true;\
				(e)->i[temp_i].Raw = 0xFFFFFFFF;\
				break;\
			}\
		}\
	}\

/* data search & update reg mapper */
/* a: ptr_reg */
/* b: cmp str */
/* c: value to fill */
/* d: found flag */
#define DUAL_READ_LOG_FILL_VAL_HW(a, b, c, d, e, f, g, h,...) \
    if (false == (d))\
    {\
		if (0 == memcmp(b, #h, sizeof(#h)))\
		{\
			(a)->h.Raw = (c);\
			(d) = true;\
			(e)->h.Raw = 0xFFFFFFFF;\
		}\
    }\

/* data search & update reg mapper */
/* a: ptr_reg */
/* b: cmp str */
/* c: value to fill */
/* d: found flag */
#define DUAL_READ_CHECK_LOG_FLAG(a, b, c, d, e, f, g, h,...) \
	if (0 == (a)->g.h)\
	{\
		(b) = true;\
		printf("Not found input config: %s.%s\n", #g, #h);\
    }\

/* data search & update reg mapper */
/* a: ptr_reg */
/* b: cmp str */
/* c: value to fill */
/* d: found flag */
#define DUAL_READ_CHECK_LOG_FLAG_SW(a, b, c, d, e, f, g, h, i, j,...) \
	if (0 == (a)->g.h)\
	{\
		(b) = true;\
		printf("Not found input config: %s.%s\n", #g, #h);\
    }\

/* data search & update reg mapper */
/* a: ptr_reg */
/* b: cmp str */
/* c: value to fill */
/* d: found flag */
#define DUAL_READ_CHECK_LOG_FLAG_HW(a, b, c, d, e, f, g, h,...) \
	if (0 == (a)->h.Raw)\
	{\
		(b) = true;\
		printf("Not found input config: %s\n", #h);\
    }\

/* data search & update reg mapper */
/* a: ptr_reg */
/* b: cmp str */
/* c: value to fill */
/* d: found flag */
#define DUAL_READ_CHECK_LOG_FLAG_ENGINE(a, b, c, d, e, f, g, h, i, j, k,...) \
	if (0 == (a)->h.Raw)\
	{\
		(b) = true;\
		printf("Not found input config: %s\n", #h);\
    }\

/* data search & update reg mapper */
/* a: ptr_reg */
/* b: cmp str */
/* c: value to fill */
/* d: found flag */
#define DUAL_READ_CHECK_LOG_FLAG_ENGINE_D(a, b, c, d, e, f, g, h, i, j, k,...) \
	if (0 == (a)->i.Raw)\
	{\
		(b) = true;\
		printf("Not found input config: %s\n", #i);\
    }\

/* data search & update reg mapper */
/* a: ptr_reg */
/* b: cmp str */
/* c: value to fill */
/* d: found flag */
#define DUAL_READ_CHECK_LOG_FLAG_ARRAY(a, b, c, d, e, f, g, h, i, j, k, l,...) \
	for (int temp_i=0;temp_i<l;temp_i++)\
	{\
		if (0 == (a)->h[temp_i].Raw)\
		{\
			(b) = true;\
			printf("Not found input config: %s[%d]\n", #h, temp_i);\
		}\
    }\

/* data search & update reg mapper */
/* a: ptr_reg */
/* b: cmp str */
/* c: value to fill */
/* d: found flag */
#define DUAL_READ_CHECK_LOG_FLAG_ARRAY_D(a, b, c, d, e, f, g, h, i, j, k, l,...) \
	for (int temp_i=0;temp_i<l;temp_i++)\
	{\
		if (0 == (a)->i[temp_i].Raw)\
		{\
			(b) = true;\
			printf("Not found input config: %s[%d]\n", #i, temp_i);\
		}\
    }\

/* data search & update reg mapper */
/* a: ptr_reg */
/* b: cmp str */
/* c: value to fill */
/* d: found flag */
#define DUAL_READ_CHECK_REG_FLAG_HW(a, b, c, d, e, f, g, h,...) \
	if (0 == (a)->h.Raw)\
	{\
		(c) = true;\
		(b)->h.Raw = 0xFFFFFFFF;\
		printf("Not found reg: %s\n", #h);\
    }\

/* data search & update reg mapper */
/* a: ptr_reg */
/* b: cmp str */
/* c: value to fill */
/* d: found flag */
#define DUAL_READ_CHECK_REG_FLAG_ENGINE(a, b, c, d, e, f, g, h, i, j, k,...) \
	if (0 == (a)->h.Raw)\
	{\
		(c) = true;\
		(b)->h.Raw = 0xFFFFFFFF;\
		printf("Not found reg: %s\n", #h);\
    }\

/* data search & update reg mapper */
/* a: ptr_reg */
/* b: cmp str */
/* c: value to fill */
/* d: found flag */
#define DUAL_READ_CHECK_REG_FLAG_ENGINE_D(a, b, c, d, e, f, g, h, i, j, k,...) \
	if (0 == (a)->i.Raw)\
	{\
		(c) = true;\
		(b)->i.Raw = 0xFFFFFFFF;\
		printf("Not found reg: %s\n", #i);\
    }\

/* data search & update reg mapper */
/* a: ptr_reg */
/* b: cmp str */
/* c: value to fill */
/* d: found flag */
#define DUAL_READ_CHECK_REG_FLAG_ARRAY(a, b, c, d, e, f, g, h, i, j, k, l,...) \
	for (int temp_i=0;temp_i<l;temp_i++)\
	{\
		if (0 == (a)->h[temp_i].Raw)\
		{\
			(c) = true;\
			(b)->h[temp_i].Raw = 0xFFFFFFFF;\
			printf("Not found reg: %s[%d]\n", #h, temp_i);\
		}\
    }\

/* data search & update reg mapper */
/* a: ptr_reg */
/* b: cmp str */
/* c: value to fill */
/* d: found flag */
#define DUAL_READ_CHECK_REG_FLAG_ARRAY_D(a, b, c, d, e, f, g, h, i, j, k, l,...) \
	for (int temp_i=0;temp_i<l;temp_i++)\
	{\
		if (0 == (a)->i[temp_i].Raw)\
		{\
			(c) = true;\
			(b)->i[temp_i].Raw = 0xFFFFFFFF;\
			printf("Not found reg: %s[%d]\n", #i, temp_i);\
		}\
    }\

#define DUAL_LOG_PRINT_TXT(a, b, c, d, e, f, g, h,...) \
	b(a, "%s %s.%s%s%d\n", DUAL_IN_DUMP_HEADER, #g, #h, DUAL_SPACE_EQUAL_SYMBOL_STR, (c)->g.h);

#define DUAL_LOG_PRINT_SW_TXT(a, b, c, d, e, f, g, h, i, j,...) \
	b(a, "%s %s.%s%s%d\n", DUAL_IN_DUMP_HEADER, #g, #h, DUAL_SPACE_EQUAL_SYMBOL_STR, (c)->g.h);

#define DUAL_LOG_PRINT_TXT_HW(a, b, c, d, e, f, g, h,...) \
	b(a, "%s %s%s0x%x\n", DUAL_IN_DUMP_HEADER, #h, DUAL_SPACE_EQUAL_SYMBOL_STR, (c)->h.Raw);

#define DUAL_LOG_PRINT_TXT_ENGINE(a, b, c, d, e, f, g, h, i, j, k,...) \
	b(a, "%s %s%s0x%x\n", DUAL_IN_DUMP_HEADER, #h, DUAL_SPACE_EQUAL_SYMBOL_STR, (c)->h.Raw);

#define DUAL_LOG_PRINT_TXT_ENGINE_D(a, b, c, d, e, f, g, h, i, j, k,...) \
	b(a, "%s %s%s0x%x\n", DUAL_IN_DUMP_HEADER, #i, DUAL_SPACE_EQUAL_SYMBOL_STR, (c)->i.Raw);

#define DUAL_LOG_PRINT_TXT_ARRAY(a, b, c, d, e, f, g, h, i, j, k, l,...) \
	for (int temp_i=0;temp_i<l;temp_i++)\
	{\
		b(a, "%s %s[%d]%s0x%x\n", DUAL_IN_DUMP_HEADER, #h, temp_i, DUAL_SPACE_EQUAL_SYMBOL_STR, (c)->h[temp_i].Raw);\
	}\

#define DUAL_LOG_PRINT_TXT_ARRAY_D(a, b, c, d, e, f, g, h, i, j, k, l,...) \
	for (int temp_i=0;temp_i<l;temp_i++)\
	{\
		b(a, "%s %s[%d]%s0x%x\n", DUAL_IN_DUMP_HEADER, #i, temp_i, DUAL_SPACE_EQUAL_SYMBOL_STR, (c)->i[temp_i].Raw);\
	}\

#define DUAL_LOG_PRINT_TXT_ENGINE_OUT(a, b, c, d, e, f, g, h, i, j, k,...) \
	b(a, "%s %s%s0x%x\n", DUAL_OUT_DUMP_HEADER, #h, DUAL_SPACE_EQUAL_SYMBOL_STR, (c)->h.Raw);

#define DUAL_LOG_PRINT_TXT_ENGINE_OUT_D(a, b, c, d, e, f, g, h, i, j, k,...) \
	b(a, "%s %s%s0x%x\n", DUAL_OUT_DUMP_HEADER, #i, DUAL_SPACE_EQUAL_SYMBOL_STR, (c)->i.Raw);

#define DUAL_LOG_PRINT_TXT_ARRAY_OUT(a, b, c, d, e, f, g, h, i, j, k, l,...) \
	for (int temp_i=0;temp_i<l;temp_i++)\
	{\
		b(a, "%s %s[%d]%s0x%x\n", DUAL_OUT_DUMP_HEADER, #h, temp_i, DUAL_SPACE_EQUAL_SYMBOL_STR, (c)->h[temp_i].Raw);\
	}\

#define DUAL_LOG_PRINT_TXT_ARRAY_OUT_D(a, b, c, d, e, f, g, h, i, j, k, l,...) \
	for (int temp_i=0;temp_i<l;temp_i++)\
	{\
		b(a, "%s %s[%d]%s0x%x\n", DUAL_OUT_DUMP_HEADER, #i, temp_i, DUAL_SPACE_EQUAL_SYMBOL_STR, (c)->i[temp_i].Raw);\
	}\

#define DUAL_PRINT_SW_TXT(a, b, c, d, e, f, g, h, i, j,...) \
	b(a, "%s %s.%s%s%d (0x%x) [knobs] %s [c model] %s\n", DUAL_ENG_DUMP_HEADER, #g, #h, DUAL_SPACE_EQUAL_SYMBOL_STR, (c)->g.h, (c)->g.h, #i, #j);

#define DUAL_PRINT_HW_TXT(a, b, c, d, e, f, g, h, i,...) \
	b(a, "%s %s.%s%s%d (0x%x) [c model] %s\n", DUAL_ENG_DUMP_HEADER, #g, #h, DUAL_SPACE_EQUAL_SYMBOL_STR, (c)->g.Bits.h, (c)->g.Bits.h, #i);

#define DUAL_UART_PRINT_TXT(a, b, c, d, e, f, g, h,...) \
	a("%s %s.%s%s%d\n", DUAL_IN_DUMP_HEADER, #g, #h, DUAL_SPACE_EQUAL_SYMBOL_STR, (b)->g.h);

#define DUAL_UART_PRINT_SW_TXT(a, b, c, d, e, f, g, h, i, j,...) \
	a("%s %s.%s%s%d\n", DUAL_IN_DUMP_HEADER, #g, #h, DUAL_SPACE_EQUAL_SYMBOL_STR, (b)->g.h);

#define DUAL_UART_PRINT_TXT_HW(a, b, c, d, e, f, g, h,...) \
	a("%s %s%s0x%x\n", DUAL_IN_DUMP_HEADER, #h, DUAL_SPACE_EQUAL_SYMBOL_STR, (b)->h.Raw);

#define DUAL_UART_PRINT_TXT_ENGINE(a, b, c, d, e, f, g, h, i, j, k,...) \
	a("%s %s%s0x%x\n", DUAL_IN_DUMP_HEADER, #h, DUAL_SPACE_EQUAL_SYMBOL_STR, (b)->h.Raw);

#define DUAL_UART_PRINT_TXT_ENGINE_D(a, b, c, d, e, f, g, h, i, j, k,...) \
	a("%s %s%s0x%x\n", DUAL_IN_DUMP_HEADER, #i, DUAL_SPACE_EQUAL_SYMBOL_STR, (b)->i.Raw);

#define DUAL_UART_PRINT_TXT_ARRAY(a, b, c, d, e, f, g, h, i, j, k, l,...) \
	for (int temp_i=0;temp_i<l;temp_i++)\
	{\
		a("%s %s[%d]%s0x%x\n", DUAL_IN_DUMP_HEADER, #h, temp_i, DUAL_SPACE_EQUAL_SYMBOL_STR, (b)->h[temp_i].Raw);\
	}\

#define DUAL_UART_PRINT_TXT_ARRAY_D(a, b, c, d, e, f, g, h, i, j, k, l,...) \
	for (int temp_i=0;temp_i<l;temp_i++)\
	{\
		a("%s %s[%d]%s0x%x\n", DUAL_IN_DUMP_HEADER, #i, temp_i, DUAL_SPACE_EQUAL_SYMBOL_STR, (b)->i[temp_i].Raw);\
	}\

#define DUAL_UART_PRINT_TXT_ENGINE_OUT(a, b, c, d, e, f, g, h, i, j, k,...) \
	a("%s %s%s0x%x\n", DUAL_OUT_DUMP_HEADER, #h, DUAL_SPACE_EQUAL_SYMBOL_STR, (b)->h.Raw);

#define DUAL_UART_PRINT_TXT_ENGINE_OUT_D(a, b, c, d, e, f, g, h, i, j, k,...) \
	a("%s %s%s0x%x\n", DUAL_OUT_DUMP_HEADER, #i, DUAL_SPACE_EQUAL_SYMBOL_STR, (b)->i.Raw);

#define DUAL_UART_PRINT_TXT_ARRAY_OUT(a, b, c, d, e, f, g, h, i, j, k, l,...) \
	for (int temp_i=0;temp_i<l;temp_i++)\
	{\
		a("%s %s[%d]%s0x%x\n", DUAL_OUT_DUMP_HEADER, #h, temp_i, DUAL_SPACE_EQUAL_SYMBOL_STR, (b)->h[temp_i].Raw);\
	}\

#define DUAL_UART_PRINT_TXT_ARRAY_OUT_D(a, b, c, d, e, f, g, h, i, j, k, l,...) \
	for (int temp_i=0;temp_i<l;temp_i++)\
	{\
		a("%s %s[%d]%s0x%x\n", DUAL_OUT_DUMP_HEADER, #i, temp_i, DUAL_SPACE_EQUAL_SYMBOL_STR, (b)->i[temp_i].Raw);\
	}\

#define DUAL_LOG_REG_TXT_HW(a, b, c, d, e, f, g, h,...) \
	b(a, "%s 0x%08X%s0x%08x \\\\%s\n", DUAL_REG_DUMP_HEADER, (unsigned int)((char *)&((c)->h) - (char *)(c) + (d)), DUAL_SPACE_EQUAL_SYMBOL_STR, (c)->h.Raw, #h);

#define DUAL_LOG_REG_TXT_ENGINE(a, b, c, d, e, f, g, h, i, j, k,...) \
	b(a, "%s 0x%08X%s0x%08x \\\\%s\n", DUAL_REG_DUMP_HEADER, (unsigned int)((char *)&((c)->h) - (char *)(c) + (d)), DUAL_SPACE_EQUAL_SYMBOL_STR, (c)->h.Raw, #h);

#define DUAL_LOG_REG_TXT_ENGINE_D(a, b, c, d, e, f, g, h, i, j, k,...) \
	b(a, "%s 0x%08X%s0x%08x \\\\%s\n", DUAL_REG_DUMP_HEADER, (unsigned int)((char *)&((c)->i) - (char *)(c) + (d)), DUAL_SPACE_EQUAL_SYMBOL_STR, (c)->i.Raw, #i);

#define DUAL_LOG_REG_TXT_ARRAY(a, b, c, d, e, f, g, h, i, j, k, l,...) \
	for (int temp_i=0;temp_i<l;temp_i++)\
	{\
		b(a, "%s 0x%08X%s0x%08x \\\\%s[%d]\n", DUAL_REG_DUMP_HEADER, (unsigned int)((char *)&((c)->h[temp_i]) - (char *)(c) + (d)), DUAL_SPACE_EQUAL_SYMBOL_STR, (c)->h[temp_i].Raw, #h, temp_i);\
	}\

#define DUAL_LOG_REG_TXT_ARRAY_D(a, b, c, d, e, f, g, h, i, j, k, l,...) \
	for (int temp_i=0;temp_i<l;temp_i++)\
	{\
		b(a, "%s 0x%08X%s0x%08x \\\\%s[%d]\n", DUAL_REG_DUMP_HEADER, (unsigned int)((char *)&((c)->i[temp_i]) - (char *)(c) + (d)), DUAL_SPACE_EQUAL_SYMBOL_STR, (c)->i[temp_i].Raw, #i, temp_i);\
	}\

#define DUAL_UART_REG_TXT_HW(a, b, c, d, e, f, g, h,...) \
	a("%s 0x%08X%s0x%08x \\\\%s\n", DUAL_REG_DUMP_HEADER, (unsigned int)((char *)&((b)->h) - (char *)(b) + (c)), DUAL_SPACE_EQUAL_SYMBOL_STR, (b)->h.Raw, #h);

#define DUAL_UART_REG_TXT_ENGINE(a, b, c, d, e, f, g, h, i, j, k,...) \
	a("%s 0x%08X%s0x%08x \\\\%s\n", DUAL_REG_DUMP_HEADER, (unsigned int)((char *)&((b)->h) - (char *)(b) + (c)), DUAL_SPACE_EQUAL_SYMBOL_STR, (b)->h.Raw, #h);

#define DUAL_UART_REG_TXT_ENGINE_D(a, b, c, d, e, f, g, h, i, j, k,...) \
	a("%s 0x%08X%s0x%08x \\\\%s\n", DUAL_REG_DUMP_HEADER, (unsigned int)((char *)&((b)->i) - (char *)(b) + (c)), DUAL_SPACE_EQUAL_SYMBOL_STR, (b)->i.Raw, #i);

#define DUAL_UART_REG_TXT_ARRAY(a, b, c, d, e, f, g, h, i, j, k, l,...) \
	for (int temp_i=0;temp_i<l;temp_i++)\
	{\
		a("%s 0x%08X%s0x%08x \\\\%s[%d]\n", DUAL_REG_DUMP_HEADER, (unsigned int)((char *)&((b)->h[temp_i]) - (char *)(b) + (c)), DUAL_SPACE_EQUAL_SYMBOL_STR, (b)->h[temp_i].Raw, #h, temp_i);\
	}\

#define DUAL_UART_REG_TXT_ARRAY_D(a, b, c, d, e, f, g, h, i, j, k, l,...) \
	for (int temp_i=0;temp_i<l;temp_i++)\
	{\
		a("%s 0x%08X%s0x%08x \\\\%s[%d]\n", DUAL_REG_DUMP_HEADER, (unsigned int)((char *)&((b)->i[temp_i]) - (char *)(b) + (c)), DUAL_SPACE_EQUAL_SYMBOL_STR, (b)->i[temp_i].Raw, #i, temp_i);\
	}\

#define DUAL_READ_KNOBS_VAR(a, b, c, d, e, f, g, h, i, j,...) \
	if (false == (d))\
	{\
		if (0 == memcmp(a, #i, sizeof(#i)))\
		{\
			(c)->g.h = (b);\
			(d) = true;\
		}\
	}\

#define DUAL_CONVERT_REG_TO_DUAL(a, b, c, d, e, f, g, h,...) (b)->h.Raw = (a)->ptr_isp_reg->h.Raw;
#define DUAL_CONVERT_UNI_TO_DUAL(a, b, c, d, e, f, g, h,...) (b)->h.Raw = (a)->ptr_isp_uni->h.Raw;
#define DUAL_CONVERT_DUAL_TO_UNI(a, b, c, d, e, f, g, h,...) (a)->h.Raw = (b)->h.Raw;

#define DUAL_COPY_DUAL_A_A(a, b, c, d, e, f, g, h, i, j, k,...) (a)->h.Raw = (b)->h.Raw;
#define DUAL_COPY_DUAL_A_B(a, b, c, d, e, f, g, h, i, j, k,...) (a)->i.Raw = (b)->h.Raw;
#define DUAL_COPY_DUAL_B_B(a, b, c, d, e, f, g, h, i, j, k,...) (a)->i.Raw = (b)->i.Raw;

#define DUAL_COPY_ARRAY_A_A(a, b, c, d, e, f, g, h, i, j, k, l,...) \
	for (int temp_i=0;temp_i<l;temp_i++)\
	{\
		(a)->h[temp_i].Raw = (b)->h[temp_i].Raw;\
	}\

#define DUAL_COPY_ARRAY_A_B(a, b, c, d, e, f, g, h, i, j, k, l,...) \
	for (int temp_i=0;temp_i<l;temp_i++)\
	{\
		(a)->i[temp_i].Raw = (b)->h[temp_i].Raw;\
	}\

#define DUAL_COPY_ARRAY_B_B(a, b, c, d, e, f, g, h, i, j, k, l,...) \
	for (int temp_i=0;temp_i<l;temp_i++)\
	{\
		(a)->i[temp_i].Raw = (b)->i[temp_i].Raw;\
	}\

#define DUAL_CONFIG_SW_LUT_DECLARE(a, b, c, d, e, f, g, h, i, j,...) int j;
#define DUAL_CONFIG_HW_LUT_DECLARE(a, b, c, d, e, f, g, h, i, j,...) int i;

typedef struct DUAL_CONFIG_LUT_STRUCT
{
	DUAL_SW_LIST(DUAL_CONFIG_SW_LUT_DECLARE,,,,,,);
	DUAL_HW_UNI_LIST(DUAL_CONFIG_HW_LUT_DECLARE,,,,,,);
	DUAL_HW_A_LIST(DUAL_CONFIG_HW_LUT_DECLARE,,,,,,);
	DUAL_HW_B_LIST(DUAL_CONFIG_HW_LUT_DECLARE,,,,,,);
}DUAL_CONFIG_LUT_STRUCT;

/* error enum */
typedef enum DUAL_DATA_TYPE_ENUM
{
    DUAL_DATA_UINT_ENUM=0,
    DUAL_DATA_INT_ENUM,
}DUAL_DATA_TYPE_ENUM;

typedef struct RAL_MAP_STRUCT
{
    bool cmodel_found_flag;
    int ral_found_count;
	unsigned int addr;
	int start_bits;
    char c_model_name[DUAL_MAX_NAME_SIZE];
    char ral_reg_name[DUAL_MAX_NAME_SIZE];
    char ral_field_name[DUAL_MAX_NAME_SIZE];
	DUAL_DATA_TYPE_ENUM data_type;
    unsigned int knobs_value;
    unsigned int dump_value;
    int value;
	unsigned int mask;
	int bit_length;
    bool dump_found_flag;
    bool append_flag;
    int append_value;
	int dual_no;
}RAL_MAP_STRUCT;

typedef struct VAR_MAP_STRUCT
{
    char var_name[DUAL_MAX_NAME_SIZE];
    int value;
    bool var_found_flag;
}VAR_MAP_STRUCT;

typedef struct RAL_DESCRIPTION_STRUCT
{
    int used_no;
    int max_no;
    RAL_MAP_STRUCT reg_list[DUAL_MAX_REG_NO];
    int used_var_no;
    int max_var_no;
    VAR_MAP_STRUCT var_list[DUAL_MAX_VAR_NO];
}RAL_DESCRIPTION_STRUCT;

extern bool dual_sel_check_support(int dual_sel);
#endif
