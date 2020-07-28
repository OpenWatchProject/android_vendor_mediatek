/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2005
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

/*******************************************************************************
*==============================================================================
*                 HISTORY
* Below this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
*------------------------------------------------------------------------------
* $Log: ims_interface_ap.h,v $
*
*------------------------------------------------------------------------------
* Upper this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
*==============================================================================
*******************************************************************************/
#ifndef _VOLTE_CORE_RCS_INTERFACE_H
#define _VOLTE_CORE_RCS_INTERFACE_H

#include <pthread.h>
/* Change Log -
v4 2017/4/17
*/

#define VoLTE_UA_MAGIC_NUMBER   (0x12140500)

/*ims_interface_ap.h */
#define IMCB_IMC_VOLTE_EVENT_EXTERNAL_IMC_START_CODE (100000)

#define IMC_MAX_IMPU_LEN                256
#define IMC_MAX_DOMAIN_NAME_LEN         256
#define IMC_MAX_URI_LENGTH              128

#define VOLTE_MAX_ACCOUNT_NUM                           (4)
#define VOLTE_MAX_CALL_SESSION                          (3*VOLTE_MAX_ACCOUNT_NUM)
#define VOLTE_MAX_SRVCC_CALLS                           (7)
#define VOLTE_MAX_CELL_ID_LENGTH                        (64)
#define VOLTE_MAX_URI_LENGTH                            (IMC_MAX_URI_LENGTH)
#define VOLTE_MAX_CONF_CONTROL_URI_LENGTH               (IMC_MAX_CONF_CONTROL_URI_LENGTH)
#define VOLTE_MAX_DOMAIN_NAME_LENGTH                    IMC_MAX_DOMAIN_NAME_LEN
#define VOLTE_MAX_USER_AGENT_LENGTH                     (128)
#define VOLTE_MAX_MNC_MCC_LENGTH                        (4)
#define VOLTE_MAX_GRUU_LENGTH                           (128)
#define VOLTE_MAX_ASSOCIATED_URI                        (VOLTE_MAX_URI_LENGTH << 2)
#define VOLTE_MAX_ASSERTED_URI                          (VOLTE_MAX_URI_LENGTH << 2)
#define VOLTE_MAX_IP_LENGTH                             (16)
#define VOLTE_MAX_ADDRESS_LENGTH                        (64)
#define VOLTE_MAX_ADDRESS_LIST_LENGTH                   (256)
#define VOLTE_MAX_REG_UID_LENGTH                        IMC_MAX_IMPU_LEN
#define VOLTE_MAX_REG_UIDS_LENGTH                       IMC_MAX_IMPU_TLV_LEN
#define VOLTE_MAX_AUTH_NC                               (12)
#define VOLTE_MAX_AUTH_NONCE                            (64)
#define VOLTE_MAX_AUTH_REALM                            (32)
#define VOLTE_MAX_AUTH_RESPONSE                         (256)
#define VOLTE_MAX_AUTH_AUTS                             (256)
#define VOLTE_MAX_AUTH_CK                               (256)
#define VOLTE_MAX_AUTH_IK                               (256)
#define VOLTE_MAX_IMEI_LENGTH                           (20)
#define VOLTE_MAX_IMSI_LENGTH                           (16)
#define VOLTE_MAX_REG_CAPABILITY_LENGTH                 (256)
#define VOLTE_MAX_CALL_CAPABILITY_LENGTH                (128)
#define VOLTE_MAX_SDP_PARAMETER_SET_LENGTH              (256) // H.264, Spros parameter sets, must align with LTE_VT_SRV_UA_PARAMETER_SIZE_MAX.
#define VOLTE_MAX_RTP_ADDRESS_LENGTH                    (64)// shall be align to SDP c attribute
#define VOLTE_MAX_SDP_FMTP_SUB_VALUE_LENGTH             (16)// The maximum length of a fmtp specific parameter value
#define VOLTE_MAX_SDP_FMTP_SUB_VALUE_LENGTH_IN          (32)// The maximum length of a fmtp specific parameter value internal
#define VOLTE_MAX_SDP_FMTP_TOT_VALUE_LENGTH             (32)// The maximum length of total fmtp specific parameter key and value, must align with LTE_VT_SRV_UA_FMTP_SIZE_MAX.
#define VOLTE_MAX_SDP_TWOBYTE_LENGTH                    (2)
#define VOLTE_MAX_SDP_EGBYTE_LENGTH                     (8)
#define VOLTE_IPADDR_LENGTH                             (16) // IP address in binary
#define VOLTE_MAX_DATA_TO_UPPER_LENGTH                  (4000)
#define VOLTE_MAX_DATA_TO_IMC_LENGTH                    (512)
#define VOLTE_MAX_TIMESTAMP_LENGTH                      (256)
#define VOLTE_MAX_SDP_RED_VALUE_LENGTH                  (16)
#define VOLTE_MAX_IF_NAME_LENGTH                        (IMC_MAX_IF_NAME_LENGTH)
#define VOLTE_MAX_SECURIT_VERIFY_LENGTH                 (768) //6 algo combo
#define VOLTE_MAX_CALL_NUM_IN_CONF                      (5)
#define VOLTE_MAX_TIME_STAMP_LENGTH                     (32)
#define VOLTE_MAX_PLANI_LENGTH                          (256)
#define VOLTE_MAX_LEVEL_SIZE                            (20)
#define VOLTE_MAX_CNAME_LENGTH                          (52)
#define VOLTE_USSD_STRING_MAX_LENGTH                    (1000)
#define VOLTE_USSD_LANGUAGE_MAX_LENGTH                  (32)
#define VOLTE_MAX_MSISDN_LENGTH                         (20)  // dialing number
#define VOLTE_MAX_SMS_TPDA_LENGTH                       (21)
#define VOLTE_MAX_REASON_TEXT_LENGTH                    (128)
#define VOLTE_MAX_SERVICE_ROUTE_LENGTH  256
#define VOLTE_MAX_P_ACCESS_NETWORK_INFO_LENGTH          (256)
#define VOLTE_MAX_DIGIT_PIDENTIFIER_LENGTH              (256)
#define VOLTE_MAX_DIGIT_PPA_LENGTH                      (4096)


