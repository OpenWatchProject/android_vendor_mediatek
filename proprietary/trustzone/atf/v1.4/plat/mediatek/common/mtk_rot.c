/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <cassert.h>
#include <debug.h>
#include <mtk_rot.h>

/*******************************************************************************
 * SMC call from LK to receive the root of trust info
 ******************************************************************************/
#define ROOT_OF_TRUST_SIZE (96)

uint8_t rot_write_enable = 1;

struct root_of_trust_info_t {
	unsigned char pubk_hash[32];
	int device_lock_state;
	int verified_boot_state;
	unsigned int os_version;
	unsigned int os_patchlevel;
	unsigned char vbmeta_digest[32];
	unsigned int vendor_patchlevel;
	unsigned int boot_patchlevel;
	unsigned int reserved[2];
};

static struct root_of_trust_info_t g_rot_info = { {0} };
static uint32_t *p_root_of_trust_info = (uint32_t *)(uintptr_t) &g_rot_info;
static uint8_t g_rot_smc_count;
/* 1 for vboot1.0(os_patchlevel = 0), 2 for avb2.0(os_patchlevel != 0) */
static uint32_t g_vb_version = 1;

uint64_t sip_save_root_of_trust_info(uint32_t arg0, uint32_t arg1,
	uint32_t arg2, uint32_t arg3)
{
	CASSERT(sizeof(struct root_of_trust_info_t) == ROOT_OF_TRUST_SIZE, RoT_wrong_size);

	INFO("ROT: 0x%x, 0x%x, 0x%x, 0x%x\n", arg0, arg1, arg2, arg3);

	if (!rot_write_enable)
		return -4;

	g_rot_smc_count++;

	*p_root_of_trust_info = arg0;
	p_root_of_trust_info++;
	*p_root_of_trust_info = arg1;
	p_root_of_trust_info++;
	*p_root_of_trust_info = arg2;
	p_root_of_trust_info++;
	*p_root_of_trust_info = arg3;
	p_root_of_trust_info++;

	/* vboot1.0: smc count = 3, vb2.0: smc count = 6 */
	if (g_rot_smc_count == 3)
		g_vb_version = g_rot_info.os_patchlevel == 0 ? 1 : 2;

	if (g_rot_smc_count == 3 && g_vb_version == 1)
		rot_write_enable = 0;
	else if (g_rot_smc_count == 6 && g_vb_version == 2)
		rot_write_enable = 0;

	return 0;
}

/*******************************************************************************
 * SMC call from LK TEE OS to retrieve the root of trust info
 ******************************************************************************/
static uint32_t *p_root_of_trust_info_head = (uint32_t *)(uintptr_t) &g_rot_info;
static uint32_t *p_root_of_trust_info_pos = (uint32_t *)(uintptr_t) &g_rot_info;

uint64_t sip_get_root_of_trust_info(uint32_t *arg0, uint32_t *arg1,
	uint32_t *arg2, uint32_t *arg3)
{
	CASSERT(sizeof(struct root_of_trust_info_t) == ROOT_OF_TRUST_SIZE, RoT_wrong_size);

	if (rot_write_enable)
		return -4;

	/* we use the same SMC call ID for getting whole structure data
	 * therefore, reset the pos if it reaches the end
	 */
	if (4*(p_root_of_trust_info_pos - p_root_of_trust_info_head) >= sizeof(struct root_of_trust_info_t))
		p_root_of_trust_info_pos = p_root_of_trust_info_head;

	/* get 16 bytes each time */
	*arg0 = *p_root_of_trust_info_pos;
	p_root_of_trust_info_pos++;
	*arg1 = *p_root_of_trust_info_pos;
	p_root_of_trust_info_pos++;
	*arg2 = *p_root_of_trust_info_pos;
	p_root_of_trust_info_pos++;
	*arg3 = *p_root_of_trust_info_pos;
	p_root_of_trust_info_pos++;

	INFO("ROT RET: 0x%x, 0x%x, 0x%x, 0x%x\n", *arg0, *arg1, *arg2, *arg3);

	return 0;
}
