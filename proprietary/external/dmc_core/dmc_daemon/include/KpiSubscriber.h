
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
 * MediaTek Inc. (C) 2010. All rights reserved.
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
/**
 * @file KpiSubscriber.h
 *
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The header file defines the APIs used in libmdmi.
 *
 * @copyright MediaTek Inc. (C) 2015. All rights reserved.
 */
#ifndef __KPI_SUBSCRIBER_H__
#define __KPI_SUBSCRIBER_H__

#include "KpiObj.h"
#include "DmcDefs.h"
#include "TranslatorLoader.h"
#include "MdmEventsSubscriber.h"
#include "SubscriptionBatchMdmRequest.h"

#include "libmdmonitor.h"
#include <MonitorCmdProxy.h>
#include <MonitorTrapReceiver.h>
#include <JsonCmdEncoder.h>
#include <string.h>

using namespace libmdmonitor;

class MdmEventsSubscriber;

class KpiSubscriber
{
public:
    KpiSubscriber(MdmEventsSubscriber *subscriber);
    virtual ~KpiSubscriber();

    bool isEnabled();
    // Override function must call parent
    bool enableSubscriber();
    // Override function must call parent
    void disableSubscriber();
    bool SubscriptionHandler(
            bool bSubscribe,
            KPI_OBJ_TYPE kpiObjType,
            KPI_SOURCE_TYPE source,
            MSGID msgId);
    // Batch subscription, reutrn number of success requests.
    bool SubscribeHandlerBatch(
            bool bSubscribe, KPI_SOURCE_TYPE source, SubscriptionBatchMdmRequest *pRequests);
    static bool IsMdmServiceRunning();
    static bool StartMdmServiceSync();
    static void StopMdmService();

private:
    bool mEnabled;
    MdmEventsSubscriber *m_pSubscriber;
    MonitorCmdProxy<JsonCmdEncoder> *m_pCmdClient;
    SID m_sessionId;
    MonitorTrapReceiver *m_pTrapReceiver;
    static void TrapCB(
            void *param,
            uint32_t timestamp,
            TRAP_TYPE trapType,
            size_t len,
            const uint8_t *data,
            size_t discard_count);

    bool SubscribeMsgToMdm(
            bool bSubscribe, TRAP_TYPE trapType, MSGID msgId);
    bool SubscribeMsgToMdmBatch(
            bool bSubscribe, TRAP_TYPE trapType, MSGID *msgId, uint32_t msgSize);
};

#endif
