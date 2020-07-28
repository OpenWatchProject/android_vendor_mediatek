#include <arch.h>
#include <platform_def.h>
#include <mmio.h>

void disable_scu(unsigned long mpidr) {
	uint32_t axi_config = (mpidr & MPIDR_CLUSTER_MASK) ? MP1_AXI_CONFIG : MP0_AXI_CONFIG;
	mmio_write_32(axi_config, mmio_read_32(axi_config) | ACINACTM);
}

void enable_scu(unsigned long mpidr) {
	uint32_t axi_config = (mpidr & MPIDR_CLUSTER_MASK) ? MP1_AXI_CONFIG : MP0_AXI_CONFIG;
	mmio_write_32(axi_config, mmio_read_32(axi_config) & ~ACINACTM);
}
