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

#ifndef __PLAT_DCM_H__
#define __PLAT_DCM_H__

extern uint64_t plat_dcm_mcsi_a_addr;
extern uint32_t plat_dcm_mcsi_a_val;
extern int plat_dcm_initiated;

extern void plat_dcm_mcsi_a_backup(void);
extern void plat_dcm_mcsi_a_restore(void);
extern void plat_dcm_rgu_enable(void);
extern void plat_dcm_msg_handler(uint64_t x1);

#define RGU_DCM_ENABLE ((0x1 << 0))

#define ALL_DCM_TYPE (ARMCORE_DCM_TYPE | MCUSYS_DCM_TYPE \
			| STALL_DCM_TYPE | BIG_CORE_DCM_TYPE \
			| GIC_SYNC_DCM_TYPE | RGU_DCM_TYPE \
			| INFRA_DCM_TYPE | PERI_DCM_TYPE \
			| DDRPHY_DCM_TYPE | EMI_DCM_TYPE | DRAMC_DCM_TYPE)

enum {
	ARMCORE_DCM_TYPE	= (1U << 0),
	MCUSYS_DCM_TYPE		= (1U << 1),
	INFRA_DCM_TYPE		= (1U << 2),
	PERI_DCM_TYPE		= (1U << 3),
	EMI_DCM_TYPE		= (1U << 4),
	DRAMC_DCM_TYPE		= (1U << 5),
	DDRPHY_DCM_TYPE		= (1U << 6),
	STALL_DCM_TYPE		= (1U << 7),
	BIG_CORE_DCM_TYPE	= (1U << 8),
	GIC_SYNC_DCM_TYPE	= (1U << 9),
	LAST_CORE_DCM_TYPE	= (1U << 10),
	RGU_DCM_TYPE		= (1U << 11),
	TOPCKG_DCM_TYPE		= (1U << 12),
	LPDMA_DCM_TYPE		= (1U << 13),
	NR_DCM_TYPE = 14,
};

#endif
