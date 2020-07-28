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
#include <sys/epoll.h>  // epoll_create, epoll_event
#include <errno.h>     // errno
#include <string.h>    // strerror
// #include <linux/in.h>   // INADDR_NONE
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
#define LOG_TAG "gpshal_worker"
#include <cutils/sockets.h>
#include <log/log.h>     /*logging in logcat*/
#define LOGD(fmt, arg ...) ALOGD("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGW(fmt, arg ...) ALOGW("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGE(fmt, arg ...) ALOGE("%s: " fmt, __FUNCTION__ , ##arg)
#endif
#endif
//=========================================================

#define GPSHAL_WORKER_THREAD_TIMEOUT (30*1000)

#define fieldp_copy(dst, src, f)  (dst)->f  = (src)->f
#define field_copy(dst, src, f)  (dst).f   = (src).f

//=========================================================
DebugData mnlDebugData; //for GNSS HIDL v1.1 GPS debug interface

static void update_mnld_reboot() {
// GPSHAL_DEBUG_FUNC_SCOPE2(g_gpshal_ctx.mutex_gps_state_intent);
    gpshal_state state = g_gpshal_ctx.gps_state_intent;
    LOGD("state: (%d)", state);

    hal2mnl_setNfwAccess(g_gpshal_ctx.proxy_apps, strlen(g_gpshal_ctx.proxy_apps));
    switch (state) {
        case GPSHAL_STATE_INIT:
        case GPSHAL_STATE_STOP:
            gpshal2mnl_gps_init();
            break;
        case GPSHAL_STATE_START:
            gpshal2mnl_gps_init();
            gpshal2mnl_gps_start();
            break;
        case GPSHAL_STATE_CLEANUP:
        default:
            LOGW("Current gps_state_intent: %s (%d)",
                gpshal_state_to_string(state), state);
    }
}
static void update_location(gps_location location) {
    // Use mutex or not ?
    //     If no, the issue may occur.
    //     If yes, there will be a little performance impact.
  //  GPSHAL_DEBUG_FUNC_SCOPE;
    GpsLocation_ext loc;
    //dump_gps_location(location);
    gpshal_state state = g_gpshal_ctx.gps_state;
    memset(&loc, 0, sizeof(GpsLocation_ext));
    //LOGD("state: (%d)", state);
    if (GPSHAL_STATE_START == state) {
        loc.legacyLocation.size      = sizeof(loc);
        loc.legacyLocation.flags     = location.flags;
        loc.legacyLocation.latitude  = location.lat;
        loc.legacyLocation.longitude = location.lng;
        loc.legacyLocation.altitude  = location.alt;
        loc.legacyLocation.speed     = location.speed;
        loc.legacyLocation.bearing   = location.bearing;
        loc.legacyLocation.accuracy  = loc.horizontalAccuracyMeters = location.h_accuracy;
        loc.verticalAccuracyMeters       = location.v_accuracy;
        loc.speedAccuracyMetersPerSecond = location.s_accuracy;
        loc.bearingAccuracyDegrees       = location.b_accuracy;
        loc.legacyLocation.timestamp = location.timestamp;

        /// not report elapsed real time in mnld, it will be reported in frw.
        loc.elapsedRealtime.flags = 0; //HAS_TIMESTAMP_NS | HAS_TIME_UNCERTAINTY_NS;
        loc.elapsedRealtime.timestampNs = 0;
        loc.elapsedRealtime.timeUncertaintyNs = 0;
        LOGD("hacc=%f,vacc=%f,sacc=%f,bacc=%f", loc.horizontalAccuracyMeters, loc.verticalAccuracyMeters,
            loc.speedAccuracyMetersPerSecond, loc.bearingAccuracyDegrees);
        g_gpshal_ctx.gps_cbs->location_cb(&loc);
        //For GNSS HIDL v1.1 Gps debug interface
        mnlDebugData.position.valid = true;
        mnlDebugData.position.latitudeDegrees   = location.lat;
        mnlDebugData.position.longitudeDegrees  = location.lng;
        mnlDebugData.position.altitudeMeters    = location.alt;
        mnlDebugData.position.speedMetersPerSec = location.speed;
        mnlDebugData.position.bearingDegrees    = location.bearing;
        mnlDebugData.position.horizontalAccuracyMeters     = location.h_accuracy;
        mnlDebugData.position.verticalAccuracyMeters       = location.v_accuracy;
        mnlDebugData.position.speedAccuracyMetersPerSecond = location.s_accuracy;
        mnlDebugData.position.bearingAccuracyDegrees       = location.b_accuracy;
        mnlDebugData.time.timeEstimate = location.timestamp;
    } else {
        // Do not report this location to GLP to avoid strange TTFF time issue
        LOGW("we have a location when gps_state_intent: %s (%d)",
                gpshal_state_to_string(state), state);
    }
}
static void update_gps_status(gps_status status) {
    GpsStatus s;
    memset(&s, 0, sizeof(GpsStatus));
    LOGD("  status=%d", status);
    s.size   = sizeof(s);
    s.status = status;
    g_gpshal_ctx.gps_cbs->status_cb(&s);
}

