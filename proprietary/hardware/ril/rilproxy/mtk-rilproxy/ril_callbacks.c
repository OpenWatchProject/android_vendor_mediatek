/* //device/system/reference-ril/reference-ril.c
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#include <telephony/ril_cdma_sms.h>
#include "librilutils.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <alloca.h>
#include "atchannel.h"
#include "at_tok.h"
#include "misc.h"
#include <getopt.h>
#include <sys/socket.h>
#include <cutils/sockets.h>
#include <termios.h>
#include <sys/system_properties.h>
#include <vendor/mediatek/proprietary/hardware/ril/rilproxy/librilutils/proto/sap-api.pb.h>
#include "pb_decode.h"
#include "pb_encode.h"
#include <arpa/inet.h>

#include <telephony/mtk_ril.h>
#include "Rfx.h"
#include "socket_ext_api.h"

#undef LOG_TAG
#define LOG_TAG "RIL-RP"
#include <log/log.h>
#include <cutils/properties.h>

#ifdef HAVE_AEE_FEATURE
#include "aee.h"
#endif

static void *noopRemoveWarning( void *a ) { return a; }
#define RIL_UNUSED_PARM(a) noopRemoveWarning((void *)&(a));

#define MAX_AT_RESPONSE 0x1000

/* pathname returned from RIL_REQUEST_SETUP_DATA_CALL / RIL_REQUEST_SETUP_DEFAULT_PDP */
#define PPP_TTY_PATH "eth0"

// Default MTU value
#define DEFAULT_MTU 1500

#ifdef USE_TI_COMMANDS

// Enable a workaround
// 1) Make incoming call, do not answer
// 2) Hangup remote end
// Expected: call should disappear from CLCC line
// Actual: Call shows as "ACTIVE" before disappearing
#define WORKAROUND_ERRONEOUS_ANSWER 1

// Some varients of the TI stack do not support the +CGEV unsolicited
// response. However, they seem to send an unsolicited +CME ERROR: 150
#define WORKAROUND_FAKE_CGEV 1
#endif

/* Modem Technology bits */
#define MDM_GSM         0x01
#define MDM_WCDMA       0x02
#define MDM_CDMA        0x04
#define MDM_EVDO        0x08
#define MDM_LTE         0x10

typedef struct {
    int supportedTechs; // Bitmask of supported Modem Technology bits
    int currentTech;    // Technology the modem is currently using (in the format used by modem)
    int isMultimode;

    // Preferred mode bitmask. This is actually 4 byte-sized bitmasks with different priority values,
    // in which the byte number from LSB to MSB give the priority.
    //
    //          |MSB|   |   |LSB
    // value:   |00 |00 |00 |00
    // byte #:  |3  |2  |1  |0
    //
    // Higher byte order give higher priority. Thus, a value of 0x0000000f represents
    // a preferred mode of GSM, WCDMA, CDMA, and EvDo in which all are equally preferrable, whereas
    // 0x00000201 represents a mode with GSM and WCDMA, in which WCDMA is preferred over GSM
    int32_t preferredNetworkMode;
    int subscription_source;

} ModemInfo;

static ModemInfo *sMdmInfo;
// TECH returns the current technology in the format used by the modem.
// It can be used as an l-value
#define TECH(mdminfo)                 ((mdminfo)->currentTech)
// TECH_BIT returns the bitmask equivalent of the current tech
#define TECH_BIT(mdminfo)            (1 << ((mdminfo)->currentTech))
#define IS_MULTIMODE(mdminfo)         ((mdminfo)->isMultimode)
#define TECH_SUPPORTED(mdminfo, tech) ((mdminfo)->supportedTechs & (tech))
#define PREFERRED_NETWORK(mdminfo)    ((mdminfo)->preferredNetworkMode)
// CDMA Subscription Source
#define SSOURCE(mdminfo)              ((mdminfo)->subscription_source)

