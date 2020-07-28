/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
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

#include <arch_helpers.h>
#include <debug.h>
#include <mmio.h>
#include <platform_def.h>
#include <plat_debug.h>

static unsigned long dfd_enabled;
static unsigned long dfd_base_addr;
static unsigned long dfd_chain_length;

static void dfd_setup(unsigned long base_addr, unsigned long chain_length)
{
	/* bit[0] : rg_rw_dfd_internal_dump_en -> 1 */
	/* bit[2] : rg_rw_dfd_clock_stop_en -> 1 */
	sync_writel(DFD_INTERNAL_CTL, 0x5);

	/* bit[14] : early scan mode */
	writel(DFD_INTERNAL_CTL, readl(DFD_INTERNAL_CTL)|(0x1 << 14));

	/*
	 * bit[6:3] : rg_rw_dfd_trigger_sel
	 * 4’b0000 -> JTAG trigger
	 * 4’b0001 -> WD trigger
	 * 4’b0011 -> SW trigger
	 */
	writel(DFD_INTERNAL_CTL, readl(DFD_INTERNAL_CTL)|(0x1 << 3));

	/*
	 * bit[0] : rg_rw_dfd_auto_power_on -> 1
	 * bit[2:1] : rg_rw_dfd_auto_power_on_dely -> 1 (10us)
	 * bit[4:2] : rg_rw_dfd_power_on_wait_time -> 1 (20us)
	 */
	writel(DFD_INTERNAL_PWR_ON, 0xb);

	/* longest scan chain length */
	writel(DFD_CHAIN_LENGTH0, chain_length);

	/*
	 * bit[1:0] : rg_rw_dfd_shift_clock_ratio
	 * 2’b00 -> 1:2
	 */
	writel(DFD_INTERNAL_SHIFT_CLK_RATIO, 0x0);

	/* total 20 test_so */
	writel(DFD_INTERNAL_TEST_SO_0, 0x14);

	/* Only one group of test_so */
	writel(DFD_INTERNAL_NUM_OF_TEST_SO_GROUP, 0x1);

	/* for DFD-3.0 setup */
	sync_writel(DFD_V30_CTL, 0x1);

	/* set base address */
	writel(DFD_V30_BASE_ADDR, (base_addr & 0xfff00000));

	/* setup global variables for suspend/resume */
	dfd_enabled = 1;
	dfd_base_addr = base_addr;
	dfd_chain_length = chain_length;

	dsbsy();
}

void dfd_resume(void)
{
	if (dfd_enabled)
		dfd_setup(dfd_base_addr, dfd_chain_length);
}

int dfd_smc_dispatcher(unsigned long arg0, unsigned long arg1, unsigned long arg2)
{
	int ret = 0;

	switch (arg0) {
	case PLAT_MTK_DFD_SETUP_MAGIC:
		dfd_setup(arg1, arg2);
		break;
	default:
		ret = -1;
		break;
	}

	return ret;
}

void clear_all_on_mux(void)
{
	sync_writel(MCU_ALL_PWR_ON_CTRL, mmio_read_32(MCU_ALL_PWR_ON_CTRL) & ~(1 << 2));
	sync_writel(MCU_ALL_PWR_ON_CTRL, mmio_read_32(MCU_ALL_PWR_ON_CTRL) & ~(1 << 1));
}
