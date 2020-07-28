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

#define LOG_TAG "GnssHAL_GnssMeasurementInterface"

#include "GnssMeasurement.h"

namespace android {
namespace hardware {
namespace gnss {
namespace V1_1 {
namespace implementation {

sp<V1_1::IGnssMeasurementCallback>
        GnssMeasurement::sGnssMeasureCbIface = nullptr;
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

    IGnssMeasurementCallback::GnssData gnssData;
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

        gnssData.measurements[i].v1_0 = (V1_0::IGnssMeasurementCallback::GnssMeasurement){
            .flags = entry.legacyMeasurement.flags,
            .svid = entry.legacyMeasurement.svid,
            .constellation = static_cast<V1_0::GnssConstellationType>(
                    entry.legacyMeasurement.constellation),
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
        gnssData.measurements[i].accumulatedDeltaRangeState =
                    entry.legacyMeasurement.accumulated_delta_range_state;
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

    auto ret = sGnssMeasureCbIface->gnssMeasurementCb(gnssData);
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
void GnssMeasurement::gpsMeasurementCb(GpsData* gpsData) {
    if (sGnssMeasureCbIface == nullptr) {
        ALOGE("%s: GNSSMeasurement Callback Interface configured incorrectly", __func__);
        return;
    }

    if (gpsData == nullptr) {
        ALOGE("%s: Invalid GpsData from GNSS HAL", __func__);
        return;
    }

    IGnssMeasurementCallback::GnssData gnssData;
    size_t measurementCount = std::min(gpsData->measurement_count,
                                         static_cast<size_t>(V1_0::GnssMax::SVS_COUNT));
    gnssData.measurements.resize(measurementCount);

    for (size_t i = 0; i < measurementCount; i++) {
        auto entry = gpsData->measurements[i];
        gnssData.measurements[i].v1_0.flags = entry.flags;
        gnssData.measurements[i].v1_0.svid = static_cast<int32_t>(entry.prn);
        if (entry.prn >= 1 && entry.prn <= 32) {
            gnssData.measurements[i].v1_0.constellation = V1_0::GnssConstellationType::GPS;
        } else {
            gnssData.measurements[i].v1_0.constellation =
                  V1_0::GnssConstellationType::UNKNOWN;
        }

        gnssData.measurements[i].v1_0.timeOffsetNs = entry.time_offset_ns;
        gnssData.measurements[i].v1_0.state = entry.state;
        gnssData.measurements[i].v1_0.receivedSvTimeInNs = entry.received_gps_tow_ns;
        gnssData.measurements[i].v1_0.receivedSvTimeUncertaintyInNs =
            entry.received_gps_tow_uncertainty_ns;
        gnssData.measurements[i].v1_0.cN0DbHz = entry.c_n0_dbhz;
        gnssData.measurements[i].v1_0.pseudorangeRateMps = entry.pseudorange_rate_mps;
        gnssData.measurements[i].v1_0.pseudorangeRateUncertaintyMps =
                entry.pseudorange_rate_uncertainty_mps;
        gnssData.measurements[i].v1_0.accumulatedDeltaRangeState =
                entry.accumulated_delta_range_state;
        gnssData.measurements[i].v1_0.accumulatedDeltaRangeM =
                entry.accumulated_delta_range_m;
        gnssData.measurements[i].v1_0.accumulatedDeltaRangeUncertaintyM =
                entry.accumulated_delta_range_uncertainty_m;

        if (entry.flags & GNSS_MEASUREMENT_HAS_CARRIER_FREQUENCY) {
            gnssData.measurements[i].v1_0.carrierFrequencyHz = entry.carrier_frequency_hz;
        } else {
            gnssData.measurements[i].v1_0.carrierFrequencyHz = 0;
        }

        if (entry.flags & GNSS_MEASUREMENT_HAS_CARRIER_PHASE) {
            gnssData.measurements[i].v1_0.carrierPhase = entry.carrier_phase;
        } else {
            gnssData.measurements[i].v1_0.carrierPhase = 0;
        }

        if (entry.flags & GNSS_MEASUREMENT_HAS_CARRIER_PHASE_UNCERTAINTY) {
            gnssData.measurements[i].v1_0.carrierPhaseUncertainty = entry.carrier_phase_uncertainty;
        } else {
            gnssData.measurements[i].v1_0.carrierPhaseUncertainty = 0;
        }

        gnssData.measurements[i].v1_0.multipathIndicator =
                static_cast<IGnssMeasurementCallback::GnssMultipathIndicator>(
                        entry.multipath_indicator);

        if (entry.flags & GNSS_MEASUREMENT_HAS_SNR) {
            gnssData.measurements[i].v1_0.snrDb = entry.snr_db;
        } else {
            gnssData.measurements[i].v1_0.snrDb = 0;
        }

        gnssData.measurements[i].v1_0.agcLevelDb = 0;
        gnssData.measurements[i].accumulatedDeltaRangeState = entry.accumulated_delta_range_state;
    }

    auto clockVal = gpsData->clock;
    static uint32_t discontinuity_count_to_handle_old_clock_type = 0;

    gnssData.clock.leapSecond = clockVal.leap_second;
    /*
     * GnssClock only supports the more effective HW_CLOCK type, so type
     * handling and documentation complexity has been removed.  To convert the
     * old GPS_CLOCK types (active only in a limited number of older devices),
     * the GPS time information is handled as an always discontinuous HW clock,
     * with the GPS time information put into the full_bias_ns instead - so that
     * time_ns - full_bias_ns = local estimate of GPS time. Additionally, the
     * sign of full_bias_ns and bias_ns has flipped between GpsClock &
     * GnssClock, so that is also handled below.
     */
    switch (clockVal.type) {
        case GPS_CLOCK_TYPE_UNKNOWN:
            // Clock type unsupported.
            ALOGE("Unknown clock type provided.");
            break;
        case GPS_CLOCK_TYPE_LOCAL_HW_TIME:
            // Already local hardware time. No need to do anything.
            break;
        case GPS_CLOCK_TYPE_GPS_TIME:
            // GPS time, need to convert.
            clockVal.flags |= GPS_CLOCK_HAS_FULL_BIAS;
            clockVal.full_bias_ns = clockVal.time_ns;
            clockVal.time_ns = 0;
            gnssData.clock.hwClockDiscontinuityCount =
                    discontinuity_count_to_handle_old_clock_type++;
            break;
    }

    gnssData.clock.timeNs = clockVal.time_ns;
    gnssData.clock.timeUncertaintyNs = clockVal.time_uncertainty_ns;
    /*
     * Definition of sign for full_bias_ns & bias_ns has been changed since N,
     * so flip signs here.
     */
    gnssData.clock.fullBiasNs = -(clockVal.full_bias_ns);
    gnssData.clock.biasNs = -(clockVal.bias_ns);
    gnssData.clock.biasUncertaintyNs = clockVal.bias_uncertainty_ns;
    gnssData.clock.driftNsps = clockVal.drift_nsps;
    gnssData.clock.driftUncertaintyNsps = clockVal.drift_uncertainty_nsps;
    gnssData.clock.gnssClockFlags = clockVal.flags;

    auto ret = sGnssMeasureCbIface->gnssMeasurementCb(gnssData);
    if (!ret.isOk()) {
        ALOGE("%s: Unable to invoke callback", __func__);
    }
}

// Methods from ::android::hardware::gnss::V1_0::IGnssMeasurement follow.
Return<V1_0::IGnssMeasurement::GnssMeasurementStatus>
GnssMeasurement::setCallback(const sp<V1_0::IGnssMeasurementCallback>&) {
    return V1_0::IGnssMeasurement::GnssMeasurementStatus::ERROR_GENERIC;
}


// Methods from ::android::hardware::gnss::V1_1::IGnssMeasurement follow.
Return<V1_0::IGnssMeasurement::GnssMeasurementStatus>
GnssMeasurement::setCallback_1_1(
        const sp<V1_1::IGnssMeasurementCallback>& callback,
        bool enableFullTracking) {

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
}  // namespace V1_1
}  // namespace gnss
}  // namespace hardware
}  // namespace android
