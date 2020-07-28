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
#include "RpCdmaLteNwRatSwitchHandler.h"
#include <telephony/mtk_ril.h>
#include "modecontroller/RpCdmaLteDefs.h"

/*****************************************************************************
 * Class RpCdmaLteNwRatSwitchHandler
 *****************************************************************************/

RFX_IMPLEMENT_CLASS("RpCdmaLteNwRatSwitchHandler", RpCdmaLteNwRatSwitchHandler,
        RpCdmaNwRatSwitchHandler);

RpCdmaLteNwRatSwitchHandler::RpCdmaLteNwRatSwitchHandler() :
        RpCdmaNwRatSwitchHandler(),
        mPrefNwType(-1),
        mNwsMode(NWS_MODE_CDMALTE),
        mVolteState(VOLTE_OFF),
        mRatSwitchCaller(RAT_SWITCH_INIT),
        mSugggestedOnRadioCapablity(RIL_CAPABILITY_NONE),
        mAction(NULL),
        mMessage(NULL),
        mIsCdma3gDualModeCard(false) {
}

RpCdmaLteNwRatSwitchHandler::RpCdmaLteNwRatSwitchHandler(
        RpNwRatController* nwRatController) :
        RpCdmaNwRatSwitchHandler(nwRatController),
        mPrefNwType(-1),
        mNwsMode(NWS_MODE_CDMALTE),
        mVolteState(VOLTE_OFF),
        mRatSwitchCaller(RAT_SWITCH_INIT),
        mSugggestedOnRadioCapablity(RIL_CAPABILITY_NONE),
        mAction(NULL),
        mMessage(NULL),
        mIsCdma3gDualModeCard(false){
}

RpCdmaLteNwRatSwitchHandler::~RpCdmaLteNwRatSwitchHandler() {
}

