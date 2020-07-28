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
#define  LOG_TAG "Mtkcam/GyroCollector"
#include <mtkcam/utils/std/Log.h>

// MTKCAM
#include <mtkcam/utils/hw/GyroCollector.h> // GyroCollector
#include <mtkcam/utils/std/StlUtils.h> // NSCam::SpinLock
#include <mtkcam/utils/std/Trace.h> // CAM_TRACE

// SensorListener
#include <android/sensor.h> // dependecy of SensorListener.h
#include <mtkcam/def/BuiltinTypes.h> // dependecy of SensorListener.h
#include <mtkcam/utils/sys/SensorListener.h> // SensorListener

// AOSP
#include <utils/SystemClock.h> // elapsedRealtime(), uptimeMillis()
#include <utils/RWLock.h> // RWLock
#include <cutils/compiler.h> // CC_LIKELY, CC_UNLIKELY

// LINUX
#include <sys/prctl.h>

// STL
#include <cstdlib>      // std::abs
#include <memory>       // std::unique_ptr, std::shared_ptr
#include <vector>       // std::vector
#include <functional>   // std::function
#include <atomic>       // std::atomic
#include <thread>       // std::thread
#include <condition_variable>
#include <chrono>
#include <future>

/* Define _DEBUG to enable debug log */
#define _DEBUG

#define _DEBUG_TRIGGER          0
#define _DEBUG_SENSOR_LISTENER  0
#define _DEBUG_GETDATA          0
#define _DEBUG_GET_DEEP_SLEEP   0
#define _DEBUG_DQCOLLECTOR      0
#define _DEBUG_TIMESCROPE       0

// MY_LOG
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
// FUNCTION_SCOPE
#ifdef _DEBUG
#define FUNCTION_SCOPE          auto __scope_logger__ = create_scope_logger(__FUNCTION__)
#include <memory>
static std::shared_ptr<char> create_scope_logger(const char* functionName) __attribute__((unused));
static std::shared_ptr<char> create_scope_logger(const char* functionName)
{
    char* pText = const_cast<char*>(functionName);
    CAM_LOGD("[%s] + ", pText);
    return std::shared_ptr<char>(pText, [](char* p){ CAM_LOGD("[%s] -", p); });
}
#else
#define FUNCTION_SCOPE              do {} while(0)
#endif
// SCOPE_TIMER
#if defined(_DEBUG) && (_DEBUG_TIMESCROPE != 0)
#define SCOPE_TIMER(VAR, TEXT)  auto VAR = create_scope_timer(TEXT)
#define FUNCTION_TIME_SCOPE     SCOPE_TIMER(t__func, __FUNCTION__)
#include <memory>
#include <functional>
#include <utils/SystemClock.h> // elapsedRealtime(), uptimeMillis()
static std::unique_ptr<const char, std::function<void(const char*)> >
create_scope_timer(const char* text)
{
    auto t1 = android::elapsedRealtimeNano();
    return std::unique_ptr<const char, std::function<void(const char*)> >
        (
            text,
            [t1](const char* t)->void
            {
                auto t2 = android::elapsedRealtimeNano();
                MY_LOGD("%s --> duration(ns): %" PRId64 "", t, (t2 -t1));
            }
        );
}
#else
#define SCOPE_TIMER(VAR, TEXT)  do{}while(0)
#define FUNCTION_TIME_SCOPE     SCOPE_TIMER(t__func, __FUNCTION__)
#endif // SCOPE_TIMER


using namespace NSCam::Utils;


#if MTKCAM_HAVE_GYROCOLLECTOR_SUPPORT


// Typedefs of GyroInfo
typedef std::mutex      GyroCollector_StateLock;
// Typedefs of SensorListener
typedef std::mutex      SensorListener_Lock;
typedef std::unique_ptr<
    SensorListener,
    std::function<void(SensorListener*)>
> SensorListener_Ptr;


// To give two info stacks, one for writing data, another for reading.
// After a data has been written into this double queue, a dirty flag is marked
// as true. While reading, if the dirty flag is true, the read buffer will be
// updated (lazy update).
//
// Notice that, this double queue mechanism is designed for write often case and
// who do not want to slow down the performance of writing thread. (Write first)
class GyroInfoWriteFirstQueue
{
public:
    typedef std::function<
        GyroCollector::GyroInfoContainer(
                const std::vector<GyroCollector::GyroInfo>&,
                int
                )
        >
        T_GetData_Func;


public:
    void write(const GyroCollector::GyroInfo& info)
    {
        std::lock_guard<decltype(m_gyroWLock)> l(m_gyroWLock);
        m_gyroWInfos[m_gyroWInfoIdx] = info;
        m_gyroWInfoIdx = (m_gyroWInfoIdx + 1) % GyroCollector::STACK_SIZE;
        m_dirtyFlag = true; // mark as dirty!
    }

    void clear()
    {
        std::vector<GyroCollector::GyroInfo> _empty;
        _empty.resize(GyroCollector::STACK_SIZE);

        {
            android::RWLock::AutoWLock lr(m_gyroRLock); // enter write section
            std::lock_guard<decltype(m_gyroWLock)> l(m_gyroWLock);
            std::swap(m_gyroWInfos, _empty);
            m_gyroWInfoIdx = 0;
            m_dirtyFlag = true;
        }
    }

    GyroCollector::GyroInfoContainer read(const T_GetData_Func& f)
    {
        FUNCTION_SCOPE;

        do {
            bool isDirty = false;

            // check dirty flag if it's dirty or not
            m_gyroWLock.lock();
            isDirty = m_dirtyFlag;
            m_gyroWLock.unlock();

            // if it's dirty,
            if (isDirty) {
#if defined(_DEBUG) && (_DEBUG_DQCOLLECTOR > 0)
                MY_LOGD("%s: dirty, need update", __FUNCTION__);
#endif

                m_updaterLock.lock();
                if (m_updating) {
                    m_updaterLock.unlock();
                    // if it's being updated, use cached
                    break;
                }
                else {
                    // updating!
                    m_updating = true;
                    m_updaterLock.unlock();
                }

                {
                    android::RWLock::AutoWLock lr(m_gyroRLock); // enter write section
                    std::lock_guard<decltype(m_gyroWLock)> l(m_gyroWLock);
                    // due to the sizes of m_gyroWInfos and m_gyroRInfos are
                    // always the same, we can always use std::copy here
                    std::copy(
                            m_gyroWInfos.begin(), m_gyroWInfos.end(),
                            m_gyroRInfos.begin());

                    m_gyroRInfoIdx = m_gyroWInfoIdx;
                    m_dirtyFlag = false; // updated, mark as non-dirty
                }

                m_updaterLock.lock();
                m_updating = false;
                m_updaterLock.unlock();
            }
        } while(0);

        GyroCollector::GyroInfoContainer rQ;
        // reading section
        {
            android::RWLock::AutoRLock l(m_gyroRLock);
            // invoke function
            if (CC_LIKELY(f))
                rQ = f(m_gyroRInfos, m_gyroRInfoIdx);
        }

        return rQ; //RVO
    }


public:
    GyroInfoWriteFirstQueue()
        : m_gyroWInfoIdx(0),    m_gyroRInfoIdx(0)
        , m_dirtyFlag(false) ,  m_updating(0)
    {
        m_gyroWInfos.resize(GyroCollector::STACK_SIZE);
        m_gyroRInfos.resize(GyroCollector::STACK_SIZE);
    }

    ~GyroInfoWriteFirstQueue() = default;


private:
    volatile int                            m_gyroWInfoIdx;
    volatile int                            m_gyroRInfoIdx;

    volatile bool                           m_dirtyFlag;

    volatile bool                           m_updating;
    // updater lock
    std::mutex                              m_updaterLock;

    // write buffer
    std::vector<GyroCollector::GyroInfo>    m_gyroWInfos;
    std::mutex                              m_gyroWLock;

    // read buffer
    std::vector<GyroCollector::GyroInfo>    m_gyroRInfos;
    android::RWLock                         m_gyroRLock;

};


// ----------------------------------------------------------------------------
// static members
// ----------------------------------------------------------------------------
static GyroInfoWriteFirstQueue  g_gyroStack;

// GyroCollector operation
// IDLE <--> ACTIVATING <--> ACTIVATED <--> STOPPING <--> IDLE ...
constexpr static int            STATE_IDLE         = 0;
constexpr static int            STATE_ACTIVATING   = 1;
constexpr static int            STATE_ACTIVATED    = 2;
constexpr static int            STATE_STOPPING     = 3;
static GyroCollector_StateLock  g_stateLock;
static int                      g_state(STATE_IDLE);
static std::atomic<long>        g_tsTrigger(0); // saves timestamp of triggering
static std::atomic<long>        g_tsStopped(0); // saves timestamp of stopped

// Executable thread
static std::future<void>    g_futureExe;

// SensorListener
static SensorListener_Ptr       g_sensorListener;
static SensorListener_Lock      g_sensorListenerLock;


// ----------------------------------------------------------------------------
// static functions
// ----------------------------------------------------------------------------
//
// returns the durtion of deep sleep
// domian: millisecond (ps: 1 millisecond = 1,000,000 nanosecond)
//
static inline int64_t get_deep_sleep_duration()
{
    int64_t rDuration = std::abs(
            android::elapsedRealtime() - android::uptimeMillis()
            );
#if defined(_DEBUG) && (_DEBUG_GET_DEEP_SLEEP > 0)
    MY_LOGD("%s: deep sleep duration (ms) = %" PRId64 "", __FUNCTION__, rDuration);
#endif
    return rDuration;
}

// start/stop listening gyro information
//  @note These two functions are NOT thread-safe.
static inline bool start_listening();
static inline bool stop_listening();

//
// sensor listener
//  @note The timestamp of sensor listener "includes" deep sleep duration.
//
static void sensor_listener_callback(ASensorEvent event)
{
#if defined(_DEBUG) && (_DEBUG_SENSOR_LISTENER > 0)
    FUNCTION_SCOPE;
#endif

    if (event.type == ASENSOR_TYPE_GYROSCOPE) {
        GyroCollector::GyroInfo info;
        info.timestamp = event.timestamp;
        info.x = event.vector.x;
        info.y = event.vector.y;
        info.z = event.vector.z;

#if defined(_DEBUG) && (_DEBUG_SENSOR_LISTENER > 0)
        MY_LOGD("%s: (ts,x,y,z)=(%" PRId64 ",%f,%f,%f)",
                __FUNCTION__,
                info.timestamp,
                info.x,
                info.y,
                info.z
                );
#endif

        g_gyroStack.write(info);

        // check ts of triggering, if duration is greater than threshold.
        if (std::abs(android::elapsedRealtime() - g_tsTrigger.load()) >= static_cast<long>(GyroCollector::IDLE_TIMEOUT)) {
            bool _need_stop = false;

            // enter critical section
            {
                std::lock_guard<GyroCollector_StateLock> _l(g_stateLock);
                if (g_state == STATE_ACTIVATED) {
                    g_state = STATE_STOPPING;
                    _need_stop = true;
                }
            }

            if (_need_stop) {
                std::thread([]() {
                    CAM_TRACE_NAME("GyroCollector@threadStopSensorListener");
                    ::prctl(PR_SET_NAME, "GyroCollector", 0, 0, 0);
                    MY_LOGD("stop listener thread [+]");
                    // stop_listening will wait until sensor_listener_callback returned.
                    if ( CC_LIKELY(stop_listening()) ) {
                        std::lock_guard<GyroCollector_StateLock> _l(g_stateLock);
                        g_state = STATE_IDLE;
                    }
                    else {
                        // stop fail, mark as ACTIAVED
                        std::lock_guard<GyroCollector_StateLock> _l(g_stateLock);
                        g_state = STATE_ACTIVATED;
                    }
                    g_tsStopped.store(android::elapsedRealtime());
                    MY_LOGD("stop listener thread [-]");
                }).detach();
            }
        }
    }
}


//
// Start the sensor listener for listening gyro info
//
static inline bool start_listening()
{
    CAM_TRACE_NAME("GyroCollector@start_listening");
    FUNCTION_SCOPE;
    FUNCTION_TIME_SCOPE;

    std::lock_guard<SensorListener_Lock> _l(g_sensorListenerLock);

    if (g_sensorListener.get() != nullptr) {
        // no need to create again
        return false;
    }

    g_sensorListener = SensorListener_Ptr(
            SensorListener::createInstance("GyroCollector"),
            [](SensorListener* p) { if (p) p->destroyInstance(); }
            );

    if (CC_UNLIKELY(g_sensorListener.get() == nullptr)) {
        MY_LOGE("create SensorListener failed");
        return false;
    }

    // set listener
    auto result = g_sensorListener->setListener(sensor_listener_callback);
    if (CC_UNLIKELY(!result)) {
        MY_LOGE("SensorListener::setListener returns false");
        g_sensorListener = nullptr; // clean up
        return false;
    }

    // start listening
    result = g_sensorListener->enableSensor(
            SensorListener::SensorType_Gyro,
            GyroCollector::INTERVAL);
    if (CC_UNLIKELY(!result)) {
        MY_LOGE("SensorListener::enableSensor returns false");
        g_sensorListener = nullptr; // clean up
        return false;
    }

    return true;
}


//
// Stop listening gyro info
//
static inline bool stop_listening()
{
    CAM_TRACE_NAME("GyroCollector@stop_listening");
    FUNCTION_SCOPE;
    FUNCTION_TIME_SCOPE;

    bool result = true;

    {
        std::lock_guard<SensorListener_Lock> _l(g_sensorListenerLock);
        if (CC_LIKELY(g_sensorListener.get())) {
            result = g_sensorListener->disableSensor(SensorListener::SensorType_Gyro);
            if (CC_UNLIKELY(!result)) {
                MY_LOGE("SensorListener::disableSensor returns false");
            }
            g_sensorListener = nullptr;
        }
    }

    g_gyroStack.clear();

    return result;
}

