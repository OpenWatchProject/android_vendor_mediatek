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
#include "utils/Timers.h"
#include "libmtkrilutils.h"
#include "ratconfig.h"
#include "rfx_properties.h"
#include "RtcCapabilitySwitchController.h"
#include "RfxRadioCapabilityData.h"
#include "RfxMessageId.h"
#include "RfxRilUtils.h"
#include "RfxAction.h"
#include "RtcCapabilityGetController.h"
#include "RtcCapabilitySwitchChecker.h"
#include "RtcCapabilitySwitchUtil.h"
#include "data/RtcDataAllowController.h"
#include "power/RtcRadioController.h"

#define RFX_LOG_TAG "RtcCapa"
#define UNKNOW_SLOT  (-1)

/*****************************************************************************
 * Class RtcCapabilitySwitchController
 *****************************************************************************/

RFX_IMPLEMENT_CLASS("RtcCapabilitySwitchController", RtcCapabilitySwitchController, RfxController);

RFX_REGISTER_DATA_TO_REQUEST_ID(RfxRadioCapabilityData, RfxRadioCapabilityData, RFX_MSG_REQUEST_SET_RADIO_CAPABILITY);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxRadioCapabilityData, RFX_MSG_REQUEST_GET_RADIO_CAPABILITY);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxIntsData, RFX_MSG_REQUEST_CAPABILITY_SWITCH_SET_MAJOR_SIM);

RtcCapabilitySwitchController::RtcCapabilitySwitchController() :
        m_request_count(0), m_max_capability(0), m_new_main_slot(0), m_close_radio_count(0),
        m_new_main_slot_by_modem_id(-1), m_is_started(false), m_pending_request(NULL),
        m_checker_controller(NULL) {
    logI(RFX_LOG_TAG, "constructor entered");
}

RtcCapabilitySwitchController::~RtcCapabilitySwitchController() {
}

void RtcCapabilitySwitchController::onInit() {
    RfxController::onInit();

    const int request_id_list[] = {
        RFX_MSG_REQUEST_SET_RADIO_CAPABILITY,
        RFX_MSG_REQUEST_GET_RADIO_CAPABILITY,
        RFX_MSG_REQUEST_CAPABILITY_SWITCH_SET_MAJOR_SIM,
    };

    logD(RFX_LOG_TAG, "onInit");
    // register request & URC id list
    // NOTE. one id can only be registered by one controller
    for (int i = 0; i < RfxRilUtils::rfxGetSimCount(); i++) {
        registerToHandleRequest(i, request_id_list, sizeof(request_id_list) / sizeof(const int));
    }
    resetLock(MAX_RFX_SLOT_ID, RFX_STATUS_KEY_CAPABILITY_SWITCH_STATE);
    if (isPendingUndoneSwitch()) {
        m_new_main_slot = getUndoneSwitch();
        registerStatusKeys();
    }
    if(getGeminiMode() == GEMINI_MODE_L_AND_L &&
            RfxRilUtils::rfxGetSimCount() == 2 && RfxRilUtils::isCtVolteSupport()) {
        for (int i = 0; i < RfxRilUtils::rfxGetSimCount(); i++) {
             getStatusManager(i)->registerStatusChanged(
                     RFX_STATUS_KEY_VOLTE_STATE, RfxStatusChangeCallback(
                     this, &RtcCapabilitySwitchController::onVolteStateChanged));
        }
    }

    getNonSlotScopeStatusManager()->registerStatusChanged(
            RFX_STATUS_KEY_DEFAULT_DATA_SIM,
            RfxStatusChangeCallback(this, &RtcCapabilitySwitchController::onDefaultDataChanged));

    m_checker_controller = (RtcCapabilitySwitchChecker*)
            findController(RFX_OBJ_CLASS_INFO(RtcCapabilitySwitchChecker));
}

bool RtcCapabilitySwitchController::isPendingUndoneSwitch() {
    int undone_sim = getUndoneSwitch() + 1;
    int main_sim = RfxRilUtils::getMajorSim();
    if (undone_sim != 0 && undone_sim != main_sim) {
        return true;
    }
    return false;
}

