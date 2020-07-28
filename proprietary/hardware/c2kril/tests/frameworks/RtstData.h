/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
#ifndef __RTST_DATA_H__
#define __RTST_DATA_H__
/*****************************************************************************
 * Include
 *****************************************************************************/
#include <Parcel.h>
#include <String8.h>
#include <Vector.h>
#include <gtest/gtest.h>
#include <telephony/mtk_ril.h>
#include "RtstLog.h"
#include "RtstDefs.h"

/*****************************************************************************
 * Name Space
 *****************************************************************************/
using ::android::String8;
using ::android::Parcel;
using ::android::Vector;
using ::testing::AssertionResult;
using ::testing::AssertionSuccess;
using ::testing::AssertionFailure;

/*****************************************************************************
 * Define
 *****************************************************************************/
#define RTST_TAG "RTF"


/*
 * Data Type Supported by RIL test framework
 */
#define RTST_INT8  "int8"
#define RTST_INT32  "int32"
#define RTST_INT64  "int64"
#define RTST_VOID   "void"
#define RTST_STRING "string"
#define RTST_RAW    "raw"

/*
 * Macro to mark the test case begin
 *
 * EXAMPLE:
 * <code>
 *     TEST(MyTestCase, MyTest) {
 *         RTST_CASE_BEGIN();
 *         .... // Other RTST_XXX to write here
 *         RTST_CASE_END();
 *     }
 * </code>
 *
 * SEE ALSO: RTST_CASE_BEGIN_EX, RTST_CASE_END
 */
#define RTST_CASE_BEGIN() \
        _RTST_CASE_BEGIN(ril, false, true)


/*
 * Macro to mark the test case begin with extra two parameters
 *
 * PARAMTERS:
 *
 *  _rfx_assert:
 *      if true, we will use RTST_ASSERT
 *      if false, use gtest ASSERT
 *
 *  _step_test:
 *      if true, we will trigger the test in each RTST_XXX
 *      if false, we will triger the test at RTST_CASE_END
 *
 * EXAMPLE:
 * <code>
 *     TEST(MyTestCase, MyTest) {
 *         RTST_CASE_BEGIN_EX(true, true);
 *         .... // Other RTST_XXX to write here
 *         RTST_CASE_END();
 *     }
 * </code>
 *
 * SEE ALSO: RTST_CASE_BEGIN, RTST_CASE_END
 */
#define RTST_CASE_BEGIN_EX(_rfx_assert, _step_test)                  \
        _RTST_CASE_BEGIN(ril, _rfx_assert, _step_test)


/*
 * Macro to mark the test case end
 *
 * EXAMPLE:
 * <code>
 *     TEST(MyTestCase, MyTest) {
 *         RTST_CASE_BEGIN();
 *         .... // Other RTST_XXX to write here
 *         RTST_CASE_END();
 *     }
 * </code>
 *
 * SEE ALSO: RTST_CASE_BEGIN_EX, RTST_CASE_BEGIN
 */
#define RTST_CASE_END() _RTST_CASE_END(ril)


/*
 * Macro to mock the ril request data
 *
 * PARAMTERS:
 *
 *  _reqId:
 *     The RIL request ID
 *  _dataNum:
 *     The parameter number
 *  ...:
 *     Parameter list, parameter consist of by {type, value}
 *     For the type, see RTST_INT32, RTST_INT64...
 *     For the value, we use string to describe
 *         RTST_INT32  => decimal, such as "4096" or "125"
 *         RTST_INT64  => deciaml, such as "4294967296"
 *         RTST_VOID   => suchas ""
 *         RTST_STRING => such as "AT+ECSCB=0"
 *         RTST_RAW    => hex format, such as "45defe78"
 *
 *
 * EXAMPLE:
 * <code>
 *     TEST(MyTestCase, MyTest) {
 *         RTST_CASE_BEGIN();
 *         RTST_RIL_REQUEST(RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION, 1, RTST_INT32, "0");
 *         ...
 *         RTST_CASE_END();
 *     }
 * </code>
 *
 * SEE ALSO: RTST_RIL_REQUEST_WITH_SLOT, RTST_RIL_VOID_REQUEST RTST_RIL_REQUEST_NC
 *           RTST_RIL_VOID_REQUEST_WITH_SLOT_ID, RTST_RIL_REQUEST_WITH_SLOT_NC
 */
#define RTST_RIL_REQUEST(_reqId, _dataNum, ...)  \
        RTST_RIL_REQUEST_WITH_SLOT(_reqId, RTST_SLOT_ID_0, _dataNum, __VA_ARGS__)

/*
 * The difference between RTST_RIL_REQUEST_NC and RTST_RIL_REQUEST is
 *    The RTST_RIL_REQUEST may add a data len info before the data
 *    according the following rule
 *         1. All data type are INT32s, will add the number of INT32
 *         2. All data type are STRING, and the number >= 2, will add the number of STRING
 *    The RTST_RIL_REQUEST_NC will not add the data len
 *
 * SEE ALSO: RTST_RIL_REQUEST_WITH_SLOT, RTST_RIL_VOID_REQUEST RTST_RIL_REQUEST
 *           RTST_RIL_VOID_REQUEST_WITH_SLOT_ID, RTST_RIL_REQUEST_WITH_SLOT_NC
 */
#define RTST_RIL_REQUEST_NC(_reqId, _dataNum, ...)  \
        RTST_RIL_REQUEST_WITH_SLOT_NC(_reqId, RTST_SLOT_ID_0, _dataNum, __VA_ARGS__)


/*
 * Macro to mock the ril request data with slot ID
 *
 * PARAMTERS:
 *
 *  _reqId:
 *     The RIL request ID
 *  _slotId:
 *     The slot ID
 *  _dataNum:
 *     The parameter number
 *  ...:
 *     Parameter list, parameter consist of by {type, value}
 *     For the type, see RTST_INT32, RTST_INT64...
 *     For the value, we use string to describe
 *         RTST_INT32  => decimal, such as "4096" or "125"
 *         RTST_INT64  => deciaml, such as "4294967296"
 *         RTST_VOID   => suchas ""
 *         RTST_STRING => such as "AT+ECSCB=0"
 *         RTST_RAW    => hex format, such as "45defe78"
 *
 *
 * EXAMPLE:
 * <code>
 *     TEST(MyTestCase, MyTest) {
 *         RTST_CASE_BEGIN();
 *         RTST_RIL_REQUEST_WITH_SLOT(
 *                 RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION,
 *                 RTST_SLOT_ID_1, 1, RTST_INT32, "0");
 *         ...
 *         RTST_CASE_END();
 *     }
 * </code>
 *
 * SEE ALSO: RTST_RIL_REQUEST, RTST_RIL_VOID_REQUEST, RTST_RIL_REQUEST_WITH_SLOT_NC
 *           RTST_RIL_VOID_REQUEST_WITH_SLOT_ID, RTST_RIL_REQUEST_NC
 */
