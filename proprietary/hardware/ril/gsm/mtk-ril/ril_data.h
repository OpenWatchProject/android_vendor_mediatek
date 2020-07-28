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

#ifndef RIL_DATA_H
#define RIL_DATA_H 1

#ifdef DATA_CHANNEL_CTX
#undef DATA_CHANNEL_CTX
#endif
#define DATA_CHANNEL_CTX getRILChannelCtxFromToken(t)

#define SM_CAUSE_BASE 3072 // Defined in AT_DOCUMENT_full.doc (0xC00)
#define SM_CAUSE_END (SM_CAUSE_BASE + 0x72)
#define ESM_CAUSE_BASE 3328 // ESM cause    (0xD00)
#define ESM_CAUSE_END (ESM_CAUSE_BASE + 0xFF)

#define ME_PDN_URC "ME PDN ACT"
#define ME_DEDICATE_URC "ME ACT"
#define NW_DEDICATE_URC "NW ACT"

#define AUTHTYPE_NONE       2
#define AUTHTYPE_PAP        0
#define AUTHTYPE_CHAP       1
#define AUTHTYPE_PAP_CHAP   3
#define AUTHTYPE_NOT_SET    (-1)

#define SETUP_DATA_PROTOCOL_IP      "IP"
#define SETUP_DATA_PROTOCOL_IPV6    "IPV6"
#define SETUP_DATA_PROTOCOL_IPV4V6  "IPV4V6"

#define INVALID_CID -1

#define IPV4        0
#define IPV6        1
#define IPV4V6      2

#define NULL_IPV4_ADDRESS "0.0.0.0"
#define NULL_IPV6_ADDRESS "0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0"
#define MAX_IPV4_ADDRESS 17
#define MAX_IPV6_ADDRESS 42
#define MAX_NUM_DNS_ADDRESS_NUMBER 2
#define MAX_CCMNI_NUMBER 8
#define MAX_MPS_CCMNI_NUMBER 32
#define MAX_PASSWORD_LEN 256

#define DATA_STATE_ACTIVE 2
#define DATA_STATE_LINKDOWN 1
#define DATA_STATE_INACTIVE 0
//[C2K][IRAT] Indicate the PDN is deactivated fail before.
#define DATA_STATE_NEED_DEACT -1

#define MAX_L2P_VALUE_LENGTH 20
#define L2P_VALUE_UPS "M-UPS"
#define L2P_VALUE_CCMNI "M-CCMNI"
#define L2P_VALUE_MBIM "M-MBIM"

#define MAX_CCMNI_IFNAME_LENGTH 20
#define CCMNI_IFNAME_CCMNI "ccmni"
#define CCMNI_IFNAME_CCEMNI "ccemni"

#define DEFAULT_MAX_PDP_NUM 6

#define WIFI_MAX_PDP_NUM 10
#define WIFI_CID_OFFSET 100

#define DONGLE_MAX_PDP_NUM 2
#define REQUEST_DATA_DETACH 0
#define REQUEST_DATA_ATTACH 1

#define AT_COMMAND_MEDIUM_LENGTH 256

#define CCCI_IOC_MAGIC    'C'
#ifdef CCCI_IOC_FORCE_FD
    #undef CCCI_IOC_FORCE_FD
#endif
#define CCCI_IOC_FORCE_FD    _IOW(CCCI_IOC_MAGIC, 16, unsigned int)

#define REPLACE_APN_FLAG (-1)
#define PROPERTY_RE_IA_FLAG "persist.vendor.radio.re.ia.flag"
#define PROPERTY_RE_IA_APN "persist.vendor.radio.re.ia-apn"
#define PROP_DATA_ALLOW_STATUS "vendor.ril.data.allow.status"

#define MODEM_RAT_LTE 7
#define RAT_2_3G 0
#define HOME_RAT_REGISTERED 1
#define ROAMING_RAT_REGISTERED 5

#define SKIP_DATA_SETTINGS -2

static RIL_SOCKET_ID s_data_ril_cntx[] = {
    RIL_SOCKET_1,
    RIL_SOCKET_2,
    RIL_SOCKET_3,
    RIL_SOCKET_4
};

static RILChannelId sCmdChannel4Id[] = {
    RIL_CMD_4,
    RIL_CMD2_4,
    RIL_CMD3_4,
    RIL_CMD4_4
};

#define disable_test_load

#ifdef AT_RSP_FREE
#undef AT_RSP_FREE
#endif

#define AT_RSP_FREE(rsp)    \
if (rsp) {                  \
    at_response_free(rsp);  \
    rsp = NULL;             \
}

#ifdef FREEIF
#undef FREEIF
#endif

#define FREEIF(data)    \
if (data != NULL) {     \
    free(data);         \
    data = NULL;        \
}

// Multi-PS attach Start
static const char PROPERTY_IA[MAX_SIM_COUNT][50] = {
    "persist.vendor.radio.ia",
    "persist.vendor.radio.ia.1",
    "persist.vendor.radio.ia.2",
    "persist.vendor.radio.ia.3"
};

static const char PROPERTY_IA_FROM_NETWORK[MAX_SIM_COUNT][50] = {
    "vendor.ril.ia.network",
    "vendor.ril.ia.network.1",
    "vendor.ril.ia.network.2",
    "vendor.ril.ia.network.3"
};

static const char PROPERTY_TEMP_IA[MAX_SIM_COUNT][50] = {
    "vendor.ril.radio.ia",
    "vendor.ril.radio.ia.1",
    "vendor.ril.radio.ia.2",
    "vendor.ril.radio.ia.3"
};
// Multi-PS attach End
#define PROPERTY_IA_APN "persist.vendor.radio.ia-apn"
#define PROPERTY_IA_APN_SET_ICCID "vendor.ril.ia.iccid"
#define PROPERTY_IA_PASSWORD_FLAG "persist.vendor.radio.ia-pwd-flag"
#define PROPERTY_TEMP_IA_APN "vendor.ril.radio.ia-apn"
#define INVALID_APN_VALUE "this_is_an_invalid_apn"

// Fix: APN reset timing issue
#define RIL_DATA_INIT_MUTEX       {PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER,\
                                 PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER}

#define PROPERTY_SIM_OPERATOR_NUMERIC "gsm.sim.operator.numeric"
#define ATTACH_APN_NOT_SUPPORT 0
#define ATTACH_APN_PARTIAL_SUPPORT 1
#define ATTACH_APN_FULL_SUPPORT 2

#define UNSPECIFIED_IPV6_GATEWAY    "::"
#define DACONCPTR(src, dst) const char *dst = src.dst
#define DACONCPTR_P(src, dst) const char *dst = src->dst
#define DACONINT(src, dst) const int dst = src.dst
#define DACONINT_P(src, dst) const int dst = src->dst
#define DAINT_P(src, dst) int dst = src->dst

#define INIT_REQ_SETUP_CNF(CONFIG)      \
    DACONCPTR_P(CONFIG, requestedApn);  \
    DACONCPTR_P(CONFIG, username);      \
    DACONCPTR_P(CONFIG, password);      \
    DACONCPTR_P(CONFIG, profile);       \
    DACONINT_P(CONFIG, authType);       \
    DAINT_P(CONFIG, protocol);       \
    DACONINT_P(CONFIG, interfaceId);    \

/// M: Ims Data Framework {@
typedef struct {
    int isHandOver;
    int eranType;
} epdgConfig_t;

typedef struct {
    int ucho_status;  // 0: start_ho 1: stop_ho
    int fgho_result;  // 0: fail(RDS_FALSE) 1: success(RDS_TRUE)
    int interfaceId;
    int esource_ran_type;  // ran_type_e
    int etarget_ran_type;  // ran_type_e
} epdgHandoverStatus_t;

typedef enum {
    PDN_SETUP_THRU_MOBILE = 0,
    PDN_SETUP_THRU_WIFI = 1,
    PDN_SETUP_THRU_MAX,
    PDN_SETUP_THRU_ERR = -1
} pdn_setup_thru_type_e;

typedef enum {
    E_HO_TO_LTE = 0,
    E_HO_TO_WIFI = 1,
    E_HO_FROM_LTE = 0,
    E_HO_FROM_WIFI = 1,
    E_HO_UNKNOWN = -1
} pdn_ho_to_e;
/// @}

typedef struct {
    const char *radioType;
    const char* requestedApn;
    const char *username;
    const char *password;
    int authType;
    int protocol;
    // O migration - start
    const char *roamingProtocol;
    int supportedApnTypesBitmap;
    int bearerBitmap;
    int modemCognitive;
    int mtu;
    int mvnoType;
    const char* mvnoMatchData;
    int roamingAllowed;
    // O migration - end
    int interfaceId;
    int availableCid;   //for epdg
    int retryCount;
    const char *profile;
    void *pQueryRatRsp;
} reqSetupConf_t;


typedef void ( *pSetupDataCallFunc) (const reqSetupConf_t *, const void *, RIL_Token);

typedef enum {
    E_SETUP_DATA_CALL_FUNC_ERR = -1,
    E_SETUP_DATA_CALL_OVER_IPV6 = 0,
    E_SETUP_DATA_CALL_EMERGENCY,
    E_SETUP_DATA_CALL_FALLBACK,
    E_SETUP_DATA_CALL_OVER_EPDG,
    E_SETUP_DATA_CALL_HO_EPDG,
    E_SETUP_DATA_CALL_FUNC_CNT
} setup_data_call_type_e;

typedef enum {
    MAL_DM_EGACT_CAUSE_DONT_CARE = 0,
    MAL_DM_EGACT_CAUSE_DEACT_NORMAL,
    MAL_DM_EGACT_CAUSE_DEACT_RA_INITIAL_ERROR,
    MAL_DM_EGACT_CAUSE_DEACT_NO_PCSCF,
    MAL_DM_EGACT_CAUSE_DEACT_RA_REFRESH_ERROR,
} mal_dm_disconn_cause_t;

