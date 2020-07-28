#include <arch_helpers.h>
#include <debug.h>
#include <mmio.h>
#include <mt_spm.h>
#include <mt_spm_internal.h>
#include <mt_spm_reg.h>
#include <mt_spm_vcorefs.h>
#include <plat_pm.h>
#include <platform.h>
#include <platform_def.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static struct wake_status spm_wakesta; /* record last wakesta */
static wake_reason_t spm_wake_reason = WR_NONE;

static unsigned int resource_usage;

static unsigned char is_sleep_dpidle;
static unsigned int dpidle_pcm_timer_val;
static unsigned int dpidle_wake_src;

#define __WAKE_SRC_FOR_DPIDLE_COMMON__ ( \
	(R12_PCM_TIMER_EVENT) | \
	(R12_KP_IRQ_B) | \
	(R12_APWDT_EVENT_B) | \
	(R12_APXGPT1_EVENT_B) | \
	(R12_CONN2AP_SPM_WAKEUP_B) | \
	(R12_EINT_EVENT_B) | \
	(R12_CONN_WDT_IRQ_B) | \
	(R12_CCIF0_EVENT_B) | \
	(R12_SC_SSPM2SPM_WAKEUP) | \
	(R12_SC_SCP2SPM_WAKEUP) | \
	(R12_SC_ADSP2SPM_WAKEUP) | \
	(R12_USBX_CDSC_B) | \
	(R12_USBX_POWERDWN_B) | \
	(R12_SYS_TIMER_EVENT_B) | \
	(R12_EINT_EVENT_SECURE_B) | \
	(R12_CCIF1_EVENT_B) | \
	(R12_AFE_IRQ_MCU_B) | \
	(R12_SYS_CIRQ_IRQ_B) | \
	(R12_MD2AP_PEER_WAKEUP_EVENT) | \
	(R12_MD1_WDT_B) | \
	(R12_AP2AP_PEER_WAKEUP_EVENT) | \
	(R12_SPM_CPU_WAKEUP_EVENT))

#if defined(CFG_MICROTRUST_TEE_SUPPORT)
#define WAKE_SRC_FOR_DPIDLE \
	(__WAKE_SRC_FOR_DPIDLE_COMMON__)
#else
#define WAKE_SRC_FOR_DPIDLE \
	(__WAKE_SRC_FOR_DPIDLE_COMMON__ | R12_SEJ_EVENT_B)
#endif

static struct pwr_ctrl dpidle_ctrl = {
	.wake_src = WAKE_SRC_FOR_DPIDLE,

	/* Auto-gen Start */

	/* SPM_AP_STANDBY_CON */
#if MCUSYS_MTCMOS_ON
	.reg_wfi_op = 1,
	.reg_wfi_type = 0,
	.reg_mp0_cputop_idle_mask = 1,
	.reg_mp1_cputop_idle_mask = 1,
	.reg_mcusys_idle_mask = 0,
	.reg_md_apsrc_1_sel = 0,
	.reg_md_apsrc_0_sel = 0,
	.reg_conn_apsrc_sel = 0,
#else
	.reg_wfi_type = 1,
	.reg_md_apsrc_1_sel = 0,
	.reg_md_apsrc_0_sel = 0,
	.reg_conn_apsrc_sel = 0,
#endif

	/* SPM_SRC_REQ */
	.reg_spm_apsrc_req = 0,
	.reg_spm_f26m_req = 0,
	.reg_spm_infra_req = 0,
	.reg_spm_vrf18_req = 0,
	.reg_spm_ddr_en_req = 0,
	.reg_spm_ddr_en2_req = 0,
	.reg_spm_dvfs_req = 0,
	.reg_spm_sw_mailbox_req = 0,
	.reg_spm_sspm_mailbox_req = 0,
	.reg_spm_adsp_mailbox_req = 0,
	.reg_spm_scp_mailbox_req = 0,
	.reg_spm_mcusys_pwr_event_req = 0,
	.cpu_md_dvfs_sop_force_on = 0,

	/* SPM_SRC_MASK */
	.reg_md_srcclkena_0_mask_b = 1,
	.reg_md_srcclkena2infra_req_0_mask_b = 0,
	.reg_md_apsrc2infra_req_0_mask_b = 1,
	.reg_md_apsrc_req_0_mask_b = 1,
	.reg_md_vrf18_req_0_mask_b = 1,
	.reg_md_ddr_en_0_mask_b = 1,
	.reg_md_ddr_en2_0_mask_b = 0,
	.reg_md_srcclkena_1_mask_b = 0,
	.reg_md_srcclkena2infra_req_1_mask_b = 0,
	.reg_md_apsrc2infra_req_1_mask_b = 0,
	.reg_md_apsrc_req_1_mask_b = 0,
	.reg_md_vrf18_req_1_mask_b = 0,
	.reg_md_ddr_en_1_mask_b = 0,
	.reg_md_ddr_en2_1_mask_b = 0,
	.reg_conn_srcclkena_mask_b = 1,
	.reg_conn_srcclkenb_mask_b = 0,
	.reg_conn_infra_req_mask_b = 1,
	.reg_conn_apsrc_req_mask_b = 1,
	.reg_conn_vrf18_req_mask_b = 1,
	.reg_conn_ddr_en_mask_b = 1,
	.reg_conn_ddr_en2_mask_b = 0,
	.reg_srcclkeni0_srcclkena_mask_b = 1,
	.reg_srcclkeni0_infra_req_mask_b = 1,
	.reg_srcclkeni1_srcclkena_mask_b = 0,
	.reg_srcclkeni1_infra_req_mask_b = 0,
	.reg_srcclkeni2_srcclkena_mask_b = 0,
	.reg_srcclkeni2_infra_req_mask_b = 0,
	.reg_infrasys_apsrc_req_mask_b = 1,
	.reg_infrasys_ddr_en_mask_b = 1,
	.reg_infrasys_ddr_en2_mask_b = 0,
	.reg_md32_srcclkena_mask_b = 1,
	.reg_conn_vfe28_req_mask_b = 0,

	/* SPM_SRC2_MASK */
	.reg_md32_infra_req_mask_b = 1,
	.reg_md32_apsrc_req_mask_b = 1,
	.reg_md32_vrf18_req_mask_b = 1,
	.reg_md32_ddr_en_mask_b = 1,
	.reg_md32_ddr_en2_mask_b = 0,
	.reg_scp_srcclkena_mask_b = 1,
	.reg_scp_infra_req_mask_b = 1,
	.reg_scp_apsrc_req_mask_b = 1,
	.reg_scp_vrf18_req_mask_b = 1,
	.reg_scp_ddr_en_mask_b = 1,
	.reg_scp_ddr_en2_mask_b = 0,
	.reg_ufs_srcclkena_mask_b = 1,
	.reg_ufs_infra_req_mask_b = 1,
	.reg_ufs_apsrc_req_mask_b = 1,
	.reg_ufs_vrf18_req_mask_b = 1,
	.reg_ufs_ddr_en_mask_b = 1,
	.reg_ufs_ddr_en2_mask_b = 0,
	.reg_disp0_apsrc_req_mask_b = 1,
	.reg_disp0_ddr_en_mask_b = 1,
	.reg_disp0_ddr_en2_mask_b = 0,
	.reg_disp1_apsrc_req_mask_b = 1,
	.reg_disp1_ddr_en_mask_b = 1,
	.reg_disp1_ddr_en2_mask_b = 0,
	.reg_gce_infra_req_mask_b = 1,
	.reg_gce_apsrc_req_mask_b = 1,
	.reg_gce_vrf18_req_mask_b = 1,
	.reg_gce_ddr_en_mask_b = 1,
	.reg_gce_ddr_en2_mask_b = 0,
	.reg_emi_ch0_ddr_en_mask_b = 0,
	.reg_emi_ch1_ddr_en_mask_b = 0,
	.reg_emi_ch0_ddr_en2_mask_b = 0,
	.reg_emi_ch1_ddr_en2_mask_b = 0,

	/* SPM_SRC3_MASK */
	.reg_dvfsrc_event_trigger_mask_b = 1,
	.reg_sw2spm_int0_mask_b = 0,
	.reg_sw2spm_int1_mask_b = 0,
	.reg_sw2spm_int2_mask_b = 0,
	.reg_sw2spm_int3_mask_b = 0,
	.reg_sc_adsp2spm_wakeup_mask_b = 0,
	.reg_sc_sspm2spm_wakeup_mask_b = 0,
	.reg_sc_scp2spm_wakeup_mask_b = 0,
	.reg_csyspwrreq_mask = 1,
	.reg_spm_srcclkena_reserved_mask_b = 0,
	.reg_spm_infra_req_reserved_mask_b = 0,
	.reg_spm_apsrc_req_reserved_mask_b = 0,
	.reg_spm_vrf18_req_reserved_mask_b = 0,
	.reg_spm_ddr_en_reserved_mask_b = 0,
	.reg_spm_ddr_en2_reserved_mask_b = 0,
	.reg_audio_dsp_srcclkena_mask_b = 1,
	.reg_audio_dsp_infra_req_mask_b = 1,
	.reg_audio_dsp_apsrc_req_mask_b = 1,
	.reg_audio_dsp_vrf18_req_mask_b = 1,
	.reg_audio_dsp_ddr_en_mask_b = 1,
	.reg_audio_dsp_ddr_en2_mask_b = 0,
	.reg_mcusys_pwr_event_mask_b = 0,
	.reg_msdc0_srcclkena_mask_b = 0,
	.reg_msdc0_infra_req_mask_b = 0,
	.reg_msdc0_apsrc_req_mask_b = 0,
	.reg_msdc0_vrf18_req_mask_b = 0,
	.reg_msdc0_ddr_en_mask_b = 0,
	.reg_msdc0_ddr_en2_mask_b = 0,
	.reg_conn_srcclkenb2pwrap_mask_b = 0,

	/* SPM_SRC4_MASK */
	.ccif_event_mask_b = 0xFFFF,
	.reg_apu_core0_srcclkena_mask_b = 0,
	.reg_apu_core0_infra_req_mask_b = 0,
	.reg_apu_core0_apsrc_req_mask_b = 0,
	.reg_apu_core0_vrf18_req_mask_b = 0,
	.reg_apu_core0_ddr_en_mask_b = 0,
	.reg_apu_core1_srcclkena_mask_b = 0,
	.reg_apu_core1_infra_req_mask_b = 0,
	.reg_apu_core1_apsrc_req_mask_b = 0,
	.reg_apu_core1_vrf18_req_mask_b = 0,
	.reg_apu_core1_ddr_en_mask_b = 0,
	.reg_apu_core2_srcclkena_mask_b = 0,
	.reg_apu_core2_infra_req_mask_b = 0,
	.reg_apu_core2_apsrc_req_mask_b = 0,
	.reg_apu_core2_vrf18_req_mask_b = 0,
	.reg_apu_core2_ddr_en_mask_b = 0,
	.reg_apu_core2_ddr_en2_mask_b = 0,

	/* SPM_SRC5_MASK */
	.reg_mcusys_merge_apsrc_req_mask_b = 0,
	.reg_mcusys_merge_ddr_en_mask_b = 0,
	.reg_mcusys_merge_ddr_en2_mask_b = 0,
	.reg_apu_core0_ddr_en2_mask_b = 0,
	.reg_apu_core1_ddr_en2_mask_b = 0,
	.reg_cg_check_ddr_en_mask_b = 0,
	.reg_cg_check_ddr_en2_mask_b = 0,

	/* SPM_WAKEUP_EVENT_MASK */
	.reg_wakeup_event_mask = 0xE1282202,

	/* SPM_WAKEUP_EVENT_EXT_MASK */
	.reg_ext_wakeup_event_mask = 0xFFFFFFFF,

	/* SPM_SRC6_MASK */
	.reg_msdc1_srcclkena_mask_b = 0,
	.reg_msdc1_infra_req_mask_b = 0,
	.reg_msdc1_apsrc_req_mask_b = 0,
	.reg_msdc1_vrf18_req_mask_b = 0,
	.reg_msdc1_ddr_en_mask_b = 0,
	.reg_msdc1_ddr_en2_mask_b = 0,
	.reg_msdc1_srcclkena_ack_mask = 0,
	.reg_msdc1_infra_ack_mask = 0,
	.reg_msdc1_apsrc_ack_mask = 0,
	.reg_msdc1_vrf18_ack_mask = 0,
	.reg_msdc1_ddr_en_ack_mask = 0,
	.reg_msdc1_ddr_en2_ack_mask = 0,

#if MCUSYS_MTCMOS_ON
	/* MP0_CPU0_WFI_EN */
	.mp0_cpu0_wfi_en = 0,

	/* MP0_CPU1_WFI_EN */
	.mp0_cpu1_wfi_en = 0,

	/* MP0_CPU2_WFI_EN */
	.mp0_cpu2_wfi_en = 0,

	/* MP0_CPU3_WFI_EN */
	.mp0_cpu3_wfi_en = 0,

	/* MP0_CPU4_WFI_EN */
	.mp0_cpu4_wfi_en = 0,

	/* MP0_CPU5_WFI_EN */
	.mp0_cpu5_wfi_en = 0,

	/* MP0_CPU6_WFI_EN */
	.mp0_cpu6_wfi_en = 0,

	/* MP0_CPU7_WFI_EN */
	.mp0_cpu7_wfi_en = 0,
#endif
	/* Auto-gen End */
};

struct spm_lp_scen __spm_dpidle = {
	.pwrctrl = &dpidle_ctrl,
};


void spm_dpidle_storage_mask_set(void)
{
	struct pwr_ctrl *pwrctrl;

	pwrctrl = __spm_dpidle.pwrctrl;

	pwrctrl->reg_ufs_srcclkena_mask_b = 0;
	pwrctrl->reg_ufs_infra_req_mask_b = 0;
	pwrctrl->reg_ufs_apsrc_req_mask_b = 0;
	pwrctrl->reg_ufs_vrf18_req_mask_b = 0;
	pwrctrl->reg_ufs_ddr_en_mask_b = 0;
	pwrctrl->reg_ufs_ddr_en2_mask_b = 0;
}

