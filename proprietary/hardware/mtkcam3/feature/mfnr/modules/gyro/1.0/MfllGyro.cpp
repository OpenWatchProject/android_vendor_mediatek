/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
#define LOG_TAG "MfllCore/Gyro"

#include "MfllGyro.h"
#include <mtkcam3/feature/mfnr/MfllLog.h>

// AOSP
#include <utils/SystemClock.h> // elapsedRealtime(), uptimeMillis()

// MTKCAM
#include <mtkcam/utils/std/StlUtils.h> // NSCam::SpinLock

// STL
#include <algorithm>    // std::find_if
#include <future>       // std::future
#include <iterator>     // std::back_insert
#include <cstdlib>      // std::abs

// gyro queue size is supposed to be defined in makefile
#ifndef MFLL_GYRO_QUEUE_SIZE
#define MFLL_GYRO_QUEUE_SIZE 20
#endif

// enable additional debug info
// #define __DEBUG


#ifdef __DEBUG
#include <memory>
#define FUNCTION_SCOPE \
auto __scope_logger__ = [](const char* f)->std::shared_ptr<const char>{ \
    mfllLogD("[%s] + ", f); \
    return std::shared_ptr<const char>(f, [](const char* p){mfllLogD("[%s] -", p);}); \
}(__FUNCTION__)
#else
#define FUNCTION_SCOPE  do{}while(0)
#endif


using namespace mfll;
using android::sp;


// ----------------------------------------------------------------------------
// factory
// ----------------------------------------------------------------------------
IMfllGyro* IMfllGyro::createInstance()
{
    return static_cast<IMfllGyro*>(new MfllGyro);
}

void IMfllGyro::destroyInstance()
{
    decStrong((void*)this);
}


// ----------------------------------------------------------------------------
// static methods (helpers)
// ----------------------------------------------------------------------------
// returns the durtion of deep sleep
// domian: millisecond (ps: 1 millisecond = 1,000,000 nanosecond)
static inline int64_t get_deep_sleep_duration()
{
    int64_t rDuration = std::abs(
            android::elapsedRealtime() - android::uptimeMillis()
            );
#ifdef __DEBUG
    mfllLogD("%s: deep sleep duration (ms) = %lld", __FUNCTION__, rDuration);
#endif
    return rDuration;
}

// ----------------------------------------------------------------------------
// Gyro info queue, singleton, thread-safe
// ----------------------------------------------------------------------------
static struct
{
    // Attributes
    std::deque<IMfllGyro::MfllGyroInfo>  infoQueue;
    NSCam::SpinLock                      infoQueueLock;

    // To add a gyro info to queue, if the size of queue is greater than
    // MFLL_GYRO_QUEUE_SIZE, the oldest one will be removed.
    //  @param info                 Call by value gyro info, caller can use
    //                              std::move to move the argument if it's unused
    //                              anymore.
    inline void addInfo(IMfllGyro::MfllGyroInfo info)
    {
        std::lock_guard<NSCam::SpinLock> __l(infoQueueLock);

        while (infoQueue.size() >= MFLL_GYRO_QUEUE_SIZE) {
            infoQueue.pop_front();
        }

        infoQueue.push_back(std::move(info));
    }

    // To get gyro info in a range [ts_start, ts_end]. O(n) = O(MFLL_GYRO_QUEUE_SIZE)
    //  @param ts_start             Time w/o deep sleep, in nanosecond.
    //  @param ts_end               Time w/o deep sleep, in nanosecond.
    //  @return                     The gyro info.
    inline std::deque<IMfllGyro::MfllGyroInfo> getInfo(int64_t ts_start, int64_t ts_end)
    {
        std::deque<IMfllGyro::MfllGyroInfo> rQueGyroInfo;

        // add deep sleep duration because the timestamps of gyro includes deep sleep time
        int64_t deepSleepDuration = get_deep_sleep_duration() * 1000000L; // ms->nanosecond
        ts_start += deepSleepDuration;
        ts_end   += deepSleepDuration;

#ifdef __DEBUG
        mfllLogD("%s: [ts_start, ts_end]=[%" PRId64 ", %" PRId64 "]", __FUNCTION__, ts_start, ts_end);
#endif

        // apporch O(n), w/ copy constructors, where n = MFLL_GYRO_QUEUE_SIZE
        {
            std::lock_guard<NSCam::SpinLock> __l(infoQueueLock);
            std::copy_if(
                    infoQueue.begin(),
                    infoQueue.end(),
                    std::back_inserter(rQueGyroInfo),
                    [&ts_start, &ts_end](const IMfllGyro::MfllGyroInfo& itr) -> bool {
                        return (itr.timestamp >= ts_start && itr.timestamp <= ts_end);
                    });
        }

        return rQueGyroInfo;
        // return std::move(rQueGyroInfo);
        // we don't invoke std::movet to move rQueGyroInfo because rQueGyroInfo is
        // an rvalue and compiler shall optimize it (RVO)
    }

    // To clear all gyro info
    inline void clearInfo()
    {
        std::lock_guard<NSCam::SpinLock> __l(infoQueueLock);
        infoQueue.clear();
    }

} sInfoQueue;


