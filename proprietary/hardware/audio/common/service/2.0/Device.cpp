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

#define LOG_TAG "DeviceHAL"
//#define LOG_NDEBUG 0

#include <algorithm>
#include <memory.h>
#include <string.h>

#include <android/log.h>

#include "Conversions.h"
#include "Device.h"
#include "HidlUtils.h"
#include "StreamIn.h"
#include "StreamOut.h"
#include "Util.h"

namespace android {
namespace hardware {
namespace audio {
namespace V2_0 {
namespace implementation {

using ::android::hardware::audio::common::V2_0::ThreadInfo;
using ::android::hardware::audio::common::V2_0::HidlUtils;

// For callback processing, keep the device sp list to avoid device be free by system
static Vector<sp<Device>> gCallbackDeviceList;

Device::Device(audio_hw_device_mtk_t *device)
    : mDevice(device) {
}

Device::~Device() {
    int status = audio_hw_device_close(mDevice);
    ALOGW_IF(status, "Error closing audio hw device %p: %s", mDevice,
             strerror(-status));
    mDevice = nullptr;
}

// static
Result Device::analyzeStatus(const char *funcName, int status) {
    if (status != 0) {
        ALOGW("Device %s: %s", funcName, strerror(-status));
    }
    switch (status) {
    case 0:
        return Result::OK;
    case -EINVAL:
        return Result::INVALID_ARGUMENTS;
    case -ENODATA:
        return Result::INVALID_STATE;
    case -ENODEV:
        return Result::NOT_INITIALIZED;
    case -ENOSYS:
        return Result::NOT_SUPPORTED;
    default:
        return Result::INVALID_STATE;
    }
}

void Device::closeInputStream(audio_stream_in_t *stream) {
    mDevice->close_input_stream(mDevice, stream);
}

void Device::closeOutputStream(audio_stream_out_t *stream) {
    mDevice->close_output_stream(mDevice, stream);
}

char *Device::halGetParameters(const char *keys) {
    return mDevice->get_parameters(mDevice, keys);
}

int Device::halSetParameters(const char *keysAndValues) {
    return mDevice->set_parameters(mDevice, keysAndValues);
}

// Methods from ::android::hardware::audio::V2_0::IDevice follow.
Return<Result> Device::initCheck() {
    return analyzeStatus("init_check", mDevice->init_check(mDevice));
}

Return<Result> Device::setMasterVolume(float volume) {
    if (mDevice->set_master_volume == NULL) {
        return Result::NOT_SUPPORTED;
    }
    if (!isGainNormalized(volume)) {
        ALOGW("Can not set a master volume (%f) outside [0,1]", volume);
        return Result::INVALID_ARGUMENTS;
    }
    return analyzeStatus("set_master_volume",
                         mDevice->set_master_volume(mDevice, volume));
}

Return<void> Device::getMasterVolume(getMasterVolume_cb _hidl_cb) {
    Result retval(Result::NOT_SUPPORTED);
    float volume = 0;
    if (mDevice->get_master_volume != NULL) {
        retval = analyzeStatus("get_master_volume",
                               mDevice->get_master_volume(mDevice, &volume));
    }
    _hidl_cb(retval, volume);
    return Void();
}

Return<Result> Device::setMicMute(bool mute) {
    return analyzeStatus("set_mic_mute", mDevice->set_mic_mute(mDevice, mute));
}

Return<void> Device::getMicMute(getMicMute_cb _hidl_cb) {
    bool mute = false;
    Result retval =
        analyzeStatus("get_mic_mute", mDevice->get_mic_mute(mDevice, &mute));
    _hidl_cb(retval, mute);
    return Void();
}

Return<Result> Device::setMasterMute(bool mute) {
    Result retval(Result::NOT_SUPPORTED);
    if (mDevice->set_master_mute != NULL) {
        retval = analyzeStatus("set_master_mute",
                               mDevice->set_master_mute(mDevice, mute));
    }
    return retval;
}

Return<void> Device::getMasterMute(getMasterMute_cb _hidl_cb) {
    Result retval(Result::NOT_SUPPORTED);
    bool mute = false;
    if (mDevice->get_master_mute != NULL) {
        retval = analyzeStatus("get_master_mute",
                               mDevice->get_master_mute(mDevice, &mute));
    }
    _hidl_cb(retval, mute);
    return Void();
}

Return<void> Device::getInputBufferSize(const AudioConfig &config,
                                        getInputBufferSize_cb _hidl_cb) {
    audio_config_t halConfig;
    HidlUtils::audioConfigToHal(config, &halConfig);
    size_t halBufferSize = mDevice->get_input_buffer_size(mDevice, &halConfig);
    Result retval(Result::INVALID_ARGUMENTS);
    uint64_t bufferSize = 0;
    if (halBufferSize != 0) {
        retval = Result::OK;
        bufferSize = halBufferSize;
    }
    _hidl_cb(retval, bufferSize);
    return Void();
}

Return<void> Device::openOutputStream(int32_t ioHandle,
                                      const DeviceAddress &device,
                                      const AudioConfig &config,
                                      AudioOutputFlag flags,
                                      openOutputStream_cb _hidl_cb) {
    audio_config_t halConfig;
    HidlUtils::audioConfigToHal(config, &halConfig);
    audio_stream_out_t *halStream;
    ALOGV(
        "open_output_stream handle: %d devices: %x flags: %#x "
        "srate: %d format %#x channels %x address %s",
        ioHandle, static_cast<audio_devices_t>(device.device),
        static_cast<audio_output_flags_t>(flags), halConfig.sample_rate,
        halConfig.format, halConfig.channel_mask,
        deviceAddressToHal(device).c_str());
    int status = mDevice->open_output_stream(
                     mDevice, ioHandle, static_cast<audio_devices_t>(device.device),
                     static_cast<audio_output_flags_t>(flags), &halConfig, &halStream,
                     deviceAddressToHal(device).c_str());
    ALOGV("open_output_stream status %d stream %p", status, halStream);
    sp<IStreamOut> streamOut;
    if (status == OK) {
        streamOut = new StreamOut(this, halStream);
    }
    AudioConfig suggestedConfig;
    HidlUtils::audioConfigFromHal(halConfig, &suggestedConfig);
    _hidl_cb(analyzeStatus("open_output_stream", status), streamOut,
             suggestedConfig);
    return Void();
}

Return<void> Device::openInputStream(int32_t ioHandle,
                                     const DeviceAddress &device,
                                     const AudioConfig &config,
                                     AudioInputFlag flags, AudioSource source,
                                     openInputStream_cb _hidl_cb) {
    audio_config_t halConfig;
    HidlUtils::audioConfigToHal(config, &halConfig);
    audio_stream_in_t *halStream;
    ALOGV(
        "open_input_stream handle: %d devices: %x flags: %#x "
        "srate: %d format %#x channels %x address %s source %d",
        ioHandle, static_cast<audio_devices_t>(device.device),
        static_cast<audio_input_flags_t>(flags), halConfig.sample_rate,
        halConfig.format, halConfig.channel_mask,
        deviceAddressToHal(device).c_str(),
        static_cast<audio_source_t>(source));
    int status = mDevice->open_input_stream(
                     mDevice, ioHandle, static_cast<audio_devices_t>(device.device),
                     &halConfig, &halStream, static_cast<audio_input_flags_t>(flags),
                     deviceAddressToHal(device).c_str(),
                     static_cast<audio_source_t>(source));
    ALOGV("open_input_stream status %d stream %p", status, halStream);
    sp<IStreamIn> streamIn;
    if (status == OK) {
        streamIn = new StreamIn(this, halStream);
    }
    AudioConfig suggestedConfig;
    HidlUtils::audioConfigFromHal(halConfig, &suggestedConfig);
    _hidl_cb(analyzeStatus("open_input_stream", status), streamIn,
             suggestedConfig);
    return Void();
}

Return<bool> Device::supportsAudioPatches() {
    return version() >= AUDIO_DEVICE_API_VERSION_3_0;
}

Return<void> Device::createAudioPatch(const hidl_vec<AudioPortConfig> &sources,
                                      const hidl_vec<AudioPortConfig> &sinks,
                                      createAudioPatch_cb _hidl_cb) {
    Result retval(Result::NOT_SUPPORTED);
    AudioPatchHandle patch = 0;
    if (version() >= AUDIO_DEVICE_API_VERSION_3_0) {
        std::unique_ptr<audio_port_config[]> halSources(
            HidlUtils::audioPortConfigsToHal(sources));
        std::unique_ptr<audio_port_config[]> halSinks(
            HidlUtils::audioPortConfigsToHal(sinks));
        audio_patch_handle_t halPatch = AUDIO_PATCH_HANDLE_NONE;
        retval = analyzeStatus(
                     "create_audio_patch",
                     mDevice->create_audio_patch(mDevice, sources.size(), &halSources[0],
                                                 sinks.size(), &halSinks[0], &halPatch));
        if (retval == Result::OK) {
            patch = static_cast<AudioPatchHandle>(halPatch);
        }
    }
    _hidl_cb(retval, patch);
    return Void();
}

Return<Result> Device::releaseAudioPatch(int32_t patch) {
    if (version() >= AUDIO_DEVICE_API_VERSION_3_0) {
        return analyzeStatus(
                   "release_audio_patch",
                   mDevice->release_audio_patch(
                       mDevice, static_cast<audio_patch_handle_t>(patch)));
    }
    return Result::NOT_SUPPORTED;
}

Return<void> Device::getAudioPort(const AudioPort &port,
                                  getAudioPort_cb _hidl_cb) {
    audio_port halPort;
    HidlUtils::audioPortToHal(port, &halPort);
    Result retval = analyzeStatus("get_audio_port",
                                  mDevice->get_audio_port(mDevice, &halPort));
    AudioPort resultPort = port;
    if (retval == Result::OK) {
        HidlUtils::audioPortFromHal(halPort, &resultPort);
    }
    _hidl_cb(retval, resultPort);
    return Void();
}

Return<Result> Device::setAudioPortConfig(const AudioPortConfig &config) {
    if (version() >= AUDIO_DEVICE_API_VERSION_3_0) {
        struct audio_port_config halPortConfig;
        HidlUtils::audioPortConfigToHal(config, &halPortConfig);
        return analyzeStatus(
                   "set_audio_port_config",
                   mDevice->set_audio_port_config(mDevice, &halPortConfig));
    }
    return Result::NOT_SUPPORTED;
}

Return<AudioHwSync> Device::getHwAvSync() {
    int halHwAvSync;
    Result retval = getParam(AudioParameter::keyHwAvSync, &halHwAvSync);
    return retval == Result::OK ? halHwAvSync : AUDIO_HW_SYNC_INVALID;
}

Return<Result> Device::setScreenState(bool turnedOn) {
    return setParam(AudioParameter::keyScreenState, turnedOn);
}

Return<void> Device::getParameters(const hidl_vec<hidl_string> &keys,
                                   getParameters_cb _hidl_cb) {
    getParametersImpl(keys, _hidl_cb);
    return Void();
}

Return<Result> Device::setParameters(
    const hidl_vec<ParameterValue> &parameters) {
    return setParametersImpl(parameters);
}

Return<void> Device::debugDump(const hidl_handle &fd) {
    if (fd.getNativeHandle() != nullptr && fd->numFds == 1) {
        analyzeStatus("dump", mDevice->dump(mDevice, fd->data[0]));
    }
    return Void();
}

Return<Result> Device::setupParametersCallback(const sp<IMTKPrimaryDeviceCallback> &callback) {
    if (mDevice->setup_parameters_callback == NULL) { return Result::NOT_SUPPORTED; }
    int result = mDevice->setup_parameters_callback(mDevice, Device::syncCallback, this);
    if (result == 0) {
        mCallback = callback;
    }
    return Stream::analyzeStatus("set_parameters_callback", result);
}

int Device::syncCallback(device_parameters_callback_event_t event, audio_hw_device_set_parameters_callback_t *param, void *cookie) {
    wp<Device> weakSelf(reinterpret_cast<Device *>(cookie));
    sp<Device> self = weakSelf.promote();
    if (self == nullptr || self->mCallback == nullptr) { return 0; }
    ALOGV("syncCallback() event %d", event);
    switch (event) {
    case DEVICE_CBK_EVENT_SETPARAMETERS: {
        IMTKPrimaryDeviceCallback::SetParameters *pData = new IMTKPrimaryDeviceCallback::SetParameters();
        pData->paramchar_len = param->paramchar_len;
        uint8_t *dst = reinterpret_cast<uint8_t *>(&pData->paramchar[0]);
        const uint8_t *src = reinterpret_cast<uint8_t *>(&param->paramchar[0]);
        memcpy(dst, src, param->paramchar_len);
        self->mCallback->setParametersCallback(*pData);
        delete pData;
        break;
    }
    default:
        ALOGW("syncCallback() unknown event %d", event);
        break;
    }
    return 0;
}

int Device::audioParameterChangedCallback(const char *param, void *cookie) {
    ALOGV("Device::%s(), audioType = %s, cookie = %p", __FUNCTION__, param, cookie);
    wp<Device> weakSelf(reinterpret_cast<Device*>(cookie));
    sp<Device> self = weakSelf.promote();
    hidl_string audioTypeName = param;
    Return<void> result = self->mAudioParameterChangedCallback->audioParameterChangedCallback(audioTypeName);
    return result.isOk();
}


Return<Result> Device::setAudioParameterChangedCallback(const sp<IAudioParameterChangedCallback>& callback) {
    ALOGV("Device::%s()", __FUNCTION__);
    if (mDevice->set_audio_parameter_changed_callback == NULL) return Result::NOT_SUPPORTED;
    int result = mDevice->set_audio_parameter_changed_callback(mDevice, Device::audioParameterChangedCallback, this);
    if (result != 0) {
        ALOGE("%s(), result != 0 (%d)", __FUNCTION__, result);
    }

    // Keep device sp in the list
    gCallbackDeviceList.push_back(this);

    mAudioParameterChangedCallback = callback;
    return Stream::analyzeStatus("set_audio_parameter_changed_callback", result);
}

Return<Result> Device::clearAudioParameterChangedCallback() {
    ALOGV("Device::%s()", __FUNCTION__);
    if (mDevice->clear_audio_parameter_changed_callback == NULL) return Result::NOT_SUPPORTED;
    int result = mDevice->clear_audio_parameter_changed_callback(mDevice, this);
    if (result != 0) {
        ALOGE("%s(), result != 0 (%d)", __FUNCTION__, result);
    }

    // Remove the device from sp list
    for (Vector<sp<Device>>::iterator iter = gCallbackDeviceList.begin(); iter != gCallbackDeviceList.end();) {
        if (*iter == this) {
            ALOGD("%s(), Find matched device, remove it!, (cur size = %zu)", __FUNCTION__, gCallbackDeviceList.size());
            iter = gCallbackDeviceList.erase(iter);
        } else {
            iter++;
        }
    }

    return Stream::analyzeStatus("clear_audio_parameter_changed_callback", result);
}

Return<void> Device::prepareForVOWReading(uint32_t frameSize, uint32_t framesCount,
                                          IMTKPrimaryDevice::prepareForVOWReading_cb _hidl_cb)  {
    ThreadInfo threadInfo = { 0, 0 };
    Result retval(Result::NOT_SUPPORTED);
    _hidl_cb(retval,
             CommandMQVOWRead::Descriptor(), VOWDataMQ::Descriptor(), StatusMQVOWRead::Descriptor(), threadInfo, 0, 0);
    return Void();
}

Return<void> Device::getVoiceUnlockULTime(IMTKPrimaryDevice::getVoiceUnlockULTime_cb _hidl_cb)  {
    Result retval(Result::NOT_SUPPORTED);
    _hidl_cb(retval, 0, 0);
    return Void();
}

Return<Result> Device::setVoiceUnlockSRC(uint32_t outSR, uint32_t outChannel) {
    Result retval(Result::NOT_SUPPORTED);
    return retval;
}
Return<Result> Device::startVoiceUnlockDL() {
    Result retval(Result::NOT_SUPPORTED);
    return retval;
}

Return<Result> Device::stopVoiceUnlockDL() {
    Result retval(Result::NOT_SUPPORTED);
    return retval;
}

Return<Result> Device::getVoiceUnlockDLInstance() {
    Result retval(Result::NOT_SUPPORTED);
    return retval;
}

}  // namespace implementation
}  // namespace V2_0
}  // namespace audio
}  // namespace hardware
}  // namespace android
