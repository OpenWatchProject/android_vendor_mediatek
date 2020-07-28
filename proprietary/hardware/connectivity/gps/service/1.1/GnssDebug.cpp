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

#define LOG_TAG "GnssHAL_GnssDebugInterface"

#include <log/log.h>

#include "GnssDebug.h"

namespace android {
namespace hardware {
namespace gnss {
namespace V1_0 {
namespace implementation {

GnssDebug::GnssDebug(const GpsDebugInterface_ext* gpsDebugIface) : mGnssDebugIface(gpsDebugIface) {}

// Methods from ::android::hardware::gnss::V1_0::IGnssDebug follow.
Return<void> GnssDebug::getDebugData(getDebugData_cb _hidl_cb)  {
    /*
     * This is a new interface and hence there is no way to retrieve the
     * debug data from the HAL.
     */
    if (mGnssDebugIface) {
        ::DebugData debugData;
        IGnssDebug::DebugData data;
        bool ret = mGnssDebugIface->get_internal_state(&debugData);
        if (ret) {
            data.position = (IGnssDebug::PositionDebug){
                .valid = debugData.position.valid,
                .latitudeDegrees = debugData.position.latitudeDegrees,
                .longitudeDegrees = debugData.position.longitudeDegrees,
                .altitudeMeters = debugData.position.altitudeMeters,
                .speedMetersPerSec = debugData.position.speedMetersPerSec,
                .bearingDegrees = debugData.position.bearingDegrees,
                .horizontalAccuracyMeters = debugData.position.horizontalAccuracyMeters,
                .verticalAccuracyMeters = debugData.position.verticalAccuracyMeters,
                .speedAccuracyMetersPerSecond = debugData.position.speedAccuracyMetersPerSecond,
                .bearingAccuracyDegrees = debugData.position.bearingAccuracyDegrees,
                .ageSeconds = debugData.position.ageSeconds
            };
            data.time = (IGnssDebug::TimeDebug){
                .timeEstimate = debugData.time.timeEstimate,
                .timeUncertaintyNs = debugData.time.timeUncertaintyNs,
                .frequencyUncertaintyNsPerSec = debugData.time.frequencyUncertaintyNsPerSec,
            };

            int count = 0;
            for (; count < GNSS_MAX_SVS; count++) {
                if (debugData.satelliteDataArray[count].svid == 0) {
                    break;
                }
            }
            data.satelliteDataArray.resize(count);
            for (int i = 0; i < count; i++) {
                auto entry = debugData.satelliteDataArray[i];
                data.satelliteDataArray[i] = (IGnssDebug::SatelliteData) {
                    .svid = entry.svid,
                    .constellation = (V1_0::GnssConstellationType) entry.constellation,
                    .ephemerisType = (IGnssDebug::SatelliteEphemerisType) entry.ephemerisType,
                    .ephemerisSource = (IGnssDebug::SatelliteEphemerisSource)entry.ephemerisSource,
                    .ephemerisHealth = (IGnssDebug::SatelliteEphemerisHealth)entry.ephemerisHealth,
                    .ephemerisAgeSeconds = entry.ephemerisAgeSeconds,
                    .serverPredictionIsAvailable = entry.serverPredictionIsAvailable,
                    .serverPredictionAgeSeconds = entry.serverPredictionAgeSeconds
                };
            }
            _hidl_cb(data);
        }
    }
    return Void();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace gnss
}  // namespace hardware
}  // namespace android