// ----------------------------------------------------------------------------
// GyroCollector
// ----------------------------------------------------------------------------
void
GyroCollector::
trigger()
{
    CAM_TRACE_NAME("GyroCollector@trigger");

#if defined(_DEBUG) && (_DEBUG_TRIGGER != 0)
    FUNCTION_SCOPE;
    FUNCTION_TIME_SCOPE;
#endif

    // check stopped timestamp
    if (std::abs(android::elapsedRealtime() - g_tsStopped.load()) < static_cast<long>(GyroCollector::IDLE_TIMEOUT)) {
        // avoid waste resource for immediately start after stooped.
        return;
    }

    bool _need_start = false;
    {
        std::lock_guard<GyroCollector_StateLock> _l(g_stateLock);
        if (g_state == STATE_IDLE) {
            g_state = STATE_ACTIVATING;
            _need_start = true;
        }
    }


    if (_need_start) {
        // flag represents if it's need to start_listening
        bool bStart = false;

        // std::shared_future::valid
        if (g_futureExe.valid()) {
            switch (g_futureExe.wait_for(std::chrono::seconds(0))) {
            case std::future_status::deferred: // Job hasn't been executed yet.
            case std::future_status::ready: // Job has finished.
                bStart = true; // It's ok to start a new job.
                break;
            case std::future_status::timeout: // Job is still in executing
                MY_LOGW("start_listening is still in executing");
                break;
            default:;
                MY_LOGW("start_listening is in unknow state!");
            }
        }
        else {
            bStart = true;
        }

        if (bStart) {
            // async trigger gyro collector
            auto fu = std::async(std::launch::async, [](){
                    CAM_TRACE_NAME("GyroCollector@threadStartSensorListener");
                    ::prctl(PR_SET_NAME, "GyroCollector@start_listening", 0, 0, 0);
                    MY_LOGD("start listener thread [+]");
                    if (start_listening()) {
                        std::lock_guard<GyroCollector_StateLock> _l(g_stateLock);
                        g_state = STATE_ACTIVATED;
                    }
                    else {
                        MY_LOGW("start listening failed, state back to idle");
                        std::lock_guard<GyroCollector_StateLock> _l(g_stateLock);
                        g_state = STATE_IDLE;
                    }
                    MY_LOGD("start listener thread [-]");
            });

            g_futureExe = std::move(fu);
        }
    }

    // update trigger timestamp
    g_tsTrigger.store(android::elapsedRealtime());
}


GyroCollector::GyroInfoContainer
GyroCollector::
getData(
        int64_t ts_start,
        int64_t ts_end
        )
{
    CAM_TRACE_NAME("GyroCollector@getData");
    FUNCTION_SCOPE;
    FUNCTION_TIME_SCOPE;

    // add deep sleep duration
    int64_t deepSleepDuration = get_deep_sleep_duration() * 1000000L;

    ts_start += deepSleepDuration;
    ts_end   += deepSleepDuration;

    // retrieve function object
    auto func = [&ts_start, &ts_end](const std::vector<GyroInfo>& vt, int index)
    {
        GyroInfoContainer rQ_;

        // check the oldest element first, makes timestamp in ordered
        auto i = index;
        size_t dataSize = 0;

#if defined(_DEBUG) && (_DEBUG_GETDATA != 0)
        MY_LOGD("%s: [s, e]=[%" PRId64 ",%" PRId64 "], data=%" PRId64 "", __FUNCTION__,
                ts_start, ts_end, vt[i].timestamp);
#endif

        // check if in range
        if ((vt[i].timestamp > 0)
                && (vt[i].timestamp >= ts_start)
                && (vt[i].timestamp <= ts_end)) {
            rQ_[dataSize++] = vt[i];
        }

        i = (i + 1) % GyroCollector::STACK_SIZE;

        for (; i != index; i = (i + 1) % GyroCollector::STACK_SIZE) {
#if defined(_DEBUG) && (_DEBUG_GETDATA != 0)
            MY_LOGD("%s: [s, e]=[%" PRId64 ",%" PRId64 "], data=%" PRId64 "", __FUNCTION__,
                    ts_start, ts_end, vt[i].timestamp);
#endif

            // check if in range
            if ((vt[i].timestamp > 0)
                    && (vt[i].timestamp >= ts_start)
                    && (vt[i].timestamp <= ts_end)) {
                rQ_[dataSize++] = vt[i];
            }
        }

        rQ_.setSize(dataSize);

        return rQ_; // RVO
    };

    return g_gyroStack.read(func); // RVO
}


#else // MTKCAM_HAVE_GYROCOLLECTOR_SUPPORT

void
GyroCollector::trigger()
{
}


GyroCollector::GyroInfoContainer
GyroCollector::getData(int64_t, int64_t)
{
    return GyroInfoContainer();
}


#endif // MTKCAM_HAVE_GYROCOLLECTOR_SUPPORT
