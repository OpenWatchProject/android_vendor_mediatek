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

#include <arch.h>
#include <debug.h>
#include <mmio.h>
#include <platform_def.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <mtk_sip_svc.h>
#include <ufs_crypto.h>
#include <crypto.h>
#include <delay_timer.h>

#ifdef MTK_UFS_SUPPORT

/*
 * UFS DVT option
 * Enable this option for UFS DVT.
 */

/* #define MTK_UFS_DVT */

/*
 * In early development stage, key derivation meshanism
 * may not be ready yet. We could use pre-built key instead
 * temporarily.
 */

#ifdef MTK_UFS_DVT
#define MTK_UFS_CRYPTO_KEY_PREBUILD
#else
/* #define MTK_UFS_CRYPTO_KEY_PREBUILD */
#endif

/* UFS HW FDE specific algorithm: AES-CBC-ESSIV-128-bits */
static const uint32_t ufs_hw_fde_cap_id = 7;

/* UFS HW FDE specific crypto configuration id */
static const uint32_t ufs_hw_fde_cfg_id;

/* To save atf ROM size, assign key size by a small global */
static const uint32_t ufs_hw_fde_key_size_bits = 128;

static int ufs_crypto_mode = UFS_CRYPTO_MODE_UNDEFINED;
static int ufs_crypto_en = UFS_CRYPTO_POWER_OFF;

#ifdef MTK_UFS_CRYPTO_KEY_PREBUILD
/* prebuilt key with SHA256 applied */
uint32_t ufs_crypto_key_prebuilt[3][16] = {
	{ /* for 128 bit key */
		0xa5a5a5a5, 0x5a5a5a5a, 0x55aa55aa, 0xaa55aa55,
		0x0,         0x0,    0x0,        0x0,
		0x9309da32, 0x87a0c7fc, 0x1b58a359, 0x14fc38ea,
		0x68c7b319, 0x0dcb6355, 0xa0e565e9, 0x8765ccb5,
	},
	{ /* for 192 bit key */
		0xa5a5a5a5, 0x5a5a5a5a, 0x55aa55aa, 0xaa55aa55,
		0xa5a5a5a5, 0x5a5a5a5a, 0x0,        0x0,
		0xad168c0f, 0x502b6ae1, 0x30199ad0, 0x69488013,
		0x42659ad8, 0xf94f95ca, 0xf8084fc1, 0x7a2059e6,
	},
	{ /* for 256 bit key */
		0xa5a5a5a5, 0x5a5a5a5a, 0x55aa55aa, 0xaa55aa55,
		0xa5a5a5a5, 0x5a5a5a5a, 0x55aa55aa, 0xaa55aa55,
		0x6a3d2ed7, 0x228ff865, 0x7ea361c0, 0x88610f0c,
		0x5f3f1c57, 0x8bc5316e, 0x12ab5119, 0x82e947aa,
	},
};
#endif

static int ufs_crypto_cfg_mode(unsigned int mode)
{
	if (ufs_crypto_mode == UFS_CRYPTO_MODE_UNDEFINED) {

		ufs_crypto_mode = mode;

		if (mode == UFS_CRYPTO_MODE_HW_FDE)
			CRYPTO_LOG("ufs-crypto=hw-fde\n");
		else if (mode == UFS_CRYPTO_MODE_HW_FBE)
			CRYPTO_LOG("ufs-crypto=hw-fbe\n");
		else
			CRYPTO_LOG("error: ufs-crypto=unknown\n");

	} else {

		if (ufs_crypto_mode != mode) {

			CRYPTO_LOG("error: inconsistent ufs-crypto, as-is:%d, to-be:%d\n",
				ufs_crypto_mode, mode);

			return -1;
		}
	}

	return 0;
}

