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
#include <stdlib.h>
#include <string.h>
#include "nw/RtcRatSwitchController.h"
#include "RfxLog.h"
#include "RtcModeSwitchController.h"
#include "RfxRootController.h"
#include "RfxVoidData.h"
#include "RfxIntsData.h"
#include "RfxMessageId.h"
#include "RfxMainThread.h"
#include "rfx_properties.h"
#include "capabilityswitch/RtcCapabilityGetController.h"
#include "RfxRilUtils.h"
#include <libmtkrilutils.h>

/*****************************************************************************
 * Class RtcModeSwitchController
 *****************************************************************************/

RFX_IMPLEMENT_CLASS("RtcModeSwitchController", RtcModeSwitchController, RfxController);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxVoidData, RFX_MSG_REQUEST_MODE_CONTROLLER_DEAL_PENDING_MODE_SWITCH);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_SWITCH_MODE_FOR_ECC);
RtcModeSwitchController::RtcModeSwitchController() : mOldCCapabilitySlot(-1),
        mCapabilityReportedCount(0) {
    mPendingRecord = new PendingSwitchRecord();
    mSwitchInfo = new ModeSwitchInfo;

    for (int slotId = RFX_SLOT_ID_0; slotId < RFX_SLOT_COUNT; slotId++) {
        mSwitchInfo->card_type[slotId] = CARD_TYPE_INVALID;
        mSwitchInfo->card_state[slotId] = CARD_STATE_INVALID;
        mSwitchInfo->isCt3GDualMode[slotId] = false;
        mSwitchInfo->rat_mode[slotId] = RAT_MODE_INVALID;
    }

    initCCapabilitySlotId();
    mCardTypeReadyController = RFX_OBJ_GET_INSTANCE(RtcCardTypeReadyController);
}

RtcModeSwitchController::~RtcModeSwitchController() {
    delete mSwitchInfo;
    delete mPendingRecord;
    RFX_OBJ_CLOSE_INSTANCE(RtcCardTypeReadyController);
}

void RtcModeSwitchController::onInit() {
    RfxController::onInit();
    RFX_LOG_D(RTC_MODE_CONTROLLER_TAG, "RtcModeSwitchController::onInit");
    const int request_id_list[] = {
        RFX_MSG_REQUEST_MODE_CONTROLLER_DEAL_PENDING_MODE_SWITCH,
        RFX_MSG_REQUEST_SWITCH_MODE_FOR_ECC
    };

    for (int slotId = RFX_SLOT_ID_0; slotId < RFX_SLOT_COUNT; slotId++) {
        registerToHandleRequest(slotId, request_id_list, sizeof(request_id_list)/sizeof(const int));
        RtcRatSwitchController* ratSwitchController = (RtcRatSwitchController *) findController(
                slotId, RFX_OBJ_CLASS_INFO(RtcRatSwitchController));
        ratSwitchController->mRatSwitchSignal.connect(this,
                &RtcModeSwitchController::onRatSwitchDone);

        int radioCapability = getStatusManager(slotId)->getIntValue(RFX_STATUS_KEY_SLOT_CAPABILITY, 0);
        if (radioCapability > 0) {
            mCapabilityReportedCount++;
            // When KEY_SLOT_CAPABILITY valid, if mCCapabilitySlot still has not been applied C on this key.
            // We consider it is not support C currently and set C slot to invaild value(-1) to distinguish 5/6M.
            if (mCCapabilitySlot == slotId && !hasCRadioCapability(radioCapability)) {
                setCCapabilitySlotId(-1);
            }
        } else {
            getStatusManager(slotId)->registerStatusChangedEx(RFX_STATUS_KEY_SLOT_CAPABILITY,
                RfxStatusChangeCallbackEx(this, &RtcModeSwitchController::onCapabilityReported));
        }
    }
}

void RtcModeSwitchController::onDeinit() {
    mSwitchInfo->switchQueue.clear();

    RfxController::onDeinit();
}

void RtcModeSwitchController::onCardTypeReady(int* card_type, int* card_state, int slotNum) {
    RFX_ASSERT(slotNum == RFX_SLOT_COUNT);
    bool is_ct3g_dualmode[MAX_RFX_SLOT_ID] = { false };
    int rat_mode[MAX_RFX_SLOT_ID] = { 0 };
    int real_card_state[MAX_RFX_SLOT_ID] = { 0 };
    int ct3gStatus[MAX_RFX_SLOT_ID] = { 0 };
    for (int i = 0; i < slotNum; i++) {
        real_card_state[i] = mPendingRecord->updateCardStateIfRecordCovered(card_state[i],i);
        is_ct3g_dualmode[i] = isCt3GDualMode(i, card_type[i]);
        ct3gStatus[i] = getStatusManager(i)->getIntValue(RFX_STATUS_KEY_CDMA3G_SWITCH_CARD, -1);
        rat_mode[i] = RAT_MODE_INVALID;
        RFX_LOG_D(RTC_MODE_CONTROLLER_TAG,
                "[SMC][onCardTypeReady] Slot:%d, card type:%d, card state:%d",
                i, card_type[i], real_card_state[i]);
    }
    mPendingRecord->cancel();
    enterModeSwitch(card_type, real_card_state, is_ct3g_dualmode, rat_mode, slotNum, ct3gStatus);
}

