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
#include "RtcCommSimController.h"
#include "phb/RtcPhbSimIoController.h"
#include <cstring>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <telephony/mtk_ril.h>
#include "RfxVoidData.h"
#include "RfxStringsData.h"
#include "RfxMainThread.h"
#include "RfxRilUtils.h"
#include "RfxStringData.h"
#include "RfxSimIoRspData.h"
#include "rfx_properties.h"
#include <mtkconfigutils.h>
#include "RfxIntsData.h"

using ::android::String8;
using namespace std;

RFX_IMPLEMENT_CLASS("RtcCommSimController", RtcCommSimController, RfxController);

RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxIntsData, \
        RFX_MSG_REQUEST_GET_CURRENT_UICC_CARD_PROVISIONING_STATUS);

/*****************************************************************************
 * Class RtcCommSimController
 *****************************************************************************/
RtcCommSimController::RtcCommSimController() {
        setTag(String8("RtcCommSimCtrl"));
}


RtcCommSimController::~RtcCommSimController() {
}

void RtcCommSimController::onInit() {
    // Required: invoke super class implementation
    RfxController::onInit();

    // register request & URC id list for radio state check
    const int request_id_list[] = {
        RFX_MSG_REQUEST_GET_SIM_STATUS,
        RFX_MSG_REQUEST_ENTER_SIM_PIN,
        RFX_MSG_REQUEST_ENTER_SIM_PUK,
        RFX_MSG_REQUEST_ENTER_SIM_PIN2,
        RFX_MSG_REQUEST_ENTER_SIM_PUK2,
        RFX_MSG_REQUEST_CHANGE_SIM_PIN,
        RFX_MSG_REQUEST_CHANGE_SIM_PIN2,
        RFX_MSG_REQUEST_SIM_ENTER_NETWORK_DEPERSONALIZATION,
        RFX_MSG_REQUEST_SIM_ENTER_DEPERSONALIZATION,
        RFX_MSG_REQUEST_SIM_IO,
        RFX_MSG_REQUEST_SIM_AUTHENTICATION,
        RFX_MSG_REQUEST_ISIM_AUTHENTICATION,
        RFX_MSG_REQUEST_GENERAL_SIM_AUTH,
        RFX_MSG_REQUEST_SIM_OPEN_CHANNEL,
        RFX_MSG_REQUEST_SIM_CLOSE_CHANNEL,
        RFX_MSG_REQUEST_SIM_TRANSMIT_APDU_BASIC,
        RFX_MSG_REQUEST_SIM_TRANSMIT_APDU_CHANNEL,
        RFX_MSG_REQUEST_SIM_GET_ATR,
        RFX_MSG_REQUEST_SIM_GET_ICCID,
        RFX_MSG_REQUEST_SIM_SAP_CONNECT,
        RFX_MSG_REQUEST_SIM_SAP_DISCONNECT,
        RFX_MSG_REQUEST_SIM_SAP_APDU,
        RFX_MSG_REQUEST_SIM_SAP_TRANSFER_ATR,
        RFX_MSG_REQUEST_SIM_SAP_POWER,
        RFX_MSG_REQUEST_SIM_SAP_RESET_SIM,
        RFX_MSG_REQUEST_SIM_SAP_SET_TRANSFER_PROTOCOL,
        RFX_MSG_REQUEST_SIM_SAP_ERROR_RESP,
        RFX_MSG_REQUEST_GET_IMSI,
        RFX_MSG_REQUEST_QUERY_FACILITY_LOCK,
        RFX_MSG_REQUEST_SET_FACILITY_LOCK,
        RFX_MSG_REQUEST_SIM_SET_SIM_NETWORK_LOCK,
        RFX_MSG_REQUEST_SIM_QUERY_SIM_NETWORK_LOCK,
        RFX_MSG_REQUEST_SIM_VSIM_NOTIFICATION,
        RFX_MSG_REQUEST_SIM_VSIM_OPERATION,
        RFX_MSG_REQUEST_CDMA_SUBSCRIPTION,
        RFX_MSG_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE,
        RFX_MSG_REQUEST_SET_SIM_CARD_POWER,
        RFX_MSG_REQUEST_SET_SIM_POWER,
        RFX_MSG_REQUEST_GET_SUBLOCK_MODEM_STATUS,
        RFX_MSG_REQUEST_UPDATE_SUBLOCK_SETTINGS,
        RFX_MSG_REQUEST_SIM_ENTER_DEVICE_NETWORK_DEPERSONALIZATION,
        RFX_MSG_REQUEST_SET_ALLOWED_CARRIERS,
        RFX_MSG_REQUEST_GET_ALLOWED_CARRIERS,
        RFX_MSG_REQUEST_ACTIVATE_UICC_CARD,
        RFX_MSG_REQUEST_DEACTIVATE_UICC_CARD,
        RFX_MSG_REQUEST_GET_CURRENT_UICC_CARD_PROVISIONING_STATUS,
        RFX_MSG_REQUEST_SML_RSU_REQUEST,
    };

    const int urc_id_list[] = {
        RFX_MSG_URC_SIM_SLOT_LOCK_POLICY_NOTIFY,
    };

    for (int i = 0; i < SIM_APP_COUNT; i++) {
        for (int j = 0; j < SIM_CACHED_FILE_COUNT; j++) {
            memset(&mIoResponse[i][j], 0, sizeof(RIL_SIM_IO_Cache_Response));
        }
    }

    // NOTE. one id can only be registered by one controller
    registerToHandleRequest(request_id_list, sizeof(request_id_list)/sizeof(const int));
    registerToHandleUrc(urc_id_list, sizeof(urc_id_list)/sizeof(const int));

    // register callbacks to get required information
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_RADIO_STATE,
            RfxStatusChangeCallback(this, &RtcCommSimController::onRadioStateChanged));

    // register callbacks to re-report
    getStatusManager()->registerStatusChanged(RFX_STATUS_CONNECTION_STATE,
            RfxStatusChangeCallback(this, &RtcCommSimController::onConnectionStateChanged));

    // register callbacks to mode switch status
    getNonSlotScopeStatusManager()->registerStatusChanged(RFX_STATUS_KEY_MODESWITCH_FINISHED,
            RfxStatusChangeCallback(this, &RtcCommSimController::onModeSwitchFinished));

    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_GSM_CACHE_FCP,
            RfxStatusChangeCallback(this, &RtcCommSimController::onSimFileChanged));
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_GSM_CACHE_BINARY,
            RfxStatusChangeCallback(this, &RtcCommSimController::onSimFileChanged));

    // register callbacks when SIM reset happens.
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_UICC_RESET_APPLICATION,
            RfxStatusChangeCallback(this, &RtcCommSimController::onSimResetChanged));
}

