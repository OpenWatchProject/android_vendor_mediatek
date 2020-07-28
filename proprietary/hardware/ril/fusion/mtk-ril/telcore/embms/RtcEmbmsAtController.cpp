/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2015. All rights reserved.
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

#include "RtcEmbmsAtController.h"
#include "RtcEmbmsUtils.h"
#include "RfxStatusDefs.h"
#include "RfxMainThread.h"

#include "RfxMisc.h"
#include <telephony/mtk_ril.h>
#include <sys/socket.h>
#include "rfx_properties.h"
#include <sys/stat.h>
#include "RfxMessageId.h"
#include "RfxAtLine.h"
#include "RfxIntsData.h"
#include "RfxStringData.h"
#include "RfxVoidData.h"
#include "embms/RfxEmbmsGetCoverageRespData.h"
#include "embms/RfxEmbmsLocalEnableRespData.h"
#include "embms/RfxEmbmsDisableRespData.h"
#include "embms/RfxEmbmsLocalStartSessionReqData.h"
#include "embms/RfxEmbmsLocalStartSessionRespData.h"
#include "embms/RfxEmbmsLocalStopSessionReqData.h"
#include "embms/RfxEmbmsGetTimeRespData.h"
#include "embms/RfxEmbmsLocalSessionNotifyData.h"
#include "embms/RfxEmbmsLocalSaiNotifyData.h"
#include "embms/RfxEmbmsLocalOosNotifyData.h"

//extern "C" {
//#include "at_tok.h"
//}

#define RFX_LOG_TAG "RtcEmbmsAt"
#define VDBG 1

#define EMBMS_OK        "OK\n"
#define EMBMS_ERROR     "ERROR\n"
#define EMBMS_FLIGTH_MODE   "%MBMSEV:99\n"

/*****************************************************************************
 * Class RfxDataController
 * The class is created if the slot is single mode, LWG or C,
 * During class life time always communicate with one modem, gsm or c2k.
 *****************************************************************************/

RFX_IMPLEMENT_CLASS("RtcEmbmsAtController", RtcEmbmsAtController, RfxController);

RtcEmbmsAtController::RtcEmbmsAtController() :
    mSessionInfoList(NULL),
    mIsActiveSession(false),
    mIsFlightOn(false),
    mIsEmbmsSupport(false),
    mEmbmsEnabled(false),
    mSdkVersion(11001){
    // If you want to debug like old behavior.
    //mSdkVersion = 10603;
    mSdkVersion = 11001;
}

RtcEmbmsAtController::~RtcEmbmsAtController() {
}

void RtcEmbmsAtController::onInit() {
    RfxController::onInit();  // Required: invoke super class implementation

    logI(RFX_LOG_TAG, "onInit:mSdkVersion:%d", mSdkVersion);
    mSessionInfoList = new Vector<RtcEmbmsSessionInfo*>();

    const int request_id_list[] = {
            RFX_MSG_REQUEST_EMBMS_AT_CMD,
            RFX_MSG_REQUEST_EMBMS_ENABLE,
            RFX_MSG_REQUEST_EMBMS_DISABLE,
            RFX_MSG_REQUEST_EMBMS_START_SESSION,
            RFX_MSG_REQUEST_EMBMS_STOP_SESSION,
            RFX_MSG_REQUEST_EMBMS_GET_TIME,
            RFX_MSG_REQUEST_RTC_EMBMS_GET_COVERAGE_STATE,
            RFX_MSG_REQUEST_EMBMS_TRIGGER_CELL_INFO_NOTIFY,
            };
    const int urc_id_list[] = {
            RFX_MSG_URC_EMBMS_AT_INFO,
            RFX_MSG_URC_EMBMS_START_SESSION_RESPONSE,
            RFX_MSG_URC_EMBMS_CELL_INFO_NOTIFICATION,
            RFX_MSG_URC_EMBMS_COVERAGE_STATE,
            RFX_MSG_URC_EMBMS_ACTIVE_SESSION,
            RFX_MSG_URC_EMBMS_AVAILABLE_SESSION,
            RFX_MSG_URC_EMBMS_SAI_LIST_NOTIFICATION,
            RFX_MSG_URC_EMBMS_OOS_NOTIFICATION,
        };

    mIsEmbmsSupport = isEmbmsSupported();
    // register request & URC id list
    // NOTE. one id can only be registered by one controller
    // Register even if not mIsEmbmsSupport, to intercept msg
    registerToHandleRequest(request_id_list,
            sizeof(request_id_list) / sizeof(int));
    if (mIsEmbmsSupport) {
        registerToHandleUrc(urc_id_list, sizeof(urc_id_list) / sizeof(int));
    }

    mIsActiveSession = false;
    mIsFlightOn = false;
    mEmbmsEnabled = false;

    // When sim switch happens, context will be moved to main protocol, session cleared, but embms enabled on new sim.
    // So eMBMS MBMSAL layer will disable eMBMS and make it as reseted.
    // No matter DSS_NO_RESET or not.
    getNonSlotScopeStatusManager()->registerStatusChanged(RFX_STATUS_KEY_MAIN_CAPABILITY_SLOT,
        RfxStatusChangeCallback(this, &RtcEmbmsAtController::onMainCapabilityChanged));

    // On L+L platform, default sim switch will not necessary trigger sim switch.
    // Modem may not aware of this at all.
    if (RtcEmbmsUtils::isDualLteSupport()) {
        getNonSlotScopeStatusManager()->registerStatusChanged(RFX_STATUS_KEY_DEFAULT_DATA_SIM,
            RfxStatusChangeCallback(this, &RtcEmbmsAtController::onDefaultDataChanged));
    }



    RfxMainThread::waitLooper();
}

void RtcEmbmsAtController::onDeinit() {
    logI(RFX_LOG_TAG, "onDeinit");
    RtcEmbmsUtils::freeSessionList(mSessionInfoList);
    delete mSessionInfoList;
    mSessionInfoList = NULL;
    RfxController::onDeinit();
}

bool RtcEmbmsAtController::onHandleRequest(const sp<RfxMessage>& message) {
    logD(RFX_LOG_TAG, "Slot[%d] Handle request %s", this->getSlotId(),
        idToString(message->getId()));

    switch (message->getId()) {
        // common commands can send to md1(gsm)/md3(c2k) both.
        case RFX_MSG_REQUEST_EMBMS_AT_CMD:
            handleAtRequest(message);
            break;
        default:
            logE(RFX_LOG_TAG, "unknown request:%s, ignore!", idToString(message->getId()));
            break;
    }
    return true;
}

bool RtcEmbmsAtController::onHandleResponse(const sp<RfxMessage>& message) {
    logD(RFX_LOG_TAG, "Handle response %s.", idToString(message->getId()));

    switch (message->getId()) {
        case RFX_MSG_REQUEST_EMBMS_AT_CMD:
            handleAtResponse(message);
            break;
        case RFX_MSG_REQUEST_EMBMS_ENABLE:
            handleEnableResponse(message);
            break;
        case RFX_MSG_REQUEST_EMBMS_DISABLE:
            handleDisableResponse(message);
            break;
        case RFX_MSG_REQUEST_EMBMS_START_SESSION:
            handleStartSessionResponse(message);
            break;
        case RFX_MSG_REQUEST_EMBMS_STOP_SESSION:
            handleStopSessionResponse(message);
            break;
        case RFX_MSG_REQUEST_EMBMS_GET_TIME:
            handleGetNetworkTimeResponse(message);
            break;
        case RFX_MSG_REQUEST_RTC_EMBMS_GET_COVERAGE_STATE:
            handleGetCoverageResponse(message);
            break;
        case RFX_MSG_REQUEST_EMBMS_TRIGGER_CELL_INFO_NOTIFY:
            handleTriggerCellInfoNotifyResponse(message);
            break;
        default:
            logE(RFX_LOG_TAG, "unknown response:%s, ignore!", idToString(message->getId()));
            break;
    }
    return true;
}

