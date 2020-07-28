/*
 * Copyright (c) 2015-2018 MICROTRUST Incorporated
 * All rights reserved
 *
 * This file and software is confidential and proprietary to MICROTRUST Inc.
 * Unauthorized copying of this file and software is strictly prohibited.
 * You MUST NOT disclose this file and software unless you get a license
 * agreement from MICROTRUST Incorporated.
 */
#include <assert.h>
#include <debug.h>
#include <string.h>

#include <platform.h>
#include <arch_helpers.h>
#include <platform_def.h>

#include <bl_common.h>
#include <gic_v2.h>
#include <gic_v3.h>
#include <stdio.h>
#include <mt_gic_v3.h>
#include <runtime_svc.h>
#include <fiq_smp_call.h>
#include <mtk_aee_debug.h>

#include "plat_teei.h"
#include "plat_private.h"
#include "devapc.h"
#include <rng.h>

#include "teei_private.h"
#include "teei_atf.h"
#include "platform_def_teei.h"
#include "plat_teei_inner.h"

#define SZ_64K                          (0x00010000)
#define DRM_SEC_EXC_CNT 8

static struct tee_arg_t *teeiBootCfg;
static struct tee_keys_t *teeiKeys;
unsigned int sec_exc[8];
unsigned int nsec_exc[8];
unsigned int SEC_EXC_CNT;
unsigned int NSEC_EXC_CNT;
unsigned int NSEC_UART;
unsigned int SEC_TIMER;
unsigned int SEC_APP_INTR;
unsigned int SEC_DRV_INTR;
unsigned int SEC_RDRV_INTR;
unsigned int SEC_TDUMP_INTR;
unsigned int drm_sec_exc[DRM_SEC_EXC_CNT] = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned int drm_sec_enable[DRM_SEC_EXC_CNT] = {0, 0, 0, 0, 0, 0, 0, 0};

unsigned int gicd_get_isenabler(unsigned int base, unsigned int id)
{
	unsigned int bit_num = id & ((1 << ISENABLER_SHIFT) - 1);
	unsigned int reg_val = gicd_read_isenabler(base, id);

	return (reg_val >> bit_num) & 0x1;
}

#ifndef LEGACY_GIC_SUPPORT
static void config_ns_group1(int enable)
{
	unsigned int scr_val = 0;

	scr_val = read_scr();
	write_scr(scr_val | SCR_NS_BIT);
	isb();

	if (enable)
		gicc_write_grpen1_el1(0x1);
	else
		gicc_write_grpen1_el1(0x0);

	isb();

	write_scr(scr_val);
	isb();
}
#endif

void disable_group(unsigned int grp)
{
#ifndef LEGACY_GIC_SUPPORT
	/* only support for group 1 now */
	if (grp != 1)
		return;

	config_ns_group1(0);
#else
	unsigned int  pre;

	pre = gicd_v3_read_ctlr(GICD_BASE);
	pre = pre & (~(1<<grp));
	gicd_v3_write_ctlr(GICD_BASE, pre);
	gicd_v3_do_wait_for_rwp(GICD_BASE);
#endif
}

void enable_group(unsigned int grp)
{
#ifndef LEGACY_GIC_SUPPORT
	/* only support for group 1 now */
	if (grp != 1)
		return;

	config_ns_group1(1);
#else
	unsigned int  pre;

	pre = gicd_v3_read_ctlr(GICD_BASE);
	pre = pre | (1<<grp);
	gicd_v3_write_ctlr(GICD_BASE, pre);
	gicd_v3_do_wait_for_rwp(GICD_BASE);
#endif
}

void  disable_ns_exc_intr(unsigned int gicd_base)
{
	unsigned int index;

	for (index = 0;  index < NSEC_EXC_CNT; index++) {
		gicd_set_icenabler(gicd_base, nsec_exc[index]);
		gicd_v3_do_wait_for_rwp(gicd_base);
	}
}

void  enable_ns_exc_intr(unsigned int gicd_base)
{
	unsigned int index;

	for (index = 0;  index < NSEC_EXC_CNT; index++) {
		gicd_set_isenabler(gicd_base,  nsec_exc[index]);
		gicd_v3_do_wait_for_rwp(gicd_base);
	}
}

