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

#ifndef __PLAT_PRIVATE_H__
#define __PLAT_PRIVATE_H__
#define MTK_NO_NEED_CODE	0
/*******************************************************************************
 * Function and variable prototypes
 ******************************************************************************/
#define DEVINFO_SIZE 4
#define HRID_SIZE 2
#define LINUX_KERNEL_32 0

typedef struct {
	uint32_t atf_magic;
	uint32_t tee_support;
	uint32_t tee_entry;
	uint64_t tee_boot_arg_addr;
	uint32_t hwuid[4];     /* HW Unique id for t-base used */
	uint32_t atf_hrid_size; /* Check this atf_hrid_size to read from HRID array */
	uint32_t HRID[8];      /* HW random id for t-base used */
	uint32_t atf_log_port;
	uint32_t atf_log_baudrate;
	uint64_t atf_log_buf_start;
	uint32_t atf_log_buf_size;
	uint32_t atf_irq_num;
	uint32_t devinfo[DEVINFO_SIZE];
	uint64_t atf_aee_debug_buf_start;
	uint32_t atf_aee_debug_buf_size;
	uint32_t msg_fde_key[4]; /* size of message auth key is 16bytes(128 bits) */
#if CFG_TEE_SUPPORT
	uint32_t tee_rpmb_size;
#endif
} atf_arg_t, *atf_arg_t_ptr;

void plat_configure_mmu_el3(unsigned long total_base,
				unsigned long total_size,
				unsigned long,
				unsigned long,
				unsigned long,
				unsigned long);

void plat_cci_init(void);
void plat_cci_enable(void);
void plat_cci_disable(void);

/* Declarations for plat_mt_gic.c */
void plat_mt_gic_init(void);

/* Declarations for plat_topology.c */
int mt_setup_topology(void);

void setup_syscnt(void);
void plat_delay_timer_init(void);

uint32_t plat_get_spsr_for_bl32_entry(void);
uint32_t plat_get_spsr_for_bl33_entry(void);

void enable_ns_access_to_cpuectlr(void);

extern atf_arg_t gteearg;
extern uint64_t mtk_lk_stage;

#endif /* __PLAT_PRIVATE_H__ */
