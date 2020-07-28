/*
 * Copyright (c) since 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MT_LPM_DISPATCH_H__
#define __MT_LPM_DISPATCH_H__

#include <stdint.h>
#include <arch_helpers.h>
#include <mt_lpm_smc.h>


typedef uint64_t (*mt_lpm_dispatch_fn)(uint64_t x1, uint64_t x2,
					uint64_t x3, uint64_t x4,
					void *cookie,
					void *handle,
					uint64_t flags);

#define invoke_mt_lpm_dispatch(id, x2, x3, x4,\
			cookie, handle, flags) ({\
	uint64_t res = 0;\
	if (IS_MT_LPM_SMC(id)) {\
		unsigned int user = 0;\
		user = MT_LPM_SMC_USER(id);\
		if (mt_dispatcher.enable & (1<<user)) {\
			res = mt_dispatcher.fn[user](\
					MT_LPM_SMC_USER_ID(id),\
					x2, x3, x4,\
					cookie,\
					handle,\
					flags);\
		} \
	} res; })


struct mt_dispatch_ctrl {
	unsigned int enable;
	mt_lpm_dispatch_fn fn[mt_lpm_smc_user_max];
};

void mt_lpm_dispatcher_registry(int id, mt_lpm_dispatch_fn fn);

extern struct mt_dispatch_ctrl mt_dispatcher;
#endif

