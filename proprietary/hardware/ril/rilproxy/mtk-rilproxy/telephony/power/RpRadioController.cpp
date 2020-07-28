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
#include "RpRadioController.h"
#include "RpCdmaRadioController.h"
#include "RadioConstants.h"
#include "Parcel.h"
#include "RfxMainThread.h"
#include "RfxSocketState.h"
#include <log/log.h>
#include "utils/RefBase.h"
#include "utils/Errors.h"
#include <cutils/properties.h>
#include <string.h>
#include "util/RpFeatureOptionUtils.h"
#include "modecontroller/RpCdmaLteDefs.h"

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/
#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "RpRadioCtrl"
#endif
#define LOG_SIZE 500

#define ARRAY_LENGTH(array) (sizeof(array)/sizeof(array[0]))

#define FIRST_TIME_RLOGD(last_step,cur_step,log) if(last_step!=cur_step){RLOGD(log);last_step=cur_step;} // if last_step isn't cur_step, then log it, otherwise jump this log, means only log the 1st time of cur_step.

extern "C" void setRadioState(RIL_RadioState newState, RIL_SOCKET_ID rid);

RFX_IMPLEMENT_CLASS("RpRadioController", RpRadioController, RfxController);

bool RpRadioController::inDynamicSwitchRadioOff = false;
bool RpRadioController::isDynamicSwitchRadioOn = false;

RpRadioController::RpRadioController() {
}

RpRadioController::~RpRadioController() {
}

static const int requestFor6M[] = {
    RIL_REQUEST_RADIO_POWER,
    RIL_REQUEST_ENABLE_MODEM,
};

static const int urc[] = {
    RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED
};

static const int INVALID_VALUE = -1;

void RpRadioController::onInit() {
    RfxController::onInit();
    slotId = getSlotId();
    RLOGD("RpRadioController init(): %d", slotId);

    //WARNING: TEST for CDMA
    //if (slotId == 0) {
    //    getStatusManager() ->
    //        setIntValue(RFX_STATUS_KEY_SUGGESTED_RADIO_CAPABILITY, RIL_CAPABILITY_CDMA_ON_LTE);
    //}

    getStatusManager()->setIntValue(RFX_STATUS_KEY_RADIO_STATE, RADIO_STATE_UNAVAILABLE);
    getStatusManager()->setIntValue(RFX_STATUS_KEY_RADIO_STATE_C2K, RADIO_STATE_UNAVAILABLE);
    if (RpFeatureOptionUtils::isC2kSupport()) {
        registerToHandleRequest(requestFor6M, ARRAY_LENGTH(requestFor6M));
    }
    // To sync radio state with rild, RpRadioController need register RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED
    registerToHandleUrc(urc, ARRAY_LENGTH(urc));
    registerForStatusChange();
    MessageQueue = new RadioMessage();
    inDynamicSwitchRadioOff = false;
    isDynamicSwitchRadioOn = false;
    canHandleErrStep = 0;
    int modemOffState = getNonSlotScopeStatusManager()->getIntValue(RFX_STATUS_KEY_MODEM_OFF_STATE, MODEM_OFF_IN_IDLE);
    isUnderCapabilitySwitch = (modemOffState == MODEM_OFF_BY_SIM_SWITCH) ? true : false;
    RLOGD("RpRadioController init():Done");
}

void RpRadioController::registerForStatusChange() {
    RLOGD("RpRadioController: registerForStatusChange");
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_RADIO_STATE,
        RfxStatusChangeCallback(this, &RpRadioController::onRadioStateChanged));
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_RADIO_CAPABILITY,
        RfxStatusChangeCallback(this, &RpRadioController::onCapabilityChanged));
    getNonSlotScopeStatusManager()->registerStatusChanged(RFX_STATUS_KEY_MODEM_OFF_STATE,
        RfxStatusChangeCallback(this, &RpRadioController::onModemOffStateChanged));
}

void RpRadioController::onRadioStateChanged(RfxStatusKeyEnum key,
    RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    //sync with rild's state
    int newValue = value.asInt();
    int oldValue = old_value.asInt();
    bool requestPower = getStatusManager()->getBoolValue(RFX_STATUS_KEY_REQUEST_RADIO_POWER, false);
    if ((requestPower == false) && ((RIL_RadioState)newValue == RADIO_STATE_ON) &&
            ((RIL_RadioState)oldValue != RADIO_STATE_UNAVAILABLE)) {
        RLOGD("radio State: %d -> %d, STATUS_KEY_REQUEST_RADIO_POWER = false not update to RILJ",
                oldValue, newValue);
    } else {
        //RLOGD("radio State: %d -> %d, using google native API for urc", oldValue, newValue);
        setRadioState((RIL_RadioState)newValue, (RIL_SOCKET_ID)slotId);
    }
}

void RpRadioController::onModemOffStateChanged(RfxStatusKeyEnum key,
    RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    int newValue = value.asInt();
    int oldValue = old_value.asInt();
    if (newValue == MODEM_OFF_BY_SIM_SWITCH) {
        RLOGD("Enter Sim switch state");
        isUnderCapabilitySwitch = true;
    } else {
        RLOGD("Leave Sim switch state");
        isUnderCapabilitySwitch = false;
        int radioState = getStatusManager()->getIntValue(RFX_STATUS_KEY_RADIO_STATE, RADIO_STATE_UNAVAILABLE);
        //if radio is unavailable, no need to set radio on/off according to capability
        if (radioState != RADIO_STATE_UNAVAILABLE) {
            refreshRadioState();
        }
    }
}

void RpRadioController::suggestedCapability(int desiredCapability, RpSuggestRadioCapabilityCallback &callback) {
    int oldCapability = getStatusManager()->getIntValue(RFX_STATUS_KEY_SUGGESTED_RADIO_CAPABILITY, INVALID_VALUE);
    getStatusManager()->setIntValue(RFX_STATUS_KEY_SUGGESTED_RADIO_CAPABILITY, desiredCapability);
    bool requestPower = getStatusManager()->getBoolValue(RFX_STATUS_KEY_REQUEST_RADIO_POWER, false);
    if (!CAPABILITY_HAS_CDMA(desiredCapability) && (oldCapability != desiredCapability)) {
        RpCdmaRadioController::getInstance()->registerForCdmaCapability(slotId, CAPABILITY_HAS_CDMA(desiredCapability));
    }
    if (((desiredCapability == RIL_CAPABILITY_NONE) || (desiredCapability != RIL_CAPABILITY_NONE && requestPower )) &&
            (!inDynamicSwitchRadioOff) && (oldCapability != desiredCapability)) {
        RLOGD("suggestCapability slot[%d] %d -> %d, requestPower:%d, inDynamicSwitchRadioOff:%d",
                slotId, oldCapability, desiredCapability, requestPower, inDynamicSwitchRadioOff);
        sp<RfxMessage> radioRequest =
            RfxMessage::obtainRequest(slotId, RADIO_TECH_GROUP_GSM, RIL_REQUEST_RADIO_POWER);
        radioRequest->getParcel()->writeInt32(1);
        int power = (requestPower) ? 1 : 0;
        radioRequest->getParcel()->writeInt32(power);
        suggestCapabilityMap.insert({radioRequest->getPToken(), callback});
        RfxMainThread::enqueueMessageFront(radioRequest);
    } else {
        /*RLOGD("suggestCapability slot[%d] %d -> %d, requestPower:%d, inDynamicSwitchRadioOff:%d, Should not change to CAPABILITY_NONE",
                slotId, oldCapability, desiredCapability, requestPower, inDynamicSwitchRadioOff);*/
        callback.invoke(SUCCESS_WITHOUT_SETTING_RADIO);
    }
    return;
}

void RpRadioController::onCapabilityChanged(RfxStatusKeyEnum key,
    RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    int oldValue = old_value.asInt();
    int newValue = value.asInt();

    // check if not available
    int radioState = getStatusManager()->getIntValue(RFX_STATUS_KEY_RADIO_STATE, RADIO_STATE_UNAVAILABLE);
    if (radioState == RADIO_STATE_UNAVAILABLE) {
        RLOGD("capability: %d -> %d, not update for RADIO_STATE_UNAVAILABLE", oldValue, newValue);
        return;
    }
    if (newValue == RIL_CAPABILITY_NONE) {
        //RLOGD("capability: %d -> %d, update as RADIO_STATE_OFF", oldValue, newValue);
        getStatusManager()->setIntValue(RFX_STATUS_KEY_RADIO_STATE, RADIO_STATE_OFF);
    } else {
        //RLOGD("capability: %d -> %d, update as RADIO_STATE_ON", oldValue, newValue);
        getStatusManager()->setIntValue(RFX_STATUS_KEY_RADIO_STATE, RADIO_STATE_ON);
    }
    return;
}

bool RpRadioController::onPreviewMessage(const sp<RfxMessage>& message) {
    if (message->getType() == REQUEST) {
        if (canHandleRequest(message)) {
            return true;
        } else {
            return false;
        }
    } else {
        //RLOGD("Not Request, don't do check");
        return true;
    }
}

