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
#include "RpIrInvalidMccConvert.h"
#include "RpIrMccConvertMappingTables.h"
#include "RfxLog.h"
#include "cutils/properties.h"
#include "stdlib.h"
#include "string.h"

namespace android {
extern int property_get(const char *key, char *value, const char *default_value);
}
const char *RpIrInvalidMccConvert::TAG = "[RpIrInvalidMccConvert]";
/**
 * Check mcc by sid ltm off.
 * @param mccMnc the MCCMNC
 * @return the MCCMNC
 */
String8 RpIrInvalidMccConvert::checkMccBySidLtmOff(String8 mccMnc) {
    RFX_LOG_D(TAG, "[checkMccBySidLtmOff] mccMnc = : %s", mccMnc.string());

    char strSid[PROPERTY_VALUE_MAX] = {0};
    char strLtmOff[PROPERTY_VALUE_MAX] = {0};
    char strdef[2] = "";

    memset(strSid, 0, sizeof(char)*PROPERTY_VALUE_MAX);
    property_get("vendor.cdma.operator.sid", strSid, strdef);

    memset(strLtmOff, 0, sizeof(char)*PROPERTY_VALUE_MAX);
    property_get("vendor.cdma.operator.ltmoffset", strLtmOff, strdef);

    RFX_LOG_D(TAG, "[checkMccBySidLtmOff] Sid = %s, Ltm_off = %s", strSid, strLtmOff);

    String8 strMcc = getMccFromConflictTableBySidLtmOff(strSid, strLtmOff);
    String8 tempMcc;
    String8 strMccMnc;

    RFX_LOG_D(TAG, "[checkMccBySidLtmOff] MccFromConflictTable = %s", strMcc.string());

    if (!strMcc.isEmpty()) {
        tempMcc = strMcc;
    } else {
        strMcc = getMccFromMINSTableBySid(strSid);
        RFX_LOG_D(TAG, "[checkMccBySidLtmOff] MccFromMINSTable = %s", strMcc.string());
        if (!strMcc.isEmpty()) {
            tempMcc = strMcc;
        } else {
            tempMcc = mccMnc;
        }
    }

    RFX_LOG_D(TAG, "[checkMccBySidLtmOff] tempMcc = %s", tempMcc.string());

    int mcc = atoi(string(tempMcc.string()).substr(0, 3).c_str());
    if (mcc == 310 || mcc == 311 || mcc == 312) {
        strMccMnc = getMccMncFromSidMccMncListBySid(strSid);
        RFX_LOG_D(TAG, "[checkMccBySidLtmOff] MccMnc = %s", strMccMnc.string());
        if (!strMccMnc.isEmpty()) {
            tempMcc = strMccMnc;
        }
    }

    return tempMcc;
}

/**
 * Get mcc from conflict table by sid ltm off.
 * @param sSid the SID
 * @param sLtmOff the LTM off
 * @return MCC
 */
String8 RpIrInvalidMccConvert::getMccFromConflictTableBySidLtmOff(const char *sSid, const char *sLtmOff) {
    RFX_LOG_D(TAG, "[getMccFromConflictTableBySidLtmOff] sSid = %s, sLtm_off = %s", sSid, sLtmOff);
    if (sSid == NULL || strlen(sSid) <= 0 || strlen(sSid) > 5
            || sLtmOff == NULL || strlen(sLtmOff) <= 0) {
        RFX_LOG_D(TAG, "[getMccFromConflictTableBySidLtmOff] please check the param ");
        return String8("");
    }

    int sid;

    sid = atoi(sSid);
    if (sid < 0) {
        return String8("");
    }

    int ltmoff;
    ltmoff = atoi(sLtmOff);

    RFX_LOG_D(TAG, " [getMccFromConflictTableBySidLtmOff] sid=%d, ltmoff=%d", sid, ltmoff);
    RFX_LOG_D(TAG, " [getMccFromConflictTableBySidLtmOff] mccSidMapSize = %d",
            RpIrMccConvertMappingTables::MCC_SID_LTM_OFF_MAP_SIZE);

    RpIrMccSidLtmOff const *pRpIrMccSidLtmOff = NULL;

    for (int i = 0; i < RpIrMccConvertMappingTables::MCC_SID_LTM_OFF_MAP_SIZE; i++) {
        pRpIrMccSidLtmOff = &RpIrMccConvertMappingTables::MCC_SID_LTM_OFF_MAP[i];
        int max = (pRpIrMccSidLtmOff->mLtmOffMax) * PARAM_FOR_OFFSET;
        int min = (pRpIrMccSidLtmOff->mLtmOffMin) * PARAM_FOR_OFFSET;

        RFX_LOG_D(TAG, "[getMccFromConflictTableBySidLtmOff] pRpIrMccSidLtmOff[i].mSid = %d, \
                sid = %d, i=%d, ltm_off = %d, max = %d, min = %d",
                pRpIrMccSidLtmOff[i].mSid, sid, i, ltmoff, max, min);

        if (pRpIrMccSidLtmOff->mSid == sid && (ltmoff <= max && ltmoff >= min)) {
            char retMcc[33];
            sprintf(retMcc, "%d", pRpIrMccSidLtmOff->mMcc);
            RFX_LOG_D(TAG, " [getMccFromConflictTableBySidLtmOff] Mcc = %s", retMcc);
            return String8(retMcc);
        }
    }

    return String8("");
}

