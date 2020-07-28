/*
 * Copyright (C) 2018 The Android Open Source Project
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
#define LOG_TAG "GnssMeasurement"

#include "GnssMeasurement.h"

#include <log/log.h>
#include <utils/SystemClock.h>

namespace android {
namespace hardware {
namespace gnss {
namespace V2_0 {
namespace implementation {

sp<V2_0::IGnssMeasurementCallback> GnssMeasurement::sGnssMeasureCbIface = nullptr;

GpsMeasurementCallbacks_ext GnssMeasurement::sGnssMeasurementCbs = {
    .size = sizeof(GpsMeasurementCallbacks_ext),
    .measurement_callback = gpsMeasurementCb,
    .gnss_measurement_callback = gnssMeasurementCb
};

GnssMeasurement::GnssMeasurement(const GpsMeasurementInterface_ext* gpsMeasurementIface)
    : mGnssMeasureIface(gpsMeasurementIface) {}

void GnssMeasurement::gnssMeasurementCb(GnssData_ext* halGnssData) {
    if (sGnssMeasureCbIface == nullptr) {
        ALOGE("%s: GNSSMeasurement Callback Interface configured incorrectly", __func__);
        return;
    }

    if (halGnssData == nullptr) {
        ALOGE("%s: Invalid GnssData from GNSS HAL", __func__);
        return;
    }

    V2_0::IGnssMeasurementCallback::GnssData gnssData;
    size_t measurementCount = std::min(halGnssData->measurement_count,
                                         static_cast<size_t>(V1_0::GnssMax::SVS_COUNT));
    gnssData.measurements.resize(measurementCount);

    for (size_t i = 0; i < measurementCount; i++) {
        auto entry = halGnssData->measurements[i];
        auto state = static_cast<GnssMeasurementState>(entry.legacyMeasurement.state);
        if (state & IGnssMeasurementCallback::GnssMeasurementState::STATE_TOW_DECODED) {
          state |= IGnssMeasurementCallback::GnssMeasurementState::STATE_TOW_KNOWN;
        }
        if (state & IGnssMeasurementCallback::GnssMeasurementState::STATE_GLO_TOD_DECODED) {
          state |= IGnssMeasurementCallback::GnssMeasurementState::STATE_GLO_TOD_KNOWN;
        }

        gnssData.measurements[i].v1_1.v1_0 = (V1_0::IGnssMeasurementCallback::GnssMeasurement){
            .flags = entry.legacyMeasurement.flags,
            .svid = entry.legacyMeasurement.svid,
            .constellation = V1_0::GnssConstellationType::UNKNOWN,
            .timeOffsetNs = entry.legacyMeasurement.time_offset_ns,
            .state = state,
            .receivedSvTimeInNs = entry.legacyMeasurement.received_sv_time_in_ns,
            .receivedSvTimeUncertaintyInNs =
                    entry.legacyMeasurement.received_sv_time_uncertainty_in_ns,
            .cN0DbHz = entry.legacyMeasurement.c_n0_dbhz,
            .pseudorangeRateMps = entry.legacyMeasurement.pseudorange_rate_mps,
            .pseudorangeRateUncertaintyMps =
                    entry.legacyMeasurement.pseudorange_rate_uncertainty_mps,
            .accumulatedDeltaRangeState = entry.legacyMeasurement.accumulated_delta_range_state,
            .accumulatedDeltaRangeM = entry.legacyMeasurement.accumulated_delta_range_m,
            .accumulatedDeltaRangeUncertaintyM =
                    entry.legacyMeasurement.accumulated_delta_range_uncertainty_m,
            .carrierFrequencyHz = entry.legacyMeasurement.carrier_frequency_hz,
            .carrierCycles = entry.legacyMeasurement.carrier_cycles,
            .carrierPhase = entry.legacyMeasurement.carrier_phase,
            .carrierPhaseUncertainty = entry.legacyMeasurement.carrier_phase_uncertainty,
            .multipathIndicator = static_cast<IGnssMeasurementCallback::GnssMultipathIndicator>(
                    entry.legacyMeasurement.multipath_indicator),
            .snrDb = entry.legacyMeasurement.snr_db,
            .agcLevelDb = entry.agc_level_db
        };
        /// v1.1
        gnssData.measurements[i].v1_1.accumulatedDeltaRangeState =
                    entry.legacyMeasurement.accumulated_delta_range_state;
        /// v2.0
        entry.codeType[7] = '\0';  // one bye word
        gnssData.measurements[i].codeType.setToExternal(entry.codeType, strlen(entry.codeType));

        gnssData.measurements[i].state = state;
        gnssData.measurements[i].constellation = static_cast<V2_0::GnssConstellationType>(
                    entry.legacyMeasurement.constellation);
    }

    auto clockVal = halGnssData->clock;
    gnssData.clock = {
        .gnssClockFlags = clockVal.flags,
        .leapSecond = clockVal.leap_second,
        .timeNs = clockVal.time_ns,
        .timeUncertaintyNs = clockVal.time_uncertainty_ns,
        .fullBiasNs = clockVal.full_bias_ns,
        .biasNs = clockVal.bias_ns,
        .biasUncertaintyNs = clockVal.bias_uncertainty_ns,
        .driftNsps = clockVal.drift_nsps,
        .driftUncertaintyNsps = clockVal.drift_uncertainty_nsps,
        .hwClockDiscontinuityCount = clockVal.hw_clock_discontinuity_count
    };

    /// v2.0
    ElapsedRealtime timestamp = {
            .flags = halGnssData->elapsedRealtime.flags,
            .timestampNs = halGnssData->elapsedRealtime.timestampNs,
            .timeUncertaintyNs = halGnssData->elapsedRealtime.timeUncertaintyNs
    };

    gnssData.elapsedRealtime = timestamp;

    auto ret = sGnssMeasureCbIface->gnssMeasurementCb_2_0(gnssData);
    if (!ret.isOk()) {
        ALOGE("%s: Unable to invoke callback", __func__);
    }
}

/*
 * The code in the following method has been moved here from GnssLocationProvider.
 * It converts GpsData to GnssData. This code is no longer required in
 * GnssLocationProvider since GpsData is deprecated and no longer part of the
 * GNSS interface.
 */
