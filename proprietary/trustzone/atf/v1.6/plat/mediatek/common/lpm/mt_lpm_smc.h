/*
 * Copyright (c) since 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MT_LPM_SMC_H__
#define __MT_LPM_SMC_H__

/*
 * mtk lpm smc user format as bellow
 * bit[31 ~ 24]: magic number
 * bit[23 ~ 16]: user number
 * bit[15 ~ 00]: user id
 */

#define MT_LPM_SMC_MAGIC		0xDA000000
#define MT_LPM_SMC_USER_MASK		0xff
#define MT_LPM_SMC_USER_SHIFT		16

#define MT_LPM_SMC_USER_ID_MASK		0x0000ffff

enum mt_lpm_smc_user_id {
	mt_lpm_smc_user_cpu_pm = 0,
	mt_lpm_smc_user_spm_dbg,
	mt_lpm_smc_user_spm,
	mt_lpm_smc_user_max,
};


#define IS_MT_LPM_SMC(smcid)\
	((smcid & MT_LPM_SMC_MAGIC) == MT_LPM_SMC_MAGIC)


/* get real user id */
#define MT_LPM_SMC_USER(id) \
	((id >> MT_LPM_SMC_USER_SHIFT)\
			& MT_LPM_SMC_USER_MASK)


#define MT_LPM_SMC_USER_ID(uid)\
	(uid & MT_LPM_SMC_USER_ID_MASK)



/* sink user id to smc's user id */
#define MT_LPM_SMC_USER_SINK(user, uid) \
	(((uid & MT_LPM_SMC_USER_ID_MASK)\
		| ((user & MT_LPM_SMC_USER_MASK)\
			<< MT_LPM_SMC_USER_SHIFT))\
	| MT_LPM_SMC_MAGIC)



/* sink cpu pm's smc id */
#define MT_LPM_SMC_USER_ID_CPU_PM(uid) \
	MT_LPM_SMC_USER_SINK(mt_lpm_smc_user_cpu_pm, uid)
/* sink spm's smc id */
#define MT_LPM_SMC_USER_ID_SPM(uid) \
	MT_LPM_SMC_USER_SINK(mt_lpm_smc_user_spm, uid)



/* sink cpu pm's user id */
#define MT_LPM_SMC_USER_CPU_PM(uid)\
			MT_LPM_SMC_USER_ID_CPU_PM(uid)

/* sink spm's user id */
#define MT_LPM_SMC_USER_SPM(uid)\
			MT_LPM_SMC_USER_ID_SPM(uid)


/* behavior */
#define MT_LPM_SMC_ACT_SET		(1<<0UL)
#define MT_LPM_SMC_ACT_CLR		(1<<1UL)
#define MT_LPM_SMC_ACT_GET		(1<<2UL)
/* compatible action for legacy smc from lk */
#define MT_LPM_SMC_ACT_COMPAT		(1<<7UL)

enum MT_LPM_SPMC_COMPAT_ID {
	MT_LPM_SPMC_COMPAT_LK_FW_INIT,
	MT_LPM_SPMC_COMPAT_LK_MCDI_WDT_DUMP,
};

#endif

