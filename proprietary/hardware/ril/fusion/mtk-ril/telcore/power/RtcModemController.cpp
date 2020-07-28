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
#include "RtcModemController.h"
#include "RfxVoidData.h"

#define RFX_LOG_TAG "RpMDCtrl"
#define ARRAY_LENGTH(array) (sizeof(array)/sizeof(array[0]))

/*****************************************************************************
 * Class RtcModemController
 *****************************************************************************/

RFX_IMPLEMENT_CLASS("RtcModemController", RtcModemController, RfxController);

void RtcModemController::onInit() {
    logD(RFX_LOG_TAG, "Init()");
    RfxController::onInit();

    static const int request[] = {
        RFX_MSG_REQUEST_MODEM_POWERON,
        RFX_MSG_REQUEST_MODEM_POWEROFF,
        RFX_MSG_REQUEST_RESET_RADIO,
        RFX_MSG_REQUEST_RESTART_RILD,
        RFX_MSG_REQUEST_SHUTDOWN,
    };

    for (int i = 0; i < RFX_SLOT_COUNT; i++) {
        registerToHandleRequest(i, request, ARRAY_LENGTH(request));
    }
}

void RtcModemController::onDeinit() {
    RfxController::onDeinit();
}

RtcModemController::RtcModemController() {
}


RtcModemController::~RtcModemController() {
}

bool RtcModemController::onPreviewMessage(const sp<RfxMessage>& message) {
    if (REQUEST == message->getType()) {
        if (canHandleRequest(message)) {
            return true;
        } else {
            return false;
        }
    } else {
        logD(RFX_LOG_TAG, "Not Request, don't do check");
        return true;
    }
}

bool RtcModemController::onCheckIfResumeMessage(const sp<RfxMessage>& message) {
    if (REQUEST == message->getType()) {
        if (canHandleRequest(message)) {
            return true;
        } else {
            return false;
        }
    } else {
        return true;
    }
}

bool RtcModemController::canHandleRequest(const sp<RfxMessage>& message __unused) {
    // Avoid MD power off conflict
    int modemOffState = getNonSlotScopeStatusManager()->getIntValue(RFX_STATUS_KEY_MODEM_OFF_STATE,
            MODEM_OFF_IN_IDLE);
    if (modemOffState != MODEM_OFF_IN_IDLE && modemOffState != MODEM_OFF_BY_POWER_OFF) {
            logD(RFX_LOG_TAG, "canHandleRequest: false, modemOffState=%d",
                    modemOffState);
        return false;
    }
    logD(RFX_LOG_TAG, "canHandleRequest: true");
    return true;
}

bool RtcModemController::onHandleRequest(const sp<RfxMessage>& message) {
    int msgId = message->getId();
    logD(RFX_LOG_TAG, "RtcModemController: handle request id %d", msgId);

    // Avoid MD power off conflict
    int modemOffState = getNonSlotScopeStatusManager()->getIntValue(RFX_STATUS_KEY_MODEM_OFF_STATE,
            MODEM_OFF_IN_IDLE);
    int mainSlot = getNonSlotScopeStatusManager()->getIntValue(
            RFX_STATUS_KEY_MAIN_CAPABILITY_SLOT, 0);

    switch (msgId) {
    case RFX_MSG_REQUEST_MODEM_POWERON:
        requestModemPower(true, message);
        break;
    case RFX_MSG_REQUEST_MODEM_POWEROFF:
    case RFX_MSG_REQUEST_SHUTDOWN:
        if (msgId == RFX_MSG_REQUEST_SHUTDOWN &&
                message->getSlotId() != mainSlot) {
            logI(RFX_LOG_TAG,
                    "no need to handle RFX_MSG_REQUEST_SHUTDOWN in non-main capability slot");
            sp<RfxMessage> responseMsg = RfxMessage::obtainResponse(RIL_E_SUCCESS, message, true);
            responseToRilj(responseMsg);
        } else {
            logI(RFX_LOG_TAG,
                    "do shutdown on main capability slot");
            if (modemOffState == MODEM_OFF_IN_IDLE) {
                getNonSlotScopeStatusManager()->setIntValue(RFX_STATUS_KEY_MODEM_OFF_STATE,
                        MODEM_OFF_BY_POWER_OFF);
                logD(RFX_LOG_TAG, "RtcModemController: set RFX_STATUS_KEY_MODEM_OFF_STATE to %d",
                        MODEM_OFF_BY_POWER_OFF);
            }
            getStatusManager()->setBoolValue(RFX_STATUS_KEY_HAD_POWER_OFF_MD, true);
            requestModemPower(false, message);
        }
        break;
    case RFX_MSG_REQUEST_RESET_RADIO:
    case RFX_MSG_REQUEST_RESTART_RILD:
        if (modemOffState == MODEM_OFF_IN_IDLE) {
            getNonSlotScopeStatusManager()->setIntValue(RFX_STATUS_KEY_MODEM_OFF_STATE,
                    MODEM_OFF_BY_RESET_RADIO);
            logD(RFX_LOG_TAG, "RtcModemController: set RFX_STATUS_KEY_MODEM_OFF_STATE to %d",
                    MODEM_OFF_BY_RESET_RADIO);
            requestToMcl(message, true);
        }
        break;
    default:
        break;
    }
    return true;
}

void RtcModemController::requestModemPower(bool power, const sp<RfxMessage>& message) {
    int slotId = message->getSlotId();

    logD(RFX_LOG_TAG, "requestModemPower, slotId=%d, power=%d", slotId, power);
    requestToMcl(message, true);
}

bool RtcModemController::onHandleResponse(const sp<RfxMessage>& message) {
    int msgId = message->getId();
    logD(RFX_LOG_TAG, "RtcModemController: handle response id %d", msgId);

    switch (msgId) {
    case RFX_MSG_REQUEST_MODEM_POWERON:
        responseModemPower(message);
        break;
    case RFX_MSG_REQUEST_MODEM_POWEROFF:
    case RFX_MSG_REQUEST_SHUTDOWN:
        responseModemPower(message);
        break;
    case RFX_MSG_REQUEST_RESET_RADIO:
    case RFX_MSG_REQUEST_RESTART_RILD:
        responseToRilj(message);
        break;
    default:
        break;
    }
    return true;
}

bool RtcModemController::onCheckIfRejectMessage(const sp<RfxMessage>& message,
        bool isModemPowerOff,int radioState) {
    logD(RFX_LOG_TAG, "onCheckIfRejectMessage, id = %d, isModemPowerOff = %d, rdioState = %d",
            message->getId(), isModemPowerOff, radioState);
    // always execute modem power related request
    return false;
}

void RtcModemController::responseModemPower(const sp<RfxMessage>& message) {
    RIL_Errno error = message->getError();
    // avoid send request eatly than modem handler constructor
    if (RIL_E_REQUEST_NOT_SUPPORTED == error) {
        sp<RfxMessage> retryMsg = RfxMessage::obtainRequest(message->getId(), RfxVoidData(),
                message, false);
        requestModemPower((RFX_MSG_REQUEST_MODEM_POWERON == message->getId())? true: false,
                retryMsg);
        return;
    }
    getNonSlotScopeStatusManager()->setIntValue(RFX_STATUS_KEY_MODEM_OFF_STATE,
            MODEM_OFF_IN_IDLE);
    responseToRilj(message);
}
