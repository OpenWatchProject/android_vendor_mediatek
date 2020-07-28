/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <string.h>
#include <arch.h>
#include <mcucfg.h>
#include <mmio.h>
#include <assert.h>
#include <debug.h>

void disable_scu(unsigned long mpidr)
{
	uintptr_t axi_config = 0;

	VERBOSE("[ATF]: %s()\n", __func__);

/*	mcsib_sw_workaround();*/
	switch (mpidr & MPIDR_CLUSTER_MASK) {
		case 0x000:
			axi_config = MP0_AXI_CONFIG;
/*			mmio_setbits_32((uintptr_t)axi_config, MP0_ACINACTM);*/
			mmio_write_32(axi_config,
                                      mmio_read_32(axi_config) | MPx_AXI_CONFIG_acinactm |
                                      MPx_AXI_CONFIG_ainacts);
			break;
		case 0x100:
			axi_config = MP1_AXI_CONFIG;
/*			mmio_setbits_32((uintptr_t)axi_config, MP1_ACINACTM);*/
			mmio_write_32(axi_config,
                                      mmio_read_32(axi_config) | MPx_AXI_CONFIG_acinactm |
                                      MPx_AXI_CONFIG_ainacts);
			break;
		case 0x200:
			axi_config = MP2_AXI_CONFIG;
			/*mmio_setbits_32((uintptr_t)axi_config, MP2_ACINACTM);*/
			mmio_write_32(axi_config,
                                      mmio_read_32(axi_config) | MP2_AXI_CONFIG_acinactm |
                                      MP2_AXI_CONFIG_ainacts);
			break;
		default:
			ERROR("[ATF]: disable_scu: mpidr not exists\n");
			assert(0);
	}
}

void enable_scu(unsigned long mpidr)
{
	uintptr_t axi_config = 0;

	VERBOSE("[ATF]: %s()\n", __func__);

	switch (mpidr & MPIDR_CLUSTER_MASK) {
		case 0x000:
			axi_config = MP0_AXI_CONFIG;
			mmio_write_32(axi_config,
				      (mmio_read_32(axi_config) & ~MPx_AXI_CONFIG_acinactm)& ~MPx_AXI_CONFIG_ainacts);
			break;
		case 0x100:
			axi_config = MP1_AXI_CONFIG;
			mmio_write_32(axi_config,
				      (mmio_read_32(axi_config) & ~MPx_AXI_CONFIG_acinactm)& ~MPx_AXI_CONFIG_ainacts);
			break;
		case 0x200:
			axi_config = MP2_AXI_CONFIG;
			mmio_write_32(axi_config,
				      (mmio_read_32(axi_config) & ~MP2_AXI_CONFIG_acinactm)& ~MP2_AXI_CONFIG_ainacts);
			break;
		default:
			ERROR("[ATF]: enable_scu: mpidr not exists\n");
			assert(0);
	}
}
