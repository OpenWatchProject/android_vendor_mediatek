/*
 * Copyright (c) 2014-2015, ARM Limited and Contributors. All rights reserved.
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

#ifndef __PLAT_PM_H__
#define __PLAT_PM_H__

#ifndef __ASSEMBLY__
extern void gic_cpuif_init(void);
extern void gic_rdist_save(void);
extern void gic_rdist_restore(void);
extern void gic_rdist_restore_all(void);
extern void gic_cpuif_deactivate(unsigned int gicc_base);
extern void gic_dist_save(void);
extern void gic_dist_restore(void);
extern void gic_setup(void);
extern void gic_sgi_save_all(void);
extern void gic_sgi_restore_all(void);
extern void gic_sync_dcm_enable(void);
extern void gic_sync_dcm_disable(void);

extern void dfd_resume(void);

void mtk_suspend_footprint_log(int idx);
void mtk_suspend_timestamp_log(int idx);

enum mtk_suspend_mode {
	MTK_MCDI_MODE = 1,
	MTK_SODI_MODE,
	MTK_SODI3_MODE,
	MTK_DPIDLE_MODE,
	MTK_SUSPEND_MODE,
};
#endif

#define footprint_addr(cpu)	(mtk_suspend_footprint_addr + (cpu << 2))
#define timestamp_addr(cpu,idx)	(mtk_suspend_timestamp_addr + ((cpu * 16 + idx) << 3))

#define MTK_SUSPEND_FOOTPRINT_ENTER_CPUIDLE		0
#define MTK_SUSPEND_FOOTPRINT_BEFORE_ATF		1
#define MTK_SUSPEND_FOOTPRINT_ENTER_ATF			2
#define MTK_SUSPEND_FOOTPRINT_RESERVE_P1		3
#define MTK_SUSPEND_FOOTPRINT_RESERVE_P2		4
#define MTK_SUSPEND_FOOTPRINT_ENTER_SPM_SUSPEND		5
#define MTK_SUSPEND_FOOTPRINT_LEAVE_SPM_SUSPEND		6
#define MTK_SUSPEND_FOOTPRINT_BEFORE_WFI		7
#define MTK_SUSPEND_FOOTPRINT_AFTER_WFI			8
#define MTK_SUSPEND_FOOTPRINT_BEFORE_MMU		9
#define MTK_SUSPEND_FOOTPRINT_AFTER_MMU			10
#define MTK_SUSPEND_FOOTPRINT_ENTER_SPM_SUSPEND_FINISH	11
#define MTK_SUSPEND_FOOTPRINT_LEAVE_SPM_SUSPEND_FINISH	12
#define MTK_SUSPEND_FOOTPRINT_LEAVE_ATF			13
#define MTK_SUSPEND_FOOTPRINT_AFTER_ATF			14
#define MTK_SUSPEND_FOOTPRINT_LEAVE_CPUIDLE		15

#define MTK_SUSPEND_TIMESTAMP_ENTER_CPUIDLE		0
#define MTK_SUSPEND_TIMESTAMP_BEFORE_ATF		1
#define MTK_SUSPEND_TIMESTAMP_ENTER_ATF			2
#define MTK_SUSPEND_TIMESTAMP_BEFORE_L2_FLUSH		3
#define MTK_SUSPEND_TIMESTAMP_AFTER_L2_FLUSH		4
#define MTK_SUSPEND_TIMESTAMP_ENTER_SPM_SUSPEND		5
#define MTK_SUSPEND_TIMESTAMP_LEAVE_SPM_SUSPEND		6
#define MTK_SUSPEND_TIMESTAMP_GIC_P1			7
#define MTK_SUSPEND_TIMESTAMP_GIC_P2			8
#define MTK_SUSPEND_TIMESTAMP_BEFORE_WFI		9
#define MTK_SUSPEND_TIMESTAMP_AFTER_WFI			10
#define MTK_SUSPEND_TIMESTAMP_RESERVE_P1		11
#define MTK_SUSPEND_TIMESTAMP_RESERVE_P2		12
#define MTK_SUSPEND_TIMESTAMP_GIC_P3			13
#define MTK_SUSPEND_TIMESTAMP_GIC_P4			14
#define MTK_SUSPEND_TIMESTAMP_ENTER_SPM_SUSPEND_FINISH	15
#define MTK_SUSPEND_TIMESTAMP_LEAVE_SPM_SUSPEND_FINISH	16
#define MTK_SUSPEND_TIMESTAMP_LEAVE_ATF			17
#define MTK_SUSPEND_TIMESTAMP_AFTER_ATF			18
#define MTK_SUSPEND_TIMESTAMP_LEAVE_CPUIDLE		19
#define MTK_SUSPEND_TIMESTAMP_MAX			20

#endif /* __PLAT_PM_H__ */
