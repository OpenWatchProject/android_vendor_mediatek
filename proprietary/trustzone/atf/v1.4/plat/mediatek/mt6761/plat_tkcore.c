#include <assert.h>
#include <debug.h>
#include <string.h>
#include <stdio.h>
#include <platform.h>
#include <mmio.h>
#include <platform_def.h>
#include <console.h>
#include "plat_private.h"
#include "tkcore.h"

#include <arch_helpers.h>

#include <rng.h>

/* Code */
uint32_t plat_tkcore_secure_service_request(uint64_t svc_id,
		uint64_t *p1, uint64_t *p2)
{
	uint32_t rc;
	uint32_t linear_id = platform_get_core_pos(read_mpidr());

	(void) p1;
	(void) p2;
	(void) linear_id;

	switch (svc_id) {
	case SERVICE_TEST:
		rc = 0;
		break;
	case SERVICE_TRNG:
		*p1 = 0;
		rc = plat_get_rnd((uint32_t *) p1);
		break;
	default:
		rc = SERVICE_ERROR;
	}

	return rc;
}
