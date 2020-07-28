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

#define LOG_TAG "GnssHAL_GnssInterface"

#include "Gnss.h"
#include <GnssUtils.h>

namespace android {
namespace hardware {
namespace gnss {
namespace V1_1 {
namespace implementation {

std::vector<std::unique_ptr<ThreadFuncArgs>> Gnss::sThreadFuncArgsList;
sp<IGnssCallback> Gnss::sGnssCbIface = nullptr;
bool Gnss::sInterfaceExists = false;
bool Gnss::sWakelockHeldGnss = false;
bool Gnss::sWakelockHeldFused = false;

GpsCallbacks_ext Gnss::sGnssCb = {
    .size = sizeof(GpsCallbacks_ext),
    .location_cb = locationCb,
    .status_cb = statusCb,
    .sv_status_cb = gpsSvStatusCb,
    .nmea_cb = nmeaCb,
    .set_capabilities_cb = setCapabilitiesCb,
    .acquire_wakelock_cb = acquireWakelockCb,
    .release_wakelock_cb = releaseWakelockCb,
    .create_thread_cb = createThreadCb,
    .request_utc_time_cb = requestUtcTimeCb,
    .set_system_info_cb = setSystemInfoCb,
    .gnss_sv_status_cb = gnssSvStatusCb,

    .set_name_cb = setNameCb,
    .request_location_cb = requestLocationCb
};

uint32_t Gnss::sCapabilitiesCached = 0;
uint16_t Gnss::sYearOfHwCached = 0;
sem_t Gnss::sSem;

Gnss::Gnss(gps_device_t_ext* gnssDevice) :
        mDeathRecipient(new GnssHidlDeathRecipient(this)) {
    /* Error out if an instance of the interface already exists. */
    LOG_ALWAYS_FATAL_IF(sInterfaceExists);
    sInterfaceExists = true;

    if (gnssDevice == nullptr) {
        ALOGE("%s: Invalid device_t handle", __func__);
        return;
    }

    mGnssIface = gnssDevice->get_gps_interface(gnssDevice);
    sem_init(&sSem, 0, 1);
}

Gnss::~Gnss() {
    sInterfaceExists = false;
    sThreadFuncArgsList.clear();
    sem_destroy(&sSem);
}

void Gnss::locationCb(GpsLocation_ext* location) {
    sem_wait(&sSem);
    if (sGnssCbIface == nullptr) {
        ALOGE("%s: GNSS Callback Interface configured incorrectly", __func__);
        sem_post(&sSem);
        return;
    }

    if (location == nullptr) {
        ALOGE("%s: Invalid location from GNSS HAL", __func__);
        sem_post(&sSem);
        return;
    }

    GnssLocation gnssLocation = V1_0::implementation::convertToGnssLocation(location);
    auto ret = sGnssCbIface->gnssLocationCb(gnssLocation);
    if (!ret.isOk()) {
        ALOGE("%s: Unable to invoke callback", __func__);
    }
    sem_post(&sSem);
}

void Gnss::statusCb(GpsStatus* gnssStatus) {
    sem_wait(&sSem);
    if (sGnssCbIface == nullptr) {
        ALOGE("%s: GNSS Callback Interface configured incorrectly", __func__);
        sem_post(&sSem);
        return;
    }

    if (gnssStatus == nullptr) {
        ALOGE("%s: Invalid GpsStatus from GNSS HAL", __func__);
        sem_post(&sSem);
        return;
    }

    IGnssCallback::GnssStatusValue status =
            static_cast<IGnssCallback::GnssStatusValue>(gnssStatus->status);

    auto ret = sGnssCbIface->gnssStatusCb(status);
    if (!ret.isOk()) {
        ALOGE("%s: Unable to invoke callback", __func__);
    }
    sem_post(&sSem);
}

void Gnss::gnssSvStatusCb(GnssSvStatus_ext* status) {
    sem_wait(&sSem);
    if (sGnssCbIface == nullptr) {
        ALOGE("%s: GNSS Callback Interface configured incorrectly", __func__);
        sem_post(&sSem);
        return;
    }

    if (status == nullptr) {
        ALOGE("Invalid status from GNSS HAL %s", __func__);
        sem_post(&sSem);
        return;
    }

    IGnssCallback::GnssSvStatus svStatus;
    svStatus.numSvs = status->num_svs;

    if (svStatus.numSvs > static_cast<uint32_t>(V1_0::GnssMax::SVS_COUNT)) {
        ALOGW("Too many satellites %u. Clamps to %d.", svStatus.numSvs, V1_0::GnssMax::SVS_COUNT);
        svStatus.numSvs = static_cast<uint32_t>(V1_0::GnssMax::SVS_COUNT);
    }

    for (size_t i = 0; i < svStatus.numSvs; i++) {
        auto svInfo = status->gnss_sv_list[i];
        IGnssCallback::GnssSvInfo gnssSvInfo = {
            .svid = svInfo.legacySvInfo.svid,
            .constellation = static_cast<V1_0::GnssConstellationType>(
                    svInfo.legacySvInfo.constellation),
            .cN0Dbhz = svInfo.legacySvInfo.c_n0_dbhz,
            .elevationDegrees = svInfo.legacySvInfo.elevation,
            .azimuthDegrees = svInfo.legacySvInfo.azimuth,
            .svFlag = static_cast<uint8_t>(svInfo.legacySvInfo.flags),
            .carrierFrequencyHz = svInfo.carrier_frequency};
        svStatus.gnssSvList[i] = gnssSvInfo;
    }

    auto ret = sGnssCbIface->gnssSvStatusCb(svStatus);
    if (!ret.isOk()) {
        ALOGE("%s: Unable to invoke callback", __func__);
    }
    sem_post(&sSem);
}

/*
 * This enum is used by gpsSvStatusCb() method below to convert GpsSvStatus
 * to GnssSvStatus for backward compatibility. It is only used by the default
 * implementation and is not part of the GNSS interface.
 */
enum SvidValues : uint16_t {
    GLONASS_SVID_OFFSET = 64,
    GLONASS_SVID_COUNT = 24,
    BEIDOU_SVID_OFFSET = 200,
    BEIDOU_SVID_COUNT = 35,
    SBAS_SVID_MIN = 33,
    SBAS_SVID_MAX = 64,
    SBAS_SVID_ADD = 87,
    QZSS_SVID_MIN = 193,
    QZSS_SVID_MAX = 200
};

/*
 * The following code that converts GpsSvStatus to GnssSvStatus is moved here from
 * GnssLocationProvider. GnssLocationProvider does not require it anymore since GpsSvStatus is
 * being deprecated and is no longer part of the GNSS interface.
 */
void Gnss::gpsSvStatusCb(GpsSvStatus* svInfo) {
    sem_wait(&sSem);
    if (sGnssCbIface == nullptr) {
        ALOGE("%s: GNSS Callback Interface configured incorrectly", __func__);
        sem_post(&sSem);
        return;
    }

    if (svInfo == nullptr) {
        ALOGE("Invalid status from GNSS HAL %s", __func__);
        sem_post(&sSem);
        return;
    }

    IGnssCallback::GnssSvStatus svStatus;
    svStatus.numSvs = svInfo->num_svs;
    /*
     * Clamp the list size since GnssSvStatus can support a maximum of
     * GnssMax::SVS_COUNT entries.
     */
    ///M: fix max numSvs as GPS_MAX_SVS
    if (svStatus.numSvs > static_cast<uint32_t>(GPS_MAX_SVS)) {
        ALOGW("Too many satellites %u. Clamps to %d.", svStatus.numSvs, GPS_MAX_SVS);
        svStatus.numSvs = static_cast<uint32_t>(GPS_MAX_SVS);
    }
    /// M: mtk update end

    uint32_t ephemerisMask = svInfo->ephemeris_mask;
    uint32_t almanacMask = svInfo->almanac_mask;
    uint32_t usedInFixMask = svInfo->used_in_fix_mask;
    /*
     * Conversion from GpsSvInfo to IGnssCallback::GnssSvInfo happens below.
     */
    for (size_t i = 0; i < svStatus.numSvs; i++) {
        IGnssCallback::GnssSvInfo& info = svStatus.gnssSvList[i];
        info.svid = svInfo->sv_list[i].prn;
        if (info.svid >= 1 && info.svid <= 32) {
            info.constellation = GnssConstellationType::GPS;
        } else if (info.svid > GLONASS_SVID_OFFSET &&
                   info.svid <= GLONASS_SVID_OFFSET + GLONASS_SVID_COUNT) {
            info.constellation = GnssConstellationType::GLONASS;
            info.svid -= GLONASS_SVID_OFFSET;
        } else if (info.svid > BEIDOU_SVID_OFFSET &&
                 info.svid <= BEIDOU_SVID_OFFSET + BEIDOU_SVID_COUNT) {
            info.constellation = GnssConstellationType::BEIDOU;
            info.svid -= BEIDOU_SVID_OFFSET;
        } else if (info.svid >= SBAS_SVID_MIN && info.svid <= SBAS_SVID_MAX) {
            info.constellation = GnssConstellationType::SBAS;
            info.svid += SBAS_SVID_ADD;
        } else if (info.svid >= QZSS_SVID_MIN && info.svid <= QZSS_SVID_MAX) {
            info.constellation = GnssConstellationType::QZSS;
        } else {
            ALOGD("Unknown constellation type with Svid = %d.", info.svid);
            info.constellation = GnssConstellationType::UNKNOWN;
        }

        info.cN0Dbhz = svInfo->sv_list[i].snr;
        info.elevationDegrees = svInfo->sv_list[i].elevation;
        info.azimuthDegrees = svInfo->sv_list[i].azimuth;
        // TODO: b/31702236
        info.svFlag = static_cast<uint8_t>(IGnssCallback::GnssSvFlags::NONE);

        /*
         * Only GPS info is valid for these fields, as these masks are just 32
         * bits, by GPS prn.
         */
        if (info.constellation == GnssConstellationType::GPS) {
            int32_t svidMask = (1 << (info.svid - 1));
            if ((ephemerisMask & svidMask) != 0) {
                info.svFlag |= IGnssCallback::GnssSvFlags::HAS_EPHEMERIS_DATA;
            }
            if ((almanacMask & svidMask) != 0) {
                info.svFlag |= IGnssCallback::GnssSvFlags::HAS_ALMANAC_DATA;
            }
            if ((usedInFixMask & svidMask) != 0) {
                info.svFlag |= IGnssCallback::GnssSvFlags::USED_IN_FIX;
            }
        }
    }

    auto ret = sGnssCbIface->gnssSvStatusCb(svStatus);
    if (!ret.isOk()) {
        ALOGE("%s: Unable to invoke callback", __func__);
    }
    sem_post(&sSem);
}

void Gnss::nmeaCb(GpsUtcTime timestamp, const char* nmea, int length) {
    sem_wait(&sSem);
    if (sGnssCbIface == nullptr) {
        ALOGE("%s: GNSS Callback Interface configured incorrectly", __func__);
        sem_post(&sSem);
        return;
    }

    android::hardware::hidl_string nmeaString;
    nmeaString.setToExternal(nmea, length);
    auto ret = sGnssCbIface->gnssNmeaCb(timestamp, nmeaString);
    if (!ret.isOk()) {
        ALOGE("%s: Unable to invoke callback", __func__);
    }
    sem_post(&sSem);
}

void Gnss::setCapabilitiesCb(uint32_t capabilities) {
    sem_wait(&sSem);
    if (sGnssCbIface == nullptr) {
        ALOGE("%s: GNSS Callback Interface configured incorrectly", __func__);
        sem_post(&sSem);
        return;
    }

    auto ret = sGnssCbIface->gnssSetCapabilitesCb(capabilities);
    if (!ret.isOk()) {
        ALOGE("%s: Unable to invoke callback", __func__);
    }

    // Save for reconnection when some legacy hal's don't resend this info
    sCapabilitiesCached = capabilities;
    sem_post(&sSem);
}

void Gnss::acquireWakelockCb() {
    acquireWakelockGnss();
}

void Gnss::releaseWakelockCb() {
    releaseWakelockGnss();
}


void Gnss::acquireWakelockGnss() {
    sWakelockHeldGnss = true;
    updateWakelock();
}

void Gnss::releaseWakelockGnss() {
    sWakelockHeldGnss = false;
    updateWakelock();
}

void Gnss::acquireWakelockFused() {
    sWakelockHeldFused = true;
    updateWakelock();
}

void Gnss::releaseWakelockFused() {
    sWakelockHeldFused = false;
    updateWakelock();
}

void Gnss::updateWakelock() {
    // Track the state of the last request - in case the wake lock in the layer above is reference
    // counted.
    static bool sWakelockHeld = false;

    sem_wait(&sSem);
    if (sGnssCbIface == nullptr) {
        ALOGE("%s: GNSS Callback Interface configured incorrectly", __func__);
        sem_post(&sSem);
        return;
    }

    if (sWakelockHeldGnss || sWakelockHeldFused) {
        if (!sWakelockHeld) {
            ALOGI("%s: GNSS HAL Wakelock acquired due to gps: %d, fused: %d", __func__,
                    sWakelockHeldGnss, sWakelockHeldFused);
            sWakelockHeld = true;
            auto ret = sGnssCbIface->gnssAcquireWakelockCb();
            if (!ret.isOk()) {
                ALOGE("%s: Unable to invoke callback", __func__);
            }
        }
    } else {
        if (sWakelockHeld) {
            ALOGI("%s: GNSS HAL Wakelock released", __func__);
        } else  {
            // To avoid burning power, always release, even if logic got here with sWakelock false
            // which it shouldn't, unless underlying *.h implementation makes duplicate requests.
            ALOGW("%s: GNSS HAL Wakelock released, duplicate request", __func__);
        }
        sWakelockHeld = false;
        auto ret = sGnssCbIface->gnssReleaseWakelockCb();
        if (!ret.isOk()) {
            ALOGE("%s: Unable to invoke callback", __func__);
        }
    }
    sem_post(&sSem);
}

void Gnss::requestUtcTimeCb() {
    sem_wait(&sSem);
    if (sGnssCbIface == nullptr) {
        ALOGE("%s: GNSS Callback Interface configured incorrectly", __func__);
        sem_post(&sSem);
        return;
    }

    auto ret = sGnssCbIface->gnssRequestTimeCb();
    if (!ret.isOk()) {
            ALOGE("%s: Unable to invoke callback", __func__);
    }
    sem_post(&sSem);
}

pthread_t Gnss::createThreadCb(const char* name, void (*start)(void*), void* arg) {
    return createPthread(name, start, arg, &sThreadFuncArgsList);
}

void Gnss::setSystemInfoCb(const LegacyGnssSystemInfo* info) {
    sem_wait(&sSem);
    if (sGnssCbIface == nullptr) {
        ALOGE("%s: GNSS Callback Interface configured incorrectly", __func__);
        sem_post(&sSem);
        return;
    }

    if (info == nullptr) {
        ALOGE("Invalid GnssSystemInfo from GNSS HAL %s", __func__);
        sem_post(&sSem);
        return;
    }

    IGnssCallback::GnssSystemInfo gnssInfo = {
        .yearOfHw = info->year_of_hw
    };

    auto ret = sGnssCbIface->gnssSetSystemInfoCb(gnssInfo);
    if (!ret.isOk()) {
            ALOGE("%s: Unable to invoke callback", __func__);
    }

    // Save for reconnection when some legacy hal's don't resend this info
    sYearOfHwCached = info->year_of_hw;
    sem_post(&sSem);
}

void Gnss::setNameCb(const char* name, int length) {
    sem_wait(&sSem);
    if (sGnssCbIface == nullptr) {
        ALOGE("%s: GNSS Callback Interface configured incorrectly", __func__);
        sem_post(&sSem);
        return;
    }

    android::hardware::hidl_string nameString;
    nameString.setToExternal(name, length);
    auto ret = sGnssCbIface->gnssNameCb(nameString);
    if (!ret.isOk()) {
        ALOGE("%s: Unable to invoke callback", __func__);
    }
    sem_post(&sSem);
}

void Gnss::requestLocationCb(bool independentFromGnss) {
    sem_wait(&sSem);
    if (sGnssCbIface == nullptr) {
        ALOGE("%s: GNSS Callback Interface configured incorrectly", __func__);
        sem_post(&sSem);
        return;
    }

    auto ret = sGnssCbIface->gnssRequestLocationCb(independentFromGnss);
    if (!ret.isOk()) {
        ALOGE("%s: Unable to invoke callback", __func__);
    }
    sem_post(&sSem);
}



// Methods from ::android::hardware::gnss::V1_0::IGnss follow.
Return<bool> Gnss::setCallback(const sp<::android::hardware::gnss::V1_0::IGnssCallback>&) {
    return false;
}

// Methods from ::android::hardware::gnss::V1_1::IGnss follow.
Return<bool> Gnss::setCallback_1_1(
    const sp<::android::hardware::gnss::V1_1::IGnssCallback>& callback) {
    if (mGnssIface == nullptr) {
        ALOGE("%s: Gnss interface is unavailable", __func__);
        return false;
    }

    if (callback == nullptr)  {
        ALOGE("%s: Null callback ignored", __func__);
        return false;
    }

    sem_wait(&sSem);
    if (sGnssCbIface != NULL) {
        ALOGW("%s called more than once. Unexpected unless test.", __func__);
        sGnssCbIface->unlinkToDeath(mDeathRecipient);
    }

    sGnssCbIface = callback;
    callback->linkToDeath(mDeathRecipient, 0 /*cookie*/);
    sem_post(&sSem);

    // If this was received in the past, send it up again to refresh caller.
    // mGnssIface will override after init() is called below, if needed
    // (though it's unlikely the gps.h capabilities or system info will change.)
    if (sCapabilitiesCached != 0) {
        setCapabilitiesCb(sCapabilitiesCached);
    }
    if (sYearOfHwCached != 0) {
        LegacyGnssSystemInfo info;
        info.year_of_hw = sYearOfHwCached;
        setSystemInfoCb(&info);
    }

    return (mGnssIface->init(&sGnssCb) == 0);
}

Return<bool> Gnss::start()  {
    if (mGnssIface == nullptr) {
        ALOGE("%s: Gnss interface is unavailable", __func__);
        return false;
    }

    return (mGnssIface->start() == 0);
}

Return<bool> Gnss::stop()  {
    if (mGnssIface == nullptr) {
        ALOGE("%s: Gnss interface is unavailable", __func__);
        return false;
    }

    return (mGnssIface->stop() == 0);
}

Return<void> Gnss::cleanup()  {
    if (mGnssIface == nullptr) {
        ALOGE("%s: Gnss interface is unavailable", __func__);
    } else {
        mGnssIface->cleanup();
    }
    return Void();
}

Return<bool> Gnss::injectLocation(double latitudeDegrees,
                                  double longitudeDegrees,
                                  float accuracyMeters)  {
    if (mGnssIface == nullptr) {
        ALOGE("%s: Gnss interface is unavailable", __func__);
        return false;
    }

    return (mGnssIface->inject_location(latitudeDegrees, longitudeDegrees, accuracyMeters) == 0);
}

Return<bool> Gnss::injectBestLocation(const GnssLocation&) {
    if (mGnssIface == nullptr) {
        ALOGE("%s: Gnss interface is unavailable", __func__);
        return false;
    }

    double latitudeDegrees = 0;
    double longitudeDegrees = 0;
    float accuracyMeters = 0;

    return (mGnssIface->inject_fused_location(latitudeDegrees, longitudeDegrees, accuracyMeters) == 0);
}


Return<bool> Gnss::injectTime(int64_t timeMs, int64_t timeReferenceMs,
                              int32_t uncertaintyMs) {
    if (mGnssIface == nullptr) {
        ALOGE("%s: Gnss interface is unavailable", __func__);
        return false;
    }

    return (mGnssIface->inject_time(timeMs, timeReferenceMs, uncertaintyMs) == 0);
}

Return<void> Gnss::deleteAidingData(V1_0::IGnss::GnssAidingData aidingDataFlags) {
    if (mGnssIface == nullptr) {
        ALOGE("%s: Gnss interface is unavailable", __func__);
    } else {
        mGnssIface->delete_aiding_data(static_cast<GpsAidingData>(aidingDataFlags));
    }
    return Void();
}

Return<bool> Gnss::setPositionMode(V1_0::IGnss::GnssPositionMode,
        V1_0::IGnss::GnssPositionRecurrence, uint32_t, uint32_t, uint32_t)  {
    return false;
}

Return<bool> Gnss::setPositionMode_1_1(
    ::android::hardware::gnss::V1_0::IGnss::GnssPositionMode mode,
    ::android::hardware::gnss::V1_0::IGnss::GnssPositionRecurrence recurrence,
    uint32_t minIntervalMs,
    uint32_t preferredAccuracyMeters,
    uint32_t preferredTimeMs,
    bool lowPowerMode) {

    if (mGnssIface == nullptr) {
        ALOGE("%s: Gnss interface is unavailable", __func__);
        return false;
    }

    return (mGnssIface->set_position_mode(static_cast<GpsPositionMode>(mode),
                                          static_cast<GpsPositionRecurrence>(recurrence),
                                          minIntervalMs,
                                          preferredAccuracyMeters,
                                          preferredTimeMs,
                                          lowPowerMode) == 0);
}

Return<sp<V1_0::IAGnssRil>> Gnss::getExtensionAGnssRil() {
    if (mGnssIface == nullptr) {
        ALOGE("%s: Gnss interface is unavailable", __func__);
        return nullptr;
    }

    if (mGnssRil == nullptr) {
        const AGpsRilInterface* agpsRilIface = static_cast<const AGpsRilInterface*>(
                mGnssIface->get_extension(AGPS_RIL_INTERFACE));
        if (agpsRilIface == nullptr) {
            ALOGI("%s: GnssRil interface not implemented by HAL", __func__);
        } else {
            mGnssRil = new V1_0::implementation::AGnssRil(agpsRilIface);
        }
    }
    return mGnssRil;
}

Return<sp<V1_0::IGnssConfiguration>> Gnss::getExtensionGnssConfiguration()  {
    return nullptr;
}

Return<sp<V1_1::IGnssConfiguration>> Gnss::getExtensionGnssConfiguration_1_1()  {
    if (mGnssIface == nullptr) {
        ALOGE("%s: Gnss interface is unavailable", __func__);
        return nullptr;
    }

    if (mGnssConfig == nullptr) {
        const GnssConfigurationInterface_ext* gnssConfigIface =
                static_cast<const GnssConfigurationInterface_ext*>(
                        mGnssIface->get_extension(GNSS_CONFIGURATION_INTERFACE));

        if (gnssConfigIface == nullptr) {
            ALOGW("%s: GnssConfiguration interface not implemented by HAL", __func__);
        } else {
            mGnssConfig = new V1_1::implementation::GnssConfiguration(gnssConfigIface);
        }
    }
    return mGnssConfig;
}

Return<sp<V1_0::IGnssGeofencing>> Gnss::getExtensionGnssGeofencing()  {
    if (mGnssIface == nullptr) {
        ALOGE("%s: Gnss interface is unavailable", __func__);
        return nullptr;
    }

    if (mGnssGeofencingIface == nullptr) {
        const GpsGeofencingInterface_ext* gpsGeofencingIface =
                static_cast<const GpsGeofencingInterface_ext*>(
                        mGnssIface->get_extension(GPS_GEOFENCING_INTERFACE));

        if (gpsGeofencingIface == nullptr) {
            ALOGE("%s: GnssGeofencing interface not implemented by HAL", __func__);
        } else {
            mGnssGeofencingIface = new V1_0::implementation::GnssGeofencing(gpsGeofencingIface);
        }
    }

    return mGnssGeofencingIface;
}

Return<sp<V1_0::IAGnss>> Gnss::getExtensionAGnss()  {
    if (mGnssIface == nullptr) {
        ALOGE("%s: Gnss interface is unavailable", __func__);
        return nullptr;
    }

    if (mAGnssIface == nullptr) {
        const AGpsInterface* agpsIface = static_cast<const AGpsInterface*>(
                mGnssIface->get_extension(AGPS_INTERFACE));
        if (agpsIface == nullptr) {
            ALOGE("%s: AGnss interface not implemented by HAL", __func__);
        } else {
            mAGnssIface = new V1_0::implementation::AGnss(agpsIface);
        }
    }
    return mAGnssIface;
}

Return<sp<V1_0::IGnssNi>> Gnss::getExtensionGnssNi()  {
    if (mGnssIface == nullptr) {
        ALOGE("%s: Gnss interface is unavailable", __func__);
        return nullptr;
    }

    if (mGnssNi == nullptr) {
        const GpsNiInterface* gpsNiIface = static_cast<const GpsNiInterface*>(
                mGnssIface->get_extension(GPS_NI_INTERFACE));
        if (gpsNiIface == nullptr) {
            ALOGI("%s: GnssNi interface not implemented by HAL", __func__);
        } else {
            mGnssNi = new V1_0::implementation::GnssNi(gpsNiIface);
        }
    }
    return mGnssNi;
}

Return<sp<V1_0::IGnssMeasurement>> Gnss::getExtensionGnssMeasurement() {
    return nullptr;
}

Return<sp<V1_1::IGnssMeasurement>> Gnss::getExtensionGnssMeasurement_1_1() {

    if (mGnssIface == nullptr) {
        ALOGE("%s: Gnss interface is unavailable", __func__);
        return nullptr;
    }

    if (mGnssMeasurement == nullptr) {
        const GpsMeasurementInterface_ext* gpsMeasurementIface =
                static_cast<const GpsMeasurementInterface_ext*>(
                        mGnssIface->get_extension(GPS_MEASUREMENT_INTERFACE));

        if (gpsMeasurementIface == nullptr) {
            ALOGE("%s: GnssMeasurement interface not implemented by HAL", __func__);
        } else {
            mGnssMeasurement = new V1_1::implementation::GnssMeasurement(gpsMeasurementIface);
        }
    }
    return mGnssMeasurement;
}

Return<sp<V1_0::IGnssNavigationMessage>> Gnss::getExtensionGnssNavigationMessage() {
    if (mGnssIface == nullptr) {
        ALOGE("%s: Gnss interface is unavailable", __func__);
        return nullptr;
    }

    if (mGnssNavigationMessage == nullptr) {
        const GpsNavigationMessageInterface* gpsNavigationMessageIface =
                static_cast<const GpsNavigationMessageInterface*>(
                        mGnssIface->get_extension(GPS_NAVIGATION_MESSAGE_INTERFACE));

        if (gpsNavigationMessageIface == nullptr) {
            ALOGI("%s: GnssNavigationMessage interface not implemented by HAL", __func__);
        } else {
            mGnssNavigationMessage = new V1_0::implementation::GnssNavigationMessage(gpsNavigationMessageIface);
        }
    }

    return mGnssNavigationMessage;
}

Return<sp<V1_0::IGnssXtra>> Gnss::getExtensionXtra()  {
    if (mGnssIface == nullptr) {
        ALOGE("%s: Gnss interface is unavailable", __func__);
        return nullptr;
    }

    if (mGnssXtraIface == nullptr) {
        const GpsXtraInterface* gpsXtraIface = static_cast<const GpsXtraInterface*>(
                mGnssIface->get_extension(GPS_XTRA_INTERFACE));

        if (gpsXtraIface == nullptr) {
            ALOGI("%s: GnssXtra interface not implemented by HAL", __func__);
        } else {
            mGnssXtraIface = new V1_0::implementation::GnssXtra(gpsXtraIface);
        }
    }

    return mGnssXtraIface;
}

Return<sp<V1_0::IGnssDebug>> Gnss::getExtensionGnssDebug()  {
    if (mGnssIface == nullptr) {
        ALOGE("%s: Gnss interface is unavailable", __func__);
        return nullptr;
    }

    if (mGnssDebug == nullptr) {
        const GpsDebugInterface_ext* gpsDebugIface = static_cast<const GpsDebugInterface_ext*>(
                mGnssIface->get_extension(GPS_DEBUG_INTERFACE));

        if (gpsDebugIface == nullptr) {
            ALOGI("%s: GnssDebug interface not implemented by HAL", __func__);
        } else {
            mGnssDebug = new V1_0::implementation::GnssDebug(gpsDebugIface);
        }
    }

    return mGnssDebug;
}

Return<sp<V1_0::IGnssBatching>> Gnss::getExtensionGnssBatching()  {
    if (mGnssIface == nullptr) {
        ALOGE("%s: Gnss interface is unavailable", __func__);
        return nullptr;
    }

    if (mGnssBatching == nullptr) {
        hw_module_t* module;
        const FlpLocationInterface* flpLocationIface = nullptr;
        int err = hw_get_module(FUSED_LOCATION_HARDWARE_MODULE_ID, (hw_module_t const**)&module);

        if (err != 0) {
            ALOGE("gnss flp hw_get_module failed: %d", err);
        } else if (module == nullptr) {
            ALOGE("Fused Location hw_get_module returned null module");
        } else if (module->methods == nullptr) {
            ALOGE("Fused Location hw_get_module returned null methods");
        } else {
            hw_device_t* device;
            err = module->methods->open(module, FUSED_LOCATION_HARDWARE_MODULE_ID, &device);
            if (err != 0) {
                ALOGE("flpDevice open failed: %d", err);
            } else {
                flp_device_t * flpDevice = reinterpret_cast<flp_device_t*>(device);
                flpLocationIface = flpDevice->get_flp_interface(flpDevice);
            }
        }

        if (flpLocationIface == nullptr) {
            ALOGE("%s: GnssBatching interface is not implemented by HAL", __func__);
        } else {
            mGnssBatching = new V1_0::implementation::GnssBatching(flpLocationIface);
        }
    }
    return mGnssBatching;
}

void Gnss::handleHidlDeath() {
    ALOGW("GNSS service noticed HIDL death. Stopping all GNSS operations.");

    /// M: move here! Do not callback to system_server to avoid gnss hidl service NE
    /*
     * This has died, so close it off in case (race condition) callbacks happen
     * before HAL processes above messages.
     */
    sem_wait(&sSem);
    sGnssCbIface = nullptr;
    sem_post(&sSem);

    // commands down to the HAL implementation
    stop(); // stop ongoing GPS tracking
    if (mGnssMeasurement != nullptr) {
        mGnssMeasurement->close();
    }
    if (mGnssNavigationMessage != nullptr) {
        mGnssNavigationMessage->close();
    }
    if (mGnssBatching != nullptr) {
        mGnssBatching->stop();
        mGnssBatching->cleanup();
    }
    cleanup();

}

IGnss* HIDL_FETCH_IGnss(const char* /* hal */) {
    hw_module_t* module;
    IGnss* iface = nullptr;
    int err = hw_get_module(GPS_HARDWARE_MODULE_ID, (hw_module_t const**)&module);

    if (err == 0) {
        hw_device_t* device;
        err = module->methods->open(module, GPS_HARDWARE_MODULE_ID, &device);
        if (err == 0) {
            iface = new Gnss(reinterpret_cast<gps_device_t_ext*>(device));
        } else {
            ALOGE("gnssDevice open %s failed: %d", GPS_HARDWARE_MODULE_ID, err);
        }
    } else {
      ALOGE("gnss hw_get_module %s failed: %d", GPS_HARDWARE_MODULE_ID, err);
    }
    return iface;
}

}  // namespace implementation
}  // namespace V1_1
}  // namespace gnss
}  // namespace hardware
}  // namespace android
