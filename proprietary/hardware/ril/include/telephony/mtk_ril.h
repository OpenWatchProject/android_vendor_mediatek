/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#ifndef MTK_RIL_H
#define MTK_RIL_H

#include <telephony/ril.h>
#include <telephony/mtk_ril_embms_def.h>
#include <stdbool.h>

#ifndef MTK_TELEPHONYWARE_SUPPORT
#include <log/log.h>

#define LOGV RLOGV
#define LOGD RLOGD
#define LOGE RLOGE
#define LOGI RLOGI
#define LOGW RLOGW
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_SIM_COUNT 4
#define MAX_PROP_CHARS   50
#define MAX_RFX_SLOT_COUNT 4
extern int RFX_SLOT_COUNT;

#define PROPERTY_3G_SIM "persist.vendor.radio.simswitch"

#define MAX_IPV4_ADDRESS_LENGTH 17
#define MAX_IPV6_ADDRESS_LENGTH 65

// M: Ims Data Framework
#define MAX_PCSCF_NUMBER 16

// M: [OD over ePDG] start
#define RAT_TYPE_UNSPEC 0
#define RAT_TYPE_MOBILE_3GPP 1
#define RAT_TYPE_WIFI 2
#define RAT_TYPE_MOBILE_3GPP2 3
#define RAT_TYPE_MAX 4
#define RAT_TYPE_KEY 1000
// M: [OD over ePDG] end

// M: [Inactive Timer] start
#define APN_AUTH_TYPE_MAX_NUM 7
#define APN_INACTIVE_TIMER_KEY 3
// M: [Inactive Timer] end

#define MAX_LCE_THRESHOLD_NUMBER 15

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif

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
} Qos;

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
} PktFilter;

typedef struct {
    int authTokenNumber;
    int authTokenList[16];
    int flowIdNumber;
    int flowIdList[4][4];
} AuthToken;

typedef struct {
    int linkedPfNumber;
    int linkedPfList [16];
    int authtokenFlowIdNumber;
    AuthToken authtokenFlowIdList[4];
} TftParameter;

typedef struct {
    int operation;
    int pfNumber;
    PktFilter pfList [16]; //for response, just send necessary length. check responseSetupDedicateDataCall in ril.cpp
    TftParameter tftParameter;
} Tft;

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
    Qos             qos;
    int             hasTft;
    Tft             tft;
    int             hasPcscf;
    char            pcscf[MAX_PCSCF_NUMBER * (MAX_IPV6_ADDRESS_LENGTH)];
} RIL_Dedicate_Data_Call_Struct;

typedef struct {
    int             isValid;
    Qos             qos;
    int             emergency_ind;
    int             pcscf_discovery_flag;
    int             signalingFlag;
    int             isHandover;
    int             assigned_rate;  //assign by MAL
} RIL_Default_Bearer_VA_Config_Struct;

// FastDormancy
typedef struct {
    int paramNumber; // record total number of arguments of this mode
    int mode; // allowed mode:0,1,2,3
    int parameter1; // only mode 2 and 3 has parameter1 this field
    int parameter2; // only mode2 has parameter2 this field
} RIL_FdModeStructure;

/// M: ril proxy
/* <name> in CNAP URC is an up to 80 characters long string containing the calling name  */
#define MAX_CNAP_LENGTH 81
/* <ccidx> in CLCC response ranges from 1 to N.
 * N, the maximum number of simultaneous call control processes is implementation specific.
 * For GSM, N is 7
 */
#define MAX_GSMCALL_CONNECTIONS 7   // only 7 connections allowed in GSM
typedef enum {
    MT_CALL_NONE = 0,
    MT_CALL_REJECTED = 1,
    MT_CALL_MISSED = 2,
    MT_CALL_NUMREDIRECT = 3,
    MT_CALL_RQ = 4,
    MT_CALL_GWSD = 10,

    MT_CALL_DIAL_IMS_STK = 100,  // >= 100 is for IMS call.
    MT_CALL_IMS_GWSD = 101,
    MT_CALL_ENRICHED_CALLING = 102
} RIL_CALL_INFO_TYPE;

typedef enum {
    RADIO_TECH_GROUP_GSM = 0,
    RADIO_TECH_GROUP_C2K = 1,
    RADIO_TECH_GROUP_ATCI = 2,
    RADIO_TECH_GROUP_SAP_GSM = 3,
    RADIO_TECH_GROUP_SAP_C2K = 4,
    RADIO_TECH_GROUP_SAP_BT = 5,
    // External SIM [Start]
    RADIO_TECH_GROUP_VSIM = 6,
    // External SIM [End]
    RADIO_TECH_GROUP_NUM
} RILD_RadioTechnology_Group;

typedef enum {
    RFX_MSG_TYPE_NONE = -1,
    REQUEST = 0,
    RESPONSE = 1,
    URC = 2,
    REQUEST_ACK = 3,
    RAW_URC = 4,
    EVENT = 5,
    STATUS_SYNC = 6,
    SAP_REQUEST = 7,
    SAP_RESPONSE = 8,
    SAP_URC = 9,
    RFX_MSG_TYPE_NUM
} RFX_MESSAGE_TYPE;

/// M: ril proxy
typedef struct RequestDestInfo {
    int request;
    RILD_RadioTechnology_Group dest;
} RequestDestInfo;

typedef struct DispatchInfo {
    RequestDestInfo* reqInfo;
    int32_t token;
    RIL_SOCKET_ID socketId;
    struct DispatchInfo *p_next;
} DispatchInfo;

typedef enum {
    ApplyRC_NONEED,
    ApplyRC_SUCCESS,
    APPLYRC_FAIL
} ApplyRadioCapabilityResult;

typedef struct {
    int command;    /* one of the commands listed for TS 27.007 +CRSM*/
    int fileid;     /* EF id */
    char *path;     /* "pathid" from TS 27.007 +CRSM command.
                       Path is in hex asciii format eg "7f205f70"
                       Path must always be provided.
                     */
    int p1;
    int p2;
    int p3;
    char *data;     /* May be NULL*/
    char *pin2;     /* May be NULL*/
    char *aidPtr;   /* AID value, See ETSI 102.221 8.1 and 101.220 4, NULL if no value. */
    int sessionId;    /* Channel, which is used to get files from multi-application */
} RIL_SIM_IO_EX_v6;

// MTK-START: SIM GBA
//General SIM Authentication
typedef struct {
    int sessionId;
    int mode;
    char * param1;
    char * param2;
    int tag;
} RIL_SimAuthStructure;
// MTK-END

/* Used by RIL_UNSOL_CRSS_NOTIFICATION */
// MTK-START: SIM ME LOCK
typedef struct{
    int catagory;
    int state;
    int retry_cnt;
    int autolock_cnt;
    int num_set;
    int total_set;
    int key_state;
} RIL_SimMeLockCatInfo;

typedef struct{
    RIL_SimMeLockCatInfo catagory[7];
    char imsi[16];
    int isgid1;
    char gid1[16];
    int isgid2;
    char gid2[16];
    int mnclength;
} RIL_SimMeLockInfo;
// MTK-END

// MTK-START: SIM TMO RSU
typedef enum {
    MTK_RIL_SIM_GSM_REQ = 0,
    MTK_RIL_SIM_CDMA_REQ,
    MTK_RIL_SIM_COMM_REQ,
    MTK_RIL_SIM_GSM_URC,
    MTK_RIL_SIM_CDMA_URC,
    MTK_RIL_SIM_COMM_URC
 } RIL_SimHanlderClass;
// MTK-END

typedef enum {
    POWER_DOWN,
    POWER_UP,
    POWER_UP_PASS_THROUGH,
}RIL_CardPowerState;

/* VILTE start */
typedef struct {
    int             phone; //Slot id
    int             call_id;
    int             call_state;

    char           *data;
} RIL_VT_CALL_STATUS_UPDATE;

typedef struct {
    int             slot_id;
    int             size;
    char           *data;
} RIL_VT_SERVICE_MSG;
/* VILTE end */

typedef struct {
    int    code;            /*
                             * 0: +CCWA
                             * 1: +CDIP
                             * 2: +CLIP
                             * 3: +COLP
                             */
    int    type;            /* type of address octet in integer format (refer GSM 04.08 [8] subclause 10.5.4.7) */
    char * number;          /* string type phone number of format specified by <type> */
    char * alphaid;         /*
                             * optional string type alphanumeric representation of <number>
                             * corresponding to the entry found in phonebook;
                             */
    int    cli_validity;    /* CLI validity value,
                               0: PRESENTATION_ALLOWED,
                               1: PRESENTATION_RESTRICTED,
                               2: PRESENTATION_UNKNOWN
                            */
} RIL_CrssNotification;

// IMS Data notification response
typedef struct {
    int phone; //Slot id
    int aid; //Allocated ID by MD
    char* type; //ims or emergency pdn type
    int action; //activate or deacitvate
} RIL_IMS_BearerNotification;

typedef struct {
    int phone; //Slot id
    char* type; //ims or emergency pdn type
    char* event; //info event type (e.g. CC 33 event)
    char* extra; //extra info for this event
} RIL_IMS_DataInfoNotify;

typedef struct {
    int account_id;
    int response_code;
    char* direction;
    char* sip_msg_type;
    char* method;
    char* reason_phrase;
    char* warn_text;
} RIL_SipRegInfo;

enum ImsRegInfoTypeEnum {
    IMS_REGISTERING = 0,
    IMS_REGISTERED,
    IMS_REGISTER_FAIL
};

typedef struct {
    int report_type;                  // refer to enum ImsRegStatusReportType
    int account_id;
    int expire_time;                  // How long will the IMS registration connection expire
    int error_code;                   // Error code if registration fail
    char* uri;                           // Profile URI
    char* error_msg;                     // Error message if registration fail
} RIL_ImsRegInfo;

typedef struct {
    int reg_state;
    int reg_type;
    int ext_info;
    int dereg_cause;
    int ims_retry;
    int rat;
    int sip_uri_type;
} RIL_EiregData;


typedef struct {
    int             status;     /* A RIL_DataCallFailCause, 0 which is PDP_FAIL_NONE if no error */
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
    int             rat;        /* 0: UNSPEC, 1: MOBILE_3GPP, 2: WIFI, 3: MOBILE_3GPP2
                                   4: TYPE_MAX, 0xFE: RDS_RAN_NONE, 0xFF: RDS_RAN_DENY */
} MTK_RIL_Data_Call_Response_v11;

/// M: Add the new network type. @{
typedef enum {
PREF_NET_TYPE_LTE_GSM                         = 101, /* LTE/GSM */
PREF_NET_TYPE_LTE_TDD_ONLY                    = 102, /* LTE TDD only */
PREF_NET_TYPE_CDMA_GSM                        = 103, /* CDMA,GSM(2G Global) */
PREF_NET_TYPE_CDMA_EVDO_GSM                   = 104, /* CDMA,EVDO,GSM */
PREF_NET_TYPE_LTE_CDMA_EVDO_GSM               = 105, /* LTE,CDMA,EVDO,GSM(4G Global, 4M) */
} RIL_Mtk_PreferredNetworkType;
/// @}

// M: [VzW] Data Framework @{
typedef struct {
    int cid;                    /* Context ID, uniquely identifies this call */
    char *apn_name;             /* APN name */
    char *bearer_proto;         /* One of the PDP_type values in TS 27.007 section 10.1.1.
                                   For example, "IP", "IPV6", "IPV4V6". */
    int pco_id;                 /* The protocol ID for this box.  Note that only IDs from
                                   FF00H - FFFFH are accepted.  If more than one is included
                                   from the network, multiple calls should be made to send all
                                   of them. */
    int contents_length;        /* The number of octets in the contents. */
    char *contents;             /* Carrier-defined content.  It is binary, opaque and
                                   loosely defined in LTE Layer 3 spec 24.008 */
} RIL_PCO_Data_attached;
// M: [VzW] Data Framework @}

/// M: CC: Normal/Emergency call redial @{
/* Used by:
 *   RIL_LOCAL_REQUEST_EMERGENCY_REDIAL
 *   RIL_LOCAL_C2K_REQUEST_REDIAL
 *   RIL_LOCAL_C2K_REQUEST_EMERGENCY_REDIAL
 */
typedef struct {
    char *address;
    int clir;
            /* (same as 'n' paremeter in TS 27.007 7.7 "+CLIR"
             * clir == 0 on "use subscription default value"
             * clir == 1 on "CLIR invocation" (restrict CLI presentation)
             * clir == 2 on "CLIR suppression" (allow CLI presentation)
             */
    RIL_UUS_Info *uusInfo;      /* NULL or Pointer to User-User Signaling Information */
    int mode;
    int callId;
} RIL_Redial;
/// @}

