/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2014. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include <arch.h>
#include <arch_helpers.h>
#include <console.h>
#include <context_mgmt.h>
#include <debug.h>
#include <mt_spm.h>
#include <mt_mpu.h>
#include <mtk_plat_common.h>
#include <plat_debug.h>
#include <plat_sip_svc.h>
#include <platform.h>
#include <platform_def.h>
#include <runtime_svc.h>
#include <mtk_sip_svc.h>
#include <plat_pm.h>
#include <mtk_i2c.h>

#include <mt_spm.h>
#include <mt_spm_vcorefs.h>
#include <mt_spm_dpidle.h>
#include <mt_spm_sodi.h>
#include <mt_spm_idle.h>
#include <devapc.h>
#include <rpmb_hmac.h>
#include <crypto_hw.h>
#include <fiq_cache.h>
#include <rng.h>
#include <amms.h>
#include <crypto.h>
#include <md.h>
#include <msdc.h>
#include <crypto_util.h>

/*******************************************************************************
 * SIP top level handler for servicing SMCs.
 ******************************************************************************/
uint64_t mediatek_plat_sip_handler(uint32_t smc_fid,
			  uint64_t x1,
			  uint64_t x2,
			  uint64_t x3,
			  uint64_t x4,
			  void *cookie,
			  void *handle,
			  uint64_t flags)
{
	uint64_t rc;
	uint32_t ns;
	struct atf_arg_t *teearg;

	rc = 0;
	teearg = &gteearg;
	uint32_t rnd_val0 = 0, rnd_val1 = 0, rnd_val2 = 0, rnd_val3 = 0;

	cpu_context_t *ns_cpu_context;
	uint32_t mpidr = 0;
	uint32_t linear_id = 0;
	uint64_t spsr = 0;


	mpidr = read_mpidr();
	linear_id = platform_get_core_pos(mpidr);
	ns_cpu_context = (cpu_context_t *) cm_get_context_by_index(linear_id, NON_SECURE);

	/* Determine which security state this SMC originated from */
	ns = is_caller_non_secure(flags);
	if (!ns) {
		/***************************************/
		/* SiP SMC service secure world's call */
		/***************************************/
		switch (smc_fid) {

		default:
			rc = SMC_UNK;
			console_init(teearg->atf_log_port, UART_CLOCK, UART_BAUDRATE);
			ERROR("%s: unhandled Sec SMC (0x%x)\n", __func__, smc_fid);
			console_uninit();
		}

	} else {
		/***************************************/
		/* SiP SMC service normal world's call */
		/***************************************/
		spsr = SMC_GET_EL3(ns_cpu_context, CTX_SPSR_EL3);
		if (mtk_lk_stage == 1) {
			if (GET_RW(spsr) == MODE_RW_64) {
				/* INFO("originated from LK 64\n"); */
				switch (smc_fid) {
				default:
					rc = SMC_UNK;
					console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE);
					ERROR("%s: unknown 64b LK SMC(0x%x)\n", __func__, smc_fid);
					console_uninit();
				}
			} else if ((GET_RW(spsr) == MODE_RW_32) &&
					((spsr & MODE32_MASK) == MODE32_svc)) {
				/* INFO("originated from LK 32\n"); */
				switch (smc_fid) {
				case MTK_SIP_LK_MD_REG_WRITE_AARCH32:
					rc = sip_write_md_regs((uint32_t)x1, (uint32_t)x2, (uint32_t)x3, (uint32_t)x4);
					break;
#ifdef __MTK_RPMB
				case MTK_SIP_LK_RPMB_INIT_AARCH32:
					/* create shared memory for rpmb atf module */
					rpmb_init();
					break;
				case MTK_SIP_LK_RPMB_UNINIT_AARCH32:
					/* mark leaving lk and release resources. */
					rpmb_uninit();
					break;
				case MTK_SIP_LK_RPMB_HMAC_AARCH32:
					/* rpmb hmac calculation module */
					rc = rpmb_hmac(x1, x2);
					SMC_RET1(handle, rc);
					break;
#endif
				case MTK_SIP_LK_CRYPTO_HW_ENGINE_INIT_AARCH32:
					rc = lk_share_mem_check(x1, x2);
					if (rc) {
						SMC_RET1(handle, rc);
						break;
					}
					rc = crypto_hw_init((intptr_t)x1, x2);
					SMC_RET1(handle, rc);
					break;
				case MTK_SIP_LK_CRYPTO_HW_ENGINE_DISABLE_AARCH32:
					rc = crypto_hw_disable();
					SMC_RET1(handle, rc);
					break;
				case MTK_SIP_LK_CRYPTO_SHA256_INIT_AARCH32:
					rc = sha256hw_init();
					SMC_RET1(handle, rc);
					break;
				case MTK_SIP_LK_CRYPTO_SHA256_PROCESS_AARCH32:
					rc = sha256hw_process((const intptr_t)x1, (unsigned long)x2);
					SMC_RET1(handle, rc);
					break;
				case MTK_SIP_LK_CRYPTO_SHA256_DONE_AARCH32:
					rc = sha256hw_done();
					SMC_RET1(handle, rc);
					break;
				case MTK_SIP_LK_GET_RND_AARCH32:
					rc = plat_get_rnd(&rnd_val0);
					SMC_RET2(handle, rc, rnd_val0);
					break;
				/*amms*/
				case MTK_SIP_LK_AMMS_MD_BASE_ADDR_AARCH32:
					rc = sip_amms_set_md_base_address((unsigned long long)x1);
					break;
				case MTK_SIP_LK_AMMS_GET_MD_BASE_ADDR_AARCH32:
					rc = sip_amms_get_md_base_address();
					SMC_RET1(handle, rc);
					break;
				case MTK_SIP_LK_DAPC_INIT_AARCH32:
					if (!x1 & !x2 & !x3) {
						/* Setup DEVAPC in ATF */
						rc = start_devapc();
					} else {
						rc = dump_devapc(x1, x2, x3);
					}
					break;
				/* --- LK Kernel Common ---*/
				case MTK_SIP_KERNEL_SPM_ARGS_AARCH32:
					spm_args(x1, x2, x3);
					break;
				/*MPU*/
				case MTK_SIP_KERNEL_EMIMPU_READ_AARCH32:
					rc = sip_emi_mpu_read(x1);
					break;
				case MTK_SIP_KERNEL_EMIMPU_WRITE_AARCH32:
					rc = sip_emi_mpu_write(x1, x2);
					break;
				case MTK_SIP_KERNEL_EMIMPU_CLEAR_AARCH32:
					rc = sip_emi_mpu_clear_protection(x1);
					break;
				case MTK_SIP_KERNEL_EMIMPU_SET_AARCH32:
					rc = sip_emi_mpu_set_protection(x1, x2, x3);
					break;
				/*--- End of LK Kernel Common ---*/
				default:
					rc = SMC_UNK;
					console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE);
					ERROR("%s: unknown 32b LK SMC(0x%x)\n", __func__, smc_fid);
					console_uninit();
				}
			} else {
				rc = SMC_UNK;
				console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE);
				ERROR("%s: unknown LK SMC(0x%x)\n", __func__, smc_fid);
				console_uninit();
			}
		} else if (mtk_lk_stage == 0) {
			if (GET_RW(spsr) == MODE_RW_64) {
				/* INFO("originated from kernel 64\n"); */
				switch (smc_fid) {
				case MTK_SIP_KERNEL_SPM_SUSPEND_ARGS_AARCH64:
					spm_suspend_args(x1, x2, x3, x4);
					break;
				case MTK_SIP_KERNEL_SPM_FIRMWARE_STATUS_AARCH64:
					rc = spm_load_firmware_status();
					break;
				case MTK_SIP_KERNEL_SPM_IRQ0_HANDLER_AARCH64:
					spm_irq0_handler(x1);
					break;
				case MTK_SIP_KERNEL_SPM_AP_MDSRC_REQ_AARCH64:
					spm_ap_mdsrc_req(x1);
					break;
				case MTK_SIP_KERNEL_DO_FIQ_CACHE_AARCH64:
					cache_flush_all_by_fiq();
					rc = 0;
					break;
				case MTK_SIP_KERNEL_SPM_PWR_CTRL_ARGS_AARCH64:
					spm_pwr_ctrl_args(x1, x2, x3);
					break;
				case MTK_SIP_KERNEL_SPM_GET_PWR_CTRL_ARGS_AARCH64:
					rc = spm_get_pwr_ctrl_args(x1, x2, x3);
					break;
				case MTK_SIP_KERNEL_SPM_LEGACY_SLEEP_AARCH64:
					/* spm_legacy_sleep_wfi(x1, x2, x3); */
					break;
				case MTK_SIP_KERNEL_SPM_VCOREFS_ARGS_AARCH64:
					spm_vcorefs_args(x1, x2, x3);
					break;
				case MTK_SIP_KERNEL_SPM_DPIDLE_ARGS_AARCH64:
					spm_dpidle_args(x1, x2, x3);
					break;
				case MTK_SIP_KERNEL_SPM_SODI_ARGS_AARCH64:
					spm_sodi_args(x1, x2, x3);
					break;
				case MTK_SIP_KERNEL_SPM_SLEEP_DPIDLE_ARGS_AARCH64:
					spm_sleep_dpidle_args(x1, x2, x3);
					break;
				case MTK_SIP_KERNEL_CHECK_SECURE_CG_AARCH64:
					rc = spm_idle_check_secure_cg(x1, x2, x3);
					break;
				case MTK_SIP_KERNEL_SPM_DUMMY_READ_AARCH64:
					/* spm_dummy_read(x1, x2); */
					break;
				case MTK_SIP_KERNEL_SPM_ARGS_AARCH64:
					spm_args(x1, x2, x3);
					break;
				case MTK_SIP_KERNEL_I2C_SEC_WRITE_AARCH64:
					rc = i2c_set_secure_reg(x1, x2, x3);
					break;
				case MTK_SIP_KERNEL_GET_RND_AARCH64:
					INFO("[TRNG] get notify from kernel!\n");
					if (x1  == 0x74726e67) {
						rc = plat_get_rnd(&rnd_val0);
						if (rc != 0) {
							INFO("[TRNG] plat_get_rnd 0 failed!\n");
							break;
						}
						rc = plat_get_rnd(&rnd_val1);
						if (rc != 0) {
							INFO("[TRNG] plat_get_rnd 1 failed!\n");
							break;
						}
						rc = plat_get_rnd(&rnd_val2);
						if (rc != 0) {
							INFO("[TRNG] plat_get_rnd 2 failed!\n");
							break;
						}
						rc = plat_get_rnd(&rnd_val3);
						if (rc != 0) {
							INFO("[TRNG] plat_get_rnd 3 failed!\n");
							break;
						}
						SMC_RET4(handle, rnd_val0, rnd_val1, rnd_val2, rnd_val3);
					} else {
						INFO("[TRNG] wrong magic number!\n");
					}
					break;
				case MTK_SIP_KERNEL_AMMS_GET_FREE_ADDR_AARCH64:
					rc = sip_kernel_amms_get_free_addr();
					SMC_RET1(handle, rc);
					break;
				case MTK_SIP_KERNEL_AMMS_GET_FREE_LENGTH_AARCH64:
					rc = sip_kernel_amms_get_free_length();
					SMC_RET1(handle, rc);
					break;
#ifdef MTK_MSDC_HW_FDE
				case MTK_SIP_KERNEL_HW_FDE_MSDC_CTL_AARCH64:
					rc = msdc_crypto_ctl(x1, x2, x3);
					break;
#endif
				case MTK_SIP_KERNEL_HW_FDE_KEY_AARCH64:
					rc = hwfde_set_key(x1, x2, x3);
					break;
				/*MPU*/
				case MTK_SIP_KERNEL_EMIMPU_READ_AARCH64:
					rc = sip_emi_mpu_read(x1);
					break;
				case MTK_SIP_KERNEL_EMIMPU_WRITE_AARCH64:
					rc = sip_emi_mpu_write(x1, x2);
					break;
				case MTK_SIP_KERNEL_EMIMPU_CLEAR_AARCH64:
					rc = sip_emi_mpu_clear_protection(x1);
					break;
				case MTK_SIP_KERNEL_EMIMPU_SET_AARCH64:
					rc = sip_emi_mpu_set_protection(x1, x2, x3);
					break;
				case MTK_SIP_KERNEL_DFD_AARCH64:
					rc = dfd_smc_dispatcher(x1, x2, x3);
					break;
				case MTK_SIP_POWER_DOWN_CORE_AARCH64:
					mt_core_ops(x1, x2, 0);
					break;
				case MTK_SIP_POWER_DOWN_CLUSTER_AARCH64:
					mt_cluster_ops(x1, x2, 0);
					break;
				case MTK_SIP_POWER_FLOW_DEBUG_AARCH64:
					break;
				default:
					rc = SMC_UNK;
					console_init(teearg->atf_log_port, UART_CLOCK, UART_BAUDRATE);
					ERROR("%s: unknown 64B kernel SMC(0x%x)\n", __func__, smc_fid);
					console_uninit();
				}
			} else if ((GET_RW(spsr) == MODE_RW_32) &&
				((spsr & MODE32_MASK) == MODE32_svc)) {
				/* INFO("originated from kernel 32\n"); */
				switch (smc_fid) {
				case MTK_SIP_KERNEL_SPM_SUSPEND_ARGS_AARCH32:
					spm_suspend_args(x1, x2, x3, x4);
					break;
				case MTK_SIP_KERNEL_SPM_FIRMWARE_STATUS_AARCH32:
					rc = spm_load_firmware_status();
					break;
				case MTK_SIP_KERNEL_SPM_IRQ0_HANDLER_AARCH32:
					spm_irq0_handler(x1);
					break;
				case MTK_SIP_KERNEL_SPM_AP_MDSRC_REQ_AARCH32:
					spm_ap_mdsrc_req(x1);
					break;
				case MTK_SIP_KERNEL_DO_FIQ_CACHE_AARCH32:
					cache_flush_all_by_fiq();
					rc = 0;
					break;
				case MTK_SIP_KERNEL_SPM_PWR_CTRL_ARGS_AARCH32:
					spm_pwr_ctrl_args(x1, x2, x3);
					break;
				case MTK_SIP_KERNEL_SPM_GET_PWR_CTRL_ARGS_AARCH32:
					rc = spm_get_pwr_ctrl_args(x1, x2, x3);
					break;
				case MTK_SIP_KERNEL_SPM_LEGACY_SLEEP_AARCH32:
					/* spm_legacy_sleep_wfi(x1, x2, x3); */
					break;
				case MTK_SIP_KERNEL_SPM_VCOREFS_ARGS_AARCH32:
					spm_vcorefs_args(x1, x2, x3);
					break;
				case MTK_SIP_KERNEL_SPM_DPIDLE_ARGS_AARCH32:
					spm_dpidle_args(x1, x2, x3);
					break;
				case MTK_SIP_KERNEL_SPM_SODI_ARGS_AARCH32:
					spm_sodi_args(x1, x2, x3);
					break;
				case MTK_SIP_KERNEL_SPM_SLEEP_DPIDLE_ARGS_AARCH32:
					spm_sleep_dpidle_args(x1, x2, x3);
					break;
				case MTK_SIP_KERNEL_CHECK_SECURE_CG_AARCH32:
					rc = spm_idle_check_secure_cg(x1, x2, x3);
					break;
				case MTK_SIP_KERNEL_SPM_DUMMY_READ_AARCH32:
					/* spm_dummy_read(x1, x2); */
					break;
				case MTK_SIP_KERNEL_SPM_ARGS_AARCH32:
					spm_args(x1, x2, x3);
					break;
				case MTK_SIP_KERNEL_I2C_SEC_WRITE_AARCH32:
					rc = i2c_set_secure_reg(x1, x2, x3);
					break;
				case MTK_SIP_KERNEL_GET_RND_AARCH32:
					INFO("[TRNG] get notify from kernel!\n");
					if (x1  == 0x74726e67) {
						rc = plat_get_rnd(&rnd_val0);
						if (rc != 0) {
							INFO("[TRNG] plat_get_rnd 0 failed!\n");
							break;
						}
						rc = plat_get_rnd(&rnd_val1);
						if (rc != 0) {
							INFO("[TRNG] plat_get_rnd 1 failed!\n");
							break;
						}
						rc = plat_get_rnd(&rnd_val2);
						if (rc != 0) {
							INFO("[TRNG] plat_get_rnd 2 failed!\n");
							break;
						}
						rc = plat_get_rnd(&rnd_val3);
						if (rc != 0) {
							INFO("[TRNG] plat_get_rnd 3 failed!\n");
							break;
						}
						SMC_RET4(handle, rnd_val0, rnd_val1, rnd_val2, rnd_val3);
					} else {
						INFO("[TRNG] wrong magic number!\n");
					}
					break;
				case MTK_SIP_KERNEL_AMMS_GET_FREE_ADDR_AARCH32:
					rc = sip_kernel_amms_get_free_addr();
					SMC_RET1(handle, rc);
					break;
				case MTK_SIP_KERNEL_AMMS_GET_FREE_LENGTH_AARCH32:
					rc = sip_kernel_amms_get_free_length();
					SMC_RET1(handle, rc);
					break;
#ifdef MTK_MSDC_HW_FDE
				case MTK_SIP_KERNEL_HW_FDE_MSDC_CTL_AARCH32:
					rc = msdc_crypto_ctl(x1, x2, x3);
					break;
#endif
				case MTK_SIP_KERNEL_HW_FDE_KEY_AARCH32:
					rc = hwfde_set_key(x1, x2, x3);
					break;
				/*MPU*/
				case MTK_SIP_KERNEL_EMIMPU_READ_AARCH32:
					rc = sip_emi_mpu_read(x1);
					break;
				case MTK_SIP_KERNEL_EMIMPU_WRITE_AARCH32:
					rc = sip_emi_mpu_write(x1, x2);
					break;
				case MTK_SIP_KERNEL_EMIMPU_CLEAR_AARCH32:
					rc = sip_emi_mpu_clear_protection(x1);
					break;
				case MTK_SIP_KERNEL_EMIMPU_SET_AARCH32:
					rc = sip_emi_mpu_set_protection(x1, x2, x3);
					break;
				case MTK_SIP_KERNEL_DFD_AARCH32:
					rc = dfd_smc_dispatcher(x1, x2, x3);
					break;
				case MTK_SIP_POWER_DOWN_CORE_AARCH32:
					mt_core_ops(x1, x2, 0);
					break;
				case MTK_SIP_POWER_DOWN_CLUSTER_AARCH32:
					mt_cluster_ops(x1, x2, 0);
					break;
				case MTK_SIP_POWER_FLOW_DEBUG_AARCH32:
					break;
				default:
					rc = SMC_UNK;
					console_init(teearg->atf_log_port, UART_CLOCK, UART_BAUDRATE);
					ERROR("%s: unknown 32B kernel SMC(0x%x)\n", __func__, smc_fid);
					console_uninit();
				}
			} else {
				rc = SMC_UNK;
				console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE);
				ERROR("%s: unknown kernel stage SMC(0x%x)\n", __func__, smc_fid);
				console_uninit();
			}
		} else {	/* mtk_lk_stage */
			rc = SMC_UNK;
			console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE);
			ERROR("%s: unknown stage SMC (0x%x)\n", __func__, smc_fid);
			console_uninit();
		}
	}
	SMC_RET1(handle, rc);
}

