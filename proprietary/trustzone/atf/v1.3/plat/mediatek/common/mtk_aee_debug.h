/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
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
#ifndef __MTK_AEE_DEBUG_H__
#define __MTK_AEE_DEBUG_H__
#include "fiq_smp_call.h"

extern void aee_wdt_dump_all_core(void *cookie);
extern void aee_wdt_dump(void *cookie);
extern void aee_flush_log_cache(void);
extern void wdt_notify_other_cores(inter_cpu_call_func_t func, void *info, int wait);
void check_wdt_sgi_routing(void);
uint32_t ram_console_set_exp_type_atf(uint32_t exp_type);

/*******************************************************************************
 * MTK struct
 ******************************************************************************/

enum AEE_EXP_TYPE_NUM {
	AEE_EXP_TYPE_HWT = 1,
	AEE_EXP_TYPE_KE = 2,
	AEE_EXP_TYPE_NESTED_PANIC = 3,
	AEE_EXP_TYPE_SMART_RESET = 4, /* mrdump key */
	AEE_EXP_TYPE_HANG_DETECT = 5,
	AEE_EXP_TYPE_LK_CRASH = 6,
};

#endif /* __MTK_AEE_DEBUG_H__ */
