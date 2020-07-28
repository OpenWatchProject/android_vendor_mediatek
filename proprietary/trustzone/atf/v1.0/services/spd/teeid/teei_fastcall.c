/*
 * Copyright (c) 2015-2018 MICROTRUST Incorporated
 * All rights reserved
 *
 * This file and software is confidential and proprietary to MICROTRUST Inc.
 * Unauthorized copying of this file and software is strictly prohibited.
 * You MUST NOT disclose this file and software unless you get a license
 * agreement from MICROTRUST Incorporated.
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <arch_helpers.h>
#include <console.h>
#include <platform.h>
#include <context_mgmt.h>
#include <runtime_svc.h>
#include <bl31.h>
#include <fiq_smp_call.h>
#include <bl_common.h>
#include <gic_v2.h>
#include <gic_v3.h>
#include <interrupt_mgmt.h>
#include "plat_private.h"
#include "plat_teei.h"
#include "teei_private.h"
#include "teei_atf.h"
#include "platform_def_teei.h"
#include "log.h"

#define MICROTRUST_LOG 0
#define SMC_STRING_LOG 0

#define TOUCH_EINT_NUM	8
#define TOUCH_INTR_NUM	206
#define I2C4_DAPC_ID	98

#define RAND_SEED_SIZE 4
#define SMC_CALL_INTERRUPTED_IRQ 0x54
#define SMC_CALL_DONE 0x45

/*
 * Notes:
 *   * 32 bit SMC take and return only 32 bit results; remaining bits are undef.
 *     * Never use remaining values for anything.
 *   * PM callbacks are dummy.
 *     * Implement resume and migrate callbacks.
 *   * We update secure system registers at every return. We could optimize this.
 * To be consireded:
 *   * Initialization checks: Check non-null context
 *   * On-demand intialization
 *   * State checking: Chech teei does not return incorrect way
 *     (fastcall vs normal SMC / interrupt)
 *   * Disable FIQs, if occurring when teei is not ok to handle them.
 */

/* MSM areas */
struct msm_area_t msm_area;

uint64_t pre_mpidr;
uint32_t uart_apc_num;
uint32_t TEEI_STATE;
uint32_t spsr_el3_t;
uint32_t scr_el3_t;
uint64_t elr_el3_t;

static uint32_t non_irq_num;
static uint64_t secure_os_state;
/* Context for each core. gp registers not used by SPD. */
struct teei_context *secure_context = msm_area.secure_context;

#define SYSTEM_MODE (6)
#define VENDOR_MODE (10)

static void teei_migrate_context_core(uint64_t from_mpidr, uint64_t to_mpidr)
{
	uint32_t from_linear_id = platform_get_core_pos(from_mpidr);
	uint32_t to_linear_id = platform_get_core_pos(to_mpidr);
	struct teei_context *teei_ctx = &secure_context[to_linear_id];

	memcpy(teei_ctx, &secure_context[from_linear_id], sizeof(secure_context[from_linear_id]));
	cm_set_context_by_mpidr(to_mpidr, &teei_ctx->cpu_ctx, SECURE);
}

static void teei_prepare_switch_entry(uint32_t secure_state)
{

	if (secure_state == SECURE) {
		/*add your device context here ,
		 * maybe include save nt device state  and restore
		 * t devcie state  --steven
		 */
		/*set_module_apc(uart_apc_num, 0, 1);*/
		/*gic context*/
		migrate_gic_context(SECURE);
		cpu_context_t *s_context = (cpu_context_t *) CM_GET_CONTEXT(SECURE);
		/* Entry to teei */
		el3_state_t *el3sysregs = get_el3state_ctx(s_context);
		/*write_ctx_reg(el3sysregs, CTX_SPSR_EL3, 0x1d1);*/
		/*write_ctx_reg(el3sysregs, CTX_SCR_EL3, SCR_EL3_S);*/

		uint64_t scr_el3 = read_ctx_reg(el3sysregs, CTX_SCR_EL3) & ~SCR_FIQ_BIT;

		write_ctx_reg(el3sysregs, CTX_SCR_EL3, scr_el3);
	} else {
		/*add your device context here ,
		 * maybe include save t device state  and restore
		 * nt devcie state  --steven
		 */
		/*set_module_apc(uart_apc_num, 0, 0); */

		/*gic context*/
		migrate_gic_context(NON_SECURE);
		cpu_context_t *ns_context = (cpu_context_t *) CM_GET_CONTEXT(NON_SECURE);
		el3_state_t *el3sysregs = get_el3state_ctx(ns_context);
		/* write_ctx_reg(el3sysregs, CTX_SPSR_EL3, nwdEntrySpsr);*/
		/*write_ctx_reg(el3sysregs, CTX_SCR_EL3, SCR_EL3_NS_FIQ);*/

		uint64_t scr_el3 = read_ctx_reg(el3sysregs, CTX_SCR_EL3) | SCR_FIQ_BIT;

		write_ctx_reg(el3sysregs, CTX_SCR_EL3, scr_el3);
	}
}
static void teei_passing_param(uint64_t x1, uint64_t x2, uint64_t x3)
{
	cpu_context_t *s_context = (cpu_context_t *) CM_GET_CONTEXT(SECURE);
	gp_regs_t *s_gpregs = get_gpregs_ctx(s_context);

	write_ctx_reg(s_gpregs, CTX_GPREG_X1, x1);
	write_ctx_reg(s_gpregs, CTX_GPREG_X2, x2);
	write_ctx_reg(s_gpregs, CTX_GPREG_X3, x3);

}

static void ns_teei_passing_param(uint64_t x1, uint64_t x2, uint64_t x3)
{
	cpu_context_t *ns_context = (cpu_context_t *) CM_GET_CONTEXT(NON_SECURE);
	gp_regs_t *ns_gpregs = get_gpregs_ctx(ns_context);

	write_ctx_reg(ns_gpregs, CTX_GPREG_X1, x1);
	write_ctx_reg(ns_gpregs, CTX_GPREG_X2, x2);
	write_ctx_reg(ns_gpregs, CTX_GPREG_X3, x3);
}

static void teei_ns_passing_param(uint64_t x1)
{
	cpu_context_t *s_context = (cpu_context_t *) CM_GET_CONTEXT(NON_SECURE);
	gp_regs_t *s_gpregs = get_gpregs_ctx(s_context);

	write_ctx_reg(s_gpregs, CTX_GPREG_X1, x1);
}

void teei_register_fiq_handler(void)
{
	uint32_t flags = 0;

	set_interrupt_rm_flag(flags, NON_SECURE);

	uint32_t rc = register_interrupt_type_handler(INTR_TYPE_S_EL1,
										teei_fiq_handler,
										flags);

	if (rc != 0)
		DBG_PRINTF("======  FIQ register failed. =======\n\r");
}

static void teei_trigger_soft_intr(uint32_t id)
{
	trigger_soft_intr(id);
}


/************************************************************************************
 ********************************fastcall handler************************************
 ************************************************************************************/

