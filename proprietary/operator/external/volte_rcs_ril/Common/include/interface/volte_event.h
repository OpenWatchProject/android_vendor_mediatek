#ifndef __VOLTE_EVENT_H__
#define __VOLTE_EVENT_H__

#include "ims_interface_ap.h"

#define VOLTE_MAX_SERVICE_ROUTE_LENGTH  256

enum VoLTE_Event_e {
    VOLTE_EVENT_DUMMY                       = 0,

    /* ------------------------------------------------ */
    /*  VoLTE Ctrl --> VoLTE Service                    */
    /* ------------------------------------------------ */
    VOLTE_EVENT_TO_SERVICE_START_CODE       = 10000,
    /* Registration */
    VOLTE_EVENT_TO_SERVICE_REG_START_CODE   = VOLTE_EVENT_TO_SERVICE_START_CODE + 1000,
    /* Call */
    VOLTE_EVENT_TO_SERVICE_CALL_START_CODE  = VOLTE_EVENT_TO_SERVICE_START_CODE + 2000,
    /* SMS */
    VOLTE_EVENT_TO_SERVICE_SMS_START_CODE   = VOLTE_EVENT_TO_SERVICE_START_CODE + 3000,
    /* VT */
    VOLTE_EVENT_TO_SERVICE_VT_START_CODE    = VOLTE_EVENT_TO_SERVICE_START_CODE + 4000,
    /* SS */
    VOLTE_EVENT_TO_SERVICE_SS_START_CODE    = VOLTE_EVENT_TO_SERVICE_START_CODE + 5000,

    VOLTE_EVENT_TO_SERVICE_USSD_START_CODE  = VOLTE_EVENT_TO_SERVICE_START_CODE + 6000,
    /* ------------------------------------------------ */
    /*  VoLTE Ctrl <-- VoLTE Service                    */
    /* ------------------------------------------------ */
    VOLTE_EVENT_FROM_SERVICE_START_CODE     = 20000,
    /* Registration */
    VOLTE_EVENT_FROM_SERVICE_REG_START_CODE = VOLTE_EVENT_FROM_SERVICE_START_CODE + 1000,
    /* Call */
    VOLTE_EVENT_FROM_SERVICE_CALL_START_CODE = VOLTE_EVENT_FROM_SERVICE_START_CODE + 2000,
    /* SMS */
    VOLTE_EVENT_FROM_SERVICE_SMS_START_CODE  = VOLTE_EVENT_FROM_SERVICE_START_CODE + 3000,
    /* VT */
    VOLTE_EVENT_FROM_SERVICE_VT_START_CODE   = VOLTE_EVENT_FROM_SERVICE_START_CODE + 4000,
    /* SS */
    VOLTE_EVENT_FROM_SERVICE_SS_START_CODE   = VOLTE_EVENT_FROM_SERVICE_START_CODE + 5000,

    VOLTE_EVENT_FROM_SERVICE_USSD_START_CODE = VOLTE_EVENT_FROM_SERVICE_START_CODE + 6000,
    /* ------------------------------------------------ */
    /*  VoLTE UAs --> VoLTE Service                     */
    /* ------------------------------------------------ */
    VOLTE_EVENT_FROM_UAS_START_CODE         = 30000,
    //VOLTE_REQ_GET_ACCOUNT_INFO              = VOLTE_EVENT_FROM_UAS_START_CODE + 1,                  /* VoLTE_Event_Account_Item_t */

    /* ------------------------------------------------ */
    /*  VoLTE UAs <-- VoLTE Service                     */
    /* ------------------------------------------------ */
    VOLTE_EVENT_TO_UAS_START_CODE           = 40000,
    //VOLTE_RSP_GET_ACCOUNT_INFO              = VOLTE_EVENT_TO_UAS_START_CODE + 1,                    /* VoLTE_Event_Account_Info_t */

    /* ------------------------------------------------ */
    /*  VoLTE Service (Internal)                        */
    /* ------------------------------------------------ */
    VOLTE_EVENT_SERVICE_INTERNAL_START_CODE = 50000,

    /* Timer */
    VOLTE_EVENT_SERVICE_INTERNAL_TIMER      = VOLTE_EVENT_SERVICE_INTERNAL_START_CODE + 5000,
    VOLTE_REQ_TIMER_CREATE_FAILED           = VOLTE_EVENT_SERVICE_INTERNAL_TIMER + 1,               /* VoLTE_Event_Timer_t */
    VOLTE_REQ_TIMER_TIMEOUT                 = VOLTE_EVENT_SERVICE_INTERNAL_TIMER + 2,               /* VoLTE_Event_Timer_t */


