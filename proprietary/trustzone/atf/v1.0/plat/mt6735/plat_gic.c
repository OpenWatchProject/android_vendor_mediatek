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
#include <debug.h>
#include <gic_v2.h>
#include <gic_v3.h>
#include <interrupt_mgmt.h>
#include <platform.h>
#include <plat_config.h>
#include <stdint.h>
#include "plat_def.h"
#include "plat_private.h"
#include <platform_def.h>
#if CFG_MICROTRUST_TEE_SUPPORT
#include "plat_teei.h"
#endif
#include <stdio.h>  //for printf

#ifndef MAX_GIC_NR
#define MAX_GIC_NR	1
#endif

#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))

/* For saving ATF size, we reduce 1020 -> 320 */
struct gic_chip_data {
	unsigned int saved_spi_enable[DIV_ROUND_UP(320, 32)];
	unsigned int saved_spi_conf[DIV_ROUND_UP(320, 16)];
	unsigned int saved_spi_target[DIV_ROUND_UP(320, 4)];
	unsigned int saved_spi_group[DIV_ROUND_UP(320, 32)];
	unsigned int saved_spi_pending[DIV_ROUND_UP(320, 32)];
	unsigned int saved_spi_priority[DIV_ROUND_UP(320, 4)];
};

/*
struct gic_cpuif_data{
	unsigned int saved_ppi_enable[DIV_ROUND_UP(32, 32)];
	unsigned int saved_ppi_conf[DIV_ROUND_UP(32, 16)];
};
*/

static struct gic_chip_data gic_data[MAX_GIC_NR];
//static struct gic_cpuif_data cpuif_data[PLATFORM_CORE_COUNT];

#if CFG_MICROTRUST_TEE_SUPPORT
void gicc_write_grpen1_el1(uint64_t val)
{
	unsigned int gicc_base, pre;

	gicc_base = get_plat_config()->gicc_base;

	if (val) {
		pre = gicc_read_ctlr(gicc_base);
		pre = pre | (1<<1);
		gicc_write_ctlr(gicc_base, pre);
	} else {
		pre = gicc_read_ctlr(gicc_base);
		pre = pre & (~(1<<1));
		gicc_write_ctlr(gicc_base, pre);
	}
}

void gicd_v3_do_wait_for_rwp(unsigned int gicd_base)
{

}

#define SZ_64K			(0x00010000)
int gic_populate_rdist(unsigned int *rdist_base)
{
	*rdist_base = get_plat_config()->gicd_base - SZ_64K;
	return 0;
}

uint64_t gic_mpidr_to_affinity(uint64_t mpidr)
{
	unsigned int aff = platform_get_core_pos(mpidr);

	return (uint64_t)aff;
}
#endif

void gic_dist_save(void)
{
	unsigned int gic_irqs;
	unsigned int dist_base;
	int i;

	//dist_base = mt_get_cfgvar(CONFIG_GICD_ADDR);
	dist_base = get_plat_config()->gicd_base;

	gic_irqs = 32 * ((gicd_read_typer(dist_base) & IT_LINES_NO_MASK) + 1);

	for (i = 0; i < DIV_ROUND_UP(gic_irqs, 16); i++)
		gic_data[0].saved_spi_conf[i] =
			mmio_read_32(dist_base + GICD_ICFGR + i * 4);

	for (i = 0; i < DIV_ROUND_UP(gic_irqs, 4); i++)
		gic_data[0].saved_spi_priority[i] =
			mmio_read_32(dist_base + GICD_IPRIORITYR + i * 4);

	for (i = 0; i < DIV_ROUND_UP(gic_irqs, 4); i++)
		gic_data[0].saved_spi_target[i] =
			mmio_read_32(dist_base + GICD_ITARGETSR + i * 4);

	for (i = 0; i < DIV_ROUND_UP(gic_irqs, 32); i++)
		gic_data[0].saved_spi_enable[i] =
			mmio_read_32(dist_base + GICD_ISENABLER + i * 4);

	for (i = 0; i < DIV_ROUND_UP(gic_irqs, 32); i++)
		gic_data[0].saved_spi_group[i] =
			mmio_read_32(dist_base + GICD_IGROUPR + i * 4);

	for (i = 0; i < DIV_ROUND_UP(gic_irqs, 32); i++)
		gic_data[0].saved_spi_pending[i] =
			mmio_read_32(dist_base + GICD_ISPENDR + i * 4);
}

