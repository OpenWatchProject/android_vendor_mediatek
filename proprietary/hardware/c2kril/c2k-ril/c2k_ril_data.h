#ifndef C2K_RIL_DATA_H
#define C2K_RIL_DATA_H

#include "telephony/mtk_ril.h"
#include "atchannel.h"

// CDMA PS fail cause defined by MD3.
typedef enum {
    PS_FEATURE_UNSUPPORTED = 140,
    PS_UNKNOWN_PDP_CONTEXT = 143,
    PS_ALREADY_ACTIVATED = 146,
    PS_NETWORK_FAILURE = 158,
    PS_MAX_PDP_CONTEXT_COUNT_REACHED = 178,

    PS_GENERAL_ERROR = 0x8000,
    PS_UNAUTHORIZED_APN = 0x8001,
    PS_PDN_LIMITED_EXCEEDED = 0x8002,
    PS_NO_PGW_AVALAIABLE = 0x8003,
    PS_PGW_UNREACHABLE = 0x8004,
    PS_PGW_REJECT = 0x8005,
    PS_INSUFICIENT_PARAMETERS = 0x8006,
    PS_RESOURCE_UNAVAILABLE = 0x8007,
    PS_ADMIN_PROHIBITED = 0x8008,
    PS_PDN_ID_ALREADY_IN_USE = 0x8009,
    PS_SUBSCRIPTION_LIMITATION = 0x800a,
    PS_PDN_CONN_ALREADY_EXIST_FOR_PDN = 0x800b,
    PS_EMERGENCY_NOT_ALLOWED = 0x800c,
    PS_RECONNECT_NOT_ALLOWED = 0x800d,

    PS_EAPAKA_FAILURE = 0x8080,
    PS_RETRY_TIMER_THROTTLING = 0x8081,
    PS_NETWORK_NO_RESPONSE = 0x8082,
    PS_PDN_ATTACH_ABORT = 0x8083,
    PS_PDN_LINIT_EXCEEDED_IN_UE_SIDE = 0x8084,
    PS_PDN_ID_ALREADY_IN_USE_IN_UE_SIDE = 0x8085,
    PS_INVALID_PDN_ATTACH_REQUEST = 0x8086,
    PS_PDN_REC_FAILURE = 0x8087,
    PS_MAIN_CONN_SETUP_FAILURE = 0x8088,
    PS_BEARER_RESOURCE_UNAVAILABLE = 0x8089,
    PS_OP_ABORT_BY_USER = 0x808a,
    PS_RTT_DATA_CONNECTED = 0x808b,
    PS_EAPAKA_REJECT = 0x808c,
    PS__LCP_NEGO3_FAILURE = 0x808d,
    PS_TCH_SETUP_FAILURE = 0x808e,
    PS_NW_NO_RESPONSE_IN_LCP = 0x808f,
    PS_NW_NO_RESPONSE_IN_AUTH = 0x8090,

    PS_ERROR_UNSPECIFIED = 0x80b0,
    PS_AUTH_FAIL = 0x80b1,
    PS_MIP_PPP_LCP_TIMEOUT = 0x80b2,
    PS_MIP_PPP_NCP_TIMEOUT = 0x80b3,
    PS_SIP_PPP_LCP_TIMEOUT = 0x80b4,
    PS_SIP_PPP_NCP_TIMEOUT = 0x80b5,
    PS_MIP_PPP_OPT_MISMATCH = 0x80b6,
    PS_SIP_PPP_OPT_MISMATCH = 0x80b7,
    PS_MIP_SOLICITATION_TIMEOUT = 0x80b8,
    PS_MIP_RRQ_TIMEOUT = 0x80b9,
    PS_MIP_RRP_ERROR = 0x80ba,
    PS_MIP_PPP_LCP_FAIL = 0x80bb,
    PS_SIP_PPP_LCP_FAIL = 0x80bc,
    PS_MIP_PPP_NCP_FAIL = 0x80bd,
    PS_SIP_PPP_NCP_FAIL = 0x80be,
    PS_A12_AUTH_FAIL = 0x80bf,
    PS_EAPAKA_AUTH_FAIL = 0x80c0,
    PS_MIP_ADMIN_PROHIB_FAIL = 0x80c1,
    PS_MULTI_TCH_FAIL = 0x80c2,
    PS_A12_LCP_TIMEOUT = 0x80c3,

    PS_BUSY = 0x80e0,
    PS_INVALID_PARAMETER = 0x80e1,

    PS_CMD_CONFLICT_C2K_AP_DEACT_RETRY_NEEDED = 0x80e2,

} AT_PS_ERROR;

#define MODIFY_CAUSE_PPP_RENEGOTIATION 99

#define ME_PDN_URC " ME PDN ACT "
#define ME_PDN_DEACT_URC " ME PDN DEACT "
#define NW_PDN_DEACT_URC " NW PDN DEACT "
#define NW_MODIFY_URC " NW MODIFY "

#define INVALID_CID -1

#define AUTHTYPE_NOT_SET    (-1)
#define AUTHTYPE_PAP          0
#define AUTHTYPE_CHAP         1
#define AUTHTYPE_NONE         2
#define AUTHTYPE_PAP_CHAP     3

#define IPV4        0
#define IPV6        1
#define IPV4V6      2

#define SETUP_DATA_PROTOCOL_IP      "IP"
#define SETUP_DATA_PROTOCOL_IPV6    "IPV6"
#define SETUP_DATA_PROTOCOL_IPV4V6  "IPV4V6"

#define REQUEST_DATA_ATTACH 1

#define NULL_IPV4_ADDRESS "0.0.0.0"
#define IPV6_PREFIX "FE80:0000:0000:0000:"
#define NULL_ADDR ""
#define MAX_NUM_DNS_ADDRESS_NUMBER 2
#define MAX_APN_NAME_LENGTH 128
#define AT_COMMAND_MEDIUM_LENGTH 256

// this property record default data sim's iccid
#define PROPERTY_RIL_DATA_ICCID "persist.vendor.radio.data.iccid"

// value 1: 4G on sim1
// value 2: 4G on sim2
#define PROPERTY_4G_SIM "persist.vendor.radio.simswitch"

// the IA record format is: iccid,apn
static const char RP_DATA_PROPERTY_IA[4][30] = {
    "vendor.ril.c2kirat.ia.sim1",
    "vendor.ril.c2kirat.ia.sim2",
    "vendor.ril.c2kirat.ia.sim3",
    "vendor.ril.c2kirat.ia.sim4",
};

// value 0: sim1 attaching or attached
// value 1: sim2 attaching or attached
#define RP_ATTACH_SIM "vendor.ril.attach.sim"


/* data connect state */
typedef enum {
    DATA_STATE_NEED_DEACT = -1,
    DATA_STATE_INACTIVE = 0,
    DATA_STATE_LINKDOWN = 1,
    DATA_STATE_ACTIVE = 2
} DATA_STATE;

typedef struct {
    int interfaceId;
    int cid;
    int active; //0: inactive, 1: link down, 2: active
    char apn[MAX_APN_NAME_LENGTH];
    char addressV4[MAX_IPV4_ADDRESS_LENGTH];
    char addressV6[MAX_IPV6_ADDRESS_LENGTH];
    char dnsV4[MAX_NUM_DNS_ADDRESS_NUMBER][MAX_IPV4_ADDRESS_LENGTH];
    char dnsV6[MAX_NUM_DNS_ADDRESS_NUMBER][MAX_IPV6_ADDRESS_LENGTH];
    int mtu;
    /// IMS @{
    int signalingFlag;
    int bearerId;
    char pcscf[MAX_PCSCF_NUMBER][MAX_IPV6_ADDRESS_LENGTH];
    /// @}
} PdnInfo;

typedef enum {
    DISABLE_CCMNI = 0,
    ENABLE_CCMNI = 1
} CCMNI_STATUS;

typedef enum {
    IRAT_NO_RETRY = 0,
    IRAT_NEED_RETRY = 1
} IRAT_RETRY_STATUS;

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
    RESUME_DATA_TRANSFER = 1
} IRAT_DATA_TRANSFER_STATE;

typedef enum {
    NO_SUGGESTED_TIME = -1,
    NO_SUGGESTION = 0,
    HAS_PDN_RETRY_TIME_SUGGESTION = 1
} MD_SUGGESTED_RETRY_TIME_STATUS;

// IRAT sync PDN support.
typedef struct {
    int interfaceId;
    int cid;
    char apn[MAX_APN_NAME_LENGTH];
    char addressV4[MAX_IPV4_ADDRESS_LENGTH];
    char addressV6[MAX_IPV6_ADDRESS_LENGTH];
    int pdnStatus;
} SyncPdnInfo;

/** global file funtions declaration **/
void rilDataInitialization(RILChannelCtx *pChannel);
void combineDataAttach(RILChannelCtx *p_channel);
int rilDataMain(int request, void *data, size_t datalen, RIL_Token t);
int rilDataUnsolicited(const char *s, const char *sms_pdu);

// Skip IRAT notification to MAL
extern bool skipIratUrcToMal(const char *urc);

void requestGetImsDataCallInfo(void *data, size_t datalen, RIL_Token t);
void requestReuseImsDataCall(void *data, size_t datalen, RIL_Token t);
int setUnsolResponseFilterLinkCapacityEstimate(bool enable, RIL_Token t);

#endif