static void update_gps_sv(gnss_sv_info sv) {

   // GPSHAL_DEBUG_FUNC_SCOPE;
    int i;
    GnssSvStatus_ext gss;
    //dump_gnss_sv_info(sv);
    memset(&gss, 0, sizeof(GnssSvStatus_ext));
    gss.size = sizeof(gss);
    gss.num_svs = sv.num_svs;
    if (gss.num_svs > GNSS_MAX_SVS) {
        gss.num_svs = GNSS_MAX_SVS;
    }
    for (i = 0; i < gss.num_svs; i++) {
        GnssSvInfo* dst = &gss.gnss_sv_list[i].legacySvInfo;
        gnss_sv*    src = &sv.sv_list[i];
        dst->size = sizeof(*dst);
        fieldp_copy(dst, src, svid);
        fieldp_copy(dst, src, constellation);
        fieldp_copy(dst, src, c_n0_dbhz);
        fieldp_copy(dst, src, elevation);
        fieldp_copy(dst, src, azimuth);
        fieldp_copy(dst, src, flags);
        fieldp_copy(&gss.gnss_sv_list[i], src, carrier_frequency);
        mnlDebugData.satelliteDataArray[i].svid = gss.gnss_sv_list[i].legacySvInfo.svid;
        mnlDebugData.satelliteDataArray[i].constellation = gss.gnss_sv_list[i].legacySvInfo.constellation;
    }
    g_gpshal_ctx.gps_cbs->gnss_sv_status_cb(&gss);
}

