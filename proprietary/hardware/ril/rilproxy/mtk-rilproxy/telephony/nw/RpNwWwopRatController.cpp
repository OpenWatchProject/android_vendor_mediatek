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
#include "RpNwWwopRatController.h"
#include "modecontroller/RpCdmaLteDefs.h"
#include "util/RpFeatureOptionUtils.h"
#include "RfxLog.h"
#include "RfxStatusDefs.h"
#include <telephony/mtk_ril.h>

#define RAT_CTRL_TAG "RpNwWwopRatCtrl"

/*****************************************************************************
 * Class RpNwWwopRatController
 *****************************************************************************/

RFX_IMPLEMENT_CLASS("RpNwWwopRatController", RpNwWwopRatController, RfxController);

bool RpNwWwopRatController::sIsInSwitching = false;

RpNwWwopRatController::RpNwWwopRatController() :
    mCurAppFamilyType(APP_FAM_UNKNOWN),
    mCurPreferedNetWorkType(-1),
    mCurNwsMode(NWS_MODE_CSFB),
    mDesiredNetWorkTypeFromRILJ(-1),
    mPreferredNetWorkTypeFromRILJ(-1),
    mNewAppFamilyType(APP_FAM_UNKNOWN),
    mNewPreferedNetWorkType(-1),
    mNewNwsMode(NWS_MODE_CSFB),
    mCtrlAction(ACT_RAT_SWITCH_BEGIN),
    mSugggestedResetRadioCapablity(-1),
    mAction(NULL),
    mMessage(NULL),
    mRestrictedCount(0) {
}

RpNwWwopRatController::~RpNwWwopRatController() {
}

void RpNwWwopRatController::onInit() {
    // Required: invoke super class implementation
    RfxController::onInit();

    logD(RAT_CTRL_TAG, "onInit");

    // define and register request & urc id list
    const int request_id_list[] = {
        RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE,
        RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE,
        RIL_LOCAL_C2K_REQUEST_CONFIG_EVDO_MODE,
        RIL_REQUEST_VOICE_RADIO_TECH,
        RIL_LOCAL_REQUEST_SET_SVLTE_RAT_MODE,
        RIL_LOCAL_C2K_REQUEST_SET_SVLTE_RAT_MODE
    };
    const int urc_id_list[] = {};

    if (RpFeatureOptionUtils::isC2kSupport()) {
        registerToHandleRequest(request_id_list, sizeof(request_id_list)/sizeof(int));
        registerToHandleUrc(urc_id_list, 0);
    }

    getStatusManager()->setIntValue(RFX_STATUS_KEY_NWS_MODE, mCurNwsMode);
    getStatusManager()->setIntValue(RFX_STATUS_KEY_PREFERRED_NW_TYPE, mCurPreferedNetWorkType);
    getStatusManager()->setBoolValue(RFX_STATUS_KEY_IS_RAT_MODE_SWITCHING, false);

    mPendingRestrictedRatSwitchRecord.prefNwType = -1;
    mPendingInitRatSwitchRecord.prefNwType = -1;
    mPendingNormalRatSwitchRecord.prefNwType = -1;
    mRpRadioController = (RpRadioController *) findController(
            RFX_OBJ_CLASS_INFO(RpRadioController));

    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_EMERGENCY_MODE,
            RfxStatusChangeCallback(this, &RpNwWwopRatController::onRestrictedModeChanged));
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_EMERGENCY_CALLBACK_MODE,
            RfxStatusChangeCallback(this, &RpNwWwopRatController::onRestrictedModeChanged));
}

void RpNwWwopRatController::onDeinit() {
    RfxController::onDeinit();
}

bool RpNwWwopRatController::onHandleRequest(const sp<RfxMessage> &message) {
    int msg_id = message->getId();
    int engineerMode;
    sp<RfxMessage> resToRilj;

    switch (msg_id) {
        case RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE: {
            engineerMode = getEnginenerMode();
            int32_t stgCount;
            int32_t nwType = 0;
            message->getParcel()->readInt32(&stgCount);
            message->getParcel()->readInt32(&nwType);
            // Add for compatible of 93MD.
            if (nwType > PREF_NET_TYPE_LTE_TDD_ONLY) {
                logD(RAT_CTRL_TAG, "convert new added networktype");
                if (nwType == PREF_NET_TYPE_CDMA_GSM) {
                    if (mCurAppFamilyType == APP_FAM_3GPP2) {
                        nwType = PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO;
                    } else {
                        nwType = PREF_NET_TYPE_GSM_ONLY;
                    }
                } else if (nwType == PREF_NET_TYPE_CDMA_EVDO_GSM) {
                    if (mCurAppFamilyType == APP_FAM_3GPP2) {
                        nwType = PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO;
                    } else {
                        nwType = PREF_NET_TYPE_GSM_ONLY;
                    }
                } else if (nwType == PREF_NET_TYPE_LTE_CDMA_EVDO_GSM) {
                    if (mCurAppFamilyType == APP_FAM_3GPP2) {
                        nwType = PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA;
                    } else {
                        nwType = PREF_NET_TYPE_LTE_GSM;
                    }
                }
            }

            mDesiredNetWorkTypeFromRILJ = nwType;
            // there is no sim, so ignore it
            if (getStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE, 0) == CARD_TYPE_NONE) {
                logD(RAT_CTRL_TAG, "onHandleRequest: return directly because no sim.");
                sp<RfxMessage> resToRilj = RfxMessage::obtainResponse(
                        RIL_E_SUCCESS, message);
                responseToRilj(resToRilj);
                return true;
            }
            if (engineerMode == ENGINEER_MODE_AUTO) {
                logD(RAT_CTRL_TAG, "[setPrefNwType] nwType:%d", nwType);
                switchNwRat(mCurAppFamilyType, nwType, mCurNwsMode,
                        RAT_SWITCH_NORMAL, NULL, message);
            } else {
                if (engineerMode == ENGINEER_MODE_CDMA) {
                    if (nwType == PREF_NET_TYPE_CDMA_ONLY
                            || nwType == PREF_NET_TYPE_EVDO_ONLY) {
                        logD(RAT_CTRL_TAG, "[setPrefNwType] ENG mode, switch to %d", nwType);
                        switchNwRat(mCurAppFamilyType, nwType, mCurNwsMode,
                                RAT_SWITCH_NORMAL, NULL, message);
                        return true;
                    }
                }
                logD(RAT_CTRL_TAG, "[setPrefNwType] ENG mode, switch not executed!");
                responseSetPreferredNetworkType(RIL_E_GENERIC_FAILURE, message);
            }
            return true;
        }
        case RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE:
            handleGetPreferredNwType(message);
            return true;
        case RIL_REQUEST_VOICE_RADIO_TECH:
            logD(RAT_CTRL_TAG, "mCurNwsMode: %d, sIsInSwitching: %s", mCurNwsMode,
                    (sIsInSwitching ? "true" : "false"));
            if (sIsInSwitching) {
                resToRilj = RfxMessage::obtainResponse(RIL_E_SUCCESS, message);
                resToRilj->getParcel()->writeInt32(1);
                resToRilj->getParcel()->writeInt32(RADIO_TECH_UNKNOWN);
                responseToRilj(resToRilj);
                return true;
            }
            if (mCurNwsMode == NWS_MODE_CSFB) {
                sp<RfxMessage> msg = RfxMessage::obtainRequest(RADIO_TECH_GROUP_GSM,
                        RIL_REQUEST_VOICE_RADIO_TECH, message);
                requestToRild(msg);
            } else if (mCurNwsMode == NWS_MODE_CDMALTE) {
                resToRilj = RfxMessage::obtainResponse(RIL_E_SUCCESS, message);
                resToRilj->getParcel()->writeInt32(1);
                resToRilj->getParcel()->writeInt32(RADIO_TECH_1xRTT);
                responseToRilj(resToRilj);
            } else {
                resToRilj = RfxMessage::obtainResponse(RIL_E_SUCCESS, message);
                resToRilj->getParcel()->writeInt32(1);
                resToRilj->getParcel()->writeInt32(RADIO_TECH_UNKNOWN);
                responseToRilj(resToRilj);
            }
            return true;
    }
    return false;
}

void RpNwWwopRatController::handleGetPreferredNwType(const sp<RfxMessage> &message) {
    sp<RfxMessage> reqToRild;
    sp<RfxMessage> resToRilj;
    RILD_RadioTechnology_Group radioTech = RADIO_TECH_GROUP_GSM;

    logD(RAT_CTRL_TAG, "[handleGetPreferredNwType] mPreferredNetWorkTypeFromRILJ:%d",
                    mPreferredNetWorkTypeFromRILJ);
    if (mRestrictedCount > 0 &&
        mPendingNormalRatSwitchRecord.prefNwType != -1) {
        logD(RAT_CTRL_TAG, "[handleGetPreferredNwType] in Restricted mode:%d",
                mPendingNormalRatSwitchRecord.prefNwType);
        resToRilj = RfxMessage::obtainResponse(RIL_E_SUCCESS, message);
        resToRilj->getParcel()->writeInt32(1);
        resToRilj->getParcel()->writeInt32(mPendingNormalRatSwitchRecord.prefNwType);
        responseToRilj(resToRilj);
        return;
    } else if (sIsInSwitching == true) {
        logD(RAT_CTRL_TAG, "[handleGetPreferredNwType] in RAT switching Desired:%d",
                mDesiredNetWorkTypeFromRILJ);
        resToRilj = RfxMessage::obtainResponse(RIL_E_SUCCESS, message);
        resToRilj->getParcel()->writeInt32(1);
        resToRilj->getParcel()->writeInt32(mDesiredNetWorkTypeFromRILJ);
        responseToRilj(resToRilj);
        return;
    } else {
        switch(mPreferredNetWorkTypeFromRILJ) {
            // LWCG global mode
            case PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA:
            // WCG global mode
            case PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO:
            // LC mode
            case PREF_NET_TYPE_LTE_CDMA_EVDO:
            // TDS-CDMA case
            case PREF_NET_TYPE_TD_SCDMA_ONLY:
            case PREF_NET_TYPE_TD_SCDMA_WCDMA:
            case PREF_NET_TYPE_TD_SCDMA_LTE:
            case PREF_NET_TYPE_TD_SCDMA_GSM:
            case PREF_NET_TYPE_TD_SCDMA_GSM_LTE:
            case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA:
            case PREF_NET_TYPE_TD_SCDMA_WCDMA_LTE:
            case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_LTE:
            case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO:
            case PREF_NET_TYPE_TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA:
                logD(RAT_CTRL_TAG, "[handleGetPreferredNwType] directly return type:%d",
                        mPreferredNetWorkTypeFromRILJ);
                resToRilj = RfxMessage::obtainResponse(RIL_E_SUCCESS, message);
                resToRilj->getParcel()->writeInt32(1);
                resToRilj->getParcel()->writeInt32(mPreferredNetWorkTypeFromRILJ);
                responseToRilj(resToRilj);
                return;

            case PREF_NET_TYPE_CDMA_EVDO_AUTO:
            case PREF_NET_TYPE_CDMA_ONLY:
            case PREF_NET_TYPE_EVDO_ONLY:
                if (getMajorSlotId() != m_slot_id) {
                    logD(RAT_CTRL_TAG, "[handleGetPreferredNwType] directly return type:%d",
                            mPreferredNetWorkTypeFromRILJ);
                    resToRilj = RfxMessage::obtainResponse(RIL_E_SUCCESS, message);
                    resToRilj->getParcel()->writeInt32(1);
                    resToRilj->getParcel()->writeInt32(mPreferredNetWorkTypeFromRILJ);
                    responseToRilj(resToRilj);
                    return;
                }
                radioTech = RADIO_TECH_GROUP_C2K;
                break;

            case PREF_NET_TYPE_LTE_GSM_WCDMA:
            case PREF_NET_TYPE_GSM_WCDMA:
            case PREF_NET_TYPE_GSM_ONLY:
            case PREF_NET_TYPE_WCDMA:
            case PREF_NET_TYPE_GSM_WCDMA_AUTO:
            case PREF_NET_TYPE_LTE_ONLY:
            case PREF_NET_TYPE_LTE_WCDMA:
            case PREF_NET_TYPE_LTE_GSM:
            case PREF_NET_TYPE_LTE_TDD_ONLY:
                radioTech = RADIO_TECH_GROUP_GSM;
                break;
            default:
                logD(RAT_CTRL_TAG, "[handleGetPreferredNwType] RILJ never request prefNwType!");
                break;
        }
    }
    logD(RAT_CTRL_TAG, "[handleGetPreferredNwType] request to %s",
            sourceToString(radioTech));
    reqToRild = RfxMessage::obtainRequest(radioTech,
            RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE, message);
    requestToRild(reqToRild);
}
void RpNwWwopRatController::setRestrictedNetworkMode(const int prefNwType,
        const sp<RfxAction>& action) {
    if (getEnginenerMode() == ENGINEER_MODE_AUTO) {
        if (prefNwType == -1) {
            logD(RAT_CTRL_TAG, "[setRestrictedNetworkMode] leaving restricted mode");
            mPendingRestrictedRatSwitchRecord.prefNwType = -1;
            int prefNwType = -1;
            if (mPendingInitRatSwitchRecord.prefNwType != -1) {
                logD(RAT_CTRL_TAG, "[setRestrictedNetworkMode] Init pending record in queue");
                prefNwType = mPendingInitRatSwitchRecord.prefNwType;
                mPendingInitRatSwitchRecord.prefNwType = -1;
                switchNwRat(mPendingInitRatSwitchRecord.appFamType,
                        prefNwType,
                        mPendingInitRatSwitchRecord.nwsMode,
                        RAT_SWITCH_RESTRICT, action, NULL);
            } else if (mPendingNormalRatSwitchRecord.prefNwType != -1) {
                logD(RAT_CTRL_TAG, "[setRestrictedNetworkMode] Norm pending record in queue");
                prefNwType = mPendingNormalRatSwitchRecord.prefNwType;
                mPendingNormalRatSwitchRecord.prefNwType = -1;
                switchNwRat(mPendingNormalRatSwitchRecord.appFamType,
                        prefNwType,
                        mPendingNormalRatSwitchRecord.nwsMode,
                        RAT_SWITCH_RESTRICT, action, NULL);
            } else {
                logD(RAT_CTRL_TAG, "[setRestrictedNetworkMode] No pending record in queue");
                action->act();
                doPendingRatSwitchRecord();
            }
        } else {
            logD(RAT_CTRL_TAG, "[setRestrictedNetworkMode] entering restricted mode: %d",
                    prefNwType);
            if (mPendingInitRatSwitchRecord.prefNwType == -1) {
                // save current state to pending queue
                queueRatSwitchRecord(mCurAppFamilyType, mCurPreferedNetWorkType, mCurNwsMode,
                        RAT_SWITCH_INIT, NULL, NULL);
            }
            if (isCdmaDualModeSimCard() == true ||
                        isCdmaOnlySim() == true ||
                    // None slot need set AppFamilyType to 3GPP2 for LC networkType to ensure
                    // RIL_REQUEST_VOICE_RADIO_TECH can query 1xRTT.
                    (isNoSimInserted() && prefNwType == PREF_NET_TYPE_LTE_CDMA_EVDO)) {
                switchNwRat(APP_FAM_3GPP2, prefNwType, mCurNwsMode, RAT_SWITCH_RESTRICT, action, NULL);
            } else {
                switchNwRat(APP_FAM_3GPP, prefNwType, mCurNwsMode, RAT_SWITCH_RESTRICT, action, NULL);
            }
        }
    } else {
        logD(RAT_CTRL_TAG, "[setRestrictedNetworkMode] ENG mode, switch not executed!");
        action->act();
    }
}

int RpNwWwopRatController::selectNwTypeBySlot(int prefNwType) {
    int newPrefNwType = prefNwType;
    bool is3gCdmaSim = getStatusManager()->getBoolValue(
            RFX_STATUS_KEY_CT3G_DUALMODE_CARD, false);

    if (isNoSimInserted()) {
        // No SIM inserted
        if (mPreferredNetWorkTypeFromRILJ != -1) {
            // user pref rat is received
            newPrefNwType = mPreferredNetWorkTypeFromRILJ;
        }
    } else {
        // SIM is inserted
        int mainSlot = getIntSysProp("persist.vendor.radio.simswitch", 1) - 1;
        if (mainSlot == m_slot_id) {
            if (isNewSimCard(m_slot_id)) {
                // filter C2K if no cdma capability in main slot
                setPreferredNetWorkTypeToSysProp(m_slot_id, -1);
                logD(RAT_CTRL_TAG, "New sim inserted, clear the sim card system property. ");
                if (getCdmaSlotId() == m_slot_id) {
                    mPreferredNetWorkTypeFromRILJ = PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA;
                    return PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA;
                } else {
                    mPreferredNetWorkTypeFromRILJ = PREF_NET_TYPE_LTE_GSM_WCDMA;
                    return PREF_NET_TYPE_LTE_GSM_WCDMA;
                }
            } else {
                mPreferredNetWorkTypeFromRILJ = getPreferredNetWorkTypeFromSysProp(m_slot_id);
            }
        } else {
            if (isNewSimCard(m_slot_id)) {
                if (getCdmaSlotId() == m_slot_id) {
                    return PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO;
                } else {
                    return newPrefNwType;
                }
            } else {
                if (getCdmaSlotId() != m_slot_id) {
                    return newPrefNwType;
                }
            }
        }
        logD(RAT_CTRL_TAG, "[setAppFamilyType] newPrefNwType: %d, mPreferredNetWorkTypeFromRILJ: %d",
                newPrefNwType, mPreferredNetWorkTypeFromRILJ);
        if (mPreferredNetWorkTypeFromRILJ != -1 && (mainSlot == m_slot_id
                        || (mPreferredNetWorkTypeFromRILJ != PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA
                        && mPreferredNetWorkTypeFromRILJ != PREF_NET_TYPE_LTE_CDMA_EVDO))) {
            // user pref rat is received
            newPrefNwType = mPreferredNetWorkTypeFromRILJ;
        } else {
            // never receive user pref rat, ex: device just boot-up
            if (newPrefNwType == PREF_NET_TYPE_LTE_CDMA_EVDO) {
                // For main card, change LC to LWCG mode
                if (mainSlot == m_slot_id) {
                    newPrefNwType = PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA;
                } else {
                // For non-main card, change LC to C mode
                    newPrefNwType = PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO;
                }
            } else if (newPrefNwType == PREF_NET_TYPE_CDMA_EVDO_AUTO ||
                               newPrefNwType == PREF_NET_TYPE_CDMA_ONLY ||
                               newPrefNwType == PREF_NET_TYPE_EVDO_ONLY) {
                // change C to WCG mode
                newPrefNwType = PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO;
            } else {
                if (is3gCdmaSim) {
                    if (newPrefNwType == PREF_NET_TYPE_GSM_WCDMA
                            || newPrefNwType == PREF_NET_TYPE_LTE_GSM_WCDMA) {
                        if (getCdmaSlotId() == m_slot_id) {
                            // change WG to WCG mode
                            newPrefNwType = PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO;
                        }
                    }
                }
            }
        }
    }
    return newPrefNwType;
}

void RpNwWwopRatController::setAppFamilyType(const AppFamilyType appFamType,
        const int prefNwType, int cardState, const sp<RfxAction> &action) {
    logD(RAT_CTRL_TAG, "[setAppFamilyType] appFamType: %d, "
            "prefNwType: %d, cardState: %d", appFamType, prefNwType, cardState);
    // add for ECC both card no sim inserted
    if (cardState == CARD_STATE_NO_CHANGED && !isNoSimInserted() &&
            appFamType == mCurAppFamilyType) {
        logD(RAT_CTRL_TAG, "Card state not changed, ignore switch request");
        if (action != NULL) {
            action->act();
        }
        return;
    }
    mCurAppFamilyType = appFamType;
    int newPrefNwType = selectNwTypeBySlot(prefNwType);
    NwsMode nwsMode = NWS_MODE_CDMALTE;
    if (cardState == CARD_STATE_CARD_TYPE_CHANGED) {
        if (isCdmaDualModeSimCard() == true &&
            (newPrefNwType == PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO ||
            newPrefNwType == PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA)) {
            nwsMode = (appFamType == APP_FAM_3GPP) ? NWS_MODE_CSFB : NWS_MODE_CDMALTE;
        } else {
            nwsMode = mCurNwsMode;
            if (sIsInSwitching == true) {
                nwsMode = (appFamType == APP_FAM_3GPP) ? NWS_MODE_CSFB : NWS_MODE_CDMALTE;
            }
        }
    }
    if (isNoSimInserted()) {
        // No SIM inserted
        nwsMode = mCurNwsMode;
    }

    if (appFamType == APP_FAM_3GPP) {
        switchNwRat(appFamType, newPrefNwType, NWS_MODE_CSFB, RAT_SWITCH_INIT, action, NULL);
    } else {
        if (getEnginenerMode() == ENGINEER_MODE_AUTO) {
            if (mPendingNormalRatSwitchRecord.prefNwType != -1 &&
                isCdmaDualModeSimCard() != true &&
                newPrefNwType != PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO &&
                newPrefNwType != PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA) {
                switchNwRat(appFamType, newPrefNwType,
                        mPendingNormalRatSwitchRecord.nwsMode, RAT_SWITCH_INIT,
                        action, NULL);
            } else {
                switchNwRat(appFamType, newPrefNwType, nwsMode, RAT_SWITCH_INIT, action, NULL);
            }
        } else {
            doNwSwitchForEngMode(action);
        }
    }
}

void RpNwWwopRatController::setNwsMode(const NwsMode nwsMode, const sp<RfxAction> &action) {
    logD(RAT_CTRL_TAG, "[setNwsMode] nwsMode: %d", nwsMode);
    if (getEnginenerMode() == ENGINEER_MODE_AUTO) {
        if (nwsMode != mCurNwsMode) {
            switchNwRat(mCurAppFamilyType, mCurPreferedNetWorkType, nwsMode, RAT_SWITCH_NWS,
                    action, NULL);
        } else {
            logD(RAT_CTRL_TAG, "[setNwsMode] switch not executed");
        }
    } else {
        logD(RAT_CTRL_TAG, "[setNwsMode] ENG mode, switch not executed!");
    }
}

