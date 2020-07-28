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

#ifndef __UFS_PLATFORM_H
#define __UFS_PLATFORM_H

 /*=======================================================================*/
 /* UFS Related Registers												  */
 /*=======================================================================*/
#define UFSHCI_BASE         (0x11290000)

enum {
	UFS_REG_CONTROLLER_ENABLE   = 0x34,
	UFS_REG_CRYPTO_CAPABILITY   = 0x100,
	UFS_REG_CRYPTO_CAPABILITY_X = 0x104,
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

#endif /* __UFS_PLATFORM_H */