static int net2modem[] = {
    MDM_GSM | MDM_WCDMA,                                 // 0  - GSM / WCDMA Pref
    MDM_GSM,                                             // 1  - GSM only
    MDM_WCDMA,                                           // 2  - WCDMA only
    MDM_GSM | MDM_WCDMA,                                 // 3  - GSM / WCDMA Auto
    MDM_CDMA | MDM_EVDO,                                 // 4  - CDMA / EvDo Auto
    MDM_CDMA,                                            // 5  - CDMA only
    MDM_EVDO,                                            // 6  - EvDo only
    MDM_GSM | MDM_WCDMA | MDM_CDMA | MDM_EVDO,           // 7  - GSM/WCDMA, CDMA, EvDo
    MDM_LTE | MDM_CDMA | MDM_EVDO,                       // 8  - LTE, CDMA and EvDo
    MDM_LTE | MDM_GSM | MDM_WCDMA,                       // 9  - LTE, GSM/WCDMA
    MDM_LTE | MDM_CDMA | MDM_EVDO | MDM_GSM | MDM_WCDMA, // 10 - LTE, CDMA, EvDo, GSM/WCDMA
    MDM_LTE,                                             // 11 - LTE only
};

static int32_t net2pmask[] = {
    MDM_GSM | (MDM_WCDMA << 8),                          // 0  - GSM / WCDMA Pref
    MDM_GSM,                                             // 1  - GSM only
    MDM_WCDMA,                                           // 2  - WCDMA only
    MDM_GSM | MDM_WCDMA,                                 // 3  - GSM / WCDMA Auto
    MDM_CDMA | MDM_EVDO,                                 // 4  - CDMA / EvDo Auto
    MDM_CDMA,                                            // 5  - CDMA only
    MDM_EVDO,                                            // 6  - EvDo only
    MDM_GSM | MDM_WCDMA | MDM_CDMA | MDM_EVDO,           // 7  - GSM/WCDMA, CDMA, EvDo
    MDM_LTE | MDM_CDMA | MDM_EVDO,                       // 8  - LTE, CDMA and EvDo
    MDM_LTE | MDM_GSM | MDM_WCDMA,                       // 9  - LTE, GSM/WCDMA
    MDM_LTE | MDM_CDMA | MDM_EVDO | MDM_GSM | MDM_WCDMA, // 10 - LTE, CDMA, EvDo, GSM/WCDMA
    MDM_LTE,                                             // 11 - LTE only
};

static int is3gpp2(int radioTech) {
    switch (radioTech) {
        case RADIO_TECH_IS95A:
        case RADIO_TECH_IS95B:
        case RADIO_TECH_1xRTT:
        case RADIO_TECH_EVDO_0:
        case RADIO_TECH_EVDO_A:
        case RADIO_TECH_EVDO_B:
        case RADIO_TECH_EHRPD:
            return 1;
        default:
            return 0;
    }
}

typedef enum {
    SIM_ABSENT = 0,
    SIM_NOT_READY = 1,
    SIM_READY = 2, /* SIM_READY means the radio state is RADIO_STATE_SIM_READY */
    SIM_PIN = 3,
    SIM_PUK = 4,
    SIM_NETWORK_PERSONALIZATION = 5,
    RUIM_ABSENT = 6,
    RUIM_NOT_READY = 7,
    RUIM_READY = 8,
    RUIM_PIN = 9,
    RUIM_PUK = 10,
    RUIM_NETWORK_PERSONALIZATION = 11
} SIM_Status;

#if defined(ANDROID_MULTI_SIM)
static void
onRequest (int request, void *data, size_t datalen, RIL_Token t, RIL_SOCKET_ID socket_id);
#else
static void
onRequest (int request, void *data, size_t datalen, RIL_Token t);
#endif
static void updateConnectionState(RIL_SOCKET_ID socketId, int isConnected);
#if defined(ANDROID_MULTI_SIM)
static void onSapRequest(int request, void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID socket_id);
#else
static void onSapRequest(int request, void *data, size_t datalen, RIL_Token t);
#endif
static RIL_RadioState currentState(RIL_SOCKET_ID socket_id);
static int onSupports (int requestCode);
static void onCancel (RIL_Token t);
static const char *getVersion();

