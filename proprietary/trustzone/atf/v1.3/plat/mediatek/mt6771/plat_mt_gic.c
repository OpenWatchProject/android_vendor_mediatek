/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
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

#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <bl31_fiq_handler.h>
#include <context.h>
#include <context_mgmt.h>
#include <debug.h>
#include <fiq_smp_call.h>
#include <gic_v2.h>
#include <gicv3.h>
#include <gicv3_private.h>
#include <interrupt_mgmt.h>
#include <mt_gic_v3.h>
#include <mtk_aee_debug.h>
#include <mtk_plat_common.h>
#include "plat_private.h"
#include <platform.h>
#include <platform_def.h>
#include <stdint.h>
#if CFG_MICROTRUST_TEE_SUPPORT
#include "plat_teei.h"
#endif
#include <stdio.h>
#include <pccif.h>
//#define GIC_DEBUG
#define MAX_IRQ_NR			(342)
#ifndef MAX_GIC_NR
#define MAX_GIC_NR			(1)
#endif

#define MAX_RDIST_NR			(64)

#define DIV_ROUND_UP(n,d)		(((n) + (d) - 1) / (d))

#define SZ_64K				(0x00010000)

/* helpers for later ICC encode macros
 * Indirect stringification.  Doing two levels allows the parameter to be a
 * macro itself.  For example, compile with -DFOO=bar, __stringify(FOO)
 * converts to "bar".
 */
#define __stringify_1(x)	#x
#define __stringify(x)		__stringify_1(x)

/* since gcc not support most ARMv8 ICC sysreg in asm,
 * we learn Linux's way to encode them */
#define sys_reg(op0, op1, crn, crm, op2) \
        ((((op0)-2)<<19)|((op1)<<16)|((crn)<<12)|((crm)<<8)|((op2)<<5))

__asm__ (
"       .irp    num,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30\n"
"       .equ    __reg_num_x\\num, \\num\n"
"       .endr\n"
"       .equ    __reg_num_xzr, 31\n"
"\n"
"       .macro  mrs_s, rt, sreg\n"
"       .inst   0xd5300000|(\\sreg)|(__reg_num_\\rt)\n"
"       .endm\n"
"\n"
"       .macro  msr_s, sreg, rt\n"
"       .inst   0xd5100000|(\\sreg)|(__reg_num_\\rt)\n"
"       .endm\n"
);

#define ICC_GRPEN0_EL1                  sys_reg(3, 0, 12, 12, 6)
#define ICC_GRPEN1_EL1                  sys_reg(3, 0, 12, 12, 7)
#define ICC_GRPEN1_EL3                  sys_reg(3, 6, 12, 12, 7)
#define ICC_IAR0_EL1_			sys_reg(3, 0, 12,  8, 0)
#define ICC_HPPIR0_EL1_			sys_reg(3, 0, 12,  8, 2)
#define ICC_HPPIR1_EL1_			sys_reg(3, 0, 12, 12, 2)
#define ICC_SGI0R_EL1			sys_reg(3, 0, 12, 11, 7)
#define ICC_EOIR0_EL1_			sys_reg(3, 0, 12,  8, 1)
/* end of ICC sysreg encode macros */

#define MPIDR_LEVEL_BITS_SHIFT  3
#define MPIDR_LEVEL_BITS        (1 << MPIDR_LEVEL_BITS_SHIFT)
#define MPIDR_LEVEL_MASK ((1 << MPIDR_LEVEL_BITS) - 1)

#define MPIDR_LEVEL_SHIFT(level) \
        (((1 << level) >> 1) << MPIDR_LEVEL_BITS_SHIFT)

#define MPIDR_AFFINITY_LEVEL(mpidr, level) \
        ((mpidr >> MPIDR_LEVEL_SHIFT(level)) & MPIDR_LEVEL_MASK)

#define CPU_LOGIC_MAP_RESET_VAL	(-1L)
static uint64_t cpu_logical_map[PLATFORM_CORE_COUNT];

struct gic_chip_data {
	unsigned int saved_enable[DIV_ROUND_UP(MAX_IRQ_NR, 32)];
	unsigned int saved_conf[DIV_ROUND_UP(MAX_IRQ_NR, 16)];
	unsigned int saved_priority[DIV_ROUND_UP(MAX_IRQ_NR, 4)];
	uint64_t saved_target[DIV_ROUND_UP(MAX_IRQ_NR, 1)];
	unsigned int saved_group[DIV_ROUND_UP(MAX_IRQ_NR, 32)];
	unsigned int saved_grpmod[DIV_ROUND_UP(MAX_IRQ_NR, 32)];
	unsigned int rdist_base[MAX_RDIST_NR];
#if CFG_MICROTRUST_TEE_SUPPORT
	unsigned int saved_spi_pending[DIV_ROUND_UP(MAX_IRQ_NR, 32)];
#endif
	unsigned int saved_active_sel;
	unsigned int saved_sgi[PLATFORM_CORE_COUNT];
};

static struct gic_chip_data gic_data[MAX_GIC_NR];

#if  CFG_MICROTRUST_TEE_SUPPORT
 void gicc_write_ctlr_el1(uint64_t val)
{
	__asm__ volatile("msr_s " __stringify(ICC_V3_CTLR_EL1) ", %0" : : "r" (val));
}

 uint64_t  gicc_read_ctlr_el1(void)
{
	uint64_t val = 0;
	__asm__ volatile("mrs_s %0, " __stringify(ICC_V3_CTLR_EL1) : "=r" (val));
	return val;
}

#endif

void gic_sgi_save_all(void)
{
	int cpuid = 0;
	unsigned int base;
	unsigned int cpu_offset = 0;

	base = MT_GIC_RDIST_BASE;
	base += SZ_64K;
	for (cpuid = 0; cpuid < PLATFORM_CORE_COUNT; cpuid++) {
		cpu_offset = cpuid*SZ_64K*2;
		gic_data[0].saved_sgi[cpuid] = mmio_read_32(base + GICD_V3_ISPENDR + cpu_offset) & SGI_MASK;
#ifdef GIC_DEBUG
		NOTICE("[ATF GIC dump] sgi_save CPU%d &0x%x = 0x%x\n",
			cpuid, base + GICD_V3_ISPENDR + cpu_offset,
			gic_data[0].saved_sgi[cpuid]);
#endif
	}
}

