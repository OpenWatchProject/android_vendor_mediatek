/*
 * Copyright (c) 2016-2018 TRUSTONIC LIMITED
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the TRUSTONIC LIMITED nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <platform.h>
#include <mmio.h>
#include <arch_helpers.h>

#include <tbase_private.h>
#include <context_mgmt.h>
#include <runtime_svc.h>
#include <gic_v2.h>
#include <mtk_plat_common.h>

#include "tbase_private.h"
#include <plat_tbase.h>
#include <rng.h>

#define FIRMWARE_VERSION        "N/A"
#define FIRMWARE_BINARY_VERSION "N/A"
#define TEE_BOOT_INFO_ADDR (&gteearg)

/*
 * Commands issued by Trustonic TEE that must be implemented by the integrator.
 *
 * param DataId: the command (see below for details)
 * param Length: Size of the requested information (passed by Kinibi and already set to
 *     the expected size, 16 bytes).
 * param Out: Buffer which will contain the requested information, already allocated.
 *
 * return PLAT_TBASE_INPUT_OK if success.
 *        PLAT_TBASE_INPUT_ERROR in case of error.
 *
 * Supported command:
 *   PLAT_TBASE_INPUT_HWIDENTITY, request to get the HW unique key
 *       (could be public, but must be cryptographically unique).
 *   PLAT_TBASE_INPUT_HWKEY, request to get the HW key (must be secret,
 *       better for security if unique but not critical)
 *   PLAT_TBASE_INPUT_RNG, request to get random number
 *       (used as seed in Trustonic TEE)
 */
uint32_t plat_tbase_input(uint64_t DataId, uint64_t *Length, void *out)
{
	uint32_t Status = PLAT_TBASE_INPUT_OK;
	uint32_t length = 0, idx = 0;

	/* Implement associated actions */
	switch (DataId) {
	case PLAT_TBASE_INPUT_HWIDENTITY: {
		struct atf_arg_t *teearg = (struct atf_arg_t *)(uintptr_t)TEE_BOOT_INFO_ADDR;
		/* Maybe we can move this to a memcpy instead */
		for (idx = 0; idx < (*Length / sizeof(uint32_t)) &&
		     idx < (sizeof(teearg->hwuid) / sizeof(uint32_t)); idx++) {
			((uint32_t *)out)[idx] = teearg->hwuid[idx];
			length += sizeof(uint32_t);
		}

		break;
	}
	case PLAT_TBASE_INPUT_HWKEY: {
		struct atf_arg_t *teearg = (struct atf_arg_t *)(uintptr_t)TEE_BOOT_INFO_ADDR;
		/* Maybe we can move this to a memcpy instead */
		for (idx = 0; idx < (*Length / sizeof(uint32_t)) &&
		     idx < (sizeof(teearg->HRID) / sizeof(uint32_t)) ; idx++) {
			((uint32_t *)out)[idx] = teearg->HRID[idx];
			length += sizeof(uint32_t);
		}
		*Length = length;
		break;
	}
	case PLAT_TBASE_INPUT_RNG: {
		NOTICE("PLAT_TBASE_INPUT_RNG\n");
		Status = plat_get_rnd((uint32_t *)out);
		*Length = 4;
		break;
	}
	case PLAT_TBASE_INPUT_FIRMWARE_VERSION: {
		memcpy(out, FIRMWARE_VERSION, sizeof(FIRMWARE_VERSION));
		*Length = sizeof(FIRMWARE_VERSION);
		break;
	}

	case PLAT_TBASE_INPUT_FIRMWARE_BINARY_VERSION: {
		memcpy(out, FIRMWARE_BINARY_VERSION, sizeof(FIRMWARE_BINARY_VERSION));
		*Length = sizeof(FIRMWARE_BINARY_VERSION);
		break;
	}
	default:
		/* Unsupported request */
		Status = PLAT_TBASE_INPUT_ERROR;
		break;
	}

	return Status;
}



/*
 * Abstraction API that must be customized by the integrator if "FIQ Forward" feature is supported.
 * Else, Implementation can remain empty.
 */
void plat_tbase_fiqforward_init(void)
{
	DBG_PRINTF("Configuring Kinibi forwarded FIQs...\n");

	/* Watchdog FIQ configuration */
	tbase_fiqforward_configure(WDT_IRQ_BIT_ID,    /* interrupt id */
				   TBASE_FLAG_SET);  /* enable forward */
}

void plat_blacklist_memory_init(void)
{
	/* Example code to blacklist 1 4K page at address 0x1000000, unblacklist it
	 * and then blacklist 5 pages.
		fc_response_t resp;
		tbase_monitor_fastcall(TEE_SMC_FASTCALL_BLACKLIST_ADD,
		0x1000000,
		0x1000,
		0x1234,
		0x0,
		&resp );
		tbase_monitor_fastcall(TEE_SMC_FASTCALL_BLACKLIST_RM,
		0x1000000,
		0x0,
		0x1234,
		0x0,
		&resp );
		tbase_monitor_fastcall(TEE_SMC_FASTCALL_BLACKLIST_ADD,
		0x1000000,
		0x5000,
		0xABBA,
		0x0,
		&resp );
	*/
}

