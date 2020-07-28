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

#ifndef ANDROID_HARDWARE_AUDIO_DEVICE_H
#define ANDROID_HARDWARE_AUDIO_DEVICE_H

#include PATH(android/hardware/audio/FILE_VERSION/IDevice.h)

#include "ParametersUtil.h"

#include <memory>

#include <hardware/audio.h>
#include <media/AudioParameter.h>

#include <hidl/Status.h>

#include <hardware/audio_mtk.h>

#include <hidl/MQDescriptor.h>

#include <VersionUtils.h>

#include <vendor/mediatek/hardware/audio/5.1/IAudioParameterChangedCallback.h>
#include <vendor/mediatek/hardware/audio/5.1/IMTKPrimaryDevice.h>
#include <vendor/mediatek/hardware/audio/5.1/IMTKPrimaryDeviceCallback.h>

namespace android {
namespace hardware {
namespace audio {
namespace CPP_VERSION {
namespace implementation {

using ::android::sp;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::audio::common::CPP_VERSION::implementation::AudioInputFlagBitfield;
using ::android::hardware::audio::common::CPP_VERSION::implementation::AudioOutputFlagBitfield;
using namespace ::android::hardware::audio::common::CPP_VERSION;
using namespace ::android::hardware::audio::CPP_VERSION;

using ::vendor::mediatek::hardware::audio::V5_1::IAudioParameterChangedCallback;
using ::vendor::mediatek::hardware::audio::V5_1::IMTKPrimaryDeviceCallback;
using ::vendor::mediatek::hardware::audio::V5_1::IMTKPrimaryDevice;

struct Device : public IDevice, public ParametersUtil {
    explicit Device(audio_hw_device_mtk_t *device);

    // Methods from ::android::hardware::audio::CPP_VERSION::IDevice follow.
    Return<Result> initCheck()  override;
    Return<Result> setMasterVolume(float volume)  override;
    Return<void> getMasterVolume(getMasterVolume_cb _hidl_cb)  override;
    Return<Result> setMicMute(bool mute)  override;
    Return<void> getMicMute(getMicMute_cb _hidl_cb)  override;
    Return<Result> setMasterMute(bool mute)  override;
    Return<void> getMasterMute(getMasterMute_cb _hidl_cb)  override;
    Return<void> getInputBufferSize(const AudioConfig &config,
                                    getInputBufferSize_cb _hidl_cb)  override;

    std::tuple<Result, sp<IStreamOut>> openOutputStreamImpl(int32_t ioHandle,
                                                            const DeviceAddress& device,
                                                            const AudioConfig& config,
                                                            AudioOutputFlagBitfield flags,
                                                            AudioConfig* suggestedConfig);
    std::tuple<Result, sp<IStreamIn>> openInputStreamImpl(
        int32_t ioHandle, const DeviceAddress& device, const AudioConfig& config,
        AudioInputFlagBitfield flags, AudioSource source, AudioConfig* suggestedConfig);

    Return<void> openOutputStream(int32_t ioHandle, const DeviceAddress& device,
                                  const AudioConfig& config, AudioOutputFlagBitfield flags,
                                  const SourceMetadata& sourceMetadata,
                                  openOutputStream_cb _hidl_cb) override;
    Return<void> openInputStream(int32_t ioHandle, const DeviceAddress& device,
                                 const AudioConfig& config, AudioInputFlagBitfield flags,
                                 const SinkMetadata& sinkMetadata,
                                 openInputStream_cb _hidl_cb) override;

    Return<bool> supportsAudioPatches()  override;
    Return<void> createAudioPatch(const hidl_vec<AudioPortConfig>& sources,
                                  const hidl_vec<AudioPortConfig> &sinks,
                                  createAudioPatch_cb _hidl_cb)  override;
    Return<Result> releaseAudioPatch(int32_t patch)  override;
    Return<void> getAudioPort(const AudioPort &port, getAudioPort_cb _hidl_cb)  override;
    Return<Result> setAudioPortConfig(const AudioPortConfig &config)  override;

    Return<Result> setScreenState(bool turnedOn)  override;

    Return<void> getHwAvSync(getHwAvSync_cb _hidl_cb) override;
    Return<void> getParameters(const hidl_vec<ParameterValue>& context,
                               const hidl_vec<hidl_string>& keys,
                               getParameters_cb _hidl_cb) override;
    Return<Result> setParameters(const hidl_vec<ParameterValue>& context,
                                 const hidl_vec<ParameterValue>& parameters) override;
    Return<void> getMicrophones(getMicrophones_cb _hidl_cb) override;
    Return<Result> setConnectedState(const DeviceAddress& address, bool connected) override;

    Return<void> debug(const hidl_handle& fd, const hidl_vec<hidl_string>& options) override;

    // Utility methods for extending interfaces.
    Result analyzeStatus(const char* funcName, int status,
                         const std::vector<int>& ignoreErrors = {});
    void closeInputStream(audio_stream_in_t *stream);
    void closeOutputStream(audio_stream_out_t *stream);
    audio_hw_device_mtk_t *device() const { return mDevice; }

    Return<Result> setupParametersCallback(const sp<IMTKPrimaryDeviceCallback> &callback);
    Return<Result> setAudioParameterChangedCallback(const sp<IAudioParameterChangedCallback>& callback);
    Return<Result> clearAudioParameterChangedCallback();

private:
    audio_hw_device_mtk_t *mDevice;

    virtual ~Device();

    // Methods from ParametersUtil.
    char *halGetParameters(const char *keys) override;
    int halSetParameters(const char *keysAndValues) override;

    uint32_t version() const { return mDevice->common.version; }

    sp<IMTKPrimaryDeviceCallback> mCallback;
    sp<IAudioParameterChangedCallback> mAudioParameterChangedCallback;
    static int audioParameterChangedCallback(const char *param, void *cookie);
    static int syncCallback(device_parameters_callback_event_t event, audio_hw_device_set_parameters_callback_t *param, void *cookie);
};

}  // namespace implementation
}  // namespace CPP_VERSION
}  // namespace audio
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_AUDIO_DEVICE_H
