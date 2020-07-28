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

#include <assert.h>
#include <debug.h>
#include <mmio.h>
#include <platform_def.h>
#include <delay_timer.h>
#include <mcucfg.h>
#include <mtspmc.h>

#include "mtspmc_private.h"

#if CONFIG_SPMC_SPARK == 1

static void ildo_set_retention(int cpu)
{
	uintptr_t reg;
	unsigned int ildo_ret_volt = VOLTAGE_04;

	/* set retention voltage */
	reg = per_cpu(0, cpu, ILDO_CONTROL5);
	mmio_clrsetbits_32(reg, ILDO_RET_VOSEL, ildo_ret_volt);

	/* disable iLDO bypass mode */
	reg = per_cpu(0, cpu, ILDO_CONTROL8);
	mmio_setbits_32(reg, ILDO_BYPASS_B);
}

/**
 * program CPUPWRCTLR_EL1[9:7].WFE_RET_CTRL and
 * CPUPWRCTLR_EL1[6:4].WFI_RET_CTRL
 *
 * @tick: write @tick to the CPU register CPUPWRCTLR_EL1

 * The @tick value can only be accepted with range 0~7 which the
 * meaning is
 *
 * 0b000 Disable the retention circuit. This is the reset value.
 * 0b001 2   Architectural Timer ticks are required before retention entry.
 * 0b010 8   Architectural Timer ticks are required before retention entry.
 * 0b011 32  Architectural Timer ticks are required before retention entry.
 * 0b100 64  Architectural Timer ticks are required before retention entry.
 * 0b101 128 Architectural Timer ticks are required before retention entry.
 * 0b110 256 Architectural Timer ticks are required before retention entry.
 * 0b111 512 Architectural Timer ticks are required before retention entry.
 */
static void spark_set_retention(int tick)
{
	uint32_t v;

	tick &= RET_CTRL_MASK;

	v = read_cpupwrctlr_el1();
	v &= ~((RET_CTRL_MASK << WFE_RET_CTRL_SHIFT) |
	       (RET_CTRL_MASK << WFI_RET_CTRL_SHIFT));
	v |= (tick << WFE_RET_CTRL_SHIFT) | (tick << WFI_RET_CTRL_SHIFT);
	write_cpupwrctlr_el1(v);
}

void spark_enable(int cluster, int cpu)
{
	uintptr_t reg;

	/* Only CPU6/7 have ILDO */
	if (cpu > 5)
		ildo_set_retention(cpu);

	spark_set_retention(1);

	reg = per_cpu(cluster, cpu, MCUCFG_SPARK);
	mmio_setbits_32(reg, SW_SPARK_EN);

	VERBOSE("%s: 0x%x: %x\n", __func__, reg, mmio_read_32(reg));
}

void spark_disable(int cluster, int cpu)
{
	uintptr_t reg;

	spark_set_retention(0);

	reg = per_cpu(cluster, cpu, MCUCFG_SPARK);
	mmio_clrbits_32(reg, SW_SPARK_EN);

	VERBOSE("%s: 0x%x: %x\n", __func__, reg, mmio_read_32(reg));
}
#else /* CONFIG_SPMC_SPARK == 1 */

void spark_enable(int cluster, int cpu)
{
}

void spark_disable(int cluster, int cpu)
{
}
#endif /* CONFIG_SPMC_SPARK == 0 */

void mcucfg_disable_gic_wakeup(int cluster, int cpu)
{
	mmio_setbits_32(MCUCFG_CPC_FLOW_CTRL_CFG, GIC_WAKEUP_IGNORE(cpu));
}

void mcucfg_enable_gic_wakeup(int cluster, int cpu)
{
	mmio_clrbits_32(MCUCFG_CPC_FLOW_CTRL_CFG, GIC_WAKEUP_IGNORE(cpu));
}

void mcucfg_set_bootaddr(int cluster, int cpu, uintptr_t bootaddr)
{
	assert(cluster == 0);

	mmio_write_32(per_cpu(cluster, cpu, MCUCFG_BOOTADDR), bootaddr);
}

uintptr_t mcucfg_get_bootaddr(int cluster, int cpu)
{
	assert(cluster == 0);

	return mmio_read_32(per_cpu(cluster, cpu, MCUCFG_BOOTADDR));
}

