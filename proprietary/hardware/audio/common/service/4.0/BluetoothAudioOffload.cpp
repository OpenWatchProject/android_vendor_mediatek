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

#define LOG_TAG "BluetoothAudioOffload_AudioHal"

#include "BluetoothAudioOffload.h"
//#include "AudioALSAHardware.h"
#include <hardware_legacy/AudioMTKHardwareInterface.h>

#include <utils/Log.h>
#include <android/log.h>
#include <dlfcn.h>
#include <cutils/properties.h>

#if defined(__LP64__)
#define HAL_LIBRARY_PATH1 "/system/lib64/hw"
#define HAL_LIBRARY_PATH2 "/vendor/lib64/hw"
#else
#define HAL_LIBRARY_PATH1 "/system/lib/hw"
#define HAL_LIBRARY_PATH2 "/vendor/lib/hw"
#endif
#define AUDIO_HAL_PREFIX "audio.primary"
#define PLATFORM_ID "ro.hardware"
#define BOARD_PLATFORM_ID "ro.board.platform"

namespace android {
namespace hardware {
namespace bluetooth {
namespace a2dp {
namespace V1_0 {
namespace implementation {

IBluetoothAudioOffload* HIDL_FETCH_IBluetoothAudioOffload(const char* /* name */) {
    return new BluetoothAudioOffload();
}

static    AudioMTKHardwareInterface *gAudioHardware = NULL;
static void *AudioHwhndl_factory = NULL;

// Methods from ::android::hardware::bluetooth::a2dp::V1_0::IBluetoothAudioOffload follow.
Return<::android::hardware::bluetooth::a2dp::V1_0::Status> BluetoothAudioOffload::startSession(
    const sp<::android::hardware::bluetooth::a2dp::V1_0::IBluetoothAudioHost>& hostIf __unused,
    const ::android::hardware::bluetooth::a2dp::V1_0::CodecConfiguration& codecConfig __unused) {
    /**
     * Initialize the audio platform if codecConfiguration is supported.
     * Save the the IBluetoothAudioHost interface, so that it can be used
     * later to send stream control commands to the HAL client, based on
     * interaction with Audio framework.
     */
    char prop[PATH_MAX];
    char path[PATH_MAX];

    ALOGD("+%s()", __FUNCTION__);

    if (gAudioHardware == NULL || AudioHwhndl_factory==NULL)
    {
        do
        {
            if (property_get(PLATFORM_ID, prop, NULL) == 0)
            {
                snprintf(path, sizeof(path), "%s/%s.default.so",
                         HAL_LIBRARY_PATH1, prop);
                if (access(path, R_OK) == 0) break;

                snprintf(path, sizeof(path), "%s/%s.default.so",
                     HAL_LIBRARY_PATH2, prop);
                if (access(path, R_OK) == 0) break;
            }
            else
            {
                snprintf(path, sizeof(path), "%s/%s.%s.so",
                         HAL_LIBRARY_PATH1, AUDIO_HAL_PREFIX, prop);
                if (access(path, R_OK) == 0) break;

                snprintf(path, sizeof(path), "%s/%s.%s.so",
                     HAL_LIBRARY_PATH2, AUDIO_HAL_PREFIX, prop);
                if (access(path, R_OK) == 0) break;

                if (property_get(BOARD_PLATFORM_ID, prop, NULL) == 0)
                {
                    snprintf(path, sizeof(path), "%s/%s.default.so",
                             HAL_LIBRARY_PATH1, prop);
                    if (access(path, R_OK) == 0) break;

                    snprintf(path, sizeof(path), "%s/%s.default.so",
                         HAL_LIBRARY_PATH2, prop);
                    if (access(path, R_OK) == 0) break;
                }
                else
                {
                    snprintf(path, sizeof(path), "%s/%s.%s.so",
                         HAL_LIBRARY_PATH1, AUDIO_HAL_PREFIX, prop);
                     if (access(path, R_OK) == 0) break;

                     snprintf(path, sizeof(path), "%s/%s.%s.so",
                          HAL_LIBRARY_PATH2, AUDIO_HAL_PREFIX, prop);
                     if (access(path, R_OK) == 0) break;
                }
            }
        }while(0);

        ALOGD ("%s Load %s",__FUNCTION__, path);
        AudioHwhndl_factory = dlopen(path, RTLD_NOW);
    }

    if(AudioHwhndl_factory == NULL){
        ALOGE("%s", dlerror());
        ALOGD("-startSession open AudioHwhndl path [%s] fail", path);
        return ::android::hardware::bluetooth::a2dp::V1_0::Status::FAILURE;
    }
    else
    {
        create_AudioMTKHw* func1 = (create_AudioMTKHw*)dlsym(AudioHwhndl_factory, "createMTKAudioHardware");
        ALOGD("%s %d func1 %p", __FUNCTION__, __LINE__, func1);
        const char* dlsym_error1 = dlerror();
        if (func1 == NULL)
        {
            ALOGD("-startSession dlsym createMTKAudioHardware fail");
            dlclose(AudioHwhndl_factory);
            AudioHwhndl_factory = NULL;
            return ::android::hardware::bluetooth::a2dp::V1_0::Status::FAILURE;
        }
        gAudioHardware = func1();
        ALOGD("%s %d gAudioHardware %p",__FUNCTION__,__LINE__,gAudioHardware);
    }

    if (gAudioHardware != NULL) {
        gAudioHardware->setBluetoothAudioOffloadParam(hostIf, codecConfig, true);
        ALOGD("-%s()", __FUNCTION__);
        return ::android::hardware::bluetooth::a2dp::V1_0::Status::SUCCESS;
    } else {
        ALOGE("-%s() fail! gAudioHardware=NULL", __FUNCTION__);
        return ::android::hardware::bluetooth::a2dp::V1_0::Status::FAILURE;
    }
}

Return<void> BluetoothAudioOffload::streamStarted(
    ::android::hardware::bluetooth::a2dp::V1_0::Status status __unused) {
    /**
     * Streaming on control path has started,
     * HAL server should start the streaming on data path.
     */
    ALOGD("+%s(), status=%d", __FUNCTION__, (int)status);
    // 0: Start SUCCESS, 1: Start FAILURE, 3: Start PENDING
    if (status == ::android::hardware::bluetooth::a2dp::V1_0::Status::SUCCESS) {
        gAudioHardware->setA2dpSuspendStatus(0);
    } else if (status == ::android::hardware::bluetooth::a2dp::V1_0::Status::FAILURE) {
        gAudioHardware->setA2dpSuspendStatus(1);
    } else if (status == ::android::hardware::bluetooth::a2dp::V1_0::Status::PENDING) {
        gAudioHardware->setA2dpSuspendStatus(3);
    }

    ALOGD("-%s()", __FUNCTION__);
    return Void();
}

Return<void> BluetoothAudioOffload::streamSuspended(
    ::android::hardware::bluetooth::a2dp::V1_0::Status status __unused) {
    /**
     * Streaming on control path has suspend,
     * HAL server should suspend the streaming on data path.
     */
    ALOGD("+%s(), status=%d", __FUNCTION__, (int)status);
    // 4: Suspend SUCCESS, 5: Suspend FAILURE, 7: Suspend PENDING
    if (status == ::android::hardware::bluetooth::a2dp::V1_0::Status::SUCCESS) {
        gAudioHardware->setA2dpSuspendStatus(4);
    } else if (status == ::android::hardware::bluetooth::a2dp::V1_0::Status::FAILURE) {
        gAudioHardware->setA2dpSuspendStatus(5);
    } else if (status == ::android::hardware::bluetooth::a2dp::V1_0::Status::PENDING) {
        gAudioHardware->setA2dpSuspendStatus(7);
    }

    ALOGD("-%s()", __FUNCTION__);
    return Void();
}

Return<void> BluetoothAudioOffload::endSession() {
    /**
     * Cleanup the audio platform as remote A2DP Sink device is no
     * longer active
     */
    ALOGD("+%s()", __FUNCTION__);
    ::android::hardware::bluetooth::a2dp::V1_0::CodecConfiguration codecinfo;

    if (gAudioHardware) {
        ALOGD("%s %d gAudioHardware %p",__FUNCTION__,__LINE__,gAudioHardware);
        gAudioHardware->setBluetoothAudioOffloadParam(NULL, codecinfo, false);
    }

    ALOGD("-%s()", __FUNCTION__);
    return Void();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace a2dp
}  // namespace bluetooth
}  // namespace hardware
}  // namespace android
