#include <pthread.h>
#include <sys/epoll.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>  // struct sockaddr_in
#include <stdarg.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <cutils/properties.h>

#include "mnld.h"
#include "mnld_utile.h"
#include "mnl_common.h"
#include "mtk_gps.h"
#include "agps_agent.h"
#include "mtk_lbs_utility.h"
#include "mnl2hal_interface.h"
#include "mnl2agps_interface.h"
#include "data_coder.h"
#include "gps_controller.h"
#include "epo.h"
#include "qepo.h"
#include "mtknav.h"
#include "op01_log.h"
#include "mtk_flp_main.h"
#include "mnl_flp_test_interface.h"
#include "mtk_geofence_main.h"
#include "gps_dbg_log.h"
#include "mpe.h"
#include "mnl_at_interface.h"
#include "Mnld2NlpUtilsInterface.h"
#include "mtk_flp_screen_monitor.h"
#include "Meta2MnldInterface.h"
#include "Mnld2DebugInterface.h"
#include "Debug2MnldInterface.h"
#include <unistd.h>

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
#define LOGD(...) tag_log(1, "[mnld]", __VA_ARGS__);
#define LOGW(...) tag_log(1, "[mnld] WARNING: ", __VA_ARGS__);
#define LOGE(...) tag_log(1, "[mnld] ERR: ", __VA_ARGS__);
#else
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "MNLD"

#include <cutils/sockets.h>
#include <log/log.h>     /*logging in logcat*/
#define LOGD(fmt, arg ...) ALOGD("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGW(fmt, arg ...) ALOGW("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGE(fmt, arg ...) ALOGE("%s: " fmt, __FUNCTION__ , ##arg)
#ifdef CONFIG_GPS_ENG_LOAD
#define LOGD_ENG(fmt, arg ...) ALOGD("%s: " fmt, __FUNCTION__ , ##arg)
#else
#define LOGD_ENG(fmt, arg ...) NULL
#endif
#endif

static void mnld_fsm(mnld_gps_event event, int data1, int data2, void* data3);


static mnld_context g_mnld_ctx;
UINT8 sv_type_agps_set = 0;
UINT8 sib8_16_enable = 0;
UINT8 lppe_enable = 0;// if mnl support lppe, use this variable to determin on or off, else, it is invalid
mnl_agps_agps_settings g_settings_from_agps;
MTK_GPS_MNL_INFO mtk_gps_mnl_info;// include lib information:vertion and if support lppe
mnl_agps_gnss_settings g_settings_to_agps = {
    .gps_satellite_support     = 1,
    .glonass_satellite_support = 0,
    .beidou_satellite_support  = 0,
    .galileo_satellite_support = 0,
};
extern int gps_epo_type;
extern mtk_socket_fd gpslogd_fd;
extern bool mnld_exiting;

#if ANDROID_MNLD_PROP_SUPPORT
extern char chip_id[PROPERTY_VALUE_MAX];
#define MNL_CONFIG_STATUS "persist.vendor.radio.mnl.prop"
#else
extern char chip_id[100];
#endif

extern int in_meta_factory;
extern MTK_GPS_SV_BLACKLIST svBlacklist;
extern bool g_enable_full_tracking;
volatile UINT8 enable_debug2app = DEBUG2MNLD_INTERFACE_DEBUG_REQ_STATUS_CATEGORY_STOP_DEBUG;
/******************************************************************************************************************************
Steps to add NFW(None Framework) user visibility control:
Step 1. Add NFW user app name in nfw_app_name_list;
Step 2. Add NFW user ID to enum MNLD_NFW_USER;
Step 3. Complete the API to update to new NFW user allowed to access location or not, and call the API in hal_set_nfw_access();
Step 4. Call mtk_gps_get_nfw_visibility(nfw_user) API to check whether nfw_user in visibility check list
Step 5. Fill nfw_notification structure and call mnl2hal_nfw_notify() API in correct time.
*******************************************************************************************************************************/
char nfw_app_name_list[MNLD_NFW_USER_NUM][MNLD_NFW_USER_NAME_LEN] = {MNLD_NFW_PROXY_NAME};
nfw_notification nfw_user_notify[MNLD_NFW_USER_NUM];
bool g_mnld_nfw_notified[MNLD_NFW_USER_NUM] = {0};
nfw_notification nfw_user_notify_default[MNLD_NFW_USER_NUM] = {
    {//NFW AGPS
    .proxyAppPackageName = MNLD_NFW_PROXY_NAME,
    .protocolStack = NFW_PS_OTHER,
    .otherProtocolStackName = "MODEM",
    .requestor = NFW_REQUESTOR_MODEM_CHIPSET_VENDOR,
    .requestorId = "MTK",
    .inEmergencyMode = 0,
    .isCachedLocation = 0
    }
};

/*****************************************
MNLD FSM
*****************************************/
static const char* get_mnld_gps_state_str(mnld_gps_state input) {
    switch (input) {
    case MNLD_GPS_STATE_IDLE:
        return "IDLE";
    case MNLD_GPS_STATE_STARTING:
        return "STARTING";
    case MNLD_GPS_STATE_STARTED:
        return "STARTED";
    case MNLD_GPS_STATE_STOPPING:
        return "STOPPING";
    case MNLD_GPS_STATE_OFL_STARTING:
        return "OFL_STARTING";
    case MNLD_GPS_STATE_OFL_STARTED:
        return "OFL_STARTED";
    case MNLD_GPS_STATE_OFL_STOPPING:
        return "OFL_STOPPING";
    case MNLD_GPS_STATE_SUSPEND:
        return "SUSPEND";
    default:
        break;
    }
    return "UNKNOWN";
}

static const char* get_mnld_gps_event_str(mnld_gps_event input) {
    switch (input) {
    case GPS_EVENT_START:
        return "START";
    case GPS_EVENT_STOP:
        return "STOP";
    case GPS_EVENT_RESET:
        return "RESET";
    case GPS_EVENT_START_DONE:
        return "START_DONE";
    case GPS_EVENT_STOP_DONE:
        return "STOP_DONE";
    case GPS_EVENT_OFFLOAD_START:
        return "OFFLOAD_START";
    case GPS_EVENT_SUSPEND:
        return "SUSPEND";
    case GPS_EVENT_SUSPEND_DONE:
        return "SUSPEND_DONE";
    case GPS_EVENT_SUSPEND_CLOSE:
        return "SUSPEND_CLOSE";
    default:
        break;
    }
    return "UNKNOWN";
}

static void do_gps_reset_hdlr() {
    mnld_gps_client* agps = &g_mnld_ctx.gps_status.clients.agps;
    if (agps->need_reset_ack) {
        agps->need_reset_ack = false;
        mnl2agps_reset_gps_done();
    }
}

static void do_gps_started_hdlr(int si_assist_req) {
    mnld_gps_client* hal = &g_mnld_ctx.gps_status.clients.hal;
    mnld_gps_client* raw_meas = &g_mnld_ctx.gps_status.clients.raw_meas;
    mnld_gps_client* agps = &g_mnld_ctx.gps_status.clients.agps;
    mnld_gps_client* flp = &g_mnld_ctx.gps_status.clients.flp;
    mnld_gps_client* flp_test = &g_mnld_ctx.gps_status.clients.flp_test;
    mnld_gps_client* geofence = &g_mnld_ctx.gps_status.clients.geofence;
    mnld_gps_client* at_cmd_test = &g_mnld_ctx.gps_status.clients.at_cmd_test;
    mnld_gps_client* factory = &g_mnld_ctx.gps_status.clients.factory;
    if (hal->need_open_ack) {
        hal->need_open_ack = false;
        mnl2hal_gps_status(MTK_GPS_STATUS_SESSION_ENGINE_ON);
        mnl2hal_gps_status(MTK_GPS_STATUS_SESSION_BEGIN);
        mnl2agps_gps_open(si_assist_req);
    }

    if (agps->need_open_ack) {
        agps->need_open_ack = false;
        mnl2agps_open_gps_done();
    }

    if (flp->need_open_ack) {
        flp->need_open_ack = false;
        /*add open ack message to flp*/
        LOGD("MNLD_FLP_DEBUG:support_auto_switch:%d,offload_auto:%d",mnld_support_auto_switch_mode(),mnld_offload_is_auto_mode());
        if (mnld_support_auto_switch_mode()) {   //New offload auto switch mode
            LOGD("Auto switch mode");
            if (mnld_offload_is_auto_mode()) {
                LOGD("Offload Auto mode");
                mnld_flp_ofl_gps_open_done();
            } else {
                LOGD("HBD auto mode");
                mnld_flp_hbd_gps_open_done();
            }
        } else { //always host
            LOGD("HBD auto mode");
            mnld_flp_hbd_gps_open_done();
        }
        mnl2agps_gps_open(si_assist_req);
    }

    if (geofence->need_open_ack) {
        geofence->need_open_ack = false;
        /*add open ack message to gfc*/
        LOGD("MNLD_GFC_DEBUG:support_auto_switch:%d,offload_auto:%d",mnld_support_auto_switch_mode(),mnld_offload_is_auto_mode());
        if (mnld_support_auto_switch_mode()) {   //New offload auto switch mode
            LOGD("Auto switch mode");
            if (mnld_offload_is_auto_mode()) {
                LOGD("Offload Auto mode");
                mnld_gfc_ofl_gps_open_done();
            } else {
                LOGD("HBD auto mode");
                mnld_gfc_hbd_gps_open_done();
            }
        } else { //always host
            LOGD("HBD auto mode");
            mnld_gfc_hbd_gps_open_done();
        }
        mnl2agps_gps_open(si_assist_req);
    }

    if (flp_test->need_open_ack) {
        flp_test->need_open_ack = false;
        /*add open ack message to flp_test*/
    }

    if (at_cmd_test->need_open_ack) {
        at_cmd_test->need_open_ack = false;
        /*add open ack message to at_cmd_test*/
    }

    if (factory->need_open_ack) {
        factory->need_open_ack = false;
        /*add open ack message to factory*/
        mtk_gps_mnl_create_dbg_port(PMTK_CONNECTION_SOCKET_PORT);  //Create debug socket in running time
    }

    if (raw_meas->need_open_ack) {
        raw_meas->need_open_ack = false;
        int ret = mtk_gps_set_param(MTK_PARAM_CMD_ENABLE_FULL_TRACKING, &g_enable_full_tracking);
        if (ret < 0) {
            LOGW("set measurement parameter fail");
        }
    }

    if (enable_debug2app == DEBUG2MNLD_INTERFACE_DEBUG_REQ_STATUS_CATEGORY_START_DEBUG) {
        Mnld2DebugInterface_mnldUpdateGpsStatus(&g_mnld_ctx.fds.fd_debug_client,
            MNLD2DEBUG_INTERFACE_MNLD_GPS_STATUS_CATEGORY_GPS_STARTED);
    }

    do_gps_reset_hdlr();

    if (g_mnld_ctx.gps_status.delete_aiding_flag) {
        start_timer(g_mnld_ctx.gps_status.timer_reset, MNLD_GPS_RESET_TIMEOUT);
        gps_control_gps_reset(g_mnld_ctx.gps_status.delete_aiding_flag);
        g_mnld_ctx.gps_status.delete_aiding_flag = 0;
    }
}

static void do_gps_stopped_hdlr() {
    mnld_gps_client* hal = &g_mnld_ctx.gps_status.clients.hal;
    if (hal->need_close_ack) {
        hal->need_close_ack = false;
        mnl2hal_gps_status(MTK_GPS_STATUS_SESSION_END);
        mnl2hal_gps_status(MTK_GPS_STATUS_SESSION_ENGINE_OFF);
        mnl2agps_gps_close();
    }

    mnld_gps_client* agps = &g_mnld_ctx.gps_status.clients.agps;
    if (agps->need_close_ack) {
        agps->need_close_ack = false;
        mnl2agps_close_gps_done();
    }

    mnld_gps_client* flp = &g_mnld_ctx.gps_status.clients.flp;
    if (flp->need_close_ack) {
        flp->need_close_ack = false;
        /* add close ack message to flp*/
        LOGD("MNLD_FLP_DEBUG:support_auto_switch:%d,offload_auto:%d",mnld_support_auto_switch_mode(),mnld_offload_is_auto_mode());
        if (mnld_support_auto_switch_mode()) {   //New offload auto switch mode
            LOGD("Auto switch mode");
            if (mnld_offload_is_auto_mode()) {
                LOGD("Offload Auto mode");
                mnld_flp_ofl_gps_close_done();
            } else {
                LOGD("HBD auto mode");
                mnld_flp_hbd_gps_close_done();
            }
        } else { //always host
            LOGD("HBD auto mode");
            mnld_flp_hbd_gps_close_done();
        }
    }

    mnld_gps_client* geofence = &g_mnld_ctx.gps_status.clients.geofence;
    if (geofence->need_close_ack) {
        geofence->need_close_ack = false;
        /* add close ack message to geofence*/
        LOGD("MNLD_GFC_DEBUG:support_auto_switch:%d,offload_auto:%d",mnld_support_auto_switch_mode(),mnld_offload_is_auto_mode());
        if (mnld_support_auto_switch_mode()) {   //New offload auto switch mode
            LOGD("Auto switch mode");
            if (mnld_offload_is_auto_mode()) {
                LOGD("Offload Auto mode");
                mnld_gfc_ofl_gps_close_done();
            } else {
                LOGD("HBD auto mode");
                mnld_gfc_hbd_gps_close_done();
            }
        } else {
            LOGD("HBD auto mode");
            mnld_gfc_hbd_gps_close_done();
        }
    }

    mnld_gps_client* flp_test = &g_mnld_ctx.gps_status.clients.flp_test;
    if (flp_test->need_close_ack) {
        flp_test->need_close_ack = false;
        /* add close ack message to flp_test*/
    }

    mnld_gps_client* at_cmd_test = &g_mnld_ctx.gps_status.clients.at_cmd_test;
    if (at_cmd_test->need_close_ack) {
        at_cmd_test->need_close_ack = false;
        /* add close ack message to at_cmd_test*/
    }

    mnld_gps_client* factory = &g_mnld_ctx.gps_status.clients.factory;
    if (factory->need_close_ack) {
        factory->need_close_ack = false;
        /* add close ack message to factory*/
    }

    mnld_gps_client* raw_meas = &g_mnld_ctx.gps_status.clients.raw_meas;
    if (raw_meas->need_close_ack) {
        raw_meas->need_close_ack = false;
        /* add close ack message to raw_meas*/
    }

    if (enable_debug2app == DEBUG2MNLD_INTERFACE_DEBUG_REQ_STATUS_CATEGORY_START_DEBUG) {
        Mnld2DebugInterface_mnldUpdateGpsStatus(&g_mnld_ctx.fds.fd_debug_client,
            MNLD2DEBUG_INTERFACE_MNLD_GPS_STATUS_CATEGORY_GPS_STOPPED);
    }
    //Mnld2NlpUtilsInterface_cancelNlpLocation(&g_mnld_ctx.fds.fd_nlp_utils, NLP_REQUEST_SRC_MNL);

    do_gps_reset_hdlr();
}

static bool is_all_gps_client_exit() {
    mnld_gps_client* hal = &g_mnld_ctx.gps_status.clients.hal;
    mnld_gps_client* raw_meas = &g_mnld_ctx.gps_status.clients.raw_meas;
    mnld_gps_client* agps = &g_mnld_ctx.gps_status.clients.agps;
    mnld_gps_client* flp = &g_mnld_ctx.gps_status.clients.flp;
    mnld_gps_client* flp_test = &g_mnld_ctx.gps_status.clients.flp_test;
    mnld_gps_client* geofence = &g_mnld_ctx.gps_status.clients.geofence;
    mnld_gps_client* at_cmd_test = &g_mnld_ctx.gps_status.clients.at_cmd_test;
    mnld_gps_client* factory = &g_mnld_ctx.gps_status.clients.factory;
    if (hal->gps_used == false && agps->gps_used == false && flp->gps_used == false
        && flp_test->gps_used == false && at_cmd_test->gps_used == false && factory->gps_used == false
 && geofence->gps_used == false && raw_meas->gps_used == false) {
        return true;
    } else {
        return false;
    }
}

static bool is_a_gps_client_exist() {
    mnld_gps_client* hal = &g_mnld_ctx.gps_status.clients.hal;
    mnld_gps_client* raw_meas = &g_mnld_ctx.gps_status.clients.raw_meas;
    mnld_gps_client* agps = &g_mnld_ctx.gps_status.clients.agps;
    mnld_gps_client* flp = &g_mnld_ctx.gps_status.clients.flp;
    mnld_gps_client* flp_test = &g_mnld_ctx.gps_status.clients.flp_test;
    mnld_gps_client* geofence = &g_mnld_ctx.gps_status.clients.geofence;
    mnld_gps_client* at_cmd_test = &g_mnld_ctx.gps_status.clients.at_cmd_test;
    mnld_gps_client* factory = &g_mnld_ctx.gps_status.clients.factory;
    if (hal->gps_used == true || agps->gps_used == true || flp->gps_used == true || flp_test->gps_used == true
        || at_cmd_test->gps_used == true || factory->gps_used == true || geofence->gps_used == true || raw_meas->gps_used == true) {
        return true;
    } else {
        return false;
    }
}

void mtk_gps_clear_gps_user() {
    mnld_gps_client* hal = &g_mnld_ctx.gps_status.clients.hal;
    mnld_gps_client* raw_meas = &g_mnld_ctx.gps_status.clients.raw_meas;
    mnld_gps_client* agps = &g_mnld_ctx.gps_status.clients.agps;
    mnld_gps_client* flp = &g_mnld_ctx.gps_status.clients.flp;
    mnld_gps_client* flp_test = &g_mnld_ctx.gps_status.clients.flp_test;
    mnld_gps_client* geofence = &g_mnld_ctx.gps_status.clients.geofence;
    mnld_gps_client* at_cmd_test = &g_mnld_ctx.gps_status.clients.at_cmd_test;
    mnld_gps_client* factory = &g_mnld_ctx.gps_status.clients.factory;

    if (hal->gps_used)
        hal->gps_used = false;
    if (agps->gps_used)
        agps->gps_used = false;
    if (flp->gps_used)
        flp->gps_used = false;
    if (flp_test->gps_used)
        flp_test->gps_used = false;
    if (geofence->gps_used)
        geofence->gps_used = false;
    if (at_cmd_test->gps_used)
        at_cmd_test->gps_used = false;
    if (factory->gps_used)
        factory->gps_used = false;
    if (raw_meas->gps_used)
        raw_meas->gps_used = false;
}
int mtk_gps_get_gps_user() {
    mnld_gps_client* hal = &g_mnld_ctx.gps_status.clients.hal;
    mnld_gps_client* raw_meas = &g_mnld_ctx.gps_status.clients.raw_meas;
    mnld_gps_client* agps = &g_mnld_ctx.gps_status.clients.agps;
    mnld_gps_client* flp = &g_mnld_ctx.gps_status.clients.flp;
    mnld_gps_client* flp_test = &g_mnld_ctx.gps_status.clients.flp_test;
    mnld_gps_client* geofence = &g_mnld_ctx.gps_status.clients.geofence;
    mnld_gps_client* at_cmd_test = &g_mnld_ctx.gps_status.clients.at_cmd_test;
    mnld_gps_client* factory = &g_mnld_ctx.gps_status.clients.factory;
    int gps_user = GPS_USER_UNKNOWN;

    if (hal->gps_used)
        gps_user |= GPS_USER_APP;
    if (agps->gps_used)
        gps_user |= GPS_USER_AGPS;
    if (flp->gps_used)
        gps_user |= GPS_USER_FLP;
    if (flp_test->gps_used)
        gps_user |= GPS_USER_OFL_TEST;
    if (geofence->gps_used)
        gps_user |= GPS_USER_GEOFENCE;
    if (at_cmd_test->gps_used)
        gps_user |= GPS_USER_AT_CMD;
    if (factory->gps_used)
        gps_user |= GPS_USER_META;
    if (raw_meas->gps_used)
        gps_user |= GPS_USER_RAW_MEAS;
    return gps_user;
}

char mtk_gps_get_nfw_visibility(MNLD_NFW_USER nfw_user) {
    return g_mnld_ctx.nfw_user_visibility[nfw_user];
}

void mtk_gps_set_nfw_visibility(int usr, bool set_value) {
    g_mnld_ctx.nfw_user_visibility[usr] = set_value;
}

void mtk_gps_set_nfw_visibility_all(bool set_value) {
    int idx = 0;

    for(idx = 0; idx < MNLD_NFW_USER_NUM; idx++) {
        mtk_gps_set_nfw_visibility(idx, set_value);
    }
}

void mtk_gps_dump_nfw_visibility(void) {
    int idx = 0;

    for(idx = 0; idx < MNLD_NFW_USER_NUM; idx++) {
        LOGD("nfw_user_visibility[%d]:%d", idx, g_mnld_ctx.nfw_user_visibility[idx]);
    }
}

