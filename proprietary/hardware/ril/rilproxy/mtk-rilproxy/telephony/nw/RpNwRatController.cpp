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
#include "RpNwRatController.h"
#include "RfxLog.h"
#include "RfxStatusDefs.h"
#include "RpNwController.h"
#include "RpGsmNwRatSwitchHandler.h"
#include "RpCdmaNwRatSwitchHandler.h"
#include "RpCdmaLteNwRatSwitchHandler.h"
#include "modecontroller/RpCdmaLteDefs.h"
#include "util/RpFeatureOptionUtils.h"
#include "RpNwWwopRatController.h"
#include <telephony/mtk_ril.h>

#define RAT_CTRL_TAG "RpNwRatController"

/*****************************************************************************
 * Class RpNwRatController
 *****************************************************************************/

RFX_IMPLEMENT_CLASS("RpNwRatController", RpNwRatController, RfxController);

bool RpNwRatController::sIsInSwitching = false;

RpNwRatController::RpNwRatController() :
    mCurAppFamilyType(APP_FAM_UNKNOWN),
    mCurPreferedNetWorkType(-1),
    mPreferredNetWorkTypeFromRILJ(-1),
    mCurNwsMode(NWS_MODE_CSFB),
    mNewAppFamilyType(APP_FAM_UNKNOWN),
    mNewPreferedNetWorkType(-1),
    mNewNwsMode(NWS_MODE_CSFB),
    mCapabilitySlotId(0),
    mDefaultNetworkType(-1),
    mCardType(CARD_TYPE_INVALID),
    mCurVolteState(VOLTE_OFF),
    mNewVolteState(VOLTE_OFF),
    mNwRatSwitchHandler(NULL),
    mNwRatController(NULL) {
}

RpNwRatController::~RpNwRatController() {
    RFX_OBJ_CLOSE(mNwRatSwitchHandler);
}

void RpNwRatController::setPreferredNetworkType(const int prefNwType,
        const sp<RfxAction>& action) {
    if (isWwop()) {
        mNwRatController->setRestrictedNetworkMode(prefNwType, action);
        return;
    }
    if (getEnginenerMode() == ENGINEER_MODE_AUTO) {
        if (prefNwType == -1) {
            mPendingRestrictedRatSwitchRecord.prefNwType = -1;
            doPendingRatSwitchRecord();
        } else {
            logD(RAT_CTRL_TAG, "setPreferredNetworkType() entering restricted mode: %d", prefNwType);
            mPendingRestrictedRatSwitchRecord.prefNwType = prefNwType;
            mPendingRestrictedRatSwitchRecord.appFamType = APP_FAM_3GPP2;
            mPendingRestrictedRatSwitchRecord.ratSwitchCaller = RAT_SWITCH_RESTRICT;
            mPendingRestrictedRatSwitchRecord.nwsMode = mCurNwsMode;
            mPendingRestrictedRatSwitchRecord.volteState = mCurVolteState;
            if (mPendingInitRatSwitchRecord.prefNwType == -1
                    && mPendingNormalRatSwitchRecord.prefNwType == -1) {
                // save current state to pending queue
                queueRatSwitchRecord(mCurAppFamilyType, mCurPreferedNetWorkType, mCurNwsMode, mCurVolteState,
                        RAT_SWITCH_INIT, action, NULL);
            }
            switchNwRat(APP_FAM_3GPP2, PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA, mCurNwsMode, mCurVolteState,
                    RAT_SWITCH_RESTRICT, action, NULL);
        }
    }
}

void RpNwRatController::setPreferredNetworkType(const RatSwitchInfo ratSwtichInfo) {
    const AppFamilyType appFamType = ratSwtichInfo.app_family_type;
    const int prefNwType = ratSwtichInfo.network_type;
    const int type = ratSwtichInfo.card_state;
    const sp<RfxAction>& action = ratSwtichInfo.action;
    const int ratMode = ratSwtichInfo.rat_mode;
    const int cardType = ratSwtichInfo.card_type;

    if (isWwop()) {
        mNwRatController->setAppFamilyType(appFamType, prefNwType, type, action);
        return;
    }
    logD(RAT_CTRL_TAG, "setPreferredNetworkType() from mode controller, "
            "appFamType is %d, prefNwType is %d, type is %d, ratMode is %d, cardType is %d.",
            appFamType, prefNwType, type, ratMode, cardType);
    int capabilitySlotId = getCapabilitySlotId();
    if (type == CARD_STATE_NO_CHANGED && appFamType == mNewAppFamilyType
            && mCapabilitySlotId == capabilitySlotId) {
        if (action != NULL) {
            action->act();
        }
        return;
    }
    if (type == CARD_STATE_NOT_HOT_PLUG || type == CARD_STATE_HOT_PLUGIN) {
        clearInvalidPendingRecords();
    }
    mCapabilitySlotId = capabilitySlotId;

    int oldAppFamilyType = mNewAppFamilyType;
    mNewAppFamilyType = appFamType;
    mDefaultNetworkType = prefNwType;
    mCardType = cardType;

    getPreferredNetWorkTypeFromRILJ();
    int newPrefNwType = prefNwType;
    if (mPreferredNetWorkTypeFromRILJ != -1
            && mPreferredNetWorkTypeFromRILJ != PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA) {
        if (appFamType == APP_FAM_3GPP2) {
             newPrefNwType = filterPrefNwType(appFamType,
                     mPreferredNetWorkTypeFromRILJ, prefNwType);
         } else {
            //RILJ pref networktype may be TDD only, for 3GPP, should use networktype from
            //mode controller
            if (mPreferredNetWorkTypeFromRILJ == PREF_NET_TYPE_LTE_TDD_ONLY) {
                newPrefNwType = prefNwType;
                int msg_id = RIL_LOCAL_REQUEST_SET_FDD_TDD_MODE;
                sp<RfxMessage> gsmMessage = RfxMessage::obtainRequest(m_slot_id,
                        RADIO_TECH_GROUP_GSM, msg_id);
                gsmMessage->getParcel()->writeInt32(1);
                gsmMessage->getParcel()->writeInt32(SVLTE_FDD_TDD_MODE);
                requestToRild(gsmMessage);
            } else {
                newPrefNwType = mPreferredNetWorkTypeFromRILJ;
            }
         }
         logD(RAT_CTRL_TAG, "mPreferredNetWorkTypeFromRILJ filtered as %d.", newPrefNwType);
    }
    // if hot plug out, should not use RILJ pref network type, use default pref
    //network type from mode controller
    if (type == CARD_STATE_HOT_PLUGOUT) {
        newPrefNwType = prefNwType;
    }
    if (appFamType == APP_FAM_3GPP) {
        ///M: for denali IR settings-> FTA LTE only test
        if (getEnginenerMode() == ENGINEER_MODE_LTE) {
            RpRadioController* radioController =
                        (RpRadioController *) findController(RFX_OBJ_CLASS_INFO(RpRadioController));
            int capability = RIL_CAPABILITY_NONE;
            capability = RIL_CAPABILITY_GSM_ONLY;
            RpSuggestRadioCapabilityCallback callback =
            RpSuggestRadioCapabilityCallback(this,
                    &RpNwRatController::onSuggestRadioCapabilityResult);
            radioController->suggestedCapability(capability, callback);
            if (action != NULL) {
                action->act();
            }
            return;
        }

        // For CT Dual Volte
        NwsMode newNwsMode = NWS_MODE_CSFB;
        VolteState newVolteState = mNewVolteState;

        // For CT Dual Volte C+C Case: CDMA 4G card is in side slot, but AppFamilyType is 3GPP.
        if (is4GNetworkMode(newPrefNwType) && needConfigRatModeForCtDualVolte()) {
            newNwsMode = NWS_MODE_LTEONLY;

            // [VoLTE] filter VolteState by newPrefNwType and ratMode
            newVolteState = filterVolteState(newPrefNwType, newVolteState, ratMode);
            newNwsMode = filterNwsMode(appFamType, newNwsMode, newVolteState);
        }
        /* For gsm card, create the ratSwitchHandler and set the related state*/
        switchNwRat(appFamType, newPrefNwType, newNwsMode, newVolteState, RAT_SWITCH_INIT,
                action, NULL);
    } else {
        if (getEnginenerMode() == ENGINEER_MODE_AUTO) {
            NwsMode newNwsMode = NWS_MODE_CDMALTE;
            VolteState newVolteState = mNewVolteState;
            if (mPendingNormalRatSwitchRecord.prefNwType != -1 && oldAppFamilyType == appFamType) {
                newNwsMode = mPendingNormalRatSwitchRecord.nwsMode;
                newVolteState = mPendingNormalRatSwitchRecord.volteState;
            } else {
                /* For ct 3g dual mode card Home and roaming switch */
                if (type == CARD_STATE_CARD_TYPE_CHANGED && oldAppFamilyType == appFamType) {
                    newNwsMode = mNewNwsMode;
                }
            }

            // [VoLTE] filter VolteState by newPrefNwType and ratMode
            newVolteState = filterVolteState(newPrefNwType, newVolteState, ratMode);
            newNwsMode = filterNwsMode(appFamType, newNwsMode, newVolteState);

            switchNwRat(appFamType, newPrefNwType, newNwsMode, newVolteState,
                        RAT_SWITCH_INIT, action, NULL);
        } else {
            doNwSwitchForEngMode(action);
        }
    }
}

