#ifndef __GPS_AGPS_INTERFACE_H__
#define __GPS_AGPS_INTERFACE_H__

// #include <hardware/gps.h>

#if defined(__ANDROID_OS__)
#include <android/log.h>
#endif
#include <stdbool.h>

#if defined(ANDROID)
#define AGPS_TO_MNL "/data/agps_supl/agps_to_mnl"
#define MNL_TO_AGPS "/data/agps_supl/mnl_to_agps"
#else
#define AGPS_TO_MNL "agps_to_mnl"
#define MNL_TO_AGPS "mnl_to_agps"
#endif

#define MNL_AGPS_MAX_BUFF_SIZE (64 * 1024)

#define MNL_AGPS_INTERFACE_VERSION 1

#define REQUEST_SETID_IMSI     (1<<0L)
#define REQUEST_SETID_MSISDN   (1<<1L)

#define REQUEST_REFLOC_CELLID  (1<<0L)
#define REQUEST_REFLOC_MAC     (1<<1L)   // not ready

#define MNLD_STRNCPY(dst,src,size) do{\
                                       strncpy((char *)(dst), (src), (size - 1));\
                                      (dst)[size - 1] = '\0';\
                                     }while(0)

typedef enum {
    MNL_AGPS_NI_TYPE_VOICE = 1,
    MNL_AGPS_NI_TYPE_UMTS_SUPL,
    MNL_AGPS_NI_TYPE_UMTS_CTRL_PLANE,
    MNL_AGPS_NI_TYPE_EMERGENCY_SUPL,
} mnl_agps_ni_type;

typedef enum {
    MNL_AGPS_NOTIFY_TYPE_NONE = 0,
    MNL_AGPS_NOTIFY_TYPE_NOTIFY_ONLY,
    MNL_AGPS_NOTIFY_TYPE_NOTIFY_ALLOW_NO_ANSWER,
    MNL_AGPS_NOTIFY_TYPE_NOTIFY_DENY_NO_ANSWER,
    MNL_AGPS_NOTIFY_TYPE_PRIVACY,
} mnl_agps_notify_type;

typedef enum {
    MNL_AGPS_NI_ENCODING_TYPE_NONE = 0,
    MNL_AGPS_NI_ENCODING_TYPE_GSM7,
    MNL_AGPS_NI_ENCODING_TYPE_UTF8,
    MNL_AGPS_NI_ENCODING_TYPE_UCS2,
} mnl_agps_ni_encoding_type;

typedef enum {
  GPEVT_TYPE_UNKNOWN = 0,                      //  0
  GPEVT_SUPL_SLP_CONNECT_BEGIN,                //  1
  GPEVT_SUPL_SLP_CONNECTED,                    //  2
  GPEVT_SUPL_SSL_CONNECT_BEGIN,                //  3
  GPEVT_SUPL_SSL_CONNECTED,                    //  4
  GPEVT_SUPL_ASSIST_DATA_RECEIVED,             //  5
  GPEVT_SUPL_ASSIST_DATA_VALID,                //  6
  GPEVT_SUPL_FIRST_POS_FIX,                    //  7
  GPEVT_SUPL_MEAS_TIME_OUT,                    //  8
  GPEVT_SUPL_MEAS_RESPONSE_SENT,               //  9
  GPEVT_SUPL_SSL_CLOSED,                       // 10
  GPEVT_SUPL_SLP_DISCONNECTED,                 // 11

  GPEVT_CP_MOLR_SENT,                          // 12
  GPEVT_CP_MTLR_RECEIVED,                      // 13
  GPEVT_CP_ASSIST_DATA_RECEIVED,               // 14
  GPEVT_CP_ASSIST_DATA_VALID,                  // 15
  GPEVT_CP_FIRST_POS_FIX,                      // 16
  GPEVT_CP_MEAS_TIME_OUT,                      // 17
  GPEVT_CP_MEAS_RESPONSE_SENT,                 // 18

  GPEVT_GNSS_HW_START,                         // 19
  GPEVT_GNSS_HW_STOP,                          // 20
  GPEVT_GNSS_RESET_STORED_SATELLITE_DATA,      // 21

  GPEVT_EPO_SERVER_CONNECT_BEGIN,              // 22
  GPEVT_EPO_SERVER_CONNECTED,                  // 23
  GPEVT_EPO_DATA_RECEIVED,                     // 24
  GPEVT_EPO_SERVER_DISCONNECTED,               // 25
  GPEVT_EPO_DATA_VALID,                        // 26

  GPEVT_HOT_STILL_DATA_VALID,                  // 27

  GPEVT_TYPE_MAX                               // 28
}gpevt_type;