void RpNwWwopRatController::switchNwRat(const AppFamilyType appFamType,
        int prefNwType, NwsMode nwsMode, const RatSwitchCaller ratSwitchCaller,
        const sp<RfxAction> &action, const sp<RfxMessage> &message) {
    logD(RAT_CTRL_TAG, "[switchNwRat] appFamType: %d, prefNwType: %d, nwsMode: %d"
            " ratSwitchCaller: %s, sIsInSwitching: %s",
            appFamType, prefNwType, nwsMode, switchCallerToString(ratSwitchCaller),
            sIsInSwitching ? "true" : "false");

    if (sIsInSwitching) {
        queueRatSwitchRecord(appFamType, prefNwType, nwsMode, ratSwitchCaller, action, message);
    } else {
        sp<RfxMessage> resToRilj;
        if (mRestrictedCount > 0 && ratSwitchCaller != RAT_SWITCH_RESTRICT
                && ratSwitchCaller != RAT_SWITCH_NWS) {
            if (ratSwitchCaller == RAT_SWITCH_INIT) {
                logD(RAT_CTRL_TAG, "[switchNwRat] Skip INIT request in restricted mode!");
                if (action != NULL) {
                    action->act();
                }
            } else {
                logD(RAT_CTRL_TAG, "[switchNwRat] quene request in restricted mode!");
                queueRatSwitchRecord(appFamType, prefNwType, nwsMode, ratSwitchCaller, NULL, NULL);
                if (action != NULL) {
                    action->act();
                }
                if (message != NULL) {
                    resToRilj = RfxMessage::obtainResponse(RIL_E_SUCCESS, message);
                    responseToRilj(resToRilj);
                }
            }
            return;
        }
        if (mRestrictedCount == 0 && ratSwitchCaller == RAT_SWITCH_NWS
                && mCurPreferedNetWorkType != PREF_NET_TYPE_TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA
                && mCurPreferedNetWorkType != PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO
                && mCurPreferedNetWorkType != PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA
                && mCurPreferedNetWorkType != PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO) {
            logD(RAT_CTRL_TAG, "[switchNwRat] Not global mode, IR switch rejected!");
            if (action != NULL) {
                action->act();
            }
            if (message != NULL) {
                resToRilj = RfxMessage::obtainResponse(RIL_E_GENERIC_FAILURE, message);
                responseToRilj(resToRilj);
            }
            return;
        }
        logD(RAT_CTRL_TAG, "[switchNwRat] curAppFamType: %d, curPrefNwType: %d, curNwsMode: %d",
                mCurAppFamilyType, mCurPreferedNetWorkType, mCurNwsMode);
        calculateModemSettings(prefNwType, appFamType, nwsMode);
        mModemSettings.ratSwitchCaller = ratSwitchCaller;
        if (mModemSettings.prefNwType == -1) {
            updateState(mCurNwsMode, mCurPreferedNetWorkType);
            if (action != NULL) {
                action->act();
            }
            if (message != NULL) {
                resToRilj = RfxMessage::obtainResponse(RIL_E_GENERIC_FAILURE, message);
                responseToRilj(resToRilj);
            }
            return;
        }
        if (mModemSettings.appFamType == mCurAppFamilyType
                && mModemSettings.prefNwType == mCurPreferedNetWorkType
                && mModemSettings.nwsMode == mCurNwsMode
                && (ratSwitchCaller == RAT_SWITCH_NWS || ratSwitchCaller == RAT_SWITCH_NORMAL)) {
            logD(RAT_CTRL_TAG, "[switchNwRat] Already in desired mode, switch not executed");
            if (ratSwitchCaller == RAT_SWITCH_NORMAL) {
                mPreferredNetWorkTypeFromRILJ = prefNwType;
                if (getMajorSlotId() == m_slot_id) {
                    logD(RAT_CTRL_TAG, "[switchNwRat] rilj_nw_type set to %d", mPreferredNetWorkTypeFromRILJ);
                    setPreferredNetWorkTypeToSysProp(m_slot_id, mPreferredNetWorkTypeFromRILJ);
                }
            }
            if (action != NULL) {
                action->act();
            }
            if (message != NULL) {
                resToRilj = RfxMessage::obtainResponse(RIL_E_SUCCESS, message);
                responseToRilj(resToRilj);
            }
            return;
        }
        sIsInSwitching = true;
        mAction = action;
        mMessage = message;
        getStatusManager()->setBoolValue(RFX_STATUS_KEY_IS_RAT_MODE_SWITCHING, true);
        controlSwitchFlow(ACT_RAT_SWITCH_BEGIN);
    }
}

void RpNwWwopRatController::controlSwitchFlow(RAT_CTRL_ACTION nextAction) {
    logD(RAT_CTRL_TAG, "[controlSwitchFlow] prefNwType:%d, nwsMode:%d, caller:%s, act:%d",
            mModemSettings.prefNwType, mModemSettings.nwsMode,
            switchCallerToString(mModemSettings.ratSwitchCaller), nextAction);
    mCtrlAction = nextAction;
    switch (nextAction) {
        case ACT_RAT_SWITCH_BEGIN:
            onRatSwitchStart(mModemSettings.prefNwType, mModemSettings.nwsMode);
        case ACT_CONFIG_RADIO_OFF:
            // radio off MD firstly, if its final state should be off
            mCtrlAction = ACT_CONFIG_RADIO_OFF;
            configRadioPower(false);
            break;
        case ACT_CONFIG_EVDO_MODE:
            configEvdoMode();
            break;
        case ACT_CONFIG_SVLTE_MODE:
            configSvlteMode();
            break;
        case ACT_CONFIG_RAT_MODE:
            configRatMode();
            break;
        case ACT_CONFIG_PHONE_MODE:
            configPhoneMode();
            break;
        case ACT_CONFIG_RADIO_ON:
            // radio on MD, if its final state should be on
            configRadioPower(true);
            break;
        case ACT_RAT_SWITCH_DONE:
            updateState(mModemSettings.nwsMode, mModemSettings.prefNwType);
            onRatSwitchDone(mModemSettings.prefNwType);
            if (mModemSettings.ratSwitchCaller == RAT_SWITCH_NORMAL) {
                mPreferredNetWorkTypeFromRILJ = mDesiredNetWorkTypeFromRILJ;
                if (getMajorSlotId() == m_slot_id) {
                    logD(RAT_CTRL_TAG, "Switch done, rilj_nw_type set to %d", mPreferredNetWorkTypeFromRILJ);
                    setPreferredNetWorkTypeToSysProp(m_slot_id, mPreferredNetWorkTypeFromRILJ);
                }
            }
            if (mAction != NULL) {
                logD(RAT_CTRL_TAG, "Switch done, mAction->act()");
                mAction->act();
            }
            if (mMessage != NULL) {
                responseSetPreferredNetworkType(RIL_E_SUCCESS, mMessage);
            }
            logD(RAT_CTRL_TAG, "Rat switch done!");
            doPendingRatSwitchRecord();
            break;
        case ACT_RAT_SWITCH_FAIL:
            logD(RAT_CTRL_TAG, "Rat switch fail!");
            updateState(mCurNwsMode, mCurPreferedNetWorkType);
            onRatSwitchDone(mCurPreferedNetWorkType);
            if (mAction != NULL) {
                mAction->act();
            }
            if (mMessage != NULL) {
                responseSetPreferredNetworkType(RIL_E_GENERIC_FAILURE, mMessage);
            }
            doPendingRatSwitchRecord();
            break;
    }
}

void RpNwWwopRatController::configRadioPower(bool state) {
    bool needChangeRadioState = false;
    int currentRadioCapability = getStatusManager()->getIntValue(
            RFX_STATUS_KEY_SUGGESTED_RADIO_CAPABILITY, RIL_CAPABILITY_NONE);
    int expectRadioCapability = getSuggestedRadioCapability();
    int targetRadioCapability  = RIL_CAPABILITY_NONE;

    logD(RAT_CTRL_TAG, "currentRadioCapability: %d expectRadioCapability: %d state:%s",
            currentRadioCapability, expectRadioCapability, (state ? "on" : "off"));
    if (state) {
        mSugggestedResetRadioCapablity = expectRadioCapability;
        if (currentRadioCapability != expectRadioCapability) {
            logD(RAT_CTRL_TAG, "Config radio capability to %d", expectRadioCapability);
            RpSuggestRadioCapabilityCallback callback =
                    RpSuggestRadioCapabilityCallback(this,
                            &RpNwWwopRatController::onSuggestRadioOnResult);
            mRpRadioController->suggestedCapability(expectRadioCapability,
                    callback);
        } else {
            logD(RAT_CTRL_TAG, "No need to set radio on");
            controlSwitchFlow(ACT_RAT_SWITCH_DONE);
        }
    } else {
        int keyRadioState = getStatusManager()->getIntValue(
                        RFX_STATUS_KEY_RADIO_STATE, RADIO_STATE_UNAVAILABLE);
        switch (currentRadioCapability) {
            case RIL_CAPABILITY_NONE:
                // both MD1 & MD3 are off already
                needChangeRadioState = false;
                break;
            case RIL_CAPABILITY_GSM_ONLY:
                if (keyRadioState == RADIO_STATE_UNAVAILABLE) {
                    needChangeRadioState = false;
                    logD(RAT_CTRL_TAG, "keyRadioState = RADIO_STATE_UNAVAILABLE");
                    break;
                }
                if (expectRadioCapability == RIL_CAPABILITY_GSM_ONLY
                        || expectRadioCapability == RIL_CAPABILITY_CDMA_ON_LTE) {
                    needChangeRadioState = false;
                } else {
                    // radio off MD1 first
                    needChangeRadioState = true;
                    targetRadioCapability = RIL_CAPABILITY_NONE;
                }
                break;
            case RIL_CAPABILITY_CDMA_ONLY:
                if (expectRadioCapability == RIL_CAPABILITY_CDMA_ONLY
                        || expectRadioCapability == RIL_CAPABILITY_CDMA_ON_LTE) {
                    needChangeRadioState = false;
                } else {
                    // radio off MD3 first
                    needChangeRadioState = true;
                    targetRadioCapability = RIL_CAPABILITY_NONE;
                }
                break;
            case RIL_CAPABILITY_CDMA_ON_LTE:
                if (expectRadioCapability == RIL_CAPABILITY_CDMA_ON_LTE) {
                    needChangeRadioState = false;
                } else {
                    needChangeRadioState = true;
                    targetRadioCapability = expectRadioCapability;
                }
                break;
        }
        if (needChangeRadioState) {
            logD(RAT_CTRL_TAG, "Config radio capability to %d", targetRadioCapability);
            RpSuggestRadioCapabilityCallback callback =
                    RpSuggestRadioCapabilityCallback(this,
                            &RpNwWwopRatController::onSuggestRadioOffResult);
            mRpRadioController->suggestedCapability(targetRadioCapability,
                    callback);
        } else {
            logD(RAT_CTRL_TAG, "No need to set radio off");
            controlSwitchFlow(ACT_CONFIG_EVDO_MODE);
        }
    }
}

void RpNwWwopRatController::onSuggestRadioOffResult(SuggestRadioResult result) {
    if (result == SUCCESS) {
        // done (suggest complete or do nothing)
        logD(RAT_CTRL_TAG, "[onSuggestRadioOffResult] SUCCESS");
        controlSwitchFlow(ACT_CONFIG_EVDO_MODE);
    } else if (result == SUCCESS_WITHOUT_SETTING_RADIO) {
        // RILJ request off state
        logD(RAT_CTRL_TAG, "[onSuggestRadioOffResult] SUCCESS_WITHOUT_SETTING_RADIO");
        controlSwitchFlow(ACT_CONFIG_EVDO_MODE);
    } else if (result == ERROR) {
        // fail
        logD(RAT_CTRL_TAG, "[onSuggestRadioOffResult] ERROR");
        controlSwitchFlow(ACT_RAT_SWITCH_FAIL);
    } else {
        logD(RAT_CTRL_TAG, "[onSuggestRadioOffResult] WTF");
        controlSwitchFlow(ACT_RAT_SWITCH_FAIL);
    }
}

