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

#define EMIT_BPIALL		0xee070fd5
#define EMIT_SMC		0xe1600070
#define ESR_EL3_A64_SMC0	0x5e000000

#if DYNAMIC_WORKAROUND_CVE_2018_3639
	.globl	workaround_bpiall_runtime_exceptions
func	workaround_bpiall_runtime_exceptions
#else
	.globl	workaround_bpiall_vbar0_runtime_exceptions
.macro	enter_workaround _from_vector
#endif
	/*
	 * Save register state to enable a call to AArch32 S-EL1 and return
	 * Identify the original calling vector in w2 (==_from_vector)
	 * Use w3-w6 for additional register state preservation while in S-EL1
	 */

	/* Save GP regs */
	stp	x0, x1, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X0]
#if !DYNAMIC_WORKAROUND_CVE_2018_3639
	stp	x2, x3, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X2]
	stp	x4, x5, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X4]
#endif
	stp	x6, x7, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X6]
	stp	x8, x9, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X8]
	stp	x10, x11, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X10]
	stp	x12, x13, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X12]
	stp	x14, x15, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X14]
	stp	x16, x17, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X16]
	stp	x18, x19, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X18]
	stp	x20, x21, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X20]
	stp	x22, x23, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X22]
	stp	x24, x25, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X24]
	stp	x26, x27, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X26]
	stp	x28, x29, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X28]

#if !DYNAMIC_WORKAROUND_CVE_2018_3639
	/* Identify the original exception vector */
	mov	w2, \_from_vector

	/* Preserve 32-bit system registers in GP registers through the workaround */
	mrs	x3, esr_el3
	mrs	x4, spsr_el3
#endif
	mrs	x5, scr_el3
	mrs	x6, sctlr_el1

#if !DYNAMIC_WORKAROUND_CVE_2018_3639
	/*
	 * Preserve LR and ELR_EL3 registers in the GP regs context.
	 * Temporarily use the CTX_GPREG_SP_EL0 slot to preserve ELR_EL3
	 * through the workaround. This is OK because at this point the
	 * current state for this context's SP_EL0 is in the live system
	 * register, which is unmodified by the workaround.
	 */
	mrs	x7, elr_el3
	stp	x30, x7, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_LR]
#endif
	/*
	 * Load system registers for entry to S-EL1.
	 */

	/* Mask all interrupts and set AArch32 Supervisor mode */
	movz	w8, SPSR_MODE32(MODE32_svc, SPSR_T_ARM, SPSR_E_LITTLE, SPSR_AIF_MASK)

	/* Switch EL3 exception vectors while the workaround is executing. */
	adr	x9, workaround_bpiall_vbar1_runtime_exceptions

	/* Setup SCTLR_EL1 with MMU off and I$ on */
	ldr	x10, stub_sel1_sctlr

	/* Land at the S-EL1 workaround stub */
	adr	x11, aarch32_stub

	/*
	 * Setting SCR_EL3 to all zeroes means that the NS, RW
	 * and SMD bits are configured as expected.
	 */
	msr	scr_el3, xzr
	msr	spsr_el3, x8
	msr	vbar_el3, x9
	msr	sctlr_el1, x10
	msr	elr_el3, x11

	eret
#if DYNAMIC_WORKAROUND_CVE_2018_3639
endfunc workaround_bpiall_runtime_exceptions
#else
.endm
#endif

	/* ---------------------------------------------------------------------
	 * This vector table is used at runtime to enter the workaround at
	 * AArch32 S-EL1 for Sync/IRQ/FIQ/SError exceptions.  If the workaround
	 * is not enabled, the existing runtime exception vector table is used.
	 * ---------------------------------------------------------------------
	 */
#if !DYNAMIC_WORKAROUND_CVE_2018_3639
vector_base workaround_bpiall_vbar0_runtime_exceptions

	/* ---------------------------------------------------------------------
	 * Current EL with SP_EL0 : 0x0 - 0x200
	 * ---------------------------------------------------------------------
	 */
