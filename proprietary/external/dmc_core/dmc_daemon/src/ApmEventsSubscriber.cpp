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


#include "ApmEventsSubscriber.h"
#include "dmc_utils.h"
#include <sys/system_properties.h>


#undef TAG
#define TAG "APM-Subscriber"

ApmEventsSubscriber::ApmEventsSubscriber(
        KpiSubscriptionTable *kpiSubTable, ReqQManager *reqManager, TranslatorLoader *loader):
        EventsSubscriber(TAG, kpiSubTable, reqManager, loader), m_pApmService(nullptr) {
    // TODO, start service when >= 1 AP KPI being subscribed
    m_pApmService = new ApmService(this);
    m_pApmService->registApmHidlServiceAsyc();
    updateApmSubscriptionProperty(false, "constructor");
}

ApmEventsSubscriber::~ApmEventsSubscriber() {
    if (m_pApmService != nullptr) {
        m_pApmService = nullptr;
    }
}

bool ApmEventsSubscriber::enableSubscriber() {
    if (isSubscriberEnabled()) {
        DMC_LOGW(TAG, "enableSubscriber() skipped, already enabled");
        return true;
    }
    return EventsSubscriber::enableSubscriber();
}

void ApmEventsSubscriber::disableSubscriber() {
    if (isSubscriberEnabled()) {
        updateApmSubscriptionProperty(false, "subscriber disabled");

        // Disable and unsubscribe all messages
        EventsSubscriber::disableSubscriber();
        // Notify APM clients to clear subscribed messaged.
        std::vector<APM_MSGID> subscribedMsgs;
        getSubscribedMsgs(subscribedMsgs);
        m_pApmService->updateSubscriptionState(subscribedMsgs);

        // Stop PKM service
        stopPkmService();
    }
}

DMC_RESULT_CODE ApmEventsSubscriber::SubscribeHandler(
        bool bSubscribe,
        KPI_OBJ_TYPE kpiObjType,
        KPI_SOURCE_TYPE source,
        MSGID msgId) {

    if (!isSubscriberEnabled()) {
        DMC_LOGE(TAG, "SubscribeHandler failed, subscriber not enabled!");
        return DMC_RESULT_FAIL;
    }
    if (!m_pApmService->isServiceRegisted()) {
        DMC_LOGE(TAG, "SubscribeHandler failed, APM service not ready!");
        return DMC_RESULT_FAIL;
    }

    bool bStopPkmService = false;
    if (bSubscribe) {
        if (getSubscribedMsgSize() == 0) {
            updateApmSubscriptionProperty(true, "first APM msg");
            DMC_LOGD(TAG, "Subscribe first APM msg");
        }

        if (isPkmKpiType(kpiObjType)) {
            if (getPkmSubscribedMsgSize() == 0) {
                // Start PKM service
                DMC_LOGD(TAG, "Subscribe first PKM msg, start PKM service");
                if (!startPkmServiceSync()) {
                    DMC_LOGE(TAG, "Start PKM service failed!");
                    return DMC_RESULT_FAIL;
                }
            }
        }
    } else {
        // Error handling
        if (getSubscribedMsgSize() == 0) {
            DMC_LOGW(TAG, "Nothing to unsubscribe.");
            return DMC_RESULT_FAIL;
        }
        // Unsubscribe the lastest msg, stop the PKM service
        if (getSubscribedMsgSize() == 1) {
            updateApmSubscriptionProperty(false, "final APM msg");
            DMC_LOGD(TAG, "Unsubscribe the last APM msg");
        }
        if (isPkmKpiType(kpiObjType)) {
            if (getPkmSubscribedMsgSize() == 1) {
                // Stop PKM service after updateSubscriptionState() call
                // Because PKM is also an APM client
                bStopPkmService = true;
            }
        }
    }

    // Call parent to update mSubscribedMsgMap if executed subscription request.
    EventsSubscriber::SubscribeHandler(bSubscribe, kpiObjType, source, msgId);

    // Notify APM client subscription status updated
    std::vector<APM_MSGID> subscribedMsgs;
    getSubscribedMsgs(subscribedMsgs);
    m_pApmService->updateSubscriptionState(subscribedMsgs);

    if (bStopPkmService) {
        DMC_LOGD(TAG, "Unsubscribe the last PKM msg, stop PKM service");
        stopPkmService();
    }

    return DMC_RESULT_SUCCESS;
}

int64_t ApmEventsSubscriber::getApmSessionParam() {
    return (int64_t)getPacketMonitorParameter();
}

