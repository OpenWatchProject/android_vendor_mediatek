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

#ifndef VENDOR_MEDIATEK_HARDWARE_POWER_V2_1_POWER_H
#define VENDOR_MEDIATEK_HARDWARE_POWER_V2_1_POWER_H

//#include <android/hardware/power/1.0/IPower.h>
//#include <vendor/mediatek/hardware/power/1.1/IPower.h>
#include <vendor/mediatek/hardware/power/2.1/IPower.h>
#include <hwbinder/IPCThreadState.h>

#include <hidl/Status.h>

#include <hidl/MQDescriptor.h>
#include <hardware/hardware.h>
#include <hardware/power.h>
namespace vendor {
namespace mediatek {
namespace hardware {
namespace power {
namespace V2_1 {
namespace implementation {

//using ::android::hardware::power::V1_0::PowerHint;
using ::android::hardware::power::V1_0::Feature;
using ::android::hardware::power::V1_0::PowerStatePlatformSleepState;
using ::android::hardware::power::V1_0::Status;
using ::android::hardware::power::V1_1::PowerStateSubsystem;
using ::android::hardware::power::V1_1::PowerStateSubsystemSleepState;
using ::vendor::mediatek::hardware::power::V2_1::IPower;
using MtkPowerCmd_2_0 = ::vendor::mediatek::hardware::power::V2_0::MtkPowerCmd;
using MtkPowerCmd_2_1 = ::vendor::mediatek::hardware::power::V2_1::MtkPowerCmd;
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
    Return<void> mtkPowerHint(::vendor::mediatek::hardware::power::V2_0::MtkPowerHint hint, int32_t data)  override;
    Return<void> mtkCusPowerHint(int32_t hint, int32_t data)  override;
    Return<void> notifyAppState(const hidl_string& packName, const hidl_string& actName, int32_t pid, MtkActState state)  override;
    Return<void> notifyAppState_2_1(const hidl_string& packName, const hidl_string& actName, int32_t pid, ::vendor::mediatek::hardware::power::V2_0::MtkActState state, int32_t uid)  override;
    Return<int32_t> querySysInfo(::vendor::mediatek::hardware::power::V2_0::MtkQueryCmd cmd, int32_t param)  override;
    Return<int32_t> scnReg()  override;
    Return<void> scnConfig(int32_t hdl, MtkPowerCmd_2_0 cmd, int32_t param1, int32_t param2, int32_t param3, int32_t param4)  override;
    Return<void> scnConfig_2_1(int32_t hdl, MtkPowerCmd_2_1 cmd, int32_t param1, int32_t param2, int32_t param3, int32_t param4)  override;
    Return<void> scnUnreg(int32_t hdl)  override;
    Return<void> scnEnable(int32_t hdl, int32_t timeout)  override;
    Return<void> scnDisable(int32_t hdl)  override;
    Return<void> scnUltraCfg(int32_t hdl, int32_t ultracmd, int32_t param1, int32_t param2, int32_t param3, int32_t param4)  override;
    Return<void> setSysInfo(int32_t type, const hidl_string& data)  override;

  private:
    void powerHintInternal(int32_t hint, int32_t data);
};

extern "C" IPower* HIDL_FETCH_IPower(const char* name);

}  // namespace implementation
}  // namespace V2_1
}  // namespace power
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

#endif  // VENDOR_MEDIATEK_HARDWARE_POWER_V2_1_POWER_H