bool RtcModeSwitchController::isEnableSwitchMode(int* card_type, int* card_state,
        bool* is_ct3g_dualmode, int* rat_mode, int slotNum, int* ct3gStatus) {
    if (mCapabilityReportedCount != RFX_SLOT_COUNT) {
        RFX_LOG_D(RTC_MODE_CONTROLLER_TAG,
                "[SMC][isEnableSwitchMode] Return, mCapabilityReportedCount = %d",
                mCapabilityReportedCount);
        mPendingRecord->save(card_type, card_state, is_ct3g_dualmode, rat_mode, slotNum,
                ct3gStatus);
        return false;
    }

    int modemOffState = getNonSlotScopeStatusManager()->getIntValue(RFX_STATUS_KEY_MODEM_OFF_STATE,
            MODEM_OFF_IN_IDLE);
    switch (modemOffState) {
        case MODEM_OFF_BY_POWER_OFF:
        case MODEM_OFF_BY_RESET_RADIO:
            RFX_LOG_D(RTC_MODE_CONTROLLER_TAG,
                    "[SMC][isEnableSwitchMode] Return, modemOffState = %s",
                    printModemOffState(modemOffState));
            return false;
        case MODEM_OFF_BY_MODE_SWITCH:
            RFX_LOG_D(RTC_MODE_CONTROLLER_TAG,
                    "[SMC][isEnableSwitchMode] Pended, modemOffState = %s",
                    printModemOffState(modemOffState));
            mPendingRecord->save(card_type, card_state, is_ct3g_dualmode, rat_mode, slotNum,
                    ct3gStatus);
            return false;
        case MODEM_OFF_BY_SIM_SWITCH:
        case MODEM_OFF_BY_WORLD_PHONE:
            RFX_LOG_D(RTC_MODE_CONTROLLER_TAG,
                "[SMC][isEnableSwitchMode] Pended, modemOffState = %s",
                printModemOffState(modemOffState));
            mPendingRecord->save(card_type, card_state, is_ct3g_dualmode, rat_mode, slotNum,
                    ct3gStatus);
            getNonSlotScopeStatusManager()->registerStatusChangedEx(RFX_STATUS_KEY_MODEM_OFF_STATE,
                    RfxStatusChangeCallbackEx(this, &RtcModeSwitchController::onModemOffStateChanged));
            return false;
        default:
            return true;
    }
}

void RtcModeSwitchController::onModemOffStateChanged(int slotId, RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(slotId);
    RFX_UNUSED(key);
    RFX_UNUSED(old_value);
    int modemOffState = value.asInt();
    switch (modemOffState) {
        case MODEM_OFF_BY_MODE_SWITCH:
        case MODEM_OFF_BY_POWER_OFF:
        case MODEM_OFF_BY_RESET_RADIO:
            getNonSlotScopeStatusManager()->unRegisterStatusChangedEx(RFX_STATUS_KEY_MODEM_OFF_STATE,
                    RfxStatusChangeCallbackEx(this, &RtcModeSwitchController::onModemOffStateChanged));
            RFX_LOG_D(RTC_MODE_CONTROLLER_TAG,
                    "[SMC][onModemOffStateChanged]modemOffState = %s, cancel callback",
                    printModemOffState(modemOffState));
            break;
        case MODEM_OFF_IN_IDLE:
            getNonSlotScopeStatusManager()->unRegisterStatusChangedEx(RFX_STATUS_KEY_MODEM_OFF_STATE,
                    RfxStatusChangeCallbackEx(this, &RtcModeSwitchController::onModemOffStateChanged));
            dealPendedModeSwitch();
            break;
        default:
            break;
     }
}

void RtcModeSwitchController::onCapabilityReported(int slotId, RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    RFX_UNUSED(old_value);
    int radioCapability = value.asInt();
    if (radioCapability > 0) {
        mCapabilityReportedCount++;
        getStatusManager(slotId)->unRegisterStatusChangedEx(RFX_STATUS_KEY_SLOT_CAPABILITY,
                RfxStatusChangeCallbackEx(this, &RtcModeSwitchController::onCapabilityReported));
        if (mCCapabilitySlot == slotId && !hasCRadioCapability(radioCapability)) {
            setCCapabilitySlotId(-1);
        }
    }

    // All slot`s radio capability are ready here, execute mode switch if has pended record.
    if (mCapabilityReportedCount == RFX_SLOT_COUNT && mPendingRecord->isPendingState()) {
        RFX_LOG_D(RTC_MODE_CONTROLLER_TAG,
                "[SMC][onCapabilityReported] In this case, SIM task ready notification is "
                "before all slot radio capability report");
        dealPendedModeSwitch();
    }
}

void RtcModeSwitchController::onCallCountChanged(int slotId, RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    RFX_UNUSED(old_value);
    int callCount = value.asInt();
    // When current call finished, execute pended mode switch.
    if (callCount == 0) {
        getStatusManager(slotId)->unRegisterStatusChangedEx(RFX_STATUS_KEY_AP_VOICE_CALL_COUNT,
                RfxStatusChangeCallbackEx(this, &RtcModeSwitchController::onCallCountChanged));
        RFX_LOG_D(RTC_MODE_CONTROLLER_TAG, "[SMC][onCallCountChanged] Slot %d call finished", slotId);
        dealPendedModeSwitch();
    }
}

