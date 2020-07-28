/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
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

#ifndef __MSDC_PLATFORM_H
#define __MSDC_PLATFORM_H

#ifdef MTK_MSDC_HW_FDE
#include <plat_private.h>


 /*=======================================================================*/
 /* MSDC Related Registers */
 /*=======================================================================*/

#define	EMMC52_AES_EN           (MSDC0_BASE + 0x600)
#define EMMC52_AES_CFG_GP0      (MSDC0_BASE + 0x604)
#define EMMC52_AES_IV0_GP0      (MSDC0_BASE + 0x610)
#define EMMC52_AES_IV1_GP0      (MSDC0_BASE + 0x614)
#define EMMC52_AES_IV2_GP0      (MSDC0_BASE + 0x618)
#define EMMC52_AES_IV3_GP0      (MSDC0_BASE + 0x61C)
#define EMMC52_AES_CTR0_GP0     (MSDC0_BASE + 0x620)
#define EMMC52_AES_CTR1_GP0     (MSDC0_BASE + 0x624)
#define EMMC52_AES_CTR2_GP0     (MSDC0_BASE + 0x628)
#define EMMC52_AES_CTR3_GP0     (MSDC0_BASE + 0x62C)
#define EMMC52_AES_KEY0_GP0     (MSDC0_BASE + 0x630)
#define EMMC52_AES_KEY1_GP0     (MSDC0_BASE + 0x634)
#define EMMC52_AES_KEY2_GP0     (MSDC0_BASE + 0x638)
#define EMMC52_AES_KEY3_GP0     (MSDC0_BASE + 0x63C)
#define EMMC52_AES_KEY4_GP0     (MSDC0_BASE + 0x640)
#define EMMC52_AES_KEY5_GP0     (MSDC0_BASE + 0x644)
#define EMMC52_AES_KEY6_GP0     (MSDC0_BASE + 0x648)
#define EMMC52_AES_KEY7_GP0     (MSDC0_BASE + 0x64C)
#define EMMC52_AES_TKEY0_GP0    (MSDC0_BASE + 0x650)
#define EMMC52_AES_TKEY1_GP0    (MSDC0_BASE + 0x654)
#define EMMC52_AES_TKEY2_GP0    (MSDC0_BASE + 0x658)
#define EMMC52_AES_TKEY3_GP0    (MSDC0_BASE + 0x65C)
#define EMMC52_AES_TKEY4_GP0    (MSDC0_BASE + 0x660)
#define EMMC52_AES_TKEY5_GP0    (MSDC0_BASE + 0x664)
#define EMMC52_AES_TKEY6_GP0    (MSDC0_BASE + 0x668)
#define EMMC52_AES_TKEY7_GP0    (MSDC0_BASE + 0x66c)

#define EMMC52_AES_SWST         (MSDC0_BASE + 0x670)

#define EMMC52_AES_CFG_GP1      (MSDC0_BASE + 0x674)
#define EMMC52_AES_IV0_GP1      (MSDC0_BASE + 0x680)
#define EMMC52_AES_IV1_GP1      (MSDC0_BASE + 0x684)
#define EMMC52_AES_IV2_GP1      (MSDC0_BASE + 0x688)
#define EMMC52_AES_IV3_GP1      (MSDC0_BASE + 0x68C)
#define EMMC52_AES_CTR0_GP1     (MSDC0_BASE + 0x690)
#define EMMC52_AES_CTR1_GP1     (MSDC0_BASE + 0x694)
#define EMMC52_AES_CTR2_GP1     (MSDC0_BASE + 0x698)
#define EMMC52_AES_CTR3_GP1     (MSDC0_BASE + 0x69C)
#define EMMC52_AES_KEY0_GP1     (MSDC0_BASE + 0x6A0)
#define EMMC52_AES_KEY1_GP1     (MSDC0_BASE + 0x6A4)
#define EMMC52_AES_KEY2_GP1     (MSDC0_BASE + 0x6A8)
#define EMMC52_AES_KEY3_GP1     (MSDC0_BASE + 0x6AC)
#define EMMC52_AES_KEY4_GP1     (MSDC0_BASE + 0x6B0)
#define EMMC52_AES_KEY5_GP1     (MSDC0_BASE + 0x6B4)
#define EMMC52_AES_KEY6_GP1     (MSDC0_BASE + 0x6B8)
#define EMMC52_AES_KEY7_GP1     (MSDC0_BASE + 0x6BC)
#define EMMC52_AES_TKEY0_GP1    (MSDC0_BASE + 0x6C0)
#define EMMC52_AES_TKEY1_GP1    (MSDC0_BASE + 0x6C4)
#define EMMC52_AES_TKEY2_GP1    (MSDC0_BASE + 0x6C8)
#define EMMC52_AES_TKEY3_GP1    (MSDC0_BASE + 0x6CC)
#define EMMC52_AES_TKEY4_GP1    (MSDC0_BASE + 0x6D0)
#define EMMC52_AES_TKEY5_GP1    (MSDC0_BASE + 0x6D4)
#define EMMC52_AES_TKEY6_GP1    (MSDC0_BASE + 0x6D8)
#define EMMC52_AES_TKEY7_GP1    (MSDC0_BASE + 0x6DC)

/* EMMC52_AES_EN mask*/
#define EMMC52_AES_ON               (0x1 << 0)
#define EMMC52_AES_SWITCH_VALID0    (0x1 << 1)
#define EMMC52_AES_SWITCH_VALID1    (0x1 << 2)
#define EMMC52_AES_CLK_DIV_SEL      (0x7 << 4)

