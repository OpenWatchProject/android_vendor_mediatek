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
#include "RfxLog.h"
#include "RpIrCdmaHandler.h"
#include "RpIrUtils.h"
#include "power/RadioConstants.h" // move to RfxStatusDefs.h
#include "util/RpFeatureOptionUtils.h"

#define RFX_LOG_TAG "[IRC][CdmaHandler]"

/*****************************************************************************
 * Class RpIrCdmaHandler
 *****************************************************************************/

RFX_IMPLEMENT_CLASS("RpIrCdmaHandler", RpIrCdmaHandler, RpIrBaseHandler);

RpIrCdmaHandler::RpIrCdmaHandler() :
        RpIrBaseHandler(),
        mIsCT3GCardType(false) {
}

RpIrCdmaHandler::RpIrCdmaHandler(RpIrController *ir_controller) :
        RpIrBaseHandler(ir_controller),
        mIsCT3GCardType(false) {
}

RpIrCdmaHandler::~RpIrCdmaHandler() {
}

void RpIrCdmaHandler::setNwsMode(NwsMode nwsMode) {
    logD(RFX_LOG_TAG, "setNwsMode: %s", Nws2Str(nwsMode));
    if (nwsMode != mPtrIrController->getNwsMode()) {
        sp<RfxAction> action = new RfxAction0(this, &RpIrCdmaHandler::onNwsModeChanged);
        mPtrIrController->setNwsMode(nwsMode, action);
    } else {
        mPtrIrController->setNwsMode(nwsMode, sp<RfxAction>(NULL));
    }
    setState(STATE_NETWORK_SELECTED);
}

void RpIrCdmaHandler::resumeNetwork() {
    NwsMode currentNwsMode = mPtrIrController->getNwsMode();
    logD(RFX_LOG_TAG, "resumeNetwork: currentNwsMode: %s", Nws2Str(currentNwsMode));
    sp<RfxMessage> resumeCdmaRegRequest =
            RfxMessage::obtainRequest(m_slot_id, RADIO_TECH_GROUP_C2K,
            RIL_REQUEST_RESUME_REGISTRATION_CDMA);
    requestToRild(resumeCdmaRegRequest);
}

void RpIrCdmaHandler::dispose() {
  unregisterBaseListener();
  unregisterPlmnChangedListener();
  unregisterForServiceStateChanged();
}

void RpIrCdmaHandler::setIfEnabled(bool isEnabled) {
    enableSuspend(isEnabled);
}

bool RpIrCdmaHandler::enableNoServiceDelay() {
    return false;
}

void RpIrCdmaHandler::onInit() {
    RpIrBaseHandler::onInit();
    registerBaseListener();
    setState(STATE_INIT);
}

bool RpIrCdmaHandler::onHandleUrc(const sp<RfxMessage>& message) {
    int id = message->getId();

    switch (id) {
    case RIL_UNSOL_CDMA_PLMN_CHANGED:
        handlePlmnChanged(message);
        return true;
        break;
    default:
        break;
    }
    return false;
}

void RpIrCdmaHandler::handlePlmnChanged(const sp<RfxMessage>& message) {
    logD(RFX_LOG_TAG, "handlePlmnChanged, msg = %s", message->toString().string());
    if (getState() == STATE_INIT) {
        logD(RFX_LOG_TAG, "plmn changed in INIT state");
        return;
    }

    postponeNoServiceMessageIfNeeded(NO_SERVICE_DELAY_TIME);
    if (message->getError() == RIL_E_SUCCESS) {
        Parcel *p = message->getParcel();
        int32_t plmn_count = p->readInt32();
        String16 plmn = p->readString16();
        String16 mnc = p->readString16();
        plmn.append(mnc);
        String8 plmn8(plmn);
        setState(STATE_SELECTING_NETWORK);
        processPlmnChanged(plmn8);
    }
}


void RpIrCdmaHandler::onCT3GDualModeCardChanged(RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    RFX_UNUSED(old_value);
    mIsCT3GCardType = value.asBool();
    logD(RFX_LOG_TAG, "onCT3GDualModeCardChanged, mIsCT3GCardType = %s",
                      Bool2Str(mIsCT3GCardType));
    if (mIsCT3GCardType && getSlotId() == RpIrUtils::getCdmaSlotId()
            && mPtrIrController->getIrControllerState() == true) {
        enableSuspend(true);
        mPtrIrController->set3GIrEnableState(true);
    }
}