static uint64_t teei_fastcall_handler(uint32_t smc_fid,
							uint64_t x1,
							uint64_t x2,
							uint64_t x3,
							uint64_t x4,
							void *cookie,
							void *handle,
							uint64_t flags)
{
	uint64_t mpidr = read_mpidr();
	uint32_t linear_id = platform_get_core_pos(mpidr);
	struct teei_context *teei_ctx = &secure_context[linear_id];
	int caller_security_state = flags & 1;
	static uint32_t count;
	int state = 0;
	uint64_t smc_ret;

#ifdef TUI_SUPPORT
	int ret = 0;
#endif

	if (caller_security_state == SECURE) {
#if MICROTRUST_LOG
		UART_INIT;
#if SMC_STRING_LOG
		DBG_PRINTF("ft, t to nt id[%s] cid[%ld]\n\r",
				teei_smc_call_id_to_string(maskSWdRegister(smc_fid)),
				linear_id);
#else
		DBG_PRINTF("ft, t to nt id[%x] cid[%ld]\n\r",
				maskSWdRegister(smc_fid),
				linear_id);
#endif
		UART_UNINIT;
#endif

		assert(handle == CM_GET_CONTEXT(SECURE));

			switch (maskSWdRegister(smc_fid)) {
			case T_BOOT_NT_OS: {
				/* Return values from fastcall already in cpu_context!*/
				TEEI_STATE = TEEI_BOOT;
				teei_register_fiq_handler();
				prepare_gic_for_nsec_boot();
				/* it will return to teei_synchronous_sp_entry call*/
				teei_synchronous_sp_exit(teei_ctx, 0, 1);
				break;
			}
			case T_ACK_N_OS_READY: {
				ns_teei_passing_param(x1, x2, x3);
				non_irq_num = NSEC_BOOT_INTR;
				teei_trigger_soft_intr(NSEC_CALL_INTR);
				TEEI_STATE = TEEI_SERVICE_READY;
				teei_synchronous_sp_exit(teei_ctx, SMC_CALL_DONE, 1);
				break;
			}
			case T_ACK_N_FAST_CALL: {
				ns_teei_passing_param(x1, x2, x3);
				non_irq_num = NSEC_BOOT_INTR;
				teei_trigger_soft_intr(NSEC_CALL_INTR);
				teei_synchronous_sp_exit(teei_ctx, SMC_CALL_DONE, 1);
				break;
			}
			case T_ACK_N_INIT_FC_BUF: {
				if (count == 1) {
					TEEI_STATE = TEEI_BUF_READY;
					enable_group(1);
				}
				count = 1;
				ns_teei_passing_param(x1, x2, x3);
				teei_synchronous_sp_exit(teei_ctx, SMC_CALL_DONE, 1);
				break;
			}
			case T_GET_BOOT_PARAMS: {
				x1 =  TEEI_BOOT_PARAMS;
				x2 =  TEEI_SECURE_PARAMS;
				x3 =  WDT_IRQ_BIT_ID;
				teei_passing_param(x1, x2, x3);
				SMC_RET1(handle, T_GET_BOOT_PARAMS);
				break;
			}
			case T_BOOT_TA_MODE: {
				x1 =  VENDOR_MODE;
				teei_passing_param(x1, x2, x3);
				SMC_RET1(handle, T_BOOT_TA_MODE);
				break;
			}
#ifdef TUI_SUPPORT
			case T_SET_TUI_EINT: {
				DBG_PRINTF("mt_eint_set_secure_deint start!\n\r");
				mt_eint_set_secure_deint(TOUCH_EINT_NUM, TOUCH_INTR_NUM);
				gicd_set_icpendr(GICD_BASE, TOUCH_INTR_NUM);
				SET_MODULE_APC(I2C4_DAPC_ID, 0, 1);
				SMC_RET1(handle, T_SET_TUI_EINT);
				break;
			}
			case T_CLR_TUI_EINT: {
				SET_MODULE_APC(I2C4_DAPC_ID, 0, 0);
				ret = mt_eint_clr_deint(TOUCH_INTR_NUM);
				if (ret)
					DBG_PRINTF("mt_eint_clr_deint fail!\n\r");
				gicd_set_icpendr(GICD_BASE, TOUCH_INTR_NUM);
				DBG_PRINTF("mt_eint_set_secure_deint end !\n\r");
				SMC_RET1(handle, T_CLR_TUI_EINT);
				break;
			}
#endif
			case T_SET_DRM_INT: {
#if MICROTRUST_LOG
				tf_printf("T_SET_DRM_INT: set fiq '%d'\n", (uint32_t)x1);
#endif
				state = set_drm_int_secure(x1);
				teei_passing_param(state, x2, x3);
				SMC_RET1(handle, T_SET_DRM_INT);
				break;
			}
			case T_CLR_DRM_INT: {
#if MICROTRUST_LOG
				tf_printf("T_CLR_DRM_INT: clear fiq '%d'\n", (uint32_t)x1);
#endif
				state = set_drm_int_nonsecure(x1);
				teei_passing_param(state, x2, x3);
				SMC_RET1(handle, T_CLR_DRM_INT);
				break;
			}
			case T_SW_GENERATE_INT: {
#if MICROTRUST_LOG
				tf_printf("T_SW_GENERATE_INT: trigger interrupt '%d'\n", (uint32_t)x1);
#endif
				teei_trigger_soft_intr((uint32_t)x1);
				teei_passing_param(0, x2, x3);
				SMC_RET1(handle, T_SW_GENERATE_INT);
				break;
			}
			case T_GET_RAND_SEED: {
				x1 = teei_rng();
#if MICROTRUST_LOG
				tf_printf("T_GET_RAND_SEED rand [%llx]\n\r", x1);
#endif
				teei_passing_param(x1, RAND_SEED_SIZE, 0);
				SMC_RET1(handle, T_GET_RAND_SEED);
				break;
			}

			case T_VUART_LOG_CALL: {
				mt_log_secure_os_print(0xff & x1);
				SMC_RET1(handle, T_GET_BOOT_PARAMS);
				break;
			}
			default:  {
				DBG_PRINTF("teei_fastcall_handler T SMC_UNK %x\n\r", smc_fid);
				SMC_RET1(handle, SMC_UNK);
				break;
			}
		}
	} else {
		/*set_module_apc(uart_apc_num, 0, 1); */
#if MICROTRUST_LOG
		UART_INIT;
#if SMC_STRING_LOG
		DBG_PRINTF("ft,nt to t,id[%s]cid[%ld]\n\r", teei_smc_call_id_to_string(smc_fid), linear_id);
#else
		DBG_PRINTF("ft,nt to t,id[%x]cid[%ld]\n\r", smc_fid, linear_id);
#endif
		UART_UNINIT;
#endif

		assert(handle == CM_GET_CONTEXT(NON_SECURE));

			switch (smc_fid) {
			case N_INIT_T_BOOT_STAGE1:
			case N_SWITCH_TO_T_OS_STAGE2:
			case N_INIT_T_FC_BUF:
			case N_INVOKE_T_FAST_CALL: {
				cm_el1_sysregs_context_save(NON_SECURE);

				if (smc_fid == N_INIT_T_FC_BUF || smc_fid == N_INIT_T_BOOT_STAGE1)
					teei_passing_param(x1, x2, x3);
				teei_prepare_switch_entry(SECURE);
				if (smc_fid == N_INVOKE_T_FAST_CALL)
					teei_trigger_soft_intr(SEC_APP_INTR);
				if (TEEI_STATE == TEEI_BOOT)
					prepare_gic_for_sec_boot();
				smc_ret = teei_synchronous_sp_entry(teei_ctx);

				cm_el1_sysregs_context_restore(NON_SECURE);
				cm_set_next_eret_context(NON_SECURE);
				teei_prepare_switch_entry(NON_SECURE);
				SMC_RET1(handle, smc_ret);
				break;
			}
			case N_SWITCH_CORE: {
#if MICROTRUST_LOG
				UART_INIT;
				DBG_PRINTF("SC       [%ld] to [%ld]\n\r",
							platform_get_core_pos(x2),
							platform_get_core_pos(x1));
				UART_UNINIT;
#endif

				teei_migrate_context_core(x2, x1);
				SMC_RET1(handle, N_SWITCH_CORE);
				break;
			}

			case N_GET_SE_OS_STATE: {
#if MICROTRUST_LOG
				UART_INIT;
				DBG_PRINTF("N_GET_SE_OS_STATE state [0x%x]\n\r",
						secure_os_state);
				UART_UNINIT;
#endif

				ns_teei_passing_param(x1, x2, x3);
				SMC_RET1(handle, secure_os_state);
				break;
			}

			case N_GET_NON_IRQ_NUM: {
				ns_teei_passing_param(x1, x2, x3);
				SMC_RET1(handle, non_irq_num);
				break;
			}


			case N_GET_T_FP_DEVICE_ID:
			case N_GET_T_FP_DEVICE_ID_32:
			{
				unsigned int id = teei_get_fp_id();

				UART_INIT;
				DBG_PRINTF("SC N_GET_T_FP_DEVICE_ID [%d] [%d]\n\r", platform_get_core_pos(x2),
				platform_get_core_pos(x1));
				UART_UNINIT;
				/*teearg->tee_dev[4].dev_type = FP_TYPE*/
				teei_ns_passing_param(id);

				SMC_RET1(handle, N_GET_T_FP_DEVICE_ID);
				break;
			}
			default:
			{
				DBG_PRINTF("teei_fastcall_handler NT SMC_UNK %x\n\r", smc_fid);
				SMC_RET1(handle, SMC_UNK);
				break;
			}
		}
	}
	return 0;
}

