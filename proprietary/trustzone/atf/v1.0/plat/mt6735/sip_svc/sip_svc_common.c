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
#include <assert.h>
#include <runtime_svc.h>
#include <debug.h>
#include <sip_svc.h>
#include <sip_error.h>
#include <platform.h>
#include <mmio.h>
#include <console.h> //set_uart_flag(), clear_uart_flag();
#include "plat_private.h"   //for atf_arg_t_ptr
#include "sip_private.h"
#include "mt_cpuxgpt.h"

#include <xlat_tables.h>
#include <emi_drv.h>
#include <log.h>
#include <rng.h>
#include "mtk_rot.h"
/*******************************************************************************
 * SIP top level handler for servicing SMCs.
 ******************************************************************************/

static struct kernel_info k_info;

static void save_kernel_info(uint64_t pc, uint64_t r0, uint64_t r1,
								uint64_t k32_64)
{
	/* Compatible to already MP platform/project                  *
	 * only 64 bits will re-enter ATF in previsous ARMv8 platform *
	 * ignore k32_64 flag                                         *
	 */
	k_info.k32_64 = LINUX_KERNEL_64;
	k_info.pc = pc;
	k_info.r0 = r0;
	k_info.r1 = r1;
}

uint64_t get_kernel_k32_64(void)
{
	return k_info.k32_64;
}

uint64_t get_kernel_info_pc(void)
{
	return k_info.pc;
}

uint64_t get_kernel_info_r0(void)
{
	return k_info.r0;
}

uint64_t get_kernel_info_r1(void)
{
	return k_info.r1;
}
uint64_t get_kernel_info_r2(void)
{
	return k_info.r2;
}

extern void bl31_prepare_kernel_entry(uint64_t k32_64);
extern void el3_exit(void);
extern int start_devapc(void);


/*******************************************************************************
 * SMC Call for Kernel MCUSYS register write
 ******************************************************************************/

static uint64_t mcusys_write_count;
static uint64_t sip_mcusys_write(unsigned int reg_addr, unsigned int reg_value)
{
	if ((reg_addr & 0x3) || (reg_addr < MCUCFG_BASE) || (reg_addr >= MCUCFG_BASE + 0x4000)) {
		ERROR("sip_mcusys_write: %x Invalid Range.\n", reg_addr);
		return SIP_SVC_E_INVALID_Range;
	}

	/* Perform range check */
	if ((reg_addr >= MP0_MISC_CONFIG0 && reg_addr <= MP0_MISC_CONFIG9) ||
	    (reg_addr >= MP1_MISC_CONFIG0 && reg_addr <= MP1_MISC_CONFIG9)) {
		return SIP_SVC_E_PERMISSION_DENY;
	}

	if (check_cpuxgpt_write_permission(reg_addr, reg_value) == 0) {
		/* Not allow to clean enable bit[0], Force to set bit[0] as 1 */
		reg_value |= 0x1;
	}

	mmio_write_32(reg_addr, reg_value);
	dsb();

	mcusys_write_count++;

	return SIP_SVC_E_SUCCESS;
}

/*******************************************************************************
 * SIP top level handler for servicing SMCs.
 ******************************************************************************/
