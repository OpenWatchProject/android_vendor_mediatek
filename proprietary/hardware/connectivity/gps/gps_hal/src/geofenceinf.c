#include <cutils/sockets.h>
#include <log/log.h>     /*logging in logcat*/
#include <inttypes.h>
#include "gpshal.h"
#include "geofencehal_worker.h"

#if 1
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
#define LOGD(...) tag_log(1, "[gps_dbg_log]", __VA_ARGS__);
#define LOGW(...) tag_log(1, "[gps_dbg_log] WARNING: ", __VA_ARGS__);
#define LOGE(...) tag_log(1, "[gps_dbg_log] ERR: ", __VA_ARGS__);
#define  TRC(f)       ((void)0)
#else
#define LOGD(fmt, arg ...) ALOGD("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGW(fmt, arg ...) ALOGW("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGE(fmt, arg ...) ALOGE("%s: " fmt, __FUNCTION__ , ##arg)
#define  TRC(f)       ALOGD("%s", __func__)
#endif
#endif

#ifndef UNUSED
#define UNUSED(x) (x)=(x)
#endif

/**********************************************************
 *  Global Variables                                      *
 **********************************************************/
extern unsigned int gConfigHalGeofence;

/*********************************************************/
/* FLP Geofence Interface implementation                 */
/*********************************************************/
void mtk_hal_geofence_init(GpsGeofenceCallbacks* callbacks) {
    hal2_geofence_init();
    LOGD("init geofence");
    g_gpshal_ctx.geofence_cbs= callbacks;
    return;
}

void mtk_hal_geofence_add_geofences(int32_t geofence_id, double latitude, double longitude,
    double radius_meters, int last_transition, int monitor_transitions,
    int notification_responsiveness_ms, int unknown_timer_ms) {
    MTK_GEOFENCE_PROPERTY_T dbg_fence;

    if(!is_gfc_exist()) {
        LOGE("gfc not exist");
        return;
    }
    dbg_fence.geofence_id = geofence_id;
    dbg_fence.latitude = latitude;
    dbg_fence.longitude = longitude;
    dbg_fence.radius = radius_meters;
    dbg_fence.last_transition = last_transition;
    dbg_fence.monitor_transition = monitor_transitions;
    dbg_fence.notification_period = notification_responsiveness_ms;
    dbg_fence.unknown_timer = unknown_timer_ms;
    dbg_fence.alive = 1;
    dbg_fence.latest_state = last_transition >> 1;
    dbg_fence.config = gConfigHalGeofence;
    mtk_geo_inject_info_get(dbg_fence.geofence_id, &dbg_fence.coordinate_dn, &dbg_fence.coordinate_de);
    hal2_geofence_add_geofences(dbg_fence);
    return;
}

void mtk_hal_geofence_pause_geofence(int32_t geofence_id) {
    if(!is_gfc_exist()) {
        LOGE("gfc not exist");
        return;
    }
    hal2_geofence_pause_geofence(geofence_id);
    return;
}

void mtk_hal_geofence_resume_geofence(int32_t geofence_id, int monitor_transitions) {
    if(!is_gfc_exist()) {
        LOGE("gfc not exist");
        return;
    }
    hal2_geofence_resume_geofence(geofence_id,monitor_transitions);
    return;
}

void mtk_hal_geofence_remove_geofences(int32_t geofence_id) {
    if(!is_gfc_exist()) {
        LOGE("gfc not exist");
        return;
    }
    hal2_geofence_remove_geofences(geofence_id);
    return;
}

/*************************************************************/
/*  Handle message from mnl to GPS/Geofence HAL              */
/*************************************************************/
void mtk_geofence_transition_callbacks_proc(MTK_FLP_MSG_T *prmsg) {
    int32_t geofence_id, transition;
    GpsLocation location;
    int64_t timestamp;
    #ifdef MTK_64_PLATFORM
    unsigned char ratio = 2; //32 to 64-bits
    unsigned char padding_diff = 4;
    unsigned char loc_in[128] = {0};
    unsigned int sizeof_loc_in = sizeof(GpsLocation) - sizeof(size_t)/ratio - padding_diff;
    #else
    uint32_t sizeof_loc_in = sizeof(GpsLocation);
    #endif
    memcpy(&geofence_id, (((char*)prmsg)+sizeof(MTK_FLP_MSG_T)), sizeof(int32_t));
    #ifdef MTK_64_PLATFORM
    memcpy(loc_in, (((char*)prmsg)+sizeof(MTK_FLP_MSG_T)+sizeof(int32_t)), sizeof_loc_in);
    mtk_loc_rearrange(loc_in, &location);
    #else
    memcpy(&location, (((char*)prmsg)+sizeof(MTK_FLP_MSG_T)+sizeof(int32_t)), sizeof_loc_in);
    #endif
    memcpy(&transition, (((char*)prmsg)+sizeof(MTK_FLP_MSG_T)+sizeof(int32_t)+sizeof_loc_in), sizeof(int32_t));
    memcpy(&timestamp, (((char*)prmsg)+sizeof(MTK_FLP_MSG_T)+sizeof(int32_t)+sizeof_loc_in+sizeof(int32_t)), sizeof(int64_t));
    LOGD("geo transition: id=%" PRId32 ", transition=%" PRId32 ", latlon=%lf,%lf",geofence_id,transition,location.latitude,location.longitude);

    if(g_gpshal_ctx.geofence_cbs == NULL) {
        LOGE("mtk_geofence_transition_callbacks_proc: NO geofence callbacks assigned");
        return;
    }

    //geofence CB for transition ntf
    if (set_buff_transition_fence(geofence_id, transition) == 0) {
       g_gpshal_ctx.geofence_cbs->geofence_transition_callback(geofence_id, &location, transition, timestamp);
    }
}

void mtk_geofence_callbacks_proc(MTK_FLP_MSG_T *prmsg) {
    MTK_GEOFENCE_CALLBACK_T cbs;

    memcpy(&cbs,(char*)prmsg+sizeof(MTK_FLP_MSG_T),sizeof(MTK_GEOFENCE_CALLBACK_T));
    //LOGD("geo_cb_proc: id=%d, result=%d",cbs.cb_id,cbs.result);

    if(g_gpshal_ctx.geofence_cbs == NULL) {
        LOGE("mtk_geofence_callbacks_proc: NO geofence callbacks assigned");
        return;
    }
    switch(cbs.cb_id) {
    case GEOFENCE_ADD_CALLBACK:
        g_gpshal_ctx.geofence_cbs->geofence_add_callback(cbs.geofence_id, cbs.result);
        break;
    case GEOFENCE_REMOVE_CALLBACK:
        g_gpshal_ctx.geofence_cbs->geofence_remove_callback(cbs.geofence_id, cbs.result);
        break;
    case GEOFENCE_PAUSE_CALLBACK:
        g_gpshal_ctx.geofence_cbs->geofence_pause_callback(cbs.geofence_id, cbs.result);
        break;
    case GEOFENCE_RESUME_CALLBACK:
        g_gpshal_ctx.geofence_cbs->geofence_resume_callback(cbs.geofence_id, cbs.result);
        break;
    default:
        LOGE("mtk_geofence_callbacks_proc: Unknown callback id:%d", cbs.cb_id);
        break;
    }
}

const GpsGeofencingInterface mtkGeofence_inf = {
    sizeof(GpsGeofencingInterface),
    mtk_hal_geofence_init,
    mtk_hal_geofence_add_geofences,
    mtk_hal_geofence_pause_geofence,
    mtk_hal_geofence_resume_geofence,
    mtk_hal_geofence_remove_geofences,
};