extern const char * requestToString(int request);
void registerSignal();

/*** Static Variables ***/
static const RIL_RadioFunctions s_callbacks = {
    RIL_VERSION,
    onRequest,
    currentState,
    onSupports,
    onCancel,
    getVersion,
    updateConnectionState
};

#ifdef RIL_SHLIB
static const struct RIL_Env *s_rilenv;

#define RIL_onRequestComplete(t, e, response, responselen) s_rilenv->OnRequestComplete(t,e, response, responselen)
#define RIL_onUnsolicitedResponse(a,b,c,d) s_rilenv->OnUnsolicitedResponse(a,b,c,d)
#if defined(ANDROID_MULTI_SIM)
#define RIL_onUnsolicitedResponse(a,b,c,d) s_rilenv->OnUnsolicitedResponse(a,b,c,d)
#else
#define RIL_onUnsolicitedResponse(a,b,c) s_rilenv->OnUnsolicitedResponse(a,b,c)
#endif

#define RIL_requestTimedCallback(a,b,c) s_rilenv->RequestTimedCallback(a,b,c)
#endif

const struct RIL_Env *s_rilsapenv;
static const RIL_RadioFunctions s_sapcallbacks = {
    RIL_VERSION,
    onSapRequest,
    currentState,
    onSupports,
    onCancel,
    getVersion,
    updateConnectionState
};

/// M: SAP
#define RIL_SAP_onRequestComplete(t, e, response, responselen) s_rilsapenv->OnRequestComplete(t, e \
        , response, responselen)
#if defined(ANDROID_MULTI_SIM)
#define RIL_SAP_onUnsolicitedResponse(unsolResponse, data, datalen, socket_id) \
        s_rilsapenv->OnUnsolicitedResponse(unsolResponse, data, datalen, socket_id)
#else
#define RIL_SAP_onUnsolicitedResponse(unsolResponse, data, datalen) \
        s_rilsapenv->OnUnsolicitedResponse(unsolResponse, data, datalen)
#endif
#define RIL_SAP_requestTimedCallback(a,b,c) s_rilsapenv->RequestTimedCallback(a,b,c)
#define RIL_SAP_requestProxyTimedCallback(a,b,c,d,e) \
        RLOGD("%s request timed callback %s to %s", __FUNCTION__, e, proxyIdToString((int)d)); \
        s_rilsapenv->RequestProxyTimedCallback(a,b,c,(int)d)
#define RIL_SAP_queryMyChannelId(a) s_rilsapenv->QueryMyChannelId(a)
#define RIL_SAP_queryMyProxyIdByThread  s_rilsapenv->QueryMyProxyIdByThread()
void sendBtSapResponseComplete(RIL_Token t, RIL_Errno ret, MsgId msgId, void *data);
RIL_RadioState getRadioState(RIL_SOCKET_ID rid);
typedef struct SapSocketRequest {
    int token;
    MsgHeader* curr;
    struct SapSocketRequest* p_next;
    RIL_SOCKET_ID socketId;
} SapSocketRequest;
static SapSocketRequest *sapPendingRequests = NULL;
static pthread_mutex_t sapPendingRequestsMutex = PTHREAD_MUTEX_INITIALIZER;
///

// Radio States for Multi-RIL Client
static RIL_RadioState sStates[] = {
    RADIO_STATE_UNAVAILABLE,
    RADIO_STATE_UNAVAILABLE,
    RADIO_STATE_UNAVAILABLE,
    RADIO_STATE_UNAVAILABLE
};

static pthread_mutex_t s_state_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t s_state_cond = PTHREAD_COND_INITIALIZER;

static int s_port = -1;
static const char * s_device_path = NULL;
static int          s_device_socket = 0;

/* trigger change to this with s_state_cond */
static int s_closed = 0;

static int sFD;     /* file desc of AT channel */
static char sATBuffer[MAX_AT_RESPONSE+1];
static char *sATBufferCur = NULL;

static const struct timeval TIMEVAL_SIMPOLL = {1,0};
static const struct timeval TIMEVAL_CALLSTATEPOLL = {0,500000};
static const struct timeval TIMEVAL_0 = {0,0};