int RtcCapabilitySwitchController::getUndoneSwitch() {
    char temp[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get("persist.vendor.radio.pendcapswt", temp, "-1");
    return atoi(temp);
}

bool RtcCapabilitySwitchController::isReadyForMessage(const sp<RfxMessage>& message, bool log) {
    int msg_id = message->getId();
    if (msg_id == RFX_MSG_REQUEST_SET_RADIO_CAPABILITY) {
        RIL_RadioCapability *capability = (RIL_RadioCapability *)message->getData()->getData();
        if (capability->phase != RC_PHASE_FINISH ) {
            return m_checker_controller->isReadyForSwitch(log);
        }
    }
    return true;
}

bool RtcCapabilitySwitchController::onPreviewMessage(const sp<RfxMessage>& message) {
    int msg_id = message->getId();
    RIL_RadioCapability *capability = (RIL_RadioCapability *)message->getData()->getData();
    if (msg_id == RFX_MSG_REQUEST_SET_RADIO_CAPABILITY) {
        RIL_RadioCapability *capability = (RIL_RadioCapability *)message->getData()->getData();
        if (capability->phase == RC_PHASE_APPLY) {
            // record new main slot
            if (strstr(capability->logicalModemUuid, "0") != NULL) {
                m_new_main_slot = message->getSlotId();
                rfx_property_set("persist.vendor.radio.pendcapswt",
                                 String8::format("%d", m_new_main_slot).string());
            }
            logD(RFX_LOG_TAG, "onPreview: new_main:%d", m_new_main_slot);
        }
    }
    return isReadyForMessage(message, true);
}

bool RtcCapabilitySwitchController::onCheckIfResumeMessage(const sp<RfxMessage>& message) {
    return isReadyForMessage(message, false);
}

void RtcCapabilitySwitchController::calculateNewMainSlot(int capability, int slot, char* modemId) {
    int diff = (m_max_capability ^ capability);
    if (strstr(modemId, "0") != NULL) {
        m_new_main_slot_by_modem_id = slot;
    } else if ((strcmp(modemId, "") == 0) && (m_new_main_slot_by_modem_id == -1)) {
        logD(RFX_LOG_TAG, "calculateMainSlot, modemId is wrong, switch to default data sim");
        m_new_main_slot_by_modem_id = getNonSlotScopeStatusManager()->getIntValue(
                RFX_STATUS_KEY_DEFAULT_DATA_SIM);
    }
    logD(RFX_LOG_TAG,
         "calculateMainSlot,maxCap=%d, newMainSlot=%d, cap=%d, slot=%d, id=%s, newMainSlotById=%d",
         m_max_capability, m_new_main_slot, capability, slot, modemId, m_new_main_slot_by_modem_id);

    if (diff & RAF_LTE) {
        if (capability & RAF_LTE) {
            m_max_capability = capability;
            m_new_main_slot = slot;
        }
    } else if (diff & RAF_TD_SCDMA) {
        if (capability & RAF_TD_SCDMA) {
            m_max_capability = capability;
            m_new_main_slot = slot;
        }
    } else if (diff & RAF_UMTS) {
        if (capability & RAF_UMTS) {
            m_max_capability = capability;
            m_new_main_slot = slot;
        }
    }
    if ((m_request_count == RfxRilUtils::rfxGetSimCount()) &&
            (m_new_main_slot_by_modem_id != -1)) {
        m_new_main_slot = m_new_main_slot_by_modem_id;
        logD(RFX_LOG_TAG, "calculateMainSlot newMainSlot=%d", m_new_main_slot);
    }
}

bool RtcCapabilitySwitchController::onHandleRequest(const sp<RfxMessage> &message) {
    int msg_id = message->getId();
    RIL_RadioCapability *capability = NULL;
    char tempstr[RFX_PROPERTY_VALUE_MAX] = { 0 };

    switch (msg_id) {
        case RFX_MSG_REQUEST_SET_RADIO_CAPABILITY:
            capability = (RIL_RadioCapability *)message->getData()->getData();
            logI(RFX_LOG_TAG,
                 "RadioCapability version=%d, session=%d, phase=%d, rat=%d, logicMD=%s, status=%d",
                 capability->version, capability->session, capability->phase, capability->rat,
                 capability->logicalModemUuid, capability->status);
            switch (capability->phase) {
                case RC_PHASE_START: {
                    if (m_checker_controller->isInCalling(true)) {
                        logI(RFX_LOG_TAG, "in calling state, return!");
                        responseToRilj(RfxMessage::obtainResponse(
                                       RIL_E_OP_NOT_ALLOWED_DURING_VOICE_CALL, message, true));
                        return true;
                    }
                    if (m_checker_controller->isInPukLock(true)) {
                        logI(RFX_LOG_TAG, "in PUK lock state, return!");
                        responseToRilj(RfxMessage::obtainResponse(
                                       RIL_E_INVALID_SIM_STATE, message, true));
                        return true;
                    }
                    if ((strcmp(capability->logicalModemUuid, "") == 0) &&
                            (capability->rat == 0)) {
                        logI(RFX_LOG_TAG, "invalid modem uuid, return!");
                        responseToRilj(RfxMessage::obtainResponse(
                                       RIL_E_INVALID_ARGUMENTS, message, false));
                        return true;
                    }
                    m_request_count = 0;
                    m_max_capability = 0;
                    m_new_main_slot = 0;
                    m_new_main_slot_by_modem_id = -1;
                    m_is_started = true;
                    responseToRilj(RfxMessage::obtainResponse(RIL_E_SUCCESS, message, true));
                    return true;
                }
                case RC_PHASE_APPLY: {
                    if (m_is_started == true) {
                        // Legacy three-physes solution
                        m_request_count++;
                        calculateNewMainSlot(capability->rat, message->getSlotId(),
                                             capability->logicalModemUuid);
                        if (m_request_count == RfxRilUtils::rfxGetSimCount()) {
                            m_request_count = 0;
                            rfx_property_set("vendor.ril.rc.session.id1",
                                             String8::format("%d", capability->session).string());
                            if (m_checker_controller->isSkipCapabilitySwitch(m_new_main_slot)) {
                                m_is_started = false;
                                //No need to do, send urc to rilj directly
                                sp<RfxMessage> set_capability_request =
                                        RfxMessage::obtainResponse(RIL_E_SUCCESS, message, true);
                                responseToRilj(set_capability_request);
                                updateRadioCapability();
                                return true;
                            }
                            ///TODO: check op rules
                            m_pending_request = message;
                            handleRequestSwitchCapability();
                        } else {
                            sp<RfxMessage> set_capability_request =
                                    RfxMessage::obtainResponse(RIL_E_SUCCESS, message, true);
                            responseToRilj(set_capability_request);
                        }
                    } else {
                        // New one-physe solution, only one apply request to new main slot
                        if (!m_checker_controller->isSkipCapabilitySwitch(m_new_main_slot)) {
                            m_pending_request = message;
                            handleRequestSwitchCapability();
                        } else {
                            sp<RfxMessage> set_capability_request =
                                    RfxMessage::obtainResponse(RIL_E_SUCCESS, message, true);
                            responseToRilj(set_capability_request);
                        }
                    }
                    return true;
                }
                case RC_PHASE_FINISH: {
                    sp<RfxMessage> set_capability_request =
                            RfxMessage::obtainResponse(RIL_E_SUCCESS, message, true);
                    responseToRilj(set_capability_request);
                    return true;
                }
                default:
                    sp<RfxMessage> set_capability_request =
                            RfxMessage::obtainResponse(RIL_E_INVALID_ARGUMENTS, message, true);
                    responseToRilj(set_capability_request);
                    return true;
            }
            break;
        case RFX_MSG_REQUEST_GET_RADIO_CAPABILITY:
            requestToMcl(message);
            break;
        default:
            break;
    }
    return true;
}

void RtcCapabilitySwitchController::powerOffRadio() {
    m_close_radio_count = 0;
    int radio_capability;
    for (int i = 0; i < RfxRilUtils::rfxGetSimCount(); i++) {
        radio_capability = getStatusManager(i)->getIntValue(RFX_STATUS_KEY_SLOT_CAPABILITY, 0);
        if ((radio_capability & RAF_CDMA_GROUP) != 0) {
            resetLock(i, RFX_STATUS_KEY_RADIO_LOCK);
            sp<RfxAction> action0 = new RfxAction1<int>(
                    this, &RtcCapabilitySwitchController::onRequestRadioOffDone, i);
            RtcRadioController* radio_controller =
                    (RtcRadioController *)findController(i, RFX_OBJ_CLASS_INFO(RtcRadioController));
            radio_controller->moduleRequestRadioPower(false, action0, RFOFF_CAUSE_SIM_SWITCH);
        }
    }

    for (int i = 0; i < RfxRilUtils::rfxGetSimCount(); i++) {
        radio_capability = getStatusManager(i)->getIntValue(RFX_STATUS_KEY_SLOT_CAPABILITY, 0);
        if ((radio_capability & RAF_CDMA_GROUP) == 0) {
            resetLock(i, RFX_STATUS_KEY_RADIO_LOCK);
            sp<RfxAction> action0 = new RfxAction1<int>(
                    this, &RtcCapabilitySwitchController::onRequestRadioOffDone, i);
            RtcRadioController* radio_controller =
                    (RtcRadioController *)findController(i, RFX_OBJ_CLASS_INFO(RtcRadioController));
            radio_controller->moduleRequestRadioPower(false, action0, RFOFF_CAUSE_SIM_SWITCH);
        }
    }
}

void RtcCapabilitySwitchController::onRequestRadioOffDone(int slotId) {
    m_close_radio_count++;
    logD(RFX_LOG_TAG, "onRequestRadioOffDone slotid=%d closeRadioCount=%d", slotId,
         m_close_radio_count);
    setLock(slotId, RFX_STATUS_KEY_RADIO_LOCK);
    if (m_close_radio_count == RfxRilUtils::rfxGetSimCount()) {
        m_close_radio_count = 0;
        requestSetMajorSim();
    }
}

void RtcCapabilitySwitchController::requestSetMajorSim() {
    char tempstr[RFX_PROPERTY_VALUE_MAX] = { 0 };

    rfx_property_get("persist.vendor.radio.simswitch", tempstr, "1");
    int current_main_slot = atoi(tempstr) - 1;
    int msg_data[1];
    msg_data[0] = m_new_main_slot;
    sp<RfxMessage> msg = RfxMessage::obtainRequest(
            current_main_slot, RFX_MSG_REQUEST_CAPABILITY_SWITCH_SET_MAJOR_SIM,
            RfxIntsData(msg_data, 1));
    requestToMcl(msg);
}

void RtcCapabilitySwitchController::processSetMajorSimResponse(const sp<RfxMessage> &message) {
    char property_value[RFX_PROPERTY_VALUE_MAX] = { 0 };
    int session_id;

    rfx_property_get("vendor.ril.rc.session.id1", property_value, "-1");
    session_id = atoi(property_value);
    if (message->getError() != RIL_E_SUCCESS) {
        if (session_id != -1) {
            int msg_data[1];
            msg_data[0] = m_new_main_slot;
            //retry if session hasn't been terminated
            sp<RfxMessage> msg = RfxMessage::obtainRequest(
                    message->getSlotId(), message->getId(), RfxIntsData(msg_data, 1));
            requestToMcl(msg);
        }
    } else {
        int modem_off_state = getLockState(MAX_RFX_SLOT_ID, RFX_STATUS_KEY_MODEM_OFF_STATE);
        if (modem_off_state == MODEM_OFF_BY_SIM_SWITCH) {
            resetLock(MAX_RFX_SLOT_ID, RFX_STATUS_KEY_MODEM_OFF_STATE);
        }
        getNonSlotScopeStatusManager()->setIntValue(RFX_STATUS_KEY_CAPABILITY_SWITCH_STATE,
                CAPABILITY_SWITCH_STATE_ENDING);
        if (m_pending_request != NULL) {
            sp<RfxMessage> set_capability_request =
                    RfxMessage::obtainResponse(RIL_E_SUCCESS, m_pending_request, true);
            responseToRilj(set_capability_request);
            m_pending_request = NULL;
        }
        rfx_property_set("persist.vendor.radio.pendcapswt", String8::format("%d", -1).string());
    }
}

bool RtcCapabilitySwitchController::onHandleResponse(const sp<RfxMessage> &message) {
    int msg_id = message->getId();

    logD(RFX_LOG_TAG, "onHandleResponse:%s", idToString(msg_id));
    switch (msg_id) {
        case RFX_MSG_REQUEST_CAPABILITY_SWITCH_SET_MAJOR_SIM: {
            processSetMajorSimResponse(message);
            break;
        }
        case RFX_MSG_REQUEST_GET_RADIO_CAPABILITY: {
            responseToRilj(message);
            break;
        }
        default:
            break;
    }
    return true;
}

bool RtcCapabilitySwitchController::onCheckIfRejectMessage(const sp<RfxMessage>& message,
        bool isModemPowerOff, int radioState) {
    RFX_UNUSED(radioState);
    int msg_id = message->getId();
    RIL_RadioCapability *capability = NULL;
    //logD(RFX_LOG_TAG, "onCheckIfRejectMessage, msg_id: %s", idToString(msg_id));
    if (msg_id == RFX_MSG_REQUEST_GET_RADIO_CAPABILITY ||
        (msg_id == RFX_MSG_REQUEST_SET_RADIO_CAPABILITY && !isModemPowerOff) ||
        msg_id == RFX_MSG_REQUEST_CAPABILITY_SWITCH_SET_MAJOR_SIM) {
        return false;
    } else {
        return true;
    }
}

void RtcCapabilitySwitchController::onDefaultDataChanged(RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant new_value) {
    int old_slot = old_value.asInt();
    int new_slot = new_value.asInt();
    logD(RFX_LOG_TAG, "onDefaultDataChanged:%s(%d==>%d)",
         RfxStatusManager::getKeyString(key), old_slot, new_slot);
    rfx_property_set("persist.vendor.radio.pendcapswt",
                     String8::format("%d", new_slot).string());

    /*
    if (m_checker_controller->isReadyForSwitch(true) && m_is_started == false) {
        checkAndSwitchCapability();
    } else {
        logD(RFX_LOG_TAG, "onDefaultDataChanged, not ready for switch");
        registerStatusKeys();
    }
    */
}

void RtcCapabilitySwitchController::onVolteStateChanged(RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant new_value) {
    int old_state = old_value.asInt();
    int high_priority_slot = m_checker_controller->getHigherPrioritySlot();
    logD(RFX_LOG_TAG, "onVolteStateChanged:%s(%s==>%s), high_priority_slot = %d",
         RfxStatusManager::getKeyString(key), old_value.toString().string(),
         new_value.toString().string(), high_priority_slot);

    if (high_priority_slot >= 0 && old_state != -1) {
        if (m_checker_controller->isReadyForSwitch(true) && m_is_started == false) {
            checkAndSwitchCapability();
        } else {
            logD(RFX_LOG_TAG, "onVolteStateChanged, not ready for switch");
            registerStatusKeys();
        }
    }
}

void RtcCapabilitySwitchController::checkAndSwitchCapability() {
    int high_priority_slot = m_checker_controller->getHigherPrioritySlot();
    logD(RFX_LOG_TAG, "checkAndSwitchCapability, high_priority_slot=%d", high_priority_slot);

    if (high_priority_slot >= 0) {
        m_new_main_slot = high_priority_slot;
    }
    if (m_checker_controller->isSkipCapabilitySwitch(m_new_main_slot)) {
        return;
    }
    handleRequestSwitchCapability();
}

void RtcCapabilitySwitchController::registerStatusKeys() {
    for (int i = 0; i < RfxRilUtils::rfxGetSimCount(); i++) {
        getStatusManager(i)->registerStatusChanged(
                RFX_STATUS_KEY_RADIO_LOCK,
                RfxStatusChangeCallback(this, &RtcCapabilitySwitchController::onStatusKeyChanged));
        getStatusManager(i)->registerStatusChanged(
                RFX_STATUS_KEY_EMERGENCY_MODE,
                RfxStatusChangeCallback(this, &RtcCapabilitySwitchController::onStatusKeyChanged));
        getStatusManager(i)->registerStatusChanged(
                RFX_STATUS_KEY_AP_VOICE_CALL_COUNT,
                RfxStatusChangeCallback(this, &RtcCapabilitySwitchController::onStatusKeyChanged));
        getStatusManager(i)->registerStatusChanged(
                RFX_STATUS_KEY_EMERGENCY_CALLBACK_MODE,
                RfxStatusChangeCallback(this, &RtcCapabilitySwitchController::onStatusKeyChanged));
        getStatusManager(i)->registerStatusChanged(
                RFX_STATUS_KEY_SIM_STATE,
                RfxStatusChangeCallback(this, &RtcCapabilitySwitchController::onStatusKeyChanged));
        if(getGeminiMode() == GEMINI_MODE_L_AND_L && RfxRilUtils::rfxGetSimCount() == 2) {
            getStatusManager(i)->registerStatusChanged(
                    RFX_STATUS_KEY_GSM_IMSI, RfxStatusChangeCallback(
                    this, &RtcCapabilitySwitchController::onStatusKeyChanged));
            getStatusManager(i)->registerStatusChanged(
                    RFX_STATUS_KEY_C2K_IMSI, RfxStatusChangeCallback(
                    this, &RtcCapabilitySwitchController::onStatusKeyChanged));
        }
    }
    getNonSlotScopeStatusManager()->registerStatusChanged(
            RFX_STATUS_KEY_MODEM_OFF_STATE,
            RfxStatusChangeCallback(this, &RtcCapabilitySwitchController::onStatusKeyChanged));
    getNonSlotScopeStatusManager()->registerStatusChanged(
            RFX_STATUS_KEY_CAPABILITY_SWITCH_WAIT_MODULE,
            RfxStatusChangeCallback(this, &RtcCapabilitySwitchController::onStatusKeyChanged));
    getNonSlotScopeStatusManager()->registerStatusChanged(
            RFX_STATUS_KEY_CAPABILITY_SWITCH_STATE,
            RfxStatusChangeCallback(this, &RtcCapabilitySwitchController::onStatusKeyChanged));
}

void RtcCapabilitySwitchController::unregisterStatusKeys() {
    for (int i = 0; i < RfxRilUtils::rfxGetSimCount(); i++) {
        getStatusManager(i)->unRegisterStatusChanged(
                RFX_STATUS_KEY_RADIO_LOCK,
                RfxStatusChangeCallback(this, &RtcCapabilitySwitchController::onStatusKeyChanged));
        getStatusManager(i)->unRegisterStatusChanged(
                RFX_STATUS_KEY_EMERGENCY_MODE,
                RfxStatusChangeCallback(this, &RtcCapabilitySwitchController::onStatusKeyChanged));
        getStatusManager(i)->unRegisterStatusChanged(
                RFX_STATUS_KEY_AP_VOICE_CALL_COUNT,
                RfxStatusChangeCallback(this, &RtcCapabilitySwitchController::onStatusKeyChanged));
        getStatusManager(i)->unRegisterStatusChanged(
                RFX_STATUS_KEY_EMERGENCY_CALLBACK_MODE,
                RfxStatusChangeCallback(this, &RtcCapabilitySwitchController::onStatusKeyChanged));
        getStatusManager(i)->unRegisterStatusChanged(
                RFX_STATUS_KEY_SIM_STATE,
                RfxStatusChangeCallback(this, &RtcCapabilitySwitchController::onStatusKeyChanged));
        if(getGeminiMode() == GEMINI_MODE_L_AND_L && RfxRilUtils::rfxGetSimCount() == 2) {
            getStatusManager(i)->unRegisterStatusChanged(
                    RFX_STATUS_KEY_GSM_IMSI, RfxStatusChangeCallback(
                    this, &RtcCapabilitySwitchController::onStatusKeyChanged));
            getStatusManager(i)->unRegisterStatusChanged(
                    RFX_STATUS_KEY_C2K_IMSI, RfxStatusChangeCallback(
                    this, &RtcCapabilitySwitchController::onStatusKeyChanged));
        }
    }
    getNonSlotScopeStatusManager()->unRegisterStatusChanged(
            RFX_STATUS_KEY_MODEM_OFF_STATE,
            RfxStatusChangeCallback(this, &RtcCapabilitySwitchController::onStatusKeyChanged));
    getNonSlotScopeStatusManager()->unRegisterStatusChanged(
            RFX_STATUS_KEY_CAPABILITY_SWITCH_WAIT_MODULE,
            RfxStatusChangeCallback(this, &RtcCapabilitySwitchController::onStatusKeyChanged));
    getNonSlotScopeStatusManager()->unRegisterStatusChanged(
            RFX_STATUS_KEY_CAPABILITY_SWITCH_STATE,
            RfxStatusChangeCallback(this, &RtcCapabilitySwitchController::onStatusKeyChanged));

}

void RtcCapabilitySwitchController::onStatusKeyChanged(RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant new_value) {
    logD(RFX_LOG_TAG, "onStatusKeyChanged:%s(%s==>%s)", RfxStatusManager::getKeyString(key),
         old_value.toString().string(), new_value.toString().string());

    if (m_checker_controller->isReadyForSwitch(true) && m_is_started == false) {
        unregisterStatusKeys();
        checkAndSwitchCapability();
    }
}

/*
//TODO: move to op plug-in
bool RtcCapabilitySwitchController::isSatisfyOperatorRules(int new_major_slot) {
    bool ret = true;
    char optr_value[RFX_PROPERTY_VALUE_MAX] = {0};
    rfx_property_get("persist.vendor.operator.optr", optr_value, "0");
    if (strcmp(optr_value, OPERATOR_OP01) == 0) {
        ret = isSatisfyOp01Rules(new_major_slot);
    }
    return ret;
}

bool RtcCapabilitySwitchController::isSatisfyOp01Rules(int new_major_slot) {
    bool ret = true;
    bool has_op01_sim = false;
    int insert_status[4] = {0}; // 0: no sim, 1: op01 sim, 2: other sim
    int sim_count = RfxRilUtils::rfxGetSimCount();
    char sim_mode_prop[RFX_PROPERTY_VALUE_MAX] = {0};
    rfx_property_get("persist.vendor.radio.sim.mode", sim_mode_prop, "0");
    int simMode = atoi(sim_mode_prop);

    // get sim card insert status
    for (int i = 0; i < sim_count; i++) {
        int sim_state = getStatusManager(i)->getIntValue(RFX_STATUS_KEY_SIM_STATE);
        if (sim_state == RFX_SIM_STATE_READY && (((simMode >> i) & 1) == 1)) {
            if (RtcCapabilitySwitchUtil::isOp01Sim(getImsi(i))) {
                insert_status[i] = 1;
                has_op01_sim = true;
            } else {
                insert_status[i] = 2;
            }
        } else {
            insert_status[i] = 0;
        }
    }
    logD(RFX_LOG_TAG, "isSatisfyOp01Rules, insert_status=%d,%d, simMode=%d, new_major_slot=%d",
            insert_status[0], insert_status[1], simMode, new_major_slot);

    // switch to non-op01 sim
    if (has_op01_sim && (insert_status[new_major_slot] != 1)) {
        ret = false;
    }
    return ret;
}
*/

void RtcCapabilitySwitchController::handleRequestSwitchCapability() {
    setAllLocks();
    backupRadioPower();
    powerOffRadio();
}

void RtcCapabilitySwitchController::setAllLocks(void) {
    setLock(MAX_RFX_SLOT_ID, RFX_STATUS_KEY_MODEM_OFF_STATE);
    for (int i = RFX_SLOT_ID_0; i < RfxRilUtils::rfxGetSimCount(); i++) {
        setLock(i, RFX_STATUS_KEY_RADIO_LOCK);
    }
    setLock(MAX_RFX_SLOT_ID, RFX_STATUS_KEY_CAPABILITY_SWITCH_STATE);
}

void RtcCapabilitySwitchController::setLock(int slot_id, RfxStatusKeyEnum key) {
    switch (key) {
        case RFX_STATUS_KEY_MODEM_OFF_STATE: {
            getNonSlotScopeStatusManager()->setIntValue(key, MODEM_OFF_BY_SIM_SWITCH);
            break;
        }
        case RFX_STATUS_KEY_RADIO_LOCK: {
            if (slot_id >= RFX_SLOT_ID_0 && slot_id < MAX_RFX_SLOT_ID) {
                getStatusManager(slot_id)->setIntValue(key, RADIO_LOCK_BY_SIM_SWITCH);
            }
            break;
        }
        case RFX_STATUS_KEY_CAPABILITY_SWITCH_STATE: {
            getNonSlotScopeStatusManager()->setIntValue(key, CAPABILITY_SWITCH_STATE_START);
            break;
        }

        default:
            break;
    }
}

int RtcCapabilitySwitchController::getLockState(int slot_id, RfxStatusKeyEnum key) {
    int ret = -1;
    switch (key) {
        case RFX_STATUS_KEY_MODEM_OFF_STATE: {
            ret = getNonSlotScopeStatusManager()->getIntValue(
                    key, MODEM_OFF_IN_IDLE);
            break;
        }
        case RFX_STATUS_KEY_RADIO_LOCK: {
            if (slot_id >= RFX_SLOT_ID_0 && slot_id < MAX_RFX_SLOT_ID) {
                ret = getStatusManager(slot_id)->getIntValue(key, RADIO_LOCK_IDLE);
            }
            break;
        }
        case RFX_STATUS_KEY_CAPABILITY_SWITCH_STATE: {
            ret = getNonSlotScopeStatusManager()->getIntValue(key);
            break;
        }

        default:
            break;
    }

    return ret;
}

void RtcCapabilitySwitchController::resetLock(int slot_id, RfxStatusKeyEnum key) {
    switch (key) {
        case RFX_STATUS_KEY_MODEM_OFF_STATE: {
            getNonSlotScopeStatusManager()->setIntValue(key, MODEM_OFF_IN_IDLE);
            break;
        }
        case RFX_STATUS_KEY_RADIO_LOCK: {
            if (slot_id >= RFX_SLOT_ID_0 && slot_id < MAX_RFX_SLOT_ID) {
                getStatusManager(slot_id)->setIntValue(key, RADIO_LOCK_IDLE);
            }
            break;
        }
        case RFX_STATUS_KEY_CAPABILITY_SWITCH_STATE: {
            getNonSlotScopeStatusManager()->setIntValue(key, CAPABILITY_SWITCH_STATE_IDLE);
            break;
        }

        default:
            break;
    }
}

void RtcCapabilitySwitchController::backupRadioPower() {
    for (int i = 0; i < RfxRilUtils::rfxGetSimCount(); i++) {
        m_backup_radio_power[i] = getStatusManager(i)->getBoolValue(
                RFX_STATUS_KEY_REQUEST_RADIO_POWER, false);
        logD(RFX_LOG_TAG, "m_backup_radio_power[%d]=%d", i, m_backup_radio_power[i]);
    }
}

void RtcCapabilitySwitchController::powerOnRadio() {
    for (int i = 0; i < RfxRilUtils::rfxGetSimCount(); i++) {
        RadioPowerLock radio_lock = (RadioPowerLock) getLockState(i, RFX_STATUS_KEY_RADIO_LOCK);
        logD(RFX_LOG_TAG, "powerOnRadio[%d]:%d,radio_lock:%d,start:%d", i, m_backup_radio_power[i],
             radio_lock, m_is_started);
        if (radio_lock == RADIO_LOCK_BY_SIM_SWITCH) {
            resetLock(i, RFX_STATUS_KEY_RADIO_LOCK);
        }
        if (m_is_started == false && m_backup_radio_power[i] == true) {
            // Three-phase solution will power on radio from Java
            RtcRadioController* radio_controller = (RtcRadioController *)findController(
                    i, RFX_OBJ_CLASS_INFO(RtcRadioController));
            radio_controller->moduleRequestRadioPower(m_backup_radio_power[i], NULL,
                                                      RFOFF_CAUSE_SIM_SWITCH);
        }
    }
}

void RtcCapabilitySwitchController::notifySetRatDone() {
    int state = getLockState(MAX_RFX_SLOT_ID, RFX_STATUS_KEY_CAPABILITY_SWITCH_STATE);
    logD(RFX_LOG_TAG, "notifySetRatDone, state: %d", state);
    if (state == CAPABILITY_SWITCH_STATE_ENDING) {
        getNonSlotScopeStatusManager()->setIntValue(
                RFX_STATUS_KEY_CAPABILITY_SWITCH_STATE, CAPABILITY_SWITCH_STATE_IDLE);
        RtcDataAllowController* data_controller = (RtcDataAllowController *)findController(
                RFX_OBJ_CLASS_INFO(RtcDataAllowController));
        if (data_controller) {
            data_controller->resendAllowData();
        }
        powerOnRadio();
        resetLock(MAX_RFX_SLOT_ID, RFX_STATUS_KEY_CAPABILITY_SWITCH_STATE);
        m_is_started = false;
    }
}

void RtcCapabilitySwitchController::updateRadioCapability() {
    RtcCapabilityGetController* get_controller = (RtcCapabilityGetController *)
            findController(RFX_OBJ_CLASS_INFO(RtcCapabilityGetController));
    char temp[RFX_PROPERTY_VALUE_MAX] = { 0 };
    memset(temp, 0, sizeof(temp));
    rfx_property_get("persist.vendor.radio.c_capability_slot", temp, "1");
    int cslot = atoi(temp) - 1;
    get_controller->updateRadioCapability(cslot);
}