int RtcModeSwitchController::getCallingSlot() {
    int callingSlot = -1;
    for (int slotId = RFX_SLOT_ID_0; slotId < RFX_SLOT_COUNT; slotId++) {
        if (getStatusManager(slotId)->getIntValue(RFX_STATUS_KEY_AP_VOICE_CALL_COUNT, 0) > 0) {
            callingSlot = slotId;
            break;
        }
    }
    return callingSlot;
}

int RtcModeSwitchController::getCallingSlotWithCChangeCase() {
    int callingSlot = getCallingSlot();
    // Mode switch need unidirectional sync with call flow
    if (callingSlot >= RFX_SLOT_ID_0) {
        mOldCCapabilitySlot = mCCapabilitySlot;
        int newCCapabilitySlot = calculateCCapabilitySlot();
        // Can not do mode switch if it is C slot change case has call.
        if (newCCapabilitySlot != mOldCCapabilitySlot && hasCErat(getStatusManager(
                    mOldCCapabilitySlot)->getIntValue(RFX_STATUS_KEY_PREFERRED_NW_TYPE))) {
            RFX_LOG_D(RTC_MODE_CONTROLLER_TAG, "[SMC][getCallingSlotWithCChangeCase] Slot %d",
                    callingSlot);
            return callingSlot;
        }
    }
    return -1;
}

void RtcModeSwitchController::handlePendedByCall(int slotId) {
    if (mPendingRecord->isPendingState()) {
        RFX_LOG_D(RTC_MODE_CONTROLLER_TAG,
            "[SMC][handlePendedByCall] Already has pending record, switch it");
        dealPendedModeSwitch();
    } else {
        RFX_LOG_D(RTC_MODE_CONTROLLER_TAG,
                "[SMC][handlePendedByCall] Pended and listen slot%d call state", slotId);
        mPendingRecord->save(mSwitchInfo->card_type, mSwitchInfo->card_state,
                mSwitchInfo->isCt3GDualMode, mSwitchInfo->rat_mode, RFX_SLOT_COUNT,
                mSwitchInfo->ct3gStatus);
        getStatusManager(slotId)->registerStatusChangedEx(
                RFX_STATUS_KEY_AP_VOICE_CALL_COUNT,
                RfxStatusChangeCallbackEx(this, &RtcModeSwitchController::onCallCountChanged));
    }
}

void RtcModeSwitchController::enterModeSwitch(int* card_type, int* card_state, bool* is_ct3g_dualmode,
        int* rat_mode, int slotNum, int* ct3gStatus) {
    if (!isEnableSwitchMode(card_type, card_state, is_ct3g_dualmode, rat_mode, slotNum,
            ct3gStatus)) {
        return;
    }

    for (int slotId = RFX_SLOT_ID_0; slotId < slotNum; slotId++) {
        mSwitchInfo->card_type[slotId] = card_type[slotId];
        mSwitchInfo->card_state[slotId] = card_state[slotId];
        mSwitchInfo->isCt3GDualMode[slotId] = is_ct3g_dualmode[slotId];
        mSwitchInfo->rat_mode[slotId] = rat_mode[slotId];
        mSwitchInfo->ct3gStatus[slotId] = ct3gStatus[slotId];
    }

    int callingSlot = getCallingSlotWithCChangeCase();
    int iccidNotReadySlot = getIccidNotReadySlot();
    int emcsSlot = getEMCSSlot();
    RtcRatSwitchController* ratSwitchController = (RtcRatSwitchController *) findController(
            RFX_SLOT_ID_0, RFX_OBJ_CLASS_INFO(RtcRatSwitchController));
    if (callingSlot >= RFX_SLOT_ID_0) {
        handlePendedByCall(callingSlot);
        return;
    } else if (iccidNotReadySlot >= RFX_SLOT_ID_0) {
        handlePendedByIccid(iccidNotReadySlot);
        return;
    } else if (emcsSlot >= RFX_SLOT_ID_0 && ratSwitchController->isECBMRestrictedModeSupport()) {
        handlePendedByEMCS(emcsSlot);
        return;
    }

    switchState(STATE_START_MODE_SWITCH);
}

void RtcModeSwitchController::startSwitchMode() {
    /// Set Flag for exclusion with SimSwitch & IPO OFF.
    getNonSlotScopeStatusManager()->setIntValue(RFX_STATUS_KEY_MODEM_OFF_STATE,
            MODEM_OFF_BY_MODE_SWITCH);
    mOldCCapabilitySlot = mCCapabilitySlot;
    // No need calculate and update C slot if current project not support C.
    if (mCCapabilitySlot >= RFX_SLOT_ID_0) {
        int cCapabilitySlot = calculateCCapabilitySlot();
        setCCapabilitySlotId(cCapabilitySlot);
    }
    setupSwitchQueue();
}

void RtcModeSwitchController::finishSwitchMode() {
    if (mPendingRecord->isPendingState()) {
        RFX_LOG_D(RTC_MODE_CONTROLLER_TAG, "[SMC][finishSwitchMode]enqueue pending message to"
                " main queue.");
        enqueuePendingRequest();
    }
    getNonSlotScopeStatusManager()->setIntValue(RFX_STATUS_KEY_MODEM_OFF_STATE, MODEM_OFF_IN_IDLE);
    getNonSlotScopeStatusManager()->setIntValue(RFX_STATUS_KEY_MODESWITCH_FINISHED, 1, true);
}