void mcucfg_init_archstate(int cluster, int cpu, int arm64)
{
	uint32_t reg;

	assert(cluster == 0);

	reg = per_cluster(cluster, MCUCFG_INITARCH);

	/* aa64naa32 in bits[16:23] */
	if (arm64)
		mmio_setbits_32(reg, 1 << (16 + cpu));
	else
		mmio_clrbits_32(reg, 1 << (16 + cpu));
}

/**
 * Return the someone subsystem's power state.
 *
 * @mask: mask to SPM_CPU_PWR_STATUS to query the power state
 *        of one subsystem.
 * RETURNS:
 * 0 (the subsys was powered off)
 * 1 (the subsys was powered on)
 */
int spm_get_powerstate(uint32_t mask)
{
	return (mmio_read_32(SPM_PWR_STATUS) & mask);
}

int spm_get_cluster_powerstate(int cluster)
{
	assert(cluster == 0);

	return spm_get_powerstate(MP0_CPUTOP);
}

int spm_get_cpu_powerstate(int cluster, int cpu)
{
	uint32_t mask;

	assert(cluster == 0);

	/*
	 * A quick way to specify the mask of cpu[0-7] in
	 * register SPM_PWR_STATUS whereas the bits are
	 * BITS[9:12] (CPU0~3) and BITS[15:18] (CPU4~7)
	 */
	if (cpu < 4)
		mask = 1 << (9 + cpu);
	else
		mask = 1 << (15 + cpu - 4);

	return spm_get_powerstate(mask);
}

int spmc_init(void)
{
#if CONFIG_SPMC_MODE == 1
	INFO("SPM: enable SPMC mode\n");
#else
	INFO("SPM: enable CPC mode\n");
#endif

	mmio_write_32(SPM_POWERON_CONFIG_EN, PROJECT_CODE | BCLK_CG_EN);

	/* TINFO=SPMC_INIT: release PWR_RST_B of Core 1-7 */
	mmio_setbits_32(per_cpu(0, 1, SPM_CPU_PWR), PWR_RST_B);
	mmio_setbits_32(per_cpu(0, 2, SPM_CPU_PWR), PWR_RST_B);
	mmio_setbits_32(per_cpu(0, 3, SPM_CPU_PWR), PWR_RST_B);
	mmio_setbits_32(per_cpu(0, 4, SPM_CPU_PWR), PWR_RST_B);
	mmio_setbits_32(per_cpu(0, 5, SPM_CPU_PWR), PWR_RST_B);
	mmio_setbits_32(per_cpu(0, 6, SPM_CPU_PWR), PWR_RST_B);
	mmio_setbits_32(per_cpu(0, 7, SPM_CPU_PWR), PWR_RST_B);

	/* INFO=SPMC_INIT: clear resetpwron of mcusys/cluster/core0 */
	mmio_clrbits_32(SPM_MCUSYS_PWR_CON, RESETPWRON_CONFIG);
	mmio_clrbits_32(SPM_MP0_CPUTOP_PWR_CON, RESETPWRON_CONFIG);
	mmio_clrbits_32(per_cpu(0, 0, SPM_CPU_PWR), RESETPWRON_CONFIG);

#if CONFIG_SPMC_MODE == 0
	/* enable CPC */
	mmio_setbits_32(MCUCFG_CPC_FLOW_CTRL_CFG, CPC_CTRL_ENABLE);
#endif

	return 0;
}

#if CONFIG_SPMC_MODE == 0
/**
 * Power on a core with specified cluster and core index
 * with CPC
 *
 * @cluster: the cluster ID of the CPU which to be powered on
 * @cpu: the CPU ID of the CPU which to be powered on
 */
static void spm_poweron_cpu_cpc(int cluster, int cpu)
{
	/* info CPC that CPU hotplug on */
	mmio_setbits_32(MCUCFG_CPC_FLOW_CTRL_CFG, SSPM_ALL_PWR_CTRL_EN);

	/* Set mp0_spmc_pwr_on_cpuX = 1 */
	mmio_setbits_32(per_cpu(cluster, cpu, SPM_CPU_PWR), PWR_ON);

	/* wait for power on ack */
	while (!spm_get_cpu_powerstate(cluster, cpu))
		;

	/* info CPC that CPU hotplug off */
	mmio_clrbits_32(MCUCFG_CPC_FLOW_CTRL_CFG, SSPM_ALL_PWR_CTRL_EN);
}

