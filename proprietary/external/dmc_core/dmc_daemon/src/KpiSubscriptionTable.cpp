
/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include "KpiSubscriptionTable.h"
#include "dmc_utils.h"

#undef TAG
#define TAG "DMC-KpiSubscriptionTable"

void KpiSubscriptionTable::regist(SubscriptionInfo *pInfo) {
    std::map<KPI_OBJ_TYPE, SubscriptionInfo *>::iterator it;
    if ((it = mSubInfoMap.find(pInfo->type)) == mSubInfoMap.end()) {
        mSubInfoMap.insert(make_pair(pInfo->type, pInfo));
    }
}

void KpiSubscriptionTable::unregist(KPI_OBJ_TYPE type) {
    std::map<KPI_OBJ_TYPE, SubscriptionInfo *>::iterator it;
    if ((it = mSubInfoMap.find(type)) != mSubInfoMap.end()) {
        delete it->second;
        mSubInfoMap.erase(it);
    }
}

bool KpiSubscriptionTable::isKpiSubscribed(KPI_OBJ_TYPE type) {
    std::map<KPI_OBJ_TYPE, SubscriptionInfo *>::iterator it;
    if ((it = mSubInfoMap.find(type)) != mSubInfoMap.end()) {
        return true;
    }
    return false;
}

const SubscriptionInfo *KpiSubscriptionTable::findSubscriptionInfo(KPI_OBJ_TYPE type) {
    std::map<KPI_OBJ_TYPE, SubscriptionInfo *>::iterator it;
    if ((it = mSubInfoMap.find(type)) != mSubInfoMap.end()) {
        return (SubscriptionInfo *) it->second;
    }
    return NULL;
}

void KpiSubscriptionTable::reset() {
    mSubInfoMap.clear();
}

void KpiSubscriptionTable::dump() {
    std::map<KPI_OBJ_TYPE, SubscriptionInfo *>::iterator it;
    if (mSubInfoMap.empty()) {
        DMC_LOGD(TAG, "KpiSubscriptionTable empty!");
        return;
    }

    DMC_LOGD(TAG, "KpiSubscriptionTable: dump registed KPI list[type, oid]");
    for (it = mSubInfoMap.begin(); it != mSubInfoMap.end(); ++it) {
        SubscriptionInfo *pInfo = it->second;
        DMC_LOGD(TAG, "[%d, %s]", pInfo->type, pInfo->oid.getOidString());
    }
}
