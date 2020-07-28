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

#include <cutils/properties.h>
#include <telephony/mtk_ril.h>
#include <Errors.h>
#include "RpCapabilitySwitchController.h"
#include "RfxLog.h"
#include "RfxStatusManager.h"
#include "util/RpFeatureOptionUtils.h"
#include "power/RadioConstants.h"
#include "power/RpRadioController.h"
#include <libmtkrilutils.h>

#define RFX_LOG_TAG "RpCapabilitySwitchController"

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/

RFX_IMPLEMENT_CLASS("RpCapabilitySwitchController", RpCapabilitySwitchController, RfxController);

RpCapabilitySwitchController::RpCapabilitySwitchController() : kModeSwitching(1),
    mCloseRadioCount(-1), mC2kChannelLocked(false) {
}

RpCapabilitySwitchController::~RpCapabilitySwitchController() {
}

void RpCapabilitySwitchController::onInit() {
    RfxController::onInit(); // Required: invoke super class implementation
    RFX_LOG_D(RFX_LOG_TAG, "RpCapabilitySwitchController, onInit");
    const int request_id_list[] = {
        RIL_REQUEST_SET_RADIO_CAPABILITY
    };

    // register request & URC id list
    // NOTE. one id can only be registered by one controller
    if (RpFeatureOptionUtils::isC2kSupport()) {
        registerToHandleRequest(0, request_id_list, (sizeof(request_id_list)/sizeof(int)));
        registerToHandleRequest(1, request_id_list, (sizeof(request_id_list)/sizeof(int)));
        //registerToHandleUrc(urc_id_list, (sizeof(urc_id_list)/sizeof(int)));
    }
    char tempstr[PROPERTY_VALUE_MAX];
    memset(tempstr, 0, sizeof(tempstr));
    property_get("persist.vendor.radio.simswitch", tempstr, "1");
    int capabilitySlotId = atoi(tempstr) - 1;
    getNonSlotScopeStatusManager()->setIntValue(
            RFX_STATUS_KEY_MAIN_CAPABILITY_SLOT, capabilitySlotId);
}

bool RpCapabilitySwitchController::onPreviewMessage(const sp<RfxMessage>& message) {
    int modeSwitching = getNonSlotScopeStatusManager()->getIntValue(
        RFX_STATUS_KEY_MODE_SWITCH, 0);
    int modemOffState = getNonSlotScopeStatusManager()->getIntValue(
        RFX_STATUS_KEY_MODEM_OFF_STATE, MODEM_OFF_IN_IDLE);
    bool modemPowerOff = getStatusManager()->getBoolValue(
        RFX_STATUS_KEY_MODEM_POWER_OFF, false);
    RFX_LOG_D(RFX_LOG_TAG, "RpCapabilitySwitchController,onPreviewMessage modeSwitching = %d, %d",
        modeSwitching, modemOffState);

    if (!modemPowerOff && modemOffState != MODEM_OFF_BY_POWER_OFF) {
        char tmp[PROPERTY_VALUE_MAX] = {0};
        property_get(PROPERTY_NET_CDMA_MDMSTAT, tmp, "not");
        if(strncmp(tmp, "ready", 5) != 0) {
            RFX_LOG_D(RFX_LOG_TAG, "onPreviewMessage, PROPERTY_NET_CDMA_MDMSTAT:%s", tmp);
            return false;
        }
    }

    // if the mutex key RFX_STATUS_KEY_MODEM_OFF_STATE is in mode switch state, sim switch need to
    //  wait for mode switch finish.
    if (modemOffState == MODEM_OFF_BY_MODE_SWITCH) {
        modeSwitching = 1;
    }
    if (modeSwitching == kModeSwitching){
        if (checkPhaseIfFinish(message)){
            RFX_LOG_D(RFX_LOG_TAG, "onPreviewMessage, modeswitching in finish phase ,by pass");
            return true;
        }
        return false;
    }
    if(mCloseRadioCount > -1 && mCloseRadioCount < RFX_SLOT_COUNT){
        return false;
    }
    return isReadyForMessage(message, true);
}

