#ifndef __DATAMNGR_IF_H__
#define __DATAMNGR_IF_H__

#include "mal_cfg.h"    // NEED_TO_BE_NOTICED, to be removed from the common part

#ifndef MAX_IPV6_ADDRESS_LENGTH
#define MAX_IPV6_ADDRESS_LENGTH 65
#endif

#undef DM_MAX_PCSCF_NUMBER
#define DM_MAX_PCSCF_NUMBER 16

#define STRING_SIZE 128

#ifndef MAX_NUM_DNS_ADDRESS_NUMBER
#define MAX_NUM_DNS_ADDRESS_NUMBER 2
#endif

#ifndef CONCATENATE_NUM
#define CONCATENATE_NUM 11
#endif

#define DATAMNGR_MAX_SIM (4)
#define DATAMNGR_SIM_ID_INVALID(sim_id) ((sim_id) < 0 || (sim_id) >= DATAMNGR_MAX_SIM)

typedef struct datamngr_hdr
{
    int sim_id;
}datamngr_hdr_t;
typedef struct datamngr_hdr* datamngr_hdr_ptr_t;

typedef struct {
    //0: QCI is selected by network
    //[1-4]: value range for guaranteed bit rate Traffic Flows
    //[5-9]: value range for non-guarenteed bit rate Traffic Flows
    //[128-254]: value range for Operator-specific QCIs
    int qci; //class of EPS QoS
    int dlGbr; //downlink guaranteed bit rate
    int ulGbr; //uplink guaranteed bit rate
    int dlMbr; //downlink maximum bit rate
    int ulMbr; //uplink maximum bit rate
} MAL_Qos;

typedef struct {
    int id;
    int precedence;
    int direction;
    int networkPfIdentifier;
    int bitmap;
    char address[MAX_IPV6_ADDRESS_LENGTH];
    char mask[MAX_IPV6_ADDRESS_LENGTH];
    int protocolNextHeader;
    int localPortLow;
    int localPortHigh;
    int remotePortLow;
    int remotePortHigh;
    int spi;
    int tos;
    int tosMask;
    int flowLabel;
} MAL_PktFilter;

#define MAX_AUTH_TOKEN_NUMBER (16)
#define MAX_FLOW_ID_NUMBER (4)
typedef struct {
    int authTokenNumber;
    int authTokenList[MAX_AUTH_TOKEN_NUMBER];
    int flowIdNumber;
    int flowIdList[MAX_FLOW_ID_NUMBER][4];
} MAL_AuthToken;

#define MAX_LINKED_PF_NUMBER (16)
#define MAX_AUTH_TOKEN_FLOW_ID_NUMBER (4)
typedef struct {
    int linkedPfNumber;
    int linkedPfList [MAX_LINKED_PF_NUMBER];
    int authtokenFlowIdNumber;
    MAL_AuthToken authtokenFlowIdList[MAX_AUTH_TOKEN_FLOW_ID_NUMBER];
} MAL_TftParameter;

#define MAX_PF_LIST_NUMBER (16)
typedef struct {
    int operation;
    int pfNumber;
    MAL_PktFilter pfList[MAX_PF_LIST_NUMBER]; //for response, just send necessary length. check responseSetupDedicateDataCall in ril.cpp
    MAL_TftParameter tftParameter;
} MAL_Tft;

typedef struct {
    unsigned short  pcscfNum;
    unsigned short  pcscfIpv6Mask;
    unsigned char   pcscf[DM_MAX_PCSCF_NUMBER][16];
} dm_pcscf_t;
typedef dm_pcscf_t mal_datamngr_pcscf_into_t;
typedef dm_pcscf_t* mal_datamngr_pcscf_into_ptr_t;

typedef struct {
    int             ddcId;
    int             interfaceId;
    int             primaryCid;
    int             cid;        /* Context ID, uniquely identifies this call */
    int             active;     /* 0=inactive, 1=active/physical link down, 2=active/physical link up */
    int             signalingFlag;
    int             bearerId;
    int             failCause;
    int             hasQos;
    MAL_Qos         qos;
    int             hasTft;
    MAL_Tft         tft;
    mal_datamngr_pcscf_into_t pcscfs;
    datamngr_hdr_t hdr;
} MAL_Dedicate_Data_Call_Struct;

