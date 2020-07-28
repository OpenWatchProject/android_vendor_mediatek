#include <arch_helpers.h>
#include <platform.h>
#include <platform_def.h>
#include <mmio.h>

#define EMI_WFIFO	(0x05f8)

void enable_emi_wfifo(void)
{
	mmio_write_32(MCUCFG_BASE + EMI_WFIFO, 0xf);
}
