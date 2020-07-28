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
#include <stdio.h>
#include <stdlib.h>
#include <cutils/properties.h>
#include "RfxLog.h"
#include "RfxObject.h"
#include "RfxRootController.h"
#include "RpIrUtils.h"
#include "util/RpFeatureOptionUtils.h"

#define RFX_LOG_TAG "[IRC][Utils]"
/*****************************************************************************
 * Class RpIrUtils
 *****************************************************************************/
extern "C" int RatConfig_isWcdmaSupported();
extern "C" int RatConfig_isTdscdmaSupported();

RpIrUtils::RpIrUtils() {
}

RpIrUtils::~RpIrUtils() {
}

const char* RpIrUtils::boolToString(bool value) {
    return value ? "true" : "false";
}

const char* RpIrUtils::nwsModeToString(NwsMode mode) {
    switch (mode) {
    case NWS_MODE_UNKNOWN:
        return "NWS_MODE_UNKNOWN";
    case NWS_MODE_CDMALTE:
        return "NWS_MODE_CDMALTE";
    case NWS_MODE_CSFB:
        return "NWS_MODE_CSFB";
    case NWS_MODE_LTEONLY:
        return "NWS_MODE_LTEONLY";
    default:
        RFX_LOG_E(RFX_LOG_TAG , "nwsModeToString, invalid NWS Mode");
        break;
    }
    RFX_ASSERT(0);
    return "";
}

const char* RpIrUtils::volteStateToString(VolteState volteState) {
    switch (volteState) {
    case VOLTE_ON:
        return "VOLTE_ON";
    case VOLTE_OFF:
        return "VOLTE_OFF";
    default:
        RFX_LOG_E(RFX_LOG_TAG , "volteStateToString, invalid Volte State");
        break;
    }
    RFX_ASSERT(0);
    return "";
}

const char* RpIrUtils::serviceTypeToString(ServiceType serviceType) {
    switch (serviceType) {
        case OUT_OF_SERVICE:
            return "OUT_OF_SERVICE";
        case IN_SEARCHING:
            return "IN_SEARCHING";
        case IN_SERVICE:
            return "IN_SERVICE";
        default:
            RFX_LOG_E(RFX_LOG_TAG , "serviceTypeToString, invalid service type");
            break;
    }
    RFX_ASSERT(0);
    return "";
}

const char* RpIrUtils::prefNwType2Str(int prefNwType) {
    switch (prefNwType) {
    case PREF_NET_TYPE_GSM_WCDMA:
        return "PREF_NET_TYPE_GSM_WCDMA";
    case PREF_NET_TYPE_GSM_ONLY:
        return "PREF_NET_TYPE_GSM_ONLY";
    case PREF_NET_TYPE_WCDMA:
        return "PREF_NET_TYPE_WCDMA";
    case PREF_NET_TYPE_GSM_WCDMA_AUTO:
        return "PREF_NET_TYPE_GSM_WCDMA_AUTO";
    case PREF_NET_TYPE_CDMA_EVDO_AUTO:
        return "PREF_NET_TYPE_CDMA_EVDO_AUTO";
    case PREF_NET_TYPE_CDMA_ONLY:
        return "PREF_NET_TYPE_CDMA_ONLY";
    case PREF_NET_TYPE_EVDO_ONLY:
        return "PREF_NET_TYPE_EVDO_ONLY";
    case PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO:
        return "PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO";
    case PREF_NET_TYPE_LTE_CDMA_EVDO:
        return "PREF_NET_TYPE_LTE_CDMA_EVDO";
    case PREF_NET_TYPE_LTE_GSM_WCDMA:
        return "PREF_NET_TYPE_LTE_GSM_WCDMA";
    case PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA:
        return "PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA";
    case PREF_NET_TYPE_LTE_ONLY:
        return "PREF_NET_TYPE_LTE_ONLY";
    case PREF_NET_TYPE_LTE_WCDMA:
        return "PREF_NET_TYPE_LTE_WCDMA";
    case PREF_NET_TYPE_TD_SCDMA_ONLY:
        return "PREF_NET_TYPE_TD_SCDMA_ONLY";
    case PREF_NET_TYPE_TD_SCDMA_WCDMA:
        return "PREF_NET_TYPE_TD_SCDMA_WCDMA";
    case PREF_NET_TYPE_TD_SCDMA_LTE:
        return "PREF_NET_TYPE_TD_SCDMA_LTE";
    case PREF_NET_TYPE_TD_SCDMA_GSM:
        return "PREF_NET_TYPE_TD_SCDMA_GSM";
    case PREF_NET_TYPE_TD_SCDMA_GSM_LTE:
        return "PREF_NET_TYPE_TD_SCDMA_GSM_LTE";
    case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA:
        return "PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA";
    case PREF_NET_TYPE_TD_SCDMA_WCDMA_LTE:
        return "PREF_NET_TYPE_TD_SCDMA_WCDMA_LTE";
    case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_LTE:
        return "PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_LTE";
    case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO:
        return "PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO";
    case PREF_NET_TYPE_TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA:
        return "PREF_NET_TYPE_TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA";
    case PREF_NET_TYPE_LTE_GSM:
        return "PREF_NET_TYPE_LTE_GSM";
    case PREF_NET_TYPE_LTE_TDD_ONLY:
        return "PREF_NET_TYPE_LTE_TDD_ONLY";
    default:
        break;
    }
    return "unknown";
}

