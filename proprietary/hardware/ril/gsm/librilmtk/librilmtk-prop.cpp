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

#include <cutils/properties.h>
#include <telephony/mtk_ril.h>
#include <libmtkrilutils.h>
#include <rilc.h>
#include <string.h>
#include <time.h>

#undef LOG_TAG
#define LOG_TAG "RILC-MTK"

#define RIL_REQUEST_VENDOR_BASE 2000
#define RIL_LOCAL_GSM_REQUEST_VENDOR_BASE 2500
#define MIN(a,b) ((a)<(b) ? (a) : (b))

#ifdef MTK_RIL
const char* proxyIdToString(int id)
{
    switch (id) {
    case RIL_CMD_PROXY_1:
        return "RIL_CMD_PROXY_1";
    case RIL_CMD_PROXY_2:
        return "RIL_CMD_PROXY_2";
    case RIL_CMD_PROXY_3:
        return "RIL_CMD_PROXY_3";
    case RIL_CMD_PROXY_4:
        return "RIL_URC_PROXY";
    case RIL_CMD_PROXY_5:
        return "RIL_CMD_PROXY_4";
    case RIL_CMD_PROXY_6:
        return "RIL_ATCI_PROXY";
#ifdef MTK_MUX_CHANNEL_64
    case RIL_CMD_PROXY_7:
        return "RIL_CMD_PROXY_7";
    case RIL_CMD_PROXY_8:
        return "RIL_CMD_PROXY_8";
    case RIL_CMD_PROXY_9:
        return "RIL_CMD_PROXY_9";
    case RIL_CMD_PROXY_10:
        return "RIL_CMD_PROXY_10";
    case RIL_CMD_PROXY_11:
        return "RIL_CMD_PROXY_11";
#endif

    case RIL_CMD2_PROXY_1:
        return "RIL_CMD2_PROXY_1";
    case RIL_CMD2_PROXY_2:
        return "RIL_CMD2_PROXY_2";
    case RIL_CMD2_PROXY_3:
        return "RIL_CMD2_PROXY_3";
    case RIL_CMD2_PROXY_4:
        return "RIL_URC2_PROXY";
    case RIL_CMD2_PROXY_5:
        return "RIL_CMD2_PROXY_4";
    case RIL_CMD2_PROXY_6:
        return "RIL_ATCI2_PROXY";
#ifdef MTK_MUX_CHANNEL_64
    case RIL_CMD2_PROXY_7:
        return "RIL_CMD2_PROXY_7";
    case RIL_CMD2_PROXY_8:
        return "RIL_CMD2_PROXY_8";
    case RIL_CMD2_PROXY_9:
        return "RIL_CMD2_PROXY_9";
    case RIL_CMD2_PROXY_10:
        return "RIL_CMD2_PROXY_10";
    case RIL_CMD2_PROXY_11:
        return "RIL_CMD2_PROXY_11";
#endif

    case RIL_CMD3_PROXY_1:
        return "RIL_CMD3_PROXY_1";
    case RIL_CMD3_PROXY_2:
        return "RIL_CMD3_PROXY_2";
    case RIL_CMD3_PROXY_3:
        return "RIL_CMD3_PROXY_3";
    case RIL_CMD3_PROXY_4:
        return "RIL_URC3_PROXY";
    case RIL_CMD3_PROXY_5:
        return "RIL_CMD3_PROXY_4";
    case RIL_CMD3_PROXY_6:
        return "RIL_ATCI3_PROXY";
#ifdef MTK_MUX_CHANNEL_64
    case RIL_CMD3_PROXY_7:
        return "RIL_CMD3_PROXY_7";
    case RIL_CMD3_PROXY_8:
        return "RIL_CMD3_PROXY_8";
    case RIL_CMD3_PROXY_9:
        return "RIL_CMD3_PROXY_9";
    case RIL_CMD3_PROXY_10:
        return "RIL_CMD3_PROXY_10";
    case RIL_CMD3_PROXY_11:
        return "RIL_CMD3_PROXY_11";
#endif

    case RIL_CMD4_PROXY_1:
        return "RIL_CMD4_PROXY_1";
    case RIL_CMD4_PROXY_2:
        return "RIL_CMD4_PROXY_2";
    case RIL_CMD4_PROXY_3:
        return "RIL_CMD4_PROXY_3";
    case RIL_CMD4_PROXY_4:
        return "RIL_URC4_PROXY";
    case RIL_CMD4_PROXY_5:
        return "RIL_CMD4_PROXY_4";
    case RIL_CMD4_PROXY_6:
        return "RIL_ATCI4_PROXY";
#ifdef MTK_MUX_CHANNEL_64
    case RIL_CMD4_PROXY_7:
        return "RIL_CMD4_PROXY_7";
    case RIL_CMD4_PROXY_8:
        return "RIL_CMD4_PROXY_8";
    case RIL_CMD4_PROXY_9:
        return "RIL_CMD4_PROXY_9";
    case RIL_CMD4_PROXY_10:
        return "RIL_CMD4_PROXY_10";
    case RIL_CMD4_PROXY_11:
        return "RIL_CMD4_PROXY_11";
#endif

    default:
        return "<unknown proxy>";
    }
}
#endif /* MTK_RIL */