void gic_dist_restore(void)
{
	unsigned int gic_irqs;
	unsigned int dist_base;
	unsigned int ctlr;
	int i;

//	dist_base = mt_get_cfgvar(CONFIG_GICD_ADDR);
	dist_base = get_plat_config()->gicd_base;

	gic_irqs = 32 * ((gicd_read_typer(dist_base) & IT_LINES_NO_MASK) + 1);

	/* Disable the distributor before going further */
	ctlr = gicd_read_ctlr(dist_base);
	ctlr &= ~(ENABLE_GRP0 | ENABLE_GRP1);
	gicd_write_ctlr(dist_base, ctlr);


	for (i = 0; i < DIV_ROUND_UP(gic_irqs, 16); i++)
		mmio_write_32(dist_base + GICD_ICFGR + i * 4, gic_data[0].saved_spi_conf[i]);

	for (i = 0; i < DIV_ROUND_UP(gic_irqs, 4); i++)
		mmio_write_32(dist_base + GICD_IPRIORITYR + i * 4, gic_data[0].saved_spi_priority[i]);

	for (i = 0; i < DIV_ROUND_UP(gic_irqs, 4); i++)
		mmio_write_32(dist_base + GICD_ITARGETSR + i * 4, gic_data[0].saved_spi_target[i]);

	for (i = 0; i < DIV_ROUND_UP(gic_irqs, 32); i++)
		mmio_write_32(dist_base + GICD_ISENABLER + i * 4, gic_data[0].saved_spi_enable[i]);

	for (i = 0; i < DIV_ROUND_UP(gic_irqs, 32); i++)
		mmio_write_32(dist_base + GICD_IGROUPR + i * 4, gic_data[0].saved_spi_group[i]);

	for (i = 0; i < DIV_ROUND_UP(gic_irqs, 32); i++)
		mmio_write_32(dist_base + GICD_ISPENDR + i * 4, gic_data[0].saved_spi_pending[i]);


	gicd_write_ctlr(dist_base, ctlr | ENABLE_GRP0  | ENABLE_GRP1);
}

/*
void gic_cpu_save(void)
{
	int i;
	unsigned int dist_base;
	uint64_t mpidr;
	int cpuid;

	mpidr = read_mpidr();
	cpuid = platform_get_core_pos(mpidr);

	dist_base = mt_get_cfgvar(CONFIG_GICD_ADDR);

	for (i = 0; i < DIV_ROUND_UP(32, 32); i++)
		cpuif_data[cpuid].saved_ppi_enable[i] = mmio_read_32(dist_base + GICD_ISENABLER + i * 4);

	for (i = 0; i < DIV_ROUND_UP(32, 16); i++)
		cpuif_data[cpuid].saved_ppi_conf[i] = mmio_read_32(dist_base + GICD_ICFGR + i * 4);
}

void gic_cpu_restore(void)
{
	int i;
	unsigned int dist_base, cpu_base;
	uint64_t mpidr;
	int cpuid;
	unsigned int val;

	mpidr = read_mpidr();
	cpuid = platform_get_core_pos(mpidr);

	dist_base = mt_get_cfgvar(CONFIG_GICD_ADDR);
	cpu_base = mt_get_cfgvar(CONFIG_GICC_ADDR);

	for (i = 0; i < DIV_ROUND_UP(32, 32); i++)
		mmio_write_32(dist_base + GICD_ISENABLER + i * 4, cpuif_data[cpuid].saved_ppi_enable[i]);

	for (i = 0; i < DIV_ROUND_UP(32, 16); i++)
		mmio_write_32(dist_base + GICD_ICFGR + i * 4, cpuif_data[cpuid].saved_ppi_conf[i]);

	for (i = 0; i < DIV_ROUND_UP(32, 4); i++)
		mmio_write_32(dist_base + GICD_IPRIORITYR + i * 4, 0xa0a0a0a0);

	val = ENABLE_GRP0 | ENABLE_GRP1 | FIQ_EN | FIQ_BYP_DIS_GRP0;
	val |= IRQ_BYP_DIS_GRP0 | FIQ_BYP_DIS_GRP1 | IRQ_BYP_DIS_GRP1;

	gicc_write_pmr(cpu_base, 0x1 << 7); //GIC_PRI_MASK);
	gicc_write_ctlr(cpu_base, val);
}
*/
void irq_raise_softirq(unsigned int map, unsigned int irq)
{
    int satt;

    satt = 1 << 15;

    if(plat_ic_get_interrupt_type(irq) == INTR_TYPE_S_EL1)
    {
        satt = 0;
    }

//    gicd_write_sgir(mt_get_cfgvar(CONFIG_GICD_ADDR), (map << 16) | satt | irq);
    gicd_write_sgir(get_plat_config()->gicd_base, (map << 16) | satt | irq);

    dsb();
}

