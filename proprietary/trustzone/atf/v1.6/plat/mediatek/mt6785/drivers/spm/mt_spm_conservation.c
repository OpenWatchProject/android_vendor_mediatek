#include <arch_helpers.h>
#include <debug.h>
#include <mmio.h>
#include <mt_spm.h>
#include <mt_spm_reg.h>
#include <mt_spm_vcorefs.h>
#include <plat_pm.h>
#include <platform.h>
#include <platform_def.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <mt_spm_conservation.h>


static struct wake_status spm_wakesta; /* record last wakesta */
static wake_reason_t spm_wake_reason = WR_NONE;

void go_to_spm_before_wfi_no_resume(
	int stateId, unsigned int ext_opand,
	struct spm_lp_scen *spm_lp, int resource_req)
{
	struct pwr_ctrl *pwrctrl;
	uint64_t mpidr = read_mpidr();
	uint32_t cpu = plat_core_pos_by_mpidr(mpidr);

	pwrctrl = spm_lp->pwrctrl;

	/*
	 * 1) Setup scenario setting
	 * 2) Set CPU to SPM wakeup event, trigger SPM FW scenario switch
	 */
	__spm_set_fw_resume_option(pwrctrl);

	__spm_set_cpu_status(cpu);

	__spm_set_power_control(pwrctrl);

	__spm_set_wakeup_event(pwrctrl);

	__spm_sync_vcore_dvfs_power_control(pwrctrl, __spm_vcorefs.pwrctrl);

	__spm_set_pcm_flags(pwrctrl);

	__spm_src_req_update(pwrctrl, resource_req);

	if (stateId == MTK_IDLEBUS26M_MODE)
		__spm_set_pcm_wdt(1);

	__spm_send_cpu_wakeup_event();
}

void go_to_spm_before_wfi(
	int stateId, unsigned int ext_opand,
	struct spm_lp_scen *spm_lp, int resource_req)
{
	struct pcm_desc *pcmdesc = NULL;
	struct pwr_ctrl *pwrctrl;
	uint64_t mpidr = read_mpidr();
	uint32_t cpu = plat_core_pos_by_mpidr(mpidr);
	int spmfw_idx = __spm_get_spmfw_idx();

	if (dyna_load_pcm[spmfw_idx].ready) {
		pcmdesc = &(dyna_load_pcm[spmfw_idx].desc);
	} else {
		INFO("firmware is not ready!!!\n");
		return;
	}

	pwrctrl = spm_lp->pwrctrl;

#if 0
	INFO("Online CPU is %d, suspend FW ver. is %s\n",
			cpu, pcmdesc->version);

	INFO("sec = %u, wakesrc = 0x%x (%u)(%u)\n",
			pwrctrl->timer_val, pwrctrl->wake_src,
			is_cpu_pdn(pwrctrl->pcm_flags),
			is_infra_pdn(pwrctrl->pcm_flags));
#endif

	__spm_set_cpu_status(cpu);
	__spm_reset_and_init_pcm(pcmdesc);
	__spm_kick_im_to_fetch(pcmdesc);
	__spm_init_pcm_register();

	__spm_sync_vcore_dvfs_power_control(pwrctrl, __spm_vcorefs.pwrctrl);

	__spm_set_power_control(pwrctrl);
	__spm_src_req_update(pwrctrl, resource_req);
	__spm_set_wakeup_event(pwrctrl);

	if (stateId == MTK_IDLEBUS26M_MODE)
		__spm_set_pcm_wdt(1);

	__spm_kick_pcm_to_run(pwrctrl);
}

void go_to_spm_after_wfi(int stateId, unsigned int ext_opand
	, struct spm_lp_scen *spm_lp)
{
	struct pcm_desc *pcmdesc = NULL;
	int spmfw_idx = __spm_get_spmfw_idx();

	if (dyna_load_pcm[spmfw_idx].ready) {
		pcmdesc = &(dyna_load_pcm[spmfw_idx].desc);
	} else {
		INFO("firmware is not ready!!!\n");
		return;
	}
	__spm_ext_int_wakeup_req_clr();

	if (stateId == MTK_IDLEBUS26M_MODE)
		__spm_set_pcm_wdt(0);

	__spm_get_wakeup_status(&spm_wakesta);
	__spm_clean_after_wakeup();
	spm_wake_reason = __spm_output_wake_reason(&spm_wakesta, pcmdesc);
#if 0
	INFO("spm_wake_reason=%d\n", spm_wake_reason);
#endif
}

int spm_conservation(int stateId, unsigned int ext_opand
	, struct spm_lp_scen *spm_lp, int resource_req)
{
	if (!spm_lp)
		return -1;

	spm_lock_get();

#if SPM_FW_NO_RESUME
	go_to_spm_before_wfi_no_resume(stateId, ext_opand
				, spm_lp, resource_req);
#else
	go_to_spm_before_wfi(stateId, ext_opand
				, spm_lp, resource_req);
#endif

	spm_lock_release();

	return 0;
}

void spm_conservation_finish(int stateId
	, unsigned int ext_opand, struct spm_lp_scen *spm_lp)
{
	spm_lock_get();
	go_to_spm_after_wfi(stateId, ext_opand, spm_lp);
	spm_lock_release();
}

