/*
 * Copyright (c) 2015,  ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms,  with or without
 * modification,  are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice,  this
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
 * AND ANY EXPRESS OR IMPLIED WARRANTIES,  INCLUDING,  BUT NOT LIMITED TO,  THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
 * CONSEQUENTIAL DAMAGES (INCLUDING,  BUT NOT LIMITED TO,  PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,  DATA,  OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,  WHETHER IN
 * CONTRACT,  STRICT LIABILITY,  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <platform_def.h>
#include <assert.h>
#include <debug.h>
#include <delay_timer.h>
#include <mmio.h>

#include "spmc_private.h"


#define SPMC_DEBUG

#ifdef SPMC_DEBUG
#define pr_debug(fmt, ...)  INFO(fmt, ##__VA_ARGS__)
#else
#define pr_debug(fmt, ...)
#endif

#if 0 /* require view before remove */

#define RETRY_TIME_USEC   (10)
#define RETRY_TIME_CURR   (1)
#define DELAY_TIME_MEASURE (1)//(20000)//(1)

/* #define IGNORE_PWR_ACK		1 */
#define DDR_RESERVE_ENABLE	1
#define AO_REG_PATCH (1)


/* APB Module infracfg_ao */
/*
#define INFRA_TOPAXI_PROTECTEN_1_SET (INFRACFG_AO_BASE + 0x2B4)
#define INFRA_TOPAXI_PROTECTEN_1_CLR (INFRACFG_AO_BASE + 0x2B8)
#define INFRA_TOPAXI_PROTECTSTA1_1	(INFRACFG_AO_BASE + 0x2BC)
*/

#define MCUSYS_PROTECTEN_SET		(INFRACFG_AO_BASE + 0x2B4)
#define MCUSYS_PROTECTEN_CLR		(INFRACFG_AO_BASE + 0x2B8)
#define MCUSYS_PROTECTEN_STA1		(INFRACFG_AO_BASE + 0x2C0)


#define IDX_PROTECT_MP0_CACTIVE		0
#define IDX_PROTECT_ICC0_CACTIVE	1
#define IDX_PROTECT_ICD0_CACTIVE	2
#define IDX_PROTECT_L2C0_CACTIVE	3

#define IDX_PROTECT_MP1_CACTIVE		4
#define IDX_PROTECT_ICC1_CACTIVE	5
#define IDX_PROTECT_ICD1_CACTIVE	6
#define IDX_PROTECT_L2C1_CACTIVE	7

#define IDX_PROTECT_MP2_CACTIVE		8
#define IDX_PROTECT_ICC2_CACTIVE	9
#define IDX_PROTECT_ICD2_CACTIVE	10
#define IDX_PROTECT_L2C2_CACTIVE	11
// TODO: ACP protection
#define IDX_PROTECT_ACP0_CACTIVE	16
#define IDX_PROTECT_ACP1_CACTIVE	17

/* #define MPx_SNOOP_CTRL				(0x10390000) */
#define MP0_SNOOP_CTRL				(0x0C501000)
#define MP1_SNOOP_CTRL				(0x0C501100)
#define MP2_SNOOP_CTRL				(0x0C501200)
#define MPx_SNOOP_STATUS			(0x0C500028)	/*(0x1039000C) */
#define MPx_SNOOP_ENABLE			(0x3)

#define DVTFlowCtrl				 (0x10200070)

#define mcu_spm_read(addr)		spm_read(addr)
#define mcu_spm_write(addr,  val)	spm_write(addr,  val)

char big_on = 0;		/* at most 4 cores */
char little_on = 0x1;		/* [7:0] = core7~core0,  core 0 is power-on in defualt */

int mcsib_sw_workaround_main(void);
void spark2_setldo_AMUXSEL(int cputop_mpx, unsigned int cpu_corex, unsigned int amuxsel,unsigned int vret);
unsigned long read_cpuectlr(void);
void write_cpuectlr(unsigned long);

void little_spmc_info(void)
{
	PRINTF_SPMC("============== cluster0 =================\n");
	PRINTF_SPMC("Little SW HW T0:0x%x \t0x%x\n", mmio_read_32(MP0_CPUTOP_SPMC_CTL),
		    mmio_read_32(MP0_CPUTOP_PWR_CON));
	PRINTF_SPMC("Little SW HW c0:0x%x \t0x%x\n", mmio_read_32(CPUSYS0_CPU0_SPMC_CTL),
		    mmio_read_32(MP0_CPU0_PWR_CON));
	PRINTF_SPMC("Little SW HW c1:0x%x \t0x%x\n", mmio_read_32(CPUSYS0_CPU1_SPMC_CTL),
		    mmio_read_32(MP0_CPU1_PWR_CON));
	PRINTF_SPMC("Little SW HW c2:0x%x \t0x%x\n", mmio_read_32(CPUSYS0_CPU2_SPMC_CTL),
		    mmio_read_32(MP0_CPU2_PWR_CON));
	PRINTF_SPMC("Little SW HW c3:0x%x \t0x%x\n", mmio_read_32(CPUSYS0_CPU3_SPMC_CTL),
		    mmio_read_32(MP0_CPU3_PWR_CON));
	PRINTF_SPMC("============== cluster1 =================\n");
	PRINTF_SPMC("Little SW HW T1:0x%x \t0x%x\n", mmio_read_32(MP1_CPUTOP_SPMC_CTL),
		    mmio_read_32(MP1_CPUTOP_PWR_CON));
	PRINTF_SPMC("Little SW HW c4:0x%x \t0x%x\n", mmio_read_32(CPUSYS1_CPU0_SPMC_CTL),
		    mmio_read_32(MP1_CPU0_PWR_CON));
	PRINTF_SPMC("Little SW HW c5:0x%x \t0x%x\n", mmio_read_32(CPUSYS1_CPU1_SPMC_CTL),
		    mmio_read_32(MP1_CPU0_PWR_CON));
	PRINTF_SPMC("Little SW HW c6:0x%x \t0x%x\n", mmio_read_32(CPUSYS1_CPU2_SPMC_CTL),
		    mmio_read_32(MP1_CPU0_PWR_CON));
	PRINTF_SPMC("Little SW HW c7:0x%x \t0x%x\n", mmio_read_32(CPUSYS1_CPU3_SPMC_CTL),
		    mmio_read_32(MP1_CPU0_PWR_CON));
	PRINTF_SPMC("=========================================\n");
}

void little_wfi_wfe_status(){
	unsigned int tmp;
	PRINTF_SPMC("CPU_IDLE_STA_0x%x=0x%x\n", CPU_IDLE_STA,mmio_read_32(CPU_IDLE_STA));

	tmp=mmio_read_32(MP0_CA7_MISC_CONFIG);
	PRINTF_SPMC("MP0_CA7_MISC_CONFIG%x=0x%x\n",MP0_CA7_MISC_CONFIG,tmp);
	PRINTF_SPMC("cor3~0_WFE=%d %d %d %d\n",(tmp&MP0_CPU3_STANDBYWFE)>>23,(tmp&MP0_CPU2_STANDBYWFE)>>22,(tmp&MP0_CPU1_STANDBYWFE)>>21,(tmp&MP0_CPU0_STANDBYWFE)>>20);
	tmp=mmio_read_32(MP1_CA7_MISC_CONFIG);
	PRINTF_SPMC("MP1_CA7_MISC_CONFIG%x=0x%x\n",MP1_CA7_MISC_CONFIG,tmp);
	PRINTF_SPMC("cor7~4_WFE=%d %d %d %d\n",(tmp&MP1_CPU3_STANDBYWFE)>>23,(tmp&MP1_CPU2_STANDBYWFE)>>22,(tmp&MP1_CPU1_STANDBYWFE)>>21,(tmp&MP1_CPU0_STANDBYWFE)>>20);
	mmio_write_32(CA15M_MON_SEL, (mmio_read_32(CA15M_MON_SEL) & 0xFF) | 0x1b);
	tmp=mmio_read_32(MP2_CA15M_MON_L);
	PRINTF_SPMC("MP1_CA7_MISC_CONFIG%x=0x%x\n",MP2_CA15M_MON_L,tmp);
	PRINTF_SPMC("cor9~8_WFE=%d %d\n",(tmp&MP2_CPU1_STANDBYWFE)>>5,(tmp&MP2_CPU0_STANDBYWFE)>>4);
}
#if 0
static void big_spmc_info(void)
{
// if it print CPUSYS2_CPU0_SPMC_CTL and CPUSYS2_CPU1_SPMC_CTL when cluster on power off state.
//the cluster poser on flow will fail
PRINTF_SPMC("Big SPMC CTL Top2:0x%x\n",mmio_read_32(MP2_CPUTOP_SPMC_CTL));
PRINTF_SPMC("Big SPMC STA Top2:0x%x\n",mmio_read_32(MP2_CPUTOP_SPMC_STA));
/*
PRINTF_SPMC("Big SwSeq SPMC T2:0x%x C0:0x%x C1:0x%x\n",
				mmio_read_32(MP2_CPUTOP_SPMC_CTL),
				mmio_read_32(CPUSYS2_CPU0_SPMC_CTL),
				mmio_read_32(CPUSYS2_CPU1_SPMC_CTL));
PRINTF_SPMC("Big SwSeq SPMC T2:0x%x C0:0x%x C1:0x%x\n",
				mmio_read_32(MP2_CPUTOP_SPMC_STA),
				mmio_read_32(CPUSYS2_CPU0_SPMC_STA),
				mmio_read_32(CPUSYS2_CPU1_SPMC_STA));
PRINTF_SPMC("PWR_STATUS_0x%x=0x%x\n", PWR_STATUS, mmio_read_32(PWR_STATUS));
*/
}
#endif
void set_cpu_retention_control(int retention_value)
{
	uint64_t cpuectlr;

	cpuectlr = read_cpuectlr();
	cpuectlr = ((cpuectlr >> 3) << 3);
	cpuectlr |= retention_value;
	write_cpuectlr(cpuectlr);
}


/*
 * SPMC Mode
 */

/*
GG
	Set bypass_cpu_spmc_mode = 0
	Wait 200ns
	FOR ( each cluster n)
	FOR (each core m)
			Set mp<n>_spmc_resetpwron_config_cpu<m> = 0
			Set mp<n>_spmc_pwr_rst_cpu<m> = 0
		ENDFOR
		Set mp<n>_spmc_resetpwron_config_cputop = 0
		Set mp<n>_spmc_pwr_rst_cputop = 0
		Set mp<n>_spmc_pwr_clk_dis_cputop = 0
	ENDFOR
*/

int spmc_init(void)
{
	int err = 0;

	/* TINFO="enable SPM register control" */
	mmio_write_32(POWERON_CONFIG_EN, (SPM_PROJECT_CODE << 16) | (0x1 << 0));
	PRINTF_SPMC("POWERON_CONFIG_EN_0x%x=0x%x\n", POWERON_CONFIG_EN, mmio_read_32(POWERON_CONFIG_EN));
#if CONFIG_SPMC_MODE != 0
	mmio_write_32(BYPASS_SPMC, 0x0);	/* de-assert Bypass SPMC  0: SPMC mode  1: Legacy mode */
	PRINTF_SPMC("BYPASS_SPMC_0x%x=0x%x\n", BYPASS_SPMC, mmio_read_32(BYPASS_SPMC));
	/* udelay(200); */
	PRINTF_SPMC("[%s]change to SPMC mode !!!\n", __func__);
#endif
	/*mmio_write_32(0x10202008, 0x1);*/
#if 0
	mmio_write_32(MP1_CPU0_PWR_CON, mmio_read_32(MP1_CPU0_PWR_CON) & ~PWR_ON_2ND);
	PRINTF_SPMC("MP1_CPU0_PWR_CON_0x%x=0x%x\n", MP1_CPU0_PWR_CON, mmio_read_32(MP1_CPU0_PWR_CON));
	mmio_write_32(MP1_CPU0_PWR_CON, mmio_read_32(MP1_CPU0_PWR_CON) | PWR_RST_B);
	PRINTF_SPMC("MP1_CPU0_PWR_CON_0x%x=0x%x\n", MP1_CPU0_PWR_CON, mmio_read_32(MP1_CPU0_PWR_CON));

	mmio_write_32(MP1_CPUTOP_PWR_CON, mmio_read_32(MP1_CPUTOP_PWR_CON) & ~PWR_ON_2ND);
	PRINTF_SPMC("MP1_CPUTOP_PWR_CON_0x%x=0x%x\n", MP1_CPUTOP_PWR_CON, mmio_read_32(MP1_CPUTOP_PWR_CON));
	mmio_write_32(MP1_CPUTOP_PWR_CON, mmio_read_32(MP1_CPUTOP_PWR_CON) | PWR_RST_B);
	PRINTF_SPMC("MP1_CPUTOP_PWR_CON_0x%x=0x%x\n", MP1_CPUTOP_PWR_CON, mmio_read_32(MP1_CPUTOP_PWR_CON));
	mmio_write_32(MP1_CPUTOP_PWR_CON, mmio_read_32(MP1_CPUTOP_PWR_CON) & ~PWR_CLK_DIS);
	PRINTF_SPMC("MP1_CPUTOP_PWR_CON_0x%x=0x%x\n", MP1_CPUTOP_PWR_CON, mmio_read_32(MP1_CPUTOP_PWR_CON));

	mmio_write_32(MP2_CPU0_PWR_CON, mmio_read_32(MP2_CPU0_PWR_CON) & ~PWR_ON_2ND);
	PRINTF_SPMC("MP2_CPU0_PWR_CON_0x%x=0x%x\n", MP2_CPU0_PWR_CON, mmio_read_32(MP2_CPU0_PWR_CON));
	mmio_write_32(MP2_CPU0_PWR_CON, mmio_read_32(MP2_CPU0_PWR_CON) | PWR_RST_B);
	PRINTF_SPMC("MP2_CPU0_PWR_CON_0x%x=0x%x\n", MP2_CPU0_PWR_CON, mmio_read_32(MP2_CPU0_PWR_CON));

	mmio_write_32(MP2_CPUTOP_PWR_CON, mmio_read_32(MP2_CPUTOP_PWR_CON) & ~PWR_ON_2ND);
	PRINTF_SPMC("MP2_CPUTOP_PWR_CON_0x%x=0x%x\n", MP2_CPUTOP_PWR_CON, mmio_read_32(MP2_CPUTOP_PWR_CON));
	mmio_write_32(MP2_CPUTOP_PWR_CON, mmio_read_32(MP2_CPUTOP_PWR_CON) | PWR_RST_B);
	PRINTF_SPMC("MP2_CPUTOP_PWR_CON_0x%x=0x%x\n", MP2_CPUTOP_PWR_CON, mmio_read_32(MP2_CPUTOP_PWR_CON));
	mmio_write_32(MP2_CPUTOP_PWR_CON, mmio_read_32(MP2_CPUTOP_PWR_CON) & ~PWR_CLK_DIS);
	PRINTF_SPMC("MP2_CPUTOP_PWR_CON_0x%x=0x%x\n", MP2_CPUTOP_PWR_CON, mmio_read_32(MP2_CPUTOP_PWR_CON));

#else
/* MP0 SPMC power Ctrl signals */
	mmio_write_32(MP0_CPU0_PWR_CON, mmio_read_32(MP0_CPU0_PWR_CON) & ~PWR_ON_2ND);
	mmio_write_32(MP0_CPU0_PWR_CON, mmio_read_32(MP0_CPU0_PWR_CON) | PWR_RST_B);
	mmio_write_32(MP0_CPU1_PWR_CON, mmio_read_32(MP0_CPU1_PWR_CON) & ~PWR_ON_2ND);
	mmio_write_32(MP0_CPU1_PWR_CON, mmio_read_32(MP0_CPU1_PWR_CON) | PWR_RST_B);
	mmio_write_32(MP0_CPU2_PWR_CON, mmio_read_32(MP0_CPU2_PWR_CON) & ~PWR_ON_2ND);
	mmio_write_32(MP0_CPU2_PWR_CON, mmio_read_32(MP0_CPU2_PWR_CON) | PWR_RST_B);
	mmio_write_32(MP0_CPU3_PWR_CON, mmio_read_32(MP0_CPU3_PWR_CON) & ~PWR_ON_2ND);
	mmio_write_32(MP0_CPU3_PWR_CON, mmio_read_32(MP0_CPU3_PWR_CON) | PWR_RST_B);

	mmio_write_32(MP0_CPUTOP_PWR_CON, mmio_read_32(MP0_CPUTOP_PWR_CON) & ~PWR_ON_2ND);
	mmio_write_32(MP0_CPUTOP_PWR_CON, mmio_read_32(MP0_CPUTOP_PWR_CON) | PWR_RST_B);
	mmio_write_32(MP0_CPUTOP_PWR_CON, mmio_read_32(MP0_CPUTOP_PWR_CON) & ~PWR_CLK_DIS);
/* MP1 SPMC power Ctrl signals */
	mmio_write_32(MP1_CPU0_PWR_CON, mmio_read_32(MP1_CPU0_PWR_CON) & ~PWR_ON_2ND);
	mmio_write_32(MP1_CPU0_PWR_CON, mmio_read_32(MP1_CPU0_PWR_CON) | PWR_RST_B);
	mmio_write_32(MP1_CPU1_PWR_CON, mmio_read_32(MP1_CPU1_PWR_CON) & ~PWR_ON_2ND);
	mmio_write_32(MP1_CPU1_PWR_CON, mmio_read_32(MP1_CPU1_PWR_CON) | PWR_RST_B);
	mmio_write_32(MP1_CPU2_PWR_CON, mmio_read_32(MP1_CPU2_PWR_CON) & ~PWR_ON_2ND);
	mmio_write_32(MP1_CPU2_PWR_CON, mmio_read_32(MP1_CPU2_PWR_CON) | PWR_RST_B);
	mmio_write_32(MP1_CPU3_PWR_CON, mmio_read_32(MP1_CPU3_PWR_CON) & ~PWR_ON_2ND);
	mmio_write_32(MP1_CPU3_PWR_CON, mmio_read_32(MP1_CPU3_PWR_CON) | PWR_RST_B);

	mmio_write_32(MP1_CPUTOP_PWR_CON, mmio_read_32(MP1_CPUTOP_PWR_CON) & ~PWR_ON_2ND);
	mmio_write_32(MP1_CPUTOP_PWR_CON, mmio_read_32(MP1_CPUTOP_PWR_CON) | PWR_RST_B);
	mmio_write_32(MP1_CPUTOP_PWR_CON, mmio_read_32(MP1_CPUTOP_PWR_CON) & ~PWR_CLK_DIS);
/* MP2 SPMC power Ctrl signals */
	mmio_write_32(MP2_CPU0_PWR_CON, mmio_read_32(MP2_CPU0_PWR_CON) & ~PWR_ON_2ND);
	mmio_write_32(MP2_CPU0_PWR_CON, mmio_read_32(MP2_CPU0_PWR_CON) | PWR_RST_B);
	mmio_write_32(MP2_CPU1_PWR_CON, mmio_read_32(MP2_CPU1_PWR_CON) & ~PWR_ON_2ND);
	mmio_write_32(MP2_CPU1_PWR_CON, mmio_read_32(MP2_CPU1_PWR_CON) | PWR_RST_B);

	mmio_write_32(MP2_CPUTOP_PWR_CON, mmio_read_32(MP2_CPUTOP_PWR_CON) & ~PWR_ON_2ND);
	mmio_write_32(MP2_CPUTOP_PWR_CON, mmio_read_32(MP2_CPUTOP_PWR_CON) | PWR_RST_B);
	mmio_write_32(MP2_CPUTOP_PWR_CON, mmio_read_32(MP2_CPUTOP_PWR_CON) & ~PWR_CLK_DIS);


/* setup_sw_cluster_default_state(0x10,STA_POWER_ON); */
/* setup_sw_core_default_state(0x0,STA_POWER_ON); */

/* MP0 not support
#if SPMC_SPARK2
		little_spark2_setldo(0);
		little_spark2_core_enable(0,1);
#endif
*/
#endif

	return err;
}


