/*
 * Copyright (c) 2013-2015, ARM Limited and Contributors. All rights reserved.
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
#include <asm_macros.S>
#include <platform_def.h>

	.globl	plat_secondary_cold_boot_setup
	.globl	plat_report_exception
	.globl	platform_is_primary_cpu
	.globl	plat_crash_console_init
	.globl	plat_crash_console_putc
	.globl	platform_mem_init
	.globl	read_cpuectlr
	.globl	write_cpuectlr
	.globl	read_cpuactlr
	.globl	write_cpuactlr


	/* -----------------------------------------------------
	 * void plat_secondary_cold_boot_setup (void);
	 *
	 * This function performs any platform specific actions
	 * needed for a secondary cpu after a cold reset e.g
	 * mark the cpu's presence, mechanism to place it in a
	 * holding pen etc.
	 * -----------------------------------------------------
	 */
func plat_secondary_cold_boot_setup
	/* Platform does not do cold boot for secondary CPU */
cb_panic:
	b	cb_panic
endfunc plat_secondary_cold_boot_setup

func read_cpuectlr
	MRS	x0, S3_1_C15_C2_1
	ret
endfunc	read_cpuectlr

func write_cpuectlr
	MSR	S3_1_C15_C2_1, x0
	ret
endfunc	write_cpuectlr

func read_cpuactlr
        dsb sy
        MRS x0, S3_1_C15_C2_0     /* @Read CPUACTLR_EL1 into Rt */
        dsb sy
        isb
	ret
endfunc read_cpuactlr

func write_cpuactlr
        dsb sy
        MSR S3_1_C15_C2_0, x0     /* @write CPUACTLR_EL1 into Rt */
        isb
        dsb sy
	ret
endfunc write_cpuactlr

func platform_is_primary_cpu
	and	x0, x0, #(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK)
	cmp	x0, #PLAT_PRIMARY_CPU
	cset	x0, eq
	ret
endfunc platform_is_primary_cpu

	/* ---------------------------------------------
	 * int plat_crash_console_init(void)
	 * Function to initialize the crash console
	 * without a C Runtime to print crash report.
	 * Clobber list : x0, x1, x2
	 * ---------------------------------------------
	 */
func plat_crash_console_init
	mov	x0, #0x1
	ret
endfunc plat_crash_console_init

	/* ---------------------------------------------
	 * int plat_crash_console_putc(void)
	 * Function to print a character on the crash
	 * console without a C Runtime.
	 * Clobber list : x1, x2
	 * ---------------------------------------------
	 */
func plat_crash_console_putc
	/*
	 * Check teearg->atf_log_buf_size.
	 * Exit if atf_log_buf_size equals 0
	 */
	adr	x2, ptr_atf_crash_flag
	ldr	x2, [x2]
	mov_imm x1, 0xffffffff
	cmp	w2, w1
	b.ne	1f

	/* Sent to UART if ptr_atf_crash_flag equals 0xffffffff */
	mov_imm x1, UART0_BASE
	b console_core_putc
1:
	/*
	 * get cpu id
	 */
	mrs	x1, mpidr_el1
	and	x2, x1, #MPIDR_CPU_MASK			/* refer to platform_get_core_pos */
	and	x1, x1, #MPIDR_CLUSTER_MASK
	add	x1, x2, x1, LSR #6					/* x1 = cpu id (cpu id = aff0 + aff1*4 ) */

	adr	x2, ptr_except_write_pos_per_cpu
	ldr	x2, [x2]
	add x2, x2, x1, LSL # 3					/* plus (cpu_id * 8) --> &p_atf_log_ctrl->atf_except_write_pos_per_cpu[cpu_id]
										  						 * x2 = &p_atf_log_ctrl->atf_except_write_pos_per_cpu[cpu_id];
																	 */
	/* log write */
	ldr	x1, [x2]										/* w1 = p_atf_log_ctrl->atf_except_write_pos_per_cpu[cpu_id] */
	/*
	  * Check if x1( p_atf_log_ctrl->atf_except_write_pos_per_cpu[cpu_id])
	  * is greater than atf_crash_buf_upper_bound_addr
	  */
	ldr	x2, =atf_crash_buf_upper_bound_addr
	ldr	x2, [x2]
	cmp	x1, x2
	bhi	done
	/* x1 = w0 --> *(p_atf_log_ctrl->atf_except_write_pos_per_cpu[cpu_id]) = c) */
	strb	w0, [x1]
	/* w1++ */
	add	x1, x1, #1
	mov x0, x1
	/* Get cpu linear id in x1 */
	mrs	x1, mpidr_el1
	and	x2, x1, #MPIDR_CPU_MASK
	and	x1, x1, #MPIDR_CLUSTER_MASK
	add	x1, x2, x1, LSR #6

	/* Save updated write ptr to p_atf_log_ctrl->atf_except_write_pos_per_cpu[cpu_id] */
	adr	x2, ptr_except_write_pos_per_cpu
	ldr	x2, [x2]
	/*
	 * plus (cpu_id * 8) --> &p_atf_log_ctrl->atf_except_write_pos_per_cpu[cpu_id]
	 * x2 = &p_atf_log_ctrl->atf_except_write_pos_per_cpu[cpu_id];
	 */
	add x2, x2, x1, LSL # 3
	str	x0, [x2]
done:
	ret
endfunc plat_crash_console_putc

	/* --------------------------------------------------------
	 * void platform_mem_init (void);
	 *
	 * Any memory init, relocation to be done before the
	 * platform boots. Called very early in the boot process.
	 * --------------------------------------------------------
	 */
func platform_mem_init
	mov	x0, #0
	ret
endfunc platform_mem_init

