#include <arch_helpers.h>
#include <debug.h>
#include <mmio.h>
#include <mt_spm.h>
#include <mt_spm_internal.h>
#include <mt_spm_pmic_wrap.h>
#include <mt_spm_reg.h>
#include <mt_spm_vcorefs.h>
#include <plat_pm.h>
#include <platform.h>
#include <platform_def.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>


/* for internal debug */
#define SPM_BYPASS_SYSPWREQ     1

static struct wake_status spm_wakesta; /* record last wakesta */
static unsigned int spm_wake_reason = WR_NONE;

static unsigned int resource_usage;

#define __WAKE_SRC_FOR_SODI_COMMON__ (\
	WAKE_SRC_R12_PCM_TIMER | \
	WAKE_SRC_R12_SSPM_WDT_EVENT_B | \
	WAKE_SRC_R12_KP_IRQ_B | \
	WAKE_SRC_R12_APXGPT1_EVENT_B | \
	WAKE_SRC_R12_CONN2AP_SPM_WAKEUP_B | \
	WAKE_SRC_R12_EINT_EVENT_B | \
	WAKE_SRC_R12_CCIF0_EVENT_B | \
	WAKE_SRC_R12_SSPM_SPM_IRQ_B | \
	WAKE_SRC_R12_USB_CDSC_B | \
	WAKE_SRC_R12_USB_POWERDWN_B | \
	WAKE_SRC_R12_SYS_TIMER_EVENT_B | \
	WAKE_SRC_R12_EINT_EVENT_SECURE_B | \
	WAKE_SRC_R12_CCIF1_EVENT_B | \
	WAKE_SRC_R12_AFE_IRQ_MCU_B | \
	WAKE_SRC_R12_MD2AP_PEER_EVENT_B | \
	WAKE_SRC_R12_MD1_WDT_B | \
	WAKE_SRC_R12_CLDMA_EVENT_B)

#if defined(CFG_MICROTRUST_TEE_SUPPORT)
#define WAKE_SRC_FOR_SODI	\
	(__WAKE_SRC_FOR_SODI_COMMON__)
#else
#define WAKE_SRC_FOR_SODI	\
	(__WAKE_SRC_FOR_SODI_COMMON__ | WAKE_SRC_R12_SEJ_WDT_GPT_B)
#endif


static struct pwr_ctrl sodi_ctrl = {
	.wake_src = WAKE_SRC_FOR_SODI,

	/* Auto-gen Start */

	/* SPM_AP_STANDBY_CON */
	.wfi_op = 1,
	.mp0_cputop_idle_mask = 0,
	.mcusys_idle_mask = 1,
	.mcu_ddren_req_dbc_en = 0,
	.mcu_apsrc_sel = 0,
	.mm_mask_b = 0,
	.md_ddr_en_0_dbc_en = 1,
	.md_ddr_en_1_dbc_en = 0,
	.md_mask_b = 0x1,
	.lte_mask_b = 0,
	.srcclkeni_mask_b = 1,
	.md_apsrc_1_sel = 0,
	.md_apsrc_0_sel = 0,
	.conn_ddr_en_dbc_en = 1,
	.conn_mask_b = 1,
	.conn_apsrc_sel = 0,
	.conn_srcclkena_sel_mask = 0x1,

	/* SPM_SRC_REQ */
	.spm_apsrc_req = 0,
	.spm_f26m_req = 0,
	.spm_lte_req = 0,
	.spm_infra_req = 0,
	.spm_vrf18_req = 0,
	.spm_dvfs_req = 0,
	.spm_dvfs_force_down = 0,
	.spm_ddren_req = 0,
	.spm_rsv_src_req = 0,
	.spm_ddren_2_req = 0,
	.cpu_md_dvfs_sop_force_on = 0,