int is_flp_user_exist() {
    mnld_gps_client* flp = &g_mnld_ctx.gps_status.clients.flp;
    return flp->gps_used;
}

int is_geofence_user_exist() {
    mnld_gps_client* geofence = &g_mnld_ctx.gps_status.clients.geofence;
    return geofence->gps_used;
}


static bool is_all_hbd_gps_client_exit() {
    mnld_gps_client* hal = &g_mnld_ctx.gps_status.clients.hal;
    mnld_gps_client* raw_meas = &g_mnld_ctx.gps_status.clients.raw_meas;
    mnld_gps_client* agps = &g_mnld_ctx.gps_status.clients.agps;
    mnld_gps_client* at_cmd_test = &g_mnld_ctx.gps_status.clients.at_cmd_test;
    mnld_gps_client* factory = &g_mnld_ctx.gps_status.clients.factory;
    if (hal->gps_used == false && agps->gps_used == false && at_cmd_test->gps_used == false
        && factory->gps_used == false && raw_meas->gps_used == false) {
        return true;
    } else {
        return false;
    }
}

static bool is_a_hbd_gps_client_exist() {
    mnld_gps_client* hal = &g_mnld_ctx.gps_status.clients.hal;
    mnld_gps_client* raw_meas = &g_mnld_ctx.gps_status.clients.raw_meas;
    mnld_gps_client* agps = &g_mnld_ctx.gps_status.clients.agps;
    mnld_gps_client* at_cmd_test = &g_mnld_ctx.gps_status.clients.at_cmd_test;
    mnld_gps_client* factory = &g_mnld_ctx.gps_status.clients.factory;
    if (hal->gps_used == true || agps->gps_used == true || at_cmd_test->gps_used == true
        || factory->gps_used == true || raw_meas->gps_used == true) {
        return true;
    } else {
        return false;
    }
}

extern int start_time_out;
extern int nmea_data_time_out;
static void fsm_gps_state_idle(mnld_gps_event event, int data1, int data2, void* data3) {
    LOGD_ENG("fsm_gps_state_idle() data1=%d,data2=%d,data3=%p\n", data1, data2, data3);
    switch (event) {
    case GPS_EVENT_START: {
        if (!(mnl_offload_is_enabled() &&
            (mnld_offload_is_auto_mode() || mnld_offload_is_always_on_mode()))) {
            mnl2hal_request_wakelock();
        }
        #if ANDROID_MNLD_PROP_SUPPORT
        if (get_gps_cmcc_log_enabled()) {
            op01_log_gps_start();
        }
        #else
        op01_log_gps_start();
        #endif
        g_mnld_ctx.gps_status.gps_start_time = get_tick();
        g_mnld_ctx.gps_status.wait_first_location = true;
        g_mnld_ctx.gps_status.gps_state = MNLD_GPS_STATE_STARTING;
        int gps_user = mtk_gps_get_gps_user();
        // if no other users except GPS_USER_FLP or GPS_USER_OFL_TEST, bypass restart
        if (((gps_user & (GPS_USER_FLP | GPS_USER_OFL_TEST | GPS_USER_GEOFENCE)) == gps_user) && mnl_offload_is_enabled() &&
            (mnld_offload_is_auto_mode() || mnld_offload_is_always_on_mode())) {
            LOGE("gps_user: %d, not run MNL start timer\n", gps_user);
            LOGE("still enable start timer");
            start_timer(g_mnld_ctx.gps_status.timer_start, start_time_out);
        } else {
            start_timer(g_mnld_ctx.gps_status.timer_start, start_time_out);
        }
        gps_control_gps_start(g_mnld_ctx.gps_status.delete_aiding_flag);
        g_mnld_ctx.gps_status.delete_aiding_flag = 0;
        break;
    }
    case GPS_EVENT_STOP: {
        do_gps_stopped_hdlr();
        break;
    }
    case GPS_EVENT_RESET: {
        do_gps_reset_hdlr();
        break;
    }
    case GPS_EVENT_OFFLOAD_START: {
        if (!(mnl_offload_is_enabled() &&
            (mnld_offload_is_auto_mode() || mnld_offload_is_always_on_mode()))) {
            mnl2hal_request_wakelock();
        }
        #if ANDROID_MNLD_PROP_SUPPORT
        if (get_gps_cmcc_log_enabled()) {
            op01_log_gps_start();
        }
        #else
        op01_log_gps_start();
        #endif
        g_mnld_ctx.gps_status.gps_start_time = get_tick();
        g_mnld_ctx.gps_status.wait_first_location = true;
        g_mnld_ctx.gps_status.gps_state = MNLD_GPS_STATE_OFL_STARTING;
        int gps_user = mtk_gps_get_gps_user();
        // if no other users except GPS_USER_FLP or GPS_USER_OFL_TEST, bypass restart
        if (((gps_user & (GPS_USER_FLP | GPS_USER_OFL_TEST | GPS_USER_GEOFENCE)) == gps_user) && mnl_offload_is_enabled() &&
            (mnld_offload_is_auto_mode() || mnld_offload_is_always_on_mode())) {
            LOGE("gps_user: %d, not run MNL start timer\n", gps_user);
            LOGE("still enable start timer");
            start_timer(g_mnld_ctx.gps_status.timer_start, start_time_out);
        } else {
            start_timer(g_mnld_ctx.gps_status.timer_start, start_time_out);
        }
        gps_control_gps_start(g_mnld_ctx.gps_status.delete_aiding_flag);
        g_mnld_ctx.gps_status.delete_aiding_flag = 0;
        break;
    }
    case GPS_EVENT_START_DONE:
        // Need fix by MNL,it will send MTK_AGPS_CB_START_REQ to mnld when stopping or started.
        // LOGE("fsm_gps_state_stopping() unexpected event=%d", event);
        // break;
    case GPS_EVENT_STOP_DONE:
    default: {
        LOGE("fsm_gps_state_idle() unexpected gps_event=%d", event);
        CRASH_TO_DEBUG();
        break;
    }
    }
}

static void fsm_gps_state_starting(mnld_gps_event event, int data1, int data2, void* data3) {
    LOGD_ENG("fsm_gps_state_starting() data2=%d,data3=%p\n", data2, data3);
    switch (event) {
    case GPS_EVENT_STOP: {
        // Need to handle starting-stop event for FLP offload like started-stop
        if (mnl_offload_is_enabled() &&
            (mnld_offload_is_auto_mode() || mnld_offload_is_always_on_mode())) {
            #if 1
            LOGE("fsm_gps_state_starting, !NOT! handle starting stop event for offload");
            #else
            LOGE("fsm_gps_state_starting, handle starting stop event for offload");
            do_gps_started_hdlr(0);
            if (is_all_gps_client_exit()) {
                g_mnld_ctx.gps_status.gps_state = MNLD_GPS_STATE_STOPPING;
                start_timer(g_mnld_ctx.gps_status.timer_stop, MNLD_GPS_STOP_TIMEOUT);
                gps_control_gps_stop();
            }
            #endif
        }
        break;
    }
    case GPS_EVENT_START:
    case GPS_EVENT_OFFLOAD_START:
    case GPS_EVENT_RESET: {
        // do nothing
        break;
    }
    case GPS_EVENT_START_DONE: {
        stop_timer(g_mnld_ctx.gps_status.timer_start);
        do_gps_started_hdlr(data1);
        if (is_all_gps_client_exit()) {
            g_mnld_ctx.gps_status.gps_state = MNLD_GPS_STATE_STOPPING;
            start_timer(g_mnld_ctx.gps_status.timer_stop, MNLD_GPS_STOP_TIMEOUT);
            gps_control_gps_stop();
        } else {
            g_mnld_ctx.gps_status.gps_state = MNLD_GPS_STATE_STARTED;
            //Update mtknav and qepo if download finish before start done.
            if (mtknav_update_flag == true) {
                mtknav_update_mtknav_file(mtknav_res);
                mtknav_update_flag = false;
            }
            if (qepo_update_flag == true) {
                qepo_update_quarter_epo_file(qepo_dl_res);
                qepo_update_flag = false;
            }
            if (qepo_BD_update_flag == true) {
                qepo_update_quarter_epo_bd_file(qepo_bd_dl_res);
                qepo_BD_update_flag = false;
            }
            if (qepo_GA_update_flag == true) {
                qepo_update_quarter_epo_ga_file(qepo_ga_dl_res);
                qepo_GA_update_flag = false;
            }
            if (is_a_hbd_gps_client_exist() || !(mnld_support_auto_switch_mode())) {
                mnld_flp_session.type = MNLD_FLP_CAPABILITY_AP_MODE;
                mnld_gfc_session.type = MNLD_GFC_CAPABILITY_AP_MODE;
                if (mnld_flp_session.type != mnld_flp_session.pre_type) {
                    mnld_flp_session.id = mnld_flp_gen_new_session();
                    mnld_flp_session.pre_type = mnld_flp_session.type;
                    g_mnld_ctx.gps_status.clients.flp.gps_used = false;
                    mnld_flp_session_update();
                }
                if (mnld_gfc_session.type != mnld_gfc_session.pre_type) {
                    mnld_gfc_session.id = mnld_gfc_gen_new_session();
                    mnld_gfc_session.pre_type = mnld_gfc_session.type;
                    g_mnld_ctx.gps_status.clients.geofence.gps_used = false;
                    mnld_gfc_session_update();
                }
            } else {
                mnld_flp_session.type = MNLD_FLP_CAPABILITY_AP_MODE;
                mnld_flp_session.pre_type = mnld_flp_session.type;
                mnld_gfc_session.type = MNLD_GFC_CAPABILITY_AP_MODE;
                mnld_gfc_session.pre_type = mnld_gfc_session.type;
                start_timer(g_mnld_ctx.gps_status.timer_switch_ofl_mode, MNLD_GPS_SWITCH_OFL_MODE_TIMEOUT);
            }
        }
        break;
    }
    case GPS_EVENT_STOP_DONE:
    default: {
        LOGE("fsm_gps_state_starting() unexpected gps_event=%d", event);
        CRASH_TO_DEBUG();
        break;
    }
    }
}

static void fsm_gps_state_started(mnld_gps_event event, int data1, int data2, void* data3) {
    LOGD_ENG("fsm_gps_state_started() data1=%d,data2=%d,data3=%p\n", data1, data2, data3);
    switch (event) {
    case GPS_EVENT_START: {
        do_gps_started_hdlr(0);
        break;
    }
    case GPS_EVENT_STOP: {
        if (is_all_gps_client_exit()) {
            g_mnld_ctx.gps_status.gps_state = MNLD_GPS_STATE_STOPPING;
            start_timer(g_mnld_ctx.gps_status.timer_stop, MNLD_GPS_STOP_TIMEOUT);
            stop_timer(g_mnld_ctx.gps_status.timer_switch_ofl_mode);

#if defined(GPS_SUSPEND_SUPPORT)
            if (mnld_gps_suspend_is_enabled()) {
                bool to_go = false;
                bool meta_or_factory = in_meta_factory;
                int timeout_sec = mnld_gps_suspend_get_timeout_sec();

                if (g_mnld_ctx.gps_status.is_in_nmea_timeout_handler || meta_or_factory) {
                    // For nmea timeout case, need to do close rather than suspend
                    // and meta_or_factory not utilize suspend mode
                    to_go = false;
                } else if (g_mnld_ctx.screen_status == SCREEN_STATUS_ON) {
                    // The strategy is using suspend to replace close when screen on,
                    // so timeout_sec is ignored and to_go is set true.
                    to_go = true;
                } else if (g_mnld_ctx.screen_status == SCREEN_STATUS_OFF &&
                    timeout_sec > 0) {
                    // timeout_sec > 0 stands for it needs to keep in suspend for
                    // some time (timeout_sec) when user stop gps under screen off condition.
                    // So, to_go still true;
                    // Otherwise, we need not to go in suspend status, so to_go is false
                    to_go = true;
                }

                LOGW("mnld_gps_suspend check: to_go = %d, scr = %d, timeout_sec = %d, nmea_timeout = %d, meta_or_factory = %d",
                    to_go, g_mnld_ctx.screen_status, timeout_sec,
                    g_mnld_ctx.gps_status.is_in_nmea_timeout_handler, meta_or_factory);

                if (to_go) {
                    gps_control_gps_suspend();
                    break;
                }
            }
#endif
            gps_control_gps_stop();
        } else {
            do_gps_stopped_hdlr();
            if (is_all_hbd_gps_client_exit() && mnld_support_auto_switch_mode()) {
                start_timer(g_mnld_ctx.gps_status.timer_switch_ofl_mode, MNLD_GPS_SWITCH_OFL_MODE_TIMEOUT);
            }
        }
        break;
    }
    case GPS_EVENT_RESET: {
        if (g_mnld_ctx.gps_status.delete_aiding_flag) {
            start_timer(g_mnld_ctx.gps_status.timer_reset, MNLD_GPS_RESET_TIMEOUT);
            gps_control_gps_reset(g_mnld_ctx.gps_status.delete_aiding_flag);
            g_mnld_ctx.gps_status.delete_aiding_flag = 0;
        }
        break;
    }
    case GPS_EVENT_START_DONE:
        // MNL restart.
        if (GPS_USER_APP & mtk_gps_get_gps_user()) {
            mnl2agps_reaiding_req();
        }
        LOGE("fsm_gps_state_started() unexpected event=%d", event);
        break;
    case GPS_EVENT_OFFLOAD_START:
        // Do nothing
        LOGE("fsm_gps_state_started() unexpected event=%d", event);
        break;
    case GPS_EVENT_STOP_DONE:
    default: {
        LOGE("fsm_gps_state_started() unexpected gps_event=%d", event);
        CRASH_TO_DEBUG();
        break;
    }
    }
}

static void fsm_gps_state_stopping(mnld_gps_event event, int data1, int data2, void* data3) {
    LOGD_ENG("fsm_gps_state_stopping() data1=%d,data2=%d,data3=%p\n", data1, data2, data3);
    switch (event) {
    case GPS_EVENT_START:
    case GPS_EVENT_STOP: {
        // do nothing
        break;
    }
    case GPS_EVENT_RESET: {
        do_gps_reset_hdlr();
        break;
    }
    case GPS_EVENT_STOP_DONE: {
        stop_timer(g_mnld_ctx.gps_status.timer_stop);
        do_gps_stopped_hdlr();
        if (is_a_gps_client_exist()) {
            LOGD("OFFLOAD debug: restarting");
            g_mnld_ctx.gps_status.gps_state = MNLD_GPS_STATE_STARTING;
            int gps_user = mtk_gps_get_gps_user();
            // if no other users except GPS_USER_FLP or GPS_USER_OFL_TEST, bypass restart
            if (((gps_user & (GPS_USER_FLP | GPS_USER_OFL_TEST | GPS_USER_GEOFENCE)) == gps_user) && mnl_offload_is_enabled() &&
            (mnld_offload_is_auto_mode() || mnld_offload_is_always_on_mode())) {
                LOGE("gps_user: %d\n", gps_user);
            } else {
                start_timer(g_mnld_ctx.gps_status.timer_start, start_time_out);
            }
            gps_control_gps_start(g_mnld_ctx.gps_status.delete_aiding_flag);
            g_mnld_ctx.gps_status.delete_aiding_flag = 0;
        } else {
            LOGD("OFFLOAD debug: stop done");
            if (!(mnl_offload_is_enabled() &&
            (mnld_offload_is_auto_mode() || mnld_offload_is_always_on_mode()))) {
                mnl2hal_release_wakelock();
            }
            #if ANDROID_MNLD_PROP_SUPPORT
            if (get_gps_cmcc_log_enabled()) {
                op01_log_gps_stop();
            }
            #else
            op01_log_gps_stop();
            #endif
            g_mnld_ctx.gps_status.gps_stop_time = get_tick();
            g_mnld_ctx.gps_status.wait_first_location = false;
            g_mnld_ctx.gps_status.gps_state = MNLD_GPS_STATE_IDLE;
            LOGD("OFFLOAD debug: update session");

            if(mnld_support_auto_switch_mode()) {
                mnld_flp_session.type = MNLD_FLP_CAPABILITY_AP_OFL_MODE;
                if (mnld_flp_session.type != mnld_flp_session.pre_type) {
                    mnld_flp_session.id = mnld_flp_gen_new_session();
                    mnld_flp_session.pre_type = mnld_flp_session.type;
                    g_mnld_ctx.gps_status.clients.flp.gps_used = false;
                    mnld_flp_session_update();
                }
                mnld_gfc_session.type = MNLD_GFC_CAPABILITY_AP_OFL_MODE;
                if (mnld_gfc_session.type != mnld_gfc_session.pre_type) {
                    mnld_gfc_session.id = mnld_gfc_gen_new_session();
                    mnld_gfc_session.pre_type = mnld_gfc_session.type;
                    g_mnld_ctx.gps_status.clients.geofence.gps_used = false;
                    mnld_gfc_session_update();
                }
            }
        }
        break;
    }
#if defined(GPS_SUSPEND_SUPPORT)
    case GPS_EVENT_SUSPEND_DONE: {
        stop_timer(g_mnld_ctx.gps_status.timer_stop);
        do_gps_stopped_hdlr();
        if (is_a_gps_client_exist()) {
            LOGD("OFFLOAD debug: restarting");
            g_mnld_ctx.gps_status.gps_state = MNLD_GPS_STATE_STARTING;
            int gps_user = mtk_gps_get_gps_user();
            // if no other users except GPS_USER_FLP or GPS_USER_OFL_TEST, bypass restart
            if (((gps_user & (GPS_USER_FLP | GPS_USER_OFL_TEST | GPS_USER_GEOFENCE)) == gps_user) && mnl_offload_is_enabled() &&
            (mnld_offload_is_auto_mode() || mnld_offload_is_always_on_mode())) {
                LOGE("gps_user: %d\n", gps_user);
            } else {
                start_timer(g_mnld_ctx.gps_status.timer_start, start_time_out);
            }
            gps_control_gps_start(g_mnld_ctx.gps_status.delete_aiding_flag);
            g_mnld_ctx.gps_status.delete_aiding_flag = 0;
        } else {
            LOGD("OFFLOAD debug: suspend done");
            if (!(mnl_offload_is_enabled() &&
            (mnld_offload_is_auto_mode() || mnld_offload_is_always_on_mode()))) {
                mnl2hal_release_wakelock();
            }
    #if ANDROID_MNLD_PROP_SUPPORT
            if (get_gps_cmcc_log_enabled()) {
                op01_log_gps_stop();
            }
    #else
            op01_log_gps_stop();
    #endif
            g_mnld_ctx.gps_status.gps_stop_time = get_tick();
            g_mnld_ctx.gps_status.wait_first_location = false;
            g_mnld_ctx.gps_status.gps_state = MNLD_GPS_STATE_SUSPEND;
            if (g_mnld_ctx.screen_status == SCREEN_STATUS_OFF &&
                mnld_gps_suspend_get_timeout_sec() > 0) {
                // Need to change suspend to close after timeout
                g_mnld_ctx.gps_status.is_suspend_timer_running = true;
                start_timer(g_mnld_ctx.gps_status.timer_suspend,
                    mnld_gps_suspend_get_timeout_sec());
            }

            LOGD("OFFLOAD debug: update session");

            if(mnld_support_auto_switch_mode()) {
                mnld_flp_session.type = MNLD_FLP_CAPABILITY_AP_OFL_MODE;
                if (mnld_flp_session.type != mnld_flp_session.pre_type) {
                    mnld_flp_session.id = mnld_flp_gen_new_session();
                    mnld_flp_session.pre_type = mnld_flp_session.type;
                    g_mnld_ctx.gps_status.clients.flp.gps_used = false;
                    mnld_flp_session_update();
                }
                mnld_gfc_session.type = MNLD_GFC_CAPABILITY_AP_OFL_MODE;
                if (mnld_gfc_session.type != mnld_gfc_session.pre_type) {
                    mnld_gfc_session.id = mnld_gfc_gen_new_session();
                    mnld_gfc_session.pre_type = mnld_gfc_session.type;
                    g_mnld_ctx.gps_status.clients.geofence.gps_used = false;
                    mnld_gfc_session_update();
                }
            }
        }
        break;
    }
#endif /* GPS_SUSPEND_SUPPORT */
    case GPS_EVENT_OFFLOAD_START:
    case GPS_EVENT_START_DONE:
        // MNL restart.
        LOGE("fsm_gps_state_stopping() unexpected event=%d", event);
        break;
    default: {
        LOGE("fsm_gps_state_stopping() unexpected gps_event=%d", event);
        CRASH_TO_DEBUG();
        break;
    }
    }
}

