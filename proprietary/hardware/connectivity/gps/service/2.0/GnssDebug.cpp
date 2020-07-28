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
namespace V2_0 {
namespace implementation {

#define GNSS_DEBUG_UNKNOWN_UTC_TIME     (1483228800000ULL) // 1/1/2017 00:00 GMT
#define GNSS_DEBUG_UNKNOWN_UTC_TIME_UNC (1000)
#define GNSS_DEBUG_UNKNOWN_FREQUENCY_UNC (5.0E4)

GnssDebug::GnssDebug(const GpsDebugInterface_ext* gpsDebugIface) : mGnssDebugIface(gpsDebugIface) {}

// Methods from ::android::hardware::gnss::V1_0::IGnssDebug follow.
Return<void> GnssDebug::getDebugData(getDebugData_cb _hidl_cb)  {
    if (mGnssDebugIface) {
        ::DebugData debugData;
        V1_0::IGnssDebug::DebugData data;
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

            data.time = (V1_0::IGnssDebug::TimeDebug){
                .timeEstimate = static_cast<int64_t>((debugData.time.timeEstimate != 0) ?
                        debugData.time.timeEstimate :
                        GNSS_DEBUG_UNKNOWN_UTC_TIME),
                .timeUncertaintyNs = (debugData.time.timeUncertaintyNs != 0) ?
                        debugData.time.timeUncertaintyNs :
                        (float)(GNSS_DEBUG_UNKNOWN_UTC_TIME_UNC),
                .frequencyUncertaintyNsPerSec = (debugData.time.frequencyUncertaintyNsPerSec != 0) ?
                        (float) debugData.time.frequencyUncertaintyNsPerSec :
                        (float)(GNSS_DEBUG_UNKNOWN_FREQUENCY_UNC)
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
                data.satelliteDataArray[i] = (V1_0::IGnssDebug::SatelliteData) {
                    .svid = entry.svid,
                    .constellation = (V1_0::GnssConstellationType) entry.constellation,
                    .ephemerisType = (V1_0::IGnssDebug::SatelliteEphemerisType) entry.ephemerisType,
                    .ephemerisSource = (V1_0::IGnssDebug::SatelliteEphemerisSource)entry.ephemerisSource,
                    .ephemerisHealth = (V1_0::IGnssDebug::SatelliteEphemerisHealth)entry.ephemerisHealth,
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

Return<void> GnssDebug::getDebugData_2_0(getDebugData_2_0_cb _hidl_cb)  {
    /*
     * This is a new interface and hence there is no way to retrieve the
     * debug data from the HAL.
     */
    if (mGnssDebugIface) {
        ::DebugData gpsDebugData;
        V2_0::IGnssDebug::DebugData data;
        bool ret = mGnssDebugIface->get_internal_state(&gpsDebugData);
        if (ret) {
            data.position = (IGnssDebug::PositionDebug){
                .valid = gpsDebugData.position.valid,
                .latitudeDegrees = gpsDebugData.position.latitudeDegrees,
                .longitudeDegrees = gpsDebugData.position.longitudeDegrees,
                .altitudeMeters = gpsDebugData.position.altitudeMeters,
                .speedMetersPerSec = gpsDebugData.position.speedMetersPerSec,
                .bearingDegrees = gpsDebugData.position.bearingDegrees,
                .horizontalAccuracyMeters = gpsDebugData.position.horizontalAccuracyMeters,
                .verticalAccuracyMeters = gpsDebugData.position.verticalAccuracyMeters,
                .speedAccuracyMetersPerSecond = gpsDebugData.position.speedAccuracyMetersPerSecond,
                .bearingAccuracyDegrees = gpsDebugData.position.bearingAccuracyDegrees,
                .ageSeconds = gpsDebugData.position.ageSeconds
            };
            data.time = (V1_0::IGnssDebug::TimeDebug){
                .timeEstimate = static_cast<int64_t>((gpsDebugData.time.timeEstimate != 0) ?
                        gpsDebugData.time.timeEstimate :
                        GNSS_DEBUG_UNKNOWN_UTC_TIME),
                .timeUncertaintyNs = (gpsDebugData.time.timeUncertaintyNs != 0) ?
                        gpsDebugData.time.timeUncertaintyNs :
                        (float)(GNSS_DEBUG_UNKNOWN_UTC_TIME_UNC),
                .frequencyUncertaintyNsPerSec = (gpsDebugData.time.frequencyUncertaintyNsPerSec != 0) ?
                        (float) gpsDebugData.time.frequencyUncertaintyNsPerSec :
                        (float)(GNSS_DEBUG_UNKNOWN_FREQUENCY_UNC)
            };

            int count = 0;
            for (; count < GNSS_MAX_SVS; count++) {
                if (gpsDebugData.satelliteDataArray[count].svid == 0) {
                    break;
                }
            }
            data.satelliteDataArray.resize(count);
            for (int i = 0; i < count; i++) {
                auto entry = gpsDebugData.satelliteDataArray[i];
                data.satelliteDataArray[i].v1_0 = (V1_0::IGnssDebug::SatelliteData) {
                    .svid = entry.svid,
                    .constellation = V1_0::GnssConstellationType::UNKNOWN,
                    .ephemerisType = (IGnssDebug::SatelliteEphemerisType) entry.ephemerisType,
                    .ephemerisSource = (IGnssDebug::SatelliteEphemerisSource)entry.ephemerisSource,
                    .ephemerisHealth = (IGnssDebug::SatelliteEphemerisHealth)entry.ephemerisHealth,
                    .ephemerisAgeSeconds = entry.ephemerisAgeSeconds,
                    .serverPredictionIsAvailable = entry.serverPredictionIsAvailable,
                    .serverPredictionAgeSeconds = entry.serverPredictionAgeSeconds
                };
                data.satelliteDataArray[i].constellation = (V2_0::GnssConstellationType) entry.constellation;
            }
            _hidl_cb(data);
        }
    }
    return Void();
}

}  // namespace implementation
}  // namespace V2_0
}  // namespace gnss
}  // namespace hardware
}  // namespace android
