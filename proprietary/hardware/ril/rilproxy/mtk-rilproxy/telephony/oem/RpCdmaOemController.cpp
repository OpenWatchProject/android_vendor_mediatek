/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
#include "RpCdmaOemController.h"
#include "RfxTimer.h"
#include <cutils/properties.h>
#include <cutils/jstring.h>
#include <string>
#include <stdlib.h>
#include <utils/String16.h>
#include <stdio.h>
#include <cctype>
#include "RfxLog.h"
#include "util/RpFeatureOptionUtils.h"
#include "modecontroller/RpCdmaLteModeController.h"
#include <libmtkrilutils.h>

using ::android::String16;
using ::android::String8;
/*****************************************************************************
 * Class RpCdmaOemController
 *****************************************************************************/
#define RP_CDMA_OEM_TAG "RpCdmaOemCtrl"
/*DESTRILD*/
#define DEST_C2K_RILD "DESTRILD:C2K"
#define DEST_GSM_RILD "DESTRILD:GSM"
#define RPC_OEM_STRING_STATUS_SYNC "STATUS_SYNC"
#define GSM_TRM_MODE 0
#define CDMA_TRM_MODE 1

RFX_IMPLEMENT_CLASS("RpCdmaOemController", RpCdmaOemController, RfxController);
RFX_OBJ_IMPLEMENT_SINGLETON_CLASS(RpCdmaOemController);

RpCdmaOemController::RpCdmaOemController() :
    m_str_cdma_imei(""),
    m_str_cdma_imeisv(""),
    m_str_cdma_esnHex(""),
    m_str_cdma_meidHex(""),
    m_str_cdma_uimid(""),
    m_str_gsm_imei(""),
    m_str_gsm_imeisv(""),
    m_str_cdma_esnDec(""),
    m_str_cdma_meidDec("") {
}

RpCdmaOemController::~RpCdmaOemController() {
}

void RpCdmaOemController::onInit() {
    RfxController::onInit(); // Required: invoke super class implementation
    RFX_LOG_D(RP_CDMA_OEM_TAG, "[RpCdmaOemController] onInit()");
    const int request_id_list[] = {
        RIL_REQUEST_BASEBAND_VERSION,
        RIL_REQUEST_OEM_HOOK_RAW,
        RIL_REQUEST_OEM_HOOK_STRINGS,
        RIL_REQUEST_DEVICE_IDENTITY,
        RIL_REQUEST_SCREEN_STATE,
        RIL_REQUEST_GSM_DEVICE_IDENTITY,
        RIL_REQUEST_SET_TRM,
        RIL_REQUEST_SET_UNSOLICITED_RESPONSE_FILTER,
        RIL_REQUEST_SEND_DEVICE_STATE,
    };
    const int urc_id_list[] = {

    };
    if (RpFeatureOptionUtils::isC2kSupport()) {
        registerToHandleRequest(request_id_list, (sizeof(request_id_list)/sizeof(int)));
        registerToHandleUrc(urc_id_list, 0);
    }
}

bool RpCdmaOemController::onHandleRequest(const sp<RfxMessage>& message) {
    int msg_id = message->getId();
    RFX_LOG_D(RP_CDMA_OEM_TAG, "[RpCdmaOemController] xonHandleRequest req %d", msg_id);

    switch (msg_id) {
        case RIL_REQUEST_BASEBAND_VERSION: {
            requestBaseBand(message);
        }
        break;
        case RIL_REQUEST_OEM_HOOK_RAW:
            handleOemHookRawReq(message);
            break;
        case RIL_REQUEST_OEM_HOOK_STRINGS:
            handleOemHookStringsReq(message);
            break;
        case RIL_REQUEST_DEVICE_IDENTITY:
            requestDeviceIdentity(message);
            break;
        case RIL_REQUEST_SCREEN_STATE:
            handleScreenStateReq(message);
            break;
        case RIL_REQUEST_SET_TRM:
            requestSetTrm(message);
            break;
        case RIL_REQUEST_SET_UNSOLICITED_RESPONSE_FILTER:
            requestSetUnsolResponseFilter(message);
            break;
        case RIL_REQUEST_SEND_DEVICE_STATE:
            requestSendDeviceState(message);
            break;
        default:
        break;
    }
    return true;
}

