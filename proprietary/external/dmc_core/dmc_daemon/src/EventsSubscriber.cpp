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
 * MediaTek Inc. (C) 2019 All rights reserved.
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

#include "EventsSubscriber.h"
#include "dmc_utils.h"
#include <time.h>

EventsSubscriber::EventsSubscriber(const char *tag,
        KpiSubscriptionTable *kpiSubTable, ReqQManager *reqManager, TranslatorLoader *loader):
        mReqManager(NULL),
        mTranslatorLoader(NULL),
        mEnabled(false),
        mTranslatorConfig(NULL),
        m_pKpiDroppedCount(NULL),
        m_pKpiTotalCount(NULL) {
    mReqManager = reqManager;
    mTranslatorLoader = loader;
    mKpiSubTable = kpiSubTable;
    mTag = tag;

    DMC_LOGD(mTag, "constructor()");
}

bool EventsSubscriber::enableSubscriber() {
    if (!mTranslatorLoader->isInitialized()) {
        DMC_LOGE(mTag, "enableSubscriber() failed! loader not being initialized");
        return false;
    }
    if (!loadTranslatorConfig()) {
        DMC_LOGE(mTag, "enableSubscriber() failed! load config fail");
        return false;
    }
    mSubscribedMsgMap.clear();

    uint8_t kpiGroupSize = getKpiGroupSize();
    m_pKpiDroppedCount = new KPI_COUNT_UNIT[kpiGroupSize];
    memset(m_pKpiDroppedCount, 0, sizeof(KPI_COUNT_UNIT)*kpiGroupSize);
    m_pKpiTotalCount = new KPI_COUNT_UNIT[kpiGroupSize];
    memset(m_pKpiTotalCount, 0, sizeof(KPI_COUNT_UNIT)*kpiGroupSize);

    // Prepare alloc information buffer
    mReqManager->initEventInfoBuffer(kpiGroupSize);

    mEnabled = true;
    DMC_LOGD(mTag, "enableSubscriber() success!");
    return true;
}

void EventsSubscriber::disableSubscriber() {
    if (isSubscriberEnabled()) {
        unloadTranslatorConfig();

        if (m_pKpiDroppedCount != NULL) {
            delete m_pKpiDroppedCount;
            m_pKpiDroppedCount = NULL;
        }
        if (m_pKpiTotalCount != NULL) {
            delete m_pKpiTotalCount;
            m_pKpiTotalCount = NULL;
        }
        mReqManager->freeEventInfoBuffer();
        resetSubscribedMsgMap();
        mEnabled = false;
        DMC_LOGD(mTag, "disableSubscriber() success!");
    }
}

bool EventsSubscriber::isSubscriberEnabled() {
    if (mTranslatorConfig == NULL) {
        return false;
    }
    if (!mTranslatorLoader->isInitialized()) {
        return false;
    }
    return mEnabled;
}

bool EventsSubscriber::isMsgSubscribed(MSGID msgId) {
    std::map<MSGID, KPI_OBJ_TYPE>::iterator itor = mSubscribedMsgMap.find(msgId);
    return (itor == mSubscribedMsgMap.end())? false : true;
}

bool EventsSubscriber::isKpiSubscribed(KPI_OBJ_TYPE type) {
    return mKpiSubTable->isKpiSubscribed(type);
}

uint32_t EventsSubscriber::getSubscribedMsgSize() {
    return mSubscribedMsgMap.size();
}

void EventsSubscriber::getSubscribedMap(std::map<MSGID, KPI_OBJ_TYPE> &subscribedMap) {
    subscribedMap.clear();
    
    std::map<MSGID, KPI_OBJ_TYPE>::iterator itor;
    for (itor = mSubscribedMsgMap.begin(); itor != mSubscribedMsgMap.end(); itor++) {
        subscribedMap[(MSGID)itor->first] = (KPI_OBJ_TYPE)itor->second;
    }
}

void EventsSubscriber::getSubscribedMsgs(std::vector<MSGID> &subscribedMsgs) {
    subscribedMsgs.clear();
    std::map<MSGID, KPI_OBJ_TYPE>::iterator itor;
    for (itor = mSubscribedMsgMap.begin(); itor != mSubscribedMsgMap.end(); itor++) {
        subscribedMsgs.push_back((MSGID)itor->first);
    }
}