	/* SPM_SRC_MASK */
#if SPM_BYPASS_SYSPWREQ
	.csyspwreq_mask = 0x1,
#endif
	.ccif0_md_event_mask_b = 1,
	.ccif0_ap_event_mask_b = 1,
	.ccif1_md_event_mask_b = 1,
	.ccif1_ap_event_mask_b = 1,
	.ccifmd_md1_event_mask_b = 0,
	.ccifmd_md2_event_mask_b = 0,
	.dsi0_vsync_mask_b = 0,
	.dsi1_vsync_mask_b = 0,
	.dpi_vsync_mask_b = 0,
	.isp0_vsync_mask_b = 0,
	.isp1_vsync_mask_b = 0,
	.md_srcclkena_0_infra_mask_b = 1,
	.md_srcclkena_1_infra_mask_b = 0,
	.conn_srcclkena_infra_mask_b = 1,
	.sspm_srcclkena_infra_mask_b = 0,
	.srcclkeni_infra_mask_b = 0,
	.md_apsrc_req_0_infra_mask_b = 0,
	.md_apsrc_req_1_infra_mask_b = 0,
	.conn_apsrcreq_infra_mask_b = 0,
	.mcu_apsrcreq_infra_mask_b = 0,
	.md_ddr_en_0_mask_b = 1,
	.md_ddr_en_1_mask_b = 0,
	.md_vrf18_req_0_mask_b = 0,
	.md_vrf18_req_1_mask_b = 0,
	.md1_dvfs_req_mask = 0x3,
	.cpu_dvfs_req_mask = 1,
	.emi_bw_dvfs_req_mask = 1,
	.md_srcclkena_0_dvfs_req_mask_b = 0,
	.md_srcclkena_1_dvfs_req_mask_b = 0,
	.conn_srcclkena_dvfs_req_mask_b = 0,

	/* SPM_SRC2_MASK */
	.dvfs_halt_mask_b = 0,
	.vdec_req_mask_b = 0,
	.gce_req_mask_b = 1,
	.cpu_md_dvfs_req_merge_mask_b = 0,
	.md_ddr_en_dvfs_halt_mask_b = 0,
	.dsi0_vsync_dvfs_halt_mask_b = 0,
	.dsi1_vsync_dvfs_halt_mask_b = 0,
	.dpi_vsync_dvfs_halt_mask_b = 0,
	.isp0_vsync_dvfs_halt_mask_b = 0,
	.isp1_vsync_dvfs_halt_mask_b = 0,
	.conn_ddr_en_mask_b = 1,
	.disp_req_mask_b = 1,
	.disp1_req_mask_b = 1,
	.mfg_req_mask_b = 0,
	.mcu_ddren_req_mask_b = 0,
	.mcu_apsrc_req_mask_b = 0,
	.ps_c2k_rccif_wake_mask_b = 0,
	.l1_c2k_rccif_wake_mask_b = 0,
	.sdio_on_dvfs_req_mask_b = 0,
	.emi_boost_dvfs_req_mask_b = 0,
	.cpu_md_emi_dvfs_req_prot_dis = 0,
	.dramc_spcmd_apsrc_req_mask_b = 0,
	.emi_boost_dvfs_req_2_mask_b = 0,
	.emi_bw_dvfs_req_2_mask = 0,
	.gce_vrf18_req_mask_b = 0,

	/* SPM_WAKEUP_EVENT_MASK */
	.spm_wakeup_event_mask = 0xF1683A08,

	/* SPM_WAKEUP_EVENT_EXT_MASK */
	.spm_wakeup_event_ext_mask = 0xFFFFFFFF,

	/* SPM_SRC3_MASK */
	.md_ddr_en_2_0_mask_b = 0,
	.md_ddr_en_2_1_mask_b = 0,
	.conn_ddr_en_2_mask_b = 0,
	.dramc_spcmd_apsrc_req_2_mask_b = 0,
	.spare1_ddren_2_mask_b = 0,
	.spare2_ddren_2_mask_b = 0,
	.ddren_emi_self_refresh_ch0_mask_b = 0,
	.ddren_emi_self_refresh_ch1_mask_b = 0,
	.ddren_mm_state_mask_b = 1,
	.ddren_sspm_apsrc_req_mask_b = 0,
	.ddren_dqssoc_req_mask_b = 0,
	.ddren2_emi_self_refresh_ch0_mask_b = 0,
	.ddren2_emi_self_refresh_ch1_mask_b = 0,
	.ddren2_mm_state_mask_b = 0,
	.ddren2_sspm_apsrc_req_mask_b = 0,
	.ddren2_dqssoc_req_mask_b = 0,

	/* MP0_CPU0_WFI_EN */
	.mp0_cpu0_wfi_en = 1,

