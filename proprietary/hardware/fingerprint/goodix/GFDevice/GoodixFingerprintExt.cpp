/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include "IGoodixFingerprint.h"

#define LOG_TAG "GoodixFingerprintExt"

namespace android {

GFNotify GoodixFingerprintExt::notify = NULL;
GoodixFingerprintExt::GoodixFingerprintExt(){
}

GoodixFingerprintExt::~GoodixFingerprintExt(){
}

int32_t GoodixFingerprintExt::setSafeClass(uint32_t safeClass) {
    return gf_hal_set_safe_class(NULL, (gf_safe_class_t)safeClass);
}

int32_t GoodixFingerprintExt::navigate(uint32_t navMode) {
    return gf_hal_navigate(NULL, (gf_nav_mode_t)navMode);
}

int32_t GoodixFingerprintExt::stopNavigation() {
    return gf_hal_cancel(NULL);
}

int32_t GoodixFingerprintExt::enableFingerprintModule(uint8_t enableFlag) {
    return gf_hal_enable_fingerprint_module(NULL, enableFlag);
}

int32_t GoodixFingerprintExt::cameraCapture() {
    return gf_hal_camera_capture(NULL);
}

int32_t GoodixFingerprintExt::stopCameraCapture() {
    return gf_hal_cancel(NULL);
}

int32_t GoodixFingerprintExt::enableFfFeature(uint8_t enableFlag) {
    return gf_hal_enable_ff_feature(NULL, enableFlag);
}

int32_t GoodixFingerprintExt::screenOn() {
    return gf_hal_screen_on();
}

int32_t GoodixFingerprintExt::screenOff() {
    return gf_hal_screen_off();
}


int32_t GoodixFingerprintExt::resetLockout(const uint8_t *token, ssize_t tokenLength) {
    ALOG(LOG_VERBOSE, LOG_TAG, "reset_lockout()\n");
    if (tokenLength != sizeof(gf_hw_auth_token_t) ) {
        ALOG(LOG_VERBOSE, LOG_TAG, "enroll() : invalid token size %zu\n", tokenLength);
        return -1;
    }
    const gf_hw_auth_token_t* authToken = reinterpret_cast<const gf_hw_auth_token_t*>(token);
    return gf_hal_reset_lockout(NULL, authToken);
}

int32_t GoodixFingerprintExt::setNotifyCallback(void *extNotify) {
    ALOGD("goodix setNotifyCallback");
    GoodixFingerprintExt::notify = (GFNotify) extNotify;
    return 0;
}

} // namespace android
