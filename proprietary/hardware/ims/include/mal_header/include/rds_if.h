#ifndef __RDS_IF_H__
#define __RDS_IF_H__

#include <pthread.h> // this header included in conn.h before

// #include "conn.h"
#include "datamngr_if.h"

#define RDS_VER_MAJOR 0
#define RDS_VER_MINOR 1
#define RDS_VERNO ((RDS_VER_MAJOR << 16) | RDS_VER_MINOR)

#define RDS_STR_LEN 32
#define RDS_FQDN_LEN 128 //[20151012] actual length need to confirm with Alan-YL
#define RDS_MCC_LEN 3
#define RDS_MNC_LEN 3
#define RDS_UNUSED(x) ((void)(x))
#define RDS_LOG_LEN 256

#define RDS_MAX_HOCNT 65535
#define WOS_CALL_CNT_MAX 16 // currently call max count is 7, range is 1~7
#define RDS_THRD_CNT_MAX 16

#define RDS_MAX_PDN_NUM 16
#define RDS_APNCNT 32

#define RDS_MPS_SUPPORT_MAX 4

#define RDS_SIP_CTX_ARGS_MAX 16

#define RDS_COUNTRY_DATA_LEN 3

#define RDS_IPSEC_POLICY_RAT "net.handover.rat"
#define RDS_THLTE_ECSQTH     "net.handover.thlte"
#define RDS_TH1xR_ECIOTH     "net.handover.th1x"

// GSM RIL Unify interface command - [REQ]
#define RIL_RDS_REQ_INITCONN         "RIL_RDS_INITCONN"
#define RIL_RDS_REQ_EXITCONN         "RIL_RDS_EXITCONN"
#define RIL_RDS_REQ_SETUPCALL        "RIL_RDS_SETUPCALL"
#define RIL_RDS_REQ_DEACTCALL        "RIL_RDS_DEACTCALL"
#define RIL_RDS_REQ_UTCMD            "RIL_RDS_UTCMD"
#define RIL_RDS_REQ_GET_HO_STATUS    "RIL_RDS_GET_HO_STATUS"
#define RIL_RDS_REQ_GET_RETRY_TIME   "RIL_RDS_GET_RETRY_TIME"
// WOS Unify interface command - [REQ]
#define WOS_RDS_REQ_INITCONN         "WOS_RDS_INITCONN"
#define WOS_RDS_REQ_EXITCONN         "WOS_RDS_EXITCONN"
#define WOS_RDS_REQ_GET_WIFI_STATUS  "WOS_RDS_GET_WIFI_STATUS"
#define WOS_RDS_REQ_GET_RAN_TYPE     "WOS_RDS_GET_RAN_TYPE"
#define WOS_RDS_REQ_GET_LAST_ERR     "WOS_RDS_GET_LAST_ERR"
#define WOS_RDS_REQ_SET_UI_PARAM     "WOS_RDS_SET_UI_PARAM"
#define WOS_RDS_REQ_SET_WIFI_STAT    "WOS_RDS_SET_WIFI_STAT"
#define WOS_RDS_REQ_SET_CALL_STAT    "WOS_RDS_SET_CALL_STAT"
#define WOS_RDS_REQ_SET_MD_STAT      "WOS_RDS_SET_MD_STAT"
#define WOS_RDS_REQ_SET_RADIO_STAT   "WOS_RDS_SET_RADIO_STAT"
#define WOS_RDS_REQ_SET_PING_STAT    "WOS_RDS_SET_PING_STAT"
#define WOS_RDS_REQ_WIFIMNTR         "WOS_RDS_WIFIMNTR"
#define WOS_RDS_REQ_SET_LOCATION     "WOS_RDS_SET_LOCATION"
#define WOS_RDS_REQ_SET_VOICE_MDSTAT "WOS_RDS_SET_VOICE_MDSTAT"
// WOS Unify interface command - [IND]
#define WOS_RDS_IND_GET_RAN_TYPE     "WOS_RDS_GET_RAN_TYPE"
#define WOS_RDS_IND_GET_LAST_ERR     "WOS_RDS_GET_LAST_ERR"
#define WOS_RDS_IND_HO_STATUS        "WOS_RDS_HO_STATUS"
#define WOS_RDS_IND_RVOUT            "WOS_RDS_RVOUT"
#define WOS_RDS_IND_WIFIPDN_ACT      "WOS_RDS_WIFIPDN_ACT"
#define WOS_RDS_IND_WIFIMON_CFG      "WOS_RDS_WIFIMON_CFG"
#define WOS_RDS_IND_WIFI_DISABLE     "WOS_RDS_WIFI_DISABLE"
#define WOS_RDS_IND_GET_LOCATION     "WOS_RDS_GET_LOCATION"
// VTS Unify interface command - [REQ]
#define VTS_RDS_REQ_SET_VTCALL_STAT  "VTS_RDS_SET_VTCALL_STAT"