void RtcCommSimController::onRadioStateChanged(RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    int oldState = -1, newState = -1;

    RFX_UNUSED(key);
    oldState = old_value.asInt();
    newState = value.asInt();

    logD(mTag, "onRadioStateChanged (%d, %d) (slot %d)", oldState, newState, getSlotId());

    if (newState == RADIO_STATE_UNAVAILABLE) {
        // Modem SIM task is not ready because radio is not available
        getStatusManager()->setBoolValue(RFX_STATUS_KEY_MODEM_SIM_TASK_READY, false, true);
        getStatusManager()->setIntValue(RFX_STATUS_KEY_CDMA3G_SWITCH_CARD, -1);
    }
}

void RtcCommSimController::onConnectionStateChanged(RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    int oldState = -1, newState = -1;

    RFX_UNUSED(key);
    oldState = old_value.asBool();
    newState = value.asBool();

    logD(mTag, "onConnectionStateChanged (%d, %d) (slot %d)", oldState, newState, getSlotId());

    if (!oldState && newState) {
        // RILJ connects to RILD and SML SLOT URC has reported
        if (mCacheSmlMsg != NULL) {
            int* smlData = (int*)(mCacheSmlMsg->getData()->getData());
            int smlDataLength = mCacheSmlMsg->getData()->getDataLength();
            logD(mTag, "onConnectionStateChanged %s %d %d %d %d %d (slot %d)",
                    mCacheSmlMsg->toString().string(), smlDataLength, smlData[0], smlData[1],
                    smlData[2], smlData[3], getSlotId());
            // Here don't remove cache to avoid the timing issue
            responseToRilj(mCacheSmlMsg);
        } else {
            logD(mTag, "onConnectionStateChanged mCacheSmlMsg == NULL");
        }
    }
}