int spmc_cputop_mpx_onoff(int cputop_mpx, int state, int mode)
{
	int err = 0;
	unsigned int mpx_mask;
#if SPMC_DVT
	unsigned int MPx_AXI_CONFIG, MPx_SNOOP_CTRL;
#endif
	unsigned int MPx_CPUTOP_SPMC, MPx_CPUTOP_PWR_CON;
	unsigned int MPx_CPUTOP_PWR_STA_MASK, MPx_CPUx_STANDBYWFI;

	/* TINFO="enable SPM register control" */
	mmio_write_32(POWERON_CONFIG_EN, (SPM_PROJECT_CODE << 16) | (0x1 << 0));

	PRINTF_SPMC("######## %s cputop_mpx=%d state=%d mode=%d\n", __func__, cputop_mpx, state,
		    mode);

	if (cputop_mpx == CPUTOP_MP0) {
		mpx_mask = (1 << IDX_PROTECT_ICC0_CACTIVE) |
		    (1 << IDX_PROTECT_ICD0_CACTIVE) |
		    (1 << IDX_PROTECT_MP0_CACTIVE) | (1 << IDX_PROTECT_L2C0_CACTIVE);
#if SPMC_DVT
		MPx_AXI_CONFIG = MP0_AXI_CONFIG;
		MPx_SNOOP_CTRL = MP0_SNOOP_CTRL;
#endif
		MPx_CPUTOP_SPMC = MP0_CPUTOP_SPMC_CTL;
		MPx_CPUTOP_PWR_CON = MP0_CPUTOP_PWR_CON;
		MPx_CPUTOP_PWR_STA_MASK = MP0_CPUTOP_PWR_STA_MASK;
		MPx_CPUx_STANDBYWFI = MP0_STANDBYWFI;
	} else if (cputop_mpx == CPUTOP_MP1) {
		mpx_mask = (1 << IDX_PROTECT_ICC1_CACTIVE) |
		    (1 << IDX_PROTECT_ICD1_CACTIVE) |
		    (1 << IDX_PROTECT_MP1_CACTIVE) | (1 << IDX_PROTECT_L2C1_CACTIVE);
#if SPMC_DVT
		MPx_AXI_CONFIG = MP1_AXI_CONFIG;
		MPx_SNOOP_CTRL = MP1_SNOOP_CTRL;
#endif
		MPx_CPUTOP_SPMC = MP1_CPUTOP_SPMC_CTL;
		MPx_CPUTOP_PWR_CON = MP1_CPUTOP_PWR_CON;
		MPx_CPUTOP_PWR_STA_MASK = MP1_CPUTOP_PWR_STA_MASK;
		MPx_CPUx_STANDBYWFI = MP1_STANDBYWFI;
	} else {
		mpx_mask = (1 << IDX_PROTECT_ICC2_CACTIVE) |
		    (1 << IDX_PROTECT_ICD2_CACTIVE) |
		    (1 << IDX_PROTECT_MP2_CACTIVE) | (1 << IDX_PROTECT_L2C2_CACTIVE);
#if SPMC_DVT
		MPx_AXI_CONFIG = MP2_AXI_CONFIG;
		MPx_SNOOP_CTRL = MP2_SNOOP_CTRL;
#endif
		MPx_CPUTOP_SPMC = MP2_CPUTOP_SPMC_CTL;
		MPx_CPUTOP_PWR_CON = MP2_CPUTOP_PWR_CON;
		MPx_CPUTOP_PWR_STA_MASK = MP2_CPUTOP_PWR_STA_MASK;
		MPx_CPUx_STANDBYWFI = MP2_STANDBYWFI;
	}

	/*PRINTF_SPMC("MPx_CPUTOP_PWR_STA_MASK=%x\n", MPx_CPUTOP_PWR_STA_MASK);*/
	/*PRINTF_SPMC("MPx_AXI_CONFIG=%x\n", MPx_AXI_CONFIG);*/
	/*PRINTF_SPMC("mpx_mask=%x\n", mpx_mask);*/
	/*PRINTF_SPMC("MPx_SNOOP_CTRL=%x\n", MPx_SNOOP_CTRL);*/


	if (state == STA_POWER_DOWN) {

#if SPMC_DVT			/* by callee in plat_affinst_off() */
		/* TINFO="Start to turn off MP0_CPUTOP" */
/* ################ BUS PROTECT ENABLE and SNOOP/DVM DISABLE ################ */
		mmio_write_32(MPx_SNOOP_CTRL, mmio_read_32(MPx_SNOOP_CTRL) & ~MPx_SNOOP_ENABLE);
		PRINTF_SPMC("MPx_SNOOP_CTRL_0x%x=0x%x\n", MPx_SNOOP_CTRL, mmio_read_32(MPx_SNOOP_CTRL));
		while ((mmio_read_32(MPx_SNOOP_STATUS) & (1 << 31)) == (1 << 31));
		PRINTF_SPMC("MPx_SNOOP_STATUS_0x%x=0x%x\n", MPx_SNOOP_STATUS, mmio_read_32(MPx_SNOOP_STATUS));
		/* mcsib_sw_workaround_main(); */
		/*      Program MP<n>_AXI_CONFIG acinactm to 1 */
		/*      Wait mp<n>_STANDBYWFIL2 to high */
		/* TINFO="Set acinactm = 1" */
		if (cputop_mpx == CPUTOP_MP2)
			mmio_write_32(MPx_AXI_CONFIG,
				      mmio_read_32(MPx_AXI_CONFIG) | MP2_AXI_CONFIG_acinactm |
				      MP2_AXI_CONFIG_ainacts);
		else
			mmio_write_32(MPx_AXI_CONFIG,
				      mmio_read_32(MPx_AXI_CONFIG) | MPx_AXI_CONFIG_acinactm |
				      MPx_AXI_CONFIG_ainacts);
		PRINTF_SPMC("MPx_AXI_CONFIG_0x%x=0x%x\n", MPx_AXI_CONFIG, mmio_read_32(MPx_AXI_CONFIG));
#endif
		/* TINFO="Wait STANDBYWFIL2 for Cluster 0" */
		while (!(mmio_read_32(CPU_IDLE_STA) & MPx_CPUx_STANDBYWFI));
		PRINTF_SPMC("CPU_IDLE_STA_0x%x=0x%x\n", CPU_IDLE_STA, mmio_read_32(CPU_IDLE_STA));

		/* TINFO="Set ADB pwrdnreqn for Cluster 0" */
		/* mmio_write_32(INFRA_TOPAXI_PROTECTEN_1,  mmio_read_32(INFRA_TOPAXI_PROTECTEN_1)|mpx_mask); */
		mmio_write_32(MCUSYS_PROTECTEN_SET, mpx_mask);
		PRINTF_SPMC("MCUSYS_PROTECTEN_SET_0x%x=0x%x\n", MCUSYS_PROTECTEN_SET, mmio_read_32(MCUSYS_PROTECTEN_SET));
		/* TINFO="Wait ADB pwrdnreqn for Cluster 0" */
		while ((mmio_read_32(MCUSYS_PROTECTEN_STA1) & mpx_mask) != mpx_mask);
		PRINTF_SPMC("MCUSYS_PROTECTEN_STA1_0x%x=0x%x\n", MCUSYS_PROTECTEN_STA1, mmio_read_32(MCUSYS_PROTECTEN_STA1));
/* ################ BUS PROTECT ENABLE and SNOOP/DVM DISABLE ################ */

		if (mode == MODE_AUTO_SHUT_OFF) {
			/* Cluster<n>Core0 set cluster<n>.SPMC.MP<n>.sw_spark_en (mcucfg_reg) 1 */
			/*mmio_write_32(MPx_CPUTOP_SPMC, mmio_read_32(MPx_CPUTOP_SPMC) | sw_spark_en);*/
			/* Cluster<n>Core0 set cluster<n>.SPMC.MP<n>.sw sw_no_wait_for_q_channel (mcucfg_reg) 0 */
			mmio_write_32(MPx_CPUTOP_SPMC,
				      mmio_read_32(MPx_CPUTOP_SPMC) & ~sw_no_wait_for_q_channel);
			/* Cluster<n>Core0 set cluster<n>.SPMC.MP<n>.sw_coq_dis (mcucfg_reg) 0 */
			if (cputop_mpx == CPUTOP_MP2)
				mmio_write_32(PTP3_B_COQ, mmio_read_32(PTP3_B_COQ) & ~B_SW_COQ_DIS);
			else
			mmio_write_32(MPx_CPUTOP_SPMC, mmio_read_32(MPx_CPUTOP_SPMC) & ~sw_coq_dis);
		} else {	/* MODE_SPMC_HW or MODE_DORMANT */
			/* Set  cluster<n>.SPMC.MP<n>.sw_no_wait_for_q_channel (mcucfg_reg) 1 */
			mmio_write_32(MPx_CPUTOP_SPMC,
				      mmio_read_32(MPx_CPUTOP_SPMC) | sw_no_wait_for_q_channel);
			if (cputop_mpx == CPUTOP_MP2)
				mmio_write_32(PTP3_B_COQ, mmio_read_32(PTP3_B_COQ) | B_SW_COQ_DIS);
			else
				mmio_write_32(MPx_CPUTOP_SPMC, mmio_read_32(MPx_CPUTOP_SPMC) | sw_coq_dis);
		}
		PRINTF_SPMC("MPx_CPUTOP_SPMC_0x%x=0x%x\n", MPx_CPUTOP_SPMC, mmio_read_32(MPx_CPUTOP_SPMC));
/* Set mp<n>_spmc_resetpwron_config_cputop to 0 */
/*		mmio_write_32(MPx_CPUTOP_PWR_CON, mmio_read_32(MPx_CPUTOP_PWR_CON) & ~PWR_ON_2ND);*/

		if (mode == MODE_DORMANT) {	/*  Set mp<n>_spmc_sram_dormant_en 0 */
			mmio_write_32(SPMC_DORMANT_ENABLE,
				      mmio_read_32(SPMC_DORMANT_ENABLE) | (MP0_SPMC_SRAM_DORMANT_EN
									   << cputop_mpx));
		} else {	/* MODE_SPMC_HW or MODE_AUTO_SHUT_OFF *//*     Set mp<n>_spmc_sram_dormant_en 0 */
			mmio_write_32(SPMC_DORMANT_ENABLE,
				      mmio_read_32(SPMC_DORMANT_ENABLE) & ~(MP0_SPMC_SRAM_DORMANT_EN
									    << cputop_mpx));
		}
		PRINTF_SPMC("MPx_CPUTOP_SPMC_0x%x=0x%x\n", MPx_CPUTOP_SPMC, mmio_read_32(MPx_CPUTOP_SPMC));
/*	Set  mp<n>_spmc_pwr_on_cputop 0 */
		mmio_write_32(MPx_CPUTOP_PWR_CON, mmio_read_32(MPx_CPUTOP_PWR_CON) & ~PWR_ON);
		if (mode == MODE_SPMC_HW) {	/* TINFO="Wait until PWR_STATUS = 0" */
			while (mmio_read_32(PWR_STATUS) & MPx_CPUTOP_PWR_STA_MASK);
		}
#if AO_REG_PATCH
		if (cputop_mpx == CPUTOP_MP2) {
			PRINTF_SPMC("Before : CPUSYS2_PWR_RST_CTL_0x%x=0x%x\n", CPUSYS2_PWR_RST_CTL, mmio_read_32(CPUSYS2_PWR_RST_CTL));
			mmio_write_32(CPUSYS2_PWR_RST_CTL, mmio_read_32(CPUSYS2_PWR_RST_CTL) | ((0x1)<<1));
			PRINTF_SPMC("After : CPUSYS2_PWR_RST_CTL_0x%x=0x%x\n", CPUSYS2_PWR_RST_CTL, mmio_read_32(CPUSYS2_PWR_RST_CTL));
		}
#endif
		PRINTF_SPMC("MPx_CPUTOP_SPMC_0x%x=0x%x\n", MPx_CPUTOP_SPMC, mmio_read_32(MPx_CPUTOP_SPMC));
		mmio_write_32(CPU_EXT_BUCK_ISO, mmio_read_32(CPU_EXT_BUCK_ISO) | (0x1 << cputop_mpx));
		PRINTF_SPMC("CPU_EXT_BUCK_ISO_0x%x=0x%x\n", CPU_EXT_BUCK_ISO, mmio_read_32(CPU_EXT_BUCK_ISO));
		/* TINFO="Finish to turn off MP0_CPUTOP" */
	} else {
		/* STA_POWER_ON */
		/* Set {topaon_apb_mask,sw_rst_b}=01 *//*Set {topaon_apb_mask,sw_rst_b}=10 */
#if 0
		mmio_write_32(0x10202008, (mmio_read_32(0x10202008) & ~(0x3)) | 0x2);
		/* mp2_vproc_ext_off=0(DUAL_VCORE_VCA15MPWR_ISO_AON=0) */
		mmio_write_32(0x108c0260, mmio_read_32(0x108c0260) & ~(0x1 << 2));
		udelay(100);
		mmio_write_32(0x10202008, (mmio_read_32(0x10202008) & ~(0x3)) | 0x1);	/* Set {topaon_apb_mask,sw_rst_b}=01 */
#endif
		/* TINFO="Start to turn on MP0_CPUTOP" */

#if AO_REG_PATCH
		if (cputop_mpx == CPUTOP_MP2) {
			PRINTF_SPMC("[Before]W/O BUCK_EXT_ISO : CPUSYS2_PWR_RST_CTL_0x%x=0x%x\n", CPUSYS2_PWR_RST_CTL, mmio_read_32(CPUSYS2_PWR_RST_CTL));
			mmio_write_32(CPUSYS2_PWR_RST_CTL, mmio_read_32(CPUSYS2_PWR_RST_CTL) & (~(0x1)));
			PRINTF_SPMC("[After]W/O BUCK_EXT_ISO : CPUSYS2_PWR_RST_CTL_0x%x=0x%x\n", CPUSYS2_PWR_RST_CTL, mmio_read_32(CPUSYS2_PWR_RST_CTL));
		}
#endif
		mmio_write_32(CPU_EXT_BUCK_ISO, mmio_read_32(CPU_EXT_BUCK_ISO) & ~(0x1 << cputop_mpx));/* mp2_vproc_ext_off=0(DUAL_VCORE_VCA15MPWR_ISO_AON=0)*/
		PRINTF_SPMC("CPU_EXT_BUCK_ISO_0x%x=0x%x\n", CPU_EXT_BUCK_ISO, mmio_read_32(CPU_EXT_BUCK_ISO));
#if AO_REG_PATCH
		if (cputop_mpx == CPUTOP_MP2) {
			PRINTF_SPMC("Before : CPUSYS2_PWR_RST_CTL_0x%x=0x%x\n", CPUSYS2_PWR_RST_CTL, mmio_read_32(CPUSYS2_PWR_RST_CTL));
			mmio_write_32(CPUSYS2_PWR_RST_CTL, mmio_read_32(CPUSYS2_PWR_RST_CTL) | (0x1));
			mmio_write_32(CPUSYS2_PWR_RST_CTL, mmio_read_32(CPUSYS2_PWR_RST_CTL) & (~(0x1 << 1)));
			PRINTF_SPMC("After : CPUSYS2_PWR_RST_CTL_0x%x=0x%x\n", CPUSYS2_PWR_RST_CTL, mmio_read_32(CPUSYS2_PWR_RST_CTL));
		}
#endif
		/* TINFO="Set PWR_ON = 1" */
		PRINTF_SPMC("PWR_STATUS_0x%x=0x%x\n", PWR_STATUS, mmio_read_32(PWR_STATUS));
		PRINTF_SPMC("MPx_CPUTOP_PWR_CON_0x%x=0x%x\n", MPx_CPUTOP_PWR_CON,mmio_read_32(MPx_CPUTOP_PWR_CON));
		mmio_write_32(MPx_CPUTOP_PWR_CON, mmio_read_32(MPx_CPUTOP_PWR_CON) | PWR_ON);
		PRINTF_SPMC("MPx_CPUTOP_PWR_CON_0x%x=0x%x\n", MPx_CPUTOP_PWR_CON,
			    mmio_read_32(MPx_CPUTOP_PWR_CON));
		/* TINFO="Wait until PWR_STATUS = 1" */
		while (!(mmio_read_32(PWR_STATUS) & MPx_CPUTOP_PWR_STA_MASK));
		PRINTF_SPMC("PWR_STATUS_0x%x=0x%x\n", PWR_STATUS, mmio_read_32(PWR_STATUS));
	/*	PRINTF_SPMC("PWR_STATUS_0x%x=0x%x\n", PWR_STATUS, mmio_read_32(PWR_STATUS));
		PRINTF_SPMC("0x10200590<=0x58\n");
mmio_write_32(0x10200590, 0x58);
PRINTF_SPMC("0x10200594=>0x%x\n", mmio_read_32(0x10200594));
		PRINTF_SPMC("PWR_STATUS_0x%x=0x%x\n", PWR_STATUS, mmio_read_32(PWR_STATUS));*/

/* ################ BUS PROTECT DISABLE and SNOOP/DVM ENABLE################ */
		/* TINFO="Release bus protect" */
		/* TINFO="Release ADB pwrdnreqn for Cluster 0" */
		/* mmio_write_32(INFRA_TOPAXI_PROTECTEN_1, mmio_read_32(INFRA_TOPAXI_PROTECTEN_1) & ~mpx_mask); */
		mmio_write_32(MCUSYS_PROTECTEN_CLR, mpx_mask);
		PRINTF_SPMC("MCUSYS_PROTECTEN_CLR_0x%x=0x%x\n",
			    MCUSYS_PROTECTEN_CLR,
			    mmio_read_32(MCUSYS_PROTECTEN_CLR));

		/* TINFO="Wait ADB ~pwrdnreqn for Cluster 0" */
		while ((mmio_read_32(MCUSYS_PROTECTEN_STA1) & mpx_mask) != 0);
		PRINTF_SPMC("MCUSYS_PROTECTEN_STA1_0x%x=0x%x\n", MCUSYS_PROTECTEN_STA1,
			    mmio_read_32(MCUSYS_PROTECTEN_STA1));

#if SPMC_DVT			/* by callee in plat_affinst_off() */
/*	Program MP<n>_AXI_CONFIG acinactm to 0*/
		if (cputop_mpx == CPUTOP_MP2)
			mmio_write_32(MPx_AXI_CONFIG,
				      (mmio_read_32(MPx_AXI_CONFIG) & ~MP2_AXI_CONFIG_acinactm)& ~MP2_AXI_CONFIG_ainacts);
		else
			mmio_write_32(MPx_AXI_CONFIG,
				      (mmio_read_32(MPx_AXI_CONFIG) & ~MPx_AXI_CONFIG_acinactm)& ~MPx_AXI_CONFIG_ainacts);
		PRINTF_SPMC("MPx_AXI_CONFIG_0x%x=0x%x\n", MPx_AXI_CONFIG,
			    mmio_read_32(MPx_AXI_CONFIG));
/*	Program MCSI-A Slave Control Register (MP0:slave0,  MP1:slave1) */
/*to enable snoop/DVM function for related slave port through ACE(0x10390000) or APB(0x10200000) interface*/
/*	Polling CCI Status register through ACE interface (0x1039000C[0]) returns to 0*/
/*		mmio_write_32(MPx_SNOOP_CTRL, mmio_read_32(MPx_SNOOP_CTRL) | MPx_SNOOP_ENABLE);*/
/*	  PRINTF_SPMC("MPx_SNOOP_CTRL_0x%x=0x%x\n", MPx_SNOOP_CTRL, mmio_read_32(MPx_SNOOP_CTRL));*/
		/*mmio_write_32(0x10394000, mmio_read_32(0x10394000) | MPx_SNOOP_ENABLE);
		   mmio_write_32(0x10395000, mmio_read_32(0x10395000) | MPx_SNOOP_ENABLE); */
		mmio_write_32(MPx_SNOOP_CTRL, mmio_read_32(MPx_SNOOP_CTRL) | MPx_SNOOP_ENABLE);
		PRINTF_SPMC("MPx_SNOOP_CTRL%x=0x%x\n", MPx_SNOOP_CTRL,
			    mmio_read_32(MPx_SNOOP_CTRL));
		while ((mmio_read_32(MPx_SNOOP_STATUS) & (1 << 31)) == (1 << 31));
		/* mcsib_sw_workaround_main(); */
		PRINTF_SPMC("MPx_SNOOP_STATUS_0x%x=0x%x\n", MPx_SNOOP_STATUS,
			    mmio_read_32(MPx_SNOOP_STATUS));
#endif
/* ################ BUS PROTECT DISABLE and SNOOP/DVM ENABLE################ */

		/* TINFO="Finish to turn on MP0_CPUTOP" */

	}
	return err;
}

int spmc_cpu_corex_onoff(int linear_id, int state, int mode)
{
	int err = 0;
	unsigned int CPUSYSx_CPUx_SPMC_CTL, MPx_CPUx_PWR_CON, MPx_CPUx_STANDBYWFI,MPx_CPUx_PWR_STA_MASK;
	unsigned int MPx_CA7_MISC_CONFIG = 0, MPx_CPUx_STANDBYWFE = 0;

	/* TINFO="enable SPM register control" */
	mmio_write_32(POWERON_CONFIG_EN, (SPM_PROJECT_CODE << 16) | (0x1 << 0));
	PRINTF_SPMC(">>>>>>>> %s >>>>>>>>linear_id=%d state=%d mode=%d\n", __func__, linear_id,
		    state, mode);
	switch (linear_id) {
	case 0:
		CPUSYSx_CPUx_SPMC_CTL = CPUSYS0_CPU0_SPMC_CTL;
		MPx_CPUx_STANDBYWFI = MP0_CPU0_STANDBYWFI;
		MPx_CPUx_PWR_CON = MP0_CPU0_PWR_CON;
		MPx_CPUx_PWR_STA_MASK = MP0_CPU0_PWR_STA_MASK;
		MPx_CA7_MISC_CONFIG = MP0_CA7_MISC_CONFIG;
		MPx_CPUx_STANDBYWFE = MP0_CPU0_STANDBYWFE;
		break;
	case 1:
		CPUSYSx_CPUx_SPMC_CTL = CPUSYS0_CPU1_SPMC_CTL;
		MPx_CPUx_STANDBYWFI = MP0_CPU1_STANDBYWFI;
		MPx_CPUx_PWR_CON = MP0_CPU1_PWR_CON;
		MPx_CPUx_PWR_STA_MASK = MP0_CPU1_PWR_STA_MASK;
		MPx_CA7_MISC_CONFIG = MP0_CA7_MISC_CONFIG;
		MPx_CPUx_STANDBYWFE = MP0_CPU1_STANDBYWFE;
		break;
	case 2:
		CPUSYSx_CPUx_SPMC_CTL = CPUSYS0_CPU2_SPMC_CTL;
		MPx_CPUx_STANDBYWFI = MP0_CPU2_STANDBYWFI;
		MPx_CPUx_PWR_CON = MP0_CPU2_PWR_CON;
		MPx_CPUx_PWR_STA_MASK = MP0_CPU2_PWR_STA_MASK;
		MPx_CA7_MISC_CONFIG = MP0_CA7_MISC_CONFIG;
		MPx_CPUx_STANDBYWFE = MP0_CPU2_STANDBYWFE;
		break;
	case 3:
		CPUSYSx_CPUx_SPMC_CTL = CPUSYS0_CPU3_SPMC_CTL;
		MPx_CPUx_STANDBYWFI = MP0_CPU3_STANDBYWFI;
		MPx_CPUx_PWR_CON = MP0_CPU3_PWR_CON;
		MPx_CPUx_PWR_STA_MASK = MP0_CPU3_PWR_STA_MASK;
		MPx_CA7_MISC_CONFIG = MP0_CA7_MISC_CONFIG;
		MPx_CPUx_STANDBYWFE = MP0_CPU3_STANDBYWFE;
		break;
	case 4:
		CPUSYSx_CPUx_SPMC_CTL = CPUSYS1_CPU0_SPMC_CTL;
		MPx_CPUx_STANDBYWFI = MP1_CPU0_STANDBYWFI;
		MPx_CPUx_PWR_CON = MP1_CPU0_PWR_CON;
		MPx_CPUx_PWR_STA_MASK = MP1_CPU0_PWR_STA_MASK;
		MPx_CA7_MISC_CONFIG = MP1_CA7_MISC_CONFIG;
		MPx_CPUx_STANDBYWFE = MP1_CPU0_STANDBYWFE;
		break;
	case 5:
		CPUSYSx_CPUx_SPMC_CTL = CPUSYS1_CPU1_SPMC_CTL;
		MPx_CPUx_STANDBYWFI = MP1_CPU1_STANDBYWFI;
		MPx_CPUx_PWR_CON = MP1_CPU1_PWR_CON;
		MPx_CPUx_PWR_STA_MASK = MP1_CPU1_PWR_STA_MASK;
		MPx_CA7_MISC_CONFIG = MP1_CA7_MISC_CONFIG;
		MPx_CPUx_STANDBYWFE = MP1_CPU1_STANDBYWFE;
		break;
	case 6:
		CPUSYSx_CPUx_SPMC_CTL = CPUSYS1_CPU2_SPMC_CTL;
		MPx_CPUx_STANDBYWFI = MP1_CPU2_STANDBYWFI;
		MPx_CPUx_PWR_CON = MP1_CPU2_PWR_CON;
		MPx_CPUx_PWR_STA_MASK = MP1_CPU2_PWR_STA_MASK;
		MPx_CA7_MISC_CONFIG = MP1_CA7_MISC_CONFIG;
		MPx_CPUx_STANDBYWFE = MP1_CPU2_STANDBYWFE;
		break;
	case 7:
		CPUSYSx_CPUx_SPMC_CTL = CPUSYS1_CPU3_SPMC_CTL;
		MPx_CPUx_STANDBYWFI = MP1_CPU3_STANDBYWFI;
		MPx_CPUx_PWR_CON = MP1_CPU3_PWR_CON;
		MPx_CPUx_PWR_STA_MASK = MP1_CPU3_PWR_STA_MASK;
		MPx_CA7_MISC_CONFIG = MP1_CA7_MISC_CONFIG;
		MPx_CPUx_STANDBYWFE = MP1_CPU3_STANDBYWFE;
		break;
	case 8:
		CPUSYSx_CPUx_SPMC_CTL = CPUSYS2_CPU0_SPMC_CTL;
		MPx_CPUx_STANDBYWFI = MP2_CPU0_STANDBYWFI;
		MPx_CPUx_PWR_CON = MP2_CPU0_PWR_CON;
		MPx_CPUx_PWR_STA_MASK = MP2_CPU0_PWR_STA_MASK;
		MPx_CA7_MISC_CONFIG = MP2_CA15M_MON_L;
		MPx_CPUx_STANDBYWFE = MP2_CPU0_STANDBYWFE;
		break;
	case 9:
		CPUSYSx_CPUx_SPMC_CTL = CPUSYS2_CPU1_SPMC_CTL;
		MPx_CPUx_STANDBYWFI = MP2_CPU1_STANDBYWFI;
		MPx_CPUx_PWR_CON = MP2_CPU1_PWR_CON;
		MPx_CPUx_PWR_STA_MASK = MP2_CPU1_PWR_STA_MASK;
		MPx_CA7_MISC_CONFIG = MP2_CA15M_MON_L;
		MPx_CPUx_STANDBYWFE = MP2_CPU1_STANDBYWFE;
		break;

	default:
		ERROR("[ATF]: %s() CPU%d not exists\n", __func__, (int)linear_id);
		assert(0);
	}

	PRINTF_SPMC("MPx_CA7_MISC_CONFIG=0x%x MPx_CPUx_STANDBYWFE=0x%x\n", MPx_CA7_MISC_CONFIG,
		    MPx_CPUx_STANDBYWFE);
	PRINTF_SPMC("MPx_CPUx_STANDBYWFI=0x%x\n", MPx_CPUx_STANDBYWFI);

	if (state == STA_POWER_DOWN) {
		/* TINFO="Start to turn off MP0_CPU0" */
		if (!(cpu_bitmask & (1 << linear_id))) {
			PRINTF_SPMC("core%d already turn off !!! cpu_bitmask=0x%x\n", linear_id,cpu_bitmask);
			return 0;
		}
		if(linear_id>=8)
			mmio_write_32(DBUGPWRDUP, mmio_read_32(DBUGPWRDUP) & ~(0x1<<(linear_id-8)));

		if (mode == MODE_AUTO_SHUT_OFF) {
			mmio_write_32(CPUSYSx_CPUx_SPMC_CTL,
				      mmio_read_32(CPUSYSx_CPUx_SPMC_CTL) &
				      ~cpu_sw_no_wait_for_q_channel);
			PRINTF_SPMC("CPUSYSx_CPUx_SPMC_CTL_0x%x=0x%x\n", CPUSYSx_CPUx_SPMC_CTL,
				    mmio_read_32(CPUSYSx_CPUx_SPMC_CTL));
/*			mmio_write_32(CPUSYSx_CPUx_SPMC_CTL,
				      mmio_read_32(CPUSYSx_CPUx_SPMC_CTL) | cpu_sw_spark_en);
			PRINTF_SPMC("CPUSYSx_CPUx_SPMC_CTL_0x%x=0x%x\n", CPUSYSx_CPUx_SPMC_CTL,
				    mmio_read_32(CPUSYSx_CPUx_SPMC_CTL)); */
			set_cpu_retention_control(1);
		} else {
			mmio_write_32(CPUSYSx_CPUx_SPMC_CTL,
				      mmio_read_32(CPUSYSx_CPUx_SPMC_CTL) |
				      cpu_sw_no_wait_for_q_channel);
		PRINTF_SPMC("MPx_CPUx_PWR_CON_0x%x=0x%x\n", MPx_CPUx_PWR_CON,
				mmio_read_32(MPx_CPUx_PWR_CON));
#if SPMC_DVT			/* wait WFE */
		if(linear_id>=8){
			mmio_write_32(CA15M_MON_SEL, (mmio_read_32(CA15M_MON_SEL) & 0xFF) | 0x1b);
			while (!(mmio_read_32(MPx_CA7_MISC_CONFIG) & MPx_CPUx_STANDBYWFE));
			}
		else{
			while(!(mmio_read_32(MPx_CA7_MISC_CONFIG) & MPx_CPUx_STANDBYWFE));
			}
			PRINTF_SPMC("MPx_CA7_MISC_CONFIG%x=0x%x\n",MPx_CA7_MISC_CONFIG,mmio_read_32(MPx_CA7_MISC_CONFIG));
#else				/* wait WFI */
			PRINTF_SPMC("CPU_IDLE_STA_0x%x=0x%x\n", CPU_IDLE_STA,
				    mmio_read_32(CPU_IDLE_STA));
			while (!(mmio_read_32(CPU_IDLE_STA) & MPx_CPUx_STANDBYWFI));
#endif
		}

/*		mmio_write_32(MPx_CPUx_PWR_CON, mmio_read_32(MPx_CPUx_PWR_CON) & ~PWR_ON_2ND);
		PRINTF_SPMC("MPx_CPUx_PWR_CON_0x%x=0x%x\n", MPx_CPUx_PWR_CON,
			    mmio_read_32(MPx_CPUx_PWR_CON));*/
		/* TINFO="Set PWR_ON = 0" */
		mmio_write_32(MPx_CPUx_PWR_CON, mmio_read_32(MPx_CPUx_PWR_CON) & ~PWR_ON);
		PRINTF_SPMC("MPx_CPUx_PWR_CON_0x%x=0x%x\n", MPx_CPUx_PWR_CON,
			    mmio_read_32(MPx_CPUx_PWR_CON));
		/* TINFO="Wait until CPU_PWR_STATUS = 0 */
		if (mode == MODE_SPMC_HW) {
			while (mmio_read_32(PWR_STATUS) & MPx_CPUx_PWR_STA_MASK);
			PRINTF_SPMC("PWR_STATUS_0x%x=0x%x\n", PWR_STATUS, mmio_read_32(PWR_STATUS));
		}
		cpu_bitmask &= ~(1 << linear_id);
		PRINTF_SPMC("cpu_bitmask=0x%x\n", cpu_bitmask);
		/* TINFO="Finish to turn off MP0_CPU0" */
	} else {
/*		if(linear_id>=8)
			mmio_write_32(DBUGPWRDUP, mmio_read_32(DBUGPWRDUP) | (0x1<<(linear_id-8)));*/

		/* TINFO="Start to turn on MP0_CPU0" */
		/* TINFO="Set PWR_ON = 1" */
		PRINTF_SPMC("PWR_STATUS_0x%x=0x%x\n", PWR_STATUS, mmio_read_32(PWR_STATUS));
		/*mmio_write_32(MPx_CPUx_PWR_CON,  mmio_read_32(MPx_CPUx_PWR_CON) | PWR_RST_B); */
		PRINTF_SPMC("MPx_CPUx_PWR_CON_0x%x=0x%x\n", MPx_CPUx_PWR_CON, mmio_read_32(MPx_CPUx_PWR_CON));
		mmio_write_32(MPx_CPUx_PWR_CON, mmio_read_32(MPx_CPUx_PWR_CON) | PWR_ON);
		PRINTF_SPMC("MPx_CPUx_PWR_CON_0x%x=0x%x\n", MPx_CPUx_PWR_CON, mmio_read_32(MPx_CPUx_PWR_CON));
		/* TINFO="Wait until CPU_PWR_STATUS = 1 and CPU_PWR_STATUS_2ND = 1" */
//		while ((mmio_read_32(PWR_STATUS) & MPx_CPUx_PWR_STA_MASK) != MPx_CPUx_PWR_STA_MASK);
/*		mmio_write_32(CPUSYSx_CPUx_SPMC_CTL, mmio_read_32(CPUSYSx_CPUx_SPMC_CTL)| cpu_sw_spark_en);*/
		PRINTF_SPMC("PWR_STATUS_0x%x=0x%x\n", PWR_STATUS, mmio_read_32(PWR_STATUS));
		cpu_bitmask |= (1 << linear_id);
		PRINTF_SPMC("cpu_bitmask=0x%x\n", cpu_bitmask);
#if SPMC_SPARK2
		if (linear_id >= 4) {
			little_spark2_setldo(linear_id);
			little_spark2_core_enable(linear_id, 1);
		}
#if SPMC_DVT
		/*
		AGPIO-BIG	 PAD_EINT7	      0x11F20610[19](DRV1_TR[19]): 1-Analog mode, 0-digtial mode (default)
		AGPIO-Little PAD_DISP_PWM	  0x11C10610[23](DRV1_RB[23]): 1-Analog mode, 0-digtial mode (default)
		AGPIO-LL	 PAD_VOW_CLK_MISO 0x11C10610[19](DRV1_RB[19]): 1-Analog mode, 0-digtial mode (default)
		*/
		mmio_write_32(0x11F20610, mmio_read_32(0x11F20610) | 0x80000);
		PRINTF_SPMC("[%s] 0x11F20610=0x%x\n", __func__, mmio_read_32(0x11F20610));

		/* 3. Read LkgMon BinCountPre #1 */
		/* TODO: let core into wfe */
		/* spark2_setldo_AMUXSEL(cputop_mpx,cpu_corex,0,0x3f);//AMUXSEL=0 */
		//spark2_setldo_AMUXSEL(CPUTOP_MP2, 0, 2, 0x34);	/* AMUXSEL=0 */
		spark2_setldo_AMUXSEL(CPUTOP_MP2, 1, 2, 0x34);	/* AMUXSEL=0 */
#endif
#endif
		/* TINFO="Finish to turn on MP0_CPU0" */
		PRINTF_SPMC("[AT] PowerOn CPU %d successfully\n",linear_id);
	}
	return err;
}

#if SPMC_SW_MODE

int setup_sw_cluster_default_state(int select, int state)
{
	unsigned int addr_spmc, tmp, i;

	PRINTF_SPMC("%s select:%x state=%d\n", __func__, select, state);
	/* dr_spmc = select_spmc_base(select, &nb_srampd); */
	switch (select) {
	case 0x10:		/* LL cluster */
		addr_spmc = MP0_CPUTOP_SPMC_CTL;
		break;
	case 0x20:		/* L cluster */
		addr_spmc = MP1_CPUTOP_SPMC_CTL;
		break;
	default:
		ERROR("[ATF]: %s() CPU%d not exists\n", __func__, (int)select);
		assert(0);
	}
	if (state == STA_POWER_DOWN) {
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) | 0x8000);	/* ckiso=1 */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) | 0x40);	/* sw_iso=1 */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) & 0xffffdfff);	/* sw_sram_isointb=0 */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) | 0x400000);	/* sw_hot_plug_reset=1 */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) | 0x4000);	/* clk_dis=1 */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) & 0xfffffff7);	/* pre1_pdb=0 */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) & 0xffffffdf);	/* sw_logic_pdb[5]=0 */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) & 0xffffe07f);	/* sleepb=0 */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) & 0xffc0ffff);	/* sw_pd=5'h0 */
	} else {		/* POWER ON */
		for (i = 0; i < 0x36; i++) {
			tmp = (i >> 1) ^ i;	/*binaryToGray */
			/* PRINTF_SPMC("binaryToGray[%d]=0x%x\n",i,tmp); */
			/* sw_pd=5'h0=1 */
			mmio_write_32(addr_spmc, (mmio_read_32(addr_spmc) & 0xffc0ffff) | tmp << 16);
		}
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) | 0x8);	/* pre1_pdb */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) | 0x10);	/* pre2_pdb */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) | 0x20);	/* sw_logic_pdb[5]=1 */
		/* sw_pwr_on=1 sw_pwr_on_override_en=1 */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) | 0x1800000);
		for (i = 0; i < 0x36; i++) {
			tmp = (i >> 1) ^ i;	/* binaryToGray */
			/* PRINTF_SPMC("binaryToGray[%d]=0x%x\n",i,tmp); */
			mmio_write_32(addr_spmc, (mmio_read_32(addr_spmc) & 0xffffe07f) | tmp << 7);	/* sleepb=1 */
		}
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) & 0xffffffbf);	/* sw_iso=0 */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) | 0x2000);	/* sw_sram_isointb=1 */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) & 0xffff7fff);	/* ckiso=0 */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) & 0xffffbfff);	/* clk_dis=0 */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) & 0xffbfffff);	/* sw_hot_plug_reset=0 */
	}
	return 0;
}

int setup_sw_core_default_state(int select, int state)
{
	unsigned int addr_spmc, tmp, i;

	PRINTF_SPMC("%s select:%x state=%d\n", __func__, select, state);
	/* dr_spmc = select_spmc_base(select, &nb_srampd); */
	switch (select) {
	case 0:
		addr_spmc = CPUSYS0_CPU0_SPMC_CTL;
		break;
	case 1:
		addr_spmc = CPUSYS0_CPU1_SPMC_CTL;
		break;
	case 2:
		addr_spmc = CPUSYS0_CPU2_SPMC_CTL;
		break;
	case 3:
		addr_spmc = CPUSYS0_CPU3_SPMC_CTL;
		break;
	case 4:
		addr_spmc = CPUSYS1_CPU0_SPMC_CTL;
		break;
	case 5:
		addr_spmc = CPUSYS1_CPU1_SPMC_CTL;
		break;
	case 6:
		addr_spmc = CPUSYS1_CPU2_SPMC_CTL;
		break;
	case 7:
		addr_spmc = CPUSYS1_CPU3_SPMC_CTL;
		break;
	case 0x10:		/* LL cluster */
		addr_spmc = MP0_CPUTOP_SPMC_CTL;
		break;
	case 0x20:		/* L cluster */
		addr_spmc = MP1_CPUTOP_SPMC_CTL;
		break;
	default:
		ERROR("[ATF]: %s() CPU%d not exists\n", __func__, (int)select);
		assert(0);
	}
	if (state == STA_POWER_DOWN) {
		PRINTF_SPMC("[%s] [cluster%d]0x%x Reg:0x%x\n", __func__, select, addr_spmc,
			    mmio_read_32(addr_spmc));
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) | 0x400);	/* cpu_sw_ckiso=1 */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) | 0x40);	/* cpu_sw_iso=1 */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) & 0xfffffeff);	/* cpu_sw_sram_isointb=0 */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) | 0x200);	/* cpu_sw_clk_dis=0 */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) & 0xfffffff7);	/* cpu_sw_logic_pre1_pdb=0 [3] */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) & 0xffffffef);	/* cpu_sw__logic_pre2_pdb=0 [4] */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) & 0xffffffdf);	/* cpu_sw_logic_pdb=0 [5] */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) & 0xffffff7f);	/* cpu_sw_sram_sleepb=0 */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) & 0xffff07ff);	/* cpu_sw_pd[15:11]=5'h0 */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) | 0x10000);	/* cpu_sw_hot_plug_reset=1 */
		PRINTF_SPMC("[%s] [cluster%d]0x%x Reg:0x%x\n", __func__, select, addr_spmc,
			    mmio_read_32(addr_spmc));
	} else {		/* POWER ON */
		PRINTF_SPMC("[%s] [core%d]0x%x Reg:0x%x\n", __func__, select, addr_spmc,
			    mmio_read_32(addr_spmc));
		for (i = 0; i < 26; i++) {
			tmp = (i >> 1) ^ i;	/* binaryToGray */
			/* PRINTF_SPMC("binaryToGray[%d]=0x%x\n",i,tmp); */
			 /* cpu_sw_pd[15:11]=1 */
			mmio_write_32(addr_spmc, (mmio_read_32(addr_spmc) & 0xffff07ff) | tmp << 11);
		}
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) | 0x8);	/* cpu_sw_lofic_pre1_pdb=1 */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) | 0x10);	/* cpu_sw_lofic_pre2_pdb=1 */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) | 0x20);	/* cpu_sw_logic_pdb=1 */
		/* cpu_sw_powr_on_override_en =1 cpu_sw_pwr_on=1 */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) | 0x60000);

		/* mmio_write_32(addr_spmc,mmio_read_32(addr_spmc) | cpu_sw_sram_sleepb);//cpu_sw_sram_sleepb */
		/* mmio_write_32(addr_spmc,mmio_read_32(addr_spmc) & ~cpu_sw_iso);//cpu_sw_iso */
		/* mmio_write_32(addr_spmc,mmio_read_32(addr_spmc) | cpu_sw_sram_isointb); */
		/* mmio_write_32(addr_spmc,mmio_read_32(addr_spmc) & ~cpu_sw_ckiso); */
		/* mmio_write_32(addr_spmc,mmio_read_32(addr_spmc) & ~cpu_sw_clk_dis); */
		/* mmio_write_32(addr_spmc,mmio_read_32(addr_spmc) & ~cpu_sw_hot_plug_reset); */
		PRINTF_SPMC("[%s] [core%d]0x%x Reg:0x%x\n", __func__, select, addr_spmc,
			    mmio_read_32(addr_spmc));

	}
	return 0;
}



/*
static unsigned int select_spmc_base(int select,  unsigned int *nb_srampd)
{
	unsigned int addr_spmc;

	switch(select)
	{
	case 0x1:
			addr_spmc = CPUSYS0_CPU0_SPMC_CTL;
			if (nb_srampd)
				*nb_srampd = 16;
			break;
	case 0x2:
			addr_spmc = CPUSYS0_CPU1_SPMC_CTL;
			if (nb_srampd)
				*nb_srampd = 16;
			break;
	case 0x10:
			addr_spmc = PTP3_CPUTOP_SPMC;
			if (nb_srampd)
				*nb_srampd = 32;
			break;
	default:
			PRINTF_SPMC("Should be not here\n");
			assert(0);
	}
	return addr_spmc;
}
*/
/**
 * Before enable the SW sequenwce,  all of the big cores must be turn offset.
 * SO the function cannot be called on a big core
 * HW-API:BigSPMCSwPwrSeqEn
 * select: 0x1:core0 0x2:core1 0x4:all 0x10:top
 * state : current state STA_POWER_DOWN or STA_POWER_ON
 */
/*
   1. if Select invalid,  exit/return error -1
	For Select
	2. Read  sw_fsm_override  & fsm_state_out
	3. if sw_fsm_override=0 & fsm_state_out=0
	1. Set all CPU SW state to OFF (no sequencing required)
	- sram_pd = 5'h00 for CPU & 6'h00 for TOP
	- sw_logic_*_pdb = 0
	- sw_sram_sleepb = 0
	- sw_iso = 1,  sw_sram_isointb = 0
	- sw_hot_plug_reset = 1
	2. Write all cpu sw_fsm_override = 1
	3. Poll/wait for all cpu fsm_state_out = 1
	3. else return error -2 or -3
	"0: Ok
	-1: Invalid parameter
	-2: sw_fsm_override=1
	-3: fsm_state_out=1
	-4: Timeout occurred"

*/

int little_spmc_sw_pwr_seq_en(int select, int state)
{

	unsigned int tmp, result, retry;
	unsigned int addr_spmc;	/* = select_spmc_base(select,  0); */

	/* TINFO="enable SPM register control" */
	mmio_write_32(POWERON_CONFIG_EN, (SPM_PROJECT_CODE << 16) | (0x1 << 0));

	switch (select) {
	case 0:
		addr_spmc = CPUSYS0_CPU0_SPMC_CTL;
		break;
	case 1:
		addr_spmc = CPUSYS0_CPU1_SPMC_CTL;
		break;
	case 2:
		addr_spmc = CPUSYS0_CPU2_SPMC_CTL;
		break;
	case 3:
		addr_spmc = CPUSYS0_CPU3_SPMC_CTL;
		break;
	case 4:
		addr_spmc = CPUSYS1_CPU0_SPMC_CTL;
		break;
	case 5:
		addr_spmc = CPUSYS1_CPU1_SPMC_CTL;
		break;
	case 6:
		addr_spmc = CPUSYS1_CPU2_SPMC_CTL;
		break;
	case 7:
		addr_spmc = CPUSYS1_CPU3_SPMC_CTL;
		break;
	case 0x10:		/*LL cluster */
		addr_spmc = MP0_CPUTOP_SPMC_CTL;
		break;
	case 0x20:		/*L cluster */
		addr_spmc = MP1_CPUTOP_SPMC_CTL;
		break;
	default:
		ERROR("[ATF]: %s() CPU%d not exists\n", __func__, (int)select);
		assert(0);
	}

	PRINTF_SPMC("[%s] Sel:0x%x Reg:0x%x\n", __func__, select, addr_spmc);	/*big sw power sequence error FSM */
	/* TODO: no register */
	/*if ((mmio_read_32(addr_spmc) & FSM_STATE_OUT_MASK) == FSM_ON) */
	{
		/*FSM_out is not zero */
		/*PRINTF_SPMC("SwPsqE ESt1\n");//big sw power sequence error FSM */
		/*return -3; //-2: fsm_state_out=1 */

	}
	if ((mmio_read_32(addr_spmc) & sw_fsm_override)) {
		/*FSM_out is not zero */
		PRINTF_SPMC("SwPsqE EFO1\n");
		return -2; /*-1: sw_fsm_override=1*/
	}


	if (select == 0x10 || select == 0x20) {
#if 0
/*??????????????????????????????????????????????????????????????????????????????????*/
/*This part was moved to kernel */
/*Enable buck first*/
		tmp = mmio_read_32(SPM_MP2_CPUSYS_PWR_CON) | (1 << 0);
		mmio_write_32(SPM_MP2_CPUSYS_PWR_CON, tmp);	/*Release pwr_rst_b */

		tmp = mmio_read_32(WDT_SWSYSRST) | 0x88000800;
		mmio_write_32(WDT_SWSYSRST, tmp);

		tmp = mmio_read_32(SPM_CPU_EXT_BUCK_ISO) & ~(0x3);
		mmio_write_32(SPM_CPU_EXT_BUCK_ISO, tmp);
		/*ISOLATE*/ tmp = (mmio_read_32(WDT_SWSYSRST) & ~(0x0800)) | 0x88000000;
		mmio_write_32(WDT_SWSYSRST, tmp);
/*??????????????????????????????????????????????????????????????????????????????????*/
#endif
		if (state == STA_POWER_DOWN) {
			tmp =
			    mmio_read_32(addr_spmc) & ~((sw_pd) | sw_logic_pdb | sw_logic_pre1_pdb |
							sw_logic_pre2_pdb | sw_sram_sleepb |
							sw_sram_isointb);
			tmp |= (sw_iso | sw_hot_plug_reset);
			mmio_write_32(addr_spmc, tmp);
			PRINTF_SPMC("MP%d_CPUTOP_SPMC_CTL_0x%x=0x%x\n", (select == 0x10 ? 0 : 1),
				    addr_spmc, mmio_read_32(addr_spmc));
		}
		tmp = mmio_read_32(addr_spmc) | sw_fsm_override;
		mmio_write_32(addr_spmc, tmp);
		PRINTF_SPMC("MP%d_CPUTOP_SPMC_CTL_0x%x=0x%x\n", (select == 0x10 ? 0 : 1), addr_spmc,
			    mmio_read_32(addr_spmc));
		result = 0;
		for (retry = 10; retry > 0; retry--) {
			/* TODO: no register */
			/*if ((mmio_read_32(addr_spmc) & FSM_STATE_OUT_MASK) == FSM_ON) */
			{
				result = 1;
				break;
			}
			udelay(RETRY_TIME_USEC);
		}
		if (!result) {
			PRINTF_SPMC("SwPsqE ETopTO\n");	/*TO=timeout */
			return -4;	/*timeout */
		}
	} else if (select >= 0x0 && select <= 0x7) {
		if (state == STA_POWER_DOWN) {
			tmp =
			    mmio_read_32(addr_spmc) & ~(cpu_sw_pd | cpu_sw_pd |
							cpu_sw_logic_pre1_pdb |
							cpu_sw_logic_pre2_pdb | cpu_sw_sram_sleepb |
							cpu_sw_sram_isointb);
			tmp |= (cpu_sw_iso | cpu_sw_hot_plug_reset);
			mmio_write_32(addr_spmc, tmp);
			PRINTF_SPMC("CPU%d_SPMC_CTL_0x%x=0x%x\n", select, addr_spmc,
				    mmio_read_32(addr_spmc));
		}
		tmp = mmio_read_32(addr_spmc) | cpu_sw_fsm_override;
		mmio_write_32(addr_spmc, tmp);
		PRINTF_SPMC("CPU%d_SPMC_CTL_0x%x=0x%x\n", select, addr_spmc,
			    mmio_read_32(addr_spmc));
/* return 0;*/
		result = 0;
		for (retry = 10; retry > 0; retry--) {
/* TODO: no register*/
			/*if ((mmio_read_32(addr_spmc) & FSM_STATE_OUT_MASK)  ==  FSM_ON) {// TODO: no register */
			result = 1;
			break;
			/*   } */
			udelay(RETRY_TIME_USEC);

		}
		if (!result) {
			PRINTF_SPMC("SwPsqE ETO:%x\n", select);
			return -4;	/*timeout */
		}
	} else
		return -1;
	udelay(RETRY_TIME_USEC);
#if 0
	if ((select & 0xF0) != 0) {	/*cluster */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) | 0x8000);	/*ckiso */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) | 0x40);	/*sw_iso */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) & 0xFFFFDFFF);	/*sw_sram_isointb */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) | 0x400000);	/*sw_hot_plug_reset=1 */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) | 0x4000);	/*clk_dis */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) & 0xFFFFFFF7);	/*pre1_pdb=0 */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) & 0xFFFFFFDF);	/*sw_logic_pdb[5]=0 */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) & 0xFFFFE07F);	/*sleepb=0 */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) & 0xFFC0FFFF);	/*sw_pd=5'h0 */
	} else {
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) | 0x400);	/*ckiso */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) | 0x40);	/*sw_iso */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) & 0xFFFFFEFF);	/*sw_sram_isointb */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) | 0x200);	/*clk_dis */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) & 0xFFFFFFF7);	/*pre1_pdb=0 [3] */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) & 0xFFFFFFEF);	/*pre2_pdb=0 [4] */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) & 0xFFFFFFDF);	/*logic_pdb=0 [5] */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) & 0xFFFFFF7F);	/*sleepb=0 */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) & 0xFFFF07FF);	/*sw_pd[15:11]=5'h0 */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) | 0x10000);	/*sw_hot_plug_reset=1 */
	}
	PRINTF_SPMC("sw_fsm_override=1\n");
	tmp = mmio_read_32(addr_spmc) | SW_FSM_OVERRIDE;
	mmio_write_32(addr_spmc, tmp);
	if ((select & 0xF0) != 0) {	/*cluster */
		mmio_write_32(addr_spmc, mmio_read_32(addr_spmc) | 0x38);	/* pre1_pdb=1 */
		while (!(mmio_read_32(addr_spmc) & SW_LOGIC_PDBO_ALL_ON_ACK)) ;	/*wait for 1 */
	}
