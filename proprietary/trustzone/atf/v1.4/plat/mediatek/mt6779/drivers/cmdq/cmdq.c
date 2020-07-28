/*
* Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
*
* SPDX-License-Identifier: BSD-3-Clause
*/

#include <mmio.h>
#include <debug.h>
#include "cmdq.h"

#define GCE_BASE		0x10228000
#define GCE_DEBUG_CTRL		(GCE_BASE + 0x54)

enum cmdq_smc_request {
	CMDQ_ENABLE_DEBUG,
};

void smc_cmdq_request(unsigned long request)
{
	switch (request) {
	case CMDQ_ENABLE_DEBUG:
		mmio_write_32(GCE_DEBUG_CTRL, 0x10000);
		INFO("[cmdq] enable ctrl done\n");
		break;
	};
}

