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

namespace vendor {
namespace mediatek {
namespace hardware {
namespace audio {
namespace V2_1 {
namespace implementation {


MTKPrimaryDevice::MTKPrimaryDevice(audio_hw_device_mtk_t *device)
    : mDevice(new android::hardware::audio::V2_0::implementation::Device(device)) {
}

MTKPrimaryDevice::~MTKPrimaryDevice() {}

// Methods from ::android::hardware::audio::V2_0::IDevice follow.
Return<Result> MTKPrimaryDevice::initCheck()  {
    return mDevice->initCheck();
}

Return<Result> MTKPrimaryDevice::setMasterVolume(float volume)  {
    return mDevice->setMasterVolume(volume);
}

Return<void> MTKPrimaryDevice::getMasterVolume(getMasterVolume_cb _hidl_cb)  {
    return mDevice->getMasterVolume(_hidl_cb);
}

Return<Result> MTKPrimaryDevice::setMicMute(bool mute)  {
    return mDevice->setMicMute(mute);
}

Return<void> MTKPrimaryDevice::getMicMute(getMicMute_cb _hidl_cb)  {
    return mDevice->getMicMute(_hidl_cb);
}

Return<Result> MTKPrimaryDevice::setMasterMute(bool mute)  {
    return mDevice->setMasterMute(mute);
}

Return<void> MTKPrimaryDevice::getMasterMute(getMasterMute_cb _hidl_cb)  {
    return mDevice->getMasterMute(_hidl_cb);
}

Return<void> MTKPrimaryDevice::getInputBufferSize(
    const AudioConfig &config, getInputBufferSize_cb _hidl_cb)  {
    return mDevice->getInputBufferSize(config, _hidl_cb);
}

Return<void> MTKPrimaryDevice::openOutputStream(
    int32_t ioHandle,
    const DeviceAddress &device,
    const AudioConfig &config,
    AudioOutputFlag flags,
    openOutputStream_cb _hidl_cb)  {
    return mDevice->openOutputStream(ioHandle, device, config, flags, _hidl_cb);
}

Return<void> MTKPrimaryDevice::openInputStream(
    int32_t ioHandle,
    const DeviceAddress &device,
    const AudioConfig &config,
    AudioInputFlag flags,
    AudioSource source,
    openInputStream_cb _hidl_cb)  {
    return mDevice->openInputStream(ioHandle, device, config, flags, source, _hidl_cb);
}

Return<bool> MTKPrimaryDevice::supportsAudioPatches() {
    return mDevice->supportsAudioPatches();
}

Return<void> MTKPrimaryDevice::createAudioPatch(
    const hidl_vec<AudioPortConfig> &sources,
    const hidl_vec<AudioPortConfig> &sinks,
    createAudioPatch_cb _hidl_cb)  {
    return mDevice->createAudioPatch(sources, sinks, _hidl_cb);
}

Return<Result> MTKPrimaryDevice::releaseAudioPatch(int32_t patch)  {
    return mDevice->releaseAudioPatch(patch);
}

Return<void> MTKPrimaryDevice::getAudioPort(const AudioPort &port, getAudioPort_cb _hidl_cb)  {
    return mDevice->getAudioPort(port, _hidl_cb);
}

Return<Result> MTKPrimaryDevice::setAudioPortConfig(const AudioPortConfig &config)  {
    return mDevice->setAudioPortConfig(config);
}

Return<AudioHwSync> MTKPrimaryDevice::getHwAvSync()  {
    return mDevice->getHwAvSync();
}

Return<Result> MTKPrimaryDevice::setScreenState(bool turnedOn)  {
    return mDevice->setScreenState(turnedOn);
}

Return<void> MTKPrimaryDevice::getParameters(
    const hidl_vec<hidl_string> &keys, getParameters_cb _hidl_cb)  {
    return mDevice->getParameters(keys, _hidl_cb);
}

Return<Result> MTKPrimaryDevice::setParameters(const hidl_vec<ParameterValue> &parameters)  {
    return mDevice->setParameters(parameters);
}

Return<void> MTKPrimaryDevice::debugDump(const android::hardware::hidl_handle &fd)  {
    return mDevice->debugDump(fd);
}


// Methods from ::android::hardware::audio::V2_0::IPrimaryDevice follow.
Return<Result> MTKPrimaryDevice::setVoiceVolume(float volume)  {
    return mDevice->analyzeStatus(
               "set_voice_volume",
               mDevice->device()->set_voice_volume(mDevice->device(), volume));
}

Return<Result> MTKPrimaryDevice::setMode(AudioMode mode)  {
    return mDevice->analyzeStatus(
               "set_mode",
               mDevice->device()->set_mode(mDevice->device(), static_cast<audio_mode_t>(mode)));
}

Return<void> MTKPrimaryDevice::getBtScoNrecEnabled(getBtScoNrecEnabled_cb _hidl_cb)  {
    bool enabled;
    Result retval = mDevice->getParam(android::AudioParameter::keyBtNrec, &enabled);
    _hidl_cb(retval, enabled);
    return Void();
}

Return<Result> MTKPrimaryDevice::setBtScoNrecEnabled(bool enabled)  {
    return mDevice->setParam(android::AudioParameter::keyBtNrec, enabled);
}

Return<void> MTKPrimaryDevice::getBtScoWidebandEnabled(getBtScoWidebandEnabled_cb _hidl_cb)  {
    bool enabled;
    Result retval = mDevice->getParam(AUDIO_PARAMETER_KEY_BT_SCO_WB, &enabled);
    _hidl_cb(retval, enabled);
    return Void();
}

Return<Result> MTKPrimaryDevice::setBtScoWidebandEnabled(bool enabled)  {
    return mDevice->setParam(AUDIO_PARAMETER_KEY_BT_SCO_WB, enabled);
}

Return<void> MTKPrimaryDevice::getTtyMode(getTtyMode_cb _hidl_cb)  {
    int halMode;
    Result retval = mDevice->getParam(AUDIO_PARAMETER_KEY_TTY_MODE, &halMode);
    TtyMode mode = retval == Result::OK ? TtyMode(halMode) : TtyMode::OFF;
    _hidl_cb(retval, mode);
    return Void();
}

Return<Result> MTKPrimaryDevice::setTtyMode(IPrimaryDevice::TtyMode mode)  {
    return mDevice->setParam(AUDIO_PARAMETER_KEY_TTY_MODE, static_cast<int>(mode));
}

Return<void> MTKPrimaryDevice::getHacEnabled(getHacEnabled_cb _hidl_cb)  {
    bool enabled;
    Result retval = mDevice->getParam(AUDIO_PARAMETER_KEY_HAC, &enabled);
    _hidl_cb(retval, enabled);
    return Void();
}

Return<Result> MTKPrimaryDevice::setHacEnabled(bool enabled)  {
    return mDevice->setParam(AUDIO_PARAMETER_KEY_HAC, enabled);
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
}  // namespace V2_1
}  // namespace audio
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor
