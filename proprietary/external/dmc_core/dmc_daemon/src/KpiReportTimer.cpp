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

#include "KpiReportTimer.h"
#include <errno.h>
#include <unistd.h>

static const char *TAG = "DMC-Core";

struct ThreadArgument {
    KpiReportTimer *arg1;
    int32_t *arg2;
};

// ============================ Public interface for TranslatorLoader ============================
KpiReportTimer::KpiReportTimer(): mHandler(NULL), mHandlerParam(NULL) {
    DMC_LOGD(TAG, "constructor()");

    // Kill Signal Initialization
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_flags = 0;
    action.sa_handler = signalHandler;
    sigaction(SIGUSR1, &action, (struct sigaction *)0);
    // Setting SIGUSR1
    sigset_t sigmask;
    sigemptyset(&sigmask);
    sigaddset(&sigmask, SIGUSR1);
}

KpiReportTimer::~KpiReportTimer() {
    DMC_LOGD(TAG, "destructor()");
}

void KpiReportTimer::setTimerHandler(KpiTimerHandler handler, void *param) {
    mHandler = handler;
    mHandlerParam = param;
}

bool KpiReportTimer::startReportEntry(KPI_OBJ_TYPE kpiType, uint32_t timeSlotMs) {
    DMC_LOGD(TAG, "startReportEntry for kpiType = %d, timeSlotMs = %u", kpiType, timeSlotMs);

    ReportInfo *pInfo = findReportEntry(kpiType);
    if (pInfo == NULL) {
        pthread_t thread;
        ThreadArgument *pThreadArgument = (ThreadArgument *)calloc(1, sizeof(ThreadArgument));
        if (pThreadArgument == NULL) {
            DMC_LOGE(TAG, "OOM calloc ThreadArgument failed!");
            return false;
        }
        pThreadArgument->arg1 = this;
        pThreadArgument->arg2 = (int32_t *)calloc(1, sizeof(int32_t));
        if (pThreadArgument->arg2 == NULL) {
            DMC_LOGE(TAG, "OOM calloc int32_t failed!");
            free(pThreadArgument);
            return false;
        }

        *(pThreadArgument->arg2) = (int32_t)kpiType;

        // Free ThreadArgument in reportKpiThread()
        if (pthread_create(&thread, NULL, reportKpiThread, (void *)pThreadArgument)) {
            DMC_LOGE(TAG, "Create thread failed");
            free(pThreadArgument->arg2);
            free(pThreadArgument);
            return false;
        } else {
            DMC_LOGD(TAG, "Create thread %ld success", thread);
            pInfo = new ReportInfo(kpiType, timeSlotMs, thread);
            if (pInfo == NULL) {
                DMC_LOGD(TAG, "OOM, kill thread %ld", thread);
                stopThread(thread);
                return false;
            }
            mReportMap.insert(make_pair(kpiType, pInfo));
        }

    } else {
        DMC_LOGW(TAG, "Skip startReportEntry for exist kpiType = %d", kpiType);
    }
    return false;
}

void KpiReportTimer::stopReportEntry(KPI_OBJ_TYPE kpiType) {
    DMC_LOGD(TAG, "stopReportEntry for kpiType = %d", kpiType);

    ReportInfo *pInfo = findReportEntry(kpiType);
    if (pInfo != NULL) {
        stopThread(pInfo->mThread);
        map<KPI_OBJ_TYPE, ReportInfo *>::iterator it;
        if ((it = mReportMap.find(kpiType)) != mReportMap.end()) {
            delete it->second;
            mReportMap.erase(it);
        }
    } else {
        DMC_LOGW(TAG, "Skip stopReportEntry for kpiType = %d", kpiType);
    }
}

uint32_t KpiReportTimer::getReportTimeSlot(KPI_OBJ_TYPE kpiType) {
    ReportInfo *pInfo = findReportEntry(kpiType);
    if (pInfo == NULL) {
        DMC_LOGW(TAG, "getReportTimeSlot failed for kpiType = %d", kpiType);
        return 0;
    }
    return pInfo->mTimeSlotMs;
}

ReportInfo *KpiReportTimer::findReportEntry(KPI_OBJ_TYPE kpiType) {
    map<KPI_OBJ_TYPE, ReportInfo *>::iterator it;
    if ((it = mReportMap.find(kpiType)) != mReportMap.end()) {
        return (ReportInfo *) it->second;
    }
    return NULL;
}

