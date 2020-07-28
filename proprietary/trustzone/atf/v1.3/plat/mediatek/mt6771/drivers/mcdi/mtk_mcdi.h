#ifndef __MTK_MCDI_H__
#define __MTK_MCDI_H__

enum {
	MCDI_SMC_EVENT_ASYNC_WAKEUP_EN = 0,

	NF_MCDI_SMC_EVENT
};

void sspm_set_bootaddr(unsigned long long bootaddr);
void sspm_standbywfi_irq_enable(unsigned long cpu);
void sspm_gicirqout_irq_enable(unsigned long cpu);
void sspm_cluster_pwr_off_notify(unsigned long cluster);
void sspm_cluster_pwr_on_notify(unsigned long cluster);
void sspm_drcc_cali_done_notify(void);
void mcdi_update_async_wakeup_enable(unsigned long long value);
void mcdi_receive_cmd(unsigned long long cmd, unsigned long long arg1,
			unsigned long long arg2);

#endif /* __MTK_MCDI_H__ */
