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
#include <mt_i2c.h>
/* #include <crypto.h> */

int32_t i2c_set_secure_reg(uint32_t ch, uint32_t offset, uint32_t value)
{
	int32_t ret = MTK_SIP_E_INVALID_PARAM;

	/* CRYPTO_LOG_INIT(); */

	if ((ch != I2C2) && (ch != I2C4) && (ch != I2C3)) {
		/*
		 * CRYPTO_LOG("i2c_set_secure_reg wrong channel %d\n", ch);
		 */
		goto DONE;
	}

	if (((offset & 0xFFF) < OFFSET_SEC_CONTROL) ||
	    ((offset & 0xFFF) > OFFSET_SEC_ADDR)) {
		/*
		 * CRYPTO_LOG("i2c_set_secure_reg wrong offset %x\n", offset);
		 */
		goto DONE;
	}

	if (ch == I2C2) {
		if (offset == OFFSET_ROLLBACK)
			mmio_write_16(I2C2_BASE_SE + OFFSET_ROLLBACK, 0);
		else if (offset == OFFSET_MULTI_DMA)
			mmio_write_16(I2C2_BASE_SE + OFFSET_MULTI_DMA, 2);
	} else if (ch == I2C3) {
		if (offset == OFFSET_ROLLBACK)
			mmio_write_16(I2C3_BASE_SE + OFFSET_ROLLBACK, 0);
		else if (offset == OFFSET_MULTI_DMA)
			mmio_write_16(I2C3_BASE_SE + OFFSET_MULTI_DMA, 2);
	} else if (ch == I2C4) {
		if (offset == OFFSET_ROLLBACK)
			mmio_write_16(I2C4_BASE_SE + OFFSET_ROLLBACK, 0);
		else if (offset == OFFSET_MULTI_DMA)
			mmio_write_16(I2C4_BASE_SE + OFFSET_MULTI_DMA, 2);
	}

	ret = MTK_SIP_E_SUCCESS;
	/*
	 * CRYPTO_LOG("i2c_set_secure_reg done %x\n", value);
	 */
DONE:
	/*
	 * CRYPTO_LOG_UNINIT();
	 */
	return ret;
}
