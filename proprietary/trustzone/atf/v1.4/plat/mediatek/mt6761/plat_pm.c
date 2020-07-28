/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
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

/* common headers */
#include <stdbool.h>
#include <assert.h>
#include <debug.h>
#include <mmio.h>
#include <psci.h>
#include <errno.h>
#include <bl31.h>

/* mediatek platform specific headers */
#include <fiq_cache.h>
#include <platform_def.h>
#include <scu.h>
#include <mtk_plat_common.h>
#include <mt_spm.h>
#include <mt_spm_dpidle.h>
#include <mt_spm_sodi.h>
#include <mtspmc.h>
#include <mtk_mcdi.h>
#include <power_tracer.h>
#include <plat_pm.h>
#include <plat_private.h>
#include <plat_dcm.h>
#include <wfifo.h>
#include <rtc.h>
#include <log.h>
#include <mtk_plat_gpio.h>
#include <mtk_cm_mgr.h>
#include <mtk_rgu.h>

#if USE_COHERENT_MEM
#define __coherent  __section("tzfw_coherent_mem")
#else
#define __coherent
#endif

#define STA_POWER_DOWN		0
#define STA_POWER_ON		1

static int is_mtk_suspend[PLATFORM_CORE_COUNT] __coherent;
uintptr_t mtk_suspend_footprint_addr __coherent;
uintptr_t mtk_suspend_timestamp_addr __coherent;

void mtk_suspend_footprint_log(int idx)
{
	unsigned long mpidr = read_mpidr_el1();
	int cpu = (mpidr & 0xff) | ((mpidr & 0xff00) >> 6);

	if (idx == MTK_SUSPEND_FOOTPRINT_ENTER_ATF)
		is_mtk_suspend[cpu] = 1;

	if (is_mtk_suspend[cpu] != 1 || mtk_suspend_footprint_addr == 0)
		return;

	mmio_write_32(footprint_addr(cpu),
		      mmio_read_32(footprint_addr(cpu)) | (1 << idx));

	isb();
	dsb();

	if (idx == MTK_SUSPEND_FOOTPRINT_LEAVE_ATF)
		is_mtk_suspend[cpu] = 0;
}

void mtk_suspend_timestamp_log(int idx)
{
	unsigned long mpidr = read_mpidr_el1();
	int cpu = (mpidr & 0xff) | ((mpidr & 0xff00) >> 6);

	if (is_mtk_suspend[cpu] != 1 || mtk_suspend_timestamp_addr == 0)
		return;

	mmio_write_64(timestamp_addr(cpu, idx), read_cntpct_el0());

	isb();
	dsb();
}

#ifndef ENABLE_CPU_SELF_POWERDOWN
int mt_cluster_ops(int cputop_mpx, int mode, int state)
{
	if (cputop_mpx >= PLATFORM_CLUSTER_COUNT) {
		ERROR("[ATF]: %s() Cluster%d not exists\n", __func__, (int)cputop_mpx);
		assert(0);
	}

	INFO("%s: cluster=%d, mode=%d\n", __func__, cputop_mpx, mode);

	if (state == STA_POWER_DOWN)
		spm_poweroff_cluster(cputop_mpx);
	else
		spm_poweron_cluster(cputop_mpx);

	return 0;
}

int mt_core_ops(int cpux, int mode, int state)
{
	int cluster, cpu;

	if (cpux >= PLATFORM_CORE_COUNT) {
		ERROR("[ATF]: %s() CPU%d not exists\n", __func__, (int)cpux);
		assert(0);
	}

	INFO("%s: cpu=%d, mode=%d\n", __func__, cpux, mode);

	cluster = cpux / 4;
	cpu = cpux % 4;

	if (state == STA_POWER_DOWN)
		spm_poweroff_cpu(cluster, cpu);
	else
		spm_poweron_cpu(cluster, cpu);

	return 0;
}
#else  /* ENABLE_CPU_SELF_POWERDOWN */
int mt_cluster_ops(int cputop_mpx, int mode, int state)
{
	panic();

	return -1;
}