typedef unsigned int UINT32;
typedef int INT32;
typedef unsigned short UINT16;
typedef short INT16;
typedef unsigned char UINT8;
typedef char INT8;

/* inet_msg_def.h */
typedef enum {
    INET_METHOD_SIP_REGISTER,       ///<  0, SIP REGISTER
    INET_METHOD_SIP_INVITE,         ///<  1, SIP INVITE
    INET_METHOD_SIP_REFER,          ///<  2, SIP REFER
    INET_METHOD_SIP_UPDATE,         ///<  3, SIP UPDATE
    INET_METHOD_SIP_CANCEL,         ///<  4, SIP CANCEL
    INET_METHOD_SIP_MESSAGE,        ///<  5, SIP MESSAGE
    INET_METHOD_SIP_ACK,            ///<  6, SIP ACK
    INET_METHOD_SIP_BYE,            ///<  7, SIP BYE
    INET_METHOD_SIP_OPTIONS,        ///<  8, SIP OPTIONS
    INET_METHOD_SIP_SUBSCRIBE,      ///<  9, SIP SUBSCRIBE
    INET_METHOD_SIP_NOTIFY,         ///< 10, SIP NOTIFY
    INET_METHOD_SIP_PUBLISH,        ///< 11, SIP PUBLISH
    INET_METHOD_SIP_INFO,           ///< 12, SIP INFO
    INET_METHOD_SIP_PRACK,          ///< 13, SIP PRACK
    INET_METHOD_HTTP_GET,
    INET_METHOD_HTTP_POST,
    INET_METHOD_HTTP_TRACE,
    INET_METHOD_HTTP_HEAD,
    INET_METHOD_HTTP_PUT,
    INET_METHOD_HTTP_DELETE,
    INET_METHOD_HTTP_CONNECT,
    INET_METHOD_RTSP_OPTIONS,
    INET_METHOD_RTSP_REDIRECT,
    INET_METHOD_RTSP_DESCRIBE,
    INET_METHOD_RTSP_SETUP,
    INET_METHOD_RTSP_PLAY,
    INET_METHOD_RTSP_PAUSE,
    INET_METHOD_RTSP_TEARDOWN,
    INET_METHOD_RTSP_SET_PARAMETER,
    INET_METHOD_PXY_DEBUG,
    INET_METHOD_UNRECOGNIZED,
    INET_METHOD_NUM,
    INET_METHOD_MAX                     = 0xFFFFFFFF
} inet_method_enum;

