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

#ifndef _MT_M4U_H_
#define _MT_M4U_H_

#define M4U_MM_DOMAIN   0
#define M4U_VPU_DOMIAN  1

/* m4u atf debug parameter */
/* 0~19 leave to iommu bank */
#define M4U_ATF_SECURITY_DEBUG_EN  (20)
#define M4U_ATF_BANK1_4_TF         (21)
#define M4U_ATF_DUMP_INFO          (22)

/* common macro definitions */
#define F_VAL(val, msb, lsb) (((val)&((1<<(msb-lsb+1))-1))<<lsb)
#define F_VAL_L(val, msb, lsb) (((val)&((1L<<(msb-lsb+1))-1))<<lsb)
#define F_MSK(msb, lsb)     F_VAL(0xffffffff, msb, lsb)
#define F_MSK_L(msb, lsb)     F_VAL_L(0xffffffffffffffffL, msb, lsb)
#define F_BIT_SET(bit)	  (1<<(bit))
#define F_BIT_VAL(val, bit)  ((!!(val))<<(bit))
#define F_MSK_SHIFT(regval, msb, lsb) (((regval)&F_MSK(msb, lsb))>>lsb)

/* REG OFFSET */
#define REG_MMU_IVRP_PADDR_SEC       0x114

#define REG_MMU_INT_L2_CONTROL_SEC      0x120
#define F_INT_L2_CLR_BIT_SEC (1<<12)
#define F_INT_L2_TABLE_WALK_FAULT_SEC		  F_BIT_SET(1)
#define F_INT_L2_INVALD_DONE_SEC		       F_BIT_SET(4)

#define REG_MMU_INT_MAIN_CONTROL_SEC    0x124
#define F_INT_TRANSLATION_FAULT_SEC(MMU)	F_BIT_SET(0+(MMU)*7)
#define F_INT_INVALID_PA_FAULT_SEC(MMU)		F_BIT_SET(2+(MMU)*7)

#define REG_MMU_L2_FAULT_ST_SEC	 0x130

#define REG_MMU_MAIN_FAULT_ST_SEC       0x134
#define F_INT_MMU0_MSK_SEC                              F_MSK(2, 0)
#define F_INT_MMU1_MSK_SEC                              F_MSK(9, 7)


#define REG_MMU_TBWALK_FAULT_VA_SEC	 0x138
#define F_MMU_TBWALK_FAULT_VA_MSK_SEC   F_MSK(31, 12)
#define F_MMU_TBWALK_FAULT_LAYER_SEC(regval) F_MSK_SHIFT(regval, 0, 0)

#define REG_MMU_FAULT_VA_SEC(mmu)	 (0x13c+((mmu)<<3))
#define F_MMU_FAULT_VA_MSK_SEC	F_MSK(31, 12)
#define F_MMU_FAULT_VA_WRITE_BIT_SEC    F_BIT_SET(1)
#define F_MMU_FAULT_VA_LAYER_BIT_SEC    F_BIT_SET(0)

#define REG_MMU_INVLD_PA_SEC(mmu)	 (0x140+((mmu)<<3))
#define REG_MMU_INT_ID_SEC(mmu)	     (0x150+((mmu)<<2))
/* only for MM iommu bit[11:2] */
#define F_MMU0_INT_ID_TF_MSK_SEC	F_MSK(11, 2)

/* ================================================================ */
/* SMI larb */
/* ================================================================ */

#define SMI_LARB_NON_SEC_CONx(larb_port)	(0x380 + ((larb_port)<<2))
#define F_SMI_NON_SEC_MMU_EN(en)			F_BIT_VAL(en, 0)
#define F_SMI_MMU_EN						F_BIT_SET(0)

#define SMI_LARB_SEC_CONx(larb_port)	(0xf80 + ((larb_port)<<2))
#define F_SMI_SEC_MMU_EN(en)			F_BIT_VAL(en, 0)
#define F_SMI_SEC_EN(sec)				F_BIT_VAL(sec, 1)
#define F_SMI_DOMN(domain)				F_VAL(domain, 8, 4)

int32_t m4u_dump_secure_reg(uint64_t m4u_id, uint32_t *val);
void m4u_security_debug_en(void);
void m4u_bank1_4_tf_en(void);
void m4u_dump_bank1_4_info(void);

#endif