#ifdef C2K_RIL

typedef struct {
    char * register_state;
    char *lac;
    char * cid;
    char * radio_technology;
    char * basestation_id;
    char * basestation_latitude;
    char * basestation_longitude;
    char * concurrent_service;
    char * system_id;
    char * network_id;
    char * roaming_indicator;
    char * prl_state;
    char * prl_roaming_indicator;
    char * deny_reason;
    char * psc;  // 14
    char * network_exist;  // 15
    //char * service_state;    // only for via-telecom modem
} RIL_Registration_state;

typedef struct {
    RIL_CardState card_state;
    RIL_PinState  universal_pin_state;             /* applicable to USIM and CSIM: RIL_PINSTATE_xxx */
    int           pin1Count;
    int           pin2Count;
    int           puk1Count;
    int           puk2Count;
    int           gsm_umts_subscription_app_index; /* value < RIL_CARD_MAX_APPS */
    int           cdma_subscription_app_index;     /* value < RIL_CARD_MAX_APPS */
    int           num_applications;                /* value <= RIL_CARD_MAX_APPS */
    RIL_AppStatus applications[RIL_CARD_MAX_APPS];
    int           physicalSlotId;                  /* Add from android radio hidl interface v1.2*/
    char          *atr;                            /* Add from android radio hidl interface v1.2*/
    char          *iccId;                          /* Add from android radio hidl interface v1.2*/
    char          *eid;                            /* Add from android radio hidl interface v1.4*/
} VIA_RIL_CardStatus;

typedef struct {
    char *imei;
    char *imeisv;
    char *esnHex;
    char *meidHex;
    char *uimid;
    char *esnDec;
    char *meidDec;
} RIL_Identity;

typedef struct {
    char *mcc;
    char *mnc;
} RIL_Mccmnc;

typedef enum {
    GSMPHONE,
    IMSPHONE
} DIAL_SOURCE;

#endif /* C2K_RIL */

// M: [C2K] IRAT feature start
typedef struct {
    int sourceRat;  // Source RAT
    int targetRat;  // Target RAT
    int action;  // IRAT action
    int type;  // IRAT type, from source RAT to target RAT
} RIL_Pdn_IratInfo;
// M: [C2K] IRAT feature end

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif

// SMS-START
typedef struct {
    int used;
    int total;
} RIL_SMS_Memory_Status;

typedef struct {
    int format;
    int vp;
    int pid;
    int dcs;
} RIL_SmsParams;

typedef struct {
    int mode;
    char *channelConfigInfo;
    char *languageConfigInfo;
    int isAllLanguageOn;
} RIL_CBConfigInfo;

typedef struct {
    int warningType;
    int messageId;
    int serialNumber;
    char *plmnId;
    char *securityInfo;
} RIL_CBEtwsNotification;
// SMS-END

// PHB START
typedef enum {
    RIL_PHB_ADN = 0,
    RIL_PHB_FDN = 1,
    RIL_PHB_MSISDN = 2,
    RIL_PHB_ECC = 3
} RIL_PhbStorageType;

typedef struct {
    int type;                  /* type of the entry, refer to RIL_PhbStorageType */
    int index;                 /* the stored index of the entry */
    char *number;              /* the phone number */
    int ton;                   /* type of the number */
    char * alphaId;            /* the alpha ID, using Hexdecimal coding method */
} RIL_PhbEntryStructure;

typedef struct {
    char * storage;
    int used;
    int total;
} RIL_PHB_MEM_STORAGE_RESPONSE;

typedef struct {
    int index;          // values in the range of location numbers of phonebook memory
    char * number;      // phone number of format <type>
    int type;           // type of address octet in integer format; default 145 when number include "+", otherwise 129
    char * text;        // associated with the number
    int hidden;         // 0
    char * group;       // indicating a group the entry may belong to
    char * adnumber;    // an additional number of format<adtype>
    int adtype;         //
    char * secondtext;  // a second text field associated with the number
    char * email;       // email field
} RIL_PHB_ENTRY;
// PHB END

// AGPS APN
typedef enum {
    REQUEST_DATA_CONNECTION = 0,
    CLOSE_DATA_CONNECTION,
    GPS_START,
    GPS_FIX_RESULT,
    GPS_STOP,
} VIA_GPS_Event;

typedef struct {
    VIA_GPS_Event event;
    int gps_status;
} VIA_GPS_EVENT_Data;

// External SIM [Start]
typedef struct VsimMessage {
    uint32_t transaction_id;
    uint32_t message_id;
    uint32_t slotId;
    uint32_t length;
} VsimMessage;

typedef struct RIL_VsimEvent {
    uint32_t transaction_id;
    uint32_t eventId;
    uint32_t sim_type; //0: local sim, 1: remote sim
} RIL_VsimEvent;

typedef struct RIL_VsimOperationEvent {
    int32_t transaction_id;
    int32_t eventId; //0: ATR, 1: APDU
    int32_t result; // 0: successful, < 0: error cause
    int32_t data_length;
    char *data;  // It is ATR in case of reset response, APDU incase of APDU request
} RIL_VsimOperationEvent;

typedef struct {
    char * useraddr;
    char * end_point;
    char * entity;
    char * display_text;
    char * status;
} RIL_Conference_Participants;

typedef struct RIL_ImsPendingInd {
    int32_t slot;
    int32_t request;
    int32_t responseType;
    void* data;
    int data_size;
    struct RIL_ImsPendingInd *next;
} RIL_ImsPendingInd;

typedef struct {           // Default -1 for int and null for char*
    int opId;              // Operator ID
    int requestId;         // Request function ID
    int requestType;       // Reques type
    char* data;            // In hex string format ([a-fA-F0-9]*)
    int reserveInt1;       // RFU
    int reserveInt2;       // RFU
    char* reserveString1;  // RFU
} RIL_RsuRequest;

typedef struct {           // Default -1 for int and null for char*
    int opId;              // Operator ID
    int requestId;         // Request function ID
    int errCode;           // Error code
    char* data;            // In hex string format ([a-fA-F0-9]*)
    long long time;        // Currently used to indicate temporary unlock time remaining (s)
    int version;           // 2 x MSB for major and 2 x LSB for minor
    int status;            // Lock status
    int reserveInt1;       // RFU
    int reserveInt2;       // RFU
    char* reserveString1;  // RFU
} RIL_RsuResponse;

enum MessageId {
    MSG_ID_INITIALIZATION = 1,
    MSG_ID_GET_PLATFORM_CAPABILITY = 2,
    MSG_ID_EVENT_REQUEST = 3,
    MSG_ID_UICC_RESET_RESPONSE = 4,
    MSG_ID_UICC_APDU_RESPONSE = 5,
    MSG_ID_UICC_POWER_DOWN_RESPONSE = 6,
    MSG_ID_GET_SERVICE_STATE_REQUEST = 7,

    MSG_ID_INITIALIZATION_RESPONSE = 1001,
    MSG_ID_GET_PLATFORM_CAPABILITY_RESPONSE = 1002,
    MSG_ID_EVENT_RESPONSE = 1003,
    MSG_ID_UICC_RESET_REQUEST = 1004,
    MSG_ID_UICC_APDU_REQUEST = 1005,
    MSG_ID_UICC_POWER_DOWN_REQUEST = 1006,
    MSG_ID_GET_SERVICE_STATE_RESPONSE = 1007,

    MSG_ID_UICC_AUTHENTICATION_DONE_IND = 1009,
    MSG_ID_UICC_AUTHENTICATION_ABORT_IND = 1010,

    // Internal message id (not be aware of vsim adatpor)
    MSG_ID_UICC_AUTHENTICATION_REQUEST_IND = 2001,

    MSG_ID_UICC_TEST_MODE_REQUEST           = 5001,
};

enum EventTypeId {
    REQUEST_TYPE_ENABLE_EXTERNAL_SIM = 1,
    REQUEST_TYPE_DISABLE_EXTERNAL_SIM = 2,
    REQUEST_TYPE_PLUG_OUT = 3,
    REQUEST_TYPE_PLUG_IN = 4,
    REQUEST_TYPE_SET_PERSIST_TYPE = 5,  // no-used
    REQUEST_TYPE_SET_MAPPING_INFO = 6,
    REQUEST_TYPE_RESET_MAPPING_INFO = 7,
    REQUEST_TYPE_SET_TIMEOUT_TIMER = 8,
    REQUEST_TYPE_SET_PERSIST_TIMEOUT = 11,
    EVENT_TYPE_SEND_RSIM_AUTH_IND = 201,    // internal event id
    EVENT_TYPE_RECEIVE_RSIM_AUTH_RSP = 202, // internal event id
    EVENT_TYPE_RSIM_AUTH_DONE = 203,        // internal event id
    EVENT_TYPE_EXTERNAL_SIM_CONNECTED = 204, // internal event id

    REQUEST_TYPE_ATR_EVENT = 1,
    REQUEST_TYPE_APDU_EVENT = 2,
    REQUEST_TYPE_CARD_POWER_DOWN = 3,
    REQUEST_TYPE_AUTH_IND_EVENT = 4,
    REQUEST_TYPE_AUTH_ABORT_EVENT = 5,
    REQUEST_TYPE_AUTH_DONE_EVENT = 6,
    REQUEST_TYPE_TEST_MODE = 7,
};

enum SimType {
    SIM_TYPE_LOCAL_SIM = 1,
    SIM_TYPE_REMOTE_SIM = 2,
};
// External SIM [End]

//Dialog Event Package Info [Start]
typedef struct RIL_DialogInfo {
    int dialogId;
    int callState;
    int callType;
    bool isPullable;
    bool isCallHeld;
    bool isMt;
    char * address;
    char * remoteAddress;
} RIL_DialogInfo;
//Dialog Event Package Info [end]

typedef struct {
    int hysteresisMs;
    int hysteresisDlKbps;
    int hysteresisUlKbps;
    int thresholdDlKbpsNumber;
    int thresholdDlKbpsList[MAX_LCE_THRESHOLD_NUMBER];
    int thresholdUlKbpsNumber;
    int thresholdUlKbpsList[MAX_LCE_THRESHOLD_NUMBER];
    int accessNetwork;
} RIL_LinkCapacityReportingCriteria;

// ESIM - Start
typedef enum {
    INACTIVE  = 0, // Physical slot is inactive
    ACTIVE    = 1, // Physical slot is active
} RIL_SlotState;

typedef struct {
  RIL_CardState  card_state; // Card state in the physical slot
  RIL_SlotState  slotState; // Slot state Active/Inactive

  char           *atr;
  uint32_t       logicalSlotId;
  char           *iccId;

  char           *eid;  // For HIDL radio_config 1.2
} RIL_SimSlotStatus;
// ESIM - End

typedef struct {
    int uid;
    char* cert;
    int certLength;
    char* msg;
    int msgLength;
} RIL_CertMsg;

typedef struct {
    int uid;
    char* msg;
    int msgLength;
} RIL_AuthMsg;

typedef struct {
    char* id;
    int uid;
    int toActive;
} RIL_Capability;

typedef struct {
    int error;
    char* rnd;
    int rndLength;
    int custId;
} RIL_CertResponse;

typedef struct {
    int error;
    char* devId;
    int devIdLength;
    int capMask;
} RIL_AuthResponse;

typedef struct {
    int id;
    char* name;
} RIL_QueryCap;

typedef enum {
    IPV4_TCP = 0,
    IPV4_UDP = 1,
    IPV6_TCP = 2,
    IPV6_UDP = 3
} RIL_PacketType;

#define INVALID_ID -1

/*********************************************************************************/
/*  Vendor request                                                               */
/*********************************************************************************/
#define RIL_REQUEST_VENDOR_BASE 2000

#define RIL_REQUEST_RESUME_REGISTRATION (RIL_REQUEST_VENDOR_BASE + 0)
// MTK-START: SIM
// "data" is NULL *
// "response" is a const char * containing ATR in hexadecimal format
#define RIL_REQUEST_SIM_GET_ATR (RIL_REQUEST_VENDOR_BASE + 1)
#define RIL_REQUEST_SET_SIM_POWER (RIL_REQUEST_VENDOR_BASE + 2)
// MTK-END

// modem power
#define RIL_REQUEST_MODEM_POWERON (RIL_REQUEST_VENDOR_BASE + 3)
#define RIL_REQUEST_MODEM_POWEROFF (RIL_REQUEST_VENDOR_BASE + 4)

