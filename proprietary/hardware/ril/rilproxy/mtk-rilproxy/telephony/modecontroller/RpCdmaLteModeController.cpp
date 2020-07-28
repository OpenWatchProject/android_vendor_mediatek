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

#include <assert.h>
#include <cutils/properties.h>
#include <stdlib.h>
#include <string.h>
#include "at_tok.h"
#include "nw/RpNwRatController.h"
#include "power/RpModemController.h"
#include "RfxAction.h"
#include "RfxLog.h"
#include "RpCardModeControllerFactory.h"
#include "RpCardTypeReadyController.h"
#include "RpCdmaLteModeController.h"
#include "utils/String8.h"
#include "power/RadioConstants.h"
#include "RfxTimer.h"
#include "sim/RpSimController.h"
#include "RfxRootController.h"
#include "RfxSocketStateManager.h"
#include "power/RpRadioController.h"
#include "util/RpFeatureOptionUtils.h"
#include "RpModeInitController.h"
#include "capabilityswitch/RpCapabilityGetController.h"
#include <libmtkrilutils.h>
#include "ratconfig.h"

using ::android::String8;

/*****************************************************************************
 * Class RpCdmaLteModeController
 *****************************************************************************/

RFX_IMPLEMENT_CLASS("RpCdmaLteModeController", RpCdmaLteModeController, RfxController);

int RpCdmaLteModeController::sCdmaSocketSlotId = getCdmaSocketSlotId();
int RpCdmaLteModeController::sIsSvlteSupport = isSvlteSupport();

RpCdmaLteModeController::RpCdmaLteModeController() {
    mPendingRecord = new PendingSwitchRecord();
    mSwitchInfo = new ModeSwitchInfo;

    mCdmaLteModeSlotId = getActiveCdmaLteModeSlotId();
    for (int slotId = SIM_ID_1; slotId < RFX_SLOT_COUNT; slotId++) {
        mSwitchInfo->card_types[slotId] = CARD_TYPE_INVALID;
        mSwitchInfo->card_states[slotId] = CARD_STATE_INVALID;
        mSwitchInfo->cardTypeBeSwitch[slotId] = INVALID_MODE;
        mSwitchInfo->isCt3GDualMode[slotId] = false;
        if (slotId == mCdmaLteModeSlotId) {
            mSwitchInfo->card_modes[mCdmaLteModeSlotId] = RADIO_TECH_MODE_CDMALTE;
            mSwitchInfo->old_card_modes[mCdmaLteModeSlotId] = RADIO_TECH_MODE_CDMALTE;
        } else {
            mSwitchInfo->card_modes[slotId] = RADIO_TECH_MODE_CSFB;
            mSwitchInfo->old_card_modes[slotId] = RADIO_TECH_MODE_CSFB;
        }
        mSwitchInfo->rat_mode[slotId] = RAT_MODE_INVALID;
    }
    mSwitchInfo->closeRadioCount = 0;
    mSwitchInfo->isEccCase = false;
    mSwitchInfo->mOldCdmaSocketSlotId = 0;

    mCardTypeReadyController = RFX_OBJ_GET_INSTANCE(RpCardTypeReadyController);

    mIsListeningCall = false;
}

RpCdmaLteModeController::~RpCdmaLteModeController() {
    delete mSwitchInfo;
    delete mPendingRecord;
    RFX_OBJ_CLOSE_INSTANCE(RpCardTypeReadyController);
}

void RpCdmaLteModeController::onInit() {
    RfxController::onInit();
    const int request_id_list[] = {
        RIL_REQUEST_ENTER_RESTRICT_MODEM,
        RIL_REQUEST_ENTER_RESTRICT_MODEM_C2K,
        RIL_REQUEST_LEAVE_RESTRICT_MODEM,
        RIL_REQUEST_LEAVE_RESTRICT_MODEM_C2K,
        RILPROXY_LOCAL_REQUEST_DEAL_PENDING_MODE_SWITCH,
        RIL_REQUEST_SWITCH_MODE_FOR_ECC
    };
    if (RpFeatureOptionUtils::isC2kSupport()) {
        for (int index = 0; index < RFX_SLOT_COUNT; index++) {
            registerToHandleRequest(index, request_id_list, sizeof(request_id_list)/sizeof(const int));
        }
    }
}

void RpCdmaLteModeController::onDeinit() {
    mSwitchInfo->switchQueue.clear();

    RfxController::onDeinit();
}

void RpCdmaLteModeController::enterModeSwitch(int* card_types, int* card_state, bool* ct3gDualMode,
        int slotNum) {
    if (!isEnableSwitchMode(card_types, card_state, ct3gDualMode, slotNum)) {
        return;
    }

    cancelListeningCallStatus();

    for (int slotId = SIM_ID_1; slotId < slotNum; slotId++) {
        mSwitchInfo->card_types[slotId] = card_types[slotId];
        mSwitchInfo->card_states[slotId] = card_state[slotId];
        mSwitchInfo->old_card_modes[slotId] = mSwitchInfo->card_modes[slotId];
        mSwitchInfo->isCt3GDualMode[slotId] = ct3gDualMode[slotId];
    }

    if (waitC2KSocketConnectDone()) {
        switchState(STATE_PREPARE_FOR_SWITCH);
    }
}

void RpCdmaLteModeController::onC2kSocketConnected(int slotId, RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    RfxSocketState newSocketState = value.asSocketState();
    RfxSocketState oldSocketState = old_value.asSocketState();

    RFX_LOG_D(RP_CDMALTE_MODE_TAG, "[SMC][onC2kSocketConnected] Slot %d, SocketState : %s -> %s",
            slotId, oldSocketState.toString().string(), newSocketState.toString().string());
    if (newSocketState.getSocketState(RfxSocketState::SOCKET_C2K)
            && !oldSocketState.getSocketState(RfxSocketState::SOCKET_C2K)) {
        getStatusManager(slotId)->unRegisterStatusChangedEx(RFX_STATUS_KEY_SOCKET_STATE,
                RfxStatusChangeCallbackEx(this, &RpCdmaLteModeController::onC2kSocketConnected));
        int modemOffState = getNonSlotScopeStatusManager()->getIntValue(
                RFX_STATUS_KEY_MODEM_OFF_STATE, MODEM_OFF_IN_IDLE);
        if (modemOffState != MODEM_OFF_BY_MODE_SWITCH) {
            enterModeSwitch(mSwitchInfo->card_types, mSwitchInfo->card_states,
                mSwitchInfo->isCt3GDualMode, RFX_SLOT_COUNT);
        } else {
            RFX_LOG_D(RP_CDMALTE_MODE_TAG,
                    "[SMC][onC2kSocketConnected] Cancel due to modemOffState = %d",
                    modemOffState);
        }
    }
}

void RpCdmaLteModeController::onCardTypeReady(int* card_types, int* card_state, int slotNum,
    CardTypeReadyReason ctrr) {
    RFX_UNUSED(ctrr);
    assert(slotNum != RFX_SLOT_COUNT);
    for (int i = 0; i < slotNum; i++) {
        RFX_LOG_D(RP_CDMALTE_MODE_TAG,
                "[SMC][onCardTypeReady] Slot:%d, card type:%d, card state:%d",
                i, card_types[i], card_state[i]);
    }

    mPendingRecord->cancel();
    bool* ct3gDualMode = new bool[RFX_SLOT_COUNT];
    for (int slotId = 0; slotId<RFX_SLOT_COUNT; slotId ++) {
        ct3gDualMode[slotId] = isCt3GDualMode(slotId, card_types[slotId]);
        /**
         * M: ISSUE: ALPS04788236
         * In common slot load, if CT4G + CT3G are plugin, will trigger RpCdmaLteModeController
         * to switch card type for CT3G card and do not switch network for any card and change
         * all cards status to no_change. When CT3G is changed to GSM, will tigger mode_switch
         * again.But, because the CT4G card status has alread been changed to NO_CHAGNE, so NW
         * will no switch network.
         * So, we should change the CT4G card status to HOT_PLUGIN for the up special case.
         */
        if (isCommontSlotSupport() && ct3gDualMode[slotId]
                && card_state[slotId] == CARD_STATE_CARD_TYPE_CHANGED
                && card_types[slotId] == RFX_CARD_TYPE_SIM) {
            for (int i = 0; i < RFX_SLOT_COUNT; i++) {
                if (slotId != i && is4GCdmaCard(card_types[i])
                        && card_state[i] == CARD_STATE_NO_CHANGED) {
                    card_state[i] = CARD_STATE_HOT_PLUGIN;
                    RFX_LOG_D(RP_CDMALTE_MODE_TAG,
                            "[SMC][onCardTypeReady] COMMON_SLOT, \
                            :card [%d] to CARD_STATE_HOT_PLUGIN", i);
                }
            }
        }
    }
    enterModeSwitch(card_types, card_state, ct3gDualMode, slotNum);
    delete[] ct3gDualMode;
}