/**
 * Power off a core with specified cluster and core index
 * with CPC
 *
 * @cluster: the cluster ID of the CPU which to be powered off
 * @cpu: the CPU ID of the CPU which to be powered off
 */
static void spm_poweroff_cpu_cpc(int cluster, int cpu)
{
	/* Set mp0_spmc_pwr_on_cpuX = 0 */
	mmio_clrbits_32(per_cpu(cluster, cpu, SPM_CPU_PWR), PWR_ON);

	/* wait for power on ack */
	while (spm_get_cpu_powerstate(cluster, cpu))
		;
}

/**
 * Power off a cluster with specified index
 * with CPC
 *
 * @cluster: the cluster index which to be powered off
 */
static void spm_poweroff_cluster_cpc(int cluster)
{
	/* CPU hotplug do not power off cluster for ARMv8.2 */
}

/**
 * Power on a cluster with specified index
 * with CPC
 *
 * @cluster: the cluster index which to be powered on
 */
static void spm_poweron_cluster_cpc(int cluster)
{
	/* CPU hotplug do not power on cluster for ARMv8.2 */
}
#endif /* CONFIG_SPMC_MODE == 0 */

/**
 * Power on a core with specified cluster and core index
 *
 * @cluster: the cluster ID of the CPU which to be powered on
 * @cpu: the CPU ID of the CPU which to be powered on
 */
void spm_poweron_cpu(int cluster, int cpu)
{
	INFO("spmc: power on core %d.%d\n", cluster, cpu);

#if CONFIG_SPMC_MODE == 0
	spm_poweron_cpu_cpc(cluster, cpu);

	INFO("spmc: power on core %d.%d successfully\n", cluster, cpu);
	return;
#else

	/* TINFO="Start to turn on MP0_CPU0_SPMC" */
	/* TINFO="Set VPROC_EXT_OFF = 0" */
	mmio_clrbits_32(per_cpu(cluster, cpu, SPM_CPU_PWR), VPROC_EXT_OFF);

	/* TINFO="Set PWR_ON = 1" */
	mmio_setbits_32(per_cpu(cluster, cpu, SPM_CPU_PWR), PWR_ON);

	/* TINFO="Wait until MP0_CPU0_SPMC_PWR_STA_MASK = 1" */
	while (!spm_get_cpu_powerstate(cluster, cpu))
		;

	/* TINFO="Set PWR_RST_B = 1" */
	mmio_setbits_32(per_cpu(cluster, cpu, SPM_CPU_PWR), PWR_RST_B);

	/* TINFO="Finish to turn on MP0_CPU0_SPMC" */
	INFO("spmc: power on core %d.%d successfully\n", cluster, cpu);
#endif
}

/**
 * Power off a core with specified cluster and core index
 *
 * @cluster: the cluster ID of the CPU which to be powered off
 * @cpu: the CPU ID of the CPU which to be powered off
 */
void spm_poweroff_cpu(int cluster, int cpu)
{
	INFO("spmc: power off core %d.%d\n", cluster, cpu);

#if CONFIG_SPMC_MODE == 0
	spm_poweroff_cpu_cpc(cluster, cpu);

	INFO("spmc: power off core %d.%d successfully\n", cluster, cpu);
	return;
#else

	/* TINFO="Start to turn off MP0_CPU0_SPMC" */
	/* TINFO="Set PWR_RST_B = 0" */
	mmio_clrbits_32(per_cpu(cluster, cpu, SPM_CPU_PWR), PWR_RST_B);

	/* TINFO="Set PWR_ON = 0" */
	mmio_clrbits_32(per_cpu(cluster, cpu, SPM_CPU_PWR), PWR_ON);

	/* TINFO="Wait until MP0_CPU0_SPMC_PWR_STA_MASK = 0" */
	while (spm_get_cpu_powerstate(cluster, cpu))
		;

	/* TINFO="Set VPROC_EXT_OFF = 1" */
	mmio_setbits_32(per_cpu(cluster, cpu, SPM_CPU_PWR), VPROC_EXT_OFF);

	/* TINFO="Finish to turn off MP0_CPU0_SPMC" */
	INFO("spmc: power off core %d.%d successfully\n", cluster, cpu);
#endif
}

