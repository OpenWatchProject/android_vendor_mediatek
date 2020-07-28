#include <debug.h>
#include <mmio.h>
#include <mtk_plat_common.h>
#include <platform.h>
#include <platform_def.h>
#include <console.h>
#include <stdint.h>
#include "ccci.h"

unsigned int ccci_md_dbgsys_config(uint64_t reg_id, uint64_t value)
{
	switch (reg_id) {
	case MD_REG_PC_MONITOR:
		mmio_write_32(DBGSYS_PCMONITOR_BASE + 0x800, value);
		break;
	case MD_REG_MDMCU_BUSMON:
		mmio_write_32(DBGSYS_MDMCU_BUSMON_BASE + 0x10, value);
		break;
	case MD_REG_MDINFRA_BUSMON:
		mmio_write_32(DBGSYS_MDINFRA_BUSMON_BASE + 0x10, value);
		break;
	default:
		break;
	}

	return 0;
}
