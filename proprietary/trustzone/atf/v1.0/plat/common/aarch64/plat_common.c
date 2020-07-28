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

#include <arch_helpers.h>
#include <xlat_tables.h>
#include <platform.h>
#include <debug.h>
#include <bl_common.h>
#include <assert.h>
#include <context.h>
#include <context_mgmt.h>
#include <fiq_smp_call.h>
#include <log.h>
#include <runtime_svc.h>
#include <console.h>
#include "plat_private.h"   //for atf_arg_t_ptr
#include <spinlock.h>

#if 0
/* For check mode test case */
#define MTK_AEE_TC_CHECK_MODE
#endif

#if 0
/* For early exception, coming cpu dump its stack */
#define MTK_AEE_TC_KWDT_NOT_READY
#endif

#define DCACHE_IS_ENABLE (read_sctlr_el3() & SCTLR_C_BIT)
spinlock_t aee_wdt_dump_lock;
extern atf_log_ctrl_t *p_atf_log_ctrl;
void aee_flush_log_cache (void)
{
	flush_dcache_range((unsigned long)p_atf_log_ctrl->info.atf_buf_addr, p_atf_log_ctrl->info.atf_buf_size);
}

extern void mt_log_set_crash_flag(void);
uint64_t wdt_kernel_cb_addr = 0;

/*
 * The following 2 platform setup functions are weakly defined. They
 * provide typical implementations that may be re-used by multiple
 * platforms but may also be overridden by a platform if required.
 */
#pragma weak bl31_plat_enable_mmu
#pragma weak bl32_plat_enable_mmu

#define atf_low_level_log(x)

void bl31_plat_enable_mmu(uint32_t flags)
{
	enable_mmu_el3(flags);
}

void bl32_plat_enable_mmu(uint32_t flags)
{
	enable_mmu_el1(flags);
}

void __attribute__((weak)) plat_pre_wdt_dump(void) {}

void aee_wdt_dump()
{
	struct atf_aee_regs *regs;
	cpu_context_t *ns_cpu_context;
	uint64_t mpidr = read_mpidr();
	uint64_t mode, reg_dump, invalid_mode = 0;
	uint32_t linear_id = platform_get_core_pos(mpidr);
	unsigned long sp_phys, count;
	uint64_t *pp;
	uint64_t spsr;
	int i;

	/* save context if from non-secure */
	if(read_scr_el3() & SCR_NS_BIT) {
		cm_el1_sysregs_context_save(NON_SECURE);
/*#if defined(CFG_MICROTRUST_TEE_SUPPORT)*/
#if 0
		cm_fpregs_context_save(NON_SECURE);
#endif
	}

	/* lock aee_wdt_dump_lock for fine print */
	if (DCACHE_IS_ENABLE)
		spin_lock(&aee_wdt_dump_lock);

	plat_pre_wdt_dump();

	/* To mark "AEE dump start" at dormant_log flag onto ram_console */
	atf_low_level_log(0xA7F00E00);

	/* Log starts here... */
	set_uart_flag();
	printf("%s: on cpu%d\n", __func__, (int)linear_id);

	ns_cpu_context = cm_get_context_by_mpidr(mpidr, NON_SECURE);
	/* compatible to the earlier chipset */
#if (defined(MACH_TYPE_MT6735) || defined(MACH_TYPE_MT6735M) || \
	defined(MACH_TYPE_MT6753) || defined(MACH_TYPE_MT8173))
	atf_arg_t_ptr teearg = (atf_arg_t_ptr)(uintptr_t)TEE_BOOT_INFO_ADDR;
#else
	atf_arg_t_ptr teearg = &gteearg;;
#endif
	regs = (void *)(teearg->atf_aee_debug_buf_start + (linear_id * sizeof(struct atf_aee_regs)));
	if (regs != NULL) {
		/* save debug infos */
		regs->pstate = SMC_GET_EL3(ns_cpu_context, CTX_SPSR_EL3)
		regs->pc = SMC_GET_EL3(ns_cpu_context, CTX_ELR_EL3)
		regs->sp = read_ctx_reg(get_sysregs_ctx(ns_cpu_context), CTX_SP_EL1);
		for (i=0; i<31; i++)
			regs->regs[i] = SMC_GET_GP(ns_cpu_context, (CTX_GPREG_X0 + (i<<3)));

		/* dump debug infos in pretty print */
		printf("(%d) pc:<%016lx> lr:<%016lx> sp:<%016lx> pstate: %lx\n",
			(int)linear_id, regs->pc, regs->regs[30], regs->sp, regs->pstate);
		for(i=29; i>=0; i-=3)
			printf("(%d) x%02d: %016lx x%02d: %016lx x%02d: %016lx\n",
				(int)linear_id, i, regs->regs[i], (i-1), regs->regs[i-1], (i-2), regs->regs[i-2]);
	}