void RtcCommSimController::onModeSwitchFinished(RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {

    RFX_UNUSED(key);

    logD(mTag, "onModeSwitchFinished (%d, %d) (slot %d)", old_value.asInt(), value.asInt(),
            getSlotId());

    sp<RfxMessage> message = RfxMessage::obtainUrc(getSlotId(),
            RFX_MSG_URC_RESPONSE_SIM_STATUS_CHANGED, RfxVoidData());
    responseToRilj(message);
}

void RtcCommSimController::onSimFileChanged(RfxStatusKeyEnum key, RfxVariant old_value,
        RfxVariant value) {
    char* response = (char *)strdup(value.asString8().string());
    char* tok = NULL;
    int app = -1;
    int fileNum = -1;
    RFX_UNUSED(old_value);

    // Clear the cache context if key value is cleared.
    if (response == NULL || strlen(response) == 0) {
        for (int i = 0; i < SIM_APP_COUNT; i++) {
            for (int j = 0; j < SIM_CACHED_FILE_COUNT; j++) {
                if (mIoResponse[i][j].path != NULL) {
                    free(mIoResponse[i][j].path);
                    mIoResponse[i][j].path = NULL;
                }

                if (mIoResponse[i][j].get_rsp.simResponse != NULL) {
                    free(mIoResponse[i][j].get_rsp.simResponse);
                    mIoResponse[i][j].get_rsp.simResponse = NULL;
                }

                if (mIoResponse[i][j].read_binary.simResponse != NULL) {
                    free(mIoResponse[i][j].read_binary.simResponse);
                    mIoResponse[i][j].read_binary.simResponse = NULL;
                }

                memset(&mIoResponse[i][j], 0, sizeof(RIL_SIM_IO_Cache_Response));
            }
        }
    } else {
        tok = strtok(response, ",");

        if (tok != NULL) {
            app = atoi(tok);

            tok = strtok(NULL, ",");
            fileNum = atoi(tok);

            tok = strtok(NULL, ",");
            for (int i = 0; (i < fileNum) && (tok != NULL); i++) {
                // Get the cached file index.
                int id = -1;
                for (int j = 0; j < SIM_CACHED_FILE_COUNT; j++) {
                    String8 path(RIL_SIM_CACHE_FILES[app][j].path);
                    path.append(String8::format("%X", RIL_SIM_CACHE_FILES[app][j].fileid));
                    String8 tokPath("3F00");
                    tokPath.append(String8::format("%s", tok));

                    if (strcmp(path.string(), tokPath.string()) == 0) {
                        id = j;
                        break;
                    }
                }

                tok = strtok(NULL, ",");
                // Cache the file information to mIoResponse.
                if (id != -1) {
                    mIoResponse[app][id].fileid = RIL_SIM_CACHE_FILES[app][id].fileid;

                    int len = strlen(RIL_SIM_CACHE_FILES[app][id].path);
                    if (mIoResponse[app][id].path == NULL) {
                        mIoResponse[app][id].path = (char*) malloc(len * sizeof(char) + 1);
                    }
                    strncpy(mIoResponse[app][id].path, RIL_SIM_CACHE_FILES[app][id].path, len);
                    mIoResponse[app][id].path[len] = '\0';

                    if (strlen(tok) >= 4) {
                        if (key == RFX_STATUS_KEY_GSM_CACHE_FCP) {
                            String8 swFcp(tok);

                            mIoResponse[app][id].get_rsp.sw1 = strtol(string(swFcp.string())
                                    .substr(0, 2).c_str(), NULL, 16);
                            mIoResponse[app][id].get_rsp.sw2 = strtol(string(swFcp.string())
                                    .substr(2, 2).c_str(), NULL, 16);

                            if (strlen(tok) > 4) {
                                if (mIoResponse[app][id].get_rsp.simResponse == NULL) {
                                    mIoResponse[app][id].get_rsp.simResponse = (char*) malloc(
                                            (strlen(tok) - 4) * sizeof(char) + 1);
                                }
                                strncpy(mIoResponse[app][id].get_rsp.simResponse, tok + 4,
                                        strlen(tok) - 4);
                                mIoResponse[app][id].get_rsp.simResponse[strlen(tok) - 4] = '\0';
                            } else {
                                mIoResponse[app][id].get_rsp.simResponse = NULL;
                            }

                            mIoResponse[app][id].get_rsp.valid = true;
                        } else if (key == RFX_STATUS_KEY_GSM_CACHE_BINARY) {
                            String8 swBinary(tok);
                            mIoResponse[app][id].read_binary.sw1 = strtol(string(swBinary.string())
                                    .substr(0, 2).c_str(), NULL, 16);
                            mIoResponse[app][id].read_binary.sw2 = strtol(string(swBinary.string())
                                    .substr(2, 2).c_str(), NULL, 16);
                            if (strlen(tok) > 4) {
                                if (mIoResponse[app][id].read_binary.simResponse == NULL) {
                                    mIoResponse[app][id].read_binary.simResponse = (char*) malloc(
                                           (strlen(tok) - 4) * sizeof(char) + 1);
                                }
                                strncpy(mIoResponse[app][id].read_binary.simResponse, tok + 4,
                                       strlen(tok) - 4);
                                mIoResponse[app][id].read_binary.simResponse[strlen(tok) - 4] =
                                       '\0';
                            } else {
                                mIoResponse[app][id].read_binary.simResponse = NULL;
                            }

                            mIoResponse[app][id].read_binary.valid = true;
                        } else {
                            logE(mTag, "onSimFileChanged key error key: %d (slot %d)", key,
                                   getSlotId());
                        }
                    } else {
                        logE(mTag, "onSimFileChanged sw error len: %zu (slot %d)",
                               strlen(tok), getSlotId());
                    }
                } else {
                    logE(mTag, "onSimFileChanged the file is not requested : %s (slot %d)",
                            tok, getSlotId());
                }

                tok = strtok(NULL, ",");
            }
        }
    }

    if (response != NULL) {
        free(response);
    }
}

void RtcCommSimController::onSimResetChanged(RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    logD(mTag, "onSimResetChanged (%s, %s) (slot %d)", (const char *)(old_value.asString8()),
            (const char *)(value.asString8()), getSlotId());

    String8 aid = value.asString8();

    // Clear SIM parameters just when card is an euicc card.
    if (getStatusManager()->getBoolValue(RFX_STATUS_KEY_IS_EUICC_CARD, false)) {
        // Clear GSM systemProperty and statusKey.
        if (aid.isEmpty() || (strncmp(aid.string(), "A0000000871002", 14) == 0)) {
            String8 gsmMccMnc("vendor.gsm.ril.uicc.mccmnc");
            gsmMccMnc.append((m_slot_id == 0)? "" : String8::format(".%d", m_slot_id));
            rfx_property_set(gsmMccMnc.string(), "");
            getStatusManager()->setString8Value(RFX_STATUS_KEY_UICC_GSM_NUMERIC,
                    String8(""));
            getStatusManager()->setString8Value(RFX_STATUS_KEY_GSM_IMSI, String8(""));
            getStatusManager()->setString8Value(RFX_STATUS_KEY_GSM_SPN, String8(""));
            getStatusManager()->setString8Value(RFX_STATUS_KEY_GSM_GID1, String8(""));
            getStatusManager()->setString8Value(RFX_STATUS_KEY_GSM_PNN, String8(""));
        }

        // reset CDMA systemProperty and statusKey
        if (aid.isEmpty() || (strncmp(aid.string(), "A0000003431002", 14) == 0)) {
            String8 cdmaMccMnc("vendor.cdma.ril.uicc.mccmnc");
            cdmaMccMnc.append((m_slot_id == 0)? "" : String8::format(".%d", m_slot_id));
            rfx_property_set(cdmaMccMnc.string(), "");
            getStatusManager()->setString8Value(RFX_STATUS_KEY_UICC_CDMA_NUMERIC, String8(""));

            String8 cdmaSubscriberId("vendor.ril.uim.subscriberid");
            cdmaSubscriberId.append(String8::format(".%d", (m_slot_id + 1)));
            rfx_property_set(cdmaSubscriberId.string(), "");
            getStatusManager()->setString8Value(RFX_STATUS_KEY_C2K_IMSI, String8(""));

            getStatusManager()->setString8Value(RFX_STATUS_KEY_CDMA_SPN, String8(""));
            getStatusManager()->setIntValue(RFX_STATUS_KEY_UICC_SUB_CHANGED_STATUS, -1);
        }

        // reset common systemProperty and statusKey
        if (aid.isEmpty()) {
            resetSimParameters();
        }
    }
}

void RtcCommSimController::resetSimParameters() {
    String8 pin1("vendor.gsm.sim.retry.pin1");
    String8 pin2("vendor.gsm.sim.retry.pin2");
    String8 puk1("vendor.gsm.sim.retry.puk1");
    String8 puk2("vendor.gsm.sim.retry.puk2");

    pin1.append((m_slot_id == 0)? "" : String8::format(".%d", (m_slot_id + 1)));
    pin2.append((m_slot_id == 0)? "" : String8::format(".%d", (m_slot_id + 1)));
    puk1.append((m_slot_id == 0)? "" : String8::format(".%d", (m_slot_id + 1)));
    puk2.append((m_slot_id == 0)? "" : String8::format(".%d", (m_slot_id + 1)));

    rfx_property_set(pin1.string(), "");
    rfx_property_set(pin2.string(), "");
    rfx_property_set(puk1.string(), "");
    rfx_property_set(puk2.string(), "");
}

bool RtcCommSimController::onCheckIfRejectMessage(
        const sp<RfxMessage>& message, bool isModemPowerOff, int radioState) {
    int msgId = message->getId();

    if (!isModemPowerOff && (radioState == (int)RADIO_STATE_OFF) &&
            (msgId == RFX_MSG_REQUEST_GET_SIM_STATUS ||
             msgId == RFX_MSG_REQUEST_ENTER_SIM_PIN ||
             msgId == RFX_MSG_REQUEST_ENTER_SIM_PUK ||
             msgId == RFX_MSG_REQUEST_ENTER_SIM_PIN2 ||
             msgId == RFX_MSG_REQUEST_ENTER_SIM_PUK2 ||
             msgId == RFX_MSG_REQUEST_CHANGE_SIM_PIN ||
             msgId == RFX_MSG_REQUEST_CHANGE_SIM_PIN2 ||
             msgId == RFX_MSG_REQUEST_SIM_ENTER_NETWORK_DEPERSONALIZATION ||
             msgId == RFX_MSG_REQUEST_SIM_ENTER_DEPERSONALIZATION ||
             msgId == RFX_MSG_REQUEST_SIM_IO ||
             msgId == RFX_MSG_REQUEST_ISIM_AUTHENTICATION ||
             msgId == RFX_MSG_REQUEST_GENERAL_SIM_AUTH ||
             msgId == RFX_MSG_REQUEST_SIM_AUTHENTICATION ||
             msgId == RFX_MSG_REQUEST_SIM_OPEN_CHANNEL ||
             msgId == RFX_MSG_REQUEST_SIM_CLOSE_CHANNEL ||
             msgId == RFX_MSG_REQUEST_SIM_TRANSMIT_APDU_BASIC ||
             msgId == RFX_MSG_REQUEST_SIM_TRANSMIT_APDU_CHANNEL ||
             msgId == RFX_MSG_REQUEST_SIM_GET_ATR ||
             msgId == RFX_MSG_REQUEST_SIM_GET_ICCID ||
             msgId == RFX_MSG_REQUEST_SIM_SAP_CONNECT ||
             msgId == RFX_MSG_REQUEST_SIM_SAP_DISCONNECT ||
             msgId == RFX_MSG_REQUEST_SIM_SAP_APDU ||
             msgId == RFX_MSG_REQUEST_SIM_SAP_TRANSFER_ATR ||
             msgId == RFX_MSG_REQUEST_SIM_SAP_POWER ||
             msgId == RFX_MSG_REQUEST_SIM_SAP_RESET_SIM ||
             msgId == RFX_MSG_REQUEST_SIM_SAP_SET_TRANSFER_PROTOCOL ||
             msgId == RFX_MSG_REQUEST_SIM_SAP_ERROR_RESP ||
             msgId == RFX_MSG_REQUEST_GET_IMSI ||
             msgId == RFX_MSG_REQUEST_QUERY_FACILITY_LOCK ||
             msgId == RFX_MSG_REQUEST_SET_FACILITY_LOCK ||
             msgId == RFX_MSG_REQUEST_SIM_SET_SIM_NETWORK_LOCK ||
             msgId == RFX_MSG_REQUEST_SIM_QUERY_SIM_NETWORK_LOCK ||
             msgId == RFX_MSG_REQUEST_CDMA_SUBSCRIPTION ||
             msgId == RFX_MSG_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE ||
             msgId == RFX_MSG_REQUEST_SET_SIM_CARD_POWER ||
             msgId == RFX_MSG_REQUEST_SET_SIM_POWER ||
             msgId == RFX_MSG_REQUEST_ACTIVATE_UICC_CARD ||
             msgId == RFX_MSG_REQUEST_DEACTIVATE_UICC_CARD ||
             msgId == RFX_MSG_REQUEST_GET_CURRENT_UICC_CARD_PROVISIONING_STATUS ||
             msgId == RFX_MSG_REQUEST_UPDATE_SUBLOCK_SETTINGS ||
             msgId == RFX_MSG_REQUEST_GET_SUBLOCK_MODEM_STATUS ||
             msgId == RFX_MSG_REQUEST_SIM_ENTER_DEVICE_NETWORK_DEPERSONALIZATION ||
             msgId == RFX_MSG_REQUEST_SET_ALLOWED_CARRIERS ||
             msgId == RFX_MSG_REQUEST_GET_ALLOWED_CARRIERS ||
             msgId == RFX_MSG_REQUEST_SML_RSU_REQUEST)) {
        return false;
    } else if (msgId == RFX_MSG_REQUEST_SIM_VSIM_NOTIFICATION ||
            msgId == RFX_MSG_REQUEST_SIM_VSIM_OPERATION) {
        return false;
    }

    return RfxController::onCheckIfRejectMessage(message, isModemPowerOff, radioState);
}

bool RtcCommSimController::onHandleSimIoRequest(const sp<RfxMessage>& msg) {
    int fileId = 0;
    char *path = NULL;
    RIL_SIM_IO_v6 *pData = (RIL_SIM_IO_v6*)(msg->getData()->getData());
    fileId = pData->fileid;
    path = pData->path;
    switch(fileId) {
        case 0x6F3A: // ADN
        case 0x6F3B: // FDN
        case 0x6F49: // SDN
        case 0x6F4A: // EXT1
        case 0x6F4B: // EXT2
        case 0x6F4C: { // EXT3
                if (path != NULL) {
                    logD(mTag, "SIM IO fileId 2GSIM %s %s", idToString(fileId), path);
                    // if path include 7F10, 0x6F3A is ADN
                    if ((fileId != 0x6F3A) ||
                            (strstr(path, "7F10") != NULL || strstr(path, "7f10") != NULL)) {
                        sp<RfxMessage> phbMsg = RfxMessage::obtainRequest(msg->getSlotId(),
                                RFX_MSG_REQUEST_PHB_SIM_IO, msg, true);
                        phbMsg->setPriority(MTK_RIL_REQUEST_PRIORITY_LOW);
                        requestToMcl(phbMsg);
                    } else {
                        requestToMcl(msg);
                    }
                } else {
                    logD(mTag, "SIM IO fileId 2GSIM %s path is null", idToString(fileId));
                    requestToMcl(msg);
                }
            }
            break;
        case 0x4F30: { // EFPBR
                logD(mTag, "SIM IO fileId PBR %s", idToString(fileId));
                sp<RfxMessage> phbMsg = RfxMessage::obtainRequest(msg->getSlotId(),
                        RFX_MSG_REQUEST_PHB_PBR_SIM_IO, msg, true);
                phbMsg->setPriority(MTK_RIL_REQUEST_PRIORITY_LOW);
                requestToMcl(phbMsg);
            }
            break;
        default: {
                RtcPhbSimIoController* phbSimIoController = (RtcPhbSimIoController *)findController(
                        m_slot_id, RFX_OBJ_CLASS_INFO(RtcPhbSimIoController));
                if (phbSimIoController->onCheckIfPhbRequest(fileId) == true) {
                    sp<RfxMessage> phbMsg = RfxMessage::obtainRequest(msg->getSlotId(),
                            RFX_MSG_REQUEST_PHB_SIM_IO, msg, true);
                    phbMsg->setPriority(MTK_RIL_REQUEST_PRIORITY_LOW);
                    requestToMcl(phbMsg);
                } else {
                    // Do not use cache as modem is not supppported in all of platforms.
                    // handleSimIo(msg);
                    requestToMcl(msg);
                }
            }
            break;
    }
    return true;
}

bool RtcCommSimController::onHandleRequest(const sp<RfxMessage>& msg) {
    int msg_id = msg->getId();
    switch (msg_id) {
        case RFX_MSG_REQUEST_SIM_IO : {
                onHandleSimIoRequest(msg);
            }
            break;
        case RFX_MSG_REQUEST_SIM_GET_ICCID : {
                handleGetIccid(msg);
            }
            break;
        case RFX_MSG_REQUEST_GET_IMSI : {
                handleGetImsi(msg);
            }
            break;
        case RFX_MSG_REQUEST_GET_SIM_STATUS :
        case RFX_MSG_REQUEST_ENTER_SIM_PIN :
        case RFX_MSG_REQUEST_ENTER_SIM_PUK :
        case RFX_MSG_REQUEST_ENTER_SIM_PIN2 :
        case RFX_MSG_REQUEST_ENTER_SIM_PUK2 :
        case RFX_MSG_REQUEST_CHANGE_SIM_PIN :
        case RFX_MSG_REQUEST_CHANGE_SIM_PIN2 :
        case RFX_MSG_REQUEST_SIM_ENTER_NETWORK_DEPERSONALIZATION :
        case RFX_MSG_REQUEST_SIM_ENTER_DEPERSONALIZATION :
        case RFX_MSG_REQUEST_ISIM_AUTHENTICATION :
        case RFX_MSG_REQUEST_GENERAL_SIM_AUTH :
        case RFX_MSG_REQUEST_SIM_AUTHENTICATION :
        case RFX_MSG_REQUEST_SIM_OPEN_CHANNEL :
        case RFX_MSG_REQUEST_SIM_CLOSE_CHANNEL :
        case RFX_MSG_REQUEST_SIM_TRANSMIT_APDU_BASIC :
        case RFX_MSG_REQUEST_SIM_TRANSMIT_APDU_CHANNEL :
        case RFX_MSG_REQUEST_SIM_GET_ATR :
        case RFX_MSG_REQUEST_SIM_SAP_CONNECT :
        case RFX_MSG_REQUEST_SIM_SAP_DISCONNECT :
        case RFX_MSG_REQUEST_SIM_SAP_APDU :
        case RFX_MSG_REQUEST_SIM_SAP_TRANSFER_ATR :
        case RFX_MSG_REQUEST_SIM_SAP_POWER :
        case RFX_MSG_REQUEST_SIM_SAP_RESET_SIM :
        case RFX_MSG_REQUEST_SIM_SAP_SET_TRANSFER_PROTOCOL :
        case RFX_MSG_REQUEST_SIM_SAP_ERROR_RESP :
        case RFX_MSG_REQUEST_SIM_SET_SIM_NETWORK_LOCK :
        case RFX_MSG_REQUEST_SIM_QUERY_SIM_NETWORK_LOCK:
        case RFX_MSG_REQUEST_SIM_VSIM_NOTIFICATION:
        case RFX_MSG_REQUEST_SIM_VSIM_OPERATION:
        case RFX_MSG_REQUEST_CDMA_SUBSCRIPTION:
        case RFX_MSG_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE:
        case RFX_MSG_REQUEST_SET_SIM_CARD_POWER:
        case RFX_MSG_REQUEST_SET_SIM_POWER:
        case RFX_MSG_REQUEST_ACTIVATE_UICC_CARD:
        case RFX_MSG_REQUEST_DEACTIVATE_UICC_CARD:
        case RFX_MSG_REQUEST_UPDATE_SUBLOCK_SETTINGS:
        case RFX_MSG_REQUEST_GET_SUBLOCK_MODEM_STATUS:
        case RFX_MSG_REQUEST_SIM_ENTER_DEVICE_NETWORK_DEPERSONALIZATION:
        case RFX_MSG_REQUEST_SET_ALLOWED_CARRIERS:
        case RFX_MSG_REQUEST_GET_ALLOWED_CARRIERS:
        case RFX_MSG_REQUEST_SML_RSU_REQUEST: {
                // Send RMC directly
                requestToMcl(msg);
            }
            break;
        case RFX_MSG_REQUEST_GET_CURRENT_UICC_CARD_PROVISIONING_STATUS: {
                handleGetCurrentUiccCardProcisioningStatus(msg);
            }
            break;
        case RFX_MSG_REQUEST_QUERY_FACILITY_LOCK :
        case RFX_MSG_REQUEST_SET_FACILITY_LOCK :
            if (isCallBarringUsage(msg)) {
                handleCallBarring(msg);
            } else {
                requestToMcl(msg);
            }
            break;
        default:
            logD(mTag, "Not Support the req %s", idToString(msg_id));
            break;
    }

    return true;
}

bool RtcCommSimController::onHandleResponse(const sp<RfxMessage>& msg) {
    int msg_id = msg->getId();
    switch (msg_id) {
        case RFX_MSG_REQUEST_GET_SIM_STATUS :
        case RFX_MSG_REQUEST_ENTER_SIM_PIN :
        case RFX_MSG_REQUEST_ENTER_SIM_PUK :
        case RFX_MSG_REQUEST_ENTER_SIM_PIN2 :
        case RFX_MSG_REQUEST_ENTER_SIM_PUK2 :
        case RFX_MSG_REQUEST_CHANGE_SIM_PIN :
        case RFX_MSG_REQUEST_CHANGE_SIM_PIN2 :
        case RFX_MSG_REQUEST_SIM_ENTER_NETWORK_DEPERSONALIZATION :
        case RFX_MSG_REQUEST_SIM_ENTER_DEPERSONALIZATION :
        case RFX_MSG_REQUEST_SIM_IO :
        case RFX_MSG_REQUEST_ISIM_AUTHENTICATION :
        case RFX_MSG_REQUEST_GENERAL_SIM_AUTH :
        case RFX_MSG_REQUEST_SIM_AUTHENTICATION :
        case RFX_MSG_REQUEST_SIM_OPEN_CHANNEL :
        case RFX_MSG_REQUEST_SIM_CLOSE_CHANNEL :
        case RFX_MSG_REQUEST_SIM_TRANSMIT_APDU_BASIC :
        case RFX_MSG_REQUEST_SIM_TRANSMIT_APDU_CHANNEL :
        case RFX_MSG_REQUEST_SIM_GET_ATR :
        case RFX_MSG_REQUEST_SIM_GET_ICCID :
        case RFX_MSG_REQUEST_GET_IMSI :
        case RFX_MSG_REQUEST_QUERY_FACILITY_LOCK :
        case RFX_MSG_REQUEST_SET_FACILITY_LOCK :
        case RFX_MSG_REQUEST_SIM_SET_SIM_NETWORK_LOCK :
        case RFX_MSG_REQUEST_SIM_QUERY_SIM_NETWORK_LOCK:
        case RFX_MSG_REQUEST_SIM_VSIM_NOTIFICATION:
        case RFX_MSG_REQUEST_SIM_VSIM_OPERATION:
        case RFX_MSG_REQUEST_CDMA_SUBSCRIPTION:
        case RFX_MSG_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE:
        case RFX_MSG_REQUEST_SET_SIM_CARD_POWER:
        case RFX_MSG_REQUEST_SET_SIM_POWER:
        case RFX_MSG_REQUEST_ACTIVATE_UICC_CARD:
        case RFX_MSG_REQUEST_DEACTIVATE_UICC_CARD:
        case RFX_MSG_REQUEST_GET_CURRENT_UICC_CARD_PROVISIONING_STATUS:
        case RFX_MSG_REQUEST_UPDATE_SUBLOCK_SETTINGS:
        case RFX_MSG_REQUEST_GET_SUBLOCK_MODEM_STATUS:
        case RFX_MSG_REQUEST_SIM_ENTER_DEVICE_NETWORK_DEPERSONALIZATION:
        case RFX_MSG_REQUEST_SET_ALLOWED_CARRIERS:
        case RFX_MSG_REQUEST_GET_ALLOWED_CARRIERS:
        case RFX_MSG_REQUEST_SML_RSU_REQUEST: {
                // Send RILJ directly
                responseToRilj(msg);
            }
            break;
        case RFX_MSG_REQUEST_SIM_SAP_CONNECT :
        case RFX_MSG_REQUEST_SIM_SAP_DISCONNECT :
        case RFX_MSG_REQUEST_SIM_SAP_APDU :
        case RFX_MSG_REQUEST_SIM_SAP_TRANSFER_ATR :
        case RFX_MSG_REQUEST_SIM_SAP_POWER :
        case RFX_MSG_REQUEST_SIM_SAP_RESET_SIM :
        case RFX_MSG_REQUEST_SIM_SAP_SET_TRANSFER_PROTOCOL :
        case RFX_MSG_REQUEST_SIM_SAP_ERROR_RESP : {
                // Send BT directly
                responseToBT(msg);
            }
            break;
        default:
            logD(mTag, "Not Support the req %s", idToString(msg_id));
            break;
    }

    return true;
}

bool RtcCommSimController::onHandleUrc(const sp<RfxMessage>& message) {
    switch (message->getId()) {
        case RFX_MSG_URC_SIM_SLOT_LOCK_POLICY_NOTIFY: {
            mCacheSmlMsg = RfxMessage::obtainUrc(getSlotId(), message->getId(), message);
            int* smlData = (int*)(message->getData()->getData());
            int smlDataLength = message->getData()->getDataLength();
            logD(mTag, "onHandleUrc %s %d %d %d %d %d (slot %d)", message->toString().string(),
                    smlDataLength, smlData[0], smlData[1], smlData[2], smlData[3], getSlotId());
            responseToRilj(message);
            break;
        }
        default:
            return false;
    }
    return true;
}

void RtcCommSimController::handleCallBarring(const sp<RfxMessage>& msg) {
    int msg_id = msg->getId();

    // Check if current project is a data only project by CONFIG_SS_MODE
    FeatureValue featurevalue;
    memset(featurevalue.value, 0, sizeof(featurevalue.value));
    mtkGetFeature(CONFIG_SS_MODE, &featurevalue);
    logD(mTag, "Check data only project, CONFIG_SS_MODE = %s", featurevalue.value);
    if (strcmp(featurevalue.value, "0") == 0) {
        sp<RfxMessage> responseMsg = RfxMessage::obtainResponse(RIL_E_REQUEST_NOT_SUPPORTED,
                msg, false);
        responseToRilj(responseMsg);
        return;
    }

    // convert the message id for call barring usage
    if (msg_id == RFX_MSG_REQUEST_QUERY_FACILITY_LOCK) {
        msg_id = RFX_MSG_REQUEST_QUERY_CALL_BARRING;
    } else if (msg_id == RFX_MSG_REQUEST_SET_FACILITY_LOCK) {
        msg_id = RFX_MSG_REQUEST_SET_CALL_BARRING;
    }

    // Re-enqueue the message into main thread, RtcSuppServController will handle it
    sp<RfxMessage> newMsg = RfxMessage::obtainRequest(msg_id,
            RfxStringsData(msg->getData()->getData(), msg->getData()->getDataLength()), msg, true);
    RfxMainThread::enqueueMessage(newMsg);
}

bool RtcCommSimController::isCallBarringUsage(const sp<RfxMessage>& msg) {
    bool r = false;
    char** pStrings = (char**)(msg->getData()->getData());
    String8 facility(pStrings[0]);
    String8 facFd("FD");
    String8 facSc("SC");

    if (facility != facFd && facility != facSc) {
        r = true;
    }
    logD(mTag, "isCallBarringUsage: %s, facility = %s", r ? "true" : "false", facility.string());
    return r;
}

void RtcCommSimController::handleGetIccid(const sp<RfxMessage>& msg) {
    String8 iccid = getStatusManager()->getString8Value(RFX_STATUS_KEY_SIM_ICCID);

    if (!iccid.isEmpty()) {
        sp<RfxMessage> response = RfxMessage::obtainResponse(m_slot_id,
                RFX_MSG_REQUEST_SIM_GET_ICCID, RIL_E_SUCCESS, RfxStringData((void*)iccid.string(),
                strlen(iccid.string())), msg);
        logD(mTag, "handleGetIccid iccid: %.7s", iccid.string());

        responseToRilj(response);
    } else {
        // Send request to modem.
        logD(mTag, "handleGetIccid iccid: %.7s, send request to mcl", iccid.string());
        requestToMcl(msg);
    }
}

void RtcCommSimController::handleGetImsi(const sp<RfxMessage>& msg) {
    int cardType = getStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE, -1);
    char** pStrings = (char**)(msg->getData()->getData());
    int app = -1;
    String8 imsi("");
    String8 aid(((pStrings != NULL) && (pStrings[0] != NULL))? pStrings[0] : "");
    bool bSendToMD = true;

    if (aid.isEmpty()) {
        if (cardType & RFX_CARD_TYPE_SIM) {
            app = UICC_APP_SIM;
        } else if (cardType & RFX_CARD_TYPE_RUIM) {
            app = UICC_APP_RUIM;
        } else if (cardType & RFX_CARD_TYPE_CSIM) {
            app = UICC_APP_CSIM;
        } else {
            logD(mTag, "Could not get app id because card type is not ready!");
        }
    } else if (strncmp(aid.string(), "A0000000871002", 14) == 0) {
        app = UICC_APP_USIM;
    } else if (strncmp(aid.string(), "A0000000871004", 14) == 0) {
        // ISIM
        app = UICC_APP_ISIM;
    } else if (strncmp(aid.string(), "A0000003431002", 14) == 0) {
        // CSIM
        app = UICC_APP_CSIM;
    } else if (strncmp(aid.string(), "A000000000RUIM", 14) == 0) {
        // RUIM
        app = UICC_APP_RUIM;
    } else if (strncmp(aid.string(), "A0000000000GSM", 14) == 0) {
        app = UICC_APP_USIM;
        bSendToMD = false;
    } else if (strncmp(aid.string(), "A0000000000C2K", 14) == 0) {
        app = UICC_APP_RUIM;
        bSendToMD = false;
    } else {
        logD(mTag, "Not support the aid %s", aid.string());
    }

    if ((app == UICC_APP_SIM) || ((app == UICC_APP_USIM))) {
        imsi = getStatusManager()->getString8Value(RFX_STATUS_KEY_GSM_IMSI);
    } else if ((app == UICC_APP_RUIM) || ((app == UICC_APP_CSIM))) {
        imsi = getStatusManager()->getString8Value(RFX_STATUS_KEY_C2K_IMSI);
    }

    if (!imsi.isEmpty()) {
        sp<RfxMessage> response = RfxMessage::obtainResponse(m_slot_id,
                RFX_MSG_REQUEST_GET_IMSI, RIL_E_SUCCESS, RfxStringData((void*)imsi.string(),
                strlen(imsi.string())), msg);
        responseToRilj(response);
    } else if (!bSendToMD) {
        logD(mTag, "Not send the request to MD");
        sp<RfxMessage> response = RfxMessage::obtainResponse(m_slot_id,
                RFX_MSG_REQUEST_GET_IMSI, RIL_E_SUCCESS, RfxVoidData(), msg);
        responseToRilj(response);
    } else {
        // Send request to modem.
        requestToMcl(msg);
    }
}

