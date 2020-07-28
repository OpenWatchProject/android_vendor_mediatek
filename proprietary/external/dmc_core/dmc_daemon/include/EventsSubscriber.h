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

#ifndef __EVENTS_SUBSCRIBER_H__
#define __EVENTS_SUBSCRIBER_H__

#include "ReqQManager.h"
#include "TranslatorLoader.h"
#include "KpiSubscriptionTable.h"
#include "DmcDefs.h"
#include "KpiUtils.h"
#include <map>
#include <vector>

class EventsSubscriber
{
public:
    EventsSubscriber(const char *tag,
            KpiSubscriptionTable *kpiSubTable, ReqQManager *reqManager, TranslatorLoader *loader);
    virtual ~EventsSubscriber() {}

    // Child class must call parent handler
    virtual DMC_RESULT_CODE SubscribeHandler(
            bool bSubscribe,
            KPI_OBJ_TYPE kpiObjType,
            KPI_SOURCE_TYPE source,
            MSGID msgId);
    virtual bool getEventsInfo(KPI_GROUP_TYPE groupType, EventsInfo &eventsInfo);

    virtual bool enableSubscriber();
    virtual void disableSubscriber();
public:
    const char *getTranslatorVersion();
    bool isMsgSubscribed(MSGID msgId);
    bool isKpiSubscribed(KPI_OBJ_TYPE type);
    uint32_t getSubscribedMsgSize();
    void getSubscribedMap(std::map<MSGID, KPI_OBJ_TYPE> &subscribedMap);
    void getSubscribedMsgs(std::vector<MSGID> &subscribedMsgs);
    void getSubscribedKpiObjs(std::vector<KPI_OBJ_TYPE> &subscribedObjs);
    KPI_OBJ_TYPE getObjTypeByMsgId(MSGID msgId);

    uint16_t getKpiSize();
    uint8_t getKpiGroupSize();
    uint64_t getKpiTimestampDelta();
    int32_t getPacketMonitorParameter();
    const translator_kpi_range *getKpiTypeRange(KPI_SOURCE_TYPE source);
    KPI_GID getKpiGroupId(KPI_OBJ_TYPE kpiObjType);
    const TrapTable *getTrapTableByKpi(KPI_OBJ_TYPE kpiObjType);
    KPI_OBJ_RESULT_CODE translateKpi(
            KPI_OBJ_TYPE kpiType,
            KPI_MSGID msgId,
            KPI_SIMID simId,
            uint64_t timestampUs,
            KPI_SOURCE_TYPE source,
            KpiObj &outObj,
            uint64_t datalen,
            const void *data,
            const void *param);

protected:
    std::map<MSGID, KPI_OBJ_TYPE> mSubscribedMsgMap;
    ReqQManager *mReqManager;
    TranslatorLoader *mTranslatorLoader;
    KpiSubscriptionTable *mKpiSubTable;

    bool isSubscriberEnabled();
    void incKpiDroppedCount(KPI_GID groupId, KPI_COUNT_UNIT count);
    void incKpiTotalCount(KPI_GID groupId, KPI_COUNT_UNIT count);
    void resetKpiCount();
    void resetSubscribedMsgMap();
    uint64_t getCurrentTimestampUs();

private:
    EventsSubscriber() {};
    const char *mTag;
    bool mEnabled;
    const TranslatorConfig *mTranslatorConfig;
    bool loadTranslatorConfig();
    void unloadTranslatorConfig();
    KPI_COUNT_UNIT *m_pKpiDroppedCount;
    KPI_COUNT_UNIT *m_pKpiTotalCount;
};

#endif