unsigned int get_ack_info()
{
//    return gicc_read_IAR(mt_get_cfgvar(CONFIG_GICC_ADDR));
    return gicc_read_IAR(get_plat_config()->gicc_base);

}

void ack_sgi(unsigned int iar)
{
//    gicc_write_EOIR(mt_get_cfgvar(CONFIG_GICC_ADDR), iar);
    gicc_write_EOIR(get_plat_config()->gicc_base, iar);

}

void mt_atf_trigger_irq()
{
//    gicd_set_ispendr(mt_get_cfgvar(CONFIG_GICD_ADDR), ATF_LOG_IRQ_ID);
    gicd_set_ispendr(get_plat_config()->gicd_base, ATF_LOG_IRQ_ID);

}

void mask_wdt_fiq()
{
//	gicd_set_icenabler(mt_get_cfgvar(CONFIG_GICD_ADDR), WDT_IRQ_BIT_ID);
    gicd_set_icenabler(get_plat_config()->gicd_base, WDT_IRQ_BIT_ID);

}


//===========================================================================
// Set WDT interrupt pending. It is for simulation of WDT when processing SMC
// "AEEDump".
//===========================================================================
void mt_atf_trigger_WDT_FIQ()
{
//    gicd_set_ispendr(mt_get_cfgvar(CONFIG_GICD_ADDR), WDT_IRQ_BIT_ID);
    gicd_set_ispendr(get_plat_config()->gicd_base, WDT_IRQ_BIT_ID);

}
/*******************************************************************************
 * This function does some minimal GICv3 configuration. The Firmware itself does
 * not fully support GICv3 at this time and relies on GICv2 emulation as
 * provided by GICv3. This function allows software (like Linux) in later stages
 * to use full GICv3 features.
 ******************************************************************************/
