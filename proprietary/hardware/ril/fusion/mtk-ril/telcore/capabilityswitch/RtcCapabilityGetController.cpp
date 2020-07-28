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
#include "RtcCapabilityGetController.h"
#include "RtcCapabilitySwitchController.h"
#include "RtcCapabilitySwitchUtil.h"
#include "RfxRadioCapabilityData.h"
#include "rfx_properties.h"
#include "RfxRilUtils.h"
/*****************************************************************************
 * Class RtcCapabilityGetController
 *****************************************************************************/
#define RFX_LOG_TAG "RtcCapa"

RFX_IMPLEMENT_CLASS("RtcCapabilityGetController", RtcCapabilityGetController, RfxController);

RFX_REGISTER_DATA_TO_URC_ID(RfxRadioCapabilityData, RFX_MSG_URC_RADIO_CAPABILITY);

RtcCapabilityGetController::RtcCapabilityGetController() : m_cslot(0), m_first_urc(true) {
    logI(RFX_LOG_TAG, "constructor entered");
}

RtcCapabilityGetController::~RtcCapabilityGetController() {
}

void RtcCapabilityGetController::onInit() {
    RfxController::onInit();

    const int urc_id_list[] = {
        RFX_MSG_URC_RADIO_CAPABILITY,
    };
    logD(RFX_LOG_TAG, "onInit");

    // register request & URC id list
    // NOTE. one id can only be registered by one controller
    registerToHandleUrc(urc_id_list, sizeof(urc_id_list) / sizeof(const int));
    char tempstr[RFX_PROPERTY_VALUE_MAX] = { 0 };
    memset(tempstr, 0, sizeof(tempstr));
    rfx_property_get("persist.vendor.radio.c_capability_slot", tempstr, "1");
    m_cslot = atoi(tempstr) - 1;
}

bool RtcCapabilityGetController::onHandleUrc(const sp<RfxMessage> &message) {
    int msg_id = message->getId();
    RIL_RadioCapability *capability = (RIL_RadioCapability *)message->getData()->getData();

    logD(RFX_LOG_TAG, "onHandleUrc, handle: %s", idToString(msg_id));
    logI(RFX_LOG_TAG, "RadioCapability version=%d, session=%d, phase=%d, rat=%d, uuid=%s, state=%d",
         capability->version, capability->session, capability->phase, capability->rat,
         capability->logicalModemUuid, capability->status);
    if (msg_id == RFX_MSG_URC_RADIO_CAPABILITY) {
        // 3. if the URC need to be sent to RILJ, send it,
        //    be able to update parceled data if required
        responseToRilj(message);
    }
    return true;
}

// Only allow to be called by RtcModeSwitchController for once after SIM switch
void RtcCapabilityGetController::updateRadioCapability(int cslot) {
    char property_value[RFX_PROPERTY_VALUE_MAX] = { 0 };
    int session_id;

    rfx_property_get("vendor.ril.rc.session.id1", property_value, "-1");
    session_id = atoi(property_value);
    if (session_id != -1 || getNonSlotScopeStatusManager()->getIntValue(
            RFX_STATUS_KEY_CAPABILITY_SWITCH_STATE) == CAPABILITY_SWITCH_STATE_ENDING) {
        updateRadioCapability(cslot, session_id);
        // Clear session id and set radio state if SIM switch has been done successfully
        rfx_property_set("vendor.ril.rc.session.id1", "-1");
        if (RtcCapabilitySwitchUtil::isDssNoResetSupport() == false) {
            for (int i = 0; i < RfxRilUtils::rfxGetSimCount(); i++) {
                getStatusManager(i)->setIntValue(
                        RFX_STATUS_KEY_RADIO_STATE, RADIO_STATE_OFF, false, false);
            }
        }
        getNonSlotScopeStatusManager()->registerStatusChanged(
                RFX_STATUS_KEY_MODESWITCH_FINISHED,
                RfxStatusChangeCallback(this, &RtcCapabilityGetController::onModeSwitchFinished));
    } else if (cslot != m_cslot || m_first_urc) {
        updateRadioCapability(cslot, session_id);
    } else {
        logI(RFX_LOG_TAG, "updateRadioCapability cslot=%d is same with before.", cslot);
    }
}

void RtcCapabilityGetController::onModeSwitchFinished(RfxStatusKeyEnum key, RfxVariant old_value,
                                                            RfxVariant value) {
    RFX_UNUSED(key);
    logD(RFX_LOG_TAG, "onModeSwitchFinished (%d, %d)", old_value.asInt(), value.asInt());
    RtcCapabilitySwitchController* switchController = (RtcCapabilitySwitchController *)
            findController(RFX_OBJ_CLASS_INFO(RtcCapabilitySwitchController));
    switchController->notifySetRatDone();
    getNonSlotScopeStatusManager()->unRegisterStatusChanged(
            RFX_STATUS_KEY_MODESWITCH_FINISHED,
            RfxStatusChangeCallback(this, &RtcCapabilityGetController::onModeSwitchFinished));
}

