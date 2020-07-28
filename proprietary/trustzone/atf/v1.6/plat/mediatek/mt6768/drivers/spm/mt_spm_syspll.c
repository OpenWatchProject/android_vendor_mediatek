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


/* for internal debug */
#define SPM_BYPASS_SYSPWREQ     1

static unsigned int syspll_resource_usage;

#define __WAKE_SRC_FOR_SYSPLL_COMMON__ (\
	(WAKE_SRC_R12_PCM_TIMER) | \
	(WAKE_SRC_R12_SSPM_WDT_EVENT_B) | \
	(WAKE_SRC_R12_KP_IRQ_B) | \
	(WAKE_SRC_R12_APWDT_EVENT_B) | \
	(WAKE_SRC_R12_APXGPT1_EVENT_B) | \
	(WAKE_SRC_R12_CONN2AP_SPM_WAKEUP_B) | \
	(WAKE_SRC_R12_EINT_EVENT_B) | \
	(WAKE_SRC_R12_CONN_WDT_IRQ_B) | \
	(WAKE_SRC_R12_CCIF0_EVENT_B) | \
	(WAKE_SRC_R12_SSPM_SPM_IRQ_B) | \
	(WAKE_SRC_R12_SCP_SPM_IRQ_B) | \
	(WAKE_SRC_R12_SCP_WDT_EVENT_B) | \
	(WAKE_SRC_R12_USB_CDSC_B) | \
	(WAKE_SRC_R12_USB_POWERDWN_B) | \
	(WAKE_SRC_R12_SYS_TIMER_EVENT_B) | \
	(WAKE_SRC_R12_EINT_EVENT_SECURE_B) | \
	(WAKE_SRC_R12_CCIF1_EVENT_B) | \
	(WAKE_SRC_R12_AFE_IRQ_MCU_B) | \
	(WAKE_SRC_R12_MD2AP_PEER_EVENT_B) | \
	(WAKE_SRC_R12_MD1_WDT_B) | \
	(WAKE_SRC_R12_CLDMA_EVENT_B))

#if defined(CFG_MICROTRUST_TEE_SUPPORT)
#define WAKE_SRC_FOR_SYSPLL	\
	(__WAKE_SRC_FOR_SYSPLL_COMMON__)
#else
#define WAKE_SRC_FOR_SYSPLL	\
	(__WAKE_SRC_FOR_SYSPLL_COMMON__\
		| WAKE_SRC_R12_SEJ_WDT_GPT_B)
#endif


static struct pwr_ctrl syspll_ctrl = {
	.wake_src = WAKE_SRC_FOR_SYSPLL,

	/* Auto-gen Start */

	/* SPM_AP_STANDBY_CON */
#if MCUSYS_MTCMOS_ON
	/* non-CPC mode */
	.wfi_op = WFI_OP_AND,
	.wfi_type = MCUSYS_IDLE_AS_WFI,
	.mcusys_idle_mask = 0x1,
#else
	.wfi_op = WFI_OP_OR,
	.wfi_type = MCUSYS_PWRSTATUS_AS_WFI,
	.mcusys_idle_mask = 0,
#endif
	.mm_mask_b = 0,
	.md_ddr_en_0_dbc_en = 0x1,
	.md_ddr_en_1_dbc_en = 0,
	.md_mask_b = 0x1,
	.sspm_mask_b = 0x1,
	.scp_mask_b = 0x1,
	.srcclkeni_mask_b = 0x1,
	.md_apsrc_1_sel = 0,
	.md_apsrc_0_sel = 0,
	.conn_ddr_en_dbc_en = 0x1,
	.conn_mask_b = 0x1,
	.conn_apsrc_sel = 0,
	.conn_srcclkena_sel_mask = 0x1,

	/* SPM_SRC_REQ */
	.spm_apsrc_req = 0,
	.spm_f26m_req = 0,
	.spm_infra_req = 0,
	.spm_vrf18_req = 0,
	.spm_ddren_req = 0,
	.spm_rsv_src_req = 0,
	.spm_ddren_2_req = 0,
	.cpu_md_dvfs_sop_force_on = 0,

