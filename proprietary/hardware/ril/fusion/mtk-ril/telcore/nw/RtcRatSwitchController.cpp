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
#include "RfxMisc.h"
#include "RfxRilUtils.h"
#include "RtcRatSwitchController.h"

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/

#define RAT_CTRL_TAG "RtcRatSwCtrl"

RFX_IMPLEMENT_CLASS("RtcRatSwitchController", RtcRatSwitchController, RfxController);

RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_SET_PREFERRED_NETWORK_TYPE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxIntsData, RFX_MSG_REQUEST_GET_PREFERRED_NETWORK_TYPE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxIntsData, RFX_MSG_REQUEST_VOICE_RADIO_TECH);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxIntsData, RFX_MSG_REQUEST_GET_GMSS_RAT_MODE);

RFX_REGISTER_DATA_TO_URC_ID(RfxIntsData, RFX_MSG_URC_VOICE_RADIO_TECH_CHANGED);

#ifndef RIL_LOCAL_REQUEST_OEM_HOOK_ATCI_INTERNAL
#define RIL_LOCAL_REQUEST_OEM_HOOK_ATCI_INTERNAL 1
#endif
#define VOLTE_REGISTERATION_STATE 0x01
bool RtcRatSwitchController::sIsInSwitching = false;

static const char PROPERTY_ICCID_SIM[4][25] = {
    "vendor.ril.iccid.sim1",
    "vendor.ril.iccid.sim2",
    "vendor.ril.iccid.sim3",
    "vendor.ril.iccid.sim4"
};

static const char *DISABLE_CDMA_RAT_MCCMNC[] = {
    "51009", "51028", "45006"
};

#define CACHE_ICCID_SIZE          4
#define PROPERTY_CHCHE_ICCID      "persist.vendor.radio.icc_cache"
#define PROPERTY_CHCHE_NW_TYPE    "persist.vendor.radio.nw_type_cache"
#define PROPERTY_OPERATOR         "persist.vendor.operator.optr"
#define OPERATOR_OP12             "OP12"
#define OPERATOR_OP20             "OP20"

RtcRatSwitchController::RtcRatSwitchController() :
    mDefaultNetworkType(-1),
    mCurPreferedNetWorkType(-1),
    mPhoneMode(RADIO_TECH_UNKNOWN),
    mNwsMode(NWS_MODE_CSFB),
    mNetworkTypeIndex(-1),
    mNetworkTypeCache(-1),
    mRestrictedCount(0),
    mInRestrictedMode(false),
    mAbortAction(NULL) {
    mTokensGetEgmssForHvolte.clear();
}

RtcRatSwitchController::~RtcRatSwitchController() {
}

void RtcRatSwitchController::onInit() {
    // Required: invoke super class implementation
    RfxController::onInit();
    logV(RAT_CTRL_TAG, "[onInit]");
    const int request_id_list[] = {
        RFX_MSG_REQUEST_SET_PREFERRED_NETWORK_TYPE,
        RFX_MSG_REQUEST_GET_PREFERRED_NETWORK_TYPE,
        RFX_MSG_REQUEST_VOICE_RADIO_TECH,
        RFX_MSG_REQUEST_GET_GMSS_RAT_MODE,
        RFX_MSG_REQUEST_LOCAL_ABORT_AVAILABLE_NETWORK
    };

    const int atci_request_id_list[] = {
        RIL_LOCAL_REQUEST_OEM_HOOK_ATCI_INTERNAL
    };

    const int urc_id_list[] = {
        RFX_MSG_URC_GMSS_RAT_CHANGED
    };

    // register request & URC id list
    // NOTE. one id can only be registered by one controller
    registerToHandleRequest(request_id_list, sizeof(request_id_list)/sizeof(const int), DEFAULT);
    registerToHandleRequest(atci_request_id_list,
            sizeof(atci_request_id_list)/sizeof(const int), HIGHEST);
    registerToHandleUrc(urc_id_list, sizeof(urc_id_list)/sizeof(const int));

    getStatusManager()->setIntValue(RFX_STATUS_KEY_PREFERRED_NW_TYPE, mCurPreferedNetWorkType);
    getStatusManager()->setBoolValue(RFX_STATUS_KEY_IS_RAT_MODE_SWITCHING, false);
    getStatusManager()->setIntValue(RFX_STATUS_KEY_NWS_MODE, NWS_MODE_UNKNOWN);

    for (int slotId = RFX_SLOT_ID_0; slotId < RFX_SLOT_COUNT; slotId++) {
        getStatusManager(slotId)->registerStatusChangedEx(RFX_STATUS_KEY_AP_VOICE_CALL_COUNT,
                RfxStatusChangeCallbackEx(this, &RtcRatSwitchController::onApVoiceCallCountChanged));
    }
#ifndef MTK_TC1_COMMON_SERVICE
    if (RfxRilUtils::isCtVolteSupport()) {
        getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_VOLTE_STATE,
                RfxStatusChangeCallback(this, &RtcRatSwitchController::onVolteStateChanged));
    }
    if (isHvolteSupport()) {
        getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_IMS_REGISTRATION_STATE,
            RfxStatusChangeCallback(this, &RtcRatSwitchController::onImsRegistrationStatusChanged));
    }
#endif
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_UICC_GSM_NUMERIC,
            RfxStatusChangeCallback(this, &RtcRatSwitchController::onUiccMccMncChanged));
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_UICC_CDMA_NUMERIC,
            RfxStatusChangeCallback(this, &RtcRatSwitchController::onUiccMccMncChanged));

    if (isECBMRestrictedModeSupport()) {
        getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_EMERGENCY_MODE,
                RfxStatusChangeCallback(this, &RtcRatSwitchController::onRestrictedModeChanged));
        getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_EMERGENCY_CALLBACK_MODE,
                RfxStatusChangeCallback(this, &RtcRatSwitchController::onRestrictedModeChanged));
    }
}

void RtcRatSwitchController::onDeInit() {
    for (int slotId = RFX_SLOT_ID_0; slotId < RFX_SLOT_COUNT; slotId++) {
        getStatusManager(slotId)->unRegisterStatusChangedEx(RFX_STATUS_KEY_AP_VOICE_CALL_COUNT,
                RfxStatusChangeCallbackEx(this, &RtcRatSwitchController::onApVoiceCallCountChanged));
    }
#ifndef MTK_TC1_COMMON_SERVICE
    if (RfxRilUtils::isCtVolteSupport()) {
        getStatusManager()->unRegisterStatusChanged(RFX_STATUS_KEY_VOLTE_STATE,
                RfxStatusChangeCallback(this, &RtcRatSwitchController::onVolteStateChanged));
    }
    if (isHvolteSupport()) {
        getStatusManager()->unRegisterStatusChanged(RFX_STATUS_KEY_IMS_REGISTRATION_STATE,
           RfxStatusChangeCallback(this, &RtcRatSwitchController::onImsRegistrationStatusChanged));
    }
#endif
    getStatusManager()->unRegisterStatusChanged(RFX_STATUS_KEY_UICC_GSM_NUMERIC,
        RfxStatusChangeCallback(this, &RtcRatSwitchController::onUiccMccMncChanged));
    getStatusManager()->unRegisterStatusChanged(RFX_STATUS_KEY_UICC_CDMA_NUMERIC,
        RfxStatusChangeCallback(this, &RtcRatSwitchController::onUiccMccMncChanged));

    if (isECBMRestrictedModeSupport()) {
        getStatusManager()->unRegisterStatusChanged(RFX_STATUS_KEY_EMERGENCY_MODE,
                RfxStatusChangeCallback(this, &RtcRatSwitchController::onRestrictedModeChanged));
        getStatusManager()->unRegisterStatusChanged(RFX_STATUS_KEY_EMERGENCY_CALLBACK_MODE,
                RfxStatusChangeCallback(this, &RtcRatSwitchController::onRestrictedModeChanged));
    }
    RfxController::onDeinit();
}

bool RtcRatSwitchController::onHandleRequest(const sp<RfxMessage>& message) {
    int msg_id = message->getId();
    // logD(RAT_CTRL_TAG, "[onHandleRequest] %s", RFX_ID_TO_STR(msg_id));

    switch (msg_id) {
        case RFX_MSG_REQUEST_SET_PREFERRED_NETWORK_TYPE:
            setPreferredNetworkType(message);
            break;
        case RFX_MSG_REQUEST_GET_PREFERRED_NETWORK_TYPE:
            getPreferredNetworkType(message);
            break;
        case RFX_MSG_REQUEST_VOICE_RADIO_TECH:
            requestVoiceRadioTech(message);
            break;
        default:
            break;
    }
    return true;
}

bool RtcRatSwitchController::onHandleUrc(const sp<RfxMessage>& message) {
    int msg_id = message->getId();
    logV(RAT_CTRL_TAG, "[onHandleUrc] %s", RFX_ID_TO_STR(msg_id));
    switch (msg_id) {
        case RFX_MSG_URC_GMSS_RAT_CHANGED:
            handleGmssRatChanged(message);
            return true;
        default:
            logW(RAT_CTRL_TAG, "[onHandleUrc] default case");
            break;
    }
    return true;
}

bool RtcRatSwitchController::onHandleResponse(const sp<RfxMessage>& response) {
    int msg_id = response->getId();

    switch (msg_id) {
        case RFX_MSG_REQUEST_SET_PREFERRED_NETWORK_TYPE:
            responseSetPreferredNetworkType(response);
            return true;
        case RFX_MSG_REQUEST_GET_PREFERRED_NETWORK_TYPE:
            responseGetPreferredNetworkType(response);
            return true;
        case RFX_MSG_REQUEST_VOICE_RADIO_TECH:
            responseGetVoiceRadioTech(response);
            return true;
        case RFX_MSG_REQUEST_GET_GMSS_RAT_MODE:
            responseGetGmssRatMode(response);
            return true;
        case RFX_MSG_REQUEST_LOCAL_ABORT_AVAILABLE_NETWORK:
            if (mAbortAction != NULL) {
                mAbortAction->act();
                mAbortAction = NULL;
            } else {
                if (isAPInCall() != true) {
                    processPendingRatSwitchRecord();
                }
            }
            return true;
        default:
            logW(RAT_CTRL_TAG, "[onHandleResponse] %s", RFX_ID_TO_STR(msg_id));
            break;
    }
    return false;
}

