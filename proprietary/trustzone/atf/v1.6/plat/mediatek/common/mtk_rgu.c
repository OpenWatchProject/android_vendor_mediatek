/*
 * Copyright (c) 2013-2015, ARM Limited and Contributors. All rights reserved.
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

#include <debug.h>
#include <mmio.h>
#include <platform.h>
#include <mtk_rgu.h>
#include <delay_timer.h>

#define RGU_SW_RESET_DELAY_MS   (100)

static void plat_rgu_set_time_out_value(unsigned int value)
{
	unsigned int timeout;
	/*
	* TimeOut = BitField 15:5
	* Key	  = BitField  4:0 = 0x08
	*/

	/* sec * 32768 / 512 = sec * 64 = sec * 1 << 6 */
	timeout = value * (1 << 6);
	timeout = timeout << 5;
	mmio_write_32(MTK_WDT_LENGTH, (timeout | MTK_WDT_LENGTH_KEY));
}

/* called in low power enter/exit function to force WDT stop/start */
void plat_rgu_suspend_notify(void)
{
	unsigned int tmp;

	tmp = mmio_read_32(MTK_WDT_MODE);

	/* keep record of wdt is disabled in ATF, and wdt should */
	/* be enabled again depends on this flag when system	 */
	/* resume						 */
	if (tmp & MTK_WDT_MODE_ENABLE)
		mmio_write_32(MTK_WDT_NONRST_REG2,
			mmio_read_32(MTK_WDT_NONRST_REG2) |
			MTK_WDT_NONRST_REG2_SUSPEND_DIS);
	else
		mmio_write_32(MTK_WDT_NONRST_REG2,
			mmio_read_32(MTK_WDT_NONRST_REG2) &
			~MTK_WDT_NONRST_REG2_SUSPEND_DIS);

	tmp &= ~MTK_WDT_MODE_ENABLE;
	mmio_write_32(MTK_WDT_MODE, tmp | MTK_WDT_MODE_KEY);
	mmio_write_32(MTK_WDT_RESTART, MTK_WDT_RESTART_KEY);
}

void plat_rgu_resume_notify(void)
{
	if (mmio_read_32(MTK_WDT_NONRST_REG2) &
	    MTK_WDT_NONRST_REG2_SUSPEND_DIS) {
		unsigned int tmp;

		tmp = mmio_read_32(MTK_WDT_MODE);
		mmio_write_32(MTK_WDT_MODE, tmp | MTK_WDT_MODE_KEY |
				MTK_WDT_MODE_ENABLE);
		mmio_write_32(MTK_WDT_RESTART, MTK_WDT_RESTART_KEY);
	}
}

void plat_rgu_change_tmo(unsigned int tmo)
{
	unsigned int wdt_status = mmio_read_32(MTK_WDT_STATUS);
	/* change watchdog timeout to tmo to avoid */
	/* pmic reset when sysrst/eintrst		*/

	if (wdt_status & (MTK_WDT_STATUS_EINT_RST |
	    MTK_WDT_STATUS_SYSRST_RST)) {
		plat_rgu_set_time_out_value(tmo);
		mmio_write_32(MTK_WDT_RESTART, MTK_WDT_RESTART_KEY);
	}
}

void plat_rgu_sw_reset(void)
{
	unsigned int tmp;

	plat_rgu_dump_reg();

	mmio_write_32(MTK_WDT_RESTART, MTK_WDT_RESTART_KEY);

	tmp = mmio_read_32(MTK_WDT_BASE);

	tmp &= ~(MTK_WDT_MODE_DUAL_MODE | MTK_WDT_MODE_IRQ);
	tmp |= MTK_WDT_MODE_KEY | MTK_WDT_MODE_EXTEN;
	mmio_write_32(MTK_WDT_BASE, tmp);

	mmio_write_32(MTK_WDT_SWRST, MTK_WDT_SWRST_KEY);
	mdelay(RGU_SW_RESET_DELAY_MS);

	wfi();
	ERROR("MTK System Reset: operation not handled.\n");
	panic();
}

