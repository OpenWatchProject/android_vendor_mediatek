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
#ifndef __APM_EVENTS_SUBSCRIBER_H__
#define __APM_EVENTS_SUBSCRIBER_H__

#include "IApMonitor.h"
#include "EventsSubscriber.h"
#include "KpiObj.h"
//#include "ap_monitor_server.h"
#include "ApmService.h"

#define APM_SUBSCRIPTION_PROP_NAME "vendor.dmc.apm.active"

using vendor::mediatek::hardware::apmonitor::V2_0::implementation::ApmService;

class ApmEventsSubscriber: public EventsSubscriber, public IApMonitor
{
public:
    ApmEventsSubscriber(
            KpiSubscriptionTable *kpiSubTable, ReqQManager *reqManager, TranslatorLoader *loader);
    ~ApmEventsSubscriber();

    // Interface of EventsSubscriber
    bool enableSubscriber() override;
    void disableSubscriber() override;

    DMC_RESULT_CODE SubscribeHandler(
            bool bSubscribe,
            KPI_OBJ_TYPE kpiObjType,
            KPI_SOURCE_TYPE source,
            MSGID msgId) override;

    // Interface of IApMonitor
    uint64_t getTimestampUs() override;
    DMC_RESULT_CODE handleKpi(KpiHeader header, uint32_t len, const void *data) override;
    void getSubscribedMsgs(std::vector<APM_MSGID> &subscribedMsgs) override;
    int64_t getApmSessionParam() override;
    DMC_RESULT_CODE setPacketPayloadSize(const ObjId &objId, uint32_t size);
    DMC_RESULT_CODE setPacketPayloadType(const ObjId &objId, OTA_PACKET_TYPE type);

private:
    android::sp<ApmService> m_pApmService;
    static bool startPkmServiceSync();
    static void stopPkmService();
    static bool isPkmServiceRunning();

    void updateApmSubscriptionProperty(bool enable, const char *cause);
    unsigned int getPkmSubscribedMsgSize();
    bool isPkmKpiType(KPI_OBJ_TYPE kpiObjType);
};

#endif

