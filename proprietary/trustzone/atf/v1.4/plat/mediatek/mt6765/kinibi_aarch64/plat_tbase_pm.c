/*
 * Copyright (c) 2016 TRUSTONIC LIMITED
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
#include <bl_common.h>
#include <assert.h>
#include <arch_helpers.h>
#include <console.h>
#include <platform.h>
#include <context_mgmt.h>
#include <runtime_svc.h>
#include <bl31.h>

#include <debug.h>

#include <psci.h>
#include "tbase_private.h"

/*******************************************************************************
 * The target cpu is being turned on.
 ******************************************************************************/
static void tbase_cpu_on_handler(uint64_t target_cpu)
{
	uint64_t mpidr = read_mpidr();
	uint32_t linear_id = platform_get_core_pos(mpidr);
	struct tbase_context *tbase_ctx = &secure_context[linear_id];

	/*TBASE_STATE_ON: set ready for tbase fiq/fastcall/smc handler*/
	tbase_ctx->state = TBASE_STATE_ON;
}

/*******************************************************************************
 * This cpu is being turned off.
 ******************************************************************************/
static int32_t tbase_cpu_off_handler(uint64_t cookie)
{
	uint64_t mpidr = read_mpidr();
	uint32_t linear_id = platform_get_core_pos(mpidr);
	struct tbase_context *tbase_ctx = &secure_context[linear_id];

	assert(tbase_ctx->state == TBASE_STATE_ON);

#if DEBUG
	/* printf("\r\ntbase_cpu_off_handler %d\r\n", linear_id); */
#endif

	tbase_ctx->state = TBASE_STATE_OFF;
	return 0;
}

/*******************************************************************************
 * This cpu is being suspended. S-EL1 state must have been saved in the
 * resident cpu (mpidr format), if any.
 ******************************************************************************/
static void tbase_cpu_suspend_handler(uint64_t power_state)
{
	uint64_t mpidr = read_mpidr();
	uint32_t linear_id = platform_get_core_pos(mpidr);
	struct tbase_context *tbase_ctx = &secure_context[linear_id];

	assert(tbase_ctx->state == TBASE_STATE_ON);

#if DEBUG
	/* printf("\r\ntbase_cpu_suspend_handler %d\r\n", linear_id); */
#endif

	tbase_ctx->state = TBASE_STATE_SUSPEND;
}

/*******************************************************************************
 * This cpu has been turned on.
 ******************************************************************************/
static void tbase_cpu_on_finish_handler(uint64_t cookie)
{
	uint64_t mpidr = read_mpidr();
	uint32_t linear_id = platform_get_core_pos(mpidr);
	struct tbase_context *tbase_ctx = &secure_context[linear_id];

	assert(tbase_ctx->state == TBASE_STATE_OFF);

	/* Core specific initialization; */
	tbase_init_core(mpidr);
#if DEBUG
	/* printf("\r\ntbase_cpu_on_finish_handler %d\r\n", linear_id); */
#endif

	/* TBASE_STATE_ON: set ready for tbase fiq/fastcall/smc handler */
	tbase_ctx->state = TBASE_STATE_ON;
}

/*******************************************************************************
 * This cpu has resumed from suspend.
 ******************************************************************************/
static void tbase_cpu_suspend_finish_handler(uint64_t suspend_level)
{
	uint64_t mpidr = read_mpidr();
	uint32_t linear_id = platform_get_core_pos(mpidr);
	struct tbase_context *tbase_ctx = &secure_context[linear_id];

	assert(tbase_ctx->state == TBASE_STATE_SUSPEND);
#if DEBUG
	/* printf("\r\ntbase_cpu_suspend_finish_handler %d\r\n", linear_id); */
#endif

	/* TBASE_STATE_ON: set ready for tbase fiq/fastcall/smc handler */
	tbase_ctx->state = TBASE_STATE_ON;
}

/*****************************************************************************
 * Report the current resident cpu (mpidr format) if it is a UP/UP migratable.
 * This function is supposed to be hooked in spd_pm_ops_t
 * Currently not supported through PCSI in Mediatek's EL3
 *****************************************************************************
 */
#if 0
static int32_t tbase_cpu_migrate_info(uint64_t *resident_cpu)
{
}
#endif

/*****************************************************************************
 * Migrate TBASE.
 * This function is supposed to be hooked in spd_pm_ops_t
 * Currently not supported through PCSI in Mediatek's EL3
 *****************************************************************************
 */
#if 0
static void tbase_migrate_handler(uint64_t x1, uint64_t par2)
{
}
#endif

/*******************************************************************************
 * Structure populated by the TBASE Dispatcher to be given a chance to perform any
 * TBASE bookkeeping before PSCI executes a power mgmt. operation.
 ******************************************************************************/
const spd_pm_ops_t tbase_pm = {
	tbase_cpu_on_handler,
	tbase_cpu_off_handler,
	tbase_cpu_suspend_handler,
	tbase_cpu_on_finish_handler,
	tbase_cpu_suspend_finish_handler,
	NULL,		/*tbase_migrate_handler not supported through PCSI*/
	NULL,		/*tbase_cpu_migrate_info not supported through PSCI*/
};

void register_spd_pm_hook(void)
{
	psci_register_spd_pm_hook(&tbase_pm);
}


