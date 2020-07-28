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

#include <utils/Log.h>
#include <hidl/HidlTransportSupport.h>
#include <hidl/LegacySupport.h>
#include <android/hardware/audio/2.0/IDevicesFactory.h>
#include <android/hardware/audio/effect/2.0/IEffectsFactory.h>
#include <android/hardware/soundtrigger/2.1/ISoundTriggerHw.h>
#include <cutils/properties.h>

using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using android::hardware::registerPassthroughServiceImplementation;

using android::hardware::audio::effect::V2_0::IEffectsFactory;
using android::hardware::audio::V2_0::IDevicesFactory;
using android::hardware::soundtrigger::V2_1::ISoundTriggerHw;
using android::hardware::registerPassthroughServiceImplementation;

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

#ifdef FORCE_DIRECTCOREDUMP
    directcoredump_init();
    if (prctl(PR_SET_DUMPABLE, 1) < 0) {
        ALOGD("audiohalservice set dumpable fail");
    }
#endif
    configureRpcThreadpool(16, true /*callerWillJoin*/);
    android::status_t status;
    ALOGD("registering IDevicesFactory");
    status = registerPassthroughServiceImplementation<IDevicesFactory>();
    ALOGD("registered IDevicesFactory");
    LOG_ALWAYS_FATAL_IF(status != OK, "Error while registering audio service: %d", status);
    ALOGD("registering IEffectsFactory");
    status = registerPassthroughServiceImplementation<IEffectsFactory>();
    ALOGD("registered IEffectsFactory");
    LOG_ALWAYS_FATAL_IF(status != OK, "Error while registering audio effects service: %d", status);
    // Soundtrigger might be not present.
    ALOGD("registering ISoundTriggerHw sound_trigger.primary");
    status = registerPassthroughServiceImplementation<ISoundTriggerHw>();
    ALOGD("registered ISoundTriggerHwsound_trigger.primary");
    ALOGE_IF(status != OK, "Error while registering soundtrigger service: %d", status);
    joinRpcThreadpool();
    return status;
}
