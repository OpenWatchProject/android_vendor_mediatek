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

#include <string.h>

#define LOG_TAG "MTKDevicesFactoryHalHidl"
//#define LOG_NDEBUG 0

#include <android/hardware/audio/5.0/IDevice.h>
#include "HalDeathHandler.h"
#include <utils/Log.h>

#include "ConversionHelperHidl.h"
#include "MTKDeviceHalHidl.h"
#include "MTKDevicesFactoryHalHidl.h"

using ::android::hardware::audio::V5_0::IDevice;
using ::android::hardware::audio::V5_0::Result;
using ::android::hardware::Return;

namespace android {

// static
sp<MTKDevicesFactoryHalInterface> MTKDevicesFactoryHalInterface::create() {
    return new MTKDevicesFactoryHalHidl();
}

MTKDevicesFactoryHalHidl::MTKDevicesFactoryHalHidl() {
    mDevicesFactory = IDevicesFactory::getService();
    if (mDevicesFactory != 0) {
        // It is assumed that DevicesFactory is owned by rilproxy
        // But we don't want the same life cycle with rilproxy
        mDevicesFactory->linkToDeath(HalDeathHandler::getInstance(), 0 /*cookie*/);
    } else {
        ALOGE("Failed to obtain IDevicesFactory service.");
    }
}

MTKDevicesFactoryHalHidl::~MTKDevicesFactoryHalHidl() {
}

// static
status_t MTKDevicesFactoryHalHidl::openDevice(const char *name, sp<MTKDeviceHalInterface> *device) {
    if (mDevicesFactory == 0) return NO_INIT;
    Result retval = Result::NOT_INITIALIZED;
    Return<void> ret = mDevicesFactory->openDevice(
            name,
            [&](Result r, const sp<IDevice>& result) {
                retval = r;
                if (retval == Result::OK) {
                    *device = new MTKDeviceHalHidl(result);
                }
            }
    );
    if (ret.isOk()) {
        if (retval == Result::OK) return OK;
        else if (retval == Result::INVALID_ARGUMENTS) return BAD_VALUE;
        else return NO_INIT;
    }
    return FAILED_TRANSACTION;
}

} // namespace android
