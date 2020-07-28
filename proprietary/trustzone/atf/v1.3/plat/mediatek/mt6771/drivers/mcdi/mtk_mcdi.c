#include <mmio.h>
#include <sspm_reg.h>
#include <mtk_mcdi.h>

void __attribute__((weak)) gic_sgi_save_all(void)
{
}

void __attribute__((weak)) gic_sgi_restore_all(void)
{
}

void sspm_set_bootaddr(unsigned long long bootaddr)
{
	mmio_write_32(SSPM_MBOX_3_BASE + 4 * (MCDI_MBOX_BOOTADDR), (unsigned int)bootaddr);
}

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

void sspm_drcc_cali_done_notify(void)
{
	mmio_write_32(SSPM_MBOX_3_BASE + 4 * MCDI_MBOX_DRCC_CALI_DONE, 1);
}

void mcdi_update_async_wakeup_enable(unsigned long long value)
{
	unsigned int cpu = (unsigned int)value;

	mmio_write_32(SSPM_CFGREG_ACAO_WAKEUP_EN, (1 << cpu));
}

void mcdi_receive_cmd(unsigned long long x1, unsigned long long x2,
			unsigned long long x3)
{
	unsigned long long cmd = x1;

	switch (cmd) {
	case MCDI_SMC_EVENT_ASYNC_WAKEUP_EN:
		mcdi_update_async_wakeup_enable(x2);
		break;
	default:
		break;
	}
}
