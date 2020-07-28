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


#ifndef android_hardware_gnss_V1_1_GnssConfiguration_H_
#define android_hardware_gnss_V1_1_GnssConfiguration_H_

#include <android/hardware/gnss/1.1/IGnssConfiguration.h>
#include <hidl/Status.h>
#include <hardware/gps.h>
#include <hardware/gps_mtk.h>

namespace android {
namespace hardware {
namespace gnss {
namespace V1_1 {
namespace implementation {

using ::android::hardware::gnss::V1_1::IGnssConfiguration;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_string;
using ::android::sp;

using BlacklistedSource = V1_1::IGnssConfiguration::BlacklistedSource;

/*
 * Interface for passing GNSS configuration info from platform to HAL.
 */
struct GnssConfiguration : public IGnssConfiguration {
    GnssConfiguration(const GnssConfigurationInterface_ext* gnssConfigIface);

    /*
     * Methods from ::android::hardware::gnss::V1_0::IGnssConfiguration follow.
     * These declarations were generated from IGnssConfiguration.hal.
     */
    Return<bool> setSuplVersion(uint32_t version) override;
    Return<bool> setSuplMode(uint8_t mode) override;
    Return<bool> setSuplEs(bool enabled) override;
    Return<bool> setLppProfile(uint8_t lppProfile) override;
    Return<bool> setGlonassPositioningProtocol(uint8_t protocol) override;
    Return<bool> setEmergencySuplPdn(bool enable) override;
    Return<bool> setGpsLock(uint8_t lock) override;

    // Methods from ::android::hardware::gnss::V1_1::IGnssConfiguration follow.
    Return<bool> setBlacklist(const hidl_vec<BlacklistedSource>& blacklist) override;

 private:
    const GnssConfigurationInterface_ext* mGnssConfigIface = nullptr;

};

}  // namespace implementation
}  // namespace V1_1
}  // namespace gnss
}  // namespace hardware
}  // namespace android

#endif  // android_hardware_gnss_V1_1_GnssConfiguration_H_
