#ifndef __DUAL_CAL_H__
#define __DUAL_CAL_H__

#define DUAL_PLATFORM_DRIVER /* to change define with platform unique */
#define LOG_DEFINE_WORKAROUND /*temp work around due to __arm__ is not define*/

#if defined(_MSC_VER)
#include <conio.h>
#define dual_kbhit() _kbhit()
#define dual_stricmp                                    _stricmp
#define DUAL_FOLDER_SYMBOL_STR                          "\\"
#define DUAL_FOLDER_SYMBOL_CHAR                         '\\'
#define dual_sprintf(dst_ptr, size_dst, format, ...)    sprintf_s(dst_ptr, size_dst, format, __VA_ARGS__)
#define dual_fopen(file_ptr, filename_ptr, mode)        fopen_s(&file_ptr, filename_ptr, mode)
#define dual_fscanf                                     fscanf_s
#define DUAL_MOVE_CMD                                   "move"
#define DUAL_COPY_CMD                                   "copy"
#define DUAL_CMP_CMD                                    "fc"
#define DUAL_DEL_CMD                                    "del"
#define dual_sscanf_1(...)                              sscanf_s(__VA_ARGS__)
#define dual_sscanf_2(...)                              sscanf_s(__VA_ARGS__)
#define dual_sscanf_8(...)                              sscanf_s(__VA_ARGS__)
#define DUAL_DEFAULT_PATH ".\\dual_conf"
#elif defined(linux) || defined(__linux) || defined(__arm__)
#define dual_kbhit() true
#define dual_stricmp                                    strcasecmp
#define DUAL_FOLDER_SYMBOL_STR                          "/"
#define DUAL_FOLDER_SYMBOL_CHAR                         '/'
#define dual_sprintf(dst_ptr, size_dst, ...)            sprintf(dst_ptr, __VA_ARGS__)
#define dual_fopen(file_ptr, filename_ptr, mode)        file_ptr = fopen(filename_ptr, mode)
#define dual_fscanf                                     fscanf
#define DUAL_MOVE_CMD                                   "mv"
#define DUAL_COPY_CMD                                   "cp"
#define DUAL_CMP_CMD                                    "cmp"
#define DUAL_DEL_CMD                                    "rm"
#define dual_sscanf_1(x, y, a, b)                       sscanf(x, y, a)
#define dual_sscanf_2(x, y, a, b, c, d)                 sscanf(x, y, a, b)
#define dual_sscanf_8(x, y, a, b, c, d, e, f, g, h,...) sscanf(x, y, a, b, c, d, e, f, g, h)
#define O_BINARY (0)
#define DUAL_DEFAULT_PATH "./dual_conf"
#else
#error("Non-supported c compiler environment\n")
#endif
#ifdef LOG_DEFINE_WORKAROUND
    #pragma message("FPGA/EVB/Phone environment")
    #define LOG_TAG     "Dual_cal"
    #include <mtkcam/utils/std/Log.h>
    #define uart_printf(fmt, arg...) CAM_LOGE("[%s]" fmt"\n", __FUNCTION__, ##arg)
    #define dual_driver_printf(...) {CAM_LOGE(__VA_ARGS__);}
    #define dual_driver_printf_no_prefix(...) {CAM_LOGE(__VA_ARGS__);}
#else
    #ifdef DUAL_PLATFORM_DRIVER
    #if defined(_MSC_VER)           // VC
    #define dual_driver_printf dual_printf_prefix
    #define dual_driver_printf_no_prefix printf
    #define uart_printf printf
    #elif defined(linux) || defined(__linux) //Linux
    #if defined(__arm__) //arm
    #define NEW_LINE_CHAR   "\n"
    #if defined(USING_MTK_LDVT)     // LDVT
        #pragma message("LDVT environment")
        #include "uvvf.h"
        #define LOG_TAG     "Dual_cal"
        #define uart_printf(fmt, arg...) VV_MSG(LOG_TAG "[%s] " fmt NEW_LINE_CHAR, __func__, ##arg)
        #define dual_driver_printf(...) {VV_ERRMSG( "[%s][%d]",__FUNCTION__,__LINE__);VV_MSG( __VA_ARGS__);}
        #define dual_driver_printf_no_prefix(...) {VV_ERRMSG( "[%s][%d]",__FUNCTION__,__LINE__);VV_MSG( __VA_ARGS__);}
    #else //not LDVT
    #pragma message("FPGA/EVB/Phone environment")
        //#include <cutils/log.h>        // Android
        #define LOG_TAG     "Dual_cal"
        #include <mtkcam/utils/std/Log.h>
        #define uart_printf(fmt, arg...) CAM_LOGE("[%s]" fmt"\n", __FUNCTION__, ##arg)
        #define dual_driver_printf(...) {CAM_LOGE(__VA_ARGS__);}
        #define dual_driver_printf_no_prefix(...) {CAM_LOGE(__VA_ARGS__);}
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

#define TWIN_SCENARIO_NORMAL        (0x0)
#define TWIN_SCENARIO_AF_FAST_P1    (0x1)
#define TWIN_SCENARIO_AF_FAST_P2    (0x2)

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

#define DUAL_MODE_CAL(dual_sel) (((dual_sel & (0x10 + 0x1))?1:0) + ((dual_sel & 0x2)?1:0) + ((dual_sel & 0x4)?1:0) + ((dual_sel & 0x8)?1:0))
#define DUAL_MOD(num, denom) (((denom)==1)?0:(((denom)==2)?((num)&0x1):(((denom)==4)?\
    ((num)&0x3):(((denom)==8)?((num)&0x7):((num)%(denom))))))
#define DUAL_INT_DIV(num, denom) (((denom)==1)?(num):(((denom)==2)?((unsigned int)(num)>>0x1):\
    (((denom)==4)?((unsigned int)(num)>>0x2):(((denom)==8)?((unsigned int)(num)>>0x3):((num)/(denom))))))

#define DUAL_RAW_WD         (4348)
#define DUAL_RAW_WD_D       (4348)
#define DUAL_RRZ_WD_ALL     (6292)
#define DUAL_RSS_R2_WD_ALL  (2752)
#define DUAL_CRZ_R1_WD_ALL  (640)
#define DUAL_CRZ_R2_WD_ALL  (768)

/* macro for debug print to file, #include <stdio.h> */
#define dual_printf_prefix(...) {\
    const char *ptr_char = get_current_file_name(__FILE__);\
    printf("[%s][%s][%d] ", ptr_char,  __FUNCTION__, __LINE__);\
    printf(__VA_ARGS__);\
}

#define DUAL_MAX_NAME_SIZE (128)
#define DUAL_MAX_REG_NO (90000)
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
    CMD(n, DUAL_MESSAGE_RSS_SCALING_UP_ERROR)\
	CMD(n, DUAL_MESSAGE_RSS_XS_XE_CAL_ERROR)\
    CMD(n, DUAL_MESSAGE_CRZ_SCALING_UP_ERROR)\
	CMD(n, DUAL_MESSAGE_CRZ_XS_XE_CAL_ERROR)\
    CMD(n, DUAL_MESSAGE_RAL_NO_OVER_SIZE_ERROR)\
    CMD(n, DUAL_MESSAGE_VAR_NO_OVER_SIZE_ERROR)\
    CMD(n, DUAL_MESSAGE_RAL_MASK_CHECK_ERROR)\
    CMD(n, DUAL_MESSAGE_INVALID_RRZ_IN_CROP_HT_ERROR)\
    CMD(n, DUAL_MESSAGE_INVALID_RSS_IN_CROP_HT_ERROR)\
    CMD(n, DUAL_MESSAGE_INVALID_CRZ_IN_CROP_HT_ERROR)\
    CMD(n, DUAL_MESSAGE_INVALID_DUAL_SEL_ERROR)\
    CMD(n, DUAL_MESSAGE_INVALID_LE_INV_CTL_ERROR)\
    CMD(n, DUAL_MESSAGE_INVALID_AF_BLK_XSIZE_ERROR)\
    CMD(n, DUAL_MESSAGE_INVALID_AF_BLK_NUM_ERROR)\
    CMD(n, DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR)\
    CMD(n, DUAL_MESSAGE_INVALID_AF_SGG_CONFIG_ERROR)\
    CMD(n, DUAL_MESSAGE_INVALID_PIX_BUS_SEPI_ERROR)\
    CMD(n, DUAL_MESSAGE_INVALID_PIX_BUS_SEPO_ERROR)\
    CMD(n, DUAL_MESSAGE_INVALID_PIX_BUS_AMXO_ERROR)\
    CMD(n, DUAL_MESSAGE_INVALID_PIX_BUS_BMXO_ERROR)\
    CMD(n, DUAL_MESSAGE_INVALID_LSCI_STRIDE_ERROR)\
    CMD(n, DUAL_MESSAGE_INVALID_PDO_STRIDE_ERROR)\
    CMD(n, DUAL_MESSAGE_SEP_IN_ALIGN_ERROR)\
    CMD(n, DUAL_MESSAGE_SEP_OUT_ALIGN_ERROR)\
    CMD(n, DUAL_MESSAGE_BIN_OUT_ALIGN_ERROR)\
    CMD(n, DUAL_MESSAGE_BMX_OUT_ALIGN_ERROR)\
    CMD(n, DUAL_MESSAGE_RMX_OUT_ALIGN_ERROR)\
    CMD(n, DUAL_MESSAGE_SEP_SIZE_CAL_ERROR)\
    CMD(n, DUAL_MESSAGE_CRP_R3_SIZE_CAL_ERROR)\
    CMD(n, DUAL_MESSAGE_MRG_R1_SIZE_CAL_ERROR)\
    CMD(n, DUAL_MESSAGE_MRG_R2_SIZE_CAL_ERROR)\
    CMD(n, DUAL_MESSAGE_MRG_R3_SIZE_CAL_ERROR)\
    CMD(n, DUAL_MESSAGE_MRG_R4_SIZE_CAL_ERROR)\
    CMD(n, DUAL_MESSAGE_MRG_R5_SIZE_CAL_ERROR)\
    CMD(n, DUAL_MESSAGE_MRG_R6_SIZE_CAL_ERROR)\
    CMD(n, DUAL_MESSAGE_MRG_R7_SIZE_CAL_ERROR)\
    CMD(n, DUAL_MESSAGE_MRG_R8_SIZE_CAL_ERROR)\
    CMD(n, DUAL_MESSAGE_MRG_R9_SIZE_CAL_ERROR)\
    CMD(n, DUAL_MESSAGE_MRG_R10_SIZE_CAL_ERROR)\
    CMD(n, DUAL_MESSAGE_UFEO_CONFIG_ERROR)\
    CMD(n, DUAL_MESSAGE_UFGO_CONFIG_ERROR)\
    CMD(n, DUAL_MESSAGE_RRZO_CONFIG_ERROR)\
    CMD(n, DUAL_MESSAGE_PSB_CONFIG_ERROR)\
    CMD(n, DUAL_MESSAGE_RMB_CONFIG_ERROR)\
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
    DUAL_MESSAGE_UNKNOWN = 0,
    DUAL_ERROR_MESSAGE_DATA(,DUAL_ENUM_DECLARE)
}DUAL_MESSAGE_ENUM;

/* tile loss */
#define DUAL_TILE_LOSS_IBPC         (ptr_in_param->CAMCTL_R1A_CAMCTL_EN4.Bits.CAMCTL_IBPC_R1_EN? 4: 0)
#define DUAL_TILE_LOSS_IOBC         (ptr_in_param->CAMCTL_R1A_CAMCTL_EN4.Bits.CAMCTL_IOBC_R1_EN? 8: 0)
#define DUAL_TILE_LOSS_IRM          (ptr_in_param->CAMCTL_R1A_CAMCTL_EN4.Bits.CAMCTL_IRM_R1_EN? 4: 0)
#define DUAL_TILE_LOSS_OBC          (ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_OBC_R1_EN? 6: 0)
#define DUAL_TILE_LOSS_BPC          (ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_BPC_R1_EN? 4: 0)
#define DUAL_TILE_LOSS_FUS          (ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_FUS_R1_EN? 4: 0)
#define DUAL_TILE_LOSS_RNR          (ptr_in_param->CAMCTL_R1A_CAMCTL_EN4.Bits.CAMCTL_RNR_R1_EN? 6: 0)
#define DUAL_TILE_LOSS_HLR          (2)
#define DUAL_TILE_LOSS_LTM          (2)
#define DUAL_TILE_LOSS_ALL          (DUAL_TILE_LOSS_IBPC + \
                                     DUAL_TILE_LOSS_IOBC + \
                                     DUAL_TILE_LOSS_IRM + \
                                     DUAL_TILE_LOSS_OBC + \
                                     DUAL_TILE_LOSS_BPC + \
                                     DUAL_TILE_LOSS_FUS + \
                                     DUAL_TILE_LOSS_RNR + \
                                     DUAL_TILE_LOSS_HLR + \
                                     DUAL_TILE_LOSS_LTM)
#define DUAL_TILE_LOSS_ALL_L        (DUAL_TILE_LOSS_IBPC + \
                                     DUAL_TILE_LOSS_IOBC + \
                                     DUAL_TILE_LOSS_IRM + \
                                     DUAL_TILE_LOSS_OBC + \
                                     DUAL_TILE_LOSS_BPC + \
                                     DUAL_TILE_LOSS_FUS + \
                                     DUAL_TILE_LOSS_RNR + \
                                     DUAL_TILE_LOSS_HLR + \
                                     DUAL_TILE_LOSS_LTM)

#define DUAL_TILE_LOSS_AF_B         (DUAL_TILE_LOSS_IBPC + \
                                     DUAL_TILE_LOSS_IOBC + \
                                     DUAL_TILE_LOSS_IRM + \
                                     DUAL_TILE_LOSS_OBC + \
                                     DUAL_TILE_LOSS_BPC + \
                                     DUAL_TILE_LOSS_FUS + \
                                     DUAL_TILE_LOSS_RNR)

#define DUAL_TILE_LOSS_RRZ          (8)
#define DUAL_TILE_LOSS_RRZ_L        (6)
#define DUAL_TILE_LOSS_RRZ_TAP      (5) /* 2 tp: 2, 6 tp: 4, 8 tp: 5 */
#define DUAL_TILE_LOSS_RRZ_TAP_L    (3) /* 2 tp: 1, 6 tp: 2, 8 tp: 3 */

#define DUAL_TILE_LOSS_YUV          (8)
#define DUAL_TILE_LOSS_YUV_L        (10)

#define DUAL_TILE_LOSS_RSS_R2       (DUAL_TILE_LOSS_YUV + 8 * 2)   // 4 * n / 2
#define DUAL_TILE_LOSS_RSS_R2_L     (DUAL_TILE_LOSS_YUV_L + 8 * 2)   // 4 * n / 2

#define DUAL_TILE_LOSS_CRZ_R1       (DUAL_TILE_LOSS_YUV + ((8 + 3) / 4) * 2)   // n / 2
#define DUAL_TILE_LOSS_CRZ_R1_L     (DUAL_TILE_LOSS_YUV_L + ((8 + 3) / 4) * 2)   // n / 2

#define DUAL_TILE_LOSS_CRZ_R2       (DUAL_TILE_LOSS_YUV + ((8 + 3) / 4) * 2)
#define DUAL_TILE_LOSS_CRZ_R2_L     (DUAL_TILE_LOSS_YUV_L + ((8 + 3) / 4) * 2)

#define DUAL_AF_TAPS                (12) /* must be even */
#define DUAL_AF_MAX_BLOCK_WIDTH     (80) /* must be even */
#define DUAL_AF_MIN_IN_WIDTH        (56) /* must be even */
#define DUAL_AF_LEFT_MARGIN         (14)
#define DUAL_AF_RGIHT_MARGIN        (16)
#define DUAL_AF_BLOCK_BYTE          (48) /* must be even */

#define DULL_MRG_R11B_MAX_WD        (128)
#define DULL_MRG_R11C_MAX_WD        (128)
#define DULL_MRG_R12B_MAX_WD        (64)
#define DULL_MRG_R12C_MAX_WD        (64)
#define DULL_MRG_R14B_MAX_WD        (128)
#define DULL_MRG_R14C_MAX_WD        (128)

#define DUAL_RSS_TILE_LOSS_4_TAP    (2)
#define DUAL_RSS_TILE_LOSS_4_TAP_L  (1)
#define DUAL_RSS_PREC_BITS_CUB_ACC  (20)
#define DUAL_RSS_PREC_VAL_CUB_ACC   (1 << DUAL_RSS_PREC_BITS_CUB_ACC)
#define DUAL_RSS_CONFIG_BITS        (20)

#define DUAL_CRZ_TILE_LOSS_2_TAP    (2)
#define DUAL_CRZ_TILE_LOSS_2_TAP_L  (1)
#define DUAL_CRZ_TILE_LOSS_2_TAP_UV (2)
#define DUAL_CRZ_PREC_BITS_2_TAP    (15)
#define DUAL_CRZ_PREC_VAL_2_TAP     (1 << DUAL_CRZ_PREC_BITS_2_TAP)
#define DUAL_CRZ_PREC_BITS_SRC_ACC  (20)
#define DUAL_CRZ_PREC_VAL_SRC_ACC   (1 << DUAL_CRZ_PREC_BITS_SRC_ACC)
#define DUAL_CRZ_CONFIG_BITS        (20)

#define DUAL_RRZ_PREC               (32768)

#define DUAL_ALIGN_PIXEL_MODE(n)    (1<<(n))
#define DUAL_RRZ_ALIGN(n)           (2 << (n))
#define DUAL_RRZ_MASK(n)            ((2 << (n)) - 1)
#define DUAL_RSS_ALIGN(n)           (2 << (n))
#define DUAL_RSS_MASK(n)            ((2 << (n)) - 1)
#define DUAL_CRZ_ALIGN(n)           (2 << (n))
#define DUAL_CRZ_MASK(n)            ((2 << (n)) - 1)
#define DUAL_RLB_ALIGN_SHIFT        (2) /* 8, 2<<2 */
#define DUAL_RLB_ALIGN_PIX          (2 << (DUAL_RLB_ALIGN_SHIFT))

/* RRZO: RAW format */
#define DUAL_OUT_FMT_BAYER_8        (8)
#define DUAL_OUT_FMT_BAYER_10       (9)
#define DUAL_OUT_FMT_BAYER_12       (10)
#define DUAL_OUT_FMT_BAYER_14       (11)
#define DUAL_OUT_FMT_BAYER_8_2      (12)
#define DUAL_OUT_FMT_BAYER_10_2     (13)
#define DUAL_OUT_FMT_BAYER_12_2     (14)
#define DUAL_OUT_FMT_BAYER_14_2     (15)
#define DUAL_OUT_FMT_MIPI_10        (16)
#define DUAL_OUT_FMT_BAYER_15       (17)
#define DUAL_OUT_FMT_BAYER_16       (18)
#define DUAL_OUT_FMT_BAYER_19       (19)
#define DUAL_OUT_FMT_BAYER_24       (20)
#define DUAL_OUT_FMT_FG_8           (0)
#define DUAL_OUT_FMT_FG_10          (1)
#define DUAL_OUT_FMT_FG_12          (2)
#define DUAL_OUT_FMT_FG_14          (3)

/* YUVO: YUV format */
#define DUAL_OUT_FMT_YUV422         (0)
#define DUAL_OUT_FMT_YUV422_2       (1)
#define DUAL_OUT_FMT_YUV422_3       (2)
#define DUAL_OUT_FMT_YUV420_2       (3)
#define DUAL_OUT_FMT_YUV420_3       (4)
#define DUAL_OUT_FMT_Y              (5)

/* CRZO: YUV format */
#define DUAL_OUT_FMT_YUV422_8       (0)
#define DUAL_OUT_FMT_Y_8            (1)

#define DUAL_UFO_AU_BIT             (6)
#define DUAL_UFO_AU_SIZE            (1 << DUAL_UFO_AU_BIT)

#define DUAL_CALC_BUS_XIZE(pixel_width, bit_per_pixel, bus_cut_shift) \
    (((unsigned int)(((unsigned int)pixel_width) * (bit_per_pixel) + (1 << (bus_cut_shift + 3)) - 1) >> (bus_cut_shift + 3)) * (1 << bus_cut_shift) - 1)

/* PLATFORM parameters */
/* a, b, c, d, e, f reserved */
/* support mode */
#define DUAL_SEL_SUPPORT_LIST(CMD, a, b, c, d, e, f) \
    /* LSC */\
    CMD(a, b, c, d, e, f, 0x3) /* LSB:110|0:MSB */\
    CMD(a, b, c, d, e, f, 0x5) /* LSB:101|0:MSB */\
    CMD(a, b, c, d, e, f, 0x6) /* LSB:011|0:MSB */\
    CMD(a, b, c, d, e, f, 0x7) /* LSB:111|0:MSB */\
    CMD(a, b, c, d, e, f, 0x16)/* LSB:01101:MSB */\

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
    CMD(a, b, c, d, e, f, SW, TWIN_RRZ_OUT_WD, TWIN_RRZ_OUT_WD, TWIN_RRZ_OUT_WD)\
    /* RSS_R2 DZ */\
    CMD(a, b, c, d, e, f, SW, TWIN_RSS_R2_IN_CROP_HT, TWIN_RSS_R2_IN_CROP_HT, TWIN_RSS_R2_IN_CROP_HT)\
    CMD(a, b, c, d, e, f, SW, TWIN_RSS_R2_HORI_INT_OFST, TWIN_RSS_R2_HORI_INT_OFST, TWIN_RSS_R2_HORI_INT_OFST)\
    CMD(a, b, c, d, e, f, SW, TWIN_RSS_R2_HORI_INT_OFST_LAST, TWIN_RSS_R2_HORI_INT_OFST_LAST, TWIN_RSS_R2_HORI_INT_OFST_LAST)\
    CMD(a, b, c, d, e, f, SW, TWIN_RSS_R2_HORI_SUB_OFST, TWIN_RSS_R2_HORI_SUB_OFST, TWIN_RSS_R2_HORI_SUB_OFST)\
    CMD(a, b, c, d, e, f, SW, TWIN_RSS_R2_OUT_WD, TWIN_RSS_R2_OUT_WD, TWIN_RSS_R2_OUT_WD)\
    /* CRZ_R1 DZ */\
    CMD(a, b, c, d, e, f, SW, TWIN_CRZ_R1_IN_CROP_HT, TWIN_CRZ_R1_IN_CROP_HT, TWIN_CRZ_R1_IN_CROP_HT)\
    CMD(a, b, c, d, e, f, SW, TWIN_CRZ_R1_HORI_INT_OFST, TWIN_CRZ_R1_HORI_INT_OFST, TWIN_CRZ_R1_HORI_INT_OFST)\
    CMD(a, b, c, d, e, f, SW, TWIN_CRZ_R1_HORI_INT_OFST_LAST, TWIN_CRZ_R1_HORI_INT_OFST_LAST, TWIN_CRZ_R1_HORI_INT_OFST_LAST)\
    CMD(a, b, c, d, e, f, SW, TWIN_CRZ_R1_HORI_SUB_OFST, TWIN_CRZ_R1_HORI_SUB_OFST, TWIN_CRZ_R1_HORI_SUB_OFST)\
    CMD(a, b, c, d, e, f, SW, TWIN_CRZ_R1_OUT_WD, TWIN_CRZ_R1_OUT_WD, TWIN_CRZ_R1_OUT_WD)\
    /* CRZ_R2 DZ */\
    CMD(a, b, c, d, e, f, SW, TWIN_CRZ_R2_IN_CROP_HT, TWIN_CRZ_R2_IN_CROP_HT, TWIN_CRZ_R2_IN_CROP_HT)\
    CMD(a, b, c, d, e, f, SW, TWIN_CRZ_R2_HORI_INT_OFST, TWIN_CRZ_R2_HORI_INT_OFST, TWIN_CRZ_R2_HORI_INT_OFST)\
    CMD(a, b, c, d, e, f, SW, TWIN_CRZ_R2_HORI_INT_OFST_LAST, TWIN_CRZ_R2_HORI_INT_OFST_LAST, TWIN_CRZ_R2_HORI_INT_OFST_LAST)\
    CMD(a, b, c, d, e, f, SW, TWIN_CRZ_R2_HORI_SUB_OFST, TWIN_CRZ_R2_HORI_SUB_OFST, TWIN_CRZ_R2_HORI_SUB_OFST)\
    CMD(a, b, c, d, e, f, SW, TWIN_CRZ_R2_OUT_WD, TWIN_CRZ_R2_OUT_WD, TWIN_CRZ_R2_OUT_WD)\
    /* AF */\
    CMD(a, b, c, d, e, f, SW, TWIN_AF_OFFSET, TWIN_AF_OFFSET, TWIN_AF_OFFSET)\
    CMD(a, b, c, d, e, f, SW, TWIN_AF_BLOCK_XNUM, TWIN_AF_BLOCK_XNUM, TWIN_AF_BLOCK_XNUM)\
    /* RAWI */\
    CMD(a, b, c, d, e, f, SW, TWIN_RAWI_XSIZE, TWIN_RAWI_IN_WD, TWIN_RAWI_XSIZE)\
    CMD(a, b, c, d, e, f, SW, TWIN_RAWI_YSIZE, TWIN_RAWI_IN_HT, TWIN_RAWI_YSIZE)\
    /* SCRNARIO */\
    CMD(a, b, c, d, e, f, SW, TWIN_SCENARIO, TWIN_SCENARIO, TWIN_SCENARIO)\

/* PLATFORM parameters */
/* AAA: reg, BBB: field, CCC: c model variable */
/* R/W type: R, RW, W */
#define DUAL_HW_UNI_LIST(CMD, a, b, c, d, e, f) \