// Interface of IApMonitor
DMC_RESULT_CODE ApmEventsSubscriber::handleKpi(
        KpiHeader header, uint32_t len, const void *data) {
    MSGID msgId = header.msgId;
    KPI_SIMID simId = header.simId;
    KPI_TIMESTAMP timestampUs = header.timestampMs*1000;
    KPI_SOURCE_TYPE source = KPI_SOURCE_TYPE_APM;

    uint8_t numKpiTranslated = 0;
    int32_t beginIdx = KPI_TYPE_BEGIN;
    int32_t endIdx = getKpiSize();
    /*
    const translator_kpi_range *pKpiRange = getKpiTypeRange(KPI_SOURCE_TYPE_APM);
    if (pKpiRange != NULL) {
        beginIdx = pKpiRange->beginIdx;
        endIdx = pKpiRange->endIdx;
    }
    */
    DMC_LOGD(TAG, "[%d]Got APM event. msgId = [%" PRIu64"], len = %d",
            simId, msgId, len);

    if (!isSubscriberEnabled()) {
        DMC_LOGE(TAG, "Skip event in disabled state");
        return DMC_RESULT_SKIPPED;
    }

    // Ignore trap message not being subscribed
    if (!isMsgSubscribed(msgId)) {
        DMC_LOGE(TAG, "Not subscribe msgId = [%" PRIu64"]", msgId);
        return DMC_RESULT_SKIPPED;
    }

    // DMC_LOGD(TAG, "beginIdx=%" PRId32", endIdx=%" PRId32"", beginIdx, endIdx);
    if (timestampUs == 0) {
        timestampUs = getCurrentTimestampUs();
    }
    uint64_t delta = getKpiTimestampDelta();
    if (timestampUs > delta) {
        timestampUs -= delta;
    }

    for (int32_t kpiType = beginIdx; kpiType <= endIdx; kpiType++) {
        // Skip KPI_OBJ_TYPE type not being subscribed.
        if (!isKpiSubscribed(kpiType)) {
            continue;
        }
        const TrapTable *trapTable = getTrapTableByKpi((KPI_OBJ_TYPE)kpiType);
        if (NULL == trapTable) {
            continue;
        }
        if (NULL == trapTable->GetUsedTrapByID(source, msgId)) {
            continue;
        }

        KpiObj kpiObj(kpiType);
        KPI_OBJ_RESULT_CODE ret = EventsSubscriber::translateKpi(
                kpiType, (KPI_MSGID)msgId, (KPI_SIMID)simId, timestampUs,
                source, kpiObj, len, data, NULL);

        KPI_GID groupType = getKpiGroupId((KPI_OBJ_TYPE)kpiType);
        if (KPI_GROUP_TYPE_UNDEFINED == groupType) {
            DMC_LOGE(TAG, "Invalid groupId with type = %d, assigned to 0", kpiType);
            groupType = 0;
        }

        switch(ret) {
            case KPI_OBJ_RESULT_CODE_SUCESS:
                numKpiTranslated++;
                incKpiTotalCount(groupType, 1);
                break;
            case KPI_OBJ_RESULT_CODE_FAIL:
                incKpiTotalCount(groupType, 1);
                incKpiDroppedCount(groupType, 1);
                DMC_LOGE(TAG, "Drop event for KPI %d", kpiType);
                continue;
            case KPI_OBJ_RESULT_CODE_SKIPPED:
            case KPI_OBJ_RESULT_CODE_UNDEFIEND:
            default:
                // Do nothing.
                continue;
        }

        size_t dataSize = 0;
        char *ptr = (char *)kpiObj.GetByteArray(dataSize);
        // Send KPI to queue and then dispatch to DM APP
        mReqManager->EnqueueRequest(kpiObj.GetType(), groupType, ptr, dataSize, timestampUs);

        DMC_LOGD(TAG, "[%d]translateKpi(%d) done. msgId=[%" PRIu64"], dataSize=%d, time=%" PRIu64", ret=%d",
                simId, kpiType, msgId, dataSize, timestampUs, ret);
    }
    if (numKpiTranslated > 0) {
        return DMC_RESULT_SUCCESS;
    }
    return DMC_RESULT_FAIL;
}

