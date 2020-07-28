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

#include "RpEmbmsAtController.h"
#include "RpEmbmsUtils.h"
#include "RfxStatusDefs.h"
#include "RfxMainThread.h"

#include "misc.h"
#include <telephony/mtk_ril.h>
#include "socket_channel.h"
#include <sys/socket.h>
#include <cutils/properties.h>
#include <sys/stat.h>

extern "C" {
#include "at_tok.h"
}

#define RFX_LOG_TAG "RP_EMBMS_AT"
#ifdef VDBG
#undef VDBG
#endif
#define VDBG 1

#define EMBMS_OK            "OK\n"
#define EMBMS_ERROR         "ERROR\n"
#define EMBMS_FLIGTH_MODE   "%MBMSEV:99\n"

/*****************************************************************************
 * Class RfxDataController
 * The class is created if the slot is single mode, LWG or C,
 * During class life time always communicate with one modem, gsm or c2k.
 *****************************************************************************/

RFX_IMPLEMENT_CLASS("RpEmbmsAtController", RpEmbmsAtController, RfxController);

RpEmbmsAtController::RpEmbmsAtController() :
    mSdkVersion(11001) {
    writeMutex = PTHREAD_MUTEX_INITIALIZER;
    // If you want to debug like old behavior.
    mSdkVersion = 10603;
}

RpEmbmsAtController::~RpEmbmsAtController() {
    pthread_mutex_unlock(&writeMutex);
    pthread_mutex_destroy(&writeMutex);
}

void RpEmbmsAtController::onInit() {
    RfxController::onInit();  // Required: invoke super class implementation

    logI(RFX_LOG_TAG, "onInit:mSdkVersion:%d", mSdkVersion);
    mSessionInfoList = new Vector<RpEmbmsSessionInfo*>();

    const int request_id_list[] = {
            RIL_REQUEST_EMBMS_AT_CMD,
            RIL_LOCAL_REQUEST_EMBMS_AT_CMD,
            RIL_LOCAL_REQUEST_EMBMS_ENABLE,
            RIL_LOCAL_REQUEST_EMBMS_DISABLE,
            RIL_LOCAL_REQUEST_EMBMS_START_SESSION,
            RIL_LOCAL_REQUEST_EMBMS_STOP_SESSION,
            RIL_LOCAL_REQUEST_EMBMS_GET_TIME,
            RIL_LOCAL_REQUEST_EMBMS_GET_COVERAGE_STATE,
            RIL_LOCAL_REQUEST_EMBMS_TRIGGER_CELL_INFO_NOTIFY,
            };
    const int urc_id_list[] = {
            // RIL_LOCAL_GSM_UNSOL_EMBMS_AT_INFO, // In fact not used.
            RIL_LOCAL_GSM_UNSOL_EMBMS_START_SESSION_RESPONSE,
            RIL_LOCAL_GSM_UNSOL_EMBMS_CELL_INFO_NOTIFICATION,
            RIL_LOCAL_GSM_UNSOL_EMBMS_COVERAGE_STATE,
            RIL_LOCAL_GSM_UNSOL_EMBMS_ACTIVE_SESSION,
            RIL_LOCAL_GSM_UNSOL_EMBMS_AVAILABLE_SESSION,
            RIL_LOCAL_GSM_UNSOL_EMBMS_SAI_LIST_NOTIFICATION,
            RIL_LOCAL_GSM_UNSOL_EMBMS_OOS_NOTIFICATION,
        };

    // register request & URC id list
    // NOTE. one id can only be registered by one controller
    registerToHandleRequest(request_id_list,
            sizeof(request_id_list) / sizeof(int));
    registerToHandleUrc(urc_id_list, sizeof(urc_id_list) / sizeof(int));

    mIsActiveSession = false;
    mIsFlightOn = false;
    mIsEmbmsSupport = isEmbmsSupported();
    mEmbmsEnabled = false;

    if (RpEmbmsUtils::isDualLteSupport()) {
        getNonSlotScopeStatusManager()->registerStatusChanged(RFX_STATUS_KEY_DEFAULT_DATA_SIM,
          RfxStatusChangeCallback(this, &RpEmbmsAtController::onDefaultDataChanged));
    }

    getNonSlotScopeStatusManager()->registerStatusChanged(RFX_STATUS_KEY_MAIN_CAPABILITY_SLOT,
        RfxStatusChangeCallback(this, &RpEmbmsAtController::onMainCapabilityChanged));

    RfxMainThread::waitLooper();
}

void RpEmbmsAtController::onDeinit() {
    logD(RFX_LOG_TAG, "onDeinit");
    RpEmbmsUtils::freeSessionList(mSessionInfoList);
    delete mSessionInfoList;
    mSessionInfoList = NULL;
    RfxController::onDeinit();
}

bool RpEmbmsAtController::onHandleRequest(const sp<RfxMessage>& message) {
    logD(RFX_LOG_TAG, "Handle request %s", requestToString(message->getId()));
    //D.K. TBD Debug only
    logD(RFX_LOG_TAG, "this->getSlotId() %d", this->getSlotId());

    switch (message->getId()) {
        // common commands can send to md1(gsm)/md3(c2k) both.
        case RIL_REQUEST_EMBMS_AT_CMD:
            handleAtRequest(message);
            break;
        default:
            logD(RFX_LOG_TAG, "unknown request, ignore!");
            break;
    }
    return true;
}

bool RpEmbmsAtController::onHandleResponse(const sp<RfxMessage>& message) {
    logD(RFX_LOG_TAG, "Handle response %s.", requestToString(message->getId()));

    switch (message->getId()) {
        case RIL_LOCAL_REQUEST_EMBMS_AT_CMD:
            handleAtResponse(message);
            break;
        case RIL_LOCAL_REQUEST_EMBMS_ENABLE:
            handleEnableResponse(message);
            break;
        case RIL_LOCAL_REQUEST_EMBMS_DISABLE:
            handleDisableResponse(message);
            break;
        case RIL_LOCAL_REQUEST_EMBMS_START_SESSION:
            handleStartSessionResponse(message);
            break;
        case RIL_LOCAL_REQUEST_EMBMS_STOP_SESSION:
            handleStopSessionResponse(message);
            break;
        case RIL_LOCAL_REQUEST_EMBMS_GET_TIME:
            handleGetNetworkTimeResponse(message);
            break;
        case RIL_LOCAL_REQUEST_EMBMS_GET_COVERAGE_STATE:
            handleGetCoverageResponse(message);
            break;
        case RIL_LOCAL_REQUEST_EMBMS_TRIGGER_CELL_INFO_NOTIFY:
            handleTriggerCellInfoNotifyResponse(message);
            break;
        default:
            logD(RFX_LOG_TAG, "unknown response, ignore!");
            break;
    }
    return true;
}

bool RpEmbmsAtController::onHandleUrc(const sp<RfxMessage>& message) {
    logD(RFX_LOG_TAG, "Handle URC %s Slot %d", requestToString(message->getId()), message->getSlotId());

    if (getSlotId() != RpEmbmsUtils::getDefaultDataSlotId()) {
        logD(RFX_LOG_TAG, "ignore URC due to not on DefaultDataSlot:%d", RpEmbmsUtils::getDefaultDataSlotId());
        return true;
    }
    message->resetParcelDataStartPos();
    switch (message->getId()) {
        // case RIL_LOCAL_GSM_UNSOL_EMBMS_AT_INFO
        // Same with solicited AT response, forward string result
        // Should change to RIL_UNSOL_EMBMS_AT_INFO for hidl
        //    handleAtResponse(message);
        //    break;

        case RIL_LOCAL_GSM_UNSOL_EMBMS_START_SESSION_RESPONSE:
            handleStartSessionUrc(message);
            break;

        case RIL_LOCAL_GSM_UNSOL_EMBMS_CELL_INFO_NOTIFICATION:
            handleCellInfoUrc(message);
            break;

        case RIL_LOCAL_GSM_UNSOL_EMBMS_COVERAGE_STATE:
            handleCoverageUrc(message);
            break;

        case RIL_LOCAL_GSM_UNSOL_EMBMS_OOS_NOTIFICATION:
            if (mSdkVersion >= 10901) {
                handleOosUrc(message);
            }
            break;
        case RIL_LOCAL_GSM_UNSOL_EMBMS_SAI_LIST_NOTIFICATION:
            if (mSdkVersion >= 10901) {
                handleSaiUrc(message);
            }
            break;

        case RIL_LOCAL_GSM_UNSOL_EMBMS_ACTIVE_SESSION:
            handleActiveSessionStatus(message);
            message->resetParcelDataStartPos();
            handleSessionListUrc(message);
            break;

        case RIL_LOCAL_GSM_UNSOL_EMBMS_AVAILABLE_SESSION:
            handleSessionListUrc(message);
            break;

        default:
            logD(RFX_LOG_TAG, "unknown urc, ignore!");
            break;
    }
    return true;
}