    /* ------------------------------------------------ */
    /*  UA --> VoLTE SIP Stack                          */
    /* ------------------------------------------------ */
    VOLTE_EVENT_TO_SIPTX_START_CODE         = 90000,
    VOLTE_REQ_SIPTX_CREATE                  = VOLTE_EVENT_TO_SIPTX_START_CODE + 1,                  /* VoLTE_Stack_Channel_Info_t */
    VOLTE_REQ_SIPTX_DESTROY                 = VOLTE_EVENT_TO_SIPTX_START_CODE + 2,                  /* N/A */
    VOLTE_REQ_SIPTX_CONNECT                 = VOLTE_EVENT_TO_SIPTX_START_CODE + 3,                  /* VoLTE_Stack_Connection_Info_t */
    VOLTE_REQ_SIPTX_DISCONNECT              = VOLTE_EVENT_TO_SIPTX_START_CODE + 4,                  /* N/A */
    VOLTE_REQ_SIPTX_SEND_SIPMSG             = VOLTE_EVENT_TO_SIPTX_START_CODE + 5,                  /* VoLTE_Stack_Sip_Info_t */
    VOLTE_REQ_SIPTX_MEM_DEBUG               = VOLTE_EVENT_TO_SIPTX_START_CODE + 6,                  /* VoLTE_Debug_Info_t */
    VOLTE_REQ_SIPTX_SETTING                 = VOLTE_EVENT_TO_SIPTX_START_CODE + 7,                  /* VoLTE_Event_Setting_t */
    VOLTE_REQ_SIPTX_AUTO_TESTING_ID_UPDATE  = VOLTE_EVENT_TO_SIPTX_START_CODE + 8,                  /* VoLTE_AutoTesting_Info_t */
    VOLTE_REQ_SIPTX_ABORT_SIPMSG            = VOLTE_EVENT_TO_SIPTX_START_CODE + 9,                  /* VoLTE_Stack_Sip_Info_t */
    VOLTE_REQ_SIPTX_RESET_TCP               = VOLTE_EVENT_TO_SIPTX_START_CODE + 10,                 /* VoLTE_Stack_Channel_Info_t */
    VOLTE_REQ_SIPTX_START_KEEP_ALIVE        = VOLTE_EVENT_TO_SIPTX_START_CODE + 11,                 /* VoLTE_Stack_Channel_Info_t */
    VOLTE_REQ_SIPTX_STOP_KEEP_ALIVE         = VOLTE_EVENT_TO_SIPTX_START_CODE + 12,                 /* VoLTE_Stack_Channel_Info_t */
    VOLTE_IND_SIPTX_CALL_SESSION_TERMINATED = VOLTE_EVENT_TO_SIPTX_START_CODE + 13,                 /* VoLTE_Stack_Sip_Info_t */
    VOLTE_IND_SIPTX_UA_STATE                = VOLTE_EVENT_TO_SIPTX_START_CODE + 14,                 /* VoLTE_Event_UA_State_t */


