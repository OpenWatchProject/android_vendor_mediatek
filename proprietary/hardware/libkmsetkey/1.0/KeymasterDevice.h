/*
 **
 ** Copyright 2016, The Android Open Source Project
 **
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 **
 **     http://www.apache.org/licenses/LICENSE-2.0
 **
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 */

#ifndef HIDL_GENERATED_vendor_mediatek_hardware_keymaster_attestation_V1_0_KeymasterDevice_H_
#define HIDL_GENERATED_vendor_mediatek_hardware_keymaster_attestation_V1_0_KeymasterDevice_H_

#include <vendor/mediatek/hardware/keymaster_attestation/1.0/IKeymasterDevice.h>
#include <vendor/mediatek/hardware/keymaster_attestation/1.0/types.h>
#include <hidl/Status.h>
#include "kmattest.h"

#include <hidl/MQDescriptor.h>
namespace vendor {
namespace mediatek {
namespace hardware {
namespace keymaster_attestation {
namespace V1_0 {
namespace implementation {

using ::vendor::mediatek::hardware::keymaster_attestation::V1_0::IKeymasterDevice;
using ::vendor::mediatek::hardware::keymaster_attestation::V1_0::StatusCode;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_string;
using ::android::sp;

class KeymasterDevice : public IKeymasterDevice {
	public:
		KeymasterDevice();
		~KeymasterDevice();

		// Methods from ::android::hardware::keymaster::V3_0::IKeymasterDevice follow.
		Return<StatusCode> attestKeyInstall(const hidl_vec<uint8_t>& data);

	private:
		kmsetkey_device* device;
		const hw_module_t* module;
};

extern "C" IKeymasterDevice* HIDL_FETCH_IKeymasterDevice(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace keymaster_attestation
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

#endif  // HIDL_GENERATED_vendor_mediatek_hardware_keymaster_attestation_V1_0_KeymasterDevice_H_