typedef enum {
    INET_HDR_ACCEPT,                    /* 0 */
    INET_HDR_ACCEPT_CHARSET,            /* 01 */
    INET_HDR_ACCEPT_CONTACT,            /* 02 */
    INET_HDR_ACCEPT_ENCODING,           /* 03 */
    INET_HDR_ACCEPT_LANGUAGE,           /* 04 */
    INET_HDR_ACCEPT_RANGES,             /* 05 */
    INET_HDR_AGE,                       /* 06 */
    INET_HDR_ALERT_INFO,                /* 07 */
    INET_HDR_ALLOW,                     /* 08 */
    INET_HDR_AUTH_INFO,                 /* 09 */
    INET_HDR_AUTHORIZATION,             /* 10 */
    INET_HDR_BSID,                      /* 11 */
    INET_HDR_CACHE_CONTROL,             /* 12 */
    INET_HDR_CALL_ID,                   /* 13 */
    INET_HDR_CONTACT,                   /* 14 */
    INET_HDR_CONNECTION,                /* 15 */
    INET_HDR_CONTENT_DISPOSITION,       /* 16 */
    INET_HDR_CONTENT_ENCODING,          /* 17 */
    INET_HDR_CONTENT_LANGUAGE,          /* 18 */
    INET_HDR_CONTENT_LENGTH,            /* 19 */
    INET_HDR_CONTENT_TYPE,              /* 20 */
    INET_HDR_CONTENT_ID,                /* 21 */
    INET_HDR_CONTENT_LOCATION,          /* 22 */
    INET_HDR_CONTENT_MD5,               /* 23 */
    INET_HDR_CONTENT_RANGE,             /* 24 */
    INET_HDR_CONTENT_TRANSER_ENCODING,  /* 25 */
    INET_HDR_COOKIE,                    /* 26 */
    INET_HDR_CSEQ,                      /* 27 */
    INET_HDR_DATE,                      /* 28 */
    INET_HDR_ERROR_INFO,                /* 29 */
    INET_HDR_EXPECT,                    /* 30 */
    INET_HDR_EXPIRES,                   /* 31 */
    INET_HDR_ETAG,                      /* 32 */
    INET_HDR_FROM,                      /* 33 */
    INET_HDR_HOST,                      /* 34 */
    INET_HDR_IF_MATCH,                  /* 35 */
    INET_HDR_IF_MODIFIED_SINCE,         /* 36 */
    INET_HDR_IF_NONE_MATCH,             /* 37 */
    INET_HDR_IF_RANGE,                  /* 38 */
    INET_HDR_IF_UNMODIFIED_SINCE,       /* 39 */
    INET_HDR_IN_REPLY_TO,               /* 40 */
    INET_HDR_LAST_MODIFIED,             /* 41 */
    INET_HDR_LOCATION,                  /* 42 */
    INET_HDR_MAX_FORWARDS,              /* 43 */
    INET_HDR_METHOD,                    /* 44 */
    INET_HDR_MIN_EXPIRES,               /* 45 */
    INET_HDR_MIME_VERSION,              /* 46 */
    INET_HDR_ORGANIZATION,              /* 47 */
    INET_HDR_PRAGMA,                    /* 48 */
    INET_HDR_PRIORITY,                  /* 49 */
    INET_HDR_PROXY_AUTHENTICATE,        /* 50 */
    INET_HDR_PROXY_AUTHORIZATION,       /* 51 */
    INET_HDR_PROXY_REQUIRE,             /* 52 */
    INET_HDR_RANGE,                     /* 53 */
    INET_HDR_REASON_PHRASE,             /* 54 */
    INET_HDR_RECORD_ROUTE,              /* 55 */
    INET_HDR_REPLY_TO,                  /* 56 */
    INET_HDR_REFER,                     /* 57 */
    INET_HDR_REFER_TO,                  /* 58 */
    INET_HDR_REQUIRE,                   /* 59 */
    INET_HDR_RESP_CODE,                 /* 60 */
    INET_HDR_RETRY_AFTER,               /* 61 */
    INET_HDR_ROUTE,                     /* 62 */
    INET_HDR_SERVER,                    /* 63 */
    INET_HDR_SET_COOKIE,                /* 64 */
    INET_HDR_SET_COOKIE2,               /* 65 */
    INET_HDR_SESSION_EXPIRES,           /* 66 */
    INET_HDR_SUBJECT,                   /* 67 */
    INET_HDR_SUPPORTED,                 /* 68 */
    INET_HDR_TIMESTAMP,                 /* 69 */
    INET_HDR_TRAILER,                   /* 70 */
    INET_HDR_TRANSFER_ENCODING,         /* 71 */
    INET_HDR_TE,                        /* 72 */
    INET_HDR_TO,                        /* 73 */
    INET_HDR_UNSUPPORTED,               /* 74 */
    INET_HDR_UPGRADE,                   /* 75 */
    INET_HDR_URI,                       /* 76 */
    INET_HDR_USER_AGENT,                /* 77 */
    INET_HDR_VIA,                       /* 78 */
    INET_HDR_VARY,                      /* 79 */
    INET_HDR_WARNING,                   /* 80 */
    INET_HDR_WWW_AUTHENTICATE,          /* 81 */
    INET_HDR_REQUEST_URI,               /* 82 */
    INET_HDR_START_LINE_PROTO,          /* 83 */
    INET_HDR_MIN_SE,                    /* 84 */
    INET_HDR_EVENT,                     /* 85 */
    INET_HDR_SUBSCRIPTION_STATE,        /* 86 */
    INET_HDR_SIP_IF_MATCH,              /* 87 */
    INET_HDR_SIP_ETAG,                  /* 88 */
    INET_HDR_P_ALERTING_MODE,           /* 89 */
    INET_HDR_P_PREFERRED_IDENTITY,      /* 90 */
    INET_HDR_PRIVACY,                   /* 91 */
    INET_HDR_PRIV_ANSWER_MODE,          /* 92 */
    INET_HDR_ANSWER_MODE,               /* 93 */
    INET_HDR_REFERRED_BY,               /* 94 */
    INET_HDR_REPLACES,                  /* 95 */
    INET_HDR_JOIN,                      /* 96 */
    INET_HDR_ALLOW_EVENTS,              /* 97 */
    INET_HDR_MSG_WAITING,               /* 98 */
    INET_HDR_MSG_ACCOUNT,               /* 99 */
    INET_HDR_MW_VOICE_MSG,              /* 100 */
    INET_HDR_MW_FAX_MSG,                /* 101 */
    INET_HDR_MW_PAGER_MSG,              /* 102 */
    INET_HDR_MW_MM_MSG,                 /* 103 */
    INET_HDR_MW_TEXT_MSG,               /* 104 */
    INET_HDR_MW_NONE_MSG,               /* 105 */
    INET_HDR_RACK,                      /* 106 */
    INET_HDR_RSEQ,                      /* 107 */
    INET_HDR_SESSION,                   /* 108 */
    INET_HDR_TRANSPORT,                 /* 109 */
    INET_HDR_RTPINFO,                   /* 110 */
    INET_HDR_REASON,                    /* 111 */
    INET_HDR_REJECT_CONTACT,            /* 112 */
    INET_HDR_P_ASSERTED_IDENTITY,       /* 113 */
    INET_HDR_SERVICE_ROUTE,             /* 114 */
    INET_HDR_P_ACCESS_NETWORK_INFO,     /* 115 */
    INET_HDR_P_ASSOCIATED_URI,          /* 116 */
    INET_HDR_P_CALLED_PARTY_ID,         /* 117 */
    INET_HDR_P_VISITED_NETWORK_ID,      /* 118 */
    INET_HDR_P_ASSERTED_SERVICE,        /* 119 */
    INET_HDR_P_PREFERRED_SERVICE,       /* 120 */
    INET_HDR_SECURITY_CLIENT,           /* 121 */
    INET_HDR_SECURITY_SERVER,           /* 122 */
    INET_HDR_SECURITY_VERIFY,           /* 123 */
    INET_HDR_HISTORY_INFO,              /* 124 */
    INET_HDR_REQUEST_DISPOSITION,       /* 125 */
    INET_HDR_P_EARLY_MEDIA,             /* 126 */
    INET_HDR_PATH,                      /* 127 */
    INET_HDR_P_LAST_ACCESS_NETWORK_INFO,/* 128 */
    INET_HDR_P_EMERGENCY_CALL_MODE_PREFERENCE,  /* 129 */
    INET_HDR_GEOLOCATION,               /* 130 */
    INET_HDR_GEOLOCATION_ROUTING,       /* 131 */
    INET_HDR_FEATURE_CAPS,              /* 132 */
    INET_HDR_RECV_INFO,                 /* 133 */
    INET_HDR_INFO_PACKAGE,              /* 134 */
    INET_HDR_P_EMERGENCY_INFO,          /* 135 */
    INET_HDR_IDENTITY,                  /* 136 */
    INET_HDR_IDENTITY_INFO,             /* 137 */
    INET_HDR_CELLULAR_NETWORK_INFO,     /* 138 */
    INET_HDR_UNRECOGNIZED,
    INET_HDR_NUM,
    INET_HDR_MAX                        = 0xFFFFFFFF
} inet_header_field_enum;