/**
 * Power off a cluster with specified index
 *
 * @cluster: the cluster index which to be powered off
 */
void spm_poweroff_cluster(int cluster)
{
	INFO("spmc: power off cluster %d\n", cluster);

#if CONFIG_SPMC_MODE == 0
	spm_poweroff_cluster_cpc(cluster);

	INFO("spmc: power off cluster %d successfully\n", cluster);
	return;
#else

	/* TINFO="Start to turn off MP0_SPMC" */
	/* TINFO="Set bus protect - step1 : 0" */
	mmio_write_32(INFRA_TOPAXI_PROTECTEN_SET, MP0_SPMC_PROT_STEP1_0_MASK);

	while (!(mmio_read_32(INFRA_TOPAXI_PROTECTEN_STA1) &
		 MP0_SPMC_PROT_STEP1_0_MASK))
		;

	/* TINFO="Set bus protect - step1 : 1" */
	mmio_write_32(INFRA_TOPAXI_PROTECTEN_1_SET, MP0_SPMC_PROT_STEP1_1_MASK);

	while (!(mmio_read_32(INFRA_TOPAXI_PROTECTEN_STA1_1) &
		 MP0_SPMC_PROT_STEP1_1_MASK))
		;

	/* TINFO="Set PWR_RST_B = 0" */
	mmio_clrbits_32(per_cluster(cluster, SPM_CLUSTER_PWR), PWR_RST_B);

	/* TINFO="Set PWR_ON = 0" */
	mmio_clrbits_32(per_cluster(cluster, SPM_CLUSTER_PWR), PWR_ON);

	/* TINFO="Wait until MP0_SPMC_PWR_STA_MASK = 0" */
	while (spm_get_cluster_powerstate(cluster))
		;

	/* TINFO="Set VPROC_EXT_OFF = 1" */
	mmio_setbits_32(per_cluster(cluster, SPM_CLUSTER_PWR), VPROC_EXT_OFF);

	/* TINFO="Finish to turn off MP0_SPMC" */
	INFO("spmc: power off cluster %d successfully\n", cluster);
#endif
}

/**
 * Power on a cluster with specified index
 *
 * @cluster: the cluster index which to be powered on
 */
void spm_poweron_cluster(int cluster)
{
	INFO("spmc: power on cluster %d\n", cluster);

#if CONFIG_SPMC_MODE == 0
	spm_poweron_cluster_cpc(cluster);

	INFO("spmc: power on cluster %d successfully\n", cluster);
	return;
#else

	/* TINFO="Start to turn on MP0_SPMC" */
	/* TINFO="Set VPROC_EXT_OFF = 0" */
	mmio_clrbits_32(per_cluster(cluster, SPM_CLUSTER_PWR), VPROC_EXT_OFF);

	/* TINFO="Set PWR_ON = 1" */
	mmio_setbits_32(per_cluster(cluster, SPM_CLUSTER_PWR), PWR_ON);

	/* TINFO="Wait until MP0_SPMC_PWR_STA_MASK = 1" */
	while (!spm_get_cluster_powerstate(cluster))
		;

	/* TINFO="Set PWR_RST_B = 1" */
	mmio_setbits_32(per_cluster(cluster, SPM_CLUSTER_PWR), PWR_RST_B);

	/* TINFO="Release bus protect - step1 : 0" */
	mmio_write_32(INFRA_TOPAXI_PROTECTEN_CLR, MP0_SPMC_PROT_STEP1_0_MASK);

	/* TINFO="Release bus protect - step1 : 1" */
	mmio_write_32(INFRA_TOPAXI_PROTECTEN_1_CLR, MP0_SPMC_PROT_STEP1_1_MASK);

	/* TINFO="Finish to turn on MP0_SPMC" */
	INFO("spmc: power on cluster %d successfully\n", cluster);
#endif
}
