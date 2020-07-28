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

#ifndef __DMC_CORE_H__
#define __DMC_CORE_H__

#include "IDmcCore.h"
#include "SessionManager.h"
#include "TranslatorLoader.h"
#include "KpiSubscriptionTable.h"
#include "LocalEventsSubscriber.h"
#include "ApmEventsSubscriber.h"
#include "MdmEventsSubscriber.h"
#include "ReqQManager.h"
#include "DmcService.h"
#include "KpiReportTimer.h"

#include "libmdmonitor.h"
#include "DmcDefs.h"
#include "dmc_utils.h"
#include "ObjId.h"
#include "SessionInfo.h"

using vendor::mediatek::hardware::dmc::V1_1::implementation::DmcService;

class DmcCore : public IDmcCore
{
public:
    DmcCore();
    virtual ~DmcCore();
    bool isReady();
    bool isMdmServiceRunning();
    static DMC_RESULT_CODE eventsCb(record_t *pRec, void *param);
    static DMC_RESULT_CODE kpiTimerCb(KPI_OBJ_TYPE kpiType, uint64_t timestamp, void *param);

    DMC_RESULT_CODE createSession(const SessionInfo *info, int32_t &sessionId) override;
    DMC_RESULT_CODE closeSession(int32_t sessionId) override;
    bool isValidSession(int32_t sessionId) override;
    DMC_RESULT_CODE registerEventCallback(EventCallback callback, void *param) override;
    DMC_RESULT_CODE enableSubscription() override;
    DMC_RESULT_CODE disableSubscription() override;
    DMC_RESULT_CODE setPacketPayloadSize(const ObjId *oid, uint32_t size) override;
    DMC_RESULT_CODE setpacketpayloadType(const ObjId *oid, OTA_PACKET_TYPE type) override;
    DMC_RESULT_CODE getSessionStats(int32_t session, KPI_GROUP_TYPE group, SessionStats_t &stats) override;
    DMC_RESULT_CODE queryEvent(int32_t session, const ObjId *oid, KpiObj &obj) override;
    DMC_RESULT_CODE queryEventAsync(int32_t session, uint8_t simId, const ObjId *oid) override;

    DMC_RESULT_CODE subscribe(const ObjId *oid) override;
    DMC_RESULT_CODE unSubscribe(const ObjId *oid) override;

private:
    EventCallback m_pEventCallback;
    void *m_pEventParam;
    DMC_RESULT_CODE subscribeHandler(const ObjId *oid, bool bSubscribe, bool bBatchMode);
    SessionManager *m_pSessionManager;
    TranslatorLoader *m_pTranslatorLoader;
    LocalEventsSubscriber *m_pLocalEventsSubscriber;
    ApmEventsSubscriber *m_pApmEventsSubscriber;
    MdmEventsSubscriber *m_pMdmEventsSubscriber;
    ReqQManager *m_pReqQueueManager;
    KpiSubscriptionTable *m_pKpiSubscriptionTable;
    android::sp<DmcService> m_pDmcService;
    KpiReportTimer *m_pKpiReportTimer;

    bool mReady;
    bool mSubscriptionEnabled;

    bool startMdmServiceSync();
    void stopMdmService();
};

#endif
