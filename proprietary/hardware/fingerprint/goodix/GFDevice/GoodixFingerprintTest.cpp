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
#include "gf_hal_test.h"
#define LOG_TAG "GoodixFingerprintTest"

namespace android {

    GFNotify GoodixFingerprintTest::notify = NULL;
    GoodixFingerprintTest::GoodixFingerprintTest(){
    }

    GoodixFingerprintTest::~GoodixFingerprintTest(){
    }

    int32_t GoodixFingerprintTest::testCmd(uint32_t cmdId, const uint8_t* param, uint32_t paramLen) {
        ALOGD("goodix testCmd");
        return gf_hal_test_cmd(NULL, cmdId, param, paramLen);
    }

    int32_t GoodixFingerprintTest::setNotifyCallback(void *testNotify) {
        ALOGD("goodix setNotifyCallback");
        GoodixFingerprintTest::notify = (GFNotify) testNotify;
        return 0;
    }
} // namespace android
