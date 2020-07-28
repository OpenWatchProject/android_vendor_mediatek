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

#include "MdmEventsSubscriber.h"
#include "TranslateParams.h"
#include "dmc_utils.h"

#undef TAG
#define TAG "MDM-Subscriber"


using namespace libmdmonitor;

MdmEventsSubscriber::MdmEventsSubscriber(
        KpiSubscriptionTable *kpiSubTable, ReqQManager *reqManager, TranslatorLoader *loader):
        EventsSubscriber(TAG, kpiSubTable, reqManager, loader) {
    m_pDecoder = new IcdDecoder();
    m_kpiSubscriber = new KpiSubscriber(this);

    // EM timestamp from ...
    m_rtosTime = 0;
    m_elapsedMDTime = 0;
    m_referenceTimestampUs = 0;
    m_prevMDTimestamp = 0;
}

MdmEventsSubscriber::~MdmEventsSubscriber()
{
    if (m_pDecoder != NULL) {
        delete m_pDecoder;
        m_pDecoder = NULL;
    }
    if (m_kpiSubscriber != NULL) {
        delete m_kpiSubscriber;
        m_kpiSubscriber = NULL;
    }
}

/* ============================ Public Interfaces of MdmEventsSubscriber ============================ */
bool MdmEventsSubscriber::enableSubscriber() {
    if (isSubscriberEnabled()) {
        DMC_LOGW(TAG, "enableSubscriber() skipped, already enabled");
        return true;
    }
    if (!m_kpiSubscriber->enableSubscriber()) {
        return false;
    }
    return EventsSubscriber::enableSubscriber();
}

void MdmEventsSubscriber::disableSubscriber() {
    if (isSubscriberEnabled()) {
        m_kpiSubscriber->disableSubscriber();
        m_rtosTime = 0;
        m_elapsedMDTime = 0;
        m_referenceTimestampUs = 0;
        m_prevMDTimestamp = 0;
        EventsSubscriber::disableSubscriber();
    }
}

DMC_RESULT_CODE MdmEventsSubscriber::SubscribeHandler(
        bool bSubscribe,
        KPI_OBJ_TYPE kpiObjType,
        KPI_SOURCE_TYPE source,
        MSGID msgId) {
    if (!isSubscriberEnabled()) {
        DMC_LOGE(TAG, "SubscribeEvent() failed, subscriber not enabled!");
        return DMC_RESULT_FAIL;
    }
    if (m_kpiSubscriber->SubscriptionHandler(bSubscribe, kpiObjType, source, msgId)) {
        // Call parent to update mSubscribedMsgMap if executed subscription request.
        EventsSubscriber::SubscribeHandler(bSubscribe, kpiObjType, source, msgId);
    }
    return DMC_RESULT_SUCCESS;
}

DMC_RESULT_CODE MdmEventsSubscriber::SubscribeHandlerBatch(
        bool bSubscribe, SubscriptionBatchMdmRequest *pRequests) {
    if (!isSubscriberEnabled()) {
        DMC_LOGE(TAG, "SubscribeHandlerBatch() failed, subscriber not enabled!");
        return DMC_RESULT_FAIL;
    }

    int failCount = 0;
    if (!SubscribeHandlerBatchInner(bSubscribe, KPI_SOURCE_TYPE_MDM_EM_MSG, pRequests)) {
        failCount++;
    }
    if (!SubscribeHandlerBatchInner(bSubscribe, KPI_SOURCE_TYPE_MDM_EM_OTA, pRequests)) {
        failCount++;
    }
    if (!SubscribeHandlerBatchInner(bSubscribe, KPI_SOURCE_TYPE_MDM_ICD_RECORD, pRequests)) {
        failCount++;
    }
    if (!SubscribeHandlerBatchInner(bSubscribe, KPI_SOURCE_TYPE_MDM_ICD_EVENT, pRequests)) {
        failCount++;
    }

    return (failCount > 0)? DMC_RESULT_FAIL : DMC_RESULT_SUCCESS;
}

bool MdmEventsSubscriber::IsMdmServiceRunning() {
    return KpiSubscriber::IsMdmServiceRunning();
}

bool MdmEventsSubscriber::StartMdmServiceSync() {
    return KpiSubscriber::StartMdmServiceSync();
}

