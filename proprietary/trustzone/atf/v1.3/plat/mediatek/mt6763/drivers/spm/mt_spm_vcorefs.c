#include <arch_helpers.h>
#include <debug.h>
#include <delay_timer.h>
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

#define DVFSRC_LEVEL         (DVFSRC_BASE + 0x50)
#define DVFSRC_SEC_SW_REQ    (DVFSRC_BASE + 0x84)
#define DVFSRC_MD_REQUEST    (DVFSRC_BASE + 0x28)
#define DVFSRC_MD_SW_CONTROL (DVFSRC_BASE + 0x2c)
#define DVFSRC_FORCE         (DVFSRC_BASE + 0x80)
#define DVFSRC_BASIC_CONTROL (DVFSRC_BASE + 0x0)

#define SPM_DVFS_TIMEOUT       1000     /* 1ms */

static struct pwr_ctrl vcorefs_ctrl = {
	.wake_src		= R12_PCM_TIMER,

	/* default VCORE DVFS is disabled */
	.pcm_flags		= (SPM_FLAG_RUN_COMMON_SCENARIO | SPM_FLAG_DIS_VCORE_DVS | SPM_FLAG_DIS_VCORE_DFS),

	/* Auto-gen Start */

	/* SPM_AP_STANDBY_CON */
	.wfi_op = WFI_OP_AND,
	.mp0_cputop_idle_mask = 0,
	.mp1_cputop_idle_mask = 0,
	.mcusys_idle_mask = 0,
	.mm_mask_b = 0,
	.md_ddr_en_0_dbc_en = 0,
	.md_ddr_en_1_dbc_en = 0,
	.md_mask_b = 0,
	.sspm_mask_b = 0,
	.lte_mask_b = 0,
	.srcclkeni_mask_b = 0,
	.md_apsrc_1_sel = 0,
	.md_apsrc_0_sel = 0,
	.conn_ddr_en_dbc_en = 0,
	.conn_mask_b = 0,
	.conn_apsrc_sel = 0,

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
	.csyspwreq_mask = 0,
	.ccif0_md_event_mask_b = 0,
	.ccif0_ap_event_mask_b = 0,
	.ccif1_md_event_mask_b = 0,
	.ccif1_ap_event_mask_b = 0,
	.ccifmd_md1_event_mask_b = 0,
	.ccifmd_md2_event_mask_b = 0,
	.dsi0_vsync_mask_b = 0,
	.dsi1_vsync_mask_b = 0,
	.dpi_vsync_mask_b = 0,
	.isp0_vsync_mask_b = 0,
	.isp1_vsync_mask_b = 0,
	.md_srcclkena_0_infra_mask_b = 0,
	.md_srcclkena_1_infra_mask_b = 0,
	.conn_srcclkena_infra_mask_b = 0,
	.sspm_srcclkena_infra_mask_b = 0,
	.srcclkeni_infra_mask_b = 0,
	.md_apsrc_req_0_infra_mask_b = 0,
	.md_apsrc_req_1_infra_mask_b = 0,
	.conn_apsrcreq_infra_mask_b = 0,
	.sspm_apsrcreq_infra_mask_b = 0,
	.md_ddr_en_0_mask_b = 0,
	.md_ddr_en_1_mask_b = 0,
	.md_vrf18_req_0_mask_b = 0,
	.md_vrf18_req_1_mask_b = 0,
	.md1_dvfs_req_mask = 0,
	.cpu_dvfs_req_mask = 0,
	.emi_bw_dvfs_req_mask = 0,
	.md_srcclkena_0_dvfs_req_mask_b = 0,
	.md_srcclkena_1_dvfs_req_mask_b = 0,
	.conn_srcclkena_dvfs_req_mask_b = 0,