/* =========================================== *
 * General Types							   *
 * =========================================== */
typedef char 					rds_char;
/* portable wide character for unicode character set */
typedef unsigned short			rds_wchar;
/* portable 8-bit unsigned integer */
typedef unsigned char           rds_uint8;
/* portable 8-bit signed integer */
typedef signed char             rds_int8;
/* portable 16-bit unsigned integer */
typedef unsigned short int      rds_uint16;
/* portable 16-bit signed integer */
typedef signed short int        rds_int16;
/* portable 32-bit unsigned integer */
typedef unsigned int            rds_uint32;
/* portable 32-bit signed integer */
typedef signed int              rds_int32;
/* portable 64-bit unsigned integer */
typedef unsigned long long int  rds_uint64;
/* portable 64-bit signed integer */
typedef signed long long int    rds_int64;
/* boolean representation */
typedef enum
{
    RDS_ERR_INPUT_LEN = -100,
    RDS_UNDEF = -1,

    /* FALSE value */
    RDS_FALSE = 0,
    /* TRUE value */
    RDS_TRUE  = 1
} rds_bool;

typedef enum {
    RDS_PREF_WIFI_ONLY = 0,
    RDS_PREF_CELLULAR_PREF = 1,
    RDS_PREF_WIFI_PREF = 2,
    RDS_PREF_CELLULAR_ONLY = 3,
    RDS_PREF_WIFI_PREF1 = 4,
    RDS_PREF_BYPASS = 5,	// By Pass : NON
    RDS_PREF_MAX
} rds_pref_e;

#define RDS_TASK_ID(x) ((x)-RDS_TASK_OFFSET)

typedef enum
{
    RDS_TASK_OFFSET = 0xF000,
    RDS_TASK_EPDGS,
    RDS_TASK_RILD,
    RDS_TASK_IMSM,
    RDS_TASK_C2KRILD,
    RDS_TASK_RILPXY,
    RDS_TASK_UT,
    RDS_TASK_RCS,
    RDS_TASK_ALL
} rds_ext_task_e;

typedef enum {
        RDS_RAN_UNSPEC = 0, //[20150910]
        RDS_RAN_MOBILE_3GPP = 1,
        RDS_RAN_WIFI = 2,
        RDS_RAN_MOBILE_3GPP2,
        RDS_RAN_TYPE_MAX,
        RDS_RAN_NONE = 0xFE, //DENY by PS, but can try CS
        RDS_RAN_DENY = 0xFF  //DENY by RDS
} ran_type_e;

typedef enum {
        IMS_SRV_GENERAL = 0,
        IMS_SRV_VOCALL = 1,
        IMS_SRV_VICALL = 2,
        IMS_SRV_VOVICALL = 3,
} imsrv_type_e;

typedef enum {
        RDS_HO_START = 0,
        RDS_HO_STOP = 1,
        RDS_HO_MAX
} ho_notify_t;

typedef enum {
    WOS_IMSCALL_NONE = 0,  //reserve type to skip 0
    WOS_IR92_VOCALL = 1,
    WOS_IR94_VICALL = 2,
    SMS_OVER_IP =3, //passed by URC, not WOS
    WOS_IPME =4,
    WOS_CALL_TYPE_CNT,
} wos_call_t;

typedef enum {
    RDS_RADIO_OFF = 0,
    RDS_RADIO_UNAVAIL = 1,
    RDS_RADIO_ON = 2
} radio_type_t;

typedef enum {
    VTCALL_BIT_RATE_ADAPTATION = 1,
    VTCALL_PACKET_LOSS_RATE,
    VTCALL_ROUND_TRIP_LATENCY,
    VTCALL_ATTR_MAX
} vtcall_attr_t;

typedef enum {
    VTS_CMD_NONE = 0, // reserve type to skip 0
    VTS_SET_VTCALL_STATUS,
    VTS_CMD_MAX
} rds_vts_cmd_t;

