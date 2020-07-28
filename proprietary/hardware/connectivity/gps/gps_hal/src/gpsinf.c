/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
#include "gpshal.h"
#include "gpshal_param_check.h"
#include "hal2mnl_interface.h"
#include "mtk_lbs_utility.h"
#if 0
#ifdef LOGD
#undef LOGD
#endif
#ifdef LOGW
#undef LOGW
#endif
#ifdef LOGE
#undef LOGE
#endif
#if 0
#define LOGD(...) tag_log(1, "[gpshal]", __VA_ARGS__);
#define LOGW(...) tag_log(1, "[gpshal] WARNING: ", __VA_ARGS__);
#define LOGE(...) tag_log(1, "[gpshal] ERR: ", __VA_ARGS__);
#else
#define LOG_TAG "gpsinf"
#include <cutils/sockets.h>
#include <log/log.h>     /*logging in logcat*/
#define LOGD(fmt, arg ...) ALOGD("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGW(fmt, arg ...) ALOGW("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGE(fmt, arg ...) ALOGE("%s: " fmt, __FUNCTION__ , ##arg)
#endif
#endif
//=========================================================

extern struct hw_module_t HAL_MODULE_INFO_SYM;
extern DebugData mnlDebugData; //for GNSS HIDL v1.1 GPS debug interface

//=========================================================
// Gps Measurement Interface

// Thread: BackgroundThread
//      GpsMeasurementsProvider uses GpsLocationProvider's handler
static int measinf_init(GpsMeasurementCallbacks_ext* callbacks, bool enableFullTracking) {
    g_gpshal_ctx.meas_cbs = callbacks;
    int ret = hal2mnl_set_gps_measurement(true, enableFullTracking);
    return (ret > 0)?
            GPS_GEOFENCE_OPERATION_SUCCESS :
            GPS_GEOFENCE_ERROR_GENERIC;
}

// Thread: BackgroundThread
//      GpsMeasurementsProvider uses GpsLocationProvider's handler
static void measinf_close(void) {
    hal2mnl_set_gps_measurement(false, false);
}

const GpsMeasurementInterface_ext mtk_gps_meas_inf = {
    sizeof(GpsMeasurementInterface_ext),
    measinf_init,
    measinf_close
};

//=========================================================
// Gnss Measurement Correction Interface
static bool meas_correct_set_callback(MeasurementCorrectionCallbacks_ext* callbacks) {
    int capabilities = LOS_SATS | EXCESS_PATH_LENGTH | REFLECTING_PLANE;
    g_gpshal_ctx.meas_correct_cbs = callbacks;
    g_gpshal_ctx.meas_correct_cbs->set_capabilities_cb(capabilities);
    return true;
}

static bool meas_correct_set_correction(MeasurementCorrections* corrections) {
    LOGD("setCorrections");
    /*LOGD("corrections = lat: %f, lng: %f, alt: %f, hUnc: %f, vUnc: %f, toa: %llu num_satCorr: %d, ",
          corrections->latitudeDegrees, corrections->longitudeDegrees, corrections->altitudeMeters,
          corrections->horizontalPositionUncertaintyMeters,
          corrections->verticalPositionUncertaintyMeters,
          corrections->toaGpsNanosecondsOfWeek,
          corrections->num_satCorrection);
    int i = 0;*/
    int ret = 0;
    if(corrections->num_satCorrection > GNSS_MAX_SVS) {
        corrections->num_satCorrection = GNSS_MAX_SVS;
    }
    /*for (i=0; i < corrections->num_satCorrection; i++) {
        SingleSatCorrection* ssc = &(corrections->satCorrections[i]);
        LOGD("i = %d, singleSatCorrection = flags: %d, constellation: %d, svid: %d, cfHz: %f, probLos: %f,"
              " epl: %f, eplUnc: %f",
              i,
              ssc->singleSatCorrectionFlags,
              ssc->constellation,
              ssc->svid, ssc->carrierFrequencyHz,
              ssc->probSatIsLos, ssc->excessPathLengthMeters,
              ssc->excessPathLengthUncertaintyMeters);
        LOGD("reflecting plane = lat: %f, lng: %f, alt: %f, azm: %f",
              ssc->reflectingPlane.latitudeDegrees,
              ssc->reflectingPlane.longitudeDegrees,
              ssc->reflectingPlane.altitudeMeters,
              ssc->reflectingPlane.azimuthDegrees);
    }*/
    ret = hal2mnl_set_correction(corrections);
    return (ret > 0) ? true:false;
}

