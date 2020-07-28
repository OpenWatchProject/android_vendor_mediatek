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
#include <mt_common.h>
#include <plat_private.h>
#include <platform.h>
#include <runtime_svc.h>
#include <spinlock.h>
#include <stdio.h>
#include <xlat_tables.h>

#define LINUX_KERNEL_64BIT_MTK_OFFSET	0x80000
#define LINUX_KERNEL_32BIT_MTK_OFFSET	0x8000
#define LINUX_KERNEL_64BIT_VA_OFFSET	0xffffff8000000000
#define LINUX_KERNEL_32BIT_VA_OFFSET	0xc0000000

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
//#define MODE32_SVC_LR_REG	18

extern atf_arg_t gteearg;
extern uint64_t wdt_kernel_cb_addr;
extern uint64_t get_kernel_k32_64(void);

spinlock_t aee_wdt_dump_lock;
extern atf_log_ctrl_t *p_atf_log_ctrl;

uint64_t mtk_lk_stage = 1;
#ifdef MTK_HWT_ELR_DEBUG
uint32_t aee_dump_happen;
#endif
void aee_flush_log_cache(void)
{
	flush_dcache_range((unsigned long)p_atf_log_ctrl->info.atf_log_addr,
		p_atf_log_ctrl->info.atf_log_size);
}
void aee_wdt_dump(void __unused *cookie)
{
	struct atf_aee_regs *regs;
	cpu_context_t *ns_cpu_context;
	uint64_t mpidr = read_mpidr();
	uint32_t linear_id = platform_get_core_pos(mpidr);
	unsigned long sp_phys, count;
	uint64_t *pp;
	uint64_t spsr;
	int i;
	uint64_t sp_el1;
#ifdef MTK_HWT_ELR_DEBUG
	uint64_t elr_el3_readback;

#endif
	/* save context if from non-secure */
	if(read_scr_el3() & SCR_NS_BIT)
		cm_el1_sysregs_context_save(NON_SECURE);

	ns_cpu_context = cm_get_context_by_mpidr(mpidr, NON_SECURE);
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
	if (mtk_lk_stage || (0 == wdt_kernel_cb_addr))
		console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE);

	/* Log starts here... */
	INFO("%s: on cpu%d\n", __func__, (int)linear_id);

	/* compatible to the earlier chipset */
#if (defined(MACH_TYPE_MT6735) || defined(MACH_TYPE_MT6735M) || \
	defined(MACH_TYPE_MT6753) || defined(MACH_TYPE_MT8173))
	atf_arg_t_ptr teearg = (atf_arg_t_ptr)(uintptr_t)TEE_BOOT_INFO_ADDR;
#else
	atf_arg_t_ptr teearg = &gteearg;
#endif
	regs = (void *)(teearg->atf_aee_debug_buf_start + (linear_id * sizeof(struct atf_aee_regs)));

	/* save debug infos */
	regs->pstate = SMC_GET_EL3(ns_cpu_context, CTX_SPSR_EL3)
	regs->pc = SMC_GET_EL3(ns_cpu_context, CTX_ELR_EL3);

	for (i=0; i<31; i++)
		regs->regs[i] = SMC_GET_GP(ns_cpu_context, (CTX_GPREG_X0 + (i<<3)));
	if (GET_RW(spsr) == MODE_RW_32)
		regs->sp = regs->regs[MODE32_SVC_SP_REG]; /* SVC SP */
	else
		regs->sp = read_ctx_reg(get_sysregs_ctx(ns_cpu_context), CTX_SP_EL1);

	/* dump debug infos in pretty print */
	INFO("(%d) pc:<%016lx> lr:<%016lx> sp:<%016lx> pstate=%lx\n",
		(int)linear_id, regs->pc, regs->regs[30], regs->sp, regs->pstate);
	for (i=29; i>=0; i-=3) {
		INFO("(%d) x%02d: %016lx x%02d: %016lx x%02d: %016lx\n",
			(int)linear_id, i, regs->regs[i], (i-1), regs->regs[i-1], (i-2), regs->regs[i-2]);
	}

#ifdef MTK_AEE_TC_KWDT_NOT_READY
	wdt_kernel_cb_addr = 0;
