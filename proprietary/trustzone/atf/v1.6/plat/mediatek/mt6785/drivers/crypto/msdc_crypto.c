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

#include <arch.h>
#include <debug.h>
#include <mmio.h>
#include <mtk_sip_svc.h>
#include <platform_def.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <msdc_crypto.h>
#include <crypto.h>

#ifdef MTK_MSDC_HW_FDE

static struct msdc_crypt_config crypt_cap =
/*================MSDC_CRYPTO_AES_CBC_ESSIV=======================================================*/
{
	MSDC_CRYPTO_AES_CBC_ESSIV,
	512,
	BIT_128,
	{
		0xa5a5a5a5, 0x5a5a5a5a, 0x55aa55aa, 0xaa55aa55,
		0x0,        0x0,        0x0,        0x0,
	},
	BIT_256,
	{
		0x9309da32, 0x87a0c7fc, 0x1b58a359, 0x14fc38ea,
		0x68c7b319, 0x0dcb6355, 0xa0e565e9, 0x8765ccb5,
	},
	BIT_0,
	{
		0x0,         0x0,    0x0,        0x0,
	},
};

/**
 * msdc_crypto_get_key
 *
 *
 */
static uint32_t msdc_get_length_by_enum(uint8_t key_bit)
{
	uint32_t length = 0;

	switch (key_bit) {
	case BIT_128:
		length = 128;
		break;
	case BIT_192:
		length = 192;
		break;
	case BIT_256:
		length = 256;
		break;
	default:
		break;
	}

	return length;
}

static void msdc_crypto_get_key(uint32_t cap_id)
{
	uint32_t i = 0;
	uint32_t key_length = msdc_get_length_by_enum(crypt_cap.key_bit);
	uint32_t tkey_length = msdc_get_length_by_enum(crypt_cap.tkey_bit);

#ifndef MTK_UFS_CRYPTO_KEY_PREBUILD
	/* get raw fde key */
	memcpy((void *)&(crypt_cap.key[0]), gteearg.msg_fde_key, (key_length >> 3));
	memcpy((void *)&(crypt_cap.tkey[0]), &(hwfde_hashed_key[0]), (tkey_length >> 3));

#endif

	CRYPTO_LOG("Crypto Key Size: %d bits\n", key_length);

	CRYPTO_LOG("Crypto Key:\n");
	for (i = 0; i < (key_length >> 3) / sizeof(uint32_t); i++)
		CRYPTO_LOG("\tKEY[%d] = 0x%x\n", i, crypt_cap.key[i]);

	CRYPTO_LOG("Crypto SHA256(Key):\n");
	for (i = 0; i < (tkey_length >> 3) / sizeof(uint32_t); i++)
		CRYPTO_LOG("\tTKEY[%d] = 0x%x\n", i, crypt_cap.tkey[i]);
}

/* used for FBE init */
static void msdc_crypto_cfg(uint32_t cap_id, uint32_t cfg_id)
{
	/* switch crypto engine to MSDC */
	mmio_write_32(PERI_FDI_AES_SI_CTRL,
		(mmio_read_32(PERI_FDI_AES_SI_CTRL) | (0x1 << PERI_FDI_AES_SI_CTRL_MSDC0_FDE_EN)));

	/* select aes setting */
	/* set div 0, key slot */
	/* enable AES(secure), by setting AES on bit */
	mmio_write_32(EMMC52_AES_EN,
		((mmio_read_32(EMMC52_AES_EN) & ~EMMC52_AES_CLK_DIV_SEL) | (0x1 << (cfg_id+1) | EMMC52_AES_ON)));

	/* disable AES(normal), by setting AES bypass bit */
	/*
	 * disable AES in ATF as default
	 * normal world will enable when needed
	 */
	mmio_write_32(EMMC52_AES_SWST,
		mmio_read_32(EMMC52_AES_SWST) | EMMC52_AES_BYPASS);

	/* enable CQ crypto */
	mmio_write_32(CQHCI_CQCFG,
		mmio_read_32(CQHCI_CQCFG) | CQ_CRYPTO_ENABLE);
}