void  disable_s_exc_intr(unsigned int gicd_base)
{
	unsigned int index;

	for (index = 0;  index < SEC_EXC_CNT; index++) {
		gicd_set_icenabler(gicd_base, sec_exc[index]);
		gicd_v3_do_wait_for_rwp(gicd_base);
	}
	for (index = 0;  index < DRM_SEC_EXC_CNT; index++) {
		if (drm_sec_exc[index] != 0) {
			if (drm_sec_exc[index] < 32) {
				unsigned int rdist_sgi_base = 0;

				if (gic_populate_rdist(&rdist_sgi_base) == -1)
					return;
				rdist_sgi_base += SZ_64K;
				drm_sec_enable[index] = gicd_get_isenabler(rdist_sgi_base,  drm_sec_exc[index]);
				gicd_set_icenabler(rdist_sgi_base, drm_sec_exc[index]);
				gicd_v3_do_wait_for_rwp(rdist_sgi_base);
			} else {
				drm_sec_enable[index] = gicd_get_isenabler(gicd_base,  drm_sec_exc[index]);
				gicd_set_icenabler(gicd_base, drm_sec_exc[index]);
				gicd_v3_do_wait_for_rwp(gicd_base);
			}
		}
	}
}

void  enable_s_exc_intr(unsigned int gicd_base)
{
	unsigned int index;

	for (index = 0;  index < SEC_EXC_CNT; index++) {
		gicd_set_isenabler(gicd_base,  sec_exc[index]);
		gicd_v3_set_irouter(gicd_base, sec_exc[index], gic_mpidr_to_affinity(read_mpidr()));
		gicd_v3_do_wait_for_rwp(gicd_base);

	}
	for (index = 0;  index < DRM_SEC_EXC_CNT; index++) {
		if (drm_sec_exc[index] != 0 && drm_sec_enable[index]) {
			if (drm_sec_exc[index] < 32) {
				unsigned int rdist_sgi_base = 0;

				if (gic_populate_rdist(&rdist_sgi_base) == -1)
					return;
				rdist_sgi_base += SZ_64K;
				gicd_set_isenabler(rdist_sgi_base, drm_sec_exc[index]);
				gicd_v3_do_wait_for_rwp(rdist_sgi_base);
			} else {
				gicd_set_isenabler(gicd_base,  drm_sec_exc[index]);
				gicd_v3_set_irouter(gicd_base, drm_sec_exc[index], gic_mpidr_to_affinity(read_mpidr()));
				gicd_v3_do_wait_for_rwp(gicd_base);
			}
		}
	}
}

/* virtual timer */
DEFINE_SYSREG_RW_FUNCS(cntkctl_el1)
#define enable_cntkctl_el0vten(x)  (x |= (EL0VTEN_BIT))
#define disable_cntkctl_el0vten(x)  (x &= (~EL0VTEN_BIT))
int drm_config_virtual_timer_el1_rw(int rw)
{
	unsigned int ctl = 0;

	if (rw) {
		ctl = read_cntkctl_el1();
		enable_cntkctl_el0vten(ctl);
		write_cntkctl_el1(ctl);
	} else {
		ctl = read_cntkctl_el1();
		disable_cntkctl_el0vten(ctl);
		write_cntkctl_el1(ctl);
	}
	return 0;
}
/* gicr */
unsigned int enable_ppi_fiq(unsigned int fiq)
{
	unsigned int base = 0;

	/* get the base of redistributor first */
	if (gic_populate_rdist(&base) == -1)
		return 0;

	/* the SGI/PPI base is at 64K offset of current rdist base */
	base += SZ_64K;
#if !defined(SPD_trusty)
	/* set all SGI/PPI as NS-group1 by default */
	gicd_write_igroupr(base, 0, ~0);
#endif
	/* set fiq smp call as fiq */
	gicd_clr_igroupr(base, fiq);
	gicd_set_ipriorityr(base, fiq, GIC_HIGHEST_SEC_PRIORITY);
	gicd_set_isenabler(base, fiq);
	gicd_v3_do_wait_for_rwp(base);
	return base;
}
unsigned int disable_ppi_fiq(unsigned int fiq)
{
	unsigned int base = 0;

	/* get the base of redistributor first */
	if (gic_populate_rdist(&base) == -1)
		return 0;

	/* the SGI/PPI base is at 64K offset of current rdist base */
	base += SZ_64K;
	/* clr fiq smp call as fiq */
	gicd_set_igroupr(base, fiq);
	gicd_set_ipriorityr(base, fiq, GIC_HIGHEST_NS_PRIORITY);
	gicd_set_icenabler(base, fiq);
	gicd_v3_do_wait_for_rwp(base);
	return base;
}
int set_drm_int_secure(unsigned int id)
{
	int state = 1;
	unsigned int index;

	for (index = 0; index < DRM_SEC_EXC_CNT; index++) {
		if (drm_sec_exc[index] == 0) {
			drm_sec_exc[index] = id;
			state = 0;
			break;
		}
	}
	if (id < 32)
		goto set_gicr;

	gicd_set_icenabler(GICD_BASE,  id);
	gicd_clr_igroupr(GICD_BASE, id);
	gicd_v3_set_irouter(GICD_BASE, id, gic_mpidr_to_affinity(read_mpidr()));
	gicd_set_isenabler(GICD_BASE,  id);
	gicd_v3_do_wait_for_rwp(GICD_BASE);
	return state;
set_gicr:
	/* gicr */
	enable_ppi_fiq(id);
	return state;
}
int set_drm_int_nonsecure(unsigned int id)
{
	int state = 1;
	unsigned int index;

	for (index = 0; index < DRM_SEC_EXC_CNT; index++) {
		if (drm_sec_exc[index] == id)
			drm_sec_exc[index] = 0;

	}
	if (id < 32)
		goto set_gicr;

	gicd_set_icenabler(GICD_BASE,  id);
	gicd_set_igroupr(GICD_BASE, id);
	/***********************************************************
	 * gicd_v3_set_irouter(gicd_base, id,gic_mpidr_to_affinity(read_mpidr()));
	//not sure if it is OK
	*************************************************************/
	gicd_v3_set_irouter(GICD_BASE, id, GICD_V3_IROUTER_SPI_MODE_ANY);
	gicd_set_isenabler(GICD_BASE,  id);
	gicd_v3_do_wait_for_rwp(GICD_BASE);
	return state;
set_gicr:
	/* gicr */
	disable_ppi_fiq(id);
	return state;
}