// ----------------------------------------------------------------------------
// MfllGyroImp, thread-safe, singleton while using, released after no one use it.
// ----------------------------------------------------------------------------
class mfll::MfllGyroImp
{
// method
public:
    // To get gyro info from sInfoQueue (thread-safe)
    std::deque<IMfllGyro::MfllGyroInfo> getInfo(const int64_t& ts_start, const int64_t& ts_end) const
    {
        return sInfoQueue.getInfo(ts_start, ts_end);
    }

    // Clear all gyro info (thread-safe)
    void clearInfo()
    {
        sInfoQueue.clearInfo();
    }

// static methos
public:
    static std::shared_ptr<MfllGyroImp> getInstance()
    {
        std::lock_guard<std::mutex> __l(sLock);
        std::shared_ptr<MfllGyroImp> inst = sInstance.lock();
        if (inst.get() == nullptr) {
            inst = std::shared_ptr<MfllGyroImp>(new MfllGyroImp);
            sInstance = inst;
        }
        return inst;
    }


    static void sensor_listener_callback(ASensorEvent event)
    {
        if (event.type == ASENSOR_TYPE_GYROSCOPE) {
            IMfllGyro::MfllGyroInfo gyroInfo;
            gyroInfo.timestamp = event.timestamp;
            gyroInfo.vector.x() = event.vector.x;
            gyroInfo.vector.y() = event.vector.y;
            gyroInfo.vector.z() = event.vector.z;

#ifdef __DEBUG
        mfllLogD("%s: (ts,x,y,z)=(%lld,%f,%f,%f)",
                __FUNCTION__,
                gyroInfo.timestamp,
                gyroInfo.vector.x(),
                gyroInfo.vector.y(),
                gyroInfo.vector.z()
                );
#endif
            sInfoQueue.addInfo(std::move(gyroInfo));
        }
    }


public:
    MfllGyroImp()
    {
        FUNCTION_SCOPE;

        m_pSensorListener = std::unique_ptr<SensorListener, std::function<void(SensorListener*)>>
        (
            SensorListener::createInstance(LOG_TAG),
            [](SensorListener* self)->void { if (self) self->destroyInstance(); }
        );

        if (m_pSensorListener.get() == NULL) {
            mfllLogE("create SensorListener fail");
            return;
        }

        // set listener
        auto b = m_pSensorListener->setListener(MfllGyroImp::sensor_listener_callback);
        if (!b) {
            mfllLogE("set sensor listener returns fail");
            return;
        }

        if (!m_pSensorListener->enableSensor(
                    SensorListener::SensorType_Gyro,
                    MFLL_GYRO_DEFAULT_INTERVAL_MS))
        {
            mfllLogE("SensorListener::enableSensor returns fail");
            return;
        }
    }

    virtual ~MfllGyroImp()
    {
        FUNCTION_SCOPE;

        if (m_pSensorListener.get() == NULL) {
            mfllLogE("SensorListener is NULL, cannot stop");
        }

        if (m_pSensorListener->disableSensor(SensorListener::SensorType_Gyro)) {
            // ok good, stopped it
        }
        else {
            mfllLogE("SensorListener::disableSensor returns fail");
        }
    }


private:
    std::unique_ptr<SensorListener, std::function<void(SensorListener*)> >
        m_pSensorListener;

    // singleton of weak pointer implementation
    static std::mutex                   sLock;
    static std::weak_ptr<MfllGyroImp>   sInstance;
};
std::mutex                  MfllGyroImp::sLock;
std::weak_ptr<MfllGyroImp>  MfllGyroImp::sInstance;


// ----------------------------------------------------------------------------
// MfllGyro
// ----------------------------------------------------------------------------
enum MfllErr MfllGyro::init(sp<IMfllNvram>& /* nvramProvider */)
{
    return MfllErr_Ok;
}


enum MfllErr MfllGyro::start()
{
    return MfllErr_Ok;
}


enum MfllErr MfllGyro::stop()
{
    return MfllErr_Ok;
}


std::deque<IMfllGyro::MfllGyroInfo> MfllGyro::getGyroInfo(
        const int64_t& ts_start,
        const int64_t& ts_end) const
{
    if (m_gyroImpl.get())
        return m_gyroImpl->getInfo(ts_start, ts_end);

    mfllLogE("gyro instance is NULL");
    return std::deque<MfllGyroInfo>();
}


std::deque<IMfllGyro::MfllGyroInfo> MfllGyro::takeGyroInfo(
        const int64_t& ts_start,
        const int64_t& ts_end,
        bool /* clearInvalidated */)
{
    if (m_gyroImpl.get())
        return m_gyroImpl->getInfo(ts_start, ts_end);

    mfllLogE("gyro instance is NULL");
    return std::deque<MfllGyroInfo>();
}


std::deque<IMfllGyro::MfllGyroInfo> MfllGyro::takeGyroInfoQueue()
{
    mfllLogE("takeGyroInfoQueue hasn't been implemented");
    return std::deque<MfllGyroInfo>();
}

enum MfllErr MfllGyro::sendCommand(
        const std::string&          /* cmd */,
        const std::deque<void*>&    /* dataset */
        )
{
    return MfllErr_NotImplemented;
}


// ----------------------------------------------------------------------------
// constructor
// ----------------------------------------------------------------------------
MfllGyro::MfllGyro()
{
    m_gyroImpl = MfllGyroImp::getInstance();
}

MfllGyro::~MfllGyro()
{
    m_gyroImpl = nullptr;
}
