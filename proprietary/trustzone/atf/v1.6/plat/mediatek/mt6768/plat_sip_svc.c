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
#include <bl_common.h>
#include <console.h>
#include <context_mgmt.h>
#include <debug.h>
#include <mt_spm.h>
#include <mtk_plat_common.h>
#include <plat_debug.h>
#include <plat_sip_svc.h>
#include <platform.h>
#include <platform_def.h>
#include <runtime_svc.h>
#include <mtk_sip_svc.h>
#include <plat_debug.h>
#include <plat_pm.h>

#include <mtk_mcdi.h>
#include <mt_spm.h>
#include <mt_spm_vcorefs.h>
#include <mt_spm_idle.h>
#include <rpmb_hmac.h>
#include <crypto_hw.h>
#include <rng.h>
#include <md.h>
#include <mtk_drcc.h>
#include <mtk_udi_api.h>
#include <crypto.h>
#include <ufs_crypto.h>
#include <msdc_crypto.h>
#include <mt_i2c.h>
#include <mt_ildo_atf.h>
#include <devapc.h>
#include <mpu_v1.h>
#include <amms.h>
#include <eint.h>
#include <ccci.h>
#if defined(MTK_DEVMPU_SUPPORT)
#include <devmpu.h>
#endif
#if defined(MTK_ENABLE_MPU_HAL_SUPPORT)
#include <mpu_ctrl/mpu_api.h>
#endif
#include <mt_spm_syspll.h>
#include <mt_spm_dram_bus26m.h>
#include <crypto_util.h>

uint64_t mediatek_plat_sip_handler_secure(uint32_t smc_fid,
			  uint64_t x1,
			  uint64_t x2,
			  uint64_t x3,
			  uint64_t x4,
			  void *cookie,
			  void *handle,
			  uint64_t flags)
{
	uint64_t rc = 0;


	switch (smc_fid) {
	case MTK_SIP_TEE_SEC_DEINT_CONFIGURE_AARCH32:
	case MTK_SIP_TEE_SEC_DEINT_CONFIGURE_AARCH64:
		rc = mt_eint_set_secure_deint(x1, x2);
		break;
	case MTK_SIP_TEE_SEC_DEINT_RELEASE_AARCH32:
	case MTK_SIP_TEE_SEC_DEINT_RELEASE_AARCH64:
		rc = mt_eint_clr_deint(x1);
		break;
	case MTK_SIP_TEE_APC_MODULE_SET_AARCH32:
	case MTK_SIP_TEE_APC_MODULE_SET_AARCH64:
		rc = sip_tee_apc_request(SIP_APC_MODULE_SET, x1, x2, x3);
		SMC_RET1(handle, rc);
		break;
	case MTK_SIP_TEE_APC_MM2ND_SET_AARCH32:
	case MTK_SIP_TEE_APC_MM2ND_SET_AARCH64:
		rc = sip_tee_apc_request(SIP_APC_MM2ND_SET, x1, x2, x3);
		SMC_RET1(handle, rc);
		break;
	case MTK_SIP_TEE_APC_MASTER_SET_AARCH32:
	case MTK_SIP_TEE_APC_MASTER_SET_AARCH64:
		rc = sip_tee_apc_request(SIP_APC_MASTER_SET, x1, x2, x3);
		SMC_RET1(handle, rc);
		break;
	default:
		rc = SMC_UNK;
		ERROR("%s: unhandled Sec SMC (0x%x)\n", __func__, smc_fid);
	}
	SMC_RET1(handle, rc);
}

uint64_t mediatek_plat_sip_handler_hypervisor(uint32_t smc_fid,
			  uint64_t x1,
			  uint64_t x2,
			  uint64_t x3,
			  uint64_t x4,
			  void *cookie,
			  void *handle,
			  uint64_t flags)
{
	uint64_t rc = 0;

	switch (smc_fid) {
#ifdef MTK_ENABLE_GENIEZONE
#ifdef MTK_DEVMPU_SUPPORT
	case MTK_SIP_HYP_DEVMPU_PERM_SET_AARCH32:
	case MTK_SIP_HYP_DEVMPU_PERM_SET_AARCH64:
		rc = sip_devmpu_rw_perm_set(x1, x2, x3);
		SMC_RET1(handle, rc);
		break;
#endif
#endif
	default:
		rc = SMC_UNK;
		ERROR("%s: unknown hypervisor SMC(0x%x)\n", __func__, smc_fid);
	}
	SMC_RET1(handle, rc);
}

uint64_t mediatek_plat_sip_handler_bootloader(uint32_t smc_fid,
			  uint64_t x1,
			  uint64_t x2,
			  uint64_t x3,
			  uint64_t x4,
			  void *cookie,
			  void *handle,
			  uint64_t flags)
{
	uint64_t rc = 0;
	uint32_t rnd_val0 = 0;

	switch (smc_fid) {
	case MTK_SIP_LK_MD_REG_WRITE_AARCH32:
		rc = sip_write_md_regs((uint32_t)x1, (uint32_t)x2, (uint32_t)x3, (uint32_t)x4);
		break;
#ifndef ATF_BYPASS_DRAM
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
#if ILDO_ENABLE
	case MTK_SIP_LK_ILDO_READ_EFUSE_AARCH32:
		rc = ildo_read_efuse_from_lk(x1, x2);
		break;
#endif
	case MTK_SIP_LK_PLAT_DEBUG_AARCH32:
		rc = plat_debug_smc_dispatcher(x1, x2, x3);
		break;
	case MTK_SIP_LK_DAPC_INIT_AARCH32:
		if (!x1 & !x2 & !x3) {
			/* Setup DEVAPC in ATF */
			rc = start_devapc();
		}
		break;
#endif /* #ifndef ATF_BYPASS_DRAM */
	case MTK_SIP_LK_AMMS_MD_BASE_ADDR_AARCH32:
		rc = sip_amms_set_md_base_address(x1);
		break;
	case MTK_SIP_LK_AMMS_GET_MD_BASE_ADDR_AARCH32:
		rc = sip_amms_get_md_base_address();
		SMC_RET1(handle, rc);
		break;
	case MTK_SIP_LK_AMMS_MD_POS_ADDR_AARCH32:
		rc = sip_amms_set_md_pos_addr(x1);
		break;
	case MTK_SIP_LK_AMMS_MD_POS_LENGTH_AARCH32:
		rc = sip_amms_set_md_pos_length(x1);
		break;
	case MTK_SIP_LK_AMMS_GET_MD_POS_ADDR_AARCH32:
		rc = sip_amms_get_md_pos_addr();
		SMC_RET1(handle, rc);
		break;
	case MTK_SIP_LK_AMMS_GET_MD_POS_LENGTH_AARCH32:
		rc = sip_amms_get_md_pos_length();
		SMC_RET1(handle, rc);
		break;
	case MTK_SIP_LK_AMMS_MD_POS_MD_VIEW_ADDR_AARCH32:
		rc = sip_amms_set_md_view_pos_addr(x1);
		break;
	case MTK_SIP_LK_AMMS_GET_MD_POS_MD_VIEW_ADDR_AARCH32:
		rc = sip_amms_get_md_view_pos_addr();
		SMC_RET1(handle, rc);
		break;
	/* --- LK Kernel Common ---*/
	case MTK_SIP_KERNEL_MCDI_ARGS_AARCH32:
		mcdi_handler(x1, x2, x3);
		break;
	case MTK_SIP_KERNEL_SPM_ARGS_AARCH32:
		spm_args(x1, x2, x3);
		break;
	/* MPU */
	case MTK_SIP_KERNEL_EMIMPU_READ_AARCH32:
		rc = sip_emi_mpu_read(x1);
		break;
	case MTK_SIP_KERNEL_EMIMPU_WRITE_AARCH32:
		rc = sip_emi_mpu_write(x1, x2);
		break;
	case MTK_SIP_KERNEL_EMIMPU_CLEAR_AARCH32:
#if defined(MTK_ENABLE_MPU_HAL_SUPPORT)
		rc = sip_mpu_request_ree_perm_check(x1);
		if (rc)
			break;
#endif
		rc = sip_emi_mpu_clear_protection(x1);
		break;
	case MTK_SIP_KERNEL_EMIMPU_SET_AARCH32:
#if defined(MTK_ENABLE_MPU_HAL_SUPPORT)
		rc = sip_mpu_request_ree_perm_check(
			(((uint32_t)x1) >> 24) & 0xFF);
		if (rc)
			break;
#endif
		rc = sip_emi_mpu_set_protection(x1, x2, x3);
		break;
#ifdef MTK_DEVMPU_SUPPORT
	case MTK_SIP_KERNEL_DEVMPU_VIO_GET_AARCH32:
		rc = sip_devmpu_vio_get(x1, &x1, &x2);
		SMC_RET3(handle, rc, x1, x2);
		break;
#if (1 == TARGET_BUILD_VARIANT_ENG)
	case MTK_SIP_KERNEL_DEVMPU_PERM_GET_AARCH32:
		rc = sip_devmpu_rw_perm_get(x1, &x1, &x2);
		SMC_RET3(handle, rc, x1, x2);
		break;
#endif
#endif
	/*--- End of LK Kernel Common ---*/
	default:
		rc = SMC_UNK;
		ERROR("%s: unknown LK SMC(0x%x)\n", __func__, smc_fid);
	}
	SMC_RET1(handle, rc);
}

