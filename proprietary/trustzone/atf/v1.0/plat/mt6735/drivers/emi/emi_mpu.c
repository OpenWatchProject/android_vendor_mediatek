#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <debug.h>
#include <interrupt_mgmt.h>
#include <platform.h>
#include <plat_config.h>
#include <stdint.h>
#include "plat_def.h"
#include "plat_private.h"
#include <platform_def.h>
#include <stdio.h>  //for printf
#include <emi_drv.h>
#include <sip_error.h>
#include <mmio.h>

#if 0
#define readl(addr) (__raw_readl(addr))
#define writel(b,addr) __raw_writel(b,addr)
#define IOMEM(reg) (reg)
/*
 * emi_mpu_set_region_protection: protect a region.
 * @start: start address of the region
 * @end: end address of the region
 * @region: EMI MPU region id
 * @access_permission: EMI MPU access permission
 * Return 0 for success, otherwise negative status code.
 */
int emi_mpu_set_region_protection(unsigned int start, unsigned int end, int region, unsigned int access_permission)
{
	int ret = 0;
	unsigned int tmp, tmp2;
	unsigned int ax_pm, ax_pm2;

	if((end != 0) || (start !=0))
	{
		/*Address 64KB alignment*/
		start -= EMI_PHY_OFFSET;
		end -= EMI_PHY_OFFSET;
		start = start >> 16;
		end = end >> 16;

		if (end <= start)
		{
			return -1;
		}
	}

	ax_pm  = (access_permission << 16) >> 16;
	ax_pm2 = (access_permission >> 16);

	switch (region) {
	case 0:
		tmp = readl(IOMEM(EMI_MPUI)) & 0xFFFF0000;
		tmp2 = readl(IOMEM(EMI_MPUI_2ND)) & 0xFFFF0000;
		writel(0, EMI_MPUI);
		writel(0, EMI_MPUI_2ND);
		writel((start << 16) | end, EMI_MPUA);
		writel(tmp2 | ax_pm2, EMI_MPUI_2ND);
		writel(tmp | ax_pm, EMI_MPUI);
		break;

	case 1:
		tmp = readl(IOMEM(EMI_MPUI)) & 0x0000FFFF;
		tmp2 = readl(IOMEM(EMI_MPUI_2ND)) & 0x0000FFFF;
		writel(0, EMI_MPUI);
		writel(0, EMI_MPUI_2ND);
		writel((start << 16) | end, EMI_MPUB);
		writel(tmp2 | (ax_pm2 << 16), EMI_MPUI_2ND);
		writel(tmp | (ax_pm << 16), EMI_MPUI);
		break;

	case 2:
		tmp = readl(IOMEM(EMI_MPUJ)) & 0xFFFF0000;
		tmp2 = readl(IOMEM(EMI_MPUJ_2ND)) & 0xFFFF0000;
		writel(0, EMI_MPUJ);
		writel(0, EMI_MPUJ_2ND);
		writel((start << 16) | end, EMI_MPUC);
		writel(tmp2 | ax_pm2, EMI_MPUJ_2ND);
		writel(tmp | ax_pm, EMI_MPUJ);
		break;

	case 3:
		tmp = readl(IOMEM(EMI_MPUJ)) & 0x0000FFFF;
		tmp2 = readl(IOMEM(EMI_MPUJ_2ND)) & 0x0000FFFF;
		writel(0, EMI_MPUJ);
		writel(0, EMI_MPUJ_2ND);
		writel((start << 16) | end, EMI_MPUD);
		writel(tmp2 | (ax_pm2 << 16), EMI_MPUJ_2ND);
		writel(tmp | (ax_pm << 16), EMI_MPUJ);
		break;

	case 4:
		tmp = readl(IOMEM(EMI_MPUK)) & 0xFFFF0000;
		tmp2 = readl(IOMEM(EMI_MPUK_2ND)) & 0xFFFF0000;
		writel(0, EMI_MPUK);
		writel(0, EMI_MPUK_2ND);
		writel((start << 16) | end, EMI_MPUE);
		writel(tmp2 | ax_pm2, EMI_MPUK_2ND);
		writel(tmp | ax_pm, EMI_MPUK);
		break;

	case 5:
		tmp = readl(IOMEM(EMI_MPUK)) & 0x0000FFFF;
		tmp2 = readl(IOMEM(EMI_MPUK_2ND)) & 0x0000FFFF;
		writel(0, EMI_MPUK);
		writel(0, EMI_MPUK_2ND);
		writel((start << 16) | end, EMI_MPUF);
		writel(tmp2 | (ax_pm2 << 16), EMI_MPUK_2ND);
		writel(tmp | (ax_pm << 16), EMI_MPUK);
		break;

	case 6:
		tmp = readl(IOMEM(EMI_MPUL)) & 0xFFFF0000;
		tmp2 = readl(IOMEM(EMI_MPUL_2ND)) & 0xFFFF0000;
		writel(0, EMI_MPUL);
		writel(0, EMI_MPUL_2ND);
		writel((start << 16) | end, EMI_MPUG);
		writel(tmp2 | ax_pm2, EMI_MPUL_2ND);
		writel(tmp | ax_pm, EMI_MPUL);
		break;

	case 7:
		tmp = readl(IOMEM(EMI_MPUL)) & 0x0000FFFF;
		tmp2 = readl(IOMEM(EMI_MPUL_2ND)) & 0x0000FFFF;
		writel(0, EMI_MPUL);
		writel(0, EMI_MPUL_2ND);
		writel((start << 16) | end, EMI_MPUH);
		writel(tmp2 | (ax_pm2 << 16), EMI_MPUL_2ND);
		writel(tmp | (ax_pm << 16), EMI_MPUL);
		break;

#if defined(MACH_TYPE_MT6735) || defined(MACH_TYPE_MT6753)
	case 8:
		tmp = readl(IOMEM(EMI_MPUI2)) & 0xFFFF0000;
		tmp2 = readl(IOMEM(EMI_MPUI2_2ND)) & 0xFFFF0000;
		writel(0, EMI_MPUI2);
		writel(0, EMI_MPUI2_2ND);
		writel((start << 16) | end, EMI_MPUA2);
		writel(tmp2 | ax_pm2, EMI_MPUI2_2ND);
		writel(tmp | ax_pm, EMI_MPUI2);
		break;

	case 9:
		tmp = readl(IOMEM(EMI_MPUI2)) & 0x0000FFFF;
		tmp2 = readl(IOMEM(EMI_MPUI2_2ND)) & 0x0000FFFF;
		writel(0, EMI_MPUI2);
		writel(0, EMI_MPUI2_2ND);
		writel((start << 16) | end, EMI_MPUB2);
		writel(tmp2 | (ax_pm2 << 16), EMI_MPUI2_2ND);
		writel(tmp | (ax_pm << 16), EMI_MPUI2);
		break;

	case 10:
		tmp = readl(IOMEM(EMI_MPUJ2)) & 0xFFFF0000;
		tmp2 = readl(IOMEM(EMI_MPUJ2_2ND)) & 0xFFFF0000;
		writel(0, EMI_MPUJ2);
		writel(0, EMI_MPUJ2_2ND);
		writel((start << 16) | end, EMI_MPUC2);
		writel(tmp2 | ax_pm2, EMI_MPUJ2_2ND);
		writel(tmp | ax_pm, EMI_MPUJ2);
		break;

	case 11:
		tmp = readl(IOMEM(EMI_MPUJ2)) & 0x0000FFFF;
		tmp2 = readl(IOMEM(EMI_MPUJ2_2ND)) & 0x0000FFFF;
		writel(0, EMI_MPUJ2);
		writel(0, EMI_MPUJ2_2ND);
		writel((start << 16) | end, EMI_MPUD2);
		writel(tmp2 | (ax_pm2 << 16), EMI_MPUJ2_2ND);
		writel(tmp | (ax_pm << 16), EMI_MPUJ2);
		break;

	case 12:
		tmp = readl(IOMEM(EMI_MPUK2)) & 0xFFFF0000;
		tmp2 = readl(IOMEM(EMI_MPUK2_2ND)) & 0xFFFF0000;
		writel(0, EMI_MPUK2);
		writel(0, EMI_MPUK2_2ND);
		writel((start << 16) | end, EMI_MPUE2);
		writel(tmp2 | ax_pm2, EMI_MPUK2_2ND);
		writel(tmp | ax_pm, EMI_MPUK2);
		break;

	case 13:
		tmp = readl(IOMEM(EMI_MPUK2)) & 0x0000FFFF;
		tmp2 = readl(IOMEM(EMI_MPUK2_2ND)) & 0x0000FFFF;
		writel(0, EMI_MPUK2);
		writel(0, EMI_MPUK2_2ND);
		writel((start << 16) | end, EMI_MPUF2);
		writel(tmp2 | (ax_pm2 << 16), EMI_MPUK2_2ND);
		writel(tmp | (ax_pm << 16), EMI_MPUK2);
		break;

	case 14:
		tmp = readl(IOMEM(EMI_MPUL2)) & 0xFFFF0000;
		tmp2 = readl(IOMEM(EMI_MPUL2_2ND)) & 0xFFFF0000;
		writel(0, EMI_MPUL2);
		writel(0, EMI_MPUL2_2ND);
		writel((start << 16) | end, EMI_MPUG2);
		writel(tmp2 | ax_pm2, EMI_MPUL2_2ND);
		writel(tmp | ax_pm, EMI_MPUL2);
		break;

	case 15:
		tmp = readl(IOMEM(EMI_MPUL2)) & 0x0000FFFF;
		tmp2 = readl(IOMEM(EMI_MPUL2_2ND)) & 0x0000FFFF;
		writel(0, EMI_MPUL2);
		writel(0, EMI_MPUL2_2ND);
		writel((start << 16) | end, EMI_MPUH2);
		writel(tmp2 | (ax_pm2 << 16), EMI_MPUL2_2ND);
		writel(tmp | (ax_pm << 16), EMI_MPUL2);
		break;
#endif

	default:
		ret = -1;
		break;
	}

	return ret;
}
#endif