bool RpCdmaOemController::onHandleUrc(const sp<RfxMessage>& message) {
    return true;
}

bool RpCdmaOemController::onHandleResponse(const sp<RfxMessage>& message) {
    int msg_id = message->getId();
    RFX_LOG_D(RP_CDMA_OEM_TAG, "[RpCdmaOemController] onHandleResponse req %d", msg_id);
    switch (msg_id) {
        case RIL_REQUEST_BASEBAND_VERSION: {
            responseBaseBand(message);
        }
        break;
        case RIL_REQUEST_OEM_HOOK_RAW: {
            responseToRilj(message);
        }
        break;
        case RIL_REQUEST_OEM_HOOK_STRINGS: {
            responseToRilj(message);
        }
        break;
        case RIL_REQUEST_DEVICE_IDENTITY: {
            responseDeviceIdentity(message);
        }
        break;
        case RIL_REQUEST_SCREEN_STATE: {
           handleScreenStateRsp(message);
        }
        break;
        case RIL_REQUEST_GSM_DEVICE_IDENTITY: {
            responseDeviceIdentity(message);
        }
        break;
        case RIL_REQUEST_SET_TRM: {
            responseSetTrm(message);
        }
        break;
        case RIL_REQUEST_SET_UNSOLICITED_RESPONSE_FILTER:
            responseSetUnsolResponseFilter(message);
        break;
        case RIL_REQUEST_SEND_DEVICE_STATE:
            responseSendDeviceState(message);
        break;
        default:
        break;
    }
    return true;
}

void RpCdmaOemController::requestBaseBand(const sp<RfxMessage>& message) {
    if (RpCdmaLteModeController::getCdmaSocketSlotId() == m_slot_id) {
        sp<RfxMessage> cdma_baseband_request =
                RfxMessage::obtainRequest(RADIO_TECH_GROUP_C2K,
                        message->getId(), message);
        requestToRild(cdma_baseband_request);
    }
    requestToRild(message);
}

void RpCdmaOemController::requestDeviceIdentity(const sp<RfxMessage>& message) {
    int slotId = message->getSlotId();
    int cdma_socket_id =
            getNonSlotScopeStatusManager()->getIntValue(RFX_STATUS_KEY_CDMA_SOCKET_SLOT, 0);
    RFX_LOG_D(RP_CDMA_OEM_TAG, "[requestDeviceIdentity]"
            " slotId = %d, cdma_socket_id = %d", slotId, cdma_socket_id);

    if (slotId != cdma_socket_id) {
       RFX_LOG_D(RP_CDMA_OEM_TAG, "[requestDeviceIdentity]"
               " slotId =! cdma_socket_id");
        requestToRild(message);
    } else {
        sp<RfxMessage> gsm_device_identity_request =
               RfxMessage::obtainRequest(RADIO_TECH_GROUP_GSM, RIL_REQUEST_GSM_DEVICE_IDENTITY,
               message, true);
        sp<RfxMessage> cdma_device_identity_request =
                RfxMessage::obtainRequest(RADIO_TECH_GROUP_C2K, RIL_REQUEST_DEVICE_IDENTITY,
                message, true);
        RFX_LOG_D(RP_CDMA_OEM_TAG, "[requestDeviceIdentity] request GIMEI and CMEID");
        requestToRild(gsm_device_identity_request);
        requestToRild(cdma_device_identity_request);
    }
}