void RtcCapabilityGetController::updateRadioCapability(int cslot, int session_id) {
    char property_value[RFX_PROPERTY_VALUE_MAX] = { 0 };
    int radio_capability;

    m_cslot = cslot;
    m_first_urc = false;

    if (RtcCapabilitySwitchUtil::isDisableC2kCapability()) {
        for (int i = 0; i < RfxRilUtils::rfxGetSimCount(); i++) {
            radio_capability = getStatusManager(i)->getIntValue(RFX_STATUS_KEY_SLOT_CAPABILITY, 0);
            logI(RFX_LOG_TAG, "updateRadioCapability, no c2k, session=%d, capability[%d] = %d",
                 session_id, i, radio_capability);
            sendRadioCapabilityURC(i, session_id, radio_capability);
        }
    } else {
        for (int i = 0; i < RfxRilUtils::rfxGetSimCount(); i++) {
            radio_capability = getStatusManager(i)->getIntValue(RFX_STATUS_KEY_SLOT_CAPABILITY, 0);
            if (i == cslot) {
                radio_capability |= (RAF_CDMA_GROUP | RAF_EVDO_GROUP);
            } else {
                radio_capability &= ~(RAF_CDMA_GROUP | RAF_EVDO_GROUP);
            }
            logI(RFX_LOG_TAG, "updateRadioCapability, cslot=%d, session=%d, capability[%d] = %d",
                 cslot, session_id, i, radio_capability);
            getStatusManager(i)->setIntValue(
                    RFX_STATUS_KEY_SLOT_CAPABILITY, radio_capability, false, false);

            // always send CDMA capability from Android Q
            radio_capability |= (RAF_CDMA_GROUP | RAF_EVDO_GROUP);
            sendRadioCapabilityURC(i, session_id, radio_capability);
        }
    }
}

void RtcCapabilityGetController::sendRadioCapabilityURC(int slot_id, int session_id, int radio_capability) {
    RIL_RadioCapability rc;

    memset(&rc, 0, sizeof(RIL_RadioCapability));
    rc.version = RIL_RADIO_CAPABILITY_VERSION;
    rc.session = session_id;
    rc.phase = RC_PHASE_UNSOL_RSP;
    rc.status = RC_STATUS_SUCCESS;
    rc.rat = radio_capability;
    RfxRilUtils::getLogicalModemId(rc.logicalModemUuid, MAX_UUID_LENGTH, slot_id);
    sp<RfxMessage> urc = RfxMessage::obtainUrc(
            slot_id, RFX_MSG_URC_RADIO_CAPABILITY,
            RfxRadioCapabilityData(&rc, sizeof(RIL_RadioCapability)));
    responseToRilj(urc);
}

void RtcCapabilityGetController::updateRadioCapabilityForWMChange(int world_mode) {
    int fixed_radio_capability, radio_capability;
    int major_slot = getNonSlotScopeStatusManager()->getIntValue(
            RFX_STATUS_KEY_MAIN_CAPABILITY_SLOT, 0);

    if (RfxRilUtils::isTplusWSupport() || RfxRilUtils::getKeep3GMode() != 0) {
        return;
    }

    for (int i = 0; i < RfxRilUtils::rfxGetSimCount(); i++) {
        if (i == major_slot) {
            continue;
        }
        fixed_radio_capability = getStatusManager(i)->getIntValue(
                RFX_STATUS_KEY_SLOT_FIXED_CAPABILITY, 0);
        radio_capability = getStatusManager(i)->getIntValue(RFX_STATUS_KEY_SLOT_CAPABILITY, 0);
        if (world_mode == 2) {
            // Remove 3G raf for non major SIMs in TDD mode
            radio_capability &= ~RAF_UMTS;
        } else {
            radio_capability |= (fixed_radio_capability & RAF_UMTS);
        }
        getStatusManager(i)->setIntValue(
                RFX_STATUS_KEY_SLOT_CAPABILITY, radio_capability, false, false);
        if (RtcCapabilitySwitchUtil::isDisableC2kCapability() == false) {
            // always send CDMA capability from Android Q
            radio_capability |= (RAF_CDMA_GROUP | RAF_EVDO_GROUP);
        }
        sendRadioCapabilityURC(i, -1, radio_capability);
    }
}