static void fsm_gps_state_ofl_starting(mnld_gps_event event, int data1, int data2, void* data3) {
    LOGD_ENG("fsm_gps_state_starting() data2=%d,data3=%p\n", data2, data3);
    switch (event) {
    case GPS_EVENT_STOP: {
        // Need to handle starting-stop event for FLP offload like started-stop
        if (mnl_offload_is_enabled() &&
            (mnld_offload_is_auto_mode() || mnld_offload_is_always_on_mode())) {
            #if 1
            LOGE("fsm_gps_state_starting, !NOT! handle starting stop event for offload");
            #else
            LOGE("fsm_gps_state_starting, handle starting stop event for offload");
            do_gps_started_hdlr(0);
            if (is_all_gps_client_exit()) {
                g_mnld_ctx.gps_status.gps_state = MNLD_GPS_STATE_STOPPING;
                start_timer(g_mnld_ctx.gps_status.timer_stop, MNLD_GPS_STOP_TIMEOUT);
                gps_control_gps_stop();
            }
            #endif
        }
        break;
    }
    case GPS_EVENT_START:
    case GPS_EVENT_OFFLOAD_START:
    case GPS_EVENT_RESET: {
        // do nothing
        break;
    }
    case GPS_EVENT_START_DONE: {
        stop_timer(g_mnld_ctx.gps_status.timer_start);
        do_gps_started_hdlr(data1);
        if (is_all_gps_client_exit() || is_a_hbd_gps_client_exist()) {
            g_mnld_ctx.gps_status.gps_state = MNLD_GPS_STATE_OFL_STOPPING;
            start_timer(g_mnld_ctx.gps_status.timer_stop, MNLD_GPS_STOP_TIMEOUT);
            gps_control_gps_stop();
        } else {
            g_mnld_ctx.gps_status.gps_state = MNLD_GPS_STATE_OFL_STARTED;
            mnld_flp_session.type = MNLD_FLP_CAPABILITY_OFL_MODE;
            mnld_gfc_session.type = MNLD_GFC_CAPABILITY_OFL_MODE;
        }
        break;
    }
    case GPS_EVENT_STOP_DONE:
    default: {
        LOGE("fsm_gps_state_starting() unexpected gps_event=%d", event);
        CRASH_TO_DEBUG();
        break;
    }
    }
}

static void fsm_gps_state_ofl_started(mnld_gps_event event, int data1, int data2, void* data3) {
    LOGD_ENG("fsm_gps_state_started() data1=%d,data2=%d,data3=%p\n", data1, data2, data3);
    switch (event) {
    case GPS_EVENT_START: {
        do_gps_started_hdlr(0);
        if (is_a_hbd_gps_client_exist()) {
            //flp
            g_mnld_ctx.gps_status.clients.flp.gps_used = false;
            g_mnld_ctx.gps_status.clients.flp.need_open_ack = false;
            g_mnld_ctx.gps_status.clients.flp.need_close_ack = false;
            g_mnld_ctx.gps_status.gps_state = MNLD_GPS_STATE_OFL_STOPPING;
            //geofence
            g_mnld_ctx.gps_status.clients.geofence.gps_used = false;
            g_mnld_ctx.gps_status.clients.geofence.need_open_ack = false;
            g_mnld_ctx.gps_status.clients.geofence.need_close_ack = false;
            start_timer(g_mnld_ctx.gps_status.timer_stop, MNLD_GPS_STOP_TIMEOUT);
            gps_control_gps_stop();
        }
        break;
    }
    case GPS_EVENT_STOP: {
        if (is_flp_user_exist() || is_geofence_user_exist()) {
            do_gps_stopped_hdlr();
            LOGW("GPS_EVENT_STOP cmd sent by error user!!");
        } else {
            g_mnld_ctx.gps_status.gps_state = MNLD_GPS_STATE_OFL_STOPPING;
            start_timer(g_mnld_ctx.gps_status.timer_stop, MNLD_GPS_STOP_TIMEOUT);
            gps_control_gps_stop();
        }
        break;
    }
    case GPS_EVENT_RESET: {
        //if (g_mnld_ctx.gps_status.delete_aiding_flag) {
        //    start_timer(g_mnld_ctx.gps_status.timer_reset, MNLD_GPS_RESET_TIMEOUT);
        //    gps_control_gps_reset(g_mnld_ctx.gps_status.delete_aiding_flag);
        //    g_mnld_ctx.gps_status.delete_aiding_flag = 0;
        //}
        break;
    }
    case GPS_EVENT_START_DONE:
        // MNL restart.
        if (GPS_USER_APP & mtk_gps_get_gps_user()) {
            mnl2agps_reaiding_req();
        }
        LOGE("fsm_gps_state_started() unexpected event=%d", event);
        break;
    case GPS_EVENT_OFFLOAD_START:
        LOGE("fsm_gps_state_started() unexpected event=%d", event);
        break;
    case GPS_EVENT_STOP_DONE:
    default: {
        LOGE("fsm_gps_state_started() unexpected gps_event=%d", event);
        CRASH_TO_DEBUG();
        break;
    }
    }
}

static void fsm_gps_state_ofl_stopping(mnld_gps_event event, int data1, int data2, void* data3) {
    LOGD_ENG("fsm_gps_state_stopping() data1=%d,data2=%d,data3=%p\n", data1, data2, data3);
    switch (event) {
    case GPS_EVENT_START:
    case GPS_EVENT_STOP: {
        // do nothing
        break;
    }
    case GPS_EVENT_RESET: {
        do_gps_reset_hdlr();
        break;
    }
    case GPS_EVENT_STOP_DONE: {
        stop_timer(g_mnld_ctx.gps_status.timer_stop);
        do_gps_stopped_hdlr();
        if ((is_flp_user_exist() || is_geofence_user_exist()) && is_all_hbd_gps_client_exit()) {
            g_mnld_ctx.gps_status.gps_state = MNLD_GPS_STATE_OFL_STARTING;
            int gps_user = mtk_gps_get_gps_user();
            // if no other users except GPS_USER_FLP or GPS_USER_OFL_TEST, bypass restart
            if (((gps_user & (GPS_USER_FLP | GPS_USER_OFL_TEST| GPS_USER_GEOFENCE)) == gps_user) && mnl_offload_is_enabled() &&
            (mnld_offload_is_auto_mode() || mnld_offload_is_always_on_mode())) {
                LOGE("gps_user: %d\n", gps_user);
            } else {
                start_timer(g_mnld_ctx.gps_status.timer_start, start_time_out);
            }
            gps_control_gps_start(g_mnld_ctx.gps_status.delete_aiding_flag);
            g_mnld_ctx.gps_status.delete_aiding_flag = 0;
        } else {
            if (!(mnl_offload_is_enabled() &&
            (mnld_offload_is_auto_mode() || mnld_offload_is_always_on_mode()))) {
                mnl2hal_release_wakelock();
            }
            #if ANDROID_MNLD_PROP_SUPPORT
            if (get_gps_cmcc_log_enabled()) {
                op01_log_gps_stop();
            }
            #else
            op01_log_gps_stop();
            #endif
            g_mnld_ctx.gps_status.gps_stop_time = get_tick();
            g_mnld_ctx.gps_status.wait_first_location = false;
            g_mnld_ctx.gps_status.gps_state = MNLD_GPS_STATE_IDLE;

            if (is_a_hbd_gps_client_exist()) {  //some hbd user exist, must switch to AP mode and start gps
                mnld_flp_session.type = MNLD_FLP_CAPABILITY_AP_MODE;
                if (mnld_flp_session.type != mnld_flp_session.pre_type) {
                    mnld_flp_session.id = mnld_flp_gen_new_session();
                    mnld_flp_session.pre_type = mnld_flp_session.type;
                    g_mnld_ctx.gps_status.clients.flp.gps_used = false;
                    mnld_flp_session_update();
                }
                mnld_gfc_session.type = MNLD_GFC_CAPABILITY_AP_MODE;
                if (mnld_gfc_session.type != mnld_gfc_session.pre_type) {
                    mnld_gfc_session.id = mnld_gfc_gen_new_session();
                    mnld_gfc_session.pre_type = mnld_gfc_session.type;
                    g_mnld_ctx.gps_status.clients.geofence.gps_used = false;
                    mnld_gfc_session_update();
                }
                mnld_fsm(GPS_EVENT_START, 0, 0, NULL);
            } else {   // flp user exit normally and no user exist, so switch to idle state directly
                if(mnld_support_auto_switch_mode()) {
                    mnld_flp_session.type = MNLD_FLP_CAPABILITY_AP_OFL_MODE;
                    if (mnld_flp_session.type != mnld_flp_session.pre_type) {
                        mnld_flp_session.id = mnld_flp_gen_new_session();
                        mnld_flp_session.pre_type = mnld_flp_session.type;
                        g_mnld_ctx.gps_status.clients.flp.gps_used = false;
                        mnld_flp_session_update();
                    }
                    mnld_gfc_session.type = MNLD_GFC_CAPABILITY_AP_OFL_MODE;
                    if (mnld_gfc_session.type != mnld_gfc_session.pre_type) {
                        mnld_gfc_session.id = mnld_gfc_gen_new_session();
                        mnld_gfc_session.pre_type = mnld_gfc_session.type;
                        g_mnld_ctx.gps_status.clients.geofence.gps_used = false;
                        mnld_gfc_session_update();
                    }
                }
            }
        }
        break;
    }
    case GPS_EVENT_OFFLOAD_START:
    case GPS_EVENT_START_DONE:
        // MNL restart.
        LOGE("fsm_gps_state_stopping() unexpected event=%d", event);
        break;
    default: {
        LOGE("fsm_gps_state_stopping() unexpected gps_event=%d", event);
        CRASH_TO_DEBUG();
        break;
    }
    }
}

#if defined(GPS_SUSPEND_SUPPORT)
static void fsm_gps_state_suspend(mnld_gps_event event, int data1, int data2, void* data3) {
    LOGD("fsm_gps_state_suspend() data1=%d,data2=%d,data3=%p\n", data1, data2, data3);
    switch (event) {
    case GPS_EVENT_START: {
        if (!(mnl_offload_is_enabled() &&
            (mnld_offload_is_auto_mode() || mnld_offload_is_always_on_mode()))) {
            mnl2hal_request_wakelock();
        }
    #if ANDROID_MNLD_PROP_SUPPORT
        if (get_gps_cmcc_log_enabled()) {
            op01_log_gps_start();
        }
    #else
        op01_log_gps_start();
    #endif
        g_mnld_ctx.gps_status.gps_start_time = get_tick();
        g_mnld_ctx.gps_status.wait_first_location = true;
        g_mnld_ctx.gps_status.gps_state = MNLD_GPS_STATE_STARTING;
        int gps_user = mtk_gps_get_gps_user();
        // if no other users except GPS_USER_FLP or GPS_USER_OFL_TEST, bypass restart
        if (((gps_user & (GPS_USER_FLP | GPS_USER_OFL_TEST | GPS_USER_GEOFENCE)) == gps_user) && mnl_offload_is_enabled() &&
            (mnld_offload_is_auto_mode() || mnld_offload_is_always_on_mode())) {
            LOGE("gps_user: %d, not run MNL start timer\n", gps_user);
            LOGE("still enable start timer");
            start_timer(g_mnld_ctx.gps_status.timer_start, start_time_out);
        } else {
            start_timer(g_mnld_ctx.gps_status.timer_start, start_time_out);
        }

        // stop suspend timer
        if (g_mnld_ctx.gps_status.is_suspend_timer_running) {
            stop_timer(g_mnld_ctx.gps_status.timer_suspend);
            g_mnld_ctx.gps_status.is_suspend_timer_running = false;
        }

        gps_control_gps_start(g_mnld_ctx.gps_status.delete_aiding_flag);
        g_mnld_ctx.gps_status.delete_aiding_flag = 0;
        break;
    }
    case GPS_EVENT_STOP: {
        do_gps_stopped_hdlr();
        break;
    }
    case GPS_EVENT_RESET: {
        do_gps_reset_hdlr();
        break;
    }
    case GPS_EVENT_OFFLOAD_START: {
        if (!(mnl_offload_is_enabled() &&
            (mnld_offload_is_auto_mode() || mnld_offload_is_always_on_mode()))) {
            mnl2hal_request_wakelock();
        }
    #if ANDROID_MNLD_PROP_SUPPORT
        if (get_gps_cmcc_log_enabled()) {
            op01_log_gps_start();
        }
    #else
        op01_log_gps_start();
    #endif
        g_mnld_ctx.gps_status.gps_start_time = get_tick();
        g_mnld_ctx.gps_status.wait_first_location = true;
        g_mnld_ctx.gps_status.gps_state = MNLD_GPS_STATE_OFL_STARTING;
        int gps_user = mtk_gps_get_gps_user();
        // if no other users except GPS_USER_FLP or GPS_USER_OFL_TEST, bypass restart
        if (((gps_user & (GPS_USER_FLP | GPS_USER_OFL_TEST | GPS_USER_GEOFENCE)) == gps_user) && mnl_offload_is_enabled() &&
            (mnld_offload_is_auto_mode() || mnld_offload_is_always_on_mode())) {
            LOGE("gps_user: %d, not run MNL start timer\n", gps_user);
            LOGE("still enable start timer");
            start_timer(g_mnld_ctx.gps_status.timer_start, start_time_out);
        } else {
            start_timer(g_mnld_ctx.gps_status.timer_start, start_time_out);
        }
        gps_control_gps_start(g_mnld_ctx.gps_status.delete_aiding_flag);
        g_mnld_ctx.gps_status.delete_aiding_flag = 0;
        break;
    }
    case GPS_EVENT_SUSPEND_CLOSE: {
        if (g_mnld_ctx.gps_status.is_suspend_timer_running) {
            stop_timer(g_mnld_ctx.gps_status.timer_suspend);
            g_mnld_ctx.gps_status.is_suspend_timer_running = false;
        }
        gps_control_gps_suspend_to_close();
        g_mnld_ctx.gps_status.gps_state = MNLD_GPS_STATE_IDLE;
        break;
    }
    case GPS_EVENT_START_DONE:
        // Need fix by MNL,it will send MTK_AGPS_CB_START_REQ to mnld when stopping or started.
        // LOGE("fsm_gps_state_stopping() unexpected event=%d", event);
        // break;
    case GPS_EVENT_STOP_DONE:
    default: {
        LOGE("fsm_gps_state_idle() unexpected gps_event=%d", event);
        CRASH_TO_DEBUG();
        break;
    }
    }
}
#endif

int mnld_gps_controller_mnl_nmea_timeout(void) {
    if(mnld_is_gps_or_ofl_started_done()) {
        if (g_mnld_ctx.gps_status.timer_nmea_monitor != 0) {
            // stop_timer(g_mnld_ctx.gps_status.timer_nmea_monitor);
            start_timer(g_mnld_ctx.gps_status.timer_nmea_monitor, nmea_data_time_out);
        }
    } else {
        LOGD("GPS not in started done state, ignore timer for NMEA monitor!!!");
    }
    return 0;
}

int mnld_gps_start_nmea_monitor() {
    if(mnld_is_gps_or_ofl_started_done()) {
        if (g_mnld_ctx.gps_status.timer_nmea_monitor != 0) {
            start_timer(g_mnld_ctx.gps_status.timer_nmea_monitor, nmea_data_time_out);
        }
    } else {
        LOGD("GPS not in started done state, ignore timer for NMEA monitor!!!");
    }
    return 0;
}

int mnld_gps_stop_nmea_monitor() {
    if (g_mnld_ctx.gps_status.timer_nmea_monitor != 0) {
        stop_timer(g_mnld_ctx.gps_status.timer_nmea_monitor);
    }
    return 0;
}

static int mnld_fsm_offload_old_user = 0;
static int mnld_fsm_offload_lp_mode = 0;
static int mnld_fsm_offload_fwk_wakelock = 0;
static void mnld_fsm(mnld_gps_event event, int data1, int data2, void* data3) {
    mnld_gps_state gps_state = g_mnld_ctx.gps_status.gps_state;

    if (!(mnl_offload_is_enabled() &&
            (mnld_offload_is_auto_mode() || mnld_offload_is_always_on_mode()))) {
        int user_bitmap;
        user_bitmap = mtk_gps_get_gps_user();
        LOGW("mnld_fsm() state=[%s] event=[%s], user=0x%08x, old user=0x%08x",
            get_mnld_gps_state_str(gps_state), get_mnld_gps_event_str(event),
            user_bitmap, mnld_fsm_offload_old_user);
    } else {
        // Wakelock control for FLP offload user
        int user_bitmap;
        user_bitmap = mtk_gps_get_gps_user();
        LOGW("mnld_fsm() state=[%s] event=[%s], user=0x%08x, old user=0x%08x",
            get_mnld_gps_state_str(gps_state), get_mnld_gps_event_str(event),
            user_bitmap, mnld_fsm_offload_old_user);

        // Libmnl.so need to know once the users changed, then
        // it can notify connsys to enter low power mode or NMEA mode
        mtk_gps_ofl_set_user(user_bitmap);

        // If only FLP offload user or No user, should stop nmea timer
        if ((user_bitmap & (GPS_USER_FLP | GPS_USER_OFL_TEST | GPS_USER_GEOFENCE)) == user_bitmap) {
            if (!mnld_fsm_offload_lp_mode) {
                LOGD("mnld_fsm() enter offload lp mode");
                mnld_gps_stop_nmea_monitor();
                gps_control_kernel_wakelock_give();
                if (mnld_fsm_offload_fwk_wakelock) {
                    mnl2hal_release_wakelock();
                    mnld_fsm_offload_fwk_wakelock = 0;
                }
                mnld_fsm_offload_lp_mode = 1;
            }
        } else {
            if (mnld_fsm_offload_lp_mode) {
                LOGD("mnld_fsm() leave offload lp mode");
                mnl2hal_request_wakelock();
                gps_control_kernel_wakelock_take();
                mnld_gps_start_nmea_monitor();
                mnld_fsm_offload_lp_mode = 0;
                mnld_fsm_offload_fwk_wakelock = 1;
            }
        }
        mnld_fsm_offload_old_user = user_bitmap;
    }

    switch (gps_state) {
    case MNLD_GPS_STATE_IDLE: {
        fsm_gps_state_idle(event, data1, data2, data3);
        break;
    }
    case MNLD_GPS_STATE_STARTING: {
        fsm_gps_state_starting(event, data1, data2, data3);
        break;
    }
    case MNLD_GPS_STATE_STARTED: {
        fsm_gps_state_started(event, data1, data2, data3);
        break;
    }
    case MNLD_GPS_STATE_STOPPING: {
        fsm_gps_state_stopping(event, data1, data2, data3);
        break;
    }
    case MNLD_GPS_STATE_OFL_STARTING: {
        fsm_gps_state_ofl_starting(event, data1, data2, data3);
        break;
    }
    case MNLD_GPS_STATE_OFL_STARTED: {
        fsm_gps_state_ofl_started(event, data1, data2, data3);
        break;
    }
    case MNLD_GPS_STATE_OFL_STOPPING: {
        fsm_gps_state_ofl_stopping(event, data1, data2, data3);
        break;
    }
#if defined(GPS_SUSPEND_SUPPORT)
    case MNLD_GPS_STATE_SUSPEND: {
        fsm_gps_state_suspend(event, data1, data2, data3);
        break;
    }
#endif
    default: {
        LOGE("mnld_fsm() unexpected gps_state=%d", gps_state);
        CRASH_TO_DEBUG();
        break;
    }
    }
}

/*****************************************
HAL -> MNL
*****************************************/
static void hal_reboot() {
    LOGW("hal_reboot");
    mnld_gps_client* hal = &g_mnld_ctx.gps_status.clients.hal;
    hal->gps_used = false;
    hal->need_open_ack = false;
    hal->need_close_ack = false;
    hal->need_reset_ack = false;
    mnld_fsm(GPS_EVENT_STOP, 0, 0, NULL);
}

static void hal_gps_init() {
    LOGW("hal_gps_init");
    g_mnld_ctx.gps_status.is_gps_init = true;
    mnl2agps_gps_init();
}

static void hal_gps_start() {
    LOGW("hal_gps_start");
    mnld_gps_client* hal = &g_mnld_ctx.gps_status.clients.hal;
    hal->gps_used = true;
    hal->need_open_ack = true;
    hal->need_close_ack = false;
    hal_start_gps_trigger_epo_download();
    mnld_fsm(GPS_EVENT_START, 0, 0, NULL);
}

static void hal_gps_stop() {
    LOGW("hal_gps_stop");
    mnld_gps_client* hal = &g_mnld_ctx.gps_status.clients.hal;
    hal->gps_used = false;
    hal->need_open_ack = false;
    hal->need_close_ack = true;
    mnld_fsm(GPS_EVENT_STOP, 0, 0, NULL);
}

static void hal_gps_cleanup() {
    LOGW("hal_gps_cleanup");
    g_mnld_ctx.gps_status.is_gps_init = false;
    mnl2agps_gps_cleanup();
}

static void hal_raw_meas_gps_start() {
    LOGW("hal_raw_meas_gps_start");
    mnld_gps_client* raw_meas = &g_mnld_ctx.gps_status.clients.raw_meas;
    raw_meas->gps_used = true;
    raw_meas->need_open_ack = true;
    raw_meas->need_close_ack = false;
    mnld_fsm(GPS_EVENT_START, 0, 0, NULL);
}

static void hal_raw_meas_gps_stop() {
    LOGW("hal_raw_meas_gps_stop");
    mnld_gps_client* raw_meas = &g_mnld_ctx.gps_status.clients.raw_meas;
    raw_meas->gps_used = false;
    raw_meas->need_open_ack = false;
    raw_meas->need_close_ack = true;
    mnld_fsm(GPS_EVENT_STOP, 0, 0, NULL);
}

static void hal_gps_inject_time(int64_t time, int64_t time_reference, int uncertainty) {
    LOGD("hal_gps_inject_time  time=%llu time_reference=%llu uncertainty=%d",
        time, time_reference, uncertainty);
    // TODO libmnl.so
    ntp_context  ntp_inject;

    memset(&ntp_inject, 0, sizeof(ntp_context));
    ntp_inject.time = time;
    ntp_inject.timeReference = time_reference;
    ntp_inject.uncertainty = uncertainty;
    if (mnld_is_gps_started_done()) {
        mtk_gps_inject_ntp_time((MTK_GPS_NTP_T*)&ntp_inject);
    }
}

static void hal_gps_inject_location(double lat, double lng, float acc) {
    int ret = 0;
    MTK_GPS_NLP_T nlp_inject;
    nlp_context context;
    bool alt_valid = false;
    float altitude = 0.0f;
    bool source_valid = true;
    bool source_gnss = false;
    bool source_nlp = true;
    bool source_sensor = false;

    // LOGW("lat=%f lng=%f acc=%f", lat, lng, acc);
    memset(&nlp_inject, 0, sizeof(MTK_GPS_NLP_T));
    memset(&context, 0, sizeof(nlp_context));
    if (clock_gettime(CLOCK_MONOTONIC , &context.ts) == -1) {
        LOGE("clock_gettime failed reason=[%s]\n", strerror(errno));
        return;
    }
    nlp_inject.accuracy = acc;
    nlp_inject.lattidude = lat;
    nlp_inject.longitude = lng;
    nlp_inject.timeReference[0] = (UINT32)context.ts.tv_sec;
    nlp_inject.timeReference[1] = (UINT32)context.ts.tv_nsec;
    nlp_inject.started = mnld_is_gps_started_done();
    nlp_inject.type = 0;
    if (mnld_is_gps_started_done()) {
        mtk_gps_inject_nlp_location(&nlp_inject);
    }

    mnld_nfw_mnl2agps_location_sync(lat, lng, acc, alt_valid, altitude, source_valid, source_gnss, source_nlp, source_sensor);
}

static void hal_gps_delete_aiding_data(int flags) {
    LOGW("hal_gps_delete_aiding_data  flags=0x%x", flags);
    mnl2agps_delete_aiding_data(flags);
    mnld_gps_client* hal = &g_mnld_ctx.gps_status.clients.hal;
    hal->need_reset_ack = false;    // HAL no need the ACK
    g_mnld_ctx.gps_status.delete_aiding_flag |= flags;
    mnld_fsm(GPS_EVENT_RESET, 0, 0, NULL);
}

static void hal_gps_set_position_mode(gps_pos_mode mode, gps_pos_recurrence recurrence,
        int min_interval, int preferred_acc, int preferred_time, bool lowPowerMode) {
    LOGD("hal_gps_set_position_mode  mode=%d recurrence=%d min_interval=%d preferred_acc=%d preferred_time=%d lowPowerMode=%d",
        mode, recurrence, min_interval, preferred_acc, preferred_time, lowPowerMode);
    mnl2agps_set_position_mode(mode);
}

static void hal_data_conn_open(const char* apn) {
    LOGD("hal_data_conn_open");
    mnl2agps_data_conn_open(apn);
}

static void hal_data_conn_open_with_apn_ip_type(uint64_t network_handle,
        const char* apn, apn_ip_type ip_type) {
    LOGD("hal_data_conn_open_with_apn_ip_type network_handle=%llu apn=%s ip_type=%d",
            network_handle, apn, ip_type);
    mnl2agps_data_conn_open_ip_type(apn, ip_type, true, network_handle);
}

static void hal_data_conn_closed() {
    LOGD("hal_data_conn_closed");
    mnl2agps_data_conn_closed();
}

static void hal_data_conn_failed() {
    LOGD("hal_data_conn_failed");
    mnl2agps_data_conn_failed();
}

static void hal_set_server(agps_type type, const char* hostname, int port) {
    LOGD("hal_set_server");
    mnl2agps_set_server(type, hostname, port);
}

static void hal_set_ref_location(cell_type type, int mcc, int mnc, int lac, int cid) {
    LOGD("hal_set_ref_location");
    mnl2agps_set_ref_loc(type, mcc, mnc, lac, cid);
}

static void hal_set_id(agps_id_type type, const char* setid) {
    LOGD("hal_set_id");
    mnl2agps_set_set_id(type, setid);
}

static void hal_ni_message(char* msg, int len) {
    LOGD("hal_ni_message, len=%d", len);
    mnl2agps_ni_message(msg, len);
}

static void hal_ni_respond(int notif_id, ni_user_response_type user_response) {
    LOGD("hal_ni_respond  notif_id=%d user_response=%d", notif_id, user_response);
    mnl2agps_ni_respond(notif_id, user_response);
}

static void hal_update_network_state(uint64_t network_handle, bool is_connected,
            unsigned short capabilities, const char* apn) {
    LOGD("hal_update_network_state  networkHandle=%llu isConnected=%d capabilities=%d apn=[%s]",
        network_handle, is_connected, capabilities, apn);
    /***Remove network state update to AGPSD, due to this msg type never used in AGPSD***/
    //mnl2agps_update_network_state(connected, type, roaming, extra_info);
    mnl2agps_update_network_state_with_handle(network_handle, is_connected, capabilities, apn);
    mnl_epo_status* epo_status = &g_mnld_ctx.epo_status;
    if(is_connected) {
        qepo_invalid_dl_cnt_clear();
    }
    epo_status->is_network_connected = is_connected;
    if ((capabilities & NOT_METERED) == NOT_METERED) {
        epo_status->is_wifi_connected = is_connected;
    } else {
        epo_status->is_wifi_connected = false;
    }

    if (mnld_is_gps_or_ofl_started()) {
        epo_read_cust_config();
        if (is_connected && epo_status->is_epo_downloading == false &&
            epo_downloader_is_file_invalid() && epo_is_epo_download_enabled()) {
            epo_status->is_epo_downloading = true;
            epo_downloader_start();
        }
    } else if ((capabilities & NOT_METERED) == NOT_METERED) {
        epo_read_cust_config();
        if (is_connected && epo_status->is_epo_downloading == false && epo_is_wifi_trigger_enabled() &&
            epo_downloader_is_file_invalid() && epo_is_epo_download_enabled()) {
            epo_status->is_epo_downloading = true;
            epo_downloader_start();
        }
    }
}

void hal_start_gps_trigger_epo_download() {
    mnl_epo_status* epo_status = &g_mnld_ctx.epo_status;
    LOGD_ENG("is_network_connected=%d,is_epo_downloading=%d",
      epo_status->is_network_connected, epo_status->is_epo_downloading);
    epo_read_cust_config();
    if (epo_status->is_epo_downloading == false
        && epo_downloader_is_file_invalid() && epo_is_epo_download_enabled()) {
        epo_status->is_epo_downloading = true;
        epo_downloader_start();
    }
}

bool is_network_connected() {
    mnl_epo_status* epo_status = &g_mnld_ctx.epo_status;
    return epo_status->is_network_connected;
}

bool is_wifi_network_connected() {
    mnl_epo_status* epo_status = &g_mnld_ctx.epo_status;
    return epo_status->is_wifi_connected;
}

static void hal_update_network_availability(int available, const char* apn) {
    LOGD("hal_update_network_availability  available=%d apn=[%s]", available, apn);
    mnl2agps_update_network_availability(available, apn);
}

static void hal_set_gps_measurement(bool enabled, bool enableFullTracking) {
    LOGD("hal_set_gps_measurement  enabled=%d, flag=%d", enabled, enableFullTracking);

    g_mnld_ctx.gps_status.is_gps_meas_enabled = enabled;
    g_enable_full_tracking = enableFullTracking;
    if (mnld_is_gps_or_ofl_started_done()) {
        int ret = mtk_gps_set_param(MTK_PARAM_CMD_ENABLE_FULL_TRACKING, &g_enable_full_tracking);
        if (ret < 0) {
            LOGW("set measurement parameter fail");
        }
    }

    if(enabled) {
        hal_raw_meas_gps_start();
    } else {
        hal_raw_meas_gps_stop();
    }
}

static void hal_set_gps_navigation(bool enabled) {
    LOGD("hal_set_gps_navigation  enabled=%d", enabled);
    g_mnld_ctx.gps_status.is_gps_navi_enabled = enabled;
}

static void hal_set_vzw_debug(bool enabled) {
    LOGD("hal_set_vzw_debug  enabled=%d", enabled);
    mnl2agps_vzw_debug_screen_enable(enabled);
}

static void hal_update_gnss_config(const char* config_data, int length) {
    int ret = mnl2agps_update_configuration(config_data, length);
    LOGD("hal_update_gnss_config  length=%d, ret=%d", length, ret);
}

static void hal_set_sv_blacklist(long long* blacklist, int size) {
    memset(svBlacklist.sv_list, 0, sizeof(svBlacklist.sv_list));
    memcpy(svBlacklist.sv_list, blacklist, sizeof(svBlacklist.sv_list));

    if(mnld_is_gps_or_ofl_started_done()) {
        LOGD("hal_set_sv_blacklist to mnl");
        int ret = mtk_gps_set_param(MTK_PARAM_CMD_SET_SV_BLACKLIST, &svBlacklist);    //set to libmnl in run time
        if (ret < 0) {
            LOGW("set SV blacklist parameter fail");
        }
    } else {
        LOGD("hal_set_sv_blacklist gps not start");
    }
    for(int i = 0; i < size; i++) {
        LOGD("hal_set_sv_blacklist constellation:%d, svidlist:0x%llx", i, blacklist[i]);
    }
}

static void hal_set_correction(measurement_corrections* corrections) {
    int idx = 0;
    MTK_GPS_PARAM_MEAS_CORR meas_correction;

    memset(&meas_correction, 0, sizeof(MTK_GPS_PARAM_MEAS_CORR));
    memcpy(&meas_correction, corrections, sizeof(MTK_GPS_PARAM_MEAS_CORR));

    LOGD("CORRECTIONS: lat(%f), lon(%f), alt(%f), hUnc(%f), vUncs(%f), toa(%lld), svcnt(%lld)",
        meas_correction.latitudeDegrees, meas_correction.longitudeDegrees, meas_correction.altitudeMeters, meas_correction.horizontalPositionUncertaintyMeters, meas_correction.verticalPositionUncertaintyMeters, meas_correction.toaGpsNanosecondsOfWeek, meas_correction.num_satCorrection);
    if(meas_correction.num_satCorrection > MTK_MNLD_GNSS_MAX_SVS) {
        meas_correction.num_satCorrection = MTK_MNLD_GNSS_MAX_SVS;
    }
    for(idx = 0; idx < meas_correction.num_satCorrection; idx++) {
        LOGD_ENG("SV CORRETION[%d]:Flags(0x%x), constellation(%d), svid(%d), cFHz(%f), probLos(%f), ePL(%f), ePLUnc(%f)",
            idx, meas_correction.satCorrections[idx].singleSatCorrectionFlags, meas_correction.satCorrections[idx].constellation, meas_correction.satCorrections[idx].svid, meas_correction.satCorrections[idx].carrierFrequencyHz,
            meas_correction.satCorrections[idx].probSatIsLos, meas_correction.satCorrections[idx].excessPathLengthMeters, meas_correction.satCorrections[idx].excessPathLengthUncertaintyMeters);
        LOGD_ENG("SV CORRETION[%d] Reflecting Plane:lat(%f), lgt(%f), alt(%f), azim(%f)",
            idx, meas_correction.satCorrections[idx].reflectingPlane.latitudeDegrees, meas_correction.satCorrections[idx].reflectingPlane.longitudeDegrees, meas_correction.satCorrections[idx].reflectingPlane.altitudeMeters, meas_correction.satCorrections[idx].reflectingPlane.azimuthDegrees);
    }
    if (mnld_is_gps_or_ofl_started_done()) {
        INT32 ret = 0;
        ret = mtk_gps_set_param(MTK_PARAM_CMD_CONFIG_BLUE_MEAS_CORR, &meas_correction);
        if (ret < 0) {
            LOGW("set correction fail");
        }
    }
}

/*Update location visibility of nfw user(s) to bool arry g_mnld_ctx.nfw_user_visibility*/
static void mnld_nfw_visiblity_update(char* proxyApps, int length) {
    char *app_name_start = proxyApps;
    int app_name_len = 0;
    int idx = 0;
    int set_app_idx = 0;

    memset(g_mnld_ctx.nfw_user_visibility, 0, sizeof(g_mnld_ctx.nfw_user_visibility));  //All NFW app users are not allowed to get GPS location as default.
    if(proxyApps == NULL) {
        memset(nfw_app_name_list[set_app_idx], 0, sizeof(nfw_app_name_list[set_app_idx]));
        memset(nfw_user_notify[set_app_idx].proxyAppPackageName, 0, sizeof(nfw_user_notify[set_app_idx].proxyAppPackageName));
        memset(nfw_user_notify_default[set_app_idx].proxyAppPackageName, 0, sizeof(nfw_user_notify_default[set_app_idx].proxyAppPackageName));
        LOGD("proxyApps NULL, Set all NFW user denied!!!");
    } else if((length == 0) || (strlen(proxyApps) == 0)) {
        LOGD("Set all NFW user denied!!!");
    } else {
        if(length > MNLD_NFW_USER_NAME_LEN*MNLD_NFW_USER_NUM_MAX) {
            length = MNLD_NFW_USER_NAME_LEN*MNLD_NFW_USER_NUM_MAX;
        }
        app_name_start = proxyApps;
        for(idx = 0; idx <= length; idx++) {
            if(proxyApps[idx] == ' ' || proxyApps[idx] == '\0') {  //APP name divided by ' '
                int app_idx = 0;
                app_name_len = &proxyApps[idx] - app_name_start;  //calculate app name length by poiter shift.
                if(app_name_len >= MNLD_NFW_USER_NAME_LEN) {
                    app_name_len = MNLD_NFW_USER_NAME_LEN - 1;
                }

#if 0
                for(app_idx = 0; app_idx < MNLD_NFW_USER_NUM; app_idx++) {
                    if((app_name_len == strlen(nfw_app_name_list[app_idx]))
                        && (!strncmp(app_name_start, nfw_app_name_list[app_idx], strlen(nfw_app_name_list[app_idx])))) {  //APP name matched
                        LOGD("Set visibility for NFW User[%d]: %s, len:%d", app_idx, nfw_app_name_list[app_idx], app_name_len);
                        g_mnld_ctx.nfw_user_visibility[app_idx] = true;  //Set visibility
                    }
                }
#else
                //Ignore proxy package name comparing, use visibility update package name as notify name
                MNLD_STRNCPY(nfw_app_name_list[set_app_idx], app_name_start, app_name_len+1);
                MNLD_STRNCPY(nfw_user_notify[set_app_idx].proxyAppPackageName, app_name_start, app_name_len+1);
                MNLD_STRNCPY(nfw_user_notify_default[set_app_idx].proxyAppPackageName, app_name_start, app_name_len+1);
                g_mnld_ctx.nfw_user_visibility[set_app_idx] = true;  //Set visibility
                LOGD("Set NFW user[%d] allowed:%s", set_app_idx, nfw_app_name_list[set_app_idx]);
#endif

                if(proxyApps[idx] == ' ') {
                    set_app_idx++;
                    app_name_start = &proxyApps[idx+1];  //Point to the next APP name
                }
            }
        }
    }
}

static void hal_set_nfw_access(char* proxyApps, int length) {
    if(proxyApps != NULL) {
        LOGD("hal_set_nfw_access  length=%d, proxyApps:%s, nlp visibility ctrl:%d, gnss visibility ctrl:%d ", length, proxyApps, mnld_nfw_ctrl_nlp_enabled(), mnld_nfw_ctrl_gnss_enabled());
    } else {
        LOGD("hal_set_nfw_access length=%d, proxyApps:NULL, nlp visibility ctrl:%d, gnss visibility ctrl:%d ", length, mnld_nfw_ctrl_nlp_enabled(), mnld_nfw_ctrl_gnss_enabled());
    }

    if(mnld_nfw_ctrl_nlp_enabled() || mnld_nfw_ctrl_gnss_enabled()) {
        mnld_nfw_visiblity_update(proxyApps, length);
    } else {
        mtk_gps_set_nfw_visibility_all(true);
    }
    mnl2agps_update_gnss_access_control(g_mnld_ctx.nfw_user_visibility[MNLD_NFW_USER_AGPS]);
}

static hal2mnl_interface g_hal2mnl_interface = {
    hal_reboot,
    hal_gps_init,
    hal_gps_start,
    hal_gps_stop,
    hal_gps_cleanup,
    hal_gps_inject_time,
    hal_gps_inject_location,
    hal_gps_delete_aiding_data,
    hal_gps_set_position_mode,
    hal_data_conn_open,
    hal_data_conn_open_with_apn_ip_type,
    hal_data_conn_closed,
    hal_data_conn_failed,
    hal_set_server,
    hal_set_ref_location,
    hal_set_id,
    hal_ni_message,
    hal_ni_respond,
    hal_update_network_state,
    hal_update_network_availability,
    hal_set_gps_measurement,
    hal_set_gps_navigation,
    hal_set_vzw_debug,
    hal_update_gnss_config,
    hal_set_sv_blacklist,
    hal_set_correction,
    hal_set_nfw_access,
};

/*****************************************
AGPSD -> MNL
*****************************************/

static void agps_reboot() {
    LOGW("agps_reboot");
    mnld_gps_client* agps = &g_mnld_ctx.gps_status.clients.agps;
    agps->gps_used = false;
    agps->need_open_ack = false;
    agps->need_close_ack = false;
    agps->need_reset_ack = false;
    mnl2agps_update_gnss_access_control(g_mnld_ctx.nfw_user_visibility[MNLD_NFW_USER_AGPS]);
    mnld_fsm(GPS_EVENT_STOP, 0, 0, NULL);
}

static nfw_protocal_stack mnld_nfw_agps_protocal_stack(mnl_agps_open_type open_type) {
    switch(open_type) {
        case MNL_AGPS_OPEN_TYPE_SUPL:
            return NFW_PS_SUPL;

        case MNL_AGPS_OPEN_TYPE_CP_NILR:
        case MNL_AGPS_OPEN_TYPE_CP_MTLR:
        case MNL_AGPS_OPEN_TYPE_CP_MOLR:
        case MNL_AGPS_OPEN_TYPE_CP_QUERY:
        case MNL_AGPS_OPEN_TYPE_CP_MLR:
            return NFW_PS_CTRL_PLANE;

        case MNL_AGPS_OPEN_TYPE_UNKNOWN:
        case MNL_AGPS_OPEN_TYPE_C2K:
        default :
            return NFW_PS_OTHER;
    }
}

void mnld_nfw_set_default_notification(MNLD_NFW_USER usr) {
    if(usr < MNLD_NFW_USER_NUM) {
        memset(&nfw_user_notify[usr], 0, sizeof(nfw_notification));
        memcpy(&nfw_user_notify[usr], &nfw_user_notify_default[usr], sizeof(nfw_notification));
    } else {
        LOGW("[NFW]set default wrong user:%d", usr);
    }
}

void mnld_nfw_notify2hal_default(MNLD_NFW_USER usr, nfw_response_type response_type) {
    nfw_user_notify_default[usr].responseType = response_type;
    LOGD("user[%d]:%s, ps:%d, otherPSName:%s, requestor:%d, requestorID:%s, responseType:%d, inEmergencyMode:%d, cashed:%d",
        usr, nfw_user_notify_default[usr].proxyAppPackageName,
        nfw_user_notify_default[usr].protocolStack, nfw_user_notify_default[usr].otherProtocolStackName,
        nfw_user_notify_default[usr].requestor, nfw_user_notify_default[usr].requestorId,
        nfw_user_notify_default[usr].responseType, nfw_user_notify_default[usr].inEmergencyMode, nfw_user_notify_default[usr].isCachedLocation);
    if(mnl2hal_nfw_notify(nfw_user_notify_default[usr]) == -1) {
        LOGW("nfw(%d) notify to hal fail!!!", usr);
    }
    g_mnld_nfw_notified[usr] = true;
}

