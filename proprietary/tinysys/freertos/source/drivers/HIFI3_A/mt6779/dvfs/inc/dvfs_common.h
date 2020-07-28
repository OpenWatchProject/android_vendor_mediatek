/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

#ifndef _DVFS_COMMON_
#define _DVFS_COMMON_

#include <stdint.h>
#include <main.h>
#include <driver_api.h>
#include <mt_reg_base.h>
#include <mt_gpt.h>

#include <stdio.h>
#include <stdarg.h> /* va_list, va_start, va_arg, va_end */
#include <mt_printf.h>

#define pr_fmt(fmt) fmt

#define DVFS_LOG_ENABLE

#ifdef DVFS_LOG_ENABLE
#define DVFS_LOG_E(fmt, ...)     PRINTF_E("[E]" pr_fmt(fmt), ##__VA_ARGS__)
#define DVFS_LOG_W(fmt, ...)     PRINTF_W("[W]" pr_fmt(fmt), ##__VA_ARGS__)
#define DVFS_LOG_D(fmt, ...)     PRINTF_D("[D]" pr_fmt(fmt), ##__VA_ARGS__)
#define DVFS_LOG_V(fmt, ...)
#else
#define DVFS_LOG_E(fmt, ...)
#define DVFS_LOG_W(fmt, ...)
#define DVFS_LOG_D(fmt, ...)
#define DVFS_LOG_V(fmt, ...)
#endif

#define DBG_WAKEUP_SRC_UNMASK             (1 << 0)
#define DBG_WAKEUP_SRC_MASK               (1 << 1)
#define DBG_SPM_SRC_RELEASE               (1 << 2)
#define DBG_SPM_SRC_REQUEST               (1 << 3)
#define DBG_ENTER_WFI                     (1 << 4)
#define DBG_EXIT_WFI                      (1 << 5)
#define DBG_ADSP_SEL_MUX_BIT              (16)    //bit[19:16]

/*****************************************
 * Extenal Variables
 ****************************************/
extern uint32_t g_sleep_flag;
extern uint32_t g_active_adsp_mux_value;

extern uint32_t g_adsppll_status;
extern uint32_t g_adsppll_unlock;

extern uint32_t g_force_adsppll_on;
extern uint32_t g_sleep_tick_us;

void adsp_setup_adsppll(uint32_t adsp_enable) INTERNAL_FUNCTION;

#endif
