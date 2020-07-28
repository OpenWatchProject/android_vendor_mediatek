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

#ifndef ANDROID_HARDWARE_DEVICE_HAL_HIDL_H
#define ANDROID_HARDWARE_DEVICE_HAL_HIDL_H

#include <android/hardware/audio/5.0/IDevice.h>
#include "MTKDeviceHalInterface.h"
#include "ConversionHelperHidl.h"

using ::android::hardware::audio::V5_0::IDevice;
using ::android::hardware::Return;

namespace android {

class MTKDeviceHalHidl : public MTKDeviceHalInterface, public ConversionHelperHidl
{
public:
    // Set global audio parameters.
    virtual status_t setParameters(const String8& kvPairs);

    // Get global audio parameters.
    virtual status_t getParameters(const String8& keys, String8 *values);

private:
    friend class MTKDevicesFactoryHalHidl;
    sp<IDevice> mDevice;

    // Can not be constructed directly by clients.
    explicit MTKDeviceHalHidl(const sp<IDevice>& device);

    // The destructor automatically closes the device.
    virtual ~MTKDeviceHalHidl();
};

} // namespace android

#endif // ANDROID_HARDWARE_DEVICE_HAL_HIDL_H