void RtcModeSwitchController::setupSwitchQueue() {
    mSwitchInfo->switchQueue.clear();

    if (RFX_SLOT_COUNT == 1) {
        mSwitchInfo->switchQueue.add(RFX_SLOT_ID_0);
        return;
    }

    for (int i = RFX_SLOT_ID_0; i < RFX_SLOT_COUNT; i++) {
        if (i != mCCapabilitySlot) {
            mSwitchInfo->switchQueue.add(i);
        }
    }
    if (mCCapabilitySlot >= RFX_SLOT_ID_0) {
        mSwitchInfo->switchQueue.add(mCCapabilitySlot);
    }
}

void RtcModeSwitchController::doSwitchRadioTech() {
    int switchSlot = mSwitchInfo->switchQueue.itemAt(0);
    mSwitchInfo->switchQueue.removeAt(0);
    switchRadioTechnology(switchSlot);
}

void RtcModeSwitchController::switchRadioTechnology(int slotId) {
    bool switchSlotHasCall = (getStatusManager(slotId)->getIntValue(
            RFX_STATUS_KEY_AP_VOICE_CALL_COUNT, 0) > 0);
    RFX_LOG_D(RTC_MODE_CONTROLLER_TAG,
            "[SMC][switchRadioTechnology] Slot : %d, hasCall : %d", slotId, switchSlotHasCall);
    // If current slot has call, skip rat switch it.
    if (switchSlotHasCall) {
        handleSendEratFailByCall(slotId);
    } else {
        RatSwitchInfo switchInfo = {mSwitchInfo->card_type[slotId],
                mSwitchInfo->card_state[slotId],
                mSwitchInfo->isCt3GDualMode[slotId],
                mSwitchInfo->rat_mode[slotId], mSwitchInfo->ct3gStatus[slotId]};

        RtcRatSwitchController* ratSwitchController = (RtcRatSwitchController *)findController(
                slotId, RFX_OBJ_CLASS_INFO(RtcRatSwitchController));
        ratSwitchController->setPreferredNetworkType(switchInfo);
    }
}

void RtcModeSwitchController::onRatSwitchDone(int slotId, int error) {
    RFX_LOG_D(RTC_MODE_CONTROLLER_TAG, "[SMC][onRatSwitchDone] Slot : %d switch done, error : %d,"
            " switchQueue size : %zu", slotId, error, mSwitchInfo->switchQueue.size());
    // Flow goto here mean although we check has no call when start mode switch,
    // it still has call when old CSlot send ERAT.
    // If ERAT failed by call ,Error will be CME_OPERATION_NOT_ALLOWED_ERR
    if (error == RIL_E_OPERATION_NOT_ALLOWED) {
        handleSendEratFailByCall(slotId);
        return;
    }

    if (mSwitchInfo->switchQueue.isEmpty()) {
         switchState(STATE_FINISH_MODE_SWITCH);
    } else if (mSwitchInfo->switchQueue.size() >= 1) {
        if (mOldCCapabilitySlot != mCCapabilitySlot && slotId == mOldCCapabilitySlot
                && error != 0) {
            int curNwType = getStatusManager(slotId)->getIntValue(RFX_STATUS_KEY_PREFERRED_NW_TYPE);
            if (hasCErat(curNwType) &&
                    ((isInternalLoad() == 1) || (isUserLoad() != 1))) {
                // C capability slot change case, if old C capability slot still has C ERAT here, assert!
                RFX_LOG_E(RTC_MODE_CONTROLLER_TAG, "[SMC][onRatSwitchDone] C change case, Switch "
                        "old C capability slot %d fail, curNwType %d, assert!",
                        mCCapabilitySlot, curNwType);
                RFX_ASSERT(0);
            } else {
                RFX_LOG_E(RTC_MODE_CONTROLLER_TAG, "set ERAT fail in user load. reset radio");
                rfx_property_set("vendor.ril.mux.report.case", "2");
                rfx_property_set("vendor.ril.muxreport", "1");
            }
        }
        int switchSlot = mSwitchInfo->switchQueue.itemAt(0);
        RFX_LOG_D(RTC_MODE_CONTROLLER_TAG, "[SMC][onRatSwitchDone] Start switch Slot: %d",
                   switchSlot);
        mSwitchInfo->switchQueue.removeAt(0);
        switchRadioTechnology(switchSlot);
    }
}

void RtcModeSwitchController::handleSendEratFailByCall(int slotId) {
    RFX_LOG_D(RTC_MODE_CONTROLLER_TAG, "[SMC][handleSendEratFailByCall] Slot %d", slotId);
    bool switchSlotHasCall = (getStatusManager(slotId)->getIntValue(
            RFX_STATUS_KEY_AP_VOICE_CALL_COUNT, 0) > 0);
    if (hasCall()) {
        // Restore mode switch state to previous, and wait call finish notification to trigger
        // switch pended record.
        mSwitchInfo->switchQueue.clear();
        setCCapabilitySlotId(mOldCCapabilitySlot);
        updateRadioCapability();
        getNonSlotScopeStatusManager()->setIntValue(RFX_STATUS_KEY_MODEM_OFF_STATE,
                MODEM_OFF_IN_IDLE);
        handlePendedByCall(getCallingSlot());
    } else {
        switchRadioTechnology(slotId);
    }
}