void gic_sgi_restore_all(void)
{
	int cpuid = 0;
	unsigned int base;
	unsigned int cpu_offset = 0;

	base = MT_GIC_RDIST_BASE;
	base += SZ_64K;
	for (cpuid = 0; cpuid < PLATFORM_CORE_COUNT; cpuid++) {
		cpu_offset = cpuid*SZ_64K*2;
		mmio_write_32(base + GICD_V3_ICPENDR + cpu_offset, SGI_MASK);
		mmio_write_32(base + GICD_V3_ISPENDR + cpu_offset, gic_data[0].saved_sgi[cpuid] & SGI_MASK);
#ifdef GIC_DEBUG
		NOTICE("[ATF GIC dump] sgi_restore CPU%d &0x%x = 0x%x\n",
			cpuid, base + GICD_V3_ISPENDR + cpu_offset,
			mmio_read_32(base + GICD_V3_ISPENDR + cpu_offset) & SGI_MASK);
#endif
	}
}

static void gicc_write_grpen0_el1(uint64_t val)
{
	__asm__ volatile("msr_s " __stringify(ICC_GRPEN0_EL1) ", %0" : : "r" (val));
}

void gicc_write_grpen1_el1(uint64_t val)
{
	__asm__ volatile("msr_s " __stringify(ICC_GRPEN1_EL1) ", %0" : : "r" (val));
}
#if defined(SPD_trusty)
static void gicc_write_grpen1_el3(uint64_t val)
{
	__asm__ volatile("msr_s " __stringify(ICC_GRPEN1_EL3) ", %0" : : "r" (val));
}
#endif

static uint64_t gicc_read_hppir0_el1(void)
{
	uint64_t val = 0;

	__asm__ volatile("mrs_s %0, " __stringify(ICC_HPPIR0_EL1_) : "=r" (val));

	return val;
}

static uint64_t gicc_read_hppir1_el1(void)
{
	uint64_t val = 0;

	__asm__ volatile("mrs_s %0, " __stringify(ICC_HPPIR1_EL1_) : "=r" (val));

	return val;
}

static uint32_t gicc_read_iar0_el1(void)
{
	uint64_t val = 0;

	__asm__ volatile("mrs_s %0, " __stringify(ICC_IAR0_EL1_) : "=r" (val));

	return val;
}

static void gicc_write_eoi0_el1(uint64_t val)
{
	__asm__ volatile("msr_s " __stringify(ICC_EOIR0_EL1_) ", %0" : : "r" (val));
}

static void gicc_write_sgi0_el1(uint64_t val)
{
	__asm__ volatile("msr_s " __stringify(ICC_SGI0R_EL1) ", %0" : : "r" (val));
}

void gicd_v3_do_wait_for_rwp(unsigned int gicd_base)
{
	while (gicd_v3_read_ctlr(gicd_base) & GICD_V3_CTLR_RWP);
}

int gic_populate_rdist(unsigned int *rdist_base)
{
	int cpuid = 0;
	unsigned int reg = 0;
	unsigned int base = 0;

	cpuid = platform_get_core_pos(read_mpidr());
	reg = gicd_v3_read_pidr2(MT_GIC_BASE) & GIC_V3_PIDR2_ARCH_MASK;

	if ((reg != GIC_V3_PIDR2_ARCH_GICv3) && (reg != GIC_V3_PIDR2_ARCH_GICv4)) {
		ERROR("We are in trouble, not GIC-v3 or v4\n");
		return -1;
	}

	base = MT_GIC_RDIST_BASE + cpuid*SZ_64K*2;

	*rdist_base = base;
	/* save a copy for later save/restore use */
	gic_data[0].rdist_base[cpuid] = base;

#ifdef GIC_DEBUG
	INFO("cpu(%d), rdist_base = 0x%x\n", cpuid, *rdist_base);
#endif
	return 0;
}

/* TODO: check all registers to save */
void gic_dist_save(void)
{
	unsigned int gic_irqs;
	unsigned int dist_base;
	int i;

	/* TODO: pending bit MUST added */
	dist_base = MT_GIC_BASE;

	gic_irqs = MAX_IRQ_NR;

	for (i = 1; i < DIV_ROUND_UP(gic_irqs, 16); i++)
		gic_data[0].saved_conf[i] =
			mmio_read_32(dist_base + GICD_ICFGR + i * 4);

	for (i = 8; i < DIV_ROUND_UP(gic_irqs, 4); i++)
		gic_data[0].saved_priority[i] =
			mmio_read_32(dist_base + GICD_IPRIORITYR + i * 4);

	for (i = 1; i < DIV_ROUND_UP(gic_irqs, 1); i++)
		gic_data[0].saved_target[i] =
			mmio_read_64(dist_base + GICD_V3_IROUTER + i*8);

	for (i = 1; i < DIV_ROUND_UP(gic_irqs, 32); i++)
		gic_data[0].saved_enable[i] =
			mmio_read_32(dist_base + GICD_ISENABLER + i * 4);

	for (i = 1; i < DIV_ROUND_UP(gic_irqs, 32); i++)
		gic_data[0].saved_group[i] =
			mmio_read_32(dist_base + GICD_IGROUPR + i * 4);

	for (i = 1; i < DIV_ROUND_UP(gic_irqs, 32); i++)
		gic_data[0].saved_grpmod[i] =
			mmio_read_32(dist_base + GICE_V3_IGRPMOD0 + i * 4);
#if CFG_MICROTRUST_TEE_SUPPORT
	for (i = 1; i < DIV_ROUND_UP(gic_irqs, 32); i++)
		gic_data[0].saved_spi_pending[i] =
			mmio_read_32(dist_base + GICD_ISPENDR + i * 4);
#endif
}

static uint32_t rdist_has_saved[PLATFORM_CORE_COUNT];

