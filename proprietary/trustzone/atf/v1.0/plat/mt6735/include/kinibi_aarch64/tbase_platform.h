/*
 * Copyright (c) 2014 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

#ifndef __TBASE_PLATFORM_H__
#define __TBASE_PLATFORM_H__

#include <bl_common.h>
#include <tbase_private.h>
#include <plat_def.h>
#include <tbase.h>

#define ATF_VERSION_CODE(major, minor) (major * 100 + minor)
/* Enable PM hooks */
#define TBASE_PM_ENABLE            1
/* Wrapper on ATF API for ATF version prior to 0.5 */
#if (ATF_VERSION_CODE(VERSION_MAJOR, VERSION_MINOR) <= ATF_VERSION_CODE(0, 4))
	#define cm_get_context_by_mpidr(mpidr, state)           cm_get_context(mpidr, state)
	#define cm_set_context_by_mpidr(mpidr, context, state)  cm_set_context(mpidr, context, state)
#endif
/* Enable SiP fastcall routing to tbase */
#define TBASE_SIP_ROUTE_ENABLE     0
/* Enable OEM fastcall routing to tbase */
#define TBASE_OEM_ROUTE_ENABLE     0


#define TEE_PARAMS (((atf_arg_t_ptr)(uintptr_t)TEE_BOOT_INFO_ADDR)->tee_boot_arg_addr)

#define TBASE_NWD_DRAM_BASE (((bootCfg_t *)(uintptr_t)TEE_PARAMS)->dRamBase)
#define TBASE_NWD_DRAM_SIZE (((bootCfg_t *)(uintptr_t)TEE_PARAMS)->dRamSize)
#define TBASE_SWD_DRAM_BASE (((bootCfg_t *)(uintptr_t)TEE_PARAMS)->secDRamBase)
#define TBASE_SWD_DRAM_SIZE (((bootCfg_t *)(uintptr_t)TEE_PARAMS)->secDRamSize)
#define TBASE_SWD_IMEM_BASE (((bootCfg_t *)(uintptr_t)TEE_PARAMS)->secDRamBase)
#define TBASE_SWD_IMEM_SIZE (0)
#define TBASE_GIC_DIST_BASE (BASE_GICD_BASE)
#define TBASE_GIC_CPU_BASE  (BASE_GICC_BASE)

#define TBASE_GIC_VERSION   (2)
#define TBASE_SPI_COUNT     (((bootCfg_t *)(uintptr_t)TEE_PARAMS)->total_number_spi)
#define TBASE_SSIQ_NRO      (((bootCfg_t *)(uintptr_t)TEE_PARAMS)->ssiq_number)

#define TBASE_MONITOR_FLAGS (TBASE_MONITOR_FLAGS_DEFAULT)

/* Registers available in SWd entry */
#define TBASE_MAX_MONITOR_CALL_REGS  TBASE_MONITOR_CALL_REGS_MIN
#define TBASE_NWD_REGISTER_COUNT     TBASE_NWD_REGISTERS_MIN


/* TODO: check that flags match to actual used */
#define TBASE_REGISTER_FILE_MMU_FLAGS (LOWER_ATTRS(ATTR_IWBWA_OWBWA_NTR_INDEX | ISH | AP_RW)  |\
				       UPPER_ATTRS(XN) | \
				       LEVEL3)

#define TBASE_GIC_DIST_BASE (BASE_GICD_BASE)
#define TBASE_GIC_CPU_BASE  (BASE_GICC_BASE)
/* ********************************************************** */
/* Macros for platform specific hooking to SPD */

/* Character output function */
#ifndef __ASSEMBLY__
extern void mt_log_secure_os_print(int c);
#endif
#define TBASE_OUTPUT_PUTC(c) mt_log_secure_os_print(c)

/* Execution status change; default is no function */
#define TBASE_EXECUTION_STATUS(status)

#endif /* __TBASE_PLATFORM_H__ */
