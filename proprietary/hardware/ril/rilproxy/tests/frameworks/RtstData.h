/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2016. All rights reserved.
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
#include "RfxDefs.h"
#include "RfxVariant.h"
#include "RfxStatusDefs.h"
#include "RfxClassInfo.h"

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
 *      if true, we will use RFX_ASSERT
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
        RTST_RIL_REQUEST_WITH_SLOT(_reqId, RFX_SLOT_ID_0, _dataNum, __VA_ARGS__)


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
        RTST_RIL_REQUEST_WITH_SLOT_NC(_reqId, RFX_SLOT_ID_0, _dataNum, __VA_ARGS__)


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
 *                 RFX_SLOT_ID_1, 1, RTST_INT32, "0");
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
        RTST_RIL_VOID_REQUEST_WITH_SLOT(_reqId, RFX_SLOT_ID_0)

/*
 * Helper macro for RIL request without parameters
 *
 * SEE ALSO: RTST_RIL_REQUEST_WITH_SLOT, RTST_RIL_REQUEST, RTST_RIL_VOID_REQUEST
 */
#define RTST_RIL_VOID_REQUEST_WITH_SLOT(_reqId, _slotId)             \
        RTST_RIL_REQUEST_WITH_SLOT(_reqId, _slotId, 1, RTST_VOID, "")

#define RTST_RIL_VOID_RESPONSE_FROM_CDMA(_reqId, _err, _serial) \
        RTST_RIL_RESPONSE_FROM_CDMA_WITH_SLOT(_reqId, RFX_SLOT_ID_0, _err, _serial, 1, RTST_VOID, "")

#define RTST_RIL_RESPONSE_FROM_CDMA_WITH_SLOT(_reqId, _slotId, _err, _serial, _dataNum, ...) \
        _RTST_RIL_RESPONSE_WITH_SLOT(ril, _reqId, _slotId, _err, RADIO_TECH_GROUP_C2K, _serial, _dataNum, __VA_ARGS__)


#define RTST_RIL_RESPONSE_FROM_CDMA_WITH_SLOT_NC(_reqId, _slotId, _err, _serial, _dataNum, ...) \
        _RTST_RIL_RESPONSE_WITH_SLOT_NC(ril, _reqId, _slotId, _err, RADIO_TECH_GROUP_C2K, _serial, _dataNum, __VA_ARGS__)

#define RTST_RIL_RESPONSE_FROM_CDMA_NC(_reqId, _err, _serial, _dataNum, ...) \
        RTST_RIL_RESPONSE_FROM_CDMA_WITH_SLOT_NC(_reqId, RFX_SLOT_ID_0, _err, _serial, _dataNum, __VA_ARGS__)

#define RTST_RIL_RESPONSE_FROM_CDMA(_reqId, _err, _serial, _dataNum, ...) \
        RTST_RIL_RESPONSE_FROM_CDMA_WITH_SLOT(_reqId, RFX_SLOT_ID_0, _err, _serial, _dataNum, __VA_ARGS__)


#define RTST_RIL_URC_FROM_CDMA_WITH_SLOT(_urcId, _slotId, _dataNum, ...) \
        _RTST_RIL_URC_WITH_SLOT(ril, _urcId, _slotId, RADIO_TECH_GROUP_C2K, _dataNum, __VA_ARGS__)


#define RTST_RIL_URC_FROM_CDMA_WITH_SLOT_NC(_urcId, _slotId, _dataNum, ...) \
        _RTST_RIL_URC_WITH_SLOT_NC(ril, _urcId, _slotId, RADIO_TECH_GROUP_C2K, _dataNum, __VA_ARGS__)

#define RTST_RIL_URC_FROM_CDMA_NC(_urcId, _dataNum, ...) \
        RTST_RIL_URC_FROM_CDMA_WITH_SLOT_NC(_urcId, RFX_SLOT_ID_0, _dataNum, __VA_ARGS__)

#define RTST_RIL_VOID_URC_FROM_CDMA(_urcId) \
        RTST_RIL_URC_FROM_CDMA(_urcId, 1, RTST_VOID, "")

#define RTST_RIL_URC_FROM_CDMA(_urcId, _dataNum, ...) \
        RTST_RIL_URC_FROM_CDMA_WITH_SLOT(_urcId, RFX_SLOT_ID_0, _dataNum, __VA_ARGS__)


#define RTST_RIL_VOID_RESPONSE_FROM_GSM(_reqId, _err, _serial) \
        RTST_RIL_RESPONSE_FROM_GSM_WITH_SLOT(_reqId, RFX_SLOT_ID_0, _err, _serial, 1, RTST_VOID, "")

#define RTST_RIL_RESPONSE_FROM_GSM_WITH_SLOT(_reqId, _slotId, _err, _serial, _dataNum, ...) \
        _RTST_RIL_RESPONSE_WITH_SLOT(ril, _reqId, _slotId, _err, RADIO_TECH_GROUP_GSM, _serial, _dataNum, __VA_ARGS__)


#define RTST_RIL_RESPONSE_FROM_GSM_WITH_SLOT_NC(_reqId, _slotId, _err, _serial, _dataNum, ...) \
        _RTST_RIL_RESPONSE_WITH_SLOT_NC(ril, _reqId, _slotId, _err, RADIO_TECH_GROUP_GSM, _serial, _dataNum, __VA_ARGS__)

#define RTST_RIL_RESPONSE_FROM_GSM_NC(_reqId, _err, _serial, _dataNum, ...) \
        RTST_RIL_RESPONSE_FROM_GSM_WITH_SLOT_NC(_reqId, RFX_SLOT_ID_0, _err, _serial, _dataNum, __VA_ARGS__)

#define RTST_RIL_RESPONSE_FROM_GSM(_reqId, _err, _serial, _dataNum, ...) \
        RTST_RIL_RESPONSE_FROM_GSM_WITH_SLOT(_reqId, RFX_SLOT_ID_0, _err, _serial, _dataNum, __VA_ARGS__)


#define RTST_RIL_URC_FROM_GSM_WITH_SLOT(_urcId, _slotId, _dataNum, ...) \
        _RTST_RIL_URC_WITH_SLOT(ril, _urcId, _slotId, RADIO_TECH_GROUP_GSM, _dataNum, __VA_ARGS__)


#define RTST_RIL_URC_FROM_GSM_WITH_SLOT_NC(_urcId, _slotId, _dataNum, ...) \
        _RTST_RIL_URC_WITH_SLOT_NC(ril, _urcId, _slotId, RADIO_TECH_GROUP_GSM, _dataNum, __VA_ARGS__)

#define RTST_RIL_URC_FROM_GSM_NC(_urcId, _dataNum, ...) \
        RTST_RIL_URC_FROM_GSM_WITH_SLOT_NC(_urcId, RFX_SLOT_ID_0, _dataNum, __VA_ARGS__)

#define RTST_RIL_VOID_URC_FROM_GSM(_urcId) \
        RTST_RIL_URC_FROM_GSM(_urcId, 1, RTST_VOID, "")

#define RTST_RIL_URC_FROM_GSM(_urcId, _dataNum, ...) \
        RTST_RIL_URC_FROM_GSM_WITH_SLOT(_urcId, RFX_SLOT_ID_0, _dataNum, __VA_ARGS__)



#define RTST_EXPECTED_RIL_REQUEST_TO_CDMA_RIL(_reqId, _token, _dataNum, ...) \
        _RTST_EXPECTED_RIL_REQUEST_TO_CDMA_RIL(ril, _reqId, _token, _dataNum, __VA_ARGS__)

#define RTST_EXPECTED_RIL_REQUEST_TO_CDMA_RIL_NC(_reqId, _token, _dataNum, ...) \
        _RTST_EXPECTED_RIL_REQUEST_TO_CDMA_RIL_NC(ril, _reqId, _token, _dataNum, __VA_ARGS__)

