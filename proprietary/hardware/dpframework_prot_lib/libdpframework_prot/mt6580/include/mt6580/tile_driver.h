#ifndef __TILE_DRIVER_H__
#define __TILE_DRIVER_H__

#ifdef MTK_LOG_ENABLE
#undef MTK_LOG_ENABLE
#endif
#define MTK_LOG_ENABLE 1
#include "tile_isp_reg.h"
#include "tile_mdp_reg.h"

#define TILE_PLATFORM_DRIVER /* to change define with platform unique */

#if defined(_MSC_VER)
#include <conio.h>
#define _kbhit() _kbhit()
#define mtk_stricmp                                             _stricmp
#define FOLDER_SYMBOL_STR                                       "\\"
#define FOLDER_SYMBOL_CHAR                                      '\\'
#define mtk_sprintf(dst_ptr, size_dst, format, ...)             sprintf_s(dst_ptr, size_dst, format, __VA_ARGS__)
#define mtk_fopen(file_ptr, filename_ptr, mode)                 fopen_s(&file_ptr, filename_ptr, mode)
#define mtk_fscanf                                              fscanf_s
#define COPY_CMD                                                "copy"
#define CMP_CMD                                                 "fc"
#define DEL_CMD                                                 "del"
#define mtk_sscanf_1(x, y, a, b)                               sscanf_s(x, y, a, b)
#define mtk_sscanf_2(x, y, a, b, c, d)							sscanf_s(x, y, a, b, c, d)
#define TILE_SAVE_FILE_FIRST_DIR ".\\tile_conf\\"
#define TILE_SAVE_FILE_SECOND_DIR ".\\"
#elif defined(linux) || defined(__linux) || defined(__arm__)
#define _kbhit() true
#define mtk_stricmp                                             strcasecmp
#define FOLDER_SYMBOL_STR                                       "/"
#define FOLDER_SYMBOL_CHAR                                      '/'
#define mtk_sprintf(dst_ptr, size_dst, ...)                     sprintf(dst_ptr, __VA_ARGS__)
#define mtk_fopen(file_ptr, filename_ptr, mode)                 file_ptr = fopen(filename_ptr, mode)
#define mtk_fscanf                                              fscanf
#define COPY_CMD                                                "cp"
#define CMP_CMD                                                 "cmp"
#define DEL_CMD                                                 "rm"
#define mtk_sscanf_1(x, y, a, b)                               sscanf(x, y, a)
#define mtk_sscanf_2(x, y, a, b, c, d)		sscanf(x, y, a, b)
#define TILE_SAVE_FILE_FIRST_DIR "./tile_conf/"
#define TILE_SAVE_FILE_SECOND_DIR "./"
#define O_BINARY (0)
#else
#error("Non-supported c compiler environment\n")
#endif

#ifdef TILE_PLATFORM_DRIVER
#if defined(_MSC_VER)           // VC
#define tile_driver_printf printf_prefix
#define tile_driver_printf_no_prefix printf
#define uart_printf printf
#elif defined(linux) || defined(__linux) //Linux
#if defined(__arm__) || defined(__aarch64__)//arm
//#define tile_driver_printf(...)
//#define tile_driver_printf_no_prefix(...)
#define NEW_LINE_CHAR   "\n"
#if defined(USING_MTK_LDVT)     // LDVT
#pragma message("LDVT environment")
#include "uvvf.h"
#define uart_printf(fmt, arg...) VV_MSG(LOG_TAG "[%s] " fmt NEW_LINE_CHAR, __func__, ##arg)
#else //not LDVT
//#pragma message("FPGA/EVB/Phone environment")
#include <android/log.h>        // Android
#define tile_driver_printf(fmt, ...) ((void)__android_log_print(ANDROID_LOG_ERROR, "MDP/T", "[" __FILE__ "][%d] " fmt, __LINE__, ##__VA_ARGS__))
#define tile_driver_printf_no_prefix(fmt, ...) ((void)__android_log_print(ANDROID_LOG_ERROR, "MDP/T", fmt, ##__VA_ARGS__))
#define uart_printf(fmt, ...) ((void)__android_log_print(ANDROID_LOG_ERROR, "MDP/T", "[] " fmt, ##__VA_ARGS__))
#endif //END LDVT
#else //not arm
#define tile_driver_printf printf_prefix
#define tile_driver_printf_no_prefix printf
#define uart_printf printf
#endif //END platform
#endif //END VC
#else
#define tile_driver_printf printf_prefix
#define tile_driver_printf_no_prefix printf
#endif

#define MAX_TILE_SIZE (8192)

#ifdef TILE_PLATFORM_DRIVER
#define MAX_TILE_WIDTH_HW (768)
#define MAX_TILE_HEIGHT_HW (8192)
#define MAX_TILE_TOT_NO (100)
#define MAX_ISP_DUMP_HEX_PER_TILE (36)
#define MAX_MDP_DUMP_HEX_PER_TILE (72)
#define MAX_ISP_TILE_TDR_HEX_NO (MAX_TILE_TOT_NO*MAX_ISP_DUMP_HEX_PER_TILE)
#define MAX_MDP_TILE_TDR_HEX_NO (MAX_TILE_TOT_NO*MAX_MDP_DUMP_HEX_PER_TILE)
#define MAX_REGISTER_STRING_LENGTH (4)
#define MAX_TILE_PREV_NO (4)
#define MAX_TILE_BRANCH_NO (4)
#define MIN_TILE_FUNC_NO (3)
#define MAX_TILE_FUNC_NO (32) /* smaller or equal to (PREVIOUS_BLK_NO_OF_START-1) */
#define MAX_INPUT_TILE_FUNC_NO (4)
#define MAX_FORWARD_FUNC_CAL_LOOP_NO (16*MAX_TILE_FUNC_NO)
#define MAX_TILE_TDR_REG_NO (2)
#define MAX_TILE_FUNC_EN_NO (64)
#define MAX_TILE_FUNC_SUBRDMA_NO (16)
#else
#define MAX_TILE_WIDTH_HW (768) //(8192)
#define MAX_TILE_HEIGHT_HW (8192)
#define MAX_TILE_TOT_NO (100)
#define MAX_ISP_DUMP_HEX_PER_TILE (36)
#define MAX_MDP_DUMP_HEX_PER_TILE (72)
#define MAX_ISP_TILE_TDR_HEX_NO (MAX_TILE_TOT_NO*MAX_ISP_DUMP_HEX_PER_TILE)
#define MAX_MDP_TILE_TDR_HEX_NO (MAX_TILE_TOT_NO*MAX_MDP_DUMP_HEX_PER_TILE)
#define MAX_REGISTER_STRING_LENGTH (4)
#define MAX_TILE_PREV_NO (4)
#define MAX_TILE_BRANCH_NO (4)
#define MIN_TILE_FUNC_NO (3)
#define MAX_TILE_FUNC_NO (32) /* smaller or equal to (PREVIOUS_BLK_NO_OF_START-1) */
#define MAX_INPUT_TILE_FUNC_NO (4)
#define MAX_FORWARD_FUNC_CAL_LOOP_NO (16*MAX_TILE_FUNC_NO)
#define MAX_TILE_TDR_REG_NO (2)
#define MAX_TILE_FUNC_EN_NO (64)
#define MAX_TILE_FUNC_SUBRDMA_NO (16)
#endif

/* common define */

#ifndef __cplusplus
#define bool unsigned char
#define true 1
#define false 0
#endif

#define MTK_MOD(num, denom) (((denom)==2)?((num)&0x1):(((denom)==4)?((num)&0x3):((num)%(denom))))

/* tile cal & dump order */
#define TILE_CAL_DUMP_ORDER_TEST/* for test tdr sort only */
#define TILE_ORDER_Y_FIRST (0x1)
#define TILE_ORDER_RIGHT_TO_LEFT (0x2)
#define TILE_ORDER_BOTTOM_TO_TOP (0x4)

/* normalized offset up to 20 bits */
#define REZ_OFFSET_SHIFT_FACTOR (20)
#define MAX_PATHNAME_LENGTH (256)
#define MAX_FILENAME_LENGTH (128)
#define MAX_COMMAND_LENGTH (512)
#define MAX_DUMP_COLUMN_LENGTH (1024)
#define MIN_MCU_BUFFER_NO (2)
#define PREVIOUS_BLK_NO_OF_START (0xFF)

/* debug log dump & parse */
#define TILE_DEBUG_DUMP_VERSION "V.0.0.0.1"
#define TILE_DEBUG_DUMP_VERSION_START_CHAR 'V'
#define TILE_DEBUG_DUMP_VERSION_SIZE (sizeof(TILE_DEBUG_DUMP_VERSION))
/* must be size of TILE_DEBUG_DUMP_VERSION */
#define TILE_DEBUG_EQUAL_SYMBOL_CHR '='
#define TILE_DEBUG_SPACE_EQUAL_SYMBOL_STR " = "

/* 4 bytes alignment */
#define TILE_4_BYTES_PENDING(x) (((x) & 0x3)?((x) + 4 - ((x) & 0x3)):(x))

/* macro for debug print to file, #include <stdio.h> */
#define printf_prefix(...) {\
    const char *ptr_char = get_current_file_name(__FILE__);\
    printf("[%s][%s][%d] ", ptr_char,  __FUNCTION__, __LINE__);\
    printf(__VA_ARGS__);\
}

/* TILE RUN DPFRAMEWORK */
typedef enum TILE_CAL_MAIN_DP_ENUM
{
    TILE_CAL_MAIN_DP_INIT_FUNC_PROP=0,
    TILE_CAL_MAIN_DP_INIT_FRAME_MODE,
    TILE_CAL_MAIN_DP_CLOSE_FRAME_MODE,
    TILE_CAL_MAIN_DP_INIT_TILE_MODE,
    TILE_CAL_MAIN_DP_CLOSE_TILE_MODE,
    TILE_CAL_MAIN_DP_MAX_NO
}TILE_CAL_MAIN_DP_ENUM;

/* last_irq_mode */
typedef enum TILE_LAST_IRQ_MODE_ENUM
{
    TILE_LAST_IRQ_FRAME_STOP=0,
    TILE_LAST_IRQ_LINE_END,
    TILE_LAST_IRQ_PER_TILE,
    TILE_LAST_IRQ_MODE_MAX_NO
}TILE_LAST_IRQ_MODE_ENUM;

/* MAX TILE WIDTH & HEIGHT */
#define MAX_SIZE (65536)

/* TILE HORIZONTAL BUFFER */
#define MAX_TILE_BACKUP_HORZ_NO (8)

/* Tile edge */
#define TILE_EDGE_BOTTOM_MASK (0x8)
#define TILE_EDGE_TOP_MASK (0x4)
#define TILE_EDGE_RIGHT_MASK (0x2)
#define TILE_EDGE_LEFT_MASK (0x1)
#define TILE_EDGE_HORZ_MASK (TILE_EDGE_RIGHT_MASK + TILE_EDGE_LEFT_MASK)

typedef enum TILE_FUNC_ID_ENUM
{
    LAST_MODULE_ID_OF_START = (0xFFFFFFF),
    NULL_TILE_ID  = (0xFFFFFFF),
    /* isp func id */
    TILE_FUNC_IMGI_ID = (1001),
    TILE_FUNC_UNPACK_ID = (98),
    TILE_FUNC_BPC_2D_ID = (202),
    TILE_FUNC_LSC_ID = (45),
    TILE_FUNC_422_444_ID = (92),
    TILE_FUNC_CFA_ID = (58),
    TILE_FUNC_LCE_ID = (207),
    TILE_FUNC_444_422_ID = (51),
    TILE_FUNC_NBC_ID = (209),
    TILE_FUNC_SEEE_ID = (211),
    TILE_FUNC_SL2_ID = (213),
    TILE_FUNC_CDRZ_ID = (276),
    TILE_FUNC_IMGO_ID = (1021),
    TILE_FUNC_IMG2O_ID = (1028),
    TILE_FUNC_RSP_ID = (292),
    TILE_FUNC_MDP_CROP_ID = (296),
    TILE_FUNC_XM1_ID = (7001),
    TILE_FUNC_XM2_ID = (7002),
    TILE_FUNC_XM3_ID = (7003),
    TILE_FUNC_XM4_ID = (7004),
    /* sub RDMA */
    TILE_FUNC_LSCI_ID = (1006),
    TILE_FUNC_LCEI_ID = (1005),
    /* mdp func ids */
    MDP_TILE_FUNC_ID_ENUM_DECLARE
}TILE_FUNC_ID_ENUM;

typedef enum TILE_GROUP_NUM_ENUM
{
	TILE_ISP_GROUP_NUM = 0,
    TILE_MDP_GROUP_NUM,
	TILE_GROUP_TOTAL_NUM
}TILE_GROUP_NUM_ENUM;

typedef enum TILE_RUN_MODE_ENUM
{
	TILE_RUN_MODE_SUB_OUT = 0x1,
	TILE_RUN_MODE_SUB_IN = TILE_RUN_MODE_SUB_OUT + 0x2,
	TILE_RUN_MODE_MAIN = TILE_RUN_MODE_SUB_IN + 0x4
}TILE_RUN_MODE_ENUM;

/* SCENARIO */
#define TILE_SCENARIO_IC (0)
#define TILE_SCENARIO_VR (1)
#define TILE_SCENARIO_ZSD (2)
#define TILE_SCENARIO_IP (3)
#define TILE_SCENARIO_VSS (6)

