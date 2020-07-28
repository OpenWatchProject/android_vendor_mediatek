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

#ifndef _MT_I2C_H_
#define _MT_I2C_H_

/* multi-channel I2C */
#define I2C2	2
#define I2C3	3
#define I2C4	4
#define I2C7	7

enum I2C_REGS_OFFSET {
	OFFSET_SEC_CONTROL    = 0xF00,
	OFFSET_SEC_CCC_RANG   = 0xF04,
	OFFSET_SEC_CCC_EXT    = 0xF08,
	OFFSET_SEC_CCC_CTRL   = 0xF0C,
	OFFSET_SEC_DOW_CTRL0  = 0xF10,
	OFFSET_SEC_DOW_CTRL1  = 0xF14,
	OFFSET_SEC_DOW_CTRL2  = 0xF18,
	OFFSET_CHANNEL_LOCK   = 0xF80,
	OFFSET_CHANNEL_SEC    = 0xF84,
	OFFSET_DCM_EN         = 0xF88,
	OFFSET_MULTI_DMA      = 0xF8C,
	OFFSET_DMA_REQ        = 0xF90,
	OFFSET_DCM_NREQ       = 0xF94,
	OFFSET_ROLLBACK       = 0xF98,
	OFFSET_CHANNEL_CLOCK  = 0xF9C,
	OFFSET_SEC_ADDR       = 0xFA0,
};

int32_t i2c_set_secure_reg(uint32_t ch, uint32_t offset, uint32_t value);

#endif