bool RpCapabilitySwitchController::onCheckIfResumeMessage(const sp<RfxMessage>& message) {
    int modeSwitching = getNonSlotScopeStatusManager()->getIntValue(
        RFX_STATUS_KEY_MODE_SWITCH, 0);
    int modemOffState = getNonSlotScopeStatusManager()->getIntValue(
        RFX_STATUS_KEY_MODEM_OFF_STATE, MODEM_OFF_IN_IDLE);
    bool modemPowerOff = getStatusManager()->getBoolValue(
        RFX_STATUS_KEY_MODEM_POWER_OFF, false);

    if (!modemPowerOff && modemOffState != MODEM_OFF_BY_POWER_OFF) {
        char tmp[PROPERTY_VALUE_MAX] = {0};
        property_get(PROPERTY_NET_CDMA_MDMSTAT, tmp, "not");
        if(strncmp(tmp, "ready", 5) != 0) {
            RFX_LOG_D(RFX_LOG_TAG, "onPreviewMessage, PROPERTY_NET_CDMA_MDMSTAT:%s", tmp);
            return false;
        }
    }

    // if the mutex key RFX_STATUS_KEY_MODEM_OFF_STATE is in mode switch state, sim switch need to
    //  wait for mode switch finish.
    if (modemOffState == MODEM_OFF_BY_MODE_SWITCH) {
        modeSwitching = 1;
    }

    if (modeSwitching == kModeSwitching){
        if (checkPhaseIfFinish(message)){
            RFX_LOG_D(RFX_LOG_TAG, "onCheckIfResumeMessage, modeswitching in finish phase ,by pass");
            return true;
        }
        return false;
    }
    if(mCloseRadioCount > -1 && mCloseRadioCount < RFX_SLOT_COUNT){
        // RFX_LOG_D(RFX_LOG_TAG, "onPreviewMessage, mCloseRadioCount:%d", mCloseRadioCount);
        return false;
    }
    return isReadyForMessage(message, false);
}

bool RpCapabilitySwitchController::isReadyForMessage(const sp<RfxMessage>& message, bool log) {
    if (isDssNoResetSupport()) {
        int c2k_slot = getStatusManager(RFX_SLOT_ID_UNKNOWN)->getIntValue(
            RFX_STATUS_KEY_CDMA_SOCKET_SLOT, 0);
        RIL_RadioCapability rc;
        getRadioCapability(message, rc);
        if (rc.phase == RC_PHASE_APPLY && message->getSlotId() != c2k_slot &&
            !mC2kChannelLocked && hasCdmaCard() && !hasVsimCard()) {
            if (log) {
                RFX_LOG_D(RFX_LOG_TAG, "isReadyForMessage, wait c2k lock:%d", mC2kChannelLocked);
            }
            return false;
        }
    }
    return true;
}