void RpNwRatController::setPreferredNetworkType(const int prefNwType,
        const sp<RfxMessage>& message) {
    int cardType = getStatusManager(m_slot_id)->getIntValue(
            RFX_STATUS_KEY_CARD_TYPE, 0);
    int engineerMode = getEnginenerMode();
    logD(RAT_CTRL_TAG,
            "setPreferredNetworkType() from normal, prefNwType is %d,mNewPreferedNetWorkType is %d, cardType is %d. ",
            prefNwType, mNewPreferedNetWorkType, cardType);
   // there is no sim, so ignore it
    if (cardType == CARD_TYPE_NONE) {
        sp<RfxMessage> resToRilj = RfxMessage::obtainResponse(
                RIL_E_SUCCESS, message);
        responseToRilj(resToRilj);
        return;
    }
    AppFamilyType appFamilyType = mNewAppFamilyType;
    VolteState volteState = mNewVolteState;
    int capability = getDefaultNetworkType();
    if (appFamilyType == APP_FAM_3GPP) {
        ///M: for denali IR settings-> FTA LTE only test
        if (engineerMode == ENGINEER_MODE_LTE) {
            sp<RfxMessage> resToRilj = RfxMessage::obtainResponse(
                RIL_E_GENERIC_FAILURE, message);
            responseToRilj(resToRilj);
            return;
        }
        /* For gsm card, create the ratSwitchHandler and set the related state*/
        if ((capability == PREF_NET_TYPE_GSM_ONLY) &&
                    (prefNwType == PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA ||
                    prefNwType == PREF_NET_TYPE_LTE_CDMA_EVDO)) {
            logD(RAT_CTRL_TAG, "setPreferredNetworkType: 3GPP can not register CDMA.");
            sp<RfxMessage> resToRilj = RfxMessage::obtainResponse(
                    RIL_E_GENERIC_FAILURE, message);
            responseToRilj(resToRilj);
            return;
        }

        // For CT Dual Volte
        NwsMode nwsMode = NWS_MODE_CSFB;

        // For CT Dual Volte C+C Case: CDMA 4G card is in side slot, but AppFamilyType is 3GPP.
        if (is4GNetworkMode(prefNwType) && needConfigRatModeForCtDualVolte()) {
            // Check pending
            nwsMode = mNewNwsMode;
            if (mPendingNormalRatSwitchRecord.prefNwType != -1) {
                nwsMode = mPendingNormalRatSwitchRecord.nwsMode;
                volteState = mPendingNormalRatSwitchRecord.volteState;
                logD(RAT_CTRL_TAG, "Has pending normal: update nwsMode from %d to %d.",
                        mNewNwsMode, nwsMode);
            } else if (mPendingInitRatSwitchRecord.prefNwType != -1) {
                nwsMode = mPendingInitRatSwitchRecord.nwsMode;
                volteState = mPendingInitRatSwitchRecord.volteState;
                logD(RAT_CTRL_TAG, "Has pending init: update nwsMode from %d to %d.",
                        mNewNwsMode, nwsMode);
            }

            // [VoLTE] filter VolteState by newPrefNwType and ratMode
            volteState = filterVolteState(prefNwType, volteState, -1);
            nwsMode = filterNwsMode(appFamilyType, nwsMode, volteState);
        }

        switchNwRat(appFamilyType, prefNwType, nwsMode, volteState, RAT_SWITCH_NORMAL,
                    NULL, message);
    } else if (appFamilyType == APP_FAM_3GPP2) {
        int targetPrefNwType = filterPrefNwType(appFamilyType, prefNwType, capability);
        logD(RAT_CTRL_TAG, "the prefer network type is filtered as %d, capability is %d .",
                      targetPrefNwType, capability);
        if ((engineerMode == ENGINEER_MODE_AUTO)
                || (engineerMode == ENGINEER_MODE_CDMA &&
                        (targetPrefNwType == PREF_NET_TYPE_CDMA_ONLY
                        || targetPrefNwType == PREF_NET_TYPE_EVDO_ONLY
                        || targetPrefNwType == PREF_NET_TYPE_CDMA_EVDO_AUTO))) {
            NwsMode nwsMode = mNewNwsMode;

            if (mPendingNormalRatSwitchRecord.prefNwType != -1) {
                nwsMode = mPendingNormalRatSwitchRecord.nwsMode;
                volteState = mPendingNormalRatSwitchRecord.volteState;
                logD(RAT_CTRL_TAG, "Has pending normal: update nwsMode from %d to %d.",
                        mNewNwsMode, nwsMode);
            } else if (mPendingInitRatSwitchRecord.prefNwType != -1) {
                nwsMode = mPendingInitRatSwitchRecord.nwsMode;
                volteState = mPendingInitRatSwitchRecord.volteState;
                logD(RAT_CTRL_TAG, "Has pending init: update nwsMode from %d to %d.",
                        mNewNwsMode, nwsMode);
            }

            // [VoLTE] filter VolteState by PreferredNetWorkTypeFromRILJ
            VolteState newVolteState = filterVolteState(targetPrefNwType, volteState, -1);
            NwsMode newNwsMode = filterNwsMode(appFamilyType, nwsMode, newVolteState);

            switchNwRat(appFamilyType, targetPrefNwType, newNwsMode, newVolteState, RAT_SWITCH_NORMAL,
                    NULL, message);
        } else {
            logD(RAT_CTRL_TAG, "setPreferredNetworkType: return directly with nothing to do.");
            sp<RfxMessage> resToRilj = RfxMessage::obtainResponse(
                    RIL_E_SUCCESS, message);
            responseToRilj(resToRilj);
        }
    } else {
        sp<RfxMessage> resToRilj = RfxMessage::obtainResponse(
                RIL_E_GENERIC_FAILURE, message);
        responseToRilj(resToRilj);
    }
}

int RpNwRatController::filterPrefNwType(const AppFamilyType appFamilyType,
        int prefNwType, int capability) {
    int targetPrefNwType = prefNwType;
    switch (prefNwType) {
    case PREF_NET_TYPE_LTE_CDMA_EVDO:
    case PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA:
        if (appFamilyType == APP_FAM_3GPP2) {
            if (capability == PREF_NET_TYPE_CDMA_EVDO_AUTO) {
                targetPrefNwType = PREF_NET_TYPE_CDMA_EVDO_AUTO;
            }
            if (capability == PREF_NET_TYPE_LTE_CDMA_EVDO) {
                targetPrefNwType = PREF_NET_TYPE_LTE_CDMA_EVDO;
            }
        } else if (appFamilyType == APP_FAM_3GPP) {
            if (capability == PREF_NET_TYPE_LTE_GSM_WCDMA) {
                targetPrefNwType = PREF_NET_TYPE_LTE_GSM_WCDMA;
            }
            if (capability == PREF_NET_TYPE_GSM_WCDMA) {
                targetPrefNwType = PREF_NET_TYPE_GSM_WCDMA;
            }
            if (capability == PREF_NET_TYPE_GSM_ONLY) {
                targetPrefNwType = PREF_NET_TYPE_GSM_ONLY;
            }
        }
        break;
    case PREF_NET_TYPE_CDMA_EVDO_AUTO:
    case PREF_NET_TYPE_CDMA_ONLY:
    case PREF_NET_TYPE_EVDO_ONLY:
    case PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO:
    case PREF_NET_TYPE_GSM_WCDMA:
    case PREF_NET_TYPE_GSM_ONLY:
    case PREF_NET_TYPE_WCDMA:
    case PREF_NET_TYPE_GSM_WCDMA_AUTO:
    case PREF_NET_TYPE_LTE_ONLY:
        break;
    case PREF_NET_TYPE_LTE_TDD_ONLY:
        if (getCapabilitySlotId() != m_slot_id) {
            targetPrefNwType = capability;
        }
        break;
    case PREF_NET_TYPE_LTE_GSM_WCDMA:
    case PREF_NET_TYPE_LTE_WCDMA:
    case PREF_NET_TYPE_LTE_GSM:
        /* the cdma card is in 3g mode, filter the 4g mode to 3g mode.*/
        if (appFamilyType == APP_FAM_3GPP2) {
            if (capability == PREF_NET_TYPE_CDMA_EVDO_AUTO) {
                targetPrefNwType = PREF_NET_TYPE_GSM_WCDMA;
            }
        }
        break;
    }
    return targetPrefNwType;
}

void RpNwRatController::creatRatSwitchHandlerIfNeeded(const AppFamilyType appFamType){
    if (appFamType == APP_FAM_3GPP) {
        // boot up work flow for gsm card.
        if (mNwRatSwitchHandler == NULL) {
            RFX_OBJ_CREATE_EX(mNwRatSwitchHandler, RpGsmNwRatSwitchHandler, this, (this));
            // for sim card hot plug in/out
        } else if (appFamType != mCurAppFamilyType) {
            if (!isRestrictedModeSupport()) {
                clearInvalidPendingRecords();
            }
            RFX_OBJ_CLOSE(mNwRatSwitchHandler);
            RFX_OBJ_CREATE_EX(mNwRatSwitchHandler, RpGsmNwRatSwitchHandler, this, (this));
        }
    } else if (appFamType == APP_FAM_3GPP2) {
        // boot up work flow for cdma card.
        if (mNwRatSwitchHandler == NULL) {
            RFX_OBJ_CREATE_EX(mNwRatSwitchHandler, RpCdmaLteNwRatSwitchHandler, this, (this));
            // for sim card hot plug in/out
        } else if (appFamType != mCurAppFamilyType) {
            if (!isRestrictedModeSupport()) {
                clearInvalidPendingRecords();
            }
            RFX_OBJ_CLOSE(mNwRatSwitchHandler);
            RFX_OBJ_CREATE_EX(mNwRatSwitchHandler, RpCdmaLteNwRatSwitchHandler, this, (this));
        }
    }
}

AppFamilyType RpNwRatController::getAppFamilyType() {
    if (isWwop()) {
        return mNwRatController->getAppFamilyType();
    }
    return mCurAppFamilyType;
}

int RpNwRatController::getPreferredNetworkType() {
    if (isWwop()) {
        return mNwRatController->getPreferredNetworkType();
    }
    return mCurPreferedNetWorkType;
}

void RpNwRatController::setNwsMode(const NwsMode nwsMode, const sp<RfxAction>& action) {
    if (isWwop()) {
        mNwRatController->setNwsMode(nwsMode, action);
        return;
    }
    logD(RAT_CTRL_TAG, "setNwsMode(), nwsMode is %d, mNewNwsMode is %d.", nwsMode, mNewNwsMode);
    if (getEnginenerMode() == ENGINEER_MODE_AUTO) {
        // Take an example, if nwsMode is CDMALTE, but mNewNwsMode is still CDMALTE, and the record
        // in queue is CSFB, we should also refresh the record's nwsmode in queue, otherwise, the
        // record in queue will be execute finally with wrong NWS mode.
        if (nwsMode != mNewNwsMode ||
            (mPendingNormalRatSwitchRecord.prefNwType != -1
             && nwsMode != mPendingNormalRatSwitchRecord.nwsMode)) {
            AppFamilyType appFamilyType = mNewAppFamilyType;
            if (appFamilyType == APP_FAM_3GPP2) {
                int prefNwType = mNewPreferedNetWorkType;
                VolteState volteState = mNewVolteState;

                if (mPendingNormalRatSwitchRecord.prefNwType != -1) {
                    prefNwType = mPendingNormalRatSwitchRecord.prefNwType;
                    volteState = mPendingNormalRatSwitchRecord.volteState;
                }

                // [VoLTE] If VoLTE on, need recalculate nw type to switch LWG/LTE Only
                if (volteState == VOLTE_ON || needConfigRatModeForCtDualVolte()) {
                    prefNwType = calculateVolteNetworkType(appFamilyType, nwsMode, volteState);
                }

                switchNwRat(appFamilyType, prefNwType, nwsMode, volteState, RAT_SWITCH_NWS, action,
                        NULL);
            } else if (appFamilyType == APP_FAM_3GPP) {
                // For CT VoLTE C+C Case: CDMA 4G card is in side slot, but AppFamilyType is 3GPP
                if (needConfigRatModeForCtDualVolte()) {
                    int prefNwType = mNewPreferedNetWorkType;
                    VolteState volteState = mNewVolteState;

                    if (mPendingNormalRatSwitchRecord.prefNwType != -1) {
                        volteState = mPendingNormalRatSwitchRecord.volteState;
                    }

                    prefNwType = calculateVolteNetworkType(appFamilyType, nwsMode, volteState);
                    if (is4GNetworkMode(prefNwType)) {
                        switchNwRat(appFamilyType, prefNwType, nwsMode, volteState, RAT_SWITCH_NWS,
                                action, NULL);
                    }
                }
            }
        }
    }
}

NwsMode RpNwRatController::getNwsMode() {
    if (isWwop()) {
        return mNwRatController->getNwsMode();
    }
    return mCurNwsMode;
}

VolteState RpNwRatController::getVolteState() {
    if (isWwop()) {
        return VOLTE_OFF;
    }
    return mCurVolteState;
}

void RpNwRatController::onInit() {
    // Required: invoke super class implementation
    RfxController::onInit();

    logD(RAT_CTRL_TAG,"onInit");

    // define and register request & urc id list
    const int request_id_list[] = {
        RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE,
        RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE,
        RIL_LOCAL_C2K_REQUEST_CONFIG_EVDO_MODE,
        RIL_REQUEST_VOICE_RADIO_TECH,
        RIL_LOCAL_REQUEST_SET_SVLTE_RAT_MODE,
        RIL_LOCAL_C2K_REQUEST_SET_SVLTE_RAT_MODE};
    const int urc_id_list[] = { };
    if (isWwop()) {
        RFX_OBJ_CREATE(mNwRatController, RpNwWwopRatController, this);
        logD(RAT_CTRL_TAG, "onInit(), Create WWOP RAT controller ");
        return;
    }

    if (RpFeatureOptionUtils::isC2kSupport()) {
        registerToHandleRequest(request_id_list, sizeof(request_id_list)/sizeof(int));
        registerToHandleUrc(urc_id_list, 0);
    }
    getStatusManager()->setIntValue(RFX_STATUS_KEY_NWS_MODE, mCurNwsMode);
    getStatusManager()->setIntValue(RFX_STATUS_KEY_PREFERRED_NW_TYPE, mCurPreferedNetWorkType);

      // register callbacks to get required information
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_CARD_TYPE,
        RfxStatusChangeCallback(this, &RpNwRatController::onCardTypeChanged));

    if (RpFeatureOptionUtils::isCtVolteSupport() && !(isWwop())) {
        getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_VOLTE_STATE,
                RfxStatusChangeCallback(this, &RpNwRatController::onVolteStateChanged));
        getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_CDMA_CARD_TYPE,
                RfxStatusChangeCallback(this, &RpNwRatController::onCTVolteCardTypeChanged));

        for (int slotId = RFX_SLOT_ID_0; slotId < RFX_SLOT_COUNT; slotId++) {
            getStatusManager(slotId)->registerStatusChangedEx(RFX_STATUS_KEY_VOICE_CALL_COUNT,
                    RfxStatusChangeCallbackEx(this, &RpNwRatController::onVoiceCallCountChanged));
        }
    }
}

