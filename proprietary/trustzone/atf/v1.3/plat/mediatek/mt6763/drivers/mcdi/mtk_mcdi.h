#ifndef __MTK_MCDI_H__
#define __MTK_MCDI_H__

void sspm_standbywfi_irq_enable(unsigned long cpu);
void sspm_gicirqout_irq_enable(unsigned long cpu);
void sspm_cluster_pwr_off_notify(unsigned long cluster);
void sspm_cluster_pwr_on_notify(unsigned long cluster);

#endif /* __MTK_MCDI_H__ */
