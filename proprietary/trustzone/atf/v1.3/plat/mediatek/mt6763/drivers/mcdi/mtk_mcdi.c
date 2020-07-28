#include <mmio.h>
#include <sspm_reg.h>

void sspm_cluster_pwr_off_notify(unsigned long cluster)
{
    mmio_write_32(SSPM_MBOX_3_BASE + 4 * (MCDI_MBOX_CLUSTER_0_ATF_ACTION_DONE + cluster), 1);
}

void sspm_cluster_pwr_on_notify(unsigned long cluster)
{
    mmio_write_32(SSPM_MBOX_3_BASE + 4 * (MCDI_MBOX_CLUSTER_0_ATF_ACTION_DONE + cluster), 0);
}

void sspm_standbywfi_irq_enable(unsigned long cpu)
{
	mmio_write_32(SSPM_CFGREG_ACAO_INT_SET, STANDBYWFI_EN(cpu));
}

void sspm_gicirqout_irq_enable(unsigned long cpu)
{
	mmio_write_32(SSPM_CFGREG_ACAO_INT_SET, GIC_IRQOUT_EN(cpu));
}

