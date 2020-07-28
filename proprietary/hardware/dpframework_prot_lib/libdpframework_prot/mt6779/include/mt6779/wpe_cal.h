#ifndef _WPE_TILE_H_
#define _WPE_TILE_H_

#define WPE_PLATFORM_DRIVER /* to change define with platform unique */

/* following define can be changed */
#define TPIPE_WPE_BASE_HW     (0x1502A000)
#define TPIPE_WPE_ADDR_START  (0x1502A000)
#define TPIPE_WPE_ADDR_END    (0x1502A510)

#if defined(_MSC_VER)
#include <conio.h>
#define wpe_kbhit() _kbhit()
#define wpe_stricmp                                             _stricmp
#define WPE_FOLDER_SYMBOL_STR                                       "\\"
#define WPE_FOLDER_SYMBOL_CHAR                                      '\\'
#define wpe_sprintf(dst_ptr, size_dst, format, ...)             sprintf_s(dst_ptr, size_dst, format, __VA_ARGS__)
#define wpe_fopen(file_ptr, filename_ptr, mode)                 fopen_s(&file_ptr, filename_ptr, mode)
#define wpe_fscanf                                              fscanf_s
#define WPE_MOVE_CMD                                                "move"
#define WPE_COPY_CMD                                                "copy"
#define WPE_CMP_CMD                                                 "fc"
#define WPE_DEL_CMD                                                 "del"
#define wpe_sscanf_1(...)                               sscanf_s(__VA_ARGS__)
#define wpe_sscanf_2(...)							sscanf_s(__VA_ARGS__)
#define wpe_sscanf_8(...)							sscanf_s(__VA_ARGS__)
//	use for non-string only
#define wpe_sscanf_2d(src_ptr, format_ptr, dst_ptr_1, dst_ptr_2, size_1, size_2)							sscanf_s(src_ptr, format_ptr, dst_ptr_1, dst_ptr_2)
#define wpe_sscanf_3d(src_ptr, format_ptr, dst_ptr_1, dst_ptr_2, dst_ptr_3, size_1, size_2, size_3)				sscanf_s(src_ptr, format_ptr, dst_ptr_1, dst_ptr_2, dst_ptr_3)
#define wpe_sscanf_4d(src_ptr, format_ptr, dst_ptr_1, dst_ptr_2, dst_ptr_3, dst_ptr_4, size_1, size_2, size_3, size_4)	sscanf_s(src_ptr, format_ptr, dst_ptr_1, dst_ptr_2, dst_ptr_3, dst_ptr_4)
//	use for string only
#define wpe_sscanf_2s(src_ptr, format_ptr, dst_ptr_1, dst_ptr_2, size_1, size_2)							sscanf_s(src_ptr, format_ptr, dst_ptr_1, size_1, dst_ptr_2, size_2)
#define wpe_sscanf_3s(src_ptr, format_ptr, dst_ptr_1, dst_ptr_2, dst_ptr_3, size_1, size_2, size_3)				sscanf_s(src_ptr, format_ptr, dst_ptr_1, size_1, dst_ptr_2, size_2, dst_ptr_3, size_3)
#define wpe_sscanf_4s(src_ptr, format_ptr, dst_ptr_1, dst_ptr_2, dst_ptr_3, dst_ptr_4, size_1, size_2, size_3, size_4)	sscanf_s(src_ptr, format_ptr, dst_ptr_1, size_1, dst_ptr_2, size_2, dst_ptr_3, size_3, dst_ptr_4, size_4)
#elif defined(linux) || defined(__linux) || defined(__arm__)
#define wpe_kbhit() true
#define wpe_stricmp                                             strcasecmp
#define WPE_FOLDER_SYMBOL_STR                                       "/"
#define WPE_FOLDER_SYMBOL_CHAR                                      '/'
#define wpe_sprintf(dst_ptr, size_dst, ...)                     sprintf(dst_ptr, __VA_ARGS__)
#define wpe_fopen(file_ptr, filename_ptr, mode)                 file_ptr = fopen(filename_ptr, mode)
#define wpe_fscanf                                              fscanf
#define WPE_MOVE_CMD                                                "mv"
#define WPE_COPY_CMD                                                "cp"
#define WPE_CMP_CMD                                                 "cmp"
#define WPE_DEL_CMD                                                 "rm"
#define wpe_sscanf_1(x, y, a, b)                               sscanf(x, y, a)
#define wpe_sscanf_2(x, y, a, b, c, d)		sscanf(x, y, a, b)
#define wpe_sscanf_8(x, y, a, b, c, d, e, f, g, h,...)		sscanf(x, y, a, b, c, d, e, f, g, h)
#define O_BINARY (0)
//	use for non-string only
#define wpe_sscanf_2d	wpe_sscanf_2s
#define wpe_sscanf_3d	wpe_sscanf_3s
#define wpe_sscanf_4d	wpe_sscanf_4s
//	use for string only
#define wpe_sscanf_2s(src_ptr, format_ptr, dst_ptr_1, dst_ptr_2, size_1, size_2)							sscanf(src_ptr, format_ptr, dst_ptr_1, dst_ptr_2)
#define wpe_sscanf_3s(src_ptr, format_ptr, dst_ptr_1, dst_ptr_2, dst_ptr_3,  size_1, size_2, size_3)				sscanf(src_ptr, format_ptr, dst_ptr_1, dst_ptr_2, dst_ptr_3)
#define wpe_sscanf_4s(src_ptr, format_ptr, dst_ptr_1, dst_ptr_2, dst_ptr_3, dst_ptr_4,  size_1, size_2, size_3, size_4)	sscanf(src_ptr, format_ptr, dst_ptr_1, dst_ptr_2, dst_ptr_3, dst_ptr_4)
#else
#error("Non-supported c compiler environment\n")
#endif

#ifdef WPE_PLATFORM_DRIVER
#if defined(_MSC_VER)           // VC
#define wpe_driver_printf wpe_printf_prefix
#define wpe_driver_printf_no_prefix printf
#define wpe_uart_printf printf
#elif defined(linux) || defined(__linux) //Linux
#if defined(__arm__) || defined(__aarch64__) //arm
#define wpe_driver_printf(...)
#define wpe_driver_printf_no_prefix(...)
#define NEW_LINE_CHAR   "\n"
#if defined(USING_MTK_LDVT)     // LDVT
#pragma message("LDVT environment")
#include "uvvf.h"
#define wpe_uart_printf(fmt, arg...) VV_MSG(LOG_TAG "[%s] " fmt NEW_LINE_CHAR, __func__, ##arg)
#else //not LDVT
//#pragma message("FPGA/EVB/Phone environment")
#include <android/log.h>        // Android
#define wpe_driver_printf(fmt, ...) ((void)__android_log_print(ANDROID_LOG_INFO, "WPE/T", "[%s][%d] " fmt, __FILE__, __LINE__, ##__VA_ARGS__))
#define wpe_driver_printf_no_prefix(fmt, ...) ((void)__android_log_print(ANDROID_LOG_INFO, "WPE/T", fmt, ##__VA_ARGS__))
#define wpe_uart_printf(fmt, ...) ((void)__android_log_print(ANDROID_LOG_INFO, "WPE/T", fmt, ##__VA_ARGS__))
#endif //END LDVT
#else //not arm
#define wpe_driver_printf wpe_printf_prefix
#define wpe_driver_printf_no_prefix printf
#define wpe_uart_printf printf
#endif //END platform
#endif //END VC
#else
#define wpe_driver_printf wpe_printf_prefix
#define wpe_driver_printf_no_prefix printf
#define wpe_uart_printf wpe_printf_prefix
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

/* last_irq_mode */
typedef enum WPE_LAST_IRQ_MODE_ENUM
{
    WPE_LAST_IRQ_FRAME_STOP=0,
    WPE_LAST_IRQ_LINE_END,
    WPE_LAST_IRQ_PER_TILE,
    WPE_LAST_IRQ_MODE_MAX_NO
}WPE_LAST_IRQ_MODE_ENUM;

#define WPE_VGEN_OFFSET_SHIFT_FACTOR (24)
#define MAX_WPE_TILE_TOT_NO (256)
#define MAX_WPE_DL_HEX_PER_TILE_WPE (3)
#define MAX_WPE_DUMP_HEX_PER_TILE_WPE (24)
#define MAX_WPE_CAL_HEX_PER_TILE_WPE (8)
#define MAX_WPE_TILE_TDR_HEX_NO_WPE (MAX_WPE_TILE_TOT_NO * MAX_WPE_DUMP_HEX_PER_TILE_WPE)
#define MAX_WPE_DL_TILE_TDR_HEX_NO_WPE (MAX_WPE_TILE_TOT_NO * MAX_WPE_DL_HEX_PER_TILE_WPE)

/* WPE format */
#define WPE_WPEO_FMT_Y (0)     //WPEO
#define WPE_WPEO_FMT_U (1)
#define WPE_WPEO_FMT_V (2)
#define WPE_WPEO_FMT_UV420_1 (3)
#define WPE_WPEO_FMT_UV422_1 (4)
#define WPE_WPEO_FMT_YUV422_1 (5)
#define WPE_WPEO_FMT_FG_1 (6)

/* DL last TDR mask */
#define WPE_DL_LAST_TILE_DEFAULT (0x5a5a)

/* macro for debug print to file, #include <stdio.h> */
#define wpe_printf_prefix(...) {\
    const char *ptr_char = get_current_file_name(__FILE__);\
    printf("[%s][%s][%d] ", ptr_char,  __FUNCTION__, __LINE__);\
    printf(__VA_ARGS__);\
}

#define WPE_ERROR_MESSAGE_DATA(n, CMD) \
    CMD(n, WPE_MESSAGE_OK)\
    CMD(n, WPE_MESSAGE_FILE_OPEN_ERROR)\
    CMD(n, WPE_MESSAGE_NULL_PTR_ERROR)\
    CMD(n, WPE_MESSAGE_MISSING_READ_REG_ERROR)\
    CMD(n, WPE_MESSAGE_INVALID_CONFIG_ERROR)\
	CMD(n, WPE_MESSAGE_RAL_NO_OVER_SIZE_ERROR)\
	CMD(n, WPE_MESSAGE_VAR_NO_OVER_SIZE_ERROR)\
	CMD(n, WPE_MESSAGE_RAL_MASK_CHECK_ERROR)\
	CMD(n, WPE_MESSAGE_DUPLICATED_TDR_DUMP_MASK_ERROR)\
	CMD(n, WPE_MESSAGE_TDR_INV_DIFFERENT_TILE_CONFIG_NO_ERROR)\
	CMD(n, WPE_MESSAGE_INCONSISTENT_TDR_MASK_LSB_ERROR)\
	CMD(n, WPE_MESSAGE_INCONSISTENT_DL_TDR_ERROR)\
	CMD(n, WPE_MESSAGE_OVER_MAX_MASK_WORD_NO_ERROR)\
	CMD(n, WPE_MESSAGE_INCONSISTENT_TDR_DUMP_MASK_ERROR)\
	CMD(n, WPE_MESSAGE_INVALID_SRC_STRIDE_CONFIG_ERROR)\
	CMD(n, WPE_MESSAGE_INVALID_VGEN_CONFIG_ERROR)\
	CMD(n, WPE_MESSAGE_FILE_STAT_QUERY_FAIL)\
	CMD(n, WPE_MESSAGE_FILE_OVER_MAX_BUFFER_SIZE_ERROR)\
	CMD(n, WPE_MESSAGE_OVER_MAX_BUFFER_SIZE_ERROR)\
	CMD(n, WPE_MESSAGE_WPEO_XSIZE_ERROR)\
	CMD(n, WPE_MESSAGE_WPEO_STRIDE_ERROR)\
	CMD(n, WPE_MESSAGE_MSKO_XSIZE_ERROR)\
    CMD(n, WPE_MESSAGE_LAST_IRQ_ERROR)

#define WPE_ENUM_STRING(n, a) if ((a) == (n)) return #a;
#define WPE_ENUM_DECLARE(n, a) a,

/* to prevent from directly calling macro */
#define WPE_GET_ERROR_NAME(n) \
    if (0 == (n)) return "WPE_MESSAGE_UNKNOWN";\
    WPE_ERROR_MESSAGE_DATA(n, WPE_ENUM_STRING)\
    return "";

/* error enum */
typedef enum WPE_MESSAGE_ENUM
{
    WPE_MESSAGE_UNKNOWN=0,
    WPE_ERROR_MESSAGE_DATA(, WPE_ENUM_DECLARE)
}WPE_MESSAGE_ENUM;

#define TDR_REG_TYPE_DECLARE(a, b, c, d, e, f, g, h, i, j, k, m, n, p, q,...) TDR_REG_TYPE_DECLARE_##q(a, b, c, d, e, f, g, h, i, j, k, m, n, p, q,...)
#define TDR_REG_TYPE_DECLARE_U(a, b, c, d, e, f, g, h, i, j, k, m, n, p, q,...) unsigned int m;
#define TDR_REG_TYPE_DECLARE_S(a, b, c, d, e, f, g, h, i, j, k, m, n, p, q,...) int m;
#define WPE_PARSE_TPIPE(a, b, c, d, e, f, g, h, i, j,...) \
{\
	if (false == (d))\
	{\
		if (0 == memcmp(#g, (b), sizeof(#g)))\
		{\
			(a)->g = (c);\
			(d) = true;\
			(e)++;\
		}\
	}\
}\


#define WPE_PARSE_VAR(a, b, c, d, e, f, g, h, i, j,...) \
{\
	if (false == (c))\
	{\
		if (0 == memcmp(#h, (b)->var_name, sizeof(#h)))\
		{\
			(a)->h = (b)->value;\
			(c) = true;\
			(d)++;\
		}\
	}\
}\

/* register to dump */
//a: ptr of register table
//b: word val to dump
//c: current id
//d: merge count per dump
//e: ptr of dump word
//f: word count or offset to dump
#define TDR_MASK_TO_DUMP(a, b, c, d, e, f, max_count, result, word_no, g, h, i, j, k, l,...) \
    /* different id or word */\
    if (WPE_MESSAGE_OK == (result))\
    {\
        if ((g) != (c))\
        {\
			/* prevent corrupt memory */\
            (d)++;\
            /* update current id */\
            (c) = (g);\
			if (b)\
			{\
				(e)++;\
				(b) = 0;\
			}\
        }\
        /* valid reg */\
        if (h)\
        {\
            if ((b) & j)/* check duplicated mask with check flag */\
            {\
				(result) = WPE_MESSAGE_DUPLICATED_TDR_DUMP_MASK_ERROR;\
                wpe_driver_printf("Error: %s\r\n", wpe_print_error_message(result));\
                wpe_driver_printf("Duplicated register: %s, mask: 0x%08X\r\n", #i, j);\
            }\
            else\
            {\
				if (((unsigned int)1<<k) == (j & (((unsigned int)1<<k) + (((unsigned int)1<<k) - 1))))\
				{\
					/* or mask count */\
					(b) |= j;\
				}\
				else\
				{\
					(result) = WPE_MESSAGE_INCONSISTENT_TDR_MASK_LSB_ERROR;\
					wpe_driver_printf("Error: %s\r\n", wpe_print_error_message(result));\
					wpe_driver_printf("Error register: %s, mask: 0x%08X, lsb: 0x%08X\r\n", #i, j, 1<<k);\
				}\
            }\
        }\
    }\

/* register to dump */
//a: ptr of register table
//b: ptr of register flag
//c: current id
//d: merge count per dump
//e: ptr of dump word
//f: word count or offset to dump
#define TDR_DUMP_INV_REG(a, b, c, d, e, f, max_count, result, word_no, g, h, i, j, k, l,...) TDR_DUMP_INV_REG_##l(a, b, c, d, e, f, max_count, result, word_no, g, h, i, j, k, l,...)
#define TDR_DUMP_INV_REG_U(a, b, c, d, e, f, max_count, result, word_no, g, h, i, j, k, l,...) \
    /* different id or word */\
	if (WPE_MESSAGE_OK == (result))\
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
			(c) = g;\
		}\
		/* valid reg */\
		if (h)\
		{\
			if ((d) & j)\
			{\
				(result) = WPE_MESSAGE_DUPLICATED_TDR_DUMP_MASK_ERROR;\
				wpe_driver_printf("Error: %s\r\n", wpe_print_error_message(result));\
				wpe_driver_printf("Duplicated register: %s, mask: 0x%08X\r\n", #i, j);\
			}\
			else\
			{\
				if (((unsigned int)1<<k) == (j & (((unsigned int)1<<k) + (((unsigned int)1<<k) - 1))))\
				{\
					/* prevent corrupt memory */\
					if ((f) < (max_count))\
					{\
						(b)->i = (*((e) + (f)) >> k) & (j >> k);\
					}\
                    /* debug display */\
                    /* wpe_driver_printf("hex no: %02d, inv dump name: %s\r\n", (f), #i); */\
					/* or mask count */\
					(d) |= j;\
				}\
				else\
				{\
					(result) = WPE_MESSAGE_INCONSISTENT_TDR_MASK_LSB_ERROR;\
					wpe_driver_printf("Error: %s\r\n", wpe_print_error_message(result));\
					wpe_driver_printf("Error register: %s, mask: 0x%08X, lsb: 0x%08X\r\n", #i, j, 1<<k);\
				}\
			}\
		}\
    }\

#define TDR_DUMP_INV_REG_S(a, b, c, d, e, f, max_count, result, word_no, g, h, i, j, k, l,...) \
    /* different id or word */\
	if (WPE_MESSAGE_OK == (result))\
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
		if (h)\
		{\
			if ((d) & j)\
			{\
				(result) = WPE_MESSAGE_DUPLICATED_TDR_DUMP_MASK_ERROR;\
				wpe_driver_printf("Error: %s\r\n", wpe_print_error_message(result));\
				wpe_driver_printf("Duplicated register: %s, mask: 0x%08X\r\n", #i, j);\
			}\
			else\
			{\
				if (((unsigned int)1<<k) == (j & (((unsigned int)1<<k) + (((unsigned int)1<<k) - 1))))\
				{\
					/* prevent corrupt memory */\
					if ((f) < (max_count))\
					{\
						int temp_reg = (*((e) + (f)) >> k) & (j >> k);\
						if (temp_reg & (0x1 + (j >> (k + 1))))\
						{\
							(b)->i = -(int)((j >> k) - (unsigned int)temp_reg + 1);\
						}\
						else\
						{\
							(b)->i = temp_reg;\
						}\
					}\
                    /* debug display */\
                    wpe_driver_printf("hex no: %02d, inv dump name: %s\r\n", (f), #i);\
					/* or mask count */\
					(d) |= j;\
				}\
				else\
				{\
					(result) = WPE_MESSAGE_INCONSISTENT_TDR_MASK_LSB_ERROR;\
					wpe_driver_printf("Error: %s\r\n", wpe_print_error_message(result));\
					wpe_driver_printf("Error register: %s, mask: 0x%08X, lsb: 0x%08X\r\n", #i, j, 1<<k);\
				}\\
			}\
		}\
    }\

/* register to dump */
//a: ptr of register table
//b: word val to dump
//c: current id
//d: merge count per dump
//e: ptr of dump word
//f: word count or offset to dump
#define TDR_REG_TO_DUMP(a, b, c, d, e, f, max_count, result, out, g, h, i, j, k, l,...) TDR_REG_TO_DUMP_##l(a, b, c, d, e, f, max_count, result, out, g, h, i, j, k, l,...)
#define TDR_REG_TO_DUMP_U(a, b, c, d, e, f, max_count, result, out, g, h, i, j, k, l,...) \
    /* different id or word */\
    if (WPE_MESSAGE_OK == (result))\
    {\
        if ((g) != (c))\
        {\
            /* last merge count > 0 */\
            if ((d) > 0)\
            {\
                /* prevent corrupt memory */\
				if (e)\
				{\
					if ((f) <  (max_count))\
					{\
						*((e) + (f)) = (b);\
					}\
					(b) = 0;\
				}\
                (f)++;\
                (d) = 0;\
            }\
            /* update current id */\
            (c) = (g);\
        }\
        /* valid reg */\
        if (h)\
        {\
            if ((d) & j)/* check duplicated mask with check flag */\
            {\
				(result) = WPE_MESSAGE_DUPLICATED_TDR_DUMP_MASK_ERROR;\
                wpe_driver_printf("Error: %s\r\n", wpe_print_error_message(result));\
                wpe_driver_printf("Duplicated register: %s, mask: 0x%08X\r\n", #i, j);\
            }\
            else\
            {\
				if (e)\
				{\
					if (((unsigned int)1<<k) == (j & (((unsigned int)1<<k) + (((unsigned int)1<<k) - 1))))\
					{\
						/* check applied last tdr */\
						(b) |= ((out)->i << k) & j;\
						/* wpe_driver_printf("current hex no: %d, dump name: %s\r\n", (f), #i); */\
						if ((out)->i & (~(j>>k)))\
						{\
							(result) = WPE_MESSAGE_INCONSISTENT_TDR_DUMP_MASK_ERROR;\
							wpe_driver_printf("Error: %s\r\n", wpe_print_error_message(result));\
							wpe_driver_printf("Error hex no: %d, dump name: %s, val: 0x%08X, mask: 0x%08X\r\n", (f), #i, (out)->i, j);\
						}\
					}\
					else\
					{\
						(result) = WPE_MESSAGE_INCONSISTENT_TDR_MASK_LSB_ERROR;\
						wpe_driver_printf("Error: %s\r\n", wpe_print_error_message(result));\
						wpe_driver_printf("Error register: %s, mask: 0x%08X, lsb: 0x%08X\r\n", #i, j, 1<<k);\
					}\
				}\
                /* or mask count */\
                (d) |= j;\
            }\
        }\
    }\

#define TDR_REG_TO_DUMP_S(a, b, c, d, e, f, max_count, result, out, g, h, i, j, k, l,...) \
    /* different id or word */\
    if (WPE_MESSAGE_OK == (result))\
    {\
        if ((g) != (c))\
        {\
            /* last merge count > 0 */\
            if ((d) > 0)\
            {\
                /* prevent corrupt memory */\
				if (e)\
				{\
					if ((f) <  (max_count))\
					{\
						*((e) + (f)) = (b);\
					}\
					(b) = 0;\
				}\
                (f)++;\
                (d) = 0;\
            }\
            /* update current id */\
            (c) = (g);\
        }\
        /* valid reg */\
        if (i)\
        {\
            if ((d) & k)/* check duplicated mask with check flag */\
            {\
				(result) = WPE_MESSAGE_DUPLICATED_TDR_DUMP_MASK_ERROR;\
                wpe_driver_printf("Error: %s\r\n", wpe_print_error_message(result));\
                wpe_driver_printf("Duplicated register: %s, mask: 0x%08X\r\n", #j, k);\
            }\
            else\
            {\
				if (e)\
				{\
					if (((unsigned int)1<<k) == (j & (((unsigned int)1<<k) + (((unsigned int)1<<k) - 1))))\
					{\
						(b) |= ((out)->i << k) & j;\
						/* wpe_driver_printf("current hex no: %d, dump name: %s\r\n", (f), #i; */\
						if ((out)->i >= 0)\
						{\
							if ((out)->i & (~(j>>k)))\
							{\
								(result) = WPE_MESSAGE_INCONSISTENT_TDR_DUMP_MASK_ERROR;\
								wpe_driver_printf("Error: %s\r\n", wpe_print_error_message(result));\
								wpe_driver_printf("Error hex no: %d, dump name: %s, val: 0x%08X, mask: 0x%08X\r\n", (f), #i, (out)->i, j);\
							}\
						}\
						else\
						{\
							if ((out)->i ^ ((~(j>>k)) | ((out)->i & (j>>k))))\
							{\
								(result) = WPE_MESSAGE_INCONSISTENT_TDR_DUMP_MASK_ERROR;\
								wpe_driver_printf("Error: %s\r\n", wpe_print_error_message(result));\
								wpe_driver_printf("Error hex no: %d, dump name: %s, val: 0x%08X, mask: 0x%08X\r\n", (f), #i, (out)->i, j);\
							}\
						}\
					}\
					else\
					{\
						(result) = WPE_MESSAGE_INCONSISTENT_TDR_MASK_LSB_ERROR;\
						wpe_driver_printf("Error: %s\r\n", wpe_print_error_message(result));\
						wpe_driver_printf("Error register: %s, mask: 0x%08X, lsb: 0x%08X\r\n", #i, j, 1<<k);\
					}\
				}\
                /* or mask count */\
                (d) |= j;\
            }\
        }\
    }\

#define WPE_UART_FPRINT_TDR_DUMP_HEX(a, b, c, d, e, f, max_count, result, word_no, g, h, i, j, k, m,...) WPE_UART_FPRINT_TDR_DUMP_HEX_##m(a, b, c, d, e, f, max_count, result, word_no, g, h, i, j, k, m,...)
#define WPE_UART_FPRINT_TDR_DUMP_HEX_U(a, b, c, d, e, f, max_count, result, word_no, g, h, i, j, k, m,...) \
    /* different id or word */\
    if (WPE_MESSAGE_OK == (result))\
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
                (result) = WPE_MESSAGE_DUPLICATED_TDR_DUMP_MASK_ERROR;\
                wpe_driver_printf("Error: %s\r\n", wpe_print_error_message(result));\
                wpe_driver_printf("Duplicated register: %s, mask: 0x%08X\r\n", #i, j);\
            }\
            else\
            {\
                if (0 == (d))\
                {\
                    /* prevent corrupt memory */\
                    if ((f) < (max_count))\
                    {\
                        b("(%d) 0x%08X", (f), *((e) + (f)));\
                    }\
                }\
				if (((unsigned int)1<<k) == (j & (((unsigned int)1<<k) + (((unsigned int)1<<k) - 1))))\
				{\
                    /* prevent corrupt memory */\
                    if ((f) < (max_count))\
                    {\
                        int val = (*((e) + (f)) >> k) & ((j) >> k);\
                        b(" [%s][0x%X]=%d", #i, j, val);\
                    }\
					/* wpe_driver_printf("current hex no: %d, dump name: %s\r\n", (f), #i); */\
				}\
				else\
				{\
					(result) = WPE_MESSAGE_INCONSISTENT_TDR_MASK_LSB_ERROR;\
					wpe_driver_printf("Error: %s\r\n", wpe_print_error_message(result));\
					wpe_driver_printf("Error register: %s, mask: 0x%08X, lsb: 0x%08X\r\n", #i, j, 1<<k);\
				}\
                /* or mask count */\
                (d) |= j;\
            }\
        }\
    }\

#define WPE_UART_FPRINT_TDR_DUMP_HEX_S(a, b, c, d, e, f, max_count, result, word_no, g, h, i, j, k, l, m, n, p,...) \
    /* different id or word */\
    if (WPE_MESSAGE_OK == (result))\
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
                (result) = WPE_MESSAGE_DUPLICATED_TDR_DUMP_MASK_ERROR;\
                wpe_driver_printf("Error: %s\r\n", wpe_print_error_message(result));\
                wpe_driver_printf("Duplicated register: %s, mask: 0x%08X\r\n", #i, j);\
            }\
            else\
            {\
                if (0 == (d))\
                {\
                    /* prevent corrupt memory */\
                    if ((f) < (max_count))\
                    {\
                        b("(%d) 0x%08X", (f), *((e) + (f)));\
                    }\
                }\
				if (((unsigned int)1<<k) == (j & (((unsigned int)1<<k) + (((unsigned int)1<<k) - 1))))\
				{\
                    /* prevent corrupt memory */\
                    if ((f) < (max_count))\
                    {\
                        int val = (*((e) + (f)) >> k) & ((j) >> k);\
						if (val & (0x1 + ((j) >> (k + 1))))\
						{\
							b(" [%s][0x%X]=%d", #i, j, -(int)(((j) >> k) - (unsigned int)val + 1));\
						}\
						else\
						{\
							b(" [%s][0x%X]=%d", #i, j, val);\
						}\
                    }\
                    /* wpe_driver_printf("hex no: %d, dump name: %s\r\n", (f), #i);*/\
				}\
				else\
				{\
					(result) = WPE_MESSAGE_INCONSISTENT_TDR_MASK_LSB_ERROR;\
					wpe_driver_printf("Error: %s\r\n", wpe_print_error_message(result));\
					wpe_driver_printf("Error register: %s, mask: 0x%08X, lsb: 0x%08X, lsb: 0x%08X\r\n", #i, j, 1<<k);\
				}\
                /* or mask count */\
                (d) |= (j);\
            }\
        }\
    }\

#define WPE_DUMP_TPIPE(a, b, c, d, e, f, g, h, i, j,...) b("%s = %d", #g, (a)->g);

/* register table (Cmodel, platform, tile driver) for SW parameters */
/* a, b, c, d, e reserved */
/* register name of current c model */
/* register name of HW ISP & platform parameters */
/* U: unsigned int, S: signed int */
#define WPE_TPIPE_SW_REG_LUT(CMD, a, b, c, d, e, f) \
    /* 0: stop final, 1: stop per line, 2: stop per tile*/\
    CMD(a, b, c, d, e, f, sw_wpe.tpipe_irq_mode, TILE_IRQ_MODE,, U)\
    /* IMGI input width & height */\
    CMD(a, b, c, d, e, f, sw_wpe.tpipe_width_wpe, TILE_WIDTH_WPE_A,, U)\
    CMD(a, b, c, d, e, f, sw_wpe.tpipe_height_wpe, TILE_HEIGHT_WPE_A,, U)\
	/* log_en: 0, 1 */\
    CMD(a, b, c, d, e, f, sw_wpe.tpipe_log_en, WPE_TILE_LOG_EN,, U)\

/* working name */
/* tpipe name */
/* HW register name, field name */
/* U: unsigned, S: signed */
#define WPE_TPIPE_HW_REG_LUT(CMD, a, b, c, d, e, f) \
    /* Common */\
	CMD(a, b, c, d, e, f, top_wpe.cachi_en, WPE_A_CTL_DMA_EN, CACHI_EN, U)\
	CMD(a, b, c, d, e, f, top_wpe.veci_en, WPE_A_CTL_DMA_EN, VECI_EN, U)\
	CMD(a, b, c, d, e, f, top_wpe.vec2i_en, WPE_A_CTL_DMA_EN, VEC2I_EN, U)\
	CMD(a, b, c, d, e, f, top_wpe.vec3i_en, WPE_A_CTL_DMA_EN, VEC3I_EN, U)\
	CMD(a, b, c, d, e, f, top_wpe.wpeo_en, WPE_A_CTL_DMA_EN, WPEO_EN, U)\
	CMD(a, b, c, d, e, f, top_wpe.msko_en, WPE_A_CTL_DMA_EN, MSKO_EN, U)\
	CMD(a, b, c, d, e, f, top_wpe.wpe_ispcrop_en, WPE_A_CTL_MOD_EN, ISP_CROP_EN, U)\
	CMD(a, b, c, d, e, f, top_wpe.wpe_mdpcrop_en, WPE_A_CTL_MOD_EN, MDP_CROP_EN, U)\
	CMD(a, b, c, d, e, f, top_wpe.wpe_c24_en, WPE_A_CTL_MOD_EN, C24_EN, U)\
	CMD(a, b, c, d, e, f, top_wpe.vgen_en, WPE_A_CTL_MOD_EN, VGEN_EN, U)\
	CMD(a, b, c, d, e, f, top_wpe.sync_en, WPE_A_CTL_MOD_EN, SYNC_EN, U)\
	/* CAHCI */\
	CMD(a, b, c, d, e, f, top_wpe.wpe_cachi_fmt, WPE_A_CTL_FMT_SEL, CACHI_FMT, U)\
	CMD(a, b, c, d, e, f, top_wpe.wpe_cachi_plane, WPE_A_CTL_FMT_SEL, CACHI_PLANE, U)\
	CMD(a, b, c, d, e, f, top_wpe.wpe_cachi_fmt_bit, WPE_A_CTL_FMT_SEL, CACHI_FMT_BIT, U)\
    /* VECI */\
    CMD(a, b, c, d, e, f, veci.veci_stride, WPE_A_VECI_STRIDE, STRIDE, U)\
    /* VEC2I */\
    CMD(a, b, c, d, e, f, vec2i.vec2i_stride, WPE_A_VEC2I_STRIDE, STRIDE, U)\
    /* VEC3I */\
    CMD(a, b, c, d, e, f, vec3i.vec3i_stride, WPE_A_VEC3I_STRIDE, STRIDE, U)\
    /* VGEN */\
    CMD(a, b, c, d, e, f, wpe.vgen_input_width, WPE_A_VGEN_IN_IMG, VGEN_IN_WD, U)\
    CMD(a, b, c, d, e, f, wpe.vgen_input_height,WPE_A_VGEN_IN_IMG, VGEN_IN_HT, U)\
    CMD(a, b, c, d, e, f, wpe.vgen_output_width, WPE_A_VGEN_OUT_IMG, VGEN_OUT_WD, U)\
    CMD(a, b, c, d, e, f, wpe.vgen_output_height, WPE_A_VGEN_OUT_IMG, VGEN_OUT_HT, U)\
    CMD(a, b, c, d, e, f, wpe.vgen_luma_horizontal_integer_offset, WPE_A_VGEN_HORI_INT_OFST, VGEN_HORI_INT_OFST, U)\
    CMD(a, b, c, d, e, f, wpe.vgen_luma_horizontal_subpixel_offset, WPE_A_VGEN_HORI_SUB_OFST, VGEN_HORI_SUB_OFST, U)\
    CMD(a, b, c, d, e, f, wpe.vgen_luma_vertical_integer_offset, WPE_A_VGEN_VERT_INT_OFST, VGEN_VERT_INT_OFST, U)\
    CMD(a, b, c, d, e, f, wpe.vgen_luma_vertical_subpixel_offset, WPE_A_VGEN_VERT_SUB_OFST, VGEN_VERT_SUB_OFST, U)\
    CMD(a, b, c, d, e, f, wpe.vgen_horizontal_coeff_step, WPE_A_VGEN_HORI_STEP, VGEN_HORI_STEP, U)\
    CMD(a, b, c, d, e, f, wpe.vgen_vertical_coeff_step, WPE_A_VGEN_VERT_STEP, VGEN_VERT_STEP, U)\
    /* WPEO */\
    CMD(a, b, c, d, e, f, wpeo.wpeo_stride, WPE_A_WPEO_STRIDE, STRIDE, U)\
    CMD(a, b, c, d, e, f, wpeo.wpeo_xoffset, WPE_A_WPEO_CROP, XOFFSET, U)\
    CMD(a, b, c, d, e, f, wpeo.wpeo_yoffset, WPE_A_WPEO_CROP, YOFFSET, U)\
    CMD(a, b, c, d, e, f, wpeo.wpeo_xsize, WPE_A_WPEO_XSIZE, XSIZE, U)\
    CMD(a, b, c, d, e, f, wpeo.wpeo_ysize, WPE_A_WPEO_YSIZE, YSIZE, U)\
    /* MSKO */\
    CMD(a, b, c, d, e, f, msko.msko_stride, WPE_A_MSKO_STRIDE, STRIDE, U)\
    CMD(a, b, c, d, e, f, msko.msko_xoffset, WPE_A_MSKO_CROP, XOFFSET, U)\
    CMD(a, b, c, d, e, f, msko.msko_yoffset, WPE_A_MSKO_CROP, YOFFSET, U)\
    CMD(a, b, c, d, e, f, msko.msko_xsize, WPE_A_MSKO_XSIZE, XSIZE, U)\
    CMD(a, b, c, d, e, f, msko.msko_ysize, WPE_A_MSKO_YSIZE, YSIZE, U)\

/* a, b, d, e, f reserved */
/* max_count: upper bound of dump hex, result: isp result */
/* c: ptr of tile_reg_map, reserved */
/* id of dump word, first line should be 0, other id can be different between neighboring dump word */
/* valid logic condition to dump */
/* mask of register field */
/* tdr name to update */
/* U: unsigned, S: signed */
#define WPE_TPIPE_INTERENAL_LUT(CMD, a, b, c, d, e, f, max_count, result, word_no)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 0, true, VGEN_IN_XSTART, 0x0000FFFF, 0, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 0, true, VGEN_IN_XEND, 0xFFFF0000, 16, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 1, true, VGEN_OUT_XSTART, 0x0000FFFF, 0, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 1, true, VGEN_OUT_XEND, 0xFFFF0000, 16, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 2, true, VGEN_IN_YSTART, 0x0000FFFF, 0, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 2, true, VGEN_IN_YEND, 0xFFFF0000, 16, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 3, true, VGEN_OUT_YSTART, 0x0000FFFF, 0, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 3, true, VGEN_OUT_YEND, 0xFFFF0000, 16, U)\

#define TDR_REG_CHECK_EN(ptr, reg) (1 == (ptr)->reg)
/* a, b, d, e, f reserved */
/* max_count: upper bound of dump hex, result: isp result */
/* c: ptr of tile_reg_map, reserved */
/* id of dump word, first line should be 0, other id can be different between neighboring dump word */
/* valid logic condition to dump */
/* mask of register field */
/* tdr name to update */
/* U: unsigned, S: signed */
#define WPE_TPIPE_TDR_DUMP_LUT(CMD, a, b, c, d, e, f, max_count, result, word_no)\
	/* CTRL */\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 0, true, LAST_IRQ, 0x00000020, 5, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 0, true, WPE_CTRL_TILE_LOAD_SIZE, 0x00FF0000, 16, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 1, true, CACHI_EN, 0x00000001, 0, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 1, true, VECI_EN, 0x00000002, 1, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 1, true, VEC2I_EN, 0x00000004, 2, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 1, true, VEC3I_EN, 0x00000008, 3, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 1, true, WPEO_EN, 0x00000010, 4, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 1, true, MSKO_EN, 0x00000020, 5, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 1, true, VGEN_EN, 0x00000040, 6, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 1, true, SYNC_EN, 0x00000080, 7, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 1, true, WPE_MDP_CROP_EN, 0x00000100, 8, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 1, true, WPE_ISP_CROP_EN, 0x00000200, 9, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 1, true, WPE_C24_EN, 0x00000400, 10, U)\
	/* VECI */\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 2, TDR_REG_CHECK_EN(a, top_wpe.veci_en), VECI_OFFSET_ADDR, 0x0FFFFFFF, 0, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 3, TDR_REG_CHECK_EN(a, top_wpe.veci_en), VECI_XSIZE, 0x0000FFFF, 0, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 3, TDR_REG_CHECK_EN(a, top_wpe.veci_en), VECI_YSIZE, 0xFFFF0000, 16, U)\
	/* VEC2I */\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 4, TDR_REG_CHECK_EN(a, top_wpe.vec2i_en), VEC2I_OFFSET_ADDR, 0x0FFFFFFF, 0, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 5, TDR_REG_CHECK_EN(a, top_wpe.vec2i_en), VEC2I_XSIZE, 0x0000FFFF, 0, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 5, TDR_REG_CHECK_EN(a, top_wpe.vec2i_en), VEC2I_YSIZE, 0xFFFF0000, 16, U)\
	/* VEC3I */\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 6, TDR_REG_CHECK_EN(a, top_wpe.vec3i_en), VEC3I_OFFSET_ADDR, 0x0FFFFFFF, 0, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 7, TDR_REG_CHECK_EN(a, top_wpe.vec3i_en), VEC3I_XSIZE, 0x0000FFFF, 0, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 7, TDR_REG_CHECK_EN(a, top_wpe.vec3i_en), VEC3I_YSIZE, 0xFFFF0000, 16, U)\
    /* WPEO */\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 8, TDR_REG_CHECK_EN(a, top_wpe.wpeo_en), WPEO_OFFSET_ADDR, 0x0FFFFFFF, 0, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 9, TDR_REG_CHECK_EN(a, top_wpe.wpeo_en), WPEO_XSIZE, 0x0000FFFF, 0, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 9, TDR_REG_CHECK_EN(a, top_wpe.wpeo_en), WPEO_YSIZE, 0xFFFF0000, 16, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 10, TDR_REG_CHECK_EN(a, top_wpe.wpeo_en), WPEO_XOFFSET, 0x0000FFFF, 0, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 10, TDR_REG_CHECK_EN(a, top_wpe.wpeo_en), WPEO_YOFFSET, 0xFFFF0000, 16, U)\
    /* MSKO */\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 11, TDR_REG_CHECK_EN(a, top_wpe.msko_en), MSKO_OFFSET_ADDR, 0x0FFFFFFF, 0, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 12, TDR_REG_CHECK_EN(a, top_wpe.msko_en), MSKO_XSIZE, 0x0000FFFF, 0, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 12, TDR_REG_CHECK_EN(a, top_wpe.msko_en), MSKO_YSIZE, 0xFFFF0000, 16, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 13, TDR_REG_CHECK_EN(a, top_wpe.msko_en), MSKO_XOFFSET, 0x0000FFFF, 0, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 13, TDR_REG_CHECK_EN(a, top_wpe.msko_en), MSKO_YOFFSET, 0xFFFF0000, 16, U)\
    /* VGEN */\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 14, TDR_REG_CHECK_EN(a, top_wpe.vgen_en), VGEN_Input_Image_W, 0x0000FFFF, 0, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 14, TDR_REG_CHECK_EN(a, top_wpe.vgen_en), VGEN_Input_Image_H, 0xFFFF0000, 16, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 15, TDR_REG_CHECK_EN(a, top_wpe.vgen_en), VGEN_Output_Image_W, 0x0000FFFF, 0, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 15, TDR_REG_CHECK_EN(a, top_wpe.vgen_en), VGEN_Output_Image_H, 0xFFFF0000, 16, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 16, TDR_REG_CHECK_EN(a, top_wpe.vgen_en), VGEN_Luma_Horizontal_Integer_Offset, 0x0000FFFF, 0, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 16, TDR_REG_CHECK_EN(a, top_wpe.vgen_en), VGEN_Luma_Vertical_Integer_Offset, 0xFFFF0000, 16, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 17, TDR_REG_CHECK_EN(a, top_wpe.vgen_en), VGEN_Luma_Horizontal_Subpixel_Offset, 0x00FFFFFF, 0, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 18, TDR_REG_CHECK_EN(a, top_wpe.vgen_en), VGEN_Luma_Vertical_Subpixel_Offset, 0x00FFFFFF, 0, U)\
	/* WPE_ISP_CROP */\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 19, TDR_REG_CHECK_EN(a, top_wpe.wpe_ispcrop_en), WPE_ISP_XSTART, 0x0000FFFF, 0, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 19, TDR_REG_CHECK_EN(a, top_wpe.wpe_ispcrop_en), WPE_ISP_XEND, 0xFFFF0000, 16, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 20, TDR_REG_CHECK_EN(a, top_wpe.wpe_ispcrop_en), WPE_ISP_YSTART, 0x0000FFFF, 0, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 20, TDR_REG_CHECK_EN(a, top_wpe.wpe_ispcrop_en), WPE_ISP_YEND, 0xFFFF0000, 16, U)\
	/* WPE_MDP_CROP */\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 21, TDR_REG_CHECK_EN(a, top_wpe.wpe_mdpcrop_en), WPE_MDP_XSTART, 0x0000FFFF, 0, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 21, TDR_REG_CHECK_EN(a, top_wpe.wpe_mdpcrop_en), WPE_MDP_XEND, 0xFFFF0000, 16, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 22, TDR_REG_CHECK_EN(a, top_wpe.wpe_mdpcrop_en), WPE_MDP_YSTART, 0x0000FFFF, 0, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 22, TDR_REG_CHECK_EN(a, top_wpe.wpe_mdpcrop_en), WPE_MDP_YEND, 0xFFFF0000, 16, U)\
    /* WPE_C42 */\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 23, TDR_REG_CHECK_EN(a, top_wpe.wpe_c24_en), WPE_C24_TILE_EDGE, 0x0000000F, 0, U)\

#define WPE_TPIPE_END_CONF_LUT(CMD, a, b, c, d, e, f, max_count, result, word_no)\
	/* WPE_END_POS */\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 0, true, WPE_END_LAST_IRQ, 0x00000020, 5, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 0, true, WPE_DL_LOAD_SIZE, 0x0000FF00, 8, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 0, true, WPE_DL_LAST_TILE, 0xFFFF0000, 16, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 1, true, WPE_END_XSTART, 0x0000FFFF, 0, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 1, true, WPE_END_XEND, 0xFFFF0000, 16, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 2, true, WPE_END_YSTART, 0x0000FFFF, 0, U)\
    CMD(a, b, c, d, e, f, max_count, result, word_no, 2, true, WPE_END_YEND, 0xFFFF0000, 16, U)\

#define TDR_REG_VAL(ptr, reg) ((ptr)->reg)
#define WPE_DMA_BW_ACC(a, b, c, d, e, f, g, h,...) \
	if (g)\
	{\
		(c) += (h);\
	}\

/* a, b, d, e, f reserved */
/* enable flag */
/* x byte size */
/* y byte size */
/* scaling ratio */
#define WPE_TPIPE_DMA_BW_LUT(CMD, a, b, c, d, e, f) \
    CMD(a, b, c, d, e, f, TDR_REG_CHECK_EN(a, top_wpe.veci_en), (TDR_REG_VAL(b, VECI_XSIZE) + 1)*(TDR_REG_VAL(b, VECI_YSIZE) + 1))\
    CMD(a, b, c, d, e, f, TDR_REG_CHECK_EN(a, top_wpe.vec2i_en), (TDR_REG_VAL(b, VEC2I_XSIZE) + 1)*(TDR_REG_VAL(b, VEC2I_YSIZE) + 1))\
    CMD(a, b, c, d, e, f, TDR_REG_CHECK_EN(a, top_wpe.vec3i_en), (TDR_REG_VAL(b, VEC3I_XSIZE) + 1)*(TDR_REG_VAL(b, VEC3I_YSIZE) + 1))\
    CMD(a, b, c, d, e, f, TDR_REG_CHECK_EN(a, top_wpe.wpeo_en), (TDR_REG_VAL(b, WPEO_XSIZE) + 1)*(TDR_REG_VAL(b, WPEO_YSIZE) + 1))\
    CMD(a, b, c, d, e, f, TDR_REG_CHECK_EN(a, top_wpe.msko_en), (TDR_REG_VAL(b, MSKO_XSIZE) + 1)*(TDR_REG_VAL(b, MSKO_YSIZE) + 1))\
    CMD(a, b, c, d, e, f, TDR_REG_CHECK_EN(a, top_wpe.vgen_en), 2*14*TDR_REG_VAL(b, VGEN_Output_Image_W)*TDR_REG_VAL(b, VGEN_Output_Image_H)/10)\

#define WPE_TPIPE_CAL_DUMP_LUT(CMD, a, b, c, d, e, f, max_count, result, word_no)\
	WPE_TPIPE_END_CONF_LUT(CMD, a, b, c, d, e, f, max_count, result, word_no)\
	WPE_TPIPE_INTERENAL_LUT(CMD, a, b, c, d, e, f, max_count, result, word_no)

typedef struct TDR_REG_STRUCT
{
    WPE_TPIPE_TDR_DUMP_LUT(TDR_REG_TYPE_DECLARE,,,,,,,,,)
    WPE_TPIPE_END_CONF_LUT(TDR_REG_TYPE_DECLARE,,,,,,,,,)
	WPE_TPIPE_INTERENAL_LUT(TDR_REG_TYPE_DECLARE,,,,,,,,,)
}TDR_REG_STRUCT;

typedef struct WPE_WORKING_BUFFER_STRUCT
{
	TDR_REG_STRUCT tdr_reg;
}WPE_WORKING_BUFFER_STRUCT;

typedef struct WPE_TPIPE_CONFIG_SW_STRUCT
{
    unsigned int tpipe_irq_mode;
    unsigned int tpipe_width_wpe;
    unsigned int tpipe_height_wpe;
    unsigned int tpipe_log_en;
}WPE_TPIPE_CONFIG_SW_STRUCT;

typedef struct WPE_TPIPE_CONFIG_TOP_STRUCT
{
	unsigned int wpe_en;
	unsigned int wpe2_en;
	unsigned int veci_en;
	unsigned int vec2i_en;
	unsigned int vec3i_en;
	unsigned int wpeo_en;
	unsigned int msko_en;
	unsigned int wpe_ispcrop_en;
	unsigned int wpe_mdpcrop_en;
	unsigned int wpe_c24_en;
	unsigned int wpe_cachi_fmt;
	unsigned int wpe_cachi_plane;
	unsigned int wpe_cachi_fmt_bit;
	unsigned int cachi_en;
	unsigned int vgen_en;
	unsigned int sync_en;;
}WPE_TPIPE_CONFIG_TOP_STRUCT;

typedef struct WPE_TPIPE_CONFIG_VECI_STRUCT
{
    unsigned int veci_v_flip_en;
	unsigned int veci_stride;
	unsigned int veci_xsize;
	unsigned int veci_ysize;
}WPE_TPIPE_CONFIG_VECI_STRUCT;

typedef struct WPE_TPIPE_CONFIG_VEC2I_STRUCT
{
    unsigned int vec2i_v_flip_en;
	unsigned int vec2i_stride;
	unsigned int vec2i_xsize;
	unsigned int vec2i_ysize;
}WPE_TPIPE_CONFIG_VEC2I_STRUCT;

typedef struct WPE_TPIPE_CONFIG_VEC3I_STRUCT
{
    unsigned int vec3i_v_flip_en;
	unsigned int vec3i_stride;
	unsigned int vec3i_xsize;
	unsigned int vec3i_ysize;
}WPE_TPIPE_CONFIG_VEC3I_STRUCT;

typedef struct WPE_TPIPE_CONFIG_WPE_STRUCT
{
    unsigned int vgen_input_width;
    unsigned int vgen_input_height;
    unsigned int vgen_output_width;
    unsigned int vgen_output_height;
    unsigned int vgen_luma_horizontal_integer_offset;/* pixel base */
    unsigned int vgen_luma_horizontal_subpixel_offset;/* 24 bits base */
    unsigned int vgen_luma_vertical_integer_offset;/* pixel base */
    unsigned int vgen_luma_vertical_subpixel_offset;/* 24 bits base */
    unsigned int vgen_horizontal_coeff_step;
    unsigned int vgen_vertical_coeff_step;
}WPE_TPIPE_CONFIG_WPE_STRUCT;

typedef struct WPE_TPIPE_CONFIG_WPEO_STRUCT
{
    unsigned int wpeo_stride;
    unsigned int wpeo_xoffset;
    unsigned int wpeo_yoffset;
    unsigned int wpeo_xsize;
    unsigned int wpeo_ysize;
}WPE_TPIPE_CONFIG_WPEO_STRUCT;

typedef struct WPE_TPIPE_CONFIG_MSKO_STRUCT
{
    unsigned int msko_stride;
    unsigned int msko_xoffset;
    unsigned int msko_yoffset;
    unsigned int msko_xsize;
    unsigned  int msko_ysize;
}WPE_TPIPE_CONFIG_MSKO_STRUCT;

typedef struct WPE_TPIPE_CONFIG_STRUCT
{
    WPE_TPIPE_CONFIG_TOP_STRUCT top_wpe;
    WPE_TPIPE_CONFIG_SW_STRUCT sw_wpe;
    WPE_TPIPE_CONFIG_VECI_STRUCT veci;
    WPE_TPIPE_CONFIG_VEC2I_STRUCT vec2i;
    WPE_TPIPE_CONFIG_VEC3I_STRUCT vec3i;
    WPE_TPIPE_CONFIG_WPE_STRUCT wpe;
    WPE_TPIPE_CONFIG_WPEO_STRUCT wpeo;
    WPE_TPIPE_CONFIG_MSKO_STRUCT msko;
}WPE_TPIPE_CONFIG_STRUCT;

extern int query_wpe_working_buffer_size(void);
extern int query_wpe_tdr_out_buffer_size(int tile_num);
extern int wpe_cal_main(const WPE_TPIPE_CONFIG_STRUCT *ptr_tpipe_in, int dl_tile_no, char *ptr_tdr_working_buffer, const int working_buffer_size,
				 const char *ptr_dl_tdr_in, int dl_buffer_size, int dl_in_word_num_per_tile, int *ptr_wpe_tile_no,
				 char *ptr_tdr_out, int out_buffer_size, int *ptr_tdr_word_num_per_tile,
				 char *ptr_cal_out, int cal_buffer_size, int *ptr_cal_word_num_per_tile);
extern int wpe_cal_main(const WPE_TPIPE_CONFIG_STRUCT *ptr_tpipe_in, int dl_tile_no, char *ptr_tdr_working_buffer, const int working_buffer_size,
				 const char *ptr_dl_tdr_in, int dl_buffer_size, int dl_in_word_num_per_tile, int *ptr_wpe_tile_no,
				 char *ptr_tdr_out, int out_buffer_size, int *ptr_tdr_word_num_per_tile,
				 char *ptr_cal_out, int cal_buffer_size, int *ptr_cal_word_num_per_tile, unsigned int *ptr_wpe_dma_bw);
#endif
