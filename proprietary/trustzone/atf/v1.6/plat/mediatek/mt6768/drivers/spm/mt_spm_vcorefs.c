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
#define DVFSRC_LEVEL         (DVFSRC_BASE + 0x50)
#define DVFSRC_SEC_SW_REQ    (DVFSRC_BASE + 0x84)
#define SPM_DVFS_TIMEOUT       1000     /* 1ms */

static struct pwr_ctrl vcorefs_ctrl = {
	.wake_src		= R12_PCM_TIMER,

	/* default VCORE DVFS is disabled */
	.pcm_flags = (SPM_FLAG_RUN_COMMON_SCENARIO |
			SPM_FLAG_DIS_VCORE_DVS | SPM_FLAG_DIS_VCORE_DFS),

	/* Auto-gen Start */

	/* SPM_AP_STANDBY_CON */
#if MCUSYS_MTCMOS_ON
	.wfi_op = 0x1,
	.wfi_type = 0,
	.mcusys_idle_mask = 0x1,
#else
	.wfi_op = 0,
	.wfi_type = 0x1,
	.mcusys_idle_mask = 0,
#endif
	.mm_mask_b = 0,
	.md_ddr_en_0_dbc_en = 0,
	.md_ddr_en_1_dbc_en = 0,
	.md_mask_b = 0,
	.sspm_mask_b = 0,
	.srcclkeni_mask_b = 0,
	.md_apsrc_1_sel = 0,
	.md_apsrc_0_sel = 0,
	.conn_ddr_en_dbc_en = 0,
	.conn_mask_b = 0,
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
	.csyspwreq_mask = 0,
	.ccif0_md_event_mask_b = 0,
	.ccif0_ap_event_mask_b = 0,
	.ccif1_md_event_mask_b = 0,
	.ccif1_ap_event_mask_b = 0,
	.md_srcclkena_0_infra_mask_b = 0,
	.md_srcclkena_1_infra_mask_b = 0,
	.conn_srcclkena_infra_mask_b = 0,
	.srcclkeni_infra_mask_b = 0,
	.md_apsrc_req_0_infra_mask_b = 0,
	.md_apsrc_req_1_infra_mask_b = 0,
	.conn_apsrcreq_infra_mask_b = 0,
	.md_ddr_en_0_mask_b = 0,
	.md_ddr_en_1_mask_b = 0,
	.md_vrf18_req_0_mask_b = 0,
	.md_vrf18_req_1_mask_b = 0,

	/* SPM_SRC2_MASK */
	.vdec_req_mask_b = 0,
	.conn_ddr_en_mask_b = 0,
	.mfg_req_mask_b = 0,
	.ufs_srcclkena_mask_b = 0,
	.ufs_vrf18_req_mask_b = 0,
	.gce_vrf18_req_mask_b = 0,

	/* SPM_WAKEUP_EVENT_MASK */
	.spm_wakeup_event_mask = 0,

	/* SPM_WAKEUP_EVENT_EXT_MASK */
	.spm_wakeup_event_ext_mask = 0,

	/* SPM_SRC3_MASK */
	.md_ddr_en_2_0_mask_b = 0,
	.md_ddr_en_2_1_mask_b = 0,
	.conn_ddr_en_2_mask_b = 0,
	.ddren_emi_self_refresh_ch0_mask_b = 0,
	.ddren_emi_self_refresh_ch1_mask_b = 0,
	.ddren2_emi_self_refresh_ch0_mask_b = 0,
	.ddren2_emi_self_refresh_ch1_mask_b = 0,

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


void spm_request_dvfs_opp(uint64_t id, uint64_t reg)
{
	int i = 0;

	switch (id) {
	case 0: /* ZQTX */
		mmio_write_32(DVFSRC_SEC_SW_REQ, reg);
		if (reg != 0x0) {
			while ((mmio_read_32(DVFSRC_LEVEL) >> 16) < 0x4) {
				if (i >= SPM_DVFS_TIMEOUT)
					break;
				udelay(1);
				i++;
			}
			return;
		}
		break;
	default:
		break;
	}
}
static void spm_dvfsfw_init(uint64_t boot_up_opp, uint64_t dram_issue)
{
	if (__spmfw_idx == SPMFW_LP4X_2CH_3600 || __spmfw_idx == SPMFW_LP4_2CH_3200) {
		mmio_write_32(SPM_DVS_LEVEL, (0x1 << 18) | (0x1 << 3));
		mmio_write_32(SPM_DFS_LEVEL, (0x1 << 2));
		mmio_write_32(SPM_SW_RSV_9, (0x1 << 31) | (0x1 << 14));
	} else if (__spmfw_idx == SPMFW_LP3_1CH_1866) {
		mmio_write_32(SPM_DVS_LEVEL, (0x1 << 18) | (0x1 << 3));
		mmio_write_32(SPM_DFS_LEVEL, (0x1 << 2));
		mmio_write_32(SPM_SW_RSV_9, (0x1 << 31) | (0x1 << 14));
	}
	mmio_write_32(DVFSRC_EVENT_MASK_CON, 0x1FFFF);
	mmio_write_32(DVFSRC_EVENT_SEL, 0x0);
}

void __spm_sync_vcore_dvfs_power_control(struct pwr_ctrl *dest_pwr_ctrl, const struct pwr_ctrl *src_pwr_ctrl)
{
#if 1
	uint32_t dvfs_mask = SPM_FLAG_DIS_VCORE_DVS | SPM_FLAG_DIS_VCORE_DFS;

	dest_pwr_ctrl->pcm_flags = (dest_pwr_ctrl->pcm_flags & (~dvfs_mask)) |
					(src_pwr_ctrl->pcm_flags & dvfs_mask);

	if (dest_pwr_ctrl->pcm_flags_cust)
		dest_pwr_ctrl->pcm_flags_cust = (dest_pwr_ctrl->pcm_flags_cust & (~dvfs_mask)) |
						(src_pwr_ctrl->pcm_flags & dvfs_mask);
#endif
}

static void spm_go_to_vcorefs(uint64_t spm_flags)
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

		__spm_kick_pcm_to_run(pwrctrl);
	}
}

static void spm_vcorefs_freq_hopping(uint64_t gps_on)
{
	int i = 0;

	if (gps_on) {
		mmio_write_32(SW2SPM_MAILBOX_0, (mmio_read_32(SW2SPM_MAILBOX_0) & ~0x3) | 0x3);
		mmio_write_32(SPM_CPU_WAKEUP_EVENT, 1);
		while ((mmio_read_32(SPM2SW_MAILBOX_0) & 0x1) != 0x1) {
			if (i >= SPM_DVFS_TIMEOUT)
				break;
			udelay(1);
			i++;
		}
		mmio_write_32(SW2SPM_MAILBOX_0, (mmio_read_32(SW2SPM_MAILBOX_0) & ~0x1));
		mmio_write_32(SPM_CPU_WAKEUP_EVENT, 0);
		mmio_write_32(SPM2SW_MAILBOX_0, (mmio_read_32(SPM2SW_MAILBOX_0) & ~0x1));
	} else {
		mmio_write_32(SW2SPM_MAILBOX_0, (mmio_read_32(SW2SPM_MAILBOX_0) & ~0x3) | 0x1);
		__spm_send_cpu_wakeup_event();
		while ((mmio_read_32(SPM2SW_MAILBOX_0) & 0x1) != 0x1) {
			if (i >= SPM_DVFS_TIMEOUT)
				break;
			udelay(1);
			i++;
		}
		mmio_write_32(SW2SPM_MAILBOX_0, (mmio_read_32(SW2SPM_MAILBOX_0) & ~0x1));
		mmio_write_32(SPM_CPU_WAKEUP_EVENT, 0);
		mmio_write_32(SPM2SW_MAILBOX_0, (mmio_read_32(SPM2SW_MAILBOX_0) & ~0x1));
	}
}

void spm_vcorefs_args(uint64_t x1, uint64_t x2, uint64_t x3)
{
	uint64_t cmd = x1;

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
	case VCOREFS_SMC_CMD_4:
		spm_vcorefs_freq_hopping(x2);
		break;
	default:
		break;
	}
}