static void update_nmea(int64_t timestamp, const char* nmea, int length) {
    /*LOGD("  timestamp=%lld nmea=[%s] length=%d",
        timestamp, nmea, length);*/
    g_gpshal_ctx.gps_cbs->nmea_cb(timestamp, nmea, length);
}
static void update_gps_capabilities(gps_capabilites capabilities) {
    LOGD("  capabilities=0x%x", capabilities);
//    g_gpshal_ctx.gps_cbs->set_capabilities_cb(capabilities);
    g_gpshal_ctx.gps_cap = capabilities;
}
static void update_gps_measurements(gps_data data) {
   // GPSHAL_DEBUG_FUNC_SCOPE;
    size_t i;
    GpsData gd;
    //dump_gps_data(data);
    memset(&gd, 0, sizeof(GpsData));
    gd.size = sizeof(gd);
    field_copy(gd, data, measurement_count);
    for (i = 0; i < GPS_MAX_MEASUREMENT; i++) {
        GpsMeasurement*  dst = &gd.measurements[i];
        gps_measurement* src = &data.measurements[i];
        dst->size  = sizeof(*dst);
        fieldp_copy(dst, src, flags);
        fieldp_copy(dst, src, prn);
        fieldp_copy(dst, src, time_offset_ns);
        fieldp_copy(dst, src, state);
        fieldp_copy(dst, src, received_gps_tow_ns);
        fieldp_copy(dst, src, received_gps_tow_uncertainty_ns);
        fieldp_copy(dst, src, c_n0_dbhz);
        fieldp_copy(dst, src, pseudorange_rate_mps);
        fieldp_copy(dst, src, pseudorange_rate_uncertainty_mps);
        fieldp_copy(dst, src, accumulated_delta_range_state);
        fieldp_copy(dst, src, accumulated_delta_range_m);
        fieldp_copy(dst, src, accumulated_delta_range_uncertainty_m);
        fieldp_copy(dst, src, pseudorange_m);
        fieldp_copy(dst, src, pseudorange_uncertainty_m);
        fieldp_copy(dst, src, code_phase_chips);
        fieldp_copy(dst, src, code_phase_uncertainty_chips);
        fieldp_copy(dst, src, carrier_frequency_hz);
        fieldp_copy(dst, src, carrier_cycles);
        fieldp_copy(dst, src, carrier_phase);
        fieldp_copy(dst, src, carrier_phase_uncertainty);
        fieldp_copy(dst, src, loss_of_lock);
        fieldp_copy(dst, src, bit_number);
        fieldp_copy(dst, src, time_from_last_bit_ms);
        fieldp_copy(dst, src, doppler_shift_hz);
        fieldp_copy(dst, src, doppler_shift_uncertainty_hz);
        fieldp_copy(dst, src, multipath_indicator);
        fieldp_copy(dst, src, snr_db);
        fieldp_copy(dst, src, elevation_deg);
        fieldp_copy(dst, src, elevation_uncertainty_deg);
        fieldp_copy(dst, src, azimuth_deg);
        fieldp_copy(dst, src, azimuth_uncertainty_deg);
        fieldp_copy(dst, src, used_in_fix);
    }

    // To do things like field_copy(gd, data, clock)
    {
        GpsClock*  dst = &gd.clock;
        gps_clock* src = &data.clock;
        dst->size = sizeof(*dst);
        fieldp_copy(dst, src, flags);
        fieldp_copy(dst, src, leap_second);
        fieldp_copy(dst, src, type);
        fieldp_copy(dst, src, time_ns);
        fieldp_copy(dst, src, time_uncertainty_ns);
        fieldp_copy(dst, src, full_bias_ns);
        fieldp_copy(dst, src, bias_ns);
        fieldp_copy(dst, src, bias_uncertainty_ns);
        fieldp_copy(dst, src, drift_nsps);
        fieldp_copy(dst, src, drift_uncertainty_nsps);
        mnlDebugData.time.timeUncertaintyNs = src->time_uncertainty_ns;
        mnlDebugData.time.frequencyUncertaintyNsPerSec = src->drift_nsps;
    }
    g_gpshal_ctx.meas_cbs->measurement_callback(&gd);
}
static void update_gps_navigation(gps_nav_msg msg) {
  //  GPSHAL_DEBUG_FUNC_SCOPE;
    GpsNavigationMessage gnm;
    //dump_gps_nav_msg(msg);
    memset(&gnm, 0, sizeof(GpsNavigationMessage));
    gnm.size = sizeof(gnm);
    field_copy(gnm, msg, prn);
    field_copy(gnm, msg, type);
    field_copy(gnm, msg, status);
    field_copy(gnm, msg, message_id);
    field_copy(gnm, msg, submessage_id);
    field_copy(gnm, msg, data_length);
    gnm.data = (uint8_t*) msg.data;
    g_gpshal_ctx.navimsg_cbs->navigation_message_callback(&gnm);
}

