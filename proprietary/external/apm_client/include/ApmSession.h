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
/**
 * @file ApmSession.h
 *
 * @author mtk33273 (Ankur Saxena)
 *
 * @brief The header file defines @c APM Session class.
 *
 * @copyright MediaTek Inc. (C) 2015. All rights reserved.
 */

#ifndef __APM_SESSION_H__
#define __APM_SESSION_H__

#include "ap_monitor.h"
#include "ApmControlCallback.h"

#include <stdint.h>
#include <stddef.h>
#include <sys/system_properties.h>
#include <vendor/mediatek/hardware/apmonitor/2.0/IApmService.h>
#include <vector>
#include <pthread.h>

using namespace android;
using namespace vendor::mediatek::hardware::apmonitor;

using android::hardware::hidl_vec;
using android::hidl::base::V1_0::IBase;
using vendor::mediatek::hardware::apmonitor::V2_0::apmResult_e;
using vendor::mediatek::hardware::apmonitor::V2_0::IApmControlCallback;
using vendor::mediatek::hardware::apmonitor::V2_0::implementation::ApmControlCallback;

#define APM_HIDL_SERVER "apm_hidl_service"
#define APM_SERVICE_ACTIVE_PROP "vendor.dmc.apm.active"
#ifndef UNUSED
    #define UNUSED(x) do{(void)(x); } while (0);
#endif

class ApmSession
{
public:
    ApmSession();
    ~ApmSession();
    bool shouldSubmitKpi(APM_MSGID msgId);
    bool isSubscriptionStateInitialized(void);
    bool isApmServiceReady(void);
    bool isApmServiceActive();
    bool connect(void);
    void disconnect(const char *cause);
    bool submitKpi(APM_MSGID msgId, APM_SIMID simId, timeStamp timestampUs,
            uint32_t len, const void *data);
    int64_t getSessionParam();
    uint64_t getCurrentTimestamp(void);
    void reset(void);

private:
    pthread_mutex_t mMutex;
    APM_SID mSessionId;
    int64_t mSessionParam;
    android::sp<V2_0::IApmService> m_pConnectionBase_V2_0;
    android::sp<ApmControlCallback> m_pControlCallback;
    std::vector<APM_MSGID> mSubscribedMsgList;
    bool mIsServiceReady;
    bool mIsSubscriptionStateInitialized;

    static void onUpdateKpiSubStateListener(
            const std::vector<APM_MSGID> &subscribedMessages, void *param);
    static void onQueryKpiListener(
            APM_MSGID msgId, void *param);
    void dumpSubscribedMsgList(const char *cause);
    static void appendString(char **out, const char *format, ...);
};

extern ApmSession *g_pSession;

#endif