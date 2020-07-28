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

#ifndef _SOTER_VENDEORAPI_H_
#define _SOTER_VENDEORAPI_H_

/* Used for OEM customers to customize parameters */
#define MAX_SOTER_EXPORT_LEN    (4 * 1024) 	//4K
#define OTHERFILEDS_LENGTH      256 		//define for JSON format usage
#define DECIMAL                 10
#define HEXDECIMAL              16

/* Table of counter */
struct counter_table {
    uint32_t uid;
    uint32_t value;
};

/* RPMB related interface */

/**
*   Check for whether ATTK exists or not.
*
*   @return	0 for exists and digest is correct; -1 for not exists and -2 for exists but digest is incorrect.
*/

int soter_attk_exist(void);

/**
 *   Mark next ATTK as current one.
 *
 *   @return	true for success or false for fail.
 */

bool soter_attk_use_next(void);

/**
 *   Read ATTK private key data.
 *
 *   @param [out] 	key_data	ATTK data
 *   @param [out]	key_data_len	ATTK data length
 *
 *   @return	0 for success and non-zero for failure
 */

uint32_t soter_attk_read(uint8_t *key_data, uint32_t *key_data_len, uint8_t **hash);

/**
 *   Write ATTK private key data.
 *
 *   @param [in]        key_data        ATTK data
 *   @param [in]        key_data_len    ATTK data length
 *   @copy_num [in]	copy numbers of ATTK
 *
 *   @return     0 for success and non-zero for failure
 */

uint32_t soter_attk_write(uint8_t *key_data, uint32_t key_data_len, uint8_t *hash, uint32_t copy_num);

/**
*   Counter read function.
*
*   @param [out]        counter value
*
*   @return     0 for success and non-zero for failure
*/

uint32_t read_counter(uint32_t *counter);

uint32_t write_counter(uint32_t value, bool add);

#endif	// _SOTER_VENDEORAPI_H_