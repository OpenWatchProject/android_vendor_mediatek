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
#include "RpNwPhoneTypeUpdater.h"
#include "RpNwRatController.h"
#include "RpNwDefs.h"
#include "power/RadioConstants.h"

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/
RFX_IMPLEMENT_CLASS("RpNwPhoneTypeUpdater", RpNwPhoneTypeUpdater, RfxController);

RpNwPhoneTypeUpdater::RpNwPhoneTypeUpdater() {
    logD(RP_NW_PTU_TAG, " Constructor RpNwPhoneTypeUpdater");
    mLastDataRadioTech = RADIO_TECH_UNKNOWN;
}

RpNwPhoneTypeUpdater::~RpNwPhoneTypeUpdater() {
}

void RpNwPhoneTypeUpdater::onInit() {
    RfxController::onInit(); // Required: invoke super class implementation
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_SERVICE_STATE,
            RfxStatusChangeCallback(this, &RpNwPhoneTypeUpdater::onServiceStateChanged));

    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_NWS_MODE,
            RfxStatusChangeCallback(this, &RpNwPhoneTypeUpdater::onNwsModeChanged));
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_RADIO_CAPABILITY,
            RfxStatusChangeCallback(this, &RpNwPhoneTypeUpdater::onRadioCapabilityChanged));

    int mode = getStatusManager()->getIntValue(RFX_STATUS_KEY_NWS_MODE, NWS_MODE_CSFB);
    if (mode == NWS_MODE_CDMALTE) {
        getStatusManager()->setIntValue(RFX_STATUS_KEY_VOICE_TYPE, RADIO_TECH_GROUP_C2K);
        getStatusManager()->setIntValue(RFX_STATUS_KEY_DATA_TYPE, RADIO_TECH_GROUP_C2K);
    } else {
        getStatusManager()->setIntValue(RFX_STATUS_KEY_VOICE_TYPE, RADIO_TECH_GROUP_GSM);
        getStatusManager()->setIntValue(RFX_STATUS_KEY_DATA_TYPE, RADIO_TECH_GROUP_GSM);
    }
    logD(RP_NW_PTU_TAG, " onInit() mode:%d", mode);
}

void RpNwPhoneTypeUpdater::onNwsModeChanged(RfxStatusKeyEnum key,
        RfxVariant oldValue, RfxVariant newValue) {
    RFX_UNUSED(key);
    int oldType = oldValue.asInt();
    int newType = newValue.asInt();
    int csType = RADIO_TECH_GROUP_GSM;

    if (newType == NWS_MODE_CDMALTE) {
        csType = RADIO_TECH_GROUP_C2K;
        if (mLastDataRadioTech == RADIO_TECH_UNKNOWN
                || RfxNwServiceState::isCdmaGroup(mLastDataRadioTech)) {
            getStatusManager()->setIntValue(RFX_STATUS_KEY_DATA_TYPE, RADIO_TECH_GROUP_C2K);
        }
    } else if (newType == NWS_MODE_CSFB || newType == NWS_MODE_LTEONLY) {
        getStatusManager()->setIntValue(RFX_STATUS_KEY_DATA_TYPE, RADIO_TECH_GROUP_GSM);
    }
    getStatusManager()->setIntValue(RFX_STATUS_KEY_VOICE_TYPE, csType);
    logD(RP_NW_PTU_TAG, " SIM%d NwsMode changed:%d, csType:%d, mLastDataRadioTech:%d",
            m_slot_id, newType, csType, mLastDataRadioTech);
}

void RpNwPhoneTypeUpdater::onServiceStateChanged(RfxStatusKeyEnum key,
        RfxVariant oldValue, RfxVariant newValue) {
    RFX_UNUSED(key);
    RfxNwServiceState oldSS = oldValue.asServiceState();
    RfxNwServiceState newSS = newValue.asServiceState();

    if (getNonSlotScopeStatusManager()->getIntValue(RFX_STATUS_KEY_CDMA_SOCKET_SLOT, 0)
            == m_slot_id) {
        logD(RP_NW_PTU_TAG, " SIM%d onServiceStateChanged: newSS: %s",
                m_slot_id, newSS.toString().string());
        if (oldSS.getRilVoiceRegState() != newSS.getRilVoiceRegState()
                || oldSS.getRilVoiceRadioTech() != newSS.getRilVoiceRadioTech()) {
            onVoiceServiceStateChanged(oldSS, newSS);
        }

        if (oldSS.getRilDataRegState() != newSS.getRilDataRegState()
                || oldSS.getRilDataRadioTech() != newSS.getRilDataRadioTech()) {
            updatePsType(newSS);
        }
    } else {
        getStatusManager()->setIntValue(RFX_STATUS_KEY_VOICE_TYPE, RADIO_TECH_GROUP_GSM);
        getStatusManager()->setIntValue(RFX_STATUS_KEY_DATA_TYPE, RADIO_TECH_GROUP_GSM);
    }
}