bool RpIrUtils::is3GPrefNwType(int prefNwType) {
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
    case PREF_NET_TYPE_LTE_TDD_ONLY:
        return false;
    default:
        return true;
    }
}

bool RpIrUtils::isWorldPhoneSupport() {
    bool isWdmdaSupport = RatConfig_isWcdmaSupported() == 1 ? true : false;
    bool isTdsCDMASupport = RatConfig_isTdscdmaSupported() == 1 ? true :false;
    return isWdmdaSupport && isTdsCDMASupport;
}

bool RpIrUtils::isWorldModeSupport() {
    char strProp[PROPERTY_VALUE_MAX] = {0};
    property_get("ro.vendor.mtk_md_world_mode_support", strProp, "0");
    int nProp = atoi(strProp);
    return nProp == 1 ? true : false;
}

bool RpIrUtils::isViceSimSupportLte() {
    char strProp[PROPERTY_VALUE_MAX] = {0};
    property_get("persist.vendor.radio.mtk_ps2_rat", strProp, "");
    return strcmp("L/W/G", strProp) == 0 ? true : false;
}

bool RpIrUtils::isSimReady(int slot_id) {
    int card_type = RFX_OBJ_GET_INSTANCE(RfxRootController)->getStatusManager(
            slot_id)->getIntValue(RFX_STATUS_KEY_CARD_TYPE, 0);
    if (card_type <= 0) {
        RFX_LOG_D(RFX_LOG_TAG, "isSimReady, slot=%d, card type=%d, not ready",
                slot_id, card_type);
        return false;
    }
    int sim_state = RFX_OBJ_GET_INSTANCE(RfxRootController)->getStatusManager(
            slot_id)->getIntValue(RFX_STATUS_KEY_SIM_STATE);
    bool bSimReady = sim_state == RFX_SIM_STATE_READY ? true : false;
    RFX_LOG_D(RFX_LOG_TAG, "isSimReady, slot=%d, bSimReady=%s, card_type=%d, sim_state=%d",
            slot_id, Bool2Str(bSimReady), card_type, sim_state);
    return bSimReady;
}

