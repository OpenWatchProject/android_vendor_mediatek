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
#include "RtcOemController.h"
#include "RfxIntsData.h"
#include "RfxStringsData.h"
#include "RfxVoidData.h"
#include <libmtkrilutils.h>
/*****************************************************************************
 * Class RfxController
 *****************************************************************************/

#define RFX_LOG_TAG "RpOemCtlr"
#define ARRAY_LENGTH(array) (sizeof(array)/sizeof(array[0]))
#define RTC_OEM_STRING_STATUS_SYNC "STATUS_SYNC"
#define RTC_OEM_STRING_SCREEN_STATE_ON "SCREEN_STATE_ON"
#define RTC_OEM_STRING_SCREEN_STATE_OFF "SCREEN_STATE_OFF"

RFX_IMPLEMENT_CLASS("RtcOemController", RtcOemController, RfxController);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_SET_TX_POWER_STATUS);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringsData, RfxVoidData, RFX_MSG_REQUEST_SEND_SAR_IND);

RtcOemController::RtcOemController() {
}

RtcOemController::~RtcOemController() {
}

void RtcOemController::onInit() {
    RfxController::onInit();
    logD(RFX_LOG_TAG, "init()");
    static const int request[] = {
        RFX_MSG_REQUEST_GET_HARDWARE_CONFIG,
        RFX_MSG_REQUEST_GET_IMEI,
        RFX_MSG_REQUEST_GET_IMEISV,
        RFX_MSG_REQUEST_DEVICE_IDENTITY,
        RFX_MSG_REQUEST_BASEBAND_VERSION,
        RFX_MSG_REQUEST_OEM_HOOK_RAW,
        RFX_MSG_REQUEST_OEM_HOOK_STRINGS,
        RFX_MSG_REQUEST_SET_TRM,
        RFX_MSG_REQUEST_GET_ACTIVITY_INFO,
        RFX_MSG_REQUEST_QUERY_MODEM_THERMAL,
        RFX_MSG_REQUEST_SET_TX_POWER_STATUS,
        RFX_MSG_REQUEST_GET_PHONE_CAPABILITY,
        RFX_MSG_REQUEST_ENABLE_DSDA_INDICATION,
        RFX_MSG_REQUEST_GET_DSDA_STATUS,
        RFX_MSG_REQUEST_ROUTE_CERTIFICATE,
        RFX_MSG_REQUEST_ROUTE_AUTH,
        RFX_MSG_REQUEST_ENABLE_CAPABILITY,
        RFX_MSG_REQUEST_ABORT_CERTIFICATE,
        RFX_MSG_REQUEST_QUERY_CAPABILITY,
        RFX_MSG_REQUEST_SEND_SAR_IND,
    };

    registerToHandleRequest(request, ARRAY_LENGTH(request));

    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_TELEPHONY_ASSISTANT_STATUS,
            RfxStatusChangeCallback(this, &RtcOemController::onTelephonyAssistantStatusChanged));
}

bool RtcOemController::onCheckIfRejectMessage(const sp<RfxMessage>& message,
        bool isModemPowerOff,int radioState) {
    if (message->getId() == RFX_MSG_REQUEST_GET_PHONE_CAPABILITY) {
        return false;
    }
    if(radioState == (int)RADIO_STATE_OFF && (
            message->getId() == RFX_MSG_REQUEST_GET_HARDWARE_CONFIG
            || message->getId() == RFX_MSG_REQUEST_GET_IMEI
            || message->getId() == RFX_MSG_REQUEST_GET_IMEISV
            || message->getId() == RFX_MSG_REQUEST_DEVICE_IDENTITY
            || message->getId() == RFX_MSG_REQUEST_BASEBAND_VERSION
            || message->getId() == RFX_MSG_REQUEST_OEM_HOOK_RAW
            || message->getId() == RFX_MSG_REQUEST_OEM_HOOK_STRINGS
            || message->getId() == RFX_MSG_REQUEST_SET_TRM
            || message->getId() == RFX_MSG_REQUEST_GET_ACTIVITY_INFO
            || message->getId() == RFX_MSG_REQUEST_QUERY_MODEM_THERMAL
            || message->getId() == RFX_MSG_REQUEST_SET_TX_POWER_STATUS
            || message->getId() == RFX_MSG_REQUEST_ENABLE_DSDA_INDICATION
            || message->getId() == RFX_MSG_REQUEST_GET_DSDA_STATUS
            || message->getId() == RFX_MSG_REQUEST_ROUTE_CERTIFICATE
            || message->getId() == RFX_MSG_REQUEST_ROUTE_AUTH
            || message->getId() == RFX_MSG_REQUEST_ENABLE_CAPABILITY
            || message->getId() == RFX_MSG_REQUEST_ABORT_CERTIFICATE
            || message->getId() == RFX_MSG_REQUEST_QUERY_CAPABILITY
            )) {
        return false;
    }
    return RfxController::onCheckIfRejectMessage(message, isModemPowerOff, radioState);
}

