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

#ifndef VENDOR_HARDWARE_MEDIATEK_MTKPOWER_V1_0_MTKPERF_H
#define VENDOR_HARDWARE_MEDIATEK_MTKPOWER_V1_0_MTKPERF_H

#include <vendor/mediatek/hardware/mtkpower/1.0/IMtkPerf.h>

#include <hwbinder/IPCThreadState.h>

#include <hidl/Status.h>

#include <hidl/MQDescriptor.h>
#include <hardware/hardware.h>
#include <hardware/power.h>

namespace vendor {
namespace mediatek {
namespace hardware {
namespace mtkpower {
namespace V1_0 {
namespace implementation {

using ::vendor::mediatek::hardware::mtkpower::V1_0::IMtkPerf;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::IPCThreadState;
using ::android::sp;
using ::android::hardware::hidl_vec;

struct MtkPerf : public IMtkPerf {
    MtkPerf();
    ~MtkPerf();

    Return<int32_t> perfLockAcquire(int32_t hdl, uint32_t duration, const hidl_vec<int32_t>& boostList, int32_t reserved)  override;
    Return<void> perfLockRelease(int32_t hdl, int32_t reserved)  override;
};

extern "C" IMtkPerf* HIDL_FETCH_IMtkPerf(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace mtkpower
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

#endif  // VENDOR_HARDWARE_MEDIATEK_MTKPOWER_V1_0_MTKPERF_H