static void ufs_crypto_power_ctl(enum ufs_crypto_power power)
{
	if (power == UFS_CRYPTO_POWER_ON && ufs_crypto_en == UFS_CRYPTO_POWER_OFF) {

		/* in-line encryption feature enable */
		mmio_write_32(UFSHCI_BASE + UFS_REG_CONTROLLER_ENABLE,
					(mmio_read_32(UFSHCI_BASE + UFS_REG_CONTROLLER_ENABLE) | (0x1 << 1)));

		/* Normal world change key enable */
		mmio_write_32(UFSHCI_BASE + UFS_REG_SECURE_CAP, 1);
		mmio_write_32(UFSHCI_BASE + UFS_REG_SECURE_MAP, 0);

		/* clear CG bit for ufs_aes clk */
		mmio_write_32(UFS_AES_CG_CLR, UFS_AES_CG);

		ufs_crypto_en = UFS_CRYPTO_POWER_ON;

	} else if (power == UFS_CRYPTO_POWER_OFF && ufs_crypto_en == UFS_CRYPTO_POWER_ON) {

		/* set CG bit for ufs_aes clk */
		mmio_write_32(UFS_AES_CG_SET, UFS_AES_CG);

		/* assume crypto IP will be truned-off automatically, just update flag here */
		ufs_crypto_en = UFS_CRYPTO_POWER_OFF;

	}
}

static int32_t ufs_crypto_hie_get_cap(uint32_t hie_cap)
{
	CRYPTO_LOG("hie_cap: 0x%x\n", hie_cap);
	if (hie_cap & BC_AES_128_XTS)
		return 0;
	else if (hie_cap & BC_AES_256_XTS)
		return 1;
	else
		return -1;
}

void ufs_crypto_hie_program_key(uint32_t addr, uint32_t hie_para, uint32_t key1, uint32_t key2, uint32_t key3)
{
	uint32_t start_byte = (hie_para >> UFS_HIE_PARAM_OFS_KEY_START_BYTE) & 0xFF;
	uint32_t total_byte = (hie_para >> UFS_HIE_PARAM_OFS_KEY_TOTAL_BYTE) & 0xFF;

	/* program key1 */

	if (start_byte >= total_byte)
		return;

	mmio_write_32(UFSHCI_BASE + addr + start_byte,  key1);
	CRYPTO_LOG("0x%x=0x%x\n", UFSHCI_BASE + addr + start_byte, key1);

	start_byte += sizeof(uint32_t);

	/* program key2 */

	if (start_byte >= total_byte)
		return;

	mmio_write_32(UFSHCI_BASE + addr + start_byte,  key2);
	CRYPTO_LOG("0x%x=0x%x\n", UFSHCI_BASE + addr + start_byte, key2);

	start_byte += sizeof(uint32_t);

	/* program key3 */

	if (start_byte >= total_byte || key3 == 0xFFFFFFFF)
		return;

	mmio_write_32(UFSHCI_BASE + addr + start_byte,  key3);
	CRYPTO_LOG("0x%x=0x%x\n", UFSHCI_BASE + addr + start_byte, key3);
}

uint32_t ufs_crypto_hie_init_request(uint32_t hie_para)
{
	uint32_t addr, cfg_ptr, i;
	union ufs_cpt_cap cpt_cap;
	union ufs_cap_cfg cpt_cfg;
	uint32_t cfg_id = (hie_para >> UFS_HIE_PARAM_OFS_CFG_ID) & 0xFF;
	uint32_t cap_id = (hie_para >> UFS_HIE_PARAM_OFS_MODE) & 0xFF;

	ufs_crypto_cfg_mode(UFS_CRYPTO_MODE_HW_FBE);

	cap_id = ufs_crypto_hie_get_cap(cap_id);

	/* init crypto cfg */
	memset(&cpt_cfg, 0, sizeof(cpt_cfg));

	/* enable this cfg */
	cpt_cfg.cfgx.cfg_en = 1;

	/* init capability id */
	cpt_cfg.cfgx.cap_id = (uint8_t)cap_id;

	/* init data unit size: fixed as 4 KB for UFS */
	cpt_cfg.cfgx.du_size = (1 << UFS_CRYPTO_DATA_UNIT_SIZE_4KB);

	/*
	 * Get address of cfg[cfg_id], this is also
	 * address of key in cfg[cfg_id].
	 */
	cpt_cap.cap_raw = mmio_read_32(UFSHCI_BASE + UFS_REG_CRYPTO_CAPABILITY);
	cfg_ptr = cpt_cap.cap.cfg_ptr;
	addr = (cfg_ptr << 8) + (uint32_t)(cfg_id << 7);

	/* write configuration only to register */
	for (i = 16; i < 32; i++) {
		mmio_write_32(UFSHCI_BASE + (addr + i * 4), cpt_cfg.cfgx_raw[i]);
		CRYPTO_LOG("0x%x=0x%x\n", UFSHCI_BASE + (addr + i * 4), cpt_cfg.cfgx_raw[i]);
	}

	return addr;
}

