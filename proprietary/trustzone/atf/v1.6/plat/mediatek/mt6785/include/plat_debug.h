/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __PLATFORM_DEBUG_H__
#define __PLATFORM_DEBUG_H__

#define CPC_FLOW_CTRL_CFG	(0x0C53A814)
#define CPC_POWER_STATUS	(0x0C53A840)
#define MCUCFG_INITARCH		(0x0C53C8E4)
#define SPMC_PWR_RST_CORE1	(0x1000620C)

#define MASK_CPC_CORE0_ONLY	(0xC001)

#define CORE_PWRDN_EN_MASK	(0x1)

#define CA75_CPU6_DBG_CONTROL	(0x0C533308)
#define CA75_CPU7_DBG_CONTROL	(0x0C533B08)
#define CA75_CPU6_CIRCULAR_BUF	(0x0C533400)
#define CA75_CPU7_CIRCULAR_BUF	(0x0C533C00)

#define BIT_CA75_LASTPC_DIS	(1 << 0)

#define DFD_INTERNAL_CTL	(0x0C53A040)

#define CA75_CPU6_SRAM_DELSEL	(0x0C533240)
#define CA75_CPU7_SRAM_DELSEL	(0x0C533A40)

#define CPU_SRAM_DELSEL_MASK	(0xffffff)

#define PLAT_MTK_CIRCULAR_BUFFER_UNLOCK	(0xefab4133)
#define PLAT_MTK_CIRCULAR_BUFFER_LOCK	(0xefab4134)
#define PLAT_LASTPC_SMC_READ_ENTRY	(0xefab4140)

extern void clear_all_on_mux(void);
extern void circular_buffer_setup(void);
extern unsigned int plat_debug_smc_dispatcher(unsigned long arg0, unsigned long arg1, unsigned long arg2);
extern void sram_delsel_setup(void);
extern int dfd_smc_dispatcher(unsigned long arg0, unsigned long arg1, unsigned long arg2, unsigned long arg3);
#endif