/* EMMC52_AES_CFG_GP0 mask*/
#define EMMC52_AES_MODE_0           (0x1F << 0)
#define EMMC52_AES_KEY_SIZE_0       (0x3 << 8)
#define EMMC52_AES_DECRYPT_0        (0x1 << 12)
#define EMMC52_AES_DATA_UNIT_SIZE_0 (0x1FFF << 16)

/* EMMC52_AES_CFG_GP1 mask*/
#define EMMC52_AES_MODE_1           (0x1F << 0)
#define EMMC52_AES_KEY_SIZE_1       (0x3 << 8)
#define EMMC52_AES_DECRYPT_1        (0x1 << 12)
#define EMMC52_AES_DATA_UNIT_SIZE_1 (0x1FFF << 16)

/* EMMC52_AES_SWST */
#define EMMC52_AES_SWITCH_START_ENC (0x1 << 0)
#define EMMC52_AES_SWITCH_START_DEC (0x1 << 1)
#define EMMC52_AES_BYPASS           (0x1 << 2)

/* map from AES Spec */
enum {
	MSDC_CRYPTO_XTS_AES         = 4,
	MSDC_CRYPTO_AES_CBC_ESSIV   = 9,
	MSDC_CRYPTO_BITLOCKER       = 17,
	MSDC_CRYPTO_AES_ECB         = 0,
	MSDC_CRYPTO_AES_CBC         = 1,
	MSDC_CRYPTO_AES_CTR         = 2,
	MSDC_CRYPTO_AES_CBC_MAC     = 3,
} aes_mode;

enum {
	BIT_128 = 0,
	BIT_192 = 1,
	BIT_256 = 2,
	BIT_0 = 4,
};

struct msdc_crypt_config {
	uint8_t aes_mode;
	uint32_t data_unit_size;
	uint8_t key_bit;
	uint32_t key[8];
	uint8_t tkey_bit;
	uint32_t tkey[8];
	uint8_t iv_bit;
	uint32_t iv[4];
};

/* CQHCI crypto start */
#define CQHCI_BASE              (MSDC0_BASE + 0x800)

#define CQHCI_CQCFG             (CQHCI_BASE + 0x8)
#define CQHCI_CRCAP             (CQHCI_BASE + 0x100)

#define CQ_ENABLE        0x1
#define CQ_CRYPTO_ENABLE 0x2

#define PERI_MSDC0_MHOST_TBL    (PERI_BASE + 0x440)
#define PERI_MSDC0_MHOST_TBL_1  (PERI_BASE + 0x450)

union cqhci_cpt_cap {
	uint32_t cap_raw;
	struct {
		uint8_t cap_cnt;
		uint8_t cfg_cnt;
		uint8_t resv;
		uint8_t cfg_ptr;
	} cap;
};

union cqhci_cpt_capx {
	uint32_t capx_raw;
	struct {
		uint8_t alg_id;
		uint8_t du_size;
		uint8_t key_size;
		uint8_t resv;
	} capx;
};

union cqhci_cap_cfg {
	uint32_t cfgx_raw[32];
	struct {
		uint32_t key[16];
		uint8_t du_size;
		uint8_t cap_id;
		uint16_t resv0:15;
		uint16_t cfg_en:1;
		uint8_t mu1ti_host;
		uint8_t resv1;
		uint16_t vsb;
		uint32_t resv2[14];
	} cfgx;
};

struct cqhci_crypto {
	uint32_t cfg_id;
	uint32_t cap_id;
	union cqhci_cpt_cap cap;
	union cqhci_cpt_capx capx;
	union cqhci_cap_cfg cfg;
};

/* capability id */
enum {
	CQHCI_CRYPTO_AES_XTS_128 = 0,
	CQHCI_CRYPTO_AES_XTS_256 = 1,
	CQHCI_CRYPTO_BITLOCKER_AES_CBC_128 = 2,
	CQHCI_CRYPTO_BITLOCKER_AES_CBC_256 = 3,
	CQHCI_CRYPTO_AES_ECB_128 = 4,
	CQHCI_CRYPTO_AES_ECB_192 = 5,
	CQHCI_CRYPTO_AES_ECB_256 = 6,
	CQHCI_CRYPTO_ESSIV_AES_CBC_128 = 7,
	CQHCI_CRYPTO_ESSIV_AES_CBC_192 = 8,
	CQHCI_CRYPTO_ESSIV_AES_CBC_256 = 9,
};

/* data unit size */
enum {
	CQHCI_CRYPTO_DU_SIZE_512B = 0,
};

/* CQHCI crypto end */

/* MSDC crypto control flags */
#define MSDC_AES_CTL_INIT           (1 << 0)
#define MSDC_AES_CTL_SUSPEND        (1 << 1)
#define MSDC_AES_CTL_RESUME         (1 << 2)
#define MSDC_AES_CTL_SET_KEY        (1 << 3)
#define MSDC_AES_CTL_CQHCI_INIT     (1 << 4)
#define MSDC_AES_CTL_CQHCI_SET_KEY  (1 << 5)

int32_t msdc_crypto_ctl(uint32_t ctl, uint32_t param2, uint32_t param3);

#endif /* End of #ifdef MTK_MSDC_HW_FDE */

#endif /* __MSDC_PLATFORM_H */
