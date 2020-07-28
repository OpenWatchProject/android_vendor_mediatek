#ifndef __RCS_PROXY_ADAPTER_H__
#define __RCS_PROXY_ADAPTER_H__

#include "rcs_comm.h"
#include <sys/cdefs.h>

// list of the RCS proxy events
// b/w RCS_proxy and RCS_proxy_adapter
// the NAMES OF THE EVENT AND THEIR VALUE IS SAME IN JAVA CLASSES
enum RCS_PROXY_Event_e
{
    /* ----------------------------------------------------- */
    /*  RCS_PROXY_ADAPTER --> RCS_PROXY                      */
    /*  This table should keep sync with RcsUaAdapter.java   */
    /* ----------------------------------------------------- */
    BASE = 10000,
    /* request ims registartion info */
    CMD_REQ_REG_INFO                = BASE + 1,
    /* request VoLTE stack for IMS Registration */
    CMD_IMS_REGISTER                = BASE + 2,
    /* request VoLTE stack for IMS Deregistration */
    CMD_IMS_DEREGISTER              = BASE + 3,
    /* send SIP request */
    CMD_SEND_SIP_MSG                = BASE + 4,
    /* [ROI]update VoPS info to volte stack */
    CMD_IMS_NOTIFY_VOPS_INFO        = BASE + 5,
    /* [ROI]update IMS settings */
    CMD_IMS_UPDATE_SETTING          = BASE + 6,
    /* [ROI]add capability for rcs service tag */
    CMD_IMS_ADD_CAPABILITY          = BASE + 7,
    /* [ROI]notify RAT change info */
    CMD_IMS_NOTIFY_RAT_CHANGE       = BASE + 8,
    /* [ROI]notify network change info */
    CMD_IMS_NOTIFY_NETWORK_CHANGE   = BASE + 9,
    /* [ROI]send back authentication request result */
    CMD_IMS_AUTH_REQ_RESULT         = BASE + 10,
    /* [ROI]send back geolocation request result */
    CMD_IMS_GEOLOCATION_REQ_RESULT  = BASE + 11,
    /* [ROI]send back query state result */
    CMD_IMS_QUERY_STATE_RESULT      = BASE + 12,
    /* [ROI]notify 3GPP RAT change info */
    CMD_IMS_NOTIFY_3GPP_RAT_CHANGE  = BASE + 13,
    /* notify rcs status to rds */
    CMD_RDS_NOTIFY_RCS_CONN_INIT    = BASE + 14,
    /* notify rcs connection status to rds */
    CMD_RDS_NOTIFY_RCS_CONN_ACTIVE  = BASE + 15,
    /* notify rcs connection status to rds */
    CMD_RDS_NOTIFY_RCS_CONN_INACTIVE= BASE + 16,
    /* [ROI]Realtime update IMS settings */
    CMD_IMS_UPDATE_RT_SETTING       = BASE + 17,
    /* [ROI]update Digits Line Reg Info */
    CMD_IMS_REG_DIGITLINE           = BASE + 18,
    /* RCS service activation */
    CMD_RCS_ACTIVATION              = BASE + 19,
    /* RCS service deactivation */
    CMD_RCS_DEACTIVATION            = BASE + 20,
    /* operator on SIM card */
    CMD_SIM_OPERATOR                = BASE + 21,
    /*set Service Activation State, sync with rild*/
    CMD_SERVICE_ACTIVATION_STATE    = BASE + 22,
    /* set service feaure tags, sync with rild */
    CMD_UPDATE_SERVICE_TAGS         = BASE + 23,
    /* confirm the unpublish is completed */
    CMD_UNPUBLISH_COMPLETED         = BASE + 24,
    /* active acct id for rcs */
    CMD_ACTIVE_RCS_SLOT_ID          = BASE + 25,
    /* trigger IMS restoration */
    CMD_IMS_RESTORATION             = BASE + 26,
    /* indicate UA state to enable/disable IMS dereg suspend */
    CMD_ENABLE_IMS_DEREG_SUSPEND    = BASE + 27,


    /* response ims registartion info */
    RSP_REQ_REG_INFO                = BASE + 31,
    /* intermediate response for IMS Registration */
    RSP_IMS_REGISTERING             = BASE + 32,
    /* final response of IMS Registration */
    RSP_IMS_REGISTER                = BASE + 33,
    /* intermediate response for IMS Deregistration */
    RSP_IMS_DEREGISTERING           = BASE + 34,
    /* final response of IMS Deregistration */
    RSP_IMS_DEREGISTER              = BASE + 35,
    /* a SIP request response or a SIP request from server */
    RSP_EVENT_SIP_MSG               = BASE + 36,