void RpEmbmsAtController::handleAtRequest(const sp<RfxMessage>& request) {
    int need_forward = 0;
    if (!mIsEmbmsSupport) {
        logD(RFX_LOG_TAG, "mIsEmbmsSupport = %d", mIsEmbmsSupport);
        responseErrorString(request);
        return;
    }

    Parcel* parcel = request->getParcel();
    char *data = RpEmbmsUtils::strdupReadString(parcel);
    request->resetParcelDataStartPos();

    // Check AT command format.
    if (data != NULL) {
        logD(RFX_LOG_TAG, "handleAtRequest:%s", (char*)data);
        int totalLength = strlen(data);
        if (totalLength > (512-1)) {
            logE(RFX_LOG_TAG, "Error!! The length is too long (more than 512)");
            if (data != NULL) {
                free(data);
            }
            return;
        } else if (totalLength > 0 &&
                data[totalLength - 1] != '\r' &&
                data[totalLength - 1] != '\n') {
            logE(RFX_LOG_TAG, "Dbg terminate character:%d", data[totalLength - 1] );
        }
    }

    if (data == NULL) {
        logE(RFX_LOG_TAG, "Error!! requestEmbmsAt: get null!!");
        responseErrorString(request);
    } else if (strStartsWith(data, "AT%MBMSCMD=\"CURRENT_SAI_LIST\"")) {
        logD(RFX_LOG_TAG, "Pass to rild");
        need_forward = 1;
    } else if (strStartsWith(data, "AT%MBMSCMD=\"NETWORK INFORMATION\"")) {
        logD(RFX_LOG_TAG, "Pass to rild");
        need_forward = 1;
    } else if (strStartsWith(data, "AT%MBMSCMD=\"BSSI_SIGNAL_LEVEL\"")) {
        logD(RFX_LOG_TAG, "Pass to rild");
        need_forward = 1;
    } else if (strStartsWith(data, "AT+CMEE=")) {
        logD(RFX_LOG_TAG, "Pass to rild");
        need_forward = 1;
    } else if (strStartsWith(data, "AT+CFUN?")) {
        logD(RFX_LOG_TAG, "Pass to rild");
        need_forward = 1;
    } else if (strStartsWith(data, "AT%MBMSEV=")) {
        logD(RFX_LOG_TAG, "Pass to rild");
        need_forward = 1;
    } else if (strStartsWith(data, "AT%MBMSEV?")) {
        requestAtGetCoverage(request);
        logD(RFX_LOG_TAG, "requestAtGetCoverage");
    } else if (strStartsWith(data, "AT%MBMSCMD?")) {
        logD(RFX_LOG_TAG, "Pass to rild");
        need_forward = 1;
    } else if (strStartsWith(data, "AT%MBMSCMD=?")) {
        logD(RFX_LOG_TAG, "Pass to rild");
        need_forward = 1;
    } else if (strStartsWith(data, "AT%MBMSCMD=\"DEVICE INFO\"")) {
        logD(RFX_LOG_TAG, "Pass to rild");
        need_forward = 1;
    } else if (strStartsWith(data, "ATE")) {
        logD(RFX_LOG_TAG, "Pass to rild");
        need_forward = 1;
    // --- Special handle area ---
    } else if (strStartsWith(data, "AT%MBMSCMD=\"SIB16_GET_NETWORK_TIME\"")) {
        requestAtGetNetworkTime(request);
        logD(RFX_LOG_TAG, "requestAtGetNetworkTime");
    } else if (strStartsWith(data, "AT%MBMSCMD=\"MBMS_PREFERENCE\"")) {
        if (RpEmbmsUtils::isAtCmdEnableSupport()) {
            logD(RFX_LOG_TAG, "Pass to rild");
            need_forward = 1;
        } else {
            requestAtEnable(request);
            logD(RFX_LOG_TAG, "requestAtSetPreference");
        }
    } else if (strStartsWith(data, "AT%MBMSCMD=\"ENABLE_EMBMS\"")) {
        requestAtEnable(request);
        logI(RFX_LOG_TAG, "requestAtEnable.mSdkVersion:%d", mSdkVersion);
    } else if (strStartsWith(data, "AT%MBMSCMD=\"AVAILABILITY_INFO\"")) {
        requestAtSetAvailbility(request);
        logD(RFX_LOG_TAG, "requestAtSetAvailbility");
    } else if (strStartsWith(data, "AT%MBMSCMD=\"ACTIVATE\"")) {
        requestAtActivate(request, EMBMS_ACTIVE_SESSION);
        logD(RFX_LOG_TAG, "requestAtActivate 1");
    } else if (strStartsWith(data, "AT%MBMSCMD=\"DEACTIVATE\"")) {
        requestAtActivate(request, EMBMS_DEACTIVE_SESSION);
        logD(RFX_LOG_TAG, "requestAtActivate 0");
    } else if (strStartsWith(data, "AT%MBMSCMD=\"DEACTIVATE_ALL\"")) {
        requestAtActivate(request, EMBMS_DEACTIVE_ALL_SESSION);
        logD(RFX_LOG_TAG, "requestAtActivate 2");
    // --- Not support area ---
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
    } else {
        logD(RFX_LOG_TAG, "Not support");
        responseErrorString(request);
    }

    if (need_forward) {/* Need forward */
        //Change RIL_REQUEST_EMBMS_AT_CMD to RIL_LOCAL_REQUEST_EMBMS_AT_CMD
        sp<RfxMessage> newMsg = RfxMessage::obtainRequest(RADIO_TECH_GROUP_GSM,
            RIL_LOCAL_REQUEST_EMBMS_AT_CMD, request, true);

        handleRequestDefault(newMsg);
    }

    if (data != NULL) {
        free(data);
    }
}

void RpEmbmsAtController::responseErrorString(const sp<RfxMessage>& response) {
    char *responseStr = (char*) EMBMS_ERROR;
    responseToAtci(response, responseStr);
}

void RpEmbmsAtController::responseToAtci(const sp<RfxMessage>& response,
        const char *responseStr) {
    logD(RFX_LOG_TAG, "Start responseToEmbmsAt: %s.", responseStr);
    // Change RIL_LOCAL_REQUEST_EMBMS_AT_CMD to RIL_REQUEST_EMBMS_AT_CMD for hidl
    Parcel* np = new Parcel();
    np->writeInt32(0);  // RESPONSE_SOLICITED
    np->writeInt32(response->getPToken() & 0xFFFFFFFF);
    np->writeInt32(response->getError() & 0xFFFFFFFF);

    sp<RfxMessage> newMsg = RfxMessage::obtainResponse(response->getSlotId(), response->getPId(),
        response->getPToken(), RIL_REQUEST_EMBMS_AT_CMD, response->getToken(), response->getError(),
        response->getSource(), np, response->getPTimeStamp(), response->getRilToken());
    writeStringToParcel(np, responseStr);
    responseToRilj(newMsg);
}

