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
#include "RtcImsSmsController.h"
#include <telephony/mtk_ril.h>
#include "RfxMessageId.h"
#include "RfxImsSmsData.h"
#include "RfxSmsRspData.h"
#include "rfx_properties.h"
#include "RfxVoidData.h"
#include "RfxRilUtils.h"

using ::android::String8;

RFX_IMPLEMENT_CLASS("RtcImsSmsController", RtcImsSmsController, RfxController);


// Register solicited message
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxImsSmsData, RfxSmsRspData, \
        RFX_MSG_REQUEST_IMS_SEND_SMS);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxImsSmsData, RfxSmsRspData, \
        RFX_MSG_REQUEST_IMS_SEND_SMS_EX);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxVoidData, \
        RFX_MSG_REQUEST_SET_SMS_FWK_READY);


/*****************************************************************************
 * Class RtcImsSmsController
 *****************************************************************************/
RtcImsSmsController::RtcImsSmsController() {
        setTag(String8("RtcImsSmsCtrl"));
        mGsmCtrl = NULL;
        mCdmaCtrl = NULL;
}


RtcImsSmsController::~RtcImsSmsController() {
}

void RtcImsSmsController::onInit() {
    // Required: invoke super class implementation
    RfxController::onInit();

    RFX_OBJ_CREATE(mGsmCtrl, RtcGsmSmsController, this);
    RFX_OBJ_CREATE(mCdmaCtrl, RtcCdmaSmsController, this);
    RFX_OBJ_CREATE(mConCatSmsRoot, RtcConCatSmsRoot, this);

    const int request_id_list[] = {
        RFX_MSG_REQUEST_IMS_SEND_SMS,
        RFX_MSG_REQUEST_IMS_SEND_SMS_EX,
        RFX_MSG_REQUEST_SET_SMS_FWK_READY,
    };

    // register request & URC id list
    // NOTE. one id can only be registered by one controller
    if (RfxRilUtils::isSmsSupport()) {
        registerToHandleRequest(request_id_list, sizeof(request_id_list)/sizeof(const int));
    }
}

bool RtcImsSmsController::onCheckIfRejectMessage(
        const sp<RfxMessage>& message, bool isModemPowerOff, int radioState) {
    int msgId = message->getId();
    bool isWfcSupport = RfxRilUtils::isWfcSupport();

    if (!isModemPowerOff && (radioState == (int)RADIO_STATE_OFF) &&
            (msgId == RFX_MSG_REQUEST_IMS_SEND_SMS || msgId == RFX_MSG_REQUEST_IMS_SEND_SMS_EX) &&
            (isWfcSupport)) {
        logD(mTag, "onCheckIfRejectMessage, isModemPowerOff %d, isWfcSupport %d",
                isModemPowerOff, isWfcSupport);
        return false;
    } else if (msgId == RFX_MSG_REQUEST_SET_SMS_FWK_READY) {
        logD(mTag, "Always let RFX_MSG_REQUEST_SET_SMS_FWK_READY go!");
        return false;
    }

    return RfxController::onCheckIfRejectMessage(message, isModemPowerOff, radioState);
}

bool RtcImsSmsController::onHandleRequest(const sp<RfxMessage>& msg) {
    int msg_id = msg->getId();
    switch (msg_id) {
        case RFX_MSG_REQUEST_IMS_SEND_SMS:
        case RFX_MSG_REQUEST_IMS_SEND_SMS_EX:
            {
                RIL_IMS_SMS_Message *pIms = (RIL_IMS_SMS_Message*)msg->getData()->getData();
                if (pIms->tech == RADIO_TECH_3GPP) {
                    /*
                     * CDMA call is on-going, RF is occupied by CDMA call, IMS SMS will fail
                     * retry SMS in CS domain directly.
                     */
                    if (isInCdmaCall()) {
                        sp<RfxMessage> rsp =
                                RfxMessage::obtainResponse(RIL_E_SMS_SEND_FAIL_RETRY, msg);
                        responseToRilj(rsp);
                        break;
                    }
                    mGsmCtrl->handleRequest(msg);
                } else {
                    mCdmaCtrl->handleRequest(msg);
                }
            }
            break;
        case RFX_MSG_REQUEST_SET_SMS_FWK_READY: {
                mGsmCtrl->handleRequest(msg);
                sp<RfxMessage> resToRilj;
                resToRilj = RfxMessage::obtainResponse(m_slot_id,
                        RFX_MSG_REQUEST_SET_SMS_FWK_READY, RIL_E_SUCCESS, RfxVoidData(), msg);
                responseToRilj(resToRilj);
            }
            break;
        default:
            logD(mTag, "Not Support the req %d", msg_id);
            break;
    }
    return true;
}