typedef enum {
    FWK_EGACT_CAUSE_DEACT_BASE = 2000,
    FWK_EGACT_CAUSE_DEACT_NORMAL = 2001,
    FWK_EGACT_CAUSE_DEACT_RA_INITIAL_ERROR = 2002,
    FWK_EGACT_CAUSE_DEACT_NO_PCSCF  = 2003,
    FWK_EGACT_CAUSE_DEACT_RA_REFRESH_ERROR = 2004,
} fwk_disconn_cause_t;

typedef struct {
    int cid;
    const char *username;
    const char *password;
    int authType;
    int queryMtu;
    const char *requestedApn;
    const char *pszIPStr;
    RILChannelCtx *pChannel;
} config_pco_param_t;


#ifndef UNUSED
    #define UNUSED(x) (x)   //eliminate "warning: unused parameter"
#endif

void requestSetupDataCall(void * data, size_t datalen, RIL_Token t);
void requestSetupDataCallFromMal(void * data, size_t datalen, RIL_Token t);
void requestOrSendDataCallListIpv6(RILChannelCtx* rilchnlctx, RIL_Token *t, RIL_SOCKET_ID rid);
void requestDeactiveDataCall(void * data, size_t datalen, RIL_Token t);
void requestLastDataCallFailCause(void * data, size_t datalen, RIL_Token t);
void requestLastDataCallFailCauseFromMal(void * data, size_t datalen, RIL_Token t);
void requestDataCallList(void * data, size_t datalen, RIL_Token t);
void requestSetInitialAttachApn(void * data, size_t datalen, RIL_Token t);
// LCE service start
void requestStartLce(void* data, size_t datalen, RIL_Token t);
void requestStopLce(void* data, size_t datalen, RIL_Token t);
void requestPullLceData(void* data, size_t datalen, RIL_Token t);
void onLceDataResult(void* param);
void requestSetLinkCapacityReportingCriteria(void* data, size_t datalen, RIL_Token t);
void onLinkCapacityEstimate(void* param);
// LCE service end
void onDataCallListChanged(RIL_Token t);
int getAuthTypeInt(int authTypeInt);

extern int rilDataMain(int request, void *data, size_t datalen, RIL_Token t);
extern int rilDataUnsolicited(const char *s, const char *sms_pdu, RILChannelCtx* p_channel);
extern int isSimInserted(RIL_SOCKET_ID rid);

// M: [VzW] Data Framework
void syncDisabledApnToMd(RILChannelCtx *pChannel);

/**********************************************************
 * MSIM Section Start
**********************************************************/
/***************  Basic Function  ***************/
void requestAllowData(void* data, size_t datalen, RIL_Token t);
void requestDataAttach(void* data, size_t datalen, RIL_Token t);
void requestDataIdle(void* data, size_t datalen, RIL_Token t);
int detachPs(RILChannelCtx* pChannel);
int needDetachPs(RIL_SOCKET_ID rid);
int doPsAttachAsync(void* param);

void requestGetImsDataCallInfo(void *data, size_t datalen, RIL_Token t);
void requestReuseImsDataCall(void *data, size_t datalen, RIL_Token t);

#define MAX_RETRY_PS_ATTACH 20

typedef enum {
    // Recieve this error cause when try to attach both protocls
    DC_MSIM_MULTI_ATTACH_ERROR = 4112,
    // For the case of EDALLOW command conflict (e.g. AP send EDALLOW=1 to both SIMs)
    DC_MSIM_ALLOW_ERROR = 4117,
    // Recieve this error cause when command conflict with EGTYPE
    DC_MSIM_CMD_CONFLICT_ERROR = 4118
} DC_MSIM_ERROR_CAUSE;

typedef enum {
    ATTACH_WHEN_NEEDED = 0,
    ALWAYS_ATTACH = 1,
    ALWAYS_ATTACH_WHEN_RADIO_OFF = 2,
    FOLLOW_ON_FLAG = 3,
    ALWAYS_ATTACH_WITH_CHECK = 4
} ATTACH_MODE;

typedef struct {
    int mode;
    RILChannelCtx* pDataChannel;
} AsyncAttachParam;

/***************  PS Attach Enhancement  ***************/
void requestRegisterPs(void* data, size_t datalen, RIL_Token t);
int doAttachBeforeRadioOn(RIL_SOCKET_ID rid, int onOff, RILChannelCtx *pChannel);
int combineAttachAndFollowOn(RIL_SOCKET_ID rid, int onOff, RILChannelCtx *pChannel);
int isNeedFollowOn(RIL_SOCKET_ID rid);
int isNeedCombineAttach(RIL_SOCKET_ID rid);
void onReattachForSimHotPlug(void* param);

// VzW IMS Data
extern bool skipIratUrcToMal(const char *urc);

#define PROPERTY_NEED_FOLLOW_ON "persist.vendor.ril.data.need.follow.on"
#define PROPERTY_NEED_COMBINED_ATTACH "persist.vendor.ril.data.need.combined.attach"

typedef enum {
    // Always NOT do combined attach
    COMBINED_ATTACH_DISABLED = 0,
    // Do combined attach under certain situation
    COMBINED_ATTACH_ENABLED = 1,
    // Always do combined attach no matter SIM inserted status, default data SIM, data enable
    COMBINED_ATTACH_FORCED = 2
} COMBINED_ATTACH_TYPE;

