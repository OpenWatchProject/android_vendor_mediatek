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
#include <arch_helpers.h>
#include <debug.h> /*For INFO*/
#include <mmio.h>
#include <mtk_sip_svc.h>
#include <runtime_svc.h>

#define LTE_OP_ZERO_2_TWO     (0x00000)
#define LTE_OP_THREE_2_FIVE   (0x00001)
#define LTE_OP_SIX_SEVEN      (0x00002)

#define LTE_SBC_LOCK_VAL (0x0000659E)

/*******************************************************************************
 * SMC Call for register write (for LTE C2K public key hash)
 ******************************************************************************/
static uint32_t md_reg_write_and_check(uint32_t addr, uint32_t value)
{
	INFO("write addr 0x%x value 0x%x\n", addr, value);
	mmio_write_32(addr, value);
	INFO("read addr 0x%x = 0x%x\n", addr, mmio_read_32(addr));
	if (mmio_read_32(addr) != value)
		return MTK_SIP_E_MD_REG_WRITE_FAIL;

	return MTK_SIP_E_SUCCESS;
}

uint64_t sip_write_md_regs(uint32_t cmd_type, uint32_t val1, uint32_t val2, uint32_t val3)
{
	uint32_t test_val = 0x11223344;
	uint32_t ret = MTK_SIP_E_SUCCESS;

	switch (cmd_type) {
	case LTE_OP_ZERO_2_TWO:
		ret = md_reg_write_and_check(LTE_SBC_PUBK_HASH0, val1);
		if (ret)
			return ret;
		ret = md_reg_write_and_check(LTE_SBC_PUBK_HASH1, val2);
		if (ret)
			return ret;
		ret = md_reg_write_and_check(LTE_SBC_PUBK_HASH2, val3);
		if (ret)
			return ret;

		break;

	case LTE_OP_THREE_2_FIVE:
		ret = md_reg_write_and_check(LTE_SBC_PUBK_HASH3, val1);
		if (ret)
			return ret;
		ret = md_reg_write_and_check(LTE_SBC_PUBK_HASH4, val2);
		if (ret)
			return ret;
		ret = md_reg_write_and_check(LTE_SBC_PUBK_HASH5, val3);
		if (ret)
			return ret;

		break;

	case LTE_OP_SIX_SEVEN:
		ret = md_reg_write_and_check(LTE_SBC_PUBK_HASH6, val1);
		if (ret)
			return ret;
		ret = md_reg_write_and_check(LTE_SBC_PUBK_HASH7, val2);
		if (ret)
			return ret;
		/* lock */
		mmio_write_32(LTE_SBC_LOCK, LTE_SBC_LOCK_VAL);
		INFO("addr 0x%x = 0x%x\n", LTE_SBC_LOCK,
				mmio_read_32(LTE_SBC_LOCK));

		/* test lock */
		mmio_write_32(LTE_SBC_PUBK_HASH7, test_val);
		dsb();

		if (test_val == mmio_read_32(LTE_SBC_PUBK_HASH7))
			return MTK_SIP_E_LOCK_FAIL;
		INFO("LTE lock test pass\n");

		break;

	default:
		return MTK_SIP_E_NOT_SUPPORTED;
	}

	return MTK_SIP_E_SUCCESS;
}
