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

#ifndef _HF_MANAGER_H_
#define _HF_MANAGER_H_

#include <fcntl.h>
#include <poll.h>
#include <mutex>
#include <bitset>
#include <memory>

#include <linux/ioctl.h>

enum {
    HIGH_FREQUENCY_SENSOR_GYRO = 1,
    HIGH_FREQUENCY_SENSOR_ACCEL,
    HIGH_FREQUENCY_SENSOR_MAG,
    HIGH_FREQUENCY_SENSOR_PRESSURE,
    HIGH_FREQUENCY_SENSOR_MAX,
};

#define HIGH_FREQUENCY_ACCURANCY_UNRELIALE 0
#define HIGH_FREQUENCY_ACCURANCY_LOW       1
#define HIGH_FREQUENCY_ACCURANCY_MEDIUM    2
#define HIGH_FREQUENCY_ACCURANCY_HIGH      3

#define HF_MANAGER_SENSOR_DISABLE          0
#define HF_MANAGER_SENSOR_ENABLE           1
#define HF_MANAGER_SENSOR_FLUSH            2
#define HF_MANAGER_SENSOR_CALIBRATION      3
#define HF_MANAGER_SENSOR_CONFIG_CALI      4
#define HF_MANAGER_SENSOR_SELFTEST         5

#define HF_MANAGER_SENSOR_REGISTER_STATUS _IOWR('a', 1, unsigned int)

#define HIGH_FREQUENCY_GYRO_DIV 57295787.785569f

typedef struct hf_manager_cmd {
    uint8_t sensor_id;
    uint8_t action;
    int64_t delay;
    int64_t latency;
} __packed hf_manager_cmd;

typedef struct hf_manager_event {
    int64_t timestamp;
    uint8_t sensor_id;
    uint8_t accurancy;
    uint8_t action;
    uint8_t reserved;
    union {
        int32_t word[6];
        int8_t byte[0];
    };
} __packed hf_manager_event;

typedef struct hf_sensors_event_t {
    int64_t timestamp;
    uint8_t sensor_id;
    uint8_t accurancy;
    uint8_t reserved[2];
    union {
        float data[6];
    };
} __packed hf_sensors_event_t;

class HfManager {
public:
    HfManager();
    ~HfManager();

    int getFd(void);
    int findSensor(int sensor);
    int enableSensor(int sensor, int64_t delayNs);
    int enableSensor(int sensor, int64_t delayNs, int64_t latencyNs);
    int disableSensor(int sensor);

protected:
    int initCheck() const;
    int enableDisable(int sensor, int enable,
        int64_t delayNs, int64_t latencyNs);
    void disableAllSensor(void);
    bool checkRegisterStatus(int sensor);

private:
    int mFd;
    std::bitset<HIGH_FREQUENCY_SENSOR_MAX> sensorState;
    std::bitset<HIGH_FREQUENCY_SENSOR_MAX> sensorRegister;
};

class HfLooper {
public:
    HfLooper(int fd, size_t pollBufferSize);
    ~HfLooper();
    ssize_t eventLooper(void (*callback)(hf_sensors_event_t const *));

protected:
    int initCheck() const;

private:
    static constexpr int pollMaxBufferSize = 32;

    int mFd;
    size_t mPollBufferSize;
    struct pollfd mPollFds[1];
    int mNumPollFds;
    std::unique_ptr<hf_manager_event[]> pollBuffer;
};
#endif
