/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arch_helpers.h>
#include <assert.h>
#include <console.h>
#include <context_mgmt.h>
#include <debug.h>
#include <fiq_cache.h>
#include <fiq_smp_call.h>
#include <log.h>
#include <mmio.h>
#include <mt_cpuxgpt.h>
#include <mtk_aee_debug.h>
#include <mtk_latch.h>
#include <mtk_plat_common.h>
#include <mtk_rot.h>
#include <mtk_sip_svc.h>
#include <mtk_ut_helper.h>
#include <mtk_gic_v3_main.h>
#include <cmdq.h>
#include <platform.h>
#include <runtime_svc.h>
#include <uuid.h>
#ifdef MTK_ENABLE_MPU_HAL_SUPPORT
#include <mpu_ctrl/mpu_api.h>
#endif

/* Mediatek SiP Service UUID */
DEFINE_SVC_UUID(mtk_sip_svc_uid,
		0xf7582ba4, 0x4262, 0x4d7d, 0x80, 0xe5,
		0x8f, 0x95, 0x05, 0x00, 0x0f, 0x3d);

#pragma weak mediatek_plat_sip_handler
uint64_t mediatek_plat_sip_handler(uint32_t smc_fid,
				uint64_t x1,
				uint64_t x2,
				uint64_t x3,
				uint64_t x4,
				void *cookie,
				void *handle,
				uint64_t flags)
{
	ERROR("%s: unhandled SMC (0x%x)\n", __func__, smc_fid);
	SMC_RET1(handle, SMC_UNK);
}

#pragma weak lastpc_postinit
void lastpc_postinit(void)
{
}

#pragma weak mt_irq_dump_status
uint64_t mt_irq_dump_status(uint32_t irq)
{
	return 1;
}

#pragma weak bl31_post_platform_setup
void bl31_post_platform_setup(void)
{
}

/* 32 bits == MODE_RW_32, 64 bits == MODE_RW_64 */
uint64_t lk_type;
uint64_t kernel_type;

/* This function handles Mediatek defined SiP Calls */
uint64_t mediatek_sip_handler(uint32_t smc_fid,
			uint64_t x1,
			uint64_t x2,
			uint64_t x3,
			uint64_t x4,
			void *cookie,
			void *handle,
			uint64_t flags)
{
	uint64_t rc = 0;
	uint32_t ns;
	uint32_t cpu;
	uint64_t mpidr;
	uint64_t spsr = 0;
	uint64_t el = 0;
	cpu_context_t *ns_cpu_context;
	struct atf_arg_t *teearg;
	uint64_t atf_crash_log_addr;
	uint64_t atf_crash_log_size;
	uint32_t atf_crash_flag_addr;
	uint32_t *atf_crash_flag_addr_ptr;



	rc = 0;
	mpidr = read_mpidr();
	teearg = &gteearg;
	/* do not use platform_get_core_pos(mpidr) directly */
	cpu = (uint32_t) plat_core_pos_by_mpidr(mpidr);
	if (cpu >= PLATFORM_CORE_COUNT) {
		ERROR("[%s]cpu-%u is out of max number:%u!\n", __func__, cpu, PLATFORM_CORE_COUNT);
		SMC_RET1(handle, SMC_UNK);
	}
	/* if parameter is sent from SMC32. Clean top 32 bits */
	clean_top_32b_of_param(smc_fid, &x1, &x2, &x3, &x4);

	/* Determine which security state this SMC originated from */
	ns = is_caller_non_secure(flags);
	if (!ns) {
		/* SiP SMC service secure world's call */
		assert(handle == cm_get_context_by_index(cpu, SECURE));
		switch (smc_fid) {
		case MTK_SIP_KERNEL_AEE_DUMP_AARCH32:
		case MTK_SIP_KERNEL_AEE_DUMP_AARCH64:
			if (x1 == 0) {
				wdt_notify_other_cores(aee_wdt_dump, 0, 0);
				aee_wdt_dump(cookie);
			} else if (x1 == 1) {
				cm_el1_sysregs_context_restore(NON_SECURE);
				cm_set_next_eret_context(NON_SECURE);
				fiq_icc_isr();
			}
			SMC_RET0(handle);
			break;
		case MTK_SIP_GET_ROOT_OF_TRUST_AARCH32:
		case MTK_SIP_GET_ROOT_OF_TRUST_AARCH64: {
			uint64_t rc;
			uint32_t arg0, arg1, arg2, arg3;

			if (rot_write_enable)
				SMC_RET1(handle, SMC_UNK);
			rc = sip_get_root_of_trust_info(&arg0, &arg1, &arg2, &arg3);
			if (rc == 0)
				SMC_RET4(handle, arg0, arg1, arg2, arg3);
			SMC_RET1(handle, rc);
			break;
			}
#ifdef MTK_ENABLE_MPU_HAL_SUPPORT
		case MTK_SIP_TEE_MPU_PERM_SET_AARCH32:
		case MTK_SIP_TEE_MPU_PERM_SET_AARCH64: {
			rc = sip_tee_mpu_request(x1, x2, x3);
			SMC_RET1(handle, rc);
			break;
			}
#endif
		}
	} else {
		/* SiP SMC service normal world's call */
		ns_cpu_context = (cpu_context_t *) cm_get_context_by_index(cpu, NON_SECURE);
		spsr = SMC_GET_EL3(ns_cpu_context, CTX_SPSR_EL3);
		el = GET_EL(spsr);
		if (mtk_lk_stage == 1) {
			if (((el != MODE_EL2) && (GET_RW(spsr) == MODE_RW_64)) ||
				((el == MODE_EL2) && (lk_type == MODE_RW_64))) {
				/* INFO("originated from LK 64\n"); */
				switch (smc_fid) {
#ifdef MTK_ENABLE_MPU_HAL_SUPPORT
#ifdef MTK_ENABLE_GENIEZONE
				case MTK_SIP_HYP_MPU_PERM_SET_AARCH32:
				case MTK_SIP_HYP_MPU_PERM_SET_AARCH64:
					rc = sip_gz_mpu_request(x1, x2, x3);
					SMC_RET1(handle, rc);
					break;
#endif
#endif
				case MTK_SIP_LK_ROOT_OF_TRUST_AARCH64: {
					uint64_t rc;

					if (!rot_write_enable)
						SMC_RET1(handle, SMC_UNK);
					rc = sip_save_root_of_trust_info(x1, x2, x3, x4);
					SMC_RET1(handle, rc);
					break;
					}
#ifdef MTK_ATF_RAM_DUMP
				case MTK_SIP_RAM_DUMP_ADDR_AARCH64:
					atf_ram_dump_base = x1;
					atf_ram_dump_size = x2;
					SMC_RET0(handle);
					break;
#endif
				}
			} else if (((el != MODE_EL2) && (GET_RW(spsr) == MODE_RW_32) &&
				((spsr & MODE32_MASK) == MODE32_svc)) ||
				((el == MODE_EL2) && (lk_type == MODE_RW_32))) {
				switch (smc_fid) {
				case MTK_SIP_LK_ROOT_OF_TRUST_AARCH32: {
					uint64_t rc;

					if (!rot_write_enable)
						SMC_RET1(handle, SMC_UNK);
					rc = sip_save_root_of_trust_info(x1, x2, x3, x4);
					SMC_RET1(handle, rc);
					break;
					}
#if MTK_SIP_SET_AUTHORIZED_SECURE_REG_ENABLE
				/* Has been upstream by Tablet project; keep it */
				case MTK_SIP_SET_AUTHORIZED_SECURE_REG: {
					/* only use ret here */
					uint64_t ret;

					ret = mt_sip_set_authorized_sreg((uint32_t)x1,
						(uint32_t)x2);
					SMC_RET1(handle, ret);
					break;
					}
#endif
				case MTK_SIP_KERNEL_BOOT_AARCH32:
					/* post platform init, after LK stage */
					bl31_post_platform_setup();
					boot_to_kernel(x1, x2, x3, x4);
					SMC_RET0(handle);
					break;
				case MTK_SIP_LK_WDT_AARCH32:
					set_kernel_k32_64(LINUX_KERNEL_32);
					wdt_kernel_cb_addr = x1;
					NOTICE("MTK_SIP_LK_WDT : 0x%lx\n", wdt_kernel_cb_addr);
					rc = teearg->atf_aee_debug_buf_start;
					SMC_RET1(handle, rc);
					break;
				case MTK_SIP_LK_DUMP_ATF_LOG_INFO_AARCH32:
					console_init(teearg->atf_log_port, UART_CLOCK, UART_BAUDRATE);
					atf_crash_log_addr = mt_log_get_crash_log_addr();
					atf_crash_log_size = mt_log_get_crash_log_size();
					atf_crash_flag_addr_ptr = mt_log_get_crash_flag_addr();
					atf_crash_flag_addr = (uint64_t)atf_crash_flag_addr_ptr&0xffffffff;

#if defined(MTK_UT_PATTERN_0)
					/* generate raw data */
					*atf_crash_flag_addr_ptr = 0xFFFFFFFF;
#endif
					INFO("Get ATF log buffer info. for AEE dump in LK.\n");
					INFO("Buf addr:0x%lx, buf size:%lu, flag addr:0x%x, flag value:0x%x\n",
						atf_crash_log_addr, atf_crash_log_size,
						atf_crash_flag_addr, *atf_crash_flag_addr_ptr);

					console_uninit();
					SMC_RET3(handle, atf_crash_log_addr, atf_crash_log_size, atf_crash_flag_addr);
					break;
#ifdef MTK_ATF_RAM_DUMP
				case MTK_SIP_RAM_DUMP_ADDR_AARCH32:
					atf_ram_dump_base = x1<<32 | (x2&SMC32_PARAM_MASK);
					atf_ram_dump_size = x3<<32 | (x4&SMC32_PARAM_MASK);
					SMC_RET0(handle);
					break;
#endif
				case MTK_SIP_LK_LASTBUS_AARCH32:
					if (x1 == 0)
						rc = lastbus_perimon_init((uint32_t)x2, (uint32_t)x3);
					else if (x1 == 1)
						rc = lastbus_perimon_check_hang();
					else
						rc = lastbus_perimon_get((uint32_t)x2);
					SMC_RET1(handle, rc);
					break;
				case MTK_SIP_LK_LASTPC_AARCH32:
					lastpc_postinit();
					SMC_RET0(handle);
					break;
#ifdef MTK_ENABLE_MPU_HAL_SUPPORT
#ifdef MTK_ENABLE_GENIEZONE
				case MTK_SIP_HYP_MPU_PERM_SET_AARCH32:
				case MTK_SIP_HYP_MPU_PERM_SET_AARCH64:
					rc = sip_gz_mpu_request(x1, x2, x3);
					SMC_RET1(handle, rc);
					break;
#endif
#endif
				}
			}
		} else if (mtk_lk_stage == 0) {
			if (((el != MODE_EL2) && (GET_RW(spsr) == MODE_RW_64)) ||
				((el == MODE_EL2) && (kernel_type == MODE_RW_64))) {
				switch (smc_fid) {
				case MTK_SIP_KERNEL_WDT_AARCH64:
					wdt_kernel_cb_addr = x1;
					NOTICE("MTK_SIP_KERNEL_WDT : 0x%lx\n", wdt_kernel_cb_addr);
					NOTICE("teearg->atf_aee_debug_buf_start : 0x%lx\n",
						   teearg->atf_aee_debug_buf_start);
					rc = teearg->atf_aee_debug_buf_start;
					SMC_RET1(handle, rc);
					break;
				case MTK_SIP_KERNEL_TIME_SYNC_AARCH64:
					INFO("kernel time sync 0x%16lx 0x%16lx atf: 0x%16lx\n", x1, x2, sched_clock());
					/* in arch32, high 32 bits is stored in x2 and this would be 0 in arch64 */
					sched_clock_init(sched_clock() - (x1 + (x2 << 32)), 0);
					MT_LOG_KTIME_SET();
					rc = MTK_SIP_E_SUCCESS;
					SMC_RET1(handle, rc);
					break;
				case MTK_SIP_KERNEL_DO_FIQ_CACHE_AARCH64:
					cache_flush_all_by_fiq();
					rc = 0;
					SMC_RET1(handle, rc);
					break;
				case MTK_SIP_KERNEL_GIC_DUMP_AARCH64:
					rc = mt_irq_dump_status(x1);
					SMC_RET1(handle, rc);
					break;
#ifdef MTK_ENABLE_MPU_HAL_SUPPORT
#ifdef MTK_ENABLE_GENIEZONE
				case MTK_SIP_HYP_MPU_PERM_SET_AARCH32:
				case MTK_SIP_HYP_MPU_PERM_SET_AARCH64:
					rc = sip_gz_mpu_request(x1, x2, x3);
					SMC_RET1(handle, rc);
					break;
#endif
#endif
				case MTK_SIP_KERNEL_ATF_DEBUG_AARCH64:
#if (1 == TARGET_BUILD_VARIANT_ENG)
					switch (x1) {
					case GENERATE_ATF_CRASH_REPORT:
						{
						/* test code for ATF crash, limit address for security hole */
						x2 = x2 & SMC32_PARAM_MASK;
						if (x2 >= 0x0 && x2 <= 0x1000) {
							/* Jump to Null address and assert */
							void (*fp)(void);

							fp = (void (*)(void))x2;
							(*fp)();
							}
						}
						break;
					case ATF_TEST_SW_TRIGGER_WDT:
						gicd_set_ispendr(BASE_GICD_BASE, WDT_IRQ_BIT_ID);
						break;
					case ATF_TEST_DO_PANIC:
						INFO("UT for assertion!\n");
						assert(0);
					default:
						INFO("UT index 0x%lx is not support!\n", x1);
					}
#endif
					SMC_RET0(handle);
					break;

				case MTK_SIP_CMDQ_CONTROL_AARCH32:
				case MTK_SIP_CMDQ_CONTROL_AARCH64:
					smc_cmdq_request(x1);
					SMC_RET0(handle);
					break;
				}
			} else if (((el != MODE_EL2) && (GET_RW(spsr) == MODE_RW_32) &&
				((spsr & MODE32_MASK) == MODE32_svc)) ||
				((el == MODE_EL2) && (kernel_type == MODE_RW_32))) {
				/* INFO("originated from kernel 32\n"); */
				switch (smc_fid) {
#if MTK_SIP_SET_AUTHORIZED_SECURE_REG_ENABLE
				/* Has been upstream by Tablet project; keep it */
				case MTK_SIP_SET_AUTHORIZED_SECURE_REG: {
					/* only use ret here */
					uint64_t ret;

					ret = mt_sip_set_authorized_sreg((uint32_t)x1,
						(uint32_t)x2);
					SMC_RET1(handle, ret);
					break;
					}
#endif
				case MTK_SIP_KERNEL_WDT_AARCH32:
					wdt_kernel_cb_addr = x1;
					NOTICE("MTK_SIP_KERNEL_WDT : 0x%lx\n", wdt_kernel_cb_addr);
					NOTICE("teearg->atf_aee_debug_buf_start : 0x%lx\n",
						   teearg->atf_aee_debug_buf_start);
					rc = teearg->atf_aee_debug_buf_start;
					SMC_RET1(handle, rc);
					break;
				case MTK_SIP_KERNEL_TIME_SYNC_AARCH32:
					INFO("kernel time sync 0x%16lx 0x%16lx atf: 0x%16lx\n", x1, x2, sched_clock());
					/* in arch32, high 32 bits is stored in x2 and this would be 0 in arch64 */
					sched_clock_init(sched_clock() - (x1 + (x2 << 32)), 0);
					MT_LOG_KTIME_SET();
					rc = MTK_SIP_E_SUCCESS;
					SMC_RET1(handle, rc);
					break;
				case MTK_SIP_KERNEL_DO_FIQ_CACHE_AARCH32:
					cache_flush_all_by_fiq();
					rc = 0;
					SMC_RET1(handle, rc);
					break;
				case MTK_SIP_KERNEL_GIC_DUMP_AARCH32:
					rc = mt_irq_dump_status(x1);
					SMC_RET1(handle, rc);
					break;
				case MTK_SIP_KERNEL_ATF_DEBUG_AARCH32:
#if (1 == TARGET_BUILD_VARIANT_ENG)
					switch (x1) {
					case GENERATE_ATF_CRASH_REPORT:
						{
						/* test code for ATF crash, limit address for security hole */
						x2 = x2 & SMC32_PARAM_MASK;
						if (x2 >= 0x0 && x2 <= 0x1000) {
							/* Jump to Null address and assert */
							void (*fp)(void);

							fp = (void (*)(void))x2;
							(*fp)();
							}
						}
						break;
					case ATF_TEST_SW_TRIGGER_WDT:
						gicd_set_ispendr(BASE_GICD_BASE, WDT_IRQ_BIT_ID);
						break;
					case ATF_TEST_DO_PANIC:
						INFO("UT for assertion!\n");
						assert(0);
					default:
						INFO("UT index 0x%lx is not support!\n", x1);
					}
#endif
					SMC_RET0(handle);
					break;
				}
			}
		}
	}
	return mediatek_plat_sip_handler(smc_fid, x1, x2, x3, x4,
					cookie, handle, flags);
}

