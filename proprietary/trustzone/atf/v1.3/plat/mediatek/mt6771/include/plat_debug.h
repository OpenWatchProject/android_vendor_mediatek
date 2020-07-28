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

#define readl(addr)		mmio_read_32((addr))
#define writel(addr, val)	mmio_write_32((addr), (val))
#define sync_writel(addr, val)	do { mmio_write_32((addr), (val)); dsbsy(); } while (0)

#define MCU_BIU_BASE				(0x0c530000)
#define MISC1_CFG_BASE				(0xB00)
#define CA15M_CFG_BASE				(0x2000)
#define DFD_INTERNAL_CTL			(MCU_BIU_BASE+MISC1_CFG_BASE+0x00)
#define DFD_INTERNAL_PWR_ON			(MCU_BIU_BASE+MISC1_CFG_BASE+0x08)
#define DFD_CHAIN_LENGTH0			(MCU_BIU_BASE+MISC1_CFG_BASE+0x0c)
#define DFD_INTERNAL_SHIFT_CLK_RATIO		(MCU_BIU_BASE+MISC1_CFG_BASE+0x10)
#define DFD_CHAIN_LENGTH1			(MCU_BIU_BASE+MISC1_CFG_BASE+0x1c)
#define DFD_INTERNAL_TEST_SO_0			(MCU_BIU_BASE+MISC1_CFG_BASE+0x28)
#define DFD_INTERNAL_NUM_OF_TEST_SO_GROUP	(MCU_BIU_BASE+MISC1_CFG_BASE+0x30)
#define DFD_INTERNAL_MCSIB			(MCU_BIU_BASE+MISC1_CFG_BASE+0x40)
#define DFD_INTERNAL_MCSIB_SEL_STATUS		(MCU_BIU_BASE+MISC1_CFG_BASE+0x44)
#define CA15M_DBG_CONTROL			(MCU_BIU_BASE+CA15M_CFG_BASE+0x728)
#define CA15M_PWR_RST_CTL			(MCU_BIU_BASE+CA15M_CFG_BASE+0x08)
#define VPROC_EXT_CTL				(0x10006290)

#define CFG_SF_CTRL				(0x0c510014)
#define CFG_SF_INI				(0x0c510010)
#define SNOOP_FILTER_BASE			(0x0C600000)
#define SNOOP_FILTER_LEN			(0x00100000)

#define BIT_CA15M_L2PARITY_EN			(1 << 1)
#define BIT_CA15M_LASTPC_DIS			(1 << 8)

#define MP1_CPUTOP_PWR_CON			(0x10006218)
#define WDT_DEBUG_CTL				(0x10007040)

/* to indicate that the DFD is enabled */
#ifdef CFG_PLAT_SRAM_FLAG
#define PLAT_SRAM_FLAG1				(0x12dbf4)
#define BIT_DFD_VALID				(1 << 0)
#endif

/* workaround for mp2 */
#define CA15M_CACHE_CTL				(MCU_BIU_BASE+CA15M_CFG_BASE+0xc)

#define MCU_ALL_PWR_ON_CTRL			(0x0C530B58)

#define PLAT_MTK_DFD_SETUP_SRAM_MAGIC		(0xefab4130)
#define PLAT_MTK_DFD_RESET_SF_MAGIC		(0xefab4131)
#define PLAT_MTK_DFD_RETURN_SRAM_MAGIC		(0xefab4132)
#define PLAT_MTK_CIRCULAR_BUFFER_UNLOCK		(0xefab4133)
#define PLAT_MTK_CIRCULAR_BUFFER_LOCK		(0xefab4134)

#define DBGTOP_LAR				(0x0d0a0fb0)
#define DBGTOP_DBGRST_ALL			(0x0d0a0028)
#define DBGTOP_UNLOCK_KEY			(0xc5acce55)

extern void dfd_setup(void);
extern void dfd_disable(void);
extern void circular_buffer_setup(void);
extern void l2c_parity_check_setup(void);
extern void clear_all_on_mux(void);

extern int plat_debug_smc_dispatcher(unsigned long arg0, unsigned long arg1, unsigned long arg2);

extern void disable_dbgrst_all(void);
#endif