bool RpCdmaLteModeController::isEnableSwitchMode(int* card_types, int* card_state, bool* ct3gDualMode,
        int slotNum) {
     getNonSlotScopeStatusManager()->setIntValue(RFX_STATUS_KEY_CDMALTE_MODE_SLOT_READY,
            CDMALTE_MODE_NOT_READY);
    int modemOffState = getNonSlotScopeStatusManager()->getIntValue(RFX_STATUS_KEY_MODEM_OFF_STATE,
            MODEM_OFF_IN_IDLE);
    int worldModeState = getNonSlotScopeStatusManager()->getIntValue(
            RFX_STATUS_KEY_GSM_WORLD_MODE_STATE, 1);
    if (modemOffState == MODEM_OFF_BY_POWER_OFF
        || (sIsSvlteSupport && (modemOffState == MODEM_OFF_BY_SIM_SWITCH
        || modemOffState == MODEM_OFF_BY_RESET_RADIO))) {
        RFX_LOG_D(RP_CDMALTE_MODE_TAG,
                "[SMC][isEnableSwitchMode] Return, modemOffState = %d", modemOffState);
        return false;
    } else if (modemOffState == MODEM_OFF_BY_MODE_SWITCH
            || modemOffState == MODEM_OFF_BY_SIM_SWITCH
            || worldModeState == 0) {
        RFX_LOG_D(RP_CDMALTE_MODE_TAG, "[SMC][isEnableSwitchMode] Pended");
        if (mIsListeningCall) {
            updatePendingRequestInfo(card_types, ct3gDualMode, true);
        } else {
            mPendingRecord->save(card_types, card_state, ct3gDualMode, slotNum);
        }

        if (modemOffState == MODEM_OFF_BY_SIM_SWITCH) {
            getNonSlotScopeStatusManager()->registerStatusChangedEx(RFX_STATUS_KEY_MODEM_OFF_STATE,
                    RfxStatusChangeCallbackEx(this, &RpCdmaLteModeController::onModemOffStateChanged));
            RFX_LOG_D(RP_CDMALTE_MODE_TAG, "[SMC]listen modem_off_state,"
                    " for modemOffState == MODEM_OFF_BY_SIM_SWITCH");
        } else if (worldModeState == 0) {
            getNonSlotScopeStatusManager()->registerStatusChangedEx(
                    RFX_STATUS_KEY_GSM_WORLD_MODE_STATE, RfxStatusChangeCallbackEx(this,
                    &RpCdmaLteModeController::onWorldModeStateChange));
            RFX_LOG_D(RP_CDMALTE_MODE_TAG, "[SMC]listen WORL_MODE_STATE,"
                    " for worldModeState == 0");
        }
        return false;
    } else {
        return true;
    }
}

void RpCdmaLteModeController::onWorldModeStateChange(int slotId, RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    int worldModeState = value.asInt();
    RFX_LOG_D(RP_CDMALTE_MODE_TAG,
                "[SMC][onWorldModeStateChange] slotId:%d, oldValue:%d, newValue%d",
                slotId, old_value.asInt(), worldModeState);
    if (worldModeState != 0) {
        getNonSlotScopeStatusManager()->unRegisterStatusChangedEx(RFX_STATUS_KEY_GSM_WORLD_MODE_STATE,
                    RfxStatusChangeCallbackEx(this, &RpCdmaLteModeController::onWorldModeStateChange));
        dealPendedModeSwitch();
    }
}

void RpCdmaLteModeController::onModemOffStateChanged(int slotId, RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    int modemOffState = value.asInt();
    switch (modemOffState) {
        case MODEM_OFF_BY_MODE_SWITCH:
        case MODEM_OFF_BY_POWER_OFF:
            getNonSlotScopeStatusManager()->unRegisterStatusChangedEx(RFX_STATUS_KEY_MODEM_OFF_STATE,
                    RfxStatusChangeCallbackEx(this, &RpCdmaLteModeController::onModemOffStateChanged));
            RFX_LOG_D(RP_CDMALTE_MODE_TAG,
                    "[SMC][onModemOffStateChanged]modemOffState = %d, cancel callback", modemOffState);
            break;
        case MODEM_OFF_IN_IDLE:
            getNonSlotScopeStatusManager()->unRegisterStatusChangedEx(RFX_STATUS_KEY_MODEM_OFF_STATE,
                    RfxStatusChangeCallbackEx(this, &RpCdmaLteModeController::onModemOffStateChanged));
            dealPendedModeSwitch();
            break;
        default:
            break;
     }
}

void RpCdmaLteModeController::calculateCardMode() {
    RpCardModeControllerFactory::getRpCardModeController()->calculateCardMode(mSwitchInfo);
}

void RpCdmaLteModeController::configModemStatus(int* card_types, int slotNum) {
    int* cardTypes = new int[slotNum +1];
    cardTypes[slotNum] = CARD_TYPE_NONE;
    for (int index = 0; index < slotNum; index++) {
        if (mSwitchInfo->isCt3GDualMode[index]) {
            cardTypes[index] = CARD_TYPE_RUIM;
        } else {
            cardTypes[index] = card_types[index];
        }
    }
    RFX_LOG_D(RP_CDMALTE_MODE_TAG, "[SMC][configModemStatus] start. cdmaSlot : %d,"
            " cardType0:%d, cardType1:%d", sCdmaSocketSlotId, cardTypes[0], cardTypes[1]);
    sp<RfxAction> action = new RfxAction1<int>(this,
            &RpCdmaLteModeController::onConfigModemStatusFinished, sCdmaSocketSlotId);
    RpModemController::getInstance() -> configModemStatus(cardTypes[0], cardTypes[1],
            sCdmaSocketSlotId, action);
    delete[] cardTypes;
}

void RpCdmaLteModeController::onConfigModemStatusFinished(int cSlotId) {
    RFX_LOG_D(RP_CDMALTE_MODE_TAG, "[SMC][configModemStatus] finished. cdmaSlot : %d", cSlotId);
    getNonSlotScopeStatusManager()->setIntValue(RFX_STATUS_KEY_CSLOT_CHANGED_STATUS,
               CSLOT_WILL_NOT_CHANGED);
    switchState(STATE_SWITCH_RADIO_TECHNOLOGY);
}

