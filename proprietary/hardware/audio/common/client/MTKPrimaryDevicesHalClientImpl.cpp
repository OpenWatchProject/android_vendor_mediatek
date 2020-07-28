/*
* Copyright (C) 2017 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
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

#include <string.h>

#define LOG_TAG "MTKPrimaryDevicesHalClientImpl"

#include <utils/Log.h>
#include "MTKPrimaryDevicesHalClientImpl.h"
#include "AudioAssert.h"
#include <system/audio.h>

namespace android {

MTKPrimaryDevicesHalClientImpl::MTKPrimaryDevicesHalClientImpl() {

    mDevicesFactoryHal = MTKDevicesFactoryHalInterface::create();
    sp<MTKDeviceHalInterface> dev;
    int rc = mDevicesFactoryHal->openDevice(AUDIO_HARDWARE_MODULE_ID_PRIMARY, &dev);
    if (rc) {
        ALOGE("MTKPrimaryDevicesHalClientImpl() error %d loading module %s", rc, AUDIO_HARDWARE_MODULE_ID_PRIMARY);
        mPrimaryDev = NULL;
    } else {
        ALOGD("mPrimaryDev is created first");
        mPrimaryDev = dev;
    }
}

MTKPrimaryDevicesHalClientImpl::~MTKPrimaryDevicesHalClientImpl() {
    ALOGD("~MTKPrimaryDevicesHalClientImpl");
}

status_t MTKPrimaryDevicesHalClientImpl::setParameters(const String8 &keyValuePairs) {

    if (mPrimaryDev == NULL) {
        ALOGE("%s NO_INIT", __FUNCTION__);
        return NO_INIT;
    }
    ALOGD("setParameters() %s", keyValuePairs.string());
    Mutex::Autolock _l(mLock);
    return mPrimaryDev->setParameters(keyValuePairs);
}

String8 MTKPrimaryDevicesHalClientImpl::getParameters(const String8 &keys) {
    String8 out_s8;
    status_t result;
    if (mPrimaryDev == NULL) {
        ALOGE("%s NO_INIT", __FUNCTION__);
        return String8();
    }
    ALOGD("getParameters() Send %s", keys.string());
    Mutex::Autolock _l(mLock);
    result = mPrimaryDev->getParameters(keys, &out_s8);
    if (result == OK) {
        ALOGD("getParameters() Receive %s", out_s8.string());
        return out_s8;
    } else {
        ALOGE("%s Error result %d", __FUNCTION__, result);
        return String8();
    }
}

} // namespace android
