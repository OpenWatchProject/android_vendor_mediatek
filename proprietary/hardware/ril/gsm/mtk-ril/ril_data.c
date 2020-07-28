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

#include <telephony/mtk_ril.h>
#include <telephony/mtk_rs.h>
#include <libmtkrilutils.h>
#include <cutils/properties.h>
#include <cutils/sockets.h>
#include <ctype.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <log/log.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <pthread.h>
#include <alloca.h>
#include <getopt.h>
#include <termios.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <linux/route.h>
#include "mal.h"
#include "atchannels.h"
#include "at_tok.h"
#include "misc.h"
#include "ril_callbacks.h"
#include "hardware/ccci_intf.h"
#include "netutils/ifc.h"
#include "ifcutils/ifc.h"
#include "ratconfig.h"

#define IPV6_PREFIX "FE80:0000:0000:0000:"
#define NULL_ADDR ""

#define PKT_BITMASK_IMC_BMP_NONE                0x00000000
#define PKT_BITMASK_IMC_BMP_V4_ADDR             0x00000001
#define PKT_BITMASK_IMC_BMP_V6_ADDR             0x00000002
#define PKT_BITMASK_IMC_BMP_PROTOCOL            0x00000004
#define PKT_BITMASK_IMC_BMP_LOCAL_PORT_SINGLE   0x00000008
#define PKT_BITMASK_IMC_BMP_LOCAL_PORT_RANGE    0x00000010
#define PKT_BITMASK_IMC_BMP_REMOTE_PORT_SINGLE  0x00000020
#define PKT_BITMASK_IMC_BMP_REMOTE_PORT_RANGE   0x00000040
#define PKT_BITMASK_IMC_BMP_SPI                 0x00000080
#define PKT_BITMASK_IMC_BMP_TOS                 0x00000100
#define PKT_BITMASK_IMC_BMP_FLOW_LABEL          0x00000200

#define REACT_CCMNI 99
#define REQUEST_TYPE_NORMAL 0
#define REQUEST_TYPE_HANDOVER 3

#define IMS_CHANNEL_MASK 0x1F8070  // IMS channels: 5,6,7,16,17,18,19,20,21th channel

#define NW_PDN_DEACT " NW PDN DEACT "
#define ME_PDN_DEACT " ME PDN DEACT "
#define NW_DEACT " NW DEACT "
#define ME_DEACT " ME DEACT "
#define NW_REACT " NW REACT "

#define MAX_AT_CMD_LENGTH 512

pSetupDataCallFunc setupDataCallFunc[E_SETUP_DATA_CALL_FUNC_CNT] = \
    {requestSetupDataCallOverIPv6, requestSetupDataCallEmergency, requestSetupDataCallFallback, \
     requestSetupDataCallOverEpdg};

int sIsSkipDetachForSimRefresh = 0;

// for JPN IA
extern int bUbinSupport;  //support Universal BIN(WorldMode)
int bSetOpPropOneShot = 0;
static const struct timeval TIMEVAL_500_MS = {0,500e3};

PdnInfo* pdn_info[MAX_SIM_COUNT] = {NULL,NULL,NULL,NULL};
PdnInfo* pdn_info_wifi[MAX_SIM_COUNT] = {NULL,NULL,NULL,NULL};
PdnInfo* pdn_info_l2w[MAX_SIM_COUNT] = {NULL,NULL,NULL,NULL};

// Keep previously available cid
int prevAvailableCid[MAX_SIM_COUNT] = {INVALID_CID,INVALID_CID};

int max_pdn_support = DEFAULT_MAX_PDP_NUM;
int max_pdn_support_23G = DEFAULT_MAX_PDP_NUM;
int max_pdn_support_4G = 0;
int isCid0Support = 0;
int max_pdp_number = 0;
int pdnInfoSize = 0;
int attachApnSupport = -1; //0: not support, 1: partial support, 2: full support;
char s_l2p_value[MAX_L2P_VALUE_LENGTH] = {0};
char s_ccmni_ifname[MAX_CCMNI_IFNAME_LENGTH] = {0};
// OP12: Bar PDN with handover flag.
// The use of flag to check if need to pass the handover flag to modem.
int s_isBarApnHandover = 0;
int pdnFailCauseSupportVer = -1; // MD support version for last PDN fail cause
int pdnReasonSupportVer = -1; // MD support version for PDN cause reason
int pdnMdVersion = -1; // MD support version

// Fix: APN reset timing issue
static pthread_mutex_t setIaMutex[MAX_SIM_COUNT] = RIL_DATA_INIT_MUTEX;
// for IA roaming cache protection
static pthread_mutex_t iaRoamingMutex[MAX_SIM_COUNT] = RIL_DATA_INIT_MUTEX;

/// [C2K][IRAT] code start. {@
#define MAX_RETRY_EACTS 30
#define EACTS_RETRY_INTERVAL 500000

int nReactPdnCount = 0;
static int nDeactPdnCount = 0;
static int nReactSuccCount = 0;
static int nIratDeactCount = 0;
PDN_IRAT_ACTION nIratAction = IRAT_ACTION_UNKNOWN;
static PDN_IRAT_TYPE nIratType = IRAT_TYPE_UNKNOWN;
static SyncPdnInfo* pSyncPdnInfo = NULL;
static const struct timeval TIMEVAL_10 = {10,0};
/// [C2K][IRAT] code end. @}

// M: VDF MMS over ePDG @{
#define MAX_LENGTH_APN_TYPES_TO_RDS 128  //MAX string length of ApnTypes to RDS

#define APN_RAT_CELLULAR_ONLY 1;
#define APN_RAT_WIFI_ONLY 2;
#define APN_RAT_CELLULAR_WIFI 3;
// @}

/// M: Ims Data Framework {@
#define _IN6_IS_ULA(a)  \
    ((((a)->s6_addr[0] & 0xff) == 0xfc) || (((a)->s6_addr[0] & 0xff) == 0xfd))
#define HANDOVER_MAX_COUNT 65535
#define SIM_ID_INVALID (-1)
void *g_rds_conn_ptr = NULL;
HoPdnRemovePendingAction g_ho_pdn_remove_pending_act[MAX_SIM_COUNT] = {{0, 0}};
bool hoLtePdnSuccess[MAX_SIM_COUNT][WIFI_MAX_PDP_NUM] = {0};
int mHandOverDir[MAX_SIM_COUNT] = {E_HO_DIR_UNKNOWN};
typedef dm_req_t epdgDeactReq_t;
typedef rr_ddc_cnf_t epdgDeactRsp_t;
typedef dm_resp_setup_data_call_t queryRatRsp_t;

#define HO_STATUS_INVALID   (-1)
#define HO_START  (0)
#define HO_STOP (1)
#define REASON_RDS_DEACT_LTE_PDN (100)

static const char PROPERTY_FD_TIMER[4][50] = {
    "persist.vendor.radio.fd.off.counter",
    "persist.vendor.radio.fd.counter",
    "persist.vendor.radio.fd.off.r8.counter",
    "persist.vendor.radio.fd.r8.counter"
};

// prototype used mal_datamngr_data_call_info_req_t which is defined in "mal.h"
void makeDedicatedDataResponse(int concatenatedBearerNum, int activatedPdnNum,
        int *activatedCidList, mal_datamngr_data_call_info_req_t* response, RIL_SOCKET_ID rid);
void dumpMalDataResponse(mal_datamngr_data_call_info_req_t* response, const char* reason);
void freeMalDataResponse(mal_datamngr_data_call_info_req_t* response);
void responseUnsolDataCallRspToMal(RIL_Token t, const mal_datamngr_data_call_info_req_t *pRespData, size_t datalen);
void responseUnsolImsDataCallListToMal(RILChannelCtx* rilchnlctx,
        char *apn, int interfaceId, int rid);
void setRspEranType(int eran_type, MTK_RIL_Data_Call_Response_v11 *pRspData,
        mal_datamngr_data_call_info_req_t* responseToMal, RIL_SOCKET_ID rid);
void createMalDataResponse(int interfaceId, int cid, MTK_RIL_Data_Call_Response_v11* response,
        mal_datamngr_data_call_info_req_t* responseToMal, RIL_SOCKET_ID rid);
void initialMalDataCallResponse(mal_datamngr_data_call_info_req_t* responseToMal, int length);
void dumpApnConfig(rds_apn_cfg_req_t* apnConfig, int num);
void createDedicateDataResponse(int cid, MALRIL_Dedicate_Data_Call_Struct* response, RIL_SOCKET_ID rid);
void initialDedicateDataCallResponse(MALRIL_Dedicate_Data_Call_Struct* responses, int length);
void dumpDedicateDataResponse(MALRIL_Dedicate_Data_Call_Struct* response, const char* reason);
void querySuggestionTimeFromRds(const reqSetupConf_t *pReqSetupConfig,
    MTK_RIL_Data_Call_Response_v11* responseFromRds, RIL_Token t);
void notifyDedicatedBearerDeact(int rid, int cid);
/// @}
// prototype used ifreq in <linux/if.h>
void ril_data_set_ipv6addr(int s, struct ifreq *ifr, const char *addr);

void createDedicateDataResponse(int cid, MALRIL_Dedicate_Data_Call_Struct* response, RIL_SOCKET_ID rid);
void initialDedicateDataCallResponse(MALRIL_Dedicate_Data_Call_Struct* responses, int length);
void dumpDedicateDataResponse(MALRIL_Dedicate_Data_Call_Struct* response, const char* reason);

#define SIM_ID_INVALID (-1)

// M: Data Framework - CC 33 @{
#define MTK_CC33_SUPPORT "persist.vendor.data.cc33.support"
#define TELCEL_PDP_RETRY_PLMN "33402"
#define TELCEL_PDP_RETRY_PLMN2 "334020"
#define CLARO_PDP_RETRY_PLMN "732101"
int isCC33Support();
// M: Data Framework - CC 33 @}

// M: [General Operator] Data Framework - WWOP requirements: Telstra
#define TELSTRA_PDP_RETRY_PLMN "50501"

const char *LOG_TAG_STR = "RIL-DATA";

#ifdef LOG_TAG
    #undef LOG_TAG
#endif
#define LOG_TAG LOG_TAG_STR

int sock_fd[MAX_MPS_CCMNI_NUMBER] = {0};
int sock6_fd[MAX_MPS_CCMNI_NUMBER] = {0};

char TEMP_IA_PASSWORD[MAX_SIM_COUNT][MAX_PASSWORD_LEN] = {0};

extern int s_isSimSwitching;

/* Refer to system/core/libnetutils/ifc_utils.c */
extern int ifc_disable(const char *ifname);
extern int ifc_remove_default_route(const char *ifname);

void clearPdnInfo(PdnInfo* info);
void onPdnDeact(void* param);
void onPdnDeactResult(char* urc, RIL_SOCKET_ID rid);
int getAttachApnSupport();

static void clearIaCache(int iaSimId);

void onReAttachForSettingAttachApn(void* param);
void onAttachApnReset(void* param);

void onBarApn(char* urc, RIL_SOCKET_ID rid);

extern int isOp12Support();
extern int isOp12MccMnc(char *mccmnc);

// FIXME: It's better to extend definPdnCtx for a extra para roamingProtocol to make it
//             into common flow.
char* g_iaRoamingProtocol[MAX_SIM_COUNT] = {NULL, NULL, NULL, NULL}; // MAX_SIM_COUNT=4
// APN control mode for IMS handover
int setApnControlMode(RIL_SOCKET_ID rid);
#define VDF_ITALY "22210"
// M: OP17 IA
#define DOCOMO_DOMESTIC_PLMN "440xx"
// IA end

int isRequestToDeactDataCallByReasonFromMAL(int reason);
int isRequestToDeactDataCallByReasonFromFWK(int reason);
void onDualApnCheckNetworkChange(void* param);


//M:  [C2K][IRAT code start. {@
static void onNwDetach(void* param);
//M:  [C2K][IRAT] code end. @}

//Fucntion prototype
void ril_data_ioctl_init(int index);
void ril_data_setflags(int s, struct ifreq *ifr, int set, int clr);
void ril_data_setaddr(int s, struct ifreq *ifr, const char *addr);

// to check if LTE is working for CDMA
int isLteDataOnCdma(RIL_SOCKET_ID rid);

// FastDormancy
void requestSetFdMode(void * data, size_t datalen, RIL_Token t);

/// M:  SetDataProfiles @{
void syncApnTableToMdAndRds(void* data, size_t datalen, RIL_Token t);
int syncApnTableToMd(void* data, size_t datalen, RIL_Token t);
int syncApnTableToRds(void* data, size_t datalen, RIL_Token t);
int getApnProfileId(int apnType);
bool bitmaskHasTech(int bearerBitmask, int radioTech);
/// @}

//Global variables/strcuture
static int disableFlag = 1;
int gprs_failure_cause = 0;

extern int gcf_test_mode;
extern int s_md_off;
extern GEMINI_MODE getGeminiMode();
extern int queryRadioState(RILSubSystemId subsystem, RIL_SOCKET_ID rid);

static const struct timeval TIMEVAL_0 = {0,0};
static const struct timeval TIMEVAL_300_ms = {0,300e3};

#define PROPERTY_DATA_NETWORK_TYPE "vendor.gsm.network.type"
static const char PROPERTY_NW_MCCMNC[4][40] = {
    "vendor.ril.nw.operator.mccmnc.1",
    "vendor.ril.nw.operator.mccmnc.2",
    "vendor.ril.nw.operator.mccmnc.3",
    "vendor.ril.nw.operator.mccmnc.4",
};
#define DEFAULT_NODATA_APN "NODATA"

typedef struct {
    char apn[PROPERTY_VALUE_MAX];
    RILChannelCtx* pDataChannel;
} OnReAttachInfo;

static int isFallbackPdpRetryRunning(RIL_SOCKET_ID rid, const char* apn);
static int isFallbackPdpNeeded(RIL_SOCKET_ID rid, const char* apn, int protocol);

bool isKeepDefaultPdn(int interfaceId, RIL_SOCKET_ID rid, char* apn);
bool isDefaultPdnReuseIa(RIL_SOCKET_ID rid);

void ril_data_ioctl_init(int index)
{
    if (sock_fd[index] > 0) {
        close(sock_fd[index]);
    }
    sock_fd[index] = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd[index] < 0) {
        LOGE("Couldn't create IP socket: errno=%d", errno);
    } else {
        LOGD("Allocate sock_fd=%d, for cid=%d", sock_fd[index], index+1);
    }
#ifdef INET6
    if (sock6_fd[index] > 0) {
        close(sock6_fd[index]);
    }
    sock6_fd[index] = socket(AF_INET6, SOCK_DGRAM, 0);
    if (sock6_fd[index] < 0) {
        sock6_fd[index] = -errno;    /* save errno for later */
        LOGE("Couldn't create IPv6 socket: errno=%d", errno);
    } else {
        LOGD("Allocate sock6_fd=%d, for cid=%d", sock6_fd[index], index+1);
    }
#endif
}

/* For setting IFF_UP: ril_data_setflags(s, &ifr, IFF_UP, 0) */
/* For setting IFF_DOWN: ril_data_setflags(s, &ifr, 0, IFF_UP) */
void ril_data_setflags(int s, struct ifreq *ifr, int set, int clr)
{
    if (ioctl(s, SIOCGIFFLAGS, ifr) < 0) {
        goto error_SIOCGIFFLAGS;
    }

    ifr->ifr_flags = (ifr->ifr_flags & (~clr)) | set;
    if (ioctl(s, SIOCSIFFLAGS, ifr) < 0) {
        goto error_SIOCSIFFLAGS;
    }

    return;

error_SIOCGIFFLAGS:
    LOGE("ril_data_setflags: set SIOCGIFFLAGS Error!");
    return;

error_SIOCSIFFLAGS:
    LOGE("ril_data_setflags: set SIOCSIFFLAGS Error!");
    return;
}

inline void ril_data_init_sockaddr_in(struct sockaddr_in *sin, const char *addr)
{
    sin->sin_family = AF_INET;
    sin->sin_port = 0;
    sin->sin_addr.s_addr = inet_addr(addr);
}

void ril_data_setaddr(int s, struct ifreq *ifr, const char *addr)
{
    int ret = 0;

    ril_data_init_sockaddr_in((struct sockaddr_in *) &ifr->ifr_addr, addr);
    ret = ioctl(s, SIOCSIFADDR, ifr);
    if (ret < 0)
        goto terminate;
    return;
terminate:
    LOGE("ril_data_setaddr: error in set SIOCSIFADDR:%d - %d:%s", ret, errno, strerror(errno));
    return;
}

void initialDataCallResponse(MTK_RIL_Data_Call_Response_v11* responses, int length) {
    int i = 0;
    for (i=0; i<length; i++) {
        memset(&responses[i], 0, sizeof(MTK_RIL_Data_Call_Response_v11));
        responses[i].status = PDP_FAIL_ERROR_UNSPECIFIED;
        responses[i].cid = INVALID_CID;
    }
}

int getAuthType(const char* authTypeStr) {
    int  authType = atoi(authTypeStr);
    //Application 0->none, 1->PAP, 2->CHAP, 3->PAP/CHAP;
    //Modem 0->PAP, 1->CHAP, 2->NONE, 3->PAP/CHAP;
    switch (authType) {
        case 0:
            return AUTHTYPE_NONE;
        case 1:
            return AUTHTYPE_PAP;
        case 2:
            return AUTHTYPE_CHAP;
        case 3:
            return AUTHTYPE_CHAP;
        default:
            return AUTHTYPE_NOT_SET;
    }
}

int getLastDataCallFailCause()
{
    if (gprs_failure_cause == SM_OPERATOR_BARRED ||
            gprs_failure_cause == SM_MBMS_CAPABILITIES_INSUFFICIENT ||
            gprs_failure_cause == SM_LLC_SNDCP_FAILURE ||
            gprs_failure_cause == SM_INSUFFICIENT_RESOURCES ||
            gprs_failure_cause == SM_MISSING_UNKNOWN_APN ||
            gprs_failure_cause == SM_UNKNOWN_PDP_ADDRESS_TYPE ||
            gprs_failure_cause == SM_USER_AUTHENTICATION_FAILED ||
            gprs_failure_cause == SM_ACTIVATION_REJECT_GGSN ||
            gprs_failure_cause == SM_ACTIVATION_REJECT_UNSPECIFIED ||
            gprs_failure_cause == SM_SERVICE_OPTION_NOT_SUPPORTED ||
            gprs_failure_cause == SM_SERVICE_OPTION_NOT_SUBSCRIBED ||
            gprs_failure_cause == SM_SERVICE_OPTION_OUT_OF_ORDER ||
            gprs_failure_cause == SM_NSAPI_IN_USE ||
            gprs_failure_cause == SM_REGULAR_DEACTIVATION ||
            gprs_failure_cause == SM_QOS_NOT_ACCEPTED ||
            gprs_failure_cause == SM_NETWORK_FAILURE ||
            gprs_failure_cause == SM_REACTIVATION_REQUESTED ||
            gprs_failure_cause == SM_FEATURE_NOT_SUPPORTED ||
            gprs_failure_cause == SM_SEMANTIC_ERROR_IN_TFT ||
            gprs_failure_cause == SM_SYNTACTICAL_ERROR_IN_TFT ||
            gprs_failure_cause == SM_UNKNOWN_PDP_CONTEXT ||
            gprs_failure_cause == SM_SEMANTIC_ERROR_IN_PACKET_FILTER ||
            gprs_failure_cause == SM_SYNTACTICAL_ERROR_IN_PACKET_FILTER ||
            gprs_failure_cause == SM_PDP_CONTEXT_WITHOU_TFT_ALREADY_ACTIVATED ||
            gprs_failure_cause == SM_MULTICAST_GROUP_MEMBERSHIP_TIMEOUT ||
            gprs_failure_cause == SM_BCM_VIOLATION ||
            gprs_failure_cause == SM_LAST_PDN_DISC_NOT_ALLOWED ||
            gprs_failure_cause == SM_ONLY_IPV4_ALLOWED ||
            gprs_failure_cause == SM_ONLY_IPV6_ALLOWED ||
            gprs_failure_cause == SM_ONLY_SINGLE_BEARER_ALLOWED ||
            gprs_failure_cause == ESM_INFORMATION_NOT_RECEIVED ||
            gprs_failure_cause == SM_PDN_CONNECTION_NOT_EXIST ||
            gprs_failure_cause == SM_MULTIPLE_PDN_APN_NOT_ALLOWED ||
            gprs_failure_cause == SM_COLLISION_WITH_NW_INITIATED_REQUEST ||
            gprs_failure_cause == ESM_UNSUPPORTED_QCI_VALUE ||
            gprs_failure_cause == SM_BEARER_HANDLING_NOT_SUPPORT ||
            gprs_failure_cause == SM_MAX_PDP_NUMBER_REACHED ||
            gprs_failure_cause == SM_APN_NOT_SUPPORT_IN_RAT_PLMN ||
            gprs_failure_cause == SM_INVALID_TRANSACTION_ID_VALUE ||
            gprs_failure_cause == SM_SEMENTICALLY_INCORRECT_MESSAGE ||
            gprs_failure_cause == SM_INVALID_MANDATORY_INFO ||
            gprs_failure_cause == SM_MESSAGE_TYPE_NONEXIST_NOT_IMPLEMENTED ||
            gprs_failure_cause == SM_MESSAGE_TYPE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE ||
            gprs_failure_cause == SM_INFO_ELEMENT_NONEXIST_NOT_IMPLEMENTED ||
            gprs_failure_cause == SM_CONDITIONAL_IE_ERROR ||
            gprs_failure_cause == SM_MESSAGE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE ||
            gprs_failure_cause == SM_PROTOCOL_ERROR ||
            gprs_failure_cause == SM_APN_RESTRICTION_VALUE_INCOMPATIBLE_WITH_PDP_CONTEXT ||
            gprs_failure_cause == PDP_FAIL_FALLBACK_RETRY
            ) {
        return gprs_failure_cause;
    } else if (gprs_failure_cause == 0x0E0F){
        //SM_LOCAL_REJECT_ACT_REQ_DUE_TO_GPRS_ATTACH_FAILURE
        //Modem will retry attach
        LOGI("getLastDataCallFailCause(): GMM error %X", gprs_failure_cause);
    } else if (gprs_failure_cause == 0x150A || gprs_failure_cause == 0x1402) {
        LOGI("ESM_TIMER_TIMOUT(0x150A) or REJECT_IMS_PDN_BLOCK(0x1402): %X", gprs_failure_cause);
    } else {
       gprs_failure_cause = 14;  //FailCause.Unknown
    }

    return gprs_failure_cause;
}

/* Change name from requestOrSendPDPContextList to requestOrSendDataCallList */
static void requestOrSendDataCallList(RIL_Token *t, RIL_SOCKET_ID rid)
{
    /* Because the RIL_Token* t may be NULL passed due to receive URC: Only t is NULL, 2nd parameter rid is used */
    RILChannelCtx* rilchnlctx = NULL;
    if (t != NULL) {
        rilchnlctx = getRILChannelCtxFromToken(*t);
    } else {
        rilchnlctx = getChannelCtxbyProxy();
    }
    requestOrSendDataCallListIpv6(rilchnlctx, t, rid);
}

/* 27:RIL_REQUEST_SETUP_DATA_CALL/RIL_REQUEST_SETUP_DEFAULT_PDP */
/* ril_commands.h : {RIL_REQUEST_SETUP_DATA_CALL, dispatchDataCall, responseSetupDataCall} */
/* ril_commands.h : {RIL_REQUEST_SETUP_DEFAULT_PDP, dispatchStrings, responseStrings, RIL_CMD_PROXY_3} */
/* Change name from requestSetupDefaultPDP to requestSetupDataCall */
void requestSetupDataCall(void * data, size_t datalen, RIL_Token t)
{
    /* In GSM with CDMA version: DOUNT - data[0] is radioType(GSM/UMTS or CDMA), data[1] is profile,
     * data[2] is apn, data[3] is username, data[4] is passwd, data[5] is authType (added by Android2.1)
     * data[6] is cid field added by mtk for Multiple PDP contexts setup support 2010-04
     */
    int requestParamNumber = (datalen/sizeof(char*));
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    LOGD("[%d]requestSetupData with datalen=%d and parameter number=%d",
            rid, (int) datalen, requestParamNumber);
    reqSetupConf_t reqSetupConf = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    reqSetupConf.radioType = ((const char **)data)[0];
    reqSetupConf.profile = ((const char **)data)[1];
    reqSetupConf.requestedApn = ((const char **)data)[2];
    reqSetupConf.username = ((const char **)data)[3];
    reqSetupConf.password = ((const char **)data)[4];
    reqSetupConf.authType = getAuthType(((const char **)data)[5]);
    reqSetupConf.protocol = get_protocol_type(((const char **)data)[6]);

    reqSetupConf.roamingProtocol = ((const char **)data)[7];
    reqSetupConf.supportedApnTypesBitmap = atoi(((const char **)data)[8]);
    reqSetupConf.bearerBitmap = atoi(((const char **)data)[9]);
    reqSetupConf.modemCognitive = atoi(((const char **)data)[10]);
    reqSetupConf.mtu = atoi(((const char **)data)[11]);
    reqSetupConf.mvnoType = atoi(((const char **)data)[12]);
    reqSetupConf.mvnoMatchData = ((const char **)data)[13];
    reqSetupConf.roamingAllowed = atoi(((const char **)data)[14]);
    reqSetupConf.interfaceId = atoi(((const char **)data)[15]) - 1;

    int isFB = isFallbackPdpNeeded(rid, reqSetupConf.requestedApn, reqSetupConf.protocol);
    int setupFuncIdx = E_SETUP_DATA_CALL_FUNC_ERR;

    /// M: Ims Data Framework {@
    reqSetupConf.availableCid = INVALID_CID;
    reqSetupConf.retryCount = 1;
    RIL_Default_Bearer_VA_Config_Struct defaultBearerVaConfig;
    memset(&defaultBearerVaConfig, 0, sizeof(RIL_Default_Bearer_VA_Config_Struct));

    RIL_Errno rilErrno = RIL_E_GENERIC_FAILURE;
    dumpReqSetupConfig(&reqSetupConf);
    if (getImsParamInfo(rid, reqSetupConf.interfaceId, &defaultBearerVaConfig) != 0) {
            LOGE("get ims param failed!!");
            goto error;
    } else {    // not IMS/EIMS will get the config fill of 0
        dumpDefaultBearerConfig(&defaultBearerVaConfig);
        if (isEpdgSupport()) {
            int nRet = queryEpdgRat(rid, &reqSetupConf, &defaultBearerVaConfig);
            switch (nRet) {
            case PDN_SETUP_THRU_MOBILE:
                break;
            case PDN_SETUP_THRU_WIFI: {
                    reqSetupConf.availableCid = getAvailableWifiCid(rid);
                    if (reqSetupConf.availableCid == INVALID_CID) {
                        LOGE("error not enough CID to use");
                        goto error;
                    } else {
                        LOGD("avaiable cid=%d for wifi", reqSetupConf.availableCid);
                    }
                }
                break;
            case PDN_SETUP_THRU_ERR:
            default:
                if ((NULL != reqSetupConf.requestedApn) &&
                    (NULL != strcasestr(reqSetupConf.requestedApn, "ims"))) {
                    rilErrno = RIL_E_REQUEST_NOT_SUPPORTED;
                }
                LOGE("queryEpdgRat error!!");
                goto error;
            }
        }
    }

    if (reqSetupConf.availableCid != INVALID_CID) {
        setupFuncIdx = E_SETUP_DATA_CALL_OVER_EPDG;
    } else if (defaultBearerVaConfig.emergency_ind == 1) {  // Emergency
        setupFuncIdx = E_SETUP_DATA_CALL_EMERGENCY;
    } else if (isFB) {
        setupFuncIdx = E_SETUP_DATA_CALL_FALLBACK;
    } else {
        setupFuncIdx = E_SETUP_DATA_CALL_OVER_IPV6;
    }
    /// @}

    setupDataCallFunc[setupFuncIdx] (&reqSetupConf, (void *) &defaultBearerVaConfig, t);

finish:
    FREEIF(reqSetupConf.pQueryRatRsp);
    return;

error:
    LOGE("[%s] error E", __FUNCTION__);
    FREEIF(reqSetupConf.pQueryRatRsp);
    /// M: Ims Data Framework {@
    if (isEpdgSupport()) {
        if (reqSetupConf.retryCount == PDN_SETUP_THRU_MAX) { //only wifi rat and setupData fail.
            MTK_RIL_Data_Call_Response_v11* response = (MTK_RIL_Data_Call_Response_v11*)
                calloc(1, sizeof(MTK_RIL_Data_Call_Response_v11));
            querySuggestionTimeFromRds(&reqSetupConf, response, t);
            RIL_onRequestComplete(t, rilErrno, response,
                    sizeof(MTK_RIL_Data_Call_Response_v11));
            freeDataResponse(response);
            free(response);
            return;
        }
    }
    /// @}
    RIL_onRequestComplete(t, rilErrno, NULL, 0);
}

void requestDeactiveAllDataCall(void * data, size_t datalen, RIL_Token t)
{
    UNUSED(data);
    UNUSED(datalen);

    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    LOGD("[%s] rid = %d", __FUNCTION__, rid);

    int i = 0;
    int err;
    int interfaceId = INVALID_CID;
    int lastPdnCid = INVALID_CID;
    int needHandleLastPdn = 0;
    int lastPdnState = DATA_STATE_INACTIVE;
    int isEmergency = 0;
    int pdnReason = NO_CAUSE;

    for(i = 0; i < pdnInfoSize; i++) {
        interfaceId = pdn_info[rid][i].interfaceId;
        if (interfaceId == INVALID_CID ||
                (interfaceId != INVALID_CID && isImsInterfaceId(interfaceId))) {
            if (interfaceId != INVALID_CID) {
                LOGD("[%s] skip ims related interfaceId%d, cid%d!",
                        __FUNCTION__, interfaceId, pdn_info[rid][i].cid);
            }
            continue;
        }

        if (!pdn_info[rid][i].isDedicateBearer) {
            err = deactivateDataCall(pdn_info[rid][i].cid, DATA_CHANNEL_CTX);

            switch (err) {
            case CME_SUCCESS:
                break;
            case CME_L4C_CONTEXT_CONFLICT_DEACT_ALREADY_DEACTIVATED:
                LOGD("[%s] deactivateDataCall cid%d already deactivated",
                        __FUNCTION__, pdn_info[rid][i].cid);
                break;
            case CME_LAST_PDN_NOT_ALLOW_LR11:
                if(pdnFailCauseSupportVer < MD_LR11) continue;
            case CME_LAST_PDN_NOT_ALLOW:
                if (needHandleLastPdn == 0) {
                    needHandleLastPdn = 1;
                    lastPdnState = pdn_info[rid][i].active;
                    isEmergency = pdn_info[rid][i].isEmergency;
                    lastPdnCid = pdn_info[rid][i].cid;
                    pdnReason = pdn_info[rid][i].reason;
                    LOGD("[%s] deactivateDataCall cid%d is the last PDN, state: %d",
                            __FUNCTION__, lastPdnCid, lastPdnState);
                }
                break;
            case CME_LOCAL_REJECTED_DUE_TO_PS_SWITCH:
                // in this case, we make the deactivation fail
                // after switch, framework will check data call list and to deactivation again
                LOGD("[%s] receive LOCAL_REJECTED_DUE_TO_PS_SWITCH", __FUNCTION__);
                continue;
                /// [C2K][IRAT] new fail cause happened during IRAT.
            case CME_L4C_CMD_CONFLICT_C2K_AP_DEACT_RETRY_NEEDED:
                pdn_info[rid][i].active = DATA_STATE_NEED_DEACT;
                LOGD("[RILData_GSM_IRAT] receive L4C_CMD_CONFLICT_C2K_AP_DEACT_RETRY_NEEDED.");
                break;
            default:
                if (s_isSimSwitching) {
                    LOGD("[%s] failed due to SIM switching, err = %d", __FUNCTION__, err);
                    break;
                }
                continue;
            };

            configureNetworkInterface(interfaceId, DISABLE_CCMNI, rid);

            if (pdn_info[rid][i].active != DATA_STATE_NEED_DEACT) {
                // clear only default bearer
                clearPdnInfo(&pdn_info[rid][i]);
            }
        }
    }

    if (needHandleLastPdn) {
        if (lastPdnState != DATA_STATE_LINKDOWN) {
            if (0 != handleLastPdnDeactivation(isEmergency, lastPdnCid, DATA_CHANNEL_CTX)) {
                pdn_info[rid][lastPdnCid].active = DATA_STATE_NEED_DEACT;
                goto error;
            }
        } else {
            LOGD("[%s] last pdn alread linkdown", __FUNCTION__);
        }
        pdn_info[rid][lastPdnCid].active = DATA_STATE_LINKDOWN;
        pdn_info[rid][lastPdnCid].cid = lastPdnCid;
        pdn_info[rid][lastPdnCid].primaryCid = lastPdnCid;
        pdn_info[rid][lastPdnCid].reason = pdnReason;
    }

    //response deactivation result first then do re-attach
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;

error:
    LOGE("[%s] error E", __FUNCTION__);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

void requestDeactiveDataCall(void * data, size_t datalen, RIL_Token t)
{
    UNUSED(datalen);
    const char *cid;
    int err;
    ATResponse *p_response = NULL;
    int i = 0, lastPdnCid = INVALID_CID;
    int interfaceId = atoi(((const char **)data)[0]);
    int reason = atoi(((const char **)data)[1]);
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    LOGD("[%d][%s] interfaceId=%d, reason: %d X", rid, __FUNCTION__, interfaceId, reason);

    int needHandleLastPdn = 0;
    int lastPdnState = DATA_STATE_INACTIVE;
    int isEmergency = 0;
    int pdnReason = NO_CAUSE;
    PdnInfo *pdnInfo = NULL;

    // AT+CGACT=<state>,<cid>;  <state>:0-deactivate;1-activate
    for(i = 0; i < pdnInfoSize; i++) {
        if (pdn_info[rid][i].interfaceId == interfaceId
                && !pdn_info[rid][i].isDedicateBearer) {
            pdnInfo = &pdn_info[rid][i];

            /// M: Ims Data Framework {@
            // reason 100 means that RDS disconnect LTE pdn, no need to ask RDS again
            if (reason != REASON_RDS_DEACT_LTE_PDN &&
                    0 != deactivatePdnThruEpdg(rid, interfaceId, reason)) {
                LOGD("no deactivate thru mobile!!");
                continue;
            }
            /// @}

            if (isKeepDefaultPdn(interfaceId, rid, pdnInfo->apn)
                    && !isRequestToDeactDataCallByReasonFromFWK(reason)) {
                needHandleLastPdn = 1;
                lastPdnState = pdnInfo->active;
                isEmergency = pdnInfo->isEmergency;
                lastPdnCid = pdnInfo->cid;
                pdnReason = pdnInfo->reason;
                LOGI("[%s] keep cid%d, state: %d", __FUNCTION__, i, lastPdnState);
                break;
            }

            if (isRequestToDeactDataCallByReasonFromFWK(reason)) {
                err = deactivateDataCallByReason(pdnInfo->cid, reason - FWK_EGACT_CAUSE_DEACT_BASE,
                        DATA_CHANNEL_CTX);
            } else {
                err = deactivateDataCall(pdnInfo->cid, DATA_CHANNEL_CTX);
            }

            switch (err) {
            case CME_SUCCESS:
                break;
            case CME_L4C_CONTEXT_CONFLICT_DEACT_ALREADY_DEACTIVATED:
                LOGI("[%s] deactivateDataCall cid%d already deactivated", __FUNCTION__, i);
                break;
            case CME_LAST_PDN_NOT_ALLOW_LR11:
                if(pdnFailCauseSupportVer < MD_LR11) goto error;
            case CME_LAST_PDN_NOT_ALLOW:
                if (needHandleLastPdn == 0) {
                    needHandleLastPdn = 1;
                    lastPdnState = pdnInfo->active;
                    isEmergency = pdnInfo->isEmergency;
                    lastPdnCid = pdnInfo->cid;
                    pdnReason = pdnInfo->reason;
                    LOGI("[%s] deactivateDataCall cid%d is the last PDN, state: %d", __FUNCTION__, i, lastPdnState);
                }
                break;
            case CME_LOCAL_REJECTED_DUE_TO_PS_SWITCH:
                // in this case, we make the deactivation fail
                // after switch, framework will check data call list and to deactivation again
                LOGE("[%s] receive LOCAL_REJECTED_DUE_TO_PS_SWITCH", __FUNCTION__);
                goto error;

                /// [C2K][IRAT] new fail cause happened during IRAT.
            case CME_L4C_CMD_CONFLICT_C2K_AP_DEACT_RETRY_NEEDED:
                pdnInfo->active = DATA_STATE_NEED_DEACT;
                LOGI("[RILData_GSM_IRAT] receive L4C_CMD_CONFLICT_C2K_AP_DEACT_RETRY_NEEDED.");
                break;

            default:
                if (s_isSimSwitching) {
                    LOGI("[%s] failed due to SIM switching, err = %d", __FUNCTION__, err);
                    break;
                }
                goto error;
            };
        }
    }

    /// M: Ims Data Framework {@
    if (reason != REASON_RDS_DEACT_LTE_PDN) {
        configureNetworkInterface(interfaceId, DISABLE_CCMNI, rid);
    }

    // reason 100 means that RDS disconnect LTE pdn, no need to ask RDS again
    if (pdnInfo == NULL && reason != REASON_RDS_DEACT_LTE_PDN) {
        for (i = 0; i < WIFI_MAX_PDP_NUM; i++) {
            if (pdn_info_wifi[rid][i].interfaceId == interfaceId) {
                LOGD("[%d] deactivate and clear wifi pdn with interface %d", rid, interfaceId);
                pdnInfo = &pdn_info_wifi[rid][i];
                deactivatePdnThruEpdg(rid, interfaceId, reason);
                clearPdnInfo(pdnInfo);
            }
        }
    }
    /// @}

    for (i = 0; i < pdnInfoSize; i++) {
        if (pdn_info[rid][i].interfaceId == interfaceId
                && !pdn_info[rid][i].isDedicateBearer
                && pdn_info[rid][i].active != DATA_STATE_NEED_DEACT) {
            // clear only default bearer
            clearPdnInfo(&pdn_info[rid][i]);
        }
    }

    if (needHandleLastPdn) {
        RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));

        // notify rilproxy the deactivation failure if data setting off
        if (getDataEnable(rid) == 0) {
            int deactInd[1];
            // [0]: <cause>
            deactInd[0] = CME_LAST_PDN_NOT_ALLOW;
            RIL_UNSOL_RESPONSE(RIL_LOCAL_GSM_UNSOL_PDN_DEACT_FAILURE_IND,
                    deactInd, sizeof(deactInd), rid);
        }
        if (lastPdnState != DATA_STATE_LINKDOWN) {
            if (0 != handleLastPdnDeactivation(isEmergency, lastPdnCid, DATA_CHANNEL_CTX)) {
                pdn_info[rid][lastPdnCid].active = DATA_STATE_NEED_DEACT;
                goto error;
            }
        } else {
            LOGD("[%s] last pdn alread linkdown", __FUNCTION__);
        }

        pdn_info[rid][lastPdnCid].active = DATA_STATE_LINKDOWN;
        pdn_info[rid][lastPdnCid].cid = lastPdnCid;
        pdn_info[rid][lastPdnCid].primaryCid = lastPdnCid;
        pdn_info[rid][lastPdnCid].reason = pdnReason;
    }

    //response deactivation result first then do re-attach
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    AT_RSP_FREE(p_response);
    return;
error:
    LOGE("[%s] error E", __FUNCTION__);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    AT_RSP_FREE(p_response);
}

void requestLastDataCallFailCause(void * data, size_t datalen, RIL_Token t)
{
    UNUSED(data);
    UNUSED(datalen);
    int lastPdpFailCause = 14;
    lastPdpFailCause = getLastDataCallFailCause();
    RIL_onRequestComplete(t, RIL_E_SUCCESS, &lastPdpFailCause, sizeof(lastPdpFailCause));
}

void requestDataCallList(void * data, size_t datalen, RIL_Token t)
{
    UNUSED(data);
    UNUSED(datalen);
    requestOrSendDataCallList(&t, getRILIdByChannelCtx(getRILChannelCtxFromToken(t)));
}

void requestSetInitialAttachApn(void * data, size_t datalen __unused, RIL_Token t)
{
    RIL_InitialAttachApn_v15* param = data;
    int isApnDiff = 1;
    int isCurIaInvalid = 0;
    char operatorNumeric[PROPERTY_VALUE_MAX] = {0};
    RILChannelCtx* pChannel = getRILChannelCtxFromToken(t);
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(pChannel);

    // Fix: APN reset timing issue
    pthread_mutex_lock(&setIaMutex[rid]);
    if (param->apn == NULL) {
        param->apn = calloc(1, sizeof(char));
    }
    if (param->username == NULL) {
        param->username = calloc(1, sizeof(char));
    }
    if (param->password == NULL) {
        param->password = calloc(1, sizeof(char));
    }
    LOGD("IA: requestSetInitialAttachApn [apn=%s, protocol=%s, roamingProtocol = %s, \
auth_type=%d, username=%s, password=%s, isCid0Support=%d, skipDetach=%d, canHandleIms=%d",
            param->apn, param->protocol, param->roamingProtocol, param->authtype, param->username,
            "XXXXX", isCid0Support, sIsSkipDetachForSimRefresh, param->canHandleIms);
    pthread_mutex_lock(&iaRoamingMutex[rid]);
    // OP12: save roaming protocol to pass to MD after.
    asprintf(&g_iaRoamingProtocol[rid], "%s", param->roamingProtocol);
    pthread_mutex_unlock(&iaRoamingMutex[rid]);
    if (setApnControlMode(rid) == 0) {
        // Currently, we don't change the attach APN back to avoid re-attach behaviour
        // when IMS handover end. If trigger re-attach, it might resulting lab test
        // fail for unexpected detach
        LOGD("IA: requestSetInitialAttachApn return when IMS handover end");
        // Fix: APN reset timing issue
        pthread_mutex_unlock(&setIaMutex[rid]);
        return;
    }

    // Multi-PS attach Start
    GEMINI_MODE mode = getGeminiMode();
    if (mode != GEMINI_MODE_L_AND_L) {
        if (rid + 1 != (unsigned int) RIL_get3GSIM()) {
            LOGE("IA: requestSetInitialAttachApn but not main socket");
            // return success to pass VTS case
            goto finish;
        }
    // Multi-PS attach End
    } else if (getAttachApnSupport() == ATTACH_APN_NOT_SUPPORT) {
        LOGD("IA: requestSetInitialAttachApn but attach apn is not support");
        goto error;
    }

    getMSimProperty(rid, PROPERTY_SIM_OPERATOR_NUMERIC, operatorNumeric);

    if (isCid0Support) {
        int triggerPsAttach = 1;
        char* apn = "";
        int isCurIaInvalid = 0;
        char iccid[PROPERTY_VALUE_MAX] = {0};
        char iaProperty[PROPERTY_VALUE_MAX*2] = {0};
        char iaPwdFlag[PROPERTY_VALUE_MAX] = {0};
        int canHandleIms = param->canHandleIms;

        if (getSimCount() > 1) {
            if (mode != GEMINI_MODE_L_AND_L) {
                //This is for gemini load that attach apn should be set on data SIM
                int dataSim = getDefaultDataSim();
                int simId3G = RIL_get3GSIM();

                if (dataSim == 0) {
                    LOGD("IA: requestSetInitialAttachApn data disabled, trigger attach on 4G SIM");
                } else if (simId3G != dataSim) {
                    LOGD("IA: requestSetInitialAttachApn data not enabled on 4G SIM, defind CID0\
 and not to trigger PS re-attach");
                    triggerPsAttach = 0;
                }
            }
        }
        int iaSimId = rid;

        property_get(PROPERTY_ICCID_SIM[rid], iccid, "");
        getIaCache(iaSimId, iaProperty);
        getMSimProperty(iaSimId, PROPERTY_IA_PASSWORD_FLAG, iaPwdFlag);

        if (0 == strcmp(operatorNumeric, VDF_ITALY)) {
            //Only VDF Italy raise P-CSCF discovery flag
            canHandleIms = 1;
            LOGD("IA: requestSetInitialAttachApn VDF Italy raise P-CSCF discovery flag");
        }

        if (strlen(iaProperty) != 0) {
            //here we need to check if current IA property is different than what we want to set
            //if different, need to clear cache and do PS detach
            //this scenario happens when attach APN is changed
            char iaParameter[PROPERTY_VALUE_MAX*2] = {0};
            if (strlen(param->password) == 0) {
                snprintf(iaParameter, PROPERTY_VALUE_MAX*2, "%s,%s,%d,%s,%d,%s,%s", iccid,
                        param->protocol, param->authtype, param->username,
                        canHandleIms, param->roamingProtocol, param->apn);
            } else {
                //when password is set, iccid is not recorded
                //so we do not need to compare iccid
                snprintf(iaParameter, PROPERTY_VALUE_MAX*2, "%s,%s,%d,%s,%d,%s,%s", "",
                        param->protocol, param->authtype, param->username,
                        canHandleIms, param->roamingProtocol, param->apn);
            }
            // special handle for VZWIMS
            if (isOp12Support()) {
                if (strcmp(param->apn, "VZWIMS") == 0 || strcmp(param->apn, "IMS") == 0) {
                    // VZWIMS and IMS are the same apn and we don't do reattach again
                    // check if the apn is changed
                    // check VZWIMS
                    if (strlen(param->password) == 0) {
                        snprintf(iaParameter, sizeof(iaParameter), "%s,%s,%d,%s,%d,%s", iccid, param->protocol,
                                param->authtype, param->username, param->canHandleIms, "VZWIMS");
                    } else {
                        //when password is set, iccid is not recorded
                        //so we do not need to compare iccid
                        snprintf(iaParameter, sizeof(iaParameter), "%s,%s,%d,%s,%d,%s", "", param->protocol, param->authtype,
                                param->username, param->canHandleIms, "VZWIMS");
                    }
                    if (strcmp(iaParameter, iaProperty) == 0) {
                        triggerPsAttach = 0;
                        LOGD("IA: requestSetInitialAttachApn, IA cache is equal to VZWIMS");
                    }
                    // check IMS
                    if (strlen(param->password) == 0) {
                        snprintf(iaParameter, sizeof(iaParameter), "%s,%s,%d,%s,%d,%s", iccid, param->protocol,
                                param->authtype, param->username, param->canHandleIms, "IMS");
                    } else {
                        //when password is set, iccid is not recorded
                        //so we do not need to compare iccid
                        snprintf(iaParameter, sizeof(iaParameter), "%s,%s,%d,%s,%d,%s", "", param->protocol, param->authtype,
                                param->username, param->canHandleIms, "IMS");
                    }
                    if (strcmp(iaParameter, iaProperty) == 0) {
                        triggerPsAttach = 0;
                        LOGD("IA: requestSetInitialAttachApn, IA cache is equal to IMS");
                    }
                }
            }
            if (strcmp(iaParameter, iaProperty) != 0) {
                LOGD("IA: requestSetInitialAttachApn the parameter is different than cache");
                if (strlen(iccid) > 0) {
                    memset(iaProperty, 0, sizeof(iaProperty));
                }
            }
        } else if (atoi(iaPwdFlag)) {
            //the IA Cache is empty due to password
            char iaProperty[PROPERTY_VALUE_MAX*2] = {0};
            char iaParameter[PROPERTY_VALUE_MAX*2] = {0};
            getTempIa(iaSimId, iaProperty);
            snprintf(iaParameter, PROPERTY_VALUE_MAX*2, "%s,%s,%d,%s,%d,%s,%s,%s", iccid,
                    param->protocol, param->authtype, param->username, canHandleIms,
                    param->roamingProtocol, param->apn, param->password);
            if (strcmp(iaParameter, iaProperty) != 0) {
                LOGD("IA: requestSetInitialAttachApn the parameter(pwd) is different than cache \
[%s][%s]", iaParameter, iaProperty);
            } else {
                LOGV("IA: requestSetInitialAttachApn the parameter(pwd) is the same as cache");
                isApnDiff = 0;
            }
        }

        if (strlen(iaProperty) == 0 && isApnDiff) {
            int isOp01 = 0;
            int isOp01VolteOn = isOp01Volte();
            int check = 0;
            char optr[PROPERTY_VALUE_MAX] = {0};
            char* apn = "";
            RIL_Default_Bearer_VA_Config_Struct* defaultBearer = NULL;

            property_get("persist.vendor.operator.optr", optr, "");
            isOp01 = (strcmp("OP01", optr) == 0);

            if (canHandleIms || isOp01VolteOn) {
                defaultBearer = calloc(1, sizeof(RIL_Default_Bearer_VA_Config_Struct));
                if (defaultBearer == NULL) {
                    goto error;
                }
                defaultBearer->signalingFlag = isOp01VolteOn ? 0 : 1;
                defaultBearer->pcscf_discovery_flag  =1;
                defaultBearer->emergency_ind = 0;
            }

            if (strlen(param->password) != 0) {
                strncpy(TEMP_IA_PASSWORD[iaSimId], param->password, sizeof(TEMP_IA_PASSWORD[iaSimId])-1);
                setMSimProperty(iaSimId, PROPERTY_IA_PASSWORD_FLAG, "1");
            } else {
                memset(TEMP_IA_PASSWORD[iaSimId], 0, sizeof(TEMP_IA_PASSWORD[iaSimId]));
                setMSimProperty(iaSimId, PROPERTY_IA_PASSWORD_FLAG, "0");
            }

            // for op01 test sim, force apn to empty.
            if (RIL_isTestSim(rid) && (isOp01 || isOp01VolteOn)) {
                apn = "";
            } else {
                apn = param->apn;
            }

            // If current IA is invalid, not do reattach,
            // otherwise, do reattach if other conditions also satisfied.
            isCurIaInvalid = isIaInvalid(pChannel);
            LOGD("IA: requestSetInitialAttachApn isCurIaInvalid = %d", isCurIaInvalid);

            check = definePdnCtx(defaultBearer, apn, get_protocol_type(param->protocol), 0,
                    getAuthTypeInt(param->authtype), param->username, param->password,
                    pChannel);

            if (defaultBearer != NULL) {
                free(defaultBearer);
                defaultBearer = NULL;
            }

            if (0 == check) {
                LOGE("IA: requestSetInitialAttachApn define CID0 failed");
                at_send_command("AT+EGREA=0", NULL, pChannel);
                // On fwk APN changed, 'set IA' and 'deactivate data call' could occur almost in
                // the same time causing AT commands conflicts.
                TimeCallbackParam* param = (TimeCallbackParam*)calloc(1, sizeof(TimeCallbackParam));
                if (param != NULL) {
                    param->rid = rid;
                    LOGE("IA: requestSetInitialAttachApn relay onAttachApnReset to data channel.");
                    RIL_requestProxyTimedCallback (onAttachApnReset,
                           param,
                           &TIMEVAL_300_ms,
                           pChannel->id,
                           "onAttachApnReset");
                }
                goto error;
            }
            setTempIa(iaSimId, iccid, param->protocol, param->authtype, param->username,
                        canHandleIms, param->apn, param->roamingProtocol);

            //update IA APN ICCID
            char maskIccid[PROPERTY_VALUE_MAX] = {0};
            givePrintableIccid(iccid, maskIccid);
            LOGV("IA: requestSetInitialAttachApn update IA ICCID [%s]", maskIccid);
            setMSimProperty(iaSimId, PROPERTY_IA_APN_SET_ICCID, iccid);

            if (strlen(param->password) == 0) {
                setIaCache(iaSimId, iccid, param->protocol, param->authtype, param->username,
                        canHandleIms, param->apn, param->roamingProtocol);
            } else {
                LOGV("IA: requestSetInitialAttachApn initial attach APN contain password, \
set patch to invalid value");
                setIaCache(iaSimId, "", "", 0, "", 0, "", "");
            }

            // M: OP17 IA Start
            getMSimProperty(rid, "vendor.gsm.ril.sim.op17", optr);
            bool isOp17 = (strcmp("1", optr) == 0)? true : false;
            int radioState = queryRadioState(RIL_SIM, rid);
            int cid = 0;
            if (triggerPsAttach && queryCurrentConnType(pChannel) &&
                    RAT_2_3G != querryCurrentRat(pChannel) &&
                    (!isOp17 ||
                    (isOp17 && strncmp(operatorNumeric, DOCOMO_DOMESTIC_PLMN, 3) != 0)) &&
                    !isCurIaInvalid &&
                    (radioState != 0) &&
                    isCidActive(&cid, 1, DATA_CHANNEL_CTX)) {
                LOGD("IA: requestSetInitialAttachApn relay re-attach to data channel");
                OnReAttachInfo* pInfo = calloc(1, sizeof(OnReAttachInfo));
                pInfo->pDataChannel = pChannel;
                strncpy(pInfo->apn, param->apn, PROPERTY_VALUE_MAX-1);
                RIL_requestProxyTimedCallback (onReAttachForSettingAttachApn, (void*)pInfo, &TIMEVAL_0,
                    pInfo->pDataChannel->id, "onReAttachForSettingAttachApn");
            } else {
                LOGD("IA: requestSetInitialAttachApn not to do PS attach");
            }
        } else {
            LOGD("IA: requestSetInitialAttachApn valid IA property is set, \
not to do attach again");
        }
    } else {
        LOGD("IA: requestSetInitialAttachApn but CID0 is not support");
    }

finish:
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    // Fix: APN reset timing issue
    pthread_mutex_unlock(&setIaMutex[rid]);
    return;

error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    // Fix: APN reset timing issue
    pthread_mutex_unlock(&setIaMutex[rid]);
}

// M: Data Framework - common part enhancement @{
void requestSyncDataSettingsToMd(void* data, size_t datalen, RIL_Token t) {
    UNUSED(datalen);
    LOGD("requestSyncDataSettingsToMd start");
    char *cmd1 = NULL;
    char *cmd2 = NULL;
    int dataOn = ((int *)data)[0];
    int dataRoamingOn = ((int *)data)[1];

    if (dataOn == SKIP_DATA_SETTINGS && dataRoamingOn == SKIP_DATA_SETTINGS) {
        LOGD("No need to send to MD, requestSyncDataSettingsToMd end");
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        return;
    }

    // M: Data Framework - CC 33
    if (isCC33Support() == 1) {
        asprintf(&cmd1, "AT+EDSS=%d", dataOn);
        at_send_command(cmd1, NULL, DATA_CHANNEL_CTX);
        free(cmd1);
    }

    // AT+ECNCFG=<mobile_data>,<data_roaming>,[<volte>,<ims_test_mode>]
    asprintf(&cmd2, "AT+ECNCFG=%d,%d", dataOn, dataRoamingOn);
    at_send_command(cmd2, NULL, DATA_CHANNEL_CTX);
    free(cmd2);

    LOGD("requestSyncDataSettingsToMd end");
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
}
// M: Data Framework - common part enhancement @}

// M: Data Framework - CC 33 @{
int isCC33Support() {
    char value[PROPERTY_VALUE_MAX] = {0};
    int ret = 0;
    property_get(MTK_CC33_SUPPORT, value, "0");
    ret = atoi(value);
    return ret;
}

void requestSetRemoveRestrictEutranMode(void * data, size_t datalen, RIL_Token t)
{
    UNUSED(datalen);
    int err=0;
    int type=0;
    char *cmd = NULL;;
    ATResponse *p_response = NULL;
    //0: disable +ECODE33 URC
    //1: enable  +ECODE33 URC
    type = ((int *)data)[0];
    LOGV("ENTER requestSetRemoveRestrictEutranMode type:%d", type);

    asprintf(&cmd, "AT+ECODE33=%d", type);
    err = at_send_command(cmd, &p_response, DATA_CHANNEL_CTX);
    free(cmd);

    if (isATCmdRspErr(err, p_response)) {
        LOGE("requestSetRemoveRestrictEutranMode failed !");
        goto error;
    }

    LOGV("requestSetRemoveRestrictEutranMode ok !");
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);
    return;

error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}
// M: Data Framework - CC 33 @}

/* Change name from onPdpContextListChanged to onDataCallListChanged */
/* It can be called in onUnsolicited() mtk-ril\ril_callbacks.c */
void onDataCallListChanged(void* param)
{
    RIL_SOCKET_ID rid = *((RIL_SOCKET_ID *) param);
    requestOrSendDataCallList(NULL, rid);
}

void onMePdnActive(void* param)
{
    MePdnActiveInfo* pInfo = (MePdnActiveInfo*)param;
    RILChannelCtx* rilchnlctx = getChannelCtxbyProxy();
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(rilchnlctx);
    int activatedCid = pInfo->activeCid;
    int reason = pInfo->reason;
    int err = 0;
    ATLine *p_cur = NULL;
    ATResponse *p_response = NULL;
    char *out = NULL;
    int i;
    int isFindNoDataPdn = 0;

    LOGI("[%d]onMePdnActive: CID%d is activated and current state is %d",
            rid, activatedCid, pdn_info[rid][activatedCid].active);
    // [C2K][IRAT] Check react PDN count for cid 0 case.
    if (nIratAction == IRAT_ACTION_TARGET_STARTED && nReactPdnCount > 0) {
        onIratPdnReactSucc(activatedCid, rilchnlctx);
        free(param);
        return;
    }

    if (pdn_info[rid][activatedCid].active == DATA_STATE_INACTIVE) {
        pdn_info[rid][activatedCid].active = DATA_STATE_LINKDOWN; // Update with link down state.
        pdn_info[rid][activatedCid].cid = activatedCid;
        pdn_info[rid][activatedCid].primaryCid = activatedCid;
        pdn_info[rid][activatedCid].reason = reason;
    }

    //Check no data PDN
    for (i = 0; i < pdnInfoSize; i++) {
        if (1 == pdn_info[rid][i].isNoDataPdn) {
            LOGI("[%d]onMePdnActive: CID%d NoData PDN %s exist", rid, i, pdn_info[rid][i].apn);
            isFindNoDataPdn = 1;
            break;
        }
    }

    // Normal case(not NoDataPdn) will goes through, NoDataPdn case will skip below.
    if (0 == isFindNoDataPdn) {
        err = at_send_command_multiline ("AT+CGCONTRDP", "+CGCONTRDP:", &p_response, rilchnlctx);
        if (isATCmdRspErr(err, p_response)) {
            LOGE("[%d]onMePdnActive: AT+CGCONTRDP response error", rid);
        } else {
            for (p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next) {
                char *line = p_cur->line;
                int cid;
                int type;

                err = at_tok_start(&line);
                if (err < 0)
                    break;

                err = at_tok_nextint(&line, &cid); //cid
                if (err < 0)
                    break;

                err = at_tok_nextint(&line, &type); //type
                if (err < 0)
                    break;

                err = at_tok_nextstr(&line, &out); //apn
                if (err < 0)
                    break;

                if (out != NULL && strlen(out) > 0 && apnMatcher(out, DEFAULT_NODATA_APN) == 0)
                {
                    //NODATA APN is found
                    LOGI("[%d]onMePdnActive: NoData APN is found [%s] Cid:%d", rid, out, cid);
                    pdn_info[rid][cid].isNoDataPdn = 1;
                    isFindNoDataPdn = 1;
                    break;
                }

                // Since "+CGEV: ME PDN ACT" only notify the active CID (e.g. CID0),
                // but no APN name, so update the APN name here in the case if there
                // any requirement need to check activeCID's(e.g. CID0) APN name.
                // For instance, Bar PDN case should check the exist PDN apn name.
                // However, the update flow is not specific so take it as common flow.
                LOGV("[%d]onMePdnActive: APN:[%s], cid:%d, State=", rid, out, cid);
                if (pdn_info[rid][activatedCid].cid == cid &&
                        pdn_info[rid][activatedCid].active == DATA_STATE_LINKDOWN) {
                    strncpy(pdn_info[rid][activatedCid].apn, out, strlen(out));
                    pdn_info[rid][activatedCid].apn[strlen(out)] = 0;
                    LOGD("[%d]onMePdnActive: update APN name done.", rid);

                    // M: [VzW] Data Framework @{
                    if (isOp12Support()) {
                        if (strcmp(out, "VZWIMS") == 0 || strcmp(out, "VZWINTERNET") == 0) {
                            RIL_SOCKET_ID rilid = getRILIdByChannelCtx(rilchnlctx);
                            getAndSendPcoStatus(rilid, cid, out);
                        }
                    }
                    // M: [VzW] Data Framework @}
                }
            }
        }

        if (0 == isFindNoDataPdn)
            LOGD("[%d]onMePdnActive: NoData APN is not found", rid);
    }

    free(param);
    AT_RSP_FREE(p_response);
}

void onReAttachForSettingAttachApn(void* param) {
    int *matchCount = (int *) calloc(1, pdnInfoSize * sizeof(int));
    if (matchCount == NULL) {
        LOGE("[%s] matchCount calloc failed.", __FUNCTION__);
        return;
    }
    OnReAttachInfo* pInfo = (OnReAttachInfo*)param;
    RILChannelCtx* pChannel = pInfo->pDataChannel;

    LOGD("IA: onReAttachForSettingAttachApn start detach, skipDetach: %d",
            sIsSkipDetachForSimRefresh);
    AsyncAttachParam* reAttach = calloc(1, sizeof(AsyncAttachParam));
    if (reAttach == NULL) {
        LOGE("[%s] calloc for reAttach failed.", __FUNCTION__);
        goto finish;
    }
    reAttach->mode = ALWAYS_ATTACH_WITH_CHECK;

    // Multi-PS attach Start
    // The attach command should use EGTYPE =1 instead of EGTYPE =4 in MPS project.
    if (isMultiPsAttachSupport()) {
        LOGV("IA: onReAttachForSettingAttachApn- use EGTYPE=1");
        reAttach->mode = ALWAYS_ATTACH;
    }
    // Multi-PS attach End

    reAttach->pDataChannel = pChannel;
    char* apn = pInfo->apn;
    int isReplaceApnFlag = 0;
    int i;

    for (i = 0; i < pdnInfoSize; i++) {
        matchCount[i] = 0;
    }

    // Modem 3G will detach/attach after sim refresh automatically, but modem 4G will only do detach itself,
    // and expect AP to trigger attach after sim refresh, In the past, AP do detach/attach after sim refresh
    // to fulfill modem 2G/3G/4G expectation generally, it will cause AT&T case fail now due to unexpected
    // additional detach from AP after sim refresh for 3G modem
    // solution: skip first detach comming after sim refresh
    if(sIsSkipDetachForSimRefresh == 0) {
        //when AT+EGREA=1 is set, EUTRAN would not be disabled after PS detached
        at_send_command("AT+EGREA=1", NULL, pChannel);
        detachPs(pChannel);
        LOGD("IA: onReAttachForSettingAttachApn detach done and start attach");
        //when AT+EGREA=0 is set, EUTRAN would be disabled after PS detached
        at_send_command("AT+EGREA=0", NULL, pChannel);
    } else {
        sIsSkipDetachForSimRefresh = 0;
    }

    // Use non-blocking AT command to do PS attach, to avoid
    // modem AT command conflict
    if (!doPsAttachAsync(reAttach)) {
        LOGE("IA: onReAttachForSettingAttachApn trigger attach failed");
        //we sill continue to update cache even if attach is failed
    }
    //Trigger query apn to update network assigned apn
    int ret = queryMatchedPdnWithSameApn((const char *)apn, matchCount, pChannel);
    LOGV("IA: onReAttachForSettingAttachApn re-attach complete, ret = %d", ret);

finish:
    if(matchCount != NULL) {
        free(matchCount);
    }

    free(pInfo);
}

void onAttachApnReset(void* param)
{
    TimeCallbackParam* timeCallbackParam = (TimeCallbackParam*) param;
    RIL_SOCKET_ID rid = timeCallbackParam->rid;
    char* urc = (char*) (timeCallbackParam->urc);
    int err = 0;

    // Fix: APN reset timing issue
    if ((err = pthread_mutex_trylock(&setIaMutex[rid])) != 0){
        LOGD("onAttachApnReset: skip because requestSetInitialAttachApn locks.");
        FREEIF(timeCallbackParam->urc);
        FREEIF(timeCallbackParam);
        return;
    }
    if (urc == NULL) {
        // Change attach APN failed, we need to re-attach it.
        LOGD("onAttachApnReset: send RIL_UNSOL_RESET_ATTACH_APN because change IA APN failed.");
        RIL_UNSOL_RESPONSE(RIL_UNSOL_RESET_ATTACH_APN, NULL, 0, rid);
    } else {
        /* MD may have got valid IA when we receive +EAPN urc
           so we should double check the validation before
           clear IA cache, otherwise it will trigger a unnecessary
           set IA flow if we send RIL_UNSOL_RESET_ATTACH_APN.
           */
        RILChannelCtx* pChannel = getChannelCtxbyProxy();
        int isCurIaInvalid = isIaInvalid(pChannel);
        if (isCurIaInvalid) {
            int iaSimId = rid;
            clearIaCache(iaSimId);
            LOGD("onAttachApnReset: invalid IA, clean cache and send RIL_UNSOL_RESET_ATTACH_APN");
            RIL_UNSOL_RESPONSE(RIL_UNSOL_RESET_ATTACH_APN, NULL, 0, rid);
        } else {
            LOGD("onAttachApnReset: skip because IA APN is already valid.");
        }
        free(timeCallbackParam->urc);
    }
    free(timeCallbackParam);
    // Fix: APN reset timing issue
    pthread_mutex_unlock(&setIaMutex[rid]);
}

// M: [LTE][Low Power][UL traffic shaping] @{
static unsigned int convertNetworkType(unsigned int uiResponse)
{
    unsigned int uiRet = 0;

    /* mapping */
    switch(uiResponse)
    {
    case 0:     //GSM
    case 1:     //GSM compact
        uiRet = 1;        // GPRS only
        break;
    case 2:     //UTRAN
        uiRet = 3;        // UMTS
        break;
    case 3:     //GSM w/EGPRS
        uiRet = 2;        // EDGE
        break;
    case 4:     //UTRAN w/HSDPA
        uiRet = 9;        // HSDPA
        break;
    case 5:     //UTRAN w/HSUPA
        uiRet = 10;        // HSUPA
        break;
    case 6:     //UTRAN w/HSDPA and HSUPA
        uiRet = 11;        // HSPA
        break;
    //for LTE
    case 7:     //E-UTRAN
        uiRet = 14;        // LTE
        break;
    default:
        uiRet = 0;        // Unknown
        break;
    }

    return uiRet;
}

void onLteAccessStratumResult(char *urc, const RIL_SOCKET_ID rid)
{
    int ret;
    unsigned int response[2];
    char* tokenStr = NULL;
    int count = 1;

    // LOGD("onLteAccessStratumResult:%s",urc);

    // +EDRBSTATE:<drb_state>[,<AcT>]
    // <drb_state> 1 if DRB exists, and 0 otherwise
    // <AcT> only reported when drb_state = 0, report the current PS access technology
    ret = at_tok_start(&urc);
    if (ret < 0) goto error;

    // get <lteAccessStratum_status>
    ret = at_tok_nextint(&urc, &response[0]);
    if (ret < 0) goto error;

    if (response[0] == 0) { // 2G/3G/4G connected but no DRB exists/4G AS idle
        // get <Act>
        ret = at_tok_nextint(&urc, &response[1]);
        if (ret < 0) goto error;
        count = 2;
        response[1] = convertNetworkType(response[1]);
    }

    if (1 == count) {
        LOGD("onLteAccessStratumResult (<lteAccessStratum_status>=%d)", response[0]);
    } else {
        LOGD("onLteAccessStratumResult (<lteAccessStratum_status>=%d, <PS network type>=%d)",
                response[0], response[1]);
    }

    RIL_UNSOL_RESPONSE(RIL_UNSOL_LTE_ACCESS_STRATUM_STATE_CHANGE, &response, count*sizeof(int), rid); // MTK defined UNSOL EVENT
    return;

error:
    LOGE("There is something wrong with the +EDRBSTATE");
}

void requestSetLteAccessStratumReport(void* data, size_t datalen, RIL_Token t)
{
    UNUSED(datalen);
    int err=0;
    int enabled=0;
    char *cmd = NULL;;
    ATResponse *p_response = NULL;
    // 0: user turn off LTE AS URC report
    // 1: user turn on LTE AS URC report
    enabled = ((int*) data)[0];
    LOGV("ENTER requestSetLteAccessStratumReport enabled:%d", enabled);

    asprintf(&cmd, "AT+EDRB=%d", enabled);
    err = at_send_command(cmd, &p_response, DATA_CHANNEL_CTX);
    free(cmd);

    if (isATCmdRspErr(err, p_response)) {
        LOGE("requestSetLteAccessStratumReport failed !");
        goto error;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);
    return;

error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

void requestSetLteUplinkDataTransfer(void* data, size_t datalen, RIL_Token t)
{
    UNUSED(datalen);
    int err = 0;
    int state = 0;
    int interfaceId = -1;

    // Tclose timer(sec.) << 16 | 0 : stop uplink data transfer with Tclose timer
    // 1 : start uplink data transfer
    state = ((int*) data)[0];
    interfaceId = ((int*) data)[1];

    LOGV("ENTER requestSetLteUplinkDataTransfer state:%d, interfaceId:%d", state, interfaceId);

    setNetworkTransmitState(interfaceId, state);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
}
// M: [LTE][Low Power][UL traffic shaping] @}

static int isFallbackPdpRetryRunning(RIL_SOCKET_ID rid, const char* apn)
{
    char SimOperatorNumeric[PROPERTY_VALUE_MAX] = {0};
    getMSimProperty(rid, PROPERTY_SIM_OPERATOR_NUMERIC, SimOperatorNumeric);
    char networkType[PROPERTY_VALUE_MAX] = {0};
    char *nwType;
    int sim_index = rid;
    int ret = 0;

    //Check HPLMN
    // M: [General Operator] Data Framework - WWOP requirements: Telstra @{
    if (0 == strncmp(SimOperatorNumeric, TELSTRA_PDP_RETRY_PLMN, PROPERTY_VALUE_MAX-1)) {
        LOGD("requestSetupData SIM in Telstra 505-01 [%s]", SimOperatorNumeric);
        property_get(PROPERTY_DATA_NETWORK_TYPE, networkType, "");
        //Check if 3G network
        nwType = strtok(networkType, ",");
        while(sim_index > 0) {
            if (nwType != NULL) {
                nwType = strtok(NULL,",");
            }
            sim_index--;
        }

        if (nwType == NULL) {
            LOGD("requestSetupData: Not register yet");
        } else {
            LOGD("requestSetupData: registered in [%s] NW", nwType);
            if ((0 == strcmp(nwType, "UMTS")) ||
                (0 == strcmp(nwType, "HSDPA")) ||
                (0 == strcmp(nwType, "HSUPA")) ||
                (0 == strcmp(nwType, "HSPA")) ||
                (0 == strcmp(nwType, "HSPAP")) ||
                (0 == strcmp(nwType, "LTE"))) {
                //Check if IPv4v6 and default APN
                if ((NULL != apn) && (NULL == strstr(apn, "mms"))) {
                    LOGD("requestSetupData: match FALLBACK PDP retry support");
                    ret = 1;
                    return ret;
                }
            }
        }
    }
    // M: [General Operator] Data Framework - WWOP requirements: Telstra @}

    LOGD("requestSetupData: Not FALLBACK PDP retry");

    return ret;
}

int isLteDataOnCdma(RIL_SOCKET_ID rid)
{
    int r1 = RatConfig_isC2kSupported();
    int r2 = getActiveSvlteModeSlotId();
    int ret = r1 && (r2 == (rid + 1));

    LOGD("isLteOnCdma:%d, isCdmaSupport=%d, getActiveSvlteModeSlotId=%d, rid=%d",
        ret, r1, r2, rid);

    return ret;
}

int isRequestToDeactDataCallByReasonFromMAL(int reason)
{
    return (reason == MAL_DM_EGACT_CAUSE_DONT_CARE ||
            reason == MAL_DM_EGACT_CAUSE_DEACT_NORMAL ||
            reason == MAL_DM_EGACT_CAUSE_DEACT_RA_INITIAL_ERROR ||
            reason == MAL_DM_EGACT_CAUSE_DEACT_NO_PCSCF ||
            reason == MAL_DM_EGACT_CAUSE_DEACT_RA_REFRESH_ERROR);
}

int isRequestToDeactDataCallByReasonFromFWK(int reason)
{
    return (reason == FWK_EGACT_CAUSE_DEACT_NORMAL ||
            reason == FWK_EGACT_CAUSE_DEACT_RA_INITIAL_ERROR ||
            reason == FWK_EGACT_CAUSE_DEACT_NO_PCSCF ||
            reason == FWK_EGACT_CAUSE_DEACT_RA_REFRESH_ERROR);
}

static int isFallbackPdpNeeded(RIL_SOCKET_ID rid, const char* apn, int protocol)
{
    int ret = 0;
    char operatorNumeric[PROPERTY_VALUE_MAX] = {0};
    property_get((char *)PROPERTY_NW_MCCMNC[rid], operatorNumeric, "");

    //Check registered PLMN
    // M: Data Framework - CC33 @{
    if (0 == strncmp(operatorNumeric, TELCEL_PDP_RETRY_PLMN, PROPERTY_VALUE_MAX-1)
        || 0 == strncmp(operatorNumeric, TELCEL_PDP_RETRY_PLMN2, PROPERTY_VALUE_MAX-1)
        || 0 == strncmp(operatorNumeric, CLARO_PDP_RETRY_PLMN, PROPERTY_VALUE_MAX-1)) {
        LOGD("requestSetupData registered in CC33 operator [%s]", operatorNumeric);
        if (isCC33Support() == 0 && isApFallbackSupport()) ret = 1;
    // M: Data Framework - CC33 @}
    // M: [General Operator] Data Framework - WWOP requirements: Telstra @{
    } else if (0 == strncmp(operatorNumeric, TELSTRA_PDP_RETRY_PLMN, PROPERTY_VALUE_MAX-1)) {
        LOGD("requestSetupData registered in Telstra 505-01 [%s]", operatorNumeric);
        ret = isFallbackPdpRetryRunning(rid, apn);
    // M: [General Operator] Data Framework - WWOP requirements: Telstra @}
    } else {
        LOGD("requestSetupData common");
        if (!isLteDataOnCdma(rid) && isApFallbackSupport()) ret = 1;
    }

    if (ret) {
        if ((IPV4V6 != protocol)
                || ((NULL != apn) && (NULL != strcasestr(apn, "ims")))
                || (pdnReasonSupportVer < MD_LR11)) {
            LOGD("requestSetupData DON'T trigger AP IPv4v6 fallback because NOT support \
(IPv4 or IPv6 only)/IMS/no PDN reason cause report");
            ret = 0;
        }
    }

    return ret;
}

static void clearIaCache(int iaSimId) {
    char temp[PROPERTY_VALUE_MAX] = {0};
    LOGD("clearIaCache");
    setIaCache(iaSimId, "", "", 0, "", 0, "", "");
    setTempIa(iaSimId, "", "", 0, "", 0, "", "");

    getMSimProperty(iaSimId, PROPERTY_IA_APN_SET_ICCID, temp);
    if (strcmp(temp, "") != 0) {
        setMSimProperty(iaSimId, PROPERTY_IA_APN_SET_ICCID, "");
    }
    memset(temp, 0, PROPERTY_VALUE_MAX);
    getMSimProperty(iaSimId, PROPERTY_IA_PASSWORD_FLAG, temp);
    if (strcmp(temp, "") != 0) {
        setMSimProperty(iaSimId, PROPERTY_IA_PASSWORD_FLAG, "");
    }
}

// When receiving +EAPNMOD, check if MD is using different IA APN with the value in IA cache,
// clear IA cache if different.
// e.g. IA cache contains class 1 APN, while MD is using class 3 APN after lwcg mode C->L IRAT,
// and MD send +EAPNMOD:1 to require IA change. AP need to clear cache first or
// requestSetInitialAttachApn() will not make IA change.
static void clearIaCacheIfNeed(RIL_SOCKET_ID rid) {
    ATResponse *p_response = NULL;
    ATLine *p_cur = NULL;
    char *line = NULL;
    int err = 0;
    int cid = -1;
    char *out = NULL;
    char *apn = NULL;
    RILChannelCtx* pChannel = getChannelCtxbyProxy();

    // Multi-PS attach Start
    int iaSimId = rid;
    // Multi-PS attach End

    char iaApn[PROPERTY_VALUE_MAX] = {0};
    char iaTempApn[PROPERTY_VALUE_MAX] = {0};
    getMSimProperty(iaSimId, PROPERTY_IA_APN, iaApn);
    getMSimProperty(iaSimId, PROPERTY_TEMP_IA_APN, iaTempApn);

    err = at_send_command_multiline("AT+CGDCONT?", "+CGDCONT:", &p_response, pChannel);

    // Response:
    // +CGDCONT=<cid>,<PDP_type>,<APN>,<PDP_addr>,<d_comp>,<h_comp>[,<IPv4AddrAlloc>
    // [,<request_type>[,<P-CSCF_discovery>[,<IM_CN_Signalling_Flag_Ind>]]]]
    if (isATCmdRspErr(err, p_response)) {
        LOGE("clearIaCacheIfNeed AT+CGDCONT? response error");
    } else {
        for (p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next) {
            line = p_cur->line;
            err = at_tok_start(&line);
            if (err < 0) {
                continue;
            }

            LOGD("clearIaCacheIfNeed: line is %s", line);
            err = at_tok_nextint(&line, &cid); // cid
            if (err < 0) {
                continue;
            }

            if (cid == 0) {
                err = at_tok_nextstr(&line, &out); // skip pdp_type
                if (err < 0) {
                    continue;
                }

                err = at_tok_nextstr(&line, &apn); // apn
                if (err < 0) {
                    continue;
                }

                LOGD("clearIaCacheIfNeed: apn = %s, iaApn = %s, iaTempApn = %s",
                        apn, iaApn, iaTempApn);
                if ((0 != strlen(iaApn) && 0 != apnMatcher(apn, iaApn)) ||
                        (0 != strlen(iaTempApn) && 0 != apnMatcher(apn, iaTempApn))) {
                    clearIaCache(iaSimId);
                }
                break;
            }
        }
    }

    AT_RSP_FREE(p_response);
}

static void onApnMod(void* param) {
    TimeCallbackParam* timeCallbackParam = (TimeCallbackParam*) param;
    RIL_SOCKET_ID rid = timeCallbackParam->rid;
    char* urc = (char*) (timeCallbackParam->urc);
    LOGD("onApnMod: rid = %d, urc = %s", rid, urc);

    // Check if need to clear IA cache
    clearIaCacheIfNeed(rid);

    // For the bar PDN in the case of LC/LWCG
    onBarApn(urc, rid);

    free(timeCallbackParam->urc);
    free(timeCallbackParam);
}

bool isKeepDefaultPdn(int interfaceId, RIL_SOCKET_ID rid, char* apn) {
    char operatorNumeric[PROPERTY_VALUE_MAX] = {0};
    char dataRat[PROPERTY_VALUE_MAX] = {0};

    if (0 != interfaceId) {
        LOGD("[%s] not default pdn", __FUNCTION__);
        return false;
    }

    property_get((char *)PROPERTY_NW_MCCMNC[rid], operatorNumeric, "");
    getMSimProperty(rid, PROPERTY_DATA_NETWORK_TYPE, dataRat);
    LOGD("[%d][%s] operatorNumeric = %s, dataRat = %s",
            rid, __FUNCTION__, operatorNumeric, dataRat);

    if ((isOp01Support() && NULL != strcasestr(apn, "cmnet")) ||
            isOp07Support() || isOp08Support()) {
        if (isWfcSupport() || 0 == strcmp(dataRat, "LTE")) {
            return true;
        }
    } else if (0 == strcmp(operatorNumeric, "23430") || isWfcSupport()) {
        if (0 == strcmp(dataRat, "LTE")) {
            return isDefaultPdnReuseIa(rid);
        }
    }

    return false;
}

bool isDefaultPdnReuseIa(RIL_SOCKET_ID rid) {
    int index = 0;
    char property_ia_from_network[PROPERTY_VALUE_MAX] = {0};

    property_get(PROPERTY_IA_FROM_NETWORK[rid], property_ia_from_network, "");
    LOGD("[%s] property_ia_from_network: %s", __FUNCTION__, property_ia_from_network);

    for (index; index < pdnInfoSize; index++) {
        if (0 == apnMatcher(property_ia_from_network, pdn_info[rid][index].apn)) {
            LOGD("[%s] pdn_info[%d][%d].apn: %s, ", __FUNCTION__,
                    rid, index, pdn_info[rid][index].apn);
            break;
        }
    }

    if (index >= pdnInfoSize) {
        return false;
    } else if (pdn_info[rid][index].interfaceId== 0) {
        LOGD("[%s] pdn_info[%d][%d].interfaceId: %d, ", __FUNCTION__,
                rid, index, pdn_info[rid][index].interfaceId);
        return true;
    } else {
        return false;
    }
}

extern int rilDataMain(int request, void *data, size_t datalen, RIL_Token t)
{
    switch (request) {
        case RIL_REQUEST_SETUP_DATA_CALL:
            requestSetupDataCall(data, datalen, t);
            break;
        case RIL_REQUEST_DATA_CALL_LIST:
            requestDataCallList(data, datalen, t);
            break;
        case RIL_REQUEST_DEACTIVATE_DATA_CALL:
            requestDeactiveDataCall(data, datalen, t);
            break;
        case RIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE:
            requestLastDataCallFailCause(data, datalen, t);
            break;
        case RIL_REQUEST_ALLOW_DATA:
            requestAllowData(data, datalen, t);
            break;
        case RIL_REQUEST_SET_PS_REGISTRATION:
            requestRegisterPs(data, datalen, t);
            break;
        // LCE service start
        case RIL_REQUEST_START_LCE:
            requestStartLce(data, datalen, t);
            break;
        case RIL_REQUEST_STOP_LCE:
            requestStopLce(data, datalen, t);
            break;
        case RIL_REQUEST_PULL_LCEDATA:
            requestPullLceData(data, datalen, t);
            break;
        case RIL_REQUEST_SET_LINK_CAPACITY_REPORTING_CRITERIA:
            requestSetLinkCapacityReportingCriteria(data, datalen, t);
            break;
        // LCE service end
        case RIL_REQUEST_SET_INITIAL_ATTACH_APN:
            requestSetInitialAttachApn(data, datalen, t);
            break;
        // M: [C2K][IRAT] request start. {@
        case RIL_LOCAL_GSM_REQUEST_SET_ACTIVE_PS_SLOT:
            requestSetActivePsSimSlot(data, datalen, t);
            break;
        case RIL_LOCAL_GSM_REQUEST_CONFIRM_INTER_3GPP_IRAT_CHANGE:
            confirmIratChange(data, datalen, t);
            break;
        // M: [C2K][IRAT] request end. @}
        case RIL_REQUEST_SET_DATA_PROFILE:
            syncApnTableToMdAndRds(data, datalen, t);
            break;
        // M: Data Framework - common part enhancement
        case RIL_REQUEST_SYNC_DATA_SETTINGS_TO_MD:
            requestSyncDataSettingsToMd(data, datalen, t);
            break;
        // M: Data Framework - CC 33
        case RIL_REQUEST_SET_REMOVE_RESTRICT_EUTRAN_MODE:
            requestSetRemoveRestrictEutranMode(data, datalen, t);
            break;
        // FastDormancy
        case RIL_REQUEST_SET_FD_MODE:
            requestSetFdMode(data, datalen, t);
            break;
        // M: MPS feature
        case RIL_LOCAL_REQUEST_DEACTIVATE_ALL_DATA_CALL:
            requestDeactiveAllDataCall(data, datalen, t);
            break;
        // M: [LTE][Low Power][UL traffic shaping] @{
        case RIL_REQUEST_SET_LTE_ACCESS_STRATUM_REPORT:
            requestSetLteAccessStratumReport(data, datalen, t);
            break;
        case RIL_REQUEST_SET_LTE_UPLINK_DATA_TRANSFER:
            requestSetLteUplinkDataTransfer(data, datalen, t);
            break;
        // M: [LTE][Low Power][UL traffic shaping] @}
        // M: Ims Data Framework {@
        case RIL_LOCAL_REQUEST_HANDOVER_IND:
            requestHandoverInd(data, datalen, t);
            break;
        case RIL_LOCAL_REQUEST_WIFI_DISCONNECT_IND:
            requestWifiDisconnectInd(data, datalen, t);
            break;
        case RIL_LOCAL_REQUEST_PCSCF_DISCOVERY_PCO:
            requestPcscfPco(data, datalen, t);
            break;
        case RIL_LOCAL_REQUEST_WIFI_CONNECT_IND:
            requestWifiConnectInd(data, datalen, t);
            break;
        case RIL_LOCAL_REQUEST_SETUP_DATA_CALL_ALT:
            requestSetupDataCallFromMal(data, datalen, t);
            break;
        case RIL_LOCAL_REQUEST_PCSCF_DISCOVERY_PCO_ALT:
            requestPcscfPcoFromMal(data, datalen, t);
            break;
        case RIL_LOCAL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE_ALT:
            requestLastDataCallFailCauseFromMal(data, datalen, t);
            break;
        // @}
        case RIL_LOCAL_REQUEST_GET_IMS_DATA_CALL_INFO:
            requestGetImsDataCallInfo(data, datalen, t);
            break;
        case RIL_LOCAL_REQUEST_REUSE_IMS_DATA_CALL:
            requestReuseImsDataCall(data, datalen, t);
            break;
        case RIL_REQUEST_SETPROP_IMS_HANDOVER:
            requestSetpropImsHandover(data, datalen, t);
            break;
        case RIL_REQUEST_START_KEEPALIVE:
            requestStartKeepalive(data, datalen, t);
            break;
        case RIL_REQUEST_STOP_KEEPALIVE:
            requestStopKeepalive(data, datalen, t);
            break;
        default:
            return 0; /* no matched request */
    }

    return 1; /* request found and handled */

}

extern int rilDataUnsolicited(const char *s, const char *sms_pdu, RILChannelCtx* p_channel)
{
    UNUSED(sms_pdu);
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);

    if (strStartsWith(s, "+CGEV: NW DEACT") || strStartsWith(s, "+CGEV: ME DEACT") ||
            strStartsWith(s, "+CGEV: NW REACT") ||
            strStartsWith(s, "+CGEV: NW PDN DEACT") || strStartsWith(s, "+CGEV: ME PDN DEACT")) {
        char* urc = NULL;
        asprintf(&urc, "%s", s);
        TimeCallbackParam* param = (TimeCallbackParam*) calloc(1,
                sizeof(TimeCallbackParam));
        if (param == NULL) {
            LOGE("[%s] calloc failed!", __FUNCTION__);
            return 1;
        }
        param->rid = rid;
        param->urc = urc;
        RIL_requestProxyTimedCallback(onPdnDeact, param, &TIMEVAL_0,
                getRILChannelId(RIL_DATA, rid), "onPdnDeact");
        return 1;
    } else if (strStartsWith(s, "+CGEV: NW ACT") || strStartsWith(s, "+CGEV: NW MODIFY")) {
        //+CGEV: NW ACT <p_cid>, <cid>, <event_type>
        //+CGEV: NW MODIFY <cid>, <change_reason>, <event_type>
        LOGD("rilDataUnsolicited receive bearer activation/modification by network [%s]", s);
        char* urc = NULL;
        asprintf(&urc, "%s", s);

        TimeCallbackParam* param = (TimeCallbackParam*)calloc(1, sizeof(TimeCallbackParam));
        if (param == NULL) {
            LOGE("[%s] calloc failed!", __FUNCTION__);
            return 1;
        }
        param->rid = rid;
        param->urc = urc;
        if (strStartsWith(s, "+CGEV: NW MODIFY")) {
            param->isModification = 1;

            // M: [VzW] Data Framework @{
            // +CGEV: NW MODIFY <cid>, <change_reason>, <event_type>
            char* urc = NULL;
            asprintf(&urc, "%s", s);
            TimeCallbackParam* paramPco = (TimeCallbackParam*)calloc(1, sizeof(TimeCallbackParam));
            if (paramPco == NULL) {
                LOGE("[%s] calloc failed!", __FUNCTION__);
                free(param);
                return 1;
            }
            paramPco->rid = s_data_ril_cntx[rid];
            paramPco->urc = urc;

            // Keep +CGEV: ME MODIFY to avoid side effect.
            RIL_requestProxyTimedCallback(getAndSendPcoStatusWithParam, paramPco,
                &TIMEVAL_0, getRILChannelId(RIL_DATA, rid), "getAndSendPcoStatusWithParam");
            // M: [VzW] Data Framework @}
        } else
            param->isModification = 0;

        //be aware that the param->urc & param should be free in onNetworkBearerUpdate
        RIL_requestProxyTimedCallback (onNetworkBearerUpdate,
               param,
               &TIMEVAL_0,
               getRILChannelId(RIL_DATA, rid),
               "onNetworkBearerUpdate");
        return 1;
    } else if (strStartsWith(s, "+CGEV: ME")) {
        if (strStartsWith(s, "+CGEV: ME PDN ACT")) {
            int err = 0;
            int activatedCid = INVALID_CID;
            int reason = NO_CAUSE;
            int otherCid = INVALID_CID;

            err = at_tok_start(&s);
            /* s => +CGEV: ME PDN ACT X */

            if (err >= 0 && strStartsWith(s, " ME PDN ACT ")) {
                char *tempParam = NULL;

                err = at_tok_nextstr(&s, &tempParam);

                char *firstParam = tempParam + strlen(ME_PDN_URC);
                activatedCid = atoi(firstParam);

                LOGD("[%s] default bearer activated [CID=%d]", __FUNCTION__, activatedCid);

                if (err >= 0 && at_tok_hasmore(&s)) {
                    err = at_tok_nextint(&s, &reason);

                    LOGD("[%s] default bearer activated [CID=%d, Reason=%d]", __FUNCTION__,
                            activatedCid, reason);

                    if (err >= 0 && at_tok_hasmore(&s)) {
                        err = at_tok_nextint(&s, &otherCid);

                       LOGD("[%s] default bearer activated [CID=%d, Reason=%d, OtherCID=%d]",
                               __FUNCTION__, activatedCid, reason, otherCid);
                    }
                }
            }

            MePdnActiveInfo* pInfo = calloc(1, sizeof(MePdnActiveInfo));

            if (NULL == pInfo) {
                LOGE("rilDataUnsolicited allocate pInfo fail!!");
                return 1;
            }

            pInfo->activeCid = activatedCid;
            pInfo->reason = reason;
            pInfo->pDataChannel = NULL;

            LOGD("rilDataUnsolicited CID%d is activated and current state is %d",
                    pInfo->activeCid, pdn_info[rid][pInfo->activeCid].active);

            RIL_requestProxyTimedCallback (onMePdnActive,
                (void *) pInfo,
                &TIMEVAL_0,
                getRILChannelId(RIL_DATA, rid),
                "onMePdnActive");
        } else if (strStartsWith(s, "+CGEV: ME MODIFY")) {
            // +CGEV: ME MODIFY <cid>, <change_reason>, <event_type>
            char* urc = NULL;
            asprintf(&urc, "%s", s);
            TimeCallbackParam* param = (TimeCallbackParam*)calloc(1, sizeof(TimeCallbackParam));
            if (param == NULL) {
                LOGE("[%s] calloc failed!", __FUNCTION__);
                return 1;
            }
            param->rid = s_data_ril_cntx[rid];
            param->urc = urc;
            RIL_requestProxyTimedCallback(getAndSendPcoStatusWithParam, param,
                    &TIMEVAL_0, getRILChannelId(RIL_DATA, rid), "getAndSendPcoStatusWithParam");
        } else {
            //+CGEV: ME related cases should be handled in setup data call request handler
            LOGD("rilDataUnsolicited ignore +CGEV: ME cases (%s)", s);
        }
        return 1;
    } else if (strStartsWith(s, "+CGEV:")) {
        // M: [C2K][IRAT] specific process start
        if (strStartsWith(s, "+CGEV: NW DETACH")) {
            RIL_requestProxyTimedCallback(onNwDetach, &s_data_ril_cntx[rid],
                    &TIMEVAL_0, getRILChannelId(RIL_DATA, rid), "onNwDetach");
            return 1;
        }
        // M: [C2K][IRAT] specific process end

        if (s_md_off) {
            LOGD("rilDataUnsolicited(): modem off!");
            RIL_UNSOL_RESPONSE(RIL_UNSOL_DATA_CALL_LIST_CHANGED, NULL, 0, rid);
         } else {
            LOGD("rilDataUnsolicited before data call list: urc = %s.", s);
            RIL_requestProxyTimedCallback (onDataCallListChanged,
                &s_data_ril_cntx[rid],
                &TIMEVAL_0,
                getRILChannelId(RIL_DATA, rid),
                "onDataCallListChanged");
         }

        return 1;

#ifdef WORKAROUND_FAKE_CGEV
    } else if (strStartsWith(s, "+CME ERROR: 150")) {

        if (s_md_off)    {
            LOGD("rilDataUnsolicited(): modem off!");
            RIL_UNSOL_RESPONSE(RIL_UNSOL_DATA_CALL_LIST_CHANGED, NULL, 0, rid);
         } else {
            RIL_requestProxyTimedCallback (onDataCallListChanged,
                &s_data_ril_cntx[rid],
                &TIMEVAL_0,
                getRILChannelId(RIL_DATA, rid),
                "onDataCallListChanged");
         }

        return 1;
#endif /* WORKAROUND_FAKE_CGEV */
    } else if (strStartsWith(s, "+ECODE33")) {
        // M: Data Framework - CC 33
        RIL_onUnsolicitedResponse(RIL_UNSOL_REMOVE_RESTRICT_EUTRAN, NULL, 0, rid);
        return 1;
    // M: [C2K][IRAT] URC start. {@
    } else if (strStartsWith(s, "+EI3GPPIRAT:")) {
        LOGD("[RILData_GSM_URC] EI3GPPIRAT = %s.", s);
        char* urc = NULL;
        asprintf(&urc, "%s", s);
        TimeCallbackParam* param = (TimeCallbackParam*) calloc(1,
                sizeof(TimeCallbackParam));
        if (param == NULL) {
            LOGE("[%s] calloc failed!", __FUNCTION__);
            return 1;
        }
        param->rid = rid;
        param->urc = urc;
        RIL_requestProxyTimedCallback(onIratEvent, param, &TIMEVAL_0,
                getRILChannelId(RIL_DATA, rid), "onIratEvent");
        return 1;
    } else if (strStartsWith(s, "+EPDNCTRANS:")) {
        LOGD("[RILData_GSM_URC] %s", s);
        char* urc = NULL;
        asprintf(&urc, "%s", s);
        TimeCallbackParam* param = (TimeCallbackParam*) calloc(1,
                sizeof(TimeCallbackParam));
        if (param == NULL) {
            LOGE("[%s] calloc failed!", __FUNCTION__);
            return 1;
        }
        param->rid = rid;
        param->urc = urc;
        RIL_requestProxyTimedCallback(onLwcgIratEvent, param, &TIMEVAL_0,
                getRILChannelId(RIL_DATA, rid), "onLwcgIratEvent");
        return 1;
    } else if (strStartsWith(s, "+EGCONTRDP:")) {
        LOGD("[RILData_GSM_URC] EGCONTRDP nIratAction = %d.", nIratAction);

        char* urc = NULL;
        asprintf(&urc, "%s", s);
        TimeCallbackParam* param = (TimeCallbackParam*) calloc(1,
                sizeof(TimeCallbackParam));
        if (param == NULL) {
            LOGE("[%s] calloc failed!", __FUNCTION__);
            return 1;
        }
        param->rid = rid;
        param->urc = urc;
        RIL_requestProxyTimedCallback(onPdnSyncFromSourceRat, param, &TIMEVAL_0,
                getRILChannelId(RIL_DATA, rid), "onPdnSyncFromSourceRat");
        return 1;
    //M: [C2K][IRAT] URC end. @}
    } else if (strStartsWith(s, "+EAPN:")) {
        if (strStartsWith(s, "+EAPN: \"this_is_an_invalid_apn\"")) {
            TimeCallbackParam* param = (TimeCallbackParam*)calloc(1, sizeof(TimeCallbackParam));
            if (param == NULL) {
                LOGE("[%s] calloc failed!", __FUNCTION__);
                return 1;
            }
            param->rid = rid;

            // Pass URC string to distinguish with changing attach APN fail case.
            char* urc = NULL;
            asprintf(&urc, "%s", s);
            param->urc = urc;

            RIL_requestProxyTimedCallback (onAttachApnReset,
                   param,
                   &TIMEVAL_0,
                   getRILChannelId(RIL_DATA, rid),
                   "onAttachApnReset");
        } else {
            LOGD("rilDataUnsolicited(): not handle message of +EAPN");
        }
        return 1;
    } else if (strStartsWith(s, "+EAPNMOD:")) {
        // M: IA-change attach APN
        LOGD("rilDataUnsolicited receive +EAPNMOD [%s]", s);
        int err, apn;
        char* urc = (char*)s;
        err = at_tok_start(&urc);
        if (err < 0) return 0;

        err = at_tok_nextint(&urc, &apn);
        if (err < 0) return 0;

        // 1. Notify URC to data framework to change attach APN.
        RIL_UNSOL_RESPONSE(RIL_UNSOL_DATA_ATTACH_APN_CHANGED,
                &apn, sizeof(apn), rid);

        // 2. Check if need to clear IA cache
        // 3. For the bar PDN in the case of LC/LWCG
        TimeCallbackParam* param = (TimeCallbackParam*) calloc(1,
                sizeof(TimeCallbackParam));
        if (param == NULL) {
            LOGD("[%d][%s] param calloc failed.", rid, __FUNCTION__);
            return 0;
        }
        param->rid = rid;
        param->urc = urc;
        RIL_requestProxyTimedCallback(onApnMod, param, &TIMEVAL_0,
                getRILChannelId(RIL_DATA, rid), "onApnMod");
        return 1;
    } else if (strStartsWith(s, "+CGPRCO:")) {
        char* urc = NULL;
        asprintf(&urc, "%s", s);
        TimeCallbackParam* param = (TimeCallbackParam*) calloc(1, sizeof(TimeCallbackParam));
        if (param == NULL) {
            LOGE("[%s] calloc failed!", __FUNCTION__);
            return 1;
        }
        param->rid = rid;
        param->urc = urc;
        RIL_requestProxyTimedCallback(onPcoStatus, param,
                &TIMEVAL_0, getRILChannelId(RIL_DATA, rid), "onPcoStatus");
        return 1;
    // LCE URC start @{
    } else if (strStartsWith(s, "+ELCE:")) {
        TimeCallbackParam* param = (TimeCallbackParam*) calloc(1, sizeof(TimeCallbackParam));
        if (param == NULL) {
            LOGE("[%s] calloc failed!", __FUNCTION__);
            return 1;
        }
        param->rid = rid;
        param->urc = (char*)s;
        RIL_requestProxyTimedCallback(onLceDataResult, param,
                &TIMEVAL_0, getRILChannelId(RIL_DATA, rid), "onLceDataResult");
        return 1;
    } else if (strStartsWith(s, "+EXLCE:")) {
        TimeCallbackParam* param = (TimeCallbackParam*) calloc(1, sizeof(TimeCallbackParam));
        if (param == NULL) {
            LOGE("[%s] calloc failed!", __FUNCTION__);
            return 1;
        }
        param->rid = rid;
        param->urc = (char*)s;
        RIL_requestProxyTimedCallback(onLinkCapacityEstimate, param,
                &TIMEVAL_0, getRILChannelId(RIL_DATA, rid), "onLinkCapacityEstimate");
        return 1;
    // LCE URC end @}
    // M: [LTE][Low Power][UL traffic shaping] @{
    } else if (strStartsWith(s, "+EDRBSTATE:")) {
        onLteAccessStratumResult((char*) s, rid);
        return 1;
    /// Keep Alive @}
    } else if (strStartsWith(s, "+EKALIVE:")) {
        TimeCallbackParam* param = (TimeCallbackParam*) calloc(1, sizeof(TimeCallbackParam));
        if (param == NULL) {
            LOGE("[%s] calloc failed for +EKALIVE: urc", __FUNCTION__);
            return 1;
        }
        param->rid = rid;
        param->urc = (char*)s;
        RIL_requestProxyTimedCallback(onUpdateKeepaliveStatus, param,
                &TIMEVAL_0, getRILChannelId(RIL_DATA, rid), "onUpdateKeepaliveStatus");
        return 1;
    }
    /// @}

    return 0;
}

/// M: Ims Data Framework {@
void dumpDefaultBearerConfig(const RIL_Default_Bearer_VA_Config_Struct *pDefaultBearerVaConfig) {
    if (pDefaultBearerVaConfig != NULL) {
        LOGD("dump defaultBearerVaConfig QOS[qci=%d, dlGbr=%d, ulGbr=%d, dlMbr=%d, \
        ulMbr=%d] flag[emergency_ind=%d, pcscf_discovery_flag=%d, signalingFlag=%d, assigned_rat=%d]",
        pDefaultBearerVaConfig->qos.qci, pDefaultBearerVaConfig->qos.dlGbr,
        pDefaultBearerVaConfig->qos.ulGbr, pDefaultBearerVaConfig->qos.dlMbr,
        pDefaultBearerVaConfig->qos.ulMbr, pDefaultBearerVaConfig->emergency_ind,
        pDefaultBearerVaConfig->pcscf_discovery_flag, pDefaultBearerVaConfig->signalingFlag,
        pDefaultBearerVaConfig->assigned_rate);
    }
}
/// @}

// LCE URC start
void onLceDataResult(void* param)
{
    // +ELCE: <last_hop_cap>,<confidenc_level>,<lce_suspend>
    TimeCallbackParam* timeCallbackParam = (TimeCallbackParam*)param;
    char* urc = (char*)(timeCallbackParam->urc);
    RIL_SOCKET_ID rid = timeCallbackParam->rid;
    int ret;
    unsigned int t_last_hop_capacity_kbps = 0;
    unsigned int t_confidence_level = 0;
    unsigned int t_lce_suspended = 0;
    RIL_LceDataInfo* response = NULL;

    LOGD("[%d][%s] URC : %s", rid, __FUNCTION__, urc);

    response = (RIL_LceDataInfo *) calloc(1, sizeof(RIL_LceDataInfo));
    if (response == NULL) {
        LOGE("[%s] calloc failed!", __FUNCTION__);
        goto error;
    }

    ret = at_tok_start(&urc);
    if (ret < 0) goto error;

    ret = at_tok_nextint(&urc, &t_last_hop_capacity_kbps);
    if (ret < 0) goto error;

    if (at_tok_hasmore(&urc)) {
        ret = at_tok_nextint(&urc, &t_confidence_level);
        if (ret < 0) goto error;

        if (at_tok_hasmore(&urc)) {
            ret = at_tok_nextint(&urc, &t_lce_suspended);
            if (ret < 0) goto error;
        }
    }

    response->last_hop_capacity_kbps = t_last_hop_capacity_kbps;
    response->confidence_level = (unsigned char) t_confidence_level;
    response->lce_suspended = (unsigned char) t_lce_suspended;

    RIL_UNSOL_RESPONSE(RIL_UNSOL_LCEDATA_RECV, response, sizeof(RIL_LceDataInfo), rid);
    goto finish;

error:
    LOGE("[%d][%s] parsing error", rid, __FUNCTION__);

finish:
    LOGV("[%d][%s] finish", rid, __FUNCTION__);
    FREEIF(timeCallbackParam->urc);
    FREEIF(timeCallbackParam);
    FREEIF(response);
}

void onLinkCapacityEstimate(void* param) {
    FREEIF(param);
    LOGD("[%s] do nothing", __FUNCTION__);
}
// LCE URC end

// M: [C2K][IRAT] code start
static void onNwDetach(void* param) {
    RIL_SOCKET_ID rilid = *((RIL_SOCKET_ID *) param);

    if (nIratAction == IRAT_ACTION_SOURCE_STARTED
            || nIratAction == IRAT_ACTION_SOURCE_FINISHED) {
        LOGD("[RILData_GSM_URC] Ignore source NW DETACH during IRAT: status = %d",
                nIratAction);
        return;
    }

    if (s_md_off) {
        LOGD("onNwDetach(): modem off!");
        RIL_UNSOL_RESPONSE(RIL_UNSOL_DATA_CALL_LIST_CHANGED, NULL, 0, rilid);
    } else {
        LOGD("onNwDetach(): before data call list");
        onDataCallListChanged(param);
    }
}
// M: [C2K][IRAT] code end

// Fast Dormancy
void requestSetFdMode(void * data, size_t datalen, RIL_Token t) {
    UNUSED(datalen);

    int paramNumber = 0;
    int mode = 0;
    int parameter1 = 0;
    int parameter2 = 0;
    ATResponse *p_response = NULL;
    int err = 0;
    char cmdLine[PROPERTY_VALUE_MAX] = {0};
    memset(cmdLine, 0, sizeof(cmdLine));
    char timerValue[12] = {0};
    memset(timerValue, 0, sizeof(timerValue));

    paramNumber = ((int *)data)[0];
    LOGV("requestSetFdMode:paramNumber = %d", paramNumber);

    if (paramNumber == 1) {
        snprintf(cmdLine, sizeof(cmdLine), "AT+EFD=%d", ((int *)data)[1]);
    } else if (paramNumber == 2) {
        snprintf(cmdLine, sizeof(cmdLine), "AT+EFD=%d,%d", ((int *)data)[1], ((int *)data)[2]);
    } else if (paramNumber == 3) {
        parameter1 = ((int *)data)[2];
        parameter2 = ((int *)data)[3];

        // Set timer property.
        snprintf(timerValue, sizeof(timerValue), "%d", parameter2);
        property_set(PROPERTY_FD_TIMER[parameter1], timerValue);

        snprintf(cmdLine, sizeof(cmdLine), "AT+EFD=%d,%d,%d", ((int *)data)[1], parameter1,
                parameter2);
    } else {
        LOGE("requestSetFdMode: weird, should never be here!");
    }

    err = at_send_command(cmdLine, &p_response, DATA_CHANNEL_CTX);

    if (isATCmdRspErr(err, p_response)) {
        goto error;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    AT_RSP_FREE(p_response);
    return;

error:
    RIL_onRequestComplete(t, RIL_E_MODEM_ERR, NULL, 0);
    AT_RSP_FREE(p_response);
}

/**********************************************************
 * MSIM Section Start
**********************************************************/

/***************  Basic Function  ***************/
void requestAllowData(void* data, size_t datalen, RIL_Token t) {
    int isDataAllowed = ((int *)data)[0];
    LOGI("requestAllowData: isDataAllowed= %x", isDataAllowed);
    if (isMultiPsAttachSupport()) {
        setDataAllowed(isDataAllowed, t);
    } else {
        if (isDataAllowed == REQUEST_DATA_ATTACH) {
            requestDataAttach(data, datalen, t);
        } else {
            requestDataIdle(data, datalen, t);
        }
    }
}

void requestDataAttach(void* data, size_t datalen, RIL_Token t) {
    int isDataAttach = ((int *) data)[0];
    int err = 0;
    int cause = 0;
    ATResponse *p_response = NULL;

    LOGD("[RILData_GSM] requestDataAttach start, isDataAttach=%d", isDataAttach);
    // Set to auto attach mode so that modem will do PS re-attach when network detach happened
    err = at_send_command("AT+EGTYPE=4", &p_response, DATA_CHANNEL_CTX);
    if (isATCmdRspErr(err, p_response)) {
        int retry = 0;
        int retryInterval = 500 * 1000;  // for usleep 0.5 sec
        if (p_response != NULL && p_response->success == 0) {
            cause = at_get_cme_error(p_response);
        }
        LOGE("requestDataAttach fail, retry PS attach, cause: %d", cause);
        if (cause == DC_MSIM_MULTI_ATTACH_ERROR) {
            // multi-attach, err = at_get_cme_error(p_response);
            goto errorConflict;
        }
        AT_RSP_FREE(p_response);
        while (retry < MAX_RETRY_PS_ATTACH) {
            // General retry, up to 2 min
            retryInterval = (1 << (retry >> 2)) * 1000 * 1000;  // retry 1 1 1 1 2 2 2 2 4... sec ...
            usleep(retryInterval);
            LOGD("requestDataAttach retry#%d", retry);
            err = at_send_command("AT+EGTYPE=4", &p_response, DATA_CHANNEL_CTX);
            if (isATCmdRspErr(err, p_response)) {
                retry++;
                LOGV("requestDataAttach retry fail");
                if (MAX_RETRY_PS_ATTACH == retry) {
                    LOGE("requestDataAttach fail to set connetion type");
                    goto error;
                }
                AT_RSP_FREE(p_response);
            } else {
                break;
            }
        }
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    AT_RSP_FREE(p_response);
    LOGD("requestDataAttach finish");
    return;

error:
    LOGE("[RILData_GSM] requestDataAttach response ERROR");
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    AT_RSP_FREE(p_response);
    return;
errorConflict:
    LOGE("[RILData_GSM] requestDataAttach response ERROR, multi-attach");
    RIL_onRequestComplete(t, convertReqErrToOemErr(RIL_REQUEST_ALLOW_DATA, cause), NULL, 0);
    AT_RSP_FREE(p_response);
}

void requestDataIdle(void* data, size_t datalen, RIL_Token t) {
    RILChannelCtx* pChannel = getRILChannelCtxFromToken(t);
    int err = 0;

    do {
        err = detachPs(pChannel);
    } while (err == -1);

    if (t != NULL) {
        if (err == 0) {
            RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        } else {
            RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
        }
    }
}

int detachPs(RILChannelCtx* pChannel) {
    ATResponse *p_response = NULL;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(pChannel);
    int err = 0;
    int isError = 0;
    int isWhenNeededError = 0;
    int skipDetach = 0;
    int detachCount = 0;
    int isInserted = 1;

    LOGD("requestDataIdle");

    if (getSimCount() >= 2) {
        isInserted = isSimInserted(rid);
    }

    err = at_send_command((isInserted == 1)? "AT+EGTYPE=0" : "AT+EGTYPE=0,1", &p_response, pChannel);
    if (isATCmdRspErr(err, p_response)) {
        if (err == AT_ERROR_CHANNEL_CLOSED) {
            goto error_channel_closed;
        }
        isWhenNeededError = 1;
    }
    AT_RSP_FREE(p_response);

    do {
        LOGD("requestDataIdle:detach2, in while & isWhenNeededError = %d", isWhenNeededError);
        if (isWhenNeededError) {
            // Due to previous AT+EGTYPE may failed and cause when needed mode is not set
            // we have to retry setting when needed mode
            err = at_send_command((isInserted == 1)? "AT+EGTYPE=0" : "AT+EGTYPE=0,1", &p_response, pChannel);
            if (isATCmdRspErr(err, p_response)) {
                isWhenNeededError = 1;
                if (err == AT_ERROR_CHANNEL_CLOSED) {
                    goto error_channel_closed;
                }
            } else {
                isWhenNeededError = 0;
            }
            AT_RSP_FREE(p_response)
        }

        err = at_send_command("AT+CGATT=0", &p_response, pChannel);
        if (isATCmdRspErr(err, p_response) || isWhenNeededError) {
            if (err == AT_ERROR_CHANNEL_CLOSED) {
                goto error_channel_closed;
            }
            // fail
            isError = 1;
            detachCount++;
            LOGE("requestDataIdle(): fail to detach, count=%d", detachCount);
            sleep(1);
        } else {
            // success
            LOGI("requestDataIdle:AT+CGATT=0->detach success ");
            isError = 0;
            skipDetach = 1;
        }

        AT_RSP_FREE(p_response);
    } while (skipDetach == 0 && detachCount < 50 && s_md_off == 0 && needDetachPs(rid));

    if (isError == 1) {
        if (isWhenNeededError == 0 && !needDetachPs(rid)) {
            LOGI("requestDataIdle: fail to CGATT, but radio off ->detach success");
        } else {
            goto error;
        }
    }

    AT_RSP_FREE(p_response);
    return 0;

error:
    AT_RSP_FREE(p_response);
    return 1;
error_channel_closed:
    LOGE("requestDataIdle(): failed due to channel closed");
    usleep(200 * 1000);
    AT_RSP_FREE(p_response);
    return -1;
}

int needDetachPs(RIL_SOCKET_ID rid) {
    int ret = 0;
    RIL_RadioState radio_state = getRadioState(rid);
    int simInserted = isSimInserted(rid);
    if (radio_state == RADIO_STATE_ON && simInserted == 1) {
        ret = 1;
    }

    LOGD("[RILData_GSM] needDetachPs: radio state is %d, SIM insert state is %d,return %d",
            radio_state, simInserted, ret);
    return ret;
}

// Non-blocking ps always attach
int doPsAttachAsync(void* param) {
    AsyncAttachParam* attach = (AsyncAttachParam*)param;
    ATResponse *p_response = NULL;
    int err = 0;
    char *cmd = NULL;

    asprintf(&cmd, "AT+EGTYPE=%d", attach->mode);
    LOGD("doPsAttachAsync:send AT command(EGTYPE=%d) to do non-blocking attach", attach->mode);
    err = at_send_command(cmd, &p_response, attach->pDataChannel);
    free(cmd);
    free(param);

    if (isATCmdRspErr(err, p_response)) {
        LOGE("asyncPsAttach fail to set connetion type");
        goto error;
    }
    AT_RSP_FREE(p_response);
    return 1;

error:
    AT_RSP_FREE(p_response);
    return 0;
}

/***************  PS Attach Enhancement  ***************/
void requestRegisterPs(void* data, size_t datalen, RIL_Token t) {
    int isDataAttach = ((int *)data)[0];
    LOGI("requestRegisterPs isDataAttach= %x", isDataAttach);

    if (isDataAttach == REQUEST_DATA_ATTACH) {
        requestDataAttach(data, datalen, t);
    } else {
        requestDataIdle(data, datalen, t);
    }
}

int doAttachBeforeRadioOn(RIL_SOCKET_ID rid, int onOff, RILChannelCtx *pChannel) {
    int ret = 0;

    if (isMultiPsAttachSupport()) {
        ret = combineAttachMultiAttach(rid, onOff, pChannel);
    } else {
        ret = combineAttachAndFollowOn(rid, onOff, pChannel);
    }
    return ret;
}

// Combine attach and follow on flag Start
int combineAttachAndFollowOn(RIL_SOCKET_ID rid, int onOff, RILChannelCtx *pChannel) {
    RILChannelCtx* pCommandChannel = NULL;
    ATResponse *p_response = NULL;
    int err = 0;
    int majorSim = RIL_get3GSIM();
    int currSim = rid + 1;
    int needAttach = 0;

    // Check point 1: Don't attach if turn off radio
    if (onOff == 0) {
        LOGI("[RILData_GSM] radio off, don't attach");
        return 0;
    }

    // Check point 2: Only do combine attach on main protocol
    if (majorSim != currSim) {
        LOGI("[RILData_GSM] CombineAttachAndFollowOn 3/4G:%d, rid:%d,"
                "not main protocol, don't combine attach", majorSim, rid);
        return 0;
    }

    // Get Data enable and default data
    LOGI("[RILData_GSM] CombineAttachAndFollowOn 3/4G:%d, rid:%d, SIM Cnt:%d",
            majorSim, rid, getSimCount());

    // Follow on
    if (isNeedFollowOn(rid) == 1) {
        err = at_send_command("AT+EGTYPE=3", &p_response, pChannel);
        if (isATCmdRspErr(err, p_response)) {
            LOGE("[RILData_GSM] set follow on flag fail");
        }
        AT_RSP_FREE(p_response);
    }
    if (isNeedCombineAttach(rid) == 1) {
        err = at_send_command("AT+EGTYPE=4", &p_response, pChannel);
        if (isATCmdRspErr(err, p_response)) {
            LOGE("[RILData_GSM] Send EGTYPE=4 fail");
        } else {
            updateDataAllowStatus((int) rid, true);
        }
        AT_RSP_FREE(p_response);
    }

    return err;
}

int isNeedFollowOn(RIL_SOCKET_ID rid) {
    int followOn = 0;
    int currSim = rid + 1;
    int followOnProp = 1;

    char propertyValue[PROPERTY_VALUE_MAX] = {0};
    property_get(PROPERTY_NEED_FOLLOW_ON, propertyValue, "1");
    followOnProp = atoi(propertyValue);

    switch (followOnProp) {
        case FOLLOW_ON_DISABLED:
            followOn = 0;
            break;
        case FOLLOW_ON_ENABLED:
            // Data on & current SIM is set to be dafault data SIM
            if (getDataEnable(rid) != 0 && getDefaultDataSim() == currSim) {
                followOn = 1;
            }
            break;
        case FOLLOW_ON_FORCED:
            followOn = 1;
            break;
        default:
            LOGI("isNeedFollowOn: invalid propVal!!!(%d)", followOnProp);
            break;
    }
    LOGI("isNeedFollowOn: %d", followOn);
    return followOn;
}

int isNeedCombineAttach(RIL_SOCKET_ID rid)
{
    int defaultSim = 0;
    int currSim = rid + 1;
    int combineAttachProp = 0;
    int needAttach = 0;
    char propertyValue[PROPERTY_VALUE_MAX] = {0};
    property_get(PROPERTY_NEED_COMBINED_ATTACH, propertyValue, "1");
    combineAttachProp = atoi(propertyValue);

    switch (combineAttachProp) {
        case COMBINED_ATTACH_DISABLED:
            needAttach = 0;
            break;
        case COMBINED_ATTACH_ENABLED:
            if (getSimCount() == 1) {
                // AOSP single card will not send ALLOW_DATA request, do combine attach by default
                needAttach = 1;
            } else {
                defaultSim = getDefaultDataSim();
                if (defaultSim != 0) {
                    if (defaultSim == currSim) {
                        // Default data on myself, and I am main protocol, do combine attach
                        needAttach = 1;
                        LOGI("isNeedCombineAttach C1: default data set on main protocol");
                    } else {
                        //Default data not on myself
                        LOGI("isNeedCombineAttach C3: default data not set on main protocol");
                    }
                } else {
                    //default data unset, do combine attach if SIM inserted
                    if (isSimInserted((currSim - 1)) == 1) {
                        needAttach = 1;
                        LOGI("isNeedCombineAttach C4: data unset, main card is inserted, attach");
                    } else {
                        LOGI("isNeedCombineAttach C5: data unset, main card is not inserted");
                    }
                }
            }
            break;
        case COMBINED_ATTACH_FORCED:
            needAttach = 1;
            break;
        default:
            LOGI("isNeedCombineAttach: invalid propVal!!!(%d)", combineAttachProp);
            break;
    }
    LOGI("isNeedCombineAttach: %d", needAttach);
    return needAttach;
}
// Combine attach and follow on flag End

// Re-attach when main protocal SIM hot plugin, because the combineAttach
// is skipped when power on because SIM is not inserted.
// Scenario: Only one 3G CDMA SIM inserted, plug out 3G CDMA SIM, plugin 4G CDMA SIM,
// UE will turn on SIM1's radio when 3G CDMA SIM plugged out for CTA case
// but combine attach will not attach SIM1 because SIM is not inserted.
void onReattachForSimHotPlug(void* param) {
    TimeCallbackParam* timeCallbackParam = (TimeCallbackParam*) param;
    AsyncAttachParam* reAttach = calloc(1, sizeof(AsyncAttachParam));
    if (reAttach == NULL) {
        LOGE("[%s] calloc failed!", __FUNCTION__);
        goto finish;
    }
    RIL_SOCKET_ID rid = timeCallbackParam->rid;

    int defaultDataSim = getDefaultDataSim();
    int n3gSimId = RIL_get3GSIM();
    RIL_RadioState radio_state = getRadioState(rid);

    reAttach->mode = ALWAYS_ATTACH_WITH_CHECK;
    reAttach->pDataChannel = getChannelCtxbyProxy();

    LOGD("[RILData_GSM] onReattachForSimHotPlug default:%d, radio_state:%d, n3gSimId:%d, rid:%d",
            defaultDataSim, radio_state, n3gSimId, rid);
    if ((defaultDataSim == 0) && (n3gSimId == (rid + 1)) && (radio_state == RADIO_STATE_ON)) {
        doPsAttachAsync(reAttach);
    }
finish:
    FREEIF(timeCallbackParam);
}

/***************  Multiple PS Attach  ***************/
// The use of setDataAllowed() is to indicate that which SIM is allowed to use data.
// AP should make sure both two protocols are NOT set to ON at the same time in the
// case of default data switch flow.
// <Usage> AT+EDALLOW= <data_allowed_on_off>
//                   0 -> off
//                   1 -> on
void setDataAllowed(int allowed, RIL_Token t) {
    ATResponse *p_response = NULL;
    int err = 0;
    int cause = 0;
    char *cmd = NULL;
    RIL_SOCKET_ID rilid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));

    LOGD("setDataAllow: setDataAllowed= %x", allowed);

    asprintf(&cmd, "AT+EDALLOW=%d", allowed);
    err = at_send_command(cmd, &p_response, DATA_CHANNEL_CTX);
    free(cmd);

    if (isATCmdRspErr(err, p_response)) {
        LOGE("setDataAllowed: response error");
        if (p_response != NULL && p_response->success == 0) {
            // Return the error cause (e.g. 4117) for the upper to do error handling.
            cause = at_get_cme_error(p_response);
            if (cause == DC_MSIM_ALLOW_ERROR) {
                // multi-allow
                goto errorConflict;
            }
        }
        LOGD("setDataAllowed: response error");
        goto error;
    }
    AT_RSP_FREE(p_response);

    // Send URC to notify framework the EDALLOW is done.
    LOGD("setDataAllow: send URC RIL_UNSOL_DATA_ALLOWED allowed= %x ", allowed);
    RIL_UNSOL_RESPONSE(RIL_UNSOL_DATA_ALLOWED, &allowed, sizeof(int), rilid);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;

error:
    AT_RSP_FREE(p_response);
    LOGE("setDataAllowed: modem response ERROR cause = ", cause);
    RIL_onRequestComplete(t, cause, NULL, 0);
    return;

errorConflict:
    LOGE("setDataAllowed response ERROR, multi-allow");
    RIL_onRequestComplete(t, convertReqErrToOemErr(RIL_REQUEST_ALLOW_DATA, cause), NULL, 0);
    AT_RSP_FREE(p_response);
    return;
}

int combineAttachMultiAttach(RIL_SOCKET_ID rid, int onOff, RILChannelCtx *pChannel) {
    int majorSim = RIL_get3GSIM();
    int currSim = rid + 1;
    ATResponse *p_response = NULL;
    int err = 0;

    LOGD("combineAttachMultiAttach");
    RILChannelCtx* pDataChannel = getChannelCtxbyProxy();
    // Set follow on flag.
    if (isNeedFollowOn(rid)) {
        AsyncAttachParam *followOn = calloc(1, sizeof(AsyncAttachParam));
        if (followOn == NULL) {
            LOGE("[%s] calloc failed!", __FUNCTION__);
            return 1;
        }
        followOn->mode = FOLLOW_ON_FLAG;

        // setRadioPower use NW channel, we have to use the same channel to avoid command occupy.
        if (majorSim == currSim) {
            followOn->pDataChannel = pChannel;
            doPsAttachAsync(followOn);
        } else {
            followOn->pDataChannel = pDataChannel;
            RIL_requestProxyTimedCallback(doPsAttachAsync, followOn,
                    &TIMEVAL_0, pDataChannel->id, "doPsAttachAsync");
        }
    }

    // Set PS attach.
    if (onOff == 1) {
        AsyncAttachParam* attach = calloc(1, sizeof(AsyncAttachParam));
        if (attach == NULL) {
            LOGE("[%s] calloc failed for onOff = 1!", __FUNCTION__);
            return 1;
        }
        attach->mode = ALWAYS_ATTACH;

        if (majorSim == currSim) {
            attach->pDataChannel = pChannel;
            doPsAttachAsync(attach);
        } else {
            attach->pDataChannel = pDataChannel;
            RIL_requestProxyTimedCallback(doPsAttachAsync, attach,
                    &TIMEVAL_0, pDataChannel->id, "doPsAttachAsync");
        }
    }

    return 1;
}

/**********************************************************
 * MSIM Section End
**********************************************************/

/**********************************************************
 * Data Util Section Start
**********************************************************/
int getDataEnable(RIL_SOCKET_ID rid)
{
    int ret = 0, i = 0, dataPhone = 0;
    char dataEnable[PROPERTY_VALUE_MAX] = {0};
    char iccid[PROPERTY_VALUE_MAX] = {0};
    char maskIccid[PROPERTY_VALUE_MAX] = {0};
    char *token = NULL;

    if (isSimInserted(rid) == 0) {
        LOGV("[RILData_GSM] SIM%d not inserted", rid);
        return ret;
    }

    property_get(PROPERTY_MOBILE_DATA_ENABLE, dataEnable, "");
    if(strlen(dataEnable) == 0) {
        LOGV("Data enable not set");
    } else {
        getIccidbySimSlot(iccid, rid);
        givePrintableIccid(iccid, maskIccid);
        LOGI("getDataEnable: iccid = %s", maskIccid);
        token = strtok(dataEnable, ",");
        while (token != NULL) {
            if (strlen(token) != 0 && strStartsWith(token, iccid)) {
                LOGV("getDataEnable: matched");
                ret = 1;
                break;
            }
            token = strtok(NULL, ",");
        }
    }
    return ret;
}

int getDefaultDataSim()
{
    int i = 0;
    char iccid[PROPERTY_VALUE_MAX] = {0};
    char defaultData[PROPERTY_VALUE_MAX] = {0};
    char maskIccid[PROPERTY_VALUE_MAX] = {0};

    property_get(PROPERTY_RIL_DATA_ICCID, defaultData, "");
    if (strlen(defaultData) == 0 || strcmp("N/A", defaultData) == 0) {
        LOGI("Default Data not set");
        return 0;
    }

    givePrintableIccid(defaultData, maskIccid);
    LOGI("ICCID of Default Data is %s", maskIccid);

    int simCount = getSimCount();
    for (i = 0; i < simCount; i++) {
        property_get(PROPERTY_ICCID_SIM[i], iccid, "");
        if (strlen(iccid) == 0 || strcmp("N/A", iccid) == 0) {
            LOGV("SIM%d not inserted", i);
            continue;
        }
        if (strStartsWith(iccid, defaultData)) {
            givePrintableIccid(iccid, maskIccid);
            LOGV("Default data is SIM%d, ICCID = %s", (i + 1), maskIccid);
            return (i + 1);
        }
    }
    LOGD("Default Data SIM not found");
    return 0;
}

void getIccidbySimSlot(char *iccid, RIL_SOCKET_ID rid) {
    char maskIccid[PROPERTY_VALUE_MAX] = {0};
    property_get(PROPERTY_ICCID_SIM[rid], iccid, "");
    givePrintableIccid(iccid, maskIccid);

    LOGD("[RILData_GSM] SIM%d Iccid:%s",rid, maskIccid);
    return;
}

int convertReqErrToOemErr(int reqId, int errCause) {
    if (reqId == RIL_REQUEST_ALLOW_DATA) {
        switch (errCause) {
            case DC_MSIM_MULTI_ATTACH_ERROR:
            case DC_MSIM_ALLOW_ERROR:
                return RIL_E_OEM_ERROR_1;
            default:
                LOGI("convertReqErrToOemErr: invalid errCause(%d) for request(%d)", errCause, reqId);
                break;
        }
    }
    return RIL_E_INVALID_RESPONSE;
}

void onSimInsertChangedForData(RIL_SOCKET_ID rid, const char* urc) {
    clearAttachApnCacheIfNecessary(rid, urc);

    // ReAttach for CTA case
    if (strStartsWith(urc, "+ESIMS: 1,12")) {
        TimeCallbackParam* param = (TimeCallbackParam*) calloc(1,
                sizeof(TimeCallbackParam));
        if (param == NULL) {
            LOGE("[%d][%s] param calloc failed.", rid, __FUNCTION__);
            return;
        }
        param->rid = rid;
        RIL_requestProxyTimedCallback(onReattachForSimHotPlug, param,
                &TIMEVAL_0, getRILChannelId(RIL_DATA, rid), "onReattachForSimHotPlug");
     }
}

/**********************************************************
 * Data Util Section End
**********************************************************/

// COPY_FROM_MTK_RIL_DATA @{
int isSupportBearerRdpCommand()
{
    //TODO: here we should figure out how to know if RDP command is supported
    return 1;
}

static bool isFallbackPdpRetryFail(RIL_SOCKET_ID rid)
{
    LOGD("isFallbackPdpRetryFail, cause = 0x%x", gprs_failure_cause);

    char operatorNumeric[PROPERTY_VALUE_MAX] = {0};
    property_get((char *)PROPERTY_NW_MCCMNC[rid], operatorNumeric, "");

    //Check registered PLMN
    // M: Data Framework - CC 33 @{
    if (0 == strncmp(operatorNumeric, TELCEL_PDP_RETRY_PLMN, PROPERTY_VALUE_MAX-1)
        || 0 == strncmp(operatorNumeric, TELCEL_PDP_RETRY_PLMN2, PROPERTY_VALUE_MAX-1)
        || 0 == strncmp(operatorNumeric, CLARO_PDP_RETRY_PLMN, PROPERTY_VALUE_MAX-1)) {
        LOGD("isFallbackPdpRetryFail registered in CC33 operator [%s]", operatorNumeric);
        if (isCC33Support() == 0) {
            return (gprs_failure_cause != SM_ONLY_IPV4_ALLOWED) &&
                    (gprs_failure_cause != SM_ONLY_IPV6_ALLOWED);
        }
    // M: Data Framework - CC 33 @}
    // M: [General Operator] Data Framework - WWOP requirements: Telstra @{
    } else if (0 == strncmp(operatorNumeric, TELSTRA_PDP_RETRY_PLMN, PROPERTY_VALUE_MAX-1)) {
        LOGD("isFallbackPdpRetryFail registered in Telstra 505-01 [%s]", operatorNumeric);
        return (gprs_failure_cause == SM_INSUFFICIENT_RESOURCES) ||
                (gprs_failure_cause == SM_MISSING_UNKNOWN_APN) ||
                (gprs_failure_cause == SM_UNKNOWN_PDP_ADDRESS_TYPE) ||
                (gprs_failure_cause == SM_USER_AUTHENTICATION_FAILED) ||
                (gprs_failure_cause == SM_ACTIVATION_REJECT_GGSN) ||
                (gprs_failure_cause == SM_ACTIVATION_REJECT_UNSPECIFIED) ||
                (gprs_failure_cause == SM_SERVICE_OPTION_NOT_SUPPORTED) ||
                (gprs_failure_cause == SM_SERVICE_OPTION_NOT_SUBSCRIBED) ||
                (gprs_failure_cause == SM_SERVICE_OPTION_OUT_OF_ORDER) ||
                (gprs_failure_cause == SM_OPERATOR_BARRED) ||
                (gprs_failure_cause == SM_NETWORK_FAILURE) ||
                (gprs_failure_cause == SM_ONLY_SINGLE_BEARER_ALLOWED);
    // M: [General Operator] Data Framework - WWOP requirements: Telstra @}
    } else {
        LOGD("isFallbackPdpRetryFail common");
        return (gprs_failure_cause != SM_ONLY_IPV4_ALLOWED) &&
                (gprs_failure_cause != SM_ONLY_IPV6_ALLOWED) &&
                (gprs_failure_cause != SM_OPERATOR_BARRED) &&
                (gprs_failure_cause != SM_MISSING_UNKNOWN_APN) &&
                (gprs_failure_cause != SM_UNKNOWN_PDP_ADDRESS_TYPE) &&
                (gprs_failure_cause != SM_USER_AUTHENTICATION_FAILED) &&
                (gprs_failure_cause != SM_ACTIVATION_REJECT_GGSN) &&
                (gprs_failure_cause != SM_SERVICE_OPTION_NOT_SUPPORTED) &&
                (gprs_failure_cause != SM_SERVICE_OPTION_NOT_SUBSCRIBED) &&
                (gprs_failure_cause != SM_NSAPI_IN_USE) &&
                (gprs_failure_cause != SM_PROTOCOL_ERROR);
    }

    return false;
}

void initialCidTable() {
    getPdpCapability(NULL);
    pdnInfoSize = max_pdp_number > max_pdn_support ? max_pdp_number : max_pdn_support;

    int simCount = getSimCount();
    int i = 0;
    for (i = 0; i < simCount; i++) {
        if (pdn_info[i] == NULL) {
            pdn_info[i] = malloc(pdnInfoSize * sizeof(PdnInfo));
            if (pdn_info[i] == NULL) {
                LOGE("[%s] calloc for pdn_info[i] failed!", __FUNCTION__);
                return;
            }
            int j = 0;
            for (j = 0; j < pdnInfoSize; j++) {
                clearPdnInfo(&pdn_info[i][j]);
            }
            LOGI("[%d]Pdn information is initizlized, size = [%d, %d, %d]",
                 i, max_pdp_number, max_pdn_support, pdnInfoSize);
        }
    }

    // Wifi PDN
    LOGD("Wifi Pdn information is initizlized, size = [%d, %d]",
            simCount, WIFI_MAX_PDP_NUM);
    for (i = 0; i < simCount; i++) {
        if (pdn_info_wifi[i] == NULL) {
            pdn_info_wifi[i] = calloc(1, WIFI_MAX_PDP_NUM * sizeof(PdnInfo));
            int j = 0;
            for (j = 0; j < WIFI_MAX_PDP_NUM; j++) {
                clearPdnInfo(&pdn_info_wifi[i][j]);
            }
        }
    }

    // Temp EPDG PDN info for handover
    for (i = 0; i < simCount; i++) {
        if (pdn_info_l2w[i] == NULL) {
            pdn_info_l2w[i] = calloc(1, sizeof(PdnInfo));
            clearPdnInfo(pdn_info_l2w[i]);
        }
    }
}

void clearPdnInfo(PdnInfo* info)
{
    memset(info, 0, sizeof(PdnInfo));
    info->interfaceId = INVALID_CID;
    info->primaryCid = INVALID_CID;
    info->cid = INVALID_CID;
    info->bearerId = INVALID_CID;
    info->ddcId = INVALID_CID;
    info->mtu = 0;  // means not be specified
    info->reason = NO_CAUSE;

    info->hoStatus.interfaceId = INVALID_CID;
    info->hoStatus.ucho_status = HO_STATUS_INVALID;
}

/// M: Ims Data Framework {@
void clearRdsSocketHandle(){
    if (g_rds_conn_ptr != NULL) {
        if (rild_rds_conn_exit != NULL) {
            rild_rds_conn_exit(g_rds_conn_ptr);
            g_rds_conn_ptr = NULL;
            LOGD("[%s] Call rild_rds_conn_exit success", __FUNCTION__);
        } else {
            LOGE("[%s] rild_rds_conn_exit is null", __FUNCTION__);
        }
    }
    LOGD("clearRdsSocketHandle");
}

void initialMalDataCallResponse(
            mal_datamngr_data_call_info_req_t* responses, int length) {
    int i = 0;
    for (i=0; i<length; i++) {
        memset(&responses[i], 0, sizeof(mal_datamngr_data_call_info_req_t));
        responses[i].status = PDP_FAIL_ERROR_UNSPECIFIED;
        responses[i].cid = INVALID_CID;
    }
}
/// @}


void clearAllPdnInfoInDetail()
{
    int simCount = getSimCount();
    int i = 0;
    for (i = 0; i < simCount; i++) {
        if (pdn_info[i] != NULL) {
            int j = 0;
            for (j = 0; j < pdnInfoSize; j++) {
                clearPdnInfo(&pdn_info[i][j]);
                pdn_info[i][j].active = DATA_STATE_INACTIVE;
            }
        }
        LOGD("[%d]clearAllPdnInfoInDetail completed", i);
    }
}

int getAvailableCid(RIL_SOCKET_ID rid)
{
    int i = 0;
    //start from index 1 since we should not select CID0
    //CID0 is for network attach given PDN connection
    for(i = 1; i < pdnInfoSize; i++) {
        if (pdn_info[rid][i].interfaceId == INVALID_CID &&
                pdn_info[rid][i].active == DATA_STATE_INACTIVE && prevAvailableCid[rid] != i) {
            prevAvailableCid[rid] = i;
            LOGD("[%d]The available CID is %d", rid, i);
            return i;
        }
    }

    return INVALID_CID;
}

int getAvailableCidBasedOnSpecificCid(int sCid, RIL_SOCKET_ID rid)
{
    int i = 0;
    //start from specific cid since we should consider some cases, like this,
    //CID0 is for network attach given PDN connection,
    //CID1 is reserved for IA MD fallback.
    for(i = sCid; i < pdnInfoSize; i++) {
        if (pdn_info[rid][i].interfaceId == INVALID_CID &&
                pdn_info[rid][i].active == DATA_STATE_INACTIVE) {
            LOGD("[%d]The available CID is %d", rid, i);
            return i;
        }
    }

    return INVALID_CID;
}

int getAvailableWifiCid(RIL_SOCKET_ID rid)
{
    int i = 0;
    for (i = 0; i < WIFI_MAX_PDP_NUM; i++) {
        if (pdn_info_wifi[rid][i].interfaceId == INVALID_CID && pdn_info_wifi[rid][i].active == DATA_STATE_INACTIVE) {
            LOGD("[%d]The available wifi CID is %d", rid, i);
            return i;
        }
    }
    return INVALID_CID;
}

void getPdpCapability(RIL_Token t)
{
    ATResponse *p_response = NULL;
    int err = 0;
    int response23G = 1;
    int response4G = 1;
    char *line = NULL;
    ATLine *p_cur = NULL;

    getCcmniIfname();
    getL2pValue(t);

    if (t) {
        err = at_send_command_multiline("AT+CGDCONT=?", "+CGDCONT:", &p_response, DATA_CHANNEL_CTX);
    } else {
        LOGI("The ril token is null, use URC instead");
        err = at_send_command_multiline("AT+CGDCONT=?", "+CGDCONT:", &p_response,
                getChannelCtxbyProxy());
    }
    if (isATCmdRspErr(err, p_response)) {
        LOGE("AT+CGDCONT=? response error");
        goto error;
    } else {
        //The response would be the following liness
        //+CGDCONT: (1-6),"IP",,,(0),(0),(0-1)
        //+CGDCONT: (1-6),"IPV6",,,(0),(0),(0-1)
        //+CGDCONT: (1-6),"IPV4V6",,,(0),(0),(0-1)
        for (p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next) {
            line = p_cur->line;
            err = at_tok_start(&line);
            if (err < 0) continue;

            char* range;
            char* tok;
            int value[2]={0};
            int count = 0;
            err = at_tok_nextstr(&line, &range);
            if (err < 0) continue;

            tok = strtok (range,"(-)");
            while (tok != NULL)
            {
              value[count] = atoi(tok);
              count++;
              tok = strtok (NULL, "(-)");
            }
            LOGD("Support range is: (%d - %d)", value[0], value[1]);

            if(value[0] == 0) {
                isCid0Support = 1;
            }

            if((value[1] - value[0] + 1) > max_pdp_number) {
                max_pdp_number = value[1] - value[0] + 1;
            }
        }
    }
    AT_RSP_FREE(p_response);

    if (t) {
        at_send_command("AT+EPDNCTRANS=1", NULL, DATA_CHANNEL_CTX);
        err = at_send_command_singleline("AT+EGPCAP=?", "+EGPCAP:", &p_response, DATA_CHANNEL_CTX);
    } else {
        LOGI("The ril token is null, use URC instead");
        at_send_command("AT+EPDNCTRANS=1", NULL, getChannelCtxbyProxy());
        err = at_send_command_singleline("AT+EGPCAP=?", "+EGPCAP:", &p_response,
                getChannelCtxbyProxy());
    }

    if (isATCmdRspErr(err, p_response)) {
        LOGI("Not supoprt PDP capability command");
    } else {
        line = p_response->p_intermediates->line;

        err = at_tok_start(&line);
        if (err < 0) goto error;

        err = at_tok_nextint(&line, &max_pdn_support_23G);
        if (err < 0) goto error;

        err = at_tok_nextint(&line, &max_pdn_support_4G);
        if (max_pdn_support_4G > 0)
            max_pdn_support_4G++;
        if (err < 0) goto error;
    }
    AT_RSP_FREE(p_response);

    max_pdn_support = max_pdn_support_4G > max_pdn_support_23G ? max_pdn_support_4G : max_pdn_support_23G;
    if (!isCid0Support) {
        ++max_pdn_support;
        ++max_pdp_number;
        //when CID0 is not support, the index of pdn list array should be start from 1
        //so we increase the array size by 1 so that the array range is from 1 to max cid
    }

    LOGD("Current PDP capability is [%d, %d, %d, %d, %d]", isCid0Support, max_pdn_support,
            max_pdn_support_23G, max_pdn_support_4G, max_pdp_number);
    return;
error:
    AT_RSP_FREE(p_response);
    LOGE("get PDP capability failed, current PDP capability [%d, %d, %d, %d %d]", isCid0Support,
            max_pdn_support, max_pdn_support_23G, max_pdn_support_4G, max_pdp_number);

}

void getCcmniIfname() {
    int enableMd1 = 0;
    char prop_value[PROPERTY_VALUE_MAX] = { 0 };

    property_get("ro.vendor.mtk_md1_support", prop_value, "0");
    enableMd1 = atoi(prop_value);

    if (enableMd1) {
        snprintf(s_ccmni_ifname, sizeof(s_ccmni_ifname), "%s", CCMNI_IFNAME_CCMNI);
    } else {
        snprintf(s_ccmni_ifname, sizeof(s_ccmni_ifname), "%s", CCMNI_IFNAME_CCEMNI);
    }

    LOGD("CCMNI IFNAME: %s", s_ccmni_ifname);
}

void getL2pValue(RIL_Token t) {

    ATResponse *p_response = NULL;
    char* line = NULL;
    char* out;
    int err = 0;
    bool bSupportCcmni = false;

    if (t) {
        err = at_send_command_singleline("AT+CGDATA=?", "+CGDATA:", &p_response, DATA_CHANNEL_CTX);
    } else {
        LOGI("The ril token is null, use URC instead");
        err = at_send_command_singleline("AT+CGDATA=?", "+CGDATA:", &p_response, getChannelCtxbyProxy());
    }

    if (err < 0 || p_response->success == 0) {
        LOGE("AT+CGDATA=? response error");
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    while(at_tok_hasmore(&line)) {
        err = at_tok_nextstr(&line, &out);
        if (err < 0) goto error;

        if(strcmp(L2P_VALUE_CCMNI, out) == 0) {
            bSupportCcmni = true;
            break;
        }
    }
    AT_RSP_FREE(p_response);

    if(bSupportCcmni == true) {
        snprintf(s_l2p_value, sizeof(s_l2p_value), "%s", L2P_VALUE_CCMNI);
    } else {
        snprintf(s_l2p_value, sizeof(s_l2p_value), "%s", L2P_VALUE_UPS);
    }

    LOGD("L2P Value: %s",s_l2p_value);
    return;

error:
    AT_RSP_FREE(p_response);
    snprintf(s_l2p_value, sizeof(s_l2p_value), "%s", L2P_VALUE_UPS);
    LOGE("Get L2P value fail, set L2P value as %s", s_l2p_value);
    return;
}

int getAuthTypeInt(int authTypeInt) {
    //Application 0->none, 1->PAP, 2->CHAP, 3->PAP/CHAP;
    //Modem 0->PAP, 1->CHAP, 2->NONE, 3->PAP/CHAP;
    switch (authTypeInt) {
        case 0:
            return AUTHTYPE_NONE;
        case 1:
            return AUTHTYPE_PAP;
        case 2:
            return AUTHTYPE_CHAP;
        case 3:
            return AUTHTYPE_PAP_CHAP;
        default:
            return AUTHTYPE_NOT_SET;
    }
}

int updatePdnAddressByCid(int cid, RILChannelCtx* rilchnlctx)
{
    int err = -1;
    char *line = NULL, *out = NULL;
    char *cmd = NULL;
    ATResponse *p_response = NULL;
    int responseCid = -1;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(rilchnlctx);

    /* Use AT+CGPADDR=cid to query the ip address assigned to this PDP context indicated via this cid */
    asprintf(&cmd, "AT+CGPADDR=%d", cid);
    err = at_send_command_singleline(cmd, "+CGPADDR:", &p_response, rilchnlctx);
    free(cmd);

    /* Parse the response to get the ip address */
    if (isATCmdRspErr(err, p_response)) {
        goto error;
    }
    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    /* line => +CGPADDR: <cid>,<PDP_addr_1>,<PDP_addr_2> */

    if (err < 0)
        goto error;

    /* Get 1st parameter: CID */
    err = at_tok_nextint(&line, &responseCid);
    if (err < 0)
        goto error;

    /* Get 2nd parameter: IPAddr1 */
    err = at_tok_nextstr(&line, &out);
    if (err < 0)
        goto error;

    if (get_address_type(out) == IPV4)
        snprintf(pdn_info[rid][responseCid].addressV4, sizeof(pdn_info[rid][responseCid].addressV4), "%s", out);
    else {
        /// M: Ims Data Framework @{
        if (!isOp16Support()) {
            convert_ipv6_address(out, pdn_info[rid][responseCid].addressV6, 1);
        } else {
            epdgHandoverStatus_t* hoStatus = &pdn_info[rid][responseCid].hoStatus;
            if (hoStatus->ucho_status == HO_STATUS_INVALID) {
                convert_ipv6_address(out, pdn_info[rid][responseCid].addressV6, 1);
            } else if ((hoStatus->esource_ran_type != RDS_RAN_WIFI || hoStatus->etarget_ran_type != RDS_RAN_MOBILE_3GPP) &&
                (hoStatus->esource_ran_type != RDS_RAN_WIFI || hoStatus->etarget_ran_type != RDS_RAN_MOBILE_3GPP2)){
                convert_ipv6_address(out, pdn_info[rid][responseCid].addressV6, 1);
            }
        }
        /// @}
    }
    /* Only for IPv4V6 + dual stack PDP context */
    /* Get 3rd paramter: IPaddr 2*/
    if ( at_tok_hasmore(&line)) {
        err = at_tok_nextstr(&line, &out);
        if (err < 0) {
            goto error;
        }

        if (get_address_type(out) == IPV4)
            snprintf(pdn_info[rid][responseCid].addressV4, sizeof(pdn_info[rid][responseCid].addressV4), "%s", out);
        else {
            /// M: Ims Data Framework @{
            if (!isOp16Support()) {
                convert_ipv6_address(out, pdn_info[rid][responseCid].addressV6, 1);
            } else {
                epdgHandoverStatus_t* hoStatus = &pdn_info[rid][responseCid].hoStatus;
                if (hoStatus->ucho_status == HO_STATUS_INVALID) {
                    convert_ipv6_address(out, pdn_info[rid][responseCid].addressV6, 1);
                } else if ((hoStatus->esource_ran_type != RDS_RAN_WIFI || hoStatus->etarget_ran_type != RDS_RAN_MOBILE_3GPP) &&
                    (hoStatus->esource_ran_type != RDS_RAN_WIFI || hoStatus->etarget_ran_type != RDS_RAN_MOBILE_3GPP2)){
                    convert_ipv6_address(out, pdn_info[rid][responseCid].addressV6, 1);
                }
            }
            /// @}
        }
    }
    AT_RSP_FREE(p_response);
    return 0;
error:
    /* Free the p_response for +CGPADDR: */
    AT_RSP_FREE(p_response);
    return -1;
}

int isCidActive(int *cidList, int size, RILChannelCtx * rilchnlctx) {
    int i;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(rilchnlctx);
    updateActiveStatus(rilchnlctx);
    for (i = 0; i < size; i++) {
        int cid = cidList[i];
        LOGV("pdn_info[%d][%d], state: %d", rid, cid, pdn_info[rid][cid].active);
        if (DATA_STATE_INACTIVE == pdn_info[rid][cid].active) return 0;
    }
    return 1;
}

int updateActiveStatus(RILChannelCtx* rilchnlctx) {
    ATResponse *p_response = NULL;
    ATLine *p_cur;
    int maxCidSupported = 0;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(rilchnlctx);

    int err = at_send_command_multiline ("AT+CGACT?", "+CGACT:", &p_response, rilchnlctx);
    if (isATCmdRspErr(err, p_response)) {
        LOGE("updateActiveStatus AT+CGACT? response ERROR");
        AT_RSP_FREE(p_response);
        return 0;
    }

    for (p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next)
        maxCidSupported++;

    LOGD("[%d]updateActiveStatus PDP capability [%d %d %d %d %d]", rid,
        maxCidSupported, max_pdn_support, max_pdn_support_23G, max_pdn_support_4G, max_pdp_number);

    int actChars = 0;
    int inactChars = 0;
    int charBufSize = pdnInfoSize * 10;
    char* activeCidsBuf = calloc(1, charBufSize * sizeof(char));
    if (activeCidsBuf == NULL) {
        LOGE("[%s] allocate activeCidsBuf fail!!", __FUNCTION__);
        goto finish;
    }
    char* inactiveCidsBuf = calloc(1, charBufSize * sizeof(char));
    if (inactiveCidsBuf == NULL) {
        LOGE("[%s] allocate inactiveCidsBuf fail!!", __FUNCTION__);
        goto finish;
    }
    for (p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next) {
        char *line = p_cur->line;

        err = at_tok_start(&line);
        if (err < 0)
            break;

        int responseCid = 0;
        err = at_tok_nextint(&line, &responseCid);
        if (err < 0)
            break;

        err = at_tok_nextint(&line, &pdn_info[rid][responseCid].active);
        if (err < 0)
            break;

        if (pdn_info[rid][responseCid].active > 0) {
            // 0=inactive, 1=active/physical link down, 2=active/physical link up
            if (pdn_info[rid][responseCid].interfaceId == INVALID_CID) {
                pdn_info[rid][responseCid].active = DATA_STATE_LINKDOWN;
                LOGD("[%d]updateActiveStatus CID%d is linkdown", rid, responseCid);
            } else {
                pdn_info[rid][responseCid].active = DATA_STATE_ACTIVE;
                LOGD("[%d]updateActiveStatus CID%d is active", rid, responseCid);
            }

            if (pdn_info[rid][responseCid].cid == INVALID_CID) {
                pdn_info[rid][responseCid].cid = responseCid;
                pdn_info[rid][responseCid].primaryCid = responseCid;
            }

            actChars += snprintf(activeCidsBuf + actChars, charBufSize - actChars,
                                 "%d:%d, ", responseCid, pdn_info[rid][responseCid].primaryCid);
        } else {
            pdn_info[rid][responseCid].active = DATA_STATE_INACTIVE;
            if (inactChars < charBufSize) {
                inactChars += snprintf(inactiveCidsBuf + inactChars, charBufSize - inactChars,
                                       "%d:%d, ", responseCid, pdn_info[rid][responseCid].primaryCid);
            }
        }
    }
    if (strlen(activeCidsBuf) > 0) {
        LOGI("[%d]updateActiveStatus active list [CID:primary CID] = [%s]", rid, activeCidsBuf);
    }
    if (strlen(inactiveCidsBuf) > 0) {
        LOGD("[%d]updateActiveStatus inactive list [CID:primary CID] = [%s]", rid, inactiveCidsBuf);
    }

finish:
    FREEIF(activeCidsBuf);
    FREEIF(inactiveCidsBuf);

    AT_RSP_FREE(p_response);

    //this may not be real max supported CID
    //should use getPdpCapability to know PDP capability
    return maxCidSupported;
}

int updateDns(RILChannelCtx* rilchnlctx)
{
    char *line = NULL, *out = NULL;
    char *cmd = NULL;
    ATLine *p_cur = NULL;
    ATResponse *p_response = NULL;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(rilchnlctx);

    //Use AT+CGPRCO? to query each cid's dns server address
    //+CGPRCO:<cid>,"<dnsV4-1>","<dnsV4-2>","<dnsV6-1>","<dnsV6-2>"
    int err = at_send_command_multiline("AT+CGPRCO?", "+CGPRCO:", &p_response, rilchnlctx);
    if (isATCmdRspErr(err, p_response))
        goto error;

    int i = 0;
    for (i = 0; i<pdnInfoSize; i++) {
        //clear DNS information in CID table since we would query it again
        memset(pdn_info[rid][i].dnsV4, 0, sizeof(pdn_info[rid][i].dnsV4));
        memset(pdn_info[rid][i].dnsV6, 0, sizeof(pdn_info[rid][i].dnsV6));
    }

    for (p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next) {
        line = p_cur->line;
        err = at_tok_start(&line);

        if (err < 0)
            goto error;

        /* Get 1st parameter: CID */
        int responseCid = -1;
        err = at_tok_nextint(&line, &responseCid);
        if (err < 0)
            goto error;

        int i = 0;
        int v4Count = 0;
        int v6Count = 0;
        for (i = 0; i < MAX_NUM_DNS_ADDRESS_NUMBER*2; i++) {
            if (!at_tok_hasmore(&line))
                break;

            err = at_tok_nextstr(&line, &out);
            if (err < 0)
                goto error;

            if (out != NULL && strlen(out) > 0 && strcmp(NULL_IPV4_ADDRESS, out) != 0) {
                if (get_address_type(out) == IPV6) {
                    convert_ipv6_address(out, pdn_info[rid][responseCid].dnsV6[v6Count], 0);
                    LOGD("updateDns DNS%d V6 for cid%d", v6Count+1, responseCid);
                    ++v6Count;
                } else {
                    strncpy(pdn_info[rid][responseCid].dnsV4[v4Count], out, MAX_IPV4_ADDRESS_LENGTH-1);
                    LOGD("updateDns DNS%d V4 for cid%d", v4Count+1, responseCid);
                    ++v4Count;
                }
            }
        }

        // <inital attach>, only skip now
        if (at_tok_hasmore(&line)) {
            int isIa = 0;
            err = at_tok_nextint(&line, &isIa);
            if (err < 0)
                goto error;
        }

        if (at_tok_hasmore(&line)) {
            // <mtu size>, default is 0 (if query fail, modem will return 0)
            int mtu = 0;
            err = at_tok_nextint(&line, &mtu);
            if (err < 0)
                goto error;
            if (mtu != 0) {
                LOGD("updateDns mtu for cid%d: %d", responseCid, pdn_info[rid][responseCid].mtu);
                pdn_info[rid][responseCid].mtu = mtu;
            }
        }

    }

    AT_RSP_FREE(p_response);
    return 0;
error:
    /* Free the p_response for +CGPADDR: */
    AT_RSP_FREE(p_response);
    return -1;
}

/// M: Ims Data Framework @{
int updateQos(RILChannelCtx* rilchnlctx)
{
    ATResponse *p_response = NULL;
    ATLine *p_cur;
    int i;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(rilchnlctx);

    for (i=0; i<pdnInfoSize; i++) {
        //clear QOS information in CID table since we would query it again
        memset(&pdn_info[rid][i].qos, 0, sizeof(Qos));
    }
    //AT+CGEQOS? responses a list of [+CGEQOS: <cid>,<QCI>,[<DL_GBR>,<UL_GBR>],[<DL_MBR>,<UL_MBR>]]
    //AT+CGEQOSRDP response a list of [+CGEQOSRDP: <cid>,<QCI>,[<DL_GBR>,<UL_GBR>],[<DL_MBR>,<UL_MBR>][,<DL_AMBR>,<UL_AMBR>]]
    //AT+CGEQREQ? responses a list of [+CSEQREQ: <cid>,<Traffic class>,<Maximum bitrate UL>,
    //<Maximum bitrate DL> ,<Guaranteed bitrate UL>,<Guaranteed bitrate DL>,<Delivery order>,
    //<Maximum SDU size>,<SDU error ratio>,<Residual bit error ratio>,<Delivery of erroneous SDUs>,
    //<Transfer delay>,<Traffic handling priority>
    int err = at_send_command_multiline ("AT+CGEQOSRDP", "+CGEQOSRDP:", &p_response, rilchnlctx);
    if (isATCmdRspErr(err, p_response)) {
        LOGE("updateQos AT+CGEQOSRDP response ERROR");
        goto error;
    }

    for (p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next) {
        char *line = p_cur->line;

        err = at_tok_start(&line);
        if (err < 0)
            goto error;

        int responseCid = 0;
        err = at_tok_nextint(&line, &responseCid);
        if (err < 0)
            goto error;

        err = at_tok_nextint(&line, &pdn_info[rid][responseCid].qos.qci);
        if (err < 0)
            goto error;

        if (at_tok_hasmore(&line)) {
            if (*line != ',') {
                err = at_tok_nextint(&line, &pdn_info[rid][responseCid].qos.dlGbr);
                if (err < 0)
                    goto error;

                err = at_tok_nextint(&line, &pdn_info[rid][responseCid].qos.ulGbr);
                if (err < 0)
                    goto error;
            } else {
                // skip comma
                line++;
            }
            if (at_tok_hasmore(&line)) {
                if (*line != ',') {
                    err = at_tok_nextint(&line, &pdn_info[rid][responseCid].qos.dlMbr);
                    if (err < 0)
                        goto error;

                    err = at_tok_nextint(&line, &pdn_info[rid][responseCid].qos.ulMbr);
                    if (err < 0)
                        goto error;
                } else {
                    // skip comma
                    line++;
                }
            }
            // TODO: parse and store DL&UL AMBR value
        }
    }
    AT_RSP_FREE(p_response);
    return 0;
error:
    AT_RSP_FREE(p_response);
    return -1;
}

int updateTft(RILChannelCtx* rilchnlctx)
{
    ATResponse *p_response = NULL;
    ATLine *p_cur;
    int i = 0;
    char* out = NULL;
    int hasDot = 0;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(rilchnlctx);

    for (i=0; i<pdnInfoSize; i++) {
        //clear Tft information in CID table since we would query it again
        memset(&pdn_info[rid][i].tft, 0, sizeof(Tft));
    }

    //AT+CGTFTRDP responses a list of [+CGTFTRDP: <cid>,<packet filter identifier>,<evaluation precedence index>,
    //<remote address and subnet mask>,<protocol number (ipv4) / next header (ipv6)>,<local port range>,
    //<remote port range>,<ipsec security parameter index (spi)>,<type of service (tos) (ipv4) and mask / traffic class (ipv6) and mask>,
    //<flow label (ipv6)>,<direction>,<NW packet filter Identifier>]
    //AT +CGTFT? responses a list of [+CGTFT: <cid>,<pktFilterId>,<evalPrecIdx>,<addr_n_mask>, <protocolNum>,<dstPortRange>,<srcPortRange>,
    //<spi>,<tos_n_mask>, <flow_label>]....for R8
    int err = at_send_command_multiline ("AT+CGTFTRDP", "+CGTFTRDP:", &p_response, rilchnlctx);
    if (isATCmdRspErr(err, p_response)) {
        LOGE("updateTft AT+CGTFTRDP response ERROR");
        goto error;
    }

    for (p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next) {
        char *line = p_cur->line;

        err = at_tok_start(&line);
        if (err < 0)
            goto error;

        int responseCid = 0;
        err = at_tok_nextint(&line, &responseCid);
        if (err < 0)
            goto error;

        PktFilter* pkFilter = &(pdn_info[rid][responseCid].tft.pfList[pdn_info[rid][responseCid].tft.pfNumber]);

        err = at_tok_nextint(&line, &pkFilter->id);
        if (err < 0)
            goto error;

        err = at_tok_nextint(&line, &pkFilter->precedence);
        if (err < 0)
            goto error;

        char *addressAndMask = NULL;
        err = at_tok_nextstr(&line, &addressAndMask); //remote address and subnet mask
        if (err < 0)
            goto error;

        int dotCount = 0;
        int length = strlen(addressAndMask);
        for(i = 0; i<length; i++) {
            if(addressAndMask[i] == '.')
                ++dotCount;
        }

        int innerDotCount = 0;
        for(i = 0; i<length; i++) {
            if(addressAndMask[i] == '.')
                ++innerDotCount;

            if (dotCount > 7 && innerDotCount == 16) {//the end of IPv6 address
                strncpy(pkFilter->address, addressAndMask, i);
                strncpy(pkFilter->mask, addressAndMask + i + 1,
                    MIN(sizeof(pkFilter->mask), (sizeof(addressAndMask) - i + 1))); //add one since there is a dot between address and mask

                pkFilter->bitmap |= PKT_BITMASK_IMC_BMP_V6_ADDR;
                break;
            } else if (dotCount <= 7 && innerDotCount == 4) {//the end of IPv4 address
                strncpy(pkFilter->address, addressAndMask, i);
                strncpy(pkFilter->mask, addressAndMask + i + 1,
                    MIN(sizeof(pkFilter->mask), (sizeof(addressAndMask) - i + 1))); //add one since there is a dot between address and mask

                pkFilter->bitmap |= PKT_BITMASK_IMC_BMP_V4_ADDR;
                break;
            }
        }

        err = at_tok_nextstr(&line, &out);
        if (err < 0) {
            goto error;
        } else if (strlen(out) > 0) {
            pkFilter->protocolNextHeader = atoi(out);
            pkFilter->bitmap |= PKT_BITMASK_IMC_BMP_PROTOCOL;
        }

        char* tokenStr = NULL;
        err = at_tok_nextstr(&line, &tokenStr);
        if (err < 0) {
            goto error;
        } else if (strlen(tokenStr) > 0) {
            length = strlen(tokenStr);
            for (i = 0; i<length; i++) {
                if (tokenStr[i] == '.') {
                    hasDot = 1;
                    char str[10] = {0};
                    strncpy(str, tokenStr, i);
                    pkFilter->localPortLow = atoi(str);
                    pkFilter->localPortHigh = atoi(tokenStr+i+1); //add one since there is a dot between low and high
                    LOGD("updateTft get packet filter[%d] local port [%d, %d]", pkFilter->id, pkFilter->localPortLow, pkFilter->localPortHigh);
                    break;
                }
            }

            if(!hasDot && length) {
                pkFilter->localPortLow = atoi(tokenStr);
                pkFilter->localPortHigh = pkFilter->localPortLow;
            }
            hasDot = 0;

            if (pkFilter->localPortLow == pkFilter->localPortHigh)
                pkFilter->bitmap |= PKT_BITMASK_IMC_BMP_LOCAL_PORT_SINGLE;
            else
                pkFilter->bitmap |= PKT_BITMASK_IMC_BMP_LOCAL_PORT_RANGE;
        }

        err = at_tok_nextstr(&line, &tokenStr);
        if (err < 0) {
            goto error;
        } else if (strlen(tokenStr) > 0) {
            length = strlen(tokenStr);
            for (i = 0; i<length; i++) {
                if (tokenStr[i] == '.') {
                    hasDot = 1;
                    char str[10] = {0};
                    strncpy(str, tokenStr, i);
                    pkFilter->remotePortLow = atoi(str);
                    pkFilter->remotePortHigh = atoi(tokenStr+i+1); //add one since there is a dot between low and high
                    LOGD("updateTft get packet filter[%d] remote port [%d, %d]", pkFilter->id, pkFilter->remotePortLow, pkFilter->remotePortHigh);
                    break;
                }
            }
            if(!hasDot && length) {
                pkFilter->remotePortLow = atoi(tokenStr);
                pkFilter->remotePortHigh = pkFilter->remotePortLow;
            }
            hasDot = 0;


            if (pkFilter->remotePortLow == pkFilter->remotePortHigh)
                pkFilter->bitmap |= PKT_BITMASK_IMC_BMP_REMOTE_PORT_SINGLE;
            else
                pkFilter->bitmap |= PKT_BITMASK_IMC_BMP_REMOTE_PORT_RANGE;
        }

        err = at_tok_nextstr(&line, &tokenStr); //hex string
        if (err < 0) {
            goto error;
        } else if (strlen(tokenStr) > 0) {
            pkFilter->spi = (int)strtol(tokenStr, NULL, 16);
            pkFilter->bitmap |= PKT_BITMASK_IMC_BMP_SPI;
        }

        err = at_tok_nextstr(&line, &tokenStr);
        if (err < 0) {
            goto error;
        } else if (strlen(tokenStr) > 0) {
            length = strlen(tokenStr);
            for(i = 0; i<length; i++) {
                if(tokenStr[i] == '.') {
                    char str[10] = {0};
                    strncpy(str, tokenStr, i);
                    pkFilter->tos = atoi(str);
                    pkFilter->tosMask = atoi(tokenStr+i+1); //add one since there is a dot between tos and mask
                    LOGD("updateTft get packet filter[%d] tos [%d, %d]", pkFilter->id, pkFilter->tos, pkFilter->tosMask);

                    pkFilter->bitmap |= PKT_BITMASK_IMC_BMP_TOS;
                    break;
                }
            }
        }

        err = at_tok_nextstr(&line, &tokenStr); //hex string
        if (err < 0) {
            goto error;
        } else if (strlen(tokenStr) > 0) {
            pkFilter->flowLabel = (int)strtol(tokenStr, NULL, 16);
            pkFilter->bitmap |= PKT_BITMASK_IMC_BMP_FLOW_LABEL;
        }

        err = at_tok_nextint(&line, &pkFilter->direction);
        if (err < 0)
            goto error;

        err = at_tok_nextint(&line, &pkFilter->networkPfIdentifier);
        if (err < 0)
            goto error;

        pdn_info[rid][responseCid].tft.pfNumber++;
    }
    AT_RSP_FREE(p_response);


    //+EGLNKPF? The read command returns current setting of linked packet filter(s)
    //response a list of [+EGLNKPF: <cid>, <linked packet filter identifier>, K]
    err = at_send_command_multiline ("AT+EGLNKPF?", "+EGLNKPF:", &p_response, rilchnlctx);
    if (isATCmdRspErr(err, p_response)) {
        LOGE("updateTft AT+EGLNKPF? response ERROR");
        goto error;
    }

    for (p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next) {
        char *line = p_cur->line;

        err = at_tok_start(&line);
        if (err < 0)
            goto error;

        int responseCid = 0;
        err = at_tok_nextint(&line, &responseCid);
        if (err < 0)
            goto error;

        do {
            err = at_tok_nextstr(&line, &out); //linked packet filter identifier
            if (err < 0)
                goto error;
            pdn_info[rid][responseCid].tft.tftParameter.linkedPfList[pdn_info[rid][responseCid].tft.tftParameter.linkedPfNumber] = atoi(out);
            ++pdn_info[rid][responseCid].tft.tftParameter.linkedPfNumber;
        } while (at_tok_hasmore(&line));
    }
    AT_RSP_FREE(p_response);
    return 0;

error:
    AT_RSP_FREE(p_response);
    return -1;
}
/// @}

int updateDefaultBearerInfo(RILChannelCtx* rilchnlctx)
{
    //Be aware that this method is used only for retrieving P-CSCF address and signaling flag
    ATResponse *p_response = NULL;
    ATLine *p_cur;
    int count = 0;
    int i;
    int* pcscfCount = NULL;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(rilchnlctx);

    for (i=0; i<pdnInfoSize; i++) {
        //clear default bearer information in CID table since we would query it again
        memset(pdn_info[rid][i].pcscf, 0, MAX_PCSCF_NUMBER * MAX_IPV6_ADDRESS_LENGTH * sizeof(char));
        memset(pdn_info[rid][i].apn, 0, sizeof(pdn_info[rid][i].apn));
        pdn_info[rid][i].signalingFlag = 0;
        pdn_info[rid][i].bearerId = INVALID_CID;
    }

    //+CGCONTRDP response a list of [+CGCONTRDP: <cid>,<bearer_id>,<apn>[,<local_addr and subnet_mask>[,<gw_addr>
    //[,<DNS_prim_addr>[,<DNS_sec_addr>[,<P-CSCF_prim_addr>[,<P-CSCF_sec_addr>[,<IM_CN_Signalling_Flag>[,<LIPA_indication>]]]]]]]]]
    int err = at_send_command_multiline ("AT+CGCONTRDP", "+CGCONTRDP:", &p_response, rilchnlctx);
    if (isATCmdRspErr(err, p_response)) {
        LOGE("updateDefaultBearerInfo AT+CGCONTRDP response ERROR");
        goto error;
    }

    for (p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next)
        ++count;

    LOGD("[%d]updateDefaultBearerInfo AT+CGCONTRDP response count [%d]", rid, count);

    if (count == 0) {
        LOGE("updateDefaultBearerInfo AT+CGCONTRDP response success but no intermediate");
        goto done;
    }

    pcscfCount = (int*)calloc(1, pdnInfoSize * sizeof(int));
    if (pcscfCount == NULL) {
        LOGE("[%s] calloc for pcscfCount failed!", __FUNCTION__);
        goto error;
    }
    memset(pcscfCount, 0, pdnInfoSize * sizeof(int));

    for (p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next) {
        char *line = p_cur->line;

        err = at_tok_start(&line);
        if (err < 0)
            goto error;

        int responseCid = 0;
        int bearerId = 0;
        char* apn = NULL;

        err = at_tok_nextint(&line, &responseCid);
        if (err < 0)
            goto error;

        err = at_tok_nextint(&line, &pdn_info[rid][responseCid].bearerId);
        if (err < 0)
            goto error;

        err = at_tok_nextstr(&line, &apn); //apn
        if (err < 0)
            goto error;

        if (strlen(apn) > 0)
            strncpy(pdn_info[rid][responseCid].apn, apn, strlen(apn));

        if (at_tok_hasmore(&line)) {
            char* out = NULL;

            err = at_tok_nextstr(&line, &out); //local_addr and subnet_mask
            if (err < 0)
                goto error;
        } else {
            continue;
        }

        if (at_tok_hasmore(&line)) {
            char* out = NULL;

            err = at_tok_nextstr(&line, &out); //gw_addr
            if (err < 0)
                goto error;
        } else {
            continue;
        }

        if (at_tok_hasmore(&line)) {
            char* out = NULL;

            err = at_tok_nextstr(&line, &out); //DNS_prim_addr
            if (err < 0)
                goto error;
        } else {
            continue;
        }

        if (at_tok_hasmore(&line)) {
            char* out = NULL;

            err = at_tok_nextstr(&line, &out); //DNS_sec_addr
            if (err < 0)
                goto error;
        } else {
            continue;
        }

        if (at_tok_hasmore(&line)) {
            char* out = NULL;

            err = at_tok_nextstr(&line, &out); //P-CSCF_prim_addr
            if (err < 0)
                goto error;

            if (strlen(out) > 0){
                strncpy(pdn_info[rid][responseCid].pcscf[pcscfCount[responseCid]++], out, MAX_IPV6_ADDRESS_LENGTH-1);
            }
        } else {
            continue;
        }

        if (at_tok_hasmore(&line)) {
            char* out = NULL;

            err = at_tok_nextstr(&line, &out); //P-CSCF_sec_addr
            if (err < 0)
                goto error;

            if (strlen(out) > 0){
                strncpy(pdn_info[rid][responseCid].pcscf[pcscfCount[responseCid]++], out, MAX_IPV6_ADDRESS_LENGTH-1);
            }
        } else {
            continue;
        }

        if (at_tok_hasmore(&line)) {
            char* out = NULL;

            err = at_tok_nextint(&line, &pdn_info[rid][responseCid].signalingFlag); //IM_CN_Signalling_Flag
            if (err < 0)
                goto error;
        } else {
            continue;
        }
    }

done:
    AT_RSP_FREE(p_response);
    if (pcscfCount)
        free(pcscfCount);
    return 0;
error:
    AT_RSP_FREE(p_response);
    if (pcscfCount)
        free(pcscfCount);
    return -1;

}

/// M: Ims Data Framework @{
int updateDedicateBearerInfo(RILChannelCtx* rilchnlctx)
{
    ATResponse *p_response = NULL;
    ATLine *p_cur;
    int count = 0;
    int i;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(rilchnlctx);

    for (i=0; i<pdnInfoSize; i++) {
        if (pdn_info[rid][i].isDedicateBearer) {
            LOGD("[%d]updateDedicateBearerInfo clear pdn_info[%d]", rid, i);
            //clear dedicate bearer information in CID table since we would query it again
            pdn_info[rid][i].primaryCid = INVALID_CID;
            pdn_info[rid][i].bearerId = INVALID_CID;
            pdn_info[rid][i].signalingFlag = 0;
            pdn_info[rid][i].cid = INVALID_CID;;
            pdn_info[rid][i].interfaceId = INVALID_CID;
            pdn_info[rid][i].active = 0;
        }
    }

    //+CGSCONTRDP responses a list of [+CGSCONTRDP: <cid>,<p_cid>,<bearer_id>[,<IM_CN_Signalling_Flag>]]
    int err = at_send_command_multiline ("AT+CGSCONTRDP", "+CGSCONTRDP:", &p_response, rilchnlctx);
    if (isATCmdRspErr(err, p_response)) {
        LOGE("[%d]updateDedicateBearerInfo AT+CGSCONTRDP response ERROR", rid);
        goto error;
    }

    for (p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next)
        ++count;

    if (count > 0) {
        for (p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next) {
            char *line = p_cur->line;

            err = at_tok_start(&line);
            if (err < 0)
                goto error;

            int responseCid = INVALID_CID;
            err = at_tok_nextint(&line, &responseCid);
            if (err < 0)
                goto error;

            err = at_tok_nextint(&line, &pdn_info[rid][responseCid].primaryCid);
            if (err < 0)
                goto error;

            err = at_tok_nextint(&line, &pdn_info[rid][responseCid].bearerId);
            if (err < 0)
                goto error;

            if (at_tok_hasmore(&line)) {
                err = at_tok_nextint(&line, &pdn_info[rid][responseCid].signalingFlag);
                if (err < 0)
                    goto error;
            }

            //Update basic information
            pdn_info[rid][responseCid].cid = responseCid;
            pdn_info[rid][responseCid].isDedicateBearer = 1;
            pdn_info[rid][responseCid].interfaceId = pdn_info[rid][pdn_info[rid][responseCid].primaryCid].interfaceId;
            pdn_info[rid][responseCid].active = pdn_info[rid][pdn_info[rid][responseCid].primaryCid].active;

        }
    } else {
        LOGV("[%d]updateDedicateBearerInfo AT+CGSCONTRDP success but no intermediate", rid);
    }

    AT_RSP_FREE(p_response);
    return 0;
error:
    AT_RSP_FREE(p_response);
    return -1;
}

void updateDynamicParameter(RILChannelCtx* rilchnlctx)
{
    int isSupportBearerRdpCommand = 1;
    // Please reference to ALPS03513990
    if (s_md_off) {
        LOGE("updateDynamicParameter: skip due to modem off");
        return;
    }

    //TODO: here we should figure out how to know if RDP command is supported
    if (isSupportBearerRdpCommand) {
        if (updateDefaultBearerInfo(rilchnlctx) < 0)
            LOGE("updateDynamicParameter updateDefaultBearerInfo failed");

        if (updateDedicateBearerInfo(rilchnlctx) < 0)
            LOGE("updateDynamicParameter updateDedicateBearerInfo failed");

        if (updateQos(rilchnlctx) < 0)
            LOGE("updateDynamicParameter updateQos failed");

        if (updateTft(rilchnlctx) < 0)
            LOGE("updateDynamicParameter updateTft failed");
    } else {
        LOGV("updateDynamicParameter skip un-supported RDP command");
    }
}


int configureQos(int cid, Qos* qos, RILChannelCtx* rilchnlctx)
{
    char* cmd = NULL;
    ATResponse *p_response = NULL;
    int failCause = CME_SUCCESS;
    LOGD("configureQos CID=%d, QOS[qci=%d, dlGbr=%d, ulGbr=%d, dlMbr=%d, ulMbr=%d]",
            cid, qos->qci, qos->dlGbr, qos->ulGbr, qos->dlMbr, qos->ulMbr);
    //+CGEQOS=[<cid>[,<QCI>[,<DL_GBR>,<UL_GBR>[,<DL_MBR>,<UL_MBR]]]]
    //+CGEQREQ=[<cid>[,<Traffic class>[,<Maximum bitrate UL>[,<Maximum bitrate DL>
    //[,<Guaranteed bitrate UL>[,<Guaranteed bitrate DL>[,<Delivery order>[,<Maximum SDU size>
    //[,<SDU error ratio>[,<Residual bit error ratio>[,<Delivery of erroneous SDUs>[,<Transfer delay>
    //[,<Traffic handling priority>[,<Source statistics descriptor>[,<Signalling indication>]]]]]]]]]]]]]]]
    if (max_pdn_support_4G > 0) {
        asprintf(&cmd, "AT+CGEQOS=%d,%d,%d,%d,%d,%d", cid, qos->qci, qos->dlGbr, qos->ulGbr, qos->dlMbr, qos->ulMbr);
    } else {
        asprintf(&cmd, "AT+CGEQREQ=%d,%d,%d,%d,%d,%d", cid, qos->qci, qos->ulMbr, qos->dlMbr, qos->ulGbr, qos->dlGbr);
    }
    int err = at_send_command(cmd, &p_response, rilchnlctx);
    free(cmd);
    if (isATCmdRspErr(err, p_response)) {
        failCause = at_get_cme_error(p_response);
        LOGE("configureQos AT+CGEQREQ err=%d", failCause);
    }
    AT_RSP_FREE(p_response);

    return failCause;
}

int configureTft(int cid, Tft* tft, RILChannelCtx* rilchnlctx)
{
    int i, err;
    char* cmd = NULL;
    int failCause = CME_SUCCESS;
    ATResponse *p_response = NULL;
    LOGD("configureTft CID=%d, TFT[pfNumber=%d, operation=%d]", cid, tft->pfNumber, tft->operation);

    //IMC_TFT_OPCODE_SPARE = 0,       // (0x00) Spare
    //IMC_TFT_OPCODE_CREATE_NEW_TFT = 1,  // (0x01) Create new TFT
    //IMC_TFT_OPCODE_DELETE_TFT = 2,      // (0x02) Delete existing TFT
    //IMC_TFT_OPCODE_ADD_PF = 3,          // (0x03) Add packet filters to existing TFT
    //IMC_TFT_OPCODE_REPLACE_PF = 4,      // (0x04) Replace packet filters in existing TFT
    //IMC_TFT_OPCODE_DELETE_PF = 5,       // (0x05) Delete packet filters from existing TFT
    //IMC_TFT_OPCODE_NOTFT_OP = 6,        // (0x06) No TFT operation
    //IMC_TFT_OPCODE_RESERVED // (0x07) Reserved
    switch (tft->operation) {
        case 1: //create new TFT, this is used when activating a bearer
        case 3: //add packet filter, this is used when modifying a bearer
        case 4: //replace packet filter, this is used when modifying bearer
            for (i=0; i<tft->pfNumber; i++) {
                cmd = getConfigTftATCmd(cid, &tft->pfList[i]);
                err = at_send_command(cmd, &p_response, rilchnlctx);
                free(cmd);
                if (isATCmdRspErr(err, p_response)) {
                    failCause = at_get_cme_error(p_response);
                    LOGE("configureTft AT+CGTFT err=%d", failCause);
                }
                AT_RSP_FREE(p_response);
                if (failCause) goto end;
            }

            for (i=0; i<tft->tftParameter.linkedPfNumber; i++) {
                //+EGLNKPF=[<cid>, [<linked packet filter identifier>]]
                //The set command is used to specify a linked packet filter associated within a bearer modification (TFT or QoS modify transaction)
                asprintf(&cmd, "AT+EGLNKPF=%d,%d", cid, tft->tftParameter.linkedPfList[i]);
                err = at_send_command(cmd, &p_response, rilchnlctx);
                free(cmd);
                if (isATCmdRspErr(err, p_response)) {
                    failCause = at_get_cme_error(p_response);
                    LOGE("configureTft AT+EGLNKPF err=%d", failCause);
                }
                AT_RSP_FREE(p_response);
                if (failCause) goto end;
            }
            break;
        case 5: //delete packet filter, this is used when modify bearer
            //+EGDELPF=[<cid>, [<packet filter identifier>]]
            for (i=0; i<tft->pfNumber; i++) {
                asprintf(&cmd, "AT+EGDELPF=%d,%d", cid, tft->pfList[i].id);
                err = at_send_command(cmd, &p_response, rilchnlctx);
                free(cmd);
                if (isATCmdRspErr(err, p_response)) {
                    failCause = at_get_cme_error(p_response);
                    LOGE("configureTft AT+EGDELPF err=%d", failCause);
                }
                AT_RSP_FREE(p_response);
                if (failCause) goto end;
            }
            break;
        case 6:
            LOGD("configureTft receive No TFT operation");
            break;
        default:
            LOGE("configureTft receive unsupported operation [%d]", tft->operation);
            failCause = CME_ERROR_NON_CME;
    }

end:
    return failCause;
}
/// @}

// IP maybe changed in some cases.
void updateNetworkInterface(int interfaceId, int mask, RIL_SOCKET_ID rid) {
    LOGD("[%d]updateNetworkInterface interface %d, mask= %02x", rid, interfaceId, mask);
    if (interfaceId == INVALID_CID) {
        LOGE("[%d]interface %d is invalid, ignore update!", rid, interfaceId);
        return;
    }

    struct ifreq ifr;
    int sim_id = getValidSimId(rid, __FUNCTION__);
    mal_datamngr_trigger_rs_rsp_ptr_t rsp_ptr =
            calloc(1, sizeof(mal_datamngr_trigger_rs_rsp_t));

    memset(&ifr, 0, sizeof(struct ifreq));
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s%d", s_ccmni_ifname, interfaceId);

    // Reset connection only if IPv4/v6 address changed.
    if ((mask & 0x04) == 0) {
        ifc_reset_connections(ifr.ifr_name, mask);
    }

    ril_data_ioctl_init(interfaceId);

    int i = 0;
    for (i = 0; i < pdnInfoSize; i++) {
        if (pdn_info[rid][i].interfaceId == interfaceId) {
            if (strlen(pdn_info[rid][i].addressV4) > 0 && (mask & 0x01)) {
                ril_data_setflags(sock_fd[interfaceId], &ifr, IFF_UP, 0);
                ril_data_setaddr(sock_fd[interfaceId], &ifr, pdn_info[rid][i].addressV4);
            }

            if (strlen(pdn_info[rid][i].addressV6) > 0 && ((mask & 0x02) || (mask & 0x04))) {
                ril_data_setflags(sock6_fd[interfaceId], &ifr, IFF_UP, 0);
                if (mal_datamngr_trigger_rs_tlv != NULL) {
                    mal_datamngr_trigger_rs_tlv(mal_once(1, mal_cfg_type_sim_id, sim_id), ifr.ifr_name,
                                                rsp_ptr, 0, NULL, NULL, NULL);
                    LOGD("[%s] Call mal_datamngr_trigger_rs_tlv success", __FUNCTION__);
                } else {
                    LOGE("[%s] mal_datamngr_trigger_rs_tlv is null", __FUNCTION__);
                }
                int errNo = triggerIpv6Rs(ifr.ifr_name);
                if (errNo < 0) {
                    LOGD("updateNetworkInterface triggerIpv6Rs %s fail %d", ifr.ifr_name, errNo);
                }
                ril_data_set_ipv6addr(sock6_fd[interfaceId], &ifr, pdn_info[rid][i].addressV6);
            }
        }
    }

    FREEIF(rsp_ptr);
    close(sock_fd[interfaceId]);
    close(sock6_fd[interfaceId]);
    sock_fd[interfaceId] = 0;
    sock6_fd[interfaceId] = 0;
}

void configureNetworkInterface(int interfaceId, int isUp, RIL_SOCKET_ID rid)
{
    struct ifreq ifr;
    int ignoreReset = 0;
    //Configure the IP address to the CCMNI interface
    /* Open the network interface: CCMNI */
    LOGD("[%d]configureNetworkInterface interface %d to %s", rid, interfaceId, isUp ? "UP" : "DOWN");

    if (isUp == REACT_CCMNI) {
        // If Irat trigger react pdp, ignore reset.
        ignoreReset = 1;
    }

    if (interfaceId == INVALID_CID) {
        LOGE("[%d]interface %d is invalid, ignore config!", rid, interfaceId);
        return;
    }

    memset(&ifr, 0, sizeof(struct ifreq));
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s%d", s_ccmni_ifname, interfaceId);


    // MD Irat, the ccmni interfaces shared by MD1 & MD3
    if (isUp) {
        bindNetworkInterfaceWithModem(interfaceId, MD_SYS1);
    }


    if (isUp && (ignoreReset == 0) ) {
        // set the network interface down first before up
        // to prevent from unknown exception causing not close related
        // dev file description
        LOGV("[%d]set network interface down before up", rid);
        setNwIntfDown(ifr.ifr_name);
    }
    ril_data_ioctl_init(interfaceId);

    int i = 0;
    for (i = 0; i < pdnInfoSize; i++) {
        if (pdn_info[rid][i].interfaceId == interfaceId) {
            if (isUp) {
                if (strlen(pdn_info[rid][i].addressV4) > 0) {
                    ril_data_setflags(sock_fd[interfaceId], &ifr, IFF_UP, 0);
                    ril_data_setaddr(sock_fd[interfaceId], &ifr, pdn_info[rid][i].addressV4);
                }

                if (strlen(pdn_info[rid][i].addressV6) > 0) {
                    ril_data_setflags(sock6_fd[interfaceId], &ifr, IFF_UP, 0);
                    ril_data_set_ipv6addr(sock6_fd[interfaceId], &ifr, pdn_info[rid][i].addressV6);
                }
            } else {
                setNwIntfDown(ifr.ifr_name);
            }
        }
    }
    for (i = 0; i < WIFI_MAX_PDP_NUM; i++) {
        if (pdn_info_wifi[rid][i].interfaceId == interfaceId) {
            if (!isUp) {
                LOGE("[%d] Down Wifi PDN interface %d", rid, interfaceId);
                setNwIntfDown(ifr.ifr_name);
            }
        }
    }

    close(sock_fd[interfaceId]);
    close(sock6_fd[interfaceId]);
    sock_fd[interfaceId] = 0;
    sock6_fd[interfaceId] = 0;
}

void setNwIntfDown(const char * pzIfName) {
    LOGV("setNwIntfDown %s", pzIfName);
    ifc_reset_connections(pzIfName, RESET_ALL_ADDRESSES);
    ifc_remove_default_route(pzIfName);
    ifc_disable(pzIfName);
}

/// M: Ims Data Framework {@
void convertToGlobalAddress(char* addressV6, char* prefix) {
    LOGD("[%s]convert linklocal %s by prefix %s ", __FUNCTION__, addressV6, prefix);
    struct in6_addr ip;
    char result[MAX_IPV6_ADDRESS_LENGTH];

    int ret = inet_pton(AF_INET6, addressV6, &ip);
    if (ret >= 0) {
        snprintf(result, sizeof(result), "%s:%x:%x:%x:%x",
            prefix, htons(ip.s6_addr16[4]),htons(ip.s6_addr16[5]),
            htons(ip.s6_addr16[6]),htons(ip.s6_addr16[7]));
        strncpy(addressV6, result, sizeof(result));
    } else {
        LOGD("[%s]convert inet_pton fail ", __FUNCTION__);
    }
}

void createMalDataResponse(int interfaceId, int cid, MTK_RIL_Data_Call_Response_v11* response,
        mal_datamngr_data_call_info_req_t* responseToMal, RIL_SOCKET_ID rid)
{
    UNUSED(interfaceId);
    int i = 0;
    int defaultCid = -1;

    if (response->active != DATA_STATE_ACTIVE) {
        LOGD("[%s], cid: %d is not alive, skip data response creation",
            __FUNCTION__, response->cid);
        return;
    }

    //sync AOSP pdn info
    responseToMal->active = response->active;
    responseToMal->mtu = response->mtu;
    responseToMal->status = response->status;
    responseToMal->cid = response->cid;
    responseToMal->eran_type = response->rat;
    responseToMal->type = strdup(response->type);
    responseToMal->ifname = strdup(response->ifname);
    responseToMal->addresses = strdup(response->addresses);
    responseToMal->dnses = strdup(response->dnses);
    responseToMal->gateways = strdup(response->gateways);
    responseToMal->pcscf = strdup(response->pcscf);

    LOGD("[%s], find default cid: %d at pdn_info[%d][%d]", __FUNCTION__, cid, rid, i);
    createDedicateDataResponse(cid, &responseToMal->defaultBearer, rid);
}
/// @}

bool isPreferDnsV6First(RIL_SOCKET_ID rid) {
    return true;
}

int createDataResponse(int interfaceId, int protocol, MTK_RIL_Data_Call_Response_v11* response, RIL_SOCKET_ID rid)
{
    int activatedPdnNum = 0;
    int i = 0, nDnsesLen = 0;
    int targetPdnIndex = -1;
    char addressV4[MAX_IPV4_ADDRESS_LENGTH] = {0};
    char addressV6[MAX_IPV6_ADDRESS_LENGTH] = {0};
    char dnsV4[MAX_NUM_DNS_ADDRESS_NUMBER][MAX_IPV4_ADDRESS_LENGTH] = {{0}};
    char dnsV6[MAX_NUM_DNS_ADDRESS_NUMBER][MAX_IPV6_ADDRESS_LENGTH] = {{0}};
    char pcscf[MAX_PCSCF_NUMBER][MAX_IPV6_ADDRESS_LENGTH] = {{0}};

    int v4DnsLength = 0;
    int v6DnsLength = 0;
    int pcscfLength = 0;
    int defaultCidIdx = 0;
    const char *pAddrV6 = NULL;
    PdnInfo *pdnInfo = NULL;
    bool isOverEpdg = false;

    response->active = DATA_STATE_INACTIVE;

    for (i = 0; i < WIFI_MAX_PDP_NUM; i++) {
        if (pdn_info_wifi[rid][i].interfaceId == interfaceId &&
                pdn_info_wifi[rid][i].active == DATA_STATE_ACTIVE) {

            /// M: Ims Data Framework {@
            // Skip updating WIFI PND info if PDN on LTE is ready when W->L.
            if (hoLtePdnSuccess[rid][interfaceId] == true) {
                LOGD("[%d][%s] hoLtePdnSuccess: ignore wifi PDN for interfaceId=%d ",
                     rid, __FUNCTION__, interfaceId);
                break;
            }
            /// @}
            isOverEpdg = true;
            pdnInfo = &pdn_info_wifi[rid][i];
            targetPdnIndex = i;

            response->active = pdnInfo->active;
            asprintf(&response->ifname, "%s%d", s_ccmni_ifname, interfaceId);

            if (strlen(pdnInfo->addressV4) > 0 && (protocol == IPV4 || protocol == IPV4V6))
                strncpy(addressV4, pdnInfo->addressV4, MAX_IPV4_ADDRESS_LENGTH-1);
            if (strlen(pdnInfo->addressV6) > 0 && (protocol == IPV6 || protocol == IPV4V6))
                strncpy(addressV6, pdnInfo->addressV6, MAX_IPV6_ADDRESS_LENGTH-1);

            int j = 0;
            for (j = 0; j < MAX_NUM_DNS_ADDRESS_NUMBER; j++) {
                if (strlen(pdnInfo->dnsV4[j]) > 0) {
                    strncpy(dnsV4[j], pdnInfo->dnsV4[j], MAX_IPV4_ADDRESS_LENGTH-1);
                    v4DnsLength += strlen(pdnInfo->dnsV4[j]);
                    if (j != 0 || v6DnsLength != 0)
                        ++v4DnsLength;  // add one space
                }
                if (strlen(pdnInfo->dnsV6[j]) > 0) {
                    strncpy(dnsV6[j], pdnInfo->dnsV6[j], MAX_IPV6_ADDRESS_LENGTH-1);
                    v6DnsLength += strlen(pdnInfo->dnsV6[j]);
                    if (j != 0 || v4DnsLength != 0)
                        ++v6DnsLength;  // add one space
                }
            }

            for (j = 0; j < MAX_PCSCF_NUMBER; j++) {
                if (strlen(pdnInfo->pcscf[j]) > 0) {
                    strncpy(pcscf[j], pdnInfo->pcscf[j], MAX_IPV6_ADDRESS_LENGTH-1);
                    pcscfLength += strlen(pdnInfo->pcscf[j]);
                    if (j != 0)
                        ++pcscfLength;  // add one space
                }
            }

            defaultCidIdx = i + WIFI_CID_OFFSET;
            response->mtu = pdnInfo->mtu;
            response->rat = pdnInfo->rat;
        }
    }
    if (targetPdnIndex >= 0) {
        LOGD("[%s], copy pdn_info_wifi[%d][%d] to data response for ifid=%d",
                __FUNCTION__, rid, targetPdnIndex, interfaceId);
        targetPdnIndex = -1;
    }

    if (pdnInfo == NULL) {
        for (i = 0; i < pdnInfoSize; i++) {
            if (pdn_info[rid][i].interfaceId == interfaceId &&
                    !pdn_info[rid][i].isDedicateBearer &&
                    pdn_info[rid][i].active == DATA_STATE_ACTIVE) {
                pdnInfo = &pdn_info[rid][i];
                targetPdnIndex = i;
                // There is cid bind to the interface of the response, so set to active
                response->active = pdnInfo->active;
                asprintf(&response->ifname, "%s%d", s_ccmni_ifname, interfaceId);

                if (strlen(pdnInfo->addressV4) > 0 && (protocol == IPV4 || protocol == IPV4V6))
                    strncpy(addressV4, pdnInfo->addressV4, MAX_IPV4_ADDRESS_LENGTH-1);
                if (strlen(pdnInfo->addressV6) > 0 && (protocol == IPV6 || protocol == IPV4V6))
                    strncpy(addressV6, pdnInfo->addressV6, MAX_IPV6_ADDRESS_LENGTH-1);

                int j = 0;
                for (j = 0; j < MAX_NUM_DNS_ADDRESS_NUMBER; j++) {
                    if (strlen(pdnInfo->dnsV4[j]) > 0) {
                        strncpy(dnsV4[j], pdnInfo->dnsV4[j], MAX_IPV4_ADDRESS_LENGTH-1);
                        v4DnsLength += strlen(pdnInfo->dnsV4[j]);
                        if (j != 0 || v6DnsLength != 0)
                            ++v4DnsLength;  // add one space
                    }
                    if (strlen(pdnInfo->dnsV6[j]) > 0) {
                        strncpy(dnsV6[j], pdnInfo->dnsV6[j], MAX_IPV6_ADDRESS_LENGTH-1);
                        v6DnsLength += strlen(pdnInfo->dnsV6[j]);
                        if (j != 0 || v4DnsLength != 0)
                            ++v6DnsLength;  // add one space
                    }
                }

                for (j = 0; j < MAX_PCSCF_NUMBER; j++) {
                    if (strlen(pdnInfo->pcscf[j]) > 0) {
                        strncpy(pcscf[j], pdnInfo->pcscf[j], MAX_IPV6_ADDRESS_LENGTH-1);
                        pcscfLength += strlen(pdnInfo->pcscf[j]);
                        if (j != 0)
                            ++pcscfLength;  // add one space
                    }
                }

                defaultCidIdx = i;
                response->mtu = pdnInfo->mtu;
                response->rat = pdnInfo->rat;
            }
        }
    }
    if (targetPdnIndex >= 0) {
        LOGD("[%d][%s], copy pdn_info[%d] to data response for ifid=%d",
                rid, __FUNCTION__, targetPdnIndex, interfaceId);
    }

    response->status = PDP_FAIL_NONE;
    response->cid = interfaceId;

    int addressV4Length = strlen(addressV4);
    int addressV6Length = strcmp(addressV6, "::") == 0 ? 0 : strlen(addressV6);

    if (addressV4Length > 0 && addressV6Length > 0) {
        asprintf(&response->type, SETUP_DATA_PROTOCOL_IPV4V6);
        if (!isIpv6Global(addressV6) && strlen(pdnInfo->prefix) != 0) {
            convertToGlobalAddress(addressV6, pdnInfo->prefix);
        }
        asprintf(&response->addresses, "%s %s", addressV4, addressV6);

        if (isIpv6Global(addressV6)) {
            asprintf(&response->gateways, "%s %s", addressV4, UNSPECIFIED_IPV6_GATEWAY);
        } else {
            asprintf(&response->gateways, "%s", response->addresses);
        }
    } else if (addressV6Length > 0) {
        asprintf(&response->type, SETUP_DATA_PROTOCOL_IPV6);
        if (!isIpv6Global(addressV6) && strlen(pdnInfo->prefix) != 0) {
            convertToGlobalAddress(addressV6, pdnInfo->prefix);
        }
        asprintf(&response->addresses, "%s", addressV6);
        if (isIpv6Global(addressV6)) {
            asprintf(&response->gateways, "%s", UNSPECIFIED_IPV6_GATEWAY);
        } else {
            asprintf(&response->gateways, "%s", response->addresses);
        }
    } else {
        asprintf(&response->type, SETUP_DATA_PROTOCOL_IP);
        asprintf(&response->addresses, "%s", addressV4);
        asprintf(&response->gateways, "%s", response->addresses);
    }

    if (isOverEpdg) {
        char v6TextForm[MAX_IPV6_ADDRESS_LENGTH];
        inet_ntop(AF_INET6, addressV6, v6TextForm, MAX_IPV6_ADDRESS_LENGTH);
        bool isV6Set = strcmp(v6TextForm, "::") != 0;
        bool isV4Set = strlen(addressV4) != 0;

        if (isV4Set && isV6Set) {
            if (!isIpv6LinkLocal(addressV6)) {
                asprintf(&response->gateways, "%s %s", addressV4, UNSPECIFIED_IPV6_GATEWAY);
            } else {
                asprintf(&response->gateways, "%s %s", addressV4, addressV6);
            }
        } else if (isV6Set) {
            if (!isIpv6LinkLocal(addressV6)) {
                asprintf(&response->gateways, "%s", UNSPECIFIED_IPV6_GATEWAY);
            } else {
                asprintf(&response->gateways, "%s", addressV6);
            }
        } else if (isV4Set) {
            asprintf(&response->gateways, "%s", addressV4);
        }
    }

    nDnsesLen = v4DnsLength + v6DnsLength + 1;
    response->dnses = calloc(1, nDnsesLen * sizeof(char));
    if (response->dnses == NULL) {
        LOGE("[%s] calloc for response->dnses failed!", __FUNCTION__);
        return defaultCidIdx;
    }
    memset(response->dnses, 0, nDnsesLen * sizeof(char));

    for (i = 0; i < MAX_NUM_DNS_ADDRESS_NUMBER; i++) {
        if (isPreferDnsV6First(rid)) {
            if (v6DnsLength > 0 && strlen(dnsV6[i]) > 0) {
                int currentLength = strlen(response->dnses);
                if (currentLength > 0) {
                    strncat(response->dnses+currentLength, " ", 1);
                }
                strncat(response->dnses+strlen(response->dnses), dnsV6[i], strlen(dnsV6[i]));
            }
            if (v4DnsLength > 0 && strlen(dnsV4[i]) > 0) {
                int currentLength = strlen(response->dnses);
                if (currentLength > 0) {
                    strncat(response->dnses+currentLength, " ", 1);
                }
                strncat(response->dnses+strlen(response->dnses), dnsV4[i], strlen(dnsV4[i]));
            }
        } else {
            if (v4DnsLength > 0 && strlen(dnsV4[i]) > 0) {
                int currentLength = strlen(response->dnses);
                if (currentLength > 0) {
                    strncat(response->dnses+currentLength, " ", 1);
                }
                strncat(response->dnses+strlen(response->dnses), dnsV4[i], strlen(dnsV4[i]));
            }
            if (v6DnsLength > 0 && strlen(dnsV6[i]) > 0) {
                int currentLength = strlen(response->dnses);
                if (currentLength > 0) {
                    strncat(response->dnses+currentLength, " ", 1);
                }
                strncat(response->dnses+strlen(response->dnses), dnsV6[i], strlen(dnsV6[i]));
            }
        }
    }

    response->pcscf= calloc(1, pcscfLength * sizeof(char) + 1);
    if (response->pcscf == NULL) {
        LOGE("[%s] calloc for response->pcscf failed!", __FUNCTION__);
        return defaultCidIdx;
    }
    memset(response->pcscf, 0, pcscfLength * sizeof(char) + 1);
    for (i = 0; i < MAX_PCSCF_NUMBER; i++) {
        if (pcscfLength > 0 && strlen(pcscf[i]) > 0) {
            int currentLength = strlen(response->pcscf);
            if (currentLength > 0)
                strncat(response->pcscf+currentLength, " ", 1);

            strncat(response->pcscf+strlen(response->pcscf), pcscf[i], strlen(pcscf[i]));
        }
    }

    return defaultCidIdx;
}

void createDedicateDataResponse(int cid, MALRIL_Dedicate_Data_Call_Struct* response, RIL_SOCKET_ID rid)
{
    PdnInfo *pdnInfo = NULL;

    if (cid >= WIFI_CID_OFFSET) {
        pdnInfo = &pdn_info_wifi[rid][cid - WIFI_CID_OFFSET];
        response->primaryCid = cid;
        response->cid = cid;
        response->bearerId = 0;
        // IMSM will update these info to IMCB directly.
        response->hasQos = 0;
        response->hasTft = 0;
    } else {
        pdnInfo = &pdn_info[rid][cid];
        response->primaryCid = pdnInfo->primaryCid;
        response->cid = pdnInfo->cid;
        response->bearerId = pdnInfo->bearerId;
        response->hasQos = 1;
        response->hasTft = 1;
        memcpy(&response->qos, &pdnInfo->qos, sizeof(Qos));
        memcpy(&response->tft, &pdnInfo->tft, sizeof(Tft));
    }

    response->interfaceId = pdnInfo->interfaceId;
    response->active = pdnInfo->active;
    response->signalingFlag = pdnInfo->signalingFlag;


    int i = 0;
    for (i=0; i<MAX_PCSCF_NUMBER; i++) {
        if (strlen(pdnInfo->pcscf[i]) > 0) {
            response->hasPcscf = 1;
            if (i == 0)
                sprintf(response->pcscf + strlen(response->pcscf), "%s", pdnInfo->pcscf[i]);
            else
                sprintf(response->pcscf + strlen(response->pcscf), " %s", pdnInfo->pcscf[i]);
        }
    }
}

void makeDedicatedDataResponse(int concatenatedBearerNum, int activatedPdnNum,
        int *activatedCidList, mal_datamngr_data_call_info_req_t* response, RIL_SOCKET_ID rid)
{
    LOGD("[%s] concatenatedBearNum: %d", __FUNCTION__, concatenatedBearerNum);
    if (concatenatedBearerNum > 0) { //create concatenated dedicate bearer response
        int count = 0;
        int i = 0;
        MALRIL_Dedicate_Data_Call_Struct* concatenateResponses = alloca(concatenatedBearerNum * sizeof(MALRIL_Dedicate_Data_Call_Struct));
        initialDedicateDataCallResponse(concatenateResponses, concatenatedBearerNum);

        for (i = 0; i<activatedPdnNum; i++) {
            if (pdn_info[rid][activatedCidList[i]].isDedicateBearer) {
                createDedicateDataResponse(activatedCidList[i], &concatenateResponses[count], rid);
                dumpDedicateDataResponse(&concatenateResponses[count], "dump response (dedicate)");
                ++count;
            }
        }
    }
}

void freeDataResponse(MTK_RIL_Data_Call_Response_v11* response)
{
    FREEIF(response->type);
    FREEIF(response->ifname);
    FREEIF(response->addresses);
    FREEIF(response->gateways);
    FREEIF(response->dnses);
    FREEIF(response->pcscf);
}

void freeMalDataResponse(mal_datamngr_data_call_info_req_t* response)
{
    if (response->ifname != NULL) {
        LOGD("freeMalDataResponse: type=%s, ifname=%s, addresses=%s, dnses=%s, gateways=%s, pcscf=%s",
                response->type, response->ifname, response->addresses, response->dnses,
                response->gateways, response->pcscf);
    }

    FREEIF(response->type);
    FREEIF(response->ifname);
    FREEIF(response->addresses);
    FREEIF(response->gateways);
    FREEIF(response->dnses);
    FREEIF(response->pcscf);
}


void dumpDataResponse(MTK_RIL_Data_Call_Response_v11* response, const char* reason)
{
    LOGD("%s data call response: status=%d, suggestedRetryTime=%d, interfaceId=%d, active=%d, \
type=%s, ifname=%s, pcscf=%s",
            reason == NULL ? "dumpDataResponse" : reason,
            response->status, response->suggestedRetryTime,
            response->cid, response->active, response->type,
            response->ifname, response->pcscf);
}

/// M: Ims Data Framework
void dumpMalDataResponse(mal_datamngr_data_call_info_req_t* response, const char* reason)
{
    LOGD("%s data call response: status=%d, suggestedRetryTime=%d, interfaceId=%d, active=%d, mtu=%d, "
         "type=%s, ifname=%s, addresses=%s, dnses=%s, gateways=%s, pcscf=%s, eran_type=%d",
            reason == NULL ? "dumpDataResponse" : reason,
            response->status, response->suggestedRetryTime,
            response->cid, response->active,
            response->mtu, response->type,
            response->ifname, response->addresses,
            response->dnses, response->gateways,
            response->pcscf, response->eran_type);
    dumpDedicateDataResponse(&response->defaultBearer, reason);
}
/// @}

void dumpDedicateDataResponse(MALRIL_Dedicate_Data_Call_Struct* response, const char* reason)
{
    int i;
    char linkedPf[64] = {0};
    for (i=0; i<response->tft.tftParameter.linkedPfNumber; i++) {
        char str[5] = {0};
        snprintf(str, sizeof(str), "%d ", response->tft.tftParameter.linkedPfList[i]);
        strncat(linkedPf, str, 64 - strlen(linkedPf) - 1);
    }

    /*char authtok[64] = {0};
    for (i=0; i<response->tft.tftParameter.authtokenFlowIdNumber; i++) {
        char str[5] = {0};
        sprintf(str, "%d ", response->tft.tftParameter.authtokenFlowIdList[i]);
        strcat(authtok, str);
    }*/

    LOGD("%s dedicate data call response: ddcid=%d, interfaceId=%d, primaryCid=%d, cid=%d, active=%d, signalingFlag=%d, bearerId=%d, failCause=%d, pcscf=%s, \
QOS[qci=%d, dlGbr=%d, ulGbr=%d, dlMbr=%d, ulMbr=%d], PF number=%d, TftParameter[linked: %s]",
        reason == NULL ? "dumpDedicateDataResponse" : reason, response->ddcId,
        response->interfaceId, response->primaryCid, response->cid, response->active, response->signalingFlag, response->bearerId, response->failCause, response->pcscf,
        response->qos.qci, response->qos.dlGbr, response->qos.ulGbr, response->qos.dlMbr, response->qos.ulMbr,
        response->tft.pfNumber, linkedPf);

    if (response->tft.pfNumber > 0) {
        for (i=0; i<response->tft.pfNumber; i++) {
            LOGD("%s dedicate data call response CID%d-%d: TFT[id=%d, precedence=%d, bitmap=0x%08x, \
mask=%s, protocolNextHeader=%d, localPortLow=%d, localPortHigh=%d, remotePortLow=%d, remotePortHigh=%d, \
spi=%08x, tos=%d, tosMask=%d, flowLabel=%05x, direction=%d, networkPfIdentifier=%d]",
                reason == NULL ? "dumpDedicateDataResponse" : reason, response->cid, i,
                response->tft.pfList[i].id, response->tft.pfList[i].precedence, response->tft.pfList[i].bitmap,
                response->tft.pfList[i].mask, response->tft.pfList[i].protocolNextHeader, response->tft.pfList[i].localPortLow,
                response->tft.pfList[i].localPortHigh, response->tft.pfList[i].remotePortLow, response->tft.pfList[i].remotePortHigh,
                response->tft.pfList[i].spi, response->tft.pfList[i].tos, response->tft.pfList[i].tosMask,
                response->tft.pfList[i].flowLabel, response->tft.pfList[i].direction, response->tft.pfList[i].networkPfIdentifier);
        }
    }
}

int apnMatcher (const char* out, const char* apn) {
    //The APN is composed of two parts as follows: The APN Network Identifier & The APN Operator Identifier
    //The APN Operator Identifier is in the format of "mnc<MNC>.mcc<MCC>.gprs"
    //The valid APN format: <apn>[.mnc<MNC>.mcc<MCC>[.gprs]]
    int apnLength = strlen(apn);
    int j = 0;
    int result = -1;

    if ((int)strlen(out) < apnLength) return result;

    result = strcasecmp(out, apn);

    if (result == 0) {
        if ( !((strlen(apn) == strlen(out))
            || strStartsWith((out+apnLength),".mnc") || strStartsWith((out+apnLength),".MNC")
            || strStartsWith((out+apnLength),".mcc") || strStartsWith((out+apnLength),".MCC"))) {
            LOGV("apnMatcher: apns are equal but format unexpected");
            result = -1;
        }
    }

    if ((NULL != strcasestr(out, apn)) && strStartsWith((out+apnLength),".")) {
        result = 0;
        LOGV("apnMatcher: apn are equal as apn.xxx type");
    }

    return result;

}

int queryMatchedPdnWithSameApn(const char* apn, int* matchedCidList, RILChannelCtx* rilchnlctx)
{
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(rilchnlctx);
    char reuse[PROPERTY_VALUE_MAX] = {0};
    char reuseApnName[PROPERTY_VALUE_MAX] = {0};
    int replaceApnFlag = 0;
    char replaceApn[PROPERTY_VALUE_MAX] = {0};
    /**
     * [vendor.ril.pdn.reuse]
     * 0: bip decides to skip reusing IA apn
     * 1: default value, bip is not involved in this setup data call
     * 2: bip decides to reuse IA apn
     */
    property_get("vendor.ril.pdn.reuse", reuse, "1");
    if (strcmp("0", reuse) == 0) {
        LOGI("[%d]queryMatchedPdnWithSameApn not reuse is set", rid);
        return 0;
    } else if (strcmp("2", reuse) == 0) {
        property_get("vendor.ril.pdn.name.reuse", reuseApnName, "");
        LOGD("[%d]queryMatchedPdnWithSameApn: vendor.ril.pdn.name.reuse = %s", rid, reuseApnName);
    }

    int matchedCount = 0;
    ATResponse *p_response = NULL;
    ATLine *p_cur = NULL;
    int err = 0;
    char *out = NULL;

    if (updateDefaultBearerInfo(rilchnlctx) < 0) {
        LOGE("[%d]queryMatchedPdnWithSameApn updateDefaultBearerInfo failed [apn=%s]", rid, apn);
        return matchedCount;
    }

    err = at_send_command_multiline ("AT+CGCONTRDP", "+CGCONTRDP:", &p_response, rilchnlctx);
    if (isATCmdRspErr(err, p_response)) {
        LOGE("[%d]queryMatchedPdnWithSameApn AT+CGCONTRDP response error", rid);
    } else {
        for (p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next) {
            char *line = p_cur->line;
            int cid;
            int type;

            err = at_tok_start(&line);
            if (err < 0) {
                break;
            }
            err = at_tok_nextint(&line, &cid); // cid
            if (err < 0) {
                break;
            }
            err = at_tok_nextint(&line, &type); // type
            if (err < 0) {
                break;
            }
            err = at_tok_nextstr(&line, &out); // apn
            if (err < 0) {
                break;
            }
            int j = 0;
            for (j = 0; j < pdnInfoSize; j++) {
                if (j == 0 && cid == 0) {
                    char iaApn[PROPERTY_VALUE_MAX] = {0};
                    char iaTempApn[PROPERTY_VALUE_MAX] = {0};

                    int iaSimId = rid;

                    getMSimProperty(iaSimId, PROPERTY_IA_APN, iaApn);
                    getMSimProperty(iaSimId, PROPERTY_TEMP_IA_APN, iaTempApn);
                    LOGV("[%d]IA: store network assigned APN [%s]", rid, out);
                    property_set(PROPERTY_IA_FROM_NETWORK[iaSimId], out);
                    if ((0 != strlen(iaApn) && 0 != apnMatcher(out, iaApn)) ||
                        (0 != strlen(iaTempApn) && 0 != apnMatcher(out, iaTempApn))){
                        if (0 != strlen(iaApn)) {
                            LOGI("[%d]queryMatchedPdnWithSameApn attach APN is different to NW \
                                    assigned [%s, %s]", rid, iaApn, out);
                            setMSimProperty(iaSimId, PROPERTY_RE_IA_APN, iaApn);
                        } else {
                            LOGI("[%d]queryMatchedPdnWithSameApn attach APN is different to NW \
                                    assigned [%s, %s]", rid, iaTempApn, out);
                            setMSimProperty(iaSimId, PROPERTY_RE_IA_APN, iaTempApn);
                        }
                        setIaReplaceFlag(iaSimId, REPLACE_APN_FLAG);
                    } else {
                        setMSimProperty(iaSimId, PROPERTY_RE_IA_APN, "");
                        setIaReplaceFlag(iaSimId, 0);
                    }
                    replaceApnFlag = getIaReplaceFlag(iaSimId);
                    getIaReplaceApn(iaSimId, replaceApn);
                }
                if (j == cid && (pdn_info[rid][j].active == DATA_STATE_LINKDOWN ||
                                 pdn_info[rid][j].active == DATA_STATE_ACTIVE)) {
                    //only linkdown connection need to be queried, use strcmpi : case-insensitive

                    if (((out != NULL && strlen(out) > 0 && apnMatcher(out, apn) == 0) ||
                              (strcmp("2", reuse) == 0 && apnMatcher(reuseApnName, apn) == 0)) ||
                              ((REPLACE_APN_FLAG == replaceApnFlag) && (apnMatcher(replaceApn, apn) == 0)) ||
                              (0 == strcmp(apn, ""))) {
                        matchedCidList[matchedCount] = cid;
                        ++matchedCount;

                        if (strcmp("2", reuse) == 0) {
                            LOGD("[%d]queryMatchedPdnWithSameApn force reuse linkdown apn", rid);
                            property_set("vendor.ril.pdn.reuse","");
                        }

                        if (REPLACE_APN_FLAG == replaceApnFlag) {
                            LOGD("[%d]queryMatchedPdnWithSameApn to be replaced APN is [%s]", rid, replaceApn);
                        }

                        LOGD("[%d]queryMatchedPdnWithSameApn result [%d, %s, %s, %d]", rid, cid, apn, out, matchedCount);
                        break;
                    }
                }
            }
        }
    }
    AT_RSP_FREE(p_response);
    LOGD("[%d]queryMatchedPdnWithSameApn result [%d, %s]", rid, matchedCount, apn);
    return matchedCount;
}

int isCidTypeMatched(int cid, int protocol, RILChannelCtx* rilchnlctx)
{
    int result = 0;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(rilchnlctx);
    if (pdn_info[rid][cid].active != DATA_STATE_INACTIVE) {
        updatePdnAddressByCid(cid, rilchnlctx);
        switch (protocol) {
            case IPV4:
                if ((strlen(pdn_info[rid][cid].addressV4) > 0
                    && strcmp(pdn_info[rid][cid].addressV4, NULL_IPV4_ADDRESS) != 0)
                    && (strlen(pdn_info[rid][cid].addressV6) == 0
                    || strcmp(pdn_info[rid][cid].addressV6, NULL_IPV6_ADDRESS) == 0))
                    result = 1;
                break;
            case IPV6:
                if ((strlen(pdn_info[rid][cid].addressV6) > 0
                    && strcmp(pdn_info[rid][cid].addressV6, NULL_IPV6_ADDRESS) != 0)
                    && (strlen(pdn_info[rid][cid].addressV4) == 0
                    || strcmp(pdn_info[rid][cid].addressV4, NULL_IPV4_ADDRESS) == 0))
                    result = 1;
                break;
            case IPV4V6:
                if ((strlen(pdn_info[rid][cid].addressV4) > 0
                    && strcmp(pdn_info[rid][cid].addressV4, NULL_IPV4_ADDRESS) != 0)
                    || (strlen(pdn_info[rid][cid].addressV6) > 0
                    && strcmp(pdn_info[rid][cid].addressV6, NULL_IPV6_ADDRESS) != 0)) {
                    result = 1;
                }
                break;
        }
    }
    if (result == 0) {
        LOGD("[%d]isCidTypeMatched result=%d [cid=%d, active=%d, protocol=%d]",
                rid, result, cid, pdn_info[rid][cid].active, protocol);
    }
    return result;
}

void convertIPv6AddressToDotFormat(char* address, int maxLength)
{
    int ret = 1;
    struct in6_addr v6Address;
    memset(&v6Address, 0, sizeof(v6Address));
    // ret: -1, error occurs, ret: 0, invalid address, ret: 1, success;
    ret = inet_pton(AF_INET6, address, &v6Address);
    LOGD("inet_pton ret: %d", ret);
    memset(address, 0, sizeof(char) * maxLength);
    if (ret >= 0) {
        sprintf(address, "%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d",
            v6Address.s6_addr[0], v6Address.s6_addr[1], v6Address.s6_addr[2], v6Address.s6_addr[3],
            v6Address.s6_addr[4], v6Address.s6_addr[5], v6Address.s6_addr[6], v6Address.s6_addr[7],
            v6Address.s6_addr[8], v6Address.s6_addr[9], v6Address.s6_addr[10], v6Address.s6_addr[11],
            v6Address.s6_addr[12], v6Address.s6_addr[13], v6Address.s6_addr[14], v6Address.s6_addr[15]);
    }
}

/// M: Ims Data Framework
char* getConfigTftATCmd(int cid, PktFilter* pktFilter)
{
    int cmdSize = 256*sizeof(char);
    char* cmd = (char*)calloc(1, cmdSize);
    assert(cmd != NULL);

    //R8
    //+CGTFT=<cid>[,<pktFilterId>,<evalPrecIdx>[,<addr_n_mask>[,<protocolNum> [,<dstPortRange>[,<srcPortRange>[,<spi>[,<tos_n_mask>[, <flow_label>]]]]]]]]

    //27.007
    //+CGTFT=[<cid>,[<packet filter identifier>,<evaluation precedence index>[,<remote address and subnet mask>
    //[,<protocol number (ipv4) / next header (ipv6)>[,<local port range>[,<remote port range>
    //[,<ipsec security parameter index (spi)>[,<type of service (tos) (ipv4) and mask / traffic class (ipv6) and mask>
    //[,<flow label (ipv6)>[,<direction>]]]]]]]]]]

    LOGD("getConfigTftATCmd bitmap [0x%08x]", pktFilter->bitmap);

    snprintf(cmd, cmdSize - 1, "AT+CGTFT=%d,%d,%d", cid, pktFilter->id, pktFilter->precedence);

    if ((pktFilter->bitmap & PKT_BITMASK_IMC_BMP_V4_ADDR) || (pktFilter->bitmap & PKT_BITMASK_IMC_BMP_V6_ADDR)) {
        if (pktFilter->bitmap & PKT_BITMASK_IMC_BMP_V6_ADDR) {
            //convert IPv6 address from xxxx:xxxx::xxxx format to xxx.xxx.xxx.xxx.xxx.xxx.xxx...... format)
            convertIPv6AddressToDotFormat(pktFilter->address, MAX_IPV6_ADDRESS_LENGTH);
            convertIPv6AddressToDotFormat(pktFilter->mask, MAX_IPV6_ADDRESS_LENGTH);
        }
        snprintf(cmd+strlen(cmd), cmdSize - strlen(cmd) - 1,",\"%s.%s\"", strcmp(pktFilter->address, "") ? pktFilter->address : NULL_IPV4_ADDRESS,
            strcmp(pktFilter->mask, "") ? pktFilter->mask : NULL_IPV4_ADDRESS);
    } else {
        strncpy(cmd+strlen(cmd), ",\"\"", cmdSize - strlen(cmd) - 1);
    }

    if (pktFilter->bitmap & PKT_BITMASK_IMC_BMP_PROTOCOL)
        snprintf(cmd+strlen(cmd), cmdSize - strlen(cmd) - 1, ",%d", pktFilter->protocolNextHeader);
    else
        strncpy(cmd+strlen(cmd), ",", cmdSize - strlen(cmd) - 1);

    if (max_pdn_support_4G > 0) {
        if ((pktFilter->bitmap & PKT_BITMASK_IMC_BMP_LOCAL_PORT_SINGLE) ||
            (pktFilter->bitmap & PKT_BITMASK_IMC_BMP_LOCAL_PORT_RANGE)) {
            snprintf(cmd+strlen(cmd), cmdSize - strlen(cmd) - 1, ",\"%d.%d\"", pktFilter->localPortLow, pktFilter->localPortHigh);
        } else {
            strncpy(cmd+strlen(cmd), ",\"\"", cmdSize - strlen(cmd) - 1);
        }
    }

    if ((pktFilter->bitmap & PKT_BITMASK_IMC_BMP_REMOTE_PORT_SINGLE) ||
        (pktFilter->bitmap & PKT_BITMASK_IMC_BMP_REMOTE_PORT_RANGE)) {
        snprintf(cmd+strlen(cmd), cmdSize - strlen(cmd) - 1, ",\"%d.%d\"", pktFilter->remotePortLow, pktFilter->remotePortHigh);
    } else {
        strncpy(cmd+strlen(cmd), ",\"\"", cmdSize - strlen(cmd) - 1);
    }


    if (pktFilter->bitmap & PKT_BITMASK_IMC_BMP_SPI)
        snprintf(cmd+strlen(cmd), cmdSize - strlen(cmd) - 1, ",%08x", pktFilter->spi);
    else
        strncpy(cmd+strlen(cmd), ",", cmdSize - strlen(cmd) - 1);

    if (pktFilter->bitmap & PKT_BITMASK_IMC_BMP_TOS)
        snprintf(cmd+strlen(cmd), cmdSize - strlen(cmd) - 1, ",\"%d.%d\"", pktFilter->tos, pktFilter->tosMask);
    else
        strncpy(cmd+strlen(cmd), ",\"\"", cmdSize - strlen(cmd) - 1);

    if (pktFilter->bitmap & PKT_BITMASK_IMC_BMP_FLOW_LABEL)
        snprintf(cmd+strlen(cmd), cmdSize - strlen(cmd) - 1, ",%05x", pktFilter->flowLabel);
    else
        strncpy(cmd+strlen(cmd), ",", cmdSize - strlen(cmd) - 1);

    if (max_pdn_support_4G > 0)
        snprintf(cmd+strlen(cmd), cmdSize - strlen(cmd) - 1, ",%d", pktFilter->direction);

    return cmd;
}

int clearQosTftContext(int cid, RILChannelCtx* rilchnlctx)
{
    ATResponse *p_response = NULL;
    char cmd[PROPERTY_VALUE_MAX] = {0};
    int err = 0, failCause = 0;

    //before define QOS & TFT, clear previous configuration
    LOGD("clearQosTftContext to clear tft context of CID%d", cid);

    memset(cmd, 0, sizeof(cmd));
    if (max_pdn_support_4G > 0) {
        snprintf(cmd, sizeof(cmd), "AT+CGEQOS=%d", cid);
    } else {
        snprintf(cmd, sizeof(cmd), "AT+CGEQREQ=%d", cid);
    }

    failCause = at_send_command(cmd, &p_response, rilchnlctx);
    if (isATCmdRspErr(err, p_response)) {
        failCause = ERROR_CAUSE_GENERIC_FAIL; //since this command is to clear qos locally, use generic fail cause
        LOGD("clearQosTftContext AT+CGEQOS for clear err=%d", failCause);
    }
    AT_RSP_FREE(p_response);

    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "AT+CGTFT=%d", cid);
    failCause = at_send_command(cmd, &p_response, rilchnlctx);
    if (isATCmdRspErr(err, p_response)) {
        failCause = ERROR_CAUSE_GENERIC_FAIL; //since this command is to clear tft locally, use generic fail cause
        LOGD("clearQosTftContext AT+CGTFT for clear err=%d", failCause);
    }
    AT_RSP_FREE(p_response);

    //+EGDELPF=<cid> causes all of the packet filters to be deleted in the user defined TFT for context number <cid> to become undefined
    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "AT+EGDELPF=%d", cid);
    failCause = at_send_command(cmd, &p_response, rilchnlctx);
    if (isATCmdRspErr(err, p_response)) {
        failCause = ERROR_CAUSE_GENERIC_FAIL; //since this command is to clear tft locally, use generic fail cause
        LOGD("clearQosTftContext AT+EGDELPF for clear err=%d", failCause);
    }
    AT_RSP_FREE(p_response);

    //+EGLNKPF=[<cid>] causes all of the linked packet filters in the user defined TFT for context number <cid> to become undefined
    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "AT+EGLNKPF=%d", cid);
    failCause = at_send_command(cmd, &p_response, rilchnlctx);
    if (isATCmdRspErr(err, p_response)) {
        failCause = ERROR_CAUSE_GENERIC_FAIL; //since this command is to clear tft locally, use generic fail cause
        LOGD("clearQosTftContext AT+EGLNKPF for clear err=%d", failCause);
    }
    AT_RSP_FREE(p_response);

    return failCause;
}

// Query all activated dedicated PDN list for a default bearer.
int getDedicatedPdnList(const int def_bearer_cid,
                            const RIL_SOCKET_ID rid, int *pDedicatedPdnList) {
    int count = 0;
    int i = 0;

    if (pDedicatedPdnList == NULL) {
        return 0;
    }

    for (i = 0; i < pdnInfoSize; i++) {
        if (pdn_info[rid][i].isDedicateBearer &&
                pdn_info[rid][i].primaryCid == def_bearer_cid &&
                pdn_info[rid][i].cid != INVALID_CID &&
                pdn_info[rid][i].active == DATA_STATE_ACTIVE) {
            pDedicatedPdnList[count] = pdn_info[rid][i].cid;
            count++;
        }
    }
    return count;
}

void notifyLinkdownDedicatedBearer(const int def_bearer_cid, const RIL_SOCKET_ID rid) {
    int pdn_size = pdnInfoSize * sizeof(int);
    int *pDedicatedPdnList = (int *)malloc(pdn_size);
    if (pDedicatedPdnList == NULL) {
        LOGD("[%d]%s() malloc failed!!", rid, __FUNCTION__);
        return;
    }
    memset(pDedicatedPdnList, INVALID_CID, pdn_size);
    int count = 0;
    int i = 0;

    count = getDedicatedPdnList(def_bearer_cid, rid, pDedicatedPdnList);

    LOGD("[%d]%s() def_bearer_cid: %d, count: %d",
            rid, __FUNCTION__, def_bearer_cid, count);

    for (i = 0; i < count; i++) {
        int cid = pDedicatedPdnList[i];
        MALRIL_Dedicate_Data_Call_Struct* pResponse = alloca(sizeof(MALRIL_Dedicate_Data_Call_Struct));
        assert(pResponse != NULL);
        initialDedicateDataCallResponse(pResponse, 1);
        createDedicateDataResponse(cid, pResponse, rid);
        dumpDedicateDataResponse(pResponse, "dump response (dedicate)");
        RIL_UNSOL_RESPONSE(RIL_LOCAL_GSM_UNSOL_DEDICATE_BEARER_ACTIVATED, pResponse,
                sizeof(MALRIL_Dedicate_Data_Call_Struct), rid);
    }
    free(pDedicatedPdnList);
}


void initialDedicateDataCallResponse(MALRIL_Dedicate_Data_Call_Struct* responses, int length)
{
    int i = 0;
    for (i = 0; i<length; i++) {
        memset(&responses[i], 0, sizeof(MALRIL_Dedicate_Data_Call_Struct));
        responses[i].ddcId = INVALID_CID;
        responses[i].interfaceId = INVALID_CID;
        responses[i].primaryCid = INVALID_CID;
        responses[i].cid = INVALID_CID;
    }
}
/// @}

int get_address_type(char* addr) {
    int type = IPV4;
    int length = strlen(addr);
    if (length >= MAX_IPV6_ADDRESS_LENGTH)
        type = IPV4V6;
    if (strlen(addr) >= MAX_IPV4_ADDRESS_LENGTH)
        type = IPV6;

    return type;
}

int convert_ipv6_address(char* pdp_addr, char* ipv6_addr, int isLinkLocal) {
    char *p = NULL;
    int value = 0;
    int len = 0;
    int count = 8;
    char tmpOutput[MAX_IPV6_ADDRESS_LENGTH] = {0};

    memset(ipv6_addr, 0, MAX_IPV6_ADDRESS_LENGTH);
    if (isLinkLocal == 1) {
        strncpy(ipv6_addr, IPV6_PREFIX, strlen(IPV6_PREFIX));
    } else {
        strncpy(ipv6_addr, NULL_ADDR, strlen(NULL_ADDR));
        ipv6_addr[strlen(NULL_ADDR)] = 0;
    }

    p = strtok(pdp_addr, ".");

    //32.1.13.184.0.0.0.3.61.48.97.182.50.254.113.251
    //2001:0DB8:0000:0003:3D30:61B6:32FE:71FB

    //Skip the 64 bit of this PDP address if this pdp adress is a local link address
    if (isLinkLocal == 1) {
        while (count) {
            p = strtok(NULL, ".");
            count--;
        }
    }

    while (p) {
        memset(tmpOutput, 0, MAX_IPV6_ADDRESS_LENGTH);
        value = atoi(p);
        snprintf(tmpOutput, MAX_IPV6_ADDRESS_LENGTH, "%02X", value);
        strncat(ipv6_addr + strlen(ipv6_addr), tmpOutput, MAX_IPV6_ADDRESS_LENGTH-strlen(ipv6_addr)-1);
        p = strtok(NULL, ".");
        if (p == NULL) {
            LOGE("The format of IP address is illegal");
            return -1;
        }
        memset(tmpOutput, 0, MAX_IPV6_ADDRESS_LENGTH);
        value = atoi(p);
        snprintf(tmpOutput, MAX_IPV6_ADDRESS_LENGTH, "%02X:", value);
        strncat(ipv6_addr + strlen(ipv6_addr), tmpOutput, MAX_IPV6_ADDRESS_LENGTH-strlen(ipv6_addr)-1);
        p = strtok(NULL, ".");
    }

    len = strlen(ipv6_addr);
    ipv6_addr[len-1] = '\0';

    return 0;
}

int get_protocol_type(const char* protocol) {
    int type = IPV4;

    if (protocol == NULL)
        return type;

    if (!strcmp(protocol, SETUP_DATA_PROTOCOL_IP))
        type = IPV4;
    else if (!strcmp(protocol, SETUP_DATA_PROTOCOL_IPV6))
        type = IPV6;
    else if (!strcmp(protocol, SETUP_DATA_PROTOCOL_IPV4V6))
        type = IPV4V6;

    LOGD("The protocol type is %d", type);
    return type;
}

void ril_data_set_ipv6addr(int s, struct ifreq *ifr, const char *addr)
{
    struct in6_ifreq ifreq6;
    int ret = 0;

    ret = ioctl(s,SIOCGIFINDEX, ifr);
    if (ret < 0)
       goto terminate;

    inet_pton(AF_INET6, addr, &ifreq6.ifr6_addr);
    ifreq6.ifr6_prefixlen = 64;
    ifreq6.ifr6_ifindex = ifr->ifr_ifindex;
    ret = ioctl(s,SIOCSIFADDR, &ifreq6);
    if (ret < 0)
       goto terminate;

    return;
terminate:
    LOGE("error in set IP address v6:%d - %d:%s", ret, errno, strerror(errno));
    return;
}

/// M: Ims Data Framework {@
void requestSetupDataCallEmergency(const reqSetupConf_t *pReqSetupConfig, const void *ptrDefaultBearerVaConfig,
    RIL_Token t)
{
    char *cmd = NULL;
    int err = 0;
    ATResponse *p_response = NULL;
    char *line = NULL, *out = NULL;
    ATLine *p_cur = NULL;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));

    RILChannelCtx* pRilchnlctx = getChannelCtxbyProxy();

    INIT_REQ_SETUP_CNF(pReqSetupConfig);

    int i = 0;
    const RIL_Default_Bearer_VA_Config_Struct *pDefaultBearerVaConfig = (const RIL_Default_Bearer_VA_Config_Struct *) ptrDefaultBearerVaConfig;

    MTK_RIL_Data_Call_Response_v11 *response = calloc(1, sizeof(MTK_RIL_Data_Call_Response_v11));
    mal_datamngr_data_call_info_req_t* responseToMal = (mal_datamngr_data_call_info_req_t*)
            calloc(1, sizeof(mal_datamngr_data_call_info_req_t));

    assert(response != NULL);
    assert(responseToMal != NULL);

    initialDataCallResponse(response, 1);
    initialMalDataCallResponse(responseToMal, 1);

    int activatedPdnNum = 0;
    int concatenatedBearerNum = 0;
    int *activatedCidList = alloca(pdnInfoSize * sizeof(int));
    assert(activatedCidList != NULL);
    int isEmergency = 1, currUsedEmergencyCid = INVALID_CID;
    int isNoDataActive = 0;
    epdgConfig_t epdgConfig = {0, 0};

    configEpdg(&epdgConfig, pDefaultBearerVaConfig);

    //Disable MD fallback for AP fallback
    //AT+EGFB=<fallback_enable>,<ipv4_first>
    err = at_send_command("AT+EGFB=0,1", &p_response, DATA_CHANNEL_CTX);
    AT_RSP_FREE(p_response);

    LOGD("[%d][%s] E", rid, __FUNCTION__);
    for (i = 0; i < pdnInfoSize; i++) {
        activatedCidList[i] = INVALID_CID;
    }

    // check in if there a emergency pdn is used
    for (i = 0; i < pdnInfoSize; i++) {
        if (pdn_info[rid][i].active == DATA_STATE_ACTIVE &&
                pdn_info[rid][i].isEmergency == isEmergency) {
            activatedCidList[activatedPdnNum] = i;
            activatedPdnNum++;
            currUsedEmergencyCid = i;
            break;
        }
    }

    if(activatedPdnNum) {
        LOGD("Already has emergency PDN, [cid: %d]", currUsedEmergencyCid);
    } else {
        int availableCid = getAvailableCid(rid);
        if (availableCid == INVALID_CID) {
            LOGE("[%s] no available CID to use", __FUNCTION__);
            goto error;
        } else {
            LOGD("[%s] available CID is [%d]", __FUNCTION__, availableCid);
        }

        //ALPS02972991 enforce change emc protocol to IPv6 to compliance TMOUS SPEC.
        int emcProtocol = protocol;
        if (isOp08Support() && (isSimInserted(rid) == 0)) {
            emcProtocol = IPV6;
        }

        if (0 == definePdnCtx(pDefaultBearerVaConfig, requestedApn, emcProtocol, availableCid, authType, username, password, pRilchnlctx))
            goto error;

        // Activate PDP
        if (PDN_FAILED >= activatePdn(availableCid, &activatedPdnNum, &concatenatedBearerNum,
                activatedCidList, isEmergency, isNoDataActive, pRilchnlctx))
            goto error;
    }

    for (i = 0; i < activatedPdnNum; i++) {
        int cid = activatedCidList[i];
        if (pdn_info[rid][cid].isDedicateBearer) {
            LOGD("[%s] skip update PDP address of CID%d since it is a dedicate bearer [active=%d]",
                    __FUNCTION__, cid, pdn_info[rid][cid].active);
        } else {
            if (updatePdnAddressByCid(cid, pRilchnlctx) < 0) {
                LOGD("[%s] update PDP address of CID%d failed", __FUNCTION__, cid);
                //we do not goto error since we expect to update all activated PDP address here
            }
            if (0 == bindPdnToIntf(activatedPdnNum, interfaceId, cid, pRilchnlctx)) {
                goto error;
            }
        }
    }

    updateDynamicParameter(pRilchnlctx);

    if (updateDns(pRilchnlctx) < 0) {
        LOGE("[%s] updateDns failed", __FUNCTION__);
        goto error;
    }

    if (DATA_STATE_INACTIVE == isCidActive(activatedCidList, activatedPdnNum, pRilchnlctx)) {
        goto error;
    }

    configureNetworkInterface(interfaceId, ENABLE_CCMNI, rid);

    int cid = createDataResponse(interfaceId, protocol, response, rid);
    createMalDataResponse(interfaceId, cid, response, responseToMal, rid);
    makeDedicatedDataResponse(concatenatedBearerNum, activatedPdnNum, activatedCidList, responseToMal, rid);
    setRspEranType(epdgConfig.eranType, response, responseToMal, rid);
    dumpDataResponse(response, "requestSetupDataCallImsEmergency dump response");
    dumpMalDataResponse(responseToMal, "requestSetupDataCallImsEmergency dump malResponse");

    responseUnsolDataCallRspToMal(t, responseToMal , sizeof(mal_datamngr_data_call_info_req_t));
    RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(MTK_RIL_Data_Call_Response_v11));

    goto finish;

error:
    for (i = 0; i < activatedPdnNum; i++)
        clearPdnInfo(&pdn_info[rid][activatedCidList[i]]);

    responseUnsolDataCallRspToMal(t, NULL , 0);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);

    AT_RSP_FREE(p_response);

    LOGE("[%s] response ERROR X", __FUNCTION__);

finish:
    if (response != NULL) {
        freeDataResponse(response);
        free(response);
    }

    if (responseToMal != NULL) {
        freeMalDataResponse(responseToMal);
        free(responseToMal);
    }

    UNUSED(profile);
    LOGD("[%s] response OK X", __FUNCTION__);
    return;
}
///@}

void requestSetupDataCallOverIPv6(const reqSetupConf_t *pReqSetupConfig, const void* ptrDefaultBearerVaConfig,
    RIL_Token t)
{
    char *cmd = NULL;
    int err = 0;
    ATResponse *p_response = NULL;
    char *line = NULL, *out = NULL;
    ATLine *p_cur = NULL;
    int isEmergency = 0;
    int i = 0;
    int isNoDataActive = 0;
    epdgConfig_t epdgConfig = {0, 0};
    int unavailableCid = INVALID_CID;
    bool isPdpFailCauseUpdateNeeded = false;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    int suggestedRetryTime = NO_SUGGESTED_TIME;
    int linkdownCid = INVALID_CID;

    INIT_REQ_SETUP_CNF(pReqSetupConfig);
    const RIL_Default_Bearer_VA_Config_Struct *pDefaultBearerVaConfig =
            (const RIL_Default_Bearer_VA_Config_Struct *) ptrDefaultBearerVaConfig;
    RIL_Default_Bearer_VA_Config_Struct *tmpDefaultBearerVaConfig = NULL;
    MTK_RIL_Data_Call_Response_v11* response = (MTK_RIL_Data_Call_Response_v11*)
            calloc(1, sizeof(MTK_RIL_Data_Call_Response_v11));
    mal_datamngr_data_call_info_req_t* responseToMal = (mal_datamngr_data_call_info_req_t*)
            calloc(1, sizeof(mal_datamngr_data_call_info_req_t));

    LOGV("[%d][RILData_GSM] requestSetupDataCallOverIPv6:requestedApn = %s", rid, requestedApn);
    int activatedPdnNum = 0;
    int concatenatedBearerNum = 0;
    int *activatedCidList = (int *) calloc(1, pdnInfoSize * sizeof(int));

    if (response == NULL || responseToMal == NULL || activatedCidList == NULL) {
        LOGE("[%s] calloc for response/responseToMal/activatedCidList failed!", __FUNCTION__);
        return;
    }

    initialDataCallResponse(response, 1);
    initialMalDataCallResponse(responseToMal, 1);

    for (i = 0; i < pdnInfoSize; i++) {
        activatedCidList[i] = INVALID_CID;
    }

    /// M: Ims Data Framework {@
    queryRatRsp_t* pQueryRatRsp = (queryRatRsp_t*) (pReqSetupConfig->pQueryRatRsp);
    if (pQueryRatRsp != NULL) {
        isNoDataActive = pQueryRatRsp->resp.islteonly == 1 ? 1 : 0;
    }
    configEpdg(&epdgConfig, pDefaultBearerVaConfig);
    /// @}

    // If under detached, ignore CGACT/EGACT, which may bring PS on again by MD.
    int slotId = getMappingSIMByCurrentMode(rid);
    if (!isMultiPsAttachSupport() && !isDataAllow(slotId)) {
        LOGD("[%d][RILData_GSM][%s] isDataAllow() is false.", rid, __FUNCTION__);
        response->status = gprs_failure_cause = PDP_FAIL_DATA_NOT_ALLOW;
        goto error;
    }

    if ((((NULL != requestedApn) && (NULL != strstr(requestedApn, "ims")))
            || (pdnReasonSupportVer < MD_LR11) || isMdFallbackSupport())
            && !isFallbackNotSupportByOp()) {
        if (isCC33Support() == 0 && !isLteDataOnCdma(rid)) {
            // M: only set IPv4v6 fallback for non-IRAT support projects
            // AT+EGFB=<fallback_enable>,<ipv4_first>
            err = at_send_command("AT+EGFB=1,1", &p_response, DATA_CHANNEL_CTX);
            AT_RSP_FREE(p_response);
        } else {
            // M: Data Framework - CC 33
            // Disable MD fallback due to CC33 or LTE on CDMA
            err = at_send_command("AT+EGFB=0,1", &p_response, DATA_CHANNEL_CTX);
            AT_RSP_FREE(p_response);
        }
    } else {
        //Disable MD fallback for AP fallback
        err = at_send_command("AT+EGFB=0,1", &p_response, DATA_CHANNEL_CTX);
        AT_RSP_FREE(p_response);
    }

    //Before activating PDN, query if there is any existed PDN with the same APN
    //It could be re-used if the APN is the same
    int queryResult = queryMatchedPdnWithSameApn(requestedApn, activatedCidList, DATA_CHANNEL_CTX);
    if (queryResult > 0 && isCidTypeMatched(activatedCidList[0], protocol, DATA_CHANNEL_CTX)) {
        LOGI("[%d][RILData_GSM] requestSetupDataCallOverIPv6 matched PDN is found [%d]", rid, queryResult);
        activatedPdnNum = queryResult;
        if (pdn_info[rid][activatedCidList[0]].active == DATA_STATE_LINKDOWN) {
            linkdownCid = activatedCidList[0];
        }
    } else {
        // get available Cid
        int availableCid = getAvailableCid(rid);

        // Multi-PS attach Start
        int iaSimId = rid;
        // Multi-PS attach End

        char iaApn[PROPERTY_VALUE_MAX] = {0};
        getMSimProperty(iaSimId, PROPERTY_IA_APN, iaApn);
        if (strcasecmp(requestedApn, iaApn) != 0 &&
                RAT_2_3G != querryCurrentRat(DATA_CHANNEL_CTX) &&
                isMdFallbackSupport()) {
            availableCid = getAvailableCidBasedOnSpecificCid(2, rid);
        }

        if (availableCid == INVALID_CID) {
            LOGE("[%d][RILData_GSM] requestSetupDataCallOverIPv6 no available CID to use", rid);
            response->status = gprs_failure_cause = PDP_FAIL_MAX_ACTIVE_PDP_CONTEXT_REACHED;
            goto error;
        } else {
            LOGI("[%d][RILData_GSM] requestSetupDataCallOverIPv6 available CID is [%d]", rid, availableCid);
        }
        // define Pdn ctx
        char overrideApn[PROPERTY_VALUE_MAX] = {0};
        property_get("vendor.ril.pdn.overrideApn", overrideApn, "");
        if (strlen(overrideApn) > 0) {
            if (0 == definePdnCtx(pDefaultBearerVaConfig, overrideApn, protocol, availableCid,
                    authType, username, password, DATA_CHANNEL_CTX)) {
                isPdpFailCauseUpdateNeeded = true;
                goto error;
            }
            property_set("vendor.ril.pdn.overrideApn", "");
        } else {
            if (0 == definePdnCtx(pDefaultBearerVaConfig, requestedApn, protocol, availableCid,
                    authType, username, password, DATA_CHANNEL_CTX)) {
                isPdpFailCauseUpdateNeeded = true;
                goto error;
            }
        }

        // packet-domain event reporting: +CGEREP=<mode>,<bfr>
        err = at_send_command("AT+CGEREP=1,0", NULL, DATA_CHANNEL_CTX);

        // activate PDN
        if (PDN_FAILED >= activatePdn(availableCid, &activatedPdnNum, &concatenatedBearerNum,
                             activatedCidList, isEmergency, isNoDataActive, DATA_CHANNEL_CTX)) {
            isPdpFailCauseUpdateNeeded = true;
            suggestedRetryTime = getModemSuggestedRetryTime(availableCid, DATA_CHANNEL_CTX);
            goto error;
        }
    }

    for (i = 0; i < activatedPdnNum; i++) {
        int cid = activatedCidList[i];
        if (pdn_info[rid][cid].isDedicateBearer) {
            LOGD("[%d][%s] skip update PDP address of CID%d since it is a dedicate bearer [active=%d]",
                    rid, __FUNCTION__, cid, pdn_info[rid][cid].active);
        } else {
            if (updatePdnAddressByCid(cid, DATA_CHANNEL_CTX) < 0) {
                LOGD("[%d][%s] update PDP address of CID%d failed", rid, __FUNCTION__, cid);
                // we do not goto error since we expect to update all activated PDP address here
            }
            if (0 == bindPdnToIntf(activatedPdnNum, interfaceId, cid, DATA_CHANNEL_CTX)) {
                //deact PDP of unavailableCid if binding data failed except initial attach PDP
                if (0 != cid) {
                    unavailableCid = cid;
                }
                response->status = gprs_failure_cause = PDP_FAIL_ERROR_UNSPECIFIED;
                goto error;
            }
        }
    }

    /// M: Ims Data Framework {@
    updateDynamicParameter(DATA_CHANNEL_CTX);
    /// @}

    if (updateDns(DATA_CHANNEL_CTX) < 0) {
        LOGE("[%d][%s] updateDns failed", rid, __FUNCTION__);
        response->status = gprs_failure_cause = PDP_FAIL_ERROR_UNSPECIFIED;
        goto error;
    }

    if (DATA_STATE_INACTIVE == isCidActive(activatedCidList, activatedPdnNum, DATA_CHANNEL_CTX)) {
        response->status = gprs_failure_cause = PDP_FAIL_LOST_CONNECTION;
        goto error;
    }

    configureNetworkInterface(interfaceId, ENABLE_CCMNI, rid);
    int cid = createDataResponse(interfaceId, protocol, response, rid);
    dumpDataResponse(response, "requestSetupDataCallOverIPv6 dump response");

    createMalDataResponse(interfaceId, cid, response, responseToMal, rid);
    makeDedicatedDataResponse(concatenatedBearerNum, activatedPdnNum, activatedCidList, responseToMal, rid);

    setRspEranType(epdgConfig.eranType, response, responseToMal, rid);
    dumpMalDataResponse(responseToMal, "requestSetupDataCallOverIPv6 dump mal response");
    responseUnsolDataCallRspToMal(t, responseToMal, sizeof(mal_datamngr_data_call_info_req_t));

    RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(MTK_RIL_Data_Call_Response_v11));

    if (isOp12Support()) {
        getAndSendPcoStatus(rid, activatedCidList[0], NULL);
    }
    // The dedicated bearer info will be updated in updateDynamicParameter
    if (linkdownCid >= 0) {
        notifyLinkdownDedicatedBearer(linkdownCid,rid);
    }
    ///@ }

    goto finish;

error:
    if (unavailableCid != INVALID_CID) {
        deactivateDataCall(unavailableCid, DATA_CHANNEL_CTX);
    }
    /// M: Ims Data Framework {@
    if (isEpdgSupport()) {
        response->status = PDP_FAIL_ACTIVATION_REJECT_UNSPECIFIED;
        response->cid = interfaceId;

        responseToMal->status = PDP_FAIL_ACTIVATION_REJECT_UNSPECIFIED;
        responseToMal->cid = interfaceId;

        if (isPdpFailCauseUpdateNeeded) {
            if (0 != gprs_failure_cause) {
                response->status = gprs_failure_cause;
                responseToMal->status = gprs_failure_cause;
            }
        }
        storeModemSuggestedRetryTime(suggestedRetryTime, response);
        responseToMal->suggestedRetryTime =
            (suggestedRetryTime > 0) ? (suggestedRetryTime * 1000) : suggestedRetryTime;
        responseUnsolDataCallRspToMal(t, responseToMal, sizeof(mal_datamngr_data_call_info_req_t));
        // Try another RAT
        if (pReqSetupConfig->retryCount < PDN_SETUP_THRU_MAX) {
            reqSetupConf_t *pTempReqSetupConfig = (reqSetupConf_t *)pReqSetupConfig;
            tmpDefaultBearerVaConfig = (RIL_Default_Bearer_VA_Config_Struct*)
                    calloc(1, sizeof(RIL_Default_Bearer_VA_Config_Struct));
            if (tmpDefaultBearerVaConfig == NULL) {
                LOGE("[%s] calloc for tmpDefaultBearerVaConfig failed!", __FUNCTION__);
                goto finish;
            }
            memcpy(tmpDefaultBearerVaConfig,
                   pDefaultBearerVaConfig,
                   sizeof(RIL_Default_Bearer_VA_Config_Struct));
            pTempReqSetupConfig->retryCount++;
            int nRet = queryEpdgRat(
                            getRILIdByChannelCtx(getRILChannelCtxFromToken(t)),
                            pTempReqSetupConfig,
                            tmpDefaultBearerVaConfig);
            switch (nRet) {
                case PDN_SETUP_THRU_WIFI:
                    pTempReqSetupConfig->availableCid = getAvailableWifiCid(rid);
                    if (pTempReqSetupConfig->availableCid != INVALID_CID) {
                        requestSetupDataCallOverEpdg(pTempReqSetupConfig, tmpDefaultBearerVaConfig, t);
                        return;
                    }
                    break;
                default:
                    LOGE("PDN retry fail");
            }
        }
        querySuggestionTimeFromRds(pReqSetupConfig, response, t);
    /// @}
    } else {
        responseUnsolDataCallRspToMal(t, NULL, 0);
        if (isPdpFailCauseUpdateNeeded) {
            if (0 != gprs_failure_cause) {
                response->status = gprs_failure_cause;
            }
        }
        storeModemSuggestedRetryTime(suggestedRetryTime, response);
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(MTK_RIL_Data_Call_Response_v11));

    for (i = 0; i < activatedPdnNum; i++) {
        if (pdn_info[rid][activatedCidList[i]].active == DATA_STATE_INACTIVE) {
            clearPdnInfo(&pdn_info[rid][activatedCidList[i]]);
        }
    }

finish:
    if (response != NULL) {
        freeDataResponse(response);
        free(response);
    }

    if (activatedCidList != NULL) {
        free(activatedCidList);
    }

    if (responseToMal!= NULL) {
        freeMalDataResponse(responseToMal);
        free(responseToMal);
    }

    if (tmpDefaultBearerVaConfig != NULL) {
        free(tmpDefaultBearerVaConfig);
    }
    LOGV("[%d][RILData_GSM] requestSetupDataCallOverIPv6 finish", rid);
    return;
}

void requestSetupDataCallFallback(const reqSetupConf_t *pReqSetupConfig, const void * ptrDefaultBearerVaConfig,
    RIL_Token t)
{
    char *cmd = NULL;
    int err = 0;
    ATResponse *p_response = NULL;
    char *line = NULL, *out = NULL;
    ATLine *p_cur = NULL;
    int isEmergency = 0;
    int i = 0;
    int isNoDataActive = 0;
    int isFallbackNeeded = 1;
    bool isPdpFailCauseUpdateNeeded = false;
    bool isFallbackRetrying = false;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    INIT_REQ_SETUP_CNF(pReqSetupConfig);
    int suggestedRetryTime = NO_SUGGESTED_TIME;

    const RIL_Default_Bearer_VA_Config_Struct *pDefaultBearerVaConfig =
            (const RIL_Default_Bearer_VA_Config_Struct *) ptrDefaultBearerVaConfig;
    MTK_RIL_Data_Call_Response_v11* response =
            (MTK_RIL_Data_Call_Response_v11*) malloc(sizeof(MTK_RIL_Data_Call_Response_v11));
    mal_datamngr_data_call_info_req_t* responseToMal = (mal_datamngr_data_call_info_req_t*)
            calloc(1, sizeof(mal_datamngr_data_call_info_req_t));
    int activatedPdnNum = 0;
    int concatenatedBearerNum = 0;
    int *activatedCidList = (int *) malloc(pdnInfoSize * sizeof(int));

    if (response == NULL || responseToMal == NULL || activatedCidList == NULL) {
        LOGE("[%s] calloc for response/responseToMal/activatedCidList failed!", __FUNCTION__);
        return;
    }

    initialDataCallResponse(response, 1);
    initialMalDataCallResponse(responseToMal, 1);

    int isAddrV4V6Bitmap = 0; //Bitmap: 00:none, 01:IPv4, 10:IPv6, 11: IPv4v6
    int originalNwCause = 0;
    int errIPv4v6 = 0;
    int errIPv4 = 0;
    int errIPv6 = 0;
    int errPdnContAct = PDN_FAILED; //PDN context and activation response status
    epdgConfig_t epdgConfig = {0, 0};

    configEpdg(&epdgConfig, pDefaultBearerVaConfig);

    // If under detached, ignore CGACT/EGACT, which may bring PS on again by MD.
    int slotId = getMappingSIMByCurrentMode(rid);
    if (!isMultiPsAttachSupport() && !isDataAllow(slotId)) {
        LOGD("[%d][RILData_GSM][%s] isDataAllow() is false.", rid, __FUNCTION__);
        response->status = gprs_failure_cause = PDP_FAIL_DATA_NOT_ALLOW;
        goto error;
    }

    for (i = 0; i < pdnInfoSize; i++) {
        activatedCidList[i] = INVALID_CID;
    }

    // Check if the IPv4 and IPv6 PDP context existed
    for (i = 0; i < pdnInfoSize; i++) {
        if ((interfaceId == pdn_info[rid][i].interfaceId)
                && (DATA_STATE_ACTIVE == pdn_info[rid][i].active)) {
            isFallbackRetrying = true;
            updatePdnAddressByCid(i, DATA_CHANNEL_CTX);
            if (strlen(pdn_info[rid][i].addressV4) > 0
                    && strcmp(pdn_info[rid][i].addressV4, NULL_IPV4_ADDRESS) != 0) {
                isAddrV4V6Bitmap |= IPV4 + 1;
            }
            if (strlen(pdn_info[rid][i].addressV6) > 0
                    && strcmp(pdn_info[rid][i].addressV6, NULL_IPV4_ADDRESS) != 0) {
                isAddrV4V6Bitmap |= IPV6 + 1;
            }
            LOGI("[%d]requestSetupDataCallFallback bitmap[0x%x](00:none, 01:IPv4, 10:IPv6, 11: IPv4v6)",
                    rid, isAddrV4V6Bitmap);
            break;
        }
    }

    // Disable MD fallback for AP fallback
    // AT+EGFB=<fallback_enable>,<ipv4_first>
    err = at_send_command("AT+EGFB=0,1", &p_response, DATA_CHANNEL_CTX);
    AT_RSP_FREE(p_response);

    // Retry one of IPv4 or IPv6 since the PDP context existed
    if ((IPV4V6 + 1) == isAddrV4V6Bitmap) {
        LOGE("[%d]requestSetupDataCallFallback should not retry here due to both IPv4 and IPv6 exist", rid);
    } else if (((IPV4 + 1) == isAddrV4V6Bitmap) || ((IPV6 + 1) == isAddrV4V6Bitmap)) {
        if ((IPV4 + 1) == isAddrV4V6Bitmap) {
            protocol = IPV6;  // has IPv4 but no IPv6, retry IPv6
        } else {
            protocol = IPV4;  // has IPv6 but no IPv4, retry IPv4
        }
        LOGD("[%d]requestSetupDataCallFallback retry [%d](v4=0;v6=1)", rid, protocol);

        int availableCid = getAvailableCid(rid);
        if (availableCid == INVALID_CID) {
            LOGE("[%d]requestSetupDataCallFallback no available CID to use", rid);
            response->status = gprs_failure_cause = PDP_FAIL_MAX_ACTIVE_PDP_CONTEXT_REACHED;
            goto error;
        } else {
            LOGI("[%d]requestSetupDataCallFallback available CID is [%d]", rid, availableCid);
        }
        // define Pdn ctx
        char overrideApn[PROPERTY_VALUE_MAX] = {0};
        property_get("vendor.ril.pdn.overrideApn", overrideApn, "");
        if (strlen(overrideApn) > 0) {
            errPdnContAct = definePdnCtx(pDefaultBearerVaConfig, overrideApn, protocol,
                    availableCid, authType, username, password, DATA_CHANNEL_CTX);
            if (PDN_SUCCESS == errPdnContAct) {
                property_set("vendor.ril.pdn.overrideApn", "");
            }
        } else {
            errPdnContAct = definePdnCtx(pDefaultBearerVaConfig, requestedApn, protocol,
                    availableCid, authType, username, password, DATA_CHANNEL_CTX);
        }
        if (PDN_FAILED == errPdnContAct) {
            isPdpFailCauseUpdateNeeded = true;
            goto error;
        }

        // packet-domain event reporting: +CGEREP=<mode>,<bfr>
        err = at_send_command("AT+CGEREP=1,0", NULL, DATA_CHANNEL_CTX);

        // activate PDN
        errPdnContAct = activatePdn(availableCid, &activatedPdnNum, &concatenatedBearerNum,
                             activatedCidList, isEmergency, isNoDataActive, DATA_CHANNEL_CTX);
        if (PDN_FAILED >= errPdnContAct) {
            if (PDN_FAILED == errPdnContAct) {
                isPdpFailCauseUpdateNeeded = true;
            }
            suggestedRetryTime = getModemSuggestedRetryTime(availableCid, DATA_CHANNEL_CTX);
            goto error;
        } else {
            protocol = IPV4V6;
        }
    } else {
        // After PS attached, query if there is any existed PDN with the same APN
        // It could be re-used if the APN is the same
        int queryResult = queryMatchedPdnWithSameApn(requestedApn, activatedCidList, DATA_CHANNEL_CTX);
        if (queryResult > 0) {
            for (i = 0; i < queryResult; i++) {
                int cid = activatedCidList[i];
                if (isCidTypeMatched(cid, protocol, DATA_CHANNEL_CTX)) {
                    if (strlen(pdn_info[rid][cid].addressV4) > 0
                            && strcmp(pdn_info[rid][cid].addressV4, NULL_IPV4_ADDRESS) != 0) {
                        isAddrV4V6Bitmap |= IPV4 + 1;
                    }
                    if (strlen(pdn_info[rid][cid].addressV6) > 0
                            && strcmp(pdn_info[rid][cid].addressV6, NULL_IPV6_ADDRESS) != 0) {
                        isAddrV4V6Bitmap |= IPV6 + 1;
                    }
                    LOGI("[%d]requestSetupDataCallFallback queryResult[%d]; all success:0x11, all error:0x00"
                            " [0x%x] (00:none, 01:IPv4, 10:IPv6, 11: IPv4v6)", rid, queryResult,
                            isAddrV4V6Bitmap);

                    // Check which protocol is accepted
                    if ((IPV4V6 + 1) == isAddrV4V6Bitmap) {
                        protocol = IPV4V6;
                        errIPv4 = 0;
                        errIPv6 = 0;
                    } else if ((IPV4 + 1) == isAddrV4V6Bitmap) {
                        protocol = IPV4;
                        errIPv6 = 1;
                    } else {
                        protocol = IPV6;
                        errIPv4 = 1;
                    }
                }
            }
            activatedPdnNum = queryResult;
        } else {
            int availableCid = getAvailableCid(rid);
            if (availableCid == INVALID_CID) {
                LOGE("[%d]requestSetupDataCallFallback no available CID to use", rid);
                response->status = gprs_failure_cause = PDP_FAIL_MAX_ACTIVE_PDP_CONTEXT_REACHED;
                goto error;
            } else {
                LOGI("[%d]requestSetupDataCallFallback available CID is [%d]", rid, availableCid);
            }
            // define Pdn ctx
            char overrideApn[PROPERTY_VALUE_MAX] = {0};
            property_get("vendor.ril.pdn.overrideApn", overrideApn, "");
            if (strlen(overrideApn) > 0) {
                errPdnContAct = definePdnCtx(pDefaultBearerVaConfig, overrideApn, protocol,
                        availableCid, authType, username, password, DATA_CHANNEL_CTX);
                if (PDN_SUCCESS == errPdnContAct) {
                    property_set("vendor.ril.pdn.overrideApn", "");
                }
            } else {
                errPdnContAct = definePdnCtx(pDefaultBearerVaConfig, requestedApn, protocol,
                        availableCid, authType, username, password, DATA_CHANNEL_CTX);
            }
            if (PDN_FAILED == errPdnContAct) {
                isPdpFailCauseUpdateNeeded = true;
                goto error;
            }

            // packet-domain event reporting: +CGEREP=<mode>,<bfr>
            err = at_send_command("AT+CGEREP=1,0", NULL, DATA_CHANNEL_CTX);

            // activate PDN
            errPdnContAct = activatePdn(availableCid, &activatedPdnNum, &concatenatedBearerNum,
                    activatedCidList, isEmergency, isNoDataActive, DATA_CHANNEL_CTX);
            if (PDN_FAILED >= errPdnContAct) {
                suggestedRetryTime = getModemSuggestedRetryTime(availableCid, DATA_CHANNEL_CTX);
                // IPv4v6 is failed without any IPv4 or IPv6 accepted, try both IPv4 and IPv6
                // If both IPv4 and IPv6 fallback are rejected, keep the original NW cause for data FW
                originalNwCause = gprs_failure_cause;
                errIPv4v6 = 1;

                if (CHANNEL_CLOSED == errPdnContAct) {
                    goto error;
                }
                if (LOCAL_REJECT_CAUSE == errPdnContAct) {
                    isPdpFailCauseUpdateNeeded = true;
                    goto error;
                }
            }

            errPdnContAct = PDN_FAILED;
            // Fallback if IPv4v6 is rejected
            if (errIPv4v6) {
                isAddrV4V6Bitmap = 0;

                if (SM_ONLY_IPV6_ALLOWED != originalNwCause) {
                    // Fallback IPv4 first
                    availableCid = getAvailableCid(rid);
                    if (availableCid == INVALID_CID) {
                        LOGE("[%d]requestSetupDataCallFallback no available CID to use", rid);
                        errIPv4 = 1;
                    } else {
                        LOGI("[%d]requestSetupDataCallFallback available CID is [%d]", rid, availableCid);
                    }

                    // define Pdn ctx
                    char overrideApn[PROPERTY_VALUE_MAX] = {0};
                    property_get("vendor.ril.pdn.overrideApn", overrideApn, "");
                    if (0 == errIPv4) {
                        if (strlen(overrideApn) > 0) {
                            errPdnContAct = definePdnCtx(pDefaultBearerVaConfig, overrideApn, IPV4, availableCid,
                                        authType, username, password, DATA_CHANNEL_CTX);

                            if (PDN_SUCCESS == errPdnContAct) {
                                property_set("vendor.ril.pdn.overrideApn", "");
                            }
                        } else {
                            errPdnContAct = definePdnCtx(pDefaultBearerVaConfig, requestedApn, IPV4, availableCid,
                                        authType, username, password, DATA_CHANNEL_CTX);
                        }
                    }

                    if (PDN_FAILED == errPdnContAct) {
                        errIPv4 = 1;
                    } else {
                        errPdnContAct = activatePdn(availableCid, &activatedPdnNum, &concatenatedBearerNum,
                                activatedCidList, isEmergency, isNoDataActive, DATA_CHANNEL_CTX);
                        if (PDN_FAILED >= errPdnContAct) {
                            suggestedRetryTime = getModemSuggestedRetryTime(availableCid,
                                    DATA_CHANNEL_CTX);
                            errIPv4 = 1;
                            if (CHANNEL_CLOSED == errPdnContAct) {
                                goto error;
                            }
                        } else {
                            errIPv4 = 0;
                            errIPv6 = 1;
                            isAddrV4V6Bitmap |= IPV4 + 1;
                        }
                    }
                } else {
                    errIPv4 = 1;
                }

                errPdnContAct = PDN_FAILED;
                if (1 == errIPv4 && SM_ONLY_IPV4_ALLOWED != originalNwCause) {
                    // Fallback IPv6 second
                    availableCid = getAvailableCid(rid);
                    if (availableCid == INVALID_CID) {
                        LOGE("[%d]requestSetupDataCallFallback no available CID to use", rid);
                        errIPv6 = 1;
                    } else {
                        LOGI("[%d]requestSetupDataCallFallback available CID is [%d]", rid, availableCid);
                    }

                    // define Pdn ctx
                    char overrideApn[PROPERTY_VALUE_MAX] = {0};
                    property_get("vendor.ril.pdn.overrideApn", overrideApn, "");
                    if (0 == errIPv6) {
                        if (strlen(overrideApn) > 0) {
                            errPdnContAct = definePdnCtx(pDefaultBearerVaConfig, overrideApn, IPV6, availableCid,
                                        authType, username, password, DATA_CHANNEL_CTX);
                            if (PDN_SUCCESS == errPdnContAct) {
                                property_set("vendor.ril.pdn.overrideApn", "");
                            }
                        } else {
                            errPdnContAct = definePdnCtx(pDefaultBearerVaConfig, requestedApn, IPV6, availableCid,
                                        authType, username, password, DATA_CHANNEL_CTX);
                        }
                    }

                    if (PDN_FAILED == errPdnContAct) {
                        errIPv6 = 1;
                    } else {
                        errPdnContAct = activatePdn(availableCid, &activatedPdnNum, &concatenatedBearerNum,
                                activatedCidList, isEmergency, isNoDataActive, DATA_CHANNEL_CTX);
                        if (PDN_FAILED >= errPdnContAct) {
                            suggestedRetryTime = getModemSuggestedRetryTime(availableCid,
                                    DATA_CHANNEL_CTX);
                            errIPv6 = 1;
                            if (CHANNEL_CLOSED == errPdnContAct) {
                                goto error;
                            }
                        } else {
                            errIPv6 = 0;
                            isAddrV4V6Bitmap |= IPV6 + 1;
                        }
                    }
                }

                if (errIPv4 && errIPv6) {
                    LOGE("[%d]requestSetupDataCallFallback both IPv4 and IPv6 fallback are failed", rid);
                    gprs_failure_cause = originalNwCause; // Respond data FW the original NW cause
                    isPdpFailCauseUpdateNeeded = true;
                    goto error;
                } else {
                    // Check which protocol is accepted
                    if ((IPV4 + 1) == isAddrV4V6Bitmap) {
                        protocol = IPV4;
                    } else {
                        protocol = IPV6;
                    }
                }
            } else {
                // Fallback if anyone of IPv4 or IPv6 is rejected
                // Check if IPv4 or IPv6 or both PDP context accepted
                int ret = updatePdnAddressByCid(availableCid, DATA_CHANNEL_CTX);
                if (ret != 0) {
                    LOGE("[%d]requestSetupDataCallFallback: updatePdnAddressByCid fail, deactive pdn:cid=%d ",
                            rid, availableCid);
                    deactivateDataCall(availableCid, DATA_CHANNEL_CTX);
                    response->status = gprs_failure_cause = PDP_FAIL_ERROR_UNSPECIFIED;
                    goto error;
                }
                isAddrV4V6Bitmap = 0;
                if (strlen(pdn_info[rid][availableCid].addressV4) > 0
                        && strcmp(pdn_info[rid][availableCid].addressV4, NULL_IPV4_ADDRESS) != 0) {
                    isAddrV4V6Bitmap |= IPV4 + 1;
                }
                if (strlen(pdn_info[rid][availableCid].addressV6) > 0
                        && strcmp(pdn_info[rid][availableCid].addressV6, NULL_IPV4_ADDRESS) != 0) {
                    isAddrV4V6Bitmap |= IPV6 + 1;
                }
                LOGD("[%d]requestSetupDataCallFallback must be 0x01 or 0x10 [0x%x]"
                        "(00:none, 01:IPv4, 10:IPv6, 11: IPv4v6)", rid, isAddrV4V6Bitmap);

                // If either IPv4 or IPv6 is accepted
                if (((IPV4V6 + 1) != isAddrV4V6Bitmap) && (0 != isAddrV4V6Bitmap)) {
                    if ((IPV4 + 1) == isAddrV4V6Bitmap) {
                        protocol = IPV4;  // has IPv4 but no IPv6, fallback IPv6
                        errIPv6 = 1;
                    } else {
                        protocol = IPV6;  // has IPv6 but no IPv4, fallback IPv4
                        errIPv4 = 1;
                    }
                } else {
                    // Either IPv4v6 accepted or rejected
                    LOGD("[%d]requestSetupDataCallFallback all success:0x11, all error:0x00 [0x%x] "
                            "(00:none, 01:IPv4, 10:IPv6, 11: IPv4v6)", rid, isAddrV4V6Bitmap);
                    if (0 == isAddrV4V6Bitmap) {
                        response->status = gprs_failure_cause = PDP_FAIL_ERROR_UNSPECIFIED;
                        goto error;
                    }
                }
            }
        }
    }

    for (i = 0; i < activatedPdnNum; i++) {
        int cid = activatedCidList[i];
        if (pdn_info[rid][cid].isDedicateBearer) {
            LOGD("[%d][%s] skip update PDP address of CID%d since it is a dedicate bearer [active=%d]",
                    rid, __FUNCTION__, cid, pdn_info[rid][cid].active);
        } else {
            if (updatePdnAddressByCid(cid, DATA_CHANNEL_CTX) < 0) {
                LOGD("[%d][%s] update PDP address of CID%d failed", rid, __FUNCTION__, cid);
                // we do not goto error since we expect to update all activated PDP address here
            }
            if (0 == bindPdnToIntf(activatedPdnNum, interfaceId, cid, DATA_CHANNEL_CTX)) {
                response->status = gprs_failure_cause = PDP_FAIL_ERROR_UNSPECIFIED;
                goto error;
            }
        }
        if ((IPV4_ONLY == pdn_info[rid][cid].reason)
                || (IPV6_ONLY == pdn_info[rid][cid].reason)) {
            isFallbackNeeded = 0;
            LOGD("[%d][%s] don't need to fallback anymore due to reason(%d), -1: no cause, "
                    "0: ipv4 only, 1: ipv6 only, 2: single bearer only allowed",
                    rid, __FUNCTION__, pdn_info[rid][cid].reason);
        } else if (SINGLE_BEARER_ONLY_ALLOWED == pdn_info[rid][cid].reason) {
            isAddrV4V6Bitmap = 0;
            gprs_failure_cause = SM_ONLY_SINGLE_BEARER_ALLOWED;
            if (strlen(pdn_info[rid][cid].addressV4) > 0
                    && strcmp(pdn_info[rid][cid].addressV4, NULL_IPV4_ADDRESS) != 0) {
                isAddrV4V6Bitmap |= IPV4 + 1;
            }
            if (strlen(pdn_info[rid][cid].addressV6) > 0
                    && strcmp(pdn_info[rid][cid].addressV6, NULL_IPV4_ADDRESS) != 0) {
                isAddrV4V6Bitmap |= IPV6 + 1;
            }
            if (((IPV4V6 + 1) != isAddrV4V6Bitmap) && (0 != isAddrV4V6Bitmap)) {
                if ((IPV4 + 1) == isAddrV4V6Bitmap) {
                    errIPv6 = 1;
                } else {
                    errIPv4 = 1;
                }
            }
        }
    }

    updateDynamicParameter(DATA_CHANNEL_CTX);

    if (updateDns(DATA_CHANNEL_CTX) < 0) {
        LOGE("[%d][%s] updateDns failed", rid, __FUNCTION__);
        response->status = gprs_failure_cause = PDP_FAIL_ERROR_UNSPECIFIED;
        goto error;
    }

    if (DATA_STATE_INACTIVE == isCidActive(activatedCidList, activatedPdnNum, DATA_CHANNEL_CTX)) {
        response->status = gprs_failure_cause = PDP_FAIL_LOST_CONNECTION;
        goto error;
    }

    if (isFallbackRetrying) {
        configureNetworkInterface(interfaceId, REACT_CCMNI, rid);
    } else {
        configureNetworkInterface(interfaceId, ENABLE_CCMNI, rid);
    }

    int cid = createDataResponse(interfaceId, protocol, response, rid);
    if (errIPv4 || errIPv6) {
        if (isFallbackNeeded && isFallbackPdpRetryFail(rid)) {
            response->status = PDP_FAIL_FALLBACK_RETRY;
        }
    } else {
        LOGE("[%d]requestSetupDataCallFallback both v4 and v6 are accepted", rid);
    }

    // Basically we don't need to care about dedicated bearer due to Telstra only retry on 3G
    createMalDataResponse(interfaceId, cid, response, responseToMal, rid);
    makeDedicatedDataResponse(concatenatedBearerNum, activatedPdnNum, activatedCidList, responseToMal, rid);
    setRspEranType(epdgConfig.eranType, response, responseToMal, rid);
    dumpDataResponse(response, "requestSetupDataCallFallback dump response");
    dumpMalDataResponse(responseToMal, "requestSetupDataCallFallback dump mal response");
    responseUnsolDataCallRspToMal(t, responseToMal, sizeof(mal_datamngr_data_call_info_req_t));

    RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(MTK_RIL_Data_Call_Response_v11));

    goto finish;

error:
    LOGE("[%d]requestSetupDataCallFallback response ERROR", rid);
    for (i = 0; i < activatedPdnNum; i++)
        clearPdnInfo(&pdn_info[rid][activatedCidList[i]]);

    if (isFallbackRetrying) {
        if (isPdpFailCauseUpdateNeeded) {
            if (isFallbackNeeded && isFallbackPdpRetryFail(rid)) {
                response->status = PDP_FAIL_FALLBACK_RETRY;
            } else if (0 != gprs_failure_cause) {
                response->status = gprs_failure_cause;
            }
        }
    } else {
        if (isPdpFailCauseUpdateNeeded) {
            if (0 != gprs_failure_cause) {
                response->status = gprs_failure_cause;
            }
        }
    }

    responseUnsolDataCallRspToMal(t, NULL, 0);

    if (CHANNEL_CLOSED == errPdnContAct) {
        RIL_onRequestComplete(t, RIL_E_RADIO_NOT_AVAILABLE, NULL, 0);
    } else {
        storeModemSuggestedRetryTime(suggestedRetryTime, response);
        RIL_onRequestComplete(t, RIL_E_SUCCESS,
                response, sizeof(MTK_RIL_Data_Call_Response_v11));
    }

finish:
    freeDataResponse(response);
    freeMalDataResponse(responseToMal);
    free(response);
    free(activatedCidList);
    free(responseToMal);

    LOGV("[%d]requestSetupDataCallFallback finish", rid);
    return;
}

void requestOrSendDataCallListIpv6(RILChannelCtx* rilchnlctx, RIL_Token *t, RIL_SOCKET_ID rid)
{
    ATResponse *p_response = NULL;
    ATLine *p_cur = NULL;
    int err = 0;
    char *out = NULL;
    int i = 0, j = 0;
    MTK_RIL_Data_Call_Response_v11* responsesOfActive = NULL;
    mal_datamngr_data_call_info_req_t* responsesToMalOfActive = NULL;
    int activeCount = 0;
    int totalDedicateBearerCount = 0;
    int updateToMbimIndex = 0;
    int mbimCid = INVALID_CID;
    int sim_id = getValidSimId(rid, __FUNCTION__);
    int maxCcmniNum = MAX_CCMNI_NUMBER;

    if (isMultiPsAttachSupport()) {
        maxCcmniNum = MAX_MPS_CCMNI_NUMBER;
    }

    LOGV("[%d][%s] X", rid, __FUNCTION__);
    if (!s_md_off) {
        if (t != NULL) {
            rilchnlctx = getRILChannelCtxFromToken(*t);
        } else {
            rilchnlctx = getChannelCtxbyProxy();
        }
        if (queryCurrentConnType(rilchnlctx) == 0) {
            LOGD("[%d]requestOrSendDataCallListIpv6 conn type when needed then skip.", rid);
            if (t != NULL) {
                RIL_onRequestComplete(*t, RIL_E_SUCCESS, NULL, 0);
            } else {
                RIL_UNSOL_RESPONSE(RIL_UNSOL_DATA_CALL_LIST_CHANGED, NULL, 0, rid);
            }
            // Skip query since current is not PS active.
            return;
        }
    }

    MTK_RIL_Data_Call_Response_v11* responses = (MTK_RIL_Data_Call_Response_v11*) calloc(1,
            maxCcmniNum * sizeof(MTK_RIL_Data_Call_Response_v11));
    mal_datamngr_data_call_info_req_t* responsesToMal = (mal_datamngr_data_call_info_req_t*) calloc(1,
            maxCcmniNum * sizeof(mal_datamngr_data_call_info_req_t));
    if (responses == NULL || responsesToMal == NULL) {
        goto error;
    }
    initialDataCallResponse(responses, maxCcmniNum);
    initialMalDataCallResponse(responsesToMal, maxCcmniNum);

    if (!s_md_off) {
        updateActiveStatus(rilchnlctx);

        for (i = 0; i < pdnInfoSize; i++) {
            if (!pdn_info[rid][i].isDedicateBearer)
                updatePdnAddressByCid(i, rilchnlctx);
                //not to check error since we expect to query all PDN connections even if error occurred
        }

        if (updateDns(rilchnlctx) < 0) {
            LOGE("[%d]requestOrSendDataCallListIpv6 updateDns failed", rid);
            goto error;
        }
    }

    for (i = 0; i < maxCcmniNum; i++) {
        int cid = createDataResponse(i, IPV4V6, &responses[i], rid);
        createMalDataResponse(i, cid, &responses[i], &responsesToMal[i],rid);
        if (responses[i].active == DATA_STATE_ACTIVE) {
            int currDefaultCid = responsesToMal[i].defaultBearer.cid;

            ++activeCount;

            /// M: Ims Data Framework {@
            // to create dedicate bearer response
            int dedicateBearerNum = 0;
            for (j = 0; j < pdnInfoSize; j++) {
                if (pdn_info[rid][j].isDedicateBearer
                        && pdn_info[rid][j].cid != INVALID_CID
                        && pdn_info[rid][j].primaryCid == currDefaultCid
                        && pdn_info[rid][j].active == DATA_STATE_ACTIVE)
                {
                    ++dedicateBearerNum;
                }
            }

            if (dedicateBearerNum > 0) { //create concatenated dedicate bearer response
                int count = 0;
                MALRIL_Dedicate_Data_Call_Struct* dedicateResponses =
                        alloca(dedicateBearerNum * sizeof(MALRIL_Dedicate_Data_Call_Struct));
                initialDedicateDataCallResponse(dedicateResponses, dedicateBearerNum);
                for (j=0; j<pdnInfoSize; j++) {
                    if (pdn_info[rid][j].isDedicateBearer
                            && pdn_info[rid][j].cid != INVALID_CID
                            && pdn_info[rid][j].primaryCid == responses[i].cid
                            && pdn_info[rid][j].active == DATA_STATE_ACTIVE)
                    {
                        createDedicateDataResponse(j, &dedicateResponses[count], rid);
                        dumpDedicateDataResponse(&dedicateResponses[count],
                                "requestOrSendDataCallListIpv6 dump dedicate bearer response");
                        ++count;
                    }
                }

                responsesToMal[i].concatenateNum = dedicateBearerNum;
                responsesToMal[i].concatenate = dedicateResponses;

                totalDedicateBearerCount += dedicateBearerNum;
            }
            /// @}
        }
    }

    if (activeCount > 0) {
        LOGI("requestOrSendDataCallListIpv6 active interface number is %d", activeCount);
        responsesOfActive = (MTK_RIL_Data_Call_Response_v11*)
                calloc(1, activeCount * sizeof(MTK_RIL_Data_Call_Response_v11));
        if (responsesOfActive == NULL) {
            goto error;
        }
        responsesToMalOfActive = (mal_datamngr_data_call_info_req_t*)
                calloc(1, activeCount * sizeof(mal_datamngr_data_call_info_req_t));
        assert(responsesToMalOfActive != NULL);
        int index = 0;
        for (i = 0; i < maxCcmniNum; i++) {
            if (responses[i].active == DATA_STATE_ACTIVE) {
                dumpDataResponse(&responses[i],
                        "requestOrSendDataCallListIpv6 dump interface status");
                dumpMalDataResponse(&responsesToMal[i],
                        "requestOrSendDataCallListIpv6 dump mal interface status");
                memcpy(&responsesOfActive[index], &responses[i],
                        sizeof(MTK_RIL_Data_Call_Response_v11));
                memcpy(&responsesToMalOfActive[index], &responsesToMal[i],
                        sizeof(mal_datamngr_data_call_info_req_t));
                dumpDataResponse(&responsesOfActive[index],
                        "**after copy responsesOfActive dump interface status");
                dumpMalDataResponse(&responsesToMalOfActive[index],
                        "**after copy malResponsesOfActive dump mal interface status");
                ++index;
            }
        }
    } else {
        LOGD("[%d]requestOrSendDataCallListIpv6 no active response", rid);
    }

    if (t != NULL) {
        RIL_onRequestComplete(*t, RIL_E_SUCCESS,
            activeCount > 0 ? responsesOfActive : NULL,
            activeCount * sizeof(MTK_RIL_Data_Call_Response_v11)
            /*+ totalDedicateBearerCount * sizeof(RIL_Dedicate_Data_Call_Struct)*/);
    } else {
        RIL_UNSOL_RESPONSE(RIL_UNSOL_DATA_CALL_LIST_CHANGED,
            activeCount > 0 ? responsesOfActive : NULL,
            activeCount * sizeof(MTK_RIL_Data_Call_Response_v11)
            /*+ totalDedicateBearerCount * sizeof(RIL_Dedicate_Data_Call_Struct)*/,
            rid);
        mal_datamngr_data_call_info_rsp_ptr_t rsp_ptr =
                calloc(1, activeCount * sizeof(mal_datamngr_data_call_info_rsp_t));
        mal_datamngr_data_call_info_req_ptr_t req_ptr[activeCount];
        for (int i = 0; i < activeCount; i++) {
            req_ptr[i] = &responsesToMalOfActive[i];
            LOGD("[%s] The pdn memory address: req_ptr[%d]=%p , responsesToMalOfActive[%d]=%p", __FUNCTION__, i, req_ptr[i], i, &responsesToMalOfActive[i]);
        }
        if (sim_id != SIM_ID_INVALID) {
            if (mal_datamngr_notify_data_call_list_tlv != NULL) {
                int ret = mal_datamngr_notify_data_call_list_tlv (mal_once(1, mal_cfg_type_sim_id, sim_id), activeCount,
                        req_ptr, &rsp_ptr, NULL, NULL, NULL, NULL);
                LOGD("[%s] Call mal_datamngr_notify_data_call_list_tlv success", __FUNCTION__);
            } else {
                LOGE("[%s] mal_datamngr_notify_data_call_list_tlv is null", __FUNCTION__);
            }
        }
        FREEIF(rsp_ptr);
    }
    goto finish;

error:
    LOGE("[%d]requestOrSendDataCallListIpv6 response ERROR", rid);
    if (t != NULL) {
        RIL_onRequestComplete(*t, RIL_E_GENERIC_FAILURE, NULL, 0);
    } else {
        RIL_UNSOL_RESPONSE(RIL_UNSOL_DATA_CALL_LIST_CHANGED, NULL, 0, rid);
        mal_datamngr_data_call_info_rsp_ptr_t rsp_ptr =
                calloc(1, activeCount * sizeof(mal_datamngr_data_call_info_rsp_t));
        if (sim_id != SIM_ID_INVALID) {
            if (mal_datamngr_notify_data_call_list_tlv != NULL) {
                int ret = mal_datamngr_notify_data_call_list_tlv (mal_once(1, mal_cfg_type_sim_id, sim_id), 0,
                        NULL, &rsp_ptr, NULL, NULL, NULL, NULL);
                LOGD("[%s] Call mal_datamngr_notify_data_call_list_tlv success", __FUNCTION__);
            } else {
                LOGE("[%s] mal_datamngr_notify_data_call_list_tlv is null", __FUNCTION__);
            }
        }
        FREEIF(rsp_ptr);
    }
    AT_RSP_FREE(p_response);

finish:

    if (responses != NULL) {
        for (i = 0; i < maxCcmniNum; i++)
            freeDataResponse(&responses[i]);

        free(responses);
    }

    if (responsesOfActive != NULL) {
        LOGE("[%d]requestOrSendDataCallListIpv6 free responsesOfActive", rid);
        free(responsesOfActive);
    }

    if (responsesToMal != NULL) {
        for (i = 0; i < maxCcmniNum; i++)
            freeMalDataResponse(&responsesToMal[i]);

        free(responsesToMal);
    }

    if (responsesToMalOfActive!= NULL) {
        LOGE("[%d]requestOrSendDataCallListIpv6 free responsesToMalOfActive", rid);
        free(responsesToMalOfActive);
    }

    return;
}

// M: VzW

void syncApnTableToMdAndRds(void* data, size_t datalen, RIL_Token t){
    int resultOfSyncApnTable = syncApnTableToMd(data, datalen, t);
    int resultOfSyncApnTableToRds = syncApnTableToRds(data, datalen, t);

    if (resultOfSyncApnTable && resultOfSyncApnTableToRds) {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_MODEM_ERR, NULL, 0);
    }
}

int syncApnTableToMd(void* data, size_t datalen, RIL_Token t) {
    int result = 1;
    RILChannelCtx* pChannel = getRILChannelCtxFromToken(t);
    ATResponse *p_response = NULL;
    RIL_MtkDataProfileInfo **dataProfilePtrs = (RIL_MtkDataProfileInfo **) data;

    char cmd[AT_COMMAND_MEDIUM_LENGTH] = {0};
    int err = 0;
    int failCause = PDP_FAIL_NONE;
    int num = datalen / sizeof(RIL_MtkDataProfileInfo *);

    // DataProfile info
    int profileId = 0;
    char *apn = NULL;
    char *protocol = NULL;
    char *roamingProtocol = NULL;
    int authType = 0;
    char *user = NULL;
    char *password = NULL;
    int type = 0;
    int maxConnsTime = 0;
    int maxConns = 0;
    int waitTime = 0;
    int enabled = 0;
    int supportedTypesBitmask = 0;
    int bearerBitmask = 0;
    int mtu = 0;
    char *mvnoType = NULL;
    char *mvnoMatchData = NULL;
    int inactiveTimer = 0;

    // local ver.
    int apnClass = 0;
    int throttlingTime = 0;
    char *apnBearer = NULL;
    char *apnEnable = NULL;
    int apnTime = 0;

    LOGD("[%s]syncApnTable with datalen=%d, num of pararmeters = %d",
            __FUNCTION__, datalen, num);

    // Reset the old parameters in initial phase
    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "AT+EAPNSYNC");
    err = at_send_command(cmd, &p_response, pChannel);
    if (isATCmdRspErr(err, p_response)) {
        failCause = at_get_cme_error(p_response);
        LOGE("syncApnInfo AT+EAPNSYNC err=%d in initial phase", failCause);
    }
    AT_RSP_FREE(p_response);
    // if AT command not support, just take it as ok to pass VTS case.
    if (failCause == PDP_FAIL_CONDITIONAL_IE_ERROR) {
        return 1;
    }

    for (int i = 0; i < num; i++) {
        profileId = dataProfilePtrs[i]->profileId;
        apn = dataProfilePtrs[i]->apn;
        if (apn == NULL) {
            LOGD("[%s] apn name is empty", __FUNCTION__);
            continue;
        }
        protocol = dataProfilePtrs[i]->protocol;
        roamingProtocol = dataProfilePtrs[i]->roamingProtocol;
        authType = dataProfilePtrs[i]->authType;
        user = dataProfilePtrs[i]->user;
        password = dataProfilePtrs[i]->password;
        type = dataProfilePtrs[i]->type;
        maxConnsTime = dataProfilePtrs[i]->maxConnsTime;
        maxConns = dataProfilePtrs[i]->maxConns;
        waitTime = dataProfilePtrs[i]->waitTime;
        enabled = dataProfilePtrs[i]->enabled;
        if (enabled) {
            apnEnable = "Enabled";
        } else {
            apnEnable = "Disabled";
        }
        supportedTypesBitmask = dataProfilePtrs[i]->supportedTypesBitmask;
        bearerBitmask = dataProfilePtrs[i]->bearerBitmask;
        mtu = dataProfilePtrs[i]->mtu;
        mvnoType = dataProfilePtrs[i]->mvnoType;
        mvnoMatchData = dataProfilePtrs[i]->mvnoMatchData;
        inactiveTimer = dataProfilePtrs[i]->inactiveTimer;

        LOGD("[%s]index=%d, anp=%s, profileId=%d, supportedTypesBitmask=%d",
                __FUNCTION__, i, apn, profileId, supportedTypesBitmask);

        apnClass = getClassType(apn, supportedTypesBitmask);
        apnBearer = "LTE";

        char throttling_time[PROPERTY_VALUE_MAX] = {0};
        property_get("persist.vendor.radio.throttling_time", throttling_time, "900");
        throttlingTime = atoi(throttling_time);

        // AT+EAPNSYNC=<APN_index>,<APN>,<inactivity_time>
        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "AT+EAPNSYNC=%d,\"%s\",%d", i, apn, inactiveTimer);
        err = at_send_command(cmd, &p_response, pChannel);
        if (isATCmdRspErr(err, p_response)) {
            failCause = at_get_cme_error(p_response);
            LOGE("syncApnInfo AT+EAPNSYNC err=%d", failCause);
        }
        AT_RSP_FREE(p_response);

        // AT+VZWAPNE=<wapn>,<apncl>,<apnni>,<apntype>,<apnb>,<apned>,<apntime>
        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "AT+VZWAPNE=%d,%d,\"%s\",\"%s\",\"%s\",\"%s\",%d",
                i, apnClass, apn, protocol, apnBearer, apnEnable, apnTime);
        err = at_send_command(cmd, &p_response, pChannel);
        if (isATCmdRspErr(err, p_response)) {
            failCause = at_get_cme_error(p_response);
            LOGE("syncApnInfo AT+VZWAPNE err=%d", failCause);
            goto error;
        }
        AT_RSP_FREE(p_response);

        // AT+VZWAPNETMR=<wapn>,<max_conn>,<max_conn_t>,<wait_time>,<throttling_time>
        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "AT+VZWAPNETMR=%d,%d,%d,%d,%d",
                i, maxConns, maxConnsTime, waitTime, throttlingTime);
        err = at_send_command(cmd, &p_response, pChannel);
        if (isATCmdRspErr(err, p_response)) {
            failCause = at_get_cme_error(p_response);
            LOGE("syncApnTimer AT+VZWAPNETMR err=%d", failCause);
            goto error;
        }
        AT_RSP_FREE(p_response);
    }
    return result;
error:
    AT_RSP_FREE(p_response);
    result = 0;
    return result;
}

// M: [VzW] Data Framework @{
// Disable APN for VZW
void syncDisabledApnToMd(RILChannelCtx *pChannel) {
    FILE* fPtr = fopen("/mobile_info/vzwapn_info.txt", "r");
    char apnName[32];
    char *cmd = NULL;
    int err = 0;
    ATResponse *p_response = NULL;
    int failCause = PDP_FAIL_NONE;

    if (fPtr == NULL) {
        LOGE("syncDisabledApnToMd(), open file failed");
    } else {
        LOGD("syncDisabledApnToMd(), start");
        while (fgets(apnName, sizeof(apnName), fPtr) != NULL) {
            // AT+VZWAPNE=<wapn>,<apncl>,<apnni>,<apntype>,<apnb>,<apned>,<apntime>
            if (strcmp(apnName,"VZWADMIN") == 0) {
                asprintf(&cmd, "AT+VZWAPNE=0,2,\"%s\",\"IPV4V6\",\"LTE\",\"Disabled\",0",apnName);
            } else if (strcmp(apnName,"VZWIMS") == 0) {
                asprintf(&cmd, "AT+VZWAPNE=1,1,\"%s\",\"IPV4V6\",\"LTE\",\"Disabled\",0",apnName);
            } else {
                LOGD("syncDisabledApnToMd(), No need to disabled VZWADMIN / VZWIMS");
            }

            if (cmd != NULL) {
                err = at_send_command(cmd, &p_response, pChannel);
                free(cmd);

                if (isATCmdRspErr(err, p_response)) {
                    failCause = at_get_cme_error(p_response);
                    LOGE("syncDisabledApnToMd(), AT+VZWAPNE err=%d", failCause);
                }
                AT_RSP_FREE(p_response);
            }
        }
        fclose(fPtr);
    }
}
// M: [VzW] Data Framework @}

// M: VDF MMS over ePDG @{
int getApnProfileId(int apnType) {
    int profileId = -1;
    switch (apnType) {
        case RIL_APN_TYPE_DEFAULT:
            profileId = RIL_DATA_PROFILE_DEFAULT;
            break;
        case RIL_APN_TYPE_MMS:
            profileId = RIL_DATA_PROFILE_VENDOR_MMS;
            break;
        case RIL_APN_TYPE_SUPL:
            profileId = RIL_DATA_PROFILE_VENDOR_SUPL;
            break;
        case RIL_APN_TYPE_DUN:
            profileId = RIL_DATA_PROFILE_TETHERED;
            break;
        case RIL_APN_TYPE_HIPRI:
            profileId = RIL_DATA_PROFILE_VENDOR_HIPRI;
            break;
        case RIL_APN_TYPE_FOTA:
            profileId = RIL_DATA_PROFILE_FOTA;
            break;
        case RIL_APN_TYPE_IMS:
            profileId = RIL_DATA_PROFILE_IMS;
            break;
        case RIL_APN_TYPE_CBS:
            profileId = RIL_DATA_PROFILE_CBS;
            break;
        case RIL_APN_TYPE_IA:
            profileId = RIL_DATA_PROFILE_DEFAULT;
            break;
        case RIL_APN_TYPE_EMERGENCY:
            profileId = RIL_DATA_PROFILE_VENDOR_EMERGENCY;
            break;
        case RIL_APN_TYPE_MCX:
            profileId = RIL_DATA_PROFILE_VENDOR_MCX;
            break;
        case RIL_APN_TYPE_WAP:
            profileId = RIL_DATA_PROFILE_VENDOR_WAP;
            break;
        case RIL_APN_TYPE_XCAP:
            profileId = RIL_DATA_PROFILE_VENDOR_XCAP;
            break;
        case RIL_APN_TYPE_BIP:
            profileId = RIL_DATA_PROFILE_VENDOR_BIP;
            break;
        case RIL_APN_TYPE_VSIM:
            profileId = RIL_DATA_PROFILE_VENDOR_VSIM;
            break;
        default:
            LOGD("[%s]cannot find apnType=%d", __FUNCTION__, apnType);
            break;
    }
    LOGD("[%s]apnType=%d, profileId=%d", __FUNCTION__, apnType, profileId);
    return profileId;
}

bool bitmaskHasTech(int bearerBitmask, int radioTech) {
    if (bearerBitmask == 0) {
        return true;
    } else if (radioTech >= 1) {
        return ((bearerBitmask & (1 << (radioTech - 1))) != 0);
    }
    return false;
}

int syncApnTableToRds(void* data, size_t datalen, RIL_Token t) {
    int result = 1;
    int dataCount = (datalen/sizeof(RIL_MtkDataProfileInfo *));
    int listCount = 0;
    rds_apn_cfg_req_t* apnConfig = (rds_apn_cfg_req_t*) calloc(1, sizeof(rds_apn_cfg_req_t));
    assert(apnConfig != NULL);
    rds_apn_cfg_table_t* apnTable = (rds_apn_cfg_table_t*) calloc(dataCount, sizeof(rds_apn_cfg_table_t));
    assert(apnTable != NULL);
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    RIL_MtkDataProfileInfo **dataProfilePtrs = (RIL_MtkDataProfileInfo **) data;

    char *apnTypes = (char *)malloc(MAX_LENGTH_APN_TYPES_TO_RDS);
    assert(apnTypes != NULL);
    char *tempProileId = (char *)calloc(10, sizeof(char));
    assert(tempProileId != NULL);
    char *symbolStr = "|";
    int profileId = 0;
    int countOfApnType = 0;
    int apnTypesBitmask = 0;
    int bearerBitmask = 0;
    int invertIwlanBitMask = ~(1 << (RADIO_TECH_IWLAN - 1)) & 0xffffff;
    int sim_id = getValidSimId(rid, __FUNCTION__);

    LOGD("syncApnTableToRds start with datalen=%d, param  number=%d",
                (int) datalen, dataCount);

    if (isMultipleImsSupport() || getMainProtocolRid() == rid) {
        if (dataCount > RDS_APNCNT) {
            dataCount = RDS_APNCNT;
            LOGD("syncApnTableToRds dataCount exceeds limit:%d", RDS_APNCNT);
        }

        for(int i = 0; i < dataCount; i++) {
            if (dataProfilePtrs[i]->apn != NULL) {
                LOGD("[%s] index=%d, apn: %s", __FUNCTION__, i, dataProfilePtrs[i]->apn);
                // set Apn Name to apnTable
                cpStr(apnTable[i].apn_name, dataProfilePtrs[i]->apn,
                        strlen(dataProfilePtrs[i]->apn), STRING_SIZE);
            } else {
                LOGD("[%s] apn name is empty", __FUNCTION__);
                continue;
            }
            apnTypesBitmask = dataProfilePtrs[i]->supportedTypesBitmask;

            // The format of apnTable.apn_type is "profile_id|profile_id"
            // mapping apn type to data profile id
            memset(apnTypes, 0, STRING_SIZE * sizeof(char));
            countOfApnType = 0;
            // skip NONE, ALL and MTKALL
            if (apnTypesBitmask == RIL_APN_TYPE_UNKNOWN ||
                    apnTypesBitmask == RIL_APN_TYPE_ALL ||
                    apnTypesBitmask == RIL_APN_TYPE_MTKALL) {
            } else {
                for (int j = 0; j < RIL_APN_TYPE_COUNT; j++) {
                    if (apnTypesBitmask & (1 << j)) {
                        profileId = getApnProfileId(1 << j);
                        if (profileId >= 0) {
                            if (countOfApnType == 0) {
                                snprintf(apnTypes, MAX_LENGTH_APN_TYPES_TO_RDS - 1, "%d", profileId);
                            } else {
                                strncat(apnTypes, symbolStr, strlen(symbolStr));
                                snprintf(tempProileId, (10 * sizeof(char)) - 1, "%d", profileId);
                                strncat(apnTypes, tempProileId, strlen(tempProileId));
                            }
                            countOfApnType++;
                        }
                    }
                }

                // set Apn types to apnTable
                LOGD("[%s] index=%d, apnTypes: %s", __FUNCTION__, i, apnTypes);
                cpStr(apnTable[i].apn_type, apnTypes, strlen(apnTypes), STRING_SIZE);
            }

            // set Protocol to apnTable
            cpStr(apnTable[i].ip_protocol, dataProfilePtrs[i]->protocol,
                    strlen(dataProfilePtrs[i]->protocol), STRING_SIZE);

            bearerBitmask = dataProfilePtrs[i]->bearerBitmask;

            if (bearerBitmask == 0) {
                apnTable[i].rat_ability = APN_RAT_CELLULAR_ONLY;
            } else {
                if (bitmaskHasTech(bearerBitmask, RADIO_TECH_IWLAN)) {
                    if ((bearerBitmask & invertIwlanBitMask) == 0) {
                        apnTable[i].rat_ability = APN_RAT_WIFI_ONLY;
                    } else {
                        apnTable[i].rat_ability = APN_RAT_CELLULAR_WIFI;
                    }
                } else {
                    apnTable[i].rat_ability = APN_RAT_CELLULAR_ONLY;
                }
            }

            // set ApnTable to apnConfig
            listCount++;
            apnConfig->apn_cfg[listCount-1] = apnTable[i];
        }
        apnConfig->list_cnt = listCount;
        apnConfig->ucsim_id = sim_id;
        dumpApnConfig(apnConfig, listCount);
        if (rild_rds_set_apncfg != NULL) {
            rds_bool nReqRet = rild_rds_set_apncfg(apnConfig);
            if (nReqRet != RDS_TRUE) {
                result = 0;
                LOGE("[%s] sync apn to rds failed!!", __FUNCTION__);
            }
        } else {
            LOGE("[%s] MAL not supported!!", __FUNCTION__);
        }
    } else {
        LOGD("[%s] ignore apn sync event at non main protocol sim", __FUNCTION__);
    }

    LOGD("syncApnTableToRds done");
    free(apnConfig);
    free(apnTable);
    free(apnTypes);
    free(tempProileId);
    return result;
}
/// @}

// LCE service start
void requestStartLce(void* data, size_t datalen, RIL_Token t)
{
    UNUSED(datalen);
    int reportIntervalMs = 0;
    int mode = 0;
    RIL_LceStatusInfo* response = (RIL_LceStatusInfo*) calloc(1, sizeof(RIL_LceStatusInfo));
    if (response == NULL) {
        LOGE("[%s] calloc for response failed!", __FUNCTION__);
        goto error;
    }

    reportIntervalMs = ((int*) data)[0];
    mode = ((int*) data)[1] + 1; // PULL mode: 2; PUSH mode: 1;
    LOGD("[%s] reportIntervalMs=%d, mode=%d", __FUNCTION__, reportIntervalMs, mode);

    if(0 == setLceMode(mode, reportIntervalMs, response, DATA_CHANNEL_CTX)) {
        goto error;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(RIL_LceStatusInfo));
    goto finish;

error:
    LOGE("[%s] error", __FUNCTION__);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);

finish:
    free(response);
    LOGV("[%s] finish", __FUNCTION__);
    return;
}

void requestStopLce(void* data, size_t datalen, RIL_Token t)
{
    UNUSED(data);
    UNUSED(datalen);
    int reportIntervalMs = 0;
    int mode = 0; // STOP mode: 0;
    RIL_LceStatusInfo* response = (RIL_LceStatusInfo*) calloc(1, sizeof(RIL_LceStatusInfo));
    if (response == NULL) {
        LOGE("[%s] calloc for response failed!", __FUNCTION__);
        goto error;
    }

    LOGV("[%s] enter", __FUNCTION__);

    if(0 == setLceMode(mode, reportIntervalMs, response, DATA_CHANNEL_CTX)) {
        goto error;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(RIL_LceStatusInfo));
    goto finish;

error:
    LOGE("[%s] error", __FUNCTION__);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);

finish:
    free(response);
    LOGV("[%s] finish", __FUNCTION__);
    return;
}

void requestPullLceData(void* data, size_t datalen, RIL_Token t)
{
    UNUSED(data);
    UNUSED(datalen);
    int err = 0;
    ATResponse *p_response = NULL;
    char* line;
    int t_mode = 0; // STOP mode: 0; START with PUSH mode: 1; START with PULL mode: 2;
    unsigned int t_last_hop_capacity_kbps = 0;
    unsigned int t_confidence_level = 0;
    unsigned int t_lce_suspended = 0;
    RIL_LceDataInfo* response = (RIL_LceDataInfo*) calloc(1, sizeof(RIL_LceDataInfo));
    if (response == NULL) {
        LOGE("[%s] calloc for response failed!", __FUNCTION__);
        goto error;
    }

    // +ELCE: <mode>[,<last_hop_cap>, <confidenc_level>,<lce_suspend>]
    err = at_send_command_singleline("AT+ELCE?", "+ELCE:", &p_response, DATA_CHANNEL_CTX);
    if (err != 0 || p_response->success == 0 || p_response->p_intermediates == NULL) {
        LOGE("[%s] AT+ELCE? got error response", __FUNCTION__);
        goto error;
    } else {
        line = p_response->p_intermediates->line;

        err = at_tok_start(&line);
        if (err < 0) goto error;

        err = at_tok_nextint(&line, &t_mode);
        if (err < 0) goto error;

        if (at_tok_hasmore(&line)) {
            err = at_tok_nextint(&line, &t_last_hop_capacity_kbps);
            if (err < 0) goto error;

            if (at_tok_hasmore(&line)) {
                err = at_tok_nextint(&line, &t_confidence_level);
                if (err < 0) goto error;

                if (at_tok_hasmore(&line)) {
                    err = at_tok_nextint(&line, &t_lce_suspended);
                    if (err < 0) goto error;
                }
            }
        }

        LOGD("[%s] response [mode=%d, last hop capacity kbps=%u, confidence level=%u, "
                "lce suspended=%u]", __FUNCTION__, t_mode, t_last_hop_capacity_kbps,
                t_confidence_level, t_lce_suspended);
        response->last_hop_capacity_kbps = t_last_hop_capacity_kbps;
        response->confidence_level = (unsigned char) t_confidence_level;
        response->lce_suspended = (unsigned char) t_lce_suspended;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(RIL_LceDataInfo));
    goto finish;

error:
    LOGE("[%s] error", __FUNCTION__);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);

finish:
    if (response != NULL) {
        free(response);
    }
    AT_RSP_FREE(p_response);
    LOGV("[%s] finish", __FUNCTION__);
    return;
}

int setLceMode(int lceMode, int reportIntervalMs, RIL_LceStatusInfo* response, RILChannelCtx *pChannel)
{
    int nRet = 0;
    int err = 0;
    char *cmd = NULL;
    ATResponse *p_response = NULL;
    char* line = NULL;
    int lceStatus = -1; // not supported: -1; stopped: 0; active: 1;
    unsigned int actualIntervalMs = 0;

    if (lceMode > 0) {
        asprintf(&cmd, "AT+ELCE=%d,%d", lceMode, reportIntervalMs);
    } else {
        asprintf(&cmd, "AT+ELCE=%d", lceMode);
    }

    err = at_send_command_singleline(cmd, "+ELCE:", &p_response, pChannel);
    free(cmd);
    if (err != 0 || p_response->success == 0 || p_response->p_intermediates == NULL) {
        LOGE("[%s] got error response", __FUNCTION__);
        response->lce_status = (char) lceStatus;
    } else {
        line = p_response->p_intermediates->line;

        err = at_tok_start(&line);
        if (err < 0) goto error;

        err = at_tok_nextint(&line, &lceStatus);
        if (err < 0) goto error;

        if (at_tok_hasmore(&line)) {
            err = at_tok_nextint(&line, &actualIntervalMs);
            if (err < 0) goto error;
        }

        LOGD("[%s] response [lce status=%d, actual interval ms=%u]",
                __FUNCTION__, lceStatus, actualIntervalMs);
        response->lce_status = (char) lceStatus;
        response->actual_interval_ms = actualIntervalMs;
    }

    nRet = 1;
    AT_RSP_FREE(p_response);
    LOGV("[%s] finished X", __FUNCTION__);
    return nRet;

error:
    AT_RSP_FREE(p_response);
    LOGE("[%s] parsing error X", __FUNCTION__);
    return nRet;
}

void requestSetLinkCapacityReportingCriteria(void* data, size_t datalen, RIL_Token t) {
    UNUSED(data);
    UNUSED(datalen);

    // New version of LCE service in legacy modem is not ready yet.
    RIL_onRequestComplete(t, RIL_E_LCE_NOT_SUPPORTED, NULL, 0);
}
// LCE service end


// Keep alive start
void requestStartKeepalive(void* data, size_t datalen, RIL_Token t) {
    RIL_KeepaliveRequest *pRequest;
    UNUSED(datalen);
    RIL_KeepaliveStatus* response = (RIL_KeepaliveStatus*) calloc(1, sizeof(RIL_KeepaliveStatus));
    if (response == NULL) {
        LOGE("[%s] calloc for response failed!", __FUNCTION__);
        goto error;
    }

    char *cmd = NULL;
    char *line = NULL;
    ATResponse *p_response = NULL;
    int ret = 0;
    int sessionHandle = -1;
    int code = -1;  // ACTIVE(0): MD can do keep-alive for this session

    pRequest = (RIL_KeepaliveRequest*)data;

    // Currently we only support UDP
    int keepAliveType = (pRequest->type == NATT_IPV4) ? IPV4_UDP : IPV6_UDP;
    const char* sourceAddress = pRequest->sourceAddress;
    int sourcePort = pRequest->sourcePort;
    const char* destinationAddress = pRequest->destinationAddress;
    int destinationPort = pRequest->destinationPort;
    int maxKeepaliveIntervalMillis = pRequest->maxKeepaliveIntervalMillis;
    int interfaceId = pRequest->cid;
    int cid = INVALID_CID;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));

    LOGD("[%s] keepAliveType=%d, sourceAddress=%s, sourcePort=%d, destinationAddress=%s \
            destinationPort=%d,maxKeepaliveIntervalMillis=%d, Request.cid=%d",
            __FUNCTION__, keepAliveType, sourceAddress, sourcePort, destinationAddress,
            destinationPort,maxKeepaliveIntervalMillis, interfaceId);

    // For Legacy MD, mCid in DataConnection.java is interface id (ccmni id) of rild.
    // So we need get cid from interface id.
    for(int i = 0; i < pdnInfoSize; i++) {
        if (pdn_info[rid][i].interfaceId == interfaceId
                && !pdn_info[rid][i].isDedicateBearer) {
            PdnInfo* pPdnInfo = &pdn_info[rid][i];
            cid = pPdnInfo->cid;
            LOGD("[%s] get cid %d by interfaceId %d", __FUNCTION__, cid, interfaceId);
            break;
        }
    }

    // Add check cid for VTS case
    if (cid < 0 || cid > pdnInfoSize) {
        LOGE("[%s] cid %d error!", __FUNCTION__, cid);
        RIL_onRequestComplete(t, RIL_E_INVALID_ARGUMENTS, NULL, 0);
        goto finish;
    }

    asprintf(&cmd, "AT+EKALIVE=1,%d,%d,\"%s\",\"%s\",\"%d\",\"%d\",%d", keepAliveType,
            cid, sourceAddress, destinationAddress, sourcePort, destinationPort,
            maxKeepaliveIntervalMillis/1000);

    ret = at_send_command_singleline(cmd, "+EKALIVE:", &p_response, DATA_CHANNEL_CTX);
    free(cmd);

    if (ret != 0 || p_response == NULL || isATCmdRspErr(ret, p_response)) {
        LOGE("[%s] got error response", __FUNCTION__);
        goto error;
    }

    line = p_response->p_intermediates->line;

    ret = at_tok_start(&line);
    if (ret < 0) goto error;

    ret = at_tok_nextint(&line, &sessionHandle);
    if (ret < 0) goto error;
    response->sessionHandle = sessionHandle;

    ret = at_tok_nextint(&line, &code);
    if (ret < 0) goto error;
    response->code = code;

    RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(RIL_KeepaliveStatus));
    goto finish;

error:
    LOGE("[%s] error", __FUNCTION__);
    // For DBG
    //response->sessionHandle = 100;
    //response->code = KEEPALIVE_ACTIVE;
    //RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(RIL_KeepaliveStatus));
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);

finish:
    FREEIF(response);
    AT_RSP_FREE(p_response);
    LOGV("[%s] finish", __FUNCTION__);
}

void requestStopKeepalive(void* data, size_t datalen, RIL_Token t) {
    UNUSED(datalen);

    ATResponse *p_response = NULL;
    int ret = 0;
    char cmdLine[PROPERTY_VALUE_MAX] = {0};
    memset(cmdLine, 0, sizeof(cmdLine));
    int sessionHandle = ((int *)data)[0];
    LOGD("[%s] sessionHandle=%d", __FUNCTION__, sessionHandle);

    // Add check cid for VTS case
    if (sessionHandle < 0 || sessionHandle > pdnInfoSize) {
        LOGE("[%s] sessionHandle %d over pdn Size: %d!", __FUNCTION__, sessionHandle, pdnInfoSize);
        RIL_onRequestComplete(t, RIL_E_INVALID_ARGUMENTS, NULL, 0);
        return;
    }

    snprintf(cmdLine, sizeof(cmdLine), "AT+EKALIVE=0,%d", sessionHandle);

    ret = at_send_command(cmdLine, &p_response, DATA_CHANNEL_CTX);

    if (p_response == NULL || isATCmdRspErr(ret, p_response)) {
        goto error;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    AT_RSP_FREE(p_response);
    return;

error:
    // for DBG
    //RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    RIL_onRequestComplete(t, RIL_E_MODEM_ERR, NULL, 0);
    AT_RSP_FREE(p_response);
}

void onUpdateKeepaliveStatus(void* param) {
    //+EKALIVE: <conn_id>,<md_status>
    TimeCallbackParam* timeCallbackParam = (TimeCallbackParam*)param;
    char* urc = (char*)(timeCallbackParam->urc);
    RIL_SOCKET_ID rid = timeCallbackParam->rid;
    int ret;
    int sessionHandle = -1;
    int code = -1;  // ACTIVE(0): MD can do keep-alive for this session

    LOGD("[%d][%s] URC : %s", rid, __FUNCTION__, urc);

    RIL_KeepaliveStatus* keepaliveStatus = NULL;
    keepaliveStatus = (RIL_KeepaliveStatus *) calloc(1, sizeof(RIL_KeepaliveStatus));
    if (keepaliveStatus == NULL) {
        LOGE("[%s] calloc failed!", __FUNCTION__);
        goto error;
    }

    ret = at_tok_start(&urc);
    if (ret < 0) goto error;

    ret = at_tok_nextint(&urc, &sessionHandle);
    if (ret < 0) goto error;

    ret = at_tok_nextint(&urc, &code);
    if (ret < 0) goto error;

    keepaliveStatus->sessionHandle = (uint32_t)sessionHandle;
    keepaliveStatus->code = (RIL_KeepaliveStatusCode)code;

    LOGD("[%d][%s] sessionHandle: %d, code: %d", rid, __FUNCTION__,
            keepaliveStatus->sessionHandle, keepaliveStatus->code);

    RIL_UNSOL_RESPONSE(RIL_UNSOL_KEEPALIVE_STATUS, keepaliveStatus,
            sizeof(RIL_KeepaliveStatus), rid);
    goto finish;

error:
    LOGE("[%d][%s] parsing error", rid, __FUNCTION__);

finish:
    LOGV("[%d][%s] finish", rid, __FUNCTION__);
    FREEIF(timeCallbackParam->urc);
    FREEIF(timeCallbackParam);
    FREEIF(keepaliveStatus);
}

// Keep alive end

/// M: Ims Data Framework {@
bool deactivateDedicateDataCall(int cid, RILChannelCtx* rilchnlctx) {
    char *cmd;
    int err;
    ATResponse *p_response = NULL;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(rilchnlctx);

    if (pdn_info[rid][cid].isDedicateBearer) {
        asprintf(&cmd, "AT+CGACT=0,%d", cid);
        err = at_send_command(cmd, &p_response, rilchnlctx);
        free(cmd);
        if (isATCmdRspErr(err, p_response)) {
            if (p_response->success == 0) {
                int error = at_get_cme_error(p_response);
                if (error == CME_L4C_CONTEXT_CONFLICT_DEACT_ALREADY_DEACTIVATED) {
                    //L4C error: L4C_CONTEXT_CONFLICT_DEACT_ALREADY_DEACTIVATED(0x1009)
                    LOGI("requestDeactivateDedicateDataCall cid%d already deactivated", cid);
                }
            } else {
                LOGE("requestDeactivateDedicateDataCall cid%d AT+CGACT response fail", cid);
                goto error;
            }
        }
        clearPdnInfo(&pdn_info[rid][cid]);
        AT_RSP_FREE(p_response);
        return true;
    } else {
        LOGE("requestDeactivateDedicateDataCall CID%d is not a dedicate bearer", cid);
        goto error;
    }

error:
    AT_RSP_FREE(p_response);
    return false;
}
///@}

int handleLastPdnDeactivation (const int isEmergency, const int lastPDNCid, RILChannelCtx* rilchnlctx)
{
    int ret = 0;
    int err;
    ATResponse *p_response = NULL;
    char *cmd = NULL;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(rilchnlctx);

    LOGD("[RILData_GSM] handleLastPdnDeactivation to handle the last PDN deactivation, isEmergency:%d", isEmergency);
    if(isEmergency) { //Last PDN is emergency PDN, detach frist then re-attach again
        /// For ALPS03276636: @{
        //  When AP sent "AT+CGATT=0", RAC sent DETACH_REQ to EMM, but with a flag "is_user_ps_reattach" as false,
        //  this will caused NWSEL to search on 2/3G network, and UE will move to 3G.
        //  So we need with flag "is_user_ps_reattach" as true, ap hand to send "AT+EGREA=1" before send "AT+CGATT=0";
        //when AT+EGREA=1 is set, EUTRAN would not be disabled after PS detached
        at_send_command("AT+EGREA=1", NULL, rilchnlctx);
        LOGD("[RILData_GSM] [%s] AT+EGREA=1 sent..", __FUNCTION__);
        err = at_send_command("AT+CGATT=0", &p_response, rilchnlctx);
        //when AT+EGREA=0 is set, EUTRAN would be disabled after PS detached
        at_send_command("AT+EGREA=0", NULL, rilchnlctx);
        LOGD("[RILData_GSM] [%s] AT+EGREA=0 sent..", __FUNCTION__);
        //  @}
        if (isATCmdRspErr(err, p_response)) {
            LOGE("handleLastPdnDeactivation detach response ERROR, (still do reattach...)");
            //we do not exit here since we expect the network interface should be set to down
            //goto error; //Currently, ignore this error, still do reattach from modem member suggestion
        }
        AT_RSP_FREE(p_response);

        err = at_send_command("AT+CGATT=1", &p_response, rilchnlctx);
        if (isATCmdRspErr(err, p_response)) {
            LOGE("handleLastPdnDeactivation re-attach response ERROR");
            //goto error;  //Currently, ignore this error, from modem member suggestion
        }
    } else {  // General last PDN
        /// M: Ims Data Framework {@
        // M: ALPS03116943 Deactivate all dedicated PDN connection for last PDN @{
        int i = 0;
        for (i = 0; i < pdnInfoSize; i++) {
            if (pdn_info[rid][i].isDedicateBearer && pdn_info[rid][i].primaryCid == lastPDNCid) {
                LOGD("[%d]deactivate dedicate pdn CID%d info due to default pdn CID%d linkdown",
                        rid, pdn_info[rid][i].cid, lastPDNCid);
                if (deactivateDedicateDataCall(pdn_info[rid][i].cid, rilchnlctx)) {
                    RIL_UNSOL_RESPONSE(RIL_LOCAL_GSM_UNSOL_DEDICATE_BEARER_DEACTIVATED, &i, sizeof(int), rid);
                }
            }
        }
        /// @}
        asprintf(&cmd, "AT+EGLD=%d", lastPDNCid);
        err = at_send_command(cmd, &p_response, rilchnlctx);
        free(cmd);
        if (isATCmdRspErr(err, p_response)) {
            LOGE("handleLastPdnDeactivation link down ERROR");
            goto error;
        }
    }

    AT_RSP_FREE(p_response);
    return ret;

error:
    ret = -1;
    AT_RSP_FREE(p_response);
    return ret;
}

int deactivateDataCall(int cid, RILChannelCtx* rilchnlctx)
{
    int ret = CME_SUCCESS;
    ATResponse *p_response = NULL;
    int err = 0;
    char *cmd = NULL;

    if (cid < 0 || cid > pdnInfoSize) {
        LOGE("[RILData_GSM] [%s] cid: %d, index over boundry (%d)", __FUNCTION__, cid, max_pdn_support);
        ret = CME_ERROR_NON_CME;
        goto error;
    }

    asprintf(&cmd, "AT+CGACT=0,%d", cid);
    err = at_send_command(cmd, &p_response, rilchnlctx);
    free(cmd);
    if (isATCmdRspErr(err, p_response)) {
        if (p_response != NULL && p_response->success == 0) {
            ret = at_get_cme_error(p_response);
        } else {
            LOGD("[RILData_GSM] [%s] cid%d AT+CGACT response fail", __FUNCTION__, cid);
            ret = CME_UNKNOWN;
        }
    }

error:
    AT_RSP_FREE(p_response);
    if (ret != CME_SUCCESS || err != 0) {
        LOGI("[RILData_GSM] [%s] ret: %d, err: %d", __FUNCTION__, ret, err);
    }
    return ret;
}

int deactivateDataCallByReason(int cid, int reason, RILChannelCtx* rilchnlctx) {
    int ret = CME_SUCCESS;
    ATResponse *p_response = NULL;
    int err = 0;
    char *cmd = NULL;

    if (cid < 0 || cid > pdnInfoSize) {
        LOGD("[RILData_GSM] [%s] cid: %d, index over boundry (%d)", __FUNCTION__,
                cid, max_pdn_support);
        ret = CME_ERROR_NON_CME;
        goto finish;
    }

    // AT+EGACT=<state>,<cid>,<rat>,<reason>
    asprintf(&cmd, "AT+EGACT=0,%d,0,%d", cid, reason);
    err = at_send_command(cmd, &p_response, rilchnlctx);
    free(cmd);
    if (isATCmdRspErr(err, p_response)) {
        if (p_response != NULL && p_response->success == 0) {
            ret = at_get_cme_error(p_response);
        } else {
            LOGD("[RILData_GSM] [%s] cid:%d, reason: %d, AT+EGACT response fail",
                    __FUNCTION__, cid, reason);
            ret = CME_UNKNOWN;
        }
    }

finish:
    AT_RSP_FREE(p_response);
    if (ret != CME_SUCCESS || err != 0) {
        LOGI("[RILData_GSM] [%s] ret: %d, err: %d", __FUNCTION__, ret, err);
    }
    return ret;
}

void requestClearDataBearer(void *data, size_t datalen, RIL_Token t)
{
    UNUSED(data);
    UNUSED(datalen);
    int i = 0, lastPdnCid;
    int needHandleLastPdn = 0;
    int lastPdnState = DATA_STATE_INACTIVE;
    int isEmergency = 0;
    int isSignaling = 0;
    int err;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));

    LOGV("[%d]requestClearDataBearer", rid);
    /* Because the RIL_Token* t may be NULL passed due to receive URC: Only t is NULL, 2nd parameter rilid is used */
    RILChannelCtx* rilchnlctx = NULL;
    if (t != NULL)
        rilchnlctx = getRILChannelCtxFromToken(t);
    else {
        rilchnlctx = getChannelCtxbyProxy();
    }

    //  Query Active Status //if any IMS activated , try to clear
    if (updateActiveStatus(rilchnlctx)) {
        for (; i < pdnInfoSize; i++) {
            if (pdn_info[rid][i].cid != INVALID_CID
                && pdn_info[rid][i].active != DATA_STATE_INACTIVE
                && !pdn_info[rid][i].isDedicateBearer
                && pdn_info[rid][i].signalingFlag) {
                err = deactivateDataCall(pdn_info[i][rid].cid, rilchnlctx);
                switch (err) {
                case CME_SUCCESS:
                    break;
                case CME_L4C_CONTEXT_CONFLICT_DEACT_ALREADY_DEACTIVATED:
                    LOGD("[%s] deactivateDataCall cid:%d already deactivated", __FUNCTION__, i);
                    break;
                case CME_LAST_PDN_NOT_ALLOW_LR11:
                    if(pdnFailCauseSupportVer < MD_LR11) goto error;
                case CME_LAST_PDN_NOT_ALLOW:
                    if (needHandleLastPdn == 0) {
                        needHandleLastPdn = 1;
                        lastPdnState = pdn_info[rid][i].active;
                        isEmergency = pdn_info[rid][i].isEmergency;
                        isSignaling = pdn_info[rid][i].signalingFlag;
                        lastPdnCid = pdn_info[rid][i].cid;
                        LOGI("[%s] deactivateDataCall cid:%d is the last PDN, state: %d", __FUNCTION__, i, lastPdnState);
                    }
                    break;
                case CME_ERROR_NON_CME:
                    LOGD("[%s] ignore this, cid:%d, error: %d", __FUNCTION__, i, err);
                    break;
                default:
                    LOGE("[%s] deactivateDataCall cid:%d failed, error:%d", __FUNCTION__, i, err);
                    goto error;
                };

                configureNetworkInterface(pdn_info[rid][i].interfaceId, DISABLE_CCMNI, rid);
                clearPdnInfo(&pdn_info[rid][i]);
            }
        }

        // handle last PDN issue
        if (needHandleLastPdn) {
            if (lastPdnState != DATA_STATE_LINKDOWN) {
                if(0 != handleLastPdnDeactivation(isEmergency, lastPdnCid, rilchnlctx)) {
                    goto error;
                }
            } else {
                LOGD("[%s] last pdn alread linkdown", __FUNCTION__);
            }
            pdn_info[rid][lastPdnCid].active = DATA_STATE_LINKDOWN;
            pdn_info[rid][lastPdnCid].cid = lastPdnCid;
            pdn_info[rid][lastPdnCid].primaryCid = lastPdnCid;
            pdn_info[rid][lastPdnCid].signalingFlag = isSignaling;
        }
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;

error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}


void onNetworkBearerUpdate(void* param)
{
    //this method is used for parsing
    //+CGEV: NW ACT <p_cid>, <cid>, <event_type>
    //+CGEV: NW MODIFY <cid>, <change_reason>, <event_type>
    TimeCallbackParam* timeCallbackParam = (TimeCallbackParam*)param;
    char* urc = (char*)(timeCallbackParam->urc);
    RIL_SOCKET_ID rid = timeCallbackParam->rid;
    int isModification = timeCallbackParam->isModification;
    RILChannelCtx* pChannel = getChannelCtxbyProxy();
    LOGI("onNetworkBearerUpdate [rid=%d, modify=%d, urc=%s]", rid, isModification, urc);

    int err = 0;
    char *line = NULL;

    err = at_tok_start(&urc);
    if (err < 0) {
        LOGE("onNetworkBearerUpdate token start fail");
        goto error;
    }

    LOGV("onNetworkBearerUpdate token start [%s]", urc);

    if (isModification) {
        urc += 11; //skip prefix " NW MODIFY "
    } else {
        urc += 8; //skip prefix " NW ACT "
    }
    LOGV("onNetworkBearerUpdate skip prefix [%s]", urc);

    int cid = INVALID_CID;
    int defaultCid = INVALID_CID;
    int reason = 0;

    err = at_tok_nextstr(&urc, &line);
    if (err < 0) {
        LOGE("onNetworkBearerUpdate ERROR occurs when parsing defaultCid");
        goto error;
    }

    if (isModification) {
        cid = atoi(line);
    } else {
        defaultCid = atoi(line);
    }
    err = at_tok_nextstr(&urc, &line);
    if (err < 0) {
        LOGE("onNetworkBearerUpdate ERROR occurs when parsing CID");
        goto error;
    }

    if (isModification) {
        reason = atoi(line);
    } else {
        cid = atoi(line);
    }

    pdn_info[rid][cid].cid = cid;
    if (isModification) {
        defaultCid = pdn_info[rid][cid].primaryCid;
    } else {
        pdn_info[rid][cid].primaryCid = defaultCid;
    }
    if (defaultCid == cid) {
        pdn_info[rid][cid].isDedicateBearer = 0;
        if (!isModification && pdn_info[rid][cid].active == DATA_STATE_INACTIVE) {
            LOGD("onNetworkBearerUpdate a default bearer is activated [cid=%d]", cid);
            pdn_info[rid][cid].active = DATA_STATE_LINKDOWN; //for default bearer, set to linkdown state
        } else {
            LOGD("onNetworkBearerUpdate not to update active status [cid=%d, active=%d]",
                    cid, pdn_info[rid][cid].active);
        }
    } else if (INVALID_CID != defaultCid) {
        if (isModification) {
            LOGD("onNetworkBearerUpdate a dedicate bearer is modified [cid=%d]", cid);
        } else {
            LOGD("onNetworkBearerUpdate a dedicate bearer is activated [cid=%d]", cid);
        }
        pdn_info[rid][cid].isDedicateBearer = 1;
        pdn_info[rid][cid].interfaceId = pdn_info[rid][defaultCid].interfaceId;
        pdn_info[rid][cid].active = pdn_info[rid][defaultCid].active;
    }

    updateDynamicParameter(pChannel);

    //the notification would be sent only if the activated bearer is a dedicate bearer
    //while default bearer would be set to linkdown state
    if (pdn_info[rid][cid].primaryCid != INVALID_CID) {  // avoid access violation
        LOGD("onNetworkBearerUpdate cid=%d, primaryCid=%d, active=%d", cid,
                pdn_info[rid][cid].primaryCid, pdn_info[rid][pdn_info[rid][cid].primaryCid].active);
    } else {
        LOGD("onNetworkBearerUpdate cid=%d, primaryCid=%d", cid, pdn_info[rid][cid].primaryCid);
    }
    if (DATA_STATE_INACTIVE == isCidActive(&cid, 1, pChannel)) {
        goto error;
    }

    if (INVALID_CID != pdn_info[rid][cid].primaryCid
            && pdn_info[rid][pdn_info[rid][cid].primaryCid].active == DATA_STATE_ACTIVE) {
        if (pdn_info[rid][cid].isDedicateBearer) {
            MALRIL_Dedicate_Data_Call_Struct* response = alloca(sizeof(MALRIL_Dedicate_Data_Call_Struct));
            initialDedicateDataCallResponse(response, 1);
            createDedicateDataResponse(cid, response, rid);
            dumpDedicateDataResponse(response, "onNetworkBearerUpdate dump response");
            int sim_id = getValidSimId(rid, __FUNCTION__);

            if (isModification) {
                mal_datamngr_notify_dedicated_bearer_modify_rsp_ptr_t rsp_ptr =
                        calloc(1, sizeof(mal_datamngr_notify_dedicated_bearer_modify_rsp_t));
                MAL_Dedicate_Data_Call_Struct *req_ptr =
                    (MAL_Dedicate_Data_Call_Struct *) response;
                if (sim_id != SIM_ID_INVALID) {
                    if (mal_datamngr_notify_dedicated_bearer_modify_tlv != NULL) {
                        int ret = mal_datamngr_notify_dedicated_bearer_modify_tlv (mal_once(1, mal_cfg_type_sim_id, sim_id),
                                req_ptr, rsp_ptr, 0, NULL, NULL, NULL);
                        LOGD("[%s] Call mal_datamngr_notify_dedicated_bearer_modify_tlv success", __FUNCTION__);
                    } else {
                        LOGE("[%s] mal_datamngr_notify_dedicated_bearer_modify_tlv is null", __FUNCTION__);
                    }
                }
                FREEIF(rsp_ptr);
                RIL_UNSOL_RESPONSE(RIL_UNSOL_DEDICATE_BEARER_MODIFIED,
                        response, sizeof(RIL_Dedicate_Data_Call_Struct), rid);
            }

            else {
                mal_datamngr_notify_dedicated_bearer_act_rsp_ptr_t rsp_ptr =
                        calloc(1, sizeof(mal_datamngr_notify_dedicated_bearer_act_rsp_t));
                MAL_Dedicate_Data_Call_Struct *req_ptr =
                    (MAL_Dedicate_Data_Call_Struct *) response;
                if (sim_id != SIM_ID_INVALID) {
                    if (mal_datamngr_notify_dedicated_bearer_act_tlv != NULL) {
                        int ret = mal_datamngr_notify_dedicated_bearer_act_tlv (mal_once(1, mal_cfg_type_sim_id, sim_id),
                                req_ptr, rsp_ptr, 0, NULL, NULL, NULL);
                        LOGD("[%s] Call mal_datamngr_notify_dedicated_bearer_act_tlv success", __FUNCTION__);
                    } else {
                        LOGE("[%s] mal_datamngr_notify_dedicated_bearer_act_tlv is null", __FUNCTION__);
                    }
                }
                FREEIF(rsp_ptr);
                RIL_UNSOL_RESPONSE(RIL_UNSOL_DEDICATE_BEARER_ACTIVATED,
                        response, sizeof(RIL_Dedicate_Data_Call_Struct), rid);
            }
        } else {
            if (pdn_info[rid][cid].active == DATA_STATE_ACTIVE) {
                onDataCallListChanged(&s_data_ril_cntx[rid]);
                LOGD("udpate default bearer thru DataCallListChanged");
            } else {
                LOGD("onNetworkBearerUpdate not send URC due to default bearer is not in active state");
            }
        }
    } else {
        LOGE("onNetworkBearerUpdate default bearer is not activated, not to send URC of dedicate bearer activation");
    }

    //we have to free these here since the param->urc and param is allocated in rilDataUnsolicited in ril_data
    //(case "+CGEV: NW ACT" and "+CGEV: NW MODIFY')
    free(timeCallbackParam->urc);
    free(timeCallbackParam);
    return;

error:
    free(timeCallbackParam->urc);
    free(timeCallbackParam);
}
/// @}

void onPdnDeact(void* param) {
    TimeCallbackParam* timeCallbackParam = (TimeCallbackParam*)param;
    char* urc = (char*)(timeCallbackParam->urc);
    RIL_SOCKET_ID rid = timeCallbackParam->rid;

    onPdnDeactResult(urc, rid);

    free(timeCallbackParam->urc);
    free(timeCallbackParam);
}

void onPdnDeactResult(char* urc, RIL_SOCKET_ID rid) {
    int err = 0;
    int primaryCid = INVALID_CID;
    int cid = INVALID_CID;
    int r11FormatCheck = 0;
    int isNeedGetPrimary = 1;
    char *line = NULL;
    int isDedicateBearer = 0;
    bool bMeDeact = false;
    bool bPdnHoStart = false;
    RILChannelCtx* rilchnlctx = getChannelCtxbyProxy();

    //+CGEV: NW DEACT <PDP_type>, <PDP_addr>, [<cid>]
    //+CGEV: NW REACT <PDP_type>, <PDP_addr>, [<cid>]
    //R12: +CGEV: NW PDN DEACT <cid>
    //R12: +CGEV: ME PDN DEACT <cid>
    //R12: +CGEV: NW DEACT <p_cid>, <cid>, <event_type>   this two for VoLTE
    //R12: +CGEV: ME DEACT <p_cid>, <cid>, <event_type>

    LOGD("[%d]onPdnDeactResult receive %s", rid, urc);
    err = at_tok_start(&urc);
    if (err < 0)
        goto error;

    if (strlen(urc) > 0) {
        LOGV("[%d]onPdnDeactResult token start [%s]", rid, urc);

        if (strncmp(NW_PDN_DEACT, urc, strlen(NW_PDN_DEACT)) == 0) {
            urc += strlen(NW_PDN_DEACT);
        } else if (strncmp(ME_PDN_DEACT, urc, strlen(ME_PDN_DEACT)) == 0) {
            urc += strlen(ME_PDN_DEACT);
            bMeDeact = true;
        } else if (strncmp(NW_DEACT, urc, strlen(NW_DEACT)) == 0) {
            urc += strlen(NW_DEACT);
            r11FormatCheck = 1;
        } else if (strncmp(ME_DEACT, urc, strlen(ME_DEACT)) == 0) {
            urc += strlen(ME_DEACT);
            r11FormatCheck = 1;
            bMeDeact = true;
        } else if (strncmp(NW_REACT, urc, strlen(NW_REACT)) == 0) {
            urc += strlen(NW_REACT);
            r11FormatCheck = 1;
        }

        LOGV("[%d]onPdnDeactResult skip prefix [%s]", rid, urc);

        if (r11FormatCheck != 0) {
            LOGV("[%d]onPdnDeactResult skip old format: +CGEV: NW DEACT <PDP_type>, <PDP_addr>, [<cid>] ", rid);
            //Skip IP type
            err = at_tok_nextstr(&urc, &line); //PDP_type
            if (err < 0) {
                LOGE("[%d]ERROR occurs when parsing PDP type", rid);
                goto error;
            }

            do {
                // check first digit is 0 or not for distinguish from previous
                // md version urc or not of "NW/ME DEACT"
                if (strlen(line) > 0 && isdigit(line[0])) {
                    primaryCid = atoi(line);
                    isNeedGetPrimary = 0;
                    LOGV("[%d]not r11", rid);
                    break;
                }

                if (!at_tok_hasmore(&urc)) {
                    LOGE("[%d]ERROR occurs", rid);
                    goto error;
                }

                //Skip IP address
                err = at_tok_nextstr(&urc, &line); //PDP_addr
                if (err < 0) {
                    LOGE("[%d]ERROR occurs when parsing ip address", rid);
                    goto error;
                }

                if (!at_tok_hasmore(&urc)) {
                    LOGE("[%d]ERROR occurs", rid);
                    goto error;
                }
            } while(0);
        }

        if (isNeedGetPrimary) {
            err = at_tok_nextint(&urc, &primaryCid); //cid or primary cid
            if (err < 0) {
                LOGE("[%d]ERROR occurs when parsing first parameter", rid);
                goto error;
            }
        }

        if (at_tok_hasmore(&urc)) {
            err = at_tok_nextint(&urc, &cid); //cid
            if (err < 0) {
                LOGE("[%d]ERROR occurs when parsing cid", rid);
                goto error;
            }
        } else {
            cid = primaryCid;
        }
        LOGD("[%d]onPdnDeactResult primaryCid=%d, cid=%d", rid, primaryCid, cid);

        // Ignore PDN deact message report from source RAT during Irat except dedicated bearer
        if (nIratAction == IRAT_ACTION_SOURCE_STARTED ||
                nIratAction == IRAT_ACTION_SOURCE_FINISHED) {
            if (primaryCid != cid) {
                notifyDedicatedBearerDeact(rid, cid);
            }
            LOGI("[%d][RILData_GSM_URC] Ignore source PDN deact during IRAT: status = %d, urc = %s.",
                    rid, nIratAction, urc);
            onUpdateIratStatus(rid);
            return;
        }

        int interfaceId = pdn_info[rid][cid].interfaceId;

        int i = 0;
        int hasSameInterfaceCid = 0;

        for (i = 0; i < pdnInfoSize; i++) {
            if (!pdn_info[rid][i].isDedicateBearer
                    && pdn_info[rid][i].interfaceId != INVALID_CID
                    && pdn_info[rid][i].interfaceId == interfaceId
                    && pdn_info[rid][i].primaryCid!= cid)
            {
                LOGD("[%d]CID%d has the same interfaceId with CID%d", rid, i, cid);
                hasSameInterfaceCid = 1;
                break;
            }
        }
        /// M: Ims Data Framework {@
        bPdnHoStart = isPdnHoStartFromRds(rid, &pdn_info[rid][cid]);

        if (!hasSameInterfaceCid && !bMeDeact) {
            LOGI("[%d]Deactivate PDN CID/interfaceId/hoStart/eran is [%d, %d, %d, %d]",
                    rid, cid, interfaceId, bPdnHoStart, pdn_info[rid][cid].rat);
            // pdn_info[rid][cid].deactbyNw = 1;
            if (!bPdnHoStart && pdn_info[rid][cid].rat != RDS_RAN_WIFI) {
                deactivatePdnThruEpdg(rid, interfaceId, 0);

                configureNetworkInterface(interfaceId, DISABLE_CCMNI, rid);

                // M: for sync AP/MD, needs to unbind interfaceId with cid to MD when process PDN deactivation.
                char *cmd = NULL;
                ATResponse *p_response = NULL;
                LOGD("[%d]onPdnDeactResult needs to unbind interfaceId=%d when PDN cid=%d is activated",
                        rid, interfaceId, cid);
                asprintf(&cmd, "AT+EGLD=%d", cid);
                err = at_send_command(cmd, &p_response, rilchnlctx);
                free(cmd);
                if (isATCmdRspErr(err, p_response)) {
                    LOGE("[%d]onPdnDeactResult link down ERROR", rid);
                }
                AT_RSP_FREE(p_response);
            }
        }

        int sim_id = getValidSimId(rid, __FUNCTION__);
        for (i = 0; i < pdnInfoSize; i++) {
            if(pdn_info[rid][i].isDedicateBearer && pdn_info[rid][i].primaryCid == cid) {
                LOGD("[%d]clean dedicate pdn CID%d info due to default pdn CID%d deactivated",
                        rid, pdn_info[rid][i].cid, cid);
                clearPdnInfo(&pdn_info[rid][i]);
                notifyDedicatedBearerDeact(rid, i);
            }
        }

        if (pdn_info[rid][cid].isDedicateBearer) {
            LOGD("[%d]CID%d is a dedicate bearer, send deactivated URC", rid, cid);
            isDedicateBearer = 1;
            notifyDedicatedBearerDeact(rid, cid);
        }

        /// M: For ALPS03020916 Add "pdn_info[rid][cid].interfaceId != INVALID_CID"
        ///    If the interfaceId is invalid, we shouldn't change the pendingRemoveLtePdn,
        ///    which will indicate should remove the lte pdn info when handover.
        if (pdn_info[rid][cid].interfaceId != INVALID_CID
                && bPdnHoStart && pdn_info[rid][cid].rat != RDS_RAN_WIFI) {
            // Remove LTE PDN when handover finished.
            g_ho_pdn_remove_pending_act[rid].needRemove = true;
            g_ho_pdn_remove_pending_act[rid].cid = cid;
        }

        if (!bMeDeact) {
            if (!bPdnHoStart) {
                if (pdn_info[rid][cid].rat != RDS_RAN_WIFI) {
                    clearPdnInfo(&pdn_info[rid][cid]);
                    LOGD("clear lte pdn_info[%d][%d]", rid, cid);
                } else {
                    // M: ALPS02800653 When handover from LTE -> WIFI,
                    //    we mark the eran type of LTE PDN as WIFI,
                    //    and we clear the LTE PDN after handover & NW DEACT comes.
                    clearPdnInfo(&pdn_info[rid][cid]);
                    pdn_info[rid][cid].rat = RDS_RAN_WIFI;
                    LOGD("clear lte pdn_info[%d][%d] after L->W", rid, cid);
                }
            }
        }
        /// @}
    }

    if (s_md_off) {
        LOGD("[%d]rilDataUnsolicited(): modem off!", rid);
        requestOrSendDataCallListIpv6(getChannelCtxbyProxy(), NULL, rid);
    /// M: Ims Data Framework {@
    } else if (!isDedicateBearer) {
        if (!bPdnHoStart && pdn_info[rid][cid].rat != RDS_RAN_WIFI) {
            // query data calllist only if the bearer is a default bearer
            requestOrSendDataCallListIpv6(getChannelCtxbyProxy(), NULL, rid);
        } else {
            if (bPdnHoStart && mHandOverDir[rid] == E_HO_DIR_WIFI_TO_LTE) {
                updateActiveStatus(rilchnlctx);
                if (pdn_info[rid][cid].active != DATA_STATE_ACTIVE) {
                    clearPdnInfo(&pdn_info[rid][cid]);
                    LOGD("clear lte pdn for cid %d after previous W -> L success", cid);
                }
            }
            LOGD("[%d][%s] not send data call list for L to W handover", rid, __FUNCTION__);
        }
    }
    /// @}

error:
    return;
}

void onSetAttachApn(void* param)
{
    char* cmd = NULL;
    RILChannelCtx* pChannel = getChannelCtxbyProxy();
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(pChannel);
    LOGD("[%d]onSetAttachApn, to set IA apn again", rid);

    char* iaProperty = alloca(PROPERTY_VALUE_MAX*2);
    memset(iaProperty, 0, PROPERTY_VALUE_MAX*2);

    int iaSimId = rid;

    getIaCache(iaSimId, iaProperty);
    if (strlen(iaProperty) > 0) {
        defineAttachApnIfIACacheExisted(rid, pChannel);
    } else {
        LOGD("[%d]onSetAttachApn, but no IA cache, do nothing", rid);
    }
}

void configPcoParam(config_pco_param_t *param) {
    char *cmd = NULL;
    int err = 0;
    int len = 0;
    char *temp_cmd = NULL;
    char *temp = NULL;
    int pcscf_reselection = 0;

    int isIaApn = isInitialAttachApn(param->requestedApn,
            //param->pszIPStr, param->authType, param->username, param->password,
            param->pChannel, param->cid);

    // AT+CGPRCO=<cid>,<user_name>,<passwd>,<DNS1>,<DNS2>,<authType>,<req_v6dns>,<req_v4dns>
    // AT+CGPRCO = <cid>, <username>, <passwd>, <dns1>, <dns2> <auth_type>,<dnsv6_request>
    // ,<dnsv4_request>,<isIA>,<mtu_request>,<op-PCO_code>,<op-PCO_content>,<MSISDN>
    if (AUTHTYPE_NOT_SET == param->authType) {
        len = asprintf(&cmd, "AT+CGPRCO=%d,\"\",\"\",\"\",\"\",%d,1,1,%d,%d", param->cid
                , AUTHTYPE_NONE, isIaApn, param->queryMtu);
    } else {
        len = asprintf(&cmd, "AT+CGPRCO=%d,\"%s\",\"%s\",\"\",\"\",%d,1,1,%d,%d", param->cid
                , param->username, param->password, param->authType, isIaApn, param->queryMtu);
    }

    if (isOp12Support()) {
        // For VzW, set MSISDN flag for initial attach APN
        if((NULL != param->requestedApn) &&
                (NULL != strcasestr(param->requestedApn, "ims"))) {
            pcscf_reselection = 1;
        }
        asprintf(&temp_cmd, ",\"%s\",\"%s\",%d,%d",
            OP12_PCO_ID, OP12_PLMN, isIaApn, pcscf_reselection);
        if (temp_cmd == NULL) {
            LOGE("[%s] allocate temp_cmd err", __FUNCTION__);
            goto error;
        }
        len = len + strlen(temp_cmd);
        temp = (char*) calloc(len + 1, sizeof(char));
        if (temp == NULL) {
            LOGE("[%s] allocate temp err", __FUNCTION__);
            goto error;
        }
        strncpy(temp, cmd, len);
        strncat(temp, temp_cmd, strlen(temp_cmd));
        free(cmd);
        free(temp_cmd);
        cmd = temp;
    }
    err = at_send_command(cmd, NULL, param->pChannel);
    FREEIF(cmd);
    return;
error:
    FREEIF(cmd);
    return;
}

void parseAndSendPcoStatus(char* line, RIL_SOCKET_ID rid, int cid, char* apnname) {
    // +CGPRCO:<cid>, <v4_dns1>, <v4_dns2>, <v6_dns1>, <v6_dns2>
    // , <initial_attach>, <mtu_size>, <op-PCO_code:op-PCO_content:op-PCO_action>
    char* out = NULL;
    int err = 0;
    RIL_PCO_Data pcoData;
    char pcoIdStr[5];
    char plmn[7];
    int pcoContent = -1;

    err = at_tok_start(&line);
    if (err < 0) {
        LOGE("[%d][%s] parse line error: %d", rid, __FUNCTION__, err);
        return;
    }

    // <cid>
    int responseCid = -1;
    err = at_tok_nextint(&line, &responseCid);
    if (err < 0) {
        LOGE("[%d][%s] parse cid error: %d", rid, __FUNCTION__, err);
        return;
    }

    if (cid != INVALID_CID && cid  != responseCid) {
        LOGI("[%d][%s] not reported cid: %d, %d", rid, __FUNCTION__, cid, responseCid);
        return;
    }

    // <v4_dns1>, <v4_dns2>, <v6_dns1>, <v6_dns2>
    int i = 0;
    for (i = 0; i < MAX_NUM_DNS_ADDRESS_NUMBER * 2; i++) {
        if (!at_tok_hasmore(&line)) {
            break;
        }
        err = at_tok_nextstr(&line, &out);
        if (err < 0) {
            LOGE("[%d][%s] parse dns error: %d", rid, __FUNCTION__, err);
            return;
        }
    }

    // <inital attach>
    if (at_tok_hasmore(&line)) {
        int isIa = 0;
        err = at_tok_nextint(&line, &isIa);
        if (err < 0) {
            LOGE("[%d][%s] parse inital_attach error: %d", rid, __FUNCTION__, err);
            return;
        }
    }

    // <mtu_size>
    if (at_tok_hasmore(&line)) {
        int mtu = 0;
        err = at_tok_nextint(&line, &mtu);
        if (err < 0) {
            LOGE("[%d][%s] parse mtu_size error: %d", rid, __FUNCTION__, err);
            return;
        }
    }

    // <op-PCO_code:op-PCO_content:op-PCO_action>
    if (at_tok_hasmore(&line)) {
        err = at_tok_nextstr(&line, &out);
        if (err < 0) {
            LOGE("[%d][%s] parse OP PCO error: %d", rid, __FUNCTION__, err);
            return;
        }
        char *token = strsep(&out, ":");
        if (token == NULL) {
            LOGE("[%d][%s] parse OP PCO code error", rid, __FUNCTION__);
            return;
        }
        strncpy(pcoIdStr, token, strlen(token));

        token = strsep(&out, ":");
        if (token == NULL) {
            LOGE("[%d][%s] parse OP PCO content error", rid, __FUNCTION__);
            return;
        }
        strncpy(plmn, token, strlen(token));

        token = strsep(&out, ":");
        if (token == NULL || strlen(token) == 0) {
            LOGE("[%d][%s] parse OP PCO action eror", rid, __FUNCTION__);
            return;
        }

        pcoContent = atoi(token);

        if (isOp12MccMnc(plmn) != 1 && strcmp(pcoIdStr, OP12_PCO_ID) == 0) {
            // TODO: Extend here if need to support another carrier.
            LOGE("[%d][%s] Do not handle PCO for PLMN: %s or PCO ID: %s\n",
                    rid,  __FUNCTION__, plmn, pcoIdStr);
            return;
        }

        if (pcoContent > 255 || pcoContent < 0) {
            LOGE("[%s] PCO value is out of range.\n");
            return;
        }

        // cid
        pcoData.cid = pdn_info[rid][responseCid].interfaceId;

        // Bearer proto  (Currently, there's no need to use this parameter.)
        int isAddrV4V6Bitmap = 0; //Bitmap: 00:none, 01:IPv4, 10:IPv6, 11: IPv4v6
        if (strlen(pdn_info[rid][responseCid].addressV4) > 0 &&
                strcmp(pdn_info[rid][responseCid].addressV4, NULL_IPV4_ADDRESS) != 0) {
            isAddrV4V6Bitmap |= IPV4 + 1;
        }
        if (strlen(pdn_info[rid][responseCid].addressV6) > 0 &&
                strcmp(pdn_info[rid][responseCid].addressV6, NULL_IPV4_ADDRESS) != 0) {
            isAddrV4V6Bitmap |= IPV6 + 1;
        }
        if ((IPV4V6 + 1) == isAddrV4V6Bitmap) {
            asprintf(&pcoData.bearer_proto, "IPV4V6");
        } else if (((IPV4 + 1) == isAddrV4V6Bitmap) || ((IPV6 + 1) == isAddrV4V6Bitmap)) {
            if ((IPV4 + 1) == isAddrV4V6Bitmap) {
                asprintf(&pcoData.bearer_proto, "IP");
            } else {
                asprintf(&pcoData.bearer_proto, "IPV6");
            }
        } else if (0 == isAddrV4V6Bitmap) {
            int ipType = getAddressTypeByCid(responseCid, rid);
            LOGD("[%s] getAddressTypeByCid: %d", __FUNCTION__, ipType);
            if (ipType == IPV4) {
                asprintf(&pcoData.bearer_proto, "IP");
            } else if (ipType == IPV4V6) {
                asprintf(&pcoData.bearer_proto, "IPV4V6");
            } else if (ipType == IPV6) {
                asprintf(&pcoData.bearer_proto, "IPV6");
            } else {
                asprintf(&pcoData.bearer_proto, "");
            }
        }

        pcoData.pco_id = (int) strtol(pcoIdStr, NULL, 16);

        // PCO contents length (1 byte for FF00 VZW.)
        pcoData.contents_length = 1;

        // PCO value (hex string) (Current only support 1 byte for VZW PCO FF00H)
        pcoData.contents = (char*) malloc(pcoData.contents_length * sizeof(char));
        if (pcoData.contents == NULL) {
            LOGE("[%s] calloc for pcoData.contents failed!", __FUNCTION__);
            return;
        }
        pcoData.contents[0] = pcoContent;

        LOGD("[%s] PLMN = %s, \
            interface = %d, \
            bearerProto = %s, \
            pco id(decimal) = %s, \
            contents length = %d",
            __FUNCTION__,
            plmn,
            pcoData.cid,
            pcoData.bearer_proto,
            pcoData.pco_id,
            pcoData.contents_length);

        int j;
        for (j = 0; j < pcoData.contents_length; j++) {
            LOGD("[%d][%s] contents[%d] = %d", rid, __FUNCTION__, j, pcoData.contents[j]);
        }

        if (pcoData.cid != -1) {
            RIL_UNSOL_RESPONSE(RIL_UNSOL_PCO_DATA, &pcoData,
                    sizeof(pcoData), rid);
        } else {
            // Only for data attached.
            //RIL_PCO_Data_attached
            // Only for data attached.
            char* name = NULL;

            // Use string array instead of create a new structure for apn name(string type).
            char* responseStr[7] = {NULL};
            if (apnname != NULL && strlen(apnname) > 0) {
                name = (char *) malloc(sizeof(char) * (strlen(apnname) + 1));
                if (name == NULL) {
                    LOGE("[%s] malloc for name failed!", __FUNCTION__);
                    return;
                }
                strncpy(name, apnname, strlen(apnname));
            } else {
                if (strlen(pdn_info[rid][responseCid].apn) > 0
                        && pdn_info[rid][responseCid].active == DATA_STATE_LINKDOWN) {
                    name = (char *) calloc(sizeof(char), (strlen(pdn_info[rid][responseCid].apn) + 1));
                    if (name == NULL) {
                        LOGE("[%s] calloc for name failed!", __FUNCTION__);
                        return;
                    }
                    strncpy(name, pdn_info[rid][responseCid].apn, strlen(pdn_info[rid][responseCid].apn));
                    name[strlen(pdn_info[rid][responseCid].apn)] = 0;
                } else {
                    LOGI("[%s] RIL_UNSOL_PCO_DATA for interface id -1 case.", __FUNCTION__);
                    RIL_UNSOL_RESPONSE(RIL_UNSOL_PCO_DATA, &pcoData,
                            sizeof(pcoData), rid);
                    return;
                }
            }

            LOGI("[%s] APN name [%s]", __FUNCTION__, name);
            if (name != NULL && strlen(name) > 0
                && (strcmp(name, VZW_IMS_NI) == 0 || strcmp(name, VZW_INTERNET_NI) == 0)) {
                RIL_PCO_Data_attached pcoDataAttached;
                pcoDataAttached.cid = pcoData.cid;
                asprintf(&pcoDataAttached.apn_name, "%s", name);
                asprintf(&pcoDataAttached.bearer_proto, "%s", pcoData.bearer_proto);
                pcoDataAttached.pco_id = pcoData.pco_id;
                pcoDataAttached.contents_length = pcoData.contents_length;
                pcoDataAttached.contents = (char *) malloc(sizeof(char) * pcoDataAttached.contents_length);
                if (pcoDataAttached.contents == NULL) {
                    LOGE("[%s] malloc for pcoDataAttached.contents failed!", __FUNCTION__);
                    return;
                }
                for (int i = 0; i < pcoDataAttached.contents_length; i++) {
                    pcoDataAttached.contents[i] = pcoData.contents[i];
                }

                RIL_UNSOL_RESPONSE(RIL_UNSOL_PCO_DATA_AFTER_ATTACHED, &pcoDataAttached,
                    sizeof(pcoDataAttached), rid);
            }
        }
    }
}

int getAddressTypeByCid(int cid, RIL_SOCKET_ID rid) {
    int err = -1;
    char *line = NULL, *out = NULL;
    char *cmd = NULL;
    ATResponse *p_response = NULL;
    int responseCid = -1;
    RILChannelCtx* rilchnlctx = getChannelCtxbyProxy();

    /* Use AT+CGPADDR=cid to query the ip address assigned to this PDP context indicated via this cid */
    asprintf(&cmd, "AT+CGPADDR=%d", cid);
    err = at_send_command_singleline(cmd, "+CGPADDR:", &p_response, rilchnlctx);
    free(cmd);

    /* Parse the response to get the ip address */
    if (isATCmdRspErr(err, p_response)) {
        goto error;
    }
    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    /* line => +CGPADDR: <cid>,<PDP_addr_1>,<PDP_addr_2> */

    if (err < 0)
        goto error;

    /* Get 1st parameter: CID */
    err = at_tok_nextint(&line, &responseCid);
    if (err < 0)
        goto error;

    /* Get 2nd parameter: IPAddr1 */
    err = at_tok_nextstr(&line, &out);
    if (err < 0)
        goto error;

    return get_address_type(out);

error:
    AT_RSP_FREE(p_response);
    return -1;
}

void getAndSendPcoStatusWithParam(void* param) {
    TimeCallbackParam* timeCallbackParam = (TimeCallbackParam*)param;
    RIL_SOCKET_ID rid = timeCallbackParam->rid;
    char* line = (char*)(timeCallbackParam->urc);
    int err;

    LOGD("[%s] receive %s, rid = %d", __FUNCTION__, line, rid);
    err = at_tok_start(&line);
    if (err < 0) {
        LOGE("[%s] parse urc error", __FUNCTION__);
        goto error;
    }

    // <cid>
    int responseCid = -1;
    err = at_tok_nextint(&line, &responseCid);
    if (err < 0) {
        LOGE("[%s] parse cid error", __FUNCTION__);
        goto error;
    }

    getAndSendPcoStatus(rid, responseCid, NULL);

    FREEIF(timeCallbackParam->urc);
    FREEIF(timeCallbackParam);
    return;

error:
    FREEIF(timeCallbackParam->urc);
    FREEIF(timeCallbackParam);
}

void getAndSendPcoStatus(RIL_SOCKET_ID rid, int cid, char* apnName) {
    if (!isOp12Support()) {
        return;
    }

    ATLine *p_cur = NULL;
    ATResponse *p_response = NULL;
    RILChannelCtx* pChannel = getChannelCtxbyProxy();

    // Use AT+CGPRCO? to query each cid's dns server address
    // +CGPRCO:<cid>, <v4_dns1>, <v4_dns2>, <v6_dns1>, <v6_dns2>
    // , <initial_attach>, <mtu_size>, <op-PCO_code:op-PCO_content:op-PCO_action>
    int err = at_send_command_multiline("AT+CGPRCO?", "+CGPRCO:", &p_response, pChannel);
    if (isATCmdRspErr(err, p_response)) {
        LOGE("[%d][%s] can not get the PCO status", rid, __FUNCTION__);
    } else {
        for (p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next) {
            parseAndSendPcoStatus(p_cur->line, rid, cid, apnName);
        }
    }

    AT_RSP_FREE(p_response);
    return;
}

void onPcoStatus(void* param) {
    // +CGPRCO:<cid>, <v4_dns1>, <v4_dns2>, <v6_dns1>, <v6_dns2>
    // , <initial_attach>, <mtu_size>, <op-PCO_code:op-PCO_content:op-PCO_action>
    TimeCallbackParam* timeCallbackParam = (TimeCallbackParam*)param;
    char* line = (char*)(timeCallbackParam->urc);
    RIL_SOCKET_ID rid = timeCallbackParam->rid;

    LOGD("[%s] receive %s, rid = %d", __FUNCTION__, line, rid);
    parseAndSendPcoStatus(line, rid, INVALID_CID, NULL);

    FREEIF(timeCallbackParam->urc);
    FREEIF(timeCallbackParam);
    return;
}

int definePdnCtx(const RIL_Default_Bearer_VA_Config_Struct *pDefaultBearerVaConfig,
        const char *requestedApn, int protocol, int availableCid,
        int authType, const char *username, const char* password, RILChannelCtx *pChannel)
{
    int nRet = 1; //success
    int err = 0;
    ATResponse *p_response = NULL;
    char cmd[AT_COMMAND_MEDIUM_LENGTH] = {0};
    int emergency_ind = 0;              //0:normal, 1:emergency
    int pcscf_discovery_flag = 0;
    int signalingFlag = 0;
    char tmp[PROPERTY_VALUE_MAX] = {0};
    int requestType = REQUEST_TYPE_NORMAL;
    int isHandover = 0;
    int queryMtu = 0;
    const char *pszIPStr;

    config_pco_param_t configPco = {0};

    configPco.cid = availableCid;
    configPco.username = username;
    configPco.password = password;
    configPco.authType = authType;
    configPco.requestedApn = requestedApn;
    configPco.pChannel = pChannel;

    //+CGDCONT=[<cid>[,<PDP_type>[,<APN>[,<PDP_addr>[,<d_comp>[,<h_comp>[,<IPv4AddrAlloc>[,<emergency_indication>
        //[,<P-CSCF_discovery>[,<IM_CN_Signalling_Flag_Ind>]]]]]]]]]]
    if (pDefaultBearerVaConfig != NULL) {
        emergency_ind = pDefaultBearerVaConfig->emergency_ind;
        pcscf_discovery_flag = pDefaultBearerVaConfig->pcscf_discovery_flag;
        signalingFlag = pDefaultBearerVaConfig->signalingFlag;
        isHandover = pDefaultBearerVaConfig->isHandover;
        LOGD("pDefaultBearerVaConfig emergency_ind=%d, pcscf_discovery_flag=%d, signalingFlag=%d, isHandover=%d",
               emergency_ind, pcscf_discovery_flag, signalingFlag, isHandover);
    }

    // In the case of barAPN, need to set the handover flag as well.
    if (isHandover || s_isBarApnHandover) {
        LOGD("[%s] PDP context is for handover from a non-3GPP access network.", __FUNCTION__);
        requestType = REQUEST_TYPE_HANDOVER;
    }

    if (protocol == IPV4V6) {
        pszIPStr = SETUP_DATA_PROTOCOL_IPV4V6;
    } else if (protocol == IPV6) {
        pszIPStr = SETUP_DATA_PROTOCOL_IPV6;
    } else {
        pszIPStr = SETUP_DATA_PROTOCOL_IP;
    }
    configPco.pszIPStr = pszIPStr;

    // [mtu]
    property_get("vendor.ril.data.query.mtu", tmp, "1"); // [query mtu]  0:turn off ; 1: turn on
    queryMtu = atoi(tmp);
    configPco.queryMtu = queryMtu;
    if (isOp07Support()) {
        configPco.queryMtu = 0;
    }

    // AT+CGPRCO
    configPcoParam(&configPco);

    //+CGDCONT=[<cid>[,<PDP_type>[,<APN>[,<PDP_addr>[,<d_comp>[,<h_comp>[,<IPv4AddrAlloc>
    //[,<emergency_indication>[,<P-CSCF_discovery>[,<IM_CN_Signalling_Flag_Ind>]]]]]]]]]]
    if (max_pdn_support_4G > 0) {
        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "AT+CGDCONT=%d,\"%s\",\"%s\",,0,0,0,%d,%d,%d",availableCid,
                pszIPStr, requestedApn, emergency_ind, pcscf_discovery_flag, signalingFlag);
        if (requestType == REQUEST_TYPE_HANDOVER) {
            memset(cmd, 0, sizeof(cmd));
            // new AT spec
            snprintf(cmd, sizeof(cmd), "AT+CGDCONT=%d,\"%s\",\"%s\",,0,0,0,%d,%d,%d", availableCid,
                    pszIPStr, requestedApn, requestType, pcscf_discovery_flag, signalingFlag);
            // Reset the barApnHandover flag.
            s_isBarApnHandover = 0;
        }
    } else {
        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "AT+CGDCONT=%d,\"%s\",\"%s\",,0,0",
                availableCid, pszIPStr, requestedApn);
    }

    err = at_send_command(cmd, &p_response, pChannel);

    if (isATCmdRspErr(err, p_response)) {
        if(p_response != NULL && p_response->success == 0) {
            gprs_failure_cause = at_get_cme_error(p_response);
            LOGD("AT+CGDCONT err=%d", gprs_failure_cause);
            //Need to handle UNKNOWN_PDP_ADDR_OR_TYPE in framework
        }
        nRet = 0;
    }
    AT_RSP_FREE(p_response);

    RIL_SOCKET_ID iaSimId = getRILIdByChannelCtx(pChannel);
    // Send romaing protocol to modem via EGDCONT, the original req is from OP12
    // However, this can be used for common flow as well.
    // Currently, modem only check the roaming protocol of cid0, but designed for all
    // cid number for extension.
    // using mutex to protect the section because more than one thread may run definePdnCtx
    pthread_mutex_lock(&iaRoamingMutex[iaSimId]);
    LOGD("definePdnCtx:EGDCONT-cid= %d, roamingProtocol = %s",
        availableCid, g_iaRoamingProtocol[iaSimId]);
    if (g_iaRoamingProtocol[iaSimId] != NULL) {
        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "AT+EGDCONT=%d,\"%s\"",
                availableCid, g_iaRoamingProtocol[iaSimId]);
        err = at_send_command(cmd, NULL, pChannel);
        free(g_iaRoamingProtocol[iaSimId]);
        g_iaRoamingProtocol[iaSimId] = NULL;
    }
    pthread_mutex_unlock(&iaRoamingMutex[iaSimId]);
    return nRet;
}

int activatePdn(int availableCid, int *pActivatedPdnNum, int *pConcatenatedBearerNum, int *activatedCidList, int isEmergency, int isNoDataActive, RILChannelCtx *pChannel)
{
    int nRet = PDN_FAILED;
    char *cmd = NULL;
    int err = 0;
    ATResponse *p_response = NULL;
    char *line = NULL, *out = NULL;
    ATLine *p_cur = NULL;
    int activatedPdnNum = 0;
    int concatenatedBearerNum = 0;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(pChannel);

    *pActivatedPdnNum = activatedPdnNum;
    *pConcatenatedBearerNum = concatenatedBearerNum;

    LOGD("[%d][%s] availableCid: %d, isEmergency: %d, isNoDataActive E",
            rid, __FUNCTION__, availableCid, isEmergency, isNoDataActive);

    if (0 == isNoDataActive) {
        /* Use AT+CGACT=1,cid to activate PDP context indicated via this cid */
        asprintf(&cmd, "AT+CGACT=1,%d", availableCid);
    } else {
        /* Use AT+EGACT=1,<cid>,<rat> ///rat: 0 -> all rat, 1 -> 23G, 2 -> 4G*/
        asprintf(&cmd, "AT+EGACT=1,%d,2", availableCid);
    }

    //+CGEV: ME PDN ACT <cid>
    //+CGEV: ME PDN ACT <cid>[,<reason>[,<cid_other>]]
    err = at_send_command_multiline(cmd, "+CGEV: ME PDN ACT ", &p_response, pChannel);
    free(cmd);
    if (isATCmdRspErr(err, p_response)) {
        if (err == AT_ERROR_CHANNEL_CLOSED) {
            nRet = CHANNEL_CLOSED;
            goto error_channel_closed;
        }
    }

    for (p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next) {
        line = p_cur->line;
        err = at_tok_start(&line);
        /* line => +CGEV: ME PDN ACT X */
        if (err < 0) {
            gprs_failure_cause = PDP_FAIL_ERROR_UNSPECIFIED;
            goto error;
        }

        if (strStartsWith(line, " ME PDN ACT ")) {
            int activatedCid = INVALID_CID;
            int primaryCid = INVALID_CID;
            int reason = NO_CAUSE;
            int otherCid = INVALID_CID;
            char *tempParam = NULL;

            err = at_tok_nextstr(&line, &tempParam);
            if (err < 0) {
                gprs_failure_cause = PDP_FAIL_ERROR_UNSPECIFIED;
                goto error;
            }

            char *firstParam = tempParam + strlen(ME_PDN_URC);
            activatedCid = atoi(firstParam);
            primaryCid = activatedCid;
            pdn_info[rid][activatedCid].isDedicateBearer = 0;
            pdn_info[rid][activatedCid].active = DATA_STATE_LINKDOWN;
            LOGD("[%d][%s] default bearer activated [CID=%d]", rid, __FUNCTION__, activatedCid);

            if (at_tok_hasmore(&line)) {
                err = at_tok_nextint(&line, &reason);
                if (err < 0) {
                    gprs_failure_cause = PDP_FAIL_ERROR_UNSPECIFIED;
                    goto error;
                }

                LOGD("[%d][%s] default bearer activated [CID=%d, Reason=%d]", rid, __FUNCTION__,
                            activatedCid, reason);

                if (at_tok_hasmore(&line)) {
                    err = at_tok_nextint(&line, &otherCid);
                    if (err < 0) {
                        gprs_failure_cause = PDP_FAIL_ERROR_UNSPECIFIED;
                        goto error;
                    }

                    LOGD("[%d][%s] default bearer activated [CID=%d, Reason=%d, OtherCID=%d]",
                                rid, __FUNCTION__, activatedCid, reason, otherCid);
                }
            }

            if (1 == isNoDataActive) {
                pdn_info[rid][activatedCid].isNoDataPdn = 1;
                LOGD("[%d][%s] NoData PDN activated [CID=%d]", rid, __FUNCTION__, activatedCid);
            }

            activatedCidList[activatedPdnNum] = activatedCid;
            pdn_info[rid][activatedCid].primaryCid = primaryCid;
            pdn_info[rid][activatedCid].cid = activatedCid;
            pdn_info[rid][activatedCid].reason = reason;
            pdn_info[rid][availableCid].isEmergency = isEmergency;
            pdn_info[rid][availableCid].rat = RDS_RAN_MOBILE_3GPP;
            ++activatedPdnNum;
        }
    }

    LOGD("[%d][%s] %d PDN activated (%d concatenated)",
            rid, __FUNCTION__, activatedPdnNum, concatenatedBearerNum);

    if (isATCmdRspErr(err, p_response) || !activatedPdnNum) {
        if (p_response->success == 0) {
            gprs_failure_cause = at_get_cme_error(p_response);
            if (gprs_failure_cause > SM_CAUSE_BASE && gprs_failure_cause < SM_CAUSE_END) {
                gprs_failure_cause -= SM_CAUSE_BASE;
                LOGE("[%d][%s] AT+CGACT SM CAUSE: %x", rid, __FUNCTION__, gprs_failure_cause);
            } else if (gprs_failure_cause > ESM_CAUSE_BASE &&
                         gprs_failure_cause <= ESM_CAUSE_END) {
                gprs_failure_cause -= ESM_CAUSE_BASE;
                LOGE("[%d][%s] AT+CGACT ESM CAUSE: %x", rid, __FUNCTION__, gprs_failure_cause);
            } else if (gprs_failure_cause == 126 || gprs_failure_cause == 4634) {
                // The following are failure cause to activate a context defined in TS 27.007
                // and convert to 24.008 cause
                gprs_failure_cause = SM_INSUFFICIENT_RESOURCES;
                LOGE("[%d][%s] AT+CGACT SM CAUSE: %x", rid, __FUNCTION__, gprs_failure_cause);
            } else if (gprs_failure_cause == 127 || gprs_failure_cause == 4635) {
                gprs_failure_cause = SM_MISSING_UNKNOWN_APN;
                LOGE("[%d][%s] AT+CGACT SM CAUSE: %x", rid, __FUNCTION__, gprs_failure_cause);
            } else if (gprs_failure_cause == 128) {
                gprs_failure_cause = SM_UNKNOWN_PDP_ADDRESS_TYPE;
                LOGE("[%d][%s] AT+CGACT SM CAUSE: %x", rid, __FUNCTION__, gprs_failure_cause);
            } else if (gprs_failure_cause == 129) {
                gprs_failure_cause = SM_USER_AUTHENTICATION_FAILED;
                LOGE("[%d][%s] AT+CGACT SM CAUSE: %x", rid, __FUNCTION__, gprs_failure_cause);
            } else if (gprs_failure_cause == 130) {
                gprs_failure_cause = SM_ACTIVATION_REJECT_GGSN;
                LOGE("[%d][%s] AT+CGACT SM CAUSE: %x", rid, __FUNCTION__, gprs_failure_cause);
            } else if (gprs_failure_cause == 131) {
                gprs_failure_cause = SM_ACTIVATION_REJECT_UNSPECIFIED;
                LOGE("[%d][%s] AT+CGACT SM CAUSE: %x", rid, __FUNCTION__, gprs_failure_cause);
            } else if (gprs_failure_cause == 132) {
                gprs_failure_cause = SM_SERVICE_OPTION_NOT_SUPPORTED;
                LOGE("[%d][%s] AT+CGACT SM CAUSE: %x", rid, __FUNCTION__, gprs_failure_cause);
            } else if (gprs_failure_cause == 133) {
                gprs_failure_cause = SM_SERVICE_OPTION_NOT_SUBSCRIBED;
                LOGE("[%d][%s] AT+CGACT SM CAUSE: %x", rid, __FUNCTION__, gprs_failure_cause);
            } else if (gprs_failure_cause == 134) {
                gprs_failure_cause = SM_SERVICE_OPTION_OUT_OF_ORDER;
                LOGE("[%d][%s] AT+CGACT SM CAUSE: %x", rid, __FUNCTION__, gprs_failure_cause);
            } else if (gprs_failure_cause == 135) {
                gprs_failure_cause = SM_NSAPI_IN_USE;
                LOGE("[%d][%s] AT+CGACT SM CAUSE: %x", rid, __FUNCTION__, gprs_failure_cause);
            } else if (gprs_failure_cause == 136) {
                gprs_failure_cause = SM_REGULAR_DEACTIVATION;
                LOGE("[%d][%s] AT+CGACT SM CAUSE: %x", rid, __FUNCTION__, gprs_failure_cause);
            } else if (gprs_failure_cause == 137) {
                gprs_failure_cause = SM_QOS_NOT_ACCEPTED;
                LOGE("[%d][%s] AT+CGACT SM CAUSE: %x", rid, __FUNCTION__, gprs_failure_cause);
            } else if (gprs_failure_cause == 138) {
                gprs_failure_cause = SM_NETWORK_FAILURE;
                LOGE("[%d][%s] AT+CGACT SM CAUSE: %x", rid, __FUNCTION__, gprs_failure_cause);
            } else if (gprs_failure_cause == 139) {
                gprs_failure_cause = SM_REACTIVATION_REQUESTED;
                LOGE("[%d][%s] AT+CGACT SM CAUSE: %x", rid, __FUNCTION__, gprs_failure_cause);
            } else if (gprs_failure_cause == 140) {
                gprs_failure_cause = SM_FEATURE_NOT_SUPPORTED;
                LOGE("[%d][%s] AT+CGACT SM CAUSE: %x", rid, __FUNCTION__, gprs_failure_cause);
            } else if (gprs_failure_cause == 141) {
                gprs_failure_cause = SM_SEMANTIC_ERROR_IN_TFT;
                LOGE("[%d][%s] AT+CGACT SM CAUSE: %x", rid, __FUNCTION__, gprs_failure_cause);
            } else if (gprs_failure_cause == 142) {
                gprs_failure_cause = SM_SYNTACTICAL_ERROR_IN_TFT;
                LOGE("[%d][%s] AT+CGACT SM CAUSE: %x", rid, __FUNCTION__, gprs_failure_cause);
            } else if (gprs_failure_cause == 143) {
                gprs_failure_cause = SM_UNKNOWN_PDP_CONTEXT;
                LOGE("[%d][%s] AT+CGACT SM CAUSE: %x", rid, __FUNCTION__, gprs_failure_cause);
            } else if (gprs_failure_cause == 144) {
                gprs_failure_cause = SM_SEMANTIC_ERROR_IN_PACKET_FILTER;
                LOGE("[%d][%s] AT+CGACT SM CAUSE: %x", rid, __FUNCTION__, gprs_failure_cause);
            } else if (gprs_failure_cause == 145) {
                gprs_failure_cause = SM_SYNTACTICAL_ERROR_IN_PACKET_FILTER;
                LOGE("[%d][%s] AT+CGACT SM CAUSE: %x", rid, __FUNCTION__, gprs_failure_cause);
            } else if (gprs_failure_cause == 146) {
                gprs_failure_cause = SM_PDP_CONTEXT_WITHOU_TFT_ALREADY_ACTIVATED;
                LOGE("[%d][%s] AT+CGACT SM CAUSE: %x", rid, __FUNCTION__, gprs_failure_cause);
            } else if (gprs_failure_cause == 147) {
                gprs_failure_cause = SM_MULTICAST_GROUP_MEMBERSHIP_TIMEOUT;
                LOGE("[%d][%s] AT+CGACT SM CAUSE: %x", rid, __FUNCTION__, gprs_failure_cause);
            } else if (gprs_failure_cause == 148) {
                gprs_failure_cause = SM_BCM_VIOLATION;
                LOGE("[%d][%s] AT+CGACT SM CAUSE: %x", rid, __FUNCTION__, gprs_failure_cause);
            } else if (gprs_failure_cause == 149) {
                gprs_failure_cause = SM_USER_AUTHENTICATION_FAILED;
                LOGE("[%d][%s] AT+CGACT SM CAUSE: %x", rid, __FUNCTION__, gprs_failure_cause);
            } else if (gprs_failure_cause == 3911 && pdnMdVersion >= MD_LR11) {
                gprs_failure_cause = 3910; // align with the value defined by data fw
                nRet = LOCAL_REJECT_CAUSE;
                LOGE("[%d][%s] AT+CGACT err=%d", rid, __FUNCTION__, gprs_failure_cause);
            } else {
                nRet = LOCAL_REJECT_CAUSE;
                //Need to handle UNKNOWN_PDP_ADDR_OR_TYPE in framework
                LOGE("[%d][%s] AT+CGACT err=%d", rid, __FUNCTION__, gprs_failure_cause);
            }
        }
#ifdef disable_test_load
        goto error;
#endif
    } else {
        gprs_failure_cause = 0;
        LOGD("[%s] reset gprs_failure_cause", __FUNCTION__);
    }

    nRet = PDN_SUCCESS;
    *pActivatedPdnNum = activatedPdnNum;
    *pConcatenatedBearerNum = concatenatedBearerNum;
    AT_RSP_FREE(p_response);

    LOGV("[%d][%s] finished X", rid, __FUNCTION__);
    return nRet;
error:
    AT_RSP_FREE(p_response);
    LOGE("[%d][%s] error X", rid, __FUNCTION__);
    return nRet;

error_channel_closed:
    gprs_failure_cause = PDP_FAIL_ERROR_UNSPECIFIED;
    AT_RSP_FREE(p_response);
    LOGE("[%d][%s] failed due to SIM switch", rid, __FUNCTION__);
    return nRet;
}

int bindPdnToIntf(int activatedPdnNum,int interfaceId,int cid,RILChannelCtx * pChannel)
{
    int nRet = 1; //0:failed, 1: success
    char cmd[PROPERTY_VALUE_MAX] = {0};
    int err = 0;
    ATResponse *p_response = NULL;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(pChannel);

    LOGD("[%d][%s] activatedPdnNum: %d, interfaceId: %d E",
            rid, __FUNCTION__, activatedPdnNum, interfaceId);
    if (pdn_info[rid][cid].interfaceId == INVALID_CID
            || pdn_info[rid][cid].interfaceId != interfaceId) {
        // Always leave data state firstly
        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "AT+EGLD=%d", cid);
        err = at_send_command(cmd, &p_response, pChannel);
        if (isATCmdRspErr(err, p_response)) {
            LOGD("[%d][%s] skip link down error for cid=%d", rid, __FUNCTION__, cid);
        }
        AT_RSP_FREE(p_response);

        /* AT+CGDATA=<L2P>,<cid>,<channel ID> */
        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "AT+CGDATA=\"%s\",%d,%d",
                s_l2p_value, cid, interfaceId+1); //The channel id is from 1~n
        err = at_send_command(cmd, &p_response, pChannel);
        if (isATCmdRspErr(err, p_response)) {
            LOGE("[%d][%s] CID%d fail to bind interface%d", rid, __FUNCTION__, cid, interfaceId);
            nRet = 0;
            goto error;
        } else {
            //The PDP is bind to a network interface, set to active
            LOGD("[%d][%s] CID%d is bind to interface%d", rid, __FUNCTION__, cid, interfaceId);
            pdn_info[rid][cid].interfaceId = interfaceId;
            pdn_info[rid][cid].active = DATA_STATE_ACTIVE;
        }
    } else {
        LOGD("[%d][%s] not to bind interface again, CID%d is already bind to interface%d",
                rid, __FUNCTION__, cid, interfaceId);
    }

    LOGD("[%d][%s] finished X", rid, __FUNCTION__);
    AT_RSP_FREE(p_response);
    return nRet;

error:
    LOGD("[%d][%s] error X", rid, __FUNCTION__);
    AT_RSP_FREE(p_response);
    return nRet;
}

int getAttachApnSupport()
{
    int err = 0;
    char* support = NULL;
    char* line = NULL;
    ATResponse *p_response = NULL;

    if (attachApnSupport == -1) {
        //+EGMR: "1" for partial support
        //+EGMR: "0" for full support
        err = at_send_command_singleline("AT+EGMR=0,16", "+EGMR:", &p_response,
                getChannelCtxbyProxy());
        if (err < 0 || p_response->success == 0) {
            LOGE("IA: getAttachApnSupport get error response");
            goto error;
        }

        line = p_response->p_intermediates->line;
        err = at_tok_start(&line);
        if (err < 0) {
            goto error;
        }
        err = at_tok_nextstr(&line, &support);
        if (err < 0) {
            goto error;
        }
        if (strcmp("1", support) == 0) {
            attachApnSupport = ATTACH_APN_PARTIAL_SUPPORT;
        } else {
            attachApnSupport = ATTACH_APN_NOT_SUPPORT;
        }
        LOGV("IA: getAttachApnSupport is [%d]", attachApnSupport);
        at_response_free(p_response);
    }
    return attachApnSupport;

error:
    at_response_free(p_response);
    return ATTACH_APN_NOT_SUPPORT;
}

bool isNeedUseIaCache() {
    bool ret = true;

    if (isOp12Support()) {
        /* VZW requires (SS 2.4) can't do attach is class1 or class2
           is disabled. So we should not use IA cache to define cid0
           before syncApnTable(VZWAPNE).
        */
        ret = false;
    }
    return ret;
}

// Multi-PS Attach Start
int defineAttachApnBeforeRadioOn(RIL_SOCKET_ID rid, RILChannelCtx *pChannel) {
    int ret = 0;
    GEMINI_MODE proj_type = getGeminiMode();
    if (proj_type != GEMINI_MODE_L_AND_L) {
        ret = defineAttachApnIfIACacheExisted(rid, pChannel);
    } else {
        ret = defineMultiAttachApnIfIACacheExisted(rid, pChannel);
    }
    return ret;
}

void defineAttachApnAsync(void* param) {
    char* cmd = NULL;
    RILChannelCtx* pChannel = (RILChannelCtx*)param;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(pChannel);
    LOGD("[%d]defineAttachApnAsync", rid);

    defineAttachApnIfIACacheExisted(rid, pChannel);
}

int defineMultiAttachApnIfIACacheExisted(RIL_SOCKET_ID rid, RILChannelCtx *pChannel) {
    LOGD("defineMultiAttachApnIfIACacheExisted");
    int n3gSimId = RIL_get3GSIM();
    int currSim = rid + 1;
    ATResponse *p_response = NULL;
    int err = 0;

    RILChannelCtx* pDataChannel = getChannelCtxbyProxy();

    // setRadioPower use NW channel, we have to use the same channel to avoid command occupy.
    if (n3gSimId == currSim) {
        defineAttachApnIfIACacheExisted(rid, pChannel);
    } else {
        RIL_requestProxyTimedCallback(defineAttachApnAsync, pDataChannel, &TIMEVAL_0,
                pDataChannel->id, "defineAttachApnAsync");
    }

    return 1;
}
// Multi-PS Attach End

int defineAttachApnIfIACacheExisted(RIL_SOCKET_ID rid, RILChannelCtx *pChannel)
{
    int ret = 0;
    int isOp01 = 0;
    int check = 0;
    char optr[PROPERTY_VALUE_MAX] = {0};
    GEMINI_MODE proj_type = getGeminiMode();
    if (proj_type != GEMINI_MODE_L_AND_L) {
        if ((rid + 1) != (unsigned int) RIL_get3GSIM()) {
            LOGD("IA: defineAttachApnIfIACacheExisted but not main socket");
            return 0;
        }
    }

    int iaSimId = rid;

    property_get("persist.vendor.operator.optr", optr, "");
    isOp01 = (strcmp("OP01", optr) == 0);

    if (getAttachApnSupport() == ATTACH_APN_NOT_SUPPORT) {
        //Attach apn mechanism is not support, set to always attach
        LOGV("IA: defineAttachApnIfIACacheExisted IA mechanism is not supported");
        if (isOp01) {
            check = definePdnCtx(NULL, "", IPV4V6, 0, AUTHTYPE_NOT_SET, "", "", pChannel);
        } else {
            check = definePdnCtx(NULL, "", IPV4, 0, AUTHTYPE_NOT_SET, "", "", pChannel);
        }
        ret = 1;
    } else if (isNeedUseIaCache()) {
        //check if IA APN is set or not
        char iccidProperty[PROPERTY_VALUE_MAX] = {0};
        char maskIccid[PROPERTY_VALUE_MAX] = {0};
        char apnIccid[PROPERTY_VALUE_MAX] = {0};
        getMSimProperty(iaSimId, PROPERTY_IA_APN_SET_ICCID, apnIccid);
        property_get(PROPERTY_ICCID_SIM[rid], iccidProperty, "");

        int authType = 0;
        char* iaProperty = alloca(PROPERTY_VALUE_MAX*2);
        char* iccid = NULL;
        char* protocol = NULL;
        char* apn = NULL;
        char* username = NULL;
        char* roamingProtocol = NULL;
        int canHandleIms = 0;
        memset(iaProperty, 0, PROPERTY_VALUE_MAX*2);
        pthread_mutex_lock(&iaRoamingMutex[iaSimId]);
        if (g_iaRoamingProtocol[iaSimId] != NULL) {
            free(g_iaRoamingProtocol[iaSimId]);
            g_iaRoamingProtocol[iaSimId] = NULL;
        }
        pthread_mutex_unlock(&iaRoamingMutex[iaSimId]);
        getIaCache(iaSimId, iaProperty);

        //The format of IA property is "[ICCID],[Protocol],[APN],[Auth Type],[Username]"
        //Ex. "89860011010746583662,IP,internet,0,username"
        if (strlen(iaProperty) > 0) {
            check = at_tok_nextstr(&iaProperty, &iccid);
            if (!at_tok_hasmore(&iaProperty)) {
                LOGE("IA: defineAttachApnIfIACacheExisted property parsing iccid error");
                goto finish;
            }
            check = at_tok_nextstr(&iaProperty, &protocol);
            if (!at_tok_hasmore(&iaProperty)){
                LOGE("IA: defineAttachApnIfIACacheExisted property parsing protocol error");
                goto finish;
            }
            check = at_tok_nextint(&iaProperty, &authType);
            if (!at_tok_hasmore(&iaProperty)){
                LOGE("IA: defineAttachApnIfIACacheExisted roperty parsing authType error");
                goto finish;
            }
            check = at_tok_nextstr(&iaProperty, &username);
            if (!at_tok_hasmore(&iaProperty)){
                LOGE("IA: defineAttachApnIfIACacheExisted property parsing username error");
                goto finish;
            }
            check = at_tok_nextint(&iaProperty, &canHandleIms);

            check = at_tok_nextstr(&iaProperty, &roamingProtocol);
            if (strlen(roamingProtocol) > 0) {
                pthread_mutex_lock(&iaRoamingMutex[iaSimId]);
                g_iaRoamingProtocol[iaSimId] = calloc(1, (strlen(roamingProtocol) + 1));
                strncpy(g_iaRoamingProtocol[iaSimId], roamingProtocol, strlen(roamingProtocol));
                pthread_mutex_unlock(&iaRoamingMutex[iaSimId]);
            }
            check = at_tok_nextstr(&iaProperty, &apn);
            givePrintableIccid(iccid, maskIccid);
            LOGD("IA: defineAttachApnIfIACacheExisted property parsing result \
[%s, %s, %d, %s, %d, %s, %s]", maskIccid, protocol, authType,
                    username, canHandleIms, g_iaRoamingProtocol[iaSimId], apn);

            if (strlen(iccid) > 0 && strlen(protocol) > 0) {
                if (strcmp(iccidProperty, iccid) == 0) {
                    RIL_Default_Bearer_VA_Config_Struct* defaultBearer = NULL;
                    int isOp01VolteOn = isOp01Volte();

                    LOGV("IA: defineAttachApnIfIACacheExisted cache is matched with current SIM");
                    // Always request PCSCF for OP01 project as operator requests.
                    if (canHandleIms || isOp01VolteOn) {
                        defaultBearer = calloc(1, sizeof(RIL_Default_Bearer_VA_Config_Struct));
                        if (defaultBearer == NULL) {
                            LOGE("IA: defaultBearer calloc failed.");
                            goto finish;
                        }
                        defaultBearer->signalingFlag = isOp01VolteOn ? 0 : 1;
                        defaultBearer->pcscf_discovery_flag  =1;
                        defaultBearer->emergency_ind = 0;
                    }

                    // for op01 test sim, force apn to empty.
                    if (RIL_isTestSim(rid) && (isOp01 || isOp01VolteOn)) {
                        apn = "";
                    }

                    if (definePdnCtx(defaultBearer, apn, get_protocol_type(protocol), 0, getAuthTypeInt(authType),
                            username, "", pChannel)) {
                        LOGE("IA: defineAttachApnIfIACacheExisted define attach APN success, set \
to always attach mode");
                        ret = 1;
                    } else {
                        if (4096 == gprs_failure_cause) {
                            LOGD("Ignore err 4096 of cmd conflict");
                            ret = 1;
                        } else {
                            LOGE("IA: defineAttachApnIfIACacheExisted define attach APN error");
                        }
                    }
                    if (NULL != defaultBearer) {
                        free(defaultBearer);
                    }
                } else {
                    LOGD("IA: defineAttachApnIfIACacheExisted SIM is changed");
                }
            } else {
                LOGE("IA: defineAttachApnIfIACacheExisted invalid IA property");
            }
        } else {
            LOGE("IA: defineAttachApnIfIACacheExisted empty IA property");

            char iaPwdFlag[PROPERTY_VALUE_MAX] = {0};
            getMSimProperty(iaSimId, PROPERTY_IA_PASSWORD_FLAG, iaPwdFlag);
            if (atoi(iaPwdFlag)) {
                LOGE("IA: defineAttachApnIfIACacheExisted empty IA due to password");
                char powerOffModem[PROPERTY_VALUE_MAX] = {0};
                property_get("ro.vendor.mtk_flight_mode_power_off_md", powerOffModem, "0");
                if (atoi(powerOffModem)) {
                    LOGE("IA: defineAttachApnIfIACacheExisted IA depend on SIM loaded");
                    ret = 0;
                    goto finish;
                } else {
                    LOGE("IA: defineAttachApnIfIACacheExisted no SIM loaded but IA password");
                }
            }

            //Code change due to we need AT+CGPRCO when power on attach
            if (strcmp(iccidProperty, apnIccid) == 0) {
                LOGD("IA: defineAttachApnIfIACacheExisted IA APN is already set and do nothing");
                return 0;
            }
        }
    }
finish:
    if (ret == 0) {
        LOGD("IA: defineAttachApnIfIACacheExisted clear IA cache and IA ICCID");
        setIaCache(iaSimId, "", "", 0, "", 0, "", "");
        setTempIa(iaSimId, "", "", 0, "", 0, "", "");

        if (getAttachApnSupport() == ATTACH_APN_PARTIAL_SUPPORT) { //partial solution
            char* apnName = INVALID_APN_VALUE;
            LOGV("IA: Set APN name %s.", apnName);
            // The following is for partial solution
            // set to invalid apn and always attach mode to let ESM block PS attach
            check = definePdnCtx(NULL, apnName, get_protocol_type("IP"), 0, AUTHTYPE_NOT_SET,
                    "", "", pChannel);
        }

        setMSimProperty(iaSimId, PROPERTY_IA_APN_SET_ICCID, "");
    }
    return ret;
}

void clearAttachApnCacheIfNecessary(RIL_SOCKET_ID rid, const char* urc)
{
    GEMINI_MODE proj_type = getGeminiMode();
    if (proj_type != GEMINI_MODE_L_AND_L) {
        if ((rid + 1) != (unsigned int) RIL_get3GSIM()) {
            LOGV("IA: clearAttachApnCacheIfNecessary but not main socket");
            return;
        }
    }
    int iaSimId = rid;

    if (strStartsWith(urc, "+ESIMS: 1,2") || //SIM_REFRESH
        strStartsWith(urc, "+ESIMS: 1,9") || //SIM_VSIM_ON
        strStartsWith(urc, "+ESIMS: 1,12") || //SIM_PLUG_IN
        strStartsWith(urc, "+ESIMS: 1,14")) //SIM_RECOVERY_END
    {
        LOGV("IA: receive SIM event [%s], clear IA cache and ICCID", urc);
        setIaCache(iaSimId, "", "", 0, "", 0, "", "");
        setMSimProperty(iaSimId, PROPERTY_IA_APN_SET_ICCID, "");
        setMSimProperty(iaSimId, PROPERTY_IA_PASSWORD_FLAG, "0");
        memset(TEMP_IA_PASSWORD[iaSimId], 0, sizeof(TEMP_IA_PASSWORD[iaSimId]));
        setTempIa(iaSimId, "", "", 0, "", 0, "", "");
        if (strStartsWith(urc, "+ESIMS: 1,2")) {
            LOGD("IA: skip detach after sim refresh");
            sIsSkipDetachForSimRefresh = 1;
        }
    } else if (strStartsWith(urc, "+ESIMS: 1,6") || //SIM_ACCESS_PROFILE_OFF
            strStartsWith(urc, "+ESIMS: 1,8")) //DUALSIM_CONNECTED
    {
        //for this two event, SIM state will not be changed
        //need to set initial attach APN here due to SIM ready will not be triggerred
        LOGV("IA: receive SIM event [%s], config IA apn again and clear IA ICCID", urc);
        setMSimProperty(iaSimId, PROPERTY_IA_APN_SET_ICCID, "");
        setMSimProperty(iaSimId, PROPERTY_IA_PASSWORD_FLAG, "0");
        memset(TEMP_IA_PASSWORD[iaSimId], 0, sizeof(TEMP_IA_PASSWORD[iaSimId]));
        setTempIa(iaSimId, "", "", 0, "", 0, "", "");
        RIL_requestProxyTimedCallback (onSetAttachApn, NULL, &TIMEVAL_0,
                getRILChannelId(RIL_DATA, rid), "onSetAttachApn");
    }
    else if (strStartsWith(urc, "+ESIMS: 0,11")) //SIM_PLUG_OUT
    {
        LOGV("IA: receive SIM event [%s], clear IA apn", urc);
        setMSimProperty(iaSimId, PROPERTY_IA_APN_SET_ICCID, "");
        setMSimProperty(iaSimId, PROPERTY_IA_PASSWORD_FLAG, "0");
        memset(TEMP_IA_PASSWORD[iaSimId], 0, sizeof(TEMP_IA_PASSWORD[iaSimId]));
        setTempIa(iaSimId, "", "", 0, "", 0, "", "");
    }
}

void getIaCache(int iaSimId, char* cache)
{
    //property1: [ICCID],[Protocol],[Auth Type],[Username]
    //property2: [APN]
    char iaProperty[PROPERTY_VALUE_MAX] = {0};
    char iaProperty2[PROPERTY_VALUE_MAX] = {0};
    char maskIccid[PROPERTY_VALUE_MAX] = {0};

    property_get(PROPERTY_IA[iaSimId], iaProperty, "");
    getMSimProperty(iaSimId, PROPERTY_IA_APN, iaProperty2);

    if (strlen(iaProperty) > 0)
        snprintf(cache, PROPERTY_VALUE_MAX*2, "%s,%s", iaProperty, iaProperty2);
    else
        snprintf(cache, 1, "%s", ""); //no IA cache
    givePrintableIccid(cache, maskIccid);
    LOGD("IA: getIaCache(%d) [%s]", iaSimId, maskIccid);
}

void setIaCache(int iaSimId, const char* iccid, const char* protocol, const int authtype,
        const char* username, const int canHandleIms, const char* apn,
        const char* roamingProtocol)
{
    char iaPropertyOld[PROPERTY_VALUE_MAX] = {0};
    char iaProperty2Old[PROPERTY_VALUE_MAX] = {0};
    property_get(PROPERTY_IA[iaSimId], iaPropertyOld, "");
    getMSimProperty(iaSimId, PROPERTY_IA_APN, iaProperty2Old);
    if (iccid == NULL || strlen (iccid) == 0) {
        if(strcmp(iaPropertyOld, "") == 0 && strcmp(iaProperty2Old, "") == 0) {
            LOGD("IA: setIaCache(%d) cache no change", iaSimId);
        } else {
            property_set(PROPERTY_IA[iaSimId], "");
            setMSimProperty(iaSimId, PROPERTY_IA_APN, "");
            LOGD("IA: setIaCache(%d) clear cache", iaSimId);
        }
    } else {
        char iaProperty[PROPERTY_VALUE_MAX] = {0};
        char iaProperty2[PROPERTY_VALUE_MAX] = {0};
        char maskIccid[PROPERTY_VALUE_MAX] = {0};
        snprintf(iaProperty, sizeof(iaProperty), "%s,%s,%d,%s,%d,%s", iccid, protocol,
                authtype, username, canHandleIms, roamingProtocol);
        snprintf(iaProperty2, sizeof(iaProperty2), "%s", apn);
        if(strcmp(iaPropertyOld, iaProperty) == 0 && strcmp(iaProperty2Old, iaProperty2) == 0) {
            LOGD("IA: setIaCache(%d) cache no change", iaSimId);
        } else {
            property_set(PROPERTY_IA[iaSimId], iaProperty);
            setMSimProperty(iaSimId, PROPERTY_IA_APN, iaProperty2);
            givePrintableIccid(iaProperty, maskIccid);
            LOGD("IA: setIaCache(%d) [%s][%s]", iaSimId, maskIccid, iaProperty2);
        }
    }
}

void getTempIa(int iaSimId, char* cache)
{
    //property1: [ICCID],[Protocol],[Auth Type],[Username]
    //property2: [APN]
    char iaProperty[PROPERTY_VALUE_MAX] = {0};
    char iaProperty2[PROPERTY_VALUE_MAX] = {0};
    char maskIccid[PROPERTY_VALUE_MAX] = {0};
    property_get(PROPERTY_TEMP_IA[iaSimId], iaProperty, "");
    getMSimProperty(iaSimId, PROPERTY_TEMP_IA_APN, iaProperty2);

    if (strlen(iaProperty) > 0)
        snprintf(cache, PROPERTY_VALUE_MAX*2, "%s,%s,%s", iaProperty, iaProperty2, TEMP_IA_PASSWORD[iaSimId]);
    else
        snprintf(cache, 1, "%s", ""); //no IA cache
    givePrintableIccid(cache, maskIccid);
    LOGD("IA: getTempIa(%d) [%s]", iaSimId, maskIccid);
}

void setTempIa(int iaSimId, const char* iccid, const char* protocol, const int authtype,
        const char* username, const int canHandleIms, const char* apn,
        const char* roamingProtocol)
{
    char iaPropertyOld[PROPERTY_VALUE_MAX] = {0};
    char iaProperty2Old[PROPERTY_VALUE_MAX] = {0};
    property_get(PROPERTY_TEMP_IA[iaSimId], iaPropertyOld, "");
    getMSimProperty(iaSimId, PROPERTY_TEMP_IA_APN, iaProperty2Old);
    if (iccid == NULL || strlen (iccid) == 0) {
        if(strcmp(iaPropertyOld, "") == 0 && strcmp(iaProperty2Old, "") == 0) {
            LOGD("IA: setTempIa(%d) cache no change", iaSimId);
        } else {
            property_set(PROPERTY_TEMP_IA[iaSimId], "");
            setMSimProperty(iaSimId, PROPERTY_TEMP_IA_APN, "");
            LOGD("IA: setTempIa(%d) clear cache", iaSimId);
        }
    } else {
        char iaProperty[PROPERTY_VALUE_MAX] = {0};
        char iaProperty2[PROPERTY_VALUE_MAX] = {0};
        char maskIccid[PROPERTY_VALUE_MAX] = {0};
        snprintf(iaProperty, sizeof(iaProperty), "%s,%s,%d,%s,%d,%s", iccid, protocol,
                authtype, username, canHandleIms, roamingProtocol);
        snprintf(iaProperty2, sizeof(iaProperty2), "%s", apn);
        if(strcmp(iaPropertyOld, iaProperty) == 0 && strcmp(iaProperty2Old, iaProperty2) == 0) {
            LOGD("IA: setTempIa(%d) cache no change", iaSimId);
        } else {
            property_set(PROPERTY_TEMP_IA[iaSimId], iaProperty);
            setMSimProperty(iaSimId, PROPERTY_TEMP_IA_APN, iaProperty2);
            givePrintableIccid(iaProperty, maskIccid);
            LOGD("IA: setTempIa(%d) [%s][%s]", iaSimId, maskIccid, iaProperty2);
        }
    }
}

int getIaReplaceFlag(int iaSimId)
{
    char iaProperty[PROPERTY_VALUE_MAX] = { 0 };
    int flag = 0;
    getMSimProperty(iaSimId, PROPERTY_RE_IA_FLAG, iaProperty);

    if (strlen(iaProperty) > 0) {
        flag = atoi(iaProperty);
        LOGV("IA: getIaReplaceFlag(%d) [%d]", iaSimId, flag);
    }
    return flag;
}

void setIaReplaceFlag(int iaSimId, int flag)
{
    char* pflag = NULL;

    asprintf(&pflag, "%d", flag);
    setMSimProperty(iaSimId, PROPERTY_RE_IA_FLAG, pflag);
    free(pflag);
    LOGV("IA: setIaReplaceFlag(%d) [%d]", iaSimId, flag);
}

void getIaReplaceApn(int iaSimId, char* buffer)
{
    char iaProperty[PROPERTY_VALUE_MAX] = {0};
    getMSimProperty(iaSimId, PROPERTY_RE_IA_APN, iaProperty);

    if (strlen(iaProperty) > 0)
        snprintf(buffer, PROPERTY_VALUE_MAX, "%s", iaProperty);
    else
        snprintf(buffer, 1, "%s", ""); //no APN

    LOGD("IA: getIaReplaceApn(%d) [%s], [%s]", iaSimId, buffer, iaProperty);
}

int queryCurrentConnType(RILChannelCtx* rilchnlctx) {
    int err=0;
    ATResponse *p_response = NULL;
    char* line;
    int ret = 0;

    err = at_send_command_singleline("AT+EGTYPE?", "+EGTYPE:", &p_response, rilchnlctx);

    if (err != 0 || p_response->success == 0 ||
            p_response->p_intermediates == NULL) goto error;

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &ret);

    LOGV("queryCurrentConnType: ret=%d", ret);
    at_response_free(p_response);
    return ret;
error:
    at_response_free(p_response);
    return 0;
}

int querryCurrentRat(RILChannelCtx* rilchnlctx)
{
    LOGV("querryCurrentRat: AT+CGREG");
    int err=0;
    ATResponse *p_response = NULL;
    char* line;
    int ret = -1;
    int stat = -1;
    int act = -1;
    int skip = 0;
    int len = 0;

    err = at_send_command_singleline("AT+CGREG?", "+CGREG:", &p_response, rilchnlctx);

    if (err != 0 || p_response->success == 0 ||
            p_response->p_intermediates == NULL) goto error;

    line = p_response->p_intermediates->line;

    len = strlen(line);
    LOGV("querryCurrentRat: line is %s, len is %d", line, len);
    err = at_tok_start(&line);
    if (err < 0) {
        LOGE("querryCurrentRat: at_tok_start err");
        goto error;
    }

    if(len > 7)
    {
        err = at_tok_nextint(&line, &skip); //n
        if (err < 0) {
            LOGE("querryCurrentRat: at_tok_nextint err, skip is %d", skip);
            goto error;
        }
    }

    err = at_tok_nextint(&line, &stat); //stat
    if (err < 0) {
        LOGE("querryCurrentRat: at_tok_nextint err, registrationStatus is %d", stat);
        goto error;
    }

    err = at_tok_nexthexint(&line, &skip); //lac
    if (err < 0) {
        LOGE("querryCurrentRat: at_tok_nextint err, lac is %d", skip);
        goto error;
    }

    err = at_tok_nexthexint(&line, &skip); //ci
    if (err < 0) {
        LOGE("querryCurrentRat: at_tok_nextint err, ci is %d", skip);
        goto error;
    }

    err = at_tok_nextint(&line, &act); //act
    if (err < 0) {
        LOGE("querryCurrentRat: at_tok_nextint err, rat is %d", act);
        goto error;
    }

    LOGV("querryCurrentRat: stat is %d, act is %d", stat, act);

    /*
            0 - GSM , 2 -UTRAN, 3 - GSM w/EGPRS, 4 -UTRAN w/HSDPA
            5 - UTRAN w/HSUPA, 6 - UTRAN w/HSDPA and HSUPA, 7 - E-UTRAN
            Not to do reattach in the case of 2/3G and therefore act<7 means 2/3G.
      */
    //TODO: This should revised to a new API which provided by NW.
    if ((stat == HOME_RAT_REGISTERED || stat == ROAMING_RAT_REGISTERED) && act < 7) {
        LOGI("querryCurrentRat: in 2/3G case");
        // In 2/3 case, not to do reattach
        ret = 0;
    }

    at_response_free(p_response);
    return ret;

error:
    at_response_free(p_response);
    return ret;
}

int isIaInvalid(RILChannelCtx* rilchnlctx)
{
    LOGD("isIaInvalid: AT+CGDCONT?");

    ATResponse *p_response = NULL;
    char *line = NULL;
    ATLine *p_cur = NULL;

    int err = 0;
    char *skip = NULL;
    char *apn = NULL;
    int apn_invalid = 0;
    int cid = -1;

    err = at_send_command_multiline("AT+CGDCONT?", "+CGDCONT:", &p_response, rilchnlctx);

    if (isATCmdRspErr(err, p_response)) {
        LOGE("isIaInvalid AT+CGDCONT? response error");
        goto error;
    } else {
        for (p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next) {
            line = p_cur->line;
            err = at_tok_start(&line);
            if (err < 0) continue;

            LOGD("isIaInvalid: line is %s", line);
            err = at_tok_nextint(&line, &cid); //cid
            if (err < 0) continue;

            if (cid == 0) {
                err = at_tok_nextstr(&line, &skip); //pdp_type
                if (err < 0) continue;

                err = at_tok_nextstr(&line, &apn); //apn
                if (err < 0) continue;

                if (strcmp(INVALID_APN_VALUE, apn) == 0) {
                    apn_invalid = 1;
                }
                AT_RSP_FREE(p_response);
                return apn_invalid;
            } else {
                continue;
            }
        }
        AT_RSP_FREE(p_response);
        return apn_invalid;
    }

error:
    AT_RSP_FREE(p_response);
    return apn_invalid;
}

void dumpReqSetupConfig(const reqSetupConf_t * pConfig)
{
    if (pConfig) {
        LOGD("requestedApn=%s, username=%s, passwd=%s, authTYpe=%d, protocol=%d, interfaceId=%d, profile=%s",
            pConfig->requestedApn, pConfig->username, pConfig->password, pConfig->authType,
            pConfig->protocol, pConfig->interfaceId, pConfig->profile);
    }
}

void dumpPdnInfo(PdnInfo * pPdnInfo)
{
    if (pPdnInfo) {
        int i = 0;
        LOGD("[%s] pdn info: intfid=%d, primaryCid=%d, cid=%d, ddcid=%d, \
isDedicatedBearer=%d, isEmergency=%d, active=%d, signalingFlag=%d, apn=%s, \
bearerId=%d, isNotDataPdn=%d, mtu=%d, rat=%d",
            __FUNCTION__, pPdnInfo->interfaceId, pPdnInfo->primaryCid, pPdnInfo->cid,
            pPdnInfo->ddcId, pPdnInfo->isDedicateBearer, pPdnInfo->isEmergency,
            pPdnInfo->active, pPdnInfo->signalingFlag, pPdnInfo->apn,
            pPdnInfo->bearerId, pPdnInfo->isNoDataPdn, pPdnInfo->mtu, pPdnInfo->rat);
    }
}

void dumpAllPdnInfo(RIL_SOCKET_ID rid) {
    int i = 0;
    for(; i < pdnInfoSize; i++) {
        dumpPdnInfo(&pdn_info[rid][i]);
    }
}

void dumpApnConfig(rds_apn_cfg_req_t* apnConfig, int num) {
    for(int i = 0; i < num; i++) {
        LOGD("[%s] apn = %s, type = %s, rat = %d, , ip = %s",
            __FUNCTION__, apnConfig->apn_cfg[i].apn_name, apnConfig->apn_cfg[i].apn_type,
            apnConfig->apn_cfg[i].rat_ability, apnConfig->apn_cfg[i].ip_protocol);
    }
}

bool isIpv6LinkLocal(const char *ipv6Addr) {
    if (ipv6Addr) {
        struct sockaddr_in6 sa;
        int ret = inet_pton(AF_INET6, ipv6Addr, &(sa.sin6_addr));
        if (ret <= 0) {
            LOGE("[%s] inet_pton failed.", __FUNCTION__);
            return false;
        }
        if (IN6_IS_ADDR_LINKLOCAL(&sa.sin6_addr)) {
            LOGD ("isIpv6LinkLocal: true");
            return true;
        }
    }
    LOGD ("isIpv6LinkLocal: false");
    return false;
}

int isIpv6Global(const char *ipv6Addr) {
    int nRet = 0;
    if (ipv6Addr) {
        struct sockaddr_in6 sa;
        int ret = inet_pton(AF_INET6, ipv6Addr, &(sa.sin6_addr));
        if (ret <= 0) {
            LOGE("[%s] inet_pton failed.", __FUNCTION__);
            return nRet;
        }

        if (IN6_IS_ADDR_MULTICAST(&sa.sin6_addr)) {
            LOGD("multi-cast\n");
            if (IN6_IS_ADDR_MC_GLOBAL(&sa.sin6_addr)) {
                LOGD("global");
                nRet = 1;
            } else {
                LOGD("not global");
            }
        } else {
            if (IN6_IS_ADDR_LINKLOCAL(&sa.sin6_addr)) {
                LOGD ("link-local");
            } else if (IN6_IS_ADDR_SITELOCAL(&sa.sin6_addr)) {
                LOGD ("site-local");
            } else if (IN6_IS_ADDR_V4MAPPED(&sa.sin6_addr)) {
                LOGD ("v4mapped");
            } else if (IN6_IS_ADDR_V4COMPAT(&sa.sin6_addr)) {
                LOGD ("v4compat");
            } else if (IN6_IS_ADDR_LOOPBACK(&sa.sin6_addr)) {
                LOGD ("host");
            } else if (IN6_IS_ADDR_UNSPECIFIED(&sa.sin6_addr)) {
                LOGD ("unspecified");
            } else if (_IN6_IS_ULA(&sa.sin6_addr)) {
                LOGD ("uni-local");
            } else {
                LOGD ("global");
                nRet = 1;
            }
        }
    } else {
        LOGE("input ipv6 address is null!!");
    }
    return nRet;
}

/// M: Ims Data Framework
bool isLegalPdnRequest(int interfaceId, RIL_SOCKET_ID rid) {
    int i = 0;
    int targetIndex = -1;
    PdnInfo* pPdnInfo = NULL;
    bool isWifiPdnExist = false;
    bool isLtePdnExist = false;
    LOGD("[%d] isLegalPdnRequest: interfaceId %d", rid, interfaceId);

    for(; i < WIFI_MAX_PDP_NUM; i++) {
        if(pdn_info_wifi[rid][i].interfaceId == interfaceId
                && pdn_info_wifi[rid][i].active != DATA_STATE_INACTIVE) {
            targetIndex = i;
            isWifiPdnExist = true;
            pPdnInfo = &pdn_info_wifi[rid][i];
            LOGD("[%d]isLegalPdnRequest: find same ifid %d, active pdn_info_wifi[%d][%d]",
                    rid, interfaceId, rid, i);
            break;
        }
    }
    i = 0;
    for(; i < pdnInfoSize; i++) {
        if(pdn_info[rid][i].interfaceId == interfaceId
                && pdn_info[rid][i].active == DATA_STATE_ACTIVE
                && !pdn_info[rid][i].isDedicateBearer) {
            targetIndex = i;
            isLtePdnExist = true;
            pPdnInfo = &pdn_info[rid][i];
            LOGD("isLegalPdnRequest: find same ifid %d, active pdn_info[%d]",
                interfaceId, i);
            break;
        }
    }

    //HO SDC reqeust, the pdn must exist in pdn list(wifi/lte)
    if (!isWifiPdnExist && !isLtePdnExist) {
        LOGD("isLegalPdnRequest: No any pdn in wifi or lte pdn list, maybe already removed");
        return false;
    }

    return true;
}
///@}

int getModemSuggestedRetryTime(int cId, RILChannelCtx *pChannel) {
    int err = 0;
    char *cmd = NULL;
    ATResponse *p_response = NULL;
    char* line = NULL;
    int cid = INVALID_CID;
    int suggestion = NO_SUGGESTION;
    int suggestedRetryTime = NO_SUGGESTED_TIME;

    /* Use AT+EACTDELAYT=cid to query MD suggested retry timer via this cid */
    asprintf(&cmd, "AT+EACTDELAYT=%d", cId);

    // +EACTDELAYT: <cid>,<suggest action>[,<suggest delay time>]
    // <cid> : integer
    // 0-10
    //
    // <suggest action> : integer
    // 0 : do not have suggestion
    // 1 : have suggestion by previous PDN activate reject
    //
    // <suggest delay time> (seconds) : integer
    // 0~4294967295 (2^32 -1)
    err = at_send_command_singleline(cmd, "+EACTDELAYT:", &p_response, pChannel);
    free(cmd);
    if (err != 0 || p_response->success == 0 || p_response->p_intermediates == NULL) {
        LOGE("[%s] Modem isn't ready to support suggested retry timer", __FUNCTION__);
        goto error;
    } else {
        line = p_response->p_intermediates->line;

        err = at_tok_start(&line);
        if (err < 0) goto error;

        err = at_tok_nextint(&line, &cid);
        if (err < 0) goto error;

        if (cid != cId) {
            LOGE("[%s] cid%d is different to MD assigned cid%d", __FUNCTION__, cId, cid);
            goto error;
        }

        if (at_tok_hasmore(&line)) {
            err = at_tok_nextint(&line, &suggestion);
            if (err < 0) goto error;

            if ((suggestion == HAS_PDN_RETRY_TIME_SUGGESTION) && at_tok_hasmore(&line)) {
                err = at_tok_nextint(&line, &suggestedRetryTime);
                if (err < 0) goto error;
            }
        }

        LOGD("[%s] response [cid=%d, suggestion=%d, modem suggested time=%d]",
                __FUNCTION__, cid, suggestion, suggestedRetryTime);
    }

    AT_RSP_FREE(p_response);
    LOGV("[%s] finished X", __FUNCTION__);
    return suggestedRetryTime;

error:
    AT_RSP_FREE(p_response);
    LOGE("[%s] parsing error X", __FUNCTION__);
    return NO_SUGGESTED_TIME;

}

void storeModemSuggestedRetryTime(int time, MTK_RIL_Data_Call_Response_v11* response) {
    // Converts suggestedRetryTime in milliseconds (ms) if has meaningfully suggested retry time.
    int suggestedRetryTime = time > 0 ? time * 1000 : time;
    response->suggestedRetryTime = suggestedRetryTime;
}

// [C2K][IRAT] code start {@
void requestSetActivePsSimSlot(void * data, size_t datalen, RIL_Token t) {
    UNUSED(datalen);
    // AT+EACTS=x can only be sent through main protocol(protocol 1).
    RILChannelCtx* p_channel = getRILChannelCtxFromToken(t);
    int err = 0;
    int psSlot = ((int *) data)[0];
    psSlot = getActivePsSlot(psSlot);
    LOGI("[RILData_GSM_IRAT] requestSetActivePsSimSlot: psSlot = %d", psSlot);
    //Set active slot id to modem when PS SIM switch after detach.
    err = setPsActiveSlot(psSlot, p_channel);
    if (err != 0) {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    } else {
        LOGE("[RILData_GSM_IRAT] requestSetActivePsSimSlot fail!");
        RIL_onRequestComplete(t, RIL_E_MODEM_ERR, NULL, 0);
    }
}

int getActivePsSlot(int psSlot) {
    char prop_value[PROPERTY_VALUE_MAX] = { 0 };
    int actSim = 0;
    int n3gSim = 0;
    property_get(PROPERTY_3G_SIM, prop_value, "1");
    n3gSim = atoi(prop_value);
    LOGD("getActivePsSlot: psSlot = %d, 3gSim = %d", psSlot, n3gSim);

    if (n3gSim == 1) {
        actSim = psSlot;
    } else {
        if (psSlot == n3gSim) {
            actSim = 1;
        } else {
            actSim = 2;
        }
    }
    return actSim;
}

void confirmIratChange(void *data, size_t datalen, RIL_Token t) {
    UNUSED(datalen);
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    int err = -1;
    int apDecision = 0;
    char *cmd = NULL;
    ATResponse *p_response = NULL;
    apDecision = ((int *) data)[0];
    LOGD("[RILData_GSM_IRAT] confirmIratChange: apDecision=%d.", apDecision);
    // Confirm IRAT change.
    asprintf(&cmd, "AT+EI3GPPIRAT=%d", apDecision);
    err = at_send_command(cmd, &p_response, DATA_CHANNEL_CTX);
    free(cmd);
    if (isATCmdRspErr(err, p_response)) {
        LOGE("confirmIratChange failed !");
        goto error;
    }
    LOGD("[RILData_GSM_IRAT] confirmIratChange succeed -");
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);
    return;
error:
    RIL_onRequestComplete(t, RIL_E_MODEM_ERR, NULL, 0);
    at_response_free(p_response);
}

/*
 * This function is only used by c2k irat feature, return value is delivered to MD1 as a flag.
 * When irat happened, this flag need to be delivered between two modems to mark ia apn.
 */
int isInitialAttachApn(const char *requestedApn, RILChannelCtx *pChannel, int cid) {
    int isIaApn = 0;
    char iccid[PROPERTY_VALUE_MAX] = { 0 };
    char iaProperty[PROPERTY_VALUE_MAX] = { 0 };
    char apnParameter[PROPERTY_VALUE_MAX] = { 0 };
    int slotId = getMappingSIMByCurrentMode(getRILIdByChannelCtx(pChannel));
    property_get(RP_DATA_PROPERTY_IA[slotId], iaProperty, "");
    //LOGD("[RILData_GSM_IRAT]: isInitialAttachApn iaProperty=%s, slotId=%d", iaProperty, slotId);
    property_get(PROPERTY_ICCID_SIM[slotId], iccid, "");
    if (strlen(iaProperty) > 0) {
        snprintf(apnParameter, PROPERTY_VALUE_MAX, "%s,%s", iccid, requestedApn);
        //LOGD("[RILData_GSM_IRAT]: isInitialAttachApn apnParameter=%s.", apnParameter);
        if (strcmp(apnParameter, iaProperty) == 0) {
            LOGI("[RILData_GSM_IRAT]: %s is ia.", requestedApn);
            isIaApn = 1;
        }
    }
    if (isIaApn != 1 && cid == 0) {
        isIaApn = 1;
    }
    return isIaApn;
}

/*
  * The use of receive modem bar pdn urc as well as need to check if the APN already
  * established by checking the PDN info, if yes, it should set the handover flag by using
  * CGDCONT.
  */
void onBarApn(char* urc, RIL_SOCKET_ID rid) {
    int i = 0;
    int err = 0;
    char apn_ni[PROPERTY_VALUE_MAX] = {0};
    int vzw_apn_class = 0;

    LOGD("onBarApn: urc = %s.", urc);

    // +EAPNMOD: <Class>
    // Class = VzW Class APN number, e.g 3 = Internet
    err = at_tok_start(&urc);
    if (err < 0) {
        LOGE("onBarApn: error 0.");
        return;
    }

    /* Get 1st parameter: class */
    err = at_tok_nextint(&urc, &vzw_apn_class);
    if (err < 0) {
        LOGE("onBarApn: error 0");
        return;
    }

    // Mapping APN class and name.
    switch (vzw_apn_class) {
        case APN_CLASS_1:
            strncpy(apn_ni, VZW_IMS_NI, (PROPERTY_VALUE_MAX - 1));
            break;
        case APN_CLASS_2:
            strncpy(apn_ni, VZW_ADMIN_NI, (PROPERTY_VALUE_MAX - 1));
            break;
        case APN_CLASS_3:
            strncpy(apn_ni, VZW_INTERNET_NI, (PROPERTY_VALUE_MAX - 1));
            break;
        case APN_CLASS_4:
            strncpy(apn_ni, VZW_APP_NI, (PROPERTY_VALUE_MAX - 1));
            break;
        default:
            LOGE("onBarApn: APN class mapping not matched.");
            break;
    }

    LOGI("onBarApn: class = %d , apnname = %s pdnInfoSize = %d",
            vzw_apn_class, apn_ni, pdnInfoSize);

    for (i = 0; i < pdnInfoSize; i++) {
        LOGD("onBarApn: PDN info APN = %s ", pdn_info[rid][i].apn);
        if(strcmp(pdn_info[rid][i].apn, apn_ni) == 0) {
            s_isBarApnHandover = 1;
        }
    }
}

void onIratSourceStarted(int sourceRat, int targetRat, int type, RIL_SOCKET_ID rid) {
    LOGD("[RILData_GSM_IRAT] onIratSourceStarted: sourceRat = %d, targetRat = %d",
            sourceRat, targetRat);
    suspendAllDataTransmit(rid);
    onIratStateChanged(sourceRat, targetRat, IRAT_ACTION_SOURCE_STARTED, type, rid);
}

void onIratSourceFinished(int sourceRat, int targetRat, int type, RIL_SOCKET_ID rid) {
    LOGD("[RILData_GSM_IRAT] onIratSourceFinished: sourceRat = %d, targetRat = %d",
            sourceRat, targetRat);
    switch (type) {
        case IRAT_TYPE_FAILED: {
            // IRAT fail: 1. resume all interfaces.
            // 2. deactviate previous deactivate failed PDNs.
            // 3. report IRAT status.
            resumeAllDataTransmit(rid);
            int i = 0;
            for (i = 0; i < pdnInfoSize; i++) {
                if (pdn_info[rid][i].cid != INVALID_CID && pdn_info[rid][i].active == DATA_STATE_NEED_DEACT) {
                    deactivatePdnByCid(pdn_info[rid][i].cid, rid);
                }
            }
            nIratAction = IRAT_ACTION_UNKNOWN;
            break;
        }
        case IRAT_TYPE_LTE_EHRPD: {
            clearAllPdnInfo(rid);
            break;
        }
        case IRAT_TYPE_LTE_HRPD: {
            resumeAllDataTransmit(rid);
            // Down all CCMNI interfaces and clean PDN info.
            cleanupPdnsForFallback(rid);
            break;
        }
        case IRAT_TYPE_LWCG_LTE_EHRPD: {
            clearAllPdnInfo(rid);
            break;
        }
        default: {
            LOGW("[RILData_C2K_IRAT] Never should run into this case: type = %d.", type);
            break;
        }
    }
    onIratStateChanged(sourceRat, targetRat, IRAT_ACTION_SOURCE_FINISHED, type, rid);
}

void onIratTargetStarted(int sourceRat, int targetRat, int type, RIL_SOCKET_ID rid) {
    LOGD("[RILData_GSM_IRAT] onIratTargetStarted: sourceRat = %d, targetRat = %d",
            sourceRat, targetRat);
    nReactPdnCount = 0;
    nDeactPdnCount = 0;
    nReactSuccCount = 0;
    if (pSyncPdnInfo == NULL) {
        pSyncPdnInfo = calloc(1, pdnInfoSize * sizeof(SyncPdnInfo));
        if (pSyncPdnInfo == NULL) {
            LOGE("[%s] calloc for pSyncPdnInfo failed!", __FUNCTION__);
            return;
        }
        // Initialize sync PDN info list.
        clearAllSyncPdnInfo();
        LOGI("[RILData_GSM_IRAT] onIratTargetStarted init sync PDN info, size = %d.",
                pdnInfoSize);
    }
    onIratStateChanged(sourceRat, targetRat, IRAT_ACTION_TARGET_STARTED, type, rid);
}

void onIratTargetFinished(int sourceRat, int targetRat, int type, RIL_SOCKET_ID rid) {
    LOGI("[RILData_GSM_IRAT] onIratTargetFinished: sourceRat = %d, targetRat = %d, "
            "nReactPdnCount = %d, nReactSuccCount = %d", sourceRat,
            targetRat, nReactPdnCount, nReactSuccCount);
    switch (type) {
        case IRAT_TYPE_FAILED: {
            clearAllPdnInfo(rid);
            break;
        }
        case IRAT_TYPE_EHRPD_LTE: {
            // IRAT succeed.
            if (nDeactPdnCount != 0 || nReactPdnCount != nReactSuccCount) {
                handleDeactedOrFailedPdns(IRAT_NEED_RETRY, rid);
                cleanupFailedPdns(rid);
            }
            onDataCallListChanged(&s_data_ril_cntx[rid]);
            break;
        }
        case IRAT_TYPE_HRPD_LTE: {
            if (nDeactPdnCount != 0) {
                handleDeactedOrFailedPdns(IRAT_NO_RETRY, rid);
            }
            cleanupFailedPdns(rid);
            RIL_onUnsolicitedResponse(RIL_UNSOL_DATA_CALL_LIST_CHANGED, NULL, 0, rid);
            LOGD("[RILData_C2K_IRAT] fallback case finished.");
            break;
        }
        case IRAT_TYPE_LWCG_EHRPD_LTE: {
            handleLwcgFailedPdns(rid);
            cleanupFailedPdns(rid);
            onDataCallListChanged(&s_data_ril_cntx[rid]);
            break;
        }
        default: {
            LOGW("[RILData_C2K_IRAT] Never should run into this case: type = %d.",
                    type);
            break;
        }
    }
    clearAllSyncPdnInfo();
    onIratStateChanged(sourceRat, targetRat, IRAT_ACTION_TARGET_FINISHED, type, rid);
    nIratAction = IRAT_ACTION_UNKNOWN;
}

void onIratStateChanged(int sourceRat, int targetRat, int action,
        int type, RIL_SOCKET_ID rid) {
    RIL_Pdn_IratInfo *pPdnIratInfo = (RIL_Pdn_IratInfo *) alloca(
            sizeof(RIL_Pdn_IratInfo));
    memset(pPdnIratInfo, 0, sizeof(RIL_Pdn_IratInfo));
    pPdnIratInfo->sourceRat = sourceRat;
    pPdnIratInfo->targetRat = targetRat;
    pPdnIratInfo->action = action;
    pPdnIratInfo->type = type;
    LOGI("[RILData_GSM_IRAT] onIratStateChanged: sourceRat = %d, targetRat = %d, "
            "nReactPdnCount = %d, nReactSuccCount = %d", sourceRat,
            targetRat, nReactPdnCount, nReactSuccCount);
    RIL_onUnsolicitedResponse(RIL_LOCAL_GSM_UNSOL_INTER_3GPP_IRAT_STATE_CHANGE,
            pPdnIratInfo, sizeof(RIL_Pdn_IratInfo), rid);
}

void clearAllPdnInfo(RIL_SOCKET_ID rid) {
    if (pdn_info[rid] != NULL) {
        int i = 0;
        for (i = 0; i < pdnInfoSize; i++) {
            clearPdnInfo(&pdn_info[rid][i]);
        }
    }
    /// M: Ims Data Framework {@
    if (pdn_info_wifi[rid] != NULL) {
        int i = 0;
        for (i = 0; i < WIFI_MAX_PDP_NUM; i++) {
            clearPdnInfo(&pdn_info_wifi[rid][i]);
        }
    }
    /// @}
}

void clearAllSyncPdnInfo() {
    if (pSyncPdnInfo != NULL) {
        int i = 0;
        for (i = 0; i < pdnInfoSize; i++) {
            clearSyncPdnInfo(&pSyncPdnInfo[i]);
        }
    }
}

void clearSyncPdnInfo(SyncPdnInfo* info) {
    memset(info, 0, sizeof(SyncPdnInfo));
    info->interfaceId = INVALID_CID;
    info->cid = INVALID_CID;
    info->pdnStatus = IRAT_PDN_STATUS_UNKNOWN;
}

void onIratPdnReactSucc(int cid, RILChannelCtx* rilchnlctx) {
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(rilchnlctx);
    LOGI("[%d][RILData_GSM_IRAT] onIratPdnReactSucc: cid = %d, pdnStatus = %d.",
            rid, cid, pSyncPdnInfo[cid].pdnStatus);
    if (pSyncPdnInfo[cid].pdnStatus == IRAT_PDN_STATUS_SYNCED) {
        pSyncPdnInfo[cid].pdnStatus = IRAT_PDN_STATUS_REACTED;
    } else if (pSyncPdnInfo[cid].pdnStatus == IRAT_PDN_STATUS_UNKNOWN) {
        // UNKONW CID means the PDN is not sync from source RAT, usually it is unbound CID 0.
        LOGI("[RILData_GSM_IRAT] React on unknow CID%d.", cid);
        return;
    }
    if (updatePdnAddressByCid(cid, rilchnlctx) < 0) {
        goto error;
    }
    if (0 == rebindPdnToIntf(pSyncPdnInfo[cid].interfaceId, cid, rilchnlctx)) {
        goto error;
    }
    if (updateDns(rilchnlctx) < 0) {
        goto error;
    }
    nReactSuccCount++;
    reconfigureNetworkInterface(pSyncPdnInfo[cid].interfaceId, rid);
    LOGD("[RILData_GSM_IRAT] Cid%d react succeed during IRAT.", cid);

    // OP12 IMS Data
    if (isOp12Support()) {
        responseUnsolImsDataCallListToMal(rilchnlctx, pSyncPdnInfo[cid].apn,
                pSyncPdnInfo[cid].interfaceId, rid);
    }

    return;
error:
    LOGE("[RILData_GSM_IRAT] onIratPdnReactSucc error happens, cid = %d.", cid);
    // Resume data transfer for the interface in case of MD is powered off.
    // Ex: IPO power on, MD powered off and the AT command will sent fail
    // Here we need to resume the data transmit
    setNetworkTransmitState(pSyncPdnInfo[cid].interfaceId, RESUME_DATA_TRANSFER);
    return;
}

void handleDeactedOrFailedPdns(int retryFailedPdn, RIL_SOCKET_ID rid) {
    if (pSyncPdnInfo != NULL) {
        int i = 0;
        for (i = 0; i < pdnInfoSize; i++) {
            LOGD("[RILData_GSM_IRAT] handleDeactedOrFailedPdns: i = %d, cid = %d, status = %d.",
                    i, pSyncPdnInfo[i].cid, pSyncPdnInfo[i].pdnStatus);
            // Deactivate PDN which is already deactivate in source RAT.
            if (pSyncPdnInfo[i].pdnStatus == IRAT_PDN_STATUS_DEACTED) {
                deactivatePdnByCid(pSyncPdnInfo[i].cid, rid);
            }
        }
        if (retryFailedPdn) {
            for (i = 0; i < pdnInfoSize; i++) {
                LOGI("[RILData_GSM_IRAT] handleDeactedOrFailedPdns: i = %d, cid = %d, status = %d.",
                    i, pSyncPdnInfo[i].cid, pSyncPdnInfo[i].pdnStatus);
                // Retry for failed PDNs.
                if (pSyncPdnInfo[i].pdnStatus == IRAT_PDN_STATUS_SYNCED) {
                    reactFailedPdnByCid(pSyncPdnInfo[i].cid, rid);
                }
            }
        }
    }
}

void handleLwcgFailedPdns(RIL_SOCKET_ID rid) {
    int i = 0;
    for (i = 0; i < pdnInfoSize; i++) {
        LOGD("[RILData_GSM_IRAT] handleLwcgFailedPdns: i = %d, cid = %d, status = %d.",
                i, pSyncPdnInfo[i].cid, pSyncPdnInfo[i].pdnStatus);
        // Retry for failed PDNs.
        if (pSyncPdnInfo[i].pdnStatus == IRAT_PDN_STATUS_SYNCED) {
            reactFailedPdnByCid(pSyncPdnInfo[i].cid, rid);
        }
    }
}

// Retry to deactivate PDN in target RAT which is already deactivated by source RAT,
// we don't check the result even if the deactivate failed.
void deactivatePdnByCid(int cid, RIL_SOCKET_ID rid) {
    int i = 0, lastPdnCid;
    int lastPdnState = DATA_STATE_INACTIVE;
    int isEmergency = 0;
    int isSignaling = 0;
    LOGD("[%d][RILData_GSM_IRAT] deactivatePdnByCid: cid = %d.", rid, cid);
    RILChannelCtx* rilchnlctx = getChannelCtxbyProxy();
    int err = deactivateDataCall(cid, rilchnlctx);
    switch (err) {
        case CME_SUCCESS:
            break;
        case CME_L4C_CONTEXT_CONFLICT_DEACT_ALREADY_DEACTIVATED:
            LOGD("[RILData_GSM_IRAT] deactivateDataCall cid:%d already deactivated", cid);
            break;
        case CME_ERROR_NON_CME:
            LOGD("[RILData_GSM_IRAT] ignore this, cid:%d, error: %d", i, err);
            break;
        case CME_L4C_CMD_CONFLICT_C2K_AP_DEACT_RETRY_NEEDED:
            pdn_info[rid][i].active = DATA_STATE_NEED_DEACT;
            LOGD("[RILData_GSM_IRAT] receive L4C_CMD_CONFLICT_C2K_AP_DEACT_RETRY_NEEDED.");
            break;
        default:
            LOGE("[RILData_GSM_IRAT] deactivateDataCall cid:%d failed, error:%d", i, err);
            break;
    }
    if (pdn_info[rid][i].active != DATA_STATE_NEED_DEACT) {
        clearPdnInfo(&pdn_info[rid][cid]);
    }
    // Rest network interface
    configureNetworkInterface(pdn_info[rid][cid].interfaceId, DISABLE_CCMNI, rid);
}

void reactFailedPdnByCid(int cid, RIL_SOCKET_ID rid) {
    int activatedPdnNum = 0;
    int concatenatedBearerNum = 0;
    int *activatedCidList = (int *) calloc(1, pdnInfoSize * sizeof(int));
    if (activatedCidList == NULL) {
        LOGE("[%s] calloc for activatedCidList failed!", __FUNCTION__);
        return;
    }
    LOGD("[%d][RILData_GSM_IRAT] reactFailedPdnByCid: cid = %d.", rid, cid);
    RILChannelCtx* rilchnlctx = getChannelCtxbyProxy();
    if (nIratType == IRAT_TYPE_LWCG_EHRPD_LTE && cid == 0) {
        activatedPdnNum = 1;
        activatedCidList[0] = cid;
    } else if (PDN_FAILED >= activatePdn(cid, &activatedPdnNum, &concatenatedBearerNum,
                    activatedCidList, pdn_info[rid][cid].isEmergency, 0, rilchnlctx)) {
        goto error;
    }
    // activatedPdnNum should be always 1 here and dedicate bear is false.
    int i = 0;
    for (i = 0; i < activatedPdnNum; i++) {
        int cid = activatedCidList[i];
        if (pdn_info[rid][cid].isDedicateBearer) {
            LOGI("[%d][RILData_GSM_IRAT] skip update PDP address of CID%d since it is "
                    "a dedicate bearer [active=%d]", rid, cid, pdn_info[rid][cid].active);
        } else {
            if (updatePdnAddressByCid(cid, rilchnlctx) < 0) {
                LOGE("[%d][RILData_GSM_IRAT] update PDP address of CID%d failed",
                        rid, cid);
            }
            if (0 == rebindPdnToIntf(pdn_info[rid][cid].interfaceId, cid, rilchnlctx)) {
                goto error;
            }
        }
    }
    // Doesn't need to update QoS and TFT since doesn't support on IRAT
    updateDynamicParameter(rilchnlctx);
    if (updateDns(rilchnlctx) < 0) {
        LOGE("[%d][%s] updateDns failed", rid, __FUNCTION__);
        goto error;
    }
    if (DATA_STATE_INACTIVE
            == isCidActive(activatedCidList, activatedPdnNum, rilchnlctx)) {
        goto error;
    }
    // Prevent the interface down before up cause the CS unsync
    // modify param ENABLE_CCMNI -> REACT_CCMNI
    configureNetworkInterface(pdn_info[rid][cid].interfaceId, REACT_CCMNI, rid);
    // Resume data transfer for the interface.
    setNetworkTransmitState(pdn_info[rid][cid].interfaceId, RESUME_DATA_TRANSFER);
    pSyncPdnInfo[cid].pdnStatus = IRAT_PDN_STATUS_REACTED;
    if(activatedCidList != NULL) {
        free(activatedCidList);
    }
    return;
error:
    LOGD("[%d][RILData_GSM_IRAT] reactFailedPdnByCid failed - XXXXX.", rid);
    if(activatedCidList != NULL) {
        free(activatedCidList);
    }
}

void cleanupFailedPdns(RIL_SOCKET_ID rid) {
    if (pSyncPdnInfo != NULL) {
        int i = 0;
        for (i = 0; i < pdnInfoSize; i++) {
            LOGD("[%d][RILData_GSM_IRAT] cleanupFailedPdns: i = %d, cid = %d, status = %d.",
                    rid, i, pSyncPdnInfo[i].cid, pSyncPdnInfo[i].pdnStatus);
            if (pSyncPdnInfo[i].pdnStatus == IRAT_PDN_STATUS_SYNCED) {
                cleanupFailedPdnByCid(pSyncPdnInfo[i].cid, rid);
            }
        }
    }
}

void cleanupFailedPdnByCid(int cid, RIL_SOCKET_ID rid) {
    int i = 0;
    int interfaceId = pdn_info[rid][cid].interfaceId;
    LOGI("[%d][RILData_GSM_IRAT] cleanupFailedPdnByCid: cid = %d, interfaceId = %d.",
            rid, cid, interfaceId);
    // Resume data transfer and disable the CCMNI interface.
    setNetworkTransmitState(interfaceId, RESUME_DATA_TRANSFER);
    configureNetworkInterface(interfaceId, DISABLE_CCMNI, rid);
    // Clear dedicate PDN which attached to the deactivated PDN.
    for (i = 0; i < pdnInfoSize; i++) {
        if (pdn_info[rid][i].isDedicateBearer && pdn_info[rid][i].primaryCid == cid) {
            LOGI("[%d][RILData_GSM_IRAT] Clean dedicate pdn CID%d info due to "
                    "default pdn CID%d deactivated", rid, pdn_info[rid][i].cid, cid);
            clearPdnInfo(&pdn_info[rid][i]);
        }
    }
    clearPdnInfo(&pdn_info[rid][cid]);
}

void cleanupPdnsForFallback(RIL_SOCKET_ID rid) {
    if (pdn_info[rid] != NULL) {
        int i = 0;
        for (i = 0; i < pdnInfoSize; i++) {
            LOGD("[RILData_GSM_IRAT] cleanupPdnsForFallback: i = %d, cid = %d, active = %d.",
                    i, pdn_info[rid][i].cid, pdn_info[rid][i].active);
            if (pdn_info[rid][i].cid != INVALID_CID) {
                cleanupFailedPdnByCid(pdn_info[rid][i].cid, rid);
            }
        }
    }
}

void onIratEvent(void* param) {
    int err = 0;
    int action = -1;
    int sourceRat = 0;
    int targetRat = 0;
    char *out = NULL;
    TimeCallbackParam* timeCallbackParam = (TimeCallbackParam*) param;
    char* urc = (char*) (timeCallbackParam->urc);
    RIL_SOCKET_ID rid = timeCallbackParam->rid;
    // +EIGPPIRAT: <start>,<source RAT>,<target RAT>
    // 1: source rat start  2: target rat start
    // 3: source rat finish 4: target rat finish
    // RAT: 0: not specified 1:LTE 2:EHRPD 3:HRPD
    LOGD("[RILData_GSM_IRAT] onIratEvent:state = %d, urc = %s.", nIratAction, urc);
    err = at_tok_start(&urc);
    if (err < 0) {
        LOGE("[RILData_GSM_IRAT] onIratEvent error 0.");
        goto error;
    }
    /* Get 1st parameter: start */
    err = at_tok_nextint(&urc, &action);
    if (err < 0) {
        LOGE("[RILData_GSM_IRAT] onIratEvent error 1.");
        goto error;
    }
    /* Get 2nd parameter: source rat */
    err = at_tok_nextint(&urc, &sourceRat);
    if (err < 0) {
        LOGE("[RILData_GSM_IRAT] onIratEvent error 2.");
        goto error;
    }
    /* Get 3rd parameter: target rat */
    err = at_tok_nextint(&urc, &targetRat);
    if (err < 0) {
        LOGE("[RILData_GSM_IRAT] onIratEvent error 3.");
        goto error;
    }
    nIratAction = action;
    // Get IRAT type for later process.
    nIratType = getIratType(sourceRat, targetRat);
    if (action == IRAT_ACTION_SOURCE_STARTED) {
        onIratSourceStarted(sourceRat, targetRat, nIratType, rid);
    } else if (action == IRAT_ACTION_TARGET_STARTED) {
        onIratTargetStarted(sourceRat, targetRat, nIratType, rid);
    } else if (action == IRAT_ACTION_SOURCE_FINISHED) {
        onIratSourceFinished(sourceRat, targetRat, nIratType, rid);
    } else if (action == IRAT_ACTION_TARGET_FINISHED) {
        onIratTargetFinished(sourceRat, targetRat, nIratType, rid);
    }
    LOGD("[RILData_GSM_IRAT] onIratEvent [%d, %d, %d, %d]", action, rid,
            sourceRat, targetRat);
    free(timeCallbackParam->urc);
    free(timeCallbackParam);
    return;
error:
    free(timeCallbackParam->urc);
    free(timeCallbackParam);
    return;
}

void onLwcgIratEvent(void* param) {
    int err = 0;
    int action = -1;
    int sourceRat = 0;
    int targetRat = 0;
    char *out = NULL;
    TimeCallbackParam* timeCallbackParam = (TimeCallbackParam*) param;
    char* urc = (char*) (timeCallbackParam->urc);
    RIL_SOCKET_ID rid = timeCallbackParam->rid;
    char* tmp = urc;
    // +EPDNCTRANS: <action>,<source RAT>,<target RAT>
    // 1: source rat start  2: target rat start
    // 3: source rat finish 4: target rat finish
    // RAT: 1:1XRTT 2:HRPD 3:EHRPD 4:LTE 5:UMTS 6:GSM
    LOGI("[RILData_GSM_IRAT] onLwcgIratEvent:state = %d, urc = %s.", nIratAction, urc);
    err = at_tok_start(&urc);
    if (err < 0) {
        LOGE("[RILData_GSM_IRAT] onLwcgIratEvent error 0.");
        goto error;
    }
    /* Get 1st parameter: start */
    err = at_tok_nextint(&urc, &action);
    if (err < 0) {
        LOGE("[RILData_GSM_IRAT] onLwcgIratEvent error 1.");
        goto error;
    }
    /* Get 2nd parameter: source rat */
    err = at_tok_nextint(&urc, &sourceRat);
    if (err < 0) {
        LOGE("[RILData_GSM_IRAT] onLwcgIratEvent error 2.");
        goto error;
    }
    /* Get 3rd parameter: target rat */
    err = at_tok_nextint(&urc, &targetRat);
    if (err < 0) {
        LOGE("[RILData_GSM_IRAT] onLwcgIratEvent error 3.");
        goto error;
    }
    nIratAction = action;
    // Get IRAT type for later process.
    nIratType = getLwcgIratType(sourceRat, targetRat);
    if (nIratType == IRAT_TYPE_LWCG_LTE_EHRPD || nIratType == IRAT_TYPE_LWCG_EHRPD_LTE) {
        if (action == IRAT_ACTION_SOURCE_STARTED) {
            onIratSourceStarted(sourceRat, targetRat, nIratType, rid);
        } else if (action == IRAT_ACTION_TARGET_STARTED) {
            onIratTargetStarted(sourceRat, targetRat, nIratType, rid);
        } else if (action == IRAT_ACTION_SOURCE_FINISHED) {
            onIratSourceFinished(sourceRat, targetRat, nIratType, rid);
        } else if (action == IRAT_ACTION_TARGET_FINISHED) {
            onIratTargetFinished(sourceRat, targetRat, nIratType, rid);
        }
    }
    LOGI("[RILData_GSM_IRAT] onLwcgIratEvent [%d, %d, %d, %d]", action, rid,
            sourceRat, targetRat);
    free(timeCallbackParam->urc);
    free(timeCallbackParam);
    return;
error:
    free(timeCallbackParam->urc);
    free(timeCallbackParam);
    return;
}

int onPdnSyncFromSourceRat(void* param) {
    int err = 0;
    int cid = INVALID_CID;
    int interfaceId = INVALID_CID;
    int pdnStatus = IRAT_PDN_STATUS_UNKNOWN;
    char *out = NULL;
    TimeCallbackParam* timeCallbackParam = (TimeCallbackParam*) param;
    char* urc = (char*) (timeCallbackParam->urc);
    RIL_SOCKET_ID rid = timeCallbackParam->rid;
    // +EGCONTRDP: <cid>,<APN_name>,<interface id>,<need_deact>,<PDP_addr_1>, [<PDP_addr2>]
    LOGD("[%d][RILData_GSM_IRAT] onPdnSyncFromSourceRat: action = %d, urc = %s",
            rid, nIratAction, urc);
    if (nIratAction != IRAT_ACTION_TARGET_STARTED) {
        LOGD("[%d][RILData_GSM_IRAT] Ignore onPdnSyncFromSourceRat.", rid);
        return -1;
    }
    err = at_tok_start(&urc);
    if (err < 0) {
        LOGE("[%d][RILData_GSM_IRAT] onPdnSyncFromSourceRat error 0.", rid);
        goto error;
    }
    /* Get 1st parameter: CID */
    err = at_tok_nextint(&urc, &cid);
    if (err < 0) {
        LOGE("[%d][RILData_GSM_IRAT] onPdnSyncFromSourceRat error 1.", rid);
        goto error;
    }
    pdn_info[rid][cid].cid = cid;
    pSyncPdnInfo[cid].cid = cid;
    /* Get 2nd parameter: apn name */
    err = at_tok_nextstr(&urc, &out);
    if (err < 0) {
        LOGE("[%d][RILData_GSM_IRAT] onPdnSyncFromSourceRat error 2.", rid);
        goto error;
    }
    snprintf(pdn_info[rid][cid].apn, sizeof(pdn_info[rid][cid].apn), "%s", out);
    snprintf(pSyncPdnInfo[cid].apn, sizeof(pSyncPdnInfo[cid].apn), "%s", out);
    /* Get 3rd parameter: interface id */
    err = at_tok_nextint(&urc, &interfaceId);
    if (err < 0) {
        LOGE("[%d][RILData_GSM_IRAT] onPdnSyncFromSourceRat error 3.", rid);
        goto error;
    }
    // -1 because we +1 when bind data using CGDATA
    pdn_info[rid][cid].interfaceId = interfaceId - 1;
    pSyncPdnInfo[cid].interfaceId = interfaceId - 1;
    /* Get 4th parameter: interface id */
    err = at_tok_nextint(&urc, &pdnStatus);
    if (err < 0) {
        LOGE("[%d][RILData_GSM_IRAT] onPdnSyncFromSourceRat error 4.", rid);
        goto error;
    }
    pSyncPdnInfo[cid].pdnStatus = pdnStatus;
    /* Get 5th parameter: IPAddr1 */
    err = at_tok_nextstr(&urc, &out);
    if (err < 0) {
        LOGE("[%d][RILData_GSM_IRAT] onPdnSyncFromSourceRat error 5.", rid);
        goto error;
    }
    if (get_address_type(out) == IPV4) {
        snprintf(pSyncPdnInfo[cid].addressV4, sizeof(pSyncPdnInfo[cid].addressV4), "%s", out);
    } else {
        convert_ipv6_address(out, pSyncPdnInfo[cid].addressV6, 1);
    }
    /* Only for IPv4V6 + dual stack PDP context */
    /* Get 6th paramter: IPaddr 2*/
    if (at_tok_hasmore(&urc)) {
        err = at_tok_nextstr(&urc, &out);
        if (err < 0) {
            LOGE("[%d][RILData_GSM_IRAT] onPdnSyncFromSourceRat error 6.", rid);
            goto error;
        }
        if (get_address_type(out) == IPV4) {
            snprintf(pSyncPdnInfo[cid].addressV4, sizeof(pSyncPdnInfo[cid].addressV4), "%s", out);
        } else {
            convert_ipv6_address(out, pSyncPdnInfo[cid].addressV6, 1);
        }
    }
    pdn_info[rid][cid].primaryCid= cid;
    pdn_info[rid][cid].isDedicateBearer = 0;
    pdn_info[rid][cid].isEmergency = 0;
    pdn_info[rid][cid].rat = RDS_RAN_MOBILE_3GPP;

    if (nIratType != IRAT_TYPE_LWCG_EHRPD_LTE || cid != 0) {
        pdn_info[rid][cid].active = DATA_STATE_INACTIVE;
    }
    if (pSyncPdnInfo[cid].pdnStatus == IRAT_PDN_STATUS_DEACTED) {
        nDeactPdnCount++;
    }
    nReactPdnCount++;
    LOGI("[%d][RILData_GSM_IRAT] onPdnSyncFromSourceRat: cid = %d, interfaceId = %d, "
                "iRatPdnStatus = %d, pdnCount = %d, apn = %s.",
            rid, pSyncPdnInfo[cid].cid, pSyncPdnInfo[cid].interfaceId, pSyncPdnInfo[cid].pdnStatus,
            nReactPdnCount, pdn_info[rid][cid].apn);
    free(timeCallbackParam->urc);
    free(timeCallbackParam);
    return 0;
error:
    free(timeCallbackParam->urc);
    free(timeCallbackParam);
    return -1;
}

// Get network interface name, always return ccmni with/without IRAT feature support.
char* getNetworkInterfaceName(int interfaceId) {
    char* ret = "";
    ret = ccci_get_node_name(USR_NET_0 + interfaceId, MD_SYS1);
    //LOGD("getNetworkInterfaceName: interface %d's name = %s", interfaceId, ret);
    return ret;
}

// Bind CCMNI interface with specified modem.
void bindNetworkInterfaceWithModem(int interfaceId, int modemId) {
    LOGD("[RILData_GSM_IRAT] bindNetworkInterfaceWithModem interface %d to modem %d", interfaceId,
            modemId);
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(struct ifreq));
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", getNetworkInterfaceName(interfaceId));
    ifc_ccmni_md_cfg(ifr.ifr_name, modemId);
}

int rebindPdnToIntf(int interfaceId, int cid, RILChannelCtx * pChannel) {
    int nRet = 1;  // 0:failed, 1: success
    char *cmd = NULL;
    int err = 0;
    ATResponse *p_response = NULL;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(pChannel);
    LOGD("[%d][RILData_GSM_IRAT] rebindPdnToIntf: cid = %d, interfaceId = %d E",
            rid, cid, interfaceId);
    /* AT+CGDATA=<L2P>,<cid>,<channel ID> */
    asprintf(&cmd, "AT+CGDATA=\"%s\",%d,%d", s_l2p_value, cid, interfaceId + 1);  // The channel id is from 1~n
    if (cmd != NULL) {
        err = at_send_command(cmd, &p_response, pChannel);
        free(cmd);
    } else {
        LOGE("[%d][RILData_GSM_IRAT] rebindPdnToIntf: cmd alloc fail!", rid);
        goto error;
    }
    if (isATCmdRspErr(err, p_response)) {
        LOGE("[%d][RILData_GSM_IRAT] rebindPdnToIntf: CID%d fail to bind interface%d",
                rid, cid, interfaceId);
        nRet = 0;
        goto error;
    } else {
        // The PDP is bind to a network interface, set to active
        LOGD("[%d][RILData_GSM_IRAT] rebindPdnToIntf: CID%d is bind to interface%d",
            rid, cid, interfaceId);
        pdn_info[rid][cid].interfaceId = interfaceId;
        pdn_info[rid][cid].active = DATA_STATE_ACTIVE;
    }
    LOGD("[%d][RILData_GSM_IRAT] rebindPdnToIntf: finished X", rid);
    AT_RSP_FREE(p_response);
    return nRet;
error:
    AT_RSP_FREE(p_response);
    return nRet;
}

// Reconfigure CCMNI interface, the sequence need to be in order.
// 1. Bind CCMNI interface with current MD id.
// 2. Reset IP stack if IP changed.
// 3. Resume data transfer for the interface.
void reconfigureNetworkInterface(int interfaceId, RIL_SOCKET_ID rid) {
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(struct ifreq));
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", getNetworkInterfaceName(interfaceId));
    // Bind CCMNI interface with current MD id.
    ifc_ccmni_md_cfg(ifr.ifr_name, MD_SYS1);
    LOGI("[%d][RILData_GSM_IRAT] reconfigureNetworkInterface: id = %d, name = %s.",
            rid, interfaceId, ifr.ifr_name);
    int resetMask = 0;
    int i = 0;
    for (i = 0; i < pdnInfoSize; i++) {
        if (pdn_info[rid][i].interfaceId == interfaceId) {
            if (strlen(pSyncPdnInfo[i].addressV4) > 0) {
                if (strcmp(pSyncPdnInfo[i].addressV4, pdn_info[rid][i].addressV4) != 0) {
                    resetMask |= 0x01;
                }
            }
            if (strlen(pSyncPdnInfo[i].addressV6) > 0) {
                if (strcmp(pSyncPdnInfo[i].addressV6, pdn_info[rid][i].addressV6) != 0) {
                    resetMask |= 0x02;
                } else {
                    // IPv6 prefix may change even if the interface-id is not changed
                    resetMask |= 0x04;
                }
            }
        }
    }
    LOGD("[%d][RILData_GSM_IRAT] reconfigureNetworkInterface resetMask = %2x.", rid, resetMask);
    // Reset IP stack if IP changed.
    if (resetMask != 0) {
        updateNetworkInterface(interfaceId, resetMask, rid);
    }
    // Resume data transfer for the interface.
    setNetworkTransmitState(interfaceId, RESUME_DATA_TRANSFER);
}

void resumeAllDataTransmit(RIL_SOCKET_ID rid) {
    LOGD("[RILData_GSM_IRAT] resumeAllDataTransmit...");
    int i = 0;
    for (i = 0; i < pdnInfoSize; i++) {
        if (pdn_info[rid][i].active == DATA_STATE_ACTIVE) {
            setNetworkTransmitState(pdn_info[rid][i].interfaceId, RESUME_DATA_TRANSFER);
        }
    }
}

void suspendAllDataTransmit(RIL_SOCKET_ID rid) {
    LOGD("[RILData_GSM_IRAT] suspendAllDataTransmit...");
    int i = 0;
    for (i = 0; i < pdnInfoSize; i++) {
        if (pdn_info[rid][i].active == DATA_STATE_ACTIVE) {
            setNetworkTransmitState(pdn_info[rid][i].interfaceId, SUSPEND_DATA_TRANSFER);
        }
    }
}

void setNetworkTransmitState(int interfaceId, int state) {
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(struct ifreq));
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", getNetworkInterfaceName(interfaceId));
    LOGD("[RILData_GSM_IRAT] setNetworkTransmitState: id = %d, name= %s, state = %d.",
            interfaceId, ifr.ifr_name, state);
    ifc_set_txq_state(ifr.ifr_name, state);
}

int getIratType(int sourceRat, int targetRat) {
    int iratType = IRAT_TYPE_UNKNOWN;
    if (sourceRat == RAT_LTE && targetRat == RAT_EHRPD) {
        iratType = IRAT_TYPE_LTE_EHRPD;
    } else if (sourceRat == RAT_LTE && (targetRat == RAT_HRPD || targetRat == RAT_1XRTT)) {
        iratType = IRAT_TYPE_LTE_HRPD;
    } else if (sourceRat == RAT_EHRPD && targetRat == RAT_LTE) {
        iratType = IRAT_TYPE_EHRPD_LTE;
    } else if ((sourceRat == RAT_HRPD || sourceRat == RAT_1XRTT) && targetRat == RAT_LTE) {
        iratType = IRAT_TYPE_HRPD_LTE;
    } else if (sourceRat == targetRat) {
        iratType = IRAT_TYPE_FAILED;
    }
    LOGD("[RILData_C2K_IRAT] getIratType: iratType = %d.", iratType);
    return iratType;
}

int getLwcgIratType(int sourceRat, int targetRat) {
    int iratType = IRAT_TYPE_UNKNOWN;
    if (sourceRat == RAT_LTE && targetRat == RAT_EHRPD) {
        iratType = IRAT_TYPE_LWCG_LTE_EHRPD;
    } else if (sourceRat == RAT_LTE && (targetRat == RAT_HRPD || targetRat == RAT_1XRTT)) {
        iratType = IRAT_TYPE_LWCG_LTE_HRPD;
    } else if (sourceRat == RAT_EHRPD && targetRat == RAT_LTE) {
        iratType = IRAT_TYPE_LWCG_EHRPD_LTE;
    } else if ((sourceRat == RAT_HRPD || sourceRat == RAT_1XRTT) && targetRat == RAT_LTE) {
        iratType = IRAT_TYPE_LWCG_HRPD_LTE;
    }
    LOGD("[RILData_C2K_IRAT] getLwcgIratType: iratType = %d.", iratType);
    return iratType;
}

void onUpdateIratStatus(RIL_SOCKET_ID rid) {
    nIratDeactCount++;
    LOGD("[onUpdateIratStatus] rid = %d", rid);
    RILChannelCtx* pDataChannel = getChannelCtxbyProxy();
    RIL_requestProxyTimedCallback(onResetIratStatus, NULL,
            &TIMEVAL_10, pDataChannel->id ,"onResetIratStatus");
}

void onResetIratStatus() {
    LOGI("[onResetIratStatus] nIratDeactCount = %d, nIratAction = %d", nIratDeactCount, nIratAction);
    nIratDeactCount--;
    // during IRAT multiple PDN DEACT may be received, reset by the last one.
    // we suppose the time snap between two DEACT is less than 10s, this should be enough.
    // Add nIratAction = source start/finish check to avoid the two IRAT events happen within 10s
    // Ex: 3G->4G ---(<10s)--- 4G -> 3G, the nIratAction should not be reset in this case
    if ((nIratDeactCount == 0) &&
        (nIratAction == IRAT_ACTION_SOURCE_STARTED || nIratAction == IRAT_ACTION_SOURCE_FINISHED)) {
        nIratAction = IRAT_ACTION_UNKNOWN;
    }
}

/// M: Ims Data Framework {@
int getImsParamInfo(RIL_SOCKET_ID rid, int intfId,
        RIL_Default_Bearer_VA_Config_Struct * pImsParam) {
    // interface id for MAL, IMS: 5, EIMS: 6
    // 0: Success;  -1: Fail
    int ret = 0;

    if (isImsSupport() && pImsParam != NULL) {
        if (sizeof(mal_datamngr_ims_info_rsp_t) == sizeof(RIL_Default_Bearer_VA_Config_Struct)) {
            mal_datamngr_ims_info_req_ptr_t req_ptr =
                    calloc(1, sizeof(mal_datamngr_ims_info_req_t));
            assert(req_ptr != NULL);
            req_ptr->network_id = intfId;

            mal_datamngr_ims_info_rsp_ptr_t rsp_ptr =
                    (mal_datamngr_ims_info_rsp_ptr_t) pImsParam;
            int sim_id = getValidSimId(rid, __FUNCTION__);
            if (sim_id != SIM_ID_INVALID) {
                if (mal_datamngr_get_ims_info_tlv != NULL) {
                    ret = mal_datamngr_get_ims_info_tlv (mal_once(1, mal_cfg_type_sim_id, sim_id),
                            req_ptr, rsp_ptr, 0, NULL, NULL, NULL);
                    LOGD("[%s] Call mal_datamngr_get_ims_info_tlv success", __FUNCTION__);
                } else if (mal_datamngr_get_ims_info != NULL){
                    ret = mal_datamngr_get_ims_info(
                        mal_once(1, mal_cfg_type_sim_id, sim_id),
                        req_ptr, rsp_ptr);
                    LOGD("[%s] Call mal_datamngr_get_ims_info success", __FUNCTION__);
                } else {
                    LOGD("[%s] mal_datamngr_get_ims_info and mal_datamngr_get_ims_info_tlv are null", __FUNCTION__);
                }
                // TODO: Handle response code from MAL
            } else {
                LOGE("[%d][%s] Skip with invalid SIM ID: %d", rid, __FUNCTION__, sim_id);
            }
            FREEIF(req_ptr);
        } else {
            LOGE("[%d][%s] mismatch structure between RILD and MAL!", rid, __FUNCTION__);
            ret = -1;
        }
    }

    return ret;
}

void responseUnsolDataCallRspToMal(RIL_Token t, const mal_datamngr_data_call_info_req_t *pRespData,
    size_t datalen) {
    UNUSED(datalen);
    // Only response this duplicate response to MAL while requst is not from MAL
    if (isEpdgSupport()) {
        RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));

        if (pRespData) {
            LOGD("[%d][%s] eran_type:%d, cause:%d", rid, __FUNCTION__,
                    pRespData->eran_type, pRespData->status);
        } else {
            LOGD("[%d][%s] error, respData: Null", rid, __FUNCTION__);
        }

        mal_datamngr_data_call_info_req_ptr_t req_ptr =
                (mal_datamngr_data_call_info_req_ptr_t) pRespData;
        mal_datamngr_data_call_info_rsp_ptr_t rsp_ptr =
                calloc(1, sizeof(mal_datamngr_data_call_info_rsp_t));
        assert(rsp_ptr != NULL);
        // Let's forward it!
        int sim_id = getValidSimId(rid, __FUNCTION__);
        if (sim_id != SIM_ID_INVALID) {
            if (mal_datamngr_set_data_call_info_tlv != NULL) {
                mal_datamngr_set_data_call_info_tlv (mal_once(1, mal_cfg_type_sim_id, sim_id),
                        req_ptr, rsp_ptr, 0, NULL, NULL, NULL);
                LOGD("[%s] Call mal_datamngr_set_data_call_info_tlv success", __FUNCTION__);
            } else if (mal_datamngr_set_data_call_info != NULL){
                mal_datamngr_set_data_call_info(
                    mal_once(1, mal_cfg_type_sim_id, sim_id), // The control params.
                    req_ptr, // Data call response to MAL.
                    rsp_ptr); // Response from MAL.
                LOGD("[%s] Call mal_datamngr_set_data_call_info success", __FUNCTION__);
            } else {
                LOGE("[%s] mal_datamngr_set_data_call_info and mal_datamngr_set_data_call_info_tlv are null", __FUNCTION__);
            }
            // TODO: Handle response code from MAL.
        }
        FREEIF(rsp_ptr);
    }
}

void responseUnsolImsDataCallListToMal(RILChannelCtx* rilchnlctx,
        char *apn, int interfaceId, int rid) {
    LOGD("%s apn: %s, interface: %d, rid: %d", __FUNCTION__, apn, interfaceId, rid);

    if ((NULL != apn) && (NULL != strcasestr(apn, "ims"))) {
        int sim_id = getValidSimId(rid, __FUNCTION__);
        MTK_RIL_Data_Call_Response_v11* response = (MTK_RIL_Data_Call_Response_v11*)
                calloc(1, sizeof(MTK_RIL_Data_Call_Response_v11));
        mal_datamngr_data_call_info_req_t* responseToMal = (mal_datamngr_data_call_info_req_t*) calloc(1,
                sizeof(mal_datamngr_data_call_info_req_t));
        assert(response != NULL);
        assert(responseToMal != NULL);
        if (response != NULL && sim_id != SIM_ID_INVALID) {
            initialDataCallResponse(response, 1);
            initialMalDataCallResponse(responseToMal, 1);
            updateDynamicParameter(rilchnlctx);
            int cid = createDataResponse(interfaceId, IPV4V6, response, rid);
            createMalDataResponse(interfaceId, cid, response, responseToMal, rid);
            dumpDataResponse(response, "responseUnsolImsDataCallListToMal dump response");
            dumpMalDataResponse(responseToMal, "responseUnsolImsDataCallListToMal dump ims response");

            mal_datamngr_data_call_info_req_ptr_t req_ptr =
                    (mal_datamngr_data_call_info_req_ptr_t) responseToMal;
            mal_datamngr_data_call_info_rsp_ptr_t rsp_ptr =
                    calloc(1, sizeof(mal_datamngr_data_call_info_rsp_t));
            assert(rsp_ptr != NULL);

            if (mal_datamngr_notify_data_call_list_tlv != NULL) {
                mal_datamngr_notify_data_call_list_tlv (mal_once(1, mal_cfg_type_sim_id, sim_id), 1,
                        (mal_datamngr_data_call_info_req_ptr_t *) req_ptr,
                        (mal_datamngr_data_call_info_rsp_ptr_t *) rsp_ptr,
                        NULL, NULL, NULL, NULL);
            } else {
                LOGE("[%s] mal_datamngr_notify_data_call_list_tlv is null", __FUNCTION__);
            }
            FREEIF(rsp_ptr);
        } else {
            LOGE("[%s] allocate response space fail", __FUNCTION__);
        }

        if (response != NULL) {
            freeDataResponse(response);
            free(response);
        }
        if (responseToMal != NULL) {
            freeMalDataResponse(responseToMal);
            free(responseToMal);
        }
    }
}

void configEpdg(epdgConfig_t *pConfig, const RIL_Default_Bearer_VA_Config_Struct * pDefaultBearerConfig) {
    if (pConfig == NULL) {
        LOGE("configEpdg, ERROR: pConfig is NULL");
        return;
    }

    if (isEpdgSupport() && pDefaultBearerConfig && pConfig  && isImsSupport()) {
        pConfig->isHandOver = pDefaultBearerConfig->isHandover;
        pConfig->eranType = pDefaultBearerConfig->assigned_rate;
    } else {
        pConfig->eranType = RDS_RAN_MOBILE_3GPP;
    }

    if (pConfig) {
        LOGD("epdgConfig, isHandOver: %d, eran_type: %d", pConfig->isHandOver, pConfig->eranType);
    }
}

void setRspEranType(int eran_type, MTK_RIL_Data_Call_Response_v11 *pRspData,
        mal_datamngr_data_call_info_req_t* responseToMal, RIL_SOCKET_ID rid)
{
    if (isEpdgSupport() && responseToMal != NULL) {
        int cid = responseToMal->defaultBearer.cid;
        if (cid >= WIFI_CID_OFFSET) {
            pdn_info_wifi[rid][cid - WIFI_CID_OFFSET].rat = eran_type;
        } else {
            pdn_info[rid][cid].rat = eran_type;
        }
        pRspData->rat = eran_type;
        responseToMal->eran_type = eran_type;
    }
}

void cpStr(char *pszDst, const char* pszStr, int strSize, int bufferSize) {
    int cpStrSize = (bufferSize > strSize) ? strSize: bufferSize - 1;
    strncpy(pszDst, pszStr, cpStrSize);
    pszDst[cpStrSize] = '\0';
}

int queryEpdgRat(RIL_SOCKET_ID rid, reqSetupConf_t *pConfig, RIL_Default_Bearer_VA_Config_Struct* pDefaultBearerVaConfig)
{
    int nRet = PDN_SETUP_THRU_MOBILE;
    LOGD("[%d][%s] X", rid, __FUNCTION__);

    if ((!isEpdgSupport() && isImsSupport()) || !isSysEpdgSupport()) {
        LOGD("[%s] EPDG is not supported", __FUNCTION__);
        return nRet;
    }

    if ((!isMultipleImsSupport()) && (getMainProtocolRid() != rid)) {
        LOGD("[%d][%s] EPDG is not supported on non-main protocol, return directly",
                rid, __FUNCTION__);
        return nRet;
    }

    nRet = PDN_SETUP_THRU_ERR;
    dm_req_setup_data_call_t queryRatReq = {{0},{0},{0},{0},{0},0,0,0,0,0,{0,0,0,0,0},0,0,0,0,0,0};

    cpStr(queryRatReq.radioType, pConfig->radioType, strlen(pConfig->radioType), STRING_SIZE);
    cpStr(queryRatReq.apn, pConfig->requestedApn, strlen(pConfig->requestedApn), STRING_SIZE);
    cpStr(queryRatReq.username, pConfig->username, strlen(pConfig->username), STRING_SIZE);
    cpStr(queryRatReq.passwd, pConfig->password, strlen(pConfig->password), STRING_SIZE);
    cpStr(queryRatReq.profile, pConfig->profile, strlen(pConfig->profile), STRING_SIZE);

    queryRatReq.authType = pConfig->authType;
    queryRatReq.protocol = pConfig->protocol;
    queryRatReq.interfaceId = pConfig->interfaceId;
    queryRatReq.retryCount = pConfig->retryCount;  // start from 1

    void *ptrData = (void *) &queryRatReq.mIsValid;
    memcpy(ptrData, pDefaultBearerVaConfig, sizeof(RIL_Default_Bearer_VA_Config_Struct));

    if (pDefaultBearerVaConfig->emergency_ind == 1) {
        queryRatReq.ran_type = pDefaultBearerVaConfig->assigned_rate;
    } else {
        queryRatReq.ran_type = RDS_RAN_UNSPEC;
    }

    int sim_id = getValidSimId(rid, __FUNCTION__);
    if (sim_id == SIM_ID_INVALID) {
        LOGE("[%d][%s] Invalid SIM ID %d, return directly!", rid, __FUNCTION__, sim_id);
        goto error;
    }
    queryRatReq.sim_id = sim_id;

    // don't free this here, cause rsp need to be used from outside the function
    if (pConfig->pQueryRatRsp == NULL) {
        pConfig->pQueryRatRsp = (queryRatRsp_t *) calloc(1, sizeof(queryRatRsp_t));
    }
    assert(pConfig->pQueryRatRsp != NULL);
    queryRatRsp_t *pQueryRatRsp = (queryRatRsp_t *) (pConfig->pQueryRatRsp);
    if (NULL == pQueryRatRsp) {
        nRet = PDN_SETUP_THRU_MOBILE;
        LOGE("[%s] can't allocate mem", __FUNCTION__);
        goto error;
    }

    while (queryRatReq.retryCount <= PDN_SETUP_THRU_MAX) {
        memset(pQueryRatRsp, 0, sizeof(queryRatRsp_t));

        rds_bool nReqRet = RDS_UNDEF;
        if (rild_rds_sdc_req != NULL) {
            nReqRet = rild_rds_sdc_req(&queryRatReq, pQueryRatRsp);
            LOGD("[%s] Call rild_rds_sdc_req success", __FUNCTION__);
        } else {
            LOGE("[%s] rild_rds_sdc_req is null", __FUNCTION__);
        }
        if (nReqRet != RDS_TRUE) {
            LOGE("[%d][%s] query rat return failed!!", rid, __FUNCTION__);
            break;
        } else {
            LOGD("[%d][%s] rsp status: %d, rsp rat: %d, request rat: %d", rid,
                    __FUNCTION__, pQueryRatRsp->ril_status,
                    pQueryRatRsp->resp.eran_type, pDefaultBearerVaConfig->assigned_rate);
            if (pQueryRatRsp->ril_status == RIL_E_SUCCESS) {
                if (pQueryRatRsp->resp.eran_type == RDS_RAN_MOBILE_3GPP ||
                    pQueryRatRsp->resp.eran_type == RDS_RAN_MOBILE_3GPP2) {
                    LOGD("[%d][%s] setup thru mobile, islteonly:%d",
                            rid,__FUNCTION__, pQueryRatRsp->resp.islteonly);
                    nRet = PDN_SETUP_THRU_MOBILE;
                    pDefaultBearerVaConfig->assigned_rate = pQueryRatRsp->resp.eran_type;
                } else if (pQueryRatRsp->resp.eran_type == RDS_RAN_WIFI) {
                    LOGD("[%d][%s] setup thru wifi with rsp", rid, __FUNCTION__);
                    nRet = PDN_SETUP_THRU_WIFI;
                    pDefaultBearerVaConfig->assigned_rate = pQueryRatRsp->resp.eran_type;
                } else {
                    LOGD("[%d][%s] queryRatInfoResp error: ran type: %d",
                            rid, __FUNCTION__, pQueryRatRsp->resp.eran_type);
                    pDefaultBearerVaConfig->assigned_rate = RDS_RAN_UNSPEC;
                }
                break;
            } else {
                if (pQueryRatRsp->resp.eran_type == RDS_RAN_MOBILE_3GPP ||
                    pQueryRatRsp->resp.eran_type == RDS_RAN_MOBILE_3GPP2) {
                    LOGD("[%d][%s] setup thru mobile failed, then try next rat",
                            rid, __FUNCTION__);
                    queryRatReq.retryCount++;
                } else if (pQueryRatRsp->resp.eran_type == RDS_RAN_WIFI) {
                    LOGD("[%d][%s] setup thru wifi failed, then try next rat",
                            rid, __FUNCTION__);
                    queryRatReq.retryCount++;
                } else {
                    LOGD("[%d][%s] rsp eran type is unknown, return failed", rid, __FUNCTION__);
                    nRet = PDN_SETUP_THRU_ERR;
                    break;
                }
            }
        }
    }
    if (queryRatReq.retryCount > PDN_SETUP_THRU_MAX) {
        LOGD("[%d][%s] try all the RATs failed", rid, __FUNCTION__);
        nRet = PDN_SETUP_THRU_ERR;
    }

error:
    LOGD("[%d][%s] ret: %d E", rid, __FUNCTION__, nRet);
    return nRet;
}

int deactivatePdnThruEpdg(RIL_SOCKET_ID rid, int interfaceId, int reason)
{
    int nRet = 0;

    if (isEpdgSupport() && isImsSupport()) {
        epdgDeactReq_t epdgDeactReq = {0, 0, 0};
        epdgDeactRsp_t epdgDeactRsp = {0, 0, 0};
        epdgDeactReq.id = interfaceId;
        epdgDeactReq.cause = reason;

        int sim_id = getValidSimId(rid, __FUNCTION__);
        if (sim_id != SIM_ID_INVALID) {
            epdgDeactReq.sim_id = sim_id;
            rds_bool nRdsRet = RDS_UNDEF;
            if (rild_rds_ddc_req != NULL) {
                nRdsRet = rild_rds_ddc_req(&epdgDeactReq, &epdgDeactRsp);
                LOGD("[%s] Call rild_rds_ddc_req success", __FUNCTION__);
            } else {
                LOGE("[%s] rild_rds_ddc_req is null", __FUNCTION__);
            }
            LOGD("[%d][%s] nRdsRet: %d, status: %d, eran_type: %d", rid, __FUNCTION__,
                    nRdsRet, epdgDeactRsp.status, epdgDeactRsp.eran_type);

            if (nRdsRet == RDS_TRUE) {
                if (epdgDeactRsp.status == RIL_E_SUCCESS) {
                    if (epdgDeactRsp.eran_type == RDS_RAN_MOBILE_3GPP ||
                        epdgDeactRsp.eran_type == RDS_RAN_MOBILE_3GPP2) {
                        LOGD("still deactivate thru mobile");
                    } else if (epdgDeactRsp.eran_type == RDS_RAN_WIFI) {
                        LOGD("deactivate thru wifi-off load");
                        nRet = 1;
                    }
                }
            }
        } else {
            LOGE("[%d][%s] Invalid SIM ID %d, return directly!", rid, __FUNCTION__, sim_id);
        }
    }
    LOGD("[%d][%s] nRet: %d", rid, __FUNCTION__, nRet);
    return nRet;
}

bool isPdnHoStart(int hoStatus) {
    return (hoStatus == HO_START) ? true: false;
}

bool isPdnHoStartFromRds(RIL_SOCKET_ID rid, PdnInfo *pInfo) {
    bool bRet = false;
    rds_rb_get_ho_status_t* ho_status = NULL;
    do {
        if (g_rds_conn_ptr == NULL){
            LOGE("[%s] g_rds_conn_ptr is null", __FUNCTION__);
            break;
        }
        ho_status = (rds_rb_get_ho_status_t*)calloc(1, sizeof(rds_rb_get_ho_status_t));
        assert(ho_status != NULL);
        int sim_id = getValidSimId(rid, __FUNCTION__);
        if (sim_id == SIM_ID_INVALID) {
            LOGD("[%d][%s] Invalid SIM ID %d", rid, __FUNCTION__, sim_id);
            break;
        }
        rds_rb_get_ho_status_req_t req = {0, 0};
        req.ifid = pInfo->interfaceId;
        req.ucsim_id = sim_id;
        rds_bool nRet = RDS_UNDEF;
        if (rds_rild_get_ho_status != NULL) {
            nRet = rds_rild_get_ho_status(ho_status, &req, g_rds_conn_ptr);
            LOGD("[%s] Call rds_rild_get_ho_status success", __FUNCTION__);
        } else {
            LOGE("[%s] rds_rild_get_ho_status is null", __FUNCTION__);
        }
        if(nRet == RDS_FALSE){
            LOGD("[%s] get Ho status from Rds fail ", __FUNCTION__);
            break;
        }
        LOGD("[%d][%s] Interface%d from Rds_ho_start_cnt = %d pInfo->ho_start_cnt = %d",
                rid, __FUNCTION__, pInfo->interfaceId, ho_status->start_key, pInfo->ho_start_cnt);
        if ((int)ho_status->start_key != pInfo->ho_start_cnt) {
            bRet = true;
            break;
        } else {
            LOGD("[%d][%s] pInfo->ho_stop_cnt = %d", rid, __FUNCTION__, pInfo->ho_stop_cnt);
            if (pInfo->ho_start_cnt != pInfo->ho_stop_cnt) {
                bRet = true;
                break;
            }
        }
    } while(false);
    FREEIF(ho_status);
    return bRet;
}

bool getHoRefCountFromRds(RIL_SOCKET_ID rid, PdnInfo *pInfo) {
    bool bRet = false;
    rds_rb_get_ho_status_t* ho_status = NULL;
    do {
        if (g_rds_conn_ptr == NULL) {
            LOGE("[%s] g_rds_conn_ptr is null", __FUNCTION__);
            break;
        }
        ho_status = (rds_rb_get_ho_status_t*)calloc(1, sizeof(rds_rb_get_ho_status_t));
        assert(ho_status != NULL);

        int sim_id = getValidSimId(rid, __FUNCTION__);
        if (sim_id == SIM_ID_INVALID) {
            LOGE("[%d][%s] Invalid SIM ID %d", rid, __FUNCTION__, sim_id);
            break;
        }

        rds_rb_get_ho_status_req_t req = {0, 0};
        req.ifid = pInfo->interfaceId;
        req.ucsim_id = sim_id;
        rds_bool nRet = RDS_UNDEF;
        if (rds_rild_get_ho_status != NULL) {
            nRet = rds_rild_get_ho_status(ho_status, &req, g_rds_conn_ptr);
            LOGD("[%s] Call rds_rild_get_ho_status success", __FUNCTION__);
        } else {
            LOGE("[%s] rds_rild_get_ho_status is null", __FUNCTION__);
        }
        if (nRet == RDS_FALSE) {
            LOGD("[%d][%s] get Ho status from Rds fail ", rid, __FUNCTION__);
            break;
        }
        pInfo->ho_start_cnt = ho_status->start_key;
        LOGD("[%d][%s] Interface %d Rds_ho_start_cnt = %d",
                rid, __FUNCTION__, pInfo->interfaceId, pInfo->ho_start_cnt);
        bRet = true;
    } while (false);

    FREEIF(ho_status);
    return bRet;
}

bool isHOSuccess(int hoResult) {
    return (hoResult == 1) ? true: false;
}

int handoverTo(const epdgHandoverStatus_t * pHoStatus) {
    int to = E_HO_UNKNOWN;
    if (isEpdgSupport() && isImsSupport()) {
        switch (pHoStatus->etarget_ran_type) {
        case RDS_RAN_MOBILE_3GPP:
        case RDS_RAN_MOBILE_3GPP2:
            to = E_HO_TO_LTE;
            break;
        case RDS_RAN_WIFI:
            to = E_HO_TO_WIFI;
            break;
        default:
            LOGE("error, unknown target eran: %d, src eran: %d",
                    pHoStatus->etarget_ran_type,
                    pHoStatus->esource_ran_type);
            break;
        };
    }
    return to;
}

int handoverFrom(const epdgHandoverStatus_t *pHoStatus) {
    int to = E_HO_UNKNOWN;
    if (isEpdgSupport() && isImsSupport()) {
        switch (pHoStatus->esource_ran_type) {
        case RDS_RAN_MOBILE_3GPP:
        case RDS_RAN_MOBILE_3GPP2:
            to = E_HO_FROM_LTE;
            break;
        case RDS_RAN_WIFI:
            to = E_HO_FROM_WIFI;
            break;
        default:
            LOGE("error, unknown target eran: %d, src eran: %d",
                    pHoStatus->etarget_ran_type,
                    pHoStatus->esource_ran_type);
            break;
        };
    }
    return to;
}

void requestHandoverInd(void *data, size_t datalen, RIL_Token t) {
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    if (isEpdgSupport() && isImsSupport()) {
        int nParam = (datalen/sizeof(int));
        if (nParam != 5) {
            LOGE("[%s] data len mismatched, [5, %d]", __FUNCTION__, nParam);
            goto error;
        }

        bool validActHoCid = false;
        int *pData = (int *) data;
        epdgHandoverStatus_t hoStatus = {0, 0, 0, 0, 0};
        PdnInfo *pPdnInfo = NULL;
        int i = 0;

        memcpy(&hoStatus, pData, datalen);
        dumpHandoverStatus(&hoStatus);

        if (handoverFrom(&hoStatus) == E_HO_FROM_WIFI && handoverTo(&hoStatus) == E_HO_TO_LTE) {
            for (i = 0; i < WIFI_MAX_PDP_NUM; i++) {
                if (pdn_info_wifi[rid][i].interfaceId == hoStatus.interfaceId
                        && pdn_info_wifi[rid][i].active != DATA_STATE_INACTIVE) {
                    pPdnInfo = &pdn_info_wifi[rid][i];
                    break;
                }
            }
            if (pPdnInfo != NULL) {
                memcpy(&pPdnInfo->hoStatus, &hoStatus, sizeof(hoStatus));
                if (getHoRefCountFromRds(rid, pPdnInfo) == false){
                    goto error;
                }
                if (isPdnHoStart(hoStatus.ucho_status)) {
                    pPdnInfo->ho_stop_cnt = pPdnInfo->ho_start_cnt - 1;
                    LOGD("[%d][%s] WIFI handover to LTE started, set state as linkdown temporarily.",
                            rid, __FUNCTION__);
                    mHandOverDir[rid] = E_HO_DIR_WIFI_TO_LTE;
                } else {
                    mHandOverDir[rid] = E_HO_DIR_UNKNOWN;
                    pPdnInfo->ho_stop_cnt++;
                    pPdnInfo->ho_stop_cnt = pPdnInfo->ho_stop_cnt % HANDOVER_MAX_COUNT;
                    if (isHOSuccess(hoStatus.fgho_result)) {
                        // WIFI handover to LTE success, clear wifi pdn info.
                        LOGD("[%d][%s] WIFI handover to LTE success, clear wifi pdn info.",
                                rid, __FUNCTION__);
                        if (hoStatus.interfaceId > INVALID_CID &&
                                  hoStatus.interfaceId < MAX_CCMNI_NUMBER) {
                            hoLtePdnSuccess[rid][hoStatus.interfaceId] = false;
                        }
                        // Need to copy the handover control variables from wifi pdn
                        for (i = 0; i < pdnInfoSize; i++) {
                            if (pdn_info[rid][i].interfaceId == hoStatus.interfaceId
                                    && pdn_info[rid][i].active == DATA_STATE_ACTIVE
                                    && !pdn_info[rid][i].isDedicateBearer) {
                                pdn_info[rid][i].ho_start_cnt = pPdnInfo->ho_start_cnt;
                                pdn_info[rid][i].ho_stop_cnt = pPdnInfo->ho_stop_cnt;
                                LOGD("[%d][%s]Copy handover [start, end] = [%d, %d] from wifi pdn info",
                                        rid, __FUNCTION__, pdn_info[rid][i].ho_start_cnt,
                                        pdn_info[rid][i].ho_stop_cnt);
                                break;
                            }
                        }
                        clearPdnInfo(pPdnInfo);
                        pPdnInfo = NULL;
                    } else {
                        // WIFI handover to LTE failed, restore state as active.
                        LOGD("[%d][%s] WIFI handover to LTE failed, restore state as active.",
                                rid,__FUNCTION__);
                    }
                    g_ho_pdn_remove_pending_act[rid].needRemove = false;
                    g_ho_pdn_remove_pending_act[rid].cid = INVALID_CID;
                }

                dumpPdnInfo(pPdnInfo);
            } else {
                LOGE("[%d][%s] no valid wifi pdn info !!", rid, __FUNCTION__);
            }
        } else if (handoverFrom(&hoStatus) == E_HO_FROM_LTE && handoverTo(&hoStatus) == E_HO_TO_WIFI) {
            int targetCid = -1;
            for (i = 0; i < pdnInfoSize; i++) {
                if (pdn_info[rid][i].interfaceId == hoStatus.interfaceId
                        && !pdn_info[rid][i].isDedicateBearer) {
                    pPdnInfo = &pdn_info[rid][i];
                    targetCid = i;
                    break;
                }
            }
            if (pPdnInfo != NULL) {
                memcpy(&pPdnInfo->hoStatus, &hoStatus, sizeof(hoStatus));
                getHoRefCountFromRds(rid, pPdnInfo);
                dumpPdnInfo(pPdnInfo);

                if (isPdnHoStart(hoStatus.ucho_status)) {
                    LOGD("[%d][%s] LTE handover to WIFI started.", rid, __FUNCTION__);
                    pPdnInfo->ho_stop_cnt = pPdnInfo->ho_start_cnt - 1;
                    mHandOverDir[rid] = E_HO_DIR_LTE_TO_WIFI;
                } else {
                    mHandOverDir[rid] = E_HO_DIR_UNKNOWN;
                    pPdnInfo->ho_stop_cnt++;
                    pPdnInfo->ho_stop_cnt = pPdnInfo->ho_stop_cnt % HANDOVER_MAX_COUNT;
                    if (isHOSuccess(hoStatus.fgho_result)) {
                        LOGD("[%d][%s] LTE handover to WIFI success, copy pdn info from LTE to WIFI.",
                                rid, __FUNCTION__);

                        int nWifiCid = getAvailableWifiCid(rid);
                        if (nWifiCid == INVALID_CID) {
                            LOGE("[%d][%s] no valid WIFI cid, need to extend boundary",
                                    rid, __FUNCTION__);
                            goto error;
                        } else {
                            pPdnInfo->rat = RDS_RAN_WIFI;
                            PdnInfo *pWifiPdnInfo = &pdn_info_wifi[rid][nWifiCid];
                            memcpy(pWifiPdnInfo, pPdnInfo, sizeof(PdnInfo));
                            pWifiPdnInfo->cid = nWifiCid;
                            pWifiPdnInfo->active = DATA_STATE_ACTIVE;
                            pWifiPdnInfo->primaryCid = nWifiCid;
                            pWifiPdnInfo->interfaceId = pdn_info_l2w[rid]->interfaceId;
                            pWifiPdnInfo->mtu = pdn_info_l2w[rid]->mtu;
                            dumpPdnInfo(pWifiPdnInfo);
                            LOGD("[%d] Update data call list after handover L2W success", rid);
                        }
                    } else {
                        LOGD("[%d][%s] LTE handover to WIFI failed.", rid, __FUNCTION__);
                    }
                    if (g_ho_pdn_remove_pending_act[rid].needRemove &&
                            g_ho_pdn_remove_pending_act[rid].cid == targetCid) {
                        clearPdnInfo(&pdn_info[rid][targetCid]);
                        LOGD("[%d] clear lte pdn for cid %d", rid, targetCid);
                    }
                    requestOrSendDataCallListIpv6(getRILChannelCtxFromToken(t), NULL, rid);
                    g_ho_pdn_remove_pending_act[rid].needRemove = false;
                    g_ho_pdn_remove_pending_act[rid].cid = INVALID_CID;
                }
            } else {
                LOGE("[%d][%s] no valid LTE pdn info !!", rid, __FUNCTION__);
            }
        }
    } else {
       LOGE("[%d][%s] no support epdg, do nothing!!", rid, __FUNCTION__);
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

void requestWifiDisconnectInd(void *data, size_t datalen, RIL_Token t)
{
    UNUSED(datalen);
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));

    if (isEpdgSupport() && isImsSupport()) {
        int interfaceId = atoi(((const char **)data)[0]);
        int reason = atoi(((const char **)data)[1]);
        int i = 0;
        int isDownIntf = 1;

        LOGD("[%d][%s] interfaceId=%d, reason=%d", rid, __FUNCTION__, interfaceId, reason);

        for (i = 0; i < pdnInfoSize; i++) {
            if (pdn_info[rid][i].interfaceId == interfaceId
                    && !pdn_info[rid][i].isDedicateBearer) {
                isDownIntf = 0;
                break;
            }
        }
        if (isDownIntf) {
            configureNetworkInterface(interfaceId, DISABLE_CCMNI, rid);
        }

        for (i = 0; i < WIFI_MAX_PDP_NUM; i++) {
            if (pdn_info_wifi[rid][i].interfaceId == interfaceId) {
                LOGD("[%d][%s] clear wifi pdn info for cid = %d",
                        rid, __FUNCTION__, pdn_info_wifi[rid][i].cid);
                dumpPdnInfo(&pdn_info_wifi[rid][i]);
                clearPdnInfo(&pdn_info_wifi[rid][i]);

                RILChannelCtx* pDataChannel = getRILChannelCtxFromToken(t);
                RIL_requestProxyTimedCallback(onDataCallListChanged, &s_data_ril_cntx[rid],
                        &TIMEVAL_0, pDataChannel->id,
                        "onDataCallListChanged due to wifi disconnect notify");
                break;
            }
        }
    } else {
       LOGE("[%d][%s] no support epdg, do nothing!!", rid, __FUNCTION__);
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;
}

void requestWifiConnectInd(void *data, size_t datalen, RIL_Token t) {

    UNUSED(datalen);
    if (!isEpdgSupport()) {
        LOGD("[%s] not support EPDG, return", __FUNCTION__);
        goto error;
    }
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    // reset EPDG PDN info first
    clearPdnInfo(pdn_info_l2w[rid]);

    // The total # of tlv items
    int nParam = 0;
    data = readInt(data, &nParam);
    LOGD("[%d][%s] nParam: %d", rid, __FUNCTION__, nParam);

    // Use type-length-value format
    int i = 0;
    for (i = 0; i < nParam; i++) {
        int nType = 0;
        int nLen = 0;
        data = readInt(data, &nType);
        data = readInt(data, &nLen);
        LOGD("[%d][%s] nType: %d, nLen: %d", rid, __FUNCTION__, nType, nLen);

        switch (nType) {
            case MALRIL_PARCEL_FIELD_INTERFACEID:
                data = readInt(data, &pdn_info_l2w[rid]->interfaceId);
                LOGD("[%d][%s] ifId: %d", rid, __FUNCTION__, pdn_info_l2w[rid]->interfaceId);
                break;
            case MALRIL_PARCEL_FIELD_MTU:
                data = readInt(data, &pdn_info_l2w[rid]->mtu);
                LOGD("[%d][%s] mtu: %d", rid, __FUNCTION__, pdn_info_l2w[rid]->mtu);
                break;
            case MALRIL_PARCEL_FIELD_UNKNOWN:
            default:
                LOGD("[%d][%s] Unknow type: %d", rid, __FUNCTION__, nType);
                data = readBytes(data, nLen);
                break;
        }
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);

}

void *readInt(void *pData, int *pValue) {
    *pValue = *(int *)pData;
    pData = sizeof(int) + (char *)pData;
    return pData;
}

void *readBytes(void *pData, int bytes) {
    pData = sizeof(char)*bytes + (char *)pData;
    return pData;
}

void requestSetupDataCallOverEpdg(const reqSetupConf_t *pReqSetupConfig, const void * pDefaultBearerVaConfig,
    RIL_Token t)
{
    if (!isEpdgSupport() && isImsSupport()) {
        LOGD("[%s] not support EPDG, return", __FUNCTION__);
        return;
    }

    int i = 0;
    int cid = pReqSetupConfig->availableCid;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    LOGD("[%d][%s] X, reserved cid, interfaceId=(%d,%d)", rid, __FUNCTION__, cid, pReqSetupConfig->interfaceId);

    const queryRatRsp_t *pQueryRatRsp = (const queryRatRsp_t *) pReqSetupConfig->pQueryRatRsp;
    const RIL_Default_Bearer_VA_Config_Struct *pDefaultBearerConfig =
            (const RIL_Default_Bearer_VA_Config_Struct *) pDefaultBearerVaConfig;

    MTK_RIL_Data_Call_Response_v11* response = (MTK_RIL_Data_Call_Response_v11*)
            calloc(1, sizeof(MTK_RIL_Data_Call_Response_v11));
    mal_datamngr_data_call_info_req_t* responseToMal = (mal_datamngr_data_call_info_req_t*)
            calloc(1, sizeof(mal_datamngr_data_call_info_req_t));
    assert(response != NULL);
    assert(responseToMal != NULL);
    if (response != NULL) {
        initialDataCallResponse(response, 1);
    } else {
        LOGE("[%s] allocate response space fail, return", __FUNCTION__);
        return;
    }

    if (responseToMal != NULL) {
        initialMalDataCallResponse(responseToMal, 1);
    } else {
        LOGE("[%s] allocate responseToMal space fail, return", __FUNCTION__);
        return;
    }

    if (pQueryRatRsp == NULL) {
        LOGE("error: Query Rat Rsp is null");
        goto error;
    }

    // Update related pdn info and response getting from EDPG
    PdnInfo *pPdnInfo = &pdn_info_wifi[rid][cid];
    pPdnInfo->interfaceId = pReqSetupConfig->interfaceId;    // Need this interface Id for checking HO
    pPdnInfo->primaryCid = pPdnInfo->cid = pPdnInfo->bearerId = cid;
    pPdnInfo->ddcId = INVALID_CID;
    pPdnInfo->isDedicateBearer = 0;
    pPdnInfo->isEmergency = (pDefaultBearerConfig->emergency_ind == 1) ? 1: 0;
    pPdnInfo->active = DATA_STATE_ACTIVE;
    pPdnInfo->signalingFlag = pDefaultBearerConfig->signalingFlag;
    cpStr(pPdnInfo->apn, pReqSetupConfig->requestedApn, strlen(pReqSetupConfig->requestedApn), sizeof(pPdnInfo->apn));

    //convert ipv4
    LOGI("[%s], epdg return type: %d", __FUNCTION__, pQueryRatRsp->resp.type);
    LOGI("[%s], epdg return ip addr [v4, v6], [%d, %s]", __FUNCTION__, pQueryRatRsp->resp.ipv4Addr, pQueryRatRsp->resp.ipv6Addr);
    if (pQueryRatRsp->resp.type == IPV4 || pQueryRatRsp->resp.type == IPV4V6) {
        inet_ntop(AF_INET, &pQueryRatRsp->resp.ipv4Addr, pPdnInfo->addressV4, MAX_IPV4_ADDRESS_LENGTH);
    }
    if (pQueryRatRsp->resp.type == IPV6 || pQueryRatRsp->resp.type == IPV4V6) {
        inet_ntop(AF_INET6, pQueryRatRsp->resp.ipv6Addr, pPdnInfo->addressV6, MAX_IPV6_ADDRESS_LENGTH);
    }
    LOGI("[%s], epdg return converted ip addr [v4, v6], [%s, %s]", __FUNCTION__, pPdnInfo->addressV4, pPdnInfo->addressV6);

    LOGD("query rat rsp dns num [v4, v6], [%d, %d]", pQueryRatRsp->resp.ipv4DnsNum,
        pQueryRatRsp->resp.ipv6DnsNum);
    if (pQueryRatRsp->resp.ipv4DnsNum <= MAX_NUM_DNS_ADDRESS_NUMBER) {
        for (; i < (int) pQueryRatRsp->resp.ipv4DnsNum; i++) {
            inet_ntop(AF_INET, pQueryRatRsp->resp.ipv4Dnses[i],
                pPdnInfo->dnsV4[i], MAX_IPV4_ADDRESS_LENGTH);
        }
    } else {
        LOGE("v4 dns number > max (%d)", MAX_NUM_DNS_ADDRESS_NUMBER);
    }

    if (pQueryRatRsp->resp.ipv6DnsNum <= MAX_NUM_DNS_ADDRESS_NUMBER) {
        for (i = 0; i < (int) pQueryRatRsp->resp.ipv6DnsNum; i++) {
            inet_ntop(AF_INET6, pQueryRatRsp->resp.ipv6Dnses[i],
                pPdnInfo->dnsV6[i], MAX_IPV6_ADDRESS_LENGTH);
        }
    } else {
        LOGE("v6 dns number > max (%d)", MAX_NUM_DNS_ADDRESS_NUMBER);
    }

    pPdnInfo->mtu = pQueryRatRsp->resp.mtuSize;
    pPdnInfo->rat = RDS_RAN_WIFI;

    // M: ALPS03113673
    if (getRildInterfaceCtrlSupport() == 1) {
        configureEpdgNetworkInterface(pPdnInfo->interfaceId,
                pQueryRatRsp->resp.type, ENABLE_CCMNI, rid);
    }

    // use MAL-RDS returned type to create response.
    int defaultCid = createDataResponse(pReqSetupConfig->interfaceId, pQueryRatRsp->resp.type, response, rid);
    createMalDataResponse(pReqSetupConfig->interfaceId, defaultCid, response, responseToMal, rid);

    //Extract prefix from global ipv6 address
    if (strcmp(pPdnInfo->addressV6, "::") != 0) {
        struct in6_addr ip;
        int ret = inet_pton(AF_INET6, pPdnInfo->addressV6, &ip);
        if (ret >= 0) {
            snprintf(pPdnInfo->prefix, sizeof(pPdnInfo->prefix), "%x:%x:%x:%x", htons(ip.s6_addr16[0]),
                htons(ip.s6_addr16[1]), htons(ip.s6_addr16[2]), htons(ip.s6_addr16[3]));
        }
        LOGD("[%d][%s] wifi pdn set prefix to %s", rid, __FUNCTION__, pPdnInfo->prefix);
    }

    setRspEranType(pQueryRatRsp->resp.eran_type, response, responseToMal, rid);
    dumpDataResponse(response, "requestSetupDataCallOverEpdg dump response");
    dumpMalDataResponse(responseToMal, "requestSetupDataCallOverEpdg dump mal response");

    responseUnsolDataCallRspToMal(t, responseToMal, sizeof(mal_datamngr_data_call_info_req_t));
    RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(MTK_RIL_Data_Call_Response_v11));
    LOGD("%s E sucess!!", __FUNCTION__);

    freeDataResponse(response);
    freeMalDataResponse(responseToMal);
    free(response);
    free(responseToMal);
    return;

error:
    responseUnsolDataCallRspToMal(t, NULL, 0);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    LOGD("%s E failed!!", __FUNCTION__);

    if (response != NULL) {
        freeDataResponse(response);
        free(response);
    }

    if (responseToMal != NULL) {
        freeMalDataResponse(responseToMal);
        free(responseToMal);
    }
}

void requestSetupDataCallHoEpdg(const reqSetupConf_t *pReqConf, const void *pDefaultBearerConf, RIL_Token t, const int serialId)
{
    if (!isEpdgSupport() && isImsSupport()) {
        LOGD("[%s] not support EPDG, return", __FUNCTION__);
        return;
    }

    // W -> L (HO)
    int i = 0;
    int activatedPdnNum = 0;
    int concatenatedBearerNum = 0;
    int *activatedCidList = alloca(pdnInfoSize * sizeof(int));
    int isNoDataActive = 0;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    int sim_id = getValidSimId(rid, __FUNCTION__);
    int suggestedRetryTime = NO_SUGGESTED_TIME;

    const RIL_Default_Bearer_VA_Config_Struct *pDefBearerConf =
            (const RIL_Default_Bearer_VA_Config_Struct *) pDefaultBearerConf;

    MTK_RIL_Data_Call_Response_v11* response = (MTK_RIL_Data_Call_Response_v11*)
            calloc(1, sizeof(MTK_RIL_Data_Call_Response_v11));
    mal_datamngr_data_call_info_req_t* responseToMal = (mal_datamngr_data_call_info_req_t*)
            calloc(1, sizeof(mal_datamngr_data_call_info_req_t));
    mal_datamngr_data_call_info_req_ptr_t req_ptr =
            (mal_datamngr_data_call_info_req_ptr_t) responseToMal;
    mal_datamngr_data_call_info_rsp_ptr_t rsp_ptr =
            calloc(1, sizeof(mal_datamngr_data_call_info_rsp_t));
    if (response != NULL) {
        initialDataCallResponse(response, 1);
    } else {
        LOGE("[%s] allocate response space fail, return", __FUNCTION__);
        FREEIF(rsp_ptr);
        if (responseToMal != NULL) {
            free(responseToMal);
        }
        return;
    }

    if (responseToMal != NULL) {
        initialMalDataCallResponse(responseToMal, 1);
    } else {
         FREEIF(rsp_ptr);
        if (response != NULL) {
            free(response);
        }
        LOGE("[%s] allocate responseToMal space fail, return", __FUNCTION__);
        return;
    }

    LOGD("[%d][%s] X", rid, __FUNCTION__);

    for (i = 0; i < pdnInfoSize; i++) {
        activatedCidList[i] = INVALID_CID;
    }

    int queryResult = queryMatchedPdnWithSameApn(pReqConf->requestedApn, activatedCidList, DATA_CHANNEL_CTX);
    if (queryResult > 0 && isCidTypeMatched(activatedCidList[0], pReqConf->protocol, DATA_CHANNEL_CTX)) {
        LOGI("[%s] matched PDN is found [%d]", __FUNCTION__, queryResult);
        activatedPdnNum = queryResult;
    } else {
        int availableCid = getAvailableCid(rid);
        if (availableCid == INVALID_CID) {
            LOGE("[%s] no available CID to use", __FUNCTION__);
            goto error;
        } else {
            LOGD("[%s] available CID is [%d]", __FUNCTION__, availableCid);
        }

        if (0 == definePdnCtx(pDefaultBearerConf, pReqConf->requestedApn, pReqConf->protocol,
                availableCid, pReqConf->authType, pReqConf->username, pReqConf->password,
                DATA_CHANNEL_CTX)) {
            LOGE("[%s] definePdnCtx error", __FUNCTION__);
            goto error;
        }

        if (PDN_FAILED >= activatePdn(availableCid, &activatedPdnNum, &concatenatedBearerNum, activatedCidList,
                pDefBearerConf->emergency_ind, isNoDataActive, DATA_CHANNEL_CTX)) {
            LOGE("[%s] activatePdn error", __FUNCTION__);
            suggestedRetryTime = getModemSuggestedRetryTime(availableCid, DATA_CHANNEL_CTX);
            goto error;
        }
    }

    for (i = 0; i < activatedPdnNum; i++) {
        int cid = activatedCidList[i];
        if (pdn_info[rid][cid].isDedicateBearer) {
            LOGD("[%s] skip update PDP address of CID%d since it is a dedicate bearer [active=%d]",
                    __FUNCTION__, cid, pdn_info[rid][cid].active);
        } else {
            if (updatePdnAddressByCid(cid, DATA_CHANNEL_CTX) < 0) {
                LOGD("[%s] update PDP address of CID%d failed", __FUNCTION__, cid);
                // we do not goto error since we expect to update all activated PDP address here
            }
            // To revise the IPV6 address from link-local to global format according to wifi pdn
            adaptIpDiscontinuityNw(pReqConf->interfaceId, cid, rid);
            if (0 == bindPdnToIntf(activatedPdnNum, pReqConf->interfaceId, cid, DATA_CHANNEL_CTX)) {
                LOGE("[%s] bindPdnToIntf error", __FUNCTION__);
                goto error;
            }
        }
    }

    updateDynamicParameter(DATA_CHANNEL_CTX);
    if (updateDns(DATA_CHANNEL_CTX) < 0) {
        LOGE("[%s] updateDns failed", __FUNCTION__);
        goto error;
    }

    if (DATA_STATE_INACTIVE == isCidActive(activatedCidList, activatedPdnNum, DATA_CHANNEL_CTX)) {
        goto error;
    }

    // This is handover case (WIFI to LTE), so ccmniX interface should be already up. Do nothing here
    // configureNetworkInterface(pReqConf->interfaceId, ENABLE_CCMNI, rid);
    if (pReqConf->interfaceId > INVALID_CID && pReqConf->interfaceId < MAX_CCMNI_NUMBER) {
        hoLtePdnSuccess[rid][pReqConf->interfaceId] = true;
    }
    int cid = createDataResponse(pReqConf->interfaceId, pReqConf->protocol, response, rid);
    createMalDataResponse(pReqConf->interfaceId, cid, response, responseToMal, rid);
    makeDedicatedDataResponse(concatenatedBearerNum, activatedPdnNum, activatedCidList, responseToMal, rid);
    setRspEranType(RDS_RAN_MOBILE_3GPP, response, responseToMal, rid);
    dumpDataResponse(response, "requestSetupDataCallHoEpdg dump response");
    dumpMalDataResponse(responseToMal, "requestSetupDataCallHoEpdg dump mal response");

    if (sim_id != SIM_ID_INVALID) {
        if (mal_rilproxy_resp_data_call_info_tlv != NULL) {
            int ret = mal_rilproxy_resp_data_call_info_tlv (mal_once(1, mal_cfg_type_sim_id, sim_id), serialId,
                    req_ptr, rsp_ptr, 0, NULL, NULL, NULL);
            LOGD("[%s] Call mal_rilproxy_resp_data_call_info_tlv success", __FUNCTION__);
            RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        } else {
            LOGE("[%s] mal_rilproxy_resp_data_call_info_tlv is NULL", __FUNCTION__);
            RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
        }
    } else{
            LOGE("[%s] SIM ID is invalid", __FUNCTION__);
            RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    }

    FREEIF(rsp_ptr);

    freeDataResponse(response);
    freeMalDataResponse(responseToMal);
    free(response);
    free(responseToMal);

    {   // Update Data call list to FWK if W -> L HO successfully.
        // 1. Send data call list to fwk here, instead of sending to data queue,
        //    which may be blocked, and cause status un-sync with RDS. see ALPS03051739.
        // 2. For responseDataCallList() will create different data format for MAL & fwk.
        //    So we reset isReqFromMAL() first to get right data fromat to fwk. see ALPS03094974.
        requestOrSendDataCallListIpv6(getChannelCtxbyProxy(), NULL, rid);
    }

    LOGD("[%s] E successful", __FUNCTION__);
    return;

error:
    response->status = gprs_failure_cause;
    response->cid = pReqConf->interfaceId;
    if (sim_id != SIM_ID_INVALID) {
        if (mal_rilproxy_resp_data_call_info_tlv != NULL) {
            responseToMal->status = gprs_failure_cause;
            responseToMal->cid = pReqConf->interfaceId;
            responseToMal->active = DATA_STATE_INACTIVE;
            responseToMal->ifname = NULL;

            int ret = mal_rilproxy_resp_data_call_info_tlv (mal_once(1, mal_cfg_type_sim_id, sim_id), serialId,
                    req_ptr, rsp_ptr, 0, NULL, NULL, NULL);
            LOGD("[%s] Call mal_rilproxy_resp_data_call_info_tlv success", __FUNCTION__);
            RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        } else {
            LOGE("[%s] mal_rilproxy_resp_data_call_info_tlv is NULL", __FUNCTION__);
            RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
        }
    } else{
            LOGE("[%s] SIM ID is invalid", __FUNCTION__);
            RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    }
    FREEIF(rsp_ptr);
    LOGD("%s E failed!!", __FUNCTION__);

    if (response != NULL) {
        freeDataResponse(response);
        free(response);
    }

    if (responseToMal != NULL) {
        freeMalDataResponse(responseToMal);
        free(responseToMal);
    }
}

void adaptIpDiscontinuityNw(int interfaceId, int cid, RIL_SOCKET_ID rid)
{
    int i = 0;

    if (!isOp16Support()) {
        for (i = 0; i < WIFI_MAX_PDP_NUM; i++) {
            if (pdn_info_wifi[rid][i].interfaceId == interfaceId &&
                    strlen(pdn_info_wifi[rid][i].prefix) != 0) {
                strncpy(pdn_info[rid][cid].prefix, pdn_info_wifi[rid][i].prefix,
                        sizeof(pdn_info[rid][cid].prefix) -1);
                LOGD("[%d][%s] copy prefix %s from wifi[%d].active=%d to lte[%d].active=%d",
                        rid, __FUNCTION__, pdn_info[rid][cid].prefix
                        , i, pdn_info_wifi[rid][i].active, cid, pdn_info[rid][cid].active);
            }
        }
        return;
    }
    LOGD("[%d][%s] Support for operator OP16", rid, __FUNCTION__);
    for (i = 0; i < WIFI_MAX_PDP_NUM; i++) {
        if (pdn_info_wifi[rid][i].interfaceId == interfaceId) {
            LOGD("[%d][%s] copy V6 address for pdn_info_wifi[%d][%d], cid = %d by interfaceId = %d",
                    rid, __FUNCTION__, rid, i, cid, interfaceId);
            dumpPdnInfo(&pdn_info_wifi[rid][i]);
            strncpy(pdn_info[rid][cid].addressV6, pdn_info_wifi[rid][i].addressV6,
                    sizeof(pdn_info[rid][cid].addressV6) - 1);
            memcpy(&pdn_info[rid][cid].hoStatus, &pdn_info_wifi[rid][i].hoStatus,
                    sizeof(epdgHandoverStatus_t));
            dumpPdnInfo(&pdn_info[rid][cid]);
        }
    }
}

int getValidSimId(RIL_SOCKET_ID rid, const char *caller) {
    int sim_id = SIM_ID_INVALID;

    if (rid >= RIL_SOCKET_NUM) {
        LOGE("Invalid rid %d for %s", rid, caller);
    } else {
        sim_id = (int)rid;
        LOGD("[%d][getValidSimId] for %s", rid, caller);
    }

    return sim_id;
}

void dumpHandoverStatus(const epdgHandoverStatus_t * pHoStatus) {
    if (pHoStatus) {
        LOGD("[%s] hoStatus, ucho_status=%d, fgho_result=%d, interfaceId=%d, esource_ran_type=%d, etarget_ran_type=%d",
                __FUNCTION__, pHoStatus->ucho_status, pHoStatus->fgho_result, pHoStatus->interfaceId,
                pHoStatus->esource_ran_type, pHoStatus->etarget_ran_type);
     }
}

void requestPcscfPco(void *data, size_t datalen, RIL_Token t)
{
    UNUSED(datalen);
    int cid = *(int *)data;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    LOGD("[%d]requestPcscfPco CID=%d", rid, cid);

    if (updateDefaultBearerInfo(DATA_CHANNEL_CTX) < 0) {
        LOGE("[%d]requestPcscfPco updateDefaultBearerInfo failed", rid);
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    } else {
        int i;
        int pcscfLength = 0;
        char* pcscf = NULL;
        for (i = 0; i < MAX_PCSCF_NUMBER; i++) {
            if (strlen(pdn_info[rid][cid].pcscf[i]) > 0) {
                pcscfLength += strlen(pdn_info[rid][cid].pcscf[i]);
                if (i != 0)
                    ++pcscfLength; //add one space
            }
        }
        pcscf= calloc(1, pcscfLength * sizeof(char) + 1);
        if (pcscf == NULL) {
            LOGE("[%s] calloc for pcscf failed!", __FUNCTION__);
            return;
        }
        for (i = 0; i < MAX_PCSCF_NUMBER; i++) {
            if (strlen(pdn_info[rid][cid].pcscf[i]) > 0) {
                int currentLength = strlen(pcscf);
                if (currentLength > 0) {
                    strncat(pcscf+currentLength, " ", pcscfLength - currentLength - 1);
                    currentLength = strlen(pcscf);
                }
                strncat(pcscf+currentLength, pdn_info[rid][cid].pcscf[i], pcscfLength - currentLength - 1);
            }
        }
        LOGD("[%d]requestPcscfPco response [%s]", rid, pcscf);
        RIL_onRequestComplete(t, RIL_E_SUCCESS, pcscf, strlen(pcscf));

        free(pcscf);
    }
}

void configureEpdgNetworkInterface(int interfaceId, int protocol, int isUp, RIL_SOCKET_ID rid) {
    struct ifreq ifr;
    //Configure the IP address to the CCMNI interface
    /* Open the network interface: CCMNI */
    LOGD("[%d]configureEpdgNetworkInterface interface %d to %s with protocol %d",
             rid, interfaceId, isUp ? "UP" : "DOWN", protocol);
    if (interfaceId == INVALID_CID) {
        LOGE("interface %d is invalid, ignore config!", interfaceId);
        return;
    }
    memset(&ifr, 0, sizeof(struct ifreq));
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s%d", s_ccmni_ifname, interfaceId);

    ril_data_ioctl_init(interfaceId);

    if (isUp) {
        // set the network interface down first before up
        // to prevent from unknown exception causing not close related
        // dev file description, and clear dirty ip address.
        LOGD("set network interface %s down before up", ifr.ifr_name);
        setNwIntfDown(ifr.ifr_name);

        // Install address and up interface
        int i = 0;
        for (i = 0; i < WIFI_MAX_PDP_NUM; i++) {
            if (pdn_info_wifi[rid][i].interfaceId == interfaceId) {
                if (strlen(pdn_info_wifi[rid][i].addressV4) > 0
                        && (protocol == IPV4 || protocol == IPV4V6)) {
                    LOGD("ipv4 addr: %s", pdn_info_wifi[rid][i].addressV4);
                    ril_data_setflags(sock_fd[interfaceId], &ifr, IFF_UP, 0);
                    ril_data_setaddr(sock_fd[interfaceId], &ifr, pdn_info_wifi[rid][i].addressV4);
                }
                if (strlen(pdn_info_wifi[rid][i].addressV6) > 0
                        && (protocol == IPV6 || protocol == IPV4V6)) {
                    LOGD("ipv6 addr: %s", pdn_info_wifi[rid][i].addressV6);
                    ril_data_setflags(sock6_fd[interfaceId], &ifr, IFF_UP, 0);
                    ril_data_set_ipv6addr(sock6_fd[interfaceId], &ifr, pdn_info_wifi[rid][i].addressV6);
                }
            }
        }
    } else {
        setNwIntfDown(ifr.ifr_name);
    }

    close(sock_fd[interfaceId]);
    close(sock6_fd[interfaceId]);
    sock_fd[interfaceId] = 0;
    sock6_fd[interfaceId] = 0;
}


/// M: Reset MAL cache @{
void clearRilDataMalCache() {
    LOGD("Mal restart, reset ril data Mal cache");
    // TODO: In order to avoid build error, always get main protocol rid for MAL.
    clearAllPdnInfo(getMainProtocolRid());
}
/// @}
int setPsActiveSlot(int actedSim, RILChannelCtx *pChannel) {
    char *cmd = NULL;
    ATResponse *p_response = NULL;
    int ret = 1;
    int err = -1;
    int retryCount = 0;
    int svlteSlotId = 0;
    char tempstr[PROPERTY_VALUE_MAX];
    int currMajorSim = 1;
    int isLcMode = 0;

    if (!isEactsSupport()) {
        LOGD("setPsActiveSlot, EACTS not support.");
        return 1;
    }

    memset(tempstr, 0, sizeof(tempstr));
    property_get("persist.vendor.radio.simswitch", tempstr, "1");
    currMajorSim = atoi(tempstr);
    svlteSlotId = getActiveSvlteModeSlotId();
    if ((getSimCount() == 2) && (svlteSlotId == currMajorSim)) {
        isLcMode = 1;
    }

    LOGI("[RILData_GSM_IRAT] mode is %d, act sim: %d", isLcMode, actedSim);
    if (!isLcMode) {
        LOGD("[RILData_GSM_IRAT] mode is not LC+G or G+LC, don't send EACTS.");
        return 1;
    }
    // TODO: phase out EACTS is the right way. Retry fix AT command conflict is temporary.
    asprintf(&cmd, "AT+EACTS=%d", actedSim);
    do {
        int retErr = 0;
        err = at_send_command(cmd, &p_response, pChannel);
        retErr = isATCmdRspErr(err, p_response);
        at_response_free(p_response);
        retryCount++;
        LOGD("[RILData_GSM_IRAT] setPsActiveSlot %s err: %d, count: %d", cmd, err, retryCount);
        if (retErr == 0) {
            break;
        } else if (retErr && retryCount == MAX_RETRY_EACTS) {
            ret = 0;
            break;
        } else if (retErr) {
            usleep(EACTS_RETRY_INTERVAL);
        }
    } while (retryCount < MAX_RETRY_EACTS);
    free(cmd);
    return ret;
}
/// [C2K][IRAT] code end. @}

// M: JPN IA Start{
// run and should be invoked on SIM CH
void enableMdSuspendResume(void* param) {
    const char *propDataOpSusRsmMd = "vendor.gsm.ril.data.op.suspendmd";
    if (param == NULL) {
        LOGE("[%s] param is NULL", __FUNCTION__);
        return;
    }
    TimeCallbackParam* timeCallbackParam = (TimeCallbackParam*) param;
    int rid = timeCallbackParam->rid;
    at_send_command("AT+ECOPS=1", NULL, getChannelCtxbyProxy());
    at_send_command("AT+EMSR=0,1", NULL, getChannelCtxbyProxy());
    property_set(propDataOpSusRsmMd, "1");
    LOGD("[%s]-%d: %s 1", __FUNCTION__, rid, propDataOpSusRsmMd);
    FREEIF(param);
}

// run and should be invoked on SIM CH
void setOpPropertyBySim(RIL_SOCKET_ID rid, char *mccmnc) {
    const char *propDataOpSusRsmMd = "vendor.gsm.ril.data.op.suspendmd";
    const char *propConditionallyAttach = "vendor.gsm.ril.cond.attach";
    const char *propOp129Sim = "vendor.gsm.ril.sim.op129";
    if (isOp17MccMnc(mccmnc) || isOp129MccMnc(mccmnc)) {
        if (isOp17MccMnc(mccmnc)) {
            setMSimProperty(rid, "vendor.gsm.ril.sim.op17", "1");
        } else {
            property_set(propOp129Sim, "1");
        }
        property_set(propConditionallyAttach, "1");

        LOGD("[%s]-%d:, %s", __FUNCTION__, rid, propConditionallyAttach);

        if (getMainProtocolRid() == rid &&
                !isInternationalRoamingEnabled() &&
                ((isWorldPhoneSupport() && bUbinSupport) || !isWorldPhoneSupport()) &&
                !bSetOpPropOneShot) {
            TimeCallbackParam* param = (TimeCallbackParam*) calloc(1, sizeof(TimeCallbackParam));
            if (param == NULL) {
                LOGE("[%s]-%d: param calloc failed.", __FUNCTION__, rid);
                return;
            }
            param->rid = rid;
            RIL_requestProxyTimedCallback(enableMdSuspendResume, param, &TIMEVAL_0,
                    getRILChannelId(RIL_SIM, rid), "enableMdSuspendResume");
            LOGD("[%s]-%d: set enableMdSuspendResume as callback", __FUNCTION__, rid);
        } else if (!bSetOpPropOneShot) {
            property_set(propDataOpSusRsmMd, "0");
            LOGD("[%s]-%d: %s 0", __FUNCTION__, rid, propDataOpSusRsmMd);
        }
        bSetOpPropOneShot = 1;
    }
}

// run and should be invoked on SIM CH
void setOpPropertyBySbp(RIL_SOCKET_ID rid) {
    char sbpId[PROPERTY_VALUE_MAX] = {0};
    property_get("persist.vendor.radio.sim.opid", sbpId, "0");
    int sbp = atoi(sbpId);
    LOGD("[%s]-%d: got d-sbp = %d", __FUNCTION__, rid, sbp);

    if (sbp == 129) {
        // 44051 belong to sbp-129, hardcoding here just meant to set properties
        setOpPropertyBySim(rid, "44051");
    } else if (sbp == 17) {
        // 44010 belong to sbp-17, hardcoding here just meant to set properties
        setOpPropertyBySim(rid, "44010");
    }
}

// run and should be invoked on SIM CH
void setupOpPropertyCb(void* param) {
    TimeCallbackParam* timeCallbackParam = (TimeCallbackParam*) param;
    int rid = timeCallbackParam->rid;
    int cbCtr = timeCallbackParam->isModification;
    int hasCallback = 0;
    SIM_Status simState = getSIMStatus(rid);
    LOGD("[%s]-%d: SIM status is %d", __FUNCTION__, rid, simState);

    free(timeCallbackParam);

    if (simState == SIM_READY || simState == USIM_READY) {
        LOGD("[%s]-%d: SIM is ready, start to query IMSI", __FUNCTION__, rid);
        char mccmnc[10];
        memset(&mccmnc, 0, sizeof(mccmnc));
        if (readMccMnc(rid, mccmnc) == READ_MCC_MNC_ERR_SUCCESS) {
            setOpPropertyBySim(rid, mccmnc);
        } else {
            LOGD("[%s]-%d: failed to query MCC/MNC", __FUNCTION__, rid);
            hasCallback = 1;
        }
    } else if (simState == SIM_PIN ||
            simState == USIM_PIN ||
            simState == RUIM_PIN) {
        LOGD("[%s]-%d: SIM is locked, try SBP property", __FUNCTION__, rid);
        setOpPropertyBySbp(rid);
    } else if (simState == SIM_BUSY) {
        hasCallback = 1;
    } else {
        LOGD("[%s]-%d: SIM isn't ready: %d, stop procedure...", __FUNCTION__, rid, simState);
    }

    if (hasCallback) {
        TimeCallbackParam* paramNew = (TimeCallbackParam*) calloc(1, sizeof(TimeCallbackParam));
        if (paramNew == NULL) {
            LOGD("[%s]-%d: calloc failed.", __FUNCTION__, rid);
            return;
        }
        paramNew->rid = rid;
        paramNew->isModification = ++cbCtr;
        LOGD("[%s]-%d: SIM is busy, %dth re-try later", __FUNCTION__, rid, cbCtr);
        RIL_requestProxyTimedCallback(setupOpPropertyCb, paramNew, &TIMEVAL_500_MS,
                getRILChannelId(RIL_SIM, rid), "setupOpPropertyCb");
    } else {
        LOGD("[%s]-%d: X", __FUNCTION__, rid);
    }
}

// run and should be invoked on SIM CH
void setupOpProperty(RIL_SOCKET_ID rid) {
    LOGD("[%s]-%d: E", __FUNCTION__, rid);
    TimeCallbackParam* param = (TimeCallbackParam*) calloc(1, sizeof(TimeCallbackParam));
    if (param == NULL) {
        LOGE("[%s]-%d: param calloc failed.", __FUNCTION__, rid);
        return;
    }
    param->rid = rid;
    param->isModification = 0;//use it as a counter

    RIL_requestProxyTimedCallback(setupOpPropertyCb, param, &TIMEVAL_0,
            getRILChannelId(RIL_SIM, rid), "setupOpPropertyCb");
}
// M: JPN IA End}
int setApnControlMode(RIL_SOCKET_ID rid) {
    char* cmd = NULL;
    ATResponse *p_response = NULL;
    int err = 0;
    RILChannelCtx* pChannel = getChannelCtxbyProxy();
    int ret = 1;

    // VZW IMS handover
    char imsHandOver[PROPERTY_VALUE_MAX] = {0};
    property_get("vendor.ril.imshandover", imsHandOver, "0");
    LOGD("setAPNControlMode imsHandOver: %s", imsHandOver);
    if ((0 == strcmp(imsHandOver, "1")) || (0 == strcmp(imsHandOver, "2"))) {
        // VZW AP Fully Control IA (Initial Attach) APN
        // AT+ EVZWAPFCIA =<mode>
        // <mode>
        // 0 : AP does not fully control VzW LTE ATTACH APN config,
        //     modem can force AP to change VzW LTE ATTACH APN
        // 1 : AP fully control VzW LTE ATTACH APN config,
        //     modem cannot suggest/force AP to change VzW LTE ATTACH APN
        if (0 == strcmp(imsHandOver, "1")) {
            asprintf(&cmd, "AT+EVZWAPFCIA=%d", 1);
        } else if (0 == strcmp(imsHandOver, "2")) {
            asprintf(&cmd, "AT+EVZWAPFCIA=%d", 0);
            // reset to initial state
            property_set("vendor.ril.imshandover", "0");
            // return 0 to skip set IA flow
            ret = 0;
        }
        err = at_send_command(cmd, &p_response, pChannel);
        free(cmd);
        if (isATCmdRspErr(err, p_response)) {
            LOGD("requestSetInitialAttachApn EAPCTRLAPN failed!");
        }
        AT_RSP_FREE(p_response);
    }
    return ret;
}

bool isImsInterfaceId(int interfaceId) {
    return ((IMS_CHANNEL_MASK & (1 << interfaceId)) != 0);
}
// COPY_FROM_MTK_RIL_DATA @}
void requestSetupDataCallFromMal(void * data, size_t datalen, RIL_Token t)
{
    /* In GSM with CDMA version: DOUNT - data[0] is radioType(GSM/UMTS or CDMA), data[1] is profile,
     * data[2] is apn, data[3] is username, data[4] is passwd, data[5] is authType (added by Android2.1)
     * data[6] is cid field added by mtk for Multiple PDP contexts setup support 2010-04
     */
    int requestParamNumber = (datalen/sizeof(char*));
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    LOGD("[%s][%d]requestSetupData with datalen=%d and parameter number=%d",
            __FUNCTION__, rid, (int) datalen, requestParamNumber);
    int serialId = -1;

    reqSetupConf_t reqSetupConf = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    reqSetupConf.radioType = ((const char **)data)[0];
    reqSetupConf.profile = ((const char **)data)[1];
    reqSetupConf.requestedApn = ((const char **)data)[2];
    reqSetupConf.username = ((const char **)data)[3];
    reqSetupConf.password = ((const char **)data)[4];
    reqSetupConf.authType = getAuthType(((const char **)data)[5]);
    reqSetupConf.protocol = get_protocol_type(((const char **)data)[6]);

    reqSetupConf.interfaceId = requestParamNumber > 7 ? atoi(((const char **)data)[7]) : INVALID_CID;


/// M: IMS/EPDG @{
    reqSetupConf.availableCid = INVALID_CID;
    reqSetupConf.retryCount = 1;
/// M: IMS/EPDG @}

    int isFB = isFallbackPdpNeeded(rid, reqSetupConf.requestedApn, reqSetupConf.protocol);

    //VoLTE
    RIL_Default_Bearer_VA_Config_Struct defaultBearerVaConfig;
    memset(&defaultBearerVaConfig, 0, sizeof(RIL_Default_Bearer_VA_Config_Struct));

    dumpReqSetupConfig(&reqSetupConf);
    LOGD("defaultBearerVaConfig.isValid=%d", defaultBearerVaConfig.isValid);

    if (reqSetupConf.interfaceId < 0) {
        int i = 0, j = 0;
        for (i = 0; i < 10; i++) {
            if (reqSetupConf.interfaceId == INVALID_CID) {
                reqSetupConf.interfaceId = i;
                for (j = 0; j<max_pdn_support; j++) {
                    if (pdn_info[rid][j].interfaceId == i) {
                        LOGD("requestSetupData interfaceId [%d] is already in use", i);
                        reqSetupConf.interfaceId = INVALID_CID;
                        break;
                    }
                }
            }
        }
        LOGD("requestSetupData selected ineterfaceId is %d", reqSetupConf.interfaceId);
    }

    if (!isLegalPdnRequest(reqSetupConf.interfaceId, rid)) {
         LOGD("requestSetupData illegal pdn request by ifid = %d", reqSetupConf.interfaceId);
         goto error;
    }

    if (requestParamNumber > 8) {
        defaultBearerVaConfig.isValid = atoi(((const char **)data)[8]);
        if (requestParamNumber == 19) {
            defaultBearerVaConfig.isHandover = atoi(((const char **)data)[17]);
            LOGD("isHandover=%d", defaultBearerVaConfig.isHandover);
        }
    }

    if (defaultBearerVaConfig.isValid) {
        defaultBearerVaConfig.qos.qci = atoi(((const char **) data)[9]);
        defaultBearerVaConfig.qos.dlGbr = atoi(((const char **) data)[10]);
        defaultBearerVaConfig.qos.ulGbr = atoi(((const char **) data)[11]);
        defaultBearerVaConfig.qos.dlMbr = atoi(((const char **) data)[12]);
        defaultBearerVaConfig.qos.ulMbr = atoi(((const char **) data)[13]);
        defaultBearerVaConfig.emergency_ind = atoi(((const char **) data)[14]);
        defaultBearerVaConfig.pcscf_discovery_flag = atoi(((const char **) data)[15]);
        defaultBearerVaConfig.signalingFlag = atoi(((const char **) data)[16]);

        dumpDefaultBearerConfig(&defaultBearerVaConfig);
    }


    if (requestParamNumber != 19) {
        LOGE("requestSetupData with incorrect parameters, requestParamNumber=%d", requestParamNumber);
        goto error;
    } else {
        serialId = atoi(((const char **)data)[18]);
        LOGD("[%s] serial id is: %d",__FUNCTION__,serialId);
    }


    if (isEpdgSupport()) {
        requestSetupDataCallHoEpdg(&reqSetupConf, (void *) &defaultBearerVaConfig, t, serialId);
        //defaultBearerVaConfig.isHandover = 1;
    } else {
        LOGE("[%s] isEpdgSupport is false", __FUNCTION__);
    }


finish:
    FREEIF(reqSetupConf.pQueryRatRsp);
    return;

error:
    LOGE("[%s] error E", __FUNCTION__);
    FREEIF(reqSetupConf.pQueryRatRsp);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

void requestLastDataCallFailCauseFromMal(void * data, size_t datalen, RIL_Token t)
{
    UNUSED(datalen);
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    int serialId = -1;
    if (data == NULL) {
        LOGE("[%s] Do not receive serial from MAL", __FUNCTION__);
    } else {
        serialId = ((int*)data)[0];
        LOGD("[%s] serialId is %d", __FUNCTION__, serialId);
    }
    int lastPdpFailCause = 14;
    lastPdpFailCause = getLastDataCallFailCause();

    int sim_id = getValidSimId(rid, __FUNCTION__);
    mal_rilproxy_last_fail_cause_rsp_ptr_t rsp_ptr =
            calloc(1, sizeof(mal_rilproxy_last_fail_cause_rsp_t));
    assert(rsp_ptr != NULL);
    if (sim_id != SIM_ID_INVALID) {
        if (mal_rilproxy_resp_last_fail_cause_tlv != NULL) {
            int ret = mal_rilproxy_resp_last_fail_cause_tlv (mal_once(1, mal_cfg_type_sim_id, sim_id), serialId, lastPdpFailCause, rsp_ptr,
                                                                                            0, NULL, NULL, NULL);
            LOGD("[%s] Call mal_rilproxy_resp_last_fail_cause_tlv success", __FUNCTION__);
            RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        } else {
            LOGE("[%s] mal_rilproxy_resp_last_fail_cause_tlv is null", __FUNCTION__);
            RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
        }
    } else {
        LOGE("[%s] SIM ID is invalid", __FUNCTION__);
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    }
    FREEIF(rsp_ptr);
}

void requestPcscfPcoFromMal(void *data, size_t datalen, RIL_Token t)
{
    UNUSED(datalen);
    int cid = ((int *)data)[0];
    int serialId = ((int *)data)[1];
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    LOGD("[%d]requestPcscfPco CID=%d and serialId=%d", rid, cid, serialId);

    if (updateDefaultBearerInfo(DATA_CHANNEL_CTX) < 0) {
        LOGE("[%d]requestPcscfPco updateDefaultBearerInfo failed", rid);
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    } else {
        int i;
        int pcscfLength = 0;
        char* pcscf = NULL;
        for (i = 0; i < MAX_PCSCF_NUMBER; i++) {
            if (strlen(pdn_info[rid][cid].pcscf[i]) > 0) {
                pcscfLength += strlen(pdn_info[rid][cid].pcscf[i]);
                if (i != 0)
                    ++pcscfLength; //add one space
            }
        }
        pcscf= calloc(1, pcscfLength * sizeof(char) + 1);
        assert(pcscf != NULL);
        for (i = 0; i < MAX_PCSCF_NUMBER; i++) {
            if (strlen(pdn_info[rid][cid].pcscf[i]) > 0) {
                int currentLength = strlen(pcscf);
                if (currentLength > 0)
                    strncat(pcscf+currentLength, " ", 1);

                strncat(pcscf+strlen(pcscf), pdn_info[rid][cid].pcscf[i],
                    MIN((pcscfLength - strlen(pcscf)), strlen(pdn_info[rid][cid].pcscf[i])));
            }
        }
        LOGD("[%d]requestPcscfPco response [%s]", rid, pcscf);

        int sim_id = getValidSimId(rid, __FUNCTION__);
        mal_rilproxy_pcscf_pco_rsp_ptr_t rsp_ptr =
                calloc(1, sizeof(mal_rilproxy_pcscf_pco_rsp_t));
        assert(rsp_ptr != NULL);
        if (sim_id != SIM_ID_INVALID) {
            if (mal_rilproxy_resp_pcscf_pco_tlv != NULL) {
                int ret = mal_rilproxy_resp_pcscf_pco_tlv (mal_once(1, mal_cfg_type_sim_id, sim_id), serialId, pcscf,
                        rsp_ptr, 0, NULL, NULL, NULL);
                LOGD("[%s] Call mal_rilproxy_resp_pcscf_pco_tlv success", __FUNCTION__);
                RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
            } else {
                LOGE("[%s] mal_rilproxy_resp_pcscf_pco_tlv is null", __FUNCTION__);
                RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
            }
        } else {
            LOGE("[%s] SIM ID is invalid", __FUNCTION__);
            RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
        }
        FREEIF(rsp_ptr);
        free(pcscf);
    }
}

void querySuggestionTimeFromRds(const reqSetupConf_t *pReqSetupConfig,
    MTK_RIL_Data_Call_Response_v11* response, RIL_Token t) {

    if (g_rds_conn_ptr == NULL) {
        LOGE("[%s] g_rds_conn_ptr is null", __FUNCTION__);
        return;
    }

    rds_get_retry_time_req_t* retryTimeReq = (rds_get_retry_time_req_t*)
        calloc(1, sizeof(rds_get_retry_time_req_t));
    rds_get_retry_time_rsp_t* retryTimeRsp = (rds_get_retry_time_rsp_t*)
        calloc(1, sizeof(rds_get_retry_time_rsp_t));
    assert(retryTimeReq != NULL);
    assert(retryTimeRsp != NULL);

    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    int sim_id = getValidSimId(rid, __FUNCTION__);
    if (sim_id != SIM_ID_INVALID) {
        retryTimeReq->ucsim_id = sim_id;
        retryTimeReq->ifid = pReqSetupConfig->interfaceId;
        cpStr(retryTimeReq->apn,pReqSetupConfig->requestedApn,
            strlen(pReqSetupConfig->requestedApn),STRING_SIZE);
        cpStr(retryTimeReq->profile, pReqSetupConfig->profile,
            strlen(pReqSetupConfig->profile), STRING_SIZE);

        LOGD("[%s] query ePDG retry time: [%s, %d, %s]", __FUNCTION__,
            retryTimeReq->apn, retryTimeReq->ifid, retryTimeReq->profile);
        if (rds_if_req != NULL) {
            if (rds_if_req(RDS_USR_GSMRIL, RIL_RDS_REQ_GET_RETRY_TIME,
                    retryTimeReq, retryTimeRsp, g_rds_conn_ptr) == RDS_TRUE) {
                LOGD("[%s] fail cause: %d retry time: %d", __FUNCTION__,
                retryTimeRsp->failcause, retryTimeRsp->suggested_time);
                response->status = retryTimeRsp->failcause;
                storeModemSuggestedRetryTime(retryTimeRsp->suggested_time, response);
            }
        } else {
            LOGE("[%s] rds_if_req is null", __FUNCTION__);
        }
    } else {
        LOGE("[%d][%s] Invalid SIM ID %d", rid, __FUNCTION__, sim_id);
    }

    free(retryTimeReq);
    free(retryTimeRsp);
}

void updateDataAllowStatus(int slotId, bool allow) {
    char propertyVal[PROPERTY_VALUE_MAX] = {0};
    if (allow) {
        for (int i = 0; i < getSimCount(); i++) {
            getMSimProperty(i, PROP_DATA_ALLOW_STATUS, propertyVal);
            propertyVal[1] = '\0';
            if (i != slotId && atoi(propertyVal) != 0) {
                setMSimProperty(i, PROP_DATA_ALLOW_STATUS, "0");
            }
        }
        setMSimProperty(slotId, PROP_DATA_ALLOW_STATUS, "1");
    } else {
        setMSimProperty(slotId, PROP_DATA_ALLOW_STATUS, "0");
    }
    LOGD("[RILData_GSM][%s] SIM%d: %d", __FUNCTION__, slotId, allow);
}

bool isDataAllow(int slotId) {
    char propertyVal[PROPERTY_VALUE_MAX] = {0};
    getMSimProperty(slotId, PROP_DATA_ALLOW_STATUS, propertyVal);
    if (strlen(propertyVal) != 0 && atoi(propertyVal) == 1) {
        return true;
    }
    return false;
}

extern bool skipIratUrcToMal(const char *urc) {
    if (nIratAction == IRAT_ACTION_SOURCE_STARTED || nIratAction == IRAT_ACTION_SOURCE_FINISHED) {
        if (strStartsWith(urc, "+CGEV: NW DEACT") || strStartsWith(urc, "+CGEV: ME DEACT") ||
                strStartsWith(urc, "+CGEV: NW REACT") || strStartsWith(urc, "+CGEV: NW DETACH") ||
                strStartsWith(urc, "+CGEV: NW PDN DEACT") ||
                strStartsWith(urc, "+CGEV: ME PDN DEACT")) {
            LOGD("[%s] skip urc %s", __FUNCTION__, urc);
            return true;
        }
    }
    return false;
}

void notifyDedicatedBearerDeact(int rid, int cid) {
    int sim_id = getValidSimId(rid, __FUNCTION__);
    if (sim_id != SIM_ID_INVALID) {
        mal_datamngr_notify_dedicated_bearer_deact_rsp_ptr_t rsp_ptr =
                calloc(1, sizeof(mal_datamngr_notify_dedicated_bearer_deact_rsp_t));
        if (rsp_ptr == NULL) {
            LOGE("[%s] calloc for rsp_ptr failed!", __FUNCTION__);
            return;
        }

        if (mal_datamngr_notify_dedicated_bearer_deact_tlv != NULL) {
            int ret = mal_datamngr_notify_dedicated_bearer_deact_tlv (
                    mal_once(1, mal_cfg_type_sim_id, sim_id),
                    cid, rsp_ptr, 0, NULL, NULL, NULL);
            LOGD("[%d][%s] cid %d", rid, __FUNCTION__, cid);
        } else {
            LOGE("[%d][%s] function pointer is null", rid, __FUNCTION__);
        }
        FREEIF(rsp_ptr);
        RIL_UNSOL_RESPONSE(RIL_UNSOL_DEDICATE_BEARER_DEACTIVATED,
                        &cid, sizeof(int), rid);
    }
}

void requestGetImsDataCallInfo(void *data, size_t datalen, RIL_Token t) {
    UNUSED(datalen);
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    const int profileId = atoi(((const char **)data)[0]);
    const char* requestedApn = ((const char **)data)[1];
    const int protocol = get_protocol_type(((const char **)data)[2]);
    const int interfaceId = atoi(((const char **)data)[3]) - 1; // zero based
    bool bFound = false;

    LOGD("[%d][%s] profileId = %d, requestedApn = %s, protocol = %d, interfaceId = %d",
            rid, __FUNCTION__, profileId, requestedApn, protocol, interfaceId);

    // TODO: Thread safe
    // TODO: Need another way to check IMS PDN instead of using APN name direcly.
    // This logic is wrapped with operator 12, looks safe so far.
    // Condition 1: Is IMS PDN.
    // Condition 2: Active state is not inactive.
    // Condition 3: The protocol is matched.
    for (int i = 0; i < pdnInfoSize; i++) {
        if (RIL_DATA_PROFILE_IMS == profileId &&
                strcasestr(pdn_info[rid][i].apn, "ims") &&
                pdn_info[rid][i].active == DATA_STATE_LINKDOWN &&
                isCidTypeMatched(pdn_info[rid][i].cid, protocol, getRILChannelCtxFromToken(t))) {
            bFound = true;
            LOGD("[%d][%s] Found PDN[%d] %s info with state %d", rid, __FUNCTION__,
                    i, requestedApn, pdn_info[rid][i].active);
            break;
        }
    }

    if (bFound) {
        MTK_RIL_Data_Call_Response_v11* response = (MTK_RIL_Data_Call_Response_v11*)
                calloc(1, sizeof(MTK_RIL_Data_Call_Response_v11));
        assert(response != NULL);
        initialDataCallResponse(response, 1);
        createDataResponse(interfaceId, protocol, response, getRILIdByChannelCtx(getRILChannelCtxFromToken(t)));
        dumpDataResponse(response, "requestGetDataCallInfo dump response");
        RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(MTK_RIL_Data_Call_Response_v11));
        freeDataResponse(response);
        free(response);
    } else {
        LOGD("[%d][%s] Not found existing PDN %s", rid, __FUNCTION__, requestedApn);
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    }
}

void requestReuseImsDataCall(void *data, size_t datalen, RIL_Token t) {
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    RILChannelCtx * rilContext = getRILChannelCtxFromToken(t);

    int requestParamNumber = (datalen/sizeof(char*));
    const char* requestedApn = ((const char **)data)[2];
    const int protocol = get_protocol_type(((const char **)data)[6]);
    const int interfaceId = atoi(((const char **)data)[15]) - 1;

    int i = 0;
    int *activatedCidList = (int *) calloc(1, pdnInfoSize * sizeof(int));
    assert(activatedCidList != NULL);
    for (i = 0; i < pdnInfoSize; i++) {
        activatedCidList[i] = INVALID_CID;
    }

    LOGD("[%d][%s] requestedApn = %s, protocol = %d, interfaceId = %d",
            rid, __FUNCTION__, requestedApn, protocol, interfaceId);

    MTK_RIL_Data_Call_Response_v11* response = (MTK_RIL_Data_Call_Response_v11*)
            calloc(1, sizeof(MTK_RIL_Data_Call_Response_v11));
    assert(response != NULL);
    mal_datamngr_data_call_info_req_t* responseToMal = (mal_datamngr_data_call_info_req_t*)
            calloc(1, sizeof(mal_datamngr_data_call_info_req_t));
    assert(responseToMal != NULL);


    // TODO: Consider IMS PDN over ePDG case but not now, i.e. To query RAT to RDS.

    int activatedPdnNum = queryMatchedPdnWithSameApn(requestedApn, activatedCidList, rilContext);
    // The IA cid shall be 0 and only one
    int cid = activatedCidList[0];
    if (activatedPdnNum > 0 &&
            cid > INVALID_CID &&
            isCidTypeMatched(cid, protocol, rilContext)) {
        // TODO: Thread Safe
        pdn_info[rid][cid].active = DATA_STATE_ACTIVE;

        if (updatePdnAddressByCid(cid, rilContext) < 0) {
            LOGD("[%s] update PDP address of CID%d failed", __FUNCTION__, cid);
            // we do not goto error since we expect to update all activated PDP address here
        }
        if (0 == bindPdnToIntf(1, interfaceId, cid, rilContext)) {
            LOGD("[%s] bindPdnToIntf of CID%d failed", __FUNCTION__, cid);
        }
        updateDynamicParameter(rilContext);

        if (updateDns(rilContext) < 0) {
            LOGE("[%s] updateDns failed", __FUNCTION__);
        }

        configureNetworkInterface(interfaceId, ENABLE_CCMNI, rid);

        initialDataCallResponse(response, 1);
        int defaultCid = createDataResponse(interfaceId, protocol, response,
                getRILIdByChannelCtx(getRILChannelCtxFromToken(t)));
        dumpDataResponse(response, "requestReuseImsDataCall dump response");

        createMalDataResponse(interfaceId, defaultCid, response, responseToMal, rid);
        // Not possible to reuse IMS PDN over WIFI
        setRspEranType(RDS_RAN_MOBILE_3GPP, response, responseToMal, rid);
        dumpMalDataResponse(responseToMal, "requestReuseImsDataCall dump mal response");
        responseUnsolDataCallRspToMal(t, responseToMal, sizeof(mal_datamngr_data_call_info_req_t));

        RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(MTK_RIL_Data_Call_Response_v11));

        if (isOp12Support()) {
            getAndSendPcoStatus(rid, activatedCidList[0], NULL);
        }
        // The dedicated bearer info will be updated in updateDynamicParameter
        notifyLinkdownDedicatedBearer(cid, rid);
        ///@ }

    } else {
        LOGD("[%d][%s] Reuse IMS PDN failed! Shall not happened!!!",
            rid, __FUNCTION__);
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    }

    if (response != NULL) {
        freeDataResponse(response);
        free(response);
    }
    if (responseToMal!= NULL) {
        freeMalDataResponse(responseToMal);
        free(responseToMal);
    }
    FREEIF(activatedCidList);
}

void requestSetpropImsHandover(void *data, size_t datalen, RIL_Token t) {
    UNUSED(datalen);
    const char* value = ((const char **)data)[0];
    if (value != NULL) {
        property_set("vendor.ril.imshandover", value);
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    }
}

void setUnsolResponseFilterLinkCapacityEstimate(bool enable, RIL_Token t) {
    UNUSED(enable);
    UNUSED(t);
    LOGD("[%s] do nothing", __FUNCTION__);
}

void clearIaCacheAndDefineInvalidIa() {
    // Multi-PS attach Start
    int iaSimId;
    char* apnName = INVALID_APN_VALUE;

    GEMINI_MODE proj_type = getGeminiMode();
    if (proj_type != GEMINI_MODE_L_AND_L) {
        iaSimId = 0;
        setIaCache(iaSimId, "", "", 0, "", 0, "", "");

        RILChannelCtx *pChannel = getChannelCtxbyProxy();
        if (getAttachApnSupport() == ATTACH_APN_PARTIAL_SUPPORT) {  // partial solution
            LOGD("IA: Set APN name %s.", apnName);
            // The following is for partial solution
            // set to invalid apn and always attach mode to let ESM block PS attach
            definePdnCtx(NULL, apnName, get_protocol_type("IP"), 0, AUTHTYPE_NOT_SET,
                    "", "", pChannel);
        }
        setMSimProperty(iaSimId, PROPERTY_IA_APN_SET_ICCID, "");
    } else {
        for (iaSimId = 0; iaSimId < getSimCount(); iaSimId++) {
            setIaCache(iaSimId, "", "", 0, "", 0, "", "");

            RILChannelCtx *pChannel = getChannelCtxbyProxy();
            if (getAttachApnSupport() == ATTACH_APN_PARTIAL_SUPPORT) {  // partial solution
                LOGD("IA: Set APN name %s for SIM%d", apnName, iaSimId + 1);
                // The following is for partial solution
                // set to invalid apn and always attach mode to let ESM block PS attach
                definePdnCtx(NULL, apnName, get_protocol_type("IP"), 0, AUTHTYPE_NOT_SET,
                        "", "", pChannel);
            }
            setMSimProperty(iaSimId, PROPERTY_IA_APN_SET_ICCID, "");
        }
    }
    // Multi-PS attach End
}

int setFastdormancyState(int state, int slot) {
    ATResponse *p_response = NULL;
    int err = 0;
    char cmdLine[PROPERTY_VALUE_MAX] = {0};
    memset(cmdLine, 0, sizeof(cmdLine));
    RILChannelCtx *pChannel = getChannelCtxbyProxy();

    snprintf(cmdLine, sizeof(cmdLine), "AT+EFD=%d", state);
    err = at_send_command(cmdLine, &p_response, pChannel);

    if (isATCmdRspErr(err, p_response)) {
        AT_RSP_FREE(p_response);
        return 0;
    }

    AT_RSP_FREE(p_response);
    return 1;
}

int isSysEpdgSupport() {
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("persist.vendor.wfc.sys_wfc_support", property_value, "0");
    int ret = atoi(property_value);

    return ret;
}