    VOLTE_EVENT_TO_SIPTX_REG_START_CODE     = VOLTE_EVENT_TO_SIPTX_START_CODE + 1000,
    VOLTE_REQ_SIPTX_REG_REG                 = VOLTE_EVENT_TO_SIPTX_REG_START_CODE + 1,              /* VoLTE_Event_Reg_Item_t */
    VOLTE_REQ_SIPTX_REG_REREG               = VOLTE_EVENT_TO_SIPTX_REG_START_CODE + 2,              /* VoLTE_Event_Reg_Item_t */
    VOLTE_REQ_SIPTX_REG_DEREG               = VOLTE_EVENT_TO_SIPTX_REG_START_CODE + 3,              /* VoLTE_Event_Reg_Item_t */
    VOLTE_REQ_SIPTX_REG_NETWORK_CHANGE      = VOLTE_EVENT_TO_SIPTX_REG_START_CODE + 11,             /* VoLTE_Event_Network_Change_t */
    VOLTE_REQ_SIPTX_REG_UPDATE_SETTING      = VOLTE_EVENT_TO_SIPTX_REG_START_CODE + 12,             /* VoLTE_Event_Setting_t */
    VOLTE_REQ_SIPTX_REG_ADD_BIND            = VOLTE_EVENT_TO_SIPTX_REG_START_CODE + 13,             /* VoLTE_Event_Reg_Item_t */
    VOLTE_REQ_SIPTX_REG_DEL_BIND            = VOLTE_EVENT_TO_SIPTX_REG_START_CODE + 14,             /* VoLTE_Event_Reg_Item_t */
    VOLTE_REQ_SIPTX_REG_CLEAR_BIND          = VOLTE_EVENT_TO_SIPTX_REG_START_CODE + 15,             /* N/A */
    VOLTE_REQ_SIPTX_REG_ADD_CAPABILITY      = VOLTE_EVENT_TO_SIPTX_REG_START_CODE + 16,             /* VoLTE_Event_Reg_Capability_t */
    VOLTE_REQ_SIPTX_REG_DEL_CAPABILITY      = VOLTE_EVENT_TO_SIPTX_REG_START_CODE + 17,             /* VoLTE_Event_Reg_Capability_t */
    VOLTE_REQ_SIPTX_REG_GET_STATE           = VOLTE_EVENT_TO_SIPTX_REG_START_CODE + 18,             /* VoLTE_Event_Reg_Item_t */
    VOLTE_REQ_SIPTX_REG_RESET               = VOLTE_EVENT_TO_SIPTX_REG_START_CODE + 19,             /* VoLTE_Event_Reg_Item_t */
    VOLTE_CNF_SIPTX_REG_AUTH_REQ            = VOLTE_EVENT_TO_SIPTX_REG_START_CODE + 20,             /* VoLTE_Event_Reg_Auth_Resp_t */
    VOLTE_CNF_SIPTX_REG_TIMER_EXPIRY        = VOLTE_EVENT_TO_SIPTX_REG_START_CODE + 21,             /* VoLTE_Event_Timer_t */
    VOLTE_REQ_SIPTX_REG_RAT_CHANGE          = VOLTE_EVENT_TO_SIPTX_REG_START_CODE + 22,             /* VoLTE_Event_Rat_Change_t */
    VOLTE_REQ_SIPTX_REG_RESTORE             = VOLTE_EVENT_TO_SIPTX_REG_START_CODE + 23,             /* VoLTE_Event_Reg_Item_t */
    VOLTE_REQ_SIPTX_REG_3GPP_RAT_CHANGE     = VOLTE_EVENT_TO_SIPTX_REG_START_CODE + 24,             /* VoLTE_Event_3gpp_Rat_Change_t */
    VOLTE_CNF_SIPTX_REG_QUERY_CALL_STATE    = VOLTE_EVENT_TO_SIPTX_REG_START_CODE + 25,             /* VoLTE_Event_List_Current_Call_t */
    VOLTE_REQ_SIPTX_REG_MD_LOWER_LAYER_ERR  = VOLTE_EVENT_TO_SIPTX_REG_START_CODE + 26,             /* VoLTE_Event_MD_Lower_Layer_Err_t */
    VOLTE_REQ_SIPTX_REG_SEND_SUBSRIBE       = VOLTE_EVENT_TO_SIPTX_REG_START_CODE + 27,             /* VoLTE_Event_Reg_Item_t */
    VOLTE_REQ_SIPTX_REG_IP_CHANGE_NOTIFY    = VOLTE_EVENT_TO_SIPTX_REG_START_CODE + 28,             /* VoLTE_Event_IP_Change_Notify_t */
    VOLTE_REQ_SIPTX_REG_QUERY_IF_EM_TS_EXIST= VOLTE_EVENT_TO_SIPTX_REG_START_CODE + 29,
    VOLTE_CNF_SIPTX_GEOLOCATION_REQ         = VOLTE_EVENT_TO_SIPTX_REG_START_CODE + 30,             /* VoLTE_Event_Geolocation_Info_t */
    VOLTE_REQ_SIPTX_REG_NOTIFY_IMSVOPS      = VOLTE_EVENT_TO_SIPTX_REG_START_CODE + 31,             /* VoLTE_Event_Notify_t  */
    VOLTE_REQ_SIPTX_REG_NOTIFY_SIP_BLOCK    = VOLTE_EVENT_TO_SIPTX_REG_START_CODE + 32,             /* VoLTE_Event_Notify_t  */
    VOLTE_CNF_SIPTX_PRESENCE_UNPUBLISH      = VOLTE_EVENT_TO_SIPTX_REG_START_CODE + 33,             /* VoLTE_Event_Notify_t  */
    VOLTE_REQ_SIPTX_REG_IRAT_WITHOUT_IP_CHANGE  = VOLTE_EVENT_TO_SIPTX_REG_START_CODE + 34,             /* VoLTE_Event_iRat_without_IP_Change_t  */
    VOLTE_REQ_SIPTX_IF_REG_READY            = VOLTE_EVENT_TO_SIPTX_REG_START_CODE + 35,
    VOLTE_REQ_SIPTX_REG_3GPP_OOS_IND        = VOLTE_EVENT_TO_SIPTX_REG_START_CODE + 36,
    VOLTE_REQ_SIPTX_REG_DIGITLINE           = VOLTE_EVENT_TO_SIPTX_REG_START_CODE + 37,             /* VoLTE_Event_ATCmd_Relay_t */
    VOLTE_REQ_SIPTX_REG_NOTIFY_CONF_SUBSC   = VOLTE_EVENT_TO_SIPTX_REG_START_CODE + 38,             /* VoLTE_Event_Notify_t  */
    VOLTE_IND_SIPTX_REG_EXTERNAL_UA_STATE   = VOLTE_EVENT_TO_SIPTX_REG_START_CODE + 45,             /* VoLTE_Event_External_UA_State_t  */

