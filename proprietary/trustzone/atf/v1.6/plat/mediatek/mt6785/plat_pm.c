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
#include <arch_helpers.h>
#include <assert.h>
#include <debug.h>
#include <mmio.h>
#include <psci.h>
#include <errno.h>
#include <platform.h>
#include <bl31.h>

/* mediatek platform specific headers */
#include <platform_def.h>
#include <scu.h>
#include <mtk_plat_common.h>
#include <mt_spm.h>
#include <mt_spm_syspll.h>
#include <mt_spm_dram_bus26m.h>
#include <mtspmc.h>
#include <mtk_mcdi.h>
#include <power_tracer.h>
#include <plat_debug.h>
#include <plat_pm.h>
#include <plat_private.h>
#include <plat_dcm.h>
#include <rtc.h>
#include <log.h>
#include <mtk_plat_gpio.h>
#include <mtk_gic_v3_main.h>
#include <mt_ildo_atf.h>
#include <mtk_cm_mgr.h>
#include <mtk_rgu.h>
#if USE_COHERENT_MEM
#define __coherent  __section("tzfw_coherent_mem")
#else
#define __coherent
#endif

#define STA_POWER_DOWN		0
#define STA_POWER_ON		1

#if ENABLE_PLAT_COMPAT
static int is_mtk_suspend[PLATFORM_CORE_COUNT] __coherent;
uintptr_t mtk_suspend_footprint_addr __coherent;
uintptr_t mtk_suspend_timestamp_addr __coherent;
static unsigned int emi_selftest_start, emi_selftest_end, emi_selftest_msb;

static void save_emi_selftest_range(void)
{
	emi_selftest_start = mmio_read_32(EMI_CEN_BASE + 0x98c);
	emi_selftest_end = mmio_read_32(EMI_CEN_BASE + 0x990);
	emi_selftest_msb = mmio_read_32(EMI_CEN_BASE + 0x994);
}

static void restore_emi_selftest_range(void)
{
	mmio_write_32(EMI_CEN_BASE + 0x98c, emi_selftest_start);
	mmio_write_32(EMI_CEN_BASE + 0x990, emi_selftest_end);
	mmio_write_32(EMI_CEN_BASE + 0x994, emi_selftest_msb);
}

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
#endif

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
#ifndef ATF_BYPASS_DRAM
	int cluster, cpu;

	if (cpux >= PLATFORM_CORE_COUNT) {
		ERROR("[ATF]: %s() CPU%d not exists\n", __func__, (int)cpux);
		assert(0);
	}

	INFO("%s: cpu=%d, mode=%d\n", __func__, cpux, mode);

	cluster = 0;
	cpu = cpux;

	if (state == STA_POWER_DOWN)
		spm_poweroff_cpu(cluster, cpu);
	else
		spm_poweron_cpu(cluster, cpu);
#endif

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

#if ENABLE_PLAT_COMPAT
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
	case MTK_IDLEDRAM_MODE:
		/* This block is for DRAM */
		spm_dram_bus26m(state_id);
		break;
	case MTK_IDLEBUS26M_MODE:
		/* This block is for BUS26M */
		spm_dram_bus26m(state_id);
		break;
	case MTK_IDLESYSPLL_MODE:
		/* This block is for SYSPLL */
		spm_syspll(state_id);
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
	case MTK_IDLEDRAM_MODE:
		/* This block is for DRAM */
		spm_dram_bus26m_finish(state_id);
		break;
	case MTK_IDLEBUS26M_MODE:
		/* This block is for BUS26M */
		spm_dram_bus26m_finish(state_id);
		break;
	case MTK_IDLESYSPLL_MODE:
		/* This block is for SYSPLL */
		spm_syspll_finish(state_id);
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

static char is_any_core_spm_mode(int state_id)
{
	return (state_id == MTK_IDLEBUS26M_MODE ||
		state_id == MTK_IDLESYSPLL_MODE ||
		state_id == MTK_IDLEDRAM_MODE);
}

