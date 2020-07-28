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
#include "RtcRadioController.h"
#include "RfxMainThread.h"
#include "utils/RefBase.h"
#include "utils/Errors.h"
#include <string.h>
#include "RfxVoidData.h"
#include "RfxIntsData.h"
#include "RfxRilUtils.h"

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/

#define RFX_LOG_TAG "RtcRadioCont"
#define ARRAY_LENGTH(array) (sizeof(array)/sizeof(array[0]))

static const int INVALID_VALUE = -1;
static const int RETRY_TIME_MS = 500;

extern "C" void setRadioState(RIL_RadioState newState, RIL_SOCKET_ID rid);


RFX_IMPLEMENT_CLASS("RtcRadioController", RtcRadioController, RfxController);

RtcRadioController::RtcRadioController() {
}

RtcRadioController::~RtcRadioController() {
}

void RtcRadioController::onInit() {
    RfxController::onInit();
    logD(RFX_LOG_TAG, "init()");

    static const int request[] = {
        RFX_MSG_REQUEST_RADIO_POWER,
        RFX_MSG_REQUEST_COMMAND_BEFORE_RADIO_POWER,
        RFX_MSG_REQUEST_BOOT_TURN_ON_RADIO,
        RFX_MSG_REQUEST_ENABLE_MODEM,
    };

    getStatusManager()->setIntValue(RFX_STATUS_KEY_RADIO_STATE, RADIO_STATE_UNAVAILABLE);
    registerToHandleRequest(request, ARRAY_LENGTH(request));
    registerForStatusChange();

    logD(RFX_LOG_TAG, "init():Done");
}

void RtcRadioController::registerForStatusChange() {
    logD(RFX_LOG_TAG, "registerForStatusChange");
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_RADIO_STATE,
        RfxStatusChangeCallback(this, &RtcRadioController::onRadioStateChanged));
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_CARD_TYPE,
        RfxStatusChangeCallback(this, &RtcRadioController::onSimStateChanged));
}

void RtcRadioController::onRadioStateChanged(RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    //sync with rild's state
    int newValue = value.asInt();
    int oldValue = old_value.asInt();
    bool requestPower = getStatusManager()->getBoolValue(RFX_STATUS_KEY_REQUEST_RADIO_POWER, false);
    if ((requestPower == false) && ((RIL_RadioState)newValue == RADIO_STATE_ON) &&
            ((RIL_RadioState)oldValue != RADIO_STATE_UNAVAILABLE)) {
        logI(RFX_LOG_TAG, "radio State: %d -> %d, STATUS_KEY_REQUEST_RADIO_POWER = false not update to RILJ",
                oldValue, newValue);
    } else {
        if (newValue != oldValue) {
            logI(RFX_LOG_TAG, "radio State: %d -> %d, using google native API for urc", oldValue, newValue);
            setRadioState((RIL_RadioState)newValue, (RIL_SOCKET_ID)m_slot_id);
        } else {
            logI(RFX_LOG_TAG, "radio state not change (%d), do not update to RILJ", newValue);
        }
    }
}

void RtcRadioController::onSimStateChanged(RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    //sync with rild's state
    int newValue = value.asInt();
    int oldValue = old_value.asInt();
    for (int i = 0; i < RfxRilUtils::rfxGetSimCount(); i++) {
        if (getStatusManager(i)->getIntValue(RFX_STATUS_KEY_CARD_TYPE, 0) == -1) {
            logE(RFX_LOG_TAG, "onSimStateChanged: only one card ready, unregister %d and return",
                    m_slot_id);
            // unregister
            getStatusManager()->unRegisterStatusChanged(RFX_STATUS_KEY_CARD_TYPE,
                   RfxStatusChangeCallback(this, &RtcRadioController::onSimStateChanged));
            return;
        }
    }

    int simStatus = 0;
    for (int i = 0 ; i < RfxRilUtils::rfxGetSimCount(); i++) {
        simStatus |= ((getStatusManager(i)->getIntValue(RFX_STATUS_KEY_CARD_TYPE, 0)> 0) ? 1 : 0) << i;
    }
    logD(RFX_LOG_TAG, "onSimStateChanged: simStatus %d", simStatus);

    RadioPowerLock radioLock =
            (RadioPowerLock) getStatusManager(m_slot_id)->getIntValue(
            RFX_STATUS_KEY_RADIO_LOCK, RADIO_LOCK_IDLE);

    if (radioLock != RADIO_LOCK_IDLE) {
        logI(RFX_LOG_TAG, "Radio is locked. radioLock: %d", radioLock);
        getStatusManager()->unRegisterStatusChanged(RFX_STATUS_KEY_CARD_TYPE,
                RfxStatusChangeCallback(this, &RtcRadioController::onSimStateChanged));
        return;
    }

    getStatusManager()->setIntValue(RFX_STATUS_KEY_RADIO_LOCK, RADIO_LOCK_BY_RADIO);
    sp<RfxMessage> reqMsg = RfxMessage::obtainRequest(m_slot_id,
            RFX_MSG_REQUEST_BOOT_TURN_ON_RADIO, RfxIntsData(&simStatus, 1));
    mPendingRequest.insert({reqMsg->getPToken(), reqMsg});
    requestToMcl(reqMsg, true);

    // unregister
    logD(RFX_LOG_TAG, "onSimStateChanged: unregister %d", m_slot_id);
    getStatusManager()->unRegisterStatusChanged(RFX_STATUS_KEY_CARD_TYPE,
            RfxStatusChangeCallback(this, &RtcRadioController::onSimStateChanged));
}

