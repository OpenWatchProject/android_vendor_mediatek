/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RpCdmaSmsLocalReqRsp.h"
#include "RfxLog.h"

/*****************************************************************************
 * Class RpCdmaSmsLocalReqRsp
 *****************************************************************************/

RpCdmaSmsLocalReqRsp::RpCdmaSmsLocalReqRsp() :
    mDebug(true),
    mType(CDMA_SMS_LOCAL_REQ_NONE),
    mResult(false),
    mId(-1),
    mAckPdu(String8("")),
    mErrorCode(0),
    mErrorClass(0) {
}

void RpCdmaSmsLocalReqRsp::setType(int type) {
    mType = type;
}

void RpCdmaSmsLocalReqRsp::setResult(bool result) {
    mResult = result;
}

void RpCdmaSmsLocalReqRsp::setId(int id) {
    mId = id;
}

void RpCdmaSmsLocalReqRsp::setAckPdu(String8 pdu) {
    mAckPdu = pdu;
}

void RpCdmaSmsLocalReqRsp::setErrorCode(int errorCode) {
    mErrorCode = errorCode;
}

void RpCdmaSmsLocalReqRsp::setErrorClass(int errorClass) {
    mErrorClass = errorClass;
}

int RpCdmaSmsLocalReqRsp::getType() {
    return mType;
}

int RpCdmaSmsLocalReqRsp::getResult() {
    return mResult;
}

int RpCdmaSmsLocalReqRsp::getId() {
    return mId;
}

String8 RpCdmaSmsLocalReqRsp::getAckPdu() {
    return mAckPdu;
}

int RpCdmaSmsLocalReqRsp::getErrorCode() {
    return mErrorCode;
}

int RpCdmaSmsLocalReqRsp::getErrorClass() {
    return mErrorClass;
}

int RpCdmaSmsLocalReqRsp::getError() {
    /**
     * Return the error code directly now.
     * But it may need to combine the error class and error code together in the future. We keep
     * the extension here.
     */
    return mErrorCode;
}

String8 RpCdmaSmsLocalReqRsp::toString() {
    return String8::format(
            "Type: %s, Result: %d, Id: %d, AckPdu: %s, Error Code: %d, Error class: %d, Error: %d",
            typeToString(getType()).string(), getResult(), getId(), getAckPdu().string(),
            getErrorCode(), getErrorClass(), getError());
}

String8 RpCdmaSmsLocalReqRsp::typeToString(int type) {
    switch (type) {
        case CDMA_SMS_LOCAL_REQ_CMGS_RSP: return String8("CDMA_SMS_LOCAL_REQ_CMGS_RSP");
        case CDMA_SMS_LOCAL_REQ_CNMA_RSP: return String8("CDMA_SMS_LOCAL_REQ_CNMA_RSP");

        default: return String8("CDMA_SMS_LOCAL_REQ_NONE");
    }
}

void RpCdmaSmsLocalReqRsp::log(String8 s) {
    if (mDebug) RFX_LOG_D("RpCdmaSmsLocalReqRsp: ", "%s", s.string());
}
