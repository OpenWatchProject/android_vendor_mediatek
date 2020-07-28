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

#define LOG_TAG "MTKPrimaryDeviceHAL"

#include "MTKPrimaryDevice.h"
#include "Util.h"

#include <cmath>

using ::android::hardware::audio::V4_0::implementation::isGainNormalized;

namespace vendor {
namespace mediatek {
namespace hardware {
namespace audio {
namespace V4_1 {
namespace implementation {

MTKPrimaryDevice::MTKPrimaryDevice(audio_hw_device_mtk_t *device)
    : mDevice(new android::hardware::audio::V4_0::implementation::Device(device)) {}

MTKPrimaryDevice::~MTKPrimaryDevice() {}

// Methods from ::android::hardware::audio::V4_0::IDevice follow.
Return<Result> MTKPrimaryDevice::initCheck() {
    return mDevice->initCheck();
}

Return<Result> MTKPrimaryDevice::setMasterVolume(float volume) {
    return mDevice->setMasterVolume(volume);
}

Return<void> MTKPrimaryDevice::getMasterVolume(getMasterVolume_cb _hidl_cb) {
    return mDevice->getMasterVolume(_hidl_cb);
}

Return<Result> MTKPrimaryDevice::setMicMute(bool mute) {
    return mDevice->setMicMute(mute);
}

Return<void> MTKPrimaryDevice::getMicMute(getMicMute_cb _hidl_cb) {
    return mDevice->getMicMute(_hidl_cb);
}

Return<Result> MTKPrimaryDevice::setMasterMute(bool mute) {
    return mDevice->setMasterMute(mute);
}

Return<void> MTKPrimaryDevice::getMasterMute(getMasterMute_cb _hidl_cb) {
    return mDevice->getMasterMute(_hidl_cb);
}

Return<void> MTKPrimaryDevice::getInputBufferSize(const AudioConfig &config,
                                               getInputBufferSize_cb _hidl_cb) {
    return mDevice->getInputBufferSize(config, _hidl_cb);
}

Return<void> MTKPrimaryDevice::openOutputStream(int32_t ioHandle, const DeviceAddress& device,
                                             const AudioConfig& config,
                                             AudioOutputFlagBitfield flags,
                                             const SourceMetadata& sourceMetadata,
                                             openOutputStream_cb _hidl_cb) {
    return mDevice->openOutputStream(ioHandle, device, config, flags, sourceMetadata, _hidl_cb);
}

Return<void> MTKPrimaryDevice::openInputStream(int32_t ioHandle, const DeviceAddress& device,
                                            const AudioConfig& config, AudioInputFlagBitfield flags,
                                            const SinkMetadata& sinkMetadata,
                                            openInputStream_cb _hidl_cb) {
    return mDevice->openInputStream(ioHandle, device, config, flags, sinkMetadata, _hidl_cb);
}

Return<bool> MTKPrimaryDevice::supportsAudioPatches() {
    return mDevice->supportsAudioPatches();
}

Return<void> MTKPrimaryDevice::createAudioPatch(const hidl_vec<AudioPortConfig> &sources,
                                             const hidl_vec<AudioPortConfig> &sinks,
                                             createAudioPatch_cb _hidl_cb) {
    return mDevice->createAudioPatch(sources, sinks, _hidl_cb);
}

Return<Result> MTKPrimaryDevice::releaseAudioPatch(int32_t patch) {
    return mDevice->releaseAudioPatch(patch);
}

Return<void> MTKPrimaryDevice::getAudioPort(const AudioPort& port, getAudioPort_cb _hidl_cb) {
    return mDevice->getAudioPort(port, _hidl_cb);
}

Return<Result> MTKPrimaryDevice::setAudioPortConfig(const AudioPortConfig& config) {
    return mDevice->setAudioPortConfig(config);
}

Return<Result> MTKPrimaryDevice::setScreenState(bool turnedOn) {
    return mDevice->setScreenState(turnedOn);
}

Return<void> MTKPrimaryDevice::getHwAvSync(getHwAvSync_cb _hidl_cb) {
    return mDevice->getHwAvSync(_hidl_cb);
}
Return<void> MTKPrimaryDevice::getParameters(const hidl_vec<ParameterValue>& context,
                                          const hidl_vec<hidl_string>& keys,
                                          getParameters_cb _hidl_cb) {
    return mDevice->getParameters(context, keys, _hidl_cb);
}
Return<Result> MTKPrimaryDevice::setParameters(const hidl_vec<ParameterValue>& context,
                                            const hidl_vec<ParameterValue>& parameters) {
    return mDevice->setParameters(context, parameters);
}
Return<void> MTKPrimaryDevice::getMicrophones(getMicrophones_cb _hidl_cb) {
    return mDevice->getMicrophones(_hidl_cb);
}
Return<Result> MTKPrimaryDevice::setConnectedState(const DeviceAddress& address, bool connected) {
    return mDevice->setConnectedState(address, connected);
}

// Methods from ::android::hardware::audio::V4_0::IPrimaryDevice follow.
Return<Result> MTKPrimaryDevice::setVoiceVolume(float volume) {
    if (!isGainNormalized(volume)) {
        ALOGW("Can not set a voice volume (%f) outside [0,1]", volume);
        return Result::INVALID_ARGUMENTS;
    }
    return mDevice->analyzeStatus("set_voice_volume",
               mDevice->device()->set_voice_volume(mDevice->device(), volume));
}

Return<Result> MTKPrimaryDevice::setMode(AudioMode mode) {
    return mDevice->analyzeStatus(
        "set_mode",
        mDevice->device()->set_mode(mDevice->device(), static_cast<audio_mode_t>(mode)));
}

Return<void> MTKPrimaryDevice::getBtScoNrecEnabled(getBtScoNrecEnabled_cb _hidl_cb) {
    bool enabled;
    Result retval = mDevice->getParam(android::AudioParameter::keyBtNrec, &enabled);
    _hidl_cb(retval, enabled);
    return Void();
}

Return<Result> MTKPrimaryDevice::setBtScoNrecEnabled(bool enabled) {
    return mDevice->setParam(android::AudioParameter::keyBtNrec, enabled);
}

Return<void> MTKPrimaryDevice::getBtScoWidebandEnabled(getBtScoWidebandEnabled_cb _hidl_cb) {
    bool enabled;
    Result retval = mDevice->getParam(AUDIO_PARAMETER_KEY_BT_SCO_WB, &enabled);
    _hidl_cb(retval, enabled);
    return Void();
}

Return<Result> MTKPrimaryDevice::setBtScoWidebandEnabled(bool enabled) {
    return mDevice->setParam(AUDIO_PARAMETER_KEY_BT_SCO_WB, enabled);
}

static const char* convertTtyModeFromHIDL(IPrimaryDevice::TtyMode mode) {
    switch (mode) {
        case IPrimaryDevice::TtyMode::OFF:
            return AUDIO_PARAMETER_VALUE_TTY_OFF;
        case IPrimaryDevice::TtyMode::VCO:
            return AUDIO_PARAMETER_VALUE_TTY_VCO;
        case IPrimaryDevice::TtyMode::HCO:
            return AUDIO_PARAMETER_VALUE_TTY_HCO;
        case IPrimaryDevice::TtyMode::FULL:
            return AUDIO_PARAMETER_VALUE_TTY_FULL;
        default:
            return nullptr;
    }
}
static IPrimaryDevice::TtyMode convertTtyModeToHIDL(const char* halMode) {
    if (strcmp(halMode, AUDIO_PARAMETER_VALUE_TTY_OFF) == 0)
        return IPrimaryDevice::TtyMode::OFF;
    else if (strcmp(halMode, AUDIO_PARAMETER_VALUE_TTY_VCO) == 0)
        return IPrimaryDevice::TtyMode::VCO;
    else if (strcmp(halMode, AUDIO_PARAMETER_VALUE_TTY_HCO) == 0)
        return IPrimaryDevice::TtyMode::HCO;
    else if (strcmp(halMode, AUDIO_PARAMETER_VALUE_TTY_FULL) == 0)
        return IPrimaryDevice::TtyMode::FULL;
    return IPrimaryDevice::TtyMode(-1);
}

Return<void> MTKPrimaryDevice::getTtyMode(getTtyMode_cb _hidl_cb) {
    android::String8 halMode;
    Result retval = mDevice->getParam(AUDIO_PARAMETER_KEY_TTY_MODE, &halMode);
    if (retval != Result::OK) {
        _hidl_cb(retval, TtyMode::OFF);
        return Void();
    }
    TtyMode mode = convertTtyModeToHIDL(halMode);
    if (mode == TtyMode(-1)) {
        ALOGE("HAL returned invalid TTY value: %s", halMode.c_str());
        _hidl_cb(Result::INVALID_STATE, TtyMode::OFF);
        return Void();
    }
    _hidl_cb(Result::OK, mode);
    return Void();
}

Return<Result> MTKPrimaryDevice::setTtyMode(IPrimaryDevice::TtyMode mode) {
    const char* modeStr = convertTtyModeFromHIDL(mode);
    if (modeStr == nullptr) {
        ALOGW("Can not set an invalid TTY value: %d", mode);
        return Result::INVALID_ARGUMENTS;
    }
    return mDevice->setParam(AUDIO_PARAMETER_KEY_TTY_MODE, modeStr);
}

Return<void> MTKPrimaryDevice::getHacEnabled(getHacEnabled_cb _hidl_cb) {
    bool enabled;
    Result retval = mDevice->getParam(AUDIO_PARAMETER_KEY_HAC, &enabled);
    _hidl_cb(retval, enabled);
    return Void();
}

Return<Result> MTKPrimaryDevice::setHacEnabled(bool enabled) {
    return mDevice->setParam(AUDIO_PARAMETER_KEY_HAC, enabled);
}

Return<Result> MTKPrimaryDevice::setBtScoHeadsetDebugName(const hidl_string& name) {
    return mDevice->setParam(AUDIO_PARAMETER_KEY_BT_SCO_HEADSET_NAME, name.c_str());
}
Return<void> MTKPrimaryDevice::getBtHfpEnabled(getBtHfpEnabled_cb _hidl_cb) {
    bool enabled;
    Result retval = mDevice->getParam(AUDIO_PARAMETER_KEY_HFP_ENABLE, &enabled);
    _hidl_cb(retval, enabled);
    return Void();
}
Return<Result> MTKPrimaryDevice::setBtHfpEnabled(bool enabled) {
    return mDevice->setParam(AUDIO_PARAMETER_KEY_HFP_ENABLE, enabled);
}
Return<Result> MTKPrimaryDevice::setBtHfpSampleRate(uint32_t sampleRateHz) {
    return mDevice->setParam(AUDIO_PARAMETER_KEY_HFP_SET_SAMPLING_RATE, int(sampleRateHz));
}
Return<Result> MTKPrimaryDevice::setBtHfpVolume(float volume) {
    if (!isGainNormalized(volume)) {
        ALOGW("Can not set BT HFP volume (%f) outside [0,1]", volume);
        return Result::INVALID_ARGUMENTS;
    }
    // Map the normalized volume onto the range of [0, 15]
    return mDevice->setParam(AUDIO_PARAMETER_KEY_HFP_VOLUME,
                             static_cast<int>(std::round(volume * 15)));
}
Return<Result> MTKPrimaryDevice::updateRotation(IPrimaryDevice::Rotation rotation) {
    // legacy API expects the rotation in degree
    return mDevice->setParam(AUDIO_PARAMETER_KEY_ROTATION, int(rotation) * 90);
}

Return<void> MTKPrimaryDevice::debug(const hidl_handle& fd, const hidl_vec<hidl_string>& options) {
    return mDevice->debug(fd, options);
}

Return<Result> MTKPrimaryDevice::setupParametersCallback(const sp<IMTKPrimaryDeviceCallback> &callback)  {
    return mDevice->setupParametersCallback(callback);
}

Return<void> MTKPrimaryDevice::prepareForVOWReading(uint32_t frameSize, uint32_t framesCount, prepareForVOWReading_cb _hidl_cb) {
    return mDevice->prepareForVOWReading(frameSize, framesCount, _hidl_cb);
}

Return<void> MTKPrimaryDevice::getVoiceUnlockULTime(getVoiceUnlockULTime_cb _hidl_cb) {
    return mDevice->getVoiceUnlockULTime(_hidl_cb);
}

Return<Result> MTKPrimaryDevice::setVoiceUnlockSRC(uint32_t outSR, uint32_t outChannel) {
    return mDevice->setVoiceUnlockSRC(outSR, outChannel);
}

Return<Result> MTKPrimaryDevice::startVoiceUnlockDL() {
    return mDevice->startVoiceUnlockDL();
}

Return<Result> MTKPrimaryDevice::stopVoiceUnlockDL() {
    return mDevice->stopVoiceUnlockDL();
}

Return<Result> MTKPrimaryDevice::getVoiceUnlockDLInstance() {
    return mDevice->getVoiceUnlockDLInstance();
}

Return<Result> MTKPrimaryDevice::setAudioParameterChangedCallback(const sp<IAudioParameterChangedCallback>& callback)  {
    return mDevice->setAudioParameterChangedCallback(callback);
}

Return<Result> MTKPrimaryDevice::clearAudioParameterChangedCallback()  {
    return mDevice->clearAudioParameterChangedCallback();
}

}  // namespace implementation
}  // namespace V4_1
}  // namespace audio
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor
