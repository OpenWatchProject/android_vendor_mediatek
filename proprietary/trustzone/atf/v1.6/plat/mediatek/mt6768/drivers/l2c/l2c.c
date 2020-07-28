#include <arch_helpers.h>
#include <arch.h>
#include <l2c.h>
#include <mcucfg.h>
#include <mmio.h>
#include <plat_private.h>
#include <platform_def.h>
#include <spinlock.h>

#define CLUSTERPWRCTLR_EL1 S3_0_C15_C3_5

void config_L2_size(void)
{
	/*
	 * L3 Cache Share Disable Flow:
	 * 1) Configured the “L3_SHARE_EN” to 0 for disabling the L3 Share function.
	 * 2) Executed DSB instruction.
	 * 3) Configured the “CLUSTERPWRCTLR[7:4]” register to “1111”.
	 * 4) Polling the “CLUSTERPWRSTAT[7:4]” register is equal to “1111”.
	 * 5) Configured the “L3_SHARE_PRE_EN” to 0 for releasing the SRAM power fixed.
	 * 5) Executed DSB instruction.
	 *
	 */

	unsigned long v = 0;
	/* clean l3_share_en */
	mmio_write_64_relax(MP0_CLUSTER_CFG0, mmio_read_64(MP0_CLUSTER_CFG0) & ~(1 << L3_SHARE_EN));
	dsb();

	__asm__ volatile ("mrs %0, S3_0_C15_C3_5" : "=r" (v));
	v |= (0xf << 4);
	__asm__ volatile ("msr S3_0_C15_C3_5, %0" : : "r" (v));
	dsb();

PWR_LOOP:
	__asm__ volatile ("mrs %0, S3_0_C15_C3_7" : "=r" (v));
	if (((v >> 0x4) & 0xf) != 0xf)
		goto PWR_LOOP;

	mmio_write_64_relax(MP0_CLUSTER_CFG0, mmio_read_64(MP0_CLUSTER_CFG0) & ~(1 << L3_SHARE_PRE_EN));
	dsb();
}

#if 0 /* not supported */
spinlock_t l2_share_lock;

uint64_t switch_L2_size(uint64_t option, uint64_t share_cluster_num, uint64_t cluster_borrow_return)
{
	unsigned int cache_cfg0, cache_cfg1;
	int ret = MTK_SIP_E_SUCCESS;
	struct _l2c_share_info share_info;

	share_info.cluster_borrow = (cluster_borrow_return >> 16) & 0xFFFF;
	share_info.cluster_return = cluster_borrow_return & 0xFFFF;
	share_info.share_cluster_num = share_cluster_num;

	spin_lock(&l2_share_lock);
	dis_i_d_dcsw_op_all(DCCISW);

	cache_cfg0 = mmio_read_32(MP0_CA7L_CACHE_CONFIG);
	cache_cfg0 &= ~(0xf << L2C_SIZE_CFG_OFF);
	cache_cfg1 = mmio_read_32(MP1_CA7L_CACHE_CONFIG);
	cache_cfg1 &= ~(0xf << L2C_SIZE_CFG_OFF);

	switch (option) {
	case BORROW_L2:
		if (share_info.share_cluster_num == 1) {
			cache_cfg0 |= (share_info.cluster_borrow << L2C_SIZE_CFG_OFF);
			mmio_write_32(MP0_CA7L_CACHE_CONFIG, cache_cfg0);
			cache_cfg0 = mmio_read_32(MP0_CA7L_CACHE_CONFIG) | (0x1 << L2C_SHARE_ENABLE);
			mmio_write_32(MP0_CA7L_CACHE_CONFIG, cache_cfg0);
		} else if (share_info.share_cluster_num == 2) {
			cache_cfg0 |= (share_info.cluster_borrow << L2C_SIZE_CFG_OFF);
			mmio_write_32(MP0_CA7L_CACHE_CONFIG, cache_cfg0);
			cache_cfg0 = mmio_read_32(MP0_CA7L_CACHE_CONFIG) | (0x1 << L2C_SHARE_ENABLE);
			mmio_write_32(MP0_CA7L_CACHE_CONFIG, cache_cfg0);

			cache_cfg1 |= (share_info.cluster_borrow << L2C_SIZE_CFG_OFF);
			mmio_write_32(MP1_CA7L_CACHE_CONFIG, cache_cfg1);
			cache_cfg1 = mmio_read_32(MP1_CA7L_CACHE_CONFIG) | (0x1 << L2C_SHARE_ENABLE);
			mmio_write_32(MP1_CA7L_CACHE_CONFIG, cache_cfg1);
		}
		break;

	case RETURN_L2:
		if (share_info.share_cluster_num == 1) {
			cache_cfg0 = mmio_read_32(MP0_CA7L_CACHE_CONFIG) |
						(share_info.cluster_return << L2C_SIZE_CFG_OFF);
			mmio_write_32(MP0_CA7L_CACHE_CONFIG, cache_cfg0);
			cache_cfg0 = mmio_read_32(MP0_CA7L_CACHE_CONFIG) & ~(0x1 << L2C_SHARE_ENABLE);
			mmio_write_32(MP0_CA7L_CACHE_CONFIG, cache_cfg0);
		} else if (share_info.share_cluster_num == 2) {
			cache_cfg0 = mmio_read_32(MP0_CA7L_CACHE_CONFIG) |
						(share_info.cluster_return << L2C_SIZE_CFG_OFF);
			mmio_write_32(MP0_CA7L_CACHE_CONFIG, cache_cfg0);
			cache_cfg0 = mmio_read_32(MP0_CA7L_CACHE_CONFIG) & ~(0x1 << L2C_SHARE_ENABLE);
			mmio_write_32(MP0_CA7L_CACHE_CONFIG, cache_cfg0);

			/* mp1 L2$ 512KB */
			cache_cfg1 = mmio_read_32(MP1_CA7L_CACHE_CONFIG) |
						(share_info.cluster_return << L2C_SIZE_CFG_OFF);
			mmio_write_32(MP1_CA7L_CACHE_CONFIG, cache_cfg1);
			cache_cfg1 = mmio_read_32(MP1_CA7L_CACHE_CONFIG) & ~(0x1 << L2C_SHARE_ENABLE);
			mmio_write_32(MP1_CA7L_CACHE_CONFIG, cache_cfg1);
		}
		break;

	default:
		ret = MTK_SIP_E_NOT_SUPPORTED;
		break;
	}

	enable_cache();

	spin_unlock(&l2_share_lock);
	return ret;
}
#endif