    VOLTE_EVENT_TO_SIPTX_IPSEC_START_CODE   = VOLTE_EVENT_TO_SIPTX_START_CODE + 1100,
    VOLTE_REQ_SIPTX_IPSEC_SETUP             = VOLTE_EVENT_TO_SIPTX_IPSEC_START_CODE + 1,            /* VoLTE_Stack_IPSec_Info_t */
    VOLTE_REQ_SIPTX_IPSEC_CONNECT           = VOLTE_EVENT_TO_SIPTX_IPSEC_START_CODE + 2,            /* VoLTE_Stack_IPSec_Info_t */
    VOLTE_REQ_SIPTX_IPSEC_DELETE            = VOLTE_EVENT_TO_SIPTX_IPSEC_START_CODE + 3,            /* VoLTE_Stack_IPSec_Info_t */
    VOLTE_REQ_SIPTX_IPSEC_CLEAR             = VOLTE_EVENT_TO_SIPTX_IPSEC_START_CODE + 4,            /* VoLTE_Stack_IPSec_Info_t */
    VOLTE_REQ_SIPTX_IPSEC_CHANGE_TO_REGULAR = VOLTE_EVENT_TO_SIPTX_IPSEC_START_CODE + 5,
    VOLTE_REQ_SIPTX_IPSEC_REFRESH           = VOLTE_EVENT_TO_SIPTX_IPSEC_START_CODE + 6,            /* VoLTE_Stack_IPSec_Info_t */


    /* ------------------------------------------------ */
    /*  UA <-- VoLTE SIP Stack                          */
    /* ------------------------------------------------ */
    VOLTE_EVENT_FROM_SIPTX_START_CODE       = 95000,
    VOLTE_RSP_SIPTX_CREATE                  = VOLTE_EVENT_FROM_SIPTX_START_CODE + 1,                /* VoLTE_Stack_Result_t */
    VOLTE_RSP_SIPTX_DESTROY                 = VOLTE_EVENT_FROM_SIPTX_START_CODE + 2,                /* VoLTE_Stack_Result_t */
    VOLTE_RSP_SIPTX_CONNECT                 = VOLTE_EVENT_FROM_SIPTX_START_CODE + 3,                /* VoLTE_Stack_Result_t */
    VOLTE_RSP_SIPTX_DISCONNECT              = VOLTE_EVENT_FROM_SIPTX_START_CODE + 4,                /* VoLTE_Stack_Result_t */
    VOLTE_RSP_SIPTX_SEND_SIPMSG             = VOLTE_EVENT_FROM_SIPTX_START_CODE + 5,                /* VoLTE_Stack_Result_t */
    VOLTE_RSP_SIPTX_IND_SIPMSG              = VOLTE_EVENT_FROM_SIPTX_START_CODE + 6,                /* VoLTE_Stack_Sip_Info_t */
    VOLTE_RSP_SIPTX_ABORT_SIPMSG            = VOLTE_EVENT_FROM_SIPTX_START_CODE + 7,                /* VoLTE_Stack_Result_t */
    VOLTE_RSP_SIPTX_RESET_TCP               = VOLTE_EVENT_FROM_SIPTX_START_CODE + 8,                /* VoLTE_Stack_Sip_Info_t */
    VOLTE_RSP_SIPTX_START_KEEP_ALIVE        = VOLTE_EVENT_FROM_SIPTX_START_CODE + 9,                 /* VoLTE_Stack_Channel_Info_t */
    VOLTE_RSP_SIPTX_STOP_KEEP_ALIVE         = VOLTE_EVENT_FROM_SIPTX_START_CODE + 10,                /* VoLTE_Stack_Channel_Info_t */