/*****************************
* [20151016]:TMO IKEV2 Err Retry
* need Po-Chun's help on fill in the actual err code enum*/
typedef enum {
    EA_ATCH_ERR_NONE = 0,
    EA_ATCH_ERR_VZ   = 0xFFF10000,
    EA_ATCH_ERR_WW   = 0xFFF20000,
    EA_ATCH_ERR_TMO  = 0xFFF30000,
    ERROR_NOTIFY_BASE             = 100000,
    UNSUPPORTED_CRITICAL_PAYLOAD  = ERROR_NOTIFY_BASE + 1,
    INVALID_MAJOR_VERSION         = ERROR_NOTIFY_BASE + 5,
    INVALID_SYNTAX                = ERROR_NOTIFY_BASE + 7,
    INVALID_MESSAGE_ID            = ERROR_NOTIFY_BASE + 9,
    NO_PROPOSAL_CHOSEN            = ERROR_NOTIFY_BASE + 14,
    INVALID_KE_PAYLOAD            = ERROR_NOTIFY_BASE + 17,
    AUTHENTICATION_FAILED         = ERROR_NOTIFY_BASE + 24,
    SINGLE_PAIR_REQUIRED          = ERROR_NOTIFY_BASE + 34,
    NO_ADDITIONAL_SAS             = ERROR_NOTIFY_BASE + 35,
    INTERNAL_ADDRESS_FAILURE      = ERROR_NOTIFY_BASE + 36,
    FAILED_CP_REQUIRED            = ERROR_NOTIFY_BASE + 37,
    TS_UNACCEPTABLE               = ERROR_NOTIFY_BASE + 38,
    PDN_CONNECTION_REJECTION      = ERROR_NOTIFY_BASE + 8192,
    NO_SUBSCRIPTION               = ERROR_NOTIFY_BASE + 9000,
    NETWORK_TOO_BUSY              = ERROR_NOTIFY_BASE + 10000,
    NETWORK_FAILURE               = ERROR_NOTIFY_BASE + 10500,
    ROAMING_NOR_ALLOWED           = ERROR_NOTIFY_BASE + 11000,
    RAT_DISALLOWED                = ERROR_NOTIFY_BASE + 11001,
    EA_TMO_NETWORK_FAILURE        = NETWORK_FAILURE,
    EA_TMO_NETWORK_TOO_BUSY       = NETWORK_TOO_BUSY,
    EA_TMO_RAT_DISALLOWED         = RAT_DISALLOWED,
    EA_TMO_ROAMING_NOR_ALLOWED    = ROAMING_NOR_ALLOWED,
    EA_ATCH_ERR_CNT,
} ea_atch_err;
/*****************************/

typedef enum {
    RDS_WFCA_CMDRSV = 0,  //reserve
    RDS_WFCA_CFGEN,       //configuration
    RDS_WFCA_QUERY,       //query statistics
} rds_wfca_cmd_e;

typedef enum {
    WFCA_STAT_RSV = 0,         //reserve
    WFCA_STAT_UPDATE,          //following contains valid update info
    WFCA_STAT_NOUPDATE,        //No update from LTECSR(under threshould)
    WFCA_STAT_UNINIT = 255,    //WFCA statistics hasn¡¦t been correctly cfg by RDS
} wfca_state_e;

typedef enum {
    RDS_RTPIF_IMS,
    RDS_RTPIF_EMG,
    RDS_RTPIF_CNT,
} wfca_rtpif_e;

typedef enum {
    RDS_RAT_UNKNOWN = 0,
    RDS_RAT_GPRS = 1,
    RDS_RAT_EDGE = 2,
    RDS_RAT_UMTS = 3,
    RDS_RAT_HSDPA = 9,
    RDS_RAT_HSUPA = 10,
    RDS_RAT_HSPA = 11,
    RDS_RAT_LTE = 14
} rds_rat_e;

typedef enum {
    RDS_MD1 = 0,
    RDS_MD2 = 1,
    RDS_MD3 = 2,
    RDS_MD_CNT,
} rds_md_e;

typedef struct _rds_ru_set_mdstat_req_t{
    rds_uint8 md_idx;      // 0:MD1 1:MD2 2:MD3
    rds_uint8 ucsim_id;     // for futhur flexibility
    rds_uint8 fgisroaming; // RDS_TRUE, RDS_FALSE
    rds_uint8 md_rat;      // rds_rat_e
    rds_uint8 srv_state;    //service state
    rds_char  plmn_id[RDS_STR_LEN]; //currently only keep SIM mcc
} rds_ru_set_mdstat_req_t;

