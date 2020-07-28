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
#include "RpIrStrategyCommon.h"
#include "RpIrNwRatSwitchCallback.h"

#define RFX_LOG_TAG "[IRC][RpIrNwRatSwitchCallback]"

/*****************************************************************************
 * Class RpIrNwRatSwitchCallback
 *****************************************************************************/
RFX_IMPLEMENT_CLASS("RpIrNwRatSwitchCallback", RpIrNwRatSwitchCallback, RfxController);

RpIrNwRatSwitchCallback::RpIrNwRatSwitchCallback() :
        mRpIrController(NULL),
        mRpIrCdmaHandler(NULL),
        mRpIrLwgHandler(NULL),
        mRpIrStrategy(NULL) {
}

RpIrNwRatSwitchCallback::RpIrNwRatSwitchCallback(
        RpIrController *rpIrController,
        RpIrCdmaHandler *rpIrCdmaHandler,
        RpIrLwgHandler *rpIrLwgHandler,
        RpIrStrategy *rpIrStrategy) :
        mRpIrController(rpIrController),
        mRpIrCdmaHandler(rpIrCdmaHandler),
        mRpIrLwgHandler(rpIrLwgHandler),
        mRpIrStrategy(rpIrStrategy) {
}

RpIrNwRatSwitchCallback::~RpIrNwRatSwitchCallback() {
}

void RpIrNwRatSwitchCallback::onInit() {
    RfxController::onInit();
}


void RpIrNwRatSwitchCallback::onRatSwitchStart(const int curPrefNwType,
        const int newPrefNwType, const NwsMode curNwsMode,
        const NwsMode newNwsMode, const VolteState curVolteState,
        const VolteState newVolteState) {
    logD(RFX_LOG_TAG, "onRatSwitchStart, curPrefNwType=%s, newPrefNwType=%s, curNwsMode=%s, newNwsMode=%s \
curVolteState=%s, newVolteState=%s",
            RpIrUtils::prefNwType2Str(curPrefNwType), RpIrUtils::prefNwType2Str(newPrefNwType),
            Nws2Str(curNwsMode), Nws2Str(newNwsMode),
            VolteState2Str(curVolteState), VolteState2Str(newVolteState));
    mRpIrController->setVolteState(newVolteState);
    mRpIrController->setVolteStateChanged(curVolteState == newVolteState ? false : true);
    if ((curVolteState == VOLTE_ON && newVolteState == VOLTE_OFF)
            || RpIrUtils::isViceOfTwoCtDualVolte(getSlotId())) {
        /*
         * Preview PLMN trigger NWS mode change when receive PLMN change info,
         * no need wait for reg suspend info.
         * mPreviewPlmnHandled is a flag to mark if preview plmn is handled.
         * It prevent redundant NWS mode jude flow.
         * mPreviewPlmnHandled is for 3G IR and will be reset when radio state changed
         * caused by NWS mode change (switch between MD1 and MD3 will trigger radio state change).
         * When VoLTE on, 3G IR logic is work and NWS mode switch between NWS_MODE_LTEONLY and
         * NWS_MODE_CSFB. However, ONLY MD1 work for both the 2 NWS mode
         * and mPreviewPlmnHandled can not be reset because no radio state change.
         * So, ignore mPreviewPlmnHandled when VoLTE on, reset it when VoLTE turn off.
         */
        mRpIrLwgHandler->setPreviewPlmnHandleState(false);
        mRpIrCdmaHandler->setPreviewPlmnHandleState(false);
    }

    if (!RpIrUtils::isSupportRoaming(getSlotId())) {
        logD(RFX_LOG_TAG, "onRatSwitchStart() not support roaming return");
        mRpIrController->set3GIrEnableState(false);
        return;
    }

    if (!RpIrUtils::is3GPrefNwType(newPrefNwType) && mRpIrController->needEnable3GIrForSpecialCase()) {
        // AP IR enable, MD IR disable. Disable AP and MD IR while switching
        mRpIrController->setIrControllerEnableState(false);
        mRpIrController->setGmssEnableState(false);
        mRpIrLwgHandler->setIfEnabled(true);
        if (RpIrUtils::isViceOfTwoCtDualVolte(getSlotId())) {
            mRpIrCdmaHandler->setIfEnabled(false);
        } else {
            mRpIrCdmaHandler->setIfEnabled(true);
        }
        mRpIrController->setIsSwitchingTo3GMode(true);
    } else {
        const bool curPrefNwTypeIs3G = RpIrUtils::is3GPrefNwType(curPrefNwType);
        const bool newPrefNwTypeIs3G = RpIrUtils::is3GPrefNwType(newPrefNwType);

        if ((!curPrefNwTypeIs3G && newPrefNwTypeIs3G) || (curPrefNwTypeIs3G && !newPrefNwTypeIs3G)) {
            logD(RFX_LOG_TAG, "onRatSwitchStart, disable IR Controller and Gmss handle while switching");
            // disable IR controll while switching between AP-iRAT IR and MD-iRAT IR
            if (!mRpIrCdmaHandler->isCT3GCardType()) {
                mRpIrController->setIrControllerEnableState(false);
            }
            mRpIrController->setGmssEnableState(false);
        }

        /*
         * disable IR if NwType switch to non-global mode,
         * skip enable cdma and lwg handler to avoid redundant action.
         */
        if (mRpIrController->isWwop() &&
                newPrefNwType != PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO &&
                newPrefNwType != PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA  &&
                newPrefNwType != PREF_NET_TYPE_TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA &&
                newPrefNwType != PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO) {
            logD(RFX_LOG_TAG, "onRatSwitchStart, disable IR in non-global mode");
            mRpIrController->setIrEnableState(false, newPrefNwTypeIs3G);
        } else {
            if (newPrefNwTypeIs3G) {
                mRpIrLwgHandler->setIfEnabled(true);
                mRpIrCdmaHandler->setIfEnabled(true);
            } else {
                mRpIrController->set3GIrEnableState(false);
            }
        }

        if (!curPrefNwTypeIs3G && newPrefNwTypeIs3G) {
            mRpIrController->setIsSwitchingTo3GMode(true);
        }
    }

    mRpIrController->onNwsModeChange(newNwsMode);

    mRpIrStrategy->onRatSwitchStart(newPrefNwType, newNwsMode,
            mRpIrController->getNwRatController()->getRatSwitchCaller());
}