/*
 * The cluster in ARMv8.2 was powered off by default when
 * all cores were off. MCDI would judge the all cores off
 * and included cluster off or not.
 *
 * We turned off the default mechanism and let MCDI take over
 */
static void coordinate_cluster_pwrdn(void)
{
	int pstate, plvl;

	pstate = psci_get_suspend_powerstate();
	plvl = psci_get_pstate_pwrlvl(pstate);

	/*
	 * if CPU requested cluster power down,
	 * set CLUSTERPWRDN_EL1[0] = 0
	 */
	if (plvl)
		write_clusterpwrdn_el1(0);
	else
		write_clusterpwrdn_el1(1);
}

/*******************************************************************************
 * Function which implements the common MTK_platform specific operations to power down a
 * cpu in response to a CPU_OFF or CPU_SUSPEND request.
 ******************************************************************************/
static void plat_cpu_pwrdwn_common(int cluster, int cpu, int state_id,
					unsigned int afflvl)
{
	unsigned int idx = plat_my_core_pos();
	unsigned long long from_isolated_core = (get_isolation_status() & (1ULL << idx)) >> idx;

	if (from_isolated_core)
		gic_dpg_cfg_set();

	/* Prevent interrupts from spuriously waking up this cpu */
	gic_rdist_save();
	gic_cpuif_deactivate(0);
	gic600_rdistif_deactivate();

	coordinate_cluster_pwrdn();

	/* mcdi cpu pwrdwn intreface */
	if (state_id == MTK_MCDI_MODE)
		plat_mcdi_before_cpu_pwrdwn(cpu, cluster, afflvl, MPIDR_AFFLVL0);
}

/*******************************************************************************
 * Function which implements the common MTK_platform specific operations to power down a
 * cluster in response to a CPU_OFF or CPU_SUSPEND request.
 ******************************************************************************/
static void plat_cluster_pwrdwn_common(uint64_t mpidr, int state_id)
{
	mtk_suspend_timestamp_log(MTK_SUSPEND_TIMESTAMP_GIC_P1);
#ifndef ATF_BYPASS_DRAM
	if (state_id == MTK_MCDI_MODE)
		plat_mcdi_before_cluster_pwrdwn();
	else
		gic_dist_save();
#endif
	save_emi_selftest_range();
	mtk_suspend_timestamp_log(MTK_SUSPEND_TIMESTAMP_GIC_P2);
}

/*******************************************************************************
 * Function which implements the common MTK_platform specific operations to power down
 * mcusys in response to a CPU_OFF or CPU_SUSPEND request.
 ******************************************************************************/
static void plat_mcusys_pwrdwn_common(int state_id)
{
	if (is_any_core_spm_mode(state_id))
		gic_sgi_save_all();

	if (state_id != MTK_MCDI_MODE)
		cpc_set_mcusys_off_en();
	if (state_id == MTK_SUSPEND_MODE) {
		dvfsrc_backup_req();
		/* Switch to md ddr non-turbo table */
		dvfsrc_md_ddr_turbo(0);
	}
}