#define RTST_RIL_REQUEST_WITH_SLOT(_reqId, _slotId, _dataNum, ...)   \
        _RTST_RIL_REQUEST_WITH_SLOT(ril, _reqId, _slotId, _dataNum, __VA_ARGS__)

/*
 * The difference between RTST_RIL_REQUEST_WITH_SLOT and RTST_RIL_REQUEST_WITH_SLOT_NC is
 *    The RTST_RIL_REQUEST_WITH_SLOT may add a data len info before the data
 *    according the following rule
 *         1. All data type are INT32s, will add the number of INT32
 *         2. All data type are STRING, and the number >= 2, will add the number of STRING
 *    The RTST_RIL_REQUEST_WITH_SLOT_NC will not add the data len
 *
 * SEE ALSO: RTST_RIL_REQUEST, RTST_RIL_VOID_REQUEST, RTST_RIL_REQUEST_WITH_SLOT
 *           RTST_RIL_VOID_REQUEST_WITH_SLOT, RTST_RIL_REQUEST_NC
 */
#define RTST_RIL_REQUEST_WITH_SLOT_NC(_reqId, _slotId, _dataNum, ...) \
        _RTST_RIL_REQUEST_WITH_SLOT_NC(ril, _reqId, _slotId, _dataNum, __VA_ARGS__)

/*
 * Helper macro for RIL request without parameters and slot ID
 *
 * SEE ALSO: RTST_RIL_REQUEST_WITH_SLOT, RTST_RIL_REQUEST, RTST_RIL_VOID_REQUEST_WITH_SLOT
 */
#define RTST_RIL_VOID_REQUEST(_reqId) \
        RTST_RIL_VOID_REQUEST_WITH_SLOT(_reqId, RTST_SLOT_ID_0)

/*
 * Helper macro for RIL request without parameters
 *
 * SEE ALSO: RTST_RIL_REQUEST_WITH_SLOT, RTST_RIL_REQUEST, RTST_RIL_VOID_REQUEST
 */
#define RTST_RIL_VOID_REQUEST_WITH_SLOT(_reqId, _slotId)             \
        RTST_RIL_REQUEST_WITH_SLOT(_reqId, _slotId, 1, RTST_VOID, "")

/*
 * Macro to provide the expected RIL response data
 *
 * PARAMTERS:
 *  _reqId:
 *      The RIL request ID
 *  _err:
 *      The RIL error code, SEE ALSO RIL_Errno
 *  _dataNum:
 *      The parameter number
 *   ...
 *      The parameters list, detail info see RTST_RIL_REQUEST
 *
 * EXAMPLE:
 * <code>
 *     TEST(MyTestCase, MyTest) {
 *         RTST_CASE_BEGIN();
 *         ...
 *         RTST_EXPECTED_RIL_RESPONSE(
 *                 RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG,
 *                 RIL_E_SUCCESS, 4,
 *                 RTST_INT32, "1"
 *                 RTST_INT32, "0"
 *                 RTST_INT32, "0"
 *                 RTST_INT32, "0"
 *                 );
 *         ...
 *         RTST_CASE_END();
 *     }
 * </code>
 *
 * SEE ALSO: RTST_EXPECTED_RIL_RESPONSE_WITH_SLOT, RTST_EXPECTED_RIL_VOID_RESPONSE
 *           RTST_EXPECTED_RIL_VOID_RESPONSE_WITH_SLOT
 */
#define RTST_EXPECTED_RIL_RESPONSE(_reqId, _err, _dataNum, ...) \
        RTST_EXPECTED_RIL_RESPONSE_WITH_SLOT(_reqId, RTST_SLOT_ID_0, _err, _dataNum, __VA_ARGS__)

/*
 * The difference between RTST_EXPECTED_RIL_RESPONSE_NC and RTST_EXPECTED_RIL_RESPONSE is
 *    The RTST_EXPECTED_RIL_RESPONSE may add a data len info before the data
 *    according the following rule
 *         1. All data type are INT32s, will add the number of INT32
 *         2. All data type are STRING, and the number >= 2, will add the number of STRING
 *    The RTST_EXPECTED_RIL_RESPONSE_NC will not add the data len
 *
 * SEE ALSO: RTST_EXPECTED_RIL_RESPONSE_WITH_SLOT, RTST_EXPECTED_RIL_VOID_RESPONSE
 *           RTST_EXPECTED_RIL_VOID_RESPONSE_WITH_SLOT RTST_EXPECTED_RIL_RESPONSE
 *           RTST_EXPECTED_RIL_RESPONSE_WITH_SLOT_NC
 */
#define RTST_EXPECTED_RIL_RESPONSE_NC(_reqId, _err, _dataNum, ...) \
        RTST_EXPECTED_RIL_RESPONSE_WITH_SLOT_NC(_reqId, RTST_SLOT_ID_0, _err, _dataNum, __VA_ARGS__)
/*
 * Macro to provide the expected RIL response data with slot
 *
 * PARAMTERS:
 *  _reqId:
 *      The RIL request ID
 *  _slotId:
 *      The SLOT ID
 *  _err:
 *      The RIL error code, SEE ALSO RIL_Errno
 *  _dataNum:
 *      The parameter number
 *   ...
 *      The parameters list, detail info see RTST_RIL_REQUEST
 *
 * EXAMPLE:
 * <code>
 *     TEST(MyTestCase, MyTest) {
 *         RTST_CASE_BEGIN();
 *         ...
 *         RTST_EXPECTED_RIL_RESPONSE_WITH_SLOT(
 *                 RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG,
 *                 RTST_SLOT_ID_1,
 *                 RIL_E_SUCCESS, 4,
 *                 RTST_INT32, "1"
 *                 RTST_INT32, "0"
 *                 RTST_INT32, "0"
 *                 RTST_INT32, "0"
 *                 );
 *         ...
 *         RTST_CASE_END();
 *     }
 * </code>
 *
 * SEE ALSO: RTST_EXPECTED_RIL_RESPONSE, RTST_EXPECTED_RIL_VOID_RESPONSE
 *           RTST_EXPECTED_RIL_VOID_RESPONSE_WITH_SLOT
 */
#define RTST_EXPECTED_RIL_RESPONSE_WITH_SLOT(_reqId, _slotId, _err, _dataNum, ...) \
        _RTST_EXPECTED_RIL_RESPONSE_WITH_SLOT(ril, _reqId, _slotId, _err, _dataNum, __VA_ARGS__)

/*
 * The difference between RTST_EXPECTED_RIL_RESPONSE_WITH_SLOT_NC and
 *    RTST_EXPECTED_RIL_RESPONSE_WITH_SLOT is
 *    The RTST_EXPECTED_RIL_RESPONSE_WITH_SLOT may add a data len info before the data
 *    according the following rule
 *         1. All data type are INT32s, will add the number of INT32
 *         2. All data type are STRING, and the number >= 2, will add the number of STRING
 *    The RTST_EXPECTED_RIL_RESPONSE_WITH_SLOT_NC will not add the data len
 *
 * SEE ALSO: RTST_EXPECTED_RIL_RESPONSE_WITH_SLOT, RTST_EXPECTED_RIL_VOID_RESPONSE
 *           RTST_EXPECTED_RIL_VOID_RESPONSE_WITH_SLOT RTST_EXPECTED_RIL_RESPONSE
 *           RTST_EXPECTED_RIL_RESPONSE_NC
 */
#define RTST_EXPECTED_RIL_RESPONSE_WITH_SLOT_NC(_reqId, _slotId, _err, _dataNum, ...) \
        _RTST_EXPECTED_RIL_RESPONSE_WITH_SLOT_NC(ril, _reqId, _slotId, _err, _dataNum, __VA_ARGS__)
/*
 * Helper macro for RIL response without parameters and slot ID
 *
 * SEE ALSO: RTST_EXPECTED_RIL_RESPONSE, RTST_EXPECTED_RIL_RESPONSE_WITH_SLOT
 *           RTST_EXPECTED_RIL_VOID_RESPONSE_WITH_SLOT
 */
#define RTST_EXPECTED_RIL_VOID_RESPONSE(_reqId, _err) \
        RTST_EXPECTED_RIL_VOID_RESPONSE_WITH_SLOT(_reqId, RTST_SLOT_ID_0, _err)

/*
 * Helper macro for RIL response with parameters
 *
 * SEE ALSO: RTST_EXPECTED_RIL_RESPONSE, RTST_EXPECTED_RIL_RESPONSE_WITH_SLOT
 *           RTST_EXPECTED_RIL_VOID_RESPONSE
 */
#define RTST_EXPECTED_RIL_VOID_RESPONSE_WITH_SLOT(_reqId, _slot, _err) \
        RTST_EXPECTED_RIL_RESPONSE_WITH_SLOT(_reqId, _slot, _err, 1, "void", "")


/*
 * Macro to provide the expected AT command and mock the response of the AT
 *
 * PARAMTERS:
 *  _channel:
 *      AT command channel ID
 *  _at:
 *      The expected AT command
 *  _dataNum:
 *      The response number
 *  ...
 *      The response list
 *
 * EXAMPLES:
 * <code>
 *     TEST(MyTestCase, MyTest) {
 *         RTST_CASE_BEGIN();
 *         ...
 *         RTST_AT_CMD(RIL_CMD_PROXY_1, "AT+ECSCB?", 2, "+ECSCB:0", "OK");
 *         ...
 *         RTST_CASE_END();
 *     }
 * </code>
 *
 * SEE ALSO: RTST_AT_CMD_WITH_SLOT
 */
#define RTST_AT_CMD(_channel, _at, _dataNum, ...) \
    RTST_AT_CMD_WITH_SLOT(RTST_SLOT_ID_0, _channel, _at, _dataNum, __VA_ARGS__)


#define RTST_AT_CMD_NO_ASSERT(_channel, _at, _dataNum, ...) \
    RTST_AT_CMD_WITH_SLOT_NO_ASSERT(RTST_SLOT_ID_0, _channel, _at, _dataNum, __VA_ARGS__)


/*
 * Macro to provide the expected AT command and mock the response of the AT with slot ID
 *
 * PARAMTERS:
 *  _slot:
 *      Slot ID
 *  _channel:
 *      AT command channel ID
 *  _at:
 *      The expected AT command
 *  _dataNum:
 *      The response number
 *  ...
 *      The response list
 *
 * EXAMPLES:
 *
 * <code>
 *     TEST(MyTestCase, MyTest) {
 *         RTST_CASE_BEGIN();
 *         ...
 *         RTST_AT_CMD_WITH_SLOT(RTST_SLOT_ID_1, RIL_CMD_PROXY_1, "AT+ECSCB?", 2, "+ECSCB:0", "OK");
 *         ...
 *         RTST_CASE_END();
 *     }
 * </code>
 *
 * SEE ALSO: RTST_AT_CMD
 */
#define RTST_AT_CMD_WITH_SLOT(_slot, _channel, _at, _dataNum, ...) \
    _RTST_AT_ITEMS(ril, _slot, _channel, _at, _dataNum, __VA_ARGS__)

#define RTST_AT_CMD_WITH_SLOT_NO_ASSERT(_slot, _channel, _at, _dataNum, ...) \
    _RTST_AT_ITEMS_NO_ASSERT(ril, _slot, _channel, _at, _dataNum, __VA_ARGS__)


/*
 * Macro to mock the urc string from modem
 *
 * PARAMETERS:
 *  _urc:
 *      The URC string
 *
 * EXAMPLES
 *
 * <code>
 *     TEST(MyTestCase, MyTest) {
 *         RTST_CASE_BEGIN();
 *         ...
 *         RTST_URC_STRING("^SMMEMFULL:\"ME\"");
 *         ...
 *         RTST_CASE_END();
 *     }
 * </code>
 *
 * SEE ALSO: RTST_URC_STRING_WITH_SLOT
 */
#define RTST_URC_STRING(_urc) \
    RTST_URC_STRING_WITH_SLOT(RTST_SLOT_ID_0, _urc)

/*
 * Macro to mock the urc string from modem with slot ID
 *
 * PARAMETERS:
 *  _slot:
 *      Slot ID
 *  _urc:
 *      The URC string
 *
 * EXAMPLES
 *
 * <code>
 *     TEST(MyTestCase, MyTest) {
 *         RTST_CASE_BEGIN();
 *         ...
 *         RTST_URC_STRING_WITH_SLOT(RTST_SLOT_ID_1, "^SMMEMFULL:\"ME\"");
 *         ...
 *         RTST_CASE_END();
 *     }
 * </code>
 *
 * SEE ALSO: RTST_URC_STRING
 */
#define RTST_URC_STRING_WITH_SLOT(_slot, _urc) \
        _RTST_URC_STRING_WITH_SLOT(ril, _slot, _urc)

/*
 * Macro to provided expected RIL URC from RILD
 *
 * PARAMTERS:
 *  _urcId:
 *      The RIL URC ID
 *  _dataNum:
 *      The parameters number
 *  ...
 *      The parameters list, see RTST_RIL_REQUEST
 *
 * EXAMPLES
 *
 * <code>
 *     TEST(MyTestCase, MyTest) {
 *         RTST_CASE_BEGIN();
 *         ...
 *         RTST_EXPECTED_RIL_URC(
 *             RIL_UNSOL_CDMA_CARD_INITIAL_ESN_OR_MEID,
 *             1,
 *             RTST_STRING,
 *             "00000000,12345678");
 *         ...
 *         RTST_CASE_END();
 *     }
 * </code>
 *
 * SEE ALSO: RTST_EXPECTED_RIL_URC_WITH_SLOT, RTST_EXPECTED_RIL_VOID_URC
 *           RTST_EXPECTED_RIL_VOID_URC_WITH_SLOT RTST_EXPECTED_RIL_URC_WITH_SLOT_NC
 *           RTST_EXPECTED_RIL_URC_NC
 */
#define RTST_EXPECTED_RIL_URC(_urcId, _dataNum, ...) \
    RTST_EXPECTED_RIL_URC_WITH_SLOT(_urcId, RTST_SLOT_ID_0, _dataNum, __VA_ARGS__)


/*
 * The difference between RTST_EXPECTED_RIL_URC_NC and RTST_EXPECTED_RIL_URC is
 *    The RTST_EXPECTED_RIL_URC may add a data len info before the data
 *    according the following rule
 *         1. All data type are INT32s, will add the number of INT32
 *         2. All data type are STRING, and the number >= 2, will add the number of STRING
 *    The RTST_EXPECTED_RIL_URC_NC will not add the data len
 *
 * SEE ALSO: RTST_EXPECTED_RIL_URC_WITH_SLOT, RTST_EXPECTED_RIL_VOID_URC
 *           RTST_EXPECTED_RIL_VOID_URC_WITH_SLOT, RTST_EXPECTED_RIL_URC_WITH_SLOT_NC
 *           RTST_EXPECTED_RIL_URC
 */
#define RTST_EXPECTED_RIL_URC_NC(_urcId, _dataNum, ...) \
    RTST_EXPECTED_RIL_URC_WITH_SLOT_NC(_urcId, RTST_SLOT_ID_0, _dataNum, __VA_ARGS__)


/*
 * Macro to provided expected RIL URC from RILD with slot ID
 *
 * PARAMTERS:
 *  _urcId:
 *      The RIL URC ID
 *  _slotId
 *      Slot ID
 *  _dataNum:
 *      The parameters number
 *  ...
 *      The parameters list, see RTST_RIL_REQUEST
 *
 * EXAMPLES
 *
 * <code>
 *     TEST(MyTestCase, MyTest) {
 *         RTST_CASE_BEGIN();
 *         ...
 *         RTST_EXPECTED_RIL_URC(
 *             RIL_UNSOL_CDMA_CARD_INITIAL_ESN_OR_MEID,
 *             RTST_SLOT_ID_1,
 *             1,
 *             RTST_STRING,
 *             "00000000,12345678");
 *         ...
 *         RTST_CASE_END();
 *     }
 * </code>
 *
 * SEE ALSO: RTST_EXPECTED_RIL_URC, RTST_EXPECTED_RIL_VOID_URC RTST_EXPECTED_RIL_URC_NC
 *           RTST_EXPECTED_RIL_VOID_URC_WITH_SLOT RTST_EXPECTED_RIL_URC_WITH_SLOT_NC
 */
#define RTST_EXPECTED_RIL_URC_WITH_SLOT(_urcId, _slot, _dataNum, ...) \
        _RTST_EXPECTED_RIL_URC_WITH_SLOT(ril, _urcId, _slot, _dataNum, __VA_ARGS__)

#define RTST_EXPECTED_RIL_URC_WITH_SLOT_NC(_urcId, _slot, _dataNum, ...) \
        _RTST_EXPECTED_RIL_URC_WITH_SLOT_NC(ril, _urcId, _slot, _dataNum, __VA_ARGS__)

/*
 * Helper macro for RIL URC without slot ID and parameters
 *
 * SEE ALSO: RTST_EXPECTED_RIL_URC, RTST_EXPECTED_RIL_URC_WITH_SLOT
 *           RTST_EXPECTED_RIL_VOID_URC_WITH_SLOT
 *           RTST_EXPECTED_RIL_URC_NC, RTST_EXPECTED_RIL_URC_WITH_SLOT_NC
 */
#define RTST_EXPECTED_RIL_VOID_URC(_urcId) \
        RTST_EXPECTED_RIL_VOID_URC_WITH_SLOT(_urcId, RTST_SLOT_ID_0)

/*
 * Helper macro for RIL URC without parameters
 *
 * SEE ALSO: RTST_EXPECTED_RIL_URC, RTST_EXPECTED_RIL_URC_WITH_SLOT
 *           RTST_EXPECTED_RIL_VOID_URC
 *           RTST_EXPECTED_RIL_URC_NC, RTST_EXPECTED_RIL_URC_WITH_SLOT_NC
 */
#define RTST_EXPECTED_RIL_VOID_URC_WITH_SLOT(_urcId, _slot) \
        RTST_EXPECTED_RIL_URC_WITH_SLOT(_urcId, _slot, 1, RTST_VOID, "")

/*
 * Macro to Mock the system property value
 *
 * PARAMETERS
 *  _key:
 *      the key of the system property, such as "ro.mtk_fd_support"
 *  _value:
 *      the value of the system property, such as "1"
 *
 * EXAMPLES
 *
 * <code>
 *     TEST(MyTestCase, MyTest) {
 *         RTST_CASE_BEGIN();
 *         ...
 *         RTST_SYSTEM_PROPERTY("ro.mtk_fd_support", "1");
 *         ...
 *         RTST_CASE_END();
 *     }
 * </code>
 *
 * SEE ALSO: RTST_EXPECTED_SYSTEM_PROPERTY, RTST_EXPECTED_SYSTEM_PROPERTY_WITH_DELAY
 */
#define RTST_SYSTEM_PROPERTY(_key, _value) \
        _RTST_SYSTEM_PROPERTY(ril, _key, _value)

/*
 * Macro to provide the expected system property value
 * it will delay 10s to check
 *
 * PARAMETERS
 *  _key:
 *      the key of the system property, such as "vendor.ril.fd.mode"
 *  _value:
 *      the value of the system property, such as "1"
 *
 * EXAMPLES
 *
 * <code>
 *     TEST(MyTestCase, MyTest) {
 *         RTST_CASE_BEGIN();
 *         ...
 *         RTST_EXPECTED_SYSTEM_PROPERTY("vendor.ril.fd.mode", "1");
 *         ...
 *         RTST_CASE_END();
 *     }
 * </code>
 *
 * SEE ALSO: RTST_SYSTEM_PROPERTY, RTST_EXPECTED_SYSTEM_PROPERTY_WITH_DELAY
 */
#define RTST_EXPECTED_SYSTEM_PROPERTY(_key, _value) \
        _RTST_EXPECTED_SYSTEM_PROPERTY(ril, _key, _value)

/*
 * Same as RTST_EXPECTED_SYSTEM_PROPERTY except you can change the default delay value
 * if you check the value after the macro RTST_EXPECTED_RIL_RESPONSE, you can set
 * the delay as 0 to speed up the test procedure
 */
#define RTST_EXPECTED_SYSTEM_PROPERTY_WITH_DELAY(_key, _value, _delay) \
        _RTST_EXPECTED_SYSTEM_PROPERTY_WITH_DELAY(ril, _key, _value, _delay)

