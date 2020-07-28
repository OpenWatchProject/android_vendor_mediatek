#include <arch_helpers.h>
#include <arch.h>
#include <l2c.h>
#include <mcucfg.h>
#include <mmio.h>
#include <plat_private.h>
#include <platform_def.h>
#include <spinlock.h>

extern void dcsw_op_level1(unsigned int type);
extern void dcsw_op_level2(unsigned int type);

void config_L2_size()
{
	unsigned int cache_cfg0;

	/* mp0 L2$ 1024KB;*/
	cache_cfg0 = mmio_read_32(MP0_CA7L_CACHE_CONFIG) & (0xF << L2C_SIZE_CFG_OFF);
	cache_cfg0 |= (0x7 << L2C_SIZE_CFG_OFF);
	mmio_write_32(MP0_CA7L_CACHE_CONFIG, cache_cfg0);
	cache_cfg0 &= ~(0x1 << L2C_SHARE_ENABLE);
	mmio_write_32(MP0_CA7L_CACHE_CONFIG, cache_cfg0);
}