void MdmEventsSubscriber::StopMdmService() {
    return KpiSubscriber::StopMdmService();
}

// ============================ Private Interfaces of MdmEventsSubscriber ============================

bool MdmEventsSubscriber::SubscribeHandlerBatchInner(
        bool bSubscribe, KPI_SOURCE_TYPE source,
        SubscriptionBatchMdmRequest *pRequests) {

    std::vector<SubscriptionRequest> requests;
    if (!pRequests->getRequestList(source, requests)) {
        DMC_LOGE(TAG, "getRequestList(%s) failed!", KpiUtils::kpiSource2String(source));
        return false;
    }

    if (requests.size() == 0) {
        DMC_LOGD(TAG, "SubscribeHandlerBatchInner(%s) empty request", KpiUtils::kpiSource2String(source));
        return true;
    }

    if (m_kpiSubscriber->SubscribeHandlerBatch(bSubscribe, source, pRequests)) {
        DMC_LOGD(TAG, "SubscribeHandlerBatch(%s), bSubscribe = %d",
                KpiUtils::kpiSource2String(source), bSubscribe);
        for (unsigned int i = 0; i < requests.size(); i++) {
            SubscriptionRequest request = requests[i];
            EventsSubscriber::SubscribeHandler(
                    bSubscribe, request.mObjType, source, request.mMsgId);
        }
        return true;
    }

    DMC_LOGE(TAG, "SubscribeHandlerBatch(%s) failed!", KpiUtils::kpiSource2String(source));
    return false;
}


