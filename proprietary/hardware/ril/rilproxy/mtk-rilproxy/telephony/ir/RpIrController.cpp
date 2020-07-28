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
#include <stdio.h>
#include <stdlib.h>
#include <cutils/properties.h>
#include "RfxLog.h"
#include "RpIrCdmaHandler.h"
#include "RpIrController.h"
#include "RpIrLwgHandler.h"
#include "RpIrMccTable.h"
#include "RpIrStrategyCommon.h"
#include "RpIrNwRatSwitchCallback.h"
#include "util/RpFeatureOptionUtils.h"
#include <libmtkrilutils.h>

#define RFX_LOG_TAG "[IRC]"
/*****************************************************************************
 * Class RpIrController
 *****************************************************************************/

int RpIrController::sNoServiceDelayTime = NO_SERVICE_DELAY_TIME;

RFX_IMPLEMENT_CLASS("RpIrController", RpIrController, RfxController);

RpIrController::RpIrController():
    mPtrSwitchStrategy(NULL),
    mPtrLwgHandler(NULL),
    mPtrCdmaHandler(NULL),
    mIsEnabled(false),
    mIsGmssEnable(true),
    mIsSwitchingTo3GMode(false),
    mIsIrControllerState(false),
    mIsVolteStateChanged(false),
    mVolteState(VOLTE_OFF),
    mNwsMode(NWS_MODE_UNKNOWN),
    mNwRatSwitchCallback(NULL),
    mNwRatController(NULL),
    mLastValidCdmaCardType(0),
    mLastValidImsi("") {
    memset(&mGmssInfo, -1, sizeof(GmssInfo));
}

RpIrController::~RpIrController() {
}

void RpIrController::onInit() {
    RfxController::onInit();

    char strProp[PROPERTY_VALUE_MAX] = {0};
    property_get("persist.vendor.sys.ct.ir.switcher", strProp, "1");
    int nProp = atoi(strProp);
    if (nProp != 1) {
        logD(RFX_LOG_TAG, "onInit CT IR switcher is 0, return.");
        return;
    }

    memset(strProp, 0, sizeof(char) * PROPERTY_VALUE_MAX);
    property_get("persist.vendor.radio.ct.ir.engmode", strProp, "0");
    nProp = atoi(strProp);
    if (nProp != ENGINEER_MODE_AUTO) {
        logD(RFX_LOG_TAG, "onInit CT IR engmode is %d, return.", nProp);
        return;
    }

    const int urc_id_list[] = {
        RIL_UNSOL_GMSS_RAT_CHANGED,
    };
    if (RpFeatureOptionUtils::isC2kSupport()) {
        registerToHandleUrc(urc_id_list, 1);
    }
    RFX_OBJ_CREATE_EX(mPtrLwgHandler, RpIrLwgHandler, this, (this));
    RFX_OBJ_CREATE_EX(mPtrCdmaHandler, RpIrCdmaHandler, this, (this));

    // for debug to adjuest no service delay time
    char strTemp[20] = {0};
    sprintf(strTemp, "%d", NO_SERVICE_DELAY_TIME);
    memset(strProp, 0, sizeof(char)*PROPERTY_VALUE_MAX);
    property_get("persist.vendor.sys.ct.ir.nsd", strProp, strTemp);
    sNoServiceDelayTime = atoi(strProp);

    // get ir mode
    memset(strProp, 0, sizeof(char)*PROPERTY_VALUE_MAX);
    property_get("persist.vendor.sys.ct.ir.mode", strProp, "0");
    int nMode = atoi(strProp);

    logD(RFX_LOG_TAG, "onInit ir mode=%d", nMode);

    if (nMode == 0 || nMode == 4 || nMode == 5) {
        logD(RFX_LOG_TAG, "onInit StrategyCommon");
        RFX_OBJ_CREATE_EX(mPtrSwitchStrategy, RpIrStrategyCommon, this,
                    (this, (IRpIrNwController*)mPtrLwgHandler,
                    (IRpIrNwController*)mPtrCdmaHandler));
    } else {
        logD(RFX_LOG_TAG, "onInit no Strategy!!!");
        RFX_ASSERT(0);
    }

    mNwsMode = NWS_MODE_CDMALTE;

    RFX_OBJ_CREATE_EX(mNwRatSwitchCallback,
                      RpIrNwRatSwitchCallback,
                      this,
                      (this, mPtrCdmaHandler, mPtrLwgHandler, mPtrSwitchStrategy));

    mNwRatController = (RpNwRatController*)findController(RFX_OBJ_CLASS_INFO(RpNwRatController));
    mNwRatController->registerRatSwitchCallback(mNwRatSwitchCallback);
    // Register RFX_STATUS_KEY_CDMA_SOCKET_SLOT.
    getNonSlotScopeStatusManager()->registerStatusChanged(RFX_STATUS_KEY_CDMA_SOCKET_SLOT,
            RfxStatusChangeCallback(this, &RpIrController::onCdmaSocketSlotChanged));

    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_C2K_IMSI,
            RfxStatusChangeCallback(this, &RpIrController::onImsiChanged));
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_CDMA_CARD_TYPE,
            RfxStatusChangeCallback(this, &RpIrController::onCdmaCardTypeChanged));
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_SIM_STATE,
            RfxStatusChangeCallback(this, &RpIrController::onSimStateChanged));
}

