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

static struct pwr_ctrl vcorefs_ctrl = {
	.wake_src		= R12_PCM_TIMER_EVENT,

	/* default VCORE DVFS is disabled */
	.pcm_flags = (SPM_FLAG_RUN_COMMON_SCENARIO |
			SPM_FLAG_DISABLE_VCORE_DVS | SPM_FLAG_DISABLE_VCORE_DFS),

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
	.reg_msdc0_srcclkena_mask_b = 1,
	.reg_msdc0_infra_req_mask_b = 1,
	.reg_msdc0_apsrc_req_mask_b = 1,
	.reg_msdc0_vrf18_req_mask_b = 1,
	.reg_msdc0_ddr_en_mask_b = 1,
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
	.reg_wakeup_event_mask = 0xEFFFFFFF,

	/* SPM_WAKEUP_EVENT_EXT_MASK */
	.reg_ext_wakeup_event_mask = 0xFFFFFFFF,

	/* SPM_SRC6_MASK */
	.reg_msdc1_srcclkena_mask_b = 1,
	.reg_msdc1_infra_req_mask_b = 1,
	.reg_msdc1_apsrc_req_mask_b = 1,
	.reg_msdc1_vrf18_req_mask_b = 1,
	.reg_msdc1_ddr_en_mask_b = 1,
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
}
void spm_dvfsfw_init(__uint64_t boot_up_opp, __uint64_t dram_issue)
{
	/* SPM_VCORE_DVFS_SHORTCUT */
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT00, 0x08400001);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT01, 0x18200002);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT02, 0x38100004);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT03, 0x58040008);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT04, 0x78020010);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT05, 0x22200020);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT06, 0x32100040);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT07, 0x52040080);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT08, 0x72020100);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT09, 0x41100200);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT10, 0x61080400);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT11, 0x81020800);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT12, 0x91011000);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT13, 0x00000000);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT14, 0x00000000);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT15, 0x14208000);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT_CON00, 0x76F43210);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT_CON01, 0x00088768);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT_CON02, 0x76F43210);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT_CON03, 0x11188768);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT_CON04, 0x76543210);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT_CON05, 0x22288768);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT_CON06, 0x76543210);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT_CON07, 0x33388768);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT_CON08, 0x76543210);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT_CON09, 0x44488768);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT_CON10, 0x765432FF);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT_CON11, 0x555BBA98);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT_CON12, 0x76543210);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT_CON13, 0x666BBA98);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT_CON14, 0x76543210);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT_CON15, 0x777BBA98);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT_CON16, 0x76543210);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT_CON17, 0x888BBA98);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT_CON18, 0x76566666);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT_CON19, 0x99966698);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT_CON20, 0x76577777);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT_CON21, 0xAAA77A78);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT_CON22, 0x76588888);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT_CON23, 0xBBBCB888);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT_CON24, 0xBBBBBBBB);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT_CON25, 0xBBBCBBBB);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT_CON26, 0x00000000);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT_CON27, 0x00000000);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT_CON28, 0x00000000);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT_CON29, 0x00000000);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT_CON30, 0x76543210);
	mmio_write_32(SPM_VCORE_DVFS_SHORTCUT_CON31, 0xFFF88768);
	mmio_write_32(SPM_DVFS_MISC, (mmio_read_32(SPM_DVFS_MISC) &
		~(SPM_DVFS_FORCE_ENABLE_LSB)) | (SPM_DVFSRC_ENABLE_LSB));

	mmio_write_32(SPM_DVFS_LEVEL, 0x00000001);
	mmio_write_32(SPM_DVS_DFS_LEVEL, 0x08000000);
}

void __spm_sync_vcore_dvfs_power_control(struct pwr_ctrl *dest_pwr_ctrl, const struct pwr_ctrl *src_pwr_ctrl)
{
#if 1
	__uint32_t dvfs_mask = SPM_FLAG_DISABLE_VCORE_DVS | SPM_FLAG_DISABLE_VCORE_DFS;

	dest_pwr_ctrl->pcm_flags = (dest_pwr_ctrl->pcm_flags & (~dvfs_mask)) |
					(src_pwr_ctrl->pcm_flags & dvfs_mask);

	if (dest_pwr_ctrl->pcm_flags_cust)
		dest_pwr_ctrl->pcm_flags_cust = (dest_pwr_ctrl->pcm_flags_cust & (~dvfs_mask)) |
						(src_pwr_ctrl->pcm_flags & dvfs_mask);
#endif
}

void dvfsrc_ddr_request(int en)
{
	if (en) {
		/* request DDR1600 for MD */
		mmio_write_32(DVFSRC_SW_REQ8, 0x00002000);

		if (!(mmio_read_32(DVFSRC_BASIC_CONTROL) & 0x00008000)) {
			while (mmio_read_32(SPM_DVFS_LEVEL) == 0x00001000)
				;
		}
	} else {
		/* release  DDR1600 */
		mmio_write_32(DVFSRC_SW_REQ8, 0x00000000);
	}
}

void spm_go_to_vcorefs(__uint64_t spm_flags)
{
	struct pcm_desc *pcmdesc = NULL;
	struct pwr_ctrl *pwrctrl;
	int spmfw_idx = __spm_get_spmfw_idx();
	static int spm_for_is_spmfw_running;

	if (dyna_load_pcm[spmfw_idx].ready) {
		pcmdesc = &(dyna_load_pcm[spmfw_idx].desc);
	} else {
		INFO("firmware is not ready!!!\n");
		return;
	}

	pwrctrl = __spm_vcorefs.pwrctrl;

	if (spm_for_is_spmfw_running) {
		set_pwrctrl_pcm_flags(pwrctrl, spm_flags);
		__spm_set_power_control(pwrctrl);
		__spm_set_pcm_flags(pwrctrl);
		__spm_send_cpu_wakeup_event();
	} else {
		set_pwrctrl_pcm_flags(pwrctrl, spm_flags);

		__spm_reset_and_init_pcm(pcmdesc);

		__spm_kick_im_to_fetch(pcmdesc);

		__spm_init_pcm_register();

		__spm_set_power_control(pwrctrl);

		__spm_set_wakeup_event(pwrctrl);

		__spm_kick_pcm_to_run(pwrctrl);

		spm_for_is_spmfw_running = 1;
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

