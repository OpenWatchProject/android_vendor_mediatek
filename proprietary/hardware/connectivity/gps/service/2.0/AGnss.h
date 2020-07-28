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

#ifndef ANDROID_HARDWARE_GNSS_V2_0_AGNSS_H
#define ANDROID_HARDWARE_GNSS_V2_0_AGNSS_H

#include <ThreadCreationWrapper.h>
#include <android/hardware/gnss/2.0/IAGnss.h>
#include <hardware/gps_internal.h>
#include <hardware/gps_mtk.h>
#include <hidl/Status.h>
#include <netinet/in.h>

namespace android {
namespace hardware {
namespace gnss {
namespace V2_0 {
namespace implementation {

using ::android::hardware::gnss::V2_0::IAGnss;
using ::android::hardware::gnss::V2_0::IAGnssCallback;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_string;
using ::android::sp;

/*
 * Extended interface for AGNSS support. Also contains wrapper methods to allow
 * methods from IAGnssCallback interface to be passed into the conventional
 * implementation of the GNSS HAL.
 */
struct AGnss : public IAGnss {
    AGnss(const AGpsInterface_ext* agpsIface);
    ~AGnss();
    /*
     * Methods from ::android::hardware::gnss::V1_0::IAGnss interface follow.
     * These declarations were generated from IAGnss.hal.
     */
    //Return<void> setCallback(const sp<V1_0::IAGnssCallback>& callback) override;
    // Methods from ::android::hardware::gnss::V2_0::IAGnss follow.
    Return<void> setCallback(const sp<V2_0::IAGnssCallback>& callback) override;
    Return<bool> dataConnClosed() override;
    Return<bool> dataConnFailed() override;
    Return<bool> setServer(V2_0::IAGnssCallback::AGnssType type,
                         const hidl_string& hostname, int32_t port) override;
    Return<bool> dataConnOpen(uint64_t networkHandle, const hidl_string& apn,
                              IAGnss::ApnIpType apnIpType) override;

    /*
     * Callback methods to be passed into the conventional GNSS HAL by the default
     * implementation. These methods are not part of the IAGnss base class.
     */
    static pthread_t createThreadCb(const char* name, void (*start)(void*), void* arg);
    static void statusCb(AGpsStatus* status);

    /*
     * Holds function pointers to the callback methods.
     */
    static AGpsCallbacks sAGnssCb;

 private:
    const AGpsInterface_ext* mAGnssIface = nullptr;
    static sp<IAGnssCallback> sAGnssCbIface;
    static std::vector<std::unique_ptr<ThreadFuncArgs>> sThreadFuncArgsList;
    static bool sInterfaceExists;
};

}  // namespace implementation
}  // namespace V2_0
}  // namespace gnss
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_GNSS_V2_0_AGNSS_H