typedef enum {
    // Always NOT do follow on
    FOLLOW_ON_DISABLED = 0,
    // Do follow on under certain situation
    FOLLOW_ON_ENABLED = 1,
    // Always do follow on no matter SIM inserted status, default data SIM, data enable
    FOLLOW_ON_FORCED = 2
} FOLLOW_ON_TYPE;

/***************  Multiple PS Attach  ***************/
void setDataAllowed(int allowed, RIL_Token t);
int combineAttachMultiAttach(RIL_SOCKET_ID rid, int onOff, RILChannelCtx *pChannel);

/**********************************************************
 * MSIM Section End
**********************************************************/

/**********************************************************
 * Data Util Section Start
**********************************************************/
int getDataEnable();
int getDefaultDataSim();
void getIccidbySimSlot(char *iccid, RIL_SOCKET_ID rid);
int convertReqErrToOemErr(int reqId, int cause);
void onSimInsertChangedForData(RIL_SOCKET_ID rid, const char* urc);

extern int isMultiPsAttachSupport();

#define PROPERTY_RIL_DATA_ICCID "persist.vendor.radio.data.iccid"
#define PROPERTY_MOBILE_DATA_ENABLE "persist.vendor.radio.mobile.data"

/**********************************************************
 * Data Util Section End
**********************************************************/

/// M: Ims Data Framework @{


void clearRilDataMalCache();
void requestWifiConnectInd(void *data, size_t datalen, RIL_Token t);
void dumpDefaultBearerConfig(
    const RIL_Default_Bearer_VA_Config_Struct *pDefaultBearerVaConfig);
void requestPcscfPcoFromMal(void *data, size_t datalen, RIL_Token t);
/// @}

//FALLBACK PDP retry
#define PDP_FAIL_FALLBACK_RETRY (-1000)

// M: MPS feature
bool isImsInterfaceId(int interfaceId);

typedef enum {
    SM_OPERATOR_BARRED = 0b00001000,
    SM_MBMS_CAPABILITIES_INSUFFICIENT = 0b00011000,
    SM_LLC_SNDCP_FAILURE = 0b00011001,
    SM_INSUFFICIENT_RESOURCES = 0b00011010,
    SM_MISSING_UNKNOWN_APN = 0b00011011,
    SM_UNKNOWN_PDP_ADDRESS_TYPE = 0b00011100,
    SM_USER_AUTHENTICATION_FAILED = 0b00011101,
    SM_ACTIVATION_REJECT_GGSN = 0b00011110 ,
    SM_ACTIVATION_REJECT_UNSPECIFIED = 0b00011111,
    SM_SERVICE_OPTION_NOT_SUPPORTED = 0b00100000,
    SM_SERVICE_OPTION_NOT_SUBSCRIBED = 0b00100001,
    SM_SERVICE_OPTION_OUT_OF_ORDER = 0b00100010,
    SM_NSAPI_IN_USE = 0b00100011,
    SM_REGULAR_DEACTIVATION = 0b00100100,
    SM_QOS_NOT_ACCEPTED = 0b00100101,
    SM_NETWORK_FAILURE = 0b00100110,
    SM_REACTIVATION_REQUESTED = 0b00100111,
    SM_FEATURE_NOT_SUPPORTED = 0b00101000,
    SM_SEMANTIC_ERROR_IN_TFT = 0b00101001,
    SM_SYNTACTICAL_ERROR_IN_TFT = 0b00101010,
    SM_UNKNOWN_PDP_CONTEXT = 0b00101011,
    SM_SEMANTIC_ERROR_IN_PACKET_FILTER = 0b00101100,
    SM_SYNTACTICAL_ERROR_IN_PACKET_FILTER = 0b00101101,
    SM_PDP_CONTEXT_WITHOU_TFT_ALREADY_ACTIVATED = 0b00101110,
    SM_MULTICAST_GROUP_MEMBERSHIP_TIMEOUT = 0b00101111,
    SM_BCM_VIOLATION = 0b00110000,
    SM_LAST_PDN_DISC_NOT_ALLOWED = 0b00110001,
    SM_ONLY_IPV4_ALLOWED = 0b00110010,
    SM_ONLY_IPV6_ALLOWED = 0b00110011,
    SM_ONLY_SINGLE_BEARER_ALLOWED = 0b00110100,
    ESM_INFORMATION_NOT_RECEIVED = 0b00110101,
    SM_PDN_CONNECTION_NOT_EXIST = 0b00110110,
    SM_MULTIPLE_PDN_APN_NOT_ALLOWED = 0b00110111,
    SM_COLLISION_WITH_NW_INITIATED_REQUEST = 0b00111000,
    ESM_UNSUPPORTED_QCI_VALUE = 0b00111011,
    SM_BEARER_HANDLING_NOT_SUPPORT = 0b00111100,
    SM_MAX_PDP_NUMBER_REACHED = 0b01000001,
    SM_APN_NOT_SUPPORT_IN_RAT_PLMN = 0b01000010,
    SM_INVALID_TRANSACTION_ID_VALUE = 0b01010001,
    SM_SEMENTICALLY_INCORRECT_MESSAGE = 0b01011111,
    SM_INVALID_MANDATORY_INFO = 0b01100000,
    SM_MESSAGE_TYPE_NONEXIST_NOT_IMPLEMENTED = 0b01100001,
    SM_MESSAGE_TYPE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE = 0b01100010,
    SM_INFO_ELEMENT_NONEXIST_NOT_IMPLEMENTED = 0b01100011,
    SM_CONDITIONAL_IE_ERROR = 0b01100100,
    SM_MESSAGE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE = 0b01100101,
    SM_PROTOCOL_ERROR = 0b01101111,
    SM_APN_RESTRICTION_VALUE_INCOMPATIBLE_WITH_PDP_CONTEXT = 0b01110000
} AT_SM_Error;

