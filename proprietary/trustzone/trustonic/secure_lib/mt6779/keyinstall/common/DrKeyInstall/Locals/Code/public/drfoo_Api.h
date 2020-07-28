/*
 * Copyright (c) 2013 TRUSTONIC LIMITED
 * All rights reserved
 * 
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

/**
 * @file   drfoo_Api.h
 * @brief  Contains DCI command definitions and data structures
 *
 */

#ifndef __DRFOOAPI_H__
#define __DRFOOAPI_H__

#include "dci.h"

/*
 * Command ID's
 */
#define CMD_ID_01       1
#define CMD_ID_02       2
/*... add more command ids when needed */

/**
 * command message.
 *
 * @param len Lenght of the data to process.
 * @param data Data to be processed
 */
typedef struct {
    dciCommandHeader_t  header;     /**< Command header */
    uint32_t            len;        /**< Length of data to process */
} cmd_t;

/**
 * Response structure
 */
typedef struct {
    dciResponseHeader_t header;     /**< Response header */
    uint32_t            len;
} rsp_t;

/*
 * Sample 01 data structure
 */
typedef struct {
    uint32_t len;
    uint32_t addr;
} sample01_t;

/*
 * Sample 02 data structure
 */
typedef struct {
    uint32_t data;
} sample02_t;

/*
 * DCI message data.
 */
typedef struct {
    union {
        cmd_t     command;
        rsp_t     response;
    };

    union {
        sample01_t  sample01;
        sample02_t  sample02;
    };

} dciMessage_t;

/*
 * Driver UUID. Update accordingly after reserving UUID
 */
#define DRV_DBG_UUID { { 7, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } }

#endif // __DRFOOAPI_H__