void RpEmbmsAtController::urcToAtci(const sp<RfxMessage>& response,
        const char *responseStr, const char *responseStr_mask) {
    if (responseStr_mask) {
        logD(RFX_LOG_TAG, "Start urcToAtci: %s.", responseStr_mask);
    } else {
        logD(RFX_LOG_TAG, "Start urcToAtci: %s.", responseStr);
    }

    sp<RfxMessage> newMsg = RfxMessage::obtainUrc(response->getSlotId(),
                                    RIL_UNSOL_EMBMS_AT_INFO,
                                    response, false);
    Parcel* np = newMsg->getParcel();
    writeStringToParcel(np, responseStr);
    responseToRilj(newMsg);
}

void RpEmbmsAtController::handleAtResponse(const sp<RfxMessage>& response) {
    int32_t type = response->getType();
    int32_t token = response->getToken();
    int32_t error = response->getError();

    char *responseStr = NULL;
    bool returnError = false;
    int err = 0;
    Parcel* p = response->getParcel();
    if (error != RIL_E_SUCCESS) {
        responseErrorString(response);
        return;
    } else {
        responseStr = RpEmbmsUtils::strdupReadString(p);
        if (responseStr == NULL) {
            logE(RFX_LOG_TAG, "Error! RpEmbmsUtils::strdupReadString get NULL");
            responseErrorString(response);
            return;
        }
        logD(RFX_LOG_TAG, "type: %d, token %d, error %d, str:%s"
                , type, token, error, responseStr);
        responseToAtci(response, responseStr);
        free(responseStr);
    }
}

void RpEmbmsAtController::handleRequestDefault(const sp<RfxMessage>& request) {
    sp<RfxMessage> message = RfxMessage::obtainRequest(RADIO_TECH_GROUP_GSM,
                                                        request->getId(),
                                                        request,
                                                        true);

    requestToRild(message);
    logD(RFX_LOG_TAG, "Send request %s",
            requestToString(request->getId()));
}


void RpEmbmsAtController::requestAtEnable(const sp<RfxMessage>& request) {
    // new: AT%MBMSCMD="ENABLE_EMBMS",[0,1]
    // old: AT%MBMSCMD="MBMS_PREFERENCE",[0,1]
    Parcel* parcel = request->getParcel();
    char *data = RpEmbmsUtils::strdupReadString(parcel);
    char *data_to_free = data;
    int input_err = 0;
    int enable;

    int count;
    int type;
    int trans_id;
    char *line;

    mIsFlightOn = false;
    if (data == NULL) {
        logE(RFX_LOG_TAG, "Error! RpEmbmsUtils::strdupReadString get NULL");
        responseErrorString(request);
        return;
    }

    //  skip to next ','
    input_err = at_tok_nextstr(&data, &line);
    if (input_err < 0) {
        logE(RFX_LOG_TAG, "Input parameter error!!");
        if (data_to_free != NULL) {
            free(data_to_free);
        }
        responseErrorString(request);
        return;
    }

    input_err = at_tok_nextint(&data, &enable);
    if (input_err < 0) {
        logE(RFX_LOG_TAG, "Input parameter error!!");
        if (data_to_free != NULL) {
            free(data_to_free);
        }
        responseErrorString(request);
        return;
    }

    if (data_to_free != NULL) {
        free(data_to_free);
    }

    //  initial global data array for active info
    if (mSessionInfoList != NULL && !mSessionInfoList->isEmpty()) {
        RpEmbmsUtils::freeSessionList(mSessionInfoList);
    }

    int msg = RIL_LOCAL_REQUEST_EMBMS_ENABLE;
    if (enable) {
        msg = RIL_LOCAL_REQUEST_EMBMS_ENABLE;
    } else {
        msg = RIL_LOCAL_REQUEST_EMBMS_DISABLE;
    }

    sp<RfxMessage> newMsg = RfxMessage::obtainRequest(RADIO_TECH_GROUP_GSM,
        msg, request, false);
    Parcel* newParcel = newMsg->getParcel();
    count = 2;
    trans_id = request->getToken();

    newParcel->writeInt32(count);
    newParcel->writeInt32(trans_id);
    newParcel->writeInt32(EMBMS_COMMAND_ATCI);
    requestToRild(newMsg);
}

void RpEmbmsAtController::handleEnableResponse(const sp<RfxMessage>& response) {
    RIL_EMBMS_LocalEnableResp entry;
    int32_t tmp = 0;
    int id = response->getId();
    memset(&entry, 0, sizeof(entry));
    entry.response = EMBMS_GENERAL_FAIL;
    Parcel* p = response->getParcel();
    char *responseStr = NULL;

    mIsActiveSession = false;

    if (response->getError() == RIL_E_SUCCESS && p->dataAvail() > 0) {
        p->readInt32(&entry.trans_id);
        p->readInt32(&entry.response);
        p->readInt32(&tmp);
        entry.interface_index_valid = tmp;
        p->readInt32(&tmp);
        entry.interface_index = tmp;
        char* interface_name = RpEmbmsUtils::strdupReadString(p);
        if (interface_name == NULL) {
            logE(RFX_LOG_TAG, "Error! RpEmbmsUtils::strdupReadString get NULL");
            responseErrorString(response);
            return;
        }
        strncpy(entry.interface_name, interface_name, EMBMS_MAX_LEN_INTERFACE);
        free(interface_name);
        logD(RFX_LOG_TAG,  "handleEnableResponse request %d,tid %d,rsp %d,idx_valid %d,idx %d,\
            iface_name %s",
            id, entry.trans_id, entry.response, entry.interface_index_valid, entry.interface_index,
            entry.interface_name);
    } else {
        logD(RFX_LOG_TAG,  "handleEnableResponse error!");
    }


    if (response->getError() == RIL_E_SUCCESS && entry.response == EMBMS_GENERAL_SUCCESS) {
        // for RJIL old middleware version
        if (RpEmbmsUtils::isRjilSupport()) {
            asprintf(&responseStr, EMBMS_OK);
        } else {
            //%MBMSCMD:rmnet1,1
            //OK
            asprintf(&responseStr, "%%MBMSCMD:%s,%d\nOK\n", entry.interface_name,
                entry.interface_index);
        }
        mEmbmsEnabled = true;
    } else {
        asprintf(&responseStr, EMBMS_ERROR);
    }

    responseToAtci(response, responseStr);
    free(responseStr);
}

void RpEmbmsAtController::handleDisableResponse(const sp<RfxMessage>& response) {
    RIL_EMBMS_LocalDisableResp entry;
    int32_t tmp;
    int id = response->getId();
    memset(&entry, 0, sizeof(entry));
    entry.response = EMBMS_GENERAL_FAIL;
    Parcel* p = response->getParcel();
    char *responseStr = NULL;

    if (response->getError() == RIL_E_SUCCESS && p->dataAvail() > 0) {
        p->readInt32(&entry.trans_id);
        p->readInt32(&entry.response);
        logD(RFX_LOG_TAG,  "handleDisableResponse request %d,tid %d,rsp %d",
        id, entry.trans_id, entry.response);
    } else {
        logD(RFX_LOG_TAG,  "handleDisableResponse error!");
    }

    if (response->getError() == RIL_E_SUCCESS && entry.response == EMBMS_GENERAL_SUCCESS) {
        asprintf(&responseStr, EMBMS_OK);
        mEmbmsEnabled = false;
        //  clear active info
        if (mSessionInfoList != NULL && !mSessionInfoList->isEmpty()) {
            RpEmbmsUtils::freeSessionList(mSessionInfoList);
        }
    } else {
        asprintf(&responseStr, EMBMS_ERROR);
    }

    // Do not response silent disable eMBMS to middleware
    // The original default sim is not current default sim, so do not response

    if (getSlotId() != RpEmbmsUtils::getDefaultDataSlotId()) {
        logD(RFX_LOG_TAG,  "ignore DisableResponse due to not on DefaultDataSlot:%d",
            RpEmbmsUtils::getDefaultDataSlotId());
    } else {
        responseToAtci(response, responseStr);
    }

    free(responseStr);
}

