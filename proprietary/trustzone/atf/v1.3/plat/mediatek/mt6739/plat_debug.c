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

#include <arch_helpers.h>
#include <debug.h>
#include <mmio.h>
#include <plat_debug.h>
#include <platform_def.h>

static void dfd_reset_sf(void)
{
	unsigned long ret;

	/* bit[0] : rg_rw_dfd_internal_dump_en -> 1 */
	sync_writel(DFD_INTERNAL_CTL, 0x1);

	/*
	 * bit[0] : rg_rw_dfd_mcsib_df_disable -> 0
	 * and clear rg_mcsib_cfg_rst_ini_sf_dfd
	 */
	ret = readl(DFD_INTERNAL_MCSIB);
	sync_writel(DFD_INTERNAL_MCSIB, ret & ~0x1);

	/*
	 * bit[0] : rg_rw_dfd_mcsib_df_disable -> 1
	 * Set back to 1 for next sf disable
	 */
	sync_writel(DFD_INTERNAL_MCSIB, ret | 0x1);
}

static void dfd_setup_sf_sram(void)
{
	unsigned long ret;

	/* bit[0] : rg_rw_dfd_internal_dump_en -> 1 */
	sync_writel(DFD_INTERNAL_CTL, 0x1);

	/*
	 * bit[0] : sf1 off -> 1 (disable snoop filter)
	 * bit[1] : sf2 off -> 1 (disable snoop filter)
	 */
	ret = readl(CFG_SF_CTRL);
	sync_writel(CFG_SF_CTRL, ret | 0x3);

	/*
	 * bit[1] : rg_rw_dfd_mcsib_sram_sel -> 1
	 */
	ret = readl(DFD_INTERNAL_MCSIB);
	sync_writel(DFD_INTERNAL_MCSIB, ret | (0x1 << 1));

	/*
	 * Bit[0] : rg_r_dfd_mcsib_sel_staus
	 * Polling this bit, go to next step when this bit is '1'
	 */
	while ((readl(DFD_INTERNAL_MCSIB_SEL_STATUS) & 0x1) != 0x1)
		;
}

static void dfd_return_sf_sram(void)
{
	unsigned long ret;

	dfd_reset_sf();

	/*
	 * bit[1] : rg_rw_dfd_mcsib_sram_sel -> 0
	 */
	ret = readl(DFD_INTERNAL_MCSIB);
	sync_writel(DFD_INTERNAL_MCSIB, ret & ~(0x1 << 1));

	/*
	 * Bit[0] : rg_r_dfd_mcsib_sel_staus
	 * Polling this bit, go to next step when this bit is '1'
	 */
	while ((readl(DFD_INTERNAL_MCSIB_SEL_STATUS) & 0x1) != 0x0)
		;

	/*
	 * bit[0] : set 1 to trigger snoop filter initialization (sf1)
	 * bit[1] : set 1 to trigger snoop filter initialization (sf2)
	 */
	ret = readl(CFG_SF_INI);
	sync_writel(CFG_SF_INI, ret | 0x3);

	/*
	 * bit[16] : polling 1 when initialization finish (sf1)
	 * bit[17] : polling 1 when initialization finish (sf2)
	 */
	while (((readl(CFG_SF_INI) & (0x3 << 16)) >> 16) != 0x3)
		;

	/* flush all */
	dcsw_op_all(DCCISW);
}

void dfd_setup(void)
{
	/* bit[0] : rg_rw_dfd_internal_dump_en -> 1 */
	sync_writel(DFD_INTERNAL_CTL, 0x1);

	writel(DFD_INTERNAL_CTL, readl(DFD_INTERNAL_CTL)|(0x1 << 2));

	/* bit[14] : rg_rw_dfd_early_scan_mode -> 1 */
	writel(DFD_INTERNAL_CTL, readl(DFD_INTERNAL_CTL)|(0x1 << 14));

	/*
	 * bit[6:3] : rg_rw_dfd_trigger_sel
	 * 4b0000 -> JTAG trigger
	 * 4b0001 -> WD trigger
	 * 4b0011 -> SW trigger
	 */
	writel(DFD_INTERNAL_CTL, readl(DFD_INTERNAL_CTL)|(0x1 << 3));

	/*
	 * bit[0] : rg_rw_dfd_auto_power_on -> 1
	 * bit[2:1] : rg_rw_dfd_auto_power_on_dely -> 1 (10us)
	 * bit[4:2] : rg_rw_dfd_power_on_wait_time -> 1 (20us)
	 */
	writel(DFD_INTERNAL_PWR_ON, 0xb);

	/* longest scan chain length */
	writel(DFD_CHAIN_LENGTH0, 0x61A8);

	/*
	 * bit[1:0] : rg_rw_dfd_shift_clock_ratio
	 * 2b00 -> 1:2
	 */
	writel(DFD_INTERNAL_SHIFT_CLK_RATIO, 0x0);

	/* total 51 test_so */
	writel(DFD_INTERNAL_TEST_SO_0, 0x34);

	/* Only one group of test_so */
	writel(DFD_INTERNAL_NUM_OF_TEST_SO_GROUP, 0x1);

	dsbsy();

	/* workaround for big */
	sync_writel(CA15M_CACHE_CTL, 0x0);

	/* set dfd_valid */
	sync_writel(PLAT_SRAM_FLAG1, readl(PLAT_SRAM_FLAG1)|BIT_DFD_VALID);
}

void dfd_disable(void)
{
	sync_writel(DFD_INTERNAL_CTL, 0x0);

	/* clear dfd_valid */
	sync_writel(PLAT_SRAM_FLAG1, readl(PLAT_SRAM_FLAG1)&~BIT_DFD_VALID);
}

/* setup other debug features */
void circular_buffer_setup(void)
{
	/* clear DBG_CONTROL.lastpc_disable to enable circular buffer */
	sync_writel(CA15M_DBG_CONTROL, readl(CA15M_DBG_CONTROL) & ~(BIT_CA15M_LASTPC_DIS));
}

void circular_buffer_unlock(void)
{
	/* set DBG_CONTROL.lastpc_disable */
	sync_writel(CA15M_DBG_CONTROL, readl(CA15M_DBG_CONTROL) | (BIT_CA15M_LASTPC_DIS));
	/* set DFD.en */
	sync_writel(DFD_INTERNAL_CTL, 0x1);
}

void circular_buffer_lock(void)
{
	/* clear DFD.en */
	sync_writel(DFD_INTERNAL_CTL, 0x0);
}

void l2c_parity_check_setup(void)
{
	/* enable DBG_CONTROL.l2parity_en */
	sync_writel(CA15M_DBG_CONTROL, readl(CA15M_DBG_CONTROL) | BIT_CA15M_L2PARITY_EN);
}

int plat_debug_smc_dispatcher(unsigned long arg0, unsigned long arg1, unsigned long arg2)
{
	int ret = 0;

	switch (arg0) {
		case PLAT_MTK_DFD_SETUP_SRAM_MAGIC:
			dfd_setup_sf_sram();
			break;
		case PLAT_MTK_DFD_RESET_SF_MAGIC:
			dfd_reset_sf();
			break;
		case PLAT_MTK_DFD_RETURN_SRAM_MAGIC:
			dfd_return_sf_sram();
			break;
		case PLAT_MTK_CIRCULAR_BUFFER_UNLOCK:
			circular_buffer_unlock();
			break;
		case PLAT_MTK_CIRCULAR_BUFFER_LOCK:
			circular_buffer_lock();
			break;
		default:
			ret = -1;
			break;
	}

	return ret;
}

/* Disable DBGRST_ALL to keep ETB content after WDT reset */
void disable_dbgrst_all(void)
{
	sync_writel(DBGTOP_LAR, DBGTOP_UNLOCK_KEY);
	sync_writel(DBGTOP_DBGRST_ALL, 0x0);
	sync_writel(DBGTOP_LAR, 0x0);
}