#if 0
void gicv3_cpuif_setup(void)
{
	unsigned int scr_val, val;
	uintptr_t base;

	/*
	 * When CPUs come out of reset they have their GICR_WAKER.ProcessorSleep
	 * bit set. In order to allow interrupts to get routed to the CPU we
	 * need to clear this bit if set and wait for GICR_WAKER.ChildrenAsleep
	 * to clear (GICv3 Architecture specification 5.4.23).
	 * GICR_WAKER is NOT banked per CPU, compute the correct base address
	 * per CPU.
	 */
	base = gicv3_get_rdist(BASE_GICR_BASE, read_mpidr());
	if (base == (uintptr_t)NULL) {
		/* No re-distributor base address. This interface cannot be
		 * configured.
		 */
		panic();
	}

	val = gicr_read_waker(base);

	val &= ~WAKER_PS;
	gicr_write_waker(base, val);
	dsb();

	/* We need to wait for ChildrenAsleep to clear. */
	val = gicr_read_waker(base);
	while (val & WAKER_CA) {
		val = gicr_read_waker(base);
	}

	/*
	 * We need to set SCR_EL3.NS in order to see GICv3 non-secure state.
	 * Restore SCR_EL3.NS again before exit.
	 */
	scr_val = read_scr();
	write_scr(scr_val | SCR_NS_BIT);
	isb();	/* ensure NS=1 takes effect before accessing ICC_SRE_EL2 */

	/*
	 * By default EL2 and NS-EL1 software should be able to enable GICv3
	 * System register access without any configuration at EL3. But it turns
	 * out that GICC PMR as set in GICv2 mode does not affect GICv3 mode. So
	 * we need to set it here again. In order to do that we need to enable
	 * register access. We leave it enabled as it should be fine and might
	 * prevent problems with later software trying to access GIC System
	 * Registers.
	 */
	val = read_icc_sre_el3();
	write_icc_sre_el3(val | ICC_SRE_EN | ICC_SRE_SRE);

	val = read_icc_sre_el2();
	write_icc_sre_el2(val | ICC_SRE_EN | ICC_SRE_SRE);

	write_icc_pmr_el1(GIC_PRI_MASK);
	isb();	/* commite ICC_* changes before setting NS=0 */

	/* Restore SCR_EL3 */
	write_scr(scr_val);
	isb();	/* ensure NS=0 takes effect immediately */
}

/*******************************************************************************
 * This function does some minimal GICv3 configuration when cores go
 * down.
 ******************************************************************************/
void gicv3_cpuif_deactivate(void)
{
	unsigned int val;
	uintptr_t base;

	/*
	 * When taking CPUs down we need to set GICR_WAKER.ProcessorSleep and
	 * wait for GICR_WAKER.ChildrenAsleep to get set.
	 * (GICv3 Architecture specification 5.4.23).
	 * GICR_WAKER is NOT banked per CPU, compute the correct base address
	 * per CPU.
	 */
	base = gicv3_get_rdist(BASE_GICR_BASE, read_mpidr());
	if (base == (uintptr_t)NULL) {
		/* No re-distributor base address. This interface cannot be
		 * configured.
		 */
		panic();
	}

	val = gicr_read_waker(base);
	val |= WAKER_PS;
	gicr_write_waker(base, val);
	dsb();

	/* We need to wait for ChildrenAsleep to set. */
	val = gicr_read_waker(base);
	while ((val & WAKER_CA) == 0) {
		val = gicr_read_waker(base);
	}
}
#endif

/*******************************************************************************
 * Enable secure interrupts and use FIQs to route them. Disable legacy bypass
 * and set the priority mask register to allow all interrupts to trickle in.
 ******************************************************************************/
void gic_cpuif_setup(unsigned int gicc_base)
{
	unsigned int val;

	//val = gicc_read_iidr(gicc_base);
    val = gicc_read_ctlr(gicc_base);

#if 0
	/*
	 * If GICv3 we need to do a bit of additional setup. We want to
	 * allow default GICv2 behaviour but allow the next stage to
	 * enable full gicv3 features.
	 */
	if (((val >> GICC_IIDR_ARCH_SHIFT) & GICC_IIDR_ARCH_MASK) >= 3) {
		gicv3_cpuif_setup();
	}
#endif
	val = ENABLE_GRP0 | ENABLE_GRP1 | FIQ_EN | FIQ_BYP_DIS_GRP0;
	val |= IRQ_BYP_DIS_GRP0 | FIQ_BYP_DIS_GRP1 | IRQ_BYP_DIS_GRP1;

    //val = ENABLE_GRP0 | ENABLE_GRP1;
	gicc_write_pmr(gicc_base, GIC_PRI_MASK);
	gicc_write_ctlr(gicc_base, val);
	//gicc_write_ctlr(gicc_base, 0x1B);
}

/*******************************************************************************
 * Place the cpu interface in a state where it can never make a cpu exit wfi as
 * as result of an asserted interrupt. This is critical for powering down a cpu
 ******************************************************************************/