static void update_gnss_measurements(gnss_data data) {
    size_t i;
    GnssData_ext gd;
    //dump_gnss_data(data);
    memset(&gd, 0, sizeof(GnssData_ext));
    gd.size = sizeof(gd);
    field_copy(gd, data, measurement_count);
    for (i = 0; i < MTK_HAL_GNSS_MAX_MEASUREMENT; i++) {
        GnssMeasurement*  dst = &gd.measurements[i].legacyMeasurement;
        gnss_measurement*  src = &data.measurements[i];
        dst->size  = sizeof(*dst);
        fieldp_copy(dst, src, flags);
        fieldp_copy(dst, src, svid);
        fieldp_copy(dst, src, constellation);
        fieldp_copy(dst, src, time_offset_ns);
        fieldp_copy(dst, src, state);
        fieldp_copy(dst, src, received_sv_time_in_ns);
        fieldp_copy(dst, src, received_sv_time_uncertainty_in_ns);
        fieldp_copy(dst, src, c_n0_dbhz);
        fieldp_copy(dst, src, pseudorange_rate_mps);
        fieldp_copy(dst, src, pseudorange_rate_uncertainty_mps);
        fieldp_copy(dst, src, accumulated_delta_range_state);
        fieldp_copy(dst, src, accumulated_delta_range_m);
        fieldp_copy(dst, src, accumulated_delta_range_uncertainty_m);
        fieldp_copy(dst, src, carrier_frequency_hz);
        fieldp_copy(dst, src, carrier_cycles);
        fieldp_copy(dst, src, carrier_phase);
        fieldp_copy(dst, src, carrier_phase_uncertainty);
        fieldp_copy(dst, src, multipath_indicator);
        fieldp_copy(dst, src, snr_db);
        fieldp_copy(&gd.measurements[i], src, agc_level_db);
        /// todo
        MNLD_STRNCPY(gd.measurements[i].codeType, data.measurements[i].codeType, sizeof(gd.measurements[i].codeType));
    }

    // To do things like field_copy(gd, data, clock)
    {
        GnssClock*  dst = &gd.clock;
        gnss_clock* src = &data.clock;
        dst->size = sizeof(*dst);
        fieldp_copy(dst, src, flags);
        fieldp_copy(dst, src, leap_second);
        fieldp_copy(dst, src, time_ns);
        fieldp_copy(dst, src, time_uncertainty_ns);
        fieldp_copy(dst, src, full_bias_ns);
        fieldp_copy(dst, src, bias_ns);
        fieldp_copy(dst, src, bias_uncertainty_ns);
        fieldp_copy(dst, src, drift_nsps);
        fieldp_copy(dst, src, drift_uncertainty_nsps);
        fieldp_copy(dst, src, hw_clock_discontinuity_count);
        mnlDebugData.time.timeUncertaintyNs = src->time_uncertainty_ns;
        mnlDebugData.time.frequencyUncertaintyNsPerSec = src->drift_nsps;
        /// not report elapsed real time in mnld, real time items will be ignored.
        gd.elapsedRealtime.flags = 0; //HAS_TIMESTAMP_NS | HAS_TIME_UNCERTAINTY_NS;
        gd.elapsedRealtime.timestampNs = 0;
        gd.elapsedRealtime.timeUncertaintyNs = 0;
    }

    g_gpshal_ctx.meas_cbs->gnss_measurement_callback(&gd);
}

static void update_gnss_navigation(gnss_nav_msg msg) {
    GnssNavigationMessage gnm;
    //dump_gnss_nav_msg(msg);
    memset(&gnm, 0, sizeof(GnssNavigationMessage));
    gnm.size = sizeof(gnm);
    field_copy(gnm, msg, svid);
    field_copy(gnm, msg, type);
    field_copy(gnm, msg, status);
    field_copy(gnm, msg, message_id);
    field_copy(gnm, msg, submessage_id);
    field_copy(gnm, msg, data_length);
    gnm.data = (uint8_t*) msg.data;
    g_gpshal_ctx.navimsg_cbs->gnss_navigation_message_callback(&gnm);
}