const MeasurementCorrectionInterface mtk_gnss_meas_correction_inf = {
    sizeof(MeasurementCorrectionInterface),
    meas_correct_set_callback,
    meas_correct_set_correction
};

//=========================================================
// Gnss Visibility Control Interface
static bool visibility_set_callback(GnssVisibilityControlCallback_ext* callbacks) {
    g_gpshal_ctx.visibility_control_cbs = callbacks;
    return true;
}

static bool enable_nfw_access(const char* proxyApps, int32_t length) {
    LOGD("enable_nfw_access proxyApps: [%s], len=%d", proxyApps, length);
    if(length > HAL_NFW_USER_NUM_MAX*HAL_NFW_USER_NAME_LEN) {
        LOGW("Over length(%d), set to MAX:%d", length, HAL_NFW_USER_NUM_MAX*HAL_NFW_USER_NAME_LEN-1);
        length = HAL_NFW_USER_NUM_MAX*HAL_NFW_USER_NAME_LEN-1;
    }
    memset(g_gpshal_ctx.proxy_apps, 0, sizeof(g_gpshal_ctx.proxy_apps));  //Clear buffer before copy
    strncpy(g_gpshal_ctx.proxy_apps, proxyApps, length);
    hal2mnl_setNfwAccess(g_gpshal_ctx.proxy_apps, length);
    return true;
}

const GnssVisibilityControlInterface mtk_visibility_control_inf = {
    sizeof(GnssVisibilityControlInterface),
    visibility_set_callback,
    enable_nfw_access
};

//=========================================================
// Gps Navigation Message Interface

// Thread: BackgroundThread
// GpsNavigationMessageProvider uses GpsLocationProvider's handler
static int navimsginf_init(GpsNavigationMessageCallbacks* callbacks) {
    g_gpshal_ctx.navimsg_cbs = callbacks;
    int ret = hal2mnl_set_gps_navigation(true);
    return (ret > 0)?
            GPS_NAVIGATION_MESSAGE_OPERATION_SUCCESS :
            GPS_NAVIGATION_MESSAGE_ERROR_GENERIC;
}

// Thread: BackgroundThread
//      GpsNavigationMessageProvider uses GpsLocationProvider's handler
static void navimsginf_close(void) {
    hal2mnl_set_gps_navigation(false);
}

const GpsNavigationMessageInterface mtk_navi_msg_inf = {
    sizeof(GpsNavigationMessageInterface),
    navimsginf_init,
    navimsginf_close
};

static int vzwdebuginf_init(VzwDebugCallbacks* callbacks) {
    g_gpshal_ctx.vzw_debug_cbs = callbacks;
    return 0;
}

static void vzwdebuginf_set(bool enabled) {
    hal2mnl_set_vzw_debug(enabled);
}

const VzwDebugInterface mtk_vzw_debug_inf = {
    sizeof(VzwDebugInterface),
    vzwdebuginf_init,
    vzwdebuginf_set
};

static void gnss_configuration_update(const char* config_data, int32_t length) {
    hal2mnl_update_gnss_config(config_data, length);
}

static void gnss_setBlacklist(long long* blacklist, int32_t size) {
    hal2mnl_setBlacklist(blacklist, size);
}

static void gnss_setEsExtensionSec(uint32_t emergencyExtensionSeconds) {
    char temp_string[64];
    LOGD("Set Emergency State Extension Second:%d", emergencyExtensionSeconds);
    memset(temp_string, 0, sizeof(temp_string));
    sprintf(temp_string, "emergencyExtensionSeconds=%d", emergencyExtensionSeconds);
    hal2mnl_update_gnss_config(temp_string, strlen(temp_string));
}

const GnssConfigurationInterface_ext mtk_gnss_config_inf = {
    sizeof(GnssConfigurationInterface_ext),
    gnss_configuration_update,
    gnss_setBlacklist,
    gnss_setEsExtensionSec
};

static bool get_internal_state(DebugData* debugData) {
    memcpy(debugData, &mnlDebugData, sizeof(DebugData));
    LOGD("debugData->time:Esttimems:%lld, uncNs: %.3f, ppb: %.3f",
        debugData->time.timeEstimate,
        debugData->time.timeUncertaintyNs,
        debugData->time.frequencyUncertaintyNsPerSec);
    return true;
}

const GpsDebugInterface_ext mtk_gps_debug_inf = {
    sizeof(GpsDebugInterface_ext),
    get_internal_state
};

//=========================================================
// Implementation of
//     Gps Iterface

