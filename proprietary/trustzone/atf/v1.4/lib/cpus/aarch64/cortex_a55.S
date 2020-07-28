/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cortex_a55.h>
#include <arch.h>
#include <asm_macros.S>
#include <bl_common.h>
#include <cpu_macros.S>
#include <plat_macros.S>

	/* ---------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ---------------------------------------------
	 */
func cortex_a55_core_pwr_dwn
	/* ---------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------
	 */
	mrs	x0, CORTEX_A55_CPUPWRCTLR_EL1
	orr	x0, x0, #CORTEX_A55_CORE_PWRDN_EN_MASK
	msr	CORTEX_A55_CPUPWRCTLR_EL1, x0
	isb
	ret
endfunc cortex_a55_core_pwr_dwn

	/* ---------------------------------------------
	 * This function provides cortex_a55 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_a55_regs, "aS"
cortex_a55_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_a55_cpu_reg_dump
	adr	x6, cortex_a55_regs
	mrs	x8, CORTEX_A55_CPUECTLR_EL1
	ret
endfunc cortex_a55_cpu_reg_dump


	/* -------------------------------------------------
	 * The CPU Ops reset function for Cortex-A55.
	 * Shall clobber: x0-x19
	 * -------------------------------------------------
	 */
func cortex_a55_reset_func
	mov	x19, x30

	/* Performance Management Registers enable */
	/* Make sure accesses from EL0/EL1 are not trapped to EL2 */
	mrs	x0, actlr_el2
	orr	x0, x0, #(1 << 12)
	msr	actlr_el2, x0
	isb

	/* Make sure accesses from EL0/EL1 and EL2 are not trapped to EL3 */
	mrs	x0, actlr_el3
	orr	x0, x0, #(1 << 12)
	msr	actlr_el3, x0
	isb

	ret	x19
endfunc cortex_a55_reset_func


declare_cpu_ops cortex_a55, CORTEX_A55_MIDR, \
	cortex_a55_reset_func, \
	cortex_a55_core_pwr_dwn