void RpIrCdmaHandler::onServiceStateChanged(RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    RFX_UNUSED(old_value);
    if (getState() == STATE_INIT) {
        logD(RFX_LOG_TAG, "onServiceStateChanged in INIT state");
        return;
    }
    RIL_RADIO_CAPABILITY radio_cap;
    radio_cap = (RIL_RADIO_CAPABILITY)getStatusManager()->getIntValue(RFX_STATUS_KEY_RADIO_CAPABILITY);

    if (!CAPABILITY_HAS_CDMA(radio_cap) || mPtrIrController->getNwsMode() == NWS_MODE_CSFB) {
        logD(RFX_LOG_TAG, "onServiceStateChanged but CDMA radio off, ignore");
        return;
    }
    RfxNwServiceState serviceState = (RfxNwServiceState) value.asServiceState();
    int regVoiceState = convertVoiceRegState(serviceState.getRilVoiceRegState());
    int regDataState = serviceState.getRilDataRegState();
    logD(RFX_LOG_TAG, "onServiceStateChanged, VoiceState:%d, DataState:%d",
            regVoiceState, regDataState);
    if (regVoiceState == RIL_REG_STATE_HOME
            || regVoiceState == RIL_REG_STATE_ROAMING
            || regDataState == RIL_REG_STATE_HOME
            || regDataState == RIL_REG_STATE_ROAMING
            || (getStatusManager()->getBoolValue(RFX_STATUS_KEY_CT3G_DUALMODE_CARD)
                    && RpIrUtils::isSimLocked(getSlotId())
                    && serviceState.getCdmaNetworkExist())) {
        setServiceType(IN_SERVICE);
    } else if ((regVoiceState == RIL_REG_STATE_NOT_REG_NOT_SEARCHING
                || regVoiceState == RIL_REG_STATE_UNKNOWN)
               && (regDataState == RIL_REG_STATE_NOT_REG_NOT_SEARCHING
                   || regDataState == RIL_REG_STATE_UNKNOWN)) {
        setServiceType(OUT_OF_SERVICE);
    } else {
        setServiceType(IN_SEARCHING);
    }

    mPreVoiceState = regVoiceState;
    mPreDataState = regDataState;
}


void RpIrCdmaHandler::onSocketStateChanged(RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    RfxSocketState old_state = old_value.asSocketState();
    RfxSocketState new_state = value.asSocketState();

    bool old_c2k = old_state.getSocketState(RfxSocketState::SOCKET_C2K);
    bool new_c2k = new_state.getSocketState(RfxSocketState::SOCKET_C2K);

    logD(RFX_LOG_TAG, "onSocketStateChanged, state = %s, old = %s, new = %s",
                          stateToString(getState()), Bool2Str(old_c2k), Bool2Str(new_c2k));

    if (old_c2k == new_c2k) {
        return;
    }

    if (new_c2k) {
        if (getState() == STATE_INIT) {
            removeNoServiceMessage();
            if (mPtrIrController->getIrControllerEnableState()) {
                enableSuspend(true);
                resetToInitialState();
            }
            registerPlmnChangedListener();
            setState(STATE_GETTING_PLMN);
        }
    } else {
        if (!RpIrUtils::isViceOfTwoCtDualVolte(getSlotId())) {
            unregisterForServiceStateChanged();
            unregisterPlmnChangedListener();
            setState(STATE_INIT);
            mPreVoiceState = -1;
            mPreDataState = -1;
        }
        if (mPtrListener) {
            mPtrListener->onRadioStateChanged(false);
        }
        setServiceType(OUT_OF_SERVICE);
    }
}

void RpIrCdmaHandler::onRadioCapabilityChanged(RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    RIL_RADIO_CAPABILITY old_cap, new_cap;
    old_cap = (RIL_RADIO_CAPABILITY)old_value.asInt();
    new_cap = (RIL_RADIO_CAPABILITY)value.asInt();

    logD(RFX_LOG_TAG, "onRadioCapabilityChanged, state = %s, old = %d, new = %d",
                      stateToString(getState()), old_cap, new_cap);

    if (!CAPABILITY_HAS_CDMA(old_cap) && CAPABILITY_HAS_CDMA(new_cap)) {
        removeNoServiceMessage();
        registerForServiceStateChanged();
        if (mPtrListener) {
            mPtrListener->onRadioStateChanged(true);
        }
        mPreVoiceState = -1;
        mPreDataState = -1;
    } else if (CAPABILITY_HAS_CDMA(old_cap) && !CAPABILITY_HAS_CDMA(new_cap)) {
        if (!RpIrUtils::isViceOfTwoCtDualVolte(getSlotId())) {
            unregisterForServiceStateChanged();
            mPreVoiceState = -1;
            mPreDataState = -1;
        }
        if (getState() != STATE_INIT) {
            setState(STATE_NO_SERVICE);
        }
        if (mPtrListener) {
            mPtrListener->onRadioStateChanged(false);
        }
        mServiceType = OUT_OF_SERVICE;
    }
}

