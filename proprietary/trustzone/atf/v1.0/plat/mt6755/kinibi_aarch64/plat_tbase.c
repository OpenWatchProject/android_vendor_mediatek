/*
 * Copyright (c) 2014 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

#include <arch_helpers.h>
#include <assert.h>
#include <debug.h>
#include <gic_v2.h>
#include <gic_v3.h>
#include <interrupt_mgmt.h>
#include <mmio.h>
#include "plat_private.h"
#include <platform.h>
#include <plat_def.h>
#include <string.h>
#include <tbase_private.h>
#include <tzc400.h>
#include <rng.h>
#include <plat_config.h>
#include <plat_tbase.h>

#define FIRMWARE_VERSION        "N/A"
#define FIRMWARE_BINARY_VERSION "N/A"
#define TEE_BOOT_INFO_ADDR (&gteearg)

#define PLAT_TBASE_INPUT_OK		((uint32_t)0)
#define PLAT_TBASE_INPUT_ERROR		((uint32_t)-1)

/*Forward declarations*/
static void coreDump(uint32_t coreId);
static void platformDump(void);
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
		atf_arg_t *teearg = (atf_arg_t *)(uintptr_t)TEE_BOOT_INFO_ADDR;
		/* Maybe we can move this to a memcpy instead */
		for (idx = 0; idx < (*Length / sizeof(uint32_t)) &&
			idx < (sizeof(teearg->hwuid) / sizeof(uint32_t)); idx++) {
			((uint32_t *)out)[idx] = teearg->hwuid[idx];
			length += sizeof(uint32_t);
		}

		break;
	}
	case PLAT_TBASE_INPUT_HWKEY: {
		atf_arg_t *teearg = (atf_arg_t *)(uintptr_t)TEE_BOOT_INFO_ADDR;
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

uint32_t plat_tbase_dump(void)
{
	uint32_t linear_id = platform_get_core_pos(read_mpidr());

	printf("core %d is dumped !\n", (int)linear_id);

	aee_wdt_dump();

	return 0;
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

/* <t-base handler for EL1->EL3 forwarded FIQs */
uint32_t plat_tbase_forward_fiq(uint32_t fiqId)
{
	uint32_t Status = PLAT_TBASE_INPUT_OK;
	uint32_t linear_id = platform_get_core_pos(read_mpidr());

	/* Verbosity */
	printf("core %d EL3 received forwarded FIQ %d from <t-base !\n", (int)linear_id,
              (int) fiqId);

	/* Handle forwarded FIQ */
	switch (fiqId) {
	case WDT_IRQ_BIT_ID:
		/* FIQ dump */
		platformDump();
		break;
	case 161:
		/* just a simple test */
		printf("%s: That's a test !\n", __func__);
		break;
	default:
		/* Unknown FIQ */

		printf("%s: FIQ %d was forwarded but no processing was associated to it.\n",
                       __func__, fiqId);
		Status = PLAT_TBASE_INPUT_ERROR;
		break;
	}


	return Status;
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

/* Private code */
static void coreDump(uint32_t coreId)
{
	uint64_t mpidr = read_mpidr();
	uint32_t linear_id = platform_get_core_pos(mpidr);

	uint32_t SGITarget;

	assert(coreId < PLATFORM_CORE_COUNT);

	/* Check if target core is the one we're currently running on */
	if (coreId == linear_id) {
		/* Current core : let's call the handler directly */
		plat_tbase_dump();
	} else {
		/* Other cores : let's do the same using SGIs */

		/* Configure SGI */
		gicd_clr_igroupr(get_plat_config()->gicd_base, FIQ_SMP_CALL_SGI);
		gicd_set_ipriorityr(get_plat_config()->gicd_base, FIQ_SMP_CALL_SGI, GIC_HIGHEST_SEC_PRIORITY);

		/* Enable SGI */
		gicd_set_isenabler(get_plat_config()->gicd_base, FIQ_SMP_CALL_SGI);

		/* Send SGI to this specific core */
		SGITarget = 1 << coreId;

		/* Trigger SGI */
		irq_raise_softirq(SGITarget, FIQ_SMP_CALL_SGI);
	}
}

static void platformDump(void)
{
	uint64_t mpidr = read_mpidr();
	uint32_t linear_id = platform_get_core_pos(mpidr);
	uint32_t i;

	/* Dump all cores, one by one */
	for (i = 0 ; i < PLATFORM_CORE_COUNT ; i++) {
		/* Do not dump current (primary) core, we'll do it after at last */
		if (i != linear_id) {
			/* Dump core 'i' */
			coreDump(i);
		}
	}

	/* All secondary cores have been dumped, let's dump primary core */
	coreDump(linear_id);
}