/**
 * Get mcc from MINS table by sid.
 * @param sSid the SID
 * @return MCC
 */
String8 RpIrInvalidMccConvert::getMccFromMINSTableBySid(const char *sSid) {
    RFX_LOG_D(TAG, " [getMccFromMINSTableBySid] sid = %s", sSid);
    if (sSid == NULL || strlen(sSid) <= 0 || strlen(sSid) > 5) {
        RFX_LOG_D(TAG, "[getMccFromMINSTableBySid] please check the param ");
        return String8("");
    }

    int sid;
    sid = atoi(sSid);
    if (sid < 0) {
        return String8("");
    }

    RFX_LOG_D(TAG, "[getMccFromMINSTableBySid] size = %d", RpIrMccConvertMappingTables::MCC_IDD_NDD_SID_MAP_SIZE);

    RpIrMccIddNddSid const *pRpIrMccIddNddSid = NULL;
    for (int i = 0; i < RpIrMccConvertMappingTables::MCC_IDD_NDD_SID_MAP_SIZE; i++) {
        pRpIrMccIddNddSid = &RpIrMccConvertMappingTables::MCC_IDD_NDD_SID_MAP[i];
        RFX_LOG_D(TAG, "[getMccFromMINSTableBySid] sid = %d, i=%d, \
                pRpIrMccIddNddSid[i].SidMin = %d, pRpIrMccIddNddSid[i].SidMax = %d",
                sid, i, pRpIrMccIddNddSid[i].mSidMin, pRpIrMccIddNddSid[i].mSidMax);

        if (sid >= pRpIrMccIddNddSid->mSidMin && sid <= pRpIrMccIddNddSid->mSidMax) {
            char retMcc[33];
            sprintf(retMcc, "%d", pRpIrMccIddNddSid->mMcc);
            RFX_LOG_D(TAG, " [queryMccFromConflictTableBySid] Mcc = %s", retMcc);
            return String8(retMcc);
        }
    }

    return String8("");
}

/**
 * Get mccmnc from SidMccMncList by sid.
 * @param sSid the SID
 * @return MCCMNC
 */
String8 RpIrInvalidMccConvert::getMccMncFromSidMccMncListBySid(const char * sSid) {
    RFX_LOG_D(TAG, " [getMccMncFromSidMccMncListBySid] sid = %s", sSid);
    if (sSid == NULL || strlen(sSid) <= 0 || strlen(sSid) > 5) {
        RFX_LOG_D(TAG, "[getMccMncFromSidMccMncListBySid] please check the param ");
        return String8("");
    }

    int sid;
    sid = atoi(sSid);
    if (sid < 0) {
        return String8("");
    }
    RpIrSidMccMnc const *pRpIrSidMccMnc = NULL;

    int left = 0;
    int right = RpIrMccConvertMappingTables::SID_MCC_MNC_LIST_SIZE - 1;
    int mid;
    int mccMnc = 0;

    while (left <= right) {
        mid = (left + right) / 2;
        pRpIrSidMccMnc = &RpIrMccConvertMappingTables::SID_MCC_MNC_LIST[mid];
        if (sid < pRpIrSidMccMnc->mSid) {
            right = mid - 1;
        } else if (sid > pRpIrSidMccMnc->mSid) {
            left = mid + 1;
        } else {
            mccMnc = pRpIrSidMccMnc->mMccMnc;
            break;
        }
    }

    if (mccMnc != 0) {
        char retMcc[33];
        sprintf(retMcc, "%d", mccMnc);

        RFX_LOG_D(TAG, "[getMccMncFromSidMccMncListBySid] retMcc = %s", retMcc);
        return String8(retMcc);
    } else {
        return String8("");
    }
}
