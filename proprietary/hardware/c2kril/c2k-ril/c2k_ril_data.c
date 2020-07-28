#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <cutils/properties.h>
#include <errno.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <linux/route.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <libmtkrilutils.h>
#include <telephony/mtk_rs.h>

#include "at_tok.h"
#include "c2k_ril_data.h"
#include "hardware/ccci_intf.h"
#include "ifcutils/ifc.h"
#include "misc.h"
#include "netutils/ifc.h"
#include "ril_callbacks.h"
#include "ril_sim.h"

// IMS
#include <mal.h>

/** external global variables and functions declaration begin **/
extern int s_md3_off;

/** external global variables and functions declaration end **/

/** local file MACRO defination begin **/
#define UNUSED(x) (x)   //eliminate "warning: unused parameter"

#define SIM_ID_INVALID (-1)
#define PCO_ID_STR_LEN (5)
#define PLMN_LEN (7)

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

#define MAX_AT_CMD_LENGTH 512

/** local file MACRO defination end **/

/** local file variable defination begin **/
static int sock_fd_v4 = -1;
static int sock_fd_v6 = -1;

// Store the last data call fail cause.
static int nLastDataCallFailCause = 0;

static char* pcL2pValue = "M-CCMNI";

static PdnInfo* pPdnInfo = NULL;
static int nMaxPdnSupport = 0;
static int nPdnInfoSize = 0;
static const struct timeval TIMEVAL_0 = { 0, 0 };
// [C2K] [IRAT] start
static const struct timeval TIMEVAL_10 = { 10, 0 };
static SyncPdnInfo* pSyncPdnInfo = NULL;
static int nDeactPdnCount = 0;
static int nReactPdnCount = 0;
static int nReactSuccCount = 0;
static int nIratDeactCount = 0;
static PDN_IRAT_ACTION nIratAction = IRAT_ACTION_UNKNOWN;
static PDN_IRAT_TYPE nIratType = IRAT_TYPE_UNKNOWN;
// [C2K] [IRAT] end

/** local file variable defination end **/

/** local file funtions declaration begin **/
void initialPdnInfoList(RILChannelCtx *pChannel);
void requestSetupDataCall(void * data, size_t datalen, RIL_Token t);
void requestDeactiveDataCall(void * data, size_t datalen, RIL_Token t);
void requestLastDataCallFailCause(void * data, size_t datalen, RIL_Token t);
void requestDataCallList(void * data, size_t datalen, RIL_Token t);
void confirmIratChange(void *data, size_t datalen, RIL_Token t);
void onDataCallListChanged();

void clearAllPdnInfo();
void clearPdnInfo(PdnInfo* info);
int getPdnCapability(RILChannelCtx *pChannel);
int getAvailableCid();
int get4GCapabilitySlot();
int getDefaultDataSim();
int definePdnContext(const RIL_Default_Bearer_VA_Config_Struct *pDefaultBearerVaConfig,
        const char *requestedApn, int protocol, int availableCid, int authType,
        const char *username, const char* password, RILChannelCtx *pChannel);
int activatePdn(int availableCid, RILChannelCtx *pChannel);
int updatePdnAddressByCid(int cid, RILChannelCtx *pChannel);

int updateDnsAndMtu(RILChannelCtx *pChannel);
void getAndSendPcoStatus(int cid);
int updateActiveStatus(RILChannelCtx *pChannel);
int bindPdnToInterface(int interfaceId, int cid, RILChannelCtx *pChannel);
void bindNetworkInterfaceWithModem(int interfaceId, int modemId);
void updateNetworkInterface(int interfaceId, int mask);
void configureNetworkInterface(int interfaceId, int isUp);
int isATCmdRspErr(int err, const ATResponse *p_response);

void onMePdnActive(void* urc);
void onPdnModified(void* urc);
void onPdnDeactResult(void* urc);

void initialDataCallResponse(MTK_RIL_Data_Call_Response_v11* responses, int length);
void fillDataCallResponse(int interfaceId, MTK_RIL_Data_Call_Response_v11* response);
void dumpDataResponse(MTK_RIL_Data_Call_Response_v11* response, const char* reason);

void openSocketsForNetworkInterfaceConfig();
void closeSocketsAfterNetworkInterfaceConfig();
void setStateOfNwInterface(int s, struct ifreq *ifr, int state, int clr);
void setIpv4AddrToNwInterface(int s, struct ifreq *ifr, const char *addr);
void setIpv6AddrToNwInterface(int s, struct ifreq *ifr, const char *addr);

int convertIpv6Address(char* pdp_addr, char* ipv6_addr, int isLinkLocal);
int getPdnAddressType(char* addr);
int getProtocolType(const char* protocol);
int getAuthType(const char* authTypeStr);
void updatePdnAddressWithAddr(int responseCid, char *addr);

char* getNetworkInterfaceName(int interfaceId);
int isInitialAttachApn(const char *requestedApn);

int queryMatchedPdnWithSameApn(const char* apn);
int apnMatcher(const char* existApn, const char* apn);

void onIratEvent(void* urc);
void onLwcgIratEvent(void* urc);
void onPdnSyncFromSourceRat(void* urc);
void onIratSourceStarted(int sourceRat, int targetRat, int type);
void onIratSourceFinished(int sourceRat, int targetRat, int type);
void onIratTargetStarted(int sourceRat, int targetRat, int type);
void onIratTargetFinished(int sourceRat, int targetRat, int type);
void onIratStateChanged(int sourceRat, int targetRat, int state,
        int isSourceRil);

void clearAllSyncPdnInfo();
void clearSyncPdnInfo(SyncPdnInfo* info);
void cleanupFailedPdns();
void cleanupPdnByCid(int cid);
void cleanupPdnsForFallback();

int getIratType(int sourceRat, int targetRat);
int getLwcgIratType(int sourceRat, int targetRat);
void onIratPdnReactSucc(int cid);
void handleDeactedOrFailedPdns();
void handleLwcgFailedPdns();
void deactivatePdnByCid(int cid);
void retryForFailedPdnByCid(int cid);
void retryForLwcgFailedPdnByCid(int cid);

int rebindPdnToInterface(int interfaceId, int cid, RILChannelCtx * pChannel);
void reconfigureNetworkInterface(int interfaceId);
void setNetworkTransmitState(int interfaceId, int state);
void resumeAllDataTransmit();
void suspendAllDataTransmit();
void onUpdateIratStatus();
void onResetIratStatus();
void requestSetInitialAttachApn(void * data, size_t datalen, RIL_Token t);

void requestDataIdle(RIL_Token t);
void requestDataAttach(RIL_Token t);
void doPsAttach(RILChannelCtx *p_channel);
void requestOrSendDataCallList(RIL_Token t);
void freeDataResponse(MTK_RIL_Data_Call_Response_v11* response);
void setNetworkInterfaceDown(const char* interfaceName);
// Supply modem suggested retry time to FW.
int getModemSuggestedRetryTime(int queryCid, RILChannelCtx *pChannel);

// LCE service start
void requestStartLce(void* data, size_t datalen, RIL_Token t);
void requestStopLce(void* data, size_t datalen, RIL_Token t);
void requestPullLceData(void* data, size_t datalen, RIL_Token t);
int setLceMode(int lceMode, int reportIntervalMs, RIL_LceStatusInfo* response,
    RILChannelCtx *pChannel);
void onLceDataResult(void* urc);
void requestSetLinkCapacityReportingCriteria(void* data, size_t datalen, RIL_Token t);
void onLinkCapacityEstimate(void* urc);
// LCE service end

void syncApnTable(void* data, size_t datalen, RIL_Token t);

// Multi-PS Attach
void requestDeactiveAllDataCall(void * data, size_t datalen, RIL_Token t);

/// IMS. @{
int prepareImsParamInfo(int interfaceId, RIL_Default_Bearer_VA_Config_Struct *imsParam);
int getImsParamInfo(int intfId,
        RIL_Default_Bearer_VA_Config_Struct * pDefaultBearerVaConfig);
void initialMalDataCallResponse(mal_datamngr_data_call_info_req_ptr_t responses, int length);
void fillMalDataCallResponse(MTK_RIL_Data_Call_Response_v11* response,
        mal_datamngr_data_call_info_req_ptr_t responseToMal);
int updateDefaultBearerInfo(int interfaceId, RILChannelCtx* rilchnlctx);
void updateDedicateDataResponse(int cid, MALRIL_Dedicate_Data_Call_Struct* response);
void responseUnsolDataCallRspToMal(const mal_datamngr_data_call_info_req_ptr_t pRespData);
void responseUnsolImsDataCallListToMal(RILChannelCtx* rilchnlctx,
        char *apn, int interfaceId, int rid);
void freeMalDataResponse(mal_datamngr_data_call_info_req_ptr_t response);
void dumpDefaultBearerConfig(const RIL_Default_Bearer_VA_Config_Struct *pDefaultBearerVaConfig);
void dumpDedicateDataResponse(MALRIL_Dedicate_Data_Call_Struct* response, const char* reason);
int getValidSimId();
// Sync PDN deactivation cause to MAL
void updatePdnDeactStateToMal(unsigned interfaceId, int state, char* cause);
void resetPdnDeactStateToMal(unsigned interfaceId, char* cause);
/// @}

/** local file funtions declaration end **/


/** local file functions implementation **/

void rilDataInitialization(RILChannelCtx *pChannel) {
    // Lwcg irat 92 modem need this command.
    at_send_command("AT+EPDNCTRANS=1", NULL, pChannel);
    initialPdnInfoList(pChannel);
}

// Need to be called when system init.
void initialPdnInfoList(RILChannelCtx *pChannel) {
    if (pPdnInfo == NULL) {
        nPdnInfoSize = getPdnCapability(pChannel); // cid0 is used in Irat.
        pPdnInfo = calloc(1, nPdnInfoSize * sizeof(PdnInfo));
        assert(pPdnInfo != NULL);
        clearAllPdnInfo();
        LOGD("[%s][RILData_Init] Pdn list size = %d.", __FUNCTION__, nPdnInfoSize);
    }
}

void clearPdnInfo(PdnInfo* info) {
    memset(info, 0, sizeof(PdnInfo));
    info->active = DATA_STATE_INACTIVE;
    info->interfaceId = INVALID_CID;
    info->cid = INVALID_CID;
    info->mtu = -1;
    // IMS @{
    info->bearerId = INVALID_CID;
    // @}
}

void clearAllPdnInfo() {
    if (pPdnInfo != NULL) {
        int i = 0;
        for (i = 0; i < nPdnInfoSize; i++) {
            clearPdnInfo(&pPdnInfo[i]);
        }
    }
}

int getPdnCapability(RILChannelCtx *pChannel) {
    ATResponse *p_response = NULL;
    int err = 0;

    char *line = NULL;
    ATLine *p_cur = NULL;
    int max_pdn_support_23G = 0;
    int max_pdn_support_4G = 0;
    int max_pdn_support_c2k = 0;

    err = at_send_command_singleline("AT+EGPCAP=?", "+EGPCAP:", &p_response,
            pChannel);
    if (p_response == NULL) {
        LOGE("[%s] p_response is NULL", __FUNCTION__);
        goto error;
    }
    // Resoponse: +EGPCAP:<Max PDP supported by 2/3G>, <Max PDN supported by 4G>, <Max PDN supported by c2k>
    // Resoponse: +EGPCAP:0,0,8
    if (isATCmdRspErr(err, p_response)) {
        LOGE("Not supoprt PDN capability command.");
    } else {
        line = p_response->p_intermediates->line;

        err = at_tok_start(&line);
        if (err < 0) {
            goto error;
        }

        err = at_tok_nextint(&line, &max_pdn_support_23G);
        if (err < 0) {
            goto error;
        }

        err = at_tok_nextint(&line, &max_pdn_support_4G);
        if (err < 0) {
            goto error;
        }
        err = at_tok_nextint(&line, &max_pdn_support_c2k);
        if (err < 0) {
            goto error;
        }
    }
    AT_RSP_FREE(p_response);
    LOGD("[RILData_Init] getPdnCapability: PDN capability is [ %d, %d, %d ]", max_pdn_support_23G,
            max_pdn_support_4G, max_pdn_support_c2k);
    return max_pdn_support_c2k;

error:
    AT_RSP_FREE(p_response);
    LOGE("[RILData_Init] getPdnCapability failed, current PDP capability [%d, %d %d]",
            max_pdn_support_23G, max_pdn_support_4G, max_pdn_support_c2k);
    return max_pdn_support_c2k;
}

// Get network interface name, return cc3mni for pure C2K project, return ccmni with Irat feature support.
char* getNetworkInterfaceName(int interfaceId) {
    char* ret = "";
    if (isSvlteSupport()) {
        int curSim = getCdmaSocketSlotId();
        int majorSim = get4GCapabilitySlot();
        if (curSim == majorSim) {
            ret = ccci_get_node_name(USR_NET_0 + interfaceId, MD_SYS1);
        } else {
            ret = ccci_get_node_name(USR_NET_0 + interfaceId, MD_SYS3);
        }
    } else if (isSrlteSupport()) {
        ret = ccci_get_node_name(USR_NET_0 + interfaceId, MD_SYS1);
    } else {
        ret = ccci_get_node_name(USR_NET_0 + interfaceId, MD_SYS3);
    }
    return ret;
}

int getAvailableCid() {
    int i = 1;
    //start from index 1 since we should not select CID0
    //CID0 is for network attach given PDN connection
    for (i = 1; i < nPdnInfoSize; i++) {
        LOGD("[%s] i=%d, nPdnInfoSize = %d, pPdnInfo[i].interfaceId = %d, pPdnInfo[i].active = %d.",
                __FUNCTION__, i, nPdnInfoSize, pPdnInfo[i].interfaceId, pPdnInfo[i].active);
        if (pPdnInfo[i].interfaceId == INVALID_CID
            && pPdnInfo[i].active == DATA_STATE_INACTIVE) {
            LOGD("[%s] The available CID is %d.",__FUNCTION__, i);
            return i;
        }
    }

    return INVALID_CID;
}

void requestSetupDataCall(void * data, size_t datalen, RIL_Token t) {
    const int requestParamNumber = (datalen / sizeof(char*));
    const char* radioType = ((const char **)data)[0];
    const int profile = atoi(((const char **) data)[1]);
    const char* apn = ((const char **) data)[2];
    const char* username = ((const char **) data)[3];
    const char* password = ((const char **) data)[4];
    const int authType = getAuthType(((const char **) data)[5]);
    const int protocol = getProtocolType(((const char **) data)[6]);
    const char* roamingProtocol = ((const char **)data)[7];
    const int supportedApnTypesBitmap = atoi(((const char **)data)[8]);
    const int bearerBitmap = atoi(((const char **)data)[9]);
    const int modemCognitive = atoi(((const char **)data)[10]);
    const int mtu = atoi(((const char **)data)[11]);
    const int mvnoType = atoi(((const char **)data)[12]);
    const char* mvnoMatchData = ((const char **)data)[13];
    const int roamingAllowed = atoi(((const char **)data)[14]);
    const int interfaceId = atoi(((const char **)data)[15]) - 1;
    int availableCid = INVALID_CID;

    LOGI("[%s][RILData_Setup] datalen=%d, parameter number=%d, profile=%d, interfaceId=%d, apn=[%s]",
            __FUNCTION__, datalen, requestParamNumber, profile, interfaceId, apn);

    MTK_RIL_Data_Call_Response_v11* response = (MTK_RIL_Data_Call_Response_v11*) calloc(1,
            sizeof(MTK_RIL_Data_Call_Response_v11));
    assert(response != NULL);
    mal_datamngr_data_call_info_req_ptr_t responseToMal = NULL;

    initialDataCallResponse(response, 1);
    // Modem suggested retry time.
    int suggestedRetryTime = NO_SUGGESTED_TIME;
    int isPdpFailCauseUpdateNeeded = 0;
    RILChannelCtx* pChannel = getRILChannelCtxFromToken(t);

    // IMS @{
    RIL_Default_Bearer_VA_Config_Struct *pImsParam = NULL;
    if (profile == RIL_DATA_PROFILE_IMS ||
            profile == RIL_DATA_PROFILE_VENDOR_EMERGENCY) {
        // To read IMS request parameter from MAL.
        pImsParam = (RIL_Default_Bearer_VA_Config_Struct *)
                calloc(1, sizeof(RIL_Default_Bearer_VA_Config_Struct));
        assert(pImsParam != NULL);
        if (prepareImsParamInfo(interfaceId, pImsParam) != 0) {
            LOGE("get ims param failed!!");
            goto error;
        }
        dumpDefaultBearerConfig(pImsParam);

        // Prepare response to MAL
        responseToMal = (mal_datamngr_data_call_info_req_ptr_t) calloc(1,
                sizeof(mal_datamngr_data_call_info_req_t));
        assert(responseToMal != NULL);
        initialMalDataCallResponse(responseToMal, 1);
    }
    /// @}

    // Reuse the previous activated PDP in case phone process crash but RILD
    // and MD still maintain a PDP context, If we want to reactivate a new PDP,
    // it will fails on HRPD network because C2K only support single PDP on HRPD.
    int matchedCid = queryMatchedPdnWithSameApn(apn);
    if (matchedCid != INVALID_CID) {
        availableCid = matchedCid;
        LOGD("[%s][RILData_Setup] reuse cid %d.", __FUNCTION__, matchedCid);
    } else {
        // Get available CID, define PDN context and configure protocol settings.
        availableCid = getAvailableCid();
        LOGI("[%s][RILData_Setup] availableCid=%d.", __FUNCTION__, availableCid);
        if (0 == definePdnContext(pImsParam,
                apn, protocol, availableCid, authType,
                username, password, pChannel)) {
            isPdpFailCauseUpdateNeeded = 1;
            goto error;
        }

        if (0 == activatePdn(availableCid, pChannel)) {
            isPdpFailCauseUpdateNeeded = 1;
            suggestedRetryTime = getModemSuggestedRetryTime(availableCid, pChannel);
            goto error;
        }
    }

    if (updatePdnAddressByCid(availableCid, pChannel) < 0) {
        response->status = nLastDataCallFailCause = PDP_FAIL_ERROR_UNSPECIFIED;
        goto error;
    }
    if (0 == bindPdnToInterface(interfaceId, availableCid, pChannel)) {
        response->status = nLastDataCallFailCause = PDP_FAIL_ERROR_UNSPECIFIED;
        goto error;
    }

    if (updateDnsAndMtu(pChannel) < 0) {
        response->status = nLastDataCallFailCause = PDP_FAIL_ERROR_UNSPECIFIED;
        goto error;
    }

    bindNetworkInterfaceWithModem(interfaceId, MD_SYS3);

    configureNetworkInterface(interfaceId, ENABLE_CCMNI);
    // Store APN name when PDN activated successfully.
    sprintf(pPdnInfo[availableCid].apn, "%s", apn);

    // Assign correct eran type for C2K.
    response->rat = RDS_RAN_MOBILE_3GPP2;

    // Query pcscf address for IMS
    if (isImsSupport()) {
        updateDefaultBearerInfo(interfaceId, pChannel);
    }

    fillDataCallResponse(interfaceId, response);
    dumpDataResponse(response, "requestSetupDataCall dump response");

    // Query pcscf address
    if (profile == RIL_DATA_PROFILE_IMS ||
            profile == RIL_DATA_PROFILE_VENDOR_EMERGENCY) {
        /// M: Deliver IMS dedicated response back to MAL.
        fillMalDataCallResponse(response, responseToMal);
        dumpDedicateDataResponse(&responseToMal->defaultBearer,
                                 "requestSetupDataCall dump ims response");
        responseUnsolDataCallRspToMal(responseToMal);
    }
    /// @}

    RIL_onRequestComplete(t, RIL_E_SUCCESS, response,
            sizeof(MTK_RIL_Data_Call_Response_v11));
    getAndSendPcoStatus(availableCid);
    goto finish;

error:
    LOGE("[%s][RILData_Setup] failed. availableCid = %d.", __FUNCTION__, availableCid);
    if (availableCid != INVALID_CID) {
        clearPdnInfo(&pPdnInfo[availableCid]);
    }

    if (isPdpFailCauseUpdateNeeded && nLastDataCallFailCause != 0) {
        response->status = nLastDataCallFailCause;
    }

    response->suggestedRetryTime = suggestedRetryTime;

    RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(MTK_RIL_Data_Call_Response_v11));

