/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
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
#include <mmio.h>
#include <platform_def.h>
#include <spm.h>
#include <debug.h>
#include <delay_timer.h>
#include <mcucfg.h>
#include "mtcmos.h"

/* Legacy mode of mtcmos control*/
#define STA_POWER_DOWN		0
#define STA_POWER_ON		1

#define MCUSYS_PROTECTEN_SET	(0x100002B4)
#define MCUSYS_PROTECTEN_STA1	(0x100002C0)
#define MCUSYS_PROTECTEN_CLR	(0x100002B8)
unsigned int cpu_bitmask = 1;

void mtcmos_little_cpu_off(void)
{
}

int spm_mtcmos_ctrl_mp0_cputop_shut_down(int state)
{
	int err = 0;
	/* TINFO="enable SPM register control" */
	spm_write(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (0x1 << 0));

	if (state == STA_POWER_DOWN) {
		/* TINFO="Start to turn off MCU2" */
		/* TINFO="Set bus protect" */
		spm_write(MCUSYS_PROTECTEN_SET, MCU2_PROT_BIT_MASK);
#ifndef IGNORE_MTCMOS_CHECK
		while ((spm_read(MCUSYS_PROTECTEN_STA1) & MCU2_PROT_BIT_ACK_MASK) != MCU2_PROT_BIT_ACK_MASK) {
		}
#endif
		/* TINFO="Set PWR_ISO = 1" */
		spm_write(MCU2_PWR_CON, spm_read(MCU2_PWR_CON) | PWR_ISO);
		/* TINFO="Set SRAM_CKISO = 1" */
		spm_write(MCU2_PWR_CON, spm_read(MCU2_PWR_CON) | SRAM_CKISO);
		/* TINFO="Set SRAM_ISOINT_B = 0" */
		spm_write(MCU2_PWR_CON, spm_read(MCU2_PWR_CON) & ~SRAM_ISOINT_B);
		/* TINFO="Delay 1us" */
		udelay(1);
		/* TINFO="Set SRAM_PDN = 1" */
		spm_write(MCU2_PWR_CON, spm_read(MCU2_PWR_CON) | MCU2_SRAM_PDN);
#ifndef IGNORE_MTCMOS_CHECK
		/* TINFO="Wait until MCU2_SRAM_PDN_ACK = 1" */
		while ((spm_read(MCU2_PWR_CON) & MCU2_SRAM_PDN_ACK) != MCU2_SRAM_PDN_ACK) {
				/* Require 26M for SRAM delay IP */
		}
#endif
		/* TINFO="Set PWR_RST_B = 0" */
		spm_write(MCU2_PWR_CON, spm_read(MCU2_PWR_CON) & ~PWR_RST_B);
		/* TINFO="Set PWR_CLK_DIS = 1" */
		spm_write(MCU2_PWR_CON, spm_read(MCU2_PWR_CON) | PWR_CLK_DIS);
		/* TINFO="Set PWR_ON = 0" */
		spm_write(MCU2_PWR_CON, spm_read(MCU2_PWR_CON) & ~PWR_ON);
		/* TINFO="Set PWR_ON_2ND = 0" */
		spm_write(MCU2_PWR_CON, spm_read(MCU2_PWR_CON) & ~PWR_ON_2ND);
#ifndef IGNORE_MTCMOS_CHECK
		/* TINFO="Wait until CPU_PWR_STATUS = 0 and CPU_PWR_STATUS_2ND = 0" */
		while ((spm_read(CPU_PWR_STATUS) & MCU2_CPU_PWR_STA_MASK)
		       || (spm_read(CPU_PWR_STATUS_2ND) & MCU2_CPU_PWR_STA_MASK)) {
				/* controlled by SPMC; ALL CPU should be in WFI; ACINATS=1 & ACINACTM=1; ?6M required */
		}
#endif
		/* TINFO="CPU_EXT_BUCK_ISO[0]=1"*/
		spm_write(CPU_EXT_BUCK_ISO, spm_read(CPU_EXT_BUCK_ISO) | (0x1 << 0));
		/* TINFO="Finish to turn off MCU2" */
	} else {    /* STA_POWER_ON */
		/* TINFO="Start to turn on MCU2" */
		/* TINFO="CPU_EXT_BUCK_ISO[0]=0"*/
		spm_write(CPU_EXT_BUCK_ISO, spm_read(CPU_EXT_BUCK_ISO) & ~(0x1 << 0));
		/* TINFO="Set PWR_ON = 1" */
		spm_write(MCU2_PWR_CON, spm_read(MCU2_PWR_CON) | PWR_ON);
		/* TINFO="Set PWR_ON_2ND = 1" */
		spm_write(MCU2_PWR_CON, spm_read(MCU2_PWR_CON) | PWR_ON_2ND);
#ifndef IGNORE_MTCMOS_CHECK
		/* TINFO="Wait until CPU_PWR_STATUS = 1 and CPU_PWR_STATUS_2ND = 1" */
		while (((spm_read(CPU_PWR_STATUS) & MCU2_CPU_PWR_STA_MASK) != MCU2_CPU_PWR_STA_MASK)
		       || ((spm_read(CPU_PWR_STATUS_2ND) & MCU2_CPU_PWR_STA_MASK) != MCU2_CPU_PWR_STA_MASK)) {
				/* controlled by SPMC; ALL CPU should be in WFI; ACINATS=1 & ACINACTM=1; ?6M required */
		}
#endif
		/* TINFO="Set PWR_ISO = 0" */
		spm_write(MCU2_PWR_CON, spm_read(MCU2_PWR_CON) & ~PWR_ISO);
		/* TINFO="Set SRAM_PDN = 0" */
		spm_write(MCU2_PWR_CON, spm_read(MCU2_PWR_CON) & ~(0x1 << 8));
#ifndef IGNORE_MTCMOS_CHECK
		/* TINFO="Wait until MCU2_SRAM_PDN_ACK_BIT0 = 0" */
		while (spm_read(MCU2_PWR_CON) & MCU2_SRAM_PDN_ACK_BIT0) {
				/* Require 26M for SRAM delay IP */
		}
#endif
		/* TINFO="Delay 1us" */
		udelay(1);
		/* TINFO="Set SRAM_ISOINT_B = 1" */
		spm_write(MCU2_PWR_CON, spm_read(MCU2_PWR_CON) | SRAM_ISOINT_B);
		/* TINFO="Delay 1us" */
		udelay(1);
		/* TINFO="Set SRAM_CKISO = 0" */
		spm_write(MCU2_PWR_CON, spm_read(MCU2_PWR_CON) & ~SRAM_CKISO);
		/* TINFO="Set PWR_CLK_DIS = 0" */
		spm_write(MCU2_PWR_CON, spm_read(MCU2_PWR_CON) & ~PWR_CLK_DIS);
		/* TINFO="Set PWR_RST_B = 1" */
		spm_write(MCU2_PWR_CON, spm_read(MCU2_PWR_CON) | PWR_RST_B);
		/* TINFO="Release bus protect" */
		spm_write(MCUSYS_PROTECTEN_CLR, MCU2_PROT_BIT_MASK);
#ifndef IGNORE_MTCMOS_CHECK
		/* Note that this protect ack check after releasing protect has been ignored */
#endif
		/* TINFO="Finish to turn on MCU2" */
	}
	return err;
}

