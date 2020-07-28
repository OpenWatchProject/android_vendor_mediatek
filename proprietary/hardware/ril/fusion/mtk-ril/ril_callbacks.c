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
#include <getopt.h>
#include <cutils/sockets.h>
#include <termios.h>
#include "pb_decode.h"
#include "pb_encode.h"
#include <arpa/inet.h>
//#include <telephony/mtk_ril.h>
#include "ril_callbacks.h"
#include <vendor/mediatek/proprietary/hardware/ril/platformlib/common/libmtkrilutils/proto/sap-api.pb.h>
#include <mtk_log.h>

#include "Rfx.h"

#undef LOG_TAG
#define LOG_TAG "RIL-Fusion"

static void *noopRemoveWarning( void *a ) { return a; }
#define RIL_UNUSED_PARM(a) noopRemoveWarning((void *)&(a));

#if defined(ANDROID_MULTI_SIM)
static void
onRequest (int request, void *data, size_t datalen, RIL_Token t, RIL_SOCKET_ID socket_id);
#else
static void
onRequest (int request, void *data, size_t datalen, RIL_Token t);
#endif
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
static void updateConnectionState(RIL_SOCKET_ID socketId, int isConnected);

/*** Static Variables ***/
static const RIL_RadioFunctions s_callbacks = {
    RIL_VERSION,
    onRequest,
    currentState,
    onSupports,
    onCancel,
    getVersion,
    updateConnectionState,
};

#ifdef RIL_SHLIB
static const struct RIL_Env *s_rilenv;
#endif

/// M: BT sap @{
static const struct RIL_Env *s_rilsapenv;
static const RIL_RadioFunctions s_sapcallbacks = {
    RIL_VERSION,
    onSapRequest,
    currentState,
    onSupports,
    onCancel,
    getVersion,
    updateConnectionState,
};

void sendSapResponseComplete(RIL_Token t, RIL_Errno ret, MsgId msgId, void *data);
// @}

RIL_RadioState getRadioState(RIL_SOCKET_ID rid);

static int s_port = -1;
static const char * s_device_path = NULL;
static int          s_device_socket = 0;

static const struct timeval TIMEVAL_SIMPOLL = {1,0};
static const struct timeval TIMEVAL_CALLSTATEPOLL = {0,500000};
static const struct timeval TIMEVAL_0 = {0,0};

static RIL_RadioState s_radio_state[MAX_SIM_COUNT];
static pthread_mutex_t s_state_mutex[MAX_SIM_COUNT];

void setRadioState(RIL_RadioState newState, RIL_SOCKET_ID rid);
int toRealSlot(int slotId);

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
    RIL_SOCKET_ID socId = RIL_SOCKET_1;
    #if defined(ANDROID_MULTI_SIM)
    socId = socket_id;
    #endif
    mtkLogD(LOG_TAG, "[RilFusion] onRequest: request = %d, datalen = %zu, slotId = %d",
            request, datalen, socId);
    rfx_enqueue_request_message(request, data, datalen, t, toRealSlot(socId));
}

#if defined(ANDROID_MULTI_SIM)
static void onSapRequest(int request, void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID socket_id) {
#else
static void onSapRequest(int request, void *data, size_t datalen, RIL_Token t) {
#endif
    mtkLogD(LOG_TAG, "[RilFusion] onSapRequest: %d", request);

    RIL_SOCKET_ID socketId = RIL_SOCKET_1;
    #if defined(ANDROID_MULTI_SIM)
    socketId = socket_id;
    #endif

    if (request < MsgId_RIL_SIM_SAP_CONNECT /* MsgId_UNKNOWN_REQ */ ||
            request > MsgId_RIL_SIM_SAP_SET_TRANSFER_PROTOCOL) {
        mtkLogE(LOG_TAG, "invalid request");
        RIL_SIM_SAP_ERROR_RSP rsp;
        rsp.dummy_field = 1;
        sendSapResponseComplete(t, (RIL_Errno) Error_RIL_E_REQUEST_NOT_SUPPORTED,
                MsgId_RIL_SIM_SAP_ERROR_RESP, &rsp);
        return;
    }

    /*if (s_md_off) {
        RFX_LOG_D(LOG_TAG, "MD off and reply failure to Sap message");
        RIL_SIM_SAP_CONNECT_RSP rsp;
        rsp.response = RIL_SIM_SAP_CONNECT_RSP_Response_RIL_E_SAP_CONNECT_FAILURE;
        rsp.has_max_message_size = false;
        rsp.max_message_size = 0;
        sendSapResponseComplete(t, Error_RIL_E_RADIO_NOT_AVAILABLE, MsgId_RIL_SIM_SAP_CONNECT,
                &rsp);
        return;
    }*/

    rfx_enqueue_sap_request_message(request, data, datalen, t, socketId);
}

void sendSapResponseComplete(RIL_Token t, RIL_Errno ret, MsgId msgId, void *data) {
    const pb_field_t *fields = NULL;
    size_t encoded_size = 0;
    uint32_t written_size = 0;
    size_t buffer_size = 0;
    pb_ostream_t ostream;
    bool success = false;
    ssize_t written_bytes;
    int i = 0;

    mtkLogD(LOG_TAG, "sendSapResponseComplete, start (%d)", msgId);

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
        case MsgId_RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS:
            fields = RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP_fields;
            break;
        default:
            mtkLogE(LOG_TAG, "sendSapResponseComplete, MsgId is mistake!");
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
            mtkLogD(LOG_TAG, "sendSapResponseComplete, Size: %zu (0x%zx) Size as written: 0x%x",
                encoded_size, encoded_size, written_size);
            // Send response
            RFX_SAP_onRequestComplete(t, ret, buffer, buffer_size);
        } else {
            mtkLogE(LOG_TAG, "sendSapResponseComplete, Encode failed!");
        }
    } else {
        mtkLogE(LOG_TAG, "Not sending response type %d: encoded_size: %zu. encoded size result: %d",
                msgId, encoded_size, success);
    }
}