finish:
    freeDataResponse(response);
    free(response);
    freeMalDataResponse(responseToMal);
    free(responseToMal);
    LOGD("[%s][RILData_Setup] finished. availableCid = %d.", __FUNCTION__, availableCid);
    // IMS
    FREEIF(pImsParam);
    return;
}

void requestDeactiveDataCall(void * data, size_t datalen, RIL_Token t) {
    UNUSED(datalen);
    char *cmd = NULL;
    int err = 0;
    ATResponse *p_response = NULL;
    int i = 0;
    int interfaceId = atoi(((const char **) data)[0]);

    LOGD("[%s][RILData_Deact] interfaceId=%d", __FUNCTION__, interfaceId);

    // AT+CGACT=<state>,<cid>;  <state>:0-deactivate;1-activate
    for (i = 0; i < nPdnInfoSize; i++) {
        if (pPdnInfo[i].interfaceId == interfaceId) {
            asprintf(&cmd, "AT+CGACT=0,%d", i);
            err = at_send_command_notimeout(cmd, &p_response, getRILChannelCtxFromToken(t));
            free(cmd);
            if (p_response == NULL) {
                LOGE("[%s] p_response is NULL", __FUNCTION__);
                goto error;
            }
            if (isATCmdRspErr(err, p_response)) {
                if (p_response == NULL) {
                    LOGE("[%s][RILData_Deact] p_response is null!", __FUNCTION__);
                    goto error;
                }
                int errCode = at_get_cme_error(p_response);
                if (errCode == PS_CMD_CONFLICT_C2K_AP_DEACT_RETRY_NEEDED) {
                    pPdnInfo[i].active = DATA_STATE_NEED_DEACT;
                }
                goto error;
            }
            AT_RSP_FREE(p_response);
        }
    }

    configureNetworkInterface(interfaceId, DISABLE_CCMNI);
    for (i = 0; i < nPdnInfoSize; i++) {
        if (pPdnInfo[i].interfaceId == interfaceId) {
            clearPdnInfo(&pPdnInfo[i]);
        }
    }

    LOGD("[%s][RILData_Deact] succeed.", __FUNCTION__);

    //response deactivation result first then do re-attach
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;

error:
    LOGE("[%s][RILData_Deact] failed.", __FUNCTION__);
    configureNetworkInterface(interfaceId, DISABLE_CCMNI);
    RIL_onRequestComplete(t, RIL_E_NETWORK_ERR, NULL, 0);
    AT_RSP_FREE(p_response);
}

void requestAllowData(void* data, size_t datalen, RIL_Token t) {
    UNUSED(datalen);
    int isDataAttach = ((int *) data)[0];

    LOGD("[%s][RILData_C2K_Util] isDataAttach= %x", __FUNCTION__, isDataAttach);

    // Multi-PS Attach
    if (isMultiPsAttachSupport()) {
        // Report URC to framework to update attach config.
        RIL_onUnsolicitedResponse(RIL_UNSOL_DATA_ALLOWED, &isDataAttach, sizeof(int));
    } else {
        if (isDataAttach == REQUEST_DATA_ATTACH) {
            requestDataAttach(t);
        } else {
            requestDataIdle(t);
        }
    }

    // For C2K RIL, response detach or attach directly.
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
}

void requestDataAttach(RIL_Token t)
{
    doPsAttach(getRILChannelCtxFromToken(t));
}

void requestDataIdle(RIL_Token t) {
    char *cmd = NULL;
    int err = 0;
    ATResponse *p_response = NULL;

    asprintf(&cmd, "AT+CGATT=0");
    err = at_send_command(cmd, &p_response, getRILChannelCtxFromToken(t));
    free(cmd);
    if (p_response == NULL) {
        LOGE("[%s] p_response is NULL", __FUNCTION__);
        return;
    }
    if (isATCmdRspErr(err, p_response)) {
        LOGE("[%s][RILData_C2K_Util] fail with CGATT, err = %d", __FUNCTION__, err);
    }
    AT_RSP_FREE(p_response);
}

int get4GCapabilitySlot() {
    char tmp[PROPERTY_VALUE_MAX] = { 0 };
    int simId = 0;

    property_get(PROPERTY_4G_SIM, tmp, "1");
    simId = atoi(tmp);
    return simId;
}

void doPsAttach(RILChannelCtx *p_channel) {
    char *cmd = NULL;
    int err = 0;
    ATResponse *p_response = NULL;

    asprintf(&cmd, "AT+CGATT=1");
    err = at_send_command(cmd, &p_response, p_channel);
    free(cmd);
    if (p_response == NULL) {
        LOGE("[%s] p_response is NULL", __FUNCTION__);
        return;
    }
    if (isATCmdRspErr(err, p_response)) {
        LOGE("[%s][RILData_C2K_Util] fail with CGATT, err = %d", __FUNCTION__, err);
    }
    AT_RSP_FREE(p_response);
}

void combineDataAttach(RILChannelCtx *p_channel) {
    int curSim = getCdmaSocketSlotId();
    int defDataSim = getDefaultDataSim();
    int capSim = get4GCapabilitySlot();
    int needAttach = 0;
    // No need to check sim inserted status as CTA case maybe set
    // RADIO_ON with AT+CPON, and then hot plugin CT 3G will have
    // no chance to send CGATT command to do attach.

    // Multi-PS Attach
    if (isMultiPsAttachSupport()) {
        LOGD("[%s][RILData_C2K_MPS] Multi PS always attach", __FUNCTION__);
        needAttach = 1;
    } else if (isSvlteSupport() || isSrlteSupport()) {
        if (defDataSim == 0) {
            // Data unset, attach if the current SIM is major SIM
            if (capSim == curSim) {
                LOGD("[%s][RILData_C2K_SRLTE] C1", __FUNCTION__);
                needAttach = 1;
            } else {
                LOGD("[%s][RILData_C2K_SRLTE] C2", __FUNCTION__);
            }
        } else {
            if (defDataSim == curSim) {
                LOGD("[%s][RILData_C2K_SRLTE] C3", __FUNCTION__);
                needAttach = 1;
            } else {
                LOGD("[%s][RILData_C2K_SRLTE] C4", __FUNCTION__);
            }
        }
        // specific check for hot-plug case, check another sim status,
        // if another sim is attach, we needn't send attach before radio on.
        if (needAttach) {
            char record[PROPERTY_VALUE_MAX] = { 0 };
            property_get(RP_ATTACH_SIM, record, "-1");
            int attachSim = atoi(record) + 1;
            if (attachSim > 0 && attachSim != curSim) {
                needAttach = 0;
                LOGI("[%s][RILData_C2K_SRLTE] skipped! attachSim:%d.", __FUNCTION__, attachSim);
            }
        }
    } else {
        LOGD("[%s][RILData_C2K] unexpected project.", __FUNCTION__);
    }

    LOGI("[%s][RILData_C2K] capSim = %d, curSim = %d, defDataSim = %d,"
        " needAttach = %d.", __FUNCTION__, capSim, curSim, defDataSim, needAttach);
    if (needAttach) {
        doPsAttach(p_channel);
    }
}

int getDefaultDataSim() {
    int i = 0;
    char iccid[PROPERTY_VALUE_MAX] = {0};
    char defaultData[PROPERTY_VALUE_MAX] = {0};

    property_get(PROPERTY_RIL_DATA_ICCID, defaultData, "");
    if (strlen(defaultData) == 0 || strcmp("N/A", defaultData) == 0) {
        LOGD("[RILData_C2K] Default Data not set");
        return 0;
    }

    for (i = 0; i < getSimCount(); i++) {
        property_get(PROPERTY_ICCID_SIM[i], iccid, "");
        if (strlen(iccid) == 0 || strcmp("N/A", iccid) == 0) {
            LOGD("[RILData_C2K] SIM%d not inserted", i);
            continue;
        }
        if (strStartsWith(iccid, defaultData)) {
            return (i + 1);
        }
    }
    LOGD("[RILData_C2K] Not found");
    return 0;
}

//TODO: implement for specific fail causes.
void requestLastDataCallFailCause(void * data, size_t datalen, RIL_Token t) {
    UNUSED(data);
    UNUSED(datalen);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, &nLastDataCallFailCause,
            sizeof(nLastDataCallFailCause));
}

void requestDataCallList(void * data, size_t datalen, RIL_Token t) {
    UNUSED(data);
    UNUSED(datalen);
    requestOrSendDataCallList(t);
}

void requestOrSendDataCallList(RIL_Token t) {
    ATResponse *p_response = NULL;
    ATLine *p_cur = NULL;
    int err = 0;
    char *out = NULL;
    int i = 0;
    MTK_RIL_Data_Call_Response_v11* responsesOfActive = NULL;
    int activeCount = 0;
    int sim_id = getValidSimId();
    MTK_RIL_Data_Call_Response_v11* responses = (MTK_RIL_Data_Call_Response_v11*) calloc(1,
            nPdnInfoSize * sizeof(MTK_RIL_Data_Call_Response_v11));
    mal_datamngr_data_call_info_req_ptr_t responsesToMal = (mal_datamngr_data_call_info_req_ptr_t)
            calloc(1, nPdnInfoSize * sizeof(mal_datamngr_data_call_info_req_t));
    if (responses == NULL || responsesToMal == NULL) {
        goto error;
    }
    initialDataCallResponse(responses, nPdnInfoSize);
    initialMalDataCallResponse(responsesToMal, nPdnInfoSize);

    RILChannelCtx *pChannel = NULL;
    if (t == NULL) {
        RIL_SOCKET_ID rid = getCdmaSocketSlotId() - 1;
        pChannel = getChannelCtxbyProxy(rid);
    } else {
        pChannel = getRILChannelCtxFromToken(t);
    }

    updateActiveStatus(pChannel);

    for (i = 0; i < nPdnInfoSize; i++) {
        updatePdnAddressByCid(i, pChannel);
        // Not to check error since we expect to query all PDN connections even if error occurred
        // IMS
        if (isImsSupport()) {
            if (pPdnInfo[i].active == DATA_STATE_ACTIVE) {
                updateDefaultBearerInfo(pPdnInfo[i].interfaceId, pChannel);
            }
        }
    }

    if (updateDnsAndMtu(pChannel) < 0) {
        LOGE("[%s][RILData_Query] updateDnsAndMtu failed - XXXXXX", __FUNCTION__);
        goto error;
    }

    for (i = 0; i < nPdnInfoSize; i++) {
        fillDataCallResponse(i, &responses[i]);
        if (responses[i].active == DATA_STATE_ACTIVE) {
            ++activeCount;
        }
    }

    LOGI("[%s][RILData_Query] activeCount = %d", __FUNCTION__, activeCount);
    if (activeCount > 0) {
        responsesOfActive = (MTK_RIL_Data_Call_Response_v11*) calloc(1,
                activeCount * sizeof(MTK_RIL_Data_Call_Response_v11));
        assert(responsesOfActive != NULL);

        int index = 0;
        for (i = 0; i < nPdnInfoSize; i++) {
            if (responses[i].active == DATA_STATE_ACTIVE) {
                memcpy(&responsesOfActive[index], &responses[i],
                                    sizeof(MTK_RIL_Data_Call_Response_v11));
                fillMalDataCallResponse(&responsesOfActive[index], &responsesToMal[index]);
                ++index;
                dumpDataResponse(&responses[i], "requestOrSendDataCallList dump interface status");
                dumpDedicateDataResponse(&responsesToMal[i].defaultBearer,
                        "requestOrSendDataCallList dump ims status");
            }
        }
    }

    if (t) {
        RIL_onRequestComplete(t, RIL_E_SUCCESS,
                activeCount > 0 ? responsesOfActive : NULL,
                activeCount * sizeof(MTK_RIL_Data_Call_Response_v11));
    } else {
        RIL_onUnsolicitedResponse(RIL_UNSOL_DATA_CALL_LIST_CHANGED,
                activeCount > 0 ? responsesOfActive : NULL,
                activeCount * sizeof(MTK_RIL_Data_Call_Response_v11));
        // Sync IMS data call list to MAL
        if (isImsSupport()) {
            LOGI("[RILData_Query] Sync IMS data call list to MAL");
            mal_datamngr_data_call_info_rsp_ptr_t rsp_ptr =
                    calloc(1, activeCount * sizeof(mal_datamngr_data_call_info_rsp_t));
            assert(rsp_ptr != NULL);
            mal_datamngr_data_call_info_req_ptr_t req_ptr[activeCount];
            for (int i = 0; i < activeCount; i++) {
                req_ptr[i] = &responsesToMal[i];
                LOGD("[%s] The pdn memory address: req_ptr[%d]=%p , responsesToMal[%d]=%p",
                        __FUNCTION__, i, req_ptr[i], i, &responsesToMal[i]);
            }
            if (sim_id != SIM_ID_INVALID) {
                if (mal_datamngr_notify_data_call_list_tlv != NULL) {
                    int ret = mal_datamngr_notify_data_call_list_tlv(
                            mal_once(1, mal_cfg_type_sim_id, sim_id), activeCount,
                            req_ptr, (mal_datamngr_data_call_info_rsp_ptr_t *) rsp_ptr,
                            NULL, NULL, NULL, NULL);
                    LOGD("[%s] Call mal_datamngr_notify_data_call_list_tlv success", __FUNCTION__);
                } else {
                    LOGE("[%s] mal_datamngr_notify_data_call_list_tlv is null", __FUNCTION__);
                }
            }
            FREEIF(rsp_ptr);
        }
    }
    goto finish;

error:
    LOGE("[%s][RILData_Query] failed - XXXXXX.", __FUNCTION__);
    if (t != NULL) {
        RIL_onRequestComplete(t, RIL_E_NETWORK_ERR, NULL, 0);
    } else {
        RIL_onUnsolicitedResponse(RIL_UNSOL_DATA_CALL_LIST_CHANGED, NULL, 0);
        // Sync IMS data call list to MAL
        if (isImsSupport()) {
            LOGE("[RILData_Query] Sync Null IMS data call list to MAL");
            mal_datamngr_data_call_info_rsp_ptr_t rsp_ptr =
                    calloc(1, activeCount * sizeof(mal_datamngr_data_call_info_rsp_t));
            assert(rsp_ptr != NULL);
            if (sim_id != SIM_ID_INVALID) {
                if (mal_datamngr_notify_data_call_list_tlv != NULL) {
                    int ret = mal_datamngr_notify_data_call_list_tlv(
                            mal_once(1, mal_cfg_type_sim_id, sim_id), 0,
                            NULL, (mal_datamngr_data_call_info_rsp_ptr_t *) rsp_ptr,
                            NULL, NULL, NULL, NULL);
                    LOGD("[%s] Call mal_datamngr_notify_data_call_list_tlv success", __FUNCTION__);
                } else {
                    LOGE("[%s] mal_datamngr_notify_data_call_list_tlv is null", __FUNCTION__);
                }
            }
            FREEIF(rsp_ptr);
       }
    }

    AT_RSP_FREE(p_response);

finish:
    if (responses != NULL) {
        for (i = 0; i < nPdnInfoSize; i++) {
            freeDataResponse(&responses[i]);
            freeMalDataResponse(&responsesToMal[i]);
        }
        free(responses);
        free(responsesToMal);
    }

    if (responsesOfActive != NULL) {
        LOGE("[%s]free responsesOfActive", __FUNCTION__);
        free(responsesOfActive);
    }
    return;
}

void onDataCallListChanged() {
    requestOrSendDataCallList(NULL);
}

void onMePdnActive(void* urc) {
    // Initial attached APN activated, keep although no initail attached APN in C2K.
    char *p = strstr(urc, ME_PDN_URC);
    if (p == NULL) {
        LOGE("[RILData_C2K_IRAT] onMePdnActive invalid URC.");
        goto finish;
    }
    p = p + strlen(ME_PDN_URC);
    int activatedCid = atoi(p);
    LOGD("[RILData_C2K_IRAT] onMePdnActive: CID = %d, state = %d, IRAT state = %d.",
            activatedCid, pPdnInfo[activatedCid].active, nIratAction);

    if (nIratAction == IRAT_ACTION_TARGET_STARTED) {
        onIratPdnReactSucc(activatedCid);
    } else {
        if (pPdnInfo[activatedCid].active == DATA_STATE_INACTIVE) {
            pPdnInfo[activatedCid].active = DATA_STATE_LINKDOWN; // Update with link down state.
            pPdnInfo[activatedCid].cid = activatedCid;
        }
    }

finish:
    free(urc);
}

void onPdnModified(void* urc) {
    int err = 0;
    int cid = INVALID_CID;
    int cause = -1;
    char *line = urc;

    //+CGEV: NW MODIFY <cid>, <cause>, 0, ex:+CGEV: NW MODIFY 1,99,0
    LOGD("[RILData_URC] onPdnModified receive %s", line);
    err = at_tok_start(&line);
    if (err < 0) {
        LOGE("[RILData_URC] onPdnModified return with error %d.", err);
        goto error;
    }

    if ((strlen(line) > 0)
        && (strncmp(NW_MODIFY_URC, line, strlen(NW_MODIFY_URC)) == 0)) {
        line += strlen(NW_MODIFY_URC);
        err = at_tok_nextint(&line, &cid);
        if (err < 0) {
            LOGE("ERROR occurs when parsing cid.");
            goto error;
        }

        if (at_tok_hasmore(&line)) {
            err = at_tok_nextint(&line, &cause);
            if (err < 0) {
                LOGE("ERROR occurs when parsing cause.");
                goto error;
            }
            if (cause == MODIFY_CAUSE_PPP_RENEGOTIATION) {
                RIL_SOCKET_ID rid = getCdmaSocketSlotId() - 1;
                RILChannelCtx *pChannel = getChannelCtxbyProxy(rid);
                if (updatePdnAddressByCid(cid, pChannel) < 0) {
                    LOGE("ERROR when updating PDN address caused by NW modify.");
                    goto error;
                }
                if (updateDnsAndMtu(pChannel) < 0) {
                    LOGE("ERROR when updating DNS caused by NW modify.");
                    goto error;
                }

                updateNetworkInterface(pPdnInfo[cid].interfaceId, RESET_ALL_ADDRESSES);

                if (s_md3_off) {
                    RIL_onUnsolicitedResponse(RIL_UNSOL_DATA_CALL_LIST_CHANGED, NULL, 0);
                } else {
                    RIL_requestProxyTimedCallback(onDataCallListChanged, NULL,
                            &TIMEVAL_0, DATA_CHANNEL);
                }
            }
        } else {
            LOGE("ERROR because no modify cause.");
            goto error;
        }
    } else {
        LOGE("[RILData_URC] onPdnModified return because invalid URC.");
        goto error;
    }

error:
    free(urc);
    return;
}

void onPdnDeact(void* urc) {
    // Ignore PDN deact message report from source RAT during Irat.
    if (nIratAction == IRAT_ACTION_SOURCE_STARTED || nIratAction == IRAT_ACTION_SOURCE_FINISHED) {
        LOGI("[RILData_C2K_URC] Ignore source PDN deact during IRAT: status = %d.", nIratAction);
        onUpdateIratStatus();
    } else {
        LOGD("[RILData_C2K_URC] PDN deact by NW/ME.");
        onPdnDeactResult(urc);
    }
    free(urc);
}

void onPdnDeactResult(void* urc) {
    int err = 0;
    int cid = INVALID_CID;
    char *line = urc;

    //C2K: +CGEV: NW PDN DEACT <cid>
    //C2K: +CGEV: ME PDN DEACT <cid>

    LOGD("[RILData_URC] onPdnDeactResult receive %s", line);
    err = at_tok_start(&line);
    if (err < 0) {
        LOGE("onPdnDeactResult return with error %d.", err);
        goto error;
    }

    if (strlen(line) > 0) {
        LOGD("onPdnDeactResult token start [%s]", line);

        if (strncmp(ME_PDN_DEACT_URC, line, strlen(ME_PDN_DEACT_URC)) == 0) {
            line += strlen(ME_PDN_DEACT_URC);
        } else if (strncmp(NW_PDN_DEACT_URC, line, strlen(NW_PDN_DEACT_URC))
                == 0) {
            line += strlen(NW_PDN_DEACT_URC);
        }
        LOGD("onPdnDeactResult skip prefix [%s]", line);

        err = at_tok_nextint(&line, &cid); //cid
        if (err < 0) {
            LOGE("ERROR occurs when parsing first parameter");
            goto error;
        }

        LOGD("[RILData_URC] onPdnDeactResult: cid=%d", cid);

        int interfaceId = pPdnInfo[cid].interfaceId;
        if (interfaceId == INVALID_CID) {
            LOGE("[RILData_URC] onPdnDeactResult goto error because of INVALID interfaceId=%d ", interfaceId);
            goto error;
        }

        configureNetworkInterface(interfaceId, DISABLE_CCMNI);
        clearPdnInfo(&pPdnInfo[cid]);
        if (s_md3_off) {
            RIL_onUnsolicitedResponse(RIL_UNSOL_DATA_CALL_LIST_CHANGED, NULL, 0);
        } else {
            RIL_requestProxyTimedCallback(onDataCallListChanged, NULL, &TIMEVAL_0, DATA_CHANNEL);
        }
    } else {
        LOGE("[RILData_URC] onPdnDeactResult return because invalid URC.");
    }

error:
    return;
}

void initialDataCallResponse(MTK_RIL_Data_Call_Response_v11* responses, int length) {
    int i = 0;
    for (i = 0; i < length; i++) {
        memset(&responses[i], 0, sizeof(MTK_RIL_Data_Call_Response_v11));
        responses[i].status = PDP_FAIL_ERROR_UNSPECIFIED;
        responses[i].cid = INVALID_CID;
    }
}

void fillDataCallResponse(int interfaceId, MTK_RIL_Data_Call_Response_v11* response) {
    int i = 0, nDnsesLen = 0;
    char addressV4[MAX_IPV4_ADDRESS_LENGTH] = { 0 };
    char addressV6[MAX_IPV6_ADDRESS_LENGTH] = { 0 };
    char dnsV4[MAX_NUM_DNS_ADDRESS_NUMBER][MAX_IPV4_ADDRESS_LENGTH] = { { 0 } };
    char dnsV6[MAX_NUM_DNS_ADDRESS_NUMBER][MAX_IPV6_ADDRESS_LENGTH] = { { 0 } };

    int v4DnsLength = 0;
    int v6DnsLength = 0;
    int cid = 0;

    // IMS
    int pcscfLength = 0;
    char pcscf[MAX_PCSCF_NUMBER][MAX_IPV6_ADDRESS_LENGTH] = {{0}};

    response->active = DATA_STATE_INACTIVE;
    for (i = 0; i < nPdnInfoSize; i++) {
        if (pPdnInfo[i].interfaceId == interfaceId) {
            cid = i;

            //There is cid bind to the interface of the response, so set to active
            response->active = pPdnInfo[i].active;
            asprintf(&response->ifname, "%s", getNetworkInterfaceName(interfaceId));
            LOGD("[%s]interfaceId=%d, cid=%d, ifrName = %s",
                    __FUNCTION__, interfaceId, cid, response->ifname);

            if (strlen(pPdnInfo[i].addressV4) > 0) {
                strncpy(addressV4, pPdnInfo[i].addressV4, MAX_IPV4_ADDRESS_LENGTH - 1);
                addressV4[MAX_IPV4_ADDRESS_LENGTH - 1] = '\0';
            }
            if (strlen(pPdnInfo[i].addressV6) > 0) {
                strncpy(addressV6, pPdnInfo[i].addressV6, MAX_IPV6_ADDRESS_LENGTH - 1);
                addressV6[MAX_IPV6_ADDRESS_LENGTH - 1] = '\0';
            }

            int j = 0;
            for (j = 0; j < MAX_NUM_DNS_ADDRESS_NUMBER; j++) {
                if (strlen(pPdnInfo[i].dnsV4[j]) > 0) {
                    strncpy(dnsV4[j], pPdnInfo[i].dnsV4[j], MAX_IPV4_ADDRESS_LENGTH - 1);
                    dnsV4[j][MAX_IPV4_ADDRESS_LENGTH - 1] = '\0';
                    v4DnsLength += strlen(pPdnInfo[i].dnsV4[j]);
                    if (j != 0)
                        ++v4DnsLength; //add one space
                }
                if (strlen(pPdnInfo[i].dnsV6[j]) > 0) {
                    strncpy(dnsV6[j], pPdnInfo[i].dnsV6[j], MAX_IPV6_ADDRESS_LENGTH - 1);
                    dnsV6[j][MAX_IPV6_ADDRESS_LENGTH - 1] = '\0';
                    v6DnsLength += strlen(pPdnInfo[i].dnsV6[j]);
                    if (j != 0 || v4DnsLength != 0)
                        ++v6DnsLength; //add one space
                }
            }

            response->mtu = pPdnInfo[i].mtu;
            // IMS @{
            response->rat = RDS_RAN_MOBILE_3GPP2;
            if (isImsSupport()) {
                for (j = 0; j < MAX_PCSCF_NUMBER; j++) {
                    if (strlen(pPdnInfo[i].pcscf[j]) > 0) {
                        strncpy(pcscf[j], pPdnInfo[i].pcscf[j], MAX_IPV6_ADDRESS_LENGTH - 1);
                        pcscf[j][MAX_IPV6_ADDRESS_LENGTH - 1] = '\0';
                        pcscfLength += strlen(pPdnInfo[i].pcscf[j]);
                        if (j != 0) {
                            ++pcscfLength;
                        }
                    }
                }
            }
            /// @}
        }
    }

    response->status = PDP_FAIL_NONE;
    response->cid = interfaceId;

    int addressV4Length = strlen(addressV4);
    int addressV6Length = strlen(addressV6);

    if (addressV4Length > 0 && addressV6Length > 0) {
        asprintf(&response->type, SETUP_DATA_PROTOCOL_IPV4V6);
        asprintf(&response->addresses, "%s %s", addressV4, addressV6);
    } else if (addressV6Length > 0) {
        asprintf(&response->type, SETUP_DATA_PROTOCOL_IPV6);
        asprintf(&response->addresses, "%s", addressV6);
    } else {
        asprintf(&response->type, SETUP_DATA_PROTOCOL_IP);
        asprintf(&response->addresses, "%s", addressV4);
    }
    asprintf(&response->gateways, "%s", response->addresses);

    nDnsesLen = (v4DnsLength + v6DnsLength + 1) * sizeof(char);
    response->dnses = calloc(1, nDnsesLen);
    assert(response->dnses != NULL);

    for (i = 0; i < MAX_NUM_DNS_ADDRESS_NUMBER; i++) {
        if (v4DnsLength > 0 && strlen(dnsV4[i]) > 0) {
            int currentLength = strlen(response->dnses);
            if (currentLength > 0)
                strncat(response->dnses + currentLength, " ", nDnsesLen - currentLength);

            strncat(response->dnses + strlen(response->dnses), dnsV4[i],
                    nDnsesLen - strlen(response->dnses));
        }

        if (v6DnsLength > 0 && strlen(dnsV6[i]) > 0) {
            int currentLength = strlen(response->dnses);
            if (currentLength > 0)
                strncat(response->dnses + currentLength, " ", nDnsesLen - currentLength);

            strncat(response->dnses + strlen(response->dnses), dnsV6[i],
                    nDnsesLen - strlen(response->dnses));
        }
    }

    if (isImsSupport()) {
        // Response
        response->pcscf = calloc(1, pcscfLength * sizeof(char) + 1);
        assert(response->pcscf != NULL);
        for (i = 0; i < MAX_PCSCF_NUMBER; i++) {
            if (pcscfLength > 0 && strlen(pcscf[i]) > 0) {
                int currentLength = strlen(response->pcscf);
                if (currentLength > 0) {
                    strncat(response->pcscf + currentLength, " ", 1);
                }
                strncat(response->pcscf + strlen(response->pcscf), pcscf[i], strlen(pcscf[i]));
            }
        }
    }
}

void freeDataResponse(MTK_RIL_Data_Call_Response_v11* response) {
    FREEIF(response->type);
    FREEIF(response->ifname);
    FREEIF(response->addresses);
    FREEIF(response->gateways);
    FREEIF(response->dnses);
    if (isImsSupport()) {
        FREEIF(response->pcscf);
    }
}

void dumpDataResponse(MTK_RIL_Data_Call_Response_v11* response, const char* reason) {
    LOGD("%s data call response: status=%d, suggestedRetryTime=%d, interfaceId=%d, active=%d, \
            type=%s, ifname=%s, pcscf=%s, rat=%d",
            reason == NULL ? "dumpDataResponse" : reason, response->status,
            response->suggestedRetryTime, response->cid, response->active,
            response->type, response->ifname, response->pcscf, response->rat);
}

void openSocketsForNetworkInterfaceConfig() {
    sock_fd_v4 = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd_v4 < 0) {
        LOGE("Couldn't create IP socket: errno=%d", errno);
    }
    sock_fd_v6 = socket(AF_INET6, SOCK_DGRAM, 0);
    if (sock_fd_v6 < 0) {
        LOGE("Couldn't create IPv6 socket: errno=%d", errno);
    }
}

void closeSocketsAfterNetworkInterfaceConfig() {
    close(sock_fd_v4);
    close(sock_fd_v6);
    sock_fd_v4 = -1;
    sock_fd_v6 = -1;
}

void setStateOfNwInterface(int socket, struct ifreq *ifr, int state, int clr) {
    int ret = 0;
    LOGD("[RILData_Util] Configure network state: %d.", state);
    ret = ioctl(socket, SIOCGIFFLAGS, ifr);
    if (ret < 0) {
        LOGE("setStateOfNwInterface failed 0 - %x, err: %s", ifr->ifr_flags, strerror(errno));
        goto error;
    }
    ifr->ifr_flags = (ifr->ifr_flags & (~clr)) | state;
    ret = ioctl(socket, SIOCSIFFLAGS, ifr);
    if (ret < 0) {
        LOGE("setStateOfNwInterface failed 1 - %x, err: %s", ifr->ifr_flags, strerror(errno));
        goto error;
    }
    return;

error:
    return;
}

void setIpv4AddrToNwInterface(int socket, struct ifreq *ifr, const char *addr) {
    int ret = 0;
    struct sockaddr_in *sin = (struct sockaddr_in *) &ifr->ifr_addr;
    sin->sin_family = AF_INET;
    sin->sin_port = 0;
    sin->sin_addr.s_addr = inet_addr(addr);
    ret = ioctl(socket, SIOCSIFADDR, ifr);
    if (ret < 0) {
        LOGE("setIpv4AddrToNwInterface failed - %s.", strerror(errno));
    }
}

void setIpv6AddrToNwInterface(int s, struct ifreq *ifr, const char *addr) {
    struct in6_ifreq ifreq6;
    int ret = 0;

    ret = ioctl(s, SIOCGIFINDEX, ifr);
    if (ret < 0) {
        goto error;
    }

    ret = inet_pton(AF_INET6, addr, &ifreq6.ifr6_addr);
    if (ret < 0) {
        goto error;
    }
    ifreq6.ifr6_prefixlen = 64;
    ifreq6.ifr6_ifindex = ifr->ifr_ifindex;
    ret = ioctl(s, SIOCSIFADDR, &ifreq6);
    if (ret < 0) {
        goto error;
    }
    return;

error:
    LOGE("setIpv6AddrToNwInterface failed: %d - %d:%s", ret, errno,
            strerror(errno));
    return;

}

int definePdnContext(const RIL_Default_Bearer_VA_Config_Struct *pImsParam,
        const char *requestedApn, int protocol, int availableCid, int authType,
        const char *username, const char* password, RILChannelCtx *pChannel) {
    int nRet = 1; //success
    int err = 0;
    ATResponse *p_response = NULL;
    char cmd[AT_COMMAND_MEDIUM_LENGTH] = {0};
    int emergency_ind = 0; //0:normal, 1:emergency
    int pcscf_discovery_flag = 0;
    int signalingFlag = 0;
    const char *pszIPStr;

    int len = 0;
    char tmp[PROPERTY_VALUE_MAX] = {0};
    char optrValue[PROPERTY_VALUE_MAX] = {0};
    int queryMtu = 0;
    char *temp = NULL;
    int pcscf_reselection = 0;

    // IMS @{
    if (pImsParam != NULL) {
        emergency_ind = pImsParam->emergency_ind;
        pcscf_discovery_flag = pImsParam->pcscf_discovery_flag;
        signalingFlag = pImsParam->signalingFlag;
    }
    // @}

    //+CGDCONT=[<cid>[,<PDP_type>[,<APN>[,<PDP_addr>[,<d_comp>[,<h_comp>[,<IPv4AddrAlloc>[,<emergency_indication>
    //[,<P-CSCF_discovery>[,<IM_CN_Signalling_Flag_Ind>]]]]]]]]]]
    if (protocol == IPV4V6) {
        pszIPStr = SETUP_DATA_PROTOCOL_IPV4V6;
    } else if (protocol == IPV6) {
        pszIPStr = SETUP_DATA_PROTOCOL_IPV6;
    } else {
        pszIPStr = SETUP_DATA_PROTOCOL_IP;
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "AT+CGDCONT=%d,\"%s\",\"%s\",,0,0,0,%d,%d,%d", availableCid,
            pszIPStr, requestedApn, emergency_ind, pcscf_discovery_flag, signalingFlag);
    err = at_send_command(cmd, &p_response, pChannel);
    if (p_response == NULL) {
        LOGE("[%s] p_response is NULL", __FUNCTION__);
        return 0; //failed
    }
    if (isATCmdRspErr(err, p_response)) {
        if (p_response != NULL && p_response->success == 0) {
            nLastDataCallFailCause = at_get_cme_error(p_response);
            LOGE("[RILData_Setup] AT+CGDCONT err code = %0x", nLastDataCallFailCause);
        }
        nRet = 0; //failed
    }
    AT_RSP_FREE(p_response);

    int isIaApn = isInitialAttachApn(requestedApn);

    // set protocol configuration in PDP
    // Use AT+CGPRCO=<cid>,<user_name>,<passwd>,<DNS1>,<DNS2>,<authType>,
    // <req_v6dns>,<req_v4dns>,<isIA>,<mtu_request>,<op-PCO_code>,<op-PCO_content>,<MSISDN>

    property_get("persist.vendor.operator.optr", optrValue, "0");
    // 20: Sprint, "Pam" or "pamsn": tethering APN name for Sprint
    if (strcmp(optrValue, "OP20") == 0 &&
        (strcasecmp(requestedApn, "pam") == 0 || strcasecmp(requestedApn, "pamsn") == 0)) {
        // For MD3, username/password are all "tethering" for +CGPRCO while creating tethering pdn.
        memset(cmd, 0, sizeof(cmd));
        if (AUTHTYPE_NOT_SET == authType) {
            snprintf(cmd, sizeof(cmd), "AT+CGPRCO=%d,\"tethering\",\"tethering\",\"\",\"\",0,1,1,%d",
                    availableCid, isIaApn);
        } else {
            snprintf(cmd, sizeof(cmd), "AT+CGPRCO=%d,\"tethering\",\"tethering\",\"\",\"\",%d,1,1,%d",
                    availableCid, authType, isIaApn);
        }
    } else {
        // [query mtu]  0:turn off ; 1: turn on
        property_get("vendor.ril.data.query.mtu", tmp, "1");
        queryMtu = atoi(tmp);
        memset(cmd, 0, sizeof(cmd));
        if (AUTHTYPE_NOT_SET == authType) {
            len = snprintf(cmd, sizeof(cmd), "AT+CGPRCO=%d,\"\",\"\",\"\",\"\",0,1,1,%d,%d",
                    availableCid, isIaApn, queryMtu);
        } else {
            if (isSvlteSupport()) {
                // For svlte, CGPRCO command MD3 only support 9 parameters.
                len = snprintf(cmd, sizeof(cmd), "AT+CGPRCO=%d,\"%s\",\"%s\",\"\",\"\",%d,1,1,%d",
                        availableCid, username, password, authType, isIaApn);
            } else {
                len = snprintf(cmd, sizeof(cmd), "AT+CGPRCO=%d,\"%s\",\"%s\",\"\",\"\",%d,1,1,%d,%d",
                        availableCid, username, password, authType, isIaApn, queryMtu);
            }
        }
        if (isOp12Support()) {
            // For VzW, set MSISDN flag for initial attach APN
            if((NULL != requestedApn) && (NULL != strcasestr(requestedApn, "ims"))) {
                pcscf_reselection = 1;
            }
            char tempCmd[AT_COMMAND_MEDIUM_LENGTH] = {0};
            memset(tempCmd, 0, sizeof(tempCmd));
            snprintf(tempCmd, sizeof(tempCmd), ",\"%s\",\"%s\",%d,%d",
                OP12_PCO_ID, OP12_PLMN, isIaApn, pcscf_reselection);

            len = len + strlen(tempCmd);
            temp = (char*) calloc(len + 1, sizeof(char));
            if (temp != NULL) {
                strncpy(temp, cmd, len);
                strncat(temp, tempCmd, strlen(tempCmd));
                memset(cmd, 0, sizeof(cmd));
                strncpy(cmd, temp, sizeof(cmd) - 1);
            }
        }
    }

    err = at_send_command(cmd, NULL, pChannel);
    LOGI("[RILData_Setup] definePdnContext finish: isIaApn = %d, nRet = %d.", isIaApn, nRet);
    return nRet;
}

int activatePdn(int availableCid, RILChannelCtx *pChannel) {
    int nRet = 0;  // 0:failed, 1: success
    char *cmd = NULL;
    int err = 0;
    ATResponse *p_response = NULL;
    char *line = NULL, *out = NULL;
    ATLine *p_cur = NULL;
    int activatedCid = INVALID_CID;

    LOGD("[RILData_Setup] activatePdn start: availableCid = %d", availableCid);

    /* Use AT+CGACT=1,cid to activate PDP context indicated via this cid */
    asprintf(&cmd, "AT+CGACT=1,%d", availableCid);
    // response urc format: +CGEV: ME PDN ACT <cid>
    err = at_send_command_multiline_notimeout(cmd, "+CGEV: ME PDN ACT ", &p_response,
            pChannel);
    free(cmd);
    if (p_response == NULL) {
        LOGE("[%s] p_response is NULL", __FUNCTION__);
        goto error;
    }
    if (isATCmdRspErr(err, p_response)) {
        if (p_response != NULL && p_response->success == 0) {
            nLastDataCallFailCause = at_get_cme_error(p_response);
            LOGE("[RILData_Setup] AT+CGACT: err code = %0x", nLastDataCallFailCause);
        }
        goto error;
    }

    for (p_cur = p_response->p_intermediates; p_cur != NULL;
            p_cur = p_cur->p_next) {
        line = p_cur->line;
        err = at_tok_start(&line);
        /* line => +CGEV: ME PDN ACT X */
        if (err < 0) {
            goto error;
        }

        LOGD("[RILData_Setup] activatePdn line = %s, len = %d", line, strlen(ME_PDN_URC));
        if (strStartsWith(line, ME_PDN_URC)) {
            char *firstParam = line + strlen(ME_PDN_URC);
            LOGI("[RILData_Setup] activatePdn firstParam = %s", firstParam);
            activatedCid = atoi(firstParam);
            if (activatedCid != availableCid) {
                goto error;
            }
            // PDP is activated and does not bind to a network interface, set to link down
            pPdnInfo[activatedCid].active = DATA_STATE_LINKDOWN;
            pPdnInfo[activatedCid].cid = activatedCid;
            nRet = 1;
        }
    }

error:
    AT_RSP_FREE(p_response);
    LOGE("[RILData_Setup] [%s] finish. ret: %d, activatedCid: %d", __FUNCTION__, nRet, activatedCid);
    return nRet;
}

int updateActiveStatus(RILChannelCtx *pChannel) {
    ATResponse *p_response = NULL;
    ATLine *p_cur = NULL;
    int maxCidSupported = 0;

    int err = at_send_command_multiline("AT+CGACT?", "+CGACT:", &p_response,
            pChannel);
    if (p_response == NULL) {
        LOGE("[%s] p_response is NULL", __FUNCTION__);
        return maxCidSupported;
    }
    if (isATCmdRspErr(err, p_response)) {
        LOGE("[%s][RILData_Query] AT+CGACT? failed - XXXXXX.", __FUNCTION__);
        AT_RSP_FREE(p_response);
        return 0;
    }

    for (p_cur = p_response->p_intermediates; p_cur != NULL;
            p_cur = p_cur->p_next) {
        maxCidSupported++;
    }

    LOGI("[%s][RILData_Query] PDN capability [%d, %d]", __FUNCTION__, maxCidSupported,
            nPdnInfoSize);

    for (p_cur = p_response->p_intermediates; p_cur != NULL;
            p_cur = p_cur->p_next) {
        char *line = p_cur->line;

        err = at_tok_start(&line);
        if (err < 0) {
            LOGE("[%s][RILData_Query] ERROR 0: errno = %d.", __FUNCTION__, err);
            break;
        }

        int responseCid = 0;
        err = at_tok_nextint(&line, &responseCid);
        if (err < 0) {
            LOGE("[%s][RILData_Query] ERROR 1: errno = %d.", __FUNCTION__, err);
            break;
        }

        err = at_tok_nextint(&line, &pPdnInfo[responseCid].active);
        if (err < 0) {
            LOGE("[%s][RILData_Query] ERROR 2: errno =  %d.", __FUNCTION__, err);
            break;
        }

        if (pPdnInfo[responseCid].active > 0) {
            // 0=inactive, 1=active/physical link down, 2=active/physical link up
            if (pPdnInfo[responseCid].interfaceId == INVALID_CID) {
                pPdnInfo[responseCid].active = DATA_STATE_LINKDOWN;
                LOGI("[%s][RILData_Query] CID%d is linkdown", __FUNCTION__, responseCid);
            } else {
                pPdnInfo[responseCid].active = DATA_STATE_ACTIVE;
                LOGI("[%s][RILData_Query] CID%d is active", __FUNCTION__, responseCid);
            }
        } else {
            LOGD("[%s][RILData_Query] CID%d is inactive", __FUNCTION__, responseCid);
        }
    }
    AT_RSP_FREE(p_response);

    // This may not be real max supported CID, should use getPdnCapability to know PDN capability
    return maxCidSupported;
}

int updatePdnAddressByCid(int cid, RILChannelCtx *pChannel) {
    int err = -1;
    char *line = NULL, *out = NULL;
    char *cmd = NULL;
    ATResponse *p_response = NULL;
    int responseCid = -1;

    /* Use AT+CGPADDR=cid to query the ip address assigned to this PDP context indicated via this cid */
    asprintf(&cmd, "AT+CGPADDR=%d", cid);
    err = at_send_command_singleline(cmd, "+CGPADDR:", &p_response, pChannel);
    free(cmd);
    if (p_response == NULL) {
        LOGE("[%s] p_response is NULL", __FUNCTION__);
        goto error;
    }

    /* Parse the response to get the ip address */
    if (isATCmdRspErr(err, p_response)) {
        goto error;
    }
    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    /* line => +CGPADDR: <cid>,<PDP_addr_1>,<PDP_addr_2> */

    if (err < 0) {
        LOGE("[RILData_Query] updatePdnAddressByCid ERROR 0: errno = %d.", err);
        goto error;
    }

    /* Get 1st parameter: CID */
    err = at_tok_nextint(&line, &responseCid);
    if (err < 0) {
        LOGE("[RILData_Query] updatePdnAddressByCid ERROR 1: errno = %d.", err);
        goto error;
    }

    /* Get 2nd parameter: IPAddr1 */
    err = at_tok_nextstr(&line, &out);
    if (err < 0) {
        LOGE("[RILData_Query] updatePdnAddressByCid ERROR 2: errno = %d.", err);
        goto error;
    }
    updatePdnAddressWithAddr(responseCid, out);

    /* Only for IPv4V6 + dual stack PDP context */
    /* Get 3rd paramter: IPaddr 2*/
    if (at_tok_hasmore(&line)) {
        err = at_tok_nextstr(&line, &out);
        if (err < 0) {
            LOGE("[RILData_Query] updatePdnAddressByCid ERROR 3: errno = %d.", err);
            goto error;
        }
        updatePdnAddressWithAddr(responseCid, out);
    }
    AT_RSP_FREE(p_response);
    return 0;

error:
    /* Free the p_response for +CGPADDR: */
    AT_RSP_FREE(p_response);
    return -1;
}

void updatePdnAddressWithAddr(int responseCid, char *addr) {
    if (getPdnAddressType(addr) == IPV4) {
        sprintf(pPdnInfo[responseCid].addressV4, "%s", addr);
    } else {
        convertIpv6Address(addr, pPdnInfo[responseCid].addressV6, 1);
    }
}

int updateDnsAndMtu(RILChannelCtx *pChannel) {
    char *line = NULL;
    char *out = NULL;
    char *cmd = NULL;
    ATLine *p_cur = NULL;
    ATResponse *p_response = NULL;

    // Use AT+CGPRCO? to query each cid's dns server address and MTU
    // +CGPRCO:<cid>,"<dnsV4-1>","<dnsV4-2>","<dnsV6-1>","<dnsV6-2>", <MTU>
    int err = at_send_command_multiline("AT+CGPRCO?", "+CGPRCO:", &p_response,
            pChannel);
    if (p_response == NULL) {
        LOGE("[%s] p_response is NULL", __FUNCTION__);
        goto error;
    }
    if (isATCmdRspErr(err, p_response)) {
        LOGE("[RILData_Query] updateDnsAndMtu ERROR 0.");
        goto error;
    }

    int i = 0;
    for (i = 0; i < nPdnInfoSize; i++) {
        // clear DNS information in CID table since we would query it again
        memset(pPdnInfo[i].dnsV4, 0, sizeof(pPdnInfo[i].dnsV4));
        memset(pPdnInfo[i].dnsV6, 0, sizeof(pPdnInfo[i].dnsV6));
    }

    for (p_cur = p_response->p_intermediates; p_cur != NULL;
            p_cur = p_cur->p_next) {
        line = p_cur->line;
        err = at_tok_start(&line);

        if (err < 0) {
            LOGE("[RILData_Query] updateDnsAndMtu ERROR 1: errno = %d.", err);
            goto error;
        }

        /* Get 1st parameter: CID */
        int responseCid = -1;
        err = at_tok_nextint(&line, &responseCid);
        if (err < 0) {
            LOGE("[RILData_Query] updateDnsAndMtu ERROR 2: errno = %d.", err);
            goto error;
        }

        int v4Count = 0;
        int v6Count = 0;
        for (i = 0; i < MAX_NUM_DNS_ADDRESS_NUMBER * 2; i++) {
            if (!at_tok_hasmore(&line)) {
                break;
            }

            err = at_tok_nextstr(&line, &out);
            if (err < 0) {
                goto error;
            }

            if (out != NULL && strlen(out) > 0
                    && strcmp(NULL_IPV4_ADDRESS, out) != 0) {
                if (getPdnAddressType(out) == IPV6) {
                    convertIpv6Address(out,
                            pPdnInfo[responseCid].dnsV6[v6Count], 0);
                    ++v6Count;
                } else {
                    strncpy(pPdnInfo[responseCid].dnsV4[v4Count], out, MAX_IPV4_ADDRESS_LENGTH - 1);
                    pPdnInfo[responseCid].dnsV4[v4Count][MAX_IPV4_ADDRESS_LENGTH - 1] = '\0';
                    ++v4Count;
                }
            }
        }

        // C2K network may send MTU configure from operator network, like Verizon.
        err = at_tok_nextint(&line, &pPdnInfo[responseCid].mtu);
        LOGD("[RILData_Query] updateDnsAndMtu mtu = %d.", pPdnInfo[responseCid].mtu);
        if (err < 0) {
            LOGE("[RILData_Query] Error happens when updateing MTU for cid %d.", responseCid);
            goto error;
        }
    }

    AT_RSP_FREE(p_response);
    return 0;

error:
    LOGE("[RILData_Query] updateDnsAndMtu failed - XXXXXX.");
    AT_RSP_FREE(p_response);
    return -1;
}

void parseAndSendPcoStatus(char* line, int cid) {
    // +CGPRCO:<cid>, <v4_dns1>, <v4_dns2>, <v6_dns1>, <v6_dns2>
    // , <initial_attach>, <mtu_size>, <op-PCO_code:op-PCO_content:op-PCO_action>
    char* out = NULL;
    int err = 0;
    RIL_PCO_Data pcoData;
    int isAddrV4V6Bitmap = 0; //Bitmap: 00:none, 01:IPv4, 10:IPv6, 11: IPv4v6
    char pcoIdStr[PCO_ID_STR_LEN];
    char plmn[PLMN_LEN];
    int pcoContent = -1;

    err = at_tok_start(&line);
    if (err < 0) {
        LOGE("[%s] parse line error: %d", __FUNCTION__, err);
        return;
    }

    // <cid>
    int responseCid = -1;
    err = at_tok_nextint(&line, &responseCid);
    if (err < 0) {
        LOGE("[%s] parse cid error: %d", __FUNCTION__, err);
        return;
    }

    if (cid != INVALID_CID && cid != responseCid) {
        LOGE("[%s] not reported cid: %d, %d", __FUNCTION__, cid, responseCid);
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
            LOGE("[%s] parse dns error: %d", __FUNCTION__, err);
            return;
        }
    }

    // <inital attach>
    if (at_tok_hasmore(&line)) {
        int isIa = 0;
        err = at_tok_nextint(&line, &isIa);
        if (err < 0) {
            LOGE("[%s] parse inital_attach error: %d", __FUNCTION__, err);
            return;
        }
    }

    // <mtu_size>
    if (at_tok_hasmore(&line)) {
        int mtu = 0;
        err = at_tok_nextint(&line, &mtu);
        if (err < 0) {
            LOGE("[%s] parse mtu_size error: %d", __FUNCTION__, err);
            return;
        }
    }

    // <op-PCO_code:op-PCO_content:op-PCO_action>
    if (at_tok_hasmore(&line)) {
        err = at_tok_nextstr(&line, &out);
        if (err < 0) {
            LOGE("[%s] parse OP PCO error: %d", __FUNCTION__, err);
            return;
        }
        char *token = strsep(&out, ":");
        if (token == NULL) {
            LOGE("[%s] parse OP PCO code error", __FUNCTION__);
            return;
        }
        strncpy(pcoIdStr, token, PCO_ID_STR_LEN - 1);
        pcoIdStr[PCO_ID_STR_LEN - 1] = '\0';

        token = strsep(&out, ":");
        if (token == NULL) {
            LOGE("[%s] parse OP PCO content error", __FUNCTION__);
            return;
        }
        strncpy(plmn, token, PLMN_LEN - 1);
        plmn[PLMN_LEN - 1] = '\0';

        token = strsep(&out, ":");
        if (token == NULL || strlen(token) == 0) {
            LOGE("[%s] parse OP PCO ation eror", __FUNCTION__);
            return;
        }

        pcoContent = atoi(token);

        if (strcmp(plmn, OP12_PLMN) == 0 && strcmp(pcoIdStr, OP12_PCO_ID) == 0) {
            if (pcoContent >= 0 && pcoContent <= 255) {
                // cid
                pcoData.cid = pPdnInfo[responseCid].interfaceId;

                // Bearer proto
                if (strlen(pPdnInfo[responseCid].addressV4) > 0 && strcmp(pPdnInfo[responseCid].addressV4, NULL_IPV4_ADDRESS) != 0) {
                    isAddrV4V6Bitmap |= (IPV4 + 1);
                }
                if (strlen(pPdnInfo[responseCid].addressV6) > 0 && strcmp(pPdnInfo[responseCid].addressV6, NULL_IPV4_ADDRESS) != 0) {
                    isAddrV4V6Bitmap |= (IPV6 + 1);
                }
                if ((IPV4V6 + 1) == isAddrV4V6Bitmap) {
                    pcoData.bearer_proto = "IPV4V6";
                } else if (((IPV4 + 1) == isAddrV4V6Bitmap) || ((IPV6 + 1) == isAddrV4V6Bitmap)) {
                    if ((IPV4 + 1) == isAddrV4V6Bitmap) {
                        pcoData.bearer_proto = "IP";
                    } else {
                        pcoData.bearer_proto = "IPV6";
                    }
                } else if (0 == isAddrV4V6Bitmap) {
                    LOGE("[%s] Wrong IP type (bearer protocol).\n", __FUNCTION__);
                    return;
                }

                pcoData.pco_id = (int) strtol(pcoIdStr, NULL, 16);

                // PCO contents length (1 byte for FF00 VZW.)
                pcoData.contents_length = 1;

                // PCO value (hex string) (Current only support 1 byte for VZW PCO FF00H)
                pcoData.contents = (char*) malloc(pcoData.contents_length * sizeof(char));
                assert(pcoData.contents != NULL);
                pcoData.contents[0] = pcoContent;
            } else {
                LOGE("[%s] PCO value is out of range.\n");
                return;
            }
        } else {
            // TODO: Extend here if need to support another carrier.
            LOGE("[%s] Do not handle PCO for PLMN: %s or PCO ID: %s\n", __FUNCTION__, plmn, pcoIdStr);
            return;
        }

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

        for (int j = 0; j < pcoData.contents_length; j++) {
            LOGD("[%s] contents[%d] = %d", __FUNCTION__, j, pcoData.contents[j]);
        }

        if (pcoData.cid != -1) {
            RIL_onUnsolicitedResponse(RIL_UNSOL_PCO_DATA, &pcoData,
                    sizeof(pcoData));
        }
    }
}

void getAndSendPcoStatus(int cid) {
    if (!isOp12Support()) {
        return;
    }

    ATLine *p_cur = NULL;
    ATResponse *p_response = NULL;
    RIL_SOCKET_ID rid = getCdmaSocketSlotId() - 1;
    RILChannelCtx *pChannel = getChannelCtxbyProxy(rid);

    // Use AT+CGPRCO? to query each cid's dns server address
    // +CGPRCO:<cid>, <v4_dns1>, <v4_dns2>, <v6_dns1>, <v6_dns2>
    // , <initial_attach>, <mtu_size>, <op-PCO_code:op-PCO_content:op-PCO_action>
    int err = at_send_command_multiline("AT+CGPRCO?", "+CGPRCO:", &p_response, pChannel);
    if (p_response == NULL) {
        LOGE("[%s] p_response is NULL", __FUNCTION__);
        return;
    }
    if (isATCmdRspErr(err, p_response)) {
        LOGE("[%s] can not get the PCO status", __FUNCTION__);
    } else {
        for (p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next) {
            parseAndSendPcoStatus(p_cur->line, cid);
        }
    }

    AT_RSP_FREE(p_response);
    return;
}

int bindPdnToInterface(int interfaceId, int cid, RILChannelCtx *pChannel) {
    int nRet = 1;  // 0:failed, 1: success
    char *cmd = NULL;
    int err = 0;

    LOGI("[RILData_Setup] bindPdnToInterface start E: interfaceId = %d, cid = %d.", interfaceId, cid);
    if (cid != INVALID_CID && (pPdnInfo[cid].interfaceId == INVALID_CID
            || pPdnInfo[cid].interfaceId != interfaceId)) {
        /* AT+CGDATA=<L2P>,<cid>,<channel ID> */
        asprintf(&cmd, "AT+CGDATA=\"%s\",%d,%d", pcL2pValue, cid,
                interfaceId + 1);  // The channel id is from 1~n
        err = at_send_command(cmd, NULL, pChannel);
        free(cmd);
        if (err < 0) {
            LOGE("[RILData_Setup] CID%d fail to bind interface%d", cid,
                    interfaceId);
            nRet = 0;
            goto error;
        } else {
            pPdnInfo[cid].interfaceId = interfaceId;
            pPdnInfo[cid].active = DATA_STATE_ACTIVE;
        }
    } else {
        LOGD("[RILData_Setup] already bind to interface, not to bind again.");
    }

    return nRet;

error:
    return nRet;
}

// Bind CCMNI interface with specified modem.
void bindNetworkInterfaceWithModem(int interfaceId, int modemId) {
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(struct ifreq));
    sprintf(ifr.ifr_name, "%s", getNetworkInterfaceName(interfaceId));
    LOGI("[%s] interface %d to modem %d, ifrName = %s", __FUNCTION__, interfaceId,
            modemId, ifr.ifr_name);

    ifc_ccmni_md_cfg(ifr.ifr_name, modemId);
}

// Update network interface because PPP Re-Negotioation / IRAT, the IP and DNS may changed,
// bind the new PDN address and reset data connections.
void updateNetworkInterface(int interfaceId, int mask) {
    struct ifreq ifr;
    int sim_id = getValidSimId();
    mal_datamngr_trigger_rs_rsp_ptr_t rsp_ptr =
            calloc(1, sizeof(mal_datamngr_trigger_rs_rsp_t));
    assert(rsp_ptr != NULL);
    memset(&ifr, 0, sizeof(struct ifreq));
    sprintf(ifr.ifr_name, "%s", getNetworkInterfaceName(interfaceId));
    LOGI("[RILData_Util] updateNetworkInterface: %s", ifr.ifr_name);

    // Reset connection only if IPv4/v6 address changed.
    if ((mask & 0x04) == 0) {
        ifc_reset_connections(ifr.ifr_name, mask);
    }

    openSocketsForNetworkInterfaceConfig();
    int i = 0;
    for (i = 0; i < nPdnInfoSize; i++) {
        if (pPdnInfo[i].interfaceId == interfaceId) {
            if (strlen(pPdnInfo[i].addressV4) > 0 && (mask & 0x01)) {
                setStateOfNwInterface(sock_fd_v4, &ifr, IFF_UP, 0);
                setIpv4AddrToNwInterface(sock_fd_v4, &ifr, pPdnInfo[i].addressV4);
            }
            if (strlen(pPdnInfo[i].addressV6) > 0 && ((mask & 0x02) || (mask & 0x04))) {
                setStateOfNwInterface(sock_fd_v6, &ifr, IFF_UP, 0);
                if (mal_datamngr_trigger_rs_tlv != NULL) {
                    mal_datamngr_trigger_rs_tlv(mal_once(1, mal_cfg_type_sim_id, sim_id), ifr.ifr_name,
                                                rsp_ptr, 0, NULL, NULL, NULL);
                    LOGD("[%s] Call mal_datamngr_trigger_rs_tlv success", __FUNCTION__);
                } else {
                    LOGE("[%s] mal_datamngr_trigger_rs_tlv is null", __FUNCTION__);
                }
                int errNo = triggerIpv6Rs(ifr.ifr_name);
                if (errNo < 0) {
                    LOGE("updateNetworkInterface: trigger RS %s fail %d", ifr.ifr_name, errNo);
                }
                setIpv6AddrToNwInterface(sock_fd_v6, &ifr, pPdnInfo[i].addressV6);
            }
        }
    }
    FREEIF(rsp_ptr);
    closeSocketsAfterNetworkInterfaceConfig();
}

void configureNetworkInterface(int interfaceId, int isUp) {
    if (interfaceId <= INVALID_CID) {
        LOGE("configureNetworkInterface return directly because of invaild isUp=%d", isUp);
        return;
    }

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(struct ifreq));
    sprintf(ifr.ifr_name, "%s", getNetworkInterfaceName(interfaceId));
    openSocketsForNetworkInterfaceConfig();

    if (isUp) {
        /** Set the network interface disable first before enable
          * to prevent from unknown exception causing not close related
          * dev file description
          * Why CCMNI doesn't control it is the API goto kernel first,
          * kernel will not send the command to CCMNI as always UP.
          */
        LOGD("[RILData_Util] configureNetworkInterface set %s to down.", ifr.ifr_name);
        setNetworkInterfaceDown(ifr.ifr_name);
    }

    int i = 0;
    for (i = 0; i < nPdnInfoSize; i++) {
        if (pPdnInfo[i].interfaceId == interfaceId) {
            if (isUp) {
                LOGI("[RILData_Util] configureNetworkInterface %s", ifr.ifr_name);
                if (strlen(pPdnInfo[i].addressV4) > 0) {
                    setStateOfNwInterface(sock_fd_v4, &ifr, IFF_UP, 0);
                    setIpv4AddrToNwInterface(sock_fd_v4, &ifr, pPdnInfo[i].addressV4);
                }

                if (strlen(pPdnInfo[i].addressV6) > 0) {
                    setStateOfNwInterface(sock_fd_v6, &ifr, IFF_UP, 0);
                    setIpv6AddrToNwInterface(sock_fd_v6, &ifr, pPdnInfo[i].addressV6);
                }
            } else {
                LOGD("[RILData_Util] configureNetworkInterface reset %s to down.", ifr.ifr_name);
                setNetworkInterfaceDown(ifr.ifr_name);
            }
        }
    }

    closeSocketsAfterNetworkInterfaceConfig();
}

int getAuthType(const char* authTypeStr) {
    int authType = atoi(authTypeStr);
    // Application 0->none, 1->PAP, 2->CHAP, 3->PAP/CHAP;
    // Modem 0->PAP, 1->CHAP, 2->NONE, 3->PAP/CHAP;
    switch (authType) {
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

int getProtocolType(const char* protocol) {
    int type = IPV4;
    if (protocol == NULL) {
        return type;
    }

    if (!strcmp(protocol, SETUP_DATA_PROTOCOL_IP)) {
        type = IPV4;
    } else if (!strcmp(protocol, SETUP_DATA_PROTOCOL_IPV6)) {
        type = IPV6;
    } else if (!strcmp(protocol, SETUP_DATA_PROTOCOL_IPV4V6)) {
        type = IPV4V6;
    }

    LOGD("The protocol type is %d", type);
    return type;
}

int getPdnAddressType(char* addr) {
    int type = IPV4;
    int length = strlen(addr);
    if (length >= MAX_IPV6_ADDRESS_LENGTH) {
        type = IPV4V6;
    } else if (length >= MAX_IPV4_ADDRESS_LENGTH) {
        type = IPV6;
    }
    return type;
}

int convertIpv6Address(char* pdp_addr, char* ipv6_addr, int isLinkLocal) {
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
        strncat(ipv6_addr + strlen(ipv6_addr), tmpOutput,
                MAX_IPV6_ADDRESS_LENGTH - strlen(ipv6_addr) - 1);
        p = strtok(NULL, ".");
        if (p == NULL) {
            LOGE("The format of IP address is illegal");
            return -1;
        }
        memset(tmpOutput, 0, MAX_IPV6_ADDRESS_LENGTH);
        value = atoi(p);
        snprintf(tmpOutput, MAX_IPV6_ADDRESS_LENGTH, "%02X:", value);
        strncat(ipv6_addr + strlen(ipv6_addr), tmpOutput,
                MAX_IPV6_ADDRESS_LENGTH - strlen(ipv6_addr) - 1);
        p = strtok(NULL, ".");
    }
    len = strlen(ipv6_addr);
    ipv6_addr[len - 1] = '\0';
    return 0;
}

int isInitialAttachApn(const char *requestedApn) {
    char iccid[PROPERTY_VALUE_MAX] = { 0 };
    char iaProperty[PROPERTY_VALUE_MAX] = { 0 };
    char apnParameter[PROPERTY_VALUE_MAX] = { 0 };
    int slotId = getCdmaSocketSlotId() - 1;

    property_get(RP_DATA_PROPERTY_IA[slotId], iaProperty, "");
    LOGD("[RILData_C2K]isInitialAttachApn iaProperty=%s", iaProperty);
    property_get(PROPERTY_ICCID_SIM[slotId], iccid, "");

    if (strlen(iaProperty) > 0) {
        snprintf(apnParameter, PROPERTY_VALUE_MAX, "%s,%s", iccid, requestedApn);
        if (strcmp(apnParameter, iaProperty) == 0) {
            LOGI("[RILData_C2K] %s is ia.", requestedApn);
            return 1;
        }
    }
    return 0;
}

int queryMatchedPdnWithSameApn(const char* apn) {
    int i = 0;
    for (i = 0; i < nPdnInfoSize; i++) {
        LOGD("[RILData_Util]Query APN i=%d, apn = %s, pPdnInfo[i].interfaceId = %d, pPdnInfo[i].active = %d.",
                i, pPdnInfo[i].apn, pPdnInfo[i].interfaceId,
                pPdnInfo[i].active);
        if (pPdnInfo[i].interfaceId != INVALID_CID
                && pPdnInfo[i].active == DATA_STATE_ACTIVE
                && apnMatcher(pPdnInfo[i].apn, apn) == 0) {
            LOGI("[RILData_Util]Found reuseable CID%d for APN %s.", pPdnInfo[i].cid, apn);
            return pPdnInfo[i].cid;
        }
    }
    LOGI("[RILData_Util]Not found any cid for APN %s.", apn);
    return INVALID_CID;
}

int apnMatcher(const char* existApn, const char* apn) {
    // The APN is composed of two parts as follows: The APN Network Identifier & The APN Operator Identifier
    // The APN Operator Identifier is in the format of "mnc<MNC>.mcc<MCC>.gprs"
    // The valid APN format: <apn>[.mnc<MNC>.mcc<MCC>[.gprs]]
    int apnLength = strlen(apn);
    char *tmpExistApn = NULL;
    char *tmpApn = NULL;
    int j = 0;
    int result = -1;

    LOGI("[RILData_Util]apnMatcher: existApn = %s, apn = %s, apnLength = %d, exApnLen = %d.",
            existApn, apn, apnLength, strlen(existApn));
    if ((int) strlen(existApn) < apnLength) {
        return result;
    }

    tmpExistApn = calloc(1, (apnLength + 1) * sizeof(char));
    assert(tmpExistApn != NULL);
    tmpApn = calloc(1, (apnLength + 1) * sizeof(char));
    assert(tmpApn != NULL);

    // change to lower case to compare
    for (j = 0; j < apnLength; j++) {
        tmpExistApn[j] = tolower(existApn[j]);
        tmpApn[j] = tolower(apn[j]);
    }

    result = strcmp(tmpExistApn, tmpApn);
    if (result == 0) {
        if (!((strlen(apn) == strlen(existApn))
                || strStartsWith((existApn + apnLength), ".mnc")
                || strStartsWith((existApn + apnLength), ".MNC")
                || strStartsWith((existApn + apnLength), ".mcc")
                || strStartsWith((existApn + apnLength), ".MCC"))) {
            LOGD("[RILData_Util]apnMatcher: apns are equal but format unexpected");
            result = -1;
        }
    }

    free(tmpExistApn);
    free(tmpApn);

    return result;
}

// The AT channel for data requests is RIL_CHANNEL_2(ttySDIO6), detail see ril_commands.h.
int rilDataMain(int request, void *data, size_t datalen, RIL_Token t) {
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
    case RIL_LOCAL_C2K_REQUEST_CONFIRM_INTER_3GPP_IRAT_CHANGE:
        confirmIratChange(data, datalen, t);
        break;
    case RIL_REQUEST_SET_DATA_PROFILE:
        syncApnTable(data, datalen, t);
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
    // Sync IA APN to MD3 for IRAT case
    case RIL_REQUEST_SET_INITIAL_ATTACH_APN:
        requestSetInitialAttachApn(data, datalen, t);
        break;
    // MPS feature
    case RIL_LOCAL_C2K_REQUEST_DEACTIVATE_ALL_DATA_CALL:
        requestDeactiveAllDataCall(data, datalen, t);
        break;
    case RIL_LOCAL_REQUEST_GET_IMS_DATA_CALL_INFO:
        requestGetImsDataCallInfo(data, datalen, t);
        break;
    case RIL_LOCAL_REQUEST_REUSE_IMS_DATA_CALL:
        requestReuseImsDataCall(data, datalen, t);
        break;
    default:
        return 0; /* no matched request */
    }

    return 1; /* request found and handled */
}

int rilDataUnsolicited(const char *s, const char *sms_pdu) {
    UNUSED(sms_pdu);
    if (strStartsWith(s, "+CGEV: NW PDN DEACT")
            || strStartsWith(s, "+CGEV: ME PDN DEACT")) {
        // Make sure to allocate new buffer for URC string.
        char* urc = NULL;
        asprintf(&urc, "%s", s);
        LOGD("[RILData_C2K_URC] urc = %s.", urc);
        RIL_requestProxyTimedCallback(onPdnDeact, (void *) urc,
                &TIMEVAL_0, DATA_CHANNEL);
        return 1;
    } else if (strStartsWith(s, "+CGEV: ME")) {
        if (strStartsWith(s, "+CGEV: ME PDN ACT")) {
            LOGD("[RILData_C2K_URC] PDN activated.");
            char* urc = NULL;
            asprintf(&urc, "%s", s);
            RIL_requestProxyTimedCallback(onMePdnActive, (void *) urc,
                    &TIMEVAL_0, DATA_CHANNEL);
        } else {
            //+CGEV: ME related cases should be handled in setup data call request handler
            LOGD("[RILData_C2K_URC] ignore +CGEV: ME cases (%s)", s);
        }
        return 1;
    } else if (strStartsWith(s, "+CGEV: NW")) {
        if (strStartsWith(s, "+CGEV: NW MODIFY")) {
            LOGD("[RILData_C2K_URC] NW PDN MODIFY...");
            char* urc = NULL;
            asprintf(&urc, "%s", s);
            RIL_requestProxyTimedCallback(onPdnModified, (void *) urc, &TIMEVAL_0, DATA_CHANNEL);
        } else {
            LOGD("[RILData_C2K_URC] Ignore +CGEV: NW cases (%s)", s);
        }
        return 1;
    } else if (strStartsWith(s, "+CGEV:")) {
        LOGD("[RILData_C2K_URC] Unhandled CGEV: urc = %s.", s);
        /* Really, we can ignore NW CLASS and ME CLASS events here,
         * but right now we don't since extranous
         * RIL_UNSOL_DATA_CALL_LIST_CHANGED calls are tolerated
         */
        if (s_md3_off) {
            RIL_onUnsolicitedResponse(RIL_UNSOL_DATA_CALL_LIST_CHANGED, NULL, 0);
        } else {
            RIL_requestProxyTimedCallback(onDataCallListChanged, NULL, &TIMEVAL_0, DATA_CHANNEL);
        }

        return 1;
    } else if (strStartsWith(s, "+EI3GPPIRAT:")) {
        char* urc = NULL;
        asprintf(&urc, "%s", s);
        LOGD("[RILData_C2K_URC] urc = %s.", urc);
        RIL_requestProxyTimedCallback(onIratEvent, (void *) urc, &TIMEVAL_0, DATA_CHANNEL);
        return 1;
    } else if (strStartsWith(s, "+EPDNCTRANS:")) {
        char* urc = NULL;
        asprintf(&urc, "%s", s);
        LOGD("[RILData_C2K_URC] urc = %s.", urc);
        RIL_requestProxyTimedCallback(onLwcgIratEvent, (void *) urc, &TIMEVAL_0, DATA_CHANNEL);
        return 1;
    } else if (strStartsWith(s, "+EGCONTRDP:")) {
        LOGD("[RILData_C2K_URC] EGCONTRDP nIratAction = %d.", nIratAction);
        char* urc = NULL;
        asprintf(&urc, "%s", s);
        RIL_requestProxyTimedCallback(onPdnSyncFromSourceRat, (void *) urc,
                &TIMEVAL_0, DATA_CHANNEL);
        return 1;
    // LCE URC start @{
    } else if (strStartsWith(s, "+ELCE:")) {
        char* urc = NULL;
        asprintf(&urc, "%s", s);
        RIL_requestProxyTimedCallback(onLceDataResult, (void *) urc,
                &TIMEVAL_0, DATA_CHANNEL);
        return 1;
    } else if (strStartsWith(s, "+EXLCE:")) {
        char* urc = NULL;
        asprintf(&urc, "%s", s);
        RIL_requestProxyTimedCallback(onLinkCapacityEstimate, (void *) urc,
                &TIMEVAL_0, DATA_CHANNEL);
        return 1;
    // LCE URC end @}
    }

    return 0;
}

inline int isATCmdRspErr(int err, const ATResponse *p_response) {
    return (err < 0 || 0 == p_response->success) ? 1 : 0;
}

// Irat code start
void onIratSourceStarted(int sourceRat, int targetRat, int type) {
    LOGI("[RILData_C2K_IRAT] onIratSourceStarted: sourceRat = %d, targetRat = %d",
            sourceRat, targetRat);
    suspendAllDataTransmit();
    onIratStateChanged(sourceRat, targetRat, IRAT_ACTION_SOURCE_STARTED, type);
}

void onIratSourceFinished(int sourceRat, int targetRat, int type) {
    LOGI("[RILData_C2K_IRAT] onIratSourceFinished: sourceRat = %d, targetRat = %d",
            sourceRat, targetRat);

    switch (type) {
        case IRAT_TYPE_FAILED: {
            // IRAT fail: 1. resume all interfaces.
            // 2. deactviate previous deactivate failed PDNs.
            // 3. report IRAT status.
            resumeAllDataTransmit();
            int i = 0;
            for (i = 0; i < nPdnInfoSize; i++) {
                if (pPdnInfo[i].cid != INVALID_CID
                        && pPdnInfo[i].active == DATA_STATE_NEED_DEACT) {
                    deactivatePdnByCid(pPdnInfo[i].cid);
                }
            }
            nIratAction = IRAT_ACTION_UNKNOWN;
            break;
        }

        case IRAT_TYPE_EHRPD_LTE: {
            clearAllPdnInfo();
            break;
        }

        case IRAT_TYPE_HRPD_LTE: {
            resumeAllDataTransmit();
            // Down all CCMNI interfaces and clean PDN info.
            cleanupPdnsForFallback();
            break;
        }

        case IRAT_TYPE_LWCG_EHRPD_LTE: {
            clearAllPdnInfo();
            break;
        }

        default: {
            LOGE("[RILData_C2K_IRAT] Never should run into this case: type = %d.", type);
            break;
        }
    }
    onIratStateChanged(sourceRat, targetRat, IRAT_ACTION_SOURCE_FINISHED, type);
}

void onIratTargetStarted(int sourceRat, int targetRat, int type) {
    LOGI("[RILData_C2K_IRAT] onIratTargetStarted: sourceRat = %d, targetRat = %d",
            sourceRat, targetRat);
    nReactPdnCount = 0;
    nDeactPdnCount = 0;
    nReactSuccCount = 0;

    if (pSyncPdnInfo == NULL) {
        pSyncPdnInfo = calloc(1, nPdnInfoSize * sizeof(SyncPdnInfo));
        assert(pSyncPdnInfo != NULL);
        // Initialize sync PDN info list.
        clearAllSyncPdnInfo();
        LOGI("[RILData_C2K_IRAT] onIratTargetStarted init sync PDN info, size = %d.",
                nPdnInfoSize);
    }
    onIratStateChanged(sourceRat, targetRat, IRAT_ACTION_TARGET_STARTED, type);
}

void onIratTargetFinished(int sourceRat, int targetRat, int type) {
    LOGI("[RILData_C2K_IRAT] onIratTargetFinished: sourceRat = %d, targetRat = %d, "
            "nReactPdnCount = %d, nReactSuccCount = %d", sourceRat,
            targetRat, nReactPdnCount, nReactSuccCount);

    switch (type) {
        case IRAT_TYPE_FAILED: {
            clearAllPdnInfo();
            break;
        }

        case IRAT_TYPE_LTE_EHRPD: {
            // IRAT succeed.
            if (nDeactPdnCount != 0 || nReactPdnCount != nReactSuccCount) {
                handleDeactedOrFailedPdns(IRAT_NEED_RETRY);
                cleanupFailedPdns();
            }
            onDataCallListChanged();
            break;
        }

        case IRAT_TYPE_LTE_HRPD: {
            if (nDeactPdnCount != 0) {
                handleDeactedOrFailedPdns(IRAT_NO_RETRY);
            }
            cleanupFailedPdns();
            RIL_onUnsolicitedResponse(RIL_UNSOL_DATA_CALL_LIST_CHANGED, NULL, 0);
            LOGI("[RILData_C2K_IRAT] fallback case finished.");
            // Sync IMS data call list to MAL
            if (isImsSupport()) {
                LOGI("[RILData_Query] Sync IMS data call list to MAL");
                int sim_id = getValidSimId();
                mal_datamngr_data_call_info_rsp_ptr_t rsp_ptr =
                        calloc(1, sizeof(mal_datamngr_data_call_info_rsp_t));
                assert(rsp_ptr != NULL);
                if (sim_id != SIM_ID_INVALID) {
                    if (mal_datamngr_notify_data_call_list_tlv != NULL) {
                        int ret = mal_datamngr_notify_data_call_list_tlv(
                                mal_once(1, mal_cfg_type_sim_id, sim_id), 0,
                                NULL, (mal_datamngr_data_call_info_rsp_ptr_t *) rsp_ptr,
                                NULL, NULL, NULL, NULL);
                        LOGD("[%s] Call mal_datamngr_notify_data_call_list_tlv success", __FUNCTION__);
                    } else {
                        LOGE("[%s] mal_datamngr_notify_data_call_list_tlv is null", __FUNCTION__);
                    }
                }
                FREEIF(rsp_ptr);
            }
            break;
        }

        case IRAT_TYPE_LWCG_LTE_EHRPD: {
            handleLwcgFailedPdns();
            cleanupFailedPdns();
            onDataCallListChanged();
            break;
        }

        default: {
            LOGE("[RILData_C2K_IRAT] Never should run into this case: type = %d.", type);
            break;
        }
    }
    clearAllSyncPdnInfo();
    onIratStateChanged(sourceRat, targetRat, IRAT_ACTION_TARGET_FINISHED, type);
    nIratAction = IRAT_ACTION_UNKNOWN;
}

