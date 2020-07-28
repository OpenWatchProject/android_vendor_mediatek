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
#include "RtcCapabilitySwitchUtil.h"
#include <stdlib.h>
#include <string.h>
#include <telephony/mtk_ril.h>
#include "ratconfig.h"
#include "rfx_properties.h"
#include "RfxAction.h"
#include "RfxController.h"
#include "RfxLog.h"
#include "RfxMainThread.h"
#include "RfxMessageId.h"
#include "RfxRilUtils.h"

#define RFX_LOG_TAG "RtcCapa"

// OP01 SIMs
static const char* PLMN_TABLE_OP01[] = {
    "46000", "46002", "46007", "46008", "45412", "45413",
    // Lab test IMSI
    "00101", "00211", "00321", "00431", "00541", "00651",
    "00761", "00871", "00902", "01012", "01122", "01232",
    "46004", "46602", "50270"
};

// OP02 SIMs
static const char* PLMN_TABLE_OP02[] = {
    "46001", "46006", "46009", "45407"
};

// OP09 SIMs
static const char* PLMN_TABLE_OP09[] = {
    "46005", "45502", "46003", "46011"
};

// OP18 SIMs
static const char* PLMN_TABLE_OP18[] = {
    "405840", "405854", "405855", "405856",
    "405857", "405858", "405855", "405856",
    "405857", "405858", "405859", "405860",
    "405861", "405862", "405863", "405864",
    "405865", "405866", "405867", "405868",
    "405869", "405870", "405871", "405872",
    "405873", "405874"
};


/*****************************************************************************
 * Class RtcCapabilitySwitchUtil
 *****************************************************************************/