/************************************************************************************
 * Standard call handler
 ************************************************************************************/


static uint64_t teei_standard_call_handler(uint32_t smc_fid,
								uint64_t x1,
								uint64_t x2,
								uint64_t x3,
								uint64_t x4,
								void *cookie,
								void *handle,
								uint64_t flags)
{

	uint64_t mpidr = read_mpidr();
	uint32_t linear_id = platform_get_core_pos(mpidr);
	struct teei_context *teei_ctx = &secure_context[linear_id];
	int caller_security_state = flags&1;
	uint64_t smc_ret;
	/*unsigned int gicd_base;*/

	if (caller_security_state == SECURE) {
#if MICROTRUST_LOG
		UART_INIT;
#if SMC_STRING_LOG
		DBG_PRINTF("st, t to nt id[%s]cid[%ld]\n\r",
				teei_smc_call_id_to_string(maskSWdRegister(smc_fid)),
				linear_id);
#else
		DBG_PRINTF("st, t to nt id[%x]cid[%ld]\n\r",
				maskSWdRegister(smc_fid),
				linear_id);
#endif
		UART_UNINIT;

#endif
		assert(handle == CM_GET_CONTEXT(SECURE));
			switch (maskSWdRegister(smc_fid)) {
			case    T_SCHED_NT: {
						    non_irq_num =  NSEC_SCHED_INTR;
						    ns_teei_passing_param(x1, x2, x3);
						    teei_trigger_soft_intr(NSEC_CALL_INTR);
						    teei_synchronous_sp_exit(teei_ctx,
									     SMC_CALL_DONE, 1);
			break;
			}
			case    T_SCHED_NT_IRQ: {
			    ns_teei_passing_param(x1, x2, x3);
			    teei_synchronous_sp_exit(teei_ctx, SMC_CALL_INTERRUPTED_IRQ, 1);
			break;
			}
			case	T_SCHED_NT_LOG: {
							non_irq_num =  NSEC_LOG_INTR;
							ns_teei_passing_param(x1, x2, x3);
							teei_trigger_soft_intr(NSEC_INTSCHED_INTR);
							teei_trigger_soft_intr(NSEC_CALL_INTR);
							teei_synchronous_sp_exit(teei_ctx,
										 SMC_CALL_DONE, 1);
			break;
			}
			case T_NOTIFY_N_ERR:	{
				    ns_teei_passing_param(x1, x2, x3);
				    non_irq_num = NSEC_ERR_INTR;
				    secure_os_state = x1;
				    teei_trigger_soft_intr(NSEC_CALL_INTR);
				    DBG_PRINTF("=======  Secure world error 0x%lx =======\n\r", secure_os_state);
				    teei_synchronous_sp_exit(teei_ctx, SMC_CALL_DONE, 1);
			break;
			}
			case    T_INVOKE_N_LOAD_IMG:
			case    T_ACK_N_KERNEL_OK:
			case    T_ACK_N_NQ:
			case    T_ACK_N_INVOKE_DRV:
			case    T_INVOKE_N_DRV:
			case    T_ACK_N_BOOT_OK: {
				ns_teei_passing_param(x1, x2, x3);
			if ((maskSWdRegister(smc_fid) != T_ACK_N_BOOT_OK) &&
				(maskSWdRegister(smc_fid) != T_ACK_N_KERNEL_OK) &&
				(maskSWdRegister(smc_fid) != T_ACK_N_INVOKE_DRV) &&
				(maskSWdRegister(smc_fid) != T_INVOKE_N_DRV)) {
				non_irq_num = NSEC_APP_INTR;
				teei_trigger_soft_intr(NSEC_CALL_INTR);
				} else if (maskSWdRegister(smc_fid) == T_ACK_N_INVOKE_DRV) {
					non_irq_num =  NSEC_DRV_INTR;
					teei_trigger_soft_intr(NSEC_CALL_INTR);
				} else if (maskSWdRegister(smc_fid) == T_INVOKE_N_DRV) {
					non_irq_num =  NSEC_RDRV_INTR;
					teei_trigger_soft_intr(NSEC_CALL_INTR);
				} else {
					non_irq_num =  NSEC_BOOT_INTR;
					teei_trigger_soft_intr(NSEC_CALL_INTR);
				}

				if (maskSWdRegister(smc_fid) == T_ACK_N_KERNEL_OK)
					TEEI_STATE = TEEI_KERNEL_READY;

				if (maskSWdRegister(smc_fid) == T_ACK_N_BOOT_OK)
					TEEI_STATE = TEEI_ALL_READY;

				teei_synchronous_sp_exit(teei_ctx, SMC_CALL_DONE, 1);
				break;
			}
			default:  {
						DBG_PRINTF("teei_fastcall_handler T SMC_UNK %x\n\r", smc_fid);
						SMC_RET1(handle, SMC_UNK);
						break;
				}
			}
	} else {
		/*set_module_apc(uart_apc_num, 0, 1); */
#if MICROTRUST_LOG
		UART_INIT;
#if SMC_STRING_LOG
		DBG_PRINTF("st,nt to t, id [%s]cid[%ld]\n\r", teei_smc_call_id_to_string(smc_fid), linear_id);
#else
		DBG_PRINTF("st,nt to t, id [%x]cid[%ld]\n\r", smc_fid, linear_id);
#endif
		UART_UNINIT;

#endif
		assert(handle == CM_GET_CONTEXT(NON_SECURE));
			switch (smc_fid) {
			case  N_ACK_T_LOAD_IMG:
			case  NT_SCHED_T:
			case  N_INVOKE_T_NQ:
			case  N_INVOKE_T_DRV:
			case  N_ACK_T_INVOKE_DRV:
			case  N_INVOKE_T_LOAD_TEE: {
				cm_el1_sysregs_context_save(NON_SECURE);
				teei_prepare_switch_entry(SECURE);
				if ((smc_fid != NT_SCHED_T) && (smc_fid != N_ACK_T_LOAD_IMG) &&
					(smc_fid != N_INVOKE_T_DRV) && (smc_fid != N_ACK_T_INVOKE_DRV))
					teei_trigger_soft_intr(SEC_APP_INTR);

				if (TEEI_STATE == TEEI_BOOT)
					prepare_gic_for_sec_boot();
				if (smc_fid == N_INVOKE_T_DRV)
					teei_trigger_soft_intr(SEC_DRV_INTR);
				if (smc_fid == N_ACK_T_INVOKE_DRV)
					teei_trigger_soft_intr(SEC_RDRV_INTR);
				if (smc_fid == NT_SCHED_T) {
					if (x1 == 0x9527) {
#if MICROTRUST_LOG
						UART_INIT;
						DBG_PRINTF("====== SEC_TDUMP_INTR's Working in ATF .=====\n\r");
						UART_UNINIT;
#endif
						teei_trigger_soft_intr(SEC_TDUMP_INTR);
					}
				}
				if (smc_fid == NT_CANCEL_T_TUI)
					teei_trigger_soft_intr(SEC_TUI_CANCEL);
				smc_ret = teei_synchronous_sp_entry(teei_ctx);
				cm_el1_sysregs_context_restore(NON_SECURE);
				cm_set_next_eret_context(NON_SECURE);
				teei_prepare_switch_entry(NON_SECURE);
				SMC_RET1(handle, smc_ret);
				break;
			}
			default: {
				DBG_PRINTF("teei_fastcall_handler NT SMC_UNK %x\n\r", smc_fid);
				SMC_RET1(handle, SMC_UNK);
				break;
			}
		}
	}
	return 0;
}

