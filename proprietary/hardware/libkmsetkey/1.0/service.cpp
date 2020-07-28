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

#define LOG_TAG "vendor.mediatek.hardware.keymaster_attestation@1.0-service"

#include <vendor/mediatek/hardware/keymaster_attestation/1.0/IKeymasterDevice.h>

#include <hidl/HidlTransportSupport.h>
#include <hidl/LegacySupport.h>

using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using android::hardware::defaultPassthroughServiceImplementation;

using vendor::mediatek::hardware::keymaster_attestation::V1_0::IKeymasterDevice;

int main() {
    return defaultPassthroughServiceImplementation<IKeymasterDevice>();
}
