#include <arch_helpers.h>
#include <debug.h>
#include <delay_timer.h>
#include <mmio.h>
#include <mt_spm.h>
#include <mt_spm_internal.h>
#include <mt_spm_reg.h>
#include <mt_spm_vcorefs.h>
#include <mt_spm_pmic_wrap.h>
#include <plat_pm.h>
#include <platform.h>
#include <platform_def.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define DVFSRC_SEC_SW_REQ    (DVFSRC_BASE + 0x304)

static struct pwr_ctrl vcorefs_ctrl = {
	.wake_src		= R12_PCM_TIMER,

	/* default VCORE DVFS is disabled */
	.pcm_flags = (SPM_FLAG_RUN_COMMON_SCENARIO |
			SPM_FLAG_DIS_VCORE_DVS | SPM_FLAG_DIS_VCORE_DFS | SPM_FLAG_DISABLE_MMSYS_DVFS),

	/* Auto-gen Start */

	/* SPM_AP_STANDBY_CON */
	.wfi_op = WFI_OP_AND,
	.mp0_cputop_idle_mask = 0,
	.mp1_cputop_idle_mask = 0,
	.mcusys_idle_mask = 0,
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

	/* SPM_SRC_REQ */
	.spm_apsrc_req = 0,
	.spm_f26m_req = 0,
	.spm_infra_req = 0,
	.spm_vrf18_req = 0,
	.spm_ddren_req = 1,
	.spm_rsv_src_req = 0,
	.spm_ddren_2_req = 0,
	.cpu_md_dvfs_sop_force_on = 0,

	/* SPM_SRC_MASK */
	.csyspwreq_mask = 1,
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
	.ufs_infra_req_mask_b = 0,
	.srcclkeni_infra_mask_b = 0,
	.md_apsrc_req_0_infra_mask_b = 0x1,
	.md_apsrc_req_1_infra_mask_b = 0x1,
	.conn_apsrcreq_infra_mask_b = 0x1,
	.ufs_srcclkena_mask_b = 0,
	.md_vrf18_req_0_mask_b = 0,
	.md_vrf18_req_1_mask_b = 0,
	.ufs_vrf18_req_mask_b = 0,
	.gce_vrf18_req_mask_b = 0,
	.conn_infra_req_mask_b = 0x1,
	.gce_apsrc_req_mask_b = 0,
	.disp0_apsrc_req_mask_b = 0,
	.disp1_apsrc_req_mask_b = 0,
	.mfg_req_mask_b = 0,
	.vdec_req_mask_b = 0,

	/* SPM_SRC2_MASK */
	.md_ddr_en_0_mask_b = 0x1,
	.md_ddr_en_1_mask_b = 0,
	.conn_ddr_en_mask_b = 0x1,
	.ddren_sspm_apsrc_req_mask_b = 0x1,
	.ddren_scp_apsrc_req_mask_b = 0x1,
	.disp0_ddren_mask_b = 0x1,
	.disp1_ddren_mask_b = 0x1,
	.gce_ddren_mask_b = 0x1,
	.ddren_emi_self_refresh_ch0_mask_b = 0,
	.ddren_emi_self_refresh_ch1_mask_b = 0,

	/* SPM_WAKEUP_EVENT_MASK */
	.spm_wakeup_event_mask = 0xFFFFFFFE,

	/* SPM_WAKEUP_EVENT_EXT_MASK */
	.spm_wakeup_event_ext_mask = 0xFFFFFFFF,

	/* SPM_SRC3_MASK */
	.md_ddr_en_2_0_mask_b = 0x1,
	.md_ddr_en_2_1_mask_b = 0,
	.conn_ddr_en_2_mask_b = 0x1,
	.ddren2_sspm_apsrc_req_mask_b = 0x1,
	.ddren2_scp_apsrc_req_mask_b = 0x1,
	.disp0_ddren2_mask_b = 0,
	.disp1_ddren2_mask_b = 0,
	.gce_ddren2_mask_b = 0,
	.ddren2_emi_self_refresh_ch0_mask_b = 0,
	.ddren2_emi_self_refresh_ch1_mask_b = 0,

	/* MP0_CPU0_WFI_EN */
	.mp0_cpu0_wfi_en = 1,

	/* MP0_CPU1_WFI_EN */
	.mp0_cpu1_wfi_en = 1,

	/* MP0_CPU2_WFI_EN */
	.mp0_cpu2_wfi_en = 1,

	/* MP0_CPU3_WFI_EN */
	.mp0_cpu3_wfi_en = 1,

	/* MP1_CPU0_WFI_EN */
	.mp1_cpu0_wfi_en = 1,

	/* MP1_CPU1_WFI_EN */
	.mp1_cpu1_wfi_en = 1,

	/* MP1_CPU2_WFI_EN */
	.mp1_cpu2_wfi_en = 1,