bool RtcModeSwitchController::hasCall() {
    bool ret = false;
    for (int i = RFX_SLOT_ID_0; i < RfxRilUtils::rfxGetSimCount(); i++) {
        if (getStatusManager(i)->getIntValue(RFX_STATUS_KEY_AP_VOICE_CALL_COUNT, 0) >0) {
            ret = true;
            break;
        }
    }
    RFX_LOG_D(RTC_MODE_CONTROLLER_TAG, "[SMC][hasCall: %s", (ret) ? "true" : "false");
    return ret;
}

bool RtcModeSwitchController::hasCErat(int type) {
    bool hasCErat = false;
    switch (type) {
        case PREF_NET_TYPE_CDMA_EVDO_AUTO:
        case PREF_NET_TYPE_LTE_CDMA_EVDO:
        case PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA:
        case PREF_NET_TYPE_CDMA_ONLY:
        case PREF_NET_TYPE_EVDO_ONLY:
        case PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO:
        case PREF_NET_TYPE_CDMA_GSM:
        case PREF_NET_TYPE_CDMA_EVDO_GSM:
        case PREF_NET_TYPE_LTE_CDMA_EVDO_GSM:
            hasCErat = true;
            break;
        default:
            break;
        }
        return hasCErat;
}

bool RtcModeSwitchController::hasCRadioCapability(int radioCapability) {
    if ((radioCapability & RAF_IS95A) > 0 ||
        (radioCapability & RAF_IS95B) > 0 ||
        (radioCapability & RAF_1xRTT) > 0 ||
        (radioCapability & RAF_EVDO_0) > 0 ||
        (radioCapability & RAF_EVDO_A) > 0 ||
        (radioCapability & RAF_EHRPD) > 0) {
        return true;
    }
    return false;
}

bool RtcModeSwitchController::onHandleRequest(const sp<RfxMessage>& message) {
    int msg_id = message->getId();

    switch (msg_id) {
        case RFX_MSG_REQUEST_MODE_CONTROLLER_DEAL_PENDING_MODE_SWITCH:
            RFX_LOG_D(RTC_MODE_CONTROLLER_TAG,
                    "[SMC]onHandleRequest, REQUEST_DEAL_PENDING_MODE_SWITCH, msgToken:%d",
                    message->getToken());
            dealPendedModeSwitch();
            break;
        case RFX_MSG_REQUEST_SWITCH_MODE_FOR_ECC:
            switchModeForECC(message);
            break;
        default:
            break;
    }
    return true;
}

void RtcModeSwitchController::switchModeForECC(const sp<RfxMessage>& message) {
    int modemOffState = getNonSlotScopeStatusManager()->getIntValue(RFX_STATUS_KEY_MODEM_OFF_STATE,
            MODEM_OFF_IN_IDLE);
    if (modemOffState != MODEM_OFF_IN_IDLE || mCapabilityReportedCount != RFX_SLOT_COUNT
        || mPendingRecord->isPendingState()) {
        RFX_LOG_D(RTC_MODE_CONTROLLER_TAG, "[SMC][switchModeForECC] modemOffState = %s,"
                " mCapabilityReportedCount = %d, hasPendingRecord = %d, Just return",
                printModemOffState(modemOffState), mCapabilityReportedCount,
                mPendingRecord->isPendingState());
        responseToRilj(RfxMessage::obtainResponse(RIL_E_GENERIC_FAILURE, message));
        return;
    }

    int cardType[MAX_RFX_SLOT_ID] = { 0 };
    int cardState[MAX_RFX_SLOT_ID] = { 0 };
    bool isCt3gDualMode[MAX_RFX_SLOT_ID] = { false };
    int ratMode[MAX_RFX_SLOT_ID] = { 0 };
    bool hasCCard = false;
    int ct3gStatus[MAX_RFX_SLOT_ID] = {-1};
    for (int i = RFX_SLOT_ID_0; i < RFX_SLOT_COUNT; i++) {
        cardState[i] = CARD_STATE_NOT_HOT_PLUG;
        ratMode[i] = RAT_MODE_INVALID;
        cardType[i] = RFX_OBJ_GET_INSTANCE(RfxRootController)->getStatusManager(i)
                ->getIntValue(RFX_STATUS_KEY_CARD_TYPE, CARD_TYPE_NONE);
        isCt3gDualMode[i] = isCt3GDualMode(i, cardType[i]);
        if (!hasCCard && (containsCdma(cardType[i]) || isCt3gDualMode[i])) {
            hasCCard = true;
        }
    }

    // ECC trigger mode switch flow must satisfies below conditions:
    // 1. All slots have no C card. 2. Current switch slot must be none slot
    if (hasCCard || cardType[message->getSlotId()] != CARD_TYPE_NONE) {
        RFX_LOG_D(RTC_MODE_CONTROLLER_TAG,
                "[SMC][switchModeForECC] hasCCard = %d, ECC Slot%d cardType = %d"
                " Just return",  hasCCard, message->getSlotId(), cardType[message->getSlotId()]);
        responseToRilj(RfxMessage::obtainResponse(RIL_E_GENERIC_FAILURE, message));
        return;
    }

    int* pInt = (int*)message->getData()->getData();
    // Data from RILJ is 4(C) or 1(G) , but ratMode is CDMA_EVDO_AUTO(C) or LTE_WCDMA_GSM(G),
    // Convert it here for forward compatible.
    switch (pInt[0]) {
        case 1:
            ratMode[message->getSlotId()] = LTE_WCDMA_GSM;
            break;
        case 4:
            ratMode[message->getSlotId()] = CDMA_EVDO_AUTO;
            break;
        default:
            RFX_LOG_D(RTC_MODE_CONTROLLER_TAG, "[SMC][switchModeForECC] pInt[0] = %d, "
                "message data invalid, Just return", pInt[0]);
            responseToRilj(RfxMessage::obtainResponse(RIL_E_GENERIC_FAILURE, message));
            return;
    }

    responseToRilj(RfxMessage::obtainResponse(RIL_E_SUCCESS, message));

    RFX_LOG_D(RTC_MODE_CONTROLLER_TAG, "[SMC][switchModeForECC] Slot%d mode = %d",
            message->getSlotId(), ratMode[message->getSlotId()]);
    enterModeSwitch(cardType, cardState, isCt3gDualMode, ratMode, RFX_SLOT_COUNT, ct3gStatus);
}