void spm_dpidle_args(__uint64_t x1, __uint64_t x2, __uint64_t x3)
{
	struct pwr_ctrl *pwrctrl;

	pwrctrl = __spm_dpidle.pwrctrl;
	pwrctrl->pcm_flags = x1;
	pwrctrl->pcm_flags1 = x2;

	/* get spm resource request from kernel */
	resource_usage = x3;
}

void spm_sleep_dpidle_args(__uint64_t x1, __uint64_t x2, __uint64_t x3)
{
	struct pwr_ctrl *pwrctrl = __spm_dpidle.pwrctrl;

	is_sleep_dpidle = true;

	/* backup original dpidle setting */
	dpidle_pcm_timer_val = pwrctrl->timer_val;
	dpidle_wake_src = pwrctrl->wake_src;

	pwrctrl->timer_val = x1;
	pwrctrl->wake_src = x2;
}

void go_to_dpidle_before_wfi_no_resume(void)
{
	struct pwr_ctrl *pwrctrl;
	uint64_t mpidr = read_mpidr();
	__uint32_t cpu = platform_get_core_pos(mpidr);

	pwrctrl = __spm_dpidle.pwrctrl;

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

	__spm_src_req_update(pwrctrl, resource_usage);

	if (is_sleep_dpidle) {
		if (!pwrctrl->wdt_disable)
			__spm_set_pcm_wdt(1);
	}

	__spm_send_cpu_wakeup_event();
}

void go_to_dpidle_before_wfi(void)
{
	struct pcm_desc *pcmdesc = NULL;
	struct pwr_ctrl *pwrctrl;
	uint64_t mpidr = read_mpidr();
	__uint32_t cpu = platform_get_core_pos(mpidr);
	int spmfw_idx = __spm_get_spmfw_idx();

	if (dyna_load_pcm[spmfw_idx].ready) {
		pcmdesc = &(dyna_load_pcm[spmfw_idx].desc);
	} else {
		INFO("firmware is not ready!!!\n");
		return;
	}
	pwrctrl = __spm_dpidle.pwrctrl;

#if WAKEUP_LOG_ON
	INFO("Online CPU is %d, suspend FW ver. is %s\n",
			cpu, pcmdesc->version);

	INFO("sec = %u, wakesrc = 0x%x (%u)(%u)\n",
			pwrctrl->timer_val, pwrctrl->wake_src,
			is_cpu_pdn(pwrctrl->pcm_flags),
			is_infra_pdn(pwrctrl->pcm_flags));
#endif

	__spm_set_fw_resume_option(pwrctrl);
	__spm_set_cpu_status(cpu);
	__spm_reset_and_init_pcm(pcmdesc);
	__spm_kick_im_to_fetch(pcmdesc);
	__spm_init_pcm_register();

	__spm_sync_vcore_dvfs_power_control(pwrctrl, __spm_vcorefs.pwrctrl);

	__spm_set_power_control(pwrctrl);
	__spm_src_req_update(pwrctrl, resource_usage);
	__spm_set_wakeup_event(pwrctrl);

	if (is_sleep_dpidle) {
		if (!pwrctrl->wdt_disable)
			__spm_set_pcm_wdt(1);
	}

	__spm_kick_pcm_to_run(pwrctrl);
}

static void go_to_dpidle_after_wfi(void)
{
	struct pcm_desc *pcmdesc = NULL;
	struct pwr_ctrl *pwrctrl;
	int spmfw_idx = __spm_get_spmfw_idx();

	if (dyna_load_pcm[spmfw_idx].ready) {
		pcmdesc = &(dyna_load_pcm[spmfw_idx].desc);
	} else {
		INFO("firmware is not ready!!!\n");
		return;
	}
	__spm_ext_int_wakeup_req_clr();

	pwrctrl = __spm_dpidle.pwrctrl;

	if (is_sleep_dpidle) {
		is_sleep_dpidle = false;

		if (!pwrctrl->wdt_disable)
			__spm_set_pcm_wdt(0);

		/* restore original dpidle setting */
		pwrctrl->timer_val = dpidle_pcm_timer_val;
		pwrctrl->wake_src = dpidle_wake_src;
	}

	__spm_get_wakeup_status(&spm_wakesta);
	__spm_clean_after_wakeup();
	spm_wake_reason = __spm_output_wake_reason(&spm_wakesta, pcmdesc);
#if WAKEUP_LOG_ON
	INFO("spm_wake_reason=%d\n", spm_wake_reason);
#endif
}

void spm_dpidle(void)
{
	spm_lock_get();

	go_to_dpidle_before_wfi_no_resume();

	spm_lock_release();
}

void spm_dpidle_finish(void)
{
	spm_lock_get();
	go_to_dpidle_after_wfi();
	spm_lock_release();
}
