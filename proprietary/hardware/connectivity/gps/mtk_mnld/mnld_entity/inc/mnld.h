#ifndef __MNLD_H__
#define __MNLD_H__

#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include "mnl2hal_interface.h"
#include "mtk_socket_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MNLD_MAIN_SOCKET                "mnld_main_socket"
#define MNLD_GPS_CONTROL_SOCKET         "mnld_gps_control_socket"
#define MNLD_EPO_DOWNLOAD_SOCKET        "mnld_epo_download_socket"
#define MNLD_QEPO_DOWNLOAD_SOCKET       "mnld_qepo_download_socket"
#define MNLD_MTKNAV_DOWNLOAD_SOCKET     "mnld_mtknav_download_socket"
#define MNLD_OP01_LOG_WRITER_SOCKET     "mnld_op01_log_write_socket"
#define MNLD_TO_NLP_UTILS_SOCKET        "mtk_mnld2nlputils"
#define MNLD_MPE_SOCKET                 "mnld_mpe_socket"
#define META_TO_MNLD_SOCKET             "mtk_meta2mnld"
#define DEBUG_TO_MNLD_SOCKET            "mtk_debugService2mnld"
#define MNLD_TO_DEBUG_SOCKET            "mtk_mnld2debugService"
#define MNLD_TO_MTKLOGGER_SOCKET        "mtk_mnld2mtklogger"
#define MTKLOGGER_TO_MNLD_SOCKET        "mtk_mtklogger2mnld"
#define LOG_HIDL_INTERFACE              "mtk_lbs_log_v2s"
#define MNLD_NFW_PROXY_NAME    "com.mediatek.gnss.nonframeworklbs"

#define NLP_REQUEST_SRC_MNL             (1 << 0)
#define NLP_REQUEST_SRC_APM             (1 << 1)

#if defined(__ANDROID_OS__)
#define MNLD_OP01_LOG_PATH              "/sdcard/GPS.LOG"
#else
#define MNLD_OP01_LOG_PATH              "GPS.LOG"
#endif

#define MNLD_MAIN_HANDLER_TIMEOUT       (30 * 1000)
#define MNLD_GPS_HANDLER_TIMEOUT        (30 * 1000)
#define MNLD_EPO_HANDLER_TIMEOUT        (120 * 60 * 1000)
#define MNLD_EPO_RETRY_HANDLER_TIMEOUT  (30 * 1000)
#define MNLD_QEPO_HANDLER_TIMEOUT       (120 * 60 * 1000)
#define MNLD_OP01_HANDLER_TIMEOUT       (20 * 1000)
#define MNLD_MPE_HANDLER_TIMEOUT        (30 * 1000)
#define MNLD_GPS_START_TIMEOUT          (35 * 1000)
#define MNLD_GPS_STOP_TIMEOUT           (30 * 1000)
#define MNLD_GPS_RESET_TIMEOUT          (30 * 1000)
#define MNLD_GPS_NMEA_DATA_TIMEOUT      (10 * 1000)
#define MNLD_GPS_LOG_HANDLER_TIMEOUT    (60 * 1000)
#define MNLD_GPS_SWITCH_OFL_MODE_TIMEOUT (30 * 1000)

//Now it's 0, standing for change SUSPEND to CLOSE immediately when screen off
#define MNLD_GPS_SUSPEND_TIMEOUT        (0)


#define MNLD_INTERNAL_BUFF_SIZE         (8 * 1024)
#define MNLD_TO_APP_BUFF_SIZE           (1023)

#define MNLD_NFW_USER_NAME_LEN (64)
#define MNLD_NFW_USER_NUM_MAX (10)

typedef enum {
    MNLD_NFW_USER_AGPS = 0,
    MNLD_NFW_USER_NUM
} MNLD_NFW_USER;

typedef enum {
    MNLD_GPS_STATE_IDLE         = 0,
    MNLD_GPS_STATE_STARTING     = 1,
    MNLD_GPS_STATE_STARTED      = 2,
    MNLD_GPS_STATE_STOPPING     = 3,
    MNLD_GPS_STATE_OFL_STARTING = 4,
    MNLD_GPS_STATE_OFL_STARTED  = 5,
    MNLD_GPS_STATE_OFL_STOPPING = 6,
    MNLD_GPS_STATE_SUSPEND      = 7,
} mnld_gps_state;

