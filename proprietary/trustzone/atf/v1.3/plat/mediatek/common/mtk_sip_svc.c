/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
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
#include <assert.h>
#include <console.h>
#include <context_mgmt.h>
#include <debug.h>
#include <fiq_smp_call.h>
#include <mmio.h>
#include <mtk_aee_debug.h>
#include <mtk_plat_common.h>
#include <mtk_sip_svc.h>
#include <platform.h>
#include <runtime_svc.h>
#include <uuid.h>
#include "mtk_rot.h"
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

/* 32 bits == MODE_RW_32, 64 bits == MODE_RW_64 */
uint64_t lk_type;
uint64_t kernel_type;

/*
 * This function handles Mediatek defined SiP Calls */
uint64_t mediatek_sip_handler(uint32_t smc_fid,
			uint64_t x1,
			uint64_t x2,
			uint64_t x3,
			uint64_t x4,
			void *cookie,
			void *handle,
			uint64_t flags)
{
	uint32_t ns;
	uint32_t cpu;
	uint64_t mpidr;
	uint64_t spsr = 0;
	uint64_t el = 0;
	cpu_context_t *ns_cpu_context;

	mpidr = read_mpidr();
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
		switch(smc_fid) {
			case MTK_SIP_KERNEL_AEE_DUMP_AARCH32:
			case MTK_SIP_KERNEL_AEE_DUMP_AARCH64:
				if (0 == x1)
				{
					wdt_notify_other_cores(aee_wdt_dump, 0, 0);
					aee_wdt_dump(cookie);
				}
				else if (1 == x1) {
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
				uint64_t rc;

				rc = sip_tee_mpu_request(x1, x2, x3);
				SMC_RET1(handle, rc);
				break;
			}
#endif
		}
	} else {	/* if (!ns) */
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
				case MTK_SIP_HYP_MPU_PERM_SET_AARCH64: {
					uint64_t rc;

					rc = sip_gz_mpu_request(x1, x2, x3);
					SMC_RET1(handle, rc);
					break;
				}
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
				}
			} else if (((el != MODE_EL2) && (GET_RW(spsr) == MODE_RW_32) &&
						((spsr & MODE32_MASK) == MODE32_svc)) ||
						((el == MODE_EL2) && (lk_type == MODE_RW_32))) {
				switch (smc_fid) {
#ifdef MTK_ENABLE_MPU_HAL_SUPPORT
#ifdef MTK_ENABLE_GENIEZONE
				case MTK_SIP_HYP_MPU_PERM_SET_AARCH32:
				case MTK_SIP_HYP_MPU_PERM_SET_AARCH64: {
					uint64_t rc;

					rc = sip_gz_mpu_request(x1, x2, x3);
					SMC_RET1(handle, rc);
					break;
				}
#endif
#endif
				case MTK_SIP_LK_ROOT_OF_TRUST_AARCH32: {
					uint64_t rc;

					if (!rot_write_enable)
						SMC_RET1(handle, SMC_UNK);
					rc = sip_save_root_of_trust_info(x1, x2, x3, x4);
					SMC_RET1(handle, rc);
					break;
				}

#if MTK_SIP_SET_AUTHORIZED_SECURE_REG_ENABLE
				case MTK_SIP_SET_AUTHORIZED_SECURE_REG: {
					/* only use ret here */
					uint64_t ret;

					ret = mt_sip_set_authorized_sreg((uint32_t)x1,
						(uint32_t)x2);
					SMC_RET1(handle, ret);
				}
#endif
#if MTK_SIP_KERNEL_BOOT_ENABLE
				case MTK_SIP_KERNEL_BOOT_AARCH32:
					boot_to_kernel(x1, x2, x3, x4);
					SMC_RET0(handle);
				}
#endif
			}
		} else if (mtk_lk_stage == 0) {
			if (((el != MODE_EL2) && (GET_RW(spsr) == MODE_RW_64))
			 || ((el == MODE_EL2) && (kernel_type == MODE_RW_64))) {
				/* INFO("originated from kernel 64\n"); */
				switch (smc_fid) {
#ifdef MTK_ENABLE_MPU_HAL_SUPPORT
#ifdef MTK_ENABLE_GENIEZONE
				case MTK_SIP_HYP_MPU_PERM_SET_AARCH32:
				case MTK_SIP_HYP_MPU_PERM_SET_AARCH64: {
					uint64_t rc;

					rc = sip_gz_mpu_request(x1, x2, x3);
					SMC_RET1(handle, rc);
					break;
				}
#endif
#endif
				}
			} else if (((el != MODE_EL2) && (GET_RW(spsr) == MODE_RW_32) &&
						((spsr & MODE32_MASK) == MODE32_svc)) ||
						((el == MODE_EL2) && (kernel_type == MODE_RW_32))) {
				/* INFO("originated from kernel 32\n"); */
				switch (smc_fid) {
#if MTK_SIP_SET_AUTHORIZED_SECURE_REG_ENABLE
				case MTK_SIP_SET_AUTHORIZED_SECURE_REG: {
					/* only use ret here */
					uint64_t ret;

					ret = mt_sip_set_authorized_sreg((uint32_t)x1,
						(uint32_t)x2);
					SMC_RET1(handle, ret);
					}
#endif
				}
			}
		}
	} /* if (!ns) */
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