void RpIrNwRatSwitchCallback::onRatSwitchDone(const int curPrefNwType,
            const int newPrefNwType) {
    logD(RFX_LOG_TAG, "onRatSwitchDone, curPrefNwType=%s, newPrefNwType=%s",
            RpIrUtils::prefNwType2Str(curPrefNwType), RpIrUtils::prefNwType2Str(newPrefNwType));
    if (!RpIrUtils::isSupportRoaming(getSlotId())) {
        logD(RFX_LOG_TAG, "onRatSwitchDone()  not support roaming, return");
        return;
    }

    if (!RpIrUtils::is3GPrefNwType(newPrefNwType) && mRpIrController->needEnable3GIrForSpecialCase()) {
        mRpIrController->setIsSwitchingTo3GMode(false);
        mRpIrController->setIrControllerEnableState(true);
        mRpIrLwgHandler->setIfEnabled(true);
        if (RpIrUtils::isViceOfTwoCtDualVolte(getSlotId())) {
            mRpIrCdmaHandler->setIfEnabled(false);
        } else {
            mRpIrCdmaHandler->setIfEnabled(true);
        }
        mRpIrController->setGmssEnableState(false);
    } else {
        mRpIrController->setIsSwitchingTo3GMode(false);

        /*
         * disable IR if NwType switch to non-global mode,
         * skip enable cdma and lwg handler to avoid redundant action.
         */
        if (mRpIrController->isWwop() &&
                newPrefNwType != PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO &&
                newPrefNwType != PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA &&
                newPrefNwType != PREF_NET_TYPE_TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA &&
                newPrefNwType != PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO) {
            logD(RFX_LOG_TAG, "onRatSwitchDone, disable IR in non-global mode");
            mRpIrController->setIrEnableState(false, RpIrUtils::is3GPrefNwType(newPrefNwType));
        } else {
            if (RpIrUtils::is3GPrefNwType(newPrefNwType)) {
                mRpIrController->set3GIrEnableState(true);
            } else {
                mRpIrController->setGmssEnableState(true);
                mRpIrController->set3GIrEnableState(false);
            }
            if (mRpIrController->isWwop()) {
                mRpIrController->setIrEnableState(true, RpIrUtils::is3GPrefNwType(newPrefNwType));
                mRpIrStrategy->onWwopRatSwitchDone(curPrefNwType, newPrefNwType);
            }
        }
        // For RpIrStrategyCommon
        mRpIrStrategy->onRatSwitchDone(curPrefNwType, newPrefNwType);
    }
}

void RpIrNwRatSwitchCallback::onEctModeChangeDone(const int curPrefNwType,
            const int newPrefNwType) {
    logD(RFX_LOG_TAG, "onEctModeChangeDone, curPrefNwType=%s, newPrefNwType=%s",
            RpIrUtils::prefNwType2Str(curPrefNwType), RpIrUtils::prefNwType2Str(newPrefNwType));
    if (!RpIrUtils::isSupportRoaming(getSlotId())) {
        logD(RFX_LOG_TAG, "onEctModeChangeDone()  not support roaming, return");
        return;
    }

    if (!RpIrUtils::is3GPrefNwType(newPrefNwType) && mRpIrController->needEnable3GIrForSpecialCase()) {
        // Do nothing in current design.
    } else {
        if (RpIrUtils::is3GPrefNwType(newPrefNwType)) {
            mRpIrController->setIrControllerEnableState(true);
        }
    }
}