/* SUBMODE */
#define TILE_SUBMODE_IC_RAW (0)
#define TILE_SUBMODE_IC_YUV (1)
#define TILE_SUBMODE_IC_RGB (2)
#define TILE_SUBMODE_IC_JPG (3)
#define TILE_SUBMODE_VR_RAW (0)
#define TILE_SUBMODE_VR_YUV (1)
#define TILE_SUBMODE_VR_RGB (2)
#define TILE_SUBMODE_ZSD_RAW (0)
#define TILE_SUBMODE_ZSD_YUV (1)
#define TILE_SUBMODE_ZSD_RGB (2)
#define TILE_SUBMODE_IP_RAW (0)
#define TILE_SUBMODE_IP_YUV (1)
#define TILE_SUBMODE_IP_RGB (2)
#define TILE_SUBMODE_VSS_RAW (0)
#define TILE_SUBMODE_VSS_YUV (1)
/* YUV format */
#define CAM_IN_FMT_YUV420_2 (0)
#define CAM_IN_FMT_YUV420_3 (1)
#define CAM_IN_FMT_YUV422_1 (2)
#define CAM_IN_FMT_YUV422_2 (3)
#define CAM_IN_FMT_YUV422_3 (4)
/* RAW format */
#define CAM_IN_FMT_BAYER_8  (0)
#define CAM_IN_FMT_BAYER_10 (1)
#define CAM_IN_FMT_BAYER_12 (2)
/* RGB format */
#define CAM_IN_FMT_RGB565   (0)
#define CAM_IN_FMT_RGB888   (1)
#define CAM_IN_FMT_XRGB8888 (2)
/* LCE power no */
#define CAM_LCE_FRAC_NO (15)
#define CAM_LCE_FRAC_MASK (0x7FFF)
/* resizer prec bits */
#define TILE_RESIZER_48T_PREC_VAL (1<<15)
#define TILE_RESIZER_ACC_PREC_VAL (1<<20)
#define TILE_RESIZER_48T_PREC_BITS (15)
#define TILE_RESIZER_ACC_PREC_BITS (20)
/* resizer direction flag */
#define CAM_DIR_X (0)
#define CAM_DIR_Y (1)
/* resizer align flag */
#define CAM_UV_422_FLAG (0)
#define CAM_UV_444_FLAG (1)
/* error enum */
#define ERROR_MESSAGE_DATA(n, CMD) \
    CMD(n, ISP_MESSAGE_OK, ISP_TPIPE_MESSAGE_OK)\
    CMD(n, ISP_MESSAGE_UNKNOWN_DRIVER_CONFIGURED_REG_MODE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_DIFFERENT_TILE_CONFIG_NO_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_OVER_MAX_TILE_WORD_NO_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_OVER_MAX_TILE_TOT_NO_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_UNDER_MIN_TILE_FUNC_NO_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_OVER_MAX_TILE_FUNC_NO_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_OVER_MAX_TILE_FUNC_EN_NO_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_OVER_MAX_TILE_FUNC_PREV_NO_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_OVER_MAX_TILE_FUNC_SUBRDMA_NO_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_NOT_FOUND_INIT_TILE_FUNC_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_NOT_FOUND_FOR_TILE_FUNC_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_NOT_FOUND_BACK_TILE_FUNC_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_NOT_FOUND_TDR_TILE_FUNC_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_NOT_FOUND_ENABLE_TILE_FUNC_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_NOT_FOUND_SUB_RDMA_TDR_FUNC_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* tile dump check */\
    CMD(n, ISP_MESSAGE_DUPLICATED_TDR_DUMP_MASK_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_DUPLICATED_SUPPORT_FUNC_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_DUPLICATED_FUNC_EN_FOUND_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_DUPLICATED_FUNC_DISABLE_OUTPUT_FOUND_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_DUPLICATED_SUB_RDMA_FUNC_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_OVER_MAX_BRANCH_NO_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_OVER_MAX_INPUT_TILE_FUNC_NO_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TILE_FUNC_CANNOT_FIND_LAST_FUNC_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_SCHEDULING_BACKWARD_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_SCHEDULING_FORWARD_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_IN_CONST_X_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_IN_CONST_Y_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_OUT_CONST_X_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_OUT_CONST_Y_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_NULL_BACK_COMP_TILE_LOSS_TYPE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_NULL_FOR_COMP_TILE_LOSS_TYPE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_NULL_INIT_PTR_FOR_START_FUNC_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_INIT_INCORRECT_X_INPUT_SIZE_POS_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_INIT_INCORRECT_Y_INPUT_SIZE_POS_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_INIT_INCORRECT_X_OUTPUT_SIZE_POS_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_INIT_INCORRECT_Y_OUTPUT_SIZE_POS_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TILE_X_DIR_NOT_END_TOGETHER_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TILE_Y_DIR_NOT_END_TOGETHER_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_INCORRECT_XE_INPUT_POS_REDUCED_BY_TILE_SIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_INCORRECT_YE_INPUT_POS_REDUCED_BY_TILE_SIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_FORWARD_FUNC_CAL_LOOP_COUNT_OVER_MAX_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TILE_LOSS_OVER_TILE_HEIGHT_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TILE_LOSS_OVER_TILE_WIDTH_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TILE_HEIGHT_EQ_ONE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TILE_WIDTH_EQ_ONE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TILE_OUTPUT_HORIZONTAL_OVERLAP_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TILE_OUTPUT_VERTICAL_OVERLAP_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TP8_FOR_INVALID_OUT_XYS_XYE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TP6_FOR_INVALID_OUT_XYS_XYE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TP4_FOR_INVALID_OUT_XYS_XYE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TP2_FOR_INVALID_OUT_XYS_XYE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_SRC_ACC_FOR_INVALID_OUT_XYS_XYE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_CUB_ACC_FOR_INVALID_OUT_XYS_XYE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_BACKWARD_START_LESS_THAN_FORWARD_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_NOT_SUPPORT_RESIZER_MODE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_RECURSIVE_FOUND_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_CHECK_IN_CONFIG_ALIGN_XS_POS_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_CHECK_IN_CONFIG_ALIGN_XE_POS_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_CHECK_IN_CONFIG_ALIGN_YS_POS_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_CHECK_IN_CONFIG_ALIGN_YE_POS_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_XSIZE_NOT_DIV_BY_IN_CONST_X_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_YSIZE_NOT_DIV_BY_IN_CONST_Y_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_XSIZE_NOT_DIV_BY_OUT_CONST_X_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_YSIZE_NOT_DIV_BY_OUT_CONST_Y_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_START_FUNC_XS_NOT_DIV_BY_IN_CONST_X_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_START_FUNC_YS_NOT_DIV_BY_IN_CONST_Y_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_END_FUNC_XE_NOT_DIV_BY_OUT_CONST_X_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_END_FUNC_YE_NOT_DIV_BY_OUT_CONST_Y_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TILE_FORWARD_OUT_OVER_TILE_WIDTH_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TILE_FORWARD_OUT_OVER_TILE_HEIGHT_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TILE_BACKWARD_IN_OVER_TILE_WIDTH_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TILE_BACKWARD_IN_OVER_TILE_HEIGHT_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_FORWARD_CHECK_TOP_EDGE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_FORWARD_CHECK_BOTTOM_EDGE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_FORWARD_CHECK_LEFT_EDGE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_FORWARD_CHECK_RIGHT_EDGE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_BACKWARD_CHECK_TOP_EDGE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_BACKWARD_CHECK_BOTTOM_EDGE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_BACKWARD_CHECK_LEFT_EDGE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_BACKWARD_CHECK_RIGHT_EDGE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TDR_CHECK_NO_MODULE_EDGE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_CHECK_OUT_CONFIG_ALIGN_XS_POS_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_CHECK_OUT_CONFIG_ALIGN_XE_POS_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_CHECK_OUT_CONFIG_ALIGN_YS_POS_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_CHECK_OUT_CONFIG_ALIGN_YE_POS_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TILE_HEIGHT_ANR_II_MODE_CHECK_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_UNKNOWN_RESIZER_DIR_MODE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_DISABLE_FUNC_X_SIZE_CHECK_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_DISABLE_FUNC_Y_SIZE_CHECK_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_OUTPUT_DISABLE_INPUT_FUNC_CHECK_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_RESIZER_SRC_ACC_SCALING_UP_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_RESIZER_CUBIC_ACC_SCALING_UP_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TDR_ISP_EDGE_DIFFERENT_TB_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TDR_ISP_EDGE_DIFFERENT_LR_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TDR_CDP_EDGE_DIFFERENT_TB_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TDR_CDP_EDGE_DIFFERENT_LR_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TDR_WDMA_EDGE_DIFFERENT_TB_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TDR_WDMA_EDGE_DIFFERENT_LR_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_RDMA_MODULE_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_INCORRECT_END_FUNC_TYPE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_INCORRECT_START_FUNC_TYPE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* verification */\
    CMD(n, ISP_MESSAGE_VERIFY_4_8_TAPES_XS_OUT_INCONSISTENCE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_VERIFY_4_8_TAPES_XE_OUT_INCONSISTENCE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_VERIFY_4_8_TAPES_YS_OUT_INCONSISTENCE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_VERIFY_4_8_TAPES_YE_OUT_INCONSISTENCE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_VERIFY_CUBIC_ACC_XS_OUT_INCONSISTENCE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_VERIFY_CUBIC_ACC_XE_OUT_INCONSISTENCE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_VERIFY_CUBIC_ACC_YS_OUT_INCONSISTENCE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_VERIFY_CUBIC_ACC_YE_OUT_INCONSISTENCE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_VERIFY_SRC_ACC_XS_OUT_INCONSISTENCE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_VERIFY_SRC_ACC_XE_OUT_INCONSISTENCE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_VERIFY_SRC_ACC_YS_OUT_INCONSISTENCE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_VERIFY_SRC_ACC_YE_OUT_INCONSISTENCE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_VERIFY_BACKWARD_XS_LESS_THAN_FORWARD_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_VERIFY_FORWARD_XE_LESS_THAN_BACKWARD_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_VERIFY_BACKWARD_YS_LESS_THAN_FORWARD_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_VERIFY_FORWARD_YE_LESS_THAN_BACKWARD_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_VERIFY_FORWARD_OFFSET_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_VERIFY_FORWARD_BIAS_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* dump c model hex */\
    CMD(n, ISP_MESSAGE_TILE_CONFIG_EN_FILE_OPEN_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TILE_CONFIG_MAP_FILE_OPEN_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* tile mode control */\
    CMD(n, ISP_MESSAGE_TILE_MODE_NO_OVER_MAX_COUNT_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TILE_MODE_UNKNOWN_ENUM_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TILE_MODE_NO_DUPLICATED_INIT_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TILE_MODE_OUTPUT_FILE_COPY_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TILE_MODE_OUTPUT_FILE_CMP_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TILE_MODE_OUTPUT_FILE_DEL_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* tile constraint */\
    CMD(n, ISP_MESSAGE_DEBUG_PRINT_FILE_OPEN_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* tile platform */\
    CMD(n, ISP_MESSAGE_TILE_PLATFORM_NULL_INPUT_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TILE_PLATFORM_NULL_WORKING_BUFFER_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TILE_PLATFORM_LESS_WORKING_BUFFER_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TILE_NULL_PTR_COMP_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TILE_ISP_HEX_DUMP_COMP_DIFF_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TILE_MDP_HEX_DUMP_COMP_DIFF_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TILE_REG_MAP_COMP_DIFF_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TILE_PLATFORM_RETURN_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TILE_NULL_MEM_PTR_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_ISP_DESCRIPTOR_PTR_NON_4_BYTES_ALGIN_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_WORKING_BUFFER_PTR_NON_4_BYTES_ALGIN_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_WORKING_BUFFER_SIZE_NON_4_BYTES_ALGIN_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
	/* tdr inverse */\
    CMD(n, ISP_MESSAGE_TDR_INV_TILE_NO_OVER_MAX_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TDR_INV_DIFFERENT_TILE_CONFIG_NO_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TDR_INV_NULL_PTR_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
	/* tile ut */\
    CMD(n, ISP_MESSAGE_TILE_UT_FILE_OPEN_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* resizer coeff check */\
    CMD(n, ISP_MESSAGE_CDRZ_UNMATCH_INPUT_WIDTH_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_CDRZ_UNMATCH_INPUT_HEIGHT_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* tile verification c model */\
    CMD(n, ISP_MESSAGE_OVER_MAX_DIR_SIZE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* last irq check */\
    CMD(n, ISP_MESSAGE_TDR_LAST_IRQ_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
	/* func ptr check */\
    CMD(n, ISP_MESSAGE_UNMATCH_INIT_FUNC_PTR_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_UNMATCH_FOR_FUNC_PTR_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_UNMATCH_BACK_FUNC_PTR_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_UNMATCH_TDR_FUNC_PTR_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_NON_INIT_FRAME_RESULT_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
	/* fw cdp run check */\
    CMD(n, ISP_MESSAGE_CDP_FW_RUN_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
	/* tdr sort check */\
    CMD(n, ISP_MESSAGE_INCORRECT_ORDER_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_LAST_IRQ_NOT_SUPPORT_TDR_SORT_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TDR_SORT_OVER_MAX_H_TILE_NO_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TDR_SORT_NON_4_BYTES_TILE_INFO_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
	/* tdr edge group check */\
    CMD(n, ISP_MESSAGE_INCORRECT_TDR_EDGE_GROUP_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
	/* tile driver error check */\
    CMD(n, ISP_MESSAGE_OVER_MAX_ID_BUFFER_NO_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_MEM_DUMP_PARSE_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_DONT_CARE_END_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
	/* multi-input flow error check */\
    CMD(n, ISP_MESSAGE_TWO_MAIN_PREV_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TWO_START_PREV_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_NO_MAIN_OUTPUT_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_DIFF_PREV_CONFIG_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_TWO_MAIN_START_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_DIFF_PREV_FORWARD_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_FOR_BACK_COMP_X_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_FOR_BACK_COMP_Y_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, ISP_MESSAGE_BROKEN_SUB_PATH_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* dpframework check */\
    CMD(n, ISP_MESSAGE_DPFRAMEWORK_UNKNOWN_ENUM_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    /* ISP ERROR MESSAGE DATA */\
    ISP_TILE_ERROR_MESSAGE_ENUM(n, CMD)\
    /* MDP ERROR MESSAGE DATA */\
    MDP_TILE_ERROR_MESSAGE_ENUM(n, CMD)\
    /* final count, can not be changed */\
    CMD(n, ISP_MESSAGE_MAX_NO, ISP_TPIPE_MESSAGE_MAX_NO)

#define ISP_ENUM_DECLARE(n, a, b) a,
#define ISP_ENUM_GET_PLATFORM_ERROR_NO(n, a, b) if ((a) == (n)) return b;
#define ISP_ENUM_STRING(n, a, b) if ((a) == n) return #a;

/* to prevent from directly calling macro */
#define GET_ERROR_NAME(n) \
    if (0 == (n)) return "ISP_MESSAGE_UNKNOWN";\
    ERROR_MESSAGE_DATA(n, ISP_ENUM_STRING)\
	return "";

/* to prevent from directly calling macro */
#define GET_ISP_TILE_ERROR_NO(n) \
    if (0 == (n)) return ISP_TPIPE_MESSAGE_UNKNOWN;\
    ERROR_MESSAGE_DATA(n, ISP_ENUM_GET_PLATFORM_ERROR_NO)\
    return ISP_TPIPE_MESSAGE_UNKNOWN;

/* error enum */
typedef enum ISP_MESSAGE_ENUM
{
    ISP_MESSAGE_UNKNOWN=0,
    ERROR_MESSAGE_DATA(,ISP_ENUM_DECLARE)
}ISP_MESSAGE_ENUM;

/* error enum */
typedef enum TILE_RESIZER_MODE_ENUM
{
    TILE_RESIZER_MODE_UNKNOWN=0,//0
    TILE_RESIZER_MODE_4_TAPES,
    TILE_RESIZER_MODE_SRC_ACC,
    TILE_RESIZER_MODE_CUBIC_ACC,
    TILE_RESIZER_MODE_8_TAPES,
    TILE_RESIZER_MODE_6_TAPES,
    TILE_RESIZER_MODE_2_TAPES,
    TILE_RESIZER_MODE_MAX_NO
}TILE_RESIZER_MODE_ENUM;

/* Func ptr flag */
#define TILE_INIT_FUNC_PTR_FLAG (0x1)
#define TILE_FOR_FUNC_PTR_FLAG (0x2)
#define TILE_BACK_FUNC_PTR_FLAG (0x4)
#define TILE_TDR_FUNC_PTR_FLAG (0x8)

/* a, b, c, d, e reserved */
/* data type */
/* register name of current c model */
/* reserved */
/* value mask */
/* array bracket [] */
/* S: c model variables, U: unmasked variable, M: masked variable */
#define TILE_FUNC_BLOCK_LUT(CMD, a, b, c, d, e) \
    CMD(a, b, c, d, e, TILE_FUNC_ID_ENUM, func_num,,,, S,,)\
    CMD(a, b, c, d, e, TILE_RUN_MODE_ENUM, run_mode,,,, S,,)\
    CMD(a, b, c, d, e, unsigned int, in_stream_order,,,, S,,)\
    CMD(a, b, c, d, e, unsigned int, out_stream_order,,,, S,,)\
    CMD(a, b, c, d, e, unsigned int, in_cal_order,,,, S,,)\
    CMD(a, b, c, d, e, unsigned int, out_cal_order,,,, S,,)\
    CMD(a, b, c, d, e, unsigned int, in_dump_order,,,, S,,)\
    CMD(a, b, c, d, e, unsigned int, out_dump_order,,,, S,,)\
    CMD(a, b, c, d, e, int, in_pos_xs,,,, U,,)\
    CMD(a, b, c, d, e, int, in_pos_xe,,,, U,,)\
    CMD(a, b, c, d, e, int, full_size_x_in,,,, S,,)\
    CMD(a, b, c, d, e, int, min_in_pos_xs,,,, S,,)\
    CMD(a, b, c, d, e, int, max_in_pos_xe,,,, S,,)\
    CMD(a, b, c, d, e, int, out_pos_xs,,,, U,,)\
    CMD(a, b, c, d, e, int, out_pos_xe,,,, U,,)\
    CMD(a, b, c, d, e, int, full_size_x_out,,,, S,,)\
    CMD(a, b, c, d, e, int, in_pos_ys,,,, S,,)\
    CMD(a, b, c, d, e, int, in_pos_ye,,,, S,,)\
    CMD(a, b, c, d, e, int, full_size_y_in,,,, S,,)\
    CMD(a, b, c, d, e, int, max_in_pos_ye,,,, S,,)\
    CMD(a, b, c, d, e, int, out_pos_ys,,,, S,,)\
    CMD(a, b, c, d, e, int, out_pos_ye,,,, S,,)\
    CMD(a, b, c, d, e, int, full_size_y_out,,,, S,,)\
    CMD(a, b, c, d, e, bool, enable_flag,,,, S,,)\
    CMD(a, b, c, d, e, bool, output_disable_flag,,,, S,,)\
    CMD(a, b, c, d, e, bool, max_h_edge_flag,,,, U,,)\
    CMD(a, b, c, d, e, bool, tdr_h_disable_flag,,,, U,,)/* diff view cal with backup, to reset */\
    CMD(a, b, c, d, e, bool, h_end_flag,,,, U,,)/* new backup */\
    CMD(a, b, c, d, e, bool, max_v_edge_flag,,,, S,,)\
    CMD(a, b, c, d, e, bool, tdr_v_disable_flag,,,, S,,)/* diff view cal, to reset */\
    CMD(a, b, c, d, e, bool, v_end_flag,,,, S,,)\
    CMD(a, b, c, d, e, int, in_tile_width,,,, S,,)\
    CMD(a, b, c, d, e, int, out_tile_width,,,, S,,)\
    CMD(a, b, c, d, e, int, in_tile_height,,,, S,,)\
    CMD(a, b, c, d, e, int, out_tile_height,,,, S,,)\
    CMD(a, b, c, d, e, bool, backward_h_end_flag,,,, S,,)\
    CMD(a, b, c, d, e, bool, backward_v_end_flag,,,, S,,)\
    CMD(a, b, c, d, e, int, backward_input_xs_pos,,,, S,,)\
    CMD(a, b, c, d, e, int, backward_input_xe_pos,,,, S,,)\
    CMD(a, b, c, d, e, int, backward_output_xs_pos,,,, S,,)\
    CMD(a, b, c, d, e, int, backward_output_xe_pos,,,, S,,)\
    CMD(a, b, c, d, e, int, backward_input_ys_pos,,,, S,,)\
    CMD(a, b, c, d, e, int, backward_input_ye_pos,,,, S,,)\
    CMD(a, b, c, d, e, int, backward_output_ys_pos,,,, S,,)\
    CMD(a, b, c, d, e, int, backward_output_ye_pos,,,, S,,)\
    CMD(a, b, c, d, e, int, last_input_xs_pos,,,, S,,)\
    CMD(a, b, c, d, e, int, last_input_xe_pos,,,, S,,)\
    CMD(a, b, c, d, e, int, last_output_xs_pos,,,, S,,)\
    CMD(a, b, c, d, e, int, last_output_xe_pos,,,, S,,)\
    CMD(a, b, c, d, e, int, last_input_ys_pos,,,, S,,)\
    CMD(a, b, c, d, e, int, last_input_ye_pos,,,, S,,)\
    CMD(a, b, c, d, e, int, last_output_ys_pos,,,, S,,)\
    CMD(a, b, c, d, e, int, last_output_ye_pos,,,, S,,)\
    CMD(a, b, c, d, e, int, bias_x,,,, U,,)\
    CMD(a, b, c, d, e, int, offset_x,,,, U,,)\
    CMD(a, b, c, d, e, int, bias_x_c,,,, U,,)\
    CMD(a, b, c, d, e, int, offset_x_c,,,, U,,)\
    CMD(a, b, c, d, e, int, bias_y,,,, S,,)\
    CMD(a, b, c, d, e, int, offset_y,,,, S,,)\
    CMD(a, b, c, d, e, int, bias_y_c,,,, S,,)\
    CMD(a, b, c, d, e, int, offset_y_c,,,, S,,)\
    CMD(a, b, c, d, e, unsigned char, l_tile_loss,,,, S,,)\
    CMD(a, b, c, d, e, unsigned char, r_tile_loss,,,, S,,)\
    CMD(a, b, c, d, e, unsigned char, t_tile_loss,,,, S,,)\
    CMD(a, b, c, d, e, unsigned char, b_tile_loss,,,, S,,)\
    CMD(a, b, c, d, e, unsigned char, tdr_edge,, TILE_EDGE_HORZ_MASK,, M,,)\
    CMD(a, b, c, d, e, TILE_TDR_EDGE_GROUP_ENUM, tdr_group,,,, S,,)\
    CMD(a, b, c, d, e, unsigned char, prev_blk_num,,, [MAX_TILE_PREV_NO], S,,)\
    CMD(a, b, c, d, e, unsigned char, tot_prev_num,,,, S,,)\
    CMD(a, b, c, d, e, unsigned char, next_blk_num,,, [MAX_TILE_BRANCH_NO], S,,)\
    CMD(a, b, c, d, e, unsigned char, tot_branch_num,,,, S,,)\
    CMD(a, b, c, d, e, unsigned char, in_const_x,,,, S,,)\
    CMD(a, b, c, d, e, unsigned char, in_const_y,,,, S,,)\
    CMD(a, b, c, d, e, unsigned char, out_const_x,,,, S,,)\
    CMD(a, b, c, d, e, unsigned char, out_const_y,,,, S,,)\
    CMD(a, b, c, d, e, int, crop_bias_x,,,, S,,)\
    CMD(a, b, c, d, e, int, crop_offset_x,,,, S,,)\
    CMD(a, b, c, d, e, int, crop_bias_y,,,, S,,)\
    CMD(a, b, c, d, e, int, crop_offset_y,,,, S,,)\
    CMD(a, b, c, d, e, int, in_tile_width_backup,,,, S,,)\
    CMD(a, b, c, d, e, int, out_tile_width_backup,,,, S,,)\
    CMD(a, b, c, d, e, int, in_tile_height_backup,,,, S,,)\
    CMD(a, b, c, d, e, int, out_tile_height_backup,,,, S,,)\
    CMD(a, b, c, d, e, TILE_FUNC_ID_ENUM, last_func_num[MAX_TILE_PREV_NO],,,, S,,)\
    CMD(a, b, c, d, e, TILE_FUNC_ID_ENUM, next_func_num[MAX_TILE_BRANCH_NO],,,, S,,)\
    CMD(a, b, c, d, e, unsigned char, type,,,, S,,)\
    CMD(a, b, c, d, e, TILE_GROUP_NUM_ENUM, group_num,,,, S,,)\
    CMD(a, b, c, d, e, TILE_RESIZER_BACKWARD_CAL_ARG_STRUCT, back_arg,,,, S,,)\
    CMD(a, b, c, d, e, TILE_RESIZER_FORWARD_CAL_ARG_STRUCT, for_arg,,,, S,,)\
    CMD(a, b, c, d, e, TILE_RESIZER_PARA_BUFFER, horz_para,,, [MAX_TILE_BACKUP_HORZ_NO], S,,)\
    CMD(a, b, c, d, e, unsigned int, func_ptr_flag,,,, S,,)\

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
#define TILE_SW_REG_LUT(LABEL, CMD, a, b, c, d, e) \
	LABEL##_TILE_SW_REG_LUT(CMD, a, b, c, d, e)

/* register table (Cmodel, platform, tile driver) for HW parameters */
/* a, b, c, d, e reserved */
/* data type */
/* register name of current c model */
/* register name of HW ISP & platform parameters */
/* internal variable name of tile */
/* array bracket [xx] */
/* valid condition by tdr_en to print platform log with must string, default: false */
#define TILE_HW_REG_LUT(LABEL, CMD, a, b, c, d, e) \
	LABEL##_TILE_HW_REG_LUT(CMD, a, b, c, d, e)

/* register table (Cmodel, , tile driver) for Cmodel only parameters */
/* a, b, c, d, e reserved */
/* data type */
/* register name of current c model */
/* register name of HW ISP & platform parameters */
/* internal variable name of tile */
/* array bracket [xx] */
/* reserved */
#define TILE_CMODEL_PATH_LUT(LABEL, CMD, a, b, c, d, e) \
	LABEL##_TILE_CMODEL_PATH_LUT(CMD, a, b, c, d, e)

/* register table (Cmodel, , tile driver) for Cmodel only parameters */
/* a, b, c, d, e reserved */
/* data type */
/* register name of current c model */
/* register name of HW ISP & platform parameters */
/* internal variable name of tile */
/* array bracket [xx] */
/* reserved */
#define TILE_CMODEL_REG_LUT(LABEL, CMD, a, b, c, d, e) \
	LABEL##_TILE_CMODEL_REG_LUT(CMD, a, b, c, d, e)

/* register table ( , platform, tile driver) for Platform only parameters */
/* a, b, c, e reserved */
/* data type */
/* register name of current c model */
/* register name of HW ISP & platform parameters */
/* internal variable name of tile */
/* array bracket [xx] */
/* valid condition by tdr_en to print platform log with must string, default: false */
#define TILE_PLATFORM_REG_LUT(LABEL, CMD, a, b, c, d, e) \
    LABEL##_TILE_PLATFORM_REG_LUT(CMD, a, b, c, d, e)

/* register table ( , , tile driver) for tile driver only parameters */
/* a, b, c, d, e reserved */
/* data type */
/* register name of current c model */
/* register name of HW ISP & platform parameters */
/* internal variable name of tile */
/* array bracket [xx] */
/* reserved */
#define TILE_PLATFORM_DEBUG_REG_LUT(LABEL, CMD, a, b, c, d, e) \
	LABEL##_TILE_PLATFORM_DEBUG_REG_LUT(CMD, a, b, c, d, e)

/* register table ( , , tile driver) for tile driver only parameters */
/* a, b, c, d, e reserved */
/* data type */
/* register name of current c model */
/* register name of HW ISP & platform parameters */
/* internal variable name of tile */
/* array bracket [xx] */
/* reserved */
#define TILE_INTERNAL_TDR_REG_LUT(LABEL, CMD, a, b, c, d, e) \
	LABEL##_TILE_INTERNAL_TDR_REG_LUT(CMD, a, b, c, d, e)

/* register table ( , , tile driver) for tile driver only parameters */
/* a, b, c, d, e reserved */
/* data type */
/* register name of current c model */
/* register name of HW ISP & platform parameters */
/* internal variable name of tile */
/* array bracket [xx] */
/* reserved */

#define COMMON_TILE_INTERNAL_REG_LUT(CMD, a, b, c, d, e) \
    /* run mode */\
    CMD(a, b, c, d, e, int,,, run_mode,,,,)\
    /* frame mode flag */\
    CMD(a, b, c, d, e, int,,, first_frame,,,,)/* first frame to run frame mode */\
    CMD(a, b, c, d, e, bool,,, found_sub_in,,,,)\
    CMD(a, b, c, d, e, bool,,, found_sub_out,,,,)\
    /* tile cal & dump order flag */\
    CMD(a, b, c, d, e, unsigned int,,, src_stream_order,,,,)/* copy to RDMA in_stream_order */\
    CMD(a, b, c, d, e, unsigned int,,, src_cal_order,,,,)/* copy to RDMA in_cal_order */\
    CMD(a, b, c, d, e, unsigned int,,, src_dump_order,,,,)/* copy to RDMA in_dump_order */\
    /* special mode for diff view angle by vss mode */\
    CMD(a, b, c, d, e, bool,,, skip_tile_mode,,,,)\
    /* first func no */\
    CMD(a, b, c, d, e, int,,, first_func_en_no,,,,)\
    /* last func no */\
    CMD(a, b, c, d, e, int,,, last_func_en_no,,,,)\
    /* Internal */\
    CMD(a, b, c, d, e, bool,,, skip_x_cal,,,,)\
    CMD(a, b, c, d, e, bool,,, skip_y_cal,,,,)\
    CMD(a, b, c, d, e, bool,,, backup_x_skip_y,,,,)\
    /* debug mode with invalid offset to enable recursive forward*/\
    CMD(a, b, c, d, e, bool,,, recursive_forward_en,,,,)\
    /* max tile width */\
    CMD(a, b, c, d, e, int,,, max_log_tile_width,,,,)\
    /* max tile height */\
    CMD(a, b, c, d, e, int,,, max_log_tile_height,,,,)\
    /* backup tile width */\
    CMD(a, b, c, d, e, int,,, backup_isp_tile_width,,,,)\
    /* backup tile height */\
    CMD(a, b, c, d, e, int,,, backup_isp_tile_height,,,,)\
    /* backup tile width */\
    CMD(a, b, c, d, e, int,,, backup_mdp_tile_width,,,,)\
    /* backup tile height */\
    CMD(a, b, c, d, e, int,,, backup_mdp_tile_height,,,,)\
    /* max log input width */\
    CMD(a, b, c, d, e, int,,, max_log_input_width,,,,)\
    /* max log input height */\
    CMD(a, b, c, d, e, int,,, max_log_input_height,,,,)\
    /* max input width */\
    CMD(a, b, c, d, e, int,,, max_input_width,,,,)\
    /* max input height */\
    CMD(a, b, c, d, e, int,,, max_input_height,,,,)\
    /* backup input width */\
    CMD(a, b, c, d, e, int,,, backup_max_input_width,,,,)\
    /* backup input height */\
    CMD(a, b, c, d, e, int,,, backup_max_input_height,,,,)\
    /* max log output width */\
    CMD(a, b, c, d, e, int,,, max_log_output_width,,,,)\
    /* max log output height */\
    CMD(a, b, c, d, e, int,,, max_log_output_height,,,,)\
    /* used_word_no */\
    CMD(a, b, c, d, e, int,,, isp_used_word_no,,,,)\
    /* used_word_no */\
    CMD(a, b, c, d, e, int,,, mdp_used_word_no,,,,)\
    /* config_no_per_tile */\
    CMD(a, b, c, d, e, int,,, isp_config_no_per_tile,,,,)\
    /* config_no_per_tile */\
    CMD(a, b, c, d, e, int,,, mdp_config_no_per_tile,,,,)\
    /* vertical_tile_no */\
    CMD(a, b, c, d, e, int,,, curr_vertical_tile_no,,,,)\
    /* horizontal_tile_no */\
    CMD(a, b, c, d, e, int,,, horizontal_tile_no,,,,)\
    /* curr_horizontal_tile_no */\
    CMD(a, b, c, d, e, int,,, curr_horizontal_tile_no,,,,)\
    /* used_tile_no */\
    CMD(a, b, c, d, e, int,,, used_tile_no,,,,)\
    /* Tile IRQ */\
    CMD(a, b, c, d, e, int,,, TDR_EDGE,,,,)\
    CMD(a, b, c, d, e, int,,, CDP_TDR_EDGE,,,,)\
    CMD(a, b, c, d, e, int,,, TILE_IRQ,,,,)\
    CMD(a, b, c, d, e, int,,, LAST_IRQ,,,,)\

#define TILE_INTERNAL_REG_LUT(LABEL, CMD, a, b, c, d, e) \
	LABEL##_TILE_INTERNAL_REG_LUT(CMD, a, b, c, d, e)

#define MODE_CMP_EQ(ptr, scenario_val, mode_val) ((scenario_val == (ptr)->SCENARIO) && (mode_val == (ptr)->MODE))
#define MODE_CMP_NOT_EQ(ptr, scenario_val, mode_val) ((scenario_val != (ptr)->SCENARIO) || (mode_val != (ptr)->MODE))
#define REG_CMP_EQ(ptr, reg, val) (val == (ptr)->reg)
#define REG_CMP_NOT_EQ(ptr, reg, val) (val != (ptr)->reg)
#define REG_CMP_LE(ptr, reg, val) ((ptr)->reg <= val)
#define REG_CMP_GE(ptr, reg, val) ((ptr)->reg >= val)
#define REG_CHECK_EN(ptr, reg) (1 == (ptr)->reg)
#define REG_CHECK_DISABLED(ptr, reg) (0 == (ptr)->reg)
#define REG_CMP_AND(ptr, reg, val)  (val == ((ptr)->reg & val))

/* a, b reserved */
/* c: ptr of tile_reg_map, reserved */
/* tile module scenario/submode reg */
/* function id, same one must put together for macro code to check */
/* function name */
/* last function id */
/* last function name */
#define TILE_SUPPORT_FUN_LUT(LABEL, CMD, a, b, c, d, e, f) \
	LABEL##_TILE_SUPPORT_FUN_LUT(CMD, a, b, c, d, e, f)

/* Sub RDMA func support lut */
/* a, c, d, e reserved */
/* b: reg_map ptr reserved */
/* valid condition */
/* RDMA id, same one must put together for macro code to check */
/* RDMA func name */
/* property clone func id */
/* property clone fun name */
#define TILE_SUB_RDMA_FUNC_SUPPORT_LUT(LABEL, CMD, a, b, c, d, e) \
	LABEL##_TILE_SUB_RDMA_FUNC_SUPPORT_LUT(CMD, a, b, c, d, e)

/* enable register LUT */
/* a, b, c, d reserved */
/* d: ptr of tile_reg_map, reserved */
/* function id */
/* function name */
/* tile module enable reg */
#define TILE_FUNC_ENABLE_LUT(LABEL, CMD, a, b, c, d, e) \
	LABEL##_TILE_FUNC_ENABLE_LUT(CMD, a, b, c, d, e)

/* output disable register LUT */
/* a, b, c, d reserved */
/* d: ptr of tile_reg_map, reserved */
/* function id */
/* function name */
/* tile module output disable reg */
#define TILE_FUNC_OUTPUT_DISABLE_LUT(LABEL, CMD, a, b, c, d, e) \
    LABEL##_TILE_FUNC_OUTPUT_DISABLE_LUT(CMD, a, b, c, d, e)

/* a, b c, d, e reserved */
/* function id */
/* function name */
/* tile type: 0x1 non-fixed func to configure, 0x2 rdma, 0x4 wdma, 0x8 crop_en */
/* tile group, 0: ISP group, 1: CDP group 2: resizer with offset & crop */
/* tile group except for 2 will restrict last end < current end (to ensure WDMA end at same time) */
/* tile loss, l_loss, r_loss, t_loss, b_loss, in_x, int_y, out_x, out_y */
/* init function name, default NULL */
/* forward function name, default NULL */
/* back function name, default NULL */
/* calculate tile reg function name, default NULL */
/* input tile constraint, 0: no check, 1: to clip when enabled */
/* output tile constraint, 0: no check, 1: to clip when enabled */
#define TILE_TYPE_LOSS (0x1)/* non fixed tile loss */
#define TILE_TYPE_RDMA (0x2)
#define TILE_TYPE_WDMA (0x4)
#define TILE_TYPE_CROP_EN (0x8)
#define TILE_TYPE_DONT_CARE_END (0x10) /* only used by dpframework */

typedef enum TILE_TDR_EDGE_GROUP_ENUM
{
    TILE_TDR_EDGE_GROUP_DEFAULT=0,/* pass by last module */
	TILE_TDR_EDGE_GROUP_ISP=1,
    TILE_TDR_EDGE_GROUP_CDP=2,
    TILE_TDR_EDGE_GROUP_MDP=3,/* mpd */
    TILE_TDR_EDGE_GROUP_WDMA=4,/* wdma, end at same time */
    TILE_TDR_EDGE_GROUP_OTHER=5,/* don't care */
    TILE_TDR_EDGE_GROU_NO
}TILE_TDR_EDGE_GROUP_ENUM;

#define TILE_INIT_PROPERTY_LUT(LABEL, CMD, a, b, c, d, e, f) \
	LABEL##_TILE_INIT_PROPERTY_LUT(CMD, a, b, c, d, e, f)

/* a, b, d, e, f reserved */
/* max_count: upper bound of dump hex, result: isp result */
/* c: ptr of tile_reg_map, reserved */
/* id of dump word, first line should be 0, other id can be different between neighboring dump word */
/* valid logic condition to dump */
/* register field name of c model to merge */
/* mask of register field */
#define TILE_SUPPORT_DUMP_LUT(LABEL, CMD, a, b, c, d, e, f, max_count, result) \
	LABEL##_TILE_SUPPORT_DUMP_LUT(CMD, a, b, c, d, e, f, max_count, result)\

/* a: add old, b: addr new, c: str name */
#define TILE_CPP_CODE_PRINTF_ISP_REG_ADDR_NAME(a, b, c, d, e, f, g, h, i, j, k, m, n) \
	{\
		unsigned int temp_addr = (size_t)(&(d->m));\
		if (a < temp_addr)\
		{\
			if (a == b)\
			{\
				b = temp_addr;\
				mtk_sprintf(c, sizeof(c), "\tCMD(a, b, c, 0x%08X, %s)\\\r\n", temp_addr, #m);\
			}\
			else\
			{\
				if (temp_addr < b)\
				{\
					b = temp_addr;\
					mtk_sprintf(c, sizeof(c), "\tCMD(a, b, c, 0x%08X, %s)\\\r\n", temp_addr, #m);\
				}\
			}\
		}\
	}\

/* a: add old, b: addr new, c: str name */
#define TILE_CPP_CODE_PRINTF_MDP_REG_ADDR_NAME(a, b, c, d, e, f, g, h, i, j, k, m, n) \
	if (a < m)\
	{\
		if (a == b)\
		{\
			b = m;\
			mtk_sprintf(c, sizeof(c), "\tCMD(a, b, c, 0x%08X, %s)\\\r\n", m, #m);\
		}\
		else\
		{\
			if (m < b)\
			{\
				b = m;\
				mtk_sprintf(c, sizeof(c), "\tCMD(a, b, c, 0x%08X, %s)\\\r\n", m, #m);\
			}\
		}\
	}\

/* a: add old, b: addr new, c: str name */
#define TILE_FPRINTF_ISP_REG_ADDR_VAL(a, b, c, d, e, f, g, h, i, j, k, m, n) \
	{\
		unsigned int temp_addr = (size_t)(&(d->m));\
		if (a < temp_addr)\
		{\
			if (a == b)\
			{\
				b = temp_addr;\
				c = (unsigned int)ISP_REG((e), m);\
				/* debug display */\
				/*printf("addr: 0x%08X, name: %s, val: 0x%08X\n", b, #m, c);*/\
			}\
			else\
			{\
				if (temp_addr < b)\
				{\
					b = temp_addr;\
					c = (unsigned int)ISP_REG((e), m);\
					/* debug display */\
					/*printf("addr: 0x%08X, name: %s, val: 0x%08X\n", b, #m, c);*/\
				}\
			}\
		}\
	}\

/* a: add old, b: addr new, c: str name */
#define TILE_FPRINTF_MDP_REG_ADDR_VAL_BY_CONFIG(a, b, c, d, e, f, g, h, i, j, k, m, n) \
	if (a < m)\
	{\
		if ((a == b) || ((a < b) && (m < b)))\
		{\
			int count;\
			b = m;\
			(c).mask = 0x0;\
			/* shift field & merge */\
			for (count=0;count<32;count++)\
			{\
				/* found nonzero lsb */\
				if (0x1 & ((n)>>count))\
				{\
					(c).val = (((d)->g) << count) & (n);\
					/* or mask count */\
					(c).mask |= (n);\
					/* debug display */\
					/*printf("addr: 0x%08X, name: %s, mask: 0x%08X, val: 0x%08X\n", m, #m, (c).mask, (c).val);*/\
					break;\
				}\
			}\
		}\
		else if ((a < b) && (m == b))\
		{\
			int count;\
			/* valid mask */\
			if ((c).mask & (n))\
			{\
				printf("-------------------------------------------------------\n");\
				printf("Warning duplicated hw reg with last addr: 0x%08X\n", b);\
				printf("val: 0x%08X\n", (c).val);\
				printf("Overwrite by mdp reg name: %s\n",#i);\
				printf("mdp hw reg: %s\n",#m);\
				printf("mdp mask: 0x%08X\n",n);\
				printf("-------------------------------------------------------\n");\
			}\
			/* shift field & merge */\
			for (count=0;count<32;count++)\
			{\
				/* found nonzero lsb */\
				if (0x1 & ((n)>>count))\
				{\
					(c).val |= (((d)->g) << count) & (n);\
					/* or mask count */\
					(c).mask |= (n);\
					/* debug display */\
					/*printf("addr: 0x%08X, name: %s, mask: 0x%08X, val: 0x%08X\n", m, #m, (c).mask, (c).val);*/\
					break;\
				}\
			}\
		}\
	}\

/* a: add old, b: addr new, c: str name */
#define TILE_FPRINTF_MDP_REG_ADDR_VAL_BY_REG(a, b, c, d, e, f, g, h, i, j, k, m, n) \
	if (a < m)\
	{\
		if ((a == b) || ((a < b) && (m < b)))\
		{\
			int count;\
			b = m;\
			(c).mask = 0x0;\
			/* shift field & merge */\
			for (count=0;count<32;count++)\
			{\
				/* found nonzero lsb */\
				if (0x1 & ((n)>>count))\
				{\
					(c).val = (((d)->i) << count) & (n);\
					/* or mask count */\
					(c).mask |= (n);\
					/* debug display */\
					/*printf("mask: 0x%08X, val: 0x%08X\n", n, (d)->i);*/\
					/*printf("addr: 0x%08X, name: %s, mask: 0x%08X, val: 0x%08X\n", m, #m, (c).mask, (c).val);*/\
					break;\
				}\
			}\
		}\
		else if ((a < b) && (m == b))\
		{\
			int count;\
			/* valid mask */\
			if ((c).mask & (n))\
			{\
				printf("-------------------------------------------------------\n");\
				printf("Warning duplicated hw reg with last addr: 0x%08X\n", b);\
				printf("val: 0x%08X\n", (c).val);\
				printf("Overwrite by mdp reg name: %s\n",#i);\
				printf("mdp hw reg: %s\n",#m);\
				printf("mdp mask: 0x%08X\n",n);\
				printf("-------------------------------------------------------\n");\
			}\
			/* shift field & merge */\
			for (count=0;count<32;count++)\
			{\
				/* found nonzero lsb */\
				if (0x1 & ((n)>>count))\
				{\
					(c).val |= (((d)->i) << count) & (n);\
					/* or mask count */\
					(c).mask |= (n);\
					/* debug display */\
					/*printf("mask: 0x%08X, val: 0x%08X\n", n, (d)->i);*/\
					/*printf("addr: 0x%08X, name: %s, mask: 0x%08X, val: 0x%08X\n", m, #m, (c).mask, (c).val);*/\
					break;\
				}\
			}\
		}\
	}\

/* a: add old, b: addr new, c: str name */
#define TILE_FREAD_ISP_REG_ADDR_VAL(a, b, c, d, e, f, g, h, i, j, k, m, n) \
	if (a == (size_t)(&(d.addr->m)))\
	{\
		ISP_REG((d.val), m) = b;\
		c = true;\
		e.val->i = ISP_BITS((d.val), m, n);\
		e.flag->i = 0x1;\
		/* debug display */\
		/*printf("addr: 0x%08X, name: %s, field: %s, val: 0x%08X\n", a, #m, #n, e.val->i);*/\
	}\

/* a: add old, b: addr new, c: str name */
#define TILE_FREAD_MDP_REG_ADDR_VAL(a, b, c, d, e, f, g, h, i, j, k, m, n) \
	if (a == m)\
	{\
		int count;\
		/* shift field & merge */\
		for (count=0;count<32;count++)\
		{\
			/* found nonzero lsb */\
			if (0x1 & (n >> count))\
			{\
				d.val->i = (b >> count) & (n >> count);\
				d.flag->i = 0x1;\
				/* debug display */\
				/*printf("addr: 0x%08X, name: %s, mask: 0x%08X, val: 0x%08X\n",a, #m, n, d.val->i);*/\
				break;\
			}\
		}\
	}\

/* wrapper macro codes */
#define TILE_WRAPPER_DATA_TYPE_EXTERN(a, b, c, d, e, f, g, h, i, j, k, m, n) extern f g j;
#define TILE_WRAPPER_DATA_TYPE_DECLARE(a, b, c, d, e, f, g, h, i, j, k, m, n) f g j;
#define TILE_HW_REG_TYPE_DECLARE(a, b, c, d, e, f, g, h, i, j, k, m, n) f i j;
#define TILE_WRAPPER_CURRENT_CMODEL_COPY(a, b, c, d, e, f, g, h, i, j, k, m, n) (a)->i = (f)g;
/* output c model param cpp file */
#define TILE_WRAPPER_DATA_PRINT_CPP(a, b, c, d, e, f, g, h, i, j, k, m, n) b(a, "%s %s%s%s%d;\n", #f, #g, #j, TILE_DEBUG_SPACE_EQUAL_SYMBOL_STR, g);
/* output c model param cpp file */
#define TILE_WRAPPER_DATA_PRINT_CPP_PATH(a, b, c, d, e, f, g, h, i, j, k, m, n) b(a, "%s %s%s%s%s;\n", #f, #g, #j, TILE_DEBUG_SPACE_EQUAL_SYMBOL_STR, g);
/* data copy from reg to platform param */
#define TILE_WRAPPER_REG_TO_PLATFORM_COPY_ISP(a, b, c, d, e, f, g, h, i, j, k, m, n) (a)->h = (b)->i;
/* data copy from reg to platform param */
#define TILE_WRAPPER_REG_TO_PLATFORM_COPY_MDP(a, b, c, d, e, f, g, h, i, j, k, m, n) (a)->g = (b)->i;
/* data search & copy log to platform param */
#define TILE_WRAPPER_PLATFORM_DATA_CMP_FILL_VAL(a, b, c, d, e, f, g, h, i, j, k, m, n) \
    if (false == (d))\
    {\
		char *ptr_chr = strstr(b, #h);\
		if (ptr_chr)\
		{\
			if (0 == memcmp(ptr_chr + sizeof(#h) - 1, TILE_DEBUG_SPACE_EQUAL_SYMBOL_STR, sizeof(TILE_DEBUG_SPACE_EQUAL_SYMBOL_STR) - 1))\
			{\
				(a)->h = c;\
				(d) = true;\
			}\
        }\
    }\

#define TILE_WRAPPER_PLATFORM_DATA_CMP_FILL_VAL_FLAG_ISP(a, b, c, d, e, f, g, h, i, j, k, m, n) \
    if (false == (d))\
    {\
		char *ptr_chr = (char *)strstr(b, #h);\
		if (ptr_chr)\
		{\
			if (0 ==  memcmp(ptr_chr + sizeof(#h) - 1, TILE_DEBUG_SPACE_EQUAL_SYMBOL_STR, sizeof(TILE_DEBUG_SPACE_EQUAL_SYMBOL_STR) - 1))\
			{\
				(a)->i = c;\
				(e)->i = 1;\
				(d) = true;\
			}\
		}\
    }\

#define TILE_WRAPPER_PLATFORM_DATA_CMP_FILL_VAL_FLAG_MDP(a, b, c, d, e, f, g, h, i, j, k, m, n) \
    if (false == (d))\
    {\
		char *ptr_chr = (char *)strstr(b, #g);\
		if (ptr_chr)\
		{\
			if (0 ==  memcmp(ptr_chr + sizeof(#g) - 1, TILE_DEBUG_SPACE_EQUAL_SYMBOL_STR, sizeof(TILE_DEBUG_SPACE_EQUAL_SYMBOL_STR) - 1))\
			{\
				(a)->i = c;\
				(e)->i = 1;\
				(d) = true;\
			}\
		}\
    }\

#define TILE_WRAPPER_REG_CMP_FILL_VAL_FLAG_ISP(a, b, c, d, e, f, g, h, i, j, k, m, n) \
	if ((size_t)((b).addr) == size_t(&((d).addr->m)))\
	{\
		if ((a).flag->i)\
		{\
			printf("-------------------------------------------------------\n");\
			printf("Warning duplicated to overwrite addr: 0x%08X\n", (b).addr);\
			printf("name: %s, field: %s\n", #m, #n);\
			printf("last val: 0x%08X, new val: 0x%08X\n", (a).val->i, (b).val);\
			printf("-------------------------------------------------------\n");\
		}\
		ISP_REG((d.val), m) = (b).val;\
		(a).val->i = ISP_BITS(((d).val), m, n);\
		(a).flag->i = 1;\
		/* debug display */\
		/*printf("addr: 0x%08X, name: %s, field: %s, val: 0x%08X\n", b.addr, #m, #n, (a).val->i);*/\
		(c)++;\
    }\

#define TILE_WRAPPER_REG_CMP_FILL_VAL_FLAG_MDP(a, b, c, d, e, f, g, h, i, j, k, m, n) \
	if ((b).addr == m)\
	{\
		int count;\
		if ((a).flag->i)\
		{\
			printf("-------------------------------------------------------\n");\
			printf("Warning duplicated to overwrite addr: 0x%08X\n", b.addr);\
			printf("name: %s, field: %s\n", #m, #n);\
			printf("last val: 0x%08X, new val: 0x%08X\n", (a).val->i, b.val);\
			printf("-------------------------------------------------------\n");\
		}\
		/* shift field & merge */\
		for (count=0;count<32;count++)\
		{\
			/* found nonzero lsb */\
			if (0x1 & (n >> count))\
			{\
				(a).val->i = ((b).val >> count) & ((n) >> count);\
				(a).flag->i = 0x1;\
				/* debug display */\
				/*printf("tile name: %s, addr: 0x%08X, val: 0x%08X\n", #i, (b).addr, (b).val);*/\
				/*printf("reg name: %s, mask: 0x%08X, val: 0x%08X\n", #m, n, (a).val->i);*/\
				(c)++;\
				break;\
			}\
		}\
	}\

#define TILE_WRAPPER_REG_DATA_CMP_FILL_VAL_FLAG(a, b, c, d, e, f, g, h, i, j, k, m, n) \
    if (false == (d))\
    {\
		char *ptr_chr = (char *)strstr(b, #i);\
		if (ptr_chr)\
		{\
			if (0 ==  memcmp(ptr_chr + sizeof(#i) - 1, TILE_DEBUG_SPACE_EQUAL_SYMBOL_STR, sizeof(TILE_DEBUG_SPACE_EQUAL_SYMBOL_STR) - 1))\
			{\
				(a)->i = c;\
				(e)->i = 1;\
				(d) = true;\
			}\
		}\
    }\

/* data search & update reg mapper */
/* a: ptr_reg */
/* b: cmp str */
/* c: value to fill */
/* d: found flag */
#define TILE_UT_REG_CMP_FILL_VAL(a, b, c, d, e, f, g, h, i, j, k, m, n) \
    if (false == (d))\
    {\
		if (0 == memcmp(b, #i, sizeof(#i)))\
		{\
			(a)->i = c;\
			(d) = true;\
		}\
    }\

/* data copy from platform to reg param */
#define TILE_WRAPPER_ISP_HW_TO_REG_COPY(a, b, c, d, e, f, g, h, i, j, k, m, n) \
	(b)->i = (int)ISP_BITS((a), m, n);\
	(c)->i = true;\

/* data copy from platform to reg param */
#define TILE_WRAPPER_ISP_REG_TO_HW_COPY(a, b, c, d, e, f, g, h, i, j, k, m, n) \
	if (ISP_BITS((b), m, n))\
	{\
		printf("-------------------------------------------------------\n");\
		printf("Warning duplicated hw reg with last val: 0x%08X\n", ISP_BITS((a), m, n));\
		printf("Overwrite by isp reg name: %s\n",#i);\
		printf("hw reg: %s, field: %s\n",#m, #n);\
		printf("val: 0x%08X\n", (c)->i);\
		printf("-------------------------------------------------------\n");\
	}\
	ISP_BITS((a), m, n) = (c)->i;\
	ISP_BITS((b), m, n) =  true;\

/* data copy from platform to reg param */
#define TILE_WRAPPER_ISP_CONFIG_TO_HW_COPY(a, b, c, d, e, f, g, h, i, j, k, m, n) \
	if (ISP_BITS((b), m, n))\
	{\
		printf("Warning duplicated hw reg with last val: 0x%08X\n", ISP_BITS((a), m, n));\
		printf("Overwrite by isp config name: %s, hw reg: %s, field: %s, val: 0x%08X\n",#i, #m, #n, (c)->h);\
		printf("-------------------------------------------------------\n");\
		printf("Warning duplicated hw reg with last val: 0x%08X\n", ISP_BITS((a), m, n));\
		printf("Overwrite by isp reg name: %s\n",#i);\
		printf("hw reg: %s, field: %s\n",#m, #n);\
		printf("val: 0x%08X\n", (c)->h);\
		printf("-------------------------------------------------------\n");\
	}\
	ISP_BITS((a), m, n) = (c)->h;\
	ISP_BITS((b), m, n) =  true;\

/* data copy from platform to reg param */
#define TILE_WRAPPER_PLATFORM_TO_REG_COPY_ISP(a, b, c, d, e, f, g, h, i, j, k, m, n) (a)->i = (b)->h;
/* data copy from platform to reg param */
#define TILE_WRAPPER_PLATFORM_TO_REG_COPY_MDP(a, b, c, d, e, f, g, h, i, j, k, m, n,...) (a)->i = (b)->g;
/* output c model param txt file */
#define TILE_WRAPPER_DATA_PRINT_TXT(a, b, c, d, e, f, g, h, i, j, k, m, n) c(a, "%s%s%s%d\n", #i, #j, TILE_DEBUG_SPACE_EQUAL_SYMBOL_STR, (b)->i);
#define TILE_WRAPPER_DATA_PRINT_PATH(a, b, c, d, e, f, g, h, i, j, k, m, n) c(a, "%s%s%s%s\n", #i, #j, TILE_DEBUG_SPACE_EQUAL_SYMBOL_STR, (b)->i);
/* output platform param txt file by fprintf */
#define TILE_WRAPPER_PLATFORM_PRINT_TXT_ISP(a, b, c, d, e, f, g, h, i, j, k, m, n) \
    d(a, "%s %s%s%s%d %s\n", TPIPE_DEBUG_DUMP_HEADER, #h, #j, TILE_DEBUG_SPACE_EQUAL_SYMBOL_STR, (b)->h, (k)?LOG_REMARK_MUST_FILL_STR:"");
#define TILE_WRAPPER_PLATFORM_PRINT_TXT_MDP(a, b, c, d, e, f, g, h, i, j, k, m, n) \
    d(a, "%s %s%s%s%d %s\n", TPIPE_DEBUG_DUMP_HEADER, #g, #j, TILE_DEBUG_SPACE_EQUAL_SYMBOL_STR, (c)->i, (k)?LOG_REMARK_MUST_FILL_STR:"");
#define TILE_WRAPPER_PLATFORM_DEBUG_PRINT_TXT(a, b, c, d, e, f, g, h, i, j, k, m, n) \
    c(a, "%s %s%s%s%d %s\n", TPIPE_DEBUG_DUMP_HEADER, #i, #j, TILE_DEBUG_SPACE_EQUAL_SYMBOL_STR, (b)->i, LOG_REMARK_DEBUG_STR);
/* output platform param txt file uart printf */
#define TILE_WRAPPER_PLATFORM_ISP_UART_PRINT_TXT(a, b, c, d, e, f, g, h, i, j, k, m, n) \
    a("%s %s%s%s%d %s\n", TPIPE_DEBUG_DUMP_HEADER, #h, #j, TILE_DEBUG_SPACE_EQUAL_SYMBOL_STR, (b)->h, (k)?LOG_REMARK_MUST_FILL_STR:"");
#define TILE_WRAPPER_PLATFORM_MDP_UART_PRINT_TXT(a, b, c, d, e, f, g, h, i, j, k, m, n) \
    a("%s %s%s%s%d %s\n", TPIPE_DEBUG_DUMP_HEADER, #g, #j, TILE_DEBUG_SPACE_EQUAL_SYMBOL_STR, (c)->i, (k)?LOG_REMARK_MUST_FILL_STR:"");
#define TILE_WRAPPER_PLATFORM_UART_DEBUG_PRINT_TXT(a, b, c, d, e, f, g, h, i, j, k, m, n) \
    a("%s %s%s%s%d %s\n", TPIPE_DEBUG_DUMP_HEADER, #i, #j, TILE_DEBUG_SPACE_EQUAL_SYMBOL_STR,\
	(b)->i, LOG_REMARK_DEBUG_STR);
#define TILE_WRAPPER_PLATFORM_PRINT_TXT_BY_FLAG_ISP(a, b, c, d, e, f, g, h, i, j, k, m, n) \
	fprintf(a, "%s %s%s%s%d %s%s\n", TPIPE_DEBUG_DUMP_HEADER, #h, #j,\
	TILE_DEBUG_SPACE_EQUAL_SYMBOL_STR, (b)->i, (k)?LOG_REMARK_MUST_FILL_STR:"",\
	((d)->i)?"":LOG_REMARK_NOT_FOUND_STR);
#define TILE_WRAPPER_PLATFORM_PRINT_TXT_BY_FLAG_MDP(a, b, c, d, e, f, g, h, i, j, k, m, n) \
	fprintf(a, "%s %s%s%s%d %s%s\n", TPIPE_DEBUG_DUMP_HEADER, #g, #j,\
	TILE_DEBUG_SPACE_EQUAL_SYMBOL_STR, (b)->i, (k)?LOG_REMARK_MUST_FILL_STR:"",\
	((d)->i)?"":LOG_REMARK_NOT_FOUND_STR);
#define TILE_WRAPPER_PLATFORM_PRINT_TXT_NO_HW_LOG_FLAG_ISP(a, b, c, d, e, f, g, h, i, j, k, m, n) \
	fprintf(a, "%s %s%s%s%d %s%s%s\n", TPIPE_DEBUG_DUMP_HEADER, #h, #j,\
	TILE_DEBUG_SPACE_EQUAL_SYMBOL_STR, (b)->i, (k)?LOG_REMARK_MUST_FILL_STR:"",\
	((d)->i)?"":LOG_REMARK_NOT_FOUND_STR, LOG_REMARK_HW_REG_STR);
#define TILE_WRAPPER_PLATFORM_PRINT_TXT_NO_HW_LOG_FLAG_MDP(a, b, c, d, e, f, g, h, i, j, k, m, n) \
	fprintf(a, "%s %s%s%s%d %s%s%s\n", TPIPE_DEBUG_DUMP_HEADER, #g, #j,\
	TILE_DEBUG_SPACE_EQUAL_SYMBOL_STR, (b)->i, (k)?LOG_REMARK_MUST_FILL_STR:"",\
	((d)->i)?"":LOG_REMARK_NOT_FOUND_STR, LOG_REMARK_HW_REG_STR);
#define TILE_WRAPPER_PLATFORM_PRINT_TXT_BY_FLAG_WITH_REG_CHECK_ISP(a, b, c, d, e, f, g, h, i, j, k, m, n) \
	if ((e).flag->i)/* hw reg flag true */\
	{\
		if ((d)->i)/* with tile log*/\
    {\
			if (((b)->i) == (int)((e).val->i))\
			{\
				fprintf(a, "%s %s%s%s%d %s\n", TPIPE_DEBUG_DUMP_HEADER, #h, #j, TILE_DEBUG_SPACE_EQUAL_SYMBOL_STR,\
					(b)->i, (k)?LOG_REMARK_MUST_FILL_STR:"");\
			}\
			else\
			{\
				fprintf(a, "%s %s%s%s%d %s%s%d\n", TPIPE_DEBUG_DUMP_HEADER, #h, #j, TILE_DEBUG_SPACE_EQUAL_SYMBOL_STR,\
					(b)->i, (k)?LOG_REMARK_MUST_FILL_STR:"", LOG_REMARK_HW_DIFF_STR, ((e).val->i));\
			}\
		}\
		else/* no tile log*/\
		{\
			fprintf(a, "%s %s%s%s%d %s%s%s%d\n", TPIPE_DEBUG_DUMP_HEADER, #h, #j, TILE_DEBUG_SPACE_EQUAL_SYMBOL_STR,\
				(b)->i, LOG_REMARK_NOT_FOUND_STR, (k)?LOG_REMARK_MUST_FILL_STR:"", LOG_REMARK_HW_LOG_STR, ((e).val->i));\
		}\
	}\
	else/* hw reg flag false */\
	{\
		fprintf(a, "%s %s%s%s%d %s%s%s\n", TPIPE_DEBUG_DUMP_HEADER, #h, #j,\
			TILE_DEBUG_SPACE_EQUAL_SYMBOL_STR, (b)->i, (k)?LOG_REMARK_MUST_FILL_STR:"",\
			((d)->i)?"":LOG_REMARK_NOT_FOUND_STR, LOG_REMARK_HW_NOT_FOUND_STR);\
	}\

#define TILE_WRAPPER_PLATFORM_PRINT_TXT_BY_FLAG_WITH_REG_CHECK_MDP(a, b, c, d, e, f, g, h, i, j, k, m, n) \
	if ((e).flag->i)/* hw reg flag true */\
	{\
		if ((d)->i)/* with tile log*/\
		{\
			if (((b)->i) == (int)((e).val->i))\
			{\
				fprintf(a, "%s %s%s%s%d %s\n", TPIPE_DEBUG_DUMP_HEADER, #g, #j, TILE_DEBUG_SPACE_EQUAL_SYMBOL_STR,\
					(b)->i, (k)?LOG_REMARK_MUST_FILL_STR:"");\
    }\
    else\
    {\
				fprintf(a, "%s %s%s%s%d %s%s%d\n", TPIPE_DEBUG_DUMP_HEADER, #g, #j, TILE_DEBUG_SPACE_EQUAL_SYMBOL_STR,\
					(b)->i, (k)?LOG_REMARK_MUST_FILL_STR:"", LOG_REMARK_HW_DIFF_STR, ((e).val->i));\
			}\
		}\
		else/* no tile log*/\
		{\
			fprintf(a, "%s %s%s%s%d %s%s%s%d\n", TPIPE_DEBUG_DUMP_HEADER, #g, #j, TILE_DEBUG_SPACE_EQUAL_SYMBOL_STR,\
				(b)->i, LOG_REMARK_NOT_FOUND_STR, (k)?LOG_REMARK_MUST_FILL_STR:"", LOG_REMARK_HW_LOG_STR, ((e).val->i));\
		}\
	}\
	else/* hw reg flag false */\
	{\
		fprintf(a, "%s %s%s%s%d %s%s%s\n", TPIPE_DEBUG_DUMP_HEADER, #g, #j,\
			TILE_DEBUG_SPACE_EQUAL_SYMBOL_STR, (b)->i, (k)?LOG_REMARK_MUST_FILL_STR:"",\
			((d)->i)?"":LOG_REMARK_NOT_FOUND_STR, LOG_REMARK_HW_NOT_FOUND_STR);\
	}\

#define TILE_WRAPPER_PLATFORM_DEBUG_PRINT_TXT_BY_FLAG(a, b, c, d, e, f, g, h, i, j, k, m, n) \
	fprintf(a, "%s %s%s%s%d %s%s\n", TPIPE_DEBUG_DUMP_HEADER, #i, #j, TILE_DEBUG_SPACE_EQUAL_SYMBOL_STR, (b)->i, LOG_REMARK_DEBUG_STR, ((d)->i)?"":LOG_REMARK_NOT_FOUND_STR);
#define TILE_WRAPPER_HORZ_PARA_DECLARE(a, b, c, d, e, f, g, h, i, j, k, m, n) TILE_WRAPPER_HORZ_PARA_DECLARE_##k(f, g, j)
#define TILE_WRAPPER_HORZ_PARA_DECLARE_S(f, g, j)
#define TILE_WRAPPER_HORZ_PARA_DECLARE_U(f, g, j) f g j;
#define TILE_WRAPPER_HORZ_PARA_DECLARE_M(f, g, j) TILE_WRAPPER_HORZ_PARA_DECLARE_U(f, g, j)
#define TILE_WRAPPER_HORZ_PARA_BACKUP(a, b, c, d, e, f, g, h, i, j, k, m, n) TILE_WRAPPER_HORZ_PARA_BACKUP_##k(a, b, g);
#define TILE_WRAPPER_HORZ_PARA_BACKUP_S(a, b, g)
#define TILE_WRAPPER_HORZ_PARA_BACKUP_U(a, b, g) (a)->g = (b)->g;
#define TILE_WRAPPER_HORZ_PARA_BACKUP_M(a, b, g)TILE_WRAPPER_HORZ_PARA_BACKUP_U(a, b, g)
#define TILE_WRAPPER_HORZ_PARA_RESTORE(a, b, c, d, e, f, g, h, i, j, k, m, n) TILE_WRAPPER_HORZ_PARA_RESTORE_##k(a, b, g, i);
#define TILE_WRAPPER_HORZ_PARA_RESTORE_S(a, b, g, i)
#define TILE_WRAPPER_HORZ_PARA_RESTORE_U(a, b, g, i) (b)->g = (a)->g;
#define TILE_WRAPPER_HORZ_PARA_RESTORE_M(a, b, g, i) (b)->g = (((b)->g)&(~(i) & 0xF)) | (((a)->g)&(i));

#define TILE_WRAPPER_PLATFORM_REG_CMP(a, b, c, d, e, f, g, h, i, j, k, m, n) \
	if ((a)->i != (b)->i)\
	{\
		(c) = ISP_MESSAGE_TILE_REG_MAP_COMP_DIFF_ERROR;\
		printf("Different register %s: %d, %d\n", #i, (a)->i, (b)->i);\
	}\

/* register convert tile function */
//a: current func no
//b: ptr of TILE_FUNC_BLOCK_STRUCT[0]
//group_num
//c: func valid condition
//d: func no
//e: fun name
//f: last func no
//g: last func name
/* only select enable items */
#define TILE_MODULE_CHECK(a, b, last_no, result, group_id, c, d, e, f, g) \
    if (ISP_MESSAGE_OK == (result))\
    {\
		if (true == (c))\
		{\
			if ((last_no) == (d))\
			{\
				/*tile_driver_printf("Found mutli-in func: %s, id: %d, last func: %s, id: %d\n\n", #e, d, #g, f);*/\
				if (((b) + (a-1))->tot_prev_num < MAX_TILE_PREV_NO)\
				{\
					((b) + (a-1))->last_func_num[((b) + (a-1))->tot_prev_num] = (f);\
					((b) + (a-1))->tot_prev_num++;\
				}\
				else\
				{\
					result = ISP_MESSAGE_OVER_MAX_TILE_FUNC_PREV_NO_ERROR;\
					tile_driver_printf("Error: %s\n\n", tile_print_error_message(result));\
				}\
			}\
			else\
			{\
				/* prevent corrupt memory */\
				if ((a) < MAX_TILE_FUNC_NO)\
				{\
					(last_no) = (d);\
					/* save memset time of overall buffer */\
					memset(((b) + a), 0x0, sizeof(TILE_FUNC_BLOCK_STRUCT));\
					((b) + (a))->func_num = (d);\
					((b) + (a))->last_func_num[0] = (f);\
					((b) + (a))->group_num = (group_id);\
					((b) + (a))->tot_prev_num = 1;\
					/*tile_driver_printf("Found func: %s, id: %d, last func: %s, id: %d\n\n", #e, d, #g, f);*/\
					(a)++;\
				}\
				else\
				{\
					result = ISP_MESSAGE_OVER_MAX_TILE_FUNC_EN_NO_ERROR;\
					tile_driver_printf("Error: %s\n\n", tile_print_error_message(result));\
				}\
			}\
		}\
    }\

/* register convert tile function */
//a: current func no
//b: ptr of func_en_id[0]
//c: func id
//d: name
//e: valid condition
/* list all functions enable or not */
#define TILE_ENABLE_CHECK(a, b, last_no, result, c, d, e) \
    if (ISP_MESSAGE_OK == (result))\
    {\
		if ((last_no) != (int)(c))\
		{\
			if (last_no)\
			{\
				(a)++;\
			}\
			/* prevent corrupt memory */\
			if ((a) >= MAX_TILE_FUNC_EN_NO)\
			{\
				result = ISP_MESSAGE_OVER_MAX_TILE_FUNC_EN_NO_ERROR;\
				tile_driver_printf("Error: %s\n\n", tile_print_error_message(result));\
			}\
			else\
			{\
				(last_no) = (c);\
				((b) + (a))->func_num = (c);\
				if (e)\
				{\
					((b) + (a))->enable_flag = true;\
				}\
				else\
				{\
					((b) + (a))->enable_flag = false;\
				}\
			}\
		}\
		else\
		{\
			if (e)\
			{\
				if (((b) + (a))->enable_flag)\
				{\
					result = ISP_MESSAGE_DUPLICATED_FUNC_EN_FOUND_ERROR;\
					tile_driver_printf("Error: %s\n", tile_print_error_message(result));\
					tile_driver_printf("Duplicated func: %s, id: %d\n\n", #d, c);\
				}\
				else\
				{\
					((b) + (a))->enable_flag = true;\
				}\
			}\
		}\
    }\

/* register convert tile function */
//a: current func no
//b: ptr of func_en_id[0]
//c: func id
//d: name
//e: valid condition
/* list all functions enable or not */
#define TILE_OUTPUT_DISABLE_CHECK(a, last_no, result, b, c, d, e) \
    if (ISP_MESSAGE_OK == (result))\
    {\
        if (c == (a)->func_num)\
        {\
            if ((last_no) != (int)(c))\
            {\
                (last_no) = (c);\
                if (e)\
                {\
                    (a)->output_disable_flag = true;\
                }\
            }\
            else\
            {\
                if (e)\
                {\
                    if ((a)->output_disable_flag)\
                    {\
                        (result) = ISP_MESSAGE_DUPLICATED_FUNC_DISABLE_OUTPUT_FOUND_ERROR;\
                        tile_driver_printf("Error: %s\n", tile_print_error_message(result));\
                        tile_driver_printf("Duplicated func: %s, id: %d\n\n", #d, c);\
                    }\
                    else\
                    {\
                        (a)->output_disable_flag = true;\
                    }\
                }\
            }\
        }\
    }\


/* register convert tile function */
//a: current func no
//b: ptr of func_en_id[0]
//c: func id
//d: name
//e: master func id
//f: master name
//g: valid condition
#define TILE_SUBRDMA_CHECK(a, b, result, group_no, c, d, e, f, g) \
    if (ISP_MESSAGE_OK == (result))\
    {\
		/* check enable & duplicated later */\
		if (true == (g))\
		{\
			/* prevent corrupt memory */\
			if ((a) < MAX_TILE_FUNC_SUBRDMA_NO)\
			{\
				((b) + (a))->func_num = (c);\
				((b) + (a))->master_func_num = (e);\
				/* clear enable & tdr func flag */\
				((b) + (a))->enable_flag = false;\
				((b) + (a))->func_ptr_flag = 0x0;\
				((b) + (a))->group_num = group_no;\
				(a)++;\
			}\
			else\
			{\
				result = ISP_MESSAGE_OVER_MAX_TILE_FUNC_SUBRDMA_NO_ERROR;\
				tile_driver_printf("Error: %s\n\n", tile_print_error_message(result));\
			}\
		}\
    }\

/* run sub rdma tdr function */
//a: func ptr
//b: ptr_tile_reg_map
//c: ptr_tile_reg_dump_param
//d: rdma func no
//e: found_flag
//f: error message
//g: func no
//h: func name
//t1~t10: tile property
//p1~p4: fun ptr
#define RUN_SUB_RDMA_TDR_FUNC(a, b, c, d, e, f, g, h, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, p1, p2, p3, p4, r1, r2) \
    if (ISP_MESSAGE_OK == (f))\
    {\
        if (false == (e))\
        {\
            if ((g) == (d))\
            {\
                /* some tdr func are NULL, can not assign by p4 as func name directly */\
	            ISP_MESSAGE_ENUM (*tdr_func_ptr)(TILE_FUNC_BLOCK_STRUCT *ptr_func, TILE_REG_MAP_STRUCT* ptr_tile_reg_map,\
				TILE_REG_DUMP_STRUCT* ptr_tile_reg_dump_param);\
				tdr_func_ptr = p4;\
				if (NULL != tdr_func_ptr)\
				{\
					(f) = tdr_func_ptr(a, b, c);\
				}\
                (e) = true;\
            }\
        }\
    }\

/* init tile function */
//a: ptr of current TILE_FUNC_BLOCK_STRUCT
//b: ptr of current TILE_CAL_FUNC_STRUCT
//c: found flag
//d: group num
//e: reserved
//f: reserved
//g: func no
//h: func name
//t1~t10: tile property
//p1~p4: fun ptr
#define INIT_TILE_FUNC(a, b, c, d, e, f, g, h, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, p1, p2, p3, p4, r1, r2) \
    if (false == (b))\
    {\
        if ((g) == (a)->func_num)\
        {\
			ISP_MESSAGE_ENUM (*init_func_ptr)(TILE_FUNC_BLOCK_STRUCT *ptr_func, TILE_REG_MAP_STRUCT* ptr_tile_reg_map);\
			ISP_MESSAGE_ENUM (*for_func_ptr)(TILE_FUNC_BLOCK_STRUCT *ptr_func, TILE_REG_MAP_STRUCT* ptr_tile_reg_map);\
			ISP_MESSAGE_ENUM (*back_func_ptr)(TILE_FUNC_BLOCK_STRUCT *ptr_func, TILE_REG_MAP_STRUCT* ptr_tile_reg_map);\
			ISP_MESSAGE_ENUM (*tdr_func_ptr)(TILE_FUNC_BLOCK_STRUCT *ptr_func, TILE_REG_MAP_STRUCT* ptr_tile_reg_map,\
				TILE_REG_DUMP_STRUCT* ptr_tile_reg_dump_param);\
			(a)->group_num = (d);\
			(a)->run_mode = TILE_RUN_MODE_MAIN;\
			(a)->type = t1;\
            (a)->tdr_group = (TILE_TDR_EDGE_GROUP_ENUM)t2;\
            (a)->l_tile_loss = t3;\
            (a)->r_tile_loss = t4;\
            (a)->t_tile_loss = t5;\
            (a)->b_tile_loss = t6;\
            (a)->in_const_x = t7;\
            (a)->in_const_y = t8;\
            (a)->out_const_x = t9;\
            (a)->out_const_y = t10;\
			init_func_ptr = p1;\
			if (init_func_ptr)\
			{\
				(a)->func_ptr_flag |= TILE_INIT_FUNC_PTR_FLAG;\
			}\
			for_func_ptr = p2;\
			if (for_func_ptr)\
			{\
				(a)->func_ptr_flag |= TILE_FOR_FUNC_PTR_FLAG;\
			}\
			back_func_ptr = p3;\
			if (back_func_ptr)\
			{\
				(a)->func_ptr_flag |= TILE_BACK_FUNC_PTR_FLAG;\
			}\
			tdr_func_ptr = p4;\
			if (tdr_func_ptr)\
			{\
				(a)->func_ptr_flag |= TILE_TDR_FUNC_PTR_FLAG;\
			}\
			if (r1)\
			{\
				if (TILE_ISP_GROUP_NUM == (d))\
				{\
					(a)->in_tile_width = (c)->isp_tile_width;\
					(a)->in_tile_height = (c)->isp_tile_height;\
				}\
				else\
				{\
					(a)->in_tile_width = (c)->mdp_tile_width;\
					(a)->in_tile_height = (c)->mdp_tile_height;\
				}\
			}\
			else\
			{\
				(a)->in_tile_width = 0;\
				(a)->in_tile_height = 0;\
			}\
			if (r2)\
			{\
				if (TILE_ISP_GROUP_NUM == (d))\
				{\
					(a)->out_tile_width = (c)->isp_tile_width;\
					(a)->out_tile_height = (c)->isp_tile_height;\
				}\
				else\
				{\
					(a)->out_tile_width = (c)->mdp_tile_width;\
					(a)->out_tile_height = (c)->mdp_tile_height;\
				}\
			}\
			else\
			{\
				(a)->out_tile_width = 0;\
				(a)->out_tile_height = 0;\
			}\
            (b) = true;\
        }\
    }\

/* init tile function */
//a: ptr of current func_subrdma_list
//b: found flag
//c: reserved
//d: reserved
//e: reserved
//f: reserved
//g: func no
//h: func name
//t1~t10: tile property
//p1~p4: fun ptr
#define INIT_TILE_SUBRDMA_TDR(a, b, c, d, e, f, g, h, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, p1, p2, p3, p4, r1, r2) \
    if (false == (b))\
    {\
        if ((g) == (a)->func_num)\
        {\
			ISP_MESSAGE_ENUM (*tdr_func_ptr)(TILE_FUNC_BLOCK_STRUCT *ptr_func, TILE_REG_MAP_STRUCT* ptr_tile_reg_map,\
				TILE_REG_DUMP_STRUCT* ptr_tile_reg_dump_param);\
			tdr_func_ptr = p4;\
			(b) = true;\
			if (tdr_func_ptr)\
			{\
				(a)->func_ptr_flag |= TILE_TDR_FUNC_PTR_FLAG;\
			}\
		}\
	}\

/* run init function */
//a: func ptr
//b: found flag
//c: ptr_tile_reg_map
//d: error message
//e:
//f:
//g: func no
//h: func name
//t1~t10: tile property
//p1~p4: fun ptr
#define TILE_MODULE_INIT_RUN(a, b, c, d, e, f, g, h, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, p1, p2, p3, p4, r1, r2) \
	case (g):\
	{\
		ISP_MESSAGE_ENUM (*init_func_ptr)(TILE_FUNC_BLOCK_STRUCT *ptr_func, TILE_REG_MAP_STRUCT* ptr_tile_reg_map);\
		init_func_ptr = p1;\
		(b) = true;\
		if (init_func_ptr)\
		{\
			(d) = init_func_ptr(a, c);\
		}\
		else\
		{\
			(d) = ISP_MESSAGE_UNMATCH_INIT_FUNC_PTR_ERROR;\
			tile_driver_printf("Error: %s\n", tile_print_error_message(result));\
			tile_driver_printf("Unmatch init function no: %d, ptr name: %s\n\n", g, #p1);\
		}\
		break;\
	}\

/* run for function */
//a: func ptr
//b: found flag
//c: ptr_tile_reg_map
//d: error message
//e:
//f:
//g: func no
//h: func name
//t1~t10: tile property
//p1~p4: fun ptr
#define TILE_MODULE_FOR_RUN(a, b, c, d, e, f, g, h, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, p1, p2, p3, p4, r1, r2) \
	case (g):\
	{\
		ISP_MESSAGE_ENUM (*for_func_ptr)(TILE_FUNC_BLOCK_STRUCT *ptr_func, TILE_REG_MAP_STRUCT* ptr_tile_reg_map);\
		for_func_ptr = p2;\
		(b) = true;\
		if (for_func_ptr)\
		{\
			(d) = for_func_ptr(a, c);\
		}\
		else\
		{\
			(d) = ISP_MESSAGE_UNMATCH_FOR_FUNC_PTR_ERROR;\
			tile_driver_printf("Error: %s\n", tile_print_error_message(result));\
			tile_driver_printf("Unmatch for function no: %d, ptr name: %s\n\n", g, #p2);\
		}\
		break;\
	}\

/* run for function */
//a: func ptr
//b: found flag
//c: ptr_tile_reg_map
//d: error message
//e:
//f:
//g: func no
//h: func name
//t1~t10: tile property
//p1~p4: fun ptr
#define TILE_MODULE_BACK_RUN(a, b, c, d, e, f, g, h, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, p1, p2, p3, p4, r1, r2) \
	case (g):\
	{\
		ISP_MESSAGE_ENUM (*back_func_ptr)(TILE_FUNC_BLOCK_STRUCT *ptr_func, TILE_REG_MAP_STRUCT* ptr_tile_reg_map);\
		back_func_ptr = p3;\
		(b) = true;\
		if (back_func_ptr)\
		{\
			(d) = back_func_ptr(a, c);\
		}\
		else\
		{\
			(d) = ISP_MESSAGE_UNMATCH_BACK_FUNC_PTR_ERROR;\
			tile_driver_printf("Error: %s\n", tile_print_error_message(result));\
			tile_driver_printf("Unmatch back function no: %d, ptr name: %s\n\n", g, #p3);\
		}\
		break;\
	}\

/* run tdr function */
//a: func ptr
//b: ptr_tile_reg_dump_param
//c: ptr_tile_reg_map
//d: found flag
//e: error message
//f:
//g: func no
//h: func name
//t1~t10: tile property
//p1~p4: fun ptr
#define TILE_MODULE_TDR_RUN(a, b, c, d, e, f, g, h, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, p1, p2, p3, p4, r1, r2) \
	case g:\
	{\
		ISP_MESSAGE_ENUM (*tdr_func_ptr)(TILE_FUNC_BLOCK_STRUCT *ptr_func, TILE_REG_MAP_STRUCT* ptr_tile_reg_map,\
			TILE_REG_DUMP_STRUCT* ptr_tile_reg_dump_param);\
		tdr_func_ptr = p4;\
		(d) = true;\
		if (tdr_func_ptr)\
		{\
			(e) = tdr_func_ptr(a, c, b);\
		}\
		else\
		{\
			(e) = ISP_MESSAGE_UNMATCH_TDR_FUNC_PTR_ERROR;\
			tile_driver_printf("Error: %s\n", tile_print_error_message(result));\
			tile_driver_printf("Unmatch back function no: %d, ptr name: %s\n\n", g, #p3);\
		}\
		break;\
	}\

/* init tile function */
//a: ptr of current TILE_FUNC_BLOCK_STRUCT
//b: ptr of current TILE_CAL_FUNC_STRUCT
//c: found flag
//d: reserve
//e: reserve
//f: func no
//g: func name
//t1~t2: tile property
//p1~p4: fun ptr
#define TILE_INIT_PROPERTY_LUT_FUNC_NO_CHECK(a, b, c, d, e, f, g, h, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, p1, p2, p3, p4, r1, r2) \
    int tile_init_property_lut_func_no_check_##g = g;

/* register to dump */
//a: ptr of register table
//b: word val to dump
//c: current id
//d: merge count per dump
//e: ptr of dump word
//f: word count or offset to dump
#define TILE_REG_TO_DUMP(a, b, c, d, e, f, max_count, result, g, h, i, j) \
    /* different id or word */\
	if (ISP_MESSAGE_OK == result)\
	{\
		if ((g) != (c))\
		{\
			/* last merge count > 0 */\
			if ((d) > 0)\
			{\
				/* prevent corrupt memory */\
				if ((f) <  max_count)\
				{\
					*((e) + (f)) = (b);\
				}\
				(f)++;\
				(d) = 0;\
				(b) = 0;\
			}\
			/* update current id */\
			(c) = (g);\
		}\
		/* valid reg */\
		if (true == (h))\
		{\
            if ((d) & (j))/* check duplicated mask with check flag */\
			{\
				result = ISP_MESSAGE_DUPLICATED_TDR_DUMP_MASK_ERROR;\
				tile_driver_printf("Error: %s\n", tile_print_error_message(result));\
				tile_driver_printf("Duplicated register: %s, mask: %s\n\n", #i, #j);\
			}\
			else\
			{\
				int count;\
				/* shift field & merge */\
				for (count=0;count<32;count++)\
				{\
					/* found nonzero lsb */\
					if (0x1 & ((j)>>count))\
					{\
						(b) |= ((((a)->i) << count) & (j));\
						/* printf("hex no: %d, dump name: %s\n", (f), #i); */\
						break;\
					}\
				}\
				/* or mask count */\
				(d) |= (j);\
			}\
		}\
    }\

#define TILE_TDR_INV_REG_COPY(a, b, c, d, e, f, g, h, i, j, k, m, n) \
	if (1 == (c)->i)\
	{\
		(a)->i = (b)->i;\
	}\

#define TILE_TCM_REG_COPY(a, b, c, d, e, f, g, h, i, j, k, m, n) (g) = (a)->i;

#define TILE_WRAPPER_TDR_PRINT_REG_BY_FLAG(a, b, c, d, e, f, g, h, i, j, k, m, n) \
	if (1 == (c)->i)\
	{\
		fprintf(a, "%s%s%d\n", #i, TILE_DEBUG_SPACE_EQUAL_SYMBOL_STR, (b)->i);\
	}\

#define TILE_COPY_INV_REG(a, b, c, d, e, f, max_count, result, g, h, i, j, k) \
    /* valid reg */\
    if (true == (h))\
    {\
        (k) = (a)->i;\
    }\

/* register to dump */
//a: ptr of register table
//b: ptr of register flag
//c: current id
//d: merge count per dump
//e: ptr of dump word
//f: word count or offset to dump
#define TILE_DUMP_INV_REG(a, b, c, d, e, f, max_count, result, g, h, i, j) \
    /* different id or word */\
	if (ISP_MESSAGE_OK == result)\
	{\
		if ((g) != (c))\
		{\
			/* last merge count > 0 */\
			if ((d) > 0)\
			{\
				(f)++;\
				(d) = 0;\
			}\
			/* update current id */\
			(c) = (g);\
		}\
		/* valid reg */\
		if (true == (h))\
		{\
			if ((d) & (j))\
			{\
				result = ISP_MESSAGE_DUPLICATED_TDR_DUMP_MASK_ERROR;\
				printf("Error: %s\n", tile_print_error_message(result));\
				printf("Duplicated register: %s, mask: %s\n\n", #i, #j);\
			}\
			else\
			{\
				int count;\
				/* shift field & merge */\
				for (count=0;count<32;count++)\
				{\
					/* found nonzero lsb */\
					if (0x1 & ((j)>>count))\
					{\
						/* prevent corrupt memory */\
						if ((f) <  max_count)\
						{\
							(a)->i = (*((e) + (f)) >> count) & ((j) >> count);\
							(b)->i = 1;\
						}\
                        /* debug display */\
                        /*printf("hex no: %02d, inv dump name: %s\n", (f), #i);*/\
						break;\
					}\
				}\
				/* or mask count */\
				(d) |= (j);\
			}\
		}\
    }\

/* register to dump */
//a: file ptr
//b: not use
//c: current id
//d: merge count per dump
//e: ptr of dump word
//f: word count or offset to dump
#define TILE_FPRINT_TDR_DUMP_HEX(a, b, c, d, e, f, max_count, result, g, h, i, j) \
    /* different id or word */\
    if (ISP_MESSAGE_OK == result)\
    {\
        if ((g) != (c))\
        {\
            /* last merge count > 0 */\
            if ((d) > 0)\
            {\
                (f)++;\
                (d) = 0;\
            }\
            /* update current id */\
            (c) = (g);\
        }\
        /* valid reg */\
        if (true == (h))\
        {\
            if ((d) & (j))\
            {\
                result = ISP_MESSAGE_DUPLICATED_TDR_DUMP_MASK_ERROR;\
                printf("Error: %s\n", tile_print_error_message(result));\
                printf("Duplicated register: %s, mask: %s\n\n", #i, #j);\
            }\
            else\
            {\
                int count;\
                if (0 == (d))\
                    {\
                        /* prevent corrupt memory */\
                        if ((f) <  max_count)\
                        {\
                            fprintf(b, "\n(%d) 0x%08X", (f), *((e) + (f)));\
                        }\
                    }\
                /* shift field & merge */\
                for (count=0;count<32;count++)\
                {\
                    /* found nonzero lsb */\
                    if (0x1 & ((j)>>count))\
                    {\
                        /* prevent corrupt memory */\
                        if ((f) <  max_count)\
                        {\
                            int val = (*((e) + (f)) >> count) & ((j) >> count);\
                            fprintf(b, " [%s][0x%X]=%d", #i, (j), val);\
                        }\
						/*printf("hex no: %d, inv dump name: %s\n", (f), #i);*/\
						break;\
					}\
				}\
				/* or mask count */\
				(d) |= (j);\
			}\
		}\
    }\

#define TILE_INVERSE_FUNC_X_ORDER(a) \
	(a)->out_stream_order = ((a)->in_stream_order & TILE_ORDER_RIGHT_TO_LEFT)?\
	((a)->in_stream_order & (~TILE_ORDER_RIGHT_TO_LEFT)):((a)->in_stream_order | TILE_ORDER_RIGHT_TO_LEFT);\
	(a)->out_cal_order = ((a)->in_cal_order & TILE_ORDER_RIGHT_TO_LEFT)?\
	((a)->in_cal_order & (~TILE_ORDER_RIGHT_TO_LEFT)):((a)->in_cal_order | TILE_ORDER_RIGHT_TO_LEFT);\
	(a)->out_dump_order = ((a)->in_dump_order & TILE_ORDER_RIGHT_TO_LEFT)?\
	((a)->in_dump_order & (~TILE_ORDER_RIGHT_TO_LEFT)):((a)->in_dump_order | TILE_ORDER_RIGHT_TO_LEFT);\

#define TILE_COPY_SRC_ORDER(a, b) \
	(a)->in_stream_order = (b)->src_stream_order;\
	(a)->in_cal_order = (b)->src_cal_order;\
	(a)->in_dump_order = (b)->src_dump_order;\
	(a)->out_stream_order = (b)->src_stream_order;\
	(a)->out_cal_order = (b)->src_cal_order;\
	(a)->out_dump_order = (b)->src_dump_order;\

#define TILE_COPY_PRE_ORDER(a, b) \
	(a)->in_stream_order = (b)->out_stream_order;\
	(a)->in_cal_order = (b)->out_cal_order;\
	(a)->in_dump_order = (b)->out_dump_order;\
	(a)->out_stream_order = (b)->out_stream_order;\
	(a)->out_cal_order = (b)->out_cal_order;\
	(a)->out_dump_order = (b)->out_dump_order;\

/* tile reg & variable */
typedef struct TILE_REG_MAP_STRUCT
{
    /* COMMON */
	TILE_INTERNAL_REG_LUT(COMMON, TILE_HW_REG_TYPE_DECLARE,,,,,)
	/* ISP */
	TILE_SW_REG_LUT(ISP, TILE_HW_REG_TYPE_DECLARE,,,,,)
    TILE_HW_REG_LUT(ISP, TILE_HW_REG_TYPE_DECLARE,,,,,)
    TILE_INTERNAL_TDR_REG_LUT(ISP, TILE_HW_REG_TYPE_DECLARE,,,,,)
	TILE_INTERNAL_REG_LUT(ISP, TILE_HW_REG_TYPE_DECLARE,,,,,)
    TILE_CMODEL_REG_LUT(ISP, TILE_HW_REG_TYPE_DECLARE,,,,,)
    TILE_CMODEL_PATH_LUT(ISP, TILE_HW_REG_TYPE_DECLARE,,,,,)
    TILE_PLATFORM_REG_LUT(ISP, TILE_HW_REG_TYPE_DECLARE,,,,,)
    TILE_PLATFORM_DEBUG_REG_LUT(ISP, TILE_HW_REG_TYPE_DECLARE,,,,,)
	/* MDP */
	TILE_SW_REG_LUT(MDP, TILE_HW_REG_TYPE_DECLARE,,,,,)
    TILE_HW_REG_LUT(MDP, TILE_HW_REG_TYPE_DECLARE,,,,,)
    TILE_INTERNAL_TDR_REG_LUT(MDP, TILE_HW_REG_TYPE_DECLARE,,,,,)
	TILE_INTERNAL_REG_LUT(MDP, TILE_HW_REG_TYPE_DECLARE,,,,,)
    TILE_CMODEL_REG_LUT(MDP, TILE_HW_REG_TYPE_DECLARE,,,,,)
    TILE_CMODEL_PATH_LUT(MDP, TILE_HW_REG_TYPE_DECLARE,,,,,)
    TILE_PLATFORM_REG_LUT(MDP, TILE_HW_REG_TYPE_DECLARE,,,,,)
    TILE_PLATFORM_DEBUG_REG_LUT(MDP, TILE_HW_REG_TYPE_DECLARE,,,,,)
}TILE_REG_MAP_STRUCT;

#define TILE_CHECK_RESULT(result) \
    if (ISP_MESSAGE_OK != (result)) return(result);\


/* tile reg & variable */
typedef struct TILE_CMODEL_TDR_REG_STRUCT
{
    TILE_INTERNAL_TDR_REG_LUT(ISP, TILE_HW_REG_TYPE_DECLARE,,,,,)
    TILE_INTERNAL_TDR_REG_LUT(MDP, TILE_HW_REG_TYPE_DECLARE,,,,,)
}TILE_CMODEL_TDR_REG_STRUCT;

/* TDR by register name */
typedef struct TILE_TDR_REG_STRUCT
{
    char name[MAX_REGISTER_STRING_LENGTH];
    unsigned int val;
    unsigned int tile_no;
}TILE_TDR_REG_STRUCT;

typedef struct TILE_REG_DUMP_STRUCT
{
    TILE_TDR_REG_STRUCT tile_reg[MAX_TILE_TDR_REG_NO];
    unsigned int used_reg_no;
}TILE_REG_DUMP_STRUCT;

typedef struct TILE_RESIZER_PARA_BUFFER
{
    TILE_FUNC_BLOCK_LUT(TILE_WRAPPER_HORZ_PARA_DECLARE,,,,,)
}TILE_RESIZER_PARA_BUFFER;

typedef struct TILE_RESIZER_FORWARD_CAL_ARG_STRUCT
{
    int mode;
    int in_pos_start;
    int in_pos_end;
    int bias;
    int offset;
    int in_bias;
    int in_offset;
    int in_bias_c;
    int in_offset_c;
    int prec_bits;
    int align_flag;/* CAM_UV_444_FLAG (1), CAM_UV_422_FLAG (0) */
    int uv_flag;/* CAM_UV_444_FLAG (1), CAM_UV_422_FLAG (0) */
    int max_in_pos_end;
    int max_out_pos_end;
    int backward_out_pos_start;
    int out_pos_start;/* output */
    int out_pos_end;/* output */
    int dir_mode;/* CAM_DIR_X (0), CAM_DIR_Y (1) */
	int coeff_step;
    int offset_cal_start;
}TILE_RESIZER_FORWARD_CAL_ARG_STRUCT;

typedef struct TILE_RESIZER_BACKWARD_CAL_ARG_STRUCT
{
    int mode;
    int out_pos_start;
    int out_pos_end;
    int bias;
    int offset;
    int prec_bits;
    int align_flag;/* CAM_UV_444_FLAG (1), CAM_UV_422_FLAG (0) */
    int uv_flag;/* CAM_UV_444_FLAG (1), CAM_UV_422_FLAG (0) */
    int max_in_pos_end;
    int max_out_pos_end;
    int in_pos_start;/* output */
    int in_pos_end;/* output */
    int dir_mode;/* CAM_DIR_X (0), CAM_DIR_Y (1) */
    int coeff_step;
}TILE_RESIZER_BACKWARD_CAL_ARG_STRUCT;

typedef struct TILE_FUNC_BLOCK_STRUCT
{
    TILE_FUNC_BLOCK_LUT(TILE_WRAPPER_DATA_TYPE_DECLARE,,,,,)
    ISP_MESSAGE_ENUM (*init_func_ptr)(struct TILE_FUNC_BLOCK_STRUCT *ptr_func, TILE_REG_MAP_STRUCT* ptr_tile_reg_map);
    ISP_MESSAGE_ENUM (*for_func_ptr)(struct TILE_FUNC_BLOCK_STRUCT *ptr_func, TILE_REG_MAP_STRUCT* ptr_tile_reg_map);
    ISP_MESSAGE_ENUM (*back_func_ptr)(struct TILE_FUNC_BLOCK_STRUCT *ptr_func, TILE_REG_MAP_STRUCT* ptr_tile_reg_map);
    ISP_MESSAGE_ENUM (*tdr_func_ptr)(struct TILE_FUNC_BLOCK_STRUCT *ptr_func, TILE_REG_MAP_STRUCT* ptr_tile_reg_map);
    struct TILE_FUNC_DATA_STRUCT *func_data;
}TILE_FUNC_BLOCK_STRUCT;

typedef struct TILE_CAL_FUNC_STRUCT
{
    ISP_MESSAGE_ENUM (*init_func_ptr)(TILE_FUNC_BLOCK_STRUCT *ptr_func, TILE_REG_MAP_STRUCT* ptr_tile_reg_map);
    ISP_MESSAGE_ENUM (*for_func_ptr)(TILE_FUNC_BLOCK_STRUCT *ptr_func, TILE_REG_MAP_STRUCT* ptr_tile_reg_map);
    ISP_MESSAGE_ENUM (*back_func_ptr)(TILE_FUNC_BLOCK_STRUCT *ptr_func, TILE_REG_MAP_STRUCT* ptr_tile_reg_map);
    ISP_MESSAGE_ENUM (*tdr_func_ptr)(TILE_FUNC_BLOCK_STRUCT *ptr_func, TILE_REG_MAP_STRUCT* ptr_tile_reg_map,
        TILE_REG_DUMP_STRUCT* ptr_tile_reg_dump_param);
}TILE_CAL_FUNC_STRUCT;

typedef struct TILE_FUNC_ENABLE_STRUCT
{
	TILE_FUNC_ID_ENUM func_num;
	bool enable_flag;
    bool output_disable_flag;
}TILE_FUNC_ENABLE_STRUCT;

typedef struct TILE_FUNC_SUBRDMA_STRUCT
{
	TILE_FUNC_ID_ENUM func_num;
	TILE_FUNC_ID_ENUM master_func_num;
	bool enable_flag;
	unsigned int func_ptr_flag;
    TILE_GROUP_NUM_ENUM group_num;
}TILE_FUNC_SUBRDMA_STRUCT;

/* tile function interface to be compatiable with new c model */
typedef struct FUNC_DESCRIPTION_STRUCT
{
    TILE_FUNC_BLOCK_STRUCT func_list[MAX_TILE_FUNC_NO];
	TILE_FUNC_ENABLE_STRUCT func_en_list[MAX_TILE_FUNC_EN_NO];
	TILE_FUNC_SUBRDMA_STRUCT func_subrdma_list[MAX_TILE_FUNC_SUBRDMA_NO];
    unsigned char scheduling_forward_order[MAX_TILE_FUNC_NO];
    unsigned char scheduling_backward_order[MAX_TILE_FUNC_NO];
    unsigned char used_func_no;
    unsigned char used_en_func_no;
    unsigned char used_subrdma_func_no;
    unsigned int  valid_flag[(MAX_TILE_FUNC_NO+31)/32];
    unsigned int for_recursive_count;
}FUNC_DESCRIPTION_STRUCT;

typedef struct TILE_INFORMATION_STRUCT
{
    unsigned int pos_xs;/* tile start */
    unsigned int pos_xe;/* tile end */
    unsigned int pos_ys;/* tile start */
    unsigned int pos_ye;/* tile end */
    unsigned int tile_stop_flag;/* stop flag */
    unsigned int dump_offset_no;/* word offset */
}TILE_INFORMATION_STRUCT;

typedef struct ISP_TILE_HEX_DUMP_STRUCT
{
    unsigned int tile_config[MAX_ISP_TILE_TDR_HEX_NO];
    TILE_INFORMATION_STRUCT tile_info[MAX_TILE_TOT_NO];
    unsigned int used_word_no;
    unsigned int total_word_no;
    unsigned int config_no_per_tile;
    unsigned int curr_vertical_tile_no;
    unsigned int horizontal_tile_no;
    unsigned int used_tile_no;
    unsigned int total_tile_no;
    unsigned int curr_horizontal_tile_no;
}ISP_TILE_HEX_DUMP_STRUCT;

typedef struct MDP_TILE_INFORMATION_STRUCT
{
    unsigned int pos_xs;/* tile start */
    unsigned int pos_xe;/* tile end */
    unsigned int pos_ys;/* tile start */
    unsigned int pos_ye;/* tile end */
    unsigned int tile_stop_flag;/* stop flag */
    unsigned int dump_offset_no;/* word offset */
}MDP_TILE_INFORMATION_STRUCT;

typedef struct MDP_TILE_HEX_DUMP_STRUCT
{
    unsigned int tile_config[MAX_MDP_TILE_TDR_HEX_NO];
    MDP_TILE_INFORMATION_STRUCT tile_info[MAX_TILE_TOT_NO];
    unsigned int used_word_no;
    unsigned int total_word_no;
    unsigned int config_no_per_tile;
    unsigned int curr_vertical_tile_no;
    unsigned int horizontal_tile_no;
    unsigned int used_tile_no;
    unsigned int total_tile_no;
    unsigned int curr_horizontal_tile_no;
}MDP_TILE_HEX_DUMP_STRUCT;

typedef struct TILE_PARAM_CMODEL_STRUCT
{
    TILE_REG_MAP_STRUCT tile_reg_map;
    FUNC_DESCRIPTION_STRUCT tile_func_param;
    TILE_REG_DUMP_STRUCT  tile_reg_dump_param;
    ISP_TILE_HEX_DUMP_STRUCT isp_tile_hex_dump_param;
    MDP_TILE_HEX_DUMP_STRUCT mdp_tile_hex_dump_param;
}TILE_PARAM_CMODEL_STRUCT;

typedef struct TILE_PARAM_UT_STRUCT
{
    TILE_REG_MAP_STRUCT *ptr_tile_reg_map;
    FUNC_DESCRIPTION_STRUCT *ptr_tile_func_param;
    TILE_REG_DUMP_STRUCT  *ptr_tile_reg_dump_param;
    ISP_TILE_HEX_DUMP_STRUCT *ptr_isp_tile_hex_dump_param;
    MDP_TILE_HEX_DUMP_STRUCT *ptr_mdp_tile_hex_dump_param;
}TILE_PARAM_UT_STRUCT;

typedef struct ISP_TILE_WORKING_BUFFER_PARAM_STRUCT
{
    TILE_REG_MAP_STRUCT tile_reg_map;
    FUNC_DESCRIPTION_STRUCT tile_func_param;
    MDP_TILE_HEX_DUMP_STRUCT mdp_tile_hex_dump_param;
}ISP_TILE_WORKING_BUFFER_PARAM_STRUCT;

typedef struct MDP_TILE_CONFIG_STRUCT
{
	TILE_SW_REG_LUT(MDP, TILE_WRAPPER_DATA_TYPE_DECLARE,,,,,)
    TILE_HW_REG_LUT(MDP, TILE_WRAPPER_DATA_TYPE_DECLARE,,,,,)
    TILE_PLATFORM_REG_LUT(MDP, TILE_WRAPPER_DATA_TYPE_DECLARE,,,,,)
}MDP_TILE_CONFIG_STRUCT;

/* current c model ut wrapper */
extern ISP_MESSAGE_ENUM tile_main_ut(TILE_PARAM_UT_STRUCT *ptr_ut_tile_param);
extern int tile_main_query_working_buffer_size(void);
/* tile isp extern */
extern bool tile_init_isp_func_property(TILE_FUNC_BLOCK_STRUCT *ptr_func, TILE_REG_MAP_STRUCT *ptr_tile_reg_map);
extern ISP_MESSAGE_ENUM tile_lut_isp_func_support(int *module_no, TILE_FUNC_BLOCK_STRUCT *ptr_func,
							TILE_REG_MAP_STRUCT *ptr_tile_reg_map);
extern ISP_MESSAGE_ENUM tile_lut_isp_func_enable(int *module_no, TILE_FUNC_ENABLE_STRUCT *ptr_func_en,
							TILE_REG_MAP_STRUCT *ptr_tile_reg_map);
extern ISP_MESSAGE_ENUM tile_lut_isp_func_subrdma(int *module_no, TILE_FUNC_SUBRDMA_STRUCT *ptr_func_subrdma,
							TILE_REG_MAP_STRUCT *ptr_tile_reg_map);
extern bool tile_init_isp_func_subrdma_tdr(TILE_FUNC_SUBRDMA_STRUCT *ptr_tile_func_param);
extern ISP_MESSAGE_ENUM tile_init_isp_func_run(TILE_FUNC_BLOCK_STRUCT *ptr_func, TILE_REG_MAP_STRUCT *ptr_tile_reg_map,
							bool *found_flag);
extern ISP_MESSAGE_ENUM tile_for_isp_func_run(TILE_FUNC_BLOCK_STRUCT *ptr_func, TILE_REG_MAP_STRUCT *ptr_tile_reg_map,
							bool *found_flag);
extern ISP_MESSAGE_ENUM tile_back_isp_func_run(TILE_FUNC_BLOCK_STRUCT *ptr_func, TILE_REG_MAP_STRUCT *ptr_tile_reg_map,
							bool *found_flag);
extern ISP_MESSAGE_ENUM tile_tdr_isp_func_run(int func_num, bool enable_flag,
						   TILE_FUNC_BLOCK_STRUCT *ptr_func, TILE_REG_MAP_STRUCT *ptr_tile_reg_map,
						   TILE_REG_DUMP_STRUCT *ptr_tile_reg_dump_param, bool *found_flag);
extern ISP_MESSAGE_ENUM tile_tdr_isp_reg_to_dump(TILE_REG_MAP_STRUCT *ptr_tile_reg_map, unsigned int *hex_ptr,
							unsigned int *hex_offset, unsigned int max_count);
extern ISP_MESSAGE_ENUM tile_tdr_isp_dump_inv_reg(TILE_REG_MAP_STRUCT *ptr_tile_reg_map,
							TILE_REG_MAP_STRUCT *ptr_tile_reg_map_flag, int tile_id,
							unsigned int *hex_ptr, unsigned int max_count,
							unsigned int config_no_per_tile);
/* tile mdp extern */
extern bool tile_init_mdp_func_property(TILE_FUNC_BLOCK_STRUCT *ptr_func, TILE_REG_MAP_STRUCT *ptr_tile_reg_map);
extern ISP_MESSAGE_ENUM tile_lut_mdp_func_support(int *module_no, TILE_FUNC_BLOCK_STRUCT *ptr_func,
							TILE_REG_MAP_STRUCT *ptr_tile_reg_map);
extern ISP_MESSAGE_ENUM tile_lut_mdp_func_enable(int *module_no, TILE_FUNC_ENABLE_STRUCT *ptr_func_en,
							TILE_REG_MAP_STRUCT *ptr_tile_reg_map);
extern ISP_MESSAGE_ENUM tile_lut_mdp_func_subrdma(int *module_no, TILE_FUNC_SUBRDMA_STRUCT *ptr_func_subrdma,
							TILE_REG_MAP_STRUCT *ptr_tile_reg_map);
extern bool tile_init_mdp_func_subrdma_tdr(TILE_FUNC_SUBRDMA_STRUCT *ptr_func_subrdma);
extern ISP_MESSAGE_ENUM tile_init_mdp_func_run(TILE_FUNC_BLOCK_STRUCT *ptr_func, TILE_REG_MAP_STRUCT *ptr_tile_reg_map,
							bool *found_flag);
extern ISP_MESSAGE_ENUM tile_for_mdp_func_run(TILE_FUNC_BLOCK_STRUCT *ptr_func, TILE_REG_MAP_STRUCT *ptr_tile_reg_map,
							bool *found_flag);
extern ISP_MESSAGE_ENUM tile_back_mdp_func_run(TILE_FUNC_BLOCK_STRUCT *ptr_func, TILE_REG_MAP_STRUCT *ptr_tile_reg_map,
							bool *found_flag);
extern ISP_MESSAGE_ENUM tile_tdr_mdp_func_run(int func_num, bool enable_flag,
						   TILE_FUNC_BLOCK_STRUCT *ptr_func, TILE_REG_MAP_STRUCT *ptr_tile_reg_map,
						   TILE_REG_DUMP_STRUCT *ptr_tile_reg_dump_param, bool *found_flag);
extern ISP_MESSAGE_ENUM tile_tdr_mdp_reg_to_dump(TILE_REG_MAP_STRUCT *ptr_tile_reg_map, unsigned int *hex_ptr,
							unsigned int *hex_offset, unsigned int max_count);
extern ISP_MESSAGE_ENUM tile_tdr_mdp_dump_inv_reg(TILE_REG_MAP_STRUCT *ptr_tile_reg_map,
							TILE_REG_MAP_STRUCT *ptr_tile_reg_map_flag, int tile_id,
							unsigned int *hex_ptr, unsigned int max_count,
							unsigned int config_no_per_tile);
/* tile utility.h extern */
extern ISP_MESSAGE_ENUM tile_for_comp_resizer(TILE_RESIZER_FORWARD_CAL_ARG_STRUCT *ptr_for_arg,
                                              TILE_FUNC_BLOCK_STRUCT *ptr_func);
extern ISP_MESSAGE_ENUM tile_back_comp_resizer(TILE_RESIZER_BACKWARD_CAL_ARG_STRUCT *ptr_back_arg,
                                              TILE_FUNC_BLOCK_STRUCT *ptr_func);
#ifdef __cplusplus
extern "C"{
#endif
/* current c model extern variables with c define to handle c & cpp link issue */
extern ISP_MESSAGE_ENUM tile_main_current_cmodel(TILE_PARAM_CMODEL_STRUCT *ptr_cmodel_tile_param);
extern ISP_MESSAGE_ENUM tile_main_read_mdp_reg_file(TILE_REG_MAP_STRUCT *ptr_tile_reg_map,
										 const char* filename, const char* foldername);
/* print message api */
extern const char *tile_print_error_message(ISP_MESSAGE_ENUM n);
extern const char *get_current_file_name(const char *filename);
#ifdef __cplusplus
}
#endif
#endif