bool RpCapabilitySwitchController::onHandleRequest(const sp<RfxMessage>& message) {
    // RFX_LOG_D(RFX_LOG_TAG, "onHandleRequest, handle: %s", requestToString(message->getId()));
    int msg_id = message->getId();
    switch (msg_id) {
        case RIL_REQUEST_SET_RADIO_CAPABILITY: {
            sp<RfxMessage> c2k_request =
                    RfxMessage::obtainRequest(RADIO_TECH_GROUP_C2K, msg_id, message,true);
            char tmp[PROPERTY_VALUE_MAX] = {0};
            property_get(PROPERTY_NET_CDMA_MDMSTAT, tmp, "not");
            int c2kSlot = getStatusManager(RFX_SLOT_ID_UNKNOWN)->getIntValue(
                    RFX_STATUS_KEY_CDMA_SOCKET_SLOT, 0);
            RFX_LOG_D(RFX_LOG_TAG, "handle: %s, onHandleRequest, C2K slot=%d, %s",
                    requestToString(msg_id), c2kSlot, tmp);
            int phase = checkPhase(message);
            if ( phase == RC_PHASE_APPLY ) {
                // RFX_LOG_D(RFX_LOG_TAG, "onHandleRequest, phase RC_PHASE_APPLY");
                if(isSvlteSupport()) {
                    if(message->getSlotId() == c2kSlot) {
                        RFX_LOG_D(RFX_LOG_TAG, "onHandleRequest, phase RC_PHASE_APPLY handle C2K request");
                        requestToRild(c2k_request);
                    } else {
                        RFX_LOG_D(RFX_LOG_TAG, "onHandleRequest, phase RC_PHASE_APPLY handle GSM request,slotId= %d",
                              message->getSlotId());
                        requestToRild(message);
                    }
                } else {
                    if(strncmp(tmp, "ready", 5) != 0) {
                        RFX_LOG_D(RFX_LOG_TAG, "onHandleRequest, phase RC_PHASE_APPLY, MD3 not ready");
                        sp<RfxMessage> un_set_capability_request =
                        RfxMessage::obtainResponse(RIL_E_GENERIC_FAILURE, message, false);
                        responseToRilj(un_set_capability_request);
                    } else if (message->getSlotId() == c2kSlot) {
                        RFX_LOG_D(RFX_LOG_TAG, "onHandleRequest, phase RC_PHASE_APPLY handle C2K request");
                        mC2kChannelLocked = false;
                        requestToRild(c2k_request);
                    } else {
                        RFX_LOG_D(RFX_LOG_TAG, "onHandleRequest, phase RC_PHASE_APPLY handle GSM request,slotId= %d",
                                  message->getSlotId());
                        getNonSlotScopeStatusManager()->setIntValue(
                                RFX_STATUS_KEY_RESTRICT_MODE_STATE, RESTRICT_MODE_STATE_ON);
                        requestToRild(message);
                    }
                }
                return true;
            } else if (phase == RC_PHASE_START) {
                if (!isSimReady()) {
                    // sim is not ready, return fail to RilJ
                    RFX_LOG_D(RFX_LOG_TAG, "onHandleRequest, sim is not ready, return fail to RilJ.");
                    sp<RfxMessage> un_set_capability_request =
                    RfxMessage::obtainResponse(RIL_E_GENERIC_FAILURE, message, false);
                    responseToRilj(un_set_capability_request);
                    return true;
                }
                int modemOffState = getNonSlotScopeStatusManager()->getIntValue(
                    RFX_STATUS_KEY_MODEM_OFF_STATE, MODEM_OFF_IN_IDLE);
                if (modemOffState == MODEM_OFF_IN_IDLE) {
                    char sessionId[PROPERTY_VALUE_MAX];
                    getNonSlotScopeStatusManager()->setIntValue(RFX_STATUS_KEY_MODEM_OFF_STATE,
                       MODEM_OFF_BY_SIM_SWITCH);
                    getNonSlotScopeStatusManager()->setIntValue(
                            RFX_STATUS_KEY_CAPABILITY_SWITCH_STATE,
                            CAPABILITY_SWITCH_STATE_START);

                    memset(sessionId, 0, sizeof(sessionId));
                    sprintf(sessionId,"%d",getSession(message));
                    // RpCapabilityGetController needs the sessionId to set MODEM_OFF_STATE.
                    property_set("vendor.ril.rc.session.id1", sessionId);
                    RFX_LOG_D(RFX_LOG_TAG, "onHandleRequest, Phase START, power off radio, slot:%d", message->getSlotId());
                    mCloseRadioCount = 0;
                    for (int slot = 0; slot < RFX_SLOT_COUNT; slot++) {
                        sp<RfxAction> action0 = new RfxAction2<int, const sp<RfxMessage>>(this,
                                &RpCapabilitySwitchController::onRequestRadioOffDone, message->getSlotId(), message);
                        RpRadioController* radioController0 =
                                (RpRadioController *)findController(slot,
                                RFX_OBJ_CLASS_INFO(RpRadioController));
                        radioController0->capabilitySwitchRadioOff(true, true, action0);
                    }
                } else if (modemOffState == MODEM_OFF_BY_SIM_SWITCH) {
                    // sim switch get the key
                    if(strncmp(tmp, "ready", 5) != 0) {
                        RFX_LOG_D(RFX_LOG_TAG, "onRequestRadioOffDone, MD3 not ready");
                        sp<RfxMessage> un_set_capability_request =
                        RfxMessage::obtainResponse(RIL_E_GENERIC_FAILURE, message, false);
                        responseToRilj(un_set_capability_request);
                    } else if(message->getSlotId() == c2kSlot) {
                        RFX_LOG_D(RFX_LOG_TAG, "onHandleRequest, Phase START, to c2k, slotId = %d, c2k_slot = %d, %s",
                            message->getSlotId(), c2kSlot, tmp);
                        requestToRild(c2k_request);
                    } else {
                        RFX_LOG_D(RFX_LOG_TAG, "onHandleRequest, Phase START, to gsm, slotId = %d, c2k_slot = %d, %s",
                            message->getSlotId(), c2kSlot, tmp);
                        requestToRild(message);
                    }
                } else {
                    RIL_Errno err;
                    bool modemPowerOff = getStatusManager()->getBoolValue(
                        RFX_STATUS_KEY_MODEM_POWER_OFF, false);
                    RFX_LOG_D(RFX_LOG_TAG, "onHandleRequest, reject switch, state key = %d, %d",
                        modemOffState, modemPowerOff);

                    // in airplane mode, reture radio unavailable
                    if (modemPowerOff) {
                        err = RIL_E_RADIO_NOT_AVAILABLE;
                    } else {
                        err = RIL_E_GENERIC_FAILURE;
                    }

                    // do not get the key,return fail to RilJ
                    sp<RfxMessage> un_set_capability_request =
                        RfxMessage::obtainResponse(err, message, false);
                    responseToRilj(un_set_capability_request);
                    return true;
                }
            } else {
                RFX_LOG_D(RFX_LOG_TAG, "onHandleRequest, other Phase always to GSM");
                requestToRild(message);
            }
        }
        break;
        default:
            RFX_LOG_E(RFX_LOG_TAG, "onHandleRequest, unknown message: %d", msg_id);
            break;
    }
    return true;
}