    VOLTE_IND_SIPTX_DISCONNECT              = VOLTE_EVENT_FROM_SIPTX_START_CODE + 11,               /* VoLTE_Stack_Result_t */
    VOLTE_RSP_SIPTX_REG_QUERY_IF_EM_TS_EXIST= VOLTE_EVENT_FROM_SIPTX_START_CODE + 12,


    VOLTE_EVENT_FROM_SIPTX_REG_START_CODE   = VOLTE_EVENT_FROM_SIPTX_START_CODE + 1000,
    VOLTE_IND_SIPTX_REG_STATE               = VOLTE_EVENT_FROM_SIPTX_REG_START_CODE + 1,            /* VoLTE_Event_Reg_State_t */
    VOLTE_IND_SIPTX_REG_AUTH_REQ            = VOLTE_EVENT_FROM_SIPTX_REG_START_CODE + 2,            /* VoLTE_Event_Reg_Auth_Req_t */
    VOLTE_IND_SIPTX_USE_PROXY               = VOLTE_EVENT_FROM_SIPTX_REG_START_CODE + 3,            /* VoLTE_Event_Reg_Item_t */
    VOLTE_IND_SIPTX_REG_QUERY_STATE         = VOLTE_EVENT_FROM_SIPTX_REG_START_CODE + 4,            /* VoLTE_Event_Query_State_t */
    VOLTE_IND_SIPTX_REG_TIMER_START         = VOLTE_EVENT_FROM_SIPTX_REG_START_CODE + 5,            /* VoLTE_Event_Timer_t */
    VOLTE_IND_SIPTX_REG_TIMER_CANCEL        = VOLTE_EVENT_FROM_SIPTX_REG_START_CODE + 6, /* VoLTE_Event_Timer_t */
    VOLTE_IND_SIPTX_EMS_MODE                = VOLTE_EVENT_FROM_SIPTX_REG_START_CODE + 7,            /* VoLTE_Event_Reg_Ems_Mode_t */
    VOLTE_IND_SIPTX_REG_NOTIFY_DONE         = VOLTE_EVENT_FROM_SIPTX_REG_START_CODE + 8,            /* VoLTE_Event_Reg_Notify_Done_t */
    VOLTE_IND_SIPTX_GEOLOCATION_REQ         = VOLTE_EVENT_FROM_SIPTX_REG_START_CODE + 9,            /* VoLTE_Event_Geolocation_Info_t */
    VOLTE_IND_SIPTX_REG_CANCEL_SIP_BLOCK    = VOLTE_EVENT_FROM_SIPTX_REG_START_CODE + 10,            /* VoLTE_Event_Notify_t */
    VOLTE_IND_SIPTX_REG_DEREG_START         = VOLTE_EVENT_FROM_SIPTX_REG_START_CODE + 11,
    VOLTE_RSP_SIPTX_IF_REG_READY            = VOLTE_EVENT_FROM_SIPTX_REG_START_CODE + 12,
    VOLTE_IND_SIPTX_REG_NOTIFY_CONF_SUBSC   = VOLTE_EVENT_FROM_SIPTX_REG_START_CODE + 13,           /* VoLTE_Event_Notify_t */

    VOLTE_EVENT_FROM_SIPTX_IPSEC_START_CODE = VOLTE_EVENT_FROM_SIPTX_START_CODE + 1100,
    VOLTE_RSP_SIPTX_IPSEC_SETUP             = VOLTE_EVENT_FROM_SIPTX_IPSEC_START_CODE + 1,          /* VoLTE_Stack_Result_t */
    VOLTE_RSP_SIPTX_IPSEC_CONNECT           = VOLTE_EVENT_FROM_SIPTX_IPSEC_START_CODE + 2,          /* VoLTE_Stack_Result_t */
    VOLTE_RSP_SIPTX_IPSEC_DELETE            = VOLTE_EVENT_FROM_SIPTX_IPSEC_START_CODE + 3,          /* VoLTE_Stack_Result_t */
    VOLTE_RSP_SIPTX_IPSEC_CLEAR             = VOLTE_EVENT_FROM_SIPTX_IPSEC_START_CODE + 4,          /* VoLTE_Stack_Result_t */
    VOLTE_RSP_SIPTX_IPSEC_CHANGE_TO_REGULAR             = VOLTE_EVENT_FROM_SIPTX_IPSEC_START_CODE + 5,
    VOLTE_RSP_SIPTX_IPSEC_REFRESH           = VOLTE_EVENT_FROM_SIPTX_IPSEC_START_CODE + 6,          /* VoLTE_Stack_Result_t */

    VOLTE_EVENT_IPSEC_POLICY_START_CODE     = VOLTE_EVENT_FROM_SIPTX_START_CODE + 1200,
    VOLTE_IND_IPSEC_POLICY_FLUSH            = VOLTE_EVENT_IPSEC_POLICY_START_CODE + 1,
    VOLTE_IND_IPSEC_POLICY_ADD              = VOLTE_EVENT_IPSEC_POLICY_START_CODE + 2,                /* VoLTE_Event_IPSec_Policy_Add_Ind_t */
    VOLTE_IND_IPSEC_POLICY_DELETE           = VOLTE_EVENT_IPSEC_POLICY_START_CODE + 3,                /* VoLTE_Event_IPSec_Policy_Delete_Ind_t */
    /* ------------------------------------------------ */
    /*  VoLTE IMC (External)                            */
    /* ------------------------------------------------ */
    VOLTE_EVENT_EXTERNAL_IMC_START_CODE     = IMCB_IMC_VOLTE_EVENT_EXTERNAL_IMC_START_CODE,
} ;

#define VOLTE_NETWORK_TYPE_IS_WIFI(net_type) \
    ((net_type >= VoLTE_Event_Network_Type_802_11) && \
     (net_type <= VoLTE_Event_Network_Type_802_11ac))

#define VOLTE_NETWORK_TYPE_IS_23G(net_type) \
    ((net_type >= VoLTE_Event_Network_Type_GERAN) && \
     (net_type <= VoLTE_Event_Network_Type_UTRAN_TDD))

#define VOLTE_NETWORK_TYPE_IS_LTE(net_type) \
    ((net_type >= VoLTE_Event_Network_Type_E_UTRAN_FDD) && \
     (net_type <= VoLTE_Event_Network_Type_E_UTRAN_TDD))

enum VoLTE_Event_Network_Type_e
{
    VoLTE_Event_Network_Type_Unspec         = 0,
    VoLTE_Event_Network_Type_Unknown        = 1,
    VoLTE_Event_Network_Type_802_11         = 2,
    VoLTE_Event_Network_Type_802_11a        = 3,
    VoLTE_Event_Network_Type_802_11b        = 4,
    VoLTE_Event_Network_Type_802_11g        = 5,
    VoLTE_Event_Network_Type_802_11n        = 6,
    VoLTE_Event_Network_Type_802_11ac       = 7,
    VoLTE_Event_Network_Type_GERAN          = 8,
    VoLTE_Event_Network_Type_UTRAN_FDD      = 9,
    VoLTE_Event_Network_Type_UTRAN_TDD      =10,
    VoLTE_Event_Network_Type_CDMA2000       =11,
    VoLTE_Event_Network_Type_E_UTRAN_FDD    =12,
    VoLTE_Event_Network_Type_E_UTRAN_TDD    =13,
    /*C2K*/
    VoLTE_Event_Network_Type_C2K_1xRTT      =14,
    VoLTE_Event_Network_Type_C2K_HRPD       =15,
    VoLTE_Event_Network_Type_C2K_eHRPD      =16,
    /*add here*/
    VoLTE_Event_Network_Type_Max,
};

typedef struct _VoLTE_Event_Reg_Item_ {
    INT32                                   id;                                                 ///< the account id
    UINT8                                   cell_id[VOLTE_MAX_CELL_ID_LENGTH];                  ///< the current cell id from the modem PS
    UINT8                                   call_id[VOLTE_MAX_CALL_ID_LENGTH];                  ///< the call ua sip msg call-id
    VoLTE_Event_LBS_Location_t              loc_info;
    UINT8                                   next_pcscf;                                         ///< Try next pcscf for Register
} VoLTE_Event_Reg_Item_t;