#endif

	if (0 == wdt_kernel_cb_addr) {

		NOTICE("%s WDT not ready\n", mtk_lk_stage ? "LK" : "Kernel");
		/* Do not dump SP in kernel  */
		if ((0 != regs->sp) && (mtk_lk_stage == 1)) {
			if (GET_RW(spsr) == MODE_RW_64) {
				sp_phys = (regs->sp - LINUX_KERNEL_64BIT_VA_OFFSET) + DRAM_EMI_BASE;
			} else if(GET_RW(spsr) == MODE_RW_32 && mtk_lk_stage == 0)
				sp_phys = (regs->sp - LINUX_KERNEL_32BIT_VA_OFFSET) + DRAM_EMI_BASE;
			else
				sp_phys = regs->sp;
			NOTICE("sp_phys:0x%lx\n", sp_phys);

			/* map spphyscial memory for 2MB */
			mmap_add_region((sp_phys & ~(PAGE_SIZE_2MB_MASK)),
					(sp_phys & ~(PAGE_SIZE_2MB_MASK)),
					PAGE_SIZE_2MB,
					MT_DEVICE | MT_RW | MT_NS);

			/* re-fill translation table */
			init_xlat_tables();

			/* flush sp content  */
			flush_dcache_range((uint64_t)sp_phys, (uint64_t)0x2000);

			/* dump 8k */
			pp = (uint64_t *)(uintptr_t)sp_phys;
			count = (0x4000 - (sp_phys &(0x4000-1)))/8;

			NOTICE("dump sp(16k), count:%ld, mask: 0x3fff\n", count);
			for(i=0; i<count ; i+=4,pp+=4)
				INFO("%016lx| %016lx %016lx %016lx %016lx\n",(unsigned long)(uintptr_t)pp, \
				(unsigned long)(uintptr_t)*pp, (unsigned long)(uintptr_t)*(pp+1), \
				(unsigned long)(uintptr_t)*(pp+2), (unsigned long)(uintptr_t)*(pp+3));
		}
		NOTICE("Wait timeout.\n");
		aee_flush_log_cache();
		if (DCACHE_IS_ENABLE)
			spin_unlock(&aee_wdt_dump_lock);
		while(1);
	}
	/* wdt kernel callback addr should be ready now... */
	/* This cpu context maybe not initialized completely in EL1,
		do not enter EL1 wdt callback */
	if (sp_el1 != 0)
		SMC_SET_EL3(ns_cpu_context, CTX_ELR_EL3, wdt_kernel_cb_addr);
#ifdef MTK_HWT_ELR_DEBUG
	if(!mtk_lk_stage) {
		elr_el3_readback = SMC_GET_EL3(ns_cpu_context, CTX_ELR_EL3);
		INFO("ELR_C_RB:0x%lx\n", elr_el3_readback);
	}
#endif
	/* release aee_wdt_dump_lock */
	if (DCACHE_IS_ENABLE)
		spin_unlock(&aee_wdt_dump_lock);

	/* default enter EL1(64b) or SVC(32b) when enter AEE dump */
	if(mtk_lk_stage || (LINUX_KERNEL_32 == get_kernel_k32_64())) {
		spsr = (spsr & ~((MODE32_MASK) << MODE32_SHIFT));
		spsr = (spsr | ((MODE32_svc) << MODE32_SHIFT));
		/*
		 * Check Thumb state or ARM state of the callback function.
		 * If EL1 raised the WDT timeout from ARM state, but the callback is
		 * Thumb state, we should correct the T bit.
		 */
		spsr = (spsr & ~((SPSR_T_MASK) << SPSR_T_SHIFT));
		spsr = (spsr | ((wdt_kernel_cb_addr & SPSR_T_THUMB) << SPSR_T_SHIFT));
	} else {
		spsr = (spsr & ~((MODE_EL_MASK << MODE_EL_SHIFT)|(MODE_SP_MASK << MODE_SP_SHIFT)|(MODE_RW_MASK << MODE_RW_SHIFT)));
		spsr = (spsr | (MODE_EL1 << MODE_EL_SHIFT)|(MODE_SP_ELX << MODE_SP_SHIFT)|(MODE_RW_64 << MODE_RW_SHIFT));
	}

	/* disable IRQ when enter AEE dump */
	spsr = (spsr | (DAIF_IRQ_BIT << SPSR_AIF_SHIFT));
#ifdef MTK_AEE_TC_CHECK_MODE
	/* ready to make test case */
	if (linear_id == 0) {
		spsr = 0x000000000000008d;
		SMC_SET_EL3(ns_cpu_context, CTX_SPSR_EL3, spsr);
	}
#else
	SMC_SET_EL3(ns_cpu_context, CTX_SPSR_EL3, spsr);
#endif
	/* Restore non-secure state */
	cm_el1_sysregs_context_restore(NON_SECURE);
	cm_set_next_eret_context(NON_SECURE);
	aee_flush_log_cache();
}
void aee_wdt_dump_all_core(void __unused *cookie) {
#ifdef MTK_HWT_ELR_DEBUG
	aee_dump_happen = 1;
#endif
	mt_fiq_smp_call_function(aee_wdt_dump, 0, 0);
	aee_wdt_dump(NULL);
}