int spm_mtcmos_ctrl_mp1_cputop_shut_down(int state)
{
	int err = 0;

	/* TINFO="enable SPM register control" */
	spm_write(POWERON_CONFIG_EN, (SPM_PROJECT_CODE << 16) | (0x1 << 0));

	if (state == STA_POWER_DOWN) {
		/* TINFO="Start to turn off MCU7" */
		/* TINFO="Set bus protect" */
		spm_write(MCUSYS_PROTECTEN_SET, MCU7_PROT_BIT_MASK);
#ifndef IGNORE_MTCMOS_CHECK
		while ((spm_read(MCUSYS_PROTECTEN_STA1) & MCU7_PROT_BIT_ACK_MASK) != MCU7_PROT_BIT_ACK_MASK) {
		}
#endif
		/* TINFO="Set PWR_ISO = 1" */
		spm_write(MCU7_PWR_CON, spm_read(MCU7_PWR_CON) | PWR_ISO);
		/* TINFO="Set SRAM_CKISO = 1" */
		spm_write(MCU7_PWR_CON, spm_read(MCU7_PWR_CON) | SRAM_CKISO);
		/* TINFO="Set SRAM_ISOINT_B = 0" */
		spm_write(MCU7_PWR_CON, spm_read(MCU7_PWR_CON) & ~SRAM_ISOINT_B);
		/* TINFO="Delay 1us" */
		udelay(1);
		/* TINFO="Set SRAM_PDN = 1" */
		spm_write(MCU7_PWR_CON, spm_read(MCU7_PWR_CON) | MCU7_SRAM_PDN);
#ifndef IGNORE_MTCMOS_CHECK
		/* TINFO="Wait until MCU7_SRAM_PDN_ACK = 1" */
		while ((spm_read(MCU7_PWR_CON) & MCU7_SRAM_PDN_ACK) != MCU7_SRAM_PDN_ACK) {
				/* Require 26M for SRAM delay IP */
		}
#endif
		/* TINFO="Set PWR_RST_B = 0" */
		spm_write(MCU7_PWR_CON, spm_read(MCU7_PWR_CON) & ~PWR_RST_B);
		/* TINFO="Set PWR_CLK_DIS = 1" */
		spm_write(MCU7_PWR_CON, spm_read(MCU7_PWR_CON) | PWR_CLK_DIS);
		/* TINFO="Set PWR_ON = 0" */
		spm_write(MCU7_PWR_CON, spm_read(MCU7_PWR_CON) & ~PWR_ON);
		/* TINFO="Set PWR_ON_2ND = 0" */
		spm_write(MCU7_PWR_CON, spm_read(MCU7_PWR_CON) & ~PWR_ON_2ND);
#ifndef IGNORE_MTCMOS_CHECK
		/* TINFO="Wait until CPU_PWR_STATUS = 0 and CPU_PWR_STATUS_2ND = 0" */
		while ((spm_read(CPU_PWR_STATUS) & MCU7_CPU_PWR_STA_MASK)
		       || (spm_read(CPU_PWR_STATUS_2ND) & MCU7_CPU_PWR_STA_MASK)) {
				/* controlled by SPMC; ALL CPU should be in WFI; ACINATS=1 & ACINACTM=1; ?6M required */
		}
#endif
		/* TINFO="CPU_EXT_BUCK_ISO[1]=1"*/
		spm_write(CPU_EXT_BUCK_ISO, spm_read(CPU_EXT_BUCK_ISO) | (0x1 << 1));
		/* TINFO="Finish to turn off MCU7" */
	} else {    /* STA_POWER_ON */
		/* TINFO="Start to turn on MCU7" */
		/* TINFO="CPU_EXT_BUCK_ISO[1]=0"*/
		spm_write(CPU_EXT_BUCK_ISO, spm_read(CPU_EXT_BUCK_ISO) & ~(0x1 << 1));
		/* TINFO="Set PWR_ON = 1" */
		spm_write(MCU7_PWR_CON, spm_read(MCU7_PWR_CON) | PWR_ON);
		/* TINFO="Set PWR_ON_2ND = 1" */
		spm_write(MCU7_PWR_CON, spm_read(MCU7_PWR_CON) | PWR_ON_2ND);
#ifndef IGNORE_MTCMOS_CHECK
		/* TINFO="Wait until CPU_PWR_STATUS = 1 and CPU_PWR_STATUS_2ND = 1" */
		while (((spm_read(CPU_PWR_STATUS) & MCU7_CPU_PWR_STA_MASK) != MCU7_CPU_PWR_STA_MASK)
		       || ((spm_read(CPU_PWR_STATUS_2ND) & MCU7_CPU_PWR_STA_MASK) != MCU7_CPU_PWR_STA_MASK)) {
				/* controlled by SPMC; ALL CPU should be in WFI; ACINATS=1 & ACINACTM=1; ?6M required */
		}
#endif
		/* TINFO="Set PWR_ISO = 0" */
		spm_write(MCU7_PWR_CON, spm_read(MCU7_PWR_CON) & ~PWR_ISO);
		/* TINFO="Set SRAM_PDN = 0" */
		spm_write(MCU7_PWR_CON, spm_read(MCU7_PWR_CON) & ~(0x1 << 8));
#ifndef IGNORE_MTCMOS_CHECK
		/* TINFO="Wait until MCU7_SRAM_PDN_ACK_BIT0 = 0" */
		while (spm_read(MCU7_PWR_CON) & MCU7_SRAM_PDN_ACK_BIT0) {
				/* Require 26M for SRAM delay IP */
		}
#endif
		/* TINFO="Delay 1us" */
		udelay(1);
		/* TINFO="Set SRAM_ISOINT_B = 1" */
		spm_write(MCU7_PWR_CON, spm_read(MCU7_PWR_CON) | SRAM_ISOINT_B);
		/* TINFO="Delay 1us" */
		udelay(1);
		/* TINFO="Set SRAM_CKISO = 0" */
		spm_write(MCU7_PWR_CON, spm_read(MCU7_PWR_CON) & ~SRAM_CKISO);
		/* TINFO="Set PWR_CLK_DIS = 0" */
		spm_write(MCU7_PWR_CON, spm_read(MCU7_PWR_CON) & ~PWR_CLK_DIS);
		/* TINFO="Set PWR_RST_B = 1" */
		spm_write(MCU7_PWR_CON, spm_read(MCU7_PWR_CON) | PWR_RST_B);
		/* TINFO="Release bus protect" */
		spm_write(MCUSYS_PROTECTEN_CLR, MCU7_PROT_BIT_MASK);
#ifndef IGNORE_MTCMOS_CHECK
		/* Note that this protect ack check after releasing protect has been ignored */
#endif
		/* TINFO="Finish to turn on MCU7" */
	}
	return err;
}