/* Note: Please also modify VoLTE_Event_Reg_State_s if VoLTE_Event_Reg_State_e has any change */
enum VoLTE_Event_Reg_State_e {
    VoLTE_Event_Reg_State_Registered        = 1,
    VoLTE_Event_Reg_State_Unregistered      = 2,
    VoLTE_Event_Reg_State_Registering       = 3,
    VoLTE_Event_Reg_State_Deregistering     = 4,
    VoLTE_Event_Reg_State_Disconnected      = 5,
    VoLTE_Event_Reg_State_Authenticating    = 6,
    VoLTE_Event_Reg_State_OOS               = 7,
    /*add here*/
    VoLTE_Event_Reg_State_Connecting        = 8,
    VoLTE_Event_Reg_State_Max
};

enum VoLTE_Event_Emergency_Type_e {
    VoLTE_Event_Emergency_Normal            = 0,                                                ///< normal registration
    VoLTE_Event_Emergency_Registered        = 1,                                                ///< emergency with registration
    VoLTE_Event_Emergency_Unregistered      = 2,                                                ///< emergency without registration
};

#define VOLTE_MAX_P_ACCESS_NETWORK_INFO_LENGTH          (256)
#define VOLTE_MAX_DIGIT_PIDENTIFIER_LENGTH              (256)
#define VOLTE_MAX_DIGIT_PPA_LENGTH                      (4096)

typedef struct _VoLTE_Event_Reg_State_ {
    INT32                                   id;                                                 ///< the account id
    INT32                                   state;                                              ///< refer to ::VoLTE_Event_Reg_State_e
    INT32                                   cause;  /// < refer to ::VoLTE_Event_Reg_Cause_e
    INT32                                   sip_cause;                                          ///< refer to ::VoLTE_Event_SIP_Cause_e

    UINT32                                  conn_info;                                          ///< the connection information for the others UA to create the connection

    /* account information */
    INT8                                    local_address[VOLTE_MAX_ADDRESS_LENGTH];            ///< local ip address
    INT32                                   local_port;                                         ///< local port number
    INT32                                   protocol_type;                                      ///< refer to ::VoLTE_Stack_Protocol_Type_e
    INT32                                   protocol_version;                                   ///< refer to ::VoLTE_Stack_Protocol_Version_e
    INT8                                    public_uid[VOLTE_MAX_REG_UID_LENGTH];               ///< the public user identity
    INT8                                    private_uid[VOLTE_MAX_REG_UID_LENGTH];              ///< the private user identity
    INT8                                    home_uri[VOLTE_MAX_DOMAIN_NAME_LENGTH];             ///< the domain name of the home network
    INT8                                    pcscf_address[VOLTE_MAX_ADDRESS_LIST_LENGTH];       ///< the current used PCSCF ip address
    INT32                                   pcscf_port;                                         ///< the current used PCSCF port number
    INT8                                    imei[VOLTE_MAX_IMEI_LENGTH];                        ///< the IMEI
    INT8                                    associated_uri[VOLTE_MAX_ASSOCIATED_URI];           ///< the list of the associated URI
    INT8                                    pub_gruu[VOLTE_MAX_GRUU_LENGTH];                    ///< the public gruu
    INT8                                    temp_gruu[VOLTE_MAX_GRUU_LENGTH];                   ///< the temp gruu
    INT8                                    service_route[VOLTE_MAX_SERVICE_ROUTE_LENGTH];      ///< the service route
    INT8                                    path[VOLTE_MAX_URI_LENGTH];                         ///< the path
    INT32                                   target_port_s;                                      ///< target_port_s
    INT32                                   net_type;                                           ///< refer to ::VoLTE_Event_Network_Type_e
    INT32                                   net_type_3gpp;                                      ///< refer to ::VoLTE_Event_Network_Type_e
    INT32                                   emergency_type;                                     ///< refer to ::VoLTE_Event_Emergency_Type_e
    INT32                                   ems_mode;                                           ///< refer to ::VoLTE_Event_Ems_Mode_e
    INT32                                   retry_after;                                        ///< notify time delay for IMC retrying initial registration
    INT8                                    reg_timestamp[VOLTE_MAX_TIMESTAMP_LENGTH];          ///< TMO P-Last-Access-Network-Info "REGISTRATION TIMESTAMP"
    INT8                                    security_verify[VOLTE_MAX_SECURIT_VERIFY_LENGTH];   ///< security verify header in All the subsequent SIP requests
    INT8                                    user_agent[VOLTE_MAX_USER_AGENT_LENGTH];            ///< user-agent header for RCS usage
    INT32                                   reg_uri_type;                                       /// todo: define enum for imsi and msisdn uri
    INT8                                    msisdn_uri[VOLTE_MAX_REG_UID_LENGTH];                                         ///

    /* em related variableds */
    UINT8                                   em_reg_timestamp[VOLTE_MAX_TIME_STAMP_LENGTH];      ///< last registered timestamp in milliseconds
    INT8                                    instance_id[VOLTE_MAX_URI_LENGTH];                  ///< the instance id

    /*
        pcscf_usage
        0    default
        1    insert pcscf_addr in method:PCO head (use same pcscf)
        2    insert pcscf_addr in method:PCO tail
    */
    INT8                                    pcscf_usage;
    INT8                                    release_pdn;
    INT8                                    voice_cap;
    INT8                                    video_cap;

    INT8                                    rcs_state;
    INT32                                   rcs_feature;
    INT8                                    tmp_voice_cap;
    INT8                                    tmp_video_cap;

    INT32                                   reg_service;                                        ///< refer to ::ims_reg_service_enum
    INT32                                   via_rport;                                         ///< Via rport
    INT8                                    via_received[VOLTE_MAX_ADDRESS_LENGTH];            ///< Via received parameter

    INT8                                    emergency_reg_got_error;
    INT8                                    sms_cap;
    INT8                                    subscribe_dialog_to_server;                         ///< subscription to the conference event package
    INT8                                    digit_number_of_vline;

    INT8                                    pani[VOLTE_MAX_P_ACCESS_NETWORK_INFO_LENGTH];       ///< P-Acess-Network-Info header for RCS usage
    INT8                                    digit_pidentifier[VOLTE_MAX_DIGIT_PIDENTIFIER_LENGTH]; // 3.6.8.1 Also include following: (Mandatory) ID: GID-MTRREQ-302326
    INT8                                    digit_ppa_header_string[VOLTE_MAX_DIGIT_PPA_LENGTH];   // PPA for all the lines

    INT32                                   reg_event;                                          ///< refer to ::ims_reg_event_enum

    /* ****************************************************************************************************************************** */
    /* Notice: Aligned with volte_core_interface.h what we provided to AP                                                             */
    /* ****************************************************************************************************************************** */
} VoLTE_Event_Reg_State_t;