void gic_cpuif_deactivate(unsigned int gicc_base)
{
	unsigned int val;

	/* Disable secure, non-secure interrupts and disable their bypass */
	val = gicc_read_ctlr(gicc_base);
	val &= ~(ENABLE_GRP0 | ENABLE_GRP1);
	val |= FIQ_BYP_DIS_GRP1 | FIQ_BYP_DIS_GRP0;
	val |= IRQ_BYP_DIS_GRP0 | IRQ_BYP_DIS_GRP1;
	gicc_write_ctlr(gicc_base, val);

	val = gicc_read_iidr(gicc_base);

#if 0
	/*
	 * If GICv3 we need to do a bit of additional setup. Make sure the
	 * RDIST is put to sleep.
	 */
	if (((val >> GICC_IIDR_ARCH_SHIFT) & GICC_IIDR_ARCH_MASK) >= 3) {
		gicv3_cpuif_deactivate();
	}
#endif
}

/*******************************************************************************
 * Per cpu gic distributor setup which will be done by all cpus after a cold
 * boot/hotplug. This marks out the secure interrupts & enables them.
 ******************************************************************************/
void gic_pcpu_distif_setup(unsigned int gicd_base)
{
	unsigned int irq_num;
	gicd_write_igroupr(gicd_base, 0, ~0);

        /* Route FIQ_SMP_CALL_SGI to secure */
        gicd_clr_igroupr(gicd_base, FIQ_SMP_CALL_SGI);
	gicd_set_ipriorityr(gicd_base, FIQ_SMP_CALL_SGI, GIC_HIGHEST_SEC_PRIORITY);
	gicd_set_isenabler(gicd_base, FIQ_SMP_CALL_SGI);

	for (irq_num = 0; irq_num < MIN_SPI_ID; irq_num++) {
		if (gicd_get_igroupr(gicd_base, irq_num) == GRP1) {
			/* only setup group 1 priortiy */
			gicd_set_ipriorityr(
				gicd_base,
				irq_num,
				GIC_HIGHEST_NS_PRIORITY);
		}
	}
}

void mt_irq_set_polarity(unsigned int irq, unsigned int polarity)
{
    //unsigned long flags;
    unsigned int offset, reg_index, value;

    /*
    if (irq < (NR_GIC_SGI + NR_GIC_PPI)) {
        printk(KERN_CRIT "Fail to set polarity of interrupt %d\n", irq);
        return ;
    }
    */

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

	if (sens == MT_EDGE_SENSITIVE)
	{
		config = mmio_read_32(base + GICD_ICFGR + (irq / 16) * 4);
		config |= (0x2 << (irq % 16) * 2);
		mmio_write_32(base + GICD_ICFGR + (irq / 16) * 4, config);
	}
	else
	{
		config = mmio_read_32(base + GICD_ICFGR + (irq / 16) * 4);
		config &= ~(0x2 << (irq % 16) * 2);
		mmio_write_32(base + GICD_ICFGR + (irq / 16) * 4, config);
	}
}


/*******************************************************************************
 * Global gic distributor setup which will be done by the primary cpu after a
 * cold boot. It marks out the secure SPIs, PPIs & SGIs and enables them. It
 * then enables the secure GIC distributor interface.
 ******************************************************************************/