typedef struct _rds_ru_set_voice_mdstat_req_t{
    rds_uint8 md_idx;      // 0:MD1 1:MD2 2:MD3
    rds_uint8 ucsim_id;     // for futhur flexibility
    rds_uint8 fgisroaming; // RDS_TRUE, RDS_FALSE
    rds_uint8 md_rat;      // rds_rat_e
    rds_uint8 srv_state;    //service state
} rds_ru_set_voice_mdstat_req_t;

typedef MAL_Data_Call_Response_v11 MAL_Data_Call_RdsRsp_v11;

typedef struct _rds_event_t{
    rds_uint32 u4snd_id;
    void       *pvsnd_data;
    size_t     u4snd_data_len;
    rds_uint32 u4rcv_id;
    void       *pvrcv_data;
    size_t     u4rcv_data_len;
}rds_event_t;

typedef struct _rds_ru_set_uiparam_req_t{
    rds_uint8 ucsim_id;
    rds_uint8 fgimsolte_en; // ims over lte enable
    rds_uint8 fgwfc_en;     // wfc enable
    rds_char  omacp_fqdn[RDS_FQDN_LEN];
    rds_uint8 fgwifiui_en;  // ui wifi botton
    rds_uint8 rdspolicy;    // preference mode
    rds_uint8 fgdata_roaming_ui_en; //data roaming ui enable
    rds_uint8 fgwpref_en;	//ATT req in roaming -> has been remove from v16.0
    rds_uint8 fgimsccp_en;          // video enable
    rds_uint8 fgallow_turnoff_ims;  // KEY_CARRIER_ALLOW_TURNOFF_IMS_BOOL
    rds_int8  wifioff_reason;       // rds_wifioff_reason_e
} rds_ru_set_uiparam_req_t;

typedef struct _rds_ru_set_wifistat_req_t{
    rds_uint8 fgwifi_en;
    rds_char  szwifi_ifname[RDS_STR_LEN];
    rds_char  szwifi_ipv4addr[2*RDS_STR_LEN];
    rds_char  szwifi_ipv6addr[2*RDS_STR_LEN];
    rds_char  szwifi_macaddr[2*RDS_STR_LEN];
} rds_ru_set_wifistat_req_t;

typedef struct _rds_ru_set_wifimon_req_t{
    rds_uint32 fgwifi_rssi;
    rds_uint32 fgwifi_snr;
} rds_ru_set_wifimon_req_t;

typedef struct _rds_wifimon_config_t{
    rds_int32 sim_id;
    rds_int32 rssi_thrd_list[RDS_THRD_CNT_MAX];
    rds_int32 rssi_num;
} rds_wifimon_config_t;

typedef struct _rds_ru_set_radiostat_req_t{
    rds_uint8 md_idx;      // 0:MD1 1:MD2 2:MD3
    rds_uint8 ucsim_id;     // for futhur flexibility
    rds_uint8 radio;       // Radio on/off
} rds_ru_set_radiostat_req_t;

typedef struct _rds_ru_sst_update_req_t{
    // prevent ping-pong between two RAN,
    // rds took high low threshold to determine the H.O. time
    // The rds considers rovein RAN when signal strengh
    // over high threshold and roveout below low threshold
    rds_uint8 ucsim_id;
    rds_int32 i4wifirssih;  // wifi rssi threshold high for ROVE IN
    rds_int32 i4wifirssil;  // wifi rssi threshold low for ROVE OUT
    rds_int32 i4wifisnrh;  // wifi rssi threshold high for ROVE IN
    rds_int32 i4wifisnrl;  // wifi rssi threshold low for ROVE OUT
    rds_int32 i4wifirssih_ho;  // wifi rssi threshold high for HO IN
    rds_int32 i4wifirssil_ho;  // wifi rssi threshold low for HO OUT
    rds_int32 i4ltersrph;   // lte  rsrp threshold high
    rds_int32 i4ltersrpm;
    rds_int32 i4ltersrpl;   // lte  rsrp threshold low
    rds_int32 i4ltersrqh;   // lte  rsrp threshold high
    rds_int32 i4ltersrqm;
    rds_int32 i4ltersrql;   // lte  rsrp threshold low
    rds_int32 i43grscph;   // utran rscp threshold high
    rds_int32 i43grscpl;   // utran rscp threshold low
    rds_int32 i42grssih;   // gsm rssi threshold high
    rds_int32 i42grssil;   // gsm rssi threshold low
} rds_ru_sst_update_req_t;

