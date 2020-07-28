/*
 * Copyright (c) 2013-2015 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

#ifndef _SOTER_OEM_API_H_
#define _SOTER_OEM_API_H_

#ifdef SOTER
#include "mtk_keymaster_defs.h"
#else
#include <hardware/keymaster_defs.h>
#endif

/* Used for OEM customers to customize parameters */
#define MAX_SOTER_EXPORT_LEN    (4 * 1024) 	//4K
#define OTHERFILEDS_LENGTH      256 		//define for JSON format usage
#define DECIMAL                 10
#define HEXDECIMAL              16
#define SOTER_KEY_SIZE			2048

const char X509_pub_key_header[] = "-----BEGIN PUBLIC KEY-----\n";
const char X509_pub_key_tail[] = "\n-----END PUBLIC KEY-----";

#endif	// _SOTER_OEM_API_H_