#endif
	return 0;
}

/**
 * HW-API: BigSPMCSwPwrOn
 * selectL 0x1: core 1,  0x2:core 2,  0xF:all cores ,  0x10:TOP
 */
/*
"1. if Select invalid,  exit/return error -1
2. if Select = CPU0,  CPU1,  or ALLCPU,  read TOP SPMC Status. If not powered on,  Power  on TOP first
For each Select:
3. Read sw_fsm_override & sw_pwr_on_override_en & pwr state
4. If already power on,  continue to next Select.
5. If sw_fsm_override=0
	1. if sw_pwr_on_override=0
		 .1. Write sw_pwr_on_override_en = 1
	 2. write sw_pwr_on=1
6. else
		Sequence:
		1. sram_pd gray sequencing from 0 -> 11000
		2. sw_logic_pre1_pdb = 1
		3. Poll/wait logic_pre1_pdbo_all_on_ack = 1
		4. sw_logic_pre2_pdb = 1
		5. Poll/wait logic_pre2_pdbo_all_on_ack = 1
		6. sw_logic_pdb = 1
		7. Poll/wait logic_pdbo_all_on_ack = 1
		8. sw_sram_sleepb = 1
		9. sw_iso = 0,  sw_sram_isointb = 1
		10. sw_hot_plug_reset = 0"

*/
int little_sw_on_seq(int select)
{
	unsigned int addr_spmc, tmp, i;
	unsigned int nb_srampd, SW_LOGIC_PDBO_ALL_ON_ACK, SW_PWR_ON_OVERRIDE_EN, SW_PWR_ON,
	    SW_HOT_PLUG_RESET, /*SW_FSM_OVERRIDE,*/ SRAMPD_OFFSET, SW_PD, SPM_MPx_CPUx_PWR_CON;
	unsigned int SW_LOGIC_PRE1_PDB, SW_LOGIC_PRE1_PDBO_ALL_ON_ACK, SW_LOGIC_PRE2_PDB,
	    SW_LOGIC_PRE2_PDBO_ALL_ON_ACK, SW_LOGIC_PDB, SW_SRAM_ISOINTB, SW_CKISO, SW_CLK_DIS;
//	unsigned int MPx_SNOOP_CTRL;

	PRINTF_SPMC(">>>>>>>>>>>>[%s] select:%x\n", __func__, select);
	/*dr_spmc = select_spmc_base(select,  &nb_srampd); */
	if ((select & 0xF0) != 0) {	/*cluster */
		SW_LOGIC_PDBO_ALL_ON_ACK = logic_pdbo_all_on_ack;
//		SW_FSM_OVERRIDE = sw_fsm_override;
		SW_PWR_ON_OVERRIDE_EN = sw_pwr_on_override_en;
		SW_PWR_ON = sw_pwr_on;
		SW_HOT_PLUG_RESET = sw_hot_plug_reset;
		nb_srampd = 0x40;//0x36(ok);
		SRAMPD_OFFSET = 16;
		SW_PD = sw_pd;
		SW_LOGIC_PRE1_PDB = sw_logic_pre1_pdb;
		SW_LOGIC_PRE1_PDBO_ALL_ON_ACK = logic_pre1_pdbo_all_on_ack;
		SW_LOGIC_PRE2_PDB = sw_logic_pre2_pdb;
		SW_LOGIC_PRE2_PDBO_ALL_ON_ACK = logic_pre2_pdbo_all_on_ack;
		SW_LOGIC_PDB = sw_logic_pdb;
		SW_SRAM_ISOINTB = sw_sram_isointb;
		SW_CKISO = sw_ckiso;
		SW_CLK_DIS = sw_clk_dis;
	} else {		/*core */
		SW_LOGIC_PDBO_ALL_ON_ACK = cpu_pdbo_all_on_ack;
//		SW_FSM_OVERRIDE = cpu_sw_fsm_override;
		SW_PWR_ON_OVERRIDE_EN = cpu_sw_powr_on_override_en;
		SW_PWR_ON = cpu_sw_pwr_on;
		SW_HOT_PLUG_RESET = cpu_sw_hot_plug_reset;
		nb_srampd = 0x20;//26(ok);
		SRAMPD_OFFSET = 11;
		SW_PD = cpu_sw_pd;
		SW_LOGIC_PRE1_PDB = cpu_sw_logic_pre1_pdb;
		SW_LOGIC_PRE1_PDBO_ALL_ON_ACK = cpu_pre1_pdbo_allon_ack;
		SW_LOGIC_PRE2_PDB = cpu_sw_logic_pre2_pdb;
		SW_LOGIC_PRE2_PDBO_ALL_ON_ACK = cpu_pre2_pdbo_allon_ack;
		SW_LOGIC_PDB = cpu_sw_logic_pdb;
		SW_SRAM_ISOINTB = cpu_sw_sram_isointb;
		SW_CKISO = cpu_sw_ckiso;
		SW_CLK_DIS = cpu_sw_clk_dis;
	}

	switch (select) {
	case 0:
		addr_spmc = CPUSYS0_CPU0_SPMC_CTL;
		SPM_MPx_CPUx_PWR_CON = MP0_CPU0_PWR_CON;
		break;
	case 1:
		addr_spmc = CPUSYS0_CPU1_SPMC_CTL;
		SPM_MPx_CPUx_PWR_CON = MP0_CPU1_PWR_CON;
		break;
	case 2:
		addr_spmc = CPUSYS0_CPU2_SPMC_CTL;
		SPM_MPx_CPUx_PWR_CON = MP0_CPU2_PWR_CON;
		break;
	case 3:
		addr_spmc = CPUSYS0_CPU3_SPMC_CTL;
		SPM_MPx_CPUx_PWR_CON = MP0_CPU3_PWR_CON;
		break;
	case 4:
		addr_spmc = CPUSYS1_CPU0_SPMC_CTL;
		SPM_MPx_CPUx_PWR_CON = MP1_CPU0_PWR_CON;
		break;
	case 5:
		addr_spmc = CPUSYS1_CPU1_SPMC_CTL;
		SPM_MPx_CPUx_PWR_CON = MP1_CPU1_PWR_CON;
		break;
	case 6:
		addr_spmc = CPUSYS1_CPU2_SPMC_CTL;
		SPM_MPx_CPUx_PWR_CON = MP1_CPU2_PWR_CON;
		break;
	case 7:
		addr_spmc = CPUSYS1_CPU3_SPMC_CTL;
		SPM_MPx_CPUx_PWR_CON = MP1_CPU3_PWR_CON;
		break;
	case 0x10:		/*LL cluster */
		addr_spmc = MP0_CPUTOP_SPMC_CTL;
		SPM_MPx_CPUx_PWR_CON = MP0_CPUTOP_PWR_CON;
//		MPx_SNOOP_CTRL = MP0_SNOOP_CTRL;
		break;
	case 0x20:		/*L cluster */
		addr_spmc = MP1_CPUTOP_SPMC_CTL;
		SPM_MPx_CPUx_PWR_CON = MP1_CPUTOP_PWR_CON;
//		MPx_SNOOP_CTRL = MP1_SNOOP_CTRL;
		break;
	default:
		ERROR("%s() CPU%d not exists\n", __func__, (int)select);
		assert(0);
	}

	if (!addr_spmc) {
		PRINTF_SPMC("SwOn Inv");
		return -1;
	}
	PRINTF_SPMC("SPM_MPx_CPUx_PWR_CON%x=0x%x\n", SPM_MPx_CPUx_PWR_CON,
		    mmio_read_32(SPM_MPx_CPUx_PWR_CON));
	PRINTF_SPMC("SW_PWR_ON=0x%x\n", SW_PWR_ON);
	PRINTF_SPMC("SW_PWR_ON_OVERRIDE_EN=0x%x\n", SW_PWR_ON_OVERRIDE_EN);

	PRINTF_SPMC("MPx_CPUx_SPMC_CTL_0x%x=0x%x\n", addr_spmc, mmio_read_32(addr_spmc));
	if (mmio_read_32(addr_spmc) & SW_LOGIC_PDBO_ALL_ON_ACK) {
		PRINTF_SPMC("MPx_CPUx_SPMC_CTL_0x%x=0x%x\n", addr_spmc, mmio_read_32(addr_spmc));
		PRINTF_SPMC("select=%d Turn on already!!\n", select);
#if SPMC_DVT  ==  0
		/*		return 0;*//*turn on already */
#endif
	}
#if 0
	if (!(mmio_read_32(addr_spmc) & SW_FSM_OVERRIDE)) {
		PRINTF_SPMC("SW_FSM_OVERRIDE == 0\n");
		tmp = mmio_read_32(addr_spmc);
		if (!(tmp & SW_PWR_ON_OVERRIDE_EN)) {
			PRINTF_SPMC("SW_PWR_ON_OVERRIDE_EN == 0\n");
			tmp |= SW_PWR_ON_OVERRIDE_EN;
			mmio_write_32(addr_spmc, tmp);
		}
		tmp = mmio_read_32(addr_spmc) | SW_PWR_ON;
		mmio_write_32(addr_spmc, tmp);
	}
#endif
#if 0
/*??????????????????????????????????????????????????????????????????????????????????*/
	PRINTF_SPMC("SwOn HPRst\n");
	tmp = mmio_read_32(addr_spmc) | SW_HOT_PLUG_RESET;
	mmio_write_32(addr_spmc, tmp);
	PRINTF_SPMC("%x_SPMC_CTL_0x%x=0x%x\n", select, addr_spmc, mmio_read_32(addr_spmc));

	PRINTF_SPMC("SW_PWR_ON_OVERRIDE_EN=1\n");
	tmp |= SW_PWR_ON_OVERRIDE_EN;
	mmio_write_32(addr_spmc, tmp);
	PRINTF_SPMC("%x_SPMC_CTL_0x%x=0x%x\n", select, addr_spmc, mmio_read_32(addr_spmc));

	PRINTF_SPMC("SW_PWR_ON=1\n");
	tmp = mmio_read_32(addr_spmc) | SW_PWR_ON;
	mmio_write_32(addr_spmc, tmp);
	PRINTF_SPMC("%x_SPMC_CTL_0x%x=0x%x\n", select, addr_spmc, mmio_read_32(addr_spmc));


	if ((select & 0xF0) != 0) {	/*cluster */
		tmp = mmio_read_32(SPM_MPx_CPUx_PWR_CON) & ~MP0_CPUTOP_PWR_CLK_DIS_LSB;	/*all bit0 */
		mmio_write_32(SPM_MPx_CPUx_PWR_CON, tmp);	/*Release CPU0_PWR_RST_B */
		PRINTF_SPMC("SPM_MPx_CPUx_PWR_CON_0x%x=0x%x\n", SPM_MPx_CPUx_PWR_CON,
			    mmio_read_32(SPM_MPx_CPUx_PWR_CON));
	}
	tmp = mmio_read_32(SPM_MPx_CPUx_PWR_CON) | MCU_PWR_RST_B_LSB;	/*all bit0 */
	mmio_write_32(SPM_MPx_CPUx_PWR_CON, tmp);	/*Release CPU0_PWR_RST_B */
	PRINTF_SPMC("SPM_MPx_CPUx_PWR_CON_0x%x=0x%x\n", SPM_MPx_CPUx_PWR_CON,
		    mmio_read_32(SPM_MPx_CPUx_PWR_CON));

/*??????????????????????????????????????????????????????????????????????????????????*/
#endif
	/*power on SRAM */
/*
	tmp = mmio_read_32(addr_spmc) & ~SW_PD;
	PRINTF_SPMC("SwOn SRAM\n");
	for (i = 0; i < nb_srampd; i++) {
		tmp = tmp | (sw_pd_cmd[i]<<SRAMPD_OFFSET);
		mmio_write_32(addr_spmc, tmp);
	}
*/
	for (i = 0; i < nb_srampd; i++) {
		tmp = (i >> 1) ^ i;	/*binaryToGray */
		/*PRINTF_SPMC("binaryToGray[%d]=0x%x\n", i, tmp); */
		mmio_write_32(addr_spmc, (mmio_read_32(addr_spmc) & ~SW_PD) | tmp << SRAMPD_OFFSET);
	}

	PRINTF_SPMC("SwOn logic_pre1_pdb\n");
	tmp = mmio_read_32(addr_spmc) | SW_LOGIC_PRE1_PDB;
	mmio_write_32(addr_spmc, tmp);
	PRINTF_SPMC("SwOn Wait logic_pre1_pdb Ack\n");
	while (!(mmio_read_32(addr_spmc) & SW_LOGIC_PRE1_PDBO_ALL_ON_ACK)) ;	/*wait for 1 */
	PRINTF_SPMC("SwOn logic_pre2_pdb\n");
	tmp = mmio_read_32(addr_spmc) | SW_LOGIC_PRE2_PDB;
	mmio_write_32(addr_spmc, tmp);
	PRINTF_SPMC("SwOn WaitP logic_pre1_pdb Ack\n");
	while (!(mmio_read_32(addr_spmc) & SW_LOGIC_PRE2_PDBO_ALL_ON_ACK)) ;	/*wait for 1 */
	PRINTF_SPMC("SwOn logic_pdb\n");
	tmp = mmio_read_32(addr_spmc) | SW_LOGIC_PDB;
	mmio_write_32(addr_spmc, tmp);
	PRINTF_SPMC("SwOn Wait logic_pdb Ack\n");
	while (!(mmio_read_32(addr_spmc) & SW_LOGIC_PDBO_ALL_ON_ACK)) ;	/*wait for 1 */



	PRINTF_SPMC("SwOn SLEEPB\n");
	if ((select & 0xF0) != 0) {	/*cluster */
		for (i = 0; i < nb_srampd; i++) {
			tmp = (i >> 1) ^ i;	/*binaryToGray */
			/*PRINTF_SPMC("binaryToGray[%d]=0x%x\n", i, tmp); */
			mmio_write_32(addr_spmc,
				      (mmio_read_32(addr_spmc) & ~sw_sram_sleepb) | tmp << 7);
		}
	} else {
		tmp = mmio_read_32(addr_spmc) | cpu_sw_sram_sleepb;
		mmio_write_32(addr_spmc, tmp);
	}
	PRINTF_SPMC("SwOn sw_iso\n");
	tmp = mmio_read_32(addr_spmc) & ~cpu_sw_iso;
	PRINTF_SPMC("SwOn sw_isointb\n");
	tmp |= SW_SRAM_ISOINTB;
	mmio_write_32(addr_spmc, tmp);

	PRINTF_SPMC("SwOn ck_iso\n");
	tmp = mmio_read_32(addr_spmc) & ~SW_CKISO;
	mmio_write_32(addr_spmc, tmp);

	PRINTF_SPMC("SwOn clk_dis\n");
	tmp = mmio_read_32(addr_spmc) & ~SW_CLK_DIS;
	mmio_write_32(addr_spmc, tmp);

	PRINTF_SPMC("SwOn HPDeRST\n");
	tmp = mmio_read_32(addr_spmc) & ~SW_HOT_PLUG_RESET;
	mmio_write_32(addr_spmc, tmp);
	PRINTF_SPMC("[%s] [core%d]0x%x Reg:0x%x\n", __func__, select, addr_spmc,
		    mmio_read_32(addr_spmc));
	if ((select & 0xF0) != 0) {	/*cluster */
		unsigned int mpx_mask, MPx_AXI_CONFIG;
#if SPMC_DVT
		unsigned int MPx_SNOOP_CTRL;
#endif
#if 0
		unsigned int MPx_ARMPLL_CON0;

		PRINTF_SPMC("SwOn EnPLL\n");
		if (select == 0x10)
			MPx_ARMPLL_CON0 = ARMPLL_LL_CON0;
		else
			MPx_ARMPLL_CON0 = ARMPLL_L_CON0;

		tmp = mmio_read_32(MPx_ARMPLL_CON0) | (1 << 0);
		mmio_write_32(MPx_ARMPLL_CON0, tmp);	/*ARMPLL_CON0[0],  Enable PLL */
		udelay(20);
		PRINTF_SPMC("SwOn SelPLL\n");
#endif
		if (select == 0x10) {
			/*tmp =
			    (mmio_read_32(TOP_CKMUXSEL) & ~armpll_mux1_sel_sml_mask) |
			    armpll_mux1_sel_sml_ARMSPLL;*/
			MPx_AXI_CONFIG = MP0_AXI_CONFIG;
			mpx_mask = (1 << IDX_PROTECT_ICC0_CACTIVE) |
			    (1 << IDX_PROTECT_ICD0_CACTIVE) |
			    (1 << IDX_PROTECT_MP0_CACTIVE) | (1 << IDX_PROTECT_L2C0_CACTIVE);
#if SPMC_DVT
			MPx_SNOOP_CTRL = MP0_SNOOP_CTRL;
#endif
		} else {
			/*tmp =
			    (mmio_read_32(TOP_CKMUXSEL) & ~armpll_mux1_sel_big_mask) |
			    armpll_mux1_sel_big_ARMSPLL;*/
			MPx_AXI_CONFIG = MP1_AXI_CONFIG;
			mpx_mask = (1 << IDX_PROTECT_ICC1_CACTIVE) |
			    (1 << IDX_PROTECT_ICD1_CACTIVE) |
			    (1 << IDX_PROTECT_MP1_CACTIVE) | (1 << IDX_PROTECT_L2C1_CACTIVE);
#if SPMC_DVT
			MPx_SNOOP_CTRL = MP1_SNOOP_CTRL;
#endif
		}
		PRINTF_SPMC("MPx_AXI_CONFIG=%x\n", MPx_AXI_CONFIG);
		/*        mmio_write_32(TOP_CKMUXSEL,  tmp); //pll_div_mux1_sel = 01 = pll clock */

/* ################ BUS PROTECT DISABLE and SNOOP/DVM ENABLE################ */
		/* TINFO="Release bus protect" */
		/* TINFO="Release ADB pwrdnreqn for Cluster 0" */
		/* mmio_write_32(INFRA_TOPAXI_PROTECTEN_1, mmio_read_32(INFRA_TOPAXI_PROTECTEN_1) & ~mpx_mask); */
		mmio_write_32(MCUSYS_PROTECTEN_CLR, mpx_mask);
		PRINTF_SPMC("MCUSYS_PROTECTEN_CLR_0x%x=0x%x\n",
			    MCUSYS_PROTECTEN_CLR,
			    mmio_read_32(MCUSYS_PROTECTEN_CLR));

		/* TINFO="Wait ADB ~pwrdnreqn for Cluster 0" */
		while ((mmio_read_32(MCUSYS_PROTECTEN_STA1) & mpx_mask) != 0);
		PRINTF_SPMC("MCUSYS_PROTECTEN_STA1_0x%x=0x%x\n", MCUSYS_PROTECTEN_STA1,
			    mmio_read_32(MCUSYS_PROTECTEN_STA1));

#if SPMC_DVT			/* by callee in plat_affinst_off() */
/*	Program MP<n>_AXI_CONFIG acinactm to 0*/
		mmio_write_32(MPx_AXI_CONFIG,
				      mmio_read_32(MPx_AXI_CONFIG) & ~MPx_AXI_CONFIG_acinactm);
		PRINTF_SPMC("MPx_AXI_CONFIG_0x%x=0x%x\n", MPx_AXI_CONFIG,
			    mmio_read_32(MPx_AXI_CONFIG));

/*	Program MCSI-A Slave Control Register (MP0:slave0,  MP1:slave1) */
/*to enable snoop/DVM function for related slave port through ACE(0x10390000) or APB(0x10200000) interface*/
/*	Polling CCI Status register through ACE interface (0x1039000C[0]) returns to 0*/
/*		mmio_write_32(MPx_SNOOP_CTRL, mmio_read_32(MPx_SNOOP_CTRL) | MPx_SNOOP_ENABLE);*/
/*	  PRINTF_SPMC("MPx_SNOOP_CTRL_0x%x=0x%x\n", MPx_SNOOP_CTRL, mmio_read_32(MPx_SNOOP_CTRL));*/
		/*mmio_write_32(0x10394000, mmio_read_32(0x10394000) | MPx_SNOOP_ENABLE);
		   mmio_write_32(0x10395000, mmio_read_32(0x10395000) | MPx_SNOOP_ENABLE); */
		mmio_write_32(MPx_SNOOP_CTRL, mmio_read_32(MPx_SNOOP_CTRL) | MPx_SNOOP_ENABLE);
		PRINTF_SPMC("MPx_SNOOP_CTRL%x=0x%x\n", MPx_SNOOP_CTRL,
			    mmio_read_32(MPx_SNOOP_CTRL));
		PRINTF_SPMC("MPx_SNOOP_STATUS_0x%x=0x%x\n", MPx_SNOOP_STATUS,
			    mmio_read_32(MPx_SNOOP_STATUS));
		while ((mmio_read_32(MPx_SNOOP_STATUS) & (1 << 31)) == (1 << 31));
		/* mcsib_sw_workaround_main(); */
		PRINTF_SPMC("MPx_SNOOP_STATUS_0x%x=0x%x\n", MPx_SNOOP_STATUS,
			    mmio_read_32(MPx_SNOOP_STATUS));
#endif
/* ################ BUS PROTECT DISABLE and SNOOP/DVM ENABLE################ */
	}

	return 0;
}

