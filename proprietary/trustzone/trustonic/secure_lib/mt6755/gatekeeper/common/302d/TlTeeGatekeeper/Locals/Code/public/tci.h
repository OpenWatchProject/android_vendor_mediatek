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
#ifndef __tci_h__
#define __tci_h__

#include "def.h"
#include "mcSo.h"

typedef uint32_t tciCommandId_t;
typedef uint32_t tciResponseId_t;

/**< Responses have bit 31 set */
#define RSP_ID_MASK (1U << 31)
#define RSP_ID(cmdId) (((uint32_t)(cmdId)) | RSP_ID_MASK)
#define IS_CMD(cmdId) ((((uint32_t)(cmdId)) & RSP_ID_MASK) == 0)
#define IS_RSP(cmdId) ((((uint32_t)(cmdId)) & RSP_ID_MASK) == RSP_ID_MASK)

#define ENC_PASSWORD_KEY_LEN    MC_SO_SIZE(0, PASSWORD_KEY_LEN)
#define ENC_AUTH_TOKEN_KEY_LEN  MC_SO_SIZE(0, PERBOOT_HMAC_KEY_LEN)
#define ENC_FAILURE_RECORD_LEN  MC_SO_SIZE(0, sizeof(tee_failure_record_t))
#define TEE_FAILURE_RECORD_VERSION 1

typedef enum
{
    /* Codes returned by both NW or SW apps */
    TEE_OK                   = 0,
    TEE_ERR_UNKONWN_ERROR,
    TEE_ERR_INVALID_INPUT,

    /* Codes returned by NW TA */
    TEE_ERR_FAIL             = 100,
    TEE_ERR_BUFFER_TOO_SMALL,
    TEE_ERR_NOT_IMPLEMENTED,
    TEE_ERR_SESSION,
    TEE_ERR_MC_DEVICE,
    TEE_ERR_NOTIFICATION,
    TEE_ERR_MAP,
    TEE_ERR_UNMAP,
    TEE_ERROR_SECURE_HW_COMMUNICATION_FAILED,
    TEE_ERR_INVALID_SIGN_LENGTH,

    /* Codes returned by SW TA */
    TEE_ERR_UNKNOWN_CMD      = 500,
    TEE_ERR_SECURE_OBJECT,
    TEE_ERR_KEY_DERIVATION,
    TEE_ERR_MEMORY_ALLOCATION_FAILED,
    TEE_ERR_RANDOM_GENERATION,
    TEE_ERR_INTERNAL_ERROR

} TEE_ReturnCode_t;

typedef uint8_t perboot_hmac_key_t[PERBOOT_HMAC_KEY_LEN];

/**
 * Enumerates types of a key gatekeeper TA returns
 */
typedef enum
{
    TEE_GK_Key_Password = 0,
    TEE_GK_Key_Auth,

    TEE_GK_Key_Max
} TEE_GK_Key_t;

/***
 * Used for sending blob bewtween NWd and SWd
 */
typedef struct {
    uint32_t data;          /* 32 bits secure address */
    uint32_t data_length;   /* data length */
} data_blob_t;

/**
 * TCI command header.
 */
typedef struct{
    tciCommandId_t commandId; /**< Command ID */
} tciCommandHeader_t;


/**
 * TCI response header.
 */
typedef struct{
    tciResponseId_t     responseId; /**< Response ID (must be command ID | RSP_ID_MASK )*/
    TEE_ReturnCode_t    returnCode; /**< Return code of command */
} tciResponseHeader_t;

/**
 * Command message.
 *
 * @param len Length of the data to proces,s.
 * @param data Data to be processed
 */
typedef struct {
    tciCommandHeader_t header; /**< Command header */
    uint32_t len; /**< Length of data to process */
} command_t;

/**
 * Response structure
 */
typedef struct {
    tciResponseHeader_t header; /**< Response header */
    uint32_t len;
} response_t;

/**
 * Data used for computing password hash
 */
typedef struct {
    data_blob_t     hmac;
    data_blob_t     data;
    data_blob_t     key;
    uint64_t        salt;
} signature_t;

/**
 * Structure used to convey failure records between NWd and SWd
 */
typedef struct __attribute__((packed)) {
    uint8_t         version; // Version field always as first byte of struct
    uint64_t        suid;
    uint64_t        last_access_timestamp;
    uint32_t        failure_counter;
    data_blob_t     encrypted_failure_record;
} tee_failure_record_t;

/**
 * TCI message data.
 */
typedef struct {
    union {
        command_t command;
        response_t response;
    };

    union {
        uint64_t                uptime;
        signature_t             signature;
        tee_failure_record_t    failure_record;
        data_blob_t             random;
        data_blob_t             symmetric_key;
    };
} tciMessage_t, *tciMessage_ptr;

/**
 * Overall TCI structure.
 */
typedef struct {
    tciMessage_t message; /**< TCI message */
} tci_t;

#endif // __tci_h__