void RpIrController::onSimStateChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    if (RpIrUtils::isCdma3GDualModeCard(m_slot_id)
            && mLastValidImsi.isEmpty() && mNwsMode == NWS_MODE_CSFB
            && old_value.asInt() == RFX_SIM_STATE_LOCKED && value.asInt() == RFX_SIM_STATE_READY) {
        // If switched to roaming when SIM locked, and MD1 reported PLMN, we don't have C2K IMSI
        // to calculate correct NWS mode, so cannot switch to home in time.
        // Force to switch back to home here.
        mPtrLwgHandler->setNwsMode(NWS_MODE_CDMALTE);
    }
}

void RpIrController::onImsiChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    RFX_UNUSED(old_value);
    if (!value.asString8().isEmpty()) {
        mLastValidImsi = value.asString8();
    }
}

void RpIrController::onCdmaCardTypeChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    RFX_UNUSED(old_value);
    logD(RFX_LOG_TAG, "RpIrController::onCdmaCardTypeChanged() type = %s", value.toString().string());
    if (value.asInt() > 0) {
        mLastValidCdmaCardType = value.asInt();
    }
}

bool RpIrController::onHandleUrc(const sp<RfxMessage>& urc) {
    int urcId = urc->getId();
    sp<RfxMessage> urcToRilj = urc;
    logD(RFX_LOG_TAG, "onHandleUrc(), %d", urcId);
    if (urc->getError() == RIL_E_SUCCESS) {
        switch (urcId) {
            case RIL_UNSOL_GMSS_RAT_CHANGED:
                processGmssRatChanged(urc);
                break;
            default:
                break;
        }
    } else {
        // urc contains error
    }
    return true;
}

void RpIrController::setNwsMode(NwsMode nwsMode, const sp<RfxAction>& action) {
    logD(RFX_LOG_TAG, "setNwsMode, nwsMode=%s, mIsEnabled =%s, mIsSwitchingTo3GMode=%s",
              Nws2Str(nwsMode), Bool2Str(mIsEnabled), Bool2Str(mIsSwitchingTo3GMode));
    if (!mIsEnabled && !mIsSwitchingTo3GMode) {
        logD(RFX_LOG_TAG, "setNwsMode, nwsMode=%s, in disabled mode", Nws2Str(nwsMode));
        if (action != NULL) {
            action->act();
        }
    } else {
        bool ret = true;  // wait RAT controller add return value to setNwsMode()
        mNwRatController->setNwsMode(nwsMode, action);
        if (ret) {
            logD(RFX_LOG_TAG, "setNwsMode, nwsMode=%s", Nws2Str(nwsMode));
            mNwsMode = nwsMode;
        } else {
            logD(RFX_LOG_TAG, "setNwsMode, nwsMode=%s, return false", Nws2Str(nwsMode));
        }
    }
}

NwsMode RpIrController::getNwsMode() {
    return mNwsMode;
}