#define RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL_WITH_ACT (RIL_REQUEST_VENDOR_BASE + 5)
#define RIL_REQUEST_QUERY_AVAILABLE_NETWORKS_WITH_ACT (RIL_REQUEST_VENDOR_BASE + 6)
#define RIL_REQUEST_ABORT_QUERY_AVAILABLE_NETWORKS (RIL_REQUEST_VENDOR_BASE + 7)

// ATCI
#define RIL_REQUEST_OEM_HOOK_ATCI_INTERNAL (RIL_REQUEST_VENDOR_BASE + 8)
// M: To set language configuration for GSM cell broadcast
#define RIL_REQUEST_GSM_SET_BROADCAST_LANGUAGE (RIL_REQUEST_VENDOR_BASE + 9)
// M: To get language configuration for GSM cell broadcast
#define RIL_REQUEST_GSM_GET_BROADCAST_LANGUAGE (RIL_REQUEST_VENDOR_BASE + 10)
/**
 * RIL_REQUEST_GET_SMS_SIM_MEM_STATUS
 *
  * GET SMS SIM Card Memory's total and used number.
  *
  * "data" is const RIL_SMS_Memory_Status, contain the total and used SIM Card SMS number.
  * "response" is NULL.
  *
  * Valid errors:
  *  SUCCESS
  *  RADIO_NOT_AVAILABLE
  *  GENERIC_FAILURE
  */
#define RIL_REQUEST_GET_SMS_SIM_MEM_STATUS   (RIL_REQUEST_VENDOR_BASE + 11)
/*
* Get sms revelant parameters from EFsmsp
*/
#define RIL_REQUEST_GET_SMS_PARAMS (RIL_REQUEST_VENDOR_BASE + 12)

/*
* Set sms revelant parameters into EFsmsp
*/
#define RIL_REQUEST_SET_SMS_PARAMS (RIL_REQUEST_VENDOR_BASE + 13)
#define RIL_REQUEST_SET_ETWS (RIL_REQUEST_VENDOR_BASE + 14)
#define RIL_REQUEST_REMOVE_CB_MESSAGE (RIL_REQUEST_VENDOR_BASE + 15)

/// M: CC: proprietary incoming call indication
#define RIL_REQUEST_SET_CALL_INDICATION (RIL_REQUEST_VENDOR_BASE + 16)
/// M: CC: deprecated: Proprietary ECC handling @{
//#define RIL_REQUEST_EMERGENCY_DIAL (RIL_REQUEST_VENDOR_BASE + 17)
//#define RIL_REQUEST_SET_ECC_SERVICE_CATEGORY (RIL_REQUEST_VENDOR_BASE + 18)
/// @}
/// M: CC: proprietary call control hangup all
#define RIL_REQUEST_HANGUP_ALL (RIL_REQUEST_VENDOR_BASE + 19)
#define RIL_REQUEST_SET_PS_REGISTRATION (RIL_REQUEST_VENDOR_BASE + 20)
/// M: APC. @{
#define RIL_REQUEST_SET_PSEUDO_CELL_MODE (RIL_REQUEST_VENDOR_BASE + 21)
#define RIL_REQUEST_GET_PSEUDO_CELL_INFO (RIL_REQUEST_VENDOR_BASE + 22)
/// @}

#define RIL_REQUEST_SWITCH_MODE_FOR_ECC (RIL_REQUEST_VENDOR_BASE + 23)
#define RIL_REQUEST_GET_SMS_RUIM_MEM_STATUS (RIL_REQUEST_VENDOR_BASE + 24)

// FastDormancy
#define RIL_REQUEST_SET_FD_MODE (RIL_REQUEST_VENDOR_BASE + 25)

// World Phone
#define RIL_REQUEST_SET_TRM (RIL_REQUEST_VENDOR_BASE + 28)
/**
 * RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM_WITH_RESULT_CODE
 *
 * When STK application gets RIL_UNSOL_STK_CALL_SETUP, the call actually has
 * been initialized by ME already. (We could see the call has been in the 'call
 * list') So, STK application needs to accept/reject the call according as user
 * operations.
 *
 * "data" is int *
 * ((int *)data)[0] is == 1 for "accept" the call setup
 * ((int *)data)[0] is == 0 for "reject" the call setup
 * ((int *)data)[0] is == 0x20 for TERMINAL_CRNTLY_UNABLE_TO_PROCESS
 * ((int *)data)[0] is == 0x21for NETWORK_CRNTLY_UNABLE_TO_PROCESS
 * "response" is NULL
 *
 * Valid errors:
 *  RIL_E_SUCCESS
 *  RIL_E_RADIO_NOT_AVAILABLE (radio resetting)
 *  RIL_E_OPERATION_NOT_ALLOWED
 *  RIL_E_GENERIC_FAILURE
 */
#define RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM_WITH_RESULT_CODE  (RIL_REQUEST_VENDOR_BASE + 29)

#define RIL_REQUEST_SET_ECC_LIST (RIL_REQUEST_VENDOR_BASE + 30)

/// M: CC: deprecated - Call control framework 3G VT - Accept voice only VT call
#define RIL_REQUEST_VOICE_ACCEPT (RIL_REQUEST_VENDOR_BASE + 31)
/// M: CC: deprecated - Call control framework 3G VT - vt dial
#define RIL_REQUEST_VT_DIAL (RIL_REQUEST_VENDOR_BASE + 32)
/// M: CC: deprecated - Call control framework 3G VT - replace VT call
#define RIL_REQUEST_REPLACE_VT_CALL (RIL_REQUEST_VENDOR_BASE + 33)

/// m: CC: Call control force release call
#define RIL_REQUEST_FORCE_RELEASE_CALL (RIL_REQUEST_VENDOR_BASE + 34)
/// M: CC: Emergency mode for Fusion RIL
#define RIL_REQUEST_SET_ECC_MODE (RIL_REQUEST_VENDOR_BASE + 35)

// PHB START
#define RIL_REQUEST_QUERY_PHB_STORAGE_INFO (RIL_REQUEST_VENDOR_BASE + 36)
#define RIL_REQUEST_WRITE_PHB_ENTRY (RIL_REQUEST_VENDOR_BASE + 37)
#define RIL_REQUEST_READ_PHB_ENTRY (RIL_REQUEST_VENDOR_BASE + 38)
#define RIL_REQUEST_QUERY_UPB_CAPABILITY (RIL_REQUEST_VENDOR_BASE + 39)
#define RIL_REQUEST_EDIT_UPB_ENTRY (RIL_REQUEST_VENDOR_BASE + 40)
#define RIL_REQUEST_DELETE_UPB_ENTRY (RIL_REQUEST_VENDOR_BASE + 41)
#define RIL_REQUEST_READ_UPB_GAS_LIST (RIL_REQUEST_VENDOR_BASE + 42)
#define RIL_REQUEST_READ_UPB_GRP  (RIL_REQUEST_VENDOR_BASE + 43)
#define RIL_REQUEST_WRITE_UPB_GRP  (RIL_REQUEST_VENDOR_BASE + 44)
#define RIL_REQUEST_GET_PHB_STRING_LENGTH (RIL_REQUEST_VENDOR_BASE + 45)
#define RIL_REQUEST_GET_PHB_MEM_STORAGE (RIL_REQUEST_VENDOR_BASE + 46)
#define RIL_REQUEST_SET_PHB_MEM_STORAGE (RIL_REQUEST_VENDOR_BASE + 47)
#define RIL_REQUEST_READ_PHB_ENTRY_EXT (RIL_REQUEST_VENDOR_BASE + 48)
#define RIL_REQUEST_WRITE_PHB_ENTRY_EXT (RIL_REQUEST_VENDOR_BASE + 49)
#define RIL_REQUEST_QUERY_UPB_AVAILABLE (RIL_REQUEST_VENDOR_BASE + 50)
#define RIL_REQUEST_READ_EMAIL_ENTRY (RIL_REQUEST_VENDOR_BASE + 51)
#define RIL_REQUEST_READ_SNE_ENTRY (RIL_REQUEST_VENDOR_BASE + 52)
#define RIL_REQUEST_READ_ANR_ENTRY (RIL_REQUEST_VENDOR_BASE + 53)
#define RIL_REQUEST_READ_UPB_AAS_LIST (RIL_REQUEST_VENDOR_BASE + 54)
// PHB END

// Femtocell feature
#define RIL_REQUEST_GET_FEMTOCELL_LIST (RIL_REQUEST_VENDOR_BASE + 55)
#define RIL_REQUEST_ABORT_FEMTOCELL_LIST (RIL_REQUEST_VENDOR_BASE + 56)
#define RIL_REQUEST_SELECT_FEMTOCELL (RIL_REQUEST_VENDOR_BASE + 57)
/**
 * RIL_REQUEST_QUERY_FEMTOCELL_SYSTEM_SELECTION_MODE
 * "data" is NULL
 * "response" is int *
 * ((int *)data)[0] is the femtocell selection mode as defined in AT+EFSS
 */
#define RIL_REQUEST_QUERY_FEMTOCELL_SYSTEM_SELECTION_MODE (RIL_REQUEST_VENDOR_BASE + 58)
/**
 * RIL_REQUEST_SET_FEMTOCELL_SYSTEM_SELECTION_MODE
 * "data" is int *
 * ((int *)data)[0] is the femtocell selection mode as defined in AT+EFSS
 * "response" is NULL
 */
#define RIL_REQUEST_SET_FEMTOCELL_SYSTEM_SELECTION_MODE (RIL_REQUEST_VENDOR_BASE + 59)

/// M: eMBMS feature
#define RIL_REQUEST_EMBMS_AT_CMD (RIL_REQUEST_VENDOR_BASE + 60)
/// M: eMBMS end

#define RIL_REQUEST_SYNC_DATA_SETTINGS_TO_MD (RIL_REQUEST_VENDOR_BASE + 62)
#define RIL_REQUEST_RESET_MD_DATA_RETRY_COUNT (RIL_REQUEST_VENDOR_BASE + 63)
// MTK-END
// MTK-START: SIM GBA / AUTH
#define RIL_REQUEST_GENERAL_SIM_AUTH (RIL_REQUEST_VENDOR_BASE + 64)
// MTK-END
// M: [LTE][Low Power][UL traffic shaping] @{
#define RIL_REQUEST_SET_LTE_ACCESS_STRATUM_REPORT (RIL_REQUEST_VENDOR_BASE + 65)
#define RIL_REQUEST_SET_LTE_UPLINK_DATA_TRANSFER (RIL_REQUEST_VENDOR_BASE + 66)
// M: [LTE][Low Power][UL traffic shaping] @}

// MTK-START: SIM ME LOCK
#define RIL_REQUEST_QUERY_SIM_NETWORK_LOCK (RIL_REQUEST_VENDOR_BASE + 67)
#define RIL_REQUEST_SET_SIM_NETWORK_LOCK (RIL_REQUEST_VENDOR_BASE + 68)
// IMS blind/assured ECT

/// [IMS] IMS RIL_REQUEST ==============================================================
#define RIL_REQUEST_SET_IMS_ENABLE (RIL_REQUEST_VENDOR_BASE + 69)
#define RIL_REQUEST_SET_VOLTE_ENABLE (RIL_REQUEST_VENDOR_BASE + 70)
#define RIL_REQUEST_SET_WFC_ENABLE (RIL_REQUEST_VENDOR_BASE + 71)
#define RIL_REQUEST_SET_VILTE_ENABLE (RIL_REQUEST_VENDOR_BASE + 72)
#define RIL_REQUEST_SET_VIWIFI_ENABLE (RIL_REQUEST_VENDOR_BASE + 73)
#define RIL_REQUEST_SET_IMS_VIDEO_ENABLE (RIL_REQUEST_VENDOR_BASE + 75)
#define RIL_REQUEST_VIDEO_CALL_ACCEPT (RIL_REQUEST_VENDOR_BASE + 76)
#define RIL_REQUEST_SET_IMSCFG (RIL_REQUEST_VENDOR_BASE + 77)
/// [IMS] IMS Provision Configs
#define RIL_REQUEST_GET_PROVISION_VALUE (RIL_REQUEST_VENDOR_BASE + 78)
#define RIL_REQUEST_SET_PROVISION_VALUE (RIL_REQUEST_VENDOR_BASE + 79)
/// [IMS] IMS Bearer Activate/Deactivate
#define RIL_REQUEST_IMS_BEARER_STATE_CONFIRM (RIL_REQUEST_VENDOR_BASE + 80)
#define RIL_REQUEST_IMS_DEREG_NOTIFICATION (RIL_REQUEST_VENDOR_BASE + 82)