/*
 * Macro to init the system property before init procedure
 *
 * PRARAMETER
 *    _key:
 *        the key of the system property
 *    _value:
 *        the value of the system property
 *
 * EXAMPLES
 *
 * <code>
 *     // The case must be the first case
 *     TEST(MyTestCase, MyTest) {
 *         RTST_INIT_SYS_PROP("ro.xxx.yyy", "1")
 *         RTST_CASE_BEGIN();
 *         ...
 *         ...
 *         RTST_CASE_END();
 *     }
 * </code>
 *
 * LIMITATION
 *   Can only be used before the first RTST_CASE_BEGIN
 *   Because we init the vendor ril in the RTST_CASE_BEGIN
 */
#define RTST_INIT_SYS_PROP(_key, _value)               \
    do {                                               \
        static bool inited = false;                    \
        if (!inited) {                                 \
            RtstMRil::setEmulatorMode();               \
            RtstEnv::get()->setSysProp(_key, _value);  \
            inited = true;                             \
        }                                              \
    } while(0)


#define RTST_CLEAN_RIL_SOCKET_DATA(_slot) \
        RtstEnv::get()->cleanRilSocket(_slot)

#define RTST_CLEAN_ALL_RIL_SOCKET_DATA() \
        RtstEnv::get()->cleanAllRilSocket()

#define RTST_SET_POLL_FD_TIME(_time) \
        RtstEnv::get()->setFdTimeoutValue(_time)

// Internal Use
#define _RTST_CASE_BEGIN(_name, _rfx_assert, _step_test)                                      \
        do {                                                                                  \
            RTST_LOG_D(RTST_TAG, "RTST_CASE_BEGIN line = %d", __LINE__);                       \
            RtstEnv::get()->init();                                                           \
            RtstCase _case##_name(#_name, _rfx_assert, _step_test)

// Internal Use
#define _RTST_RIL_REQUEST_WITH_SLOT(_name, _id, _slot, _dataNum, ...)                        \
        do {                                                                                 \
            RtstRilReqItem* pItem = new RtstRilReqItem(_id, _slot);                          \
            AssertionResult r = _case##_name.stepRun(pItem, _dataNum, __VA_ARGS__);          \
            ASSERT_TRUE(r);                                                                  \
        } while(0)

// Internal Use
#define _RTST_RIL_REQUEST_WITH_SLOT_NC(_name, _id, _slot, _dataNum, ...)                     \
        do {                                                                                 \
            RtstRilReqItem* pItem = new RtstRilReqItem(_id, _slot);                          \
            pItem->getDataSequency().setSeqCalType(RtstDataSequency::FORCE_NON_COUNTABLE);   \
            AssertionResult r = _case##_name.stepRun(pItem, _dataNum, __VA_ARGS__);          \
            ASSERT_TRUE(r);                                                                  \
        } while(0)

// Internal Use
#define _RTST_EXPECTED_RIL_RESPONSE_WITH_SLOT(_name, _id, _slot, _err, _dataNum, ...)         \
        do {                                                                                  \
            RtstExpectedRilRspItem* pItem = new RtstExpectedRilRspItem(_id, _slot, _err);     \
            AssertionResult r = _case##_name.stepRun(pItem, _dataNum, __VA_ARGS__);           \
            ASSERT_TRUE(r);                                                                   \
        } while(0)

// Internal Use
#define _RTST_EXPECTED_RIL_RESPONSE_WITH_SLOT_NC(_name, _id, _slot, _err, _dataNum, ...)      \
        do {                                                                                  \
            RtstExpectedRilRspItem* pItem = new RtstExpectedRilRspItem(_id, _slot, _err);     \
            pItem->getDataSequency().setSeqCalType(RtstDataSequency::FORCE_NON_COUNTABLE);    \
            AssertionResult r = _case##_name.stepRun(pItem, _dataNum, __VA_ARGS__);           \
            ASSERT_TRUE(r);                                                                   \
        } while(0)


// Internal Use
#define _RTST_AT_ITEMS(_name, _slot, _channel, _at, _dataNum, ...)                     \
        do {                                                                           \
            int index = _case##_name.generateAtDatas(                                  \
                    _slot, _channel, _at, _dataNum, __VA_ARGS__);                      \
            AssertionResult r = _case##_name.runAt(index);                             \
            ASSERT_TRUE(r);                                                            \
        } while(0)

// Internal Use
#define _RTST_AT_ITEMS_NO_ASSERT(_name, _slot, _channel, _at, _dataNum, ...)           \
        do {                                                                           \
            int index = _case##_name.generateAtDatas(                                  \
                    _slot, _channel, _at, _dataNum, __VA_ARGS__);                      \
            AssertionResult r = _case##_name.runAt(index);                             \
        } while(0)


// Internal Use
#define _RTST_EXPECTED_RIL_URC_WITH_SLOT(_name, _urcId, _slot, _dataNum, ...)             \
        do {                                                                              \
            RtstExpectedRilUrcItem *pItem = new RtstExpectedRilUrcItem(_urcId, _slot);    \
            AssertionResult r = _case##_name.stepRun(pItem, _dataNum, __VA_ARGS__);       \
            ASSERT_TRUE(r);                                                               \
        } while(0)

// Internal Use
#define _RTST_EXPECTED_RIL_URC_WITH_SLOT_NC(_name, _urcId, _slot, _dataNum, ...)            \
        do {                                                                                \
            RtstExpectedRilUrcItem *pItem = new RtstExpectedRilUrcItem(_urcId, _slot);      \
            pItem->getDataSequency().setSeqCalType(RtstDataSequency::FORCE_NON_COUNTABLE);  \
            AssertionResult r = _case##_name.stepRun(pItem, _dataNum, __VA_ARGS__);         \
            ASSERT_TRUE(r);                                                                 \
        } while(0)

// Internal Use
#define _RTST_URC_STRING_WITH_SLOT(_name, _slot, _urc)                                        \
        do {                                                                                  \
            RtstUrcStringItem* pItem = new RtstUrcStringItem(_slot);                          \
            AssertionResult r = _case##_name.stepRun(pItem, 1, RTST_STRING, _urc);            \
            ASSERT_TRUE(r);                                                                   \
        } while(0)

// Internal Use
#define _RTST_SYSTEM_PROPERTY(_name, _key, _value)                      \
        do {                                                            \
            RtstSysPropItem *pItem = new RtstSysPropItem(_key, _value); \
            AssertionResult r = _case##_name.stepRun(pItem, 0);         \
            ASSERT_TRUE(r);                                             \
        } while(0)

// Internal Use
#define _RTST_EXPECTED_SYSTEM_PROPERTY(_name, _key, _value)                             \
        do {                                                                            \
            RtstExpectedSysPropItem *pItem = new RtstExpectedSysPropItem(_key, _value); \
            AssertionResult r = _case##_name.stepRun(pItem, 0);                         \
            ASSERT_TRUE(r);                                                             \
        } while(0)


// Internal Use
#define _RTST_EXPECTED_SYSTEM_PROPERTY_WITH_DELAY(_name, _key, _value, _delay)                  \
        do {                                                                                    \
            RtstExpectedSysPropItem *pItem = new RtstExpectedSysPropItem(_key, _value, _delay); \
            AssertionResult r = _case##_name.stepRun(pItem, 0);                                 \
            ASSERT_TRUE(r);                                                                     \
        } while(0)

// Internal Use
#define _RTST_CASE_END(_name)                                            \
            AssertionResult r = _case##_name.finalRun();                 \
            ASSERT_TRUE(r);                                              \
            RtstEnv::get()->releaseRequestInfo();                        \
            RTST_LOG_D(RTST_TAG, "RTST_CASE_END line = %d", __LINE__);    \
        } while(0)

/*****************************************************************************
 * Class RtstData
 *****************************************************************************/
/*
 * Base class for data
 */
class RtstData {
// External Interface
public:
    // Set the value of the data.
    //
    // In order to make the design simple
    // all the data are stored by string
    //
    // RETURNS: void
    void setData(const char * data);

    // The supported data type.
    //
    // SEE ALSO: RTST_INT32, RTST_INT64
    typedef enum {
        UNDEFINE,
        INT8,
        INT32,           // 32bit integer, decimal format
        INT64,           // 64bit integer, decimal format
        STRING,          // string type
        VOID,            // void type
        RAW,             // binary type, hex format

        DATA_TYPE_MAX
    } DataType;

    // Get the value of the data
    //
    // RETURNS: value of the data
    //
    // SEE ALSO: setData
    const String8 & getData() const { return m_data; }

// Constructor / Destructor
public:
    RtstData();
    virtual ~RtstData() {}

// Overridable
public:
    // Output the data to a Parcel object
    //
    // RETURNS: void
    virtual void toParcel(
        Parcel &p  // [OUT] the parcel of the data to output
    ) {
        // Make compiler happy, no build warning
        p.dataPosition();
    }

    // Check the parcel with the data
    //
    // RETURNS: AssertionResult
    virtual AssertionResult checkParcel(
        Parcel &p                       // [IN] the parcel to check
    ) {
        // Make compiler happy, no build warning
        p.dataPosition();
        return AssertionSuccess();
    }

    // Get the data type.
    //
    // RETURNS: the type of the data
    //
    // SEE ALSO: setType, RTST_INT32, RTST_INT64
    virtual DataType getType() const {
        return UNDEFINE;
    }

// Implementation
private:
    String8 m_data;
};


/*****************************************************************************
 * Class RtstVoidData
 *****************************************************************************/
/*
 * Class for void data
 */
class RtstVoidData : public RtstData{
// Constructor / Destructor
public:
    RtstVoidData() {}
    virtual ~RtstVoidData() {}

// Override
public:
    virtual AssertionResult checkParcel(Parcel &p);
    virtual DataType getType() const {
        return VOID;
    }
};

/*****************************************************************************
 * Class RtstInt8Data
 *****************************************************************************/
/*
 * Class for 8 bit integer
 */
class RtstInt8Data : public RtstData{
// External Method
public:
    // Get the 8bit integer value
    //
    // RETURNS: the 8bit integer value
    uint8_t toInt8();

// Constructor / Destructor
public:
    RtstInt8Data() {}
    virtual ~RtstInt8Data() {}

// Override
public:
    virtual void toParcel(Parcel &p);
    virtual AssertionResult checkParcel(Parcel &p);
    virtual DataType getType() const {
        return INT8;
    }
};

/*****************************************************************************
 * Class RtstInt32Data
 *****************************************************************************/
/*
 * Class for 32 bit integer
 */
class RtstInt32Data : public RtstData{
// External Method
public:
    // Get the 32bit integer value
    //
    // RETURNS: the 32bit integer value
    int32_t toInt32();

// Constructor / Destructor
public:
    RtstInt32Data() {}
    virtual ~RtstInt32Data() {}

// Override
public:
    virtual void toParcel(Parcel &p);
    virtual AssertionResult checkParcel(Parcel &p);
    virtual DataType getType() const {
        return INT32;
    }
};


/*****************************************************************************
 * Class RtstInt64Data
 *****************************************************************************/
/*
 * Class for 64bit integer
 */
class RtstInt64Data : public RtstData {
// Exernal Method
public:
    // Get the 64bit integer value
    //
    // RETURNS: the 64bit integer value
    int64_t toInt64();

// Constructor / Destructor
public:
    RtstInt64Data() {}
    virtual ~RtstInt64Data() {}

// Override
public:
    virtual void toParcel(Parcel &p);
    virtual AssertionResult checkParcel(Parcel &p);
    virtual DataType getType() const {
        return INT64;
    }
};


/*****************************************************************************
 * Class RtstStringData
 *****************************************************************************/
/*
 * Class for string data
 */
class RtstStringData : public RtstData {
// External Method
public:
    // Get the value of type string
    //
    // RETURNS: the C-style string
    const char* toString();

// Constructor / Destructor
public:
    RtstStringData() {}
    virtual ~RtstStringData() {}

// Override
public:
    virtual void toParcel(Parcel &p);
    virtual AssertionResult checkParcel(Parcel &p);
    virtual DataType getType() const {
        return STRING;
    }
};


/*****************************************************************************
 * Class RtstRawData
 *****************************************************************************/
/*
 * Class for raw data
 */
class RtstRawData : public RtstData {
// External Method
public:
    // Get the value of type raw
    //
    // RETURNS: void
    void toRaw(
        Vector<char> &raw  // [OUT] the vector to store the raw data
    );

// Constructor / Destructor
public:
    RtstRawData() {}
    virtual ~RtstRawData() {}

// Override
public:
    virtual void toParcel(Parcel &p);
    virtual AssertionResult checkParcel(Parcel &p);
    virtual DataType getType() const {
        return RAW;
    }

// Implementation
private:
    int getRawLen() const;
    void getRaw(char *buf, int len) const;
};


/*****************************************************************************
 * Class RtstDataCreator
 *****************************************************************************/
/*
 * Class for data create
 */
class RtstDataCreator {
// Overridable
public:
    // Create the RtstData and its subclass Object by type
    //
    // RETURNS: RtstData or its subcalss Object's Pointer
    virtual RtstData *createData(
        const char *type    // [IN] data type string, see RTST_INT32 ...
    );

// Constructor / Destructor
public:
    RtstDataCreator() {}
    virtual ~RtstDataCreator() {}

// Implementation
private:
    static RtstData::DataType getDataType(const char *type);
};


/*****************************************************************************
 * Class RtstSeq
 *****************************************************************************/
/*
 * Base sequence type
 */
class RtstSeq {
// Overidable
public:
    // Output the data to a Parcel object
    //
    // RETURNS: void
    virtual void toParcel(
        Parcel &p,                 // [OUT] the parcel of the data seq to output
        Vector<RtstData *> &seq    // [IN] the data sequence
    );

    // Check the parcel with the data sequency
    //
    // RETURNS: void
    virtual AssertionResult checkParcel(
        Parcel &p,                      // [IN] the parcel to check
        Vector<RtstData *> &seq         // [IN] the data sequence
    );

// Constructor / Destructor
public:
    RtstSeq() {}
    virtual ~RtstSeq() {}
};