bool ApmEventsSubscriber::startPkmServiceSync() {
    int retry = -1;
    for (retry = -1; retry < 3; retry++) {
        if (!isPkmServiceRunning()) {
            property_set("ctl.start", "pkm_service");
            DMC_LOGD(TAG, "Start PKM service");
            sleep(1);
        } else {
            DMC_LOGD(TAG, "PKM service is running");
            return true;
        }
    }
    return false;
}

void ApmEventsSubscriber::stopPkmService() {
    if (isPkmServiceRunning()) {
        property_set("ctl.stop", "pkm_service");
        DMC_LOGD(TAG, "Stop PKM service");
    }
}

bool ApmEventsSubscriber::isPkmServiceRunning() {
    bool running = false;
    char buffer[PROP_VALUE_MAX] = {'\0'};
    property_get("init.svc.pkm_service", buffer, "stopped");

    if (NULL == strstr(buffer, "running")) {
        running = false;
    } else {
        running = true;
    }
    return running;
}


void ApmEventsSubscriber::getSubscribedMsgs(std::vector<APM_MSGID> &subscribedMsgs) {
    subscribedMsgs.clear();
    std::vector<MSGID> subscribedMsgsUint64;
    EventsSubscriber::getSubscribedMsgs(subscribedMsgsUint64);

    for (unsigned int i = 0; i < subscribedMsgsUint64.size(); i++) {
        // It is OK to convert MSGID(uint_64) to APM_MSGID(uint_16) because
        // the APM MSGID is limited to uint_16
        subscribedMsgs.push_back((APM_MSGID)subscribedMsgsUint64[i]);
    }
}

DMC_RESULT_CODE ApmEventsSubscriber::setPacketPayloadSize(const ObjId &objId, uint32_t size) {
    if (!isSubscriberEnabled()) {
        DMC_LOGE(TAG, "setPacketPayloadSize() failed because APM subscriber is not enabled!");
        return DMC_RESULT_FAIL;
    }
    KPI_OBJ_RESULT_CODE code = mTranslatorLoader->setPacketPayloadSize(objId, size);
    DMC_LOGD(TAG, "setPacketPayloadSize(%" PRIu32") to OID = %s, ret = %d",
            size, objId.getOidString(), code);
    return (code != KPI_OBJ_RESULT_CODE_SUCESS)? DMC_RESULT_FAIL : DMC_RESULT_SUCCESS;
}

DMC_RESULT_CODE ApmEventsSubscriber::setPacketPayloadType(const ObjId &objId, OTA_PACKET_TYPE type) {
    if (!isSubscriberEnabled()) {
        DMC_LOGE(TAG, "setPacketPayloadType() failed because APM subscriber is not enabled!");
        return DMC_RESULT_FAIL;
    }
    KPI_OBJ_RESULT_CODE code = mTranslatorLoader->setPacketPayloadType(objId, type);
    DMC_LOGD(TAG, "setPacketPayloadType(%d) to OID = %s, ret = %d",
            type, objId.getOidString(), code);
    return (code != KPI_OBJ_RESULT_CODE_SUCESS)? DMC_RESULT_FAIL : DMC_RESULT_SUCCESS;
}

uint64_t ApmEventsSubscriber::getTimestampUs() {
    return getCurrentTimestampUs();
}

void ApmEventsSubscriber::updateApmSubscriptionProperty(bool enable, const char *cause) {
    if (enable) {
        property_set(APM_SUBSCRIPTION_PROP_NAME, "1");
    } else {
        property_set(APM_SUBSCRIPTION_PROP_NAME, "0");
    }
    DMC_LOGD(TAG, "updateApmSubscriptionProperty(%d), cause = %s", enable, cause);
}

unsigned int ApmEventsSubscriber::getPkmSubscribedMsgSize() {
    unsigned int count = 0;
    std::map<MSGID, KPI_OBJ_TYPE> subscribedMap;
    getSubscribedMap(subscribedMap);
    std::map<MSGID, KPI_OBJ_TYPE>::iterator itor;
    for (itor = subscribedMap.begin(); itor != subscribedMap.end(); itor++) {
        if (isPkmKpiType((KPI_OBJ_TYPE)itor->second)) {
            count++;
        }
    }
    return count;
}

bool ApmEventsSubscriber::isPkmKpiType(KPI_OBJ_TYPE kpiObjType) {
    const translator_kpi_range *pKpiRange = getKpiTypeRange(KPI_SOURCE_TYPE_PKM);
    if (pKpiRange != NULL) {
        if (kpiObjType >= pKpiRange->beginIdx && kpiObjType <= pKpiRange->endIdx) {
            return true;
        }
    }

    return false;
}

