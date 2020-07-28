#ifndef __UCE_PROXY_ADAPTER_H__
#define __UCE_PROXY_ADAPTER_H__

#include "volte_core_interface.h"
#include <sys/cdefs.h>


/*
 * typedef unsigned int UINT32;
 *
 * typedef int INT32;
 *
 * typedef unsigned short UINT16;
 *
 * typedef short INT16;
 *
 * typedef unsigned char UINT8;
 *
 * typedef char INT8;
 *
 */

//list of the UCE proxy events
//b/w UCE_proxy and UCE_proxy_adapter
//the NAMES OF THE EVENT AND THEIR VALUE IS SAME IN JAVA CLASSES
enum UCE_PROXY_Event_e
{
    /* ------------------------------------------------ */
    /*  UCE_PROXY_ADAPTER --> UCE_PROXY                 */
    /* ------------------------------------------------ */
    BASE = 20000,
    /* request ims registartion info */
    CMD_REQ_REG_INFO            = BASE + 1,
    /* response ims registartion info */
    RSP_REQ_REG_INFO            = BASE + 2,

    /* request VoLTE stack for IMS Registration */
    CMD_IMS_REGISTER            = BASE + 3,
    /* intermediate response for IMS Registration */
    RSP_IMS_REGISTERING         = BASE + 4,
    /* final response of IMS Registration */
    RSP_IMS_REGISTER            = BASE + 5,

    /* request VoLTE stack for IMS Deregistration */
    CMD_IMS_DEREGISTER          = BASE + 6,
    /* intermediate response for IMS Deregistration */
    RSP_IMS_DEREGISTERING       = BASE + 7,
    /* final response of IMS Deregistration */
    RSP_IMS_DEREGISTER          = BASE + 8,

    /* send SIP request */
    CMD_SEND_SIP_MSG            = BASE + 9,
    /* SIP message event, including a request response or a SIP event */
    RSP_EVENT_SIP_MSG           = BASE + 10,

    /* indicate that IMS is de-registering */
    EVENT_IMS_DEREGISTER_IND    = BASE + 11,
    /* confirm the unpublish is completed */
    CMD_UNPUBLISH_COMPLETED     = BASE + 12,
    /* request VoLTE stack for IMS recover registration */
    CMD_IMS_RECOVER_REGISTER_IND     = BASE + 13,
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

/* uce proxt structure to store Reg state informatoins */
typedef struct _UCE_PROXY_Event_Reg_State_
{
    INT32 id;           /* /< the account id */
    INT32 state;        /* /< refer to ::VoLTE_Event_Reg_State_e */
    INT32 cause;        /* /< refer to ::VoLTE_Event_Reg_Cause_e */
    UINT32 conn_info;   /* /< the connection information for the others UA to create the connection */

    /* account information */
    INT8 local_address[VOLTE_MAX_ADDRESS_LENGTH];       /* /< local ip address */
    INT32 local_port;                                   /* /< local port number */
    INT32 protocol_type;                                /* /< refer to ::VoLTE_Stack_Protocol_Type_e */
    INT32 protocol_version;                             /* /< refer to ::VoLTE_Stack_Protocol_Version_e */
    INT8 public_uid[VOLTE_MAX_REG_UID_LENGTH];          /* /< the public user identity */
    INT8 private_uid[VOLTE_MAX_REG_UID_LENGTH];         /* /< the private user identity */
    INT8 home_uri[VOLTE_MAX_URI_LENGTH];                /* /< the domain name of the home network */
    INT8 pcscf_address[VOLTE_MAX_ADDRESS_LIST_LENGTH];  /* /< the current used PCSCF ip address */
    INT32 pcscf_port;                                   /* /< the current used PCSCF port number */
    INT8 imei[VOLTE_MAX_IMEI_LENGTH];                   /* /< the IMEI */
    INT8 associated_uri[VOLTE_MAX_ASSOCIATED_URI];      /* /< the list of the associated URI */
    INT8 pub_gruu[VOLTE_MAX_GRUU_LENGTH];               /* /< the public gruu */
    INT8 temp_gruu[VOLTE_MAX_GRUU_LENGTH];              /* /< the temp gruu */
    INT8 service_route[VOLTE_MAX_URI_LENGTH];           /* /< the service route */
    INT8 path[VOLTE_MAX_URI_LENGTH];                    /* /< the path */
    INT32 net_type;                                     /* /< refer to ::VoLTE_Event_Network_Type_e */
    INT32 emergency_type;                               /* /< refer to ::VoLTE_Event_Emergency_Type_e */
    INT8 reg_timestamp[VOLTE_MAX_TIMESTAMP_LENGTH];     /* /< TMO P-Last-Access-Network-Info "REGISTRATION TIMESTAMP" */
    INT8 security_verify[VOLTE_MAX_SECURIT_VERIFY_LENGTH];
} UCE_PROXY_Event_Reg_State;

typedef struct _UCE_PROXY_Event_Reg_State2_
{
    INT32 id;           /* /< the account id */
    INT32 state;        /* /< refer to ::VoLTE_Event_Reg_State_e */
    INT32  rcs_state;
    INT32  rcs_feature;

    /* account information */
    INT8 local_address[VOLTE_MAX_ADDRESS_LENGTH];       /* /< local ip address */
    INT32 local_port;                                   /* /< local port number */
    INT32 protocol_type;                                /* /< refer to ::VoLTE_Stack_Protocol_Type_e */ ///< UDP: 0, TCP: 1
    INT32 protocol_version;                             ///< refer to ::VoLTE_Stack_Protocol_Type_e ///< IPv4: 1, IPv6: 2
    INT8 public_uid[VOLTE_MAX_REG_UID_LENGTH];          /* /< the public user identity */
	INT8 pcscf_address[VOLTE_MAX_ADDRESS_LIST_LENGTH];  /* /< the current used PCSCF ip address */
	INT32 pcscf_port;                                   /* /< the current used PCSCF port number */
    INT8  user_agent[VOLTE_MAX_USER_AGENT_LENGTH];  //user-agent used
    INT8  associated_uri[VOLTE_MAX_ASSOCIATED_URI];
    INT8  instance_id[VOLTE_MAX_URI_LENGTH];
    INT8  service_route[VOLTE_MAX_URI_LENGTH];
    INT8  home_uri[VOLTE_MAX_DOMAIN_NAME_LENGTH];
    INT32 digit_number_of_vline;
    INT8 digit_pidentifier[VOLTE_MAX_DIGIT_PIDENTIFIER_LENGTH]; // PIDENTIFIER
    INT8 digit_ppa_header_string[VOLTE_MAX_DIGIT_PPA_LENGTH]; // PPA for all the lines
} UCE_PROXY_Event_Reg_State2;

/* initialize UCE_adapter */
void UCE_Proxy_adapter_Init_hal(int source_id);

/* temp fnction to send test event response back to uce_proxy_adapter */
void send_test_event_response();

__BEGIN_DECLS
void writeEventHal(int32_t request_id, int32_t length, char* data);
__END_DECLS

#endif /* __UCE_PROXY_ADAPTER_H__ */ /* __UCE_PROXY__ADAPTER_H__ */