bool RpIrController::isCtSimCard() {
    bool ret = false;
    int cdma_card_type = getStatusManager()->getIntValue(RFX_STATUS_KEY_CDMA_CARD_TYPE);
    if (cdma_card_type <= 0
            && NWS_MODE_CSFB == getNwsMode()
            && getStatusManager()->getBoolValue(RFX_STATUS_KEY_CT3G_DUALMODE_CARD)) {
        /*********************************
        * For CDMA 3G dual mode card, RFX_STATUS_KEY_C2K_IMSI and RFX_STATUS_KEY_CDMA_CARD_TYPE
        * will be cleared in CSFB mode, and keep no value until MD3 reports them.
        * However, IR will stay in CSFB mode but never power on MD3 when they contain no value.
        * This invited a dead lock.
        * For this case, IR will record the last valid C2K IMSI and CDMA card type by itself, and
        * use last valid values in CSFB mode.
        **********************************/
        logD(RFX_LOG_TAG, "isCtSimCard, use last valid value %d", mLastValidCdmaCardType);
        cdma_card_type = mLastValidCdmaCardType;
    }
    if (cdma_card_type == CT_4G_UICC_CARD ||
        cdma_card_type == CT_UIM_SIM_CARD ||
        cdma_card_type == CT_3G_UIM_CARD) {
        ret = true;
    }
    logD(RFX_LOG_TAG, "isCtSimCard, CdmaCardType=%d, ret = %s", cdma_card_type, Bool2Str(ret));
    return ret;
}

bool RpIrController::needEnable3GIrForSpecialCase() {
    bool ret = false;

    if (getVolteState() == VOLTE_ON && mNwRatController->isWwop() == false) {
        logD(RFX_LOG_TAG, "needEnable3GIrForSpecialCase, enable for CT VoLTE");
        ret = true;
    } else if (RpIrUtils::isViceOfTwoCtDualVolte(getSlotId())) {
        logD(RFX_LOG_TAG, "needEnable3GIrForSpecialCase, enable for dual VoLTE C+C vice card");
        ret = true;
    } else if (RpIrUtils::isViceSimSupportLte()) {
        /*********************************
        * For (SRLTE+L && CMCC/CU + CT && Data on CMCC/CU), CT reg 1X only.
        * Need C2K AP IR takeover, since GMSS is disabled.
        **********************************/
        int peerSlotId = getSlotId() == 0 ? 1 : 0;
        if (RpIrUtils::getIccAppFamily(peerSlotId) == 1
                && CT_4G_UICC_CARD == RpIrUtils::getCtCardType(getSlotId())
                && getSlotId() != RpIrUtils::getMainCapSlot()) {
            logD(RFX_LOG_TAG, "needEnable3GIrForSpecialCase, enable for SRLTE+L");
            ret = true;
        }
    } else {
        // do nothing
        logD(RFX_LOG_TAG, "needEnable3GIrForSpecialCase, result = false");
    }

    return ret;
}

bool RpIrController::setIrEnableState(bool isEnabled, bool is3GPrefNwType) {
    if (isEnabled) {
        setIrControllerState(true);
        if (needEnable3GIrForSpecialCase()) {
            set3GIrEnableState(true);
            setGmssEnableState(false);
        } else {
            if (is3GPrefNwType) {
                set3GIrEnableState(true);
                setGmssEnableState(false);
            } else {
                set3GIrEnableState(false);
                setGmssEnableState(true);
            }
        }
    } else {
        setIrControllerState(false);
        set3GIrEnableState(false);
        setGmssEnableState(false);
    }

    return true;
}

void RpIrController::setIrControllerEnableState(bool isEnabled) {
    logD(RFX_LOG_TAG, "setIrControllerEnableState, isEnabled =%s", Bool2Str(isEnabled));
    mIsEnabled = isEnabled;
    mPtrSwitchStrategy->setIfEnabled(isEnabled);
}

bool RpIrController::getIrControllerEnableState() {
    return mIsEnabled;
}

void RpIrController::set3GIrEnableState(bool isEnabled) {
    logD(RFX_LOG_TAG, "set3GIrEnableState, isEnabled =%s", Bool2Str(isEnabled));
    if (mIsEnabled == isEnabled) {
        logD(RFX_LOG_TAG, "set3GIrEnableState, mIsEnabled==isEnabled :%s", Bool2Str(isEnabled));
    } else {
        setIrControllerEnableState(isEnabled);
    }
    mPtrLwgHandler->setIfEnabled(isEnabled);
    mPtrCdmaHandler->setIfEnabled(isEnabled);
}

void RpIrController::dispose() {
    mPtrLwgHandler->dispose();
    mPtrCdmaHandler->dispose();
}

void RpIrController::onNwsModeChange(NwsMode curMode) {
    logD(RFX_LOG_TAG, "onNwsModeChange, preMode=%s, curMode=%s",
               Nws2Str(mNwsMode), Nws2Str(curMode));
    if (curMode != mNwsMode) {
        mNwsMode = curMode;
        mPtrLwgHandler->startNewSearchRound();
        mPtrCdmaHandler->startNewSearchRound();
    }
}