bool RpCapabilitySwitchController::onHandleResponse(const sp<RfxMessage>& message) {
    int msg_id = message->getId();
    switch (msg_id) {
    case RIL_REQUEST_SET_RADIO_CAPABILITY:
        handleSetRadioCapabilityResponse(message);
        break;
    default:
        responseToRilj(message);
        break;
    }
    return true;
}

void RpCapabilitySwitchController::handleSetRadioCapabilityResponse(
    const sp<RfxMessage>& message) {
    if (isDssNoResetSupport()) {
        RIL_RadioCapability rc;
        getRadioCapability(message, rc);
        if (rc.phase == RC_PHASE_APPLY) {
            int c2k_slot = getStatusManager(RFX_SLOT_ID_UNKNOWN)->getIntValue(
                RFX_STATUS_KEY_CDMA_SOCKET_SLOT, 0);
            if (message->getSlotId() == c2k_slot) {
                mC2kChannelLocked = true;
            }
        }
    }
    responseToRilj(message);
}

void RpCapabilitySwitchController::onRequestRadioOffDone(int slotId, const sp<RfxMessage> message) {
    mCloseRadioCount++;
    if(mCloseRadioCount < RFX_SLOT_COUNT){
        RFX_LOG_D(RFX_LOG_TAG, "onRequestRadioOffDone, mCloseRadioCount=%d", mCloseRadioCount);
        return;
    }
    mCloseRadioCount = -1; //all radio were closed, reset mCloseRadioCount.
    mC2kChannelLocked = false;
    int msg_id = message->getId();
    // RFX_LOG_D(RFX_LOG_TAG, "onRequestRadioOffDone, slotId:%d", slotId);
    sp<RfxMessage> c2k_request =
                        RfxMessage::obtainRequest(RADIO_TECH_GROUP_C2K, msg_id, message,true);
    char tmp[PROPERTY_VALUE_MAX] = {0};
    property_get(PROPERTY_NET_CDMA_MDMSTAT, tmp, "not");
    int c2kSlot = getStatusManager(RFX_SLOT_ID_UNKNOWN)->getIntValue(
            RFX_STATUS_KEY_CDMA_SOCKET_SLOT, 0);
    // RFX_LOG_D(RFX_LOG_TAG, "onRequestRadioOffDone, C2K slot=%d, %s", c2kSlot, tmp);
    if(strncmp(tmp, "ready", 5) != 0) {
        RFX_LOG_D(RFX_LOG_TAG, "onRequestRadioOffDone, MD3 not ready");
        sp<RfxMessage> un_set_capability_request =
        RfxMessage::obtainResponse(RIL_E_GENERIC_FAILURE, message, false);
        responseToRilj(un_set_capability_request);
    } else if(slotId == c2kSlot) {
        RFX_LOG_D(RFX_LOG_TAG, "onRequestRadioOffDone, to c2k, slotId = %d, c2k_slot = %d, %s",
            slotId, c2kSlot, tmp);
        requestToRild(c2k_request);
    } else {
        RFX_LOG_D(RFX_LOG_TAG, "onRequestRadioOffDone, to gsm, slotId = %d, c2k_slot = %d, %s",
            slotId, c2kSlot, tmp);
        requestToRild(message);
    }
}