bool RtcOemController::onHandleRequest(const sp<RfxMessage>& message) {
    int request = message->getId();
    switch(request) {
        case RFX_MSG_REQUEST_OEM_HOOK_RAW:
            onHandleOemHookRaw(message);
            break;
        case RFX_MSG_REQUEST_OEM_HOOK_STRINGS:
            onHandleOemHookStrings(message);
            break;
        case RFX_MSG_REQUEST_GET_ACTIVITY_INFO:
            onHandleGetActivityInfo(message);
            break;
        default:
            requestToMcl(message);
            break;
    }
    return true;
}

bool RtcOemController::onHandleResponse(const sp<RfxMessage>& message) {
    responseToRilj(message);
    return true;
}

void RtcOemController::onHandleOemHookRaw(const sp<RfxMessage>& message) {
    char* data = (char *) message->getData()->getData();
    int index = needToHidenLog(data);
    if (index >= 0) {
        RFX_LOG_D(RFX_LOG_TAG, "onHandleOemHookRaw, data = %s ***", getHidenLogPreFix(index));
    } else {
        RFX_LOG_D(RFX_LOG_TAG, "onHandleOemHookRaw, data = %s", data);
    }
    if (isMainProtocolCommand(data)) {
        requestToMcl(message, true);
    } else {
        requestToMcl(message);
    }
}

void RtcOemController::onHandleOemHookStrings(const sp<RfxMessage>& message) {
    char** data = (char **) message->getData()->getData();
    int index = needToHidenLog(data[0]);
    if (index >= 0) {
        RFX_LOG_D(RFX_LOG_TAG, "onHandleOemHookString, data[0] = %s **", getHidenLogPreFix(index));
    } else {
        RFX_LOG_D(RFX_LOG_TAG, "onHandleOemHookString, data[0] = %s", data[0]);
    }

    if (handleExtraState(message)) {
        sp<RfxMessage> responseMsg = RfxMessage::obtainResponse(RIL_E_SUCCESS, message,
                true);
        responseToRilj(responseMsg);
        return;
    } else if (isMainProtocolCommand(data[0])) {
        requestToMcl(message, true);
    } else {
        requestToMcl(message);
    }
}

void RtcOemController::onHandleGetActivityInfo(const sp<RfxMessage>& message) {
    RFX_LOG_D(RFX_LOG_TAG, "onHandleGetActivityInfo");
    requestToMcl(message, true);
}

bool RtcOemController::isMainProtocolCommand(char* target) {
    if (strstr(target, "AT+EFUN") != NULL ||
            strstr(target, "AT+ESIMMAP") != NULL) {
        return true;
    }
    return false;
}

bool RtcOemController::handleExtraState(const sp<RfxMessage>& message) {
    char** data = (char **) message->getData()->getData();
    if (strstr(data[0], RTC_OEM_STRING_STATUS_SYNC) != NULL) {
        String8 str(data[1]);
        getStatusManager()->setString8Value(RFX_STATUS_KEY_TELEPHONY_ASSISTANT_STATUS, str);
        return true;
    }
    return false;
}

void RtcOemController::onTelephonyAssistantStatusChanged(RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    RFX_UNUSED(old_value);
    String8 newValue = value.asString8();
    int screen_state = -1;
    if (strstr(newValue, RTC_OEM_STRING_SCREEN_STATE_ON) != NULL) {
        screen_state = 1;
    } else if (strstr(newValue, RTC_OEM_STRING_SCREEN_STATE_OFF) != NULL) {
        screen_state = 0;
    }
    if (screen_state != -1) {
        getStatusManager()->setIntValue(RFX_STATUS_KEY_SCREEN_STATE, screen_state);
    }
}