void prepare_gic_for_nsec_boot(void)
{
	disable_s_exc_intr(GICD_BASE);
}

unsigned int get_irq_target(unsigned int irq)
{
	return gicd_read_itargetsr(GICD_BASE, irq);
}

void prepare_gic_for_sec_boot(void)
{
	unsigned int index;

	for (index = 0;  index < SEC_EXC_CNT; index++)
		gicd_set_icenabler(GICD_BASE,  sec_exc[index]);
}


void migrate_gic_context(uint32_t secure_state)
{
	if (secure_state == SECURE) {

		disable_ns_exc_intr(GICD_BASE);
		if (TEEI_STATE < TEEI_BUF_READY)
			disable_group(1);

		enable_s_exc_intr(GICD_BASE);
	} else {
		enable_ns_exc_intr(GICD_BASE);
		disable_s_exc_intr(GICD_BASE);
		if (TEEI_STATE < TEEI_BUF_READY)
			enable_group(1);
	}
}

void trigger_soft_intr(unsigned int id)
{
	gicd_v3_set_irouter(GICD_BASE, id, gic_mpidr_to_affinity(read_mpidr()));
	gicd_set_ispendr(GICD_BASE, id);
}
/*****************************************************************
 * void dump_gic_all_setting()
 * {
 * unsigned int gicd_base, gicc_base;
 * gicd_base = get_plat_config()->gicd_base;
 * gicc_base = get_plat_config()->gicc_base;
 * gic_print_all_setting(gicd_base, gicc_base);
 * }
******************************************************************/

void sec_exc_add(unsigned int intr_num)
{
	sec_exc[SEC_EXC_CNT] =  intr_num;
	SEC_EXC_CNT++;
}

void nsec_exc_add(unsigned int intr_num)
{
	nsec_exc[NSEC_EXC_CNT] =  intr_num;
	NSEC_EXC_CNT++;
}

void teei_init_interrupt(void)
{
	int i;

	SEC_EXC_CNT = 0;
	NSEC_EXC_CNT = 0;

	teeiBootCfg = (struct tee_arg_t *)(uintptr_t)TEEI_BOOT_PARAMS;
	teeiKeys =  (struct tee_keys_t *)(uintptr_t)TEEI_SECURE_PARAMS;

	for (i = 0; i < 5; i++) {
		if (teeiBootCfg->tee_dev[i].dev_type == MT_UART16550) {
			uart_apc_num = teeiBootCfg->tee_dev[i].apc_num;
			NSEC_UART =   teeiBootCfg->tee_dev[i].intr_num;
		}
		if (teeiBootCfg->tee_dev[i].dev_type == MT_SEC_GPT)
			SEC_TIMER = teeiBootCfg->tee_dev[i].intr_num;


	}
	SEC_APP_INTR = teeiBootCfg->ssiq_number[0];
	SEC_DRV_INTR = teeiBootCfg->ssiq_number[1];
	SEC_RDRV_INTR = teeiBootCfg->ssiq_number[2];
	SEC_TDUMP_INTR = teeiBootCfg->ssiq_number[3];


	/*init secure exlusive array*/
	sec_exc_add(SEC_TIMER);
	sec_exc_add(SEC_APP_INTR);
	sec_exc_add(SEC_DRV_INTR);
	sec_exc_add(SEC_RDRV_INTR);
	sec_exc_add(SEC_TDUMP_INTR);
#ifdef TUI_SUPPORT
	sec_exc_add(206);
	sec_exc_add(SEC_TUI_CANCEL);

#endif

}

