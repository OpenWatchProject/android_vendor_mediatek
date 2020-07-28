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

#undef LOG_TAG
#define LOG_TAG "HfManager"

#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>
#include <fcntl.h>
#include <inttypes.h>

#include <mutex>

#include <log/log.h>

#include "HfManager.h"

#define HF_MANAGER_NODE_PATH "/dev/hf_manager"

HfManager::HfManager() {
    bool status = false;
    sensorState.reset();
    sensorRegister.reset();

    mFd = TEMP_FAILURE_RETRY(open(HF_MANAGER_NODE_PATH, O_RDWR));
    if (mFd < 0) {
        ALOGE("open device failed err=%d errno=%d\n", mFd, errno);
        return;
    }

    for (int sensor = 0; sensor < HIGH_FREQUENCY_SENSOR_MAX; ++sensor) {
        status = checkRegisterStatus(sensor);
        sensorRegister.set(sensor, status);
    }

    disableAllSensor();
}

HfManager::~HfManager() {
    for (size_t sensor = 0; sensor < sensorState.size(); ++sensor) {
        if (sensorState.test(sensor))
            disableSensor(sensor);
    }
    close(mFd);
}

int HfManager::getFd(void) {
    return mFd;
}

int HfManager::initCheck() const {
    return mFd < 0 ? -ENODEV : 0;
}

int HfManager::findSensor(int sensor) {
    if (sensor >= HIGH_FREQUENCY_SENSOR_MAX)
        return -1;
    int status = sensorRegister.test(sensor);
    return !status ? -1 : sensor;
}

int HfManager::enableSensor(int sensor, int64_t delayNs) {
    return enableDisable(sensor, HF_MANAGER_SENSOR_ENABLE, delayNs, 0);
}

int HfManager::enableSensor(int sensor, int64_t delayNs, int64_t latencyNs) {
    return enableDisable(sensor, HF_MANAGER_SENSOR_ENABLE, delayNs, latencyNs);
}

int HfManager::disableSensor(int sensor) {
    return enableDisable(sensor, HF_MANAGER_SENSOR_DISABLE, 0, 0);
}

int HfManager::enableDisable(int sensor, int enable,
        int64_t delayNs, int64_t latencyNs) {
    int err = -1;
    hf_manager_cmd cmd;

    if ((err = initCheck()) < 0)
        return err;

    ALOGI("enableDisable sensor=%d enable=%d delayNs=%" PRId64 " latencyNs=%" PRId64 "\n",
        sensor, enable, delayNs, latencyNs);

    if (enable == HF_MANAGER_SENSOR_ENABLE)
        sensorState.set(sensor, 1);
    else if (enable == HF_MANAGER_SENSOR_DISABLE)
        sensorState.set(sensor, 0);

    memset(&cmd, 0, sizeof(hf_manager_cmd));
    cmd.action = enable;
    cmd.sensor_id = sensor;
    cmd.delay = delayNs;
    cmd.latency = latencyNs;
    err = TEMP_FAILURE_RETRY(write(mFd, &cmd, sizeof(hf_manager_cmd)));
    if (err < 0) {
        ALOGE("enableSensor failed err=%d errno=%d\n", err, errno);
        return err;
    }
    return err;
}

void HfManager::disableAllSensor(void) {
    for (int sensor = 0; sensor < HIGH_FREQUENCY_SENSOR_MAX; ++sensor) {
        if (sensorRegister.test(sensor))
            disableSensor(sensor);
    }
}

bool HfManager::checkRegisterStatus(int sensor) {
    int err = -1;
    unsigned int result = sensor;

    if ((err = initCheck()) < 0)
        return false;

    err = TEMP_FAILURE_RETRY(ioctl(mFd,
            HF_MANAGER_SENSOR_REGISTER_STATUS, &result));
    if (err < 0) {
        ALOGE("get sensor register failed err=%d errno=%d\n", err, errno);
        return false;
    }
    return result ? true : false;
}

#undef LOG_TAG
#define LOG_TAG "HfLooper"

static void convertEvent(hf_sensors_event_t *dst, hf_manager_event *src) {
    dst->timestamp = src->timestamp;
    dst->sensor_id = src->sensor_id;
    dst->accurancy = src->accurancy;
    dst->data[0] = (float)src->word[0] / HIGH_FREQUENCY_GYRO_DIV;
    dst->data[1] = (float)src->word[1] / HIGH_FREQUENCY_GYRO_DIV;
    dst->data[2] = (float)src->word[2] / HIGH_FREQUENCY_GYRO_DIV;
    dst->data[3] = (float)src->word[3] / HIGH_FREQUENCY_GYRO_DIV;
    dst->data[4] = (float)src->word[4] / HIGH_FREQUENCY_GYRO_DIV;
    dst->data[5] = (float)src->word[5] / HIGH_FREQUENCY_GYRO_DIV;
}

HfLooper::HfLooper(int fd, size_t pollBufferSize) {
    mPollBufferSize = (pollBufferSize > pollMaxBufferSize) ?
        pollBufferSize : pollMaxBufferSize;
    pollBuffer.reset(new hf_manager_event[mPollBufferSize]);

    mFd = fd;
    mPollFds[0].fd = fd;
    mPollFds[0].events = POLLIN;
    mPollFds[0].revents = 0;
    mNumPollFds = 1;
}

HfLooper::~HfLooper() {
    mPollFds[0].fd = mFd = -1;
    /* must close fd then pollBuffer delete, wait poll return */
    pollBuffer.reset();
}

int HfLooper::initCheck() const {
    return mFd < 0 ? -ENODEV : 0;
}

ssize_t HfLooper::eventLooper(void (*callback)(hf_sensors_event_t const *)) {
    int err = -1;
    ssize_t len = 0, count = 0;
    hf_manager_event *buffer;
    hf_sensors_event_t event;

    if ((err = initCheck()) < 0)
        return err;

    do {
        err = poll(mPollFds, mNumPollFds, -1);
    } while (err < 0 && errno == EINTR);

    if (mPollFds[0].revents & POLLIN) {
        len = mPollBufferSize * sizeof(hf_manager_event);
        memset(pollBuffer.get(), 0, len);
        len = TEMP_FAILURE_RETRY(read(mFd, pollBuffer.get(), len));
        if (len < 0) {
            ALOGE("eventLooper read failed err=%zd errno=%d\n", len, errno);
            len = 0;
        }
        if (len % sizeof(hf_manager_event)) {
            ALOGE("eventLooper len failed err=%zu errno=%d\n", len, errno);
            len = 0;
        }
        count = len / sizeof(hf_manager_event);
        buffer = pollBuffer.get();
        for (ssize_t i = 0; i < count; ++i) {
            convertEvent(&event, &buffer[i]);
            callback(&event);
        }
    }
    return count;
}