void RpNwWwopRatController::onSuggestRadioOnResult(SuggestRadioResult result) {
    if (result == SUCCESS) {
        // done (suggest complete or do nothing)
        logD(RAT_CTRL_TAG, "[onSuggestRadioOnResult] SUCCESS");
        controlSwitchFlow(ACT_RAT_SWITCH_DONE);
    } else if (result == SUCCESS_WITHOUT_SETTING_RADIO) {
        // RILJ request off state
        logD(RAT_CTRL_TAG, "[onSuggestRadioOnResult] SUCCESS_WITHOUT_SETTING_RADIO");
        controlSwitchFlow(ACT_RAT_SWITCH_DONE);
    } else if (result == ERROR) {
        // fail
        logD(RAT_CTRL_TAG, "[onSuggestRadioOnResult] ERROR");
        controlSwitchFlow(ACT_RAT_SWITCH_FAIL);
    } else {
        logD(RAT_CTRL_TAG, "[onSuggestRadioOnResult] WTF");
        controlSwitchFlow(ACT_RAT_SWITCH_FAIL);
    }
}

void RpNwWwopRatController::configEvdoMode() {
    if (mModemSettings.evdomode != EVDO_MODE_INVALID) {
        logD(RAT_CTRL_TAG, "[configEvdoMode] set evdomode: %d", mModemSettings.evdomode);
        sp<RfxMessage> message = RfxMessage::obtainRequest(m_slot_id,
                RADIO_TECH_GROUP_C2K, RIL_LOCAL_C2K_REQUEST_CONFIG_EVDO_MODE);
        message->getParcel()->writeInt32(1);
        message->getParcel()->writeInt32(mModemSettings.evdomode);
        requestToRild(message);
    } else {
        controlSwitchFlow(ACT_CONFIG_SVLTE_MODE);
    }
}

void RpNwWwopRatController::configSvlteMode() {
    if (mModemSettings.svlteMode != SVLTE_RAT_MODE_INVALID) {
        logD(RAT_CTRL_TAG, "[configSvlteMode] set svlteMode: %d", mModemSettings.svlteMode);
        int radioTechMode = RADIO_TECH_MODE_UNKNOWN;
        int preSvlteMode = -1;  // Not used at all
        int newSvlteMode = mModemSettings.svlteMode;
        bool is3gCdmaSim = getStatusManager()->getBoolValue(
                RFX_STATUS_KEY_CT3G_DUALMODE_CARD, false);
        sp<RfxMessage> gsmMessage = RfxMessage::obtainRequest(
                m_slot_id, RADIO_TECH_GROUP_GSM, RIL_LOCAL_REQUEST_SET_SVLTE_RAT_MODE);
        if (mModemSettings.nwsMode == NWS_MODE_CSFB) {
            radioTechMode = RADIO_TECH_MODE_CSFB;
        } else if (mModemSettings.nwsMode == NWS_MODE_CDMALTE) {
            radioTechMode = RADIO_TECH_MODE_CDMALTE;
        }
        gsmMessage->getParcel()->writeInt32(6);
        gsmMessage->getParcel()->writeInt32(radioTechMode);
        gsmMessage->getParcel()->writeInt32(preSvlteMode);
        gsmMessage->getParcel()->writeInt32(newSvlteMode);
        gsmMessage->getParcel()->writeInt32((int) mCurNwsMode);
        gsmMessage->getParcel()->writeInt32((int) mModemSettings.nwsMode);
        gsmMessage->getParcel()->writeInt32(is3gCdmaSim ? 1 : 0);
        requestToRild(gsmMessage);
        if (is3gCdmaSim &&
            mModemSettings.svlteMode < SVLTE_RAT_MODE_STK_SWITCH) {
            sp<RfxMessage> cdmaMessage = RfxMessage::obtainRequest(RADIO_TECH_GROUP_C2K,
                    RIL_LOCAL_C2K_REQUEST_SET_SVLTE_RAT_MODE, gsmMessage);
            cdmaMessage->getParcel()->writeInt32(6);
            cdmaMessage->getParcel()->writeInt32(radioTechMode);
            cdmaMessage->getParcel()->writeInt32(preSvlteMode);
            cdmaMessage->getParcel()->writeInt32(newSvlteMode);
            cdmaMessage->getParcel()->writeInt32((int) mCurNwsMode);
            cdmaMessage->getParcel()->writeInt32((int) mModemSettings.nwsMode);
            cdmaMessage->getParcel()->writeInt32(1);
            requestToRild(cdmaMessage);
        }
    } else {
        controlSwitchFlow(ACT_CONFIG_RAT_MODE);
    }
}

void RpNwWwopRatController::configRatMode() {
    if (mModemSettings.erat != -1 || mModemSettings.prefmode != -1) {
        sp<RfxMessage> reqToRild;
        // TODO: we should support config both ERAT & PREFMODE
        if (mModemSettings.erat != -1) {
            logD(RAT_CTRL_TAG, "[configRatMode] set erat: %d", mModemSettings.erat);
            if (mMessage != NULL) {
                reqToRild = RfxMessage::obtainRequest(RADIO_TECH_GROUP_GSM,
                        RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE, mMessage);
            } else {
                reqToRild = RfxMessage::obtainRequest(m_slot_id,
                        RADIO_TECH_GROUP_GSM, RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE);
            }
            reqToRild->getParcel()->writeInt32(1);
            // Change rat to C2K hybrid if C2K enable in WCG mode
            // due to +ERAT does not support WCG combination.
            if ((mModemSettings.prefNwType == PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO ||
                mModemSettings.prefNwType == PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO) &&
                mModemSettings.erat == 7) {
                reqToRild->getParcel()->writeInt32(PREF_NET_TYPE_CDMA_EVDO_AUTO);
            } else {
                reqToRild->getParcel()->writeInt32(mModemSettings.prefNwType);
            }
            requestToRild(reqToRild);
        } else {
            logD(RAT_CTRL_TAG, "[configRatMode] set prefmode: %d", mModemSettings.prefmode);
            if (mMessage != NULL) {
                reqToRild = RfxMessage::obtainRequest(RADIO_TECH_GROUP_C2K,
                        RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE, mMessage);
            } else {
                reqToRild = RfxMessage::obtainRequest(m_slot_id,
                        RADIO_TECH_GROUP_C2K, RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE);
            }
            reqToRild->getParcel()->writeInt32(1);
            reqToRild->getParcel()->writeInt32(mModemSettings.prefNwType);
            requestToRild(reqToRild);
        }
    } else {
        controlSwitchFlow(ACT_CONFIG_PHONE_MODE);
    }
}

void RpNwWwopRatController::configPhoneMode() {
    int tech = RADIO_TECH_UNKNOWN;

    switch (mModemSettings.prefNwType) {
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
            tech =  RADIO_TECH_HSPA;
            break;

        // we can't support volte_replacement_rat_int feature, so transfer tech LTE to GSM
        // force java framework switch to GSMPhone
        case PREF_NET_TYPE_LTE_ONLY:
        case PREF_NET_TYPE_LTE_TDD_ONLY:
            tech = RADIO_TECH_GPRS;
            break;

        case PREF_NET_TYPE_CDMA_ONLY:
        case PREF_NET_TYPE_CDMA_EVDO_AUTO:
        case PREF_NET_TYPE_EVDO_ONLY:
        case PREF_NET_TYPE_LTE_CDMA_EVDO:
            tech = RADIO_TECH_1xRTT;
            break;

        case PREF_NET_TYPE_TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA:
        case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO:
        case PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO:
        case PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA:
            if (mModemSettings.nwsMode == NWS_MODE_CDMALTE) {
                tech = RADIO_TECH_1xRTT;
            } else if (mModemSettings.nwsMode == NWS_MODE_CSFB) {
                tech = RADIO_TECH_GPRS;
            }
            break;
    }
    if (tech > RADIO_TECH_UNKNOWN) {
        sp<RfxMessage> urcToRilj = RfxMessage::obtainUrc(m_slot_id,
                RIL_UNSOL_VOICE_RADIO_TECH_CHANGED);
        urcToRilj->getParcel()->writeInt32(1);
        urcToRilj->getParcel()->writeInt32(tech);
        responseToRilj(urcToRilj);
        logD(RAT_CTRL_TAG, "[configPhoneMode] tech: %d", tech);
    } else {
        logD(RAT_CTRL_TAG, "[configPhoneMode] tech unknown!");
    }
    controlSwitchFlow(ACT_CONFIG_RADIO_ON);
}

void RpNwWwopRatController::responseSetSvlteMode(const sp<RfxMessage> &response) {
    sp<RfxMessage> responseInQueue;
    sp<RfxMessage> responseMd1;
    sp<RfxMessage> responseMd3;
    ResponseStatus responseStatus;
    bool is3gCdmaSim = getStatusManager()->getBoolValue(
            RFX_STATUS_KEY_CT3G_DUALMODE_CARD, false);

    onEctModeChangeDone(mModemSettings.prefNwType);
    if (is3gCdmaSim &&
        mModemSettings.svlteMode < SVLTE_RAT_MODE_STK_SWITCH) {
        // CDMA 3G SIM
        responseInQueue = sp<RfxMessage>(NULL);
        responseStatus = preprocessResponse(response, responseInQueue,
                RfxWaitResponseTimedOutCallback(this, &RpNwWwopRatController::onResponseTimeOut),
                s2ns(10));
        if (responseStatus == RESPONSE_STATUS_HAVE_MATCHED) {
            logD(RAT_CTRL_TAG, "Response matched");
            if (responseInQueue->getSource() == RADIO_TECH_GROUP_GSM) {
                responseMd1 = responseInQueue;
                responseMd3 = response;
            } else {
                responseMd1 = response;
                responseMd3 = responseInQueue;
            }
            if (responseMd1->getError() != RIL_E_SUCCESS) {
                logD(RAT_CTRL_TAG, "MD1 sponse error");
                controlSwitchFlow(ACT_RAT_SWITCH_FAIL);
            } else if (responseMd3->getError() != RIL_E_SUCCESS) {
                logD(RAT_CTRL_TAG, "MD3 response error");
                controlSwitchFlow(ACT_RAT_SWITCH_FAIL);
            } else {
                controlSwitchFlow(ACT_CONFIG_RAT_MODE);
            }
        } else if (responseStatus == RESPONSE_STATUS_NO_MATCH_AND_SAVE) {
            logD(RAT_CTRL_TAG, "Response saved");
        } else if (responseStatus == RESPONSE_STATUS_ALREADY_SAVED) {
            logD(RAT_CTRL_TAG, "Response have saved -> ignore");
        } else if (responseStatus == RESPONSE_STATUS_HAVE_BEEN_DELETED) {
            logD(RAT_CTRL_TAG, "Response have time out! src=%s token=%d",
                    sourceToString(response->getSource()), response->getPToken());
        } else {
            logD(RAT_CTRL_TAG, "Response misc error -> ignore");
        }
    } else {
        // CDMA 4G SIM
        int error = response->getError();
        if (error != RIL_E_SUCCESS) {
            logD(RAT_CTRL_TAG, "response error: %d", error);
            controlSwitchFlow(ACT_RAT_SWITCH_FAIL);
        } else {
            controlSwitchFlow(ACT_CONFIG_RAT_MODE);
        }
    }
}

void RpNwWwopRatController::onResponseTimeOut(const sp<RfxMessage> &response) {
    int reqId = response->getId();
    logD(RAT_CTRL_TAG, "[onResponseTimeOut] REQ=%s src=%s token=%d",
            requestToString(reqId), sourceToString(response->getSource()),
            response->getPToken());
    controlSwitchFlow(ACT_RAT_SWITCH_FAIL);
}

void RpNwWwopRatController::responseSetPreferredNetworkType(
        const RIL_Errno error, const sp<RfxMessage> &response) {
    sp<RfxMessage> resToRilj = RfxMessage::obtainResponse(error, response);
    if (error != RIL_E_SUCCESS) {
        // error
        logD(RAT_CTRL_TAG, "[responseSetPreferredNetworkType] response error");
    } else {
        // ok
        logD(RAT_CTRL_TAG, "[responseSetPreferredNetworkType] response OK");
    }
    responseToRilj(resToRilj);
}

void RpNwWwopRatController::queueRatSwitchRecord(const AppFamilyType appFamType,
        int prefNwType, NwsMode nwsMode, const RatSwitchCaller ratSwitchCaller,
        const sp<RfxAction> &action, const sp<RfxMessage> &message) {
    logD(RAT_CTRL_TAG, "[queueRatSwitchRecord] ratSwitchCaller:%s prefNwType:%d",
            switchCallerToString(ratSwitchCaller), prefNwType);
    if (ratSwitchCaller == RAT_SWITCH_RESTRICT) {
        mPendingRestrictedRatSwitchRecord.appFamType = appFamType;
        mPendingRestrictedRatSwitchRecord.prefNwType = prefNwType;
        mPendingRestrictedRatSwitchRecord.nwsMode = nwsMode;
        mPendingRestrictedRatSwitchRecord.ratSwitchCaller = RAT_SWITCH_RESTRICT;
        mPendingRestrictedRatSwitchRecord.action = action;
        mPendingRestrictedRatSwitchRecord.message = message;
    } else if (ratSwitchCaller == RAT_SWITCH_INIT) {
        mPendingInitRatSwitchRecord.appFamType = appFamType;
        mPendingInitRatSwitchRecord.prefNwType = prefNwType;
        mPendingInitRatSwitchRecord.nwsMode = nwsMode;
        mPendingInitRatSwitchRecord.ratSwitchCaller = RAT_SWITCH_INIT;
        mPendingInitRatSwitchRecord.action = action;
        mPendingInitRatSwitchRecord.message = message;
    } else if (ratSwitchCaller == RAT_SWITCH_NWS || ratSwitchCaller == RAT_SWITCH_NORMAL) {
        if (mPendingNormalRatSwitchRecord.prefNwType != -1
                && mPendingNormalRatSwitchRecord.message != NULL) {
            logD(RAT_CTRL_TAG, "[queueRatSwitchRecord] new setPreferredNwType request is sent"
                    ", old request in queue will be ignored and response");
            responseSetPreferredNetworkType(
                    RIL_E_GENERIC_FAILURE, mPendingNormalRatSwitchRecord.message);
        }
        mPendingNormalRatSwitchRecord.appFamType = appFamType;
        mPendingNormalRatSwitchRecord.prefNwType = prefNwType;
        mPendingNormalRatSwitchRecord.nwsMode = nwsMode;
        mPendingNormalRatSwitchRecord.ratSwitchCaller = ratSwitchCaller;
        mPendingNormalRatSwitchRecord.action = action;
        mPendingNormalRatSwitchRecord.message = message;
    } else {
        logD(RAT_CTRL_TAG, "[queueRatSwitchRecord] Invalid ratSwitchCaller:%d", ratSwitchCaller);
    }
}

void RpNwWwopRatController::doPendingRatSwitchRecord() {
    // param for INIT, NOR switch caller use, clean pending nw type after request is triggered
    int prefNwType = -1;
    NwsMode nwsMode = NWS_MODE_UNKNOWN;
    if (mPendingRestrictedRatSwitchRecord.prefNwType != -1) {
        logD(RAT_CTRL_TAG, "[doPendingRestrictedRatSwitchRecord] appFamType: %d, "
                "prefNwType: %d, nwsMode: %d, ratSwitchCaller: %s",
                mPendingRestrictedRatSwitchRecord.appFamType,
                mPendingRestrictedRatSwitchRecord.prefNwType,
                mPendingRestrictedRatSwitchRecord.nwsMode,
                switchCallerToString(mPendingRestrictedRatSwitchRecord.ratSwitchCaller));
        prefNwType = mPendingRestrictedRatSwitchRecord.prefNwType;
        mPendingRestrictedRatSwitchRecord.prefNwType = -1;
        switchNwRat(mPendingRestrictedRatSwitchRecord.appFamType,
                prefNwType,
                mPendingRestrictedRatSwitchRecord.nwsMode,
                mPendingRestrictedRatSwitchRecord.ratSwitchCaller,
                mPendingRestrictedRatSwitchRecord.action,
                mPendingRestrictedRatSwitchRecord.message);
    } else if (mPendingInitRatSwitchRecord.prefNwType != -1) {
        logD(RAT_CTRL_TAG, "[doPendingInitRatSwitchRecord] appFamType: %d, "
                "prefNwType: %d, nwsMode: %d, ratSwitchCaller: %s",
                mPendingInitRatSwitchRecord.appFamType,
                mPendingInitRatSwitchRecord.prefNwType,
                mPendingInitRatSwitchRecord.nwsMode,
                switchCallerToString(mPendingInitRatSwitchRecord.ratSwitchCaller));
        if (mPendingInitRatSwitchRecord.prefNwType !=
                getPreferredNetWorkTypeFromSysProp(m_slot_id) &&
                mPreferredNetWorkTypeFromRILJ != -1 &&
                getMajorSlotId() == m_slot_id) {
            prefNwType = getPreferredNetWorkTypeFromSysProp(m_slot_id);
        } else {
            prefNwType = mPendingInitRatSwitchRecord.prefNwType;
        }
        mPendingInitRatSwitchRecord.prefNwType = -1;
        switchNwRat(mPendingInitRatSwitchRecord.appFamType,
                prefNwType,
                mPendingInitRatSwitchRecord.nwsMode,
                mPendingInitRatSwitchRecord.ratSwitchCaller,
                mPendingInitRatSwitchRecord.action,
                mPendingInitRatSwitchRecord.message);
    } else if (mPendingNormalRatSwitchRecord.prefNwType != -1) {
        logD(RAT_CTRL_TAG, "[doPendingNormalRatSwitchRecord] appFamType: %d, "
                "prefNwType: %d, nwsMode: %d, ratSwitchCaller: %s",
                mCurAppFamilyType,
                mPendingNormalRatSwitchRecord.prefNwType,
                mPendingNormalRatSwitchRecord.nwsMode,
                switchCallerToString(mPendingNormalRatSwitchRecord.ratSwitchCaller));
        prefNwType = mPendingNormalRatSwitchRecord.prefNwType;
        nwsMode = mPendingNormalRatSwitchRecord.nwsMode;
        mPendingNormalRatSwitchRecord.prefNwType = -1;
        if (mPendingNormalRatSwitchRecord.ratSwitchCaller == RAT_SWITCH_NORMAL) {
            logD(RAT_CTRL_TAG, "[doPendingNormalRatSwitchRecord] change nwsMode: %d",
                    mCurNwsMode);
            // nwsMode could be changed by IR while RILJ request pending
            // use current nwsMode to do switch request
            nwsMode = mCurNwsMode;
        } else if (mPendingNormalRatSwitchRecord.ratSwitchCaller == RAT_SWITCH_NWS) {
            logD(RAT_CTRL_TAG, "[doPendingNormalRatSwitchRecord] change prefNwType: %d",
                    mCurPreferedNetWorkType);
            // prefNwType could be changed by RILJ/ModeCtrl while IR request pending
            // use current prefNwType to do switch request
            prefNwType = mCurPreferedNetWorkType;
        }
        // neither RILJ/IR can decide appFamilyType which could be changed while request pending
        // use mCurAppFamilyType to do switch request
        switchNwRat(mCurAppFamilyType, prefNwType, nwsMode,
                mPendingNormalRatSwitchRecord.ratSwitchCaller,
                mPendingNormalRatSwitchRecord.action,
                mPendingNormalRatSwitchRecord.message);
    } else {
        logD(RAT_CTRL_TAG, "[doPendingRatSwitchRecord] no pending record for this sim");
    }

    if(!hasPendingRecord()){
        RpNwWwopRatController *another = (RpNwWwopRatController *) findController(
                m_slot_id == 0 ? 1 : 0,
                RFX_OBJ_CLASS_INFO(RpNwWwopRatController));
        if (another != NULL && another->hasPendingRecord()) {
            logD(RAT_CTRL_TAG, "[doPendingRatSwitchRecord] another SIM has pending record");
            another->doPendingRatSwitchRecord();
        }
        logD(RAT_CTRL_TAG, "[doPendingRatSwitchRecord] no pending record, "
                "another sim has no pending record, finish");
    }
}

bool RpNwWwopRatController::hasPendingRecord() {
    if (mPendingInitRatSwitchRecord.prefNwType != -1
            || mPendingNormalRatSwitchRecord.prefNwType != -1
            || mPendingRestrictedRatSwitchRecord.prefNwType != -1) {
        return true;
    }
    return false;
}

