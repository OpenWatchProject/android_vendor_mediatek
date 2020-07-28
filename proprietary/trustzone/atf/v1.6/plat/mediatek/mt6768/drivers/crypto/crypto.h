/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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

 /*
  * crypto IP MUX
  * PERI_FDI_AES_SI_CTRL
  */
#define PERI_FDI_AES_SI_CTRL          (PERICFG_BASE + 0x448)
#define PERI_FDI_AES_SI_CTRL_MSDC0_FDE_EN         (4)

/*
 * Number of hashed HW FDE key slots
 *
 * In MT6799,
 * Slot 0 for eMMC or UFS
 * Slot 1 for SD card
 */
#define HWFDE_HASHED_KEY_CNT (2)
#define HWFDE_HASHED_KEY_SIZE (8)

/* SHA256(hw_fde_key) */

extern unsigned int hwfde_hashed_key[HWFDE_HASHED_KEY_CNT][HWFDE_HASHED_KEY_SIZE];

/*
 * HIE related definitions.
 * Block crypt flags
 */
enum bc_flags_bits {
	__BC_CRYPT,        /* marks the request needs crypt */
	__BC_IV_PAGE_IDX,  /* use page index as iv. */
	__BC_IV_CTX,       /* use the iv saved in crypt context */
	__BC_AES_128_XTS,  /* crypt algorithms */
	__BC_AES_192_XTS,
	__BC_AES_256_XTS,
	__BC_AES_128_CBC,
	__BC_AES_256_CBC,
	__BC_AES_128_ECB,
	__BC_AES_256_ECB,
};

enum {
	HIE_CFG_KEY_STAGE_NOT_START    = 0,
	HIE_CFG_KEY_STAGE_TRANSFERRING = 1,
};

#define BC_CRYPT		(1UL << __BC_CRYPT)
#define BC_AES_128_XTS	(1UL << __BC_AES_128_XTS)
#define BC_AES_192_XTS	(1UL << __BC_AES_192_XTS)
#define BC_AES_256_XTS	(1UL << __BC_AES_256_XTS)
#define BC_AES_128_CBC	(1UL << __BC_AES_128_CBC)
#define BC_AES_256_CBC	(1UL << __BC_AES_256_CBC)
#define BC_AES_128_ECB	(1UL << __BC_AES_128_ECB)
#define BC_AES_256_ECB	(1UL << __BC_AES_256_ECB)

#define UFS_HIE_PARAM_OFS_CFG_ID         (24)
#define UFS_HIE_PARAM_OFS_MODE           (16)
#define UFS_HIE_PARAM_OFS_KEY_TOTAL_BYTE (8)
#define UFS_HIE_PARAM_OFS_KEY_START_BYTE (0)

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
#define CRYPTO_LOG_FORCED(...)     tf_printf(CRYPTO_LOG_TAG __VA_ARGS__)
#define CRYPTO_LOG_INIT()
#define CRYPTO_LOG_UNINIT()
#endif

#define CRYPTO_OK                  (0)
#define CRYPTO_EINVAL              (22) /* Invalid argument */

/* Exported APIs */

int hie_cfg_request(unsigned int para1, unsigned int para2, unsigned int para3);
int hie_init(void);
int hwfde_set_key(unsigned int para1, unsigned int para2, unsigned int para3);

#endif /* __CRYPTO_H */