int RpCdmaLteModeController::getCurrentNetworkType(int slotId) {
    char tempstr[PROPERTY_VALUE_MAX];
    int netWorkType = -1;

    int isLteSupport = (RatConfig_isLteTddSupported() || RatConfig_isLteFddSupported());

    memset(tempstr, 0, sizeof(tempstr));
    property_get("persist.vendor.radio.simswitch", tempstr, "1");
    int  capabilitySlotId  = atoi(tempstr) - 1;
    if (mSwitchInfo->card_modes[slotId] == RADIO_TECH_MODE_CDMALTE) {
        int slotCapability = getStatusManager(slotId)->getIntValue(RFX_STATUS_KEY_SLOT_CAPABILITY, 0);
        int  isC2KSupport  = RatConfig_isC2kSupported();
        if (isLteSupport && slotId == capabilitySlotId && !mSwitchInfo->isCt3GDualMode[slotId]
                && mSwitchInfo->card_types[slotId] != CARD_TYPE_NONE
                && !(mSwitchInfo->card_types[slotId] == CARD_TYPE_RUIM
                || mSwitchInfo->card_types[slotId] == CARD_TYPE_CSIM
                || mSwitchInfo->card_types[slotId] == (CARD_TYPE_CSIM + CARD_TYPE_RUIM))) {
            netWorkType = PREF_NET_TYPE_LTE_CDMA_EVDO;
        } else if (RpFeatureOptionUtils::isCtVolteSupport()
                && RpFeatureOptionUtils::isMultipleImsSupport()
                && (slotCapability & RAF_LTE) == RAF_LTE
                && isLteSupport && !mSwitchInfo->isCt3GDualMode[slotId]
                && mSwitchInfo->card_types[slotId] != CARD_TYPE_NONE
                && !(mSwitchInfo->card_types[slotId] == CARD_TYPE_RUIM
                || mSwitchInfo->card_types[slotId] == CARD_TYPE_CSIM
                || mSwitchInfo->card_types[slotId] == (CARD_TYPE_CSIM + CARD_TYPE_RUIM))) {
            netWorkType = PREF_NET_TYPE_LTE_CDMA_EVDO;
       } else {
            netWorkType = PREF_NET_TYPE_CDMA_EVDO_AUTO;
        }
    } else if (mSwitchInfo->card_modes[slotId] == RADIO_TECH_MODE_CSFB) {
        if (mSwitchInfo->isCt3GDualMode[slotId]) {
            // For CDMA 3G Card Case:
            if (getRealCCardCount() == 1) {
                netWorkType = PREF_NET_TYPE_CDMA_EVDO_AUTO;
            } else {
                if (slotId == capabilitySlotId) {
                    netWorkType = PREF_NET_TYPE_CDMA_EVDO_AUTO;
                } else {
                    int slotCapability = getStatusManager(slotId)->getIntValue(RFX_STATUS_KEY_SLOT_CAPABILITY, 0);
                    RFX_LOG_D(RP_CDMALTE_MODE_TAG, "[SMC][getCurrentNetworkType] C card slotCapability: %d",
                            slotCapability);
                    if ((slotCapability & RAF_UMTS) == RAF_UMTS) {
                        netWorkType = PREF_NET_TYPE_GSM_WCDMA;
                    } else {
                        netWorkType = PREF_NET_TYPE_GSM_ONLY;
                    }
                }
            }
        } else if (is4GCdmaCard(mSwitchInfo->card_types[slotId])) {
            // CDMA 4G Card Case:
            if (slotId == capabilitySlotId) {
                if (isLteSupport) {
                    netWorkType = PREF_NET_TYPE_LTE_GSM_WCDMA;
                } else {
                    netWorkType = PREF_NET_TYPE_GSM_WCDMA;
                }
            } else {
                int slotCapability = getStatusManager(slotId)->getIntValue(RFX_STATUS_KEY_SLOT_CAPABILITY, 0);
                if ((slotCapability & RAF_UMTS) == RAF_UMTS) {
                    if (isLteSupport && (slotCapability & RAF_LTE) == RAF_LTE) {
                        netWorkType = PREF_NET_TYPE_LTE_GSM_WCDMA;
                    } else {
                        netWorkType = PREF_NET_TYPE_GSM_WCDMA;
                    }
                } else if (isLteSupport && (slotCapability & RAF_LTE) == RAF_LTE) {
                    netWorkType = PREF_NET_TYPE_LTE_GSM;
                } else {
                    netWorkType = PREF_NET_TYPE_GSM_ONLY;
                }
            }
        } else {
            // G Card Case:
            if (slotId == capabilitySlotId) {
                if (isLteSupport) {
                    netWorkType = PREF_NET_TYPE_LTE_GSM_WCDMA;
                } else {
                    netWorkType = PREF_NET_TYPE_GSM_WCDMA;
                }
            } else {
                int slotCapability = getStatusManager(slotId)->getIntValue(RFX_STATUS_KEY_SLOT_CAPABILITY, 0);
                if ((slotCapability & RAF_UMTS) == RAF_UMTS) {
                    if (isLteSupport && (slotCapability & RAF_LTE) == RAF_LTE) {
                        netWorkType = PREF_NET_TYPE_LTE_GSM_WCDMA;
                    } else {
                        netWorkType = PREF_NET_TYPE_GSM_WCDMA;
                    }
                } else if (isLteSupport && (slotCapability & RAF_LTE) == RAF_LTE) {
                    netWorkType = PREF_NET_TYPE_LTE_GSM;
                } else {
                    netWorkType = PREF_NET_TYPE_GSM_ONLY;
                }
            }
        }
    } else {
        assert(0);
    }

    RFX_LOG_D(RP_CDMALTE_MODE_TAG, "[SMC][getCurrentNetworkType]isLteSupport: %d,"
            " capabilitySlotId: %d, netWorkType: %d, getIs4GCCard[%d]: %d",
            isLteSupport, capabilitySlotId, netWorkType, slotId,
            (is4GCdmaCard(mSwitchInfo->card_types[slotId]) == true) ? 1 : 0);
    return netWorkType;
}

int RpCdmaLteModeController::getCCardCount() {
    int cCardNum = 0;
    char tempstr[PROPERTY_VALUE_MAX];
    for (int slotId = 0; slotId < RFX_SLOT_COUNT; slotId++) {
        memset(tempstr, 0, sizeof(tempstr));
        property_get(PROPERTY_RIL_FULL_UICC_TYPE[slotId], tempstr, "");
        if ((strstr(tempstr, "CSIM") != NULL) || (strstr(tempstr, "RUIM") != NULL)) {
            cCardNum++;
        }
    }
    return cCardNum;
}

int RpCdmaLteModeController::getRealCCardCount() {
    int cCardNum = 0;
    char tempstr[PROPERTY_VALUE_MAX];
    for (int slotId = 0; slotId < RFX_SLOT_COUNT; slotId++) {
        memset(tempstr, 0, sizeof(tempstr));
        property_get(PROPERTY_RIL_FULL_UICC_TYPE[slotId], tempstr, "");
        if ((strstr(tempstr, "CSIM") != NULL) || (strstr(tempstr, "RUIM") != NULL)) {
            cCardNum++;
        } else if ((strstr(tempstr, "SIM") != NULL) || (strstr(tempstr, "USIM") != NULL)) {
            if (mSwitchInfo->isCt3GDualMode[slotId]) {
                cCardNum++;
            }
        }
    }
    return cCardNum;
}

int RpCdmaLteModeController::getFirstCCardSlot() {
    char tempstr[PROPERTY_VALUE_MAX];
    for (int slotId = 0; slotId < RFX_SLOT_COUNT; slotId++) {
        getUiccType(slotId, tempstr, PROPERTY_VALUE_MAX);
        if (isCdmaCard(tempstr)) {
            return slotId;
        }
    }
    return -1;
}

bool RpCdmaLteModeController::isCdmaCard(char* uicc_type) {
    if ((strstr(uicc_type, "CSIM") != NULL) || (strstr(uicc_type, "RUIM") != NULL)) {
        return true;
    }
    return false;
}

void RpCdmaLteModeController::getUiccType(int slotId, char* uiccType, int uiccTypeLength) {
    memset(uiccType, 0, uiccTypeLength);
    property_get(PROPERTY_RIL_FULL_UICC_TYPE[slotId], uiccType, "");
}

int RpCdmaLteModeController::getDefaultNetworkType(int slotId) {
    return getCurrentNetworkType(slotId);
}

void RpCdmaLteModeController::setupSwitchQueue(int old_cdma_socket_slot) {
    int firstSwitchSlot = SIM_ID_1;
    int secondSwitchSlot = SIM_ID_2;
    mSwitchInfo->switchQueue.clear();

    if (RFX_SLOT_COUNT == 1) {
        mSwitchInfo->switchQueue.add(firstSwitchSlot);
        return;
    }

    if (old_cdma_socket_slot != sCdmaSocketSlotId) {
        firstSwitchSlot = old_cdma_socket_slot;
        secondSwitchSlot = sCdmaSocketSlotId;
    }

    mSwitchInfo->switchQueue.add(firstSwitchSlot);
    mSwitchInfo->switchQueue.add(secondSwitchSlot);

    if (RFX_SLOT_COUNT > 2) {
        for (int i = 0; i < RFX_SLOT_COUNT; i++) {
            if (i != firstSwitchSlot && i != secondSwitchSlot) {
                mSwitchInfo->switchQueue.add(i);
            }
        }
    }
    assert(mSwitchInfo->switchQueue.size() > 0
            && mSwitchInfo->switchQueue.size() <= RFX_SLOT_COUNT);
}

bool RpCdmaLteModeController::needSwtichCardType() {
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("persist.vendor.radio.simswitch", property_value, "1");
    int capabilitySlot = atoi(property_value) - 1;
    for (int index = 0; index < RFX_SLOT_COUNT; index++) {
        RpNwRatController* nwRatController =
                (RpNwRatController *)findController(index,
                RFX_OBJ_CLASS_INFO(RpNwRatController));
        if (mSwitchInfo->cardTypeBeSwitch[index] > INVALID_MODE) {
            return true;
        } else if (RFX_SLOT_COUNT == 2 && mSwitchInfo->isCt3GDualMode[index]
                && mSwitchInfo->card_types[index] == CARD_TYPE_SIM
                && !containsCdma(mSwitchInfo->card_types[index == 0 ? 1 : 0])
                && ((!mSwitchInfo->isCt3GDualMode[index == 0 ? 1 : 0]) ||
                        ((mSwitchInfo->isCt3GDualMode[index == 0 ? 1 : 0]) && capabilitySlot == index))
                // getNwsModeForSwitchCardType only valid when AppFamilyType is APP_FAM_3GPP2
                // Invalid if we give AppFamilyType 3GPP for CT3G card previous.
                && ((nwRatController->getNwsModeForSwitchCardType() != NWS_MODE_CSFB
                && nwRatController->getAppFamilyType() == APP_FAM_3GPP2)
                || nwRatController->getAppFamilyType() == APP_FAM_3GPP)) {
            mSwitchInfo->cardTypeBeSwitch[index] = RUIM_MODE;
            return true;
        }
    }
    return false;
}