void RpNwRatController::onDeinit() {
    getStatusManager()->unRegisterStatusChanged(RFX_STATUS_KEY_CARD_TYPE,
            RfxStatusChangeCallback(this, &RpNwRatController::onCardTypeChanged));

    if (RpFeatureOptionUtils::isCtVolteSupport() && !(isWwop())) {
        getStatusManager()->unRegisterStatusChanged(RFX_STATUS_KEY_VOLTE_STATE,
                RfxStatusChangeCallback(this, &RpNwRatController::onVolteStateChanged));
        getStatusManager()->unRegisterStatusChanged(RFX_STATUS_KEY_CDMA_CARD_TYPE,
                RfxStatusChangeCallback(this, &RpNwRatController::onCTVolteCardTypeChanged));

        for (int slotId = RFX_SLOT_ID_0; slotId < RFX_SLOT_COUNT; slotId++) {
            getStatusManager(slotId)->unRegisterStatusChangedEx(RFX_STATUS_KEY_VOICE_CALL_COUNT,
                    RfxStatusChangeCallbackEx(this, &RpNwRatController::onVoiceCallCountChanged));
        }
    }

    RfxController::onDeinit();
}

void RpNwRatController::doNwSwitchForEngMode(const sp<RfxAction>& action) {
    if (mNewAppFamilyType == APP_FAM_3GPP2) {
        switch (getEnginenerMode()) {
        case ENGINEER_MODE_CDMA:
            logD(RAT_CTRL_TAG, "Radio Avaliable, CDMA only mode. ");
            switchNwRat(APP_FAM_3GPP2, PREF_NET_TYPE_CDMA_EVDO_AUTO,
                    NWS_MODE_CDMALTE, VOLTE_OFF, RAT_SWITCH_INIT, action, NULL);
            break;
        case ENGINEER_MODE_CSFB:
            logD(RAT_CTRL_TAG, "Radio Avaliable, CSFB only mode. ");
            switchNwRat(APP_FAM_3GPP2, PREF_NET_TYPE_LTE_GSM_WCDMA,
                    NWS_MODE_CSFB, VOLTE_OFF, RAT_SWITCH_INIT, action, NULL);
            break;
        case ENGINEER_MODE_LTE:
            logD(RAT_CTRL_TAG, "Radio Avaliable, LTE only mode. ");
            switchNwRat(APP_FAM_3GPP2, PREF_NET_TYPE_LTE_ONLY, NWS_MODE_CSFB, VOLTE_OFF,
                    RAT_SWITCH_INIT, action, NULL);
            break;
        default:
            logD(RAT_CTRL_TAG, "Radio Avaliable, auto mode, do nothing. ");
            break;
        }
    }
}
bool RpNwRatController::onHandleRequest(const sp<RfxMessage>& message) {
    int msg_id = message->getId();
    // logD(RAT_CTRL_TAG,"handle req %s.", requestToString(msg_id));

    //  This is only for restart, setting is earlier than mode controller.
    if (msg_id == RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE
        && mNwRatSwitchHandler == NULL) {
        int32_t stgCount = 0;
        int32_t nwType = 0;
        message->getParcel()->readInt32(&stgCount);
        message->getParcel()->readInt32(&nwType);
        /// Add for compatible of 93MD.
        if (nwType > PREF_NET_TYPE_LTE_TDD_ONLY) {
            if (nwType == PREF_NET_TYPE_CDMA_GSM) {
                if (mNewAppFamilyType == APP_FAM_3GPP2) {
                    nwType = PREF_NET_TYPE_CDMA_ONLY;
                } else {
                    nwType = PREF_NET_TYPE_GSM_ONLY;
                }
            } else if (nwType == PREF_NET_TYPE_CDMA_EVDO_GSM) {
                if (mNewAppFamilyType == APP_FAM_3GPP2) {
                    nwType = PREF_NET_TYPE_CDMA_EVDO_AUTO;
                } else {
                    nwType = PREF_NET_TYPE_GSM_ONLY;
                }
            } else if (nwType == PREF_NET_TYPE_LTE_CDMA_EVDO_GSM) {
                if (mNewAppFamilyType == APP_FAM_3GPP2) {
                    nwType = PREF_NET_TYPE_LTE_CDMA_EVDO;
                } else {
                    nwType = PREF_NET_TYPE_LTE_GSM;
                }
            }
        }

        mPreferredNetWorkTypeFromRILJ = nwType;
        if (getCapabilitySlotId() == m_slot_id) {
            setPreferredNetWorkTypeToSysProp(m_slot_id, mPreferredNetWorkTypeFromRILJ);
        }
        logD(RAT_CTRL_TAG, "[handleSetPreferredNetworkType]setting is earlier than mode controller,"
                " send failure response ");
        if ((isWwop() != true)) {
            sp<RfxMessage> resToRilj = RfxMessage::obtainResponse(
                    RIL_E_GENERIC_FAILURE, message);
            responseToRilj(resToRilj);
        }
    }

    if (mNwRatSwitchHandler != NULL) {
        switch (msg_id) {
        case RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE:
        {
            int32_t stgCount = 0;
            int32_t nwType = 0;
            message->getParcel()->readInt32(&stgCount);
            message->getParcel()->readInt32(&nwType);
            // when C2K IR Settings set to cdma only, mPreferredNetWorkTypeFromRILJ
            // can only be updated to cdma only/evdo only/Hybid.
            if (getEnginenerMode() == ENGINEER_MODE_CDMA) {
                if (nwType == PREF_NET_TYPE_CDMA_ONLY
                        || nwType == PREF_NET_TYPE_EVDO_ONLY
                        || nwType == PREF_NET_TYPE_CDMA_EVDO_AUTO) {
                    mPreferredNetWorkTypeFromRILJ = nwType;
                }
            } else {
                /// Add for compatible of 93MD.
                if (nwType > PREF_NET_TYPE_LTE_TDD_ONLY) {
                    if (nwType == PREF_NET_TYPE_CDMA_GSM) {
                        if (mNewAppFamilyType == APP_FAM_3GPP2) {
                            nwType = PREF_NET_TYPE_CDMA_ONLY;
                        } else {
                            nwType = PREF_NET_TYPE_GSM_ONLY;
                        }
                    } else if (nwType == PREF_NET_TYPE_CDMA_EVDO_GSM) {
                        if (mNewAppFamilyType == APP_FAM_3GPP2) {
                            nwType = PREF_NET_TYPE_CDMA_EVDO_AUTO;
                        } else {
                            nwType = PREF_NET_TYPE_GSM_ONLY;
                        }
                    } else if (nwType == PREF_NET_TYPE_LTE_CDMA_EVDO_GSM) {
                        if (mNewAppFamilyType == APP_FAM_3GPP2) {
                            nwType = PREF_NET_TYPE_LTE_CDMA_EVDO;
                        } else {
                            nwType = PREF_NET_TYPE_LTE_GSM;
                        }
                    }
                }

                mPreferredNetWorkTypeFromRILJ = nwType;
                if (getCapabilitySlotId() == m_slot_id) {
                    setPreferredNetWorkTypeToSysProp(m_slot_id, mPreferredNetWorkTypeFromRILJ);
                }
            }
            setPreferredNetworkType(nwType, message);
            return true;
        }
        case RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE:
            logD(RAT_CTRL_TAG, "[handleGetPreferredNwType] mPreferredNetworkTypeFromRILJ:%d",
                mPreferredNetWorkTypeFromRILJ);
            switch (mPreferredNetWorkTypeFromRILJ) {
                case PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA:
                case PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO:
                case PREF_NET_TYPE_LTE_CDMA_EVDO: {
                    sp<RfxMessage> resToRilj = RfxMessage::obtainResponse(RIL_E_SUCCESS, message);
                    resToRilj->getParcel()->writeInt32(1);
                    resToRilj->getParcel()->writeInt32(mPreferredNetWorkTypeFromRILJ);
                    responseToRilj(resToRilj);
                    return true;
                 }
                 default:
                     break;
            }
            mNwRatSwitchHandler->requestGetPreferredNetworkType(message);
            return true;

        case RIL_REQUEST_VOICE_RADIO_TECH:
            if (sIsInSwitching) {
                sp<RfxMessage> resToRilj = RfxMessage::obtainResponse(RIL_E_SUCCESS, message);
                resToRilj->getParcel()->writeInt32(1);
                resToRilj->getParcel()->writeInt32(RADIO_TECH_UNKNOWN);
                responseToRilj(resToRilj);
                return true;
            }
            if (mCurNwsMode == NWS_MODE_CSFB || mCurNwsMode == NWS_MODE_LTEONLY) {
                if (mCurPreferedNetWorkType == PREF_NET_TYPE_LTE_ONLY ||
                        mCurPreferedNetWorkType == PREF_NET_TYPE_LTE_TDD_ONLY) {
                    sp<RfxMessage> resToRilj = RfxMessage::obtainResponse(
                        RIL_E_SUCCESS, message);
                    resToRilj->getParcel()->writeInt32(1);
                    resToRilj->getParcel()->writeInt32(RADIO_TECH_LTE);
                    logD(RAT_CTRL_TAG, "request voice radio tech, send response.voiceRadioTech = 14.");
                    responseToRilj(resToRilj);
                } else {
                    mNwRatSwitchHandler->requestGetVoiceRadioTech(message);
                }
            } else {
                sp<RfxMessage> resToRilj = RfxMessage::obtainResponse(
                        RIL_E_SUCCESS, message);
                resToRilj->getParcel()->writeInt32(1);
                resToRilj->getParcel()->writeInt32(RADIO_TECH_1xRTT);
                logD(RAT_CTRL_TAG, "request voice radio tech, send response.voiceRadioTech = 6.");
                responseToRilj(resToRilj);
            }
            return true;
        }
    }
    return false;
}