void RpEmbmsAtController::handleStartSessionResponse(const sp<RfxMessage>& response) {
    RIL_EMBMS_LocalStartSessionResp entry;
    memset(&entry, 0, sizeof(entry));
    int32_t tmp = 0;
    int id = response->getId();

    entry.response = EMBMS_GENERAL_FAIL;
    Parcel* p = response->getParcel();

    // Only handle fail case here; asynchronous success response later.
    if (response->getError() == RIL_E_SUCCESS && p->dataAvail() > 0) {
        p->readInt32(&entry.trans_id);
        p->readInt32(&entry.response);
        p->readInt32(&tmp);
        entry.tmgi_info_valid = tmp;
        char* tmgi = RpEmbmsUtils::strdupReadString(p);
        if (tmgi != NULL) {
            strncpy(entry.tmgi, tmgi, EMBMS_MAX_LEN_TMGI);
            free(tmgi);
        } else {
            RFX_LOG_E(RFX_LOG_TAG, "Error! RpEmbmsUtils::strdupReadString get NULL");
        }

        logD(RFX_LOG_TAG,  "handleStartSessionResponse request %d,tid %d,rsp %d,tmgi %s",
        id, entry.trans_id, entry.response, entry.tmgi);

        if (entry.response != EMBMS_GENERAL_SUCCESS) {
            int index;
            RpEmbmsSessionInfo* sessionInfo =
                RpEmbmsUtils::findSessionByTransId(mSessionInfoList, entry.trans_id, &index);
            if (sessionInfo != NULL) {
                delete mSessionInfoList->itemAt(index);
                mSessionInfoList->removeAt(index);
                logD(RFX_LOG_TAG, "Send resposne %s",
                        requestToString(sessionInfo->mOriginalRequest->getId()));
            } else {
                logD(RFX_LOG_TAG, "Fail to find session in send resposne");
            }

            responseErrorString(response);
            return;
        }
    } else {
        responseErrorString(response);
    }
}

void RpEmbmsAtController::handleStartSessionUrc(const sp<RfxMessage>& response) {
    RIL_EMBMS_LocalStartSessionResp entry;
    int32_t tmp = 0;
    memset(&entry, 0, sizeof(entry));
    Parcel* p = response->getParcel();
    int index = 0;

    p->readInt32(&entry.trans_id);
    p->readInt32(&entry.response);
    p->readInt32(&tmp);
    entry.tmgi_info_valid = tmp;

    logD(RFX_LOG_TAG, "got StartSessionUrc, reason:%d, valid%d", entry.response, entry.tmgi_info_valid);
    if (entry.tmgi_info_valid == 1) {
        char* tmgi = RpEmbmsUtils::strdupReadString(p);
        if (tmgi == NULL) {
            logE(RFX_LOG_TAG, "Error! RpEmbmsUtils::strdupReadString get NULL");
            responseErrorString(response);
            return;
        }
        strncpy(entry.tmgi, tmgi, EMBMS_MAX_LEN_TMGI);
        free(tmgi);

        logD(RFX_LOG_TAG, "urc tmgi:%s", entry.tmgi);
        RpEmbmsSessionInfo* sessionInfo = RpEmbmsUtils::findSessionByTmgi(
                                        mSessionInfoList, strlen(entry.tmgi), entry.tmgi, &index);
        if (sessionInfo != NULL && index != -1) {
            char *responseStr = NULL;
            // To avoid when MD response EMSSS too slow and middleware re-send AVAILABILITY_INFO,
            // and then EMSESS URC come back before ACTIVATE. Just ignore previous URC
            if (sessionInfo->mOriginalRequest == NULL) {
                return;
            }

            if (entry.response == EMBMS_SESSION_SUCCESS) {
                asprintf(&responseStr, EMBMS_OK);
            } else {
                asprintf(&responseStr, EMBMS_ERROR);
            }

            responseToAtci(sessionInfo->mOriginalRequest, responseStr);
            delete mSessionInfoList->itemAt(index);
            mSessionInfoList->removeAt(index);
            free(responseStr);
            return;
        }
    }
}

void RpEmbmsAtController::handleStopSessionResponse(const sp<RfxMessage>& response) {
    RIL_EMBMS_LocalStartSessionResp entry;
    memset(&entry, 0, sizeof(entry));
    int32_t tmp = 0;
    int id = response->getId();
    entry.response = EMBMS_GENERAL_FAIL;
    Parcel* p = response->getParcel();
    char *responseStr = NULL;

    if (response->getError() == RIL_E_SUCCESS && p->dataAvail() > 0) {
        p->readInt32(&entry.trans_id);
        p->readInt32(&entry.response);
        p->readInt32(&tmp);
        entry.tmgi_info_valid = tmp;
        char* tmgi = RpEmbmsUtils::strdupReadString(p);
        if (tmgi != NULL) {
            strncpy(entry.tmgi, tmgi, EMBMS_MAX_LEN_TMGI);
            free(tmgi);
        } else {
            logE(RFX_LOG_TAG, "Error! RpEmbmsUtils::strdupReadString get NULL");
        }

        logD(RFX_LOG_TAG,  "handleStopSessionResponse request %d,tid %d,rsp %d,tmgi %s",
        id, entry.trans_id, entry.response, entry.tmgi);
    } else {
        logD(RFX_LOG_TAG,  "handleStopSessionResponse error!");
    }

    if (response->getError() == RIL_E_SUCCESS && entry.response == EMBMS_GENERAL_SUCCESS) {
        asprintf(&responseStr, EMBMS_OK);
    } else {
        asprintf(&responseStr, EMBMS_ERROR);
    }

    responseToAtci(response, responseStr);
    free(responseStr);
}