void RpNwWwopRatController::calculateModemSettings(int prefNwType,
        AppFamilyType appFamType, NwsMode nwsMode) {
    bool isValid = false;
    bool is3gCdmaSim = getStatusManager()->getBoolValue(
            RFX_STATUS_KEY_CT3G_DUALMODE_CARD, false);
    bool isUim = isCdmaOnlySim();

    mModemSettings.prefNwType = -1;
    mModemSettings.appFamType = APP_FAM_UNKNOWN;
    mModemSettings.nwsMode = NWS_MODE_UNKNOWN;
    mModemSettings.erat = -1;
    mModemSettings.evdomode = -1;
    mModemSettings.ectmode = -1;
    mModemSettings.prefmode = -1;
    mModemSettings.md1Radio = false;
    mModemSettings.md3Radio = false;
    mModemSettings.svlteMode = SVLTE_RAT_MODE_INVALID;
    mModemSettings.ratSwitchCaller = RAT_SWITCH_UNKNOWN;
    switch (prefNwType) {
        // LWCG global mode
        case PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA:
        case PREF_NET_TYPE_TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA:
            if (appFamType == APP_FAM_3GPP2) {
                if (is3gCdmaSim) {
                    // C2K 3G SIM, change to WCG mode
                    prefNwType = PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO;
                    if (!isEratExtSupport() &&
                            getMajorSlotId() != m_slot_id) {
                        mModemSettings.prefmode = 8;
                        mModemSettings.ectmode = 2;
                        mModemSettings.evdomode = EVDO_MODE_NORMAL;
                        mModemSettings.svlteMode = SVLTE_RAT_MODE_3G;
                        if (nwsMode == NWS_MODE_CDMALTE) {
                            mModemSettings.md1Radio = false;
                            mModemSettings.md3Radio = true;
                        } else if (nwsMode == NWS_MODE_CSFB) {
                            mModemSettings.md1Radio = true;
                            mModemSettings.md3Radio = false;
                        }
                    } else {
                        if (nwsMode == NWS_MODE_CDMALTE) {
                            mModemSettings.erat = 7;
                            mModemSettings.svlteMode = SVLTE_RAT_MODE_IR_SUSPEND;
                            mModemSettings.md1Radio = false;
                            mModemSettings.md3Radio = true;
                        } else if (nwsMode == NWS_MODE_CSFB) {
                            mModemSettings.erat = 2;
                            mModemSettings.svlteMode = SVLTE_RAT_MODE_IR_SUSPEND;
                            mModemSettings.md1Radio = true;
                            mModemSettings.md3Radio = false;
                        }
                    }
                } else if (isUim) {
                    prefNwType = PREF_NET_TYPE_CDMA_EVDO_AUTO;
                    if (!isEratExtSupport() &&
                            getMajorSlotId() != m_slot_id) {
                        mModemSettings.prefmode = 2;
                        mModemSettings.svlteMode = SVLTE_RAT_MODE_3G;
                    } else {
                        mModemSettings.erat = 7;
                        mModemSettings.svlteMode = SVLTE_RAT_MODE_STK_SWITCH;
                    }
                    nwsMode = NWS_MODE_CDMALTE;
                    mModemSettings.md1Radio = false;
                    mModemSettings.md3Radio = true;
                } else {
                    // If it's not major SIM, change to WCG mode
                    if (!isEratExtSupport() &&
                            getMajorSlotId() != m_slot_id) {
                        prefNwType = PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO;
                        mModemSettings.prefmode = 8;
                        mModemSettings.ectmode = 2;
                        mModemSettings.evdomode = EVDO_MODE_NORMAL;
                        mModemSettings.svlteMode = SVLTE_RAT_MODE_3G;
                        if (nwsMode == NWS_MODE_CDMALTE) {
                            mModemSettings.md1Radio = false;
                            mModemSettings.md3Radio = true;
                        } else if (nwsMode == NWS_MODE_CSFB) {
                            mModemSettings.md1Radio = true;
                            mModemSettings.md3Radio = false;
                        }
                    } else {
                        if (!isEratExtSupport() ) {
                            mModemSettings.ectmode = 0;
                            mModemSettings.evdomode = EVDO_MODE_EHRPD;
                            mModemSettings.svlteMode = SVLTE_RAT_MODE_4G;
                        } else {
                            mModemSettings.erat = 14;
                        }
                        mModemSettings.md1Radio = true;
                        mModemSettings.md3Radio = true;
                    }
                }
                isValid = true;
            } else if (appFamType == APP_FAM_3GPP) {
                if (mRestrictedCount > 0) {
                    // ECC redial, force GSM SIM switch to global mode
                    if (!isEratExtSupport()) {
                        mModemSettings.ectmode = 0;
                        mModemSettings.evdomode = EVDO_MODE_EHRPD;
                        mModemSettings.svlteMode = SVLTE_RAT_MODE_4G;
                    } else {
                        mModemSettings.erat = 14;
                    }
                    mModemSettings.md1Radio = true;
                    mModemSettings.md3Radio = true;
                } else if (is3gCdmaSim) {
                    // C2K 3G SIM, change to WCG mode
                    prefNwType = PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO;
                    if (!isEratExtSupport() &&
                            getMajorSlotId() != m_slot_id) {
                        mModemSettings.prefmode = 8;
                        mModemSettings.ectmode = 2;
                        mModemSettings.evdomode = EVDO_MODE_NORMAL;
                        mModemSettings.svlteMode = SVLTE_RAT_MODE_3G;
                        if (nwsMode == NWS_MODE_CDMALTE) {
                            mModemSettings.md1Radio = false;
                            mModemSettings.md3Radio = true;
                        } else if (nwsMode == NWS_MODE_CSFB) {
                            mModemSettings.md1Radio = true;
                            mModemSettings.md3Radio = false;
                        }
                    } else {
                        if (nwsMode == NWS_MODE_CDMALTE) {
                            mModemSettings.erat = 7;
                            mModemSettings.svlteMode = SVLTE_RAT_MODE_IR_SUSPEND;
                            mModemSettings.md1Radio = false;
                            mModemSettings.md3Radio = true;
                        } else if (nwsMode == NWS_MODE_CSFB) {
                            mModemSettings.erat = 2;
                            mModemSettings.svlteMode = SVLTE_RAT_MODE_IR_SUSPEND;
                            mModemSettings.md1Radio = true;
                            mModemSettings.md3Radio = false;
                        }
                    }
                } else {
                    prefNwType = PREF_NET_TYPE_LTE_GSM_WCDMA;
                    nwsMode = NWS_MODE_CSFB;
                    mModemSettings.erat = 6;
                    mModemSettings.md1Radio = true;
                    mModemSettings.md3Radio = false;
                }
                isValid = true;
            }
            break;
        // WCG global mode
        case PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO:
        case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO:
            if (appFamType == APP_FAM_3GPP2) {
                if (isUim) {
                    prefNwType = PREF_NET_TYPE_CDMA_EVDO_AUTO;
                    nwsMode = NWS_MODE_CDMALTE;
                    if (!isEratExtSupport() &&
                            getMajorSlotId() != m_slot_id) {
                        mModemSettings.prefmode = 2;
                        mModemSettings.svlteMode = SVLTE_RAT_MODE_3G;
                    } else {
                        mModemSettings.erat = 7;
                        mModemSettings.svlteMode = SVLTE_RAT_MODE_STK_SWITCH;
                    }
                    mModemSettings.md1Radio = false;
                    mModemSettings.md3Radio = true;
                } else {
                    if (!isEratExtSupport() &&
                            getMajorSlotId() != m_slot_id) {
                        mModemSettings.prefmode = 8;
                        mModemSettings.ectmode = 2;
                        mModemSettings.evdomode = EVDO_MODE_NORMAL;
                        mModemSettings.svlteMode = SVLTE_RAT_MODE_3G;
                        if (nwsMode == NWS_MODE_CDMALTE) {
                            mModemSettings.md1Radio = false;
                            mModemSettings.md3Radio = true;
                        } else if (nwsMode == NWS_MODE_CSFB) {
                            mModemSettings.md1Radio = true;
                            mModemSettings.md3Radio = false;
                        }
                    } else {
                        if (nwsMode == NWS_MODE_CDMALTE) {
                            mModemSettings.erat = 7;
                            mModemSettings.svlteMode = SVLTE_RAT_MODE_IR_SUSPEND;
                            mModemSettings.md1Radio = false;
                            mModemSettings.md3Radio = true;
                        } else if (nwsMode == NWS_MODE_CSFB) {
                            mModemSettings.erat = 2;
                            mModemSettings.svlteMode = SVLTE_RAT_MODE_IR_SUSPEND;
                            mModemSettings.md1Radio = true;
                            mModemSettings.md3Radio = false;
                        }
                    }
                }
                isValid = true;
            } else if (appFamType == APP_FAM_3GPP) {
                if (is3gCdmaSim) {
                    // CDMA 3G dual mode SIM activate GSM app
                    if (!isEratExtSupport() &&
                            getMajorSlotId() != m_slot_id) {
                        mModemSettings.prefmode = 8;
                        mModemSettings.ectmode = 2;
                        mModemSettings.evdomode = EVDO_MODE_NORMAL;
                        mModemSettings.svlteMode = SVLTE_RAT_MODE_3G;
                        if (nwsMode == NWS_MODE_CDMALTE) {
                            mModemSettings.md1Radio = false;
                            mModemSettings.md3Radio = true;
                        } else if (nwsMode == NWS_MODE_CSFB) {
                            mModemSettings.md1Radio = true;
                            mModemSettings.md3Radio = false;
                        }
                    } else {
                        if (nwsMode == NWS_MODE_CDMALTE) {
                            mModemSettings.erat = 7;
                            mModemSettings.svlteMode = SVLTE_RAT_MODE_IR_SUSPEND;
                            mModemSettings.md1Radio = false;
                            mModemSettings.md3Radio = true;
                        } else if (nwsMode == NWS_MODE_CSFB) {
                            mModemSettings.erat = 2;
                            mModemSettings.svlteMode = SVLTE_RAT_MODE_IR_SUSPEND;
                            mModemSettings.md1Radio = true;
                            mModemSettings.md3Radio = false;
                        }
                    }
                } else {
                    prefNwType = PREF_NET_TYPE_GSM_WCDMA;
                    nwsMode = NWS_MODE_CSFB;
                    mModemSettings.erat = 2;
                    mModemSettings.md1Radio = true;
                    mModemSettings.md3Radio = false;
                }
                isValid = true;
            }
            break;
        // LWG mode
        case PREF_NET_TYPE_LTE_GSM_WCDMA:
        case PREF_NET_TYPE_TD_SCDMA_GSM_LTE:
        case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_LTE:
            nwsMode = NWS_MODE_CSFB;
            mModemSettings.erat = 6;
            if (appFamType == APP_FAM_3GPP2){
                mModemSettings.svlteMode = SVLTE_RAT_MODE_STK_SWITCH;
            }
            mModemSettings.md1Radio = true;
            mModemSettings.md3Radio = false;
            isValid = true;
            break;
        // C2K hybrid mode
        case PREF_NET_TYPE_CDMA_EVDO_AUTO:
            nwsMode = NWS_MODE_CDMALTE;
            if (!isEratExtSupport() &&
                    getMajorSlotId() != m_slot_id) {
                mModemSettings.prefmode = 2;
                mModemSettings.svlteMode = SVLTE_RAT_MODE_3G;
            } else {
                mModemSettings.erat = 7;
                mModemSettings.svlteMode = SVLTE_RAT_MODE_STK_SWITCH;
            }
            mModemSettings.md1Radio = false;
            mModemSettings.md3Radio = true;
            isValid = true;
            break;
        // C2K 1x mode
        case PREF_NET_TYPE_CDMA_ONLY:
            nwsMode = NWS_MODE_CDMALTE;
            if (!isEratExtSupport() &&
                    getMajorSlotId() != m_slot_id) {
                mModemSettings.prefmode = 2;
                mModemSettings.svlteMode = SVLTE_RAT_MODE_3G;
            } else {
                mModemSettings.erat = 7;
                mModemSettings.prefmode = 32;
                mModemSettings.svlteMode = SVLTE_RAT_MODE_STK_SWITCH;
            }
            mModemSettings.md1Radio = false;
            mModemSettings.md3Radio = true;
            isValid = true;
            break;
        // C2K evdo mode
        case PREF_NET_TYPE_EVDO_ONLY:
            nwsMode = NWS_MODE_CDMALTE;
            if (!isEratExtSupport() &&
                    getMajorSlotId() != m_slot_id) {
                mModemSettings.prefmode = 2;
                mModemSettings.svlteMode = SVLTE_RAT_MODE_3G;
            } else {
                mModemSettings.erat = 7;
                mModemSettings.prefmode = 64;
                mModemSettings.svlteMode = SVLTE_RAT_MODE_STK_SWITCH;
            }
            mModemSettings.md1Radio = false;
            mModemSettings.md3Radio = true;
            isValid = true;
            break;
        // LC mode
        case PREF_NET_TYPE_LTE_CDMA_EVDO:
            if (appFamType == APP_FAM_3GPP2) {
                // C only or C2K 3G SIM, change to C mode
                if (isUim || is3gCdmaSim) {
                    prefNwType = PREF_NET_TYPE_CDMA_EVDO_AUTO;
                    nwsMode = NWS_MODE_CDMALTE;
                    if (!isEratExtSupport() &&
                            getMajorSlotId() != m_slot_id) {
                        mModemSettings.prefmode = 2;
                        mModemSettings.svlteMode = SVLTE_RAT_MODE_3G;
                    } else {
                        mModemSettings.erat = 7;
                        mModemSettings.svlteMode = SVLTE_RAT_MODE_STK_SWITCH;
                    }
                    mModemSettings.md1Radio = false;
                    mModemSettings.md3Radio = true;
                } else {
                    nwsMode = NWS_MODE_CDMALTE;
                    if (!isEratExtSupport()) {
                        mModemSettings.ectmode = 0;
                        mModemSettings.evdomode = EVDO_MODE_EHRPD;
                        mModemSettings.svlteMode = SVLTE_RAT_MODE_LTE_C2K;
                    } else {
                       mModemSettings.erat = 11;
                    }
                    mModemSettings.md1Radio = true;
                    mModemSettings.md3Radio = true;
                }
                isValid = true;
            } else if (appFamType == APP_FAM_3GPP) {
                if (mRestrictedCount > 0) {
                    // ECC redial, force GSM SIM switch to global mode
                    mModemSettings.erat = 11;
                    mModemSettings.md1Radio = true;
                    mModemSettings.md3Radio = true;
                } else if (is3gCdmaSim) {
                    // C2K 3G SIM, CSFB mode change to C mode
                    prefNwType = PREF_NET_TYPE_CDMA_EVDO_AUTO;
                    nwsMode = NWS_MODE_CDMALTE;
                    if (!isEratExtSupport() &&
                            getMajorSlotId() != m_slot_id) {
                        mModemSettings.prefmode = 2;
                        mModemSettings.svlteMode = SVLTE_RAT_MODE_3G;
                    } else {
                        mModemSettings.erat = 7;
                        mModemSettings.svlteMode = SVLTE_RAT_MODE_STK_SWITCH;
                    }
                    mModemSettings.md1Radio = false;
                    mModemSettings.md3Radio = true;
                } else {
                    prefNwType = PREF_NET_TYPE_LTE_ONLY;
                    nwsMode = NWS_MODE_CSFB;
                    mModemSettings.erat = 3;
                    mModemSettings.md1Radio = true;
                    mModemSettings.md3Radio = false;
                }
                isValid = true;
            }
            break;
        // WG mode
        case PREF_NET_TYPE_GSM_WCDMA:
        case PREF_NET_TYPE_TD_SCDMA_GSM:
        case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA:
            nwsMode = NWS_MODE_CSFB;
            mModemSettings.erat = 2;
            mModemSettings.md1Radio = true;
            mModemSettings.md3Radio = false;
            isValid = true;
            break;
        // G mode
        case PREF_NET_TYPE_GSM_ONLY:
            nwsMode = NWS_MODE_CSFB;
            mModemSettings.erat = 0;
            mModemSettings.md1Radio = true;
            mModemSettings.md3Radio = false;
            isValid = true;
            break;
        // W mode
        case PREF_NET_TYPE_WCDMA:
        case PREF_NET_TYPE_TD_SCDMA_ONLY:
        case PREF_NET_TYPE_TD_SCDMA_WCDMA:
            nwsMode = NWS_MODE_CSFB;
            mModemSettings.erat = 1;
            mModemSettings.md1Radio = true;
            mModemSettings.md3Radio = false;
            isValid = true;
            break;
        // WG mode
        case PREF_NET_TYPE_GSM_WCDMA_AUTO:
            nwsMode = NWS_MODE_CSFB;
            mModemSettings.erat = 2;
            mModemSettings.md1Radio = true;
            mModemSettings.md3Radio = false;
            isValid = true;
            break;
        // L mode
        case PREF_NET_TYPE_LTE_ONLY:
            nwsMode = NWS_MODE_CSFB;
            mModemSettings.erat = 3;
            mModemSettings.md1Radio = true;
            mModemSettings.md3Radio = false;
            isValid = true;
            break;
        // LW mode
        case PREF_NET_TYPE_LTE_WCDMA:
        case PREF_NET_TYPE_TD_SCDMA_LTE:
        case PREF_NET_TYPE_TD_SCDMA_WCDMA_LTE:
            nwsMode = NWS_MODE_CSFB;
            mModemSettings.erat = 5;
            mModemSettings.md1Radio = true;
            mModemSettings.md3Radio = false;
            isValid = true;
            break;
        // LG mode
        case PREF_NET_TYPE_LTE_GSM:
            nwsMode = NWS_MODE_CSFB;
            mModemSettings.erat = 4;
            mModemSettings.md1Radio = true;
            mModemSettings.md3Radio = false;
            isValid = true;
            break;
        case PREF_NET_TYPE_LTE_TDD_ONLY:
            nwsMode = NWS_MODE_CSFB;
            mModemSettings.ectmode = 1;
            mModemSettings.md1Radio = true;
            mModemSettings.md3Radio = false;
            mModemSettings.svlteMode = SVLTE_RAT_MODE_4G_DATA_ONLY;
            isValid = true;
            break;
        default:
            isValid = false;
            break;
    }

    if (isValid) {
        mModemSettings.prefNwType = prefNwType;
        mModemSettings.appFamType = appFamType;
        mModemSettings.nwsMode = nwsMode;
        logD(RAT_CTRL_TAG, "[calculateModemSettings] prefNwType:%d appFamType:%d nwsMode:%d"
                " erat:%d prefmode:%d svlteMode:%d md1:%s md3:%s"
                " ectmode:%d evdomode:%d",
                mModemSettings.prefNwType, mModemSettings.appFamType, mModemSettings.nwsMode,
                mModemSettings.erat, mModemSettings.prefmode, mModemSettings.svlteMode,
                mModemSettings.md1Radio ? "on" : "off",
                mModemSettings.md3Radio ? "on" : "off",
                mModemSettings.ectmode, mModemSettings.evdomode);
    } else {
        mModemSettings.prefNwType = -1;
        logD(RAT_CTRL_TAG, "[calculateModemSettings] invalid switch request!!"
                " prefNwType:%d appFamType:%d nwsMode:%d",
                mModemSettings.prefNwType, mModemSettings.appFamType, mModemSettings.nwsMode);
    }
}