enum VoLTE_Event_Capability_Type_e {
    VoLTE_Event_Capability_Others           = 0,
    VoLTE_Event_Capability_ICSI             = 1,                                                ///< set or unset the ICSI capability
    VoLTE_Event_Capability_IARI             = 2,                                                ///< set or unset the ICSI capability
    VoLTE_Event_Capability_SMSIP            = 3,                                                ///< enable or disable the SMSoIP (default is enabled)
    VoLTE_Event_Capability_GRUU             = 4,                                                ///< enable or disable the GRUU (default is supported)
    VoLTE_Event_Capability_USSI             = 5,
    //VoLTE_Event_Capability_Multiple_Reg     = 5,                                                ///< stack not supported yet
    //VoLTE_Event_Capability_IPSec            = 6,                                                ///< enable or disable the ipsec connection (default is enabled)
    //VoLTE_Event_Capability_TCP              = 7,                                                ///< enable or disable the tcp connection (default is disabled)
    //VoLTE_Event_Capability_IPv6             = 8,                                                ///< enable or disable the ipv6 connection (default is disabled)
    //VoLTE_Event_Capability_Reg_Event        = 9,                                                ///< enable or disable the registration event package (default is enabled)
    //VoLTE_Event_Capability_Emergency        = 10,                                               ///< 0 : normal, 1 : emergency, 2 : emergency without registration (default is 0)
    /*add here*/
    VoLTE_Event_Capability_Max,
};

typedef struct _VoLTE_Event_Reg_Capability {
    INT32                                   id;                                                 ///< the account id
    INT32                                   type;                                               ///< refer to ::VoLTE_Event_Capability_Type_e
    INT8                                    capability[VOLTE_MAX_REG_CAPABILITY_LENGTH];        ///< the data
    INT32                                   rat_type;                                           ///< RAT type, refer to ::VoLTE_Event_Network_Type_e
} VoLTE_Event_Reg_Capability_t;

typedef struct _VoLTE_Event_External_UA_State_ {
    UINT32                                  id;                                   ///< the account id
    UINT8                                   ua_type;                              ///< UA type
    UINT8                                   ua_state;                             ///< UA state; 0->disable, 1->enable
} VoLTE_Event_External_UA_State_t;
#endif /* __VOLTE_EVENT_H__ */
