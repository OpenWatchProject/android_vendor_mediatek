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

#define LOG_TAG "mtkaudiohalservice"
#define LOG_NDEBUG 0

#include <android/hardware/audio/5.0/IDevicesFactory.h>
#include <android/hardware/audio/effect/5.0/IEffectsFactory.h>
#include <android/hardware/bluetooth/a2dp/1.0/IBluetoothAudioOffload.h>
#include <android/hardware/bluetooth/audio/2.0/IBluetoothAudioProvidersFactory.h>
#include <android/hardware/soundtrigger/2.2/ISoundTriggerHw.h>
#include <binder/ProcessState.h>
#include <cutils/properties.h>
#include <hidl/HidlTransportSupport.h>
#include <hidl/LegacySupport.h>
#include <hwbinder/ProcessState.h>

using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using android::hardware::registerPassthroughServiceImplementation;

using android::hardware::audio::effect::V5_0::IEffectsFactory;
using android::hardware::audio::V5_0::IDevicesFactory;
using android::hardware::soundtrigger::V2_2::ISoundTriggerHw;
using android::hardware::registerPassthroughServiceImplementation;

using namespace android::hardware;
using android::OK;

#define SUPPORT_AUDIO_DEVICES_FACTORY
#define SUPPORT_AUDIO_EFFECTS_FACTORY
#define SUPPORT_SOUNDTRIGGER_HW

#ifdef FORCE_DIRECTCOREDUMP
#include <sys/prctl.h>
#define SIGNUM 7
void directcoredump_init() {
    int sigtype[SIGNUM] = {SIGABRT, SIGBUS, SIGFPE, SIGILL, SIGSEGV, SIGTRAP, SIGSYS};
    char value[PROPERTY_VALUE_MAX] = {'\0'};

    // eng&userdebug load direct-coredump default enable
    // user load direct-coredump default disable due to libdirect-coredump.so will not be preloaded
    property_get("persist.vendor.aee.core.direct", value, "default");
    if (strncmp(value, "disable", sizeof("disable"))) {
        int loop;
        for (loop = 0; loop < SIGNUM; loop++) {
            signal(sigtype[loop], SIG_DFL);
        }
    }
}
#endif

int main(int /* argc */, char * /* argv */ []) {
    ALOGD("Start audiohalservice");

    signal(SIGPIPE, SIG_IGN);
    signal(SIGTERM, SIG_IGN);

#ifdef FORCE_DIRECTCOREDUMP
    directcoredump_init();
    if (prctl(PR_SET_DUMPABLE, 1) < 0) {
        ALOGD("audiohalservice set dumpable fail");
    }
#endif
    ::android::ProcessState::initWithDriver("/dev/vndbinder");
    // start a threadpool for vndbinder interactions
    ::android::ProcessState::self()->startThreadPool();

    const int32_t defaultValue = -1;
    int32_t value =
        property_get_int32("persist.vendor.audio.service.hwbinder.size_kbyte", defaultValue);
    if (value != defaultValue) {
        ALOGD("Configuring hwbinder with mmap size %d KBytes", value);
        ProcessState::initWithMmapSize(static_cast<size_t>(value) * 1024);
    }
    configureRpcThreadpool(16, true /*callerWillJoin*/);

    ALOGD("registering IDevicesFactory");
    bool fail = registerPassthroughServiceImplementation<audio::V5_0::IDevicesFactory>() != OK;
    ALOGD("registered IDevicesFactory");
    LOG_ALWAYS_FATAL_IF(fail, "Could not register audio core API 5.0");

    ALOGD("registering IEffectsFactory");
    fail = registerPassthroughServiceImplementation<audio::effect::V5_0::IEffectsFactory>() != OK;
    ALOGD("registered IEffectsFactory");
    LOG_ALWAYS_FATAL_IF(fail, "Could not register audio effect API 5.0");

    ALOGD("registering ISoundTriggerHw sound_trigger.primary");
    fail = registerPassthroughServiceImplementation<soundtrigger::V2_2::ISoundTriggerHw>() != OK;
    ALOGD("registered ISoundTriggerHwsound_trigger.primary");
    ALOGW_IF(fail, "Could not register soundtrigger API 2.2");

    ALOGD("registering Bluetooth Audio API 2.0");
    fail = registerPassthroughServiceImplementation<
                   bluetooth::audio::V2_0::IBluetoothAudioProvidersFactory>() != OK;
    ALOGD("registered Bluetooth Audio API 2.0");
    ALOGW_IF(fail, "Could not register Bluetooth Audio API 2.0");

    ALOGD("registering IBluetoothAudioOffload");
    // remove the old HIDL when Bluetooth Audio Hal V2 has offloading supported
    fail =
        registerPassthroughServiceImplementation<bluetooth::a2dp::V1_0::IBluetoothAudioOffload>() !=
        OK;
    ALOGD("registered IBluetoothAudioOffload");
    ALOGW_IF(fail, "Could not register Bluetooth audio offload 1.0");

    joinRpcThreadpool();
}