bool RtcRatSwitchController::onHandleAtciRequest(const sp<RfxMessage>& message) {
    int msg_id = message->getId();
    int dataLength;
    const char *data;
    char *pString = (char *)message->getData();

    switch (msg_id) {
        case RIL_LOCAL_REQUEST_OEM_HOOK_ATCI_INTERNAL:
            data = &pString[0];
            dataLength = strlen(data);

            logD(RAT_CTRL_TAG, "Inject AT command %s (length:%d)", data, dataLength);
            if (dataLength > 0 && strncmp(data, "AT+ERAT=", 8) == 0) {
                int rat = -1;
                int pref_rat = -1;
                int targetSlotId = 0;
                char simNo[RFX_PROPERTY_VALUE_MAX] = {0};
                rfx_property_get("persist.vendor.service.atci.sim", simNo, "0");
                logD(RAT_CTRL_TAG, "[onHandleAtciRequest] simNo: %c", simNo[0]);

                if (simNo[0] == '0') {
                    targetSlotId = 0;
                } else if (simNo[0] == '1') {
                    targetSlotId = 1;
                } else {
                    logD(RAT_CTRL_TAG, "Not support slot: %d", simNo[0]);
                    break;
                }
                if (targetSlotId == m_slot_id) {
                    sscanf(data, "AT+ERAT=%d", &rat);
                    switch (rat) {
                        case 0:
                            mCurPreferedNetWorkType = PREF_NET_TYPE_GSM_ONLY;
                            break;
                        case 1:
                            mCurPreferedNetWorkType = PREF_NET_TYPE_WCDMA;
                            break;
                        case 2:
                            mCurPreferedNetWorkType = PREF_NET_TYPE_GSM_WCDMA;
                            break;
                        case 3:
                            mCurPreferedNetWorkType = PREF_NET_TYPE_LTE_ONLY;
                            break;
                        case 4:
                            mCurPreferedNetWorkType = PREF_NET_TYPE_LTE_GSM;
                            break;
                        case 5:
                            mCurPreferedNetWorkType = PREF_NET_TYPE_LTE_WCDMA;
                            break;
                        case 6:
                            mCurPreferedNetWorkType = PREF_NET_TYPE_LTE_GSM_WCDMA;
                            break;
                        case 7:
                            mCurPreferedNetWorkType = PREF_NET_TYPE_CDMA_EVDO_AUTO;
                            break;
                        case 8:
                            mCurPreferedNetWorkType = PREF_NET_TYPE_CDMA_EVDO_GSM;
                            break;
                        case 10:
                            mCurPreferedNetWorkType = PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO;
                            break;
                        case 11:
                            mCurPreferedNetWorkType = PREF_NET_TYPE_LTE_CDMA_EVDO;
                            break;
                        case 12:
                            mCurPreferedNetWorkType = PREF_NET_TYPE_LTE_CDMA_EVDO_GSM;
                            break;
                        case 14:
                            mCurPreferedNetWorkType = PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA;
                            break;
                        default:
                            break;
                    }
                    getStatusManager()->setIntValue(RFX_STATUS_KEY_PREFERRED_NW_TYPE,
                            mCurPreferedNetWorkType);
                }
                logD(RAT_CTRL_TAG, "[onHandleAtciRequest] mCurPreferedNetWorkType=%d",
                        mCurPreferedNetWorkType);
            } else {
                logW(RAT_CTRL_TAG, "[onHandleAtciRequest] length=0");
            }
            break;

        default:
            break;
    }
    return false;
}

void RtcRatSwitchController::setPreferredNetworkType(const sp<RfxMessage>& message) {
    // there is no sim, so ignore it
    int cardType = getStatusManager(m_slot_id)->getIntValue(
        RFX_STATUS_KEY_CARD_TYPE, 0);
    if (cardType == CARD_TYPE_NONE) {
        logD(RAT_CTRL_TAG, "setPreferredNetworkType: return directly because no sim.");
        sp<RfxMessage> resToRilj = RfxMessage::obtainResponse(
                RIL_E_INTERNAL_ERR, message, false);
        responseToRilj(resToRilj);
        return;
    }

    int nwType = ((int *)message->getData()->getData())[0];
    // No matter switch success or fail, always record the rilj prefer network type in system property.
    // This is consistent with the UI behavior.
    logV(RAT_CTRL_TAG, "[setPrefNwType] from RILJ, rilj_nw_type set to %d, mInRestrictedMode:%d",
            nwType, mInRestrictedMode);
    setPreferredNetWorkTypeToSysProp(m_slot_id, nwType);
#ifndef MTK_TC1_COMMON_SERVICE
    if (mInRestrictedMode == true) {
        mInRestrictedMode = false;
    }
#endif
    switchNwRat(nwType, RAT_SWITCH_NORMAL, NULL, message);
}