void RpCdmaLteModeController::doSwitchRadioTech() {
    int switchSlot = mSwitchInfo->switchQueue.itemAt(0);
    RFX_LOG_D(RP_CDMALTE_MODE_TAG,
            "[SMC][doSwitchRadioTech] Start switch Slot: %d firstly, switchQueue size : %d",
            switchSlot, mSwitchInfo->switchQueue.size());
    mSwitchInfo->switchQueue.removeAt(0);
    switchRadioTechnology(switchSlot);
}
void RpCdmaLteModeController::onRatSwitchDone(int slotId) {
    RFX_LOG_D(RP_CDMALTE_MODE_TAG, "[SMC][onRatSwitchDone] Slot : %d switch done,"
            " switchQueue size : %d", slotId, mSwitchInfo->switchQueue.size());
    mSwitchInfo->rat_mode[slotId] = RAT_MODE_INVALID;

    // check rat switch result.
    if (!checkEratReslut(slotId)) {
        handleSendEratFailByCall(slotId);
        return;
    }
    if (mSwitchInfo->switchQueue.isEmpty()) {
        if (sIsSvlteSupport) {
            switchState(STATE_RF_ON);
        } else {
            switchState(STATE_LEAVE_RESTRICT_MODE);
        }
     } else if (mSwitchInfo->switchQueue.size() >= 1) {
         int switchSlot = mSwitchInfo->switchQueue.itemAt(0);
         RFX_LOG_D(RP_CDMALTE_MODE_TAG, "[SMC][onRatSwitchDone] Start switch Slot: %d", switchSlot);
         mSwitchInfo->switchQueue.removeAt(0);
         switchRadioTechnology(switchSlot);
    }
}

void RpCdmaLteModeController::switchRadioTechnology(int slotId) {
    sp<RfxAction> action = new RfxAction1<int>(this,
            &RpCdmaLteModeController::onRatSwitchDone, slotId);
    int networkType = getCurrentNetworkType(slotId);
    AppFamilyType app_family_type = getAppFamilyType(networkType);

    RFX_LOG_D(RP_CDMALTE_MODE_TAG,
            "[SMC][switchRadioTechnology] SlotId : %d, %s -> %s, app_family_type : %d, \
            networkType : %d, card_state: %d",
            slotId,
            mSwitchInfo->old_card_modes[slotId] == 3 ? "CDMALTE" : "CSFB",
            mSwitchInfo->card_modes[slotId] == 3 ? "CDMALTE" : "CSFB",
            app_family_type, networkType, mSwitchInfo->card_states[slotId]);
    RpNwRatController* nwRatController =
            (RpNwRatController *)findController(slotId, RFX_OBJ_CLASS_INFO(RpNwRatController));

    RatSwitchInfo switchInfo = {app_family_type,
            networkType,
            mSwitchInfo->card_states[slotId],
            action,
            mSwitchInfo->rat_mode[slotId],
            mSwitchInfo->card_types[slotId],
            mSwitchInfo->isCt3GDualMode[slotId]};

    nwRatController->setPreferredNetworkType(switchInfo);
}

/**
 * Get the SVLTE slot id.
 * @return SVLTE slot id.
 *         0 : svlte in slot1
 *         1 : svlte in slot2
 */
int RpCdmaLteModeController::getActiveCdmaLteModeSlotId() {
    int i = 0;
    char tempstr[PROPERTY_VALUE_MAX];
    char* tok = NULL;

    memset(tempstr, 0, sizeof(tempstr));
    // 3 means SVLTE mode, 2 is CSFB mode in this persist.
    property_get("persist.vendor.radio.svlte_slot", tempstr, "2,2");
    tok = strtok(tempstr, ",");
    while (tok != NULL) {
        if (3 == atoi(tok)) {
            return i;
        }
        i++;
        tok = strtok(NULL, ",");
    }
    return CSFB_ON_SLOT;
}

/**
 * Get slot id which connect to c2k rild socket.
 * @return slot id which connect to c2k rild socket
 *         0 : slot1 connect to c2k rild socket
 *         1 : slot2 connect to c2k rild socket
 */
int RpCdmaLteModeController::getCdmaSocketSlotId() {
    int ret = 0;
    char tempstr[PROPERTY_VALUE_MAX] = { 0 };

    memset(tempstr, 0, sizeof(tempstr));
    property_get("persist.vendor.radio.cdma_slot", tempstr, "1");

    ret = atoi(tempstr) - 1;
    return ret;
}

void RpCdmaLteModeController::setCdmaSocketSlotId(int slotId) {
    RFX_LOG_D(RP_CDMALTE_MODE_TAG, "[SMC]setCdmaSocketSlotId : %d -> %d",
            sCdmaSocketSlotId, slotId);
    if (slotId >= 0 && slotId < RFX_SLOT_COUNT) {
        sCdmaSocketSlotId = slotId;
        char* slot_str = NULL;
        asprintf(&slot_str, "%d", slotId + 1);
        property_set("persist.vendor.radio.cdma_slot", slot_str);
        free(slot_str);
    }
}

void RpCdmaLteModeController::setActiveCdmaLteModeSlotId() {
    int cdmaLteModeSlotId = CSFB_ON_SLOT;
    for (int i = 0;i < RFX_SLOT_COUNT; i++) {
        if (mSwitchInfo->card_modes[i] == RADIO_TECH_MODE_CDMALTE) {
            cdmaLteModeSlotId = i;
            break;
        }
    }

    if (mCdmaLteModeSlotId != cdmaLteModeSlotId) {
        mCdmaLteModeSlotId = cdmaLteModeSlotId;
        String8 cardmodestr;
        cardmodestr.clear();
        for (int i = 0;i < RFX_SLOT_COUNT - 1; i++) {
            cardmodestr.appendFormat("%d,", mSwitchInfo->card_modes[i]);
        }
        cardmodestr.appendFormat("%d", mSwitchInfo->card_modes[RFX_SLOT_COUNT - 1]);
        property_set("persist.vendor.radio.svlte_slot", cardmodestr.string());
        RFX_LOG_D(RP_CDMALTE_MODE_TAG, "[SMC]setActiveCdmaLteModeSlotId : %d,"
                "setupCdmaLteMode : %s", mCdmaLteModeSlotId, cardmodestr.string());
    }
}

void RpCdmaLteModeController::setupCdmaLteMode() {
    setActiveCdmaLteModeSlotId();
    getNonSlotScopeStatusManager()->setIntValue(
            RFX_STATUS_KEY_ACTIVE_CDMALTE_MODE_SLOT, mCdmaLteModeSlotId);
    // remember the old CdmaSoketSlotId.
    mSwitchInfo->mOldCdmaSocketSlotId = sCdmaSocketSlotId;
    setCdmaSocketSlotId(mCdmaLteModeSlotId > -1 ? mCdmaLteModeSlotId : sCdmaSocketSlotId);

    getNonSlotScopeStatusManager()->setIntValue(RFX_STATUS_KEY_CDMA_SOCKET_SLOT, sCdmaSocketSlotId);
    RFX_OBJ_GET_INSTANCE(RfxSocketStateManager)->setCdmaSocketSlotChange(sCdmaSocketSlotId);

    RpCapabilityGetController *capabilityGetController =
                (RpCapabilityGetController *)findController(RFX_SLOT_ID_UNKNOWN,
                RFX_OBJ_CLASS_INFO(RpCapabilityGetController));

    capabilityGetController->updateRadioCapability(sCdmaSocketSlotId);

    getNonSlotScopeStatusManager()->setIntValue(RFX_STATUS_KEY_CDMALTE_MODE_SLOT_READY,
            CDMALTE_MODE_READY);
    RpModeInitController* mic =
            (RpModeInitController *)findController(RFX_OBJ_CLASS_INFO(RpModeInitController));
    mic->mSwitchCdmaSlotFinishSignal.emit();
}

int RpCdmaLteModeController::getNewCdmaSocketSlotId() {
    int cdmaLteModeSlotId = CSFB_ON_SLOT;
    for (int i = 0; i < RFX_SLOT_COUNT; i++) {
        if (mSwitchInfo->card_modes[i] == RADIO_TECH_MODE_CDMALTE) {
            cdmaLteModeSlotId = i;
            break;
        }
    }
    if (cdmaLteModeSlotId == CSFB_ON_SLOT) {
        cdmaLteModeSlotId = sCdmaSocketSlotId;
    }
    return cdmaLteModeSlotId;
}

