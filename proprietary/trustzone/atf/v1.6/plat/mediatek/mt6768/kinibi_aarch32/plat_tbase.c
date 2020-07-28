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
#include <mtk_aee_debug.h>
#include "plat_private.h"
#include <platform.h>
#include <platform_def.h>
#include <string.h>
#include <tbase_private.h>
#include <tzc400.h>

/* Defines */
#define PLAT_TBASE_INPUT_HWIDENTITY	(0x1)
#define PLAT_TBASE_INPUT_HWKEY		(0x2)
#define PLAT_TBASE_INPUT_RNG		(0x3)

#define PLAT_TBASE_INPUT_OK		((uint32_t)0)
#define PLAT_TBASE_INPUT_ERROR		((uint32_t)-1)

/* Code */
/* <t-base handler for SWd fastcall INPUT */
uint32_t plat_tbase_input(uint64_t DataId, u_register_t *Length, void *out)
{
	uint32_t Status = PLAT_TBASE_INPUT_OK;
	uint32_t length = 0, idx = 0;

	/* Implement associated actions */
	switch (DataId) {
	case PLAT_TBASE_INPUT_HWIDENTITY: {
		struct atf_arg_t *teearg = &gteearg;

		NOTICE("PLAT_TBASE_INPUT_HWIDENTITY\n");
		/* Maybe we can move this to a memcpy instead */
		for (idx = 0; idx < (*Length / sizeof(uint32_t)) &&
		     idx < (sizeof(teearg->hwuid) / sizeof(uint32_t)); idx++) {
			((uint32_t *)out)[idx] = teearg->hwuid[idx];
			length += sizeof(uint32_t);
		}
		break;
	}
	case PLAT_TBASE_INPUT_HWKEY: {
		struct atf_arg_t *teearg = &gteearg;

		NOTICE("PLAT_TBASE_INPUT_HWKEY\n");
		/* Maybe we can move this to a memcpy instead */
		for (idx = 0; idx < (*Length / sizeof(uint32_t)) &&
		     idx < (sizeof(teearg->HRID) / sizeof(uint32_t)); idx++) {
			NOTICE("HRID%d:0x%x\n", idx, teearg->HRID[idx]);
			((uint32_t *)out)[idx] = teearg->HRID[idx];
			length += sizeof(uint32_t);
		}
		*Length = length;
		NOTICE("Length: %lu\n", *Length);
		break;
	}
	case PLAT_TBASE_INPUT_RNG: {
		uint32_t value = 0;
		uint32_t ready = 0;

		NOTICE("PLAT_TBASE_INPUT_RNG\n");
		mmio_write_32(TRNG_PDN_CLR, TRNG_PDN_VALUE);
		value = mmio_read_32(TRNG_CTRL);
		value |= TRNG_CTRL_START;
		((uint32_t *)out)[0] = mmio_read_32(TRNG_DATA);
		mmio_write_32(TRNG_CTRL, value);

		/* wait until ready bit is raised */
		do {
			ready = mmio_read_32(TRNG_CTRL) & TRNG_CTRL_RDY;
		} while (ready == 0);

		((uint32_t *)out)[0] = mmio_read_32(TRNG_DATA);
		value = mmio_read_32(TRNG_CTRL);
		value &= ~TRNG_CTRL_START;
		mmio_write_32(TRNG_CTRL, value);
		mmio_write_32(TRNG_PDN_SET, TRNG_PDN_VALUE);
		*Length = 4;
		break;
	}
	default:
		/* Unsupported request */
		Status = PLAT_TBASE_INPUT_ERROR;
		break;
	}
	return Status;
}

void plat_tbase_fiqforward_init(void)
{
	NOTICE("Configuring t-base forwarded FIQs...\n");
	/* Watchdog FIQ configuration */
	tbase_fiqforward_configure(WDT_IRQ_BIT_ID,
				   TBASE_FLAG_SET); /* enable forward */
	/*tbase_fiqforward_configure( FIQ_SMP_CALL_SGI, */   /* interrupt id */
	/*		TBASE_FLAG_SET ); */ /* enable forward */
	/* Another forwarded FIQ, just for testing purpose */
	/*tbase_fiqforward_configure( 161, */             /* interrupt id */
	/*				TBASE_FLAG_SET); */ /* enable forward */
	tbase_fiqforward_configure(PCCIF1_IRQ0_BIT_ID,
				   TBASE_FLAG_SET); /* enable forward */
	tbase_fiqforward_configure(PCCIF1_IRQ1_BIT_ID,
				   TBASE_FLAG_SET); /* enable forward */
}