/*****************************************************************************
 * Class RtstCountedSeq
 *****************************************************************************/
/*
 * Sequency with count
 */
class RtstCountedSeq : public RtstSeq {
// Override
public:
    virtual void toParcel(Parcel &p, Vector<RtstData *> &seq);
    virtual AssertionResult checkParcel(Parcel &p, Vector<RtstData *> &seq);
};


/*****************************************************************************
 * Class RtstDataSequency
 *****************************************************************************/
/*
 * Class to maintain the data sequence
 */
class RtstDataSequency {
// External Method
public:
    typedef enum {
        AUTO,
        FORCE_NON_COUNTABLE,
        FORCE_COUNTABLE,
    } SEQ_CAL_TYPE;

    // Output the data to a Parcel object
    //
    // RETURNS: void
    void toParcel(
         Parcel &p // [OUT] the parcel of the data seq to output,
    );

    // Check the parcel with the data
    //
    // RETURNS: AssertionResult
    AssertionResult checkParcel(
        Parcel &p  // [IN] the parcel to check
    );

    // Append data to this sequence
    //
    // RETURNS: void
    void appendWith(
        int num,    // [IN] parameter number
        ...         // [IN] parameter list, such as {"int32", "12"}, {"string", "hello"}
    );

    // Internal used
    void appendWithVaList(
        int num,       // [IN] parameter number
        va_list argp   // [IN] parameter list
    );

    // Put the datas in a String8 vector
    //
    // RETURNS: void
    void getDatas(
        Vector<const char *> &datas // the vector to put
    );

    // Set the calculate type
    //
    // RETURNS: void
    void setSeqCalType(
        SEQ_CAL_TYPE type = AUTO // [IN] the calculate type
    ) {
        m_seqCalType = type;
    }

    // Get the calculate type
    //
    // RETURNS: the calculate type
    SEQ_CAL_TYPE getSeqCalType() {
        return m_seqCalType;
    }

// Constructor / Destructor
public:
    RtstDataSequency();
    virtual ~RtstDataSequency();

// Implementation
private:
    bool isSameTypeWith(RtstData::DataType type);
    bool isMultiple();
    bool isInt8s();
    bool isInt32s();
    bool isStrings();
    void calSeqType();

    Vector<RtstData *> m_sequence;
    RtstSeq *m_seqTypePtr;
    RtstDataCreator m_creator;
    SEQ_CAL_TYPE m_seqCalType;
};

/*****************************************************************************
 * Class RtstItemBase
 *****************************************************************************/
/*
 * Base class for item
 */
class RtstItemBase {
// External Interface
public:
    // Get the reference of the data sequence
    //
    // RETURNS: the reference of the data sequence
    RtstDataSequency &getDataSequency() {
        return m_seq;
    }

    // Run the item
    //
    // RETURNS: the result of the action
    AssertionResult run();

// Constructor / Destructor
public:
    RtstItemBase() {}
    virtual ~RtstItemBase() {}

// Overidable
public:
    virtual bool isMockType() { return false; }
    virtual bool isExpectedType() { return false; }
    virtual void sendData() {}
    virtual AssertionResult checkData() { return AssertionSuccess(); }

// Implementation
private:
    RtstDataSequency m_seq;
};


/*****************************************************************************
 * Class RtstTelItemBase
 *****************************************************************************/
/*
 * Class for item with slot ID
 */
class RtstTelItemBase: public RtstItemBase {
// External Method
public:
    // Get the slot ID of the AT
    //
    // RETURNS: slot ID
    int getSlotId() const {
        return m_slotId;
    }

// Constructor / Destructor
public:
    // Construct a base RIL item
    RtstTelItemBase(
        int slotId        // [IN] slot ID
    ) : m_slotId(slotId) {
    }

    virtual ~RtstTelItemBase() {}

// Implementation
private:
    int m_slotId;
};

/*****************************************************************************
 * Class RtstRilItemBase
 *****************************************************************************/
/*
 * Class for item with slot id and ril command id
 */
class RtstRilItemBase : public RtstTelItemBase {
// External Method
public:
    // Get the RIL command ID
    int getRilCmdId() const {
        return m_cmdId;
    }

// Constructor / Destructor
public:
    // Construct a base RIL item
    RtstRilItemBase(
        int cmdId,        // [IN] RIL request/URC ID
        int slotId        // [IN] slot ID
    ) : RtstTelItemBase(slotId), m_cmdId(cmdId) {
    }

    virtual ~RtstRilItemBase() {}

// Implementation
private:
    int m_cmdId;
};


/*****************************************************************************
 * Class RtstRilReqItem
 *****************************************************************************/
/*
 * RIL request item
 */
class RtstRilReqItem : public RtstRilItemBase {
// Constructor / Destructor
public:
    // Construct a RIL request item
    RtstRilReqItem(
        int reqId,        // [IN] RIL request ID
        int slotId        // [IN] slot ID
    ) : RtstRilItemBase(reqId, slotId) {
    }

    virtual ~RtstRilReqItem() {}

// Override
public:
    virtual bool isMockType() {
        return true;
    }

    virtual void sendData();
};


/*****************************************************************************
 * Class RtstExpectedRilRspItem
 *****************************************************************************/
/*
 * Expected RIL response Item
 */
class RtstExpectedRilRspItem : public RtstRilItemBase {
// External Method
public:
    // Get the error code for the RIL Req
    //
    // RETURNS: RIL error code
    int getErrorCode() const {
        return m_errorCode;
    }

// Constructor / Destructor
public:
    // Construct a base RIL item
    RtstExpectedRilRspItem(
        int reqId,        // [IN] RIL request ID
        int slotId,       // [IN] slot ID
        int errorCode     // [IN] error code of the RIL response
    ) : RtstRilItemBase(reqId, slotId), m_errorCode(errorCode) {
    }

    virtual ~RtstExpectedRilRspItem() {}

// Override
public:
    virtual bool isExpectedType() {
        return true;
    }

    virtual AssertionResult checkData();

// Implementation
private:
    int m_errorCode;
};


/*****************************************************************************
 * Class RtstExpectedRilUrcItem
 *****************************************************************************/
/*
 * Expected RIL URC item
 */
class RtstExpectedRilUrcItem : public RtstRilItemBase {
// Constructor / Destructor
public:
    // Construct a expected RIL URC item
    RtstExpectedRilUrcItem(
        int urcId,        // [IN] URC ID
        int slotId        // [IN] slot ID
    ) : RtstRilItemBase(urcId, slotId) {
    }

    virtual ~RtstExpectedRilUrcItem() {}

// Override
public:
    virtual bool isExpectedType() {
        return true;
    }

    virtual AssertionResult checkData();
};


/*****************************************************************************
 * Class RtstExpectedInitRilUrcItem
 *****************************************************************************/
