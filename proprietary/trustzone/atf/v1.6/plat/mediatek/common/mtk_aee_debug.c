/*
 * Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
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
#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <context.h>
#include <context_mgmt.h>
#include <console.h>
#include <debug.h>
#include <delay_timer.h>
#include <fiq_smp_call.h>
#include <log.h>
#include <mtk_aee_debug.h>
#include <mtk_plat_common.h>
#include <plat_private.h>
#include <platform.h>
#include <runtime_svc.h>
#include <spinlock.h>
#include <stdio.h>
#include <xlat_tables_defs.h>
#include <mtk_rgu.h>
#include <mmio.h>
#include <mtk_gic.h>

#define LINUX_KERNEL_64BIT_MTK_OFFSET	0x80000
#define LINUX_KERNEL_32BIT_MTK_OFFSET	0x8000
#define LINUX_KERNEL_64BIT_VA_OFFSET	0xffffff8000000000
#define LINUX_KERNEL_32BIT_VA_OFFSET	0xc0000000

#define RAM_CONSOLE_SIG_ATF (0x43474244) /* DBGC */
#define RAM_CONSOLE_EXP_TYPE_MAGIC_ATF 0xaeedead0

enum RAM_CONSOLE_DEF_TYPE {
	RAM_CONSOLE_DEF_UNKNOWN = 0,
	RAM_CONSOLE_DEF_SRAM,
	RAM_CONSOLE_DEF_DRAM,
};

struct ram_console_buffer_atf {
	uint32_t sig;
	uint32_t others1[9];
	uint32_t sz_buffer;
	uint32_t off_linux;
	uint32_t others2[4];
};

struct last_reboot_reason_atf {
	uint32_t fiq_step;
	uint32_t exp_type;
	uint32_t others[0];
};

__attribute__ ((weak))
void ram_console_info_get(struct boot_tag_ram_console_info *rci)
{
	if (rci == NULL)
		return;
	rci->sram_addr = 0;
	rci->sram_size = 0;
	rci->def_type = 0;
	rci->memory_info_offset = 0;
}

uint32_t ram_console_set_exp_type_atf(uint32_t exp_type)
{
	struct boot_tag_ram_console_info ram_console_info;
	struct ram_console_buffer_atf *ram_console;
	struct last_reboot_reason_atf *lrra;
	uint32_t ret = 0;

	ram_console_info_get(&ram_console_info);
	ram_console =
		(struct ram_console_buffer_atf *)(unsigned long)ram_console_info.sram_addr;
	/* TODO: dram case checking */
	if (ram_console_info.def_type != RAM_CONSOLE_DEF_SRAM) {
		NOTICE("ram_console non-sram type(%d) NOT supported\n",
			ram_console_info.def_type);
		return ret;
	}

	NOTICE("[PL2ATF] ram_console base:%p\n", (void *)ram_console);
	if (ram_console && ram_console->off_linux &&
		(ram_console->off_linux < ram_console->sz_buffer) &&
		(ram_console->sig == RAM_CONSOLE_SIG_ATF)) {

		if (exp_type < 16) {
			NOTICE("set ram_console exp type to [%d]\n", exp_type);
			exp_type = exp_type ^ RAM_CONSOLE_EXP_TYPE_MAGIC_ATF;
			lrra = (struct last_reboot_reason_atf *)
				((unsigned long)ram_console + ram_console->off_linux);
			if (lrra->exp_type == 0)
				lrra->exp_type = exp_type;
			else
				NOTICE("set exp type failed: it had been set as %d before\n",
					lrra->exp_type ^ RAM_CONSOLE_EXP_TYPE_MAGIC_ATF);
			ret = 1;
		} else {
			NOTICE("%s:%s:off_linux:0x%x sig:0x%x sz_buffer:0x%x exp type:%d\n"
			, __func__, "FATAL", ram_console->off_linux,
			ram_console->sig, ram_console->sz_buffer, exp_type);
		}
	} else {
		NOTICE("ram_console NOT init yet\n");
	}
	return ret;
}

