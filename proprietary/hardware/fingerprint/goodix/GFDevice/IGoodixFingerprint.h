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

#ifndef IGOODIX_FINGERPRINT_H_
#define IGOODIX_FINGERPRINT_H_

#include <errno.h>
#include <string.h>
#include <iostream>
#include <cutils/log.h>
#include <utils/RefBase.h>
#include <memory>
#include "gf_hal.h"
#include "gf_hal_common.h"

namespace android {

typedef void (*GFNotify)(const gf_fingerprint_msg_t *msg);
/*
* Abstract base class for native implementation of FingerprintService.
*
* Note: This must be kept manually in sync with IGoodixFingerprintDaemon.aidl
*/
class IGoodixFingerprintBase {
    public:
        virtual int64_t preEnroll() = 0;
        virtual int32_t enroll(const void* hat, int32_t gid, int32_t timeout_sec) = 0;
        virtual int32_t postEnroll() = 0;
        virtual uint64_t getAuthenticatorId() = 0;
        virtual int32_t cancel() = 0;
        virtual int32_t remove(int32_t gid, int32_t fid) = 0;
        virtual int32_t setActiveGroup(uint32_t group_id) = 0;
        virtual int32_t authenticate(uint64_t operation_id, uint32_t gid) = 0;
        virtual int32_t enumerate() = 0;
        virtual int32_t setNotifyCallback(void *baseNotify) = 0;

    public:
        virtual ~IGoodixFingerprintBase(){};
};

class IGoodixFingerprintExt {
    public:
        virtual int32_t setSafeClass(uint32_t safeClass) = 0;
        virtual int32_t navigate(uint32_t navMode) = 0;
        virtual int32_t stopNavigation() = 0;
        virtual int32_t enableFingerprintModule(uint8_t enableFlag) = 0;
        virtual int32_t cameraCapture() = 0;
        virtual int32_t stopCameraCapture() = 0;
        virtual int32_t enableFfFeature(uint8_t enableFlag) = 0;
        virtual int32_t screenOn() = 0;
        virtual int32_t screenOff() = 0;
        virtual int32_t resetLockout(const uint8_t *token, ssize_t tokenLength) = 0;
        virtual int32_t setNotifyCallback(void *extNotify) = 0;

    public:
        virtual ~IGoodixFingerprintExt() {};
};

class IGoodixFingerprintTest {
    public:
        virtual int32_t testCmd(uint32_t cmdId, const uint8_t* param, uint32_t paramLen) = 0;
        virtual int32_t setNotifyCallback(void *testNotify) = 0;
    public:
        virtual ~IGoodixFingerprintTest() {};
};


class GoodixFingerprintBase : public IGoodixFingerprintBase{

    public:
        int64_t preEnroll() override;
        int32_t enroll(const void* hat, int32_t gid, int32_t timeout_sec) override;
        int32_t postEnroll() override;
        uint64_t getAuthenticatorId() override;
        int32_t cancel() override;
        int32_t remove(int32_t gid, int32_t fid) override;
        int32_t setActiveGroup(uint32_t group_id) override;
        int32_t authenticate(uint64_t operation_id, uint32_t gid) override;
        int32_t enumerate() override;
        int32_t setNotifyCallback(void *baseNotify) override;
    public:
        GoodixFingerprintBase();
       ~GoodixFingerprintBase();
    public:
        static GFNotify notify;
};


class GoodixFingerprintExt : public IGoodixFingerprintExt{

    public:
        int32_t setSafeClass(uint32_t safeClass) override;
        int32_t navigate(uint32_t navMode) override;
        int32_t stopNavigation() override;
        int32_t enableFingerprintModule(uint8_t enableFlag) override;
        int32_t cameraCapture() override;
        int32_t stopCameraCapture() override;
        int32_t enableFfFeature(uint8_t enableFlag) override;
        int32_t screenOn() override;
        int32_t screenOff() override;
        int32_t resetLockout(const uint8_t *token, ssize_t tokenLength) override;
        int32_t setNotifyCallback(void *extNotify) override;
    public:
        GoodixFingerprintExt();
       ~GoodixFingerprintExt();
    public:
        static GFNotify notify;;
};

class GoodixFingerprintTest : public IGoodixFingerprintTest{

    public:
        int32_t testCmd(uint32_t cmdId, const uint8_t* param, uint32_t paramLen) override;
        int32_t setNotifyCallback(void *testNotify) override;
    public:
        GoodixFingerprintTest();
       ~GoodixFingerprintTest();
    public:
        static GFNotify notify;;
};

} // namespace android

#endif // IGOODIX_FINGERPRINT_H_

