
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

#ifndef __KPI_REPORT_TIMER_H__
#define __KPI_REPORT_TIMER_H__

#include "KpiObj.h"
#include "DmcDefs.h"
#include "dmc_utils.h"
#include <pthread.h>
#include <inttypes.h>
#include <map>

using namespace std;

typedef DMC_RESULT_CODE (*KpiTimerHandler)(KPI_OBJ_TYPE kpiType, uint64_t timestamp, void *param);

struct ReportInfo {
    ReportInfo(KPI_OBJ_TYPE type, uint32_t timeSlotMs, pthread_t thread):
            mKpiType(type), mTimeSlotMs(timeSlotMs), mThread(thread) {}

    KPI_OBJ_TYPE mKpiType;
    uint32_t mTimeSlotMs;
    pthread_t mThread;
};

class KpiReportTimer {
public:
    KpiReportTimer();
    virtual ~KpiReportTimer();

    void setTimerHandler(KpiTimerHandler handler, void *param);
    bool startReportEntry(KPI_OBJ_TYPE kpiType, uint32_t timeSlotMs);
    void stopReportEntry(KPI_OBJ_TYPE kpiType);
    bool isReportEntryExist(KPI_OBJ_TYPE kpiType);
    ReportInfo *findReportEntry(KPI_OBJ_TYPE kpiType);
    uint32_t getReportTimeSlot(KPI_OBJ_TYPE kpiType);
    int reportEntryCount();
    void stopReport();
private:
    map<KPI_OBJ_TYPE, ReportInfo *>mReportMap;
    KpiTimerHandler mHandler;
    void *mHandlerParam;

    uint64_t getCurrentTimestampUs();
    static void *reportKpiThread(void *arg);
    static void signalHandler(int ignore);
    static void stopThread(pthread_t threadId);
};

#endif