typedef struct  {
    int status;     /* A MAL_DataCallFailCause, 0 which is PDP_FAIL_NONE if no error */
    int suggestedRetryTime; /* If status != 0, this fields indicates the suggested retry
                           back-off timer value RIL wants to override the one
                           pre-configured in FW.
                           The unit is miliseconds.
                           The value < 0 means no value is suggested.
                           The value 0 means retry should be done ASAP.
                           The value of INT_MAX(0x7fffffff) means no retry. */
    int cid;        /* interface ID, uniquely identifies this call */
    int active;     /* 0=inactive, 1=active/physical link down, 2=active/physical link up */
    int mtuSize;    /* 0 is default and query fail*/
    int type;       // ip_type_t
                                /* One of the PDP_type values in TS 27.007 section 10.1.1.
                       For example, "IP", "IPV6", "IPV4V6", or "PPP". If status is
                       PDP_FAIL_ONLY_SINGLE_BEARER_ALLOWED this is the type supported
                       such as "IP" or "IPV6" */
    char ifname[STRING_SIZE];     /* The network interface name */
    unsigned ipv4Addr;
    unsigned char ipv6Addr[16];
    /*char *          addresses;*/  /* A space-delimited list of addresses with optional "/" prefix length,
                       e.g., "192.0.1.3" or "192.0.1.11/16 2001:db8::1/64".
                       May not be empty, typically 1 IPv4 or 1 IPv6 or
                       one of each. If the prefix length is absent the addresses
                       are assumed to be point to point with IPv4 having a prefix
                       length of 32 and IPv6 128. */
    /*unsigned dnsNum;
    unsigned dnsIpv6Mask;
    unsigned char dnses[MAX_NUM_DNS_ADDRESS_NUMBER][16];    */
    unsigned ipv4DnsNum;
    unsigned char ipv4Dnses[MAX_NUM_DNS_ADDRESS_NUMBER][4];
    unsigned ipv6DnsNum;
    unsigned char ipv6Dnses[MAX_NUM_DNS_ADDRESS_NUMBER][16];

    /*char *          dnses;*/      /* A space-delimited list of DNS server addresses,
                       e.g., "192.0.1.3" or "192.0.1.11 2001:db8::1".
                       May be empty. */

    unsigned ipv4Gateway;
    unsigned char ipv6Gateway[16];

    /*char *          gateways;*/   /* A space-delimited list of default gateway addresses,
                       e.g., "192.0.1.3" or "192.0.1.11 2001:db8::1".
                       May be empty in which case the addresses represent point
                       to point connections. */
    //VoLTE
    //unsigned char*                           pcscf;
    unsigned network_id;
    int                             concatenateNum;
    MAL_Dedicate_Data_Call_Struct   concatenate[CONCATENATE_NUM];
    int                             defaultBearerValid;
    MAL_Dedicate_Data_Call_Struct   defaultBearer;
      int eran_type;  // 1:RDS_RAN_MOBILE_3GPP, 2:RDS_RAN_WIFI
      int islteonly;  // 0: 2/3, 4G allowed, 1: 4G only

    unsigned u4sbpid; // for rds check EA sbp id setting
    char apn[STRING_SIZE];  // for RDS rild_rds_sdc_req()
    char sim_id;
} MAL_Data_Call_Response_v11;

typedef struct {

    char radioType[STRING_SIZE];
    char profile[STRING_SIZE];
    char apn[STRING_SIZE];
    char username[STRING_SIZE];
    char passwd[STRING_SIZE];
    int authType;   // auth_type_t
    int protocol;   // ip_type_t
    int interfaceId; // RILD_Req, aka fake
    int retryCount; // SDC retry cnt

    int mIsValid;
    MAL_Qos qos;
    int emergency_ind;
    int pcscf_discovery_flag;
    int signaling_flag;
    int isHandover;
    int ran_type; // for VzW IMS on eHRPD
    char sim_id;
}MAL_Setup_Data_Call_Struct;

typedef MAL_Setup_Data_Call_Struct dm_req_setup_data_call_t;
typedef dm_req_setup_data_call_t mal_datamngr_setup_data_call_req_t;
typedef dm_req_setup_data_call_t* mal_datamngr_setup_data_call_req_ptr_t;
typedef dm_req_setup_data_call_t mal_datamngr_set_pdn_info_req_t;
typedef dm_req_setup_data_call_t* mal_datamngr_set_pdn_info_req_ptr_t;