int mt_core_ops(int cpux, int mode, int state)
{
	panic();

	return -1;
}
#endif /* !ENABLE_CPU_SELF_POWERDOWN */

#ifdef ATF_BYPASS_DRAM
static void plat_spm_suspend(int state_id)
{
}
#else
static void plat_spm_suspend(int state_id)
{
	switch (state_id) {
	case MTK_MCDI_MODE:
		/* This block is for MCDI */
		break;
	case MTK_SODI_MODE:
		/* This block is for SODI */
		spm_sodi();
		break;
	case MTK_SODI3_MODE:
		/* This block is for SODI3 */
		spm_sodi();
		break;
	case MTK_DPIDLE_MODE:
		/* This block is for Deep Idle */
		spm_dpidle();
		break;
	case MTK_SUSPEND_MODE:
		/* This block is for Suspend */
		spm_suspend();
		break;
	default:
		break;
	}
}
#endif

#ifdef ATF_BYPASS_DRAM
static void plat_spm_suspend_finish(int state_id)
{
}
#else
static void plat_spm_suspend_finish(int state_id)
{
	switch (state_id) {
	case MTK_MCDI_MODE:
		/* This block is for MCDI */
		break;
	case MTK_SODI_MODE:
		/* This block is for SODI */
		spm_sodi_finish();
		break;
	case MTK_SODI3_MODE:
		/* This block is for SODI3 */
		spm_sodi_finish();
		break;
	case MTK_DPIDLE_MODE:
		/* This block is for Deep Idle */
		spm_dpidle_finish();
		break;
	case MTK_SUSPEND_MODE:
		/* This block is for Suspend */
		spm_suspend_finish();
		break;
	default:
		break;
	}
}
#endif

static void plat_mcdi_enter(unsigned long cpu, int state_id)
{
	if (state_id == MTK_MCDI_MODE)
		sspm_standbywfi_irq_enable(cpu);
}

static bool is_any_core_dpidle_sodi_mode(int state_id)
{
	return (state_id == MTK_SODI_MODE || state_id == MTK_SODI3_MODE || state_id == MTK_DPIDLE_MODE);
}

/*******************************************************************************
 * Function which implements the common MTK_platform specific operations to power down a
 * cpu in response to a CPU_OFF or CPU_SUSPEND request.
 ******************************************************************************/
static void plat_cpu_pwrdwn_common(int cluster, int cpu, int state_id)
{
	/* disable auto shutoff */
	spm_disable_cpu_autoff(cluster, cpu);

	/* Prevent interrupts from spuriously waking up this cpu */
	gic_rdist_save();
	gic_cpuif_deactivate(0);
}

/*******************************************************************************
 * Function which implements the common MTK_platform specific operations to power down a
 * cluster in response to a CPU_OFF or CPU_SUSPEND request.
 ******************************************************************************/
static void plat_cluster_pwrdwn_common(uint64_t mpidr, int state_id, int cluster)
{
	/* Disable coherency if this cluster is to be turned off */
	plat_cci_disable();
	disable_scu(mpidr);

	mtk_suspend_timestamp_log(MTK_SUSPEND_TIMESTAMP_GIC_P1);

	/* disable auto shutoff */
	spm_disable_cluster_autoff(cluster);

	if (state_id != MTK_MCDI_MODE)
		gic_dist_save();
	mtk_suspend_timestamp_log(MTK_SUSPEND_TIMESTAMP_GIC_P2);
}

/*******************************************************************************
 * Function which implements the common MTK_platform specific operations to power down
 * mcusys in response to a CPU_OFF or CPU_SUSPEND request.
 ******************************************************************************/
static void plat_mcusys_pwrdwn_common(int state_id)
{
	if (is_any_core_dpidle_sodi_mode(state_id))
		gic_sgi_save_all();
}

