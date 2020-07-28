/*
 * Copyright (c) 2015-2018 MICROTRUST Incorporated
 * All rights reserved
 *
 * This file and software is confidential and proprietary to MICROTRUST Inc.
 * Unauthorized copying of this file and software is strictly prohibited.
 * You MUST NOT disclose this file and software unless you get a license
 * agreement from MICROTRUST Incorporated.
 */
#ifndef __TEEI_ATF_H__
#define __TEEI_ATF_H__

#ifndef ATF_MAJOR_VERSION
	error major;
#endif
#ifndef ATF_MINOR_VERSION
	error minor;
#endif

#include <debug.h>
#include "devapc.h"

#if (MTK_PLATFORM == 6771 || MTK_PLATFORM == 6768)
	#define MT_POLARITY MT_POLARITY_HIGH
#else
	#define MT_POLARITY MT_POLARITY_LOW
#endif

#if (ATF_MAJOR_VERSION == 1 && ATF_MINOR_VERSION >= 6) || (ATF_MAJOR_VERSION > 1)
	#include <mtk_plat_common.h>
	#define CM_GET_CONTEXT(x)	cm_get_context_by_index(platform_get_core_pos(read_mpidr()), x)
	#define UART_INIT	console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE)
	#define UART_UNINIT	console_uninit()
	#define GICD_BASE	BASE_GICD_BASE
	#define GICC_BASE	BASE_GICC_BASE
	#ifdef DEBUG
	#define DBG_PRINTF(...)	printf(__VA_ARGS__)
	#endif
	extern struct atf_arg_t gteearg;

#elif (ATF_MAJOR_VERSION == 1 && ATF_MINOR_VERSION == 3) || (ATF_MAJOR_VERSION == 1 && ATF_MINOR_VERSION == 4)
	#include <mtk_plat_common.h>
	#define CM_GET_CONTEXT(x)	cm_get_context_by_index(platform_get_core_pos(read_mpidr()), x)
	#define UART_INIT	console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE)
	#define UART_UNINIT	console_uninit()
	#define GICD_BASE	BASE_GICD_BASE
	#define GICC_BASE	BASE_GICC_BASE
	#ifdef DEBUG
	#define DBG_PRINTF(...)	tf_printf(__VA_ARGS__)
	#endif
	extern struct atf_arg_t gteearg;

#elif (ATF_MAJOR_VERSION == 1 && ATF_MINOR_VERSION == 2)
	#include <plat_private.h>
	#define CM_GET_CONTEXT(x)	cm_get_context_by_mpidr(read_mpidr(), x)
	#define UART_INIT	console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE)
	#define UART_UNINIT	console_uninit()
	#define GICD_BASE	BASE_GICD_BASE
	#define GICC_BASE	BASE_GICC_BASE
	#ifdef DEBUG
	#define DBG_PRINTF(...)	tf_printf(__VA_ARGS__)
	#endif
	extern atf_arg_t gteearg;

#else
	#include <plat_private.h>
	#include "plat_def.h"
	#include "plat_config.h"
	#define CM_GET_CONTEXT(x)	cm_get_context_by_mpidr(read_mpidr(), x)
	#define UART_INIT	set_uart_flag()
	#define UART_UNINIT	clear_uart_flag()
	#define GICD_BASE	get_plat_config()->gicd_base
	#define GICC_BASE	get_plat_config()->gicc_base
	#ifdef DEBUG
	#define DBG_PRINTF(...)	printf(__VA_ARGS__)
	#endif
	extern atf_arg_t gteearg;

#endif

#ifndef DEBUG
#define DBG_PRINTF(...)
#endif

#endif