void RpCdmaOemController::responseBaseBand(const sp<RfxMessage>& message) {
    if (message->getSource() == RADIO_TECH_GROUP_C2K) {
        if (NULL != message->getParcel()) {
            String16 str = message->getParcel()->readString16();
            RFX_LOG_D(RP_CDMA_OEM_TAG, "[RpCdmaOemController] onHandleResponse RADIO_TECH_GROUP_C2K"
                " RIL_REQUEST_BASEBAND_VERSION"
                " baseband =  %s , baseband.size = %d, baseband.isEmpty = %d",
                String8(str).string(), String8(str).size(), String8(str).isEmpty());
            if (!String8(str).isEmpty() && String8(str).size() > 0) {
                property_set("vendor.cdma.version.baseband", String8(str).string());
            }
        }
    } else if (message->getSource() == RADIO_TECH_GROUP_GSM) {
        RFX_LOG_D(RP_CDMA_OEM_TAG, "[RpCdmaOemController]"
            " onHandleResponse RADIO_TECH_GROUP_GSM"
            " RIL_REQUEST_BASEBAND_VERSION");
        responseToRilj(message);
    }
}
void RpCdmaOemController::responseDeviceIdentity(const sp<RfxMessage>& message) {
    RFX_LOG_D(RP_CDMA_OEM_TAG, "[responseDeviceIdentity]");

    sp<RfxMessage> msg = sp<RfxMessage>(NULL);
    if (message->getSentOnCdmaCapabilitySlot() !=  C_SLOT_STATUS_IS_CURRENT_SLOT) {
        RFX_LOG_D(RP_CDMA_OEM_TAG, "[responseDeviceIdentity] return for GSM");
        getImeiFromMessage(message);
    } else {
        ResponseStatus responseStatus = preprocessResponse(message, msg,
            RfxWaitResponseTimedOutCallback(this, &RpCdmaOemController::responseCallBack), s2ns(30));
        if (responseStatus == RESPONSE_STATUS_HAVE_MATCHED ||
                    responseStatus == RESPONSE_STATUS_HAVE_MATCHED_WITH_C_CHANGED) {
            RFX_LOG_D(RP_CDMA_OEM_TAG, "[responseDeviceIdentity] RESPONSE_STATUS_HAVE_MATCHED ");
            sp<RfxMessage> responseMsg = NULL;
            sp<RfxMessage> gsmMsg = NULL;
            if (message->getSource() == RADIO_TECH_GROUP_C2K) {
                RFX_LOG_D(RP_CDMA_OEM_TAG, "[responseDeviceIdentity] RESPONSE_STATUS_HAVE_MATCHED"
                        " RADIO_TECH_GROUP_C2K");
                responseMsg = message;
                gsmMsg = msg;
            } else {
                RFX_LOG_D(RP_CDMA_OEM_TAG, "[responseDeviceIdentity] RESPONSE_STATUS_HAVE_MATCHED"
                        " RADIO_TECH_GROUP_GSM");
                responseMsg = msg;
                gsmMsg = message;
            }
            getMeidFromMessage(responseMsg);
            getImeiFromMessage(gsmMsg);
            if (NULL != m_str_cdma_meidHex) {
                const size_t N = m_str_cdma_meidHex.size();
                const char16_t* str = m_str_cdma_meidHex.string();

                String16 str_cdma_meid_upper = (String16)"";
                for (size_t i = 0; i < N; i++) {
                    const char16_t v = str[i];
                    if (v >= 'a' && v <= 'z') {
                        const char16_t meidUpper = (char16_t)toupper((char)v);
                        str_cdma_meid_upper.append(&meidUpper, 1);
                    } else {
                        str_cdma_meid_upper.append(&v, 1);
                    }
                }
                m_str_cdma_meidHex = str_cdma_meid_upper;
                if (!isUserLoad()) {
                    RFX_LOG_D(RP_CDMA_OEM_TAG, "[responseDeviceIdentity]"
                            "str_cdma_meid_upper= %s"
                            " m_str_cdma_meidHex = %s",
                            String8(str_cdma_meid_upper).string(),
                            String8(m_str_cdma_meidHex).string());
                }
            }
        } else {
            return;
        }
    }
    Parcel *parcel = new Parcel();
    parcel->writeInt32(0);
    parcel->writeInt32(message->getPToken() & 0xFFFFFFFF);
    parcel->writeInt32(message->getError() & 0xFFFFFFFF);
    parcel->writeInt32(4);
    parcel->writeString16(m_str_gsm_imei);
    parcel->writeString16(m_str_gsm_imeisv);
    parcel->writeString16(m_str_cdma_esnHex);
    parcel->writeString16(m_str_cdma_meidHex);

    sp<RfxMessage> imeiResponse = RfxMessage::obtainResponse(message->getSlotId(),
            message->getPId(), message->getPToken(), message->getId(),
            message->getToken(), message->getError(), message->getSource(),
            parcel, message->getTimeStamp(), message->getRilToken());
    responseToRilj(imeiResponse);
    m_str_cdma_imei = (String16)"";
    m_str_cdma_imeisv = (String16)"";
    m_str_cdma_esnHex = (String16)"";
    m_str_cdma_meidHex = (String16)"";
    m_str_cdma_uimid = (String16)"";
    m_str_gsm_imei = (String16)"";
    m_str_gsm_imeisv = (String16)"";
    m_str_cdma_esnDec = (String16)"";
    m_str_cdma_meidDec = (String16)"";
}