#define PLATFORM_MAX_CPU_BIT_MASK ((1 << PLATFORM_CORE_COUNT) - 1)

#if 0
/* For check mode test case */
#define MTK_AEE_TC_CHECK_MODE
#endif

#if 0
/* For early exception, coming cpu dump its stack */
#define MTK_AEE_TC_KWDT_NOT_READY
#endif

#define DCACHE_IS_ENABLE (read_sctlr_el3() & SCTLR_C_BIT)
#define MODE32_SVC_SP_REG	19
/* #define MODE32_SVC_LR_REG	18 */

uint64_t wdt_kernel_cb_addr;
uint64_t mtk_lk_stage = 1;

extern uint64_t get_kernel_k32_64(void);

spinlock_t aee_wdt_dump_lock;
extern atf_log_ctrl_t *p_atf_log_ctrl;

#pragma weak plat_rgu_dump_reg
void plat_rgu_dump_reg(void)
{
	return;
}

void plat_mrdump_key(void)
{
	unsigned int wdt_status = mmio_read_32(MTK_WDT_STATUS);
	if (wdt_status & (MTK_WDT_STATUS_EINT_RST |
	    MTK_WDT_STATUS_SYSRST_RST)) {
		ram_console_set_exp_type_atf(AEE_EXP_TYPE_SMART_RESET);
	} else {
		ram_console_set_exp_type_atf(AEE_EXP_TYPE_HWT);
	}

}

void aee_flush_log_cache(void)
{
	flush_dcache_range((unsigned long)p_atf_log_ctrl->info.atf_log_addr,
		p_atf_log_ctrl->info.atf_log_size);
}
static void mtk_module_dump(void)
{
	/* ATF should not enable console log since there is a change that UART is clock gated. */
	/* console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE); */
	/* dump RGU register */
	plat_mrdump_key();
	plat_rgu_change_tmo(2);
	plat_rgu_dump_reg();

}
void aee_wdt_dump(void __unused * cookie)
{
	struct atf_aee_regs *regs;
	cpu_context_t *ns_cpu_context;
	uint64_t mpidr = read_mpidr();
	uint32_t linear_id = plat_core_pos_by_mpidr(mpidr);
	unsigned long sp_phys, count;
	uint64_t *pp;
	uint64_t spsr;
	uint64_t el1_spsr;
	int i;
	uint64_t sp_el1;
	uint64_t trapped_mode;
	int execution_state;

	/* save context if from non-secure */
	if (read_scr_el3() & SCR_NS_BIT)
		cm_el1_sysregs_context_save(NON_SECURE);

	ns_cpu_context = (cpu_context_t *)cm_get_context_by_index(linear_id, NON_SECURE);

	spsr = SMC_GET_EL3(ns_cpu_context, CTX_SPSR_EL3);
	if (GET_RW(spsr) == MODE_RW_32) {
		sp_el1 = SMC_GET_GP(ns_cpu_context, (CTX_GPREG_X0 + (MODE32_SVC_SP_REG << 3)));
	} else {
		sp_el1 = read_ctx_reg(get_sysregs_ctx(ns_cpu_context), CTX_SP_EL1);
	}

	/* lock aee_wdt_dump_lock for fine print */
	if (DCACHE_IS_ENABLE)
		spin_lock(&aee_wdt_dump_lock);

	/* Always print dump info on console in LK stage */
	if (mtk_lk_stage || (wdt_kernel_cb_addr == 0))
		console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE);

	/* Log starts here... */
	INFO("%s: on cpu%d\n", __func__, (int)linear_id);

	struct atf_arg_t *teearg = &gteearg;

	regs = (void *)(teearg->atf_aee_debug_buf_start + (linear_id * sizeof(struct atf_aee_regs)));

	/* save debug infos */
	regs->pstate = SMC_GET_EL3(ns_cpu_context, CTX_SPSR_EL3);
	regs->pc = SMC_GET_EL3(ns_cpu_context, CTX_ELR_EL3);

	for (i = 0; i < 31; i++)
		regs->regs[i] = SMC_GET_GP(ns_cpu_context, (CTX_GPREG_X0 + (i<<3)));
	if (GET_RW(spsr) == MODE_RW_32)
		regs->sp = regs->regs[MODE32_SVC_SP_REG]; /* SVC SP */
	else
		regs->sp = read_ctx_reg(get_sysregs_ctx(ns_cpu_context), CTX_SP_EL1);

	/* dump debug infos in pretty print */
	INFO("(%d) pc:<%016llx> lr:<%016llx> sp:<%016llx> pstate=%llx\n",
		(int)linear_id, regs->pc, regs->regs[30], regs->sp, regs->pstate);
	for (i = 29; i >= 0; i -= 3) {
		INFO("(%d) x%02d: %016llx x%02d: %016llx x%02d: %016llx\n",
			(int)linear_id, i, regs->regs[i], (i-1), regs->regs[i-1], (i-2), regs->regs[i-2]);
	}