int RpCdmaLteModeController::getRealCardType(int slotId) {
    bool ct3g = getStatusManager(slotId)->getBoolValue(RFX_STATUS_KEY_CT3G_DUALMODE_CARD, false);
    int cardType = getStatusManager(slotId)->getIntValue(RFX_STATUS_KEY_CARD_TYPE, 0);
    RLOGD("getRealCardType, slot %d: cardType1=%d, ct3g=%d", slotId, cardType, ct3g);
    if (ct3g && cardType > 0) {
        cardType = RFX_CARD_TYPE_RUIM;
    }
    return cardType;

}

void RpCdmaLteModeController::startSwitchMode() {
    getNonSlotScopeStatusManager()->setIntValue(RFX_STATUS_KEY_MODE_SWITCH, MODE_SWITCH_START);
    /// Set Flag for exclusion with SimSwitch & IPO OFF.
    getNonSlotScopeStatusManager()->setIntValue(RFX_STATUS_KEY_MODEM_OFF_STATE,
               MODEM_OFF_BY_MODE_SWITCH);

    mSwitchInfo->isEccCase = false;
    if (sCdmaSocketSlotId != getNewCdmaSocketSlotId()) {
        getNonSlotScopeStatusManager()->setIntValue(RFX_STATUS_KEY_CSLOT_CHANGED_STATUS,
                CSLOT_WILL_BE_CHANGED);
    } else {
        getNonSlotScopeStatusManager()->setIntValue(RFX_STATUS_KEY_CSLOT_CHANGED_STATUS,
                CSLOT_WILL_NOT_CHANGED);
    }
}

bool RpCdmaLteModeController::isEmdStatusChanged(int* card_types, int slotNum) {
    int* cardTypes = new int[slotNum +1];
    cardTypes[slotNum] = CARD_TYPE_NONE;
    for (int index = 0; index < slotNum; index++) {
        if (mSwitchInfo->isCt3GDualMode[index]) {
            cardTypes[index] = CARD_TYPE_RUIM;
        } else {
            cardTypes[index] = card_types[index];
        }
    }
    int newSlotId = getNewCdmaSocketSlotId();
    bool isChanged = RpModemController::getInstance()->isEmdstatusChanged(cardTypes[0],
            cardTypes[1], newSlotId);
    RFX_LOG_D(RP_CDMALTE_MODE_TAG,
            "[SMC]isEmdStatusChanged : %d, cardType0:%d, cardType1:%d, newCdmsSocketSlotId:%d",
            isChanged, cardTypes[0], cardTypes[1], newSlotId);
    delete[] cardTypes;
    return isChanged;
}

void RpCdmaLteModeController::enterRestrictMode() {
    getNonSlotScopeStatusManager()->setIntValue(RFX_STATUS_KEY_RESTRICT_MODE_STATE,
            RESTRICT_MODE_STATE_ON);
    // send ERMS to GSM/C2K RILD
    sp<RfxMessage> parentRequest = RfxMessage::obtainRequest(sCdmaSocketSlotId,
            RADIO_TECH_GROUP_GSM, RIL_REQUEST_ENTER_RESTRICT_MODEM);
    Parcel* parcel = parentRequest->getParcel();
    parcel->writeInt32(1);
    parcel->writeInt32(ENTER_RESTRICT_MODE);
    sp<RfxMessage> gsmRequest = RfxMessage::obtainRequest(RADIO_TECH_GROUP_GSM,
            RIL_REQUEST_ENTER_RESTRICT_MODEM, parentRequest, true);
    sp<RfxMessage> c2kRequest = RfxMessage::obtainRequest(RADIO_TECH_GROUP_C2K,
            RIL_REQUEST_ENTER_RESTRICT_MODEM_C2K, parentRequest, true);
    requestToRild(gsmRequest);
    requestToRild(c2kRequest);
}

/**
 * M: Should Close the C slot's radio and another slot's G Radio.
*/
void RpCdmaLteModeController::requestRadioOff() {
    if (sCdmaSocketSlotId != getNewCdmaSocketSlotId()) {
        for (int slot = 0; slot < RFX_SLOT_COUNT; slot++) {
            bool turnOffCRadio = false;
            bool turnOffGRadio = false;
            if (sCdmaSocketSlotId == slot) {
                turnOffCRadio = true;
                turnOffGRadio = false;
            } else {
                turnOffCRadio = false;
                turnOffGRadio = true;
            }
            RFX_LOG_D(RP_CDMALTE_MODE_TAG, "[SMC]requestRadioOff, slotId:%d, CRadioOff: %d,"
                    " GRadioOff:%d ", slot, turnOffCRadio, turnOffGRadio);
            sp<RfxAction> action0 = new RfxAction1<int>(this,
                    &RpCdmaLteModeController::onRequestRadioOffDone, slot);
            RpRadioController* radioController0 =
                    (RpRadioController *)findController(slot,
                    RFX_OBJ_CLASS_INFO(RpRadioController));
            radioController0->dynamicSwitchRadioOff(turnOffCRadio, turnOffGRadio, action0);
        }
    } else {
        mSwitchInfo->closeRadioCount = RFX_SLOT_COUNT - 1;
        RFX_LOG_D(RP_CDMALTE_MODE_TAG, "[SMC]requestRadioOff, Just slotId:%d", sCdmaSocketSlotId);
        sp<RfxAction> action = new RfxAction1<int>(this,
                &RpCdmaLteModeController::onRequestRadioOffDone, sCdmaSocketSlotId);
        RpRadioController* radioController =
                (RpRadioController *)findController(sCdmaSocketSlotId,
                RFX_OBJ_CLASS_INFO(RpRadioController));
        radioController->dynamicSwitchRadioOff(true, true, action);
    }
}

void RpCdmaLteModeController::onRequestRadioOffDone(int slotId) {
    mSwitchInfo->closeRadioCount++;
    RFX_LOG_D(RP_CDMALTE_MODE_TAG, "[SMC]onRequestRadioOffDone,%d / %d, slotId:%d",
            mSwitchInfo->closeRadioCount, RFX_SLOT_COUNT, slotId);
    if (mSwitchInfo->closeRadioCount == RFX_SLOT_COUNT) {
        mSwitchInfo->closeRadioCount = 0;
        switchState(STATE_CONFIG_MODEM_STATUS);
    }
}

void RpCdmaLteModeController::leaveRestrictMode() {
    // send ERMS to GSM/C2K RILD
    sp<RfxMessage> parentRequest = RfxMessage::obtainRequest(sCdmaSocketSlotId,
            RADIO_TECH_GROUP_GSM, RIL_REQUEST_LEAVE_RESTRICT_MODEM);
    Parcel* parcel = parentRequest->getParcel();
    parcel->writeInt32(1);
    parcel->writeInt32(LEAVE_RESTRICT_MODE);
    sp<RfxMessage> gsmRequest = RfxMessage::obtainRequest(RADIO_TECH_GROUP_GSM,
            RIL_REQUEST_LEAVE_RESTRICT_MODEM, parentRequest, true);
    sp<RfxMessage> c2kRequest = RfxMessage::obtainRequest(RADIO_TECH_GROUP_C2K,
            RIL_REQUEST_LEAVE_RESTRICT_MODEM_C2K, parentRequest, true);

    requestToRild(gsmRequest);
    requestToRild(c2kRequest);
}

void RpCdmaLteModeController::finishSwitchMode() {
    getNonSlotScopeStatusManager()->setIntValue(RFX_STATUS_KEY_MODE_SWITCH, MODE_SWITCH_FINISH);
    getNonSlotScopeStatusManager()->setIntValue(RFX_STATUS_KEY_MODEM_OFF_STATE, MODEM_OFF_IN_IDLE);

    if (mPendingRecord->isPendingState()) {
        RFX_LOG_D(RP_CDMALTE_MODE_TAG, "[SMC][finishSwitchMode]enqueue pending message to"
                " main queue.");
        enqueuePendingRequest();
    }
}

bool RpCdmaLteModeController::is4GCdmaCard(int cardType) {
    if (containsUsim(cardType) && containsCdma(cardType)) {
        return true;
    }
    return false;
}

bool RpCdmaLteModeController::containsCdma(int cardType) {
     if ((cardType & CARD_TYPE_RUIM) > 0 ||
         (cardType & CARD_TYPE_CSIM) > 0) {
         return true;
     }
     return false;
}

bool RpCdmaLteModeController::containsGsm(int cardType) {
     if ((cardType & CARD_TYPE_SIM) > 0 ||
         (cardType & CARD_TYPE_USIM) > 0) {
         return true;
     }
     return false;
}