void ufs_crypto_hie_init(void)
{
	/* switch crypto engine to UFS */
	mmio_write_32(PERI_FDI_AES_SI_CTRL,
		(mmio_read_32(PERI_FDI_AES_SI_CTRL) & (~(0x1 << PERI_FDI_AES_SI_CTRL_MSDC0_FDE_EN))));

	/* enable ufs crypto IP clock and power */
	ufs_crypto_power_ctl(UFS_CRYPTO_POWER_ON);

	CRYPTO_LOG("hie (hw-fbe) init done\n");
}

void ufs_crypto_hie_cfg_request(unsigned int para1, unsigned int para2, unsigned int para3)
{
	static int stage = HIE_CFG_KEY_STAGE_NOT_START;
	static uint32_t addr, total_byte, start_byte;
	uint32_t hie_para;

	if (ufs_crypto_en == UFS_CRYPTO_POWER_OFF) {

		CRYPTO_LOG("hie_cfg_request: no power, enable crypto engine!\n");

		ufs_crypto_hie_init();
	}

	if (stage == HIE_CFG_KEY_STAGE_NOT_START) {

		CRYPTO_LOG("hie_cfg_request: 0x%x,0x%x,0x%x,%d\n", para1, para2, para3, stage);

		start_byte = 0;
		total_byte = (para1 >> UFS_HIE_PARAM_OFS_KEY_TOTAL_BYTE) & 0xFF;

		addr = ufs_crypto_hie_init_request(para1);

		hie_para = ((start_byte & 0xFF) << UFS_HIE_PARAM_OFS_KEY_START_BYTE) |
					((total_byte & 0xFF) << UFS_HIE_PARAM_OFS_KEY_TOTAL_BYTE);

		ufs_crypto_hie_program_key(addr, hie_para, para2, para3, 0xFFFFFFFF);

		start_byte += (2 * sizeof(uint32_t));

		stage = HIE_CFG_KEY_STAGE_TRANSFERRING;

	} else if (stage == HIE_CFG_KEY_STAGE_TRANSFERRING) {

		hie_para = ((start_byte & 0xFF) << UFS_HIE_PARAM_OFS_KEY_START_BYTE) |
					((total_byte & 0xFF) << UFS_HIE_PARAM_OFS_KEY_TOTAL_BYTE);

		ufs_crypto_hie_program_key(addr, hie_para, para1, para2, para3);

		start_byte += (3 * sizeof(uint32_t));

		if (start_byte >= total_byte)
			stage = HIE_CFG_KEY_STAGE_NOT_START;
	}
}


/**
 * ufs_crypto_fde_get_key
 *
 * cpt_cfg: Crypto configruration structure. key[] will be touched only in this function.
 *          Use this structure for key access to ensure the boundary of key array.
 * key_size_bits: Key size.
 * key_byte_offset: For pre-built key only. Assign key offset.
 * sha: Whether For non-pre-built key only.
 *      SHA(KEY) is required. For example, required for AES-CBC-ESSIV.
 */
static void ufs_crypto_fde_get_key(union ufs_cap_cfg *cpt_cfg, int key_size_bits,
	int key_byte_offset, int sha)
{
#ifdef MTK_UFS_CRYPTO_KEY_PREBUILD
	if (!sha)
		memset((void *)&(cpt_cfg->cfgx.key[0]), 0x5A, key_size_bits >> 3);
	else {
		/* always copy 64 byte (512 bits) to cover both key and hashed key */
		if (key_size_bits == 128) {
			memcpy((void *)&(cpt_cfg->cfgx.key[0]),
				&(ufs_crypto_key_prebuilt[0][key_byte_offset]), 64);
		} else if (key_size_bits == 192) {
			memcpy((void *)&(cpt_cfg->cfgx.key[0]),
				&(ufs_crypto_key_prebuilt[1][key_byte_offset]), 64);
		} else if (key_size_bits == 256) {
			memcpy((void *)&(cpt_cfg->cfgx.key[0]),
				&(ufs_crypto_key_prebuilt[2][key_byte_offset]), 64);
		}
	}
#else

	/* get raw fde key */
	memcpy((void *)&(cpt_cfg->cfgx.key[0]), gteearg.msg_fde_key, (key_size_bits >> 3));

	if (sha) {
		/* get sha256(fde_key), always 256 bits */
		memcpy((void *)&(cpt_cfg->cfgx.key[8]), &(hwfde_hashed_key[0]), (256 >> 3));
	}

#endif

	CRYPTO_LOG("ufs: key size %d bits\n", key_size_bits);

	for (sha = 0; sha < (key_size_bits >> 3) / sizeof(uint32_t); sha++)
		CRYPTO_LOG("\tufs: key[%d]=0x%x\n", sha, cpt_cfg->cfgx.key[sha]);

	if (sha) {
		for (sha = 0; sha < (256 >> 3) / sizeof(uint32_t); sha++)
			CRYPTO_LOG("\tufs: sha256 key[%d]=0x%x\n", sha, cpt_cfg->cfgx.key[8 + sha]);
	}
}