uint64_t sip_smc_handler(uint32_t smc_fid,
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
	atf_arg_t_ptr teearg = (atf_arg_t_ptr)(uintptr_t)TEE_BOOT_INFO_ADDR;
	unsigned int atf_crash_log_addr;
	unsigned int atf_crash_log_size;
	unsigned int atf_crash_flag_addr;
	unsigned int *atf_crash_flag_addr_ptr;
	uint32_t rnd_val0 = 0, rnd_val1 = 0, rnd_val2 = 0, rnd_val3 = 0;

	/* Determine which security state this SMC originated from */
	ns = is_caller_non_secure(flags);

	switch (smc_fid) {
	case MTK_SIP_TBASE_HWUID_AARCH32: {
		if (ns)
			SMC_RET1(handle, SMC_UNK);
		SMC_RET4(handle, teearg->hwuid[0], teearg->hwuid[1],
					teearg->hwuid[2], teearg->hwuid[3]);
		break;
	}
	case MTK_SIP_LK_ROOT_OF_TRUST_AARCH32:
	case MTK_SIP_LK_ROOT_OF_TRUST_AARCH64:
		/* only allow from LK which is non-secure */
		if (!ns)
			SMC_RET1(handle, SMC_UNK);
		if (!rot_write_enable)
			SMC_RET1(handle, SMC_UNK);
		rc = sip_save_root_of_trust_info(x1, x2, x3, x4);
		break;
	case MTK_SIP_GET_ROOT_OF_TRUST_AARCH32:
	case MTK_SIP_GET_ROOT_OF_TRUST_AARCH64: {
		uint32_t arg0, arg1, arg2, arg3;
		/* only allow from TEE which is secure */
		if (ns)
			SMC_RET1(handle, SMC_UNK);
		if (rot_write_enable)
			SMC_RET1(handle, SMC_UNK);
		rc = sip_get_root_of_trust_info(&arg0, &arg1, &arg2, &arg3);
		if (rc == SIP_SVC_E_SUCCESS)
			SMC_RET4(handle, arg0, arg1, arg2, arg3);
		break;
	}
	case MTK_SIP_KERNEL_MCUSYS_WRITE_AARCH32:
	case MTK_SIP_KERNEL_MCUSYS_WRITE_AARCH64:
		rc = sip_mcusys_write(x1, x2);
		break;
	case MTK_SIP_KERNEL_MCUSYS_ACCESS_COUNT_AARCH32:
	case MTK_SIP_KERNEL_MCUSYS_ACCESS_COUNT_AARCH64:
		rc = mcusys_write_count;
		break;
	case MTK_SIP_KERNEL_BOOT_AARCH32:
		set_uart_flag();
		printf("save kernel info\n");
		save_kernel_info(x1, x2, x3, x4);
		bl31_prepare_kernel_entry(x4);
		printf("EMI MPUS=0x%x; MPUT=0x%x\n", mmio_read_32(0x102031F0), mmio_read_32(0x102031F8));
		printf("el3_exit\n");
		clear_uart_flag();
		SMC_RET0(handle);
		break;
	case MTK_SIP_KERNEL_DAPC_INIT_AARCH32:
	case MTK_SIP_KERNEL_DAPC_INIT_AARCH64:
		/* Setup DAPC in ATF */
		start_devapc();
		break;
	case MTK_SIP_KERNEL_EMIMPU_WRITE_AARCH32:
	case MTK_SIP_KERNEL_EMIMPU_WRITE_AARCH64:
		rc = sip_emimpu_write(x1, x2);
		break;

	case MTK_SIP_KERNEL_EMIMPU_READ_AARCH32:
	case MTK_SIP_KERNEL_EMIMPU_READ_AARCH64:
		rc = (uint64_t)sip_emimpu_read(x1);
		break;
#if DEBUG
	case MTK_SIP_KERNEL_GIC_DUMP_AARCH32:
	case MTK_SIP_KERNEL_GIC_DUMP_AARCH64:
		rc = mt_irq_dump_status(x1);
		break;
#endif
	case MTK_SIP_KERNEL_WDT_AARCH32:
	case MTK_SIP_KERNEL_WDT_AARCH64:
		wdt_kernel_cb_addr = x1;
		printf("MTK_SIP_KERNEL_WDT : 0x%lx\n", wdt_kernel_cb_addr);
		printf("teearg->atf_aee_debug_buf_start : 0x%x\n",
		       teearg->atf_aee_debug_buf_start);
		rc = teearg->atf_aee_debug_buf_start;
		break;
	case MTK_SIP_KERNEL_MSG_AARCH32:
	case MTK_SIP_KERNEL_MSG_AARCH64:
		rc = SIP_SVC_E_SUCCESS;
		break;
	case MTK_SIP_LK_GET_RND_AARCH32:
	case MTK_SIP_LK_GET_RND_AARCH64:
		rc = plat_get_rnd(&rnd_val0);
		SMC_RET2(handle, rc, rnd_val0);
		break;
	case MTK_SIP_KERNEL_GET_RND_AARCH32:
	case MTK_SIP_KERNEL_GET_RND_AARCH64:
		plat_get_rnd_4(x1, &rnd_val0, &rnd_val1, &rnd_val2, &rnd_val3);
		SMC_RET4(handle, rnd_val0, rnd_val1, rnd_val2, rnd_val3);
		break;
	case MTK_SIP_KERNEL_TIME_SYNC_AARCH32:
	case MTK_SIP_KERNEL_TIME_SYNC_AARCH64:
		/* in arch32, high 32 bits is stored in x2 and this would be 0 in arch64 */
		atf_sched_clock_init(atf_sched_clock() - (x1 + (x2 << 32)), 0);
		MT_LOG_KTIME_SET();
		rc = SIP_SVC_E_SUCCESS;
		break;
	case MTK_SIP_LK_DUMP_ATF_LOG_INFO_AARCH32:
	case MTK_SIP_LK_DUMP_ATF_LOG_INFO_AARCH64:
		set_uart_flag();
		atf_crash_log_addr = mt_log_get_crash_log_addr();
		atf_crash_log_size = mt_log_get_crash_log_size();
		atf_crash_flag_addr_ptr = mt_log_get_crash_flag_addr();
		atf_crash_flag_addr = (uint64_t)atf_crash_flag_addr_ptr & 0xffffffff;

		printf("LK Dump\n");
		printf("buf addr:0x%x ,", atf_crash_log_addr);
		printf("buf size:%u ,", atf_crash_log_size);
		printf("flag addr:0x%x ,", atf_crash_flag_addr);
		printf("flag:0x%x\n", *atf_crash_flag_addr_ptr);
		clear_uart_flag();
		SMC_RET3(handle, atf_crash_log_addr, atf_crash_log_size, atf_crash_flag_addr);
		break;
	default:
		rc = SMC_UNK;
		WARN("Unimplemented SIP Call: 0x%x\n", smc_fid);
	}

	SMC_RET1(handle, rc);
}