/* PLATFORM parameters */
/* a, b, c, d, e, f reserved */
/* struct name */
/* variable name */
/* R/W type: R, RW, W */
#define DUAL_HW_A_LIST(CMD, a, b, c, d, e, f) \
    /* TG_R1 */\
    CMD(a, b, c, d, e, f, TG_R1A_TG_SEN_GRAB_PXL, TG_PXL_S, TG_R1A_PXL_S, U)\
    CMD(a, b, c, d, e, f, TG_R1A_TG_SEN_GRAB_PXL, TG_PXL_E, TG_R1A_PXL_E, U)\
    CMD(a, b, c, d, e, f, TG_R1A_TG_SEN_GRAB_LIN, TG_LIN_S, TG_R1A_LIN_S, U)\
    CMD(a, b, c, d, e, f, TG_R1A_TG_SEN_GRAB_LIN, TG_LIN_E, TG_R1A_LIN_E, U)\
    /* RAWI_R2 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_DMA_EN, CAMCTL_RAWI_R2_EN, CAMCTL_R1A_RAWI_R2_EN, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_FMT2_SEL, CAMCTL_PIX_BUS_RAWI_R2, CAMCTL_R1A_PIX_BUS_RAWI_R2, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_FMT3_SEL, CAMCTL_RAWI_R2_FMT, CAMCTL_R1A_RAWI_R2_FMT, U)\
    CMD(a, b, c, d, e, f, RAWI_R2A_RAWI_OFST_ADDR, RAWI_OFST_ADDR, RAWI_R2A_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, RAWI_R2A_RAWI_XSIZE, RAWI_XSIZE, RAWI_R2A_XSIZE, U)\
    CMD(a, b, c, d, e, f, RAWI_R2A_RAWI_YSIZE, RAWI_YSIZE, RAWI_R2A_YSIZE, U)\
    CMD(a, b, c, d, e, f, RAWI_R2A_RAWI_STRIDE, RAWI_STRIDE, RAWI_R2A_STRIDE, U)\
    /* UNP_R2 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN, CAMCTL_UNP_R2_EN, CAMCTL_R1A_UNP_R2_EN, U)\
    CMD(a, b, c, d, e, f, UNP_R2A_UNP_OFST, UNP_OFST_STB, UNP_R2A_OFST_STB, U)\
    CMD(a, b, c, d, e, f, UNP_R2A_UNP_OFST, UNP_OFST_EDB, UNP_R2A_OFST_EDB, U)\
    /* UFDI_R2 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_DMA_EN, CAMCTL_UFDI_R2_EN, CAMCTL_R1A_UFDI_R2_EN, U)\
    CMD(a, b, c, d, e, f, UFDI_R2A_UFDI_XSIZE, UFDI_XSIZE, UFDI_R2A_XSIZE, U)\
    CMD(a, b, c, d, e, f, UFDI_R2A_UFDI_YSIZE, UFDI_YSIZE, UFDI_R2A_YSIZE, U)\
    CMD(a, b, c, d, e, f, UFDI_R2A_UFDI_STRIDE, UFDI_STRIDE, UFDI_R2A_STRIDE, U)\
    /* UFD_R2 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN3, CAMCTL_UFD_R2_EN, CAMCTL_R1A_UFD_R2_EN, U)\
    CMD(a, b, c, d, e, f, UFD_R2A_UFD_AU_CON, UFD_AU_OFST, UFD_R2A_AU_OFST, U)\
    CMD(a, b, c, d, e, f, UFD_R2A_UFD_AU_CON, UFD_AU_SIZE, UFD_R2A_AU_SIZE, U)\
    CMD(a, b, c, d, e, f, UFD_R2A_UFD_AU2_CON, UFD_AU2_OFST, UFD_R2A_AU2_OFST, U)\
    CMD(a, b, c, d, e, f, UFD_R2A_UFD_AU2_CON, UFD_AU2_SIZE, UFD_R2A_AU2_SIZE, U)\
    CMD(a, b, c, d, e, f, UFD_R2A_UFD_BS2_AU_CON, UFD_BS2_AU_START, UFD_R2A_BS2_AU_START, U)\
    CMD(a, b, c, d, e, f, UFD_R2A_UFD_BS2_AU_CON, UFD_BOND_MODE, UFD_R2A_BOND_MODE, U)\
    CMD(a, b, c, d, e, f, UFD_R2A_UFD_AU3_CON, UFD_AU3_OFST, UFD_R2A_AU3_OFST, U)\
    CMD(a, b, c, d, e, f, UFD_R2A_UFD_AU3_CON, UFD_AU3_SIZE, UFD_R2A_AU3_SIZE, U)\
    CMD(a, b, c, d, e, f, UFD_R2A_UFD_BS3_AU_CON, UFD_BS3_AU_START, UFD_R2A_BS3_AU_START, U)\
    CMD(a, b, c, d, e, f, UFD_R2A_UFD_BS3_AU_CON, UFD_BOND2_MODE, UFD_R2A_BOND2_MODE, U)\
    CMD(a, b, c, d, e, f, UFD_R2A_UFD_CROP_CON1, UFD_X_START, UFD_R2A_X_START, U)\
    CMD(a, b, c, d, e, f, UFD_R2A_UFD_CROP_CON1, UFD_X_END, UFD_R2A_X_END, U)\
    CMD(a, b, c, d, e, f, UFD_R2A_UFD_CROP_CON2, UFD_Y_START, UFD_R2A_Y_START, U)\
    CMD(a, b, c, d, e, f, UFD_R2A_UFD_CROP_CON2, UFD_Y_END, UFD_R2A_Y_END, U)\
    CMD(a, b, c, d, e, f, UFD_R2A_UFD_SIZE_CON, UFD_WD, UFD_R2A_WD, U)\
    CMD(a, b, c, d, e, f, UFD_R2A_UFD_SIZE_CON, UFD_HT, UFD_R2A_HT, U)\
    CMD(a, b, c, d, e, f, UFD_R2A_UFD_CON, UFD_SEL, UFD_R2A_SEL, U)\
    CMD(a, b, c, d, e, f, UFD_R2A_UFD_ADDRESS_CON1, UFD_BITSTREAM2_OFST_ADDR, UFD_R2A_BITSTREAM2_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, UFD_R2A_UFD_ADDRESS_CON2, UFD_BITSTREAM3_OFST_ADDR, UFD_R2A_BITSTREAM3_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, UFD_R2A_UFD_ADDRESS_CON3, UFD_BITSTREAM4_OFST_ADDR, UFD_R2A_BITSTREAM4_OFST_ADDR, U)\
    /* DCPN_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN4, CAMCTL_DCPN_R1_EN, CAMCTL_R1A_DCPN_R1_EN, U)\
    /* DBN_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN2, CAMCTL_DBN_R1_EN, CAMCTL_R1A_DBN_R1_EN, U)\
    /* BIN_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN2, CAMCTL_BIN_R1_EN, CAMCTL_R1A_BIN_R1_EN, U)\
    /* SEL */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_SEL, CAMCTL_RAW_SEL, CAMCTL_R1A_RAW_SEL, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_SEL2, CAMCTL_RAWI_UFO_SEL, CAMCTL_R1A_RAWI_UFO_SEL, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_SEL, CAMCTL_SEP_SEL, CAMCTL_R1A_SEP_SEL, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_SEL, CAMCTL_BPC_R2_SEL, CAMCTL_R1A_BPC_R2_SEL, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_SEL2, CAMCTL_BPC_R3_SEL, CAMCTL_R1A_BPC_R3_SEL, U)\
    /* PIXEL MODE */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_FMT2_SEL, CAMCTL_PIX_BUS_SEPI, CAMCTL_R1A_PIX_BUS_SEPI, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_FMT_SEL, CAMCTL_PIX_BUS_SEPO, CAMCTL_R1A_PIX_BUS_SEPO, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_FMT_SEL, CAMCTL_PIX_BUS_MRG_R3O, CAMCTL_R1A_PIX_BUS_MRG_R3O, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_FMT_SEL, CAMCTL_PIX_BUS_MRG_R7O, CAMCTL_R1A_PIX_BUS_MRG_R7O, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_FMT_SEL, CAMCTL_PIX_BUS_MRG_R8O, CAMCTL_R1A_PIX_BUS_MRG_R8O, U)\
    /* SEP_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN, CAMCTL_SEP_R1_EN, CAMCTL_R1A_SEP_R1_EN, U)\
    /*CMD(a, b, c, d, e, f, SEP_R1A_SEP_CTL, SEP_EDGE, SEP_R1A_SEP_EDGE, U)*/\
    CMD(a, b, c, d, e, f, SEP_R1A_SEP_CROP, SEP_STR_X, SEP_R1A_STR_X, U)\
    CMD(a, b, c, d, e, f, SEP_R1A_SEP_CROP, SEP_END_X, SEP_R1A_END_X, U)\
    CMD(a, b, c, d, e, f, SEP_R1A_SEP_VSIZE, SEP_HT, SEP_R1A_SEP_HT, U)\
    /* IBPC_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN4, CAMCTL_IBPC_R1_EN, CAMCTL_R1A_IBPC_R1_EN, U)\
    /* IOBC_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN4, CAMCTL_IOBC_R1_EN, CAMCTL_R1A_IOBC_R1_EN, U)\
    /* IRM_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN4, CAMCTL_IRM_R1_EN, CAMCTL_R1A_IRM_R1_EN, U)\
    /* FBND_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN2, CAMCTL_FBND_R1_EN, CAMCTL_R1A_FBND_R1_EN, U)\
    /* OBC_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN, CAMCTL_OBC_R1_EN, CAMCTL_R1A_OBC_R1_EN, U)\
    /* BPC_R1 */\
	CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN, CAMCTL_BPC_R1_EN, CAMCTL_R1A_BPC_R1_EN, U)\
	CMD(a, b, c, d, e, f, BPC_R1A_BPC_BPC_TBLI1, BPC_XOFFSET, BPC_R1A_XOFFSET, U)\
	CMD(a, b, c, d, e, f, BPC_R1A_BPC_BPC_TBLI1, BPC_YOFFSET, BPC_R1A_YOFFSET, U)\
	CMD(a, b, c, d, e, f, BPC_R1A_BPC_BPC_TBLI2, BPC_XSIZE, BPC_R1A_XSIZE, U)\
	CMD(a, b, c, d, e, f, BPC_R1A_BPC_BPC_TBLI2, BPC_YSIZE, BPC_R1A_YSIZE, U)\
	CMD(a, b, c, d, e, f, BPC_R1A_BPC_BPC_CON, BPC_LE_INV_CTL, BPC_R1A_LE_INV_CTL, U)\
	CMD(a, b, c, d, e, f, BPC_R1A_BPC_PDC_POS, BPC_PDC_XCENTER, BPC_R1A_PDC_XCENTER, U)\
	CMD(a, b, c, d, e, f, BPC_R1A_BPC_PDC_POS, BPC_PDC_XCENTER, BPC_R1A_PDC_YCENTER, U)\
	CMD(a, b, c, d, e, f, BPC_R1A_BPC_PDC_CON, BPC_PDC_EN, BPC_R1A_PDC_EN, U)\
    /* BPCI_R1 */\
    CMD(a, b, c, d, e, f, BPCI_R1A_BPCI_XSIZE, BPCI_XSIZE, BPCI_R1A_XSIZE, U)\
    CMD(a, b, c, d, e, f, BPCI_R1A_BPCI_YSIZE, BPCI_YSIZE, BPCI_R1A_YSIZE, U)\
    CMD(a, b, c, d, e, f, BPCI_R1A_BPCI_STRIDE, BPCI_STRIDE, BPCI_R1A_STRIDE, U)\
    /* OBC_R2 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN2, CAMCTL_OBC_R2_EN, CAMCTL_R1A_OBC_R2_EN, U)\
    /* BPC_R2 */\
	CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN2, CAMCTL_BPC_R2_EN, CAMCTL_R1A_BPC_R2_EN, U)\
	CMD(a, b, c, d, e, f, BPC_R2A_BPC_BPC_TBLI1, BPC_XOFFSET, BPC_R2A_XOFFSET, U)\
	CMD(a, b, c, d, e, f, BPC_R2A_BPC_BPC_TBLI1, BPC_YOFFSET, BPC_R2A_YOFFSET, U)\
	CMD(a, b, c, d, e, f, BPC_R2A_BPC_BPC_TBLI2, BPC_XSIZE, BPC_R2A_XSIZE, U)\
	CMD(a, b, c, d, e, f, BPC_R2A_BPC_BPC_TBLI2, BPC_YSIZE, BPC_R2A_YSIZE, U)\
    CMD(a, b, c, d, e, f, BPC_R2A_BPC_BPC_CON, BPC_LE_INV_CTL, BPC_R2A_LE_INV_CTL, U)\
	CMD(a, b, c, d, e, f, BPC_R2A_BPC_PDC_POS, BPC_PDC_XCENTER, BPC_R2A_PDC_XCENTER, U)\
	CMD(a, b, c, d, e, f, BPC_R2A_BPC_PDC_POS, BPC_PDC_XCENTER, BPC_R2A_PDC_YCENTER, U)\
	CMD(a, b, c, d, e, f, BPC_R2A_BPC_PDC_CON, BPC_PDC_EN, BPC_R2A_PDC_EN, U)\
    /* BPCI_R2 */\
    CMD(a, b, c, d, e, f, BPCI_R2A_BPCI_XSIZE, BPCI_XSIZE, BPCI_R2A_XSIZE, U)\
    CMD(a, b, c, d, e, f, BPCI_R2A_BPCI_YSIZE, BPCI_YSIZE, BPCI_R2A_YSIZE, U)\
    CMD(a, b, c, d, e, f, BPCI_R2A_BPCI_STRIDE, BPCI_STRIDE, BPCI_R2A_STRIDE, U)\
    /* ZFUS_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN2, CAMCTL_ZFUS_R1_EN, CAMCTL_R1A_ZFUS_R1_EN, U)\
    /* FUS_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN, CAMCTL_FUS_R1_EN, CAMCTL_R1A_FUS_R1_EN, U)\
    CMD(a, b, c, d, e, f, FUS_R1A_FUS_CFG, FUS_INPUT_MODE, FUS_R1A_INPUT_MODE, U)\
    CMD(a, b, c, d, e, f, FUS_R1A_FUS_SIZE, FUS_HSIZE, FUS_R1A_HSIZE, U)\
    CMD(a, b, c, d, e, f, FUS_R1A_FUS_SIZE, FUS_VSIZE, FUS_R1A_VSIZE, U)\
    /* RNR_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN4, CAMCTL_RNR_R1_EN, CAMCTL_R1A_RNR_R1_EN, U)\
    /* DGN_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN, CAMCTL_DGN_R1_EN, CAMCTL_R1A_DGN_R1_EN, U)\
    /* LSC_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN, CAMCTL_LSC_R1_EN, CAMCTL_R1A_LSC_R1_EN, U)\
    CMD(a, b, c, d, e, f, LSC_R1A_LSC_CTL1, LSC_EXTEND_COEF_MODE, LSC_R1A_EXTEND_COEF_MODE, U)\
    CMD(a, b, c, d, e, f, LSC_R1A_LSC_CTL2, LSC_SDBLK_XNUM, LSC_R1A_SDBLK_XNUM, U)\
    CMD(a, b, c, d, e, f, LSC_R1A_LSC_CTL2, LSC_SDBLK_WIDTH, LSC_R1A_SDBLK_WIDTH, U)\
    CMD(a, b, c, d, e, f, LSC_R1A_LSC_LBLOCK, LSC_SDBLK_lWIDTH, LSC_R1A_SDBLK_lWIDTH, U)\
    CMD(a, b, c, d, e, f, LSC_R1A_LSC_TPIPE_OFST, LSC_TPIPE_OFST_X, LSC_R1A_TPIPE_OFST_X, U)\
    CMD(a, b, c, d, e, f, LSC_R1A_LSC_TPIPE_SIZE, LSC_TPIPE_SIZE_X, LSC_R1A_TPIPE_SIZE_X, U)\
    CMD(a, b, c, d, e, f, LSC_R1A_LSC_CTL3, LSC_SDBLK_HEIGHT, LSC_R1A_SDBLK_HEIGHT, U)\
    CMD(a, b, c, d, e, f, LSC_R1A_LSC_LBLOCK, LSC_SDBLK_lHEIGHT, LSC_R1A_SDBLK_lHEIGHT, U)\
    CMD(a, b, c, d, e, f, LSC_R1A_LSC_TPIPE_OFST, LSC_TPIPE_OFST_Y, LSC_R1A_TPIPE_OFST_Y, U)\
    CMD(a, b, c, d, e, f, LSC_R1A_LSC_TPIPE_SIZE, LSC_TPIPE_SIZE_Y, LSC_R1A_TPIPE_SIZE_Y, U)\
    /* LSCI_R1 */\
    CMD(a, b, c, d, e, f, LSCI_R1A_LSCI_OFST_ADDR, LSCI_OFST_ADDR, LSCI_R1A_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, LSCI_R1A_LSCI_XSIZE, LSCI_XSIZE, LSCI_R1A_XSIZE, U)\
    CMD(a, b, c, d, e, f, LSCI_R1A_LSCI_STRIDE, LSCI_STRIDE, LSCI_R1A_STRIDE, U)\
    /* WB_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN, CAMCTL_WB_R1_EN, CAMCTL_R1A_WB_R1_EN, U)\
    /* HLR_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN2, CAMCTL_HLR_R1_EN, CAMCTL_R1A_HLR_R1_EN, U)\
    /* LTM_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN, CAMCTL_LTM_R1_EN, CAMCTL_R1A_LTM_R1_EN, U)\
    CMD(a, b, c, d, e, f, LTM_R1A_LTM_BLK_NUM, LTM_BLK_X_NUM, LTM_R1A_BLK_X_NUM, U)\
    CMD(a, b, c, d, e, f, LTM_R1A_LTM_BLK_NUM, LTM_BLK_Y_NUM, LTM_R1A_BLK_Y_NUM, U)\
    CMD(a, b, c, d, e, f, LTM_R1A_LTM_BLK_SZ, LTM_BLK_WIDTH, LTM_R1A_BLK_WIDTH, U)\
    CMD(a, b, c, d, e, f, LTM_R1A_LTM_BLK_SZ, LTM_BLK_HEIGHT, LTM_R1A_BLK_HEIGHT, U)\
    CMD(a, b, c, d, e, f, LTM_R1A_LTM_TILE_NUM, LTM_TILE_BLK_X_NUM_START, LTM_R1A_TILE_BLK_X_NUM_START, U)\
    CMD(a, b, c, d, e, f, LTM_R1A_LTM_TILE_NUM, LTM_TILE_BLK_X_NUM_END, LTM_R1A_TILE_BLK_X_NUM_END, U)\
    CMD(a, b, c, d, e, f, LTM_R1A_LTM_TILE_NUM, LTM_TILE_BLK_Y_NUM_START, LTM_R1A_TILE_BLK_Y_NUM_START, U)\
    CMD(a, b, c, d, e, f, LTM_R1A_LTM_TILE_NUM, LTM_TILE_BLK_Y_NUM_END, LTM_R1A_TILE_BLK_Y_NUM_END, U)\
    CMD(a, b, c, d, e, f, LTM_R1A_LTM_TILE_CNTX, LTM_TILE_BLK_X_CNT_START, LTM_R1A_TILE_BLK_X_CNT_START, U)\
    CMD(a, b, c, d, e, f, LTM_R1A_LTM_TILE_CNTX, LTM_TILE_BLK_X_CNT_END, LTM_R1A_TILE_BLK_X_CNT_END, U)\
    CMD(a, b, c, d, e, f, LTM_R1A_LTM_TILE_CNTY, LTM_TILE_BLK_Y_CNT_START, LTM_R1A_TILE_BLK_Y_CNT_START, U)\
    CMD(a, b, c, d, e, f, LTM_R1A_LTM_TILE_CNTY, LTM_TILE_BLK_Y_CNT_END, LTM_R1A_TILE_BLK_Y_CNT_END, U)\
    /* RRZ_R1 - RRZ branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN2, CAMCTL_RRZ_R1_EN, CAMCTL_R1A_RRZ_R1_EN, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN4, CAMCTL_RLB_R1_EN, CAMCTL_R1A_RLB_R1_EN, U)\
    CMD(a, b, c, d, e, f, RRZ_R1A_RRZ_VERT_STEP, RRZ_VERT_STEP, RRZ_tmp1_VERT_STEP, U)\
    CMD(a, b, c, d, e, f, RRZ_R1A_RRZ_VERT_INT_OFST, RRZ_VERT_INT_OFST, RRZ_tmp1_VERT_INT_OFST, U)\
    CMD(a, b, c, d, e, f, RRZ_R1A_RRZ_VERT_SUB_OFST, RRZ_VERT_SUB_OFST, RRZ_tmp1_VERT_SUB_OFST, U)\
    CMD(a, b, c, d, e, f, RRZ_R1A_RRZ_OUT_IMG, RRZ_OUT_HT, RRZ_tmp1_OUT_HT, U)\
    CMD(a, b, c, d, e, f, RRZ_R1A_RRZ_OUT_IMG, RRZ_OUT_WD, RRZ_tmp1_OUT_WD, U)\
    CMD(a, b, c, d, e, f, RRZ_R1A_RRZ_HORI_STEP, RRZ_HORI_STEP, RRZ_tmp1_HORI_STEP, U)\
    CMD(a, b, c, d, e, f, RRZ_R1A_RRZ_HORI_INT_OFST, RRZ_HORI_INT_OFST, RRZ_tmp1_HORI_INT_OFST, U)\
    CMD(a, b, c, d, e, f, RRZ_R1A_RRZ_HORI_SUB_OFST, RRZ_HORI_SUB_OFST, RRZ_tmp1_HORI_SUB_OFST, U)\
    /*CMD(a, b, c, d, e, f, RRZ_R1A_RRZ_RLB_AOFST, RRZ_RLB_AOFST, RRZ_tmp1_RLB_AOFST, U)*/\
    /* HDS_SEL - RRZ branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_SEL, CAMCTL_HDS_SEL, CAMCTL_R1A_HDS_SEL, U)\
    /* GSE - RRZ branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN2, CAMCTL_GSE_R1_EN, CAMCTL_R1A_GSE_R1_EN, U)\
    /* MRG_R8 - RRZ branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN2, CAMCTL_MRG_R8_EN, CAMCTL_R1A_MRG_R8_EN, U)\
    CMD(a, b, c, d, e, f, MRG_R8A_MRG_CTL, MRG_SIG_MODE1, MRG_R8A_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, MRG_R8A_MRG_CTL, MRG_SIG_MODE2, MRG_R8A_SIG_MODE2, U)\
    /*CMD(a, b, c, d, e, f, MRG_R8A_MRG_CTL, MRG_EDGE, MRG_R8A_EDGE, U)*/\
    CMD(a, b, c, d, e, f, MRG_R8A_MRG_CROP, MRG_STR_X, MRG_R8A_STR_X, U)\
    CMD(a, b, c, d, e, f, MRG_R8A_MRG_CROP, MRG_END_X, MRG_R8A_END_X, U)\
    /*CMD(a, b, c, d, e, f, MRG_R8A_MRG_VSIZE, MRG_HT, MRG_R8A_HT, U)*/\
    /* MRG_R7 - RRZ branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN, CAMCTL_MRG_R7_EN, CAMCTL_R1A_MRG_R7_EN, U)\
    CMD(a, b, c, d, e, f, MRG_R7A_MRG_CTL, MRG_SIG_MODE1, MRG_R7A_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, MRG_R7A_MRG_CTL, MRG_SIG_MODE2, MRG_R7A_SIG_MODE2, U)\
    /*CMD(a, b, c, d, e, f, MRG_R7A_MRG_CTL, MRG_EDGE, MRG_R7A_EDGE, U)*/\
    CMD(a, b, c, d, e, f, MRG_R7A_MRG_CROP, MRG_STR_X, MRG_R7A_STR_X, U)\
    CMD(a, b, c, d, e, f, MRG_R7A_MRG_CROP, MRG_END_X, MRG_R7A_END_X, U)\
    /*CMD(a, b, c, d, e, f, MRG_R7A_MRG_VSIZE, MRG_HT, MRG_R7A_HT, U)*/\
    /* MRG_R11 - RRZ branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN3, CAMCTL_MRG_R11_EN, CAMCTL_R1A_MRG_R11_EN, U)\
    CMD(a, b, c, d, e, f, MRG_R11A_MRG_CTL, MRG_SIG_MODE1, MRG_R11A_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, MRG_R11A_MRG_CTL, MRG_SIG_MODE2, MRG_R11A_SIG_MODE2, U)\
    /*CMD(a, b, c, d, e, f, MRG_R11A_MRG_CTL, MRG_EDGE, MRG_R11A_EDGE, U)*/\
    CMD(a, b, c, d, e, f, MRG_R11A_MRG_CROP, MRG_STR_X, MRG_R11A_STR_X, U)\
    CMD(a, b, c, d, e, f, MRG_R11A_MRG_CROP, MRG_END_X, MRG_R11A_END_X, U)\
    /*CMD(a, b, c, d, e, f, MRG_R11A_MRG_VSIZE, MRG_HT, MRG_R11A_HT, U)*/\
    /* UFEG_SEL - RRZ branch*/\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_SEL, CAMCTL_UFEG_SEL, CAMCTL_R1A_UFEG_SEL, U)\
    /* PAKG_R1 - RRZ branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN, CAMCTL_PAKG_R1_EN, CAMCTL_R1A_PAKG_R1_EN, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_FMT2_SEL, CAMCTL_PAKG_R1_FG_OUT, CAMCTL_R1A_PAKG_R1_FG_OUT, U)\
    /* UFG_R1 - RRZ branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN2, CAMCTL_UFG_R1_EN, CAMCTL_R1A_UFG_R1_EN, U)\
    CMD(a, b, c, d, e, f, UFG_R1A_UFG_CON, UFG_FORCE_PCM, UFG_R1A_FORCE_PCM, U)\
    /* RRZO_R1 - RRZ branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_FMT_SEL, CAMCTL_RRZO_R1_FMT, CAMCTL_R1A_RRZO_R1_FMT, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_FMT_SEL, CAMCTL_RRZO_FG_MODE, CAMCTL_R1A_RRZO_FG_MODE, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_DMA_EN, CAMCTL_RRZO_R1_EN, CAMCTL_R1A_RRZO_R1_EN, U)\
    CMD(a, b, c, d, e, f, RRZO_R1A_RRZO_BASE_ADDR, RRZO_BASE_ADDR, RRZO_R1A_BASE_ADDR, U)\
    CMD(a, b, c, d, e, f, RRZO_R1A_RRZO_OFST_ADDR, RRZO_OFST_ADDR, RRZO_R1A_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, RRZO_R1A_RRZO_XSIZE, RRZO_XSIZE, RRZO_R1A_XSIZE, U)\
    CMD(a, b, c, d, e, f, RRZO_R1A_RRZO_YSIZE, RRZO_YSIZE, RRZO_R1A_YSIZE, U)\
    CMD(a, b, c, d, e, f, RRZO_R1A_RRZO_STRIDE, RRZO_STRIDE, RRZO_R1A_STRIDE, U)\
    /* UFGO_R1 - RRZ branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_DMA_EN, CAMCTL_UFGO_R1_EN, CAMCTL_R1A_UFGO_R1_EN, U)\
    CMD(a, b, c, d, e, f, UFGO_R1A_UFGO_BASE_ADDR, UFGO_BASE_ADDR, UFGO_R1A_BASE_ADDR, U)\
    CMD(a, b, c, d, e, f, UFGO_R1A_UFGO_OFST_ADDR, UFGO_OFST_ADDR, UFGO_R1A_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, UFGO_R1A_UFGO_XSIZE, UFGO_XSIZE, UFGO_R1A_XSIZE, U)\
    CMD(a, b, c, d, e, f, UFGO_R1A_UFGO_YSIZE, UFGO_YSIZE, UFGO_R1A_YSIZE, U)\
    CMD(a, b, c, d, e, f, UFGO_R1A_UFGO_STRIDE, UFGO_STRIDE, UFGO_R1A_STRIDE, U)\
    /* QBN_R5 - LTM branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN2, CAMCTL_QBN_R5_EN, CAMCTL_R1A_QBN_R5_EN, U)\
    CMD(a, b, c, d, e, f, QBIN_R5A_QBIN_CTL, QBIN_ACC, QBIN_R5A_ACC, U)\
    CMD(a, b, c, d, e, f, QBIN_R5A_QBIN_CTL, QBIN_ACC_MODE, QBIN_R5A_ACC_MODE, U)\
    /* MRG_R5 - LTM branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN, CAMCTL_MRG_R5_EN, CAMCTL_R1A_MRG_R5_EN, U)\
    CMD(a, b, c, d, e, f, MRG_R5A_MRG_CTL, MRG_SIG_MODE1, MRG_R5A_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, MRG_R5A_MRG_CTL, MRG_SIG_MODE2, MRG_R5A_SIG_MODE2, U)\
    /*CMD(a, b, c, d, e, f, MRG_R5A_MRG_CTL, MRG_EDGE, MRG_R5A_EDGE, U)*/\
    CMD(a, b, c, d, e, f, MRG_R5A_MRG_CROP, MRG_STR_X, MRG_R5A_STR_X, U)\
    CMD(a, b, c, d, e, f, MRG_R5A_MRG_CROP, MRG_END_X, MRG_R5A_END_X, U)\
    /*CMD(a, b, c, d, e, f, MRG_R5A_MRG_VSIZE, MRG_HT, MRG_R5A_HT, U)*/\
    /* LCS_SEL - LTM branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_SEL, CAMCTL_LCES_SEL, CAMCTL_R1A_LCES_SEL, U)\
    /* LCESO_R1 - LTM branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_DMA_EN, CAMCTL_LCESO_R1_EN, CAMCTL_R1A_LCESO_R1_EN, U)\
    /* DM_R1_SEL - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_SEL2, CAMCTL_DM_R1_SEL, CAMCTL_R1A_DM_R1_SEL, U)\
    /* MRG_R13 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN3, CAMCTL_MRG_R13_EN, CAMCTL_R1A_MRG_R13_EN, U)\
    CMD(a, b, c, d, e, f, MRG_R13A_MRG_CTL, MRG_SIG_MODE1, MRG_R13A_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, MRG_R13A_MRG_CTL, MRG_SIG_MODE2, MRG_R13A_SIG_MODE2, U)\
    /*CMD(a, b, c, d, e, f, MRG_R13A_MRG_CTL, MRG_EDGE, MRG_R13A_EDGE, U)*/\
    CMD(a, b, c, d, e, f, MRG_R13A_MRG_CROP, MRG_STR_X, MRG_R13A_STR_X, U)\
    CMD(a, b, c, d, e, f, MRG_R13A_MRG_CROP, MRG_END_X, MRG_R13A_END_X, U)\
    /*CMD(a, b, c, d, e, f, MRG_R13A_MRG_VSIZE, MRG_HT, MRG_R13A_HT, U)*/\
    /* MRG_R14 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN3, CAMCTL_MRG_R14_EN, CAMCTL_R1A_MRG_R14_EN, U)\
    CMD(a, b, c, d, e, f, MRG_R14A_MRG_CTL, MRG_SIG_MODE1, MRG_R14A_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, MRG_R14A_MRG_CTL, MRG_SIG_MODE2, MRG_R14A_SIG_MODE2, U)\
    /*CMD(a, b, c, d, e, f, MRG_R14A_MRG_CTL, MRG_EDGE, MRG_R14A_EDGE, U)*/\
    CMD(a, b, c, d, e, f, MRG_R14A_MRG_CROP, MRG_STR_X, MRG_R14A_STR_X, U)\
    CMD(a, b, c, d, e, f, MRG_R14A_MRG_CROP, MRG_END_X, MRG_R14A_END_X, U)\
    /*CMD(a, b, c, d, e, f, MRG_R14A_MRG_VSIZE, MRG_HT, MRG_R14A_HT, U)*/\
    /* DM_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN2, CAMCTL_DM_R1_EN, CAMCTL_R1A_DM_R1_EN, U)\
    CMD(a, b, c, d, e, f, DM_R1A_DM_SL_CTL, DM_SL_EN, DM_R1A_SL_EN, U)\
    /* SLK_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN, CAMCTL_SLK_R1_EN, CAMCTL_R1A_SLK_R1_EN, U)\
    CMD(a, b, c, d, e, f, SLK_R1A_SLK_RZ, SLK_HRZ_COMP, SLK_R1A_HRZ_COMP, U)\
    CMD(a, b, c, d, e, f, SLK_R1A_SLK_RZ, SLK_VRZ_COMP, SLK_R1A_VRZ_COMP, U)\
    CMD(a, b, c, d, e, f, SLK_R1A_SLK_XOFF, SLK_X_OFST, SLK_R1A_X_OFST, U)\
    CMD(a, b, c, d, e, f, SLK_R1A_SLK_SIZE, SLK_TPIPE_WD, SLK_R1A_TPIPE_WD, U)\
    CMD(a, b, c, d, e, f, SLK_R1A_SLK_SIZE, SLK_TPIPE_HT, SLK_R1A_TPIPE_HT, U)\
    /* FLC_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN3, CAMCTL_FLC_R1_EN, CAMCTL_R1A_FLC_R1_EN, U)\
    /* CCM_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN2, CAMCTL_CCM_R1_EN, CAMCTL_R1A_CCM_R1_EN, U)\
    /* GGM_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN2, CAMCTL_GGM_R1_EN, CAMCTL_R1A_GGM_R1_EN, U)\
    /* G2C_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN2, CAMCTL_G2C_R1_EN, CAMCTL_R1A_G2C_R1_EN, U)\
    /* C42_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN2, CAMCTL_C42_R1_EN, CAMCTL_R1A_C42_R1_EN, U)\
    /* YNRS_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN3, CAMCTL_YNRS_R1_EN, CAMCTL_R1A_YNRS_R1_EN, U)\
    CMD(a, b, c, d, e, f, YNRS_R1A_YNRS_CON1, YNRS_SL2_LINK, YNRS_R1A_SL2_LINK, U)\
    /* SLK_R2 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN2, CAMCTL_SLK_R2_EN, CAMCTL_R1A_SLK_R2_EN, U)\
    CMD(a, b, c, d, e, f, SLK_R2A_SLK_RZ, SLK_HRZ_COMP, SLK_R2A_HRZ_COMP, U)\
    CMD(a, b, c, d, e, f, SLK_R2A_SLK_RZ, SLK_VRZ_COMP, SLK_R2A_VRZ_COMP, U)\
    CMD(a, b, c, d, e, f, SLK_R2A_SLK_XOFF, SLK_X_OFST, SLK_R2A_X_OFST, U)\
    CMD(a, b, c, d, e, f, SLK_R2A_SLK_SIZE, SLK_TPIPE_WD, SLK_R2A_TPIPE_WD, U)\
    CMD(a, b, c, d, e, f, SLK_R2A_SLK_SIZE, SLK_TPIPE_HT, SLK_R2A_TPIPE_HT, U)\
    /* CRP_R4 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN3, CAMCTL_CRP_R4_EN, CAMCTL_R1A_CRP_R4_EN, U)\
    CMD(a, b, c, d, e, f, CRP_R4A_CRP_X_POS, CRP_XSTART, CRP_R4A_XSTART, U)\
    CMD(a, b, c, d, e, f, CRP_R4A_CRP_X_POS, CRP_XEND, CRP_R4A_XEND, U)\
    CMD(a, b, c, d, e, f, CRP_R4A_CRP_Y_POS, CRP_YSTART, CRP_R4A_YSTART, U)\
    CMD(a, b, c, d, e, f, CRP_R4A_CRP_Y_POS, CRP_YEND, CRP_R4A_YEND, U)\
    /* CRSP_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN3, CAMCTL_CRSP_R1_EN, CAMCTL_R1A_CRSP_R1_EN, U)\
    CMD(a, b, c, d, e, f, CRSP_R1A_CRSP_OUT_IMG, CRSP_WD, CRSP_R1A_tgWidth, U)\
    CMD(a, b, c, d, e, f, CRSP_R1A_CRSP_OUT_IMG, CRSP_HT, CRSP_R1A_tgHeight, U)\
    CMD(a, b, c, d, e, f, CRSP_R1A_CRSP_CROP_X, CRSP_CROP_XSTART, CRSP_R1A_CROP_XSTART, U)\
    CMD(a, b, c, d, e, f, CRSP_R1A_CRSP_CROP_X, CRSP_CROP_XEND, CRSP_R1A_CROP_XEND, U)\
    CMD(a, b, c, d, e, f, CRSP_R1A_CRSP_CROP_Y, CRSP_CROP_YSTART, CRSP_R1A_CROP_YSTART, U)\
    CMD(a, b, c, d, e, f, CRSP_R1A_CRSP_CROP_Y, CRSP_CROP_YEND, CRSP_R1A_CROP_YEND, U)\
    /* PAK_R3 YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN3, CAMCTL_PAK_R3_EN, CAMCTL_R1A_PAK_R3_EN, U)\
    CMD(a, b, c, d, e, f, PAK_R3A_PAK_CONT, PAK_YUV_DNG, PAK_R3A_YUV_DNG, U)\
    CMD(a, b, c, d, e, f, PAK_R3A_PAK_CONT, PAK_YUV_BIT, PAK_R3A_YUV_BIT, U)\
    /* PAK_R4 YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN3, CAMCTL_PAK_R4_EN, CAMCTL_R1A_PAK_R4_EN, U)\
    CMD(a, b, c, d, e, f, PAK_R4A_PAK_CONT, PAK_YUV_DNG, PAK_R4A_YUV_DNG, U)\
    CMD(a, b, c, d, e, f, PAK_R4A_PAK_CONT, PAK_YUV_BIT, PAK_R4A_YUV_BIT, U)\
    /* PAK_R5 YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN3, CAMCTL_PAK_R5_EN, CAMCTL_R1A_PAK_R5_EN, U)\
	CMD(a, b, c, d, e, f, PAK_R5A_PAK_CONT, PAK_YUV_DNG, PAK_R5A_YUV_DNG, U)\
	CMD(a, b, c, d, e, f, PAK_R5A_PAK_CONT, PAK_YUV_BIT, PAK_R5A_YUV_BIT, U)\
    /* PLNW_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN3, CAMCTL_PLNW_R1_EN, CAMCTL_R1A_PLNW_R1_EN, U)\
    /* YUVO_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_FMT2_SEL, CAMCTL_YUVO_R1_FMT, CAMCTL_R1A_YUVO_R1_FMT, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_DMA2_EN, CAMCTL_YUVO_R1_EN, CAMCTL_R1A_YUVO_R1_EN, U)\
    CMD(a, b, c, d, e, f, YUVO_R1A_YUVO_BASE_ADDR, YUVO_BASE_ADDR, YUVO_R1A_BASE_ADDR, U)\
    CMD(a, b, c, d, e, f, YUVO_R1A_YUVO_OFST_ADDR, YUVO_OFST_ADDR, YUVO_R1A_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, YUVO_R1A_YUVO_XSIZE, YUVO_XSIZE, YUVO_R1A_XSIZE, U)\
    CMD(a, b, c, d, e, f, YUVO_R1A_YUVO_YSIZE, YUVO_YSIZE, YUVO_R1A_YSIZE, U)\
    CMD(a, b, c, d, e, f, YUVO_R1A_YUVO_STRIDE, YUVO_STRIDE, YUVO_R1A_STRIDE, U)\
    /* YUVBO_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_DMA_EN, CAMCTL_YUVBO_R1_EN, CAMCTL_R1A_YUVBO_R1_EN, U)\
    CMD(a, b, c, d, e, f, YUVBO_R1A_YUVBO_BASE_ADDR, YUVBO_BASE_ADDR, YUVBO_R1A_BASE_ADDR, U)\
    CMD(a, b, c, d, e, f, YUVBO_R1A_YUVBO_OFST_ADDR, YUVBO_OFST_ADDR, YUVBO_R1A_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, YUVBO_R1A_YUVBO_XSIZE, YUVBO_XSIZE, YUVBO_R1A_XSIZE, U)\
    CMD(a, b, c, d, e, f, YUVBO_R1A_YUVBO_YSIZE, YUVBO_YSIZE, YUVBO_R1A_YSIZE, U)\
    CMD(a, b, c, d, e, f, YUVBO_R1A_YUVBO_STRIDE, YUVBO_STRIDE, YUVBO_R1A_STRIDE, U)\
    /* YUVCO_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_DMA_EN, CAMCTL_YUVCO_R1_EN, CAMCTL_R1A_YUVCO_R1_EN, U)\
    CMD(a, b, c, d, e, f, YUVCO_R1A_YUVCO_BASE_ADDR, YUVCO_BASE_ADDR, YUVCO_R1A_BASE_ADDR, U)\
    CMD(a, b, c, d, e, f, YUVCO_R1A_YUVCO_OFST_ADDR, YUVCO_OFST_ADDR, YUVCO_R1A_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, YUVCO_R1A_YUVCO_XSIZE, YUVCO_XSIZE, YUVCO_R1A_XSIZE, U)\
    CMD(a, b, c, d, e, f, YUVCO_R1A_YUVCO_YSIZE, YUVCO_YSIZE, YUVCO_R1A_YSIZE, U)\
    CMD(a, b, c, d, e, f, YUVCO_R1A_YUVCO_STRIDE, YUVCO_STRIDE, YUVCO_R1A_STRIDE, U)\
    /* BS_R3 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN, CAMCTL_BS_R3_EN, CAMCTL_R1A_BS_R3_EN, U)\
    /* RSS_R2 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN3, CAMCTL_RSS_R2_EN, CAMCTL_R1A_RSS_R2_EN, U)\
    CMD(a, b, c, d, e, f, RSS_R2A_RSS_IN_IMG, RSS_IN_WD, RDSS_R2A_Input_Image_W, U)\
    CMD(a, b, c, d, e, f, RSS_R2A_RSS_IN_IMG, RSS_IN_HT, RDSS_R2A_Input_Image_H, U)\
    CMD(a, b, c, d, e, f, RSS_R2A_RSS_OUT_IMG, RSS_OUT_WD, RDSS_R2A_Output_Image_W, U)\
    CMD(a, b, c, d, e, f, RSS_R2A_RSS_OUT_IMG, RSS_OUT_HT, RDSS_R2A_Output_Image_H, U)\
    CMD(a, b, c, d, e, f, RSS_R2A_RSS_CONTROL, RSS_HORI_EN, RDSS_R2A_Horizontal_EN, U)\
    CMD(a, b, c, d, e, f, RSS_R2A_RSS_CONTROL, RSS_VERT_EN, RDSS_R2A_Vertical_EN, U)\
    CMD(a, b, c, d, e, f, RSS_R2A_RSS_HORI_STEP, RSS_HORI_STEP, RDSS_R2A_Horizontal_Coeff_Step, U)\
    CMD(a, b, c, d, e, f, RSS_R2A_RSS_VERT_STEP, RSS_VERT_STEP, RDSS_R2A_Vertical_Coeff_Step, U)\
    CMD(a, b, c, d, e, f, RSS_R2A_RSS_LUMA_HORI_INT_OFST, RSS_LUMA_HORI_INT_OFST, RDSS_R2A_Luma_Horizontal_Integer_Offset, U)\
    CMD(a, b, c, d, e, f, RSS_R2A_RSS_LUMA_HORI_SUB_OFST, RSS_LUMA_HORI_SUB_OFST, RDSS_R2A_Luma_Horizontal_Subpixel_Offset, U)\
    /* RSSO_R2 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_DMA2_EN, CAMCTL_RSSO_R2_EN, CAMCTL_R1A_RSSO_R2_EN, U)\
    CMD(a, b, c, d, e, f, RSSO_R2A_RSSO_BASE_ADDR, RSSO_BASE_ADDR, RSSO_R2A_BASE_ADDR, U)\
    CMD(a, b, c, d, e, f, RSSO_R2A_RSSO_OFST_ADDR, RSSO_OFST_ADDR, RSSO_R2A_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, RSSO_R2A_RSSO_XSIZE, RSSO_XSIZE, RSSO_R2A_XSIZE, U)\
    CMD(a, b, c, d, e, f, RSSO_R2A_RSSO_YSIZE, RSSO_YSIZE, RSSO_R2A_YSIZE, U)\
    CMD(a, b, c, d, e, f, RSSO_R2A_RSSO_STRIDE, RSSO_STRIDE, RSSO_R2A_STRIDE, U)\
    /* GGM_R2 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN3, CAMCTL_GGM_R2_EN, CAMCTL_R1A_GGM_R2_EN, U)\
    /* G2C_R2 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN3, CAMCTL_G2C_R2_EN, CAMCTL_R1A_G2C_R2_EN, U)\
    /* C42_R2 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN3, CAMCTL_C42_R2_EN, CAMCTL_R1A_C42_R2_EN, U)\
    /* BS_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN, CAMCTL_BS_R1_EN, CAMCTL_R1A_BS_R1_EN, U)\
    /* CRZ_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN3, CAMCTL_CRZ_R1_EN, CAMCTL_R1A_CRZ_R1_EN, U)\
    CMD(a, b, c, d, e, f, CRZ_R1A_CRZ_IN_IMG, CRZ_IN_WD, CRZ_R1A_IN_WD, U)\
    CMD(a, b, c, d, e, f, CRZ_R1A_CRZ_IN_IMG, CRZ_IN_HT, CRZ_R1A_IN_HT, U)\
    CMD(a, b, c, d, e, f, CRZ_R1A_CRZ_OUT_IMG, CRZ_OUT_WD, CRZ_R1A_OUT_WD, U)\
    CMD(a, b, c, d, e, f, CRZ_R1A_CRZ_OUT_IMG, CRZ_OUT_HT, CRZ_R1A_OUT_HT, U)\
    CMD(a, b, c, d, e, f, CRZ_R1A_CRZ_CONTROL, CRZ_HORI_EN, CRZ_R1A_HORI_EN, U)\
    CMD(a, b, c, d, e, f, CRZ_R1A_CRZ_CONTROL, CRZ_VERT_EN, CRZ_R1A_VERT_EN, U)\
    CMD(a, b, c, d, e, f, CRZ_R1A_CRZ_CONTROL, CRZ_HORI_ALGO, CRZ_R1A_HORI_ALGO, U)\
    CMD(a, b, c, d, e, f, CRZ_R1A_CRZ_CONTROL, CRZ_VERT_ALGO, CRZ_R1A_VERT_ALGO, U)\
    CMD(a, b, c, d, e, f, CRZ_R1A_CRZ_HORI_STEP, CRZ_HORI_STEP, CRZ_R1A_HORI_STEP, U)\
    CMD(a, b, c, d, e, f, CRZ_R1A_CRZ_VERT_STEP, CRZ_VERT_STEP, CRZ_R1A_VERT_STEP, U)\
    CMD(a, b, c, d, e, f, CRZ_R1A_CRZ_LUMA_HORI_INT_OFST, CRZ_LUMA_HORI_INT_OFST, CRZ_R1A_LUMA_HORI_INT_OFST, U)\
    CMD(a, b, c, d, e, f, CRZ_R1A_CRZ_LUMA_HORI_SUB_OFST, CRZ_LUMA_HORI_SUB_OFST, CRZ_R1A_LUMA_HORI_SUB_OFST, U)\
    CMD(a, b, c, d, e, f, CRZ_R1A_CRZ_CHRO_HORI_INT_OFST, CRZ_CHRO_HORI_INT_OFST, CRZ_R1A_CHRO_HORI_INT_OFST, U)\
    CMD(a, b, c, d, e, f, CRZ_R1A_CRZ_CHRO_HORI_SUB_OFST, CRZ_CHRO_HORI_SUB_OFST, CRZ_R1A_CHRO_HORI_SUB_OFST, U)\
    /* PLNW_R2 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN3, CAMCTL_PLNW_R2_EN, CAMCTL_R1A_PLNW_R2_EN, U)\
    /* CRZO_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_FMT_SEL, CAMCTL_CRZO_R1_FMT, CAMCTL_R1A_CRZO_R1_FMT, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_DMA_EN, CAMCTL_CRZO_R1_EN, CAMCTL_R1A_CRZO_R1_EN, U)\
    CMD(a, b, c, d, e, f, CRZO_R1A_CRZO_BASE_ADDR, CRZO_BASE_ADDR, CRZO_R1A_BASE_ADDR, U)\
    CMD(a, b, c, d, e, f, CRZO_R1A_CRZO_OFST_ADDR, CRZO_OFST_ADDR, CRZO_R1A_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, CRZO_R1A_CRZO_XSIZE, CRZO_XSIZE, CRZO_R1A_XSIZE, U)\
    CMD(a, b, c, d, e, f, CRZO_R1A_CRZO_YSIZE, CRZO_YSIZE, CRZO_R1A_YSIZE, U)\
    CMD(a, b, c, d, e, f, CRZO_R1A_CRZO_STRIDE, CRZO_STRIDE, CRZO_R1A_STRIDE, U)\
    /* CRZBO_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_DMA_EN, CAMCTL_CRZBO_R1_EN, CAMCTL_R1A_CRZBO_R1_EN, U)\
    CMD(a, b, c, d, e, f, CRZBO_R1A_CRZBO_BASE_ADDR, CRZBO_BASE_ADDR, CRZBO_R1A_BASE_ADDR, U)\
    CMD(a, b, c, d, e, f, CRZBO_R1A_CRZBO_OFST_ADDR, CRZBO_OFST_ADDR, CRZBO_R1A_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, CRZBO_R1A_CRZBO_XSIZE, CRZBO_XSIZE, CRZBO_R1A_XSIZE, U)\
    CMD(a, b, c, d, e, f, CRZBO_R1A_CRZBO_YSIZE, CRZBO_YSIZE, CRZBO_R1A_YSIZE, U)\
    CMD(a, b, c, d, e, f, CRZBO_R1A_CRZBO_STRIDE, CRZBO_STRIDE, CRZBO_R1A_STRIDE, U)\
    /* BS_R2 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN, CAMCTL_BS_R2_EN, CAMCTL_R1A_BS_R2_EN, U)\
    /* CRZ_R2 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN3, CAMCTL_CRZ_R2_EN, CAMCTL_R1A_CRZ_R2_EN, U)\
    CMD(a, b, c, d, e, f, CRZ_R2A_CRZ_IN_IMG, CRZ_IN_WD, CRZ_R2A_IN_WD, U)\
    CMD(a, b, c, d, e, f, CRZ_R2A_CRZ_IN_IMG, CRZ_IN_HT, CRZ_R2A_IN_HT, U)\
    CMD(a, b, c, d, e, f, CRZ_R2A_CRZ_OUT_IMG, CRZ_OUT_WD, CRZ_R2A_OUT_WD, U)\
    CMD(a, b, c, d, e, f, CRZ_R2A_CRZ_OUT_IMG, CRZ_OUT_HT, CRZ_R2A_OUT_HT, U)\
    CMD(a, b, c, d, e, f, CRZ_R2A_CRZ_CONTROL, CRZ_HORI_EN, CRZ_R2A_HORI_EN, U)\
    CMD(a, b, c, d, e, f, CRZ_R2A_CRZ_CONTROL, CRZ_VERT_EN, CRZ_R2A_VERT_EN, U)\
    CMD(a, b, c, d, e, f, CRZ_R2A_CRZ_CONTROL, CRZ_HORI_ALGO, CRZ_R2A_HORI_ALGO, U)\
    CMD(a, b, c, d, e, f, CRZ_R2A_CRZ_CONTROL, CRZ_VERT_ALGO, CRZ_R2A_VERT_ALGO, U)\
    CMD(a, b, c, d, e, f, CRZ_R2A_CRZ_HORI_STEP, CRZ_HORI_STEP, CRZ_R2A_HORI_STEP, U)\
    CMD(a, b, c, d, e, f, CRZ_R2A_CRZ_VERT_STEP, CRZ_VERT_STEP, CRZ_R2A_VERT_STEP, U)\
    CMD(a, b, c, d, e, f, CRZ_R2A_CRZ_LUMA_HORI_INT_OFST, CRZ_LUMA_HORI_INT_OFST, CRZ_R2A_LUMA_HORI_INT_OFST, U)\
    CMD(a, b, c, d, e, f, CRZ_R2A_CRZ_LUMA_HORI_SUB_OFST, CRZ_LUMA_HORI_SUB_OFST, CRZ_R2A_LUMA_HORI_SUB_OFST, U)\
    CMD(a, b, c, d, e, f, CRZ_R2A_CRZ_CHRO_HORI_INT_OFST, CRZ_CHRO_HORI_INT_OFST, CRZ_R2A_CHRO_HORI_INT_OFST, U)\
    CMD(a, b, c, d, e, f, CRZ_R2A_CRZ_CHRO_HORI_SUB_OFST, CRZ_CHRO_HORI_SUB_OFST, CRZ_R2A_CHRO_HORI_SUB_OFST, U)\
    /* CRSP_R2 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN3, CAMCTL_CRSP_R2_EN, CAMCTL_R1A_CRSP_R2_EN, U)\
    CMD(a, b, c, d, e, f, CRSP_R2A_CRSP_OUT_IMG, CRSP_WD, CRSP_R2A_tgWidth, U)\
    CMD(a, b, c, d, e, f, CRSP_R2A_CRSP_OUT_IMG, CRSP_HT, CRSP_R2A_tgHeight, U)\
    CMD(a, b, c, d, e, f, CRSP_R2A_CRSP_CROP_X, CRSP_CROP_XSTART, CRSP_R2A_CROP_XSTART, U)\
    CMD(a, b, c, d, e, f, CRSP_R2A_CRSP_CROP_X, CRSP_CROP_XEND, CRSP_R2A_CROP_XEND, U)\
    CMD(a, b, c, d, e, f, CRSP_R2A_CRSP_CROP_Y, CRSP_CROP_YSTART, CRSP_R2A_CROP_YSTART, U)\
    CMD(a, b, c, d, e, f, CRSP_R2A_CRSP_CROP_Y, CRSP_CROP_YEND, CRSP_R2A_CROP_YEND, U)\
    /* PLNW_R3 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN3, CAMCTL_PLNW_R3_EN, CAMCTL_R1A_PLNW_R3_EN, U)\
    /* CRZO_R2 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_FMT_SEL, CAMCTL_CRZO_R2_FMT, CAMCTL_R1A_CRZO_R2_FMT, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_DMA2_EN, CAMCTL_CRZO_R2_EN, CAMCTL_R1A_CRZO_R2_EN, U)\
    CMD(a, b, c, d, e, f, CRZO_R2A_CRZO_BASE_ADDR, CRZO_BASE_ADDR, CRZO_R2A_BASE_ADDR, U)\
    CMD(a, b, c, d, e, f, CRZO_R2A_CRZO_OFST_ADDR, CRZO_OFST_ADDR, CRZO_R2A_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, CRZO_R2A_CRZO_XSIZE, CRZO_XSIZE, CRZO_R2A_XSIZE, U)\
    CMD(a, b, c, d, e, f, CRZO_R2A_CRZO_YSIZE, CRZO_YSIZE, CRZO_R2A_YSIZE, U)\
    CMD(a, b, c, d, e, f, CRZO_R2A_CRZO_STRIDE, CRZO_STRIDE, CRZO_R2A_STRIDE, U)\
    /* CRZBO_R2 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_DMA2_EN, CAMCTL_CRZBO_R2_EN, CAMCTL_R1A_CRZBO_R2_EN, U)\
    CMD(a, b, c, d, e, f, CRZBO_R2A_CRZBO_BASE_ADDR, CRZBO_BASE_ADDR, CRZBO_R2A_BASE_ADDR, U)\
    CMD(a, b, c, d, e, f, CRZBO_R2A_CRZBO_OFST_ADDR, CRZBO_OFST_ADDR, CRZBO_R2A_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, CRZBO_R2A_CRZBO_XSIZE, CRZBO_XSIZE, CRZBO_R2A_XSIZE, U)\
    CMD(a, b, c, d, e, f, CRZBO_R2A_CRZBO_YSIZE, CRZBO_YSIZE, CRZBO_R2A_YSIZE, U)\
    CMD(a, b, c, d, e, f, CRZBO_R2A_CRZBO_STRIDE, CRZBO_STRIDE, CRZBO_R2A_STRIDE, U)\
    /* QBN_R4 - HLR branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN, CAMCTL_QBN_R4_EN, CAMCTL_R1A_QBN_R4_EN, U)\
    CMD(a, b, c, d, e, f, QBIN_R4A_QBIN_CTL, QBIN_ACC, QBIN_R4A_ACC, U)\
    CMD(a, b, c, d, e, f, QBIN_R4A_QBIN_CTL, QBIN_ACC_MODE, QBIN_R4A_ACC_MODE, U)\
    /* MRG_R4 - HLR branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN, CAMCTL_MRG_R4_EN, CAMCTL_R1A_MRG_R4_EN, U)\
    CMD(a, b, c, d, e, f, MRG_R4A_MRG_CTL, MRG_SIG_MODE1, MRG_R4A_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, MRG_R4A_MRG_CTL, MRG_SIG_MODE2, MRG_R4A_SIG_MODE2, U)\
    /*CMD(a, b, c, d, e, f, MRG_R4A_MRG_CTL, MRG_EDGE, MRG_R4A_EDGE, U)*/\
    CMD(a, b, c, d, e, f, MRG_R4A_MRG_CROP, MRG_STR_X, MRG_R4A_STR_X, U)\
    CMD(a, b, c, d, e, f, MRG_R4A_MRG_CROP, MRG_END_X, MRG_R4A_END_X, U)\
    /*CMD(a, b, c, d, e, f, MRG_R4A_MRG_VSIZE, MRG_HT, MRG_R4A_HT, U)*/\
    /* LTMS_SEL - HLR branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_SEL, CAMCTL_LTMS_SEL, CAMCTL_R1A_LTMS_SEL, U)\
    /* QBN_R1 - LSC branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN2, CAMCTL_QBN_R1_EN, CAMCTL_R1A_QBN_R1_EN, U)\
    CMD(a, b, c, d, e, f, QBIN_R1A_QBIN_CTL, QBIN_ACC, QBIN_R1A_ACC, U)\
    CMD(a, b, c, d, e, f, QBIN_R1A_QBIN_CTL, QBIN_ACC_MODE, QBIN_R1A_ACC_MODE, U)\
    /* MRG_R1 - LSC branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN, CAMCTL_MRG_R1_EN, CAMCTL_R1A_MRG_R1_EN, U)\
    CMD(a, b, c, d, e, f, MRG_R1A_MRG_CTL, MRG_SIG_MODE1, MRG_R1A_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, MRG_R1A_MRG_CTL, MRG_SIG_MODE2, MRG_R1A_SIG_MODE2, U)\
    /*CMD(a, b, c, d, e, f, MRG_R1A_MRG_CTL, MRG_EDGE, MRG_R1A_EDGE, U)*/\
    CMD(a, b, c, d, e, f, MRG_R1A_MRG_CROP, MRG_STR_X, MRG_R1A_STR_X, U)\
    CMD(a, b, c, d, e, f, MRG_R1A_MRG_CROP, MRG_END_X, MRG_R1A_END_X, U)\
    /*CMD(a, b, c, d, e, f, MRG_R1A_MRG_VSIZE, MRG_HT, MRG_R1A_HT, U)*/\
    /* AA_SEL - LSC branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_SEL, CAMCTL_AA_SEL, CAMCTL_R1A_AA_SEL, U)\
    /* AAO_R1 - LSC branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_DMA_EN, CAMCTL_AAO_R1_EN, CAMCTL_R1A_AAO_R1_EN, U)\
    /* CRP_R1_SEL - AFO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_SEL, CAMCTL_CRP_R1_SEL, CAMCTL_R1A_CRP_R1_SEL, U)\
    /* CRP_R1 - AFO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN2, CAMCTL_CRP_R1_EN, CAMCTL_R1A_CRP_R1_EN, U)\
    CMD(a, b, c, d, e, f, CRP_R1A_CRP_X_POS, CRP_XSTART, CRP_R1A_XSTART, U)\
    CMD(a, b, c, d, e, f, CRP_R1A_CRP_X_POS, CRP_XEND, CRP_R1A_XEND, U)\
    CMD(a, b, c, d, e, f, CRP_R1A_CRP_Y_POS, CRP_YSTART, CRP_R1A_YSTART, U)\
    CMD(a, b, c, d, e, f, CRP_R1A_CRP_Y_POS, CRP_YEND, CRP_R1A_YEND, U)\
    /* AF_R1 - AFO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN2, CAMCTL_AF_R1_EN, CAMCTL_R1A_AF_R1_EN, U)\
    CMD(a, b, c, d, e, f, AF_R1A_AF_BLK_0, AF_BLK_XSIZE, AF_R1A_BLK_XSIZE, U)\
    CMD(a, b, c, d, e, f, AF_R1A_AF_BLK_0, AF_BLK_YSIZE, AF_R1A_BLK_YSIZE, U)\
    CMD(a, b, c, d, e, f, AF_R1A_AF_BLK_1, AF_BLK_XNUM, AF_R1A_BLK_XNUM, U)\
    CMD(a, b, c, d, e, f, AF_R1A_AF_BLK_PROT, AF_PROT_BLK_XSIZE, AF_R1A_PROT_BLK_XSIZE, U)\
    CMD(a, b, c, d, e, f, AF_R1A_AF_BLK_PROT, AF_PROT_BLK_YSIZE, AF_R1A_PROT_BLK_YSIZE, U)\
    CMD(a, b, c, d, e, f, AF_R1A_AF_VLD, AF_VLD_XSTART, AF_R1A_VLD_XSTART, U)\
    CMD(a, b, c, d, e, f, AF_R1A_AF_BLK_1, AF_BLK_YNUM, AF_R1A_BLK_YNUM, U)\
    CMD(a, b, c, d, e, f, AF_R1A_AF_VLD, AF_VLD_YSTART, AF_R1A_VLD_YSTART, U)\
    CMD(a, b, c, d, e, f, AF_R1A_AF_CON2, AF_DS_EN, AF_R1A_DS_EN, U)\
    CMD(a, b, c, d, e, f, AF_R1A_AF_CON, AF_H_GONLY, AF_R1A_H_GONLY, U)\
    /* AF_SEL - AFO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_SEL, CAMCTL_AF_SEL, CAMCTL_R1A_AF_SEL, U)\
    /* AFO_R1 - AFO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_DMA_EN, CAMCTL_AFO_R1_EN, CAMCTL_R1A_AFO_R1_EN, U)\
    CMD(a, b, c, d, e, f, AFO_R1A_AFO_BASE_ADDR, AFO_BASE_ADDR, AFO_R1A_BASE_ADDR, U)\
    CMD(a, b, c, d, e, f, AFO_R1A_AFO_OFST_ADDR, AFO_OFST_ADDR, AFO_R1A_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, AFO_R1A_AFO_XSIZE, AFO_XSIZE, AFO_R1A_XSIZE, U)\
    CMD(a, b, c, d, e, f, AFO_R1A_AFO_YSIZE, AFO_YSIZE, AFO_R1A_YSIZE, U)\
    CMD(a, b, c, d, e, f, AFO_R1A_AFO_STRIDE, AFO_STRIDE, AFO_R1A_STRIDE, U)\
    /* QBN_R2 - TSFSO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN2, CAMCTL_QBN_R2_EN, CAMCTL_R1A_QBN_R2_EN, U)\
    CMD(a, b, c, d, e, f, QBIN_R2A_QBIN_CTL, QBIN_ACC, QBIN_R2A_ACC, U)\
    CMD(a, b, c, d, e, f, QBIN_R2A_QBIN_CTL, QBIN_ACC_MODE, QBIN_R2A_ACC_MODE, U)\
    /* MRG_R2 - TSFSO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN2, CAMCTL_MRG_R2_EN, CAMCTL_R1A_MRG_R2_EN, U)\
    CMD(a, b, c, d, e, f, MRG_R2A_MRG_CTL, MRG_SIG_MODE1, MRG_R2A_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, MRG_R2A_MRG_CTL, MRG_SIG_MODE2, MRG_R2A_SIG_MODE2, U)\
    /*CMD(a, b, c, d, e, f, MRG_R2A_MRG_CTL, MRG_EDGE, MRG_R2A_EDGE, U)*/\
    CMD(a, b, c, d, e, f, MRG_R2A_MRG_CROP, MRG_STR_X, MRG_R2A_STR_X, U)\
    CMD(a, b, c, d, e, f, MRG_R2A_MRG_CROP, MRG_END_X, MRG_R2A_END_X, U)\
    /*CMD(a, b, c, d, e, f, MRG_R2A_MRG_VSIZE, MRG_HT, MRG_R2A_HT, U)*/\
    /* TSFSO_R1 - TSFSO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_DMA_EN, CAMCTL_TSFSO_R1_EN, CAMCTL_R1A_TSFSO_R1_EN, U)\
    /* CRP_R3_SEL - IMGO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_SEL, CAMCTL_CRP_R3_SEL, CAMCTL_R1A_CRP_R3_SEL, U)\
    /* CRP_R3 - IMGO branch*/\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN, CAMCTL_CRP_R3_EN, CAMCTL_R1A_CRP_R3_EN, U)\
    CMD(a, b, c, d, e, f, CRP_R3A_CRP_X_POS, CRP_XSTART, CRP_R3A_XSTART, U)\
    CMD(a, b, c, d, e, f, CRP_R3A_CRP_X_POS, CRP_XEND, CRP_R3A_XEND, U)\
    CMD(a, b, c, d, e, f, CRP_R3A_CRP_Y_POS, CRP_YSTART, CRP_R3A_YSTART, U)\
    CMD(a, b, c, d, e, f, CRP_R3A_CRP_Y_POS, CRP_YEND, CRP_R3A_YEND, U)\
    /* IMG_SEL - IMGO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_SEL, CAMCTL_IMG_SEL, CAMCTL_R1A_IMG_SEL, U)\
    /* MRG_R3 - IMGO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN4, CAMCTL_MRG_R3_EN, CAMCTL_R1A_MRG_R3_EN, U)\
    CMD(a, b, c, d, e, f, MRG_R3A_MRG_CTL, MRG_SIG_MODE1, MRG_R3A_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, MRG_R3A_MRG_CTL, MRG_SIG_MODE2, MRG_R3A_SIG_MODE2, U)\
    /*CMD(a, b, c, d, e, f, MRG_R3A_MRG_CTL, MRG_EDGE, MRG_R3A_EDGE, U)*/\
    CMD(a, b, c, d, e, f, MRG_R3A_MRG_CROP, MRG_STR_X, MRG_R3A_STR_X, U)\
    CMD(a, b, c, d, e, f, MRG_R3A_MRG_CROP, MRG_END_X, MRG_R3A_END_X, U)\
    /*CMD(a, b, c, d, e, f, MRG_R3A_MRG_VSIZE, MRG_HT, MRG_R3A_HT, U)*/\
    /* MRG_R12 IMGO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN3, CAMCTL_MRG_R12_EN, CAMCTL_R1A_MRG_R12_EN, U)\
    CMD(a, b, c, d, e, f, MRG_R12A_MRG_CTL, MRG_SIG_MODE1, MRG_R12A_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, MRG_R12A_MRG_CTL, MRG_SIG_MODE2, MRG_R12A_SIG_MODE2, U)\
    /*CMD(a, b, c, d, e, f, MRG_R12A_MRG_CTL, MRG_EDGE, MRG_R12A_EDGE, U)*/\
    CMD(a, b, c, d, e, f, MRG_R12A_MRG_CROP, MRG_STR_X, MRG_R12A_STR_X, U)\
    CMD(a, b, c, d, e, f, MRG_R12A_MRG_CROP, MRG_END_X, MRG_R12A_END_X, U)\
    /*CMD(a, b, c, d, e, f, MRG_R12A_MRG_VSIZE, MRG_HT, MRG_R12A_HT, U)*/\
    /* UFE_R1 - IMGO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN3, CAMCTL_UFE_R1_EN, CAMCTL_R1A_UFE_R1_EN, U)\
    CMD(a, b, c, d, e, f, UFE_R1A_UFE_CON, UFE_FORCE_PCM, UFE_R1A_FORCE_PCM, U)\
    /* PAK_R1 - IMGO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN, CAMCTL_PAK_R1_EN, CAMCTL_R1A_PAK_R1_EN, U)\
    /* IMGO_SEL - IMGO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_SEL, CAMCTL_IMGO_SEL, CAMCTL_R1A_IMGO_SEL, U)\
    /* IMGO_R1 - IMGO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_DMA_EN, CAMCTL_IMGO_R1_EN, CAMCTL_R1A_IMGO_R1_EN, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_FMT_SEL, CAMCTL_IMGO_R1_FMT, CAMCTL_R1A_IMGO_R1_FMT, U)\
    CMD(a, b, c, d, e, f, IMGO_R1A_IMGO_BASE_ADDR, IMGO_BASE_ADDR, IMGO_R1A_BASE_ADDR, U)\
    CMD(a, b, c, d, e, f, IMGO_R1A_IMGO_OFST_ADDR, IMGO_OFST_ADDR, IMGO_R1A_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, IMGO_R1A_IMGO_XSIZE, IMGO_XSIZE, IMGO_R1A_XSIZE, U)\
    CMD(a, b, c, d, e, f, IMGO_R1A_IMGO_YSIZE, IMGO_YSIZE, IMGO_R1A_YSIZE, U)\
    CMD(a, b, c, d, e, f, IMGO_R1A_IMGO_STRIDE, IMGO_STRIDE, IMGO_R1A_STRIDE, U)\
    CMD(a, b, c, d, e, f, IMGO_R1A_IMGO_CROP, IMGO_XOFFSET, IMGO_R1A_XOFFSET, U)\
    CMD(a, b, c, d, e, f, IMGO_R1A_IMGO_CROP, IMGO_YOFFSET, IMGO_R1A_YOFFSET, U)\
    /* UFEO_R1 - IMGO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_DMA_EN, CAMCTL_UFEO_R1_EN, CAMCTL_R1A_UFEO_R1_EN, U)\
    CMD(a, b, c, d, e, f, UFEO_R1A_UFEO_BASE_ADDR, UFEO_BASE_ADDR, UFEO_R1A_BASE_ADDR, U)\
    CMD(a, b, c, d, e, f, UFEO_R1A_UFEO_OFST_ADDR, UFEO_OFST_ADDR, UFEO_R1A_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, UFEO_R1A_UFEO_XSIZE, UFEO_XSIZE, UFEO_R1A_XSIZE, U)\
    CMD(a, b, c, d, e, f, UFEO_R1A_UFEO_YSIZE, UFEO_YSIZE, UFEO_R1A_YSIZE, U)\
    CMD(a, b, c, d, e, f, UFEO_R1A_UFEO_STRIDE, UFEO_STRIDE, UFEO_R1A_STRIDE, U)\
    /* FLK_SEL - FLKO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_SEL, CAMCTL_FLK_SEL, CAMCTL_R1A_FLK_SEL, U)\
    /*QBN_R3 - FLKO branch*/\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN3, CAMCTL_QBN_R3_EN, CAMCTL_R1A_QBN_R3_EN, U)\
    CMD(a, b, c, d, e, f, QBIN_R3A_QBIN_CTL, QBIN_ACC, QBIN_R3A_ACC, U)\
    CMD(a, b, c, d, e, f, QBIN_R3A_QBIN_CTL, QBIN_ACC_MODE, QBIN_R3A_ACC_MODE, U)\
    /* MRG_R10 - FLKO branch */\
    /*CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN, CAMCTL_MRG_R10_EN, CAMCTL_R1A_MRG_R10_EN, U)*/\
    CMD(a, b, c, d, e, f, MRG_R10A_MRG_CTL, MRG_SIG_MODE1, MRG_R10A_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, MRG_R10A_MRG_CTL, MRG_SIG_MODE2, MRG_R10A_SIG_MODE2, U)\
    /*CMD(a, b, c, d, e, f, MRG_R10A_MRG_CTL, MRG_EDGE, MRG_R10A_EDGE, U)*/\
    CMD(a, b, c, d, e, f, MRG_R10A_MRG_CROP, MRG_STR_X, MRG_R10A_STR_X, U)\
    CMD(a, b, c, d, e, f, MRG_R10A_MRG_CROP, MRG_END_X, MRG_R10A_END_X, U)\
    /*CMD(a, b, c, d, e, f, MRG_R10A_MRG_VSIZE, MRG_HT, MRG_R10A_HT, U)*/\
    /* MRG_R6 - ADL branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1A_CAMCTL_EN3, CAMCTL_MRG_R6_EN, CAMCTL_R1A_MRG_R6_EN, U)\
    CMD(a, b, c, d, e, f, MRG_R6A_MRG_CTL, MRG_SIG_MODE1, MRG_R6A_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, MRG_R6A_MRG_CTL, MRG_SIG_MODE2, MRG_R6A_SIG_MODE2, U)\
    /*CMD(a, b, c, d, e, f, MRG_R6A_MRG_CTL, MRG_EDGE, MRG_R6A_EDGE, U)*/\
    CMD(a, b, c, d, e, f, MRG_R6A_MRG_CROP, MRG_STR_X, MRG_R6A_STR_X, U)\
    CMD(a, b, c, d, e, f, MRG_R6A_MRG_CROP, MRG_END_X, MRG_R6A_END_X, U)\
    /*CMD(a, b, c, d, e, f, MRG_R6A_MRG_VSIZE, MRG_HT, MRG_R6A_HT, U)*/\

