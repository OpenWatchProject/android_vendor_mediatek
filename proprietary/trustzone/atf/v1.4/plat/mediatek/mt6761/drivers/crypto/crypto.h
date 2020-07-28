/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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

#ifndef __CRYPTO_H
#define __CRYPTO_H

#include <mtk_plat_common.h>
#include <platform_def.h>
#include <console.h>

#include <sha2_export.h>
#ifdef MTK_MSDC_HW_FDE
#include <msdc.h>
#endif

#define CRYPTO_SHA256_READY
/* #define CRYPTO_HW_TZCC */

/*
 * Number of hashed HW FDE key slots
 *
 *
 * Slot 0 for eMMC or UFS
 *
 */
#define HWFDE_HASHED_KEY_CNT  (1)
#define HWFDE_HASHED_KEY_SIZE (8)

/* SHA256(hw_fde_key) */

extern unsigned int hwfde_hashed_key[HWFDE_HASHED_KEY_CNT][HWFDE_HASHED_KEY_SIZE];

extern struct atf_arg_t gteearg;

#ifdef CRYPTO_SHA256_READY
#ifdef CRYPTO_HW_TZCC /* Use TZCC */
#define sha256 TZCC_SHA256
extern unsigned int TZCC_SHA256(const unsigned char *in, unsigned int inlen, unsigned char *hash);
#endif
#endif

/*
 * Debugging facility
 */

#define CRYPTO_DBG                 0
#define CRYPTO_LOG_TAG             "[CRYPTO] "

#if CRYPTO_DBG
#define CRYPTO_LOG(...)            tf_printf(CRYPTO_LOG_TAG __VA_ARGS__)

/* allow atf log to be printed with kernel uart log */
#define CRYPTO_LOG_INIT()          console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE)

/* disable atf log printing with kernel uart log */
#define CRYPTO_LOG_UNINIT()        console_uninit()
#else
#define CRYPTO_LOG(...)
#define CRYPTO_LOG_INIT()
#define CRYPTO_LOG_UNINIT()
#endif

#define CRYPTO_OK                  (0)
#define CRYPTO_EINVAL              (22) /* Invalid argument */

/* Exported APIs */

int hwfde_set_key(unsigned int para1, unsigned int para2, unsigned int para3);

#endif /* __CRYPTO_H */


