
#ifndef __GPS2AGPS_INTERFACE_H__
#define __GPS2AGPS_INTERFACE_H__

#include <sys/socket.h>
#include "mnl_agps_interface.h"

typedef struct {
    void (*agps_reboot)();

    void (*agps_open_gps_req)(int show_gps_icon, mnl_agps_open_type open_type,
        mnl_agps_open_requestor requestor_type, const char* requestor_id,
        bool emergency_call);
    void (*agps_close_gps_req)();
    void (*agps_reset_gps_req)(int flags);
    void (*agps_open_gps_rejected)(mnl_agps_open_type open_type,
        mnl_agps_open_requestor requestor_type, const char* requestor_id,
        bool emergency_call);

    void (*agps_session_done)();

    void (*ni_notify)(int session_id, mnl_agps_ni_type ni_type, mnl_agps_notify_type type,
        const char* requestor_id, const char* client_name);
    void (*data_conn_req)(int ipaddr, int is_emergency);
    void (*data_conn_release)(mnl_agps_data_connection_type agps_type);

    void (*set_id_req)(int flags);
    void (*ref_loc_req)(int flags);

    void (*rcv_pmtk)(const char* pmtk);
    void (*gpevt)(gpevt_type type);

    void (*agps_location)(mnl_agps_agps_location* location);
    void (*ni_notify2)(int session_id, mnl_agps_ni_type ni_type, mnl_agps_notify_type type,
        const char* requestor_id, const char* client_name,
        mnl_agps_ni_encoding_type requestor_id_encoding, mnl_agps_ni_encoding_type client_name_encoding);
    void (*data_conn_req2)(struct sockaddr_storage* addr, int is_emergency, mnl_agps_data_connection_type agps_type);
    void (*agps_settings_sync)(mnl_agps_agps_settings* settings);
    void (*vzw_debug_screen_output)(const char* str);
    void (*lppe_assist_common_iono) (const char* data, int len);   // refer to gnss_ha_common_ionospheric_model_struct
    void (*lppe_assist_common_trop) (const char* data, int len);   // refer to gnss_ha_common_troposphere_model_struct
    void (*lppe_assist_common_alt) (const char* data, int len);    // refer to gnss_ha_common_altitude_assist_struct
    void (*lppe_assist_common_solar) (const char* data, int len);  // refer to gnss_ha_common_solar_radiation_struct
    void (*lppe_assist_common_ccp) (const char* data, int len);    // refer to gnss_ha_common_ccp_assist_struct
    void (*lppe_assist_generic_ccp) (const char* data, int len);   // refer to gnss_ha_generic_ccp_assist_struct
    void (*lppe_assist_generic_dm) (const char* data, int len);    // refer to gnss_ha_generic_degradation_model_struct
} agps2mnl_interface;

// MNL -> AGPS
int mnl2agps_mnl_reboot();

int mnl2agps_open_gps_done();
int mnl2agps_close_gps_done();
int mnl2agps_reset_gps_done();

int mnl2agps_gps_init();
int mnl2agps_gps_cleanup();
// type:AGpsType
int mnl2agps_set_server(int type, const char* hostname, int port);
// flags:GpsAidingData
int mnl2agps_delete_aiding_data(int flags);
int mnl2agps_gps_open(int assist_req);
int mnl2agps_gps_close();
int mnl2agps_data_conn_open(const char* apn);
int mnl2agps_data_conn_open_ip_type(const char* apn, int ip_type, bool network_handle_valid, uint64_t network_handle);
int mnl2agps_data_conn_failed();
int mnl2agps_data_conn_closed();
int mnl2agps_ni_message(const char* msg, int len);
int mnl2agps_ni_respond(int session_id, int user_response);
int mnl2agps_set_ref_loc(int type, int mcc, int mnc, int lac, int cid);
int mnl2agps_set_set_id(int type, const char* setid);
int mnl2agps_update_network_state(int connected, int type, int roaming, const char* extra_info);
int mnl2agps_update_network_availability(int avaiable, const char* apn);
int mnl2agps_install_certificates(int index, int total, const char* data, int len);
int mnl2agps_revoke_certificates(const char* data, int len);
int mnl2agps_update_gnss_access_control(bool allowed);

int mnl2agps_pmtk(const char* pmtk);
int mnl2agps_raw_dbg(int enabled);
int mnl2agps_reaiding_req();
int mnl2agps_location_sync(double lat, double lng, int acc, bool alt_valid, float alt, bool source_valid, bool source_gnss, bool source_nlp, bool source_sensor);
int mnl2agps_agps_settings_ack(mnl_agps_gnss_settings* settings);
int mnl2agps_vzw_debug_screen_enable(int enabled);
int mnl2agps_set_server_extension(int type, const char* hostname, int port, int ssl, int ssl_version, int ssl_type);
int mnl2agps_update_configuration(const char* config_data, int length);
int mnl2agps_supl_host_ip(const char* ip);
int mnl2agps_tc10_lpp_support(int support_bitmask);
int mnl2agps_lppe_assist_data_req(const char* data, int len);           // refer to gnss_ha_assist_data_req_struct
int mnl2agps_lppe_assist_data_provide_ack(const char* data, int len);   // refer to gnss_ha_assist_ack_struct
int mnl2agps_set_position_mode(int pos_mode);
int mnl2agps_md_time_sync_req(const char* data, int len);
int mnl2agps_md_time_sync_rsp(const char* data, int len);
int mnl2agps_md_time_info_rsp(const char* data, int len);
int mnl2agps_update_network_state_with_handle(uint64_t network_handle, bool is_connected, unsigned short capabilities, const char* apn);

void agps2mnl_hdlr(int fd, agps2mnl_interface* mnl_interface);


int create_agps2mnl_fd();

#endif