void RtcModeSwitchController::enqueuePendingRequest() {
    sp<RfxMessage> pendingRequest = RfxMessage::obtainRequest(0,
        RFX_MSG_REQUEST_MODE_CONTROLLER_DEAL_PENDING_MODE_SWITCH, RfxVoidData());
    RfxMainThread::enqueueMessage(pendingRequest);
}

void RtcModeSwitchController::dealPendedModeSwitch() {
    int modemOffState = getNonSlotScopeStatusManager()->getIntValue(
            RFX_STATUS_KEY_MODEM_OFF_STATE, MODEM_OFF_IN_IDLE);
    if (mPendingRecord->isPendingState() && modemOffState != MODEM_OFF_BY_MODE_SWITCH) {
        RFX_LOG_D(RTC_MODE_CONTROLLER_TAG, "[SMC]Start switch mode pended");
        applyPendingRecord();
        enterModeSwitch(mSwitchInfo->card_type, mSwitchInfo->card_state,
                mSwitchInfo->isCt3GDualMode, mSwitchInfo->rat_mode, RFX_SLOT_COUNT,
                mSwitchInfo->ct3gStatus);
     }
}
bool RtcModeSwitchController::onHandleResponse(const sp<RfxMessage>& message) {
    int msg_id = message->getId();
    RFX_LOG_D(RTC_MODE_CONTROLLER_TAG,
            "[SMC]onHandleResponse, msgId: %d", msg_id);
    switch (msg_id) {
        default:
            break;
    }
    return true;
}

/**
 * Copy data of mPendingRecord to mSwitchInfo.
 */
void RtcModeSwitchController::applyPendingRecord() {
    if (mPendingRecord->isPendingState() == false) {
        return;
    }
    mPendingRecord->m_hasPendingRecord = false;
    for (int slotId = RFX_SLOT_ID_0; slotId < RFX_SLOT_COUNT; slotId++) {
        mSwitchInfo->card_type[slotId] = mPendingRecord->m_pending_card_type[slotId];
        mSwitchInfo->card_state[slotId] = mPendingRecord->m_pending_card_state[slotId];
        mSwitchInfo->isCt3GDualMode[slotId] = mPendingRecord->m_pending_isCt3gDualMode[slotId];
        mSwitchInfo->rat_mode[slotId] = mPendingRecord->m_pending_rat_mode[slotId];
    }
}

void RtcModeSwitchController::updateRadioCapability() {
    RtcCapabilityGetController* capabilityGetController =
            (RtcCapabilityGetController *)findController(RFX_OBJ_CLASS_INFO(RtcCapabilityGetController));
    capabilityGetController->updateRadioCapability(mCCapabilitySlot);
}

int RtcModeSwitchController::calculateCCapabilitySlot() {
    int cCapabilitySlot = 0;
    int realCCardCount = 0;
    int firstCCard = -1;
    int cSlotSwitchByECC = -1;

    for (int i = RFX_SLOT_ID_0; i < RFX_SLOT_COUNT; i++) {
        if (containsCdma(mSwitchInfo->card_type[i]) || mSwitchInfo->isCt3GDualMode[i]) {
            if (firstCCard == -1) {
                firstCCard = i;
            }
            realCCardCount++;
        }
        if (mSwitchInfo->rat_mode[i] == CDMA_EVDO_AUTO) {
            cSlotSwitchByECC = i;
        }
    }
    switch (realCCardCount) {
        case 0:
            cCapabilitySlot = (cSlotSwitchByECC == -1 ? mOldCCapabilitySlot : cSlotSwitchByECC);
            break;
        case 1:
            cCapabilitySlot = firstCCard;
            break;
        default: {
            char property_value[MTK_PROPERTY_VALUE_MAX] = { 0 };
            rfx_property_get("persist.vendor.radio.simswitch", property_value, "1");
            int capabilitySlot = atoi(property_value) - 1;
            if (capabilitySlot >= 0 && ( containsCdma(mSwitchInfo->card_type[capabilitySlot])
                || mSwitchInfo->isCt3GDualMode[capabilitySlot])) {
                cCapabilitySlot = capabilitySlot;
            } else {
                cCapabilitySlot = firstCCard;
            }
            break;
        }
    }
    RFX_LOG_D(RTC_MODE_CONTROLLER_TAG, "[SMC][calculateCCapabilitySlot] %d", cCapabilitySlot);
    return cCapabilitySlot;
}

bool RtcModeSwitchController::containsCdma(int cardType) {
     if ((cardType & CARD_TYPE_RUIM) > 0 ||
         (cardType & CARD_TYPE_CSIM) > 0) {
         return true;
     }
     return false;
}