void setRadioState(RIL_RadioState newState, RIL_SOCKET_ID rid);
void decodeAndSendSapMessage(void *data, size_t datalen, RIL_SOCKET_ID socketId);
int toRealSlot(int slotId);

/*** Callback methods from the RIL library to us ***/

/**
 * Call from RIL to us to make a RIL_REQUEST
 *
 * Must be completed with a call to RIL_onRequestComplete()
 *
 * RIL_onRequestComplete() may be called from any thread, before or after
 * this function returns.
 *
 * Will always be called from the same thread, so returning here implies
 * that the radio is ready to process another command (whether or not
 * the previous command has completed).
 */
#if defined(ANDROID_MULTI_SIM)
static void
onRequest (int request, void *data, size_t datalen, RIL_Token t, RIL_SOCKET_ID socket_id)
#else
static void
onRequest (int request, void *data, size_t datalen, RIL_Token t)
#endif
{
    ATResponse *p_response;
    int err;
    RIL_RadioState radioState = sStates[0];
#if !defined(ANDROID_MULTI_SIM)
    RIL_SOCKET_ID socket_id = RIL_SOCKET_1;
#endif

    radioState = sStates[toRealSlot(socket_id)];

    /* Ignore all requests when RADIO_STATE_UNAVAILABLE.
     */
    if (radioState == RADIO_STATE_UNAVAILABLE &&
            request != RIL_REQUEST_OEM_HOOK_ATCI_INTERNAL &&
            request != RIL_REQUEST_SET_TRM &&
            request != RIL_REQUEST_ALLOW_DATA &&
            request != RIL_REQUEST_SET_PS_REGISTRATION &&
            request != RIL_REQUEST_SET_DATA_PROFILE &&
            request != RIL_REQUEST_RESET_RADIO &&
            request != RIL_REQUEST_MODEM_POWERON &&
            request != RIL_REQUEST_MODEM_POWEROFF &&
            request != RIL_REQUEST_MODIFY_MODEM_TYPE &&
            // External SIM [Start]
            request != RIL_REQUEST_VSIM_NOTIFICATION &&
            request != RIL_REQUEST_VSIM_OPERATION &&
            // External SIM [End]
            // M: Data Framework - common part enhancement
            request != RIL_REQUEST_SYNC_DATA_SETTINGS_TO_MD &&
            request != RIL_REQUEST_GET_RADIO_CAPABILITY &&
            request != RIL_REQUEST_SET_RADIO_CAPABILITY &&
            request != RIL_REQUEST_RESTART_RILD) {
        RLOGD("[RilProxy] onRequest: %s, datalen = %d, radioState:%d, socket_id: %d",
                requestToString(request), (int) datalen, radioState, socket_id);
        RIL_onRequestComplete(t, RIL_E_RADIO_NOT_AVAILABLE, NULL, 0);
        return;
    }

    /// M: ril proxy
    /* Log too much: Can check RfxRoot : processMessage() for request message.
    RLOGD("[RilProxy] onRequest: request = %d, datalen = %d, radioState:%d, slotId = %d",
           request, (int) datalen, radioState, socket_id);
    */
    rfx_enqueue_request_message(request, data, datalen, t, toRealSlot(socket_id));
    /// M
}

