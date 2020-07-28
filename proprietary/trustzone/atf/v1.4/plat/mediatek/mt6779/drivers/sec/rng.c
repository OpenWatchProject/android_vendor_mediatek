#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <console.h>
#include <debug.h>
#include <interrupt_mgmt.h>
#include <mmio.h>
#include <mtk_sip_svc.h>
#include <plat_private.h>
#include <platform_def.h>
#include <platform.h>
#include <stdint.h>
#include <string.h>
#include <xlat_tables_v2.h>
#include <rng.h>

uint32_t plat_get_true_rnd(uint32_t *val)
{
	uint32_t ret = 0;
	uint32_t value = 0;

	if (val == NULL)
		return MTK_SIP_E_INVALID_PARAM;

	/* ungate */
	mmio_write_32(TRNG_PDN_CLR, TRNG_PDN_VALUE);

	/* read random data once and drop it */
	((uint32_t *)val)[0] = mmio_read_32(TRNG_DATA);

	/* start */
	value = mmio_read_32(TRNG_CTRL);
	value |= TRNG_CTRL_START;
	mmio_write_32(TRNG_CTRL, value);

	/* get random data from trng */
	while (1) {
		value = mmio_read_32(TRNG_CTRL);
		if (value & TRNG_CTRL_RDY)
			break;
	}

	((uint32_t *)val)[0] = mmio_read_32(TRNG_DATA);

	/* stop */
	value = mmio_read_32(TRNG_CTRL);
	value &= ~TRNG_CTRL_START;
	mmio_write_32(TRNG_CTRL, value);

	/* gate */
	mmio_write_32(TRNG_PDN_SET, TRNG_PDN_VALUE);

	return ret;
}

uint32_t plat_get_rnd(uint32_t *val)
{
	uint32_t ret = 0;

	if (val == NULL)
		return MTK_SIP_E_INVALID_PARAM;
	/* we call trng directly in current implementation */
	/* if we need a lot of random numbers, we should avoid
	 * using trng direclty. Instead, we should use trng only to
	 * get seed to prng, and use prng to get random number and
	 * do re-seeding when a specified amount of samples are collected,
	 * say 1000.
	 */
	ret = plat_get_true_rnd(val);

	return ret;
}