	/* MP0_CPU1_WFI_EN */
	.mp0_cpu1_wfi_en = 1,

	/* MP0_CPU2_WFI_EN */
	.mp0_cpu2_wfi_en = 1,

	/* MP0_CPU3_WFI_EN */
	.mp0_cpu3_wfi_en = 1,

	/* Auto-gen End */
};

struct spm_lp_scen __spm_sodi = {
	.pwrctrl = &sodi_ctrl,
};

void spm_sodi_args(uint64_t x1, uint64_t x2, uint64_t x3)
{
	struct pwr_ctrl *pwrctrl;

	pwrctrl = __spm_sodi.pwrctrl;
	pwrctrl->pcm_flags = x1;

	/* get spm resource request from kernel */
	resource_usage = x2;
	pwrctrl->timer_val = x3;
}

static void go_to_sodi_before_wfi(void)
{
	struct pcm_desc *pcmdesc = NULL;
	struct pwr_ctrl *pwrctrl;
	uint64_t mpidr = read_mpidr();
	uint32_t cpu = platform_get_core_pos(mpidr);

	if (dyna_load_pcm[DYNA_LOAD_PCM_SODI].ready) {
		pcmdesc = &(dyna_load_pcm[DYNA_LOAD_PCM_SODI].desc);
	} else {
		ERROR("SODI: spm fw not ready\n");
		return;
	}
	pwrctrl = __spm_sodi.pwrctrl;

	mt_spm_pmic_wrap_set_phase(PMIC_WRAP_PHASE_ALLINONE);

#if 0
	INFO("Online CPU is %d, suspend FW ver. is %s\n",
			cpu, pcmdesc->version);

	INFO("sec = %u, wakesrc = 0x%x (%u)(%u)\n",
			pwrctrl->timer_val, pwrctrl->wake_src,
			is_cpu_pdn(pwrctrl->pcm_flags),
			is_infra_pdn(pwrctrl->pcm_flags));
#endif

	__spm_set_cpu_status(cpu);
	if (__spm_get_md_srcclkena_setting() == 0)
		spm_request_dvfs_opp(1, 0xB);
	__spm_reset_and_init_pcm(pcmdesc);
	__spm_kick_im_to_fetch(pcmdesc);
	__spm_init_pcm_register();
	__spm_init_event_vector(pcmdesc);
	__spm_sync_vcore_dvfs_power_control(pwrctrl, __spm_vcorefs.pwrctrl);
	__spm_set_power_control(pwrctrl);
	__spm_src_req_update(pwrctrl, resource_usage);
	__spm_set_wakeup_event(pwrctrl);

	if (pwrctrl->pcm_flags & SPM_FLAG_ENABLE_SODI3) {
		if (!pwrctrl->wdt_disable)
			__spm_set_pcm_wdt(1);
	}

	__spm_kick_pcm_to_run(pwrctrl);
}

static void go_to_sodi_after_wfi(void)
{
	struct pcm_desc *pcmdesc = NULL;
	struct pwr_ctrl *pwrctrl;

	if (dyna_load_pcm[DYNA_LOAD_PCM_SODI].ready) {
		pcmdesc = &(dyna_load_pcm[DYNA_LOAD_PCM_SODI].desc);
	} else {
		ERROR("SODI: spm fw not ready\n");
		return;
	}

	pwrctrl = __spm_sodi.pwrctrl;

	if (pwrctrl->pcm_flags & SPM_FLAG_ENABLE_SODI3) {
		if (!pwrctrl->wdt_disable)
			__spm_set_pcm_wdt(0);
	}

	if (__spm_get_md_srcclkena_setting() == 0)
		spm_request_dvfs_opp(1, 0x0);
	__spm_get_wakeup_status(&spm_wakesta);
	__spm_clean_after_wakeup();
	spm_wake_reason = __spm_output_wake_reason(&spm_wakesta, pcmdesc);
#if 0
	INFO("spm_wake_reason=%d\n", spm_wake_reason);
#endif
}

void spm_sodi(void)
{
	spm_lock_get();
	go_to_sodi_before_wfi();
	spm_lock_release();
}

void spm_sodi_finish(void)
{
	spm_lock_get();
	go_to_sodi_after_wfi();
	spm_lock_release();
}