#define RIL_REQUEST_IMS_ECT (RIL_REQUEST_VENDOR_BASE + 83)

// [IMS] IMS Call
#define RIL_REQUEST_HOLD_CALL (RIL_REQUEST_VENDOR_BASE + 84)
#define RIL_REQUEST_RESUME_CALL (RIL_REQUEST_VENDOR_BASE + 85)
#define RIL_REQUEST_DIAL_WITH_SIP_URI (RIL_REQUEST_VENDOR_BASE + 86)
// [IMS] Emergency Dial
#define RIL_REQUEST_IMS_EMERGENCY_DIAL (RIL_REQUEST_VENDOR_BASE + 87)

#define RIL_REQUEST_SET_IMS_RTP_REPORT (RIL_REQUEST_VENDOR_BASE + 88)

// [IMS] IMS Conference Call
#define RIL_REQUEST_CONFERENCE_DIAL (RIL_REQUEST_VENDOR_BASE + 89)
#define RIL_REQUEST_ADD_IMS_CONFERENCE_CALL_MEMBER (RIL_REQUEST_VENDOR_BASE + 90)
#define RIL_REQUEST_REMOVE_IMS_CONFERENCE_CALL_MEMBER (RIL_REQUEST_VENDOR_BASE + 91)
// [IMS] IMS Video Call
#define RIL_REQUEST_VT_DIAL_WITH_SIP_URI (RIL_REQUEST_VENDOR_BASE + 92)
// [IMS] USSI
#define RIL_REQUEST_SEND_USSI (RIL_REQUEST_VENDOR_BASE + 93)
#define RIL_REQUEST_CANCEL_USSI (RIL_REQUEST_VENDOR_BASE + 94)
// [IMS] WFC
#define RIL_REQUEST_SET_WFC_PROFILE (RIL_REQUEST_VENDOR_BASE + 95)
// [IMS] Pull Call
#define RIL_REQUEST_PULL_CALL (RIL_REQUEST_VENDOR_BASE + 96)
// [IMS] Registration Report
#define RIL_REQUEST_SET_IMS_REGISTRATION_REPORT (RIL_REQUEST_VENDOR_BASE + 97)
// [IMS] Dial
#define RIL_REQUEST_IMS_DIAL (RIL_REQUEST_VENDOR_BASE + 98)
// [IMS] VT Dial
#define RIL_REQUEST_IMS_VT_DIAL (RIL_REQUEST_VENDOR_BASE + 99)
// [IMS] Emergency Dial
/// [IMS] IMS RIL_REQUEST ==============================================================
// MTK-END
#define RIL_REQUEST_SET_REMOVE_RESTRICT_EUTRAN_MODE (RIL_REQUEST_VENDOR_BASE + 100)
// MTK_TC1_FEATURE for Antenna Testing start
#define RIL_REQUEST_VSS_ANTENNA_CONF (RIL_REQUEST_VENDOR_BASE + 101)
#define RIL_REQUEST_VSS_ANTENNA_INFO (RIL_REQUEST_VENDOR_BASE + 102)
// MTK_TC1_FEATURE for Antenna Testing end
// SS
#define RIL_REQUEST_SET_CLIP  (RIL_REQUEST_VENDOR_BASE + 103)
#define RIL_REQUEST_GET_COLP  (RIL_REQUEST_VENDOR_BASE + 104)
#define RIL_REQUEST_GET_COLR  (RIL_REQUEST_VENDOR_BASE + 105)
#define RIL_REQUEST_SEND_CNAP (RIL_REQUEST_VENDOR_BASE + 106)

// Preferred Operator List
#define RIL_REQUEST_GET_POL_CAPABILITY (RIL_REQUEST_VENDOR_BASE + 107)
#define RIL_REQUEST_GET_POL_LIST (RIL_REQUEST_VENDOR_BASE + 108)
#define RIL_REQUEST_SET_POL_ENTRY (RIL_REQUEST_VENDOR_BASE + 109)

/// M: CC: Vzw/CTVolte ECC for Fusion RIL
#define RIL_REQUEST_ECC_PREFERRED_RAT (RIL_REQUEST_VENDOR_BASE + 110)

/// M: [Network][C2K] Sprint roaming control @{
#define RIL_REQUEST_SET_ROAMING_ENABLE (RIL_REQUEST_VENDOR_BASE + 111)
#define RIL_REQUEST_GET_ROAMING_ENABLE (RIL_REQUEST_VENDOR_BASE + 112)
/// @}

/// M: IMS SS @{
#define RIL_REQUEST_SET_COLP (RIL_REQUEST_VENDOR_BASE + 113)
#define RIL_REQUEST_SET_COLR (RIL_REQUEST_VENDOR_BASE + 114)
#define RIL_REQUEST_QUERY_CALL_FORWARD_IN_TIME_SLOT (RIL_REQUEST_VENDOR_BASE + 115)
#define RIL_REQUEST_SET_CALL_FORWARD_IN_TIME_SLOT (RIL_REQUEST_VENDOR_BASE + 116)
#define RIL_REQUEST_RUN_GBA (RIL_REQUEST_VENDOR_BASE + 117)
/// @}

// External SIM [START]
#define RIL_REQUEST_VSIM_NOTIFICATION (RIL_REQUEST_VENDOR_BASE + 118)
#define RIL_REQUEST_VSIM_OPERATION (RIL_REQUEST_VENDOR_BASE + 119)
// External SIM [END]
#define RIL_REQUEST_GET_GSM_SMS_BROADCAST_ACTIVATION (RIL_REQUEST_VENDOR_BASE + 120)

#define RIL_REQUEST_SET_VOICE_DOMAIN_PREFERENCE (RIL_REQUEST_VENDOR_BASE + 121)
#define RIL_REQUEST_SET_MD_IMSCFG (RIL_REQUEST_VENDOR_BASE + 122)
#define RIL_REQUEST_SET_E911_STATE (RIL_REQUEST_VENDOR_BASE + 123)
/// M: MwiService @{
#define RIL_REQUEST_SET_WIFI_ENABLED (RIL_REQUEST_VENDOR_BASE + 124)
#define RIL_REQUEST_SET_WIFI_ASSOCIATED (RIL_REQUEST_VENDOR_BASE + 125)
#define RIL_REQUEST_SET_WIFI_SIGNAL_LEVEL (RIL_REQUEST_VENDOR_BASE + 126)
#define RIL_REQUEST_SET_WIFI_IP_ADDRESS (RIL_REQUEST_VENDOR_BASE + 127)
#define RIL_REQUEST_SET_GEO_LOCATION (RIL_REQUEST_VENDOR_BASE + 128)
#define RIL_REQUEST_SET_EMERGENCY_ADDRESS_ID (RIL_REQUEST_VENDOR_BASE + 129)
/// @}
#define RIL_REQUEST_SET_SERVICE_STATE (RIL_REQUEST_VENDOR_BASE + 130)
/// M: MwiService - NATT Keep Alive
#define RIL_REQUEST_SET_NATT_KEEP_ALIVE_STATUS (RIL_REQUEST_VENDOR_BASE + 131)
/// M: WfoService - Backhaul strength.
#define RIL_REQUEST_SET_WIFI_PING_RESULT (RIL_REQUEST_VENDOR_BASE + 132)

#define RIL_REQUEST_IMS_SEND_SMS_EX (RIL_REQUEST_VENDOR_BASE + 133)

#define RIL_REQUEST_SET_SMS_FWK_READY (RIL_REQUEST_VENDOR_BASE + 134)

#define RIL_REQUEST_SET_IMS_BEARER_NOTIFICATION (RIL_REQUEST_VENDOR_BASE + 135)
// [IMS] Telephonyware IMS Config Request
#define RIL_REQUEST_IMS_CONFIG_SET_FEATURE (RIL_REQUEST_VENDOR_BASE + 136)
#define RIL_REQUEST_IMS_CONFIG_GET_FEATURE (RIL_REQUEST_VENDOR_BASE + 137)
#define RIL_REQUEST_IMS_CONFIG_SET_PROVISION (RIL_REQUEST_VENDOR_BASE + 138)
#define RIL_REQUEST_IMS_CONFIG_GET_PROVISION (RIL_REQUEST_VENDOR_BASE + 139)
#define RIL_REQUEST_IMS_CONFIG_GET_RESOURCE_CAP (RIL_REQUEST_VENDOR_BASE + 141)

// MTK-START: SIM
#define RIL_REQUEST_SIM_GET_ICCID (RIL_REQUEST_VENDOR_BASE + 142)
#define RIL_REQUEST_ENTER_DEPERSONALIZATION (RIL_REQUEST_VENDOR_BASE + 143)
// MTK-END

// PS/CS attach
#define RIL_REQUEST_DATA_CONNECTION_ATTACH (RIL_REQUEST_VENDOR_BASE + 144)
// PS/CS detach
#define RIL_REQUEST_DATA_CONNECTION_DETACH (RIL_REQUEST_VENDOR_BASE + 145)
// Cleanup all connections
#define RIL_REQUEST_RESET_ALL_CONNECTIONS (RIL_REQUEST_VENDOR_BASE + 146)

#define RIL_REQUEST_SET_VOICE_PREFER_STATUS (RIL_REQUEST_VENDOR_BASE + 147)
#define RIL_REQUEST_SET_ECC_NUM (RIL_REQUEST_VENDOR_BASE + 148)
#define RIL_REQUEST_GET_ECC_NUM (RIL_REQUEST_VENDOR_BASE + 149)
#define RIL_REQUEST_RESTART_RILD (RIL_REQUEST_VENDOR_BASE + 150)
#define RIL_REQUEST_SET_LTE_RELEASE_VERSION (RIL_REQUEST_VENDOR_BASE + 151)
#define RIL_REQUEST_GET_LTE_RELEASE_VERSION (RIL_REQUEST_VENDOR_BASE + 152)
#define RIL_REQUEST_SIGNAL_STRENGTH_WITH_WCDMA_ECIO (RIL_REQUEST_VENDOR_BASE + 153)
#define RIL_REQUEST_SET_PHONEBOOK_READY (RIL_REQUEST_VENDOR_BASE + 157)
#define RIL_REQUEST_SET_TX_POWER_STATUS (RIL_REQUEST_VENDOR_BASE + 158)
#define RIL_REQUEST_SETPROP_IMS_HANDOVER (RIL_REQUEST_VENDOR_BASE + 159)
#define RIL_REQUEST_SET_PDN_REUSE (RIL_REQUEST_VENDOR_BASE + 160)
#define RIL_REQUEST_SET_OVERRIDE_APN  (RIL_REQUEST_VENDOR_BASE + 161)
#define RIL_REQUEST_SET_PDN_NAME_REUSE  (RIL_REQUEST_VENDOR_BASE + 162)
#define RIL_REQUEST_GET_XCAP_STATUS (RIL_REQUEST_VENDOR_BASE + 163)
#define RIL_REQUEST_RESET_SUPP_SERV (RIL_REQUEST_VENDOR_BASE + 164)
#define RIL_REQUEST_SET_RCS_UA_ENABLE (RIL_REQUEST_VENDOR_BASE + 166)
#define RIL_REQUEST_SETUP_XCAP_USER_AGENT_STRING (RIL_REQUEST_VENDOR_BASE + 167)
#define RIL_REQUEST_SET_SS_PROPERTY (RIL_REQUEST_VENDOR_BASE + 169)
#define RIL_REQUEST_SMS_ACKNOWLEDGE_EX (RIL_REQUEST_VENDOR_BASE + 171)
// MTK-START: SIM SLOT LOCK
#define RIL_REQUEST_ENTER_DEVICE_NETWORK_DEPERSONALIZATION  (RIL_REQUEST_VENDOR_BASE + 172)
// MTK-END
#define RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE_EX (RIL_REQUEST_VENDOR_BASE + 173)
///M: Notify ePDG screen state
#define RIL_REQUEST_NOTIFY_EPDG_SCREEN_STATE (RIL_REQUEST_VENDOR_BASE + 174)

/// M: SUBSIDYLOCK
#define RIL_REQUEST_UPDATE_SUBLOCK_SETTINGS (RIL_REQUEST_VENDOR_BASE + 175)
#define RIL_REQUEST_GET_SUBLOCK_MODEM_STATUS (RIL_REQUEST_VENDOR_BASE + 176)