int RtcCapabilitySwitchUtil::getRafFromNetworkType(int type) {
    int raf;

    switch (type) {
        case PREF_NET_TYPE_GSM_WCDMA:
            raf = RAF_GSM_GROUP | RAF_WCDMA_GROUP;
            break;
        case PREF_NET_TYPE_GSM_ONLY:
            raf = RAF_GSM_GROUP;
            break;
        case PREF_NET_TYPE_WCDMA:
            raf = RAF_WCDMA_GROUP;
            break;
        case PREF_NET_TYPE_GSM_WCDMA_AUTO:
            raf = RAF_GSM_GROUP | RAF_WCDMA_GROUP;
            break;
        case PREF_NET_TYPE_CDMA_EVDO_AUTO:
            raf = RAF_CDMA_GROUP | RAF_EVDO_GROUP;
            break;
        case PREF_NET_TYPE_CDMA_ONLY:
            raf = RAF_CDMA_GROUP;
            break;
        case PREF_NET_TYPE_EVDO_ONLY:
            raf = RAF_EVDO_GROUP;
            break;
        case PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO:
            raf = RAF_GSM_GROUP | RAF_WCDMA_GROUP | RAF_CDMA_GROUP | RAF_EVDO_GROUP;
            break;
        case PREF_NET_TYPE_LTE_CDMA_EVDO:
            raf = RAF_LTE_GROUP | RAF_CDMA_GROUP | RAF_EVDO_GROUP;
            break;
        case PREF_NET_TYPE_LTE_GSM_WCDMA:
            raf = RAF_LTE_GROUP | RAF_GSM_GROUP | RAF_WCDMA_GROUP;
            break;
        case PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA:
            raf = RAF_LTE_GROUP | RAF_GSM_GROUP | RAF_WCDMA_GROUP | RAF_CDMA_GROUP | RAF_EVDO_GROUP;
            break;
        case PREF_NET_TYPE_LTE_ONLY:
            raf = RAF_LTE_GROUP;
            break;
        case PREF_NET_TYPE_LTE_WCDMA:
            raf = RAF_LTE_GROUP | RAF_WCDMA_GROUP;
            break;
        case PREF_NET_TYPE_TD_SCDMA_ONLY:
            raf = RAF_TD_SCDMA;
            break;
        case PREF_NET_TYPE_TD_SCDMA_WCDMA:
            raf = RAF_TD_SCDMA | RAF_WCDMA_GROUP;
            break;
        case PREF_NET_TYPE_TD_SCDMA_LTE:
            raf = RAF_TD_SCDMA | RAF_LTE_GROUP;
            break;
        case PREF_NET_TYPE_TD_SCDMA_GSM:
            raf = RAF_GSM_GROUP | RAF_TD_SCDMA;
            break;
        case PREF_NET_TYPE_TD_SCDMA_GSM_LTE:
            raf = RAF_TD_SCDMA | RAF_GSM_GROUP | RAF_LTE_GROUP;
            break;
        case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA:
            raf = RAF_WCDMA_GROUP | RAF_TD_SCDMA | RAF_GSM_GROUP;
            break;
        case PREF_NET_TYPE_TD_SCDMA_WCDMA_LTE:
            raf = RAF_LTE_GROUP | RAF_TD_SCDMA | RAF_WCDMA_GROUP;
            break;
        case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_LTE:
            raf = RAF_LTE_GROUP | RAF_TD_SCDMA | RAF_GSM_GROUP | RAF_WCDMA_GROUP;
            break;
        case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO:
            raf = RAF_TD_SCDMA | RAF_CDMA_GROUP | RAF_EVDO_GROUP | RAF_GSM_GROUP | RAF_WCDMA_GROUP;
            break;
        case PREF_NET_TYPE_TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA:
            raf = RAF_LTE_GROUP | RAF_TD_SCDMA | RAF_CDMA_GROUP | RAF_EVDO_GROUP | RAF_GSM_GROUP | RAF_WCDMA_GROUP;
            break;
        case PREF_NET_TYPE_NR_ONLY:
            raf = RAF_NR;
            break;
        case PREF_NET_TYPE_NR_LTE:
            raf = RAF_NR | RAF_LTE_GROUP;
            break;
        case PREF_NET_TYPE_NR_LTE_CDMA_EVDO:
            raf = RAF_NR | RAF_LTE_GROUP | RAF_CDMA_GROUP | RAF_EVDO_GROUP;
            break;
        case PREF_NET_TYPE_NR_LTE_GSM_WCDMA:
            raf = RAF_NR | RAF_LTE_GROUP | RAF_GSM_GROUP | RAF_WCDMA_GROUP;
            break;
        case PREF_NET_TYPE_NR_LTE_CDMA_EVDO_GSM_WCDMA:
            raf = RAF_NR | RAF_LTE_GROUP | RAF_CDMA_GROUP | RAF_EVDO_GROUP | RAF_GSM_GROUP | RAF_WCDMA_GROUP;
            break;
        case PREF_NET_TYPE_NR_LTE_WCDMA:
            raf = RAF_NR | RAF_LTE_GROUP | RAF_WCDMA_GROUP;
            break;
        case PREF_NET_TYPE_NR_LTE_TDSCDMA:
            raf = RAF_NR | RAF_LTE_GROUP | RAF_TD_SCDMA;
            break;
        case PREF_NET_TYPE_NR_LTE_TDSCDMA_GSM:
            raf = RAF_NR | RAF_LTE_GROUP | RAF_TD_SCDMA | RAF_GSM_GROUP;
            break;
        case PREF_NET_TYPE_NR_LTE_TDSCDMA_WCDMA:
            raf = RAF_NR | RAF_LTE_GROUP | RAF_TD_SCDMA | RAF_WCDMA_GROUP;
            break;
        case PREF_NET_TYPE_NR_LTE_TDSCDMA_GSM_WCDMA:
            raf = RAF_NR | RAF_LTE_GROUP | RAF_TD_SCDMA | RAF_GSM_GROUP | RAF_WCDMA_GROUP;
            break;
        case PREF_NET_TYPE_NR_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA:
            raf = RAF_NR | RAF_LTE_GROUP | RAF_TD_SCDMA | RAF_CDMA_GROUP | RAF_EVDO_GROUP | RAF_GSM_GROUP | RAF_WCDMA_GROUP;
            break;
        case PREF_NET_TYPE_LTE_GSM:
            raf = RAF_LTE_GROUP | RAF_GSM_GROUP;
            break;
        case PREF_NET_TYPE_LTE_TDD_ONLY:
            raf = RAF_LTE_GROUP;
            break;
        case PREF_NET_TYPE_CDMA_GSM:
            raf = RAF_CDMA_GROUP | RAF_GSM_GROUP;
            break;
        case PREF_NET_TYPE_CDMA_EVDO_GSM:
            raf = RAF_CDMA_GROUP | RAF_EVDO_GROUP | RAF_GSM_GROUP;
            break;
        case PREF_NET_TYPE_LTE_CDMA_EVDO_GSM:
            raf = RAF_LTE_GROUP | RAF_CDMA_GROUP | RAF_EVDO_GROUP | RAF_GSM_GROUP;
            break;
        default:
            raf = RAF_UNKNOWN;
            RFX_LOG_I(RFX_LOG_TAG, "getRafFromNetworkType, type=%d, raf=%d", type, raf);
            break;
    }
    return raf;
}