int RpCapabilitySwitchController::checkPhase(const sp<RfxMessage>& message) {
    int status;
    int request;
    int token;
    RIL_RadioCapability rc;
    int t = 0;
    Parcel* p = message->getParcel();

    memset (&rc, 0, sizeof(RIL_RadioCapability));

    status = p->readInt32(&t);
    rc.version = (int)t;

    status = p->readInt32(&t);
    rc.session= (int)t;

    status = p->readInt32(&t);
    rc.phase= (int)t;

    status = p->readInt32(&t);
    rc.rat = (int)t;

   if (rc.phase == RC_PHASE_APPLY){
        RFX_LOG_D(RFX_LOG_TAG, "check phase, RC_PHASE_APPLY");
   } else {
        if (rc.phase == RC_PHASE_START){
            //Set property for modeController to check if simSwitching,
            //It will be reset in ril init after TRM or RC_FINISH
            RFX_LOG_D(RFX_LOG_TAG, "check phase, START and set switching flag");
            property_set("vendor.ril.cdma.switching", "1");
        } else if (rc.phase == RC_PHASE_FINISH) {
            int modemOffState = getNonSlotScopeStatusManager()->getIntValue(
                RFX_STATUS_KEY_MODEM_OFF_STATE, MODEM_OFF_IN_IDLE);

            //Reset switching flag when FINISH,because sim switch may fail and not do TRM
            RFX_LOG_D(RFX_LOG_TAG, "check phase, FINISH, reset switching flag, modemOffState = %d",
                modemOffState);
            property_set("vendor.ril.cdma.switching", "0");

            if (modemOffState == MODEM_OFF_BY_SIM_SWITCH) {
                // RFX_LOG_D(RFX_LOG_TAG, "Reset modemOffState on finish stag");
                getNonSlotScopeStatusManager()->setIntValue(RFX_STATUS_KEY_MODEM_OFF_STATE,
                                      MODEM_OFF_IN_IDLE);
                getNonSlotScopeStatusManager()->setIntValue(
                        RFX_STATUS_KEY_CAPABILITY_SWITCH_STATE, CAPABILITY_SWITCH_STATE_IDLE);
            }

            char tempstr[PROPERTY_VALUE_MAX];
            memset(tempstr, 0, sizeof(tempstr));
            property_get("persist.vendor.radio.simswitch", tempstr, "1");
            int capabilitySlotId = atoi(tempstr) - 1;
            getNonSlotScopeStatusManager()->setIntValue(RFX_STATUS_KEY_MAIN_CAPABILITY_SLOT, capabilitySlotId);
        } else {
            RFX_LOG_D(RFX_LOG_TAG, "check phase = %d", rc.phase);
        }
   }
   return rc.phase;
}

