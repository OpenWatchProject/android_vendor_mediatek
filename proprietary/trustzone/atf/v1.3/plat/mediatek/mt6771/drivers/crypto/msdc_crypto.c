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

#include <arch.h>
#include <debug.h>
#include <mmio.h>
#include <mtk_sip_svc.h>
#include <platform_def.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <msdc.h>
#include <msdc_crypto.h>
#include <crypto.h>

#ifdef MTK_MSDC_HW_FDE
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
	for(i = 0; i < (key_length >> 3) / sizeof(uint32_t); i++)
		CRYPTO_LOG("\tKEY[%d] = 0x%x\n", i, crypt_cap.key[i]);

	CRYPTO_LOG("Crypto SHA256(Key):\n");
	for(i = 0; i < (tkey_length >> 3) / sizeof(uint32_t); i++)
		CRYPTO_LOG("\tTKEY[%d] = 0x%x\n", i, crypt_cap.tkey[i]);

	return;
}

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
}

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

		for (i = 0; i < 4; i++) {
			mmio_write_32(EMMC52_AES_IV0_GP0+i*4, crypt_cap.iv[i]);
		}

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

	return;
}

int32_t msdc_crypto_ctl(uint32_t ctl, uint32_t param2, uint32_t param3)
{
	CRYPTO_LOG_INIT();
	CRYPTO_LOG("msdc_crypto_ctl init, ctl: %d\n", ctl);

	if (ctl == MSDC_AES_CTL_INIT)
		msdc_crypto_cfg(param2, param3);
	else if (ctl == MSDC_AES_CTL_SUSPEND)
		;
	else if (ctl == MSDC_AES_CTL_RESUME)
		;
	else if (ctl == MSDC_AES_CTL_SET_KEY)
		msdc_crypto_init(param2, param3);
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
