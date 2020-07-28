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

unsigned int is_enable_4GB_mode(void)
{
	if(((mmio_read_32(0x10003208) & 0x8000) == 0) || ((mmio_read_32(0x10001f00) & 0x2000) == 0))
		return 0;
	else
		return 1;
}

/*
 * emi_mpu_set_region_protection: protect a region.
 * @start: start address of the region
 * @end: end address of the region
 * @region: EMI MPU region id
 * @access_permission: EMI MPU access permission
 * Return 0 for success, otherwise negative status code.
 */

static unsigned char R[EMIMPU_REGION_NUM];
uint32_t sip_emimpu_set_region_protection(unsigned long long start, unsigned long long end, unsigned int region_permission)
{

	int region;
	unsigned int access_permission, locked;
	unsigned long long emi_physical_offset;

	region = (region_permission >> 27) & 0x1F;
	access_permission = region_permission & 0xFFFFFF;
	locked = (region_permission >> 26) & 0x1;

	if((region < 0) || region >= EMIMPU_REGION_NUM)
		return SIP_SVC_E_INVALID_Range;

	if (R[region] == 1)
		return SIP_SVC_E_PERMISSION_DENY;
	else {
		if (locked)
			R[region] = 1;
	}

	if((end != 0) || (start != 0))
	{
		/* if not 4GB mode need offset 0x4000000 */
		if (is_enable_4GB_mode() == 0)
			emi_physical_offset = 0x40000000;
		else
			emi_physical_offset = 0;
		/* INFO("atf emi_physical_offset=%llx\n",emi_physical_offset); */
		/*Address 64KB alignment*/
		start -= emi_physical_offset;
		end -= emi_physical_offset;
		start = (start >> 16) & 0x1FFFF;
		end = (end >> 16) & 0x1FFFF;

		if (end < start)
		{
			return SIP_SVC_E_INVALID_Range;
		}
	}

	mmio_write_32(EMI_MPU_SA(region), start);
	mmio_write_32(EMI_MPU_EA(region), end);
	mmio_write_32(EMI_MPU_APC(region,0), access_permission);

	return SIP_SVC_E_SUCCESS;
}


void emimpu_setup(void) {
	//emimpu_set_security_access();
}

static int is_emi_mpu_reg_write_forbidden(unsigned int offset)
{
	if ((offset >= EMI_MPU_START) && (offset <= EMI_MPU_END))
		return 0;
	else
		return 1;
}


uint64_t sip_emimpu_write(unsigned int offset, unsigned int reg_value)
{
	return SIP_SVC_E_PERMISSION_DENY;
}

uint32_t sip_emimpu_read(unsigned int offset)
{
	unsigned int addr;

	if(is_emi_mpu_reg_write_forbidden(offset))
		return SIP_SVC_E_INVALID_Range;

	addr = EMI_MPU_BASE + offset;
	return mmio_read_32(addr);
}