bool RtcEmbmsAtController::onHandleUrc(const sp<RfxMessage>& message) {
    logI(RFX_LOG_TAG, "Handle URC %s Slot %d (DefaultDataSlot:%d)"
        , idToString(message->getId()),
        message->getSlotId(),
        RtcEmbmsUtils::getDefaultDataSlotId());

    if (getSlotId() != RtcEmbmsUtils::getDefaultDataSlotId()) {
        logW(RFX_LOG_TAG, "ignore URC due to not on DefaultDataSlot:%d",
            RtcEmbmsUtils::getDefaultDataSlotId());
        return true;
    }

    switch (message->getId()) {
        case RFX_MSG_URC_EMBMS_AT_INFO:
            // Same with solicited AT response, forward string result
            handleAtUrc(message);
            break;

        case RFX_MSG_URC_EMBMS_START_SESSION_RESPONSE:
            handleStartSessionUrc(message);
            break;

        case RFX_MSG_URC_EMBMS_CELL_INFO_NOTIFICATION:
            handleCellInfoUrc(message);
            break;

        case RFX_MSG_URC_EMBMS_COVERAGE_STATE:
            handleCoverageUrc(message);
            break;

        case RFX_MSG_URC_EMBMS_OOS_NOTIFICATION:
            if (mSdkVersion >= 10901) {
                handleOosUrc(message);
            }
            break;

        case RFX_MSG_URC_EMBMS_SAI_LIST_NOTIFICATION:
            if (mSdkVersion >= 10901) {
                handleSaiUrc(message);
            }
            break;

        case RFX_MSG_URC_EMBMS_ACTIVE_SESSION:
            handleActiveSessionStatus(message);
            handleSessionListUrc(message);
            break;

        case RFX_MSG_URC_EMBMS_AVAILABLE_SESSION:
            handleSessionListUrc(message);
            break;

        default:
            logE(RFX_LOG_TAG, "unknown urc:%s, ignore!", idToString(message->getId()));
            break;
    }
    return true;
}

bool RtcEmbmsAtController::onCheckIfRejectMessage(
        const sp<RfxMessage>& message, bool isModemPowerOff, int radioState) {

    int msgId = message->getId();
    if((radioState == (int)RADIO_STATE_UNAVAILABLE ||
        radioState == (int)RADIO_STATE_OFF) &&
            (msgId == RFX_MSG_REQUEST_EMBMS_AT_CMD)) {
        return false;
    }
    //others will print in RfxController::onCheckIfRejectMessage
    return RfxController::onCheckIfRejectMessage(message, isModemPowerOff, radioState);
}

int RtcEmbmsAtController::strStartsWith(const char *line, const char *prefix) {
    return RfxMisc::strStartsWith(line, prefix);
}

void RtcEmbmsAtController::handleAtRequest(const sp<RfxMessage>& request) {
    int need_forward = 0;

    if (!mIsEmbmsSupport) {
        logE(RFX_LOG_TAG, "mIsEmbmsSupport = %d", mIsEmbmsSupport);
        responseErrorString(request);
        return;
    }
    char *data = (char*)(request->getData()->getData());

    logI(RFX_LOG_TAG, "handleAtRequest:%s", (char*)data);

    // Check AT command format.
    if (data != NULL) {
        int totalLength = strlen(data);
        if (totalLength > (512*3-1)) {
            logE(RFX_LOG_TAG, "Error!! The length is too long (more than 512*3)");
            return;
        } else if (totalLength > 0 &&
                data[totalLength - 1] != '\r' &&
                data[totalLength - 1] != '\n') {
            logV(RFX_LOG_TAG, "Dbg terminate character:%d", data[totalLength - 1] );
        }
    }

    if (data == NULL) {
        logE(RFX_LOG_TAG, "Error!! requestEmbmsAt: get null!!");
        responseErrorString(request);
    } else if (strStartsWith(data, "AT%MBMSCMD=\"CURRENT_SAI_LIST\"")) {
        logV(RFX_LOG_TAG, "Pass to rild");
        need_forward = 1;
    } else if (strStartsWith(data, "AT%MBMSCMD=\"NETWORK INFORMATION\"")) {
        logV(RFX_LOG_TAG, "Pass to rild");
        need_forward = 1;
    } else if (strStartsWith(data, "AT%MBMSCMD=\"BSSI_SIGNAL_LEVEL\"")) {
        logV(RFX_LOG_TAG, "Pass to rild");
        need_forward = 1;
    } else if (strStartsWith(data, "AT+CMEE=")) {
        logV(RFX_LOG_TAG, "Pass to rild");
        need_forward = 1;
    } else if (strStartsWith(data, "AT+CFUN?")) {
        logV(RFX_LOG_TAG, "Pass to rild");
        need_forward = 1;
    } else if (strStartsWith(data, "AT%MBMSEV=")) {
        logV(RFX_LOG_TAG, "Pass to rild");
        need_forward = 1;
    } else if (strStartsWith(data, "AT%MBMSEV?")) {
        logD(RFX_LOG_TAG, "requestAtGetCoverage");
        requestAtGetCoverage(request);
    } else if (strStartsWith(data, "AT%MBMSCMD?")) {
        logV(RFX_LOG_TAG, "Pass to rild");
        need_forward = 1;
    } else if (strStartsWith(data, "AT%MBMSCMD=?")) {
        logV(RFX_LOG_TAG, "Pass to rild");
        need_forward = 1;
    } else if (strStartsWith(data, "AT%MBMSCMD=\"DEVICE INFO\"")) {
        logV(RFX_LOG_TAG, "Pass to rild");
        need_forward = 1;
    } else if (strStartsWith(data, "ATE")) {
        logV(RFX_LOG_TAG, "Pass to rild");
        need_forward = 1;
    // --- Special handle area ---
    } else if (strStartsWith(data, "AT%MBMSCMD=\"SIB16_GET_NETWORK_TIME\"")) {
        logD(RFX_LOG_TAG, "requestAtGetNetworkTime");
        requestAtGetNetworkTime(request);
    } else if (strStartsWith(data, "AT%MBMSCMD=\"MBMS_PREFERENCE\"")) {
        if (RtcEmbmsUtils::isAtCmdEnableSupport()) {
            logV(RFX_LOG_TAG, "Pass to rild");
            need_forward = 1;
        } else {
            logD(RFX_LOG_TAG, "requestAtEnable");
            requestAtEnable(request);
        }
    } else if (strStartsWith(data, "AT%MBMSCMD=\"ENABLE_EMBMS\"")) {
        logI(RFX_LOG_TAG, "requestAtEnable.mSdkVersion:%d", mSdkVersion);
        requestAtEnable(request);
    } else if (strStartsWith(data, "AT%MBMSCMD=\"AVAILABILITY_INFO\"")) {
        logD(RFX_LOG_TAG, "requestAtSetAvailbility");
        requestAtSetAvailbility(request);
    } else if (strStartsWith(data, "AT%MBMSCMD=\"ACTIVATE\"")) {
        logD(RFX_LOG_TAG, "requestAtActivate 1");
        requestAtActivate(request, EMBMS_ACTIVE_SESSION);
    } else if (strStartsWith(data, "AT%MBMSCMD=\"DEACTIVATE\"")) {
        logD(RFX_LOG_TAG, "requestAtActivate 0");
        requestAtActivate(request, EMBMS_DEACTIVE_SESSION);
    } else if (strStartsWith(data, "AT%MBMSCMD=\"DEACTIVATE_ALL\"")) {
        logD(RFX_LOG_TAG, "requestAtActivate 2");
        requestAtActivate(request, EMBMS_DEACTIVE_ALL_SESSION);
    } else if (mSdkVersion >= 10901) {
        if (strStartsWith(data, "AT+CEREG")) {
            //AT+CEREG=?
            //AT+CEREG=<n>
            //AT+CEREG?
            logV(RFX_LOG_TAG, "Pass to rild");
            need_forward = 1;
        } else if (strStartsWith(data, "AT") && strlen(data) == 2) {
            logV(RFX_LOG_TAG, "Pass to rild");
            need_forward = 1;
        }
    // --- Not support area ---
    } else {
        logE(RFX_LOG_TAG, "Not support");
        responseErrorString(request);
    }

    if (need_forward) {/* Need forward */
        handleRequestDefault(request);
    }
}

