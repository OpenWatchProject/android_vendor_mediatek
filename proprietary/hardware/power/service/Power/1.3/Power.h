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

#ifndef ANDROID_HARDWARE_POWER_V2_0_POWER_H
#define ANDROID_HARDWARE_POWER_V2_0_POWER_H

#include <android/hardware/power/1.3/IPower.h>
//#include <vendor/mediatek/hardware/power/1.2/IPower.h>
#include <vendor/mediatek/hardware/mtkpower/1.0/IMtkPerf.h>
#include <vendor/mediatek/hardware/mtkpower/1.0/IMtkPower.h>

#include <hwbinder/IPCThreadState.h>

#include <hidl/Status.h>

#include <hidl/MQDescriptor.h>
#include <hardware/hardware.h>
#include <hardware/power.h>

namespace android {
namespace hardware {
namespace power {
namespace V1_3 {
namespace implementation {

//using ::android::hardware::power::V1_0::PowerHint;
using ::android::hardware::power::V1_3::IPower;
using ::android::hardware::power::V1_0::Feature;
using ::android::hardware::power::V1_0::PowerStatePlatformSleepState;
using ::android::hardware::power::V1_0::Status;
using ::android::hardware::power::V1_1::PowerStateSubsystem;
using ::android::hardware::power::V1_1::PowerStateSubsystemSleepState;
using ::vendor::mediatek::hardware::mtkpower::V1_0::IMtkPerf;
using ::vendor::mediatek::hardware::mtkpower::V1_0::IMtkPower;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::IPCThreadState;
using ::android::sp;

struct Power : public IPower {
    //Power(power_module_t* module);
    Power();
    ~Power();
    Return<void> setInteractive(bool interactive)  override;
    Return<void> powerHint(::android::hardware::power::V1_0::PowerHint hint, int32_t data)  override;
    Return<void> powerHintAsync(::android::hardware::power::V1_0::PowerHint hint, int32_t data) override;
    Return<void> powerHintAsync_1_2(::android::hardware::power::V1_2::PowerHint hint, int32_t data) override;
    Return<void> setFeature(Feature feature, bool activate)  override;
    Return<void> getPlatformLowPowerStats(getPlatformLowPowerStats_cb _hidl_cb)  override;
    Return<void> getSubsystemLowPowerStats(getSubsystemLowPowerStats_cb _hidl_cb) override;
    Return<void> powerHintAsync_1_3(::android::hardware::power::V1_3::PowerHint hint, int32_t data) override;

#if 0
  private:
    power_module_t* mModule;
#else
  private:
    void powerHintInternal(int32_t hint, int32_t data);
#endif
};

extern "C" IPower* HIDL_FETCH_IPower(const char* name);

} // namespace implementation
}  // namespace V1_3
}  // namespace power
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_POWER_V2_0_POWER_H
