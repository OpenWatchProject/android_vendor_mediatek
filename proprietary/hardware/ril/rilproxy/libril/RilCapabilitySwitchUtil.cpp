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
#include <string.h>
#include "RilCapabilitySwitchUtil.h"
#include <telephony/mtk_ril.h>
#include "ratconfig.h"
#include <cutils/properties.h>
#include <stdlib.h>

#define LOG_TAG "RilCapa"

/*****************************************************************************
 * Class RilCapabilitySwitchUtil
 *****************************************************************************/

int RilCapabilitySwitchUtil::getRafFromNetworkType(int type) {
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
            RLOGD("getRafFromNetworkType, type=%d, raf=%d", type, raf);
            break;
    }
    return raf;
}

int RilCapabilitySwitchUtil::getAdjustedRaf(int raf) {
    raf = ((RAF_GSM_GROUP & raf) > 0) ? (RAF_GSM_GROUP | raf) : raf;
    raf = ((RAF_WCDMA_GROUP & raf) > 0) ? (RAF_WCDMA_GROUP | raf) : raf;
    raf = ((RAF_CDMA_GROUP & raf) > 0) ? (RAF_CDMA_GROUP | raf) : raf;
    raf = ((RAF_EVDO_GROUP & raf) > 0) ? (RAF_EVDO_GROUP | raf) : raf;
    raf = ((RAF_LTE_GROUP & raf) > 0) ? (RAF_LTE_GROUP | raf) : raf;
    return raf;
}

int RilCapabilitySwitchUtil::getNetworkTypeFromRaf(int raf) {
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
        default:
            int c2k_support = RatConfig_isC2kSupported();
            int lte_support = RatConfig_isLteFddSupported() | RatConfig_isLteTddSupported();
            type = c2k_support == 1 ? (PREF_NET_TYPE_CDMA_EVDO_AUTO) : (lte_support == 1 ?
                    PREF_NET_TYPE_LTE_GSM_WCDMA : PREF_NET_TYPE_GSM_WCDMA);
            RLOGD("getNetworkTypeFromRaf, raf=%d, adj_raf=%d, type=%d", raf, adj_raf, type);
            break;
    }
    return type;
}