void RpEmbmsAtController::requestAtSetAvailbility(const sp<RfxMessage>& request) {
    //  AT%MBMSCMD="AVAILABILITY_INFO",<TMGI>,<SAI_COUNT>,
    //  [<sai1>,<FREQUENCY_COUNT>,[<Freq1>,[<Freq2>,..]],
    //  [<sai2>,<FREQUENCY_COUNT>,[<Freq1>,[<Freq2>,..]]..]]
    Parcel* parcel = request->getParcel();
    char *data = RpEmbmsUtils::strdupReadString(parcel);
    char *data_to_free = data;
    char *tmgi = NULL;
    int input_err = 0;
    RpEmbmsSessionInfo inputInfo;
    int sai_count = 0;
    int sai = 0;
    int freq_count = 0;
    int freq = 0;
    int i, j;

    if (data == NULL) {
        logE(RFX_LOG_TAG, "Error! RpEmbmsUtils::strdupReadString get NULL");
        responseErrorString(request);
        return;
    }

    //  skip to next ','
    input_err = at_tok_nextstr(&data, &tmgi);
    if (input_err < 0) {
        logE(RFX_LOG_TAG, "Input parameter error!!");
        responseErrorString(request);
        return;
    }

    // tmgi
    input_err = at_tok_nextstr(&data, &tmgi);
    if (input_err < 0) {
        logE(RFX_LOG_TAG, "Input parameter error!!");
        responseErrorString(request);
        return;
    }

    if (VDBG) logD(RFX_LOG_TAG, "tmgi = %s", tmgi);

    inputInfo.mFreqCount = 0;
    inputInfo.mOriginalRequest = NULL;
    int tmgi_len = (strlen(tmgi) > EMBMS_MAX_LEN_TMGI) ? EMBMS_MAX_LEN_TMGI : strlen(tmgi);
    strncpy(inputInfo.mTmgi, tmgi, EMBMS_MAX_LEN_TMGI);
    inputInfo.mTmgi[tmgi_len] = '\0';
    inputInfo.mTmgiLen = tmgi_len;
    // sai_count
    input_err = at_tok_nextint(&data, &sai_count);
    if (input_err < 0) {
        logE(RFX_LOG_TAG, "Input parameter error!!");
        responseErrorString(request);
        return;
    }

    // TBD: trick, change sai count from 0 to 1.
    if (sai_count == 0 && at_tok_hasmore(&data)) {
        sai_count = 1;
    }

    inputInfo.mSaiCount = sai_count;

    if (VDBG) logD(RFX_LOG_TAG, "sai_count = %d", sai_count);

    if (inputInfo.mSaiCount > EMBMS_MAX_NUM_SAI) {
        logE(RFX_LOG_TAG, "Cut sai_count(%d) to EMBMS_MAX_NUM_SAI(%d)",
                inputInfo.mSaiCount, EMBMS_MAX_NUM_SAI);
        inputInfo.mSaiCount = EMBMS_MAX_NUM_SAI;
    }

    // sai & freq
    for (i = 0; i < inputInfo.mSaiCount; i++) {
        input_err = at_tok_nextint(&data, &sai);

        if (input_err < 0) {
            logE(RFX_LOG_TAG, "Input parameter error!!");
            responseErrorString(request);
            return;
        }

        if (i < EMBMS_MAX_NUM_SAI) {
            inputInfo.mSais[i] = sai;
            if (VDBG) logD(RFX_LOG_TAG, "sai = %d", sai);
        }

        input_err = at_tok_nextint(&data, &freq_count);

        if (input_err < 0) {
            logE(RFX_LOG_TAG, "Input parameter error!!");
            responseErrorString(request);
            return;
        }

        for (j = 0; j < freq_count; j++) {
            input_err = at_tok_nextint(&data, &freq);

            if (input_err < 0) {
                logE(RFX_LOG_TAG, "Input parameter error!!");
                responseErrorString(request);
                return;
            }

            if (inputInfo.mFreqCount + j < EMBMS_MAX_NUM_FREQ) {
                inputInfo.mFreqs[inputInfo.mFreqCount + j] = freq;
            }
        }

        inputInfo.mFreqCount = inputInfo.mFreqCount + freq_count;

        if (inputInfo.mFreqCount > EMBMS_MAX_NUM_FREQ) inputInfo.mFreqCount = EMBMS_MAX_NUM_FREQ;
    }

    //  Update or add
    int index = 0;
    RpEmbmsSessionInfo* sessionInfo = RpEmbmsUtils::findSessionByTmgi(
                                    mSessionInfoList, tmgi_len, tmgi, &index);

    if (index == -1) {
        RpEmbmsSessionInfo* info = new RpEmbmsSessionInfo();
        info->copy(inputInfo);
        mSessionInfoList->add(info);
        logD(RFX_LOG_TAG, "new avail info, tmgi:%s", info->mTmgi);
        for (RpEmbmsSessionInfo* item : *mSessionInfoList) {
            item->dump();
        }
    } else {
        sessionInfo->copy(inputInfo);
        logD(RFX_LOG_TAG, "update avail info, tmgi:%s", sessionInfo->mTmgi);
        for (RpEmbmsSessionInfo* item : *mSessionInfoList) {
            item->dump();
        }
    }
    // response

    if (data_to_free != NULL) {
        free(data_to_free);
    }

    char *responseStr = (char *) EMBMS_OK;
    responseToAtci(request, responseStr);
}

void RpEmbmsAtController::requestAtActivate(const sp<RfxMessage>& request
    , embms_at_activesession_enum type) {
    Parcel* parcel = request->getParcel();
    char *data = RpEmbmsUtils::strdupReadString(parcel);
    char *data_to_free = data;
    char *tmgi = NULL;
    int sessionId = INVALID_EMBMS_SESSION_ID;
    int input_err = 0;
    int trans_id = 0;
    int mcast_ip_address = 0;

    int msg = RIL_LOCAL_REQUEST_EMBMS_START_SESSION;
    trans_id = request->getToken();

    if (data == NULL) {
        logE(RFX_LOG_TAG, "Error! RpEmbmsUtils::strdupReadString get NULL");
        responseErrorString(request);
        return;
    }

    // Select type
    switch (type) {
        case EMBMS_DEACTIVE_SESSION:
        case EMBMS_DEACTIVE_ALL_SESSION:
            msg = RIL_LOCAL_REQUEST_EMBMS_STOP_SESSION;
            break;
        case EMBMS_ACTIVE_SESSION:
            msg = RIL_LOCAL_REQUEST_EMBMS_START_SESSION;
            break;
        default:
            break;
    }

    // Parse input parameter according to type
    if (type == EMBMS_DEACTIVE_ALL_SESSION) {
        //  AT%MBMSCMD="DEACTIVATE_ALL"
        tmgi = NULL;
        sessionId = INVALID_EMBMS_SESSION_ID;
    } else if (type == EMBMS_DEACTIVE_SESSION || type == EMBMS_ACTIVE_SESSION) {
        //  AT%MBMSCMD="ACTIVATE/DEACTIVATE",<TMGI-string>,<SessionID-int>,<Multicast_IP_Address-int>
        //  skip to next ','
        input_err = at_tok_nextstr(&data, &tmgi);
        if (input_err < 0) {
            logE(RFX_LOG_TAG, "Input parameter error!!");
            responseErrorString(request);
            return;
        }

        // tmgi
        input_err = at_tok_nextstr(&data, &tmgi);
        if (input_err < 0) {
            logE(RFX_LOG_TAG, "Input parameter error!!");
            responseErrorString(request);
            return;
        }

        // sessionId
        input_err = at_tok_nextint(&data, &sessionId);
        if (input_err < 0) sessionId = INVALID_EMBMS_SESSION_ID;

        // mcast_ip_address, not used
        input_err = at_tok_nextint(&data, &mcast_ip_address);
        if (input_err < 0) {
            logE(RFX_LOG_TAG, "Input parameter error!!");
            responseErrorString(request);
            return;
        }
    } else {
        logE(RFX_LOG_TAG, "requestAtActivate error!!Type not support!!");
        return;
    }

    // Query session or clear session if nessary
    // Compose command parameters
    if (type == EMBMS_DEACTIVE_ALL_SESSION || type == EMBMS_DEACTIVE_SESSION) {
        if (type == EMBMS_DEACTIVE_ALL_SESSION) {
            //  clear active info
            if (mSessionInfoList != NULL && !mSessionInfoList->isEmpty()) {
                RpEmbmsUtils::freeSessionList(mSessionInfoList);
            }
        } else {  // EMBMS_DEACTIVE_SESSION
            int index = 0;
            RpEmbmsSessionInfo* sessionInfo = RpEmbmsUtils::findSessionByTmgi(
                                            mSessionInfoList, strlen(tmgi), tmgi, &index);
            if (index != -1) {
                delete mSessionInfoList->itemAt(index);
                mSessionInfoList->removeAt(index);
            }
        }

        RIL_EMBMS_LocalStopSessionReq entry;
        entry.stop_type = type;
        entry.trans_id = trans_id;
        if (tmgi != NULL) {
            strncpy(entry.tmgi, tmgi, EMBMS_MAX_LEN_TMGI);
        } else {
            strncpy(entry.tmgi, "", EMBMS_MAX_LEN_TMGI);
        }
        entry.session_id = sessionId;

        sp<RfxMessage> newMsg = RfxMessage::obtainRequest(RADIO_TECH_GROUP_GSM,
            msg, request, false);
        Parcel* newParcel = newMsg->getParcel();

        newParcel->writeInt32(entry.stop_type);
        newParcel->writeInt32(entry.trans_id);
        RpEmbmsUtils::writeStringToParcel(*newParcel, entry.tmgi);
        newParcel->writeInt32(entry.session_id);

        requestToRild(newMsg);
    } else if (type == EMBMS_ACTIVE_SESSION) {
        int index;
        RpEmbmsSessionInfo* sessionInfo = RpEmbmsUtils::findSessionByTmgi(
                                        mSessionInfoList, strlen(tmgi), tmgi, &index);
        if (index == -1) {
            logE(RFX_LOG_TAG, "Can't find availbility info for %s!!", tmgi);
            responseErrorString(request);
            return;
        }
        sessionInfo->mTransId = trans_id;
        sessionInfo->mOriginalRequest = request;

        RIL_EMBMS_LocalStartSessionReq entry;
        entry.trans_id = trans_id;
        strncpy(entry.tmgi, tmgi, EMBMS_MAX_LEN_TMGI);
        entry.session_id = sessionId;
        entry.earfcnlist_count = sessionInfo->mFreqCount;
        entry.saiList_count = sessionInfo->mSaiCount;

        sp<RfxMessage> newMsg = RfxMessage::obtainRequest(RADIO_TECH_GROUP_GSM,
            msg, request, false);
        Parcel* newParcel = newMsg->getParcel();

        newParcel->writeInt32(entry.trans_id);
        RpEmbmsUtils::writeStringToParcel(*newParcel, entry.tmgi);
        newParcel->writeInt32(entry.session_id);
        newParcel->writeInt32(entry.earfcnlist_count);
        for (uint32_t i = 0; i < entry.earfcnlist_count; i++) {
            newParcel->writeInt32(sessionInfo->mFreqs[i]);
        }
        newParcel->writeInt32(entry.saiList_count);
        for (uint32_t i = 0; i < entry.saiList_count; i++) {
            newParcel->writeInt32(sessionInfo->mSais[i]);
        }

        requestToRild(newMsg);
    }

    if (data_to_free != NULL) {
        free(data_to_free);
    }
}

