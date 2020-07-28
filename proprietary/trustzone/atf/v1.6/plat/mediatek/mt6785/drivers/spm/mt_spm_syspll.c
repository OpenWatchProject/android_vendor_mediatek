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

#include <mt_spm_conservation.h>

static unsigned int syspll_resource_usage;

#define __WAKE_SRC_FOR_SYSPLL_COMMON__ (\
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
#define WAKE_SRC_FOR_SYSPLL	\
	(__WAKE_SRC_FOR_SYSPLL_COMMON__)
#else
#define WAKE_SRC_FOR_SYSPLL \
	(__WAKE_SRC_FOR_SYSPLL_COMMON__ \
		| R12_SEJ_EVENT_B)
#endif

static struct pwr_ctrl syspll_ctrl = {
	.wake_src = WAKE_SRC_FOR_SYSPLL,

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

struct spm_lp_scen __spm_syspll = {
	.pwrctrl = &syspll_ctrl,
};


void spm_syspll_storage_mask_set(void)
{
	struct pwr_ctrl *pwrctrl;

	pwrctrl = __spm_syspll.pwrctrl;

	pwrctrl->reg_ufs_srcclkena_mask_b = 0;
	pwrctrl->reg_ufs_infra_req_mask_b = 0;
	pwrctrl->reg_ufs_apsrc_req_mask_b = 0;
	pwrctrl->reg_ufs_vrf18_req_mask_b = 0;
	pwrctrl->reg_ufs_ddr_en_mask_b = 0;
	pwrctrl->reg_ufs_ddr_en2_mask_b = 0;
}

void spm_syspll_args(uint64_t arg1, uint64_t arg2, uint64_t arg3)
{
	struct pwr_ctrl *pwrctrl;

	pwrctrl = __spm_syspll.pwrctrl;
	pwrctrl->pcm_flags = arg1;
	pwrctrl->pcm_flags1 = arg2;

	/* get spm resource request from kernel */
	syspll_resource_usage = arg3;
}

void spm_syspll(int StateId)
{
	spm_conservation(StateId, 0
		, &__spm_syspll, syspll_resource_usage);
}

void spm_syspll_finish(int StateId)
{
	spm_conservation_finish(StateId, 0
		, &__spm_syspll);
}
