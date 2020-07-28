/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arm_arch_svc.h>
#include <asm_macros.S>
#include <context.h>
#include <cortex_a73.h>

	.globl	workaround_ssbd_vbar_runtime_exceptions
#if WORKAROUND_CVE_2017_5715
	/*
	 * This macro applies the mitigation for CVE-2018-3639.
	 * It implements a fash path where `SMCCC_ARCH_WORKAROUND_2`
	 * SMC calls from a lower EL running in AArch32 or AArch64
	 * will go through the fast and return early.
	 *
	 * The macro saves x2-x3 to the context.  In the fast path
	 * x0-x3 registers do not need to be restored as the calling
	 * context will have saved them.
	 */
	.macro apply_cve_2018_3639_wa _from_vector _jump_vector
	stp	x2, x3, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X2]

	/* Enable V4 mitigation Early*/
	mrs	x2, CORTEX_A73_IMP_DEF_REG1
	orr	x3, x2, #CORTEX_A73_IMP_DEF_REG1_DISABLE_LOAD_PASS_STORE
	msr	CORTEX_A73_IMP_DEF_REG1, x3
	isb

	/*
	 * Preserve LR and ELR_EL3 registers in the GP regs context.
	 * Temporarily use the CTX_GPREG_SP_EL0 slot to preserve ELR_EL3
	 * through the workaround. This is OK because at this point the
	 * current state for this context's SP_EL0 is in the live system
	 * register, which is unmodified by the workaround.
	 */
	mrs	x2, elr_el3
	stp	x30, x2, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_LR]
	stp	x4, x5, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X4]

	/* Preserve 32-bit system registers in GP registers through the workaround */
	mrs	x3, esr_el3
	mrs	x4, spsr_el3

	/* Mask all interrupts and set AArch64 EL3 mode with current SP*/
	movz	w2, SPSR_64(MODE_EL3, MODE_SP_ELX, SPSR_AIF_MASK)
	msr	spsr_el3, x2

	/* Land at the EL3 workaround stub */
	adr	x2, \_jump_vector
	msr	elr_el3, x2

	/* Identify the original exception vector */
	mov	w2, \_from_vector
	eret

	.endm

	/* ---------------------------------------------------------------------
	 * This vector table is used at runtime to enter the workaround at
	 * AArch32 S-EL1 for Sync/IRQ/FIQ/SError exceptions.  If the workaround
	 * is not enabled, the existing runtime exception vector table is used.
	 * ---------------------------------------------------------------------
	 */
vector_base workaround_ssbd_vbar_runtime_exceptions

	/* ---------------------------------------------------------------------
	 * Current EL with SP_EL0 : 0x0 - 0x200
	 * ---------------------------------------------------------------------
	 */
vector_entry workaround_ssbd_vbar_sync_exception_sp_el0
	b	sync_exception_sp_el0
	check_vector_size workaround_ssbd_vbar_sync_exception_sp_el0

vector_entry workaround_ssbd_vbar_irq_sp_el0
	b	irq_sp_el0
	check_vector_size workaround_ssbd_vbar_irq_sp_el0

vector_entry workaround_ssbd_vbar_fiq_sp_el0
	b	fiq_sp_el0
	check_vector_size workaround_ssbd_vbar_fiq_sp_el0

vector_entry workaround_ssbd_vbar_serror_sp_el0
	b	serror_sp_el0
	check_vector_size workaround_ssbd_vbar_serror_sp_el0

	/* ---------------------------------------------------------------------
	 * Current EL with SP_ELx: 0x200 - 0x400
	 * ---------------------------------------------------------------------
	 */
vector_entry workaround_ssbd_vbar_sync_exception_sp_elx
	b	sync_exception_sp_elx
	check_vector_size workaround_ssbd_vbar_sync_exception_sp_elx

vector_entry workaround_ssbd_vbar_irq_sp_elx
	b	irq_sp_elx
	check_vector_size workaround_ssbd_vbar_irq_sp_elx

vector_entry workaround_ssbd_vbar_fiq_sp_elx
	b	fiq_sp_elx
	check_vector_size workaround_ssbd_vbar_fiq_sp_elx

vector_entry workaround_ssbd_vbar_serror_sp_elx
	b	serror_sp_elx
	check_vector_size workaround_ssbd_vbar_serror_sp_elx

	/* ---------------------------------------------------------------------
	 * Lower EL using AArch64 : 0x400 - 0x600
	 * ---------------------------------------------------------------------
	 */
vector_entry workaround_ssbd_vbar_sync_exception_aarch64
	apply_cve_2018_3639_wa 1 workaround_bpiall_runtime_exceptions
	check_vector_size workaround_ssbd_vbar_sync_exception_aarch64

