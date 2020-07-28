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

#define LOG_TAG "vendor.mediatek.hardware.keymaster_attestation@1.1-impl"
#include "KeymasterDevice.h"

#include <cutils/log.h>
#include <dlfcn.h>

namespace vendor {
namespace mediatek {
namespace hardware {
namespace keymaster_attestation {
namespace V1_1 {
namespace implementation {

KeymasterDevice::KeymasterDevice()
{
	int rc;

	rc = hw_get_module_by_class(KMSETKEY_HARDWARE_MODULE_ID, NULL, &module);
	if (rc) {
		ALOGE("Unable to get %s module: %d\n", KMSETKEY_HARDWARE_MODULE_ID, rc);
		module = nullptr;
	}

	rc = kmsetkey_open(module, &device);
	if (rc) {
		ALOGE("Unable to open %s HAL: %d\n", KMSETKEY_HARDWARE_MODULE_ID, rc);
		device = nullptr;
	}
}

KeymasterDevice::~KeymasterDevice()
{
	int rc;

	if (device != nullptr) {
		rc = kmsetkey_close(device);
		if (rc < 0)
			ALOGE("Unable to close %s HAL: %d\n", KMSETKEY_HARDWARE_MODULE_ID, rc);
	}

	if (module != nullptr)
		dlclose(module->dso);
}

Return<vendor::mediatek::hardware::keymaster_attestation::V1_0::StatusCode> KeymasterDevice::attestKeyInstall(const hidl_vec<uint8_t> &data)
{
	int rc;

	if (device == nullptr) {
		ALOGE("No %s HAL exists\n", KMSETKEY_HARDWARE_MODULE_ID);
		return vendor::mediatek::hardware::keymaster_attestation::V1_0::StatusCode::UNIMPLEMENTED;
	}

	rc = device->attest_key_install(data.data(), data.size());
	if (rc) {
		ALOGE("call %s HAL fail: %d\n", KMSETKEY_HARDWARE_MODULE_ID, rc);
		return vendor::mediatek::hardware::keymaster_attestation::V1_0::StatusCode::KEYINSTALLFAILED;
	} else {
		ALOGI("call %s HAL success!\n", KMSETKEY_HARDWARE_MODULE_ID);
		return vendor::mediatek::hardware::keymaster_attestation::V1_0::StatusCode::OK;
	}
}

Return<StatusCode> KeymasterDevice::attestKeyCheck(const hidl_vec<uint8_t> &data)
{
	int rc;

	if (device == nullptr) {
		ALOGE("No %s HAL exists\n", KMSETKEY_HARDWARE_MODULE_ID);
		return StatusCode::UNIMPLEMENTED;
	}

	rc = device->attest_key_check(data.data(), data.size());
	if (rc) {
		ALOGE("call %s HAL fail: %c\n", KMSETKEY_HARDWARE_MODULE_ID, rc);
		return StatusCode::KEYCHECKFAILED;
	} else {
		ALOGI("call %s HAL success!\n", KMSETKEY_HARDWARE_MODULE_ID);
		return StatusCode::OK;
	}
}

IKeymasterDevice* HIDL_FETCH_IKeymasterDevice(const char* name)
{
	ALOGI("%s: Fetching keymaster device name %s", __func__, name);
	return new KeymasterDevice();
}

}  // namespace implementation
}  // namespace V1_1
}  // namespace keymaster_attestation
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor
