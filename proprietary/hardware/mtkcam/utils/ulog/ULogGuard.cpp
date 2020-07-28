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

#define LOG_TAG "ULogGuard"
#include <cstring>
#include <vector>
#include <string>
#include <algorithm>
#include <regex>
#include <signal.h>
#include <unistd.h>
#if MTKCAM_HAVE_AEE_FEATURE == 1
#include <aee.h>
#endif
#include <sys/types.h>
#include <sys/prctl.h>
#include <utils/AndroidThreads.h>
#include <cutils/properties.h>
#include <libladder.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/debug/debug.h>
#include "ULogInt.h"
#include "ULogTable.h"
#include "ULogGuard.h"
#include "ULogRTDiag.h"


CAM_ULOG_DECLARE_MODULE_ID(MOD_UNIFIED_LOG);


#define ULOG_GUARD_ENABLE_SONAME_DISPATCH

// 0x1: function; 0x2: request; 0x4: time bomb
static const char * const ULOG_GUARD_RESET_PROP_NAME = "vendor.debug.camera.ulog.guardreset";
#ifdef ULOG_GUARD_PLATFORM_DEFAULT_RESET_FLAGS
static const unsigned int ULOG_GUARD_DEFAULT_RESET_FLAGS = ULOG_GUARD_PLATFORM_DEFAULT_RESET_FLAGS;
#else
static const unsigned int ULOG_GUARD_DEFAULT_RESET_FLAGS = 0x7;
#endif

// Abort time to overwrite the default value
// Usually used to catch performance issue
static const char * const ULOG_GUARD_ABORTMS_PROP_NAME = "vendor.debug.camera.ulog.abortms";
static const char * const ULOG_GUARD_REQ_WARNMS_PROP_NAME = "vendor.debug.camera.ulog.rwarnms";
static const char * const ULOG_GUARD_INTERVALMS_PROP_NAME = "vendor.debug.camera.ulog.intms";


static const int ULOG_GUARD_DEFAULT_FUNC_WARNING_MSEC = 10000;
static const int ULOG_GUARD_DEFAULT_FUNC_TRACE_MSEC = 16000;
static const int ULOG_GUARD_DEFAULT_FUNC_ABORT_MSEC = 60000;
static const int ULOG_GUARD_DEFAULT_REQ_WARNING_MSEC = 30000;
static const int ULOG_GUARD_DEFAULT_REQ_ABORT_MSEC = 60000;
static const int ULOG_GUARD_MAX_COMPELLED_DELAY_MSEC = 120000;
static const int ULOG_GUARD_TIMEOUT_MAX_MSEC = ULOG_GUARD_DEFAULT_REQ_ABORT_MSEC + ULOG_GUARD_MAX_COMPELLED_DELAY_MSEC;
static const int ULOG_GUARD_FINALIZER_TOLERANCE = 3000;
static const int ULOG_GUARD_DEFAULT_INTERVAL_MSEC = 1000;


#if MTKCAM_HAVE_AEE_FEATURE == 1
static const char * const ULOG_AEE_NAME = "Camera/ULogGuard";
static const unsigned int ULOG_AEE_DB_FLAGS =
    DB_OPT_NE_JBT_TRACES | DB_OPT_PROCESS_COREDUMP | DB_OPT_PROC_MEM | DB_OPT_PID_SMAPS |
    DB_OPT_LOW_MEMORY_KILLER | DB_OPT_DUMPSYS_PROCSTATS | DB_OPT_FTRACE;
#endif


using namespace android;


namespace NSCam {
namespace Utils {
namespace ULog {

inline bool getFirstMatch(
    const std::regex &regObj,
    std::string::const_iterator cbegin, std::string::const_iterator cend, std::string &outName)
{
    std::smatch match;
    if (std::regex_search(cbegin, cend, match, regObj) &&
        match.ready() && match.size() > 1)
    {
        outName = match[1].str();
        return true;
    }

    return false;
}


bool getTopCamModuleName(int tid, const std::string &backtraces, std::string &outName)
{
    char tidPattern[64];

    snprintf(tidPattern, sizeof(tidPattern), "tid(%d)", tid);
    size_t begin = backtraces.find(tidPattern);
    if (begin == std::string::npos)
        return false;

    size_t end = backtraces.find("pid(", begin);
    if (end == std::string::npos)
        end = backtraces.size();

    std::regex nodeName("(\\w+Node)(\\w+)?::\\w+\\(");
    if (getFirstMatch(nodeName, backtraces.cbegin() + begin, backtraces.cbegin() + end, outName))
        return true;

#ifdef ULOG_GUARD_ENABLE_SONAME_DISPATCH
    std::regex soName("/([\\w\\.]+?(cam|3a)[\\w\\.]+?\\.so)[^\\w\\.]");
    if (getFirstMatch(soName, backtraces.cbegin() + begin, backtraces.cbegin() + end, outName))
        return true;
#endif

    return false;
}

}
}
}


// coverity[+kill]
__attribute__((noinline))
extern "C"
void FUNCTION_TIMEOUT_PleaseCheckLogToFindOwner(
    const NSCam::Utils::ULog::ThreadGuardFrame *frame,
    const std::string &backtraces)
{
    (void)frame;
    (void)backtraces;

#if MTKCAM_HAVE_AEE_FEATURE == 1
    if (frame->moduleId == NSCam::Utils::ULog::MOD_CAMERA_DEVICE) {
        const char *moduleName = NSCam::Utils::ULog::getModuleName(frame->moduleId);
        std::string suspectModuleName;
        if (NSCam::Utils::ULog::getTopCamModuleName(frame->tid, backtraces, suspectModuleName)) {
            aee_system_exception(
                ULOG_AEE_NAME,
                NULL,
                ULOG_AEE_DB_FLAGS,
                "%s/%s TIMEOUT\nCRDISPATCH_KEY:%s function TIMEOUT, key module: %s",
                moduleName,
                frame->funcName,
                moduleName,
                suspectModuleName.c_str());
        } else {
            aee_system_exception(
                ULOG_AEE_NAME,
                NULL,
                ULOG_AEE_DB_FLAGS,
                "%s/%s TIMEOUT",
                moduleName,
                frame->funcName);
        }
    } else {
        const char *moduleName = NSCam::Utils::ULog::getModuleName(frame->moduleId);
        if (frame->dispatchKey != nullptr) {
            aee_system_exception(
                ULOG_AEE_NAME,
                NULL,
                ULOG_AEE_DB_FLAGS,
                "%s/%s TIMEOUT\nCRDISPATCH_KEY:%s",
                moduleName,
                frame->funcName,
                frame->dispatchKey);
        } else {
            aee_system_exception(
                ULOG_AEE_NAME,
                NULL,
                ULOG_AEE_DB_FLAGS,
                "%s/%s TIMEOUT\nCRDISPATCH_KEY:%s function TIMEOUT",
                moduleName,
                frame->funcName,
                moduleName);
        }
    }
#else
    raise(SIGABRT);
#endif

    raise(SIGKILL); // make sure, KILL can not be caught
}


// coverity[+kill]
__attribute__((noinline))
extern "C"
void REQUEST_TIMEOUT_PleaseCheckLogToFindOwner(NSCam::Utils::ULog::ModuleId suspect)
{
    (void)suspect;

#if MTKCAM_HAVE_AEE_FEATURE == 1
    if (suspect != 0) {
        aee_system_exception(
            ULOG_AEE_NAME,
            NULL,
            ULOG_AEE_DB_FLAGS,
            "Request TIMEOUT\nCRDISPATCH_KEY:Camera request TIMEOUT, key module: %s",
            NSCam::Utils::ULog::getModuleName(suspect));
    } else {
        aee_system_exception(
            ULOG_AEE_NAME,
            NULL,
            ULOG_AEE_DB_FLAGS,
            "Request TIMEOUT");
    }
#else
    raise(SIGABRT);
#endif

    raise(SIGKILL); // make sure, KILL can not be caught
}


// coverity[+kill]
__attribute__((noinline))
extern "C"
void TIMEBOMB_TIMEOUT_PleaseCheckLogToFindOwner(
    NSCam::Utils::ULog::ModuleId moduleId, const char *dispatchKey)
{
    (void)moduleId;
    (void)dispatchKey;

#if MTKCAM_HAVE_AEE_FEATURE == 1
    aee_system_exception(
        ULOG_AEE_NAME,
        NULL,
        ULOG_AEE_DB_FLAGS,
        "%s TimeBomb TIMEOUT\nCRDISPATCH_KEY:%s",
        NSCam::Utils::ULog::getModuleName(moduleId), dispatchKey);
#else
    raise(SIGABRT);
#endif

    raise(SIGKILL); // make sure, KILL can not be caught
}



namespace NSCam {
namespace Utils {
namespace ULog {

// MUST be monotonic
const clockid_t ULOG_GUARD_CLOCK_ID = CLOCK_MONOTONIC;


inline ThreadGuardData::ThreadGuardData() : dataMutex()
{
    FRONT_GUARD = MEM_GUARD_VALUE;
    tid = gettid();
    depth = 0;
    status = 0;
    REAR_GUARD = MEM_GUARD_VALUE;
    ULogGuardMonitor::getSingleton().registerThread(this);
}


inline ThreadGuardData::~ThreadGuardData()
{
    // Since it is thread-local data, register/unregister will be the same thread
    ULogGuardMonitor::getSingleton().unregisterThread(this);
    FRONT_GUARD = 0xcccccccc;
    REAR_GUARD = 0xcccccccc;
}


ULogGuardMonitor::RequestQueue::RequestQueue()
{
    mUnused.reserve(MAX_REQUEST_NUM);
    for (size_t i = 0; i < MAX_REQUEST_NUM; i++)
        mUnused.push_back(&mRequestSlots[i]);
}


template <typename _F>
bool ULogGuardMonitor::RequestQueue::remove(_F &&match, timespec &enterTimeStamp)
{
    // Ideally, request is FIFO, it will be efficieny if search from begin.
    // However, the serial of AppRequest may be duplicated, we must search
    // from end
    int i = static_cast<int>(mInFlight.size() - 1);
    for ( ; i >= 0; i--) {
        if (match(*mInFlight[i]))
            break;
    }

    if (i >= 0) {
        enterTimeStamp = mInFlight[i]->enterTimeStamp;
        mUnused.push_back(mInFlight[i]);
        mInFlight.erase(mInFlight.begin() + i);
        return true;
    }

    return false;
}


template <typename _M, typename _F>
void ULogGuardMonitor::RequestQueue::forAll(_M &&match, _F &&oper)
{
    for (RequestRecord *record : mInFlight) {
        if (match(*record))
            if (!oper(*record))
                break;
    }
}


template <typename ... _T>
inline void ULogGuardMonitor::RequestQueue::emplace_back(_T&& ... args)
{
    RequestRecord *slot = mUnused.back();
    mUnused.pop_back();
    slot->set(std::forward<_T>(args) ...);
    mInFlight.push_back(slot);
}


inline void ULogGuardMonitor::RequestQueue::pop_front()
{
    mUnused.push_back(mInFlight.front());
    mInFlight.pop_front();
}


ULogGuardMonitor::ULogGuardMonitor() :
    mContinueRunning(false), mGuardNumber(0), mIsReqGuardEnabled(false), mIsDumpCompleted(false),
    mCompelledDelay(0)
{
    mGuardData.reserve(32);
}


ULogGuardMonitor::~ULogGuardMonitor()
{
    stop();
}


void ULogGuardMonitor::start()
{
    mContinueRunning = true;
    mThread = std::thread(&ULogGuardMonitor::run, this);
}


void ULogGuardMonitor::stop()
{
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mContinueRunning = false;
        mCond.notify_all();
    }

    if (mThread.joinable()) {
        mThread.join();
    }
}


inline void ULogGuardMonitor::registerThread(ThreadGuardData *guardData)
{
    std::lock_guard<std::mutex> lock(mMutex);
    mGuardData.push_back(guardData);
}


inline void ULogGuardMonitor::unregisterThread(ThreadGuardData *guardData)
{
    std::lock_guard<std::mutex> lock(mMutex);
    for (auto iter = mGuardData.begin(); iter != mGuardData.end(); iter++) {
        if (*iter == guardData) {
            mGuardData.erase(iter);
            break;
        }
    }
}


inline void ULogGuardMonitor::incGuardNumber()
{
    if (mGuardNumber.fetch_add(1, std::memory_order_relaxed) == 0) {
        // We can obtain mMutex only if the guard thread is waiting
        // the mCond.notify_all() will always waken the waiting
        // So it is safe that not putting mGuardNumber in the critical section
        std::lock_guard<std::mutex> lock(mMutex);
        mCond.notify_all();
    }
}


inline void ULogGuardMonitor::decGuardNumber()
{
    // coverity[side_effect_free : FALSE]
    mGuardNumber.fetch_sub(1, std::memory_order_relaxed);
}


void ULogGuardMonitor::registerReqGuard(ModuleId moduleId, RequestTypeId requestType, RequestSerial requestSerial, int warnMs)
{
    if (!mIsReqGuardEnabled.load(std::memory_order_relaxed))
        return;

    if (__unlikely(requestType != REQ_APP_REQUEST))
        return;

    bool toIncGuardNumber = false;

    {
        std::lock_guard<std::mutex> lock(mMutex);
        if (__likely(!mAppRequests.isFull())) {
            timespec enterTimeStamp;
            clock_gettime(ULOG_GUARD_CLOCK_ID, &(enterTimeStamp));
            mAppRequests.emplace_back(moduleId, requestType, requestSerial, enterTimeStamp, warnMs);
            toIncGuardNumber = true;
        }
    }

    if (toIncGuardNumber) {
        incGuardNumber();
    }
}


void ULogGuardMonitor::modifyReqGuard(ModuleId , RequestTypeId requestType, RequestSerial requestSerial, int warnMs)
{
    if (__unlikely(requestType != REQ_APP_REQUEST))
        return;

    std::lock_guard<std::mutex> lock(mMutex);

    mAppRequests.forAll(
        [requestSerial] (const RequestRecord &record) {
            return record.requestSerial == requestSerial;
        },
        [warnMs] (RequestRecord &record) {
            record.warnMs = warnMs;
            return false;
        }
    );
}


void ULogGuardMonitor::unregisterReqGuard(ModuleId moduleId, RequestTypeId requestType, RequestSerial requestSerial)
{
    if (!mIsReqGuardEnabled.load(std::memory_order_relaxed))
        return;

    if (__unlikely(requestType != REQ_APP_REQUEST))
        return;

    auto isTheSameRequest = [moduleId, requestType, requestSerial] (const RequestRecord &record) {
        return (record.moduleId == moduleId && record.requestType == requestType && record.requestSerial == requestSerial);
    };

    timespec exitTime;
    clock_gettime(ULOG_GUARD_CLOCK_ID, &exitTime);

    std::lock_guard<std::mutex> lock(mMutex);
    timespec enterTime;
    if (__likely(mAppRequests.remove(isTheSameRequest, enterTime))) {

        mCompelledDelay = timeDiffMs(enterTime, exitTime);
        if (mCompelledDelay < 1000)
            mCompelledDelay = 0;
        else if (mCompelledDelay > ULOG_GUARD_MAX_COMPELLED_DELAY_MSEC)
            mCompelledDelay = ULOG_GUARD_MAX_COMPELLED_DELAY_MSEC;

        decGuardNumber();
    }
}


void ULogGuardMonitor::registerTimeBomb(android::sp<ULogTimeBomb> &timeBomb)
{
    bool toIncGuardNumber = false;

    {
        std::lock_guard<std::mutex> lock(mMutex);
        int numOfTimeBombs = static_cast<int>(mTimeBombs.size());
        if (__likely(numOfTimeBombs < 128)) {
            mTimeBombs.emplace_back(timeBomb);
            toIncGuardNumber = true;
        } else {
            ModuleId dominatedModule = 0;
            int count = 0;
            for (auto iter = mTimeBombs.begin(); iter != mTimeBombs.end(); iter++) {
                sp<ULogTimeBomb> iBomb = iter->promote();
                if (iBomb != NULL) {
                    if (count <= 0) {
                        dominatedModule = iBomb->getModuleId();
                        count = 1;
                    } else if (dominatedModule == iBomb->getModuleId()) {
                        count++;
                        if (count * 2 > numOfTimeBombs)
                            break;
                    } else {
                        count--;
                    }
                }
            }

            if (count <= 0) {
                CAM_ULOGMI("Too many time bombs. Refuse to create new one.");
            } else {
                CAM_ULOGMI("Too many time bombs. Refuse to create new one. Dominated module = M[%s:%x]",
                    getModuleName(dominatedModule), dominatedModule);
            }
        }
    }

    if (toIncGuardNumber) {
        incGuardNumber();
    }
}


void ULogGuardMonitor::unregisterTimeBomb(ULogTimeBomb *timeBomb)
{
    bool removed = false;

    {
        std::lock_guard<std::mutex> lock(mMutex);
        for (auto iter = mTimeBombs.begin(); iter != mTimeBombs.end(); iter++) {
            // NOTE: we are in the onLastStrongRef(), should not call promote()
            if (iter->unsafe_get() == timeBomb) {
                mTimeBombs.erase(iter);
                removed = true;
                break;
            }
        }
    }

    if (removed) {
        decGuardNumber();
    }
}


void ULogGuardMonitor::registerFinalizer(android::sp<ULogGuard::IFinalizer> &finalizer)
{
    std::lock_guard<std::mutex> lock(mMutex);

    // Clean garbage
    auto it = mFinalizers.begin();
    while (it != mFinalizers.end()) {
        if (it->promote() == NULL) {
            it = mFinalizers.erase(it);
        } else {
            it++;
        }
    }

    mFinalizers.emplace_back(finalizer);
}


inline bool ULogGuardMonitor::getCpuLoading(CpuLoading &loading)
{
    if (ULogger::isModeEnabled(ULogger::TEXT_FILE_LOG | ULogger::PASSIVE_LOG)) {
        // We have permission only if "setenforce 0"
        int statFd = open("/proc/stat", O_RDONLY);

        if (statFd >= 0) {
            char buffer[100];
            char cpu[10];

            loading.runningTime = 0;
            loading.idleTime = 0;

            int n = read(statFd, buffer, sizeof(buffer));
            if (n > 10) {
                long user, nice, system, idle, iowait, irq, softirq;
                if (sscanf(buffer, "%s %ld %ld %ld %ld %ld %ld %ld", cpu, &user, &nice, &system, &idle, &iowait, &irq, &softirq) == 8) {
                    loading.runningTime = user + nice + system + iowait + irq + softirq;
                    loading.idleTime = idle;
                }
            }

            close(statFd);

            return true;
        }
    }

    loading.runningTime = 0;
    loading.idleTime = 0;

    return false;
}


inline long ULogGuardMonitor::getCpuLoadingPercentage(const CpuLoading &prev, const CpuLoading &curr)
{
    long totalRunningDiff = curr.runningTime - prev.runningTime;
    long totalTimeDiff = totalRunningDiff + (curr.idleTime - prev.idleTime);
    if (totalTimeDiff <= 0)
        return -1;
    return (totalRunningDiff * 100 + 49) / totalTimeDiff;
}


void ULogGuardMonitor::run()
{
    androidSetThreadName("ULogGuardMonitor");
    androidSetThreadPriority(getTid(), android::PRIORITY_BACKGROUND);

    int resetFlag = property_get_int32(ULOG_GUARD_RESET_PROP_NAME, ULOG_GUARD_DEFAULT_RESET_FLAGS);

    const bool funcResetEnabled = (resetFlag & 0x1) != 0;
    const bool reqResetEnabled = (resetFlag & 0x2) != 0;
    const bool bombResetEnabled = (resetFlag & 0x4) != 0;
    if (reqResetEnabled) {
        mIsReqGuardEnabled.store(true, std::memory_order_relaxed);
    }

    const int abortMsOverwritten = property_get_int32(ULOG_GUARD_ABORTMS_PROP_NAME, 0);
    const int funcAbortMs = abortMsOverwritten > 0 ? abortMsOverwritten : ULOG_GUARD_DEFAULT_FUNC_ABORT_MSEC;
    const int reqAbortMs = abortMsOverwritten > 0 ? abortMsOverwritten : ULOG_GUARD_DEFAULT_REQ_ABORT_MSEC;
    const int reqWarnMs = property_get_int32(ULOG_GUARD_REQ_WARNMS_PROP_NAME, ULOG_GUARD_DEFAULT_REQ_WARNING_MSEC);
    const int intervalMs = std::max(
        property_get_int32(ULOG_GUARD_INTERVALMS_PROP_NAME, ULOG_GUARD_DEFAULT_INTERVAL_MSEC), 5);

    std::atomic_thread_fence(std::memory_order_release);

    ULOG_IMP_LOGI("Running: func(%d) = %d,%d; req(%d) = %d,%d; time-bomb(%d). Patrol every %d ms",
        (funcResetEnabled ? 1 : 0),
        ULOG_GUARD_DEFAULT_FUNC_WARNING_MSEC, funcAbortMs,
        (reqResetEnabled ? 1 : 0),
        reqWarnMs, reqAbortMs,
        (bombResetEnabled ? 1 : 0), intervalMs);

    std::unique_lock<std::mutex> lock(mMutex);
    CpuLoading previousLoading;
    getCpuLoading(previousLoading);
    bool detectCpuLoading = false;
    static constexpr RequestSerial INVALID_SERIAL = 0xfffffff;
    RequestSerial warnedAppRequest = INVALID_SERIAL;
    int tick = 0;

    auto invokeFinalizers = [&lock, this] {
        mFinalizer = std::thread(&ULogGuardMonitor::finalize, this);
        mFinalCond.wait_for(lock, std::chrono::milliseconds(ULOG_GUARD_FINALIZER_TOLERANCE));
        ULogger::flush(10);
        if (!mIsDumpCompleted.load(std::memory_order_acquire)) {
            ULOG_IMP_LOGE("Dump did not complete in %d ms, abort anyway", ULOG_GUARD_FINALIZER_TOLERANCE);
        }
    };

    timespec showAlive;
    clock_gettime(ULOG_GUARD_CLOCK_ID, &showAlive);
    std::string backtraces;
    ULogVector<int> problematicTids;

    while (mContinueRunning) {
        tick++;
        mCond.wait_for(lock, std::chrono::milliseconds(intervalMs));

        if (!mContinueRunning)
            break;

        if (mGuardNumber.load(std::memory_order_relaxed) == 0) {
            mCond.wait(lock);
            if (!mContinueRunning)
                break;
            continue;
        }

        long loadingPercentage = -1;
        CpuLoading currentLoading;
        if (detectCpuLoading) {
            // getCpuLoading has relatively large overhead, we try to avoid if unnecessary
            if (getCpuLoading(currentLoading))
                loadingPercentage = getCpuLoadingPercentage(previousLoading, currentLoading);
        }

        int maxAbortElapsedMs = 0;
        int timeoutElapsedMs = 0;
        const ThreadGuardData *timeoutGuardData = nullptr;
        const ThreadGuardFrame *timeoutFrame = nullptr;

        detectCpuLoading = false;
        timespec now;
        clock_gettime(ULOG_GUARD_CLOCK_ID, &now);

        // ----- Function guard ------
        problematicTids.clear();
        for (ThreadGuardData *guardData : mGuardData) {
            if (guardData->FRONT_GUARD != ThreadGuardData::MEM_GUARD_VALUE) {
                // The guard data was corrupted, we skip it
                continue;
            }

            std::lock_guard<std::mutex> dataLock(guardData->dataMutex);
            if (guardData->depth > 0) {
                int threadMinToAbort = ULOG_GUARD_TIMEOUT_MAX_MSEC;

                int f = ThreadGuardData::MAX_FRAMES;
                f = std::min(f, guardData->depth) - 1; // Well, std::min() doesn't work for constexpr
                for ( ; f >= 0; f--) {
                    const ThreadGuardFrame *frame = &(guardData->frame[f]);
                    int elapsedMs = timeDiffMs(frame->timeStamp, now);
                    int warnMs = (frame->warnMs > 0) ? frame->warnMs : ULOG_GUARD_DEFAULT_FUNC_WARNING_MSEC;
                    int abortMs = (frame->abortMs > 0) ? frame->abortMs : funcAbortMs;
                    if (funcResetEnabled && elapsedMs > abortMs) {
                        // Pick the max elapsed to abort
                        int abortElapsedMs = elapsedMs - abortMs;
                        if (abortElapsedMs > maxAbortElapsedMs) {
                            maxAbortElapsedMs = abortElapsedMs;
                            timeoutElapsedMs = elapsedMs;
                            timeoutGuardData = guardData;
                            timeoutFrame = frame;
                        }
                    } else if (elapsedMs > warnMs) {
                        // if funcResetEnabled is false, frameToAbort may be negative, but it's OK
                        int frameToAbort = abortMs - elapsedMs;
                        if (frameToAbort < threadMinToAbort || (tick & 0x3) == 0) { // Avoid print too many logs per thread
                            CAM_ULOGMW(
                                "Thread tid=%d %s executed %d ms(>%d) TOO LONG M[%s:%x]; frameData = %p; CPU loading = %ld%%",
                                guardData->tid, frame->longFuncName, elapsedMs, warnMs,
                                getModuleName(frame->moduleId), frame->moduleId,
                                frame->frameData, loadingPercentage);
                            if (!(guardData->status & ThreadGuardData::BACKTRACE_PRINTED)) {
                                problematicTids.push_back(guardData->tid);
                                guardData->status |= ThreadGuardData::BACKTRACE_PRINTED;
                            }
                        }

                        if (frameToAbort < threadMinToAbort)
                            threadMinToAbort = frameToAbort;
                    }

                    if (elapsedMs >= ULOG_GUARD_DEFAULT_FUNC_WARNING_MSEC - 1500) {
                        detectCpuLoading = true;
                    }
                }
            }
        }

        if (funcResetEnabled && timeoutElapsedMs > 0 && timeoutGuardData != nullptr) {
            ThreadGuardFrame clonedFrame;
            bool isFrameStillValid = false;
            {
                std::lock_guard<std::mutex> dataLock(timeoutGuardData->dataMutex);
                // We have out of the mutex, confirm again
                int abortMs = (timeoutFrame->abortMs > 0) ? timeoutFrame->abortMs : funcAbortMs;
                int elapsedMs = timeDiffMs(timeoutFrame->timeStamp, now);
                if (elapsedMs > abortMs) { // frame data was not overwritten
                    // The frame may already exit during the unlock period
                    // But we don't care, since it is already timeout
                    // We can continue to abort if the data is still valid
                    clonedFrame = *timeoutFrame;
                    isFrameStillValid = true;
                }
            }

            if (isFrameStillValid) {
                lock.unlock();
                onTimeout(clonedFrame.moduleId, clonedFrame.tid,
                    clonedFrame.longFuncName, timeoutElapsedMs, backtraces);
                lock.lock();

                invokeFinalizers();

                FUNCTION_TIMEOUT_PleaseCheckLogToFindOwner(&clonedFrame, backtraces);
            } else {
                CAM_ULOGMI("Thread tid=%d: the timeout frame updated", timeoutGuardData->tid);
            }
        } else if (problematicTids.size() > 0) {
            lock.unlock();
            for (int tid : problematicTids)
                printBacktraceOfThread(tid);
            lock.lock();
        }

        // ----- Request guard ------
        if (!mAppRequests.isEmpty()) {
            const RequestRecord &oldest = mAppRequests.front();
            int elapsedMs = timeDiffMs(oldest.enterTimeStamp, now);
            int warnMs = ((oldest.warnMs > 0) ? oldest.warnMs : reqWarnMs) + mCompelledDelay;
            int abortMs = std::max(reqAbortMs, oldest.warnMs) + mCompelledDelay;
            if (reqResetEnabled && elapsedMs > abortMs) {
                CAM_ULOGME("R %s:%u executed %d ms(>%d) TIMEOUT; CPU loading = %ld%% FORCE STOP",
                    getRequestTypeName(oldest.requestType), oldest.requestSerial,
                    elapsedMs, abortMs, loadingPercentage);

                ModuleId suspect = 0;
                if (ULogRuntimeDiag::isEnabled()) {
                    suspect = NSCam::Utils::ULog::ULogRTDiagImpl::get().timeoutPickSuspect(
                        oldest.requestSerial, elapsedMs);
                }

                char message[256];
                if (suspect != 0) {
                    snprintf(message, sizeof(message), "R %s:%u NO EXIT(suspect: M[%s:%x])",
                        getRequestTypeName(oldest.requestType), oldest.requestSerial,
                        getModuleName(suspect), suspect);
                } else {
                    snprintf(message, sizeof(message), "R %s:%u NO EXIT",
                        getRequestTypeName(oldest.requestType), oldest.requestSerial);
                }

                lock.unlock(); // print backtrace will take long time
                onTimeout(oldest.moduleId, 0, message, elapsedMs, backtraces);
                lock.lock();

                invokeFinalizers();

                REQUEST_TIMEOUT_PleaseCheckLogToFindOwner(suspect);
            } else if (elapsedMs > warnMs) {
                if (!reqResetEnabled && elapsedMs > abortMs) {
                    CAM_ULOGME("R %s:%u executed %d ms(>%d) TOO LONG; CPU loading = %ld%% STOP WARNING",
                        getRequestTypeName(oldest.requestType), oldest.requestSerial,
                        elapsedMs, warnMs, loadingPercentage);
                    mAppRequests.pop_front();
                } else {
                    CAM_ULOGMW("R %s:%u executed %d ms(>%d) TOO LONG; compelledDelay = %d ms; CPU loading = %ld%%",
                        getRequestTypeName(oldest.requestType), oldest.requestSerial,
                        elapsedMs, warnMs, mCompelledDelay, loadingPercentage);
                }
                if (warnedAppRequest != oldest.requestSerial) {
                    if (ULogRuntimeDiag::isEnabled()) {
                        // Print suspects
                        NSCam::Utils::ULog::ULogRTDiagImpl::get().timeoutPickSuspect(
                            oldest.requestSerial, elapsedMs);
                    }
                    warnedAppRequest = oldest.requestSerial;
                }
            } else {
                warnedAppRequest = INVALID_SERIAL;
            }

            if (elapsedMs >= ULOG_GUARD_DEFAULT_REQ_WARNING_MSEC - 2500) {
                // did not refer to the property because of the overhead
                detectCpuLoading = true;
            }
        }

        // ----- Time bomb -----
        if (!mTimeBombs.empty()) {
            for (auto timeBombIt = mTimeBombs.begin(); timeBombIt != mTimeBombs.end(); ) {
                sp<ULogTimeBomb> timeBomb = timeBombIt->promote();
                if (timeBomb == NULL || !timeBomb->isValid()) {
                    timeBombIt = mTimeBombs.erase(timeBombIt);
                } else {
                    int numOfWarnPrinted = 0;
                    int minTimeToAbort = ULOG_GUARD_TIMEOUT_MAX_MSEC;

                    int elapsedMs = timeDiffMs(timeBomb->getTimeStamp(), now);
                    ModuleId moduleId = timeBomb->getModuleId();
                    const char *dispatchKey = timeBomb->getDispatchKey();
                    int abortMs = timeBomb->getAbortMs();
                    int warnMs = timeBomb->getWarnMs();

                    if (bombResetEnabled &&
                        elapsedMs >= abortMs)
                    {
                        char message[ULogTimeBombVerbose::ID_STRING_SIZE + 256];
                        message[0] = '\0';
                        if (timeBomb->getType() == ULogTimeBomb::TYPE_SERIAL) {
                            ULogTimeBombSerial *timeBombOri = static_cast<ULogTimeBombSerial*>(timeBomb.get());
                            CAM_ULOGME("TimeBomb %u of M[%s:%x] executed %d ms(>%d) TIMEOUT, dispatch key = %s",
                                timeBombOri->getSerial(), getModuleName(moduleId), moduleId,
                                elapsedMs, abortMs, dispatchKey);
                            snprintf(message, sizeof(message), "TimeBomb %u of M[%s:%x]",
                                timeBombOri->getSerial(), getModuleName(moduleId), moduleId);
                        } else if (timeBomb->getType() == ULogTimeBomb::TYPE_VERBOSE) {
                            ULogTimeBombVerbose *timeBombOri = static_cast<ULogTimeBombVerbose*>(timeBomb.get());
                            CAM_ULOGME("TimeBomb [%s] of M[%s:%x] executed %d ms(>%d) TIMEOUT, dispatch key = %s",
                                timeBombOri->getIdString(), getModuleName(moduleId), moduleId,
                                elapsedMs, abortMs, dispatchKey);
                            snprintf(message, sizeof(message), "TimeBomb [%s] of M[%s:%x]",
                                timeBombOri->getIdString(), getModuleName(moduleId), moduleId);
                        }

                        lock.unlock(); // print backtrace will take long time
                        onTimeout(moduleId, 0, message, elapsedMs, backtraces);
                        lock.lock();

                        invokeFinalizers();

                        TIMEBOMB_TIMEOUT_PleaseCheckLogToFindOwner(moduleId, dispatchKey);
                    } else if (elapsedMs >= warnMs) {
                        int timeToAbort = abortMs - elapsedMs;
                        if (numOfWarnPrinted <= 2 || timeToAbort < minTimeToAbort) { // prevent log too much
                            if (timeBomb->getType() == ULogTimeBomb::TYPE_SERIAL) {
                                ULogTimeBombSerial *timeBombOri = static_cast<ULogTimeBombSerial*>(timeBomb.get());
                                CAM_ULOGMW("TimeBomb %u of M[%s:%x] executed %d ms(>%d) TOO LONG, dispatch key = %s",
                                    timeBombOri->getSerial(), getModuleName(moduleId), moduleId,
                                    elapsedMs, warnMs, dispatchKey);
                            } else if (timeBomb->getType() == ULogTimeBomb::TYPE_VERBOSE) {
                                ULogTimeBombVerbose *timeBombOri = static_cast<ULogTimeBombVerbose*>(timeBomb.get());
                                CAM_ULOGMW("TimeBomb [%s] of M[%s:%x] executed %d ms(>%d) TOO LONG, dispatch key = %s",
                                    timeBombOri->getIdString(), getModuleName(moduleId), moduleId,
                                    elapsedMs, warnMs, dispatchKey);
                            }
                            numOfWarnPrinted++;
                            if (timeToAbort < minTimeToAbort)
                                minTimeToAbort = timeToAbort;
                        }
                    }

                    timeBombIt++;
                }
            }
        }

        if (detectCpuLoading) {
            if (currentLoading.runningTime > 0)
                previousLoading = currentLoading;
            else
                getCpuLoading(previousLoading);
        }

        previousLoading = currentLoading;

        if (now.tv_sec - showAlive.tv_sec >= 3) {
            showAlive = now;
            ULOG_IMP_LOGD("Monitoring: %zu threads, %zu requests, %zu time-bombs; %d guards. ResetFlags = 0x%x",
                mGuardData.size(), mAppRequests.size(), mTimeBombs.size(),
                mGuardNumber.load(std::memory_order_relaxed),
                static_cast<unsigned int>(resetFlag));
        }
    }

    ULOG_IMP_LOGW("Stopped: %zu threads, %zu requests, %zu time-bombs; %d guards in monitor",
        mGuardData.size(), mAppRequests.size(), mTimeBombs.size(),
        mGuardNumber.load(std::memory_order_relaxed));
}


bool ULogGuardMonitor::printToFileLog(const char *str, const char *&next)
{
    if (str[0] == '\0')
        return false;

    char buffer[512];

    size_t newLinePos = 0;
    size_t endIndex = 0;
    for (endIndex = 0; endIndex < sizeof(buffer) - 1; endIndex++)
        if (str[endIndex] == '\n') {
            newLinePos = endIndex;
            break; // PRINT ONCE EVERY NEW LINE
        } else if (str[endIndex] == '\0') {
            break;
        }

    if (endIndex == 0)
        return false;

    if (str[endIndex] == '\0') {
        CAM_ULOGMI_ALWAYS("%s", str);
        next = str + endIndex;
        return false;
    } else if (newLinePos > 0) {
        memcpy(buffer, str, newLinePos);
        buffer[newLinePos] = '\0';
        CAM_ULOGMI_ALWAYS("%s", buffer);
        next = str + newLinePos + 1;
    } else { // endIndex must > 0
        memcpy(buffer, str, endIndex);
        buffer[endIndex] = '\0';
        CAM_ULOGMI_ALWAYS("%s", buffer);
        next = str + endIndex;
    }

    // We guaranteed the "next" will always progress
    return true;
}


bool ULogGuardMonitor::printToAndroidLog(const char *str, const char *&next)
{
    if (str[0] == '\0')
        return false;

    char buffer[512];

    size_t newLinePos = 0;
    size_t endIndex = 0;
    for (endIndex = 0; endIndex < sizeof(buffer) - 1; endIndex++)
        if (str[endIndex] == '\n') {
            newLinePos = endIndex;
        } else if (str[endIndex] == '\0') {
            break;
        }

    if (endIndex == 0)
        return false;

    if (str[endIndex] == '\0') {
        ULOG_IMP_LOGI("%s", str);
        next = str + endIndex;
        return false;
    } else if (newLinePos > 0) {
        memcpy(buffer, str, newLinePos);
        buffer[newLinePos] = '\0';
        ULOG_IMP_LOGI("%s", buffer);
        next = str + newLinePos + 1;
    } else { // endIndex must > 0
        memcpy(buffer, str, endIndex);
        buffer[endIndex] = '\0';
        ULOG_IMP_LOGI("%s", buffer);
        next = str + endIndex;
    }

    // We guaranteed the "next" will always progress
    return true;
}


void ULogGuardMonitor::onTimeout(ModuleId moduleId, int guardTid, const char *guardMessage, int elapsedMs,
    std::string &bt)
{
#if MTKCAM_HAVE_AEE_FEATURE == 1
    aee_switch_ftrace(0);
#endif

    if (guardTid> 0) {
        CAM_ULOGME("TIMEOUT tid=%d %s, executed %d ms. M[%s:%x]",
            guardTid, guardMessage, elapsedMs, getModuleName(moduleId), moduleId);
    } else {
        CAM_ULOGME("TIMEOUT %s, executed %d ms", guardMessage, elapsedMs);
    }

    printBacktracesOfProcess(bt);
}


void ULogGuardMonitor::printLongStringToLog(const std::string &_str)
{
    const char *str = _str.c_str();

    if (ULogger::isFileModeEnabled()) {
        while (printToFileLog(str, str))
            ; // Print until end
    } else {
        while (printToAndroidLog(str, str))
            ; // Print until end
    }
}


void ULogGuardMonitor::printBacktracesOfProcess(std::string &bt)
{
    UnwindCurProcessBT(&bt);
    printLongStringToLog(bt);
}


void ULogGuardMonitor::printBacktracesOfProcess()
{
    std::string bt;
    printBacktracesOfProcess(bt);
}


void ULogGuardMonitor::printBacktraceOfThread(int tid)
{
    std::string bt;
    UnwindThreadBT(static_cast<pid_t>(tid), &bt);
    printLongStringToLog(bt);
}


void ULogGuardMonitor::finalize()
{
    std::list<android::wp<ULogGuard::IFinalizer>> finalizers;

    {
        std::lock_guard<std::mutex> lock(mMutex);
        finalizers = mFinalizers;
    }

    ULOG_IMP_LOGI("Start debuggee dump");

    auto pDbgMgr = IDebuggeeManager::get();
    if (pDbgMgr != nullptr) {
        std::vector<std::string> options;
        pDbgMgr->debug(std::make_shared<ULogPrinter>(MOD_UNIFIED_LOG, LOG_TAG), options);
    }

    ULOG_IMP_LOGI("Run finalizers");

    ULogGuard::Status status;
    for (auto it : finalizers) {
        sp<ULogGuard::IFinalizer> finalizer = it.promote();
        if (finalizer != NULL) {
            finalizer->onTimeout(status);
        }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    mIsDumpCompleted.store(true, std::memory_order_release);

    mFinalCond.notify_all();
}


void ULogGuard::onInit()
{
    ULogGuardMonitor::getSingleton().start();
}


void ULogGuard::onUninit()
{
    ULogGuardMonitor::getSingleton().stop();
}


thread_local std::unique_ptr<ThreadGuardData> sThreadGuardData(nullptr);


void ULogGuard::registerFuncGuard(ModuleId moduleId, const char * /* tag */,
    const char *funcName, const char *longFuncName, int warnMs, int abortMs, const char *dispatchKey, int *outGuardId)
{
    if (__unlikely(sThreadGuardData == nullptr)) {
        sThreadGuardData.reset(new ThreadGuardData);
    }

    ThreadGuardData *guardData = sThreadGuardData.get();
    {
        std::lock_guard<std::mutex> dataLock(guardData->dataMutex);
        // Maybe we can have a lock-free design, but not necessary so far
        if (__likely(guardData->depth < ThreadGuardData::MAX_FRAMES)) {
            ThreadGuardFrame *frame = &(guardData->frame[guardData->depth]);
            frame->warnMs = warnMs;
            frame->abortMs = abortMs;
            frame->moduleId = moduleId;
            frame->tid = guardData->tid;
            clock_gettime(ULOG_GUARD_CLOCK_ID, &(frame->timeStamp));
            frame->funcName = funcName; // We assumed funcName is a literal(verified)
            frame->longFuncName = longFuncName;
            frame->dispatchKey = dispatchKey;
            frame->frameData = outGuardId;
        }
        guardData->depth++;
    }

    ULogGuardMonitor::getSingleton().incGuardNumber();

    *outGuardId = ((guardData->tid << 8) | guardData->depth);
}


void ULogGuard::registerFuncGuard(ModuleId moduleId, const char *tag,
    const char *funcName, const char *longFuncName, int warnMs, int *outGuardId)
{
    registerFuncGuard(moduleId, tag, funcName, longFuncName, warnMs, 0, nullptr, outGuardId);
}


void ULogGuard::unregisterFuncGuard(int /* guardId */)
{
    {
        std::lock_guard<std::mutex> dataLock(sThreadGuardData->dataMutex);
        sThreadGuardData->depth--;
        if (sThreadGuardData->depth == 0)
            sThreadGuardData->status &= (~ThreadGuardData::BACKTRACE_PRINTED);
    }

    ULogGuardMonitor::getSingleton().decGuardNumber();
}


void ULogGuard::registerReqGuard(ModuleId moduleId, RequestTypeId requestType, RequestSerial requestSerial, int warnMs)
{
    ULogGuardMonitor::getSingleton().registerReqGuard(moduleId, requestType, requestSerial, warnMs);
}


void ULogGuard::modifyReqGuard(ModuleId moduleId, RequestTypeId requestType, RequestSerial requestSerial, int warnMs)
{
    ULogGuardMonitor::getSingleton().modifyReqGuard(moduleId, requestType, requestSerial, warnMs);
}


void ULogGuard::unregisterReqGuard(ModuleId moduleId, RequestTypeId requestType, RequestSerial requestSerial)
{
    ULogGuardMonitor::getSingleton().unregisterReqGuard(moduleId, requestType, requestSerial);
}


void ULogGuard::registerFinalizer(android::sp<IFinalizer> finalizer)
{
    ULogGuardMonitor::getSingleton().registerFinalizer(finalizer);
}


ULogTimeBomb::ULogTimeBomb(
        Type type, ModuleId moduleId, const char *dispatchKey,
        int warnMs, int abortMs, int abortMaxBound) :
    mModuleId(moduleId), mAbortMaxBound(abortMaxBound), mWarnMs(warnMs), mAbortMs(abortMs), mType(type)
{
    clock_gettime(ULOG_GUARD_CLOCK_ID, &mTimeStamp);
    strncpy(mDispatchKey, dispatchKey, sizeof(mDispatchKey));
    mDispatchKey[DISPATCH_KEY_LEN_MAX] = '\0';
    mValidGuardPattern = VALID_PATTERN;
}


ULogTimeBomb::~ULogTimeBomb()
{
    mValidGuardPattern = 0xdeaddead;
}


void ULogTimeBomb::registerToGuard(sp<ULogTimeBomb> timeBomb)
{
    if (timeBomb->getAbortMs() > 0)
        ULogGuardMonitor::getSingleton().registerTimeBomb(timeBomb);
    // unregister in ULogTimeBomb::onLastStrongRef()
}


void ULogTimeBomb::onLastStrongRef(const void *)
{
    // unregiserFromGuard
    if (getAbortMs() > 0)
        ULogGuardMonitor::getSingleton().unregisterTimeBomb(this);

    finalize();
}


int ULogTimeBomb::getElapsedMs() const
{
    timespec now;
    clock_gettime(ULOG_GUARD_CLOCK_ID, &now);
    return timeDiffMs(mTimeStamp, now);
}


// Extend the time of warn & abort, will limited by abortMaxBound
void ULogTimeBomb::extendTime(int plusWarnMs, int plusAbortMs)
{
    int abortMaxBound = mAbortMaxBound;
    if (abortMaxBound <= 0 || abortMaxBound > ULOG_GUARD_TIMEOUT_MAX_MSEC)
        abortMaxBound = ULOG_GUARD_TIMEOUT_MAX_MSEC;

    int newWarnMs = mWarnMs.load(std::memory_order_relaxed) + plusWarnMs;
    int newAbortMs = mAbortMs.load(std::memory_order_relaxed) + plusAbortMs;

    if (newWarnMs > abortMaxBound)
        newWarnMs = abortMaxBound;
    if (newAbortMs > abortMaxBound)
        newAbortMs = abortMaxBound;

    mWarnMs.store(newWarnMs, std::memory_order_relaxed);
    mAbortMs.store(newAbortMs, std::memory_order_relaxed);
}


void ULogTimeBombSerial::finalize()
{
    int elapsedMs = getElapsedMs();
    int warnMs = getWarnMs();

    if (elapsedMs >= warnMs) {
        CAM_ULOGMW("TimeBomb %u of M[%s:%x] executed %d ms(>%d) finally, dispatch key = %s",
            getSerial(), getModuleName(getModuleId()), getModuleId(), elapsedMs, warnMs, getDispatchKey());
    }
}


void ULogTimeBombVerbose::finalize()
{
    int elapsedMs = getElapsedMs();
    int warnMs = getWarnMs();

    if (elapsedMs >= warnMs) {
        CAM_ULOGMW("TimeBomb [%s] of M[%s:%x] executed %d ms(>%d) finally, dispatch key = %s",
            getIdString(), getModuleName(getModuleId()), getModuleId(), elapsedMs, warnMs, getDispatchKey());
    }
}


ULogRoutineOvertime::ULogRoutineOvertime(const please_use_CAM_ULOG_TAG_OVERTIME &,
        ModuleId moduleId, const char *logTag, const char *routineTag,
        int warnMs, unsigned int options, std::intptr_t keyValue) :
    mLogTag(logTag), mRoutineTag(routineTag), mKeyValue(keyValue), mModuleId(moduleId), mWarnMs(warnMs),
    mOptions(options & (~DYNAMIC_STRING))
{
    clock_gettime(ULOG_GUARD_CLOCK_ID, &mBeginTime);
}


ULogRoutineOvertime::ULogRoutineOvertime(const please_use_CAM_ULOG_TAG_OVERTIMEF &,
        ModuleId moduleId, const char *logTag, const char *routineTag,
        const char *dynStr, int warnMs, unsigned int options) :
    mLogTag(logTag), mRoutineTag(routineTag), mModuleId(moduleId), mWarnMs(warnMs),
    mOptions(options | DYNAMIC_STRING)
{
    mKeyValue = reinterpret_cast<std::intptr_t>(dynStr);
    clock_gettime(ULOG_GUARD_CLOCK_ID, &mBeginTime);
}


ULogRoutineOvertime::~ULogRoutineOvertime()
{
    timespec now;
    clock_gettime(ULOG_GUARD_CLOCK_ID, &now);
    int elapsedMs = timeDiffMs(mBeginTime, now);

    if (__unlikely(elapsedMs > mWarnMs)) {
        if (NSCam::Utils::ULog::ULogger::isNormalDetailsEnabled(mModuleId, NSCam::Utils::ULog::DETAILS_WARNING)) {
            if (NSCam::Utils::ULog::ULogger::isAndroidModeEnabled()) {
                if (mOptions & DYNAMIC_STRING) {
                    char *dynStr = reinterpret_cast<char*>(mKeyValue);
                    ULOG_FW_LOGW(mLogTag, "[%s] executed %d ms(>%d) TOO LONG: %s",
                        mRoutineTag, elapsedMs, mWarnMs, dynStr);
                } else {
                    if (mKeyValue != NO_KEY_VALUE) {
                        ULOG_FW_LOGW(mLogTag, "[%s] key = %" PRIdPTR "(0x%" PRIxPTR ") executed %d ms(>%d) TOO LONG",
                            mRoutineTag, mKeyValue, mKeyValue, elapsedMs, mWarnMs);
                    } else {
                        ULOG_FW_LOGW(mLogTag, "[%s] executed %d ms(>%d) TOO LONG",
                            mRoutineTag, elapsedMs, mWarnMs);
                    }
                }
            }

            if (NSCam::Utils::ULog::ULogger::isFileModeEnabled()) {
                char buffer[__CAM_ULOG_DETAILS_BUFFER_SIZE__];
                size_t contentLen = 0;

                if (mOptions & DYNAMIC_STRING) {
                    char *dynStr = reinterpret_cast<char*>(mKeyValue);
                    contentLen = snprintf(buffer, sizeof(buffer), "[%s] executed %d ms(>%d) TOO LONG: %s",
                        mRoutineTag, elapsedMs, mWarnMs, dynStr);
                } else {
                    if (mKeyValue != NO_KEY_VALUE) {
                        contentLen = snprintf(buffer, sizeof(buffer), "[%s] key = %" PRIdPTR "(0x%" PRIxPTR ") executed %d ms(>%d) TOO LONG",
                            mRoutineTag, mKeyValue, mKeyValue, elapsedMs, mWarnMs);
                    } else {
                        contentLen = snprintf(buffer, sizeof(buffer), "[%s] executed %d ms(>%d) TOO LONG",
                            mRoutineTag, elapsedMs, mWarnMs);
                    }
                }
                buffer[sizeof(buffer) - 1] = '\0';
                NSCam::Utils::ULog::ULogger::logNormalDetails(mModuleId, mLogTag, DETAILS_WARNING, buffer, contentLen);
            }
        }
    }
}


}
}
}