static void mnld_nfw_update_notification(MNLD_NFW_USER usr, mnl_agps_open_type open_type,
        mnl_agps_open_requestor requestor_type, const char* requestor_id,
        bool emergency_call){
    if(usr < MNLD_NFW_USER_NUM) {
        memset(&nfw_user_notify[usr], 0, sizeof(nfw_user_notify[usr]));
        MNLD_STRNCPY(nfw_user_notify[usr].proxyAppPackageName, nfw_app_name_list[usr], MNLD_NFW_USER_NAME_LEN);
        nfw_user_notify[usr].protocolStack = mnld_nfw_agps_protocal_stack(open_type);
        if(open_type == MNL_AGPS_OPEN_TYPE_C2K) {
            MNLD_STRNCPY(nfw_user_notify[usr].otherProtocolStackName, "C2K", MTK_MNLD_STRING_MAXLEN);
        }
        nfw_user_notify[usr].requestor = requestor_type;
        MNLD_STRNCPY(nfw_user_notify[usr].requestorId, requestor_id, MTK_MNLD_STRING_MAXLEN);
        nfw_user_notify[usr].inEmergencyMode = emergency_call;
        nfw_user_notify[usr].isCachedLocation = 0;
    } else {
        LOGW("[NFW]update wrong user:%d", usr);
    }
}

void mnld_nfw_notify2hal(MNLD_NFW_USER usr, nfw_response_type response_type) {
    nfw_user_notify[usr].responseType = response_type;
    LOGD_ENG("user[%d]:%s, ps:%d, otherPSName:%s, requestor:%d, requestorID:%s, responseType:%d, inEmergencyMode:%d, cashed:%d",
        usr, nfw_user_notify[usr].proxyAppPackageName,
        nfw_user_notify[usr].protocolStack, nfw_user_notify[usr].otherProtocolStackName,
        nfw_user_notify[usr].requestor, nfw_user_notify[usr].requestorId,
        nfw_user_notify[usr].responseType, nfw_user_notify[usr].inEmergencyMode, nfw_user_notify[usr].isCachedLocation);
    if(mnl2hal_nfw_notify(nfw_user_notify[usr]) == -1) {
        LOGW("nfw(%d) notify to hal fail!!!", usr);
    }
    g_mnld_nfw_notified[usr] = true;
}

void mnld_nfw_mnl2agps_location_sync(double lat, double lng, int acc, bool alt_valid, float alt, bool source_valid, bool source_gnss, bool source_nlp, bool source_sensor)
{
    int ret = 0;

    if((source_nlp && mnld_nfw_ctrl_nlp_enabled()) || (source_gnss && mnld_nfw_ctrl_gnss_enabled())) {
        if (mtk_gps_get_nfw_visibility(MNLD_NFW_USER_AGPS)) {
            ret = mnl2agps_location_sync(lat, lng, acc, alt_valid, alt, source_valid, source_gnss, source_nlp, source_sensor);
            LOGD("nfw ret = %d\n", ret);
            if (0 == ret) {
                LOGD("mnl2agps_location_sync success\n");
            }
            if(mtk_gps_get_gps_user() & GPS_USER_AGPS) {  //AGPS in GPS user list
                //Notify HAL allowed AGPS open GPS
                mnld_nfw_notify2hal(MNLD_NFW_USER_AGPS, NFW_RESPONSE_TYPE_ACCEPTED_LOCATION_PROVIDED);
            } else {
                mnld_nfw_notify2hal_default(MNLD_NFW_USER_AGPS, NFW_RESPONSE_TYPE_ACCEPTED_LOCATION_PROVIDED);
            }
        }
    } else {
        ret = mnl2agps_location_sync(lat, lng, acc, alt_valid, alt, source_valid, source_gnss, source_nlp, source_sensor);
        LOGD("ret = %d\n", ret);
        if (0 == ret) {
            LOGD("mnl2agps_location_sync success\n");
        }
    }
}


int g_mnld_show_icon = 1;   //Default show GPS icon(notify to framework), unless AGPS notify fobid to show.
int mnld_show_icon_get(void) {
    return g_mnld_show_icon;
}

void mnld_show_icon_set(int show_gps_icon) {
    g_mnld_show_icon = show_gps_icon;
}

static void agps_open_gps_req(int show_gps_icon, mnl_agps_open_type open_type,
        mnl_agps_open_requestor requestor_type, const char* requestor_id,
        bool emergency_call) {
    LOGW("agps_open_gps_req  show_gps_icon=%d open_type=%d requestor_type=%d requestor_id=%s emergency_call=%d, allowed:%d\n",
            show_gps_icon, open_type, requestor_type, requestor_id, emergency_call, mtk_gps_get_nfw_visibility(MNLD_NFW_USER_AGPS));
    mnld_show_icon_set(show_gps_icon);
    if(MNL_AGPS_OPEN_REQUESTOR_UNKNOWN == requestor_type) {//i.e. old AGPSD
        LOGW("May be using an old AGPSD!!!");
        mnld_nfw_set_default_notification(MNLD_NFW_USER_AGPS);
    } else {
        mnld_nfw_update_notification(MNLD_NFW_USER_AGPS, open_type, requestor_type, requestor_id, emergency_call);
    }
    mnld_gps_client* agps = &g_mnld_ctx.gps_status.clients.agps;
    agps->gps_used = true;
    agps->need_open_ack = true;
    agps->need_close_ack = false;
    mnld_fsm(GPS_EVENT_START, 0, 0, NULL);
    g_mnld_nfw_notified[MNLD_NFW_USER_AGPS] = false;
}

static void agps_close_gps_req() {
    LOGW("agps_close_gps_req, notified:%d", g_mnld_nfw_notified[MNLD_NFW_USER_AGPS]);
    mnld_gps_client* agps = &g_mnld_ctx.gps_status.clients.agps;
    agps->gps_used = false;
    agps->need_open_ack = false;
    agps->need_close_ack = true;
    mnld_fsm(GPS_EVENT_STOP, 0, 0, NULL);
    if(g_mnld_nfw_notified[MNLD_NFW_USER_AGPS] == false) {  //Never notified to HAL in one session
        mnld_nfw_notify2hal(MNLD_NFW_USER_AGPS, NFW_RESPONSE_TYPE_ACCEPTED_NO_LOCATION_PROVIDED);
    }
    mnld_nfw_set_default_notification(MNLD_NFW_USER_AGPS);
    mnld_show_icon_set(1);
}

static void agps_reset_gps_req(int flags) {
    LOGW("agps_reset_gps_req  flags=0x%x", flags);
    if (flags == 0) {
        mnl2agps_reset_gps_done();
        return;
    }
    g_mnld_ctx.gps_status.delete_aiding_flag |= flags;
    mnld_gps_client* agps = &g_mnld_ctx.gps_status.clients.agps;
    agps->need_reset_ack = true;
    mnld_fsm(GPS_EVENT_RESET, 0, 0, NULL);
}

static void agps_open_gps_rejected(mnl_agps_open_type open_type,
        mnl_agps_open_requestor requestor_type, const char* requestor_id,
        bool emergency_call) {
    LOGW("agps_open_gps_rejected  open_type=%d requestor_type=%d requestor_id=%s emergency_call=%d allowed=%d\n",
            open_type, requestor_type, requestor_id, emergency_call, mtk_gps_get_nfw_visibility(MNLD_NFW_USER_AGPS));

    mnld_nfw_update_notification(MNLD_NFW_USER_AGPS, open_type, requestor_type, requestor_id, emergency_call);
    mnld_nfw_notify2hal(MNLD_NFW_USER_AGPS, NFW_RESPONSE_TYPE_REJECTED);
}

static void agps_session_done() {
    LOGW("agps_session_done");
    // TODO libmnl.so
    gps_controller_agps_session_done();
}

static void agps_ni_notify(int session_id, mnl_agps_ni_type ni_type,
    mnl_agps_notify_type type, const char* requestor_id, const char* client_name) {
    LOGD("agps_ni_notify  session_id=%d type=%d requestor_id=[%s] client_name=[%s]",
        session_id, type, requestor_id, client_name);
    int usc2_len;
    char ucs2_buff[1024];
    char requestorId[1024] = {0};
    char clientName[1024] = {0};

    memset(ucs2_buff, 0, sizeof(ucs2_buff));
    usc2_len = asc_str_to_usc2_str(ucs2_buff, requestor_id);
    raw_data_to_hex_string(requestorId, ucs2_buff, usc2_len);

    memset(ucs2_buff, 0, sizeof(ucs2_buff));
    usc2_len = asc_str_to_usc2_str(ucs2_buff, client_name);
    raw_data_to_hex_string(clientName, ucs2_buff, usc2_len);

    mnl2hal_request_ni_notify(session_id, ni_type, type, requestorId, clientName,
        NI_ENCODING_TYPE_UCS2, NI_ENCODING_TYPE_UCS2);
}

static void agps_data_conn_req(int ipaddr, int is_emergency) {
    LOGD("agps_data_conn_req  ipaddr=0x%x is_emergency=%d", ipaddr, is_emergency);
    UNUSED(is_emergency);
    struct sockaddr_storage addr;
    memset(&addr, 0, sizeof(addr));
    struct sockaddr_in *in = (struct sockaddr_in*)&addr;
    addr.ss_family = AF_INET;
    in->sin_addr.s_addr = ipaddr;
    mnl2hal_request_data_conn(addr, MTK_AGPS_TYPE_SUPL);
}

static void agps_data_conn_release(mnl_agps_data_connection_type agps_type) {
    LOGD("agps_data_conn_release agnss_type=%d", agps_type);
    mnl2hal_release_data_conn(agps_type);
}

static void agps_set_id_req(int flags) {
    LOGD("agps_set_id_req  flags=%d", flags);
    mnl2hal_request_set_id(flags);
}

static void agps_ref_loc_req(int flags) {
    LOGD("agps_ref_loc_req  flags=%d", flags);
    mnl2hal_request_ref_loc(flags);
}

static void agps_rcv_pmtk(const char* pmtk) {
    // LOGD("agps_rcv_pmtk  pmtk=%s", pmtk);
    // TODO libmnl.so
    gps_controller_rcv_pmtk(pmtk);
}

static void agps_gpevt(gpevt_type type) {
    LOGD("agps_gpevt  type=%d", type);
    UNUSED(type);
}
static void agps_rcv_lppe_common_iono(const char* data, int len)
{
    LOGD("rcv_lppe_data\n");
    if (mnld_is_gps_and_ofl_stopped()) {
        LOGD("rcv_lppe_data: MNL stopped, return");
        return;
    }
    int ret = mtk_agps_set_param_with_payload_len(MTK_MSG_AGPS_MSG_SUPL_LPPE_ASSIST_COMMON_IONO, data, MTK_MOD_DISPATCHER, MTK_MOD_AGENT,len);

    if (ret != 0) {
        LOGD("mtk_agps_set_param fail, MTK_MSG_AGPS_MSG_SUPL_LPPE\n");
    }else{
          gnss_ha_assist_ack_struct ack;
          memset((char*)(&ack),0x0,sizeof(gnss_ha_assist_ack_struct));
          ack.type=AGPS_MD_HUGE_DATA_TYPE_LPPE_AGNSS_PROVIDE_ASSIST_DATA_COMMON_IONO;
          mnl2agps_lppe_assist_data_provide_ack((char*)(&ack), sizeof(gnss_ha_assist_ack_struct));
    }
}
static void agps_rcv_lppe_common_trop(const char* data, int len)
{
    LOGD("rcv_lppe_data\n");
    if (mnld_is_gps_and_ofl_stopped()) {
        LOGD("rcv_lppe_data: MNL stopped, return");
        return;
    }
    int ret = mtk_agps_set_param_with_payload_len(MTK_MSG_AGPS_MSG_SUPL_LPPE_ASSIST_COMMON_TROP, data, MTK_MOD_DISPATCHER, MTK_MOD_AGENT,len);
    if (ret != 0) {
        LOGD("mtk_agps_set_param fail, MTK_MSG_AGPS_MSG_SUPL_LPPE\n");
    }else{
         gnss_ha_assist_ack_struct ack;
         memset((char*)(&ack),0x0,sizeof(gnss_ha_assist_ack_struct));
         ack.type=AGPS_MD_HUGE_DATA_TYPE_LPPE_AGNSS_PROVIDE_ASSIST_DATA_COMMON_TROP;
         mnl2agps_lppe_assist_data_provide_ack((char*)(&ack), sizeof(gnss_ha_assist_ack_struct));
    }

}
static void agps_rcv_lppe_common_alt(const char* data, int len)
{
    LOGD("rcv_lppe_data\n");
    if (mnld_is_gps_and_ofl_stopped()) {
        LOGD("rcv_lppe_data: MNL stopped, return");
        return;
    }
    int ret = mtk_agps_set_param_with_payload_len(MTK_MSG_AGPS_MSG_SUPL_LPPE_ASSIST_COMMON_ALT, data, MTK_MOD_DISPATCHER, MTK_MOD_AGENT,len);
    if (ret != 0) {
        LOGD("mtk_agps_set_param fail, MTK_MSG_AGPS_MSG_SUPL_LPPE\n");
    }else{
         gnss_ha_assist_ack_struct ack;
         memset((char*)(&ack),0x0,sizeof(gnss_ha_assist_ack_struct));
         ack.type=AGPS_MD_HUGE_DATA_TYPE_LPPE_AGNSS_PROVIDE_ASSIST_DATA_COMMON_ALT;
         mnl2agps_lppe_assist_data_provide_ack((char*)(&ack), sizeof(gnss_ha_assist_ack_struct));
    }

}
static void agps_rcv_lppe_common_solar(const char* data, int len)
{
     LOGD("rcv_lppe_data\n");
    if (mnld_is_gps_and_ofl_stopped()) {
        LOGD("rcv_lppe_data: MNL stopped, return");
        return;
    }
    int ret = mtk_agps_set_param_with_payload_len(MTK_MSG_AGPS_MSG_SUPL_LPPE_ASSIST_COMMON_SOLAR, data, MTK_MOD_DISPATCHER, MTK_MOD_AGENT,len);
    if (ret != 0) {
        LOGD("mtk_agps_set_param fail, MTK_MSG_AGPS_MSG_SUPL_LPPE\n");
    }else{
         gnss_ha_assist_ack_struct ack;
         memset((char*)(&ack),0x0,sizeof(gnss_ha_assist_ack_struct));
         ack.type=AGPS_MD_HUGE_DATA_TYPE_LPPE_AGNSS_PROVIDE_ASSIST_DATA_COMMON_SOLAR;
         mnl2agps_lppe_assist_data_provide_ack((char*)(&ack), sizeof(gnss_ha_assist_ack_struct));
    }

}
static void agps_rcv_lppe_common_ccp(const char* data, int len)
{
    LOGD("rcv_lppe_data\n");
    if (mnld_is_gps_and_ofl_stopped()) {
        LOGD("rcv_lppe_data: MNL stopped, return");
        return;
    }
    int ret = mtk_agps_set_param_with_payload_len(MTK_MSG_AGPS_MSG_SUPL_LPPE_ASSIST_COMMON_CCP, data, MTK_MOD_DISPATCHER, MTK_MOD_AGENT,len);
    if (ret != 0) {
        LOGD("mtk_agps_set_param fail, MTK_MSG_AGPS_MSG_SUPL_LPPE\n");
    }else{
         gnss_ha_assist_ack_struct ack;
         memset((char*)(&ack),0x0,sizeof(gnss_ha_assist_ack_struct));
         ack.type=AGPS_MD_HUGE_DATA_TYPE_LPPE_AGNSS_PROVIDE_ASSIST_DATA_COMMON_CCP;
         mnl2agps_lppe_assist_data_provide_ack((char*)(&ack), sizeof(gnss_ha_assist_ack_struct));
    }

}
static void agps_rcv_lppe_generic_ccp(const char* data, int len)
{
    LOGD("rcv_lppe_data\n");
    if (mnld_is_gps_and_ofl_stopped()) {
        LOGD("rcv_lppe_data: MNL stopped, return");
        return;
    }
    int ret = mtk_agps_set_param_with_payload_len(MTK_MSG_AGPS_MSG_SUPL_LPPE_ASSIST_GENERIC_CCP, data, MTK_MOD_DISPATCHER, MTK_MOD_AGENT,len);
    if (ret != 0) {
        LOGD("mtk_agps_set_param fail, MTK_MSG_AGPS_MSG_SUPL_LPPE\n");
    }else{
         gnss_ha_assist_ack_struct ack;
         memset((char*)(&ack),0x0,sizeof(gnss_ha_assist_ack_struct));
         ack.type=AGPS_MD_HUGE_DATA_TYPE_LPPE_AGNSS_PROVIDE_ASSIST_DATA_GENERIC_CCP;
         mnl2agps_lppe_assist_data_provide_ack((char*)(&ack), sizeof(gnss_ha_assist_ack_struct));
    }

}
static void agps_rcv_lppe_generic_dm(const char* data, int len)
{
    LOGD("rcv_lppe_data\n");
    if (mnld_is_gps_and_ofl_stopped()) {
        LOGD("rcv_lppe_data: MNL stopped, return");
        return;
    }
    int ret = mtk_agps_set_param_with_payload_len(MTK_MSG_AGPS_MSG_SUPL_LPPE_ASSIST_GENERIC_DM, data, MTK_MOD_DISPATCHER, MTK_MOD_AGENT,len);
    if (ret != 0) {
        LOGD("mtk_agps_set_param fail, MTK_MSG_AGPS_MSG_SUPL_LPPE\n");
    }else{
         gnss_ha_assist_ack_struct ack;
         memset((char*)(&ack),0x0,sizeof(gnss_ha_assist_ack_struct));
         ack.type=AGPS_MD_HUGE_DATA_TYPE_LPPE_AGNSS_PROVIDE_ASSIST_DATA_GENERIC_DM;
         mnl2agps_lppe_assist_data_provide_ack((char*)(&ack), sizeof(gnss_ha_assist_ack_struct));
    }

}
static void agps_location(mnl_agps_agps_location* location) {
    LOGDX("agps_location  lat,lng %f,%f acc=%f used=%d",
        location->latitude, location->longitude, location->accuracy, location->accuracy_used);

    gps_location loc;
    memset(&loc, 0, sizeof(loc));
    loc.flags |= MTK_GPS_LOCATION_HAS_LAT_LONG;
    loc.lat = location->latitude;
    loc.lng = location->longitude;
    if (location->altitude_used) {
        loc.flags |= MTK_GPS_LOCATION_HAS_ALT;
        loc.alt = location->altitude;
    }
    if (location->speed_used) {
        loc.flags |= MTK_GPS_LOCATION_HAS_SPEED;
        loc.speed = location->speed;
    }
    if (location->bearing_used) {
        loc.flags |= MTK_GPS_LOCATION_HAS_BEARING;
        loc.bearing = location->bearing;
    }
    if (location->accuracy_used) {
        loc.flags |= MTK_GPS_LOCATION_HAS_HORIZONTAL_ACCURACY;
        loc.h_accuracy = location->accuracy;
    }
    if (location->timestamp_used) {
        loc.timestamp = location->timestamp;
    }
    mnl2hal_location(loc);
    MTK_GPS_NLP_T c2k_cell_location;
    nlp_context context;
    memset(&c2k_cell_location, 0, sizeof(MTK_GPS_NLP_T));
    memset(&context, 0, sizeof(nlp_context));
    if (clock_gettime(CLOCK_MONOTONIC , &context.ts) == -1) {
        LOGE("clock_gettime failed reason=[%s]\n", strerror(errno));
        return;
    }
    LOGD("ts.tv_sec = %ld, ts.tv_nsec = %ld\n",
        context.ts.tv_sec, context.ts.tv_nsec);

    c2k_cell_location.lattidude = location->latitude;
    c2k_cell_location.longitude = location->longitude;
    c2k_cell_location.accuracy = location->accuracy;
    c2k_cell_location.timeReference[0] = (UINT32)context.ts.tv_sec;
    c2k_cell_location.timeReference[1] = (UINT32)context.ts.tv_nsec;
    c2k_cell_location.type = 0;
    c2k_cell_location.started = 1;
    LOGDX("inject cell location lati= %f, longi = %f, accuracy = %f\n",
        c2k_cell_location.lattidude, c2k_cell_location.longitude, c2k_cell_location.accuracy);

    if (mnld_is_gps_started_done()) {
        mtk_gps_inject_nlp_location(&c2k_cell_location);
    }
}