typedef enum {
  MNL_AGPS_OPEN_TYPE_UNKNOWN = 0,              // 0 (i.e., old agpsd)

  MNL_AGPS_OPEN_TYPE_C2K = 0x10,               // 0x10

  MNL_AGPS_OPEN_TYPE_SUPL = 0x20,              // 0x20

  MNL_AGPS_OPEN_TYPE_CP_NILR = 0x30,           // 0x30
  MNL_AGPS_OPEN_TYPE_CP_MTLR,                  // 0x31
  MNL_AGPS_OPEN_TYPE_CP_MOLR,                  // 0x32
  MNL_AGPS_OPEN_TYPE_CP_QUERY,                 // 0x33
  MNL_AGPS_OPEN_TYPE_CP_MLR                    // 0x34
} mnl_agps_open_type;

typedef enum {
  MNL_AGPS_OPEN_REQUESTOR_UNKNOWN = 0,          // 0
  MNL_AGPS_OPEN_REQUESTOR_CARRIER,              // 1
  MNL_AGPS_OPEN_REQUESTOR_OEM,                  // 2
  MNL_AGPS_OPEN_REQUESTOR_MODEM_CHIPSET_VENDOR, // 3
  MNL_AGPS_OPEN_REQUESTOR_GNSS_CHIPSET_VENDOR,  // 4
  MNL_AGPS_OPEN_REQUESTOR_OTHER_CHIPSET_VENDOR, // 5
  MNL_AGPS_OPEN_REQUESTOR_AUTOMOBILE_CLIENT,    // 6
  MNL_AGPS_OPEN_REQUESTOR_OTHER_REQUESTOR       // 7
} mnl_agps_open_requestor;                      // 8

typedef enum {
  MNL_AGPS_DATA_CONNECT_TYPE_SUPL         = 1,
  MNL_AGPS_DATA_CONNECT_TYPE_C2K          = 2,
  MNL_AGPS_DATA_CONNECT_TYPE_SUPL_EIMS    = 3,
  MNL_AGPS_DATA_CONNECT_TYPE_SUPL_IMS     = 4,
} mnl_agps_data_connection_type;

typedef enum {
  MNL_AGPS_NETWORK_CAPABILITY_NOT_METERED = 1,  // Network is not metered.
  MNL_AGPS_NETWORK_CAPABILITY_NOT_ROAMING = 2,  // Network is not roaming.
} mnl_agps_network_capability_type;