class RtstExpectedInitRilUrcItem: public RtstExpectedRilUrcItem {
// External Method
public:
    void setForAllSlot() {
        m_allSlot = true;
    }

// Constructor / Destructor
public:
    // Construct a expected RIL URC item
    RtstExpectedInitRilUrcItem(
        int urcId,        // [IN] URC ID
        int slotId        // [IN] slot ID
    ) : RtstExpectedRilUrcItem(urcId, slotId), m_allSlot(false) {
    }

    virtual ~RtstExpectedInitRilUrcItem() {}

// Override
public:
    virtual AssertionResult checkData();
// Implement
private:
    bool m_allSlot;
};


/*****************************************************************************
 * Class RtstAtItemBase
 *****************************************************************************/
/*
 * class for base AT item
 */
class RtstAtItemBase : public RtstTelItemBase {
// External Method
public:
    // Get the channel ID of the AT
    //
    // RETURNS: channel ID
    int getChannelId() const {
        return m_channelId;
    }

// Constructor / Destructor
public:
    // Construct a base AT item
    RtstAtItemBase(
        int channelId,   // [IN] AT command Channel ID
        int slotId       // [IN] Slot ID
    ) : RtstTelItemBase(slotId), m_channelId(channelId) {
    }

    virtual ~RtstAtItemBase() {}

// Implementation
private:
    int m_channelId;
};


/*****************************************************************************
 * Class RtstExpectedAtItem
 *****************************************************************************/
/*
 * class for expected At item
 */
class RtstExpectedAtItem : public RtstAtItemBase {
// Constructor / Destructor
public:
    // Construct a expected AT item
    RtstExpectedAtItem(
        int channelId,   // [IN] AT command Channel ID
        int slotId       // [IN] Slot ID
    ) : RtstAtItemBase(channelId, slotId) {
    }

    virtual ~RtstExpectedAtItem() {}

// Override
public:
    virtual bool isExpectedType() {
        return true;
    }

    virtual AssertionResult checkData();
};


/*****************************************************************************
 * Class RtstUrcStringItem
 *****************************************************************************/
/*
 * Class for URC string item
 */
class RtstUrcStringItem: public RtstAtItemBase {
// Constructor / Destructor
public:
    // Construct a URC string item
    RtstUrcStringItem(
        int slotId       // [IN] Slot ID
    ) : RtstAtItemBase(URC_CHANNEL, slotId) {
    }

    virtual ~RtstUrcStringItem() {}

// Override
public:
    virtual bool isMockType() {
        return true;
    }

    virtual void sendData();
};

/*****************************************************************************
 * Class RtstAtResponseItem
 *****************************************************************************/
/*
 * Class for AT response item
 */
class RtstAtResponseItem : public RtstAtItemBase {
// Constructor / Destructor
public:
    // Construct a expected AT item
    RtstAtResponseItem(
        int channelId,   // [IN] AT command Channel ID
        int slotId       // [IN] Slot ID
    ) : RtstAtItemBase(channelId, slotId) {
    }

    virtual ~RtstAtResponseItem() {}
// Override
public:
    virtual bool isMockType() {
        return true;
    }

    virtual void sendData();
};


/*****************************************************************************
 * Class RtstSysPropItemBase
 *****************************************************************************/
/*
 * Class for base system property item
 */
class RtstSysPropItemBase : public RtstItemBase {
// External Method
public:
    const String8& getKey() const {
        return m_key;
    }

    const String8& getValue() const {
        return m_value;
    }
// Constructor / Destructor
public:
    RtstSysPropItemBase(
        const char *key,
        const char *value
    ) : m_key(key), m_value(value) {
    }

    virtual ~RtstSysPropItemBase() {}
// Implementation
private:
    String8 m_key;
    String8 m_value;
};


/*****************************************************************************
 * Class RtstSysPropItem
 *****************************************************************************/
/*
 * Class for system property item
 */
class RtstSysPropItem : public RtstSysPropItemBase {
// Constructor / Destructor
public:
    RtstSysPropItem(
        const char *key,
        const char *value
    ) : RtstSysPropItemBase(key, value) {
    }

    virtual ~RtstSysPropItem() {}

// Override
public:
    virtual bool isMockType() {
        return true;
    }

    virtual void sendData();
};


/*****************************************************************************
 * Class RtstExpectedSysPropItem
 *****************************************************************************/
/*
 * Class for expected system property item
 */
class RtstExpectedSysPropItem : public RtstSysPropItemBase {
// Constructor / Destructor
public:
    RtstExpectedSysPropItem(
        const char *key,
        const char *value,
        int delay = 10
    ) : RtstSysPropItemBase(key, value), m_delay(delay) {
    }

    virtual ~RtstExpectedSysPropItem() {}
// Override
public:
    virtual bool isExpectedType() {
        return true;
    }

    virtual AssertionResult checkData();

// Implementation
private:
    int m_delay;
};


/*****************************************************************************
 * Class RtstCase
 *****************************************************************************/
/*
 * Class for RIL test case
 */
class RtstCase {
// External Method
public:
    // Get the case name.
    //
    // RETURNS: the case name
    const String8 &getName() const {
        return m_caseName;
    }

    // Put an item to the case
    // The item must be created from heap
    //
    // RETURNS: void
    void put(
        RtstItemBase * item  // [IN] the item to be put
    );

    // Generate the AT data
    //
    // RETURNS: the index of the expected at item
    int generateAtDatas(
        int slot,            // [IN] slot ID
        int channel,         // [IN] channel ID
        const char *at,      // [IN] expected AT command
        int num,             // [IN] AT response number
        ...                  // [IN] AT response list
    );


    // Check if use RTST_ASSERT
    //
    // RETURNS: true if use RTST_ASSERT, or use gtest ASSERT
    bool isUseRfxAssert() {
        return m_useRfxAssert;
    }

    // Check if run the case in each RTST_XXX
    //
    // RETURNS: true if run the case in each RTST_XXX, or run the case @ RTST_CASE_END
    bool isStepRun() {
        return m_stepRun;
    }

    // Get the items of the test case
    //
    // RETURNS: the items of the test case
    const Vector<RtstItemBase *>& getItems() const {
        return m_caseItems;
    }

    // Run the case step by step if stepRun is set
    //
    // RETURNS: AssertionResult
    AssertionResult stepRun(
        RtstItemBase *item, // [IN] the pointer of the item
        int num,            // [IN] parameter number
        ...                 // [IN] parameter list
    );

    // Run the expected AT and AT response
    //
    // RETURNS: AssertionResult
    AssertionResult runAt(
        unsigned int index   // [IN] the return value of generateAtDatas
    );

    // Run all the item if not step run
    //
    // RETURNS: AssertionResult
    AssertionResult finalRun();

// Constructor / Destructor
public:
    RtstCase(
        const char *name,
        bool useRfxAssert,
        bool stepRun
    ) : m_caseName(name), m_useRfxAssert(useRfxAssert), m_stepRun(stepRun) {
    }
    ~RtstCase();

// Implementation
private:
    Vector<RtstItemBase *> m_caseItems;
    String8 m_caseName;
    bool m_useRfxAssert;
    bool m_stepRun;
};
#endif /* __RTST_DATA_H__ */
