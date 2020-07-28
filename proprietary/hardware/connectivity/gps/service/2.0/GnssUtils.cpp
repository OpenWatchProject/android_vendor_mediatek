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

#include "GnssUtils.h"

namespace android {
namespace hardware {
namespace gnss {
namespace V2_0 {
namespace implementation {

V2_0::GnssLocation convertToGnssLocation2_0(GpsLocation_ext* location) {
    V2_0::GnssLocation gnssLocation = {};
    if (location != nullptr) {
        gnssLocation.v1_0 = {
            .gnssLocationFlags = static_cast<uint16_t>(location->legacyLocation.flags & 0xff),
            .latitudeDegrees = location->legacyLocation.latitude,
            .longitudeDegrees = location->legacyLocation.longitude,
            .altitudeMeters = location->legacyLocation.altitude,
            .speedMetersPerSec = location->legacyLocation.speed,
            .bearingDegrees = location->legacyLocation.bearing,
            .horizontalAccuracyMeters = location->horizontalAccuracyMeters,
            .verticalAccuracyMeters = location->verticalAccuracyMeters,
            .speedAccuracyMetersPerSecond = location->speedAccuracyMetersPerSecond,
            .bearingAccuracyDegrees = location->bearingAccuracyDegrees,
            .timestamp = location->legacyLocation.timestamp
        };

        ElapsedRealtime timestamp = {
                .flags = location->elapsedRealtime.flags,
                .timestampNs = location->elapsedRealtime.timestampNs,
                .timeUncertaintyNs = location->elapsedRealtime.timeUncertaintyNs
        };

        gnssLocation.elapsedRealtime = timestamp;
    }

    return gnssLocation;
}

V1_0::GnssLocation convertToGnssLocation1_0(GpsLocation_ext* location) {
    V1_0::GnssLocation gnssLocation = {};
    if (location != nullptr) {
        gnssLocation = {
            .gnssLocationFlags = static_cast<uint16_t>(location->legacyLocation.flags & 0xff),
            .latitudeDegrees = location->legacyLocation.latitude,
            .longitudeDegrees = location->legacyLocation.longitude,
            .altitudeMeters = location->legacyLocation.altitude,
            .speedMetersPerSec = location->legacyLocation.speed,
            .bearingDegrees = location->legacyLocation.bearing,
            .horizontalAccuracyMeters = location->horizontalAccuracyMeters,
            .verticalAccuracyMeters = location->verticalAccuracyMeters,
            .speedAccuracyMetersPerSecond = location->speedAccuracyMetersPerSecond,
            .bearingAccuracyDegrees = location->bearingAccuracyDegrees,
            .timestamp = location->legacyLocation.timestamp
        };
    }

    return gnssLocation;
}

GnssLocation convertToGnssLocation(FlpLocation* flpLocation) {
    GnssLocation gnssLocation = {};
    if (flpLocation != nullptr) {
        gnssLocation.v1_0 = {.gnssLocationFlags = 0,  // clear here and set below
                        .latitudeDegrees = flpLocation->latitude,
                        .longitudeDegrees = flpLocation->longitude,
                        .altitudeMeters = flpLocation->altitude,
                        .speedMetersPerSec = flpLocation->speed,
                        .bearingDegrees = flpLocation->bearing,
                        .horizontalAccuracyMeters = flpLocation->accuracy,
                        .verticalAccuracyMeters = 0,
                        .speedAccuracyMetersPerSecond = 0,
                        .bearingAccuracyDegrees = 0,
                        .timestamp = flpLocation->timestamp};
        // FlpLocation flags different from GnssLocation flags
        if (flpLocation->flags & FLP_LOCATION_HAS_LAT_LONG) {
            gnssLocation.v1_0.gnssLocationFlags |= GPS_LOCATION_HAS_LAT_LONG;
        }
        if (flpLocation->flags & FLP_LOCATION_HAS_ALTITUDE) {
            gnssLocation.v1_0.gnssLocationFlags |= GPS_LOCATION_HAS_ALTITUDE;
        }
        if (flpLocation->flags & FLP_LOCATION_HAS_SPEED) {
            gnssLocation.v1_0.gnssLocationFlags |= GPS_LOCATION_HAS_SPEED;
        }
        if (flpLocation->flags & FLP_LOCATION_HAS_BEARING) {
            gnssLocation.v1_0.gnssLocationFlags |= GPS_LOCATION_HAS_BEARING;
        }
        if (flpLocation->flags & FLP_LOCATION_HAS_ACCURACY) {
            gnssLocation.v1_0.gnssLocationFlags |= GPS_LOCATION_HAS_HORIZONTAL_ACCURACY;
        }
    }

    return gnssLocation;
}

}  // namespace implementation
}  // namespace V2_0
}  // namespace gnss
}  // namespace hardware
}  // namespace android