#define RTST_EXPECTED_VOID_RIL_REQUEST_TO_CDMA_RIL(_reqId, _token) \
        RTST_EXPECTED_RIL_REQUEST_TO_CDMA_RIL(_reqId, _token, 1, RTST_VOID, "")

#define RTST_EXPECTED_RIL_REQUEST_TO_GSM_RIL_WITH_SLOT(_reqId, _slotId, _token, _dataNum, ...) \
        _RTST_EXPECTED_RIL_REQUEST_TO_GSM_RIL(ril, _reqId, _slotId, _token, _dataNum, __VA_ARGS__)

#define RTST_EXPECTED_RIL_REQUEST_TO_GSM_RIL(_reqId, _token, _dataNum, ...) \
        RTST_EXPECTED_RIL_REQUEST_TO_GSM_RIL_WITH_SLOT(_reqId, RFX_SLOT_ID_0, _token, _dataNum, __VA_ARGS__)


#define RTST_EXPECTED_VOID_RIL_REQUEST_TO_GSM_RIL_WITH_SLOT(_reqId, _slotId, _token) \
        RTST_EXPECTED_RIL_REQUEST_TO_GSM_RIL_WITH_SLOT(_reqId, _slotId, _token, 1, RTST_VOID, "")

#define RTST_EXPECTED_VOID_RIL_REQUEST_TO_GSM_RIL(_reqId, _token) \
        RTST_EXPECTED_VOID_RIL_REQUEST_TO_GSM_RIL_WITH_SLOT(_reqId, RFX_SLOT_ID_0, _token)


#define RTST_EXPECTED_RIL_REQUEST_TO_GSM_RIL_WITH_SLOT_NC(_reqId, _slotId, _token, _dataNum, ...) \
        _RTST_EXPECTED_RIL_REQUEST_TO_GSM_RIL_NC(ril, _reqId, _slotId, _token, _dataNum, __VA_ARGS__)

#define RTST_EXPECTED_RIL_REQUEST_TO_GSM_RIL_NC(_reqId, _token, _dataNum, ...) \
        RTST_EXPECTED_RIL_REQUEST_TO_GSM_RIL_WITH_SLOT_NC(_reqId, RFX_SLOT_ID_0, _token, _dataNum, __VA_ARGS__)


#define RTST_EXPECTED_VOID_RIL_REQUEST_TO_GSM_RIL_WITH_SLOT_NC(_reqId, _slotId, _token) \
        RTST_EXPECTED_RIL_REQUEST_TO_GSM_RIL_WITH_SLOT_NC(_reqId, _slotId, _token, 1, RTST_VOID, "")

#define RTST_EXPECTED_VOID_RIL_REQUEST_TO_GSM_RIL_NC(_reqId, _token) \
        RTST_EXPECTED_VOID_RIL_REQUEST_TO_GSM_RIL_WITH_SLOT_NC(_reqId, RFX_SLOT_ID_0, _token)

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
        RTST_EXPECTED_RIL_RESPONSE_WITH_SLOT(_reqId, RFX_SLOT_ID_0, _err, _dataNum, __VA_ARGS__)

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
        RTST_EXPECTED_RIL_RESPONSE_WITH_SLOT_NC(_reqId, RFX_SLOT_ID_0, _err, _dataNum, __VA_ARGS__)
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
 *                 RFX_SLOT_ID_1,
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
        RTST_EXPECTED_RIL_VOID_RESPONSE_WITH_SLOT(_reqId, RFX_SLOT_ID_0, _err)

/*
 * Helper macro for RIL response with parameters
 *
 * SEE ALSO: RTST_EXPECTED_RIL_RESPONSE, RTST_EXPECTED_RIL_RESPONSE_WITH_SLOT
 *           RTST_EXPECTED_RIL_VOID_RESPONSE
 */
#define RTST_EXPECTED_RIL_VOID_RESPONSE_WITH_SLOT(_reqId, _slot, _err) \
        RTST_EXPECTED_RIL_RESPONSE_WITH_SLOT(_reqId, _slot, _err, 1, "void", "")


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
    RTST_EXPECTED_RIL_URC_WITH_SLOT(_urcId, RFX_SLOT_ID_0, _dataNum, __VA_ARGS__)


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
    RTST_EXPECTED_RIL_URC_WITH_SLOT_NC(_urcId, RFX_SLOT_ID_0, _dataNum, __VA_ARGS__)


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
 *             RFX_SLOT_ID_1,
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
        RTST_EXPECTED_RIL_VOID_URC_WITH_SLOT(_urcId, RFX_SLOT_ID_0)

/*
 * Helper macro for RIL URC without parameters
 *
 * SEE ALSO: RTST_EXPECTED_RIL_URC, RTST_EXPECTED_RIL_URC_WITH_SLOT
 *           RTST_EXPECTED_RIL_VOID_URC
 *           RTST_EXPECTED_RIL_URC_NC, RTST_EXPECTED_RIL_URC_WITH_SLOT_NC
 */
#define RTST_EXPECTED_RIL_VOID_URC_WITH_SLOT(_urcId, _slot) \
        RTST_EXPECTED_RIL_URC_WITH_SLOT(_urcId, _slot, 1, RTST_VOID, "")

#define RTST_CLEAN_CDMA_RIL_SOCKET_DATA() \
        RtstEnv::get()->cleanCdmaRilSocket()

#define RTST_CLEAN_GSM_RIL_SOCKET_DATA() \
        RtstEnv::get()->cleanGsmRilSocket()


#define RTST_SET_POLL_FD_TIME(_time) \
        RtstEnv::get()->setFdTimeoutValue(_time)