void RpIrController::onCdmaSocketSlotChanged(RfxStatusKeyEnum key,
        RfxVariant oldValue, RfxVariant newValue){
    RFX_UNUSED(key);
    int newCdmaSlot = newValue.asInt();
    int oldCdmaSlot = oldValue.asInt();
    logD(RFX_LOG_TAG, "onCdmaSocketSlotChanged, newCdmaSlot: %d, oldCdmaSlot: %d, slot= %d",
               newCdmaSlot, oldCdmaSlot, getSlotId());

    if (!mIsEnabled) {
        logD(RFX_LOG_TAG, "onCdmaSocketSlotChanged, 3G IR is disabled, no need to handle.");
        return;
    }
    // if 3G IR is enabled, check CDMA slot and decide if need disable 3G IR or not.
    if (getSlotId() != RpIrUtils::getCdmaSlotId()
            && !RpIrUtils::isViceOfTwoCtDualVolte(getSlotId())) {
        set3GIrEnableState(false);
    }
}

bool RpIrController::needWaitImsi() {
    if (isCtSimCard()) {
        // already know it's CT card, so don't need to
        // wait for IMSI ready to get mcc
        logD(RFX_LOG_TAG, "needWaitImsi: %s, isCtSimCard", Bool2Str(false));
        return false;
    }

    bool bNeedWaitImsi = RpIrUtils::isSupportRoaming(getSlotId());
    logD(RFX_LOG_TAG, "needWaitImsi: %s", Bool2Str(bNeedWaitImsi));

    String8 imsi = getStatusManager()->getString8Value(RFX_STATUS_KEY_C2K_IMSI);
    if (imsi == String8("") && NWS_MODE_CSFB == getNwsMode() && mLastValidImsi != String8("")
            && getStatusManager()->getBoolValue(RFX_STATUS_KEY_CT3G_DUALMODE_CARD)) {
        // If IMSI cleared in CSFB mode, should not wait or it will cause dead lock.
        // See also RpIrController::isCtSimCard(), RpIrStrategyCommon::getHomeMcc().
        bNeedWaitImsi = false;
    }

    logD(RFX_LOG_TAG, "needWaitImsi: %s", Bool2Str(bNeedWaitImsi));
    return bNeedWaitImsi;
}

bool RpIrController::isWwop() {
    bool isWwop = mNwRatController->isWwop();
    logD(RFX_LOG_TAG, "isWwop:%s", Bool2Str(isWwop));
    return isWwop;
}

void RpIrController::resetSuspendNetwork() {
    // Should send AT command for all SIMs and wait for response
    if (RpIrUtils::isCdmaLteDcSupport()) {
        if (mPtrLwgHandler != NULL) {
            mPtrLwgHandler->resetSuspendNetwork();
        } else {
            logD(RFX_LOG_TAG, "No LWG handler, reset directly.");
        }
    }
}

/*****************************************************************************
 * Handle URC for GMSS RAT changed start
 *****************************************************************************/
#define RFX_LOG_TAG_GMSS "IRC_GMSS"

void RpIrController::setGmssEnableState(bool isEnabled) {
    logD(RFX_LOG_TAG_GMSS, "setGmssEnableState, isEnabled =%s", Bool2Str(isEnabled));
    mIsGmssEnable = isEnabled;
}

bool RpIrController::getGmssEnableState() {
    return mIsGmssEnable;
}