static void agps_ni_notify2(int session_id, mnl_agps_ni_type ni_type,
    mnl_agps_notify_type type, const char* requestor_id, const char* client_name,
    mnl_agps_ni_encoding_type requestor_id_encoding,
    mnl_agps_ni_encoding_type client_name_encoding) {
    LOGD("agps_ni_notify2  session_id=%d type=%d requestor_id_encoding=%d client_name_encoding=%d",
        session_id, type, requestor_id_encoding, client_name_encoding);
    mnl2hal_request_ni_notify(session_id, ni_type, type, requestor_id,
        client_name, requestor_id_encoding, client_name_encoding);
}

static void agps_data_conn_req2(struct sockaddr_storage* addr, int is_emergency,
        mnl_agps_data_connection_type agps_type) {
    LOGD("agps_data_conn_req2 is_emergency=%d agnss_type=%d", is_emergency, agps_type);
    UNUSED(is_emergency);
    mnl2hal_request_data_conn(*addr, agps_type);
}

static int get_agnss_capability(const UINT8 sv_type_agps_set, const UINT8 sv_type, char *pmtk_str) {
    char tmp[64]={0};
    int agps_cap = 0;
    int aglonass_cap = 0;
    int abeidou_cap = 0;
    int agalileo_cap = 0;
    int gnss_num = 0;

    // Axxx Enable. with HW support condition
    if (((sv_type_agps_set & 0x21) == 0x21) && ((sv_type & 0x01) == 0x01)) {
        agps_cap = 1;
    }
    if (((sv_type_agps_set & 0x12) == 0x12) && ((sv_type & 0x02) == 0x02)) {
        aglonass_cap = 1;
    }
    if (((sv_type_agps_set & 0x44) == 0x44) && ((sv_type & 0x04) == 0x04)) {
        abeidou_cap = 1;
    }
    if (((sv_type_agps_set & 0x88) == 0x88) && ((sv_type & 0x08) == 0x08)) {
        agalileo_cap = 1;
    }
    gnss_num = agps_cap + aglonass_cap + abeidou_cap + agalileo_cap;

    sprintf(pmtk_str, "$PMTK764,0,0,0");
    sprintf(tmp, ",%d", gnss_num);
    strncat(pmtk_str, tmp, PMTK_MAX_PKT_LENGTH - strlen(pmtk_str) - 1);

    if (agps_cap) {
        strncat(pmtk_str, ",0,128", PMTK_MAX_PKT_LENGTH - strlen(pmtk_str) - 1);
    }
    if (agalileo_cap) {
        strncat(pmtk_str, ",3,128", PMTK_MAX_PKT_LENGTH - strlen(pmtk_str) - 1);
    }
    if (aglonass_cap) {
        strncat(pmtk_str, ",4,128", PMTK_MAX_PKT_LENGTH - strlen(pmtk_str) - 1);
    }
    if (abeidou_cap) {
        strncat(pmtk_str, ",5,128", PMTK_MAX_PKT_LENGTH - strlen(pmtk_str) - 1);
    }
    if (lppe_enable){
        strncat(pmtk_str, ",1", PMTK_MAX_PKT_LENGTH - strlen(pmtk_str) - 1);
        }else{
          strncat(pmtk_str, ",0", PMTK_MAX_PKT_LENGTH - strlen(pmtk_str) - 1);
      }
    add_chksum(pmtk_str);
    LOGD("MNLD_PMTK764: %s, agps_cap:%d, aglonass_cap:%d, abeidou_cap:%d, agalileo_cap:%d, lppe_support:%d\n",
        pmtk_str, agps_cap, aglonass_cap, abeidou_cap, agalileo_cap, lppe_enable);
    return 0;
}

void agps_settings_sync(mnl_agps_agps_settings* s) {
    UINT8 sv_type;
    int ret;

    char pmtk_str[PMTK_MAX_PKT_LENGTH];
    memset(pmtk_str, 0, sizeof(pmtk_str));

    LOGD("agps setting, sib8_16_enable = %d, gps_sat_en = %d, glonass_sat_en = %d, \
        beidou_sat_en = %d, galileo_sat_en = %d, a_glonass_sat_en = %d, \
        a_gps_satellite_enable = %d, a_beidou_satellite_enable = %d, a_galileo_satellite_enable = %d, lppe_enable=%d\n",
        s->sib8_16_enable,
        s->gps_satellite_enable, s->glonass_satellite_enable,
        s->beidou_satellite_enable, s->galileo_satellite_enable,
        s->a_glonass_satellite_enable, s->a_gps_satellite_enable,
        s->a_beidou_satellite_enable, s->a_galileo_satellite_enable,
        s->lppe_enable);
    g_settings_from_agps = *s;

    sv_type_agps_set = 0;
    sv_type_agps_set |= (g_settings_from_agps.a_galileo_satellite_enable & 0x01) << 7;
    sv_type_agps_set |= (g_settings_from_agps.a_beidou_satellite_enable & 0x01) << 6;
    sv_type_agps_set |= (g_settings_from_agps.a_gps_satellite_enable & 0x01) << 5;
    sv_type_agps_set |= (g_settings_from_agps.a_glonass_satellite_enable & 0x01) << 4;
    sv_type_agps_set |= (g_settings_from_agps.galileo_satellite_enable & 0x01) << 3;
    sv_type_agps_set |= (g_settings_from_agps.beidou_satellite_enable & 0x01) << 2;
    sv_type_agps_set |= (g_settings_from_agps.glonass_satellite_enable & 0x01) << 1;
    sv_type_agps_set |= (g_settings_from_agps.gps_satellite_enable & 0x01);
    sib8_16_enable = g_settings_from_agps.sib8_16_enable;
    lppe_enable = g_settings_from_agps.lppe_enable && mtk_gps_mnl_info.support_lppe;
    get_chip_sv_support_capability(&sv_type);
    //LOGD("get_chip_sv_support_capability, sv_type = %d", sv_type);
    g_settings_to_agps.gps_satellite_support = (sv_type) & (0x01);
    g_settings_to_agps.glonass_satellite_support = ((sv_type) & (0x02)) >> 1;
    g_settings_to_agps.beidou_satellite_support = ((sv_type) & (0x04)) >> 2;
    g_settings_to_agps.galileo_satellite_support = ((sv_type) & (0x08)) >> 3;

    ret = mnl2agps_agps_settings_ack(&g_settings_to_agps);
    //LOGD("mnl2agps_agps_settings_ack done, ret = %d", ret);

    if (mnld_is_gps_started_done()) {
        ret = mtk_gps_set_param(MTK_PARAM_CMD_SWITCH_CONSTELLATION, &sv_type_agps_set);
        LOGD("sent CMD_SWITCH_CONSTELLATION to mnl, sv_type_agps_set = 0x%x ,ret = %d", sv_type_agps_set, ret);

        ret = mtk_gps_set_param(MTK_PARAM_CMD_SIB8_16_ENABLE, &sib8_16_enable);
        LOGD("sent CMD_SIB8_16_ENABLE to mnl, sib8_16_enable = %d ,ret = %d", sib8_16_enable, ret);
        if(mtk_gps_mnl_info.support_lppe){
           ret = mtk_gps_set_param(MTK_PARAM_CMD_LPPE_ENABLE, &lppe_enable);
            LOGD("sent MTK_PARAM_CMD_LPPE_ENABLE to mnl, lppe_enable = %d ,ret = %d", lppe_enable, ret);
        }
    } else {
        LOGD("mnl stop, mnld send pmtk764 to agpsd\n");

        // Generate PMTK764 and send to AGPSD
        get_agnss_capability(sv_type_agps_set, sv_type, pmtk_str);
        mnl2agps_pmtk(pmtk_str);
    }
}
static void vzw_debug_screen_output(const char* str) {
    LOGD("vzw_debug_screen_output  str=%s", str);
    mnl2hal_vzw_debug_screen_output(str);
}

static agps2mnl_interface g_agps2mnl_interface = {
    agps_reboot,
    agps_open_gps_req,
    agps_close_gps_req,
    agps_reset_gps_req,
    agps_open_gps_rejected,
    agps_session_done,
    agps_ni_notify,
    agps_data_conn_req,
    agps_data_conn_release,
    agps_set_id_req,
    agps_ref_loc_req,
    agps_rcv_pmtk,
    agps_gpevt,
    agps_location,
    agps_ni_notify2,
    agps_data_conn_req2,
    agps_settings_sync,
    vzw_debug_screen_output,
    agps_rcv_lppe_common_iono,
    agps_rcv_lppe_common_trop,
    agps_rcv_lppe_common_alt,
    agps_rcv_lppe_common_solar,
    agps_rcv_lppe_common_ccp,
    agps_rcv_lppe_generic_ccp,
    agps_rcv_lppe_generic_dm
};

/*****************************************
META -> MNL
*****************************************/
static void meta_req_gnss_location(int source) {
    LOGW("meta_req_gnss_location source: %d", source);
    in_meta_factory = 1;
    factory_mnld_gps_start();
}

static void meta_cancel_gnss_location(int source) {
    LOGW("meta_cancel_gnss_location source: %d", source);
    in_meta_factory = 0;
    factory_mnld_gps_stop();
}

static Meta2MnldInterface_callbacks g_meta2mnl_callbacks = {
    meta_req_gnss_location,
    meta_cancel_gnss_location
};
/////// end of meta

/*****************************************
DEBUG -> MNL
*****************************************/
void mnld_gps_output_data_handle(char* buff, int off_set) {
    char data[MNLD_TO_APP_BUFF_SIZE] = {0};
    int offset = off_set;
    main_out_put_data_type cmd;
    int lenth = 0;

    cmd = get_int(buff, &offset, MNLD_INTERNAL_BUFF_SIZE);

    switch(cmd) {
        case DATA_DEBUG2APP: {
            lenth = get_binary(buff, &offset, data, MNLD_INTERNAL_BUFF_SIZE, sizeof(data));
            LOGW("mnld send debug msg to app:%s", data);
            Mnld2DebugInterface_mnldUpdateMessageInfo(&g_mnld_ctx.fds.fd_debug_client, data);
            break;
        }
        default: {
            LOGW("unkown main_out_put_data_type cmd:%d", cmd);
            return;
        }
    }
}

int sys_gps_mnl_data2mnld_callback(const char *data, unsigned int length) {
    char buff[MNLD_TO_APP_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, GPS2MAIN_EVENT_OUTPUT_DATA);
    put_int(buff, &offset, DATA_DEBUG2APP);
    put_binary(buff, &offset, data, length);
    return safe_sendto(MNLD_MAIN_SOCKET, buff, offset);
}

static void debug_req_mnld_message(Debug2MnldInterface_DebugReqStatusCategory status) {
    Mnld2DebugInterface_MnldGpsStatusCategory gps_status;
    gps_status = (Mnld2DebugInterface_MnldGpsStatusCategory)(mnld_is_gps_started());
    switch(status) {
        case DEBUG2MNLD_INTERFACE_DEBUG_REQ_STATUS_CATEGORY_START_DEBUG:
        case DEBUG2MNLD_INTERFACE_DEBUG_REQ_STATUS_CATEGORY_STOP_DEBUG:
            enable_debug2app = status;
            LOGW("receive debug request msg:%d", status);
            if (MNLD2DEBUG_INTERFACE_MNLD_GPS_STATUS_CATEGORY_GPS_STARTED==gps_status) {
                    if(mtk_gps_set_param(MTK_PARAM_CMD_DEBUG2APP_CONFIG, (const UINT8 *)&enable_debug2app)) {
                        LOGW("send APP Debug msg to mnl fail");
                    }
            }
            break;
        default:
            LOGW("receive wrong debug msg:%d, do nothing", status);
            break;
    }
    Mnld2DebugInterface_mnldAckDebugReq(&g_mnld_ctx.fds.fd_debug_client);
    Mnld2DebugInterface_mnldUpdateGpsStatus(&g_mnld_ctx.fds.fd_debug_client,gps_status);
}

static void debug_mnld_ne_property(bool enabled) {
    if (enabled == true) {
        property_set("vendor.debug.gps.mnld.ne", "1");   //Enable mnld NE
        LOGD("Set MNLD NE property to enable");
    } else {
        property_set("vendor.debug.gps.mnld.ne", "0");   //Disable mnld NE
        LOGD("Set MNLD NE property to disable");
    }
}

static void debug_mnld_radio_property(char* value) {
    #if ANDROID_MNLD_PROP_SUPPORT
    if (strlen(value) <= 10) {  //Max length of value received from HIDL is 10 bytes(char value[10])
        property_set(MNL_CONFIG_STATUS, value);
        LOGD("gps radio property set success, value=%s", value);
    } else {
        LOGW("radio property value incorrect");
    }
    #else
    LOGW("ANDROID_MNLD_PROP_SUPPORT not set");
    #endif
}

static Debug2MnldInterface_callbacks g_debug2mnl_callbacks = {
    debug_req_mnld_message,
    debug_mnld_ne_property,
    debug_mnld_radio_property
};
/////// end of debug

#if 0
static void flp2mnl_flp_reboot() {
    LOGW("flp2mnl_flp_reboot");
    mnld_gps_client* flp = &g_mnld_ctx.gps_status.clients.flp;
    flp->gps_used = false;
    flp->need_open_ack = false;
    flp->need_close_ack = false;
    flp->need_reset_ack = false;
    mnld_fsm(GPS_EVENT_STOP, 0, 0, NULL);
}

static void flp2mnl_gps_start() {
    LOGW("flp2mnl_gps_start\n");
    mnld_gps_client* flp = &g_mnld_ctx.gps_status.clients.flp;
    flp->gps_used = true;
    flp->need_open_ack = true;  // need to confirm flp deamon
    flp->need_close_ack = false;
    mnld_fsm(GPS_EVENT_START, 0, 0, NULL);
}

static void flp2mnl_gps_stop() {
    LOGW("flp2mnl_gps_stop");
    mnld_gps_client* flp = &g_mnld_ctx.gps_status.clients.flp;
    flp->gps_used = false;
    flp->need_open_ack = false;
    flp->need_close_ack = true;
    mnld_fsm(GPS_EVENT_STOP, 0, 0, NULL);
}

static int flp2mnl_flp_lpbk(char *buff, int len) {
    int ret = 0;
    if (-1 == mnl2flp_data_send((UINT8 *)buff, len)) {
        LOGE("[FLP2MNLD]Send to HAL failed, %s\n", strerror(errno));
        ret = -1;
    }
    else {
        LOGD("[FLP2MNLD]Send to HAL successfully\n");
    }
    return ret;
}

static int flp2mnl_flp_data(MTK_FLP_OFFLOAD_MSG_T *prMsg) {
    LOGD("[FLP2MNLD]to connsys: len=%d\n", prMsg->length);
    if (prMsg->length > 0 && prMsg->length <= OFFLOAD_PAYLOAD_LEN) {
        mtk_gps_ofl_send_flp_data((UINT8 *)&prMsg->data[0], prMsg->length);
    } else {
        LOGD("[FLP2MNLD]to connsys: length is invalid\n");
    }
    return 0;
}
#endif

static int mnld_flp_attach() {
    LOGD("mnld_flp_attach");
    if (-1 == mnld_flp_attach_done()) {
        LOGE("Send attach done to FLP failed\n");
    } else {
        LOGD("Send attach done to FLP succeed\n");
    }
    return 0;
}

static void mnld_flp_hbd_gps_open() {
    LOGD("mnld_flp_hbd_gps_open\n");
    mnld_gps_client* flp = &g_mnld_ctx.gps_status.clients.flp;
    flp->gps_used = true;
    flp->need_open_ack = true;  // need to confirm flp deamon
    flp->need_close_ack = false;
    mnld_fsm(GPS_EVENT_START, 0, 0, NULL);
}

static void mnld_flp_hbd_gps_close() {
    LOGD("mnld_flp_hbd_gps_close");
    mnld_gps_client* flp = &g_mnld_ctx.gps_status.clients.flp;
    flp->gps_used = false;
    flp->need_open_ack = false;
    flp->need_close_ack = true;
    mnld_fsm(GPS_EVENT_STOP, 0, 0, NULL);
}

static void mnld_flp_ofl_link_open() {
    LOGD("mnld_flp_ofl_link_open\n");
    mnld_gps_client* flp = &g_mnld_ctx.gps_status.clients.flp;
    flp->gps_used = true;
    flp->need_open_ack = true;  // need to confirm flp deamon
    flp->need_close_ack = false;
    mnld_flp_session.type = MNLD_FLP_CAPABILITY_OFL_MODE;
    mnld_fsm(GPS_EVENT_OFFLOAD_START, 0, 0, NULL);
}

static void mnld_flp_ofl_link_close() {
    LOGD("mnld_flp_ofl_link_close");
    mnld_gps_client* flp = &g_mnld_ctx.gps_status.clients.flp;
    flp->gps_used = false;
    flp->need_open_ack = false;
    flp->need_close_ack = true;
    mnld_fsm(GPS_EVENT_STOP, 0, 0, NULL);
}

static int mnld_flp_ofl_link_send(MTK_FLP_MNL_MSG_T *prMsg) {
    LOGD("mnld_flp_ofl_link_send to connsys: len=%d,session_id=%u\n", prMsg->length, prMsg->session);
    if (prMsg->session == mnld_flp_session.id) {
        if (prMsg->length > 0 && prMsg->length <= OFFLOAD_PAYLOAD_LEN) {
            mtk_gps_ofl_send_flp_data((UINT8 *)&prMsg->data[0], prMsg->length);
        } else {
            LOGW("mnld_flp_ofl_link_send to connsys: length is invalid\n");
        }
    } else {
        LOGW("session id doesn't match\n");
    }
    return 0;
}

static flp2mnl_interface g_flp2mnl_interface = {
    mnld_flp_attach,
    mnld_flp_hbd_gps_open,
    mnld_flp_hbd_gps_close,
    mnld_flp_ofl_link_open,
    mnld_flp_ofl_link_close,
    mnld_flp_ofl_link_send
};

#if 0
static void gfc2mnl_gfc_reboot() {
    LOGW("gfc2mnl_gfc_reboot");
    mnld_gps_client* geofence = &g_mnld_ctx.gps_status.clients.geofence;
    geofence->gps_used = false;
    geofence->need_open_ack = false;
    geofence->need_close_ack = false;
    geofence->need_reset_ack = false;
    mnld_fsm(GPS_EVENT_STOP, 0, 0, NULL);
}

static void gfc2mnl_gps_start() {
    LOGW("gfc2mnl_gps_start\n");
    mnld_gps_client* geofence = &g_mnld_ctx.gps_status.clients.geofence;
    geofence->gps_used = true;
    geofence->need_open_ack = true;  // need to confirm flp deamon
    geofence->need_close_ack = false;
    mnld_fsm(GPS_EVENT_START, 0, 0, NULL);
}

static void gfc2mnl_gps_stop() {
    LOGW("gfc2mnl_gps_stop");
    mnld_gps_client* geofence = &g_mnld_ctx.gps_status.clients.geofence;
    geofence->gps_used = false;
    geofence->need_open_ack = false;
    geofence->need_close_ack = true;
    mnld_fsm(GPS_EVENT_STOP, 0, 0, NULL);
}
#endif

static int mnld_gfc_attach() {
    LOGD("mnld_gfc_attach");
    if (-1 == mnld_gfc_attach_done()) {
        LOGE("Send attach done to GFC failed\n");
    } else {
        LOGD("Send attach done to GFC succeed\n");
    }
    return 0;
}

static void mnld_gfc_hbd_gps_open() {
    LOGD("mnld_gfc_hbd_gps_open\n");
    mnld_gps_client* geofence = &g_mnld_ctx.gps_status.clients.geofence;
    geofence->gps_used = true;
    geofence->need_open_ack = true;
    geofence->need_close_ack = false;
    mnld_fsm(GPS_EVENT_START, 0, 0, NULL);
}

static void mnld_gfc_hbd_gps_close() {
    LOGD("mnld_gfc_hbd_gps_close");
    mnld_gps_client* geofence = &g_mnld_ctx.gps_status.clients.geofence;
    geofence->gps_used = false;
    geofence->need_open_ack = false;
    geofence->need_close_ack = true;
    mnld_fsm(GPS_EVENT_STOP, 0, 0, NULL);
}

static void mnld_gfc_ofl_link_open() {
    LOGD("mnld_gfc_ofl_link_open\n");
    mnld_gps_client* geofence = &g_mnld_ctx.gps_status.clients.geofence;
    geofence->gps_used = true;
    geofence->need_open_ack = true;  // need to confirm flp deamon
    geofence->need_close_ack = false;
    mnld_gfc_session.type = MNLD_GFC_CAPABILITY_OFL_MODE;
    mnld_fsm(GPS_EVENT_OFFLOAD_START, 0, 0, NULL);
}

static void mnld_gfc_ofl_link_close() {
    LOGD("mnld_gfc_ofl_link_close");
    mnld_gps_client* geofence = &g_mnld_ctx.gps_status.clients.geofence;
    geofence->gps_used = false;
    geofence->need_open_ack = false;
    geofence->need_close_ack = true;
    mnld_fsm(GPS_EVENT_STOP, 0, 0, NULL);
}

