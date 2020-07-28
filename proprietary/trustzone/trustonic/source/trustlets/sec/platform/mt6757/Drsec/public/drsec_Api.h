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
 * @file   drDapc_Api.h
 * @brief  Contains DCI command definitions and data structures
 *
 */

#ifndef __DR_M_SEC_DRV_API_H__
#define __DR_M_SEC_DRV_API_H__

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
 * DCI message data.
 */
typedef struct {
    union {
        cmd_t     command;
        rsp_t     response;
    };
    uint32_t index;
    uint32_t result;
} dciMessage_t;

/*
 * Driver UUID. Update accordingly after reserving UUID
 */
#define DRV_DBG_UUID { { 0x5, 0x12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } }

#endif // __DR_M_SEC_DRV_API_H__