static void request_wakelock() {
   // GPSHAL_DEBUG_FUNC_SCOPE;
    g_gpshal_ctx.gps_cbs->acquire_wakelock_cb();
}
static void release_wakelock() {
   // GPSHAL_DEBUG_FUNC_SCOPE;
    g_gpshal_ctx.gps_cbs->release_wakelock_cb();
}
static void request_utc_time() {
   // GPSHAL_DEBUG_FUNC_SCOPE;
    g_gpshal_ctx.gps_cbs->request_utc_time_cb();
}
static void request_data_conn(__unused struct sockaddr_storage* addr, agps_type type) {
  //  GPSHAL_DEBUG_FUNC_SCOPE;
    UNUSED(addr);
    AGpsStatus as;
    as.size   = sizeof(AGpsStatus);  // our imp supports v3
    as.type   = type;
    as.status = GPS_REQUEST_AGPS_DATA_CONN;
    // as.ipaddr = INADDR_NONE; // not used in v3
    as.addr   = *addr;
    g_gpshal_ctx.agps_cbs->status_cb(&as);
}
static void release_data_conn(agps_type type) {
    // GPSHAL_DEBUG_FUNC_SCOPE;
    AGpsStatus as;
    as.size   = sizeof(AGpsStatus_v1);  // use v1 size to omit optional fields
    as.type   = type;
    as.status = GPS_RELEASE_AGPS_DATA_CONN;
    g_gpshal_ctx.agps_cbs->status_cb(&as);
}
static void request_ni_notify(int session_id, agps_ni_type ni_type, agps_notify_type type, const char* requestor_id,
        const char* client_name, ni_encoding_type requestor_id_encoding,
        ni_encoding_type client_name_encoding) {
    /*LOGD("  session_id=%d type=%d requestor_id=[%s] client_name=[%s] requestor_id_encoding=%d client_name_encoding=%d",
        session_id, type, requestor_id, client_name, requestor_id_encoding, client_name_encoding);*/
    GpsNiNotification gnn;
    gnn.size = sizeof(gnn);
    gnn.notification_id = session_id;
    gnn.ni_type = ni_type;
    switch (type) {
        case AGPS_NOTIFY_TYPE_NOTIFY_ONLY:
            gnn.notify_flags = GPS_NI_NEED_NOTIFY;
            break;
        case AGPS_NOTIFY_TYPE_NOTIFY_ALLOW_NO_ANSWER:
            gnn.notify_flags = GPS_NI_NEED_NOTIFY|GPS_NI_NEED_VERIFY;
            break;
        case AGPS_NOTIFY_TYPE_NOTIFY_DENY_NO_ANSWER:
            gnn.notify_flags = GPS_NI_NEED_NOTIFY|GPS_NI_NEED_VERIFY;
            break;
        case AGPS_NOTIFY_TYPE_PRIVACY:
            gnn.notify_flags = GPS_NI_PRIVACY_OVERRIDE;
            break;
        default:
            gnn.notify_flags = 0;
    }
    gnn.timeout          = 8;
    gnn.default_response = GPS_NI_RESPONSE_NORESP;
    MNLD_STRNCPY(gnn.requestor_id, requestor_id,sizeof(gnn.requestor_id));
    MNLD_STRNCPY(gnn.text,client_name,sizeof(gnn.text));
    gnn.requestor_id_encoding = requestor_id_encoding;
    gnn.text_encoding         = client_name_encoding;
    gnn.extras[0] = 0;  // be an empty string ""
    if (g_gpshal_ctx.gpsni_cbs) {
        g_gpshal_ctx.gpsni_cbs->notify_cb(&gnn);
    }
}
static void request_set_id(request_setid flags) {
    //LOGD("  flags=0x%x", flags);
    if (g_gpshal_ctx.agpsril_cbs) {
        g_gpshal_ctx.agpsril_cbs->request_setid(flags);
    }
}
static void request_ref_loc(request_refloc flags) {
    //LOGD("  flags=0x%x", flags);
    if (g_gpshal_ctx.agpsril_cbs) {
        g_gpshal_ctx.agpsril_cbs->request_refloc(flags);
    }
}
static void output_vzw_debug(const char* str) {
    int len;
    len = strlen(str)+1;
    if (len > VZW_DEBUG_STRING_MAXLEN) {
        len = VZW_DEBUG_STRING_MAXLEN;
    }
    LOGD("len=%d str=%s", len, str);
    VzwDebugData vzwdebugdata;
    vzwdebugdata.size = sizeof(VzwDebugData);
    memcpy(vzwdebugdata.vzw_msg_data, str, len);
    vzwdebugdata.vzw_msg_data[VZW_DEBUG_STRING_MAXLEN - 1] = 0;

    if (g_gpshal_ctx.vzw_debug_cbs) {
        g_gpshal_ctx.vzw_debug_cbs->vzw_debug_cb(&vzwdebugdata);
    }
}

