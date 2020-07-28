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
#include "RpIrBaseHandler.h"
#include "RpIrController.h"
#include <libmtkrilutils.h>

#define RFX_LOG_TAG "[IRC][BaseHandler]"
/*****************************************************************************
 * Class RpIrBaseHandler
 *****************************************************************************/

RFX_IMPLEMENT_CLASS("RpIrBaseHandler", RpIrBaseHandler, RpIrController);

RpIrBaseHandler::RpIrBaseHandler() :
    mPtrIrController(NULL),
    mServiceType(OUT_OF_SERVICE),
    mState(STATE_UNKNOWN),
    mPreState(STATE_UNKNOWN),
    mPreVoiceState(-1),
    mPreDataState(-1),
    mIsFirstRoundSearch(true),
    mNoServiceTimerHandle(NULL),
    mPtrListener(NULL),
    mPreviewPlmnHandled(false) {
}

RpIrBaseHandler::RpIrBaseHandler(RpIrController *ir_controller) :
    mPtrIrController(ir_controller),
    mServiceType(OUT_OF_SERVICE),
    mState(STATE_UNKNOWN),
    mPreState(STATE_UNKNOWN),
    mPreVoiceState(-1),
    mPreDataState(-1),
    mIsFirstRoundSearch(true),
    mNoServiceTimerHandle(NULL),
    mPtrListener(NULL),
    mPreviewPlmnHandled(false) {
}

RpIrBaseHandler::~RpIrBaseHandler() {
}

void RpIrBaseHandler::setNwsMode(NwsMode nwsMode) {
    RFX_UNUSED(nwsMode);
}

void RpIrBaseHandler::resumeNetwork() {
}

void RpIrBaseHandler::dispose() {
}

void RpIrBaseHandler::registerListener(IRpIrNwControllerListener* listener) {
    mPtrListener = listener;
}

void RpIrBaseHandler::startNewSearchRound() {
    logD(RFX_LOG_TAG, "startNewSearchRound");
    mIsFirstRoundSearch = true;
    mPreVoiceState = -1;
    mPreDataState = -1;
}

void RpIrBaseHandler::setIfEnabled(bool isEnabled) {
    RFX_UNUSED(isEnabled);
}


bool RpIrBaseHandler::enableNoServiceDelay() {
    return true;
}

const char *RpIrBaseHandler::stateToString(StateEnum nState) const {
    switch (nState) {
        case STATE_UNKNOWN:
            return "STATE_UNKNOWN";
        case STATE_INIT:
            return "STATE_INIT";
        case STATE_NO_SERVICE:
            return "STATE_NO_SERVICE";
        case STATE_GETTING_PLMN:
            return "STATE_GETTING_PLMN";
        case STATE_SELECTING_NETWORK:
            return "STATE_SELECTING_NETWORK";
        case STATE_NETWORK_SELECTED:
            return "STATE_NETWORK_SELECTED";
        default:
            return "STATE_INVALID";
    }
}

void RpIrBaseHandler::setState(StateEnum nState) {
    logD(RFX_LOG_TAG, "setState: %s, mState=%s, mPreState=%s", stateToString(nState),
            stateToString(mState), stateToString(mPreState));
    if (mState == nState) {
        return;
    }
    mPreState = mState;
    mState = nState;
    if (nState == STATE_INIT) {
        resetToInitialState();
    }
}

StateEnum RpIrBaseHandler::getState() const {
    return mState;
}

void RpIrBaseHandler::resetToInitialState() {
    logD(RFX_LOG_TAG, "resetToInitialState");
    mIsFirstRoundSearch = true;
    mPreVoiceState = -1;
    mPreDataState = -1;
    mVecPlmns.clear();
    mPreviewPlmnHandled = false;
}


void RpIrBaseHandler::setServiceType(ServiceType serviceType) {
    logD(RFX_LOG_TAG, "setServiceType(%s), mServiceType=%s", ServiceType2Str(serviceType),
                   ServiceType2Str(mServiceType));

    if (getState() != STATE_INIT) {
        if (serviceType != IN_SERVICE) {
            setState(STATE_NO_SERVICE);
        } else if (getState() == STATE_NO_SERVICE) {
            setState(mPreState);
        }
    }

    if (mServiceType != serviceType) {
        mServiceType = serviceType;
        if (serviceType != OUT_OF_SERVICE) {
            // if on service or searching, call listener immediaetlly
            removeNoServiceMessage();
            if (mPtrListener) {
                mPtrListener->onServiceStateChanged(serviceType);
            }
        } else {
            // need delay 20s to callback no service state
            // as the service would be back soon
            if (enableNoServiceDelay()) {
                sendNoServiceMessage(RpIrController::sNoServiceDelayTime);
            }
            else {
                // notify listener
                if (mPtrListener) {
                    mPtrListener->onServiceStateChanged(serviceType);
                }
            }
        }
    }
}

void RpIrBaseHandler::onNwsModeChanged() {
    logD(RFX_LOG_TAG, "onNwsModeChanged");
    if (mPtrListener) {
        mPtrListener->onNwsModeSwitchDone();
    }
}

void RpIrBaseHandler::sendNoServiceMessage(int nDelayedTime) {
    if (mNoServiceTimerHandle == NULL) {
        mNoServiceTimerHandle = RfxTimer::start(RfxCallback0(this,
                &RpIrBaseHandler::onNoServiceTimeout), ms2ns(nDelayedTime));
    }
}