static void ufs_crypto_fde_init(uint32_t cap_id, uint32_t cfg_id)
{
	uint32_t addr, cfg_ptr, i;
	union ufs_cap_cfg cpt_cfg;
	union ufs_cpt_cap cpt_cap;
#ifdef MTK_UFS_CRYPTO_KEY_PREBUILD
	uint32_t alg_id, key_bits;
	union ufs_cpt_capx cpt_capx;
#endif

#ifndef MTK_UFS_DVT
	cap_id = ufs_hw_fde_cap_id;
	cfg_id = ufs_hw_fde_cfg_id;
#endif
	/* switch crypto engine to UFS */
	mmio_write_32(PERI_FDI_AES_SI_CTRL,
		(mmio_read_32(PERI_FDI_AES_SI_CTRL) & (~(0x1 << PERI_FDI_AES_SI_CTRL_MSDC0_FDE_EN))));

	ufs_crypto_power_ctl(UFS_CRYPTO_POWER_ON);

	/* get cfg ptr */
	cpt_cap.cap_raw = mmio_read_32(UFSHCI_BASE + UFS_REG_CRYPTO_CAPABILITY);
	cfg_ptr = cpt_cap.cap.cfg_ptr;
	addr = (cfg_ptr << 8) + (uint32_t)(cfg_id << 7);

	/* set crypto cfg */
	memset(&cpt_cfg, 0, sizeof(cpt_cfg));
	cpt_cfg.cfgx.cfg_en = 1;
	cpt_cfg.cfgx.cap_id = (uint8_t)cap_id;
	cpt_cfg.cfgx.du_size = (1 << UFS_CRYPTO_DATA_UNIT_SIZE_4KB);

#ifdef MTK_UFS_CRYPTO_KEY_PREBUILD

	/* get algo id */
	cpt_capx.capx_raw = mmio_read_32(UFSHCI_BASE + UFS_REG_CRYPTO_CAPABILITY_X + (cap_id << 2));

	/*
	 * apply key value according to different algorithms
	 * NOTICE: At most 256-bit key is supported.
	 */
	switch (cpt_capx.capx.key_size) {
	case 1:
		key_bits = 128;
		break;
	case 2:
		key_bits = 192;
		break;
	case 3:
		key_bits = 256;
		break;
	default:
		key_bits = 128; /* shall not happen */
		break;
	}

	alg_id = cpt_capx.capx.alg_id;

	if (alg_id == UFS_CRYPTO_ALGO_AES_XTS) {

		/* AES-XTS */
		ufs_crypto_fde_get_key((void *)&cpt_cfg, key_bits, 0, 0);

		/* XTS needs two keys (at most 256 bits each key), copy the 2nd key here */
		ufs_crypto_fde_get_key((void *)&cpt_cfg, key_bits, 8, 0);

	} else if (alg_id == UFS_CRYPTO_ALGO_ESSIV_AES_CBC) {

		/* AES-CBC-ESSIV */
		ufs_crypto_fde_get_key((void *)&cpt_cfg, key_bits, 0, 1);

	} else {

		/* Others, for example, AES-ECB */
		ufs_crypto_fde_get_key((void *)&cpt_cfg, key_bits, 0, 1);
	}
#else
	/*
	 * Skip key programming here, key will be programmed later when
	 * kernel the 1st time uses dm-crypt feacility.
	 */
#endif

	for (i = 0; i < 32; i++)
		mmio_write_32(UFSHCI_BASE + (addr + i * 4), cpt_cfg.cfgx_raw[i]);

	CRYPTO_LOG("hw-fde init done\n");
}