int little_spmc_sw_pwr_on(int select)
{
	int ret;
	/*power on a core before power on the TOP,  we power on TOP automatically */
	PRINTF_SPMC(">>>>>>>>>>>>>>[%s]SwPsqOn Sel:%d little_on=0x%x\n", __func__, select,
		    little_on);
	/* TINFO="enable SPM register control" */
	mmio_write_32(POWERON_CONFIG_EN, (SPM_PROJECT_CODE << 16) | (0x1 << 0));

	if (little_on & (1 << select))
		return PSCI_E_SUCCESS;

	switch (select) {
	case 0x0:
	case 0x1:
	case 0x2:
	case 0x3:
		if ((little_on & 0xF) == 0) {
			PRINTF_SPMC("SWpowerON MP0\n");
#if 0
			little_spmc_info();
#endif
			ret = little_spmc_sw_pwr_seq_en(0x10, STA_POWER_DOWN);
			if (ret)
				PRINTF_SPMC("little_spmc_sw_pwr_seq_en(0x10) return error=%d\n",
					    ret);
#if 0
			udelay(SPMC_DVT_UDELAY);
			little_spmc_info();
#endif

			ret = little_sw_on_seq(0x10);	/*power on TOP */
			if (ret)
				PRINTF_SPMC("little_sw_on_seq(0x10) return error=%d\n", ret);

#if 0
			udelay(SPMC_DVT_UDELAY);
			little_spmc_info();
#endif
		}
#if 0
		mmio_write_32(MP0_MISC_CONFIG3, mmio_read_32(MP0_MISC_CONFIG3) | 0x0000F000);
		/*mmio_write_32(MP2_CPUCFG,  0xf<<16);//set BIG poweup on AARCH64 */
		mmio_write_32(MP0_MISC_CONFIG_BOOT_ADDR(select), (unsigned long)sec_entrypoint);
		PRINTF_SPMC("mt_on_0,  entry %x\n",
			    mmio_read_32(MP0_MISC_CONFIG_BOOT_ADDR(select)));
/*			PRINTF_SPMC("Write 0x10208008=%d\n", 8+add_idx);*/
/*			mmio_write_32(0x10208008,  8+add_idx);//discussed with Scott*/
#endif
#if 0
		udelay(SPMC_DVT_UDELAY);
		little_spmc_info();
#endif
		PRINTF_SPMC(">>>>>>SWpowerON core%d\n", select);
		ret = little_spmc_sw_pwr_seq_en(select, STA_POWER_DOWN);
		if (ret)
			PRINTF_SPMC("little_spmc_sw_pwr_seq_en(%d) return error=%d\n", select, ret);
#if 0
		udelay(SPMC_DVT_UDELAY);
		little_spmc_info();
#endif

		ret = little_sw_on_seq(select);
		if (ret)
			PRINTF_SPMC("little_sw_on_seq(%d) return error=%d\n", select, ret);
#if 0
		udelay(SPMC_DVT_UDELAY);
		little_spmc_info();
#endif

		little_on |= (1 << select);
#if SPMC_SPARK2
		if (select == 0x1)
			little_spark2_setldo(select);
		little_spark2_core_enable(select, 1);
#endif
		break;
	case 0x4:
	case 0x5:
	case 0x6:
	case 0x7:
		if ((little_on & 0xF0) == 0) {
			PRINTF_SPMC(">>>>>>SW power on MP1\n");
#if 0
			little_spmc_info();
#endif
			ret = little_spmc_sw_pwr_seq_en(0x20, STA_POWER_DOWN);
			if (ret)
				PRINTF_SPMC("little_spmc_sw_pwr_seq_en(0x20) return error=%d\n",
					    ret);
#if 0
			udelay(SPMC_DVT_UDELAY);
			little_spmc_info();
#endif

			ret = little_sw_on_seq(0x20);	/*power on TOP */
			if (ret)
				PRINTF_SPMC("little_sw_on_seq(0x20) return error=%d\n", ret);

#if 0
			udelay(SPMC_DVT_UDELAY);
			little_spmc_info();
#endif
		}
#if 0
		mmio_write_32(MP1_MISC_CONFIG3, mmio_read_32(MP1_MISC_CONFIG3) | 0x0000F000);
		/*mmio_write_32(MP2_CPUCFG,  0xf<<16);//set BIG poweup on AARCH64 */
		mmio_write_32(MP1_MISC_CONFIG_BOOT_ADDR(select), (unsigned long)sec_entrypoint);
		PRINTF_SPMC("mt_on_1,  entry %x\n",
			    mmio_read_32(MP1_MISC_CONFIG_BOOT_ADDR(select)));
/*			PRINTF_SPMC("Write 0x10208008=%d\n", 8+add_idx);*/
/*			mmio_write_32(0x10208008,  8+add_idx);//discussed with Scott*/
#endif
#if 0
		udelay(SPMC_DVT_UDELAY);
		little_spmc_info();
#endif

		PRINTF_SPMC(">>>>>>SWpowerON core%d\n", select);
		ret = little_spmc_sw_pwr_seq_en(select, STA_POWER_DOWN);
		if (ret)
			PRINTF_SPMC("little_spmc_sw_pwr_seq_en(%d) return error=%d\n", select, ret);

#if 0
		udelay(SPMC_DVT_UDELAY);
		little_spmc_info();
#endif

		ret = little_sw_on_seq(select);
		if (ret)
			PRINTF_SPMC("little_sw_on_seq(%d) return error=%d\n", select, ret);

#if 0
		udelay(SPMC_DVT_UDELAY);
		little_spmc_info();
#endif

		little_on |= (1 << select);
#if SPMC_SPARK2
		little_spark2_setldo(select);
		little_spark2_core_enable(select, 1);
#endif
		break;
#if 0
	case 0xf:
		big_spmc_sw_pwr_seq_en(0x30);
		big_spmc_info();
		big_sw_on_seq(0x30);	/*power on TOP */
		big_spmc_info();
		big_sw_on_seq(0x1);
		big_spmc_info();
#if SPMC_SPARK2
		big_spark2_setldo(0, 0);
		big_spark2_core(8, 1);
#endif
		big_sw_on_seq(0x2);
		big_spmc_info();
#if SPMC_SPARK2
		big_spark2_core(9, 1);
#endif
		break;
#endif
	case 0x10:
		little_spmc_sw_pwr_seq_en(0x10, STA_POWER_DOWN);
		little_spmc_info();
		little_sw_on_seq(0x10);
		little_spmc_info();
		break;
	case 0x20:
		little_spmc_sw_pwr_seq_en(0x20, STA_POWER_DOWN);
		little_spmc_info();
		little_sw_on_seq(0x20);
		little_spmc_info();
		break;

	default:
		return -1;
	}
	PRINTF_SPMC("<<<<<<<<<<<<<<[%s]SwPsqOn Sel:%d little_on=0x%x\n", __func__, select,
		    little_on);
	return PSCI_E_SUCCESS;
}

/**
 * BigSPMCSwPwrOff
 * selectL 0x1: core 1,  0x2:core 2,  0xF:all cores ,  0x10:TOP
 */
/*
"1. if Select invalid,  exit/return error -1
		ForSelect: (Do CPU's before TOP)
		2. Read sw_fsm_override & sw_pwr_on_override_en & pwr state
		3. If already power off,  continue to next Select.
		4. If sw_fsm_override=0
				1. if sw_pwr_on_override=0
					1. Write sw_pwr_on_override_en = 1
				2. write sw_pwr_on=0
		5. else
	   Sequence:
				1. sw_iso=1,  sw_sram_isointb=0
				2. sw_logic*_pdb=0,  sw_sram_sleepb=0,  sw_pd=6??h00
				3. Poll/wait for logic_*pdb_all_on_ack=0 & logic_pdbo_all_off_ack=1
				4. sw_hot_plug_reset=1"

*/
void little_sw_off_seq(int select)
{
	unsigned int addr_spmc, tmp;
	unsigned int SW_FSM_OVERRIDE, SW_PWR_ON_OVERRIDE_EN, SW_PWR_ON, SW_ISO, SW_SRAM_ISOINTB,
	    SW_LOGIC_PRE1_PDB, SW_LOGIC_PRE2_PDB, SW_LOGIC_PDB;
	unsigned int SW_LOGIC_PRE1_PDBO_ALL_ON_ACK, SW_LOGIC_PRE2_PDBO_ALL_ON_ACK,
	    SW_HOT_PLUG_RESET, SW_SRAM_SLEEPB, SW_PD, SW_LOGIC_PDBO_ALL_OFF_ACK;
	unsigned int SW_CKISO, SW_CLK_DIS;
//	unsigned int MPx_SNOOP_CTRL, MPx_CPUx_STANDBYWFI;

	if ((select & 0xF0) != 0) {	/*cluster */
		SW_LOGIC_PDBO_ALL_OFF_ACK = logic_pdbo_all_off_ack;
		SW_FSM_OVERRIDE = sw_fsm_override;
		SW_PWR_ON_OVERRIDE_EN = sw_pwr_on_override_en;
		SW_PWR_ON = sw_pwr_on;
		SW_ISO = sw_iso;
		SW_HOT_PLUG_RESET = sw_hot_plug_reset;
		SW_PD = sw_pd;
		SW_LOGIC_PRE1_PDB = sw_logic_pre1_pdb;
		SW_LOGIC_PRE1_PDBO_ALL_ON_ACK = logic_pre1_pdbo_all_on_ack;
		SW_LOGIC_PRE2_PDB = sw_logic_pre2_pdb;
		SW_LOGIC_PRE2_PDBO_ALL_ON_ACK = logic_pre2_pdbo_all_on_ack;
		SW_LOGIC_PDB = sw_logic_pdb;
		SW_SRAM_ISOINTB = sw_sram_isointb;
		SW_SRAM_SLEEPB = sw_sram_sleepb;
		SW_CKISO = sw_ckiso;
		SW_CLK_DIS = sw_clk_dis;
	} else {		/*core */
		/*SW_LOGIC_PDBO_ALL_OFF_ACK = cpu_pdbo_all_off_ack; *//* TODO:no register */
		SW_FSM_OVERRIDE = cpu_sw_fsm_override;
		SW_PWR_ON_OVERRIDE_EN = cpu_sw_powr_on_override_en;
		SW_PWR_ON = cpu_sw_pwr_on;
		SW_ISO = cpu_sw_iso;
		SW_HOT_PLUG_RESET = cpu_sw_hot_plug_reset;
		SW_PD = cpu_sw_pd;
		SW_LOGIC_PRE1_PDB = cpu_sw_logic_pre1_pdb;
		SW_LOGIC_PRE1_PDBO_ALL_ON_ACK = cpu_pre1_pdbo_allon_ack;
		SW_LOGIC_PRE2_PDB = cpu_sw_logic_pre2_pdb;
		SW_LOGIC_PRE2_PDBO_ALL_ON_ACK = cpu_pre2_pdbo_allon_ack;
		SW_LOGIC_PDB = cpu_sw_logic_pdb;
		SW_SRAM_ISOINTB = cpu_sw_sram_isointb;
		SW_SRAM_SLEEPB = cpu_sw_sram_sleepb;
		SW_CKISO = cpu_sw_ckiso;
		SW_CLK_DIS = cpu_sw_clk_dis;
	}

	switch (select) {
	case 0:
		addr_spmc = CPUSYS0_CPU0_SPMC_CTL;
		break;
	case 1:
		addr_spmc = CPUSYS0_CPU1_SPMC_CTL;
		break;
	case 2:
		addr_spmc = CPUSYS0_CPU2_SPMC_CTL;
		break;
	case 3:
		addr_spmc = CPUSYS0_CPU3_SPMC_CTL;
		break;
	case 4:
		addr_spmc = CPUSYS1_CPU0_SPMC_CTL;
		break;
	case 5:
		addr_spmc = CPUSYS1_CPU1_SPMC_CTL;
		break;
	case 6:
		addr_spmc = CPUSYS1_CPU2_SPMC_CTL;
		break;
	case 7:
		addr_spmc = CPUSYS1_CPU3_SPMC_CTL;
		break;
	case 0x10:		/*LL cluster */
		addr_spmc = MP0_CPUTOP_SPMC_CTL;
//		MPx_SNOOP_CTRL = MP0_SNOOP_CTRL;
//		MPx_CPUx_STANDBYWFI = MP0_STANDBYWFI;
		break;
	case 0x20:		/*L cluster */
		addr_spmc = MP1_CPUTOP_SPMC_CTL;
//		MPx_SNOOP_CTRL = MP1_SNOOP_CTRL;
//		MPx_CPUx_STANDBYWFI = MP1_STANDBYWFI;
		break;
	default:
		ERROR("%s() CPU%d not exists\n", __func__, (int)select);
		assert(0);
	}
/* TODO: no register	*/
/*	if (!(mmio_read_32(addr_spmc) & FSM_STATE_OUT_MASK))// TODO: no register*/
/*		return; //turn off already*/
	if (!(mmio_read_32(addr_spmc) & SW_FSM_OVERRIDE)) {
		tmp = mmio_read_32(addr_spmc);
		if (!(tmp & SW_PWR_ON_OVERRIDE_EN)) {
			tmp |= SW_PWR_ON_OVERRIDE_EN;
			mmio_write_32(addr_spmc, tmp);
		}
		tmp &= ~SW_PWR_ON;
		mmio_write_32(addr_spmc, tmp);
	}
		if ((select & 0xF0) != 0) {	/*cluster */
			unsigned int /*MPx_ARMPLL_CON0,*/ MPx_IDLE_STA;
			unsigned int MPx_AXI_CONFIG, /*MPx_CA7_MISC_CONFIG,*/ mpx_mask;
			unsigned int MPx_CPUx_STANDBYWFI;
#if SPMC_DVT
			unsigned int MPx_SNOOP_CTRL;
#endif

			if (select == 0x10) {
				MPx_IDLE_STA = 1 << 20;
				/*MPx_ARMPLL_CON0 = ARMPLL_LL_CON0;*/
				MPx_AXI_CONFIG = MP0_AXI_CONFIG;
//				MPx_CA7_MISC_CONFIG = MP0_CA7_MISC_CONFIG;
				mpx_mask = (1 << IDX_PROTECT_ICC0_CACTIVE) |
				    (1 << IDX_PROTECT_ICD0_CACTIVE) |
				    (1 << IDX_PROTECT_MP0_CACTIVE) |
				    (1 << IDX_PROTECT_L2C0_CACTIVE);
#if SPMC_DVT
				MPx_SNOOP_CTRL = MP0_SNOOP_CTRL;
#endif
				MPx_CPUx_STANDBYWFI = MP0_STANDBYWFI;

			} else {
				MPx_IDLE_STA = 1 << 21;
				/*MPx_ARMPLL_CON0 = ARMPLL_L_CON0;*/
//				MPx_CA7_MISC_CONFIG = MP1_CA7_MISC_CONFIG;
				MPx_AXI_CONFIG = MP1_AXI_CONFIG;
				mpx_mask = (1 << IDX_PROTECT_ICC1_CACTIVE) |
				    (1 << IDX_PROTECT_ICD1_CACTIVE) |
				    (1 << IDX_PROTECT_MP1_CACTIVE) |
				    (1 << IDX_PROTECT_L2C1_CACTIVE);
#if SPMC_DVT
				MPx_SNOOP_CTRL = MP1_SNOOP_CTRL;
#endif
				MPx_CPUx_STANDBYWFI = MP1_STANDBYWFI;
			}
			PRINTF_SPMC("Wait CL %x WFI\n", select);
			/*while ((mmio_read_32(CPU_IDLE_STA) & MPx_IDLE_STA) != MPx_IDLE_STA); */
			PRINTF_SPMC("CL %x is in WFI\n", select);
			PRINTF_SPMC("CPU_IDLE_STA=0x%x\n", MPx_IDLE_STA);
			PRINTF_SPMC("MPx_AXI_CONFIG=%x\n", MPx_AXI_CONFIG);
			/*PRINTF_SPMC("MPx_ARMPLL_CON0=%x\n", MPx_ARMPLL_CON0);*/
#if SPMC_DVT			/* by callee in plat_affinst_off() */
		/* TINFO="Start to turn off MP0_CPUTOP" */
/* ################ BUS PROTECT ENABLE and SNOOP/DVM DISABLE ################ */
		mmio_write_32(MPx_SNOOP_CTRL, mmio_read_32(MPx_SNOOP_CTRL) & ~MPx_SNOOP_ENABLE);
		/* mmio_write_32(0x10394000,  mmio_read_32(0x10394000) & ~MPx_SNOOP_ENABLE); */
		/* mmio_write_32(0x10395000,  mmio_read_32(0x10395000) & ~MPx_SNOOP_ENABLE); */
		while ((mmio_read_32(MPx_SNOOP_STATUS) & (1 << 31)) == (1 << 31));
		/* mcsib_sw_workaround_main(); */
		/*      Program MP<n>_AXI_CONFIG acinactm to 1 */
		/*      Wait mp<n>_STANDBYWFIL2 to high */
		/* TINFO="Set acinactm = 1" */
		mmio_write_32(MPx_AXI_CONFIG,
				      mmio_read_32(MPx_AXI_CONFIG) | MPx_AXI_CONFIG_acinactm);
#endif
		/* TINFO="Wait STANDBYWFIL2 for Cluster 0" */
		PRINTF_SPMC("CPU_IDLE_STA_0x%x=0x%x\n", CPU_IDLE_STA, mmio_read_32(CPU_IDLE_STA));
		while (!(mmio_read_32(CPU_IDLE_STA) & MPx_CPUx_STANDBYWFI));

		/* TINFO="Set ADB pwrdnreqn for Cluster 0" */
		/* mmio_write_32(INFRA_TOPAXI_PROTECTEN_1,  mmio_read_32(INFRA_TOPAXI_PROTECTEN_1)|mpx_mask); */
		mmio_write_32(MCUSYS_PROTECTEN_SET, mpx_mask);
		/* TINFO="Wait ADB pwrdnreqn for Cluster 0" */
		while ((mmio_read_32(MCUSYS_PROTECTEN_STA1) & mpx_mask) != mpx_mask);
/* ################ BUS PROTECT ENABLE and SNOOP/DVM DISABLE ################ */

#if 0
			if (select == 0x10)
				tmp = mmio_read_32(TOP_CKMUXSEL) & ~armpll_mux1_sel_sml_mask;
			else
				tmp = mmio_read_32(TOP_CKMUXSEL) & ~armpll_mux1_sel_big_mask;

			mmio_write_32(TOP_CKMUXSEL, tmp);	/*pll_div_mux1_sel = 26MHZ */

			tmp = mmio_read_32(MPx_ARMPLL_CON0) & ~(1 << 0);
			mmio_write_32(MPx_ARMPLL_CON0, tmp);	/*ARMPLL_CON0[0],  Disable PLL */
#endif
		} else {
			/*wait WFE */
#if SPMC_DVT
			if (select < 4) {
				while (!
				       (mmio_read_32(MP0_CA7_MISC_CONFIG) &
					(MP0_CPU0_STANDBYWFE << select)));
			} else if (select < 8) {
				while (!
				       (mmio_read_32(MP1_CA7_MISC_CONFIG) &
					(MP1_CPU0_STANDBYWFE << (select - 4))));
			}
#else
			PRINTF_SPMC("Wait CPU WFI mask:%x\n", select);
			while (!(mmio_read_32(CPU_IDLE_STA) & (1 << (12 + select))));
			PRINTF_SPMC("CPU is in WFI,  mask:%x\n", select);

#endif

		}

		tmp = mmio_read_32(addr_spmc) | SW_CKISO;
		mmio_write_32(addr_spmc, tmp);

		tmp = mmio_read_32(addr_spmc) | SW_ISO;
		tmp &= ~SW_SRAM_ISOINTB;
		mmio_write_32(addr_spmc, tmp);

		tmp = mmio_read_32(addr_spmc) | SW_CLK_DIS;
		mmio_write_32(addr_spmc, tmp);

		tmp =
		    mmio_read_32(addr_spmc) & ~(SW_LOGIC_PRE1_PDB | SW_LOGIC_PRE2_PDB | SW_LOGIC_PDB
						| SW_SRAM_SLEEPB | SW_PD);
		mmio_write_32(addr_spmc, tmp);

		while ((mmio_read_32(addr_spmc) & SW_LOGIC_PRE1_PDBO_ALL_ON_ACK)) ;	/*wait for 0 */
		while ((mmio_read_32(addr_spmc) & SW_LOGIC_PRE2_PDBO_ALL_ON_ACK)) ;	/*wait for 0 */
		if ((select & 0xF0) != 0)	/*just only do cluster since core no register can poll */
			while (!(mmio_read_32(addr_spmc) & SW_LOGIC_PDBO_ALL_OFF_ACK))	/*wait for 1 */
				;

		tmp = mmio_read_32(addr_spmc) | SW_HOT_PLUG_RESET;
		mmio_write_32(addr_spmc, tmp);
#if 0
		if (select == 0x10) {
/*??????????????????????????????????????????????????????????????????????????????????*/

			tmp = mmio_read_32(SPM_MP2_CPUSYS_PWR_CON) & ~(1 << 0);
			mmio_write_32(SPM_MP2_CPUSYS_PWR_CON, tmp);	/*Release pwr_rst_b */

			tmp = mmio_read_32(SPM_MP2_CPUSYS_PWR_CON) | (1 << 4);
			mmio_write_32(SPM_MP2_CPUSYS_PWR_CON, tmp);	/*Release */

			tmp = mmio_read_32(SPM_CPU_EXT_BUCK_ISO) | (0x2);
			mmio_write_32(SPM_CPU_EXT_BUCK_ISO, tmp);
			/*ISOLATE*/
/*??????????????????????????????????????????????????????????????????????????????????*/
		}
#endif

}

int little_spmc_sw_pwr_off(int select)
{
	int ret;
	/* TINFO="enable SPM register control" */
	mmio_write_32(POWERON_CONFIG_EN, (SPM_PROJECT_CODE << 16) | (0x1 << 0));
	PRINTF_SPMC(">>>>>>>>>>>>>>[%s]SwPsqOn Sel:%d little_on=0x%x\n", __func__, select,
		    little_on);
	switch (select) {
	case 0x0:
		ret = little_spmc_sw_pwr_seq_en(select, STA_POWER_ON);
		if (ret)
			PRINTF_SPMC("little_spmc_sw_pwr_seq_en(0x10) return error=%d\n", ret);
		little_sw_off_seq(select);
		mmio_write_32(MP0_CPU0_PWR_CON, mmio_read_32(MP0_CPU0_PWR_CON) & ~PWR_ON_2ND);
		mmio_write_32(MP0_CPU0_PWR_CON, mmio_read_32(MP0_CPU0_PWR_CON) & ~PWR_ON);
		little_on &= ~(1 << select);
		PRINTF_SPMC("%s little_on:%x\n", __func__, little_on);
		if (!(little_on & 0xF)) {
			ret = little_spmc_sw_pwr_seq_en(0x10, STA_POWER_ON);
			if (ret)
				PRINTF_SPMC("little_spmc_sw_pwr_seq_en(0x10) return error=%d\n",
					    ret);
			little_sw_off_seq(0x10);
			mmio_write_32(MP0_CPUTOP_PWR_CON,
				      mmio_read_32(MP0_CPUTOP_PWR_CON) & ~PWR_ON_2ND);
			mmio_write_32(MP0_CPUTOP_PWR_CON,
				      mmio_read_32(MP0_CPUTOP_PWR_CON) & ~PWR_ON);
		}
		break;
	case 0x1:
	case 0x2:
	case 0x3:
#if 0
		udelay(SPMC_DVT_UDELAY);
		little_spmc_info();
#endif

		little_sw_off_seq(select);

#if 0
		udelay(SPMC_DVT_UDELAY);
		little_spmc_info();
#endif

		little_on &= ~(1 << select);
		PRINTF_SPMC("%s little_on:%x\n", __func__, little_on);
		if (!(little_on & 0xF)) {
			ret = little_spmc_sw_pwr_seq_en(0x10, STA_POWER_ON);
			if (ret)
				PRINTF_SPMC("little_spmc_sw_pwr_seq_en(0x10) return error=%d\n",
					    ret);
			little_sw_off_seq(0x10);
			mmio_write_32(MP0_CPUTOP_PWR_CON,
				      mmio_read_32(MP0_CPUTOP_PWR_CON) & ~PWR_ON_2ND);
			mmio_write_32(MP0_CPUTOP_PWR_CON,
				      mmio_read_32(MP0_CPUTOP_PWR_CON) & ~PWR_ON);
#if 0
			udelay(SPMC_DVT_UDELAY);
			little_spmc_info();
#endif
		}
		break;
	case 0x4:
	case 0x5:
	case 0x6:
	case 0x7:
#if 0
		udelay(SPMC_DVT_UDELAY);
		little_spmc_info();
#endif

		little_sw_off_seq(select);

#if 0
		udelay(SPMC_DVT_UDELAY);
		little_spmc_info();
#endif

		little_on &= ~(1 << select);
		PRINTF_SPMC("%s little_on:%x\n", __func__, little_on);
		if (!(little_on & 0xF0)) {
			little_sw_off_seq(0x20);
#if 0
			udelay(SPMC_DVT_UDELAY);
			little_spmc_info();
#endif
		}
		break;

	case 0xf:
		little_sw_off_seq(0x0);
		little_sw_off_seq(0x1);
		little_sw_off_seq(0x2);
		little_sw_off_seq(0x3);
		little_sw_off_seq(0x4);
		little_sw_off_seq(0x5);
		little_sw_off_seq(0x6);
		little_sw_off_seq(0x7);
		break;
	case 0x10:
		little_sw_off_seq(0x0);
		little_sw_off_seq(0x1);
		little_sw_off_seq(0x2);
		little_sw_off_seq(0x3);
		little_sw_off_seq(0x10);
		break;
	case 0x20:
		little_sw_off_seq(0x4);
		little_sw_off_seq(0x5);
		little_sw_off_seq(0x6);
		little_sw_off_seq(0x7);
		little_sw_off_seq(0x20);
		break;

	default:
		return -1;
	}
	PRINTF_SPMC("<<<<<<<<<<<<<<<<[%s]SwPsqOn Sel:%d little_on=0x%x\n", __func__, select,
		    little_on);
	return 0;

}