#define SIP_HDR_CONTENT_TYPE                INET_HDR_CONTENT_TYPE
#define SIP_HDR_EVENT                       INET_HDR_EVENT

/* sip_status_code.h */
#define SIP_MESSAGE_METHOD      INET_METHOD_SIP_MESSAGE     ///< MESSAGE
#define SIP_NOTIFY_METHOD       INET_METHOD_SIP_NOTIFY      ///< NOTIFY

/* volte_event.h */
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
    VOLTE_REQ_SIPTX_IF_REG_READY            = VOLTE_EVENT_TO_SIPTX_REG_START_CODE + 34,


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

typedef struct _VoLTE_Event_Presence_Unpublish {
    INT32                                   acct_id;
    INT32                                   data;
} VoLTE_Event_Presence_Unpublish_t;

typedef struct _VoLTE_Event_Reg_Dereg_Start_ {
    UINT32                                  id;
} VoLTE_Event_Reg_Dereg_Start_t;

/* volte_stack.h */
enum VoLTE_Stack_Result_e {
    VoLTE_Stack_Result_Disconnected     = -3,
    VoLTE_Stack_Result_Timeout          = -2,
    VoLTE_Stack_Result_Error            = -1,
    VoLTE_Stack_Result_Success          = 0
};

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

enum VoLTE_Stack_UA_Type_e {
    VoLTE_Stack_UA_Reg                  = 1,
    VoLTE_Stack_UA_Normal               = 2,
    VoLTE_Stack_UA_Call                 = 3,
    VoLTE_Stack_UA_SMS                  = 4,
    VoLTE_Stack_UA_Service              = 5,
    VoLTE_Stack_UA_RCS_Proxy            = 6,
    VoLTE_Stack_UA_USSD                 = 7,
    VoLTE_Stack_UA_Presence             = 8,
    VoLTE_Stack_UA_ClientAPI_Proxy      = 9, /*client api */
    VoLTE_Stack_UA_RCS_Proxy_Sip        = 10,/* add for ims Submarine */
    VoLTE_Stack_UA_Presence_Sip         = 11,/* add for ims Submarine */