void RpIrController::processGmssRatChanged(const sp<RfxMessage>& msg) {
    if (!mIsGmssEnable) {
        logD(RFX_LOG_TAG_GMSS, "processGmssRatChanged, Gmss not Enabled");
        responseToRilj(msg);
        return;
    }

    Parcel *p = msg->getParcel();
    mGmssInfo.count = p->readInt32();
    mGmssInfo.rat = p->readInt32();
    mGmssInfo.mcc = p->readInt32();
    mGmssInfo.status = p->readInt32();
    mGmssInfo.mspl = p->readInt32();
    mGmssInfo.isHome = p->readInt32();
    mGmssInfo.isSrlte = p->readInt32();
    logD(RFX_LOG_TAG_GMSS, "processGmssRatChanged, count=%d rat=%d mcc=%d status=%d mspl=%d home=%d srlte=%d",
            mGmssInfo.count, mGmssInfo.rat, mGmssInfo.mcc, mGmssInfo.status, mGmssInfo.mspl,
            mGmssInfo.isHome, mGmssInfo.isSrlte);

    /* Don't care IMSI if isHome is valid,
     * to avoid IR stop if FW does not get IMSI from SIM.
     */
    if (mGmssInfo.isHome != INVALID_VALUE) {
        /* Only update phone object if device is not in service or GMSS is not in select status */
        if (isWwop() == false || mGmssInfo.status != GMSS_STATUS_SELECT) {
            onGmssRatChanged(mGmssInfo.mcc);
        }
    } else if (needWaitImsi()) {
        if (getStatusManager()->getString8Value(RFX_STATUS_KEY_C2K_IMSI).isEmpty() &&
                getSlotId() == getNonSlotScopeStatusManager()->getIntValue(RFX_STATUS_KEY_ACTIVE_CDMALTE_MODE_SLOT)) {
            logD(RFX_LOG_TAG_GMSS, "processGmssRatChanged, Wait for C2K IMSI ready MCC = %d", mGmssInfo.mcc);
            getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_C2K_IMSI,
                    RfxStatusChangeCallback(this, &RpIrController::onImsiChangedForGmss));
        } else {
            onGmssRatChanged(mGmssInfo.mcc);
        }
    } else {
        onGmssRatChanged(mGmssInfo.mcc);
    }

    responseToRilj(msg);
}

void RpIrController::onImsiChangedForGmss(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    RFX_UNUSED(old_value);
    RFX_ASSERT(!value.asString8().isEmpty());
    getStatusManager()->unRegisterStatusChanged(RFX_STATUS_KEY_C2K_IMSI,
            RfxStatusChangeCallback(this, &RpIrController::onImsiChangedForGmss));
    onGmssRatChanged(mGmssInfo.mcc);
}

void RpIrController::onGmssRatChanged(int mcc) {
    if (!mIsGmssEnable) {
        logD(RFX_LOG_TAG_GMSS, "onGmssRatChanged, Gmss not Enabled");
        return;
    }
    NwsMode targetMode = NWS_MODE_UNKNOWN;

    if (mGmssInfo.status == GMSS_STATUS_ECC) {  // for ECC retry
        targetMode = getNwsModeByEccRetry();
    } else {
        targetMode = getNwsModeByMcc(mcc);
    }
    logD(RFX_LOG_TAG_GMSS, "setNwsMode, targetMode=%s", RpIrUtils::nwsModeToString(targetMode));
    mNwRatController->setNwsMode(targetMode, NULL);
    onNwsModeChange(targetMode);
}

