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
#include <platform_def.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <mtk_sip_svc.h>
#include <ufs_platform.h>

#ifdef MTK_UFS_SUPPORT

uint32_t sha256_prebuilt[3][16] = {
	{ /*for 128 bit key*/
		0xa5a5a5a5, 0x5a5a5a5a, 0x55aa55aa, 0xaa55aa55,
		0x0,         0x0,    0x0,        0x0,
		0x9309da32, 0x87a0c7fc, 0x1b58a359, 0x14fc38ea,
		0x68c7b319, 0x0dcb6355, 0xa0e565e9, 0x8765ccb5,
	},
	{ /*for 192 bit key*/
		0xa5a5a5a5, 0x5a5a5a5a, 0x55aa55aa, 0xaa55aa55,
		0xa5a5a5a5, 0x5a5a5a5a, 0x0,        0x0,
		0xad168c0f, 0x502b6ae1, 0x30199ad0, 0x69488013,
		0x42659ad8, 0xf94f95ca, 0xf8084fc1, 0x7a2059e6,
	},
	{ /*for 256 bit key*/
		0xa5a5a5a5, 0x5a5a5a5a, 0x55aa55aa, 0xaa55aa55,
		0xa5a5a5a5, 0x5a5a5a5a, 0x55aa55aa, 0xaa55aa55,
		0x6a3d2ed7, 0x228ff865, 0x7ea361c0, 0x88610f0c,
		0x5f3f1c57, 0x8bc5316e, 0x12ab5119, 0x82e947aa,
	},
};

int32_t ufs_mtk_atf_crypto_init(void)
{
	uint32_t addr, key_bits, key_bytes, cfg_ptr, alg_id, i;
	uint32_t key[16];
	union ufs_cap_cfg cpt_cfg;
	union ufs_cpt_cap cpt_cap;
	union ufs_cpt_capx cpt_capx;

	uint32_t cap_id = 7; /* AES-CBC-ESSIV-128bit */
	uint32_t cfg_id = 0; /* use slot 0 by default */

	/* in-line encryption feature enable */
	mmio_write_32(UFSHCI_BASE + UFS_REG_CONTROLLER_ENABLE,
				(mmio_read_32(UFSHCI_BASE + UFS_REG_CONTROLLER_ENABLE) | (0x1 << 1)));

	/* get algo id */
	cpt_capx.capx_raw = mmio_read_32(UFSHCI_BASE + UFS_REG_CRYPTO_CAPABILITY_X + (cap_id << 2));
	alg_id = cpt_capx.capx.alg_id;

	/* get cfg ptr */
	cpt_cap.cap_raw = mmio_read_32(UFSHCI_BASE + UFS_REG_CRYPTO_CAPABILITY);
	cfg_ptr = cpt_cap.cap.cfg_ptr;
	addr = (cfg_ptr << 8) + (uint32_t)(cfg_id << 7);

	/* set default key */
	memset(key, 0x5A, sizeof(key));

	/* set crypto cfg */
	memset(&cpt_cfg, 0, sizeof(cpt_cfg));
	cpt_cfg.cfgx.cfg_en = 1;
	cpt_cfg.cfgx.cap_id = (uint8_t)cap_id;
	cpt_cfg.cfgx.du_size = (1 << UFS_CRYPTO_DATA_UNIT_SIZE_4KB);

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
	default :
		key_bits = 128; // shall not happen
		break;
	}

	key_bytes = key_bits >> 3;  /* byte count*/

	if (UFS_CRYPTO_ALGO_AES_XTS == alg_id) {               // AES-XTS

		memcpy((void *)&(cpt_cfg.cfgx.key[0]), (void *)key, key_bytes);

		/* XTS needs two keys (at most 256 bits each key), copy the 2nd key here */
		memcpy((void *)&(cpt_cfg.cfgx.key[8]), (void *)(key + 8), key_bytes);

	} else if (UFS_CRYPTO_ALGO_ESSIV_AES_CBC == alg_id) {  // AES-CBC-ESSIV

		if (128 == key_bits)
			memcpy((void *)&(cpt_cfg.cfgx.key[0]), (void *)sha256_prebuilt[0], 64);  // always copy 64 byte = 512 bits, will cover both key and hash(key)
		else if (192 == key_bits)
			memcpy((void *)&(cpt_cfg.cfgx.key[0]), (void *)sha256_prebuilt[1], 64);  // always copy 64 byte = 512 bits, will cover both key and hash(key)
		else if (256 == key_bits)
			memcpy((void *)&(cpt_cfg.cfgx.key[0]), (void *)sha256_prebuilt[2], 64);  // always copy 64 byte = 512 bits, will cover both key and hash(key)

	} else                                                  // AES-ECB
		memcpy((void *)&(cpt_cfg.cfgx.key[0]), (void *)key, key_bytes);

	for (i = 0; i < 32; i++)
		mmio_write_32(UFSHCI_BASE + (addr + i * 4), cpt_cfg.cfgx_raw[i]);

	return SIP_SVC_E_SUCCESS;
}

#else

int32_t ufs_mtk_atf_crypto_init(void)
{
	return MTK_SIP_E_NOT_SUPPORTED;
}

#endif /* MTK_UFS_SUPPORT */