void RpCdmaOemController::getMeidFromMessage(const sp<RfxMessage>& message) {
    if (message->getError() == RIL_E_SUCCESS && NULL != message->getParcel()) {
        String16 esnHexString16 = (String16)"";
        const char16_t* pEsnHexChar16 = NULL;
        char *pEsnHexChar = NULL;
        char *pTargetChar = NULL;
        Parcel *p = message->getParcel();
        int pos = p->dataPosition();
        int32_t type = p->readInt32();
        RFX_LOG_D(RP_CDMA_OEM_TAG, "[RpCdmaOemController] responseDeviceIdentityss "
            "type = %d", type);
        m_str_cdma_imei = message->getParcel()->readString16();
        m_str_cdma_imeisv = message->getParcel()->readString16();

        // get last 8 digits of esn
        esnHexString16 = message->getParcel()->readString16();
        pEsnHexChar16 = esnHexString16.string();
        pEsnHexChar = strndup16to8(pEsnHexChar16, esnHexString16.size());
        pTargetChar = strstr(pEsnHexChar, ":0x");
        // support Hex format and Dec format
        if (pTargetChar != NULL) {
            m_str_cdma_esnHex = String16(pTargetChar+3, strlen(pTargetChar+3));
            pTargetChar = NULL;
        } else {
            m_str_cdma_esnHex = esnHexString16;
        }
        esnHexString16 = (String16)"";
        free(pEsnHexChar);

        m_str_cdma_meidHex = message->getParcel()->readString16();
        m_str_cdma_uimid = message->getParcel()->readString16();
        m_str_cdma_esnDec = message->getParcel()->readString16();
        m_str_cdma_meidDec = message->getParcel()->readString16();
        if (!isUserLoad()) {
            RFX_LOG_D(RP_CDMA_OEM_TAG, "[responseDeviceIdentity] "
                    "m_str_cdma_imei = %s, m_str_cdma_imeisv = %s,"
                    " m_str_cdma_esnHex = %s, m_str_cdma_meidHex = %s, m_str_cdma_uimid = %s,"
                    " m_str_cdma_esnDec = %s, m_str_cdma_meidDec = %s",
                    String8(m_str_cdma_imei).string(), String8(m_str_cdma_imeisv).string(),
                    String8(m_str_cdma_esnHex).string(), String8(m_str_cdma_meidHex).string(),
                    String8(m_str_cdma_uimid).string(), String8(m_str_cdma_esnDec).string(),
                    String8(m_str_cdma_meidDec).string());
        }

        p->setDataPosition(pos);
    } else {
        RFX_LOG_D(RP_CDMA_OEM_TAG, "[getMeidFromMessage] message->getParcel() = null, ERROR: %d",
                message->getError());
    }
}
void RpCdmaOemController::getImeiFromMessage(const sp<RfxMessage>& message) {
    if (message->getError() == RIL_E_SUCCESS && NULL != message->getParcel()) {
        Parcel *p = message->getParcel();
        int pos = p->dataPosition();
        int32_t type = p->readInt32();
        m_str_gsm_imei = message->getParcel()->readString16();
        m_str_gsm_imeisv = message->getParcel()->readString16();
        if (!isUserLoad()) {
            RFX_LOG_D(RP_CDMA_OEM_TAG, "[getImeiFromMessage] type = %d, m_str_gsm_imeisv = %s",
                    type, String8(m_str_gsm_imeisv).string());
        }
        p->setDataPosition(pos);
    } else {
        RFX_LOG_D(RP_CDMA_OEM_TAG, "[getImeiFromMessage] message->getParcel() = null");
    }
}

