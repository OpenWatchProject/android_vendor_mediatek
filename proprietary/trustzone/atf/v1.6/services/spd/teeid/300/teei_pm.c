/*
 * Copyright (c) 2015-2018 MICROTRUST Incorporated
 * All rights reserved
 *
 * This file and software is confidential and proprietary to MICROTRUST Inc.
 * Unauthorized copying of this file and software is strictly prohibited.
 * You MUST NOT disclose this file and software unless you get a license
 * agreement from MICROTRUST Incorporated.
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <arch_helpers.h>
#include <console.h>
#include <platform.h>
#include <context_mgmt.h>
#include <runtime_svc.h>
#include <bl31.h>
#include "teei_private.h"
#include "teei_atf.h"

#ifdef TEEI_PM_ENABLE

#include <psci.h>


static void save_sysregs_core(uint64_t fromCoreNro, uint32_t toCoreNro)
{
	if (fromCoreNro != toCoreNro) {
		cpu_context_t *cpu_ctx = &secure_context[fromCoreNro].cpu_ctx;

		memcpy(&secure_context[toCoreNro].cpu_ctx, cpu_ctx, sizeof(cpu_context_t));
	}
}
void teei_init_core(uint64_t mpidr)
{
	uint32_t linear_id = platform_get_core_pos(mpidr);
	struct teei_context *teei_ctx = &secure_context[linear_id];

	if (mpidr == teeiBootCoreMpidr)
		return;

	teei_init_secure_context(teei_ctx);
	uint32_t boot_core_nro = platform_get_core_pos(teeiBootCoreMpidr);

	save_sysregs_core(boot_core_nro, linear_id);
}
/*******************************************************************************
 * This cpu has been turned on.
 ******************************************************************************/
static void teei_cpu_on_finish_handler(u_register_t cookie)
{

	uint64_t mpidr = read_mpidr();

	/*uint32_t linear_id = platform_get_core_pos(mpidr);*/
	/*teei_context *teei_ctx = &secure_context[linear_id];*/
	teei_init_core(mpidr);
}

/*******************************************************************************
 * Structure populated by the TEEI Dispatcher to be given a chance to perform any
 * TEEI bookkeeping before PSCI executes a power mgmt. operation.
 ******************************************************************************/
const spd_pm_ops_t teei_pm = {
	NULL,
	NULL,
	NULL,
	teei_cpu_on_finish_handler,
	NULL,
	NULL,
	NULL
};

#endif