void gic_rdist_save(void)
{
	unsigned int rdist_base, i;

	/* get the base of redistributor first */
	if (gic_populate_rdist(&rdist_base) == -1) {
		return;
	}

	/* the SGI/PPI base is at 64K offset of current rdist base */
	rdist_base += SZ_64K;

	/* TODO: save router */
	/* handle SGI & PPI */
	gic_data[0].saved_conf[0] = mmio_read_32(rdist_base + GICD_ICFGR);
	for (i = 0; i <= 7; ++i)
		gic_data[0].saved_priority[i] = mmio_read_32(rdist_base + GICD_IPRIORITYR + i * 4);
	gic_data[0].saved_enable[0] = mmio_read_32(rdist_base + GICD_ISENABLER);
	gic_data[0].saved_group[0] = mmio_read_32(rdist_base + GICD_IGROUPR);
	gic_data[0].saved_grpmod[0] = mmio_read_32(rdist_base + GICE_V3_IGRPMOD0);

	rdist_has_saved[platform_get_core_pos(read_mpidr())] = 1;
	int_schedule_mode_save();
}

/* TODO: check all registers to restore */
void gic_dist_restore(void)
{
	unsigned int gic_irqs;
	unsigned int dist_base;
	unsigned int ctlr;
	unsigned int rdist_sgi_base;
	unsigned int i = 0;

	dist_base = MT_GIC_BASE;//get_plat_config()->gicd_base;
	gic_irqs = MAX_IRQ_NR;

	/* get the base of redistributor first */
	if (gic_populate_rdist(&rdist_sgi_base) == -1) {
		return;
	}

	/* the SGI/PPI base is at 64K offset of current rdist base */
	rdist_sgi_base += SZ_64K;

	/* disable first before going on */
	ctlr = gicd_v3_read_ctlr(dist_base);
	ctlr &= ~(GICD_V3_CTLR_ENABLE_G0 | GICD_V3_CTLR_ENABLE_G1NS | GICD_V3_CTLR_ENABLE_G1S);
	gicd_v3_write_ctlr(dist_base, ctlr);
	gicd_v3_do_wait_for_rwp(dist_base);

	/* handle SGI & PPI */
	mmio_write_32(rdist_sgi_base + GICD_ICFGR, gic_data[0].saved_conf[0]);
	for (i = 0; i <= 7; ++i)
		mmio_write_32(rdist_sgi_base + GICD_IPRIORITYR + i * 4, gic_data[0].saved_priority[i]);
	mmio_write_32(rdist_sgi_base + GICD_ISENABLER, gic_data[0].saved_enable[0]);
	mmio_write_32(rdist_sgi_base + GICD_IGROUPR, gic_data[0].saved_group[0]);
	mmio_write_32(rdist_sgi_base + GICE_V3_IGRPMOD0, gic_data[0].saved_grpmod[0]);

	/* restore SPI setting */
	for (i = 1; i < DIV_ROUND_UP(gic_irqs, 16); i++)
		mmio_write_32(dist_base + GICD_ICFGR + i * 4, gic_data[0].saved_conf[i]);

	for (i = 8; i < DIV_ROUND_UP(gic_irqs, 4); i++)
		mmio_write_32(dist_base + GICD_IPRIORITYR + i * 4, gic_data[0].saved_priority[i]);

	for (i = 1; i < DIV_ROUND_UP(gic_irqs, 1); i++)
		mmio_write_64(dist_base + GICD_V3_IROUTER + i*8, gic_data[0].saved_target[i]);

	for (i = 1; i < DIV_ROUND_UP(gic_irqs, 32); i++)
		mmio_write_32(dist_base + GICD_ISENABLER + i * 4, gic_data[0].saved_enable[i]);

	for (i = 1; i < DIV_ROUND_UP(gic_irqs, 32); i++)
		mmio_write_32(dist_base + GICD_IGROUPR + i * 4, gic_data[0].saved_group[i]);

	for (i = 1; i < DIV_ROUND_UP(gic_irqs, 32); i++)
		mmio_write_32(dist_base + GICE_V3_IGRPMOD0 + i * 4, gic_data[0].saved_grpmod[i]);
#if CFG_MICROTRUST_TEE_SUPPORT
	for (i = 1; i < DIV_ROUND_UP(gic_irqs, 32); i++)
		mmio_write_32(dist_base + GICD_ISPENDR + i * 4, gic_data[0].saved_spi_pending[i]);
#endif
	/* enable all groups & ARE */
	ctlr = GICD_V3_CTLR_ENABLE_G0 | GICD_V3_CTLR_ENABLE_G1NS | GICD_V3_CTLR_ENABLE_G1S |
		GICD_V3_CTLR_ARE_S | GICD_V3_CTLR_ARE_NS;

	gicd_v3_write_ctlr(dist_base, ctlr);
	gicd_v3_do_wait_for_rwp(dist_base);
	int_schedule_mode_restore();
}

void gic_rdist_restore_all(void)
{
	int cpuid = 0;
	unsigned int base, i;
	unsigned int cpu_offset = 0;

	base = MT_GIC_RDIST_BASE;
	base += SZ_64K;

	for (cpuid = 0; cpuid < PLATFORM_CORE_COUNT; cpuid++) {
		cpu_offset = cpuid*SZ_64K*2;
		mmio_write_32(base + GICD_ICFGR + cpu_offset, gic_data[0].saved_conf[0]);
		for (i = 0; i <= 7; ++i)
			mmio_write_32(base + GICD_IPRIORITYR + cpu_offset + i * 4, gic_data[0].saved_priority[i]);
		mmio_write_32(base + GICD_ISENABLER + cpu_offset, gic_data[0].saved_enable[0]);
		mmio_write_32(base + GICD_IGROUPR + cpu_offset, gic_data[0].saved_group[0]);
		mmio_write_32(base + GICE_V3_IGRPMOD0 + cpu_offset, gic_data[0].saved_grpmod[0]);
	}
}

void gic_rdist_restore(void)
{
	unsigned int rdist_sgi_base, i;

	if (rdist_has_saved[platform_get_core_pos(read_mpidr())] == 0)
		return;

	/* get the base of redistributor first */
	if (gic_populate_rdist(&rdist_sgi_base) == -1) {
		return;
	}

	/* the SGI/PPI base is at 64K offset of current rdist base */
	rdist_sgi_base += SZ_64K;

	/* handle SGI & PPI */
	mmio_write_32(rdist_sgi_base + GICD_ICFGR, gic_data[0].saved_conf[0]);
	for (i = 0; i <= 7; ++i)
		mmio_write_32(rdist_sgi_base + GICD_IPRIORITYR + i * 4, gic_data[0].saved_priority[i]);
	mmio_write_32(rdist_sgi_base + GICD_ISENABLER, gic_data[0].saved_enable[0]);
	mmio_write_32(rdist_sgi_base + GICD_IGROUPR, gic_data[0].saved_group[0]);
	mmio_write_32(rdist_sgi_base + GICE_V3_IGRPMOD0, gic_data[0].saved_grpmod[0]);
}