bool RpNwRatController::onHandleResponse(const sp<RfxMessage>& message) {
    int msg_id = message->getId();
    // logD(RAT_CTRL_TAG,"handle %s response.", requestToString(msg_id));

    if (mNwRatSwitchHandler != NULL) {
        switch (msg_id) {
        case RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE:
            if (RADIO_TECH_GROUP_GSM == message->getSource()) {
                mNwRatSwitchHandler->responseSetPreferredNetworkType(message);
            }
            return true;
        case RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE:
            mNwRatSwitchHandler->responseGetPreferredNetworkType(message);
            return true;
        case RIL_LOCAL_C2K_REQUEST_CONFIG_EVDO_MODE:
            mNwRatSwitchHandler->responseSetEvdoMode(message);
            return true;
        case RIL_LOCAL_REQUEST_SET_SVLTE_RAT_MODE:
        case RIL_LOCAL_C2K_REQUEST_SET_SVLTE_RAT_MODE: {
            if (mNwRatSwitchHandler->isCdma3gDualModeCard()) {
                logD(RAT_CTRL_TAG, "3gCdmaSim handle RIL_LOCAL_C2K_REQUEST_SET_SVLTE_RAT_MODE,"
                        " error is %d", message->getError());
                sp<RfxMessage> msg = sp < RfxMessage > (NULL);
                ResponseStatus responseStatus = preprocessResponse(message, msg,
                        RfxWaitResponseTimedOutCallback(mNwRatSwitchHandler,
                                &RpBaseNwRatSwitchHandler::onResponseTimeOut),
                        s2ns(10));
                if (message->getError() != RIL_E_SUCCESS
                        || responseStatus == RESPONSE_STATUS_HAVE_MATCHED) {
                    mNwRatSwitchHandler->responseSetRatMode(message);
                }
                // If c has changed, then stop this process
                if (responseStatus == RESPONSE_STATUS_HAVE_MATCHED_WITH_C_CHANGED) {
                    mNwRatSwitchHandler->onNwRatSwitchDone(RAT_SWITCH_FAIL);
                }
            } else {
                mNwRatSwitchHandler->responseSetRatMode(message);
            }
            return true;
        }
        case RIL_REQUEST_VOICE_RADIO_TECH:
            mNwRatSwitchHandler->responseGetVoiceRadioTech(message);
            return true;
        }
    }
    return false;
}

bool RpNwRatController::onHandleUrc(const sp<RfxMessage>& message) {
    int msg_id = message->getId();
    logD(RAT_CTRL_TAG,"handle urc %s.", urcToString(msg_id));

    switch (msg_id) {
    default:
        break;
    }
    return true;
}

const char* RpNwRatController::requestToString(int reqId) {
    switch (reqId) {
    case RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE:
        return "RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE";
    case RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE:
        return "RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE";
    case RIL_LOCAL_C2K_REQUEST_CONFIG_EVDO_MODE:
        return "RIL_LOCAL_C2K_REQUEST_CONFIG_EVDO_MODE";
    case RIL_LOCAL_REQUEST_SET_SVLTE_RAT_MODE:
    case RIL_LOCAL_C2K_REQUEST_SET_SVLTE_RAT_MODE:
        return "RIL_REQUEST_SET_SVLTE_RAT_MODE";
    case RIL_REQUEST_VOICE_RADIO_TECH:
        return "RIL_REQUEST_VOICE_RADIO_TECH";
    default:
        logD(RAT_CTRL_TAG,"<UNKNOWN_REQUEST>");
        break;
    }
    return "";
}

const char* RpNwRatController::urcToString(int reqId) {
    switch (reqId) {
    default:
        logD(RAT_CTRL_TAG,"<UNKNOWN_URC>");
        break;
    }
    return "";
}

void RpNwRatController::registerRatSwitchCallback(IRpNwRatSwitchCallback* callback) {
    if (isWwop()) {
        mNwRatController->registerRatSwitchCallback(callback);
        return;
    }
    mRpNwRatSwitchListener = callback;
}

void RpNwRatController::unRegisterRatSwitchCallback(IRpNwRatSwitchCallback* callback) {
    if (isWwop()) {
        mNwRatController->unRegisterRatSwitchCallback(callback);
        return;
    }
    mRpNwRatSwitchListener = NULL;
}

void RpNwRatController::onRatSwitchStart(const int prefNwType, const NwsMode newNwsMode,
        const VolteState volteState) {
    if (mRpNwRatSwitchListener != NULL) {
        mRpNwRatSwitchListener->onRatSwitchStart(mCurPreferedNetWorkType,
                prefNwType, mCurNwsMode, newNwsMode, mCurVolteState, volteState);
    }
}
void RpNwRatController::onRatSwitchDone(const int prefNwType){
    if (mRpNwRatSwitchListener != NULL) {
        mRpNwRatSwitchListener->onRatSwitchDone(mCurPreferedNetWorkType,
                prefNwType);
    }
}
void RpNwRatController::onEctModeChangeDone(const int prefNwType){
    if (mRpNwRatSwitchListener != NULL) {
        mRpNwRatSwitchListener->onEctModeChangeDone(mCurPreferedNetWorkType,
                prefNwType);
    }
}
void RpNwRatController::switchNwRat(const AppFamilyType appFamType,
        int prefNwType, NwsMode nwsMode, VolteState volteState,
        const RatSwitchCaller ratSwitchCaller,
        const sp<RfxAction>& action, const sp<RfxMessage>& message) {
    logD(RAT_CTRL_TAG, "switchNwRat(), appFamType is %d, prefNwType is %d, nwsMode is %d . "
            "volteState is %d, ratSwitchCaller is %d, sIsInSwitching is %s. ",
            appFamType, prefNwType, nwsMode, volteState, ratSwitchCaller,
            sIsInSwitching ? "true" : "false");

    if (sIsInSwitching) {
        queueRatSwitchRecord(appFamType, prefNwType, nwsMode, volteState, ratSwitchCaller, action, message);
    } else {
        if (ratSwitchCaller != RAT_SWITCH_RESTRICT
                && mPendingRestrictedRatSwitchRecord.prefNwType != -1) {
            queueRatSwitchRecord(appFamType, prefNwType, nwsMode, volteState, ratSwitchCaller, action, message);
            return;
        }
        sIsInSwitching = true;

        /* create the rat switch handler if need. */
        creatRatSwitchHandlerIfNeeded(appFamType);
        mCurAppFamilyType = appFamType;


        if (appFamType == APP_FAM_3GPP2) {
            /* Calculate the target preferred network type. */
            int targetPrefNwType = calculateTargetPreferredNwType(appFamType, prefNwType, nwsMode);

            /* Handle the network preferred network switch. */
            if (isValidPreferredNwType(appFamType, targetPrefNwType, nwsMode)) {
                mNewPreferedNetWorkType = targetPrefNwType;
                mNewNwsMode = nwsMode;
                mNewVolteState = volteState;
                ModemSettings mdSettings = mNwRatSwitchHandler->calculateModemSettings(
                        targetPrefNwType, appFamType, nwsMode, volteState);
                mNwRatSwitchHandler->doNwRatSwitch(mdSettings, ratSwitchCaller, action, message);
            } else {
                logD(RAT_CTRL_TAG, "switchNwRat(), invalid prefNwType is %d.", prefNwType);
                if ((ratSwitchCaller == RAT_SWITCH_NORMAL || ratSwitchCaller == RAT_SWITCH_VOLTE)
                        && message != NULL) {
                    mNwRatSwitchHandler->responseSetPreferredNetworkType(message);
                }

                // Reset state and callback action if the RAT is invalid.
                updateState(mCurNwsMode, mCurPreferedNetWorkType, mCurVolteState);
                if (action != NULL) {
                    action->act();
                }
            }
        } else if (appFamType == APP_FAM_3GPP) {
            ModemSettings mdSettings = mNwRatSwitchHandler->calculateModemSettings(prefNwType,
                    appFamType, nwsMode, volteState);
            if (mdSettings.prefNwType != -1) {
                RpRadioController* radioController =
                        (RpRadioController *) findController(RFX_OBJ_CLASS_INFO(RpRadioController));
                RpSuggestRadioCapabilityCallback callback =
                        RpSuggestRadioCapabilityCallback(this,
                                &RpNwRatController::onSuggestRadioCapabilityResult);
                radioController->suggestedCapability(
                        getSuggestedRadioCapability(mdSettings), callback);
                mNewPreferedNetWorkType = mdSettings.prefNwType;
                mNewNwsMode = nwsMode;
                mNewVolteState = volteState;
                mNwRatSwitchHandler->doNwRatSwitch(mdSettings, ratSwitchCaller, action, message);

                if (!isEratExtSupport()) {
                    mNwRatSwitchHandler->updatePhone(mdSettings);
                    updateState(mNewNwsMode, mNewPreferedNetWorkType, mNewVolteState);
                    if (action != NULL) {
                        action->act();
                    }
                    logD(RAT_CTRL_TAG, "switchNwRat(), rat switch done for 3gpp card.");
                }
            } else {
                logD(RAT_CTRL_TAG, "switchNwRat(), invalid prefNwType is %d.", prefNwType);
                if (message != NULL) {
                    sp<RfxMessage> rlt = RfxMessage::obtainResponse(
                            RIL_E_GENERIC_FAILURE, message);
                    responseToRilj(rlt);
                }
                updateState(mCurNwsMode, mCurPreferedNetWorkType, mCurVolteState);

                if (action != NULL) {
                    action->act();
                }
                logD(RAT_CTRL_TAG, "switchNwRat(), rat switch done for 3gpp card.");
            }
        }
    }
}

int RpNwRatController::getSuggestedRadioCapability(ModemSettings mdSettings) {
    int suggestedRadio = RIL_CAPABILITY_NONE;
    bool md1Radio = mdSettings.md1Radio;
    bool md3Radio = mdSettings.md3Radio;
    if (md1Radio && md3Radio) {
        suggestedRadio = RIL_CAPABILITY_CDMA_ON_LTE;
    } else if (md1Radio && !md3Radio) {
        suggestedRadio = RIL_CAPABILITY_GSM_ONLY;
    } else if (!md1Radio && md3Radio) {
        suggestedRadio = RIL_CAPABILITY_CDMA_ONLY;
    }
    return suggestedRadio;
}

void RpNwRatController::queueRatSwitchRecord(const AppFamilyType appFamType,
        int prefNwType, NwsMode nwsMode, VolteState volteState,
        const RatSwitchCaller ratSwitchCaller,
        const sp<RfxAction>& action, const sp<RfxMessage>& message) {
    /* Pending if in switching. */
    logD(RAT_CTRL_TAG, "queueRatSwitchRecord(), ratSwitchCaller:%d prefNwType:%d",
            ratSwitchCaller, prefNwType);
    if (ratSwitchCaller == RAT_SWITCH_RESTRICT) {
        mPendingRestrictedRatSwitchRecord.appFamType = appFamType;
        mPendingRestrictedRatSwitchRecord.prefNwType = prefNwType;
        mPendingRestrictedRatSwitchRecord.nwsMode = nwsMode;
        mPendingRestrictedRatSwitchRecord.volteState = volteState;
        mPendingRestrictedRatSwitchRecord.ratSwitchCaller = ratSwitchCaller;
        mPendingRestrictedRatSwitchRecord.action = action;
        mPendingRestrictedRatSwitchRecord.message = message;
    } else if (ratSwitchCaller == RAT_SWITCH_INIT) {
        mPendingInitRatSwitchRecord.appFamType = appFamType;
        mPendingInitRatSwitchRecord.prefNwType = prefNwType;
        mPendingInitRatSwitchRecord.nwsMode = nwsMode;
        mPendingInitRatSwitchRecord.volteState = volteState;
        mPendingInitRatSwitchRecord.ratSwitchCaller = ratSwitchCaller;
        mPendingInitRatSwitchRecord.action = action;
        mPendingInitRatSwitchRecord.message = message;
    } else {
        if (mPendingNormalRatSwitchRecord.prefNwType != -1
                && mPendingNormalRatSwitchRecord.message != NULL) {
            logD(RAT_CTRL_TAG, "switchNwRat(), request set prefer network type is pending, "
                    "will be ignored, send response.");
            sp<RfxMessage> resToRilj = RfxMessage::obtainResponse(
                RIL_E_GENERIC_FAILURE, mPendingNormalRatSwitchRecord.message);
            responseToRilj(resToRilj);
        }
        mPendingNormalRatSwitchRecord.appFamType = appFamType;
        mPendingNormalRatSwitchRecord.prefNwType = prefNwType;
        mPendingNormalRatSwitchRecord.nwsMode = nwsMode;
        mPendingNormalRatSwitchRecord.volteState = volteState;
        if (mPendingNormalRatSwitchRecord.ratSwitchCaller == RAT_SWITCH_VOLTE) {
            // Keep pending ratSwitchCaller is VOLTE
        } else {
            mPendingNormalRatSwitchRecord.ratSwitchCaller = ratSwitchCaller;
        }
        mPendingNormalRatSwitchRecord.action = action;
        mPendingNormalRatSwitchRecord.message = message;
    }
}