NwsMode RpIrController::getNwsModeByMcc(int mcc) {
    NwsMode nwsMode = NWS_MODE_CSFB;

    int homeMcc = mPtrSwitchStrategy->getHomeMcc();
    logD(RFX_LOG_TAG_GMSS, "getNwsModeByMcc, homeMcc = %d, mcc=%d, is_sim_ready = %s",
                      homeMcc, mcc, Bool2Str(RpIrUtils::isSimReady(getSlotId())));
    if (homeMcc == 0 && mGmssInfo.isHome == INVALID_VALUE) {
        // if bootup in CSFB mode, can't get C2K IMSI, so keep CSFB mode,
        // otherwise, it's in CDMALTE mode, should be able to get C2K IMSI
        logD(RFX_LOG_TAG_GMSS, "getNwsModeByPlmn, mcc=[%d]  ret: [NWS_MODE_CSFB]", mcc);
        return NWS_MODE_CSFB;
    }
    logD(RFX_LOG_TAG_GMSS, "getNwsModeByMcc, mcc=%d", mcc);

    if (nwsMode != NWS_MODE_CDMALTE) {
        if ((mcc == CHINA_TELECOM_MAINLAND_MCC || mcc == CHINA_TELECOM_MACAU_MCC) &&
                homeMcc == CHINA_TELECOM_MAINLAND_MCC) {
            if (getVolteState() == VOLTE_ON) {
                logD(RFX_LOG_TAG_GMSS, "getNwsModeByMcc, mcc=[%d], Volte on, ret: [NWS_MODE_LTEONLY]", mcc);
                return NWS_MODE_LTEONLY;
            } else {
                logD(RFX_LOG_TAG_GMSS, "getNwsModeByMcc, mcc=[%d], Volte off, ret: [NWS_MODE_CDMALTE]", mcc);
                return NWS_MODE_CDMALTE;
            }
        }
    }
    logD(RFX_LOG_TAG_GMSS, "gmssinfo.isHome=%d gmssinfo.isSrlte=%d", mGmssInfo.isHome, mGmssInfo.isSrlte);
    // Need to handle different EGMSS format versions
    if (mGmssInfo.isSrlte != INVALID_VALUE) {
        // +EGMSS:<rat>,<mcc>,<status>,<mspl>,<home>,<is_srlte>
        if (mGmssInfo.isSrlte == 1 ||
                GMSS_RAT_3GPP2 == mGmssInfo.rat ||
                GMSS_RAT_C2K1X == mGmssInfo.rat ||
                GMSS_RAT_C2KHRPD == mGmssInfo.rat) {
            nwsMode = NWS_MODE_CDMALTE;
        } else {
            nwsMode = NWS_MODE_CSFB;
        }
    } else if (mGmssInfo.isHome != INVALID_VALUE) {
        // Backward compatible for +EGMSS:<rat>,<mcc>,<status>,<mspl>,<home>
        nwsMode = mGmssInfo.isHome ? NWS_MODE_CDMALTE : NWS_MODE_CSFB;
        if (isWwop()) {
            if (GMSS_RAT_3GPP2 == mGmssInfo.rat ||
                GMSS_RAT_C2K1X == mGmssInfo.rat ||
                GMSS_RAT_C2KHRPD == mGmssInfo.rat) {
                nwsMode = NWS_MODE_CDMALTE;
            } else if (GMSS_RAT_GSM == mGmssInfo.rat ||
                       GMSS_RAT_WCDMA == mGmssInfo.rat) {
                nwsMode = NWS_MODE_CSFB;
            } else {
                nwsMode = (mGmssInfo.isHome == true) ? NWS_MODE_CDMALTE : NWS_MODE_CSFB;
            }
        }
    } else {
        // Backward compatible for +EGMSS:<rat>,<mcc>,<status>
        bool isHome = RpIrMccTable::isSameCountryCode(homeMcc, mcc);
        logD(RFX_LOG_TAG_GMSS, "isHome=%s", Bool2Str(isHome));
        nwsMode = isHome ? NWS_MODE_CDMALTE : NWS_MODE_CSFB;
    }

    logD(RFX_LOG_TAG_GMSS, "getNwsModeByMcc, mcc=%d, nwsMode=%s", mcc, Nws2Str(nwsMode));
    return nwsMode;
}

NwsMode RpIrController::getNwsModeByEccRetry() {
    NwsMode nwsMode = NWS_MODE_CSFB;

    // Need to handle different EGMSS format versions
    if (mGmssInfo.isSrlte != INVALID_VALUE) {
        // +EGMSS:<rat>,<mcc>,<status>,<mspl>,<home>,<is_srlte>
        if (GMSS_RAT_3GPP2 == mGmssInfo.rat ||
                GMSS_RAT_C2K1X == mGmssInfo.rat ||
                GMSS_RAT_C2KHRPD == mGmssInfo.rat) {
            nwsMode = NWS_MODE_CDMALTE;
        } else {
            nwsMode = NWS_MODE_CSFB;
        }
    } else {
        // Backward compatible for +EGMSS:<rat>,<mcc>,<status>,<mspl>,<home>
        if (GMSS_RAT_3GPP2 == mGmssInfo.rat ||
            GMSS_RAT_C2K1X == mGmssInfo.rat ||
            GMSS_RAT_C2KHRPD == mGmssInfo.rat) {
            nwsMode = NWS_MODE_CDMALTE;
        } else if (GMSS_RAT_GSM == mGmssInfo.rat ||
                 GMSS_RAT_WCDMA == mGmssInfo.rat) {
            nwsMode = NWS_MODE_CSFB;
        } else {
            if (mGmssInfo.isHome == false ||
                RpIrUtils::isCdmaDualModeSimCard(getSlotId()) == false) {
                nwsMode = NWS_MODE_CSFB;
            } else {
                nwsMode = NWS_MODE_CDMALTE;
            }
        }
    }

    logD(RFX_LOG_TAG_GMSS, "getNwsModeByEccRetry, rat=%d, mcc=%d, mspl=%d isHome=%d isSrlte=%d nwsMode=%s",
            mGmssInfo.rat, mGmssInfo.mcc, mGmssInfo.mspl, mGmssInfo.isHome,
            mGmssInfo.isSrlte, Nws2Str(nwsMode));
    return nwsMode;
}

/*****************************************************************************
 * Handle URC for GMSS RAT changed end
 *****************************************************************************/