int RtcCapabilitySwitchUtil::getAdjustedRaf(int raf) {
    raf = ((RAF_GSM_GROUP & raf) > 0) ? (RAF_GSM_GROUP | raf) : raf;
    raf = ((RAF_WCDMA_GROUP & raf) > 0) ? (RAF_WCDMA_GROUP | raf) : raf;
    raf = ((RAF_CDMA_GROUP & raf) > 0) ? (RAF_CDMA_GROUP | raf) : raf;
    raf = ((RAF_EVDO_GROUP & raf) > 0) ? (RAF_EVDO_GROUP | raf) : raf;
    raf = ((RAF_LTE_GROUP & raf) > 0) ? (RAF_LTE_GROUP | raf) : raf;
    return raf;
}

int RtcCapabilitySwitchUtil::getNetworkTypeFromRaf(int raf) {
    int type;
    int adj_raf;
    if(RatConfig_isGsmSupported() == 0) {
        raf &= ~RAF_GSM_GROUP;
    }
    adj_raf = getAdjustedRaf(raf);
    switch (adj_raf) {
        case (RAF_GSM_GROUP | RAF_WCDMA_GROUP):
            type = PREF_NET_TYPE_GSM_WCDMA;
            break;
        case (RAF_GSM_GROUP):
            type = PREF_NET_TYPE_GSM_ONLY;
            break;
        case (RAF_WCDMA_GROUP):
            type = PREF_NET_TYPE_WCDMA;
            break;
        case (RAF_CDMA_GROUP | RAF_EVDO_GROUP):
            type = PREF_NET_TYPE_CDMA_EVDO_AUTO;
            break;
        case (RAF_CDMA_GROUP):
            type = PREF_NET_TYPE_CDMA_ONLY;
            break;
        case (RAF_EVDO_GROUP):
            type = PREF_NET_TYPE_EVDO_ONLY;
            break;
        case (RAF_GSM_GROUP | RAF_WCDMA_GROUP | RAF_CDMA_GROUP | RAF_EVDO_GROUP):
            type = PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO;
            break;
        case (RAF_LTE_GROUP | RAF_CDMA_GROUP | RAF_EVDO_GROUP):
            type = PREF_NET_TYPE_LTE_CDMA_EVDO;
            break;
        case (RAF_LTE_GROUP | RAF_GSM_GROUP | RAF_WCDMA_GROUP):
            type = PREF_NET_TYPE_LTE_GSM_WCDMA;
            break;
        case (RAF_LTE_GROUP | RAF_GSM_GROUP | RAF_WCDMA_GROUP | RAF_CDMA_GROUP | RAF_EVDO_GROUP):
            type = PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA;
            break;
        case (RAF_LTE_GROUP):
            type = PREF_NET_TYPE_LTE_ONLY;
            break;
        case (RAF_LTE_GROUP | RAF_WCDMA_GROUP):
            type = PREF_NET_TYPE_LTE_WCDMA;
            break;
        case (RAF_LTE_GROUP | RAF_GSM_GROUP):
            type = PREF_NET_TYPE_LTE_GSM;
            break;
        case (RAF_CDMA_GROUP | RAF_GSM_GROUP):
            type = PREF_NET_TYPE_CDMA_GSM;
            break;
        case (RAF_CDMA_GROUP | RAF_EVDO_GROUP | RAF_GSM_GROUP):
            type = PREF_NET_TYPE_CDMA_EVDO_GSM;
            break;
        case (RAF_LTE_GROUP | RAF_CDMA_GROUP | RAF_EVDO_GROUP | RAF_GSM_GROUP):
            type = PREF_NET_TYPE_LTE_CDMA_EVDO_GSM;
            break;
        case (RAF_NR):
            type = PREF_NET_TYPE_NR_ONLY;
            break;
        case (RAF_NR | RAF_LTE_GROUP):
            type = PREF_NET_TYPE_NR_LTE;
            break;
        case (RAF_NR | RAF_LTE_GROUP | RAF_CDMA_GROUP | RAF_EVDO_GROUP):
            type = PREF_NET_TYPE_NR_LTE_CDMA_EVDO;
            break;
        case (RAF_NR | RAF_LTE_GROUP | RAF_TD_SCDMA):
            type = PREF_NET_TYPE_NR_LTE_TDSCDMA;
            break;
        case (RAF_NR | RAF_LTE_GROUP | RAF_TD_SCDMA | RAF_GSM_GROUP):
            type = PREF_NET_TYPE_NR_LTE_TDSCDMA_GSM;
            break;
        case (RAF_NR | RAF_LTE_GROUP | RAF_TD_SCDMA | RAF_WCDMA_GROUP):
            type = PREF_NET_TYPE_NR_LTE_TDSCDMA_WCDMA;
            break;
        case (RAF_NR | RAF_LTE_GROUP | RAF_TD_SCDMA | RAF_GSM_GROUP | RAF_WCDMA_GROUP):
            type = PREF_NET_TYPE_NR_LTE_TDSCDMA_GSM_WCDMA;
            break;
        case (RAF_NR | RAF_LTE_GROUP | RAF_TD_SCDMA | RAF_CDMA_GROUP | RAF_EVDO_GROUP | RAF_GSM_GROUP | RAF_WCDMA_GROUP):
            type = PREF_NET_TYPE_NR_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA;
            break;
        default:
            int c2k_support = RatConfig_isC2kSupported();
            int lte_support = RatConfig_isLteFddSupported() | RatConfig_isLteTddSupported();
            type = c2k_support == 1 ?
                   (PREF_NET_TYPE_CDMA_EVDO_AUTO) :
                   (lte_support == 1 ?
                    PREF_NET_TYPE_LTE_GSM_WCDMA : PREF_NET_TYPE_GSM_WCDMA);
            RFX_LOG_I(RFX_LOG_TAG, "getNetworkTypeFromRaf, raf=%d, adj_raf=%d, type=%d", raf,
                    adj_raf, type);
            break;
    }
    return type;
}