void RtcEmbmsAtController::responseErrorString(const sp<RfxMessage>& msg) {
    String8 responseStr(EMBMS_ERROR);
    responseToSocket(msg, responseStr.string());
}

void RtcEmbmsAtController::urcToSocket(const sp<RfxMessage>& msg,
    const char *responseStr, const char *responseStr_mask) {
    if (responseStr_mask) {
        logI(RFX_LOG_TAG, "urcToSocket:%s", responseStr_mask);
    } else {
        logI(RFX_LOG_TAG, "urcToSocket:%s", responseStr);
    }
    int strLen = 0;
    if (responseStr) {
        strLen = strlen(responseStr);
    }
    sp<RfxMessage> newMsg = RfxMessage::obtainUrc(msg->getSlotId(),
                                        RFX_MSG_URC_EMBMS_AT_INFO,
                                        RfxStringData((void*)responseStr, strLen),
                                        RADIO_TECH_GROUP_GSM);
    responseToRilj(newMsg);
}

void RtcEmbmsAtController::responseToSocket(const sp<RfxMessage>& msg,
    const char *responseStr) {
    logI(RFX_LOG_TAG, "responseToSocket:%s", responseStr);
    int strLen = 0;
    if (responseStr) {
        strLen = strlen(responseStr);
    }
    sp<RfxMessage> newMsg = RfxMessage::obtainResponse(msg->getSlotId(),
                                    RFX_MSG_REQUEST_EMBMS_AT_CMD,
                                    msg->getError(),
                                    RfxStringData((void*)responseStr, strLen),
                                    msg);
    responseToRilj(newMsg);
}

void RtcEmbmsAtController::handleAtResponse(const sp<RfxMessage>& response) {
    int32_t type = response->getType();
    int32_t token = response->getToken();
    int32_t error = response->getError();
    char *responseStr = NULL;

    if (error != RIL_E_SUCCESS) {
        responseErrorString(response);
        return;
    } else {
        responseStr = (char*)(response->getData()->getData());
        logV(RFX_LOG_TAG, "type: %d, token %d, error %d, str:%s"
                , type, token, error, responseStr);
        responseToSocket(response, responseStr);
    }
}

void RtcEmbmsAtController::handleRequestDefault(const sp<RfxMessage>& request) {
    sp<RfxMessage> message = RfxMessage::obtainRequest( request->getSlotId(),
                                                        request->getId(),
                                                        request,
                                                        true,
                                                        RADIO_TECH_GROUP_GSM);

    requestToMcl(message);
    logV(RFX_LOG_TAG, "Send request %s",idToString(request->getId()));
}


void RtcEmbmsAtController::requestAtEnable(const sp<RfxMessage>& request) {
    // new: AT%MBMSCMD="ENABLE_EMBMS",[0,1]
    // old: AT%MBMSCMD="MBMS_PREFERENCE",[0,1]
    char *data = (char*)(request->getData()->getData());
    int input_err = 0;
    int enable;

    int count;
    int type;
    int trans_id;
    char *line;

    mIsFlightOn = false;

    RfxAtLine* atline = new RfxAtLine(data, NULL);
    RfxAtLine* atline_free = atline;
    //  skip to next ','
    atline->atTokNextstr(&input_err);
    if (input_err < 0) {
        logE(RFX_LOG_TAG, "Input parameter error!!(%s:%d)", __FUNCTION__, __LINE__);
        responseErrorString(request);
        delete atline_free;
        return;
    }

    enable = atline->atTokNextint(&input_err);
    if (input_err < 0) {
        logE(RFX_LOG_TAG, "Input parameter error!!(%s:%d)", __FUNCTION__, __LINE__);
        responseErrorString(request);
        delete atline_free;
        return;
    }

    //  initial global data array for active info
    if (mSessionInfoList != NULL && !mSessionInfoList->isEmpty()) {
        RtcEmbmsUtils::freeSessionList(mSessionInfoList);
    }
    delete atline_free;
    atline_free = NULL;
    int msg_id = RFX_MSG_REQUEST_EMBMS_ENABLE;
    if (enable) {
        msg_id = RFX_MSG_REQUEST_EMBMS_ENABLE;
    } else {
        msg_id = RFX_MSG_REQUEST_EMBMS_DISABLE;
    }

    trans_id = request->getToken();
    int intdata[2];
    intdata[0] = trans_id;
    intdata[1] = EMBMS_COMMAND_ATCI;

    sp<RfxMessage> newMsg = RfxMessage::obtainRequest(msg_id,
        RfxIntsData(intdata, 2), request, false);

    requestToMcl(newMsg);
}

void RtcEmbmsAtController::handleEnableResponse(const sp<RfxMessage>& response) {
    RIL_EMBMS_LocalEnableResp* entry = NULL;
    int id = response->getId();
    char *responseStr = NULL;

    mIsActiveSession = false;

    if (response->getError() == RIL_E_SUCCESS) {
        entry = (RIL_EMBMS_LocalEnableResp *) response->getData()->getData();
        logI(RFX_LOG_TAG, "handleEnableResponse request %d,tid %d,rsp %d,idx_valid %d,idx %d,iface_name %s",
            id, entry->trans_id, entry->response, entry->interface_index_valid,
            entry->interface_index, entry->interface_name);
    } else {
        logI(RFX_LOG_TAG, "handleEnableResponse error!");
    }


    if (response->getError() == RIL_E_SUCCESS && entry->response == EMBMS_GENERAL_SUCCESS) {
        // for RJIL old middleware version
        if (RtcEmbmsUtils::isRjilSupport()) {
            asprintf(&responseStr, EMBMS_OK);
        } else {
            //%MBMSCMD:rmnet1,1
            //OK
            asprintf(&responseStr, "%%MBMSCMD:%s,%d\nOK\n", entry->interface_name,
                entry->interface_index);
        }
        mEmbmsEnabled = true;
    } else {
        asprintf(&responseStr, EMBMS_ERROR);
    }

    responseToSocket(response, responseStr);
    free(responseStr);
}