void RtcCommSimController::handleSimIo(const sp<RfxMessage>& msg) {
    RIL_SIM_IO_v6 *pData = (RIL_SIM_IO_v6*)(msg->getData()->getData());
    RIL_SIM_IO_Response sr;
    memset(&sr, 0, sizeof(RIL_SIM_IO_Response));
    bool isCached = false;
    int app = -1;
    String8 aid((pData->aidPtr != NULL)? pData->aidPtr : "");
    int cardType = getStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE, -1);

    if (aid.isEmpty() && (cardType & RFX_CARD_TYPE_SIM)) {
        app = UICC_APP_SIM;
    } else if (strncmp(aid.string(), "A0000000871002", 14) == 0) {
        app = UICC_APP_USIM;
    }

    if (app != -1) {
        for (int i = 0; i < SIM_CACHED_FILE_COUNT; i++) {
            if ((pData->fileid == mIoResponse[app][i].fileid)
                    && (strncmp(pData->path, mIoResponse[app][i].path, strlen(pData->path))
                    == 0)) {
                if ((pData->command == 0xc0) && (mIoResponse[app][i].get_rsp.valid)) {
                    // Get fcp from cache list.
                    sr.sw1 =  mIoResponse[app][i].get_rsp.sw1;
                    sr.sw2 =  mIoResponse[app][i].get_rsp.sw2;

                    if (mIoResponse[app][i].get_rsp.simResponse != NULL) {
                        asprintf(&sr.simResponse, "%s", mIoResponse[app][i].get_rsp.simResponse);
                    }

                    isCached = true;
                    break;
                } else if ((pData->command == 0xb0) && (mIoResponse[app][i].read_binary.valid)) {
                    // Get binary from cache list.
                    sr.sw1 =  mIoResponse[app][i].read_binary.sw1;
                    sr.sw2 =  mIoResponse[app][i].read_binary.sw2;

                    if (mIoResponse[app][i].read_binary.simResponse != NULL) {
                        asprintf(&sr.simResponse, "%s", mIoResponse[app][i].read_binary
                                .simResponse);
                    }

                    isCached = true;
                    break;
                }
            }
        }
    }
    logD(mTag, "handleSimIo: sw1: %d, sw2: %d, rs: %s, isCached: %d",  sr.sw1, sr.sw2,
            sr.simResponse, isCached);

    if ((app != -1) && isCached) {
        sp<RfxMessage> response = RfxMessage::obtainResponse(m_slot_id, RFX_MSG_REQUEST_SIM_IO,
                RIL_E_SUCCESS, RfxSimIoRspData((void*)&sr, sizeof(sr)), msg);
        responseToRilj(response);

        if (sr.simResponse != NULL) {
            free(sr.simResponse);
        }
    } else {
        // Send request to modem.
        requestToMcl(msg);
    }
}

void RtcCommSimController::handleGetCurrentUiccCardProcisioningStatus(const sp<RfxMessage>& msg) {
    int simPowerOnOffState = 1;

    if (getStatusManager()->getIntValue(RFX_STATUS_KEY_SIM_ONOFF_STATE) == SIM_POWER_STATE_SIM_OFF) {
        simPowerOnOffState = 0;
    }

    logD(mTag, "handleGetCurrentUiccCardProcisioningStatus simPowerOnOffState: %d", simPowerOnOffState);

    sp<RfxMessage> response = RfxMessage::obtainResponse(m_slot_id,
            RFX_MSG_REQUEST_GET_CURRENT_UICC_CARD_PROVISIONING_STATUS, RIL_E_SUCCESS,
            RfxIntsData(&simPowerOnOffState, 1), msg);
    responseToRilj(response);
}