void RtcRatSwitchController::setPreferredNetworkType(RatSwitchInfo ratSwtichInfo) {
    logV(RAT_CTRL_TAG, "[setPreferredNetworkType] ratSwtichInfo.card_type :%d, "
                    "ratSwtichInfo.card_state: %d, ratSwtichInfo.rat_mode: %d, ratSwtichInfo.isCt3GDualMode: %s, ratSwtichInfo.ct3gStatus : %d. ",
            ratSwtichInfo.card_type, ratSwtichInfo.card_state,
            ratSwtichInfo.rat_mode,
            ratSwtichInfo.isCt3GDualMode ? "true" : "false",
            ratSwtichInfo.ct3gStatus);
    //Only card not plug in/out and not occur sim switch, the card state will be card_state_no_changed
    int defaultNetworkType = calculateDefaultNetworkType(ratSwtichInfo);
    if (ratSwtichInfo.card_state == CARD_STATE_NO_CHANGED && defaultNetworkType == mDefaultNetworkType) {
        logD(RAT_CTRL_TAG, "Card state no changed! setPreferredNetworkType not executed!");
        mRatSwitchSignal.emit(m_slot_id, RIL_E_SUCCESS);
        return;
    }
    // Remove store iccid feature
#if 0
    mNetworkTypeIndex = getIccIdCacheIndex(m_slot_id);
    mNetworkTypeCache = getNwTypeCacheByIndex(mNetworkTypeIndex);
    // correct RILJ NW type from cache.
    if (mNetworkTypeCache != -1) {
        setPreferredNetWorkTypeToSysProp(m_slot_id, mNetworkTypeCache);
    }
#endif
    /* In no SIM case.
     * 1. Switch RAT if ECC retry.
     * 2. Swithc RAT if card not hot plug case
         a. First time boot up
         b. RIL proxy restart
     * 3. Skip RAT switch if C capability does not change
     */
    if (ratSwtichInfo.card_type == 0
            && !(ratSwtichInfo.rat_mode != RAT_MODE_INVALID  // not ECC retry
            || ratSwtichInfo.card_state == CARD_STATE_NOT_HOT_PLUG)) {  // not hot plug case.
        int slotCapability = getStatusManager(m_slot_id)->getIntValue(
                RFX_STATUS_KEY_SLOT_CAPABILITY, 0);
        int PreviousRaf = getSlotCapability(m_slot_id);
        logV(RAT_CTRL_TAG, "slotCapability:%d PreviousRaf:%d ", slotCapability, PreviousRaf);
        if (getSlotCapability(m_slot_id) != -1
                && !(isRafContainsCdma(PreviousRaf) ^ isRafContainsCdma(slotCapability))) {
            // case 3. do not change RAT.
            logD(RAT_CTRL_TAG, "Skip switch if C2K no change, slotCapability:%d PreviousRaf:%d ",
                    slotCapability, PreviousRaf);
            setSlotCapability(m_slot_id, getStatusManager(m_slot_id)->getIntValue(
                    RFX_STATUS_KEY_SLOT_CAPABILITY, 0));
            mRatSwitchSignal.emit(m_slot_id, RIL_E_SUCCESS);
            return;
        }
    }

    // case 1, 2. change RAT.
    setSlotCapability(m_slot_id, getStatusManager(m_slot_id)->getIntValue(
            RFX_STATUS_KEY_SLOT_CAPABILITY, 0));
    mRatSettings.prefNwTypeDefault = defaultNetworkType;
    int defaultRaf = RtcCapabilitySwitchUtil::getRafFromNetworkType(defaultNetworkType);
    int rafFromRiljNetworkType = RtcCapabilitySwitchUtil::getRafFromNetworkType(
            getPreferredNetWorkTypeFromSysProp(m_slot_id));
    if (ratSwtichInfo.isCt3GDualMode && (ratSwtichInfo.card_state == CARD_STATE_NOT_HOT_PLUG
            || ratSwtichInfo.card_state == CARD_STATE_HOT_PLUGIN)) {
        rafFromRiljNetworkType = RtcCapabilitySwitchUtil::getRafFromNetworkType(
                     PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA);
    }
    int targetRaf = defaultRaf;
    // Remove store iccid feature, always use previous rat mode.
#if 0
    int isNewSim = 0;  // isNewSimCard(m_slot_id);

    /* isNewSim value define.
     * -1:no SIM insert, keep previous iccid and nw type.
     * 0:  SIM card no change
     * 1:  new SIM card
     */
    if (isNewSim == 1) {
        setPreferredNetWorkTypeToSysProp(m_slot_id, -1);
    } else if (isNewSim == 0){
#endif
    targetRaf = (defaultRaf & rafFromRiljNetworkType);
    if (targetRaf == 0) {
        logD(RAT_CTRL_TAG, "[setPreferredNetworkType] Raf filter result is 0, "
                "so use defaultRaf as targetRaf");
        targetRaf = defaultRaf;
    }

    if (ratSwtichInfo.isCt3GDualMode
            && ratSwtichInfo.card_state == CARD_STATE_CARD_TYPE_CHANGED
            && isRafContainsCdma(targetRaf)) {
        if (ratSwtichInfo.ct3gStatus == GMSS_TRIGGER_SWITCH_SIM) {
            mNwsMode = NWS_MODE_CSFB;
        } else if (ratSwtichInfo.ct3gStatus == GMSS_TRIGGER_SWITCH_RUIM) {
            mNwsMode = NWS_MODE_CDMALTE;
        } else {
            mNwsMode = NWS_MODE_CDMALTE;
        }
    } else if (isRafContainsCdma(targetRaf)) {
        mNwsMode = NWS_MODE_CDMALTE;
    } else {
        mNwsMode = NWS_MODE_CSFB;
    }
    getStatusManager()->setIntValue(RFX_STATUS_KEY_NWS_MODE, mNwsMode);

    int targetNetworkType = RtcCapabilitySwitchUtil::getNetworkTypeFromRaf(targetRaf);
    logD(RAT_CTRL_TAG, "[setPreferredNetworkType] rafFromRiljNetworkType: %d, targetRaf: %d, targetNetworkType: %d, "
             "mNwsMode: %d, mInRestrictedMode: %d",
            rafFromRiljNetworkType, targetRaf, targetNetworkType, mNwsMode, mInRestrictedMode);
#ifndef MTK_TC1_COMMON_SERVICE
    if (mInRestrictedMode == true) {
        mInRestrictedMode = false;
    }
#endif
    switchNwRat(targetNetworkType, RAT_SWITCH_INIT, NULL, NULL);
}

void RtcRatSwitchController::setPreferredNetworkType(const int prefNwType,
        const sp<RfxAction>& action) {
    if (prefNwType == -1) {
        logD(RAT_CTRL_TAG, "[setRestrictedNetworkMode] leaving restricted mode");
        // Change caller to restrict to avoid request can't work before exit ECBM or emergency mode.
        mInRestrictedMode = false;
        if (mPendingInitRatSwitchRecord.prefNwType != -1) {
            logD(RAT_CTRL_TAG, "[setRestrictedNetworkMode] Init pending record in queue");
            mPendingRestrictedRatSwitchRecord.prefNwType = -1;
            switchNwRat(mPendingInitRatSwitchRecord.prefNwType,
                    RAT_SWITCH_INIT, action, NULL);
        } else if (mPendingNormalRatSwitchRecord.prefNwType != -1) {
            // Filter nw type avoid use nw type to switch without capability filter.
            int targetPrefNwType = filterPrefNwTypeFromRilj(
                    mPendingNormalRatSwitchRecord.prefNwType);
            logD(RAT_CTRL_TAG, "[setRestrictedNetworkMode] Norm pending record in queue");
            mPendingRestrictedRatSwitchRecord.prefNwType = -1;
            switchNwRat(targetPrefNwType,
                    RAT_SWITCH_NORMAL, action, mPendingNormalRatSwitchRecord.message);
        } else {
            logD(RAT_CTRL_TAG, "[setRestrictedNetworkMode] No pending record in queue");
            action->act();
            processPendingRatSwitchRecord();
        }
    } else {
        logD(RAT_CTRL_TAG, "[setRestrictedNetworkMode] entering restricted mode: %d",
                prefNwType);
        if (mPendingInitRatSwitchRecord.prefNwType == -1 &&
                mPendingNormalRatSwitchRecord.prefNwType == -1) {
            // save current state to pending queue if no init rat switch in queue.
            queueRatSwitchRecord(mCurPreferedNetWorkType, RAT_SWITCH_RESTRICT, NULL, NULL);
        }
        mInRestrictedMode = true;
        switchNwRat(prefNwType, RAT_SWITCH_RESTRICT, action, NULL);
    }
}

void RtcRatSwitchController::switchNwRat(int prefNwType, const RatSwitchCaller ratSwitchCaller,
        const sp<RfxAction>& action, const sp<RfxMessage>& message) {

    logV(RAT_CTRL_TAG, "[switchNwRat] CurPreferedNwType: %d, prefNwType: %d, NW scan:%d,"
            " ratSwitchCaller: %s, sIsInSwitching: %s",
            mCurPreferedNetWorkType, prefNwType, isNetworkScanOngoing(),
            switchCallerToString(ratSwitchCaller),sIsInSwitching ? "true" : "false");

    if (sIsInSwitching) {
        queueRatSwitchRecord(prefNwType, ratSwitchCaller, action, message);
    } else if ((mRestrictedCount > 0 || mInRestrictedMode)
        && ratSwitchCaller != RAT_SWITCH_RESTRICT) {
        logD(RAT_CTRL_TAG, "[switchNwRat] in restricted mode!");
        queueRatSwitchRecord(prefNwType, ratSwitchCaller, NULL, NULL);
        if (action != NULL) {
            action->act();
        }
        if (message != NULL) {
            sp<RfxMessage> resToRilj = RfxMessage::obtainResponse(RIL_E_SUCCESS, message);
            responseToRilj(resToRilj);
        }
    } else if ((ratSwitchCaller == RAT_SWITCH_NORMAL || ratSwitchCaller == RAT_SWITCH_INIT) &&
                isAPInCall() == true) {

        /* In call case.
         * 1. Init case:   return operation not allowed to mode controller.
         * 2. Normal case: return success to fwk and quene switch rat until AP call count is 0.
         */
        if (ratSwitchCaller == RAT_SWITCH_INIT) {
            mRatSwitchSignal.emit(m_slot_id, RIL_E_OPERATION_NOT_ALLOWED);
        } else {
            if (action != NULL) {
                action->act();
            }
            if (message != NULL) {
                sp<RfxMessage> resToRilj = RfxMessage::obtainResponse(RIL_E_SUCCESS, message);
                responseToRilj(resToRilj);
            }
            queueRatSwitchRecord(prefNwType, ratSwitchCaller, NULL, NULL);
        }
        logD(RAT_CTRL_TAG, "[switchNwRat] in call, do not set rat!");
    } else {
        sp<RfxMessage> resToRilj;
        int targetPrefNwType = prefNwType;

        if (ratSwitchCaller == RAT_SWITCH_NORMAL) {
            targetPrefNwType = filterPrefNwTypeFromRilj(prefNwType);

            if (targetPrefNwType == -1) {
                logD(RAT_CTRL_TAG, "[switchNwRat] from RILJ, invalid nwType:%d", prefNwType);
                if (message != NULL) {
                    sp<RfxMessage> resToRilj = RfxMessage::obtainResponse(RIL_E_GENERIC_FAILURE,
                            message);
                    responseToRilj(resToRilj);
                }
                processPendingRatSwitchRecord();
                return;
            }
        }

        if (targetPrefNwType == mCurPreferedNetWorkType && ratSwitchCaller != RAT_SWITCH_INIT) {
            logV(RAT_CTRL_TAG, "[switchNwRat] Already in desired mode:%d, switch not executed", targetPrefNwType);
            if (action != NULL) {
                action->act();
            }
            if (message != NULL) {
                if (ratSwitchCaller == RAT_SWITCH_NORMAL) {
                    mRatSettings.prefNwTypeFromRilj = prefNwType;
                }
                resToRilj = RfxMessage::obtainResponse(RIL_E_SUCCESS, message);
                responseToRilj(resToRilj);
            }
            processPendingRatSwitchRecord();
            return;
        }
        if (isNetworkScanOngoing() != -1) {
            queueRatSwitchRecord(prefNwType, ratSwitchCaller, action, message);
            sp<RfxMessage> reqToRild = RfxMessage::obtainRequest(isNetworkScanOngoing(),
                    RFX_MSG_REQUEST_LOCAL_ABORT_AVAILABLE_NETWORK, RfxVoidData());
            requestToMcl(reqToRild);
            return;
        }
        logD(RAT_CTRL_TAG, "[switchNwRat] ratSwitchCaller: %s, mCurPreferedNetWorkType: %d, "
                "mDefaultNetworkType: %d, prefNwTypeFromRilj: %d, "
                "targetPrefNwType: %d, GsmOnlySim: %d",
                switchCallerToString(ratSwitchCaller),
                mCurPreferedNetWorkType, mDefaultNetworkType,
                prefNwType, targetPrefNwType, isGsmOnlySim());

        sIsInSwitching = true;
        getStatusManager()->setBoolValue(RFX_STATUS_KEY_IS_RAT_MODE_SWITCHING, true);
        if (ratSwitchCaller == RAT_SWITCH_NORMAL) {
            mRatSettings.prefNwTypeFromRilj = prefNwType;
        }
        mRatSettings.prefNwType = targetPrefNwType;
        mRatSettings.ratSwitchCaller = ratSwitchCaller;
        mRatSettings.action = action;
        mRatSettings.message = message;

        int targetRaf = RtcCapabilitySwitchUtil::getRafFromNetworkType(targetPrefNwType);
        int currentRaf = RtcCapabilitySwitchUtil::getRafFromNetworkType(mCurPreferedNetWorkType);
        if (mRatSettings.ratSwitchCaller == RAT_SWITCH_INIT
                && isRafContainsCdma(targetRaf) && isRafContainsGsm(targetRaf)) {
            getGmssRatMode();
        } else if (mRatSettings.ratSwitchCaller == RAT_SWITCH_NORMAL
                && isRafContainsCdma(targetRaf) && !isRafContainsCdma(currentRaf)) {
            // Reset mNwsMode to CDMALTE when change target network type from G only to C.
            mNwsMode = NWS_MODE_CDMALTE;
            if (isRafContainsCdma(targetRaf) && isRafContainsGsm(targetRaf)) {
                getGmssRatMode();
            } else {
                logV(RAT_CTRL_TAG, "switchNwRat(), NW scan ongoing:%d", isNetworkScanOngoing());
                configRatMode();
            }
        } else if ((mRatSettings.ratSwitchCaller == RAT_SWITCH_RESTRICT
                || mRatSettings.ratSwitchCaller == RAT_SWITCH_NORMAL) && isGsmOnlySim()
                && (!isRafContainsCdma(targetRaf) && isRafContainsCdma(currentRaf))) {
            // Reset mNwsMode to CSFB when change target network type from C to G only for GsmSim.
            mNwsMode = NWS_MODE_CSFB;
            getStatusManager()->setIntValue(RFX_STATUS_KEY_NWS_MODE, mNwsMode);
            logV(RAT_CTRL_TAG, "switchNwRat(), NW scan ongoing:%d", isNetworkScanOngoing());
            configRatMode();
        } else {
            logV(RAT_CTRL_TAG, "switchNwRat(), NW scan ongoing:%d", isNetworkScanOngoing());
            configRatMode();
        }
    }
}

void RtcRatSwitchController::configRatMode() {
    sp<RfxMessage> reqToRild = RfxMessage::obtainRequest(m_slot_id,
            RFX_MSG_REQUEST_SET_PREFERRED_NETWORK_TYPE, RfxIntsData(&mRatSettings.prefNwType, 1));
    requestToMcl(reqToRild);
}

void RtcRatSwitchController::queueRatSwitchRecord(int prefNwType,
        const RatSwitchCaller ratSwitchCaller,
        const sp<RfxAction>& action, const sp<RfxMessage>& message) {
    /* Pending if in switching. */
    logV(RAT_CTRL_TAG, "queueRatSwitchRecord(), ratSwitchCaller:%d prefNwType:%d",
            ratSwitchCaller, prefNwType);
    if (ratSwitchCaller == RAT_SWITCH_RESTRICT) {
        mPendingRestrictedRatSwitchRecord.prefNwType = prefNwType;
        mPendingRestrictedRatSwitchRecord.ratSwitchCaller = ratSwitchCaller;
        mPendingRestrictedRatSwitchRecord.action = action;
        mPendingRestrictedRatSwitchRecord.message = message;
    } else if (ratSwitchCaller == RAT_SWITCH_INIT) {
        mPendingInitRatSwitchRecord.prefNwType = prefNwType;
        mPendingInitRatSwitchRecord.ratSwitchCaller = ratSwitchCaller;
        mPendingInitRatSwitchRecord.action = action;
        mPendingInitRatSwitchRecord.message = message;
        mPendingRestrictedRatSwitchRecord.prefNwType = -1;
    } else {
        if (mPendingNormalRatSwitchRecord.prefNwType != -1
                && mPendingNormalRatSwitchRecord.message != NULL) {
            logV(RAT_CTRL_TAG, "queueRatSwitchRecord(), set prefer network type is pending, "
                    "will be ignored, send response.");
            sp<RfxMessage> resToRilj = RfxMessage::obtainResponse(
                    RIL_E_GENERIC_FAILURE, mPendingNormalRatSwitchRecord.message);
            responseToRilj(resToRilj);
        }
        mPendingNormalRatSwitchRecord.prefNwType = prefNwType;
        mPendingNormalRatSwitchRecord.ratSwitchCaller = ratSwitchCaller;
        mPendingNormalRatSwitchRecord.action = action;
        mPendingNormalRatSwitchRecord.message = message;
        mPendingRestrictedRatSwitchRecord.prefNwType = -1;
    }
}

void RtcRatSwitchController::processPendingRatSwitchRecord() {
    // param for INIT, NOR switch caller use, clean pending nw type after request is triggered
    int prefNwType = -1;
    if (mPendingRestrictedRatSwitchRecord.prefNwType != -1 &&
            mInRestrictedMode != true) {
        logD(RAT_CTRL_TAG, "[processPendingRestrictedRatSwitchRecord] "
                "prefNwType: %d, ratSwitchCaller: %s",
                mPendingRestrictedRatSwitchRecord.prefNwType,
                switchCallerToString(mPendingRestrictedRatSwitchRecord.ratSwitchCaller));
        prefNwType = mPendingRestrictedRatSwitchRecord.prefNwType;
        mPendingRestrictedRatSwitchRecord.prefNwType = -1;
        switchNwRat(prefNwType,
                mPendingRestrictedRatSwitchRecord.ratSwitchCaller,
                mPendingRestrictedRatSwitchRecord.action,
                mPendingRestrictedRatSwitchRecord.message);
    } else if (mPendingInitRatSwitchRecord.prefNwType != -1) {
        logD(RAT_CTRL_TAG, "[processPendingInitRatSwitchRecord] "
                "prefNwType: %d, ratSwitchCaller: %s",
                mPendingInitRatSwitchRecord.prefNwType,
                switchCallerToString(mPendingInitRatSwitchRecord.ratSwitchCaller));
        prefNwType = mPendingInitRatSwitchRecord.prefNwType;
        mPendingInitRatSwitchRecord.prefNwType = -1;
        switchNwRat(prefNwType,
                mPendingInitRatSwitchRecord.ratSwitchCaller,
                mPendingInitRatSwitchRecord.action,
                mPendingInitRatSwitchRecord.message);
    } else if (mPendingNormalRatSwitchRecord.prefNwType != -1) {
        /* logD(RAT_CTRL_TAG, "[processPendingNormalRatSwitchRecord] "
                "prefNwType: %d, ratSwitchCaller: %s",
                mPendingNormalRatSwitchRecord.prefNwType,
                switchCallerToString(mPendingNormalRatSwitchRecord.ratSwitchCaller)); */
        prefNwType = mPendingNormalRatSwitchRecord.prefNwType;
        mPendingNormalRatSwitchRecord.prefNwType = -1;
        switchNwRat(prefNwType,
                mPendingNormalRatSwitchRecord.ratSwitchCaller,
                mPendingNormalRatSwitchRecord.action,
                mPendingNormalRatSwitchRecord.message);
    } else {
        RtcRatSwitchController *another = (RtcRatSwitchController *) findController(
                m_slot_id == 0 ? 1 : 0, RFX_OBJ_CLASS_INFO(RtcRatSwitchController));
        if (another != NULL && another->hasPendingRecord()) {
            logV(RAT_CTRL_TAG, "[processPendingRatSwitchRecord] another SIM has pending record");
            another->processPendingRatSwitchRecord();
        } else {
            // logD(RAT_CTRL_TAG, "[processPendingRatSwitchRecord] no pending record");
        }
    }
}

int RtcRatSwitchController::calculateDefaultNetworkType(RatSwitchInfo ratSwtichInfo) {
    int slotCapability = getStatusManager(m_slot_id)->getIntValue(
            RFX_STATUS_KEY_SLOT_CAPABILITY, 0);
    int defaultRaf = slotCapability;

    //For gsm only card(not 3g dual mode card), the default network type can only be pure GSM
    if (isGsmOnlySimFromMode(ratSwtichInfo) == true || needDisableCdmaRat()) {
        defaultRaf &= ~(RAF_CDMA_GROUP | RAF_EVDO_GROUP);
    }

    //For cdma only card, the default network type can only be pure CDMA
    if (isRafContainsCdma(defaultRaf)) {
        if ((((ratSwtichInfo.card_type & CARD_TYPE_RUIM) > 0) || ((ratSwtichInfo.card_type & CARD_TYPE_CSIM) > 0))
                    && (!((ratSwtichInfo.card_type & CARD_TYPE_USIM) > 0))
                    && (!ratSwtichInfo.isCt3GDualMode)) {
            if ((defaultRaf & RAF_EVDO_GROUP) > 0) {
                defaultRaf = RAF_CDMA_GROUP | RAF_EVDO_GROUP;
            } else {
                defaultRaf = RAF_CDMA_GROUP;
            }
        }
    }
    //For ECC suggest rat mode.
    if (ratSwtichInfo.rat_mode != RAT_MODE_INVALID) {
        defaultRaf = slotCapability;
    }
    int defaultNetworkType = RtcCapabilitySwitchUtil::getNetworkTypeFromRaf(
            defaultRaf);

    logD(RAT_CTRL_TAG, "[calculateDefaultNetworkType] card_type: %d, "
            "card_state: %d, rat_mode: %d, isCt3GDualMode: %s, ct3gStatus: %d. "
            " slotCap: %d, slotNwtype: %d, defaultRaf: %d, defNwType: %d.",
            ratSwtichInfo.card_type, ratSwtichInfo.card_state,
            ratSwtichInfo.rat_mode,
            ratSwtichInfo.isCt3GDualMode ? "true" : "false",
            ratSwtichInfo.ct3gStatus,
            slotCapability,
            RtcCapabilitySwitchUtil::getNetworkTypeFromRaf(slotCapability),
            defaultRaf, defaultNetworkType);
    return defaultNetworkType;
}

bool RtcRatSwitchController::hasPendingRecord() {
    if (mPendingInitRatSwitchRecord.prefNwType != -1
            || mPendingNormalRatSwitchRecord.prefNwType != -1
            || mPendingRestrictedRatSwitchRecord.prefNwType != -1) {
        return true;
    }
    return false;
}

void RtcRatSwitchController::getPreferredNetworkType(const sp<RfxMessage>& message) {
    sp<RfxMessage> resToRilj;

    if ((mRestrictedCount > 0 || isAPInCall() == true) &&
        mPendingNormalRatSwitchRecord.prefNwType != -1) {
        logD(RAT_CTRL_TAG, "[handleGetPreferredNwType] in restricted or call mode:%d",
                mPendingNormalRatSwitchRecord.prefNwType);
        resToRilj = RfxMessage::obtainResponse(m_slot_id, RFX_MSG_REQUEST_GET_PREFERRED_NETWORK_TYPE,
                RIL_E_SUCCESS, RfxIntsData(&mPendingNormalRatSwitchRecord.prefNwType, 1), message);

        responseToRilj(resToRilj);
        return;
    } else if (sIsInSwitching == true) {
        logD(RAT_CTRL_TAG, "[handleGetPreferredNwType] in RAT switching Desired:%d",
                mRatSettings.prefNwType);
        resToRilj = RfxMessage::obtainResponse(m_slot_id, RFX_MSG_REQUEST_GET_PREFERRED_NETWORK_TYPE,
                RIL_E_SUCCESS, RfxIntsData(&mRatSettings.prefNwType, 1), message);
        responseToRilj(resToRilj);
        return;
    } else if (mRatSettings.prefNwTypeFromRilj != -1) {
        logD(RAT_CTRL_TAG, "[handleGetPreferredNwType] prefNwTypeFromRilj:%d",
                mRatSettings.prefNwTypeFromRilj);
        resToRilj = RfxMessage::obtainResponse(m_slot_id, RFX_MSG_REQUEST_GET_PREFERRED_NETWORK_TYPE,
                RIL_E_SUCCESS, RfxIntsData(&mRatSettings.prefNwTypeFromRilj, 1), message);
        responseToRilj(resToRilj);
    } else {
        // we provide it from property because rmc may return filtered type.
        int preferredNetworkTypeFromProp = getPreferredNetWorkTypeFromSysProp(m_slot_id);
        logD(RAT_CTRL_TAG, "[handleGetPreferredNwType] preferredNetworkTypeFromProp :%d", preferredNetworkTypeFromProp);
        resToRilj = RfxMessage::obtainResponse(m_slot_id, RFX_MSG_REQUEST_GET_PREFERRED_NETWORK_TYPE,
                RIL_E_SUCCESS, RfxIntsData(&preferredNetworkTypeFromProp, 1), message);
        responseToRilj(resToRilj);
    }
}

void RtcRatSwitchController::requestVoiceRadioTech(const sp<RfxMessage>& message) {
    sp<RfxMessage> resToRilj;
    resToRilj = RfxMessage::obtainResponse(m_slot_id, RFX_MSG_REQUEST_VOICE_RADIO_TECH, RIL_E_SUCCESS,
            RfxIntsData(&mPhoneMode, 1), message);
    responseToRilj(resToRilj);
}

void RtcRatSwitchController::handleGmssRatChanged(const sp<RfxMessage>& message) {
    handleGmssRat(message);
    updatePhoneMode(GMSS_RAT);
}

void RtcRatSwitchController::setLocalAbortAvailableNetwork(const sp<RfxAction>& action) {
    logV(RAT_CTRL_TAG, "NetworkScanOngoing:%d",
            getStatusManager()->getBoolValue(RFX_STATUS_KEY_NETWORK_SCAN_ONGOING));
    if (getStatusManager()->getBoolValue(RFX_STATUS_KEY_NETWORK_SCAN_ONGOING) == true) {
        sp<RfxMessage> reqToRild = RfxMessage::obtainRequest(isNetworkScanOngoing(),
                RFX_MSG_REQUEST_LOCAL_ABORT_AVAILABLE_NETWORK, RfxVoidData());
        requestToMcl(reqToRild);
        mAbortAction = action;
    } else {
        mAbortAction = NULL;
        action->act();
    }
}

void RtcRatSwitchController::updateState(int prefNwType, RatSwitchResult switchResult) {
    logV(RAT_CTRL_TAG, "[updateState] prefNwType: %d, switchResut: %d", prefNwType, switchResult);
    if (switchResult == RAT_SWITCH_SUCC) {
        if (mRatSettings.ratSwitchCaller == RAT_SWITCH_INIT) {
            mDefaultNetworkType = mRatSettings.prefNwTypeDefault;
        }
        mCurPreferedNetWorkType = prefNwType;
        getStatusManager()->setIntValue(RFX_STATUS_KEY_PREFERRED_NW_TYPE,
                mCurPreferedNetWorkType);
    }
    getStatusManager()->setBoolValue(RFX_STATUS_KEY_IS_RAT_MODE_SWITCHING, false);
    sIsInSwitching = false;
}

void RtcRatSwitchController::responseSetPreferredNetworkType(const sp<RfxMessage>& response) {
    RIL_Errno error = response->getError();

    if (error == RIL_E_SUCCESS) {
        updateState(mRatSettings.prefNwType, RAT_SWITCH_SUCC);
        if(mRatSettings.ratSwitchCaller == RAT_SWITCH_INIT){
            mRatSwitchSignal.emit(m_slot_id, RIL_E_SUCCESS);
        }
        if (mRatSettings.action != NULL) {
            mRatSettings.action->act();
        }
        if (mRatSettings.message != NULL) {
            sp<RfxMessage> resToRilj = RfxMessage::obtainResponse(error, mRatSettings.message, false);
            responseToRilj(resToRilj);
        }
        updatePhoneMode(SWITCH_RAT);
        // logD(RAT_CTRL_TAG, "%s switch prefNwType: %d success!",
        //        switchCallerToString(mRatSettings.ratSwitchCaller), mRatSettings.prefNwType);
    } else {
        updateState(mRatSettings.prefNwType, RAT_SWITCH_FAIL);
        if (error == RIL_E_OPERATION_NOT_ALLOWED) {
            //  Queue FWK network type and return success when in call
            //  and no more switch rat from FWK.
            if(mRatSettings.ratSwitchCaller == RAT_SWITCH_NORMAL &&
                    mPendingNormalRatSwitchRecord.prefNwType == -1) {
                if (mRatSettings.message != NULL) {
                    sp<RfxMessage> resToRilj = RfxMessage::obtainResponse(RIL_E_SUCCESS, mRatSettings.message);
                    responseToRilj(resToRilj);
                    mRatSettings.message = NULL;
                }
                queueRatSwitchRecord(mRatSettings.prefNwType, RAT_SWITCH_NORMAL, NULL, NULL);
                logD(RAT_CTRL_TAG, "Queue network type: %d in call", mRatSettings.prefNwType);
            }
        }

        if(mRatSettings.ratSwitchCaller == RAT_SWITCH_INIT) {
            mRatSwitchSignal.emit(m_slot_id, error);
        }
        if (mRatSettings.action != NULL) {
            mRatSettings.action->act();
        }
        if (mRatSettings.message != NULL) {
            sp<RfxMessage> resToRilj = RfxMessage::obtainResponse(error, mRatSettings.message, false);
            responseToRilj(resToRilj);
        }
        logD(RAT_CTRL_TAG, "%s switch prefNwType: %d fail!",
                switchCallerToString(mRatSettings.ratSwitchCaller), mRatSettings.prefNwType);
    }
    processPendingRatSwitchRecord();
}

void RtcRatSwitchController::responseGetPreferredNetworkType(const sp<RfxMessage>& response) {
    responseToRilj(response);
}

void RtcRatSwitchController::responseGetVoiceRadioTech(const sp<RfxMessage>& response) {
    responseToRilj(response);
}

void RtcRatSwitchController::updatePhoneMode(PHONE_CHANGE_SOURCE source) {
    RFX_UNUSED(source);
    int tech = RADIO_TECH_UNKNOWN;

    switch (mCurPreferedNetWorkType) {
        case PREF_NET_TYPE_GSM_ONLY:
        case PREF_NET_TYPE_GSM_WCDMA:
        case PREF_NET_TYPE_GSM_WCDMA_AUTO:
        case PREF_NET_TYPE_LTE_GSM_WCDMA:
        case PREF_NET_TYPE_LTE_WCDMA:
        case PREF_NET_TYPE_LTE_GSM:
        case PREF_NET_TYPE_TD_SCDMA_GSM_LTE:
        case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_LTE:
        case PREF_NET_TYPE_TD_SCDMA_GSM:
        case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA:
        case PREF_NET_TYPE_TD_SCDMA_LTE:
        case PREF_NET_TYPE_TD_SCDMA_WCDMA_LTE:
            tech = RADIO_TECH_GPRS;
            break;

        case PREF_NET_TYPE_WCDMA:
        case PREF_NET_TYPE_TD_SCDMA_ONLY:
        case PREF_NET_TYPE_TD_SCDMA_WCDMA:
            tech = RADIO_TECH_UMTS;
            break;

        //  LTE, don't change in C2K card.
        case PREF_NET_TYPE_LTE_ONLY:
            if (mNwsMode == NWS_MODE_CSFB || true == isGsmOnlySim() || needDisableCdmaRat()
                    || isSlaveInDualCDMACard()) {
                tech = RADIO_TECH_GPRS;
            }
            break;

        case PREF_NET_TYPE_CDMA_ONLY:
        case PREF_NET_TYPE_CDMA_EVDO_AUTO:
        case PREF_NET_TYPE_EVDO_ONLY:
            tech = RADIO_TECH_1xRTT;
            break;

        case PREF_NET_TYPE_NR_ONLY:
        case PREF_NET_TYPE_NR_LTE:
        case PREF_NET_TYPE_NR_LTE_GSM_WCDMA:
        case PREF_NET_TYPE_NR_LTE_WCDMA:
        case PREF_NET_TYPE_NR_LTE_TDSCDMA:
        case PREF_NET_TYPE_NR_LTE_TDSCDMA_GSM:
        case PREF_NET_TYPE_NR_LTE_TDSCDMA_GSM_WCDMA:
        case PREF_NET_TYPE_NR_LTE_TDSCDMA_WCDMA:
            tech = RADIO_TECH_NR;
            break;

        case PREF_NET_TYPE_NR_LTE_CDMA_EVDO:
        case PREF_NET_TYPE_NR_LTE_CDMA_EVDO_GSM_WCDMA:
        case PREF_NET_TYPE_NR_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA:
            tech = RADIO_TECH_1xRTT;
            if (mNwsMode == NWS_MODE_CSFB) {
                tech = RADIO_TECH_NR;
            }
            break;

        case PREF_NET_TYPE_TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA:
        case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO:
        case PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO:
        case PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA:
        case PREF_NET_TYPE_CDMA_GSM:
        case PREF_NET_TYPE_CDMA_EVDO_GSM:
        case PREF_NET_TYPE_LTE_CDMA_EVDO_GSM:
        case PREF_NET_TYPE_LTE_CDMA_EVDO:
            tech = RADIO_TECH_1xRTT;
            if (mNwsMode == NWS_MODE_CSFB) {
                tech = RADIO_TECH_GPRS;
            }
            break;

        default:
            logW(RAT_CTRL_TAG, "[updatePhoneType] unknown Nw type: %d", mCurPreferedNetWorkType);
            break;
    }

    if (tech != RADIO_TECH_UNKNOWN && mPhoneMode != tech) {
        sp<RfxMessage> urcToRilj;
        mPhoneMode = tech;
        urcToRilj = RfxMessage::obtainUrc(m_slot_id, RFX_MSG_URC_VOICE_RADIO_TECH_CHANGED,
                RfxIntsData(&mPhoneMode, 1));
        responseToRilj(urcToRilj);
        logD(RAT_CTRL_TAG, "[updatePhoneMode] mPhoneMode: %d", mPhoneMode);
    }
}

bool RtcRatSwitchController::isSlaveInDualCDMACard() {
    // slave card and two CDMA cards and this is CDMA card.
    bool ret = false;
   if (m_slot_id != getMajorSlotId()) {
         if (isCdmaOnlySim()  || isCdmaDualModeSimCard()) {
              // need check the peer SIM is also CDMA SIM
             RtcRatSwitchController *another = (RtcRatSwitchController *) findController(
                             m_slot_id == 0 ? 1 : 0, RFX_OBJ_CLASS_INFO(RtcRatSwitchController));
             if ((another != NULL && another->isCdmaOnlySim())  ||
                 (another != NULL && another->isCdmaDualModeSimCard())) {
                 ret = true;
                 logD(RAT_CTRL_TAG, "%s this is dual CDMA mode.", __FUNCTION__);
                 return ret;
             }
        }
    }
    return ret;
}

bool RtcRatSwitchController::onCheckIfRejectMessage(const sp<RfxMessage>& message,
        bool isModemPowerOff,int radioState) {
    int id = message->getId();
    if (RFX_MSG_REQUEST_SET_PREFERRED_NETWORK_TYPE == id) {
        if (RADIO_STATE_UNAVAILABLE == radioState) {
            RtcWpController* wpController =
                    (RtcWpController *)findController(RFX_OBJ_CLASS_INFO(RtcWpController));
            if (wpController->isWorldModeSwitching()) {
                return false;
            } else {
                logD(RAT_CTRL_TAG, "onCheckIfRejectMessage:id=%d,isModemPowerOff=%d,rdioState=%d",
                        message->getId(), isModemPowerOff, radioState);
                return true;
            }
        }
        return false;
    }
    if (RFX_MSG_REQUEST_GET_PREFERRED_NETWORK_TYPE == id
            || RFX_MSG_REQUEST_VOICE_RADIO_TECH == id) {
        if (RADIO_STATE_UNAVAILABLE == radioState) {
            return true;
        }
        return false;
    }
    return RfxController::onCheckIfRejectMessage(message, isModemPowerOff, radioState);
}

bool RtcRatSwitchController::onPreviewMessage(const sp<RfxMessage>& message) {
    if (sIsInSwitching == true &&
            message->getId() == RFX_MSG_REQUEST_VOICE_RADIO_TECH) {
        logD(RAT_CTRL_TAG, "onPreviewMessage, put %s into pending list",
                RFX_ID_TO_STR(message->getId()));
        return false;
    } else {
        return true;
    }
}

bool RtcRatSwitchController::onCheckIfResumeMessage(const sp<RfxMessage>& message) {
    if (sIsInSwitching == false) {
        logD(RAT_CTRL_TAG, "resume the request %s",
                RFX_ID_TO_STR(message->getId()));
        return true;
    } else {
        return false;
    }
}

void RtcRatSwitchController::onApVoiceCallCountChanged(int slotId, RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    int oldMode = old_value.asInt();
    int mode = value.asInt();

    logD(RAT_CTRL_TAG, "%s, slotId:%d, key:%d oldMode:%d, mode:%d",
            __FUNCTION__, slotId, key, oldMode, mode);
    if (mode == 1 && oldMode == 0 &&
            isNetworkScanOngoing() != -1){
        sp<RfxMessage> reqToRild = RfxMessage::obtainRequest(isNetworkScanOngoing(),
                RFX_MSG_REQUEST_LOCAL_ABORT_AVAILABLE_NETWORK, RfxVoidData());
        requestToMcl(reqToRild);
    } else if (mode == 0 && oldMode > 0) {
        if (mRestrictedCount == 0 && isAPInCall() == false &&
                (mPendingInitRatSwitchRecord.prefNwType != -1 ||
                mPendingNormalRatSwitchRecord.prefNwType != -1) &&
                mInRestrictedMode == false) {
            logV(RAT_CTRL_TAG, "%s, slotId:%d, key:%d oldMode:%d, mode:%d",
                    __FUNCTION__, slotId, key, oldMode, mode);
            processPendingRatSwitchRecord();
        }
    }
}

void RtcRatSwitchController::onImsRegistrationStatusChanged(RfxStatusKeyEnum key,
    RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    int oldMode = old_value.asInt();
    int mode = value.asInt();

    if (oldMode != mode && isNetworkTypeContainLteCdma()) {
        sp<RfxMessage> message = RfxMessage::obtainRequest(m_slot_id,
            RFX_MSG_REQUEST_GET_GMSS_RAT_MODE, RfxVoidData());
        mTokensGetEgmssForHvolte.push_back(message->getToken());
        requestToMcl(message);
    }
}

bool RtcRatSwitchController::isAPInCall() {
    bool ret = false;

    for (int slotId = RFX_SLOT_ID_0; slotId < RFX_SLOT_COUNT; slotId++) {
        if (getStatusManager(slotId)->getIntValue(RFX_STATUS_KEY_AP_VOICE_CALL_COUNT, 0) > 0) {
            ret = true;
        }
    }
    return ret;
}

int RtcRatSwitchController::isNetworkScanOngoing() {
    for (int slotId = RFX_SLOT_ID_0; slotId < RFX_SLOT_COUNT; slotId++) {
        if (getStatusManager(slotId)->getBoolValue(RFX_STATUS_KEY_NETWORK_SCAN_ONGOING) == true) {
            return slotId;
        }
    }
    return -1;
}

bool RtcRatSwitchController::isNoSimInserted() {
    for (int i = 0; i < RFX_SLOT_COUNT; i++) {
        int cardType = getStatusManager(i)->getIntValue(
                RFX_STATUS_KEY_CARD_TYPE, 0);
        logD(RAT_CTRL_TAG, "[isNoSimInserted] SIM%d cardType: %d", i, cardType);
        if (cardType > 0) {
            return false;
        }
    }
    logD(RAT_CTRL_TAG, "[isNoSimInserted] No sim inserted");
    return true;
}

bool RtcRatSwitchController::isGsmOnlySimFromMode(RatSwitchInfo ratSwtichInfo) {
    bool ret = false;
    if ((ratSwtichInfo.card_type == RFX_CARD_TYPE_SIM
            || ratSwtichInfo.card_type == RFX_CARD_TYPE_USIM
            || ratSwtichInfo.card_type == (RFX_CARD_TYPE_SIM | RFX_CARD_TYPE_ISIM)
            || ratSwtichInfo.card_type == (RFX_CARD_TYPE_USIM | RFX_CARD_TYPE_ISIM))
            && (!ratSwtichInfo.isCt3GDualMode)) {
        ret = true;
    }

    // logD(RAT_CTRL_TAG, "[isGsmOnlySimFromMode] GSM only: %s", ret ? "true" : "false");
    return ret;
}

bool RtcRatSwitchController::isGsmOnlySim() {
    bool ret = false;
    int nCardType = getStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE);

    if ((nCardType == RFX_CARD_TYPE_SIM
            || nCardType == RFX_CARD_TYPE_USIM
            || nCardType == (RFX_CARD_TYPE_SIM | RFX_CARD_TYPE_ISIM)
            || nCardType == (RFX_CARD_TYPE_USIM | RFX_CARD_TYPE_ISIM))
            && (!isCdmaDualModeSimCard())) {
        ret = true;
    }

    // logD(RAT_CTRL_TAG, "[isGsmOnlySim] GSM only: %s", ret ? "true" : "false");
    return ret;
}

