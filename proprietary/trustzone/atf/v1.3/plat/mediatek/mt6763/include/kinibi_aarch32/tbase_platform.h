/*
 * Copyright (c) 2016 TRUSTONIC LIMITED
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the TRUSTONIC LIMITED nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
#define TBASE_GIC_RDIST_BASE (BASE_GICR_BASE) /* GICR address used in TBASE for SGI/PPI ex.CoreSwap */

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
