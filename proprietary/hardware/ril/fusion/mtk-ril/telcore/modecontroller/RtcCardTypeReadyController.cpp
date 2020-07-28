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
#include "RtcCardTypeReadyController.h"
#include "RtcModeSwitchController.h"
#include "rfx_properties.h"
#include <libmtkrilutils.h>

#define RFX_COMMON_SLOT_MERGE_NOTIFY_TIMEOUT_MSEC (1000)

/*****************************************************************************
 * Class RtcCardTypeReadyController
 *****************************************************************************/
RFX_IMPLEMENT_CLASS("RtcCardTypeReadyController", RtcCardTypeReadyController, RfxController);
RFX_OBJ_IMPLEMENT_SINGLETON_CLASS(RtcCardTypeReadyController);

RtcCardTypeReadyController::RtcCardTypeReadyController() : mTimeoutHandle(NULL),
    m_timeout_msec(RFX_COMMON_SLOT_MERGE_NOTIFY_TIMEOUT_MSEC) {
    m_card_types = new int[MAX_RFX_SLOT_ID];
    m_card_type_states = new int[MAX_RFX_SLOT_ID];
    for (int slotId = RFX_SLOT_ID_0; slotId < RFX_SLOT_COUNT; slotId++) {
        m_card_types[slotId] = CARD_TYPE_INVALID;
        m_card_type_states[slotId] = CARD_STATE_INVALID;
    }
}

RtcCardTypeReadyController::~RtcCardTypeReadyController() {
    if (m_card_types != NULL) {
        delete[] m_card_types;
        m_card_types = NULL;
    }
    if (m_card_type_states != NULL) {
        delete[] m_card_type_states;
        m_card_type_states = NULL;
    }

    if (mTimeoutHandle != NULL) {
        RfxTimer::stop(mTimeoutHandle);
        mTimeoutHandle = NULL;
    }
}

void RtcCardTypeReadyController::onInit() {
    RfxController::onInit();

    for (int i = RFX_SLOT_ID_0; i< RFX_SLOT_COUNT; i++) {
        getStatusManager(i)->registerStatusChangedEx(RFX_STATUS_KEY_MODEM_SIM_TASK_READY,
                RfxStatusChangeCallbackEx(this, &RtcCardTypeReadyController::onSimTaskReady));
    }
    getNonSlotScopeStatusManager()->registerStatusChanged(RFX_STATUS_KEY_CAPABILITY_SWITCH_STATE,
            RfxStatusChangeCallback(this, &RtcCardTypeReadyController::onSimSwitchChanged));
}

void RtcCardTypeReadyController::onDeinit() {
    for (int i = RFX_SLOT_ID_0; i< RFX_SLOT_COUNT; i++) {
        getStatusManager(i)->unRegisterStatusChangedEx(RFX_STATUS_KEY_MODEM_SIM_TASK_READY,
                RfxStatusChangeCallbackEx(this, &RtcCardTypeReadyController::onSimTaskReady));
    }
    getNonSlotScopeStatusManager()->unRegisterStatusChanged(RFX_STATUS_KEY_CAPABILITY_SWITCH_STATE,
            RfxStatusChangeCallback(this, &RtcCardTypeReadyController::onSimSwitchChanged));

    RfxController::onDeinit();
}

void RtcCardTypeReadyController::onSimTaskReady(int slotId, RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    int oldCardType = m_card_types[slotId];
    m_card_type_states[slotId] = CARD_STATE_NOT_HOT_PLUG;

    if (value.asBool() == true) {
        m_card_types[slotId] = getStatusManager(slotId)->getIntValue(RFX_STATUS_KEY_CARD_TYPE, CARD_TYPE_NONE);
        /// M: the value has been changed mean: this sim ready action is not triggered by hot plug action.
        if (old_value.asBool() == true && !isAnyCardInvalid()) {
            int newCardType = m_card_types[slotId];
            /// M:There was a sim card
            if (oldCardType > CARD_TYPE_NONE) {
                /// M: and there is a sim card, now.
                if (newCardType > CARD_TYPE_NONE) {
                    if (oldCardType != newCardType) {
                        m_card_type_states[slotId] = CARD_STATE_CARD_TYPE_CHANGED;
                    } else {
                        m_card_type_states[slotId] = CARD_STATE_NO_CHANGED;
                    }
                /// M: but there is no sim card, that mean: card has been plugout.
                } else if (newCardType == CARD_TYPE_NONE) {
                    m_card_type_states[slotId] = CARD_STATE_HOT_PLUGOUT;
                }
            /// M: There was no sim card.
            } else if (oldCardType == CARD_TYPE_NONE) {
                /// M:But there is a sim card, now. mean: card has been insertted.
                if (newCardType > CARD_TYPE_NONE) {
                    m_card_type_states[slotId] = CARD_STATE_HOT_PLUGIN;
                /// M: and there is no sim card.
                } else {
                    m_card_type_states[slotId] = CARD_STATE_NO_CHANGED;
                }
            }
        }

        if (isCommontSlotSupport() && !isVsimEnabled()) {
            if (needMergeNotify(slotId)) {
                RFX_LOG_D(RTC_MODE_CONTROLLER_TAG, "[onSimTaskReady] Slot %d, state : %d -> %d, "
                    "cardType : %d, CommonSlot need merge notify, return",
                    slotId, old_value.asBool(), value.asBool(), m_card_types[slotId]);
                if (mTimeoutHandle != NULL) {
                    RfxTimer::stop(mTimeoutHandle);
                    mTimeoutHandle = NULL;
                }
                mTimeoutHandle = RfxTimer::start(RfxCallback0(this,
                        &RtcCardTypeReadyController::onMergeNotifyTimeout),
                        ms2ns(m_timeout_msec));
                return;
            }
        } else {
            /// M: If this action is not sim task reset or bootup case, then should set
            /// other slots card state to CARD_STATE_NO_CHANGED
            if (m_card_type_states[slotId] != CARD_STATE_NOT_HOT_PLUG) {
                for (int index = RFX_SLOT_ID_0; index < RFX_SLOT_COUNT; index++) {
                    if (index != slotId) {
                        m_card_type_states[index] = CARD_STATE_NO_CHANGED;
                    }
                }
            }
        }
    } else {
        m_card_types[slotId] = CARD_TYPE_INVALID;
    }

    RFX_LOG_D(RTC_MODE_CONTROLLER_TAG, "[onSimTaskReady] Slot %d, state : %d -> %d, cardType : %d",
            slotId, old_value.asBool(), value.asBool(), m_card_types[slotId]);

    for (int i = RFX_SLOT_ID_0; i < RFX_SLOT_COUNT; i++) {
        if (m_card_types[i] == CARD_TYPE_INVALID) {
            return;
        }
    }

    if (mTimeoutHandle != NULL) {
        RfxTimer::stop(mTimeoutHandle);
        mTimeoutHandle = NULL;
    }

    RtcModeSwitchController* modeSwitchController =
            (RtcModeSwitchController *)findController(RFX_OBJ_CLASS_INFO(RtcModeSwitchController));
    modeSwitchController->onCardTypeReady(m_card_types, m_card_type_states, RFX_SLOT_COUNT);
}

