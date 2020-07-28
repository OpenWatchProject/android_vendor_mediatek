/*
 * Copyright (c) since 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mt_lpm_dispatch.h>

/*
 * Notice, this data don't link to bss section.
 * It means data structure won't be set as zero.
 * Please make sure the member will be initialized.
 */
struct mt_dispatch_ctrl mt_dispatcher
__attribute__((section("mt_lpm_s"))) = {
	.enable = 0,
};

void mt_lpm_dispatcher_registry(int id, mt_lpm_dispatch_fn fn)
{
	if (id >= mt_lpm_smc_user_max)
		return;

	mt_dispatcher.enable |= 1 << id;
	mt_dispatcher.fn[id] = fn;

	flush_dcache_range((uintptr_t)&mt_dispatcher
			, sizeof(mt_dispatcher));
}

