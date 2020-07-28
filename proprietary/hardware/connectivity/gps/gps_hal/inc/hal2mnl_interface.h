#ifndef __HAL2MNL_INTERFACE_H__
#define __HAL2MNL_INTERFACE_H__

#include "hal_mnl_interface_common.h"
#include "hardware/gps_mtk.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void (*mnld_reboot)();

    void (*location)(gps_location location);
    void (*gps_status)(gps_status status);
    void (*gps_sv)(gnss_sv_info sv);
    void (*nmea)(int64_t timestamp, const char* nmea, int length);
    void (*gps_capabilities)(gps_capabilites capabilities);
    void (*gps_measurements)(gps_data data);
    void (*gps_navigation)(gps_nav_msg msg);
    void (*gnss_measurements)(gnss_data data);
    void (*gnss_navigation)(gnss_nav_msg msg);

    void (*request_wakelock)();
    void (*release_wakelock)();

    void (*request_utc_time)();

    void (*request_data_conn)(struct sockaddr_storage* addr, agps_type type);
    void (*release_data_conn)(agps_type type);
    void (*request_ni_notify)(int session_id, agps_ni_type ni_type,
        agps_notify_type type, const char* requestor_id,
        const char* client_name, ni_encoding_type requestor_id_encoding,
        ni_encoding_type client_name_encoding);
    void (*request_set_id)(request_setid flags);
    void (*request_ref_loc)(request_refloc flags);
    void (*output_vzw_debug)(const char* str);
    void (*update_gnss_name)(const char* name, int length);
    void (*request_nlp)(bool independentFromGnss, bool isUserEmergency);
    void (*nfw_access_notify)(NfwNotification nfw_notify);
} mnl2hal_interface;

int hal2mnl_hal_reboot();

int hal2mnl_gps_init();
int hal2mnl_gps_start();
int hal2mnl_gps_stop();
int hal2mnl_gps_cleanup();

int hal2mnl_gps_inject_time(int64_t time, int64_t time_reference, int uncertainty);
int hal2mnl_gps_inject_location(double lat, double lng, float acc);
int hal2mnl_gps_delete_aiding_data(int flags);
int hal2mnl_gps_set_position_mode(gps_pos_mode mode, gps_pos_recurrence recurrence,
    int min_interval, int preferred_acc, int preferred_time, bool lowPowerMode);

int hal2mnl_data_conn_open(const char* apn);
int hal2mnl_data_conn_open_with_apn_ip_type(uint64_t networkHandle, const char* apn, apn_ip_type ip_type);
int hal2mnl_data_conn_closed();
int hal2mnl_data_conn_failed();

int hal2mnl_set_server(agps_type type, const char* hostname, int port);
int hal2mnl_set_ref_location(cell_type type, int mcc, int mnc, int lac, int cid);
int hal2mnl_set_id(agps_id_type type, const char* setid);

int hal2mnl_ni_message(char* msg, int len);
int hal2mnl_ni_respond(int notif_id, ni_user_response_type user_response);

int hal2mnl_update_network_state(uint64_t networkHandle, bool isConnected,
            uint16_t capabilities, const char* apn);
int hal2mnl_update_network_availability(int available, const char* apn);

int hal2mnl_set_gps_measurement(bool enabled, bool enableFullTracking);
int hal2mnl_set_gps_navigation(bool enabled);

int hal2mnl_set_vzw_debug(bool enabled);
int hal2mnl_update_gnss_config(const char* config_data, int length);
int hal2mnl_setBlacklist(long long* blacklist, int32_t size);
int hal2mnl_set_correction(MeasurementCorrections* corrections);
int hal2mnl_setNfwAccess(char* proxyApps, int32_t length);

// -1 means failure
int mnl2hal_hdlr(int fd, mnl2hal_interface* hdlr);

// -1 means failure
int create_mnl2hal_fd();

#ifdef __cplusplus
}
#endif

#endif

