/*
 * Copyright (c) 2015-2017 MICROTRUST Incorporated
 * All rights reserved
 *
 * This file and software is confidential and proprietary to MICROTRUST Inc.
 * Unauthorized copying of this file and software is strictly prohibited.
 * You MUST NOT disclose this file and software unless you get a license
 * agreement from MICROTRUST Incorporated.
 */

#ifndef __UT_KEYMASTER_API_H__
#define __UT_KEYMASTER_API_H__

__BEGIN_DECLS

#include <stdbool.h>
#include <stdint.h>

#include <hardware/keymaster_defs.h>


keymaster_error_t ut_ree_import_attest_keybox(const unsigned char* peakb,
											const unsigned int peakb_len);


__END_DECLS

#endif //__UT_KEYMASTER_API_H__