bool KpiReportTimer::isReportEntryExist(KPI_OBJ_TYPE kpiType) {
    map<KPI_OBJ_TYPE, ReportInfo *>::iterator it;
    if ((it = mReportMap.find(kpiType)) != mReportMap.end()) {
        return true;
    }
    return false;
}

int KpiReportTimer::reportEntryCount() {
    return mReportMap.size();
}

void KpiReportTimer::stopReport() {
    DMC_LOGD(TAG, "stopReport()");
    map<KPI_OBJ_TYPE, ReportInfo *>::iterator it;
    if (reportEntryCount() == 0) {
        return;
    }

    DMC_LOGD(TAG, "stopReport: dump registed KPI list[type, oid]");
    for (it = mReportMap.begin(); it != mReportMap.end(); ++it) {
        ReportInfo *pInfo = it->second;
        stopReportEntry(pInfo->mKpiType);
    }
}

void *KpiReportTimer::reportKpiThread(void *arg) {
    pthread_t threadMe = pthread_self();
    uint32_t timeSlotMs = 0;

    DMC_LOGD(TAG, "Enter reportKpiThread on thread = %ld", threadMe);
    ThreadArgument *pArgument = (ThreadArgument *)arg;
    if (pArgument == NULL) {
        DMC_LOGE(TAG, "Exit reportKpiThread with NULL context");
        return NULL;
    }
    KpiReportTimer *me = (KpiReportTimer *)pArgument->arg1;
    KPI_OBJ_TYPE kpiType = *(pArgument->arg2);

    // Freee thread argument
    free(pArgument->arg2);
    free(pArgument);

    ReportInfo *pInfo = me->findReportEntry(kpiType);
    if (pInfo == NULL) {
        DMC_LOGE(TAG, "Exit reportKpiThread with invalid kpiType = %d", kpiType);
        return NULL;
    }

    // Return 0 if not equal
    if (pthread_equal(pInfo->mThread, threadMe) == 0) {
        DMC_LOGE(TAG, "Exit reportKpiThread with invalid thread = %ld, kpiType = %d",
                pInfo->mThread, kpiType);
        return NULL;
    }

    timeSlotMs = me->getReportTimeSlot(kpiType);
    if (timeSlotMs == 0) {
        DMC_LOGE(TAG, "Exit reportKpiThread with invalid timeslot 0");
        return NULL;
    }

    while (true) {
        usleep(timeSlotMs*1000);
        DMC_LOGD(TAG, "Report kpiType = %d with timeslot = %u", kpiType, timeSlotMs);
        me->mHandler(kpiType, me->getCurrentTimestampUs(), me->mHandlerParam);
    }

    DMC_LOGD(TAG, "Leave reportKpiThread on thread = %ld", threadMe);

    return NULL;
}

void KpiReportTimer::signalHandler(int ignore) {
    UNUSED(ignore);

    DMC_LOGD(TAG, "Stop thread %ld" , pthread_self());
    pthread_exit(nullptr);
    return;
}

void KpiReportTimer::stopThread(pthread_t threadId) {
    DMC_LOGD(TAG, "stopThread %ld", threadId);

    if (pthread_equal(threadId, pthread_self()) != 0) {
        DMC_LOGE(TAG, "not kill self %ld", threadId);
        return;
    }

    // Query thread is exist or not
    int rc = pthread_kill(threadId, 0);
    if (rc != 0) {
        if (rc == ESRCH) {
            DMC_LOGW(TAG, "thread not exist");
        } else {
            DMC_LOGE(TAG, "stopThread kill 0 fail, rc = %d", rc);
        }
        return;
    }

    rc = pthread_kill(threadId, SIGUSR1);
    if (rc != 0) {
        DMC_LOGE(TAG, "stopThread kill SIGUSR1 fail, rc = %d", rc);
        return;
    }

    DMC_LOGD(TAG, "stopThread join killed thread");
    rc = pthread_join(threadId, NULL);
    if (rc != 0) {
        DMC_LOGE(TAG, "thread join failed");
    }
    DMC_LOGD(TAG, "stopThread success!");
}

uint64_t KpiReportTimer::getCurrentTimestampUs() {
    struct timespec res;
    clock_gettime(CLOCK_REALTIME, &res);
    uint64_t current = 1000000.0*res.tv_sec + (double)res.tv_nsec/1e3;
    return current;
}