    /* indicate that IMS is de-registering */
    EVENT_IMS_DEREGISTER_IND        = BASE + 51,
    /* [ROI]authentication request form volte stack */
    EVENT_IMS_AUTH_REQ              = BASE + 52,
    /* [ROI]geolocation request form volte stack */
    EVENT_IMS_GEOLOCATION_REQ       = BASE + 53,
    /* [ROI]query state */
    EVENT_IMS_QUERY_STATE           = BASE + 54,
    /* [ROI]EMS mode info indication */
    EVENT_IMS_EMS_MODE_INFO         = BASE + 55,
    /* [ROI]Dgitline Reg Info */
    EVENT_IMS_DIGITLING_REG_IND     = BASE + 56,
};

/* THE SAMPLE VALUE OF THE REGISTRTAION VALUE RETURNED FROM STACK */
/*
 *
 * RCS[0]
 * local address  = 192.168.43.1 ,
 * local port = 5060,
 * protocol_type = 2 ,
 * protocol_version = 1,
 * public_uid = sip:404119102654903@ims.mnc011.mcc404.3gppnetwork.org,
 * private_uid = 404119102654903@ims.mnc011.mcc404.3gppnetwork.org ,
 * home_uri = ims.mnc011.mcc404.3gppnetwork.org ,
 * pcscf_address = 192.168.43.223 ,
 * pcscf_port =5060 ,
 * associated_uri = 864855010002295 ,
 * pub_gruu = sip:+SIPP@192.168.43.1,tel:+123456789 ,
 * temp_gruu = "sip:callee@example.com;gr=urn:uuid:f81d4fae-7dec-11d0-a765-00a0c91e6bf6" ,
 * service_route = "sip:tgruu.7hs==jd7vnzga5w7fajsc7-ajd6fabz0f8g5@example.com;gr" ,
 * path = <sip:orig@10.185.16.6:30244;lr;Dpt=7624_246;ca=c2lwOis4NjI4Njg2NTkwODBAMTAuMTg1LjE2LjE2OjI2NjY1> ,
 * net_type  = -1204917360 ,
 * emergency_type = 7  ,
 * reg_timestamp = (null)
 */

typedef struct _RCS_PROXY_Event_Reg_State2_
{
    INT32 id;           /* /< the account id */
    INT32 state;        /* /< refer to ::VoLTE_Event_Reg_State_e */
    INT32 rcs_state;
    INT32 rcs_feature;

    /* account information */
    INT8 local_address[VOLTE_MAX_ADDRESS_LENGTH];       /* /< local ip address */
    INT32 local_port;                                   /* /< local port number */
    INT32 protocol_type;                                /* /< refer to ::VoLTE_Stack_Protocol_Type_e */ ///< UDP: 0, TCP: 1
    INT32 protocol_version;                             ///< refer to ::VoLTE_Stack_Protocol_Type_e ///< IPv4: 1, IPv6: 2
    INT8 public_uid[VOLTE_MAX_REG_UID_LENGTH];          /* /< the public user identity */
    INT8 private_uid[VOLTE_MAX_REG_UID_LENGTH];
    INT8 home_uri[VOLTE_MAX_DOMAIN_NAME_LENGTH];
    INT8 pcscf_address[VOLTE_MAX_ADDRESS_LIST_LENGTH];  /* /< the current used PCSCF ip address */
    INT32 pcscf_port;                                   /* /< the current used PCSCF port number */
    INT8  user_agent[VOLTE_MAX_USER_AGENT_LENGTH];  //user-agent used
    INT8  associated_uri[VOLTE_MAX_ASSOCIATED_URI];
    INT8  instance_id[VOLTE_MAX_URI_LENGTH];
    INT8  service_route[VOLTE_MAX_URI_LENGTH];
    INT8  pani[VOLTE_MAX_P_ACCESS_NETWORK_INFO_LENGTH];
    INT32  digit_number_of_vline;
    INT8 digit_pidentifier[VOLTE_MAX_DIGIT_PIDENTIFIER_LENGTH];
    INT8 digit_ppa_header_string[VOLTE_MAX_DIGIT_PPA_LENGTH];
} RCS_PROXY_Event_Reg_State2;

/* initialize RCS_adapter */
void RCS_Proxy_adapter_Init_hal(int source_id);

/* temp fnction to send test event response back to rcs_proxy_adapter */
void send_test_event_response();

__BEGIN_DECLS
void writeEventHal(int32_t request_id, int32_t length, char* data);
__END_DECLS

const char * idToString(int id);
int RCS_getInt32(void** src, int* dst);
int RCS_getStrn(void** src, char* dst, int len);
int isROISupported();
int isImsSubmarineSupported();

#endif /* __RCS_PROXY_ADAPTER_H__ */ /* __RCS_PROXY__ADAPTER_H__ */