#define RIL_REQUEST_HANGUP_WITH_REASON (RIL_REQUEST_VENDOR_BASE + 177)
#define RIL_REQUEST_IMS_GET_CURRENT_CALLS (RIL_REQUEST_VENDOR_BASE + 178)
#define RIL_REQUEST_IMS_HANGUP_WAITING_OR_BACKGROUND (RIL_REQUEST_VENDOR_BASE + 179)
#define RIL_REQUEST_IMS_HANGUP_FOREGROUND_RESUME_BACKGROUND (RIL_REQUEST_VENDOR_BASE + 180)
#define RIL_REQUEST_IMS_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE (RIL_REQUEST_VENDOR_BASE + 181)
// M: RTT @{
#define RIL_REQUEST_SET_RTT_MODE (RIL_REQUEST_VENDOR_BASE + 182)
#define RIL_REQUEST_SEND_RTT_MODIFY_REQUEST (RIL_REQUEST_VENDOR_BASE + 183)
#define RIL_REQUEST_SEND_RTT_TEXT (RIL_REQUEST_VENDOR_BASE + 184)
#define RIL_REQUEST_RTT_MODIFY_REQUST_RESPONSE (RIL_REQUEST_VENDOR_BASE + 185)
// @}

#define RIL_REQUEST_QUERY_VOPS_STATUS (RIL_REQUEST_VENDOR_BASE + 186)

#define RIL_REQUEST_GET_TS25_NAME (RIL_REQUEST_VENDOR_BASE + 187)
#define RIL_REQUEST_ENABLE_CA_PLUS_FILTER (RIL_REQUEST_VENDOR_BASE + 188)
// M: GWSD @{
#define RIL_REQUEST_SET_GWSD_MODE (RIL_REQUEST_VENDOR_BASE + 189)
#define RIL_REQUEST_SET_GWSD_CALL_VALID (RIL_REQUEST_VENDOR_BASE + 190)
#define RIL_REQUEST_SET_GWSD_IGNORE_CALL_INTERVAL (RIL_REQUEST_VENDOR_BASE + 191)
// @}
#define RIL_REQUEST_MODIFY_MODEM_TYPE (RIL_REQUEST_VENDOR_BASE + 192)
#define RIL_REQUEST_ENABLE_DSDA_INDICATION (RIL_REQUEST_VENDOR_BASE + 193)
#define RIL_REQUEST_GET_DSDA_STATUS (RIL_REQUEST_VENDOR_BASE + 194)
#define RIL_REQUEST_SET_SIP_HEADER (RIL_REQUEST_VENDOR_BASE + 195)
#define RIL_REQUEST_SIP_HEADER_REPORT (RIL_REQUEST_VENDOR_BASE + 196)
#define RIL_REQUEST_SET_IMS_CALL_MODE (RIL_REQUEST_VENDOR_BASE + 197)
#define RIL_REQUEST_GET_VOICE_DOMAIN_PREFERENCE (RIL_REQUEST_VENDOR_BASE + 198)
#define RIL_REQUEST_ACTIVATE_UICC_CARD (RIL_REQUEST_VENDOR_BASE + 199)
#define RIL_REQUEST_DEACTIVATE_UICC_CARD (RIL_REQUEST_VENDOR_BASE + 200)
#define RIL_REQUEST_GET_CURRENT_UICC_CARD_PROVISIONING_STATUS (RIL_REQUEST_VENDOR_BASE + 201)
#define RIL_REQUEST_SET_WFC_CONFIG (RIL_REQUEST_VENDOR_BASE + 202)

// MUSE WFC requirement
#define RIL_REQUEST_IWLAN_REGISTER_CELLULAR_QUALITY_REPORT (RIL_REQUEST_VENDOR_BASE + 203)
#define RIL_REQUEST_QUERY_SSAC_STATUS (RIL_REQUEST_VENDOR_BASE + 204)
#define RIL_REQUEST_GET_SUGGESTED_PLMN_LIST (RIL_REQUEST_VENDOR_BASE + 205)

// toggle RTT audio
#define RIL_REQUEST_TOGGLE_RTT_AUDIO_INDICATION (RIL_REQUEST_VENDOR_BASE + 206)

// NR request
#define RIL_REQUEST_CONFIG_A2_OFFSET (RIL_REQUEST_VENDOR_BASE + 207)
#define RIL_REQUEST_CONFIG_B1_OFFSET (RIL_REQUEST_VENDOR_BASE + 208)
#define RIL_REQUEST_ENABLE_SCG_FAILURE (RIL_REQUEST_VENDOR_BASE + 209)
#define RIL_REQUEST_DISABLE_NR (RIL_REQUEST_VENDOR_BASE + 210)
#define RIL_REQUEST_SET_TX_POWER (RIL_REQUEST_VENDOR_BASE + 211)
#define RIL_REQUEST_SEARCH_STORED_FREQUENCY_INFO (RIL_REQUEST_VENDOR_BASE + 212)
#define RIL_REQUEST_SEARCH_RAT (RIL_REQUEST_VENDOR_BASE + 213)
#define RIL_REQUEST_SET_BACKGROUND_SEARCH_TIMER (RIL_REQUEST_VENDOR_BASE + 214)

#define RIL_REQUEST_ROUTE_CERTIFICATE (RIL_REQUEST_VENDOR_BASE + 215)
#define RIL_REQUEST_ROUTE_AUTH (RIL_REQUEST_VENDOR_BASE + 216)
#define RIL_REQUEST_ENABLE_CAPABILITY (RIL_REQUEST_VENDOR_BASE + 217)
#define RIL_REQUEST_ABORT_CERTIFICATE (RIL_REQUEST_VENDOR_BASE + 218)

#define RIL_REQUEST_ECC_REDIAL_APPROVE (RIL_REQUEST_VENDOR_BASE + 219)

// M: GWSD @{
#define RIL_REQUEST_SET_GWSD_KEEP_ALIVE_PDCP (RIL_REQUEST_VENDOR_BASE + 220)
#define RIL_REQUEST_SET_GWSD_KEEP_ALIVE_IPDATA (RIL_REQUEST_VENDOR_BASE + 221)
// @}

#define RIL_REQUEST_DEACTIVATE_NR_SCG_COMMUNICACTION (RIL_REQUEST_VENDOR_BASE + 222)
#define RIL_REQUEST_GET_DEACTIVATE_NR_SCG_COMMUNICACTION (RIL_REQUEST_VENDOR_BASE + 223)

#define RIL_REQUEST_SET_MAX_UL_SPEED (RIL_REQUEST_VENDOR_BASE + 224)
#define RIL_REQUEST_SMART_RAT_SWITCH (RIL_REQUEST_VENDOR_BASE + 225)
#define RIL_REQUEST_GET_SMART_RAT_SWITCH (RIL_REQUEST_VENDOR_BASE + 226)
#define RIL_REQUEST_SET_SMART_RAT_SWITCH (RIL_REQUEST_VENDOR_BASE + 227)
/// M: send sar ind
#define RIL_REQUEST_SEND_SAR_IND (RIL_REQUEST_VENDOR_BASE + 228)
#define RIL_REQUEST_SET_CALL_ADDITIONAL_INFO (RIL_REQUEST_VENDOR_BASE + 229)
// Common RSU request
#define RIL_REQUEST_SML_RSU_REQUEST (RIL_REQUEST_VENDOR_BASE + 230)

  /*********************************************************************************/
/*  Vendor local request                                                         */
/*********************************************************************************/
#define RIL_LOCAL_REQUEST_VENDOR_BASE 2500

/// M: [C2K 6M][NW] Set the SVLTE RAT mode @{
#define RIL_LOCAL_REQUEST_SET_SVLTE_RAT_MODE (RIL_LOCAL_REQUEST_VENDOR_BASE + 0)

/// M: [C2K 6M][NW] Set the SVLTE RAT mode  @}
#define RIL_LOCAL_GSM_REQUEST_SWITCH_CARD_TYPE (RIL_LOCAL_REQUEST_VENDOR_BASE + 1)
/// M: [C2K]Dynamic switch support. @{
#define RIL_REQUEST_ENTER_RESTRICT_MODEM (RIL_LOCAL_REQUEST_VENDOR_BASE + 2)
#define RIL_REQUEST_LEAVE_RESTRICT_MODEM (RIL_LOCAL_REQUEST_VENDOR_BASE + 3)
/// @}
#define RIL_REQUEST_GSM_DEVICE_IDENTITY (RIL_LOCAL_REQUEST_VENDOR_BASE + 4)

#define RIL_REQUEST_SET_REG_SUSPEND_ENABLED (RIL_LOCAL_REQUEST_VENDOR_BASE + 5)

// C2K SVLTE remote SIM access
#define RIL_REQUEST_CONFIG_MODEM_STATUS (RIL_LOCAL_REQUEST_VENDOR_BASE + 6)

#define RIL_LOCAL_REQUEST_MODE_SWITCH_GSM_SET_TRM (RIL_LOCAL_REQUEST_VENDOR_BASE + 7)
// M: [C2K] MD IRAT feature
#define RIL_LOCAL_GSM_REQUEST_CONFIRM_INTER_3GPP_IRAT_CHANGE (RIL_LOCAL_REQUEST_VENDOR_BASE + 8)
#define RIL_LOCAL_GSM_REQUEST_SET_ACTIVE_PS_SLOT (RIL_LOCAL_REQUEST_VENDOR_BASE + 9)
#define RIL_LOCAL_REQUEST_QUERY_MODEM_THERMAL (RIL_LOCAL_REQUEST_VENDOR_BASE + 10)
/* Request modem on/off from thermal */
#define RIL_LOCAL_REQUEST_SET_MODEM_THERMAL (RIL_LOCAL_REQUEST_VENDOR_BASE + 11)

#define  RIL_REQUEST_SET_STK_UTK_MODE  (RIL_LOCAL_REQUEST_VENDOR_BASE + 12)
#define RIL_LOCAL_REQUEST_CDMA_SMS_SPECIFIC_TO_GSM (RIL_LOCAL_REQUEST_VENDOR_BASE + 13)

#define RIL_LOCAL_REQUEST_AT_COMMAND_WITH_PROXY (RIL_LOCAL_REQUEST_VENDOR_BASE + 14)
/// M: EPDG feature. Update PS state from MAL @{
#define RIL_LOCAL_REQUEST_MAL_PS_RGEGISTRATION_STATE (RIL_LOCAL_REQUEST_VENDOR_BASE + 15)
/// @}

/// M: Ims Data Framework {@
#define RIL_LOCAL_REQUEST_HANDOVER_IND (RIL_LOCAL_REQUEST_VENDOR_BASE + 16)
#define RIL_LOCAL_REQUEST_WIFI_DISCONNECT_IND (RIL_LOCAL_REQUEST_VENDOR_BASE + 17)
#define RIL_LOCAL_REQUEST_PCSCF_DISCOVERY_PCO (RIL_LOCAL_REQUEST_VENDOR_BASE + 18)
#define RIL_LOCAL_REQUEST_WIFI_CONNECT_IND (RIL_LOCAL_REQUEST_VENDOR_BASE + 19)
///@}
/// M: eMBMS feature
#define RIL_LOCAL_REQUEST_EMBMS_AT_CMD (RIL_LOCAL_REQUEST_VENDOR_BASE + 20)
#define RIL_LOCAL_REQUEST_EMBMS_ENABLE (RIL_LOCAL_REQUEST_VENDOR_BASE + 21)
#define RIL_LOCAL_REQUEST_EMBMS_DISABLE (RIL_LOCAL_REQUEST_VENDOR_BASE + 22)
#define RIL_LOCAL_REQUEST_EMBMS_START_SESSION (RIL_LOCAL_REQUEST_VENDOR_BASE + 23)
#define RIL_LOCAL_REQUEST_EMBMS_STOP_SESSION (RIL_LOCAL_REQUEST_VENDOR_BASE + 24)
#define RIL_LOCAL_REQUEST_EMBMS_GET_TIME (RIL_LOCAL_REQUEST_VENDOR_BASE + 25)
#define RIL_LOCAL_REQUEST_EMBMS_GET_COVERAGE_STATE (RIL_LOCAL_REQUEST_VENDOR_BASE + 26)
#define RIL_LOCAL_REQUEST_EMBMS_SET_E911 (RIL_LOCAL_REQUEST_VENDOR_BASE + 27)
/// M: eMBMS end
// M: MPS feature
#define RIL_LOCAL_REQUEST_DEACTIVATE_ALL_DATA_CALL (RIL_LOCAL_REQUEST_VENDOR_BASE + 28)