static void plat_power_domain_on_finish_common(unsigned int afflvl, int state_id,
					       uint64_t mpidr, int cluster)
{
	if (afflvl == MPIDR_AFFLVL0)
		return;

	/* setup circular buffer */
	circular_buffer_setup();

	/* setup sram delsel */
	sram_delsel_setup();

	/* Perform the common mcusys specific operations */
	if (is_any_core_spm_mode(state_id))
		gic_sgi_restore_all();

	mtk_suspend_timestamp_log(MTK_SUSPEND_TIMESTAMP_RESERVE_P1);
	mtk_suspend_timestamp_log(MTK_SUSPEND_TIMESTAMP_RESERVE_P2);
	mtk_suspend_timestamp_log(MTK_SUSPEND_TIMESTAMP_GIC_P3);

#if 0/*ILDO_ENABLE*/
	ildo_init(cpu, 1);
#endif

	if (state_id != MTK_MCDI_MODE) {
		cpc_clr_mcusys_off_en();

		gic_setup();
		restore_emi_selftest_range();
		gic_dist_restore();
#ifndef ATF_BYPASS_DRAM
		dfd_resume();
#endif
	}
	/* Switch to md ddr turbo table */
	dvfsrc_md_ddr_turbo(1);

	if (state_id == MTK_SUSPEND_MODE)
		dvfsrc_restore_req();

	mtk_suspend_timestamp_log(MTK_SUSPEND_TIMESTAMP_GIC_P4);

#ifndef ATF_BYPASS_DRAM
	/* Perform the common cluster specific operations */
	if (state_id == MTK_MCDI_MODE)
		plat_mcdi_after_cluster_pwron();
#endif

#ifndef ATF_BYPASS_DRAM
	/* Enable rgu dcm if this cluster was off */
	plat_dcm_rgu_enable();
#endif

#if defined(MTK_CM_MGR) && !defined(MTK_FPGA_EARLY_PORTING)
	/* init cpu stall counter */
	init_cpu_stall_counter_all();
#endif /* MTK_CM_MGR */

	if ((mmio_read_32(SPM_SW_RSV_0) & R12_MD1_WDT_B) &&
			mmio_read_32(MD_WDTSR) & (0x7  << 0x8)) {
		INFO("trigger MD WDT IRQ\n");
		INFO("SPM_SW_RSV_0: 0x%x, WDTCR: 0x%x, WDTSR: 0x%x\n",
				mmio_read_32(SPM_SW_RSV_0), mmio_read_32(MD_WDTCR), mmio_read_32(MD_WDTSR));
		mt_trigger_md_wdt_irq();
	}

	/* trigger WDT FIQ if needed while leaving from low power scenario */
	if (plat_rgu_read_status() &
		(MTK_WDT_STATUS_EINT_RST | MTK_WDT_STATUS_SYSRST_RST))
		mt_atf_trigger_WDT_FIQ();
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

	INFO("%s: mpidr=0x%lx, sec_entrypoint=0x%lx, afflvl=%d, state=%d\n",
	     __func__, mpidr, sec_entrypoint, afflvl, state);

	cpu = plat_core_pos_by_mpidr(mpidr);
	cluster = 0;

	/* power on the cluster */
	if (afflvl >= MPIDR_AFFLVL1) {
		if (!spm_get_cluster_powerstate(cluster)) {
			spm_poweron_cluster(cluster);

#if defined(MTK_CM_MGR) && !defined(MTK_FPGA_EARLY_PORTING)
			/* init cpu stall counter */
			init_cpu_stall_counter_all();
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
	unsigned long mpidr;

	INFO("%s: afflvl=%d, state=%d\n", __func__, afflvl, state);

	/* Determine if any platform actions need to be executed. */
	if (plat_do_plat_actions(afflvl, state) == -EAGAIN)
		return;

	mpidr = read_mpidr_el1();

	/* Prevent interrupts from spuriously waking up this cpu */
	gic_dpg_cfg_set();
	gic_rdist_save();
	gic_cpuif_deactivate(0);
	gic600_rdistif_deactivate();

	/* prevent unintended IRQs from waking up the hot-unplugged core */
	mcucfg_disable_gic_wakeup(0, plat_core_pos_by_mpidr(mpidr));

	/*
	 * agree cluster off even this core was going to be
	 * hot-unplgged. In case of this core refused to cluster off
	 * in plat_affinst_suspend.
	 */
	write_clusterpwrdn_el1(0);

	trace_power_flow(mpidr, CPU_DOWN);

	if (afflvl != MPIDR_AFFLVL0)
		trace_power_flow(mpidr, CLUSTER_DOWN);
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
	uint64_t mpidr;
	int state_id;

	/* Determine if any platform actions need to be executed. */
	if (plat_do_plat_actions(afflvl, state) == -EAGAIN)
		return;

	mpidr = read_mpidr();
	cpu = plat_core_pos_by_mpidr(mpidr);
	cluster = 0;
	state_id = psci_get_suspend_stateid();

	mtk_suspend_footprint_log(MTK_SUSPEND_FOOTPRINT_ENTER_SPM_SUSPEND);
	mtk_suspend_timestamp_log(MTK_SUSPEND_TIMESTAMP_ENTER_SPM_SUSPEND);
	plat_spm_suspend(state_id);
	mtk_suspend_footprint_log(MTK_SUSPEND_FOOTPRINT_LEAVE_SPM_SUSPEND);
	mtk_suspend_timestamp_log(MTK_SUSPEND_TIMESTAMP_LEAVE_SPM_SUSPEND);

#ifndef ATF_BYPASS_DRAM
	spark_disable(cluster, cpu);
#endif

	/* Perform the common cpu specific operations */
	plat_cpu_pwrdwn_common(cluster, cpu, state_id, afflvl);

	/* Perform the common cluster specific operations */
	if (afflvl != MPIDR_AFFLVL0)
		plat_cluster_pwrdwn_common(mpidr, state_id);

	/* Perform the common mcusys specific operations */
	plat_mcusys_pwrdwn_common(state_id);

	if (state_id == MTK_IDLEBUS26M_MODE || state_id == MTK_SUSPEND_MODE)
		MT_LOG_KTIME_CLEAR();

#ifndef ATF_BYPASS_DRAM
	plat_dcm_mcsi_a_backup();
#endif
	set_cpu_suspend_state(cpu);
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
	unsigned long mpidr = read_mpidr_el1();
#ifndef ATF_BYPASS_DRAM
	unsigned long cpu = plat_core_pos_by_mpidr(mpidr);
#endif

	/* Determine if any platform actions need to be executed. */
	if (plat_do_plat_actions(afflvl, state) == -EAGAIN)
		return;

#ifndef ATF_BYPASS_DRAM
	if (afflvl >= MPIDR_AFFLVL1) {
		/* Enable rgu dcm if this cluster was off */
		plat_dcm_rgu_enable();
		trace_power_flow(mpidr, CLUSTER_UP);
	}

#if ILDO_ENABLE
	/* for init(only on system boot up) */
	ildo_init(cpu, 0);
#endif

	spark_enable(0, cpu);
#endif

	/* Allow IRQs to wakeup this core in IDLE flow */
	mcucfg_enable_gic_wakeup(0, cpu);

	/* Enable the gic cpu interface */
	gic600_rdistif_init();
	gic_cpuif_init();
	gic_rdist_restore();
	gic_dpg_cfg_clr();

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
	uint64_t mpidr;
	int state_id;

	/* Determine if any platform actions need to be executed. */
	if (plat_do_plat_actions(afflvl, state) == -EAGAIN)
		return;

	mpidr = read_mpidr();
	cpu = plat_core_pos_by_mpidr(mpidr);
	cluster = 0;
	state_id = psci_get_suspend_stateid();

	/*
	 * Hint CPC that an cpu is on, then CPC can spend much less time on
	 * last core protect mechanism
	 */
	cpc_core_on_hint(cpu);

	if (state_id == MTK_MCDI_MODE)
		plat_mcdi_after_cpu_pwron(cpu, cluster);

	plat_power_domain_on_finish_common(afflvl, state_id, mpidr, cluster);

	mtk_suspend_footprint_log(MTK_SUSPEND_FOOTPRINT_ENTER_SPM_SUSPEND_FINISH);
	mtk_suspend_timestamp_log(MTK_SUSPEND_TIMESTAMP_ENTER_SPM_SUSPEND_FINISH);
	plat_spm_suspend_finish(state_id);
	mtk_suspend_footprint_log(MTK_SUSPEND_FOOTPRINT_LEAVE_SPM_SUSPEND_FINISH);
	mtk_suspend_timestamp_log(MTK_SUSPEND_TIMESTAMP_LEAVE_SPM_SUSPEND_FINISH);

	/* Enable the gic cpu interface */
	gic600_rdistif_init();
	gic_cpuif_init();

	if (state_id == MTK_MCDI_MODE || state_id == MTK_SUSPEND_MODE)
		gic_rdist_restore();
	else
		gic_rdist_restore_all();
	gic_dpg_cfg_clr();

#ifndef ATF_BYPASS_DRAM
#if ILDO_ENABLE
	ildo_init(cpu, 1);
#endif
	spark_enable(cluster, cpu);

	plat_dcm_mcsi_a_restore();
#endif
	clear_cpu_suspend_state(cpu);
}

static unsigned int plat_get_sys_suspend_power_state(void)
{
	/* StateID: 0, StateType: 1(power down), PowerLevel: 2(system) */
	return psci_make_powerstate(0, 1, 2);
}
#else
/*******************************************************************************
 * Clone from MT8173
 ******************************************************************************/
static void plat_cpu_standby(plat_local_state_t cpu_state)
{
}

static uintptr_t secure_entrypoint;

static int plat_power_domain_on(unsigned long mpidr)
{
	return 0;
}

static void plat_power_domain_off(const psci_power_state_t *state)
{
}

static void plat_power_domain_suspend(const psci_power_state_t *state)
{
}

static void plat_power_domain_on_finish(const psci_power_state_t *state)
{
}

static void plat_power_domain_suspend_finish(const psci_power_state_t *state)
{
}

static int plat_validate_power_state(unsigned int power_state,
					psci_power_state_t *req_state)
{
	return 0;
}

static void plat_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
}
#endif

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
#if ENABLE_PLAT_COMPAT
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
#else
static const plat_psci_ops_t plat_plat_pm_ops = {
	.cpu_standby			= plat_cpu_standby,
	.pwr_domain_on			= plat_power_domain_on,
	.pwr_domain_on_finish		= plat_power_domain_on_finish,
	.pwr_domain_off			= plat_power_domain_off,
	.pwr_domain_suspend		= plat_power_domain_suspend,
	.pwr_domain_suspend_finish	= plat_power_domain_suspend_finish,
	.system_off			= plat_system_off,
	.system_reset			= plat_system_reset,
	.validate_power_state		= plat_validate_power_state,
	.get_sys_suspend_power_state	= plat_get_sys_suspend_power_state,
};
#endif

#if ENABLE_PLAT_COMPAT
/*******************************************************************************
 * Export the platform specific power ops & initialize the mtk_platform power
 * controller
 ******************************************************************************/
int platform_setup_pm(const plat_pm_ops_t **plat_ops)
{
	*plat_ops = &plat_plat_pm_ops;

#ifndef ATF_BYPASS_DRAM
	/*
	 * init the warm reset config for boot CPU
	 * reset arch as AARCH64
	 * reset addr as function bl31_warm_entrypoint()
	 */
	mcucfg_init_archstate(0, 0, 1);
	mcucfg_set_bootaddr(0, 0, (uintptr_t) bl31_warm_entrypoint);
#endif

#if defined(MTK_CM_MGR) && !defined(MTK_FPGA_EARLY_PORTING)
	/* init cpu stall counter */
	init_cpu_stall_counter_all();
#endif /* MTK_CM_MGR */
	return 0;
}
#else
int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	*psci_ops = &plat_plat_pm_ops;
	secure_entrypoint = sec_entrypoint;
	return 0;
}

/*
 * The PSCI generic code uses this API to let the platform participate in state
 * coordination during a power management operation. It compares the platform
 * specific local power states requested by each cpu for a given power domain
 * and returns the coordinated target power state that the domain should
 * enter. A platform assigns a number to a local power state. This default
 * implementation assumes that the platform assigns these numbers in order of
 * increasing depth of the power state i.e. for two power states X & Y, if X < Y
 * then X represents a shallower power state than Y. As a result, the
 * coordinated target local power state for a power domain will be the minimum
 * of the requested local power states.
 */
plat_local_state_t plat_get_target_pwr_state(unsigned int lvl,
					     const plat_local_state_t *states,
					     unsigned int ncpu)
{
	plat_local_state_t target = PLAT_MAX_OFF_STATE, temp;

	assert(ncpu);

	do {
		temp = *states++;
		if (temp < target)
			target = temp;
	} while (--ncpu);

	return target;
}
#endif
