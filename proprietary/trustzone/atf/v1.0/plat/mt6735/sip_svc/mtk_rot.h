/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

extern uint8_t rot_write_enable;

/*******************************************************************************
 * SMC call from LK to receive the root of trust info
 ******************************************************************************/
uint64_t sip_save_root_of_trust_info(uint32_t arg0, uint32_t arg1,
	uint32_t arg2, uint32_t arg3);

/*******************************************************************************
 * SMC call from LK TEE OS to retrieve the root of trust info
 ******************************************************************************/
uint64_t sip_get_root_of_trust_info(uint32_t *arg0, uint32_t *arg1,
	uint32_t *arg2, uint32_t *arg3);
