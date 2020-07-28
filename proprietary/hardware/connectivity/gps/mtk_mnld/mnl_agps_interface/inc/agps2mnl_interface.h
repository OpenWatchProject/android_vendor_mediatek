
#ifndef __AGPS2GPS_INTERFACE_H__
#define __AGPS2GPS_INTERFACE_H__

#include "mnl_agps_interface.h"

typedef struct {
    void (*mnl_reboot)();

    void (*open_gps_done)();
    void (*close_gps_done)();
    void (*reset_gps_done)();

    void (*gps_init)();
    void (*gps_cleanup)();
    void (*set_server)(int type, const char* hostname, int port);
    void (*delete_aiding_data)(int flags);
    void (*gps_open)(int assist_req);
    void (*gps_close)();
    void (*data_conn_open)(const char* apn);
    void (*data_conn_failed)();
    void (*data_conn_closed)();

    void (*ni_message)(const char* msg, int len);
    void (*ni_respond)(int session_id, int user_response);

    void (*set_ref_loc)(int type, int mcc, int mnc, int lac, int cid);
    void (*set_set_id)(int type, const char* setid);

    void (*update_network_state)(int connected, int type, int roaming, const char* extra_info);
    void (*update_network_availability)(int avaiable, const char* apn);

    void (*rcv_pmtk)(const char* pmtk);
    void (*raw_dbg)(int enabled);
    void (*reaiding_req)();
    void (*data_conn_open_ip_type)(const char* apn, int ip_type, bool network_handle_valid, uint64_t network_handle);
    void (*install_certificates)(int index, int total, const char* data, int len);
    void (*revoke_certificates)(const char* data, int len);
    void (*location_sync)(double lat, double lng, int acc, bool alt_valid, float alt, bool source_valid, bool source_gnss, bool source_nlp, bool source_sensor);
    void (*agps_settings_ack) (mnl_agps_gnss_settings* settings);
    void (*vzw_debug_screen_enable) (int enabled);
    void (*set_server_extension) (int type, const char* hostname, int port, int ssl, int ssl_version, int ssl_type);
    void (*update_configuration) (const char* config_data, int length);
    void (*supl_host_ip) (const char* ip);
    void (*tc10_lpp_support) (int support_bitmask);
    void (*lppe_assist_data_req) (const char* data, int len);           // refer to gnss_ha_assist_data_req_struct
    void (*lppe_assist_data_provide_ack) (const char* data, int len);   // refer to gnss_ha_assist_ack_struct
    void (*set_position_mode)(int pos_mode);
    void (*update_gnss_access_control)(bool allowed);
    void (*md_time_sync_req)(const char* data, int len);
    void (*md_time_sync_rsp)(const char* data, int len);
    void (*md_time_info_rsp)(const char* data, int len);
    void (*update_network_state_with_handle)(uint64_t network_handle, bool is_connected, unsigned short capabilities, const char* apn);
} mnl2agps_interface;

int agps2mnl_agps_reboot();

int agps2mnl_agps_open_gps_req(int show_gps_icon, mnl_agps_open_type open_type,
    mnl_agps_open_requestor requestor_type, const char* requestor_id,
    bool emergency_call);
int agps2mnl_agps_close_gps_req();
int agps2mnl_agps_reset_gps_req(int flags);
int agps2mnl_agps_open_gps_rejected(mnl_agps_open_type open_type,
    mnl_agps_open_requestor requestor_type, const char* requestor_id,
    bool emergency_call);

int agps2mnl_agps_session_done();

int agps2mnl_ni_notify(int session_id, mnl_agps_ni_type ni_type, mnl_agps_notify_type type, const char* requestor_id, const char* client_name);
int agps2mnl_ni_notify2(int session_id, mnl_agps_ni_type ni_type, mnl_agps_notify_type type, const char* requestor_id, const char* client_name,
    mnl_agps_ni_encoding_type requestor_id_encoding, mnl_agps_ni_encoding_type client_name_encoding);
int agps2mnl_data_conn_req(int ipaddr, int is_emergency);
int agps2mnl_data_conn_req2(struct sockaddr_storage* addr, int is_emergency,
    mnl_agps_data_connection_type agps_type);
int agps2mnl_data_conn_release(mnl_agps_data_connection_type agps_type);
// flags refer to REQUEST_SETID_IMSI and REQUEST_SETID_MSISDN
int agps2mnl_set_id_req(int flags);
// flags refer to REQUEST_REFLOC_CELLID and REQUEST_REFLOC_MAC
int agps2mnl_ref_loc_req(int flags);

int agps2mnl_pmtk(const char* pmtk);
int agps2mnl_gpevt(gpevt_type type);

int agps2mnl_agps_location(mnl_agps_agps_location* location);
int agps2mnl_cell_location(mnl_agps_agps_location* location);

int agps2mnl_agps_settings_sync(mnl_agps_agps_settings* settings);
int agps2mnl_vzw_debug_screen_output(const char* str);
int agps2mnl_supl_dns_req(const char* fqdn);
int agps2mnl_lppe_assist_data_provide_common_iono(const char* data, int len);       // refer to gnss_ha_common_ionospheric_model_struct
int agps2mnl_lppe_assist_data_provide_common_trop(const char* data, int len);       // refer to gnss_ha_common_troposphere_model_struct
int agps2mnl_lppe_assist_data_provide_common_alt(const char* data, int len);        // refer to gnss_ha_common_altitude_assist_struct
int agps2mnl_lppe_assist_data_provide_common_solar(const char* data, int len);      // refer to gnss_ha_common_solar_radiation_struct
int agps2mnl_lppe_assist_data_provide_common_ccp(const char* data, int len);        // refer to gnss_ha_common_ccp_assist_struct
int agps2mnl_lppe_assist_data_provide_generic_ccp(const char* data, int len);       // refer to gnss_ha_generic_ccp_assist_struct
int agps2mnl_lppe_assist_data_provide_generic_dm(const char* data, int len);        // refer to gnss_ha_generic_degradation_model_struct
void mnl2agps_hdlr(int fd, mnl2agps_interface* agps_interface);

int create_mnl2agps_fd(const char* agps2mnl_path, const char* mnl2agps_path);

#endif

