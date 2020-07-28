#include <stdint.h>
#include <mmio.h>
#include <mtk_sip_svc.h>
#include <last_emi.h>

uint64_t sip_last_emi_reset(uint64_t start, uint64_t end)
{
	/* MSB for start address */
	mmio_write_32(EMI_AXI_BIST_ADR2, (start & (0x1UL << 32)) >> 32);

	/* MSB for end address */
	mmio_write_32(EMI_AXI_BIST_ADR2, (end & (0x1UL << 32)) >> 28);

	/* LSB for start and end address */
	mmio_write_32(EMI_AXI_BIST_ADR0, start & 0xffffffff);
	mmio_write_32(EMI_AXI_BIST_ADR1, end & 0xffffffff);

	return MTK_SIP_E_SUCCESS;
}
