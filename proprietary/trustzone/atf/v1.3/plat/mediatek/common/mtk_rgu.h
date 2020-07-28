/*
 * Copyright (c) 2014-2015, ARM Limited and Contributors. All rights reserved.
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

#ifndef __MTK_RGU_H__
#define __MTK_RGU_H__

/*******************************************************************************
 * WDT Registers
 ******************************************************************************/
#define MTK_WDT_BASE			(RGU_BASE)
#define MTK_WDT_SIZE			(0x1000)
#define MTK_WDT_MODE			(MTK_WDT_BASE+0x0000)
#define MTK_WDT_LENGTH		(MTK_WDT_BASE+0x0004)
#define MTK_WDT_RESTART		(MTK_WDT_BASE+0x0008)
#define MTK_WDT_STATUS		(MTK_WDT_BASE+0x000C)
#define MTK_WDT_INTERVAL		(MTK_WDT_BASE+0x0010)
#define MTK_WDT_SWRST		(MTK_WDT_BASE+0x0014)
#define MTK_WDT_SWSYSRST		(MTK_WDT_BASE+0x0018)
#define MTK_WDT_NONRST_REG		(MTK_WDT_BASE+0x0020)
#define MTK_WDT_NONRST_REG2		(MTK_WDT_BASE+0x0024)
#define MTK_WDT_REQ_MODE		(MTK_WDT_BASE+0x0030)
#define MTK_WDT_REQ_IRQ_EN		(MTK_WDT_BASE+0x0034)
#define MTK_WDT_EXT_REQ_CON		(MTK_WDT_BASE+0x0038)
#define MTK_WDT_DEBUG_CTL		(MTK_WDT_BASE+0x0040)
#define MTK_WDT_LATCH_CTL		(MTK_WDT_BASE+0x0044)
#define MTK_WDT_DEBUG_CTL2		(MTK_WDT_BASE+0x00A0)
#define MTK_WDT_COUNTER		(MTK_WDT_BASE+0x0514)

/* WDT_STATUS */
#define MTK_WDT_STATUS_SPM_THERMAL_RST	(1 << 0)
#define MTK_WDT_STATUS_SPM_RST		(1 << 1)
#define MTK_WDT_STATUS_EINT_RST		(1 << 2)
#define MTK_WDT_STATUS_SYSRST_RST		(1 << 3) /* from PMIC */
#define MTK_WDT_STATUS_DVFSP_RST		(1 << 4)
#define MTK_WDT_STATUS_PMCU_RST		(1 << 16)
#define MTK_WDT_STATUS_MDDBG_RST		(1 << 17)
#define MTK_WDT_STATUS_THERMAL_DIRECT_RST	(1 << 18)
#define MTK_WDT_STATUS_DEBUG_RST		(1 << 19)
#define MTK_WDT_STATUS_SECURITY_RST	(1 << 28)
#define MTK_WDT_STATUS_IRQ_ASSERT		(1 << 29)
#define MTK_WDT_STATUS_SW_WDT_RST		(1 << 30)
#define MTK_WDT_STATUS_HW_WDT_RST		(1 << 31)

#define MTK_WDT_MODE_DUAL_MODE	0x0040
#define MTK_WDT_MODE_IRQ		0x0008
#define MTK_WDT_MODE_IRQ_LEVEL_EN	0x0020
#define MTK_WDT_MODE_AUTO_RESTART	0x0010 /* Reserved */
#define MTK_WDT_MODE_KEY		0x22000000
#define MTK_WDT_MODE_EXTEN		0x0004
#define MTK_WDT_MODE_ENABLE		0x0001
#define MTK_WDT_SWRST_KEY		0x1209
#define MTK_WDT_RESTART_KEY		0x1971
#define MTK_WDT_LENGTH_KEY		0x0008

void plat_rgu_change_tmo(unsigned int tmo);
void plat_rgu_sw_reset(void);
void plat_rgu_dump_reg(void);

#endif