/* PLATFORM parameters */
/* a, b, c, d, e, f reserved */
/* struct name */
/* variable name */
/* R/W type: R, RW, W */
#define DUAL_HW_B_LIST(CMD, a, b, c, d, e, f) \
    /* TG_R1 */\
    CMD(a, b, c, d, e, f, TG_R1B_TG_SEN_GRAB_PXL, TG_PXL_S, TG_R1B_PXL_S, U)\
    CMD(a, b, c, d, e, f, TG_R1B_TG_SEN_GRAB_PXL, TG_PXL_E, TG_R1B_PXL_E, U)\
    CMD(a, b, c, d, e, f, TG_R1B_TG_SEN_GRAB_LIN, TG_LIN_S, TG_R1B_LIN_S, U)\
    CMD(a, b, c, d, e, f, TG_R1B_TG_SEN_GRAB_LIN, TG_LIN_E, TG_R1B_LIN_E, U)\
    /* RAWI_R2 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_DMA_EN, CAMCTL_RAWI_R2_EN, CAMCTL_R1B_RAWI_R2_EN, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_FMT2_SEL, CAMCTL_PIX_BUS_RAWI_R2, CAMCTL_R1B_PIX_BUS_RAWI_R2, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_FMT3_SEL, CAMCTL_RAWI_R2_FMT, CAMCTL_R1B_RAWI_R2_FMT, U)\
    CMD(a, b, c, d, e, f, RAWI_R2B_RAWI_OFST_ADDR, RAWI_OFST_ADDR, RAWI_R2B_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, RAWI_R2B_RAWI_XSIZE, RAWI_XSIZE, RAWI_R2B_XSIZE, U)\
    CMD(a, b, c, d, e, f, RAWI_R2B_RAWI_YSIZE, RAWI_YSIZE, RAWI_R2B_YSIZE, U)\
    CMD(a, b, c, d, e, f, RAWI_R2B_RAWI_STRIDE, RAWI_STRIDE, RAWI_R2B_STRIDE, U)\
    /* UNP_R2 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN, CAMCTL_UNP_R2_EN, CAMCTL_R1B_UNP_R2_EN, U)\
    CMD(a, b, c, d, e, f, UNP_R2B_UNP_OFST, UNP_OFST_STB, UNP_R2B_OFST_STB, U)\
    CMD(a, b, c, d, e, f, UNP_R2B_UNP_OFST, UNP_OFST_EDB, UNP_R2B_OFST_EDB, U)\
    /* UFDI_R2 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_DMA_EN, CAMCTL_UFDI_R2_EN, CAMCTL_R1B_UFDI_R2_EN, U)\
    CMD(a, b, c, d, e, f, UFDI_R2B_UFDI_XSIZE, UFDI_XSIZE, UFDI_R2B_XSIZE, U)\
    CMD(a, b, c, d, e, f, UFDI_R2B_UFDI_YSIZE, UFDI_YSIZE, UFDI_R2B_YSIZE, U)\
    CMD(a, b, c, d, e, f, UFDI_R2B_UFDI_STRIDE, UFDI_STRIDE, UFDI_R2B_STRIDE, U)\
    /* UFD_R2 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN3, CAMCTL_UFD_R2_EN, CAMCTL_R1B_UFD_R2_EN, U)\
    CMD(a, b, c, d, e, f, UFD_R2B_UFD_AU_CON, UFD_AU_OFST, UFD_R2B_AU_OFST, U)\
    CMD(a, b, c, d, e, f, UFD_R2B_UFD_AU_CON, UFD_AU_SIZE, UFD_R2B_AU_SIZE, U)\
    CMD(a, b, c, d, e, f, UFD_R2B_UFD_AU2_CON, UFD_AU2_OFST, UFD_R2B_AU2_OFST, U)\
    CMD(a, b, c, d, e, f, UFD_R2B_UFD_AU2_CON, UFD_AU2_SIZE, UFD_R2B_AU2_SIZE, U)\
    CMD(a, b, c, d, e, f, UFD_R2B_UFD_BS2_AU_CON, UFD_BS2_AU_START, UFD_R2B_BS2_AU_START, U)\
    CMD(a, b, c, d, e, f, UFD_R2B_UFD_BS2_AU_CON, UFD_BOND_MODE, UFD_R2B_BOND_MODE, U)\
    CMD(a, b, c, d, e, f, UFD_R2B_UFD_AU3_CON, UFD_AU3_OFST, UFD_R2B_AU3_OFST, U)\
    CMD(a, b, c, d, e, f, UFD_R2B_UFD_AU3_CON, UFD_AU3_SIZE, UFD_R2B_AU3_SIZE, U)\
    CMD(a, b, c, d, e, f, UFD_R2B_UFD_BS3_AU_CON, UFD_BS3_AU_START, UFD_R2B_BS3_AU_START, U)\
    CMD(a, b, c, d, e, f, UFD_R2B_UFD_BS3_AU_CON, UFD_BOND2_MODE, UFD_R2B_BOND2_MODE, U)\
    CMD(a, b, c, d, e, f, UFD_R2B_UFD_CROP_CON1, UFD_X_START, UFD_R2B_X_START, U)\
    CMD(a, b, c, d, e, f, UFD_R2B_UFD_CROP_CON1, UFD_X_END, UFD_R2B_X_END, U)\
    CMD(a, b, c, d, e, f, UFD_R2B_UFD_CROP_CON2, UFD_Y_START, UFD_R2B_Y_START, U)\
    CMD(a, b, c, d, e, f, UFD_R2B_UFD_CROP_CON2, UFD_Y_END, UFD_R2B_Y_END, U)\
    CMD(a, b, c, d, e, f, UFD_R2B_UFD_SIZE_CON, UFD_WD, UFD_R2B_WD, U)\
    CMD(a, b, c, d, e, f, UFD_R2B_UFD_SIZE_CON, UFD_HT, UFD_R2B_HT, U)\
    CMD(a, b, c, d, e, f, UFD_R2B_UFD_CON, UFD_SEL, UFD_R2B_SEL, U)\
    CMD(a, b, c, d, e, f, UFD_R2B_UFD_ADDRESS_CON1, UFD_BITSTREAM2_OFST_ADDR, UFD_R2B_BITSTREAM2_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, UFD_R2B_UFD_ADDRESS_CON2, UFD_BITSTREAM3_OFST_ADDR, UFD_R2B_BITSTREAM3_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, UFD_R2B_UFD_ADDRESS_CON3, UFD_BITSTREAM4_OFST_ADDR, UFD_R2B_BITSTREAM4_OFST_ADDR, U)\
    /* DCPN_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN4, CAMCTL_DCPN_R1_EN, CAMCTL_R1B_DCPN_R1_EN, U)\
    /* DBN_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN2, CAMCTL_DBN_R1_EN, CAMCTL_R1B_DBN_R1_EN, U)\
    /* BIN_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN2, CAMCTL_BIN_R1_EN, CAMCTL_R1B_BIN_R1_EN, U)\
    /* SEL */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_SEL, CAMCTL_RAW_SEL, CAMCTL_R1B_RAW_SEL, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_SEL2, CAMCTL_RAWI_UFO_SEL, CAMCTL_R1B_RAWI_UFO_SEL, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_SEL, CAMCTL_SEP_SEL, CAMCTL_R1B_SEP_SEL, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_SEL, CAMCTL_BPC_R2_SEL, CAMCTL_R1B_BPC_R2_SEL, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_SEL2, CAMCTL_BPC_R3_SEL, CAMCTL_R1B_BPC_R3_SEL, U)\
    /* PIXEL MODE */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_FMT2_SEL, CAMCTL_PIX_BUS_SEPI, CAMCTL_R1B_PIX_BUS_SEPI, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_FMT_SEL, CAMCTL_PIX_BUS_SEPO, CAMCTL_R1B_PIX_BUS_SEPO, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_FMT_SEL, CAMCTL_PIX_BUS_MRG_R3O, CAMCTL_R1B_PIX_BUS_MRG_R3O, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_FMT_SEL, CAMCTL_PIX_BUS_MRG_R7O, CAMCTL_R1B_PIX_BUS_MRG_R7O, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_FMT_SEL, CAMCTL_PIX_BUS_MRG_R8O, CAMCTL_R1B_PIX_BUS_MRG_R8O, U)\
    /* SEP_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN, CAMCTL_SEP_R1_EN, CAMCTL_R1B_SEP_R1_EN, U)\
    /*CMD(a, b, c, d, e, f, SEP_R1B_SEP_CTL, SEP_EDGE, SEP_R1B_SEP_EDGE, U)*/\
    CMD(a, b, c, d, e, f, SEP_R1B_SEP_CROP, SEP_STR_X, SEP_R1B_STR_X, U)\
    CMD(a, b, c, d, e, f, SEP_R1B_SEP_CROP, SEP_END_X, SEP_R1B_END_X, U)\
    CMD(a, b, c, d, e, f, SEP_R1B_SEP_VSIZE, SEP_HT, SEP_R1B_SEP_HT, U)\
    /* IBPC_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN4, CAMCTL_IBPC_R1_EN, CAMCTL_R1B_IBPC_R1_EN, U)\
    /* IOBC_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN4, CAMCTL_IOBC_R1_EN, CAMCTL_R1B_IOBC_R1_EN, U)\
    /* IRM_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN4, CAMCTL_IRM_R1_EN, CAMCTL_R1B_IRM_R1_EN, U)\
    /* FBND_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN2, CAMCTL_FBND_R1_EN, CAMCTL_R1B_FBND_R1_EN, U)\
    /* OBC_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN, CAMCTL_OBC_R1_EN, CAMCTL_R1B_OBC_R1_EN, U)\
    /* BPC_R1 */\
	CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN, CAMCTL_BPC_R1_EN, CAMCTL_R1B_BPC_R1_EN, U)\
	CMD(a, b, c, d, e, f, BPC_R1B_BPC_BPC_TBLI1, BPC_XOFFSET, BPC_R1B_XOFFSET, U)\
	CMD(a, b, c, d, e, f, BPC_R1B_BPC_BPC_TBLI1, BPC_YOFFSET, BPC_R1B_YOFFSET, U)\
	CMD(a, b, c, d, e, f, BPC_R1B_BPC_BPC_TBLI2, BPC_XSIZE, BPC_R1B_XSIZE, U)\
	CMD(a, b, c, d, e, f, BPC_R1B_BPC_BPC_TBLI2, BPC_YSIZE, BPC_R1B_YSIZE, U)\
	CMD(a, b, c, d, e, f, BPC_R1B_BPC_BPC_CON, BPC_LE_INV_CTL, BPC_R1B_LE_INV_CTL, U)\
	CMD(a, b, c, d, e, f, BPC_R1B_BPC_PDC_POS, BPC_PDC_XCENTER, BPC_R1B_PDC_XCENTER, U)\
	CMD(a, b, c, d, e, f, BPC_R1B_BPC_PDC_POS, BPC_PDC_XCENTER, BPC_R1B_PDC_YCENTER, U)\
	CMD(a, b, c, d, e, f, BPC_R1B_BPC_PDC_CON, BPC_PDC_EN, BPC_R1B_PDC_EN, U)\
    /* BPCI_R1 */\
    CMD(a, b, c, d, e, f, BPCI_R1B_BPCI_XSIZE, BPCI_XSIZE, BPCI_R1B_XSIZE, U)\
    CMD(a, b, c, d, e, f, BPCI_R1B_BPCI_YSIZE, BPCI_YSIZE, BPCI_R1B_YSIZE, U)\
    CMD(a, b, c, d, e, f, BPCI_R1B_BPCI_STRIDE, BPCI_STRIDE, BPCI_R1B_STRIDE, U)\
    /* OBC_R2 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN2, CAMCTL_OBC_R2_EN, CAMCTL_R1B_OBC_R2_EN, U)\
    /* BPC_R2 */\
	CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN2, CAMCTL_BPC_R2_EN, CAMCTL_R1B_BPC_R2_EN, U)\
	CMD(a, b, c, d, e, f, BPC_R2B_BPC_BPC_TBLI1, BPC_XOFFSET, BPC_R2B_XOFFSET, U)\
	CMD(a, b, c, d, e, f, BPC_R2B_BPC_BPC_TBLI1, BPC_YOFFSET, BPC_R2B_YOFFSET, U)\
	CMD(a, b, c, d, e, f, BPC_R2B_BPC_BPC_TBLI2, BPC_XSIZE, BPC_R2B_XSIZE, U)\
	CMD(a, b, c, d, e, f, BPC_R2B_BPC_BPC_TBLI2, BPC_YSIZE, BPC_R2B_YSIZE, U)\
    CMD(a, b, c, d, e, f, BPC_R2B_BPC_BPC_CON, BPC_LE_INV_CTL, BPC_R2B_LE_INV_CTL, U)\
	CMD(a, b, c, d, e, f, BPC_R2B_BPC_PDC_POS, BPC_PDC_XCENTER, BPC_R2B_PDC_XCENTER, U)\
	CMD(a, b, c, d, e, f, BPC_R2B_BPC_PDC_POS, BPC_PDC_XCENTER, BPC_R2B_PDC_YCENTER, U)\
	CMD(a, b, c, d, e, f, BPC_R2B_BPC_PDC_CON, BPC_PDC_EN, BPC_R2B_PDC_EN, U)\
    /* BPCI_R2 */\
    CMD(a, b, c, d, e, f, BPCI_R2B_BPCI_XSIZE, BPCI_XSIZE, BPCI_R2B_XSIZE, U)\
    CMD(a, b, c, d, e, f, BPCI_R2B_BPCI_YSIZE, BPCI_YSIZE, BPCI_R2B_YSIZE, U)\
    CMD(a, b, c, d, e, f, BPCI_R2B_BPCI_STRIDE, BPCI_STRIDE, BPCI_R2B_STRIDE, U)\
    /* ZFUS_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN2, CAMCTL_ZFUS_R1_EN, CAMCTL_R1B_ZFUS_R1_EN, U)\
    /* FUS_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN, CAMCTL_FUS_R1_EN, CAMCTL_R1B_FUS_R1_EN, U)\
    CMD(a, b, c, d, e, f, FUS_R1B_FUS_CFG, FUS_INPUT_MODE, FUS_R1B_INPUT_MODE, U)\
    CMD(a, b, c, d, e, f, FUS_R1B_FUS_SIZE, FUS_HSIZE, FUS_R1B_HSIZE, U)\
    CMD(a, b, c, d, e, f, FUS_R1B_FUS_SIZE, FUS_VSIZE, FUS_R1B_VSIZE, U)\
    /* RNR_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN4, CAMCTL_RNR_R1_EN, CAMCTL_R1B_RNR_R1_EN, U)\
    /* DGN_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN, CAMCTL_DGN_R1_EN, CAMCTL_R1B_DGN_R1_EN, U)\
    /* LSC_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN, CAMCTL_LSC_R1_EN, CAMCTL_R1B_LSC_R1_EN, U)\
    CMD(a, b, c, d, e, f, LSC_R1B_LSC_CTL1, LSC_EXTEND_COEF_MODE, LSC_R1B_EXTEND_COEF_MODE, U)\
    CMD(a, b, c, d, e, f, LSC_R1B_LSC_CTL2, LSC_SDBLK_XNUM, LSC_R1B_SDBLK_XNUM, U)\
    CMD(a, b, c, d, e, f, LSC_R1B_LSC_CTL2, LSC_SDBLK_WIDTH, LSC_R1B_SDBLK_WIDTH, U)\
    CMD(a, b, c, d, e, f, LSC_R1B_LSC_LBLOCK, LSC_SDBLK_lWIDTH, LSC_R1B_SDBLK_lWIDTH, U)\
    CMD(a, b, c, d, e, f, LSC_R1B_LSC_TPIPE_OFST, LSC_TPIPE_OFST_X, LSC_R1B_TPIPE_OFST_X, U)\
    CMD(a, b, c, d, e, f, LSC_R1B_LSC_TPIPE_SIZE, LSC_TPIPE_SIZE_X, LSC_R1B_TPIPE_SIZE_X, U)\
    CMD(a, b, c, d, e, f, LSC_R1B_LSC_CTL3, LSC_SDBLK_HEIGHT, LSC_R1B_SDBLK_HEIGHT, U)\
    CMD(a, b, c, d, e, f, LSC_R1B_LSC_LBLOCK, LSC_SDBLK_lHEIGHT, LSC_R1B_SDBLK_lHEIGHT, U)\
    CMD(a, b, c, d, e, f, LSC_R1B_LSC_TPIPE_OFST, LSC_TPIPE_OFST_Y, LSC_R1B_TPIPE_OFST_Y, U)\
    CMD(a, b, c, d, e, f, LSC_R1B_LSC_TPIPE_SIZE, LSC_TPIPE_SIZE_Y, LSC_R1B_TPIPE_SIZE_Y, U)\
    /* LSCI_R1 */\
    CMD(a, b, c, d, e, f, LSCI_R1B_LSCI_OFST_ADDR, LSCI_OFST_ADDR, LSCI_R1B_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, LSCI_R1B_LSCI_XSIZE, LSCI_XSIZE, LSCI_R1B_XSIZE, U)\
    CMD(a, b, c, d, e, f, LSCI_R1B_LSCI_STRIDE, LSCI_STRIDE, LSCI_R1B_STRIDE, U)\
    /* WB_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN, CAMCTL_WB_R1_EN, CAMCTL_R1B_WB_R1_EN, U)\
    /* HLR_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN2, CAMCTL_HLR_R1_EN, CAMCTL_R1B_HLR_R1_EN, U)\
    /* LTM_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN, CAMCTL_LTM_R1_EN, CAMCTL_R1B_LTM_R1_EN, U)\
    CMD(a, b, c, d, e, f, LTM_R1B_LTM_BLK_NUM, LTM_BLK_X_NUM, LTM_R1B_BLK_X_NUM, U)\
    CMD(a, b, c, d, e, f, LTM_R1B_LTM_BLK_NUM, LTM_BLK_Y_NUM, LTM_R1B_BLK_Y_NUM, U)\
    CMD(a, b, c, d, e, f, LTM_R1B_LTM_BLK_SZ, LTM_BLK_WIDTH, LTM_R1B_BLK_WIDTH, U)\
    CMD(a, b, c, d, e, f, LTM_R1B_LTM_BLK_SZ, LTM_BLK_HEIGHT, LTM_R1B_BLK_HEIGHT, U)\
    CMD(a, b, c, d, e, f, LTM_R1B_LTM_TILE_NUM, LTM_TILE_BLK_X_NUM_START, LTM_R1B_TILE_BLK_X_NUM_START, U)\
    CMD(a, b, c, d, e, f, LTM_R1B_LTM_TILE_NUM, LTM_TILE_BLK_X_NUM_END, LTM_R1B_TILE_BLK_X_NUM_END, U)\
    CMD(a, b, c, d, e, f, LTM_R1B_LTM_TILE_NUM, LTM_TILE_BLK_Y_NUM_START, LTM_R1B_TILE_BLK_Y_NUM_START, U)\
    CMD(a, b, c, d, e, f, LTM_R1B_LTM_TILE_NUM, LTM_TILE_BLK_Y_NUM_END, LTM_R1B_TILE_BLK_Y_NUM_END, U)\
    CMD(a, b, c, d, e, f, LTM_R1B_LTM_TILE_CNTX, LTM_TILE_BLK_X_CNT_START, LTM_R1B_TILE_BLK_X_CNT_START, U)\
    CMD(a, b, c, d, e, f, LTM_R1B_LTM_TILE_CNTX, LTM_TILE_BLK_X_CNT_END, LTM_R1B_TILE_BLK_X_CNT_END, U)\
    CMD(a, b, c, d, e, f, LTM_R1B_LTM_TILE_CNTY, LTM_TILE_BLK_Y_CNT_START, LTM_R1B_TILE_BLK_Y_CNT_START, U)\
    CMD(a, b, c, d, e, f, LTM_R1B_LTM_TILE_CNTY, LTM_TILE_BLK_Y_CNT_END, LTM_R1B_TILE_BLK_Y_CNT_END, U)\
    /* RRZ_R1 - RRZ branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN2, CAMCTL_RRZ_R1_EN, CAMCTL_R1B_RRZ_R1_EN, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN4, CAMCTL_RLB_R1_EN, CAMCTL_R1B_RLB_R1_EN, U)\
    CMD(a, b, c, d, e, f, RRZ_R1B_RRZ_VERT_STEP, RRZ_VERT_STEP, RRZ_tmp2_VERT_STEP, U)\
    CMD(a, b, c, d, e, f, RRZ_R1B_RRZ_VERT_INT_OFST, RRZ_VERT_INT_OFST, RRZ_tmp2_VERT_INT_OFST, U)\
    CMD(a, b, c, d, e, f, RRZ_R1B_RRZ_VERT_SUB_OFST, RRZ_VERT_SUB_OFST, RRZ_tmp2_VERT_SUB_OFST, U)\
    CMD(a, b, c, d, e, f, RRZ_R1B_RRZ_OUT_IMG, RRZ_OUT_HT, RRZ_tmp2_OUT_HT, U)\
    CMD(a, b, c, d, e, f, RRZ_R1B_RRZ_OUT_IMG, RRZ_OUT_WD, RRZ_tmp2_OUT_WD, U)\
    CMD(a, b, c, d, e, f, RRZ_R1B_RRZ_HORI_STEP, RRZ_HORI_STEP, RRZ_tmp2_HORI_STEP, U)\
    CMD(a, b, c, d, e, f, RRZ_R1B_RRZ_HORI_INT_OFST, RRZ_HORI_INT_OFST, RRZ_tmp2_HORI_INT_OFST, U)\
    CMD(a, b, c, d, e, f, RRZ_R1B_RRZ_HORI_SUB_OFST, RRZ_HORI_SUB_OFST, RRZ_tmp2_HORI_SUB_OFST, U)\
    /*CMD(a, b, c, d, e, f, RRZ_R1B_RRZ_RLB_AOFST, RRZ_RLB_AOFST, RRZ_tmp2_RLB_AOFST, U)*/\
    /* HDS_SEL - RRZ branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_SEL, CAMCTL_HDS_SEL, CAMCTL_R1B_HDS_SEL, U)\
    /* GSE - RRZ branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN2, CAMCTL_GSE_R1_EN, CAMCTL_R1B_GSE_R1_EN, U)\
    /* MRG_R8 - RRZ branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN2, CAMCTL_MRG_R8_EN, CAMCTL_R1B_MRG_R8_EN, U)\
    CMD(a, b, c, d, e, f, MRG_R8B_MRG_CTL, MRG_SIG_MODE1, MRG_R8B_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, MRG_R8B_MRG_CTL, MRG_SIG_MODE2, MRG_R8B_SIG_MODE2, U)\
    /*CMD(a, b, c, d, e, f, MRG_R8B_MRG_CTL, MRG_EDGE, MRG_R8B_EDGE, U)*/\
    CMD(a, b, c, d, e, f, MRG_R8B_MRG_CROP, MRG_STR_X, MRG_R8B_STR_X, U)\
    CMD(a, b, c, d, e, f, MRG_R8B_MRG_CROP, MRG_END_X, MRG_R8B_END_X, U)\
    /*CMD(a, b, c, d, e, f, MRG_R8B_MRG_VSIZE, MRG_HT, MRG_R8B_HT, U)*/\
    /* MRG_R7 - RRZ branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN, CAMCTL_MRG_R7_EN, CAMCTL_R1B_MRG_R7_EN, U)\
    CMD(a, b, c, d, e, f, MRG_R7B_MRG_CTL, MRG_SIG_MODE1, MRG_R7B_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, MRG_R7B_MRG_CTL, MRG_SIG_MODE2, MRG_R7B_SIG_MODE2, U)\
    /*CMD(a, b, c, d, e, f, MRG_R7B_MRG_CTL, MRG_EDGE, MRG_R7B_EDGE, U)*/\
    CMD(a, b, c, d, e, f, MRG_R7B_MRG_CROP, MRG_STR_X, MRG_R7B_STR_X, U)\
    CMD(a, b, c, d, e, f, MRG_R7B_MRG_CROP, MRG_END_X, MRG_R7B_END_X, U)\
    /*CMD(a, b, c, d, e, f, MRG_R7B_MRG_VSIZE, MRG_HT, MRG_R7B_HT, U)*/\
    /* MRG_R11 - RRZ branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN3, CAMCTL_MRG_R11_EN, CAMCTL_R1B_MRG_R11_EN, U)\
    CMD(a, b, c, d, e, f, MRG_R11B_MRG_CTL, MRG_SIG_MODE1, MRG_R11B_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, MRG_R11B_MRG_CTL, MRG_SIG_MODE2, MRG_R11B_SIG_MODE2, U)\
    /*CMD(a, b, c, d, e, f, MRG_R11B_MRG_CTL, MRG_EDGE, MRG_R11B_EDGE, U)*/\
    CMD(a, b, c, d, e, f, MRG_R11B_MRG_CROP, MRG_STR_X, MRG_R11B_STR_X, U)\
    CMD(a, b, c, d, e, f, MRG_R11B_MRG_CROP, MRG_END_X, MRG_R11B_END_X, U)\
    /*CMD(a, b, c, d, e, f, MRG_R11B_MRG_VSIZE, MRG_HT, MRG_R11B_HT, U)*/\
    /* UFEG_SEL - RRZ branch*/\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_SEL, CAMCTL_UFEG_SEL, CAMCTL_R1B_UFEG_SEL, U)\
    /* PAKG_R1 - RRZ branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN, CAMCTL_PAKG_R1_EN, CAMCTL_R1B_PAKG_R1_EN, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_FMT2_SEL, CAMCTL_PAKG_R1_FG_OUT, CAMCTL_R1B_PAKG_R1_FG_OUT, U)\
    /* UFG_R1 - RRZ branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN2, CAMCTL_UFG_R1_EN, CAMCTL_R1B_UFG_R1_EN, U)\
    CMD(a, b, c, d, e, f, UFG_R1B_UFG_CON, UFG_FORCE_PCM, UFG_R1B_FORCE_PCM, U)\
    /* RRZO_R1 - RRZ branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_FMT_SEL, CAMCTL_RRZO_R1_FMT, CAMCTL_R1B_RRZO_R1_FMT, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_FMT_SEL, CAMCTL_RRZO_FG_MODE, CAMCTL_R1B_RRZO_FG_MODE, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_DMA_EN, CAMCTL_RRZO_R1_EN, CAMCTL_R1B_RRZO_R1_EN, U)\
    CMD(a, b, c, d, e, f, RRZO_R1B_RRZO_BASE_ADDR, RRZO_BASE_ADDR, RRZO_R1B_BASE_ADDR, U)\
    CMD(a, b, c, d, e, f, RRZO_R1B_RRZO_OFST_ADDR, RRZO_OFST_ADDR, RRZO_R1B_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, RRZO_R1B_RRZO_XSIZE, RRZO_XSIZE, RRZO_R1B_XSIZE, U)\
    CMD(a, b, c, d, e, f, RRZO_R1B_RRZO_YSIZE, RRZO_YSIZE, RRZO_R1B_YSIZE, U)\
    CMD(a, b, c, d, e, f, RRZO_R1B_RRZO_STRIDE, RRZO_STRIDE, RRZO_R1B_STRIDE, U)\
    /* UFGO_R1 - RRZ branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_DMA_EN, CAMCTL_UFGO_R1_EN, CAMCTL_R1B_UFGO_R1_EN, U)\
    CMD(a, b, c, d, e, f, UFGO_R1B_UFGO_BASE_ADDR, UFGO_BASE_ADDR, UFGO_R1B_BASE_ADDR, U)\
    CMD(a, b, c, d, e, f, UFGO_R1B_UFGO_OFST_ADDR, UFGO_OFST_ADDR, UFGO_R1B_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, UFGO_R1B_UFGO_XSIZE, UFGO_XSIZE, UFGO_R1B_XSIZE, U)\
    CMD(a, b, c, d, e, f, UFGO_R1B_UFGO_YSIZE, UFGO_YSIZE, UFGO_R1B_YSIZE, U)\
    CMD(a, b, c, d, e, f, UFGO_R1B_UFGO_STRIDE, UFGO_STRIDE, UFGO_R1B_STRIDE, U)\
    /* QBN_R5 - LTM branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN2, CAMCTL_QBN_R5_EN, CAMCTL_R1B_QBN_R5_EN, U)\
    CMD(a, b, c, d, e, f, QBIN_R5B_QBIN_CTL, QBIN_ACC, QBIN_R5B_ACC, U)\
    CMD(a, b, c, d, e, f, QBIN_R5B_QBIN_CTL, QBIN_ACC_MODE, QBIN_R5B_ACC_MODE, U)\
    /* MRG_R5 - LTM branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN, CAMCTL_MRG_R5_EN, CAMCTL_R1B_MRG_R5_EN, U)\
    CMD(a, b, c, d, e, f, MRG_R5B_MRG_CTL, MRG_SIG_MODE1, MRG_R5B_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, MRG_R5B_MRG_CTL, MRG_SIG_MODE2, MRG_R5B_SIG_MODE2, U)\
    /*CMD(a, b, c, d, e, f, MRG_R5B_MRG_CTL, MRG_EDGE, MRG_R5B_EDGE, U)*/\
    CMD(a, b, c, d, e, f, MRG_R5B_MRG_CROP, MRG_STR_X, MRG_R5B_STR_X, U)\
    CMD(a, b, c, d, e, f, MRG_R5B_MRG_CROP, MRG_END_X, MRG_R5B_END_X, U)\
    /*CMD(a, b, c, d, e, f, MRG_R5B_MRG_VSIZE, MRG_HT, MRG_R5B_HT, U)*/\
    /* LCS_SEL - LTM branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_SEL, CAMCTL_LCES_SEL, CAMCTL_R1B_LCES_SEL, U)\
    /* LCESO_R1 - LTM branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_DMA_EN, CAMCTL_LCESO_R1_EN, CAMCTL_R1B_LCESO_R1_EN, U)\
     /* DM_R1_SEL - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_SEL2, CAMCTL_DM_R1_SEL, CAMCTL_R1B_DM_R1_SEL, U)\
    /* MRG_R13 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN3, CAMCTL_MRG_R13_EN, CAMCTL_R1B_MRG_R13_EN, U)\
    CMD(a, b, c, d, e, f, MRG_R13B_MRG_CTL, MRG_SIG_MODE1, MRG_R13B_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, MRG_R13B_MRG_CTL, MRG_SIG_MODE2, MRG_R13B_SIG_MODE2, U)\
    /*CMD(a, b, c, d, e, f, MRG_R13B_MRG_CTL, MRG_EDGE, MRG_R13B_EDGE, U)*/\
    CMD(a, b, c, d, e, f, MRG_R13B_MRG_CROP, MRG_STR_X, MRG_R13B_STR_X, U)\
    CMD(a, b, c, d, e, f, MRG_R13B_MRG_CROP, MRG_END_X, MRG_R13B_END_X, U)\
    /*CMD(a, b, c, d, e, f, MRG_R13B_MRG_VSIZE, MRG_HT, MRG_R13B_HT, U)*/\
    /* MRG_R14 YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN3, CAMCTL_MRG_R14_EN, CAMCTL_R1B_MRG_R14_EN, U)\
    CMD(a, b, c, d, e, f, MRG_R14B_MRG_CTL, MRG_SIG_MODE1, MRG_R14B_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, MRG_R14B_MRG_CTL, MRG_SIG_MODE2, MRG_R14B_SIG_MODE2, U)\
    /*CMD(a, b, c, d, e, f, MRG_R14B_MRG_CTL, MRG_EDGE, MRG_R14B_EDGE, U)*/\
    CMD(a, b, c, d, e, f, MRG_R14B_MRG_CROP, MRG_STR_X, MRG_R14B_STR_X, U)\
    CMD(a, b, c, d, e, f, MRG_R14B_MRG_CROP, MRG_END_X, MRG_R14B_END_X, U)\
    /*CMD(a, b, c, d, e, f, MRG_R14B_MRG_VSIZE, MRG_HT, MRG_R14B_HT, U)*/\
    /* DM_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN2, CAMCTL_DM_R1_EN, CAMCTL_R1B_DM_R1_EN, U)\
    CMD(a, b, c, d, e, f, DM_R1B_DM_SL_CTL, DM_SL_EN, DM_R1B_SL_EN, U)\
    /* SLK_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN, CAMCTL_SLK_R1_EN, CAMCTL_R1B_SLK_R1_EN, U)\
    CMD(a, b, c, d, e, f, SLK_R1B_SLK_RZ, SLK_HRZ_COMP, SLK_R1B_HRZ_COMP, U)\
    CMD(a, b, c, d, e, f, SLK_R1B_SLK_RZ, SLK_VRZ_COMP, SLK_R1B_VRZ_COMP, U)\
    CMD(a, b, c, d, e, f, SLK_R1B_SLK_XOFF, SLK_X_OFST, SLK_R1B_X_OFST, U)\
    CMD(a, b, c, d, e, f, SLK_R1B_SLK_SIZE, SLK_TPIPE_WD, SLK_R1B_TPIPE_WD, U)\
    CMD(a, b, c, d, e, f, SLK_R1B_SLK_SIZE, SLK_TPIPE_HT, SLK_R1B_TPIPE_HT, U)\
    /* FLC_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN3, CAMCTL_FLC_R1_EN, CAMCTL_R1B_FLC_R1_EN, U)\
    /* CCM_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN2, CAMCTL_CCM_R1_EN, CAMCTL_R1B_CCM_R1_EN, U)\
    /* GGM_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN2, CAMCTL_GGM_R1_EN, CAMCTL_R1B_GGM_R1_EN, U)\
    /* G2C_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN2, CAMCTL_G2C_R1_EN, CAMCTL_R1B_G2C_R1_EN, U)\
    /* C42_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN2, CAMCTL_C42_R1_EN, CAMCTL_R1B_C42_R1_EN, U)\
    /* YNRS_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN3, CAMCTL_YNRS_R1_EN, CAMCTL_R1B_YNRS_R1_EN, U)\
    CMD(a, b, c, d, e, f, YNRS_R1B_YNRS_CON1, YNRS_SL2_LINK, YNRS_R1B_SL2_LINK, U)\
    /* SLK_R2 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN2, CAMCTL_SLK_R2_EN, CAMCTL_R1B_SLK_R2_EN, U)\
    CMD(a, b, c, d, e, f, SLK_R2B_SLK_RZ, SLK_HRZ_COMP, SLK_R2B_HRZ_COMP, U)\
    CMD(a, b, c, d, e, f, SLK_R2B_SLK_RZ, SLK_VRZ_COMP, SLK_R2B_VRZ_COMP, U)\
    CMD(a, b, c, d, e, f, SLK_R2B_SLK_XOFF, SLK_X_OFST, SLK_R2B_X_OFST, U)\
    CMD(a, b, c, d, e, f, SLK_R2B_SLK_SIZE, SLK_TPIPE_WD, SLK_R2B_TPIPE_WD, U)\
    CMD(a, b, c, d, e, f, SLK_R2B_SLK_SIZE, SLK_TPIPE_HT, SLK_R2B_TPIPE_HT, U)\
    /* CRP_R4 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN3, CAMCTL_CRP_R4_EN, CAMCTL_R1B_CRP_R4_EN, U)\
    CMD(a, b, c, d, e, f, CRP_R4B_CRP_X_POS, CRP_XSTART, CRP_R4B_XSTART, U)\
    CMD(a, b, c, d, e, f, CRP_R4B_CRP_X_POS, CRP_XEND, CRP_R4B_XEND, U)\
    CMD(a, b, c, d, e, f, CRP_R4B_CRP_Y_POS, CRP_YSTART, CRP_R4B_YSTART, U)\
    CMD(a, b, c, d, e, f, CRP_R4B_CRP_Y_POS, CRP_YEND, CRP_R4B_YEND, U)\
    /* CRSP_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN3, CAMCTL_CRSP_R1_EN, CAMCTL_R1B_CRSP_R1_EN, U)\
    CMD(a, b, c, d, e, f, CRSP_R1B_CRSP_OUT_IMG, CRSP_WD, CRSP_R1B_tgWidth, U)\
    CMD(a, b, c, d, e, f, CRSP_R1B_CRSP_OUT_IMG, CRSP_HT, CRSP_R1B_tgHeight, U)\
    CMD(a, b, c, d, e, f, CRSP_R1B_CRSP_CROP_X, CRSP_CROP_XSTART, CRSP_R1B_CROP_XSTART, U)\
    CMD(a, b, c, d, e, f, CRSP_R1B_CRSP_CROP_X, CRSP_CROP_XEND, CRSP_R1B_CROP_XEND, U)\
    CMD(a, b, c, d, e, f, CRSP_R1B_CRSP_CROP_Y, CRSP_CROP_YSTART, CRSP_R1B_CROP_YSTART, U)\
    CMD(a, b, c, d, e, f, CRSP_R1B_CRSP_CROP_Y, CRSP_CROP_YEND, CRSP_R1B_CROP_YEND, U)\
    /* PAK_R3 YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN3, CAMCTL_PAK_R3_EN, CAMCTL_R1B_PAK_R3_EN, U)\
    CMD(a, b, c, d, e, f, PAK_R3B_PAK_CONT, PAK_YUV_DNG, PAK_R3B_YUV_DNG, U)\
    CMD(a, b, c, d, e, f, PAK_R3B_PAK_CONT, PAK_YUV_BIT, PAK_R3B_YUV_BIT, U)\
    /* PAK_R4 YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN3, CAMCTL_PAK_R4_EN, CAMCTL_R1B_PAK_R4_EN, U)\
    CMD(a, b, c, d, e, f, PAK_R4B_PAK_CONT, PAK_YUV_DNG, PAK_R4B_YUV_DNG, U)\
    CMD(a, b, c, d, e, f, PAK_R4B_PAK_CONT, PAK_YUV_BIT, PAK_R4B_YUV_BIT, U)\
    /* PAK_R5 YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN3, CAMCTL_PAK_R5_EN, CAMCTL_R1B_PAK_R5_EN, U)\
    CMD(a, b, c, d, e, f, PAK_R5B_PAK_CONT, PAK_YUV_DNG, PAK_R5B_YUV_DNG, U)\
    CMD(a, b, c, d, e, f, PAK_R5B_PAK_CONT, PAK_YUV_BIT, PAK_R5B_YUV_BIT, U)\
    /* PLNW_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN3, CAMCTL_PLNW_R1_EN, CAMCTL_R1B_PLNW_R1_EN, U)\
    /* YUVO_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_FMT2_SEL, CAMCTL_YUVO_R1_FMT, CAMCTL_R1B_YUVO_R1_FMT, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_DMA2_EN, CAMCTL_YUVO_R1_EN, CAMCTL_R1B_YUVO_R1_EN, U)\
    CMD(a, b, c, d, e, f, YUVO_R1B_YUVO_BASE_ADDR, YUVO_BASE_ADDR, YUVO_R1B_BASE_ADDR, U)\
    CMD(a, b, c, d, e, f, YUVO_R1B_YUVO_OFST_ADDR, YUVO_OFST_ADDR, YUVO_R1B_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, YUVO_R1B_YUVO_XSIZE, YUVO_XSIZE, YUVO_R1B_XSIZE, U)\
    CMD(a, b, c, d, e, f, YUVO_R1B_YUVO_YSIZE, YUVO_YSIZE, YUVO_R1B_YSIZE, U)\
    CMD(a, b, c, d, e, f, YUVO_R1B_YUVO_STRIDE, YUVO_STRIDE, YUVO_R1B_STRIDE, U)\
    /* YUVBO_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_DMA_EN, CAMCTL_YUVBO_R1_EN, CAMCTL_R1B_YUVBO_R1_EN, U)\
    CMD(a, b, c, d, e, f, YUVBO_R1B_YUVBO_BASE_ADDR, YUVBO_BASE_ADDR, YUVBO_R1B_BASE_ADDR, U)\
    CMD(a, b, c, d, e, f, YUVBO_R1B_YUVBO_OFST_ADDR, YUVBO_OFST_ADDR, YUVBO_R1B_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, YUVBO_R1B_YUVBO_XSIZE, YUVBO_XSIZE, YUVBO_R1B_XSIZE, U)\
    CMD(a, b, c, d, e, f, YUVBO_R1B_YUVBO_YSIZE, YUVBO_YSIZE, YUVBO_R1B_YSIZE, U)\
    CMD(a, b, c, d, e, f, YUVBO_R1B_YUVBO_STRIDE, YUVBO_STRIDE, YUVBO_R1B_STRIDE, U)\
    /* YUVCO_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_DMA_EN, CAMCTL_YUVCO_R1_EN, CAMCTL_R1B_YUVCO_R1_EN, U)\
    CMD(a, b, c, d, e, f, YUVCO_R1B_YUVCO_BASE_ADDR, YUVCO_BASE_ADDR, YUVCO_R1B_BASE_ADDR, U)\
    CMD(a, b, c, d, e, f, YUVCO_R1B_YUVCO_OFST_ADDR, YUVCO_OFST_ADDR, YUVCO_R1B_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, YUVCO_R1B_YUVCO_XSIZE, YUVCO_XSIZE, YUVCO_R1B_XSIZE, U)\
    CMD(a, b, c, d, e, f, YUVCO_R1B_YUVCO_YSIZE, YUVCO_YSIZE, YUVCO_R1B_YSIZE, U)\
    CMD(a, b, c, d, e, f, YUVCO_R1B_YUVCO_STRIDE, YUVCO_STRIDE, YUVCO_R1B_STRIDE, U)\
    /* BS_R3 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN, CAMCTL_BS_R3_EN, CAMCTL_R1B_BS_R3_EN, U)\
    /* RSS_R2 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN3, CAMCTL_RSS_R2_EN, CAMCTL_R1B_RSS_R2_EN, U)\
    CMD(a, b, c, d, e, f, RSS_R2B_RSS_IN_IMG, RSS_IN_WD, RDSS_R2B_Input_Image_W, U)\
    CMD(a, b, c, d, e, f, RSS_R2B_RSS_IN_IMG, RSS_IN_HT, RDSS_R2B_Input_Image_H, U)\
    CMD(a, b, c, d, e, f, RSS_R2B_RSS_OUT_IMG, RSS_OUT_WD, RDSS_R2B_Output_Image_W, U)\
    CMD(a, b, c, d, e, f, RSS_R2B_RSS_OUT_IMG, RSS_OUT_HT, RDSS_R2B_Output_Image_H, U)\
    CMD(a, b, c, d, e, f, RSS_R2B_RSS_CONTROL, RSS_HORI_EN, RDSS_R2B_Horizontal_EN, U)\
    CMD(a, b, c, d, e, f, RSS_R2B_RSS_CONTROL, RSS_VERT_EN, RDSS_R2B_Vertical_EN, U)\
    CMD(a, b, c, d, e, f, RSS_R2B_RSS_HORI_STEP, RSS_HORI_STEP, RDSS_R2B_Horizontal_Coeff_Step, U)\
    CMD(a, b, c, d, e, f, RSS_R2B_RSS_VERT_STEP, RSS_VERT_STEP, RDSS_R2B_Vertical_Coeff_Step, U)\
    CMD(a, b, c, d, e, f, RSS_R2B_RSS_LUMA_HORI_INT_OFST, RSS_LUMA_HORI_INT_OFST, RDSS_R2B_Luma_Horizontal_Integer_Offset, U)\
    CMD(a, b, c, d, e, f, RSS_R2B_RSS_LUMA_HORI_SUB_OFST, RSS_LUMA_HORI_SUB_OFST, RDSS_R2B_Luma_Horizontal_Subpixel_Offset, U)\
    /* RSSO_R2 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_DMA2_EN, CAMCTL_RSSO_R2_EN, CAMCTL_R1B_RSSO_R2_EN, U)\
    CMD(a, b, c, d, e, f, RSSO_R2B_RSSO_BASE_ADDR, RSSO_BASE_ADDR, RSSO_R2B_BASE_ADDR, U)\
    CMD(a, b, c, d, e, f, RSSO_R2B_RSSO_OFST_ADDR, RSSO_OFST_ADDR, RSSO_R2B_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, RSSO_R2B_RSSO_XSIZE, RSSO_XSIZE, RSSO_R2B_XSIZE, U)\
    CMD(a, b, c, d, e, f, RSSO_R2B_RSSO_YSIZE, RSSO_YSIZE, RSSO_R2B_YSIZE, U)\
    CMD(a, b, c, d, e, f, RSSO_R2B_RSSO_STRIDE, RSSO_STRIDE, RSSO_R2B_STRIDE, U)\
    /* GGM_R2 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN3, CAMCTL_GGM_R2_EN, CAMCTL_R1B_GGM_R2_EN, U)\
    /* G2C_R2 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN3, CAMCTL_G2C_R2_EN, CAMCTL_R1B_G2C_R2_EN, U)\
    /* C42_R2 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN3, CAMCTL_C42_R2_EN, CAMCTL_R1B_C42_R2_EN, U)\
    /* BS_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN, CAMCTL_BS_R1_EN, CAMCTL_R1B_BS_R1_EN, U)\
    /* CRZ_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN3, CAMCTL_CRZ_R1_EN, CAMCTL_R1B_CRZ_R1_EN, U)\
    CMD(a, b, c, d, e, f, CRZ_R1B_CRZ_IN_IMG, CRZ_IN_WD, CRZ_R1B_IN_WD, U)\
    CMD(a, b, c, d, e, f, CRZ_R1B_CRZ_IN_IMG, CRZ_IN_HT, CRZ_R1B_IN_HT, U)\
    CMD(a, b, c, d, e, f, CRZ_R1B_CRZ_OUT_IMG, CRZ_OUT_WD, CRZ_R1B_OUT_WD, U)\
    CMD(a, b, c, d, e, f, CRZ_R1B_CRZ_OUT_IMG, CRZ_OUT_HT, CRZ_R1B_OUT_HT, U)\
    CMD(a, b, c, d, e, f, CRZ_R1B_CRZ_CONTROL, CRZ_HORI_EN, CRZ_R1B_HORI_EN, U)\
    CMD(a, b, c, d, e, f, CRZ_R1B_CRZ_CONTROL, CRZ_VERT_EN, CRZ_R1B_VERT_EN, U)\
    CMD(a, b, c, d, e, f, CRZ_R1B_CRZ_CONTROL, CRZ_HORI_ALGO, CRZ_R1B_HORI_ALGO, U)\
    CMD(a, b, c, d, e, f, CRZ_R1B_CRZ_CONTROL, CRZ_VERT_ALGO, CRZ_R1B_VERT_ALGO, U)\
    CMD(a, b, c, d, e, f, CRZ_R1B_CRZ_HORI_STEP, CRZ_HORI_STEP, CRZ_R1B_HORI_STEP, U)\
    CMD(a, b, c, d, e, f, CRZ_R1B_CRZ_VERT_STEP, CRZ_VERT_STEP, CRZ_R1B_VERT_STEP, U)\
    CMD(a, b, c, d, e, f, CRZ_R1B_CRZ_LUMA_HORI_INT_OFST, CRZ_LUMA_HORI_INT_OFST, CRZ_R1B_LUMA_HORI_INT_OFST, U)\
    CMD(a, b, c, d, e, f, CRZ_R1B_CRZ_LUMA_HORI_SUB_OFST, CRZ_LUMA_HORI_SUB_OFST, CRZ_R1B_LUMA_HORI_SUB_OFST, U)\
    CMD(a, b, c, d, e, f, CRZ_R1B_CRZ_CHRO_HORI_INT_OFST, CRZ_CHRO_HORI_INT_OFST, CRZ_R1B_CHRO_HORI_INT_OFST, U)\
    CMD(a, b, c, d, e, f, CRZ_R1B_CRZ_CHRO_HORI_SUB_OFST, CRZ_CHRO_HORI_SUB_OFST, CRZ_R1B_CHRO_HORI_SUB_OFST, U)\
    /* PLNW_R2 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN3, CAMCTL_PLNW_R2_EN, CAMCTL_R1B_PLNW_R2_EN, U)\
    /* CRZO_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_FMT_SEL, CAMCTL_CRZO_R1_FMT, CAMCTL_R1B_CRZO_R1_FMT, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_DMA_EN, CAMCTL_CRZO_R1_EN, CAMCTL_R1B_CRZO_R1_EN, U)\
    CMD(a, b, c, d, e, f, CRZO_R1B_CRZO_BASE_ADDR, CRZO_BASE_ADDR, CRZO_R1B_BASE_ADDR, U)\
    CMD(a, b, c, d, e, f, CRZO_R1B_CRZO_OFST_ADDR, CRZO_OFST_ADDR, CRZO_R1B_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, CRZO_R1B_CRZO_XSIZE, CRZO_XSIZE, CRZO_R1B_XSIZE, U)\
    CMD(a, b, c, d, e, f, CRZO_R1B_CRZO_YSIZE, CRZO_YSIZE, CRZO_R1B_YSIZE, U)\
    CMD(a, b, c, d, e, f, CRZO_R1B_CRZO_STRIDE, CRZO_STRIDE, CRZO_R1B_STRIDE, U)\
    /* CRZBO_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_DMA_EN, CAMCTL_CRZBO_R1_EN, CAMCTL_R1B_CRZBO_R1_EN, U)\
    CMD(a, b, c, d, e, f, CRZBO_R1B_CRZBO_BASE_ADDR, CRZBO_BASE_ADDR, CRZBO_R1B_BASE_ADDR, U)\
    CMD(a, b, c, d, e, f, CRZBO_R1B_CRZBO_OFST_ADDR, CRZBO_OFST_ADDR, CRZBO_R1B_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, CRZBO_R1B_CRZBO_XSIZE, CRZBO_XSIZE, CRZBO_R1B_XSIZE, U)\
    CMD(a, b, c, d, e, f, CRZBO_R1B_CRZBO_YSIZE, CRZBO_YSIZE, CRZBO_R1B_YSIZE, U)\
    CMD(a, b, c, d, e, f, CRZBO_R1B_CRZBO_STRIDE, CRZBO_STRIDE, CRZBO_R1B_STRIDE, U)\
    /* BS_R2 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN, CAMCTL_BS_R2_EN, CAMCTL_R1B_BS_R2_EN, U)\
    /* CRZ_R2 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN3, CAMCTL_CRZ_R2_EN, CAMCTL_R1B_CRZ_R2_EN, U)\
    CMD(a, b, c, d, e, f, CRZ_R2B_CRZ_IN_IMG, CRZ_IN_WD, CRZ_R2B_IN_WD, U)\
    CMD(a, b, c, d, e, f, CRZ_R2B_CRZ_IN_IMG, CRZ_IN_HT, CRZ_R2B_IN_HT, U)\
    CMD(a, b, c, d, e, f, CRZ_R2B_CRZ_OUT_IMG, CRZ_OUT_WD, CRZ_R2B_OUT_WD, U)\
    CMD(a, b, c, d, e, f, CRZ_R2B_CRZ_OUT_IMG, CRZ_OUT_HT, CRZ_R2B_OUT_HT, U)\
    CMD(a, b, c, d, e, f, CRZ_R2B_CRZ_CONTROL, CRZ_HORI_EN, CRZ_R2B_HORI_EN, U)\
    CMD(a, b, c, d, e, f, CRZ_R2B_CRZ_CONTROL, CRZ_VERT_EN, CRZ_R2B_VERT_EN, U)\
    CMD(a, b, c, d, e, f, CRZ_R2B_CRZ_CONTROL, CRZ_HORI_ALGO, CRZ_R2B_HORI_ALGO, U)\
    CMD(a, b, c, d, e, f, CRZ_R2B_CRZ_CONTROL, CRZ_VERT_ALGO, CRZ_R2B_VERT_ALGO, U)\
    CMD(a, b, c, d, e, f, CRZ_R2B_CRZ_HORI_STEP, CRZ_HORI_STEP, CRZ_R2B_HORI_STEP, U)\
    CMD(a, b, c, d, e, f, CRZ_R2B_CRZ_VERT_STEP, CRZ_VERT_STEP, CRZ_R2B_VERT_STEP, U)\
    CMD(a, b, c, d, e, f, CRZ_R2B_CRZ_LUMA_HORI_INT_OFST, CRZ_LUMA_HORI_INT_OFST, CRZ_R2B_LUMA_HORI_INT_OFST, U)\
    CMD(a, b, c, d, e, f, CRZ_R2B_CRZ_LUMA_HORI_SUB_OFST, CRZ_LUMA_HORI_SUB_OFST, CRZ_R2B_LUMA_HORI_SUB_OFST, U)\
    CMD(a, b, c, d, e, f, CRZ_R2B_CRZ_CHRO_HORI_INT_OFST, CRZ_CHRO_HORI_INT_OFST, CRZ_R2B_CHRO_HORI_INT_OFST, U)\
    CMD(a, b, c, d, e, f, CRZ_R2B_CRZ_CHRO_HORI_SUB_OFST, CRZ_CHRO_HORI_SUB_OFST, CRZ_R2B_CHRO_HORI_SUB_OFST, U)\
    /* CRSP_R2 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN3, CAMCTL_CRSP_R2_EN, CAMCTL_R1B_CRSP_R2_EN, U)\
    CMD(a, b, c, d, e, f, CRSP_R2B_CRSP_OUT_IMG, CRSP_WD, CRSP_R2B_tgWidth, U)\
    CMD(a, b, c, d, e, f, CRSP_R2B_CRSP_OUT_IMG, CRSP_HT, CRSP_R2B_tgHeight, U)\
    CMD(a, b, c, d, e, f, CRSP_R2B_CRSP_CROP_X, CRSP_CROP_XSTART, CRSP_R2B_CROP_XSTART, U)\
    CMD(a, b, c, d, e, f, CRSP_R2B_CRSP_CROP_X, CRSP_CROP_XEND, CRSP_R2B_CROP_XEND, U)\
    CMD(a, b, c, d, e, f, CRSP_R2B_CRSP_CROP_Y, CRSP_CROP_YSTART, CRSP_R2B_CROP_YSTART, U)\
    CMD(a, b, c, d, e, f, CRSP_R2B_CRSP_CROP_Y, CRSP_CROP_YEND, CRSP_R2B_CROP_YEND, U)\
    /* PLNW_R3 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN3, CAMCTL_PLNW_R3_EN, CAMCTL_R1B_PLNW_R3_EN, U)\
    /* CRZO_R2 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_FMT_SEL, CAMCTL_CRZO_R2_FMT, CAMCTL_R1B_CRZO_R2_FMT, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_DMA2_EN, CAMCTL_CRZO_R2_EN, CAMCTL_R1B_CRZO_R2_EN, U)\
    CMD(a, b, c, d, e, f, CRZO_R2B_CRZO_BASE_ADDR, CRZO_BASE_ADDR, CRZO_R2B_BASE_ADDR, U)\
    CMD(a, b, c, d, e, f, CRZO_R2B_CRZO_OFST_ADDR, CRZO_OFST_ADDR, CRZO_R2B_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, CRZO_R2B_CRZO_XSIZE, CRZO_XSIZE, CRZO_R2B_XSIZE, U)\
    CMD(a, b, c, d, e, f, CRZO_R2B_CRZO_YSIZE, CRZO_YSIZE, CRZO_R2B_YSIZE, U)\
    CMD(a, b, c, d, e, f, CRZO_R2B_CRZO_STRIDE, CRZO_STRIDE, CRZO_R2B_STRIDE, U)\
    /* CRZBO_R2 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_DMA2_EN, CAMCTL_CRZBO_R2_EN, CAMCTL_R1B_CRZBO_R2_EN, U)\
    CMD(a, b, c, d, e, f, CRZBO_R2B_CRZBO_BASE_ADDR, CRZBO_BASE_ADDR, CRZBO_R2B_BASE_ADDR, U)\
    CMD(a, b, c, d, e, f, CRZBO_R2B_CRZBO_OFST_ADDR, CRZBO_OFST_ADDR, CRZBO_R2B_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, CRZBO_R2B_CRZBO_XSIZE, CRZBO_XSIZE, CRZBO_R2B_XSIZE, U)\
    CMD(a, b, c, d, e, f, CRZBO_R2B_CRZBO_YSIZE, CRZBO_YSIZE, CRZBO_R2B_YSIZE, U)\
    CMD(a, b, c, d, e, f, CRZBO_R2B_CRZBO_STRIDE, CRZBO_STRIDE, CRZBO_R2B_STRIDE, U)\
    /* QBN_R4 - HLR branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN, CAMCTL_QBN_R4_EN, CAMCTL_R1B_QBN_R4_EN, U)\
    CMD(a, b, c, d, e, f, QBIN_R4B_QBIN_CTL, QBIN_ACC, QBIN_R4B_ACC, U)\
    CMD(a, b, c, d, e, f, QBIN_R4B_QBIN_CTL, QBIN_ACC_MODE, QBIN_R4B_ACC_MODE, U)\
    /* MRG_R4 - HLR branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN, CAMCTL_MRG_R4_EN, CAMCTL_R1B_MRG_R4_EN, U)\
    CMD(a, b, c, d, e, f, MRG_R4B_MRG_CTL, MRG_SIG_MODE1, MRG_R4B_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, MRG_R4B_MRG_CTL, MRG_SIG_MODE2, MRG_R4B_SIG_MODE2, U)\
    /*CMD(a, b, c, d, e, f, MRG_R4B_MRG_CTL, MRG_EDGE, MRG_R4B_EDGE, U)*/\
    CMD(a, b, c, d, e, f, MRG_R4B_MRG_CROP, MRG_STR_X, MRG_R4B_STR_X, U)\
    CMD(a, b, c, d, e, f, MRG_R4B_MRG_CROP, MRG_END_X, MRG_R4B_END_X, U)\
    /*CMD(a, b, c, d, e, f, MRG_R4B_MRG_VSIZE, MRG_HT, MRG_R4B_HT, U)*/\
    /* LTMS_SEL - HLR branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_SEL, CAMCTL_LTMS_SEL, CAMCTL_R1B_LTMS_SEL, U)\
    /* QBN_R1 - LSC branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN2, CAMCTL_QBN_R1_EN, CAMCTL_R1B_QBN_R1_EN, U)\
    CMD(a, b, c, d, e, f, QBIN_R1B_QBIN_CTL, QBIN_ACC, QBIN_R1B_ACC, U)\
    CMD(a, b, c, d, e, f, QBIN_R1B_QBIN_CTL, QBIN_ACC_MODE, QBIN_R1B_ACC_MODE, U)\
    /* MRG_R1 - LSC branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN, CAMCTL_MRG_R1_EN, CAMCTL_R1B_MRG_R1_EN, U)\
    CMD(a, b, c, d, e, f, MRG_R1B_MRG_CTL, MRG_SIG_MODE1, MRG_R1B_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, MRG_R1B_MRG_CTL, MRG_SIG_MODE2, MRG_R1B_SIG_MODE2, U)\
    /*CMD(a, b, c, d, e, f, MRG_R1B_MRG_CTL, MRG_EDGE, MRG_R1B_EDGE, U)*/\
    CMD(a, b, c, d, e, f, MRG_R1B_MRG_CROP, MRG_STR_X, MRG_R1B_STR_X, U)\
    CMD(a, b, c, d, e, f, MRG_R1B_MRG_CROP, MRG_END_X, MRG_R1B_END_X, U)\
    /*CMD(a, b, c, d, e, f, MRG_R1B_MRG_VSIZE, MRG_HT, MRG_R1B_HT, U)*/\
    /* AA_SEL - LSC branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_SEL, CAMCTL_AA_SEL, CAMCTL_R1B_AA_SEL, U)\
    /* AAO_R1 - LSC branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_DMA_EN, CAMCTL_AAO_R1_EN, CAMCTL_R1B_AAO_R1_EN, U)\
    /* CRP_R1_SEL - AFO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_SEL, CAMCTL_CRP_R1_SEL, CAMCTL_R1B_CRP_R1_SEL, U)\
    /* CRP_R1 - AFO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN2, CAMCTL_CRP_R1_EN, CAMCTL_R1B_CRP_R1_EN, U)\
    CMD(a, b, c, d, e, f, CRP_R1B_CRP_X_POS, CRP_XSTART, CRP_R1B_XSTART, U)\
    CMD(a, b, c, d, e, f, CRP_R1B_CRP_X_POS, CRP_XEND, CRP_R1B_XEND, U)\
    CMD(a, b, c, d, e, f, CRP_R1B_CRP_Y_POS, CRP_YSTART, CRP_R1B_YSTART, U)\
    CMD(a, b, c, d, e, f, CRP_R1B_CRP_Y_POS, CRP_YEND, CRP_R1B_YEND, U)\
    /* AF_R1 - AFO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN2, CAMCTL_AF_R1_EN, CAMCTL_R1B_AF_R1_EN, U)\
    CMD(a, b, c, d, e, f, AF_R1B_AF_BLK_0, AF_BLK_XSIZE, AF_R1B_BLK_XSIZE, U)\
    CMD(a, b, c, d, e, f, AF_R1B_AF_BLK_0, AF_BLK_YSIZE, AF_R1B_BLK_YSIZE, U)\
    CMD(a, b, c, d, e, f, AF_R1B_AF_BLK_1, AF_BLK_XNUM, AF_R1B_BLK_XNUM, U)\
    CMD(a, b, c, d, e, f, AF_R1B_AF_BLK_PROT, AF_PROT_BLK_XSIZE, AF_R1B_PROT_BLK_XSIZE, U)\
    CMD(a, b, c, d, e, f, AF_R1B_AF_BLK_PROT, AF_PROT_BLK_YSIZE, AF_R1B_PROT_BLK_YSIZE, U)\
    CMD(a, b, c, d, e, f, AF_R1B_AF_VLD, AF_VLD_XSTART, AF_R1B_VLD_XSTART, U)\
    CMD(a, b, c, d, e, f, AF_R1B_AF_BLK_1, AF_BLK_YNUM, AF_R1B_BLK_YNUM, U)\
    CMD(a, b, c, d, e, f, AF_R1B_AF_VLD, AF_VLD_YSTART, AF_R1B_VLD_YSTART, U)\
    CMD(a, b, c, d, e, f, AF_R1B_AF_CON2, AF_DS_EN, AF_R1B_DS_EN, U)\
    CMD(a, b, c, d, e, f, AF_R1B_AF_CON, AF_H_GONLY, AF_R1B_H_GONLY, U)\
    /* AF_SEL - AFO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_SEL, CAMCTL_AF_SEL, CAMCTL_R1B_AF_SEL, U)\
    /* AFO_R1 - AFO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_DMA_EN, CAMCTL_AFO_R1_EN, CAMCTL_R1B_AFO_R1_EN, U)\
    CMD(a, b, c, d, e, f, AFO_R1B_AFO_BASE_ADDR, AFO_BASE_ADDR, AFO_R1B_BASE_ADDR, U)\
    CMD(a, b, c, d, e, f, AFO_R1B_AFO_OFST_ADDR, AFO_OFST_ADDR, AFO_R1B_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, AFO_R1B_AFO_XSIZE, AFO_XSIZE, AFO_R1B_XSIZE, U)\
    CMD(a, b, c, d, e, f, AFO_R1B_AFO_YSIZE, AFO_YSIZE, AFO_R1B_YSIZE, U)\
    CMD(a, b, c, d, e, f, AFO_R1B_AFO_STRIDE, AFO_STRIDE, AFO_R1B_STRIDE, U)\
    /* QBN_R2 - TSFSO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN2, CAMCTL_QBN_R2_EN, CAMCTL_R1B_QBN_R2_EN, U)\
    CMD(a, b, c, d, e, f, QBIN_R2B_QBIN_CTL, QBIN_ACC, QBIN_R2B_ACC, U)\
    CMD(a, b, c, d, e, f, QBIN_R2B_QBIN_CTL, QBIN_ACC_MODE, QBIN_R2B_ACC_MODE, U)\
    /* MRG_R2 - TSFSO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_DMA_EN, CAMCTL_TSFSO_R1_EN, CAMCTL_R1B_TSFSO_R1_EN, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN2, CAMCTL_MRG_R2_EN, CAMCTL_R1B_MRG_R2_EN, U)\
    CMD(a, b, c, d, e, f, MRG_R2B_MRG_CTL, MRG_SIG_MODE1, MRG_R2B_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, MRG_R2B_MRG_CTL, MRG_SIG_MODE2, MRG_R2B_SIG_MODE2, U)\
    /*CMD(a, b, c, d, e, f, MRG_R2B_MRG_CTL, MRG_EDGE, MRG_R2B_EDGE, U)*/\
    CMD(a, b, c, d, e, f, MRG_R2B_MRG_CROP, MRG_STR_X, MRG_R2B_STR_X, U)\
    CMD(a, b, c, d, e, f, MRG_R2B_MRG_CROP, MRG_END_X, MRG_R2B_END_X, U)\
    /*CMD(a, b, c, d, e, f, MRG_R2B_MRG_VSIZE, MRG_HT, MRG_R2B_HT, U)*/\
    /* CRP_R3_SEL - IMGO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_SEL, CAMCTL_CRP_R3_SEL, CAMCTL_R1B_CRP_R3_SEL, U)\
    /* CRP_R3 - IMGO branch*/\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN, CAMCTL_CRP_R3_EN, CAMCTL_R1B_CRP_R3_EN, U)\
    CMD(a, b, c, d, e, f, CRP_R3B_CRP_X_POS, CRP_XSTART, CRP_R3B_XSTART, U)\
    CMD(a, b, c, d, e, f, CRP_R3B_CRP_X_POS, CRP_XEND, CRP_R3B_XEND, U)\
    CMD(a, b, c, d, e, f, CRP_R3B_CRP_Y_POS, CRP_YSTART, CRP_R3B_YSTART, U)\
    CMD(a, b, c, d, e, f, CRP_R3B_CRP_Y_POS, CRP_YEND, CRP_R3B_YEND, U)\
    /* IMG_SEL - IMGO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_SEL, CAMCTL_IMG_SEL, CAMCTL_R1B_IMG_SEL, U)\
    /* MRG_R3 - IMGO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN4, CAMCTL_MRG_R3_EN, CAMCTL_R1B_MRG_R3_EN, U)\
    CMD(a, b, c, d, e, f, MRG_R3B_MRG_CTL, MRG_SIG_MODE1, MRG_R3B_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, MRG_R3B_MRG_CTL, MRG_SIG_MODE2, MRG_R3B_SIG_MODE2, U)\
    /*CMD(a, b, c, d, e, f, MRG_R3B_MRG_CTL, MRG_EDGE, MRG_R3B_EDGE, U)*/\
    CMD(a, b, c, d, e, f, MRG_R3B_MRG_CROP, MRG_STR_X, MRG_R3B_STR_X, U)\
    CMD(a, b, c, d, e, f, MRG_R3B_MRG_CROP, MRG_END_X, MRG_R3B_END_X, U)\
    /*CMD(a, b, c, d, e, f, MRG_R3B_MRG_VSIZE, MRG_HT, MRG_R3B_HT, U)*/\
    /* MRG_R12 IMGO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN3, CAMCTL_MRG_R12_EN, CAMCTL_R1B_MRG_R12_EN, U)\
    CMD(a, b, c, d, e, f, MRG_R12B_MRG_CTL, MRG_SIG_MODE1, MRG_R12B_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, MRG_R12B_MRG_CTL, MRG_SIG_MODE2, MRG_R12B_SIG_MODE2, U)\
    /*CMD(a, b, c, d, e, f, MRG_R12B_MRG_CTL, MRG_EDGE, MRG_R12B_EDGE, U)*/\
    CMD(a, b, c, d, e, f, MRG_R12B_MRG_CROP, MRG_STR_X, MRG_R12B_STR_X, U)\
    CMD(a, b, c, d, e, f, MRG_R12B_MRG_CROP, MRG_END_X, MRG_R12B_END_X, U)\
    /*CMD(a, b, c, d, e, f, MRG_R12B_MRG_VSIZE, MRG_HT, MRG_R12B_HT, U)*/\
    /* UFE_R1 - IMGO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN3, CAMCTL_UFE_R1_EN, CAMCTL_R1B_UFE_R1_EN, U)\
    CMD(a, b, c, d, e, f, UFE_R1B_UFE_CON, UFE_FORCE_PCM, UFE_R1B_FORCE_PCM, U)\
    /* PAK_R1 - IMGO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN, CAMCTL_PAK_R1_EN, CAMCTL_R1B_PAK_R1_EN, U)\
    /* IMGO_SEL - IMGO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_SEL, CAMCTL_IMGO_SEL, CAMCTL_R1B_IMGO_SEL, U)\
    /* IMGO_R1 - IMGO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_DMA_EN, CAMCTL_IMGO_R1_EN, CAMCTL_R1B_IMGO_R1_EN, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_FMT_SEL, CAMCTL_IMGO_R1_FMT, CAMCTL_R1B_IMGO_R1_FMT, U)\
    CMD(a, b, c, d, e, f, IMGO_R1B_IMGO_BASE_ADDR, IMGO_BASE_ADDR, IMGO_R1B_BASE_ADDR, U)\
    CMD(a, b, c, d, e, f, IMGO_R1B_IMGO_OFST_ADDR, IMGO_OFST_ADDR, IMGO_R1B_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, IMGO_R1B_IMGO_XSIZE, IMGO_XSIZE, IMGO_R1B_XSIZE, U)\
    CMD(a, b, c, d, e, f, IMGO_R1B_IMGO_YSIZE, IMGO_YSIZE, IMGO_R1B_YSIZE, U)\
    CMD(a, b, c, d, e, f, IMGO_R1B_IMGO_STRIDE, IMGO_STRIDE, IMGO_R1B_STRIDE, U)\
    CMD(a, b, c, d, e, f, IMGO_R1B_IMGO_CROP, IMGO_XOFFSET, IMGO_R1B_XOFFSET, U)\
    CMD(a, b, c, d, e, f, IMGO_R1B_IMGO_CROP, IMGO_YOFFSET, IMGO_R1B_YOFFSET, U)\
    /* UFEO_R1 - IMGO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_DMA_EN, CAMCTL_UFEO_R1_EN, CAMCTL_R1B_UFEO_R1_EN, U)\
    CMD(a, b, c, d, e, f, UFEO_R1B_UFEO_BASE_ADDR, UFEO_BASE_ADDR, UFEO_R1B_BASE_ADDR, U)\
    CMD(a, b, c, d, e, f, UFEO_R1B_UFEO_OFST_ADDR, UFEO_OFST_ADDR, UFEO_R1B_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, UFEO_R1B_UFEO_XSIZE, UFEO_XSIZE, UFEO_R1B_XSIZE, U)\
    CMD(a, b, c, d, e, f, UFEO_R1B_UFEO_YSIZE, UFEO_YSIZE, UFEO_R1B_YSIZE, U)\
    CMD(a, b, c, d, e, f, UFEO_R1B_UFEO_STRIDE, UFEO_STRIDE, UFEO_R1B_STRIDE, U)\
    /* FLK_SEL - FLKO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_SEL, CAMCTL_FLK_SEL, CAMCTL_R1B_FLK_SEL, U)\
    /*QBN_R3 - FLKO branch*/\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN3, CAMCTL_QBN_R3_EN, CAMCTL_R1B_QBN_R3_EN, U)\
    CMD(a, b, c, d, e, f, QBIN_R3B_QBIN_CTL, QBIN_ACC, QBIN_R3B_ACC, U)\
    CMD(a, b, c, d, e, f, QBIN_R3B_QBIN_CTL, QBIN_ACC_MODE, QBIN_R3B_ACC_MODE, U)\
    /* MRG_R10 - FLKO branch */\
    /*CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN, CAMCTL_MRG_R10_EN, CAMCTL_R1B_MRG_R10_EN, U)*/\
    CMD(a, b, c, d, e, f, MRG_R10B_MRG_CTL, MRG_SIG_MODE1, MRG_R10B_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, MRG_R10B_MRG_CTL, MRG_SIG_MODE2, MRG_R10B_SIG_MODE2, U)\
    /*CMD(a, b, c, d, e, f, MRG_R10B_MRG_CTL, MRG_EDGE, MRG_R10B_EDGE, U)*/\
    CMD(a, b, c, d, e, f, MRG_R10B_MRG_CROP, MRG_STR_X, MRG_R10B_STR_X, U)\
    CMD(a, b, c, d, e, f, MRG_R10B_MRG_CROP, MRG_END_X, MRG_R10B_END_X, U)\
    /*CMD(a, b, c, d, e, f, MRG_R10B_MRG_VSIZE, MRG_HT, MRG_R10B_HT, U)*/\
    /* MRG_R6 - ADL branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1B_CAMCTL_EN3, CAMCTL_MRG_R6_EN, CAMCTL_R1B_MRG_R6_EN, U)\
    CMD(a, b, c, d, e, f, MRG_R6B_MRG_CTL, MRG_SIG_MODE1, MRG_R6B_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, MRG_R6B_MRG_CTL, MRG_SIG_MODE2, MRG_R6B_SIG_MODE2, U)\
    /*CMD(a, b, c, d, e, f, MRG_R6B_MRG_CTL, MRG_EDGE, MRG_R6B_EDGE, U)*/\
    CMD(a, b, c, d, e, f, MRG_R6B_MRG_CROP, MRG_STR_X, MRG_R6B_STR_X, U)\
    CMD(a, b, c, d, e, f, MRG_R6B_MRG_CROP, MRG_END_X, MRG_R6B_END_X, U)\
    /*CMD(a, b, c, d, e, f, MRG_R6B_MRG_VSIZE, MRG_HT, MRG_R6B_HT, U)*/\