void RpCdmaOemController::responseCallBack(const sp<RfxMessage>& message) {
    RFX_LOG_D(RP_CDMA_OEM_TAG, "[responseDeviceIdentity]  responseCallBack ");
    sp<RfxMessage> un_cdma_socket_request =
            RfxMessage::obtainResponse(RIL_E_GENERIC_FAILURE, message, true);
    responseToRilj(un_cdma_socket_request);
}

void RpCdmaOemController::handleOemHookRawReq(const sp<RfxMessage>& message) {
    Parcel *p = message->getParcel();
    int32_t countRaws = 0;
    int8_t i8;
    p->readInt32 (&countRaws);
    RFX_LOG_D(RP_CDMA_OEM_TAG, "[handleOemHookRawReq] countRaws: %d", countRaws);

    int len = strlen(DEST_C2K_RILD);
    if (countRaws <= len) {
        requestToRild(message);
    } else {

        char* data = (char *) calloc(1, countRaws + 1);
        if (data == NULL) {
            RFX_LOG_E(RP_CDMA_OEM_TAG, "[handleOemHookRawReq] calloc fail");
            sp<RfxMessage> response = RfxMessage::obtainResponse(RIL_E_NO_MEMORY, message);
            responseToRilj(response);
            return;
        }
        p->read(data, countRaws);
        char* c2k_str = strstr(data, DEST_C2K_RILD);
        if (c2k_str != NULL) {
            sp<RfxMessage> c2k_hook_raw_request = RfxMessage::obtainRequest(
                    RADIO_TECH_GROUP_C2K,
                    message->getId(),
                    message);
            c2k_hook_raw_request->getParcel()->writeInt32(countRaws - len);
            data[c2k_str-data] = '\0';
            RFX_LOG_D(RP_CDMA_OEM_TAG, "[handleOemHookRawReq]To c2k, newData: %s", data);
            c2k_hook_raw_request->getParcel()->writeCString(data);
            requestToRild(c2k_hook_raw_request);

        } else if (strstr(data, DEST_GSM_RILD) != NULL) {
            sp<RfxMessage> gsm_hook_raw_request = RfxMessage::obtainRequest(
                    RADIO_TECH_GROUP_GSM,
                    message->getId(),
                    message);
            gsm_hook_raw_request->getParcel()->writeInt32(countRaws - strlen(DEST_GSM_RILD));
            data[countRaws - len] = '\0';
            RFX_LOG_D(RP_CDMA_OEM_TAG, "[handleOemHookRawReq]To gsm, newData: %s", data);
            gsm_hook_raw_request->getParcel()->writeCString(data);
            requestToRild(gsm_hook_raw_request);
        } else {
            RFX_LOG_D(RP_CDMA_OEM_TAG, "[handleOemHookRawReq]to gsm, default");
            requestToRild(message);
        }

        free(data);
    }
}

void RpCdmaOemController::handleOemHookStringsReq(const sp<RfxMessage>& message) {
    Parcel *p = message->getParcel();
    int32_t countStrings = 0;
    String16 s16;
    p->readInt32 (&countStrings);
    RFX_LOG_D(RP_CDMA_OEM_TAG, "[handleOemHookStringsReq] countStrings: %d", countStrings);
    if (countStrings <= 1) {
        requestToRild(message);
    } else if (handleExtraState(message)) {
        sp<RfxMessage> responseMsg = RfxMessage::obtainResponse(RIL_E_SUCCESS, message,
                true);
        responseToRilj(responseMsg);
        return;
    } else {
        int pos = p->dataPosition();
        for (int i = 0 ; i < countStrings; i++) {
            s16 = p->readString16();
        }

        // Add local variable here not use String8(s16).string() to avoid temp object free issue.
        String8 last_string = String8(s16);
        const char* pString = last_string.string();
        p->setDataPosition(pos);
        RFX_LOG_D(RP_CDMA_OEM_TAG, "[handleOemHookStringsReq]string: %s, dataPosition: %d", pString, pos);
        if (pString != NULL && strncmp(pString, DEST_C2K_RILD, strlen(DEST_C2K_RILD)) == 0) {
            sp<RfxMessage> c2k_hook_strings_request = RfxMessage::obtainRequest(
                    RADIO_TECH_GROUP_C2K,
                    message->getId(),
                    message);
            c2k_hook_strings_request->getParcel()->writeInt32(countStrings - 1);
            for (int i = 0 ; i < countStrings - 1; i++) {
                s16 = p->readString16();
                c2k_hook_strings_request->getParcel()->writeString16(s16);
                RFX_LOG_D(RP_CDMA_OEM_TAG, "[handleOemHookStringsReq]To c2k, string: %s", String8(s16).string());
            }
            requestToRild(c2k_hook_strings_request);
        } else if (pString != NULL && strncmp(pString, DEST_GSM_RILD, strlen(DEST_GSM_RILD)) == 0) {
            sp<RfxMessage> gsm_hook_strings_request = RfxMessage::obtainRequest(
                    RADIO_TECH_GROUP_GSM,
                    message->getId(),
                    message);
            gsm_hook_strings_request->getParcel()->writeInt32(countStrings - 1);
            for (int i = 0 ; i < countStrings - 1; i++) {
                s16 = p->readString16();
                gsm_hook_strings_request->getParcel()->writeString16(s16);
                RFX_LOG_D(RP_CDMA_OEM_TAG, "[handleOemHookStringsReq]to gsm, string: %s", String8(s16).string());
            }
            requestToRild(gsm_hook_strings_request);
        } else {
            RFX_LOG_D(RP_CDMA_OEM_TAG, "[handleOemHookStringsReq]to gsm, default");
            requestToRild(message);
        }
    }
}