void RpNwWwopRatController::registerRatSwitchCallback(IRpNwRatSwitchCallback *callback) {
    mRpNwRatSwitchListener = callback;
}

void RpNwWwopRatController::unRegisterRatSwitchCallback(IRpNwRatSwitchCallback *callback) {
    RFX_UNUSED(callback);
    mRpNwRatSwitchListener = NULL;
}

void RpNwWwopRatController::onRatSwitchStart(const int prefNwType, const NwsMode newNwsMode) {
    if (mRpNwRatSwitchListener != NULL) {
        mRpNwRatSwitchListener->onRatSwitchStart(mCurPreferedNetWorkType,
                prefNwType, mCurNwsMode, newNwsMode, VOLTE_OFF, VOLTE_OFF);
    }
}

void RpNwWwopRatController::onRatSwitchDone(const int prefNwType) {
    if (mRpNwRatSwitchListener != NULL) {
        mRpNwRatSwitchListener->onRatSwitchDone(mCurPreferedNetWorkType,
                prefNwType);
    }
}

void RpNwWwopRatController::onEctModeChangeDone(const int prefNwType) {
    if (mRpNwRatSwitchListener != NULL) {
        mRpNwRatSwitchListener->onEctModeChangeDone(mCurPreferedNetWorkType, prefNwType);
    }
}

int RpNwWwopRatController::getSuggestedRadioCapability() {
    int suggestedRadio = RIL_CAPABILITY_NONE;
    bool md1Radio = mModemSettings.md1Radio;
    bool md3Radio = mModemSettings.md3Radio;

    if (md1Radio && md3Radio) {
        suggestedRadio = RIL_CAPABILITY_CDMA_ON_LTE;
    } else if (md1Radio && !md3Radio) {
        suggestedRadio = RIL_CAPABILITY_GSM_ONLY;
    } else if (!md1Radio && md3Radio) {
        suggestedRadio = RIL_CAPABILITY_CDMA_ONLY;
    }
    logD(RAT_CTRL_TAG, "[getSuggestedRadio] %d", suggestedRadio);

    return suggestedRadio;
}

void RpNwWwopRatController::updateState(NwsMode nwsMode, int prefNwType) {
    logD(RAT_CTRL_TAG, "[updateState] nwsMode: %d, prefNwType: %d", nwsMode, prefNwType);
    mCurNwsMode = nwsMode;
    mCurPreferedNetWorkType = prefNwType;
    getStatusManager()->setIntValue(RFX_STATUS_KEY_NWS_MODE, mCurNwsMode);
    getStatusManager()->setIntValue(RFX_STATUS_KEY_PREFERRED_NW_TYPE, mCurPreferedNetWorkType);
    sIsInSwitching = false;
    getStatusManager()->setBoolValue(RFX_STATUS_KEY_IS_RAT_MODE_SWITCHING, false);
}

bool RpNwWwopRatController::onHandleResponse(const sp<RfxMessage> &response) {
    int msg_id = response->getId();
    int error;
    int stgCount = 0;
    int prefNwType = -1;
    int source = RADIO_TECH_GROUP_NUM;
    sp<RfxMessage> resToRilj;
    logD(RAT_CTRL_TAG, "[onHandleResponse] %s", requestToString(msg_id));

    switch (msg_id) {
        case RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE:
            error = response->getError();
            if (error != RIL_E_SUCCESS) {
                logD(RAT_CTRL_TAG, "response error: %d, restrictCount: %d, caller is: %s.", error,
                        mRestrictedCount, switchCallerToString(mModemSettings.ratSwitchCaller));
                // M: During ECC, NWS config rat fail, but, we should allow NWS switch phone
                // successfully, otherwise, ECC fail.
                if (mRestrictedCount > 0
                        && mModemSettings.ratSwitchCaller == RAT_SWITCH_NWS
                        && mModemSettings.prefNwType == mCurPreferedNetWorkType) {
                    controlSwitchFlow(ACT_CONFIG_PHONE_MODE);
                } else {
                    controlSwitchFlow(ACT_RAT_SWITCH_FAIL);
                }
            } else {
                controlSwitchFlow(ACT_CONFIG_PHONE_MODE);
            }
            return true;
        case RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE:
            error = response->getError();
            if (error != RIL_E_SUCCESS) {
                logD(RAT_CTRL_TAG, "response error: %d", error);
                resToRilj = RfxMessage::obtainResponse(RIL_E_GENERIC_FAILURE, response);
            } else {
                response->getParcel()->readInt32(&stgCount);
                response->getParcel()->readInt32(&prefNwType);
                logD(RAT_CTRL_TAG, "prefNwType: %d", prefNwType);
                resToRilj = response;
            }
            responseToRilj(resToRilj);
            return true;
        case RIL_LOCAL_C2K_REQUEST_CONFIG_EVDO_MODE:
            error = response->getError();
            if (error != RIL_E_SUCCESS) {
                logD(RAT_CTRL_TAG, "response error: %d", error);
                controlSwitchFlow(ACT_RAT_SWITCH_FAIL);
            } else {
                source = response->getSource();
                if (source == RADIO_TECH_GROUP_C2K) {
                    controlSwitchFlow(ACT_CONFIG_SVLTE_MODE);
                } else {
                    logD(RAT_CTRL_TAG, "response source incorrect: %d", source);
                    controlSwitchFlow(ACT_RAT_SWITCH_FAIL);
                }
            }
            return true;
        case RIL_LOCAL_REQUEST_SET_SVLTE_RAT_MODE:
        case RIL_LOCAL_C2K_REQUEST_SET_SVLTE_RAT_MODE:
            responseSetSvlteMode(response);
            return true;
        case RIL_REQUEST_VOICE_RADIO_TECH:
            responseGetVoiceRadioTech(response);
            return true;
    }

    return false;
}