void RtcEmbmsAtController::handleDisableResponse(const sp<RfxMessage>& response) {
    RIL_EMBMS_DisableResp* entry = NULL;
    int id = response->getId();
    char *responseStr = NULL;

    if (response->getError() == RIL_E_SUCCESS) {
        entry = (RIL_EMBMS_DisableResp *) response->getData()->getData();
        logI(RFX_LOG_TAG, "handleDisableResponse request %d,tid %d,rsp %d",
        id, entry->trans_id, entry->response);
    } else {
        logI(RFX_LOG_TAG, "handleDisableResponse error!");
    }

    if (response->getError() == RIL_E_SUCCESS && entry->response == EMBMS_GENERAL_SUCCESS) {
        mEmbmsEnabled = false;
        //  clear active info
        if (mSessionInfoList != NULL && !mSessionInfoList->isEmpty()) {
            RtcEmbmsUtils::freeSessionList(mSessionInfoList);
        }
        asprintf(&responseStr, EMBMS_OK);
    } else {
        asprintf(&responseStr, EMBMS_ERROR);
    }


    // Do not response silent disable eMBMS to middleware
    // The original default sim is not current default sim, so do not response
    if (getSlotId() != RtcEmbmsUtils::getDefaultDataSlotId()) {
        logI(RFX_LOG_TAG, "ignore DisableResponse due to not on DefaultDataSlot:%d",
            RtcEmbmsUtils::getDefaultDataSlotId());
    } else {
        responseToSocket(response, responseStr);
    }
    free(responseStr);
}

void RtcEmbmsAtController::handleStartSessionResponse(const sp<RfxMessage>& response) {
    RIL_EMBMS_LocalStartSessionResp* entry = NULL;
    int index;
    int id = response->getId();
    // Only handle fail case here; asynchronous success response later.
    if (response->getError() == RIL_E_SUCCESS) {
        entry = (RIL_EMBMS_LocalStartSessionResp *) response->getData()->getData();

        logI(RFX_LOG_TAG, "handleStartSessionResponse response %d,tid %d,rsp %d,tmgi %s",
        id, entry->trans_id, entry->response, entry->tmgi);

        if (entry->response != EMBMS_GENERAL_SUCCESS) {
            int index;
            RtcEmbmsSessionInfo* sessionInfo =
                RtcEmbmsUtils::findSessionByTransId(mSessionInfoList, entry->trans_id, &index);
            if (sessionInfo != NULL) {
                logD(RFX_LOG_TAG, "Send resposne %s",
                        idToString(sessionInfo->mOriginalRequest->getId()));
                responseErrorString(response);
                delete mSessionInfoList->itemAt(index);
                mSessionInfoList->removeAt(index);
            } else {
                logD(RFX_LOG_TAG, "Fail to find session in send resposne");
                // Already been replied through urc
            }
            return;
        }
    } else {
        responseErrorString(response);
    }
}

void RtcEmbmsAtController::handleStartSessionUrc(const sp<RfxMessage>& response) {
    RIL_EMBMS_LocalStartSessionResp* entry = NULL;
    int index;
    entry = (RIL_EMBMS_LocalStartSessionResp *) response->getData()->getData();

    logI(RFX_LOG_TAG, "handleStartSessionUrc, reason:%d, valid%d",
        entry->response, entry->tmgi_info_valid);
    if (entry->tmgi_info_valid == 1) {
        logI(RFX_LOG_TAG, "urc tmgi:%s", entry->tmgi);
        RtcEmbmsSessionInfo* sessionInfo = RtcEmbmsUtils::findSessionByTmgi(
                                        mSessionInfoList, strlen(entry->tmgi), entry->tmgi, &index);
        if (sessionInfo != NULL && index != -1) {
            char *responseStr = NULL;
            // To avoid when MD response EMSSS too slow and middleware re-send AVAILABILITY_INFO,
            // and then EMSESS URC come back before ACTIVATE. Just ignore previous URC
            if (sessionInfo->mOriginalRequest == NULL) {
                return;
            }

            if (entry->response == EMBMS_SESSION_SUCCESS) {
                asprintf(&responseStr, EMBMS_OK);
            } else {
                asprintf(&responseStr, EMBMS_ERROR);
            }

            responseToSocket(sessionInfo->mOriginalRequest, responseStr);
            free(responseStr);
            delete mSessionInfoList->itemAt(index);
            mSessionInfoList->removeAt(index);
            return;
        }
    }
}

void RtcEmbmsAtController::handleStopSessionResponse(const sp<RfxMessage>& response) {
    RIL_EMBMS_LocalStartSessionResp* entry = NULL;
    int id = response->getId();
    char *responseStr = NULL;

    if (response->getError() == RIL_E_SUCCESS) {
        entry = (RIL_EMBMS_LocalStartSessionResp *) response->getData()->getData();
        logI(RFX_LOG_TAG, "handleStopSessionResponse request %d,tid %d,rsp %d,tmgi %s",
        id, entry->trans_id, entry->response, entry->tmgi);
    } else {
        logI(RFX_LOG_TAG, "handleStopSessionResponse error!");
    }

    if (response->getError() == RIL_E_SUCCESS && entry->response == EMBMS_GENERAL_SUCCESS) {
        asprintf(&responseStr, EMBMS_OK);
    } else {
        asprintf(&responseStr, EMBMS_ERROR);
    }

    responseToSocket(response, responseStr);
    free(responseStr);
}

