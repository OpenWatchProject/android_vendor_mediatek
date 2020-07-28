/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
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

#include <arch.h>
#include <assert.h>
#include <debug.h>
#include <mcucfg.h>
#include <mmio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void disable_scu(unsigned long mpidr)
{
	uintptr_t axi_config = 0;
	unsigned int acinactm = 0;

	switch (mpidr & MPIDR_CLUSTER_MASK) {
	case 0x000:
		axi_config = MP0_AXI_CONFIG;
		acinactm = MPx_AXI_CONFIG_ACINACTM;
		break;
	case 0x100:
		axi_config = MP2_AXI_CONFIG;
		acinactm = MP2_AXI_CONFIG_ACINACTM;
		break;
	default:
		ERROR("disable_scu: mpidr not exists\n");
		assert(0);
	}
	mmio_setbits_32(axi_config, acinactm);
}

void enable_scu(unsigned long mpidr)
{
	uintptr_t axi_config = 0;
	unsigned int acinactm = 0;

	switch (mpidr & MPIDR_CLUSTER_MASK) {
	case 0x000:
		axi_config = MP0_AXI_CONFIG;
		acinactm = MPx_AXI_CONFIG_ACINACTM;
		break;
	case 0x100:
		axi_config = MP2_AXI_CONFIG;
		acinactm = MP2_AXI_CONFIG_ACINACTM;
		break;
	default:
		ERROR("enable_scu: mpidr not exists\n");
		assert(0);
	}
	mmio_clrbits_32(axi_config, acinactm);
}