typedef struct _rds_ho_status_t{
    /* please Alan help to provide */
    rds_uint8 ucsim_id;
    rds_uint8 ucho_status; // 0: start_ho 1: stop_ho
    rds_uint8 fgho_result; // 0: fail(RDS_FALSE) 1: success(RDS_TRUE)
    rds_uint8 interfaceID; // (RILD_Req -1, aka real) /*[20150924]: ifid btween MAL/rild => keep TRUE ifid*/
    rds_uint8 esource_ran_type; //ran_type_e
    rds_uint8 etarget_ran_type; //ran_type_e
    MAL_Data_Call_Response_v11 pdninfo;
} rds_ho_status_t;

typedef struct _rds_datacall_info_t{
    rds_bool      active; //0: idle, 1: active
    ip_type_t     iptype; //0: IPV4, 1: IPV6, 2: IPV4V6
    unsigned char ipv6Addr[16];
}rds_datacall_info_t;

typedef struct _rds_set_calllist_req_t{
    rds_datacall_info_t  datacall[RDS_MAX_PDN_NUM];
}rds_set_calllist_req_t;

typedef struct _netstat_if_t{
    rds_uint32  thrghpt;  //throughput
    rds_uint32  pkterrt;  //packet err rate
    rds_uint32  pktlost;  //packet loss rate
    rds_uint32  latency;  //latency
    rds_uint32  retrans;  //retrans rate
} netstat_if_t;


typedef struct _ping_result_t{
    rds_uint8  ucsim_id;
    ran_type_e ran_type;
    rds_uint32 ave_latency;
    rds_uint32 total_loss;
}ping_result_t;

typedef struct _rds_rb_get_ho_status_req_t{
    rds_uint8  ucsim_id;
    rds_uint32 ifid;
}rds_rb_get_ho_status_req_t;

typedef struct _rds_rb_get_ho_status_t{
    rds_uint8   ucsim_id;
    rds_uint32  start_key;
    rds_uint32  end_key;
    rds_uint8   fgho_result; // previous ho result; 0: fail(RDS_FALSE) 1: success(RDS_TRUE) 255: undefined/ho ongoing(RDS_UNDEF)
    rds_uint8   esource_ran_type; //ran_type_e, source RAN of ongoing/previous handover. UNSPEC if handover has not been done.
    rds_uint8   etarget_ran_type; //ran_type_e, source RAN of ongoing/previous handover. UNSPEC if handover has not been done.
}rds_rb_get_ho_status_t;

typedef struct _rtp_netstat_t {
    rds_uint8   ucsim_id;
    rds_int32   rtp_plr;
    rds_int32   rtp_tput;
    rds_uint32  rtp_jitter;
} rtp_netstat_t;

typedef struct _rds_get_wifistatus_t {
    rds_uint8   ucsim_id;
    rds_int32   rtp_plr;
    rds_int32   rtp_tput;
    rds_uint32  rtp_jitter;
    rds_int32   wifi_rssi;
} rds_get_wifistatus_t;

typedef struct _rds_rb_update_hocnt_t{
    rds_uint8   ucsim_id;
    rds_uint32  ifid;
    rds_rb_get_ho_status_t  hocnt;
}rds_rb_update_hocnt_t;

typedef struct _rds_reg_req_t{
    rds_uint32  u4rds_task_id; //RDS_TASK_ID()
}rds_reg_req_t;

typedef struct _rds_rb_get_demand_req_t{
    rds_uint32 ucsim_id;
    rds_uint32 u4pdn_id;
} rds_rb_get_demand_req_t;

typedef struct _rds_rb_get_ran_type_rsp_t{
    rds_uint8  ucsim_id;
    rds_int32  i4rds_ret; // RDS_TRUE / RDS_FALSE
    rds_int32  u4_ifid;  //interfaceid
    rds_uint32 u4ran_type; //ran_type_e
} rds_rb_get_ran_type_rsp_t;

typedef struct _rds_rb_get_last_err_rsp_t{
    rds_uint8  ucsim_id;
    rds_int32  i4rds_ret; // RDS_TRUE / RDS_FALSE
    rds_int32  i4lasterr;
    rds_int32  i4lastsuberr;
    rds_uint32 u4lastran; // ran_type_e
} rds_rb_get_last_err_rsp_t;

typedef struct _rds_rb_last_err_ind_t{
    rds_int32  fgupdate_only;
    rds_rb_get_last_err_rsp_t rrsp;
} rds_rb_last_err_ind_t;