bool RtcRatSwitchController::isCdmaOnlySim() {
    bool ret = false;
    int nCardType = getStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE);
    if (getStatusManager()->getBoolValue(RFX_STATUS_KEY_CDMA3G_DUALMODE_CARD)) {
        logD(RAT_CTRL_TAG, "[isCdmaOnlySim] is C2K 3G dual mode card");
        return false;
    }
    if (nCardType == RFX_CARD_TYPE_RUIM) {
        ret = true;
    }
    logD(RAT_CTRL_TAG, "[isCdmaOnlySim] CardType:%d", nCardType);
    return ret;
}

bool RtcRatSwitchController::isCdmaDualModeSimCard() {
    if (getStatusManager()->getBoolValue(RFX_STATUS_KEY_CDMA3G_DUALMODE_CARD)) {
        logD(RAT_CTRL_TAG, "isCdmaDualModeSimCard, is CT3G dual mode card");
        return true;
    } else {
        int nCardType = getStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE);
        bool ret = false;
        if (RFX_FLAG_HAS_ALL(nCardType, (RFX_CARD_TYPE_USIM | RFX_CARD_TYPE_CSIM)) ||
                RFX_FLAG_HAS_ALL(nCardType, (RFX_CARD_TYPE_USIM | RFX_CARD_TYPE_RUIM))) {
            logD(RAT_CTRL_TAG, "isCdmaDualModeSimCard, nCardType=0x%x, ret = %d", nCardType, ret);
            ret = true;
        }
        return ret;
    }
}