bool RtcCapabilitySwitchUtil::isDisableC2kCapability() {
    char tmp[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get("persist.vendor.radio.disable_c2k_cap", tmp, "0");
    if (strcmp(tmp, "1") == 0) {
        RFX_LOG_I(RFX_LOG_TAG, "isDisableC2kCapability == true");
        return true;
    }
    return !RatConfig_isC2kSupported();
}

bool RtcCapabilitySwitchUtil::isDssNoResetSupport() {
    char tmp[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get("vendor.ril.simswitch.no_reset_support", tmp, "0");
    if (strcmp(tmp, "1") == 0) {
        return true;
    }
    return false;
}

bool RtcCapabilitySwitchUtil::isOp01Sim(const String8& imsi) {
    int table_size = sizeof(PLMN_TABLE_OP01) / sizeof(char*);
    for (int i = 0; i < table_size; i++) {
        if (imsi.find(PLMN_TABLE_OP01[i]) == 0) return true;
    }
    return false;
}

bool RtcCapabilitySwitchUtil::isOp02Sim(const String8& imsi) {
    int table_size = sizeof(PLMN_TABLE_OP02) / sizeof(char*);
    for (int i = 0; i < table_size; i++) {
        if (imsi.find(PLMN_TABLE_OP02[i]) == 0) return true;
    }
    return false;
}

bool RtcCapabilitySwitchUtil::isOp09Sim(const String8& imsi) {
    int table_size = sizeof(PLMN_TABLE_OP09) / sizeof(char*);
    for (int i = 0; i < table_size; i++) {
        if (imsi.find(PLMN_TABLE_OP09[i]) == 0) return true;
    }
    return false;
}

bool RtcCapabilitySwitchUtil::isOp18Sim(const String8& imsi) {
    int table_size = sizeof(PLMN_TABLE_OP18) / sizeof(char*);
    for (int i = 0; i < table_size; i++) {
        if (imsi.find(PLMN_TABLE_OP18[i]) == 0) return true;
    }
    return false;
}

bool RtcCapabilitySwitchUtil::isHvolteSupport() {
    char tempstr[RFX_PROPERTY_VALUE_MAX];
    memset(tempstr, 0, sizeof(tempstr));
    rfx_property_get("persist.vendor.mtk_ct_volte_support", tempstr, "0");
    if ((strcmp(tempstr, "3") == 0) || (strcmp(tempstr, "2") == 0)) {
        return true;
    } else {
        return false;
    }
}

bool RtcCapabilitySwitchUtil::isSimSwitchEnabled() {
    char property_value[RFX_PROPERTY_VALUE_MAX] = { 0 };

    //check if disabled from project config
    rfx_property_get("ro.vendor.mtk_disable_cap_switch", property_value, "-1");
    if (strcmp(property_value, "1") == 0) {
        RFX_LOG_I(RFX_LOG_TAG, "mtk_disable_cap_switch == 1");
        return false;
    }
    //check if FTA mode
    memset(property_value, 0, sizeof(property_value));
    rfx_property_get("vendor.gsm.gcf.testmode", property_value, "-1");
    if (strcmp(property_value, "2") == 0) {
        RFX_LOG_I(RFX_LOG_TAG, "testmode == 2");
        return false;
    }
    //check if disabled from engineer mode
    memset(property_value, 0, sizeof(property_value));
    rfx_property_get("persist.vendor.radio.simswitch.emmode", property_value, "-1");
    if (strcmp(property_value, "1") == 0) {
        RFX_LOG_I(RFX_LOG_TAG, "emmode == 1");
        return false;
    }
    return true;
}


