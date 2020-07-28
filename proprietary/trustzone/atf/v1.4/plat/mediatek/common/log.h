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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

/******************************************************************************
*
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2001
*
*******************************************************************************/

#ifndef ATF_LOG_DRV_H
#define ATF_LOG_DRV_H
#ifndef __ASSEMBLY__

#include <spinlock.h>
#include <stdint.h>
#include <platform_def.h>
#endif
#define ATF_LOG_CTRL_BUF_SIZE 512

#define ATF_CRASH_MAGIC_NO	0xdead1abf
#define ATF_LAST_LOG_MAGIC_NO	0x41544641
#define ATF_BAD_PTR		0xffffffff
#define ATF_DUMP_DONE_MAGIC_NO	0xd07ed07e

#define LOG_USE_SPIN_LOCK	0
#define MT_LOG_KTIME_FLAG_SHIFT 31

/*
  ___________________________
 |                           |
 | ATF crash reserved buffer |
 |___________________________|

  Total reserved buffer size = ATF_CRASH_LAST_LOG_SIZE + ATF_EXCEPT_BUF_SIZE

 +--------------------------------+--------------------------------------------------------+
 |    ATF_CRASH_LAST_LOG_SIZE     |    ATF_EXCEPT_BUF_SIZE_PER_CPU * PLATFORM_CORE_COUNT   |
 +--------------------------------+--------------------------------------------------------+
 |    Last ATF log for crash      | CPU-0 | CPU-1| CPU-2 | CPU-3 |...PLATFORM_CORE_COUNT-1 |
 +--------------------------------+--------------------------------------------------------+
*/
#ifdef MTK_ATF_LOG_BUF_SLIM
#define ATF_CRASH_LAST_LOG_SIZE (64*1024)
#else
#define ATF_CRASH_LAST_LOG_SIZE (128*1024)
#endif
#define ATF_EXCEPT_BUF_SIZE_PER_CPU (4*1024)
#define ATF_EXCEPT_BUF_SIZE (ATF_EXCEPT_BUF_SIZE_PER_CPU * PLATFORM_CORE_COUNT)
#ifndef __ASSEMBLY__

extern unsigned int mt_log_ktime_sync;

#define MT_LOG_KTIME_CLEAR() mt_log_ktime_sync = 0;
#define MT_LOG_KTIME_SET() mt_log_ktime_sync = 1;
#define MT_LOG_KTIME mt_log_ktime_sync

typedef union atf_log_ctrl {
    struct {
	uint64_t atf_log_addr;
	uint64_t atf_log_size;
	uint32_t atf_write_offset;
	uint32_t atf_read_offset;

	/* must keep value after reboot */
	uint64_t atf_crash_log_addr;
	uint64_t atf_crash_log_size;
	uint32_t atf_total_write_count;
	uint32_t atf_crash_flag;

	/* for FIQ/IRQ footprint, print in crash log*/
	uint64_t fiq_irq_enter_timestamp[PLATFORM_CORE_COUNT];
	uint64_t fiq_irq_quit_timestamp[PLATFORM_CORE_COUNT];
	uint32_t enter_atf_fiq_irq_num[PLATFORM_CORE_COUNT];
    } info;
    unsigned char data[ATF_LOG_CTRL_BUF_SIZE];
} atf_log_ctrl_t;

void mt_log_setup(uint64_t start, unsigned int size, unsigned int aee_buf_size, unsigned int is_abnormal_boot);
int mt_log_lock_acquire(void);
int mt_log_write(unsigned char c);
int mt_log_lock_release(void);
void bl31_log_service_register(int (*lock_get)(),
	int (*log_putc)(unsigned char),
	int (*lock_release)());
void print_log_timestamp(void);
void print_overwritten_msg(void);

int (*log_write)(unsigned char);

void mt_log_suspend_flush(void);
void mt_log_secure_os_print(int c);

uint64_t mt_log_get_crash_log_addr(void);
uint64_t mt_log_get_crash_log_size(void);
unsigned int *mt_log_get_crash_flag_addr(void);

void atf_enter_footprint(unsigned int irq);
void atf_exit_footprint(void);
unsigned int get_is_log_overwritten(void);
void clr_is_log_overwritten(void);

#ifdef MTK_ATF_RAM_DUMP
extern uint64_t	atf_ram_dump_base;
extern uint64_t	atf_ram_dump_size;
#endif
#endif
#endif
