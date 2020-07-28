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

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include "LocalEventsSubscriber.h"
#include "dmc_utils.h"

#undef TAG
#define TAG "LCM-Subscriber"

LocalEventsSubscriber::LocalEventsSubscriber(
        KpiSubscriptionTable *kpiSubTable, ReqQManager *reqManager, TranslatorLoader *loader):
        EventsSubscriber(TAG, kpiSubTable, reqManager, loader)
{
}

bool LocalEventsSubscriber::enableSubscriber() {
    if (isSubscriberEnabled()) {
        DMC_LOGW(TAG, "enableSubscriber() skipped, already enabled");
        return true;
    }
    return EventsSubscriber::enableSubscriber();
}

void LocalEventsSubscriber::disableSubscriber() {
    if (isSubscriberEnabled()) {
        EventsSubscriber::disableSubscriber();
    }
}

DMC_RESULT_CODE LocalEventsSubscriber::SubscribeHandler(
        bool bSubscribe,
        KPI_OBJ_TYPE kpiObjType,
        KPI_SOURCE_TYPE source,
        MSGID msgId) {

    if (!isSubscriberEnabled()) {
        DMC_LOGE(TAG, "SubscribeHandler failed, subscriber not enabled!");
        return DMC_RESULT_FAIL;
    }

    // Call parent to update mSubscribedMsgMap.
    EventsSubscriber::SubscribeHandler(bSubscribe, kpiObjType, source, msgId);

    // Local events are subscription driven, trigger translate directly!
    if (bSubscribe) {
        handleKpi(kpiObjType, source, msgId);
    }
    return DMC_RESULT_SUCCESS;
}

DMC_RESULT_CODE LocalEventsSubscriber::handleKpi(
        KPI_OBJ_TYPE kpiType, KPI_SOURCE_TYPE source, MSGID msgId) {
    KPI_SIMID simId = 0;
    KPI_TIMESTAMP timestampUs = getCurrentTimestampUs();

    int32_t beginIdx = 0;
    int32_t endIdx = 0;
    const translator_kpi_range *pKpiRange = getKpiTypeRange(KPI_SOURCE_TYPE_LOCAL);
    if (pKpiRange != NULL) {
        beginIdx = pKpiRange->beginIdx;
        endIdx = pKpiRange->endIdx;
    }
    DMC_LOGD(TAG, "[%d]Got Local event. msgId = [%" PRIu64"], objType = %d", simId, msgId, kpiType);

    if (!isSubscriberEnabled()) {
        DMC_LOGE(TAG, "Skip event in disabled state");
        return DMC_RESULT_SKIPPED;
    }

    // Ignore trap message not being subscribed
    if (!isMsgSubscribed(msgId)) {
        DMC_LOGE(TAG, "Not subscribe msgId = [%" PRIu64"]", msgId);
        return DMC_RESULT_SKIPPED;
    }

    //DMC_LOGD(TAG, "beginIdx=%" PRId32", endIdx=%" PRId32"", beginIdx, endIdx);

    if (kpiType < beginIdx || kpiType > endIdx) {
        DMC_LOGE(TAG, "Invalid objType = %d", kpiType);
        return DMC_RESULT_FAIL;
    }

    const TrapTable *trapTable = getTrapTableByKpi(kpiType);
    if (NULL == trapTable) {
        DMC_LOGE(TAG, "Can't find trap table");
        return DMC_RESULT_FAIL;
    }

    if (NULL == trapTable->GetUsedTrapByID(source, msgId)) {
        DMC_LOGE(TAG, "GetUsedTrapByID() false");
        return DMC_RESULT_FAIL;
    }

    uint64_t delta = getKpiTimestampDelta();
    if (timestampUs > delta) {
        timestampUs -= delta;
    }

    KpiObj kpiObj(kpiType);
    // No need to pass data
    void *data = NULL;
    KPI_OBJ_RESULT_CODE ret = EventsSubscriber::translateKpi(
            kpiType, (KPI_MSGID)msgId, (KPI_SIMID)simId, timestampUs,
            source, kpiObj, 0, data, NULL);

    KPI_GID groupType = getKpiGroupId((KPI_OBJ_TYPE)kpiType);
    if (KPI_GROUP_TYPE_UNDEFINED == groupType) {
        DMC_LOGE(TAG, "Invalid groupId with type = %d, assigned to 0", kpiType);
        groupType = 0;
    }

    switch(ret) {
        case KPI_OBJ_RESULT_CODE_SUCESS:
            incKpiTotalCount(groupType, 1);
            break;
        case KPI_OBJ_RESULT_CODE_FAIL:
            incKpiTotalCount(groupType, 1);
            incKpiDroppedCount(groupType, 1);
            DMC_LOGE(TAG, "Drop event for KPI %d", kpiType);
            return DMC_RESULT_FAIL;
        case KPI_OBJ_RESULT_CODE_SKIPPED:
        case KPI_OBJ_RESULT_CODE_UNDEFIEND:
        default:
            // Do nothing.
            DMC_LOGE(TAG, "Drop event for KPI %d, ret = %d", kpiType, ret);
            return DMC_RESULT_FAIL;
    }

    size_t dataSize = 0;
    char *ptr = (char *)kpiObj.GetByteArray(dataSize);
    // Send KPI to queue and then dispatch to DM APP
    mReqManager->EnqueueRequest(kpiObj.GetType(), groupType, ptr, dataSize, timestampUs);

    DMC_LOGD(TAG, "[%d]translateKpi(%d) done. msgId=[%" PRIu64"], dataSize=%d, time=%" PRIu64", ret=%d",
            simId, kpiType, msgId, dataSize, timestampUs, ret);

    return DMC_RESULT_SUCCESS;
}


