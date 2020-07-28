#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <context_mgmt.h>
#include <debug.h>
#include <platform.h>
#include <tkcore.h>
#include "tkcored_private.h"

static void tkcored_cpu_on_finish_handler(uint64_t cookie)
{
	int32_t rc = 0;
	uint64_t mpidr = read_mpidr();
	uint32_t linear_id = platform_get_core_pos(mpidr);
	struct tkcore_context *tkcore_ctx = &tkcored_sp_context[linear_id];

	assert(tkcore_vectors);

	/* Initialise this cpu's secure context */
	tkcored_init_secure_context((uint64_t)&tkcore_vectors->cpu_on_entry,
				    tkcored_rw, mpidr, tkcore_ctx);

	/* Enter TKCORE */
	rc = tkcored_synchronous_sp_entry(tkcore_ctx);

	/*
	 * Read the response from TKCORE. A non-zero return means that
	 * something went wrong while communicating with TKCORE.
	 */
	if (rc != 0)
		panic();

	/* Update its context to reflect the state TKCORE is in */
	set_tkcore_pstate(tkcore_ctx->state, TKCORE_PSTATE_ON);
}

static int32_t tkcored_cpu_migrate_info(uint64_t *resident_cpu)
{
	return TKCORE_MIGRATE_INFO;
}

const spd_pm_ops_t tkcored_pm = {
	NULL,
	NULL,
	NULL,
	tkcored_cpu_on_finish_handler,
	NULL,
	NULL,
	tkcored_cpu_migrate_info
};

