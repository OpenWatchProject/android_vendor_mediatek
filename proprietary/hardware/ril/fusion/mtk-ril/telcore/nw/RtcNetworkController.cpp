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
#include "RtcNetworkController.h"

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/

#define NW_CTRL_TAG "RtcNwCtrl"

RFX_IMPLEMENT_CLASS("RtcNetworkController", RtcNetworkController, RfxController);

RtcNetworkController::RtcNetworkController() :
    mNetworkScanOngoing(false),
    mForceStopNetworkScan(false),
    mNwScanMessage(NULL) {
}

RtcNetworkController::~RtcNetworkController() {
}

void RtcNetworkController::onInit() {
    // Required: invoke super class implementation
    RfxController::onInit();
   logV(NW_CTRL_TAG, "[onInit]");
    const int request_id_list[] = {
        RFX_MSG_REQUEST_SET_BAND_MODE,
        RFX_MSG_REQUEST_GET_NEIGHBORING_CELL_IDS,
        RFX_MSG_REQUEST_SET_UNSOL_CELL_INFO_LIST_RATE,
        RFX_MSG_REQUEST_VOICE_REGISTRATION_STATE,
        RFX_MSG_REQUEST_DATA_REGISTRATION_STATE,
        RFX_MSG_REQUEST_OPERATOR,
        RFX_MSG_REQUEST_QUERY_NETWORK_SELECTION_MODE,
        RFX_MSG_REQUEST_SCREEN_STATE,
        RFX_MSG_REQUEST_SET_UNSOLICITED_RESPONSE_FILTER,
        RFX_MSG_RIL_REQUEST_START_NETWORK_SCAN,
        RFX_MSG_RIL_REQUEST_STOP_NETWORK_SCAN,
        RFX_MSG_REQUEST_SET_NETWORK_SELECTION_MANUAL,
        RFX_MSG_REQUEST_SET_NETWORK_SELECTION_MANUAL_WITH_ACT,
        RFX_MSG_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC,
        RFX_MSG_REQUEST_SET_SERVICE_STATE,
        RFX_MSG_REQUEST_CDMA_SET_ROAMING_PREFERENCE,
        RFX_MSG_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE,
        RFX_MSG_REQUEST_SET_LTE_RELEASE_VERSION,
        RFX_MSG_REQUEST_GET_LTE_RELEASE_VERSION,
        RFX_MSG_REQUEST_QUERY_AVAILABLE_NETWORKS,
        RFX_MSG_REQUEST_QUERY_AVAILABLE_NETWORKS_WITH_ACT,
        RFX_MSG_REQUEST_DATA_REGISTRATION_STATE_EXT,
        RFX_MSG_REQUEST_GET_EHRPD_INFO_FOR_IMS,
        RFX_MSG_REQUEST_ABORT_QUERY_AVAILABLE_NETWORKS,
        RFX_MSG_REQUEST_REGISTER_CELLULAR_QUALITY_REPORT
    };

    const int urc_id_list[] = {
        RFX_MSG_URC_NETWORK_SCAN_RESULT
    };

    // register request & URC id list
    // NOTE. one id can only be registered by one controller
    registerToHandleRequest(request_id_list, sizeof(request_id_list)/sizeof(const int), DEFAULT);
    registerToHandleUrc(urc_id_list, sizeof(urc_id_list)/sizeof(const int));
    getNonSlotScopeStatusManager()->registerStatusChanged(RFX_STATUS_KEY_MODEM_RESET,
            RfxStatusChangeCallback(this, &RtcNetworkController::onHandleModemReset));
}