#ifdef MTK_AEE_TC_KWDT_NOT_READY
	wdt_kernel_cb_addr = 0;
#endif

	if (wdt_kernel_cb_addr == 0) {

		NOTICE("%s WDT not ready\n", mtk_lk_stage ? "LK" : "Kernel");
		/* Do not dump SP in kernel  */
		if ((regs->sp != 0) && (mtk_lk_stage == 1)) {
			if (GET_RW(spsr) == MODE_RW_64) {
				sp_phys = (regs->sp - LINUX_KERNEL_64BIT_VA_OFFSET) + DRAM_EMI_BASE;
			} else if (GET_RW(spsr) == MODE_RW_32 && mtk_lk_stage == 0)
				sp_phys = (regs->sp - LINUX_KERNEL_32BIT_VA_OFFSET) + DRAM_EMI_BASE;
			else
				sp_phys = regs->sp;
			NOTICE("sp_phys:0x%lx\n", sp_phys);

			/* map spphyscial memory for 2MB */
			mmap_add_dynamic_region((sp_phys & ~(PAGE_SIZE_2MB_MASK)),
					(sp_phys & ~(PAGE_SIZE_2MB_MASK)),
					PAGE_SIZE_2MB,
					MT_DEVICE | MT_RW | MT_NS);

			/* flush sp content  */
			flush_dcache_range((uint64_t)sp_phys, (uint64_t)0x2000);

			/* dump 8k */
			pp = (uint64_t *)(uintptr_t)sp_phys;
			count = (0x4000 - (sp_phys & (0x4000-1)))/8;

			NOTICE("dump sp(16k), count:%ld, mask: 0x3fff\n", count);
			for (i = 0; i < count ; i += 4, pp += 4)
				INFO("%016lx| %016lx %016lx %016lx %016lx\n", (unsigned long)(uintptr_t)pp, \
				(unsigned long)(uintptr_t)*pp, (unsigned long)(uintptr_t)*(pp+1), \
				(unsigned long)(uintptr_t)*(pp+2), (unsigned long)(uintptr_t)*(pp+3));
		}
		NOTICE("Wait timeout.\n");
		aee_flush_log_cache();
		if (DCACHE_IS_ENABLE)
			spin_unlock(&aee_wdt_dump_lock);
		while (1);
	}

	/* release aee_wdt_dump_lock */
	if (DCACHE_IS_ENABLE)
		spin_unlock(&aee_wdt_dump_lock);

	/* default enter EL1(64b) or SVC(32b) when enter AEE dump */
	if (mtk_lk_stage || (get_kernel_k32_64() == LINUX_KERNEL_32)) {
		trapped_mode = (spsr & ((MODE32_MASK) << MODE32_SHIFT)) >> MODE32_SHIFT;
		el1_spsr = (spsr & ~((MODE32_MASK) << MODE32_SHIFT));
		el1_spsr = (el1_spsr | ((MODE32_svc) << MODE32_SHIFT));

		/* check Thumb mode or ARM mode of the callback function. */
		/* If the exception is from ARM mode, but the callback is Thumb mode, we should correct the T bit. */
		el1_spsr = (el1_spsr & ~((SPSR_T_MASK) << SPSR_T_SHIFT));
		el1_spsr = (el1_spsr | ((wdt_kernel_cb_addr & SPSR_T_THUMB) << SPSR_T_SHIFT));
	} else {
		trapped_mode = (spsr & ((MODE_EL_MASK) << MODE_EL_SHIFT)) >> MODE_EL_SHIFT;
		el1_spsr = (spsr & ~((MODE_EL_MASK << MODE_EL_SHIFT)|(MODE_SP_MASK << MODE_SP_SHIFT)|
			(MODE_RW_MASK << MODE_RW_SHIFT)));
		el1_spsr = (el1_spsr | (MODE_EL1 << MODE_EL_SHIFT)|(MODE_SP_ELX << MODE_SP_SHIFT)|
			(MODE_RW_64 << MODE_RW_SHIFT));
	}

	/* disable IRQ when enter AEE dump */
	el1_spsr = (el1_spsr | (DAIF_IRQ_BIT << SPSR_AIF_SHIFT));
