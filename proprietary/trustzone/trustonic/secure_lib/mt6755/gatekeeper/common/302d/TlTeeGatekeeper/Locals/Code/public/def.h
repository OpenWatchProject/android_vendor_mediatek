/*
 * Copyright (c) 2015 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

#ifndef _def_H_
#define _def_H_

/**
 * Trustlet UUID
 */
#define TEE_GATEKEEPER_TL_UUID { { 7, 6, 0x10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } }

/**
 * Length of the password returned by GetPasswordKey() function
 */
#define PASSWORD_KEY_LEN 1024

/**
 * Length of the data that represents per boot unique key and lock status
 */
#define PERBOOT_HMAC_KEY_LEN 32

/**
 * Command ID's
 */
#define CMD_ID_TEE_MILLI_SINCE_BOOT           1
#define CMD_ID_TEE_GET_PASSWORD_KEY           2
#define CMD_ID_TEE_GET_RANDOM                 3
#define CMD_ID_TEE_COMPUTE_PASS_SIGNATURE     4
#define CMD_ID_TEE_GET_AUTH_TOKEN_KEY         5
#define CMD_ID_TEE_FAILURE_RECORD_ENCODE      6
#define CMD_ID_TEE_FAILURE_RECORD_DECODE      7
#define CMD_ID_TEE_FAILURE_RECORD_STORE       8
#define CMD_ID_TEE_FAILURE_RECORD_RETRIEVE    9


#endif /* _def_ */