static void plat_power_domain_on_finish_common(unsigned int afflvl, int state_id,
					       uint64_t mpidr, int cluster)
{
	/* Perform the common mcusys specific operations */
	if (is_any_core_dpidle_sodi_mode(state_id))
		gic_sgi_restore_all();

	/* Perform the common cluster specific operations */
	if (afflvl != MPIDR_AFFLVL0) {
		mtk_suspend_timestamp_log(MTK_SUSPEND_TIMESTAMP_RESERVE_P1);
		mtk_suspend_timestamp_log(MTK_SUSPEND_TIMESTAMP_RESERVE_P2);
		mtk_suspend_timestamp_log(MTK_SUSPEND_TIMESTAMP_GIC_P3);
		if (state_id != MTK_MCDI_MODE) {
			gic_setup();
			gic_dist_restore();

			dfd_resume();
		}
		mtk_suspend_timestamp_log(MTK_SUSPEND_TIMESTAMP_GIC_P4);

		/*
		 * This CPU might have woken up whilst the cluster was
		 * attempting to power down. In this case the MTK_plarform power
		 * controller will have a pending cluster power off request
		 * which needs to be cleared by writing to the PPONR register.
		 * This prevents the power controller from interpreting a
		 * subsequent entry of this cpu into a simple wfi as a power
		 * down request.
		 */
		sspm_cluster_pwr_on_notify(cluster);

		/* Enable coherency if this cluster was off */
		enable_scu(mpidr);
		plat_cci_enable();

		/* Enable rgu dcm if this cluster was off */
		plat_dcm_rgu_enable();

		/* Enable mcusys emi wfifo */
		enable_emi_wfifo();

#ifdef MTK_CM_MGR
		/* init cpu stall counter */
		/* init_cpu_stall_counter(cluster); */
#endif /* MTK_CM_MGR */
	}
}

/*******************************************************************************
* Private function which is used to determine if any platform actions
* should be performed for the specified affinity instance given its
* state. Nothing needs to be done if the 'state' is not off or if this is not
* the highest affinity level which will enter the 'state'.
*******************************************************************************/
static int32_t plat_do_plat_actions(unsigned int afflvl, unsigned int state)
{
	unsigned int max_phys_off_afflvl;

	assert(afflvl <= MPIDR_AFFLVL2);

	if (state != PSCI_STATE_OFF)
		return -EAGAIN;

	/*
	 * Find the highest affinity level which will be suspended and postpone
	 * all the platform specific actions until that level is hit.
	 */
	max_phys_off_afflvl = psci_get_max_phys_off_afflvl();
	assert(max_phys_off_afflvl != PSCI_INVALID_DATA);
	if (afflvl != max_phys_off_afflvl)
		return -EAGAIN;

	return 0;
}

/*******************************************************************************
 * MTK_platform handler called when an affinity instance is about to enter
 * standby.
 ******************************************************************************/
static void plat_affinst_standby(unsigned int power_state)
{
	unsigned int target_afflvl;
	int state_id = psci_get_suspend_stateid();

	/* Sanity check the requested state */
	target_afflvl = psci_get_pstate_afflvl(power_state);

	/*
	 * It's possible to enter standby only on affinity level 0 i.e. a cpu
	 * on the MTK_platform. Ignore any other affinity level.
	 */
	if (target_afflvl == MPIDR_AFFLVL0) {

		unsigned long mpidr;

		plat_spm_suspend(state_id);

		/* Enable the gic cpu interface */
		gic_cpuif_init();

		/* Get the mpidr for this cpu */
		mpidr = read_mpidr_el1();

		/*
		 * Enter standby state. dsb is good practice before using wfi
		 * to enter low power states.
		 */
		isb();
		dsb();

		/* Disable coherency if this cluster is to be turned off */

		plat_cci_disable();
		disable_scu(mpidr);

		wfi();

		/* Enable coherency if this cluster was off */
		enable_scu(mpidr);
		plat_cci_enable();

		plat_spm_suspend_finish(state_id);
	}
}