// MTK-START: SIM TMO RSU
/* SIM ME Lock added request id */
#define RIL_LOCAL_REQUEST_GET_SHARED_KEY (RIL_LOCAL_REQUEST_VENDOR_BASE + 29)
#define RIL_LOCAL_REQUEST_UPDATE_SIM_LOCK_SETTINGS (RIL_LOCAL_REQUEST_VENDOR_BASE + 30)
#define RIL_LOCAL_REQUEST_GET_SIM_LOCK_INFO (RIL_LOCAL_REQUEST_VENDOR_BASE + 31)
#define RIL_LOCAL_REQUEST_RESET_SIM_LOCK_SETTINGS (RIL_LOCAL_REQUEST_VENDOR_BASE + 32)
#define RIL_LOCAL_REQUEST_GET_MODEM_STATUS (RIL_LOCAL_REQUEST_VENDOR_BASE + 33)
// MTK-END

//Ims data request from MAL
#define RIL_LOCAL_REQUEST_SETUP_DATA_CALL_ALT (RIL_LOCAL_REQUEST_VENDOR_BASE + 34)
#define RIL_LOCAL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE_ALT (RIL_LOCAL_REQUEST_VENDOR_BASE + 35)
#define RIL_LOCAL_REQUEST_PCSCF_DISCOVERY_PCO_ALT (RIL_LOCAL_REQUEST_VENDOR_BASE + 36)

/// M: CC: Switch antenna
#define RIL_LOCAL_REQUEST_SWITCH_ANTENNA (RIL_LOCAL_REQUEST_VENDOR_BASE + 37)

/// M: CC: Normal/Emergency call redial @{
#define RIL_LOCAL_REQUEST_EMERGENCY_REDIAL (RIL_LOCAL_REQUEST_VENDOR_BASE + 38)
#define RIL_LOCAL_REQUEST_EMERGENCY_SESSION_BEGIN (RIL_LOCAL_REQUEST_VENDOR_BASE + 39)
#define RIL_LOCAL_REQUEST_EMERGENCY_SESSION_END (RIL_LOCAL_REQUEST_VENDOR_BASE + 40)
/// @}
// Verizon hVoLTE/E911
#define RIL_LOCAL_REQUEST_SET_HVOLTE_MODE (RIL_LOCAL_REQUEST_VENDOR_BASE + 41)

#define RIL_LOCAL_REQUEST_REUSE_IMS_DATA_CALL (RIL_LOCAL_REQUEST_VENDOR_BASE + 48)
#define RIL_LOCAL_REQUEST_GET_IMS_DATA_CALL_INFO (RIL_LOCAL_REQUEST_VENDOR_BASE + 49)
// World Mode for duplex mode remove SIM Reset
#define RIL_LOCAL_REQUEST_RESUME_WORLD_MODE (RIL_LOCAL_REQUEST_VENDOR_BASE + 50)

// SIM gets EFDIR file.
#define RIL_LOCAL_REQUEST_SIM_GET_EFDIR (RIL_LOCAL_REQUEST_VENDOR_BASE + 51)
/// M: eMBMS feature
#define RIL_LOCAL_REQUEST_EMBMS_TRIGGER_CELL_INFO_NOTIFY (RIL_LOCAL_REQUEST_VENDOR_BASE + 52)
/// M: eMBMS end
/// M: [IR][C2K] Reset Suspend Mode @{
#define RIL_LOCAL_REQUEST_RESET_SUSPEND (RIL_LOCAL_REQUEST_VENDOR_BASE + 53)
/// @}


/// M: RJIL Subsidy lock feature. @{
#define RIL_LOCAL_REQUEST_GET_SUBLOCK_MODEM_STATUS (RIL_LOCAL_REQUEST_VENDOR_BASE + 54)
#define RIL_LOCAL_REQUEST_UPDATE_SUBLOCK_SETTINGS (RIL_LOCAL_REQUEST_VENDOR_BASE + 55)
/// @}

#define RIL_LOCAL_REQUEST_ABORT_AVAILABLE_NETWORK (RIL_LOCAL_REQUEST_VENDOR_BASE + 56)

#define RIL_LOCAL_REQUEST_EMERGENCY_DIAL (RIL_LOCAL_REQUEST_VENDOR_BASE + 57)
#define RIL_LOCAL_REQUEST_IMS_EMERGENCY_DIAL (RIL_LOCAL_REQUEST_VENDOR_BASE + 58)
#define RIL_LOCAL_REQUEST_SET_ECC_SERVICE_CATEGORY (RIL_LOCAL_REQUEST_VENDOR_BASE + 59)
#define RIL_LOCAL_REQUEST_STORE_MODEM_TYPE (RIL_LOCAL_REQUEST_VENDOR_BASE + 60)
#define RIL_LOCAL_REQUEST_RELOAD_MODEM_TYPE (RIL_LOCAL_REQUEST_VENDOR_BASE + 61)

/*********************************************************************************/
/*  Vendor C2K local request                                                     */
/*********************************************************************************/
#define RIL_LOCAL_REQUEST_VENDOR_C2K_BASE 2800
#define RIL_LOCAL_C2K_REQUEST_SWITCH_CARD_TYPE (RIL_LOCAL_REQUEST_VENDOR_C2K_BASE + 0)
/// M: [C2K 6M][NW] Config EVDO mode @{
#define RIL_LOCAL_C2K_REQUEST_CONFIG_EVDO_MODE (RIL_LOCAL_REQUEST_VENDOR_C2K_BASE + 1)
/// M: [C2K 6M][NW] Config EVDO mode @}

/// M: [C2K]Dynamic switch support. @{
#define RIL_REQUEST_ENTER_RESTRICT_MODEM_C2K (RIL_LOCAL_REQUEST_VENDOR_C2K_BASE + 2)
#define RIL_REQUEST_LEAVE_RESTRICT_MODEM_C2K (RIL_LOCAL_REQUEST_VENDOR_C2K_BASE + 3)
/// @}
// C2K SVLTE remote SIM access
#define RIL_REQUEST_CONFIG_MODEM_STATUS_C2K (RIL_LOCAL_REQUEST_VENDOR_C2K_BASE + 4)
/// M: [C2K][SVLTE] Set the SVLTE RAT mode. @{
#define RIL_LOCAL_C2K_REQUEST_SET_SVLTE_RAT_MODE (RIL_LOCAL_REQUEST_VENDOR_C2K_BASE + 5)
/// M: [C2K][SVLTE] Set the SVLTE RAT mode. @}
#define RIL_REQUEST_RESUME_REGISTRATION_CDMA (RIL_LOCAL_REQUEST_VENDOR_C2K_BASE + 6)
#define RIL_REQUEST_SET_REG_SUSPEND_ENABLED_CDMA (RIL_LOCAL_REQUEST_VENDOR_C2K_BASE + 7)
#define RIL_LOCAL_REQUEST_MODE_SWITCH_C2K_SET_TRM (RIL_LOCAL_REQUEST_VENDOR_C2K_BASE + 8)
// M: [C2K] MD IRAT feature
#define RIL_LOCAL_C2K_REQUEST_CONFIRM_INTER_3GPP_IRAT_CHANGE (RIL_LOCAL_REQUEST_VENDOR_C2K_BASE + 9)

/// M: [C2K][Power-on] modem enhanced {@
#define RIL_LOCAL_REQUEST_MODEM_POWEROFF_SYNC_C2K (RIL_LOCAL_REQUEST_VENDOR_C2K_BASE + 10)
/// @}
#define RIL_REQUEST_SET_MODEM_THERMAL (RIL_LOCAL_REQUEST_VENDOR_C2K_BASE + 11)
// M: MPS feature
#define RIL_LOCAL_C2K_REQUEST_DEACTIVATE_ALL_DATA_CALL (RIL_LOCAL_REQUEST_VENDOR_C2K_BASE + 12)
/// MAL {@
#define RIL_LOCAL_C2K_REQUEST_AT_COMMAND_WITH_PROXY_CDMA (RIL_LOCAL_REQUEST_VENDOR_C2K_BASE + 13)
/// @}

/// M: [C2K 6M][NW] Set the FDD TDD mode @{
#define RIL_LOCAL_REQUEST_SET_FDD_TDD_MODE (RIL_LOCAL_REQUEST_VENDOR_C2K_BASE + 14)
/// M: [C2K 6M][NW] Set the FDD TDD mode  @}

/// M: CC: Switch HPF
#define RIL_LOCAL_C2K_REQUEST_SWITCH_HPF (RIL_LOCAL_REQUEST_VENDOR_C2K_BASE + 15)

/// M: CC: Normal/Emergency call redial @{
#define RIL_LOCAL_C2K_REQUEST_REDIAL (RIL_LOCAL_REQUEST_VENDOR_C2K_BASE + 16)
#define RIL_LOCAL_C2K_REQUEST_EMERGENCY_REDIAL (RIL_LOCAL_REQUEST_VENDOR_C2K_BASE + 17)
/// @}
/// M: C2k AGPS
#define RIL_LOCAL_C2K_REQUEST_AGPS_TCP_CONNIND (RIL_LOCAL_REQUEST_VENDOR_C2K_BASE + 18)

#define RIL_LOCAL_C2K_REQUEST_EMERGENCY_DIAL (RIL_LOCAL_REQUEST_VENDOR_C2K_BASE + 19)

/// M: UTK: Set EUTK to modem @{
#define RIL_LOCAL_C2K_REQUEST_SET_UTK_MODE (RIL_LOCAL_REQUEST_VENDOR_C2K_BASE + 20)
/// @}

/*********************************************************************************/
/*  Vendor unsol                                                                 */
/*********************************************************************************/
#define RIL_UNSOL_VENDOR_BASE 3000
#define RIL_UNSOL_RESPONSE_PLMN_CHANGED (RIL_UNSOL_VENDOR_BASE + 0)
#define RIL_UNSOL_RESPONSE_REGISTRATION_SUSPENDED (RIL_UNSOL_VENDOR_BASE + 1)
/// M: [C2K 6M][NW] add for IWLAN @{
#define RIL_UNSOL_RESPONSE_PS_NETWORK_STATE_CHANGED (RIL_UNSOL_VENDOR_BASE + 2)
/// M: [C2K 6M][NW] add for IWLAN @}
/// M: for GMSS RAT
#define RIL_UNSOL_GMSS_RAT_CHANGED (RIL_UNSOL_VENDOR_BASE + 3)

#define RIL_UNSOL_CDMA_PLMN_CHANGED (RIL_UNSOL_VENDOR_BASE + 4)

// MTK-START: SIM
#define RIL_UNSOL_VIRTUAL_SIM_ON (RIL_UNSOL_VENDOR_BASE + 5)
#define RIL_UNSOL_VIRTUAL_SIM_OFF (RIL_UNSOL_VENDOR_BASE + 6)
#define RIL_UNSOL_IMEI_LOCK (RIL_UNSOL_VENDOR_BASE + 7)
#define RIL_UNSOL_IMSI_REFRESH_DONE (RIL_UNSOL_VENDOR_BASE + 8)
// MTK-END

// ATCI
#define RIL_UNSOL_ATCI_RESPONSE (RIL_UNSOL_VENDOR_BASE + 9)

#define RIL_UNSOL_RESPONSE_ETWS_NOTIFICATION (RIL_UNSOL_VENDOR_BASE + 10)
#define RIL_UNSOL_ME_SMS_STORAGE_FULL (RIL_UNSOL_VENDOR_BASE + 11)
#define RIL_UNSOL_SMS_READY_NOTIFICATION (RIL_UNSOL_VENDOR_BASE + 12)
  #define RIL_UNSOL_RESPONSE_CS_NETWORK_STATE_CHANGED (RIL_UNSOL_VENDOR_BASE + 13)
#define RIL_UNSOL_DATA_ALLOWED (RIL_UNSOL_VENDOR_BASE + 14)

/// M: CC: Proprietary incoming call indication
#define RIL_UNSOL_INCOMING_CALL_INDICATION (RIL_UNSOL_VENDOR_BASE + 15)

#define RIL_UNSOL_INVALID_SIM (RIL_UNSOL_VENDOR_BASE + 16)
// APC
#define RIL_UNSOL_PSEUDO_CELL_INFO (RIL_UNSOL_VENDOR_BASE + 17)
#define RIL_UNSOL_NETWORK_EVENT (RIL_UNSOL_VENDOR_BASE + 18)
#define RIL_UNSOL_MODULATION_INFO (RIL_UNSOL_VENDOR_BASE + 19)
// M: Reset Attach APN
#define RIL_UNSOL_RESET_ATTACH_APN (RIL_UNSOL_VENDOR_BASE + 20)
// M: IA-change attach APN
#define RIL_UNSOL_DATA_ATTACH_APN_CHANGED (RIL_UNSOL_VENDOR_BASE + 21)
// World Phone
#define RIL_UNSOL_WORLD_MODE_CHANGED (RIL_UNSOL_VENDOR_BASE + 22)