/* used for FDE init */
static void msdc_crypto_init(uint32_t cap_id, uint32_t cfg_id)
{
	uint32_t i = 0;

	CRYPTO_LOG_INIT();

	/* cap_id is not used */

	CRYPTO_LOG("Crypto init, cap_id: %d, cfg_id: %d\n", cap_id, cfg_id);

	/* get key */
	msdc_crypto_get_key(cap_id);

	/* switch AES IP to MSDC*/
	msdc_crypto_cfg(cap_id, cfg_id);

	if (cfg_id == 0) {
		mmio_write_32(EMMC52_AES_CFG_GP0,
			((mmio_read_32(EMMC52_AES_CFG_GP0) & ~(EMMC52_AES_DATA_UNIT_SIZE_0))
			| (crypt_cap.data_unit_size << 16)));
		mmio_write_32(EMMC52_AES_CFG_GP0,
			((mmio_read_32(EMMC52_AES_CFG_GP0) & ~(EMMC52_AES_KEY_SIZE_0))
			| (crypt_cap.key_bit << 8)));
		mmio_write_32(EMMC52_AES_CFG_GP0,
			((mmio_read_32(EMMC52_AES_CFG_GP0) & ~(EMMC52_AES_MODE_0))
			| (crypt_cap.aes_mode << 0)));

		for (i = 0; i < 4; i++)
			mmio_write_32(EMMC52_AES_IV0_GP0+i*4, crypt_cap.iv[i]);

		for (i = 0; i < 8; i++)
			mmio_write_32(EMMC52_AES_KEY0_GP0+i*4, crypt_cap.key[i]);

		for (i = 0; i < 8; i++)
			mmio_write_32(EMMC52_AES_TKEY0_GP0+i*4, crypt_cap.tkey[i]);
	} else if (cfg_id == 1) {
		mmio_write_32(EMMC52_AES_CFG_GP1,
			((mmio_read_32(EMMC52_AES_CFG_GP1) & ~(EMMC52_AES_DATA_UNIT_SIZE_1))
			| (crypt_cap.data_unit_size << 16)));
		mmio_write_32(EMMC52_AES_CFG_GP1,
			((mmio_read_32(EMMC52_AES_CFG_GP1) & ~(EMMC52_AES_KEY_SIZE_1))
			| (crypt_cap.key_bit << 8)));
		mmio_write_32(EMMC52_AES_CFG_GP1,
			((mmio_read_32(EMMC52_AES_CFG_GP1) & ~(EMMC52_AES_MODE_1))
			| (crypt_cap.aes_mode << 0)));

		for (i = 0; i < 4; i++)
			mmio_write_32(EMMC52_AES_IV0_GP1+i*4, crypt_cap.iv[i]);

		for (i = 0; i < 8; i++)
			mmio_write_32(EMMC52_AES_KEY0_GP1+i*4, crypt_cap.key[i]);

		for (i = 0; i < 8; i++)
			mmio_write_32(EMMC52_AES_TKEY0_GP1+i*4, crypt_cap.tkey[i]);
	}

	CRYPTO_LOG("Crypto init done\n");
	CRYPTO_LOG_UNINIT();
}

/*
 * for cqhci only
 * return key length in bits by cap id
 */
static uint32_t msdc_get_length_by_capid(uint32_t cap_id)
{
	uint32_t length = 0;

	switch (cap_id) {
	case CQHCI_CRYPTO_AES_XTS_128:
	case CQHCI_CRYPTO_BITLOCKER_AES_CBC_128:
	case CQHCI_CRYPTO_AES_ECB_128:
	case CQHCI_CRYPTO_ESSIV_AES_CBC_128:
		length = 128;
		break;
	case CQHCI_CRYPTO_AES_ECB_192:
	case CQHCI_CRYPTO_ESSIV_AES_CBC_192:
		length = 192;
		break;
	case CQHCI_CRYPTO_AES_XTS_256:
	case CQHCI_CRYPTO_BITLOCKER_AES_CBC_256:
	case CQHCI_CRYPTO_AES_ECB_256:
	case CQHCI_CRYPTO_ESSIV_AES_CBC_256:
		length = 256;
		break;
	default:
		break;
	}

	return length;
}

static void msdc_crypto_hci_get_key(union cqhci_cap_cfg *cpt_cfg, int key_size_bits,
	int sha)
{
	int i;

	/* get raw fde key */
	memcpy((void *)&(cpt_cfg->cfgx.key[0]), gteearg.msg_fde_key, (key_size_bits >> 3));

	if (sha) {
		/* get sha256(fde_key), always 256 bits */
		memcpy((void *)&(cpt_cfg->cfgx.key[8]), &(hwfde_hashed_key[0]), (256 >> 3));
	}

	for (i = 0; i < (key_size_bits >> 3) / sizeof(uint32_t); i++)
		CRYPTO_LOG("\tcqhci: key[%d]=0x%x\n", sha, cpt_cfg->cfgx.key[i]);

	for (i = 0; i < (256 >> 3) / sizeof(uint32_t); i++)
		CRYPTO_LOG("\tcqhci: sha256 key[%d]=0x%x\n", sha, cpt_cfg->cfgx.key[8 + i]);
}