typedef struct {
    int ril_status;
    MAL_Data_Call_Response_v11 resp;
    datamngr_hdr_t hdr;
} dm_resp_setup_data_call_t;
typedef dm_resp_setup_data_call_t mal_datamngr_data_call_list_t;
typedef dm_resp_setup_data_call_t* mal_datamngr_data_call_list_ptr_t;
typedef dm_resp_setup_data_call_t mal_datamngr_setup_data_call_rsp_t;
typedef dm_resp_setup_data_call_t* mal_datamngr_setup_data_call_rsp_ptr_t;
typedef dm_resp_setup_data_call_t mal_datamngr_get_pdn_info_rsp_t;
typedef dm_resp_setup_data_call_t* mal_datamngr_get_pdn_info_rsp_ptr_t;

typedef struct {
    int id; //InterfaceID (RILD_Req-1, aka real)
    int cause;
    char sim_id;
} dm_req_t;

typedef struct {
    char ifname[STRING_SIZE];
    unsigned network_id;
    datamngr_hdr_t hdr;
} dm_req_if_t;
typedef dm_req_if_t mal_datamngr_get_pdn_info_req_t;
typedef dm_req_if_t* mal_datamngr_get_pdn_info_req_ptr_t;
typedef dm_req_if_t mal_datamngr_ims_info_req_t;
typedef dm_req_if_t* mal_datamngr_ims_info_req_ptr_t;
typedef dm_req_if_t mal_datamngr_get_pdn_state_req_t;
typedef dm_req_if_t* mal_datamngr_get_pdn_state_req_ptr_t;
typedef dm_req_if_t mal_datamngr_bearer_info_req_t;
typedef dm_req_if_t* mal_datamngr_bearer_info_req_ptr_t;


typedef struct {
    int valid;
    MAL_Qos qos;
    int emergency_ind;
    int pcscf_discovery_flag;
    int signaling_flag;
    int is_handover;
    int assigned_rate;  // 0: UNSPEC, 1: LTE, 2: WIFI
} dm_ims_para_info_t;
typedef dm_ims_para_info_t mal_datamngr_ims_info_t;
typedef dm_ims_para_info_t* mal_datamngr_ims_info_ptr_t;

typedef enum {
    MAL_IPV4 = 0,
    MAL_IPV6 = 1,
    MAL_IPV4V6 = 2,
} ip_type_t;

typedef struct {
    int ril_status;
    datamngr_hdr_t hdr;
} dm_resp_t;
typedef dm_resp_t mal_datamngr_reg_rsp_t;
typedef dm_resp_t* mal_datamngr_reg_rsp_ptr_t;
typedef dm_resp_t mal_datamngr_set_pdn_info_rsp_t;
typedef dm_resp_t* mal_datamngr_set_pdn_info_rsp_ptr_t;
typedef dm_resp_t mal_datamngr_data_call_info_rsp_t;
typedef dm_resp_t* mal_datamngr_data_call_info_rsp_ptr_t;
typedef dm_resp_t mal_datamngr_set_pdn_state_rsp_t;
typedef dm_resp_t* mal_datamngr_set_pdn_state_rsp_ptr_t;
typedef dm_resp_t mal_datamngr_notify_dedicated_bearer_act_rsp_t;
typedef dm_resp_t* mal_datamngr_notify_dedicated_bearer_act_rsp_ptr_t;
typedef dm_resp_t mal_datamngr_notify_dedicated_bearer_modify_rsp_t;
typedef dm_resp_t* mal_datamngr_notify_dedicated_bearer_modify_rsp_ptr_t;
typedef dm_resp_t mal_datamngr_notify_dedicated_bearer_deact_rsp_t;
typedef dm_resp_t* mal_datamngr_notify_dedicated_bearer_deact_rsp_ptr_t;

typedef struct{
    datamngr_hdr_t hdr;
    char ifname[STRING_SIZE];     /* The network interface name */
}datamngr_trigger_rs_t;
typedef dm_resp_t   mal_datamngr_trigger_rs_rsp_t;
typedef dm_resp_t* mal_datamngr_trigger_rs_rsp_ptr_t;