void RpIrCdmaHandler::registerBaseListener() {
    logD(RFX_LOG_TAG, "registerBaseListener");
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_SOCKET_STATE,
        RfxStatusChangeCallback(this, &RpIrCdmaHandler::onSocketStateChanged));
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_RADIO_CAPABILITY,
        RfxStatusChangeCallback(this, &RpIrCdmaHandler::onRadioCapabilityChanged));
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_CT3G_DUALMODE_CARD,
        RfxStatusChangeCallback(this, &RpIrCdmaHandler::onCT3GDualModeCardChanged));
}

void RpIrCdmaHandler::unregisterBaseListener() {
    logD(RFX_LOG_TAG, "unregisterBaseListener");
    getStatusManager()->unRegisterStatusChanged(RFX_STATUS_KEY_SOCKET_STATE,
        RfxStatusChangeCallback(this, &RpIrCdmaHandler::onSocketStateChanged));
    getStatusManager()->unRegisterStatusChanged(RFX_STATUS_KEY_RADIO_CAPABILITY,
        RfxStatusChangeCallback(this, &RpIrCdmaHandler::onRadioCapabilityChanged));
    getStatusManager()->unRegisterStatusChanged(RFX_STATUS_KEY_CT3G_DUALMODE_CARD,
        RfxStatusChangeCallback(this, &RpIrCdmaHandler::onCT3GDualModeCardChanged));
}

void RpIrCdmaHandler::registerPlmnChangedListener() {
    logD(RFX_LOG_TAG, "registerPlmnChangedListener");
    const int urc_id_list[] = {
        RIL_UNSOL_CDMA_PLMN_CHANGED
    };
    if (RpFeatureOptionUtils::isC2kSupport()) {
        registerToHandleUrc(urc_id_list, sizeof(urc_id_list)/sizeof(int));
    }
}

void RpIrCdmaHandler::unregisterPlmnChangedListener() {
    logD(RFX_LOG_TAG, "unregisterPlmnChangedListener");
    const int urc_id_list[] = {
        RIL_UNSOL_CDMA_PLMN_CHANGED
    };
    if (RpFeatureOptionUtils::isC2kSupport()) {
        unregisterToHandleUrc(urc_id_list, sizeof(urc_id_list)/sizeof(int));
    }
}

void RpIrCdmaHandler::registerForServiceStateChanged() {
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_SERVICE_STATE,
        RfxStatusChangeCallback(this, &RpIrCdmaHandler::onServiceStateChanged));
}

void RpIrCdmaHandler::unregisterForServiceStateChanged() {
    logD(RFX_LOG_TAG, "unregisterForServiceStateChanged");
    getStatusManager()->unRegisterStatusChanged(RFX_STATUS_KEY_SERVICE_STATE,
        RfxStatusChangeCallback(this, &RpIrCdmaHandler::onServiceStateChanged));
}

void RpIrCdmaHandler::enableSuspend(bool enabled) {
    logD(RFX_LOG_TAG, "enableSuspend: %s", Bool2Str(enabled));
    if (!RpIrUtils::isSupportRoaming(getSlotId())) {
        logD(RFX_LOG_TAG, "enableSuspend: not support roaming, return");
        return;
    }

    sp<RfxMessage> cdmaRegSuspendEnabledRequest =
            RfxMessage::obtainRequest(m_slot_id, RADIO_TECH_GROUP_C2K,
            RIL_REQUEST_SET_REG_SUSPEND_ENABLED_CDMA);
    Parcel* p = cdmaRegSuspendEnabledRequest->getParcel();
    p->writeInt32(1);
    p->writeInt32(enabled ? 1 : 0);

    // send request to C2K RILD
    requestToRild(cdmaRegSuspendEnabledRequest);
}