unsigned int plat_rgu_read_status(void)
{
	return mmio_read_32(MTK_WDT_STATUS);
}

/* allow dump once by one CPU to avoid dummy dump by all other CPUs */

void plat_rgu_dump_reg(void)
{
	unsigned int wdt_status;

	/* dump RGU registers */

	INFO("[RGU-ATF] MODE:0x%x\n", mmio_read_32(MTK_WDT_MODE));
	INFO("[RGU-ATF] LENGTH:0x%x\n", mmio_read_32(MTK_WDT_LENGTH));

	wdt_status = mmio_read_32(MTK_WDT_STATUS);

	INFO("[RGU-ATF] STATUS:0x%x\n", wdt_status);
	INFO("[RGU-ATF] INTERVAL:0x%x\n", mmio_read_32(MTK_WDT_INTERVAL));
	INFO("[RGU-ATF] NONRST_REG:0x%x\n", mmio_read_32(MTK_WDT_NONRST_REG));
	INFO("[RGU-ATF] NONRST_REG2:0x%x\n", mmio_read_32(MTK_WDT_NONRST_REG2));
	INFO("[RGU-ATF] REQ_MODE:0x%x\n", mmio_read_32(MTK_WDT_REQ_MODE));
	INFO("[RGU-ATF] REQ_IRQ_EN:0x%x\n", mmio_read_32(MTK_WDT_REQ_IRQ_EN));
	INFO("[RGU-ATF] EXT_REQ_CON:0x%x\n", mmio_read_32(MTK_WDT_EXT_REQ_CON));
	INFO("[RGU-ATF] DEBUG_CTL:0x%x\n", mmio_read_32(MTK_WDT_DEBUG_CTL));
	INFO("[RGU-ATF] LATCH_CTL:0x%x\n", mmio_read_32(MTK_WDT_LATCH_CTL));
	INFO("[RGU-ATF] DEBUG_CTL2:0x%x\n", mmio_read_32(MTK_WDT_DEBUG_CTL2));
	INFO("[RGU-ATF] COUNTER:0x%x\n", mmio_read_32(MTK_WDT_COUNTER));

	/* analyze interrupt source */

	INFO("[RGU-ATF] Status:\n");

	if (wdt_status & MTK_WDT_STATUS_SPM_THERMAL_RST)
		INFO("[RGU-ATF]   SPM_THERMAL\n");

	if (wdt_status & MTK_WDT_STATUS_SPM_RST)
		INFO("[RGU-ATF]   SPM\n");

	if (wdt_status & MTK_WDT_STATUS_EINT_RST)
		INFO("[RGU-ATF]   EINT\n");

	if (wdt_status & MTK_WDT_STATUS_SYSRST_RST)
		INFO("[RGU-ATF]   SYSRST\n");

	if (wdt_status & MTK_WDT_STATUS_DVFSP_RST)
		INFO("[RGU-ATF]   DVFSP\n");

	if (wdt_status & MTK_WDT_STATUS_PMCU_RST)
		INFO("[RGU-ATF]   SSPM\n");

	if (wdt_status & MTK_WDT_STATUS_MDDBG_RST)
		INFO("[RGU-ATF]   MDDBG\n");

	if (wdt_status & MTK_WDT_STATUS_THERMAL_DIRECT_RST)
		INFO("[RGU-ATF]   THERMAL_DIRECT\n");

	if (wdt_status & MTK_WDT_STATUS_DEBUG_RST)
		INFO("[RGU-ATF]   DBG\n");

	if (wdt_status & MTK_WDT_STATUS_SECURITY_RST)
		INFO("[RGU-ATF]   SEC\n");

	if (wdt_status & MTK_WDT_STATUS_IRQ_ASSERT)
		INFO("[RGU-ATF]   IRQ\n");

	if (wdt_status & MTK_WDT_STATUS_SW_WDT_RST)
		INFO("[RGU-ATF]   SW_WDT\n");

	if (wdt_status & MTK_WDT_STATUS_HW_WDT_RST)
		INFO("[RGU-ATF]   HW_WDT\n");
}
