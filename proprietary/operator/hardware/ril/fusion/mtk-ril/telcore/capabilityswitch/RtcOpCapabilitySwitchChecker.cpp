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
#include "RtcOpCapabilitySwitchChecker.h"
#include "libmtkrilutils.h"
#include "ratconfig.h"
#include "power/RtcRadioController.h"
#include "RtcCapabilityGetController.h"
#include "RtcCapabilitySwitchChecker.h"
#include "RtcCapabilitySwitchUtil.h"
#include "rfx_properties.h"
#include "RfxAction.h"
#include "RfxDefs.h"
#include "RfxLog.h"
#include "RfxMessageId.h"
#include "RfxRadioCapabilityData.h"
#include "RfxRilUtils.h"
#include "RfxRootController.h"
#include "RfxStatusDefs.h"
#include "RfxStatusManager.h"

#define RFX_LOG_TAG "RtcOpCapa"
/*****************************************************************************
 * Class RtcCapabilitySwitchChecker
 *****************************************************************************/
RFX_IMPLEMENT_CLASS("RtcOpCapabilitySwitchChecker", RtcOpCapabilitySwitchChecker, RtcCapabilitySwitchChecker);

RtcOpCapabilitySwitchChecker::RtcOpCapabilitySwitchChecker() {
}

RtcOpCapabilitySwitchChecker::~RtcOpCapabilitySwitchChecker() {
}

int RtcOpCapabilitySwitchChecker::getHigherPrioritySlot() {
    int current_major_slot = m_status_managers[MAX_RFX_SLOT_ID]->getIntValue(
            RFX_STATUS_KEY_MAIN_CAPABILITY_SLOT, 0);

    if (RtcCapabilitySwitchUtil::isSimSwitchEnabled() == false) {
        RFX_LOG_D(RFX_LOG_TAG, "getHigherPrioritySlot, sim switch is disabled");
        return SLOT_0;
    }

    int sim_count = RfxRilUtils::rfxGetSimCount();
    int gemini_mode = getGeminiMode();
    RFX_LOG_D(RFX_LOG_TAG, "getHigherPrioritySlot, sim_count=%d, mode=%d, cur=%d",
              sim_count, gemini_mode, current_major_slot);

    if (sim_count > 3) {
        RFX_LOG_D(RFX_LOG_TAG, "4sim do switch");
        return UNKNOWN_SLOT;
    } else if (sim_count == 3) {
        if ((gemini_mode != GEMINI_MODE_L_AND_L) || RatConfig_isC2kSupported() ||
            RatConfig_isTdscdmaSupported()) {
            RFX_LOG_D(RFX_LOG_TAG, "3sim do switch");
            return UNKNOWN_SLOT;
        } else {
            RFX_LOG_D(RFX_LOG_TAG, "3sim skip switch");
            return CURRENT_SLOT;
        }
    } else if (sim_count == 2) {
        // trigger sim switch for non-dual-LTE project
        if (gemini_mode != GEMINI_MODE_L_AND_L) {
            RFX_LOG_D(RFX_LOG_TAG, "single LTE project, do switch");
            return UNKNOWN_SLOT;
        } else {
            // don't support TD-SCDMA and CDMA
            if ((RatConfig_isC2kSupported() == 0) && (RatConfig_isTdscdmaSupported() == 0)) {
                RFX_LOG_D(RFX_LOG_TAG, "not support C2K and TD-SCDMA, skip switch");
                return CURRENT_SLOT;
            }
        }
    } else if (sim_count == 1) {
        RFX_LOG_D(RFX_LOG_TAG, "single sim project, main protocol should keep on slot0");
        return SLOT_0;
    } else {
        RFX_LOG_E(RFX_LOG_TAG, "sim_count <= 0, should not be here");
        return CURRENT_SLOT;
    }

    if (imsiReady(true) == false) {
        RFX_LOG_E(RFX_LOG_TAG, "getHigherPrioritySlot, sim is not ready.");
        return UNKNOWN_SLOT;
    }

    int inserted_sim_count = 0;
    int op01_sim_count = 0;
    int common_gsm_sim_count = 0;
    int c2k_sim_count = 0;
    int op01_sim_slot_id = -1;
    int c2k_sim_slot_id = -1;
    int c2k_volte_state = -1;
    int c2k_volte_off_sim_count = 0;
    int c2k_volte_on_sim_count = 0;
    int c2k_volte_off_slot_id = -1;
    for (int i = 0; i < sim_count; i++) {
        int sim_state = m_status_managers[i]->getIntValue(RFX_STATUS_KEY_SIM_STATE);
        if (sim_state != RFX_SIM_STATE_ABSENT) {
            inserted_sim_count++;
            if (RtcCapabilitySwitchUtil::isOp01Sim(getImsi(i))) {
                op01_sim_count++;
                op01_sim_slot_id = i;
            } else if(isCdmaOnlySim(i) || isCdmaDualModeSim(i) ||
                      RtcCapabilitySwitchUtil::isOp09Sim(getImsi(i))) {
                c2k_sim_count++;
                c2k_sim_slot_id = i;
                c2k_volte_state = m_status_managers[i]->getIntValue(
                        RFX_STATUS_KEY_VOLTE_STATE, -1);
                if ((c2k_volte_state == 0) || (isLteNetworkType(m_status_managers[i]->getIntValue(
                        RFX_STATUS_KEY_PREFERRED_NW_TYPE, -1)) == false)) {
                    c2k_volte_off_sim_count++;
                    c2k_volte_off_slot_id = i;
                } else {
                    c2k_volte_on_sim_count++;
                }
            } else {
                common_gsm_sim_count++;
            }
        }
    }
    RFX_LOG_D(RFX_LOG_TAG, "getHigherPrioritySlot, t:%d, c:%d(%d,%d,%d), w:%d, config:%ld",
              op01_sim_count, c2k_sim_count, c2k_volte_on_sim_count, c2k_volte_off_sim_count,
              c2k_volte_off_slot_id, common_gsm_sim_count, m_waive_enhance_config);
    if (isSupportSimSwitchEnhancement(ENHANCEMENT_T_PLUS_W) && (inserted_sim_count == 2) &&
               (common_gsm_sim_count == 1) && (op01_sim_count == 1)) {
        if (RatConfig_isTdscdmaSupported()) {
            // t + w --> always on t card if support TD-SCDMA
            return op01_sim_slot_id;
        } else {
            // t + w --> don't need to capability switch if not support TD-SCDMA
            return CURRENT_SLOT;
        }
    } else if (isSupportSimSwitchEnhancement(ENHANCEMENT_T_PLUS_C) &&
               (inserted_sim_count == 2) && (op01_sim_count == 1) && (c2k_sim_count == 1)) {
        // t + c --> always on c card
        return c2k_sim_slot_id;
    } else if (isSupportSimSwitchEnhancement(ENHANCEMENT_W_PLUS_C) &&
               (inserted_sim_count == 2) && (common_gsm_sim_count == 1) && (c2k_sim_count == 1)) {
        // w + c--> always on c card
        return c2k_sim_slot_id;
    } else if (isSupportSimSwitchEnhancement(ENHANCEMENT_W_PLUS_W) &&
               (inserted_sim_count == 2) && (common_gsm_sim_count == 2)) {
        // w + w --> don't need to capability switch
        return CURRENT_SLOT;
    } else if (isSupportSimSwitchEnhancement(ENHANCEMENT_W_PLUS_NA) &&
               (inserted_sim_count == 1) && (common_gsm_sim_count == 1)) {
        // w + empty --> don't need to capability switch
        return CURRENT_SLOT;
    } else if (isSupportSimSwitchEnhancement(ENHANCEMENT_C_PLUS_C) && (inserted_sim_count == 2) &&
               (c2k_sim_count == 2) && RfxRilUtils::isCtVolteSupport()) {
        if (RatConfig_isC2kSupported()) {
            // c (volte on) + c (volte on) --> don't need to capability switch
            // c (volte on) + c (volte off) --> always on c volte off card
            // c (volte off) + c (volte off) --> don't skip capability switch
            if (c2k_volte_on_sim_count == 2) {
                return CURRENT_SLOT;
            } else if ((c2k_volte_on_sim_count == 1) && (c2k_volte_off_sim_count == 1)) {
                return c2k_volte_off_slot_id;
            }
        } else {
            // c + c --> don't need to capability switch if not support CDMA
            return CURRENT_SLOT;
        }
    }
    return UNKNOWN_SLOT;
}