vector_entry workaround_bpiall_vbar0_sync_exception_sp_el0
	b	sync_exception_sp_el0
	check_vector_size workaround_bpiall_vbar0_sync_exception_sp_el0

vector_entry workaround_bpiall_vbar0_irq_sp_el0
	b	irq_sp_el0
	check_vector_size workaround_bpiall_vbar0_irq_sp_el0

vector_entry workaround_bpiall_vbar0_fiq_sp_el0
	b	fiq_sp_el0
	check_vector_size workaround_bpiall_vbar0_fiq_sp_el0

vector_entry workaround_bpiall_vbar0_serror_sp_el0
	b	serror_sp_el0
	check_vector_size workaround_bpiall_vbar0_serror_sp_el0

	/* ---------------------------------------------------------------------
	 * Current EL with SP_ELx: 0x200 - 0x400
	 * ---------------------------------------------------------------------
	 */
vector_entry workaround_bpiall_vbar0_sync_exception_sp_elx
	b	sync_exception_sp_elx
	check_vector_size workaround_bpiall_vbar0_sync_exception_sp_elx

vector_entry workaround_bpiall_vbar0_irq_sp_elx
	b	irq_sp_elx
	check_vector_size workaround_bpiall_vbar0_irq_sp_elx

vector_entry workaround_bpiall_vbar0_fiq_sp_elx
	b	fiq_sp_elx
	check_vector_size workaround_bpiall_vbar0_fiq_sp_elx

vector_entry workaround_bpiall_vbar0_serror_sp_elx
	b	serror_sp_elx
	check_vector_size workaround_bpiall_vbar0_serror_sp_elx

	/* ---------------------------------------------------------------------
	 * Lower EL using AArch64 : 0x400 - 0x600
	 * ---------------------------------------------------------------------
	 */
vector_entry workaround_bpiall_vbar0_sync_exception_aarch64
	enter_workaround 1
	check_vector_size workaround_bpiall_vbar0_sync_exception_aarch64

vector_entry workaround_bpiall_vbar0_irq_aarch64
	enter_workaround 2
	check_vector_size workaround_bpiall_vbar0_irq_aarch64

vector_entry workaround_bpiall_vbar0_fiq_aarch64
	enter_workaround 4
	check_vector_size workaround_bpiall_vbar0_fiq_aarch64

vector_entry workaround_bpiall_vbar0_serror_aarch64
	enter_workaround 8
	check_vector_size workaround_bpiall_vbar0_serror_aarch64

	/* ---------------------------------------------------------------------
	 * Lower EL using AArch32 : 0x600 - 0x800
	 * ---------------------------------------------------------------------
	 */
vector_entry workaround_bpiall_vbar0_sync_exception_aarch32
	enter_workaround 1
	check_vector_size workaround_bpiall_vbar0_sync_exception_aarch32

vector_entry workaround_bpiall_vbar0_irq_aarch32
	enter_workaround 2
	check_vector_size workaround_bpiall_vbar0_irq_aarch32

vector_entry workaround_bpiall_vbar0_fiq_aarch32
	enter_workaround 4
	check_vector_size workaround_bpiall_vbar0_fiq_aarch32

vector_entry workaround_bpiall_vbar0_serror_aarch32
	enter_workaround 8
	check_vector_size workaround_bpiall_vbar0_serror_aarch32
#endif
	/* ---------------------------------------------------------------------
	 * This vector table is used while the workaround is executing.  It
	 * installs a simple SMC handler to allow the Sync/IRQ/FIQ/SError
	 * workaround stubs to enter EL3 from S-EL1.  It restores the previous
	 * EL3 state before proceeding with the normal runtime exception vector.
	 * ---------------------------------------------------------------------
	 */
vector_base workaround_bpiall_vbar1_runtime_exceptions

	/* ---------------------------------------------------------------------
	 * Current EL with SP_EL0 : 0x0 - 0x200 (UNUSED)
	 * ---------------------------------------------------------------------
	 */
vector_entry workaround_bpiall_vbar1_sync_exception_sp_el0
	b	report_unhandled_exception
	nop	/* to force 8 byte alignment for the following stub */

	/*
	 * Since each vector table entry is 128 bytes, we can store the
	 * stub context in the unused space to minimize memory footprint.
	 */