unsigned int teei_get_fp_id(void)
{
	if (teeiBootCfg->tee_dev[4].dev_type == 5)
		return teeiBootCfg->tee_dev[4].base_addr;

	return 0;
}
void teei_gic_setup(void)
{
	teei_init_interrupt();
	/************************************************************************
	 * set interrupt to group 0
	 * set GIC_HIGHEST_SEC_PRIORITY
	 * set itarget to current cpu
	 * use 1-N model, means as long as one of N core can handle, this will be handled
	 * set gic edge sensitive via GICD_ICFG
	 * set low polarity
	 * disable interrupt
	 **************************************************************************/
	for (unsigned int index = 0;  index < SEC_EXC_CNT; index++) {
		gicd_clr_igroupr(GICD_BASE, sec_exc[index]);
		gicd_set_ipriorityr(GICD_BASE,  sec_exc[index], GIC_HIGHEST_SEC_PRIORITY);
		gicd_v3_set_irouter(GICD_BASE, sec_exc[index], gic_mpidr_to_affinity(read_mpidr()));
		if (sec_exc[index] == SEC_TIMER) { /* GPT: LEVEL / LOW */
			mt_irq_set_sens(GICD_BASE,  sec_exc[index], MT_LEVEL_SENSITIVE);
			mt_irq_set_polarity(sec_exc[index], MT_POLARITY_LOW);
		} else { /* SW-INT: EDGE / Don't care */
			mt_irq_set_sens(GICD_BASE,  sec_exc[index], MT_EDGE_SENSITIVE);
			mt_irq_set_polarity(sec_exc[index], MT_POLARITY_HIGH);
		}
		gicd_set_icenabler(GICD_BASE,  sec_exc[index]);
		gicd_set_icpendr(GICD_BASE, sec_exc[index]);
		gicd_v3_do_wait_for_rwp(GICD_BASE);
	}
}

void teei_triggerSgiDump(void)
{
	uint64_t mpidr;
	uint32_t linear_id;

	/* send to all cpus except the current one */
	mpidr = read_mpidr();
	linear_id = platform_get_core_pos(mpidr);
	fiq_smp_call_function(0x3FF & ~(1 << linear_id), (inter_cpu_call_func_t)aee_wdt_dump, NULL, 0);
	aee_wdt_dump(NULL);
}

void teei_ack_gic(void)
{
	uint32_t iar;

	iar = gicc_read_IAR(GICC_BASE);
	gicc_write_EOIR(GICC_BASE, iar);
}

uint32_t teei_rng(void)
{
	uint32_t value = 0;

	plat_get_rnd(&value);
	return value;
}

int s_wdt_lock;

uint64_t teei_fiq_handler(uint32_t id, uint32_t flags, void *handle, void *cookie)
{
	/***********************************************************
	* uint64_t mpidr = read_mpidr();
	* uint32_t linear_id = platform_get_core_pos(mpidr);
	* struct teei_context *teei_ctx = &secure_context[linear_id];
	**********************************************************/
	int caller_security_state = flags & 1;

	if (caller_security_state == SECURE)
		SMC_RET1(handle, SMC_UNK);

	switch (id) {
	case FIQ_SMP_CALL_SGI: {
			plat_ic_acknowledge_interrupt();
			fiq_icc_isr();
			plat_ic_end_of_interrupt(FIQ_SMP_CALL_SGI);
			SMC_RET0(handle);
			break;
		}
	case WDT_IRQ_BIT_ID: {
			/* if disable group 0 other core will not receive sgi */
			int lockval = 0;
			int tmp = 0;

			plat_ic_acknowledge_interrupt();
			__asm__ volatile(
				"1: ldxr  %w0, [%2]\n"
				" add %w0, %w0, %w3\n"
				" stxr  %w1, %w0, [%2]\n"
				" cbnz  %w1, 1b"
					: "=&r" (lockval),  "=&r" (tmp)
					: "r" (&(s_wdt_lock)), "Ir" (1)
					: "cc");
			if (s_wdt_lock == 1)
				teei_triggerSgiDump();

			plat_ic_end_of_interrupt(WDT_IRQ_BIT_ID);
			SMC_RET0(handle);
			break;
		}
	default:
		SMC_RET0(handle);
	}

	return 0;
}