bool RtcOpCapabilitySwitchChecker::isSkipCapabilitySwitch(int new_major_slot) {
    int current_major_slot = m_status_managers[MAX_RFX_SLOT_ID]->getIntValue(
            RFX_STATUS_KEY_MAIN_CAPABILITY_SLOT, 0);
    int sim_count = RfxRilUtils::rfxGetSimCount();
    bool ret = false;
    if (new_major_slot == current_major_slot) {
        RFX_LOG_D(RFX_LOG_TAG, "skip switch, new==current:%d", new_major_slot);
        return true;
    }
    if (isInPukLock(false)) {
        RFX_LOG_D(RFX_LOG_TAG, "skip switch, in PUK lock state");
        return true;
    }

    int rsim = RfxRilUtils::getRemoteSimSlot();
    if (0 != RfxRilUtils::isExternalSimSupport() && 0 != RfxRilUtils::isNonDsdaRemoteSupport() &&
        (rsim != -1)) {
        // External SIM
        RFX_LOG_D(RFX_LOG_TAG, "skip switch, rsim enabled:%d", rsim);
        if (rsim == new_major_slot) {
            ret = false;
        } else {
            ret = true;
        }
    } else {
        int high_priority_slot = getHigherPrioritySlot();
        if (high_priority_slot == current_major_slot) {
            RFX_LOG_D(RFX_LOG_TAG, "skip switch, high priority slot:%d", new_major_slot);
            ret = true;
        } else if (high_priority_slot == CURRENT_SLOT) {
            if ((sim_count == 3) && ((new_major_slot == SLOT_2) || (current_major_slot == SLOT_2))) {
                RFX_LOG_D(RFX_LOG_TAG, "3sim do switch, new slot:%d", new_major_slot);
            } else {
                RFX_LOG_D(RFX_LOG_TAG, "skip switch, keep on current slot:%d", new_major_slot);
                ret = true;
            }
        } else if (isSatisfyOperatorRules(new_major_slot) == false){
            RFX_LOG_D(RFX_LOG_TAG, "skip switch, not satisfy op rules:%d", new_major_slot);
            ret = true;
        }
    }

    if (ret) {
        rfx_property_set("persist.vendor.radio.pendcapswt", String8::format("%d", -1).string());
    } else {
        RFX_LOG_D(RFX_LOG_TAG, "do switch, new slot:%d", new_major_slot);
    }
    return ret;
}

bool RtcOpCapabilitySwitchChecker::isSatisfyOperatorRules(int new_major_slot) {
    bool ret = true;
    char optr_value[RFX_PROPERTY_VALUE_MAX] = {0};
    rfx_property_get("persist.vendor.operator.optr", optr_value, "0");
    if (strcmp(optr_value, OPERATOR_OP01) == 0) {
        ret = isSatisfyOp01Rules(new_major_slot);
    }
    return ret;
}

bool RtcOpCapabilitySwitchChecker::isSatisfyOp01Rules(int new_major_slot) {
    return true;
}


