#include <asm_macros.S>
#include "tkcored_private.h"

	.global	tkcored_enter_sp

func tkcored_enter_sp
	/* Make space for the registers that we're going to save */
	mov	x3, sp
	str	x3, [x0, #0]
	sub	sp, sp, #TKCORED_C_RT_CTX_SIZE

	/* Save callee-saved registers on to the stack */
	stp	x19, x20, [sp, #TKCORED_C_RT_CTX_X19]
	stp	x21, x22, [sp, #TKCORED_C_RT_CTX_X21]
	stp	x23, x24, [sp, #TKCORED_C_RT_CTX_X23]
	stp	x25, x26, [sp, #TKCORED_C_RT_CTX_X25]
	stp	x27, x28, [sp, #TKCORED_C_RT_CTX_X27]
	stp	x29, x30, [sp, #TKCORED_C_RT_CTX_X29]

	mrs	x19, scr_el3
	stp	x19, xzr, [sp, #TKCORED_C_RT_CTX_SCR]

	b	el3_exit
endfunc tkcored_enter_sp

	.global tkcored_exit_sp
func tkcored_exit_sp
	/* Restore the previous stack */
	mov	sp, x0

	/* Restore scr_el3 */
	ldp	x19, xzr, [sp, #(TKCORED_C_RT_CTX_SCR - TKCORED_C_RT_CTX_SIZE)]
	msr	scr_el3, x19

	/* Restore callee-saved registers on to the stack */
	ldp	x19, x20, [x0, #(TKCORED_C_RT_CTX_X19 - TKCORED_C_RT_CTX_SIZE)]
	ldp	x21, x22, [x0, #(TKCORED_C_RT_CTX_X21 - TKCORED_C_RT_CTX_SIZE)]
	ldp	x23, x24, [x0, #(TKCORED_C_RT_CTX_X23 - TKCORED_C_RT_CTX_SIZE)]
	ldp	x25, x26, [x0, #(TKCORED_C_RT_CTX_X25 - TKCORED_C_RT_CTX_SIZE)]
	ldp	x27, x28, [x0, #(TKCORED_C_RT_CTX_X27 - TKCORED_C_RT_CTX_SIZE)]
	ldp	x29, x30, [x0, #(TKCORED_C_RT_CTX_X29 - TKCORED_C_RT_CTX_SIZE)]

	mov	x0, x1
	ret
endfunc tkcored_exit_sp
