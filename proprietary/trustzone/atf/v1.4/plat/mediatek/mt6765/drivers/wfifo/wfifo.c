#include <arch_helpers.h>
#include <platform.h>
#include <platform_def.h>
#include <mmio.h>
#include <wfifo.h>

void enable_emi_wfifo(void)
{
	mmio_write_32(MCUCFG_BASE + EMI_WFIFO, 0xf);
}
