#include <arch_helpers.h>
#include <platform.h>
#include <platform_def.h>
#include <mmio.h>
#include <mtk_latch.h>

unsigned long lastbus_perimon_check_hang(void)
{
	return (mmio_read_32(BUS_PERI_R1) & 0x1);
}

unsigned long lastbus_perimon_get(unsigned int index)
{
	return (index > BUS_MAX_NR_MON) ? 0 : mmio_read_32(BUS_PERI_MON0 + index*4);
}

unsigned long lastbus_perimon_init(unsigned int val_timeout, unsigned int val_enable)
{
	mmio_write_32(BUS_PERI_R0, val_timeout & 0x3fff);
	mmio_write_32(BUS_PERI_R1, val_enable);
	return 0;
}