void RtcEmbmsAtController::requestAtSetAvailbility(const sp<RfxMessage>& request) {
    //  AT%MBMSCMD="AVAILABILITY_INFO",<TMGI>,<SAI_COUNT>,
    //  [<sai1>,<FREQUENCY_COUNT>,[<Freq1>,[<Freq2>,..]],
    //  [<sai2>,<FREQUENCY_COUNT>,[<Freq1>,[<Freq2>,..]]..]]
    char *data = (char*)(request->getData()->getData());
    char *tmgi;
    int input_err = 0;
    RtcEmbmsSessionInfo inputInfo;
    int sai_count = 0;
    int sai = 0;
    int freq_count = 0;
    int freq = 0;
    int i, j;

    RfxAtLine* atline = new RfxAtLine(data, NULL);
    RfxAtLine* atline_free = atline;
    //  skip to next ','
    atline->atTokNextstr(&input_err);
    if (input_err < 0) {
        logE(RFX_LOG_TAG, "Input parameter error!!(%s:%d)", __FUNCTION__, __LINE__);
        responseErrorString(request);
        delete atline_free;
        return;
    }

    // tmgi
    tmgi = atline->atTokNextstr(&input_err);
    if (input_err < 0) {
        logE(RFX_LOG_TAG, "Input parameter error!!(%s:%d)", __FUNCTION__, __LINE__);
        responseErrorString(request);
        delete atline_free;
        return;
    }

    if (VDBG) logV(RFX_LOG_TAG, "tmgi = %s", tmgi);

    inputInfo.mFreqCount = 0;
    inputInfo.mOriginalRequest = NULL;
    unsigned int tmgi_len = (strlen(tmgi) > EMBMS_MAX_LEN_TMGI) ? EMBMS_MAX_LEN_TMGI : strlen(tmgi);
    strncpy(inputInfo.mTmgi, tmgi, EMBMS_MAX_LEN_TMGI);
    inputInfo.mTmgi[tmgi_len] = '\0';
    inputInfo.mTmgiLen = tmgi_len;
    // sai_count
    sai_count = atline->atTokNextint(&input_err);
    if (input_err < 0) {
        logE(RFX_LOG_TAG, "Input parameter error!!(%s:%d)", __FUNCTION__, __LINE__);
        responseErrorString(request);
        delete atline_free;
        return;
    }

    // TBD: trick, change sai count from 0 to 1.
    if (sai_count == 0 && atline->atTokHasmore()) {
        sai_count = 1;
    }

    inputInfo.mSaiCount = sai_count;

    if (VDBG) logV(RFX_LOG_TAG, "sai_count = %d", sai_count);

    if (inputInfo.mSaiCount > EMBMS_MAX_NUM_SAI) {
        logE(RFX_LOG_TAG, "Cut sai_count(%d) to EMBMS_MAX_NUM_SAI(%d)",
                inputInfo.mSaiCount, EMBMS_MAX_NUM_SAI);
        inputInfo.mSaiCount = EMBMS_MAX_NUM_SAI;
    }

    // sai & freq
    for (i = 0; i < inputInfo.mSaiCount; i++) {
        sai = atline->atTokNextint(&input_err);

        if (input_err < 0) {
            logE(RFX_LOG_TAG, "Input parameter error!!(%s:%d)", __FUNCTION__, __LINE__);
            responseErrorString(request);
            delete atline_free;
            return;
        }

        if (i < EMBMS_MAX_NUM_SAI) {
            inputInfo.mSais[i] = sai;
            if (VDBG) logV(RFX_LOG_TAG, "sai = %d", sai);
        }

        freq_count = atline->atTokNextint(&input_err);

        if (input_err < 0) {
            logE(RFX_LOG_TAG, "Input parameter error!!(%s:%d)", __FUNCTION__, __LINE__);
            responseErrorString(request);
            delete atline_free;
            return;
        }

        for (j = 0; j < freq_count; j++) {
            freq = atline->atTokNextint(&input_err);

            if (input_err < 0) {
                logE(RFX_LOG_TAG, "Input parameter error!!(%s:%d)", __FUNCTION__, __LINE__);
                responseErrorString(request);
                delete atline_free;
                return;
            }

            if ((inputInfo.mFreqCount + j) < EMBMS_MAX_NUM_FREQ) {
                inputInfo.mFreqs[inputInfo.mFreqCount + j] = freq;
            }
        }

        inputInfo.mFreqCount = inputInfo.mFreqCount + freq_count;

        if (inputInfo.mFreqCount > EMBMS_MAX_NUM_FREQ) inputInfo.mFreqCount = EMBMS_MAX_NUM_FREQ;
    }

    //  Update or add
    int index;
    RtcEmbmsSessionInfo* sessionInfo = RtcEmbmsUtils::findSessionByTmgi(
                                    mSessionInfoList, tmgi_len, tmgi, &index);

    if (index == -1) {
        RtcEmbmsSessionInfo* info = new RtcEmbmsSessionInfo();
        info->copy(inputInfo);
        logD(RFX_LOG_TAG, "new avail info, tmgi:%s", info->mTmgi);
        if (mSessionInfoList != NULL) {
            mSessionInfoList->add(info);
            for (RtcEmbmsSessionInfo* item : *mSessionInfoList) {
                if (item != NULL) {
                    item->dump();
                }
            }
        }
    } else {
        sessionInfo->copy(inputInfo);
        logD(RFX_LOG_TAG, "update avail info, tmgi:%s", sessionInfo->mTmgi);
        if (mSessionInfoList != NULL) {
            for (RtcEmbmsSessionInfo* item : *mSessionInfoList) {
                if (item != NULL) {
                    item->dump();
                }
            }
        }
    }
    // response

    delete atline_free;
    atline_free = NULL;
    char *responseStr = (char *) EMBMS_OK;
    responseToSocket(request, responseStr);
}

void RtcEmbmsAtController::requestAtActivate(const sp<RfxMessage>& request
    , embms_at_activesession_enum type) {
    char *data = (char*)(request->getData()->getData());
    char *tmgi;
    int sessionId = INVALID_EMBMS_SESSION_ID;
    int input_err = 0;
    int trans_id;
    int mcast_ip_address;

    int msg_id = RFX_MSG_REQUEST_EMBMS_START_SESSION;
    trans_id = request->getToken();

    // Select type
    switch (type) {
        case EMBMS_DEACTIVE_SESSION:
        case EMBMS_DEACTIVE_ALL_SESSION:
            msg_id = RFX_MSG_REQUEST_EMBMS_STOP_SESSION;
            break;
        case EMBMS_ACTIVE_SESSION:
            msg_id = RFX_MSG_REQUEST_EMBMS_START_SESSION;
            break;
        default:
            break;
    }

    RfxAtLine* atline = new RfxAtLine(data, NULL);
    RfxAtLine* atline_free = atline;

    // Parse input parameter according to type
    if (type == EMBMS_DEACTIVE_ALL_SESSION) {
        //  AT%MBMSCMD="DEACTIVATE_ALL"
        tmgi = NULL;
        sessionId = INVALID_EMBMS_SESSION_ID;
    } else if (type == EMBMS_DEACTIVE_SESSION || type == EMBMS_ACTIVE_SESSION) {
        //  AT%MBMSCMD="ACTIVATE/DEACTIVATE",<TMGI-string>,<SessionID-int>,<Multicast_IP_Address-int>
        //  skip to next ','
        atline->atTokNextstr(&input_err);
        if (input_err < 0) {
            logE(RFX_LOG_TAG, "Input parameter error!!(%s:%d)", __FUNCTION__, __LINE__);
            responseErrorString(request);
            delete atline_free;
            return;
        }

        // tmgi
        tmgi = atline->atTokNextstr(&input_err);
        if (input_err < 0) {
            logE(RFX_LOG_TAG, "Input parameter error!!(%s:%d)", __FUNCTION__, __LINE__);
            responseErrorString(request);
            delete atline_free;
            return;
        }

        // sessionId
        sessionId = atline->atTokNextint(&input_err);
        if (input_err < 0) sessionId = INVALID_EMBMS_SESSION_ID;

        // mcast_ip_address, not used
        mcast_ip_address = atline->atTokNextint(&input_err);
        if (input_err < 0) {
            logE(RFX_LOG_TAG, "Input parameter error!!(%s:%d)", __FUNCTION__, __LINE__);
            responseErrorString(request);
            delete atline_free;
            return;
        }
    } else {
        logE(RFX_LOG_TAG, "requestAtActivate error!!Type not support!!");
        delete atline_free;
        return;
    }

    // Query session or clear session if nessary
    // Compose command parameters
    if (type == EMBMS_DEACTIVE_ALL_SESSION || type == EMBMS_DEACTIVE_SESSION) {
        if (type == EMBMS_DEACTIVE_ALL_SESSION) {
            //  clear active info
            if (mSessionInfoList != NULL && !mSessionInfoList->isEmpty()) {
                RtcEmbmsUtils::freeSessionList(mSessionInfoList);
            }
        } else {  // EMBMS_DEACTIVE_SESSION
            int index;
            RtcEmbmsSessionInfo* sessionInfo = RtcEmbmsUtils::findSessionByTmgi(
                                            mSessionInfoList, strlen(tmgi), tmgi, &index);
            if (index != -1) {
                delete mSessionInfoList->itemAt(index);
                mSessionInfoList->removeAt(index);
            }
        }

        RIL_EMBMS_LocalStopSessionReq entry;
        entry.stop_type = type;
        entry.trans_id = trans_id;
        int tmgi_len;
        if (tmgi != NULL) {
            tmgi_len = (strlen(tmgi) > EMBMS_MAX_LEN_TMGI) ? EMBMS_MAX_LEN_TMGI : strlen(tmgi);
            strncpy(entry.tmgi, tmgi, tmgi_len);
        } else {
            tmgi_len = 0;
            strncpy(entry.tmgi, "", tmgi_len);
        }
        entry.tmgi[tmgi_len] = '\0';
        logV(RFX_LOG_TAG, "entry.tmgi:%s", entry.tmgi);
        entry.session_id = sessionId;

        sp<RfxMessage> newMsg = RfxMessage::obtainRequest(msg_id,
            RfxEmbmsLocalStopSessionReqData(&entry, sizeof(entry)), request, false);

        requestToMcl(newMsg);
    } else if (type == EMBMS_ACTIVE_SESSION) {
        int index;
        RtcEmbmsSessionInfo* sessionInfo = RtcEmbmsUtils::findSessionByTmgi(
                                        mSessionInfoList, strlen(tmgi), tmgi, &index);
        if (index == -1) {
            logE(RFX_LOG_TAG, "Can't find availbility info for %s!!", tmgi);
            responseErrorString(request);
            delete atline_free;
            return;
        }
        sessionInfo->mTransId = trans_id;
        sessionInfo->mOriginalRequest = request;

        RIL_EMBMS_LocalStartSessionReq entry;
        entry.trans_id = trans_id;
        int tmgi_len = (strlen(tmgi) > EMBMS_MAX_LEN_TMGI) ? EMBMS_MAX_LEN_TMGI : strlen(tmgi);
        strncpy(entry.tmgi, tmgi, tmgi_len);
        entry.tmgi[tmgi_len] = '\0';
        entry.session_id = sessionId;
        entry.earfcnlist_count = sessionInfo->mFreqCount;
        for (uint32_t i = 0; i < entry.earfcnlist_count; i++) {
            entry.earfcnlist[i] = sessionInfo->mFreqs[i];
        }
        entry.saiList_count = sessionInfo->mSaiCount;
        for (uint32_t i = 0; i < entry.saiList_count; i++) {
            entry.saiList[i] = sessionInfo->mSais[i];
        }

        sp<RfxMessage> newMsg = RfxMessage::obtainRequest(msg_id,
            RfxEmbmsLocalStartSessionReqData(&entry, sizeof(entry)), request, false);

        requestToMcl(newMsg);
    }
    delete atline_free;
    atline_free = NULL;
}

