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

#ifndef __TLAPIROT13_H__
#define __TLAPIROT13_H__

#include "TlApi/TlApiCommon.h"
#include "TlApi/TlApiError.h"

/*
  DCI
*/

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
/* ... add more error codes when needed */

/**
 * DCI command header.
 */
typedef struct{
    dciCommandId_t commandId; /**< Command ID */
} dciCommandHeader_t;

/**
 * DCI response header.
 */
typedef struct{
    dciResponseId_t     responseId; /**< Response ID (must be command ID | RSP_ID_MASK )*/
    dciReturnCode_t     returnCode; /**< Return code of command */
} dciResponseHeader_t;

/*
 * Command ID's
 */
#define CMD_ID_01       1
#define CMD_ID_02       2
#define CMD_ID_03       3
#define CMD_ID_04       4
#define CMD_ID_05       5

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

/* Marshaled function parameters.
 * structs and union of marshaling parameters via TlApi.
 *
 * @note The structs can NEVER be packed !
 * @note The structs can NOT used via sizeof(..) !
 */

 /*
  * Function id definitions
  */
/*
#define FID_DR_OPEN_SESSION     1
#define FID_DR_CLOSE_SESSION    2
#define FID_DR_INIT_DATA        3
#define FID_DR_EXECUTE          4
#define FID_DRV_ADD_FOO         5
#define FID_DRV_SUB_FOO         6
*/

#define CMD_TUI_TEST_OPEN_SESSION     1
#define CMD_TUI_TEST_CLOSE_SESSION    2
#define CMD_TUI_TEST_INIT_DATA        3
#define CMD_TUI_EXECUTE               4
#define CMD_TUI_ADD_FOO               5
#define CMD_TUI_SUB_FOO               6
/* .. add more when needed */

/* Marshaled function parameters.
 * structs and union of marshaling parameters via TlApi.
 *
 * @note The structs can NEVER be packed !
 * @note The structs can NOT used via sizeof(..) !
 */

typedef struct {
    uint32_t    commandId;
	uint32_t    x;
	uint32_t    y;
    uint32_t    result;
} tlApiTest_t, *tlApiTest_ptr;

/** Encode cleartext with rot13.
 *
 * @param encodeData Pointer to the tlApiEncodeRot13_t structure
 * @return TLAPI_OK if data has successfully been encrypted.
 */

_TLAPI_EXTERN_C tlApiResult_t tlApiTest(tlApiTest_ptr TestData);

/**
 * Driver UUID. Update accordingly after reserving UUID
 */
#define DR_TUI_TEST_UUID { { 7, 0xC, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } }

#endif // __TLAPIROT13_H__

