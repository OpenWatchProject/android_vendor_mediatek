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

#define LOG_TAG "vendor.mediatek.hardware.keymaster_attestation@1.0-impl"
#include "KeymasterDevice.h"

#include <cutils/log.h>
#include <dlfcn.h>

namespace vendor {
namespace mediatek {
namespace hardware {
namespace keymaster_attestation {
namespace V1_0 {
namespace implementation {

KeymasterDevice::KeymasterDevice() {

	device = nullptr;

	ALOGI("get module by class: %s\n", KMSETKEY_HARDWARE_MODULE_ID);
	int rc = hw_get_module_by_class(KMSETKEY_HARDWARE_MODULE_ID, NULL, &module);

	if(rc) {
		ALOGE("Unable to open key_attestation HAL.\n");
	}

	rc = kmsetkey_open(module, &device);
	if(rc) {
		ALOGE("Error to open kmsetkey device.\n");
	}

}

KeymasterDevice::~KeymasterDevice() {

	if (device != nullptr) {
		int ret = kmsetkey_close(device);
		if (ret < 0) {
			ALOGE("Unable to close key_attestation HAL");
		}
	}
	dlclose(module->dso);
}

Return<StatusCode> KeymasterDevice::attestKeyInstall(const hidl_vec<uint8_t>& data) {

	int ret;

	if (device == nullptr) {
		ALOGE("No key_attestation HAL exists.\n");
		return StatusCode::UNIMPLEMENTED;
	}

	ret = device->attest_key_install(data.data(), data.size());
	if (ret) {
		ALOGE("call attestation key HAL fail!\n");
		return StatusCode::KEYINSTALLFAILED;
	} else {
		ALOGI("call attestation key HAL success!\n");
	}

	return StatusCode::OK;
}


IKeymasterDevice* HIDL_FETCH_IKeymasterDevice(const char* name) {

	ALOGI("%s Fetching keymaster device name %s", __func__, name);
	return new KeymasterDevice();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace keymaster_attestation
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor
