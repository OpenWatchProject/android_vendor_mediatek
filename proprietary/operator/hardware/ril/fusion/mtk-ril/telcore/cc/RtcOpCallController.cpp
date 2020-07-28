/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
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

#include "RtcOpCallController.h"
#include "RtcOpRedialController.h"

 /*****************************************************************************
 * Class RtcOpCallController
 *****************************************************************************/

#define RFX_LOG_TAG "RtcOpCC"

RFX_IMPLEMENT_CLASS("RtcOpCallController", RtcOpCallController, RtcCallController);

RtcOpCallController::RtcOpCallController() {
}

RtcOpCallController::~RtcOpCallController() {
}

void RtcOpCallController::onInit() {
    RtcCallController::onInit();
    logD(RFX_LOG_TAG, "init()");
    static const int request_id_list[] = {
        RFX_MSG_REQUEST_DIAL_FROM,
        RFX_MSG_REQUEST_SET_EMERGENCY_CALL_CONFIG,
        RFX_MSG_REQUEST_DEVICE_SWITCH,
        RFX_MSG_REQUEST_CANCEL_DEVICE_SWITCH,
        RFX_MSG_REQUEST_EMERGENCY_DIAL,
        RFX_MSG_REQUEST_DIAL,
        RFX_MSG_REQUEST_SET_INCOMING_VIRTUAL_LINE
    };

    static const int urc_id_list[] = {
        RFX_MSG_UNSOL_INCOMING_CALL_INDICATION,
        RFX_MSG_UNSOL_QUERY_TRN
    };


    registerToHandleRequest(request_id_list,
            sizeof(request_id_list) / sizeof(const int));
    registerToHandleUrc(urc_id_list,
            sizeof(urc_id_list)/sizeof(const int));
}

void RtcOpCallController::createRedialController() {
    RFX_OBJ_CREATE(mRedialCtrl, RtcOpRedialController, this);
}

bool RtcOpCallController::onHandleRequest(const sp<RfxMessage>& message) {
    int msg_id = message->getId();
    //logD(RFX_LOG_TAG, "onHandleRequest: %s", RFX_ID_TO_STR(msg_id));

    switch (msg_id) {
        case RFX_MSG_REQUEST_DIAL:
            if (rejectDualDialForDSDS()) {
                responseDialFailed(message);
                return true;
            }
            if (!isOp08Support() || !RfxRilUtils::isDigitsSupport()
                    || !handleOp08DialRequest(message)) {
                return RtcCallController::onHandleRequest(message);
            } else {
                return true;
            }
            break;
        case RFX_MSG_REQUEST_DIAL_FROM:
            if (rejectDualDialForDSDS()) {
                responseDialFailed(message);
                return true;
            }
            handleImsDialRequest(message);
            break;
        default:
            return RtcCallController::onHandleRequest(message);
    }
    updateIsImsCallExistToStatusManager(message->getSlotId());
    requestToMcl(message);
    return true;
}

bool RtcOpCallController::onHandleUrc(const sp<RfxMessage>& message) {
    int msg_id = message->getId();
    //int slotId = message->getSlotId();
    //logD(RFX_LOG_TAG, "onHandleUrc: %s", RFX_ID_TO_STR(msg_id));

    switch (msg_id) {
        case RFX_MSG_UNSOL_INCOMING_CALL_INDICATION:
            if (!isOp08Support() || !RfxRilUtils::isDigitsSupport()
                    || !handleOp08IncomingCall(message)) {
                return RtcCallController::onHandleUrc(message);
            } else {
                return true;
            }
            break;
        case RFX_MSG_UNSOL_QUERY_TRN:
            handleQueryTrn(message);
            return true;
        default:
           return RtcCallController::onHandleUrc(message);
    }

    //If other URC should be also handled here, and the following method
    //should be called, please remember to remove the annotation marks.
    //responseToRilJAndUpdateIsImsCallExist(message);
    //return true;
}

bool RtcOpCallController::onHandleResponse(const sp<RfxMessage>& message) {
    int msg_id = message->getId();
    //logD(RFX_LOG_TAG, "onHandleResponse: %s", RFX_ID_TO_STR(msg_id));

    switch (msg_id) {
        case RFX_MSG_REQUEST_DIAL_FROM:
            if (message->getError() != RIL_E_SUCCESS) {
                imsCallEstablishFailed(message->getSlotId());
                responseToRilJAndUpdateIsImsCallExist(message);
                return true;
            }
            break;
        default:
            return RtcCallController::onHandleResponse(message);
    }
    responseToRilj(message);
    return true;
}

bool RtcOpCallController::handleOp08DialRequest(const sp<RfxMessage>& message) {
    RIL_Dial *pDial = (RIL_Dial*) (message->getData()->getData());

    char *fromMsisdn;
    char *toMsisdn;
    int pos = 0;

    // If OP08 dialString matches specific format, it means to be a Digits line number
    // Query TRN via RtcOpImsController and block DIAL_REQUEST response until TRN is retrieved.
    // During this period, GsmCdmaCallTracker won't allow to poll calls or hangup

    int len = strlen(pDial->address);
    char* tmp = strstr(pDial->address, "_to_");
    if (tmp == NULL) {
        return false;
    } else {
        pos = (tmp - pDial->address) / sizeof(char);
        fromMsisdn = (char *)alloca(pos + 1);
        toMsisdn = (char *)alloca(len - pos - 3);
        memset(fromMsisdn, 0, pos + 1);
        memset(toMsisdn, 0, len - pos - 3);
        strncpy(fromMsisdn, pDial->address, pos);
        strncpy(toMsisdn, tmp + 4, len - pos - 4);
            logD(RFX_LOG_TAG, "dialString:%s, from:%s, to:%s",
                    pDial->address, fromMsisdn, toMsisdn);
    }

    // TODO:
    // Dial request won't return at once until TRN query done / timeout.
    // If user hangup the call very fast within the above period,
    // since mPendingMO in GsmCdmaCallTracker is NOT NULL, the hangup request will be pending.
    // If Dial request returns SUCCESS before TRN query done, need to fake GET_CURRENT_CALLS resp.
    if (mNoTrnTimeoutHandler.timerHandle != NULL) {
        RfxTimer::stop(mNoTrnTimeoutHandler.timerHandle);
        mNoTrnTimeoutHandler.timerHandle = NULL;
        mNoTrnTimeoutHandler.message = NULL;
    }

    mNoTrnTimeoutHandler.message = message;
    mNoTrnTimeoutHandler.timerHandle = RfxTimer::start(RfxCallback0(this,
                &RtcOpCallController::onGetTrnInternalTimeout), ms2ns(GET_TRN_TIMEOUT));

    sp<RfxAction> action;
    RtcOpImsController* opImsController =
            (RtcOpImsController *)findController(RFX_OBJ_CLASS_INFO(RtcOpImsController));
    action = new RfxAction1<const sp<RfxMessage>>(this,
            &RtcOpCallController::onGetTrnInternal, message);
    opImsController->getTrn(fromMsisdn, toMsisdn, pDial->address, action);
    return true;
}

void RtcOpCallController::onGetTrnInternal(const sp<RfxMessage> origMsg) {
    logD(RFX_LOG_TAG, "onGetTrnInternal");
    if (mNoTrnTimeoutHandler.timerHandle != NULL) {
        RfxTimer::stop(mNoTrnTimeoutHandler.timerHandle);
        mNoTrnTimeoutHandler.timerHandle = NULL;
        mNoTrnTimeoutHandler.message = NULL;
    } else {
        return;
    }

    int slotId = origMsg->getSlotId(); // get sim slot id.
    RIL_Dial *pDial = (RIL_Dial*) (origMsg->getData()->getData());

    String8 trnString =getStatusManager(slotId)->getString8Value(RFX_STATUS_KEY_TRN);

    free(pDial->address);
    int len = (trnString.length() > MAX_TRN_LENGTH) ? MAX_TRN_LENGTH : trnString.length();
    pDial->address = (char *)calloc(len + 1, sizeof(char));
    RFX_ASSERT(pDial->address != NULL);
    strncpy(pDial->address, trnString.string(), len);

    updateIsImsCallExistToStatusManager(origMsg->getSlotId());
    requestToMcl(origMsg);
}