bool RpIrUtils::isSimLocked(int slot_id) {
    int card_type = RFX_OBJ_GET_INSTANCE(RfxRootController)->getStatusManager(
            slot_id)->getIntValue(RFX_STATUS_KEY_CARD_TYPE, 0);
    if (card_type <= 0) {
        RFX_LOG_D(RFX_LOG_TAG, "isSimLocked, slot=%d, card type=%d, not ready", slot_id, card_type);
        return false;
    }
    int sim_state = RFX_OBJ_GET_INSTANCE(RfxRootController)->getStatusManager(
            slot_id)->getIntValue(RFX_STATUS_KEY_SIM_STATE);
    bool bSimLocked = sim_state == RFX_SIM_STATE_LOCKED ? true : false;
    RFX_LOG_D(RFX_LOG_TAG, "isSimLocked, slot=%d, bSimLocked=%s, card_type=%d, sim_state=%d",
            slot_id, Bool2Str(bSimLocked), card_type, sim_state);
    return bSimLocked;
}

bool RpIrUtils::isCdma3GDualModeCard(int slot_id) {
    /*********************************
    * For CDMA 3G dual mode card, RFX_STATUS_KEY_CARD_TYPE will be changed when roaming mode changed.
    * And RFX_STATUS_KEY_CT3G_DUALMODE_CARD will not change no matter CDMALTE or CSFB.
    * So, it can be used to check if card is CDMA(not only for CT) dual mode card.
    **********************************/
    bool ret = RFX_OBJ_GET_INSTANCE(RfxRootController)->getStatusManager(
            slot_id)->getBoolValue(RFX_STATUS_KEY_CT3G_DUALMODE_CARD);
    return ret;
}

bool RpIrUtils::isCdma4GDualModeCard(int slot_id) {
    /*********************************
    * For CDMA 4G dual mode card, there isn't any key like RFX_STATUS_KEY_CT3G_DUALMODE_CARD to
    * check if card is 4G dual mode card. Here we use RFX_STATUS_KEY_CARD_TYPE to check it.
    * Different with 3G dual mode card, value of RFX_STATUS_KEY_CARD_TYPE will not changed
    * in CDMALTE and CSFB.
    **********************************/
    int nCardType = RFX_OBJ_GET_INSTANCE(RfxRootController)->getStatusManager(
            slot_id)->getIntValue(RFX_STATUS_KEY_CARD_TYPE);
    bool ret = RFX_FLAG_HAS_ALL(nCardType, (RFX_CARD_TYPE_USIM | RFX_CARD_TYPE_CSIM));
    return ret;
}

bool RpIrUtils::isCdmaDualModeSimCard(int slot_id) {
    return isCdma3GDualModeCard(slot_id) || isCdma4GDualModeCard(slot_id);
}

bool RpIrUtils::isSupportRoaming(int slot_id) {
    if (!isCdmaDualModeSimCard(slot_id)) {
        return false;
    }
    bool beSupportRoaming = false;
    if (slot_id == getCdmaSlotId()) {
        beSupportRoaming = true;
    } else if (isViceOfTwoCtDualVolte(slot_id)) {
        beSupportRoaming = true;
        RFX_LOG_D(RFX_LOG_TAG, "isSupportRoaming slot=%d, ret= %s", slot_id, Bool2Str(beSupportRoaming));
    } else {
        // Do nothing
    }
    return beSupportRoaming;
}

bool RpIrUtils::isViceOfTwoCtDualVolte(int slot_id) {
    bool ret = false;
    if (RpFeatureOptionUtils::isMultipleImsSupport()
            && RpFeatureOptionUtils::isCtVolteSupport()     // CT dual VoLTE
            && CT_4G_UICC_CARD == getCtCardType(slot_id)    // CT 4G card
            && slot_id != getMainCapSlot()
            && slot_id != getCdmaSlotId()) {               // Vice card
        ret = true;
    }
    RFX_LOG_D(RFX_LOG_TAG, "isViceOfTwoCtDualVolte slot=%d, CDMA card type=%d, \
            Main Cap Slot=%d, CDMA slot ID=%d, ret= %s",
            slot_id,
            getCtCardType(slot_id),
            getMainCapSlot(),
            getCdmaSlotId(),
            Bool2Str(ret));
    return ret;
}