#ifdef MTK_AEE_TC_KWDT_NOT_READY
        wdt_kernel_cb_addr = 0;
#endif

	if(0 == wdt_kernel_cb_addr) {

		printf("Kernel WDT not ready. cpu%d\n", (int)linear_id);

		if (regs != NULL) {
			if (0 == regs->sp) {
				printf("NULL sp, skip stack dump\n");
			} else {
				/* physical address of sp */
				sp_phys = (regs->sp - 0xffffffc000000000) + 0x40000000;
				printf("sp_phys:0x%lx\n", sp_phys);

				printf("sp_phys(aligned):0x%lx\n", (sp_phys & ~(PAGE_SIZE_2MB_MASK)));

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

				printf("dump sp(16k), count:%ld, mask: 0x3fff\n", count);
				for(i=0; i<count ; i+=4,pp+=4)
					INFO("%016lx| %016lx %016lx %016lx %016lx\n",(unsigned long)(uintptr_t)pp, \
					(unsigned long)(uintptr_t)*pp, (unsigned long)(uintptr_t)*(pp+1), \
					(unsigned long)(uintptr_t)*(pp+2), (unsigned long)(uintptr_t)*(pp+3));
			}
		}
		mt_log_set_crash_flag();
		printf("Wait timeout.\n");
		aee_flush_log_cache();
		if (DCACHE_IS_ENABLE)
			spin_unlock(&aee_wdt_dump_lock);
		while(1);
	}

	/* release aee_wdt_dump lock */
	if (DCACHE_IS_ENABLE)
		spin_unlock(&aee_wdt_dump_lock);

	/* default enter EL1(64b) or SVC(32b) when enter AEE dump */
	spsr = SMC_GET_EL3(ns_cpu_context, CTX_SPSR_EL3);
	/* Check mode validation */
	if (get_kernel_k32_64() == LINUX_KERNEL_32) {
		mode = (spsr & ((MODE32_MASK) << MODE32_SHIFT)) >> MODE32_SHIFT;
		if (mode != MODE32_svc)
			invalid_mode = 2;
	} else {
		mode = (spsr & ((MODE_EL_MASK) << MODE_EL_SHIFT)) >> MODE_EL_SHIFT;
		if (mode != MODE_EL1)
			invalid_mode = 1;
	}

	/* Dump context near by */
	if (invalid_mode > 0) {
		if (invalid_mode == 2)
			INFO("Mode should be MODE32_svc\n");

		if (invalid_mode == 1)
			INFO("Mode should be MODE_EL1\n");

		for (i = 0 ; i < 4; i++) {
			reg_dump = SMC_GET_EL3(ns_cpu_context, (CTX_SCR_EL3 + (i<<3)));
			INFO("EL3_STATE + %d=0x%lx\n", i<<3, reg_dump);
		}
		aee_flush_log_cache();
		while (1)
			;
	}
	if (LINUX_KERNEL_32 == get_kernel_k32_64()){
		spsr = (spsr & ~((MODE32_MASK) << MODE32_SHIFT));
		spsr = (spsr | ((MODE32_svc) << MODE32_SHIFT));
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

	/* wdt kernel callback addr should be ready now... */
	SMC_SET_EL3(ns_cpu_context, CTX_ELR_EL3, wdt_kernel_cb_addr);

	/* Restore non-secure state */
	cm_el1_sysregs_context_restore(NON_SECURE);
/*#if defined(CFG_MICROTRUST_TEE_SUPPORT)*/
#if 0
	cm_fpregs_context_restore(NON_SECURE);
#endif
	cm_set_next_eret_context(NON_SECURE);

	/* To mark "AEE dump End" at dormant_log flag onto ram_console */
	atf_low_level_log(0xA7F00E01);
}

void __attribute__((weak)) dump_wfi_spill(void) {}
