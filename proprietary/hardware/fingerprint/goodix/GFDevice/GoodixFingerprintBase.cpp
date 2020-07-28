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

#define LOG_TAG "GoodixFingerprintBase"

namespace android {

    GFNotify GoodixFingerprintBase::notify = NULL;
    GoodixFingerprintBase::GoodixFingerprintBase(){
    }

    GoodixFingerprintBase::~GoodixFingerprintBase(){
    }

    int64_t GoodixFingerprintBase::preEnroll() {
        ALOGD("goodix preEnroll");
        return gf_hal_pre_enroll(NULL);
    }

    int32_t GoodixFingerprintBase::enroll(const void* hat, int32_t gid, int32_t timeout_sec) {
        ALOGD("goodix enroll");
        return gf_hal_enroll(NULL, hat, gid, timeout_sec);
    }

    int32_t GoodixFingerprintBase::postEnroll() {
        ALOGD("goodix postEnroll");
        return gf_hal_post_enroll(NULL);
    }

    uint64_t GoodixFingerprintBase::getAuthenticatorId() {
        ALOGD("goodix getAuthenticatorId");
        return gf_hal_get_auth_id(NULL);
    }

    int32_t GoodixFingerprintBase::cancel() {
        ALOGD("goodix cancel");
        return gf_hal_cancel(NULL);
    }

    int32_t GoodixFingerprintBase::remove(int32_t gid, int32_t fid) {
        ALOGD("goodix remove");
        return gf_hal_remove(NULL, gid, fid);
    }
    int32_t GoodixFingerprintBase::setActiveGroup(uint32_t group_id) {
        ALOGD("goodix setActiveGroup");
        gf_hal_set_active_group(NULL, group_id);
        return 0;
    }

    int32_t GoodixFingerprintBase::authenticate(uint64_t operation_id, uint32_t gid) {
        ALOGD("goodix authenticate");
        return gf_hal_authenticate(NULL, operation_id, gid);
    }

    int32_t GoodixFingerprintBase::enumerate() {
        ALOGD("goodix enumerate");
        return gf_hal_enumerate_with_callback(NULL);
    }

    int32_t GoodixFingerprintBase::setNotifyCallback(void *baseNotify) {
        ALOGD("goodix setNotifyCallback");
        GoodixFingerprintBase::notify = (GFNotify) baseNotify;
        return 0;
    }

} // namespace android