bool RtcCardTypeReadyController::isCommontSlotSupport() {
    char property_value[MTK_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get(PROPERTY_COMMON_SLOT_SUPPORT, property_value, "0");
    return (atoi(property_value) == 1);
}

bool RtcCardTypeReadyController::isVsimEnabled() {
    bool enabled = false;
    char vsim_enabled_prop[MTK_PROPERTY_VALUE_MAX] = { 0 };
    for (int i = RFX_SLOT_ID_0; i < RFX_SLOT_COUNT; i++) {
        getMSimProperty(i, (char*)PROPERTY_EXTERNAL_SIM_ENABLED, vsim_enabled_prop);
        if (atoi(vsim_enabled_prop) > 0) {
            enabled = true;
            break;
        }
    }
    return enabled;
}

bool RtcCardTypeReadyController::needMergeNotify(int slotId) {
    if (m_card_type_states[slotId] == CARD_STATE_HOT_PLUGIN) {
        for (int i = RFX_SLOT_ID_0; i < RFX_SLOT_COUNT; i++) {
            if (i == slotId) {
                continue;
            }
            if (m_card_types[i] == CARD_TYPE_NONE
                    && m_card_type_states[i] != CARD_STATE_NO_CHANGED) {
                return true;
            }
        }
    } else if (m_card_type_states[slotId] == CARD_STATE_HOT_PLUGOUT) {
        for (int i = RFX_SLOT_ID_0; i < RFX_SLOT_COUNT; i++) {
            if (i == slotId) {
                continue;
            }
            if (m_card_types[i] > CARD_TYPE_NONE) {
                return true;
            }
        }
    }
    return false;
}

void RtcCardTypeReadyController::onMergeNotifyTimeout() {
    RFX_LOG_D(RTC_MODE_CONTROLLER_TAG, "[onMergeNotifyTimeout]");
    RtcModeSwitchController* modeSwitchController =
            (RtcModeSwitchController *)findController(RFX_OBJ_CLASS_INFO(RtcModeSwitchController));
    modeSwitchController->onCardTypeReady(m_card_types, m_card_type_states, RFX_SLOT_COUNT);
}

void RtcCardTypeReadyController::setTimeoutMescForCommonSlotUT(int msec) {
    RFX_LOG_D(RTC_MODE_CONTROLLER_TAG, "[setTimeoutMescForCommonSlotUT] %d", msec);
    m_timeout_msec = msec;
}

bool RtcCardTypeReadyController::isAnyCardInvalid() {
    int i;
    // If other card type is "INVALID", it means mode switch didn't trigger
    // at previous onSimTaskReady.So, keep card state.
    for (i = 0; i < RFX_SLOT_COUNT; i++) {
        if (CARD_TYPE_INVALID == m_card_types[i]) {
            RFX_LOG_D(RTC_MODE_CONTROLLER_TAG, "[onSimTaskReady] slot[%d] is invalid, do not update\
card state to no change", i);
            break;
        }
    }
    return (i == RFX_SLOT_COUNT)? false: true;
}

void RtcCardTypeReadyController::onSimSwitchChanged(RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    int oldState = old_value.asInt();
    int newState = value.asInt();

    RFX_LOG_D(RTC_MODE_CONTROLLER_TAG, "onSimSwitchChanged (%d, %d), reset card type",
            oldState, newState);

    if (CAPABILITY_SWITCH_STATE_START == newState) {
        // Reset card type.
        for(int i = 0; i < RFX_SLOT_COUNT; i++) {
            m_card_types[i] = CARD_TYPE_INVALID;
        }
    }
}