void RpEmbmsAtController::requestAtGetCoverage(const sp<RfxMessage>& request) {
    // AT%MBMSEV?
    // No parameter parse.
    int count;
    int trans_id;
    sp<RfxMessage> newMsg = RfxMessage::obtainRequest(RADIO_TECH_GROUP_GSM,
        RIL_LOCAL_REQUEST_EMBMS_GET_COVERAGE_STATE, request, false);
    Parcel* newParcel = newMsg->getParcel();
    count = 1;
    trans_id = request->getToken();

    newParcel->writeInt32(count);
    newParcel->writeInt32(trans_id);
    requestToRild(newMsg);
}

void RpEmbmsAtController::handleGetCoverageResponse(const sp<RfxMessage>& response) {
    RIL_EMBMS_GetCoverageResp entry;
    int32_t tmp = 0;
    int id = response->getId();
    memset(&entry, 0, sizeof(entry));
    entry.response = EMBMS_GENERAL_FAIL;
    entry.coverage_state_valid = false;
    Parcel* p = response->getParcel();
    char *responseStr = NULL;
    uint32_t result_status;

    if (response->getError() == RIL_E_SUCCESS && p->dataAvail() > 0) {
        p->readInt32(&entry.trans_id);
        p->readInt32(&entry.response);
        p->readInt32(&tmp);
        entry.coverage_state_valid = tmp;
        if (entry.coverage_state_valid) {
            p->readInt32(&tmp);
            entry.coverage_state = tmp;
            logD(RFX_LOG_TAG,  "handleGetCoverageResponse request %d,tid %d,rsp %d,valid %d,cov %d",
                id, entry.trans_id, entry.response, entry.coverage_state_valid, entry.coverage_state);

            result_status = getCoverageState(entry.coverage_state);
            entry.coverage_state = result_status;
        } else {
            result_status = getCoverageState(EMBMS_LOCAL_SRV_OOS);
            entry.coverage_state = result_status;
        }
    } else {
        logD(RFX_LOG_TAG,  "handleGetCoverageResponse error!");
    }


    if (response->getError() == RIL_E_SUCCESS && entry.response == EMBMS_GENERAL_SUCCESS) {
        // for RJIL old middleware version
        if (RpEmbmsUtils::isRjilSupport()) {
            asprintf(&responseStr, "%%MBMSEV:%d\nOK\n", entry.coverage_state);
        } else {
            asprintf(&responseStr, "%%MBMSEV:%d,0\nOK\n", entry.coverage_state);
        }
    } else {
        asprintf(&responseStr, EMBMS_ERROR);
    }

    responseToAtci(response, responseStr);
    free(responseStr);
}

void RpEmbmsAtController::requestAtGetNetworkTime(const sp<RfxMessage>& request) {
    // AT%MBMSCMD="SIB16_GET_NETWORK_TIME"
    // No parameter parse.
    int count;
    int trans_id;
    sp<RfxMessage> newMsg = RfxMessage::obtainRequest(RADIO_TECH_GROUP_GSM,
        RIL_LOCAL_REQUEST_EMBMS_GET_TIME, request, false);
    Parcel* newParcel = newMsg->getParcel();
    count = 1;
    trans_id = request->getToken();

    newParcel->writeInt32(count);
    newParcel->writeInt32(trans_id);
    requestToRild(newMsg);
}

void RpEmbmsAtController::handleGetNetworkTimeResponse(const sp<RfxMessage>& response) {
    RIL_EMBMS_LocalGetTimeResp entry;
    int32_t tmp = 0;
    int64_t tmp6 = 0;
    int id = response->getId();
    memset(&entry, 0, sizeof(entry));
    entry.response = EMBMS_GENERAL_FAIL;
    Parcel* p = response->getParcel();
    char *responseStr = NULL;

    if (response->getError() == RIL_E_SUCCESS && p->dataAvail() > 0) {
        p->readInt32(&entry.trans_id);
        p->readInt32(&entry.response);
        if (entry.response == EMBMS_GET_TIME_SIB16) {
            entry.response = EMBMS_GENERAL_SUCCESS;
            p->readInt64(&tmp6);
            // The timing information is consisting of time Info UTC part of SIB-16.
            // The field indicates the integer count of 10 ms units since 00:00:00
            // on 1 January, 1900 as per 3GPP specification, 36.331
            entry.milli_sec = tmp6 / 10ll;
            p->readInt32(&tmp);
            entry.day_light_saving_valid = tmp;
            p->readInt32(&tmp);
            entry.day_light_saving = tmp;
            p->readInt32(&tmp);
            entry.leap_seconds_valid = tmp;
            p->readInt32(&tmp);
            entry.leap_seconds = tmp;
            p->readInt32(&tmp);
            entry.local_time_offset_valid = tmp;
            p->readInt32(&tmp);
            entry.local_time_offset = tmp;
        }
        logD(RFX_LOG_TAG, "handleGetNetworkTimeResponse request %d,tid %d,rsp %u,sec %llu",
            id, entry.trans_id, entry.response,
            entry.milli_sec);
        logD(RFX_LOG_TAG, "dlsaving_valid %d, dlsaving %d,ls_valid %d,ls %d,lto_valid %d,lto %d",
            entry.day_light_saving_valid, entry.day_light_saving,
            entry.leap_seconds_valid, entry.leap_seconds,
            entry.local_time_offset_valid, entry.local_time_offset);
    } else {
        logD(RFX_LOG_TAG,  "handleGetNetworkTimeResponse error!");
    }

    if (response->getError() == RIL_E_SUCCESS && entry.response == EMBMS_GENERAL_SUCCESS) {
        asprintf(&responseStr, "%%MBMSCMD:%llu\nOK\n", (long long unsigned int) entry.milli_sec);
    } else {
        asprintf(&responseStr, EMBMS_ERROR);
    }

    responseToAtci(response, responseStr);
    free(responseStr);
}

void RpEmbmsAtController::handleTriggerCellInfoNotifyResponse(const sp<RfxMessage>& response) {
    int id = response->getId();
    // Just ignore, nothing to do
    logD(RFX_LOG_TAG, "handleTriggerCellInfoNotifyResponse, id:%d", id);
}