typedef enum {
    GPS_EVENT_START         = 0,
    GPS_EVENT_STOP          = 1,
    GPS_EVENT_RESET         = 2,
    GPS_EVENT_START_DONE    = 3,
    GPS_EVENT_STOP_DONE     = 4,
    GPS_EVENT_OFFLOAD_START = 5,
    GPS_EVENT_SUSPEND       = 6,
    GPS_EVENT_SUSPEND_DONE  = 7,
    GPS_EVENT_SUSPEND_CLOSE = 8,
} mnld_gps_event;

typedef enum {
    MNLD_GPS_DO_HW_RESUME   = 0,
    MNLD_GPS_DO_HW_SUSPEND  = 1,
} mnld_gps_do_hw_ctrl_opcode;

typedef enum {
    GPS2MAIN_EVENT_START_DONE               = 0,
    GPS2MAIN_EVENT_STOP_DONE                = 1,
    GPS2MAIN_EVENT_RESET_DONE               = 2,
    GPS2MAIN_EVENT_NMEA_TIMEOUT             = 3,
    GPS2MAIN_EVENT_UPDATE_LOCATION          = 4,
    EPO2MAIN_EVENT_EPO_DONE                 = 5,
    QEPO2MAIN_EVENT_QEPO_DONE               = 6,
    QEPO2MAIN_EVENT_QEPO_BD_DONE            = 7,
    MTKNAV2MAIN_EVENT_MTKNAV_DONE           = 8,
    QEPO2MAIN_EVENT_QEPO_GA_DONE            = 9,
    GPS2MAIN_EVENT_OUTPUT_DATA              = 10,
    SCREEN2MAIN_EVENT_SCREEN_ON             = 11,
    SCREEN2MAIN_EVENT_SCREEN_OFF            = 12,
    SCREEN2MAIN_EVENT_SCREEN_UNKNOWN        = 13,
    GPS2MAIN_EVENT_SUSPEND_DONE             = 14,
    GPS2MAIN_EVENT_SUSPEND_TIMEOUT          = 15,
} main_internal_event;

typedef enum {
    DATA_DEBUG2APP               = 0,
    DATA_ENDLIST
} main_out_put_data_type;

typedef enum {
    EPO_DOWNLOAD_RESULT_SUCCESS          = 0,
    EPO_DOWNLOAD_RESULT_NO_UPDATE        = 1,
    EPO_DOWNLOAD_RESULT_FAIL             = -1,
    EPO_DOWNLOAD_RESULT_RETRY_TOO_MUCH   = -2,
} epo_download_result;

typedef enum {
    EPO_MD5_FILE_UPDATED                = 0,
    EPO_MD5_FILE_NO_UPDATE              = 1,
    EPO_MD5_DOWNLOAD_RESULT_FAIL        = -1,
} epo_md5_download_result;

typedef struct {
    int fd_hal;
    int fd_agps;
    int fd_flp;
    int fd_flp_test;
    int fd_geofence;
    int fd_at_cmd;
    int fd_int;
    int fd_mtklogger;
    mtk_socket_fd fd_mtklogger_client;
    int fd_meta;
    int fd_debug;
    mtk_socket_fd fd_nlp_utils;
    mtk_socket_fd fd_debug_client;
} mnld_fds;

typedef struct {
    bool        gps_used;
    bool        need_open_ack;
    bool        need_close_ack;
    bool        need_reset_ack;
} mnld_gps_client;

typedef struct {
    mnld_gps_client     hal;
    mnld_gps_client     agps;
    mnld_gps_client     flp;
    mnld_gps_client     flp_test;
    mnld_gps_client     geofence;
    mnld_gps_client     at_cmd_test;
    mnld_gps_client     factory;
    mnld_gps_client     raw_meas;  //RAW measurement
} mnld_gps_clients;