int RtcRatSwitchController::getMajorSlotId() {
    char tempstr[RFX_PROPERTY_VALUE_MAX];
    memset(tempstr, 0, sizeof(tempstr));
    rfx_property_get("persist.vendor.radio.simswitch", tempstr, "1");
    int majorSlotId = atoi(tempstr) - 1;
    // logD(RAT_CTRL_TAG, "[getMajorSlotId] %d", majorSlotId);
    return majorSlotId;
}

int RtcRatSwitchController::getCCapabilitySlotId() {
    char property_value[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get("persist.vendor.radio.c_capability_slot", property_value, "1");
    int cCapabilitySlot = atoi(property_value) - 1;
    return cCapabilitySlot;
}

void RtcRatSwitchController::setSlotCapability(int slotId, int val) {
    char PROPERTY_RILJ_NW_RAF[4][30] = {
        "persist.vendor.radio.raf1",
        "persist.vendor.radio.raf2",
        "persist.vendor.radio.raf3",
        "persist.vendor.radio.raf4",
    };
    setIntSysProp(PROPERTY_RILJ_NW_RAF[slotId], val);
}

int RtcRatSwitchController::getSlotCapability(int slotId) {
    char PROPERTY_RILJ_NW_RAF[4][30] = {
        "persist.vendor.radio.raf1",
        "persist.vendor.radio.raf2",
        "persist.vendor.radio.raf3",
        "persist.vendor.radio.raf4",
    };
    return getIntSysProp(PROPERTY_RILJ_NW_RAF[slotId], -1);
}


int RtcRatSwitchController::getDefaultNetWorkType(int slotId) {
    char stgBuf[RFX_PROPERTY_VALUE_MAX] = { 0 };
    getMSimProperty(slotId, (char*)"ro.telephony.default_network", stgBuf);
    int type = atoi(stgBuf);
    logV(RAT_CTRL_TAG, "getDefaultNetWorkType:: type :%d", type);
    return type;
}

void RtcRatSwitchController::setPreferredNetWorkTypeToSysProp(int slotId, int val) {
    char PROPERTY_RILJ_NW_TYPE[4][40] = {
        "persist.vendor.radio.rilj_nw_type1",
        "persist.vendor.radio.rilj_nw_type2",
        "persist.vendor.radio.rilj_nw_type3",
        "persist.vendor.radio.rilj_nw_type4",
    };
    if (getPreferredNetWorkTypeFromSysProp(slotId) != val) {
        setIntSysProp(PROPERTY_RILJ_NW_TYPE[slotId], val);
        // setNwTypeCacheByIndex(mNetworkTypeIndex, val);
    }
}

int RtcRatSwitchController::getPreferredNetWorkTypeFromSysProp(int slotId) {
    char PROPERTY_RILJ_NW_TYPE[4][40] = {
        "persist.vendor.radio.rilj_nw_type1",
        "persist.vendor.radio.rilj_nw_type2",
        "persist.vendor.radio.rilj_nw_type3",
        "persist.vendor.radio.rilj_nw_type4",
    };
    return getIntSysProp(PROPERTY_RILJ_NW_TYPE[slotId], getDefaultNetWorkType(slotId));
}

void RtcRatSwitchController::setIntSysProp(char *propName, int val) {
    char stgBuf[RFX_PROPERTY_VALUE_MAX] = { 0 };
    sprintf(stgBuf, "%d", val);
    rfx_property_set(propName, stgBuf);
}

int RtcRatSwitchController::getIntSysProp(char *propName, int defaultVal) {
    int val = -1;
    char stgBuf[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get(propName, stgBuf, "-1");
    val = strtol(stgBuf, NULL, 10);
    if (val == -1) {
        // logD(RAT_CTRL_TAG, "[getIntSysProp] %s not exist, return %d",
        //         propName, defaultVal);
        val = defaultVal;
    }
    return val;
}

int RtcRatSwitchController::getIccIdCacheIndex(int slotId) {
    char cache[RFX_PROPERTY_VALUE_MAX] = {0};
    char iccid[RFX_PROPERTY_VALUE_MAX] = {0};
    rfx_property_get(PROPERTY_ICCID_SIM[slotId], iccid, "");
    char stgBuf[RFX_PROPERTY_VALUE_MAX] = { 0 };
    getMSimProperty(m_slot_id, (char*)"ro.telephony.default_network", stgBuf);

    if (strlen(iccid) == 0 || strcmp(iccid, "N/A") == 0) {
        logV(RAT_CTRL_TAG, "getIccIdCacheIndex:: No SIM insert or not ready!");
        return -1;
    }
    for (int i = 0; i < CACHE_ICCID_SIZE; i++) {
        getMSimProperty(i, (char*)PROPERTY_CHCHE_ICCID, cache);
        logV(RAT_CTRL_TAG, "getIccIdCacheIndex::index:%d", i);
        if (strcmp(cache, iccid) == 0) {
            return i;
        } else if (strlen(cache) == 0) {
            setMSimProperty(i, (char *)PROPERTY_CHCHE_ICCID, iccid);
            setMSimProperty(i, (char *)PROPERTY_CHCHE_NW_TYPE, stgBuf);
            return i;
        }
        memset(&cache, 0, RFX_PROPERTY_VALUE_MAX);
    }
    return -1;
}

int RtcRatSwitchController::getNwTypeCacheByIndex(int index) {
    if (index != -1) {
        char cache[RFX_PROPERTY_VALUE_MAX] = {0};
        getMSimProperty(index, (char*)PROPERTY_CHCHE_NW_TYPE, cache);
        if (strlen(cache) > 0) {
            mNetworkTypeCache = strtol(cache, NULL, 10);
            logV(RAT_CTRL_TAG, "getNwTypeCacheByIndex:: mNetworkTypeCache:%d", mNetworkTypeCache);
            return mNetworkTypeCache;
        }
    }
    return -1;
}

void RtcRatSwitchController::setNwTypeCacheByIndex(int index, int type) {
    logV(RAT_CTRL_TAG, "setNwTypeCacheByIndex::index:%d type:%d cache_nwtype:%d",index, type, mNetworkTypeCache);
    if (index != -1 && type != -1 && type != mNetworkTypeCache) {
        char* nw_type = NULL;
        asprintf(&nw_type, "%d", type);
        setMSimProperty(index, (char *)PROPERTY_CHCHE_NW_TYPE, nw_type);
        mNetworkTypeCache = type;
        free(nw_type);
    }
}

int RtcRatSwitchController::isNewSimCard(int slotId) {
    int result = -1;
    char lastTimeIccid[RFX_PROPERTY_VALUE_MAX] = {0};
    char currentIccid[RFX_PROPERTY_VALUE_MAX] = {0};

    char PROPERTY_LAST_BOOT_ICCID_SIM[4][40] = {
        "persist.vendor.radio.last_iccid_sim1",
        "persist.vendor.radio.last_iccid_sim2",
        "persist.vendor.radio.last_iccid_sim3",
        "persist.vendor.radio.last_iccid_sim4",
    };

    rfx_property_get(PROPERTY_ICCID_SIM[slotId], currentIccid, "");
    rfx_property_get(PROPERTY_LAST_BOOT_ICCID_SIM[slotId], lastTimeIccid, "");
    if (strlen(lastTimeIccid) == 0 || strcmp("N/A", lastTimeIccid) == 0) {
        logD(RAT_CTRL_TAG, "[isNewSimCard]:first time boot-up");
        rfx_property_set(PROPERTY_LAST_BOOT_ICCID_SIM[slotId], currentIccid);
        result = 1;
    } else {
        if (strlen(currentIccid) == 0 || sIsInSwitching == true) {
            logD(RAT_CTRL_TAG, "[isNewSimCard]:Sim card is not ready or in switcing");
            result = 0;
        } else if (strcmp(lastTimeIccid, currentIccid) == 0) {
            // logD(RAT_CTRL_TAG, "[isNewSimCard]:Sim card is no change");
            result = 0;
        } else if (strcmp("N/A", currentIccid) == 0) {
            logV(RAT_CTRL_TAG, "[isNewSimCard]:Sim card is not insert");
            result = -1;
        } else {
            logD(RAT_CTRL_TAG, "[isNewSimCard]:SIM change, NetworkType in cache:%d",
                    mNetworkTypeCache);
            rfx_property_set(PROPERTY_LAST_BOOT_ICCID_SIM[slotId], currentIccid);
            if (mNetworkTypeCache != -1) {
                result = 0;
            } else {
                result = 1;
            }
        }
    }
    // logD(RAT_CTRL_TAG, "[isNewSimCard] return:%d", result);
    return result;
}

int RtcRatSwitchController::isRafContainsCdma(int raf) {
    int result = -1;
    if (((raf & RAF_EVDO_GROUP) > 0) || ((raf & RAF_CDMA_GROUP) > 0)) {
        result = 1;
    } else {
        result = 0;
    }
    return result;
}

int RtcRatSwitchController::isRafContainsGsm(int raf) {
    int result = -1;
    if (((raf & RAF_GSM_GROUP) > 0) || ((raf & RAF_HS_GROUP) > 0)
            || ((raf & RAF_WCDMA_GROUP) > 0) || ((raf & RAF_LTE) > 0)) {
        result = 1;
    } else {
        result = 0;
    }
    return result;
}

PsRatFamily RtcRatSwitchController::getPsRatFamily(int radioTechnology) {
    if (radioTechnology == RADIO_TECH_GPRS
            || radioTechnology == RADIO_TECH_EDGE
            || radioTechnology == RADIO_TECH_UMTS
            || radioTechnology == RADIO_TECH_HSDPA
            || radioTechnology == RADIO_TECH_HSUPA
            || radioTechnology == RADIO_TECH_HSPA
            || radioTechnology == RADIO_TECH_LTE
            || radioTechnology == RADIO_TECH_HSPAP
            || radioTechnology == RADIO_TECH_GSM
            || radioTechnology == RADIO_TECH_TD_SCDMA
            || radioTechnology == RADIO_TECH_LTE_CA) {
        return PS_RAT_FAMILY_GSM;
    } else if (radioTechnology == RADIO_TECH_IS95A
            || radioTechnology == RADIO_TECH_IS95B
            || radioTechnology == RADIO_TECH_1xRTT
            || radioTechnology == RADIO_TECH_EVDO_0
            || radioTechnology == RADIO_TECH_EVDO_A
            || radioTechnology == RADIO_TECH_EVDO_B
            || radioTechnology == RADIO_TECH_EHRPD) {
        return PS_RAT_FAMILY_CDMA;
    } else if (radioTechnology == RADIO_TECH_IWLAN) {
        return PS_RAT_FAMILY_IWLAN;
    } else {
        return PS_RAT_FAMILY_UNKNOWN;
    }
}

const char *RtcRatSwitchController::switchCallerToString(int callerEnum) {
    switch (callerEnum) {
        case RAT_SWITCH_INIT:
            return "INIT";
        case RAT_SWITCH_NORMAL:
            return "NOR";
        case RAT_SWITCH_RESTRICT:
            return "RES";
        default:
            logW(RAT_CTRL_TAG, "<UNKNOWN> %d", callerEnum);
            break;
    }
    return "";
}

int RtcRatSwitchController::filterPrefNwTypeFromRilj(const int prefNwTypeFromRilj) {
    int targetPrefNwType = -1;
    if (mDefaultNetworkType != -1) {
        int slot_capability = RtcCapabilitySwitchUtil::getRafFromNetworkType(mDefaultNetworkType);
        int rafFromType = RtcCapabilitySwitchUtil::getRafFromNetworkType(prefNwTypeFromRilj);
        int filteredRaf = (slot_capability & rafFromType);

        if (isGsmOnlySim() == true || needDisableCdmaRat()) {
            filteredRaf &= ~(RAF_CDMA_GROUP | RAF_EVDO_GROUP);
            if (filteredRaf != 0) {
                targetPrefNwType = RtcCapabilitySwitchUtil::getNetworkTypeFromRaf(filteredRaf);
            } else {
                targetPrefNwType = mDefaultNetworkType;
            }
        } else if (filteredRaf != 0) {
            targetPrefNwType = RtcCapabilitySwitchUtil::getNetworkTypeFromRaf(filteredRaf);
        } else {
            targetPrefNwType = mDefaultNetworkType;
        }
    } else {
        if (isGsmOnlySim() == true || needDisableCdmaRat()
                || getCCapabilitySlotId() != m_slot_id) {
            int rafFromType = RtcCapabilitySwitchUtil::getRafFromNetworkType(prefNwTypeFromRilj);
            rafFromType &= ~(RAF_CDMA_GROUP | RAF_EVDO_GROUP);
            targetPrefNwType = RtcCapabilitySwitchUtil::getNetworkTypeFromRaf(rafFromType);
        } else {
            targetPrefNwType = prefNwTypeFromRilj;
        }
    }
    return targetPrefNwType;
}

void RtcRatSwitchController::responseGetGmssRatMode(const sp<RfxMessage>& response) {
    handleGmssRat(response);
    Vector<int>::iterator iter = std::find(mTokensGetEgmssForHvolte.begin(),
            mTokensGetEgmssForHvolte.end(), response->getToken());
    if ((iter != mTokensGetEgmssForHvolte.end())) {
        updatePhoneMode(GMSS_RAT);
        mTokensGetEgmssForHvolte.erase(iter);
    } else {
        configRatMode();
    }
}

void RtcRatSwitchController::getGmssRatMode() {
    sp<RfxMessage> reqToRild = RfxMessage::obtainRequest(m_slot_id,
            RFX_MSG_REQUEST_GET_GMSS_RAT_MODE, RfxVoidData());
    requestToMcl(reqToRild);
}

void RtcRatSwitchController::handleGmssRat(const sp<RfxMessage>& message) {
    if (message->getType() == RESPONSE && message->getError() != RIL_E_SUCCESS) {
        logD(RAT_CTRL_TAG, "[handleGmssRat] error: %d ", message->getError());
        return;
    }

    RfxIntsData *intsData = (RfxIntsData*)message->getData();
    int *data = (int*)intsData->getData();
    GmssInfo gmssInfo;
    gmssInfo.rat = data[0];
    gmssInfo.mcc = data[1];
    gmssInfo.status = data[2];
    gmssInfo.mspl = data[3];
    gmssInfo.ishome = data[4];
    int volteRegistration = 0;

    if (gmssInfo.rat == GMSS_RAT_INVALID
            && gmssInfo.mcc == 0
            && gmssInfo.status == GMSS_STATUS_SELECT
            && gmssInfo.mspl == MSPL_RAT_NONE
            && gmssInfo.ishome == false) {
        // logD(RAT_CTRL_TAG, "[handleGmssRat] invalid value");
        return;
    }

    if (gmssInfo.status == GMSS_STATUS_ECC) {
        if (gmssInfo.rat == GMSS_RAT_3GPP2 ||
                gmssInfo.rat == GMSS_RAT_C2K1X ||
                gmssInfo.rat == GMSS_RAT_C2KHRPD) {
            mNwsMode = NWS_MODE_CDMALTE;
        } else if (gmssInfo.rat == GMSS_RAT_GSM ||
                gmssInfo.rat == GMSS_RAT_WCDMA) {
            mNwsMode = NWS_MODE_CSFB;
        } else {
            if (gmssInfo.ishome == false ||
                    isCdmaDualModeSimCard() == false) {
                mNwsMode = NWS_MODE_CSFB;
            } else {
                mNwsMode = NWS_MODE_CDMALTE;
            }
        }
    } else {
        if (RFX_FLAG_HAS(gmssInfo.mspl, MSPL_RAT_C2K) &&
                (RFX_FLAG_HAS(gmssInfo.mspl, MSPL_RAT_GSM)
                || RFX_FLAG_HAS(gmssInfo.mspl, MSPL_RAT_UMTS))) {
            if (gmssInfo.status == GMSS_STATUS_ATTACHED) {
                if (GMSS_RAT_3GPP2 == gmssInfo.rat
                        || GMSS_RAT_C2K1X == gmssInfo.rat
                        || GMSS_RAT_C2KHRPD == gmssInfo.rat) {
                    mNwsMode = NWS_MODE_CDMALTE;
                } else if (GMSS_RAT_GSM == gmssInfo.rat
                        || GMSS_RAT_WCDMA == gmssInfo.rat) {
                    mNwsMode = NWS_MODE_CSFB;
                } else {
                    // EUTRAN (LTE)
                    mNwsMode = gmssInfo.ishome ? NWS_MODE_CDMALTE : NWS_MODE_CSFB;
                }
            }
        } else if (RFX_FLAG_HAS(gmssInfo.mspl, MSPL_RAT_C2K)) {
#ifdef MTK_TC1_COMMON_SERVICE
            mNwsMode = NWS_MODE_CDMALTE;
#else
            if (isHvolteSupport() && isNetworkTypeContainLteCdma()) {
                volteRegistration = getStatusManager()->getIntValue(
                    RFX_STATUS_KEY_IMS_REGISTRATION_STATE, 0);
                if ((volteRegistration & VOLTE_REGISTERATION_STATE)
                    == VOLTE_REGISTERATION_STATE) {
                    mNwsMode = NWS_MODE_CSFB;
                } else {
                    mNwsMode = NWS_MODE_CDMALTE;
                }
            } else {
                mNwsMode = NWS_MODE_CDMALTE;
            }
#endif
        } else {
            mNwsMode = NWS_MODE_CSFB;
        }
    }
    getStatusManager()->setIntValue(RFX_STATUS_KEY_NWS_MODE, mNwsMode);
    logD(RAT_CTRL_TAG, "[handleGmssRat] rat:%d, mcc:%d, status:%d, mspl:%d, ishome:%d, NWS mode:%d,"
        "volteRegistration: %d.", gmssInfo.rat, gmssInfo.mcc, gmssInfo.status, gmssInfo.mspl,
        gmssInfo.ishome, mNwsMode, volteRegistration);
}

bool RtcRatSwitchController::isNetworkTypeContainLteCdma() {
    bool ret = false;

    switch (mCurPreferedNetWorkType) {
        case PREF_NET_TYPE_LTE_CDMA_EVDO:
        case PREF_NET_TYPE_TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA:
        case PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA:
        case PREF_NET_TYPE_LTE_CDMA_EVDO_GSM:
            ret = true;
            break;

        default:
            break;
    }
    return ret;
}

bool RtcRatSwitchController::isCT4GCard() {
    int nCardType = getStatusManager()->getIntValue(RFX_STATUS_KEY_CDMA_CARD_TYPE);
    return CT_4G_UICC_CARD == nCardType ? true : false;
}

void RtcRatSwitchController::onVolteStateChanged(RfxStatusKeyEnum key, RfxVariant old_value,
        RfxVariant value) {
    RFX_UNUSED(key);
    int old_mode = old_value.asInt();
    int new_mode = value.asInt();

    if (old_mode != new_mode && new_mode == 1 && isNetworkTypeContainLteCdma()
             && isCT4GCard()) {
        // volte status is changed and turn on the volte
        mNwsMode = NWS_MODE_CSFB;
        getStatusManager()->setIntValue(RFX_STATUS_KEY_NWS_MODE, mNwsMode);
        updatePhoneMode(GMSS_RAT);
    }
}

void RtcRatSwitchController::onUiccMccMncChanged(RfxStatusKeyEnum key, RfxVariant old_value,
        RfxVariant value) {
    RFX_UNUSED(key);
    const String8 old_mccmnc = old_value.asString8();
    const String8 new_mccmnc = value.asString8();

    // Set preferred network type if needDisableCdmaRat when MCC/MNC changed.
    if (new_mccmnc.compare(old_mccmnc) != 0 && needDisableCdmaRat()) {
        const int fromRiljNetworkType = getPreferredNetWorkTypeFromSysProp(m_slot_id);
        if (fromRiljNetworkType != -1) {
            switchNwRat(fromRiljNetworkType, RAT_SWITCH_NORMAL, NULL, NULL);
        }
    }
}

bool RtcRatSwitchController::needDisableCdmaRat() {
    String8 mccmnc_gsm(getStatusManager()->getString8Value(RFX_STATUS_KEY_UICC_GSM_NUMERIC));
    String8 mccmnc_cdma(getStatusManager()->getString8Value(RFX_STATUS_KEY_UICC_CDMA_NUMERIC));
    return (!mccmnc_gsm.isEmpty() && needDisableCdmaRatByMccMnc((const char*) mccmnc_gsm.string()))
            || (!mccmnc_cdma.isEmpty()
                    && needDisableCdmaRatByMccMnc((const char*) mccmnc_cdma.string()));
}

bool RtcRatSwitchController::needDisableCdmaRatByMccMnc(const char *mccmnc) {
    bool needDisableCdmaRat = false;
    // Compare with mccmnc for legacy SIM card disable cdma rat
    for (size_t i = 0; i < NUM_ELEMS(DISABLE_CDMA_RAT_MCCMNC); i++) {
        if (RfxMisc::strStartsWith(mccmnc, DISABLE_CDMA_RAT_MCCMNC[i])) {
            needDisableCdmaRat = true;
            break;
        }
    }
    return needDisableCdmaRat;
}

bool RtcRatSwitchController::isHvolteSupport() {
    char tempstr[RFX_PROPERTY_VALUE_MAX];
    memset(tempstr, 0, sizeof(tempstr));
    rfx_property_get("persist.vendor.mtk_ct_volte_support", tempstr, "0");
    return strcmp(tempstr, "3") == 0 ? true : false;
}

bool RtcRatSwitchController::isECBMRestrictedModeSupport() {
    bool isECBMSupport = false;
    char optr[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get(PROPERTY_OPERATOR, optr, "");
#ifdef MTK_TC1_COMMON_SERVICE
    isECBMSupport = true;
#endif
    if (strcmp(optr, OPERATOR_OP12) == 0 || strcmp(optr, OPERATOR_OP20) == 0) {
        isECBMSupport = true;
    }
    return isECBMSupport;
}

void RtcRatSwitchController::onRestrictedModeChanged(RfxStatusKeyEnum key, RfxVariant old_value,
        RfxVariant value) {
    int oldMode;
    int mode;
    if (key == RFX_STATUS_KEY_EMERGENCY_MODE) {
        oldMode = (int) old_value.asBool();
        mode = (int) value.asBool();
    } else {
        oldMode = old_value.asInt();
        mode = value.asInt();
    }

    if (mode > 0 && oldMode == 0) {
        mRestrictedCount++;
    } else if (mode == 0 && oldMode > 0) {
        mRestrictedCount--;
    }

    logD(RAT_CTRL_TAG, "[onRestrictedModeChanged] key:%d oldMode:%d, mode:%d, Restricted Count:%d",
            key, oldMode, mode, mRestrictedCount);

    if (mode == 0 && oldMode > 0) {
        if (mRestrictedCount == 0 && isAPInCall() == false) {
            logD(RAT_CTRL_TAG, "[onRestrictedModeChanged] do pending record in queue");
            processPendingRatSwitchRecord();
        }
    }
}