/* PLATFORM parameters */
/* a, b, c, d, e, f reserved */
/* struct name */
/* variable name */
/* R/W type: R, RW, W */
#define DUAL_HW_C_LIST(CMD, a, b, c, d, e, f) \
    /* TG_R1 */\
    CMD(a, b, c, d, e, f, TG_R1C_TG_SEN_GRAB_PXL, TG_PXL_S, TG_R1C_PXL_S, U)\
    CMD(a, b, c, d, e, f, TG_R1C_TG_SEN_GRAB_PXL, TG_PXL_E, TG_R1C_PXL_E, U)\
    CMD(a, b, c, d, e, f, TG_R1C_TG_SEN_GRAB_LIN, TG_LIN_S, TG_R1C_LIN_S, U)\
    CMD(a, b, c, d, e, f, TG_R1C_TG_SEN_GRAB_LIN, TG_LIN_E, TG_R1C_LIN_E, U)\
    /* RAWI_R2 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_DMA_EN, CAMCTL_RAWI_R2_EN, CAMCTL_R1C_RAWI_R2_EN, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_FMT3_SEL, CAMCTL_RAWI_R2_FMT, CAMCTL_R1C_RAWI_R2_FMT, U)\
    CMD(a, b, c, d, e, f, RAWI_R2C_RAWI_OFST_ADDR, RAWI_OFST_ADDR, RAWI_R2C_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, RAWI_R2C_RAWI_XSIZE, RAWI_XSIZE, RAWI_R2C_XSIZE, U)\
    CMD(a, b, c, d, e, f, RAWI_R2C_RAWI_YSIZE, RAWI_YSIZE, RAWI_R2C_YSIZE, U)\
    CMD(a, b, c, d, e, f, RAWI_R2C_RAWI_STRIDE, RAWI_STRIDE, RAWI_R2C_STRIDE, U)\
    /* UNP_R2 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN, CAMCTL_UNP_R2_EN, CAMCTL_R1C_UNP_R2_EN, U)\
    CMD(a, b, c, d, e, f, UNP_R2C_UNP_OFST, UNP_OFST_STB, UNP_R2C_OFST_STB, U)\
    CMD(a, b, c, d, e, f, UNP_R2C_UNP_OFST, UNP_OFST_EDB, UNP_R2C_OFST_EDB, U)\
    /* UFDI_R2 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_DMA_EN, CAMCTL_UFDI_R2_EN, CAMCTL_R1C_UFDI_R2_EN, U)\
    CMD(a, b, c, d, e, f, UFDI_R2C_UFDI_XSIZE, UFDI_XSIZE, UFDI_R2C_XSIZE, U)\
    CMD(a, b, c, d, e, f, UFDI_R2C_UFDI_YSIZE, UFDI_YSIZE, UFDI_R2C_YSIZE, U)\
    CMD(a, b, c, d, e, f, UFDI_R2C_UFDI_STRIDE, UFDI_STRIDE, UFDI_R2C_STRIDE, U)\
    /* UFD_R2 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN3, CAMCTL_UFD_R2_EN, CAMCTL_R1C_UFD_R2_EN, U)\
    CMD(a, b, c, d, e, f, UFD_R2C_UFD_AU_CON, UFD_AU_OFST, UFD_R2C_AU_OFST, U)\
    CMD(a, b, c, d, e, f, UFD_R2C_UFD_AU_CON, UFD_AU_SIZE, UFD_R2C_AU_SIZE, U)\
    CMD(a, b, c, d, e, f, UFD_R2C_UFD_AU2_CON, UFD_AU2_OFST, UFD_R2C_AU2_OFST, U)\
    CMD(a, b, c, d, e, f, UFD_R2C_UFD_AU2_CON, UFD_AU2_SIZE, UFD_R2C_AU2_SIZE, U)\
    CMD(a, b, c, d, e, f, UFD_R2C_UFD_BS2_AU_CON, UFD_BS2_AU_START, UFD_R2C_BS2_AU_START, U)\
    CMD(a, b, c, d, e, f, UFD_R2C_UFD_BS2_AU_CON, UFD_BOND_MODE, UFD_R2C_BOND_MODE, U)\
    CMD(a, b, c, d, e, f, UFD_R2C_UFD_AU3_CON, UFD_AU3_OFST, UFD_R2C_AU3_OFST, U)\
    CMD(a, b, c, d, e, f, UFD_R2C_UFD_AU3_CON, UFD_AU3_SIZE, UFD_R2C_AU3_SIZE, U)\
    CMD(a, b, c, d, e, f, UFD_R2C_UFD_BS3_AU_CON, UFD_BS3_AU_START, UFD_R2C_BS3_AU_START, U)\
    CMD(a, b, c, d, e, f, UFD_R2C_UFD_BS3_AU_CON, UFD_BOND2_MODE, UFD_R2C_BOND2_MODE, U)\
    CMD(a, b, c, d, e, f, UFD_R2C_UFD_CROP_CON1, UFD_X_START, UFD_R2C_X_START, U)\
    CMD(a, b, c, d, e, f, UFD_R2C_UFD_CROP_CON1, UFD_X_END, UFD_R2C_X_END, U)\
    CMD(a, b, c, d, e, f, UFD_R2C_UFD_CROP_CON2, UFD_Y_START, UFD_R2C_Y_START, U)\
    CMD(a, b, c, d, e, f, UFD_R2C_UFD_CROP_CON2, UFD_Y_END, UFD_R2C_Y_END, U)\
    CMD(a, b, c, d, e, f, UFD_R2C_UFD_SIZE_CON, UFD_WD, UFD_R2C_WD, U)\
    CMD(a, b, c, d, e, f, UFD_R2C_UFD_SIZE_CON, UFD_HT, UFD_R2C_HT, U)\
    CMD(a, b, c, d, e, f, UFD_R2C_UFD_CON, UFD_SEL, UFD_R2C_SEL, U)\
    CMD(a, b, c, d, e, f, UFD_R2C_UFD_ADDRESS_CON1, UFD_BITSTREAM2_OFST_ADDR, UFD_R2C_BITSTREAM2_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, UFD_R2C_UFD_ADDRESS_CON2, UFD_BITSTREAM3_OFST_ADDR, UFD_R2C_BITSTREAM3_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, UFD_R2C_UFD_ADDRESS_CON3, UFD_BITSTREAM4_OFST_ADDR, UFD_R2C_BITSTREAM4_OFST_ADDR, U)\
    /* DCPN_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN4, CAMCTL_DCPN_R1_EN, CAMCTL_R1C_DCPN_R1_EN, U)\
    /* DBN_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN2, CAMCTL_DBN_R1_EN, CAMCTL_R1C_DBN_R1_EN, U)\
    /* BIN_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN2, CAMCTL_BIN_R1_EN, CAMCTL_R1C_BIN_R1_EN, U)\
	/* SEL */\
	CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_SEL, CAMCTL_RAW_SEL, CAMCTL_R1C_RAW_SEL, U)\
	CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_SEL2, CAMCTL_RAWI_UFO_SEL, CAMCTL_R1C_RAWI_UFO_SEL, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_SEL, CAMCTL_SEP_SEL, CAMCTL_R1C_SEP_SEL, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_SEL, CAMCTL_BPC_R2_SEL, CAMCTL_R1C_BPC_R2_SEL, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_SEL2, CAMCTL_BPC_R3_SEL, CAMCTL_R1C_BPC_R3_SEL, U)\
    /* PIXEL MODE */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_FMT2_SEL, CAMCTL_PIX_BUS_SEPI, CAMCTL_R1C_PIX_BUS_SEPI, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_FMT_SEL, CAMCTL_PIX_BUS_SEPO, CAMCTL_R1C_PIX_BUS_SEPO, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_FMT_SEL, CAMCTL_PIX_BUS_MRG_R3O, CAMCTL_R1C_PIX_BUS_MRG_R3O, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_FMT_SEL, CAMCTL_PIX_BUS_MRG_R7O, CAMCTL_R1C_PIX_BUS_MRG_R7O, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_FMT_SEL, CAMCTL_PIX_BUS_MRG_R8O, CAMCTL_R1C_PIX_BUS_MRG_R8O, U)\
    /* SEP_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN, CAMCTL_SEP_R1_EN, CAMCTL_R1C_SEP_R1_EN, U)\
    /*CMD(a, b, c, d, e, f, SEP_R1C_SEP_CTL, SEP_EDGE, SEP_R1C_SEP_EDGE, U)*/\
    CMD(a, b, c, d, e, f, SEP_R1C_SEP_CROP, SEP_STR_X, SEP_R1C_STR_X, U)\
    CMD(a, b, c, d, e, f, SEP_R1C_SEP_CROP, SEP_END_X, SEP_R1C_END_X, U)\
    CMD(a, b, c, d, e, f, SEP_R1C_SEP_VSIZE, SEP_HT, SEP_R1C_SEP_HT, U)\
    /* IBPC_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN4, CAMCTL_IBPC_R1_EN, CAMCTL_R1C_IBPC_R1_EN, U)\
    /* IOBC_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN4, CAMCTL_IOBC_R1_EN, CAMCTL_R1C_IOBC_R1_EN, U)\
    /* IRM_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN4, CAMCTL_IRM_R1_EN, CAMCTL_R1C_IRM_R1_EN, U)\
    /* FBND_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN2, CAMCTL_FBND_R1_EN, CAMCTL_R1C_FBND_R1_EN, U)\
    /* OBC_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN, CAMCTL_OBC_R1_EN, CAMCTL_R1C_OBC_R1_EN, U)\
    /* BPC_R1 */\
	CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN, CAMCTL_BPC_R1_EN, CAMCTL_R1C_BPC_R1_EN, U)\
	CMD(a, b, c, d, e, f, BPC_R1C_BPC_BPC_TBLI1, BPC_XOFFSET, BPC_R1C_XOFFSET, U)\
	CMD(a, b, c, d, e, f, BPC_R1C_BPC_BPC_TBLI1, BPC_YOFFSET, BPC_R1C_YOFFSET, U)\
	CMD(a, b, c, d, e, f, BPC_R1C_BPC_BPC_TBLI2, BPC_XSIZE, BPC_R1C_XSIZE, U)\
	CMD(a, b, c, d, e, f, BPC_R1C_BPC_BPC_TBLI2, BPC_YSIZE, BPC_R1C_YSIZE, U)\
	CMD(a, b, c, d, e, f, BPC_R1C_BPC_BPC_CON, BPC_LE_INV_CTL, BPC_R1C_LE_INV_CTL, U)\
	CMD(a, b, c, d, e, f, BPC_R1C_BPC_PDC_POS, BPC_PDC_XCENTER, BPC_R1C_PDC_XCENTER, U)\
	CMD(a, b, c, d, e, f, BPC_R1C_BPC_PDC_POS, BPC_PDC_XCENTER, BPC_R1C_PDC_YCENTER, U)\
	CMD(a, b, c, d, e, f, BPC_R1C_BPC_PDC_CON, BPC_PDC_EN, BPC_R1C_PDC_EN, U)\
    /* BPCI_R1 */\
    CMD(a, b, c, d, e, f, BPCI_R1C_BPCI_XSIZE, BPCI_XSIZE, BPCI_R1C_XSIZE, U)\
    CMD(a, b, c, d, e, f, BPCI_R1C_BPCI_YSIZE, BPCI_YSIZE, BPCI_R1C_YSIZE, U)\
    CMD(a, b, c, d, e, f, BPCI_R1C_BPCI_STRIDE, BPCI_STRIDE, BPCI_R1C_STRIDE, U)\
    /* OBC_R2 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN2, CAMCTL_OBC_R2_EN, CAMCTL_R1C_OBC_R2_EN, U)\
    /* BPC_R2 */\
	CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN2, CAMCTL_BPC_R2_EN, CAMCTL_R1C_BPC_R2_EN, U)\
	CMD(a, b, c, d, e, f, BPC_R2C_BPC_BPC_TBLI1, BPC_XOFFSET, BPC_R2C_XOFFSET, U)\
	CMD(a, b, c, d, e, f, BPC_R2C_BPC_BPC_TBLI1, BPC_YOFFSET, BPC_R2C_YOFFSET, U)\
	CMD(a, b, c, d, e, f, BPC_R2C_BPC_BPC_TBLI2, BPC_XSIZE, BPC_R2C_XSIZE, U)\
	CMD(a, b, c, d, e, f, BPC_R2C_BPC_BPC_TBLI2, BPC_YSIZE, BPC_R2C_YSIZE, U)\
    CMD(a, b, c, d, e, f, BPC_R2C_BPC_BPC_CON, BPC_LE_INV_CTL, BPC_R2C_LE_INV_CTL, U)\
	CMD(a, b, c, d, e, f, BPC_R2C_BPC_PDC_POS, BPC_PDC_XCENTER, BPC_R2C_PDC_XCENTER, U)\
	CMD(a, b, c, d, e, f, BPC_R2C_BPC_PDC_POS, BPC_PDC_XCENTER, BPC_R2C_PDC_YCENTER, U)\
	CMD(a, b, c, d, e, f, BPC_R2C_BPC_PDC_CON, BPC_PDC_EN, BPC_R2C_PDC_EN, U)\
    /* BPCI_R2 */\
    CMD(a, b, c, d, e, f, BPCI_R2C_BPCI_XSIZE, BPCI_XSIZE, BPCI_R2C_XSIZE, U)\
    CMD(a, b, c, d, e, f, BPCI_R2C_BPCI_YSIZE, BPCI_YSIZE, BPCI_R2C_YSIZE, U)\
    CMD(a, b, c, d, e, f, BPCI_R2C_BPCI_STRIDE, BPCI_STRIDE, BPCI_R2C_STRIDE, U)\
    /* ZFUS_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN2, CAMCTL_ZFUS_R1_EN, CAMCTL_R1C_ZFUS_R1_EN, U)\
    /* FUS_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN, CAMCTL_FUS_R1_EN, CAMCTL_R1C_FUS_R1_EN, U)\
    CMD(a, b, c, d, e, f, FUS_R1C_FUS_CFG, FUS_INPUT_MODE, FUS_R1C_INPUT_MODE, U)\
    CMD(a, b, c, d, e, f, FUS_R1C_FUS_SIZE, FUS_HSIZE, FUS_R1C_HSIZE, U)\
    CMD(a, b, c, d, e, f, FUS_R1C_FUS_SIZE, FUS_VSIZE, FUS_R1C_VSIZE, U)\
    /* RNR_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN4, CAMCTL_RNR_R1_EN, CAMCTL_R1C_RNR_R1_EN, U)\
    /* DGN_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN, CAMCTL_DGN_R1_EN, CAMCTL_R1C_DGN_R1_EN, U)\
    /* LSC_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN, CAMCTL_LSC_R1_EN, CAMCTL_R1C_LSC_R1_EN, U)\
    CMD(a, b, c, d, e, f, LSC_R1C_LSC_CTL1, LSC_EXTEND_COEF_MODE, LSC_R1C_EXTEND_COEF_MODE, U)\
    CMD(a, b, c, d, e, f, LSC_R1C_LSC_CTL2, LSC_SDBLK_XNUM, LSC_R1C_SDBLK_XNUM, U)\
    CMD(a, b, c, d, e, f, LSC_R1C_LSC_CTL2, LSC_SDBLK_WIDTH, LSC_R1C_SDBLK_WIDTH, U)\
    CMD(a, b, c, d, e, f, LSC_R1C_LSC_LBLOCK, LSC_SDBLK_lWIDTH, LSC_R1C_SDBLK_lWIDTH, U)\
    CMD(a, b, c, d, e, f, LSC_R1C_LSC_TPIPE_OFST, LSC_TPIPE_OFST_X, LSC_R1C_TPIPE_OFST_X, U)\
    CMD(a, b, c, d, e, f, LSC_R1C_LSC_TPIPE_SIZE, LSC_TPIPE_SIZE_X, LSC_R1C_TPIPE_SIZE_X, U)\
    CMD(a, b, c, d, e, f, LSC_R1C_LSC_CTL3, LSC_SDBLK_HEIGHT, LSC_R1C_SDBLK_HEIGHT, U)\
    CMD(a, b, c, d, e, f, LSC_R1C_LSC_LBLOCK, LSC_SDBLK_lHEIGHT, LSC_R1C_SDBLK_lHEIGHT, U)\
    CMD(a, b, c, d, e, f, LSC_R1C_LSC_TPIPE_OFST, LSC_TPIPE_OFST_Y, LSC_R1C_TPIPE_OFST_Y, U)\
    CMD(a, b, c, d, e, f, LSC_R1C_LSC_TPIPE_SIZE, LSC_TPIPE_SIZE_Y, LSC_R1C_TPIPE_SIZE_Y, U)\
    /* LSCI_R1 */\
    CMD(a, b, c, d, e, f, LSCI_R1C_LSCI_OFST_ADDR, LSCI_OFST_ADDR, LSCI_R1C_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, LSCI_R1C_LSCI_XSIZE, LSCI_XSIZE, LSCI_R1C_XSIZE, U)\
    CMD(a, b, c, d, e, f, LSCI_R1C_LSCI_STRIDE, LSCI_STRIDE, LSCI_R1C_STRIDE, U)\
    /* WB_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN, CAMCTL_WB_R1_EN, CAMCTL_R1C_WB_R1_EN, U)\
    /* HLR_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN2, CAMCTL_HLR_R1_EN, CAMCTL_R1C_HLR_R1_EN, U)\
    /* LTM_R1 */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN, CAMCTL_LTM_R1_EN, CAMCTL_R1C_LTM_R1_EN, U)\
    CMD(a, b, c, d, e, f, LTM_R1C_LTM_BLK_NUM, LTM_BLK_X_NUM, LTM_R1C_BLK_X_NUM, U)\
    CMD(a, b, c, d, e, f, LTM_R1C_LTM_BLK_NUM, LTM_BLK_Y_NUM, LTM_R1C_BLK_Y_NUM, U)\
    CMD(a, b, c, d, e, f, LTM_R1C_LTM_BLK_SZ, LTM_BLK_WIDTH, LTM_R1C_BLK_WIDTH, U)\
    CMD(a, b, c, d, e, f, LTM_R1C_LTM_BLK_SZ, LTM_BLK_HEIGHT, LTM_R1C_BLK_HEIGHT, U)\
    CMD(a, b, c, d, e, f, LTM_R1C_LTM_TILE_NUM, LTM_TILE_BLK_X_NUM_START, LTM_R1C_TILE_BLK_X_NUM_START, U)\
    CMD(a, b, c, d, e, f, LTM_R1C_LTM_TILE_NUM, LTM_TILE_BLK_X_NUM_END, LTM_R1C_TILE_BLK_X_NUM_END, U)\
    CMD(a, b, c, d, e, f, LTM_R1C_LTM_TILE_NUM, LTM_TILE_BLK_Y_NUM_START, LTM_R1C_TILE_BLK_Y_NUM_START, U)\
    CMD(a, b, c, d, e, f, LTM_R1C_LTM_TILE_NUM, LTM_TILE_BLK_Y_NUM_END, LTM_R1C_TILE_BLK_Y_NUM_END, U)\
    CMD(a, b, c, d, e, f, LTM_R1C_LTM_TILE_CNTX, LTM_TILE_BLK_X_CNT_START, LTM_R1C_TILE_BLK_X_CNT_START, U)\
    CMD(a, b, c, d, e, f, LTM_R1C_LTM_TILE_CNTX, LTM_TILE_BLK_X_CNT_END, LTM_R1C_TILE_BLK_X_CNT_END, U)\
    CMD(a, b, c, d, e, f, LTM_R1C_LTM_TILE_CNTY, LTM_TILE_BLK_Y_CNT_START, LTM_R1C_TILE_BLK_Y_CNT_START, U)\
    CMD(a, b, c, d, e, f, LTM_R1C_LTM_TILE_CNTY, LTM_TILE_BLK_Y_CNT_END, LTM_R1C_TILE_BLK_Y_CNT_END, U)\
    /* RRZ_R1 - RRZ branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN2, CAMCTL_RRZ_R1_EN, CAMCTL_R1C_RRZ_R1_EN, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN4, CAMCTL_RLB_R1_EN, CAMCTL_R1C_RLB_R1_EN, U)\
    CMD(a, b, c, d, e, f, RRZ_R1C_RRZ_VERT_STEP, RRZ_VERT_STEP, RRZ_tmp3_VERT_STEP, U)\
    CMD(a, b, c, d, e, f, RRZ_R1C_RRZ_VERT_INT_OFST, RRZ_VERT_INT_OFST, RRZ_tmp3_VERT_INT_OFST, U)\
    CMD(a, b, c, d, e, f, RRZ_R1C_RRZ_VERT_SUB_OFST, RRZ_VERT_SUB_OFST, RRZ_tmp3_VERT_SUB_OFST, U)\
    CMD(a, b, c, d, e, f, RRZ_R1C_RRZ_OUT_IMG, RRZ_OUT_HT, RRZ_tmp3_OUT_HT, U)\
    CMD(a, b, c, d, e, f, RRZ_R1C_RRZ_OUT_IMG, RRZ_OUT_WD, RRZ_tmp3_OUT_WD, U)\
    CMD(a, b, c, d, e, f, RRZ_R1C_RRZ_HORI_STEP, RRZ_HORI_STEP, RRZ_tmp3_HORI_STEP, U)\
    CMD(a, b, c, d, e, f, RRZ_R1C_RRZ_HORI_INT_OFST, RRZ_HORI_INT_OFST, RRZ_tmp3_HORI_INT_OFST, U)\
    CMD(a, b, c, d, e, f, RRZ_R1C_RRZ_HORI_SUB_OFST, RRZ_HORI_SUB_OFST, RRZ_tmp3_HORI_SUB_OFST, U)\
    /*CMD(a, b, c, d, e, f, RRZ_R1C_RRZ_RLB_AOFST, RRZ_RLB_AOFST, RRZ_tmp3_RLB_AOFST, U)*/\
    /* HDS_SEL - RRZ branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_SEL, CAMCTL_HDS_SEL, CAMCTL_R1C_HDS_SEL, U)\
    /* GSE - RRZ branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN2, CAMCTL_GSE_R1_EN, CAMCTL_R1C_GSE_R1_EN, U)\
    /* MRG_R8 - RRZ branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN2, CAMCTL_MRG_R8_EN, CAMCTL_R1C_MRG_R8_EN, U)\
    CMD(a, b, c, d, e, f, MRG_R8C_MRG_CTL, MRG_SIG_MODE1, MRG_R8C_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, MRG_R8C_MRG_CTL, MRG_SIG_MODE2, MRG_R8C_SIG_MODE2, U)\
    /*CMD(a, b, c, d, e, f, MRG_R8C_MRG_CTL, MRG_EDGE, MRG_R8C_EDGE, U)*/\
    CMD(a, b, c, d, e, f, MRG_R8C_MRG_CROP, MRG_STR_X, MRG_R8C_STR_X, U)\
    CMD(a, b, c, d, e, f, MRG_R8C_MRG_CROP, MRG_END_X, MRG_R8C_END_X, U)\
    /*CMD(a, b, c, d, e, f, MRG_R8C_MRG_VSIZE, MRG_HT, MRG_R8C_HT, U)*/\
    /* MRG_R7 - RRZ branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN, CAMCTL_MRG_R7_EN, CAMCTL_R1C_MRG_R7_EN, U)\
    CMD(a, b, c, d, e, f, MRG_R7C_MRG_CTL, MRG_SIG_MODE1, MRG_R7C_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, MRG_R7C_MRG_CTL, MRG_SIG_MODE2, MRG_R7C_SIG_MODE2, U)\
    /*CMD(a, b, c, d, e, f, MRG_R7C_MRG_CTL, MRG_EDGE, MRG_R7C_EDGE, U)*/\
    CMD(a, b, c, d, e, f, MRG_R7C_MRG_CROP, MRG_STR_X, MRG_R7C_STR_X, U)\
    CMD(a, b, c, d, e, f, MRG_R7C_MRG_CROP, MRG_END_X, MRG_R7C_END_X, U)\
    /*CMD(a, b, c, d, e, f, MRG_R7C_MRG_VSIZE, MRG_HT, MRG_R7C_HT, U)*/\
    /* MRG_R11 - RRZ branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN3, CAMCTL_MRG_R11_EN, CAMCTL_R1C_MRG_R11_EN, U)\
    CMD(a, b, c, d, e, f, MRG_R11C_MRG_CTL, MRG_SIG_MODE1, MRG_R11C_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, MRG_R11C_MRG_CTL, MRG_SIG_MODE2, MRG_R11C_SIG_MODE2, U)\
    /*CMD(a, b, c, d, e, f, MRG_R11C_MRG_CTL, MRG_EDGE, MRG_R11C_EDGE, U)*/\
    CMD(a, b, c, d, e, f, MRG_R11C_MRG_CROP, MRG_STR_X, MRG_R11C_STR_X, U)\
    CMD(a, b, c, d, e, f, MRG_R11C_MRG_CROP, MRG_END_X, MRG_R11C_END_X, U)\
    /*CMD(a, b, c, d, e, f, MRG_R11C_MRG_VSIZE, MRG_HT, MRG_R11C_HT, U)*/\
    /* UFEG_SEL - RRZ branch*/\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_SEL, CAMCTL_UFEG_SEL, CAMCTL_R1C_UFEG_SEL, U)\
    /* PAKG_R1 - RRZ branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN, CAMCTL_PAKG_R1_EN, CAMCTL_R1C_PAKG_R1_EN, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_FMT2_SEL, CAMCTL_PAKG_R1_FG_OUT, CAMCTL_R1C_PAKG_R1_FG_OUT, U)\
    /* UFG_R1 - RRZ branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN2, CAMCTL_UFG_R1_EN, CAMCTL_R1C_UFG_R1_EN, U)\
    CMD(a, b, c, d, e, f, UFG_R1C_UFG_CON, UFG_FORCE_PCM, UFG_R1C_FORCE_PCM, U)\
    /* RRZO_R1 - RRZ branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_FMT_SEL, CAMCTL_RRZO_R1_FMT, CAMCTL_R1C_RRZO_R1_FMT, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_FMT_SEL, CAMCTL_RRZO_FG_MODE, CAMCTL_R1C_RRZO_FG_MODE, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_DMA_EN, CAMCTL_RRZO_R1_EN, CAMCTL_R1C_RRZO_R1_EN, U)\
    CMD(a, b, c, d, e, f, RRZO_R1C_RRZO_BASE_ADDR, RRZO_BASE_ADDR, RRZO_R1C_BASE_ADDR, U)\
    CMD(a, b, c, d, e, f, RRZO_R1C_RRZO_OFST_ADDR, RRZO_OFST_ADDR, RRZO_R1C_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, RRZO_R1C_RRZO_XSIZE, RRZO_XSIZE, RRZO_R1C_XSIZE, U)\
    CMD(a, b, c, d, e, f, RRZO_R1C_RRZO_YSIZE, RRZO_YSIZE, RRZO_R1C_YSIZE, U)\
    CMD(a, b, c, d, e, f, RRZO_R1C_RRZO_STRIDE, RRZO_STRIDE, RRZO_R1C_STRIDE, U)\
    /* UFGO_R1 - RRZ branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_DMA_EN, CAMCTL_UFGO_R1_EN, CAMCTL_R1C_UFGO_R1_EN, U)\
    CMD(a, b, c, d, e, f, UFGO_R1C_UFGO_BASE_ADDR, UFGO_BASE_ADDR, UFGO_R1C_BASE_ADDR, U)\
    CMD(a, b, c, d, e, f, UFGO_R1C_UFGO_OFST_ADDR, UFGO_OFST_ADDR, UFGO_R1C_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, UFGO_R1C_UFGO_XSIZE, UFGO_XSIZE, UFGO_R1C_XSIZE, U)\
    CMD(a, b, c, d, e, f, UFGO_R1C_UFGO_YSIZE, UFGO_YSIZE, UFGO_R1C_YSIZE, U)\
    CMD(a, b, c, d, e, f, UFGO_R1C_UFGO_STRIDE, UFGO_STRIDE, UFGO_R1C_STRIDE, U)\
    /* QBN_R5 - LTM branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN2, CAMCTL_QBN_R5_EN, CAMCTL_R1C_QBN_R5_EN, U)\
    CMD(a, b, c, d, e, f, QBIN_R5C_QBIN_CTL, QBIN_ACC, QBIN_R5C_ACC, U)\
    CMD(a, b, c, d, e, f, QBIN_R5C_QBIN_CTL, QBIN_ACC_MODE, QBIN_R5C_ACC_MODE, U)\
    /* MRG_R5 - LTM branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN, CAMCTL_MRG_R5_EN, CAMCTL_R1C_MRG_R5_EN, U)\
    CMD(a, b, c, d, e, f, MRG_R5C_MRG_CTL, MRG_SIG_MODE1, MRG_R5C_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, MRG_R5C_MRG_CTL, MRG_SIG_MODE2, MRG_R5C_SIG_MODE2, U)\
    /*CMD(a, b, c, d, e, f, MRG_R5C_MRG_CTL, MRG_EDGE, MRG_R5C_EDGE, U)*/\
    CMD(a, b, c, d, e, f, MRG_R5C_MRG_CROP, MRG_STR_X, MRG_R5C_STR_X, U)\
    CMD(a, b, c, d, e, f, MRG_R5C_MRG_CROP, MRG_END_X, MRG_R5C_END_X, U)\
    /*CMD(a, b, c, d, e, f, MRG_R5C_MRG_VSIZE, MRG_HT, MRG_R5C_HT, U)*/\
    /* LCS_SEL - LTM branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_SEL, CAMCTL_LCES_SEL, CAMCTL_R1C_LCES_SEL, U)\
    /* LCESO_R1 - LTM branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_DMA_EN, CAMCTL_LCESO_R1_EN, CAMCTL_R1C_LCESO_R1_EN, U)\
    /* DM_R1_SEL - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_SEL2, CAMCTL_DM_R1_SEL, CAMCTL_R1C_DM_R1_SEL, U)\
    /* MRG_R13 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN3, CAMCTL_MRG_R13_EN, CAMCTL_R1C_MRG_R13_EN, U)\
    CMD(a, b, c, d, e, f, MRG_R13C_MRG_CTL, MRG_SIG_MODE1, MRG_R13C_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, MRG_R13C_MRG_CTL, MRG_SIG_MODE2, MRG_R13C_SIG_MODE2, U)\
    /*CMD(a, b, c, d, e, f, MRG_R13C_MRG_CTL, MRG_EDGE, MRG_R13C_EDGE, U)*/\
    CMD(a, b, c, d, e, f, MRG_R13C_MRG_CROP, MRG_STR_X, MRG_R13C_STR_X, U)\
    CMD(a, b, c, d, e, f, MRG_R13C_MRG_CROP, MRG_END_X, MRG_R13C_END_X, U)\
    /*CMD(a, b, c, d, e, f, MRG_R13C_MRG_VSIZE, MRG_HT, MRG_R13C_HT, U)*/\
    /* MRG_R14 YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN3, CAMCTL_MRG_R14_EN, CAMCTL_R1C_MRG_R14_EN, U)\
    CMD(a, b, c, d, e, f, MRG_R14C_MRG_CTL, MRG_SIG_MODE1, MRG_R14C_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, MRG_R14C_MRG_CTL, MRG_SIG_MODE2, MRG_R14C_SIG_MODE2, U)\
    /*CMD(a, b, c, d, e, f, MRG_R14C_MRG_CTL, MRG_EDGE, MRG_R14C_EDGE, U)*/\
    CMD(a, b, c, d, e, f, MRG_R14C_MRG_CROP, MRG_STR_X, MRG_R14C_STR_X, U)\
    CMD(a, b, c, d, e, f, MRG_R14C_MRG_CROP, MRG_END_X, MRG_R14C_END_X, U)\
    /*CMD(a, b, c, d, e, f, MRG_R14C_MRG_VSIZE, MRG_HT, MRG_R14C_HT, U)*/\
    /* DM_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN2, CAMCTL_DM_R1_EN, CAMCTL_R1C_DM_R1_EN, U)\
    CMD(a, b, c, d, e, f, DM_R1C_DM_SL_CTL, DM_SL_EN, DM_R1C_SL_EN, U)\
    /* SLK_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN, CAMCTL_SLK_R1_EN, CAMCTL_R1C_SLK_R1_EN, U)\
    CMD(a, b, c, d, e, f, SLK_R1C_SLK_RZ, SLK_HRZ_COMP, SLK_R1C_HRZ_COMP, U)\
    CMD(a, b, c, d, e, f, SLK_R1C_SLK_RZ, SLK_VRZ_COMP, SLK_R1C_VRZ_COMP, U)\
    CMD(a, b, c, d, e, f, SLK_R1C_SLK_XOFF, SLK_X_OFST, SLK_R1C_X_OFST, U)\
    CMD(a, b, c, d, e, f, SLK_R1C_SLK_SIZE, SLK_TPIPE_WD, SLK_R1C_TPIPE_WD, U)\
    CMD(a, b, c, d, e, f, SLK_R1C_SLK_SIZE, SLK_TPIPE_HT, SLK_R1C_TPIPE_HT, U)\
    /* FLC_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN3, CAMCTL_FLC_R1_EN, CAMCTL_R1C_FLC_R1_EN, U)\
    /* CCM_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN2, CAMCTL_CCM_R1_EN, CAMCTL_R1C_CCM_R1_EN, U)\
    /* GGM_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN2, CAMCTL_GGM_R1_EN, CAMCTL_R1C_GGM_R1_EN, U)\
    /* G2C_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN2, CAMCTL_G2C_R1_EN, CAMCTL_R1C_G2C_R1_EN, U)\
    /* C42_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN2, CAMCTL_C42_R1_EN, CAMCTL_R1C_C42_R1_EN, U)\
    /* YNRS_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN3, CAMCTL_YNRS_R1_EN, CAMCTL_R1C_YNRS_R1_EN, U)\
    CMD(a, b, c, d, e, f, YNRS_R1C_YNRS_CON1, YNRS_SL2_LINK, YNRS_R1C_SL2_LINK, U)\
    /* SLK_R2 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN2, CAMCTL_SLK_R2_EN, CAMCTL_R1C_SLK_R2_EN, U)\
    CMD(a, b, c, d, e, f, SLK_R2C_SLK_RZ, SLK_HRZ_COMP, SLK_R2C_HRZ_COMP, U)\
    CMD(a, b, c, d, e, f, SLK_R2C_SLK_RZ, SLK_VRZ_COMP, SLK_R2C_VRZ_COMP, U)\
    CMD(a, b, c, d, e, f, SLK_R2C_SLK_XOFF, SLK_X_OFST, SLK_R2C_X_OFST, U)\
    CMD(a, b, c, d, e, f, SLK_R2C_SLK_SIZE, SLK_TPIPE_WD, SLK_R2C_TPIPE_WD, U)\
    CMD(a, b, c, d, e, f, SLK_R2C_SLK_SIZE, SLK_TPIPE_HT, SLK_R2C_TPIPE_HT, U)\
    /* CRP_R4 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN3, CAMCTL_CRP_R4_EN, CAMCTL_R1C_CRP_R4_EN, U)\
    CMD(a, b, c, d, e, f, CRP_R4C_CRP_X_POS, CRP_XSTART, CRP_R4C_XSTART, U)\
    CMD(a, b, c, d, e, f, CRP_R4C_CRP_X_POS, CRP_XEND, CRP_R4C_XEND, U)\
    CMD(a, b, c, d, e, f, CRP_R4C_CRP_Y_POS, CRP_YSTART, CRP_R4C_YSTART, U)\
    CMD(a, b, c, d, e, f, CRP_R4C_CRP_Y_POS, CRP_YEND, CRP_R4C_YEND, U)\
    /* CRSP_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN3, CAMCTL_CRSP_R1_EN, CAMCTL_R1C_CRSP_R1_EN, U)\
    CMD(a, b, c, d, e, f, CRSP_R1C_CRSP_OUT_IMG, CRSP_WD, CRSP_R1C_tgWidth, U)\
    CMD(a, b, c, d, e, f, CRSP_R1C_CRSP_OUT_IMG, CRSP_HT, CRSP_R1C_tgHeight, U)\
    CMD(a, b, c, d, e, f, CRSP_R1C_CRSP_CROP_X, CRSP_CROP_XSTART, CRSP_R1C_CROP_XSTART, U)\
    CMD(a, b, c, d, e, f, CRSP_R1C_CRSP_CROP_X, CRSP_CROP_XEND, CRSP_R1C_CROP_XEND, U)\
    CMD(a, b, c, d, e, f, CRSP_R1C_CRSP_CROP_Y, CRSP_CROP_YSTART, CRSP_R1C_CROP_YSTART, U)\
    CMD(a, b, c, d, e, f, CRSP_R1C_CRSP_CROP_Y, CRSP_CROP_YEND, CRSP_R1C_CROP_YEND, U)\
    /* PAK_R3 YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN3, CAMCTL_PAK_R3_EN, CAMCTL_R1C_PAK_R3_EN, U)\
    CMD(a, b, c, d, e, f, PAK_R3C_PAK_CONT, PAK_YUV_DNG, PAK_R3C_YUV_DNG, U)\
    CMD(a, b, c, d, e, f, PAK_R3C_PAK_CONT, PAK_YUV_BIT, PAK_R3C_YUV_BIT, U)\
    /* PAK_R4 YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN3, CAMCTL_PAK_R4_EN, CAMCTL_R1C_PAK_R4_EN, U)\
    CMD(a, b, c, d, e, f, PAK_R4C_PAK_CONT, PAK_YUV_DNG, PAK_R4C_YUV_DNG, U)\
    CMD(a, b, c, d, e, f, PAK_R4C_PAK_CONT, PAK_YUV_BIT, PAK_R4C_YUV_BIT, U)\
    /* PAK_R5 YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN3, CAMCTL_PAK_R5_EN, CAMCTL_R1C_PAK_R5_EN, U)\
    CMD(a, b, c, d, e, f, PAK_R5C_PAK_CONT, PAK_YUV_DNG, PAK_R5C_YUV_DNG, U)\
    CMD(a, b, c, d, e, f, PAK_R5C_PAK_CONT, PAK_YUV_BIT, PAK_R5C_YUV_BIT, U)\
    /* PLNW_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN3, CAMCTL_PLNW_R1_EN, CAMCTL_R1C_PLNW_R1_EN, U)\
    /* YUVO_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_FMT2_SEL, CAMCTL_YUVO_R1_FMT, CAMCTL_R1C_YUVO_R1_FMT, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_DMA2_EN, CAMCTL_YUVO_R1_EN, CAMCTL_R1C_YUVO_R1_EN, U)\
    CMD(a, b, c, d, e, f, YUVO_R1C_YUVO_BASE_ADDR, YUVO_BASE_ADDR, YUVO_R1C_BASE_ADDR, U)\
    CMD(a, b, c, d, e, f, YUVO_R1C_YUVO_OFST_ADDR, YUVO_OFST_ADDR, YUVO_R1C_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, YUVO_R1C_YUVO_XSIZE, YUVO_XSIZE, YUVO_R1C_XSIZE, U)\
    CMD(a, b, c, d, e, f, YUVO_R1C_YUVO_YSIZE, YUVO_YSIZE, YUVO_R1C_YSIZE, U)\
    CMD(a, b, c, d, e, f, YUVO_R1C_YUVO_STRIDE, YUVO_STRIDE, YUVO_R1C_STRIDE, U)\
    /* YUVBO_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_DMA_EN, CAMCTL_YUVBO_R1_EN, CAMCTL_R1C_YUVBO_R1_EN, U)\
    CMD(a, b, c, d, e, f, YUVBO_R1C_YUVBO_BASE_ADDR, YUVBO_BASE_ADDR, YUVBO_R1C_BASE_ADDR, U)\
    CMD(a, b, c, d, e, f, YUVBO_R1C_YUVBO_OFST_ADDR, YUVBO_OFST_ADDR, YUVBO_R1C_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, YUVBO_R1C_YUVBO_XSIZE, YUVBO_XSIZE, YUVBO_R1C_XSIZE, U)\
    CMD(a, b, c, d, e, f, YUVBO_R1C_YUVBO_YSIZE, YUVBO_YSIZE, YUVBO_R1C_YSIZE, U)\
    CMD(a, b, c, d, e, f, YUVBO_R1C_YUVBO_STRIDE, YUVBO_STRIDE, YUVBO_R1C_STRIDE, U)\
    /* YUVCO_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_DMA_EN, CAMCTL_YUVCO_R1_EN, CAMCTL_R1C_YUVCO_R1_EN, U)\
    CMD(a, b, c, d, e, f, YUVCO_R1C_YUVCO_BASE_ADDR, YUVCO_BASE_ADDR, YUVCO_R1C_BASE_ADDR, U)\
    CMD(a, b, c, d, e, f, YUVCO_R1C_YUVCO_OFST_ADDR, YUVCO_OFST_ADDR, YUVCO_R1C_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, YUVCO_R1C_YUVCO_XSIZE, YUVCO_XSIZE, YUVCO_R1C_XSIZE, U)\
    CMD(a, b, c, d, e, f, YUVCO_R1C_YUVCO_YSIZE, YUVCO_YSIZE, YUVCO_R1C_YSIZE, U)\
    CMD(a, b, c, d, e, f, YUVCO_R1C_YUVCO_STRIDE, YUVCO_STRIDE, YUVCO_R1C_STRIDE, U)\
    /* BS_R3 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN, CAMCTL_BS_R3_EN, CAMCTL_R1C_BS_R3_EN, U)\
    /* RSS_R2 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN3, CAMCTL_RSS_R2_EN, CAMCTL_R1C_RSS_R2_EN, U)\
    CMD(a, b, c, d, e, f, RSS_R2C_RSS_IN_IMG, RSS_IN_WD, RDSS_R2C_Input_Image_W, U)\
    CMD(a, b, c, d, e, f, RSS_R2C_RSS_IN_IMG, RSS_IN_HT, RDSS_R2C_Input_Image_H, U)\
    CMD(a, b, c, d, e, f, RSS_R2C_RSS_OUT_IMG, RSS_OUT_WD, RDSS_R2C_Output_Image_W, U)\
    CMD(a, b, c, d, e, f, RSS_R2C_RSS_OUT_IMG, RSS_OUT_HT, RDSS_R2C_Output_Image_H, U)\
    CMD(a, b, c, d, e, f, RSS_R2C_RSS_CONTROL, RSS_HORI_EN, RDSS_R2C_Horizontal_EN, U)\
    CMD(a, b, c, d, e, f, RSS_R2C_RSS_CONTROL, RSS_VERT_EN, RDSS_R2C_Vertical_EN, U)\
    CMD(a, b, c, d, e, f, RSS_R2C_RSS_HORI_STEP, RSS_HORI_STEP, RDSS_R2C_Horizontal_Coeff_Step, U)\
    CMD(a, b, c, d, e, f, RSS_R2C_RSS_VERT_STEP, RSS_VERT_STEP, RDSS_R2C_Vertical_Coeff_Step, U)\
    CMD(a, b, c, d, e, f, RSS_R2C_RSS_LUMA_HORI_INT_OFST, RSS_LUMA_HORI_INT_OFST, RDSS_R2C_Luma_Horizontal_Integer_Offset, U)\
    CMD(a, b, c, d, e, f, RSS_R2C_RSS_LUMA_HORI_SUB_OFST, RSS_LUMA_HORI_SUB_OFST, RDSS_R2C_Luma_Horizontal_Subpixel_Offset, U)\
    /* RSSO_R2 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_DMA2_EN, CAMCTL_RSSO_R2_EN, CAMCTL_R1C_RSSO_R2_EN, U)\
    CMD(a, b, c, d, e, f, RSSO_R2C_RSSO_BASE_ADDR, RSSO_BASE_ADDR, RSSO_R2C_BASE_ADDR, U)\
    CMD(a, b, c, d, e, f, RSSO_R2C_RSSO_OFST_ADDR, RSSO_OFST_ADDR, RSSO_R2C_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, RSSO_R2C_RSSO_XSIZE, RSSO_XSIZE, RSSO_R2C_XSIZE, U)\
    CMD(a, b, c, d, e, f, RSSO_R2C_RSSO_YSIZE, RSSO_YSIZE, RSSO_R2C_YSIZE, U)\
    CMD(a, b, c, d, e, f, RSSO_R2C_RSSO_STRIDE, RSSO_STRIDE, RSSO_R2C_STRIDE, U)\
    /* GGM_R2 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN3, CAMCTL_GGM_R2_EN, CAMCTL_R1C_GGM_R2_EN, U)\
    /* G2C_R2 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN3, CAMCTL_G2C_R2_EN, CAMCTL_R1C_G2C_R2_EN, U)\
    /* C42_R2 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN3, CAMCTL_C42_R2_EN, CAMCTL_R1C_C42_R2_EN, U)\
    /* BS_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN, CAMCTL_BS_R1_EN, CAMCTL_R1C_BS_R1_EN, U)\
    /* CRZ_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN3, CAMCTL_CRZ_R1_EN, CAMCTL_R1C_CRZ_R1_EN, U)\
    CMD(a, b, c, d, e, f, CRZ_R1C_CRZ_IN_IMG, CRZ_IN_WD, CRZ_R1C_IN_WD, U)\
    CMD(a, b, c, d, e, f, CRZ_R1C_CRZ_IN_IMG, CRZ_IN_HT, CRZ_R1C_IN_HT, U)\
    CMD(a, b, c, d, e, f, CRZ_R1C_CRZ_OUT_IMG, CRZ_OUT_WD, CRZ_R1C_OUT_WD, U)\
    CMD(a, b, c, d, e, f, CRZ_R1C_CRZ_OUT_IMG, CRZ_OUT_HT, CRZ_R1C_OUT_HT, U)\
    CMD(a, b, c, d, e, f, CRZ_R1C_CRZ_CONTROL, CRZ_HORI_EN, CRZ_R1C_HORI_EN, U)\
    CMD(a, b, c, d, e, f, CRZ_R1C_CRZ_CONTROL, CRZ_VERT_EN, CRZ_R1C_VERT_EN, U)\
    CMD(a, b, c, d, e, f, CRZ_R1C_CRZ_CONTROL, CRZ_HORI_ALGO, CRZ_R1C_HORI_ALGO, U)\
    CMD(a, b, c, d, e, f, CRZ_R1C_CRZ_CONTROL, CRZ_VERT_ALGO, CRZ_R1C_VERT_ALGO, U)\
    CMD(a, b, c, d, e, f, CRZ_R1C_CRZ_HORI_STEP, CRZ_HORI_STEP, CRZ_R1C_HORI_STEP, U)\
    CMD(a, b, c, d, e, f, CRZ_R1C_CRZ_VERT_STEP, CRZ_VERT_STEP, CRZ_R1C_VERT_STEP, U)\
    CMD(a, b, c, d, e, f, CRZ_R1C_CRZ_LUMA_HORI_INT_OFST, CRZ_LUMA_HORI_INT_OFST, CRZ_R1C_LUMA_HORI_INT_OFST, U)\
    CMD(a, b, c, d, e, f, CRZ_R1C_CRZ_LUMA_HORI_SUB_OFST, CRZ_LUMA_HORI_SUB_OFST, CRZ_R1C_LUMA_HORI_SUB_OFST, U)\
    CMD(a, b, c, d, e, f, CRZ_R1C_CRZ_CHRO_HORI_INT_OFST, CRZ_CHRO_HORI_INT_OFST, CRZ_R1C_CHRO_HORI_INT_OFST, U)\
    CMD(a, b, c, d, e, f, CRZ_R1C_CRZ_CHRO_HORI_SUB_OFST, CRZ_CHRO_HORI_SUB_OFST, CRZ_R1C_CHRO_HORI_SUB_OFST, U)\
    /* PLNW_R2 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN3, CAMCTL_PLNW_R2_EN, CAMCTL_R1C_PLNW_R2_EN, U)\
    /* CRZO_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_FMT_SEL, CAMCTL_CRZO_R1_FMT, CAMCTL_R1C_CRZO_R1_FMT, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_DMA_EN, CAMCTL_CRZO_R1_EN, CAMCTL_R1C_CRZO_R1_EN, U)\
    CMD(a, b, c, d, e, f, CRZO_R1C_CRZO_BASE_ADDR, CRZO_BASE_ADDR, CRZO_R1C_BASE_ADDR, U)\
    CMD(a, b, c, d, e, f, CRZO_R1C_CRZO_OFST_ADDR, CRZO_OFST_ADDR, CRZO_R1C_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, CRZO_R1C_CRZO_XSIZE, CRZO_XSIZE, CRZO_R1C_XSIZE, U)\
    CMD(a, b, c, d, e, f, CRZO_R1C_CRZO_YSIZE, CRZO_YSIZE, CRZO_R1C_YSIZE, U)\
    CMD(a, b, c, d, e, f, CRZO_R1C_CRZO_STRIDE, CRZO_STRIDE, CRZO_R1C_STRIDE, U)\
    /* CRZBO_R1 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_DMA_EN, CAMCTL_CRZBO_R1_EN, CAMCTL_R1C_CRZBO_R1_EN, U)\
    CMD(a, b, c, d, e, f, CRZBO_R1C_CRZBO_BASE_ADDR, CRZBO_BASE_ADDR, CRZBO_R1C_BASE_ADDR, U)\
    CMD(a, b, c, d, e, f, CRZBO_R1C_CRZBO_OFST_ADDR, CRZBO_OFST_ADDR, CRZBO_R1C_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, CRZBO_R1C_CRZBO_XSIZE, CRZBO_XSIZE, CRZBO_R1C_XSIZE, U)\
    CMD(a, b, c, d, e, f, CRZBO_R1C_CRZBO_YSIZE, CRZBO_YSIZE, CRZBO_R1C_YSIZE, U)\
    CMD(a, b, c, d, e, f, CRZBO_R1C_CRZBO_STRIDE, CRZBO_STRIDE, CRZBO_R1C_STRIDE, U)\
    /* BS_R2 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN, CAMCTL_BS_R2_EN, CAMCTL_R1C_BS_R2_EN, U)\
    /* CRZ_R2 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN3, CAMCTL_CRZ_R2_EN, CAMCTL_R1C_CRZ_R2_EN, U)\
    CMD(a, b, c, d, e, f, CRZ_R2C_CRZ_IN_IMG, CRZ_IN_WD, CRZ_R2C_IN_WD, U)\
    CMD(a, b, c, d, e, f, CRZ_R2C_CRZ_IN_IMG, CRZ_IN_HT, CRZ_R2C_IN_HT, U)\
    CMD(a, b, c, d, e, f, CRZ_R2C_CRZ_OUT_IMG, CRZ_OUT_WD, CRZ_R2C_OUT_WD, U)\
    CMD(a, b, c, d, e, f, CRZ_R2C_CRZ_OUT_IMG, CRZ_OUT_HT, CRZ_R2C_OUT_HT, U)\
    CMD(a, b, c, d, e, f, CRZ_R2C_CRZ_CONTROL, CRZ_HORI_EN, CRZ_R2C_HORI_EN, U)\
    CMD(a, b, c, d, e, f, CRZ_R2C_CRZ_CONTROL, CRZ_VERT_EN, CRZ_R2C_VERT_EN, U)\
    CMD(a, b, c, d, e, f, CRZ_R2C_CRZ_CONTROL, CRZ_HORI_ALGO, CRZ_R2C_HORI_ALGO, U)\
    CMD(a, b, c, d, e, f, CRZ_R2C_CRZ_CONTROL, CRZ_VERT_ALGO, CRZ_R2C_VERT_ALGO, U)\
    CMD(a, b, c, d, e, f, CRZ_R2C_CRZ_HORI_STEP, CRZ_HORI_STEP, CRZ_R2C_HORI_STEP, U)\
    CMD(a, b, c, d, e, f, CRZ_R2C_CRZ_VERT_STEP, CRZ_VERT_STEP, CRZ_R2C_VERT_STEP, U)\
    CMD(a, b, c, d, e, f, CRZ_R2C_CRZ_LUMA_HORI_INT_OFST, CRZ_LUMA_HORI_INT_OFST, CRZ_R2C_LUMA_HORI_INT_OFST, U)\
    CMD(a, b, c, d, e, f, CRZ_R2C_CRZ_LUMA_HORI_SUB_OFST, CRZ_LUMA_HORI_SUB_OFST, CRZ_R2C_LUMA_HORI_SUB_OFST, U)\
    CMD(a, b, c, d, e, f, CRZ_R2C_CRZ_CHRO_HORI_INT_OFST, CRZ_CHRO_HORI_INT_OFST, CRZ_R2C_CHRO_HORI_INT_OFST, U)\
    CMD(a, b, c, d, e, f, CRZ_R2C_CRZ_CHRO_HORI_SUB_OFST, CRZ_CHRO_HORI_SUB_OFST, CRZ_R2C_CHRO_HORI_SUB_OFST, U)\
    /* CRSP_R2 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN3, CAMCTL_CRSP_R2_EN, CAMCTL_R1C_CRSP_R2_EN, U)\
    CMD(a, b, c, d, e, f, CRSP_R2C_CRSP_OUT_IMG, CRSP_WD, CRSP_R2C_tgWidth, U)\
    CMD(a, b, c, d, e, f, CRSP_R2C_CRSP_OUT_IMG, CRSP_HT, CRSP_R2C_tgHeight, U)\
    CMD(a, b, c, d, e, f, CRSP_R2C_CRSP_CROP_X, CRSP_CROP_XSTART, CRSP_R2C_CROP_XSTART, U)\
    CMD(a, b, c, d, e, f, CRSP_R2C_CRSP_CROP_X, CRSP_CROP_XEND, CRSP_R2C_CROP_XEND, U)\
    CMD(a, b, c, d, e, f, CRSP_R2C_CRSP_CROP_Y, CRSP_CROP_YSTART, CRSP_R2C_CROP_YSTART, U)\
    CMD(a, b, c, d, e, f, CRSP_R2C_CRSP_CROP_Y, CRSP_CROP_YEND, CRSP_R2C_CROP_YEND, U)\
    /* PLNW_R3 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN3, CAMCTL_PLNW_R3_EN, CAMCTL_R1C_PLNW_R3_EN, U)\
    /* CRZO_R2 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_FMT_SEL, CAMCTL_CRZO_R2_FMT, CAMCTL_R1C_CRZO_R2_FMT, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_DMA2_EN, CAMCTL_CRZO_R2_EN, CAMCTL_R1C_CRZO_R2_EN, U)\
    CMD(a, b, c, d, e, f, CRZO_R2C_CRZO_BASE_ADDR, CRZO_BASE_ADDR, CRZO_R2C_BASE_ADDR, U)\
    CMD(a, b, c, d, e, f, CRZO_R2C_CRZO_OFST_ADDR, CRZO_OFST_ADDR, CRZO_R2C_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, CRZO_R2C_CRZO_XSIZE, CRZO_XSIZE, CRZO_R2C_XSIZE, U)\
    CMD(a, b, c, d, e, f, CRZO_R2C_CRZO_YSIZE, CRZO_YSIZE, CRZO_R2C_YSIZE, U)\
    CMD(a, b, c, d, e, f, CRZO_R2C_CRZO_STRIDE, CRZO_STRIDE, CRZO_R2C_STRIDE, U)\
    /* CRZBO_R2 - YUV branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_DMA2_EN, CAMCTL_CRZBO_R2_EN, CAMCTL_R1C_CRZBO_R2_EN, U)\
    CMD(a, b, c, d, e, f, CRZBO_R2C_CRZBO_BASE_ADDR, CRZBO_BASE_ADDR, CRZBO_R2C_BASE_ADDR, U)\
    CMD(a, b, c, d, e, f, CRZBO_R2C_CRZBO_OFST_ADDR, CRZBO_OFST_ADDR, CRZBO_R2C_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, CRZBO_R2C_CRZBO_XSIZE, CRZBO_XSIZE, CRZBO_R2C_XSIZE, U)\
    CMD(a, b, c, d, e, f, CRZBO_R2C_CRZBO_YSIZE, CRZBO_YSIZE, CRZBO_R2C_YSIZE, U)\
    CMD(a, b, c, d, e, f, CRZBO_R2C_CRZBO_STRIDE, CRZBO_STRIDE, CRZBO_R2C_STRIDE, U)\
    /* QBN_R4 - HLR branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN, CAMCTL_QBN_R4_EN, CAMCTL_R1C_QBN_R4_EN, U)\
    CMD(a, b, c, d, e, f, QBIN_R4C_QBIN_CTL, QBIN_ACC, QBIN_R4C_ACC, U)\
    CMD(a, b, c, d, e, f, QBIN_R4C_QBIN_CTL, QBIN_ACC_MODE, QBIN_R4C_ACC_MODE, U)\
    /* MRG_R4 - HLR branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN, CAMCTL_MRG_R4_EN, CAMCTL_R1C_MRG_R4_EN, U)\
    CMD(a, b, c, d, e, f, MRG_R4C_MRG_CTL, MRG_SIG_MODE1, MRG_R4C_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, MRG_R4C_MRG_CTL, MRG_SIG_MODE2, MRG_R4C_SIG_MODE2, U)\
    /*CMD(a, b, c, d, e, f, MRG_R4C_MRG_CTL, MRG_EDGE, MRG_R4C_EDGE, U)*/\
    CMD(a, b, c, d, e, f, MRG_R4C_MRG_CROP, MRG_STR_X, MRG_R4C_STR_X, U)\
    CMD(a, b, c, d, e, f, MRG_R4C_MRG_CROP, MRG_END_X, MRG_R4C_END_X, U)\
    /*CMD(a, b, c, d, e, f, MRG_R4C_MRG_VSIZE, MRG_HT, MRG_R4C_HT, U)*/\
    /* LTMS_SEL - HLR branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_SEL, CAMCTL_LTMS_SEL, CAMCTL_R1C_LTMS_SEL, U)\
    /* QBN_R1 - LSC branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN2, CAMCTL_QBN_R1_EN, CAMCTL_R1C_QBN_R1_EN, U)\
    CMD(a, b, c, d, e, f, QBIN_R1C_QBIN_CTL, QBIN_ACC, QBIN_R1C_ACC, U)\
    CMD(a, b, c, d, e, f, QBIN_R1C_QBIN_CTL, QBIN_ACC_MODE, QBIN_R1C_ACC_MODE, U)\
    /* MRG_R1 - LSC branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN, CAMCTL_MRG_R1_EN, CAMCTL_R1C_MRG_R1_EN, U)\
    CMD(a, b, c, d, e, f, MRG_R1C_MRG_CTL, MRG_SIG_MODE1, MRG_R1C_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, MRG_R1C_MRG_CTL, MRG_SIG_MODE2, MRG_R1C_SIG_MODE2, U)\
    /*CMD(a, b, c, d, e, f, MRG_R1C_MRG_CTL, MRG_EDGE, MRG_R1C_EDGE, U)*/\
    CMD(a, b, c, d, e, f, MRG_R1C_MRG_CROP, MRG_STR_X, MRG_R1C_STR_X, U)\
    CMD(a, b, c, d, e, f, MRG_R1C_MRG_CROP, MRG_END_X, MRG_R1C_END_X, U)\
    /*CMD(a, b, c, d, e, f, MRG_R1C_MRG_VSIZE, MRG_HT, MRG_R1C_HT, U)*/\
    /* AA_SEL - LSC branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_SEL, CAMCTL_AA_SEL, CAMCTL_R1C_AA_SEL, U)\
    /* AAO_R1 - LSC branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_DMA_EN, CAMCTL_AAO_R1_EN, CAMCTL_R1C_AAO_R1_EN, U)\
    /* CRP_R1_SEL - AFO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_SEL, CAMCTL_CRP_R1_SEL, CAMCTL_R1C_CRP_R1_SEL, U)\
    /* CRP_R1 - AFO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN2, CAMCTL_CRP_R1_EN, CAMCTL_R1C_CRP_R1_EN, U)\
    CMD(a, b, c, d, e, f, CRP_R1C_CRP_X_POS, CRP_XSTART, CRP_R1C_XSTART, U)\
    CMD(a, b, c, d, e, f, CRP_R1C_CRP_X_POS, CRP_XEND, CRP_R1C_XEND, U)\
    CMD(a, b, c, d, e, f, CRP_R1C_CRP_Y_POS, CRP_YSTART, CRP_R1C_YSTART, U)\
    CMD(a, b, c, d, e, f, CRP_R1C_CRP_Y_POS, CRP_YEND, CRP_R1C_YEND, U)\
    /* AF_R1 - AFO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN2, CAMCTL_AF_R1_EN, CAMCTL_R1C_AF_R1_EN, U)\
    CMD(a, b, c, d, e, f, AF_R1C_AF_BLK_0, AF_BLK_XSIZE, AF_R1C_BLK_XSIZE, U)\
    CMD(a, b, c, d, e, f, AF_R1C_AF_BLK_0, AF_BLK_YSIZE, AF_R1C_BLK_YSIZE, U)\
    CMD(a, b, c, d, e, f, AF_R1C_AF_BLK_1, AF_BLK_XNUM, AF_R1C_BLK_XNUM, U)\
    CMD(a, b, c, d, e, f, AF_R1C_AF_BLK_PROT, AF_PROT_BLK_XSIZE, AF_R1C_PROT_BLK_XSIZE, U)\
    CMD(a, b, c, d, e, f, AF_R1C_AF_BLK_PROT, AF_PROT_BLK_YSIZE, AF_R1C_PROT_BLK_YSIZE, U)\
    CMD(a, b, c, d, e, f, AF_R1C_AF_VLD, AF_VLD_XSTART, AF_R1C_VLD_XSTART, U)\
    CMD(a, b, c, d, e, f, AF_R1C_AF_BLK_1, AF_BLK_YNUM, AF_R1C_BLK_YNUM, U)\
    CMD(a, b, c, d, e, f, AF_R1C_AF_VLD, AF_VLD_YSTART, AF_R1C_VLD_YSTART, U)\
    CMD(a, b, c, d, e, f, AF_R1C_AF_CON2, AF_DS_EN, AF_R1C_DS_EN, U)\
    CMD(a, b, c, d, e, f, AF_R1C_AF_CON, AF_H_GONLY, AF_R1C_H_GONLY, U)\
    /* AF_SEL - AFO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_SEL, CAMCTL_AF_SEL, CAMCTL_R1C_AF_SEL, U)\
    /* AFO_R1 - AFO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_DMA_EN, CAMCTL_AFO_R1_EN, CAMCTL_R1C_AFO_R1_EN, U)\
    CMD(a, b, c, d, e, f, AFO_R1C_AFO_BASE_ADDR, AFO_BASE_ADDR, AFO_R1C_BASE_ADDR, U)\
    CMD(a, b, c, d, e, f, AFO_R1C_AFO_OFST_ADDR, AFO_OFST_ADDR, AFO_R1C_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, AFO_R1C_AFO_XSIZE, AFO_XSIZE, AFO_R1C_XSIZE, U)\
    CMD(a, b, c, d, e, f, AFO_R1C_AFO_YSIZE, AFO_YSIZE, AFO_R1C_YSIZE, U)\
    CMD(a, b, c, d, e, f, AFO_R1C_AFO_STRIDE, AFO_STRIDE, AFO_R1C_STRIDE, U)\
    /* QBN_R2 - TSFSO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN2, CAMCTL_QBN_R2_EN, CAMCTL_R1C_QBN_R2_EN, U)\
    CMD(a, b, c, d, e, f, QBIN_R2C_QBIN_CTL, QBIN_ACC, QBIN_R2C_ACC, U)\
    CMD(a, b, c, d, e, f, QBIN_R2C_QBIN_CTL, QBIN_ACC_MODE, QBIN_R2C_ACC_MODE, U)\
    /* MRG_R2 - TSFSO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_DMA_EN, CAMCTL_TSFSO_R1_EN, CAMCTL_R1C_TSFSO_R1_EN, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN2, CAMCTL_MRG_R2_EN, CAMCTL_R1C_MRG_R2_EN, U)\
    CMD(a, b, c, d, e, f, MRG_R2C_MRG_CTL, MRG_SIG_MODE1, MRG_R2C_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, MRG_R2C_MRG_CTL, MRG_SIG_MODE2, MRG_R2C_SIG_MODE2, U)\
    /*CMD(a, b, c, d, e, f, MRG_R2C_MRG_CTL, MRG_EDGE, MRG_R2C_EDGE, U)*/\
    CMD(a, b, c, d, e, f, MRG_R2C_MRG_CROP, MRG_STR_X, MRG_R2C_STR_X, U)\
    CMD(a, b, c, d, e, f, MRG_R2C_MRG_CROP, MRG_END_X, MRG_R2C_END_X, U)\
    /*CMD(a, b, c, d, e, f, MRG_R2C_MRG_VSIZE, MRG_HT, MRG_R2C_HT, U)*/\
    /* CRP_R3_SEL - IMGO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_SEL, CAMCTL_CRP_R3_SEL, CAMCTL_R1C_CRP_R3_SEL, U)\
    /* CRP_R3 - IMGO branch*/\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN, CAMCTL_CRP_R3_EN, CAMCTL_R1C_CRP_R3_EN, U)\
    CMD(a, b, c, d, e, f, CRP_R3C_CRP_X_POS, CRP_XSTART, CRP_R3C_XSTART, U)\
    CMD(a, b, c, d, e, f, CRP_R3C_CRP_X_POS, CRP_XEND, CRP_R3C_XEND, U)\
    CMD(a, b, c, d, e, f, CRP_R3C_CRP_Y_POS, CRP_YSTART, CRP_R3C_YSTART, U)\
    CMD(a, b, c, d, e, f, CRP_R3C_CRP_Y_POS, CRP_YEND, CRP_R3C_YEND, U)\
    /* IMG_SEL - IMGO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_SEL, CAMCTL_IMG_SEL, CAMCTL_R1C_IMG_SEL, U)\
    /* MRG_R3 - IMGO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN4, CAMCTL_MRG_R3_EN, CAMCTL_R1C_MRG_R3_EN, U)\
    CMD(a, b, c, d, e, f, MRG_R3C_MRG_CTL, MRG_SIG_MODE1, MRG_R3C_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, MRG_R3C_MRG_CTL, MRG_SIG_MODE2, MRG_R3C_SIG_MODE2, U)\
    /*CMD(a, b, c, d, e, f, MRG_R3C_MRG_CTL, MRG_EDGE, MRG_R3C_EDGE, U)*/\
    CMD(a, b, c, d, e, f, MRG_R3C_MRG_CROP, MRG_STR_X, MRG_R3C_STR_X, U)\
    CMD(a, b, c, d, e, f, MRG_R3C_MRG_CROP, MRG_END_X, MRG_R3C_END_X, U)\
    /*CMD(a, b, c, d, e, f, MRG_R3C_MRG_VSIZE, MRG_HT, MRG_R3C_HT, U)*/\
    /* MRG_R12 IMGO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN3, CAMCTL_MRG_R12_EN, CAMCTL_R1C_MRG_R12_EN, U)\
    CMD(a, b, c, d, e, f, MRG_R12C_MRG_CTL, MRG_SIG_MODE1, MRG_R12C_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, MRG_R12C_MRG_CTL, MRG_SIG_MODE2, MRG_R12C_SIG_MODE2, U)\
    /*CMD(a, b, c, d, e, f, MRG_R12C_MRG_CTL, MRG_EDGE, MRG_R12C_EDGE, U)*/\
    CMD(a, b, c, d, e, f, MRG_R12C_MRG_CROP, MRG_STR_X, MRG_R12C_STR_X, U)\
    CMD(a, b, c, d, e, f, MRG_R12C_MRG_CROP, MRG_END_X, MRG_R12C_END_X, U)\
    /*CMD(a, b, c, d, e, f, MRG_R12C_MRG_VSIZE, MRG_HT, MRG_R12C_HT, U)*/\
    /* UFE_R1 - IMGO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN3, CAMCTL_UFE_R1_EN, CAMCTL_R1C_UFE_R1_EN, U)\
    CMD(a, b, c, d, e, f, UFE_R1C_UFE_CON, UFE_FORCE_PCM, UFE_R1C_FORCE_PCM, U)\
    /* PAK_R1 - IMGO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN, CAMCTL_PAK_R1_EN, CAMCTL_R1C_PAK_R1_EN, U)\
    /* IMGO_SEL - IMGO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_SEL, CAMCTL_IMGO_SEL, CAMCTL_R1C_IMGO_SEL, U)\
    /* IMGO_R1 - IMGO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_DMA_EN, CAMCTL_IMGO_R1_EN, CAMCTL_R1C_IMGO_R1_EN, U)\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_FMT_SEL, CAMCTL_IMGO_R1_FMT, CAMCTL_R1C_IMGO_R1_FMT, U)\
    CMD(a, b, c, d, e, f, IMGO_R1C_IMGO_BASE_ADDR, IMGO_BASE_ADDR, IMGO_R1C_BASE_ADDR, U)\
    CMD(a, b, c, d, e, f, IMGO_R1C_IMGO_OFST_ADDR, IMGO_OFST_ADDR, IMGO_R1C_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, IMGO_R1C_IMGO_XSIZE, IMGO_XSIZE, IMGO_R1C_XSIZE, U)\
    CMD(a, b, c, d, e, f, IMGO_R1C_IMGO_YSIZE, IMGO_YSIZE, IMGO_R1C_YSIZE, U)\
    CMD(a, b, c, d, e, f, IMGO_R1C_IMGO_STRIDE, IMGO_STRIDE, IMGO_R1C_STRIDE, U)\
    CMD(a, b, c, d, e, f, IMGO_R1C_IMGO_CROP, IMGO_XOFFSET, IMGO_R1C_XOFFSET, U)\
    CMD(a, b, c, d, e, f, IMGO_R1C_IMGO_CROP, IMGO_YOFFSET, IMGO_R1C_YOFFSET, U)\
    /* UFEO_R1 - IMGO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_DMA_EN, CAMCTL_UFEO_R1_EN, CAMCTL_R1C_UFEO_R1_EN, U)\
    CMD(a, b, c, d, e, f, UFEO_R1C_UFEO_BASE_ADDR, UFEO_BASE_ADDR, UFEO_R1C_BASE_ADDR, U)\
    CMD(a, b, c, d, e, f, UFEO_R1C_UFEO_OFST_ADDR, UFEO_OFST_ADDR, UFEO_R1C_OFST_ADDR, U)\
    CMD(a, b, c, d, e, f, UFEO_R1C_UFEO_XSIZE, UFEO_XSIZE, UFEO_R1C_XSIZE, U)\
    CMD(a, b, c, d, e, f, UFEO_R1C_UFEO_YSIZE, UFEO_YSIZE, UFEO_R1C_YSIZE, U)\
    CMD(a, b, c, d, e, f, UFEO_R1C_UFEO_STRIDE, UFEO_STRIDE, UFEO_R1C_STRIDE, U)\
    /* FLK_SEL - FLKO branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_SEL, CAMCTL_FLK_SEL, CAMCTL_R1C_FLK_SEL, U)\
    /*QBN_R3 - FLKO branch*/\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN3, CAMCTL_QBN_R3_EN, CAMCTL_R1C_QBN_R3_EN, U)\
    CMD(a, b, c, d, e, f, QBIN_R3C_QBIN_CTL, QBIN_ACC, QBIN_R3C_ACC, U)\
    CMD(a, b, c, d, e, f, QBIN_R3C_QBIN_CTL, QBIN_ACC_MODE, QBIN_R3C_ACC_MODE, U)\
    /* MRG_R10 - FLKO branch */\
    /*CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN, CAMCTL_MRG_R10_EN, CAMCTL_R1C_MRG_R10_EN, U)*/\
    CMD(a, b, c, d, e, f, MRG_R10C_MRG_CTL, MRG_SIG_MODE1, MRG_R10C_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, MRG_R10C_MRG_CTL, MRG_SIG_MODE2, MRG_R10C_SIG_MODE2, U)\
    /*CMD(a, b, c, d, e, f, MRG_R10C_MRG_CTL, MRG_EDGE, MRG_R10C_EDGE, U)*/\
    CMD(a, b, c, d, e, f, MRG_R10C_MRG_CROP, MRG_STR_X, MRG_R10C_STR_X, U)\
    CMD(a, b, c, d, e, f, MRG_R10C_MRG_CROP, MRG_END_X, MRG_R10C_END_X, U)\
    /*CMD(a, b, c, d, e, f, MRG_R10C_MRG_VSIZE, MRG_HT, MRG_R10C_HT, U)*/\
    /* MRG_R6 - ADL branch */\
    CMD(a, b, c, d, e, f, CAMCTL_R1C_CAMCTL_EN3, CAMCTL_MRG_R6_EN, CAMCTL_R1C_MRG_R6_EN, U)\
    CMD(a, b, c, d, e, f, MRG_R6C_MRG_CTL, MRG_SIG_MODE1, MRG_R6C_SIG_MODE1, U)\
    CMD(a, b, c, d, e, f, MRG_R6C_MRG_CTL, MRG_SIG_MODE2, MRG_R6C_SIG_MODE2, U)\
    /*CMD(a, b, c, d, e, f, MRG_R6C_MRG_CTL, MRG_EDGE, MRG_R6C_EDGE, U)*/\
    CMD(a, b, c, d, e, f, MRG_R6C_MRG_CROP, MRG_STR_X, MRG_R6C_STR_X, U)\
    CMD(a, b, c, d, e, f, MRG_R6C_MRG_CROP, MRG_END_X, MRG_R6C_END_X, U)\
    /*CMD(a, b, c, d, e, f, MRG_R6C_MRG_VSIZE, MRG_HT, MRG_R6C_HT, U)*/\

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
#define DUAL_READ_LOG_FILL_VAL_ENGINE(a, b, c, d, e, f, cq, g, h, i, j, k,...) \
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
#define DUAL_READ_LOG_FILL_VAL_ENGINE_T(a, b, c, d, e, f, g, h, i, j, k,...) \
    if (false == (d))\
    {\
        if (0 == memcmp(b, #j, sizeof(#j)))\
        {\
            (a)->j.Raw = (c);\
            (d) = true;\
            (e)->j.Raw = 0xFFFFFFFF;\
        }\
    }\

/* data search & update reg mapper */
/* a: ptr_reg */
/* b: cmp str */
/* c: value to fill */
/* d: found flag */
#define DUAL_READ_LOG_FILL_VAL_ARRAY(a, b, c, d, e, f, cq, g, h, i, j, k, l,...) \
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
#define DUAL_READ_LOG_FILL_VAL_ARRAY_D(a, b, c, d, e, f, cq, g, h, i, j, k, l,...) \
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
#define DUAL_READ_LOG_FILL_VAL_ARRAY_T(a, b, c, d, e, f, g, h, i, j, k, l,...) \
    for (int temp_i=0;temp_i<l;temp_i++)\
    {\
        if (false == (d))\
        {\
            char array_name[DUAL_MAX_DUMP_COLUMN_LENGTH];\
            memset(array_name, 0x0, DUAL_MAX_DUMP_COLUMN_LENGTH);\
            dual_sprintf(array_name, DUAL_MAX_DUMP_COLUMN_LENGTH, "%s[%d]", #j, temp_i);\
            if (0 == memcmp(b, array_name, DUAL_MAX_DUMP_COLUMN_LENGTH))\
            {\
                (a)->j[temp_i].Raw = (c);\
                (d) = true;\
                (e)->j[temp_i].Raw = 0xFFFFFFFF;\
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
#define DUAL_READ_CHECK_LOG_FLAG_ENGINE(a, b, c, d, e, f, cq, g, h, i, j, k,...) \
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
#define DUAL_READ_CHECK_LOG_FLAG_ENGINE_T(a, b, c, d, e, f, g, h, i, j, k,...) \
    if (0 == (a)->j.Raw)\
    {\
        (b) = true;\
        printf("Not found input config: %s\n", #j);\
    }\

/* data search & update reg mapper */
/* a: ptr_reg */
/* b: cmp str */
/* c: value to fill */
/* d: found flag */
#define DUAL_READ_CHECK_LOG_FLAG_ARRAY(a, b, c, d, e, f, cq, g, h, i, j, k, l,...) \
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
#define DUAL_READ_CHECK_LOG_FLAG_ARRAY_D(a, b, c, d, e, f, cq, g, h, i, j, k, l,...) \
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
#define DUAL_READ_CHECK_LOG_FLAG_ARRAY_T(a, b, c, d, e, f, g, h, i, j, k, l,...) \
    for (int temp_i=0;temp_i<l;temp_i++)\
    {\
        if (0 == (a)->j[temp_i].Raw)\
        {\
            (b) = true;\
            printf("Not found input config: %s[%d]\n", #j, temp_i);\
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
#define DUAL_READ_CHECK_REG_FLAG_ENGINE(a, b, c, d, e, f, cq, g, h, i, j, k,...) \
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
#define DUAL_READ_CHECK_REG_FLAG_ENGINE_D(a, b, c, d, e, f, cq, g, h, i, j, k,...) \
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
#define DUAL_READ_CHECK_REG_FLAG_ENGINE_T(a, b, c, d, e, f, g, h, i, j, k,...) \
    if (0 == (a)->j.Raw)\
    {\
        (c) = true;\
        (b)->j.Raw = 0xFFFFFFFF;\
        printf("Not found reg: %s\n", #j);\
    }\

/* data search & update reg mapper */
/* a: ptr_reg */
/* b: cmp str */
/* c: value to fill */
/* d: found flag */
#define DUAL_READ_CHECK_REG_FLAG_ARRAY(a, b, c, d, e, f, cq, g, h, i, j, k, l,...) \
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
#define DUAL_READ_CHECK_REG_FLAG_ARRAY_D(a, b, c, d, e, f, cq, g, h, i, j, k, l,...) \
    for (int temp_i=0;temp_i<l;temp_i++)\
    {\
        if (0 == (a)->i[temp_i].Raw)\
        {\
            (c) = true;\
            (b)->i[temp_i].Raw = 0xFFFFFFFF;\
            printf("Not found reg: %s[%d]\n", #i, temp_i);\
        }\
    }\

/* data search & update reg mapper */
/* a: ptr_reg */
/* b: cmp str */
/* c: value to fill */
/* d: found flag */
#define DUAL_READ_CHECK_REG_FLAG_ARRAY_T(a, b, c, d, e, f, g, h, i, j, k, l,...) \
    for (int temp_i=0;temp_i<l;temp_i++)\
    {\
        if (0 == (a)->j[temp_i].Raw)\
        {\
            (c) = true;\
            (b)->j[temp_i].Raw = 0xFFFFFFFF;\
            printf("Not found reg: %s[%d]\n", #j, temp_i);\
        }\
    }\

#define DUAL_LOG_PRINT_TXT(a, b, c, d, e, f, g, h,...) \
    b(a, "%s %s.%s%s%d\n", DUAL_IN_DUMP_HEADER, #g, #h, DUAL_SPACE_EQUAL_SYMBOL_STR, (c)->g.h);

#define DUAL_LOG_PRINT_SW_TXT(a, b, c, d, e, f, g, h, i, j,...) \
    b(a, "%s %s.%s%s%d\n", DUAL_IN_DUMP_HEADER, #g, #h, DUAL_SPACE_EQUAL_SYMBOL_STR, (c)->g.h);

#define DUAL_LOG_PRINT_TXT_HW(a, b, c, d, e, f, g, h,...) \
    b(a, "%s %s%s0x%x\n", DUAL_IN_DUMP_HEADER, #h, DUAL_SPACE_EQUAL_SYMBOL_STR, (c)->h.Raw);

#define DUAL_LOG_PRINT_TXT_ENGINE(a, b, c, d, e, f, cq, g, h, i, j, k,...) \
    if ((d) & 0x1)\
    {\
        /* A+B, A+C, A+B+C, A/A */\
        b(a, "%s %s%s0x%x\n", DUAL_IN_DUMP_HEADER, #h, DUAL_SPACE_EQUAL_SYMBOL_STR, (c)->h.Raw);\
    }\

#define DUAL_LOG_PRINT_TXT_ENGINE_D(a, b, c, d, e, f, cq, g, h, i, j, k,...) \
    if (0x3 == ((d) & 0x3))\
    {\
        /* A+B, A+B+C, B/B */\
        b(a, "%s %s%s0x%x\n", DUAL_IN_DUMP_HEADER, #i, DUAL_SPACE_EQUAL_SYMBOL_STR, (c)->i.Raw);\
    }\

#define DUAL_LOG_PRINT_TXT_ARRAY(a, b, c, d, e, f, g, h, i, j, k, l,...) \
    if ((d) & 0x1)\
    {\
        /* A+B, A+C, A+B+C, A/A */\
        for (int temp_i=0;temp_i<l;temp_i++)\
        {\
            b(a, "%s %s[%d]%s0x%x\n", DUAL_IN_DUMP_HEADER, #h, temp_i, DUAL_SPACE_EQUAL_SYMBOL_STR, (c)->h[temp_i].Raw);\
        }\
    }\
    else\
    {\
        /* B+C, B/A */\
        b(a, "%s %s[%d]%s0x%x\n", DUAL_IN_DUMP_HEADER, #i, temp_i, DUAL_SPACE_EQUAL_SYMBOL_STR, (c)->h[temp_i].Raw);\
    }\

#define DUAL_LOG_PRINT_TXT_ARRAY_D(a, b, c, d, e, f, g, h, i, j, k, l,...) \
    if (0x3 == ((d) & 0x3))\
    {\
        /* A+B, A+B+C, B/B */\
        for (int temp_i=0;temp_i<l;temp_i++)\
        {\
            b(a, "%s %s[%d]%s0x%x\n", DUAL_IN_DUMP_HEADER, #i, temp_i, DUAL_SPACE_EQUAL_SYMBOL_STR, (c)->i[temp_i].Raw);\
        }\
    }\
    else\
    {\
        /* A+C, B+C, C/B */\
        for (int temp_i=0;temp_i<l;temp_i++)\
        {\
            b(a, "%s %s[%d]%s0x%x\n", DUAL_IN_DUMP_HEADER, #j, temp_i, DUAL_SPACE_EQUAL_SYMBOL_STR, (c)->i[temp_i].Raw);\
        }\
    }\

#define DUAL_LOG_PRINT_TXT_ENGINE_OUT(a, b, c, d, e, f, cq, g, h, i, j, k,...) \
    b(a, "%s %s%s0x%x\n", DUAL_OUT_DUMP_HEADER, #h, DUAL_SPACE_EQUAL_SYMBOL_STR, (c)->h.Raw);

#define DUAL_LOG_PRINT_TXT_ENGINE_OUT_D(a, b, c, d, e, f, cq, g, h, i, j, k,...) \
    b(a, "%s %s%s0x%x\n", DUAL_OUT_DUMP_HEADER, #i, DUAL_SPACE_EQUAL_SYMBOL_STR, (c)->i.Raw);

#define DUAL_LOG_PRINT_TXT_ARRAY_OUT(a, b, c, d, e, f, cq, g, h, i, j, k, l,...) \
    for (int temp_i=0;temp_i<l;temp_i++)\
    {\
        b(a, "%s %s[%d]%s0x%x\n", DUAL_OUT_DUMP_HEADER, #h, temp_i, DUAL_SPACE_EQUAL_SYMBOL_STR, (c)->h[temp_i].Raw);\
    }\

#define DUAL_LOG_PRINT_TXT_ARRAY_OUT_D(a, b, c, d, e, f, cq, g, h, i, j, k, l,...) \
    for (int temp_i=0;temp_i<l;temp_i++)\
    {\
        b(a, "%s %s[%d]%s0x%x\n", DUAL_OUT_DUMP_HEADER, #i, temp_i, DUAL_SPACE_EQUAL_SYMBOL_STR, (c)->i[temp_i].Raw);\
    }\

#define DUAL_PRINT_SW_TXT(a, b, c, d, e, f, g, h, i, j,...) \
    b(a, "%s %s.%s%s%d (0x%x) [knobs] %s [c model] %s\n", DUAL_ENG_DUMP_HEADER, #g, #h, DUAL_SPACE_EQUAL_SYMBOL_STR, (c)->g.h, (c)->g.h, #i, #j);\

#define DUAL_PRINT_HW_TXT(a, b, c, d, e, f, g, h, i,...) \
    b(a, "%s %s.%s%s%d (0x%x) [c model] %s\n", DUAL_ENG_DUMP_HEADER, #g, #h, DUAL_SPACE_EQUAL_SYMBOL_STR, (c)->g.Bits.h, (c)->g.Bits.h, #i);\

#define DUAL_UART_PRINT_TXT(a, b, c, d, e, f, g, h,...) \
    a("%s %s.%s%s%d\n", DUAL_IN_DUMP_HEADER, #g, #h, DUAL_SPACE_EQUAL_SYMBOL_STR, (b)->g.h);

#define DUAL_UART_PRINT_SW_TXT(a, b, c, d, e, f, g, h, i, j,...) \
    a("%s %s.%s%s%d\n", DUAL_IN_DUMP_HEADER, #g, #h, DUAL_SPACE_EQUAL_SYMBOL_STR, (b)->g.h);

#define DUAL_UART_PRINT_TXT_HW(a, b, c, d, e, f, g, h,...) \
    a("%s %s%s0x%x\n", DUAL_IN_DUMP_HEADER, #h, DUAL_SPACE_EQUAL_SYMBOL_STR, (b)->h.Raw);

#define DUAL_UART_PRINT_TXT_ENGINE(a, b, c, d, e, f, cq, g, h, i, j, k,...) \
    if ((c) & 0x1)\
    {\
        /* A+B, A+C, A+B+C, A/A */\
        a("%s %s%s0x%x\n", DUAL_IN_DUMP_HEADER, #h, DUAL_SPACE_EQUAL_SYMBOL_STR, (b)->h.Raw);\
    }\

#define DUAL_UART_PRINT_TXT_ENGINE_D(a, b, c, d, e, f, cq, g, h, i, j, k,...) \
    if (0x3== ((c) & 0x3))\
    {\
        /* A+B, A+B+C, B/B */\
        a("%s %s%s0x%x\n", DUAL_IN_DUMP_HEADER, #i, DUAL_SPACE_EQUAL_SYMBOL_STR, (b)->i.Raw);\
    }\

#define DUAL_UART_PRINT_TXT_ARRAY(a, b, c, d, e, f, g, h, i, j, k, l,...) \
    if ((c) & 0x1)\
    {\
        /* A+B, A+C, A+B+C, A/A */\
        for (int temp_i=0;temp_i<l;temp_i++)\
        {\
            a("%s %s[%d]%s0x%x\n", DUAL_IN_DUMP_HEADER, #h, temp_i, DUAL_SPACE_EQUAL_SYMBOL_STR, (b)->h[temp_i].Raw);\
        }\
    }\
    else\
    {\
        /* B+C, B/A */\
        for (int temp_i=0;temp_i<l;temp_i++)\
        {\
            a("%s %s[%d]%s0x%x\n", DUAL_IN_DUMP_HEADER, #i, temp_i, DUAL_SPACE_EQUAL_SYMBOL_STR, (b)->h[temp_i].Raw);\
        }\
    }\

#define DUAL_UART_PRINT_TXT_ARRAY_D(a, b, c, d, e, f, g, h, i, j, k, l,...) \
    if (0x3== ((c) & 0x3))\
    {\
        /* A+B, A+B+C, B/B */\
        for (int temp_i=0;temp_i<l;temp_i++)\
        {\
            a("%s %s[%d]%s0x%x\n", DUAL_IN_DUMP_HEADER, #i, temp_i, DUAL_SPACE_EQUAL_SYMBOL_STR, (b)->i[temp_i].Raw);\
        }\
    }\
    else\
    {\
        /* B+C, C/B */\
        for (int temp_i=0;temp_i<l;temp_i++)\
        {\
            a("%s %s[%d]%s0x%x\n", DUAL_IN_DUMP_HEADER, #j, temp_i, DUAL_SPACE_EQUAL_SYMBOL_STR, (b)->i[temp_i].Raw);\
        }\
    }\

#define DUAL_LOG_REG_TXT_ENGINE(a, b, c, d, e, f, cq, g, h, i, j, k,...) \
    b(a, "%s 0x%08X%s0x%08x \\\\%s\n", DUAL_REG_DUMP_HEADER, (unsigned int)((char *)&((c)->h) - (char *)(c) + (d)), DUAL_SPACE_EQUAL_SYMBOL_STR, (c)->h.Raw, #h);

#define DUAL_LOG_REG_TXT_ENGINE_D(a, b, c, d, e, f, cq, g, h, i, j, k,...) \
    b(a, "%s 0x%08X%s0x%08x \\\\%s\n", DUAL_REG_DUMP_HEADER, (unsigned int)((char *)&((c)->i) - (char *)(c) + (d)), DUAL_SPACE_EQUAL_SYMBOL_STR, (c)->i.Raw, #i);

#define DUAL_LOG_REG_TXT_ARRAY(a, b, c, d, e, f, cq, g, h, i, j, k, l,...) \
    for (int temp_i=0;temp_i<l;temp_i++)\
    {\
        b(a, "%s 0x%08X%s0x%08x \\\\%s[%d]\n", DUAL_REG_DUMP_HEADER, (unsigned int)((char *)&((c)->h[temp_i]) - (char *)(c) + (d)), DUAL_SPACE_EQUAL_SYMBOL_STR, (c)->h[temp_i].Raw, #h, temp_i);\
    }\

#define DUAL_LOG_REG_TXT_ARRAY_D(a, b, c, d, e, f, cq, g, h, i, j, k, l,...) \
    for (int temp_i=0;temp_i<l;temp_i++)\
    {\
        b(a, "%s 0x%08X%s0x%08x \\\\%s[%d]\n", DUAL_REG_DUMP_HEADER, (unsigned int)((char *)&((c)->i[temp_i]) - (char *)(c) + (d)), DUAL_SPACE_EQUAL_SYMBOL_STR, (c)->i[temp_i].Raw, #i, temp_i);\
    }\

#define DUAL_UART_REG_TXT_HW(a, b, c, d, e, f, cq, g, h,...) \
    a("%s 0x%08X%s0x%08x \\\\%s\n", DUAL_REG_DUMP_HEADER, (unsigned int)((char *)&((b)->h) - (char *)(b) + (c)), DUAL_SPACE_EQUAL_SYMBOL_STR, (b)->h.Raw, #h);

#define DUAL_UART_REG_TXT_ENGINE(a, b, c, d, e, f, cq, g, h, i, j, k,...) \
    a("%s 0x%08X%s0x%08x \\\\%s\n", DUAL_REG_DUMP_HEADER, (unsigned int)((char *)&((b)->h) - (char *)(b) + (c)), DUAL_SPACE_EQUAL_SYMBOL_STR, (b)->h.Raw, #h);

#define DUAL_UART_REG_TXT_ENGINE_D(a, b, c, d, e, f, cq, g, h, i, j, k,...) \
    a("%s 0x%08X%s0x%08x \\\\%s\n", DUAL_REG_DUMP_HEADER, (unsigned int)((char *)&((b)->i) - (char *)(b) + (c)), DUAL_SPACE_EQUAL_SYMBOL_STR, (b)->i.Raw, #i);

#define DUAL_UART_REG_TXT_ARRAY(a, b, c, d, e, f, cq, g, h, i, j, k, l,...) \
    for (int temp_i=0;temp_i<l;temp_i++)\
    {\
        a("%s 0x%08X%s0x%08x \\\\%s[%d]\n", DUAL_REG_DUMP_HEADER, (unsigned int)((char *)&((b)->h[temp_i]) - (char *)(b) + (c)), DUAL_SPACE_EQUAL_SYMBOL_STR, (b)->h[temp_i].Raw, #h, temp_i);\
    }\

#define DUAL_UART_REG_TXT_ARRAY_D(a, b, c, d, e, f, cq, g, h, i, j, k, l,...) \
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

#define DUAL_COPY_DUAL_A_A(a, b, c, d, e, f, cq, g, h, i, j, k,...) \
    (a)->h.Raw = (b)->h.Raw;

#define DUAL_COPY_DUAL_A_A_CQ_IN(a, b, c, d, e, f, cq, g, h, i, j, k,...) \
{ \
    if (cq) \
        (a)->h.Raw = (c)->h.Raw; \
    else \
        (a)->h.Raw = (b)->h.Raw; \
}

#define DUAL_COPY_DUAL_A_A_CQ_OUT(a, b, c, d, e, f, cq, g, h, i, j, k,...) \
{ \
    if (cq) \
        (b)->h.Raw = (c)->h.Raw; \
    else \
        (a)->h.Raw = (c)->h.Raw; \
}

#define DUAL_COPY_DUAL_A_B(a, b, c, d, e, f, cq, g, h, i, j, k,...) \
    (a)->i.Raw = (b)->h.Raw;

#define DUAL_COPY_DUAL_A_B_CQ_IN(a, b, c, d, e, f, cq, g, h, i, j, k,...) \
{ \
    if (cq) \
        (a)->i.Raw = (c)->h.Raw; \
    else \
        (a)->i.Raw = (b)->h.Raw; \
}

#define DUAL_COPY_DUAL_A_B_CQ_OUT(a, b, c, d, e, f, cq, g, h, i, j, k,...) \
{ \
    if (cq) \
        (b)->i.Raw = (d)->h.Raw; \
    else \
        (a)->i.Raw = (c)->h.Raw; \
}

#define DUAL_COPY_DUAL_B_A(a, b, c, d, e, f, cq, g, h, i, j, k,...) \
    (a)->h.Raw = (b)->i.Raw;

#define DUAL_COPY_DUAL_B_B(a, b, c, d, e, f, cq, g, h, i, j, k,...) \
    (a)->i.Raw = (b)->i.Raw;

#define DUAL_COPY_DUAL_B_B_CQ_IN(a, b, c, d, e, f, cq, g, h, i, j, k,...) \
{ \
    if (cq) \
        (a)->i.Raw = (c)->i.Raw; \
    else \
        (a)->i.Raw = (b)->i.Raw; \
}

#define DUAL_COPY_DUAL_B_B_CQ_OUT(a, b, c, d, e, f, cq, g, h, i, j, k,...) \
{ \
    if (cq) \
        (b)->i.Raw = (c)->i.Raw; \
    else \
        (a)->i.Raw = (c)->i.Raw; \
}

#define DUAL_COPY_ARRAY_A_A(a, b, c, d, e, f, cq, g, h, i, j, k, l,...) \
    for (int temp_i = 0; temp_i < l; temp_i++)\
    {\
        (a)->h[temp_i].Raw = (b)->h[temp_i].Raw;\
    }\

#define DUAL_COPY_ARRAY_A_B(a, b, c, d, e, f, cq, g, h, i, j, k, l,...) \
    for (int temp_i = 0; temp_i < l; temp_i++)\
    {\
        (a)->i[temp_i].Raw = (b)->h[temp_i].Raw;\
    }\

#define DUAL_COPY_ARRAY_A_B_CQ_OUT(a, b, c, d, e, f, cq, g, h, i, j, k, l,...) \
    for (int temp_i = 0; temp_i < l; temp_i++)\
    {\
		if (cq) \
			(b)->i[temp_i].Raw = (d)->h[temp_i].Raw;\
		else \
            (a)->i[temp_i].Raw = (c)->h[temp_i].Raw;\
    }\

#define DUAL_COPY_ARRAY_B_A(a, b, c, d, e, f, cq, g, h, i, j, k, l,...) \
    for (int temp_i=0;temp_i<l;temp_i++)\
    {\
        (a)->h[temp_i].Raw = (b)->i[temp_i].Raw;\
    }\

#define DUAL_COPY_ARRAY_B_B(a, b, c, d, e, f, cq, g, h, i, j, k, l,...) \
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
    /*DUAL_HW_C_LIST(DUAL_CONFIG_HW_LUT_DECLARE,,,,,,);*/
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