void RpNwPhoneTypeUpdater::onVoiceServiceStateChanged(RfxNwServiceState oldSS,
        RfxNwServiceState newSS) {
    RFX_UNUSED(oldSS);
    int regState = newSS.getRilVoiceRegState();
    int radioTech = newSS.getRilVoiceRadioTech();
    int csType = RADIO_TECH_GROUP_GSM;

    logD(RP_NW_PTU_TAG, " SIM%d Voice service state changed: regState=%d radioTech=%d",
            m_slot_id, regState, radioTech);
    if (RfxNwServiceState::isInService(regState)) {
        if (RfxNwServiceState::isGsmGroup(radioTech)) {
            // set PS phone type to GSM
            csType = RADIO_TECH_GROUP_GSM;
        } else if (RfxNwServiceState::isCdmaGroup(radioTech)) {
            // set PS phone type to C2K
            csType = RADIO_TECH_GROUP_C2K;
        } else {
            logD(RP_NW_PTU_TAG, " Invalid rat");
        }
    } else {
        // OOS -> decide by device mode
        if (getDeviceMode() == NWS_MODE_CSFB || getDeviceMode() == NWS_MODE_LTEONLY) {
            // set PS phone type to GSM
            csType = RADIO_TECH_GROUP_GSM;
        } else if (getDeviceMode() == NWS_MODE_CDMALTE) {
            // set PS phone type to C2K
            csType = RADIO_TECH_GROUP_C2K;
        } else {
            logD(RP_NW_PTU_TAG, " Invalid device mode");
        }
    }
    getStatusManager()->setIntValue(RFX_STATUS_KEY_VOICE_TYPE, csType);
}

void RpNwPhoneTypeUpdater::updatePsType(RfxNwServiceState serviceState) {
    int regState = serviceState.getRilDataRegState();
    int radioTech = serviceState.getRilDataRadioTech();
    int psType = RADIO_TECH_GROUP_GSM;

    logD(RP_NW_PTU_TAG, " SIM%d Data service state changed: regState=%d radioTech=%d",
            m_slot_id, regState, radioTech);
    if (RfxNwServiceState::isInService(regState)) {
        mLastDataRadioTech = radioTech;
        if (RfxNwServiceState::isCdmaGroup(radioTech)) {
            // set PS phone type to C2K
            psType = RADIO_TECH_GROUP_C2K;
        }
    } else {
        if (getDeviceMode() == NWS_MODE_CDMALTE) {
            int radioCapability = getStatusManager()->getIntValue(
                    RFX_STATUS_KEY_RADIO_CAPABILITY, RIL_CAPABILITY_NONE);
            if (radioCapability == RIL_CAPABILITY_CDMA_ONLY) {
                psType = RADIO_TECH_GROUP_C2K;
            } else {
                if (RfxNwServiceState::isCdmaGroup(mLastDataRadioTech)) {
                    psType = RADIO_TECH_GROUP_C2K;
                }
            }
        }
    }
    getStatusManager()->setIntValue(RFX_STATUS_KEY_DATA_TYPE, psType);
}

int RpNwPhoneTypeUpdater::getDeviceMode() {
    int mode = getStatusManager()->getIntValue(RFX_STATUS_KEY_NWS_MODE, NWS_MODE_CSFB);
    return mode;
}

const char *RpNwPhoneTypeUpdater::radioTechToString(int radioTech) {
    switch (radioTech) {
        case RADIO_TECH_GROUP_GSM:
            return "GSM";
        case RADIO_TECH_GROUP_C2K:
            return "C2K";
        default:
            return "INVALID RADIO TECH";
    }
}

bool RpNwPhoneTypeUpdater::needUpdatePsTypeForCapChange(RfxNwServiceState curSS,
        int oldCap, int newCap) {
    bool ret = false;
    bool radioCapabilityChange = (oldCap == RIL_CAPABILITY_CDMA_ONLY
            && newCap != RIL_CAPABILITY_CDMA_ONLY)
            || (oldCap != RIL_CAPABILITY_CDMA_ONLY
            && newCap == RIL_CAPABILITY_CDMA_ONLY);
    ret = radioCapabilityChange
            && getDeviceMode() == NWS_MODE_CDMALTE
            && !RfxNwServiceState::isInService(curSS.getRilDataRegState());
    return ret;
}

void RpNwPhoneTypeUpdater::onRadioCapabilityChanged(RfxStatusKeyEnum key,
        RfxVariant oldValue, RfxVariant newValue) {
    RFX_UNUSED(key);
    int old_cap = oldValue.asInt();
    int new_cap = newValue.asInt();
    if (getNonSlotScopeStatusManager()->getIntValue(RFX_STATUS_KEY_CDMA_SOCKET_SLOT, 0)
            == m_slot_id) {
        // Need update data service state changed for radio capability change
        RfxNwServiceState curSS = getStatusManager()
                ->getServiceStateValue(RFX_STATUS_KEY_SERVICE_STATE);
        if (needUpdatePsTypeForCapChange(curSS, old_cap, new_cap)) {
            logD(RP_NW_PTU_TAG, " SIM%d Radio capability changed:%d -> %d",
                    m_slot_id, old_cap, new_cap);
            updatePsType(curSS);
        }
    }
}