bool RtcModeSwitchController::isCt3GDualMode(int slotId, int cardType) {
    bool hasCard = (cardType > CARD_TYPE_NONE);
    bool isCt3G = RFX_OBJ_GET_INSTANCE(RfxRootController)->getStatusManager(
            slotId)->getBoolValue(RFX_STATUS_KEY_CDMA3G_DUALMODE_CARD, false);
    return hasCard && isCt3G;
}

void RtcModeSwitchController::setCCapabilitySlotId(int slotId) {
    RFX_LOG_D(RTC_MODE_CONTROLLER_TAG, "[SMC]setCCapabilitySlotId : %d -> %d",
            mCCapabilitySlot, slotId);
    mCCapabilitySlot = slotId;
    char* slot_str = NULL;
    asprintf(&slot_str, "%d", slotId + 1);
    rfx_property_set(PROPERTY_C_CAPABILITY_SLOT, slot_str);
    free(slot_str);
}

void RtcModeSwitchController::initCCapabilitySlotId() {
    char tempstr[MTK_PROPERTY_VALUE_MAX] = { 0 };

    memset(tempstr, 0, sizeof(tempstr));
    rfx_property_get(PROPERTY_C_CAPABILITY_SLOT, tempstr, "1");

    mCCapabilitySlot = atoi(tempstr) - 1;
    if (mCCapabilitySlot < RFX_SLOT_ID_0 || mCCapabilitySlot >= RFX_SLOT_COUNT) {
        setCCapabilitySlotId(RFX_SLOT_ID_0);
    }
}

int RtcModeSwitchController::getCCapabilitySlotId() {
    return mCCapabilitySlot;
}

const char* RtcModeSwitchController::printSwitchState(ModeSwitchState state) {
    switch (state) {
        case STATE_START_MODE_SWITCH:
            return "START_MODE_SWITCH";
        case STATE_UPDATE_RADIO_CAPABILITY:
            return "UPDATE_RADIO_CAPABILITY";
        case STATE_SWITCH_RADIO_TECHNOLOGY:
            return "SWITCH_RADIO_TECHNOLOGY";
        case STATE_FINISH_MODE_SWITCH:
            return "FINISH_MODE_SWITCH";
        default:
            return "Unknown";
    }
}

void RtcModeSwitchController::switchState(ModeSwitchState state) {
    RFX_LOG_D(RTC_MODE_CONTROLLER_TAG, "[SMC][switchState] %s", printSwitchState(state));
    switch (state) {
        case STATE_START_MODE_SWITCH:
            startSwitchMode();
            switchState(STATE_UPDATE_RADIO_CAPABILITY);
            break;
        case STATE_UPDATE_RADIO_CAPABILITY:
            updateRadioCapability();
            switchState(STATE_SWITCH_RADIO_TECHNOLOGY);
            break;
        case STATE_SWITCH_RADIO_TECHNOLOGY:
            doSwitchRadioTech();
            break;
        case STATE_FINISH_MODE_SWITCH:
            finishSwitchMode();
            break;
        default:
            RFX_LOG_D(RTC_MODE_CONTROLLER_TAG, "State %d not handled", state);
            break;
    }
}

const char* RtcModeSwitchController::printModemOffState(int state) {
    switch (state) {
        case MODEM_OFF_IN_IDLE:
            return "MODEM_OFF_IN_IDLE";
        case MODEM_OFF_BY_MODE_SWITCH:
            return "MODEM_OFF_BY_MODE_SWITCH";
        case MODEM_OFF_BY_POWER_OFF:
            return "MODEM_OFF_BY_POWER_OFF";
        case MODEM_OFF_BY_SIM_SWITCH:
            return "MODEM_OFF_BY_SIM_SWITCH";
        case MODEM_OFF_BY_RESET_RADIO:
            return "MODEM_OFF_BY_RESET_RADIO";
        case MODEM_OFF_BY_WORLD_PHONE:
            return "MODEM_OFF_BY_WORLD_PHONE";
        default:
            return "Unknown";
    }
}

RtcModeSwitchController::PendingSwitchRecord::PendingSwitchRecord() : m_hasPendingRecord(false) {
    for (int slotId = RFX_SLOT_ID_0; slotId < RFX_SLOT_COUNT; slotId++) {
        m_pending_card_type[slotId] = CARD_TYPE_INVALID;
        m_pending_card_state[slotId] = CARD_STATE_INVALID;
        m_pending_isCt3gDualMode[slotId] = false;
        m_pending_isCt3gDualMode[slotId] = RAT_MODE_INVALID;
    }
}

void RtcModeSwitchController::PendingSwitchRecord::cancel() {
    m_hasPendingRecord = false;
}

void RtcModeSwitchController::PendingSwitchRecord::save(int* card_type, int* card_state,
        bool* isCt3GDualMode, int* rat_mode, int slotNum, int* ct3gStatus) {
    RFX_ASSERT(slotNum == RFX_SLOT_COUNT);
    for (int slotId = RFX_SLOT_ID_0; slotId < RFX_SLOT_COUNT; slotId++) {
        m_pending_card_type[slotId] = card_type[slotId];
        m_pending_card_state[slotId] = updateCardStateIfRecordCovered(card_state[slotId], slotId);
        m_pending_isCt3gDualMode[slotId] = isCt3GDualMode[slotId];
        m_pending_rat_mode[slotId] = rat_mode[slotId];
        m_pending_ct3gStatus[slotId] = ct3gStatus[slotId];
    }
    m_hasPendingRecord = true;
}