vector_entry workaround_ssbd_vbar_irq_aarch64
	apply_cve_2018_3639_wa 2 workaround_bpiall_runtime_exceptions
	check_vector_size workaround_ssbd_vbar_irq_aarch64

vector_entry workaround_ssbd_vbar_fiq_aarch64
	apply_cve_2018_3639_wa 4 workaround_bpiall_runtime_exceptions
	check_vector_size workaround_ssbd_vbar_fiq_aarch64

vector_entry workaround_ssbd_vbar_serror_aarch64
	apply_cve_2018_3639_wa 8 workaround_bpiall_runtime_exceptions
	check_vector_size workaround_ssbd_vbar_serror_aarch64

	/* ---------------------------------------------------------------------
	 * Lower EL using AArch32 : 0x600 - 0x800
	 * ---------------------------------------------------------------------
	 */
vector_entry workaround_ssbd_vbar_sync_exception_aarch32
	apply_cve_2018_3639_wa 1 workaround_bpiall_runtime_exceptions
	check_vector_size workaround_ssbd_vbar_sync_exception_aarch32

vector_entry workaround_ssbd_vbar_irq_aarch32
	apply_cve_2018_3639_wa 2 workaround_bpiall_runtime_exceptions
	check_vector_size workaround_ssbd_vbar_irq_aarch32

vector_entry workaround_ssbd_vbar_fiq_aarch32
	apply_cve_2018_3639_wa 4 workaround_bpiall_runtime_exceptions
	check_vector_size workaround_ssbd_vbar_fiq_aarch32

vector_entry workaround_ssbd_vbar_serror_aarch32
	apply_cve_2018_3639_wa 8 workaround_bpiall_runtime_exceptions
	check_vector_size workaround_ssbd_vbar_serror_aarch32

#else

#define ESR_EL3_A64_SMC0	0x5e000000
#define ESR_EL3_A32_SMC0	0x4e000000

	/*
	 * This macro applies the mitigation for CVE-2018-3639.
	 * It implements a fash path where `SMCCC_ARCH_WORKAROUND_2`
	 * SMC calls from a lower EL running in AArch32 or AArch64
	 * will go through the fast and return early.
	 *
	 * The macro saves x2-x3 to the context.  In the fast path
	 * x0-x3 registers do not need to be restored as the calling
	 * context will have saved them.
	 */
	.macro apply_cve_2018_3639_wa _is_sync_exception _esr_el3_val
	stp	x2, x3, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X2]

	.if \_is_sync_exception
		/*
		 * Ensure SMC is coming from A64/A32 state on #0
		 * with W0 = SMCCC_ARCH_WORKAROUND_2
		 *
		 * This sequence evaluates as:
		 *    (W0==SMCCC_ARCH_WORKAROUND_2) ? (ESR_EL3==SMC#0) : (NE)
		 * allowing use of a single branch operation
		 */
		orr	w2, wzr, #SMCCC_ARCH_WORKAROUND_2
		cmp	x0, x2
		mrs	x3, esr_el3
		mov_imm	w2, \_esr_el3_val
		ccmp	w2, w3, #0, eq
		/*
		 * Static predictor will predict a fall-through, optimizing
		 * the `SMCCC_ARCH_WORKAROUND_2` fast path.
		 */
		bne	1f

		/*
		 * The sequence below implements the `SMCCC_ARCH_WORKAROUND_2`
		 * fast path.
		 */
		cmp	x1, xzr /* enable/disable check */

		/*
		 * When the calling context wants mitigation disabled,
		 * we program the mitigation disable function in the
		 * CPU context, which gets invoked on subsequent exits from
		 * EL3 via the `el3_exit` function.  Otherwise NULL is
		 * programmed in the CPU context, which results in caller's
		 * inheriting the EL3 mitigation state (enabled) on subsequent
		 * `el3_exit`.
		 */
		mov	x0, xzr
		adr	x1, cortex_a73_disable_wa_cve_2018_3639
		csel	x1, x1, x0, eq
		str	x1, [sp, #CTX_CVE_2018_3639_OFFSET + CTX_CVE_2018_3639_DISABLE]

		mrs	x2, CORTEX_A73_IMP_DEF_REG1
		orr	x1, x2, #CORTEX_A73_IMP_DEF_REG1_DISABLE_LOAD_PASS_STORE
		bic	x3, x2, #CORTEX_A73_IMP_DEF_REG1_DISABLE_LOAD_PASS_STORE
		csel	x3, x3, x1, eq
		msr	CORTEX_A73_IMP_DEF_REG1, x3
		eret	/* ERET implies ISB */
	.endif
1:
	/*
	 * Always enable v4 mitigation during EL3 execution.  This is not
	 * required for the fast path above because it does not perform any
	 * memory loads.
	 */
	mrs	x2, CORTEX_A73_IMP_DEF_REG1
	orr	x2, x2, #CORTEX_A73_IMP_DEF_REG1_DISABLE_LOAD_PASS_STORE
	msr	CORTEX_A73_IMP_DEF_REG1, x2
	isb

	/*
	 * The caller may have passed arguments to EL3 via x2-x3.
	 * Restore these registers from the context before jumping to the
	 * main runtime vector table entry.
	 */
	ldp	x2, x3, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X2]
	.endm