static uint16_t compute_target_list(uint8_t *cpu, unsigned int map, uint64_t cluster_id)
{
	uint16_t tlist = 0;
	uint8_t target_cpu = 0;

	for (target_cpu = *cpu; target_cpu < PLATFORM_CORE_COUNT; ++target_cpu) {
		uint64_t mpidr = cpu_logical_map[target_cpu];
		uint64_t target_cluster_id = mpidr & ~0xffUL;

		/* only check those cores in map */
		if ((((1 << target_cpu) & map) == 0) ||
			cpu_logical_map[target_cpu] == CPU_LOGIC_MAP_RESET_VAL)
			continue;
		/* gic-500 only support 16 cores in a cluster at max */
		if ((mpidr & 0xff) > 16) {
			ERROR("%s:%d: wrong cpu[%d] mpidr(%lx)\n",
				__func__, __LINE__, target_cpu, mpidr);
			break;
		}

		/* we assume cluster_id only changes _WHEN_
		 * all cores in the same cluster is counted */
		if (target_cluster_id != cluster_id) {
			break;
		} else {
			/* the core in the same cluster, add it. */
			tlist |= 1 << (mpidr & 0xff);
		}
	}

	/* update cpu for the next cluster */
	*cpu = target_cpu - 1;
	return tlist;
}

static void gic_send_sgi(uint64_t cluster_id, uint16_t tlist, unsigned int irq)
{
        uint64_t val = 0;

        val = irq << 24					|
        	(((cluster_id >> 8) & 0xff) << 16)	|
              		tlist;
#ifdef GIC_DEBUG
	INFO("\n\n### %lx ###\n\n", val);
#endif
        gicc_write_sgi0_el1(val);
}

static void dump_cpu_map(void)
{
#ifdef GIC_DEBUG
	int i = 0;

	for (;i < PLATFORM_CORE_COUNT; ++i)
		INFO("\n\n@@ cpu(%d) = %lx\n\n", i, cpu_logical_map[i]);
#endif
}

void irq_raise_softirq(unsigned int map, unsigned int irq)
{
	uint8_t cpu = 0;
	uint16_t tlist = 0;
	uint64_t cluster_id = 0;
	uint64_t mpidr = 0;

	dump_cpu_map();

	/*
         * Ensure that stores to Normal memory are visible to the
         * other CPUs before issuing the IPI.
         */
	__asm__ volatile("dmb sy" : : : "memory");

	for (cpu = 0; cpu < PLATFORM_CORE_COUNT ; ++cpu) {
		/* only check those cores in map */
		if ((((1 << cpu) & map) == 0) ||
				(cpu_logical_map[cpu] == CPU_LOGIC_MAP_RESET_VAL)) {
			continue;
		}

		/* gicv3 can only send sgi per cluster,
		 * gather all cores in map in the same cluster */
		mpidr = cpu_logical_map[cpu];
		cluster_id = mpidr & ~0xffUL;
		tlist = compute_target_list(&cpu, map, cluster_id);
#ifdef GIC_DEBUG
		INFO("\n\n%s:%d: mpidr = %lx, cluster_id = %x, tlist = %x\n\n",
			__func__, __LINE__, mpidr, cluster_id, tlist);
		INFO("\n\n%s:%d: cpu = %d\n\n", __func__, __LINE__, cpu);
#endif
		gic_send_sgi(cluster_id, tlist, irq);
	}

	/* Force the above writes to ICC_SGI0R_EL1 to be executed */
	isb();
}

unsigned int get_ack_info(void)
{
	return (gicc_read_iar0_el1()&0x3ff);
}

void ack_sgi(unsigned int irq)
{
	gicc_write_eoi0_el1(irq);
}

void mt_atf_trigger_irq()
{
	/* TODO */
#if 0
    struct atf_arg_t *teearg = &gteearg;
    gicd_set_ispendr(MT_GIC_BASE, teearg->atf_irq_num);
#endif
}

void mask_wdt_fiq()
{
	gicd_set_icenabler(MT_GIC_BASE, WDT_IRQ_BIT_ID);
}


//===========================================================================
// Set WDT interrupt pending. It is for simulation of WDT when processing SMC
// "AEEDump".
//===========================================================================
void mt_atf_trigger_WDT_FIQ()
{
	gicd_set_ispendr(MT_GIC_BASE, WDT_IRQ_BIT_ID);
}

/*******************************************************************************
 * Place the cpu interface in a state where it can never make a cpu exit wfi as
 * as result of an asserted interrupt. This is critical for powering down a cpu
 ******************************************************************************/
void gic_cpuif_deactivate(unsigned int gicc_base)
{
	unsigned int val;
	unsigned int rdist_base;

	/* disable group0 */
	gicc_write_grpen0_el1(0x0);

	/*
	 * When taking CPUs down we need to set GICR_WAKER.ProcessorSleep and
	 * wait for GICR_WAKER.ChildrenAsleep to get set.
	 * (GICv3 Architecture specification 5.4.23).
	 * GICR_WAKER is NOT banked per CPU, compute the correct base address
	 * per CPU.
	 */
	if (gic_populate_rdist(&rdist_base) == -1) {
		ERROR("%s:%d: can not get redistributor's base\n", __func__, __LINE__);
		panic();
	}

	val = mmio_read_32(rdist_base+GICR_V3_WAKER);
	val |= GICR_V3_WAKER_ProcessorSleep;
	mmio_write_32(rdist_base+GICR_V3_WAKER, val);
	dsb();

	/* wait until our children sleep */
	while ((mmio_read_32(rdist_base + GICR_V3_WAKER) & GICR_V3_WAKER_ChildrenAsleep) == 0);
}


void mt_irq_set_polarity(unsigned int irq, unsigned int polarity)
{
	/* TODO: segmented pol setting */

	unsigned int offset, reg_index, value;

	offset = (irq - GIC_PRIVATE_SIGNALS) & 0x1F;
	reg_index = (irq - GIC_PRIVATE_SIGNALS) >> 5;

	if (polarity == 0) {
		/* active low */
		value = mmio_read_32(INT_POL_CTL0 + (reg_index * 4));
		value |= (1 << offset);
		mmio_write_32((INT_POL_CTL0 + (reg_index * 4)), value);
	} else {
		/* active high */
		value = mmio_read_32(INT_POL_CTL0 + (reg_index * 4));
		value &= ~(0x1 << offset);
		mmio_write_32(INT_POL_CTL0 + (reg_index * 4), value);
	}
}