    VoLTE_Stack_UA_Count
};


typedef enum {
    INET_DATA_TYPE_MEMORY,
    INET_DATA_TYPE_FILE,
    INET_DATA_TYPE_VFILE
} inet_data_type_enum;

enum VoLTE_Event_Reg_Cause_e {
    VoLTE_Event_Reg_Cause_Normal                                = 0,
    VoLTE_Event_Reg_Cause_Internal_Error                        = 1,
    VoLTE_Event_Reg_Cause_Stack_Error                           = 2,
    VoLTE_Event_Reg_Cause_Timer_Error                           = 3,
    VoLTE_Event_Reg_Cause_Try_Next_PCSCF                        = 4,
    VoLTE_Event_Reg_Cause_PCSCF_All_Failed                      = 5,
    VoLTE_Event_Reg_Cause_Auth_Error                            = 6,
    VoLTE_Event_Reg_Cause_Reset                                 = 7,
    VoLTE_Event_Reg_Cause_AssURI_Changed                        = 8,
    VoLTE_Event_Reg_Cause_Network_initiated                     = 10,
    VoLTE_Event_Reg_Cause_Channel_Bind                          = 11,
    VoLTE_Event_Reg_Cause_OOS                                   = 12,
    VoLTE_Event_Reg_Cause_SIP_Error                             = 13,
    VoLTE_Event_Reg_Cause_Parameter_Error                       = 14,
    VoLTE_Event_Reg_Cause_Not_Bind                              = 15,
    VoLTE_Event_Reg_Cause_Not_Auto_Re_Reg                       = 16,
    VoLTE_Event_Reg_Cause_Retry_After                           = 17,
    VoLTE_Event_Reg_Cause_IMS_PDN_Fatal_Fail                    = 18,
    VoLTE_Event_Reg_Cause_MD_Lower_Layer_Err                    = 19,
    VoLTE_Event_Reg_Cause_C2K_Fail                              = 20,
    VoLTE_Event_Reg_Cause_Retry_By_RFC5626                      = 21,
    /*add here*/
    VoLTE_Event_Reg_Cause_UNKNOWN                               = 99,
    VoLTE_Event_Reg_Cause_Max                                   = VoLTE_Event_Reg_Cause_UNKNOWN,
};