void RpCdmaOemController::handleScreenStateReq(const sp<RfxMessage>& message) {
    RFX_LOG_D(RP_CDMA_OEM_TAG, "[handleScreenStateReq]slot: %d", getSlotId());
    requestToRild(message);
    if (getSlotId() == getNonSlotScopeStatusManager()->getIntValue(
            RFX_STATUS_KEY_CDMA_SOCKET_SLOT, 0)) {
        sp<RfxMessage> cdma_screenstate_request =
                RfxMessage::obtainRequest(RADIO_TECH_GROUP_C2K, message->getId(), message, true);
        requestToRild(cdma_screenstate_request);
    }
}

void RpCdmaOemController::handleScreenStateRsp(const sp<RfxMessage>& message) {
    RFX_LOG_D(RP_CDMA_OEM_TAG, "[handleScreenStateRsp]slot: %d", getSlotId());
    if (message->getSentOnCdmaCapabilitySlot() == C_SLOT_STATUS_IS_CURRENT_SLOT) {
        sp<RfxMessage> msgInQueue = sp<RfxMessage>(NULL);
        ResponseStatus responseStatus = preprocessResponse(message, msgInQueue,
                            RfxWaitResponseTimedOutCallback(this,
                                        &RpCdmaOemController::responseSetScreenState), s2ns(10));
        if (responseStatus == RESPONSE_STATUS_HAVE_MATCHED ||
                    responseStatus == RESPONSE_STATUS_HAVE_MATCHED_WITH_C_CHANGED) {
            RFX_LOG_D(RP_CDMA_OEM_TAG, "[handleScreenStateRsp]Response matched");
            if (message->getError() != RIL_E_SUCCESS) {
                responseToRilj(message);
            } else {
                responseToRilj(msgInQueue);
            }
        }
    } else {
        responseToRilj(message);
    }
}

void RpCdmaOemController::responseSetScreenState(const sp<RfxMessage>& message){
    responseToRilj(message);
}

