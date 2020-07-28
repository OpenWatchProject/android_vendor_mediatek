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

#ifndef ANDROID_HARDWARE_AUDIO_V2_0_DEVICE_H
#define ANDROID_HARDWARE_AUDIO_V2_0_DEVICE_H

#include <memory>

#include <media/AudioParameter.h>
#include <hardware/audio.h>
#include <hardware/audio_mtk.h>

#include <android/hardware/audio/2.0/IDevice.h>
#include <hidl/MQDescriptor.h>
#include <fmq/EventFlag.h>
#include <fmq/MessageQueue.h>
#include <hidl/Status.h>
#include <utils/Thread.h>

#include "ParametersUtil.h"
#include <vendor/mediatek/hardware/audio/2.1/IAudioParameterChangedCallback.h>
#include <vendor/mediatek/hardware/audio/2.1/IMTKPrimaryDevice.h>
#include <vendor/mediatek/hardware/audio/2.1/IMTKPrimaryDeviceCallback.h>

namespace android {
namespace hardware {
namespace audio {
namespace V2_0 {
namespace implementation {

using ::android::hardware::audio::common::V2_0::AudioConfig;
using ::android::hardware::audio::common::V2_0::AudioHwSync;
using ::android::hardware::audio::common::V2_0::AudioInputFlag;
using ::android::hardware::audio::common::V2_0::AudioOutputFlag;
using ::android::hardware::audio::common::V2_0::AudioPatchHandle;
using ::android::hardware::audio::common::V2_0::AudioPort;
using ::android::hardware::audio::common::V2_0::AudioPortConfig;
using ::android::hardware::audio::common::V2_0::AudioSource;
using ::android::hardware::audio::V2_0::DeviceAddress;
using ::android::hardware::audio::V2_0::IDevice;
using ::android::hardware::audio::V2_0::IStreamIn;
using ::android::hardware::audio::V2_0::IStreamOut;
using ::android::hardware::audio::V2_0::ParameterValue;
using ::android::hardware::audio::V2_0::Result;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_string;
using ::android::sp;
using ::vendor::mediatek::hardware::audio::V2_1::IAudioParameterChangedCallback;
using ::vendor::mediatek::hardware::audio::V2_1::IMTKPrimaryDeviceCallback;
using ::vendor::mediatek::hardware::audio::V2_1::IMTKPrimaryDevice;

struct Device : public IDevice, public ParametersUtil {
    typedef MessageQueue<uint8_t, kSynchronizedReadWrite> VOWDataMQ;
    typedef MessageQueue<IMTKPrimaryDevice::VOWReadParameters, kSynchronizedReadWrite> CommandMQVOWRead;
    typedef MessageQueue<IMTKPrimaryDevice::VOWReadStatus, kSynchronizedReadWrite> StatusMQVOWRead;

    explicit Device(audio_hw_device_mtk_t *device);

    // Methods from ::android::hardware::audio::V2_0::IDevice follow.
    Return<Result> initCheck()  override;
    Return<Result> setMasterVolume(float volume)  override;
    Return<void> getMasterVolume(getMasterVolume_cb _hidl_cb)  override;
    Return<Result> setMicMute(bool mute)  override;
    Return<void> getMicMute(getMicMute_cb _hidl_cb)  override;
    Return<Result> setMasterMute(bool mute)  override;
    Return<void> getMasterMute(getMasterMute_cb _hidl_cb)  override;
    Return<void> getInputBufferSize(
        const AudioConfig &config, getInputBufferSize_cb _hidl_cb)  override;
    Return<void> openOutputStream(
        int32_t ioHandle,
        const DeviceAddress &device,
        const AudioConfig &config,
        AudioOutputFlag flags,
        openOutputStream_cb _hidl_cb)  override;
    Return<void> openInputStream(
        int32_t ioHandle,
        const DeviceAddress &device,
        const AudioConfig &config,
        AudioInputFlag flags,
        AudioSource source,
        openInputStream_cb _hidl_cb)  override;
    Return<bool> supportsAudioPatches()  override;
    Return<void> createAudioPatch(
        const hidl_vec<AudioPortConfig> &sources,
        const hidl_vec<AudioPortConfig> &sinks,
        createAudioPatch_cb _hidl_cb)  override;
    Return<Result> releaseAudioPatch(int32_t patch)  override;
    Return<void> getAudioPort(const AudioPort &port, getAudioPort_cb _hidl_cb)  override;
    Return<Result> setAudioPortConfig(const AudioPortConfig &config)  override;
    Return<AudioHwSync> getHwAvSync()  override;
    Return<Result> setScreenState(bool turnedOn)  override;
    Return<void> getParameters(
        const hidl_vec<hidl_string> &keys, getParameters_cb _hidl_cb)  override;
    Return<Result> setParameters(const hidl_vec<ParameterValue> &parameters)  override;
    Return<void> debugDump(const hidl_handle &fd)  override;

    // Utility methods for extending interfaces.
    static Result analyzeStatus(const char *funcName, int status);
    void closeInputStream(audio_stream_in_t *stream);
    void closeOutputStream(audio_stream_out_t *stream);
    audio_hw_device_mtk_t *device() const { return mDevice; }

    Return<Result> setupParametersCallback(const sp<IMTKPrimaryDeviceCallback> &callback);
    //vow
    Return<void> prepareForVOWReading(uint32_t frameSize, uint32_t framesCount, IMTKPrimaryDevice::prepareForVOWReading_cb _hidl_cb);
    Return<void> getVoiceUnlockULTime(IMTKPrimaryDevice::getVoiceUnlockULTime_cb _hidl_cb);
    Return<Result> setVoiceUnlockSRC(uint32_t outSR, uint32_t outChannel);
    Return<Result> startVoiceUnlockDL();
    Return<Result> stopVoiceUnlockDL();
    Return<Result> getVoiceUnlockDLInstance();
    Return<Result> setAudioParameterChangedCallback(const sp<IAudioParameterChangedCallback>& callback);
    Return<Result> clearAudioParameterChangedCallback();

private:
    audio_hw_device_mtk_t *mDevice;

    virtual ~Device();

    std::unique_ptr<CommandMQVOWRead> mCommandMQVOWRead;
    std::unique_ptr<VOWDataMQ> mDataMQVOWRead;
    std::unique_ptr<StatusMQVOWRead> mStatusMQVOWRead;

    std::atomic<bool> mStopReadThread;
    sp<Thread> mReadThread;

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
}  // namespace V2_0
}  // namespace audio
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_AUDIO_V2_0_DEVICE_H