/// M: SAP
void encodeAndSendSapMessage(void *data, RIL_SOCKET_ID socketId) {
    SapSocketRequest *recv = (SapSocketRequest*) data;
    MsgHeader  *reqHeader = recv->curr;
    pb_ostream_t ostream;
    size_t encoded_size = 0;
    uint32_t written_size;
    size_t buffer_size = 0;
    bool success = false;
    SapSocketRequest **pendingRequests = &sapPendingRequests;

    if ((success = pb_get_encoded_size(&encoded_size, MsgHeader_fields,
        reqHeader)) && encoded_size <= INT32_MAX) {
        buffer_size = encoded_size + sizeof(uint32_t);
        uint8_t buffer[buffer_size];
        written_size = htonl((uint32_t) encoded_size);
        ostream = pb_ostream_from_buffer(buffer, buffer_size);
        pb_write(&ostream, (uint8_t *)&written_size, sizeof(written_size));
        success = pb_encode(&ostream, MsgHeader_fields, reqHeader);

        if (success) {
            RLOGD("encodeAndSendSapMessage: Size: %d (0x%x) Size as written: 0x%x",
                    (int) encoded_size, (int) encoded_size, (int) written_size);
            RLOGI("encodeAndSendSapMessage: [%d] > SAP RESPONSE type: %d. id: %d. error: %d",
                    reqHeader->token, reqHeader->type, reqHeader->id, reqHeader->error);

            // enqueue request
            pthread_mutex_lock(&sapPendingRequestsMutex);
            recv->p_next = *pendingRequests;
            *pendingRequests = recv;
            pthread_mutex_unlock(&sapPendingRequestsMutex);
            //RLOGD("encodeAndSendSapMessage add token = %d to queue", reqHeader->token);
            rfx_sap_to_rild_socket(buffer, buffer_size, socketId);
        } else {
            RLOGE("encodeAndSendSapMessage: Error while encoding response of type %d id %d \
                    buffer_size: %d: %s.", reqHeader->type, reqHeader->id, (int) buffer_size,
                    PB_GET_ERROR(&ostream));
        }
    }else {
        RLOGE("Not sending response type %d: encoded_size: %u. encoded size result: %d",
        reqHeader->type, (int) encoded_size, success);
    }

}

SapSocketRequest* checkAndDequeueRequestInfo(int token) {
    SapSocketRequest **ppCur = NULL;
    SapSocketRequest *tmp = NULL;

    pthread_mutex_lock(&sapPendingRequestsMutex);
    for(ppCur = &sapPendingRequests; *ppCur != NULL; ppCur = &((*ppCur)->p_next)) {
        // RLOGD("checkAndDequeueRequestInfo searching...token = %d",
        //        (*ppCur)->curr->token);
        if (token == (int)((*ppCur)->curr->token)) {
            tmp = *ppCur;
            *ppCur = (*ppCur)->p_next;
            //RLOGD("checkAndDequeueRequestInfo get!");
            break;
        }
    }
    pthread_mutex_unlock(&sapPendingRequestsMutex);

    return tmp;
}

void decodeAndSendSapMessage(void *data, size_t datalen,
        RIL_SOCKET_ID socketId) {
    int ret;
    MsgHeader  *header;
    pb_istream_t stream;

    stream = pb_istream_from_buffer((uint8_t *)data, datalen);
    header = (MsgHeader *)malloc(sizeof (MsgHeader));
    if (header == NULL) {
        RLOGE("OOM");
        return;
    }
    memset(header, 0, sizeof(MsgHeader));

    if (!pb_decode(&stream, MsgHeader_fields, header) ) {
        RLOGE("Error decoding protobuf buffer : %s", PB_GET_ERROR(&stream));
    }

    RLOGD("decodeAndSendSapMessage id = %d", header->id);
    if (MsgType_UNSOL_RESPONSE == header->type) { // URC
    #if defined(ANDROID_MULTI_SIM)
        RIL_SAP_onUnsolicitedResponse(header->id, header->payload->bytes, header->payload->size,
            socketId);
    #else
        RIL_SAP_onUnsolicitedResponse(header->id, header->payload->bytes, header->payload->size);
    #endif
    } else {
        //RLOGD("decodeAndSendSapMessage search token = %d", header->token);
        SapSocketRequest *req =  checkAndDequeueRequestInfo(header->token);
        RIL_SAP_onRequestComplete(req, (RIL_Errno) header->error, header->payload->bytes,
                header->payload->size);
        //free(req);
    }
    //free(header);
}
///

static void updateConnectionState(RIL_SOCKET_ID socketId, int isConnected) {
    rfx_update_connection_state(socketId, isConnected);
}

