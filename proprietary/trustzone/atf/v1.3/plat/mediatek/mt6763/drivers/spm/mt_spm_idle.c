#include <mmio.h>

#define INFRACFG_AO_BASE	0x10001000
#define INFRACFG_AO_REG(ofs)	(INFRACFG_AO_BASE + ofs)
#define MODULE_SW_CG_1_STA	INFRACFG_AO_REG(0x0094)

#define MODULE_SW_CG_1_MASK	0x08000000

uint64_t spm_idle_check_secure_cg(__uint64_t x1, __uint64_t x2, __uint64_t x3)
{
	uint32_t val = 0;

	/* Check DXCC secure core CG: 0x10001094[27] */
	val = ~mmio_read_32(MODULE_SW_CG_1_STA);
	val &= MODULE_SW_CG_1_MASK;

	return (uint64_t)val;
}