	/* MP1_CPU3_WFI_EN */
	.mp1_cpu3_wfi_en = 1,

	/* Auto-gen End */

	.md_ddr_en_0_dbc_len = 154,
};

struct spm_lp_scen __spm_vcorefs = {
	.pwrctrl	= &vcorefs_ctrl,
};

static void spm_vcorefs_pwarp_cmd(uint64_t cmd, uint64_t val)
{
	if (cmd < NR_IDX_ALL)
		mt_spm_pmic_wrap_set_cmd(PMIC_WRAP_PHASE_ALLINONE, cmd, val);
	else
		INFO("cmd out of range!\n");
}


void spm_request_dvfs_opp(__uint64_t id, __uint64_t reg)
{
	switch (id) {
	case 0: /* ZQTX */
		mmio_write_32(DVFSRC_SEC_SW_REQ, reg);
		break;
	default:
		break;
	}
}
static void spm_dvfsfw_init(__uint64_t boot_up_opp, __uint64_t dram_issue)
{
	mmio_write_32(SPM_DFS_LEVEL, (0x1 << 0));
	mmio_write_32(SPM_DVS_LEVEL, (0x1 << 18) | (0x1 << 1));
	mmio_write_32(SPM_RSV_CON, mmio_read_32(SPM_RSV_CON) | (0x1 << 1));
	mmio_write_32(SPM_SW_RSV_5, (mmio_read_32(SPM_SW_RSV_5) & ~(0xFFFF)) | (0x1 << 13));
	mmio_write_32(DVFSRC_EVENT_SEL, 0x0);
	mmio_write_32(DVFSRC_EVENT_MASK_CON, 0x1FFFF);
	mmio_write_32(DRAMC_DPY_CLK_SW_CON_SEL, mmio_read_32(DRAMC_DPY_CLK_SW_CON_SEL) | (0x3 << 20));
	mmio_write_32(DRAMC_DPY_CLK_SW_CON_SEL2, mmio_read_32(DRAMC_DPY_CLK_SW_CON_SEL2) | (0x7f));


	if (__spmfw_idx == SPMFW_LP3_1CH)
		mmio_write_32(DRAMC_DPY_CLK_SW_CON_SEL, mmio_read_32(DRAMC_DPY_CLK_SW_CON_SEL) | (0x1 << 25));
}

void __spm_sync_vcore_dvfs_power_control(struct pwr_ctrl *dest_pwr_ctrl, const struct pwr_ctrl *src_pwr_ctrl)
{
#if 1
	__uint32_t dvfs_mask = SPM_FLAG_DIS_VCORE_DVS | SPM_FLAG_DIS_VCORE_DFS | SPM_FLAG_DISABLE_MMSYS_DVFS;

	dest_pwr_ctrl->pcm_flags = (dest_pwr_ctrl->pcm_flags & (~dvfs_mask)) |
					(src_pwr_ctrl->pcm_flags & dvfs_mask);

	if (dest_pwr_ctrl->pcm_flags_cust)
		dest_pwr_ctrl->pcm_flags_cust = (dest_pwr_ctrl->pcm_flags_cust & (~dvfs_mask)) |
						(src_pwr_ctrl->pcm_flags & dvfs_mask);
#endif
}

static void spm_go_to_vcorefs(__uint64_t spm_flags)
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

	pwrctrl = __spm_vcorefs.pwrctrl;

	if (mmio_read_32(PCM_REG15_DATA) != 0) {
		set_pwrctrl_pcm_flags(pwrctrl, spm_flags);
		__spm_set_power_control(pwrctrl);
		__spm_set_pcm_flags(pwrctrl);
		__spm_send_cpu_wakeup_event();
	} else {
		set_pwrctrl_pcm_flags(pwrctrl, spm_flags);

		__spm_reset_and_init_pcm(pcmdesc);

		__spm_kick_im_to_fetch(pcmdesc);

		__spm_init_pcm_register();

		__spm_init_event_vector(pcmdesc);

		__spm_set_power_control(pwrctrl);

		__spm_set_wakeup_event(pwrctrl);

		__spm_sync_mc_dsr_power_control(pwrctrl, NULL);

		__spm_kick_pcm_to_run(pwrctrl);
	}
}

void spm_vcorefs_args(__uint64_t x1, __uint64_t x2, __uint64_t x3)
{
	__uint64_t cmd = x1;

	switch (cmd) {
	case VCOREFS_SMC_CMD_0:
		spm_dvfsfw_init(x2, x3);
		break;
	case VCOREFS_SMC_CMD_1:
		spm_go_to_vcorefs(x2);
		break;
	case VCOREFS_SMC_CMD_2:
		spm_request_dvfs_opp(x2, x3);
		break;
	case VCOREFS_SMC_CMD_3:
		spm_vcorefs_pwarp_cmd(x2, x3);
		break;
	default:
		break;
	}
}

