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

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <arch.h>
#include <debug.h>
#include <mmio.h>
#include <crypto.h>
#include <mtk_sip_svc.h>
#include <platform_def.h>
#include <mtk_i2c.h>


#define MAX_I2C_NR	7 /* Max Number of I2C controllers */
#define I2C0	0
#define I2C1	1
#define I2C2	2
#define I2C3	2
#define I2C4	4
#define I2C5	5
#define I2C6	6
#define I2C7	7
#define I2C8	8
#define I2C9	9

struct multi_chn_info {
	uint32_t chn;
	uint32_t base_se;
	int32_t is_multi_support;/* 0, not support; 1, support */
};

/* multi-channel I2C info. */
static struct multi_chn_info i2c_chn_info[MAX_I2C_NR] = {
	{I2C0, 0, 0},
	{I2C1, 0, 0},
	{I2C2, I2C2_SE_BASE, 1},
	{I2C3, I2C3_SE_BASE, 1},
	{I2C4, I2C4_SE_BASE, 1},
	{I2C5, 0, 0},
	{I2C6, I2C6_SE_BASE, 1},
};

int32_t i2c_set_secure_reg(uint32_t ch, uint32_t offset, uint32_t value)
{
	int32_t ret = MTK_SIP_E_INVALID_PARAM;

	CRYPTO_LOG_INIT();
	if (ch < MAX_I2C_NR) {
		if (!i2c_chn_info[ch].is_multi_support) {
			CRYPTO_LOG("i2c_set_secure_reg wrong multi-channel %d\n", ch);
			goto DONE;
		}
	} else {
		CRYPTO_LOG("i2c_set_secure_reg Invalid channel %d\n", ch);
		goto DONE;
	}

	if (((offset & 0xFFF) < OFFSET_SEC_CONTROL) ||
		((offset & 0xFFF) > OFFSET_SEC_ADDR)) {
		CRYPTO_LOG("i2c_set_secure_reg wrong offset 0x%x\n", offset);
		goto DONE;
	}

	mmio_write_16(i2c_chn_info[ch].base_se + (offset & 0xFFF), value & 0xFFFF);

	ret = MTK_SIP_E_SUCCESS;
	CRYPTO_LOG("i2c_set_secure_reg done 0x%x\n", value);

DONE:
	CRYPTO_LOG_UNINIT();
	return ret;

}