bool RtcModeSwitchController::PendingSwitchRecord::isPendingState() {
    return m_hasPendingRecord;
}


// Update card state if has valid pending record will be covered or ignored.
int RtcModeSwitchController::PendingSwitchRecord::updateCardStateIfRecordCovered(int card_state, int slotId) {
    if (m_hasPendingRecord == true && ((m_pending_card_state[slotId] != CARD_STATE_NO_CHANGED
            && card_state == CARD_STATE_NO_CHANGED) ||
            m_pending_card_state[slotId] == CARD_STATE_NOT_HOT_PLUG)) {
        RFX_LOG_D(RTC_MODE_CONTROLLER_TAG,
                "[SMC][updateCardStateIfRecordCovered] cardState[%d]: %d -> %d",
                slotId, card_state, m_pending_card_state[slotId]);
        return m_pending_card_state[slotId];
    } else {
        return card_state;
    }
}

bool RtcModeSwitchController::onCheckIfRejectMessage(const sp<RfxMessage>& message,
        bool isModemPowerOff, int radioState) {
    RFX_UNUSED(isModemPowerOff);
    int id = message->getPId();

    if (RFX_MSG_REQUEST_SWITCH_MODE_FOR_ECC == id) {
        if (RADIO_STATE_ON == radioState || RADIO_STATE_OFF == radioState) {
            return false;
        }
        return true;
    }
    return false;
}

void RtcModeSwitchController::handlePendedByIccid(int slotId) {
    RFX_LOG_D(RTC_MODE_CONTROLLER_TAG,
            "[SMC][handlePendedByIccid] Pended and listen slot%d iccid changed", slotId);
    mPendingRecord->save(mSwitchInfo->card_type, mSwitchInfo->card_state,
            mSwitchInfo->isCt3GDualMode, mSwitchInfo->rat_mode, RFX_SLOT_COUNT,
            mSwitchInfo->ct3gStatus);
    getStatusManager(slotId)->registerStatusChangedEx(
            RFX_STATUS_KEY_SIM_ICCID,
            RfxStatusChangeCallbackEx(this, &RtcModeSwitchController::onIccidChanged));
}

void RtcModeSwitchController::onIccidChanged(int slotId, RfxStatusKeyEnum key,
        RfxVariant oldValue, RfxVariant value) {
    RFX_UNUSED(key);
    RFX_UNUSED(oldValue);
    String8 iccid = value.asString8();
    if (iccid.string() != NULL) {
        getStatusManager(slotId)->unRegisterStatusChangedEx(RFX_STATUS_KEY_SIM_ICCID,
                RfxStatusChangeCallbackEx(this, &RtcModeSwitchController::onIccidChanged));
        RFX_LOG_D(RTC_MODE_CONTROLLER_TAG, "[SMC][onIccidChanged] Slot %d iccid ready", slotId);
        dealPendedModeSwitch();
    }
}

int RtcModeSwitchController::getIccidNotReadySlot() {
    const char *initStr = "";
    String8 empty = String8(initStr);
    String8 iccid;
    int simCount = getSimCount();
    for (int i = 0; i < simCount; i++) {
        String8 iccid = getStatusManager(i)->getString8Value(RFX_STATUS_KEY_SIM_ICCID, empty);
        if (iccid.string() == NULL) {
            return i;
        }
    }
    return -1;
}

int RtcModeSwitchController::getEMCSSlot() {
    bool isEMCS = false;
    for (int i = 0; i < getSimCount(); i++) {
        isEMCS = getStatusManager(i)->getBoolValue(RFX_STATUS_KEY_EMERGENCY_MODE, false);
        if (isEMCS) {
            return i;
        }
    }
    return -1;
}

void RtcModeSwitchController::handlePendedByEMCS(int slotId) {
    RFX_LOG_D(RTC_MODE_CONTROLLER_TAG,
            "[SMC][handlePendedByEMCS] Pended and listen slot%d EMCS changed", slotId);
    mPendingRecord->save(mSwitchInfo->card_type, mSwitchInfo->card_state,
            mSwitchInfo->isCt3GDualMode, mSwitchInfo->rat_mode, RFX_SLOT_COUNT,
            mSwitchInfo->ct3gStatus);
    getStatusManager(slotId)->registerStatusChangedEx(
            RFX_STATUS_KEY_EMERGENCY_MODE,
            RfxStatusChangeCallbackEx(this, &RtcModeSwitchController::onEMCSChanged));
}

void RtcModeSwitchController::onEMCSChanged(int slotId, RfxStatusKeyEnum key,
        RfxVariant oldValue, RfxVariant value) {
    RFX_UNUSED(key);
    RFX_UNUSED(oldValue);
    bool isEMCS = value.asBool();
    if (!isEMCS) {
        getStatusManager(slotId)->unRegisterStatusChangedEx(RFX_STATUS_KEY_EMERGENCY_MODE,
                RfxStatusChangeCallbackEx(this, &RtcModeSwitchController::onEMCSChanged));
        RFX_LOG_D(RTC_MODE_CONTROLLER_TAG, "[SMC][onEMCSChanged] Slot %d EMCS end", slotId);
        dealPendedModeSwitch();
    }
}