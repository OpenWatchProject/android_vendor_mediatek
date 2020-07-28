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

#define LOG_TAG "high_freq_sensor_test"

#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>

#include <memory>
#include <thread>

#include <log/log.h>

#include <HfManager.h>

int main(int argc, char **argv) {
    int fd = -1;
    int sensor = 0, enable = 0, delay = 0, latency = 0;

    if (argc != 5) {
        ALOGE("usage:         execute        sensor  enable  delay  latency\n");
        ALOGE("      ./high_freq_sensor_test   1        1   5000000    0\n");
        return -1;
    }

    sensor = atoi(argv[1]);
    enable = atoi(argv[2]);
    delay = atoi(argv[3]);
    latency = atoi(argv[4]);
    ALOGI("sensor=%d enable=%d delay=%d latency=%d\n", sensor, enable, delay, latency);

    ALOGI("start new mHfManager\n");
    std::unique_ptr<HfManager> mHfManager(new HfManager());
    fd = mHfManager->getFd();
    std::thread looperThread([fd]() {
        ALOGI("start looper\n");
        std::unique_ptr<HfLooper> mHfLooper(new HfLooper(fd, 64));
        while (1) {
            ssize_t err = mHfLooper->eventLooper([](hf_sensors_event_t const * event) {
                ALOGI("Receiver: id:%d, time:%" PRId64 ", [%f %f %f]\n",
                event->sensor_id, event->timestamp,
                event->data[0], event->data[1], event->data[2]);
            });
            switch (err) {
            case -ENODEV:
                ALOGE("looper stop nodevice error\n");
                break;
            }
        };
    });
    looperThread.detach();
    ALOGI("start findSensor\n");
    if (mHfManager->findSensor(sensor) < 0)
        return -2;
    if (enable) {
        ALOGI("start enableSensor\n");
        if (mHfManager->enableSensor(sensor, delay) < 0)
            return -3;
    } else {
        ALOGI("start disableSensor\n");
        if (mHfManager->disableSensor(sensor) < 0)
            return -4;
    }
    while (1)
        sleep(1);
    return 0;
}