static void ufs_crypto_fde_set_key(void)
{
	uint32_t addr, cfg_ptr, i;
	union ufs_cpt_cap cpt_cap;
	union ufs_cap_cfg cpt_cfg;

	ufs_crypto_cfg_mode(UFS_CRYPTO_MODE_HW_FDE);

	/* ensure hci inline encryption engine is enabled */
	ufs_crypto_fde_init(0, 0);

	/* get key */
	ufs_crypto_fde_get_key((void *)&cpt_cfg,
		ufs_hw_fde_key_size_bits, 0, 1);

	/*
	 * get address of cfg[cfg_id], this is also
	 * address of key in cfg[cfg_id].
	 */
	cpt_cap.cap_raw = mmio_read_32(UFSHCI_BASE + UFS_REG_CRYPTO_CAPABILITY);
	cfg_ptr = cpt_cap.cap.cfg_ptr;
	addr = (cfg_ptr << 8) + (uint32_t)(ufs_hw_fde_cfg_id << 7);

	/* program key only */
	for (i = 0; i < (ufs_hw_fde_key_size_bits >> 3) / sizeof(uint32_t); i++)
		mmio_write_32(UFSHCI_BASE + (addr + i * 4), cpt_cfg.cfgx.key[i]);

	/* program sha256 result to key[8~15] */
	for (i = 0; i < (256 >> 3) / sizeof(uint32_t); i++)
		mmio_write_32(UFSHCI_BASE + (addr + (8 + i) * 4), cpt_cfg.cfgx.key[8 + i]);
}

static void ufs_crypto_suspend(void)
{
	CRYPTO_LOG("suspend\n");

	ufs_crypto_power_ctl(UFS_CRYPTO_POWER_OFF);
}

static void ufs_crypto_resume(void)
{
	CRYPTO_LOG("resume\n");

	/* enable crypto ip in resume first anyway */
	ufs_crypto_power_ctl(UFS_CRYPTO_POWER_ON);

	if (ufs_crypto_mode == UFS_CRYPTO_MODE_HW_FBE)
		ufs_crypto_hie_init();
	else if (ufs_crypto_mode == UFS_CRYPTO_MODE_HW_FDE)
		ufs_crypto_fde_set_key();
}

int32_t ufs_crypto_ctl(uint32_t ctl, uint32_t param2, uint32_t param3)
{
	int32_t ret = MTK_SIP_E_SUCCESS;

	CRYPTO_LOG_INIT();
	CRYPTO_LOG("ufs_crypto_ctl: ctl=0x%x, para=%d,%d\n", ctl, param2, param3);

	if (ctl == UFS_AES_CTL_FDE_INIT)
		ufs_crypto_fde_init(param2, param3);
	else if (ctl == UFS_AES_CTL_SUSPEND)
		ufs_crypto_suspend();
	else if (ctl == UFS_AES_CTL_RESUME)
		ufs_crypto_resume();
	else if (ctl == UFS_AES_CTL_SET_KEY)
		ufs_crypto_fde_set_key();
	else
		ret = MTK_SIP_E_NOT_SUPPORTED;

	CRYPTO_LOG_UNINIT();

	return ret;
}

static void ufs_mphy_va09_cg_ctl(uint32_t enable)
{
	uint32_t reg;

	reg = mmio_read_32(PERICFG_BASE + 0x448);

	if (enable)
		reg = reg | (0x1 << 1);
	else
		reg = reg & ~(0x1 << 1);

	mmio_write_32(PERICFG_BASE + 0x448, reg);
}

int32_t ufs_generic_ctl(uint32_t ctl, uint32_t param2, uint32_t param3)
{
	if (ctl == UFS_MPHY_VA09_CG_CTL)
		ufs_mphy_va09_cg_ctl(param2);
	else
		return MTK_SIP_E_NOT_SUPPORTED;

	return MTK_SIP_E_SUCCESS;
}

#else

int32_t ufs_crypto_ctl(uint32_t ctl, uint32_t param2, uint32_t param3)
{
	return MTK_SIP_E_NOT_SUPPORTED;
}

#endif /* MTK_UFS_SUPPORT */

