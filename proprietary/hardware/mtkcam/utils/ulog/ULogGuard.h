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

#ifndef __MTKCAM_UTILS_STD_ULOG_GUARD_H__
#define __MTKCAM_UTILS_STD_ULOG_GUARD_H__

#include <cstdint>
#include <atomic>
#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <vector>
#include <deque>
#include <string>
#include <utility>
#include <time.h>
#include <mtkcam/utils/std/ULog.h>


namespace NSCam {
namespace Utils {
namespace ULog {


struct ThreadGuardFrame
{
    int warnMs;
    int abortMs;
    int tid;
    ModuleId moduleId;
    timespec timeStamp;
    const char *funcName;
    const char *longFuncName;
    const char *dispatchKey;
    void *frameData;

    ThreadGuardFrame() noexcept :
        warnMs(0), abortMs(0), tid(0), moduleId(0), timeStamp{0,0},
        funcName(nullptr), longFuncName(nullptr), dispatchKey(nullptr), frameData(nullptr)
    {
    }

    ThreadGuardFrame(const ThreadGuardFrame&) = default;
    ThreadGuardFrame& operator=(const ThreadGuardFrame&) = default;
};


struct ThreadGuardData final
{
    static constexpr uint32_t MEM_GUARD_VALUE = 0xabcdabcd;
    static constexpr int MAX_FRAMES = 3;

    enum StatusFlags {
        BACKTRACE_PRINTED = 0x1
    };

    uint32_t FRONT_GUARD;
    mutable std::mutex dataMutex;
    int tid;
    int depth;
    ThreadGuardFrame frame[MAX_FRAMES];
    unsigned int status;
    uint32_t REAR_GUARD; // not accessed by code, but can be checked in gdb

    ThreadGuardData();
    ~ThreadGuardData();
    ThreadGuardData(const ThreadGuardData&) = delete;
    ThreadGuardData& operator=(const ThreadGuardData&) = delete;
};


class ULogGuardMonitor
{
    friend class ULogGuard;

public:
    static ULogGuardMonitor& getSingleton() {
        return sSingleton;
    }

    void registerThread(ThreadGuardData *guardData);
    void unregisterThread(ThreadGuardData *guardData);
    void registerTimeBomb(android::sp<ULogTimeBomb> &timeBomb);
    void unregisterTimeBomb(ULogTimeBomb *timeBomb);

private:
    struct RequestRecord {
        ModuleId moduleId;
        RequestTypeId requestType;
        RequestSerial requestSerial;
        int warnMs;
        timespec enterTimeStamp;

        RequestRecord() : moduleId(0), requestType(REQ_INVALID_ID), requestSerial(0), warnMs(0), enterTimeStamp{0, 0} { }

        void set(ModuleId _moduleId, RequestTypeId _requestType, RequestSerial _requestSerial, timespec _enterTimeStamp, int _warnMs) {
            moduleId = _moduleId;
            requestType = _requestType;
            requestSerial = _requestSerial;
            warnMs = _warnMs;
            enterTimeStamp = _enterTimeStamp;
        }
    };

    class RequestQueue {
    public:
        static constexpr size_t MAX_REQUEST_NUM = 64;

        RequestQueue();

        bool isFull() const {
            return mUnused.empty();
        }

        bool isEmpty() const {
            return mInFlight.empty();
        }

        template <typename ... _T>
        void emplace_back(_T&& ... args);

        template <typename _F>
        bool remove(_F &&match, timespec &enterTimeStamp);

        template <typename _M, typename _F>
        void forAll(_M &&match, _F &&oper);

        RequestRecord &front() {
            return *(mInFlight.front());
        }

        void pop_front();

        size_t size() const {
            return mInFlight.size();
        }

    private:
        RequestRecord mRequestSlots[MAX_REQUEST_NUM];
        std::vector<RequestRecord*> mUnused;
        std::deque<RequestRecord*> mInFlight;
    };

    struct CpuLoading {
        long runningTime;
        long idleTime;

        CpuLoading() : runningTime(0), idleTime(0) { }
        CpuLoading &operator=(const CpuLoading&) = default;
    };

    static ULogGuardMonitor sSingleton;

    std::mutex mMutex;
    std::vector<ThreadGuardData *> mGuardData;
    std::thread mThread;
    std::condition_variable mCond;
    std::condition_variable mFinalCond;
    bool mContinueRunning;
    std::atomic_int mGuardNumber;
    std::atomic_bool mIsReqGuardEnabled;
    std::atomic_bool mIsDumpCompleted;
    RequestQueue mAppRequests;
    int mCompelledDelay;
    std::list<android::wp<ULogTimeBomb>> mTimeBombs;
    std::thread mFinalizer;
    std::list<android::wp<ULogGuard::IFinalizer>> mFinalizers;

    ULogGuardMonitor();
    ~ULogGuardMonitor();

    void start();
    void stop();
    void run();
    void finalize();
    void incGuardNumber();
    void decGuardNumber();
    void registerReqGuard(ModuleId moduleId, RequestTypeId requestType, RequestSerial requestSerial, int warnMs);
    void modifyReqGuard(ModuleId moduleId, RequestTypeId requestType, RequestSerial requestSerial, int warnMs);
    void unregisterReqGuard(ModuleId moduleId, RequestTypeId requestType, RequestSerial requestSerial);
    static bool printToFileLog(const char *str, const char *&next);
    static bool printToAndroidLog(const char *str, const char *&next);
    static void printLongStringToLog(const std::string &str);
    static void printBacktracesOfProcess(std::string &bt);
    static void printBacktracesOfProcess();
    static void printBacktraceOfThread(int tid);
    static void onTimeout(ModuleId moduleId, int guardTid, const char *guardMessage, int elapsedMs, std::string &bt);
    void registerFinalizer(android::sp<ULogGuard::IFinalizer> &finalizer);

    static bool getCpuLoading(CpuLoading &loading);
    static long getCpuLoadingPercentage(const CpuLoading &prev, const CpuLoading &curr);
};


}
}
}

#endif

