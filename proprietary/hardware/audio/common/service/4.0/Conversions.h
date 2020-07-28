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

#ifndef android_hardware_audio_V4_0_Conversions_H_
#define android_hardware_audio_V4_0_Conversions_H_
#define MAJOR_VERSION 4
#define MINOR_VERSION 0

#include <VersionMacro.h>


#include <string>

#include <android/hardware/audio/4.0/types.h>
#include <system/audio.h>

namespace android {
namespace hardware {
namespace audio {
namespace V4_0 {
namespace implementation {

using ::android::hardware::audio::V4_0::DeviceAddress;

std::string deviceAddressToHal(const DeviceAddress &address);

bool halToMicrophoneCharacteristics(MicrophoneInfo* pDst,
                                    const struct audio_microphone_characteristic_t& src);

}  // namespace implementation
}  // namespace V4_0
}  // namespace audio
}  // namespace hardware
}  // namespace android

#endif  // android_hardware_audio_V4_0_Conversions_H_