typedef struct{
    datamngr_hdr_t hdr;
    int pdn_id;    // interface ID defined by RILD
    int state;    // DcFailCause in dataconnection/DcFailCause.java
}datamngr_pdn_state_t;
typedef datamngr_pdn_state_t mal_datamngr_set_pdn_state_req_t;
typedef datamngr_pdn_state_t* mal_datamngr_set_pdn_state_req_ptr_t;
typedef datamngr_pdn_state_t mal_datamngr_get_pdn_state_rsp_t;
typedef datamngr_pdn_state_t* mal_datamngr_get_pdn_state_rsp_ptr_t;

// ==== RILD`s Structure ====

#define MAX_IPV6_ADDRESS_LENGTH 65
// xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx
// xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx

// VoLTE
//#define MAX_PCSCF_NUMBER 16

typedef struct {
    // 0: QCI is selected by network
    // [1-4]: value range for guaranteed bit rate Traffic Flows
    // [5-9]: value range for non-guarenteed bit rate Traffic Flows
    // [128-254]: value range for Operator-specific QCIs
    int qci;  // class of EPS QoS
    int dlGbr;  // downlink guaranteed bit rate
    int ulGbr;  // uplink guaranteed bit rate
    int dlMbr;  // downlink maximum bit rate
    int ulMbr;  // uplink maximum bit rate
} MALRIL_Qos;

typedef struct {
    int id;
    int precedence;
    int direction;
    int networkPfIdentifier;
    int bitmap;
    char address[MAX_IPV6_ADDRESS_LENGTH];
    char mask[MAX_IPV6_ADDRESS_LENGTH];
    int protocolNextHeader;
    int localPortLow;
    int localPortHigh;
    int remotePortLow;
    int remotePortHigh;
    int spi;
    int tos;
    int tosMask;
    int flowLabel;
} MALRIL_PktFilter;

typedef struct {
    int authTokenNumber;
    int authTokenList[16];
    int flowIdNumber;
    int flowIdList[4][4];
} MALRIL_AuthToken;

typedef struct {
    int linkedPfNumber;
    int linkedPfList[16];
    int authtokenFlowIdNumber;
    MALRIL_AuthToken authtokenFlowIdList[4];
} MALRIL_TftParameter;

typedef struct {
    int operation;
    int pfNumber;
    MALRIL_PktFilter pfList[16];  // for response, just send necessary length. check responseSetupDedicateDataCall in ril.cpp
    MALRIL_TftParameter tftParameter;
} MALRIL_Tft;

typedef struct {
    int             ddcId;
    int             interfaceId;
    int             primaryCid;
    int             cid;        /* Context ID, uniquely identifies this call */
    int             active;     /* 0=inactive, 1=active/physical link down, 2=active/physical link up */
    int             signalingFlag;
    int             bearerId;
    int             failCause;
    int             hasQos;
    MALRIL_Qos      qos;
    int             hasTft;
    MALRIL_Tft      tft;
    int             hasPcscf;
    char            pcscf[DM_MAX_PCSCF_NUMBER * (MAX_IPV6_ADDRESS_LENGTH + 1)];
} MALRIL_Dedicate_Data_Call_Struct;

// referring to DcFailCause in dataconnection/DcFailCause.java
typedef enum
{
    MAL_LOST_CONNECTION = 0x10004
}mal_dc_fail_cause_t;