static int mnld_gfc_ofl_link_send(MTK_GFC_MNL_MSG_T *prMsg) {
    LOGD("mnld_gfc_ofl_link_send to connsys: len=%d,session_id=%u\n", prMsg->length, prMsg->session);
    if (prMsg->session == mnld_gfc_session.id) {
        if (prMsg->length > 0 && prMsg->length <= OFFLOAD_PAYLOAD_LEN) {
            mtk_gps_ofl_send_flp_data((UINT8 *)&prMsg->data[0], prMsg->length);
        } else {
            LOGW("mnld_gfc_ofl_link_send to connsys: length is invalid\n");
        }
    } else {
        LOGW("session id doesn't match\n");
    }
    return 0;
}

static gfc2mnl_interface g_gfc2mnl_interface = {
    mnld_gfc_attach,
    mnld_gfc_hbd_gps_open,
    mnld_gfc_hbd_gps_close,
    mnld_gfc_ofl_link_open,
    mnld_gfc_ofl_link_close,
    mnld_gfc_ofl_link_send
};


void flp_test2mnl_gps_start() {
    LOGD("flp_test2mnl_gps_start\n");
    mnld_gps_client* flp_test = &g_mnld_ctx.gps_status.clients.flp_test;
    flp_test->gps_used = true;
    flp_test->need_open_ack = false;  // need to confirm flp deamon
    flp_test->need_close_ack = false;
    mnld_fsm(GPS_EVENT_START, 0, 0, NULL);
}

void flp_test2mnl_gps_stop() {
    LOGD("flp_test2mnl_gps_stop");
    mnld_gps_client* flp_test = &g_mnld_ctx.gps_status.clients.flp_test;
    flp_test->gps_used = false;
    flp_test->need_open_ack = false;
    flp_test->need_close_ack = false;
    mnld_fsm(GPS_EVENT_STOP, 0, 0, NULL);
}

extern int ofl_lpbk_tst_start;
extern int ofl_lpbk_tst_size;
#if 0
static int flp_test2mnl_flp_lpbk_start() {
    char tmp_buf[1024];
    int i;

    LOGD("[OFL]lpbk start!");
    for (i=0; i < ofl_lpbk_tst_size; i++) {
        tmp_buf[i] = 0x80;
    }
    mtk_gps_ofl_send_flp_data((UINT8 *)&tmp_buf[0], ofl_lpbk_tst_size);
    ofl_lpbk_tst_start = 1;
    return 0;
}

static int flp_test2mnl_flp_lpbk_stop() {
    LOGD("[OFL]lpbk stop!");
    ofl_lpbk_tst_start = 0;

    return 0;
}


static flp_test2mnl_interface g_flp_test2mnl_interface = {
    flp_test2mnl_gps_start,
    flp_test2mnl_gps_stop,
    flp_test2mnl_flp_lpbk_start,
    flp_test2mnl_flp_lpbk_stop,
};
#endif

void factory_mnld_gps_start() {
    LOGD("factory_mnld_gps_start\n");
    mnld_gps_client* factory = &g_mnld_ctx.gps_status.clients.factory;
    factory->gps_used = true;
    factory->need_open_ack = true;  // need to confirm flp deamon
    factory->need_close_ack = false;
    mnld_fsm(GPS_EVENT_START, 0, 0, NULL);
}

void factory_mnld_gps_stop() {
    LOGD("factory_mnld_gps_stop");
    mnld_gps_client* factory = &g_mnld_ctx.gps_status.clients.factory;
    factory->gps_used = false;
    factory->need_open_ack = false;
    factory->need_close_ack = false;
    mnld_fsm(GPS_EVENT_STOP, 0, 0, NULL);
}

// for AT cmd
int at_test2mnl_gps_start(void) {
    LOGD("at_test2mnl_gps_start\n");
    mnld_gps_client* at_test = &g_mnld_ctx.gps_status.clients.at_cmd_test;
    at_test->gps_used = true;
    at_test->need_open_ack = false;  // need to confirm flp deamon
    at_test->need_close_ack = false;
    mnld_fsm(GPS_EVENT_START, 0, 0, NULL);
    return 0;
}

int at_test2mnl_gps_stop(void) {
    LOGD("at_test2mnl_gps_stop");
    mnld_gps_client* at_test = &g_mnld_ctx.gps_status.clients.at_cmd_test;
    at_test->gps_used = false;
    at_test->need_open_ack = false;
    at_test->need_close_ack = false;
    mnld_fsm(GPS_EVENT_STOP, 0, 0, NULL);
    return 0;
}

/*****************************************
ALL -> MAIN
*****************************************/
int mnld_gps_start_done(bool is_assist_req) {
    char buff[MNLD_INTERNAL_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, GPS2MAIN_EVENT_START_DONE);
    put_int(buff, &offset, is_assist_req);
    return safe_sendto(MNLD_MAIN_SOCKET, buff, offset);
}

int mnld_gps_start_nmea_timeout() {
    char buff[MNLD_INTERNAL_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, GPS2MAIN_EVENT_NMEA_TIMEOUT);
    return safe_sendto(MNLD_MAIN_SOCKET, buff, offset);
}

int mnld_gps_stop_done() {
    char buff[MNLD_INTERNAL_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, GPS2MAIN_EVENT_STOP_DONE);
    return safe_sendto(MNLD_MAIN_SOCKET, buff, offset);
}

#if defined(GPS_SUSPEND_SUPPORT)
int mnld_gps_suspend_done() {
    char buff[MNLD_INTERNAL_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, GPS2MAIN_EVENT_SUSPEND_DONE);
    return safe_sendto(MNLD_MAIN_SOCKET, buff, offset);
}

int mnld_gps_suspend_timeout() {
    char buff[MNLD_INTERNAL_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, GPS2MAIN_EVENT_SUSPEND_TIMEOUT);
    return safe_sendto(MNLD_MAIN_SOCKET, buff, offset);
}
#endif

int mnld_gps_reset_done() {
    char buff[MNLD_INTERNAL_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, GPS2MAIN_EVENT_RESET_DONE);
    return safe_sendto(MNLD_MAIN_SOCKET, buff, offset);
}

int mnld_gps_update_location(gps_location location) {
    char buff[MNLD_INTERNAL_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, GPS2MAIN_EVENT_UPDATE_LOCATION);
    put_binary(buff, &offset, (const char*)&location, sizeof(location));
    return safe_sendto(MNLD_MAIN_SOCKET, buff, offset);
}

int mnld_epo_download_done(epo_download_result result) {
    char buff[MNLD_INTERNAL_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, EPO2MAIN_EVENT_EPO_DONE);
    put_int(buff, &offset, result);
    return safe_sendto(MNLD_MAIN_SOCKET, buff, offset);
}

int mnld_qepo_download_done(epo_download_result result) {
    char buff[MNLD_INTERNAL_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, QEPO2MAIN_EVENT_QEPO_DONE);
    put_int(buff, &offset, result);
    return safe_sendto(MNLD_MAIN_SOCKET, buff, offset);
}

int mnld_qepo_bd_download_done(epo_download_result result) {
    char buff[MNLD_INTERNAL_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, QEPO2MAIN_EVENT_QEPO_BD_DONE);
    put_int(buff, &offset, result);
    return safe_sendto(MNLD_MAIN_SOCKET, buff, offset);
}

int mnld_qepo_ga_download_done(epo_download_result result) {
    char buff[MNLD_INTERNAL_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, QEPO2MAIN_EVENT_QEPO_GA_DONE);
    put_int(buff, &offset, result);
    return safe_sendto(MNLD_MAIN_SOCKET, buff, offset);
}

int mnld_mtknav_download_done(epo_download_result result) {
    char buff[MNLD_INTERNAL_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, MTKNAV2MAIN_EVENT_MTKNAV_DONE);
    put_int(buff, &offset, result);
    return safe_sendto(MNLD_MAIN_SOCKET, buff, offset);
}

int mnld_screen_on_notify() {
    char buff[MNLD_INTERNAL_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, SCREEN2MAIN_EVENT_SCREEN_ON);
    return safe_sendto(MNLD_MAIN_SOCKET, buff, offset);
}

int mnld_screen_off_notify() {
    char buff[MNLD_INTERNAL_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, SCREEN2MAIN_EVENT_SCREEN_OFF);
    return safe_sendto(MNLD_MAIN_SOCKET, buff, offset);
}

int mnld_screen_unknown_notify() {
    char buff[MNLD_INTERNAL_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, SCREEN2MAIN_EVENT_SCREEN_UNKNOWN);
    return safe_sendto(MNLD_MAIN_SOCKET, buff, offset);
}

static int main_event_hdlr(int fd) {
    char buff[MNLD_INTERNAL_BUFF_SIZE] = {0};
    int offset = 0;
    main_internal_event cmd;
    int read_len;

    read_len = safe_recvfrom(fd, buff, sizeof(buff));
    if (read_len <= 0) {
        LOGE("main_event_hdlr() safe_recvfrom() failed read_len=%d", read_len);
        return -1;
    }

    cmd = get_int(buff, &offset, sizeof(buff));
    switch (cmd) {
    case GPS2MAIN_EVENT_START_DONE: {
        int is_assist_req = get_int(buff, &offset, sizeof(buff));
        LOGW("GPS2MAIN_EVENT_START_DONE  is_assist_req=%d", is_assist_req);
        mnld_fsm(GPS_EVENT_START_DONE, is_assist_req, 0, NULL);
        break;
    }
    case GPS2MAIN_EVENT_STOP_DONE: {
        LOGW("GPS2MAIN_EVENT_STOP_DONE");
        mnld_fsm(GPS_EVENT_STOP_DONE, 0, 0, NULL);
        break;
    }
#if defined(GPS_SUSPEND_SUPPORT)
    case GPS2MAIN_EVENT_SUSPEND_DONE: {
        LOGW("GPS2MAIN_EVENT_SUSPEND_DONE");
        mnld_fsm(GPS_EVENT_SUSPEND_DONE, 0, 0, NULL);
        break;
    }

    case GPS2MAIN_EVENT_SUSPEND_TIMEOUT: {
        if (mnld_is_gps_suspend()) {
            LOGW("GPS2MAIN_EVENT_SUSPEND_TIMEOUT");
            mnld_fsm(GPS_EVENT_SUSPEND_CLOSE, 0, 0, NULL);
        } else {
            LOGW("GPS2MAIN_EVENT_SUSPEND_TIMEOUT ignored");
        }
        break;
    }
#endif
    case GPS2MAIN_EVENT_RESET_DONE: {
        LOGW("GPS2MAIN_EVENT_RESET_DONE");
        stop_timer(g_mnld_ctx.gps_status.timer_reset);
        do_gps_reset_hdlr();
        break;
    }
    case GPS2MAIN_EVENT_NMEA_TIMEOUT: {
        LOGD("GPS2MAIN_EVENT_NMEA_TIMEOUT");
        g_mnld_ctx.gps_status.is_in_nmea_timeout_handler = true;
        if (mnld_is_gps_started_done()) {
            mtk_gps_clear_gps_user();
            LOGD("set nmea timeout event to main thread\n");
            mnld_fsm(GPS_EVENT_STOP, 0, 0, NULL);
            // send the reboot message to the related modules
            mnl2hal_mnld_reboot();
            mnl2agps_mnl_reboot();
            mnl2flp_mnld_reboot();
            mnl2gfc_mnld_reboot();
        }
        g_mnld_ctx.gps_status.is_in_nmea_timeout_handler = false;
        break;
        }
    case GPS2MAIN_EVENT_UPDATE_LOCATION: {
        gps_location location;
        get_binary(buff, &offset, (char*)&location, sizeof(buff), sizeof(gps_location));
        bool alt_valid = (location.flags & MTK_GPS_LOCATION_HAS_ALT)? true : false;
        bool source_valid = true;
        bool source_gnss = true;
        bool source_nlp = false;
        bool source_sensor = false;
        float valid_acc = (location.flags & MTK_GPS_LOCATION_HAS_HORIZONTAL_ACCURACY)? location.h_accuracy : 2000;
        LOGWX("GPS2MAIN_EVENT_UPDATE_LOCATION  lat=%f lng=%f acc=%f",
            location.lat, location.lng, valid_acc);
        LOGD_ENG("wait_first_location=%d\n", g_mnld_ctx.gps_status.wait_first_location);

        if(mnld_nfw_ctrl_gnss_enabled() && mtk_gps_get_nfw_visibility(MNLD_NFW_USER_AGPS)) {
            if (g_mnld_ctx.gps_status.wait_first_location) {
                g_mnld_ctx.gps_status.wait_first_location = false;
                g_mnld_ctx.gps_status.gps_ttff = get_tick() - g_mnld_ctx.gps_status.gps_start_time;
        #if ANDROID_MNLD_PROP_SUPPORT
                if (get_gps_cmcc_log_enabled()) {
                    op01_log_gps_location(location.lat, location.lng, g_mnld_ctx.gps_status.gps_ttff);
                }
        #else
                op01_log_gps_location(location.lat, location.lng, g_mnld_ctx.gps_status.gps_ttff);
        #endif
            }
        }
        mnld_nfw_mnl2agps_location_sync(location.lat, location.lng, (int)valid_acc, alt_valid, (float)location.alt, source_valid, source_gnss, source_nlp, source_sensor);

        LOGD_ENG("location_sync done\n");
        break;
    }
    case EPO2MAIN_EVENT_EPO_DONE: {
        epo_download_result result = get_int(buff, &offset, sizeof(buff));
        LOGD("EPO2MAIN_EVENT_EPO_DONE  result=%d", result);
        g_mnld_ctx.epo_status.is_epo_downloading = false;
        // TODO libmnl.so to inject the EPO
        bool started = mnld_is_gps_or_ofl_started_done();
        if (result == EPO_DOWNLOAD_RESULT_SUCCESS) {
            if (started) {
                epo_update_epo_file();
            }
        } else {
            unlink(EPO_UPDATE_HAL);
        }
        break;
    }
    case QEPO2MAIN_EVENT_QEPO_DONE: {
        epo_download_result result = get_int(buff, &offset, sizeof(buff));
        bool started = mnld_is_gps_or_ofl_started_done();
        LOGD("QEPO2MAIN_EVENT_QEPO_DONE  result=%d started=%d\n", result, started);
        if (started) {
            qepo_update_quarter_epo_file(result);
        } else {
            LOGW("qepo download finsh before GPS start done");
            qepo_update_flag = true;
        }
        break;
    }
    case QEPO2MAIN_EVENT_QEPO_BD_DONE: {
        epo_download_result result = get_int(buff, &offset, sizeof(buff));
        bool started = mnld_is_gps_or_ofl_started_done();
        LOGD("QEPO2MAIN_EVENT_QEPO_BD_DONE  result=%d started=%d\n", result, started);
        if (started) {
            qepo_update_quarter_epo_bd_file(result);
        } else {
            LOGW("qepo BD download finsh before GPS start done");
            qepo_BD_update_flag = true;
        }
        break;
    }
    case QEPO2MAIN_EVENT_QEPO_GA_DONE: {
        epo_download_result result = get_int(buff, &offset, sizeof(buff));
        bool started = mnld_is_gps_or_ofl_started_done();
        LOGD("QEPO2MAIN_EVENT_QEPO_GA_DONE  result=%d started=%d\n", result, started);
        if (started) {
            qepo_update_quarter_epo_ga_file(result);
        } else {
            LOGW("qepo GA download finsh before GPS start done");
            qepo_GA_update_flag = true;
        }
        break;
    }
    case MTKNAV2MAIN_EVENT_MTKNAV_DONE: {
        epo_download_result result = get_int(buff, &offset, sizeof(buff));
        bool started = mnld_is_gps_or_ofl_started_done();
        LOGD("MTKNAV2MAIN_EVENT_MTKNAV_DONE  result=%d started=%d\n", result, started);
        if (started) {
            mtknav_update_mtknav_file(result);
        } else {
            LOGW("mtknav download finsh before GPS start done");
            mtknav_update_flag = true;
        }
        break;
    }
    case GPS2MAIN_EVENT_OUTPUT_DATA: {
        mnld_gps_output_data_handle(buff, offset);
        break;
    }
    case SCREEN2MAIN_EVENT_SCREEN_UNKNOWN:
    case SCREEN2MAIN_EVENT_SCREEN_OFF: {
        if (cmd == SCREEN2MAIN_EVENT_SCREEN_OFF) {
            g_mnld_ctx.screen_status = SCREEN_STATUS_OFF;
        } else {
            g_mnld_ctx.screen_status = SCREEN_STATUS_UNKNOWN;
        }
        LOGD("SCREEN2MAIN_EVENT_SCREEN = %d", g_mnld_ctx.screen_status);
#if defined(GPS_SUSPEND_SUPPORT)
        if (mnld_is_gps_suspend()) {
            mnld_fsm(GPS_EVENT_SUSPEND_CLOSE, 0, 0, NULL);
        }
#endif
        break;
    }
    case SCREEN2MAIN_EVENT_SCREEN_ON: {
        g_mnld_ctx.screen_status = SCREEN_STATUS_ON;
        LOGD("SCREEN2MAIN_EVENT_SCREEN = %d", g_mnld_ctx.screen_status);
#if defined(GPS_SUSPEND_SUPPORT)
        if (mnld_gps_suspend_get_timeout_sec() &&
            g_mnld_ctx.gps_status.is_suspend_timer_running) {
                // It's possible timer_suspend is running, we can cancel it
                // if screen change to be on.
                // And it should be no harm to cancel it even when it's not running.
                stop_timer(g_mnld_ctx.gps_status.timer_suspend);
                g_mnld_ctx.gps_status.is_suspend_timer_running = false;
        }
#endif
        break;
    }
    default: {
        LOGW("event = %d is unhandled", cmd);
    }
    }
    return 0;
}

/*****************************************
Threads
*****************************************/
static void mnld_main_thread_timeout() {
    if (mnld_timeout_ne_enabled() == false) {
        LOGE("mnld_main_thread_timeout() dump and exit.");
        mnld_exiting = true;
        gps_dbg_log_exit_flush(0);
        mnld_block_exit();
    } else {
        LOGE("mnld_main_thread_timeout() crash here for debugging");
        CRASH_TO_DEBUG();
    }
}

static void mnld_gps_start_timeout() {
    if (mnld_timeout_ne_enabled() == false) {
        LOGE("mnld_gps_start_timeout() dump and exit.");
        mnld_exiting = true;
        gps_dbg_log_exit_flush(0);
        mnld_block_exit();
    } else {
        LOGE("mnld_gps_start_timeout() crash here for debugging");
        CRASH_TO_DEBUG();
    }
}

static void mnld_gps_stop_timeout() {
    if (mnld_timeout_ne_enabled() == false) {
        LOGE("mnld_gps_stop_timeout() dump and exit.");
        mnld_exiting = true;
        gps_dbg_log_exit_flush(0);
        mnld_block_exit();
    } else {
        LOGE("mnld_gps_stop_timeout() crash here for debugging");
        CRASH_TO_DEBUG();
    }
}

static void mnld_gps_reset_timeout() {
    if (mnld_timeout_ne_enabled() == false) {
        LOGE("mnld_gps_reset_timeout() dump and exit.");
        mnld_exiting = true;
        gps_dbg_log_exit_flush(0);
        mnld_block_exit();
    } else {
        LOGE("mnld_gps_reset_timeout() crash here for debugging");
        CRASH_TO_DEBUG();
    }
}

static void mnld_gps_switch_ofl_mode_timeout() {
    if (is_all_hbd_gps_client_exit()) {
        LOGD("switch to offload mode timer timeout, flp user prepare to stop gps");
        mnld_gps_client* flp = &g_mnld_ctx.gps_status.clients.flp;
        mnld_gps_client* geofence = &g_mnld_ctx.gps_status.clients.geofence;
        //flp
        flp->gps_used = false;
        flp->need_open_ack = false;
        flp->need_close_ack = false;
        //geofence
        geofence->gps_used = false;
        geofence->need_open_ack = false;
        geofence->need_close_ack = false;
        mnld_fsm(GPS_EVENT_STOP, 0, 0, NULL);
    } else {
        LOGD("switch to offload mode timer timeout. But some HBD user exist, so not stop gps");
    }
}

#if defined(GPS_SUSPEND_SUPPORT)
static void mnld_gps_suspend_timeout_callback() {
    if (mnld_is_gps_suspend()) {
        LOGD("Suspend mode timer timeout, leave suspend state and stop gps");
        mnld_gps_suspend_timeout();
    } else {
        LOGD("No need to handle for GPS has leaved suspend state");
    }
}
#endif