	/* SPM_SRC2_MASK */
	.dvfs_halt_mask_b = 0,
	.vdec_req_mask_b = 0,
	.gce_req_mask_b = 0,
	.cpu_md_dvfs_req_merge_mask_b = 0,
	.md_ddr_en_dvfs_halt_mask_b = 0,
	.dsi0_vsync_dvfs_halt_mask_b = 0,
	.dsi1_vsync_dvfs_halt_mask_b = 0,
	.dpi_vsync_dvfs_halt_mask_b = 0,
	.isp0_vsync_dvfs_halt_mask_b = 0,
	.isp1_vsync_dvfs_halt_mask_b = 0,
	.conn_ddr_en_mask_b = 0,
	.disp_req_mask_b = 0,
	.disp1_req_mask_b = 0,
	.mfg_req_mask_b = 0,
	.ufs_srcclkena_mask_b = 0,
	.ufs_vrf18_req_mask_b = 0,
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
	.spm_wakeup_event_mask = 0,

	/* SPM_WAKEUP_EVENT_EXT_MASK */
	.spm_wakeup_event_ext_mask = 0,

	/* SPM_SRC3_MASK */
	.md_ddr_en_2_0_mask_b = 0,
	.md_ddr_en_2_1_mask_b = 0,
	.conn_ddr_en_2_mask_b = 0,
	.dramc_spcmd_apsrc_req_2_mask_b = 0,
	.spare1_ddren_2_mask_b = 0,
	.spare2_ddren_2_mask_b = 0,
	.ddren_emi_self_refresh_ch0_mask_b = 0,
	.ddren_emi_self_refresh_ch1_mask_b = 0,
	.ddren_mm_state_mask_b = 0,
	.ddren_sspm_apsrc_req_mask_b = 0,
	.ddren_dqssoc_req_mask_b = 0,
	.ddren2_emi_self_refresh_ch0_mask_b = 0,
	.ddren2_emi_self_refresh_ch1_mask_b = 0,
	.ddren2_mm_state_mask_b = 0,
	.ddren2_sspm_apsrc_req_mask_b = 0,
	.ddren2_dqssoc_req_mask_b = 0,

	/* MP0_CPU0_WFI_EN */
	.mp0_cpu0_wfi_en = 0,

	/* MP0_CPU1_WFI_EN */
	.mp0_cpu1_wfi_en = 0,

	/* MP0_CPU2_WFI_EN */
	.mp0_cpu2_wfi_en = 0,

	/* MP0_CPU3_WFI_EN */
	.mp0_cpu3_wfi_en = 0,

	/* MP1_CPU0_WFI_EN */
	.mp1_cpu0_wfi_en = 0,

	/* MP1_CPU1_WFI_EN */
	.mp1_cpu1_wfi_en = 0,

	/* MP1_CPU2_WFI_EN */
	.mp1_cpu2_wfi_en = 0,

	/* MP1_CPU3_WFI_EN */
	.mp1_cpu3_wfi_en = 0,

	/* Auto-gen End */
};

struct spm_lp_scen __spm_vcorefs = {
	.pwrctrl	= &vcorefs_ctrl,
};

void spm_request_dvfs_opp(__uint64_t id, __uint64_t reg)
{
	int i = 0;
	int level = 0;

	if (__spmfw_idx == SPMFW_LP4X_2CH) {
		if (reg == 0x2)
			level = 0x4;
		else if (reg == 0x1)
			level = 0x2;
		else if (reg == 0x0)
			level = 0x1;
	} else if (__spmfw_idx == SPMFW_LP3_1CH) {
		if (reg == 0x2)
			level = 0x8;
		else if (reg == 0x1)
			level = 0x2;
		else if (reg == 0x0)
			level = 0x1;
	}

	switch (id) {
	case 0: /* ZQTX */
		mmio_write_32(DVFSRC_SEC_SW_REQ, reg);
		if (reg != 0x0) {
			while ((mmio_read_32(DVFSRC_LEVEL) >> 16) < level) {
				if (i >= SPM_DVFS_TIMEOUT)
					break;
				udelay(1);
				i++;
			}
		}
		break;
	case 1: /* Low Power Scenario */
		if (reg == 0x0) {
			mmio_write_32(DVFSRC_FORCE, 0x0);
			mmio_write_32(DVFSRC_BASIC_CONTROL,
				mmio_read_32(DVFSRC_BASIC_CONTROL) | (1U << 8) | (1U << 15));
		}

		mmio_write_32(DVFSRC_MD_REQUEST, reg);

		if (reg != 0x0) {
			i = 0;
			while ((mmio_read_32(DVFSRC_LEVEL) >> 16) < 0x8000) {
				if (i >= SPM_DVFS_TIMEOUT)
					break;
				udelay(1);
				i++;
			}
		} else {
			i = 0;
			while (mmio_read_32(DVFSRC_MD_REQUEST) != 0) {
				if (i >= SPM_DVFS_TIMEOUT)
					break;
				udelay(1);
				i++;
			}
			mmio_write_32(DVFSRC_BASIC_CONTROL,
				mmio_read_32(DVFSRC_BASIC_CONTROL) & ~((1U << 8) | (1U << 15)));
		}
		break;
	default:
		break;
	}
}