void RpIrBaseHandler::removeNoServiceMessage() {
    if (mNoServiceTimerHandle == NULL) {
        return;
    }
    RfxTimer::stop(mNoServiceTimerHandle);
    mNoServiceTimerHandle = NULL;
}

void RpIrBaseHandler::postponeNoServiceMessageIfNeeded(int nDelayedTime) {
    if (mNoServiceTimerHandle != NULL) {
        RfxTimer::stop(mNoServiceTimerHandle);
        mNoServiceTimerHandle = RfxTimer::start(RfxCallback0(this,
                &RpIrBaseHandler::onNoServiceTimeout), ms2ns(nDelayedTime));
    }
}

void RpIrBaseHandler::onNoServiceTimeout() {
    // set timer to NULL
    mNoServiceTimerHandle = NULL;

    if (getState() == STATE_NO_SERVICE && mServiceType != IN_SERVICE) {
        if (mPtrListener) {
            mPtrListener->onServiceStateChanged(mServiceType);
        }
    }
}

void RpIrBaseHandler::processPlmnChanged(String8 plmn) {
    if (mPtrListener && (!mPreviewPlmnHandled || mPtrIrController->getVolteState() == VOLTE_ON
            || RpIrUtils::isViceOfTwoCtDualVolte(getSlotId()))) {
        if (mPtrListener->needWaitImsi()) {
            if (getStatusManager()->getString8Value(RFX_STATUS_KEY_C2K_IMSI).isEmpty() &&
                getSlotId() == getNonSlotScopeStatusManager()->getIntValue(RFX_STATUS_KEY_ACTIVE_CDMALTE_MODE_SLOT)) {
                logD(RFX_LOG_TAG, "Wait for C2K IMSI ready mPlmn = %s", plmn.string());
                mPlmn = plmn;
                getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_C2K_IMSI,
                    RfxStatusChangeCallback(this, &RpIrBaseHandler::onImsiChanged));
            } else {
                mPtrListener->onPlmnChanged(plmn);
            }
        } else {
            mPtrListener->onPlmnChanged(plmn);
        }
    }
}

void RpIrBaseHandler::previewPlmn(String8 plmn) {
    if (mPtrListener && (!mPreviewPlmnHandled || mPtrIrController->getVolteState() == VOLTE_ON
            || RpIrUtils::isViceOfTwoCtDualVolte(getSlotId()))) {
        if (mPtrListener->needWaitImsi()) {
            if (getStatusManager()->getString8Value(RFX_STATUS_KEY_C2K_IMSI).isEmpty() &&
                getSlotId() == getNonSlotScopeStatusManager()->getIntValue(RFX_STATUS_KEY_ACTIVE_CDMALTE_MODE_SLOT)) {
                logD(RFX_LOG_TAG, "Wait for C2K IMSI ready mPlmn = %s", plmn.string());
                mPlmn = plmn;
                getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_C2K_IMSI,
                    RfxStatusChangeCallback(this, &RpIrBaseHandler::onImsiChangedForPreviewPlmn));
            } else {
                mPreviewPlmnHandled = mPtrListener->onPreviewPlmn(plmn);
            }
        } else {
            mPreviewPlmnHandled = mPtrListener->onPreviewPlmn(plmn);
        }
    }
}

void RpIrBaseHandler::setPreviewPlmnHandleState(bool beHandled) {
    mPreviewPlmnHandled = beHandled;
}

void RpIrBaseHandler::onImsiChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    RFX_UNUSED(old_value);
    RFX_ASSERT(!value.asString8().isEmpty());
    getStatusManager()->unRegisterStatusChanged(RFX_STATUS_KEY_C2K_IMSI,
                    RfxStatusChangeCallback(this, &RpIrBaseHandler::onImsiChanged));
    if (mPtrListener && (!mPreviewPlmnHandled || mPtrIrController->getVolteState() == VOLTE_ON
            || RpIrUtils::isViceOfTwoCtDualVolte(getSlotId()))) {
        mPtrListener->onPlmnChanged(mPlmn);
    }
}

void RpIrBaseHandler::onImsiChangedForPreviewPlmn(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    RFX_UNUSED(old_value);
    RFX_ASSERT(!value.asString8().isEmpty());
    getStatusManager()->unRegisterStatusChanged(RFX_STATUS_KEY_C2K_IMSI,
                    RfxStatusChangeCallback(this, &RpIrBaseHandler::onImsiChangedForPreviewPlmn));
    if (mPtrListener && (!mPreviewPlmnHandled || mPtrIrController->getVolteState() == VOLTE_ON
            || RpIrUtils::isViceOfTwoCtDualVolte(getSlotId()))) {
        mPreviewPlmnHandled = mPtrListener->onPreviewPlmn(mPlmn);
    }
}

int RpIrBaseHandler::convertVoiceRegState(int nState) {
    int ret = nState;
    switch (nState) {
        case RIL_REG_STATE_NOT_REG_EMERGENCY_CALL_ENABLED:
            ret = RIL_REG_STATE_NOT_REG_NOT_SEARCHING;
            break;
        case RIL_REG_STATE_SEARCHING_EMERGENCY_CALL_ENABLED:
            ret = RIL_REG_STATE_NOT_REG_SEARCHING;
            break;
        case RIL_REG_STATE_DENIED_EMERGENCY_CALL_ENABLED:
            ret = RIL_REG_STATE_DENIED;
            break;
        case RIL_REG_STATE_UNKNOWN_EMERGENCY_CALL_ENABLED:
            ret = RIL_REG_STATE_UNKNOWN;
            break;
        default:
            break;
    }
    return ret;
}

