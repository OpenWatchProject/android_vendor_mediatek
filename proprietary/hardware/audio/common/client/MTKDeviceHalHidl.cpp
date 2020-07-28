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

#include <stdio.h>

#define LOG_TAG "MTKDeviceHalHidl"

#include <android/hardware/audio/5.0/IPrimaryDevice.h>
#include <cutils/native_handle.h>
#include <hwbinder/IPCThreadState.h>
#include <utils/Log.h>
#include "MTKDeviceHalHidl.h"

using ::android::hardware::audio::V5_0::IPrimaryDevice;
using ::android::hardware::audio::V5_0::ParameterValue;
using ::android::hardware::audio::V5_0::Result;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;

namespace android {

MTKDeviceHalHidl::MTKDeviceHalHidl(const sp<IDevice>& device)
    : ConversionHelperHidl("Device"), mDevice(device) {
}

MTKDeviceHalHidl::~MTKDeviceHalHidl() {
    if (mDevice != 0) {
        mDevice.clear();
        hardware::IPCThreadState::self()->flushCommands();
    }
}

status_t MTKDeviceHalHidl::setParameters(const String8& kvPairs) {
    if (mDevice == 0) return NO_INIT;
    hidl_vec<ParameterValue> hidlParams;
    status_t status = parametersFromHal(kvPairs, &hidlParams);
    if (status != OK) return status;
    return processReturn("setParameters", mDevice->setParameters({} /* context */, hidlParams));
}

status_t MTKDeviceHalHidl::getParameters(const String8& keys, String8 *values) {
    values->clear();
    if (mDevice == 0) return NO_INIT;
    hidl_vec<hidl_string> hidlKeys;
    status_t status = keysFromHal(keys, &hidlKeys);
    if (status != OK) return status;
    Result retval;
    Return<void> ret = mDevice->getParameters(
            {} /* context */,
            hidlKeys,
            [&](Result r, const hidl_vec<ParameterValue>& parameters) {
                retval = r;
                if (retval == Result::OK) {
                    parametersToHal(parameters, values);
                }
            }
    );
    return processReturn("getParameters", ret, retval);
}

} // namespace android