#define ERROR_CAUSE_GENERIC_FAIL 14

typedef struct {
    int interfaceId;
    int primaryCid;
    int cid;
    int ddcId;  // for dedicate bearer
    int isDedicateBearer;
    int isEmergency;  // 0: normal, 1: emergency PDN
    int active;  // 0: inactive, 1: link down, 2: active
    int signalingFlag;
    char apn[128];
    char addressV4[MAX_IPV4_ADDRESS_LENGTH];
    char addressV6[MAX_IPV6_ADDRESS_LENGTH];
    char dnsV4[MAX_NUM_DNS_ADDRESS_NUMBER][MAX_IPV4_ADDRESS_LENGTH];
    char dnsV6[MAX_NUM_DNS_ADDRESS_NUMBER][MAX_IPV6_ADDRESS_LENGTH];
    Qos qos;
    Tft tft;
    int bearerId;
    char pcscf[MAX_PCSCF_NUMBER][MAX_IPV6_ADDRESS_LENGTH];
    int isNoDataPdn;  // 0: false, 1: true; for No Data PDN check
    int mtu;
    int rat;    /* 0: UNSPEC, 1: MOBILE_3GPP, 2: WIFI, 3: MOBILE_3GPP2
                   4: TYPE_MAX, 0xFE: RDS_RAN_NONE, 0xFF: RDS_RAN_DENY */
    epdgHandoverStatus_t hoStatus;
    int deactbyNw;  // 0: not deact, 1: deact by nw for HO stop used
    int ho_start_cnt;
    int ho_stop_cnt;
    int reason; // -1: no cause, 0: ipv4 only, 1: ipv6 only, 2: single bearer only allowed
    char prefix[MAX_IPV6_ADDRESS_LENGTH];
} PdnInfo;

typedef struct {
    RIL_SOCKET_ID rid;
    char* urc;
    int isModification;
} TimeCallbackParam;

typedef struct {
    int activeCid;
    int reason;
    RILChannelCtx* pDataChannel;
} MePdnActiveInfo;

typedef enum {
    NO_CAUSE = -1,
    IPV4_ONLY = 0,
    IPV6_ONLY = 1,
    SINGLE_BEARER_ONLY_ALLOWED = 2
} PDN_REASON;

typedef enum {
    LOCAL_REJECT_CAUSE = -2,
    CHANNEL_CLOSED = -1,
    PDN_FAILED = 0,
    PDN_SUCCESS = 1
} PDN_SETUP_STATUS;

typedef enum {
    NO_SUGGESTED_TIME = -1,
    NO_SUGGESTION = 0,
    HAS_PDN_RETRY_TIME_SUGGESTION = 1
} MD_SUGGESTED_RETRY_TIME_STATUS;

typedef enum {
    MD_NOT_SUPPORTED = 0,
    MD_WR8 = 1,
    MD_LR9 = 2,
    MD_LR11 = 3
} MD_SUPPORT_VERSION;

typedef enum {
    DATA_L4C_LAST_PDN_ERROR_CAUSE = 0,
    DATA_L4C_PDN_CAUSE_REASON,
    DATA_L4C_MD_VERSION,
    TOTAL_FEATURE_NUMBER
} Query_MD_Support_Version_Feature_List;

// [C2K][IRAT] start {@
void requestSetActivePsSimSlot(void *data, size_t datalen, RIL_Token t);
void confirmIratChange(void *data, size_t datalen, RIL_Token t);

typedef enum {
    IRAT_ACTION_UNKNOWN = 0,
    IRAT_ACTION_SOURCE_STARTED = 1,
    IRAT_ACTION_TARGET_STARTED = 2,
    IRAT_ACTION_SOURCE_FINISHED = 3,
    IRAT_ACTION_TARGET_FINISHED = 4
} PDN_IRAT_ACTION;

typedef enum {
    IRAT_TYPE_UNKNOWN = 0,
    IRAT_TYPE_LTE_EHRPD = 1,
    IRAT_TYPE_LTE_HRPD = 2,
    IRAT_TYPE_EHRPD_LTE = 3,
    IRAT_TYPE_HRPD_LTE = 4,
    IRAT_TYPE_FAILED = 5,
    IRAT_TYPE_LWCG_LTE_EHRPD = 6,
    IRAT_TYPE_LWCG_LTE_HRPD = 7,
    IRAT_TYPE_LWCG_EHRPD_LTE = 8,
    IRAT_TYPE_LWCG_HRPD_LTE = 9
} PDN_IRAT_TYPE;