pthread_mutex_t s_ActiveSocketMutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
int activeRilSktID = INVALID_SOCKET;
pthread_mutex_t proxyMutex[RIL_SUPPORT_PROXYS];
namespace android {

extern pthread_mutex_t s_dispatchMutex;
extern pthread_cond_t s_dispatchCond;

pthread_mutex_t s_queryThreadMutex = PTHREAD_MUTEX_INITIALIZER;

RequestInfoProxy* s_Proxy[RIL_SUPPORT_PROXYS] = {NULL};
pthread_t s_tid_proxy[RIL_SUPPORT_PROXYS];

char s_LastExecCmd[RIL_SUPPORT_PROXYS][MAX_UUID_LENGTH];

static android::RequestInfoProxy* createLocalRequestInfoProxy(RequestInfo* pRI, void *buffer, size_t buflen, UserCallbackInfo* pUCI, int* pId, RIL_SOCKET_ID socket_id);
static android::RequestInfoProxy* createRequestInfoProxy(RequestInfo* pRI, void *buffer, size_t buflen, UserCallbackInfo* pUCI, int* pId, RIL_SOCKET_ID socket_id);
static int enqueueProxyList(android::RequestInfoProxy ** pProxyList, android::RequestInfoProxy *pRequest);

//sim request whitelist after sim switch, not for common use
bool isSimRequest(int32_t request);
//stk request whitelist after sim switch
bool isStkWhitelistRequest(int32_t request);

static AtResponseList* pendedUrcList1 = NULL;
static AtResponseList* pendedUrcList2 = NULL;
static AtResponseList* pendedUrcList3 = NULL;
static AtResponseList* pendedUrcList4 = NULL;

extern int s_fdCommand[MAX_SIM_COUNT];

/* Only dump requests queue for every 30 seconds or greater */
static time_t dumpRequestQueueTimeStamp;

static void writeStringToParcel(Parcel &p, const char *s) {
    char16_t *s16;
    size_t s16_len;
    s16 = strdup8to16(s, &s16_len);
    p.writeString16(s16, s16_len);
    free(s16);
}

void cacheUrc(int unsolResponse, const void *data, size_t datalen, RIL_SOCKET_ID socket_id){
    //Only the URC list we wanted.
    if (unsolResponse != RIL_UNSOL_STK_PROACTIVE_COMMAND
        && unsolResponse != RIL_UNSOL_SIM_SMS_STORAGE_FULL
        && unsolResponse != RIL_UNSOL_RADIO_CAPABILITY
        && unsolResponse != RIL_UNSOL_RESPONSE_PLMN_CHANGED
        && unsolResponse != RIL_UNSOL_RESPONSE_REGISTRATION_SUSPENDED
        && unsolResponse != RIL_LOCAL_GSM_UNSOL_CT3G_DUALMODE_CARD
        && unsolResponse != RIL_LOCAL_GSM_UNSOL_CARD_TYPE_NOTIFY
        && unsolResponse != RIL_UNSOL_IMEI_LOCK
        && unsolResponse != RIL_LOCAL_C2K_UNSOL_EF_ECC
        && unsolResponse != RIL_LOCAL_C2K_UNSOL_VIA_GPS_EVENT
        && unsolResponse != RIL_LOCAL_GSM_UNSOL_EF_ECC
        && unsolResponse != RIL_LOCAL_GSM_UNSOL_ENHANCED_MODEM_POWER
        && unsolResponse != RIL_UNSOL_WORLD_MODE_CHANGED
        && unsolResponse != RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED
        && unsolResponse != RIL_UNSOL_RESPONSE_PS_NETWORK_STATE_CHANGED
        // External SIM [Start]
        && unsolResponse != RIL_UNSOL_VSIM_OPERATION_INDICATION
        // External SIM [End]
        && unsolResponse != RIL_UNSOL_IMS_REGISTRATION_INFO
        && unsolResponse != RIL_UNSOL_IMS_ENABLE_DONE
        && unsolResponse != RIL_UNSOL_IMS_DISABLE_DONE
        && unsolResponse != RIL_UNSOL_IMS_ENABLE_START
        && unsolResponse != RIL_UNSOL_IMS_DISABLE_START
        && unsolResponse != RIL_UNSOL_GET_PROVISION_DONE
        && unsolResponse != RIL_UNSOL_VOLTE_SETTING
        && unsolResponse != RIL_UNSOL_CALL_FORWARDING
        && unsolResponse != RIL_UNSOL_HARDWARE_CONFIG_CHANGED
        && unsolResponse != RIL_LOCAL_GSM_UNSOL_ESIMIND_APPLIST
        && unsolResponse != RIL_UNSOL_SIM_SLOT_LOCK_POLICY_NOTIFY) {
        RLOGI("Don't need to cache the request");
        return;
    }
    AtResponseList* urcCur = NULL;
    AtResponseList* urcPrev = NULL;
    int pendedUrcCount = 0;

    switch(socket_id) {
        case RIL_SOCKET_1:
            urcCur = pendedUrcList1;
            break;
        case RIL_SOCKET_2:
            urcCur = pendedUrcList2;
            break;
        case RIL_SOCKET_3:
            urcCur = pendedUrcList3;
            break;
        case RIL_SOCKET_4:
            urcCur = pendedUrcList4;
            break;
        default:
            RLOGE("Socket id is wrong!!");
            return;
    }
    while (urcCur != NULL) {
        RLOGD("Pended URC:%d, RILD:%s, :%s",
            pendedUrcCount,
            rilSocketIdToString(socket_id),
            requestToString(urcCur->id));
        urcPrev = urcCur;
        urcCur = urcCur->pNext;
        pendedUrcCount++;
    }
    urcCur = (AtResponseList*)calloc(1, sizeof(AtResponseList));
    if (urcCur == NULL) {
        RLOGE("OOM");
        return;
    }
    if (urcPrev != NULL)
        urcPrev->pNext = urcCur;
    urcCur->pNext = NULL;
    urcCur->id = unsolResponse;
    urcCur->datalen = datalen;
    urcCur->data = (char*)calloc(1, datalen + 1);
    if (urcCur->data == NULL) {
        RLOGE("OOM");
        free(urcCur);
        return;
    }
    urcCur->data[datalen] = 0x0;
    memcpy(urcCur->data, data, datalen);
    if (pendedUrcCount == 0) {
        switch(socket_id) {
            case RIL_SOCKET_1:
                pendedUrcList1 = urcCur;
                break;
            case RIL_SOCKET_2:
                pendedUrcList2 = urcCur;
                break;
            case RIL_SOCKET_3:
                pendedUrcList3 = urcCur;
                break;
            case RIL_SOCKET_4:
                pendedUrcList4 = urcCur;
                break;
            default:
                RLOGE("Socket id is wrong!!");
                return;
        }
    }

    if (s_fdCommand[socket_id] != -1) {
        RLOGD("s_fdCommand [%d] = %d, sendUrc again", socket_id, s_fdCommand[socket_id]);
        sendPendedUrcs(socket_id, s_fdCommand[socket_id]);
    }
    RLOGD("Current pendedUrcCount = %d", pendedUrcCount + 1);
}

static void sendUrc(RIL_SOCKET_ID socket_id, AtResponseList* urcCached) {
    AtResponseList* urc = urcCached;
    AtResponseList* urc_temp;
    while (urc != NULL) {
        RLOGD("sendPendedUrcs RIL%s, %s",
        rilSocketIdToString(socket_id),
        requestToString(urc->id));
        RIL_onUnsolicitedResponse(urc->id, urc->data, urc->datalen, socket_id);
        free(urc->data);
        urc_temp = urc;
        urc = urc->pNext;
        free(urc_temp);
    }
}

void sendPendedUrcs(RIL_SOCKET_ID socket_id, int fdCommand) {
    RLOGD("Ready to send pended URCs, socket:%s, fdCommand:%d", rilSocketIdToString(socket_id), fdCommand);
    s_fdCommand[socket_id] = fdCommand;
    if ((RIL_SOCKET_1 == socket_id) && (fdCommand != -1)) {
        sendUrc(socket_id, pendedUrcList1);
        pendedUrcList1 = NULL;
    }
    else if ((RIL_SOCKET_2 == socket_id) && (fdCommand != -1)) {
        sendUrc(socket_id, pendedUrcList2);
        pendedUrcList2 = NULL;
    }
    else if ((RIL_SOCKET_3 == socket_id) && (fdCommand != -1)) {
        sendUrc(socket_id, pendedUrcList3);
        pendedUrcList3 = NULL;
    }
    else if ((RIL_SOCKET_4 == socket_id) && (fdCommand != -1)) {
        sendUrc(socket_id, pendedUrcList4);
        pendedUrcList4 = NULL;
    }
}

// External SIM [Start]
static AtResponseList* pendedVsimUrcList1 = NULL;
static AtResponseList* pendedVsimUrcList2 = NULL;
static AtResponseList* pendedVsimUrcList3 = NULL;
static AtResponseList* pendedVsimUrcList4 = NULL;

void cacheVsimUrc(int unsolResponse, const void *data, size_t datalen, RIL_SOCKET_ID socket_id) {
    //Only the URC list we wanted.
    if (unsolResponse != RIL_UNSOL_VSIM_OPERATION_INDICATION) {
        RLOGI("Don't need to cache the request");
        return;
    }
    AtResponseList* urcCur = NULL;
    AtResponseList* urcPrev = NULL;
    int pendedUrcCount = 0;

    switch(socket_id) {
        case RIL_SOCKET_1:
            urcCur = pendedVsimUrcList1;
            break;
        case RIL_SOCKET_2:
            urcCur = pendedVsimUrcList2;
            break;
        case RIL_SOCKET_3:
            urcCur = pendedVsimUrcList3;
            break;
        case RIL_SOCKET_4:
            urcCur = pendedVsimUrcList4;
            break;
        default:
            RLOGE("Socket id is wrong!!");
            return;
    }
    while (urcCur != NULL) {
        RLOGD("Pended Vsim URC:%d, RILD:%s, :%s",
            pendedUrcCount,
            rilSocketIdToString(socket_id),
            requestToString(urcCur->id));
        urcPrev = urcCur;
        urcCur = urcCur->pNext;
        pendedUrcCount++;
    }
    urcCur = (AtResponseList*)calloc(1, sizeof(AtResponseList));
    assert (urcCur != NULL);

    if (urcPrev != NULL)
        urcPrev->pNext = urcCur;
    urcCur->pNext = NULL;
    urcCur->id = unsolResponse;
    urcCur->datalen = datalen;
    urcCur->data = (char*)calloc(1, datalen + 1);
    assert (urcCur->data != NULL);
    urcCur->data[datalen] = 0x0;
    memcpy(urcCur->data, data, datalen);
    if (pendedUrcCount == 0) {
        switch(socket_id) {
            case RIL_SOCKET_1:
                pendedVsimUrcList1 = urcCur;
                break;
            case RIL_SOCKET_2:
                pendedVsimUrcList2 = urcCur;
                break;
            case RIL_SOCKET_3:
                pendedVsimUrcList3 = urcCur;
                break;
            case RIL_SOCKET_4:
                pendedVsimUrcList4 = urcCur;
                break;
            default:
                RLOGE("Socket id is wrong!!");
                return;
        }
    }
    RLOGD("Current pendedVsimUrcCount = %d", pendedUrcCount + 1);
}

static void sendVsimUrc(RIL_SOCKET_ID socket_id, AtResponseList* urcCached) {
    AtResponseList* urc = urcCached;
    AtResponseList* urc_temp;
    while (urc != NULL) {
        RLOGD("sendVsimPendedUrcs RIL%s, %s",
        rilSocketIdToString(socket_id),
        requestToString(urc->id));
        RIL_UNSOL_RESPONSE (urc->id, urc->data, urc->datalen, socket_id);
        free(urc->data);
        urc_temp = urc;
        urc = urc->pNext;
        free(urc_temp);
    }
}

void sendPendedVsimUrcs(RIL_SOCKET_ID socket_id, int fdCommand) {
    RLOGD("Ready to send pended Vsim URCs, socket:%s, fdCommand:%d", rilSocketIdToString(socket_id), fdCommand);
    if ((RIL_SOCKET_1 == socket_id) && (fdCommand != -1)) {
        sendVsimUrc(socket_id, pendedVsimUrcList1);
        pendedVsimUrcList1 = NULL;
    }
#if (SIM_COUNT >= 2)
    else if ((RIL_SOCKET_2 == socket_id) && (fdCommand != -1)) {
        sendVsimUrc(socket_id, pendedVsimUrcList2);
        pendedVsimUrcList2 = NULL;
    }
#endif
#if (SIM_COUNT >= 3)
    else if ((RIL_SOCKET_3 == socket_id) && (fdCommand != -1)) {
        sendVsimUrc(socket_id, pendedVsimUrcList3);
        pendedVsimUrcList3 = NULL;
    }
#endif
#if (SIM_COUNT >= 4)
    else if ((RIL_SOCKET_4 == socket_id) && (fdCommand != -1)) {
        sendVsimUrc(socket_id, pendedVsimUrcList4);
        pendedVsimUrcList4 = NULL;
    }
#endif
}
// External SIM [End]

void enqueueLocalRequestResponse(RequestInfo* pRI, void *buffer, size_t buflen, UserCallbackInfo* pUCI, RIL_SOCKET_ID socket_id) {
    RequestInfoProxy * proxy = NULL;
    int proxyId;
    const char* proxyName = NULL;
    int queueSize = 0;

    proxy = createLocalRequestInfoProxy(pRI, buffer, buflen, pUCI, &proxyId, socket_id);

    proxyName = ::proxyIdToString(proxyId);

    /* Save dispatched proxy in RequestInfo */
    queueSize =  enqueueProxyList(&s_Proxy[proxyId], proxy);

    if (0 != queueSize)
    {
        RLOGD("Request timed callback to %s is busy. total:%d requests pending",
                proxyName,queueSize+1);
    }
}

void enqueue(RequestInfo* pRI, void *buffer, size_t buflen, UserCallbackInfo* pUCI, RIL_SOCKET_ID socket_id) {
    RequestInfoProxy * proxy = NULL;
    int proxyId = 0;
    const char* proxyName = NULL;
    int queueSize = 0;

    proxy = createRequestInfoProxy(pRI, buffer, buflen, pUCI, &proxyId, socket_id);

    proxyName = ::proxyIdToString(proxyId);

    /* Save dispatched proxy in RequestInfo */
    queueSize =  enqueueProxyList(&s_Proxy[proxyId], proxy);

    if (0 != queueSize)
    {
        RLOGD("Request timed callback to %s is busy. total:%d requests pending",
                proxyName,queueSize+1);
    }
}

static android::RequestInfoProxy* createLocalRequestInfoProxy(RequestInfo* pRI, void *buffer, size_t buflen, UserCallbackInfo* pUCI, int* pId, RIL_SOCKET_ID socket_id) {
    RequestInfoProxy * pRequest = NULL;
    status_t status;
    int32_t request;
    int32_t token;
    int proxyId = -1;
    int oldProxyId = -1;

    pRequest = (RequestInfoProxy *)calloc(1, sizeof(RequestInfoProxy));
    assert(pRequest);

    pRequest->p = new Parcel();

    pRequest->p_next = NULL;

    pRequest->pRI = pRI;

    pRequest->pUCI = NULL;

    request = pRI->pCI->requestNumber;
    if (request == RIL_LOCAL_REQUEST_QUERY_MODEM_THERMAL) {
        const char**    strings = (const char**)buffer;
        writeStringToParcel(*(pRequest->p), strings[0]);
        pRequest->p->setDataPosition(0);
    } else if (request == RIL_LOCAL_REQUEST_SET_MODEM_THERMAL) {
        int param = *((int*)buffer);
        // LOGI("check 2: (%d, %d)", buflen, param);
        pRequest->p->writeInt32(buflen);
        pRequest->p->writeInt32(param);
        pRequest->p->setDataPosition(0);
    } else if (request == RIL_LOCAL_REQUEST_UPDATE_SIM_LOCK_SETTINGS
            || request == RIL_LOCAL_REQUEST_UPDATE_SUBLOCK_SETTINGS) {
        const char**    strings = (const char**)buffer;
        writeStringToParcel(*(pRequest->p), strings[0]);
        pRequest->p->setDataPosition(0);
    } else {
         int32_t token;
         pRequest->p->writeInt32(request);
         // Local request for response will be handle specially,
         // so set the token id as 0xFFFFFFFF
         pRequest->p->writeInt32(0xffffffff);
         pRequest->p->write(buffer, buflen);

         status = pRequest->p->readInt32(&request);
         status = pRequest->p->readInt32 (&token);
         assert(status == NOERROR);
    }
    proxyId = pRI->pCI->proxyId;
    oldProxyId = proxyId;

    /* Shift proxyId if needed */
    if (socket_id == RIL_SOCKET_2 /*&& request != RIL_REQUEST_DUAL_SIM_MODE_SWITCH*/) {
        proxyId = proxyId + RIL_CHANNEL_OFFSET;
        /* Update */
    } else if (socket_id == RIL_SOCKET_3) {
        proxyId = proxyId + RIL_CHANNEL_SET3_OFFSET;
    } else if (socket_id == RIL_SOCKET_4) {
        proxyId = proxyId + RIL_CHANNEL_SET4_OFFSET;
    }

    /* Save dispatched proxy in RequestInfo */
    pRequest->pRI->cid = (RILChannelId) proxyId;
#ifdef MTK_RIL
    // special handle for request radio power due to modem's limitation
    // only can send ETHERMAL to protocol 1
    if(request == RIL_LOCAL_REQUEST_QUERY_MODEM_THERMAL) {
        char prop_value[PROPERTY_VALUE_MAX] = { 0 };
        int targetSim = 0;
        property_get(PROPERTY_3G_SIM, prop_value, "1");
        targetSim = atoi(prop_value) - 1;
        if (targetSim == 0) proxyId = oldProxyId;
        else if (targetSim == 1) proxyId = oldProxyId + RIL_CHANNEL_OFFSET;
        else if (targetSim == 2) proxyId = oldProxyId + RIL_CHANNEL_SET3_OFFSET;
        else if (targetSim == 3) proxyId = oldProxyId + RIL_CHANNEL_SET4_OFFSET;
        RLOGI("set request %d to protocal 1, proxyId=%d", request, proxyId);
    }
#endif
    pRequest->pRI->exec_cid = (RILChannelId) proxyId;
    (*pId) = proxyId;
    return pRequest;
}

int isMainProtocolCommandWithOemHookRaw(int request, void *buffer, size_t buflen) {
    int ret = 0;
    if (RIL_REQUEST_OEM_HOOK_RAW != request || buflen == 0) {
        return ret;
    }

    if (strstr((char *) buffer, "AT+EFUN") != NULL) {
        RLOGE("isMainProtocolCommandWithOemHookRaw(): %s", (char *) buffer);
        ret = 1;
    }
    return ret;
}

static android::RequestInfoProxy* createRequestInfoProxy(RequestInfo* pRI, void *buffer, size_t buflen, UserCallbackInfo* pUCI, int* pId, RIL_SOCKET_ID socket_id) {
    RequestInfoProxy * pRequest = NULL;
    status_t status;
    int32_t request = 0;
    int32_t token = 0;
    int proxyId = -1;
    int i = 0;
    int oldProxyId = -1;

    pRequest = (RequestInfoProxy *)calloc(1, sizeof(RequestInfoProxy));
    assert(pRequest);

    /// M: Fix memory leak issue. userCallback no need new parcel  @{
    if (pRI != NULL) {
        pRequest->p = new Parcel();
    }
    /// @}

    pRequest->p_next = NULL;

    pRequest->pRI = pRI;

    pRequest->pUCI = pUCI;

    if (buffer != NULL) {
        pRequest->p->setData((uint8_t *) buffer, buflen);

        status = pRequest->p->readInt32(&request);
        status = pRequest->p->readInt32(&token);
        assert(status == NOERROR);
        //RLOGD("create request %d", request);
        // already check the request validation in processCommandBuffer
        if (request < RIL_REQUEST_VENDOR_BASE) {
            for (i = 0; i < s_commands_size; i++) {
                if (request == s_commands[i].requestNumber) {
                    proxyId = s_commands[i].proxyId;
                    break;
                }
            }
            if (i == s_commands_size) {
                RLOGE("Invalid request id, id=%d", request);
                free(pRequest);
                return 0;
            }
        } else if (request >= RIL_REQUEST_VENDOR_BASE &&
                    request < RIL_LOCAL_GSM_REQUEST_VENDOR_BASE) {
            for (i = 0; i < s_mtk_commands_size; i++) {
                if (request == s_mtk_commands[i].requestNumber) {
                    proxyId = s_mtk_commands[i].proxyId;
                    break;
                }
            }
            if (i == s_mtk_commands_size) {
                RLOGE("Invalid request id, id=%d", request);
                free(pRequest);
                return 0;
            }
        } else if (request >= RIL_LOCAL_GSM_REQUEST_VENDOR_BASE) {
            for (i = 0; i < s_mtk_local_commands_size; i++) {
                if (request == s_mtk_local_commands[i].requestNumber) {
                    proxyId = s_mtk_local_commands[i].proxyId;
                    break;
                }
            }
            if (i == s_mtk_local_commands_size) {
                RLOGE("Invalid request id, id=%d", request);
                free(pRequest);
                return 0;
            }
        }
        oldProxyId = proxyId;

        /* Shift proxyId if needed */
        if (socket_id == RIL_SOCKET_2 /*&& request != RIL_REQUEST_DUAL_SIM_MODE_SWITCH*/) {
            proxyId = proxyId + RIL_CHANNEL_OFFSET;
            /* Update */
        } else if (socket_id == RIL_SOCKET_3) {
            proxyId = proxyId + RIL_CHANNEL_SET3_OFFSET;
        } else if (socket_id == RIL_SOCKET_4) {
            proxyId = proxyId + RIL_CHANNEL_SET4_OFFSET;
        }

        /* Save dispatched proxy in RequestInfo */
        pRequest->pRI->cid = (RILChannelId) proxyId;

#ifdef MTK_RIL
        // special handle for request radio power due to modem's limitation
        // only can send EFUN/CFUN to protocol 1
        if(request == RIL_REQUEST_RADIO_POWER
                || request == RIL_REQUEST_MODEM_POWEROFF
                || request == RIL_REQUEST_MODEM_POWERON
                || request == RIL_REQUEST_RESET_RADIO
                || request == RIL_REQUEST_SET_RADIO_CAPABILITY
                || request == RIL_LOCAL_REQUEST_QUERY_MODEM_THERMAL
                || (request == RIL_REQUEST_CONFIG_MODEM_STATUS && !isSvlteSupport())
                || request == RIL_REQUEST_ENTER_RESTRICT_MODEM
                || request == RIL_REQUEST_LEAVE_RESTRICT_MODEM
                || request == RIL_REQUEST_GET_ACTIVITY_INFO
                || request == RIL_LOCAL_REQUEST_SWITCH_ANTENNA
                /// M: Mode switch TRM feature. @{
                || request == RIL_LOCAL_REQUEST_MODE_SWITCH_GSM_SET_TRM
                || request == RIL_REQUEST_ENABLE_MODEM
                || isMainProtocolCommandWithOemHookRaw(request, buffer, buflen)) {
                /// @}
            char prop_value[PROPERTY_VALUE_MAX] = { 0 };
            int targetSim = 0;
            property_get(PROPERTY_3G_SIM, prop_value, "1");
            targetSim = atoi(prop_value) - 1;
            if (targetSim == 0) proxyId = oldProxyId;
            else if (targetSim == 1) proxyId = oldProxyId + RIL_CHANNEL_OFFSET;
            else if (targetSim == 2) proxyId = oldProxyId + RIL_CHANNEL_SET3_OFFSET;
            else if (targetSim == 3) proxyId = oldProxyId + RIL_CHANNEL_SET4_OFFSET;
            RLOGI("set request %d to protocal 1, proxyId=%d", request, proxyId);
        } else if (request == RIL_LOCAL_REQUEST_AT_COMMAND_WITH_PROXY) {
            int parcelProxyId = -1;
            status = pRequest->p->readInt32(&parcelProxyId);
            assert(status == NOERROR);
            if (parcelProxyId < 0) {
                parcelProxyId = oldProxyId;
            } else if (parcelProxyId >= RIL_CHANNEL_OFFSET) {
                parcelProxyId = oldProxyId;
            }

            // according to different feature to choose different comparsion target
            int targetSim;
            if (isMultiPsAttachSupport()) {
                targetSim = socket_id;
            } else {
                targetSim = RIL_get3GSIM() - 1;
            }

            /* Shift parcelProxyId if needed */
            proxyId = parcelProxyId;
            if (targetSim == RIL_SOCKET_2) {
                proxyId = parcelProxyId + RIL_CHANNEL_OFFSET;
            } else if (targetSim == RIL_SOCKET_3) {
                proxyId = parcelProxyId + RIL_CHANNEL_SET3_OFFSET;
            } else if (targetSim == RIL_SOCKET_4) {
                proxyId = parcelProxyId + RIL_CHANNEL_SET4_OFFSET;
            }
            pRequest->pRI->cid = (RILChannelId) proxyId;
            if (proxyId != oldProxyId) {
                RLOGI("RIL_LOCAL_REQUEST_AT_COMMAND_WITH_PROXY send to %s, parcelProxyId: %d, \
                        targetSim id: %d", ::proxyIdToString(proxyId), parcelProxyId, targetSim);
            }
        }
#endif // MTK_RIL
    }

    if (pUCI != NULL) {
        /* This is from user callback */
        proxyId = pUCI->cid;
        RLOGD("There is an usercallback request created for %s", ::proxyIdToString(proxyId));
    }
    if (pRequest->pRI != NULL) {
        pRequest->pRI->exec_cid = (RILChannelId) proxyId;
    }
    (*pId) = proxyId;

    return pRequest;
}

static int enqueueProxyList(android::RequestInfoProxy ** pProxyList, android::RequestInfoProxy *pRequest) {
    android::RequestInfoProxy ** ppCur = pProxyList;
    int i = 0;
    int j = 0;
    int ret;
    time_t currentTime;
    double seconds;

    pRequest->p_next = NULL;

    /* Multiple channel support */
    ret = pthread_mutex_lock(&s_dispatchMutex);
    assert(ret == 0);

    if (*ppCur == NULL) /* No pending task */
    {
        *ppCur = pRequest;
        if (pRequest->pRI != NULL)
        {
            RLOGD("%s pRI=%p p=%p dispatched to %s", android::requestToString(pRequest->pRI->pCI->requestNumber),
                pRequest->pRI,
                pRequest->p,
                ::proxyIdToString(pRequest->pRI->exec_cid));
        }
        else if (pRequest->pUCI != NULL)
        {
            RLOGD("Timed callback pUCI=%p dispatched to %s",pRequest->pUCI,::proxyIdToString(pRequest->pUCI->cid));
        }
        pthread_cond_broadcast(&s_dispatchCond);
    } else {
        char *out_str = (char*)calloc(1, 1024);
        assert(out_str != NULL);
        char *one_str = (char*)calloc(1, 256);
        assert(one_str != NULL);
        int out_str_len = 0;
        for ( ; *ppCur != NULL; ppCur = &((*ppCur)->p_next)) {
            out_str_len = strlen(out_str);
            /* Travel to the last one */
            if(i < 10) {  // only print 10 requests
                if ((*ppCur)->pRI != NULL) {
                    snprintf(one_str, 256,
                            (out_str_len > 0 ?
                                    ",[%d:%s pRI=%p p=%p is queued in %s]" :
                                    "[%d:%s pRI=%p p=%p is queued in %s]"), ++i,
                            android::requestToString((*ppCur)->pRI->pCI->requestNumber),
                            (*ppCur)->pRI, (*ppCur)->p, ::proxyIdToString((*ppCur)->pRI->exec_cid));
                    strncat(out_str, one_str, (1024 - out_str_len - 1));
                    memset(one_str, 0, 256);
                } else if ((*ppCur)->pUCI != NULL) {
                    snprintf(one_str, 256,
                            (out_str_len > 0 ?
                                    ",[%d:Timed callback pUCI=%p is queued in  %s]" :
                                    "[%d:Timed callback pUCI=%p is queued in  %s]"), ++i,
                            (*ppCur)->pUCI, ::proxyIdToString((*ppCur)->pUCI->cid));
                    strncat(out_str, one_str, (1024 - out_str_len - 1));
                    memset(one_str, 0, 256);
                }
            } else {
                i++;
            }
        }
        if (strlen(out_str) > 0) {
            if (isUserLoad() == 1) {
                RLOGD("%s", out_str);
            } else {
                /* Only dump requests queue for every 30 seconds or greater */
                time(&currentTime);
                seconds = difftime(currentTime, dumpRequestQueueTimeStamp);
                if (seconds >= 30) {
                    dumpRequestQueueTimeStamp = currentTime;
                    RLOGI("%s", out_str);
                }
            }
        }
        free(out_str);
        out_str = NULL;
        free(one_str);
        one_str = NULL;

        if (pRequest->pRI != NULL)
        {
            j = pRequest->pRI->exec_cid;
        }
        else if (pRequest->pUCI != NULL)
        {
            j = pRequest->pUCI->cid;
        }
        RLOGD("**** the last executed cmd for %s is %s ****", ::proxyIdToString(j), s_LastExecCmd[j]);
        *ppCur = pRequest;
    }

    ret = pthread_mutex_unlock(&s_dispatchMutex);
    assert(ret == 0);

    return i; /* how many requests in queue */
}

/*
 * This function should be encompassed by mutex s_dispatchMutex
 * to protect pProxyList manipulation (Only called by proxyLoop)
 */
android::RequestInfoProxy * dequeueProxyList(android::RequestInfoProxy ** pProxyList)
{
    android::RequestInfoProxy * pCur = *pProxyList;
    int ret;

    if (pCur != NULL)
    {
        /* Move mutex protection outside to protect atomic query */

//        ret = pthread_mutex_lock(&s_dispatchMutex);
//        assert(ret == 0);

        *pProxyList =  pCur->p_next;

//        ret = pthread_mutex_unlock(&s_dispatchMutex);
//        assert(ret == 0);
    }

    return pCur;
}

void *proxyLoop(void *param)
{
    int ret;
    android::RequestInfoProxy *proxy = NULL;
    int i; /* proxyid */
    const char * proxyName = NULL;
    bool hasHandleWork = false;

    /* Who am I */
    for (i=0; i<getRilProxysNum(); i++)
    {
        if (&s_Proxy[i] == (android::RequestInfoProxy **)param)
            break;
    }

    assert(i<getRilProxysNum());
    proxyName = ::proxyIdToString(i);

    do
    {
        /* Self check */
        assert(pthread_equal(s_tid_proxy[i], pthread_self()));

        while((ret = pthread_mutex_trylock(&proxyMutex[i])) != 0){
            RLOGD("proxyLoop is retrying to lock proxyMutex[%d], ret=%d", i, ret);
            usleep(200 * 1000);
        }
        /* Obtain dispatchMutex for proxyList here */
        ret = pthread_mutex_lock(&s_dispatchMutex);
        assert(ret == 0);

        proxy = dequeueProxyList(&s_Proxy[i]);


        if (proxy != NULL)
        {
            if (!hasHandleWork) {
                RLOGD("%s wakeup", proxyName);
                hasHandleWork = true;
            }

            /* Release dispatchMutex for next blocking call.
             * We don't want the dispatcher to wait full command execution. */
            ret = pthread_mutex_unlock(&s_dispatchMutex);
            assert(ret == 0);

            if (proxy->pRI != NULL)
            {
                RLOGI("proxy id:%d, proxyName: %s, %s pRI=%p p=%p execute on %s using channel %d",
                        i, proxyName, android::requestToString(proxy->pRI->pCI->requestNumber),
                        proxy->pRI, proxy->p, proxyName, proxy->pRI->exec_cid);
                int cmdLen = strlen(android::requestToString(proxy->pRI->pCI->requestNumber));
                memset(s_LastExecCmd[i], 0, MAX_UUID_LENGTH);
                strncpy(s_LastExecCmd[i], android::requestToString(proxy->pRI->pCI->requestNumber), MIN(cmdLen, MAX_UUID_LENGTH-1));
                proxy->pRI->pCI->dispatchFunction(*proxy->p, proxy->pRI); /* Blocking call */
                if (isUserLoad() == 0) {
                    RLOGD("proxyLoop free  pRequest=%p pRI=%p p=%p ", proxy, proxy->pRI, proxy->p);
                }
                delete proxy->p;
            } else if (proxy->pUCI != NULL) {
                RLOGD("Timed callback pUCI=%p execute on %s using channel %d, tid:%lu",
                    proxy->pUCI,
                    proxyName,
                    proxy->pUCI->cid,
                    pthread_self());
                proxy->pUCI->p_callback(proxy->pUCI->userParam);
                if (isUserLoad() == 0) {
                    RLOGD("proxyLoop free  pRequest=%p pUCI=%p pUCI->event: %p", proxy, proxy->pUCI,
                            &(proxy->pUCI->event));
                }
                free(proxy->pUCI);
            } else {
                if (isUserLoad() == 0) {
                    RLOGD("proxyLoop free  pRequest=%p", proxy);
                }
            }
            free(proxy);
            ret = pthread_mutex_unlock(&proxyMutex[i]);
            assert(ret == 0);
        }
        else
        {
            /* No pending pRIs sleep */
            if (hasHandleWork) {
                RLOGD("%s sleep", proxyName);
                hasHandleWork = false;
            }
            //ret = pthread_mutex_lock(&s_dispatchMutex);
            //assert(ret == 0);
            ret = pthread_mutex_unlock(&proxyMutex[i]);
            assert(ret == 0);

            pthread_cond_wait(&s_dispatchCond, &s_dispatchMutex);

            ret = pthread_mutex_unlock(&s_dispatchMutex);
            assert(ret == 0);
        }


    } while(1);

    return NULL;
}

void RIL_startRILProxys(void)
{
    int ret;
    int i;
    for (i=0; i < getRilProxysNum(); i++)
    {
        pthread_attr_t attr;
        pthread_mutex_init(&proxyMutex[i], NULL);
        pthread_attr_init (&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        ret = pthread_create(&s_tid_proxy[i], &attr, proxyLoop,(void*) &s_Proxy[i]);


        if (ret < 0) {
            RLOGE("Failed to create proxy thread errno:%d", errno);
            assert(0);
            return;
        }

        RLOGD("%s is up, tid:%d",::proxyIdToString(i),(int) s_tid_proxy[i]);
    }

    for (int i = 0; i < getMdMaxSimCount(); i++) {
        s_fdCommand[i] = -1;
    }

    /* Initialize time stamp: dump requests queue for every 30 seconds or greater */
    time(&dumpRequestQueueTimeStamp);  // To get current time
}


static UserCallbackInfo *
internalRequestProxyTimedCallback (RIL_TimedCallback callback, void *param,
                              const struct timeval *relativeTime,
                              int proxyId)
{
    struct timeval myRelativeTime;
    UserCallbackInfo *p_info;

    p_info = (UserCallbackInfo *) calloc(1, sizeof(UserCallbackInfo));
    if (p_info == NULL) {
        RLOGE("OOM");
        return NULL;
    }
    p_info->p_callback = callback;
    p_info->userParam = param;
    p_info->cid = (RILChannelId)proxyId;

    if (relativeTime == NULL) {
        /* treat null parameter as a 0 relative time */
        memset (&myRelativeTime, 0, sizeof(myRelativeTime));
    } else {
        /* FIXME I think event_add's tv param is really const anyway */
        memcpy (&myRelativeTime, relativeTime, sizeof(myRelativeTime));
    }

    ril_event_set(&(p_info->event), -1, false, userTimerCallback, p_info);

    RLOGD("internalRequestProxyTimedCallback malloc p_info->event: %p", &(p_info->event));

    ril_timer_add(&(p_info->event), &myRelativeTime);

    triggerEvLoop();
    return p_info;
}


extern "C" void
RIL_requestProxyTimedCallback (RIL_TimedCallback callback, void *param,
                          const struct timeval *relativeTime,
                          RILChannelId proxyId) {
    internalRequestProxyTimedCallback (callback, param,
                relativeTime, proxyId);
}

extern "C" RILChannelId
RIL_queryMyChannelId(RIL_Token t)
{
    return ((RequestInfo *) t)->exec_cid;
}

extern "C" RILChannelId
RIL_queryCommandChannelId(RIL_Token t)
{
    return ((RequestInfo *) t)->cid;
}

extern "C" int
RIL_queryMyProxyIdByThread()
{
    pthread_t selfThread = pthread_self();
    pthread_mutex_lock(&s_queryThreadMutex);
    int i;
    for (i=0; i < getRilProxysNum(); i++)
    {
        if (0 != pthread_equal(s_tid_proxy[i], selfThread))
        {
            pthread_mutex_unlock(&s_queryThreadMutex);
            return i;
        }
    }
    pthread_mutex_unlock(&s_queryThreadMutex);
    return -1;
}

extern "C" void simSwitchClearProxyList() {
    RequestInfoProxy * proxy = NULL;
    RequestInfoProxy * temp_proxy_head = NULL;
    RequestInfoProxy * temp_proxy_tail = NULL;
    int ret, i;
    const char* proxyName = NULL;
    int32_t request;
    bool dss_no_reset = isDssNoResetSupport();

    ret = pthread_mutex_lock(&s_dispatchMutex);
    assert(ret == 0);

    for (i = 0; i < getRilProxysNum(); i++){
        proxyName = ::proxyIdToString(i);
        do {
            proxy = NULL;
            proxy = dequeueProxyList(&s_Proxy[i]);
            if (proxy != NULL) {
                if (proxy->pRI != NULL) {
                    request = proxy->pRI->pCI->requestNumber;
                    // Dynamic SIM switch white list [Start]
                    if (request == RIL_REQUEST_ALLOW_DATA ||
                            (dss_no_reset && (isSimRequest(request) ||
                                    isStkWhitelistRequest(request)))) {
                        LOGI("Do not return RIL_E_RADIO_NOT_AVAILABLE for %s",
                            android::requestToString(request));
                        // save callback to time_proxy for temp.
                        if (temp_proxy_head == NULL) {
                            temp_proxy_head = proxy;
                        } else {
                            temp_proxy_tail->p_next = proxy;
                        }
                        temp_proxy_tail = proxy;
                        temp_proxy_tail->p_next = NULL;
                    // Dynamic SIM switch white list [End]
                    } else {
                        RLOGD("remove request from proxy id:%d, proxyName: %s, request: %s",
                            i, proxyName, android::requestToString(proxy->pRI->pCI->requestNumber));
                        RIL_onRequestComplete(proxy->pRI, RIL_E_RADIO_NOT_AVAILABLE, NULL, 0);
                        free(proxy);
                    }
                } else if (proxy->pUCI != NULL) {
                    RLOGD("Find request from Timed callback pUCI=%p on %s using channel %d",
                        proxy->pUCI,
                        proxyName,
                        proxy->pUCI->cid);
                    // save callback to temp_proxy for temp.
                    if (temp_proxy_head == NULL) {
                        temp_proxy_head = proxy;
                    } else {
                        temp_proxy_tail->p_next = proxy;
                    }
                    temp_proxy_tail = proxy;
                    temp_proxy_tail->p_next = NULL;
                }
            }
        } while (proxy != NULL);
        // put callback back to s_Proxy[i] & clear the temp proxy
        s_Proxy[i] = temp_proxy_head;
        temp_proxy_head = NULL;
        temp_proxy_tail = NULL;
    }
    ret = pthread_mutex_unlock(&s_dispatchMutex);
    assert(ret == 0);
}

//sim request whitelist after sim switch, not for common use
bool isSimRequest(int32_t request) {
    if (request == RIL_REQUEST_GET_SIM_STATUS ||
        request == RIL_REQUEST_GET_IMSI ||
        request == RIL_REQUEST_SIM_IO ||
        request == RIL_REQUEST_ENTER_SIM_PIN ||
        request == RIL_REQUEST_ENTER_SIM_PUK ||
        request == RIL_REQUEST_ENTER_SIM_PIN2 ||
        request == RIL_REQUEST_ENTER_SIM_PUK2 ||
        request == RIL_REQUEST_CHANGE_SIM_PIN ||
        request == RIL_REQUEST_CHANGE_SIM_PIN2 ||
        request == RIL_REQUEST_ENTER_NETWORK_DEPERSONALIZATION ||
        request == RIL_REQUEST_QUERY_FACILITY_LOCK ||
        request == RIL_REQUEST_SET_FACILITY_LOCK ||
        request == RIL_REQUEST_QUERY_SIM_NETWORK_LOCK ||
        request == RIL_REQUEST_SET_SIM_NETWORK_LOCK ||
        request == RIL_REQUEST_SET_ALLOWED_CARRIERS ||
        request == RIL_REQUEST_GET_ALLOWED_CARRIERS ||
        request == RIL_REQUEST_SIM_AUTHENTICATION ||
        request == RIL_REQUEST_ISIM_AUTHENTICATION ||
        request == RIL_REQUEST_GENERAL_SIM_AUTH ||
        request == RIL_REQUEST_SIM_TRANSMIT_APDU_BASIC ||
        request == RIL_REQUEST_SIM_OPEN_CHANNEL ||
        request == RIL_REQUEST_SIM_CLOSE_CHANNEL ||
        request == RIL_REQUEST_SIM_TRANSMIT_APDU_CHANNEL ||
        request == RIL_REQUEST_SIM_GET_ATR ||
        request == RIL_LOCAL_REQUEST_GET_SHARED_KEY ||
        request == RIL_LOCAL_REQUEST_UPDATE_SIM_LOCK_SETTINGS ||
        request == RIL_LOCAL_REQUEST_GET_SIM_LOCK_INFO ||
        request == RIL_LOCAL_REQUEST_RESET_SIM_LOCK_SETTINGS ||
        request == RIL_LOCAL_REQUEST_GET_MODEM_STATUS ||
        request == RIL_REQUEST_SET_SIM_POWER ||
        request == RIL_REQUEST_CONFIG_MODEM_STATUS ||
        request == RIL_REQUEST_ENTER_DEVICE_NETWORK_DEPERSONALIZATION) {
        return true;
    } else {
        return false;
    }
}

// stk request whitelist after sim switch
bool isStkWhitelistRequest(int32_t request) {
    if (request == RIL_REQUEST_STK_SEND_ENVELOPE_COMMAND ||
        request == RIL_REQUEST_STK_SEND_ENVELOPE_WITH_STATUS ||
        request == RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE ||
        request == RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM ||
        request == RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM_WITH_RESULT_CODE ||
        request == RIL_REQUEST_REPORT_STK_SERVICE_IS_RUNNING ||
        request == RIL_REQUEST_SET_STK_UTK_MODE) {
        return true;
    } else {
        return false;
    }
}

}