/*******************************************************************************
 * MTK_platform handler called when an affinity instance is about to be turned
 * on. The level and mpidr determine the affinity instance.
 ******************************************************************************/
#ifdef ATF_BYPASS_DRAM
static int plat_affinst_on(unsigned long mpidr, unsigned long sec_entrypoint,
			   unsigned int afflvl, unsigned int state)
{
	return PSCI_E_SUCCESS;
}
#else
static int plat_affinst_on(unsigned long mpidr, unsigned long sec_entrypoint,
			   unsigned int afflvl, unsigned int state)
{
	int cpu;
	int cluster;

	INFO("%s() mpidr = 0x%lx, sec_entrypoint = 0x%lx, afflvl = %d, state = %d\n",
	     __func__, mpidr, sec_entrypoint, afflvl, state);

	cpu = MPIDR_AFFLVL0_VAL(mpidr);
	cluster = MPIDR_AFFLVL1_VAL(mpidr);

	/* power on the cluster */
	if (afflvl >= MPIDR_AFFLVL1) {
		if (!spm_get_cluster_powerstate(cluster)) {
			spm_poweron_cluster(cluster);

#ifdef MTK_CM_MGR
			/* init cpu stall counter */
			/* init_cpu_stall_counter(cluster); */
#endif /* MTK_CM_MGR */
		}

		return PSCI_E_SUCCESS;
	}

	/* init cpu reset arch as AARCH64 */
	mcucfg_init_archstate(cluster, cpu, 1);
	mcucfg_set_bootaddr(cluster, cpu, sec_entrypoint);

	spm_poweron_cpu(cluster, cpu);

	return PSCI_E_SUCCESS;
}
#endif /* ATF_BYPASS_DRAM */

/*******************************************************************************
 * MTK_platform handler called when an affinity instance is about to be turned
 * off. The level and mpidr determine the affinity instance. The 'state' arg.
 * allows the platform to decide whether the cluster is being turned off and
 * take apt actions.
 *
 * CAUTION: This function is called with coherent stacks so that caches can be
 * turned off, flushed and coherency disabled. There is no guarantee that caches
 * will remain turned on across calls to this function as each affinity level is
 * dealt with. So do not write & read global variables across calls. It will be
 * wise to do flush a write to the global to prevent unpredictable results.
 ******************************************************************************/
static void plat_affinst_off(unsigned int afflvl, unsigned int state)
{
	int cpu;
	int cluster;
	uint64_t mpidr;

	INFO("%s: afflvl=%d, state=%d\n", __func__, afflvl, state);

	/* Determine if any platform actions need to be executed. */
	if (plat_do_plat_actions(afflvl, state) == -EAGAIN)
		return;

	mpidr = read_mpidr();
	cpu = MPIDR_AFFLVL0_VAL(mpidr);
	cluster = MPIDR_AFFLVL1_VAL(mpidr);

	spark_disable(cluster, cpu);

	/* Prevent interrupts from spuriously waking up this cpu */
	gic_rdist_save();
	gic_cpuif_deactivate(0);

	trace_power_flow(mpidr, CPU_DOWN);

	if (afflvl != MPIDR_AFFLVL0) {
		/* Disable coherency if this cluster is to be turned off */
		plat_cci_disable();

		disable_scu(mpidr);
		trace_power_flow(mpidr, CLUSTER_DOWN);
	}
}

/*******************************************************************************
 * MTK_platform handler called when an affinity instance is about to be
 * suspended. The level and mpidr determine the affinity instance. The 'state'
 * arg. allows the platform to decide whether the cluster is being turned off
 * and take apt actions.
 *
 * CAUTION: This function is called with coherent stacks so that caches can be
 * turned off, flushed and coherency disabled. There is no guarantee that caches
 * will remain turned on across calls to this function as each affinity level is
 * dealt with. So do not write & read global variables across calls. It will be
 * wise to do flush a write to the global to prevent unpredictable results.
 ******************************************************************************/
