/****************************************************************************
 *
 * Copyright (c) 2015-2017 MICROTRUST Incorporated
 * All rights reserved
 *
 * This file and software is confidential and proprietary to MICROTRUST Inc.
 * Unauthorized copying of this file and software is strictly prohibited.
 * You MUST NOT disclose this file and software unless you get a license
 * agreement from MICROTRUST Incorporated.
 *
 ***************************************************************************/

#ifndef __PLATFORM_DEF_TEEI_H__
#define __PLATFORM_DEF_TEEI_H__

#include "devapc.h"
#include <platform.h>
#include <plat_private.h>

#define INTR_OFFSET 0

#define NSEC_CALL_INTR		(284+INTR_OFFSET)
#define NSEC_INTSCHED_INTR	(285+INTR_OFFSET)

#define NSEC_APP_INTR		282
#define NSEC_BOOT_INTR		283
#define NSEC_SCHED_INTR		284
#define NSEC_DRV_INTR		287
#define NSEC_RDRV_INTR		278
#define NSEC_LOG_INTR		277
#define NSEC_ERR_INTR		276
#define SEC_TUI_CANCEL		275

#define TEEI_BOOT_PARAMS (((atf_arg_t_ptr)(uintptr_t)TEE_BOOT_INFO_ADDR)->tee_boot_arg_addr)
/* Move key param from (base + 0x200) to (base + 0x300)
 * secure isram base            : atf param
 * secure isram base + 0x100    : teei boot param (TEEI_BOOT_PARAMS)
 * secure isram base + 0x200    : secure memory param
 * secure isram base + 0x300    : teei key param (TEEI_BOOT_PARAMS + 0x200)
 */
#define TEEI_SECURE_PARAMS (TEEI_BOOT_PARAMS+0x200)

/* this should be checked and changed by platform */
#define TEEI_SPI0_APC_NUM	(72)
#define TEEI_SPI1_APC_NUM	(78)
#define TEEI_USED_SPI_NUM	TEEI_SPI0_APC_NUM

#endif