typedef struct _rds_err_rsp_t{
   /* rds will reply i4rds_ret if input size is wrong for send and recv */
   rds_int32  i4rds_ret; // RDS_TRUE / RDS_FALSE
} rds_err_rsp_t;

typedef dm_req_setup_data_call_t rds_sdc_req_t;
typedef dm_resp_setup_data_call_t rds_sdc_cnf_t;

typedef struct {
    rds_int32 ril_status;
    MAL_Data_Call_Response_v11 ind;
} rds_sdc_ind_t;

typedef dm_req_t rds_ddc_req_t;

typedef struct {
    rds_uint8 ucsim_id;
    rds_int32 status;
    rds_uint8 eran_type;
} rds_ddc_cnf_t;

typedef struct {
    rds_uint8 ucsim_id;
    rds_int32 ril_status;
    rds_int32 cid; //InterfaceID (RILD_Req-1, aka real) /*[20150924]: ifid btween MAL/rild => keep TRUE ifid*/
} rds_ddc_ind_t;

typedef struct _rds_rvout_alert_t{
    rds_uint8 ucsim_id;
    rds_bool  rvalert_en;    //is near rvout threshold
    rds_int32 i4wifirssi;    //current wifi rssi
} rds_rvout_alert_t;

typedef struct _rds_wifi_pdnact_t{
    rds_uint8 ucsim_id;
    rds_int32 ifid;     //currently will always be IMS ifid = 4
    rds_bool  pdn_rdy;  //RDS_TRUE: ready
} rds_wifi_pdnact_t;

typedef struct _rds_wifi_disi4_t{
    rds_uint8 ucsim_id;
    rds_int32 u4pdn_cnt;  //count of alive pdn
} rds_wifi_disi4_t;

typedef struct _rds_sbp_chg_ind_t{
    rds_uint8 ucsim_id;
    rds_uint32 u4sbpid; // sbp id for D-SBP
} rds_sbp_chg_ind_t;

typedef struct _rds_ims_status_t{
    rds_bool fgims_en;
    rds_uint8 sim_idx;
} rds_ims_status_t;

typedef struct _rds_ims_status_ind_t{
    rds_uint8  ucsim_id;
    rds_uint32 src_id;
} rds_ims_status_ind_t;

typedef struct _rds_get_location_t{
    rds_uint8 ucsim_id;
} rds_get_location_t;

typedef int (*rds_ho_status_ind_fn)(rds_ho_status_t *prstatus);
typedef int (*rds_rvout_alert_ind_fn)(rds_rvout_alert_t *pralert);
typedef int (*rds_wifi_pdnact_ind_fn)(rds_wifi_pdnact_t *pralert);
typedef int (*rds_get_rantype_ind_fn)( rds_rb_get_ran_type_rsp_t *prran);
typedef int (*rds_wifi_disable_ind_fn)(rds_wifi_disi4_t *prdis);
typedef int (*rds_get_lasterr_ind_fn)( rds_rb_get_last_err_rsp_t *prran);
typedef int (*rds_wifimon_cfg_ind_fn)(rds_wifimon_config_t *prconfig);
typedef int (*rds_ims_status_ind_fn)(rds_ims_status_t *prstatus);
typedef int (*rds_get_location_ind_fn)(rds_get_location_t *prloc);

typedef struct _rds_notify_funp_t{
    rds_ho_status_ind_fn      pfrds_ho_status_ind;
    rds_rvout_alert_ind_fn    pfrds_rvout_alert_ind;
    rds_wifi_pdnact_ind_fn    pfrds_wifi_pdnact_ind;
    rds_get_rantype_ind_fn    pfrds_get_rantype_ind;    //for ims pdn rantype update
    rds_wifi_disable_ind_fn   pfrds_wifi_disable_ind;
    rds_get_lasterr_ind_fn    pfrds_get_lasterr_ind;    //for ims pdn last err update
    rds_wifimon_cfg_ind_fn    pfrds_wifimon_cfg_ind;
    rds_ims_status_ind_fn     pfrds_ims_status_ind;
    rds_get_location_ind_fn   pfrds_get_location_ind;
} rds_notify_funp_t;

typedef  rds_int32  (*rds_if_fn)(rds_uint32 u4user, char *szcmd, void *prinput, void *proutput, void * conn_ptr);

typedef struct _rds_conninit_data_t {
    rds_if_fn  pfrds_if_cnf;
    rds_if_fn  pfrds_if_ind;
} rds_conninit_data_t;

