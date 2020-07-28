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

#ifndef ANDROID_HARDWARE_GNSS_V2_0_AGNSSRIL_H
#define ANDROID_HARDWARE_GNSS_V2_0_AGNSSRIL_H

#include <ThreadCreationWrapper.h>
#include <android/hardware/gnss/2.0/IAGnssRil.h>
#include <hardware/gps.h>
#include <hardware/gps_mtk.h>
#include <hidl/Status.h>

namespace android {
namespace hardware {
namespace gnss {
namespace V2_0 {
namespace implementation {

using ::android::hardware::gnss::V2_0::IAGnssRil;
using ::android::hardware::gnss::V1_0::IAGnssRilCallback;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_string;
using ::android::sp;

/*
 * Extended interface for AGNSS RIL support. An Assisted GNSS Radio Interface Layer interface
 * allows the GNSS chipset to request radio interface layer information from Android platform.
 * Examples of such information are reference location, unique subscriber ID, phone number string
 * and network availability changes. Also contains wrapper methods to allow methods from
 * IAGnssiRilCallback interface to be passed into the conventional implementation of the GNSS HAL.
 */
struct AGnssRil : public IAGnssRil {
    AGnssRil(const AGpsRilInterface_ext* aGpsRilIface);
    ~AGnssRil();

    /*
     * Methods from ::android::hardware::gnss::V1_0::IAGnssRil follow.
     * These declarations were generated from IAGnssRil.hal.
     */
    Return<void> setCallback(const sp<V1_0::IAGnssRilCallback>& callback) override;
    Return<void> setRefLocation(const V1_0::IAGnssRil::AGnssRefLocation& agnssReflocation) override;
    Return<bool> setSetId(V1_0::IAGnssRil::SetIDType type, const hidl_string& setid) override;
    Return<bool> updateNetworkState(bool connected,
                                    V1_0::IAGnssRil::NetworkType type,
                                    bool roaming) override;
    Return<bool> updateNetworkAvailability(bool available, const hidl_string& apn) override;
    static void requestSetId(uint32_t flags);
    static void requestRefLoc(uint32_t flags);

    /*
     * Callback method to be passed into the conventional GNSS HAL by the default
     * implementation. This method is not part of the IAGnssRil base class.
     */
    static pthread_t createThreadCb(const char* name, void (*start)(void*), void* arg);

    /*
     * Holds function pointers to the callback methods.
     */
    static AGpsRilCallbacks sAGnssRilCb;

    // Methods from ::android::hardware::gnss::V2_0::IAGnssRil follow.
    Return<bool> updateNetworkState_2_0(
        const V2_0::IAGnssRil::NetworkAttributes& attributes) override;

 private:
    const AGpsRilInterface_ext* mAGnssRilIface = nullptr;
    static sp<IAGnssRilCallback> sAGnssRilCbIface;
    static std::vector<std::unique_ptr<ThreadFuncArgs>> sThreadFuncArgsList;
    static bool sInterfaceExists;
};

}  // namespace implementation
}  // namespace V2_0
}  // namespace gnss
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_GNSS_V2_0_AGNSSRIL_H
