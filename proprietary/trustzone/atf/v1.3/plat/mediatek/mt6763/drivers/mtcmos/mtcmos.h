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
#ifndef __MTCMOS_H__
#define __MTCMOS_H__

/* Define MTCMOS power control */
enum {
	SRAM_SLEEP_B_ACK	= 1U << 28,
	SRAM_PDN_ACK		= 1U << 24,
	SRAM_SLEEP_B		= 1U << 12,
	SRAM_PDN		= 1U << 8,
	SRAM_PD_SLPB_CLAMP	= 1U << 7,
	SRAM_ISOINT_B		= 1U << 6,
	SRAM_CKISO		= 1U << 5,
	PWR_CLK_DIS		= 1U << 4,
	PWR_ON_2ND		= 1U << 3,
	PWR_ON			= 1U << 2,
	PWR_ISO			= 1U << 1,
	PWR_RST_B		= 1U << 0
};

enum {
	MP1_CPU3_PWR_STA_MASK	= 1U << 19,
	MP1_CPU2_PWR_STA_MASK	= 1U << 18,
	MP1_CPU1_PWR_STA_MASK	= 1U << 17,
	MP1_CPU0_PWR_STA_MASK	= 1U << 16,
	MP1_CPUTOP_PWR_STA_MASK	= 1U << 15,
	MCU_PWR_STA_MASK	= 1U << 14,
	MP0_CPU3_PWR_STA_MASK	= 1U << 12,
	MP0_CPU2_PWR_STA_MASK	= 1U << 11,
	MP0_CPU1_PWR_STA_MASK	= 1U << 10,
	MP0_CPU0_PWR_STA_MASK	= 1U << 9,
	MP0_CPUTOP_PWR_STA_MASK	= 1U << 8,
};

enum {
	L1_PDN_ACK	= 1U << 8,
	L1_PDN		= 1U << 0
};

enum {
	LITTLE_CPU3	= 1U << 12,
	LITTLE_CPU2	= 1U << 11,
	LITTLE_CPU1	= 1U << 10,
};

extern unsigned int cpu_bitmask;

/*
 * This function will power off CPU# in cluster 0.
 */
int spm_mtcmos_ctrl_mp0_cputop_dormant(int state);
int spm_mtcmos_ctrl_mp0_cputop_shut_down(int state);
int spm_mtcmos_ctrl_mp0_cpu0(int state);
int spm_mtcmos_ctrl_mp0_cpu1(int state);
int spm_mtcmos_ctrl_mp0_cpu2(int state);
int spm_mtcmos_ctrl_mp0_cpu3(int state);

int spm_mtcmos_ctrl_mp1_cputop_dormant(int state);
int spm_mtcmos_ctrl_mp1_cputop_shut_down(int state);
int spm_mtcmos_ctrl_mp1_cpu0(int state);
int spm_mtcmos_ctrl_mp1_cpu1(int state);
int spm_mtcmos_ctrl_mp1_cpu2(int state);
int spm_mtcmos_ctrl_mp1_cpu3(int state);

int mtcmos_cluster0_cpu_on(unsigned int linear_id);
int mtcmos_cluster0_cpu_off(unsigned int linear_id);
int mtcmos_cluster0_cpusys_on(unsigned int linear_id);
int mtcmos_cluster0_cpusys_off(unsigned int linear_id);

int mtcmos_cluster1_cpu_on(unsigned int linear_id);
int mtcmos_cluster1_cpu_off(unsigned int linear_id);
int mtcmos_cluster1_cpusys_on(unsigned int linear_id);
int mtcmos_cluster1_cpusys_off(unsigned int linear_id);

int mtcmos_cluster0_power(unsigned int onoff);
int mtcmos_cluster1_power(unsigned int onoff);

#endif /* __MTCMOS_H__ */
