/*
 * Copyright (C) 2018 The Android Open Source Project
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

#define LOG_TAG "DevicesFactoryHAL"

#include "DevicesFactory.h"
#include "Device.h"
//#include "PrimaryDevice.h"
#include "MTKPrimaryDevice.h"

#include <string.h>

#include <android/log.h>

namespace android {
namespace hardware {
namespace audio {
namespace CPP_VERSION {
namespace implementation {

Return<void> DevicesFactory::openDevice(const hidl_string& moduleName, openDevice_cb _hidl_cb) {
    if (moduleName == AUDIO_HARDWARE_MODULE_ID_PRIMARY) {
        return openDevice<vendor::mediatek::hardware::audio::V5_1::implementation::MTKPrimaryDevice>(moduleName.c_str(), _hidl_cb);
    }
    return openDevice(moduleName.c_str(), _hidl_cb);
}
Return<void> DevicesFactory::openPrimaryDevice(openPrimaryDevice_cb _hidl_cb) {
    return openDevice<vendor::mediatek::hardware::audio::V5_1::implementation::MTKPrimaryDevice>(AUDIO_HARDWARE_MODULE_ID_PRIMARY, _hidl_cb);
}

Return<void> DevicesFactory::openDevice(const char* moduleName, openDevice_cb _hidl_cb) {
    return openDevice<::android::hardware::audio::V5_0::implementation::Device>(moduleName, _hidl_cb);
}

template <class DeviceShim, class Callback>
Return<void> DevicesFactory::openDevice(const char* moduleName, Callback _hidl_cb) {
    audio_hw_device_mtk_t* halDevice;
    Result retval(Result::INVALID_ARGUMENTS);
    sp<DeviceShim> result;
    int halStatus = loadAudioInterface(moduleName, (audio_hw_device_t **)&halDevice);
    if (halStatus == OK) {
        result = new DeviceShim(halDevice);
        retval = Result::OK;
    } else if (halStatus == -EINVAL) {
        retval = Result::NOT_INITIALIZED;
    }
    _hidl_cb(retval, result);
    return Void();
}

// static
int DevicesFactory::loadAudioInterface(const char *if_name, audio_hw_device_t **dev) {
    const hw_module_t *mod;
    int rc;

    rc = hw_get_module_by_class(AUDIO_HARDWARE_MODULE_ID, if_name, &mod);
    if (rc) {
        ALOGE("%s couldn't load audio hw module %s.%s (%s)", __func__, AUDIO_HARDWARE_MODULE_ID,
              if_name, strerror(-rc));
        goto out;
    }
    rc = audio_hw_device_open(mod, dev);
    if (rc) {
        ALOGE("%s couldn't open audio hw device in %s.%s (%s)", __func__, AUDIO_HARDWARE_MODULE_ID,
              if_name, strerror(-rc));
        goto out;
    }
    if ((*dev)->common.version < AUDIO_DEVICE_API_VERSION_MIN) {
        ALOGE("%s wrong audio hw device version %04x", __func__, (*dev)->common.version);
        rc = -EINVAL;
        audio_hw_device_close(*dev);
        goto out;
    }
    return OK;

out:
    *dev = NULL;
    return rc;
}

IDevicesFactory* HIDL_FETCH_IDevicesFactory(const char* name) {
    return strcmp(name, "default") == 0 ? new DevicesFactory() : nullptr;
}

}  // namespace implementation
}  // namespace CPP_VERSION
}  // namespace audio
}  // namespace hardware
}  // namespace android
