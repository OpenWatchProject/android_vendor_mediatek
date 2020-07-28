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

#ifndef _DIRECT_CHANNEL_MANAGER_H_
#define _DIRECT_CHANNEL_MANAGER_H_

#include <hardware/sensors.h>
#include <utils/Mutex.h>

#include <unordered_map>
#include <vector>

#include "SensorManager.h"
#include "DirectChannel.h"

class DirectChannelManager {
public:
    static DirectChannelManager *getInstance();
    ~DirectChannelManager();
    int addDirectChannel(const struct sensors_direct_mem_t *mem);
    int removeDirectChannel(int channel_handle);
    int configDirectReport(int sensor_handle, int channel_handle, int rate_level);
    void sendDirectReportEvent(const sensors_event_t *nev, size_t n);

protected:
    DirectChannelManager();
    DirectChannelManager(const DirectChannelManager& other);
    DirectChannelManager& operator = (const DirectChannelManager& other);

private:
    int mergeDirectReportRequest(int handle);
    int stopAllDirectReportOnChannel(
            int channel_handle, std::vector<int32_t> *unstoppedSensors);
    int64_t rateLevelToDeviceSamplingPeriodNs(int handle, int rateLevel) const;

    struct DirectChannelTimingInfo {
        uint64_t lastTimestamp;
        int rateLevel;
    };

    static android::Mutex sInstanceLock;
    static DirectChannelManager *sInstance;
    android::Mutex mDirectChannelLock;
    std::unordered_map<int32_t,
            std::unordered_map<int32_t, DirectChannelTimingInfo> > mSensorToChannel;
    std::unordered_map<int32_t, std::unique_ptr<DirectChannelBase>> mDirectChannel;
    std::unordered_map<int32_t, int> mSensorRateLevel;
    int32_t mDirectChannelHandle;
    SensorManager *mSensorManager;
    SensorConnection *mDirectConnection;
};
#endif