void gps_mnld_restart_mnl_process(void) {
    LOGD("gps_mnld_restart_mnl_process\n");
    //mnld_gps_start_nmea_timeout();
    if (mnld_timeout_ne_enabled() == false) {
        LOGE("gps_mnld_restart_mnl_process() dump and exit.");
        mnld_exiting = true;
        gps_dbg_log_exit_flush(0);
        mnld_block_exit();
    } else {
        LOGE("gps_mnld_restart_mnl_process() crash here for debugging");
        CRASH_TO_DEBUG();
    }
}

void mnld_gps_request_nlp(int src) {
    LOGD("mnld_gps_request_nlp src: %d", src);
    //Mnld2NlpUtilsInterface_reqNlpLocation(&g_mnld_ctx.fds.fd_nlp_utils, src);
    bool fgemergency = true; //Emergency flag always set to true, because framework will double check it by itself
    mnl2hal_request_nlp((bool)src, fgemergency);
}

static void* mnld_main_thread(void *arg) {
    #define MAX_EPOLL_EVENT 50
    timer_t hdlr_timer = init_timer(mnld_main_thread_timeout);
    struct epoll_event events[MAX_EPOLL_EVENT];
    UNUSED(arg);

    int epfd = epoll_create(MAX_EPOLL_EVENT);
    if (epfd == -1) {
        LOGE("mnld_main_thread() epoll_create failure reason=[%s]%d",
            strerror(errno), errno);
        return 0;
    }
#ifdef MTK_AGPS_SUPPORT
    int fd_agps = g_mnld_ctx.fds.fd_agps;
#endif
    int fd_hal = g_mnld_ctx.fds.fd_hal;
    int fd_flp = g_mnld_ctx.fds.fd_flp;
    int fd_flp_test = g_mnld_ctx.fds.fd_flp_test;
    int fd_geofence = g_mnld_ctx.fds.fd_geofence;
    int fd_at_cmd = g_mnld_ctx.fds.fd_at_cmd;
    int fd_int = g_mnld_ctx.fds.fd_int;
    int fd_mtklogger = g_mnld_ctx.fds.fd_mtklogger;
    int fd_meta = g_mnld_ctx.fds.fd_meta;
    int fd_debug = g_mnld_ctx.fds.fd_debug;
#ifdef MTK_AGPS_SUPPORT
    if (epoll_add_fd(epfd, fd_agps) == -1) {
        LOGE("mnld_main_thread() epoll_add_fd() failed for fd_agps failed");
        return 0;
    }
#endif
    if (epoll_add_fd(epfd, fd_hal) == -1) {
        LOGE("mnld_main_thread() epoll_add_fd() failed for fd_hal failed");
        return 0;
    }
    if (epoll_add_fd(epfd, fd_flp) == -1) {
        LOGE("mnld_main_thread() epoll_add_fd() failed for fd_flp failed");
        return 0;
    }
    if (epoll_add_fd(epfd, fd_geofence) == -1) {
        LOGE("mnld_main_thread() epoll_add_fd() failed for fd_geofence failed");
        return 0;
    }
    //if (epoll_add_fd(epfd, fd_flp_test) == -1) {
    //    LOGE("mnld_main_thread() epoll_add_fd() failed for fd_flp_test failed");
    //    return 0;
    //}
    if (epoll_add_fd(epfd, fd_at_cmd) == -1) {
        LOGE("mnld_main_thread() epoll_add_fd() failed for fd_at_cmd failed");
        return 0;
    }
    if (epoll_add_fd(epfd, fd_int) == -1) {
        LOGE("mnld_main_thread() epoll_add_fd() failed for fd_int failed");
        return 0;
    }
    if (epoll_add_fd(epfd, fd_mtklogger) == -1) {
        LOGE("mnld_main_thread() epoll_add_fd() failed for fd_mtklogger failed");
        return 0;
    }

    if (epoll_add_fd(epfd, fd_meta) == -1) {
        LOGE("mnld_main_thread() epoll_add_fd() failed for fd_meta failed");
        return 0;
    }

    if (epoll_add_fd(epfd, fd_debug) == -1) {
        LOGE("mnld_main_thread() epoll_add_fd() failed for fd_debug failed");
        return 0;
    }

    while (1) {
        int i;
        int n = 0;
        #ifdef CONFIG_GPS_ENG_LOAD
        LOGD("mnld_main_thread() enter wait\n");
        #endif
        n = epoll_wait(epfd, events, MAX_EPOLL_EVENT , -1);
        if (n > 1) {
            LOGD("n=%d\n", n);
        } else if (n == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                LOGE("mnld_main_thread() epoll_wait failure reason=[%s]%d",
                    strerror(errno), errno);
                return 0;
            }
        }
        start_timer(hdlr_timer, MNLD_MAIN_HANDLER_TIMEOUT);
        for (i = 0; i < n; i++) {
        #ifdef MTK_AGPS_SUPPORT
            if (events[i].data.fd == fd_agps) {
                if (events[i].events & EPOLLIN) {
                    //LOGD("agps2mnl_hdlr msg");
                    agps2mnl_hdlr(fd_agps, &g_agps2mnl_interface);
                }
            } else
        #endif
            if (events[i].data.fd == fd_hal) {
                if (events[i].events & EPOLLIN) {
                    LOGD_ENG("hal2mnl_hdlr msg");
                    hal2mnl_hdlr(fd_hal, &g_hal2mnl_interface);
                }
            } else if (events[i].data.fd == fd_flp) {
                if (events[i].events & EPOLLIN) {
                    LOGD("flp2mnl_hdlr msg");
                    mtk_hal2flp_main_hdlr(fd_flp, &g_flp2mnl_interface);
                }
            }  else if (events[i].data.fd == fd_geofence) {
                if (events[i].events & EPOLLIN) {
                    LOGD("gfc2mnl_hdlr msg");
                    mtk_hal2gfc_main_hdlr(fd_geofence, &g_gfc2mnl_interface);
                }
            } else if (events[i].data.fd == fd_flp_test) {
                    if (events[i].events & EPOLLIN) {
                    LOGD("fd_flp_test msg");
                    //flp_test2mnl_hdlr(fd_flp_test, &g_flp_test2mnl_interface);
                }
            } else if (events[i].data.fd == fd_at_cmd) {
                if (events[i].events & EPOLLIN) {
                    LOGD("at_cmd2mnl_hdlr msg");
                    at_cmd2mnl_hdlr(fd_at_cmd);
                }
            } else if (events[i].data.fd == fd_int) {
                if (events[i].events & EPOLLIN) {
                    LOGD_ENG("main_event_hdlr msg");
                    main_event_hdlr(fd_int);
                }
            } else if (events[i].data.fd == fd_mtklogger) {
                if ((events[i].events & EPOLLIN) != 0) {
                    LOGD("mtklogger2mnl_hdlr msg");
                    start_timer(hdlr_timer, MNLD_GPS_LOG_HANDLER_TIMEOUT);  // mkdir in external sdcard  so slowly
                    mtklogger2mnl_hdlr(fd_mtklogger, &g_mnld_ctx.fds.fd_mtklogger_client);
                }
            } else if (events[i].data.fd == fd_meta) {
                if (events[i].events & EPOLLIN) {
                    LOGD("meta2mnl_event_hdlr msg");
                    Meta2MnldInterface_receiver_read_and_decode(fd_meta, &g_meta2mnl_callbacks);
                }
            } else if (events[i].data.fd == fd_debug) {
                if (events[i].events & EPOLLIN) {
                    LOGD_ENG("debug2mnl_event_hdlr msg");
                    Debug2MnldInterface_receiver_read_and_decode(fd_debug, &g_debug2mnl_callbacks);
                }
            } else {
                LOGE("mnld_main_thread() unknown fd=%d",
                    events[i].data.fd);
            }
        }
        stop_timer(hdlr_timer);
    }
    LOGE("mnld_main_thread() exit");
    return 0;
}

static int mnld_init() {
    pthread_t pthread_main;

    // init fds
#ifdef MTK_AGPS_SUPPORT
    g_mnld_ctx.fds.fd_agps = create_agps2mnl_fd();
    if (g_mnld_ctx.fds.fd_agps < 0) {
        LOGE("create_agps2mnl_fd() failed");
        return -1;
    }
#else
    g_mnld_ctx.fds.fd_agps = 0;
#endif

    g_mnld_ctx.fds.fd_hal = create_hal2mnl_fd();
    if (g_mnld_ctx.fds.fd_hal < 0) {
        LOGE("create_hal2mnl_fd() failed");
        return -1;
    }

    g_mnld_ctx.fds.fd_flp = create_flphal2mnl_fd();
    if (g_mnld_ctx.fds.fd_flp < 0) {
        LOGE("create_flphal2mnl_fd() failed");
        return -1;
    }
    #if 0
    g_mnld_ctx.fds.fd_flp_test = create_flp_test2mnl_fd();
    if (g_mnld_ctx.fds.fd_flp_test < 0) {
        LOGE("create_flp2mnl_fd() failed");
        return -1;
    }
    #endif

    g_mnld_ctx.fds.fd_geofence= create_gfchal2mnl_fd();
    if (g_mnld_ctx.fds.fd_geofence< 0) {
        LOGE("create_gfchal2mnl_fd() failed");
        return -1;
    }
    g_mnld_ctx.fds.fd_at_cmd = create_at2mnl_fd();
    if (g_mnld_ctx.fds.fd_at_cmd < 0) {
        LOGE("create_at2mnl_fd() failed");
        return -1;
    }
    g_mnld_ctx.fds.fd_int = socket_bind_udp(MNLD_MAIN_SOCKET);
    if (g_mnld_ctx.fds.fd_int < 0) {
        LOGE("socket_bind_udp(MNLD_MAIN_SOCKET) failed");
        return -1;
    }
    g_mnld_ctx.fds.fd_meta = mtk_socket_server_bind_local(META_TO_MNLD_SOCKET, SOCK_NS_ABSTRACT);
    if (g_mnld_ctx.fds.fd_meta < 0) {
        LOGE("create meta fd failed");
        return -1;
    }

    g_mnld_ctx.fds.fd_debug = mtk_socket_server_bind_local(DEBUG_TO_MNLD_SOCKET, SOCK_NS_ABSTRACT);
    if (g_mnld_ctx.fds.fd_debug < 0) {
        LOGE("create debug fd failed");
        return -1;
    }

    g_mnld_ctx.fds.fd_mtklogger = mtk_socket_server_bind_local(MTKLOGGER_TO_MNLD_SOCKET, SOCK_NS_ABSTRACT);
    if (g_mnld_ctx.fds.fd_mtklogger < 0) {
        LOGE("create_mtklogger2mnl_fd() failed");
        return -1;
    }

    mtk_socket_client_init_local(&g_mnld_ctx.fds.fd_debug_client,
            MNLD_TO_DEBUG_SOCKET, SOCK_NS_ABSTRACT);

    mtk_socket_client_init_local(&g_mnld_ctx.fds.fd_nlp_utils,
            MNLD_TO_NLP_UTILS_SOCKET, SOCK_NS_ABSTRACT);

    mtk_socket_client_init_local(&g_mnld_ctx.fds.fd_mtklogger_client,
            MNLD_TO_MTKLOGGER_SOCKET, SOCK_NS_ABSTRACT);
    mtk_socket_client_init_local(&gpslogd_fd, LOG_HIDL_INTERFACE, SOCK_NS_ABSTRACT);
    // init timers
    g_mnld_ctx.gps_status.timer_start = init_timer(mnld_gps_start_timeout);
    if (g_mnld_ctx.gps_status.timer_start == (timer_t)-1) {
        LOGE("init_timer(mnld_gps_start_timeout) failed");
        return -1;
    }

    g_mnld_ctx.gps_status.timer_stop = init_timer(mnld_gps_stop_timeout);
    if (g_mnld_ctx.gps_status.timer_stop == (timer_t)-1) {
        LOGE("init_timer(mnld_gps_stop_timeout) failed");
        return -1;
    }

    g_mnld_ctx.gps_status.timer_reset = init_timer(mnld_gps_reset_timeout);
    if (g_mnld_ctx.gps_status.timer_reset == (timer_t)-1) {
        LOGE("init_timer(mnld_gps_reset_timeout) failed");
        return -1;
    }

    g_mnld_ctx.gps_status.timer_nmea_monitor = init_timer(gps_mnld_restart_mnl_process);
    if (g_mnld_ctx.gps_status.timer_nmea_monitor == (timer_t)-1) {
        LOGE("init_timer(gps_mnld_restart_mnl_process) failed");
        return -1;
    }

    g_mnld_ctx.gps_status.timer_switch_ofl_mode = init_timer(mnld_gps_switch_ofl_mode_timeout);
    if (g_mnld_ctx.gps_status.timer_switch_ofl_mode == (timer_t)-1) {
        LOGE("init_timer(mnld_gps_switch_ofl_mode_timeout) failed");
        return -1;
    }

#if defined(GPS_SUSPEND_SUPPORT)
    g_mnld_ctx.gps_status.is_suspend_timer_running = false;
    g_mnld_ctx.gps_status.timer_suspend = init_timer(mnld_gps_suspend_timeout_callback);
    if (g_mnld_ctx.gps_status.timer_suspend == (timer_t)-1) {
        LOGE("init_timer(mnld_gps_suspend_timeout) failed");
        return -1;
    }
#endif
    // set screen status to unknown until screen monitor thread notify the real status
    g_mnld_ctx.screen_status = SCREEN_STATUS_UNKNOWN;

    // init threads
    pthread_create(&pthread_main, NULL, mnld_main_thread, NULL);

    // send the reboot message to the related modules
    mnl2hal_release_wakelock();
    mnl2hal_mnld_reboot();
    mnl2agps_mnl_reboot();
    mnl2flp_mnld_reboot();
    mnl2gfc_mnld_reboot();
    Mnld2DebugInterface_mnldUpdateReboot(&g_mnld_ctx.fds.fd_debug_client);
    mnld2logd_close_gpslog();
    mnld2logd_close_mpelog();
    mnld2logd_close_dumplog();
    mnld_gps_update_name();
    return 0;
}

bool mnld_is_gps_started() {
    if (g_mnld_ctx.gps_status.gps_state == MNLD_GPS_STATE_STARTING ||
        g_mnld_ctx.gps_status.gps_state == MNLD_GPS_STATE_STARTED) {
        return true;
    } else {
        return false;
    }
}

bool mnld_is_gps_or_ofl_started() {
    if (mnld_is_gps_started() ||
        g_mnld_ctx.gps_status.gps_state == MNLD_GPS_STATE_OFL_STARTING ||
        g_mnld_ctx.gps_status.gps_state == MNLD_GPS_STATE_OFL_STARTED) {
        return true;
    } else {
        return false;
    }
}

bool mnld_is_gps_started_done() {
    if (g_mnld_ctx.gps_status.gps_state == MNLD_GPS_STATE_STARTED) {
        return true;
    } else {
        return false;
    }
}

bool mnld_is_gps_or_ofl_started_done() {
    if (mnld_is_gps_started_done() ||
        g_mnld_ctx.gps_status.gps_state == MNLD_GPS_STATE_OFL_STARTED) {
        return true;
    } else {
        return false;
    }
}

bool mnld_is_gps_meas_enabled() {
    return g_mnld_ctx.gps_status.is_gps_meas_enabled;
}

bool mnld_is_gps_navi_enabled() {
    return g_mnld_ctx.gps_status.is_gps_navi_enabled;
}

// Due to gps_state extends from 4 states to 7 states (3 more for GPS offload),
// mnld_is_gps_stopped might be overdue. Considering to deprecate this API,
// and use mnld_is_gps_and_ofl_stopped as replace or change it.
bool mnld_is_gps_stopped() {
    if (g_mnld_ctx.gps_status.gps_state == MNLD_GPS_STATE_IDLE ||
        g_mnld_ctx.gps_status.gps_state == MNLD_GPS_STATE_STOPPING) {
        return true;
    } else {
        return false;
    }
}

bool mnld_is_gps_and_ofl_stopped() {
    if (g_mnld_ctx.gps_status.gps_state == MNLD_GPS_STATE_IDLE ||
        g_mnld_ctx.gps_status.gps_state == MNLD_GPS_STATE_STOPPING ||
        g_mnld_ctx.gps_status.gps_state == MNLD_GPS_STATE_OFL_STOPPING) {
        return true;
    } else {
        return false;
    }
}

#if defined(GPS_SUSPEND_SUPPORT)
bool mnld_is_gps_suspend() {
    if (g_mnld_ctx.gps_status.gps_state == MNLD_GPS_STATE_SUSPEND) {
        return true;
    } else {
        return false;
    }
}
#endif

bool mnld_is_epo_download_finished() {
    if (g_mnld_ctx.epo_status.is_epo_downloading == false) {
        return true;
    } else {
        return false;
    }
}

int main(int argc, char** argv) {
    LOGD("mnld version=0.03, offload ver=%s\n", OFL_VER);
    memset(&g_mnld_ctx, 0, sizeof(g_mnld_ctx));
    if ((argc >= 3)  //Parameter count check
        && (!strncmp(argv[2], "meta", 4) || !strncmp(argv[2], "factory", 7)
        || !strncmp(argv[2], "test", 4) || !strncmp(argv[2], "PDNTest", 7))) {
        in_meta_factory = 1;
    }
    chip_detector();
    /*get gps_epo_type begin*/
    if (strcmp(chip_id, "0x6572") == 0 || strcmp(chip_id, "0x6582") == 0 || strcmp(chip_id, "0x6570") == 0 ||
        strcmp(chip_id, "0x6580") == 0 || strcmp(chip_id, "0x6592") == 0 || strcmp(chip_id, "0x6571") == 0 ||
        strcmp(chip_id, "0x8127") == 0 || strcmp(chip_id, "0x0335") == 0 ||strcmp(chip_id, "0x8163") == 0) {
        gps_epo_type = 1;    // GPS only
    } else if (strcmp(chip_id, "0x6630") == 0 || strcmp(chip_id, "0x6752") == 0 || strcmp(chip_id, "0x6755") == 0
        || strcmp(chip_id, "0x6797") == 0 || strcmp(chip_id, "0x6632") == 0 || strcmp(chip_id, "0x6759") == 0
        || strcmp(chip_id, "0x6763") == 0 || strcmp(chip_id, "0x6758") == 0 || strcmp(chip_id, "0x6739") == 0
        || strcmp(chip_id, "0x6771") == 0 || strcmp(chip_id, "0x6775") == 0 || strcmp(chip_id, "0x6765") == 0
        || strcmp(chip_id, "0x3967") == 0 || strcmp(chip_id, "0x6761") == 0 || strcmp(chip_id, "0x6779") == 0
        || strcmp(chip_id, "0x6768") == 0 || strcmp(chip_id, "0x6885") == 0 || strcmp(chip_id, "0x6785") == 0) {
        gps_epo_type = 0;   // G+G
    } else {
        gps_epo_type = 0;   // Default is G+G
    }

    /*get initial GNSS OP Mode*/
    get_chip_gnss_op_mode();

    /*get gps_epo_type end*/
    mnld_offload_check_capability();
#if defined(GPS_SUSPEND_SUPPORT)
    mnld_gps_suspend_check_capability();
#endif
    if (mnl_init()) {
        LOGE("mnl_init: %d (%s)\n", errno, strerror(errno));
    }
    if (mnl_offload_is_enabled() == 1) {
        gps_emi_init(0);
    }
    if (adc_capture_is_enabled() == 1 && mnl_offload_is_enabled() == 0) {
        gps_emi_init(1);
    }
    if ((argc >= 3)  //Parameter count check
        && (!strncmp(argv[2], "meta", 4) || !strncmp(argv[2], "factory", 7)
        || !strncmp(argv[2], "test", 4) || !strncmp(argv[2], "PDNTest", 7))) {
        mnld_factory_test_entry(argc, argv);
    } else {
        gps_control_init();
        epo_downloader_init();
        qepo_downloader_init();
        mtknav_downloader_init();
        op01_log_init();
        mpe_function_init();
        mnld_init();
        // For MNL5.9 and later version, we move flp_monitor_init after mnld_init,
        // creating that the monitor thread after mnld msg socket initialized,
        // then the screen status can be send to mnld main thread without concern
        // of msg dropping because socket not ready.
        flp_monitor_init();
#ifdef __TEST__
        mnld_test_start();
#else
        block_here();
        //Will go here after calling mnld_block_exit
        mnld_dump_exit();
#endif
    }
/*
    LOGD("sizeof(mnld_context)=%d", sizeof(mnld_context));  // 48
    LOGD("sizeof(gps_location)=%d", sizeof(gps_location));  // 56
    LOGD("sizeof(gnss_sv)=%d", sizeof(gnss_sv ));           // 20
    LOGD("sizeof(gnss_sv_info)=%d", sizeof(gnss_sv_info));  // 5124
    LOGD("sizeof(gps_data)=%d", sizeof(gps_data ));         // 7752
    LOGD("sizeof(gps_nav_msg)=%d", sizeof(gps_nav_msg));    // 80
*/
    return 0;
}