#define RIL_UNSOL_CDMA_CARD_INITIAL_ESN_OR_MEID (RIL_UNSOL_VENDOR_BASE + 23)

/// M: CC: Ciphering indication
#define RIL_UNSOL_CIPHER_INDICATION (RIL_UNSOL_VENDOR_BASE + 24)
/// M: CC: CRSS notifications
#define RIL_UNSOL_CRSS_NOTIFICATION (RIL_UNSOL_VENDOR_BASE + 25)

/// M: CC: deprecated - For 3G VT only
#define RIL_UNSOL_VT_STATUS_INFO (RIL_UNSOL_VENDOR_BASE + 26)

/// M: CC: GSA HD Voice for 2/3G network support
#define RIL_UNSOL_SPEECH_CODEC_INFO (RIL_UNSOL_VENDOR_BASE + 27)

#define RIL_UNSOL_PHB_READY_NOTIFICATION (RIL_UNSOL_VENDOR_BASE + 28)

// Femtocell feature
#define RIL_UNSOL_FEMTOCELL_INFO (RIL_UNSOL_VENDOR_BASE + 29)

#define RIL_UNSOL_NETWORK_INFO (RIL_UNSOL_VENDOR_BASE + 30)
/// [IMS] IMS RIL_UNSOL INDICATION =====================================================
#define RIL_UNSOL_CALL_INFO_INDICATION (RIL_UNSOL_VENDOR_BASE + 31)
#define RIL_UNSOL_ECONF_RESULT_INDICATION (RIL_UNSOL_VENDOR_BASE + 32)
#define RIL_UNSOL_SIP_CALL_PROGRESS_INDICATOR (RIL_UNSOL_VENDOR_BASE + 33)
#define RIL_UNSOL_CALLMOD_CHANGE_INDICATOR (RIL_UNSOL_VENDOR_BASE + 34)
#define RIL_UNSOL_VIDEO_CAPABILITY_INDICATOR (RIL_UNSOL_VENDOR_BASE + 35)
#define RIL_UNSOL_ON_USSI (RIL_UNSOL_VENDOR_BASE + 36)
#define RIL_UNSOL_GET_PROVISION_DONE (RIL_UNSOL_VENDOR_BASE + 37)
#define RIL_UNSOL_IMS_RTP_INFO (RIL_UNSOL_VENDOR_BASE + 38)
#define RIL_UNSOL_ON_XUI (RIL_UNSOL_VENDOR_BASE + 39)
#define RIL_UNSOL_IMS_EVENT_PACKAGE_INDICATION (RIL_UNSOL_VENDOR_BASE + 40)
#define RIL_UNSOL_IMS_REGISTRATION_INFO (RIL_UNSOL_VENDOR_BASE + 41)
#define RIL_UNSOL_IMS_ENABLE_DONE (RIL_UNSOL_VENDOR_BASE + 42)
#define RIL_UNSOL_IMS_DISABLE_DONE (RIL_UNSOL_VENDOR_BASE + 43)
#define RIL_UNSOL_IMS_ENABLE_START (RIL_UNSOL_VENDOR_BASE + 44)
#define RIL_UNSOL_IMS_DISABLE_START (RIL_UNSOL_VENDOR_BASE + 45)
#define RIL_UNSOL_ECT_INDICATION (RIL_UNSOL_VENDOR_BASE + 46)
#define RIL_UNSOL_VOLTE_SETTING (RIL_UNSOL_VENDOR_BASE + 47)
#define RIL_UNSOL_IMS_BEARER_STATE_NOTIFY (RIL_UNSOL_VENDOR_BASE + 49)
#define RIL_UNSOL_IMS_BEARER_INIT (RIL_UNSOL_VENDOR_BASE + 51)
#define RIL_UNSOL_IMS_DEREG_DONE (RIL_UNSOL_VENDOR_BASE + 52)
/// [IMS] IMS RIL_UNSOL INDICATION ======================================================
// M: [VzW] Data Framework
#define RIL_UNSOL_PCO_DATA_AFTER_ATTACHED (RIL_UNSOL_VENDOR_BASE + 53)
/* M: eMBMS feature */
#define RIL_UNSOL_EMBMS_SESSION_STATUS (RIL_UNSOL_VENDOR_BASE + 54)
#define RIL_UNSOL_EMBMS_AT_INFO  (RIL_UNSOL_VENDOR_BASE + 55)
/// M: eMBMS end
// MTK-START: SIM TMO RSU
#define RIL_UNSOL_MELOCK_NOTIFICATION (RIL_UNSOL_VENDOR_BASE + 56)
// MTK-END
// / M: BIP {
#define RIL_UNSOL_STK_BIP_PROACTIVE_COMMAND (RIL_UNSOL_VENDOR_BASE + 57)
// / M: BIP }
// / M: OTASP {
#define RIL_UNSOL_TRIGGER_OTASP  (RIL_UNSOL_VENDOR_BASE + 58)
// / M: OTASP }
#define RIL_UNSOL_MD_DATA_RETRY_COUNT_RESET (RIL_UNSOL_VENDOR_BASE + 59)
#define RIL_UNSOL_REMOVE_RESTRICT_EUTRAN (RIL_UNSOL_VENDOR_BASE + 60)
#define RIL_UNSOL_PCO_STATUS (RIL_UNSOL_VENDOR_BASE + 61) // NOT_USED
// M: [LTE][Low Power][UL traffic shaping] @{
#define RIL_UNSOL_LTE_ACCESS_STRATUM_STATE_CHANGE (RIL_UNSOL_VENDOR_BASE + 62)
// M: [LTE][Low Power][UL traffic shaping] @}

// MTK-START: SIM HOT SWAP / SIM RECOVERY
#define RIL_UNSOL_SIM_PLUG_IN (RIL_UNSOL_VENDOR_BASE + 63)
#define RIL_UNSOL_SIM_PLUG_OUT (RIL_UNSOL_VENDOR_BASE + 64)
#define RIL_UNSOL_SIM_MISSING (RIL_UNSOL_VENDOR_BASE + 65)
#define RIL_UNSOL_SIM_RECOVERY (RIL_UNSOL_VENDOR_BASE + 66)
// MTK-END
// MTK-START: SIM COMMON SLOT
#define RIL_UNSOL_TRAY_PLUG_IN (RIL_UNSOL_VENDOR_BASE + 67)
#define RIL_UNSOL_SIM_COMMON_SLOT_NO_CHANGED (RIL_UNSOL_VENDOR_BASE + 68)
// MTK-END

/// M: CC: CDMA call accepted indication
#define RIL_UNSOL_CDMA_CALL_ACCEPTED (RIL_UNSOL_VENDOR_BASE + 69)

// SS
#define RIL_UNSOL_CALL_FORWARDING (RIL_UNSOL_VENDOR_BASE + 70)
// / M: STK {
#define RIL_UNSOL_STK_SETUP_MENU_RESET (RIL_UNSOL_VENDOR_BASE + 71)
// / M: STK }
#define RIL_UNSOL_ECONF_SRVCC_INDICATION (RIL_UNSOL_VENDOR_BASE + 72)
// M: [VzW] Data Framework
#define RIL_UNSOL_VOLTE_LTE_CONNECTION_STATUS (RIL_UNSOL_VENDOR_BASE + 73)

// External SIM [Start]
#define RIL_UNSOL_VSIM_OPERATION_INDICATION (RIL_UNSOL_VENDOR_BASE + 74)
// External SIM [End]

/// Ims Data Framework {@
#define RIL_UNSOL_DEDICATE_BEARER_ACTIVATED (RIL_UNSOL_VENDOR_BASE + 75)
#define RIL_UNSOL_DEDICATE_BEARER_MODIFIED (RIL_UNSOL_VENDOR_BASE + 76)
#define RIL_UNSOL_DEDICATE_BEARER_DEACTIVATED (RIL_UNSOL_VENDOR_BASE + 77)
///@}

#define RIL_UNSOL_IMS_SUPPORT_ECC (RIL_UNSOL_VENDOR_BASE + 81)
/// M: Modem Multi-IMS support count
#define RIL_UNSOL_IMS_MULTIIMS_COUNT (RIL_UNSOL_VENDOR_BASE + 82)

/// M: MwiService @{
#define RIL_UNSOL_MOBILE_WIFI_ROVEOUT (RIL_UNSOL_VENDOR_BASE + 83)
#define RIL_UNSOL_MOBILE_WIFI_HANDOVER (RIL_UNSOL_VENDOR_BASE + 84)
#define RIL_UNSOL_ACTIVE_WIFI_PDN_COUNT (RIL_UNSOL_VENDOR_BASE + 85)
#define RIL_UNSOL_WIFI_RSSI_MONITORING_CONFIG (RIL_UNSOL_VENDOR_BASE + 86)
#define RIL_UNSOL_WIFI_PDN_ERROR (RIL_UNSOL_VENDOR_BASE + 87)
#define RIL_UNSOL_REQUEST_GEO_LOCATION (RIL_UNSOL_VENDOR_BASE + 88)
#define RIL_UNSOL_WFC_PDN_STATE (RIL_UNSOL_VENDOR_BASE + 89)
#define RIL_UNSOL_NATT_KEEP_ALIVE_CHANGED (RIL_UNSOL_VENDOR_BASE + 90)
#define RIL_UNSOL_WIFI_PING_REQUEST (RIL_UNSOL_VENDOR_BASE + 91)
#define RIL_UNSOL_WIFI_PDN_OOS (RIL_UNSOL_VENDOR_BASE + 92)
/// @}
#define RIL_UNSOL_TX_POWER (RIL_UNSOL_VENDOR_BASE + 93)
#define RIL_UNSOL_LTE_NETWORK_INFO (RIL_UNSOL_VENDOR_BASE + 94)
#define RIL_UNSOL_PHYSICAL_CHANNEL_CONFIGS_MTK (RIL_UNSOL_VENDOR_BASE + 95)

#define RIL_UNSOL_LINK_CAPACITY_ESTIMATE (RIL_UNSOL_VENDOR_BASE + 96)
/// M: IMS Conference info update
#define RIL_UNSOL_IMS_CONFERENCE_INFO_INDICATION (RIL_UNSOL_VENDOR_BASE + 97)
/// M: IMS LTE message waiting info update
#define RIL_UNSOL_LTE_MESSAGE_WAITING_INDICATION (RIL_UNSOL_VENDOR_BASE + 98)

/// M: IMS Dialog Event Package Info update
#define RIL_UNSOL_IMS_DIALOG_INDICATION (RIL_UNSOL_VENDOR_BASE + 99)

// [IMS] Telephonyware IMS config
#define RIL_UNSOL_IMS_CONFIG_DYNAMIC_IMS_SWITCH_COMPLETE (RIL_UNSOL_VENDOR_BASE + 100)
#define RIL_UNSOL_IMS_CONFIG_FEATURE_CHANGED (RIL_UNSOL_VENDOR_BASE + 101)
#define RIL_UNSOL_IMS_CONFIG_CONFIG_CHANGED (RIL_UNSOL_VENDOR_BASE + 102)
#define RIL_UNSOL_IMS_CONFIG_CONFIG_LOADED (RIL_UNSOL_VENDOR_BASE + 103)

#define RIL_UNSOL_ECC_NUM (RIL_UNSOL_VENDOR_BASE + 104)

#define RIL_UNSOL_MCCMNC_CHANGED (RIL_UNSOL_VENDOR_BASE + 105)

#define RIL_UNSOL_SIGNAL_STRENGTH_WITH_WCDMA_ECIO (RIL_UNSOL_VENDOR_BASE + 106)

#define RIL_UNSOL_TX_POWER_STATUS (RIL_UNSOL_VENDOR_BASE + 107)

#define RIL_UNSOL_EMERGENCY_BEARER_SUPPORT_NOTIFY (RIL_UNSOL_VENDOR_BASE + 108)

#define RIL_UNSOL_NETWORK_REJECT_CAUSE (RIL_UNSOL_VENDOR_BASE + 109)

#define RIL_UNSOL_ON_VOLTE_SUBSCRIPTION (RIL_UNSOL_VENDOR_BASE + 110)

#define RIL_UNSOL_IMS_DATA_INFO_NOTIFY (RIL_UNSOL_VENDOR_BASE + 111)

