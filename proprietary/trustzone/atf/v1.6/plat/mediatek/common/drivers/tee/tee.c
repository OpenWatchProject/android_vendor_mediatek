/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>
#include <mtk_gic_v3_main.h>
#include <tee.h>

void sip_tee_set_pending(void)
{
	gicd_set_ispendr(BASE_GICD_BASE, TEE_SANITY_IRQ_BIT_ID);
}