void RtcEmbmsAtController::requestAtGetCoverage(const sp<RfxMessage>& request) {
    // AT%MBMSEV?
    // No parameter parse.
    int trans_id = request->getToken();
    int intdata[1];
    intdata[0] = trans_id;

    sp<RfxMessage> newMsg = RfxMessage::obtainRequest(RFX_MSG_REQUEST_RTC_EMBMS_GET_COVERAGE_STATE,
        RfxIntsData(intdata, 1), request, false);

    requestToMcl(newMsg);
}

void RtcEmbmsAtController::handleGetCoverageResponse(const sp<RfxMessage>& response) {
    RIL_EMBMS_GetCoverageResp* entry = NULL;
    int id = response->getId();
    char *responseStr = NULL;
    uint32_t result_status;

    if (response->getError() == RIL_E_SUCCESS) {
        entry = (RIL_EMBMS_GetCoverageResp *) response->getData()->getData();
        logI(RFX_LOG_TAG, "handleGetCoverageResponse request %d,tid %d,rsp %d,valid %d,cov %d",
            id, entry->trans_id, entry->response, entry->coverage_state_valid, entry->coverage_state);

        if (entry->coverage_state_valid) {
            result_status = getCoverageState(entry->coverage_state);
            entry->coverage_state = result_status;
        } else {
            result_status = getCoverageState(EMBMS_LOCAL_SRV_OOS);
            entry->coverage_state = result_status;
        }
    } else {
        logI(RFX_LOG_TAG, "handleGetCoverageResponse error!");
    }


    if (response->getError() == RIL_E_SUCCESS && entry->response == EMBMS_GENERAL_SUCCESS) {
        // for RJIL old middleware version
        if (RtcEmbmsUtils::isRjilSupport()) {
            asprintf(&responseStr, "%%MBMSEV:%d\nOK\n", entry->coverage_state);
        } else {
            asprintf(&responseStr, "%%MBMSEV:%d,0\nOK\n", entry->coverage_state);
        }
    } else {
        asprintf(&responseStr, EMBMS_ERROR);
    }

    responseToSocket(response, responseStr);
    free(responseStr);
}

void RtcEmbmsAtController::requestAtGetNetworkTime(const sp<RfxMessage>& request) {
    // AT%MBMSCMD="SIB16_GET_NETWORK_TIME"
    // No parameter parse.
    int trans_id = request->getToken();
    int intdata[1];
    intdata[0] = trans_id;

    sp<RfxMessage> newMsg = RfxMessage::obtainRequest(RFX_MSG_REQUEST_EMBMS_GET_TIME,
        RfxIntsData(intdata, 1), request, false);

    requestToMcl(newMsg);
}

void RtcEmbmsAtController::handleGetNetworkTimeResponse(const sp<RfxMessage>& response) {
    RIL_EMBMS_GetTimeResp* entry = NULL;
    int id = response->getId();
    char *responseStr = NULL;

    if (response->getError() == RIL_E_SUCCESS) {
        entry = (RIL_EMBMS_GetTimeResp *) response->getData()->getData();

        if (entry->response == (int32_t) EMBMS_GET_TIME_SIB16) {
            entry->response = (int32_t) EMBMS_GENERAL_SUCCESS;
            // The timing information is consisting of time Info UTC part of SIB-16.
            // The field indicates the integer count of 10 ms units since 00:00:00
            // on 1 January, 1900 as per 3GPP specification, 36.331
            entry->milli_sec = entry->milli_sec / 10ll;
        }
        logI(RFX_LOG_TAG, "handleGetNetworkTimeResponse request %d,tid %d,rsp %u,sec %lu",
            id, entry->trans_id, entry->response,
            entry->milli_sec);
        logI(RFX_LOG_TAG, "dlsaving_valid %d, dlsaving %d,ls_valid %d,ls %d,lto_valid %d,lto %d",
            entry->day_light_saving_valid, entry->day_light_saving,
            entry->leap_seconds_valid, entry->leap_seconds,
            entry->local_time_offset_valid, entry->local_time_offset);
    } else {
        logI(RFX_LOG_TAG, "handleGetNetworkTimeResponse error!");
    }

    if (response->getError() == RIL_E_SUCCESS
        && entry->response == (int32_t) EMBMS_GENERAL_SUCCESS) {
        asprintf(&responseStr, "%%MBMSCMD:%llu\nOK\n", (long long unsigned int) entry->milli_sec);
    } else {
        asprintf(&responseStr, EMBMS_ERROR);
    }

    responseToSocket(response, responseStr);
    free(responseStr);
}

void RtcEmbmsAtController::handleTriggerCellInfoNotifyResponse(const sp<RfxMessage>& response) {
    int id = response->getId();
    // Just ignore, nothing to do
    logI(RFX_LOG_TAG, "handleTriggerCellInfoNotifyResponse, id:%d", id);
}

void RtcEmbmsAtController::handleAtUrc(const sp<RfxMessage>& response) {
    char *responseStr = NULL;

    responseStr = (char*)(response->getData()->getData());
    // logI(RFX_LOG_TAG, "handleAtUrc %s", responseStr);

    urcToSocket(response, responseStr, NULL);
}

