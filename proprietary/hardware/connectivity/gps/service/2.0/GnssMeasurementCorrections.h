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

#ifndef ANDROID_HARDWARE_GNSS_GNSSMEASUREMENT_CORRECTIONS_V1_0_H
#define ANDROID_HARDWARE_GNSS_GNSSMEASUREMENT_CORRECTIONS_V1_0_H

#include <android/hardware/gnss/measurement_corrections/1.0/IMeasurementCorrections.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <hardware/gps_mtk.h>

namespace android {
namespace hardware {
namespace gnss {
namespace measurement_corrections {
namespace V1_0 {
namespace implementation {

using ::android::sp;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;

struct GnssMeasurementCorrections : public IMeasurementCorrections {
    GnssMeasurementCorrections(const MeasurementCorrectionInterface* mesCorrectionIface);

    // Methods from V1_0::IMeasurementCorrections follow.
    Return<bool> setCorrections(const MeasurementCorrections& corrections) override;
    Return<bool> setCallback(const sp<V1_0::IMeasurementCorrectionsCallback>& callback) override;

    static void setCapabilitiesCb(uint32_t capabilities);

    static MeasurementCorrectionCallbacks_ext sMeasurementCorrectionCbs;

 private:
    const MeasurementCorrectionInterface* mMeasurementCorrectionInterface = nullptr;
    static sp<V1_0::IMeasurementCorrectionsCallback> sMeasureCallbackCbIface;
};

}  // namespace implementation
}  // namespace V1_0
}  // namespace measurement_corrections
}  // namespace gnss
}  // namespace hardware
}  // namespace android

#endif  // android_hardware_gnss_GnssMeasurement_corrections_V1_0_H_