static void plat_affinst_suspend(unsigned long sec_entrypoint,
				 unsigned int afflvl,
				 unsigned int state)
{
	int cpu;
	int cluster;
	int linear_id;
	uint64_t mpidr;
	int state_id;

	/* Determine if any platform actions need to be executed. */
	if (plat_do_plat_actions(afflvl, state) == -EAGAIN)
		return;

	mpidr = read_mpidr();
	cpu = MPIDR_AFFLVL0_VAL(mpidr);
	cluster = MPIDR_AFFLVL1_VAL(mpidr);
	linear_id = (cluster << 2) + cpu;
	state_id = psci_get_suspend_stateid();

	mtk_suspend_footprint_log(MTK_SUSPEND_FOOTPRINT_ENTER_SPM_SUSPEND);
	mtk_suspend_timestamp_log(MTK_SUSPEND_TIMESTAMP_ENTER_SPM_SUSPEND);
	plat_spm_suspend(state_id);
	mtk_suspend_footprint_log(MTK_SUSPEND_FOOTPRINT_LEAVE_SPM_SUSPEND);
	mtk_suspend_timestamp_log(MTK_SUSPEND_TIMESTAMP_LEAVE_SPM_SUSPEND);

	spark_disable(cluster, cpu);

	/* Perform the common cpu specific operations */
	plat_cpu_pwrdwn_common(cluster, cpu, state_id);

	/* MCDI: enable standbywfi IRQ mask */
	plat_mcdi_enter(linear_id, state_id);

	/* Perform the common cluster specific operations */
	if (afflvl != MPIDR_AFFLVL0) {
		sspm_cluster_pwr_off_notify(cluster);
		plat_cluster_pwrdwn_common(mpidr, state_id, cluster);
	}

	/* Perform the common mcusys specific operations */
	plat_mcusys_pwrdwn_common(state_id);

	if (state_id == MTK_SODI3_MODE || state_id == MTK_SUSPEND_MODE)
		MT_LOG_KTIME_CLEAR();

	plat_dcm_mcsi_a_backup();
	set_cpu_suspend_state(linear_id);
}

/*******************************************************************************
 * MTK_platform handler called when an affinity instance has just been powered
 * on after being turned off earlier. The level and mpidr determine the affinity
 * instance. The 'state' arg. allows the platform to decide whether the cluster
 * was turned off prior to wakeup and do what's necessary to setup it up
 * correctly.
 ******************************************************************************/
static void plat_affinst_on_finish(unsigned int afflvl, unsigned int state)
{
	int cpu;
	int cluster;
	uint64_t mpidr;

	/* Determine if any platform actions need to be executed. */
	if (plat_do_plat_actions(afflvl, state) == -EAGAIN)
		return;

	mpidr = read_mpidr();
	cpu = MPIDR_AFFLVL0_VAL(mpidr);
	cluster = MPIDR_AFFLVL1_VAL(mpidr);

	/* Perform the common cluster specific operations */
	if (afflvl >= MPIDR_AFFLVL1) {
		enable_scu(mpidr);

		/* Enable coherency if this cluster was off */
		plat_cci_enable();

		/* Enable rgu dcm if this cluster was off */
		plat_dcm_rgu_enable();
		trace_power_flow(mpidr, CLUSTER_UP);
	}

	spark_enable(cluster, cpu);

	/* Enable the gic cpu interface */
	gic_cpuif_init();
	gic_rdist_restore();

	trace_power_flow(mpidr, CPU_UP);
}

/*******************************************************************************
 * MTK_platform handler called when an affinity instance has just been powered
 * on after having been suspended earlier. The level and mpidr determine the
 * affinity instance.
 ******************************************************************************/
