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
#include <mtk_plat_common.h>
#include <platform_def.h>
#include <tbase_private.h>


/* Enable PM hooks */
#define TBASE_PM_ENABLE            1
/* Enable SiP fastcall routing to tbase */
#define TBASE_SIP_ROUTE_ENABLE     0
/* Enable OEM fastcall routing to tbase */
#define TBASE_OEM_ROUTE_ENABLE     0

#define LEVEL3 3

#define TEE_PARAMS (gteearg.tee_boot_arg_addr)

#define TBASE_NWD_DRAM_BASE (((struct bootCfg_t *)(uintptr_t)TEE_PARAMS)->dRamBase)
#define TBASE_NWD_DRAM_SIZE (((struct bootCfg_t *)(uintptr_t)TEE_PARAMS)->dRamSize)
#define TBASE_SWD_DRAM_BASE (((struct bootCfg_t *)(uintptr_t)TEE_PARAMS)->secDRamBase+TBASE_SWD_IMEM_SIZE)
#define TBASE_SWD_DRAM_SIZE (((struct bootCfg_t *)(uintptr_t)TEE_PARAMS)->secDRamSize-TBASE_SWD_IMEM_SIZE)
#define TBASE_SWD_IMEM_BASE (((struct bootCfg_t *)(uintptr_t)TEE_PARAMS)->secDRamBase)
#define TBASE_SWD_IMEM_SIZE (2*1024*1024)
#define TBASE_GIC_DIST_BASE (BASE_GICD_BASE)
#define TBASE_GIC_RDIST_BASE  (BASE_GICR_BASE) /* GICR address used in TBASE for SGI/PPI ex.CoreSwap */

#define TBASE_GIC_VERSION   (3)
#define TBASE_SPI_COUNT     (((struct bootCfg_t *)(uintptr_t)TEE_PARAMS)->total_number_spi)
#define TBASE_SSIQ_NRO      (((struct bootCfg_t *)(uintptr_t)TEE_PARAMS)->ssiq_number)

#define TBASE_MONITOR_FLAGS (TBASE_MONITOR_FLAGS_DEFAULT)

/* Registers available in SWd entry */
#define TBASE_MAX_MONITOR_CALL_REGS  TBASE_MONITOR_CALL_REGS_MIN
#define TBASE_NWD_REGISTER_COUNT     TBASE_NWD_REGISTERS_MIN


/* TODO: check that flags match to actual used */
#define TBASE_REGISTER_FILE_MMU_FLAGS (LOWER_ATTRS(ATTR_IWBWA_OWBWA_NTR_INDEX | ISH | AP_RW)  |\
				       UPPER_ATTRS(XN) | \
				       LEVEL3)


/**********************************************************
 * Macros for platform specific hooking to SPD
 **********************************************************/

/* Character output function */
#ifndef __ASSEMBLY__
extern void mt_log_secure_os_print(int c);
#endif
#define TBASE_OUTPUT_PUTC(c) mt_log_secure_os_print(c)

/* Execution status change; default is no function */
#define TBASE_EXECUTION_STATUS(status)

#endif /* __TBASE_PLATFORM_H__ */