vector_base workaround_ssbd_vbar_runtime_exceptions

	/* ---------------------------------------------------------------------
	 * Current EL with SP_EL0 : 0x0 - 0x200
	 * ---------------------------------------------------------------------
	 */
vector_entry cortex_a73_sync_exception_sp_el0
	b	sync_exception_sp_el0
check_vector_size cortex_a73_sync_exception_sp_el0

vector_entry cortex_a73_irq_sp_el0
	b	irq_sp_el0
check_vector_size cortex_a73_irq_sp_el0

vector_entry cortex_a73_fiq_sp_el0
	b	fiq_sp_el0
check_vector_size cortex_a73_fiq_sp_el0

vector_entry cortex_a73_serror_sp_el0
	b	serror_sp_el0
check_vector_size cortex_a73_serror_sp_el0

	/* ---------------------------------------------------------------------
	 * Current EL with SP_ELx: 0x200 - 0x400
	 * ---------------------------------------------------------------------
	 */
vector_entry cortex_a73_sync_exception_sp_elx
	b	sync_exception_sp_elx
check_vector_size cortex_a73_sync_exception_sp_elx

vector_entry cortex_a73_irq_sp_elx
	b	irq_sp_elx
check_vector_size cortex_a73_irq_sp_elx

vector_entry cortex_a73_fiq_sp_elx
	b	fiq_sp_elx
check_vector_size cortex_a73_fiq_sp_elx

vector_entry cortex_a73_serror_sp_elx
	b	serror_sp_elx
check_vector_size cortex_a73_serror_sp_elx

	/* ---------------------------------------------------------------------
	 * Lower EL using AArch64 : 0x400 - 0x600
	 * ---------------------------------------------------------------------
	 */
vector_entry cortex_a73_sync_exception_aarch64
	apply_cve_2018_3639_wa _is_sync_exception=1 _esr_el3_val=ESR_EL3_A64_SMC0
	b	sync_exception_aarch64
check_vector_size cortex_a73_sync_exception_aarch64

vector_entry cortex_a73_irq_aarch64
	apply_cve_2018_3639_wa _is_sync_exception=0 _esr_el3_val=ESR_EL3_A64_SMC0
	b	irq_aarch64
check_vector_size cortex_a73_irq_aarch64

vector_entry cortex_a73_fiq_aarch64
	apply_cve_2018_3639_wa _is_sync_exception=0 _esr_el3_val=ESR_EL3_A64_SMC0
	b	fiq_aarch64
check_vector_size cortex_a73_fiq_aarch64

vector_entry cortex_a73_serror_aarch64
	apply_cve_2018_3639_wa _is_sync_exception=0 _esr_el3_val=ESR_EL3_A64_SMC0
	b	serror_aarch64
check_vector_size cortex_a73_serror_aarch64

	/* ---------------------------------------------------------------------
	 * Lower EL using AArch32 : 0x600 - 0x800
	 * ---------------------------------------------------------------------
	 */
vector_entry cortex_a73_sync_exception_aarch32
	apply_cve_2018_3639_wa _is_sync_exception=1 _esr_el3_val=ESR_EL3_A32_SMC0
	b	sync_exception_aarch32
check_vector_size cortex_a73_sync_exception_aarch32

vector_entry cortex_a73_irq_aarch32
	apply_cve_2018_3639_wa _is_sync_exception=0 _esr_el3_val=ESR_EL3_A32_SMC0
	b	irq_aarch32
check_vector_size cortex_a73_irq_aarch32

vector_entry cortex_a73_fiq_aarch32
	apply_cve_2018_3639_wa _is_sync_exception=0 _esr_el3_val=ESR_EL3_A32_SMC0
	b	fiq_aarch32
check_vector_size cortex_a73_fiq_aarch32

vector_entry cortex_a73_serror_aarch32
	apply_cve_2018_3639_wa _is_sync_exception=0 _esr_el3_val=ESR_EL3_A32_SMC0
	b	serror_aarch32
check_vector_size cortex_a73_serror_aarch32

func cortex_a73_disable_wa_cve_2018_3639

	mrs	x0, CORTEX_A73_IMP_DEF_REG1
	bic	x0, x0, #CORTEX_A73_IMP_DEF_REG1_DISABLE_LOAD_PASS_STORE
	msr	CORTEX_A73_IMP_DEF_REG1, x0
	isb
	ret
endfunc cortex_a73_disable_wa_cve_2018_3639
#endif