stub_sel1_sctlr:
	.quad	SCTLR_AARCH32_EL1_RES1 | SCTLR_I_BIT

aarch32_stub:
	.word	EMIT_BPIALL
	.word	EMIT_SMC
	check_vector_size workaround_bpiall_vbar1_sync_exception_sp_el0

vector_entry workaround_bpiall_vbar1_irq_sp_el0
	b	report_unhandled_interrupt
	check_vector_size workaround_bpiall_vbar1_irq_sp_el0

vector_entry workaround_bpiall_vbar1_fiq_sp_el0
	b	report_unhandled_interrupt
	check_vector_size workaround_bpiall_vbar1_fiq_sp_el0

vector_entry workaround_bpiall_vbar1_serror_sp_el0
	b	report_unhandled_exception
	check_vector_size workaround_bpiall_vbar1_serror_sp_el0

	/* ---------------------------------------------------------------------
	 * Current EL with SP_ELx: 0x200 - 0x400 (UNUSED)
	 * ---------------------------------------------------------------------
	 */
vector_entry workaround_bpiall_vbar1_sync_exception_sp_elx
	b	report_unhandled_exception
	check_vector_size workaround_bpiall_vbar1_sync_exception_sp_elx

vector_entry workaround_bpiall_vbar1_irq_sp_elx
	b	report_unhandled_interrupt
	check_vector_size workaround_bpiall_vbar1_irq_sp_elx

vector_entry workaround_bpiall_vbar1_fiq_sp_elx
	b	report_unhandled_interrupt
	check_vector_size workaround_bpiall_vbar1_fiq_sp_elx

vector_entry workaround_bpiall_vbar1_serror_sp_elx
	b	report_unhandled_exception
	check_vector_size workaround_bpiall_vbar1_serror_sp_elx

	/* ---------------------------------------------------------------------
	 * Lower EL using AArch64 : 0x400 - 0x600 (UNUSED)
	 * ---------------------------------------------------------------------
	 */
vector_entry workaround_bpiall_vbar1_sync_exception_aarch64
	b	report_unhandled_exception
	check_vector_size workaround_bpiall_vbar1_sync_exception_aarch64

vector_entry workaround_bpiall_vbar1_irq_aarch64
	b	report_unhandled_interrupt
	check_vector_size workaround_bpiall_vbar1_irq_aarch64

vector_entry workaround_bpiall_vbar1_fiq_aarch64
	b	report_unhandled_interrupt
	check_vector_size workaround_bpiall_vbar1_fiq_aarch64

vector_entry workaround_bpiall_vbar1_serror_aarch64
	b	report_unhandled_exception
	check_vector_size workaround_bpiall_vbar1_serror_aarch64

	/* ---------------------------------------------------------------------
	 * Lower EL using AArch32 : 0x600 - 0x800
	 * ---------------------------------------------------------------------
	 */
vector_entry workaround_bpiall_vbar1_sync_exception_aarch32
	/*
	 * w2 indicates which SEL1 stub was run and thus which original vector was used
	 * w3-w6 contain saved system register state (esr_el3 in w3)
	 */
#if DYNAMIC_WORKAROUND_CVE_2018_3639
	/* Enable V4 mitigation */
	mrs	x0, CORTEX_A73_IMP_DEF_REG1
	orr	x0, x0, #CORTEX_A73_IMP_DEF_REG1_DISABLE_LOAD_PASS_STORE
	msr	CORTEX_A73_IMP_DEF_REG1, x0
	isb
#endif
	/* Apply the restored system register state */
	msr	esr_el3, x3
	msr	spsr_el3, x4
	msr	scr_el3, x5
	msr	sctlr_el1, x6

	/*
	 * Workaround is complete, so swap VBAR_EL3 to point
	 * to workaround entry table in preparation for subsequent
	 * Sync/IRQ/FIQ/SError exceptions.
	 */
#if DYNAMIC_WORKAROUND_CVE_2018_3639
	adr	x0, workaround_ssbd_vbar_runtime_exceptions