/**
 * BigSPMCSwPwrCntrlDisable
 * select : 0x1: core0,  0x2:core1 0xF:all 0x10:TOP
 * return : 0: Ok -1: Invalid Select value -2: Invalid request.

 */
/*
"1. if Select invalid,  exit/return error -1
2. For Select,  read sw_fsm_override.
3. If sw_fsm_override = 1
	1. return error -2
4. else
	1. Write all sw_pwr_on_override_en = 0"

*/

int little_spmc_sw_pwr_cntrl_disable(int select)
{
	unsigned int addr_spmc, tmp;
	unsigned int SW_FSM_OVERRIDE, SW_PWR_ON_OVERRIDE_EN;

	if ((select & 0xF0) != 0) {	/*cluster */
		SW_FSM_OVERRIDE = sw_fsm_override;
		SW_PWR_ON_OVERRIDE_EN = sw_pwr_on_override_en;
	} else {		/*core */
		SW_FSM_OVERRIDE = cpu_sw_fsm_override;
		SW_PWR_ON_OVERRIDE_EN = cpu_sw_powr_on_override_en;
	}


	switch (select) {
	case 0:
		addr_spmc = CPUSYS0_CPU0_SPMC_CTL;
		break;
	case 1:
		addr_spmc = CPUSYS0_CPU1_SPMC_CTL;
		break;
	case 2:
		addr_spmc = CPUSYS0_CPU2_SPMC_CTL;
		break;
	case 3:
		addr_spmc = CPUSYS0_CPU3_SPMC_CTL;
		break;
	case 4:
		addr_spmc = CPUSYS1_CPU0_SPMC_CTL;
		break;
	case 5:
		addr_spmc = CPUSYS1_CPU1_SPMC_CTL;
		break;
	case 6:
		addr_spmc = CPUSYS1_CPU2_SPMC_CTL;
		break;
	case 7:
		addr_spmc = CPUSYS1_CPU3_SPMC_CTL;
		break;
	case 0x10:		/*LL cluster */
		addr_spmc = MP0_CPUTOP_SPMC_CTL;
		break;
	case 0x20:		/*L cluster */
		addr_spmc = MP1_CPUTOP_SPMC_CTL;
		break;
	default:
		ERROR("%s() CPU%d not exists\n", __func__, (int)select);
		assert(0);
	}

	switch (select) {
	case 0x0:
	case 0x1:
	case 0x2:
	case 0x3:
	case 0x4:
	case 0x5:
	case 0x6:
	case 0x7:
	case 0x10:
	case 0x20:
		if (mmio_read_32(addr_spmc) & SW_FSM_OVERRIDE) {
			return -2;
		} else {
			tmp = mmio_read_32(addr_spmc) & ~SW_PWR_ON_OVERRIDE_EN;
			mmio_write_32(addr_spmc, tmp);
		}
		break;
/*	case 0xf:
			if ((mmio_read_32(PTP3_CPU0_SPMC) & SW_FSM_OVERRIDE)||
				(mmio_read_32(PTP3_CPU1_SPMC) & SW_FSM_OVERRIDE)) {
				return -2;
			}
			tmp = mmio_read_32(PTP3_CPU0_SPMC) & ~SW_PWR_ON_OVERRIDE_EN;
			mmio_write_32(PTP3_CPU0_SPMC,  tmp);
			tmp = mmio_read_32(PTP3_CPU1_SPMC) & ~SW_PWR_ON_OVERRIDE_EN;
			mmio_write_32(PTP3_CPU1_SPMC,  tmp);
			break;
	case 0x10:
			if ((mmio_read_32(PTP3_CPU0_SPMC) & SW_FSM_OVERRIDE)||
				(mmio_read_32(PTP3_CPU1_SPMC) & SW_FSM_OVERRIDE)) {
				return -2;
			}
			tmp = mmio_read_32(PTP3_CPU0_SPMC) & ~SW_PWR_ON_OVERRIDE_EN;
			mmio_write_32(PTP3_CPU0_SPMC,  tmp);
			tmp = mmio_read_32(PTP3_CPU1_SPMC) & ~SW_PWR_ON_OVERRIDE_EN;
			mmio_write_32(PTP3_CPU1_SPMC,  tmp);

			tmp = mmio_read_32(addr_spmc) & ~SW_PWR_ON_OVERRIDE_EN;
			mmio_write_32(addr_spmc,  tmp);
*/
	default:
		return -1;
	}
	return 0;
}


#if MT6757_PRIMARY_CPU != 1
/* SWBig BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB */
/*
static unsigned int select_spmc_base(int select, unsigned int *nb_srampd)
{
    unsigned int addr_spmc;
    switch(select)
    {
	case 0x1:
	    addr_spmc = CPUSYS0_CPU0_SPMC_CTL;
	    if(nb_srampd)
		*nb_srampd = 16;
	    break;
	case 0x2:
	    addr_spmc = CPUSYS0_CPU1_SPMC_CTL;
	    if(nb_srampd)
		*nb_srampd = 16;
	    break;
	case 0x10:
	    addr_spmc = PTP3_CPUTOP_SPMC;
	    if(nb_srampd)
		*nb_srampd = 32;
	    break;
	default:
	    PRINTF_SPMC("Should be not here\n");
	    assert(0);
    }
    return addr_spmc;
}
*/
/**
 * Before enable the SW sequenwce, all of the big cores must be turn offset.
 * SO the function cannot be called on a big core
 * HW-API:BigSPMCSwPwrSeqEn
 * select: 0x1:core0 0x2:core1 0x4:all 0x10:top
 *
 */
/*
   1. if Select invalid, exit/return error -1
    For Select
    2. Read  sw_fsm_override  & fsm_state_out
    3. if sw_fsm_override=0 & fsm_state_out=0
    1. Set all CPU SW state to OFF (no sequencing required)
    - sram_pd = 5'h00 for CPU & 6'h00 for TOP
    - sw_logic_*_pdb = 0
    - sw_sram_sleepb = 0
    - sw_iso = 1, sw_sram_isointb = 0
    - sw_hot_plug_reset = 1
    2. Write all cpu sw_fsm_override = 1
    3. Poll/wait for all cpu fsm_state_out = 1
    3. else return error -2 or -3
    "0: Ok
    -1: Invalid parameter
    -2: sw_fsm_override=1
    -3: fsm_state_out=1
    -4: Timeout occurred"

*/

int big_spmc_sw_pwr_seq_en(int select)
{

	unsigned int tmp, result, retry;
	unsigned int addr_spmc_ctl, addr_spmc_sta;

	PRINTF_SPMC(">>>>>>>> [%s] select=0x%x\n", __func__, select);

	switch (select) {
	case 0:
		addr_spmc_ctl = CPUSYS2_CPU0_SPMC_CTL;
		addr_spmc_sta = CPUSYS2_CPU0_SPMC_STA;
		break;
	case 1:
		addr_spmc_ctl = CPUSYS2_CPU1_SPMC_CTL;
		addr_spmc_sta = CPUSYS2_CPU1_SPMC_STA;
		break;
	case 0x30:		/* B cluster */
		addr_spmc_ctl = MP2_CPUTOP_SPMC_CTL;
		addr_spmc_sta = MP2_CPUTOP_SPMC_STA;
		break;
	default:
		ERROR("[ATF]: %s() CPU%d not exists\n", __func__, (int)select);
		assert(0);
	}


	if ((mmio_read_32(addr_spmc_sta) & B_FSM_STATE_OUT_MASK) == B_FSM_ON) {
		/* FSM_out is not zero */
		PRINTF_SPMC("[ERROR]B_FSM_STATE_OUT_MASK is on B_FSM_ON state before set...\n");	/* big sw power sequence error FSM */
		return -3;	/* -2: fsm_state_out=1 */

	}

	if ((mmio_read_32(addr_spmc_ctl) & B_SW_FSM_OVERRIDE)) {
		/* FSM_out is not zero */
		PRINTF_SPMC("[ERROR]B_SW_FSM_OVERRIDE is high before set...\n");
		return -2;	/* -1: sw_fsm_override=1 */
	}

/* ?????????????????????????????????????????????????????????????????????????????????? */
#if 0
	/* Enable buck first */
	tmp = mmio_read_32(SPM_MP2_CPUSYS_PWR_CON) | (1 << 0);
	mmio_write_32(SPM_MP2_CPUSYS_PWR_CON, tmp);	/* Release pwr_rst_b */

	tmp = mmio_read_32(WDT_SWSYSRST) | 0x88000800;
	mmio_write_32(WDT_SWSYSRST, tmp);

	tmp = mmio_read_32(SPM_CPU_EXT_BUCK_ISO) & ~(0x3);
	mmio_write_32(SPM_CPU_EXT_BUCK_ISO, tmp);	/* ISOLATE */

	tmp = (mmio_read_32(WDT_SWSYSRST) & ~(0x0800)) | 0x88000000;
	mmio_write_32(WDT_SWSYSRST, tmp);
#endif
/* ?????????????????????????????????????????????????????????????????????????????????? */
	/* TOP */
	tmp =
	    mmio_read_32(addr_spmc_ctl) & ~(B_SW_PD | B_SW_LOGIC_PDB | B_SW_LOGIC_PRE1_PDB |
					    B_SW_LOGIC_PRE2_PDB | B_SW_SRAM_SLEEPB |
					    B_SW_SRAM_ISOINTB);
	tmp |= (B_SW_ISO | B_SW_HOT_PLUG_RESET);
	mmio_write_32(addr_spmc_ctl, tmp);

	tmp = mmio_read_32(addr_spmc_ctl) | B_SW_FSM_OVERRIDE;
	mmio_write_32(addr_spmc_ctl, tmp);
	tmp = mmio_read_32(addr_spmc_ctl);
	PRINTF_SPMC("set B_SW_FSM_OVERRIDE:%x\n", tmp);
	result = 0;

	for (retry = 10; retry > 0; retry--) {

		if ((mmio_read_32(addr_spmc_sta) & B_FSM_STATE_OUT_MASK) == B_FSM_ON) {
			result = 1;
			break;
		}
		udelay(RETRY_TIME_USEC);
	}
	big_spmc_info();
	if (!result) {
		PRINTF_SPMC("[ERROR]B_FSM_ON fail!!!!!!!!!\n");/* TO=timeout */
		return -4;/* timeout */
	}
	return 0;
}

/**
 * HW-API: BigSPMCSwPwrOn
 * selectL 0x1: core 1, 0x2:core 2, 0xF:all cores , 0x10:TOP
 */
/*
"1. if Select invalid, exit/return error -1
2. if Select = CPU0, CPU1, or ALLCPU, read TOP SPMC Status. If not powered on, Power  on TOP first
For each Select:
3. Read sw_fsm_override & sw_pwr_on_override_en & pwr state
4. If already power on, continue to next Select.
5. If sw_fsm_override=0
    1. if sw_pwr_on_override=0
	 .1. Write sw_pwr_on_override_en = 1
    2. write sw_pwr_on=1
6. else
	Sequence:
	1. sram_pd gray sequencing from 0 -> 11000
	2. sw_logic_pre1_pdb = 1
	3. Poll/wait logic_pre1_pdbo_all_on_ack = 1
	4. sw_logic_pre2_pdb = 1
	5. Poll/wait logic_pre2_pdbo_all_on_ack = 1
	6. sw_logic_pdb = 1
	7. Poll/wait logic_pdbo_all_on_ack = 1
	8. sw_sram_sleepb = 1
	9. sw_iso = 0, sw_sram_isointb = 1
	10. sw_hot_plug_reset = 0"


char sw_pd_cmd[32]={0x01,0x03,0x02,0x06,0x07,0x05,0x04,0x0c,0x0d,0x0f,
		    0x0e,0x0a,0x0b,0x09,0x08,0x18,0x19,0x1b,0x1a,0x1e,
		    0x1f,0x1d,0x1c,0x14,0x15,0x17,0x16,0x12,0x13,0x11,
		    0x10,0x30};
*/
int big_sw_on_seq(int select)
{
	unsigned int tmp, i;
	unsigned int nb_srampd;
	unsigned int addr_spmc_ctl, addr_spmc_sta;
//	unsigned int SPM_MPx_CPUx_PWR_CON;
	PRINTF_SPMC(">>>>>>>> [%s] select=0x%x\n", __func__, select);
	switch (select) {
	case 0:
		addr_spmc_ctl = CPUSYS2_CPU0_SPMC_CTL;
		addr_spmc_sta = CPUSYS2_CPU0_SPMC_STA;
//		SPM_MPx_CPUx_PWR_CON = MP2_CPU0_PWR_CON;
		nb_srampd = 17;//0x40;	/* TODO: */
		break;
	case 1:
		addr_spmc_ctl = CPUSYS2_CPU1_SPMC_CTL;
		addr_spmc_sta = CPUSYS2_CPU1_SPMC_STA;
//		SPM_MPx_CPUx_PWR_CON = MP2_CPU1_PWR_CON;
		nb_srampd = 17;//0x40;	/* TODO:16 BITS[15:0] */
		break;
	case 0x30:		/* B cluster */
		addr_spmc_ctl = MP2_CPUTOP_SPMC_CTL;
		addr_spmc_sta = MP2_CPUTOP_SPMC_STA;
//		SPM_MPx_CPUx_PWR_CON = MP2_CPUTOP_PWR_CON;
		nb_srampd = 51;//0x40;	/* TODO:50BITS [49:0] 51*/
		break;
	default:
		ERROR("[ATF]: %s() CPU%d not exists\n", __func__, (int)select);
		assert(0);
	}

	if (!addr_spmc_ctl) {
		PRINTF_SPMC("[ERROR] addr_spmc_ctl is NULL...");
		return -1;
	}
	if (mmio_read_32(addr_spmc_sta) & B_SW_LOGIC_PDBO_ALL_ON_ACK) {
		PRINTF_SPMC("[ERROR] B_SW_LOGIC_PDBO_ALL_ON_ACK=1 turn on already...");
		return 0;	/* turn on already */
	}
#if 1
	PRINTF_SPMC("SwOn ChkOR\n");

	if (!(mmio_read_32(addr_spmc_ctl) & B_SW_FSM_OVERRIDE)) {
		tmp = mmio_read_32(addr_spmc_ctl);
		if (!(tmp & B_SW_PWR_ON_OVERRIDE_EN)) {
			tmp |= B_SW_PWR_ON_OVERRIDE_EN;
			mmio_write_32(addr_spmc_ctl, tmp);
		}
		tmp = mmio_read_32(addr_spmc_ctl) | B_SW_PWR_ON;
		mmio_write_32(addr_spmc_ctl, tmp);
	}

/* ?????????????????????????????????????????????????????????????????????????????????? */
/* PRINTF_SPMC("SwOn HPRst\n"); */
/* tmp = mmio_read_32(addr_spmc_ctl) | B_SW_HOT_PLUG_RESET; */
/* mmio_write_32(addr_spmc_ctl,tmp); */

	tmp = mmio_read_32(addr_spmc_ctl) | B_SW_PWR_ON;
	mmio_write_32(addr_spmc_ctl, tmp);
	PRINTF_SPMC("SwOn ReRst&ClkDis\n");

	tmp = mmio_read_32(addr_spmc_ctl);
	PRINTF_SPMC("SwOn OrEn\n");
	tmp |= B_SW_PWR_ON_OVERRIDE_EN;
	mmio_write_32(addr_spmc_ctl, tmp);
	PRINTF_SPMC("SwOn PwrOn\n");
#endif

/*
	mmio_write_32(SPM_MPx_CPUx_PWR_CON,  mmio_read_32(SPM_MPx_CPUx_PWR_CON) & ~PWR_ON_2ND);
	mmio_write_32(SPM_MPx_CPUx_PWR_CON,  mmio_read_32(SPM_MPx_CPUx_PWR_CON) | PWR_RST_B);
    if((select & 0xF0)!=0)//cluster
		mmio_write_32(SPM_MPx_CPUx_PWR_CON,  mmio_read_32(SPM_MPx_CPUx_PWR_CON) & ~PWR_CLK_DIS);
*/

	/* power on SRAM */
	/*
	   tmp = mmio_read_32(addr_spmc_ctl) & ~B_SW_PD;

	   PRINTF_SPMC("SwOn SRAM\n");
	   for(i=0;i<nb_srampd;i++)
	   {
	   tmp = tmp | (sw_pd_cmd[i]<<B_SW_PD_OFFSET);
	   mmio_write_32(addr_spmc_ctl,tmp);
	   }
	 */
	PRINTF_SPMC("B_SW_PD[23:18]=1\n");
	for (i = 0; i < nb_srampd; i++) {
		tmp = (i >> 1) ^ i;	/*binaryToGray */
		/* PRINTF_SPMC("binaryToGray[%d]=0x%x\n", i, tmp); */
		mmio_write_32(addr_spmc_ctl,(mmio_read_32(addr_spmc_ctl) & ~B_SW_PD) | tmp << B_SW_PD_OFFSET);
	}

	PRINTF_SPMC("B_SW_LOGIC_PRE1_PDB=1\n");
	tmp = mmio_read_32(addr_spmc_ctl) | B_SW_LOGIC_PRE1_PDB;
	mmio_write_32(addr_spmc_ctl, tmp);
	PRINTF_SPMC("wait B_SW_LOGIC_PRE1_PDBO_ALL_ON_ACK==1\n");
	while (!(mmio_read_32(addr_spmc_sta) & B_SW_LOGIC_PRE1_PDBO_ALL_ON_ACK));	/* wait for 1 */
	big_spmc_info();
	PRINTF_SPMC("B_SW_LOGIC_PRE2_PDB=1\n");
	tmp = mmio_read_32(addr_spmc_ctl) | B_SW_LOGIC_PRE2_PDB;
	mmio_write_32(addr_spmc_ctl, tmp);
	PRINTF_SPMC("wait B_SW_LOGIC_PRE2_PDBO_ALL_ON_ACK==1\n");
	while (!(mmio_read_32(addr_spmc_sta) & B_SW_LOGIC_PRE2_PDBO_ALL_ON_ACK));	/* wait for 1 */
	big_spmc_info();
	PRINTF_SPMC("B_SW_LOGIC_PDB=1\n");
	tmp = mmio_read_32(addr_spmc_ctl) | B_SW_LOGIC_PDB;
	mmio_write_32(addr_spmc_ctl, tmp);
	big_spmc_info();
	PRINTF_SPMC("wait B_SW_LOGIC_PDBO_ALL_ON_ACK==1\n");
	while (!(mmio_read_32(addr_spmc_sta) & B_SW_LOGIC_PDBO_ALL_ON_ACK));	/* wait for 1 */
	big_spmc_info();
	PRINTF_SPMC("B_SW_SRAM_SLEEPB[17:12]=1\n");
	/*
	   tmp = mmio_read_32(addr_spmc_ctl) & ~B_SW_SRAM_SLEEPB;
	   for(i=0;i<nb_srampd;i++){
	   tmp = tmp | (sw_pd_cmd[i]<<B_SW_SRAM_SLEEPB_OFFSET);
	   mmio_write_32(addr_spmc_ctl,tmp);
	   }
	 */
	for (i = 0; i < nb_srampd; i++) {
		tmp = (i >> 1) ^ i;	/*binaryToGray */
		/* PRINTF_SPMC("binaryToGray[%d]=0x%x\n", i, tmp); */
		mmio_write_32(addr_spmc_ctl,
			      (mmio_read_32(addr_spmc_ctl) & ~B_SW_SRAM_SLEEPB) | tmp <<
			      B_SW_SRAM_SLEEPB_OFFSET);
	}

	big_spmc_info();
	PRINTF_SPMC("B_SW_ISO=0\n");
	tmp = mmio_read_32(addr_spmc_ctl) & ~B_SW_ISO;
	PRINTF_SPMC("B_SW_SRAM_ISOINTB=1\n");
	tmp |= B_SW_SRAM_ISOINTB;
	mmio_write_32(addr_spmc_ctl, tmp);
	big_spmc_info();
/*  */
/* BIG do not use it
	PRINTF_SPMC("SwOn ck_iso\n");
	tmp = mmio_read_32(addr_spmc_ctl) & ~B_SW_CKISO;
	mmio_write_32(addr_spmc_ctl, tmp);*/
/* BIG do not use it
	PRINTF_SPMC("SwOn clk_dis\n");
	tmp = mmio_read_32(addr_spmc_ctl) & ~B_SW_CLK_DIS;
	mmio_write_32(addr_spmc_ctl, tmp);*/

	PRINTF_SPMC("B_SW_HOT_PLUG_RESET=0\n");
	tmp = mmio_read_32(addr_spmc_ctl) & ~B_SW_HOT_PLUG_RESET;
	mmio_write_32(addr_spmc_ctl, tmp);
	big_spmc_info();

	if ((select & 0xF0) != 0) {	/*cluster */
		unsigned int mpx_mask, MPx_AXI_CONFIG;

#if 0
		unsigned int MPx_ARMPLL_CON0;

		PRINTF_SPMC("SwOn EnPLL\n");
		if (select == 0x10)
			MPx_ARMPLL_CON0 = ARMPLL_LL_CON0;
		else
			MPx_ARMPLL_CON0 = ARMPLL_L_CON0;

		tmp = mmio_read_32(MPx_ARMPLL_CON0) | (1 << 0);
		mmio_write_32(MPx_ARMPLL_CON0, tmp);	/*ARMPLL_CON0[0],  Enable PLL */
		udelay(20);
		PRINTF_SPMC("SwOn SelPLL\n");
#endif
		MPx_AXI_CONFIG = MP2_AXI_CONFIG;
		mpx_mask = (1 << IDX_PROTECT_ICC2_CACTIVE) |
			    (1 << IDX_PROTECT_ICD2_CACTIVE) |
			    (1 << IDX_PROTECT_MP2_CACTIVE) | (1 << IDX_PROTECT_L2C2_CACTIVE);
		PRINTF_SPMC("MPx_AXI_CONFIG=%x\n", MPx_AXI_CONFIG);
		/*        mmio_write_32(TOP_CKMUXSEL,  tmp); //pll_div_mux1_sel = 01 = pll clock */

/* ################ BUS PROTECT DISABLE and SNOOP/DVM ENABLE################ */
		/* TINFO="Release bus protect" */
		/* TINFO="Release ADB pwrdnreqn for Cluster 0" */
		/* mmio_write_32(INFRA_TOPAXI_PROTECTEN_1, mmio_read_32(INFRA_TOPAXI_PROTECTEN_1) & ~mpx_mask); */
		mmio_write_32(MCUSYS_PROTECTEN_CLR, mpx_mask);
		PRINTF_SPMC("MCUSYS_PROTECTEN_CLR_0x%x=0x%x\n",
			    MCUSYS_PROTECTEN_CLR,
			    mmio_read_32(MCUSYS_PROTECTEN_CLR));

		/* TINFO="Wait ADB ~pwrdnreqn for Cluster 0" */
		while ((mmio_read_32(MCUSYS_PROTECTEN_STA1) & mpx_mask) != 0);
		PRINTF_SPMC("MCUSYS_PROTECTEN_STA1_0x%x=0x%x\n", MCUSYS_PROTECTEN_STA1,
			    mmio_read_32(MCUSYS_PROTECTEN_STA1));

#if SPMC_DVT			/* by callee in plat_affinst_off() */
/*	Program MP<n>_AXI_CONFIG acinactm to 0*/
		mmio_write_32(MPx_AXI_CONFIG,
				      (mmio_read_32(MPx_AXI_CONFIG) & ~MP2_AXI_CONFIG_acinactm)& ~MP2_AXI_CONFIG_ainacts);
		PRINTF_SPMC("MPx_AXI_CONFIG_0x%x=0x%x\n", MPx_AXI_CONFIG,
			    mmio_read_32(MPx_AXI_CONFIG));
/*	Program MCSI-A Slave Control Register (MP0:slave0,  MP1:slave1) */
/*to enable snoop/DVM function for related slave port through ACE(0x10390000) or APB(0x10200000) interface*/
/*	Polling CCI Status register through ACE interface (0x1039000C[0]) returns to 0*/
/*		mmio_write_32(MPx_SNOOP_CTRL, mmio_read_32(MPx_SNOOP_CTRL) | MPx_SNOOP_ENABLE);*/
/*	  PRINTF_SPMC("MPx_SNOOP_CTRL_0x%x=0x%x\n", MPx_SNOOP_CTRL, mmio_read_32(MPx_SNOOP_CTRL));*/
		/*mmio_write_32(0x10394000, mmio_read_32(0x10394000) | MPx_SNOOP_ENABLE);
		   mmio_write_32(0x10395000, mmio_read_32(0x10395000) | MPx_SNOOP_ENABLE); */
		mmio_write_32(MP2_SNOOP_CTRL, mmio_read_32(MP2_SNOOP_CTRL) | MPx_SNOOP_ENABLE);
		PRINTF_SPMC("MPx_SNOOP_CTRL%x=0x%x\n", MP2_SNOOP_CTRL,
			    mmio_read_32(MP2_SNOOP_CTRL));
		PRINTF_SPMC("MPx_SNOOP_STATUS_0x%x=0x%x\n", MPx_SNOOP_STATUS,
			    mmio_read_32(MPx_SNOOP_STATUS));
		while ((mmio_read_32(MPx_SNOOP_STATUS) & (1 << 31)) == (1 << 31));
		/* mcsib_sw_workaround_main(); */
		PRINTF_SPMC("MPx_SNOOP_STATUS_0x%x=0x%x\n", MPx_SNOOP_STATUS,
			    mmio_read_32(MPx_SNOOP_STATUS));
#endif
/* ################ BUS PROTECT DISABLE and SNOOP/DVM ENABLE################ */
	}
	return 0;
}

