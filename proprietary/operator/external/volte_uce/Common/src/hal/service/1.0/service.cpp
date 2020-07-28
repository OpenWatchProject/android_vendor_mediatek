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

#define LOG_TAG "UCE_HAL"

#include <android-base/logging.h>
#include <hidl/HidlTransportSupport.h>
#include <utils/Looper.h>
#include "Presence.h"
#include "PresenceIndication.h"
#include "HidlService.h"
#include "uce_proxy_adapter.h"


using vendor::mediatek::hardware::presence::V1_0::IPresence;
using vendor::mediatek::hardware::presence::V1_0::implementation::Presence;
using vendor::mediatek::hardware::presence::V1_0::IPresenceIndication;
using android::hardware::Return;
using android::hardware::hidl_death_recipient;
using android::hardware::hidl_vec;
using android::hardware::Void;
using android::hidl::base::V1_0::IBase;

using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using ::android::Mutex;

struct IPresenceImpl : public IPresence {
    android::sp<IPresenceIndication> mIPresenceIndication;
    Return<void> setResponseFunctions(const android::sp<IPresenceIndication>& presenceIndication);
    Return<void> writeEvent(int32_t request_id, int32_t length, const hidl_vec<int8_t>& value);
    Return<void> writeBytes(const hidl_vec<int8_t>& value);
    Return<void> writeInt(int32_t value);
};

static android::sp<IPresenceImpl> g_service;
struct PresenceDeathRecipient : virtual public hidl_death_recipient {
    virtual void serviceDied(uint64_t cookie __unused, const android::wp<IBase> &who __unused) override {
        ALOGW("%s(), Presence hal died, set Presence indication to null", __FUNCTION__);
        if (g_service != NULL) {
            g_service->mIPresenceIndication = NULL;
        }
    }
};

static android::sp<PresenceDeathRecipient> g_presenceHalDeathRecipient = NULL;
static Mutex mLock;

int startHidlService() {
    android::status_t status;

    ALOGD("Presence Hal is starting up...");
    configureRpcThreadpool(1, true /* callerWillJoin */);
    //Setup hwbinder service
    g_service = new IPresenceImpl();
    status = g_service->registerAsService("presence_hal_service");

    g_presenceHalDeathRecipient = new PresenceDeathRecipient();

    if (status != android::OK) {
        ALOGE("Error while registering presence hidl service: %d", status);
    } else {
        ALOGD("Presence Hal is started");
    }

    joinRpcThreadpool();
    ALOGD("Presence hal is terminating...");

    return status;
}

hidl_vec<int8_t> byteToVectorList(int8_t* buffer, int length) {
    hidl_vec<int8_t> data;
    data.setToExternal(buffer, length);
    return data;
}

void eventNotify(int8_t* buffer, int32_t request_id, int32_t length) {
    ALOGD("eventNotify: buffer = %p, request_id = %d, length = %d, buffer size = %d ",
        buffer, request_id, length, sizeof(buffer));
    Mutex::Autolock autoLock(mLock);
    if (g_service != NULL && g_service->mIPresenceIndication != NULL) {
        g_service->mIPresenceIndication->readEvent(byteToVectorList(buffer, length), request_id, length);
    }
}

Return<void> IPresenceImpl::writeEvent(int32_t request_id, int32_t length, const hidl_vec<int8_t>& value) {
    ALOGD("writeEvent: request_id=%d, length=%d, value address = %p, value size = %d", request_id, length, value.data(), value.size());
    char* data = (char *)value.data();
    writeEventHal(request_id, length, data);
    return Void();
}

Return<void> IPresenceImpl::writeBytes(const hidl_vec<int8_t>& value) {
    ALOGD("writeBytes: value size = %d", value.size());
    //reserved
    return Void();
}

Return<void> IPresenceImpl::writeInt(int32_t value) {
    ALOGD("writeInt: value = %d", value);
    //reserved
    return Void();
}

Return<void> IPresenceImpl::setResponseFunctions(const android::sp<IPresenceIndication>& presenceIndication) {
    ALOGD("setResponseFunctions");
    Mutex::Autolock autoLock(mLock);
    mIPresenceIndication = presenceIndication;
    mIPresenceIndication->linkToDeath(g_presenceHalDeathRecipient, 0);
/*for test
    if(mImsaIndication != NULL) {
        ALOGD("IMSA HIDL : callback to java part test !!");
        int testdata[] = {1, 2, 3};
        void *indication = testdata;
        hidl_vec<int8_t> data;
        data.setToExternal((int8_t *) indication, sizeof(indication));
        mImsaIndication->readEvent(data, 123, 3);
    }
*/
    return Void();
}

