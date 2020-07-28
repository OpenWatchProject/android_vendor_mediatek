/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Sensors.h"

#include <android/hardware/sensors/2.0/types.h>

#include <sensors/convert.h>
#include "multihal.h"

#include <android-base/logging.h>

#include <sys/stat.h>
#include <utils/Log.h>
#include <signal.h>

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "android.hardware.sensors@2.0-service-mediatek"
#endif

#define DEBUG_CONNECTIONS false
#define UNUSED(x) (void)(x)

namespace android {
namespace hardware {
namespace sensors {
namespace V2_0 {
namespace implementation {

using namespace android::hardware::sensors::V1_0::implementation;

using ::android::hardware::sensors::V1_0::Event;
using ::android::hardware::sensors::V1_0::OperationMode;
using ::android::hardware::sensors::V1_0::RateLevel;
using ::android::hardware::sensors::V1_0::Result;
using ::android::hardware::sensors::V1_0::SharedMemInfo;
using ::android::hardware::sensors::V1_0::SensorInfo;
using ::android::hardware::sensors::V2_0::SensorTimeout;
using ::android::hardware::sensors::V2_0::WakeLockQueueFlagBits;
using ::android::hardware::sensors::V2_0::EventQueueFlagBits;

constexpr const char* kWakeLockName = "SensorsHAL_WAKEUP";

/*
 * If a multi-hal configuration file exists in the proper location,
 * return true indicating we need to use multi-hal functionality.
 */
static bool UseMultiHal() {
    const std::string& name = MULTI_HAL_CONFIG_FILE_PATH;
    struct stat buffer;
    return (stat (name.c_str(), &buffer) == 0);
}

static Result ResultFromStatus(status_t err) {
    switch (err) {
        case OK:
            return Result::OK;
        case PERMISSION_DENIED:
            return Result::PERMISSION_DENIED;
        case NO_MEMORY:
            return Result::NO_MEMORY;
        case BAD_VALUE:
            return Result::BAD_VALUE;
        default:
            return Result::INVALID_OPERATION;
    }
}

Sensors::Sensors()
    : mInitCheck(NO_INIT),
      mSensorModule(nullptr),
      mSensorDevice(nullptr),
      mEventQueueFlag(nullptr),
      mOutstandingWakeUpEvents(0),
      mReadWakeLockQueueRun(false),
      mAutoReleaseWakeLockTime(0),
      mHasWakeLock(false),
      mRunThreadEnable(false),
      mEventQueueInitialized(false) {
    status_t err = OK;
    if (UseMultiHal()) {
        mSensorModule = ::get_multi_hal_module_info();
    } else {
        err = hw_get_module(
            SENSORS_HARDWARE_MODULE_ID,
            (hw_module_t const **)&mSensorModule);
    }
    if (mSensorModule == NULL) {
        err = UNKNOWN_ERROR;
    }

    if (err != OK) {
        LOG(ERROR) << "Couldn't load "
                   << SENSORS_HARDWARE_MODULE_ID
                   << " module ("
                   << strerror(-err)
                   << ")";

        mInitCheck = err;
        return;
    }

    err = sensors_open_1(&mSensorModule->common, &mSensorDevice);

    if (err != OK) {
        LOG(ERROR) << "Couldn't open device for module "
                   << SENSORS_HARDWARE_MODULE_ID
                   << " ("
                   << strerror(-err)
                   << ")";

        mInitCheck = err;
        return;
    }

    // Require all the old HAL APIs to be present except for injection, which
    // is considered optional.
    CHECK_GE(getHalDeviceVersion(), SENSORS_DEVICE_API_VERSION_1_3);

    if (getHalDeviceVersion() == SENSORS_DEVICE_API_VERSION_1_4) {
        if (mSensorDevice->inject_sensor_data == nullptr) {
            LOG(ERROR) << "HAL specifies version 1.4, but does not implement inject_sensor_data()";
        }
        if (mSensorModule->set_operation_mode == nullptr) {
            LOG(ERROR) << "HAL specifies version 1.4, but does not implement set_operation_mode()";
        }
    }

    mInitCheck = OK;

    mRunThreadEnable = true;
    mRunThread = std::thread(startThread, this);
    ALOGI("HIDL 2.0 load sensor module success.");
}

Sensors::~Sensors() {
    mRunThreadEnable = false;
    /* send signal to wake up poll so the poll thread can exit */
    pthread_kill(mRunThread.native_handle(), SIGQUIT);
    /* wait for poll exit */
    mRunThread.join();
    deleteEventFlag();
    mReadWakeLockQueueRun = false;
    if (mEventQueueInitialized.load()) {
        /* send signal to wake up poll so the poll thread can exit */
        pthread_kill(mWakeLockThread.native_handle(), SIGQUIT);
        /* wait for wakelock thread exit */
        mWakeLockThread.join();
    }
}

status_t Sensors::initCheck() const {
    return mInitCheck;
}

Return<void> Sensors::getSensorsList(getSensorsList_cb _hidl_cb) {
    sensor_t const *list;
    size_t count = mSensorModule->get_sensors_list(mSensorModule, &list);

    std::vector<SensorInfo> out;
    out.resize(count);

    for (size_t i = 0; i < count; ++i) {
        const sensor_t *src = &list[i];
        SensorInfo *dst = &out[i];

        convertFromSensor(*src, dst);
    }

    _hidl_cb(out);

    return Void();
}

int Sensors::getHalDeviceVersion() const {
    if (!mSensorDevice) {
        return -1;
    }

    return mSensorDevice->common.version;
}

Return<Result> Sensors::setOperationMode(OperationMode mode) {
    if (getHalDeviceVersion() < SENSORS_DEVICE_API_VERSION_1_4
            || mSensorModule->set_operation_mode == nullptr) {
        return Result::INVALID_OPERATION;
    }
    return ResultFromStatus(mSensorModule->set_operation_mode((uint32_t)mode));
}

Return<Result> Sensors::initialize(
    const ::android::hardware::MQDescriptorSync<Event>& eventQueueDescriptor,
    const ::android::hardware::MQDescriptorSync<uint32_t>& wakeLockDescriptor,
    const sp<ISensorsCallback>& sensorsCallback) {
    Result result = Result::OK;

    // Ensure that all sensors are disabled
    //for (auto sensor : mSensors) {
    //    sensor.second->activate(false /* enable */);
    //}

    // Stop the Wake Lock thread if it is currently running
    if (mEventQueueInitialized.load()) {
        if (mReadWakeLockQueueRun.load()) {
            mReadWakeLockQueueRun = false;
            /* send signal to wake up poll so the poll thread can exit */
            pthread_kill(mWakeLockThread.native_handle(), SIGQUIT);
            /* wait for wakelock thread exit */
            mWakeLockThread.join();
            ALOGI("Repeat initialize, collect wakelock thread");
        }
    } else {
        ALOGI("First initialize");
    }
    mEventQueueInitialized = false;

    // Save a reference to the callback
    mCallback = sensorsCallback;

    // Create the Event FMQ from the eventQueueDescriptor. Reset the read/write positions.
    mEventQueue =
        std::make_unique<EventMessageQueue>(eventQueueDescriptor, true /* resetPointers */);

    // Ensure that any existing EventFlag is properly deleted
    deleteEventFlag();

    // Create the EventFlag that is used to signal to the framework that sensor events have been
    // written to the Event FMQ
    if (EventFlag::createEventFlag(mEventQueue->getEventFlagWord(), &mEventQueueFlag) != OK) {
        result = Result::BAD_VALUE;
    }

    // Create the Wake Lock FMQ that is used by the framework to communicate whenever WAKE_UP
    // events have been successfully read and handled by the framework.
    mWakeLockQueue =
        std::make_unique<WakeLockMessageQueue>(wakeLockDescriptor, true /* resetPointers */);

    if (!mCallback || !mEventQueue || !mWakeLockQueue || mEventQueueFlag == nullptr) {
        result = Result::BAD_VALUE;
    }

    // Start the thread to read events from the Wake Lock FMQ
    mReadWakeLockQueueRun = true;
    mWakeLockThread = std::thread(startReadWakeLockThread, this);

    mEventQueueInitialized = true;

    return result;
}

Return<Result> Sensors::activate(
        int32_t sensor_handle, bool enabled) {
    ALOGI_IF(DEBUG_CONNECTIONS, "sensor hidl activate handle(%d) enable(%d).", sensor_handle, enabled);
    return ResultFromStatus(
            mSensorDevice->activate(
                reinterpret_cast<sensors_poll_device_t *>(mSensorDevice),
                sensor_handle,
                enabled));
}

void Sensors::startThread(Sensors* sensors) {
    sensors->poll();
}

static void pollThreadSigQuit(int signo) {
    UNUSED(signo);
    pthread_exit(NULL);
}

void Sensors::poll() {
    int err = 0;
    std::unique_ptr<sensors_event_t[]> data;
    int bufferSize = kPollMaxBufferSize;
    std::vector<Event> events;

    signal(SIGQUIT, pollThreadSigQuit);

    data.reset(new sensors_event_t[bufferSize]);

    while (mRunThreadEnable.load()) {
        memset(data.get(), 0, bufferSize * sizeof(sensors_event_t));
        err = mSensorDevice->poll(
                reinterpret_cast<sensors_poll_device_t *>(mSensorDevice),
                data.get(), bufferSize);

        if (err < 0)
            continue;
        const size_t count = (size_t)err;
        events.resize(count);
        convertFromSensorEvents(count, data.get(), &events);
        // no need to wakelock, we hold wakeup source in kernel space
        postEvents(events, false);
    }
}

Return<Result> Sensors::batch(
        int32_t sensor_handle,
        int64_t sampling_period_ns,
        int64_t max_report_latency_ns) {
    ALOGI_IF(DEBUG_CONNECTIONS, "sensor hidl batch handle(%d)...", sensor_handle);
    return ResultFromStatus(
            mSensorDevice->batch(
                mSensorDevice,
                sensor_handle,
                0, /*flags*/
                sampling_period_ns,
                max_report_latency_ns));
}

Return<Result> Sensors::flush(int32_t sensor_handle) {
    ALOGI_IF(DEBUG_CONNECTIONS, "sensor hidl flush handle(%d)...", sensor_handle);
    return ResultFromStatus(mSensorDevice->flush(mSensorDevice, sensor_handle));
}

Return<Result> Sensors::injectSensorData(const Event& event) {
    if (getHalDeviceVersion() < SENSORS_DEVICE_API_VERSION_1_4
            || mSensorDevice->inject_sensor_data == nullptr) {
        return Result::INVALID_OPERATION;
    }

    sensors_event_t out;
    convertToSensorEvent(event, &out);

    return ResultFromStatus(
            mSensorDevice->inject_sensor_data(mSensorDevice, &out));
}

Return<void> Sensors::registerDirectChannel(
        const SharedMemInfo& mem, registerDirectChannel_cb _hidl_cb) {
    if (mSensorDevice->register_direct_channel == nullptr
            || mSensorDevice->config_direct_report == nullptr) {
        // HAL does not support
        _hidl_cb(Result::INVALID_OPERATION, -1);
        return Void();
    }

    sensors_direct_mem_t m;
    if (!convertFromSharedMemInfo(mem, &m)) {
      _hidl_cb(Result::BAD_VALUE, -1);
      return Void();
    }

    int err = mSensorDevice->register_direct_channel(mSensorDevice, &m, -1);

    if (err < 0) {
        _hidl_cb(ResultFromStatus(err), -1);
    } else {
        int32_t channelHandle = static_cast<int32_t>(err);
        _hidl_cb(Result::OK, channelHandle);
    }
    return Void();
}

Return<Result> Sensors::unregisterDirectChannel(int32_t channelHandle) {
    if (mSensorDevice->register_direct_channel == nullptr
            || mSensorDevice->config_direct_report == nullptr) {
        // HAL does not support
        return Result::INVALID_OPERATION;
    }

    mSensorDevice->register_direct_channel(mSensorDevice, nullptr, channelHandle);

    return Result::OK;
}

Return<void> Sensors::configDirectReport(
        int32_t sensorHandle, int32_t channelHandle, RateLevel rate,
        configDirectReport_cb _hidl_cb) {
    if (mSensorDevice->register_direct_channel == nullptr
            || mSensorDevice->config_direct_report == nullptr) {
        // HAL does not support
        _hidl_cb(Result::INVALID_OPERATION, -1);
        return Void();
    }

    sensors_direct_cfg_t cfg = {
        .rate_level = convertFromRateLevel(rate)
    };
    if (cfg.rate_level < 0) {
        _hidl_cb(Result::BAD_VALUE, -1);
        return Void();
    }

    int err = mSensorDevice->config_direct_report(mSensorDevice,
            sensorHandle, channelHandle, &cfg);

    if (rate == RateLevel::STOP) {
        _hidl_cb(ResultFromStatus(err), -1);
    } else {
        _hidl_cb(err > 0 ? Result::OK : ResultFromStatus(err), err);
    }
    return Void();
}

// static
void Sensors::convertFromSensorEvents(
        size_t count,
        const sensors_event_t *srcArray,
        std::vector<Event> *dstVec) {
    for (size_t i = 0; i < count; ++i) {
        const sensors_event_t &src = srcArray[i];
        Event *dst = &(*dstVec)[i];

        convertFromSensorEvent(src, dst);
    }
}

void Sensors::postEvents(const std::vector<Event>& events, bool wakeup) {
    constexpr int64_t kWriteTimeoutNs = 500000000; // 500ms timeout
    std::lock_guard<std::mutex> lock(mWriteLock);

    while (1) {
        if (!mEventQueueInitialized.load()) {
            ALOGE("EventQueue uninitialized");
            return;
        }

        // kWriteTimeoutNs is necessary when system_server crash but hidl don't crash
        // blocking api without timeout make polling thread can't wakeup forever
        // timeout architecture can wakeup polling thread when sensorservice ack slowly
        if (mEventQueue->writeBlocking(events.data(), events.size(),
                                       static_cast<uint32_t>(EventQueueFlagBits::EVENTS_READ),
                                       static_cast<uint32_t>(EventQueueFlagBits::READ_AND_PROCESS),
                                       kWriteTimeoutNs, mEventQueueFlag)) {
            //Using the writeBlocking method (instead of the normal write once) to fill events
            //into FMQ. And to avoid data loss cause by the upper layer sensorSevice not being
            //able to read data in time of data flooding.
            if (wakeup) {
                // Keep track of the number of outstanding WAKE_UP events in order to properly hold
                // a wake lock until the framework has secured a wake lock
                // updateWakeLock(events.size(), 0 /* eventsHandled */);
            }
            return;
        } else {
            ALOGI("Timeout to write and wait sensorservive ack");
        }
    }
}

// no need wakelock catch here, wakelock catch in kernel space
void Sensors::updateWakeLock(int32_t eventsWritten, int32_t eventsHandled) {
    std::lock_guard<std::mutex> lock(mWakeLockLock);
    int32_t newVal = mOutstandingWakeUpEvents + eventsWritten - eventsHandled;
    if (newVal < 0) {
        mOutstandingWakeUpEvents = 0;
    } else {
        mOutstandingWakeUpEvents = newVal;
    }

    if (eventsWritten > 0) {
        // Update the time at which the last WAKE_UP event was sent
        mAutoReleaseWakeLockTime = ::android::uptimeMillis() +
                                   static_cast<uint32_t>(SensorTimeout::WAKE_LOCK_SECONDS) * 1000;
    }

    if (!mHasWakeLock && mOutstandingWakeUpEvents > 0 &&
        acquire_wake_lock(PARTIAL_WAKE_LOCK, kWakeLockName) == 0) {
        mHasWakeLock = true;
    } else if (mHasWakeLock) {
        // Check if the wake lock should be released automatically if
        // SensorTimeout::WAKE_LOCK_SECONDS has elapsed since the last WAKE_UP event was written to
        // the Wake Lock FMQ.
        if (::android::uptimeMillis() > mAutoReleaseWakeLockTime) {
            ALOGD("No events read from wake lock FMQ for %d seconds, auto releasing wake lock",
                  SensorTimeout::WAKE_LOCK_SECONDS);
            mOutstandingWakeUpEvents = 0;
        }

        if (mOutstandingWakeUpEvents == 0 && release_wake_lock(kWakeLockName) == 0) {
            mHasWakeLock = false;
        }
    }
}

void Sensors::readWakeLockFMQ() {
    signal(SIGQUIT, pollThreadSigQuit);
    while (mReadWakeLockQueueRun.load()) {
        constexpr int64_t kReadTimeoutNs = 0;  // no need timeout
        uint32_t eventsHandled = 0;

        // Read events from the Wake Lock FMQ. Timeout after a reasonable amount of time to ensure
        // that any held wake lock is able to be released if it is held for too long.
        mWakeLockQueue->readBlocking(&eventsHandled, 1 /* count */, 0 /* readNotification */,
                                     static_cast<uint32_t>(WakeLockQueueFlagBits::DATA_WRITTEN),
                                     kReadTimeoutNs);
        // updateWakeLock(0 /* eventsWritten */, eventsHandled);
    }
}

void Sensors::startReadWakeLockThread(Sensors* sensors) {
    sensors->readWakeLockFMQ();
}

void Sensors::deleteEventFlag() {
    status_t status = EventFlag::deleteEventFlag(&mEventQueueFlag);
    if (status != OK) {
        ALOGI("Failed to delete event flag: %d", status);
    }
}

}  // namespace implementation
}  // namespace V2_0
}  // namespace sensors
}  // namespace hardware
}  // namespace android