void onIratStateChanged(int sourceRat, int targetRat, int action, int type) {
    RIL_Pdn_IratInfo *pPdnIratInfo = (RIL_Pdn_IratInfo *) alloca(sizeof(RIL_Pdn_IratInfo));
    memset(pPdnIratInfo, 0, sizeof(RIL_Pdn_IratInfo));
    pPdnIratInfo->sourceRat = sourceRat;
    pPdnIratInfo->targetRat = targetRat;
    pPdnIratInfo->action = action;
    pPdnIratInfo->type = type;

    LOGI("[RILData_C2K_IRAT] onIratStateChanged: sourceRat = %d, targetRat = %d, "
                " nReactSuccCount = %d", sourceRat, targetRat, nReactSuccCount);
    RIL_onUnsolicitedResponse(RIL_LOCAL_C2K_UNSOL_INTER_3GPP_IRAT_STATE_CHANGE, pPdnIratInfo,
            sizeof(RIL_Pdn_IratInfo));
}

void onIratPdnReactSucc(int cid) {
    LOGI("[RILData_C2K_IRAT] onIratPdnReactSucc: cid = %d, pdnStatus = %d.", cid,
            pSyncPdnInfo[cid].pdnStatus);
    pSyncPdnInfo[cid].pdnStatus = IRAT_PDN_STATUS_REACTED;
    pPdnInfo[cid].active = DATA_STATE_ACTIVE;

    RIL_SOCKET_ID rid = getCdmaSocketSlotId() - 1;
    RILChannelCtx* rilchnlctx = getChannelCtxbyProxy(rid);
    if (updatePdnAddressByCid(cid, rilchnlctx) < 0) {
        goto error;
    }

    if (0 == rebindPdnToInterface(pSyncPdnInfo[cid].interfaceId, cid, rilchnlctx)) {
        goto error;
    }

    if (updateDnsAndMtu(rilchnlctx) < 0) {
        goto error;
    }
    nReactSuccCount++;
    reconfigureNetworkInterface(pSyncPdnInfo[cid].interfaceId);

    // OP12 IMS Data
    if (isOp12Support()) {
        responseUnsolImsDataCallListToMal(rilchnlctx, pSyncPdnInfo[cid].apn,
                pSyncPdnInfo[cid].interfaceId, rid);
    }

    return;

error:
    LOGE("[RILData_C2K_IRAT] onIratPdnReactSucc error happens, cid = %d.", cid);
    // Resume data transfer for the interface in case of MD3 is powered off.
    // Ex: IPO power on, MD3 powered off and the AT command will sent fail
    // Here we need to resume the data transmit
    setNetworkTransmitState(pSyncPdnInfo[cid].interfaceId, RESUME_DATA_TRANSFER);
    return;
}

void confirmIratChange(void *data, size_t datalen, RIL_Token t) {
    UNUSED(datalen);
    int err = -1;
    int apDecision = 0;
    char *cmd = NULL;
    ATResponse *p_response = NULL;
    apDecision = ((int *) data)[0];
    LOGD("[RILData_C2K_IRAT] confirmIratChange: apDecision=%d.", apDecision);

    // Confirm IRAT change.
    asprintf(&cmd, "AT+EI3GPPIRAT=%d", apDecision);
    err = at_send_command(cmd, &p_response, getRILChannelCtxFromToken(t));
    free(cmd);
    if (p_response == NULL) {
        LOGE("[%s] p_response is NULL", __FUNCTION__);
        goto error;
    }
    if (isATCmdRspErr(err, p_response)) {
        LOGE("confirmIratChange failed !");
        goto error;
    }

    LOGD("[RILData_C2K_IRAT] confirmIratChange succeed -");
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);
    return;

error:
    RIL_onRequestComplete(t, RIL_E_MODEM_ERR, NULL, 0);
    at_response_free(p_response);
}

void onIratEvent(void* urc) {
    int err = 0;
    int action = -1;
    int sourceRat = 0;
    int targetRat = 0;
    char* line = urc;
    // +EI3GPPIRAT: <action>,<source RAT>,<target RAT>
    // 1: source rat start  2: target rat start
    // 3: source rat finish 4: target rat finish
    // RAT: 0: not specified 1:1XRTT 2:HRPD 3:EHRPD 4:LTE
    LOGD("[RILData_C2K_IRAT] onIratEvent: action = %d, urc = %s.", nIratAction, line);

    err = at_tok_start(&line);
    if (err < 0) {
        LOGE("[RILData_C2K_IRAT] onIratEvent error 0.");
        goto error;
    }

    /* Get 1st parameter: start */
    err = at_tok_nextint(&line, &action);
    if (err < 0) {
        LOGE("[RILData_C2K_IRAT] onIratEvent error 1.");
        goto error;
    }

    /* Get 2nd parameter: source rat */
    err = at_tok_nextint(&line, &sourceRat);
    if (err < 0) {
        LOGE("[RILData_C2K_IRAT] onIratEvent error 2.");
        goto error;
    }

    /* Get 3rd parameter: target rat */
    err = at_tok_nextint(&line, &targetRat);
    if (err < 0) {
        LOGE("[RILData_C2K_IRAT] onIratEvent error 3.");
        goto error;
    }

    nIratAction = action;
    // Get IRAT type for later process.
    nIratType = getIratType(sourceRat, targetRat);

    LOGI("[RILData_C2K_IRAT] onIratEvent [%d, %d, %d, %d]", action, sourceRat,
            targetRat, nIratType);
    if (action == IRAT_ACTION_SOURCE_STARTED) {
        onIratSourceStarted(sourceRat, targetRat, nIratType);
    } else if (action == IRAT_ACTION_TARGET_STARTED) {
        onIratTargetStarted(sourceRat, targetRat, nIratType);
    } else if (action == IRAT_ACTION_SOURCE_FINISHED) {
        onIratSourceFinished(sourceRat, targetRat, nIratType);
    } else if (action == IRAT_ACTION_TARGET_FINISHED) {
        onIratTargetFinished(sourceRat, targetRat, nIratType);
    }

    free(urc);
    return;
error:
    free(urc);
    return;
}

void onLwcgIratEvent(void* urc) {
    int err = 0;
    int action = IRAT_ACTION_UNKNOWN;
    int sourceRat = 0;
    int targetRat = 0;
    char* line = urc;

    // +EPDNCTRANS: <action>,<source RAT>,<target RAT>
    // 1: source rat start  2: target rat start
    // 3: source rat finish 4: target rat finish
    // RAT: 1:1XRTT 2:HRPD 3:EHRPD 4:LTE 5:UMTS 6:GSM
    LOGD("[RILData_C2K_IRAT] onLwcgIratEvent: action = %d, urc = %s.", nIratAction, line);

    err = at_tok_start(&line);
    if (err < 0) {
        LOGE("[RILData_C2K_IRAT] onLwcgIratEvent error 0.");
        goto error;
    }

    /* Get 1st parameter: action */
    err = at_tok_nextint(&line, &action);
    if (err < 0) {
        LOGE("[RILData_C2K_IRAT] onLwcgIratEvent error 1.");
        goto error;
    }

    /* Get 2nd parameter: source rat */
    err = at_tok_nextint(&line, &sourceRat);
    if (err < 0) {
        LOGE("[RILData_C2K_IRAT] onIratEvent error 2.");
        goto error;
    }

    /* Get 3rd parameter: target rat */
    err = at_tok_nextint(&line, &targetRat);
    if (err < 0) {
        LOGE("[RILData_C2K_IRAT] onIratEvent error 3.");
        goto error;
    }

    nIratAction = action;
    nIratType = getLwcgIratType(sourceRat, targetRat);

    LOGI("[RILData_C2K_IRAT] onLwcgIratEvent [%d, %d, %d, %d]", action, sourceRat,
            targetRat, nIratType);
    if (nIratType == IRAT_TYPE_LWCG_LTE_EHRPD || nIratType == IRAT_TYPE_LWCG_EHRPD_LTE) {
        if (action == IRAT_ACTION_SOURCE_STARTED) {
            onIratSourceStarted(sourceRat, targetRat, nIratType);
        } else if (action == IRAT_ACTION_TARGET_STARTED) {
            onIratTargetStarted(sourceRat, targetRat, nIratType);
        } else if (action == IRAT_ACTION_SOURCE_FINISHED) {
            onIratSourceFinished(sourceRat, targetRat, nIratType);
        } else if (action == IRAT_ACTION_TARGET_FINISHED) {
            onIratTargetFinished(sourceRat, targetRat, nIratType);
        }
    }

    free(urc);
    return;
error:
    free(urc);
    return;
}

void onPdnSyncFromSourceRat(void* urc) {
    int err = 0;
    int cid = INVALID_CID;
    int interfaceId = INVALID_CID;
    int pdnStatus = IRAT_PDN_STATUS_UNKNOWN;
    char *out = NULL;
    char* line = urc;

    // +EGCONTRDP: <cid>,<APN_name>,<interface id>,<need_deact>, <PDP_addr_1>, [<PDP_addr2>]
    LOGI("[RILData_C2K_IRAT] onPdnSyncFromSourceRat: action = %d, urc = %s",
            nIratAction, line);

    if (nIratAction != IRAT_ACTION_TARGET_STARTED) {
        LOGD("[RILData_C2K_IRAT] Ignore onPdnSyncFromSourceRat.");
        return;
    }
    err = at_tok_start(&line);
    if (err < 0) {
        LOGE("[RILData_C2K_IRAT] onPdnSyncFromSourceRat error 0.");
        goto error;
    }

    /* Get 1st parameter: CID */
    err = at_tok_nextint(&line, &cid);
    if (err < 0) {
        LOGE("[RILData_C2K_IRAT] onPdnSyncFromSourceRat error 1.");
        goto error;
    }
    pPdnInfo[cid].cid = cid;
    pSyncPdnInfo[cid].cid = cid;

    /* Get 2nd parameter: apn name */
    err = at_tok_nextstr(&line, &out);
    if (err < 0) {
        LOGE("[RILData_C2K_IRAT] onPdnSyncFromSourceRat error 2.");
        goto error;
    }
    sprintf(pPdnInfo[cid].apn, "%s", out);
    sprintf(pSyncPdnInfo[cid].apn, "%s", out);

    /* Get 3rd parameter: interface id */
    err = at_tok_nextint(&line, &interfaceId);
    if (err < 0) {
        LOGE("[RILData_C2K_IRAT] onPdnSyncFromSourceRat error 3.");
        goto error;
    }
    // -1 because we +1 when bind data using CGDATA
    pPdnInfo[cid].interfaceId = interfaceId - 1;
    pSyncPdnInfo[cid].interfaceId = interfaceId - 1;

    /* Get 4th parameter: pdn status */
    err = at_tok_nextint(&line, &pdnStatus);
    if (err < 0) {
        LOGE("[RILData_C2K_IRAT] onPdnSyncFromSourceModem error 4.");
        goto error;
    }
    pSyncPdnInfo[cid].pdnStatus = pdnStatus;

    /* Get 5th parameter: IPAddr1 */
    err = at_tok_nextstr(&line, &out);
    if (err < 0) {
        LOGE("[RILData_C2K_IRAT] onPdnSyncFromSourceModem error 5.");
        goto error;
    }
    if (getPdnAddressType(out) == IPV4) {
        sprintf(pSyncPdnInfo[cid].addressV4, "%s", out);
    } else {
        convertIpv6Address(out, pSyncPdnInfo[cid].addressV6, 1);
    }

    /* Only for IPv4V6 + dual stack PDP context */
    /* Get 6th paramter: IPaddr 2*/
    if (at_tok_hasmore(&line)) {
        err = at_tok_nextstr(&line, &out);
        if (err < 0) {
            LOGE("[RILData_C2K_IRAT] onPdnSyncFromSourceModem error 6.");
            goto error;
        }

        if (getPdnAddressType(out) == IPV4) {
            sprintf(pSyncPdnInfo[cid].addressV4, "%s", out);
        } else {
            convertIpv6Address(out, pSyncPdnInfo[cid].addressV6, 1);
        }
    }

    pPdnInfo[cid].active = DATA_STATE_INACTIVE;
    if (pSyncPdnInfo[cid].pdnStatus == IRAT_PDN_STATUS_DEACTED) {
        nDeactPdnCount++;
    }
    nReactPdnCount++;

    LOGI("[RILData_C2K_IRAT] onPdnSyncFromSourceRat: cid = %d, interfaceId = %d, "
            "pdnCount = %d, deactPdnCount = %d, apn = %s.",
            pSyncPdnInfo[cid].cid, pSyncPdnInfo[cid].interfaceId,
            nReactPdnCount, nDeactPdnCount, pPdnInfo[cid].apn);

error:
    free(urc);
    return;
}

int rebindPdnToInterface(int interfaceId, int cid, RILChannelCtx * pChannel) {
    int nRet = 1;  // 0:failed, 1: success
    char *cmd = NULL;
    int err = 0;

    LOGI("[RILData_C2K_IRAT] rebindPdnToInterface: cid = %d, interfaceId = %d E",
            cid, interfaceId);
    /* AT+CGDATA=<L2P>,<cid>,<channel ID>  The channel id is from 1~n */
    asprintf(&cmd, "AT+CGDATA=\"%s\",%d,%d", pcL2pValue, cid, interfaceId + 1);
    err = at_send_command(cmd, NULL, pChannel);
    free(cmd);
    if (err < 0) {
        LOGE("[RILData_Setup] CID%d fail to bind interface%d", cid,
                interfaceId);
        nRet = 0;
        goto error;
    } else {
        pPdnInfo[cid].interfaceId = interfaceId;
        pPdnInfo[cid].active = DATA_STATE_ACTIVE;
    }

    LOGD("[RILData_C2K_IRAT] rebindPdnToInterface: finished X");
    return nRet;

error:
    return nRet;
}

// Reconfigure CCMNI interface, the sequence need to be in order.
// 1. Bind CCMNI interface with current MD id.
// 2. Reset IP stack if IP changed.
// 3. Resume data transfer for the interface.
void reconfigureNetworkInterface(int interfaceId) {
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(struct ifreq));
    sprintf(ifr.ifr_name, "%s", getNetworkInterfaceName(interfaceId));
    // Bind CCMNI interface with current MD id.
    ifc_ccmni_md_cfg(ifr.ifr_name, MD_SYS3);
    LOGD("[RILData_C2K_IRAT] reconfigureNetworkInterface: id = %d, name = %s.",
            interfaceId, ifr.ifr_name);

    int resetMask = 0;
    int i = 0;
    for (i = 0; i < nPdnInfoSize; i++) {
        if (pPdnInfo[i].interfaceId == interfaceId) {
            if (strlen(pSyncPdnInfo[i].addressV4) > 0) {
                if (strcmp(pSyncPdnInfo[i].addressV4, pPdnInfo[i].addressV4) != 0) {
                    resetMask |= 0x01;
                    LOGI("IPv4 address lost during IRAT %d.", interfaceId);
                }
            }
            if (strlen(pSyncPdnInfo[i].addressV6) > 0) {
                if (strcmp(pSyncPdnInfo[i].addressV6, pPdnInfo[i].addressV6) != 0) {
                    resetMask |= 0x02;
                    LOGI("IPv6 address lost during IRAT %d.", interfaceId);
                 } else {
                    // IPv6 prefix may change even if the interface-Id is not
                    resetMask |= 0x04;
                    LOGI("Only IPv6 address, need to update after IRAT %d.", interfaceId);
                }
            }
        }
    }

    LOGD("[RILData_C2K_IRAT] reconfigureNetworkInterface resetMask = %2x.", resetMask);
    // Reset IP stack if IP changed.
    if (resetMask != 0) {
        updateNetworkInterface(interfaceId, resetMask);
    }

    // Resume data transfer for the interface.
    setNetworkTransmitState(interfaceId, RESUME_DATA_TRANSFER);
}

void resumeAllDataTransmit() {
    LOGI("[RILData_C2K_IRAT] resumeAllDataTransmit...");
    int i = 0;
    for (i = 0; i < nPdnInfoSize; i++) {
        if (pPdnInfo[i].active == DATA_STATE_ACTIVE) {
            setNetworkTransmitState(pPdnInfo[i].interfaceId, RESUME_DATA_TRANSFER);
        }
    }
}

void suspendAllDataTransmit() {
    LOGI("[RILData_C2K_IRAT] suspendAllDataTransmit...");
    int i = 0;
    for (i = 0; i < nPdnInfoSize; i++) {
        if (pPdnInfo[i].active == DATA_STATE_ACTIVE) {
            setNetworkTransmitState(pPdnInfo[i].interfaceId, SUSPEND_DATA_TRANSFER);
        }
    }
}

void setNetworkTransmitState(int interfaceId, int state) {
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(struct ifreq));
    sprintf(ifr.ifr_name, "%s", getNetworkInterfaceName(interfaceId));
    LOGI("[RILData_C2K_IRAT] setNetworkTransmitState: id = %d, name= %s, state = %d.",
            interfaceId, ifr.ifr_name, state);
    ifc_set_txq_state(ifr.ifr_name, state);
}

void handleDeactedOrFailedPdns(int retryFailedPdn) {
    if (pSyncPdnInfo != NULL) {
        int i = 0;
        for (i = 0; i < nPdnInfoSize; i++) {
            LOGI("[RILData_C2K_IRAT] handleDeactedOrFailedPdns: i = %d, cid = %d, status = %d.",
                    i, pSyncPdnInfo[i].cid, pSyncPdnInfo[i].pdnStatus);
            // Deactivate PDN which is already deactivate in source RAT.
            if (pSyncPdnInfo[i].pdnStatus == IRAT_PDN_STATUS_DEACTED) {
                deactivatePdnByCid(pSyncPdnInfo[i].cid);
            }
        }

        if (retryFailedPdn) {
            for (i = 0; i < nPdnInfoSize; i++) {
                LOGI("[RILData_C2K_IRAT] handleDeactedOrFailedPdns: i = %d, cid = %d, status = %d.",
                        i, pSyncPdnInfo[i].cid, pSyncPdnInfo[i].pdnStatus);
                if (pSyncPdnInfo[i].pdnStatus == IRAT_PDN_STATUS_SYNCED) {
                    retryForFailedPdnByCid(pSyncPdnInfo[i].cid);
                }
            }
        }
    }
}

void handleLwcgFailedPdns() {
    int i = 0;
    for (i = 0; i < nPdnInfoSize; i++) {
        LOGI("[RILData_C2K_IRAT] handleDeactedOrFailedPdns: i = %d, cid = %d, status = %d.",
                i, pSyncPdnInfo[i].cid, pSyncPdnInfo[i].pdnStatus);
        if (pSyncPdnInfo[i].pdnStatus == IRAT_PDN_STATUS_SYNCED) {
            retryForLwcgFailedPdnByCid(pSyncPdnInfo[i].cid);
        }
    }
}