/* used for FDE init, cqhci case */
static void msdc_crypto_hci_init(uint32_t cap_id, uint32_t cfg_id)
{
	uint32_t addr, cfg_ptr, i;
	union cqhci_cap_cfg cpt_cfg;
	union cqhci_cpt_cap cpt_cap;

	CRYPTO_LOG_INIT();

	/* for FDE, use ESSIV AES CBC 128, slot0 */
	cap_id = CQHCI_CRYPTO_ESSIV_AES_CBC_128;
	cfg_id = 0;

	CRYPTO_LOG("CQHCI crypto init, cap_id: %d, cfg_id: %d\n", cap_id, cfg_id);

	/* switch crypto engine to MSDC */
	mmio_write_32(PERI_FDI_AES_SI_CTRL,
		(mmio_read_32(PERI_FDI_AES_SI_CTRL) | (0x1 << PERI_FDI_AES_SI_CTRL_MSDC0_FDE_EN)));

	/* enable CQ crypto */
	mmio_write_32(CQHCI_CQCFG,
		mmio_read_32(CQHCI_CQCFG) | CQ_CRYPTO_ENABLE);

	CRYPTO_LOG("\tCQHCI: CQHCI_CQCFG=0x%x\n", mmio_read_32(CQHCI_CQCFG));

	/* get cfg ptr */
	cpt_cap.cap_raw = mmio_read_32(CQHCI_CRCAP);
	cfg_ptr = cpt_cap.cap.cfg_ptr;
	addr = (cfg_ptr << 8) + (uint32_t)(cfg_id << 7);

	/* set crypto cfg */
	memset(&cpt_cfg, 0, sizeof(cpt_cfg));
	cpt_cfg.cfgx.cfg_en = 1;
	cpt_cfg.cfgx.cap_id = (uint8_t)cap_id;
	cpt_cfg.cfgx.du_size = (1 << CQHCI_CRYPTO_DU_SIZE_512B);

	msdc_crypto_hci_get_key((void *)&cpt_cfg,
		msdc_get_length_by_capid(CQHCI_CRYPTO_ESSIV_AES_CBC_128), 1);

	for (i = 0; i < 32; i++)
		mmio_write_32(CQHCI_BASE + (addr + i * 4), cpt_cfg.cfgx_raw[i]);

	for (i = 0; i < 32; i++)
		CRYPTO_LOG("\tCQHCI: REG(0x%x)=0x%x\n",
			CQHCI_BASE + (addr + i * 4),
			mmio_read_32(CQHCI_BASE + (addr + i * 4)));

	CRYPTO_LOG("CQHCI crypto init done\n");
	CRYPTO_LOG_UNINIT();
}

int32_t msdc_crypto_ctl(uint32_t ctl, uint32_t param2, uint32_t param3)
{
	CRYPTO_LOG_INIT();
	CRYPTO_LOG("msdc_crypto_ctl init, ctl: %d, param2: %d, param3: %d\n",
		ctl, param2, param3);

	if (ctl == MSDC_AES_CTL_INIT)
		/* FBE init */
		msdc_crypto_cfg(param2, param3);
	else if (ctl == MSDC_AES_CTL_SUSPEND)
		;
	else if (ctl == MSDC_AES_CTL_RESUME)
		;
	else if (ctl == MSDC_AES_CTL_SET_KEY)
		/* FDE init, set key */
		msdc_crypto_init(param2, param3);
	else if (ctl == MSDC_AES_CTL_CQHCI_SET_KEY)
		/* FDE init (cqhci), set key */
		msdc_crypto_hci_init(param2, param3);
	else
		return MTK_SIP_E_NOT_SUPPORTED;

	CRYPTO_LOG("msdc_crypto_ctl init done\n");
	CRYPTO_LOG_UNINIT();

	return MTK_SIP_E_SUCCESS;
}

#else
int32_t msdc_crypto_ctl(uint32_t ctl, uint32_t param2, uint32_t param3)
{
	return MTK_SIP_E_NOT_SUPPORTED;
}

#endif /* MTK_MSDC_HW_FDE */