uint64_t mediatek_plat_sip_handler_kernel(uint32_t smc_fid,
			  uint64_t x1,
			  uint64_t x2,
			  uint64_t x3,
			  uint64_t x4,
			  void *cookie,
			  void *handle,
			  uint64_t flags)
{
	uint64_t rc = 0;
	uint32_t rnd_val0 = 0, rnd_val1 = 0, rnd_val2 = 0, rnd_val3 = 0;


	switch (smc_fid) {
#ifndef ATF_BYPASS_DRAM
	case MTK_SIP_KERNEL_MCDI_ARGS_AARCH32:
	case MTK_SIP_KERNEL_MCDI_ARGS_AARCH64:
		mcdi_handler(x1, x2, x3);
		break;
	case MTK_SIP_KERNEL_I2C_SEC_WRITE_AARCH32:
	case MTK_SIP_KERNEL_I2C_SEC_WRITE_AARCH64:
		rc = i2c_set_secure_reg(x1, x2, x3);
		break;
	case MTK_SIP_KERNEL_SPM_SUSPEND_ARGS_AARCH32:
	case MTK_SIP_KERNEL_SPM_SUSPEND_ARGS_AARCH64:
		spm_suspend_args(x1, x2, x3, x4);
		break;
	case MTK_SIP_KERNEL_SPM_FIRMWARE_STATUS_AARCH32:
	case MTK_SIP_KERNEL_SPM_FIRMWARE_STATUS_AARCH64:
		rc = spm_load_firmware_status();
		break;
	case MTK_SIP_KERNEL_SPM_IRQ0_HANDLER_AARCH32:
	case MTK_SIP_KERNEL_SPM_IRQ0_HANDLER_AARCH64:
		spm_irq0_handler(x1, x2);
		break;
	case MTK_SIP_KERNEL_SPM_AP_MDSRC_REQ_AARCH32:
	case MTK_SIP_KERNEL_SPM_AP_MDSRC_REQ_AARCH64:
		spm_ap_mdsrc_req(x1);
		break;
	case MTK_SIP_KERNEL_SPM_PWR_CTRL_ARGS_AARCH32:
	case MTK_SIP_KERNEL_SPM_PWR_CTRL_ARGS_AARCH64:
		spm_pwr_ctrl_args(x1, x2, x3);
		break;
	case MTK_SIP_KERNEL_SPM_GET_PWR_CTRL_ARGS_AARCH32:
	case MTK_SIP_KERNEL_SPM_GET_PWR_CTRL_ARGS_AARCH64:
		rc = spm_get_pwr_ctrl_args(x1, x2, x3);
		break;
	case MTK_SIP_KERNEL_SPM_LEGACY_SLEEP_AARCH32:
	case MTK_SIP_KERNEL_SPM_LEGACY_SLEEP_AARCH64:
		/* spm_legacy_sleep_wfi(x1, x2, x3); */
		break;
	case MTK_SIP_KERNEL_SPM_VCOREFS_ARGS_AARCH32:
	case MTK_SIP_KERNEL_SPM_VCOREFS_ARGS_AARCH64:
		spm_vcorefs_args(x1, x2, x3);
		break;
	case MTK_SIP_KERNEL_SPM_LP_ARGS_AARCH32:
	case MTK_SIP_KERNEL_SPM_LP_ARGS_AARCH64:
		if (IS_SPM_LP_SMC(IDLE_MODEL_BUS26M, x1))
			spm_dram_bus26m_args(x2, x3, x4);
		else if (IS_SPM_LP_SMC(IDLE_MODEL_DRAM, x1))
			spm_dram_bus26m_args(x2, x3, x4);
		else if (IS_SPM_LP_SMC(IDLE_MODEL_SYSPLL, x1))
			spm_syspll_args(x2, x3, x4);
		else
			ERROR("%s: Invalid SPM_LP_ARGS(0x%llx)\n", __func__, x1);
		break;
	case MTK_SIP_KERNEL_CHECK_SECURE_CG_AARCH32:
	case MTK_SIP_KERNEL_CHECK_SECURE_CG_AARCH64:
		rc = spm_idle_check_secure_cg(x1, x2, x3);
		break;
#ifdef MTK_UFS_SUPPORT
	case MTK_SIP_KERNEL_HW_FDE_UFS_CTL_AARCH32:
	case MTK_SIP_KERNEL_HW_FDE_UFS_CTL_AARCH64:
		rc = ufs_crypto_ctl(x1, x2, x3);
		break;
	case MTK_SIP_KERNEL_UFS_CTL_AARCH32:
	case MTK_SIP_KERNEL_UFS_CTL_AARCH64:
		rc = ufs_generic_ctl(x1, x2, x3);
		break;
#endif
	case MTK_SIP_KERNEL_DAPC_PERM_GET_AARCH32:
	case MTK_SIP_KERNEL_DAPC_PERM_GET_AARCH64:
		rc = devapc_perm_get(x1, x2, x3);
		break;
	case MTK_SIP_KERNEL_CLR_SRAMROM_VIO_AARCH32:
	case MTK_SIP_KERNEL_CLR_SRAMROM_VIO_AARCH64:
		rc = handle_sramrom_vio(&x1, &x2);
		SMC_RET3(handle, rc, x1, x2);
		break;
	case MTK_SIP_KERNEL_GET_RND_AARCH32:
	case MTK_SIP_KERNEL_GET_RND_AARCH64:
		if (x1  == 0x74726e67) {
			rc = plat_get_rnd(&rnd_val0);
			if (rc != 0)
				break;
			rc = plat_get_rnd(&rnd_val1);
			if (rc != 0)
				break;
			rc = plat_get_rnd(&rnd_val2);
			if (rc != 0)
				break;
			rc = plat_get_rnd(&rnd_val3);
			if (rc != 0)
				break;
			SMC_RET4(handle, rnd_val0, rnd_val1, rnd_val2, rnd_val3);
		} else {
			ERROR("%s: wrong magic number for SMC(0x%x)\n", __func__, smc_fid);
		}
		break;
#ifdef MTK_MSDC_HW_FDE
	case MTK_SIP_KERNEL_HW_FDE_MSDC_CTL_AARCH32:
	case MTK_SIP_KERNEL_HW_FDE_MSDC_CTL_AARCH64:
		rc = msdc_crypto_ctl(x1, x2, x3);
		break;
#endif
	case MTK_SIP_KERNEL_CCCI_GET_INFO_AARCH32:
	case MTK_SIP_KERNEL_CCCI_GET_INFO_AARCH64:
		rc = ccci_md_dbgsys_config(x1, x2);
		break;
	case MTK_SIP_KERNEL_HW_FDE_KEY_AARCH32:
	case MTK_SIP_KERNEL_HW_FDE_KEY_AARCH64:
		rc = hwfde_set_key(x1, x2, x3);
		break;
	/* MPU */
	case MTK_SIP_KERNEL_EMIMPU_READ_AARCH32:
	case MTK_SIP_KERNEL_EMIMPU_READ_AARCH64:
		rc = sip_emi_mpu_read(x1);
		break;
	case MTK_SIP_KERNEL_EMIMPU_WRITE_AARCH32:
	case MTK_SIP_KERNEL_EMIMPU_WRITE_AARCH64:
		rc = sip_emi_mpu_write(x1, x2);
		break;
	case MTK_SIP_KERNEL_EMIMPU_CLEAR_AARCH32:
	case MTK_SIP_KERNEL_EMIMPU_CLEAR_AARCH64:
#if defined(MTK_ENABLE_MPU_HAL_SUPPORT)
		rc = sip_mpu_request_ree_perm_check(x1);
		if (rc)
			break;
#endif
		rc = sip_emi_mpu_clear_protection(x1);
		break;
	case MTK_SIP_KERNEL_EMIMPU_SET_AARCH32:
	case MTK_SIP_KERNEL_EMIMPU_SET_AARCH64:
#if defined(MTK_ENABLE_MPU_HAL_SUPPORT)
		rc = sip_mpu_request_ree_perm_check(
			(((uint32_t)x1) >> 24) & 0xFF);
		if (rc)
			break;
#endif
		rc = sip_emi_mpu_set_protection(x1, x2, x3);
		break;
#ifdef MTK_DEVMPU_SUPPORT
	case MTK_SIP_KERNEL_DEVMPU_VIO_GET_AARCH32:
	case MTK_SIP_KERNEL_DEVMPU_VIO_GET_AARCH64:
		rc = sip_devmpu_vio_get(x1, &x1, &x2);
		SMC_RET3(handle, rc, x1, x2);
		break;
#if (1 == TARGET_BUILD_VARIANT_ENG)
	case MTK_SIP_KERNEL_DEVMPU_PERM_GET_AARCH32:
	case MTK_SIP_KERNEL_DEVMPU_PERM_GET_AARCH64:
		rc = sip_devmpu_rw_perm_get(x1, &x1, &x2);
		SMC_RET3(handle, rc, x1, x2);
		break;
#endif
#endif
	case MTK_SIP_KERNEL_CRYPTO_HIE_CFG_REQUEST_AARCH32:
	case MTK_SIP_KERNEL_CRYPTO_HIE_CFG_REQUEST_AARCH64:
		rc = hie_cfg_request(x1, x2, x3);
		break;
	case MTK_SIP_KERNEL_CRYPTO_HIE_INIT_AARCH32:
	case MTK_SIP_KERNEL_CRYPTO_HIE_INIT_AARCH64:
		rc = hie_init();
		break;
	case MTK_SIP_KERNEL_SPM_DUMMY_READ_AARCH32:
	case MTK_SIP_KERNEL_SPM_DUMMY_READ_AARCH64:
		/* spm_dummy_read(x1, x2); */
		break;
	case MTK_SIP_KERNEL_SPM_ARGS_AARCH32:
	case MTK_SIP_KERNEL_SPM_ARGS_AARCH64:
		spm_args(x1, x2, x3);
		break;
	/* iLDO */
#if ILDO_ENABLE
#if (1 == TARGET_BUILD_VARIANT_ENG)
	case MTK_SIP_KERNEL_ILDO_REG_WRITE_AARCH32:
	case MTK_SIP_KERNEL_ILDO_REG_WRITE_AARCH64:
		rc = ildo_reg_write(x1, x2);
		break;
	case MTK_SIP_KERNEL_ILDO_REG_READ_AARCH32:
	case MTK_SIP_KERNEL_ILDO_REG_READ_AARCH64:
		rc = ildo_reg_read(x1);
		break;
#endif
	case MTK_SIP_KERNEL_ILDO_SET_ENDIS_AARCH32:
	case MTK_SIP_KERNEL_ILDO_SET_ENDIS_AARCH64:
		rc = ildo_set_endis(x1);
		break;
	case MTK_SIP_KERNEL_ILDO_GET_ENDIS_AARCH32:
	case MTK_SIP_KERNEL_ILDO_GET_ENDIS_AARCH64:
		rc = ildo_get_endis();
		break;
	case MTK_SIP_KERNEL_ILDO_SET_RET_VOLT_AARCH32:
	case MTK_SIP_KERNEL_ILDO_SET_RET_VOLT_AARCH64:
		rc = ildo_change_retention_volt(x1, x2);
		break;
#endif
	/* UDI */
	case MTK_SIP_KERNEL_UDI_JTAG_CLOCK_AARCH64:
		rc = UDIRead(x1, x2, x3);
		break;
	case MTK_SIP_KERNEL_UDI_BIT_CTRL_AARCH64:
		rc = UDIBitCtrl(x1, x2);
		break;
#if (1 == TARGET_BUILD_VARIANT_ENG)
	case MTK_SIP_KERNEL_UDI_WRITE_AARCH64:
		rc = UDIRegWrite(x1, x2);
		break;
	case MTK_SIP_KERNEL_UDI_READ_AARCH64:
		rc = UDIRegRead(x1);
		break;
#endif
	/* drcc */
	case MTK_SIP_KERNEL_DRCC_INIT_AARCH32:
	case MTK_SIP_KERNEL_DRCC_INIT_AARCH64:
		rc = drcc_debug_init();
		break;
	case MTK_SIP_KERNEL_DRCC_ENABLE_AARCH32:
	case MTK_SIP_KERNEL_DRCC_ENABLE_AARCH64:
		rc = drcc_enable(x1, x2);
		break;
	case MTK_SIP_KERNEL_DRCC_TRIG_AARCH32:
	case MTK_SIP_KERNEL_DRCC_TRIG_AARCH64:
		rc = drcc_trig(x1, x2, x3);
		break;
	case MTK_SIP_KERNEL_DRCC_COUNT_AARCH32:
	case MTK_SIP_KERNEL_DRCC_COUNT_AARCH64:
		rc = drcc_count(x1, x2, x3);
		break;
	case MTK_SIP_KERNEL_DRCC_MODE_AARCH32:
	case MTK_SIP_KERNEL_DRCC_MODE_AARCH64:
		rc = drcc_mode(x1, x2);
		break;
	case MTK_SIP_KERNEL_DRCC_CODE_AARCH32:
	case MTK_SIP_KERNEL_DRCC_CODE_AARCH64:
		rc = drcc_code(x1, x2);
		break;
	case MTK_SIP_KERNEL_DRCC_HWGATEPCT_AARCH32:
	case MTK_SIP_KERNEL_DRCC_HWGATEPCT_AARCH64:
		rc = drcc_hwgatepct(x1, x2);
		break;
	case MTK_SIP_KERNEL_DRCC_VREFFILT_AARCH32:
	case MTK_SIP_KERNEL_DRCC_VREFFILT_AARCH64:
		rc = drcc_vreffilt(x1, x2);
		break;
	case MTK_SIP_KERNEL_DRCC_AUTOCALIBDELAY_AARCH32:
	case MTK_SIP_KERNEL_DRCC_AUTOCALIBDELAY_AARCH64:
		rc = drcc_autocalibdelay(x1, x2);
		break;
	case MTK_SIP_KERNEL_DRCC_FORCETRIM_AARCH32:
	case MTK_SIP_KERNEL_DRCC_FORCETRIM_AARCH64:
		rc = drcc_forcetrim(x1, x2, x3);
		break;
	case MTK_SIP_KERNEL_DRCC_PROTECT_AARCH32:
	case MTK_SIP_KERNEL_DRCC_PROTECT_AARCH64:
		rc = drcc_protect(x1, x2);
		break;
	case MTK_SIP_KERNEL_DRCC_READ_AARCH32:
	case MTK_SIP_KERNEL_DRCC_READ_AARCH64:
		rc = drcc_reg_read(x1);
		break;
	case MTK_SIP_KERNEL_DFD_AARCH32:
	case MTK_SIP_KERNEL_DFD_AARCH64:
		rc = dfd_smc_dispatcher(x1, x2, x3, x4);
		break;
	case MTK_SIP_POWER_DOWN_CORE_AARCH32:
	case MTK_SIP_POWER_DOWN_CORE_AARCH64:
		mt_core_ops(x1, x2, 0);
		break;
#endif /* #ifndef ATF_BYPASS_DRAM */
	case MTK_SIP_POWER_FLOW_DEBUG_AARCH32:
	case MTK_SIP_POWER_FLOW_DEBUG_AARCH64:
		break;
	/*AMMS SPI*/
	case MTK_SIP_KERNEL_AMMS_GET_FREE_ADDR_AARCH32:
	case MTK_SIP_KERNEL_AMMS_GET_FREE_ADDR_AARCH64:
		rc = sip_kernel_amms_get_free_addr();
		SMC_RET1(handle, rc);
		break;
	case MTK_SIP_KERNEL_AMMS_GET_FREE_LENGTH_AARCH32:
	case MTK_SIP_KERNEL_AMMS_GET_FREE_LENGTH_AARCH64:
		rc = sip_kernel_amms_get_free_length();
		SMC_RET1(handle, rc);
		break;
	case MTK_SIP_KERNEL_AMMS_MD_POS_ADDR_AARCH32:
	case MTK_SIP_KERNEL_AMMS_MD_POS_ADDR_AARCH64:
		rc = sip_amms_set_md_pos_addr(x1);
		break;
	case MTK_SIP_KERNEL_AMMS_MD_POS_LENGTH_AARCH32:
	case MTK_SIP_KERNEL_AMMS_MD_POS_LENGTH_AARCH64:
		rc = sip_amms_set_md_pos_length(x1);
		break;
	case MTK_SIP_KERNEL_AMMS_GET_MD_POS_ADDR_AARCH32:
	case MTK_SIP_KERNEL_AMMS_GET_MD_POS_ADDR_AARCH64:
		rc = sip_amms_get_md_pos_addr();
		SMC_RET1(handle, rc);
		break;
	case MTK_SIP_KERNEL_AMMS_GET_MD_POS_LENGTH_AARCH32:
	case MTK_SIP_KERNEL_AMMS_GET_MD_POS_LENGTH_AARCH64:
		rc = sip_amms_get_md_pos_length();
		SMC_RET1(handle, rc);
		break;
	case MTK_SIP_KERNEL_AMMS_GET_PENDING_AARCH32:
	case MTK_SIP_KERNEL_AMMS_GET_PENDING_AARCH64:
		rc  = sip_amms_get_pending();
		SMC_RET1(handle, rc);
		break;
	case MTK_SIP_KERNEL_AMMS_ACK_PENDING_AARCH32:
	case MTK_SIP_KERNEL_AMMS_ACK_PENDING_AARCH64:
		sip_amms_clear_pending(x1);
		SMC_RET1(handle, rc);
		break;
	case MTK_SIP_KERNEL_AMMS_GET_SEQ_ID_AARCH32:
	case MTK_SIP_KERNEL_AMMS_GET_SEQ_ID_AARCH64:
		rc = sip_amms_get_seq_id();
		SMC_RET1(handle, rc);
		break;
	case MTK_SIP_KERNEL_AMMS_POS_STRESS_TOUCH_AARCH32:
	case MTK_SIP_KERNEL_AMMS_POS_STRESS_TOUCH_AARCH64:
		rc = sip_amms_pos_stress_touch();
		SMC_RET1(handle, rc);
		break;
	default:
		rc = SMC_UNK;
		ERROR("%s: unknown kernel SMC(0x%x)\n", __func__, smc_fid);
	}
	SMC_RET1(handle, rc);
}

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
	uint64_t rc = 0;
	uint32_t ns;
	uint32_t smc_ori;
	uint32_t smc_num;

	/* Get SMC Originator bit 14.15 */
	smc_ori = GET_SMC_ORI(smc_fid);
	/* Get SMC Number. Clean bit 14.15 */
	smc_num = GET_SMC_NUM(smc_fid);

	/* Determine which security state this SMC originated from */
	ns = is_caller_non_secure(flags);
	if (!ns) {
		/* SiP SMC service secure world's call */
		return mediatek_plat_sip_handler_secure(smc_num, x1, x2, x3, x4,
					cookie, handle, flags);
	} else {
		 if (is_from_bootloader(smc_ori)) {
			/* SiP SMC service bootloader's call */
			return mediatek_plat_sip_handler_bootloader(smc_num, x1, x2, x3, x4,
						cookie, handle, flags);
#ifdef MTK_ENABLE_GENIEZONE
		} else if (is_from_hyp(smc_ori)) {
			/* SiP SMC service hypervisor's call */
			return mediatek_plat_sip_handler_hypervisor(smc_num, x1, x2, x3, x4,
						cookie, handle, flags);
#endif
		} else if (is_from_kernel(smc_ori)) {
			/* SiP SMC service kernel's call */
			return mediatek_plat_sip_handler_kernel(smc_num, x1, x2, x3, x4,
						cookie, handle, flags);
		} else {
			rc = SMC_UNK;
			ERROR("%s: unknown stage SMC (0x%x)\n", __func__, smc_fid);
			SMC_RET1(handle, rc);
		}
	}
}