bool RpCapabilitySwitchController::checkPhaseIfFinish(const sp<RfxMessage>& message) {
    int status;
    int request;
    int token;
    RIL_RadioCapability rc;
    int t = 0;
    Parcel* p = message->getParcel();
    memset (&rc, 0, sizeof(RIL_RadioCapability));

    status = p->readInt32(&t);
    rc.version = (int)t;

    status = p->readInt32(&t);
    rc.session= (int)t;

    status = p->readInt32(&t);
    rc.phase= (int)t;

    status = p->readInt32(&t);
    rc.rat = (int)t;

    message->resetParcelDataStartPos();

    if ((rc.phase == RC_PHASE_FINISH) || (rc.phase == RC_PHASE_CONFIGURED)){
        RFX_LOG_D(RFX_LOG_TAG, "checkPhaseIfFinish true");
        return true;
    } else {
        return false;
    }
}


const char* RpCapabilitySwitchController::requestToString(int reqId) {
    switch (reqId) {
        case RIL_REQUEST_SET_RADIO_CAPABILITY:
            return "SET_RADIO_CAPABILITY";
        default:
            RFX_LOG_E(RFX_LOG_TAG, "requestToString, reqId: %d", reqId);
            return "UNKNOWN_REQUEST";
    }
}

const char* RpCapabilitySwitchController::urcToString(int urcId) {
    switch (urcId) {
        default:
            RFX_LOG_E(RFX_LOG_TAG, "requestToString, urcId: %d", urcId);
            return "UNKNOWN_URC";
    }
}

bool RpCapabilitySwitchController::isSimReady() {
    int sim_state = RFX_SIM_STATE_READY;
    int c2k_lte_mode = 0;
    char property_value[PROPERTY_VALUE_MAX] = { 0 };

    property_get("ro.vendor.mtk_c2k_lte_mode", property_value, "0");
    c2k_lte_mode = atoi(property_value);
    if (c2k_lte_mode == 1 || c2k_lte_mode == 2) {
        RFX_LOG_D(RFX_LOG_TAG, "isSimReady always return true in C2K");
        return true;
    }

    for (int i = 0; i < RFX_SLOT_COUNT; i++) {
        sim_state = getStatusManager(i)->getIntValue(RFX_STATUS_KEY_SIM_STATE);
        if (sim_state == RFX_SIM_STATE_LOCKED) {
            RFX_LOG_D(RFX_LOG_TAG, "isSimReady=false, slod=%d, sim_state=%d", i, sim_state);
            return false;
        }
    }
    RFX_LOG_D(RFX_LOG_TAG, "isSimReady=true");
    return true;
}

bool RpCapabilitySwitchController::isSvlteSupport() {
    int svlteSupport = 0;
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ro.vendor.mtk_c2k_lte_mode", property_value, "0");
    svlteSupport = atoi(property_value);
    return (svlteSupport == 1);
}

int RpCapabilitySwitchController::getSession(const sp<RfxMessage>& message) {
    int status;
    RIL_RadioCapability rc;
    int t = 0;
    Parcel* p = message->getParcel();

    memset (&rc, 0, sizeof(RIL_RadioCapability));
    message->resetParcelDataStartPos();
    status = p->readInt32(&t);
    rc.version = (int)t;
    status = p->readInt32(&t);
    rc.session= (int)t;
    message->resetParcelDataStartPos();
    RFX_LOG_D(RFX_LOG_TAG, "getSession,rc.session = %d",rc.session);
    return rc.session;
}

void RpCapabilitySwitchController::getRadioCapability(
    const sp<RfxMessage>& message, RIL_RadioCapability& rc) {
    Parcel* p = message->getParcel();
    message->resetParcelDataStartPos();
    p->readInt32(&(rc.version));
    p->readInt32(&(rc.session));
    p->readInt32(&(rc.phase));
    p->readInt32(&(rc.rat));
    strncpy(rc.logicalModemUuid, String8(p->readString16()).string(),
            MAX_UUID_LENGTH - 1);
    rc.logicalModemUuid[MAX_UUID_LENGTH - 1] = '\0';
    p->readInt32(&(rc.status));
    message->resetParcelDataStartPos();
}
