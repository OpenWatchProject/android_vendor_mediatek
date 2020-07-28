/*
 * Copyright (c) 2015-2017 MICROTRUST Incorporated
 * All rights reserved
 *
 * This file and software is confidential and proprietary to MICROTRUST Inc.
 * Unauthorized copying of this file and software is strictly prohibited.
 * You MUST NOT disclose this file and software unless you get a license
 * agreement from MICROTRUST Incorporated.
 */

#ifndef __UT_KM_TAC_H__
#define __UT_KM_TAC_H__

__BEGIN_DECLS


int ut_km_import_google_key(unsigned char *data, unsigned int datalen);
int ut_km_check_google_key(void);

__END_DECLS
#endif //__UT_KM_SETKEY_H__
