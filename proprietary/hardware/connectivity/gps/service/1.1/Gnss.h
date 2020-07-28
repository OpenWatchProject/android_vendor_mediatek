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

#ifndef android_hardware_gnss_V1_1_Gnss_H_
#define android_hardware_gnss_V1_1_Gnss_H_

#include <AGnss.h>
#include <AGnssRil.h>
#include <GnssBatching.h>
#include <GnssConfiguration.h>
#include <GnssDebug.h>
#include <GnssGeofencing.h>
#include <GnssMeasurement.h>
#include <GnssNavigationMessage.h>
#include <GnssNi.h>
#include <GnssXtra.h>

#include <ThreadCreationWrapper.h>
#include <android/hardware/gnss/1.1/IGnss.h>
#include <hardware/fused_location.h>
#include <hardware/gps.h>
#include <hardware/gps_mtk.h>
#include <hidl/Status.h>

#include <semaphore.h>

namespace android {
namespace hardware {
namespace gnss {
namespace V1_1 {
namespace implementation {

using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_string;
using ::android::sp;

using LegacyGnssSystemInfo = ::GnssSystemInfo;
using GnssConstellationType = V1_0::GnssConstellationType;
using GnssLocation = V1_0::GnssLocation;

/*
 * Represents the standard GNSS interface. Also contains wrapper methods to allow methods from
 * IGnssCallback interface to be passed into the conventional implementation of the GNSS HAL.
 */
class Gnss : public IGnss {
  public:
    Gnss(gps_device_t_ext* gnss_device);
    ~Gnss();

    // Methods from ::android::hardware::gnss::V1_0::IGnss follow.
    Return<bool> setCallback(
        const sp<V1_0::IGnssCallback>& callback) override;
    Return<bool> start() override;
    Return<bool> stop() override;
    Return<void> cleanup() override;
    Return<bool> injectTime(int64_t timeMs, int64_t timeReferenceMs,
                            int32_t uncertaintyMs) override;
    Return<bool> injectLocation(double latitudeDegrees, double longitudeDegrees,
                                float accuracyMeters) override;
    Return<void> deleteAidingData(V1_0::IGnss::GnssAidingData aidingDataFlags) override;
    Return<bool> setPositionMode(
        V1_0::IGnss::GnssPositionMode mode,
        V1_0::IGnss::GnssPositionRecurrence recurrence,
        uint32_t minIntervalMs, uint32_t preferredAccuracyMeters,
        uint32_t preferredTimeMs) override;
    Return<sp<V1_0::IAGnssRil>> getExtensionAGnssRil() override;
    Return<sp<V1_0::IGnssGeofencing>> getExtensionGnssGeofencing() override;
    Return<sp<V1_0::IAGnss>> getExtensionAGnss() override;
    Return<sp<V1_0::IGnssNi>> getExtensionGnssNi() override;
    Return<sp<V1_0::IGnssMeasurement>> getExtensionGnssMeasurement() override;
    Return<sp<V1_0::IGnssNavigationMessage>>
    getExtensionGnssNavigationMessage() override;
    Return<sp<V1_0::IGnssXtra>> getExtensionXtra() override;
    Return<sp<V1_0::IGnssConfiguration>> getExtensionGnssConfiguration() override;
    Return<sp<V1_0::IGnssDebug>> getExtensionGnssDebug() override;
    Return<sp<V1_0::IGnssBatching>> getExtensionGnssBatching() override;

    // Methods from ::android::hardware::gnss::V1_1::IGnss follow.
    Return<bool> setCallback_1_1(
        const sp<V1_1::IGnssCallback>& callback) override;
    Return<bool> setPositionMode_1_1(
        V1_0::IGnss::GnssPositionMode mode,
        V1_0::IGnss::GnssPositionRecurrence recurrence,
        uint32_t minIntervalMs, uint32_t preferredAccuracyMeters, uint32_t preferredTimeMs,
        bool lowPowerMode) override;
    Return<sp<V1_1::IGnssConfiguration>>
    getExtensionGnssConfiguration_1_1() override;
    Return<sp<V1_1::IGnssMeasurement>> getExtensionGnssMeasurement_1_1() override;
    Return<bool> injectBestLocation(const V1_0::GnssLocation& location) override;