void emimpu_set_domain_secure_access(int domain, int sec_access)
{
	unsigned int d_ctrl_offset[]= {
		EMI_MPUM,  /* domain 0 */
		EMI_MPUN,  /* domain 1 */
		EMI_MPUO,  /* domain 2 */
		EMI_MPUU,  /* domain 3 */
#if defined(MACH_TYPE_MT6735) || defined(MACH_TYPE_MT6753)
		EMI_MPUM2, /* domain 4 */
		EMI_MPUN2, /* domain 5 */
		EMI_MPUO2, /* domain 6 */
		EMI_MPUU2, /* domain 7 */
#endif
	};
	unsigned int addr, value;

	if(domain < EMIMPU_DOMAIN_NUM)
	{
		addr = EMI_BASE + d_ctrl_offset[domain];
		value = mmio_read_32(addr);

		if(sec_access)
			value |= (0x1 << EMIMPU_DX_SEC_BIT);
		else
			value &= ~(0x1 << EMIMPU_DX_SEC_BIT);
	}

}

void emimpu_set_security_access(void)
{
	int domain;

	for(domain = 0; domain < EMIMPU_DOMAIN_NUM; domain++)
	{
		emimpu_set_domain_secure_access(domain, 1);
	}
}

void emimpu_setup(void) {
	emimpu_set_security_access();
}

uint64_t sip_emimpu_write(unsigned int offset, unsigned int reg_value)
{
	return SIP_SVC_E_PERMISSION_DENY;
}

uint32_t sip_emimpu_read(unsigned int offset)
{
	unsigned int addr;

	if ((offset >= EMI_CONA) && (offset <= EMI_RFCD))
		addr = EMI_BASE + offset;
	else
		return SIP_SVC_E_INVALID_Range;

	return mmio_read_32(addr);
}
void Debug_log_EMI_MPU(void)
{
	printf("ATF EMI MPUS=0x%x; MPUT=0x%x\n", mmio_read_32(0x102031F0), mmio_read_32(0x102031F8));
}
