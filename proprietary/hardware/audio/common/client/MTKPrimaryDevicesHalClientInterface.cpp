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

#define LOG_TAG "MTKPrimaryDevicesHalClient"

//#define LOG_NDEBUG 0

#include <utils/Log.h>
#include "MTKPrimaryDevicesHalClientImpl.h"
#include "AudioAssert.h"
#include <utils/Errors.h>
#include <utils/Mutex.h>

namespace android {

static Mutex mInstanceLock;
static MTKPrimaryDevicesHalClientImpl* mMTKPrimaryDevicesHalClientImpl = NULL;

MTKPrimaryDevicesHalClient *MTKPrimaryDevicesHalClient::getInstance() {

    Mutex::Autolock _l(mInstanceLock);

    if (mMTKPrimaryDevicesHalClientImpl == NULL) {
        ALOGD("CreateMTKPrimaryDevicesHalClient +Begin");
        mMTKPrimaryDevicesHalClientImpl = new MTKPrimaryDevicesHalClientImpl();
        ALOGD("CreateMTKPrimaryDevicesHalClient -End");
    }
    ASSERT(mMTKPrimaryDevicesHalClientImpl != NULL);
    return mMTKPrimaryDevicesHalClientImpl;
}

status_t MTKPrimaryDevicesHalClient::destroyInstance() {
    Mutex::Autolock _l(mInstanceLock);
    if (mMTKPrimaryDevicesHalClientImpl != NULL) {
        ALOGD("DestroyMTKPrimaryDevicesHalClient +Begin");
        delete mMTKPrimaryDevicesHalClientImpl;
        ALOGD("DestroyMTKPrimaryDevicesHalClient -End");
        mMTKPrimaryDevicesHalClientImpl = NULL;
        return NO_ERROR;
    } else {
        ALOGD("DestroyMTKPrimaryDevicesHalClient NO_INIT");
        return NO_INIT;
    }
}

} // namespace android
