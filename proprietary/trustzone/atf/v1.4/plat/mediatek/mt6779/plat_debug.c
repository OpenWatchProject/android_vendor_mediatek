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
#include <mtk_macro_helper.h>
#include <mtspmc.h>
#include <plat_debug.h>
#include <platform_def.h>

void _plat_cpu_power_off(void)
{
	unsigned long v = 0;

	__asm__ volatile("mrs %0, " __stringify(CPUPWRCTLR_EL1) : "=r" (v));
	v = v | (CORE_PWRDN_EN_MASK);
	__asm__ volatile("msr " __stringify(CPUPWRCTLR_EL1) ", %0" : : "r" (v));
	isb();
	dsbsy();
	while (1)
		wfi();
}

void circular_buffer_setup(void)
{
	sync_writel(CA75_CPU6_DBG_CONTROL, mmio_read_32(CA75_CPU6_DBG_CONTROL) & ~(BIT_CA75_LASTPC_DIS));
	sync_writel(CA75_CPU7_DBG_CONTROL, mmio_read_32(CA75_CPU7_DBG_CONTROL) & ~(BIT_CA75_LASTPC_DIS));
}

void clear_all_on_mux(void)
{
	unsigned int i, rg_cpu_hotplug_reset = SPMC_PWR_RST_CORE1;
	uintptr_t backup_bootaddr_reg[PLATFORM_CORE_COUNT] = {0};
	unsigned long backup_mcucfg_initarch_reg = mmio_read_32(MCUCFG_INITARCH);

	/* write 0 to reset_pwr_on_en (bit20) */
	sync_writel(CPC_FLOW_CTRL_CFG, mmio_read_32(CPC_FLOW_CTRL_CFG) & ~(1 << 20));

	/* power off other CPUs */
	for (i = 1; i <= PLATFORM_CORE_COUNT-1; ++i, rg_cpu_hotplug_reset += 0x4) {
		/* check power status of each CPU */
		if (mmio_read_32(CPC_POWER_STATUS) & (1 << i)) {
			/* set CPUx initarch state to AARCH64 */
			mcucfg_init_archstate(0, i, 1);

			/* backup the bootaddr of CPUx */
			backup_bootaddr_reg[i] = mcucfg_get_bootaddr(0, i);

			/* set the bootaddr of CPUx to _plat_cpu_power_off */
			mcucfg_set_bootaddr(0, i, (uintptr_t) _plat_cpu_power_off);
			dsbsy();
			/* release CPUx reset */
			sync_writel(rg_cpu_hotplug_reset, mmio_read_32(rg_cpu_hotplug_reset) | (1 << 0));
		}
	}

	/* Polling all other CPUs except CPU0 to off state */
	/* INFO("CPU0 is waiting all other CPUs to off state(0x0C53A840 = 0x%x)\n", mmio_read_32(CPC_POWER_STATUS)); */
	while (mmio_read_32(CPC_POWER_STATUS) != MASK_CPC_CORE0_ONLY)
		;
	/* INFO("Only CPU0 is on (0x0C53A840 = 0x%x)\n", mmio_read_32(CPC_POWER_STATUS)); */

	rg_cpu_hotplug_reset = SPMC_PWR_RST_CORE1;
	for (i = 1; i <= PLATFORM_CORE_COUNT-1; ++i, rg_cpu_hotplug_reset += 0x4) {
		if (mmio_read_32(rg_cpu_hotplug_reset) & 0x1) {
			/* assert CPUx reset */
			sync_writel(rg_cpu_hotplug_reset, mmio_read_32(rg_cpu_hotplug_reset) & ~(1 << 0));
			/* restore the bootaddr of CPUx */
			mcucfg_set_bootaddr(0, i, (uintptr_t) backup_bootaddr_reg[i]);
		}
	}

	/* restore MCUCFG_INTARCH */
	sync_writel(MCUCFG_INITARCH, backup_mcucfg_initarch_reg);
	dsbsy();
}

void circular_buffer_unlock(void)
{
	/* set DFD.en */
	sync_writel(DFD_INTERNAL_CTL, 0x1);
}

void circular_buffer_lock(void)
{
	/* clear DFD.en */
	sync_writel(DFD_INTERNAL_CTL, 0x0);
}

unsigned int plat_debug_smc_dispatcher(unsigned long arg0, unsigned long arg1, unsigned long arg2)
{
	unsigned int ret = 0;

	switch (arg0) {
	case PLAT_LASTPC_SMC_READ_ENTRY:
		if (arg1 == 6)
			ret = mmio_read_32(CA75_CPU6_CIRCULAR_BUF + arg2);
		else if (arg1 == 7)
			ret = mmio_read_32(CA75_CPU7_CIRCULAR_BUF + arg2);
		break;
	case PLAT_MTK_CIRCULAR_BUFFER_UNLOCK:
		circular_buffer_unlock();
		break;
	case PLAT_MTK_CIRCULAR_BUFFER_LOCK:
		circular_buffer_lock();
		break;
	default:
		ret = 0;
		break;
	}

	/* INFO("arg0 = 0x%lx, arg1 = 0x%lx, arg2 = 0x%lx, ret = 0x%x\n", arg0, arg1, arg2, ret); */
	return ret;
}

void sram_delsel_setup(void)
{
	sync_writel(CA75_CPU6_SRAM_DELSEL,
			((mmio_read_32(CA75_CPU6_SRAM_DELSEL) & ~CPU_SRAM_DELSEL_MASK))
			| 0x20 | (0xb20 << 6) | (0x20 << 18));
	sync_writel(CA75_CPU7_SRAM_DELSEL,
			((mmio_read_32(CA75_CPU7_SRAM_DELSEL) & ~CPU_SRAM_DELSEL_MASK))
			| 0x20 | (0xb20 << 6) | (0x20 << 18));
}