void RtcOpCallController::onGetTrnInternalTimeout() {
    logD(RFX_LOG_TAG, "onGetTrnInternalTimeout");
    if (mNoTrnTimeoutHandler.message != NULL) {
        sp<RfxMessage> responseMsg = RfxMessage::obtainResponse(RIL_E_GENERIC_FAILURE,
                mNoTrnTimeoutHandler.message, true);
        responseToRilj(responseMsg);
    }
    mNoTrnTimeoutHandler.timerHandle = NULL;
    mNoTrnTimeoutHandler.message = NULL;
}

void RtcOpCallController::onGetTrnForModem(const char* fromMsisdn, const char* toMsisdn) {
    logD(RFX_LOG_TAG, "onGetTrnForModem");

    String8 trnString =getStatusManager(getSlotId())->getString8Value(RFX_STATUS_KEY_TRN);

    char *msg_data[3];
    msg_data[0] = strdup(fromMsisdn);
    msg_data[1] = strdup(toMsisdn);
    msg_data[2] = strdup(trnString.string());

    sp<RfxMessage> msg = RfxMessage::obtainRequest(getSlotId(),
            RFX_MSG_REQUEST_SET_TRN,
            RfxStringsData(msg_data, 3));
    requestToMcl(msg);
}

void RtcOpCallController::handleQueryTrn(const sp<RfxMessage>& message) {
    /* +ECPI: <call_id>,<number>,<type>,<call_mode>,<seq_no>, [<redirect_num>] */
    RfxStringsData* data = (RfxStringsData*)message->getData();
    char** params = (char**)data->getData();

    int len = strlen(params[0]) + strlen(params[1]) + 4;
    char* address = (char *)alloca(len + 1);
    memset(address, 0, len + 1);
    strncpy(address, params[0], strlen(params[0]));
    strncat(address, "_to_", 4);
    strncat(address, params[1], len - strlen(params[1]));

    sp<RfxAction> action;
    RtcOpImsController* opImsController =
            (RtcOpImsController *)findController(RFX_OBJ_CLASS_INFO(RtcOpImsController));
    action = new RfxAction2<const char*, const char*>(this,
            &RtcOpCallController::onGetTrnForModem, params[0], params[1]);
    opImsController->getTrn(params[0], params[1], address, action);
}

bool RtcOpCallController::handleOp08IncomingCall(const sp<RfxMessage>& message) {
    /* +EAIC: <call_id>,<number>,<type>,<call_mode>,<seq_no>, [<redirect_num>] */
    RfxStringsData* data = (RfxStringsData*)message->getData();
    char** params = (char**)data->getData();
    int callId = atoi(params[0]);
    int seqNo = atoi(params[4]);

    if (mNoTrnTimeoutHandler.timerHandle == NULL) {
        return false;
    }

    logD(RFX_LOG_TAG, "reject MT on slot%d, since TRN query is ongoing", m_slot_id);
    //"AT+EAIC=1,callId,seqNo
    int msg_data[4];
    msg_data[0] = 1;  //disapprove
    msg_data[1] = callId;
    msg_data[2] = seqNo;
    msg_data[3] = -1;
    sp<RfxMessage> msg = RfxMessage::obtainRequest(getSlotId(),
            RFX_MSG_REQUEST_SET_CALL_INDICATION,
            RfxIntsData(msg_data, 4));
    requestToMcl(msg);
    return true;
}
