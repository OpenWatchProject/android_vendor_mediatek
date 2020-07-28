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

#define LTE_OP_ZERO_2_TWO   (0x00000)
#define LTE_OP_THREE_2_FIVE (0x00001)
#define LTE_OP_SIX_SEVEN    (0x00002)

#define C2K_OP_ZERO_2_TWO   (0x10000)
#define C2K_OP_THREE_2_FIVE (0x10001)
#define C2K_OP_SIX_SEVEN    (0x10002)

#define LTE_SBC_LOCK_VAL    (0x0000659E)
#define C2K_SBC_LOCK_VAL    (0x0000C275)

/*******************************************************************************
 * SMC Call for register write (for LTE C2K public key hash)
 ******************************************************************************/

uint64_t sip_write_md_regs(uint32_t cmd_type, uint32_t val1,uint32_t val2, uint32_t val3)
{
	uint32_t test_val = 0x11223344;
	switch (cmd_type){
	case LTE_OP_ZERO_2_TWO:
		mmio_write_32(LTE_SBC_PUBK_HASH0, val1);
		mmio_write_32(LTE_SBC_PUBK_HASH1, val2);
		mmio_write_32(LTE_SBC_PUBK_HASH2, val3);
		break;
	case LTE_OP_THREE_2_FIVE:
		mmio_write_32(LTE_SBC_PUBK_HASH3, val1);
		mmio_write_32(LTE_SBC_PUBK_HASH4, val2);
		mmio_write_32(LTE_SBC_PUBK_HASH5, val3);
		break;
	case LTE_OP_SIX_SEVEN:
		mmio_write_32(LTE_SBC_PUBK_HASH6, val1);
		mmio_write_32(LTE_SBC_PUBK_HASH7, val2);
		mmio_write_32(LTE_SBC_LOCK, LTE_SBC_LOCK_VAL);
		/* test lock */
		mmio_write_32(LTE_SBC_PUBK_HASH7, test_val);
		dsb();
		if (test_val == mmio_read_32(LTE_SBC_PUBK_HASH7))
			return MTK_SIP_E_LOCK_FAIL;
		INFO("LTE lock test pass \n");
		break;
	case C2K_OP_ZERO_2_TWO:
		/* C2K is not suppoted for mt6763 */
		break;
	case C2K_OP_THREE_2_FIVE:
		/* C2K is not suppoted for mt6763 */
		break;
	case C2K_OP_SIX_SEVEN:
		/* C2K is not suppoted for mt6763 */
		break;
	default:
		return MTK_SIP_E_NOT_SUPPORTED;
	}

	return MTK_SIP_E_SUCCESS;
}