typedef struct {
    int             status;     /* A MALRIL_DataCallFailCause, 0 which is PDP_FAIL_NONE if no error */
    int             suggestedRetryTime; /* If status != 0, this fields indicates the suggested retry
                                           back-off timer value RIL wants to override the one
                                           pre-configured in FW.
                                           The unit is miliseconds.
                                           The value < 0 means no value is suggested.
                                           The value 0 means retry should be done ASAP.
                                           The value of INT_MAX(0x7fffffff) means no retry. */
    int             cid;        /* Context ID, uniquely identifies this call */
    int             active;     /* 0=inactive, 1=active/physical link down, 2=active/physical link up */
    char *          type;       /* One of the PDP_type values in TS 27.007 section 10.1.1.
                                   For example, "IP", "IPV6", "IPV4V6", or "PPP". If status is
                                   PDP_FAIL_ONLY_SINGLE_BEARER_ALLOWED this is the type supported
                                   such as "IP" or "IPV6" */
    char *          ifname;     /* The network interface name */
    char *          addresses;  /* A space-delimited list of addresses with optional "/" prefix length,
                                   e.g., "192.0.1.3" or "192.0.1.11/16 2001:db8::1/64".
                                   May not be empty, typically 1 IPv4 or 1 IPv6 or
                                   one of each. If the prefix length is absent the addresses
                                   are assumed to be point to point with IPv4 having a prefix
                                   length of 32 and IPv6 128. */
    char *          dnses;      /* A space-delimited list of DNS server addresses,
                                   e.g., "192.0.1.3" or "192.0.1.11 2001:db8::1".
                                   May be empty. */
    char *          gateways;   /* A space-delimited list of default gateway addresses,
                                   e.g., "192.0.1.3" or "192.0.1.11 2001:db8::1".
                                   May be empty in which case the addresses represent point
                                   to point connections. */
    char *          pcscf;    /* the Proxy Call State Control Function address
                                 via PCO(Protocol Configuration Option) for IMS client. */
    int             mtu;        /* MTU received from network
                                   Value <= 0 means network has either not sent a value or
                                   sent an invalid value */
    // VoLTE
    int concatenateNum;
    MALRIL_Dedicate_Data_Call_Struct*  concatenate;
    MALRIL_Dedicate_Data_Call_Struct   defaultBearer;
    int eran_type;          /* 0: UNSPEC, 1: MOBILE_3GPP, 2: WIFI, 3: MOBILE_3GPP2
                                     4: TYPE_MAX, 0xFE: RDS_RAN_NONE, 0xFF: RDS_RAN_DENY */
} malril_data_call_info_v1_t;

#define MAL_DATAMNG_VER    (1)
#define mal_datamngr_self(info_ptr)    ((info_ptr)->ver == 1 ? &((info_ptr)->info.v1) : NULL)
#define mal_datamngr_set_ver(info_ptr, version)    (((info_ptr)->ver = version) == 1 ? &((info_ptr)->info.v1) : NULL)
#define mal_datamngr_get_ver(info_ptr)    ((info_ptr)->ver)

typedef struct
{
    union
    {
        malril_data_call_info_v1_t v1;
    }info;
    int ver;
}malril_data_call_info_t;
//typedef malril_data_call_info_t mal_datamngr_data_call_info_req_t;
//typedef malril_data_call_info_t* mal_datamngr_data_call_info_req_ptr_t;
typedef malril_data_call_info_v1_t mal_datamngr_data_call_info_req_t;
typedef malril_data_call_info_v1_t* mal_datamngr_data_call_info_req_ptr_t;

typedef struct
{
    union
    {
        mal_datamngr_ims_info_t v1;
    }info;
    int ver;
}malril_ims_info_t;
//typedef malril_ims_info_t mal_datamngr_ims_info_rsp_t;
//typedef malril_ims_info_t* mal_datamngr_ims_info_rsp_ptr_t;
typedef mal_datamngr_ims_info_t mal_datamngr_ims_info_rsp_t;
typedef mal_datamngr_ims_info_t* mal_datamngr_ims_info_rsp_ptr_t;

typedef enum {
    MALRIL_MAL_DATAMNGR_GET_IMS_INFO = 0,   //mal_datamngr_get_ims_info
    MALRIL_MAL_DATAMNGR_SET_DATA_CALL_INFO = 1, //mal_datamngr_set_data_call_info
    MALRIL_MAL_DATAMNGR_SET_PDN_STATE_TLV = 2, //mal_datamngr_set_pdn_state_tlv
    MALRIL_MAL_DATAMNGR_NOTIFY_DATA_CALL_LIST_TLV = 3, //mal_datamngr_notify_data_call_list_tlv
    MALRIL_MAL_DATAMNGR_NOTIFY_DEDICATED_BEARER_ACT_TLV = 4, //mal_datamngr_notify_dedicated_bearer_act_tlv
    MALRIL_MAL_DATAMNGR_NOTIFY_DEDICATED_BEARER_MODIFY_TLV = 5, //mal_datamngr_notify_dedicated_bearer_modify_tlv
    MALRIL_MAL_DATAMNGR_NOTIFY_DEDICATED_BEARER_DEACT_TLV = 6, //mal_datamngr_notify_dedicated_bearer_deact_tlv
    MALRIL_MAL_RILPROXY_RESP_DATA_CALL_INFO_TLV = 7, //mal_rilproxy_resp_data_call_info_tlv
    MALRIL_MAL_RILPROXY_RESP_PCSCF_PCO_TLV = 8, //mal_rilproxy_resp_pcscf_pco_tlv
    MALRIL_MAL_RILPROXY_RESP_LAST_FAIL_CAUSE_TLV = 9, //mal_rilproxy_resp_last_fail_cause_tlv
    MALRIL_API_INDEX_MAX
} MALRIL_API_INDEX_T;