typedef struct _VoLTE_Stack_Sip_Info {
    int                             conn_id;
    int                             request_id;
    int                             message_len;
    char                            message[0];
} VoLTE_Stack_Sip_Info_t;


typedef struct _VoLTE_Stack_Channel_Info {
    int                             type;               /* VoLTE_Stack_UA_Type_e */
    unsigned int                    conn_info;
} VoLTE_Stack_Channel_Info_t;


typedef struct _VoLTE_Stack_Message {
    unsigned int                    type;
    unsigned int                    len;
    unsigned int                    flag;
    void                           *data;
} VoLTE_Stack_Message_t;

typedef void (*VoLTE_Stack_Ua_Read_Sip_Fn) (VoLTE_Stack_Sip_Info_t *sip, void *user_data);

typedef void (*VoLTE_Stack_Ua_Read_Msg_Fn) (VoLTE_Stack_Message_t *msg, void *user_data);

typedef void (*VoLTE_Stack_Read_Msg_Fn) (VoLTE_Stack_Message_t *msg, void *user_data, void *channel);

typedef struct _VoLTE_Stack_Callback {
    VoLTE_Stack_Ua_Read_Sip_Fn      read_sip;
    VoLTE_Stack_Ua_Read_Msg_Fn      read_msg;
    //VoLTE_Stack_Ua_Read_Reg_Fn      read_reg;
    void                           *user_data;
} VoLTE_Stack_Callback_t;

/* volte_stack.c */
typedef struct _volte_stack_req_obj {
    pthread_mutex_t             mutex;
    pthread_cond_t              cond;
    int                         result;
    unsigned int                data[4];
} volte_stack_req_obj_t;

typedef struct _volte_stack_req_msg {
    unsigned int                type;
    char                        data[0];
} volte_stack_req_msg_t;

typedef struct _volte_stack_obj {
    void                       *channel;

    /* client */
    void                       *msgq;
    void                       *msgq_task;
    int                         reg_id;
    short                       connected;
    short                       closed;
    VoLTE_Stack_Ua_Read_Sip_Fn  ua_read_sip;
    VoLTE_Stack_Ua_Read_Msg_Fn  ua_read_msg;
    void                       *user_data;
    volte_stack_req_obj_t       req;
    pthread_mutex_t             mutex;
    pthread_mutex_t             read_mutex;
    unsigned int                last_msg_type;

    /* server */
    VoLTE_Stack_Read_Msg_Fn     read_msg;
    void                       *read_msg_user_data;
} volte_stack_obj_t;

/* volte_cap.h */
typedef struct rule_cond {
    int     len_of_param;
    int     header;
    char    *param_p;
    struct rule_cond *next_p;
} VoLTE_Stack_Match_Rule_Cond;

typedef struct rule_item {
    int     len_of_rule_item;
    int     num_of_cond;
    struct rule_cond *cond_p;
    struct rule_item *next_p;
} VoLTE_Stack_Match_Rule_Item;

typedef struct rule_set {
    int     len_of_rule_set;
    int     num_of_rule;
    int     method;

    struct rule_item    *item_p;
    struct rule_set     *next_p;
} VoLTE_Stack_Match_Rule_Set;

typedef struct rule_level_2{
    int     len_of_level_2;
    int     num_of_set;
    struct rule_set     *set_p;
} VoLTE_Stack_Match_Level_2;

