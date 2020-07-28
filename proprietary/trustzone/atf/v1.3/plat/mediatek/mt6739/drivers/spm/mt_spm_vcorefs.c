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
#define DVFSRC_MD_REQUEST    (DVFSRC_BASE + 0x28)

#define SPM_DVFS_TIMEOUT       5000     /* 5ms */

static struct pwr_ctrl vcorefs_ctrl = {
	.wake_src		= R12_PCM_TIMER,

	/* default VCORE DVFS is disabled */
	.pcm_flags		= (SPM_FLAG_RUN_COMMON_SCENARIO | SPM_FLAG_DIS_VCORE_DVS | SPM_FLAG_DIS_VCORE_DFS),
};

struct spm_lp_scen __spm_vcorefs = {
	.pwrctrl	= &vcorefs_ctrl,
};

static void spm_vcorefs_pwarp_cmd(uint64_t opp, uint64_t reg)
{
#if 0
	uint64_t cmd0, cmd1, cmd2, cmd3;
	uint64_t cmd4, cmd5, cmd6, diff;
#endif

	switch (opp) {
	case 0:
		mt_spm_pmic_wrap_set_cmd(PMIC_WRAP_PHASE_ALLINONE, CMD_0, reg);
		break;
	case 1:
		mt_spm_pmic_wrap_set_cmd(PMIC_WRAP_PHASE_ALLINONE, CMD_1, reg);
		break;
	case 2:
		mt_spm_pmic_wrap_set_cmd(PMIC_WRAP_PHASE_ALLINONE, CMD_2, reg);
		break;
	case 3:
		mt_spm_pmic_wrap_set_cmd(PMIC_WRAP_PHASE_ALLINONE, CMD_3, reg);
		break;
	}

	mt_spm_pmic_wrap_set_phase(PMIC_WRAP_PHASE_ALLINONE);
#if 0
	if (opp == 3) {
		cmd0 = mt_spm_pmic_wrap_get_cmd(PMIC_WRAP_PHASE_ALLINONE, CMD_0);
		cmd3 = mt_spm_pmic_wrap_get_cmd(PMIC_WRAP_PHASE_ALLINONE, CMD_3);
		cmd6 = mt_spm_pmic_wrap_get_cmd(PMIC_WRAP_PHASE_ALLINONE, CMD_6);

		diff = cmd0 - cmd3;
		cmd1 = cmd3 + (((diff*10)/3) * 2)/10;
		cmd2 = cmd3 + (((diff*10)/3) * 1)/10;

		diff = cmd3 - cmd6;
		cmd4 = cmd6 + (((diff*10)/3) * 2)/10;
		cmd5 = cmd6 + (((diff*10)/3) * 1)/10;

		mt_spm_pmic_wrap_set_cmd(PMIC_WRAP_PHASE_ALLINONE, CMD_1, cmd1);
		mt_spm_pmic_wrap_set_cmd(PMIC_WRAP_PHASE_ALLINONE, CMD_2, cmd2);
		mt_spm_pmic_wrap_set_cmd(PMIC_WRAP_PHASE_ALLINONE, CMD_4, cmd4);
		mt_spm_pmic_wrap_set_cmd(PMIC_WRAP_PHASE_ALLINONE, CMD_5, cmd5);

		mt_spm_pmic_wrap_set_phase(PMIC_WRAP_PHASE_ALLINONE);
	}
#endif
}

void spm_request_dvfs_opp(__uint64_t id, __uint64_t reg)
{
	int i = 0;

	switch (id) {
	case 0:
		break;
	case 1:
		mmio_write_32(DVFSRC_MD_REQUEST, reg);

		if (reg != 0x0) {
			i = 0;
			while ((mmio_read_32(DVFSRC_LEVEL) >> 16) < 0x20) {
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
	mmio_write_32(SPM_DFS_LEVEL, (0x1 << 3));
	mmio_write_32(SPM_DVS_LEVEL, (0x1 << 16) | (0x1 << 2));
	mmio_write_32(SPM_SW_RSV_9, (0x1 << 4));
	mmio_write_32(SPM_SW_FLAG, mmio_read_32(SPM_SW_FLAG) | (0x1 << 21));
	mmio_write_32(SPM_SW_FLAG, mmio_read_32(SPM_SW_FLAG) | (0x1 << 25));
	mmio_write_32(SPM_RSV_CON, mmio_read_32(SPM_RSV_CON) | (0x1 << 1));
	mmio_write_32(DVFSRC_EVENT_SEL, 0x0);
	mmio_write_32(DVFSRC_EVENT_MASK_CON, 0x100FF);
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

	if (dyna_load_pcm[DYNA_LOAD_PCM_SODI].ready) {
		pcmdesc = &(dyna_load_pcm[DYNA_LOAD_PCM_SODI].desc);
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

	/* __spm_set_power_control(pwrctrl); */

	__spm_set_wakeup_event(pwrctrl);

	__spm_kick_pcm_to_run(pwrctrl);
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
		spm_vcorefs_pwarp_cmd(x2, x3);
		break;
	default:
		break;
	}
}