#if defined(ANDROID_MULTI_SIM)
static void onSapRequest(int request, void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID socket_id) {
#else
static void onSapRequest(int request, void *data, size_t datalen, RIL_Token t) {
#endif
    RLOGD("onSapRequest: %d", request);
    RIL_UNUSED_PARM(data);
    RIL_UNUSED_PARM(datalen);
    int socketId = RIL_SOCKET_1;
    #if defined(ANDROID_MULTI_SIM)
    socketId = socket_id;
    #endif

    if (request < MsgId_RIL_SIM_SAP_CONNECT /* MsgId_UNKNOWN_REQ */ ||
            request > MsgId_RIL_SIM_SAP_SET_TRANSFER_PROTOCOL) {
        RLOGD("invalid request");
        RIL_SIM_SAP_ERROR_RSP rsp;
        rsp.dummy_field = 1;
        sendBtSapResponseComplete(t, (RIL_Errno) Error_RIL_E_REQUEST_NOT_SUPPORTED,
                                      MsgId_RIL_SIM_SAP_ERROR_RESP, &rsp);
        return;
    }

    if (RADIO_STATE_UNAVAILABLE == getRadioState(RIL_SOCKET_1)) {
        RLOGD("RADIO_STATE_UNAVAILABLE and reply failure to Sap message");
        RIL_SIM_SAP_CONNECT_RSP rsp;
        rsp.response = RIL_SIM_SAP_CONNECT_RSP_Response_RIL_E_SAP_CONNECT_FAILURE;
        rsp.has_max_message_size = false;
        rsp.max_message_size = 0;
        sendBtSapResponseComplete(t, (RIL_Errno) Error_RIL_E_RADIO_NOT_AVAILABLE,
                                      MsgId_RIL_SIM_SAP_CONNECT, &rsp);
        return;
    }

    encodeAndSendSapMessage(t, socketId);

}

RIL_RadioState getRadioState(RIL_SOCKET_ID rid)
{
    RIL_RadioState radioState = sStates[toRealSlot(rid)];
    RLOGI("getRadioState(): radioState=%d\n", radioState);
    return radioState;
}

/**
 * Synchronous call from the RIL to us to return current radio state.
 * RADIO_STATE_UNAVAILABLE should be the initial state.
 */
static RIL_RadioState
currentState(RIL_SOCKET_ID socket_id)
{
    return getRadioState(socket_id);
}
/**
 * Call from RIL to us to find out whether a specific request code
 * is supported by this implementation.
 *
 * Return 1 for "supported" and 0 for "unsupported"
 */

static int
onSupports (int requestCode __unused)
{
    //@@@ todo

    return 1;
}

static void onCancel (RIL_Token t __unused)
{
    //@@@todo

}

static const char * getVersion(void)
{
    return "android reference-ril 1.0";
}

void
setRadioState(RIL_RadioState newState, RIL_SOCKET_ID rid)
{
    RLOGD("setRadioState(%d), newState:%d", rid, newState);
    RIL_RadioState oldState;
    RIL_RadioState *pState = NULL;

    pthread_mutex_lock(&s_state_mutex);
    oldState = sStates[toRealSlot(rid)];
    pState = &(sStates[toRealSlot(rid)]);

    if (s_closed > 0) {
        // If we're closed, the only reasonable state is
        // RADIO_STATE_UNAVAILABLE
        // This is here because things on the main thread
        // may attempt to change the radio state after the closed
        // event happened in another thread
        newState = RADIO_STATE_UNAVAILABLE;
    }

    if (*pState != newState || s_closed > 0) {
        *pState = newState;

        pthread_cond_broadcast (&s_state_cond);
    }

    pthread_mutex_unlock(&s_state_mutex);


    /* do these outside of the mutex */
    if (*pState != oldState) {

#if defined(ANDROID_MULTI_SIM)
        RIL_onUnsolicitedResponse (RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED,
                                    NULL, 0, rid);
        // Sim state can change as result of radio state change
        RIL_onUnsolicitedResponse (RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED,
                                    NULL, 0, rid);
#else
        RIL_onUnsolicitedResponse (RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED,
                                    NULL, 0);
        // Sim state can change as result of radio state change
        RIL_onUnsolicitedResponse (RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED,
                                    NULL, 0);
#endif
    }
}

static void usage(char *s __unused)
{
#ifdef RIL_SHLIB
    fprintf(stderr, "reference-ril requires: -p <tcp port> or -d /dev/tty_device\n");
#else
    fprintf(stderr, "usage: %s [-p <tcp port>] [-d /dev/tty_device]\n", s);
    exit(-1);
#endif
}

extern void resetWakelock(void);

static void *
mainLoop(void *param __unused)
{
    int fd;
    int ret;
    AT_DUMP("== ", "entering mainLoop()", -1 );

    registerSignal();
    RLOGI("Finish registerSignal");

    resetWakelock();
    rfx_init();

    ril_socket_init();
    RLOGI("Finish ril socket initialization");

    return 0;
}

#ifdef RIL_SHLIB

pthread_t s_tid_mainloop;

const RIL_RadioFunctions *RIL_Init(const struct RIL_Env *env,
        int argc __unused, char **argv __unused)
{
    int ret;
    pthread_attr_t attr;

    s_rilenv = env;
    sMdmInfo = calloc(1, sizeof(ModemInfo));
    if (!sMdmInfo) {
        RLOGE("Unable to alloc memory for ModemInfo");
        return NULL;
    }
    pthread_attr_init (&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    ret = pthread_create(&s_tid_mainloop, &attr, mainLoop, NULL);
    if (ret != 0) {
        RLOGE("Failed to create mainLoop thread: %s", strerror(ret));
    }

    return &s_callbacks;
}

const RIL_RadioFunctions *RIL_SAP_Init(const struct RIL_Env *env, int argc,
        char **argv) {
    RLOGD("RIL_SAP_Init");
    RIL_UNUSED_PARM(argc);
    RIL_UNUSED_PARM(argv);
    s_rilsapenv = env;
    return &s_sapcallbacks;
}
#else /* RIL_SHLIB */
int main (int argc, char **argv)
{
    int ret;
    int fd = -1;
    int opt;

    while ( -1 != (opt = getopt(argc, argv, "p:d:"))) {
        switch (opt) {
            case 'p':
                s_port = atoi(optarg);
                if (s_port == 0) {
                    usage(argv[0]);
                }
                RLOGI("Opening loopback port %d\n", s_port);
            break;

            case 'd':
                s_device_path = optarg;
                RLOGI("Opening tty device %s\n", s_device_path);
            break;

            case 's':
                s_device_path   = optarg;
                s_device_socket = 1;
                RLOGI("Opening socket %s\n", s_device_path);
            break;

            default:
                usage(argv[0]);
        }
    }

    if (s_port < 0 && s_device_path == NULL) {
        usage(argv[0]);
    }

    RIL_register(&s_callbacks);

    mainLoop(NULL);

    return 0;
}
#endif /* RIL_SHLIB */

int isUserLoad() {
    int isUserLoad = 0;
    char property_value_emulation[PROPERTY_VALUE_MAX] = { 0 };
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("vendor.ril.emulation.userload", property_value_emulation, "0");
    if(strcmp("1", property_value_emulation) == 0) {
        return 1;
    }
    property_get("ro.build.type", property_value, "");
    isUserLoad = (strcmp("user", property_value) == 0);
    //RLOGD("isUserLoad: %d", isUserLoad);
    return isUserLoad;
}

int isInternalLoad() {
#ifdef __PRODUCTION_RELEASE__
    return 0;
#else
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("vendor.ril.emulation.production", property_value, "0");
    return (strcmp("1", property_value) != 0);
#endif /* __PRODUCTION_RELEASE__ */
}

/*
* Purpose:  Function responsible by all signal handlers treatment any new signal must be added here
* Input:      param - signal ID
* Return:    -
*/
void signalTreatment(int param)
{
    RLOGD("signal_no=%d", param);
    switch (param)
    {
    case SIGSEGV:
    case SIGABRT:
    case SIGILL:
    case SIGPIPE:
    case SIGBUS:
    case SIGFPE:
        RLOGD("trigger TRM");
    #ifdef HAVE_AEE_FEATURE
        char prop_value[PROPERTY_VALUE_MAX] = { 0 };
        property_get("persist.log.tag.tel_dbg", prop_value, "0");
        if(prop_value[0] == '1') {
            aee_system_exception("mtkrild", NULL, DB_OPT_DEFAULT, "rilproxy NE");
        }
    #endif
        property_set("vendor.ril.mux.report.case","2");
        property_set("vendor.ril.cdma.report.case", "1");
        property_set("vendor.ril.muxreport", "1");
        break;
    default:
        exit(0);
        break;
    }
}

void registerSignal()
{
    //signals treatment
    if ((isInternalLoad() == 0) && (isUserLoad() == 1)) {
        RLOGD("Setup signal handling");
        signal(SIGSEGV, signalTreatment);
        signal(SIGABRT, signalTreatment);
        signal(SIGILL, signalTreatment);
        signal(SIGPIPE, signalTreatment);
        signal(SIGBUS, signalTreatment);
        signal(SIGFPE, signalTreatment);
    }
}

/// M: SAP start
void sendBtSapResponseComplete(RIL_Token t, RIL_Errno ret, MsgId msgId, void *data) {
    const pb_field_t *fields = NULL;
    size_t encoded_size = 0;
    uint32_t written_size = 0;
    size_t buffer_size = 0;
    pb_ostream_t ostream;
    bool success = false;
    ssize_t written_bytes;
    int i = 0;

    RLOGD("sendBtSapResponseComplete, start (%d)", msgId);

    switch (msgId) {
        case MsgId_RIL_SIM_SAP_CONNECT:
            fields = RIL_SIM_SAP_CONNECT_RSP_fields;
            break;
        case MsgId_RIL_SIM_SAP_DISCONNECT:
            fields = RIL_SIM_SAP_DISCONNECT_RSP_fields;
            break;
        case MsgId_RIL_SIM_SAP_APDU:
            fields = RIL_SIM_SAP_APDU_RSP_fields;
            break;
        case MsgId_RIL_SIM_SAP_TRANSFER_ATR:
            fields = RIL_SIM_SAP_TRANSFER_ATR_RSP_fields;
            break;
        case MsgId_RIL_SIM_SAP_POWER:
            fields = RIL_SIM_SAP_POWER_RSP_fields;
            break;
        case MsgId_RIL_SIM_SAP_RESET_SIM:
            fields = RIL_SIM_SAP_RESET_SIM_RSP_fields;
            break;
        case MsgId_RIL_SIM_SAP_SET_TRANSFER_PROTOCOL:
            fields = RIL_SIM_SAP_SET_TRANSFER_PROTOCOL_RSP_fields;
            break;
        case MsgId_RIL_SIM_SAP_ERROR_RESP:
            fields = RIL_SIM_SAP_ERROR_RSP_fields;
            break;
        default:
            RLOGE("sendBtSapResponseComplete, MsgId is mistake!");
            return;
    }

    if ((success = pb_get_encoded_size(&encoded_size, fields, data)) &&
            encoded_size <= INT32_MAX) {
        //buffer_size = encoded_size + sizeof(uint32_t);
        buffer_size = encoded_size;
        uint8_t buffer[buffer_size];
        //written_size = htonl((uint32_t) encoded_size);
        ostream = pb_ostream_from_buffer(buffer, buffer_size);
        //pb_write(&ostream, (uint8_t *)&written_size, sizeof(written_size));
        success = pb_encode(&ostream, fields, data);

        if(success) {
            RLOGD("sendBtSapResponseComplete, Size: %d (0x%x) Size as written: 0x%x",
                (int) encoded_size, (int) encoded_size, (int) written_size);
            // Send response
            RIL_SAP_onRequestComplete(t, ret, buffer, buffer_size);
        } else {
            RLOGE("sendBtSapResponseComplete, Encode failed!");
        }
    } else {
        RLOGE("Not sending response type %d: encoded_size: %u. encoded size result: %d",
        msgId, (int) encoded_size, success);
    }
}
/// SAP end
