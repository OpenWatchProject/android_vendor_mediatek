#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <debug.h>
#include <emi_drv.h>
#include <interrupt_mgmt.h>
#include <mmio.h>
#include <mtk_sip_svc.h>
//#include <plat_config.h>
//#include <plat_def.h>
#include <plat_private.h>
#include <platform.h>
#include <platform_def.h>
#include <stdint.h>
#include <stdio.h>

static unsigned char R[EMIMPU_REGION_NUM];

uint32_t sip_emimpu_set_region_protection(unsigned int start, unsigned int end, unsigned int region_permission)
{

	unsigned int region;
	unsigned int apc0;
	unsigned int locked;

	region = (region_permission >> 27) & 0x1F;
	apc0 = region_permission & 0xFFFFFF;
	locked = (region_permission >> 26) & 0x1;

	if (region >= EMIMPU_REGION_NUM)
		return MTK_SIP_E_INVALID_RANGE;

	if (R[region] == 1)
		return MTK_SIP_E_PERMISSION_DENY;
	else {
		if (locked)
			R[region] = 1;
	}

	if((end != 0) || (start != 0))
	{
		/* DRAM offset 0x40000000 */
		start -= 0x4000;
		end -= 0x4000;

		if (end < start)
			return MTK_SIP_E_INVALID_RANGE;
	}

	mmio_write_32(EMI_MPU_SA(region), start);
	mmio_write_32(EMI_MPU_EA(region), end);
	mmio_write_32(EMI_MPU_APC(region, 0), apc0);

	return MTK_SIP_E_SUCCESS;
}


static int is_reg_access_forbidden(unsigned int offset)
{
	if ((offset >= EMI_MPU_START) && (offset <= EMI_MPU_END))
		return 0;
	else
		return 1;
}


uint64_t sip_emimpu_write(unsigned int offset, unsigned int reg_value)
{
	return MTK_SIP_E_PERMISSION_DENY;
}

uint32_t sip_emimpu_read(unsigned int offset)
{
	unsigned int addr;

	if(is_reg_access_forbidden(offset))
		return MTK_SIP_E_INVALID_RANGE;

	addr = EMI_MPU_BASE + offset;
	return mmio_read_32(addr);
}

/*
 * emi_mpu_set_region_protection: protect a region.
 * @start: start address of the region
 * @end: end address of the region
 * @region: EMI MPU region id
 * @access_permission: EMI MPU access permission
 * Return 0 for success, otherwise negative status code.
 */

uint32_t emi_mpu_set_region_protection(unsigned long long start,
	unsigned long long end, int region, unsigned int access_permission)
{
	int ret = 0;
	unsigned int start_align, end_align;

	access_permission = access_permission | ((region & 0x1F) << 27);
	start_align = (unsigned int) (start >> 16);
	end_align = (unsigned int) (end >> 16);

	sip_emimpu_set_region_protection(start_align, end_align, access_permission);

	return ret;
}