// Thread: Main thread of system server
// Call seq:
//     LocationManagerService.systemRunning
//     LocationManagerService.loadProvidersLocked
//     GpsLocationProvider.static
//     android_location_GpsLocationProvider_class_init_native
//     sGpsInterface->get_extension
static const void* gpsinf_get_extension(const char* name) {
    if (strcmp(name, AGPS_INTERFACE) == 0) {
        return &mtk_agps_inf;
    }
    if (strcmp(name, GPS_NI_INTERFACE) == 0) {
        return &mtk_gps_ni_inf;
    }
    if (strcmp(name, AGPS_RIL_INTERFACE) == 0) {
        return &mtk_agps_ril_inf;
    }
    if (strcmp(name, GPS_MEASUREMENT_INTERFACE) == 0) {
       return &mtk_gps_meas_inf;
    }
    if (strcmp(name, GNSS_MEASUREMENT_CORRECTION_INTERFACE) == 0) {
       return &mtk_gnss_meas_correction_inf;
    }
    if (strcmp(name, GNSS_VISIBILITY_CONTROL_INTERFACE) == 0) {
       return &mtk_visibility_control_inf;
    }
    if (strcmp(name, GPS_NAVIGATION_MESSAGE_INTERFACE) == 0) {
       return &mtk_navi_msg_inf;
    }

    if (strcmp(name, VZW_DEBUG_INTERFACE) == 0) {
       return &mtk_vzw_debug_inf;
    }
#if 0
    if (strcmp(name, GPS_GEOFENCING_INTERFACE) == 0) {
       return &mtkGeofence_inf;
    }
#endif

#if 0  // not supported, even in our MP branches
    if (!strcmp(name, GPS_GEOFENCING_INTERFACE))
        return &mtk_gps_geofencing_inf;
#endif
    if (!strcmp(name, GNSS_CONFIGURATION_INTERFACE)) {
        return &mtk_gnss_config_inf;
    }
#if 0  // not ready, even in Google's GPS JNI
    if (strcmp(name, SUPL_CERTIFICATE_INTERFACE) == 0) {
       return &mtk_supl_cert_inf;
    }
#endif
#if 0  // Similar to our EPO but we will do it in mnld.
    if (!strcmp(name, GPS_XTRA_INTERFACE))
        return &mtk_gps_xtra_inf;
#endif
    if (!strcmp(name, GPS_DEBUG_INTERFACE)) {
        return &mtk_gps_debug_inf;
    }
    return NULL;  // unsupported extension
}

// Thread: BackgroundThread
//     Its looper is shared by a lot of services including
//         LocationManagerService's handler
//         GpsLocationProvider's handler
//         GpsLocationProvider's broadcast receiver
// Call seq:
//     GpsLocationProvider.ProviderHandler.handleMessage
//     GpsLocationProvider.handleEnable
//     GpsLocationProvider.native_init
//     android_location_GpsLocationProvider_init
//     sGpsInterface->init
static int gpsinf_init(GpsCallbacks_ext* callbacks) {
    // GPSHAL_DEBUG_FUNC_SCOPE2(g_gpshal_ctx.mutex_gps_state_intent);
    if (gpshal_gpscbs_save(callbacks) != 0) {
        return -1;    //  error
    }

    gpshal_set_gps_state_intent(GPSHAL_STATE_INIT);
    gpshal2mnl_gps_init();
    return 0;  // OK to init
}

// Thread: BackgroundThread
static int gpsinf_start(void) {
    // GPSHAL_DEBUG_FUNC_SCOPE2(g_gpshal_ctx.mutex_gps_state_intent);
    memset(&mnlDebugData, 0, sizeof(DebugData));//clear debug data every session
    gpshal_set_gps_state_intent(GPSHAL_STATE_START);
    gpshal2mnl_gps_start();
    return 0;  // OK to start
}

// Thread: BackgroundThread
static int gpsinf_stop(void) {
    // GPSHAL_DEBUG_FUNC_SCOPE2(g_gpshal_ctx.mutex_gps_state_intent);
    gpshal_set_gps_state_intent(GPSHAL_STATE_STOP);
    gpshal2mnl_gps_stop();
    return 0;  // OK to stop
}

// Thread: BackgroundThread
static void gpsinf_cleanup(void) {
    // GPSHAL_DEBUG_FUNC_SCOPE2(g_gpshal_ctx.mutex_gps_state_intent);
    gpshal_set_gps_state_intent(GPSHAL_STATE_CLEANUP);
    gpshal2mnl_gps_cleanup();
}