void RpCdmaOemController::requestSetTrm(const sp<RfxMessage>& message) {
    Parcel *p = message->getParcel();
    int pos = p->dataPosition();
    int32_t type = p->readInt32();
    int32_t mode = p->readInt32();
    p->setDataPosition(pos);

    RFX_LOG_D(RP_CDMA_OEM_TAG, "[RpCdmaOemController] requestSetTrm1"
        "type = %d mode = %d", type, mode);

    if (judgeTrmMode(mode) == GSM_TRM_MODE) {
        RFX_LOG_D(RP_CDMA_OEM_TAG, "[RpCdmaOemController] requestSetTrm"
            "to GSMa");
        sp<RfxMessage> gsm_trm_request =
                RfxMessage::obtainRequest(RADIO_TECH_GROUP_GSM, RIL_REQUEST_SET_TRM, message, true);
        requestToRild(gsm_trm_request);
    } else if (judgeTrmMode(mode) == CDMA_TRM_MODE) {
        int slotId = message->getSlotId();
        int cdma_socket_id = getNonSlotScopeStatusManager()->getIntValue(RFX_STATUS_KEY_CDMA_SOCKET_SLOT, 0);
        RFX_LOG_D(RP_CDMA_OEM_TAG, "[requestSetTrm]"
                " slotId = %d, cdma_socket_id = %d", slotId, cdma_socket_id);
        if (slotId != cdma_socket_id) {
            RFX_LOG_D(RP_CDMA_OEM_TAG, "[requestSetTrm]"
                    " slotId =! cdma_socket_id");
            sp<RfxMessage> un_cdma_socket_request =
                    RfxMessage::obtainResponse(RIL_E_GENERIC_FAILURE, message, false);
            responseToRilj(un_cdma_socket_request);

        } else {
            RFX_LOG_D(RP_CDMA_OEM_TAG, "[RpCdmaOemController] requestSetTrm"
                "to CDMAa");
            sp<RfxMessage> cdma_trm_request =
                    RfxMessage::obtainRequest(RADIO_TECH_GROUP_C2K, RIL_REQUEST_SET_TRM, message, true);
            requestToRild(cdma_trm_request);
        }
    } else {
        RFX_LOG_D(RP_CDMA_OEM_TAG, "[RpCdmaOemController] requestSetTrm mode not support");
    }
}

/**
 * 0 means is GSM trm mode.
 * 1 means is CDMA trm mode.
 * 2 means other.
 */
int RpCdmaOemController::judgeTrmMode(int mode) {
    switch (mode) {
        case 1:
        case 2:
        case 3:
        case 5:
        case 6:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
        case 101:
            return 0;
            break;
        case 31:
        case 32:
        case 103:
            return 1;
            break;
        default:
            return 2;
            break;
    }
}

void RpCdmaOemController::responseSetTrm(const sp<RfxMessage>& message) {
    RFX_LOG_D(RP_CDMA_OEM_TAG, "[RpCdmaOemController]"
        " responseSetTrm ");
    responseToRilj(message);
}
void RpCdmaOemController::requestSetUnsolResponseFilter(const sp<RfxMessage>& message) {
    int slotId = message->getSlotId();
    int cdma_socket_id =
            getNonSlotScopeStatusManager()->getIntValue(RFX_STATUS_KEY_CDMA_SOCKET_SLOT, 0);
    RFX_LOG_D(RP_CDMA_OEM_TAG, "[requestSetUnsolResponseFilter]"
            " slotId = %d, cdma_socket_id = %d", slotId, cdma_socket_id);

    if (slotId != cdma_socket_id) {
       RFX_LOG_D(RP_CDMA_OEM_TAG, "[requestSetUnsolResponseFilter]"
               " slotId =! cdma_socket_id");
        requestToRild(message);
    } else {
        sp<RfxMessage> gsm_request =
                RfxMessage::obtainRequest(RADIO_TECH_GROUP_GSM,
                RIL_REQUEST_SET_UNSOLICITED_RESPONSE_FILTER, message, true);
        sp<RfxMessage> cdma_request =
                RfxMessage::obtainRequest(RADIO_TECH_GROUP_C2K,
                RIL_REQUEST_SET_UNSOLICITED_RESPONSE_FILTER, message, true);
        RFX_LOG_D(RP_CDMA_OEM_TAG, "[requestSetUnsolResponseFilter] send to GSM and CDMA RIL");
        requestToRild(gsm_request);
        requestToRild(cdma_request);
    }
}