typedef struct {
    mnld_gps_clients    clients;
    mnld_gps_state      gps_state;
    bool                is_gps_init;
    bool                is_gps_meas_enabled;
    bool                is_gps_navi_enabled;
#if defined(GPS_SUSPEND_SUPPORT)
    bool                is_suspend_timer_running;
#endif
    bool                is_in_nmea_timeout_handler;
    int                 delete_aiding_flag;

    timer_t             timer_start;
    timer_t             timer_stop;
    timer_t             timer_reset;
    timer_t             timer_nmea_monitor;
    timer_t             timer_switch_ofl_mode;
#if defined(GPS_SUSPEND_SUPPORT)
    timer_t             timer_suspend;
#endif
    time_t              gps_start_time;
    time_t              gps_stop_time;
    time_t              gps_ttff;
    bool                wait_first_location;
} mnld_gps_status;

typedef struct {
    bool            is_network_connected;
    bool            is_wifi_connected;
    bool            is_epo_downloading;
} mnl_epo_status;

typedef enum {
    SCREEN_STATUS_UNKNOWN = -1,
    SCREEN_STATUS_OFF = 0,
    SCREEN_STATUS_ON = 1,
} mnld_screen_status;

typedef struct {
    mnld_fds            fds;
    mnld_gps_status     gps_status;
    mnl_epo_status      epo_status;
    mnld_screen_status  screen_status;
    bool                nfw_user_visibility[MNLD_NFW_USER_NUM];
} mnld_context;

// GPS Control -> MAIN
int sys_gps_mnl_data2mnld_callback(const char *data, unsigned int length);
void mnld_gps_output_data_handle(char* buff, int off_set);
int mnld_gps_start_done(bool is_assist_req);
int mnld_gps_stop_done();
int mnld_gps_reset_done();
int mnld_gps_update_location(gps_location location);

// EPO Download -> MAIN
int mnld_epo_download_done(epo_download_result result);
int mnld_qepo_download_done(epo_download_result result);
int mnld_mtknav_download_done(epo_download_result result);
int mnld_qepo_bd_download_done(epo_download_result result);
int mnld_qepo_ga_download_done(epo_download_result result);

void hal_start_gps_trigger_epo_download();
bool is_network_connected();
bool is_wifi_network_connected();

// Provided for GPS Control to check the status
bool mnld_is_gps_started();
bool mnld_is_gps_or_ofl_started();
bool mnld_is_gps_started_done();
bool mnld_is_gps_or_ofl_started_done();
bool mnld_is_gps_meas_enabled();
bool mnld_is_gps_navi_enabled();
bool mnld_is_gps_stopped();
#if defined(GPS_SUSPEND_SUPPORT)
bool mnld_is_gps_suspend();
#endif
bool mnld_is_gps_and_ofl_stopped();
bool mnld_is_epo_download_finished();

int mnld_gps_controller_mnl_nmea_timeout(void);
int mnld_gps_start_nmea_monitor(void);
int mnld_gps_stop_nmea_monitor(void);
void gps_mnld_restart_mnl_process(void);
void mnld_gps_request_nlp(int src);

int mtk_gps_get_gps_user(void);

void factory_mnld_gps_start(void);
void factory_mnld_gps_stop(void);

void flp_test2mnl_gps_start(void);
void flp_test2mnl_gps_stop(void);

int is_flp_user_exist();
int is_geofence_user_exist();

int mnld_screen_on_notify();
int mnld_screen_off_notify();
int mnld_screen_unknown_notify();

char mtk_gps_get_nfw_visibility(MNLD_NFW_USER nfw_user);
void mnld_nfw_set_default_notification(MNLD_NFW_USER usr);
void mnld_nfw_notify2hal(MNLD_NFW_USER usr, nfw_response_type response_type);
void mnld_nfw_notify2hal_default(MNLD_NFW_USER usr, nfw_response_type response_type);
void mnld_nfw_mnl2agps_location_sync(double lat, double lng, int acc, bool alt_valid, float alt, bool source_valid, bool source_gnss, bool source_nlp, bool source_sensor);
void mtk_gps_set_nfw_visibility(int usr, bool set_value);
void mtk_gps_set_nfw_visibility_all(bool set_value);

void mtk_gps_dump_nfw_visibility(void);
int mnld_show_icon_get(void);
void mnld_show_icon_set(int show_gps_icon);

#ifdef __cplusplus
}
#endif

#endif

