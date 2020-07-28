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

#include "RpGsmNwRatSwitchHandler.h"

#define GSM_RAT_CTRL_TAG "RpGsmNwRatSwitchHandler"

/*****************************************************************************
 * Class RpGsmNwRatSwitchHandler
 *****************************************************************************/

RFX_IMPLEMENT_CLASS("RpGsmNwRatSwitchHandler", RpGsmNwRatSwitchHandler, RpBaseNwRatSwitchHandler);

RpGsmNwRatSwitchHandler::RpGsmNwRatSwitchHandler() :
        RpBaseNwRatSwitchHandler(),
        mAction(NULL) {
}

RpGsmNwRatSwitchHandler::RpGsmNwRatSwitchHandler(RpNwRatController* nwRatController) :
    RpBaseNwRatSwitchHandler(nwRatController),
    mAction(NULL) {
}

RpGsmNwRatSwitchHandler::~RpGsmNwRatSwitchHandler() {
}

void RpGsmNwRatSwitchHandler :: onInit () {
    RfxObject::onInit();  // Required: invoke super class implementation
    logD(LOG_TAG, "onInit");

    char prop_svlte_support[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ro.vendor.mtk_c2k_lte_mode", prop_svlte_support, "0");
    logD(LOG_TAG,"is tdd data only support: %d", atoi(prop_svlte_support));
    int activeModem = 0;
    char prop_val[PROPERTY_VALUE_MAX] = {0};
    property_get("vendor.ril.active.md", prop_val, "0");
    activeModem = atoi(prop_val);
    char tempstr[PROPERTY_VALUE_MAX] = {0};
    property_get("persist.vendor.radio.simswitch", tempstr, "1");
    int capabilitySlotId = atoi(tempstr) - 1;
    bool isMajorSlot = false;
    if (capabilitySlotId == mSlotId) {
        isMajorSlot = true;
    }
    if (activeModem != 4 && activeModem != 6) {
        if (atoi(prop_svlte_support) == 1) {
        logD(LOG_TAG, "switch for GSM card to FDD+TDD LTE");
            if (isMajorSlot) {
                int msg_id = RIL_LOCAL_REQUEST_SET_FDD_TDD_MODE;
                sp<RfxMessage> gsmMessage = RfxMessage::obtainRequest(mSlotId,
                        RADIO_TECH_GROUP_GSM, msg_id);
                gsmMessage->getParcel()->writeInt32(1);
                gsmMessage->getParcel()->writeInt32(SVLTE_FDD_TDD_MODE);
                mNwRatController->requestToRild(gsmMessage);
            }
        }
    }
}
void RpGsmNwRatSwitchHandler::requestGetPreferredNetworkType(const sp<RfxMessage>& message) {
    RpBaseNwRatSwitchHandler::requestGetPreferredNetworkType(RADIO_TECH_GROUP_GSM, message);
}

void RpGsmNwRatSwitchHandler::responseGetPreferredNetworkType(const sp<RfxMessage>& response) {
    int error = response->getError() ;
    if (error != RIL_E_SUCCESS) {
        // error
        logD(GSM_RAT_CTRL_TAG, "responseGetPreferredNetworkType(), error is %d.", error);
        mNwRatController->responseToRilj(response);
    } else {
        // ok
        int source = response->getSource();
        int32_t stgCount = 0;
        int32_t nwType = 0;
        response->getParcel()->readInt32(&stgCount);
        response->getParcel()->readInt32(&nwType);
        if (source == RADIO_TECH_GROUP_GSM) {
            sp<RfxMessage> resToRilj = RfxMessage::obtainResponse(RIL_E_SUCCESS, response);
            resToRilj->getParcel()->writeInt32(1);
            resToRilj->getParcel()->writeInt32(nwType);
            logD(GSM_RAT_CTRL_TAG, "responseGetPreferredNetworkType(), send response %d.", nwType);
            mNwRatController->responseToRilj(resToRilj);
        }
    }
}

void RpGsmNwRatSwitchHandler::responseGetPreferredNetworkType(const int nwType,
        const sp<RfxMessage>& message) {
    int source = message->getSource();
    if (source == RADIO_TECH_GROUP_GSM) {
        sp<RfxMessage> resToRilj = RfxMessage::obtainResponse(RIL_E_SUCCESS, message);
        resToRilj->getParcel()->writeInt32(1);
        resToRilj->getParcel()->writeInt32(nwType);
        logD(GSM_RAT_CTRL_TAG, "responseGetPreferredNetworkType(), send response %d via "
                "mPreferredNetWorkTypeFromRILJ.", nwType);
        mNwRatController->responseToRilj(resToRilj);
    }
}

void RpGsmNwRatSwitchHandler::requestSetPreferredNetworkType(const int prefNwType,
        const sp<RfxMessage>& message) {
    if (prefNwType != -1) {
        sp<RfxMessage> msg;
        if (message != NULL) {
            msg = RfxMessage::obtainRequest(RADIO_TECH_GROUP_GSM,
                    RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE, message);
        } else {
            msg = RfxMessage::obtainRequest(mSlotId,
                    RADIO_TECH_GROUP_GSM, RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE);
        }
        msg->getParcel()->writeInt32(1);
        msg->getParcel()->writeInt32(prefNwType);
        mNwRatController->requestToRild(msg);
    }
}

void RpGsmNwRatSwitchHandler::responseSetPreferredNetworkType(const sp<RfxMessage>& response) {
    int error = response->getError() ;

    RatSwitchResult switchResult = RAT_SWITCH_FAIL;
    if (error != RIL_E_SUCCESS) {
        // error
        mNwRatController->responseToRilj(response);
    } else {
        // ok
        if (response->getSource() == RADIO_TECH_GROUP_GSM) {
            mNwRatController->responseToRilj(response);
            switchResult = RAT_SWITCH_SUCC;
        } else {
            logD(GSM_RAT_CTRL_TAG, "responseSetPreferredNetworkType(), response src invalid!");
        }
    }

    onNwRatSwitchDone(switchResult);

    //Any result, we should do pending record.
    if (!RpNwRatController::getSwitchState()) {
        mNwRatController->doPendingRatSwitchRecord();
    }
}

ModemSettings RpGsmNwRatSwitchHandler::calculateModemSettings(int prefNwType,
        AppFamilyType appFamType, NwsMode nwsMode, VolteState volteState) {
    bool isValid = false;
    int convNwType = -1;
    ModemSettings mdSettings;
    char tempstr[PROPERTY_VALUE_MAX] = {0};
    property_get("persist.vendor.radio.simswitch", tempstr, "1");
    int capabilitySlotId = atoi(tempstr) - 1;
    int slotCapability = mNwRatController->getStatusManager(mSlotId)->getIntValue(RFX_STATUS_KEY_SLOT_CAPABILITY, 0);

    if (appFamType == APP_FAM_3GPP) {
        // GSM SIM only CSFB mode is valid
        if (nwsMode == NWS_MODE_CSFB) {
            mdSettings.md1Radio = true;
            mdSettings.md3Radio = false;
            switch (prefNwType) {
                case PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA:
                    convNwType = PREF_NET_TYPE_LTE_GSM_WCDMA;
                    mdSettings.erat = 6;
                    isValid = true;
                    // convert rat mode depend on capability
                    if (capabilitySlotId != mSlotId && (slotCapability & RAF_LTE) != RAF_LTE) {
                        if ((slotCapability & RAF_UMTS) == RAF_UMTS) {
                            convNwType = PREF_NET_TYPE_GSM_WCDMA;
                            mdSettings.erat = 2;
                        } else {
                            convNwType = PREF_NET_TYPE_GSM_ONLY;
                            mdSettings.erat = 0;
                        }
                    }
                    break;
                case PREF_NET_TYPE_LTE_CDMA_EVDO:
                case PREF_NET_TYPE_LTE_GSM_WCDMA:
                case PREF_NET_TYPE_TD_SCDMA_GSM_LTE:
                case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_LTE:
                case PREF_NET_TYPE_TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA:
                    // convert global mode to 234G
                    convNwType = PREF_NET_TYPE_LTE_GSM_WCDMA;
                    mdSettings.erat = 6;
                    isValid = true;
                    break;
                case PREF_NET_TYPE_GSM_WCDMA:
                case PREF_NET_TYPE_TD_SCDMA_GSM:
                case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA:
                case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO:

                    // convert global mode to 3G Prefer
                    convNwType = PREF_NET_TYPE_GSM_WCDMA;
                    mdSettings.erat = 2;
                    isValid = true;
                    break;
                case PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO:
                case PREF_NET_TYPE_GSM_WCDMA_AUTO:
                case PREF_NET_TYPE_CDMA_EVDO_AUTO:
                case PREF_NET_TYPE_CDMA_ONLY:
                case PREF_NET_TYPE_EVDO_ONLY:
                    // convert global mode to 23G
                    convNwType = PREF_NET_TYPE_GSM_WCDMA_AUTO;
                    mdSettings.erat = 2;
                    isValid = true;
                    break;
                case PREF_NET_TYPE_GSM_ONLY:
                    convNwType = PREF_NET_TYPE_GSM_ONLY;
                    mdSettings.erat = 0;
                    isValid = true;
                    break;
                case PREF_NET_TYPE_WCDMA:
                case PREF_NET_TYPE_TD_SCDMA_ONLY:
                case PREF_NET_TYPE_TD_SCDMA_WCDMA:
                    convNwType = PREF_NET_TYPE_WCDMA;
                    mdSettings.erat = 1;
                    isValid = true;
                    break;
                case PREF_NET_TYPE_LTE_ONLY:
                    convNwType = PREF_NET_TYPE_LTE_ONLY;
                    mdSettings.erat = 3;
                    isValid = true;
                    break;
                case PREF_NET_TYPE_LTE_WCDMA:
                case PREF_NET_TYPE_TD_SCDMA_LTE:
                case PREF_NET_TYPE_TD_SCDMA_WCDMA_LTE:
                    convNwType = PREF_NET_TYPE_LTE_WCDMA;
                    mdSettings.erat = 5;
                    isValid = true;
                    break;
                case PREF_NET_TYPE_LTE_GSM:
                    convNwType = PREF_NET_TYPE_LTE_GSM;
                    mdSettings.erat = 4;
                    isValid = true;
                    break;
            }
        } else if (nwsMode == NWS_MODE_LTEONLY) {
            mdSettings.md1Radio = true;
            mdSettings.md3Radio = false;
            if (prefNwType >= 0) {
                convNwType = PREF_NET_TYPE_LTE_ONLY;
                mdSettings.erat = 3;
                isValid = true;
            }
        }
    }

    if (isValid) {
        mdSettings.prefNwType = convNwType;
        mdSettings.appFamType = appFamType;
        mdSettings.nwsMode = nwsMode;
        mdSettings.volteState = volteState;
        logD(GSM_RAT_CTRL_TAG, "[calculateModemSettings] prefNwType:%d appFamType:%d nwsMode:%d"
                " erat:%d prefmode:%d svlteMode:%d md1Radio:%s md3Radio:%s"
                " ectmode:%d evdomode:%d slotCapability:%d capabilitySlotId:%d",
                mdSettings.prefNwType, mdSettings.appFamType, mdSettings.nwsMode,
                mdSettings.erat, mdSettings.prefmode, mdSettings.svlteMode,
                mdSettings.md1Radio ? "on" : "off",
                mdSettings.md3Radio ? "on" : "off",
                mdSettings.ectmode, mdSettings.evdomode,
                slotCapability, capabilitySlotId);
    } else {
        mdSettings.prefNwType = -1;
        logD(GSM_RAT_CTRL_TAG, "[calculateModemSettings] invalid switch request!!"
                " prefNwType:%d appFamType:%d nwsMode:%d slotCapability:%d capabilitySlotId:%d",
                mdSettings.prefNwType, mdSettings.appFamType, mdSettings.nwsMode,
                slotCapability, capabilitySlotId);
    }

    return mdSettings;
}

void RpGsmNwRatSwitchHandler::doNwRatSwitch(ModemSettings mdSettings,
        const RatSwitchCaller& ratSwitchCaller, const sp<RfxAction>& action,
        const sp<RfxMessage>& message) {
    // logD(GSM_RAT_CTRL_TAG,
    //         "doNwRatSwitch(), prefNwType to %d. NwsMode is %d, ratSwitchCaller is %d",
    //         mdSettings.prefNwType, mdSettings.nwsMode, ratSwitchCaller);

    if (mNwRatController->isEratExtSupport()) {
        mMdSettings.prefNwType = mdSettings.prefNwType;
        mMdSettings.appFamType = mdSettings.appFamType;
        mMdSettings.nwsMode = mdSettings.nwsMode;
        mMdSettings.volteState = mdSettings.volteState;
        mMdSettings.ratSwitchCaller = mdSettings.ratSwitchCaller;
        mAction = action;

        // [VoLTE] For Dual VoLTE
        if (mNwRatController->is4GNetworkMode(mMdSettings.prefNwType)
                && mNwRatController->needConfigRatModeForCtDualVolte()) {
            mNwRatController->onRatSwitchStart(mMdSettings.prefNwType, mMdSettings.nwsMode,
                    mMdSettings.volteState);
        }
    }

    requestSetPreferredNetworkType(mdSettings.prefNwType, message);
}

void RpGsmNwRatSwitchHandler::onNwRatSwitchDone(
        RatSwitchResult switchResult, int errorCode) {
    if (mNwRatController->isEratExtSupport()) {
        if (mMdSettings.prefNwType != -1) {
            // logD(GSM_RAT_CTRL_TAG,
            //         "onNwRatSwitchDone, switchResult = %d, mPrefNwType = %d, mNwsMode = %d,"
            //         " errorCode = %d, update state...",
            //         switchResult, mMdSettings.prefNwType, mMdSettings.nwsMode, errorCode);

            updatePhone(mMdSettings);
            mNwRatController->updateState(mMdSettings.nwsMode, mMdSettings.prefNwType,
                    mMdSettings.volteState);

            // [VoLTE] For Dual VoLTE
            if (mNwRatController->is4GNetworkMode(mMdSettings.prefNwType)
                    && mNwRatController->needConfigRatModeForCtDualVolte()) {
                mNwRatController->onRatSwitchDone(mMdSettings.prefNwType);
            }

            mNwRatController->handleRatSwitchResult(mMdSettings.nwsMode, mMdSettings.prefNwType,
                    mMdSettings.volteState, mMdSettings.ratSwitchCaller, switchResult, errorCode);

            if (mAction != NULL) {
                mAction->act();
            }

            // Nw rat switch done, reset mMdSettings.prefNwType to -1.
            mMdSettings.prefNwType = -1;
        }
    } else {
        logD(GSM_RAT_CTRL_TAG, "onNwRatSwitchDone, switchResult = %d, isEratExtSupport:false",
                switchResult);
    }
}
