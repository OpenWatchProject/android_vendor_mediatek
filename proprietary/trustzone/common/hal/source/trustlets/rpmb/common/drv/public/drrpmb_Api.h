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
 * @file   drrpmb_Api.h
 * @brief  Contains DCI command definitions and data structures
 *
 */

#ifndef __DRRPMBAPI_H__
#define __DRRPMBAPI_H__

typedef uint32_t dciCommandId_t;
typedef uint32_t dciResponseId_t;
typedef uint32_t dciReturnCode_t;

/**< Responses have bit 31 set */
#define RSP_ID_MASK (1U << 31)
#define RSP_ID(cmdId) (((uint32_t)(cmdId)) | RSP_ID_MASK)
#define IS_CMD(cmdId) ((((uint32_t)(cmdId)) & RSP_ID_MASK) == 0)
#define IS_RSP(cmdId) ((((uint32_t)(cmdId)) & RSP_ID_MASK) == RSP_ID_MASK)

/**
 * Return codes of driver commands.
 */
#define RET_OK                    0
#define RET_ERR_UNKNOWN_CMD       1
#define RET_ERR_NOT_SUPPORTED     2
#define RET_ERR_INTERNAL_ERROR    3
#define DCI_RPMB_CMD_PROGRAM_KEY  4
/* ... add more error codes when needed */

/**
 * DCI command header.
 */
typedef struct {
    dciCommandId_t commandId; /**< Command ID */
} dciCommandHeader_t;

/**
 * DCI response header.
 */
typedef struct {
    dciResponseId_t     responseId; /**< Response ID (must be command ID | RSP_ID_MASK )*/
    dciReturnCode_t     returnCode; /**< Return code of command */
} dciResponseHeader_t;


/*
 * Command ID's
 */
#define DCI_RPMB_CMD_READ_DATA      1
#define DCI_RPMB_CMD_GET_WCNT       2
#define DCI_RPMB_CMD_WRITE_DATA     3


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
 * Alternative access flow to improve performance. (this is customization)
 */
#define RPMB_MULTI_BLOCK_ACCESS 1

#if RPMB_MULTI_BLOCK_ACCESS
#define MAX_RPMB_TRANSFER_BLK 16
#define MAX_RPMB_REQUEST_SIZE 512*MAX_RPMB_TRANSFER_BLK //8KB(16blks) per requests.
#else
#define MAX_RPMB_TRANSFER_BLK 1
#define MAX_RPMB_REQUEST_SIZE 512*MAX_RPMB_TRANSFER_BLK //512B(1blks) per requests.
#endif

typedef struct {
    uint8_t frame[MAX_RPMB_REQUEST_SIZE];
    uint32_t frameLen;
    uint16_t type;
    uint16_t addr;
    uint16_t blks;
    uint16_t result;
} rpmb_req_t;

/*
 * DCI message data.
 */
typedef struct {
    union {
        cmd_t     command;
        rsp_t     response;
    };

    rpmb_req_t    request;

} dciMessage_t;

/*
 * Driver UUID. Update accordingly after reserving UUID
 */
#define DRV_DBG_UUID { { 7, 0x15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } }

#endif // __DRRPMBAPI_H__