bool RtcImsSmsController::onPreviewMessage(const sp<RfxMessage>& msg) {
    int msg_id = msg->getId();
    switch (msg_id) {
        case RFX_MSG_REQUEST_IMS_SEND_SMS:
        case RFX_MSG_REQUEST_IMS_SEND_SMS_EX:
            {
                RIL_IMS_SMS_Message *pIms = (RIL_IMS_SMS_Message*)msg->getData()->getData();
                if (pIms->tech == RADIO_TECH_3GPP2) {
                    return mCdmaCtrl->previewMessage(msg);
                } else {
                    return mGsmCtrl->previewMessage(msg);
                }
            }
            break;
        case RFX_MSG_REQUEST_SET_SMS_FWK_READY:
            break;
        default:
            logD(mTag, "Not Support the req %d", msg_id);
            break;
    }
    return true;

}

bool RtcImsSmsController::onCheckIfResumeMessage(const sp<RfxMessage>& msg) {
    int msg_id = msg->getId();
    switch (msg_id) {
        case RFX_MSG_REQUEST_IMS_SEND_SMS:
        case RFX_MSG_REQUEST_IMS_SEND_SMS_EX:
            {
                RIL_IMS_SMS_Message *pIms = (RIL_IMS_SMS_Message*)msg->getData()->getData();
                if (pIms->tech == RADIO_TECH_3GPP2) {
                    return mCdmaCtrl->checkIfResumeMessage(msg);
                } else {
                    return mGsmCtrl->checkIfResumeMessage(msg);
                }
            }
            break;
        default:
            logD(mTag, "Not Support the req %d", msg_id);
            break;
    }
    return false;
}

bool RtcImsSmsController::hasImsCall() {
    return getStatusManager()->getBoolValue(RFX_STATUS_KEY_IMS_CALL_EXIST, false);
}

bool RtcImsSmsController::isCdmaRat() {
    RfxNwServiceState defaultServiceState (0, 0, 0 ,0);
    RfxNwServiceState ss = getStatusManager()
            ->getServiceStateValue(RFX_STATUS_KEY_SERVICE_STATE, defaultServiceState);
    int voiceRadioTech = ss.getRilVoiceRadioTech();
    return RfxNwServiceState::isCdmaGroup(voiceRadioTech);
}

bool RtcImsSmsController::isInCall() {
    return (getStatusManager()->getIntValue(RFX_STATUS_KEY_VOICE_CALL_COUNT, 0) > 0);
}

bool RtcImsSmsController::isInCdmaCall() {
    bool inCall = isInCall();
    bool imsCall = hasImsCall();
    bool cdmaRat = isCdmaRat();
    logD(mTag, "inCall = %d, imsCall = %d, cdmaRat = %d", inCall, imsCall, cdmaRat);
    return inCall && !imsCall && cdmaRat;
}

bool RtcImsSmsController::removeReferenceIdCached(int ref) {
    for (Vector<int>::iterator iter = mImsSmsRefIdVector.begin();
            iter != mImsSmsRefIdVector.end();
            ++iter) {
        if (*iter == ref) {
            mImsSmsRefIdVector.erase(iter);
            return true; //found it and erase it
        }
    }
    return false;
}

void RtcImsSmsController::addReferenceId(int ref) {
    mImsSmsRefIdVector.push_back(ref);
}

int RtcImsSmsController::getCacheSize() {
    return mImsSmsRefIdVector.size();
}

void RtcImsSmsController::sendCdmaSms(RtcCdmaSmsMessage *msg) {
    RFX_ASSERT(msg != NULL);
    mCdmaCtrl->sendCdmaSms(msg);
}

void RtcImsSmsController::sendGsmSms(RtcGsmSmsMessage *msg) {
    RFX_ASSERT(msg != NULL);
    mGsmCtrl->sendGsmSms(msg);
}

void RtcImsSmsController::sendCdmaSmsAck(const sp<RfxMessage>& message) {
    mCdmaCtrl->sendCdmaSmsAck(message);
}

void RtcImsSmsController::sendGsmSmsAck(int success, int cause, const sp<RfxMessage>& message) {
    mGsmCtrl->sendGsmSmsAck(success, cause, message);
}

bool RtcImsSmsController::isSupportSmsFormatConvert() {
    const char *supportedPlmn[] = {
        "46003",
        "46011",
        "45502",
        "45507",
        "31000",
        "310120",
        "311870",
        "311880",
        "312010",
        "312190",
        "312530"
    };
    bool ret = false;
    String8 oper(getStatusManager()->getString8Value(RFX_STATUS_KEY_OPERATOR));
    for (size_t i = 0; i < sizeof(supportedPlmn) / sizeof(supportedPlmn[0]); i++) {
        if (0 == strcmp(supportedPlmn[i], oper.string())) {
            ret = true;
            break;
        }
    }
    logD(mTag, "isSupportSmsFormatConvert %d", ret);
    return ret;
}