void RpEmbmsAtController::handleCellInfoUrc(const sp<RfxMessage>& response) {
    int32_t entry_count = 0;
    int32_t cell_id = 0;
    int32_t status = 0;
    int32_t plmn = 0;
    char ecgi[16];  // 3+3+7 digits
    char ecgi_mask[16];
    char* cell_id_mask = NULL;
    Parcel* p = response->getParcel();
    // dispatchInts
    p->readInt32(&entry_count);
    if (entry_count != 3) {
        logD(RFX_LOG_TAG, " Error!! entry_count %d!=3", entry_count);
    }

    p->readInt32(&cell_id);
    p->readInt32(&status);
    p->readInt32(&plmn);
    cell_id_mask = RpEmbmsUtils::addLogMask(cell_id);
    logD(RFX_LOG_TAG, "handleCellInfoUrc cell_id %s, status %d, plmn %d",
        cell_id_mask, status,plmn);

    // Register with LTE network. Change flight mode to false.
    if (status == 1) {
        mIsFlightOn = false;
    }

    char *responseStr = NULL;
    char *responseStr_mask = NULL;
    asprintf(&responseStr, "+CEREG:%d\n", status);
    logD(RFX_LOG_TAG, "%s", responseStr);
    urcToAtci(response, responseStr, NULL);
    free(responseStr);

    // for RJIL old middleware version
    if (!RpEmbmsUtils::isRjilSupport()) {
        // %MBMSEV:4,<ECGI>
        sprintf(ecgi, "%05d%07X", plmn, cell_id);
        sprintf(ecgi_mask, "%05d%s", plmn, cell_id_mask);
        asprintf(&responseStr, "%%MBMSEV:4,%s\n", ecgi);
        asprintf(&responseStr_mask, "%%MBMSEV:4,%s\n", ecgi_mask);
        urcToAtci(response, responseStr, responseStr_mask);
        free(responseStr);
        free(responseStr_mask);
    }

    if (cell_id_mask) {
        cell_id_mask = NULL;
        free(cell_id_mask);
    }
}

void RpEmbmsAtController::handleCoverageUrc(const sp<RfxMessage>& response) {
    int32_t entry_count = 0;
    int32_t srv_status = 0;
    int     result_status = 0;
    Parcel* p = response->getParcel();
    // dispatchInts
    p->readInt32(&entry_count);
    if (entry_count != 1) {
        logD(RFX_LOG_TAG, " Error!! entry_count %d!=1", entry_count);
    }

    p->readInt32(&srv_status);
    result_status = getCoverageState(srv_status);

    char *responseStr = NULL;
    // for RJIL old middleware version
    if (RpEmbmsUtils::isRjilSupport()) {
        asprintf(&responseStr, "%%MBMSEV:%d\n", result_status);
    } else {
        asprintf(&responseStr, "%%MBMSEV:%d,0\n", result_status);
    }
    logD(RFX_LOG_TAG, "%s", responseStr);
    urcToAtci(response, responseStr, NULL);
    free(responseStr);
}

void RpEmbmsAtController::handleSessionListUrc(const sp<RfxMessage>& response) {
    uint32_t tmgi_info_count = 0;
    char*    tmp_tmgi = NULL;
    char*    tmp_session_id = NULL;
    int32_t trans_id = -1;
    int32_t tmp = 0;
    // %MBMSEV: 96 -> Active TMGIs (all currently activated TMGIs)
    // %MBMSEV: 97 -> Available TMGIs (all currently activated TMGIs + Available TMGIs that can be activated)
    // %MBMSEV:[96,97],count, TMGI1, TMGI2..
    String8 responseStr;

    if (mIsFlightOn) {
        logI(RFX_LOG_TAG, "skip due to flight mode");
        return;
    }

    if (response->getId()== RIL_LOCAL_GSM_UNSOL_EMBMS_ACTIVE_SESSION) {
        responseStr = String8("%MBMSEV:96");
    } else {  // RFX_MSG_URC_EMBMS_AVAILABLE_SESSION
        responseStr = String8("%MBMSEV:97");
    }

    Parcel* p = response->getParcel();

    // dispatch RIL_EMBMS_LocalSessionNotify
    p->readInt32(&tmp);
    tmgi_info_count = tmp;
    responseStr.append(String8::format(",%d", tmgi_info_count));

    logI(RFX_LOG_TAG, "handleSessionListUrc, tmgi_info_count:%d", tmgi_info_count);
    for (uint32_t i = 0; i < tmgi_info_count; i++) {
        tmp_tmgi = RpEmbmsUtils::strdupReadString(p);
        logD(RFX_LOG_TAG, "handleSessionListUrc, tmgi[%d] = [%s]", i, tmp_tmgi);
        responseStr.append(String8::format(",%s", tmp_tmgi));
        free(tmp_tmgi);

        tmp_session_id = RpEmbmsUtils::strdupReadString(p);
        free(tmp_session_id);
    }


    if (RpEmbmsUtils::isRjilSupport()) { //version 1.1.1x~1.1.20c
        responseStr = String8("%MBMSEV:0");
        logD(RFX_LOG_TAG, "%s", responseStr.string());
        urcToAtci(response, responseStr.string(), NULL);
    } else if (mSdkVersion < 10901){
        responseStr = String8("%MBMSEV:0,0");
        logD(RFX_LOG_TAG, "%s", responseStr.string());
        urcToAtci(response, responseStr.string(), NULL);
    } else {//mSdkVersion >= 10901
        logD(RFX_LOG_TAG, "%s", responseStr.string());
        urcToAtci(response, responseStr.string(), NULL);

        responseStr = String8("%MBMSEV:0,0");
        logD(RFX_LOG_TAG, "%s", responseStr.string());
        urcToAtci(response, responseStr.string(), NULL);
    }


}

void RpEmbmsAtController::handleActiveSessionStatus(const sp<RfxMessage>& response) {
    int32_t tmgi_info_count;
    bool    isActive = false;
    Parcel* p = response->getParcel();
    p->readInt32(&tmgi_info_count);

    isActive = (tmgi_info_count > 0) ? true : false;

    if (mIsActiveSession != isActive) {
        mIsActiveSession = isActive;

        // responseInts
        sp<RfxMessage> newMsg = RfxMessage::obtainUrc(response->getSlotId(),
                                        RIL_UNSOL_EMBMS_SESSION_STATUS,
                                        response, false);
        Parcel* np = newMsg->getParcel();
        int32_t reply_count = 1;
        np->writeInt32(reply_count);
        np->writeInt32((mIsActiveSession) ? 1 : 0);
        responseToRilj(newMsg);
    }
}

void RpEmbmsAtController::handleOosUrc(const sp<RfxMessage>& response) {
    uint32_t reason;
    uint32_t tmgi_info_count;
    char*    tmp_tmgi;
    int32_t trans_id = -1;
    int32_t tmp = 0;
    // %MBMSEV:95,count, TMGI1, TMGI2..
    String8 responseStr("%MBMSEV:95");

    Parcel* p = response->getParcel();

    // dispatch RIL_EMBMS_LocalOosNotify
    p->readInt32(&tmp);
    reason = tmp;

    p->readInt32(&tmp);
    tmgi_info_count = tmp;
    responseStr.append(String8::format(",%d", tmgi_info_count));

    logI(RFX_LOG_TAG, "handleOosUrc, reason:%d, tmgi_info_count%d", reason, tmgi_info_count);
    for (uint32_t i = 0; i < tmgi_info_count; i++) {
        tmp_tmgi = RpEmbmsUtils::strdupReadString(p);
        logD(RFX_LOG_TAG, "handleOosUrc, tmgi[%d] = [%s]", i, tmp_tmgi);
        responseStr.append(String8::format(",%s", tmp_tmgi));
    }

    logD(RFX_LOG_TAG, "%s", responseStr.string());
    urcToAtci(response, responseStr.string(), NULL);
}