int mtcmos_little_core_onoff(int cpu_id, int state, unsigned int chk_wfi)
{
	uint32_t reg_pwr_con;
	uint32_t bit_sram_sleep_b;
	uint32_t bit_sram_pdn;
	uint32_t bit_pwr_sta_mask;
	uint32_t bit_sram_pdn_ack;
//	uint32_t bit_sram_pdn_ack_bit0;
	uint32_t bit_wfi;
	int err = 0;
	switch (cpu_id) {
	case 0:
		reg_pwr_con = MCU3_PWR_CON;
		bit_pwr_sta_mask = MCU3_CPU_PWR_STA_MASK;
		bit_sram_sleep_b = MCU3_SRAM_SLEEP_B;
		bit_sram_pdn = MCU3_SRAM_PDN;
		bit_sram_pdn_ack = MCU3_SRAM_PDN_ACK;
//		bit_sram_pdn_ack_bit0 = MCU3_SRAM_PDN_ACK_BIT0;
		bit_wfi = 0;
		break;
        case 1:
                reg_pwr_con = MCU4_PWR_CON;
                bit_pwr_sta_mask = MCU4_CPU_PWR_STA_MASK;
                bit_sram_sleep_b = MCU4_SRAM_SLEEP_B;
                bit_sram_pdn = MCU4_SRAM_PDN;
                bit_sram_pdn_ack = MCU4_SRAM_PDN_ACK;
//                bit_sram_pdn_ack_bit0 = MCU4_SRAM_PDN_ACK_BIT0;
		bit_wfi = 0;
                break;

        case 2:
                reg_pwr_con = MCU5_PWR_CON;
                bit_pwr_sta_mask = MCU5_CPU_PWR_STA_MASK;
                bit_sram_sleep_b = MCU5_SRAM_SLEEP_B;
                bit_sram_pdn = MCU5_SRAM_PDN;
                bit_sram_pdn_ack = MCU5_SRAM_PDN_ACK;
//                bit_sram_pdn_ack_bit0 = MCU5_SRAM_PDN_ACK_BIT0;
		bit_wfi = 0;
                break;

        case 3:
                reg_pwr_con = MCU6_PWR_CON;
                bit_pwr_sta_mask = MCU6_CPU_PWR_STA_MASK;
                bit_sram_sleep_b = MCU6_SRAM_SLEEP_B;
                bit_sram_pdn = MCU6_SRAM_PDN;
                bit_sram_pdn_ack = MCU6_SRAM_PDN_ACK;
//                bit_sram_pdn_ack_bit0 = MCU6_SRAM_PDN_ACK_BIT0;
		bit_wfi = 0;
                break;

        case 4:
                reg_pwr_con = MCU8_PWR_CON;
                bit_pwr_sta_mask = MCU8_CPU_PWR_STA_MASK;
                bit_sram_sleep_b = MCU8_SRAM_SLEEP_B;
                bit_sram_pdn = MCU8_SRAM_PDN;
                bit_sram_pdn_ack = MCU8_SRAM_PDN_ACK;
//                bit_sram_pdn_ack_bit0 = MCU8_SRAM_PDN_ACK_BIT0;
		bit_wfi = 0;
                break;

        case 5:
                reg_pwr_con = MCU9_PWR_CON;
                bit_pwr_sta_mask = MCU9_CPU_PWR_STA_MASK;
                bit_sram_sleep_b = MCU9_SRAM_SLEEP_B;
                bit_sram_pdn = MCU9_SRAM_PDN;
                bit_sram_pdn_ack = MCU9_SRAM_PDN_ACK;
//                bit_sram_pdn_ack_bit0 = MCU9_SRAM_PDN_ACK_BIT0;
		bit_wfi = 0;
                break;

        case 6:
                reg_pwr_con = MCU10_PWR_CON;
                bit_pwr_sta_mask = MCU10_CPU_PWR_STA_MASK;
                bit_sram_sleep_b = MCU10_SRAM_SLEEP_B;
                bit_sram_pdn = MCU10_SRAM_PDN;
                bit_sram_pdn_ack = MCU10_SRAM_PDN_ACK;
//                bit_sram_pdn_ack_bit0 = MCU10_SRAM_PDN_ACK_BIT0;
		bit_wfi = 0;
                break;

        case 7:
                reg_pwr_con = MCU11_PWR_CON;
                bit_pwr_sta_mask = MCU11_CPU_PWR_STA_MASK;
                bit_sram_sleep_b = MCU11_SRAM_SLEEP_B;
                bit_sram_pdn = MCU11_SRAM_PDN;
                bit_sram_pdn_ack = MCU11_SRAM_PDN_ACK;
//                bit_sram_pdn_ack_bit0 = MCU11_SRAM_PDN_ACK_BIT0;
		bit_wfi = 0;
                break;

	default:
		/* should never come to here */
		return -1;
	}

		/* TINFO="enable SPM register control" */
	spm_write(POWERON_CONFIG_EN, (SPM_PROJECT_CODE << 16) | (0x1 << 0));
	if (state == STA_POWER_DOWN) {
		if (chk_wfi)
                while ((spm_read(SPM_SLEEP_TIMER_STA) & bit_wfi) == 0)
                        continue;

		/* TINFO="Start to turn off MCUx" */
		/* TINFO="Set PWR_ISO = 1" */
		spm_write(reg_pwr_con, spm_read(reg_pwr_con) | PWR_ISO);
		/* TINFO="Set SRAM_ISOINT_B = 0" */
		spm_write(reg_pwr_con, spm_read(reg_pwr_con) & ~SRAM_ISOINT_B);
		/* TINFO="Set SRAM_CKISO = 1" */
		spm_write(reg_pwr_con, spm_read(reg_pwr_con) | SRAM_CKISO);
		/* TINFO="Set PWR_RST_B = 0" */
		spm_write(reg_pwr_con, spm_read(reg_pwr_con) & ~PWR_RST_B);
		/* TINFO="Set PWR_CLK_DIS = 1" */
		spm_write(reg_pwr_con, spm_read(reg_pwr_con) | PWR_CLK_DIS);
		/* TINFO="Set PWR_ON = 0" */
		spm_write(reg_pwr_con, spm_read(reg_pwr_con) & ~PWR_ON);
		/* TINFO="Set PWR_ON_2ND = 0" */
		spm_write(reg_pwr_con, spm_read(reg_pwr_con) & ~PWR_ON_2ND);
		/* TINFO="Set SRAM_SLEEP_B = 0" */
		spm_write(reg_pwr_con, spm_read(reg_pwr_con) & ~bit_sram_sleep_b);
#ifndef IGNORE_MTCMOS_CHECK
		/* TINFO="Wait until CPU_PWR_STATUS = 0 and CPU_PWR_STATUS_2ND = 0" */
		while ((spm_read(CPU_PWR_STATUS) & bit_pwr_sta_mask)
		       || (spm_read(CPU_PWR_STATUS_2ND) & bit_pwr_sta_mask)) {
				/* controlled by SPMC; CPU should be in WFI; CPU retention must be enabled; ?6M required */
		}
#endif
		/* TINFO="Delay 1us" */
		udelay(1);
		/* TINFO="Set SRAM_PDN = 1" */
		spm_write(reg_pwr_con, spm_read(reg_pwr_con) | bit_sram_pdn);
#ifndef IGNORE_MTCMOS_CHECK
		/* TINFO="Wait until MCUx_SRAM_PDN_ACK = 1" */
		while ((spm_read(reg_pwr_con) & MCU4_SRAM_PDN_ACK) != bit_sram_pdn_ack) {
				/* Require 26M for SRAM delay IP */
		}
#endif
		/* TINFO="Set SLPB_CLAMP = 1" */
		spm_write(reg_pwr_con, spm_read(reg_pwr_con) | SLPB_CLAMP);
		/* TINFO="Finish to turn off MCUx" */
	} else {    /* STA_POWER_ON */
		/* TINFO="Start to turn on MCUx" */
#ifndef IGNORE_MTCMOS_CHECK
		/* TINFO="Wait until MCU4_SRAM_PDN_ACK = 1" */
		while ((spm_read(reg_pwr_con) & bit_sram_pdn_ack) != bit_sram_pdn_ack) {
				/* Require 26M for SRAM delay IP */
		}
#endif
		/* TINFO="Set SLPB_CLAMP = 0" */
		spm_write(reg_pwr_con, spm_read(reg_pwr_con) & ~SLPB_CLAMP);
		/* TINFO="Set SRAM_PDN = 0" */
		spm_write(reg_pwr_con, spm_read(reg_pwr_con) & ~(0x1 << 8));
#if  IGNORE_MTCMOS_CHECK
		/* TINFO="Wait until MCU4_SRAM_PDN_ACK_BIT0 = 0" */
		while (spm_read(reg_pwr_con) & bit_sram_pdn_ack_bit0) {
				/* Require 26M for SRAM delay IP */
		}
#endif
		/* TINFO="Set PWR_ON = 1" */
		spm_write(reg_pwr_con, spm_read(reg_pwr_con) | PWR_ON);
		/* TINFO="Set PWR_ON_2ND = 1" */
		spm_write(reg_pwr_con, spm_read(reg_pwr_con) | PWR_ON_2ND);
#if IGNORE_MTCMOS_CHECK
		/* TINFO="Wait until CPU_PWR_STATUS = 1 and CPU_PWR_STATUS_2ND = 1" */
		while (((spm_read(CPU_PWR_STATUS) & bit_pwr_sta_mask) != bit_pwr_sta_mask)
		       || ((spm_read(CPU_PWR_STATUS_2ND) & bit_pwr_sta_mask) != bit_pwr_sta_mask)) {
				/* controlled by SPMC; CPU should be in WFI; CPU retention must be enabled; ?6M required */
		}
#endif
		/* TINFO="Set SRAM_SLEEP_B = 1" */
		spm_write(reg_pwr_con, spm_read(reg_pwr_con) | bit_sram_sleep_b);
		/* TINFO="Set SRAM_ISOINT_B = 1" */
		spm_write(reg_pwr_con, spm_read(reg_pwr_con) | SRAM_ISOINT_B);
		/* TINFO="Set PWR_ISO = 0" */
		spm_write(reg_pwr_con, spm_read(reg_pwr_con) & ~PWR_ISO);
		/* TINFO="Delay 1us" */
		udelay(1);
		/* TINFO="Set SRAM_CKISO = 0" */
		spm_write(reg_pwr_con, spm_read(reg_pwr_con) & ~SRAM_CKISO);
		/* TINFO="Set PWR_CLK_DIS = 0" */
		spm_write(reg_pwr_con, spm_read(reg_pwr_con) & ~PWR_CLK_DIS);
		/* TINFO="Set PWR_RST_B = 1" */
		spm_write(reg_pwr_con, spm_read(reg_pwr_con) | PWR_RST_B);
		/* TINFO="Finish to turn on MCUx" */
	}
	return err;
}