void RpCdmaLteNwRatSwitchHandler::onInit() {
    RfxObject::onInit();  // Required: invoke super class implementation

    char prop_tdd_only[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ro.vendor.mtk_tdd_data_only_support", prop_tdd_only, "0");
    int  activeModem = 0;
    char active_modem[PROPERTY_VALUE_MAX] = {0};
    property_get("vendor.ril.active.md", active_modem, "0");
    activeModem = atoi(active_modem);
    char tempstr[PROPERTY_VALUE_MAX];
    memset(tempstr, 0, sizeof(tempstr));
    property_get("persist.vendor.radio.simswitch", tempstr, "1");
    int capabilitySlotId = atoi(tempstr) - 1;
    bool isMajorSlot = false;
    if (capabilitySlotId == mSlotId) {
        isMajorSlot = true;
    }
    if (activeModem != 4 && activeModem != 6) {
        if (atoi(prop_tdd_only) == 1) {
            char prop[PROPERTY_VALUE_MAX] = { 0 };
            property_get("persist.vendor.operator.optr", prop, "");
            if (isMajorSlot && (strcmp(prop, "OP09") == 0)) {
                int msg_id = RIL_LOCAL_REQUEST_SET_FDD_TDD_MODE;
                sp<RfxMessage> gsmMessage = RfxMessage::obtainRequest(mSlotId,
                        RADIO_TECH_GROUP_GSM, msg_id);
                gsmMessage->getParcel()->writeInt32(1);
                gsmMessage->getParcel()->writeInt32(SVLTE_FDD_ONLY_MODE);
                mNwRatController->requestToRild(gsmMessage);
            }
        }
    }
}
void RpCdmaLteNwRatSwitchHandler::requestGetPreferredNetworkType(const sp<RfxMessage>& message) {
    if (mNwsMode == NWS_MODE_CSFB || mNwsMode == NWS_MODE_LTEONLY) {
        sp<RfxMessage> resToRilj = RfxMessage::obtainResponse(RIL_E_SUCCESS, message);
        resToRilj->getParcel()->writeInt32(1);
        resToRilj->getParcel()->writeInt32(mPrefNwType);
        mNwRatController->responseToRilj(resToRilj);
    } else {
        RpBaseNwRatSwitchHandler::requestGetPreferredNetworkType(
                RADIO_TECH_GROUP_C2K, message);
    }
}

void RpCdmaLteNwRatSwitchHandler::responseGetPreferredNetworkType(const sp<RfxMessage>& response) {
    int error = response->getError();
    if (error != RIL_E_SUCCESS) {
        // error
        logD(CDMA_LTE_RAT_CTRL_TAG, "responseGetPreferredNetworkType(), error is %d.", error);
        mNwRatController->responseToRilj(response);
    } else {
        // ok
        int source = response->getSource();
        int32_t stgCount;
        int32_t nwType = -1;
        response->getParcel()->readInt32(&stgCount);
        response->getParcel()->readInt32(&nwType);
        if (source == RADIO_TECH_GROUP_C2K) {
            sp<RfxMessage> resToRilj = RfxMessage::obtainResponse(RIL_E_SUCCESS, response);
            resToRilj->getParcel()->writeInt32(1);
            resToRilj->getParcel()->writeInt32(nwType);
            mNwRatController->responseToRilj(resToRilj);
            logD(CDMA_LTE_RAT_CTRL_TAG, "responseGetPreferredNetworkType(), send response %d.", nwType);
        }
    }
}

void RpCdmaLteNwRatSwitchHandler::responseGetPreferredNetworkType(
        const int nwType, const sp<RfxMessage>& message) {
    sp<RfxMessage> resToRilj = RfxMessage::obtainResponse(RIL_E_SUCCESS, message);
    resToRilj->getParcel()->writeInt32(1);
    resToRilj->getParcel()->writeInt32(nwType);
    logD(CDMA_LTE_RAT_CTRL_TAG, "responseGetPreferredNetworkType(), send response %d "
            "via mPreferredNetWorkTypeFromRILJ.", nwType);
    mNwRatController->responseToRilj(resToRilj);
}

void RpCdmaLteNwRatSwitchHandler::requestSetPreferredNetworkType(const int prefNwType, const sp<RfxMessage>& message) {
    RFX_UNUSED(message);
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("persist.vendor.radio.ct.ir.engmode", property_value, "0");
    int engineerMode = atoi(property_value);
    char tempstr[PROPERTY_VALUE_MAX] = { 0 };
    property_get("persist.vendor.radio.simswitch", tempstr, "1");
    int  capabilitySlotId  = atoi(tempstr) - 1;
    logD(CDMA_LTE_RAT_CTRL_TAG, "requestSetPreferredNetworkType(), engineerMode=%d, capabilitySlotId=%d, prefNwType=%d.",
            engineerMode, capabilitySlotId, prefNwType);
    if ((!mNwRatController->isEratExtSupport()) && engineerMode == ENGINEER_MODE_CDMA
            && (prefNwType == PREF_NET_TYPE_CDMA_ONLY
                    || prefNwType == PREF_NET_TYPE_EVDO_ONLY
                    || prefNwType == PREF_NET_TYPE_CDMA_EVDO_AUTO)
            && capabilitySlotId == mSlotId) {
        int msg_id = RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE;
        sp<RfxMessage> message = RfxMessage::obtainRequest(mSlotId,
                RADIO_TECH_GROUP_C2K, msg_id);
        message->getParcel()->writeInt32(1);
        message->getParcel()->writeInt32(prefNwType);
        logD(CDMA_LTE_RAT_CTRL_TAG, "requestSetPreferredNetworkType(), set nw type to %d.",
                prefNwType);
        mNwRatController->requestToRild(message);
    }
}
void RpCdmaLteNwRatSwitchHandler::responseSetPreferredNetworkType(const sp<RfxMessage>& response) {
    sp<RfxMessage> resToRilj = RfxMessage::obtainResponse(RIL_E_SUCCESS,
            response);
    mNwRatController->responseToRilj(resToRilj);
}

ModemSettings RpCdmaLteNwRatSwitchHandler::calculateModemSettings(int prefNwType,
        AppFamilyType appFamType, NwsMode nwsMode, VolteState volteState) {
    ModemSettings mdSettings;
    mdSettings.prefNwType = prefNwType;
    mdSettings.appFamType = appFamType;
    mdSettings.nwsMode = nwsMode;
    mdSettings.volteState = volteState;
    return mdSettings;
}

void RpCdmaLteNwRatSwitchHandler::doNwRatSwitch(ModemSettings mdSettings,
        const RatSwitchCaller& ratSwitchCaller, const sp<RfxAction>& action,
        const sp<RfxMessage>& message) {
    int suggestRadioCapability = mNwRatController->getStatusManager()->getIntValue(
                    RFX_STATUS_KEY_SUGGESTED_RADIO_CAPABILITY, RIL_CAPABILITY_NONE);
    logD(CDMA_LTE_RAT_CTRL_TAG, "doNwRatSwitch(), prefNwType to %d, NwsMode is %d, volteState is %d,"
            "ratSwitchCaller is %d, suggestRadioCapability is %d .", mdSettings.prefNwType,
            mdSettings.nwsMode, mdSettings.volteState, ratSwitchCaller, suggestRadioCapability);

    mPrefNwType = mdSettings.prefNwType;
    mNwsMode = mdSettings.nwsMode;
    mVolteState = mdSettings.volteState;

    mRatSwitchCaller = ratSwitchCaller;
    mAction = action;
    mMessage = message;
    mIsCdma3gDualModeCard = mNwRatController->getStatusManager()->getBoolValue(
            RFX_STATUS_KEY_CT3G_DUALMODE_CARD, false);

    if (mPrefNwType == mNwRatController->getPreferredNetworkType()
            && mNwsMode == mNwRatController->getNwsMode()
            && mVolteState == mNwRatController->getVolteState()
            && mRatSwitchCaller == RAT_SWITCH_NORMAL
            /* If the current suggest radio capability not conform our desired
             * radio capability, need do the rat switch request.
             * For example in SIM missing and recovery mechanism, the suggest
             * radio capability be cleared when card type become 0. */
            && suggestRadioCapability != RIL_CAPABILITY_NONE) {

        // 4, toggle phone object
        if (mNwRatController->mPendingRestrictedRatSwitchRecord.prefNwType == -1) {
            logD(CDMA_LTE_RAT_CTRL_TAG, "Step4: Only Toggle active phone");
            updatePhone(mPrefNwType, mNwsMode);
        }

        onNwRatSwitchDone(RAT_SWITCH_SUCC);
    } else {
        mNwRatController->onRatSwitchStart(mPrefNwType, mNwsMode, mVolteState);
        // 1, reset radio power off if will close finally
        // 2, config evdo mode if need
        // 3, switch network mode
        // 4, toggle phone object
        // 5, set radio power
        // 6, handle the radio state changed result

        // 1, reset radio power off if will close finally
        resetRadioPowerOff();
    }
}

void RpCdmaLteNwRatSwitchHandler::resetRadioPowerOff() {
    bool cdmaRadioOn = needCdmaRadioOn(mPrefNwType, mNwsMode);
    bool gsmRadioOn = needGsmRadioOn(mPrefNwType);
    int expectedRadioCapability = RIL_CAPABILITY_NONE;
    if (cdmaRadioOn && gsmRadioOn) {
        expectedRadioCapability = RIL_CAPABILITY_CDMA_ON_LTE;
    } else if (cdmaRadioOn && !gsmRadioOn) {
        expectedRadioCapability = RIL_CAPABILITY_CDMA_ONLY;
    } else if (!cdmaRadioOn && gsmRadioOn) {
        expectedRadioCapability = RIL_CAPABILITY_GSM_ONLY;
    } else {
        expectedRadioCapability = RIL_CAPABILITY_NONE;
    }
    int suggestedRadioCapability = mNwRatController->getStatusManager()
                -> getIntValue(RFX_STATUS_KEY_SUGGESTED_RADIO_CAPABILITY, RIL_CAPABILITY_NONE);

    int targetRadioCapability = RIL_CAPABILITY_NONE;
    bool needResetRadioOff = false;
    switch (suggestedRadioCapability) {
        case RIL_CAPABILITY_NONE:
            needResetRadioOff = false;
            break;
        case RIL_CAPABILITY_GSM_ONLY:
            if (expectedRadioCapability == RIL_CAPABILITY_GSM_ONLY
                    || expectedRadioCapability == RIL_CAPABILITY_CDMA_ON_LTE) {
                needResetRadioOff = false;
            } else {
                needResetRadioOff = true;
                targetRadioCapability = RIL_CAPABILITY_NONE;
            }
            break;
        case RIL_CAPABILITY_CDMA_ONLY:
            if (expectedRadioCapability == RIL_CAPABILITY_CDMA_ONLY
                    || expectedRadioCapability == RIL_CAPABILITY_CDMA_ON_LTE) {
                needResetRadioOff = false;
            } else {
                needResetRadioOff = true;
                targetRadioCapability = RIL_CAPABILITY_NONE;
            }
            break;
        case RIL_CAPABILITY_CDMA_ON_LTE:
            if (expectedRadioCapability == RIL_CAPABILITY_GSM_ONLY) {
                needResetRadioOff = true;
                targetRadioCapability = RIL_CAPABILITY_GSM_ONLY;
            } else if (expectedRadioCapability == RIL_CAPABILITY_CDMA_ONLY) {
                needResetRadioOff = true;
                targetRadioCapability = RIL_CAPABILITY_CDMA_ONLY;
            } else if (expectedRadioCapability == RIL_CAPABILITY_NONE) {
                needResetRadioOff = true;
                targetRadioCapability = RIL_CAPABILITY_NONE;
            } else {
                needResetRadioOff = false;
            }
            break;
        }
    logD(CDMA_LTE_RAT_CTRL_TAG, "Step1: resetRadioPowerOff(), expectedRadioCapability is %d, "
            "suggestedRadioCapability is %d, targetRadioCapability is %d, needResetRadioOff is %s",
            expectedRadioCapability, suggestedRadioCapability, targetRadioCapability,
            needResetRadioOff ? "true" : "false");
    if (needResetRadioOff) {
        RpRadioController* radioController =
                (RpRadioController *) mNwRatController->findController(
                        RFX_OBJ_CLASS_INFO(RpRadioController));
        RpSuggestRadioCapabilityCallback callback =
                RpSuggestRadioCapabilityCallback(this,
                        &RpCdmaLteNwRatSwitchHandler::onResetRadioPowerOffChanged);
        radioController->suggestedCapability(targetRadioCapability, callback);
    } else {
        requestSetEvdoMode(mPrefNwType);
    }
}

void RpCdmaLteNwRatSwitchHandler::onResetRadioPowerOffChanged(SuggestRadioResult result) {
    logD(CDMA_LTE_RAT_CTRL_TAG, "onResetRadioPowerOffChanged(), radio off result is %d .", result);
    requestSetEvdoMode(mPrefNwType);
}

void RpCdmaLteNwRatSwitchHandler::requestSetEvdoMode(const int prefNwType ) {
    EvdoMode evdoMode = EVDO_MODE_INVALID;
    // M: Volte off && !(CT is slave card && roaming)
    if (mVolteState != VOLTE_ON
        && !(mNwRatController->getCapabilitySlotId() != mSlotId && mNwsMode == NWS_MODE_CSFB)) {
        switch (prefNwType) {
        /* Disable EHRPD */
        case PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO:
        case PREF_NET_TYPE_EVDO_ONLY:
        case PREF_NET_TYPE_CDMA_ONLY:
        case PREF_NET_TYPE_CDMA_EVDO_AUTO:
            evdoMode = EVDO_MODE_NORMAL;
            break;

        /* Enable EHRPD */
        case PREF_NET_TYPE_LTE_CDMA_EVDO:
        case PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA:
        /* Roaming and 4G mode should notify c2k modem*/
        case PREF_NET_TYPE_LTE_WCDMA:
        case PREF_NET_TYPE_LTE_GSM_WCDMA:
        case PREF_NET_TYPE_LTE_GSM:
        case PREF_NET_TYPE_LTE_ONLY:
            evdoMode = EVDO_MODE_EHRPD;
            break;

        default:
            break;
        }
    }

    if (evdoMode != EVDO_MODE_INVALID) {
        int msg_id = RIL_LOCAL_C2K_REQUEST_CONFIG_EVDO_MODE;
        sp<RfxMessage> message = RfxMessage::obtainRequest(mSlotId,
                RADIO_TECH_GROUP_C2K, msg_id);
        message->getParcel()->writeInt32(1);
        message->getParcel()->writeInt32(evdoMode);
        logD(CDMA_LTE_RAT_CTRL_TAG, "Step2: requestSetEvdoMode(), set evdo mode to %d.", evdoMode);
        mNwRatController->requestToRild(message);
    } else {
        // no need to config EHRPD
        logD(CDMA_LTE_RAT_CTRL_TAG, "Step2: no need to set evdo mode, prepare to set rat mode ");
        requestSetRatMode();
    }
}

void RpCdmaLteNwRatSwitchHandler::responseSetEvdoMode(const sp<RfxMessage>& response) {
    int error = response->getError() ;
    if (error != RIL_E_SUCCESS) {
        // error
        onNwRatSwitchDone(RAT_SWITCH_FAIL);
    } else {
        // ok
        int source = response->getSource();
        if (source == RADIO_TECH_GROUP_C2K) {
            requestSetRatMode();
        }
    }
}

void RpCdmaLteNwRatSwitchHandler::requestSetRatMode() {
    int radioMode = (int) setRadioTechModeFromPrefType();
    int newRatMode = (int) setRatModeFromPrefType(mPrefNwType);
    mNwsModeForSwitchCardType = mNwsMode;

    logD(CDMA_LTE_RAT_CTRL_TAG,
            "Step3: requestSetRatMode(), radioMode is %d, preferNwType is %d, newRatMode is %d, \
             preNwsMode is %d, newNwsMode is %d, is3gCdmaSim is %s",
            radioMode, mPrefNwType, newRatMode, (int)mNwRatController->getNwsMode(), (int)mNwsMode, mIsCdma3gDualModeCard ? "true" : "false");
    if (radioMode != RADIO_TECHNOLOGY_MODE_UNKNOWN
            && newRatMode != SVLTE_RAT_MODE_INVALID) {
        int msg_id = RIL_LOCAL_REQUEST_SET_SVLTE_RAT_MODE;
        sp<RfxMessage> gsmMessage = RfxMessage::obtainRequest(mSlotId,
                RADIO_TECH_GROUP_GSM, msg_id);
        gsmMessage->getParcel()->writeInt32(7);
        gsmMessage->getParcel()->writeInt32(radioMode);
        gsmMessage->getParcel()->writeInt32(mPrefNwType);
        gsmMessage->getParcel()->writeInt32(newRatMode);
        gsmMessage->getParcel()->writeInt32(
                (int) mNwRatController->getNwsMode());
        gsmMessage->getParcel()->writeInt32((int) mNwsMode);
        gsmMessage->getParcel()->writeInt32(mIsCdma3gDualModeCard ? 1 : 0);
        gsmMessage->getParcel()->writeInt32((int) mVolteState);
        mNwRatController->requestToRild(gsmMessage);
        if (mIsCdma3gDualModeCard) {
            msg_id = RIL_LOCAL_C2K_REQUEST_SET_SVLTE_RAT_MODE;
            sp<RfxMessage> cdmaMessage = RfxMessage::obtainRequest(
                    RADIO_TECH_GROUP_C2K, msg_id, gsmMessage);
            cdmaMessage->getParcel()->writeInt32(7);
            cdmaMessage->getParcel()->writeInt32(radioMode);
            cdmaMessage->getParcel()->writeInt32(mPrefNwType);
            cdmaMessage->getParcel()->writeInt32(newRatMode);
            cdmaMessage->getParcel()->writeInt32(
                    (int) mNwRatController->getNwsMode());
            cdmaMessage->getParcel()->writeInt32((int) mNwsMode);
            cdmaMessage->getParcel()->writeInt32(mIsCdma3gDualModeCard ? 1 : 0);
            cdmaMessage->getParcel()->writeInt32((int) mVolteState);
            mNwRatController->requestToRild(cdmaMessage);
        }
    } else {
        mNwRatController->updateState(mNwRatController->getNwsMode(),
                mNwRatController->getPreferredNetworkType(),
                mNwRatController->getVolteState());
    }
}

SVLTE_RAT_MODE RpCdmaLteNwRatSwitchHandler::setRatModeFromPrefType(const int prefNwType) {
    switch (prefNwType) {
    case PREF_NET_TYPE_LTE_GSM_WCDMA:
    case PREF_NET_TYPE_LTE_ONLY:
    case PREF_NET_TYPE_LTE_CDMA_EVDO:
    case PREF_NET_TYPE_LTE_WCDMA:
    case PREF_NET_TYPE_LTE_GSM:
    case PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA:
        if (prefNwType == PREF_NET_TYPE_LTE_ONLY && mVolteState == VOLTE_ON) {
            return SVLTE_RAT_MODE_LTE_ONLY;
        }
        return SVLTE_RAT_MODE_4G;

    case PREF_NET_TYPE_GSM_ONLY:
    case PREF_NET_TYPE_GSM_WCDMA:
    case PREF_NET_TYPE_GSM_WCDMA_AUTO:
    case PREF_NET_TYPE_WCDMA:
    case PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO:
    case PREF_NET_TYPE_CDMA_EVDO_AUTO:
    case PREF_NET_TYPE_EVDO_ONLY:
    case PREF_NET_TYPE_CDMA_ONLY:
        return SVLTE_RAT_MODE_3G;
    case PREF_NET_TYPE_LTE_TDD_ONLY:
        return SVLTE_RAT_MODE_4G_DATA_ONLY;
    default:
        return SVLTE_RAT_MODE_INVALID;
    }

}
RADIO_TECH_MODE RpCdmaLteNwRatSwitchHandler::setRadioTechModeFromPrefType() {
    switch (mPrefNwType) {
    case PREF_NET_TYPE_GSM_ONLY:
    case PREF_NET_TYPE_GSM_WCDMA:
    case PREF_NET_TYPE_GSM_WCDMA_AUTO:
    case PREF_NET_TYPE_WCDMA:
    case PREF_NET_TYPE_LTE_GSM_WCDMA:
    case PREF_NET_TYPE_LTE_ONLY:
    case PREF_NET_TYPE_LTE_TDD_ONLY:
    case PREF_NET_TYPE_LTE_WCDMA:
    case PREF_NET_TYPE_LTE_GSM:
        return RADIO_TECHNOLOGY_MODE_CSFB;

    case PREF_NET_TYPE_CDMA_ONLY:
    case PREF_NET_TYPE_CDMA_EVDO_AUTO:
    case PREF_NET_TYPE_EVDO_ONLY:
    case PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO:
    case PREF_NET_TYPE_LTE_CDMA_EVDO:
    case PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA:
        return RADIO_TECHNOLOGY_MODE_SVLTE;
    }
    return RADIO_TECHNOLOGY_MODE_UNKNOWN;
}
void RpCdmaLteNwRatSwitchHandler::responseSetRatMode(
        const sp<RfxMessage>& response) {
    mNwRatController->onEctModeChangeDone(mPrefNwType);
    int error = response->getError();
    if (error != RIL_E_SUCCESS) {
        // error
        logD(CDMA_LTE_RAT_CTRL_TAG, "responseSetSvlteMode(), error is %d.", error);
        onNwRatSwitchDone(RAT_SWITCH_FAIL, error);
    } else {
        requestSetPreferredNetworkType(mPrefNwType, NULL);
        // 4, toggle phone object
        if (mNwRatController->mPendingRestrictedRatSwitchRecord.prefNwType == -1) {
            logD(CDMA_LTE_RAT_CTRL_TAG, "Step4: Toggle active phone");
            updatePhone(mPrefNwType, mNwsMode);
        } else {
            logD(CDMA_LTE_RAT_CTRL_TAG, "Restricted mode, do not update phone!");
        }
        mNwRatController->onRatSwitchDone(mPrefNwType);
        // 5, set radio power
        mSugggestedOnRadioCapablity = radioPowerOn();
    }
}

int RpCdmaLteNwRatSwitchHandler::radioPowerOn() {
    int capability = RIL_CAPABILITY_NONE;
    bool cdmaRadioOn = needCdmaRadioOn(mPrefNwType, mNwsMode);
    bool gsmRadioOn = needGsmRadioOn(mPrefNwType);
    if (!cdmaRadioOn && gsmRadioOn) {
        capability = RIL_CAPABILITY_GSM_ONLY;
    } else if (!gsmRadioOn && cdmaRadioOn) {
        capability = RIL_CAPABILITY_CDMA_ONLY;
    } else if (gsmRadioOn && cdmaRadioOn) {
        capability = RIL_CAPABILITY_CDMA_ON_LTE;
    } else {
        capability = RIL_CAPABILITY_NONE;
    }

    logD(CDMA_LTE_RAT_CTRL_TAG, "radioPowerOn(), needCdmaRadioOn is %s, needGsmRadioOn is %s"
            "radioPowerOn(), capability is %d. ", cdmaRadioOn ? "true" : "false",
            gsmRadioOn ? "true" : "false", capability);
    RpRadioController* radioController =
            (RpRadioController *) mNwRatController->findController(
                    RFX_OBJ_CLASS_INFO(RpRadioController));
    RpSuggestRadioCapabilityCallback callback =
            RpSuggestRadioCapabilityCallback(this,
                    &RpCdmaLteNwRatSwitchHandler::onRadioPowerOn);
    radioController->suggestedCapability(capability, callback);
    return capability;
}

void RpCdmaLteNwRatSwitchHandler::onRadioPowerOn(SuggestRadioResult result) {
    // 6, handle the radio state changed result
    logD(CDMA_LTE_RAT_CTRL_TAG, "onRadioPowerOn(), power on result is %d .", result);
    onNwRatSwitchDone(RAT_SWITCH_SUCC);
}

void RpCdmaLteNwRatSwitchHandler::onNwRatSwitchDone(
        RatSwitchResult switchResult, int errorCode) {
    logD(CDMA_LTE_RAT_CTRL_TAG, "Step6: onNwRatSwitchDone, switchResult = %d, errorCode = %d.",
             switchResult, errorCode);

    if (switchResult == RAT_SWITCH_SUCC) {
        mNwRatController->updateState(mNwsMode, mPrefNwType, mVolteState);
        mNwRatController->getStatusManager()->setIntValue(
                RFX_STATUS_KEY_RAT_SWITCH_DONE, mRatSwitchCaller);
    } else {
        // Switch fail, update state to the previous state.
        mNwRatController->updateState(mNwRatController->getNwsMode(),
                mNwRatController->getPreferredNetworkType(),
                mNwRatController->getVolteState());
    }

    mNwRatController->handleRatSwitchResult(mNwsMode, mPrefNwType, mVolteState, mRatSwitchCaller,
            switchResult, errorCode);

    // Execute the callback function to notify switching done.
    if (mAction != NULL) {
        mAction->act();
        logD(CDMA_LTE_RAT_CTRL_TAG, "rat switch done, do the action. ");
    }

    if (mMessage != NULL) {
        responseSetPreferredNetworkType(mMessage);
        logD(CDMA_LTE_RAT_CTRL_TAG, "rat switch done, response set preferred nw type request.");
    }

    if (mNwRatController->isInCall() && mNwRatController->hasPendingVoLTERecord()) {
        // Don't doPendingRatSwitchRecord, after calling is end, trigger VoLTE rat switch again.
    } else {
        mNwRatController->doPendingRatSwitchRecord();
    }
}

bool RpCdmaLteNwRatSwitchHandler::needCdmaRadioOn(const int prefNwType, const NwsMode nwsMode) {
    logD(CDMA_LTE_RAT_CTRL_TAG, "needCdmaRadioOn, prefNwType=%d, mNwsMode = %d", prefNwType, mNwsMode);
    if (mVolteState == VOLTE_ON) {
        return false;
    }
    if (nwsMode == NWS_MODE_CDMALTE) {
        switch (prefNwType) {
            case PREF_NET_TYPE_CDMA_EVDO_AUTO:
            case PREF_NET_TYPE_CDMA_ONLY:
            case PREF_NET_TYPE_EVDO_ONLY:
            case PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO:
            case PREF_NET_TYPE_LTE_CDMA_EVDO:
            case PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA:
                return true;
            case PREF_NET_TYPE_GSM_WCDMA:
            case PREF_NET_TYPE_GSM_ONLY:
            case PREF_NET_TYPE_WCDMA:
            case PREF_NET_TYPE_GSM_WCDMA_AUTO:
            case PREF_NET_TYPE_LTE_GSM_WCDMA:
            case PREF_NET_TYPE_LTE_ONLY:
            case PREF_NET_TYPE_LTE_TDD_ONLY:
            case PREF_NET_TYPE_LTE_WCDMA:
            case PREF_NET_TYPE_LTE_GSM:
                return false;
        }
    } else if (nwsMode == NWS_MODE_CSFB) {
        char property_value[PROPERTY_VALUE_MAX] = { 0 };
        property_get("persist.vendor.radio.ct.ir.engmode", property_value, "0");
        int engineerMode = atoi(property_value);

        switch (prefNwType) {
            case PREF_NET_TYPE_CDMA_EVDO_AUTO:
            case PREF_NET_TYPE_CDMA_ONLY:
            case PREF_NET_TYPE_EVDO_ONLY:
            case PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO:
            case PREF_NET_TYPE_LTE_CDMA_EVDO:
            case PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA:
                return true;
            case PREF_NET_TYPE_LTE_WCDMA:
            case PREF_NET_TYPE_LTE_GSM_WCDMA:
            case PREF_NET_TYPE_LTE_GSM:
            case PREF_NET_TYPE_LTE_ONLY:
            if (engineerMode == ENGINEER_MODE_CSFB
                 || mNwRatController->getCapabilitySlotId() != mSlotId) {
                /* engineer set in CSFB mode c2k radio should off or CT is in slave mode and roaming */
                return false;
            } else {
                /* Roaming and 4G mode c2k radio should on*/
                return true;
            }
            case PREF_NET_TYPE_GSM_WCDMA:
            case PREF_NET_TYPE_GSM_ONLY:
            case PREF_NET_TYPE_WCDMA:
            case PREF_NET_TYPE_GSM_WCDMA_AUTO:
            case PREF_NET_TYPE_LTE_TDD_ONLY:
                return false;
        }
    }
    return false;
}

bool RpCdmaLteNwRatSwitchHandler::isCdma3gDualModeCard() {
    return mIsCdma3gDualModeCard;
}

void RpCdmaLteNwRatSwitchHandler::onResponseTimeOut(const sp<RfxMessage>& response) {
    int reqId = response->getId();
    logD(CDMA_LTE_RAT_CTRL_TAG, "onResponseTimeOut(), req=%d. ", reqId);
    onNwRatSwitchDone(RAT_SWITCH_FAIL);
}

NwsMode RpCdmaLteNwRatSwitchHandler::getNwsModeForSwitchCardType() {
    logD(CDMA_LTE_RAT_CTRL_TAG, "getNwsModeForSwitchCardType(), mNwsModeForSwitchCardType is %d. ",
            mNwsModeForSwitchCardType);
    return mNwsModeForSwitchCardType;
}

RatSwitchCaller RpCdmaLteNwRatSwitchHandler::getRatSwitchCaller(){
    return mRatSwitchCaller;
}

