/*
 * Copyright 2016 The Android Open Source Project
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

#define LOG_TAG "RCS_HAL"

#include <vendor/mediatek/hardware/rcs/2.0/IRcs.h>
#include <vendor/mediatek/hardware/rcs/2.0/IRcsIndication.h>

#include <android-base/logging.h>
#include <hidl/HidlTransportSupport.h>
#include <utils/Looper.h>
#include "HidlService.h"
#include "rcs_proxy_adapter.h"


using vendor::mediatek::hardware::rcs::V2_0::IRcs;
using vendor::mediatek::hardware::rcs::V2_0::IRcsIndication;
using android::hardware::Return;
using android::hardware::hidl_death_recipient;
using android::hardware::hidl_vec;
using android::hardware::Void;
using android::hidl::base::V1_0::IBase;

using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;

struct IRcsImpl : public IRcs {
    android::sp<IRcsIndication> mIRcsIndication;
    Return<void> setResponseFunctions(const android::sp<IRcsIndication>& rcsIndication);
    Return<void> writeEvent(int32_t account, int32_t request, const hidl_vec<int8_t>& value);
};

static android::sp<IRcsImpl> g_service;

struct RcsDeathRecipient : virtual public hidl_death_recipient {
    virtual void serviceDied(uint64_t cookie __unused, const android::wp<IBase> &who __unused) override {
        ALOGW("%s(), RCS hal died, set RCS indication to null", __FUNCTION__);
        if (g_service != NULL) {
            g_service->mIRcsIndication = NULL;
        }
    }
};

static android::sp<RcsDeathRecipient> g_rcsHalDeathRecipient = NULL;

int startHidlService() {
    android::status_t status;

    ALOGD("Rcs Hal is starting up...");
    configureRpcThreadpool(1, true /* callerWillJoin */);
    //Setup hwbinder service
    g_service = new IRcsImpl();
    status = g_service->registerAsService("rcs_hal_service");

    g_rcsHalDeathRecipient = new RcsDeathRecipient();

    if (status != android::OK) {
        ALOGE("Error while registering rcs hidl service: %d", status);
    } else {
        ALOGD("Rcs Hal is started");
    }

    joinRpcThreadpool();
    ALOGD("Rcs hal is terminating...");

    return status;
}

hidl_vec<int8_t> byteToVectorList(int8_t* buffer, int length) {
    hidl_vec<int8_t> data;
    data.setToExternal(buffer, length);
    return data;
}

void eventNotify(int8_t* buffer, int32_t request, int32_t length) {
    ALOGD("eventNotify: buffer = %p, request_id = %d, length = %d, buffer size = %d ",
        buffer, request, length, sizeof(buffer));

    if (g_service != NULL && g_service->mIRcsIndication != NULL) {
        auto ret = g_service->mIRcsIndication->readEvent(
            0, request, byteToVectorList(buffer, length));
        if (!ret.isOk()) {
            ALOGE("eventNotify: RcsIndication HIDL call fail");
        }
    }
}

Return<void> IRcsImpl::writeEvent(int32_t account, int32_t request, const hidl_vec<int8_t>& value) {
    ALOGD("writeEvent: account=%d, request=%d, value address = %p, value size = %d",
        account, request, value.data(), value.size());
    char* data = (char *)value.data();
    int32_t length = (int32_t)value.size();
    writeEventHal(request, length, data);
    return Void();
}

Return<void> IRcsImpl::setResponseFunctions(const android::sp<IRcsIndication>& rcsIndication) {
    ALOGD("setResponseFunctions");
    mIRcsIndication = rcsIndication;
    mIRcsIndication->linkToDeath(g_rcsHalDeathRecipient, 0);
    return Void();
}

