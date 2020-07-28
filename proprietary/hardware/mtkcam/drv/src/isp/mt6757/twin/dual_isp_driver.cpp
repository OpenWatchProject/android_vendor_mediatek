#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dual_cal.h"
#include "twin_drv_reg.h"

#include "isp_reg.h"
#include "dual_isp_config.h"

/* define to replace macro */
#define print_error_message(...) dual_isp_print_error_message(__VA_ARGS__)
#define get_current_file_name(...) dual_isp_get_current_file_name(__VA_ARGS__)
/* func prototype */
#ifndef DUAL_PLATFORM_DRIVER
static const char *dual_isp_print_error_message(DUAL_MESSAGE_ENUM n);
static const char *dual_isp_get_current_file_name(const char *filename);
#endif
/* extern */
extern int dual_cal_main(const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
int dual_convert_input(int dual_sel, const ISP_REG_PTR_STRUCT *ptr_isp_reg_param, DUAL_IN_CONFIG_STRUCT *ptr_dual_in_config,
						DUAL_OUT_CONFIG_STRUCT *ptr_dual_out_config);
int dual_convert_output(int dual_sel, ISP_REG_PTR_STRUCT *ptr_isp_reg_param, const DUAL_IN_CONFIG_STRUCT *ptr_dual_in_config,
						const DUAL_OUT_CONFIG_STRUCT *ptr_dual_out_config);
int dual_fprintf_in_config(int count, const DUAL_IN_CONFIG_STRUCT *ptr_dual_in_config);
int dual_printf_platform_config(const DUAL_IN_CONFIG_STRUCT *ptr_dual_in_config);
int dual_printf_platform_reg(const ISP_REG_PTR_STRUCT *ptr_isp_reg_param);
int dual_fprintf_reg_dump(int dual_sel, const ISP_REG_PTR_STRUCT *ptr_isp_reg_param);

/* return 0: pass, 1: error */
int dual_cal_platform(ISP_REG_PTR_STRUCT *ptr_isp_reg_param, DUAL_IN_CONFIG_STRUCT *ptr_dual_in_config,
					  DUAL_OUT_CONFIG_STRUCT *ptr_dual_out_config)
{
	int result = 0;
	/* update isp_reg_ptr_param to dual_isp_in_config & dual_isp_out_config */
	result = dual_convert_input(ptr_dual_in_config->SW.DUAL_SEL, ptr_isp_reg_param, ptr_dual_in_config, ptr_dual_out_config);
	if (0 == result)
	{
		/* run dual cal */
		result = dual_cal_main(ptr_dual_in_config, ptr_dual_out_config);
	}
	if (0 == result)
	{
		result = dual_convert_output(ptr_dual_in_config->SW.DUAL_SEL, ptr_isp_reg_param, ptr_dual_in_config, ptr_dual_out_config);
	}
	if ((0 != result) || ptr_dual_in_config->DEBUG.DUAL_LOG_EN)
	{
		int temp_result = dual_printf_platform_config(ptr_dual_in_config);
		if (temp_result)
		{
			result = temp_result;
		}
	}
	return result;
}

int dual_printf_platform_config(const DUAL_IN_CONFIG_STRUCT *ptr_dual_in_config)
{
	int result = dual_fprintf_in_config(0, ptr_dual_in_config);
	return result;
}

int dual_printf_platform_reg(const ISP_REG_PTR_STRUCT *ptr_isp_reg_param)
{
	int result = dual_fprintf_reg_dump(0xF, ptr_isp_reg_param);
	return result;
}

int dual_convert_input(int dual_sel, const ISP_REG_PTR_STRUCT *ptr_isp_reg_param, DUAL_IN_CONFIG_STRUCT *ptr_dual_in_config,
						DUAL_OUT_CONFIG_STRUCT *ptr_dual_out_config)
{
	if (ptr_isp_reg_param && ptr_dual_in_config && ptr_dual_out_config && (0x3 == dual_sel))
	{
		/* update dual_in_config & dual_out_config */
		if (ptr_isp_reg_param->ptr_isp_uni)
		{
			DUAL_CTRL_LIST_HW(DUAL_CONVERT_UNI_TO_DUAL, ptr_isp_reg_param, ptr_dual_in_config, ptr_dual_out_config,,,);
		}
		if (ptr_isp_reg_param->ptr_isp_reg && ptr_isp_reg_param->ptr_isp_reg_d)
		{
			/* A to A */
			DUAL_ENGINE_LIST_HW_R_RW(DUAL_COPY_DUAL_A_A, ptr_dual_in_config, ptr_isp_reg_param->ptr_isp_reg,,,,);
			DUAL_ENGINE_LIST_HW_R_R(DUAL_COPY_DUAL_A_A, ptr_dual_in_config, ptr_isp_reg_param->ptr_isp_reg,,,,);
			DUAL_ENGINE_LIST_HW_R_X(DUAL_COPY_DUAL_A_A, ptr_dual_in_config, ptr_isp_reg_param->ptr_isp_reg,,,,);
			DUAL_ENGINE_LIST_HW_RW_W(DUAL_COPY_DUAL_A_A, ptr_dual_in_config, ptr_isp_reg_param->ptr_isp_reg,,,,);
			DUAL_ENGINE_LIST_HW_RW_W(DUAL_COPY_DUAL_A_A, ptr_dual_out_config, ptr_isp_reg_param->ptr_isp_reg,,,,);
			DUAL_ENGINE_LIST_HW_R_W(DUAL_COPY_DUAL_A_A, ptr_dual_in_config, ptr_isp_reg_param->ptr_isp_reg,,,,);
			DUAL_ENGINE_LIST_HW_RW_RW(DUAL_COPY_DUAL_A_A, ptr_dual_in_config, ptr_isp_reg_param->ptr_isp_reg,,,,);
			DUAL_ENGINE_LIST_HW_RW_RW(DUAL_COPY_DUAL_A_A, ptr_dual_out_config, ptr_isp_reg_param->ptr_isp_reg,,,,);
			DUAL_ENGINE_ARRAY_HW(DUAL_COPY_ARRAY_A_A, ptr_dual_in_config, ptr_isp_reg_param->ptr_isp_reg,,,,);
			/* B to B and A to B */
			DUAL_ENGINE_LIST_HW_R_RW(DUAL_COPY_DUAL_B_B, ptr_dual_in_config, ptr_isp_reg_param->ptr_isp_reg_d,,,,);
			DUAL_ENGINE_LIST_HW_R_RW(DUAL_COPY_DUAL_B_B, ptr_dual_out_config, ptr_isp_reg_param->ptr_isp_reg_d,,,,);
			DUAL_ENGINE_LIST_HW_R_R(DUAL_COPY_DUAL_B_B, ptr_dual_in_config, ptr_isp_reg_param->ptr_isp_reg_d,,,,);
			DUAL_ENGINE_LIST_HW_RW_W(DUAL_COPY_DUAL_A_B, ptr_dual_out_config, ptr_isp_reg_param->ptr_isp_reg,,,,);
			DUAL_ENGINE_LIST_HW_R_W(DUAL_COPY_DUAL_A_B, ptr_dual_out_config, ptr_isp_reg_param->ptr_isp_reg,,,,);
			DUAL_ENGINE_LIST_HW_RW_RW(DUAL_COPY_DUAL_B_B, ptr_dual_in_config, ptr_isp_reg_param->ptr_isp_reg_d,,,,);
			DUAL_ENGINE_LIST_HW_RW_RW(DUAL_COPY_DUAL_B_B, ptr_dual_out_config, ptr_isp_reg_param->ptr_isp_reg_d,,,,);
			DUAL_ENGINE_ARRAY_HW(DUAL_COPY_ARRAY_A_B, ptr_dual_out_config, ptr_isp_reg_param->ptr_isp_reg,,,,);
		}
	}
	return 0;
}

int dual_convert_output(int dual_sel, ISP_REG_PTR_STRUCT *ptr_isp_reg_param, const DUAL_IN_CONFIG_STRUCT *ptr_dual_in_config,
						const DUAL_OUT_CONFIG_STRUCT *ptr_dual_out_config)
{
	/* update dual_in_config & dual_out_config */
	if (ptr_isp_reg_param && ptr_dual_in_config && ptr_dual_out_config)
	{
		/* update dual_in_config & dual_out_config */
		if (ptr_isp_reg_param->ptr_isp_reg && ptr_isp_reg_param->ptr_isp_reg_d && (0x3 == dual_sel))
		{
			DUAL_ENGINE_LIST_HW_OUT(DUAL_COPY_DUAL_A_A, ptr_isp_reg_param->ptr_isp_reg, ptr_dual_out_config,,,,);
			DUAL_ENGINE_LIST_HW_OUT_D(DUAL_COPY_DUAL_B_B, ptr_isp_reg_param->ptr_isp_reg_d, ptr_dual_out_config,,,,);
			DUAL_ENGINE_ARRAY_HW(DUAL_COPY_ARRAY_B_B, ptr_isp_reg_param->ptr_isp_reg_d, ptr_dual_out_config,,,,);
		}
	}
	return 0;
}

int dual_fprintf_out_config(int count, int dual_sel, const DUAL_OUT_CONFIG_STRUCT *ptr_dual_out_config)
{
	if (ptr_dual_out_config)
	{
		int dual_mode = DUAL_MODE_CAL(dual_sel);
		if (dual_mode > 1)
		{
#ifdef DUAL_PLATFORM_DRIVER
			uart_printf("%s %s %s\n", DUAL_IN_DUMP_HEADER, DUAL_DUMP_START, DUAL_DUMP_VERSION);
			DUAL_ENGINE_LIST_HW_OUT(DUAL_UART_PRINT_TXT_ENGINE_OUT, uart_printf, ptr_dual_out_config,,,,);
			DUAL_ENGINE_LIST_HW_OUT_D(DUAL_UART_PRINT_TXT_ENGINE_OUT_D, uart_printf, ptr_dual_out_config,,,,);
			DUAL_ENGINE_ARRAY_HW(DUAL_UART_PRINT_TXT_ARRAY_OUT, uart_printf, ptr_dual_out_config,,,,);
			DUAL_ENGINE_ARRAY_HW(DUAL_UART_PRINT_TXT_ARRAY_OUT_D, uart_printf, ptr_dual_out_config,,,,);
			uart_printf("%s %s\n", DUAL_IN_DUMP_HEADER, DUAL_DUMP_END);
#else
			FILE *fpt_log=NULL;
			char full_name[DUAL_MAX_COMMAND_LENGTH];
			dual_sprintf(full_name, sizeof(full_name), "%s%s%s%d%s", DUAL_DEFAULT_PATH, DUAL_FOLDER_SYMBOL_STR,
				DUAL_OUT_CONFIG_FILENAME, count, DUAL_OUT_CONFIG_EXTNAME);
			dual_fopen(fpt_log, full_name, "wb");
			if (NULL == fpt_log)
			{
				dual_sprintf(full_name, sizeof(full_name), "%s%d%s",
					DUAL_OUT_CONFIG_FILENAME, count, DUAL_OUT_CONFIG_EXTNAME);
				dual_fopen(fpt_log, full_name, "wb");
				if (NULL == fpt_log)
				{
					dual_driver_printf("Error: %s\n", print_error_message(DUAL_MESSAGE_FILE_OPEN_ERROR));
					return 1;
				}
			}
			printf("Output file: %s\n", full_name);
			fprintf(fpt_log, "%s %s %s\n", DUAL_OUT_DUMP_HEADER, DUAL_DUMP_START, DUAL_DUMP_VERSION);
			DUAL_ENGINE_LIST_HW_OUT(DUAL_LOG_PRINT_TXT_ENGINE_OUT, fpt_log, fprintf, ptr_dual_out_config,,,);
			DUAL_ENGINE_LIST_HW_OUT_D(DUAL_LOG_PRINT_TXT_ENGINE_OUT_D, fpt_log, fprintf, ptr_dual_out_config,,,);
			DUAL_ENGINE_ARRAY_HW(DUAL_LOG_PRINT_TXT_ARRAY_OUT, fpt_log, fprintf, ptr_dual_out_config,,,);
			DUAL_ENGINE_ARRAY_HW(DUAL_LOG_PRINT_TXT_ARRAY_OUT_D, fpt_log, fprintf, ptr_dual_out_config,,,);
			/* output can check flag before printf */
			fprintf(fpt_log, "%s %s\n", DUAL_OUT_DUMP_HEADER, DUAL_DUMP_END);
			fclose(fpt_log);
#endif
		}
	}
	return 0;
}

int dual_fprintf_in_config(int count, const DUAL_IN_CONFIG_STRUCT *ptr_dual_in_config)
{
	if (ptr_dual_in_config)
	{
		int dual_mode = DUAL_MODE_CAL(ptr_dual_in_config->SW.DUAL_SEL);
		if (dual_mode > 1)
		{
#ifdef DUAL_PLATFORM_DRIVER
			uart_printf("%s %s %s\n", DUAL_IN_DUMP_HEADER, DUAL_DUMP_START, DUAL_DUMP_VERSION);
			DUAL_DEBUG_LIST(DUAL_UART_PRINT_TXT, uart_printf, ptr_dual_in_config,,,,);
			DUAL_SW_LIST(DUAL_UART_PRINT_SW_TXT, uart_printf, ptr_dual_in_config,,,,);
			DUAL_CTRL_LIST_HW(DUAL_UART_PRINT_TXT_HW, uart_printf, ptr_dual_in_config,,,,);
			DUAL_ENGINE_LIST_HW_IN(DUAL_UART_PRINT_TXT_ENGINE, uart_printf, ptr_dual_in_config,,,,);
			DUAL_ENGINE_LIST_HW_IN_D(DUAL_UART_PRINT_TXT_ENGINE_D, uart_printf, ptr_dual_in_config,,,,);
			DUAL_ENGINE_ARRAY_HW(DUAL_UART_PRINT_TXT_ARRAY, uart_printf, ptr_dual_in_config,,,,);
			DUAL_ENGINE_ARRAY_HW(DUAL_UART_PRINT_TXT_ARRAY_D, uart_printf, ptr_dual_in_config,,,,);
			uart_printf("%s %s\n", DUAL_IN_DUMP_HEADER, DUAL_DUMP_END);
#else
			FILE *fpt_log=NULL;
			char full_name[DUAL_MAX_COMMAND_LENGTH];
			dual_sprintf(full_name, sizeof(full_name), "%s%s%s%d%s", DUAL_DEFAULT_PATH, DUAL_FOLDER_SYMBOL_STR,
				DUAL_IN_CONFIG_FILENAME, count, DUAL_IN_CONFIG_EXTNAME);
			dual_fopen(fpt_log, full_name, "wb");
			if (NULL == fpt_log)
			{
				dual_sprintf(full_name, sizeof(full_name), "%s%d%s",
					DUAL_IN_CONFIG_FILENAME, count, DUAL_IN_CONFIG_EXTNAME);
				dual_fopen(fpt_log, full_name, "wb");
				if (NULL == fpt_log)
				{
					dual_driver_printf("Error: %s\n", print_error_message(DUAL_MESSAGE_FILE_OPEN_ERROR));
					return 1;
				}
			}
			printf("Output file: %s\n", full_name);
			fprintf(fpt_log, "%s %s %s\n", DUAL_IN_DUMP_HEADER, DUAL_DUMP_START, DUAL_DUMP_VERSION);
			DUAL_DEBUG_LIST(DUAL_LOG_PRINT_TXT, fpt_log, fprintf, ptr_dual_in_config,,,);
			DUAL_SW_LIST(DUAL_LOG_PRINT_SW_TXT, fpt_log, fprintf, ptr_dual_in_config,,,);
			DUAL_CTRL_LIST_HW(DUAL_LOG_PRINT_TXT_HW, fpt_log, fprintf, ptr_dual_in_config,,,);
			DUAL_ENGINE_LIST_HW_IN(DUAL_LOG_PRINT_TXT_ENGINE, fpt_log, fprintf, ptr_dual_in_config,,,);
			DUAL_ENGINE_LIST_HW_IN_D(DUAL_LOG_PRINT_TXT_ENGINE_D, fpt_log, fprintf, ptr_dual_in_config,,,);
			DUAL_ENGINE_ARRAY_HW(DUAL_LOG_PRINT_TXT_ARRAY, fpt_log, fprintf, ptr_dual_in_config,,,);
			DUAL_ENGINE_ARRAY_HW(DUAL_LOG_PRINT_TXT_ARRAY_D, fpt_log, fprintf, ptr_dual_in_config,,,);
			/* input can check flag before printf */
			fprintf(fpt_log, "%s %s\n", DUAL_IN_DUMP_HEADER, DUAL_DUMP_END);
			fclose(fpt_log);
#endif
		}
	}
	return 0;
}

int dual_fprintf_reg_dump(int dual_sel, const ISP_REG_PTR_STRUCT *ptr_isp_reg_param)
{
	if (ptr_isp_reg_param)
	{
		if (ptr_isp_reg_param->ptr_isp_reg && ptr_isp_reg_param->ptr_isp_reg_d && (0x3 == dual_sel))
		{
#ifdef DUAL_PLATFORM_DRIVER
			uart_printf("%s %s %s\n", DUAL_REG_DUMP_HEADER, DUAL_DUMP_START, DUAL_DUMP_VERSION);
			DUAL_CTRL_LIST_HW(DUAL_UART_REG_TXT_HW, uart_printf, ptr_isp_reg_param->ptr_isp_uni, DUAL_ISP_BASE_UNI,,,);
			/* can check flag before dump */
			DUAL_ENGINE_LIST_HW(DUAL_UART_REG_TXT_ENGINE, uart_printf, ptr_isp_reg_param->ptr_isp_reg, DUAL_ISP_BASE_HW,,,);
			DUAL_ENGINE_ARRAY_HW(DUAL_UART_REG_TXT_ARRAY, uart_printf, ptr_isp_reg_param->ptr_isp_reg, DUAL_ISP_BASE_HW,,,);
			DUAL_ENGINE_LIST_HW_D(DUAL_UART_REG_TXT_ENGINE_D, uart_printf, ptr_isp_reg_param->ptr_isp_reg_d, DUAL_ISP_BASE_HW_D,,,);
			DUAL_ENGINE_ARRAY_HW(DUAL_UART_REG_TXT_ARRAY_D, uart_printf, ptr_isp_reg_param->ptr_isp_reg_d, DUAL_ISP_BASE_HW_D,,,);
			uart_printf("%s %s\n", DUAL_REG_DUMP_HEADER, DUAL_DUMP_END);
#else
			FILE *fpt_log=NULL;
			char full_name[DUAL_MAX_COMMAND_LENGTH];
			dual_sprintf(full_name, sizeof(full_name), "%s%s%s", DUAL_DEFAULT_PATH, DUAL_FOLDER_SYMBOL_STR,
				DUAL_REG_DUMP_FILENAME);
			dual_fopen(fpt_log, full_name, "wb");
			if (NULL == fpt_log)
			{
				dual_sprintf(full_name, sizeof(full_name), "%s", DUAL_REG_DUMP_FILENAME);
				dual_fopen(fpt_log, full_name, "wb");
				if (NULL == fpt_log)
				{
					dual_driver_printf("Error: %s\n", print_error_message(DUAL_MESSAGE_FILE_OPEN_ERROR));
					return 1;
				}
			}
			printf("Output file: %s\n", full_name);
			fprintf(fpt_log, "%s %s %s\n", DUAL_REG_DUMP_HEADER, DUAL_DUMP_START, DUAL_DUMP_VERSION);
			DUAL_CTRL_LIST_HW(DUAL_LOG_REG_TXT_HW, fpt_log, fprintf, ptr_isp_reg_param->ptr_isp_uni, DUAL_ISP_BASE_UNI,,);
			/* can check flag before dump */
			DUAL_ENGINE_LIST_HW(DUAL_LOG_REG_TXT_ENGINE, fpt_log, fprintf, ptr_isp_reg_param->ptr_isp_reg, DUAL_ISP_BASE_HW,,);
			DUAL_ENGINE_ARRAY_HW(DUAL_LOG_REG_TXT_ARRAY, fpt_log, fprintf, ptr_isp_reg_param->ptr_isp_reg, DUAL_ISP_BASE_HW,,);
			DUAL_ENGINE_LIST_HW_D(DUAL_LOG_REG_TXT_ENGINE_D, fpt_log, fprintf, ptr_isp_reg_param->ptr_isp_reg_d, DUAL_ISP_BASE_HW_D,,);
			DUAL_ENGINE_ARRAY_HW(DUAL_LOG_REG_TXT_ARRAY_D, fpt_log, fprintf, ptr_isp_reg_param->ptr_isp_reg_d, DUAL_ISP_BASE_HW_D,,);
			/* clone to 3 engines */
			fprintf(fpt_log, "%s %s\n", DUAL_REG_DUMP_HEADER, DUAL_DUMP_END);
			fclose(fpt_log);
#endif
		}
	}
	return 0;
}

#ifndef DUAL_PLATFORM_DRIVER
static const char *dual_isp_print_error_message(DUAL_MESSAGE_ENUM n)
{
    TWIN_GET_ERROR_NAME(n);
}

static const char *dual_isp_get_current_file_name(const char *filename)
{
    char *ptr = strrchr((char *)filename, DUAL_FOLDER_SYMBOL_CHAR);
    if (NULL == ptr)
    {
        return filename;
    }
    else
    {
        return (const char *)(ptr + 1);
    }
}
#endif