#define RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT_EX (RIL_UNSOL_VENDOR_BASE + 112)
#define RIL_UNSOL_RESPONSE_NEW_SMS_EX (RIL_UNSOL_VENDOR_BASE + 113)
#define RIL_UNSOL_DSBP_STATE_CHANGED (RIL_UNSOL_VENDOR_BASE + 114)
// MTK-START: SIM SLOT LOCK
#define RIL_UNSOL_SIM_SLOT_LOCK_POLICY_NOTIFY (RIL_UNSOL_VENDOR_BASE + 115)
// MTK-END
#define RIL_UNSOL_RESPONSE_CDMA_NEW_SMS_EX (RIL_UNSOL_VENDOR_BASE + 116)
#define RIL_UNSOL_NO_EMERGENCY_CALLBACK_MODE (RIL_UNSOL_VENDOR_BASE + 117)
#define RIL_UNSOL_REDIAL_EMERGENCY_INDICATION (RIL_UNSOL_VENDOR_BASE + 118)
#define RIL_UNSOL_SIM_POWER_CHANGED (RIL_UNSOL_VENDOR_BASE + 119)
#define RIL_UNSOL_CARD_DETECTED_IND (RIL_UNSOL_VENDOR_BASE + 120)
#define RIL_UNSOL_NETWORK_BAND_INFO (RIL_UNSOL_VENDOR_BASE + 121)

/// M: EUICC deprecated - Use RIL_UNSOL_ICC_SLOT_STATUS instead
#define RIL_UNSOL_SIM_SLOT_STATUS_CHANGED (RIL_UNSOL_VENDOR_BASE + 122)

/// M: CC: Proprietary incoming call indication
#define RIL_UNSOL_CALL_ADDITIONAL_INFO (RIL_UNSOL_VENDOR_BASE + 123)

/// M: CC: Proprietary additional SuppSvc info
#define RIL_UNSOL_SUPP_SVC_NOTIFICATION_EX (RIL_UNSOL_VENDOR_BASE + 124)
#define RIL_UNSOL_IMS_RESPONSE_CALL_STATE_CHANGED (RIL_UNSOL_VENDOR_BASE + 125)

// M: RTT @{
#define RIL_UNSOL_RTT_MODIFY_RESPONSE (RIL_UNSOL_VENDOR_BASE + 126)
#define RIL_UNSOL_RTT_TEXT_RECEIVE (RIL_UNSOL_VENDOR_BASE + 127)
#define RIL_UNSOL_RTT_CAPABILITY_INDICATION (RIL_UNSOL_VENDOR_BASE + 128)
#define RIL_UNSOL_RTT_MODIFY_REQUEST_RECEIVE (RIL_UNSOL_VENDOR_BASE + 129)
#define RIL_UNSOL_AUDIO_INDICATION (RIL_UNSOL_VENDOR_BASE + 130)
// @}
#define RIL_UNSOL_VOPS_INDICATION (RIL_UNSOL_VENDOR_BASE + 131)
// M: MWI: modem ask to reserve WiFi resource
#define RIL_UNSOL_WIFI_LOCK (RIL_UNSOL_VENDOR_BASE + 132)
#define RIL_UNSOL_SIP_REG_INFO (RIL_UNSOL_VENDOR_BASE + 133)
#define RIL_UNSOL_VIRTUAL_SIM_STATUS_CHANGED (RIL_UNSOL_VENDOR_BASE + 134)
#define RIL_UNSOL_ON_DSDA_CHANGED (RIL_UNSOL_VENDOR_BASE + 135)
#define RIL_UNSOL_IMS_REGISTRATION_STATE_IND (RIL_UNSOL_VENDOR_BASE + 136)
#define RIL_UNSOL_SIP_HEADER (RIL_UNSOL_VENDOR_BASE + 137)
#define RIL_UNSOL_QUALIFIED_NETWORK_TYPES_CHANGED (RIL_UNSOL_VENDOR_BASE + 138)
// MTK-START: SIM ATT RSU
#define RIL_UNSOL_ATT_SIM_LOCK_NOTIFICATION (RIL_UNSOL_VENDOR_BASE + 139)
// MTK-END
#define RIL_UNSOL_EIREG_INFO_IND (RIL_UNSOL_VENDOR_BASE + 140)

// MUSE WFC requirement
#define RIL_UNSOL_IWLAN_CELLULAR_QUALITY_CHANGED_IND  (RIL_UNSOL_VENDOR_BASE + 141)
// MUSE WFC requirement

#define RIL_UNSOL_SSAC_STATUS (RIL_UNSOL_VENDOR_BASE + 142)
#define RIL_UNSOL_CALL_RAT_INDICATION (RIL_UNSOL_VENDOR_BASE + 143)

// M: Update background mobile data usage
#define RIL_UNSOL_MOBILE_DATA_USAGE (RIL_UNSOL_VENDOR_BASE + 144)

#define RIL_UNSOL_NW_LIMIT (RIL_UNSOL_VENDOR_BASE + 145)
// Common RSU urc
#define RIL_UNSOL_SML_RSU_EVENT (RIL_UNSOL_VENDOR_BASE + 146)

/*********************************************************************************/
/*  Vendor GSM local unsol                                                       */
/*********************************************************************************/
#define RIL_LOCAL_GSM_UNSOL_VENDOR_BASE 6000
#define RIL_LOCAL_GSM_UNSOL_CARD_TYPE_NOTIFY (RIL_LOCAL_GSM_UNSOL_VENDOR_BASE + 0)
#define RIL_LOCAL_GSM_UNSOL_CT3G_DUALMODE_CARD (RIL_LOCAL_GSM_UNSOL_VENDOR_BASE + 1)
// M: [C2K] MD IRAT feature
#define RIL_LOCAL_GSM_UNSOL_INTER_3GPP_IRAT_STATE_CHANGE (RIL_LOCAL_GSM_UNSOL_VENDOR_BASE + 2)
#define RIL_LOCAL_GSM_UNSOL_CDMA_SMS_SPECIFIC_FROM_GSM (RIL_LOCAL_GSM_UNSOL_VENDOR_BASE + 3)
// modem enhanced
#define RIL_LOCAL_GSM_UNSOL_ENHANCED_MODEM_POWER (RIL_LOCAL_GSM_UNSOL_VENDOR_BASE + 4)
// M: PDN deactivation failure info to RILP
#define RIL_LOCAL_GSM_UNSOL_PDN_DEACT_FAILURE_IND (RIL_LOCAL_GSM_UNSOL_VENDOR_BASE + 5)
/// M:[Network] add for Wifi calling Service. @{
#define RIL_LOCAL_GSM_UNSOL_MAL_PS_RGEGISTRATION_STATE (RIL_LOCAL_GSM_UNSOL_VENDOR_BASE + 6)
/// @}
/// M: Ims Data Framework {@
#define RIL_LOCAL_GSM_UNSOL_DEDICATE_BEARER_ACTIVATED (RIL_LOCAL_GSM_UNSOL_VENDOR_BASE + 7)
#define RIL_LOCAL_GSM_UNSOL_DEDICATE_BEARER_MODIFIED (RIL_LOCAL_GSM_UNSOL_VENDOR_BASE + 8)
#define RIL_LOCAL_GSM_UNSOL_DEDICATE_BEARER_DEACTIVATED (RIL_LOCAL_GSM_UNSOL_VENDOR_BASE + 9)
#define RIL_LOCAL_GSM_UNSOL_MAL_DATA_CALL_LIST_CHANGED (RIL_LOCAL_GSM_UNSOL_VENDOR_BASE + 10)
/// @}
/// M: eMBMS feature
#define RIL_LOCAL_GSM_UNSOL_EMBMS_START_SESSION_RESPONSE (RIL_LOCAL_GSM_UNSOL_VENDOR_BASE + 11)
#define RIL_LOCAL_GSM_UNSOL_EMBMS_CELL_INFO_NOTIFICATION (RIL_LOCAL_GSM_UNSOL_VENDOR_BASE + 12)
#define RIL_LOCAL_GSM_UNSOL_EMBMS_COVERAGE_STATE (RIL_LOCAL_GSM_UNSOL_VENDOR_BASE + 13)
#define RIL_LOCAL_GSM_UNSOL_EMBMS_ACTIVE_SESSION (RIL_LOCAL_GSM_UNSOL_VENDOR_BASE + 14)
#define RIL_LOCAL_GSM_UNSOL_EMBMS_AVAILABLE_SESSION (RIL_LOCAL_GSM_UNSOL_VENDOR_BASE + 15)
#define RIL_LOCAL_GSM_UNSOL_EMBMS_SAI_LIST_NOTIFICATION (RIL_LOCAL_GSM_UNSOL_VENDOR_BASE + 16)
#define RIL_LOCAL_GSM_UNSOL_EMBMS_OOS_NOTIFICATION (RIL_LOCAL_GSM_UNSOL_VENDOR_BASE + 17)
/// M: eMBMS end
#define RIL_LOCAL_GSM_UNSOL_EF_ECC (RIL_LOCAL_GSM_UNSOL_VENDOR_BASE + 18)
#define RIL_LOCAL_GSM_UNSOL_IMS_CALL_EXIST (RIL_LOCAL_GSM_UNSOL_VENDOR_BASE + 19)

// Report uicc application list in OP09 A project for slot2.
#define RIL_LOCAL_GSM_UNSOL_ESIMIND_APPLIST (RIL_LOCAL_GSM_UNSOL_VENDOR_BASE + 20)

/*********************************************************************************/
/*  Vendor C2K local unsol                                                       */
/*********************************************************************************/
#define RIL_LOCAL_C2K_UNSOL_VENDOR_BASE 7000
#define RIL_LOCAL_C2K_UNSOL_CDMA_CARD_READY (RIL_LOCAL_C2K_UNSOL_VENDOR_BASE + 0)
#define RIL_LOCAL_C2K_UNSOL_CDMA_CARD_TYPE_NOTIFY (RIL_LOCAL_C2K_UNSOL_VENDOR_BASE + 1)
#define RIL_LOCAL_C2K_UNSOL_CDMA_IMSI_READY (RIL_LOCAL_C2K_UNSOL_VENDOR_BASE + 2)
// M: [C2K] MD IRAT feature
#define RIL_LOCAL_C2K_UNSOL_INTER_3GPP_IRAT_STATE_CHANGE (RIL_LOCAL_C2K_UNSOL_VENDOR_BASE + 3)
#define RIL_LOCAL_C2K_UNSOL_EF_ECC (RIL_LOCAL_C2K_UNSOL_VENDOR_BASE + 4)
/// M: [Network][C2K] add for upload engineer network info @{
#define RIL_LOCAL_C2K_UNSOL_ENG_MODE_NETWORK_INFO (RIL_LOCAL_C2K_UNSOL_VENDOR_BASE + 5)
/// @}
/// M: C2k AGPS
#define RIL_LOCAL_C2K_UNSOL_VIA_GPS_EVENT (RIL_LOCAL_C2K_UNSOL_VENDOR_BASE + 6)

/*********************************************************************************/
/*  External RPC request list                                                    */
/*  These IDs should not change after defined unless confirmed with rpc callers.
 *  When c2k ril received these requests, it will map them to c2k local request
 *  and execute.                                                                 */
/*********************************************************************************/
#define RIL_REQUEST_C2K_RPC_BASE 10000
#define RIL_REQUEST_C2K_RPC_SET_MODEM_THERMAL (RIL_REQUEST_C2K_RPC_BASE + 0)

/***********************************************************************/
/**
 *  For backward compatible, RIL implementations must defined RIL_GetRilVersion
 *  Return RIL_VERSION
 *
 *
 */

#ifndef C2K_RIL
#if defined(ANDROID_MULTI_SIM)
#define RIL_UNSOL_RESPONSE(a, b, c, d) RIL_onUnsolicitedResponse((a), (b), (c), (d))
#else
#define RIL_UNSOL_RESPONSE(a, b, c, d) RIL_onUnsolicitedResponse((a), (b), (c))
#endif
#endif

static const char PROPERTY_NW_ECC[MAX_SIM_COUNT][MAX_PROP_CHARS] = {
    "vendor.ril.ecc.service.category.list",
    "vendor.ril.ecc.service.category.list.1",
    "vendor.ril.ecc.service.category.list.2",
    "vendor.ril.ecc.service.category.list.3",
};

static const char PROPERTY_ECC_MCC[MAX_SIM_COUNT][MAX_PROP_CHARS] = {
    "vendor.ril.ecc.service.category.mcc",
    "vendor.ril.ecc.service.category.mcc.1",
    "vendor.ril.ecc.service.category.mcc.2",
    "vendor.ril.ecc.service.category.mcc.3",
};

#ifdef __cplusplus
}
#endif

#endif /*MTK_RIL_H*/