bool RpNwRatController::isValidPreferredNwType(const AppFamilyType appFamType,
        int prefNwType, NwsMode nwsMode) {
    bool isValidPreferredNwType = true;
    if (appFamType == APP_FAM_3GPP2) {
        if (nwsMode == NWS_MODE_CDMALTE) {
            switch (prefNwType) {
                case PREF_NET_TYPE_CDMA_EVDO_AUTO:
                case PREF_NET_TYPE_CDMA_ONLY:
                case PREF_NET_TYPE_EVDO_ONLY:
                case PREF_NET_TYPE_LTE_CDMA_EVDO:
                case PREF_NET_TYPE_LTE_ONLY:
                case PREF_NET_TYPE_LTE_TDD_ONLY:
                    isValidPreferredNwType = true;
                    break;

                case PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA:
                    if (isRestrictedModeSupport()) {
                        isValidPreferredNwType = true;
                        break;
                    }
                case PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO:
                case PREF_NET_TYPE_GSM_WCDMA:
                case PREF_NET_TYPE_GSM_ONLY:
                case PREF_NET_TYPE_WCDMA:
                case PREF_NET_TYPE_GSM_WCDMA_AUTO:
                case PREF_NET_TYPE_LTE_GSM_WCDMA:
                case PREF_NET_TYPE_LTE_WCDMA:
                case PREF_NET_TYPE_LTE_GSM:
                    isValidPreferredNwType = false;
                    break;
            }
        }

        if (nwsMode == NWS_MODE_CSFB) {
            switch (prefNwType) {
            case PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA:
                if (isRestrictedModeSupport()) {
                    isValidPreferredNwType = true;
                    break;
                }
            case PREF_NET_TYPE_CDMA_EVDO_AUTO:
            case PREF_NET_TYPE_CDMA_ONLY:
            case PREF_NET_TYPE_EVDO_ONLY:
            case PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO:
            case PREF_NET_TYPE_LTE_CDMA_EVDO:
                isValidPreferredNwType = false;
                break;

            case PREF_NET_TYPE_GSM_WCDMA:
            case PREF_NET_TYPE_GSM_ONLY:
            case PREF_NET_TYPE_WCDMA:
            case PREF_NET_TYPE_GSM_WCDMA_AUTO:
            case PREF_NET_TYPE_LTE_GSM_WCDMA:
            case PREF_NET_TYPE_LTE_ONLY:
            case PREF_NET_TYPE_LTE_TDD_ONLY:
            case PREF_NET_TYPE_LTE_WCDMA:
            case PREF_NET_TYPE_LTE_GSM:
                isValidPreferredNwType = true;
                break;
            }
        }

        if (nwsMode == NWS_MODE_LTEONLY) {
            if (prefNwType == PREF_NET_TYPE_LTE_ONLY) {
                isValidPreferredNwType = true;
            } else {
                isValidPreferredNwType = false;
            }
        }
    }
    return isValidPreferredNwType;
}

void RpNwRatController::doPendingRatSwitchRecord() {
    // Handle the pending item
    if (mPendingRestrictedRatSwitchRecord.prefNwType != -1) {
        logD(RAT_CTRL_TAG, "doPendingRestrictedRatSwitchRecord(), appFamType is %d, "
                "prefNwType is %d, nwsMode is %d, volteState is %d. ratSwitchCaller is %d"
                "mCurPreferedNetWorkType is %d, mCurNwsMode is %d.",
                mPendingRestrictedRatSwitchRecord.appFamType,
                mPendingRestrictedRatSwitchRecord.prefNwType,
                mPendingRestrictedRatSwitchRecord.nwsMode,
                mPendingRestrictedRatSwitchRecord.volteState,
                mPendingRestrictedRatSwitchRecord.ratSwitchCaller,
                mCurPreferedNetWorkType, mCurNwsMode);
        if (mCurPreferedNetWorkType == mPendingRestrictedRatSwitchRecord.prefNwType) {
            logD(RAT_CTRL_TAG, "doPendingRestrictedRatSwitchRecord(), "
                    "in restricted mode: prefNwType=%d, nwsMode=%d",
                    mCurPreferedNetWorkType, mCurNwsMode);
        } else {
            switchNwRat(mPendingRestrictedRatSwitchRecord.appFamType,
                    mPendingRestrictedRatSwitchRecord.prefNwType,
                    mPendingRestrictedRatSwitchRecord.nwsMode,
                    mPendingRestrictedRatSwitchRecord.volteState,
                    mPendingRestrictedRatSwitchRecord.ratSwitchCaller,
                    mPendingRestrictedRatSwitchRecord.action,
                    mPendingRestrictedRatSwitchRecord.message);
        }
    } else if (mPendingInitRatSwitchRecord.prefNwType != -1) {
        logD(RAT_CTRL_TAG, "doPendingInitRatSwitchRecord(), appFamType is %d, "
                "prefNwType is %d, nwsMode is %d, volteState is %d. ratSwitchCaller is %d",
                mPendingInitRatSwitchRecord.appFamType,
                mPendingInitRatSwitchRecord.prefNwType,
                mPendingInitRatSwitchRecord.nwsMode,
                mPendingInitRatSwitchRecord.volteState,
                mPendingInitRatSwitchRecord.ratSwitchCaller);
        PendingRatSwitchRecord tempInitRatSwitchRecord;
        tempInitRatSwitchRecord.appFamType = mPendingInitRatSwitchRecord.appFamType;
        tempInitRatSwitchRecord.prefNwType = mPendingInitRatSwitchRecord.prefNwType;
        tempInitRatSwitchRecord.nwsMode = mPendingInitRatSwitchRecord.nwsMode;
        tempInitRatSwitchRecord.volteState = mPendingInitRatSwitchRecord.volteState;
        tempInitRatSwitchRecord.ratSwitchCaller = mPendingInitRatSwitchRecord.ratSwitchCaller;
        tempInitRatSwitchRecord.action = mPendingInitRatSwitchRecord.action;
        tempInitRatSwitchRecord.message = mPendingInitRatSwitchRecord.message;
        mPendingInitRatSwitchRecord.prefNwType = -1;
        switchNwRat(tempInitRatSwitchRecord.appFamType,
                tempInitRatSwitchRecord.prefNwType,
                tempInitRatSwitchRecord.nwsMode,
                tempInitRatSwitchRecord.volteState,
                tempInitRatSwitchRecord.ratSwitchCaller,
                tempInitRatSwitchRecord.action,
                tempInitRatSwitchRecord.message);

    } else if (mPendingNormalRatSwitchRecord.prefNwType != -1) {
        logD(RAT_CTRL_TAG, "doPendingNormalRatSwitchRecord(), appFamType is %d, "
                "prefNwType is %d, nwsMode is %d, volteState is %d. ratSwitchCaller is %d",
                mPendingNormalRatSwitchRecord.appFamType,
                mPendingNormalRatSwitchRecord.prefNwType,
                mPendingNormalRatSwitchRecord.nwsMode,
                mPendingNormalRatSwitchRecord.volteState,
                mPendingNormalRatSwitchRecord.ratSwitchCaller);
        PendingRatSwitchRecord tempNormalRatSwitchRecord;
        tempNormalRatSwitchRecord.appFamType = mPendingNormalRatSwitchRecord.appFamType;
        tempNormalRatSwitchRecord.prefNwType = mPendingNormalRatSwitchRecord.prefNwType;
        tempNormalRatSwitchRecord.nwsMode = mPendingNormalRatSwitchRecord.nwsMode;
        tempNormalRatSwitchRecord.volteState = mPendingNormalRatSwitchRecord.volteState;
        tempNormalRatSwitchRecord.ratSwitchCaller = mPendingNormalRatSwitchRecord.ratSwitchCaller;
        tempNormalRatSwitchRecord.action = mPendingNormalRatSwitchRecord.action;
        tempNormalRatSwitchRecord.message = mPendingNormalRatSwitchRecord.message;
        mPendingNormalRatSwitchRecord.prefNwType = -1;
        switchNwRat(tempNormalRatSwitchRecord.appFamType,
                tempNormalRatSwitchRecord.prefNwType,
                tempNormalRatSwitchRecord.nwsMode,
                tempNormalRatSwitchRecord.volteState,
                tempNormalRatSwitchRecord.ratSwitchCaller,
                tempNormalRatSwitchRecord.action,
                tempNormalRatSwitchRecord.message);

    } else {
        RpNwRatController *another = (RpNwRatController *) findController(
                getSlotId() == 0 ? 1 : 0,
                RFX_OBJ_CLASS_INFO(RpNwRatController));
        if (another != NULL && another->hasPendingRecord()) {
            logD(RAT_CTRL_TAG, "doPendingRatSwitchRecord, another SIM has pending record, "
                    "current is %d", getSlotId());
            another->doPendingRatSwitchRecord();
        }

        logD(RAT_CTRL_TAG, "doPendingRatSwitchRecord(), no pending record, "
                "another sim has no pending record also, finish");
    }
}

bool RpNwRatController::hasPendingRecord() {
    if (mPendingInitRatSwitchRecord.prefNwType != -1
            || mPendingNormalRatSwitchRecord.prefNwType != -1
            || mPendingRestrictedRatSwitchRecord.prefNwType != -1) {
        return true;
    }
    return false;
}