void gic_distif_setup(unsigned int gicd_base)
{
	unsigned int ctr, num_ints, ctlr;

	/* Disable the distributor before going further */
	ctlr = gicd_read_ctlr(gicd_base);
	ctlr &= ~(ENABLE_GRP0 | ENABLE_GRP1);
	gicd_write_ctlr(gicd_base, ctlr);

	/*
	 * Mark out non-secure interrupts. Calculate number of
	 * IGROUPR registers to consider. Will be equal to the
	 * number of IT_LINES
	 */
	num_ints = gicd_read_typer(gicd_base) & IT_LINES_NO_MASK;
	num_ints++;
	for (ctr = 0; ctr < num_ints; ctr++)
    {
        //printf("ctr=%d\n", ctr);
		gicd_write_igroupr(gicd_base, ctr << IGROUPR_SHIFT, ~0);
    }

#if 1
    	/* Configure WDT secure interrupts now */
	gicd_clr_igroupr(gicd_base, WDT_IRQ_BIT_ID);
	gicd_set_ipriorityr(gicd_base, WDT_IRQ_BIT_ID, GIC_HIGHEST_SEC_PRIORITY);
	/* targets WDT_IRQ_BIT_ID to all cores */
	gicd_write_itargetsr(gicd_base, WDT_IRQ_BIT_ID, (PLATFORM_MAX_CPUS_MASK << ((WDT_IRQ_BIT_ID % 4) << 3)));
	mt_irq_set_sens(gicd_base, WDT_IRQ_BIT_ID, MT_EDGE_SENSITIVE);
#endif
    /* polarity is active low */
    mt_irq_set_polarity(WDT_IRQ_BIT_ID, MT_POLARITY_LOW);
#if 1
	gicd_set_isenabler(gicd_base, WDT_IRQ_BIT_ID);
#endif

#if CFG_MICROTRUST_TEE_SUPPORT
    teei_gic_setup();
#endif

#if 0
	/* Configure secure interrupts now */
	gicd_clr_igroupr(gicd_base, IRQ_TZ_WDOG);
	gicd_set_ipriorityr(gicd_base, IRQ_TZ_WDOG, GIC_HIGHEST_SEC_PRIORITY);
	gicd_set_itargetsr(gicd_base, IRQ_TZ_WDOG,
			   platform_get_core_pos(read_mpidr()));
	gicd_set_isenabler(gicd_base, IRQ_TZ_WDOG);
#endif

	gic_pcpu_distif_setup(gicd_base);

	gicd_write_ctlr(gicd_base, ctlr | ENABLE_GRP0  | ENABLE_GRP1);
}

#if 0
static void gic_print_all_setting(unsigned int gicd_base, unsigned int gicc_base)
{
    printf("GICD_CTLR=0x%08x\n", gicd_read_ctlr(gicd_base));
    //printf("GICD_TYPER=0x%08x\n", gicd_read_typer(gicd_base));
    //printf("GICD_SGIR=0x%08x\n", gicd_read_sgir(gicd_base));

    printf("GICC_CTRL=0x%08x\n", gicc_read_ctlr(gicc_base));
    printf("GICC_PMR=0x%08x\n", gicc_read_pmr(gicc_base));
}
#endif

void gic_setup(void)
{
	unsigned int gicd_base, gicc_base;

//	gicd_base = mt_get_cfgvar(CONFIG_GICD_ADDR);
	gicd_base = get_plat_config()->gicd_base;
//	gicc_base = mt_get_cfgvar(CONFIG_GICC_ADDR);
    gicc_base = get_plat_config()->gicc_base;

	gic_cpuif_setup(gicc_base);
	gic_distif_setup(gicd_base);
#if 0
    {
    unsigned int i, no_sec;
    for(i = 0; i < 0xFF; i++)
    {
        no_sec = gicd_get_igroupr(gicd_base, i);
        printf("irq = %d, group=0x%x(%s)\n", i, no_sec, no_sec ? "non-sec":"sec");

    }
    gic_print_all_setting(gicd_base, gicc_base);
    }

#endif
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
	uint32_t gicc_base;     // = mt_get_cfgvar(CONFIG_GICC_ADDR);

    gicc_base = get_plat_config()->gicc_base;

	assert(type == INTR_TYPE_S_EL1 ||
	       type == INTR_TYPE_EL3 ||
	       type == INTR_TYPE_NS);

	assert(security_state == NON_SECURE || security_state == SECURE);

	/*
	 * We ignore the security state parameter under the assumption that
	 * both normal and secure worlds are using ARM GICv2. This parameter
	 * will be used when the secure world starts using GICv3.
	 */
#if MT_GIC_ARCH == 2
	return gicv2_interrupt_type_to_line(gicc_base, type);
#else
#error "Invalid GIC architecture version specified for MT port"
#endif
}

#if MT_GIC_ARCH == 2
/*******************************************************************************
 * This function returns the type of the highest priority pending interrupt at
 * the GIC cpu interface. INTR_TYPE_INVAL is returned when there is no
 * interrupt pending.
 ******************************************************************************/