bool RpRadioController::onCheckIfResumeMessage(const sp<RfxMessage>& message) {
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

bool RpRadioController::canHandleRequest(const sp<RfxMessage>& message) {
    RFX_UNUSED(message);
    /*if (inDynamicSwitchRadioOff) {
        RpSuggestRadioCapabilityCallback callback = checkIfNeedCallback(suggestCapabilityMap, message);
        if (callback.isValid()) {
            RLOGD("pass suggest capablity request inDynamicSwitchRadioOff=true");
            canHandleErrStep = 0;
            return true;
        }
        FIRST_TIME_RLOGD(canHandleErrStep, 1, "[Req QUEUED]inDynamicSwitchRadioOff=true, don't process RADIO_POWER");
        return false;
    }

    //check sim switch
    if (isUnderCapabilitySwitch == true) {
        FIRST_TIME_RLOGD(canHandleErrStep, 2, "[Req QUEUED]Is under sim switch, don't process RADIO_POWER");
        return false;
    }*/

    // check radio state
    int radioState = getStatusManager()->getIntValue(RFX_STATUS_KEY_RADIO_STATE, RADIO_STATE_UNAVAILABLE);
    if (radioState == RADIO_STATE_UNAVAILABLE) {
        FIRST_TIME_RLOGD(canHandleErrStep, 3, "[Req QUEUED]Radio state not available, don't process RADIO_POWER");
        return false;
    }

    // check GSM socket
    RfxSocketState socketState = getStatusManager()->getSocketStateValue(RFX_STATUS_KEY_SOCKET_STATE);
    if (!socketState.getSocketState(RfxSocketState::SOCKET_GSM)) {
        FIRST_TIME_RLOGD(canHandleErrStep, 4, "[Req QUEUED]GSM Socket not connected");
        return false;
    }

    // check CDMA socket & EMDSTATUS if needed
    if (socketState.getIsCdmaSlot()) {
        // RLOGD("canHandleRequest, is cdma slot.");
        if (!socketState.getSocketState(RfxSocketState::SOCKET_C2K)) {
            FIRST_TIME_RLOGD(canHandleErrStep, 5, "[Req QUEUED]CDMA Socket not connected");
            return false;
        }

        char modemStatusIsSend[PROPERTY_VALUE_MAX] = { 0 };
        property_get(emdstatusProperty, modemStatusIsSend, "0");
        if (atoi(modemStatusIsSend) == 0) {
            FIRST_TIME_RLOGD(canHandleErrStep, 6, "[Req QUEUED]AT+EMDSTATUS not send");
            return false;
        }

        if (findRadioMessageSendToCdma())
        {
            FIRST_TIME_RLOGD(canHandleErrStep, 7, "[Req QUEUED]Last request sent to cdma not response yet");
            return false;
        }
    }

    // check suggested radio capability if needed
    int capability = getStatusManager()->getIntValue(RFX_STATUS_KEY_SUGGESTED_RADIO_CAPABILITY, INVALID_VALUE);
    if (capability == INVALID_VALUE) {
        FIRST_TIME_RLOGD(canHandleErrStep, 8, "[Req QUEUED]suggested radio capability is invalid");
        return false;
    }

    canHandleErrStep = 0;
    return true;
}

bool RpRadioController::onHandleRequest(const sp<RfxMessage>& message) {
    int msg_id = message->getId();
    int pToken = message->getPToken();
    // RLOGD("RpRadioController: handle request id %d", msg_id);
    switch (msg_id) {
    case RIL_REQUEST_RADIO_POWER:
    case RIL_REQUEST_ENABLE_MODEM:
        requestRadioPower(message);
        break;
    default:
        break;
    }
    return true;
}

void RpRadioController::requestRadioPower(const sp<RfxMessage>& message) {
    bool lastRequestPower = getStatusManager()->getBoolValue(RFX_STATUS_KEY_REQUEST_RADIO_POWER, false);
    int c2kRadioState = getStatusManager()->getIntValue(RFX_STATUS_KEY_RADIO_STATE_C2K, RADIO_STATE_UNAVAILABLE);
    bool isSwitchCard = getStatusManager()->getBoolValue(RFX_STATUS_KEY_SWITCH_CARD_TYPE_STATUS, false);

    // handle with Parcel
    Parcel* parcel = message->getParcel();
    int32_t status = 0;
    int32_t request = 0;
    int32_t token = 0;
    int32_t argc = 0;
    int32_t power = 0;

    parcel->setDataPosition(0);
    status = parcel->readInt32(&request);
    status = parcel->readInt32(&token);
    status = parcel->readInt32(&argc);
    status = parcel->readInt32(&power);
    parcel->setDataPosition(0);

    RLOGD("requestRadioPower, slotId:%d, onOff: %d", slotId, power);

    if (inDynamicSwitchRadioOff || isUnderCapabilitySwitch || isSwitchCard) {
        RpSuggestRadioCapabilityCallback callback = checkIfNeedCallback(suggestCapabilityMap, message);
        int suggestedCapability;
        if (power) {
            suggestedCapability = getStatusManager()->
                        getIntValue(RFX_STATUS_KEY_SUGGESTED_RADIO_CAPABILITY, RIL_CAPABILITY_NONE);
        } else {
            suggestedCapability = RIL_CAPABILITY_NONE;
        }
        //Also need to change CDMA Capability
        RpCdmaRadioController::getInstance()->registerForCdmaCapability(slotId, CAPABILITY_HAS_CDMA(suggestedCapability));
        if (callback.isValid()) {
            callback.invoke(SUCCESS_WITHOUT_SETTING_RADIO);
            RLOGD("Under sim switch, invoke callback without setting radio");
        } else {
            if (power) {
                getStatusManager()->setBoolValue(RFX_STATUS_KEY_REQUEST_RADIO_POWER, true);
            } else {
                getStatusManager()->setBoolValue(RFX_STATUS_KEY_REQUEST_RADIO_POWER, false);
            }
            RLOGD("Under sim switch, dont process RADIO_POWER to rild");
            sp<RfxMessage> response = RfxMessage::obtainResponse(RIL_E_ABORTED, message);
            responseToRilj(response);
        }
        return;
    }


    bool modemPowerOff = getStatusManager()->getBoolValue(RFX_STATUS_KEY_MODEM_POWER_OFF, false);
    if (modemPowerOff) {
        RLOGD("modemPowerOff, just response to RILJ");
        sp<RfxMessage> response = RfxMessage::obtainResponse(RIL_E_RADIO_NOT_AVAILABLE, message);
        responseToRilj(response);
        return;
    }

    int suggestedCapability;
    int currentCapability = getStatusManager()->getIntValue(RFX_STATUS_KEY_RADIO_CAPABILITY, RIL_CAPABILITY_NONE);
    // adjust capability if power should be off
    if (power) {
        suggestedCapability = getStatusManager()->
            getIntValue(RFX_STATUS_KEY_SUGGESTED_RADIO_CAPABILITY, RIL_CAPABILITY_NONE);
        getStatusManager()->setBoolValue(RFX_STATUS_KEY_REQUEST_RADIO_POWER, true);

        // Handle radio state update by rild, make sure to update radio state to RILJ
        int radioState = getStatusManager()->getIntValue(RFX_STATUS_KEY_RADIO_STATE, RADIO_STATE_UNAVAILABLE);
        /*RLOGD("requestRadioPower, STATUS_KEY_REQUEST_RADIO_POWER = true, radioState = %d",
                radioState);*/
        if (radioState == RADIO_STATE_ON) {
            //RLOGD("Update RADIO_STATE_ON to RILJ");
            setRadioState((RIL_RadioState)radioState, (RIL_SOCKET_ID)slotId);
        }
    } else {
        suggestedCapability = RIL_CAPABILITY_NONE;
        getStatusManager()->setBoolValue(RFX_STATUS_KEY_REQUEST_RADIO_POWER, false);
    }
    RLOGD("slotId:%d, currentCapability:%d, suggestedCapability:%d", slotId, currentCapability, suggestedCapability);
    RpCdmaRadioController::getInstance()->registerForCdmaCapability(slotId, CAPABILITY_HAS_CDMA(suggestedCapability));

    // remember as radio Message
    RadioMessage* radioMessage = new RadioMessage(message);
    int msg_id = message->getId();

    // need callback or not
    RpSuggestRadioCapabilityCallback callback = checkIfNeedCallback(suggestCapabilityMap, message);
    if (callback.isValid()) {
        RLOGD("set callback to RadioMessage");
        radioMessage->callback = callback;
    }

    // CDMA
    RfxSocketState socketState = getStatusManager()->getSocketStateValue(RFX_STATUS_KEY_SOCKET_STATE);
    if (socketState.getIsCdmaSlot()) {
        bool cdmaPower = (power != 0) ? (RpCdmaRadioController::getInstance()->adjustCdmaRadioPower()) : false;
        RLOGD("CDMA power:%d, slotId:%d", cdmaPower, slotId);
        sp<RfxMessage> cdmaRadioRequest = RfxMessage::obtainRequest(RADIO_TECH_GROUP_C2K, msg_id, message);
        cdmaRadioRequest->getParcel()->writeInt32(1);
        cdmaRadioRequest->getParcel()->writeInt32(cdmaPower ? 1 : 0);
        radioMessage->cdmaRequest = cdmaRadioRequest;
        requestToRild(cdmaRadioRequest);
        radioMessage->isSendToCdma = true;
        if (lastRequestPower != cdmaPower ||
            c2kRadioState != ((cdmaPower == true) ? RADIO_STATE_ON : RADIO_STATE_OFF)) {
            m_c2k_radio_changed_singal.postEmit(cdmaPower);
        }
        // RLOGD("radioMessage->isSendToCdma: %s", (radioMessage->isSendToCdma) ? "true" : "false");
    }

    // GSM
    bool gsmPower = CAPABILITY_HAS_GSM(suggestedCapability);
    RLOGD("GSM power:%d, slotId:%d", gsmPower, slotId);
    sp<RfxMessage> gsmRadioRequest = RfxMessage::obtainRequest(RADIO_TECH_GROUP_GSM, msg_id, message);
    gsmRadioRequest->getParcel()->writeInt32(1);
    gsmRadioRequest->getParcel()->writeInt32(gsmPower ? 1 : 0);
    radioMessage->gsmRequest = gsmRadioRequest;
    requestToRild(gsmRadioRequest);
    enqueueToMessageQueue(radioMessage);
}

void RpRadioController::enqueueToMessageQueue(RadioMessage* radioMessage) {
    RadioMessage* message = this->MessageQueue;
    RLOGD("enqueue radio message: %d", radioMessage->serial);
    for (;;) {
        if (message->nextMessage != NULL) {
            message = message->nextMessage;
        } else {
            break;
        }
    }
    message->nextMessage = radioMessage;
}


bool RpRadioController::onHandleUrc(const sp<RfxMessage>& message) {

    int msg_id = message->getId();
    // RLOGD("RpRadioController: handle urc id %d", msg_id);
    switch (msg_id) {
    case RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED:
        handleRadioStateChanged(message);
        break;
    default:
        break;
    }
    return true;

}

void RpRadioController::handleRadioStateChanged(const sp<RfxMessage>& message) {
    RILD_RadioTechnology_Group source = message->getSource();
    int oldCapability = getStatusManager()->getIntValue(RFX_STATUS_KEY_RADIO_CAPABILITY, RIL_CAPABILITY_NONE);
    int msg_id = message->getId();
    int oldRadioState = getStatusManager()->getIntValue(RFX_STATUS_KEY_RADIO_STATE_C2K, RADIO_STATE_UNAVAILABLE);

    // handle with Parcel
    Parcel* parcel = message->getParcel();
    int32_t status = 0;
    int32_t request = 0;
    int32_t token = 0;
    int32_t radioState = RADIO_STATE_UNAVAILABLE;

    parcel->setDataPosition(0);
    status = parcel->readInt32(&request);
    status = parcel->readInt32(&token);
    status = parcel->readInt32(&radioState);
    parcel->setDataPosition(0);

    // for non-c2k project
    if (!RpFeatureOptionUtils::isC2kSupport()) {
        RLOGD("urc radioState: %d, oldCapability: %d, source: %d", radioState, oldCapability, source);
        getStatusManager()->setIntValue(RFX_STATUS_KEY_RADIO_STATE, radioState);
        setRadioState((RIL_RadioState)radioState, (RIL_SOCKET_ID)slotId);
        return;
    }

    if (source == RADIO_TECH_GROUP_C2K) {
        RpCdmaRadioController::getInstance()->handleRadioStateChanged(radioState);
        getStatusManager()->setIntValue(RFX_STATUS_KEY_RADIO_STATE_C2K, radioState);
    } else {
        if (radioState == RADIO_STATE_ON && !CAPABILITY_HAS_GSM(oldCapability)) {
            getStatusManager()->setIntValue(RFX_STATUS_KEY_RADIO_CAPABILITY, oldCapability + RIL_CAPABILITY_GSM_ONLY);
        } else if ((radioState == RADIO_STATE_OFF || radioState == RADIO_STATE_UNAVAILABLE) &&
                CAPABILITY_HAS_GSM(oldCapability)) {
            getStatusManager()->setIntValue(RFX_STATUS_KEY_RADIO_CAPABILITY,
                    oldCapability - RIL_CAPABILITY_GSM_ONLY);
        }
    }

    if (radioState == RADIO_STATE_UNAVAILABLE){
        if (source == RADIO_TECH_GROUP_C2K) {
            int gRadioState = getStatusManager()->getIntValue(RFX_STATUS_KEY_RADIO_STATE, RADIO_STATE_UNAVAILABLE);
            if (oldRadioState == radioState && gRadioState != RADIO_STATE_UNAVAILABLE) {
                RLOGD("urc radioState: %d (c2k), return, oldCapability: %d, source: %d",
                        radioState, oldCapability, source);
                return;
            }
        }
        getStatusManager()->setIntValue(RFX_STATUS_KEY_RADIO_STATE, RADIO_STATE_UNAVAILABLE);
        RLOGD("urc radioState: %d (non-c2k), return after set state, oldCapability: %d, source: %d",
                radioState, oldCapability, source);
        return;
    }

    RLOGD("urc radioState: %d, oldCapability: %d, source: %d", radioState, oldCapability, source);

    if (isUnderCapabilitySwitch == true) {
        RLOGD("Under sim switch, but also set Radio state");
        //return;
    }

    refreshRadioState();
}

void RpRadioController::refreshRadioState() {
    // handle state changed if is not unavaialable
    int newCapability = getStatusManager()->getIntValue(RFX_STATUS_KEY_RADIO_CAPABILITY, RIL_CAPABILITY_NONE);
    int oldRadioState = getStatusManager()->getIntValue(RFX_STATUS_KEY_RADIO_STATE, RADIO_STATE_UNAVAILABLE);
    int newRadioState;

    if (newCapability == RIL_CAPABILITY_NONE) {
        newRadioState = RADIO_STATE_OFF;
        getStatusManager()->setIntValue(RFX_STATUS_KEY_RADIO_STATE, RADIO_STATE_OFF);
        /*RLOGD("newCapability: %d, update as RADIO_STATE_OFF, radio state: %d -> %d",
                newCapability, oldRadioState, newRadioState);*/
    } else {
        newRadioState = RADIO_STATE_ON;
        getStatusManager()->setIntValue(RFX_STATUS_KEY_RADIO_STATE, RADIO_STATE_ON);
        /*RLOGD("newCapability: %d, update as RADIO_STATE_ON, radio state: %d -> %d",
                newCapability, oldRadioState, newRadioState);*/
    }
}

bool RpRadioController::onHandleResponse(const sp<RfxMessage>& message) {

    int msg_id = message->getId();
    //RLOGD("RpRadioController: handle response id %d", msg_id);
    switch (msg_id) {
    case RIL_REQUEST_RADIO_POWER:
    case RIL_REQUEST_ENABLE_MODEM:
        handleRadioPowerResponse(message);
        break;
    default:
        break;
    }
    return true;

}

bool RpRadioController::handleRadioPowerResponse(const sp<RfxMessage>& message) {
    int serial = message->getPToken();
    // From local serial:minus, Not from local: positive
    RadioMessage* radioMessage = NULL;
    radioMessage = findRadioMessageUsingSerial(serial);

    // More check here
    if (radioMessage == NULL) {
        RLOGD("radioMessage should not be null here");
        return false;
    }

    if (message->getSource() == RADIO_TECH_GROUP_C2K) {
        RIL_Errno rilErrno = message->getError();
        //RLOGD("Handle Response From Cdma, rilErrno=%d", rilErrno);
        if (rilErrno == RIL_E_POWER_ON_UIM_NOT_READY || rilErrno == RIL_E_OP_NOT_ALLOWED_DURING_VOICE_CALL) {
            //RLOGD("C2K power on error=RIL_E_POWER_ON_UIM_NOT_READY, retry.");
            requestToRild(radioMessage->cdmaRequest);
            return false;
        } else {
            radioMessage->cdmaResponse = message;
        }
    } else {
        RIL_Errno rilErrno = message->getError();
        //RLOGD("Handle Response From Gsm, rilErrno=%d", rilErrno);
        if (rilErrno == RIL_E_OP_NOT_ALLOWED_DURING_VOICE_CALL) {
            // EFUN may error when ERAT processing, need retry
            //RLOGD("Gsm power on error=RIL_E_OP_NOT_ALLOWED_DURING_VOICE_CALL, retry.");
            requestToRild(radioMessage->gsmRequest);
            return false;
        } else {
            radioMessage->gsmResponse = message;
        }
    }
    bool ret = checkIfRequestComplete(radioMessage);
    if (ret) {
        if (radioMessage->isDynamicSwitch == true) {
            radioMessage->action->act();
        }
        sendResponse(radioMessage);
        dequeueFromMessageQueue(radioMessage);
    }
    return true;
}

void RpRadioController::dequeueFromMessageQueue(RadioMessage* radioMessage) {
    RadioMessage* message = this->MessageQueue;
    RadioMessage* prevMessage = this->MessageQueue;
    for(;;) {
        if (message == radioMessage) {
            prevMessage->nextMessage = radioMessage->nextMessage;
            RLOGD("dequeue radio message: %d", radioMessage->serial);
            delete(radioMessage);
            break;
        } else if (message == NULL) {
            RLOGD("dequeue radio message: can't find serial %d", radioMessage->serial);
            LOG_ALWAYS_FATAL("dequeueFromMessageQueue: cant find message serial %d",
            radioMessage->serial);
            break;
        } else {
            prevMessage = message;
            message = message->nextMessage;
        }
    }
}

bool RpRadioController::checkIfRequestComplete(RadioMessage* radioMessage){
    bool ret = false;
    bool sendToCdma = radioMessage->isSendToCdma;
    if ((radioMessage->isSendToCdma) == true) {
        if ((radioMessage->gsmResponse != NULL) && (radioMessage->cdmaResponse != NULL)) {
            RLOGD("radio message sendToCdma: true, both side received, radioMessage %d complete",
                    radioMessage->serial);
            ret = true;
        } else if (radioMessage->isDynamicSwitch == true) {
            if ((radioMessage->dynamicCdmaOff && radioMessage->cdmaResponse == NULL) ||
                (radioMessage->dynamicGsmOff && radioMessage->gsmResponse == NULL)) {
                RLOGD("radio message sendToCdma: true, dynamic switch radioMessage %d not complete",
                        radioMessage->serial);
                ret = false;
            } else {
                RLOGD("radio message sendToCdma: true, dynamic switch radioMessage %d complete",
                        radioMessage->serial);
                ret = true;
            }
        }
    } else {
        if ((radioMessage -> gsmResponse) != NULL ) {
            RLOGD("radio message sendToCdma: false, radioMessage %d complete", radioMessage->serial);
            ret = true;
        }
    }
    return ret;
}

void RpRadioController::sendResponse(RadioMessage* radioMessage) {
    // RLOGD("sendResponse to RILJ");
    RIL_Errno rilErrno;
    if (radioMessage->isSendToCdma) {
        if (radioMessage->gsmResponse == NULL) {
            rilErrno = radioMessage->cdmaResponse->getError();
        } else {
            RIL_Errno gsmErrno = radioMessage->gsmResponse->getError();
            RIL_Errno cdmaErrno = radioMessage->cdmaResponse->getError();
            rilErrno = combineErrno(gsmErrno, cdmaErrno);
        }
    } else {
        rilErrno = radioMessage->gsmResponse->getError();
    }

    sp<RfxMessage> response = NULL;
    if (radioMessage->gsmResponse != NULL) {
        response = RfxMessage::obtainResponse(rilErrno, radioMessage->gsmResponse);
    } else {
        response = RfxMessage::obtainResponse(rilErrno, radioMessage->cdmaResponse);
    }

    if (radioMessage->callback.isValid()) {
        RLOGD("sendResponse, notify rat controller");
        radioMessage->callback.invoke((rilErrno == 0) ? SUCCESS : ERROR);
    }
    responseToRilj(response);
}

RIL_Errno RpRadioController::combineErrno(RIL_Errno gsmErrno, RIL_Errno cdmaErrno) {
    if (gsmErrno == RIL_E_SUCCESS && cdmaErrno == RIL_E_SUCCESS) {
        return RIL_E_SUCCESS;
    } else if (gsmErrno == RIL_E_RADIO_NOT_AVAILABLE || cdmaErrno == RIL_E_RADIO_NOT_AVAILABLE) {
        RLOGD("Combine Errno(): gsmErrno: %d, cdmaErrno: %d", gsmErrno, cdmaErrno);
        return RIL_E_RADIO_NOT_AVAILABLE;
    } else {
        RLOGD("Combine Errno(): gsmErrno: %d, cdmaErrno: %d", gsmErrno, cdmaErrno);
        return RIL_E_GENERIC_FAILURE;
    }
}

RadioMessage* RpRadioController::findRadioMessageUsingSerial(int serial) {
    //RLOGD("We want to find message with serial: %d", serial);
    // TODO: message maybe return null if not exists
    RadioMessage* message = MessageQueue;
    char sLog[LOG_SIZE] = {0};
    for (;;) {
        char str[100] = {0};
        if (message == NULL) {
            RLOGD("Can't find message %d in the queue", serial);
            break;
        } else if (message->serial == serial && message != MessageQueue) {
            sprintf(str, "%d", message->serial);
            if (strlen(sLog) + strlen(str) < LOG_SIZE*sizeof(char)) {
                strncat(sLog, str, strlen(str));
            }
            //RLOGD("find %d radio message in the queue", message->serial);
            break;
        } else {
            sprintf(str, "%d", message->serial);
            if (strlen(sLog) + strlen(str) < LOG_SIZE*sizeof(char)) {
                strncat(sLog, str, strlen(str));
            }
            message = message->nextMessage;
        }
        if (strlen(sLog) + strlen(" -> ") < LOG_SIZE*sizeof(char)) {
            strncat(sLog, " -> ", strlen(" -> "));
        }
    }
    RLOGD("findRadioMessageUsingSerial, current queue:%s", sLog);
    return message;
}

bool RpRadioController::findRadioMessageSendToCdma(void) {
    // RLOGD("We want to find radio message which is sent to cdma");
    // TODO: message maybe return null if not exists
    RadioMessage* message = MessageQueue;
    for (;;) {
        if (message == NULL) {
            //RLOGD("No radio message sent to cdma");
            break;
        } else if (true == message->isSendToCdma) {
            //RLOGD("find %d radio message which is sent to cdma in the queue", message->serial);
            return true;
        } else {
            message = message->nextMessage;
        }
    }
    return false;
}


RadioMessage* RpRadioController::findRadioMessageUsingToken(int token, int source) {
    // RLOGD("We want to find message with token: %d, source: %d", token, source);
    // TODO: message maybe return null if not exists
    RadioMessage* message = MessageQueue;
    for (;;) {
        if (source == RADIO_TECH_GROUP_GSM && token == message->gsmToken) {
            RLOGD("find %d gsm token in the queue", token);
            break;
        } else if (source == RADIO_TECH_GROUP_C2K && token == message->cdmaToken) {
            RLOGD("find %d cdma token in the queue", token);
            break;
        } else {
            message = message->nextMessage;
        }
    }
    return message;
}

void RpRadioController::dynamicSwitchRadioOff(bool cdmaOff, bool gsmOff, const sp<RfxAction>& action) {
    RLOGD("dynamicSwitchRadioOff: slot%d, cdmaOff=%d, gsmOff=%d", slotId, cdmaOff, gsmOff);
    inDynamicSwitchRadioOff = true;
    requestRadioOff(cdmaOff, gsmOff, action);
}

void RpRadioController::capabilitySwitchRadioOff(bool cdmaOff, bool gsmOff, const sp<RfxAction>& action) {
    RLOGD("capabilitySwitchRadioOff: slot%d, cdmaOff=%d, gsmOff=%d", slotId, cdmaOff, gsmOff);
    //Capability is part of dynamic switch, so when capability start, we also set dynamic switch flag.
    //To avoid radio request send between capability switch and mode switch.
    inDynamicSwitchRadioOff = true;
    requestRadioOff(cdmaOff, gsmOff, action);
}

void RpRadioController::requestRadioOff(bool cdmaOff, bool gsmOff, const sp<RfxAction>& action) {
    RLOGD("requestRadioOff: slot%d, cdmaOff=%d, gsmOff=%d", slotId, cdmaOff, gsmOff);
    // remember as radio Message
    sp<RfxMessage> message = RfxMessage::obtainRequest(slotId, RADIO_TECH_GROUP_GSM, RIL_REQUEST_RADIO_POWER);
    RadioMessage* radioMessage = new RadioMessage(message);
    int msg_id = message->getId();
    radioMessage->isDynamicSwitch = true;
    radioMessage->action = action;
    bool lastRequestPower = getStatusManager()->getBoolValue(RFX_STATUS_KEY_REQUEST_RADIO_POWER, false);
    int c2kRadioState = getStatusManager()->getIntValue(RFX_STATUS_KEY_RADIO_STATE_C2K, RADIO_STATE_UNAVAILABLE);

    RfxSocketState socketState = getStatusManager()->getSocketStateValue(RFX_STATUS_KEY_SOCKET_STATE);
    if (socketState.getIsCdmaSlot() && cdmaOff) {
        // CDMA
        bool cdmaPower = false;
        RLOGD("CDMA power:%d, slotId:%d", cdmaPower, slotId);
        sp<RfxMessage> cdmaRadioRequest = RfxMessage::obtainRequest(RADIO_TECH_GROUP_C2K, msg_id, message);
        cdmaRadioRequest->getParcel()->writeInt32(1);
        cdmaRadioRequest->getParcel()->writeInt32(cdmaPower ? 1 : 0);
        radioMessage->cdmaRequest = cdmaRadioRequest;
        requestToRild(cdmaRadioRequest);
        radioMessage->isSendToCdma = true;
        radioMessage->dynamicCdmaOff = true;
        if (lastRequestPower || c2kRadioState != RADIO_STATE_OFF) {
            m_c2k_radio_changed_singal.postEmit(cdmaPower);
        }
        // RLOGD("radioMessage->isSendToCdma: %s", (radioMessage->isSendToCdma) ? "true" : "false");
    }

    if (gsmOff) {
        // GSM
        bool gsmPower = false;
        RLOGD("GSM power:%d, slotId:%d", gsmPower, slotId);
        sp<RfxMessage> gsmRadioRequest = RfxMessage::obtainRequest(RADIO_TECH_GROUP_GSM, msg_id, message);
        gsmRadioRequest->getParcel()->writeInt32(1);
        gsmRadioRequest->getParcel()->writeInt32(gsmPower ? 1 : 0);
        radioMessage->dynamicGsmOff = true;
        radioMessage->gsmRequest = gsmRadioRequest;
        requestToRild(gsmRadioRequest);
    }
    enqueueToMessageQueue(radioMessage);
    return;
}

void RpRadioController::dynamicSwitchRadioOn() {
    RLOGD("dynamicSwitchRadioOn");
    inDynamicSwitchRadioOff = false;
    isDynamicSwitchRadioOn = true;
    requestRadioOn();
    return;
}

void RpRadioController::requestRadioOn() {
    bool requestPower = getStatusManager()->getBoolValue(RFX_STATUS_KEY_REQUEST_RADIO_POWER, false);
    bool modemPowerOff = getStatusManager()->getBoolValue(RFX_STATUS_KEY_MODEM_POWER_OFF, false);
    RLOGD("setRadioOn: slot%d, requestPower:%d, modemPowerOff:%d", slotId, requestPower, modemPowerOff);
    if (!modemPowerOff && (requestPower || isDynamicSwitchRadioOn)) {
        sp<RfxMessage> radioRequest = RfxMessage::obtainRequest(slotId, RADIO_TECH_GROUP_GSM, RIL_REQUEST_RADIO_POWER);
        radioRequest->getParcel()->writeInt32(1);
        int power = (requestPower) ? 1 : 0;
        radioRequest->getParcel()->writeInt32(power);
        RfxMainThread::enqueueMessageFront(radioRequest);
    }
    return;
}


RpSuggestRadioCapabilityCallback RpRadioController::checkIfNeedCallback(std::unordered_map<int,
        RpSuggestRadioCapabilityCallback> map, const sp<RfxMessage>& message) {
    std::unordered_map<int, RpSuggestRadioCapabilityCallback>::const_iterator result = map.find(message->getPToken());
    RpSuggestRadioCapabilityCallback result_callback;

    if (result == map.end()) {
        //RLOGD("checkIfNeedCallback: Can not find token in map");
        return RpSuggestRadioCapabilityCallback();
    }

    result_callback = result->second;
    map.erase(message->getPToken());
    RLOGD("checkIfNeedCallback: true, pToken = %d, map size = %d",
            message->getPToken(), (int)(map.size()));
    return result_callback;
}

