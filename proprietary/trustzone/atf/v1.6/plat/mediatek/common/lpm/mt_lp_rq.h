/*
 * Copyright (c) since 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MT_LP_RQ__
#define __MT_LP_RQ__

/* Determine the generic resource request public type */
#define MT_LP_RQ_XO_FPM		(1 << 0L)
#define MT_LP_RQ_26M		(1 << 1L)
#define MT_LP_RQ_INFRA		(1 << 2L)
#define MT_LP_RQ_SYSPLL		(1 << 3L)
#define MT_LP_RQ_DRAM		(1 << 4L)
#define MT_LP_RQ_ALL		(0xFFFFFFFF)

struct mt_lp_resource_user {
	/* Determine the resource user mask */
	unsigned int umask;

	/* Determine the resource request user identify */
	unsigned int uid;

	/* Request the resource */
	int (*request)(struct mt_lp_resource_user *this,
		       unsigned int resource);

	/* Release the resource */
	int (*release)(struct mt_lp_resource_user *this);
};

int mt_lp_resource_user_register(char *uname, struct mt_lp_resource_user *ru);

#endif
