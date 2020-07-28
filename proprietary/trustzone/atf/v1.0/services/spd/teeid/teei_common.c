/*
 * Copyright (c) 2015-2018 MICROTRUST Incorporated
 * All rights reserved
 *
 * This file and software is confidential and proprietary to MICROTRUST Inc.
 * Unauthorized copying of this file and software is strictly prohibited.
 * You MUST NOT disclose this file and software unless you get a license
 * agreement from MICROTRUST Incorporated.
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
#include "teei_private.h"
#include "teei_atf.h"

#define MICROTRUST_LOG 0

/*******************************************************************************
 * This function takes an SP context pointer and:
 * 1. Applies the S-EL1 system register context from tsp_ctx->cpu_ctx.
 * 2. Saves the current C runtime state (callee saved registers) on the stack
 *    frame and saves a reference to this state.
 * 3. Calls el3_exit() so that the EL3 system and general purpose registers
 *    from the tsp_ctx->cpu_ctx are used to enter the secure payload image.
 ******************************************************************************/
uint64_t teei_synchronous_sp_entry(struct teei_context *teei_ctx)
{
	uint64_t rc;

	assert(teei_ctx->c_rt_ctx == 0);

	/*cpu_context_t *s_entry_context = &teei_ctx->cpu_ctx;*/
	/*gp_regs_t *s_entry_gpregs = get_gpregs_ctx(s_entry_context);*/

	/* Apply the Secure EL1 system register context and switch to it */
	assert(CM_GET_CONTEXT(SECURE) == &teei_ctx->cpu_ctx);

	cm_el1_sysregs_context_restore(SECURE);
	cm_set_next_eret_context(SECURE);

	rc = teei_enter_sp(&teei_ctx->c_rt_ctx);

#if DEBUG
	teei_ctx->c_rt_ctx = 0;
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
void teei_synchronous_sp_exit(struct teei_context *teei_ctx, uint64_t ret, uint32_t save_sysregs)
{
	/* Save the Secure EL1 system register context */
	assert(CM_GET_CONTEXT(SECURE) == &teei_ctx->cpu_ctx);
	if (save_sysregs)
		cm_el1_sysregs_context_save(SECURE);

	assert(teei_ctx->c_rt_ctx != 0);
	teei_exit_sp(teei_ctx->c_rt_ctx, ret);

	/* Should never reach here */
	assert(0);
}

/************************************************************************
 * This function translates SMC call id to string for debugging purpose
 ************************************************************************/
const char *teei_smc_call_id_to_string(uint32_t id)
{
	static const char *smc_str = "undefined";
#if MICROTRUST_LOG
		switch (id) {
		default:
			break;
		case T_BOOT_NT_OS:
			return "T_BOOT_NT_OS";
		case T_ACK_N_OS_READY:
			return "T_ACK_N_OS_READY";
		case T_ACK_N_FAST_CALL:
			return "T_ACK_N_FAST_CALL";
		case T_DUMP_STATE:
			return "T_DUMP_STATE";
		case T_ACK_N_INIT_FC_BUF:
			return "T_ACK_N_INIT_FC_BUF";
		case T_GET_BOOT_PARAMS:
			return "T_GET_BOOT_PARAMS";
		case T_WDT_FIQ_DUMP:
			return "T_WDT_FIQ_DUMP";
		case T_VUART_LOG_CALL:
			return "T_VUART_LOG_CALL";
		case T_SET_TUI_EINT:
			return "T_SET_TUI_EINT";
		case T_CLR_TUI_EINT:
			return "T_CLR_TUI_EINT";
		case T_SCHED_NT:
			return "T_SCHED_NT";
		case T_ACK_N_SYS_CTL:
			return "T_ACK_N_SYS_CTL";
		case T_ACK_N_NQ:
			return "T_ACK_N_NQ";
		case T_ACK_N_INVOKE_DRV:
			return "T_ACK_N_INVOKE_DRV";
		case T_INVOKE_N_DRV:
			return "T_INVOKE_N_DRV";
		case T_ACK_N_BOOT_OK:
			return "T_ACK_N_BOOT_OK";
		case T_INVOKE_N_LOAD_IMG:
			return "T_INVOKE_N_LOAD_IMG";
		case T_ACK_N_KERNEL_OK:
			return "T_ACK_N_KERNEL_OK";
		case T_SCHED_NT_IRQ:
			return "T_SCHED_NT_IRQ";
		case T_NOTIFY_N_ERR:
			return "T_NOTIFY_N_ERR";
		case T_SCHED_NT_LOG:
			return "T_SCHED_NT_LOG";
		case N_SWITCH_TO_T_OS_STAGE2:
			return "N_SWITCH_TO_T_OS_STAGE2";
		case N_GET_PARAM_IN:
			return "N_GET_PARAM_IN";
		case N_INIT_T_FC_BUF:
			return "N_INIT_T_FC_BUF";
		case N_INVOKE_T_FAST_CALL:
			return "N_INVOKE_T_FAST_CALL";
		case N_INIT_T_BOOT_STAGE1:
			return "N_INIT_T_BOOT_STAGE1";
		case N_SWITCH_CORE:
			return "N_SWITCH_CORE";
		case N_GET_NON_IRQ_NUM:
			return "N_GET_NON_IRQ_NUM";
		case NT_SCHED_T:
			return "NT_SCHED_T";
		case N_INVOKE_T_SYS_CTL:
			return "N_INVOKE_T_SYS_CTL";
		case N_INVOKE_T_NQ:
			return "N_INVOKE_T_NQ";
		case N_INVOKE_T_DRV:
			return "N_INVOKE_T_DRV";
		case N_ACK_T_INVOKE_DRV:
			return "N_ACK_T_INVOKE_DRV";
		case N_INVOKE_T_LOAD_TEE:
			return "N_INVOKE_T_LOAD_TEE";
		case N_ACK_T_LOAD_IMG:
			return "N_ACK_T_LOAD_IMG";
		case NT_CANCEL_T_TUI:
			return "NT_CANCEL_T_TUI";
		case T_BOOT_TA_MODE:
			return "T_BOOT_TA_MODE";
		}

#endif
	return smc_str;
}