typedef enum {
    DISABLE_CCMNI = 0,
    ENABLE_CCMNI = 1
} CCMNI_STATUS;
// [C2K][IRAT] end @}

static const char RP_DATA_PROPERTY_IA[MAX_SIM_COUNT][30] = {
    "vendor.ril.c2kirat.ia.sim1",
    "vendor.ril.c2kirat.ia.sim2",
    "vendor.ril.c2kirat.ia.sim3",
    "vendor.ril.c2kirat.ia.sim4",
};

// [C2K][IRAT] start {@
typedef enum {
    IRAT_NO_RETRY = 0,
    IRAT_NEED_RETRY = 1
} IRAT_RETRY_STATUS;

typedef enum {
    IRAT_PDN_STATUS_UNKNOWN = -1,
    IRAT_PDN_STATUS_SYNCED = 0,
    IRAT_PDN_STATUS_DEACTED = 1,
    IRAT_PDN_STATUS_REACTED = 2
} IRAT_PDN_STATUS;

typedef enum {
    RAT_UNKNOWN = 0,
    RAT_1XRTT = 1,
    RAT_HRPD = 2,
    RAT_EHRPD = 3,
    RAT_LTE = 4
} PDN_RAT_VALUE;

typedef enum {
    SUSPEND_DATA_TRANSFER = 0,
    RESUME_DATA_TRANSFER = 1,
} IRAT_DATA_TRANSFER_STATE;

typedef struct {
    int interfaceId;
    int cid;
    char apn[128];
    char addressV4[MAX_IPV4_ADDRESS_LENGTH];
    char addressV6[MAX_IPV6_ADDRESS_LENGTH];
    int pdnStatus;
} SyncPdnInfo;
// [C2K][IRAT] end @}

/// M: Ims Data Framework {@
typedef enum {
   E_HO_DIR_LTE_TO_WIFI = 0,
   E_HO_DIR_WIFI_TO_LTE = 1,
   E_HO_DIR_UNKNOWN = -1,
} pdn_ho_dir_e;

// M: To remove LTE PDN information after handover from LTE to Wifi
//    when the network deactivation comes during handover progress @{
typedef struct {
    bool needRemove;
    int cid;
} HoPdnRemovePendingAction;
/// @}


void initialDataCallResponse(MTK_RIL_Data_Call_Response_v11* responses, int length);
extern int isATCmdRspErr(int err, const ATResponse *p_response);
extern int isWorldPhoneSupport();

void initialCidTable();
int updatePdnAddressByCid(int cid, RILChannelCtx* rilchnlctx);
int updateDns(RILChannelCtx* rilchnlctx);
int updateActiveStatus(RILChannelCtx* rilchnlctx);
int isCidActive(int *cidList, int size, RILChannelCtx *rilchnlctx);

void onNetworkBearerUpdate(void* param);
void configPcoParam(config_pco_param_t *param);
void getAndSendPcoStatus(RIL_SOCKET_ID rid, int cid, char* apnName);
void getAndSendPcoStatusWithParam(void* param);
void onPcoStatus(void* param);
int getAddressTypeByCid(int cid, RIL_SOCKET_ID rid);
int definePdnCtx(const RIL_Default_Bearer_VA_Config_Struct *pDefaultBearerVaConfig, const char *requestedApn,
                    int protocol, int availableCid, int authType, const char *username, const char* password, RILChannelCtx *pChannel);
int activatePdn(int availableCid, int *pActivatedPdnNum, int *pConcatenatedBearerNum, int *activatedCidList, int isEmergency,
                    int isNoDataActive, RILChannelCtx *pChannel);
int bindPdnToIntf(int activatedPdnNum, int interfaceId, int cid,
                    RILChannelCtx *pChannel);

int getAvailableCid(RIL_SOCKET_ID rid);
int getAvailableCidBasedOnSpecificCid(int sCid, RIL_SOCKET_ID rid);
int getAvailableWifiCid(RIL_SOCKET_ID rid);
void getPdpCapability(RIL_Token t);
void getCcmniIfname();
void getL2pValue(RIL_Token t);
int configureQos(int cid, Qos* qos, RILChannelCtx* rilchnlctx);
int configureTft(int cid, Tft* tft, RILChannelCtx* rilchnlctx);
void updateNetworkInterface(int interfaceId, int mask, RIL_SOCKET_ID rid);
void configureNetworkInterface(int interfaceId, int isUp, RIL_SOCKET_ID rid);
void setNwIntfDown(const char * pzIfName);
int createDataResponse(int interfaceId, int protocol, MTK_RIL_Data_Call_Response_v11* response, RIL_SOCKET_ID rid);
void dumpDataResponse(MTK_RIL_Data_Call_Response_v11* response, const char* reason);
void freeDataResponse(MTK_RIL_Data_Call_Response_v11* response);
int convert_ipv6_address(char* pdp_addr, char* ipv6_addr, int isLinkLocal);
int get_address_type(char* addr);
int get_protocol_type(const char* protocol);
int apnMatcher (const char* out, const char* apn);
int queryMatchedPdnWithSameApn(const char* apn, int* matchedCidList, RILChannelCtx* rilchnlctx);
char* getConfigTftATCmd(int cid, PktFilter* pktFilter);
int clearQosTftContext(int cid, RILChannelCtx* rilchnlctx);
int isCidTypeMatched(int cid, int protocol, RILChannelCtx* rilchnlctx);
int queryCurrentConnType(RILChannelCtx* rilchnlctx);
int querryCurrentRat(RILChannelCtx* rilchnlctx);
int handleLastPdnDeactivation (const int isEmergency, const int lastPDNCid, RILChannelCtx* rilchnlctx);
int deactivateDataCall(int cid, RILChannelCtx* rilchnlctx);
int deactivateDataCallByReason(int cid, int reason, RILChannelCtx* rilchnlctx);