uint32_t plat_ic_get_pending_interrupt_type()
{
	uint32_t id, gicc_base;

    gicc_base = get_plat_config()->gicc_base;
	id = gicc_read_hppir(gicc_base) & 0x3FF;

	/* Assume that all secure interrupts are S-EL1 interrupts */
	if (id < 1022)
		return INTR_TYPE_S_EL1;

	if (id == 1022)
		return INTR_TYPE_INVAL;
	/*
	 * Only GIC_SPURIOUS_INTERRUPT 1023 reach here.
	 * Fix Coverity issue.
	 */
	return INTR_TYPE_INVAL;
}

/*******************************************************************************
 * This function returns the id of the highest priority pending interrupt at
 * the GIC cpu interface. INTR_ID_UNAVAILABLE is returned when there is no
 * interrupt pending.
 ******************************************************************************/
uint32_t plat_ic_get_pending_interrupt_id()
{
	uint32_t id, gicc_base;

//	gicc_base = mt_get_cfgvar(CONFIG_GICC_ADDR);
    gicc_base = get_plat_config()->gicc_base;
	id = gicc_read_hppir(gicc_base) & 0x3FF;

	if (id < 1022)
		return id;

	if (id == 1023)
		return INTR_ID_UNAVAILABLE;

	/*
	 * Find out which non-secure interrupt it is under the assumption that
	 * the GICC_CTLR.AckCtl bit is 0.
	 */
	return gicc_read_ahppir(gicc_base);
}

/*******************************************************************************
 * This functions reads the GIC cpu interface Interrupt Acknowledge register
 * to start handling the pending interrupt. It returns the contents of the IAR.
 ******************************************************************************/
uint32_t plat_ic_acknowledge_interrupt()
{
//	return gicc_read_IAR(mt_get_cfgvar(CONFIG_GICC_ADDR));
    return gicc_read_IAR(get_plat_config()->gicc_base);
}

/*******************************************************************************
 * This functions writes the GIC cpu interface End Of Interrupt register with
 * the passed value to finish handling the active interrupt
 ******************************************************************************/
void plat_ic_end_of_interrupt(uint32_t id)
{
//	gicc_write_EOIR(mt_get_cfgvar(CONFIG_GICC_ADDR), id);
    gicc_write_EOIR(get_plat_config()->gicc_base, id);

	return;
}

/*******************************************************************************
 * This function returns the type of the interrupt id depending upon the group
 * this interrupt has been configured under by the interrupt controller i.e.
 * group0 or group1.
 ******************************************************************************/
uint32_t plat_ic_get_interrupt_type(uint32_t id)
{
	uint32_t group;

//	group = gicd_get_igroupr(mt_get_cfgvar(CONFIG_GICD_ADDR), id);
	group = gicd_get_igroupr(get_plat_config()->gicd_base, id);


	/* Assume that all secure interrupts are S-EL1 interrupts */
	if (group == GRP0)
		return INTR_TYPE_S_EL1;
	else
		return INTR_TYPE_NS;
}