/** APIs **/
int (* mal_datamngr_get_ims_info)(mal_ptr_t mal_ptr, mal_datamngr_ims_info_req_ptr_t req_ptr, mal_datamngr_ims_info_rsp_ptr_t rsp_ptr);
int (* mal_datamngr_set_data_call_info)(mal_ptr_t mal_ptr, mal_datamngr_data_call_info_req_ptr_t req_ptr, mal_datamngr_data_call_info_rsp_ptr_t rsp_ptr);
int (* mal_datamngr_get_api_tlv_buff_len)(mal_ptr_t mal_ptr, int size, int *tlv_buff_len);
int (* mal_datamngr_set_pdn_state)(mal_ptr_t mal_ptr, mal_datamngr_set_pdn_state_req_ptr_t req_ptr, mal_datamngr_set_pdn_state_rsp_ptr_t rsp_ptr);
int (* mal_datamngr_get_ims_info_tlv)(mal_ptr_t mal_ptr, mal_datamngr_ims_info_req_ptr_t req_ptr, mal_datamngr_ims_info_rsp_ptr_t rsp_ptr,
        int num_req_tlv, void *req_tlv, int *num_rsp_tlv, void *rsp_tlv);
int (* mal_datamngr_set_data_call_info_tlv)(mal_ptr_t mal_ptr, mal_datamngr_data_call_info_req_ptr_t req_ptr, mal_datamngr_data_call_info_rsp_ptr_t rsp_ptr,
        int num_req_tlv, void *req_tlv, int *num_rsp_tlv, void *rsp_tlv);
int (* mal_datamngr_set_pdn_state_tlv)(mal_ptr_t mal_ptr, mal_datamngr_set_pdn_state_req_ptr_t req_ptr, mal_datamngr_set_pdn_state_rsp_ptr_t rsp_ptr,
        int num_req_tlv, void *req_tlv, int *num_rsp_tlv, void *rsp_tlv);

int (* mal_datamngr_notify_data_call_list_tlv)(mal_ptr_t mal_ptr, int num_pdn, mal_datamngr_data_call_info_req_ptr_t req_ptr[], mal_datamngr_data_call_info_rsp_ptr_t rsp_ptr[],
        int num_req_tlv[], void *req_tlv[], int *num_rsp_tlv[], void *rsp_tlv[]);
int (* mal_datamngr_notify_dedicated_bearer_act_tlv)(mal_ptr_t mal_ptr, MAL_Dedicate_Data_Call_Struct *req_ptr, mal_datamngr_notify_dedicated_bearer_act_rsp_ptr_t rsp_ptr,
        int num_req_tlv, void *req_tlv, int *num_rsp_tlv, void *rsp_tlv);
int (* mal_datamngr_notify_dedicated_bearer_modify_tlv)(mal_ptr_t mal_ptr, MAL_Dedicate_Data_Call_Struct *req_ptr, mal_datamngr_notify_dedicated_bearer_modify_rsp_ptr_t rsp_ptr,
        int num_req_tlv, void *req_tlv, int *num_rsp_tlv, void *rsp_tlv);
int (* mal_datamngr_notify_dedicated_bearer_deact_tlv)(mal_ptr_t mal_ptr, int cid, mal_datamngr_notify_dedicated_bearer_deact_rsp_ptr_t rsp_ptr,
        int num_req_tlv, void *req_tlv, int *num_rsp_tlv, void *rsp_tlv);
int (* mal_datamngr_trigger_rs_tlv)(mal_ptr_t mal_ptr, char *ifname, mal_datamngr_trigger_rs_rsp_ptr_t rsp_ptr,
        int num_req_tlv, void *req_tlv, int *num_rsp_tlv, void *rsp_tlv);

#endif