static void plat_affinst_suspend_finish(unsigned int afflvl, unsigned int state)
{
	int cpu;
	int cluster;
	int linear_id;
	uint64_t mpidr;
	int state_id;

	/* Determine if any platform actions need to be executed. */
	if (plat_do_plat_actions(afflvl, state) == -EAGAIN)
		return;

	mpidr = read_mpidr();
	cpu = MPIDR_AFFLVL0_VAL(mpidr);
	cluster = MPIDR_AFFLVL1_VAL(mpidr);
	linear_id = (cluster << 2) + cpu;
	state_id = psci_get_suspend_stateid();

	enable_cpu_status_for_cache(linear_id);

	plat_power_domain_on_finish_common(afflvl, state_id, mpidr, cluster);

	mtk_suspend_footprint_log(MTK_SUSPEND_FOOTPRINT_ENTER_SPM_SUSPEND_FINISH);
	mtk_suspend_timestamp_log(MTK_SUSPEND_TIMESTAMP_ENTER_SPM_SUSPEND_FINISH);
	plat_spm_suspend_finish(state_id);
	mtk_suspend_footprint_log(MTK_SUSPEND_FOOTPRINT_LEAVE_SPM_SUSPEND_FINISH);
	mtk_suspend_timestamp_log(MTK_SUSPEND_TIMESTAMP_LEAVE_SPM_SUSPEND_FINISH);

	/* Enable the gic cpu interface */
	gic_cpuif_init();

	if (state_id == MTK_MCDI_MODE || state_id == MTK_SUSPEND_MODE)
		gic_rdist_restore();
	else
		gic_rdist_restore_all();

	spark_enable(cluster, cpu);

	plat_dcm_mcsi_a_restore();
	clear_cpu_suspend_state(linear_id);
}

static unsigned int plat_get_sys_suspend_power_state(void)
{
	/* StateID: 0, StateType: 1(power down), PowerLevel: 2(system) */
	return psci_make_powerstate(0, 1, 2);
}

/*******************************************************************************
 * MTK handlers to shutdown/reboot the system
 ******************************************************************************/
static void __dead2 plat_system_off(void)
{
	INFO("MTK System Off\n");

#ifndef ATF_BYPASS_DRAM
	gpio_set(GPIO120, GPIO_OUT_ZERO);
	rtc_bbpu_power_down();
#endif

	wfi();
	ERROR("MTK System Off: operation not handled.\n");
	panic();
}

static void __dead2 plat_system_reset(void)
{
	/* Write the System Configuration Control Register */
	INFO("MTK System Reset\n");

	plat_rgu_sw_reset();

	wfi();
	ERROR("MTK System Reset: operation not handled.\n");
	panic();
}

/*******************************************************************************
 * Export the platform handlers to enable psci to invoke them
 ******************************************************************************/
static const plat_pm_ops_t plat_plat_pm_ops = {
	.affinst_standby		= plat_affinst_standby,
	.affinst_on			= plat_affinst_on,
	.affinst_off			= plat_affinst_off,
	.affinst_suspend		= plat_affinst_suspend,
	.affinst_on_finish		= plat_affinst_on_finish,
	.affinst_suspend_finish		= plat_affinst_suspend_finish,
	.system_off			= plat_system_off,
	.system_reset			= plat_system_reset,
	.get_sys_suspend_power_state	= plat_get_sys_suspend_power_state,
};

/*******************************************************************************
 * Export the platform specific power ops & initialize the mtk_platform power
 * controller
 ******************************************************************************/
int platform_setup_pm(const plat_pm_ops_t **plat_ops)
{
	*plat_ops = &plat_plat_pm_ops;

	/*
	 * init the warm reset config for boot CPU
	 *  a. reset arch as AARCH64
	 *  b. reset addr as function bl31_warm_entrypoint()
	 */
	mcucfg_init_archstate(0, 0, 1);
	mcucfg_set_bootaddr(0, 0, (uintptr_t) bl31_warm_entrypoint);

#ifdef MTK_CM_MGR
	/* init cpu stall counter */
	init_cpu_stall_counter_all();
#endif /* MTK_CM_MGR */

	return 0;
}
