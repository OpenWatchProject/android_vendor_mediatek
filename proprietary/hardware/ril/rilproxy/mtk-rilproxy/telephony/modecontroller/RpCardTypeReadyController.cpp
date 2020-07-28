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
#include "RpCardTypeReadyController.h"
#include "RpCdmaLteModeController.h"
#include "util/RpFeatureOptionUtils.h"

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/
RFX_IMPLEMENT_CLASS("RpCardTypeReadyController", RpCardTypeReadyController, RfxController);
RFX_OBJ_IMPLEMENT_SINGLETON_CLASS(RpCardTypeReadyController);

RpCardTypeReadyController::RpCardTypeReadyController() {
    m_card_types = new int[MAX_RFX_SLOT_COUNT];
    m_card_type_state = new int[MAX_RFX_SLOT_COUNT];
    for (int slotId = SIM_ID_1; slotId < RFX_SLOT_COUNT; slotId++) {
        m_card_types[slotId] = CARD_TYPE_INVALID;
        m_card_type_state[slotId] = CARD_STATE_INVALID;
    }
}

RpCardTypeReadyController::~RpCardTypeReadyController() {
    if (m_card_types != NULL) {
        delete[] m_card_types;
        m_card_types = NULL;
    }
    if (m_card_type_state != NULL) {
        delete[] m_card_type_state;
        m_card_type_state = NULL;
    }
}

void RpCardTypeReadyController::onInit() {
    RfxController::onInit();

    for (int i = 0; i< RFX_SLOT_COUNT; i++) {
        getStatusManager(i)->registerStatusChangedEx(RFX_STATUS_KEY_MODEM_SIM_TASK_READY,
                RfxStatusChangeCallbackEx(this, &RpCardTypeReadyController::onSimTaskReady));
    }
    getNonSlotScopeStatusManager()->registerStatusChanged(RFX_STATUS_KEY_CAPABILITY_SWITCH_STATE,
            RfxStatusChangeCallback(this, &RpCardTypeReadyController::onSimSwitchChanged));
}

void RpCardTypeReadyController::onDeinit() {
    for (int i = 0; i< RFX_SLOT_COUNT; i++) {
        getStatusManager(i)->unRegisterStatusChangedEx(RFX_STATUS_KEY_MODEM_SIM_TASK_READY,
                RfxStatusChangeCallbackEx(this, &RpCardTypeReadyController::onSimTaskReady));
    }
    getNonSlotScopeStatusManager()->unRegisterStatusChanged(RFX_STATUS_KEY_CAPABILITY_SWITCH_STATE,
            RfxStatusChangeCallback(this, &RpCardTypeReadyController::onSimSwitchChanged));

    RfxController::onDeinit();
}

void RpCardTypeReadyController::onSimTaskReady(int slotId, RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    int oldCardType = m_card_types[slotId];
    if (value.asBool() == true) {
        m_card_types[slotId] = getStatusManager(slotId)->getIntValue(RFX_STATUS_KEY_CARD_TYPE, CARD_TYPE_NONE);
    } else if (value.asBool() == false) {
        m_card_types[slotId] = CARD_TYPE_INVALID;
    }
    getNonSlotScopeStatusManager()->setIntValue(RFX_STATUS_KEY_CDMALTE_MODE_SLOT_READY,
            CDMALTE_MODE_NOT_READY);
    /// M: the value has been changed mean: this sim ready action is not triggered by hot plug action.
    if (value.asBool() != old_value.asBool()) {
        m_card_type_state[slotId] = CARD_STATE_NOT_HOT_PLUG;
    } else {
        int newCardType = m_card_types[slotId];
        /// M:There was a sim card
        if (oldCardType > CARD_TYPE_NONE) {
            /// M: and there is a sim card, now.
            if (newCardType > CARD_TYPE_NONE) {
                if (oldCardType != newCardType) {
                    m_card_type_state[slotId] = CARD_STATE_CARD_TYPE_CHANGED;
                } else {
                    m_card_type_state[slotId] = CARD_STATE_NO_CHANGED;
                }
            /// M: but there is no sim card, that mean: card has been plugout.
            } else if (newCardType == CARD_TYPE_NONE) {
                m_card_type_state[slotId] = CARD_STATE_HOT_PLUGOUT;
            } else {
                m_card_type_state[slotId] = CARD_STATE_NOT_HOT_PLUG;
            }
        /// M: There was no sim card.
        } else if (oldCardType == CARD_TYPE_NONE) {
            /// M:But there is a sim card, now. mean: card has been insertted.
            if (newCardType > CARD_TYPE_NONE) {
                m_card_type_state[slotId] = CARD_STATE_HOT_PLUGIN;
            /// M: and there is no sim card.
            } else {
                m_card_type_state[slotId] = CARD_STATE_NO_CHANGED;
            }
        } else {
            m_card_type_state[slotId] = CARD_STATE_NOT_HOT_PLUG;
        }
    }
    /// M: If this action is not sim task reset or bootup case, then should set
    /// other slots card state to CARD_STATE_NO_CHANGED
    if (m_card_type_state[slotId] != CARD_STATE_NOT_HOT_PLUG) {
        for (int index = 0; index < RFX_SLOT_COUNT; index++) {
            if (index != slotId) {
                m_card_type_state[index] = CARD_STATE_NO_CHANGED;
            }
        }
    }
    RFX_LOG_D(RP_CDMALTE_MODE_TAG, "[onSimTaskReady] Slot %d, state : %d -> %d, cardType : %d",
            slotId, old_value.asBool(), value.asBool(), m_card_types[slotId]);

    for (int i = 0; i < RFX_SLOT_COUNT; i++) {
        if (m_card_types[i] == CARD_TYPE_INVALID) {
            return;
        }
    }
    RpCdmaLteModeController* cdmaLteModeController =
            (RpCdmaLteModeController *)findController(RFX_OBJ_CLASS_INFO(RpCdmaLteModeController));
    CardTypeReadyReason ctrr = CARD_TYPE_READY_REASON_COMMON;
    if (!old_value.asBool() && value.asBool()) {
        ctrr = CARD_TYPE_READY_REASON_BOOTUP_OR_RIL_RESET;
    }
    cdmaLteModeController->onCardTypeReady(m_card_types, m_card_type_state, RFX_SLOT_COUNT, ctrr);
    for (int index = 0; index < RpFeatureOptionUtils::getSimCount(); index++) {
        m_card_type_state[index] = CARD_STATE_NO_CHANGED;
    }
}

void RpCardTypeReadyController::onSimSwitchChanged(RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    int oldState = old_value.asInt();
    int newState = value.asInt();

    RFX_LOG_D(RP_CDMALTE_MODE_TAG, "onSimSwitchChanged (%d, %d), reset card type",
            oldState, newState);

    if (CAPABILITY_SWITCH_STATE_START == newState) {
        // Reset card type.
        for(int i = 0; i < RFX_SLOT_COUNT; i++) {
            m_card_types[i] = CARD_TYPE_INVALID;
        }
    }
}