bool RtcNetworkController::onHandleRequest(const sp<RfxMessage>& message) {
    // logD(NW_CTRL_TAG, "[onHandleRequest] %s", RFX_ID_TO_STR(message->getId()));
    int msg_id = message->getId();

    switch (msg_id) {
        case RFX_MSG_REQUEST_QUERY_AVAILABLE_NETWORKS:
        case RFX_MSG_REQUEST_QUERY_AVAILABLE_NETWORKS_WITH_ACT:
            logV(NW_CTRL_TAG, "mNetworkScanOngoing:%d isAPInCall:%d",
                    mNetworkScanOngoing, isAPInCall());
            if (isAPInCall() == true) {
                sp<RfxMessage> resToRilj = RfxMessage::obtainResponse(RIL_E_MODEM_ERR, message);
                responseToRilj(resToRilj);
                return true;
            }
            if (mNetworkScanOngoing == true) {
                mNwScanMessage = message;
                mForceStopNetworkScan = true;
                sp<RfxMessage> reqToRild = RfxMessage::obtainRequest(m_slot_id,
                        RFX_MSG_REQUEST_ABORT_QUERY_AVAILABLE_NETWORKS, RfxVoidData());
                requestToMcl(reqToRild);
            } else {
                mNetworkScanOngoing = true;
                requestToMcl(message);
            }
            break;
        case RFX_MSG_RIL_REQUEST_START_NETWORK_SCAN:
            logV(NW_CTRL_TAG, "mNetworkScanOngoing:%d isAPInCall:%d",
                    mNetworkScanOngoing, isAPInCall());
            // VTS case.
            if (mNetworkScanOngoing == true) {
                mNwScanMessage = message;
                mForceStopNetworkScan = true;
                sp<RfxMessage> reqToRild = RfxMessage::obtainRequest(m_slot_id,
                        RFX_MSG_RIL_REQUEST_STOP_NETWORK_SCAN, RfxVoidData());
                requestToMcl(reqToRild);
            } else {
                mNetworkScanOngoing = true;
                requestToMcl(message);
            }
            break;
        case RFX_MSG_RIL_REQUEST_STOP_NETWORK_SCAN:
            requestToMcl(message);
            break;
        case RFX_MSG_REQUEST_SET_NETWORK_SELECTION_MANUAL:
        case RFX_MSG_REQUEST_SET_NETWORK_SELECTION_MANUAL_WITH_ACT:
        case RFX_MSG_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC:
            logV(NW_CTRL_TAG, "mNetworkScanOngoing:%d", mNetworkScanOngoing);
            if (mNetworkScanOngoing == true) {
                mNwScanMessage = message;
                RtcRatSwitchController* nwRatController = (RtcRatSwitchController *)findController
                        (RFX_OBJ_CLASS_INFO(RtcRatSwitchController));
                sp<RfxAction> action = new RfxAction0(this,
                        &RtcNetworkController::onLocalAbortAvailableNetworkDone);
                nwRatController->setLocalAbortAvailableNetwork(action);
            } else {
                requestToMcl(message);
            }
            break;
        default:
            requestToMcl(message);
            break;
    }
    return true;
}

bool RtcNetworkController::onHandleResponse(const sp<RfxMessage>& response) {
    int msg_id = response->getId();

    switch (msg_id) {
        case RFX_MSG_RIL_REQUEST_START_NETWORK_SCAN:
            responseToRilj(response);
            if (response->getError() != RIL_E_SUCCESS) {
                mNetworkScanOngoing = false;
            }
            break;
        case RFX_MSG_REQUEST_QUERY_AVAILABLE_NETWORKS:
        case RFX_MSG_REQUEST_QUERY_AVAILABLE_NETWORKS_WITH_ACT:
            mNetworkScanOngoing = false;
            responseToRilj(response);
            break;

        case RFX_MSG_REQUEST_ABORT_QUERY_AVAILABLE_NETWORKS:
        case RFX_MSG_RIL_REQUEST_STOP_NETWORK_SCAN:
            logV(NW_CTRL_TAG, "mNetworkScanOngoing:%d mForceStopNetworkScan:%d",
                    mNetworkScanOngoing, mForceStopNetworkScan);
            if (mForceStopNetworkScan == true) {
                if (mNwScanMessage != NULL) {
                    requestToMcl(mNwScanMessage);
                    mNwScanMessage = NULL;
                } else {
                    mNetworkScanOngoing = false;
                }
                mForceStopNetworkScan = false;
            } else {
                mNetworkScanOngoing = false;
                responseToRilj(response);
            }
            break;
        default:
            responseToRilj(response);
            break;
    }
    return true;
}

bool RtcNetworkController::onHandleUrc(const sp<RfxMessage>& message) {
    int msg_id = message->getId();

    if (msg_id == RFX_MSG_URC_NETWORK_SCAN_RESULT) {
        RfxNetworkScanResultData *ptr = (RfxNetworkScanResultData *)message->getData();
        RIL_NetworkScanResult *data = (RIL_NetworkScanResult *)ptr->getData();
        logV(NW_CTRL_TAG, "mNetworkScanOngoing:%d data->status:%d",
                mNetworkScanOngoing, data->status);
        if (data->status == COMPLETE) { // for now we don't support PARTIAL.
            mNetworkScanOngoing = false;
        }
    }
    responseToRilj(message);
    return true;
}

bool RtcNetworkController::onCheckIfRejectMessage(const sp<RfxMessage>& message,
        bool isModemPowerOff,int radioState) {

    /* Reject the request in radio unavailable or modem off */
    if ((radioState == (int)RADIO_STATE_UNAVAILABLE &&
            RFX_MSG_REQUEST_SET_UNSOLICITED_RESPONSE_FILTER != message->getId()) ||
            isModemPowerOff == true) {
        logD(NW_CTRL_TAG, "onCheckIfRejectMessage, id = %d, isModemPowerOff = %d, rdioState = %d",
                message->getId(), isModemPowerOff, radioState);
        return true;
    }

    return false;
}

bool RtcNetworkController::onPreviewMessage(const sp<RfxMessage>& message) {
    if (getStatusManager()->getBoolValue(RFX_STATUS_KEY_IS_RAT_MODE_SWITCHING) == true &&
            (message->getId() == RFX_MSG_REQUEST_QUERY_AVAILABLE_NETWORKS ||
            message->getId() == RFX_MSG_REQUEST_SET_NETWORK_SELECTION_MANUAL_WITH_ACT)) {
        logV(NW_CTRL_TAG, "onPreviewMessage, put %s into pending list",
                RFX_ID_TO_STR(message->getId()));
        return false;
    } else {
        return true;
    }
}

bool RtcNetworkController::onCheckIfResumeMessage(const sp<RfxMessage>& message) {
    if (getStatusManager()->getBoolValue(RFX_STATUS_KEY_IS_RAT_MODE_SWITCHING) == false &&
            (message->getId() == RFX_MSG_REQUEST_QUERY_AVAILABLE_NETWORKS ||
            message->getId() == RFX_MSG_REQUEST_SET_NETWORK_SELECTION_MANUAL_WITH_ACT)) {
        logV(NW_CTRL_TAG, "resume the request %s",
                RFX_ID_TO_STR(message->getId()));
        return true;
    } else {
        return false;
    }
}

void RtcNetworkController::onHandleModemReset(RfxStatusKeyEnum key, RfxVariant old_value,
        RfxVariant value) {
    RFX_UNUSED(key);
    bool old_mode = old_value.asBool();
    bool new_mode = value.asBool();

    logE(NW_CTRL_TAG, "onHandleModemReset:old_mode:%d, new_mode:%d, mNetworkScanOngoing:%d",
            old_mode, new_mode, mNetworkScanOngoing);

    if (new_mode == true && mNetworkScanOngoing == true) { // send plmn list complete when TRM.
        sp<RfxMessage> urcToRilj;
        RIL_NetworkScanResult* resp = (RIL_NetworkScanResult*) calloc(1, sizeof(RIL_NetworkScanResult));
        RFX_ASSERT(resp != NULL);
        memset(resp, 0, sizeof(RIL_NetworkScanResult));
        resp->status = COMPLETE; // for now we don't support PARTIAL.
        urcToRilj = RfxMessage::obtainUrc(m_slot_id, RFX_MSG_URC_NETWORK_SCAN_RESULT,
                RfxNetworkScanResultData((void*) resp, sizeof(RIL_NetworkScanResult)));
        // response to TeleCore
        responseToRilj(urcToRilj);
        free(resp);
        mNetworkScanOngoing = false;
    }
}

void RtcNetworkController::onLocalAbortAvailableNetworkDone() {
    logV(NW_CTRL_TAG, "Local abort done");
    if (mNwScanMessage != NULL) {
        requestToMcl(mNwScanMessage);
    }
    mNwScanMessage = NULL;
}

bool RtcNetworkController::isAPInCall() {
    bool ret = false;

    for (int slotId = RFX_SLOT_ID_0; slotId < RFX_SLOT_COUNT; slotId++) {
        if (getStatusManager(slotId)->getIntValue(RFX_STATUS_KEY_AP_VOICE_CALL_COUNT, 0) > 0) {
            ret = true;
        }
    }
    return ret;
}
