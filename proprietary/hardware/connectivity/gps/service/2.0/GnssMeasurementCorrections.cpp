/*
 * Copyright (C) 2019 The Android Open Source Project
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

#define LOG_TAG "GnssMeasurementCorrections"

#include "GnssMeasurementCorrections.h"
#include <log/log.h>

namespace android {
namespace hardware {
namespace gnss {
namespace measurement_corrections {
namespace V1_0 {
namespace implementation {

sp<V1_0::IMeasurementCorrectionsCallback> GnssMeasurementCorrections::sMeasureCallbackCbIface = nullptr;

MeasurementCorrectionCallbacks_ext GnssMeasurementCorrections::sMeasurementCorrectionCbs = {
    .size = sizeof(MeasurementCorrectionCallbacks_ext),
    .set_capabilities_cb = setCapabilitiesCb
};

GnssMeasurementCorrections::GnssMeasurementCorrections(
        const MeasurementCorrectionInterface* mesCorrectionIface)
        : mMeasurementCorrectionInterface(mesCorrectionIface){}


// Methods from V1_0::IMeasurementCorrections follow.
Return<bool> GnssMeasurementCorrections::setCorrections(const MeasurementCorrections& corrections) {
    if (mMeasurementCorrectionInterface == nullptr) {
        ALOGE("%s: GnssMeasurementCorrections interface is unavailable", __func__);
        return false;
    }
    ::MeasurementCorrections correctData;
    memset(&correctData, 0x00, sizeof(::MeasurementCorrections));
    correctData.latitudeDegrees = corrections.latitudeDegrees;
    correctData.latitudeDegrees = corrections.latitudeDegrees;
    correctData.longitudeDegrees = corrections.longitudeDegrees;
    correctData.altitudeMeters = corrections.altitudeMeters;
    correctData.horizontalPositionUncertaintyMeters = corrections.horizontalPositionUncertaintyMeters;
    correctData.verticalPositionUncertaintyMeters = corrections.verticalPositionUncertaintyMeters;
    correctData.toaGpsNanosecondsOfWeek =
            static_cast<unsigned long long>(corrections.toaGpsNanosecondsOfWeek);

    correctData.num_satCorrection = corrections.satCorrections.size();

    ALOGD("setCorrections size: %d", static_cast<int>(corrections.satCorrections.size()));
/*    ALOGD("corrections = lat: %f, lng: %f, alt: %f, hUnc: %f, vUnc: %f, toa: %llu, "
          "satCorrections.size: %d",
          corrections.latitudeDegrees, corrections.longitudeDegrees, corrections.altitudeMeters,
          corrections.horizontalPositionUncertaintyMeters,
          corrections.verticalPositionUncertaintyMeters,
          static_cast<unsigned long long>(corrections.toaGpsNanosecondsOfWeek),
          static_cast<int>(corrections.satCorrections.size()));*/
    int i = 0;
    for (auto ssc : corrections.satCorrections) {
        ::SingleSatCorrection* satCorrection = &correctData.satCorrections[i];
        satCorrection->singleSatCorrectionFlags = ssc.singleSatCorrectionFlags;
        satCorrection->constellation = static_cast<::GnssConstellationType>(ssc.constellation);
        satCorrection->svid = ssc.svid;
        satCorrection->carrierFrequencyHz = ssc.carrierFrequencyHz;
        satCorrection->probSatIsLos = ssc.probSatIsLos;
        satCorrection->excessPathLengthMeters = ssc.excessPathLengthMeters;
        satCorrection->excessPathLengthUncertaintyMeters = ssc.excessPathLengthUncertaintyMeters;
        satCorrection->reflectingPlane.latitudeDegrees = ssc.reflectingPlane.latitudeDegrees;
        satCorrection->reflectingPlane.longitudeDegrees = ssc.reflectingPlane.longitudeDegrees;
        satCorrection->reflectingPlane.altitudeMeters = ssc.reflectingPlane.altitudeMeters;
        satCorrection->reflectingPlane.azimuthDegrees = ssc.reflectingPlane.azimuthDegrees;
        /*ALOGD("singleSatCorrection = flags: %d, constellation: %d, svid: %d, cfHz: %f, probLos: %f,"
              " epl: %f, eplUnc: %f",
              static_cast<int>(ssc.singleSatCorrectionFlags),
              static_cast<int>(ssc.constellation),
              static_cast<int>(ssc.svid), ssc.carrierFrequencyHz,
              ssc.probSatIsLos, ssc.excessPathLengthMeters,
              ssc.excessPathLengthUncertaintyMeters);
        ALOGD("reflecting plane = lat: %f, lng: %f, alt: %f, azm: %f",
              ssc.reflectingPlane.latitudeDegrees,
              ssc.reflectingPlane.longitudeDegrees,
              ssc.reflectingPlane.altitudeMeters,
              ssc.reflectingPlane.azimuthDegrees);*/

        if (++i >= GNSS_MAX_SVS) {
            correctData.num_satCorrection = GNSS_MAX_SVS;
            break;
        }
    }

    return mMeasurementCorrectionInterface->meac_set_corrections(&correctData);
}

Return<bool> GnssMeasurementCorrections::setCallback(
        const sp<V1_0::IMeasurementCorrectionsCallback>& callback) {
    if (mMeasurementCorrectionInterface == nullptr) {
        ALOGE("%s: GnssMeasurementCorrections interface is unavailable", __func__);
        return false;
    }

    sMeasureCallbackCbIface = callback;

    return mMeasurementCorrectionInterface->meac_set_callback(&sMeasurementCorrectionCbs);
}

void GnssMeasurementCorrections::setCapabilitiesCb(uint32_t capabilities) {
    if (sMeasureCallbackCbIface == nullptr) {
        ALOGE("%s: MeasurementCorrection Callback Interface configured incorrectly", __func__);
        return;
    }

    auto ret = sMeasureCallbackCbIface->setCapabilitiesCb(capabilities);
    if (!ret.isOk()) {
        ALOGE("%s: Unable to invoke callback", __func__);
    }
}


}  // namespace implementation
}  // namespace V1_0
}  // namespace measurement_corrections
}  // namespace gnss
}  // namespace hardware
}  // namespace android