void mt_irq_set_sens(unsigned int base, unsigned int irq, unsigned int sens)
{
	unsigned int config;

	if (sens == MT_EDGE_SENSITIVE) {
		config = mmio_read_32(base + GICD_ICFGR + (irq / 16) * 4);
		config |= (0x2 << (irq % 16) * 2);
		mmio_write_32(base + GICD_ICFGR + (irq / 16) * 4, config);
	} else {
		config = mmio_read_32(base + GICD_ICFGR + (irq / 16) * 4);
		config &= ~(0x2 << (irq % 16) * 2);
		mmio_write_32(base + GICD_ICFGR + (irq / 16) * 4, config);
	}
}

static void enable_sgi_fiq(uint32_t fiq_num)
{
	unsigned int rdist_sgi_base = 0;

	assert (gic_populate_rdist((unsigned int *)&rdist_sgi_base) != -1);
#if !defined(SPD_trusty)
	/* set all SGI/PPI as NS-group1 by default */
	gicr_write_igroupr0(rdist_sgi_base, ~0x0);
#endif
	gicv3_secure_ppi_sgi_configure(rdist_sgi_base, 1, &fiq_num, INTR_GROUP0);
}

int gic_cpuif_init(void)
{
	unsigned int rdist_base = 0;
	unsigned int val = 0;
	unsigned int scr_val = 0;
	uint64_t mpidr = 0;
	uint32_t cpu = 0;
	unsigned int index;

	if (gic_populate_rdist(&rdist_base) == -1)
		return -1;

	/* Wake up this CPU redistributor */
	val = mmio_read_32(rdist_base+GICR_V3_WAKER);
	val &= ~GICR_V3_WAKER_ProcessorSleep;
	mmio_write_32(rdist_base+GICR_V3_WAKER, val);

	/* wait until our children wakeup */
	while (mmio_read_32(rdist_base + GICR_V3_WAKER) &
			GICR_V3_WAKER_ChildrenAsleep);

	/* set all SGI/PPI as non-secure GROUP1 by default.
	   rdist_base + 64K == SGI_base */
	mmio_write_32(rdist_base+SZ_64K+GICE_V3_IGROUP0, 0xffffffff);
	mmio_write_32(rdist_base+SZ_64K+GICE_V3_IGRPMOD0, 0x0);

	/*
	 * Setup the default PPI/SGI priorities.
	 * 0xD0 is the priority from kernel
	 */
	for (index = 0; index < MIN_SPI_ID; index++)
		gicd_set_ipriorityr(rdist_base+SZ_64K, index, 0xD0);
	/*
	 * We need to set SCR_EL3.NS in order to see GICv3 non-secure state.
	 * Restore SCR_EL3.NS again before exit.
	 */
	scr_val = read_scr();
	write_scr(scr_val | SCR_NS_BIT);
	isb();	/* ensure NS=1 takes effect before accessing ICC_SRE_EL2 */

	/* enable SRE bit in ICC_SRE_ELx in order */
	val = read_icc_sre_el3();
	write_icc_sre_el3(val | ICC_SRE_EN_BIT | ICC_SRE_SRE_BIT);
	isb(); /* before enable lower SRE, be sure SRE in el3 takes effect */

	val = read_icc_sre_el2();
	write_icc_sre_el2(val | ICC_SRE_EN_BIT | ICC_SRE_SRE_BIT);
	isb(); /* before enable lower SRE, be sure SRE in el2 takes effect */

	write_icc_pmr_el1(GIC_PRI_MASK);
	isb();	/* commite ICC_* changes before setting NS=0 */

	/* Restore SCR_EL3 */
	write_scr(scr_val);
	isb();	/* ensure NS=0 takes effect immediately */

	/* MUST set secure copy of icc_sre_el1 as SRE_SRE to enable FIQ,
	see GICv3 spec 4.6.4 FIQ Enable */
	val = read_icc_sre_el1();
	write_icc_sre_el1(val | ICC_SRE_SRE_BIT);
	isb(); /* before we can touch other ICC_* system registers, make sure this have effect */

	/* here we go, can handle FIQ after this */
	gicc_write_grpen0_el1(0x1);
	/* allow S-EL1 can get group1 irq */
	gicc_write_grpen1_el1(0x1);
#if defined(SPD_trusty)
	gicc_write_grpen1_el3(0x1);
#endif

	/* everytime we setup the cpu IF, add a SGI as FIQ for smp call debug */
	enable_sgi_fiq(FIQ_SMP_CALL_SGI);

	/* init mpidr table for this cpu for later sgi usage */
	mpidr = read_mpidr();
	cpu = platform_get_core_pos(mpidr);
	if (cpu < PLATFORM_CORE_COUNT)
		cpu_logical_map[cpu] = mpidr;

	return 0;
}

static void gic_distif_init(unsigned int gicd_base)
{
	unsigned int ctlr = 0;
	unsigned int irq_set = 0;
	unsigned int i = 0;

	/* disable first before going on */
	ctlr = gicd_v3_read_ctlr(gicd_base);
	ctlr &= ~(GICD_V3_CTLR_ENABLE_G0 | GICD_V3_CTLR_ENABLE_G1NS | GICD_V3_CTLR_ENABLE_G1S);
	gicd_v3_write_ctlr(gicd_base, ctlr);
	gicd_v3_do_wait_for_rwp(gicd_base);

	/* query how many irq sets in our implementation,
	 * the formula is found in section 4.3.2 GIC400 TRM */
	irq_set = (mmio_read_32(gicd_base+GICD_V3_TYPER)&0x1f)+1;

	/* set all SPI as non-secure group1 by default,
	 * index from 1, because GICE_V3_IGROUP0, GICE_V3_IGRPMOD0 are RES0,
	 * equivalent function is provided by GICR_IGROUPR0, GICE_V3_IGRPMOD0,
	 * which are both initialized in gic_cpuif_init() */
	for (i = 1; i < irq_set; i++) {
		mmio_write_32(gicd_base+GICE_V3_IGROUP0+(i<<2), 0xffffffff);
		mmio_write_32(gicd_base+GICE_V3_IGRPMOD0+(i<<2), 0x0);
	}

	/* enable all groups & ARE */
	ctlr = GICD_V3_CTLR_ENABLE_G0 | GICD_V3_CTLR_ENABLE_G1NS | GICD_V3_CTLR_ENABLE_G1S |
		GICD_V3_CTLR_ARE_S | GICD_V3_CTLR_ARE_NS;

	gicd_v3_write_ctlr(gicd_base, ctlr);
	gicd_v3_do_wait_for_rwp(gicd_base);
}

uint64_t gic_mpidr_to_affinity(uint64_t mpidr)
{
        uint64_t aff;

        aff = (MPIDR_AFFINITY_LEVEL(mpidr, 3) << 32 |
               MPIDR_AFFINITY_LEVEL(mpidr, 2) << 16 |
               MPIDR_AFFINITY_LEVEL(mpidr, 1) << 8  |
               MPIDR_AFFINITY_LEVEL(mpidr, 0));

        return aff;
}

void mt_bl31_spi_enable(uint32_t fiq_num, uint32_t int_group, uint32_t trigger_type) {

	uintptr_t gicd_base = MT_GIC_BASE;

	assert(fiq_num >= GIC_PRIVATE_SIGNALS);
	/* Configure this interrupt as a secure interrupt */
	gicd_clr_igroupr(gicd_base, fiq_num);

	/* Configure this interrupt as G0 or a G1S interrupt */
	if (int_group == INTR_GROUP1S)
		gicd_set_igrpmodr(gicd_base, fiq_num);
	else
		gicd_clr_igrpmodr(gicd_base, fiq_num);

	/* give FIQ the highest priority */
	gicd_set_ipriorityr(gicd_base, fiq_num, GIC_HIGHEST_SEC_PRIORITY);
	/* use 1-N model, means as long as one of N core can handle, this will be handled */
	gicd_write_irouter(gicd_base, fiq_num, GICD_V3_IROUTER_SPI_MODE_ANY);

	/* set trigger type, falling edage */
	mt_irq_set_sens(gicd_base, fiq_num, GET_INT_EDGE_TYPE(trigger_type));
	mt_irq_set_polarity(fiq_num, GET_INT_POLARITY_TYPE(trigger_type));

	gicd_set_isenabler(gicd_base, fiq_num);
}

#if CFG_MICROTRUST_TEE_SUPPORT
void teei_gic_setup();
#endif

void setup_int_schedule_mode(enum irq_schedule_mode mode, unsigned int active_cpu)
{
	assert(mode <= HW_MODE);
	assert(active_cpu <= 0xFF);

	if(mode == HW_MODE)
	{
		mmio_write_32(GIC_INT_MASK, (mmio_read_32(GIC_INT_MASK) & ~(GIC500_ACTIVE_SEL_MASK)) | (0x1 << GIC500_ACTIVE_SEL_SHIFT));
		mmio_write_32(GIC_INT_MASK, (mmio_read_32(GIC_INT_MASK) & ~(GIC500_ACTIVE_CPU_MASK)) | (active_cpu << GIC500_ACTIVE_CPU_SHIFT));
	}
	else if(mode == SW_MODE)
	{
		mmio_write_32(GIC_INT_MASK, (mmio_read_32(GIC_INT_MASK) & ~(GIC500_ACTIVE_SEL_MASK)));
		mmio_write_32(GIC_INT_MASK, (mmio_read_32(GIC_INT_MASK) & ~(GIC500_ACTIVE_CPU_MASK)) | (active_cpu << GIC500_ACTIVE_CPU_SHIFT));
	}
	return;
}

void int_schedule_mode_save(void)
{
	gic_data[0].saved_active_sel = mmio_read_32(GIC_INT_MASK);
}

void int_schedule_mode_restore(void)
{
	mmio_write_32(GIC_INT_MASK, gic_data[0].saved_active_sel);
}

void gic_setup(void)
{
	unsigned int gicd_base = 0;
	int	i;

	/* Init cpu logic map to avoid unnecessary SGI interrupt
		to cpu0 if WDT happens before cpu1~cpu7 on */
	for(i = 0 ; i < sizeof(cpu_logical_map)/sizeof(uint64_t); i++)
		cpu_logical_map[i] = CPU_LOGIC_MAP_RESET_VAL;
	gicd_base = MT_GIC_BASE;//get_plat_config()->gicd_base;
	gic_distif_init(gicd_base);
	gic_cpuif_init();
#if CFG_MICROTRUST_TEE_SUPPORT
    teei_gic_setup();
#endif
	/* Register WDT handler */
	request_fiq(WDT_IRQ_BIT_ID, aee_wdt_dump_all_core, INT_LEVEL_LOW, INTR_GROUP0, NULL);
	/* Register PCCIF1 IRQ0 and IRQ1 handler */
	request_fiq(PCCIF1_IRQ0_BIT_ID, ccif_irq0_handler, INT_LEVEL_LOW, INTR_GROUP0, NULL);
	request_fiq(PCCIF1_IRQ1_BIT_ID, ccif_irq1_handler, INT_LEVEL_LOW, INTR_GROUP0, NULL);
	setup_int_schedule_mode(SW_MODE, 0xF);
}

/*******************************************************************************
 * An ARM processor signals interrupt exceptions through the IRQ and FIQ pins.
 * The interrupt controller knows which pin/line it uses to signal a type of
 * interrupt. The platform knows which interrupt controller type is being used
 * in a particular security state e.g. with an ARM GIC, normal world could use
 * the GICv2 features while the secure world could use GICv3 features and vice
 * versa.
 * This function is exported by the platform to let the interrupt management
 * framework determine for a type of interrupt and security state, which line
 * should be used in the SCR_EL3 to control its routing to EL3. The interrupt
 * line is represented as the bit position of the IRQ or FIQ bit in the SCR_EL3.
 ******************************************************************************/
uint32_t plat_interrupt_type_to_line(uint32_t type, uint32_t security_state)
{
	assert(type == INTR_TYPE_S_EL1 ||
	       type == INTR_TYPE_EL3 ||
	       type == INTR_TYPE_NS);

	assert(security_state == NON_SECURE || security_state == SECURE);

	/*
	 * We ignore the security state parameter under the assumption that
	 * both normal and secure worlds are using ARM GICv2. This parameter
	 * will be used when the secure world starts using GICv3.
	 */
	/* Non-secure interrupts are signalled on the IRQ line always */
	if (type == INTR_TYPE_NS)
		return __builtin_ctz(SCR_IRQ_BIT);

	/* since we have enabled FIQ and trap FIQ in EL3 only,
	 * when type is not INTR_TYPE_NS, should
	 */
	return __builtin_ctz(SCR_FIQ_BIT);
}

/*******************************************************************************
 * This function returns the type of the highest priority pending interrupt at
 * the GIC cpu interface. INTR_TYPE_INVAL is returned when there is no
 * interrupt pending.
 ******************************************************************************/
uint32_t plat_ic_get_pending_interrupt_type()
{
	uint64_t irq = 0;

	irq = gicc_read_hppir0_el1()&0x3FF;
	switch (irq) {
		case PENDING_G1S_INTID:
		return INTR_TYPE_S_EL1;
		case PENDING_G1NS_INTID:
			return INTR_TYPE_NS;
		case GIC_SPURIOUS_INTERRUPT:
		return INTR_TYPE_INVAL;
		default:
			return INTR_TYPE_EL3;
	}
}

/*******************************************************************************
 * This function returns the id of the highest priority pending interrupt at
 * the GIC cpu interface. INTR_ID_UNAVAILABLE is returned when there is no
 * interrupt pending.
 ******************************************************************************/
uint32_t plat_ic_get_pending_interrupt_id()
{
	uint32_t irq = 0;

	irq = gicc_read_hppir0_el1()&0x3FF;
	if (irq < PENDING_G1S_INTID) {
		return irq;
	}

	if (irq == GIC_SPURIOUS_INTERRUPT) {
		return INTR_ID_UNAVAILABLE;
	}

	/*
	 * Find out which non-secure interrupt it is under the assumption that
	 * the ICC_CTLR_EL3.RM is 0
	 */
	return (gicc_read_hppir1_el1()&0x3ff);
}

/*******************************************************************************
 * This functions reads the GIC cpu interface Interrupt Acknowledge register
 * to start handling the pending interrupt. It returns the contents of the IAR.
 ******************************************************************************/
uint32_t plat_ic_acknowledge_interrupt()
{
	return gicc_read_iar0_el1();
}

/*******************************************************************************
 * This functions writes the GIC cpu interface End Of Interrupt register with
 * the passed value to finish handling the active interrupt
 ******************************************************************************/
void plat_ic_end_of_interrupt(uint32_t id)
{
	gicc_write_eoi0_el1(id);
}

/*******************************************************************************
 * This function returns the type of the interrupt id depending upon the group
 * this interrupt has been configured under by the interrupt controller i.e.
 * group0 or group1.
 ******************************************************************************/
uint32_t plat_ic_get_interrupt_type(uint32_t id)
{
	uint32_t group;

	group = gicd_get_igroupr(MT_GIC_BASE, id);

	/* Assume that all secure interrupts are S-EL1 interrupts */
	if (group == GRP0)
		return INTR_TYPE_S_EL1;
	else
		return INTR_TYPE_NS;
}
#define GIC_DEBUG
uint64_t mt_irq_dump_status(uint32_t irq)
{
	uint32_t dist_base;
	unsigned int bit;
	uint32_t result;
	uint64_t rc = 0;
#ifdef GIC_DEBUG
	NOTICE("[ATF GIC dump] irq = %d\n", irq);
#endif
	if (irq < 32) {
		return 0;
	}

	if (irq > 1020) {
		return 0;
	}

	dist_base = MT_GIC_BASE;//get_plat_config()->gicd_base;

	/* get mask */
	bit = 1 << (irq % 32);
	result = ((mmio_read_32(dist_base + GICD_ISENABLER + irq / 32 * 4) & bit)?1:0);
#ifdef GIC_DEBUG
	NOTICE("[ATF GIC dump] enable = %x\n", result);
#endif
	rc |= result;

	/*get group*/
	bit = 1 << (irq % 32);
	//0x1:irq,0x0:fiq
	result = ((mmio_read_32(dist_base + GICD_IGROUPR + irq / 32 * 4) & bit)?1:0);
#ifdef GIC_DEBUG
	NOTICE("[ATF GIC dump] group = %x (0x1:irq,0x0:fiq)\n", result);
#endif
	rc |=  result << 1;

	/* get priority */
	bit = 0xff << ((irq % 4)*8);
	result = ((mmio_read_32(dist_base + GICD_IPRIORITYR + irq / 4 * 4) & bit) >> ((irq % 4)*8));
#ifdef GIC_DEBUG
	NOTICE("[ATF GIC dump] priority = %x\n", result);
#endif
	rc |= result << 2;

	/* get sensitivity */
	bit = 0x3 << ((irq % 16)*2);
	//edge:0x2, level:0x1
	result = ((mmio_read_32(dist_base + GICD_ICFGR + irq / 16 * 4) & bit) >> ((irq % 16)*2));
#ifdef GIC_DEBUG
	NOTICE("[ATF GIC dump] sensitivity = %x (edge:0x1, level:0x0)\n", result>>1);
#endif
	rc |= (result>>1) << 10;

	/* get pending status */
	bit = 1 << (irq % 32);
	result = ((mmio_read_32(dist_base + GICD_ISPENDR + irq / 32 * 4) & bit)?1:0);
#ifdef GIC_DEBUG
	NOTICE("[ATF GIC dump] pending status = %x\n", result);
#endif
	rc |= result << 11;

	/* get active status */
	bit = 1 << (irq % 32);
	result = ((mmio_read_32(dist_base + GICD_ISACTIVER + irq / 32 * 4) & bit)?1:0);
#ifdef GIC_DEBUG
	NOTICE("[ATF GIC dump] active status = %x\n", result);
#endif
	rc |= result << 12;

	/* get polarity */
	{
		uint32_t reg = ((irq-32)/32);

		if (reg >= 8) {
			reg -= 8;
			reg += 0x70/4;
		}

		bit = 1 << (irq % 32);
		//0x0: high, 0x1:low
		result = ((mmio_read_32(INT_POL_CTL0 + reg*4) & bit)?1:0);
#ifdef GIC_DEBUG
		NOTICE("[ATF GIC dump] polarity = %x (0x0: high, 0x1:low)\n", result);
#endif
		rc |= result << 13;
	}

	/* get router, occupy 16bit in rc [14:29] */
	{
		uint64_t route = mmio_read_64(dist_base + GICD_V3_IROUTER + (irq*8));
		uint16_t cpu = 4*((route&0xff00)>>8) + (route&0xff);

		if (route & GICD_V3_IROUTER_SPI_MODE_ANY) {
			rc |= 0xffff << 14;
		} else {
			rc |= (1<<cpu) << 14;
		}
	}

	return rc;
}
#undef GIC_DEBUG
void dump_gic(void)
{
	unsigned int cpuid = 0;
	unsigned int base = 0;
	unsigned int scr_el3;

	scr_el3 = read_scr_el3();
	INFO("SCR_EL3:0x%x\n", scr_el3);

	INFO("GICD_CTLR:0x%x\n", mmio_read_32(MT_GIC_BASE + GICD_CTLR));
	INFO("ICC_CTLR_EL1:0x%lx\n", read_icc_ctlr_el1());
	INFO("ICC_CTLR_EL3:0x%lx\n", read_icc_ctlr_el3());
	INFO("ICC_PMR_EL1:0x%lx\n", read_icc_pmr_el1());
	INFO("ICC_SRE_EL1:0x%lx\n", read_icc_sre_el1());
	INFO("ICC_SRE_EL2:0x%lx\n", read_icc_sre_el2());
	INFO("ICC_SRE_EL3:0x%lx\n", read_icc_sre_el3());
	INFO("ICC_IGRPEN0_EL1:0x%lx\n", read_icc_igrpen0_el1());
	INFO("ICC_IGRPEN1_EL3:0x%lx\n", read_icc_igrpen1_el3());
	for (cpuid = 0 ; cpuid < PLATFORM_CORE_COUNT ; cpuid++) {
		base = MT_GIC_RDIST_BASE + cpuid*SZ_64K*2;
		INFO("cpu[%d]GICR_IGROUPR0:0x%x\n", cpuid, mmio_read_32(base + GICR_IGROUPR0));
		INFO("cpu[%d]GICR_IGRPMODR0:0x%x\n", cpuid, mmio_read_32(base + GICR_IGRPMODR0));
		INFO("cpu[%d]GICR_CTLR:0x%x\n", cpuid, mmio_read_32(base + GICR_CTLR));
	}
}