void RpCdmaOemController::responseSetUnsolResponseFilter(const sp<RfxMessage>& message) {
    sp<RfxMessage> msg = sp<RfxMessage>(NULL);
    if (message->getSentOnCdmaCapabilitySlot() !=  C_SLOT_STATUS_IS_CURRENT_SLOT) {
        RFX_LOG_D(RP_CDMA_OEM_TAG, "[responseSetUnsolResponseFilter] return for GSM");
        responseToRilj(message);
        return;
    }
    ResponseStatus responseStatus = preprocessResponse(message, msg,
        RfxWaitResponseTimedOutCallback(this, &RpCdmaOemController::responseCallBack), s2ns(30));
    if (responseStatus == RESPONSE_STATUS_HAVE_MATCHED ||
            responseStatus == RESPONSE_STATUS_HAVE_MATCHED_WITH_C_CHANGED) {
        RFX_LOG_D(RP_CDMA_OEM_TAG,
                "[responseSetUnsolResponseFilter] Source %d result: %d; Source %d result: %d",
                message->getSource(), message->getError(), msg->getSource(), msg->getError());
        if (message->getError() != RIL_E_SUCCESS) {
            responseToRilj(message);
        } else if (msg->getError() != RIL_E_SUCCESS) {
            responseToRilj(msg);
        } else {
            responseToRilj(message);
        }
    }
}

void RpCdmaOemController::requestSendDeviceState(const sp<RfxMessage>& message) {
    int slotId = message->getSlotId();
    int cdma_socket_id =
            getNonSlotScopeStatusManager()->getIntValue(RFX_STATUS_KEY_CDMA_SOCKET_SLOT, 0);
    RFX_LOG_D(RP_CDMA_OEM_TAG, "[requestSendDeviceState]"
            " slotId = %d, cdma_socket_id = %d", slotId, cdma_socket_id);

    if (slotId != cdma_socket_id) {
       RFX_LOG_D(RP_CDMA_OEM_TAG, "[requestSendDeviceState]"
               " slotId =! cdma_socket_id");
        requestToRild(message);
    } else {
        sp<RfxMessage> gsm_request =
                RfxMessage::obtainRequest(RADIO_TECH_GROUP_GSM,
                RIL_REQUEST_SEND_DEVICE_STATE, message, true);
        sp<RfxMessage> cdma_request =
                RfxMessage::obtainRequest(RADIO_TECH_GROUP_C2K,
                RIL_REQUEST_SEND_DEVICE_STATE, message, true);
        RFX_LOG_D(RP_CDMA_OEM_TAG, "[requestSendDeviceState] send to GSM and CDMA RIL");
        requestToRild(gsm_request);
        requestToRild(cdma_request);
    }
}

void RpCdmaOemController::responseSendDeviceState(const sp<RfxMessage>& message) {
    sp<RfxMessage> msg = sp<RfxMessage>(NULL);
    if (message->getSentOnCdmaCapabilitySlot() !=  C_SLOT_STATUS_IS_CURRENT_SLOT) {
        RFX_LOG_D(RP_CDMA_OEM_TAG, "[responseSendDeviceState] return for GSM");
        responseToRilj(message);
        return;
    }
    ResponseStatus responseStatus = preprocessResponse(message, msg,
        RfxWaitResponseTimedOutCallback(this, &RpCdmaOemController::responseCallBack), s2ns(30));
    if (responseStatus == RESPONSE_STATUS_HAVE_MATCHED ||
            responseStatus == RESPONSE_STATUS_HAVE_MATCHED_WITH_C_CHANGED) {
        RFX_LOG_D(RP_CDMA_OEM_TAG,
                "[responseSendDeviceState] Source %d result: %d; Source %d result: %d",
                message->getSource(), message->getError(), msg->getSource(), msg->getError());
        if (message->getError() != RIL_E_SUCCESS) {
            responseToRilj(message);
        } else if (msg->getError() != RIL_E_SUCCESS) {
            responseToRilj(msg);
        } else {
            responseToRilj(message);
        }
    }
}

bool RpCdmaOemController::handleExtraState(const sp<RfxMessage>& message) {
    Parcel *p = message->getParcel();
    int pos = p->dataPosition();
    // Add local variable here not use String8(s16).string() to avoid temp object free issue.
    String8 str = String8(p->readString16());
    if (strstr(str, RPC_OEM_STRING_STATUS_SYNC) != NULL) {
        str = String8(p->readString16());
        p->setDataPosition(pos);
        RFX_LOG_D(RP_CDMA_OEM_TAG, "handleExtraState, STATUS_SYNC, CMD: %s", str.string());
        getStatusManager()->setString8Value(RFX_STATUS_KEY_TELEPHONY_ASSISTANT_STATUS, str);
        return true;
    }
    p->setDataPosition(pos);
    return false;
}