void RtcEmbmsAtController::handleCellInfoUrc(const sp<RfxMessage>& response) {
    int32_t cell_id;
    int32_t status;
    int32_t plmn;
    char* ecgi = NULL;  // 3+3+7 digits
    char* ecgi_mask = NULL;
    char* cell_id_mask = NULL;

    int *data = (int *) response->getData()->getData();
    int data_length = response->getData()->getDataLength();
    cell_id = data[0];
    status = data[1];
    plmn = data[2];
    cell_id_mask = RtcEmbmsUtils::addLogMask(cell_id);

    logI(RFX_LOG_TAG, "handleCellInfoUrc cell_id %s, status %d, plmn %d",
        cell_id_mask, status,plmn);

    // Register with LTE network. Change flight mode to false.
    if (status == 1) {
        mIsFlightOn = false;
    }

    char *responseStr = NULL;
    char *responseStr_mask = NULL;
    asprintf(&responseStr, "+CEREG:%d\n", status);
    urcToSocket(response, responseStr, NULL);
    free(responseStr);

    if (!RtcEmbmsUtils::isRjilSupport()) {
        // %MBMSEV:4,<ECGI>
        asprintf(&ecgi, "%05d%07X", plmn, cell_id);
        asprintf(&ecgi_mask, "%05d%s", plmn, cell_id_mask);
        asprintf(&responseStr, "%%MBMSEV:4,%s\n", ecgi);
        asprintf(&responseStr_mask, "%%MBMSEV:4,%s\n", ecgi_mask);
        urcToSocket(response, responseStr, responseStr_mask);
        free(responseStr);
        free(responseStr_mask);
    }

    if (cell_id_mask) {
        cell_id_mask = NULL;
        free(cell_id_mask);
    }

}

void RtcEmbmsAtController::handleCoverageUrc(const sp<RfxMessage>& response) {
    int32_t srv_status;
    int     result_status;

    int *data = (int *) response->getData()->getData();
    int data_length = response->getData()->getDataLength();
    srv_status = data[0];
    result_status = getCoverageState(srv_status);

    char *responseStr = NULL;
    if (RtcEmbmsUtils::isRjilSupport()) {
        asprintf(&responseStr, "%%MBMSEV:%d\n", result_status);
    } else {
        asprintf(&responseStr, "%%MBMSEV:%d,0\n", result_status);
    }
    urcToSocket(response, responseStr, NULL);
    free(responseStr);
}

void RtcEmbmsAtController::handleSessionListUrc(const sp<RfxMessage>& response) {
    uint32_t tmgi_info_count;
    char*    tmp_tmgi;
    int32_t trans_id = -1;
    int32_t tmp;
    // %MBMSEV: 96 -> Active TMGIs (all currently activated TMGIs)
    // %MBMSEV: 97 -> Available TMGIs (all currently activated TMGIs + Available TMGIs that can be activated)
    // %MBMSEV:[96,97],count, TMGI1, TMGI2..
    String8 responseStr;
    RIL_EMBMS_LocalSessionNotify* input_data =
        (RIL_EMBMS_LocalSessionNotify *) response->getData()->getData();

    if (mIsFlightOn) {
        logI(RFX_LOG_TAG, "skip due to flight mode");
        return;
    }

    if (response->getId()== RFX_MSG_URC_EMBMS_ACTIVE_SESSION) {
        responseStr = String8("%MBMSEV:96");
    } else {  // RFX_MSG_URC_EMBMS_AVAILABLE_SESSION
        responseStr = String8("%MBMSEV:97");
    }

    // dispatch RIL_EMBMS_LocalSessionNotify
    tmgi_info_count = input_data->tmgi_info_count;
    responseStr.append(String8::format(",%d", tmgi_info_count));

    logI(RFX_LOG_TAG, "handleSessionListUrc, tmgi_info_count%d", tmgi_info_count);
    for (uint32_t i = 0; i < tmgi_info_count; i++) {
        logD(RFX_LOG_TAG, "handleSessionListUrc, tmgi[%d] = [%s]", i, input_data->tmgix[i]);
        responseStr.append(String8::format(",%s", input_data->tmgix[i]));
    }
    //
    if (RtcEmbmsUtils::isRjilSupport()) { //version 1.1.1x~1.1.20c
        responseStr = String8("%MBMSEV:0");
        logD(RFX_LOG_TAG, "%s", responseStr.string());
        urcToSocket(response, responseStr.string(), NULL);
    } else if (mSdkVersion < 10901){
        responseStr = String8("%MBMSEV:0,0");
        logD(RFX_LOG_TAG, "%s", responseStr.string());
        urcToSocket(response, responseStr.string(), NULL);
    } else {//mSdkVersion >= 10901
        responseStr.append(String8("\n"));
        logD(RFX_LOG_TAG, "%s", responseStr.string());
        urcToSocket(response, responseStr.string(), NULL);

        responseStr = String8("%MBMSEV:0,0");
        logD(RFX_LOG_TAG, "%s", responseStr.string());
        urcToSocket(response, responseStr.string(), NULL);
    }
}

void RtcEmbmsAtController::handleActiveSessionStatus(const sp<RfxMessage>& response) {
    RIL_EMBMS_LocalSessionNotify* entry = NULL;
    bool    isActive = false;

    if (response->getError() == RIL_E_SUCCESS) {
        entry = (RIL_EMBMS_LocalSessionNotify *) response->getData()->getData();

        logI(RFX_LOG_TAG, "handleActiveSessionStatus tmgi_info_count %d",
        entry->tmgi_info_count);
    } else {
        logI(RFX_LOG_TAG, "handleActiveSessionStatus error!");
        return;
    }

    isActive = (entry->tmgi_info_count > 0) ? true : false;

    if (mIsActiveSession != isActive) {
        mIsActiveSession = isActive;
        int intdata[1];
        intdata[0] = (mIsActiveSession) ? 1 : 0;
        // responseInts
        sp<RfxMessage> newMsg = RfxMessage::obtainUrc(response->getSlotId(),
                                        RFX_MSG_URC_RTC_EMBMS_SESSION_STATUS,
                                        RfxIntsData(intdata, 1), RADIO_TECH_GROUP_GSM);
        responseToRilj(newMsg);
    }
}

void RtcEmbmsAtController::handleOosUrc(const sp<RfxMessage>& response) {
    RIL_EMBMS_LocalOosNotify* data = (RIL_EMBMS_LocalOosNotify *) response->getData()->getData();
    // %MBMSEV:95,count, TMGI1, TMGI2..
    String8 responseStr("%MBMSEV:95");

    // dispatch RIL_EMBMS_LocalOosNotify
    uint32_t reason = data->reason;
    uint32_t tmgi_info_count = data->tmgi_info_count;
    responseStr.append(String8::format(",%d", tmgi_info_count));

    logI(RFX_LOG_TAG, "handleOosUrc, reason:%d, tmgi_info_count%d", reason, tmgi_info_count);

    for (uint32_t i = 0; i < tmgi_info_count; i++) {
        logI(RFX_LOG_TAG, "handleOosUrc, tmgi[%d] = [%s]", i, data->tmgix[i]);
        responseStr.append(String8::format(",%s", data->tmgix[i]));
    }
    responseStr.append(String8("\n"));
    if (tmgi_info_count > 0) {
        urcToSocket(response ,responseStr.string(), NULL);
    }
}

