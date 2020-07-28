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
#ifndef _TL_PINPAD_API_H_
#define _TL_PINPAD_API_H_

#include "tci.h"
#include "tlPinpadLayout.h"

/**
 * Command ID's for communication Trusted Application Connector -> Trusted Application.
 */
#define CMD_ID_PINPAD_DRV       0  /** No Tlc interaction, but notification from TUI driver */
#define CMD_ID_PINPAD_GET_RES   1  /** Get screen resolution and informations */
#define CMD_ID_PINPAD_PROVISION 2  /** Provision images to Trusted Application container */
#define CMD_ID_PINPAD_LAUNCH    3  /** Show pinpad and get four digits */

/**
 * Response ID's for verify pin "launch" command.
 */
#define RES_ID_PINPAD_PIN_OK           1 /** PIN is correct */
#define RES_ID_PINPAD_PIN_NOK          2 /** PIN is wrong */
#define RES_ID_PINPAD_CANCELLED        3 /** TUI Cancelled */

/**
 * Return codes
 */
#define RET_ERR_PINPAD_INVALID_LENGTH   RET_CUSTOM_START + 1
#define RET_ERR_PINPAD_WRAP             RET_CUSTOM_START + 2
#define RET_ERR_PINPAD_UNWRAP           RET_CUSTOM_START + 3
#define RET_ERR_PINPAD_NO_RESOURCE      RET_CUSTOM_START + 4
#define RET_ERR_PINPAD_IN_PROGRESS      RET_CUSTOM_START + 5

/*
 * Termination codes
 */
#define EXIT_ERROR                      ((uint32_t)(-1))

/**
 * Maximum data length, i.e. maximum size of all images combined is 303KB.
 */
#define MAX_DATA_LEN 1024 * 400

/**
 * Provision-image command message.
 * The PROVISION command will apply disguise data provided in the payload
 * buffer of the TCI.
 * Note:
 * This part is an insecure sample only. Data is transferred in clear text
 * to the Trusted Application.
 *
 * @param[in/out] len   Lenght of the data to process/secure object.
 * @param[in/out] data  Data to processed (clear text / secure object).
 */

/**
 * Launch command message.
 * The LAUNCH command will show pinpad and get four digits.
 * The data field in TCI contains 5 items as secure objects.
 * All of them will be unwrapped and used during the PIN verification.
 * Their types are:
 *   * 3 images: background, star, blank
 *   * Layout: sLayout_t
 *   * PIN: uint8_t[4]
 *
 * @param len Lenght of the data to process.
 * @param data Data to processed (secure objects).
 */
#define IDX_PINPAD_PINPAD     0
#define IDX_PINPAD_STAR       1
#define IDX_PINPAD_BLANK      2
#define IDX_PINPAD_0          3
#define IDX_PINPAD_1          4
#define IDX_PINPAD_2          5
#define IDX_PINPAD_3          6
#define IDX_PINPAD_4          7
#define IDX_PINPAD_5          8
#define IDX_PINPAD_6          9
#define IDX_PINPAD_7          10
#define IDX_PINPAD_8          11
#define IDX_PINPAD_9          12
#define IDX_CORRECT           13
#define IDX_CANCEL            14
#define IDX_VALID             15
#define IDX_PINPAD_0_PRESSED  16
#define IDX_PINPAD_1_PRESSED  17
#define IDX_PINPAD_2_PRESSED  18
#define IDX_PINPAD_3_PRESSED  19
#define IDX_PINPAD_4_PRESSED  20
#define IDX_PINPAD_5_PRESSED  21
#define IDX_PINPAD_6_PRESSED  22
#define IDX_PINPAD_7_PRESSED  23
#define IDX_PINPAD_8_PRESSED  24
#define IDX_PINPAD_9_PRESSED  25
#define IDX_CORRECT_PRESSED   26
#define IDX_CANCEL_PRESSED    27
#define IDX_VALID_PRESSED     28
#define IDX_PINPAD_LAYOUT     29
#define IDX_PINPAD_END        30

#define IDX_DELTA_PRESSED     (IDX_PINPAD_0_PRESSED - IDX_PINPAD_0)

typedef struct {
    uint32_t length;
    uint32_t offset;
} secureObjectInData_t;

typedef struct {
    tciCommandHeader_t  header;             /**< Command header */
    union {
        secureObjectInData_t objects[IDX_PINPAD_END];
    };
} cmdPinpad_t;


/**
 * Response structure Trusted Application -> Trusted Application Connector.
 */
typedef struct {
    tciResponseHeader_t header;             /**< Response header */
    union {
        struct {
            uint32_t            screenWidth;        /**< Width of screen */
            uint32_t            screenHeight;       /**< Height of screen */
        } getResolution;
        struct {
            uint8_t             res;                /**< Result of pinpad entry */
        } launch;
    };
} rspPinpad_t;

/**
 * TCI message data.
 */
typedef struct {
    cmdPinpad_t        cmdPinpad;
    rspPinpad_t        rspPinpad;
    uint32_t           len;                 /**< Length of data to process */
    uint8_t            data[MAX_DATA_LEN];  /**< In/Out Clear text / Cipher text */
} tciMessage_t;

#endif // _TL_PINPAD_API_H_
