#include <debug.h>
#include <apu.h>
#include <platform.h>
#include <plat_private.h>
#include <mmio.h>

void apu_vcore_cg_ctl(int poweron)
{

	if (poweron == 1) {
		mmio_write_32((uintptr_t)APUSYS_VCORE_CG_CLR, 0x0000000F);
		INFO("APUSYS VCORE CG CLR\n");
	} else {
		mmio_write_32((uintptr_t)APUSYS_VCORE_CG_SET, 0x0000000F);
		INFO("APUSYS VCORE CG SET\n");
	}
}
