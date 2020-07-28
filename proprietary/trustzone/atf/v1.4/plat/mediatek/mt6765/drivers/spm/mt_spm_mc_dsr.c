#include <stddef.h>

#include <mt_spm.h>
#include <mt_spm_internal.h>
#include <sleep_def.h>

void __spm_sync_mc_dsr_power_control(struct pwr_ctrl *dest_pwr_ctrl, const struct pwr_ctrl *src_pwr_ctrl)
{
	/* always disable MC-DSR */
	dest_pwr_ctrl->pcm_flags1 |= SPM_FLAG1_DISABLE_MCDSR;
}