void EventsSubscriber::getSubscribedKpiObjs(std::vector<KPI_OBJ_TYPE> &subscribedObjs) {
    subscribedObjs.clear();
    std::map<MSGID, KPI_OBJ_TYPE>::iterator itor;
    for (itor = mSubscribedMsgMap.begin(); itor != mSubscribedMsgMap.end(); itor++) {
        subscribedObjs.push_back((KPI_OBJ_TYPE)itor->second);
    }
    // Remove duplicate entries
    std::vector<KPI_OBJ_TYPE>::iterator it;
    it = std::unique(subscribedObjs.begin(), subscribedObjs.end());
    subscribedObjs.resize( std::distance(subscribedObjs.begin(), it));
}

void EventsSubscriber::resetSubscribedMsgMap() {
    mSubscribedMsgMap.clear();
}


KPI_OBJ_TYPE EventsSubscriber::getObjTypeByMsgId(MSGID msgId) {
    KPI_OBJ_TYPE objType = KPI_OBJ_TYPE_UNDEFINED;
    std::map<MSGID, KPI_OBJ_TYPE>::iterator itor = mSubscribedMsgMap.find(msgId);
    if (itor != mSubscribedMsgMap.end()) {
        objType = (KPI_OBJ_TYPE)itor->second;
    }
    return objType;
}

DMC_RESULT_CODE EventsSubscriber::SubscribeHandler(
        bool bSubscribe,
        KPI_OBJ_TYPE kpiObjType,
        KPI_SOURCE_TYPE source,
        MSGID msgId) {
    std::map<MSGID, KPI_OBJ_TYPE>::iterator itor = mSubscribedMsgMap.find(msgId);
    bool bFound = (itor == mSubscribedMsgMap.end())? false : true;
    DMC_RESULT_CODE ret = DMC_RESULT_SKIPPED;

    if (bSubscribe) {
        if (!bFound) {
            mSubscribedMsgMap[msgId] = kpiObjType;
            ret = DMC_RESULT_SUCCESS;
        }
    } else {
        if (bFound) {
            mSubscribedMsgMap.erase(itor);
            ret = DMC_RESULT_SUCCESS;
        }
    }

    DMC_LOGD(mTag, "SubscribeHandler() bSubscribe = %d, type = %d, source = %s, msgId = %" PRIu64", ret = %d",
                bSubscribe, kpiObjType, KpiUtils::kpiSource2String(source), msgId, ret);

    return ret;
}

bool EventsSubscriber::loadTranslatorConfig() {
    if (!mTranslatorLoader->isInitialized()) {
        return false;
    }
    mTranslatorConfig = mTranslatorLoader->getTranslatorConfig();

    if (mTranslatorConfig == NULL) {
        DMC_LOGE(mTag, "NULL TranslatorConfig!!!");
        return false;
    }
    DMC_LOGD(mTag, "loadTranslatorConfig() success");
    mTranslatorConfig->dump();

    return true;
}

void EventsSubscriber::unloadTranslatorConfig() {
    mTranslatorConfig = NULL;
    DMC_LOGD(mTag, "unloadTranslatorConfig() success");
}

const char *EventsSubscriber::getTranslatorVersion() {
    if (mTranslatorConfig == NULL) {
        DMC_LOGE(mTag, "Shall NOT call getTranslatorVersion()!");
        return "no version";
    }
    return mTranslatorConfig->mVerison;
}

uint16_t EventsSubscriber::getKpiSize() {
    if (mTranslatorConfig == NULL) {
        DMC_LOGE(mTag, "Shall NOT call getKpiSize()!");
        // Default 0 KPIs
        return 0;
    }
    return mTranslatorConfig->mKpiSize;
}

uint8_t EventsSubscriber::getKpiGroupSize() {
    if (mTranslatorConfig == NULL) {
        DMC_LOGE(mTag, "Shall NOT call getKpiGroupSize()!");
        // Default 1 group
        return 1;
    }
    return mTranslatorConfig->mKpiGroupSize;
}

uint64_t EventsSubscriber::getKpiTimestampDelta() {
    if (mTranslatorConfig == NULL) {
        DMC_LOGE(mTag, "Shall NOT call getKpiTimestampDelta()!");
        // Default 0
        return 0;
    }
    return mTranslatorConfig->mKpiTimestampDeltaEpochUs;
}

int32_t EventsSubscriber::getPacketMonitorParameter() {
    if (mTranslatorConfig == NULL) {
        DMC_LOGE(mTag, "Shall NOT call getPacketParameter()!");
        // Default 0
        return 0;
    }
    return mTranslatorConfig->mPkmParameter;
}


const translator_kpi_range *EventsSubscriber::getKpiTypeRange(KPI_SOURCE_TYPE source) {
    if (mTranslatorConfig == NULL) {
        DMC_LOGE(mTag, "Shall NOT call getKpiTypeRange()!");
        return NULL;
    }

    switch (source) {
        case KPI_SOURCE_TYPE_LOCAL:
        case KPI_SOURCE_TYPE_APM:
        case KPI_SOURCE_TYPE_PKM:
        case KPI_SOURCE_TYPE_MDM:
            return &(mTranslatorConfig->mKpiRange[source]);
            break;
        default:
            DMC_LOGE(mTag, "getKpiTypeRange() Unexpected source %s", KpiUtils::kpiSource2String(source));
    }
    return NULL;
}

KPI_GID EventsSubscriber::getKpiGroupId(KPI_OBJ_TYPE kpiObjType) {
    if (!isSubscriberEnabled()) {
        DMC_LOGE(mTag, "Shall NOT call getKpiGroupId()!");
        return 0;
    }
    return mTranslatorLoader->getKpiGroupId(kpiObjType);
}

const TrapTable *EventsSubscriber::getTrapTableByKpi(KPI_OBJ_TYPE kpiObjType) {
    if (!isSubscriberEnabled()) {
        DMC_LOGE(mTag, "Shall NOT call getTrapTableByKpi()!");
        return NULL;
    }
    return mTranslatorLoader->getTrapTableByKpi(kpiObjType);
}

KPI_OBJ_RESULT_CODE EventsSubscriber::translateKpi(
        KPI_OBJ_TYPE kpiType,
        KPI_MSGID msgId,
        KPI_SIMID simId,
        uint64_t timestampUs,
        KPI_SOURCE_TYPE source,
        KpiObj &outObj,
        uint64_t datalen,
        const void *data,
        const void *param) {
    if (!isSubscriberEnabled()) {
        DMC_LOGE(mTag, "Shall NOT call translateKpi()!");
        return KPI_OBJ_RESULT_CODE_FAIL;
    }
    return mTranslatorLoader->translateKpi(
            kpiType, msgId, simId, timestampUs, source, outObj, datalen, data, param);
}

void EventsSubscriber::incKpiDroppedCount(KPI_GID groupId, KPI_COUNT_UNIT count) {
    if (m_pKpiDroppedCount != NULL) {
        if ((UINT32_MAX - count) > m_pKpiDroppedCount[groupId]) {
            m_pKpiDroppedCount[groupId] += count;
        } else {
            DMC_LOGE(mTag, "KPI dropped count of group %d overflow, reset!", groupId);
            m_pKpiDroppedCount[groupId] = count;
        }
    }
}

void EventsSubscriber::incKpiTotalCount(KPI_GID groupId, KPI_COUNT_UNIT count) {
    if (m_pKpiTotalCount != NULL) {
        if ((UINT32_MAX - count) > m_pKpiTotalCount[groupId]) {
            m_pKpiTotalCount[groupId] += count;
        } else {
            DMC_LOGE(mTag, "KPI total count of group %d overflow, reset!", groupId);
            m_pKpiTotalCount[groupId] = count;
        }
    }
}

bool EventsSubscriber::getEventsInfo(KPI_GROUP_TYPE groupType, EventsInfo &eventsInfo) {
    if (!isSubscriberEnabled()) {
        DMC_LOGE(mTag, "Shall NOT call getEventsInfo(%d)!", groupType);
        return false;
    }

    if (groupType > KPI_GROUP_TYPE_UNDEFINED && groupType < getKpiGroupSize()) {
        eventsInfo.totalEvents = m_pKpiTotalCount[groupType];
        eventsInfo.droppedEvents = m_pKpiDroppedCount[groupType];
    }
    DMC_LOGD(mTag, "getEventsInfo(%d) total = %" PRIu32", dropped = %" PRIu32"",
            groupType, eventsInfo.totalEvents, eventsInfo.droppedEvents);
    return true;
}

void EventsSubscriber::resetKpiCount() {
    uint8_t kpiGroupSize = getKpiGroupSize();
    if (m_pKpiDroppedCount != NULL) {
        memset(m_pKpiDroppedCount, 0, sizeof(KPI_COUNT_UNIT)*kpiGroupSize);
    }
    if (m_pKpiTotalCount != NULL) {
        memset(m_pKpiTotalCount, 0, sizeof(KPI_COUNT_UNIT)*kpiGroupSize);
    }
}

uint64_t EventsSubscriber::getCurrentTimestampUs() {
    struct timespec res;
    clock_gettime(CLOCK_REALTIME, &res);
    uint64_t current = 1000000.0*res.tv_sec + (double)res.tv_nsec/1e3;
    return current;
}