// IA
void getIaCache(int iaSimId, char* cache);
void setIaCache(int iaSimId, const char* iccid, const char* protocol, const int authtype,
        const char* username, const int canHandleIms, const char* apn,
        const char* roamingProtocol);

void getTempIa(int iaSimId, char* cache);
void setTempIa(int iaSimId, const char* iccid, const char* protocol, const int authtype,
        const char* username, const int canHandleIms, const char* apn,
        const char* roamingProtocol);
int isIaInvalid(RILChannelCtx* rilchnlctx);
// Multi-PS Attach Start
int defineAttachApnBeforeRadioOn(RIL_SOCKET_ID rid, RILChannelCtx *pChannel);
void defineAttachApnAsync(void* param);
int defineMultiAttachApnIfIACacheExisted(RIL_SOCKET_ID rid, RILChannelCtx *pChannel);
// Multi-PS Attach End
int defineAttachApnIfIACacheExisted(RIL_SOCKET_ID rid, RILChannelCtx *pChannel);
void clearAttachApnCacheIfNecessary(RIL_SOCKET_ID rid, const char* urc);
void onSetAttachApn(void* param);
void onBarApn(char* urc, RIL_SOCKET_ID rid);
int isInitialAttachApn(const char *requestedApn, RILChannelCtx *pChannel, int cid);
int getIaReplaceFlag(int iaSimId);
void setIaReplaceFlag(int iaSimId, int flag);
void getIaReplaceApn(int iaSimId, char* buffer);

// for JPN IA
extern SIM_Status getSIMStatus(RIL_SOCKET_ID rid);

extern bool isFallbackNotSupportByOp();

static bool isFallbackPdpRetryFail(RIL_SOCKET_ID rid);
extern void givePrintableIccid(char *iccid, char *maskIccid);

void requestSetupDataCallOverIPv6(const reqSetupConf_t *pReqSetupConfig,
        const void* ptrDefaultBearerVaConfig, RIL_Token t);
void requestSetupDataCallFallback(const reqSetupConf_t *pReqSetupConfig,
    const void * ptrDefaultBearerVaConfig, RIL_Token t);

/// M: Ims Data Framework {@
void requestSetupDataCallEmergency(const reqSetupConf_t *pReqSetupConfig,
    const void *ptrDefaultBearerVaConfig, RIL_Token t);
void requestSetupDataCallOverEpdg(const reqSetupConf_t *pReqSetupConfig,
    const void * ptrDefaultBearerVaConfig, RIL_Token t);
void requestSetupDataCallHoEpdg(const reqSetupConf_t *pReqSetupConfig,
    const void *ptrDefaultBearerVaConfig, RIL_Token t, const int serialId);
/// @}

// LCE service start
void requestStartLce(void* data, size_t datalen, RIL_Token t);
void requestStopLce(void* data, size_t datalen, RIL_Token t);
void requestPullLceData(void* data, size_t datalen, RIL_Token t);
int setLceMode(int lceMode, int reportIntervalMs, RIL_LceStatusInfo* response, RILChannelCtx *pChannel);
// LCE service end

// Keep alive start
void requestStartKeepalive(void* data, size_t datalen, RIL_Token t);
void requestStopKeepalive(void* data, size_t datalen, RIL_Token t);
void onUpdateKeepaliveStatus(void* param);
// Keep alive end

int getModemSuggestedRetryTime(int availableCid, RILChannelCtx *pChannel);
void storeModemSuggestedRetryTime(int time, MTK_RIL_Data_Call_Response_v11* response);

