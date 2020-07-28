/*
 * Copyright (c) 2016 TRUSTONIC LIMITED
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the TRUSTONIC LIMITED nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
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

#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <bl31.h>
#include <context_mgmt.h>
#include <errno.h>
#include <platform.h>
#include <runtime_svc.h>
#include <stdio.h>
#include <string.h>
#include <tbase_private.h>
#include <tbase_platform.h>

/*******************************************************************************
 * This function takes an SP context pointer and:
 * 1. Applies the S-EL1 system register context from tsp_ctx->cpu_ctx.
 * 2. Saves the current C runtime state (callee saved registers) on the stack
 *    frame and saves a reference to this state.
 * 3. Calls el3_exit() so that the EL3 system and general purpose registers
 *    from the tsp_ctx->cpu_ctx are used to enter the secure payload image.
 ******************************************************************************/
uint64_t tbase_synchronous_sp_entry(struct tbase_context *tbase_ctx)
{
	uint64_t rc;

	assert(tbase_ctx->c_rt_ctx == 0);

	/* Apply the Secure EL1 system register context and switch to it */
	assert(cm_get_context_by_mpidr(read_mpidr(), SECURE) == &tbase_ctx->cpu_ctx);

	cm_el1_sysregs_context_restore(SECURE);
	cm_set_next_eret_context(SECURE);

	rc = tbase_enter_sp(&tbase_ctx->c_rt_ctx);
	#if DEBUG
	tbase_ctx->c_rt_ctx = 0;
	#endif

	return rc;
}


/*******************************************************************************
 * This function takes an SP context pointer and:
 * 1. Saves the S-EL1 system register context tp tsp_ctx->cpu_ctx.
 * 2. Restores the current C runtime state (callee saved registers) from the
 *    stack frame using the reference to this state saved in tspd_enter_sp().
 * 3. It does not need to save any general purpose or EL3 system register state
 *    as the generic smc entry routine should have saved those.
 ******************************************************************************/
void tbase_synchronous_sp_exit(struct tbase_context *tbase_ctx, uint64_t ret, uint32_t save_sysregs)
{
	/* Save the Secure EL1 system register context */
	assert(cm_get_context_by_mpidr(read_mpidr(), SECURE) == &tbase_ctx->cpu_ctx);

	if (save_sysregs)
		cm_el1_sysregs_context_save(SECURE);

	assert(tbase_ctx->c_rt_ctx != 0);
	tbase_exit_sp(tbase_ctx->c_rt_ctx, ret);

	/* Should never reach here */
	assert(0);
}