bool RtcRadioController::onPreviewMessage(const sp<RfxMessage>& message) {
    if (message->getType() == REQUEST) {
        if (canHandleRequest(message)) {
            return true;
        } else {
            return false;
        }
    } else {
        return true;
    }
}

bool RtcRadioController::onCheckIfResumeMessage(const sp<RfxMessage>& message) {
   if (message->getType() == REQUEST) {
          if (canHandleRequest(message)) {
              return true;
          } else {
              return false;
          }
      } else {
          return true;
      }
}

bool RtcRadioController::canHandleRequest(const sp<RfxMessage>& message) {
    RFX_UNUSED(message);
    RadioPowerLock radioLock = (RadioPowerLock) getStatusManager()
            ->getIntValue(RFX_STATUS_KEY_RADIO_LOCK, RADIO_LOCK_IDLE);
    int dsbpEnhancementStatus = getStatusManager()->getIntValue(
            RFX_STATUS_KEY_DSBP_ENHANCEMENT_STATE, DSBP_ENHANCEMENT_END);
    if (radioLock) {
        logD(RFX_LOG_TAG, "radio is locked by other modul, %d", (int) radioLock);
        return false;
    } else if (DSBP_ENHANCEMENT_START == dsbpEnhancementStatus) {
        logD(RFX_LOG_TAG, "modem executes DSBP procedure, pending radio request");
        return false;
    }
    return true;
}

bool RtcRadioController::onHandleRequest(const sp<RfxMessage>& message) {
    int id = message->getId();
    logD(RFX_LOG_TAG, "onHandleRequest: %s(%d)", idToString(id), id);
    switch (id) {
    case RFX_MSG_REQUEST_RADIO_POWER:
    case RFX_MSG_REQUEST_ENABLE_MODEM:
        requestRadioPower(message);
        break;
    default:
        break;
    }
    return true;
}

void RtcRadioController::requestRadioPower(const sp<RfxMessage>& message) {
    // update radio lock
    getStatusManager()->setIntValue(RFX_STATUS_KEY_RADIO_LOCK, RADIO_LOCK_BY_RADIO);

    int power = ((int *)message->getData()->getData())[0];
    logD(RFX_LOG_TAG, "requestRadioPower, slotId:%d, onOff: %d", m_slot_id, power);

    bool modemPowerOff = getStatusManager()->getBoolValue(RFX_STATUS_KEY_MODEM_POWER_OFF, false);
    bool waitToRestart = getNonSlotScopeStatusManager()->getBoolValue(
            RFX_STATUS_KEY_HAD_POWER_OFF_MD, false);
    if (modemPowerOff || waitToRestart) {
        logD(RFX_LOG_TAG, "modemPowerOff or RILD wait to restart, just response to RILJ");
        sp<RfxMessage> response = RfxMessage::obtainResponse(RIL_E_RADIO_NOT_AVAILABLE, message);
        responseToRilj(response);
        return;
    }

    // op-decouple
    sp<RfxMessage> preMsg = RfxMessage::obtainRequest(m_slot_id,
            RFX_MSG_REQUEST_COMMAND_BEFORE_RADIO_POWER, RfxIntsData(&power, 1));
    requestToMcl(preMsg);

    if (power) {
        getStatusManager()->setBoolValue(RFX_STATUS_KEY_REQUEST_RADIO_POWER, true);
    } else {
        getStatusManager()->setBoolValue(RFX_STATUS_KEY_REQUEST_RADIO_POWER, false);
    }

    // EFUN enhancement. always add cause to radio off request
    if (power) {
        mPendingRequest.insert({message->getPToken(), message});
        requestToMcl(message, true);
    } else {
        int content[2] = {power, RFOFF_CAUSE_POWER_OFF};
        sp<RfxMessage> sendMsg = RfxMessage::obtainRequest(message->getId(),
                RfxIntsData(content, 2), message, false);
        mPendingRequest.insert({sendMsg->getPToken(), sendMsg});
        requestToMcl(sendMsg, true);
    }
}

bool RtcRadioController::onHandleUrc(const sp<RfxMessage>& message) {
    RFX_UNUSED(message);
    return true;
}

bool RtcRadioController::onHandleResponse(const sp<RfxMessage>& message) {
    int id = message->getId();
    logD(RFX_LOG_TAG, "onHandleResponse: %s(%d)", idToString(id), id);
    switch (id) {
    case RFX_MSG_REQUEST_RADIO_POWER:
    case RFX_MSG_REQUEST_BOOT_TURN_ON_RADIO:
    case RFX_MSG_REQUEST_ENABLE_MODEM:
        handleRadioPowerResponse(message);
        break;
    case RFX_MSG_REQUEST_COMMAND_BEFORE_RADIO_POWER:
        // do nothing
        break;
    default:
        break;
    }
    return true;
}

bool RtcRadioController::handleRadioPowerResponse(const sp<RfxMessage>& message) {
    // update radio lock
    getStatusManager()->setIntValue(RFX_STATUS_KEY_RADIO_LOCK, RADIO_LOCK_IDLE);
    RIL_Errno rilErrno = message->getError();

    sp<RfxMessage> retryMsg = findPendingRequest(mPendingRequest, message);
    sp<RfxAction> action = findAction(mActionMap, message);
    if (RIL_E_SUCCESS != rilErrno) {
        // EFUN may error when ERAT processing, need retry
        getStatusManager()->setIntValue(RFX_STATUS_KEY_RADIO_LOCK, RADIO_LOCK_BY_RADIO);
        if ((sp<RfxAction>) NULL != action) {
            mActionMap.insert({retryMsg->getToken(), action});
        }
        mPendingRequest.insert({retryMsg->getPToken(), retryMsg});
        requestToMcl(retryMsg, true, ms2ns(RETRY_TIME_MS));
        return false;
    } else if ((sp<RfxAction>) NULL != action) {
        action->act();
        return true;
    }
    responseToRilj(message);
    return true;
}

bool RtcRadioController::onCheckIfRejectMessage(const sp<RfxMessage>& message,
        bool isModemPowerOff,int radioState) {
    logD(RFX_LOG_TAG, "onCheckIfRejectMessage, id = %d, isModemPowerOff = %d, rdioState = %d",
            message->getId(), isModemPowerOff, radioState);
    int id = message->getId();
    if (RFX_MSG_REQUEST_RADIO_POWER == id ||
            RFX_MSG_REQUEST_COMMAND_BEFORE_RADIO_POWER == id ||
            RFX_MSG_REQUEST_BOOT_TURN_ON_RADIO == id ||
            RFX_MSG_REQUEST_ENABLE_MODEM == id) {
        return false;
    }
    return true;
}

sp<RfxMessage> RtcRadioController::findPendingRequest(std::unordered_map<int, sp<RfxMessage>>&
        pendingRequest, const sp<RfxMessage>& msg) {
    std::unordered_map<int, sp<RfxMessage>>::const_iterator result =
            pendingRequest.find(msg->getPToken());

    if (result == pendingRequest.end()) {
        logD(RFX_LOG_TAG, "findPendingRequest: do not find corresponding request");
        RFX_ASSERT("Should not be here");
        return NULL;
    }
    sp<RfxMessage> requestMsg = result->second;
    pendingRequest.erase(msg->getPToken());
    logD(RFX_LOG_TAG, "findPendingRequest: find request, PToken = %d", requestMsg->getPToken());
    return requestMsg;
}

sp<RfxAction> RtcRadioController::findAction(std::unordered_map<int, sp<RfxAction>>& actionMap,
        const sp<RfxMessage>& msg) {
    std::unordered_map<int, sp<RfxAction>>::const_iterator result =
            actionMap.find(msg->getToken());

    if (result == actionMap.end()) {
        return NULL;
    }
    sp<RfxAction> action = result->second;
    actionMap.erase(msg->getToken());
    logD(RFX_LOG_TAG, "findAction: find request, PToken = %d", msg->getPToken());
    return action;
}

void RtcRadioController::moduleRequestRadioPower(bool power, const sp<RfxAction>& action,
        RadioCause cause) {
    logD(RFX_LOG_TAG, "moduleRequestRadioPower: slot%d, requestPower:%d, caller: %d",
            m_slot_id, power, cause);
    int desirePower[2] = {power ? 1 : 0, (int) cause};
    sp<RfxMessage> radioRequest = RfxMessage::obtainRequest(m_slot_id, RFX_MSG_REQUEST_RADIO_POWER,
            RfxIntsData(desirePower, 2));
    if ((sp<RfxAction>) NULL != action) {
        mActionMap.insert({radioRequest->getToken(), action});
    }

    // To avoid conflict with request from framework, we should execute request directly
    // update radio lock
    getStatusManager()->setIntValue(RFX_STATUS_KEY_RADIO_LOCK, RADIO_LOCK_BY_RADIO);
    bool modemPowerOff = getStatusManager()->getBoolValue(RFX_STATUS_KEY_MODEM_POWER_OFF, false);
    if (modemPowerOff) {
        logD(RFX_LOG_TAG, "modemPowerOff, do not execute moduleRequestRadioPower");
        return;
    }
    mPendingRequest.insert({radioRequest->getPToken(), radioRequest});
    requestToMcl(radioRequest, true);
}