bool RpCdmaLteModeController::containsUsim(int cardType) {
     if ((cardType & CARD_TYPE_USIM) > 0) {
         return true;
     }
     return false;
}

bool RpCdmaLteModeController::isUsimOnlyCard(int cardType) {
      return (containsUsim(cardType) && !containsCdma(cardType));
}

AppFamilyType RpCdmaLteModeController::getAppFamilyType(int networkType) {
    if (networkType == PREF_NET_TYPE_CDMA_EVDO_AUTO || networkType == PREF_NET_TYPE_LTE_CDMA_EVDO) {
        return APP_FAM_3GPP2;
    } else {
        return APP_FAM_3GPP;
    }
}

bool RpCdmaLteModeController::waitC2KSocketConnectDone() {
    bool isC2kSocketConnected = false;
    RfxSocketState socketState =
            getStatusManager(sCdmaSocketSlotId)->getSocketStateValue(RFX_STATUS_KEY_SOCKET_STATE);
    isC2kSocketConnected = socketState.getSocketState(RfxSocketState::SOCKET_C2K);
    RFX_LOG_D(RP_CDMALTE_MODE_TAG,
            "[SMC]C2KSocketConnected : %d, cdmaSocketSlotId : %d,  socketState : %s",
            isC2kSocketConnected, sCdmaSocketSlotId, socketState.toString().string());
    if (!isC2kSocketConnected) {
        getStatusManager(sCdmaSocketSlotId)->registerStatusChangedEx(RFX_STATUS_KEY_SOCKET_STATE,
                RfxStatusChangeCallbackEx(this, &RpCdmaLteModeController::onC2kSocketConnected));
        RFX_LOG_D(RP_CDMALTE_MODE_TAG,
                "[SMC]Slot %d wait onC2kSocketConnected", sCdmaSocketSlotId);
    }
    return isC2kSocketConnected;
}

bool RpCdmaLteModeController::onHandleRequest(const sp<RfxMessage>& message) {
    int msg_id = message->getId();

    switch (msg_id) {
        case RILPROXY_LOCAL_REQUEST_DEAL_PENDING_MODE_SWITCH:
            RFX_LOG_D(RP_CDMALTE_MODE_TAG,
                    "[SMC]onHandleRequest, REQUEST_DEAL_PENDING_MODE_SWITCH, msgToken:%d",
                    message->getToken());
            dealPendedModeSwitch();
            break;
        case RIL_REQUEST_SWITCH_MODE_FOR_ECC:
            switchModeForECC(message);
            break;
        default:
            break;
    }
    return true;
}

void RpCdmaLteModeController::enqueuePendingRequest() {
    sp<RfxMessage> pendingRequest = RfxMessage::obtainRequest(0, RADIO_TECH_GROUP_C2K,
        RILPROXY_LOCAL_REQUEST_DEAL_PENDING_MODE_SWITCH);
    RfxMainThread::enqueueMessage(pendingRequest);
}

void RpCdmaLteModeController::dealPendedModeSwitch() {
    int modemOffState = getNonSlotScopeStatusManager()->getIntValue(
            RFX_STATUS_KEY_MODEM_OFF_STATE, MODEM_OFF_IN_IDLE);
    if (mPendingRecord->isPendingState() && modemOffState != MODEM_OFF_BY_MODE_SWITCH) {
        RFX_LOG_D(RP_CDMALTE_MODE_TAG, "[SMC]Start switch mode pended");
        applyPendingRecord();
        enterModeSwitch(mSwitchInfo->card_types, mSwitchInfo->card_states,
                mSwitchInfo->isCt3GDualMode, RFX_SLOT_COUNT);
     }
}
bool RpCdmaLteModeController::onHandleResponse(const sp<RfxMessage>& message) {
    int msg_id = message->getId();
    RFX_LOG_D(RP_CDMALTE_MODE_TAG,
            "[SMC]onHandleResponse, msgId: %d", msg_id);
    switch (msg_id) {
        case RIL_REQUEST_ENTER_RESTRICT_MODEM:
        case RIL_REQUEST_ENTER_RESTRICT_MODEM_C2K: {
            if (message->getError() != RIL_E_SUCCESS) {
                RFX_LOG_E(RP_CDMALTE_MODE_TAG,
                    "[SMC]onHandleResponse, requestId:%d, msgToken: %d  FAILED!!!",
                    msg_id, message->getToken());
                return true;
            }
            sp<RfxMessage> msg = sp<RfxMessage>(NULL);
            ResponseStatus responseStatus;
            responseStatus = preprocessResponse(message, msg);
            if (responseStatus == RESPONSE_STATUS_HAVE_MATCHED) {
                switchState(STATE_RF_OFF);
            }
            break;
        }
        case RIL_REQUEST_LEAVE_RESTRICT_MODEM:
        case RIL_REQUEST_LEAVE_RESTRICT_MODEM_C2K: {
            if (message->getError() != RIL_E_SUCCESS) {
                RFX_LOG_E(RP_CDMALTE_MODE_TAG,
                    "[SMC]onHandleResponse, requestId:%d, msgToken: %d  FAILED!!!",
                    msg_id, message->getToken());
                return true;
            }
            sp<RfxMessage> msg = sp<RfxMessage>(NULL);
            ResponseStatus responseStatus;
            responseStatus = preprocessResponse(message, msg);
            if (responseStatus == RESPONSE_STATUS_HAVE_MATCHED) {
                getNonSlotScopeStatusManager()->setIntValue(RFX_STATUS_KEY_RESTRICT_MODE_STATE,
                        RESTRICT_MODE_STATE_OFF);
                switchState(STATE_RF_ON);
            }
            break;
        }
        default:
            break;
    }
    return true;
}

void RpCdmaLteModeController::responseTimedoutCallBack(const sp<RfxMessage>& message) {
    RFX_LOG_D(RP_CDMALTE_MODE_TAG,
            "[SMC]responseTimedoutCallBack, requestId:%d, msgToken: %d",
            message->getId(), message->getToken());
}

/**
 * Copy data of mPendingRecord to mSwitchInfo.
 */
void RpCdmaLteModeController::applyPendingRecord() {
    if (mPendingRecord->isPendingState() == false) {
            return;
    }
    mPendingRecord->m_hasPendingRecord = false;
    for (int slotId = 0; slotId < RFX_SLOT_COUNT; slotId++) {
        mSwitchInfo->card_types[slotId] = mPendingRecord->m_pending_card_types[slotId];
        mSwitchInfo->card_states[slotId] = mPendingRecord->m_pending_card_states[slotId];
        mSwitchInfo->isCt3GDualMode[slotId] = mPendingRecord->m_is_3g_dual_mode[slotId];
    }
}

void RpCdmaLteModeController::switchCardType() {
    for (int index = 0; index < RFX_SLOT_COUNT; index++) {
        if (mSwitchInfo->cardTypeBeSwitch[index] > INVALID_MODE) {
            RpSimController *simController = (RpSimController*)findController(index,
                    RFX_OBJ_CLASS_INFO(RpSimController));
            RFX_LOG_D(RP_CDMALTE_MODE_TAG,
                    "[SMC][switchCardType] slot %d :   %s. Exit switchState", index,
                    mSwitchInfo->cardTypeBeSwitch[index] == SIM_MODE ? "C-->G" : "G-->C");
            simController->switchCardType(mSwitchInfo->cardTypeBeSwitch[index], false);

            mSwitchInfo->cardTypeBeSwitch[index] = INVALID_MODE;
            if (!mSwitchInfo->isEccCase) {
                mSwitchInfo->card_modes[index] = mSwitchInfo->old_card_modes[index];
            }
        }
    }
    mSwitchInfo->isEccCase = false;
}

int RpCdmaLteModeController::isSvlteSupport() {
    int isSvlteSupport = 0;
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ro.vendor.mtk_c2k_lte_mode", property_value, "0");
    isSvlteSupport = atoi(property_value);
    return (isSvlteSupport == 1);
}

void RpCdmaLteModeController::triggerTrm() {
    RFX_LOG_D(RP_CDMALTE_MODE_TAG, "[SMC][triggerTrm]sCdmaSocketSlotId = %d, isEccCase = %d",
            sCdmaSocketSlotId, mSwitchInfo->isEccCase);
    getNonSlotScopeStatusManager()->setIntValue(RFX_STATUS_KEY_MODEM_OFF_STATE,
            MODEM_OFF_BY_MODE_SWITCH);
    if (mSwitchInfo->isEccCase) {
        mSwitchInfo->isEccCase = false;
        setupCdmaLteMode();
    }

    sp<RfxMessage> rilC2kRequest;
    sp<RfxMessage> rilGsmRequest;
    rilC2kRequest = RfxMessage::obtainRequest(
            sCdmaSocketSlotId,
            RADIO_TECH_GROUP_C2K,
            RIL_LOCAL_REQUEST_MODE_SWITCH_C2K_SET_TRM);
    requestToRild(rilC2kRequest);

    rilGsmRequest = RfxMessage::obtainRequest(
            sCdmaSocketSlotId,
            RADIO_TECH_GROUP_GSM,
            RIL_LOCAL_REQUEST_MODE_SWITCH_GSM_SET_TRM);
    requestToRild(rilGsmRequest);
}