// Retry to deactivate PDN in target RAT which is already deactivated by source RAT,
// we don't check the result even if the deactivate failed.
void deactivatePdnByCid(int cid) {
    LOGD("[RILData_C2K_IRAT] deactivatePdnByCid: cid = %d.", cid);
    char *cmd = NULL;
    ATResponse *p_response = NULL;
    RIL_SOCKET_ID rid = getCdmaSocketSlotId() - 1;
    RILChannelCtx *pChannel = getChannelCtxbyProxy(rid);

    asprintf(&cmd, "AT+CGACT=0,%d", cid);
    int err = at_send_command_notimeout(cmd, &p_response, pChannel);
    free(cmd);
    if (p_response == NULL) {
        LOGE("[%s] p_response is NULL", __FUNCTION__);
        goto error;
    }
    if (isATCmdRspErr(err, p_response)) {
        if (p_response == NULL) {
            LOGE("[RILData_C2K_IRAT] deactivatePdnByCid p_response is null!");
            goto error;
        }
        // Go on even if deactivated fail, clean up PDN info in RILD.
        int errCode = at_get_cme_error(p_response);
        if (errCode == PS_CMD_CONFLICT_C2K_AP_DEACT_RETRY_NEEDED) {
            pPdnInfo[cid].active = DATA_STATE_NEED_DEACT;
        }
        LOGE("[RILData_C2K_IRAT] deactivatePdnByCid[%d] failed with error[%d].",
                cid, errCode);
        goto error;
    }

    configureNetworkInterface(pPdnInfo[cid].interfaceId, DISABLE_CCMNI);
    clearPdnInfo(&pPdnInfo[cid]);
    AT_RSP_FREE(p_response);
    LOGD("[RILData_C2K_IRAT] deactivatePdnByCid finished----->.");
    return;

error:
    AT_RSP_FREE(p_response);
}

// C2K need no retry for failed PDN currently(From modem's comment).
void retryForFailedPdnByCid(int cid) {
    LOGD("[RILData_C2K_IRAT] [%s] cid = %d.", __FUNCTION__, cid);
}

void retryForLwcgFailedPdnByCid(int cid) {
    RIL_SOCKET_ID rid = getCdmaSocketSlotId() - 1;
    RILChannelCtx *pChannel = getChannelCtxbyProxy(rid);

    LOGI("[RILData_C2K_IRAT] [%s] cid = %d", __FUNCTION__, cid);
    if (0 == activatePdn(cid, pChannel)) {
        LOGE("[RILData_C2K_IRAT] [%s] activatePdn failed", __FUNCTION__);
        return;
    }

    if (updatePdnAddressByCid(cid, pChannel) < 0) {
        LOGE("[RILData_C2K_IRAT] [%s] update CID%d failed", __FUNCTION__, cid);
        return;
    }
    if (0 == rebindPdnToInterface(pPdnInfo[cid].interfaceId, cid, pChannel)) {
        LOGE("[RILData_C2K_IRAT] [%s] rebind failed", __FUNCTION__);
        return;
    }

    if (updateDnsAndMtu(pChannel) < 0) {
        LOGE("[RILData_C2K_IRAT] [%s] updateDnsAndMtu failed", __FUNCTION__);
        return;
    }

    reconfigureNetworkInterface(pSyncPdnInfo[cid].interfaceId);
    setNetworkTransmitState(pSyncPdnInfo[cid].interfaceId, RESUME_DATA_TRANSFER);
    pSyncPdnInfo[cid].pdnStatus = IRAT_PDN_STATUS_REACTED;
}

void cleanupFailedPdns() {
    if (pSyncPdnInfo != NULL) {
        int i = 0;
        for (i = 0; i < nPdnInfoSize; i++) {
            LOGI("[RILData_C2K_IRAT] cleanupFailedPdns: i = %d, cid = %d, status = %d.",
                    i, pSyncPdnInfo[i].cid, pSyncPdnInfo[i].pdnStatus);
            if (pSyncPdnInfo[i].pdnStatus == IRAT_PDN_STATUS_SYNCED) {
                cleanupPdnByCid(pSyncPdnInfo[i].cid);
            }
        }
    }
}

void cleanupPdnByCid(int cid) {
    int i = 0;
    int interfaceId = pPdnInfo[cid].interfaceId;
    LOGD("[RILData_C2K_IRAT] cleanupPdnByCid: cid = %d, interfaceId = %d.",
            cid, interfaceId);

    // Resume data transfer and disable the CCMNI interface.
    setNetworkTransmitState(interfaceId, RESUME_DATA_TRANSFER);
    configureNetworkInterface(interfaceId, DISABLE_CCMNI);
    clearPdnInfo(&pPdnInfo[cid]);
}

void cleanupPdnsForFallback() {
    if (pPdnInfo != NULL) {
        int i = 0;
        for (i = 0; i < nPdnInfoSize; i++) {
            LOGI("[RILData_C2K_IRAT] cleanupPdnsForFallback: i = %d, cid = %d, active = %d.",
                    i, pPdnInfo[i].cid, pPdnInfo[i].active);
            if (pPdnInfo[i].cid != INVALID_CID) {
                cleanupPdnByCid(pPdnInfo[i].cid);
            }
        }
    }
}