static void spm_dvfsfw_init(__uint64_t boot_up_opp, __uint64_t dram_issue)
{
	if (__spmfw_idx == SPMFW_LP4X_2CH || __spmfw_idx == SPMFW_LP3_1CH)
		mmio_write_32(SPM_DFS_LEVEL, (0x1 << 0));
	else if (__spmfw_idx == SPMFW_LP4X_1CH)
		mmio_write_32(SPM_DFS_LEVEL, (0x1 << 1));

	mmio_write_32(SPM_DVS_LEVEL, (0x1 << 16) | (0x1 << 1));
	mmio_write_32(SPM_RSV_CON, mmio_read_32(SPM_RSV_CON) | (0x1 << 1));
	mmio_write_32(SPM_SW_RSV_5, (mmio_read_32(SPM_SW_RSV_5) & ~(0xFFFF)) | (0x1 << 13));
	mmio_write_32(DVFSRC_EVENT_SEL, 0x0);
	mmio_write_32(DVFSRC_EVENT_MASK_CON, 0x1FFFF);
	mmio_write_32(DRAMC_DPY_CLK_SW_CON_SEL, mmio_read_32(DRAMC_DPY_CLK_SW_CON_SEL) | (0x3 << 20));
	mmio_write_32(DRAMC_DPY_CLK_SW_CON_SEL2, mmio_read_32(DRAMC_DPY_CLK_SW_CON_SEL2) | (0x7f));

	/* Bianco DRAM LP4-2ch HW issue, opp1 voltage change from 0.7 to 0.8 */
	if (__spmfw_idx == SPMFW_LP4X_2CH)
		if (dram_issue)
			mmio_write_32(SPM_SW_RSV_2, mmio_read_32(SPM_SW_RSV_2) | SPM_FLAG1_RESERVED_BIT4);

	if (__spmfw_idx == SPMFW_LP3_1CH)
		mmio_write_32(DRAMC_DPY_CLK_SW_CON_SEL, mmio_read_32(DRAMC_DPY_CLK_SW_CON_SEL) | (0x1 << 25));
}

void __spm_sync_vcore_dvfs_power_control(struct pwr_ctrl *dest_pwr_ctrl, const struct pwr_ctrl *src_pwr_ctrl)
{
#if 1
	__uint32_t dvfs_mask = SPM_FLAG_DIS_VCORE_DVS | SPM_FLAG_DIS_VCORE_DFS;

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

	set_pwrctrl_pcm_flags(pwrctrl, spm_flags);

	__spm_reset_and_init_pcm(pcmdesc);

	__spm_kick_im_to_fetch(pcmdesc);

	__spm_init_pcm_register();

	__spm_init_event_vector(pcmdesc);

	__spm_set_power_control(pwrctrl);

	__spm_set_wakeup_event(pwrctrl);

	__spm_kick_pcm_to_run(pwrctrl);
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
		default:
			break;
	}
}