int RpNwRatController::calculateTargetPreferredNwType(const AppFamilyType appFamType, int prefNwType, NwsMode nwsMode) {
    int targetPrefNwType = -1;
    if (nwsMode == NWS_MODE_CDMALTE) {
        if (appFamType == APP_FAM_3GPP) {
            targetPrefNwType = prefNwType;
        } else if (appFamType == APP_FAM_3GPP2) {
            switch (prefNwType) {
            case PREF_NET_TYPE_CDMA_EVDO_AUTO:
            case PREF_NET_TYPE_CDMA_ONLY:
            case PREF_NET_TYPE_EVDO_ONLY:
            case PREF_NET_TYPE_LTE_ONLY:
            case PREF_NET_TYPE_LTE_TDD_ONLY:
                targetPrefNwType = prefNwType;
                break;
            case PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO:
            case PREF_NET_TYPE_GSM_WCDMA:
            case PREF_NET_TYPE_GSM_ONLY:
            case PREF_NET_TYPE_WCDMA:
            case PREF_NET_TYPE_GSM_WCDMA_AUTO:
                targetPrefNwType = PREF_NET_TYPE_CDMA_EVDO_AUTO;
                break;
            case PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA:
                if (isRestrictedModeSupport()) {
                    targetPrefNwType = prefNwType;
                    break;
                }
            case PREF_NET_TYPE_LTE_CDMA_EVDO:
            case PREF_NET_TYPE_LTE_GSM_WCDMA:
            case PREF_NET_TYPE_LTE_WCDMA:
            case PREF_NET_TYPE_LTE_GSM:
                targetPrefNwType = PREF_NET_TYPE_LTE_CDMA_EVDO;
                // VoLTE off, CDMA 4G card is in side slot, convert networkType from 4G to 3G
                if (needConfigRatModeForCtDualVolte()) {
                    logD(LOG_TAG, "calculateTargetPreferredNwType, LTE_CDMA_EVDO to CDMA_EVDO");
                    targetPrefNwType = PREF_NET_TYPE_CDMA_EVDO_AUTO;
                }
                break;
            }
        }
    }

    if (nwsMode == NWS_MODE_CSFB) {
        switch (prefNwType) {
        case PREF_NET_TYPE_CDMA_EVDO_AUTO:
        case PREF_NET_TYPE_CDMA_ONLY:
        case PREF_NET_TYPE_EVDO_ONLY:
        case PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO:
            targetPrefNwType = PREF_NET_TYPE_GSM_WCDMA;
            break;
        case PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA:
            if (isRestrictedModeSupport()) {
                targetPrefNwType = prefNwType;
                break;
            }
        case PREF_NET_TYPE_LTE_CDMA_EVDO:
            targetPrefNwType = PREF_NET_TYPE_LTE_GSM_WCDMA;
            break;
        case PREF_NET_TYPE_GSM_WCDMA:
        case PREF_NET_TYPE_GSM_ONLY:
        case PREF_NET_TYPE_WCDMA:
        case PREF_NET_TYPE_GSM_WCDMA_AUTO:
        case PREF_NET_TYPE_LTE_GSM_WCDMA:
        case PREF_NET_TYPE_LTE_ONLY:
        case PREF_NET_TYPE_LTE_TDD_ONLY:
        case PREF_NET_TYPE_LTE_WCDMA:
        case PREF_NET_TYPE_LTE_GSM:
            targetPrefNwType = prefNwType;
            break;
        }
    }

    if (nwsMode == NWS_MODE_LTEONLY) {
        if (appFamType == APP_FAM_3GPP) {
            targetPrefNwType = prefNwType;
        } else if (appFamType == APP_FAM_3GPP2) {
            switch (prefNwType) {
            case PREF_NET_TYPE_LTE_CDMA_EVDO:
            case PREF_NET_TYPE_LTE_ONLY:
            case PREF_NET_TYPE_LTE_TDD_ONLY:
            case PREF_NET_TYPE_LTE_GSM_WCDMA:
            case PREF_NET_TYPE_LTE_WCDMA:
            case PREF_NET_TYPE_LTE_GSM:
                targetPrefNwType = PREF_NET_TYPE_LTE_ONLY;
                break;
            case PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA:
                if (isRestrictedModeSupport()) {
                    targetPrefNwType = prefNwType;
                } else {
                    targetPrefNwType = PREF_NET_TYPE_LTE_ONLY;
                }
                break;
            case PREF_NET_TYPE_CDMA_EVDO_AUTO:
            case PREF_NET_TYPE_CDMA_ONLY:
            case PREF_NET_TYPE_EVDO_ONLY:
            case PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO:
            case PREF_NET_TYPE_GSM_WCDMA:
            case PREF_NET_TYPE_GSM_ONLY:
            case PREF_NET_TYPE_WCDMA:
            case PREF_NET_TYPE_GSM_WCDMA_AUTO:
                // ignore this prefNwType
                break;
            default:
                break;
            }
        }
    }

    logD(RAT_CTRL_TAG, "calculateTargetPreferredNwType, prefNwType=%d, targetPrefNwType=%d",
            prefNwType, targetPrefNwType);
    return targetPrefNwType;
}

void RpNwRatController::updateState(NwsMode nwsMode, int prefNwType, VolteState volteState) {
    logD(RAT_CTRL_TAG,"updateNwsMode(), nwsMode is %d, prefNwType is %d. ", nwsMode, prefNwType);
    mCurNwsMode = nwsMode;
    mCurPreferedNetWorkType = prefNwType;
    mCurVolteState = volteState;
    getStatusManager()->setIntValue(RFX_STATUS_KEY_NWS_MODE, mCurNwsMode);
    getStatusManager()->setIntValue(RFX_STATUS_KEY_PREFERRED_NW_TYPE, mCurPreferedNetWorkType);
    sIsInSwitching = false;
}

bool RpNwRatController::getSwitchState() {
    return sIsInSwitching;
}

int RpNwRatController::getEnginenerMode() {
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("persist.vendor.radio.ct.ir.engmode", property_value, "0");
    int engineerMode = atoi(property_value);
    return engineerMode;
}

void RpNwRatController::clearSuggetRadioCapability() {
    RpRadioController* radioController =
            (RpRadioController *) findController(RFX_OBJ_CLASS_INFO(RpRadioController));
    RpSuggestRadioCapabilityCallback callback = RpSuggestRadioCapabilityCallback(this, &RpNwRatController::onSuggestRadioCapabilityResult);
    radioController->suggestedCapability(RIL_CAPABILITY_NONE, callback);
    logD(RAT_CTRL_TAG, "clearSuggetRadioCapability");

}

void RpNwRatController::clearInvalidPendingRecords() {
    if (mPendingInitRatSwitchRecord.prefNwType != -1) {
        logD(RAT_CTRL_TAG, "clearInvalidPendingRecords, has pending init rat switch record: appFamType is %d, "
                "prefNwType is %d, nwsMode is %d . ratSwitchCaller is %d",
                mPendingInitRatSwitchRecord.appFamType,
                mPendingInitRatSwitchRecord.prefNwType,
                mPendingInitRatSwitchRecord.nwsMode,
                mPendingInitRatSwitchRecord.ratSwitchCaller);
        if (mPendingInitRatSwitchRecord.action != NULL) {
            mPendingInitRatSwitchRecord.action->act();
        }
        if (mPendingInitRatSwitchRecord.message != NULL) {
            sp<RfxMessage> resToRilj = RfxMessage::obtainResponse(RIL_E_SUCCESS,
                    mPendingInitRatSwitchRecord.message);
            responseToRilj(resToRilj);
        }
        mPendingInitRatSwitchRecord.prefNwType = -1;
    }
    if (mPendingNormalRatSwitchRecord.prefNwType != -1) {
        logD(RAT_CTRL_TAG, "clearInvalidPendingRecords, has pending normal rat switch record: appFamType is %d, "
                "prefNwType is %d, nwsMode is %d . ratSwitchCaller is %d",
                mPendingNormalRatSwitchRecord.appFamType,
                mPendingNormalRatSwitchRecord.prefNwType,
                mPendingNormalRatSwitchRecord.nwsMode,
                mPendingNormalRatSwitchRecord.ratSwitchCaller);
        if (mPendingNormalRatSwitchRecord.action != NULL) {
            mPendingNormalRatSwitchRecord.action->act();
        }
        if (mPendingNormalRatSwitchRecord.message != NULL) {
            sp<RfxMessage> resToRilj = RfxMessage::obtainResponse(RIL_E_SUCCESS,
                    mPendingNormalRatSwitchRecord.message);
            responseToRilj(resToRilj);
        }
        mPendingNormalRatSwitchRecord.prefNwType = -1;
    }
}

bool RpNwRatController::isRestrictedModeSupport() {
    bool mode = false;
    char prop_val[PROPERTY_VALUE_MAX] = { 0 };
    property_get("persist.vendor.operator.optr", prop_val, "");
    if (strcmp("OP12", prop_val) == 0) {
        mode = true;
    }
    return mode;
}

void RpNwRatController::onSuggestRadioCapabilityResult(SuggestRadioResult result) {
    // logD(RAT_CTRL_TAG, "onSuggestRadioCapabilityResult, result is %d .", result);
}

NwsMode RpNwRatController::getNwsModeForSwitchCardType() {
    if (mNwRatSwitchHandler != NULL) {
        logD(RAT_CTRL_TAG, "getNwsModeForSwitchCardType(), NwsMode is %d. ",
                mNwRatSwitchHandler->getNwsModeForSwitchCardType());
        return mNwRatSwitchHandler->getNwsModeForSwitchCardType();
    } else {
        logD(RAT_CTRL_TAG, "getNwsModeForSwitchCardType(), mNwRatSwitchHandler has not "
                "been initialized ");
        return NWS_MODE_CSFB;
    }
}

int RpNwRatController::getCapabilitySlotId() {
    char tempstr[PROPERTY_VALUE_MAX];
    memset(tempstr, 0, sizeof(tempstr));
    property_get("persist.vendor.radio.simswitch", tempstr, "1");
    int capabilitySlotId = atoi(tempstr) - 1;
    return capabilitySlotId;
}

void RpNwRatController::setPreferredNetWorkTypeToSysProp(int slotId, int val) {
    char PROPERTY_RILJ_NW_TYPE[4][40] = {
        "persist.vendor.radio.rilj_nw_type1",
        "persist.vendor.radio.rilj_nw_type2",
        "persist.vendor.radio.rilj_nw_type3",
        "persist.vendor.radio.rilj_nw_type4",
    };
    setIntSysProp(PROPERTY_RILJ_NW_TYPE[slotId], val);
}

int RpNwRatController::getPreferredNetWorkTypeFromSysProp(int slotId) {
    char PROPERTY_RILJ_NW_TYPE[4][40] = {
        "persist.vendor.radio.rilj_nw_type1",
        "persist.vendor.radio.rilj_nw_type2",
        "persist.vendor.radio.rilj_nw_type3",
        "persist.vendor.radio.rilj_nw_type4",
    };
    return getIntSysProp(PROPERTY_RILJ_NW_TYPE[slotId], 10);
}

void RpNwRatController::setIntSysProp(char *propName, int val) {
    char stgBuf[PROPERTY_VALUE_MAX] = { 0 };
    sprintf(stgBuf, "%d", val);
    property_set(propName, stgBuf);
}

int RpNwRatController::getIntSysProp(char *propName, int defaultVal) {
    int val = -1;
    char stgBuf[PROPERTY_VALUE_MAX] = { 0 };
    property_get(propName, stgBuf, "-1");
    val = strtol(stgBuf, NULL, 10);
    if (val == -1) {
        logD(RAT_CTRL_TAG, "[getIntSysProp] %s not exist, return %d",
                propName, defaultVal);
        val = defaultVal;
    }
    return val;
}

// if all slot has no SIM, this will return true.
bool RpNwRatController::isNoSimInserted() {
    for (int i = 0; i < RFX_SLOT_COUNT; i++) {
        int cardType = getStatusManager(i)->getIntValue(
                RFX_STATUS_KEY_CARD_TYPE, 0);
        logD(RAT_CTRL_TAG, "[isNoSimInserted] SIM%d cardType: %d", i, cardType);
        if (cardType > 0) {
            return false;
        }
    }
    return true;
}

int RpNwRatController::isNewSimCard(int slotId) {
    int result = -1;
    char lastTimeIccid[PROPERTY_VALUE_MAX] = {0};
    char currentIccid[PROPERTY_VALUE_MAX] = {0};

    char PROPERTY_ICCID_SIM[4][25] = {
        "vendor.ril.iccid.sim1",
        "vendor.ril.iccid.sim2",
        "vendor.ril.iccid.sim3",
        "vendor.ril.iccid.sim4",
    };
    char PROPERTY_LAST_BOOT_ICCID_SIM[4][40] = {
        "persist.vendor.radio.last_iccid_sim1",
        "persist.vendor.radio.last_iccid_sim2",
        "persist.vendor.radio.last_iccid_sim3",
        "persist.vendor.radio.last_iccid_sim4",
    };

    property_get(PROPERTY_ICCID_SIM[slotId], currentIccid, "");
    if (strlen(currentIccid) == 0 || strcmp("N/A", currentIccid) == 0) {
        logD(RAT_CTRL_TAG, "[isNewSimCard]:iccid not ready");
        result = 0;
    }
    property_get(PROPERTY_LAST_BOOT_ICCID_SIM[slotId], lastTimeIccid, "");
    if (strlen(lastTimeIccid) == 0 || strcmp("N/A", lastTimeIccid) == 0) {
        logD(RAT_CTRL_TAG, "[isNewSimCard]:first time boot-up");
        property_set(PROPERTY_LAST_BOOT_ICCID_SIM[slotId], currentIccid);
        result = 1;
    } else {
        if (strlen(currentIccid) == 0 && sIsInSwitching == true) {
            logD(RAT_CTRL_TAG, "[isNewSimCard]:Sim card is not ready in switcing");
            result = 0;
        } else if (strcmp(lastTimeIccid, currentIccid) == 0) {
            logD(RAT_CTRL_TAG, "[isNewSimCard]:Sim card is no change");
            result = 0;
        } else {
            logD(RAT_CTRL_TAG, "[isNewSimCard]:this is new Sim card");
            property_set(PROPERTY_LAST_BOOT_ICCID_SIM[slotId], currentIccid);
            result = 1;
        }
    }
    return result;
}

