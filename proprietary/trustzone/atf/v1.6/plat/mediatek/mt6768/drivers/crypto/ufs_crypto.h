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

#ifndef __UFS_CRYPTO_H
#define __UFS_CRYPTO_H

#include <plat_private.h>

/*
 * UFS crypto IP CG
 * PERICFG_MODULE_SW_CG2
 */
#define UFS_AES_CG_SET          (INFRACFG_AO_BASE + 0xA4)
#define UFS_AES_CG_CLR          (INFRACFG_AO_BASE + 0xA8)
#define UFS_AES_CG_STA          (INFRACFG_AO_BASE + 0xAC)
#define UFS_AES_CG_BIT          (29)
#define UFS_AES_CG              (1 << UFS_AES_CG_BIT)

/* UFS crypto register in UFSHCI */
enum {
	UFS_REG_CONTROLLER_ENABLE   = 0x34,
	UFS_REG_CRYPTO_CAPABILITY   = 0x100,
	UFS_REG_CRYPTO_CAPABILITY_X = 0x104,
	UFS_REG_SECURE_CAP	    = 0x130,
	UFS_REG_SECURE_MAP	    = 0x134
};

enum {
	UFS_CRYPTO_ALGO_AES_XTS             = 0,
	UFS_CRYPTO_ALGO_BITLOCKER_AES_CBC   = 1,
	UFS_CRYPTO_ALGO_AES_ECB             = 2,
	UFS_CRYPTO_ALGO_ESSIV_AES_CBC       = 3,
};

enum {
	UFS_CRYPTO_DATA_UNIT_SIZE_4KB       = 3,
};

enum {
	UFS_CRYPTO_MODE_UNDEFINED = 0,
	UFS_CRYPTO_MODE_HW_FDE,
	UFS_CRYPTO_MODE_HW_FBE,
};

enum ufs_crypto_power {
	UFS_CRYPTO_POWER_OFF = 0,
	UFS_CRYPTO_POWER_ON,
};

union ufs_cpt_cap {
	uint32_t cap_raw;
	struct {
		uint8_t cap_cnt;
		uint8_t cfg_cnt;
		uint8_t resv;
		uint8_t cfg_ptr;
	} cap;
};

union ufs_cpt_capx {
	uint32_t capx_raw;
	struct {
		uint8_t alg_id;
		uint8_t du_size;
		uint8_t key_size;
		uint8_t resv;
	} capx;
};

union ufs_cap_cfg {
	uint32_t cfgx_raw[32];
	struct {
		uint32_t key[16];
		uint8_t du_size;
		uint8_t cap_id;
		uint16_t resv0  : 15;
		uint16_t cfg_en : 1;
		uint8_t mu1ti_host;
		uint8_t resv1;
		uint16_t vsb;
		uint32_t resv2[14];
	} cfgx;
};

/* UFS crypto control flags */
#define UFS_AES_CTL_FDE_INIT    (1 << 0)
#define UFS_AES_CTL_SUSPEND     (1 << 1)
#define UFS_AES_CTL_RESUME      (1 << 2)
#define UFS_AES_CTL_SET_KEY     (1 << 3)
#define UFS_AES_CTL_FBE_INIT    (1 << 8)

/* UFS generic control flags */
#define UFS_MPHY_VA09_CG_CTL    (1 << 0)

int32_t  ufs_crypto_ctl(uint32_t ctl, uint32_t param2, uint32_t param3);
void     ufs_crypto_hie_init(void);
uint32_t ufs_crypto_hie_init_request(uint32_t hie_para);
void     ufs_crypto_hie_cfg_request(unsigned int para1, unsigned int para2, unsigned int para3);
void     ufs_crypto_hie_program_key(uint32_t addr, uint32_t hie_para, uint32_t key1, uint32_t key2, uint32_t key3);
int32_t  ufs_generic_ctl(uint32_t ctl, uint32_t param2, uint32_t param3);

#endif /* __UFS_CRYPTO_H */

