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

/************************************************************************
 * Include
 ************************************************************************/
#include <cutils/properties.h>
#include <mutex>
#include "RfxLog.h"
#include "RfxRootController.h"
#include "RpCdmaLteDefs.h"
#include "RpOMCardModeController.h"
#include "RpCdmaLteModeController.h"


/************************************************************************
 * Class RpOMCardModeController
 ************************************************************************/
std::mutex mtxx;
RpOMCardModeController* RpOMCardModeController::s_RpOMCardModeController;

RpOMCardModeController::RpOMCardModeController() {
}

RpOMCardModeController::~RpOMCardModeController() {
}

RpOMCardModeController* RpOMCardModeController::getInstance() {
    if (s_RpOMCardModeController == NULL) {
        mtxx.lock();
        if (s_RpOMCardModeController == NULL) {
            s_RpOMCardModeController = new RpOMCardModeController();
        }
        mtxx.unlock();
    }
    return s_RpOMCardModeController;
}

int RpOMCardModeController::getPreferRadioTech(int cardType, int slot,
    int* oldCardModes) {
    RFX_UNUSED(oldCardModes);
    int prefer = RADIO_TECH_MODE_UNKNOWN;
    if ((cardType & CARD_TYPE_RUIM) > 0 ||
            (cardType & CARD_TYPE_CSIM) > 0) {
        prefer = RADIO_TECH_MODE_CDMALTE;
    } else if (cardType == CARD_TYPE_NONE) {
        prefer = oldCardModes[slot];
    } else {
        prefer = RADIO_TECH_MODE_CSFB;
    }
    RFX_LOG_D(RP_CDMALTE_MODE_TAG,
            "getPreferRadioTech; cartType:%d, preferNetWorkType: %d", cardType, prefer);
    return prefer;
}

void RpOMCardModeController::calculateCardMode(ModeSwitchInfo* info) {

    RFX_LOG_D(RP_CDMALTE_MODE_TAG,"[SMC][calculateCardMode] >>> OPERATOR_OM case");
    for (int i = 0; i < RFX_SLOT_COUNT; i++) {
        info->cardTypeBeSwitch[i] = INVALID_MODE;
    }

    int* slotPreferRadioTec = new int[RFX_SLOT_COUNT];
    /// M: how many cards been inserted.
    int insertCount = 0;
    /// M: How many cards' mode been set to cdmalte.
    int cdmaLteCount = 0;
    /// M: How man cards' mode been set to csfb.
    int csfbCount = 0;
    int mode = -1;
    int cardType = CARD_TYPE_NONE;
    for (int index = 0; index < RFX_SLOT_COUNT; index ++) {
        cardType = info->card_types[index];
        if (cardType != CARD_TYPE_NONE) {
            insertCount++;
        }
        mode = getPreferRadioTech(cardType, index, info->old_card_modes);
        if (mode == RADIO_TECH_MODE_CDMALTE) {
            cdmaLteCount++;
        } else if (mode == RADIO_TECH_MODE_CSFB) {
            csfbCount++;
        }
       slotPreferRadioTec[index] = mode;
       info->card_modes[index] = mode;
    }

    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    /// M: Get capability Slot.
    property_get("persist.vendor.radio.simswitch", property_value, "1");
    int capabilitySlot = atoi(property_value) - 1;

    /**
     * M: This case for CT4G+CT3G Dual. If ct3g card has been changed to Gsm card
     * and this ct3g card has 3/4 G capability, should set other ct cards' card mode to csfb.
     */
    if (cdmaLteCount == 1 && insertCount > 1) {
        int cSlot = -1;
        for (int index = 0; index < RFX_SLOT_COUNT; index ++) {
            mode = slotPreferRadioTec[index];
            if (mode == RADIO_TECH_MODE_CDMALTE) {
                cSlot = index;
                if (cSlot != capabilitySlot && info->isCt3GDualMode[capabilitySlot]) {
                    info->card_modes[cSlot] = RADIO_TECH_MODE_CSFB;
                    // CT3G + CT3G case, we need switch card type of cSlot.
                    if (info->isCt3GDualMode[cSlot]) {
                        info->cardTypeBeSwitch[cSlot] = SIM_MODE;
                    }
                }
                break;
            }
        }
    } else if (cdmaLteCount > 1 && insertCount > 0) {
        /**
         * M: Should set the ct card's ( which has 3/4 G capability ) mode to cdmalte.
         * other cards' mode should be set to csfb.
         * And find ct cards which will been change to Gsm Card.
         */
        if (slotPreferRadioTec[capabilitySlot] == RADIO_TECH_MODE_CDMALTE &&
                info->card_types[capabilitySlot] != CARD_TYPE_NONE) {
            info->card_modes[capabilitySlot] = RADIO_TECH_MODE_CDMALTE;
            for (int index =0; index < RFX_SLOT_COUNT; index++) {
                if (index != capabilitySlot) {
                    info->card_modes[index] = RADIO_TECH_MODE_CSFB;

                    mode = slotPreferRadioTec[index];
                    if (mode == RADIO_TECH_MODE_CDMALTE && info->isCt3GDualMode[index]) {
                        info->cardTypeBeSwitch[index] = SIM_MODE;
                    }
                }
            }
        } else {
            /// M: Find the first c card and set it's card mode to cdmalte, set other c cards' mode to csfb.
            int firstCCard = -1;
            for (int index =0; index < RFX_SLOT_COUNT; index++) {
                mode = slotPreferRadioTec[index];
                if (mode == RADIO_TECH_MODE_CDMALTE && firstCCard == -1 &&
                        info->card_types[index] != CARD_TYPE_NONE) {
                    info->card_modes[index] = RADIO_TECH_MODE_CDMALTE;
                    firstCCard = index;
                } else {
                    info->card_modes[index] = RADIO_TECH_MODE_CSFB;
                    if (mode == RADIO_TECH_MODE_CDMALTE && info->isCt3GDualMode[index]) {
                        RFX_LOG_D(RP_CDMALTE_MODE_TAG,
                                "[SMC]calculateCardMode needSwitchCardTypeSlots: %d", index);
                        info->cardTypeBeSwitch[index] = SIM_MODE;
                    }
                }
            }
        }
    }

    for (int index = 0; index < RFX_SLOT_COUNT; index++) {
        RFX_LOG_D(RP_CDMALTE_MODE_TAG,
                "[SMC]calculateCardMode slot: %d, isCt3GDualMode: %d, mode: %s",
                 index, info->isCt3GDualMode[index],
                (info->card_modes[index] == RADIO_TECH_MODE_CDMALTE ?
                "RADIO_TECH_MODE_CDMALTE" :
                (info->card_modes[index] == RADIO_TECH_MODE_CSFB ? "RADIO_TECH_MODE_CSFB" : "NULL")));
    }

    delete []slotPreferRadioTec;
}