bool RpNwRatController::isEratExtSupport() {
    bool isEratExtSupport = false;
    char eratext[PROPERTY_VALUE_MAX] = { 0 };
    property_get("vendor.ril.nw.erat.ext.support", eratext, "-1");
    if (0 == strcmp(eratext, "1")) {
        isEratExtSupport = true;
    }
    return isEratExtSupport;
}

bool RpNwRatController::isWwop() {
    bool mode = false;
    char optr[PROPERTY_VALUE_MAX] = {0};
    property_get("persist.vendor.operator.optr", optr, "");
    if (strcmp(optr, "OP12") == 0 || strcmp(optr, "OP20") == 0) {
        mode = true;
    }
    return mode;
}

void RpNwRatController::onCardTypeChanged(RfxStatusKeyEnum key,
    RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    int old_type = old_value.asInt();
    int new_type = value.asInt();

    if (old_type != new_type && new_type == CARD_TYPE_NONE) {
        logD(RAT_CTRL_TAG, "onCardTypeChanged, old_type is %d, new_type is %d.", old_type,
        new_type);
        mPreferredNetWorkTypeFromRILJ = -1;
    }
}

int RpNwRatController::getPreferredNetWorkTypeFromRILJ() {
    // Use user preferred network type before RILJ send RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE
    int newSimCard = isNewSimCard(m_slot_id);
    if (mPreferredNetWorkTypeFromRILJ == -1) {
        // For Major slotid: SIM is inserted and is not a new sim card
        if (mCapabilitySlotId == m_slot_id && !isNoSimInserted() && !newSimCard) {
            logD(RAT_CTRL_TAG, "mPreferredNetWorkTypeFromRILJ is -1, From SysProp...");
            mPreferredNetWorkTypeFromRILJ = getPreferredNetWorkTypeFromSysProp(m_slot_id);
        }
    }

    logD(RAT_CTRL_TAG,
            "mPreferredNetWorkTypeFromRILJ=%d, mCurPreferedNetWorkType=%d, mNewPreferedNetWorkType=%d",
            mPreferredNetWorkTypeFromRILJ, mCurPreferedNetWorkType, mNewPreferedNetWorkType);
    return mPreferredNetWorkTypeFromRILJ;
}

RatSwitchCaller RpNwRatController::getRatSwitchCaller() {
    if (mNwRatSwitchHandler != NULL) {
        logD(RAT_CTRL_TAG, "getRatSwitchCaller(), NwsMode is %d. ",
                mNwRatSwitchHandler->getRatSwitchCaller());
        return mNwRatSwitchHandler->getRatSwitchCaller();
    } else {
        logD(RAT_CTRL_TAG, "getRatSwitchCaller(), mNwRatSwitchHandler has not "
                "been initialized ");
        return RAT_SWITCH_NORMAL;
    }
}

bool RpNwRatController::is4GNetworkMode(int prefNwType) {
    bool is4GNetworkMode = true;
    if (prefNwType != -1) {
        switch (prefNwType) {
            case PREF_NET_TYPE_LTE_CDMA_EVDO:
            case PREF_NET_TYPE_LTE_GSM_WCDMA:
            case PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA:
            case PREF_NET_TYPE_LTE_ONLY:
            case PREF_NET_TYPE_LTE_WCDMA:
            case PREF_NET_TYPE_TD_SCDMA_LTE:
            case PREF_NET_TYPE_TD_SCDMA_GSM_LTE:
            case PREF_NET_TYPE_TD_SCDMA_WCDMA_LTE:
            case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_LTE:
            case PREF_NET_TYPE_TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA:
            case PREF_NET_TYPE_LTE_GSM:
            case PREF_NET_TYPE_LTE_CDMA_EVDO_GSM:
                break;
            default:
                is4GNetworkMode = false;
                break;
        }
    }
    return is4GNetworkMode;
}

int RpNwRatController::getDefaultNetworkType() {
    if (mDefaultNetworkType == -1) {
        // Get default network type from  mode controller
        RpCdmaLteModeController *modeController = (RpCdmaLteModeController *) findController(-1,
                RFX_OBJ_CLASS_INFO(RpCdmaLteModeController));
        mDefaultNetworkType = modeController->getDefaultNetworkType(getSlotId());
    }

    return mDefaultNetworkType;
}

int RpNwRatController::getLastPreferredNetworkType() {
    // Get last prefered network type from pending rat switch record
    int prefNwType = mNewPreferedNetWorkType;
    if (mPendingNormalRatSwitchRecord.prefNwType != -1) {
        prefNwType = mPendingNormalRatSwitchRecord.prefNwType;
        logD(RAT_CTRL_TAG, "getLastPrefNwType, Has pending normal: update prefNwType from %d to %d",
                mNewPreferedNetWorkType, prefNwType);
    } else if (mPendingInitRatSwitchRecord.prefNwType != -1) {
        prefNwType = mPendingInitRatSwitchRecord.prefNwType;
        logD(RAT_CTRL_TAG, "getLastPrefNwType, Has pending init: update prefNwType from %d to %d",
                mNewPreferedNetWorkType, prefNwType);
    } else {
        logD(RAT_CTRL_TAG, "getLastPrefNwType, use mNewPreferedNetWorkType=%d",
                mNewPreferedNetWorkType);
    }
    return prefNwType;
}

VolteState RpNwRatController::getLastVolteState() {
    // Get last VolteState from pending rat switch record
    VolteState volteState = mNewVolteState;
    if (mPendingNormalRatSwitchRecord.prefNwType != -1) {
        volteState = mPendingNormalRatSwitchRecord.volteState;
        logD(RAT_CTRL_TAG, "getLastVolteState, Has pending normal: update volteState from %d to %d",
                mNewVolteState, volteState);
    } else if (mPendingInitRatSwitchRecord.prefNwType != -1) {
        volteState = mPendingInitRatSwitchRecord.volteState;
        logD(RAT_CTRL_TAG, "getLastVolteState, Has pending init: update volteState from %d to %d",
                mNewVolteState, volteState);
    } else {
        logD(RAT_CTRL_TAG, "getLastVolteState, use mNewVolteState=%d", mNewVolteState);
    }
    return volteState;
}

NwsMode RpNwRatController::getLastNwsMode() {
    // Get last NwsMode from pending rat switch record
    NwsMode nwsMode = mNewNwsMode;
    if (mPendingNormalRatSwitchRecord.prefNwType != -1) {
        nwsMode = mPendingNormalRatSwitchRecord.nwsMode;
        logD(RAT_CTRL_TAG, "getLastNwsMode, Has pending normal: update nwsMode from %d to %d",
                mNewNwsMode, nwsMode);
    } else if (mPendingInitRatSwitchRecord.prefNwType != -1) {
        nwsMode = mPendingInitRatSwitchRecord.nwsMode;
        logD(RAT_CTRL_TAG, "getLastNwsMode, Has pending init: update nwsMode from %d to %d",
                mNewNwsMode, nwsMode);
    } else {
        logD(RAT_CTRL_TAG, "getLastNwsMode, use mNewNwsMode=%d", mNewNwsMode);
    }
    return nwsMode;
}

NwsMode RpNwRatController::filterNwsMode(const AppFamilyType appFamilyType,
        const NwsMode nwsMode, const VolteState volteState) {
    NwsMode targetNwsMode = nwsMode;
    if (appFamilyType == APP_FAM_3GPP2) {
        if (volteState == VOLTE_OFF && nwsMode == NWS_MODE_LTEONLY) {
            // Turn off VoLTE, switch NwsMode to CDMALTE
            targetNwsMode = NWS_MODE_CDMALTE;
        } else if (volteState == VOLTE_ON && nwsMode == NWS_MODE_CDMALTE) {
            // Turn on VoLTE, switch NwsMode to LTEONLY
            targetNwsMode = NWS_MODE_LTEONLY;
        }
    } else {
        if (nwsMode == NWS_MODE_CDMALTE) {
            targetNwsMode = NWS_MODE_LTEONLY;
        }
    }

    logD(RAT_CTRL_TAG, "filterNwsMode, appFamilyType=%d, volteState=%d, nwsMode=%d, targetNwsMode=%d",
            appFamilyType, volteState, nwsMode, targetNwsMode);
    return targetNwsMode;
}

VolteState RpNwRatController::filterVolteState(const int prefNwType, const VolteState volteState,
        const int ratMode) {
    if (!RpFeatureOptionUtils::isCtVolteSupport() || isWwop()
            || getEnginenerMode() != ENGINEER_MODE_AUTO) {
        return VOLTE_OFF;
    }

    // [VoLTE] Get current VolteState from StatusManager
    VolteState newVolteState = volteState;
    int volteValue = getStatusManager()->getIntValue(RFX_STATUS_KEY_VOLTE_STATE, -1);
    if (volteValue == VOLTE_OFF) {
        newVolteState = VOLTE_OFF;
    } else if (volteValue == VOLTE_ON) {
        newVolteState = VOLTE_ON;
    }
    logD(RAT_CTRL_TAG, "filterVolteState, prefNwType=%d, ratMode=%d, volteState=%d, newVolteState=%d",
            prefNwType, ratMode, volteState, newVolteState);

    // [VoLTE] filter VolteState only for newVolteState is VOLTE_ON.
    if (newVolteState != VOLTE_ON) {
        return newVolteState;
    }

    // [VoLTE] For ct 3g dual mode card fix VolteState to VOLTE_OFF
    bool is3gCdmaSim = getStatusManager()->getBoolValue(RFX_STATUS_KEY_CT3G_DUALMODE_CARD, false);
    if (newVolteState == VOLTE_ON && is3gCdmaSim) {
        logD(RAT_CTRL_TAG, "filterVolteState, For CT3G fix VolteState to OFF");
        newVolteState = VOLTE_OFF;
    }

    // [VoLTE] For ECC fix VolteState to VOLTE_OFF
    if (newVolteState == VOLTE_ON
            && (RAT_MODE_CDMA_EVDO_AUTO == ratMode || RAT_MODE_LTE_WCDMA_GSM == ratMode)) {
        newVolteState = VOLTE_OFF;
    }

    // [VoLTE] For Not CT Cdma card to VOLTE_OFF

    int cdmaCardType = getStatusManager()->getIntValue(RFX_STATUS_KEY_CDMA_CARD_TYPE, UNKOWN_CARD);
    if (newVolteState == VOLTE_ON && cdmaCardType == NOT_CT_UICC_CARD) {
        logD(RAT_CTRL_TAG, "filterVolteState, For Not CT Cdma card fix VolteState to OFF");
        newVolteState = VOLTE_OFF;
    }

    // [VoLTE] For not 4G preferred network type fix VolteState to VOLTE_OFF
    if (newVolteState == VOLTE_ON && !is4GNetworkMode(prefNwType)) {
        newVolteState = VOLTE_OFF;
    }

    return newVolteState;
}