int RpIrUtils::getCtCardType(int slot_id) {
    int nCtCardType = RFX_OBJ_GET_INSTANCE(RfxRootController)->getStatusManager(
            slot_id)->getIntValue(RFX_STATUS_KEY_CDMA_CARD_TYPE, RFX_CDMA_CARD_UNKNOWN);
    return nCtCardType;
}

int RpIrUtils::getMainCapSlot() {
    int slotId = RFX_OBJ_GET_INSTANCE(RfxRootController)->getNonSlotScopeStatusManager()->getIntValue(
            RFX_STATUS_KEY_MAIN_CAPABILITY_SLOT, RFX_SLOT_ID_UNKNOWN);
    return slotId;
}

int RpIrUtils::getCdmaSlotId() {
    int slotId = RFX_OBJ_GET_INSTANCE(RfxRootController)->getNonSlotScopeStatusManager()->getIntValue(
            RFX_STATUS_KEY_CDMA_SOCKET_SLOT, RFX_SLOT_ID_UNKNOWN);
    return slotId;
}

/*******************************************************
* public static final int APP_FAM_NONE = 0x00;
* public static final int APP_FAM_3GPP = 0x01;
* public static final int APP_FAM_3GPP2 = 0x02;

* Logic from PhoneInterfaceManagerEx::getIccAppFamily()
*
* if (fullType == TelephonyManagerEx.CARD_TYPE_NONE) {
*     iccType = TelephonyManagerEx.APP_FAM_NONE;
* } else if ((fullType & TelephonyManagerEx.CARD_TYPE_CSIM) != 0
*         && (fullType & TelephonyManagerEx.CARD_TYPE_USIM) != 0) {
*     iccType = TelephonyManagerEx.APP_FAM_3GPP2 | TelephonyManagerEx.APP_FAM_3GPP;
* } else if ((fullType & TelephonyManagerEx.CARD_TYPE_CSIM) != 0
*         || (fullType & TelephonyManagerEx.CARD_TYPE_RUIM) != 0) {
*     iccType = TelephonyManagerEx.APP_FAM_3GPP2;
* } else {
*     iccType = TelephonyManagerEx.APP_FAM_3GPP;
*
*     // Uim dual mode sim, may switch to SIM type for use
*     if (fullType == TelephonyManagerEx.CARD_TYPE_SIM) {
*         String uimDualMode = SystemProperties.get(PROPERTY_RIL_CT3G[slotId]);
*         if ("1".equals(uimDualMode)) {
*             iccType = TelephonyManagerEx.APP_FAM_3GPP2;
*         }
*     }
* }
*******************************************************/
int RpIrUtils::getIccAppFamily(int slot_id) {
    int iccFamily = 0;
    int nCardType = RFX_OBJ_GET_INSTANCE(RfxRootController)->getStatusManager(
            slot_id)->getIntValue(RFX_STATUS_KEY_CARD_TYPE);

    if (nCardType == 0) {
        iccFamily = 0; // no SIM detected
    } else if (RFX_FLAG_HAS_ALL(nCardType, (RFX_CARD_TYPE_USIM | RFX_CARD_TYPE_CSIM))) {
        iccFamily = 3; // 3GPP | 3GPP2, CDMA 4G dual mode card
    } else if (RFX_OBJ_GET_INSTANCE(RfxRootController)->getStatusManager(
            slot_id)->getBoolValue(RFX_STATUS_KEY_CT3G_DUALMODE_CARD)) {
        iccFamily = 2; // 3GPP2, CDMA 3G dual mode card
    } else if (nCardType == RFX_CARD_TYPE_CSIM | nCardType == RFX_CARD_TYPE_RUIM) {
        iccFamily = 2; // 3GPP2,
    } else {
        iccFamily = 1; // 3GPP
    }
    return iccFamily;
}

int RpIrUtils::isCdmaLteDcSupport() {
    int isCdmaLteSupport = 0;
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ro.vendor.mtk_c2k_lte_mode", property_value, "0");
    isCdmaLteSupport = atoi(property_value);
    return isCdmaLteSupport ? 1 : 0;
}