// Internal Use
#define _RTST_CASE_BEGIN(_name, _rfx_assert, _step_test)                                      \
        do {                                                                                  \
            RFX_LOG_D(RTST_TAG, "RTST_CASE_BEGIN line = %d", __LINE__);                       \
            RtstEnv::get()->init();                                                           \
            RtstCase _case##_name(#_name, _rfx_assert, _step_test);                           \
            RTST_CLEAN_CDMA_RIL_SOCKET_DATA();                                                \
            RTST_CLEAN_GSM_RIL_SOCKET_DATA();


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
#define _RTST_RIL_RESPONSE_WITH_SLOT(_name, _id, _slot, _err, _src, _serial, _dataNum, ...)   \
        do {                                                                                  \
            RtstRilRspItem* pItem = new RtstRilRspItem(_id, _slot, _err, _src, _serial);      \
            AssertionResult r = _case##_name.stepRun(pItem, _dataNum, __VA_ARGS__);           \
            ASSERT_TRUE(r);                                                                   \
        } while(0)

// Internal Use
#define _RTST_RIL_RESPONSE_WITH_SLOT_NC(_name, _id, _slot, _err, _src, _serial, _dataNum, ...)   \
        do {                                                                                     \
            RtstRilRspItem* pItem = new RtstRilRspItem(_id, _slot, _err, _src, _serial);         \
            pItem->getDataSequency().setSeqCalType(RtstDataSequency::FORCE_NON_COUNTABLE);       \
            AssertionResult r = _case##_name.stepRun(pItem, _dataNum, __VA_ARGS__);              \
            ASSERT_TRUE(r);                                                                      \
        } while(0)


// Internal Use
#define _RTST_RIL_URC_WITH_SLOT(_name, _id, _slot, _src, _dataNum, ...)                      \
        do {                                                                                 \
            RtstRilUrcItem* pItem = new RtstRilUrcItem(_id, _slot, _src);                    \
            AssertionResult r = _case##_name.stepRun(pItem, _dataNum, __VA_ARGS__);          \
            ASSERT_TRUE(r);                                                                  \
        } while(0)

// Internal Use
#define _RTST_RIL_URC_WITH_SLOT_NC(_name, _id, _slot, _src, _dataNum, ...)                   \
        do {                                                                                 \
            RtstRilUrcItem* pItem = new RtstRilUrcItem(_id, _slot, _src);                    \
            pItem->getDataSequency().setSeqCalType(RtstDataSequency::FORCE_NON_COUNTABLE);   \
            AssertionResult r = _case##_name.stepRun(pItem, _dataNum, __VA_ARGS__);          \
            ASSERT_TRUE(r);                                                                  \
        } while(0)




// Internal Use
#define _RTST_EXPECTED_RIL_REQUEST_TO_CDMA_RIL(_name, _id, _token, _dataNum, ...)             \
        do {                                                                                  \
            RtstExpectedCdmaRilReqItem* pItem = new RtstExpectedCdmaRilReqItem(_id, 0);       \
            AssertionResult r = _case##_name.stepRun(pItem, _dataNum, __VA_ARGS__);           \
            _token = pItem->getToken();                                                       \
            ASSERT_TRUE(r);                                                                   \
        } while(0)

// Internal Use
#define _RTST_EXPECTED_RIL_REQUEST_TO_CDMA_RIL_NC(_name, _id, _token, _dataNum, ...)          \
        do {                                                                                  \
            RtstExpectedCdmaRilReqItem* pItem = new RtstExpectedCdmaRilReqItem(_id, 0);       \
            pItem->getDataSequency().setSeqCalType(RtstDataSequency::FORCE_NON_COUNTABLE);    \
            AssertionResult r = _case##_name.stepRun(pItem, _dataNum, __VA_ARGS__);           \
            _token = pItem->getToken();                                                       \
            ASSERT_TRUE(r);                                                                   \
        } while(0)

// Internal Use
#define _RTST_EXPECTED_RIL_REQUEST_TO_GSM_RIL(_name, _id, _slot, _token, _dataNum, ...)       \
        do {                                                                                  \
            RtstExpectedGsmRilReqItem* pItem = new RtstExpectedGsmRilReqItem(_id, _slot);     \
            AssertionResult r = _case##_name.stepRun(pItem, _dataNum, __VA_ARGS__);           \
            _token = pItem->getToken();                                                       \
            ASSERT_TRUE(r);                                                                   \
        } while(0)

// Internal Use
#define _RTST_EXPECTED_RIL_REQUEST_TO_GSM_RIL_NC(_name, _id, _token, _dataNum, ...)           \
        do {                                                                                  \
            RtstExpectedGsmRilReqItem* pItem = new RtstExpectedGsmRilReqItem(_id, 0);         \
            pItem->getDataSequency().setSeqCalType(RtstDataSequency::FORCE_NON_COUNTABLE);    \
            AssertionResult r = _case##_name.stepRun(pItem, _dataNum, __VA_ARGS__);           \
            _token = pItem->getToken();                                                       \
            ASSERT_TRUE(r);                                                                   \
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
#define _RTST_EXPECTED_RIL_URC_WITH_SLOT(_name, _urcId, _slot, _dataNum, ...)             \
        do {                                                                              \
            RtstExpectedRilUrcItem *pItem = new RtstExpectedRilUrcItem(_urcId, _slot);    \
            AssertionResult r = _case##_name.stepRun(pItem, _dataNum, __VA_ARGS__);       \
            ASSERT_TRUE(r);                                                               \
        } while(0)

// Internal Use
#define _RTST_EXPECTED_INIT_RIL_URC_WITH_SLOT(_name, _urcId, _slot, _dataNum, ...)              \
        do {                                                                                    \
            RtstExpectedInitRilUrcItem *pItem = new RtstExpectedInitRilUrcItem(_urcId, _slot);  \
            AssertionResult r = _case##_name.stepRun(pItem, _dataNum, __VA_ARGS__);             \
            ASSERT_TRUE(r);                                                                     \
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
#define _RTST_CASE_END(_name)                                            \
            AssertionResult r = _case##_name.finalRun();                 \
            ASSERT_TRUE(r);                                              \
            RtstEnv::get()->releaseRequestInfo();                        \
            RFX_LOG_D(RTST_TAG, "RTST_CASE_END line = %d", __LINE__);    \
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
 * Class RtstExpectedCdmaRilReqItem
 *****************************************************************************/
class RtstExpectedCdmaRilReqItem : public RtstRilItemBase {
public:
    void setToken(int token) {
        m_token = token;
    }

    int getToken() {
        return m_token;
    }
// Constructor / Destructor
public:
    // Construct a RIL request item
    RtstExpectedCdmaRilReqItem(
        int reqId,        // [IN] RIL request ID
        int slotId        // [IN] slot ID
    ) : RtstRilItemBase(reqId, slotId) {
    }

    virtual ~RtstExpectedCdmaRilReqItem() {}

// Override
public:
    virtual bool isExpectedType() {
        return true;
    }

    virtual AssertionResult checkData();

private:
    int m_token;
};


/*****************************************************************************
 * Class RtstExpectedGsmRilReqItem
 *****************************************************************************/
class RtstExpectedGsmRilReqItem : public RtstRilItemBase {
public:
    void setToken(int token) {
        m_token = token;
    }

    int getToken() {
        return m_token;
    }
// Constructor / Destructor
public:
    // Construct a RIL request item
    RtstExpectedGsmRilReqItem(
        int reqId,        // [IN] RIL request ID
        int slotId        // [IN] slot ID
    ) : RtstRilItemBase(reqId, slotId) {
    }

    virtual ~RtstExpectedGsmRilReqItem() {}

// Override
public:
    virtual bool isExpectedType() {
        return true;
    }

    virtual AssertionResult checkData();

private:
    int m_token;
};


/*****************************************************************************
 * Class RtstRilRspItem
 *****************************************************************************/
/*
 * RIL response Item
 */
class RtstRilRspItem : public RtstRilItemBase {
// External Method
public:
    // Get the error code for the RIL Req
    //
    // RETURNS: RIL error code
    int getErrorCode() const {
        return m_errorCode;
    }

    RILD_RadioTechnology_Group getSource() const {
        return m_source;
    }

    int getSerial() const {
        return m_serial;
    }

// Constructor / Destructor
public:
    // Construct a base RIL item
    RtstRilRspItem(
        int reqId,        // [IN] RIL request ID
        int slotId,       // [IN] slot ID
        int errorCode,     // [IN] error code of the RIL response
        RILD_RadioTechnology_Group source,
        int serial
    ) : RtstRilItemBase(reqId, slotId),
        m_errorCode(errorCode), m_source(source), m_serial(serial) {
    }

    virtual ~RtstRilRspItem() {}

// Override
public:
    virtual bool isMockType() {
        return true;
    }

    virtual void sendData();

// Implementation
private:
    int m_errorCode;
    RILD_RadioTechnology_Group m_source;
    int m_serial;
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
 * Class RtstRilUrcItem
 *****************************************************************************/
class RtstRilUrcItem : public RtstRilItemBase {
public:
    RILD_RadioTechnology_Group getSource() const {
        return m_source;
    }

// Constructor / Destructor
public:
    // Construct a expected RIL URC item
    RtstRilUrcItem(
        int urcId,        // [IN] URC ID
        int slotId,        // [IN] slot ID
        RILD_RadioTechnology_Group source
    ) : RtstRilItemBase(urcId, slotId), m_source(source) {
    }

    virtual ~RtstRilUrcItem() {}
// Override
public:
    virtual bool isMockType() {
        return true;
    }

    virtual void sendData();

private:
    RILD_RadioTechnology_Group m_source;
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

    // Check if use RFX_ASSERT
    //
    // RETURNS: true if use RFX_ASSERT, or use gtest ASSERT
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