	/* SPM_SRC_MASK */
#if SPM_BYPASS_SYSPWREQ
	.csyspwreq_mask = 0x1,
#endif
	.ccif0_md_event_mask_b = 0x1,
	.ccif0_ap_event_mask_b = 0x1,
	.ccif1_md_event_mask_b = 0x1,
	.ccif1_ap_event_mask_b = 0x1,
	.ccif2_md_event_mask_b = 0x1,
	.ccif2_ap_event_mask_b = 0x1,
	.ccif3_md_event_mask_b = 0x1,
	.ccif3_ap_event_mask_b = 0x1,
	.md_srcclkena_0_infra_mask_b = 0x1,
	.md_srcclkena_1_infra_mask_b = 0,
	.conn_srcclkena_infra_mask_b = 0,
	.ufs_infra_req_mask_b = 0x1,
	.srcclkeni_infra_mask_b = 0,
	.md_apsrc_req_0_infra_mask_b = 0x1,
	.md_apsrc_req_1_infra_mask_b = 0x1,
	.conn_apsrcreq_infra_mask_b = 0x1,
	.ufs_srcclkena_mask_b = 0x1,
	.md_vrf18_req_0_mask_b = 0,
	.md_vrf18_req_1_mask_b = 0,
	.ufs_vrf18_req_mask_b = 0x1,
	.gce_vrf18_req_mask_b = 0x1,
	.conn_infra_req_mask_b = 0x1,
	.gce_apsrc_req_mask_b = 0x1,
	.disp0_apsrc_req_mask_b = 0x1,
	.disp1_apsrc_req_mask_b = 0x1,
	.mfg_req_mask_b = 0,
	.vdec_req_mask_b = 0,
	.mcu_apsrcreq_infra_mask_b = 0,

	/* SPM_SRC2_MASK */
	.md_ddr_en_0_mask_b = 0x1,
	.md_ddr_en_1_mask_b = 0,
	.conn_ddr_en_mask_b = 0x1,
	.ddren_md32_apsrc_req_mask_b = 0x1,
	.ddren_scp_apsrc_req_mask_b = 0x1,
	.disp0_ddren_mask_b = 0x1,
	.disp1_ddren_mask_b = 0x1,
	.gce_ddren_mask_b = 0x1,
	.ddren_emi_self_refresh_ch0_mask_b = 0,
	.ddren_emi_self_refresh_ch1_mask_b = 0,
	.mcu_apsrc_req_mask_b = 0,
	.mcu_ddren_mask_b = 0,

	/* SPM_WAKEUP_EVENT_MASK */
	.spm_wakeup_event_mask = 0xF1682200,

	/* SPM_WAKEUP_EVENT_EXT_MASK */
	.spm_wakeup_event_ext_mask = 0xFFFFFFFF,

	/* SPM_SRC3_MASK */
	.md_ddr_en_2_0_mask_b = 0x1,
	.md_ddr_en_2_1_mask_b = 0,
	.conn_ddr_en_2_mask_b = 0x1,
	.ddren2_md32_apsrc_req_mask_b = 0x1,
	.ddren2_scp_apsrc_req_mask_b = 0x1,
	.disp0_ddren2_mask_b = 0,
	.disp1_ddren2_mask_b = 0,
	.gce_ddren2_mask_b = 0,
	.ddren2_emi_self_refresh_ch0_mask_b = 0,
	.ddren2_emi_self_refresh_ch1_mask_b = 0,
	.mcu_ddren_2_mask_b = 0,

#if MCUSYS_MTCMOS_ON
	/* MP0_CPU0_WFI_EN */
	.mp0_cpu0_wfi_en = 1,

	/* MP0_CPU1_WFI_EN */
	.mp0_cpu1_wfi_en = 1,

	/* MP0_CPU2_WFI_EN */
	.mp0_cpu2_wfi_en = 1,

	/* MP0_CPU3_WFI_EN */
	.mp0_cpu3_wfi_en = 1,

	/* MP0_CPU4_WFI_EN */
	.mp0_cpu4_wfi_en = 1,

	/* MP0_CPU5_WFI_EN */
	.mp0_cpu5_wfi_en = 1,

	/* MP0_CPU6_WFI_EN */
	.mp0_cpu6_wfi_en = 1,

	/* MP0_CPU7_WFI_EN */
	.mp0_cpu7_wfi_en = 1,
#endif
	/* Auto-gen End */
};

struct spm_lp_scen __spm_syspll = {
	.pwrctrl = &syspll_ctrl,
};

void spm_syspll_args(uint64_t arg1, uint64_t arg2, uint64_t arg3)
{
	struct pwr_ctrl *pwrctrl;

	pwrctrl = __spm_syspll.pwrctrl;
	pwrctrl->pcm_flags = (uint32_t)arg1;
	pwrctrl->pcm_flags1 = (uint32_t)arg2;

	/* get spm resource request from kernel */
	syspll_resource_usage = (uint32_t)arg3;
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

