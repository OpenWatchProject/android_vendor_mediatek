/*
 * Copyright (c) since 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <bl_common.h>
#include <debug.h>
#include <mtk_sip_svc.h>
#include <plat_private.h>
#include <platform_def.h>
#include <platform.h>
#include <stdint.h>
#include <string.h>
#include <memory_layout.h>

uint64_t sec_mem_address_check(uint64_t addr, uint64_t expect_addr)
{
	if (addr != expect_addr)
		return MTK_SIP_E_INVALID_PARAM;

	return 0;
}

uint64_t sec_mem_region_check(uint64_t size, uint64_t max_size)
{
	if (max_size < size)
		return MTK_SIP_E_INVALID_RANGE;

	return 0;
}

uint64_t lk_share_mem_check(uint64_t addr, uint64_t size)
{
	uint64_t ret = 0;

	ret = sec_mem_address_check(addr, LK_SECURE_BOOT_BASE);
	if (ret)
		goto end;

	ret = sec_mem_region_check(size, LK_SECURE_BOOT_MAX_SIZE);

end:
	return ret;
}