void RpCdmaLteModeController::requestRadioOn() {
    for (int index =0; index < RFX_SLOT_COUNT; index++) {
        RpRadioController* radioController =
                (RpRadioController *)findController(index,
                RFX_OBJ_CLASS_INFO(RpRadioController));
        if (sIsSvlteSupport) {
            radioController->requestRadioOn();
        } else {
            radioController->dynamicSwitchRadioOn();
        }
    }
}

bool RpCdmaLteModeController::isRemoteSimProtocolChanged(int* card_types, int slotNum) {
    int* cardTypes = new int[slotNum +1];
    cardTypes[slotNum] = CARD_TYPE_NONE;
    for (int index = 0; index < slotNum; index++) {
        if (mSwitchInfo->isCt3GDualMode[index]) {
            cardTypes[index] = CARD_TYPE_RUIM;
        } else {
            cardTypes[index] = card_types[index];
        }
    }
    int newSlotId = getNewCdmaSocketSlotId();
    bool isChanged = RpModemController::getInstance()->isRemoteSimProtocolChanged(cardTypes[0],
            cardTypes[1], newSlotId);
    RFX_LOG_D(RP_CDMALTE_MODE_TAG,
            "[SMC]isRemoteSimProtocolChanged : %d, cardType0:%d, cardType1:%d,"
            " newCdmsSocketSlotId : %d",
            isChanged, cardTypes[0], cardTypes[1], newSlotId);
    delete[] cardTypes;
    return isChanged;
}

const char* RpCdmaLteModeController::printSwitchState(ModeSwitchState state) {
    switch (state) {
    case STATE_PREPARE_FOR_SWITCH:
        return "PREPARE_FOR_SWITCH";
    case STATE_TRIGGER_TRM:
        return "TRIGGER_TRM";
    case STATE_SWITCH_CARD_TYPE:
        return "SWITCH_CARD_TYPE";
    case STATE_START_MODE_SWITCH:
        return "START_MODE_SWITCH";
    case STATE_ENTER_RESTRICT_MODE:
        return "ENTER_RESTRICT_MODE";
    case STATE_RF_OFF:
        return "RF_OFF";
    case STATE_CONFIG_MODEM_STATUS:
        return "CONFIG_MODEM_STATUS";
    case STATE_SWITCH_RADIO_TECHNOLOGY:
        return "SWITCH_RADIO_TECHNOLOGY";
    case STATE_LEAVE_RESTRICT_MODE:
        return "LEAVE_RESTRICT_MODE";
    case STATE_RF_ON:
        return "RF_ON";
    case STATE_FINISH_MODE_SWITCH:
        return "FINISH_MODE_SWITCH";
    default:
        return "Unknown";
    }
}

void RpCdmaLteModeController::switchState(ModeSwitchState state) {
    RFX_LOG_D(RP_CDMALTE_MODE_TAG, "[SMC][switchState] %s", printSwitchState(state));
    switch (state) {
        case STATE_PREPARE_FOR_SWITCH: {
            calculateCardMode();

            if (sIsSvlteSupport && isRemoteSimProtocolChanged(mSwitchInfo->card_types,
                    RFX_SLOT_COUNT)) {
                switchState(STATE_TRIGGER_TRM);
            } else if (needSwtichCardType()) {
                switchState(STATE_SWITCH_CARD_TYPE);
            } else {
                switchState(STATE_START_MODE_SWITCH);
            }
            break;
        }
        case STATE_TRIGGER_TRM:
            triggerTrm();
            break;
        case STATE_SWITCH_CARD_TYPE:
            switchCardType();
            break;
        case STATE_START_MODE_SWITCH: {
            startSwitchMode();
            int oldCdmaSocketSlot = sCdmaSocketSlotId;
            setupCdmaLteMode();
            setupSwitchQueue(oldCdmaSocketSlot);

            if (!sIsSvlteSupport && isEmdStatusChanged(mSwitchInfo->card_types, RFX_SLOT_COUNT)
                // ALPS03702535, Dead lock after Sim switch add lock all c2k channels.
                // 1. Sim switch send ERMS=1 in the begining and will lock all c2k channels until
                //    later mode switch send EMDSTATUS done.
                // 2. Mode switch will send ERMS=1 again because isEmdStatusChanged return true.
                // 3. When mode switch send ERMS=1 to c2k rild, it is blcoked by Sim switch lock
                //    c2k channel. Cannot goto next step to send EMDSTATUS.
                // 4. Sim switch and mode switch dead lock.
                // Solution:
                // Mode switch skip send ERMS=1 in this case, send EMDSTATUS directly.
                    && (!isDssNoResetSupport() || (isDssNoResetSupport()
                    && getNonSlotScopeStatusManager()->getIntValue(
                    RFX_STATUS_KEY_RESTRICT_MODE_STATE,RESTRICT_MODE_STATE_OFF)
                    == RESTRICT_MODE_STATE_OFF))) {
                switchState(STATE_ENTER_RESTRICT_MODE);
            } else {
                switchState(STATE_CONFIG_MODEM_STATUS);
            }
            break;
        }
        case STATE_ENTER_RESTRICT_MODE:
            enterRestrictMode();
            break;
        case STATE_RF_OFF:
            requestRadioOff();
            break;
        case STATE_CONFIG_MODEM_STATUS:
            configModemStatus(mSwitchInfo->card_types, RFX_SLOT_COUNT);
            break;
        case STATE_SWITCH_RADIO_TECHNOLOGY:
            doSwitchRadioTech();
            break;
        case STATE_LEAVE_RESTRICT_MODE:
            leaveRestrictMode();
            break;
        case STATE_RF_ON:
            requestRadioOn();
            switchState(STATE_FINISH_MODE_SWITCH);
            break;
        case STATE_FINISH_MODE_SWITCH:
            finishSwitchMode();
            break;
        default:
            RFX_LOG_D(RP_CDMALTE_MODE_TAG, "State %d not handled", state);
            break;
    }
}

bool RpCdmaLteModeController::isCt3GDualMode(int slotId, int cardType) {
    bool hasCard = (cardType > CARD_TYPE_NONE);
    bool isCt3G = RFX_OBJ_GET_INSTANCE(RfxRootController)->getStatusManager(
            slotId)->getBoolValue(RFX_STATUS_KEY_CT3G_DUALMODE_CARD, false);
    return hasCard && isCt3G;
}