/* Register teei fastcalls service */
DECLARE_RT_SVC(
		teei_fastcall,
		OEN_TOS_START,
		OEN_TOS_END,
		SMC_TYPE_FAST,
		teei_fastcall_setup,
		teei_fastcall_handler
		);

/* Register teei SMC service */
/* Note: OEN_XXX constants do not apply to normal SMCs (only to fastcalls).*/
DECLARE_RT_SVC(
		teei_standard_call,
		OEN_TOS_START,
		OEN_TOS_END,
		SMC_TYPE_STD,
		NULL,
		teei_standard_call_handler
		);

#if TEEI_OEM_ROUTE_ENABLE
/* Register teei OEM SMC handler service */
DECLARE_RT_SVC(
		teei_oem_fastcall,
		OEN_OEM_START,
		OEN_OEM_END,
		SMC_TYPE_FAST,
		NULL,
		teei_fastcall_handler
		);
#endif

#if TEEI_SIP_ROUTE_ENABLE
/* Register teei SIP SMC handler service */
DECLARE_RT_SVC(
		teei_sip_fastcall,
		OEN_SIP_START,
		OEN_SIP_END,
		SMC_TYPE_FAST,
		NULL,
		teei_fastcall_handler
		);
#endif

#if TEEI_DUMMY_SIP_ROUTE_ENABLE
/* Register teei SIP SMC handler service, unfortunately because of a typo in our
 * older versions we must serve the 0x81000000 fastcall range for backward compat
 */
DECLARE_RT_SVC(
		teei_dummy_sip_fastcall,
		OEN_CPU_START,
		OEN_CPU_END,
		SMC_TYPE_FAST,
		teei_dummy_setup,
		teei_fastcall_handler
		);
#endif