void GnssMeasurement::gpsMeasurementCb(GpsData*) {
}

// Methods from ::android::hardware::gnss::V1_0::IGnssMeasurement follow.
Return<V1_0::IGnssMeasurement::GnssMeasurementStatus>
GnssMeasurement::setCallback(const sp<V1_0::IGnssMeasurementCallback>&) {
    return V1_0::IGnssMeasurement::GnssMeasurementStatus::ERROR_GENERIC;
}


// Methods from ::android::hardware::gnss::V1_1::IGnssMeasurement follow.
Return<V1_0::IGnssMeasurement::GnssMeasurementStatus>
GnssMeasurement::setCallback_1_1(const sp<V1_1::IGnssMeasurementCallback>&, bool) {
    return V1_1::IGnssMeasurement::GnssMeasurementStatus::ERROR_GENERIC;
}

Return<V1_0::IGnssMeasurement::GnssMeasurementStatus> GnssMeasurement::setCallback_2_0(
    const sp<V2_0::IGnssMeasurementCallback>& callback, bool enableFullTracking) {

    if (mGnssMeasureIface == nullptr) {
        ALOGE("%s: GnssMeasure interface is unavailable", __func__);
        return GnssMeasurementStatus::ERROR_GENERIC;
    }
    sGnssMeasureCbIface = callback;

    return static_cast<GnssMeasurement::GnssMeasurementStatus>(
            mGnssMeasureIface->init(&sGnssMeasurementCbs, enableFullTracking));
}

Return<void> GnssMeasurement::close()  {
    if (mGnssMeasureIface == nullptr) {
        ALOGE("%s: GnssMeasure interface is unavailable", __func__);
    } else {
        mGnssMeasureIface->close();
    }
    return Void();
}

}  // namespace implementation
}  // namespace V2_0
}  // namespace gnss
}  // namespace hardware
}  // namespace android