void setRadioState(RIL_RadioState newState, RIL_SOCKET_ID rid) {
    RIL_RadioState oldState;
    RIL_RadioState *pState = NULL;

    pthread_mutex_lock(&s_state_mutex[rid]);
    oldState = s_radio_state[rid];
    pState = &s_radio_state[rid];

    if (*pState != newState) {
        *pState = newState;
    }
    pthread_mutex_unlock(&s_state_mutex[rid]);

    if (*pState != oldState) {
    #if defined(ANDROID_MULTI_SIM)
        RFX_onUnsolicitedResponse(RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED, NULL, 0, rid);
    #else
        RFX_onUnsolicitedResponse(RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED, NULL, 0);
    #endif
    }
}

RIL_RadioState getRadioState(RIL_SOCKET_ID rid)
{
    RIL_RadioState radioState = s_radio_state[rid];
    mtkLogI(LOG_TAG, "getRadioState(): radioState=%d\n", radioState);
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


static void updateConnectionState(RIL_SOCKET_ID socketId, int isConnected) {
    rfx_update_connection_state(socketId, isConnected);
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
    mtkLogI(LOG_TAG, "entering mainLoop()");

    resetWakelock();
    // init modem protocol and set Radio state in MdComm layer
    // init radio state
    for (int i = 0; i < MAX_SIM_COUNT; i++) {
        s_radio_state[i] = RADIO_STATE_UNAVAILABLE;
        pthread_mutex_init(&s_state_mutex[i], NULL);
    }

    rfx_init();
    return 0;
}

#ifdef RIL_SHLIB

pthread_t s_tid_mainloop;

void RIL_setRilEnvForGT(const struct RIL_Env *env) {
    s_rilenv = env;
}

const RIL_RadioFunctions *RIL_Init(const struct RIL_Env *env, int argc __unused,
        char **argv __unused)
{
    int ret;
    int fd = -1;
    int opt;
    pthread_attr_t attr;

    s_rilenv = env;

    pthread_attr_init (&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    ret = pthread_create(&s_tid_mainloop, &attr, mainLoop, NULL);
    if (ret != 0) {
        mtkLogE(LOG_TAG, "Failed to create mainLoop thread: %s", strerror(ret));
    }

    return &s_callbacks;
}

const RIL_RadioFunctions *RIL_SAP_Init(const struct RIL_Env *env, int argc __unused,
        char **argv __unused) {
    mtkLogD(LOG_TAG, "RIL_SAP_Init");
    s_rilsapenv = env;
    return &s_sapcallbacks;
}

void RFX_onRequestComplete(RIL_Token t, RIL_Errno e, void *response, size_t responselen) {
    s_rilenv->OnRequestComplete(t, e, response, responselen);
}

#if defined(ANDROID_MULTI_SIM)
void RFX_onUnsolicitedResponse(int unsolResponse, const void *data,
        size_t datalen, RIL_SOCKET_ID socket_id) {
    s_rilenv->OnUnsolicitedResponse(unsolResponse, data, datalen, socket_id);
}
#else
void RFX_onUnsolicitedResponse(int unsolResponse, const void *data,
        size_t datalen) {
    s_rilenv->OnUnsolicitedResponse(unsolResponse, data, datalen);
}
#endif

void RFX_requestTimedCallback(RIL_TimedCallback callback, void *param,
        const struct timeval *relativeTime) {
    s_rilenv->RequestTimedCallback(callback, param, relativeTime);
}

void RFX_onRequestAck(RIL_Token t) {
    s_rilenv->OnRequestAck(t);
}

void RFX_SAP_onRequestComplete(RIL_Token t, RIL_Errno e, void *response,
        size_t responselen) {
    s_rilsapenv->OnRequestComplete(t, e, response, responselen);
}

#if defined(ANDROID_MULTI_SIM)
void RFX_SAP_onUnsolicitedResponse(int unsolResponse, void *data, size_t datalen, int socket_id) {
    s_rilsapenv->OnUnsolicitedResponse(unsolResponse, data, datalen, socket_id);
}
#else
void RFX_SAP_onUnsolicitedResponse(int unsolResponse, void *data, size_t datalen) {
    s_rilsapenv->OnUnsolicitedResponse(unsolResponse, data, datalen)
}
#endif

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
                mtkLogI(LOG_TAG, "Opening loopback port %d\n", s_port);
            break;

            case 'd':
                s_device_path = optarg;
                mtkLogI(LOG_TAG, "Opening tty device %s\n", s_device_path);
            break;

            case 's':
                s_device_path   = optarg;
                s_device_socket = 1;
                mtkLogI(LOG_TAG, "Opening socket %s\n", s_device_path);
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