void check_wdt_sgi_routing(void)
{
	cpu_context_t *ctx;
	unsigned int scr_el3;
	uintptr_t gicd_base;
	unsigned int gicr_base;
	unsigned int id;
	unsigned int cpuid;
	unsigned int wdt_igroup0_reg;
	unsigned int wdt_igroup0_mod_reg;
	unsigned int sgi_igroup0_reg;
	unsigned int sgi_igroup0_mod_reg;
	unsigned int bit_num;

	ctx = cm_get_context(NON_SECURE);
	scr_el3 = read_ctx_reg(get_el3state_ctx(ctx),CTX_SCR_EL3);
	NOTICE("NS-SCR_EL3=0x%x\n", scr_el3 & SCR_VALID_BIT_MASK);

#if !defined(SPD_none)
	ctx = cm_get_context(SECURE);
	if (NULL != ctx){
		scr_el3 = read_ctx_reg(get_el3state_ctx(ctx),CTX_SCR_EL3);
		NOTICE("S-SCR_EL3=0x%x\n", scr_el3 & SCR_VALID_BIT_MASK);
	}
#endif

	id = WDT_IRQ_BIT_ID;
	gicd_base = MT_GIC_BASE;

	wdt_igroup0_reg = gicd_read_igroupr(gicd_base, id);
	wdt_igroup0_mod_reg = gicd_read_igrpmodr(gicd_base, id);
/*	NOTICE("WDT(%d) GICD IGROUPR0:0x%x, IGRPMODR0:0x%x\n", id
								,wdt_igroup0_reg
								,wdt_igroup0_mod_reg);
*/

	bit_num = id & ((1 << IGROUPR_SHIFT) - 1);
	if (!((0 == (wdt_igroup0_reg & (1 << bit_num))) &&
	    (0 == (wdt_igroup0_mod_reg & (1 << bit_num))))){
		NOTICE("WDT(%d) is not Grp0, S-EL1 should handle it\n", id);
#if !defined(SPD_tbase)
		/* bypass tbase only */
		assert(0);
#endif
	}

	id = FIQ_SMP_CALL_SGI;
	for(cpuid = 0 ; cpuid < PLATFORM_CORE_COUNT ; cpuid++)
	{
		gicr_base = MT_GIC_RDIST_BASE + cpuid*SZ_64K*2;
		sgi_igroup0_reg = mmio_read_32(gicr_base + GICR_IGROUPR0);
		sgi_igroup0_mod_reg = mmio_read_32(gicr_base + GICR_IGRPMODR0);
/*		NOTICE("cpu[%d],SGI(%d),GICR IGROUPR0:0x%x, IGRPMODR0:0x%x\n"
							,cpuid
							,id
							,sgi_igroup0_reg
							,sgi_igroup0_mod_reg);
*/
		bit_num = id & ((1 << IGROUPR_SHIFT) - 1);
		if (!((0 == (sgi_igroup0_reg & (1 << bit_num))) &&
		    (0 == (sgi_igroup0_mod_reg & (1 << bit_num))))){
			NOTICE("cpu[%d],SGI(%d) is not Grp0, S-EL1 should handle it\n"
							,cpuid, id);
#if !defined(SPD_tbase)
			/* bypass tbase only */
			assert(0);
#endif
		}
	}
}

void gic_sync_dcm_enable(void)
{
	unsigned int val = mmio_read_32(GIC_SYNC_DCM);

	val &= ~GIC_SYNC_DCM_MASK;
	mmio_write_32(GIC_SYNC_DCM, val | GIC_SYNC_DCM_ON);
}

void gic_sync_dcm_disable(void)
{
	unsigned int val = mmio_read_32(GIC_SYNC_DCM);

	val &= ~GIC_SYNC_DCM_MASK;
	mmio_write_32(GIC_SYNC_DCM, val | GIC_SYNC_DCM_OFF);
}