void MdmEventsSubscriber::TrapCallback(TRAP_TYPE trapType, size_t len, const uint8_t *data, size_t discard_count) {
    FRAME_INFO frameInfo;
    MSGID msgId = 0;
    uint16_t frameNumber = 0;
    const uint8_t *payload = (const uint8_t *)&frameInfo;
    uint16_t kpiSize = getKpiSize();
    KPI_SOURCE_TYPE source = KPI_SOURCE_TYPE_UNDEFINED;

    if (!isSubscriberEnabled()) {
        MDM_LOGE("Skip TrapCallback() with disabled state");
        return;
    }

    DMC_LOGD(TAG, "TrapCallback() trapType [%d], discard_count [%zu]", (int)trapType, discard_count);

    if (TRAP_TYPE_UNDEFINED >= trapType || kpiSize <= trapType) {
        MDM_LOGE("Bad parameter. trapType == [%d], TRAP_TYPE_SIZE == [%d]", (int)trapType, kpiSize);
        return;
    }

    memset(&frameInfo, 0, sizeof(frameInfo));

    if (trapType == TRAP_TYPE_ICD_RECORD || trapType == TRAP_TYPE_ICD_EVENT) {
        if (!m_pDecoder->ParseTrapInfo(trapType, m_rtosTime, data, len, frameInfo, payload, frameNumber)) {
            DMC_LOGE(TAG, "Failed to parse trap info. trapType == [%d], len == [%d]", (int)trapType, (int)len);
            return;
        }

        DMC_LOGD(TAG, "TrapCallback() ParseTrapInfo SUCCESS, payload=[%p].", payload);

        if (trapType == TRAP_TYPE_ICD_RECORD) {
            DMC_LOGD(TAG, "Got ICD RECORD. code = [%" PRIu64"]\n", frameInfo.msgID);
            source = KPI_SOURCE_TYPE_MDM_ICD_RECORD;
        } else if (trapType == TRAP_TYPE_ICD_EVENT) {
            DMC_LOGD(TAG, "Got ICD EVENT. code = [%" PRIu64"]\n", frameInfo.msgID);
            source = KPI_SOURCE_TYPE_MDM_ICD_EVENT;
        }
        msgId = frameInfo.msgID;
    } else {
        FrameDecoder &decoder = *(FrameDecoder::GetInstance());

        if (!decoder.ParseTrapInfo(trapType, m_rtosTime, data, len, frameInfo)) {
            DMC_LOGE(TAG, "Failed to parse trap info. trapType == [%d], len == [%d]", (int)trapType, (int)len);
            return;
        }
        if (trapType == TRAP_TYPE_OTA) {
            source = KPI_SOURCE_TYPE_MDM_EM_OTA;
            DMC_LOGD(TAG, "Got Trace. traceID = [%" PRIu64"] globalID = [%" PRIu64"]\n", frameInfo.traceID, frameInfo.otaGlobalID);
        } else if (trapType == TRAP_TYPE_EM) {
            source = KPI_SOURCE_TYPE_MDM_EM_MSG;
            DMC_LOGD(TAG, "Got EM. MSG ID = [%" PRIu64"]", frameInfo.msgID);
        } else if (trapType == TRAP_TYPE_PSTIME) {
            // update RTOS time.
            if(UpdateRTOSTime(len, data)){
                // error handling
                DMC_LOGE(TAG, "ERROR : Failed to update MD RTOS Time.");
            }
            else {
                DMC_LOGD(TAG, "Receive frame : update MD rtos time [%u].", m_rtosTime);
                SetMDTimestamp(m_rtosTime);
            }
            return;
        }

        msgId = (TRAP_TYPE_OTA == trapType)? frameInfo.otaGlobalID : frameInfo.msgID;
    }

    // Traverse all translators's TrapTable and check if it subscribes the message,
    // translate it in application's translation library to KpiObj object,
    // then deliver the result to reqQManager.
    uint8_t numKpiTranslated = 0;
    int32_t beginIdx = KPI_TYPE_BEGIN;
    int32_t endIdx = getKpiSize();
    /*
    const translator_kpi_range *pKpiRange = getKpiTypeRange(KPI_SOURCE_TYPE_MDM);
    if (pKpiRange != NULL) {
        beginIdx = pKpiRange->beginIdx;
        endIdx = pKpiRange->endIdx;
    }
    */
    DMC_LOGD(TAG, "Got MDM event. msgId = [%" PRIu64"], source = %s",
            msgId, KpiUtils::kpiSource2String(source));

    if (!isMsgSubscribed(msgId)) {
        DMC_LOGE(TAG, "Not subscribe msgId = [%" PRIu64"]", msgId);
        return;
    }

    // DMC_LOGD(TAG, "beginIdx=%" PRId32", endIdx=%" PRId32"", beginIdx, endIdx);
    // ICD messages includes timestamp in its header
    // EM messages get from TRAP_TYPE_PSTIME EM message.
    if (trapType != TRAP_TYPE_ICD_RECORD && trapType != TRAP_TYPE_ICD_EVENT) {
        frameInfo.timestamp = GetMDTimestampUs();
    }

    uint64_t delta = getKpiTimestampDelta();
    if (frameInfo.timestamp > delta) {
        frameInfo.timestamp -= delta;
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
        if (NULL == trapTable->GetUsedTrapByID(trapType, msgId)) {
            continue;
        }

        KpiObj kpiObj(kpiType);
        ModemMonitorParam *mdmParam = new ModemMonitorParam();
        if (mdmParam == NULL) {
            DMC_LOGE(TAG, "New ModemMonitorParam failed!");
            return;
        }

        mdmParam->m_frameNumber = frameNumber;
        mdmParam->m_pFrameInfo = &frameInfo;
        mdmParam->m_pPayload = payload;

        KPI_SIMID simId = 0;
        if (frameInfo.simIdx > 0) {
            simId = frameInfo.simIdx - 1;
        } else {
            simId = 0;
        }

        KPI_OBJ_RESULT_CODE ret = EventsSubscriber::translateKpi(
                kpiType, (KPI_MSGID)msgId, simId, frameInfo.timestamp,
                source, kpiObj, len, (const void *)data, (const void *)mdmParam);

        delete mdmParam;
        mdmParam = NULL;

        KPI_GID groupType = getKpiGroupId((KPI_OBJ_TYPE)kpiType);
        if (KPI_GROUP_TYPE_UNDEFINED == groupType) {
            DMC_LOGE(TAG, "Invalid groupId with type = %d, assigned to 0", kpiType);
            groupType = 0;
        }

        switch(ret) {
            case KPI_OBJ_RESULT_CODE_SUCESS:
                numKpiTranslated++;
                incKpiTotalCount(groupType, 1);
                // Calculate lost packet from MDM for the first trap.
                if (discard_count > 0 && numKpiTranslated == 1) {
                    incKpiTotalCount(groupType, (KPI_COUNT_UNIT)discard_count);
                    incKpiDroppedCount(groupType, (KPI_COUNT_UNIT)discard_count);
                }
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
        mReqManager->EnqueueRequest(kpiObj.GetType(), groupType, ptr, dataSize, frameInfo.timestamp);

        DMC_LOGD(TAG, "translateKpi(%d) done. msgId=[%" PRIu64"], dataSize=%u, time=%" PRIu64", ret=%d",
                kpiType, (uint64_t)msgId, dataSize, frameInfo.timestamp, ret);
    }
}

int MdmEventsSubscriber::UpdateRTOSTime(size_t len, const uint8_t *data)
{
    uint8_t *count = (uint8_t *)data;
    uint8_t *data_ptr = (uint8_t *)(data + 1); // parse after count (1B)

    // check len is valid for count value
    if( ((*count) * 5) > (uint8_t)(len-1) ) {
        DMC_LOGE(TAG, "Time trap len is too short. count = %d.", *count);
        return 1;
    }

    // dump raw data for debug
    /*for(unsigned char j = 0 ;  j < len ; ++j) {
        MDM_LOGD("data [%d] = %d (0x%02x).",j,*(data+j),*(data+j));
    }*/

    // get time which time type is 1 (RTOS) and store to g_MDPSTime
    m_rtosTime = 0;
    for(uint8_t i = 0 ; i < *count ; ++i) {
        uint8_t *timeType = data_ptr; data_ptr++;
        if (*timeType == 1)
        {
            for(int i = 4 - 1 ; i >= 0 ; --i) {
                m_rtosTime += *(data_ptr+i) & 0xFF;
                if(i != 0) {
                    m_rtosTime <<= 8;
                }
            }
            return 0;
        } else {
            data_ptr += 4;
        }
    }

    return 2; // update failed.
}

void MdmEventsSubscriber::SetMDTimestamp(uint32_t currentMDTimestamp)
{
    if (m_referenceTimestampUs == 0) {
        //backup Reference time.
        m_referenceTimestampUs = getCurrentTimestampUs();
        mTranslatorLoader->setModemBaseTime(currentMDTimestamp, m_referenceTimestampUs);
        DMC_LOGD(TAG, "update m_referenceTimestampUs = %" PRIu64, m_referenceTimestampUs);
    } else if (currentMDTimestamp < m_prevMDTimestamp) {
        m_elapsedMDTime += (UINT32_MAX - m_prevMDTimestamp) + currentMDTimestamp; //buffer overflow
        m_referenceTimestampUs = getCurrentTimestampUs();
        mTranslatorLoader->setModemBaseTime(currentMDTimestamp, m_referenceTimestampUs);
        DMC_LOGD(TAG, "rtos time overflow, update m_referenceTimestampUs = %" PRIu64, m_referenceTimestampUs);
    } else {
        m_elapsedMDTime += currentMDTimestamp - m_prevMDTimestamp;
    }
    // MLOGL1(LOG_INFO, TAG, "SetMDTimestamp: prev_ms = %d, curr_ms = %d", (m_prevMDTimestamp*64)/1000, (currentMDTimestamp*64)/1000);
    m_prevMDTimestamp = currentMDTimestamp;
}

void MdmEventsSubscriber::SetDeltaMDTime(uint32_t deltaMDTime)
{
    DMC_LOGD(TAG, "SetDeltaMDTime, deltaMDTime = [%" PRIu32"]", deltaMDTime);

    if (m_elapsedMDTime > deltaMDTime) {
        m_elapsedMDTime -= deltaMDTime;
        m_prevMDTimestamp -= deltaMDTime;
    }
}

uint64_t MdmEventsSubscriber::GetMDTimestampUs(void) {
    return m_referenceTimestampUs + (m_elapsedMDTime*64);
}

uint64_t MdmEventsSubscriber::GetCurrentTimestampUs(void) {
    uint64_t time = 0;
    struct timespec res;

    clock_gettime(CLOCK_REALTIME, &res);
    time = 1000000.0 * res.tv_sec + (double) res.tv_nsec / 1e3; // fill time in microseconds

    return time;
}

