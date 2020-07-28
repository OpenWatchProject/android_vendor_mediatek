/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
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

#define ALL_DCM_TYPE	(ARMCORE_DCM_TYPE | MCUSYS_DCM_TYPE \
			 | INFRA_DCM_TYPE \
			 | EMI_DCM_TYPE | DRAMC_DCM_TYPE \
			 )

enum {
	ARMCORE_DCM = 0,
	MCUSYS_DCM,
	INFRA_DCM,
	PERI_DCM,
	EMI_DCM,
	DRAMC_DCM,
	DDRPHY_DCM,
	STALL_DCM,
	BIG_CORE_DCM,
	GIC_SYNC_DCM,	/* WE2 */
	LAST_CORE_DCM,	/* WE2 */
	RGU_DCM,	/* WE2 */
	TOPCKG_DCM,
	LPDMA_DCM,
	NR_DCM,
};

enum {
	ARMCORE_DCM_TYPE	= (1U << ARMCORE_DCM),
	MCUSYS_DCM_TYPE		= (1U << MCUSYS_DCM),
	INFRA_DCM_TYPE		= (1U << INFRA_DCM),
	PERI_DCM_TYPE		= (1U << PERI_DCM),
	EMI_DCM_TYPE		= (1U << EMI_DCM),
	DRAMC_DCM_TYPE		= (1U << DRAMC_DCM),
	DDRPHY_DCM_TYPE		= (1U << DDRPHY_DCM),
	STALL_DCM_TYPE		= (1U << STALL_DCM),
	BIG_CORE_DCM_TYPE	= (1U << BIG_CORE_DCM),
	GIC_SYNC_DCM_TYPE	= (1U << GIC_SYNC_DCM),
	LAST_CORE_DCM_TYPE	= (1U << LAST_CORE_DCM),
	RGU_DCM_TYPE		= (1U << RGU_DCM),
	TOPCKG_DCM_TYPE		= (1U << TOPCKG_DCM),
	LPDMA_DCM_TYPE		= (1U << LPDMA_DCM),
	NR_DCM_TYPE = NR_DCM,
};

extern void dcm_restore_cluster_on(unsigned long mpidr);
extern void dcm_restore_mcusys_on(void);
extern void dcm_msg_handler(uint64_t init_type);
extern unsigned long dcm_get_enabled_cnt(uint64_t type);
extern void dcm_init(void);

#endif