typedef rds_sdc_req_t rr_sdc_req_t;
typedef rds_sdc_cnf_t rr_sdc_cnf_t;
typedef rds_ddc_cnf_t rr_ddc_cnf_t;
typedef rds_ddc_req_t rr_ddc_req_t;

/*
typedef struct _rds_wfca_cfg_single_t{
    int    version;
    int    sim_slot;
    int    id;
    int    threshold;
    int    duration;
} rds_wfca_cfg_single_t;

typedef struct _wfca_callinfo_single_t {
    int           version;
    int           state;    //wfca_state_e
    int           id;
    int           loss_rate;
    int           thrpt;     //throughput*
} wfca_callinfo_single_t;
*/

typedef union _rds_wfca_ext_event_s {
    unsigned int jitter_buffer;
    unsigned int sim_id;
/*
    rds_wfca_cfg_single_t   wfca_cfg;
    wfca_callinfo_single_t  wfca_callinfo[RDS_MPS_SUPPORT_MAX];
*/
} rds_wfca_ext_event_s;

typedef struct _rds_wfca_ext_element_t
{
    int        len;      // full element size include "len"
    int        event_id;
    rds_wfca_ext_event_s s;
} rds_wfca_ext_element_t;

typedef enum {
    RDS_WFCA_EXT_NONE = 0,
    RDS_WFCA_EXT_JITTER_BUFFER = 1,
    RDS_WFCA_EXT_SIM_ID = 2,
    RDS_WFCA_EXT_MAX = 255,
} rds_wfca_event_id_e;

#define WFCA_EXT_LEN 512


typedef struct _wfca_callinfo_t {
    int           version;
    int           state;    //wfca_state_e
    int           id;
    int           loss_rate;
    int           thrpt;     //throughput*
    char          ext_body[WFCA_EXT_LEN];
} wfca_callinfo_t;

typedef struct _rds_wfca_cfg_t{
    int    version;
    int    id;
    int    threshold; // for rtp PLR
    int    duration;
} rds_wfca_cfg_t;

typedef struct _rds_wfca_req_t{
    int                cmd;    //rds_wfca_cmd_e
    rds_wfca_cfg_t     cfg;
    char               ext_body[WFCA_EXT_LEN];
} rds_wfca_req_t;

typedef enum{
    RDS_USR_RSV,
    RDS_USR_WOS,
    RDS_USR_RILPXY,
    RDS_USR_GSMRIL,
    RDS_USR_C2KRIL,
    RDS_USR_VTS,
    RDS_USR_CMDCNT,
}ril_rds_usr_e;

/* test mode purpose */
typedef enum {
	SDC_IND_SUCC = 0,
    SDC_IND_FAIL,
    PDN_NW_DDC,
    PDN_INFO_DUMP,
    UT_CFG_WONLY,
    UT_CFG_WPREF,
    UT_CFG_MONLY,
    UT_CFG_MPREF,
    UT_CFG_INIT_STAT,
    UT_CFG_TEST_CASE,
    UT_CFG_RSRP,
    UT_CFG_RSSI,
    UT_LOCK_EVENT,
    UT_UNLOCK_EVENT,
    UT_EVENT_LOCK_DUMP,
    UT_CFG_SBPID,
    UT_IMSVOPS_CTRL = 252,
    UT_TEST_START = 253,
    UT_SKIP_1stIMS = 254,
    ENABLE_ENTITY_UT = 255,
} ut_cmd_t;

typedef struct _rds_ut_cmd_t{
    rds_uint8  ucsim_id;
    rds_uint32 evnt_id;
    rds_uint32 cmd;
    union {
        rds_uint32 ril_ifid;
        rds_uint32 pol_lv; // policy level
        rds_uint32 status;
        rds_int32 value;
        rds_uint32 event;
    };
    rds_char iipv6[16]; //for SDC IND UT
} rds_ut_cmd_t;

typedef enum{
    WOS_END_CALL = 0,
    WOS_ACT_CALL = 1,
    WOS_EST_CALL = 2,
    WOS_RESET = 254,
}wos_call_stat;

typedef enum {
    WOS_IDLE = 0,
    WOS_MT_CALL,
    WOS_MO_CALL,
    WOS_CONF_CALL, // conference call
    WOS_CALL_MODE_CNT
} wos_call_mode;

typedef struct _rds_ru_set_callstat_req_t{
    rds_uint8     ucsim_id;
    wos_call_stat     call_stat;
    wos_call_t    call_type;
    rds_int32    call_id;
    wos_call_mode call_mode;
} rds_ru_set_callstat_req_t;