typedef enum {
    //---------------------------------------
    // MNL -> AGPS
    MNL_AGPS_TYPE_MNL_REBOOT  = 100,

    MNL_AGPS_TYPE_AGPS_OPEN_GPS_DONE = 110,
    MNL_AGPS_TYPE_AGPS_CLOSE_GPS_DONE,
    MNL_AGPS_TYPE_AGPS_RESET_GPS_DONE,

    MNL_AGPS_TYPE_GPS_INIT = 120,
    MNL_AGPS_TYPE_GPS_CLEANUP,
    MNL_AGPS_TYPE_DELETE_AIDING_DATA,
    MNL_AGPS_TYPE_GPS_OPEN,
    MNL_AGPS_TYPE_GPS_CLOSE,
    MNL_AGPS_TYPE_DATA_CONN_OPEN,
    MNL_AGPS_TYPE_DATA_CONN_FAILED,
    MNL_AGPS_TYPE_DATA_CONN_CLOSED,
    MNL_AGPS_TYPE_NI_MESSAGE,
    MNL_AGPS_TYPE_NI_RESPOND,
    MNL_AGPS_TYPE_SET_REF_LOC,
    MNL_AGPS_TYPE_SET_SET_ID,
    MNL_AGPS_TYPE_SET_SERVER,
    MNL_AGPS_TYPE_UPDATE_NETWORK_STATE,
    MNL_AGPS_TYPE_UPDATE_NETWORK_AVAILABILITY,
    MNL_AGPS_TYPE_DATA_CONN_OPEN_IP_TYPE,
    MNL_AGPS_TYPE_INSTALL_CERTIFICATES,
    MNL_AGPS_TYPE_REVOKE_CERTIFICATES,
    MNL_AGPS_TYPE_UPDATE_GNSS_ACCESS_CONTROL,

    MNL_AGPS_TYPE_MNL2AGPS_PMTK = 150,
    MNL_AGPS_TYPE_RAW_DBG,
    MNL_AGPS_TYPE_REAIDING,
    MNL_AGPS_TYPE_LOCATION_SYNC,
    MNL_AGPS_TYPE_SETTINGS_ACK,
    MNL_AGPS_TYPE_VZW_DEBUG_SCREEN_ENABLE,
    MNL_AGPS_TYPE_SET_SERVER_EXTENSION,
    MNL_AGPS_TYPE_UPDATE_CONFIGURATION,
    MNL_AGPS_TYPE_SUPL_HOST_IP,
    MNL_AGPS_TYPE_TC10_LPP_SUPPORT,
    MNL_AGPS_TYPE_LPPE_ASSIST_DATA_REQ,             // 160, refer to gnss_ha_assist_data_request_ind_struct
    MNL_AGPS_TYPE_LPPE_ASSIST_DATA_PROVIDE_ACK,     // refer to gnss_ha_assist_ack_struct
    MNL_AGPS_TYPE_SET_POSITION_MODE,

    MNL_AGPS_TYPE_MD_TIME_SYNC_REQ,                 // Case 1: MNL-Initiated
    MNL_AGPS_TYPE_MD_TIME_SYNC_RSP,                 // Case 2: ack to MD-Initiated
    MNL_AGPS_TYPE_MD_TIME_INFO_RSP,                 // OK to have a location with time sync
    MNL_AGPS_TYPE_UPDATE_NETWORK_STATE_WITH_HANDLE, // for GNSS HIDL 2.0 to update network state with handler

    // ---------------------------------------
    // AGPS -> MNL
    MNL_AGPS_TYPE_AGPS_REBOOT    = 200,

    MNL_AGPS_TYPE_AGPS_OPEN_GPS_REQ = 210,
    MNL_AGPS_TYPE_AGPS_CLOSE_GPS_REQ,
    MNL_AGPS_TYPE_AGPS_RESET_GPS_REQ,
    MNL_AGPS_TYPE_AGPS_OPEN_GPS_REJECTED,

    MNL_AGPS_TYPE_AGPS_SESSION_DONE = 220,

    MNL_AGPS_TYPE_NI_NOTIFY = 230,
    MNL_AGPS_TYPE_DATA_CONN_REQ,
    MNL_AGPS_TYPE_DATA_CONN_RELEASE,
    MNL_AGPS_TYPE_SET_ID_REQ,
    MNL_AGPS_TYPE_REF_LOC_REQ,
    MNL_AGPS_TYPE_AGPS_LOC,
    MNL_AGPS_TYPE_NI_NOTIFY_2,
    MNL_AGPS_TYPE_DATA_CONN_REQ2,
    MNL_AGPS_TYPE_SUPL_DNS_REQ,

    MNL_AGPS_TYPE_AGPS2MNL_PMTK = 250,
    MNL_AGPS_TYPE_GPEVT,
    MNL_AGPS_TYPE_SETTINGS_SYNC,
    MNL_AGPS_TYPE_VZW_DEBUG_SCREEN_OUTPUT,
    MNL_AGPS_TYPE_CELL_LOCATION,
    MNL_AGPS_TYPE_LPPE_ASSIST_COMMON_IONO, // refer to gnss_ha_common_ionospheric_model_struct
    MNL_AGPS_TYPE_LPPE_ASSIST_COMMON_TROP, // refer to gnss_ha_common_troposphere_model_struct
    MNL_AGPS_TYPE_LPPE_ASSIST_COMMON_ALT,  // refer to gnss_ha_common_altitude_assist_struct
    MNL_AGPS_TYPE_LPPE_ASSIST_COMMON_SOLAR,// refer to gnss_ha_common_solar_radiation_struct
    MNL_AGPS_TYPE_LPPE_ASSIST_COMMON_CCP,  // refer to gnss_ha_common_ccp_assist_struct
    MNL_AGPS_TYPE_LPPE_ASSIST_GENERIC_CCP, // refer to gnss_ha_generic_ccp_assist_struct
    MNL_AGPS_TYPE_LPPE_ASSIST_GENERIC_DM,  // refer to gnss_ha_generic_degradation_model_struct
} mnl_agps_type;
typedef enum{
    AGPS_MD_HUGE_DATA_TYPE_LPPE_AGNSS_PROVIDE_ASSIST_DATA_COMMON_IONO       = 610,  //MD -> AP  gnss_ha_common_ionospheric_model_struct
    AGPS_MD_HUGE_DATA_TYPE_LPPE_AGNSS_PROVIDE_ASSIST_DATA_COMMON_TROP       = 611,  //MD -> AP  gnss_ha_common_troposphere_model_struct
    AGPS_MD_HUGE_DATA_TYPE_LPPE_AGNSS_PROVIDE_ASSIST_DATA_COMMON_ALT        = 612,  //MD -> AP  gnss_ha_common_altitude_assist_struct
    AGPS_MD_HUGE_DATA_TYPE_LPPE_AGNSS_PROVIDE_ASSIST_DATA_COMMON_SOLAR      = 613,  //MD -> AP  gnss_ha_common_solar_radiation_struct
    AGPS_MD_HUGE_DATA_TYPE_LPPE_AGNSS_PROVIDE_ASSIST_DATA_COMMON_CCP        = 614,  //MD -> AP  gnss_ha_common_ccp_assist_struct
    AGPS_MD_HUGE_DATA_TYPE_LPPE_AGNSS_PROVIDE_ASSIST_DATA_GENERIC_CCP       = 630,  //MD -> AP  gnss_ha_generic_ccp_assist_struct
    AGPS_MD_HUGE_DATA_TYPE_LPPE_AGNSS_PROVIDE_ASSIST_DATA_GENERIC_DM        = 631,  //MD -> AP  gnss_ha_generic_degradation_model_struct
    AGPS_MD_HUGE_DATA_TYPE_LPPE_AGNSS_PROVIDE_ASSIST_DATA_ACK               = 650,  //AP -> MD  lpp_lppe_ha_gn
}lppe_ack_type;
typedef struct {
    double          latitude;           // Represents latitude in degrees
    double          longitude;          // Represents longitude in degrees
    char            altitude_used;      // 0=disabled 1=enabled
    double          altitude;           // Represents altitude in meters above the WGS 84 reference
    char            speed_used;         // 0=disabled 1=enabled
    float           speed;              // Represents speed in meters per second
    char            bearing_used;       // 0=disabled 1=enabled
    float           bearing;            // Represents heading in degrees
    char            accuracy_used;      // 0=disabled 1=enabled
    float           accuracy;           // Represents expected accuracy in meters
    char            timestamp_used;     // 0=disabled 1=enabled
    long long       timestamp;          // Milliseconds since January 1, 1970
} mnl_agps_agps_location;

typedef struct {
    char            sib8_16_enable;
    char            gps_satellite_enable;
    char            glonass_satellite_enable;
    char            beidou_satellite_enable;
    char            galileo_satellite_enable;
    char            a_glonass_satellite_enable;
    char            a_gps_satellite_enable;
    char            a_beidou_satellite_enable;
    char            a_galileo_satellite_enable;
    char            lppe_enable;
} mnl_agps_agps_settings;

typedef struct {
    char            gps_satellite_support;
    char            glonass_satellite_support;
    char            beidou_satellite_support;
    char            galileo_satellite_support;
} mnl_agps_gnss_settings;

const char* get_mnl_agps_type_str(mnl_agps_type input);

#endif

