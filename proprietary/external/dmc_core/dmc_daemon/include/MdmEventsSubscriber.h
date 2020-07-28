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

#ifndef __MDM_EVENTS_SUBSCRIBER_H__
#define __MDM_EVENTS_SUBSCRIBER_H__

#include "KpiSubscriber.h"
#include "EventsSubscriber.h"
#include "SubscriptionBatchMdmRequest.h"
#include "KpiObj.h"
#include <vector>
#include <IcdDecoder.h>
#include <FrameDecoder.h>

// To solve include ecah other issue.
class KpiSubscriber;

class MdmEventsSubscriber: public EventsSubscriber
{
public:
    MdmEventsSubscriber(
            KpiSubscriptionTable *kpiSubTable, ReqQManager *reqManager, TranslatorLoader *loader);
    ~MdmEventsSubscriber();

    bool enableSubscriber();
    void disableSubscriber();

    void TrapCallback(TRAP_TYPE trapType, size_t len, const uint8_t *data, size_t discard_count);

    DMC_RESULT_CODE SubscribeHandler(
            bool bSubscribe,
            KPI_OBJ_TYPE kpiObjType,
            KPI_SOURCE_TYPE source,
            MSGID msgId) override;

    DMC_RESULT_CODE SubscribeHandlerBatch(bool bSubscribe,
            SubscriptionBatchMdmRequest *pRequests);
    static bool IsMdmServiceRunning();
    static bool StartMdmServiceSync();
    static void StopMdmService();

private:
    KpiSubscriber *m_kpiSubscriber;
    IcdDecoder* m_pDecoder;
    uint32_t m_rtosTime;
    uint32_t m_prevMDTimestamp;
    uint64_t m_elapsedMDTime;
    uint64_t m_referenceTimestampUs;

    int UpdateRTOSTime(size_t len, const uint8_t *data);
    uint64_t GetCurrentTimestampUs(void);
    void SetMDTimestamp(uint32_t currentMDTimestamp);
    uint64_t GetMDTimestampUs(void);
    void SetDeltaMDTime(uint32_t deltaMDTime);
    bool SubscribeHandlerBatchInner(
            bool bSubscribe, KPI_SOURCE_TYPE source,
            SubscriptionBatchMdmRequest *pRequests);
};

#endif