#ifdef MTK_AEE_TC_CHECK_MODE
	/* ready to make test case */
	if (linear_id == 0) {
		spsr = 0x000000000000008d;
		SMC_SET_EL3(ns_cpu_context, CTX_SPSR_EL3, el1_spsr);
	}
#else
	SMC_SET_EL3(ns_cpu_context, CTX_SPSR_EL3, el1_spsr);
#endif
	/* wdt kernel callback addr should be ready now... */
	if (sp_el1 == 0x0) {
		execution_state = spsr & (MODE_RW_MASK << MODE_RW_SHIFT);
		NOTICE("==Trapped EL/mode(0x%llx) is %s bit. sp_el1 is NULL, busy loop in ATF==\n",
			trapped_mode, execution_state ? "32" : "64");
		while (1)
			;
	} else
		SMC_SET_EL3(ns_cpu_context, CTX_ELR_EL3, wdt_kernel_cb_addr);
	/* Restore non-secure state */
	cm_el1_sysregs_context_restore(NON_SECURE);
	cm_set_next_eret_context(NON_SECURE);
	aee_flush_log_cache();

}
void wdt_notify_other_cores(inter_cpu_call_func_t func, void *info, int wait)
{
	uint8_t cpuid = plat_core_pos_by_mpidr(read_mpidr());
	uint8_t *cpu_suspend_list;
	uint32_t cpu_suspend_map = 0;
	uint8_t cpuidx;
	int		rc;

	cpu_suspend_list = get_cpu_suspend_list();
	for (cpuidx = 0 ; cpuidx < PLATFORM_CORE_COUNT ; cpuidx++) {
		if (cpu_suspend_list[cpuidx])
			SET_BIT(cpu_suspend_map, cpuidx);
	}
	INFO("cpu_suspend_map:0x%x\n", cpu_suspend_map);

	/* ask all cores except current one to do 'func' */
	rc = fiq_smp_call_function(
		PLATFORM_MAX_CPU_BIT_MASK & (~(1 << cpuid)),
		func, info, wait);
	assert(rc == 0);
}

void aee_wdt_dump_all_core(void __unused * cookie)
{
	mask_wdt_fiq();
	mtk_module_dump();
	wdt_notify_other_cores(aee_wdt_dump, 0, 0);
	aee_wdt_dump(NULL);
}