void RtcEmbmsAtController::handleSaiUrc(const sp<RfxMessage>& response) {
    RIL_EMBMS_LocalSaiNotify* input_data =
        (RIL_EMBMS_LocalSaiNotify *) response->getData()->getData();
    RIL_EMBMS_SaiNotify output_data;
    memset(&output_data, 0, sizeof(output_data));
    // %MBMSEV:98,<SAI_COUNT>,[<csai1>, <csai2>, <csai3>,...], <INTER_FREQUENCY_COUNT>,
    // [<Frequency_1>,<SAI_COUNT_1>, [<nsai11>, <nsai12>, <nsai13>,...],
    // [<Frequency_n>,<SAI_COUNT_n>, [<nsain1>, <nsain2>, <nsain3>,...]
    String8 responseStr("%MBMSEV:98");

    // Fill in response data

    // trans_id
    output_data.trans_id = -1;
    // cf_total
    output_data.cf_total = input_data->cf_total;
    // curFreqData
    for (uint32_t i = 0; i < input_data->cf_total; i++) {
        output_data.curFreqData[i] = input_data->curFreqData[i];
    }
    // csai_count_per_group
    for (uint32_t i = 0; i < input_data->cf_total; i++) {
        output_data.csai_count_per_group[i] = 0;
    }
    // csai_total
    output_data.csai_total = input_data->csai_total;
    // curSaiData
    for (uint32_t i = 0; i < input_data->csai_total; i++) {
        output_data.curSaiData[i] = input_data->curSaiData[i];
    }
    // nf_total
    output_data.nf_total = input_data->nf_total;
    // neiFreqData
    for (uint32_t i = 0; i < input_data->nf_total; i++) {
        output_data.neiFreqData[i] = input_data->neiFreqData[i];
    }
    // nsai_count_per_group
    for (uint32_t i = 0; i < input_data->nf_total; i++) {
        output_data.nsai_count_per_group[i] = input_data->nsai_count_per_group[i];
    }
    // nsai_total
    output_data.nsai_total = input_data->nsai_total;
    // neiSaiData
    for (uint32_t i = 0; i < input_data->nsai_total; i++) {
        output_data.neiSaiData[i] = input_data->neiSaiData[i];
    }

    // SAI_COUNT
    responseStr.append(String8::format(",%d", output_data.csai_total));

    // csai1
    for (uint32_t i = 0; i < output_data.csai_total; i++) {
        responseStr.append(String8::format(",%d", output_data.curSaiData[i]));
    }
    // INTER_FREQUENCY_COUNT
    responseStr.append(String8::format(",%d", output_data.nf_total));

    int tail_index = 0;
    for (uint32_t i = 0; i < output_data.nf_total; i++) {
        // <Frequency_1>, <SAI_COUNT_1>
        responseStr.append(String8::format(",%d,%d", output_data.neiFreqData[i],
                                output_data.nsai_count_per_group[i]));

        // <nsai11>
        for (uint32_t j = 0; j < output_data.nsai_count_per_group[i]; j++) {
            responseStr.append(String8::format(",%d", output_data.neiSaiData[tail_index+j]));
        }
        tail_index += output_data.nsai_count_per_group[i];
    }
    responseStr.append(String8("\n"));
    urcToSocket(response ,responseStr.string(), NULL);
}

int RtcEmbmsAtController::getCoverageState(int srv_status) {
    /*
    %MBMSEV:<event1>,<event2>
        List of <event1>: int
        0 : Service change event
        1 : No service (No LTE coverage)
        2 : Only unicast service available
        3 : LTE unicast and eMBMS service available
        4 : Cell change (Indicate the ECGI of the new cell)
        99 : Flight mode

        List of <event2>: string
        0  : To be sent when <event1> is other than 4
        ECGI : Sent when event1 is 4 to indicate the new ECGI
    */
    int result_status;

    switch (srv_status) {
        case EMBMS_LOCAL_SRV_OOS:
            result_status = 1;
            break;
        case EMBMS_LOCAL_SRV_UNITCAST:
            result_status = 2;
            break;
        case EMBMS_LOCAL_SRV_EMBMS:
            result_status = 3;
            break;
        case EMBMS_LOCAL_SRV_E911:
            result_status = 1;
            break;
        case EMBMS_LOCAL_SRV_HVOLTE:
            result_status = 1;
            break;
        case EMBMS_LOCAL_SRV_FLIGHT_MODE:
            result_status = 99;
            mIsFlightOn = true;
            break;
        default:
            result_status = 1;
            break;
    }
    return result_status;
}

bool RtcEmbmsAtController::isEmbmsSupported() {
    char prop[RFX_PROPERTY_VALUE_MAX] = {0};
    bool isEmbmsSupported = false;
    rfx_property_get("persist.vendor.radio.embms.support", prop, "-1");
    if (!strcmp(prop, "1")) {
        return true;
    } else if (!strcmp(prop, "0")) {
        return false;
    }

    rfx_property_get("ro.vendor.mtk_embms_support", prop, "0");
    if (!strcmp(prop, "1")) {
        isEmbmsSupported = true;
    } else {
        isEmbmsSupported = false;
    }

    return isEmbmsSupported;
}

void RtcEmbmsAtController::onMainCapabilityChanged(RfxStatusKeyEnum key,
    RfxVariant old_value, RfxVariant new_value) {
    String8 responseStr(EMBMS_FLIGTH_MODE);

    EMBMS_UNUSED(key);

    int oldType = old_value.asInt();
    int newType = new_value.asInt();

    logI(RFX_LOG_TAG, "onMainCapabilityChanged[%d] embms:%d, cap %d->%d", getSlotId() ,mEmbmsEnabled
        ,oldType, newType);

    if (oldType != newType && mEmbmsEnabled) {
        logI(RFX_LOG_TAG, "capabilty change %d:%d", oldType, newType);

        sp<RfxMessage> response = RfxMessage::obtainUrc(getSlotId(),
                                        RFX_MSG_URC_EMBMS_AT_INFO,
                                        RfxStringData( (void *)responseStr.string(),
                                            responseStr.length()),
                                        RADIO_TECH_GROUP_GSM);

        urcToSocket(response ,responseStr.string(), NULL);
        mIsFlightOn = true;
        mEmbmsEnabled = false;
        //  clear active info
        if (mSessionInfoList != NULL && !mSessionInfoList->isEmpty()) {
            RtcEmbmsUtils::freeSessionList(mSessionInfoList);
        }
    }
}

void RtcEmbmsAtController::onDefaultDataChanged(RfxStatusKeyEnum key,
    RfxVariant old_value, RfxVariant new_value) {

    String8 responseStr(EMBMS_FLIGTH_MODE);

    EMBMS_UNUSED(key);

    int oldSim = old_value.asInt();
    int newSim = new_value.asInt();
    logI(RFX_LOG_TAG, "onDefaultDataChanged[%d] embms:%d, cap %d->%d", getSlotId() ,mEmbmsEnabled
        ,oldSim, newSim);

    if (oldSim != newSim) {
        if (mEmbmsEnabled) {
            logI(RFX_LOG_TAG, "default sim change %d:%d", oldSim, newSim);
            sp<RfxMessage> response = RfxMessage::obtainUrc(getSlotId(),
                                            RFX_MSG_URC_EMBMS_AT_INFO,
                                            RfxStringData( (void *)responseStr.string(),
                                                responseStr.length()),
                                            RADIO_TECH_GROUP_GSM);

            urcToSocket(response ,responseStr.string(), NULL);
            mIsFlightOn = true;

            // Do not response silent disable eMBMS to middleware
            // The original default sim is mEmbmsEnabled and need send EMBMS_DISABLE
            int intdata[2];
            intdata[0] = -1;
            intdata[1] = EMBMS_COMMAND_ATCI;

            // ignore rilToken since not used
            sp<RfxMessage> newMsg = RfxMessage::obtainRequest(getSlotId(),
                RFX_MSG_REQUEST_EMBMS_DISABLE,
                RfxIntsData(intdata, 2),
                RADIO_TECH_GROUP_GSM);
            requestToMcl(newMsg);
        }

        if (newSim == getSlotId()) {
            sp<RfxMessage> newMsg = RfxMessage::obtainRequest(getSlotId(),
                RFX_MSG_REQUEST_EMBMS_TRIGGER_CELL_INFO_NOTIFY,
                RfxVoidData(),
                RADIO_TECH_GROUP_GSM);
            requestToMcl(newMsg);
        }
    }
}