/*
 * This function is responsible for handling all SiP calls from the NS world
 */
uint64_t sip_smc_handler(uint32_t smc_fid,
			 uint64_t x1,
			 uint64_t x2,
			 uint64_t x3,
			 uint64_t x4,
			 void *cookie,
			 void *handle,
			 uint64_t flags)
{
	switch (smc_fid) {
	case SIP_SVC_CALL_COUNT:
		/* Return the number of Mediatek SiP Service Calls. */
		SMC_RET1(handle,
			 MTK_COMMON_SIP_NUM_CALLS + MTK_PLAT_SIP_NUM_CALLS);

	case SIP_SVC_UID:
		/* Return UID to the caller */
		SMC_UUID_RET(handle, mtk_sip_svc_uid);

	case SIP_SVC_VERSION:
		/* Return the version of current implementation */
		SMC_RET2(handle, MTK_SIP_SVC_VERSION_MAJOR,
			MTK_SIP_SVC_VERSION_MINOR);

	default:
		return mediatek_sip_handler(smc_fid, x1, x2, x3, x4,
			cookie, handle, flags);
	}
}

/* Define a runtime service descriptor for fast SMC calls */
DECLARE_RT_SVC(
	mediatek_sip_svc,
	OEN_SIP_START,
	OEN_SIP_END,
	SMC_TYPE_FAST,
	NULL,
	sip_smc_handler
);