void RpNwWwopRatController::responseGetVoiceRadioTech(const sp<RfxMessage> &response) {
    int error = response->getError();
    if (error != RIL_E_SUCCESS) {
        // error
        logD(RAT_CTRL_TAG, "[responseGetVoiceRadioTech] response error: %d", error);
        responseToRilj(response);
    } else {
        // ok
        int source = response->getSource();
        int32_t stgCount;
        int32_t radioTech = 0;
        response->getParcel()->readInt32(&stgCount);
        response->getParcel()->readInt32(&radioTech);
        if (source == RADIO_TECH_GROUP_GSM) {
            logD(RAT_CTRL_TAG, "[responseGetVoiceRadioTech] radioTech: %d", radioTech);
            sp<RfxMessage> resToRilj = RfxMessage::obtainResponse(RIL_E_SUCCESS, response);
            resToRilj->getParcel()->writeInt32(1);
            resToRilj->getParcel()->writeInt32(radioTech);
            responseToRilj(resToRilj);
        }
    }
}

void RpNwWwopRatController::doNwSwitchForEngMode(const sp<RfxAction> &action) {
    logD(RAT_CTRL_TAG, "[doNwSwitchForEngMode]");
    if (mCurAppFamilyType == APP_FAM_3GPP2) {
        switch (getEnginenerMode()) {
            case ENGINEER_MODE_CDMA:
                logD(RAT_CTRL_TAG, "CDMA only mode");
                switchNwRat(APP_FAM_3GPP2, PREF_NET_TYPE_CDMA_EVDO_AUTO,
                        NWS_MODE_CDMALTE, RAT_SWITCH_INIT, action, NULL);
                break;
            case ENGINEER_MODE_CSFB:
                logD(RAT_CTRL_TAG, "CSFB only mode");
                switchNwRat(APP_FAM_3GPP2, PREF_NET_TYPE_LTE_GSM_WCDMA,
                        NWS_MODE_CSFB, RAT_SWITCH_INIT, action, NULL);
                break;
            case ENGINEER_MODE_LTE:
                logD(RAT_CTRL_TAG, "LTE only mode");
                switchNwRat(APP_FAM_3GPP2, PREF_NET_TYPE_LTE_ONLY,
                        NWS_MODE_CSFB, RAT_SWITCH_INIT, action, NULL);
                break;
            default:
                logD(RAT_CTRL_TAG, "Auto mode, do nothing");
                break;
        }
    }
}

bool RpNwWwopRatController::onPreviewMessage(const sp<RfxMessage>& message) {
    if (sIsInSwitching == true && message->getId() == RIL_REQUEST_VOICE_RADIO_TECH) {
        logD(RAT_CTRL_TAG, "onPreviewMessage, put %s into pending list",
                requestToString(message->getId()));
        return false;
    } else {
        return true;
    }
}

bool RpNwWwopRatController::onCheckIfResumeMessage(const sp<RfxMessage>& message) {
    if (sIsInSwitching == false) {
        logD(RAT_CTRL_TAG, "resume the request %s",
                requestToString(message->getId()));
        return true;
    } else {
        return false;
    }
}

bool RpNwWwopRatController::onHandleUrc(const sp<RfxMessage>& message) {
    int msg_id = message->getId();
    logD(RAT_CTRL_TAG, "handle urc %s", urcToString(msg_id));

    switch (msg_id) {
        default:
            break;
    }
    return true;
}

void RpNwWwopRatController::onRestrictedModeChanged(RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    int oldMode;
    int Mode;
    if (key == RFX_STATUS_KEY_EMERGENCY_MODE) {
        oldMode = (int) old_value.asBool();
        Mode = (int) value.asBool();
    } else {
        oldMode = old_value.asInt();
        Mode = value.asInt();
    }

    logD(RAT_CTRL_TAG, "onRestrictedModeChanged, key:%d oldMode:%d, Mode:%d",
        key, oldMode, Mode);
    if (Mode > 0 && oldMode == 0) {
        mRestrictedCount++;
    } else if (Mode == 0 && oldMode > 0) {
        mRestrictedCount--;
    }
    logD(RAT_CTRL_TAG, "onRestrictedModeChanged, Restricted Count:%d",
        mRestrictedCount);

    if (mRestrictedCount == 0 && oldMode != 0) {
        logD(RAT_CTRL_TAG, "[onRestrictedModeChanged] do pending record in queue");
        doPendingRatSwitchRecord();
    }
}

const char *RpNwWwopRatController::requestToString(int reqId) {
    switch (reqId) {
        case RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE:
            return "RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE";
        case RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE:
            return "RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE";
        case RIL_LOCAL_C2K_REQUEST_CONFIG_EVDO_MODE:
            return "RIL_LOCAL_C2K_REQUEST_CONFIG_EVDO_MODE";
        case RIL_LOCAL_REQUEST_SET_SVLTE_RAT_MODE:
            return "RIL_LOCAL_REQUEST_SET_SVLTE_RAT_MODE";
        case RIL_LOCAL_C2K_REQUEST_SET_SVLTE_RAT_MODE:
            return "RIL_LOCAL_C2K_REQUEST_SET_SVLTE_RAT_MODE";
        case RIL_REQUEST_VOICE_RADIO_TECH:
            return "RIL_REQUEST_VOICE_RADIO_TECH";
        default:
            logD(RAT_CTRL_TAG, "<UNKNOWN_REQUEST>");
            break;
    }
    return "";
}

const char *RpNwWwopRatController::urcToString(int reqId) {
    switch (reqId) {
        default:
            logD(RAT_CTRL_TAG, "<UNKNOWN_URC>");
            break;
    }
    return "";
}

const char *RpNwWwopRatController::sourceToString(RILD_RadioTechnology_Group srcId) {
    switch (srcId) {
        case RADIO_TECH_GROUP_GSM:
            return "GSM";
        case RADIO_TECH_GROUP_C2K:
            return "C2K";
        default:
            return "INVALID SRC";
    }
}

const char *RpNwWwopRatController::switchCallerToString(int callerEnum) {
    switch (callerEnum) {
        case RAT_SWITCH_INIT:
            return "INIT";
        case RAT_SWITCH_NWS:
            return "NWS";
        case RAT_SWITCH_NORMAL:
            return "NOR";
        case RAT_SWITCH_RESTRICT:
            return "RES";
        default:
            logD(RAT_CTRL_TAG, "<UNKNOWN> %d", callerEnum);
            break;
    }
    return "";
}

NwsMode RpNwWwopRatController::getNwsMode() {
    return mCurNwsMode;
}

AppFamilyType RpNwWwopRatController::getAppFamilyType() {
    return mCurAppFamilyType;
}

int RpNwWwopRatController::getPreferredNetworkType() {
    return mCurPreferedNetWorkType;
}

int RpNwWwopRatController::getEnginenerMode() {
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("persist.vendor.radio.ct.ir.engmode", property_value, "0");
    int engineerMode = atoi(property_value);
    return engineerMode;
}

bool RpNwWwopRatController::isNoSimInserted() {
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

bool RpNwWwopRatController::isCdmaOnlySim() {
    bool ret = false;
    int nCardType = getStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE);
    if (getStatusManager()->getBoolValue(RFX_STATUS_KEY_CT3G_DUALMODE_CARD)) {
        logD(RAT_CTRL_TAG, "[isCdmaOnlySim] is C2K 3G dual mode card");
        return false;
    }
    if (nCardType == RFX_CARD_TYPE_RUIM) {
        ret = true;
    }
    logD(RAT_CTRL_TAG, "[isCdmaOnlySim] CardType:%d", nCardType);
    return ret;
}

bool RpNwWwopRatController::isCdmaDualModeSimCard() {
    if (getStatusManager()->getBoolValue(RFX_STATUS_KEY_CT3G_DUALMODE_CARD)) {
        logD(RAT_CTRL_TAG, "isCdmaDualModeSimCard, is CT3G dual mode card");
        return true;
    } else {
        int nCardType = getStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE);
        bool ret = RFX_FLAG_HAS_ALL(nCardType, (RFX_CARD_TYPE_USIM | RFX_CARD_TYPE_CSIM));
        logD(RAT_CTRL_TAG, "isCdmaDualModeSimCard, nCardType=0x%x, ret = %d", nCardType, ret);
        return ret;
    }
}

int RpNwWwopRatController::getMajorSlotId() {
    char tempstr[PROPERTY_VALUE_MAX];
    memset(tempstr, 0, sizeof(tempstr));
    property_get("persist.vendor.radio.simswitch", tempstr, "1");
    int majorSlotId = atoi(tempstr) - 1;
    return majorSlotId;
}

int RpNwWwopRatController::getCdmaSlotId() {
    int cdmaSlotId = getNonSlotScopeStatusManager()->getIntValue(RFX_STATUS_KEY_CDMA_SOCKET_SLOT,
            RFX_SLOT_ID_UNKNOWN);
    return cdmaSlotId;
}

bool RpNwWwopRatController::isEratExtSupport() {
    bool isEratExtSupport = false;
    char eratext[PROPERTY_VALUE_MAX] = { 0 };
    property_get("vendor.ril.nw.erat.ext.support", eratext, "-1");
    if (0 == strcmp(eratext, "1")) {
        isEratExtSupport = true;
    }
    return isEratExtSupport;
}

void RpNwWwopRatController::setPreferredNetWorkTypeToSysProp(int slotId, int val){
    char PROPERTY_RILJ_NW_TYPE[4][40] = {
        "persist.vendor.radio.rilj_nw_type1",
        "persist.vendor.radio.rilj_nw_type2",
        "persist.vendor.radio.rilj_nw_type3",
        "persist.vendor.radio.rilj_nw_type4",
    };
    setIntSysProp(PROPERTY_RILJ_NW_TYPE[slotId], val);
}

int RpNwWwopRatController::getPreferredNetWorkTypeFromSysProp(int slotId){
    char PROPERTY_RILJ_NW_TYPE[4][40] = {
        "persist.vendor.radio.rilj_nw_type1",
        "persist.vendor.radio.rilj_nw_type2",
        "persist.vendor.radio.rilj_nw_type3",
        "persist.vendor.radio.rilj_nw_type4",
    };
    return getIntSysProp(PROPERTY_RILJ_NW_TYPE[slotId], 10);
}

void RpNwWwopRatController::setIntSysProp(const char *propName, int val) {
    char stgBuf[PROPERTY_VALUE_MAX] = { 0 };
    sprintf(stgBuf, "%d", val);
    property_set(propName, stgBuf);
}

int RpNwWwopRatController::getIntSysProp(const char *propName, int defaultVal) {
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

int RpNwWwopRatController::isNewSimCard(int slotId) {
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
    logD(RAT_CTRL_TAG, "[isNewSimCard] return:%d", result);
    return result;
}