uint64_t mt_irq_dump_status(uint32_t irq)
{
	uint32_t dist_base;
	unsigned int bit;
	uint32_t reg_shift = (irq % 4) * 8;
	uint32_t result;
	uint64_t rc = 0;

	printf("[ATF GIC dump] irq = %d\n", irq);

	if (irq >= NR_MT_IRQ_LINE)
		return 0;

//	dist_base = mt_get_cfgvar(CONFIG_GICD_ADDR);
    dist_base = get_plat_config()->gicd_base;


	/* get mask */
	bit = 1 << (irq % 32);
	result = ((mmio_read_32(dist_base + GICD_ISENABLER + irq / 32 * 4) & bit)?1:0);
	printf("[ATF GIC dump] enable = %x\n", result);
	rc |= result;

	/*get group*/
	bit = 1 << (irq % 32);
	//0x1:irq,0x0:fiq
	result = ((mmio_read_32(dist_base + GICD_IGROUPR + irq / 32 * 4) & bit)?1:0);
	printf("[ATF GIC dump] group = %x (0x1:irq,0x0:fiq)\n", result);
	rc |=  result << 1;

	/* get priority */
	bit = 0xff << ((irq % 4)*8);
	result = ((mmio_read_32(dist_base + GICD_IPRIORITYR + irq / 4 * 4) & bit) >> ((irq % 4)*8));
	printf("[ATF GIC dump] priority = %x\n", result);
	rc |= result << 2;

	/* get sensitivity */
	bit = 0x3 << ((irq % 16)*2);
	//edge:0x2, level:0x1
	result = ((mmio_read_32(dist_base + GICD_ICFGR + irq / 16 * 4) & bit) >> ((irq % 16)*2));
	printf("[ATF GIC dump] sensitivity = %x (edge:0x1, level:0x0)\n", result>>1);
	rc |= (result>>1) << 10;

	/* get pending status */
	bit = 1 << (irq % 32);
	result = ((mmio_read_32(dist_base + GICD_ISPENDR + irq / 32 * 4) & bit)?1:0);
	printf("[ATF GIC dump] pending status = %x\n", result);
	rc |= result << 11;

	/* get active status */
	bit = 1 << (irq % 32);
	result = ((mmio_read_32(dist_base + GICD_ISACTIVER + irq / 32 * 4) & bit)?1:0);
	printf("[ATF GIC dump] active status = %x\n", result);
	rc |= result << 12;

	/* get polarity */
	if (irq >= 32) {
		bit = 1 << (irq % 32);
		//0x0: high, 0x1:low
		result = ((mmio_read_32(INT_POL_CTL0 + (irq-32) / 32 * 4) & bit)?1:0);
		printf("[ATF GIC dump] polarity = %x (0x0: high, 0x1:low)\n", result);
		rc |= result << 13;
	}

	/* get target cpu mask */
	result = (gicd_read_itargetsr(dist_base, irq) & (0xff << reg_shift)) >> reg_shift;
	printf("[ATF GIC dump] tartget cpu mask = 0x%x\n", result);
	rc |= result << 14;

	return rc;
}

void dump_wfi_spill(void)
{
	unsigned int isr_el1;
	unsigned int gicc_ctlr;
	unsigned int emi_enable_rep;
	unsigned int systracker_con;
	unsigned int cpuid = platform_get_core_pos(read_mpidr());

	__asm__ volatile("mrs %0, isr_el1" : "=r" (isr_el1));
	printf("[debug_wfi_spill] <%d> ISR_EL1 = 0x%x\n", cpuid, isr_el1);

	gicc_ctlr = gicc_read_ctlr(get_plat_config()->gicc_base);
	printf("[debug_wfi_spill] <%d> gicc_ctlr = 0x%x\n", cpuid, gicc_ctlr);

#ifdef MT_EMI_BASE
	emi_enable_rep = (mmio_read_32(MT_EMI_BASE + 0x1c0) & (1<<28))| (mmio_read_32(MT_EMI_BASE + 0x1c8) & (1<<28))|
			(mmio_read_32(MT_EMI_BASE + 0x1d0) & (1<<28)) | (mmio_read_32(MT_EMI_BASE + 0x200) & (1<<28))|
			(mmio_read_32(MT_EMI_BASE + 0x2c0) & (1<<28)) | (mmio_read_32(MT_EMI_BASE + 0x2c8) & (1<<28))|
			(mmio_read_32(MT_EMI_BASE + 0x2d0) & (1<<28)) | (mmio_read_32(MT_EMI_BASE + 0x300) & (1<<28));
	printf("[debug_wfi_spill] <%d> emi_enable_rep = 0x%x\n", cpuid, emi_enable_rep);
#endif

#ifdef MT_TRACKER_BASE
	systracker_con = mmio_read_32(MT_TRACKER_BASE);
	printf("[debug_wfi_spill] <%d> systracker_con = 0x%x\n", cpuid, systracker_con);
#endif
}

#else
#error "Invalid GIC architecture version specified for MT port"
#endif
