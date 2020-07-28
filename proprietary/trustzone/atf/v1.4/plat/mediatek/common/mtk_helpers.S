/*
 * Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <asm_macros.S>
.globl	mtk_assert
.section .rodata.mtk_helpers, "aS"
assert_msg:
	.asciz "ASSERT: %s <%d> : %s\n"
func mtk_assert
	stp	x29, x30, [sp, #-16]!
	stp	x4, x5, [sp, #-16]!
	stp	x6, x7, [sp, #-16]!
	stp	x8, x9, [sp, #-16]!
	stp	x10, x11, [sp, #-16]!
	stp	x12, x13, [sp, #-16]!
	stp	x14, x15, [sp, #-16]!
	stp	x16, x17, [sp, #-16]!
	stp	x18, x19, [sp, #-16]!
	mov x1, x0
	adr	x0, assert_msg
	bl tf_printf
	bl plat_rgu_dump_reg
	ldp	x18, x19, [sp],#16
	ldp	x16, x17, [sp],#16
	ldp	x14, x15, [sp],#16
	ldp	x12, x13, [sp],#16
	ldp	x10, x11, [sp],#16
	ldp	x8, x9, [sp],#16
	ldp	x6, x7, [sp],#16
	ldp	x4, x5, [sp],#16
	ldp	x29, x30, [sp],#16
	ldp	x2, x3, [sp],#16
	ldp	x0, x1, [sp],#16
	b el3_panic
endfunc mtk_assert