// Thread: BackgroundThread
static int gpsinf_inject_time(GpsUtcTime time, int64_t timeReference, int uncertainty) {
    hal2mnl_gps_inject_time(time, timeReference, uncertainty);
    return 0;  // 0:ok,   non-zero: error; but GPS JNI will ignore it
}

// Thread: BackgroundThread
//     NetworkLocationListener uses the looper of GpsLocationProvider's handler
static int gpsinf_inject_location(
        double latitude,
        double longitude,
        float  accuracy) {
    hal2mnl_gps_inject_location(latitude, longitude, accuracy);
    return 0;  // 0:ok,   non-zero: error; but GPS JNI will ignore it
}

// Thread: Binder Thread of LocationManagerService
//
// Call seq:
//     LocationManagerService.sendExtraCommand
//     GpsLocationProvider.sendExtraCommand
//     GpsLocationProvider.deleteAidingData
//     GpsLocationProvider.native_delete_aiding_data
//     android_location_GpsLocationProvider_delete_aiding_data
//     sGpsInterface->delete_aiding_data
static void gpsinf_delete_aiding_data(GpsAidingData flags) {
    hal2mnl_gps_delete_aiding_data(flags);
}

// Thread: BackgroundThread or ???
//     GpsLocationProvider's handler may call it
//     GpsLocationProvider's mBroadcastReceiver may call it (PowerManager's intent)
static int gpsinf_set_position_mode(
        GpsPositionMode mode,
        GpsPositionRecurrence recurrence,
        uint32_t min_interval,
        uint32_t preferred_accuracy,
        uint32_t preferred_time,
        bool lowPowerMode) {
    hal2mnl_gps_set_position_mode(
            mode,
            recurrence,
            min_interval,
            preferred_accuracy,
            preferred_time,
            lowPowerMode);
    return 0;  // 0:ok,   non-zero: error; GLP will show log if error
}

static int gpsinf_inject_fused_location(
        double latitude,
        double longitude,
        float  accuracy) {
    // TODO:  hal2mnl_gps_inject_fused_location(latitude, longitude, accuracy);
    UNUSED(latitude);
    UNUSED(longitude);
    UNUSED(accuracy);
    return 0;  // 0:ok,   non-zero: error; but GPS JNI will ignore it
}

static const GpsInterface_ext  mtk_gps_inf = {
    sizeof(GpsInterface_ext),
    gpsinf_init,
    gpsinf_start,
    gpsinf_stop,
    gpsinf_cleanup,
    gpsinf_inject_time,
    gpsinf_inject_location,
    gpsinf_delete_aiding_data,
    gpsinf_set_position_mode,
    gpsinf_get_extension,
    gpsinf_inject_fused_location,
};


//=========================================================
// Between
//     Gps Interface
//     Hardware Module Interface

static const GpsInterface_ext* gps_device__get_gps_interface(
        __unused struct gps_device_t_ext* device) {
    GPS_DEVICE__GET_GPS_INTERFACE__CHECK_PARAM;
    hal2mnl_hal_reboot();
    return &mtk_gps_inf;
}

static const struct gps_device_t_ext gps_device = {
    .common = {                           // hw_device_t
        .tag     = HARDWARE_DEVICE_TAG,
        .version = 0,                     // GPS JNI will not use it
        .module  = &HAL_MODULE_INFO_SYM,
        .reserved = {0},
        .close   = NULL                   // GPS JNI will not call it
        },
    .get_gps_interface = gps_device__get_gps_interface
};

//=========================================================
// Implementation of
//     Hardware Module Interface

static int gps_hw_module_open(
        __unused const struct hw_module_t* module,
        __unused char const* id,
        struct hw_device_t** device) {
    GPS_HW_MODULE_OPEN__CHECK_PARAM;
    *device = (struct hw_device_t*)&gps_device;
    return 0;  // OK: refer to the cpp of GPS JNI
}

static struct hw_module_methods_t gps_hw_module_methods = {
    .open = gps_hw_module_open
};

struct hw_module_t HAL_MODULE_INFO_SYM = {
    .tag = HARDWARE_MODULE_TAG,
    .version_major = 1,
    .version_minor = 0,
    .id  = GPS_HARDWARE_MODULE_ID,
    .name   = "MediaTek GPS Hardware Module",
    .author = "MediaTek, Inc.",
    .methods = &gps_hw_module_methods,
    .dso     = NULL,
    .reserved = {0}
};