static void update_gnss_name(const char* name, int length) {
    LOGD("length=%d, name=%s", length, name);
    if(length > GNSS_NAME_LEN) {
        length = GNSS_NAME_LEN;
    }
    MNLD_STRNCPY(g_gpshal_ctx.gps_name, name, GNSS_NAME_LEN);
}

static void request_nlp(bool independentFromGnss, bool isUserEmergency) {
    LOGD("request nlp[%d], flag=%d", independentFromGnss, isUserEmergency);
    if (g_gpshal_ctx.gps_cbs) {
        //// todo
        g_gpshal_ctx.gps_cbs->request_location_cb(independentFromGnss, isUserEmergency);
    }
}

static void nfw_access_notify(NfwNotification nfw_notify) {
    LOGD("APP name: %s, PS: %d, otherPSName: %s, requestor: %d, requestorId: %s, responseType: %d, inEmergencyMode: %d, isCachedLocation: %d"
        ,nfw_notify.proxyAppPackageName
        ,nfw_notify.protocolStack
        ,nfw_notify.otherProtocolStackName
        ,nfw_notify.requestor
        ,nfw_notify.requestorId
        ,nfw_notify.responseType
        ,nfw_notify.inEmergencyMode
        ,nfw_notify.isCachedLocation
    );
    if(g_gpshal_ctx.visibility_control_cbs && g_gpshal_ctx.visibility_control_cbs->nfw_notify_cb) {
        g_gpshal_ctx.visibility_control_cbs->nfw_notify_cb(nfw_notify);
    }
}

static mnl2hal_interface gpshal_mnl2hal_interface = {
    update_mnld_reboot,
    update_location,
    update_gps_status,
    update_gps_sv,
    update_nmea,
    update_gps_capabilities,
    update_gps_measurements,
    update_gps_navigation,
    update_gnss_measurements,
    update_gnss_navigation,
    request_wakelock,
    release_wakelock,
    request_utc_time,
    request_data_conn,
    release_data_conn,
    request_ni_notify,
    request_set_id,
    request_ref_loc,
    output_vzw_debug,
    update_gnss_name,
    request_nlp,
    nfw_access_notify,
};

//=========================================================

void gpshal_worker_thread(__unused void *arg) {
   // GPSHAL_DEBUG_FUNC_SCOPE;
    struct epoll_event events[MAX_EPOLL_EVENT];
    UNUSED(arg);

    gps_hal_cmd_list_init();

    while (true) {
        int i;
        int n;

        n = epoll_wait(g_gpshal_ctx.fd_worker_epoll, events, MAX_EPOLL_EVENT , -1);
        if (n == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                LOGE("%s() epoll_wait failure reason=[%s]%d",
                        __func__, strerror(errno), errno);
                return;
            }
        }

        for (i = 0; i < n; i++) {
            if (events[i].data.fd == g_gpshal_ctx.fd_mnl2hal) {
                if (events[i].events & EPOLLIN) {
                    mnl2hal_hdlr(g_gpshal_ctx.fd_mnl2hal,
                            &gpshal_mnl2hal_interface);
                }
            } else {
                LOGE("%s() unknown fd=%d",
                        __func__, events[i].data.fd);
            }
        }
    }  // of while (true)
}