typedef struct _VoLTE_Stack_Ua_Capability {
    char                        *user_info;         // lv0, ex. "volte_ua"
    char                        *associated_set;    // lv0, ex. "sip:ua@home,tel:+19911"
    char                        *feature_sets;      // lv1, ex. "+g.3gpp.cs-voice,urn:urn-7:3gpp-application.ims.iari.gsma-is"
    char                        *body_str;          // lv3, ex. "\r\nm=audio "
    VoLTE_Stack_Match_Level_2   *lv2;               // lv2, a tree obj
} VoLTE_Stack_Ua_Capability_t;

/* inet_msg_struct.h */
typedef struct {
    int     int_value;
    char    *str_value;
} inet_int_str_struct;

typedef struct inet_header_st {
    inet_header_field_enum  header_id;
    char                    *header_name;
    unsigned int            value;          ///< value may store pointer, the details can be found the following table
    struct inet_header_st   *next_dup_hdr;  ///< for duplicate header node
} inet_header_struct;

typedef struct inet_header_list_st {
    inet_header_struct          *object;
    struct inet_header_list_st  *next;
} inet_header_list_struct;

typedef struct {
    inet_header_list_struct *header_list;
    inet_data_type_enum     data_type;
    char                    *body_file;
    char                    *body_buf;
    unsigned int            body_file_offset;
    int                     body_len;
} inet_body_struct;

typedef struct inet_body_list_st {
    inet_body_struct            *object;
    struct inet_body_list_st    *next;
} inet_body_list_struct;

typedef struct {
    int                         magic;
    unsigned int                request_id;
    //int                         conn_id;
    unsigned int                app_type;
    inet_int_str_struct        *method;
    unsigned int               *resp_code;
    inet_header_list_struct    *header_list;
    inet_body_list_struct      *body_list;
} inet_message_struct;

typedef inet_message_struct             sip_message_struct;

/* api_channel.h */
typedef struct _channel_data_ {
    int         type;
    int         len;
    int         flag;
    void       *data;
} Channel_Data_t;

/* volte_event.h */
#if 1 //mp-n0.mp7
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
} VoLTE_Event_Reg_State_t;
#endif

/* start IMS Submarine */
typedef struct _VoLTE_Event_IMS_HIF_Data {
    int msg_type;
    int len;
    int flag;
    int acct_id;
    int ua_type;
    char data[0];
} VoLTE_Event_IMS_HIF_Data;
/* end */

#define volte_sip_stack_create(i, c, cb)    _volte_sip_stack_create((i),(c),(cb),__FILE__,__LINE__)

extern int (*volte_sip_info_free)(VoLTE_Stack_Sip_Info_t*);
extern int (*volte_sip_stack_init)();
extern void *(*_volte_sip_stack_create)(VoLTE_Stack_Channel_Info_t*, VoLTE_Stack_Ua_Capability_t*, VoLTE_Stack_Callback_t*, char*, int);
extern int (*volte_sip_stack_destroy)(void*);
extern int (*volte_sip_stack_send_sip)(void *,VoLTE_Stack_Sip_Info_t *);
extern int (*volte_sip_stack_send_msg)(void *,VoLTE_Stack_Message_t *);
extern int (*volte_sip_stack_reg_state)(void *);
extern int (*volte_sip_stack_reg_bind)(void *, int );
extern int (*volte_sip_stack_reg_capability)(void *, int , char *);
extern int (*volte_sip_stack_restore)(void *);
extern int (*Rule_Capability_Init)(VoLTE_Stack_Ua_Capability_t*);
extern int (*Rule_Capability_Deinit)(VoLTE_Stack_Ua_Capability_t*);
extern int (*Rule_Level0_Set)(VoLTE_Stack_Ua_Capability_t*, char*, char*);
extern int (*Rule_Level1_Set)(VoLTE_Stack_Ua_Capability_t*, char*);
extern int (*Rule_Level2_Set)(VoLTE_Stack_Ua_Capability_t*, const int , const int , char*);
extern int (*Rule_Level3_Set)(VoLTE_Stack_Ua_Capability_t*, char*);
extern int (*Channel_Send)(void *channel, Channel_Data_t *data);
extern int (*volte_sip_stack_ind_external_ua_state)(void *, int, int, int);

#endif //_VOLTE_CORE_RCS_INTERFACE_H