int big_spmc_sw_pwr_on(int select)
{
	/* power on a core before power on the TOP, we power on TOP automatically */
	/* extern void bl31_on_entrypoint(void); */
	PRINTF_SPMC("SwPsqOn Sel:%d on:%x\n", select, big_on);

	if (big_on & (1 << select))
		return PSCI_E_SUCCESS;

	switch (select) {
	case 0x0:
	case 0x1:
		if ((big_on & 0x3) == 0) {
#if 0
			big_spmc_info();
#endif
			big_spmc_sw_pwr_seq_en(0x30);
#if 0
			udelay(SPMC_DVT_UDELAY);
			big_spmc_info();
#endif

			big_sw_on_seq(0x30);	/* power on TOP */

#if 0
			udelay(SPMC_DVT_UDELAY);
			big_spmc_info();
#endif
		}
#if 0
		mmio_write_32(MP2_CPUCFG, 0xf << 16);	/* set BIG poweup on AARCH64 */
		mmio_write_32(MP2_MISC_CONFIG_BOOT_ADDR_L(select), (unsigned long)sec_entrypoint);
		mmio_write_32(MP2_MISC_CONFIG_BOOT_ADDR_H(select), 0);
		PRINTF_SPMC("mt_on_2, entry H:%x L:%x\n",
			    mmio_read_32(MP2_MISC_CONFIG_BOOT_ADDR_H(select)),
			    mmio_read_32(MP2_MISC_CONFIG_BOOT_ADDR_L(select)));
/* PRINTF_SPMC("Write 0x10208008=%d\n",8+add_idx); */
/* mmio_write_32(0x10208008, 8+add_idx);//discussed with Scott */
#endif
#if 0
		udelay(SPMC_DVT_UDELAY);
		big_spmc_info();
#endif

		big_spmc_sw_pwr_seq_en(select);

#if 0
		udelay(SPMC_DVT_UDELAY);
		big_spmc_info();
#endif

		big_sw_on_seq(select);

#if 0
		udelay(SPMC_DVT_UDELAY);
		big_spmc_info();
#endif

		big_on = big_on | (1 << select);
#if SPMC_SPARK2
		if (select == 0x0)
			big_spark2_setldo(0, 0);
		big_spark2_core(select + 8, 1);
#endif
		break;
	case 0xf:
		big_spmc_sw_pwr_seq_en(0x30);
		big_spmc_info();
		big_sw_on_seq(0x30);	/* power on TOP */
		big_spmc_info();
		big_sw_on_seq(0x1);
		big_spmc_info();
#if SPMC_SPARK2
		big_spark2_setldo(0, 0);
		big_spark2_core(8, 1);
#endif
		big_sw_on_seq(0x2);
		big_spmc_info();
#if SPMC_SPARK2
		big_spark2_core(9, 1);
#endif
		break;
	case 0x30:
		big_spmc_sw_pwr_seq_en(0x30);
		big_spmc_info();
		big_sw_on_seq(0x30);
		big_spmc_info();
		break;
	default:
		return -1;
	}
	return PSCI_E_SUCCESS;
}

/**
 * BigSPMCSwPwrOff
 * selectL 0x1: core 1, 0x2:core 2, 0xF:all cores , 0x10:TOP
 */
/*
"1. if Select invalid, exit/return error -1
	ForSelect: (Do CPU's before TOP)
	2. Read sw_fsm_override & sw_pwr_on_override_en & pwr state
	3. If already power off, continue to next Select.
	4. If sw_fsm_override=0
		1. if sw_pwr_on_override=0
		    1. Write sw_pwr_on_override_en = 1
		2. write sw_pwr_on=0
	5. else
       Sequence:
		1. sw_iso=1, sw_sram_isointb=0
		2. sw_logic*_pdb=0, sw_sram_sleepb=0, sw_pd=6??h00
		3. Poll/wait for logic_*pdb_all_on_ack=0 & logic_pdbo_all_off_ack=1
		4. sw_hot_plug_reset=1"

*/
void big_sw_off_seq(int select)
{
	unsigned int tmp;
	unsigned int addr_spmc_ctl, addr_spmc_sta;
//	unsigned int nb_srampd;
	PRINTF_SPMC(">>>>>>>> [%s] select=0x%x\n", __func__, select);
	switch (select) {
	case 0:
		addr_spmc_ctl = CPUSYS2_CPU0_SPMC_CTL;
		addr_spmc_sta = CPUSYS2_CPU0_SPMC_STA;
//		nb_srampd = 16;
		break;
	case 1:
		addr_spmc_ctl = CPUSYS2_CPU1_SPMC_CTL;
		addr_spmc_sta = CPUSYS2_CPU1_SPMC_STA;
//		nb_srampd = 16;
		break;
	case 0x30:		/* LL cluster */
		addr_spmc_ctl = MP2_CPUTOP_SPMC_CTL;
		addr_spmc_sta = MP2_CPUTOP_SPMC_STA;
//		nb_srampd = 32;
		break;
	default:
		ERROR("[ATF]: %s() CPU%d not exists\n", __func__, (int)select);
		assert(0);
	}

	if (!(mmio_read_32(addr_spmc_sta) & B_FSM_STATE_OUT_MASK))
		return;		/* turn off already */
	if (!(mmio_read_32(addr_spmc_ctl) & B_SW_FSM_OVERRIDE)) {
		tmp = mmio_read_32(addr_spmc_ctl);
		if (!(tmp & B_SW_PWR_ON_OVERRIDE_EN)) {
			tmp |= B_SW_PWR_ON_OVERRIDE_EN;
			mmio_write_32(addr_spmc_ctl, tmp);
		}
		tmp &= ~B_SW_PWR_ON;
		mmio_write_32(addr_spmc_ctl, tmp);
	}
/* TODO:Turn off the clock */
#if 0
	if ((select & 0xF0) != 0) {	/* cluster */
		PRINTF_SPMC("Wait CL %x WFI\n", select);
		while ((mmio_read_32(CPU_IDLE_STA) & (1 << 26)) != (1 << 26));
		PRINTF_SPMC("CL %x is in WFI\n");
		tmp = mmio_read_32(TOP_CKMUXSEL) & ~(1 << 0);
		mmio_write_32(TOP_CKMUXSEL, tmp);	/* pll_div_mux1_sel = 26MHZ */
		tmp = mmio_read_32(ARMPLL_CON0) & ~(1 << 0);
		mmio_write_32(ARMPLL_CON0, tmp);	/* ARMPLL_CON0[0], Disable PLL */

	} else {
		PRINTF_SPMC("Wait CPU WFI mask:%x\n", select);
		while (!(mmio_read_32(CPU_IDLE_STA) & (1 << (20 + select))));
		PRINTF_SPMC("CPU is in WFI, mask:%x\n", select);
	}
#endif
		if ((select & 0xF0) != 0) {	/*cluster */
			unsigned int /*MPx_ARMPLL_CON0,*/ MPx_IDLE_STA;
			unsigned int MPx_AXI_CONFIG, /*MPx_CA7_MISC_CONFIG,*/ mpx_mask;

			MPx_IDLE_STA = 1 << 21;
			/*MPx_ARMPLL_CON0 = ARMPLL_L_CON0;*/
//			MPx_CA7_MISC_CONFIG = MP1_CA7_MISC_CONFIG;
			MPx_AXI_CONFIG = MP2_AXI_CONFIG;
			mpx_mask = (1 << IDX_PROTECT_ICC2_CACTIVE) |
				    (1 << IDX_PROTECT_ICD2_CACTIVE) |
				    (1 << IDX_PROTECT_MP2_CACTIVE) |
				    (1 << IDX_PROTECT_L2C2_CACTIVE);

			PRINTF_SPMC("Wait CL %x WFI\n", select);
			/*while ((mmio_read_32(CPU_IDLE_STA) & MPx_IDLE_STA) != MPx_IDLE_STA); */
			PRINTF_SPMC("CL %x is in WFI\n", select);
			PRINTF_SPMC("CPU_IDLE_STA=0x%x\n", MPx_IDLE_STA);
			PRINTF_SPMC("MPx_AXI_CONFIG=%x\n", MPx_AXI_CONFIG);
			/*PRINTF_SPMC("MPx_ARMPLL_CON0=%x\n", MPx_ARMPLL_CON0);*/
#if SPMC_DVT			/* by callee in plat_affinst_off() */
		/* TINFO="Start to turn off MP0_CPUTOP" */
/* ################ BUS PROTECT ENABLE and SNOOP/DVM DISABLE ################ */
		mmio_write_32(MP2_SNOOP_CTRL, mmio_read_32(MP2_SNOOP_CTRL) & ~MPx_SNOOP_ENABLE);
		/* mmio_write_32(0x10394000,  mmio_read_32(0x10394000) & ~MPx_SNOOP_ENABLE); */
		/* mmio_write_32(0x10395000,  mmio_read_32(0x10395000) & ~MPx_SNOOP_ENABLE); */
		while ((mmio_read_32(MPx_SNOOP_STATUS) & (1 << 31)) == (1 << 31));
		/* mcsib_sw_workaround_main(); */
		/*      Program MP<n>_AXI_CONFIG acinactm to 1 */
		/*      Wait mp<n>_STANDBYWFIL2 to high */
		/* TINFO="Set acinactm = 1" */
		mmio_write_32(MPx_AXI_CONFIG, mmio_read_32(MPx_AXI_CONFIG) | MP2_AXI_CONFIG_acinactm | MP2_AXI_CONFIG_ainacts);
#endif
		/* TINFO="Wait STANDBYWFIL2 for Cluster 0" */
		PRINTF_SPMC("CPU_IDLE_STA_0x%x=0x%x\n", CPU_IDLE_STA, mmio_read_32(CPU_IDLE_STA));
		while (!(mmio_read_32(CPU_IDLE_STA) & MP2_STANDBYWFI));

		/* TINFO="Set ADB pwrdnreqn for Cluster 0" */
		/* mmio_write_32(INFRA_TOPAXI_PROTECTEN_1,  mmio_read_32(INFRA_TOPAXI_PROTECTEN_1)|mpx_mask); */
		mmio_write_32(MCUSYS_PROTECTEN_SET, mpx_mask);
		/* TINFO="Wait ADB pwrdnreqn for Cluster 0" */
		while ((mmio_read_32(MCUSYS_PROTECTEN_STA1) & mpx_mask) != mpx_mask);
/* ################ BUS PROTECT ENABLE and SNOOP/DVM DISABLE ################ */

#if 0
			if (select == 0x10)
				tmp = mmio_read_32(TOP_CKMUXSEL) & ~armpll_mux1_sel_sml_mask;
			else
				tmp = mmio_read_32(TOP_CKMUXSEL) & ~armpll_mux1_sel_big_mask;

			mmio_write_32(TOP_CKMUXSEL, tmp);	/*pll_div_mux1_sel = 26MHZ */

			tmp = mmio_read_32(MPx_ARMPLL_CON0) & ~(1 << 0);
			mmio_write_32(MPx_ARMPLL_CON0, tmp);	/*ARMPLL_CON0[0],  Disable PLL */
#endif
		} else {
			/*wait WFE */
#if SPMC_DVT
			mmio_write_32(CA15M_MON_SEL, (mmio_read_32(CA15M_MON_SEL) & 0xFF) | 0x1b);
			while (!(mmio_read_32(MP2_CA15M_MON_L) & 1<<(4+select)));
#else
			PRINTF_SPMC("Wait CPU WFI mask:%x\n", select);
			while (!(mmio_read_32(CPU_IDLE_STA) & (1 << (12 + select))));
			PRINTF_SPMC("CPU is in WFI,  mask:%x\n", select);

#endif
		}

/* BIG do not use it
	tmp = mmio_read_32(addr_spmc_ctl) | B_SW_CKISO;
	mmio_write_32(addr_spmc_ctl, tmp); */

	tmp = mmio_read_32(addr_spmc_ctl) | B_SW_ISO;
	tmp &= ~B_SW_SRAM_ISOINTB;
	mmio_write_32(addr_spmc_ctl, tmp);
/* BIG do not use it
	tmp = mmio_read_32(addr_spmc_ctl) | B_SW_CLK_DIS;
	mmio_write_32(addr_spmc_ctl, tmp); */


	tmp =
	    mmio_read_32(addr_spmc_ctl) & ~(B_SW_LOGIC_PRE1_PDB | B_SW_LOGIC_PRE2_PDB |
					    B_SW_LOGIC_PDB | B_SW_SRAM_SLEEPB | B_SW_PD);
	mmio_write_32(addr_spmc_ctl, tmp);

	while ((mmio_read_32(addr_spmc_sta) & B_SW_LOGIC_PRE1_PDBO_ALL_ON_ACK));	/* wait for 0 */
	while ((mmio_read_32(addr_spmc_sta) & B_SW_LOGIC_PRE2_PDBO_ALL_ON_ACK));	/* wait for 0 */
	while (!(mmio_read_32(addr_spmc_sta) & B_SW_LOGIC_PDBO_ALL_OFF_ACK));	/* wait for 1 */

	tmp = mmio_read_32(addr_spmc_ctl) | B_SW_HOT_PLUG_RESET;
	mmio_write_32(addr_spmc_ctl, tmp);
#if 0
	if (select == 0x10) {
/* ?????????????????????????????????????????????????????????????????????????????????? */
		tmp = mmio_read_32(SPM_MP2_CPUSYS_PWR_CON) & ~(1 << 0);
		mmio_write_32(SPM_MP2_CPUSYS_PWR_CON, tmp);	/* Release pwr_rst_b */

		tmp = mmio_read_32(SPM_MP2_CPUSYS_PWR_CON) | (1 << 4);
		mmio_write_32(SPM_MP2_CPUSYS_PWR_CON, tmp);	/* Release */

		tmp = mmio_read_32(SPM_CPU_EXT_BUCK_ISO) | (0x2);
		mmio_write_32(SPM_CPU_EXT_BUCK_ISO, tmp);	/* ISOLATE */
/* ?????????????????????????????????????????????????????????????????????????????????? */
	}
#endif

}

int big_spmc_sw_pwr_off(int select)
{
	PRINTF_SPMC(">>>>>>>> [%s] select=0x%x\n", __func__, select);
	switch (select) {
	case 0x0:
	case 0x1:

#if 0
		udelay(SPMC_DVT_UDELAY);
		big_spmc_info();
#endif

		big_sw_off_seq(select);

#if 0
		udelay(SPMC_DVT_UDELAY);
		big_spmc_info();
#endif

		big_on &= ~(1 << select);
		PRINTF_SPMC("%s big_on:%x\n", __func__, big_on);
		if (!big_on) {
			big_sw_off_seq(0x30);
#if 0
			udelay(SPMC_DVT_UDELAY);
			big_spmc_info();
#endif
		}
		break;
	case 0xf:
		big_sw_off_seq(0x1);
		big_sw_off_seq(0x2);
		break;
	case 0x10:
		big_sw_off_seq(0x1);
		big_sw_off_seq(0x2);
		big_sw_off_seq(0x30);
		break;
	default:
		return -1;
	}
	return 0;

}

/**
 * BigSPMCSwPwrCntrlDisable
 * select : 0x1: core0, 0x2:core1 0xF:all 0x10:TOP
 * return : 0: Ok -1: Invalid Select value -2: Invalid request.

 */
/*
"1. if Select invalid, exit/return error -1
2. For Select, read sw_fsm_override.
3. If sw_fsm_override = 1
    1. return error -2
4. else
    1. Write all sw_pwr_on_override_en = 0"

*/

int big_spmc_sw_pwr_cntrl_disable(int select)
{
	unsigned int tmp;
	unsigned int addr_spmc_ctl;

	switch (select) {
	case 0:
		addr_spmc_ctl = CPUSYS2_CPU0_SPMC_CTL;
		break;
	case 1:
		addr_spmc_ctl = CPUSYS2_CPU1_SPMC_CTL;
		break;
	case 0x10:		/* LL cluster */
		addr_spmc_ctl = MP2_CPUTOP_SPMC_CTL;
		break;
	default:
		ERROR("[ATF]: %s() CPU%d not exists\n", __func__, (int)select);
		assert(0);
	}

	switch (select) {
	case 0x0:
	case 0x1:
	case 0x10:
		if (mmio_read_32(addr_spmc_ctl) & B_SW_FSM_OVERRIDE) {
			return -2;
		} else {
			tmp = mmio_read_32(addr_spmc_ctl) & ~B_SW_PWR_ON_OVERRIDE_EN;
			mmio_write_32(addr_spmc_ctl, tmp);
		}
		break;
/*
	case 0xf:
	    if((mmio_read_32(PTP3_CPU0_SPMC) & SW_FSM_OVERRIDE)||
		(mmio_read_32(PTP3_CPU1_SPMC) & SW_FSM_OVERRIDE)){
		return -2;
	    }
	    tmp = mmio_read_32(PTP3_CPU0_SPMC) & ~SW_PWR_ON_OVERRIDE_EN;
	    mmio_write_32(PTP3_CPU0_SPMC, tmp);
	    tmp = mmio_read_32(PTP3_CPU1_SPMC) & ~SW_PWR_ON_OVERRIDE_EN;
	    mmio_write_32(PTP3_CPU1_SPMC, tmp);
	    break;
	case 0x10:
	    if((mmio_read_32(PTP3_CPU0_SPMC) & SW_FSM_OVERRIDE)||
		(mmio_read_32(PTP3_CPU1_SPMC) & SW_FSM_OVERRIDE)){
		return -2;
	    }
	    tmp = mmio_read_32(PTP3_CPU0_SPMC) & ~SW_PWR_ON_OVERRIDE_EN;
	    mmio_write_32(PTP3_CPU0_SPMC, tmp);
	    tmp = mmio_read_32(PTP3_CPU1_SPMC) & ~SW_PWR_ON_OVERRIDE_EN;
	    mmio_write_32(PTP3_CPU1_SPMC, tmp);
	    tmp = mmio_read_32(PTP3_CPUTOP_SPMC) & ~SW_PWR_ON_OVERRIDE_EN;
	    mmio_write_32(PTP3_CPUTOP_SPMC, tmp);
	    break;
*/
	default:
		return -1;
	}
	return 0;
}
#endif

#endif				/* SPMC_SW_MODE */


/*
"1. Convert mVolts to 6 bit value vretcntrl (exit/return -1 if out of range)
2. Set sparkvretcntrl
3. Write for all CPUs,  sw_spark_en = 1"

*/
/*switch 0:off 1:on*/
void big_spark2_setldo(unsigned int cpu0_amuxsel, unsigned int cpu1_amuxsel)
{
	unsigned int tmp;
	unsigned int sparkvretcntrl = 0x3f;

	PRINTF_SPMC("%s sparkvretcntrl=%x", __func__, sparkvretcntrl);
	if (cpu0_amuxsel > 15 || cpu1_amuxsel > 15)
		return;

	tmp = cpu1_amuxsel << 10 | cpu0_amuxsel << 6 | sparkvretcntrl;
	mmio_write_32(SPARK2LDO, tmp);
}

int big_spark2_core(unsigned int linear_id, unsigned int sw)
{
	unsigned int tmp;

	if (sw > 1 || linear_id < 8 || linear_id > 9)
		return -1;
	PRINTF_SPMC("%s linear_id:%d sw:%d\n", __func__, linear_id, sw);
	if (linear_id == 9) {
		tmp = ((mmio_read_32(CPUSYS2_CPU1_SPMC_CTL) >> 1) << 1) | sw;
		PRINTF_SPMC("Write %x = %x\n", CPUSYS2_CPU1_SPMC_CTL, tmp);
		mmio_write_32(CPUSYS2_CPU1_SPMC_CTL, tmp);
	} else {
		tmp = ((mmio_read_32(CPUSYS2_CPU0_SPMC_CTL) >> 1) << 1) | sw;
		PRINTF_SPMC("Write %x = %x\n", CPUSYS2_CPU0_SPMC_CTL, tmp);
		mmio_write_32(CPUSYS2_CPU0_SPMC_CTL, tmp);
	}
	return 0;
}

