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
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#include <cstdlib>
#include <iostream>
#include <time.h>
#include <thread>
#include <unistd.h>

#define LOG_TAG "ULogTestPerf"
#include <mtkcam/utils/std/Trace.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>


using namespace NSCam::Utils::ULog;

struct LoadingInfo
{
    long runningTime;
    long idleTime;
};


inline static void getCpuLoadingInfo(LoadingInfo *loading)
{
    int statFd = open("/proc/stat", O_RDONLY);

    if (statFd > 0) {
        char buffer[100];
        char cpu[10];

        loading->runningTime = 0;
        loading->idleTime = 0;

        int n = read(statFd, buffer, sizeof(buffer));
        if (n > 10) {
            long user, nice, system, idle, iowait, irq, softirq;
            if (sscanf(buffer, "%s %ld %ld %ld %ld %ld %ld %ld", cpu, &user, &nice, &system, &idle, &iowait, &irq, &softirq) == 8) {
                loading->runningTime = user + nice + system + iowait + irq + softirq;
                loading->idleTime = idle;
            }
        }

        close(statFd);
    }
}


static float calCpuLoading(const LoadingInfo &begin, const LoadingInfo &end)
{
    long runningTime = end.runningTime - begin.runningTime;
    long idleTime = end.idleTime - begin.idleTime;
    return (static_cast<float>(runningTime) * 100.0f) / static_cast<float>(runningTime + idleTime);
}


static int timeDiffUs(const timespec &begin, const timespec &end)
{
    return static_cast<int>(
        (std::int64_t{end.tv_sec} - std::int64_t{begin.tv_sec}) * 1000000 +
        (std::int64_t{end.tv_nsec} - std::int64_t{begin.tv_nsec}) / 1000);
};


static void testULogEnter(int num)
{
    std::cout << "ULogEnt\t";

    LoadingInfo loadingBegin;
    getCpuLoadingInfo(&loadingBegin);

    timespec begin;
    clock_gettime(CLOCK_MONOTONIC, &begin);

    for (int i = 0; i < num; i++) {
        CAM_ULOG_ENTER(MOD_STREAMING_HELPER, REQ_STR_FPIPE_REQUEST, i);
        CAM_ULOG_EXIT(MOD_STREAMING_HELPER, REQ_STR_FPIPE_REQUEST, i);
        CAM_ULOG_ENTER(MOD_STREAMING_HELPER, REQ_STR_FPIPE_REQUEST, i);
        CAM_ULOG_EXIT(MOD_STREAMING_HELPER, REQ_STR_FPIPE_REQUEST, i);
        CAM_ULOG_ENTER(MOD_STREAMING_HELPER, REQ_STR_FPIPE_REQUEST, i);
        CAM_ULOG_EXIT(MOD_STREAMING_HELPER, REQ_STR_FPIPE_REQUEST, i);
        CAM_ULOG_ENTER(MOD_STREAMING_HELPER, REQ_STR_FPIPE_REQUEST, i);
        CAM_ULOG_EXIT(MOD_STREAMING_HELPER, REQ_STR_FPIPE_REQUEST, i);
    }

    timespec end1;
    clock_gettime(CLOCK_MONOTONIC, &end1);

    if (ULogger::isModeEnabled(ULogger::PASSIVE_LOG)) {
        // So that the CPU loading accurate
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    } else {
        NSCam::Utils::ULog::ULogger::flush(100);
    }

    timespec end2;
    clock_gettime(CLOCK_MONOTONIC, &end2);

    LoadingInfo loadingEnd;
    getCpuLoadingInfo(&loadingEnd);

    // Measure latency
    std::cout << timeDiffUs(begin, end1) << "\t" << timeDiffUs(begin, end2);
    std::cout << "\t" << calCpuLoading(loadingBegin, loadingEnd) << std::endl;
}


static void testULogDetails(int num)
{
    std::cout << "Details\t";

    LoadingInfo loadingBegin;
    getCpuLoadingInfo(&loadingBegin);

    timespec begin;
    clock_gettime(CLOCK_MONOTONIC, &begin);

    for (int i = 0; i < num; i++) {
        CAM_ULOGD(MOD_STREAMING_HELPER, "R StreamingFeatureRequest:%d  M[S_Helper:100400c] + ", i);
        CAM_ULOGD(MOD_STREAMING_HELPER, "R StreamingFeatureRequest:%d  M[S_Helper:100400c] - ", i);
        CAM_ULOGD(MOD_STREAMING_HELPER, "R StreamingFeatureRequest:%d  M[S_Helper:100400c] + ", i);
        CAM_ULOGD(MOD_STREAMING_HELPER, "R StreamingFeatureRequest:%d  M[S_Helper:100400c] - ", i);
        CAM_ULOGD(MOD_STREAMING_HELPER, "R StreamingFeatureRequest:%d  M[S_Helper:100400c] + ", i);
        CAM_ULOGD(MOD_STREAMING_HELPER, "R StreamingFeatureRequest:%d  M[S_Helper:100400c] - ", i);
        CAM_ULOGD(MOD_STREAMING_HELPER, "R StreamingFeatureRequest:%d  M[S_Helper:100400c] + ", i);
        CAM_ULOGD(MOD_STREAMING_HELPER, "R StreamingFeatureRequest:%d  M[S_Helper:100400c] - ", i);
    }

    timespec end1;
    clock_gettime(CLOCK_MONOTONIC, &end1);

    if (ULogger::isModeEnabled(ULogger::PASSIVE_LOG)) {
        // So that the CPU loading accurate
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    } else {
        NSCam::Utils::ULog::ULogger::flush(100);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    timespec end2;
    clock_gettime(CLOCK_MONOTONIC, &end2);

    LoadingInfo loadingEnd;
    getCpuLoadingInfo(&loadingEnd);

    // Measure latency
    std::cout << timeDiffUs(begin, end1) << "\t" << timeDiffUs(begin, end2);
    std::cout << "\t" << calCpuLoading(loadingBegin, loadingEnd) << std::endl;
}


static void testAndroidLog(int num)
{
    std::cout << "Android\t";

    LoadingInfo loadingBegin;
    getCpuLoadingInfo(&loadingBegin);

    timespec begin;
    clock_gettime(CLOCK_MONOTONIC, &begin);

    for (int i = 0; i < num; i++) {
        CAM_LOGD("R StreamingFeatureRequest:%d  M[S_Helper:100400c] + ", i);
        CAM_LOGD("R StreamingFeatureRequest:%d  M[S_Helper:100400c] - ", i);
        CAM_LOGD("R StreamingFeatureRequest:%d  M[S_Helper:100400c] + ", i);
        CAM_LOGD("R StreamingFeatureRequest:%d  M[S_Helper:100400c] - ", i);
        CAM_LOGD("R StreamingFeatureRequest:%d  M[S_Helper:100400c] + ", i);
        CAM_LOGD("R StreamingFeatureRequest:%d  M[S_Helper:100400c] - ", i);
        CAM_LOGD("R StreamingFeatureRequest:%d  M[S_Helper:100400c] + ", i);
        CAM_LOGD("R StreamingFeatureRequest:%d  M[S_Helper:100400c] - ", i);
    }

    timespec end1;
    clock_gettime(CLOCK_MONOTONIC, &end1);

    // So that the CPU loading accurate
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    timespec end2;
    clock_gettime(CLOCK_MONOTONIC, &end2);

    LoadingInfo loadingEnd;
    getCpuLoadingInfo(&loadingEnd);

    // Measure latency
    std::cout << timeDiffUs(begin, end1) << "\t" << timeDiffUs(begin, end2);
    std::cout << "\t" << calCpuLoading(loadingBegin, loadingEnd) << std::endl;
}


static void testSystrace(int num)
{
    std::cout << "Systrace\t";

    LoadingInfo loadingBegin;
    getCpuLoadingInfo(&loadingBegin);

    timespec begin;
    clock_gettime(CLOCK_MONOTONIC, &begin);

    for (int i = 0; i < num; i++) {
        CAM_TRACE_BEGIN("R StreamingFeatureRequest:0  M[S_Helper:100400c] + ");
        CAM_TRACE_END();
        CAM_TRACE_BEGIN("R StreamingFeatureRequest:0  M[S_Helper:100400c] + ");
        CAM_TRACE_END();
        CAM_TRACE_BEGIN("R StreamingFeatureRequest:0  M[S_Helper:100400c] + ");
        CAM_TRACE_END();
        CAM_TRACE_BEGIN("R StreamingFeatureRequest:0  M[S_Helper:100400c] + ");
        CAM_TRACE_END();
    }

    timespec end1;
    clock_gettime(CLOCK_MONOTONIC, &end1);

    // So that the CPU loading accurate
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    timespec end2;
    clock_gettime(CLOCK_MONOTONIC, &end2);

    LoadingInfo loadingEnd;
    getCpuLoadingInfo(&loadingEnd);

    // Measure latency
    std::cout << timeDiffUs(begin, end1) << "\t" << timeDiffUs(begin, end2);
    std::cout << "\t" << calCpuLoading(loadingBegin, loadingEnd) << std::endl;
}



static void testULogTrace(int num)
{
    std::cout << "ULogTrace\t";

    LoadingInfo loadingBegin;
    getCpuLoadingInfo(&loadingBegin);

    timespec begin;
    clock_gettime(CLOCK_MONOTONIC, &begin);

    for (int i = 0; i < num; i++) {
        CAM_ULOG_TAG_BEGIN(MOD_STREAMING_HELPER, "R StreamingFeatureRequest:0  M[S_Helper:100400c] + ");
        CAM_ULOG_TAG_END();
        CAM_ULOG_TAG_BEGIN(MOD_STREAMING_HELPER, "R StreamingFeatureRequest:0  M[S_Helper:100400c] + ");
        CAM_ULOG_TAG_END();
        CAM_ULOG_TAG_BEGIN(MOD_STREAMING_HELPER, "R StreamingFeatureRequest:0  M[S_Helper:100400c] + ");
        CAM_ULOG_TAG_END();
        CAM_ULOG_TAG_BEGIN(MOD_STREAMING_HELPER, "R StreamingFeatureRequest:0  M[S_Helper:100400c] + ");
        CAM_ULOG_TAG_END();
    }

    timespec end1;
    clock_gettime(CLOCK_MONOTONIC, &end1);

    if (ULogger::isModeEnabled(ULogger::PASSIVE_LOG)) {
        // So that the CPU loading accurate
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    } else {
        NSCam::Utils::ULog::ULogger::flush(100);
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }

    timespec end2;
    clock_gettime(CLOCK_MONOTONIC, &end2);

    LoadingInfo loadingEnd;
    getCpuLoadingInfo(&loadingEnd);

    // Measure latency
    std::cout << timeDiffUs(begin, end1) << "\t" << timeDiffUs(begin, end2);
    std::cout << "\t" << calCpuLoading(loadingBegin, loadingEnd) << std::endl;
}



__attribute__((noinline))
int busyVeryBusy(int k)
{
    static int v = 1;
    v = (v + 1) % 253;
    return v + k;
}


static void testBusy()
{
    std::cout << "Busy\t";

    LoadingInfo loadingBegin;
    getCpuLoadingInfo(&loadingBegin);

    timespec begin;
    clock_gettime(CLOCK_MONOTONIC, &begin);

    int k = 10;
    for (int i = 0; i < 20000000; i++) {
        k = busyVeryBusy(k);
        k = busyVeryBusy(k);
        k = busyVeryBusy(k);
        k = busyVeryBusy(k);
        k = busyVeryBusy(k);
    }

    timespec end1;
    clock_gettime(CLOCK_MONOTONIC, &end1);

    LoadingInfo loadingEnd;
    getCpuLoadingInfo(&loadingEnd);

    // Measure latency
    std::cout << timeDiffUs(begin, end1) << "\t" << timeDiffUs(begin, end1);
    std::cout << "\t" << calCpuLoading(loadingBegin, loadingEnd) << std::endl;
}


static int strToInt(const char *str)
{
    if (str[0] == '0' && str[1] == 'x') {
        return static_cast<int>(strtol(str, NULL, 16));
    }
    return static_cast<int>(strtol(str, NULL, 10));
}


int main(int argc, char *argv[])
{
    ULogInitializer ulogInit; // The constructor will init ULog

    int num = 1000;
    int mode = 0x7;

    // Before test:
    // Fix CPU/Memory frequency
    // Turn on MTKLogger

    if (argc > 1)
        mode = strToInt(argv[1]);

    if (argc > 2)
        num = strToInt(argv[2]);

    testULogEnter(10);
    testAndroidLog(10);
    testULogDetails(10);
    if (mode & 0x8) {
        testSystrace(10);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));

    std::cout << "Mode\tLatency\tDuration\tLoading" << std::endl;
    for (int i = 0; i < 20; i++) {
        if (mode & 0x1) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10000));
            testULogEnter(num);
        }
        if (mode & 0x2) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10000));
            testAndroidLog(num);
        }
        if (mode & 0x4) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10000));
            testULogDetails(num);
        }
        if (mode & 0x8) {
            // Must enable systrace
            std::this_thread::sleep_for(std::chrono::milliseconds(10000));
            testSystrace(num);
        }
        if (mode & 0x10) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10000));
            testULogTrace(num);
        }
    }

    if (mode & 0x100) {
        for (int i = 0; i < 20; i++)
            testBusy();
    }

    return 0;
}



void overhead_Enter()
{
    CAM_ULOG_ENTER(MOD_CAMERA_HAL_SERVER, REQ_APP_REQUEST, 0);
}


void overhead_ULOGD()
{
    CAM_ULOGD(MOD_CAMERA_HAL_SERVER, "Hello world");
}


void overhead_CAM_LOGD()
{
    CAM_LOGD("Hello world");
}


void overhead_FuncLife()
{
    CAM_ULOG_FUNCLIFE(MOD_CAMERA_HAL_SERVER);
}


void overhead_CAM_TRACE()
{
    CAM_TRACE_BEGIN("Func");
}