void RpCdmaLteModeController::switchModeForECC(const sp<RfxMessage>& message) {
    int modemOffState = getNonSlotScopeStatusManager()->getIntValue(RFX_STATUS_KEY_MODEM_OFF_STATE,
            MODEM_OFF_IN_IDLE);
    int worldModeState = getStatusManager(sCdmaSocketSlotId == 1 ? 0 : 1)->getIntValue(
                RFX_STATUS_KEY_GSM_WORLD_MODE_STATE, 1);
    RfxSocketState socketState =
            getStatusManager(sCdmaSocketSlotId)->getSocketStateValue(RFX_STATUS_KEY_SOCKET_STATE);
    bool isC2kSocketConnected = socketState.getSocketState(RfxSocketState::SOCKET_C2K);

    if (modemOffState != MODEM_OFF_IN_IDLE || worldModeState != 1 || !isC2kSocketConnected) {
        RFX_LOG_D(RP_CDMALTE_MODE_TAG,
               "[SMC][switchModeForECC] suspend as modemOffState = %d,worldModeState= %d,"
                " isC2kSocketConnected = %d, Just return",
                modemOffState, worldModeState, isC2kSocketConnected);
        responseToRilj(RfxMessage::obtainResponse(RIL_E_GENERIC_FAILURE, message));
        return;
    }

    int cardTypes[MAX_RFX_SLOT_COUNT] = { 0 };
    int cardState[MAX_RFX_SLOT_COUNT] = { 0 };
    for (int i = 0; i < RFX_SLOT_COUNT; i++) {
        mSwitchInfo->rat_mode[i] = RAT_MODE_INVALID;
        cardTypes[i] = RFX_OBJ_GET_INSTANCE(RfxRootController)->getStatusManager(i)
                ->getIntValue(RFX_STATUS_KEY_CARD_TYPE, CARD_TYPE_NONE);
        // ECC only trigger mode switch by none slot.
        if (containsCdma(cardTypes[i])
                || (i == message->getSlotId() && cardTypes[i] != CARD_TYPE_NONE)) {
            RFX_LOG_D(RP_CDMALTE_MODE_TAG,
                    "[SMC][switchModeForECC] cardTypes[%d] = %d, ECC Slot = %d"
                    " Just return",  i, cardTypes[i], message->getSlotId());
            responseToRilj(RfxMessage::obtainResponse(RIL_E_GENERIC_FAILURE, message));
            return;
        }
        cardState[i] = CARD_STATE_NOT_HOT_PLUG;
    }

    responseToRilj(RfxMessage::obtainResponse(RIL_E_SUCCESS, message));
    message->getParcel()->readInt32();  // pass length
    int32_t mode = message->getParcel()->readInt32();
    // CARD_TYPE_CSIM(04); CARD_TYPE_SIM(01)
    cardTypes[message->getSlotId()] = mode;
    mSwitchInfo->rat_mode[message->getSlotId()] =
            (mode == CARD_TYPE_CSIM ? RAT_MODE_CDMA_EVDO_AUTO : RAT_MODE_LTE_WCDMA_GSM);

    RFX_LOG_D(RP_CDMALTE_MODE_TAG, "[SMC][switchModeForECC] Slot%d mode = %d",
            message->getSlotId(), mode);
    for (int slotId = SIM_ID_1; slotId < RFX_SLOT_COUNT; slotId++) {
        mSwitchInfo->card_types[slotId] = cardTypes[slotId];
        mSwitchInfo->card_states[slotId] = cardState[slotId];
        mSwitchInfo->old_card_modes[slotId] = mSwitchInfo->card_modes[slotId];
        mSwitchInfo->isCt3GDualMode[slotId] = isCt3GDualMode(slotId, cardTypes[slotId]);
    }
    mSwitchInfo->isEccCase = true;
    switchState(STATE_PREPARE_FOR_SWITCH);
}

void RpCdmaLteModeController::onCallCountChanged(int slotId, RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    RFX_UNUSED(old_value);
    int callCount = value.asInt();
    // When current call finished, execute pended mode switch.
    if (callCount == 0) {
        cancelListeningCallStatus();
        RFX_LOG_D(RP_CDMALTE_MODE_TAG, "[SMC][onCallCountChanged] Slot %d call finished", slotId);
        dealPendedModeSwitch();
    }
}

void RpCdmaLteModeController::cancelListeningCallStatus() {
    mIsListeningCall = false;
    for (int index = 0; index < getSimCount(); index++) {
        getStatusManager(index)->unRegisterStatusChangedEx(RFX_STATUS_KEY_VOICE_CALL_COUNT,
                RfxStatusChangeCallbackEx(this, &RpCdmaLteModeController::onCallCountChanged));
    }
}

void RpCdmaLteModeController::handleSendEratFailByCall(int slotId) {
    RFX_LOG_D(RP_CDMALTE_MODE_TAG, "[SMC][handleSendEratFailByCall] Slot %d", slotId);

    // cleare switchQueue
    mSwitchInfo->switchQueue.clear();
    // rollback cardMode
    for (int index = 0; index < getSimCount(); index++) {
        mSwitchInfo->card_modes[index] = mSwitchInfo->old_card_modes[index];
    }
    // rollback Cdma Socket Slot.
    rollbackCardModeAndCdmaSlot(mSwitchInfo->mOldCdmaSocketSlotId);
    updatePendingRequestInfo(NULL, NULL, false);
    getNonSlotScopeStatusManager()->setIntValue(RFX_STATUS_KEY_MODEM_OFF_STATE,
            MODEM_OFF_IN_IDLE);
    // handle pending request which be blocked by calling.
    handlePendedByCall();
}

/**
* This for handling ERAT failed when calling. If RAT is change from G->C or C->G,
* should try to trigger switch mode again.
*/
bool RpCdmaLteModeController::checkEratReslut(int slotId) {
    // Just handle the following case: G->C OR C->G.
    if (mSwitchInfo->card_modes[slotId] == mSwitchInfo->old_card_modes[slotId]) {
        return true;
    }
    return getStatusManager(slotId)->getIntValue(RFX_STATUS_KEY_RAT_SWITCH_STATUS, 1);
}

void RpCdmaLteModeController::rollbackCardModeAndCdmaSlot(int oldCdmaSocketSlotId) {
    // MTK-START: rollback mCdmaLteModeSlotId
    int cdmaLteModeSlotId = CSFB_ON_SLOT;
    int simCount = getSimCount();
    for (int i = 0; i < simCount; i++) {
        if (mSwitchInfo->old_card_modes[i] == RADIO_TECH_MODE_CDMALTE) {
            cdmaLteModeSlotId = i;
            break;
        }
    }

    RFX_LOG_D(RP_CDMALTE_MODE_TAG, "[SMC]rollback ActiveCdmaLteModeSlotId : %d -> %d,"
            "CdmaSocketSlotId : %d -> %d",
            mCdmaLteModeSlotId, cdmaLteModeSlotId, sCdmaSocketSlotId, oldCdmaSocketSlotId);

    if (mCdmaLteModeSlotId != cdmaLteModeSlotId) {
        mCdmaLteModeSlotId = cdmaLteModeSlotId;
        String8 cardmodestr;
        cardmodestr.clear();
        for (int i = 0; i < RFX_SLOT_COUNT - 1; i++) {
            cardmodestr.appendFormat("%d,", mSwitchInfo->card_modes[i]);
        }
        cardmodestr.appendFormat("%d", mSwitchInfo->card_modes[simCount - 1]);
        property_set("persist.vendor.radio.svlte_slot", cardmodestr.string());
        RFX_LOG_D(RP_CDMALTE_MODE_TAG, "setupCdmaLteMode : %s", cardmodestr.string());
    }
    getNonSlotScopeStatusManager()->setIntValue(
            RFX_STATUS_KEY_ACTIVE_CDMALTE_MODE_SLOT, mCdmaLteModeSlotId);
    // MTK-END
    // MTK-START:rollback C slot
    setCdmaSocketSlotId(mCdmaLteModeSlotId > -1 ? mCdmaLteModeSlotId : oldCdmaSocketSlotId);

    getNonSlotScopeStatusManager()->setIntValue(RFX_STATUS_KEY_CDMA_SOCKET_SLOT, sCdmaSocketSlotId);
    RFX_OBJ_GET_INSTANCE(RfxSocketStateManager)->setCdmaSocketSlotChange(sCdmaSocketSlotId);

    RpCapabilityGetController *capabilityGetController =
                (RpCapabilityGetController *)findController(RFX_SLOT_ID_UNKNOWN,
                RFX_OBJ_CLASS_INFO(RpCapabilityGetController));

    capabilityGetController->updateRadioCapability(sCdmaSocketSlotId);
    // MTK-END
}

void RpCdmaLteModeController::updatePendingRequestInfo(int* card_types,
        bool* ct3gDualMode, bool useNew) {
    int simCount = getSimCount();
    if (useNew) {
        mPendingRecord->save(card_types, mPendingRecord->m_pending_card_states,
                ct3gDualMode, simCount);
    } else {
        mPendingRecord->save(mSwitchInfo->card_types, mSwitchInfo->card_states,
                mSwitchInfo->isCt3GDualMode, simCount);
    }
    // always update the card state to NO_HOT_PLUG
    for (int index = 0; index < simCount; index++) {
        mPendingRecord->m_pending_card_states[index] = CARD_STATE_NOT_HOT_PLUG;
    }
}

/**
* Check if there is a ongoing call. If call is ongoing ,shuold listen it's status. Otherwise deal pending request.
*/
void RpCdmaLteModeController::handlePendedByCall() {
    for (int index = 0; index < getSimCount(); index++) {
        bool isCalling = (getStatusManager(index)->getIntValue(
                RFX_STATUS_KEY_VOICE_CALL_COUNT, 0) > 0);
        if (isCalling > 0) {
            mIsListeningCall = true;
            getStatusManager(index)->registerStatusChangedEx(
                    RFX_STATUS_KEY_VOICE_CALL_COUNT,
                    RfxStatusChangeCallbackEx(this, &RpCdmaLteModeController::onCallCountChanged));
            break;
        }
    }
    if (!mIsListeningCall) {
        dealPendedModeSwitch();
    }
}

bool RpCdmaLteModeController::isCommontSlotSupport() {
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get(PROPERTY_COMMON_SLOT_SUPPORT, property_value, "0");
    return (atoi(property_value) == 1);
}