/// [C2K][IRAT] code start. {@
extern int isEactsSupport();
char* getNetworkInterfaceName(int interfaceId);
void bindNetworkInterfaceWithModem(int interfaceId, int modemId);
int onPdnSyncFromSourceRat(void* param);
void onIratEvent(void* param);
void onLwcgIratEvent(void* param);
void clearAllSyncPdnInfo();
void clearAllPdnInfo(RIL_SOCKET_ID rid);
void clearSyncPdnInfo(SyncPdnInfo* info);
void cleanupFailedPdns(RIL_SOCKET_ID rid);
void cleanupFailedPdnByCid(int cid, RIL_SOCKET_ID rid);
void cleanupPdnsForFallback(RIL_SOCKET_ID rid);
void onIratPdnReactSucc(int cid, RILChannelCtx* rilchnlctx);
void onIratSourceStarted(int sourceRat, int targetRat, int type, RIL_SOCKET_ID rid);
void onIratSourceFinished(int sourceRat, int targetRat, int type, RIL_SOCKET_ID rid);
void onIratTargetStarted(int sourceRat, int targetRat, int type, RIL_SOCKET_ID rid);
void onIratTargetFinished(int sourceRat, int targetRat, int type, RIL_SOCKET_ID rid);
void onIratStateChanged(int sourceRat, int targetRat, int action,
        int type, RIL_SOCKET_ID rid);
void handleDeactedOrFailedPdns(int retryFailedPdn, RIL_SOCKET_ID rid);
void handleLwcgFailedPdns(RIL_SOCKET_ID rid);
void reactFailedPdnByCid(int cid, RIL_SOCKET_ID rid);
void deactivatePdnByCid(int cid, RIL_SOCKET_ID rid);
int getIratType(int sourceRat, int targetRat);
int getLwcgIratType(int sourceRat, int targetRat);
int rebindPdnToIntf(int interfaceId, int cid, RILChannelCtx * pChannel);
void reconfigureNetworkInterface(int interfaceId, RIL_SOCKET_ID rid);
void setNetworkTransmitState(int interfaceId, int state);
void resumeAllDataTransmit(RIL_SOCKET_ID rid);
void suspendAllDataTransmit(RIL_SOCKET_ID rid);
void onUpdateIratStatus(RIL_SOCKET_ID rid);
void onResetIratStatus();
int getActivePsSlot(int psSlot);
int setPsActiveSlot(int actedSim, RILChannelCtx *pChannel);
/// [C2K][IRAT] code end. @}
//
bool isImsInterfaceId(int interfaceId);

/// M: Ims Data Framework {@
bool isIpv6LinkLocal(const char *ipv6Addr);
int isIpv6Global(const char *ipv6Addr);
bool isLegalPdnRequest(int interfaceId, RIL_SOCKET_ID rid);
void convertToGlobalAddress(char* addressV6, char* prefix);
int updateQos(RILChannelCtx* rilchnlctx);
int updateDedicateBearerInfo(RILChannelCtx* rilchnlctx);
void updateDynamicParameter(RILChannelCtx* rilchnlctx);
void dumpReqSetupConfig(const reqSetupConf_t *pConfig);
void dumpPdnInfo(PdnInfo *pPdnInfo);
void dumpAllPdnInfo(RIL_SOCKET_ID rid);
void clearRdsSocketHandle();
int getImsParamInfo(RIL_SOCKET_ID rid, int intfId, RIL_Default_Bearer_VA_Config_Struct * pDefaultBearerVaConfig);
int getValidSimId(RIL_SOCKET_ID rid, const char *caller);
void configEpdg(epdgConfig_t *pConfig, const RIL_Default_Bearer_VA_Config_Struct *pDefaultBearerConfig);
void cpStr(char *pszDst, const char* pszStr, int strSize, int bufferSize);
int queryEpdgRat(RIL_SOCKET_ID rid, reqSetupConf_t *pConfig, RIL_Default_Bearer_VA_Config_Struct* pDefaultBearerVaConfig);
int deactivatePdnThruEpdg(RIL_SOCKET_ID rid, int interfaceId, int reason);
bool isPdnHoStart(int hoStatus);
bool isPdnHoStartFromRds(RIL_SOCKET_ID rid, PdnInfo *pInfo);
bool getHoRefCountFromRds(RIL_SOCKET_ID rid, PdnInfo *pInfo);
bool isHOSuccess(int hoResult);
int handoverTo(const epdgHandoverStatus_t *pHoStatus);
int handoverFrom(const epdgHandoverStatus_t *pHoStatus);
void requestHandoverInd(void *data, size_t datalen, RIL_Token t);
void requestWifiDisconnectInd(void *data, size_t datalen, RIL_Token t);
void adaptIpDiscontinuityNw(int interfaceId, int cid, RIL_SOCKET_ID rid);
void dumpHandoverStatus(const epdgHandoverStatus_t *pHoStatus);
void requestPcscfPco(void *data, size_t datalen, RIL_Token t);
void configureEpdgNetworkInterface(int interfaceId, int protocol, int isUp, RIL_SOCKET_ID rid);
int getDedicatedPdnList(const int def_bearer_cid, const RIL_SOCKET_ID rid, int *pDedicatedPdnList);
void notifyLinkdownDedicatedBearer(const int def_bearer_cid, const RIL_SOCKET_ID rid);
void *readInt(void *pData, int *pValue);
void *readBytes(void *pData, int bytes);
void updateDataAllowStatus(int slotId, bool allow);
bool isDataAllow(int slotId);
/// @}
void requestSetpropImsHandover(void *data, size_t datalen, RIL_Token t);
int isSysEpdgSupport();
extern void clearIaCacheAndDefineInvalidIa();
extern int setFastdormancyState(int state, int slot);
#endif /* RIL_DATA_H */