typedef struct _ru_nm_cgreg_mode_req_t{
    rds_uint8 ucsim_id;
    rds_uint8 uccgreg_mode;
} ru_nm_cgreg_mode_req_t;

typedef struct _ru_nm_cgreg_mode_cnf_t{
        char                sim_id;
        unsigned int        result;
        char                data[4096];
} ru_nm_cgreg_mode_cnf_t;

typedef struct {
    char apn_name[STRING_SIZE];
    char apn_type[STRING_SIZE]; //apn_profile_id#1|apn_profile_id#2 ... ex: 0|1001 means default|mms
    char ip_protocol[STRING_SIZE];
    int  rat_ability; //cell ability=1, wifi ability=2, cellnwifi ability=3
} rds_apn_cfg_table_t;

typedef struct {
    int  list_cnt;
    rds_apn_cfg_table_t apn_cfg[RDS_APNCNT];
    rds_uint8 ucsim_id;     // for futhur flexibility
} rds_apn_cfg_req_t;

typedef struct _rds_ru_set_vtcallstat_req_t {
    vtcall_attr_t vtcall_attr;
    rds_int32 value;
    ran_type_e ran_type;
    rds_int32 call_id;
    rds_int32 sim_id;
} rds_ru_set_vtcallstat_req_t;

typedef union {
    rds_ru_set_vtcallstat_req_t vtcallstat_req;
} vts_data_t;

typedef struct _rds_vts_req_t {
    rds_vts_cmd_t cmd;
    rds_int32 data_len;
    vts_data_t data;
} rds_vts_req_t;

typedef struct _rds_get_retry_time_req_t {
    rds_uint32 ucsim_id;
    rds_char   apn[STRING_SIZE];
    rds_uint32 ifid;
    rds_char   profile[STRING_SIZE];
} rds_get_retry_time_req_t;

typedef struct _rds_get_retry_time_rsp_t {
    rds_uint32 suggested_time;
    ran_type_e ran;
    rds_int32  failcause;
} rds_get_retry_time_rsp_t;

typedef struct _rds_set_location_t {
    rds_uint32 ucsim_id;
    rds_char   country[RDS_COUNTRY_DATA_LEN]; /* Two letter country code according to ISO 3166 + end NUL */
} rds_set_location_t;


/** APIs **/
void *(*rild_rds_conn_init)(rds_notify_funp_t *callback_fn);
rds_int32 (*rild_rds_conn_exit)(void * conn_ptr);
rds_int32 (*rild_rds_ddc_req)(dm_req_t *req, rr_ddc_cnf_t *cnf);
rds_int32 (*rild_rds_sdc_req)(dm_req_setup_data_call_t *req, dm_resp_setup_data_call_t *cnf);
rds_int32 (*rds_rild_get_ho_status)(rds_rb_get_ho_status_t *rsp, rds_rb_get_ho_status_req_t *req, void * conn_ptr);
rds_int32 (*rild_rds_set_apncfg)(rds_apn_cfg_req_t *req);
rds_int32 (* rds_get_last_err)(rds_rb_get_demand_req_t *req, rds_rb_get_last_err_rsp_t *rsp, void * conn_ptr);
rds_int32 (* rds_get_ran_type)(rds_rb_get_demand_req_t *req, rds_rb_get_ran_type_rsp_t *rsp, void * conn_ptr);
rds_int32 (* rds_notify_wifimonitor)(rds_ru_set_wifimon_req_t *req, void * conn_ptr);
rds_int32 (* rds_set_callstat)(rds_ru_set_callstat_req_t *req, void * conn_ptr);
rds_int32 (* rds_set_location)(rds_set_location_t *loc);
rds_int32 (* rds_set_mdstat)(rds_ru_set_mdstat_req_t *req, void * conn_ptr);
rds_int32 (* rds_set_radiostat)(rds_ru_set_radiostat_req_t *req, void * conn_ptr);
rds_int32 (* rds_set_ui_param)(rds_ru_set_uiparam_req_t *req, void * conn_ptr);
rds_int32 (* rds_set_wifistat)(rds_ru_set_wifistat_req_t *req, void * conn_ptr);
void * (* epdgs_rds_conn_init)(rds_notify_funp_t *callback_fn);
rds_int32 (* epdgs_rds_conn_exit)(void * conn_ptr);
rds_int32 (* rds_if_req)(rds_uint32 u4user, char *szcmd, void *prinput, void *proutput, void * conn_ptr);

#endif