#else
	adr	x0, workaround_bpiall_vbar0_runtime_exceptions
#endif
	msr	vbar_el3, x0

	/*
	 * Restore all GP regs except x2 and x3 (esr).  The value in x2
	 * indicates the type of the original exception.
	 */
        bl 	restore_gp_registers

	/* Restore LR and ELR_EL3 register state from the GP regs context */
	ldp	x30, x7, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_LR]
	msr	elr_el3, x7
	ldp     x6, x7, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X6]

	/* Fast path Sync exceptions.  Static predictor will fall through. */
	tbz	w2, #0, workaround_not_sync

#if DYNAMIC_WORKAROUND_CVE_2018_3639
	orr	w2, wzr, #SMCCC_ARCH_WORKAROUND_2
	cmp	w0, w2
	mov_imm w2, ESR_EL3_A64_SMC0
	ccmp    w3, w2, #0, eq
	beq	handle_wa_2
#endif
	/*
	 * Check if SMC is coming from A64 state on #0
	 * with W0 = SMCCC_ARCH_WORKAROUND_1
	 *
	 * This sequence evaluates as:
	 *    (W0==SMCCC_ARCH_WORKAROUND_1) ? (ESR_EL3==SMC#0) : (NE)
	 * allowing use of a single branch operation
	 */
	orr	w2, wzr, #SMCCC_ARCH_WORKAROUND_1
	cmp	w0, w2
	mov_imm	w2, ESR_EL3_A64_SMC0
	ccmp	w3, w2, #0, eq
	/* Static predictor will predict a fall through */
	bne	1f
	eret
1:
	ldp	x2, x3, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X2]
	b	sync_exception_aarch64
	check_vector_size workaround_bpiall_vbar1_sync_exception_aarch32

vector_entry workaround_bpiall_vbar1_irq_aarch32
	b	report_unhandled_interrupt
#if DYNAMIC_WORKAROUND_CVE_2018_3639
handle_wa_2:
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

	/* Enable/Disable V4 mitigation */
 	mrs     x2, CORTEX_A73_IMP_DEF_REG1
	orr     x1, x2, #CORTEX_A73_IMP_DEF_REG1_DISABLE_LOAD_PASS_STORE
	bic	x3, x2, #CORTEX_A73_IMP_DEF_REG1_DISABLE_LOAD_PASS_STORE
	csel	x3, x3, x1, eq
	msr     CORTEX_A73_IMP_DEF_REG1, x3
	eret	/* ERET implies ISB */
#endif
	/*
	 * Post-workaround fan-out for non-sync exceptions
	 */
workaround_not_sync:
	tbnz	w2, #3, workaround_bpiall_vbar1_serror
	tbnz	w2, #2, workaround_bpiall_vbar1_fiq
	/* IRQ */
	ldp	x2, x3, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X2]
	b	irq_aarch64

workaround_bpiall_vbar1_fiq:
	ldp	x2, x3, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X2]
	b	fiq_aarch64

workaround_bpiall_vbar1_serror:
	ldp	x2, x3, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X2]
	b	serror_aarch64
	check_vector_size workaround_bpiall_vbar1_irq_aarch32

vector_entry workaround_bpiall_vbar1_fiq_aarch32
	b	report_unhandled_interrupt
restore_gp_registers:
	ldp	x0, x1, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X0]
	ldp	x4, x5, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X4]
	ldp	x8, x9, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X8]
	ldp	x10, x11, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X10]
	ldp	x12, x13, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X12]
	ldp	x14, x15, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X14]
	ldp	x16, x17, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X16]
	ldp	x18, x19, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X18]
	ldp	x20, x21, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X20]
	ldp	x22, x23, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X22]
	ldp	x24, x25, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X24]
	ldp	x26, x27, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X26]
	ldp	x28, x29, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X28]
	ret
	check_vector_size workaround_bpiall_vbar1_fiq_aarch32

vector_entry workaround_bpiall_vbar1_serror_aarch32
	b	report_unhandled_exception
	check_vector_size workaround_bpiall_vbar1_serror_aarch32
