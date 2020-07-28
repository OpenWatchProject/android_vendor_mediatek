/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
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
#include <bakery_lock.h>
#include <console.h>
#include <debug.h>
#include <mmio.h>
#include <mt_spm_reg.h>
#include <mtk_plat_common.h>
#include <plat_dcm.h>
#include <plat_private.h>
#include <platform.h>
#include <platform_def.h>

/* print more log for debug */
/* #define DCM_DEBUG */
/* skip to restore DCM for debug */
/* #define DCM_SKIP_RESTORE */

extern struct atf_arg_t gteearg;

static unsigned int dcm_init_type;

void dcm_restore_cluster_on(unsigned long mpidr)
{
}

void dcm_restore_mcusys_on(void)
{
}

void dcm_msg_handler(uint64_t init_type)
{
	dcm_init_type = init_type & ALL_DCM_TYPE;
#ifdef DCM_DEBUG
	console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE);
	ERROR("%s: dcm_init_type=0x%x\n", __func__, dcm_init_type);
	console_uninit();
#endif
}

unsigned long dcm_get_enabled_cnt(uint64_t type)
{
	return 0;
}

void dcm_init(void)
{
#ifdef DCM_DEBUG
	console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE);
	ERROR("%s: dcm_init_type=0x%x\n", __func__, dcm_init_type);
	console_uninit();
#endif
}

