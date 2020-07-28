/*
 * Copyright (c) 2014-2015, ARM Limited and Contributors. All rights reserved.
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

#include <cci.h>
#include <gic_v2.h>
#include <platform_def.h>

.section .rodata.gic_reg_name, "aS"
gicc_regs:
	.asciz "gicc_hppir", "gicc_ahppir", "gicc_ctlr", ""
gicd_pend_reg:
	.asciz "gicd_ispendr regs (Offsets 0x200 - 0x278)\n"	\
		" Offset:\t\t\tvalue\n"
infra_topaxi_str:
	.asciz "INFRA_TOPAXI_SI0_CTL 0x10001200: "
print_abort_address_content:
	.asciz "Dump ELR_EL3 addr "
print_abort_address_invalid:
	.asciz "addr INVALID!"
print_abort_address_content_colon:
	.asciz ":  "
newline:
	.asciz "\n"
spacer:
	.asciz ":\t\t0x"

	/* ---------------------------------------------
	 * The below macro prints out relevant GIC
	 * registers whenever an unhandled exception is
	 * taken in BL3-1.
	 * Clobbers: x0 - x10, x16, x17, sp
	 * ---------------------------------------------
	 */
	.macro plat_crash_print_regs
	adr	x4, infra_topaxi_str
	bl	asm_print_str
	mov_imm x16, INFRACFG_AO_BASE
	ldr	w4, [x16, #0x200]
	bl	asm_print_hex
	adr	x4, newline
	bl	asm_print_str
	/* ELR_EL3 instruction content */
	adr	x4, print_abort_address_content
	bl	asm_print_str
	mrs	x4, elr_el3
	mov x16, x4
	bl	asm_print_hex
	mov x4, x16
#ifdef DRAM_EXTENSION_SUPPORT
	/* Address sanity check */
	adr x1, __RO_START__
	cmp x4, x1
	b.ls	1f
	adr x2, __RO_END__
	mov w1, #0x1
	cmp x4, x2
	b.cc 2f
1:
	ldr x2, =__DRAM_RO_START__
	mov w1, #0x0
	cmp x4, x2
	b.ls 2f
	ldr x2, =__DRAM_RO_END__
	cmp x4, x2
	cset    w1, cc
2:
	mov w0, w1
	cmp w0, #1
	b.ne NEQ
#endif
	adr	x4, print_abort_address_content_colon
	bl	asm_print_str
	ldr	w4, [x16]
	bl	asm_print_hex
	adr	x4, newline
	bl	asm_print_str
	b	DONE
NEQ:
	adr	x4, print_abort_address_invalid
	bl	asm_print_str
	adr	x4, newline
	bl	asm_print_str
DONE:
#if 0
	mov_imm x16, BASE_GICD_BASE
	mov_imm x17, BASE_GICC_BASE
	/* Load the gicc reg list to x6 */
	adr	x6, gicc_regs
	/* Load the gicc regs to gp regs used by str_in_crash_buf_print */
	ldr	w8, [x17, #GICC_HPPIR]
	ldr	w9, [x17, #GICC_AHPPIR]
	ldr	w10, [x17, #GICC_CTLR]
	/* Store to the crash buf and print to console */
	bl	str_in_crash_buf_print

	/* Print the GICD_ISPENDR regs */
	add	x7, x16, #GICD_ISPENDR
	adr	x4, gicd_pend_reg
	bl	asm_print_str
gicd_ispendr_loop:
	sub	x4, x7, x16
	cmp	x4, #0x280
	b.eq	exit_print_gic_regs
	bl	asm_print_hex

	adr	x4, spacer
	bl	asm_print_str

	ldr	x4, [x7], #8
	bl	asm_print_hex

	adr	x4, newline
	bl	asm_print_str
	b	gicd_ispendr_loop
exit_print_gic_regs:
#endif
	.endm

.section .rodata.cci_reg_name, "aS"
cci_iface_regs:
	.asciz "cci_snoop_ctrl_cluster0", "cci_snoop_ctrl_cluster1" , ""

	/* ------------------------------------------------
	 * The below macro prints out relevant interconnect
	 * registers whenever an unhandled exception is
	 * taken in BL3-1.
	 * Clobbers: x0 - x9, sp
	 * ------------------------------------------------
	 */
	.macro plat_print_interconnect_regs
//	adr	x6, cci_iface_regs
	/* Store in x7 the base address of the first interface */
/*	mov_imm	x7, (PLAT_MT_CCI_BASE + SLAVE_IFACE_OFFSET(	\
			PLAT_MT_CCI_CLUSTER0_SL_IFACE_IX))
	ldr	w8, [x7, #SNOOP_CTRL_REG]
*/	/* Store in x7 the base address of the second interface */
/*	mov_imm	x7, (PLAT_MT_CCI_BASE + SLAVE_IFACE_OFFSET(	\
			PLAT_MT_CCI_CLUSTER1_SL_IFACE_IX))
	ldr	w9, [x7, #SNOOP_CTRL_REG]
*/	/* Store to the crash buf and print to console */
/*	bl	str_in_crash_buf_print */
	.endm