void RpEmbmsAtController::handleSaiUrc(const sp<RfxMessage>& response) {
    int32_t tmp;
    RIL_EMBMS_LocalSaiNotify input_data;
    memset(&input_data, 0, sizeof(input_data));
    RIL_EMBMS_SaiNotify output_data;
    memset(&output_data, 0, sizeof(output_data));
    // %MBMSEV:98,<SAI_COUNT>,[<csai1>, <csai2>, <csai3>,!K], <INTER_FREQUENCY_COUNT>,
    // [<Frequency_1>,<SAI_COUNT_1>, [<nsai11>, <nsai12>, <nsai13>,!K]],
    // [<Frequency_n>,<SAI_COUNT_n>, [<nsain1>, <nsain2>, <nsain3>,!K]]
    String8 responseStr("%MBMSEV:98");

    Parcel* p = response->getParcel();
    // cf_total
    p->readInt32(&tmp);
    input_data.cf_total = tmp;

    // curFreqData
    for (uint32_t i = 0; i < input_data.cf_total; i++) {
        p->readInt32(&tmp);
        input_data.curFreqData[i] = tmp;
    }

    // csai_total
    p->readInt32(&tmp);
    input_data.csai_total = tmp;

    // curSaiData
    for (uint32_t i = 0; i < input_data.csai_total; i++) {
        p->readInt32(&tmp);
        input_data.curSaiData[i] = tmp;
    }

    // nf_total
    p->readInt32(&tmp);
    input_data.nf_total = tmp;

    // neiFreqData
    for (uint32_t i = 0; i < input_data.nf_total; i++) {
        p->readInt32(&tmp);
        input_data.neiFreqData[i] = tmp;
    }

    // nsai_count_per_group
    for (uint32_t i = 0; i < input_data.nf_total; i++) {
        p->readInt32(&tmp);
        input_data.nsai_count_per_group[i] = tmp;
    }

    // nsai_total
    p->readInt32(&tmp);

    // neiSaiData
    input_data.nsai_total = tmp;
    for (uint32_t i = 0; i < input_data.nsai_total; i++) {
        p->readInt32(&tmp);
        input_data.neiSaiData[i] = tmp;
    }

    // Fill in response data

    // trans_id
    output_data.trans_id = -1;
    // cf_total
    output_data.cf_total = input_data.cf_total;
    // curFreqData
    for (uint32_t i = 0; i < input_data.cf_total; i++) {
        output_data.curFreqData[i] = input_data.curFreqData[i];
    }
    // csai_count_per_group
    for (uint32_t i = 0; i < input_data.cf_total; i++) {
        output_data.csai_count_per_group[i] = 0;
    }
    // csai_total
    output_data.csai_total = input_data.csai_total;
    // curSaiData
    for (uint32_t i = 0; i < input_data.csai_total; i++) {
        output_data.curSaiData[i] = input_data.curSaiData[i];
    }
    // nf_total
    output_data.nf_total = input_data.nf_total;
    // neiFreqData
    for (uint32_t i = 0; i < input_data.nf_total; i++) {
        output_data.neiFreqData[i] = input_data.neiFreqData[i];
    }
    // nsai_count_per_group
    for (uint32_t i = 0; i < input_data.nf_total; i++) {
        output_data.nsai_count_per_group[i] = input_data.nsai_count_per_group[i];
    }
    // nsai_total
    output_data.nsai_total = input_data.nsai_total;
    // neiSaiData
    for (uint32_t i = 0; i < input_data.nsai_total; i++) {
        output_data.neiSaiData[i] = input_data.neiSaiData[i];
    }

    // SAI_COUNT
    responseStr.append(String8::format(",%d", output_data.csai_total));

    // csai1
    for (uint32_t i = 0; i < output_data.csai_total; i++) {
        responseStr.append(String8::format(",%d", output_data.curSaiData[i]));
    }
    // INTER_FREQUENCY_COUNT
    responseStr.append(String8::format(",%d", output_data.nf_total));

    int tail_index = 0, j;
    for (uint32_t i = 0; i < output_data.nf_total; i++) {
        // <Frequency_1>, <SAI_COUNT_1>
        responseStr.append(String8::format(",%d,%d", output_data.neiFreqData[i],
                                output_data.nsai_count_per_group[i]));

        // <nsai11>
        for (j = 0; j < (int) output_data.nsai_count_per_group[i]; j++) {
            responseStr.append(String8::format(",%d", output_data.neiSaiData[tail_index+j]));
        }
        tail_index += output_data.nsai_count_per_group[i];
    }

    logD(RFX_LOG_TAG, "%s", responseStr.string());
    urcToAtci(response, responseStr.string(), NULL);
}

int RpEmbmsAtController::getCoverageState(int srv_status) {
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

bool RpEmbmsAtController::isEmbmsSupported() {
    char prop[PROPERTY_VALUE_MAX] = {0};
    bool isEmbmsSupported = false;
    property_get("persist.vendor.radio.embms.support", prop, "-1");
    if (!strcmp(prop, "1")) {
        return true;
    } else if (!strcmp(prop, "0")) {
        return false;
    }

    property_get("ro.vendor.mtk_embms_support", prop, "0");
    if (!strcmp(prop, "1")) {
        isEmbmsSupported = true;
    } else {
        isEmbmsSupported = false;
    }

    return isEmbmsSupported;
}

void RpEmbmsAtController::onMainCapabilityChanged(RfxStatusKeyEnum key,
    RfxVariant old_value, RfxVariant new_value) {
    char *responseStr = (char*) EMBMS_FLIGTH_MODE;

    EMBMS_UNUSED(key);

    int oldType = old_value.asInt();
    int newType = new_value.asInt();

    logD(RFX_LOG_TAG, "onMainCapabilityChanged[%d] embms:%d, cap %d->%d", getSlotId() ,mEmbmsEnabled
        ,oldType, newType);

    if (oldType != newType && mEmbmsEnabled) {
        logD(RFX_LOG_TAG, "capabilty change %d:%d", oldType, newType);
        sp<RfxMessage> urcToRilj = RfxMessage::obtainUrc(this->getSlotId(),
                RIL_UNSOL_EMBMS_AT_INFO);
        urcToAtci(urcToRilj, responseStr, NULL);

        mIsFlightOn = true;
        mEmbmsEnabled = false;
        //  clear active info
        if (mSessionInfoList != NULL && !mSessionInfoList->isEmpty()) {
            RpEmbmsUtils::freeSessionList(mSessionInfoList);
        }
    }
}

void RpEmbmsAtController::onDefaultDataChanged(RfxStatusKeyEnum key,
    RfxVariant old_value, RfxVariant new_value) {
    char *responseStr = (char*) EMBMS_FLIGTH_MODE;

    EMBMS_UNUSED(key);
    int oldSim = old_value.asInt();
    int newSim = new_value.asInt();
    logD(RFX_LOG_TAG, "onDefaultDataChanged[%d] embms:%d, cap %d->%d", getSlotId() ,mEmbmsEnabled
        ,oldSim, newSim);

    if (oldSim != newSim) {
        if (mEmbmsEnabled) {
            logD(RFX_LOG_TAG, "default sim change %d:%d", oldSim, newSim);
            sp<RfxMessage> urcToRilj = RfxMessage::obtainUrc(this->getSlotId(),
                    RIL_UNSOL_EMBMS_AT_INFO);
            urcToAtci(urcToRilj, responseStr, NULL);
            mIsFlightOn = true;

            // Do not response silent disable eMBMS to middleware
            // The original default sim is mEmbmsEnabled and need send EMBMS_DISABLE
            sp<RfxMessage> newMsg = RfxMessage::obtainRequest(getSlotId(), RADIO_TECH_GROUP_GSM,
                RIL_LOCAL_REQUEST_EMBMS_DISABLE);
            Parcel* newParcel = newMsg->getParcel();
            int count = 2;
            int trans_id = -1;

            newParcel->writeInt32(count);
            newParcel->writeInt32(trans_id);
            newParcel->writeInt32(EMBMS_COMMAND_ATCI);
            requestToRild(newMsg);
        }
        if (newSim == getSlotId()) {
            sp<RfxMessage> newMsg = RfxMessage::obtainRequest(getSlotId(), RADIO_TECH_GROUP_GSM,
                RIL_LOCAL_REQUEST_EMBMS_TRIGGER_CELL_INFO_NOTIFY);
            requestToRild(newMsg);
        }
    }
}
