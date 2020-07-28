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

#ifndef MTK_HARDWARE_DEVICE_HAL_INTERFACE_H
#define MTK_HARDWARE_DEVICE_HAL_INTERFACE_H

#include <system/audio.h>
#include <utils/Errors.h>
#include <utils/RefBase.h>
#include <utils/String8.h>

namespace android {

class MTKDeviceHalInterface : public RefBase
{
public:
    // Set global audio parameters.
    virtual status_t setParameters(const String8& kvPairs) = 0;

    // Get global audio parameters.
    virtual status_t getParameters(const String8& keys, String8 *values) = 0;

protected:
    // Subclasses can not be constructed directly by clients.
    MTKDeviceHalInterface() {}

    // The destructor automatically closes the device.
    virtual ~MTKDeviceHalInterface() {}
};

} // namespace android

#endif // MTK_HARDWARE_DEVICE_HAL_INTERFACE_H