void clearAllSyncPdnInfo() {
    if (pSyncPdnInfo != NULL) {
        int i = 0;
        for (i = 0; i < nPdnInfoSize; i++) {
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

void onUpdateIratStatus() {
    nIratDeactCount++;
    LOGD("[onUpdateIratStatus] rid = %d", DATA_CHANNEL);
    RIL_requestProxyTimedCallback(onResetIratStatus, NULL,
            &TIMEVAL_10, DATA_CHANNEL);
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

// Get modem suggest retry time.
int getModemSuggestedRetryTime(int queryCid, RILChannelCtx *pChannel) {
    int err = 0;
    char *cmd = NULL;
    ATResponse *p_response = NULL;
    char* line = NULL;
    int modemCid = INVALID_CID;
    int suggestion = NO_SUGGESTION;
    int suggestedRetryTime = NO_SUGGESTED_TIME;

    /* Use AT+EACTDELAYT=cid to query MD suggested retry time via this cid */
    asprintf(&cmd, "AT+EACTDELAYT=%d", queryCid);

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
    if (p_response == NULL) {
        LOGE("[%s] p_response is NULL", __FUNCTION__);
        goto error;
    }
    if (isATCmdRspErr(err, p_response)) {
        LOGE("[%s] Modem isn't ready to support suggested retry time", __FUNCTION__);
        goto error;
    } else {
        line = p_response->p_intermediates->line;

        err = at_tok_start(&line);
        if (err < 0) goto error;

        err = at_tok_nextint(&line, &modemCid);
        if (err < 0) goto error;

        if (modemCid != queryCid) {
            LOGE("[%s] cid%d is different to MD assigned cid%d", __FUNCTION__, queryCid, modemCid);
            goto error;
        }

        err = at_tok_nextint(&line, &suggestion);
        if (err < 0) goto error;

        if ((suggestion == HAS_PDN_RETRY_TIME_SUGGESTION) && at_tok_hasmore(&line)) {
            err = at_tok_nextint(&line, &suggestedRetryTime);
            if (err < 0) goto error;
        }

        // Convert second of modem suggested to millisecond to send to FW.
        if (suggestedRetryTime > 0) {
            suggestedRetryTime *= 1000;
        }

        LOGI("[%s] response [cid=%d, suggestion=%d, time=%d]",
                __FUNCTION__, modemCid, suggestion, suggestedRetryTime);
    }

    AT_RSP_FREE(p_response);
    LOGD("[%s] finished X", __FUNCTION__);
    return suggestedRetryTime;

error:
    AT_RSP_FREE(p_response);
    LOGE("[%s] parsing error X", __FUNCTION__);
    return NO_SUGGESTED_TIME;
}

void setNetworkInterfaceDown(const char* interfaceName) {
    ifc_reset_connections(interfaceName, RESET_ALL_ADDRESSES);
    ifc_remove_default_route(interfaceName);
    ifc_disable(interfaceName);
}

// IMS @{
int prepareImsParamInfo(int interfaceId, RIL_Default_Bearer_VA_Config_Struct *pImsParam) {
    // 0: Success;  -1: Fail
    int ret = 0;
    // Query IMS non-AOSP parameters from MAL.
    ret = getImsParamInfo(interfaceId, pImsParam);

    return ret;
}

int getImsParamInfo(int intfId,
        RIL_Default_Bearer_VA_Config_Struct *pImsParam) {
    // interface id for MAL, IMS: 5, EIMS: 6
    // 0: Success;  -1: Fail
    int ret = 0;

    // Value of CDMA slot ID is start from 1, transfer to zero base to align GSM.
    int rid = getCdmaSocketSlotId() - 1;
    LOGD("[%d][%s] with interface ID %d", rid, __FUNCTION__, intfId);

    if (isImsSupport() && (pImsParam != NULL)) {
        if (sizeof(mal_datamngr_ims_info_rsp_t) == sizeof(RIL_Default_Bearer_VA_Config_Struct)) {
            mal_datamngr_ims_info_req_ptr_t req_ptr =
                    calloc(1, sizeof(mal_datamngr_ims_info_req_t));
            assert(req_ptr != NULL);
            req_ptr->network_id = intfId;

            mal_datamngr_ims_info_rsp_ptr_t rsp_ptr =
                    (mal_datamngr_ims_info_rsp_ptr_t) pImsParam;
            int sim_id = getValidSimId();
            if (sim_id != SIM_ID_INVALID) {
                if (mal_datamngr_get_ims_info_tlv != NULL) {
                    ret = mal_datamngr_get_ims_info_tlv (mal_once(1, mal_cfg_type_sim_id, sim_id), req_ptr, rsp_ptr,
                            0, NULL, NULL, NULL);
                    LOGD("[%s] Call mal_datamngr_get_ims_info_tlv success", __FUNCTION__);
                } else if (mal_datamngr_get_ims_info != NULL) {
                    ret = mal_datamngr_get_ims_info(
                            mal_once(1, mal_cfg_type_sim_id, sim_id),
                            req_ptr, rsp_ptr);
                    LOGD("[%s] Call mal_datamngr_get_ims_info success", __FUNCTION__);
                } else {
                    LOGE("[%s] mal_datamngr_get_ims_info and mal_datamngr_get_ims_info_tlv are null", __FUNCTION__);
                }
                // TODO: Handle response code from MAL
            } else {
                LOGE("[%d][%s] Skip with invalid SIM ID: %d", rid, __FUNCTION__, sim_id);
            }
            FREEIF(req_ptr);
        } else {
            LOGE("[%d][%s] mismatch structure between RILD and MAL!", rid, __FUNCTION__);
        }
    }
    return ret;
}

void initialMalDataCallResponse(mal_datamngr_data_call_info_req_ptr_t responses, int length) {
    int i = 0;
    for (i = 0; i < length; i++) {
        memset(&responses[i], 0, sizeof(mal_datamngr_data_call_info_req_t));
        responses[i].status = PDP_FAIL_ERROR_UNSPECIFIED;
        responses[i].cid = INVALID_CID;
        // TODO: Remove after fusion design, RAT now always be 3GPP2.
        responses[i].eran_type = RDS_RAN_MOBILE_3GPP2;
        // No dedicated bearer for IMS
        responses[i].concatenateNum = 0;
    }
}

void fillMalDataCallResponse(MTK_RIL_Data_Call_Response_v11* response,
        mal_datamngr_data_call_info_req_ptr_t responseToMal) {

    if (response == NULL) {
        LOGE("[%s], NULL data call response, skip", __FUNCTION__);
        return;
    }

    if (response->active == DATA_STATE_ACTIVE) {
        // Fill AOSP defined structure first
        responseToMal->active = response->active;
        responseToMal->mtu = response->mtu;
        responseToMal->status = response->status;
        responseToMal->cid = response->cid;
        responseToMal->eran_type = response->rat;
        if (response->type != NULL) {
            responseToMal->type = strdup(response->type);
        }
        if (response->ifname != NULL) {
            responseToMal->ifname = strdup(response->ifname);
        }
        if (response->addresses != NULL) {
            responseToMal->addresses = strdup(response->addresses);
        }
        if (response->dnses != NULL) {
            responseToMal->dnses = strdup(response->dnses);
        }
        if (response->gateways != NULL) {
            responseToMal->gateways = strdup(response->gateways);
        }
        if (response->pcscf != NULL) {
            responseToMal->pcscf = strdup(response->pcscf);
        }

        // Default bearer
        updateDedicateDataResponse(response->cid, &responseToMal->defaultBearer);
    } else {
        LOGD("[%s], cid: %d is not active, skip", __FUNCTION__, response->cid);
    }
}

void updateDedicateDataResponse(int cid, MALRIL_Dedicate_Data_Call_Struct* response) {
    PdnInfo *pdnInfo = &pPdnInfo[cid];
    // No dedicated bearer over C2K, primaryCid = cid
    response->primaryCid = pdnInfo->cid;
    response->cid = pdnInfo->cid;
    response->bearerId = pdnInfo->bearerId;
    response->interfaceId = pdnInfo->interfaceId;
    response->active = pdnInfo->active;
    response->signalingFlag = pdnInfo->signalingFlag;
    // c2k doesn't need Qos & Tft
    response->hasQos = 0;
    response->hasTft = 0;

    int i = 0;
    for (i = 0; i < MAX_PCSCF_NUMBER; i++) {
        if (strlen(pdnInfo->pcscf[i]) > 0) {
            response->hasPcscf = 1;
            if (i == 0) {
                sprintf(response->pcscf + strlen(response->pcscf), "%s", pdnInfo->pcscf[i]);
            } else {
                sprintf(response->pcscf + strlen(response->pcscf), " %s", pdnInfo->pcscf[i]);
            }
        }
    }
}

void responseUnsolDataCallRspToMal(const mal_datamngr_data_call_info_req_ptr_t pRespData) {
    // Only response this duplicate response to MAL while requst is not from MAL
    if (isImsSupport()) {
        RIL_SOCKET_ID rid = getCdmaSocketSlotId() - 1;
        if (pRespData) {
            LOGD("[%d][%s] eran_type:%d", rid, __FUNCTION__, pRespData->eran_type);
        } else {
            LOGD("[%d][%s] error, respData: Null", rid, __FUNCTION__);
        }

        mal_datamngr_data_call_info_req_ptr_t pReqInfo =
                (mal_datamngr_data_call_info_req_ptr_t) pRespData;
        mal_datamngr_data_call_info_rsp_ptr_t pRspInfo =
                calloc(1, sizeof(mal_datamngr_data_call_info_rsp_t));
        assert(pRspInfo != NULL);
        // Let's forward it!
        int sim_id = getValidSimId();
        if (sim_id != SIM_ID_INVALID) {
            if (mal_datamngr_set_data_call_info_tlv != NULL) {
                mal_datamngr_set_data_call_info_tlv (mal_once(1, mal_cfg_type_sim_id, sim_id), pReqInfo, pRspInfo, 0, NULL, NULL, NULL);
                LOGD("[%s] Call mal_datamngr_set_data_call_info_tlv success", __FUNCTION__);
            } else if (mal_datamngr_set_data_call_info != NULL) {
                mal_datamngr_set_data_call_info(
                        mal_once(1, mal_cfg_type_sim_id, sim_id), // The control params.
                        pReqInfo, // Data call response to MAL.
                        pRspInfo); // Response from MAL.
                LOGD("[%s] Call mal_datamngr_set_data_call_info success", __FUNCTION__);
            } else {
                LOGE("[%s] mal_datamngr_set_data_call_info and mal_datamngr_set_data_call_info_tlv are null", __FUNCTION__);
            }
            // TODO: Handle response code from MAL.
        }
        FREEIF(pRspInfo);
    }
}

void responseUnsolImsDataCallListToMal(RILChannelCtx* rilchnlctx,
        char *apn, int interfaceId, int rid) {
    LOGD("%s apn: %s, interface: %d, rid: %d", __FUNCTION__, apn, interfaceId, rid);

    if ((NULL != apn) && (NULL != strcasestr(apn, "ims"))) {
        int sim_id = getValidSimId(rid, __FUNCTION__);
        MTK_RIL_Data_Call_Response_v11* response = (MTK_RIL_Data_Call_Response_v11*)
                calloc(1, sizeof(MTK_RIL_Data_Call_Response_v11));
        assert(response != NULL);
        mal_datamngr_data_call_info_req_ptr_t responseToMal = (mal_datamngr_data_call_info_req_ptr_t) calloc(1,
                sizeof(mal_datamngr_data_call_info_req_t));
        assert(responseToMal != NULL);

        if (response != NULL && sim_id != SIM_ID_INVALID) {
            initialDataCallResponse(response, 1);
            initialMalDataCallResponse(responseToMal, 1);
            updateDefaultBearerInfo(interfaceId, rilchnlctx);
            fillDataCallResponse(interfaceId, response);
            fillMalDataCallResponse(response, responseToMal);

            dumpDataResponse(response, "responseUnsolImsDataCallListToMal dump response");
            dumpDedicateDataResponse(&responseToMal->defaultBearer,
                    "responseUnsolImsDataCallListToMal dump ims response");

            mal_datamngr_data_call_info_req_ptr_t req_ptr =
                    (mal_datamngr_data_call_info_req_ptr_t) responseToMal;
            mal_datamngr_data_call_info_rsp_ptr_t rsp_ptr =
                    calloc(1, sizeof(mal_datamngr_data_call_info_rsp_t));
            assert(rsp_ptr != NULL);

            if (mal_datamngr_notify_data_call_list_tlv != NULL) {
                mal_datamngr_notify_data_call_list_tlv(mal_once(1, mal_cfg_type_sim_id, sim_id), 1,
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

void freeMalDataResponse(mal_datamngr_data_call_info_req_ptr_t response) {
    if (response != NULL) {
        if (response->ifname != NULL) {
            LOGD("[%s] E type=%s, ifname=%s, addresses=%s, dnses=%s, gateways=%s",
                    __FUNCTION__, response->type, response->ifname, response->addresses,
                    response->dnses, response->gateways);
        }
        FREEIF(response->type);
        FREEIF(response->ifname);
        FREEIF(response->addresses);
        FREEIF(response->gateways);
        FREEIF(response->dnses);
        FREEIF(response->pcscf);
    }
}

// Sync PDN deactivation cause to MAL
void updatePdnDeactStateToMal(unsigned interfaceId, int state, char* cause) {
    LOGD("%s interface: %d, state: %d, cause: %s", __FUNCTION__, interfaceId, state, cause);

    mal_datamngr_set_pdn_state_req_ptr_t req =
            calloc(1, sizeof(mal_datamngr_set_pdn_state_req_t));
    assert(req != NULL);
    mal_datamngr_set_pdn_state_rsp_ptr_t rsp =
            calloc(1, sizeof(mal_datamngr_set_pdn_state_rsp_t));
    assert(rsp != NULL);
    req->pdn_id = interfaceId;
    req->state = state;

    if (mal_datamngr_set_pdn_state_tlv != NULL) {
        mal_datamngr_set_pdn_state_tlv(mal_once(0), req, rsp, 0, NULL, NULL, NULL);
        LOGD("[%s] Call mal_datamngr_set_pdn_state_tlv success", __FUNCTION__);
    } else if (mal_datamngr_set_pdn_state != NULL) {
        mal_datamngr_set_pdn_state(mal_once(0), req, rsp);
        LOGD("[%s] Call mal_datamngr_set_pdn_state success", __FUNCTION__);
    } else{
        LOGE("[%s] mal_datamngr_set_pdn_state and mal_datamngr_set_pdn_state_tlv are null", __FUNCTION__);
    }

    FREEIF(req);
    FREEIF(rsp);
}

void resetPdnDeactStateToMal(unsigned interfaceId, char* cause) {
    LOGD("%s interface: %d, cause: %s", __FUNCTION__, interfaceId, cause);
    updatePdnDeactStateToMal(interfaceId, PDP_FAIL_LOST_CONNECTION, cause);
}

// IMS Workaround patch for VzW LTE Stress 6.7
bool skipIratUrcToMal(const char *urc) {
    if (nIratAction == IRAT_ACTION_SOURCE_STARTED || nIratAction == IRAT_ACTION_SOURCE_FINISHED) {
        if (strStartsWith(urc, "+CGEV: NW PDN DEACT") ||
                strStartsWith(urc, "+CGEV: ME PDN DEACT")) {
            RLOGD("[%s] skip urc %s", __FUNCTION__, urc);
            return true;
        }
    }
    return false;
}

int updateDefaultBearerInfo(int interfaceId, RILChannelCtx* rilchnlctx) {
    //Be aware that this method is used only for retrieving P-CSCF address and signaling flag for c2k
    ATResponse *p_response = NULL;
    ATLine *p_cur = NULL;
    int responseCount = 0;
    int i = 0;
    int cid = 0;
    char *cmd = NULL;

    for (i = 0; i < nPdnInfoSize; i++) {
        //clear default bearer information with specified CID in table since we would query it again
        if (interfaceId == pPdnInfo[i].interfaceId) {
            cid = i;
            memset(pPdnInfo[cid].pcscf, 0, MAX_PCSCF_NUMBER * MAX_IPV6_ADDRESS_LENGTH * sizeof(char));
            memset(pPdnInfo[cid].apn, 0, sizeof(pPdnInfo[i].apn));
            pPdnInfo[cid].signalingFlag = 0;
            pPdnInfo[cid].bearerId = INVALID_CID;
            break;
        }
    }

    // +CGCONTRDP=<cid> will response <cid>,[<bearer_id>],<apn>,<local_addr and subnet_mask>,<gw_addr>,
    // <DNS_prim_addr>,<DNS_sec_addr>,<P-CSCF_prim_addr>,<P-CSCF_sec_addr>,<IM_CN_Signalling_Flag>
    asprintf(&cmd, "AT+CGCONTRDP=%d", cid);
    int err = at_send_command_multiline(cmd, "+CGCONTRDP:", &p_response, rilchnlctx);
    if (p_response == NULL) {
        LOGE("[%s] p_response is NULL", __FUNCTION__);
        goto error;
    }
    if (isATCmdRspErr(err, p_response)) {
        LOGE("updateDefaultBearerInfo AT+CGCONTRDP response ERROR");
        goto error;
    }

    for (p_cur = p_response->p_intermediates; p_cur != NULL;
            p_cur = p_cur->p_next) {
        responseCount++;
    }
    LOGD("[RILData_Query] updateDefaultBearerInfo PDN capability [%d, %d]", responseCount,
            nPdnInfoSize);

    int responseCid = 0;
    int bearerId = 0;
    char* apn = NULL;
    char* out = NULL;
    int count = 0;

    for (p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next) {
        char *line = p_cur->line;
        // Start parse response command
        err = at_tok_start(&line);
        if (err < 0) goto error;

        // <cid>
        err = at_tok_nextint(&line, &responseCid);
        if (err < 0) goto error;

        if (responseCid == cid) {
            // <bearer_id>
            err = at_tok_nextint(&line, &pPdnInfo[cid].bearerId);
            if (err < 0) LOGD("updateDefaultBearerInfo empty bearer_id");

            // <apn>
            err = at_tok_nextstr(&line, &apn);
            if (err < 0) goto error;
            if (strlen(apn) > 0) {
                strncpy(pPdnInfo[cid].apn, apn, MAX_APN_NAME_LENGTH - 1);
                pPdnInfo[cid].apn[MAX_APN_NAME_LENGTH - 1] = '\0';
            }

            // <local_addr and subnet_mask>
            err = at_tok_nextstr(&line, &out);
            if (err < 0) goto error;

            // <gw_addr>
            err = at_tok_nextstr(&line, &out);
            if (err < 0) goto error;

            // <DNS_prim_addr>
            err = at_tok_nextstr(&line, &out);
            if (err < 0) goto error;

            // <DNS_sec_addr>
            err = at_tok_nextstr(&line, &out);
            if (err < 0) goto error;

            // <P-CSCF_prim_addr>
            err = at_tok_nextstr(&line, &out);
            if (err < 0) goto error;
            if (strlen(out) > 0) {
                strncpy(pPdnInfo[cid].pcscf[count], out, MAX_IPV6_ADDRESS_LENGTH - 1);
                pPdnInfo[cid].pcscf[count++][MAX_IPV6_ADDRESS_LENGTH - 1] = '\0';
            }

            // <P-CSCF_sec_addr>
            err = at_tok_nextstr(&line, &out);
            if (err < 0) goto error;
            if (strlen(out) > 0) {
                strncpy(pPdnInfo[cid].pcscf[count], out, MAX_IPV6_ADDRESS_LENGTH - 1);
                pPdnInfo[cid].pcscf[count++][MAX_IPV6_ADDRESS_LENGTH - 1] = '\0';
            }

            // <IM_CN_Signalling_Flag>
            err = at_tok_nextstr(&line, &out);
            if (err < 0) goto error;
        } else {
            LOGD("AT+CGCONTRDP response cid=%d is not match to curr cid=%d", responseCid, cid);
            continue;
        }
    }

done:
    AT_RSP_FREE(p_response);
    free(cmd);
    return responseCount;
error:
    AT_RSP_FREE(p_response);
    free(cmd);
    return -1;

}

void dumpDedicateDataResponse(MALRIL_Dedicate_Data_Call_Struct* response, const char* reason) {
    if (response != NULL) {
        LOGD("%s data call response: ddcid=%d, interfaceId=%d, primaryCid=%d, cid=%d, active=%d, \
                signalingFlag=%d, bearerId=%d, failCause=%d, pcscf=%s, hasQos=%d, hasTft=%d, hasPcscf=%d",
            reason == NULL ? "dumpDedicateDataResponse" : reason, response->ddcId,
            response->interfaceId, response->primaryCid, response->cid, response->active,
            response->signalingFlag, response->bearerId, response->failCause, response->pcscf,
            response->hasQos, response->hasTft, response->hasPcscf);
    }
}


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

int getValidSimId() {
    int simId = SIM_ID_INVALID;
    int rid = getCdmaSocketSlotId() - 1;

    if (rid >= RIL_SOCKET_NUM || rid < 0) {
        LOGE("%s Invalid rid %d!", __FUNCTION__, rid);
    } else {
        simId = rid;
    }
    return simId;
}

void requestGetImsDataCallInfo(void *data, size_t datalen, RIL_Token t) {
    // VzW possibly cdma-less.
    LOGE("[%s] Not support currently!", __FUNCTION__);
    RIL_onRequestComplete(t, RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0);
}

void requestReuseImsDataCall(void *data, size_t datalen, RIL_Token t) {
    // VzW possibly cdma-less.
    LOGE("[%s] Not support currently!", __FUNCTION__);
    RIL_onRequestComplete(t, RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0);
}

/// IMS @}

// LCE service start
void requestStartLce(void* data, size_t datalen, RIL_Token t) {
    UNUSED(datalen);
    int reportIntervalMs = 0;
    int mode = 0;
    RIL_LceStatusInfo* response = (RIL_LceStatusInfo*) calloc(1, sizeof(RIL_LceStatusInfo));
    assert(response != NULL);

    reportIntervalMs = ((int*) data)[0];
    mode = ((int*) data)[1] + 1; // PULL mode: 2; PUSH mode: 1;
    LOGD("[%s] reportIntervalMs=%d, mode=%d", __FUNCTION__, reportIntervalMs, mode);

    // [NOTE] MD3 will return error currently.
    if (0 == setLceMode(mode, reportIntervalMs, response, getRILChannelCtxFromToken(t))) {
        LOGE("[%s] error", __FUNCTION__);
        RIL_onRequestComplete(t, RIL_E_MODEM_ERR, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(RIL_LceStatusInfo));
    }

    free(response);
    LOGD("[%s] finish", __FUNCTION__);
}

void requestStopLce(void* data, size_t datalen, RIL_Token t) {
    UNUSED(data);
    UNUSED(datalen);
    int reportIntervalMs = 0;
    int mode = 0; // STOP mode: 0;
    RIL_LceStatusInfo* response = (RIL_LceStatusInfo*) calloc(1, sizeof(RIL_LceStatusInfo));
    assert(response != NULL);

    LOGD("[%s] enter", __FUNCTION__);

    // [NOTE] MD3 will return error currently.
    if (0 == setLceMode(mode, reportIntervalMs, response, getRILChannelCtxFromToken(t))) {
        LOGE("[%s] error", __FUNCTION__);
        RIL_onRequestComplete(t, RIL_E_MODEM_ERR, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(RIL_LceStatusInfo));
    }

    free(response);
    LOGD("[%s] finish", __FUNCTION__);
}

void requestPullLceData(void* data, size_t datalen, RIL_Token t) {
    UNUSED(data);
    UNUSED(datalen);
    int err = 0;
    ATResponse *p_response = NULL;
    char* line;
    int t_mode = 0; // STOP mode: 0; START with PUSH mode: 1; START with PULL mode: 2;
    int t_last_hop_capacity_kbps = 0;
    int t_confidence_level = 0;
    int t_lce_suspended = 0;
    RIL_LceDataInfo* response = (RIL_LceDataInfo*) calloc(1, sizeof(RIL_LceDataInfo));
    assert(response != NULL);

    // [NOTE] MD3 will return error currently.
    // +ELCE: <mode>[,<last_hop_cap>, <confidenc_level>,<lce_suspend>]
    err = at_send_command_singleline("AT+ELCE?", "+ELCE:", &p_response, getRILChannelCtxFromToken(t));
    if (p_response == NULL) {
        LOGE("[%s] p_response is NULL", __FUNCTION__);
        goto error;
    }
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

        LOGI("[%s] response [mode=%d, last hop capacity kbps=%u, confidence level=%u, \
lce suspended=%u]", __FUNCTION__, t_mode, t_last_hop_capacity_kbps, t_confidence_level,
                t_lce_suspended);
        response->last_hop_capacity_kbps = (unsigned int) t_last_hop_capacity_kbps;
        response->confidence_level = (unsigned char) t_confidence_level;
        response->lce_suspended = (unsigned char) t_lce_suspended;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(RIL_LceDataInfo));
    goto finish;

error:
    LOGE("[%s] error", __FUNCTION__);
    RIL_onRequestComplete(t, RIL_E_MODEM_ERR, NULL, 0);

finish:
    free(response);
    AT_RSP_FREE(p_response);
    LOGD("[%s] finish", __FUNCTION__);
}

int setLceMode(int lceMode, int reportIntervalMs, RIL_LceStatusInfo* response,
        RILChannelCtx *pChannel) {
    int nRet = 0;
    int err = 0;
    char *cmd = NULL;
    ATResponse *p_response = NULL;
    char* line = NULL;
    int t_lce_status = -1; // not supported: -1; stopped: 0; active: 1;
    int t_actual_interval_ms = 0;

    if (lceMode > 0) {
        asprintf(&cmd, "AT+ELCE=%d,%d", lceMode, reportIntervalMs);
    } else {
        asprintf(&cmd, "AT+ELCE=%d", lceMode);
    }

    // [NOTE] MD3 will return error currently.
    err = at_send_command_singleline(cmd, "+ELCE:", &p_response, pChannel);
    free(cmd);
    if (p_response == NULL) {
        LOGE("[%s] p_response is NULL", __FUNCTION__);
        goto error;
    }
    if (err != 0 || p_response->success == 0 || p_response->p_intermediates == NULL) {
        LOGE("[%s] got error response", __FUNCTION__);
        response->lce_status = (char) t_lce_status;
    } else {
        line = p_response->p_intermediates->line;

        err = at_tok_start(&line);
        if (err < 0) goto error;

        err = at_tok_nextint(&line, &t_lce_status);
        if (err < 0) goto error;

        if (at_tok_hasmore(&line)) {
            err = at_tok_nextint(&line, &t_actual_interval_ms);
            if (err < 0) goto error;
        }

        LOGI("[%s] response [lce status=%d, actual interval ms=%u]",
                __FUNCTION__, t_lce_status, t_actual_interval_ms);
        response->lce_status = (char) t_lce_status;
        response->actual_interval_ms = (unsigned int) t_actual_interval_ms;
    }

    nRet = 1;
    AT_RSP_FREE(p_response);
    LOGD("[%s] finished X", __FUNCTION__);
    return nRet;

error:
    AT_RSP_FREE(p_response);
    LOGE("[%s] parsing error X", __FUNCTION__);
    return nRet;
}

void requestSetLinkCapacityReportingCriteria(void* data, size_t datalen, RIL_Token t)
{
    UNUSED(data);
    UNUSED(datalen);
    // New version of LCE service in legacy modem is not ready yet.
    RIL_onRequestComplete(t, RIL_E_LCE_NOT_SUPPORTED, NULL, 0);
}
// LCE service end

// sync attach APN to C2K also which will be sync to MD1
// when IRAT, and MD1 will use MD3 sync value as first priority.
void requestSetInitialAttachApn(void * data, size_t datalen, RIL_Token t) {
    UNUSED(datalen);
    char *cmd = NULL;
    RIL_InitialAttachApn_v15* param = data;
    RILChannelCtx* pChannel = getRILChannelCtxFromToken(t);

    LOGD("requestSetInitialAttachApn APN: %s", param->apn);

    // +EIAAPN=<apn>
    asprintf(&cmd, "AT+EIAAPN=\"%s\"", param->apn);
    int err = at_send_command(cmd, NULL, pChannel);
    free(cmd);
    if (err < 0) {
        LOGE("requestSetInitialAttachApn AT+EIAAPN response ERROR");
        RIL_onRequestComplete(t, RIL_E_MODEM_ERR, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
}

void syncApnTable(void* data, size_t datalen, RIL_Token t) {
    RILChannelCtx* pChannel = getRILChannelCtxFromToken(t);
    ATResponse *p_response = NULL;
    RIL_MtkDataProfileInfo **dataProfilePtrs = (RIL_MtkDataProfileInfo **) data;

    char *cmd = NULL;
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

    // Local ver.
    int apnClass = 0;
    int throttlingTime = 0;
    char *apnBearer = NULL;
    char *apnEnable = NULL;
    int apnTime = 0;

    LOGD("[%s] with datalen=%d, num of pararmeters = %d",
            __FUNCTION__, datalen, num);

    // Reset the old parameters in initial phase
    asprintf(&cmd, "AT+EAPNSYNC");
    err = at_send_command(cmd, &p_response, pChannel);
    free(cmd);
    if (p_response == NULL) {
        LOGE("[%s] p_response is NULL", __FUNCTION__);
        goto error;
    }
    if (isATCmdRspErr(err, p_response)) {
        failCause = at_get_cme_error(p_response);
        LOGE("syncApnInfo AT+EAPNSYNC err=%d in initial phase", failCause);
    }
    AT_RSP_FREE(p_response);


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
        asprintf(&cmd, "AT+EAPNSYNC=%d,\"%s\",%d", i, apn, inactiveTimer);
        err = at_send_command(cmd, &p_response, pChannel);
        free(cmd);
        if (p_response == NULL) {
            LOGE("[%s] p_response is NULL", __FUNCTION__);
            goto error;
        }
        if (isATCmdRspErr(err, p_response)) {
            failCause = at_get_cme_error(p_response);
            LOGE("syncApnInfo AT+EAPNSYNC err=%d", failCause);
        }
        AT_RSP_FREE(p_response);

        // AT+VZWAPNE=<wapn>,<apncl>,<apnni>,<apntype>,<apnb>,<apned>,<apntime>
        asprintf(&cmd, "AT+VZWAPNE=%d,%d,\"%s\",\"%s\",\"%s\",\"%s\",%d",
                i, apnClass, apn, protocol, apnBearer, apnEnable, apnTime);
        err = at_send_command(cmd, &p_response, pChannel);
        free(cmd);
        if (p_response == NULL) {
            LOGE("[%s] p_response is NULL", __FUNCTION__);
            goto error;
        }
        if (isATCmdRspErr(err, p_response)) {
            failCause = at_get_cme_error(p_response);
            LOGE("syncApnInfo AT+VZWAPNE err=%d", failCause);
            goto error;
        }
        AT_RSP_FREE(p_response);

        // AT+VZWAPNETMR=<wapn>,<max_conn>,<max_conn_t>,<wait_time>,<throttling_time>
        asprintf(&cmd, "AT+VZWAPNETMR=%d,%d,%d,%d,%d",
                i, maxConns, maxConnsTime, waitTime, throttlingTime);
        err = at_send_command(cmd, &p_response, pChannel);
        free(cmd);
        if (p_response == NULL) {
            LOGE("[%s] p_response is NULL", __FUNCTION__);
            goto error;
        }
        if (isATCmdRspErr(err, p_response)) {
            failCause = at_get_cme_error(p_response);
            LOGE("syncApnTimer AT+VZWAPNETMR err=%d", failCause);
            // c2k modem don't support this command, it will fail, ignore it.
        }
        AT_RSP_FREE(p_response);
    }
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;

error:
    AT_RSP_FREE(p_response);
    RIL_onRequestComplete(t, RIL_E_MODEM_ERR, NULL, 0);
}

// Multi-PS Attach
void requestDeactiveAllDataCall(void * data, size_t datalen, RIL_Token t) {
    LOGD("[%s] start with datalen=%d", __FUNCTION__, datalen);

    for (int i = 0; i < nPdnInfoSize; i++) {
        if (pPdnInfo[i].cid != INVALID_CID) {
            deactivatePdnByCid(pPdnInfo[i].cid);
        }
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
}

// LCE URC start
void onLceDataResult(void* urc)
{
    // +ELCE: <last_hop_cap>,<confidenc_level>,<lce_suspend>
    RIL_SOCKET_ID rid = getCdmaSocketSlotId() - 1;
    int ret = 0;
    unsigned int t_last_hop_capacity_kbps = 0;
    unsigned int t_confidence_level = 0;
    unsigned int t_lce_suspended = 0;
    RIL_LceDataInfo* response = NULL;
    char* line = urc;

    LOGD("[%d][%s] URC : %s", rid, __FUNCTION__, line);

    response = (RIL_LceDataInfo *)calloc(1, sizeof(RIL_LceDataInfo));
    assert(response != NULL);

    ret = at_tok_start(&line);
    if (ret < 0) goto error;

    ret = at_tok_nextint(&line, &t_last_hop_capacity_kbps);
    if (ret < 0) goto error;

    if (at_tok_hasmore(&line)) {
        ret = at_tok_nextint(&line, &t_confidence_level);
        if (ret < 0) goto error;

        if (at_tok_hasmore(&line)) {
            ret = at_tok_nextint(&line, &t_lce_suspended);
            if (ret < 0) goto error;
        }
    }

    response->last_hop_capacity_kbps = t_last_hop_capacity_kbps;
    response->confidence_level = (unsigned char) t_confidence_level;
    response->lce_suspended = (unsigned char) t_lce_suspended;

    RIL_onUnsolicitedResponse(RIL_UNSOL_LCEDATA_RECV, response, sizeof(RIL_LceDataInfo));
    goto finish;

error:
    LOGE("[%d][%s] parsing error", rid, __FUNCTION__);

finish:
    LOGV("[%d][%s] finish", rid, __FUNCTION__);
    FREEIF(urc);
    FREEIF(response);
}

void onLinkCapacityEstimate(void* urc) {
    FREEIF(urc);
    LOGD("[%s] do nothing", __FUNCTION__);
}
// LCE URC end

int setUnsolResponseFilterLinkCapacityEstimate(bool enable, RIL_Token t) {
    UNUSED(enable);
    UNUSED(t);
    LOGD("[%s] do nothing", __FUNCTION__);
    return 0;
}