int RpNwRatController::calculateVolteNetworkType(const AppFamilyType appFamilyType,
        const NwsMode nwsMode, const VolteState volteState) {
    int prefNwType = PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA;
    getPreferredNetWorkTypeFromRILJ();
    if (mPreferredNetWorkTypeFromRILJ != -1) {
        // Has users settings
        prefNwType = mPreferredNetWorkTypeFromRILJ;
    } else {
        // Get VoLTE default network type
        if (appFamilyType == APP_FAM_3GPP2) {
            if (volteState == VOLTE_ON) {
                if (nwsMode == NWS_MODE_CDMALTE || nwsMode == NWS_MODE_LTEONLY) {
                    prefNwType = PREF_NET_TYPE_LTE_ONLY;
                } else if (nwsMode == NWS_MODE_CSFB) {
                    prefNwType = PREF_NET_TYPE_LTE_GSM_WCDMA;
                }
            } else if (volteState == VOLTE_OFF) {
                if (nwsMode == NWS_MODE_CDMALTE || nwsMode == NWS_MODE_LTEONLY) {
                    prefNwType = PREF_NET_TYPE_LTE_CDMA_EVDO;
                } else if (nwsMode == NWS_MODE_CSFB) {
                    prefNwType = PREF_NET_TYPE_LTE_GSM_WCDMA;
                }
            }
        } else {
            if (nwsMode == NWS_MODE_CDMALTE || nwsMode == NWS_MODE_LTEONLY) {
                prefNwType = PREF_NET_TYPE_LTE_ONLY;
            } else {
                prefNwType = PREF_NET_TYPE_LTE_GSM_WCDMA;
            }
        }
    }

    // Get default network type from  mode controller
    int capability = getDefaultNetworkType();

    // Filter preferred network type by capability
    int targetPrefNwType = filterPrefNwType(appFamilyType, prefNwType, capability);

    logD(RAT_CTRL_TAG, "calculateVolteNetworkType, to setPreferredNetworkType: "
            "nwsMode=%d, capability=%d, prefNwType=%d, targetPrefNwType=%d", nwsMode, capability,
            prefNwType, targetPrefNwType);
    return targetPrefNwType;
}

void RpNwRatController::onVolteStateChanged(RfxStatusKeyEnum key, RfxVariant old_value,
        RfxVariant value) {
    RFX_UNUSED(key);
    int oldType = old_value.asInt();
    int newType = value.asInt();

    if (getEnginenerMode() == ENGINEER_MODE_AUTO) {
        if (oldType == newType) {
            return;
        }

        // Get volteState
        VolteState volteState = mNewVolteState;
        if (newType == VOLTE_OFF) {
            volteState = VOLTE_OFF;
        } else if (newType == VOLTE_ON) {
            volteState = VOLTE_ON;
        } else {
            volteState = VOLTE_OFF;
        }
        logD(RAT_CTRL_TAG,
                "onVolteStateChanged, mCurVolteState=%d, mNewVolteState=%d, Change to: %d",
                mCurVolteState, mNewVolteState, volteState);

        AppFamilyType appFamilyType = mNewAppFamilyType;
        int cdmaCardType = getStatusManager()->getIntValue(RFX_STATUS_KEY_CDMA_CARD_TYPE, UNKOWN_CARD);
        if (cdmaCardType == CT_4G_UICC_CARD && appFamilyType == APP_FAM_3GPP2) {
            int prefNwTypeDefault = getLastPreferredNetworkType();
            if (!is4GNetworkMode(prefNwTypeDefault)
                    && (appFamilyType == APP_FAM_3GPP2 && needConfigRatModeForCtDualVolte())) {
                if (mPreferredNetWorkTypeFromRILJ != -1) {
                    prefNwTypeDefault = mPreferredNetWorkTypeFromRILJ;
                } else {
                    prefNwTypeDefault = getDefaultNetworkType();
                }
            }

            volteState = filterVolteState(prefNwTypeDefault, volteState, -1);
            NwsMode lastNwsMode = getLastNwsMode();
            NwsMode nwsMode = filterNwsMode(appFamilyType, lastNwsMode, volteState);
            int prefNwType = calculateVolteNetworkType(appFamilyType, nwsMode, volteState);
            switchNwRat(appFamilyType, prefNwType, nwsMode, volteState, RAT_SWITCH_VOLTE, NULL,
                    NULL);
        } else {
            mNewVolteState = volteState;
            mCurVolteState = volteState;
        }
    }
}

void RpNwRatController::onCTVolteCardTypeChanged(RfxStatusKeyEnum key, RfxVariant old_value,
        RfxVariant value) {
    RFX_UNUSED(key);
    int oldType = old_value.asInt();
    int newType = value.asInt();
    logD(RAT_CTRL_TAG, "onCTVolteCardTypeChanged, old: %d, new: %d", oldType, newType);

    if (getEnginenerMode() == ENGINEER_MODE_AUTO) {
        if (oldType == newType || newType != NOT_CT_UICC_CARD) {
            logD(RAT_CTRL_TAG, "onCTVolteCardTypeChanged ignore");
            return;
        }

        AppFamilyType appFamilyType = mNewAppFamilyType;
        if (appFamilyType == APP_FAM_3GPP2) {
            int volteValue = getStatusManager()->getIntValue(RFX_STATUS_KEY_VOLTE_STATE, -1);
            if (volteValue == VOLTE_ON && getLastVolteState() == VOLTE_ON) {
                logD(RAT_CTRL_TAG, "onCTVolteCardTypeChanged, switch VOLTE_OFF: NOT_CT_UICC_CARD");
                VolteState volteState = VOLTE_OFF;
                NwsMode lastNwsMode = getLastNwsMode();
                NwsMode nwsMode = filterNwsMode(appFamilyType, lastNwsMode, volteState);
                int prefNwType = calculateVolteNetworkType(appFamilyType, nwsMode, volteState);
                switchNwRat(appFamilyType, prefNwType, nwsMode, volteState, RAT_SWITCH_VOLTE, NULL,
                        NULL);
            }
        } else if (appFamilyType == APP_FAM_3GPP && needConfigRatModeForCtDualVolte()) {
            // For CT VoLTE C+C Case: CDMA 4G card is in side slot, but AppFamilyType is 3GPP
            logD(RAT_CTRL_TAG, "onCTVolteCardTypeChanged, switch VOLTE_OFF: NOT_CT_UICC_CARD");
            VolteState volteState = VOLTE_OFF;
            NwsMode nwsMode = NWS_MODE_CSFB;
            int prefNwType = calculateVolteNetworkType(appFamilyType, nwsMode, volteState);
            if (is4GNetworkMode(prefNwType)) {
                switchNwRat(appFamilyType, prefNwType, nwsMode, volteState, RAT_SWITCH_VOLTE, NULL,
                        NULL);
            }
        }
    }
}

bool RpNwRatController::isCtDualVolteSupport() {
    return RpFeatureOptionUtils::isCtVolteSupport()
        && RpFeatureOptionUtils::isMultipleImsSupport();
}

bool RpNwRatController::needConfigRatModeForCtDualVolte() {
    int cdmaCardType = getStatusManager()->getIntValue(RFX_STATUS_KEY_CDMA_CARD_TYPE, UNKOWN_CARD);
    // 1. CT Dual Volte Support;
    // 2. Not main capability slot;
    // 3. mCardType is 4G Cdma card, but cdmaCardType is not Non-CT 4G Cdma card,
    // or cdmaCardType is CT 4G Cdma card, need follow CT dual volte flow.
    bool result = isCtDualVolteSupport() && getCapabilitySlotId() != m_slot_id
            && ((is4GCdmaCard(mCardType) && cdmaCardType != NOT_CT_UICC_CARD)
                    || cdmaCardType == CT_4G_UICC_CARD);
    return result;
}

bool RpNwRatController::is4GCdmaCard(int cardType) {
    if (containsUsim(cardType) && containsCdma(cardType)) {
        return true;
    }
    return false;
}

bool RpNwRatController::containsCdma(int cardType) {
    if ((cardType & CARD_TYPE_RUIM) > 0 || (cardType & CARD_TYPE_CSIM) > 0) {
        return true;
    }
    return false;
}

bool RpNwRatController::containsUsim(int cardType) {
    if ((cardType & CARD_TYPE_USIM) > 0) {
        return true;
    }
    return false;
}

void RpNwRatController::onVoiceCallCountChanged(int slotId, RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(slotId);
    RFX_UNUSED(key);
    int oldMode = old_value.asInt();
    int mode = value.asInt();
    if (mode == 0 && oldMode > 0) {
        if (isInCall() == false && hasPendingVoLTERecord()) {
            logD(RAT_CTRL_TAG, "onVoiceCallCountChanged, doPendingRatSwitchRecord...");
            doPendingRatSwitchRecord();
        }
    }
}

bool RpNwRatController::hasPendingVoLTERecord() {
    if (mPendingNormalRatSwitchRecord.prefNwType != -1
            && mPendingNormalRatSwitchRecord.ratSwitchCaller == RAT_SWITCH_VOLTE) {
        logD(RAT_CTRL_TAG, "hasPendingVoLTERecord, ret=true");
        return true;
    }
    return false;
}

int RpNwRatController::isInCall() {
    int ret = false;
    for (int slotId = RFX_SLOT_ID_0; slotId < RFX_SLOT_COUNT; slotId++) {
        if (getStatusManager(slotId)->getIntValue(RFX_STATUS_KEY_VOICE_CALL_COUNT, 0) > 0) {
            ret = true;
            break;
        }
    }
    return ret;
}

void RpNwRatController::handleRatSwitchResult(const NwsMode nwsMode, const int prefNwType,
        const VolteState volteState, const RatSwitchCaller ratSwitchCaller,
        const RatSwitchResult switchResult, const int errorCode) {
    logD(RAT_CTRL_TAG, "handleRatSwitchResult, nwsMode=%d, prefNwType=%d, volteState=%d,"
            " ratSwitchCaller=%d, switchResult=%d, errorCode=%d", nwsMode, prefNwType, volteState,
            ratSwitchCaller, switchResult, errorCode);

    // If execute ERAT fail in call, set the key value to fail, other case, set to success.
    if (switchResult == RAT_SWITCH_FAIL && errorCode == RIL_E_OPERATION_NOT_ALLOWED) {
        // Handle send erat fail by call for VoLTE
        AppFamilyType appFamilyType = mNewAppFamilyType;
        int cdmaCardType = getStatusManager()->getIntValue(RFX_STATUS_KEY_CDMA_CARD_TYPE, UNKOWN_CARD);
        if (ratSwitchCaller == RAT_SWITCH_VOLTE
                && cdmaCardType == CT_4G_UICC_CARD
                && appFamilyType == APP_FAM_3GPP2) {
            // QueueRatSwitchRecord VoLTE switch
            int prefNwTypeDefault = getLastPreferredNetworkType();
            if (!is4GNetworkMode(prefNwTypeDefault)
                    && (appFamilyType == APP_FAM_3GPP2 && needConfigRatModeForCtDualVolte())) {
                if (mPreferredNetWorkTypeFromRILJ != -1) {
                    prefNwTypeDefault = mPreferredNetWorkTypeFromRILJ;
                } else {
                    prefNwTypeDefault = getDefaultNetworkType();
                }
            }

            VolteState newVolteState = filterVolteState(prefNwTypeDefault, volteState, -1);
            NwsMode lastNwsMode = getLastNwsMode();
            NwsMode newNwsMode = filterNwsMode(appFamilyType, lastNwsMode, newVolteState);
            int newPrefNwType = calculateVolteNetworkType(appFamilyType, newNwsMode, newVolteState);
            logD(RAT_CTRL_TAG, "handleRatSwitchResult, queueRatSwitchRecord for VoLTE: "
                    "appFamilyType=%d, nwsMode=%d, prefNwType=%d, volteState=%d", appFamilyType,
                    newNwsMode, newPrefNwType, newVolteState);
            queueRatSwitchRecord(appFamilyType, newPrefNwType, newNwsMode, newVolteState,
                    RAT_SWITCH_VOLTE, NULL, NULL);
        }

        getStatusManager()->setIntValue(RFX_STATUS_KEY_RAT_SWITCH_STATUS, 0);
    } else {
        getStatusManager()->setIntValue(RFX_STATUS_KEY_RAT_SWITCH_STATUS, 1);
    }
}