int little_spark2_setldo(unsigned int linear_id)
{
	unsigned long long base_vret;
	unsigned int offset, tmp, sparkvretcntrl = 0x34;	/* 34=.5 3f=.6 */

	if (linear_id > 9)
		return -1;
	PRINTF_SPMC("%s sparkvretcntrl=%x", __func__, sparkvretcntrl);
	if (linear_id < 4) {
		offset = linear_id;
		base_vret = CPUSYS0_SPARKVRETCNTRL;
		tmp = (mmio_read_32(base_vret) & ~((0x3f) << (offset << 3))) | (sparkvretcntrl << (offset << 3));
		mmio_write_32(base_vret, tmp);
	} else if (linear_id < 8) {
		offset = linear_id - 4;
		base_vret = CPUSYS1_SPARKVRETCNTRL;
		tmp = (mmio_read_32(base_vret) & ~((0x3f) << (offset << 3))) | (sparkvretcntrl << (offset << 3));
		mmio_write_32(base_vret, tmp);
	} else {
		tmp = mmio_read_32(SPARK2LDO);
		tmp &= ~0x3F;
		tmp |= sparkvretcntrl;
		mmio_write_32(SPARK2LDO, tmp);
		PRINTF_SPMC("SPARK2LDO%x=0x%x\n", SPARK2LDO, mmio_read_32(SPARK2LDO));
		}
	return 0;
}

int little_spark2_core_enable(unsigned int linear_id, unsigned int sw)
{
	unsigned int CPUSYSx_CPUx_SPMC_CTL, tmp;

	if (linear_id > 9 || sw > 1)
		return -1;
/*
	unsigned int offset,  tmp, CPUSYSx_CPUx_SPMC_CTL;
	unsigned long long base_ctrl;
	unsigned int base_ctrl;

	if (linear_id<4) {
		offset = linear_id;
		base_ctrl = CPUSYS0_SPARKEN;
	} else {
		offset = linear_id-4;
		base_ctrl = CPUSYS1_SPARKEN;
	}
	tmp = (mmio_read_32(base_ctrl) & ~(1<<offset)) | (sw<<offset);
	mmio_write_32(base_ctrl,  tmp);
	PRINTF_SPMC("CPUSYSx_SPARKEN%x=0x%x\n", base_ctrl, mmio_read_32(base_ctrl));

*/

	switch (linear_id) {
	case 0:
		CPUSYSx_CPUx_SPMC_CTL = CPUSYS0_CPU0_SPMC_CTL;
		break;
	case 1:
		CPUSYSx_CPUx_SPMC_CTL = CPUSYS0_CPU1_SPMC_CTL;
		break;
	case 2:
		CPUSYSx_CPUx_SPMC_CTL = CPUSYS0_CPU2_SPMC_CTL;
		break;
	case 3:
		CPUSYSx_CPUx_SPMC_CTL = CPUSYS0_CPU3_SPMC_CTL;
		break;
	case 4:
		CPUSYSx_CPUx_SPMC_CTL = CPUSYS1_CPU0_SPMC_CTL;
		break;
	case 5:
		CPUSYSx_CPUx_SPMC_CTL = CPUSYS1_CPU1_SPMC_CTL;
		break;
	case 6:
		CPUSYSx_CPUx_SPMC_CTL = CPUSYS1_CPU2_SPMC_CTL;
		break;
	case 7:
		CPUSYSx_CPUx_SPMC_CTL = CPUSYS1_CPU3_SPMC_CTL;
		break;
	case 8:
		CPUSYSx_CPUx_SPMC_CTL = CPUSYS2_CPU0_SPMC_CTL;
		break;
	case 9:
		CPUSYSx_CPUx_SPMC_CTL = CPUSYS2_CPU1_SPMC_CTL;
		break;
	default:
		ERROR("%s() CPU%d not exists\n", __func__, (int)linear_id);
		assert(0);
	}
	tmp = (mmio_read_32(CPUSYSx_CPUx_SPMC_CTL) & ~cpu_sw_spark_en) | sw;
	mmio_write_32(CPUSYSx_CPUx_SPMC_CTL, tmp);
	PRINTF_SPMC("CPUSYSx_CPUx_SPMC_CTL_0x%x=0x%x\n", CPUSYSx_CPUx_SPMC_CTL,
		    mmio_read_32(CPUSYSx_CPUx_SPMC_CTL));
	return 0;
}

void spark2_setldo_AMUXSEL(int cputop_mpx, unsigned int cpu_corex, unsigned int amuxsel,
			   unsigned int vret)
{
	unsigned int tmp;

	PRINTF_SPMC("%s CPUTOP%d cpu_corex=%x amuxsel=%x vret=%x\n", __func__,
		    cputop_mpx == CPUTOP_MP0 ? 0 : (cputop_mpx == CPUTOP_MP1 ? 1 : 2), cpu_corex, amuxsel, vret);
	assert(amuxsel < 7);

	if (cputop_mpx == CPUTOP_MP0) {
		tmp = mmio_read_32(CPUSYS0_SPARKVRETCNTRL);
		tmp &= ~(0x3F << (8 * cpu_corex));
		tmp |= vret << (8 * cpu_corex);
		mmio_write_32(CPUSYS0_SPARKVRETCNTRL, tmp);
		PRINTF_SPMC("CPUSYS0_SPARKVRETCNTRL%x=0x%x\n", CPUSYS0_SPARKVRETCNTRL,
			    mmio_read_32(CPUSYS0_SPARKVRETCNTRL));
		tmp = mmio_read_32(CPUSYS0_AMUXSEL);
		tmp &= ~(0xF << (8 * cpu_corex));
		tmp |= amuxsel << (8 * cpu_corex);
		mmio_write_32(CPUSYS0_AMUXSEL, tmp);
		PRINTF_SPMC("CPUSYS0_SPARKVRETCNTRL%x=0x%x\n", CPUSYS0_AMUXSEL,
			    mmio_read_32(CPUSYS0_AMUXSEL));
	} else if (cputop_mpx == CPUTOP_MP1) {
		tmp = mmio_read_32(CPUSYS1_SPARKVRETCNTRL);
		tmp &= ~(0x3F << (8 * cpu_corex));
		tmp |= vret << (8 * cpu_corex);
		mmio_write_32(CPUSYS1_SPARKVRETCNTRL, tmp);
		PRINTF_SPMC("CPUSYS1_SPARKVRETCNTRL%x=0x%x\n", CPUSYS1_SPARKVRETCNTRL,
			    mmio_read_32(CPUSYS1_SPARKVRETCNTRL));
		tmp = mmio_read_32(CPUSYS1_AMUXSEL);
		tmp &= ~(0xF << (8 * cpu_corex));
		tmp |= amuxsel << (8 * cpu_corex);
		mmio_write_32(CPUSYS1_AMUXSEL, tmp);
		PRINTF_SPMC("CPUSYS1_AMUXSEL%x=0x%x\n", CPUSYS1_AMUXSEL,
			    mmio_read_32(CPUSYS1_AMUXSEL));
	}
	else{
		tmp = mmio_read_32(SPARK2LDO);
		tmp &= ~0x3F;
		tmp |= vret;
		mmio_write_32(SPARK2LDO, tmp);
		PRINTF_SPMC("SPARK2LDO%x=0x%x\n", SPARK2LDO, mmio_read_32(SPARK2LDO));
		tmp = mmio_read_32(SPARK2LDO);
		tmp &= ~(0xF << (6+cpu_corex*4));
		tmp |= amuxsel << (6+cpu_corex*4);
		mmio_write_32(SPARK2LDO, tmp);
		PRINTF_SPMC("SPARK2LDO%x=0x%x\n", SPARK2LDO, mmio_read_32(SPARK2LDO));
		}

}
#endif /* require view before remove */

void mcucfg_set_bootaddr(int cluster, int cpu, int bootaddr)
{
	mmio_write_32(per_cpu(cluster, cpu, MCUCFG_BOOTADDR), bootaddr);
}

int mcucfg_get_bootaddr(int cluster, int cpu)
{
	return mmio_read_32(per_cpu(cluster, cpu, MCUCFG_BOOTADDR));
}

void mcucfg_init_archstate(int cluster, int cpu, int arm64)
{
	mmio_setbits_32(per_cluster(cluster, MCUCFG_INITARCH),
			(arm64 & 1) << (12 + cpu));
}

/**
 * Return the someone subsystem's power state.
 *
 * @subsys: the index to SPM_PWR_STATUS to query the power state
 *          of one subsystem.
 * RETURNS:
 * 0 (the subsys was powered off)
 * 1 (the subsys was powered on)
 */
int spm_get_powerstate(int subsys)
{
	int power_state, power_state2;
	unsigned state;

retry:
	state = 0;
	power_state = mmio_read_32(SPM_PWR_STATUS);
	power_state2 = mmio_read_32(SPM_PWR_STATUS_2ND);

	if (power_state & subsys)
		state++;

	if (power_state2 & subsys)
		state++;

	if (state == 2)
		return 1;
	else if (state == 0)
		return 0;

	goto retry;
}

int spm_get_cluster_powerstate(int cluster)
{
	int subsys;

	subsys = (cluster) ? SPM_PWR_STATUS_MP1_CPUTOP : SPM_PWR_STATUS_MP0_CPUTOP;

	return spm_get_powerstate(subsys);
}

int spm_get_cpu_powerstate(int cluster, int cpu)
{
	int i;

	/*
	 * Provide a quick way to specify the power state of
	 * cpu[0-3]/cpu[4-7] which are the BITS[9:12]/BITS[16:19]
	 * of the register SPM_PWR_STATUS
	 */
	i = (cluster) ? 16 : 9;
	i = 1 << (i + cpu);

	return spm_get_powerstate(i);
}


/**
 * Power on a core with specified cluster and core index
 *
 * @cluster: the cluster ID of the CPU which to be powered on
 * @cpu: the CPU ID of the CPU which to be powered on
 */
void spm_poweron_cpu(int cluster, int cpu)
{
	pr_debug("spmc: power on core %d.%d\n", cluster, cpu);

	/* TINFO="enable SPM register control" */
	mmio_write_32(SPM_POWERON_CONFIG_EN, (SPM_PROJECT_CODE << 16) | (0x1 << 0));

	/* TINFO="Set PWR_RST_B = 0" */
	mmio_clrbits_32(per_cpu(cluster, cpu, SPM_CPU_PWRCTRL), SPM_CPU_PWR_RST_B);

	/* TINFO="Set PWR_CLK_DIS = 1" */
	mmio_setbits_32(per_cpu(cluster, cpu, SPM_CPU_PWRCTRL), SPM_CPU_PWR_CLK_DIS);

	/* TINFO="Set PWR_ON = 1" */
	mmio_setbits_32(per_cpu(cluster, cpu, SPM_CPU_PWRCTRL), SPM_CPU_PWR_ON);

	/* TINFO="Delay 1us" */
	udelay(1);

	/* TINFO="Set PWR_ON_2ND = 1" */
	mmio_setbits_32(per_cpu(cluster, cpu, SPM_CPU_PWRCTRL), SPM_CPU_PWR_ON_2ND);

	/* TINFO="Wait until PWR_STATUS = 1 and PWR_STATUS_2ND = 1" */
	while (!spm_get_cpu_powerstate(cluster, cpu))
		;

	/* TINFO="Set PWR_ISO = 0" */
	mmio_clrbits_32(per_cpu(cluster, cpu, SPM_CPU_PWRCTRL), SPM_CPU_PWR_ISO);

	/* TINFO="Set SLPB_CLAMP = 0" */
	mmio_clrbits_32(per_cpu(cluster, cpu, SPM_CPU_PWRCTRL), SPM_CPU_SRAM_PD_SLPB_CLAMP);

	/* TINFO="Set SRAM_PDN = 0" */
	mmio_clrbits_32(per_cpu(cluster, cpu, SPM_CPU_PWRCTRL), SPM_CPU_SRAM_PDN);

	/* TINFO="Wait until MP0_CPU1_SRAM_PDN_ACK_BIT0 = 0" */
	while (mmio_read_32(per_cpu(cluster, cpu, SPM_CPU_PWRCTRL)) & SPM_CPU_SRAM_PDN_ACK)
		;

	/* TINFO="Delay 1us" */
	udelay(1);

	/* TINFO="Set SRAM_ISOINT_B = 1" */
	mmio_setbits_32(per_cpu(cluster, cpu, SPM_CPU_PWRCTRL), SPM_CPU_SRAM_ISOINT_B);

	/* TINFO="Delay 1us" */
	udelay(1);

	/* TINFO="Set SRAM_CKISO = 0" */
	mmio_clrbits_32(per_cpu(cluster, cpu, SPM_CPU_PWRCTRL), SPM_CPU_SRAM_CKISO);

	/* TINFO="Set PWR_CLK_DIS = 0" */
	mmio_clrbits_32(per_cpu(cluster, cpu, SPM_CPU_PWRCTRL), SPM_CPU_PWR_CLK_DIS);

	/* TINFO="Set PWR_RST_B = 1" */
	mmio_setbits_32(per_cpu(cluster, cpu, SPM_CPU_PWRCTRL), SPM_CPU_PWR_RST_B);

	/* TINFO="Finish to turn on MP0_CPU1" */
	pr_debug("spmc: power on core %d.%d successfully\n", cluster, cpu);
}

/**
 * Power off a core with specified cluster and core index
 *
 * @cluster: the cluster ID of the CPU which to be powered off
 * @cpu: the CPU ID of the CPU which to be powered off
 */
void spm_poweroff_cpu(int cluster, int cpu)
{
	pr_debug("spmc: power off core %d.%d\n", cluster, cpu);

	/* TINFO="enable SPM register control" */
	mmio_write_32(SPM_POWERON_CONFIG_EN, (SPM_PROJECT_CODE << 16) | (0x1 << 0));

	/* TINFO="Start to turn off MP0_CPU3" */
	/* TINFO="Set PWR_ISO = 1" */
	mmio_setbits_32(per_cpu(cluster, cpu, SPM_CPU_PWRCTRL), SPM_CPU_PWR_ISO);

	/* TINFO="Set SLPB_CLAMP = 1" */
	mmio_setbits_32(per_cpu(cluster, cpu, SPM_CPU_PWRCTRL), SPM_CPU_SRAM_PD_SLPB_CLAMP);

	/* TINFO="Set SRAM_CKISO = 1" */
	mmio_setbits_32(per_cpu(cluster, cpu, SPM_CPU_PWRCTRL), SPM_CPU_SRAM_CKISO);

	/* TINFO="Set SRAM_ISOINT_B = 0" */
	mmio_clrbits_32(per_cpu(cluster, cpu, SPM_CPU_PWRCTRL), SPM_CPU_SRAM_ISOINT_B);

	/* TINFO="Set SRAM_PDN = 1" */
	mmio_setbits_32(per_cpu(cluster, cpu, SPM_CPU_PWRCTRL), SPM_CPU_SRAM_PDN);

	/* TINFO="Wait until MP0_CPU3_SRAM_PDN_ACK = 1" */
	/* SRAM PDN delay IP clock is 26MHz. Print SRAM control and ACK for debug. */
	while (!(mmio_read_32(per_cpu(cluster, cpu, SPM_CPU_PWRCTRL)) & SPM_CPU_SRAM_PDN_ACK))
		;

	/* TINFO="Set PWR_RST_B = 0" */
	mmio_clrbits_32(per_cpu(cluster, cpu, SPM_CPU_PWRCTRL), SPM_CPU_PWR_RST_B);

	/* TINFO="Set PWR_CLK_DIS = 1" */
	mmio_setbits_32(per_cpu(cluster, cpu, SPM_CPU_PWRCTRL), SPM_CPU_PWR_CLK_DIS);

	/* TINFO="Set PWR_ON = 0" */
	mmio_clrbits_32(per_cpu(cluster, cpu, SPM_CPU_PWRCTRL), SPM_CPU_PWR_ON);

	/* TINFO="Set PWR_ON_2ND = 0" */
	mmio_clrbits_32(per_cpu(cluster, cpu, SPM_CPU_PWRCTRL), SPM_CPU_PWR_ON_2ND);

	/* TINFO="Wait until PWR_STATUS = 0 and PWR_STATUS_2ND = 0" */
	/* No logic between pwr_on and pwr_ack. Print SRAM / MTCMOS
	 * control and PWR_ACK for debug.
	 */
	while (spm_get_cpu_powerstate(cluster, cpu))
		;

	/* TINFO="Finish to turn off MP0_CPU3" */
	pr_debug("spmc: power off core %d.%d successfully\n", cluster, cpu);
}

/**
 * Power off a cluster with specified index
 *
 * @cluster: the cluster index which to be powered off
 */
void spm_poweroff_cluster(int cluster)
{
	pr_debug("spmc: power off cluster %d\n", cluster);

	/* TINFO="enable SPM register control" */
	mmio_write_32(POWERON_CONFIG_EN, (SPM_PROJECT_CODE << 16) | (0x1 << 0));

	/* TINFO="Start to turn off MP0_CPUTOP" */
	/* TINFO="Set bus protect" */
	mmio_setbits_32(INFRA_TOPAXI_PROTECTEN_1, MP0_CPUTOP_PROT_BIT_2ND_MASK);

	while ((mmio_read_32(INFRA_TOPAXI_PROTECTEN_STA1_1) & MP0_CPUTOP_PROT_BIT_ACK_2ND_MASK)
	       != MP0_CPUTOP_PROT_BIT_ACK_2ND_MASK)
		;

	/* TINFO="Set PWR_ISO = 1" */
	mmio_setbits_32(per_cluster(cluster, SPM_CLUSTER_PWRCTRL), SPM_CPU_PWR_ISO);

	/* TINFO="Set SRAM_CKISO = 1" */
	mmio_setbits_32(per_cluster(cluster, SPM_CLUSTER_PWRCTRL), SPM_CPU_SRAM_CKISO);

	/* TINFO="Set SRAM_ISOINT_B = 0" */
	mmio_clrbits_32(per_cluster(cluster, SPM_CLUSTER_PWRCTRL), SPM_CPU_SRAM_ISOINT_B);

	/* TINFO="Set SRAM_PDN = 1" */
	mmio_setbits_32(per_cluster(cluster, SPM_CLUSTER_PWRCTRL), SPM_CPU_SRAM_PDN);

	/* TINFO="Wait until MP0_CPUTOP_SRAM_PDN_ACK = 1" */
	/* SRAM PDN delay IP clock is 26MHz. Print SRAM control and ACK for debug. */
	while (!(mmio_read_32(per_cluster(cluster, SPM_CLUSTER_PWRCTRL)) & SPM_CPU_SRAM_PDN_ACK))
		;

	/* TINFO="Set PWR_RST_B = 0" */
	mmio_clrbits_32(per_cluster(cluster, SPM_CLUSTER_PWRCTRL), SPM_CPU_PWR_RST_B);

	/* TINFO="Set PWR_CLK_DIS = 1" */
	mmio_setbits_32(per_cluster(cluster, SPM_CLUSTER_PWRCTRL), SPM_CPU_PWR_CLK_DIS);

	/* TINFO="Set PWR_ON = 0" */
	mmio_clrbits_32(per_cluster(cluster, SPM_CLUSTER_PWRCTRL), SPM_CPU_PWR_ON);

	/* TINFO="Set PWR_ON_2ND = 0" */
	mmio_clrbits_32(per_cluster(cluster, SPM_CLUSTER_PWRCTRL), SPM_CPU_PWR_ON_2ND);

	/* TINFO="Wait until PWR_STATUS = 0 and PWR_STATUS_2ND = 0" */
	/* No logic between pwr_on and pwr_ack. Print SRAM / MTCMOS
	 * control and PWR_ACK for debug.
	 */
	while (spm_get_cluster_powerstate(cluster))
		;

	/* TINFO="Finish to turn off MP0_CPUTOP" */
	pr_debug("spmc: power off cluster %d successfully\n", cluster);
}

/**
 * Power on a cluster with specified index
 *
 * @cluster: the cluster index which to be powered on
 */
void spm_poweron_cluster(int cluster)
{
	pr_debug("spmc: power on cluster %d\n", cluster);

	/* TINFO="enable SPM register control" */
	mmio_write_32(POWERON_CONFIG_EN, (SPM_PROJECT_CODE << 16) | (0x1 << 0));

	/* TINFO="Start to turn on MP0_CPUTOP" */
	/* TINFO="Set PWR_RST_B = 0" */
	mmio_clrbits_32(per_cluster(cluster, SPM_CLUSTER_PWRCTRL), SPM_CPU_PWR_RST_B);

	/* TINFO="Set PWR_CLK_DIS = 1" */
	mmio_setbits_32(per_cluster(cluster, SPM_CLUSTER_PWRCTRL), SPM_CPU_PWR_CLK_DIS);

	/* TINFO="Set PWR_ON = 1" */
	mmio_setbits_32(per_cluster(cluster, SPM_CLUSTER_PWRCTRL), SPM_CPU_PWR_ON);

	/* TINFO="Delay 1us" */
	udelay(1);

	/* TINFO="Set PWR_ON_2ND = 1" */
	mmio_setbits_32(per_cluster(cluster, SPM_CLUSTER_PWRCTRL), SPM_CPU_PWR_ON_2ND);

	/* TINFO="Wait until PWR_STATUS = 1 and PWR_STATUS_2ND = 1" */
	/* No logic between pwr_on and pwr_ack. Print SRAM / MTCMOS
	 * control and PWR_ACK for debug.
	 */
	while (!spm_get_cluster_powerstate(cluster))
		;

	/* TINFO="Set PWR_ISO = 0" */
	mmio_clrbits_32(per_cluster(cluster, SPM_CLUSTER_PWRCTRL), SPM_CPU_PWR_ISO);

	/* TINFO="Set SRAM_PDN = 0" */
	mmio_clrbits_32(per_cluster(cluster, SPM_CLUSTER_PWRCTRL), SPM_CPU_SRAM_PDN);

	/* TINFO="Wait until MP0_CPUTOP_SRAM_PDN_ACK_BIT0 = 0" */
	while (mmio_read_32(per_cluster(cluster, SPM_CLUSTER_PWRCTRL)) & SPM_CPU_SRAM_PDN_ACK)
		;

	/* TINFO="Delay 1us" */
	udelay(1);

	/* TINFO="Set SRAM_ISOINT_B = 1" */
	mmio_setbits_32(per_cluster(cluster, SPM_CLUSTER_PWRCTRL), SPM_CPU_SRAM_ISOINT_B);

	/* TINFO="Delay 1us" */
	udelay(1);

	/* TINFO="Set SRAM_CKISO = 0" */
	mmio_clrbits_32(per_cluster(cluster, SPM_CLUSTER_PWRCTRL), SPM_CPU_SRAM_CKISO);

	/* TINFO="Set PWR_CLK_DIS = 0" */
	mmio_clrbits_32(per_cluster(cluster, SPM_CLUSTER_PWRCTRL), SPM_CPU_PWR_CLK_DIS);

	/* TINFO="Set PWR_RST_B = 1" */
	mmio_setbits_32(per_cluster(cluster, SPM_CLUSTER_PWRCTRL), SPM_CPU_PWR_RST_B);

	/* TINFO="Release bus protect" */
	/* Note that this protect ack check after releasing protect has been ignored */
	mmio_setbits_32(INFRA_TOPAXI_PROTECTEN_1, MP0_CPUTOP_PROT_BIT_2ND_MASK);

	/* TINFO="Finish to turn on MP0_CPUTOP" */
	pr_debug("spmc: power on cluster %d successfully\n", cluster);
}
