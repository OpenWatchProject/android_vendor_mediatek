/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __TEE_H__
#define __TEE_H__

#define TEE_OP_ID_NONE			(0xFFFF0000)
#define TEE_OP_ID_SET_PENDING		(0xFFFF0001)

/*
 * SiP wrapper to set tee_sanity intr pending.
 *
 * For op_id: TEE_OP_ID_SET_PENDING used
 */
void sip_tee_set_pending(void);

#endif /* __TEE_H__ */

