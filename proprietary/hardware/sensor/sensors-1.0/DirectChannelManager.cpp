/*
 * Copyright (C) 2015 The Android Open Source Project
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
#define LOG_TAG "DirectChannelManager"

#include <sstream>
#include "DirectChannelManager.h"

android::Mutex DirectChannelManager::sInstanceLock;
DirectChannelManager *DirectChannelManager::sInstance = NULL;
DirectChannelManager *DirectChannelManager::getInstance() {
    if (sInstance == NULL) {
        android::Mutex::Autolock autoLock(sInstanceLock);
        if (sInstance == NULL) {
            sInstance = new DirectChannelManager;
        }
    }
    return sInstance;
}

DirectChannelManager::DirectChannelManager() {
    mDirectChannelHandle = 1;
    mSensorToChannel.emplace(ID_ACCELEROMETER,
                             std::unordered_map<int32_t, DirectChannelTimingInfo>());
    mSensorToChannel.emplace(ID_GYROSCOPE,
                             std::unordered_map<int32_t, DirectChannelTimingInfo>());
    mSensorToChannel.emplace(ID_MAGNETIC,
                             std::unordered_map<int32_t, DirectChannelTimingInfo>());
    mSensorToChannel.emplace(ID_GYROSCOPE_UNCALIBRATED,
                             std::unordered_map<int32_t, DirectChannelTimingInfo>());
    mSensorToChannel.emplace(ID_MAGNETIC_UNCALIBRATED,
                             std::unordered_map<int32_t, DirectChannelTimingInfo>());

    mSensorManager = SensorManager::getInstance();
    mDirectConnection = mSensorManager->createSensorConnection(numFds);
    mSensorManager->setDirectConnection(mDirectConnection);
}

DirectChannelManager::~DirectChannelManager() {
    mSensorManager->removeSensorConnection(mDirectConnection);
    mSensorManager = NULL;
}

void DirectChannelManager::sendDirectReportEvent(const sensors_event_t *nev, size_t n) {
    // short circuit to avoid lock operation
    if (n == 0) {
        return;
    }

    // no intention to block sensor delivery thread. when lock is needed ignore
    // the event (this only happens when the channel is reconfiured, so it's ok
    if (mDirectChannelLock.tryLock() == android::NO_ERROR) {
        while (n--) {
            auto i = mSensorToChannel.find(nev->sensor - ID_OFFSET);
            if (i != mSensorToChannel.end()) {
                for (auto &j : i->second) {
                    mDirectChannel[j.first]->write(nev);
                }
            }
            ++nev;
        }
        mDirectChannelLock.unlock();
    }
}

int DirectChannelManager::mergeDirectReportRequest(int handle) {
    int maxRateLevel = SENSOR_DIRECT_RATE_STOP;
    int ret = 0, activateChg = false, rateChg = false;

    auto j = mSensorToChannel.find(handle);
    if (j != mSensorToChannel.end()) {
        for (auto &i : j->second) {
            maxRateLevel = std::max(i.second.rateLevel, maxRateLevel);
        }
    }

    auto k = mSensorRateLevel.find(handle);
    if (k != mSensorRateLevel.end()) {
        if (maxRateLevel != SENSOR_DIRECT_RATE_STOP) {
            if (k->second != maxRateLevel) {
                rateChg = true;
            }
            mSensorRateLevel.erase(handle);
            mSensorRateLevel.insert(std::make_pair(handle, maxRateLevel));
        } else {
            activateChg = true;
            mSensorRateLevel.erase(handle);
        }
    } else {
        if (maxRateLevel != SENSOR_DIRECT_RATE_STOP) {
            activateChg = true;
            rateChg = true;
            mSensorRateLevel.insert(std::make_pair(handle, maxRateLevel));
        }
    }

    int64_t period = rateLevelToDeviceSamplingPeriodNs(handle, maxRateLevel);
    if (maxRateLevel !=  SENSOR_DIRECT_RATE_STOP) {
        if (rateChg)
            ret = mSensorManager->batch(mDirectConnection, handle, period, 0);
        if (activateChg)
            ret = mSensorManager->activate(mDirectConnection, handle, true);
    } else {
        if (activateChg)
            ret = mSensorManager->activate(mDirectConnection, handle, false);
    }
    return ret;
}

int DirectChannelManager::addDirectChannel(const struct sensors_direct_mem_t *mem) {
    std::unique_ptr<DirectChannelBase> ch;
    int ret = android::NO_MEMORY;

    android::Mutex::Autolock autoLock(mDirectChannelLock);
    for (const auto& c : mDirectChannel) {
        if (c.second->memoryMatches(mem)) {
            // cannot reusing same memory
            return android::BAD_VALUE;
        }
    }
    switch(mem->type) {
        case SENSOR_DIRECT_MEM_TYPE_ASHMEM:
            ch = std::make_unique<AshmemDirectChannel>(mem);
            break;
        case SENSOR_DIRECT_MEM_TYPE_GRALLOC:
            ch = std::make_unique<GrallocDirectChannel>(mem);
            break;
        default:
            ret = android::INVALID_OPERATION;
    }

    if (ch) {
        if (ch->isValid()) {
            ret = mDirectChannelHandle++;
            mDirectChannel.insert(std::make_pair(ret, std::move(ch)));
        } else {
            ret = ch->getError();
            ALOGW("Direct channel object(type:%d) has error %d upon init", mem->type, ret);
        }
    }

    return ret;
}

int DirectChannelManager::removeDirectChannel(int channel_handle) {
    // make sure no active sensor in this channel
    std::vector<int32_t> activeSensorList;
    stopAllDirectReportOnChannel(channel_handle, &activeSensorList);

    // sensor service is responsible for stop all sensors before remove direct
    // channel. Thus, this is an error.
    if (!activeSensorList.empty()) {
        std::stringstream ss;
        std::copy(activeSensorList.begin(), activeSensorList.end(),
                std::ostream_iterator<int32_t>(ss, ","));
        ALOGW("Removing channel %d when sensors (%s) are not stopped.",
                channel_handle, ss.str().c_str());
    }

    // remove the channel record
    android::Mutex::Autolock autoLock(mDirectChannelLock);
    mDirectChannel.erase(channel_handle);
    return android::NO_ERROR;
}

int DirectChannelManager::stopAllDirectReportOnChannel(
        int channel_handle, std::vector<int32_t> *activeSensorList) {
    android::Mutex::Autolock autoLock(mDirectChannelLock);
    if (mDirectChannel.find(channel_handle) == mDirectChannel.end()) {
        return android::BAD_VALUE;
    }

    std::vector<int32_t> sensorToStop;
    for (auto &it : mSensorToChannel) {
        auto j = it.second.find(channel_handle);
        if (j != it.second.end()) {
            it.second.erase(j);
            if (it.second.empty()) {
                sensorToStop.push_back(it.first);
            }
        }
    }

    if (activeSensorList != nullptr) {
        *activeSensorList = sensorToStop;
    }

    // re-evaluate and send config for all sensor that need to be stopped
    int ret = false;
    for (auto sensor_handle : sensorToStop) {
        ret = mergeDirectReportRequest(sensor_handle);
    }
    return ret;
}

int DirectChannelManager::configDirectReport(int sensor_handle,
        int channel_handle, int rate_level) {
    ALOGI("DirectChannel handle=%d, channel_handle=%d, rate_level=%d",
        sensor_handle, channel_handle, rate_level);
    if (sensor_handle == -1 && rate_level == SENSOR_DIRECT_RATE_STOP) {
        return stopAllDirectReportOnChannel(channel_handle, nullptr);
    }

    // clamp to fast
    if (rate_level > SENSOR_DIRECT_RATE_FAST) {
        rate_level = SENSOR_DIRECT_RATE_FAST;
    }

    // manage direct channel data structure
    android::Mutex::Autolock autoLock(mDirectChannelLock);
    auto i = mDirectChannel.find(channel_handle);
    if (i == mDirectChannel.end()) {
        return android::BAD_VALUE;
    }

    auto j = mSensorToChannel.find(sensor_handle);
    if (j == mSensorToChannel.end()) {
        return android::BAD_VALUE;
    }

    j->second.erase(channel_handle);
    if (rate_level != SENSOR_DIRECT_RATE_STOP) {
        j->second.insert(std::make_pair(channel_handle, (DirectChannelTimingInfo){0, rate_level}));
    }
    int ret = mergeDirectReportRequest(sensor_handle);
    if (rate_level == SENSOR_DIRECT_RATE_STOP) {
        /* must return 0 when rate stop */
        ret = 0;
    } else {
        /* must return (sensor_handle + ID_OFFSET) when non rate stop, cts use return value to check databuffer */
        ret = (ret < 0) ? ret : (sensor_handle + ID_OFFSET);
    }
    return ret;
}

int64_t DirectChannelManager::rateLevelToDeviceSamplingPeriodNs(int handle, int rateLevel) const {
    int64_t rate;

    if (mSensorToChannel.find(handle) == mSensorToChannel.end()) {
        return INT64_MAX;
    }

    switch (rateLevel) {
        case SENSOR_DIRECT_RATE_VERY_FAST:
            // No sensor support VERY_FAST, fall through
        case SENSOR_DIRECT_RATE_FAST:
            if (handle != ID_MAGNETIC && handle != ID_MAGNETIC_UNCALIBRATED)
                rate = 2500000; // 400Hz
            else
                rate = 20000000;
            break;
        case SENSOR_DIRECT_RATE_NORMAL:
            rate = 20000000; // 50 Hz
            break;
        default:
            rate = INT64_MAX;
            break;
    }
    return rate;
}