    /*
     * Callback methods to be passed into the conventional GNSS HAL by the default
     * implementation. These methods are not part of the IGnss base class.
     */
    static void locationCb(GpsLocation_ext* location);
    static void statusCb(GpsStatus* gnss_status);
    static void nmeaCb(GpsUtcTime timestamp, const char* nmea, int length);
    static void setCapabilitiesCb(uint32_t capabilities);
    static void acquireWakelockCb();
    static void releaseWakelockCb();
    static void requestUtcTimeCb();
    static pthread_t createThreadCb(const char* name, void (*start)(void*), void* arg);
    static void gnssSvStatusCb(GnssSvStatus_ext* status);
    /*
     * Deprecated callback added for backward compatibility to devices that do
     * not support GnssSvStatus.
     */
    static void gpsSvStatusCb(GpsSvStatus* status);
    static void setSystemInfoCb(const LegacyGnssSystemInfo* info);

    /*
     * Wakelock consolidation, only needed for dual use of a gps.h & fused_location.h HAL
     *
     * Ensures that if the last call from either legacy .h was to acquire a wakelock, that a
     * wakelock is held.  Otherwise releases it.
     */
    static void acquireWakelockFused();
    static void releaseWakelockFused();

    static void setNameCb(const char* name, int length);
    static void requestLocationCb(bool independentFromGnss);

    /*
     * Holds function pointers to the callback methods.
     */
    static GpsCallbacks_ext sGnssCb;

 private:
    /*
     * For handling system-server death while GNSS service lives on.
     */
    class GnssHidlDeathRecipient : public hidl_death_recipient {
      public:
        GnssHidlDeathRecipient(const sp<Gnss> gnss) : mGnss(gnss) {
        }

        virtual void serviceDied(uint64_t /*cookie*/,
                const wp<::android::hidl::base::V1_0::IBase>& /*who*/) {
            mGnss->handleHidlDeath();
        }
      private:
        sp<Gnss> mGnss;
    };

    // for wakelock consolidation, see above
    static void acquireWakelockGnss();
    static void releaseWakelockGnss();
    static void updateWakelock();
    static bool sWakelockHeldGnss;
    static bool sWakelockHeldFused;

    /*
     * Cleanup for death notification
     */
    void handleHidlDeath();

    sp<V1_0::implementation::GnssXtra> mGnssXtraIface = nullptr;
    sp<V1_0::implementation::AGnssRil> mGnssRil = nullptr;
    sp<V1_0::implementation::GnssGeofencing> mGnssGeofencingIface = nullptr;
    sp<V1_0::implementation::AGnss> mAGnssIface = nullptr;
    sp<V1_0::implementation::GnssNi> mGnssNi = nullptr;
    sp<V1_1::implementation::GnssMeasurement> mGnssMeasurement = nullptr;
    sp<V1_0::implementation::GnssNavigationMessage> mGnssNavigationMessage = nullptr;
    sp<V1_0::implementation::GnssDebug> mGnssDebug = nullptr;
    sp<V1_1::implementation::GnssConfiguration> mGnssConfig = nullptr;
    sp<V1_0::implementation::GnssBatching> mGnssBatching = nullptr;

    ///M: add semphore protection
    static sem_t sSem;

    sp<GnssHidlDeathRecipient> mDeathRecipient;
    const GpsInterface_ext* mGnssIface = nullptr;
    static sp<V1_1::IGnssCallback> sGnssCbIface;
    static std::vector<std::unique_ptr<ThreadFuncArgs>> sThreadFuncArgsList;
    static bool sInterfaceExists;

    // Values saved for resend
    static uint32_t sCapabilitiesCached;
    static uint16_t sYearOfHwCached;
};

extern "C" IGnss* HIDL_FETCH_IGnss(const char* name);

}  // namespace implementation
}  // namespace V1_1
}  // namespace gnss
}  // namespace hardware
}  // namespace android

#endif  // android_hardware_gnss_V1_1_Gnss_H_
