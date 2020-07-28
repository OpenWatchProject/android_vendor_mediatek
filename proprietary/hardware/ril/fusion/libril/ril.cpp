/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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
/* //device/libs/telephony/ril.cpp
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

#include <hardware_legacy/power.h>
#include <telephony/ril_cdma_sms.h>
#include <telephony/mtk_ril.h>
#include <cutils/sockets.h>
#include <cutils/jstring.h>
#include <telephony/record_stream.h>
#include <utils/SystemClock.h>
#include <pthread.h>
#include <cutils/jstring.h>
#include <sys/types.h>
#include <sys/limits.h>
#include <sys/system_properties.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <assert.h>
#include <ctype.h>
#include <sys/un.h>
#include <assert.h>
#include <netinet/in.h>
#include <RilSapSocket.h>
#include <ril_service.h>
#include <sap_service.h>
#include <libmtkrilutils.h>
#include <binder/ProcessState.h>
#include <mtk_log.h>
#include <mtk_properties.h>

#ifdef HAVE_AEE_FEATURE
#include "aee.h"
#endif

#include "RilOpProxy.h"
#include "radioConfig_service.h"

#undef LOG_TAG
#define LOG_TAG "RILC"

extern "C" void
RIL_onRequestComplete(RIL_Token t, RIL_Errno e, void *response, size_t responselen);

extern "C" void
RIL_onRequestAck(RIL_Token t);
namespace android {

#define PHONE_PROCESS "radio"
#define BLUETOOTH_PROCESS "bluetooth"

#define ANDROID_WAKE_LOCK_NAME "radio-interface"

#define ANDROID_WAKE_LOCK_SECS 0
#define ANDROID_WAKE_LOCK_USECS 200000

#define PROPERTY_RIL_IMPL "gsm.version.ril-impl"

// match with constant in RIL.java
#define MAX_COMMAND_BYTES (20 * 1024)

// Basically: memset buffers that the client library
// shouldn't be using anymore in an attempt to find
// memory usage issues sooner.
#define MEMSET_FREED 1

#define NUM_ELEMS(a)     (sizeof (a) / sizeof (a)[0])

/* Negative values for private RIL errno's */
#define RIL_ERRNO_INVALID_RESPONSE (-1)
#define RIL_ERRNO_NO_MEMORY (-12)

// request, response, and unsolicited msg print macro
#define PRINTBUF_SIZE 8096

typedef struct UserCallbackInfo {
    RIL_TimedCallback p_callback;
    void *userParam;
    struct ril_event event;
    struct UserCallbackInfo *p_next;
} UserCallbackInfo;

extern "C" const char * failCauseToString(RIL_Errno);
extern "C" const char * callStateToString(RIL_CallState);
extern "C" const char * radioStateToString(RIL_RadioState);
extern "C" const char * rilSocketIdToString(RIL_SOCKET_ID socket_id);

// Multiple Radio HIDL Client for IMS
extern "C" int toRealSlot(int slotId);
extern "C" int toClientSlot(int slotId, ClientId clientId);
extern "C" ClientId getClientBySlot(int slotId);
extern "C"
char ril_service_name_base[MAX_SERVICE_NAME_LENGTH] = RIL_SERVICE_NAME_BASE;
extern "C"
char ril_service_name[MAX_SERVICE_NAME_LENGTH] = RIL1_SERVICE_NAME;
/*******************************************************************/

RIL_RadioFunctions s_callbacks = {0, NULL, NULL, NULL, NULL, NULL, NULL};
static int s_registerCalled = 0;

static pthread_t s_tid_dispatch;
static int s_started = 0;

static int s_fdWakeupRead;
static int s_fdWakeupWrite;

int s_wakelock_count = 0;

static struct ril_event s_wakeupfd_event;

static pthread_mutex_t s_wakeLockCountMutex = PTHREAD_MUTEX_INITIALIZER;

// Pending Requests Mutex for Multiple Radio HIDL Client
static pthread_mutex_t s_pendingRequestsMutex_sockets[] = {PTHREAD_MUTEX_INITIALIZER,
                                                           PTHREAD_MUTEX_INITIALIZER,
                                                           PTHREAD_MUTEX_INITIALIZER,
                                                           PTHREAD_MUTEX_INITIALIZER};

static RequestInfo *s_pendingRequests_sockets[] = {NULL,
                                                   NULL,
                                                   NULL,
                                                   NULL};

static const struct timeval TIMEVAL_WAKE_TIMEOUT = {ANDROID_WAKE_LOCK_SECS,ANDROID_WAKE_LOCK_USECS};


static pthread_mutex_t s_startupMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t s_startupCond = PTHREAD_COND_INITIALIZER;

static UserCallbackInfo *s_last_wake_timeout_info = NULL;

static void *s_lastNITZTimeData[MAX_SIM_COUNT] = {NULL};
static size_t s_lastNITZTimeDataSize[MAX_SIM_COUNT];

#if RILC_LOG
    static char printBuf[PRINTBUF_SIZE];
#endif

typedef struct AtResponseList {
    int id;
    char* data;
    size_t datalen;
    AtResponseList *pNext;
} AtResponseList;

/*******************************************************************/
// cache URC
bool cacheUrc(int unsolResponse, const void *data, size_t datalen, RIL_SOCKET_ID id);
void sendPendedUrcs(RIL_SOCKET_ID socket_id, android::ClientType clientType);
void sendUrc(RIL_SOCKET_ID socket_id, AtResponseList** urcCached, android::ClientType clientType);

static AtResponseList* pendedUrcList[MAX_SIM_COUNT][CLIENT_TYPE_COUNT] = {0};
static bool bIsCallbackReady[MAX_SIM_COUNT][CLIENT_TYPE_COUNT] = {0};
static pthread_mutex_t s_pendingUrcMutex[MAX_SIM_COUNT][CLIENT_TYPE_COUNT];

/*******************************************************************/
static pthread_mutex_t s_state_mutex[MAX_SIM_COUNT];
static void grabPartialWakeLock();
void releaseWakeLock();
static void wakeTimeoutCallback(void *);

#ifdef HAVE_AEE_FEATURE
void triggerWarning(char *pErrMsg);
#endif

#ifdef RIL_SHLIB
#if defined(ANDROID_MULTI_SIM)
extern "C" void RIL_onUnsolicitedResponse(int unsolResponse, const void *data,
                                size_t datalen, RIL_SOCKET_ID socket_id);
#else
extern "C" void RIL_onUnsolicitedResponse(int unsolResponse, const void *data,
                                size_t datalen);
#endif
#endif

#if defined(ANDROID_MULTI_SIM)
#define RIL_UNSOL_RESPONSE(a, b, c, d) RIL_onUnsolicitedResponse((a), (b), (c), (d))
#else
#define RIL_UNSOL_RESPONSE(a, b, c, d) RIL_onUnsolicitedResponse((a), (b), (c))
#endif
#define CALL_UPDATECONNECTIONSTATE(a, b) s_callbacks.updateConnectionState((a), (b))

static UserCallbackInfo * internalRequestTimedCallback
    (RIL_TimedCallback callback, void *param,
        const struct timeval *relativeTime);

/** Index == requestNumber */
static CommandInfo s_commands[] = {
#include <telephony/ril_commands.h>
};

static UnsolResponseInfo s_unsolResponses[] = {
#include <telephony/ril_unsol_commands.h>
};

/// M: ril proxy
static CommandInfo s_mtk_commands[] = {
#include <telephony/mtk_ril_commands.h>
};

static UnsolResponseInfo s_mtk_unsolResponses[] = {
#include <telephony/mtk_ril_unsol_commands.h>
};
/// M

char * RIL_getServiceName() {
    return ril_service_name;
}

RequestInfo *
addRequestToList(int serial, int slotId, int request) {
    RequestInfo *pRI;
    int ret;
    RIL_SOCKET_ID socket_id = (RIL_SOCKET_ID) slotId;
    /* Hook for current context */
    /* pendingRequestsMutextHook refer to &s_pendingRequestsMutex */
    pthread_mutex_t* pendingRequestsMutexHook = NULL;
    /* pendingRequestsHook refer to &s_pendingRequests */
    RequestInfo**    pendingRequestsHook = NULL;

    pendingRequestsMutexHook = &(s_pendingRequestsMutex_sockets[toRealSlot(slotId)]);
    pendingRequestsHook = &(s_pendingRequests_sockets[toRealSlot(slotId)]);

    pRI = (RequestInfo *)calloc(1, sizeof(RequestInfo));
    if (pRI == NULL) {
        mtkLogE(LOG_TAG, "Memory allocation failed for request %s", requestToString(request));
        return NULL;
    }

    pRI->token = serial;

    if (request >= RIL_REQUEST_VENDOR_BASE) {
        for (int i = 0; i < (int)NUM_ELEMS(s_mtk_commands); i++) {
            if (request == s_mtk_commands[i].requestNumber) {
                pRI->pCI = &(s_mtk_commands[i]);
                break;
            }
        }
    } else {
        for (int i = 0; i < (int)NUM_ELEMS(s_commands); i++) {
            if (request == s_commands[i].requestNumber) {
                pRI->pCI = &(s_commands[i]);
                break;
            }
        }
    }

    if (pRI->pCI == NULL) {
        mtkLogI(LOG_TAG, "try to getOpCommandInfo from operator library");
        pRI->pCI = RilOpProxy::getOpCommandInfo(request);
    }

    if (pRI->pCI == NULL) {
        mtkLogE(LOG_TAG, "Unsupported request id %s", requestToString(request));
        free(pRI);
        return NULL;
    }

    pRI->socket_id = socket_id;
    mtkLogD(LOG_TAG, "[RilProxy] addRequestToList: pRI->socket_id = %d", pRI->socket_id);

    ret = pthread_mutex_lock(pendingRequestsMutexHook);
    assert (ret == 0);

    pRI->p_next = *pendingRequestsHook;
    *pendingRequestsHook = pRI;

    ret = pthread_mutex_unlock(pendingRequestsMutexHook);
    assert (ret == 0);

    return pRI;
}

static void triggerEvLoop() {
    int ret;
    if (!pthread_equal(pthread_self(), s_tid_dispatch)) {
        /* trigger event loop to wakeup. No reason to do this,
         * if we're in the event loop thread */
         do {
            ret = write (s_fdWakeupWrite, " ", 1);
         } while (ret < 0 && errno == EINTR);
    }
}

static void rilEventAddWakeup(struct ril_event *ev) {
    ril_event_add(ev);
    triggerEvLoop();
}

/**
 * A write on the wakeup fd is done just to pop us out of select()
 * We empty the buffer here and then ril_event will reset the timers on the
 * way back down
 */
static void processWakeupCallback(int fd, short flags, void *param) {
    char buff[16];
    int ret;

    mtkLogV(LOG_TAG, "processWakeupCallback");

    /* empty our wakeup socket out */
    do {
        ret = read(s_fdWakeupRead, &buff, sizeof(buff));
    } while (ret > 0 || (ret < 0 && errno == EINTR));
}

static void resendLastNITZTimeData(RIL_SOCKET_ID socket_id) {
    if (s_lastNITZTimeData[(int)socket_id] != NULL) {
        int responseType = (s_callbacks.version >= 13)
                           ? RESPONSE_UNSOLICITED_ACK_EXP
                           : RESPONSE_UNSOLICITED;
        // acquire read lock for the service before calling nitzTimeReceivedInd() since it reads
        // nitzTimeReceived in ril_service
        pthread_rwlock_t *radioServiceRwlockPtr = radio::getRadioServiceRwlock(
                (int) socket_id);
        radio::lockRadioServiceRlock(radioServiceRwlockPtr, (int)socket_id);
        mtkLogV(LOG_TAG, "resendLastNITZTimeData, got lock %d", (int) socket_id);
        int ret = radio::nitzTimeReceivedInd(
            (int)socket_id, responseType, 0,
            RIL_E_SUCCESS, s_lastNITZTimeData[(int)socket_id], s_lastNITZTimeDataSize[(int)socket_id]);
        if (ret == 0) {
            free(s_lastNITZTimeData[(int)socket_id]);
            s_lastNITZTimeData[(int)socket_id] = NULL;
        }
        radio::unlockRadioServiceRlock(radioServiceRwlockPtr, (int) socket_id);
        mtkLogV(LOG_TAG, "resendLastNITZTimeData, release lock %d", (int) socket_id);
    }
}

void onNewCommandConnect(RIL_SOCKET_ID socket_id, android::ClientType clientType) {

    RIL_SOCKET_ID slotId = (RIL_SOCKET_ID) toRealSlot((int) socket_id);
    ClientId client = getClientBySlot((int) socket_id);
    pthread_mutex_lock(&(s_pendingUrcMutex[slotId][clientType]));
    bIsCallbackReady[slotId][clientType] = true;
    pthread_mutex_unlock(&(s_pendingUrcMutex[slotId][clientType]));
    // Inform we are connected and the ril version
    int rilVer = s_callbacks.version;
    if(clientType == android::RADIO_AOSP) {
        RIL_UNSOL_RESPONSE(RIL_UNSOL_RIL_CONNECTED,
                                        &rilVer, sizeof(rilVer), slotId);
        // implicit radio state changed
        RIL_UNSOL_RESPONSE(RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED,
                                        NULL, 0, slotId);

        // Send last NITZ time data, in case it was missed
        if (s_lastNITZTimeData[(int)slotId] != NULL) {
            resendLastNITZTimeData(slotId);
        }

        // Get version string
        if (slotId == (RIL_get3GSIM()-1)) {
            if (s_callbacks.getVersion != NULL) {
                const char *version;
                version = s_callbacks.getVersion();
                mtkLogI(LOG_TAG, "RIL Daemon version: %s\n", version);

                mtk_property_set(PROPERTY_RIL_IMPL, version);
            } else {
                mtkLogI(LOG_TAG, "RIL Daemon version: unavailable\n");
                mtk_property_set(PROPERTY_RIL_IMPL, "unavailable");
            }
        }
    } else if (clientType == android::IMS_AOSP) {
        // implicit radio state changed
        RIL_UNSOL_RESPONSE(RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED,
                NULL, 0, slotId);
    }
    switch(client) {
        case CLIENT_RILJ:
            sendPendedUrcs(slotId, clientType);
            CALL_UPDATECONNECTIONSTATE(slotId, CONNECTION_STATE_ON);
            break;
        case CLIENT_IMS:
            sendPendedUrcs(slotId, clientType);
            break;
        default:
            mtkLogE(LOG_TAG, "Incorrect division");
    }
}

void onCommandDisconnect(RIL_SOCKET_ID socket_id, android::ClientType clientType) {
    RIL_SOCKET_ID slotId = (RIL_SOCKET_ID) toRealSlot((int) socket_id);
    ClientId client = getClientBySlot((int) socket_id);
    pthread_mutex_lock(&(s_pendingUrcMutex[slotId][clientType]));
    bIsCallbackReady[slotId][clientType] = false;
    pthread_mutex_unlock(&(s_pendingUrcMutex[slotId][clientType]));
    if (CLIENT_RILJ == client) {
        CALL_UPDATECONNECTIONSTATE(slotId, CONNECTION_STATE_OFF);
    }
}

static void userTimerCallback (int fd, short flags, void *param) {
    UserCallbackInfo *p_info;

    p_info = (UserCallbackInfo *)param;

    p_info->p_callback(p_info->userParam);


    // FIXME generalize this...there should be a cancel mechanism
    if (s_last_wake_timeout_info != NULL && s_last_wake_timeout_info == p_info) {
        s_last_wake_timeout_info = NULL;
    }

    free(p_info);
}


static void *
eventLoop(void *param) {
    int ret;
    int filedes[2];

    ril_event_init();

    pthread_mutex_lock(&s_startupMutex);

    s_started = 1;
    pthread_cond_broadcast(&s_startupCond);

    pthread_mutex_unlock(&s_startupMutex);

    ret = pipe(filedes);

    if (ret < 0) {
        mtkLogE(LOG_TAG, "Error in pipe() errno:%d", errno);
        return NULL;
    }

    s_fdWakeupRead = filedes[0];
    s_fdWakeupWrite = filedes[1];

    fcntl(s_fdWakeupRead, F_SETFL, O_NONBLOCK);

    ril_event_set (&s_wakeupfd_event, s_fdWakeupRead, true,
                processWakeupCallback, NULL);

    rilEventAddWakeup (&s_wakeupfd_event);

    // Only returns on error
    ril_event_loop();
    mtkLogE(LOG_TAG, "error in event_loop_base errno:%d", errno);
    // kill self to restart on error
    kill(0, SIGKILL);

    return NULL;
}

extern "C" void
RIL_startEventLoop(void) {
    /* spin up eventLoop thread and wait for it to get started */
    s_started = 0;
    pthread_mutex_lock(&s_startupMutex);

    RilOpProxy::initOpLibrary();

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    int result = pthread_create(&s_tid_dispatch, &attr, eventLoop, NULL);
    if (result != 0) {
        mtkLogE(LOG_TAG, "Failed to create dispatch thread: %s", strerror(result));
        goto done;
    }

    while (s_started == 0) {
        pthread_cond_wait(&s_startupCond, &s_startupMutex);
    }

done:
    pthread_mutex_unlock(&s_startupMutex);
}

// Used for testing purpose only.
extern "C" void RIL_setcallbacks (const RIL_RadioFunctions *callbacks) {
    memcpy(&s_callbacks, callbacks, sizeof (RIL_RadioFunctions));
}

extern "C" void
RIL_register (const RIL_RadioFunctions *callbacks) {
    mtkLogI(LOG_TAG, "FUSION RIL getSimCount: %d", getSimCount());

#ifdef MTK_USE_HIDL
    ProcessState::initWithDriver("/dev/vndbinder");
#endif
    if (callbacks == NULL) {
        mtkLogE(LOG_TAG, "RIL_register: RIL_RadioFunctions * null");
        return;
    }
    if (callbacks->version < RIL_VERSION_MIN) {
        mtkLogE(LOG_TAG, "RIL_register: version %d is to old, min version is %d",
             callbacks->version, RIL_VERSION_MIN);
        return;
    }

    mtkLogE(LOG_TAG, "RIL_register: RIL version %d", callbacks->version);

    if (s_registerCalled > 0) {
        mtkLogE(LOG_TAG, "RIL_register has been called more than once. "
                "Subsequent call ignored");
        return;
    }

    memcpy(&s_callbacks, callbacks, sizeof (RIL_RadioFunctions));

    s_registerCalled = 1;

    mtkLogI(LOG_TAG, "s_registerCalled flag set, %d", s_started);
    // Little self-check

    radio::registerService(&s_callbacks, s_commands);
    mtkLogI(LOG_TAG, "RILHIDL called registerService");

    RilOpProxy::registerOpService(&s_callbacks, s_commands);
    mtkLogI(LOG_TAG, "RILOP HIDL called registerOpService");

    radioConfig::registerService(&s_callbacks, s_commands);
    mtkLogI(LOG_TAG, "radioConfig called register radio config Service");
    // initialize mutex
    for (int i = 0; i < MAX_SIM_COUNT; i++) {
        pthread_mutex_init(&s_state_mutex[i], NULL);
    }
    // initialize cache urc mutex
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
    for (int i = 0; i < MAX_SIM_COUNT; i++) {
        for (int j = 0; j < CLIENT_TYPE_COUNT; j++) {
            pthread_mutex_init(&(s_pendingUrcMutex[i][j]), &attr);
        }
    }
}

extern "C" void
RIL_register_socket (const RIL_RadioFunctions *(*Init)(const struct RIL_Env *, int, char **),
        RIL_SOCKET_TYPE socketType, int argc, char **argv) {

    const RIL_RadioFunctions* UimFuncs = NULL;

    if(Init) {
        UimFuncs = Init(&RilSapSocket::uimRilEnv, argc, argv);

        switch(socketType) {
            case RIL_SAP_SOCKET:
                RilSapSocket::initSapSocket(RIL1_SERVICE_NAME, UimFuncs);
                if (getSimCount() >= 2) {
                RilSapSocket::initSapSocket(RIL2_SERVICE_NAME, UimFuncs);
                }
                if (getSimCount() >= 3) {
                RilSapSocket::initSapSocket(RIL3_SERVICE_NAME, UimFuncs);
                }
                if (getSimCount() >= 4) {
                RilSapSocket::initSapSocket(RIL4_SERVICE_NAME, UimFuncs);
                }
                break;
            default:;
        }

        mtkLogI(LOG_TAG, "RIL_register_socket: calling registerService");
        sap::registerService(UimFuncs);
    }
}

// Check and remove RequestInfo if its a response and not just ack sent back
static int
checkAndDequeueRequestInfoIfAck(struct RequestInfo *pRI, bool isAck) {
    int ret = 0;
    /* Hook for current context
       pendingRequestsMutextHook refer to &s_pendingRequestsMutex */
    pthread_mutex_t* pendingRequestsMutexHook = NULL;
    /* pendingRequestsHook refer to &s_pendingRequests */
    RequestInfo ** pendingRequestsHook = NULL;

    if (pRI == NULL) {
        return 0;
    }

    pendingRequestsMutexHook =
            &(s_pendingRequestsMutex_sockets[toRealSlot(pRI->socket_id)]);

    pendingRequestsHook =
            &(s_pendingRequests_sockets[toRealSlot(pRI->socket_id)]);


    pthread_mutex_lock(pendingRequestsMutexHook);

    for(RequestInfo **ppCur = pendingRequestsHook
        ; *ppCur != NULL
        ; ppCur = &((*ppCur)->p_next)
    ) {
        if (pRI == *ppCur) {
            ret = 1;
            if (isAck) { // Async ack
                if (pRI->wasAckSent == 1) {
                    mtkLogD(LOG_TAG, "Ack was already sent for %s", requestToString(pRI->pCI->requestNumber));
                } else {
                    pRI->wasAckSent = 1;
                }
            } else {
                *ppCur = (*ppCur)->p_next;
            }
            break;
        }
    }

    pthread_mutex_unlock(pendingRequestsMutexHook);

    return ret;
}

extern "C" void
RIL_onRequestAck(RIL_Token t) {
    RequestInfo *pRI;

    RIL_SOCKET_ID socket_id = RIL_SOCKET_1;

    pRI = (RequestInfo *)t;

    if (!checkAndDequeueRequestInfoIfAck(pRI, true)) {
        mtkLogE(LOG_TAG, "RIL_onRequestAck: invalid RIL_Token");
        return;
    }

    socket_id = pRI->socket_id;

#if VDBG
    mtkLogD(LOG_TAG, "Request Ack, %s", rilSocketIdToString(socket_id));
#endif

    appendPrintBuf("Ack [%04d]< %s", pRI->token, requestToString(pRI->pCI->requestNumber));

    if (pRI->cancelled == 0) {
        pthread_rwlock_t *radioServiceRwlockPtr = radio::getRadioServiceRwlock(
                (int) socket_id);
        radio::lockRadioServiceRlock(radioServiceRwlockPtr, (int)socket_id);
        mtkLogV(LOG_TAG, "RIL_onRequestAck, got lock %d", (int) socket_id);
        radio::acknowledgeRequest((int) socket_id, pRI->token);
        radio::unlockRadioServiceRlock(radioServiceRwlockPtr, (int) socket_id);
        mtkLogV(LOG_TAG, "RIL_onRequestAck, release lock %d", (int) socket_id);
    }
}
extern "C" void
RIL_onRequestComplete(RIL_Token t, RIL_Errno e, void *response, size_t responselen) {
    RequestInfo *pRI;
    int ret;
    RIL_SOCKET_ID socket_id = RIL_SOCKET_1;

    pRI = (RequestInfo *)t;

    if (!checkAndDequeueRequestInfoIfAck(pRI, false)) {
        mtkLogE (LOG_TAG, "RIL_onRequestComplete: invalid RIL_Token");
        return;
    }

    socket_id = pRI->socket_id;
#if VDBG
    mtkLogD(LOG_TAG, "RequestComplete, %s", rilSocketIdToString(socket_id));
#endif

    if (pRI->local > 0) {
        // Locally issued command...void only!
        // response does not go back up the command socket
        mtkLogD(LOG_TAG, "C[locl]< %s", requestToString(pRI->pCI->requestNumber));

        free(pRI);
        return;
    }

    appendPrintBuf("[%04d]< %s",
        pRI->token, requestToString(pRI->pCI->requestNumber));

    if (pRI->cancelled == 0) {
        int responseType;
        if (s_callbacks.version >= 13 && pRI->wasAckSent == 1) {
            // If ack was already sent, then this call is an asynchronous response. So we need to
            // send id indicating that we expect an ack from RIL.java as we acquire wakelock here.
            responseType = RESPONSE_SOLICITED_ACK_EXP;
            grabPartialWakeLock();
        } else {
            responseType = RESPONSE_SOLICITED;
        }

        // there is a response payload, no matter success or not.
        mtkLogD(LOG_TAG, "Calling responseFunction() for token %d", pRI->token);

        pthread_rwlock_t *radioServiceRwlockPtr;
        radioServiceRwlockPtr = radio::getRadioServiceRwlock((int) socket_id);
        radio::lockRadioServiceRlock(radioServiceRwlockPtr, (int)socket_id);
        mtkLogV(LOG_TAG, "RIL_onRequestComplete, got lock %d", (int) socket_id);
        ret = pRI->pCI->responseFunction((int) socket_id, (ClientId)(socket_id/MAX_SIM_COUNT),
                responseType, pRI->token, e, response, responselen);
        radio::unlockRadioServiceRlock(radioServiceRwlockPtr, (int) socket_id);
        mtkLogV(LOG_TAG, "RIL_onRequestComplete, release lock %d", (int) socket_id);
    }
    free(pRI);
}

extern "C"
void resetWakelock(void) {
    mtkLogD(LOG_TAG, "reset Wakelock %s", ANDROID_WAKE_LOCK_NAME);
    release_wake_lock(ANDROID_WAKE_LOCK_NAME);
}

static void
grabPartialWakeLock() {
    if (s_callbacks.version >= 13) {
        int ret;
        ret = pthread_mutex_lock(&s_wakeLockCountMutex);
        assert(ret == 0);
        acquire_wake_lock(PARTIAL_WAKE_LOCK, ANDROID_WAKE_LOCK_NAME);

        UserCallbackInfo *p_info =
                internalRequestTimedCallback(wakeTimeoutCallback, NULL, &TIMEVAL_WAKE_TIMEOUT);
        if (p_info == NULL) {
            release_wake_lock(ANDROID_WAKE_LOCK_NAME);
        } else {
            s_wakelock_count++;
            if (s_last_wake_timeout_info != NULL) {
                s_last_wake_timeout_info->userParam = (void *)1;
            }
            s_last_wake_timeout_info = p_info;
        }
        ret = pthread_mutex_unlock(&s_wakeLockCountMutex);
        assert(ret == 0);
    } else {
        acquire_wake_lock(PARTIAL_WAKE_LOCK, ANDROID_WAKE_LOCK_NAME);
    }
}

void
releaseWakeLock() {
    if (s_callbacks.version >= 13) {
        int ret;
        ret = pthread_mutex_lock(&s_wakeLockCountMutex);
        assert(ret == 0);

        if (s_wakelock_count > 1) {
            s_wakelock_count--;
        } else {
            s_wakelock_count = 0;
            release_wake_lock(ANDROID_WAKE_LOCK_NAME);
            if (s_last_wake_timeout_info != NULL) {
                s_last_wake_timeout_info->userParam = (void *)1;
            }
        }

        ret = pthread_mutex_unlock(&s_wakeLockCountMutex);
        assert(ret == 0);
    } else {
        release_wake_lock(ANDROID_WAKE_LOCK_NAME);
    }
}

/**
 * Timer callback to put us back to sleep before the default timeout
 */
static void
wakeTimeoutCallback (void *param) {
    // We're using "param != NULL" as a cancellation mechanism
    if (s_callbacks.version >= 13) {
        if (param == NULL) {
            int ret;
            ret = pthread_mutex_lock(&s_wakeLockCountMutex);
            assert(ret == 0);
            s_wakelock_count = 0;
            release_wake_lock(ANDROID_WAKE_LOCK_NAME);
            ret = pthread_mutex_unlock(&s_wakeLockCountMutex);
            assert(ret == 0);
        }
    } else {
        if (param == NULL) {
            releaseWakeLock();
        }
    }
}

#if defined(ANDROID_MULTI_SIM)
extern "C"
void RIL_onUnsolicitedResponse(int unsolResponse, const void *data,
                                size_t datalen, RIL_SOCKET_ID socket_id)
#else
extern "C"
void RIL_onUnsolicitedResponse(int unsolResponse, const void *data,
                                size_t datalen)
#endif
{
    int ret;
    bool shouldScheduleTimeout = false;
    RIL_SOCKET_ID soc_id = RIL_SOCKET_1;

#if defined(ANDROID_MULTI_SIM)
    soc_id = socket_id;
#endif

    // Grab a wake lock if needed for this reponse,
    // as we exit we'll either release it immediately
    // or set a timer to release it later.
    int i=0;

    if (s_registerCalled == 0) {
        // Ignore RIL_onUnsolicitedResponse before RIL_register
        mtkLogW(LOG_TAG, "RIL_onUnsolicitedResponse called before RIL_register");
        return;
    }

    if (soc_id >= RIL_SOCKET_NUM) {
        mtkLogE(LOG_TAG, "RIL_onUnsolicitedResponse incorrect socket id %d", soc_id);
        return;
    }
    // Cache URC, If RILJ or IMS are not ready
    bool cacheUrcRet = cacheUrc(unsolResponse,data, datalen,soc_id);
    if(cacheUrcRet) {
        return;
    }

    UnsolResponseInfo *pUnsolResponse = NULL;

    if (unsolResponse >= RIL_UNSOL_VENDOR_BASE) {
        for (i = 0; i < (int)NUM_ELEMS(s_mtk_unsolResponses); i++) {
            if (unsolResponse == s_mtk_unsolResponses[i].requestNumber) {
                pUnsolResponse = &(s_mtk_unsolResponses[i]);
                mtkLogD(LOG_TAG, "find mtk unsol index %d for %d", i, unsolResponse);
                break;
            }
        }
    } else if (unsolResponse >= RIL_UNSOL_RESPONSE_BASE) {
        for (i = 0; i < (int)NUM_ELEMS(s_unsolResponses); i++) {
            if (unsolResponse == s_unsolResponses[i].requestNumber) {
                pUnsolResponse = &(s_unsolResponses[i]);
                mtkLogD(LOG_TAG, "find unsol index %d for %d", i, unsolResponse);
                break;
            }
        }
    }

    if (pUnsolResponse == NULL) {
        mtkLogD(LOG_TAG, "try to getOpUnsolResponse from operator library");
        pUnsolResponse = RilOpProxy::getOpUnsolResponseInfo(unsolResponse);
    }

    if (pUnsolResponse == NULL) {
        mtkLogE(LOG_TAG, "unsupported unsolicited response code %d", unsolResponse);
        return;
    }

    switch (pUnsolResponse->wakeType) {
        case WAKE_PARTIAL:
            grabPartialWakeLock();
            shouldScheduleTimeout = true;
        break;

        case DONT_WAKE:
        default:
            // No wake lock is grabed so don't set timeout
            shouldScheduleTimeout = false;
            break;
    }

    appendPrintBuf("[UNSL]< %s", requestToString(unsolResponse));

    switch(unsolResponse) {
        case RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED:
            /// Add mutex to avoid async error when handle this unsolResponse.@{
            /// Such as radio state off send before unavailable.
            pthread_mutex_lock(&s_state_mutex[soc_id]);
            /// @}
            break;
    }

    int responseType;
    if (s_callbacks.version >= 13
                && pUnsolResponse->wakeType == WAKE_PARTIAL) {
        responseType = RESPONSE_UNSOLICITED_ACK_EXP;
    } else {
        responseType = RESPONSE_UNSOLICITED;
    }

    pthread_rwlock_t *radioServiceRwlockPtr;
    radioServiceRwlockPtr = radio::getRadioServiceRwlock((int) soc_id);
    int rwlockRet;

    if (unsolResponse == RIL_UNSOL_NITZ_TIME_RECEIVED) {
        // get a write lock in caes of NITZ since setNitzTimeReceived() is called
        radio::lockRadioServiceWlock(radioServiceRwlockPtr, (int) soc_id);
        radio::setNitzTimeReceived((int) soc_id, android::elapsedRealtime());
    } else {
        radio::lockRadioServiceRlock(radioServiceRwlockPtr, (int)soc_id);
    }
    mtkLogV(LOG_TAG, "RIL_onUnsolicitedResponse, got lock %d", (int) soc_id);
    ret = pUnsolResponse->responseFunction(
            (int) soc_id, responseType, 0, RIL_E_SUCCESS, const_cast<void*>(data), datalen);
    if (unsolResponse == RIL_UNSOL_NITZ_TIME_RECEIVED) {
        radio::unlockRadioServiceWlock(radioServiceRwlockPtr, (int) soc_id);
    } else {
        radio::unlockRadioServiceRlock(radioServiceRwlockPtr, (int) soc_id);
    }
    mtkLogV(LOG_TAG, "RIL_onUnsolicitedResponse, release lock %d", (int) soc_id);
    if (s_callbacks.version < 13) {
        if (shouldScheduleTimeout) {
            UserCallbackInfo *p_info = internalRequestTimedCallback(wakeTimeoutCallback, NULL,
                    &TIMEVAL_WAKE_TIMEOUT);

            if (p_info == NULL) {
                 /// relase state lock @{
                if (unsolResponse == RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED) {
                    pthread_mutex_unlock(&s_state_mutex[soc_id]);
                }
                /// @}
                goto error_exit;
            } else {
                // Cancel the previous request
                if (s_last_wake_timeout_info != NULL) {
                    s_last_wake_timeout_info->userParam = (void *)1;
                }
                s_last_wake_timeout_info = p_info;
            }
        }
    }

#if VDBG
    mtkLogI(LOG_TAG, "%s UNSOLICITED: %s length:%zu", rilSocketIdToString(soc_id),
            requestToString(unsolResponse), datalen);
#endif

    /// release state lock @{
    if (unsolResponse == RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED) {
        pthread_mutex_unlock(&s_state_mutex[soc_id]);
    }
    /// @}

    if (ret != 0 && unsolResponse == RIL_UNSOL_NITZ_TIME_RECEIVED) {
        // Unfortunately, NITZ time is not poll/update like everything
        // else in the system. So, if the upstream client isn't connected,
        // keep a copy of the last NITZ response (with receive time noted
        // above) around so we can deliver it when it is connected

        if (s_lastNITZTimeData[(int)soc_id] != NULL) {
            free(s_lastNITZTimeData[(int)soc_id]);
            s_lastNITZTimeData[(int)soc_id] = NULL;
        }

        s_lastNITZTimeData[(int)soc_id] = calloc(datalen, 1);
        if (s_lastNITZTimeData[(int)soc_id] == NULL) {
            mtkLogE(LOG_TAG, "Memory allocation failed in RIL_onUnsolicitedResponse");
            goto error_exit;
        }
        s_lastNITZTimeDataSize[(int)soc_id] = datalen;
        memcpy(s_lastNITZTimeData[(int)soc_id], data, datalen);
    }

    // Normal exit
    return;

error_exit:
    if (shouldScheduleTimeout) {
        releaseWakeLock();
    }
}

/** FIXME generalize this if you track UserCAllbackInfo, clear it
    when the callback occurs
*/
static UserCallbackInfo *
internalRequestTimedCallback (RIL_TimedCallback callback, void *param,
                                const struct timeval *relativeTime)
{
    struct timeval myRelativeTime;
    UserCallbackInfo *p_info;

    p_info = (UserCallbackInfo *) calloc(1, sizeof(UserCallbackInfo));
    if (p_info == NULL) {
        mtkLogE(LOG_TAG, "Memory allocation failed in internalRequestTimedCallback");
        return p_info;

    }

    p_info->p_callback = callback;
    p_info->userParam = param;

    if (relativeTime == NULL) {
        /* treat null parameter as a 0 relative time */
        memset (&myRelativeTime, 0, sizeof(myRelativeTime));
    } else {
        /* FIXME I think event_add's tv param is really const anyway */
        memcpy (&myRelativeTime, relativeTime, sizeof(myRelativeTime));
    }

    ril_event_set(&(p_info->event), -1, false, userTimerCallback, p_info);

    ril_timer_add(&(p_info->event), &myRelativeTime);

    triggerEvLoop();
    return p_info;
}


extern "C" void
RIL_requestTimedCallback (RIL_TimedCallback callback, void *param,
                                const struct timeval *relativeTime) {
    internalRequestTimedCallback (callback, param, relativeTime);
}

const char *
failCauseToString(RIL_Errno e) {
    switch(e) {
        case RIL_E_SUCCESS: return "E_SUCCESS";
        case RIL_E_RADIO_NOT_AVAILABLE: return "E_RADIO_NOT_AVAILABLE";
        case RIL_E_GENERIC_FAILURE: return "E_GENERIC_FAILURE";
        case RIL_E_PASSWORD_INCORRECT: return "E_PASSWORD_INCORRECT";
        case RIL_E_SIM_PIN2: return "E_SIM_PIN2";
        case RIL_E_SIM_PUK2: return "E_SIM_PUK2";
        case RIL_E_REQUEST_NOT_SUPPORTED: return "E_REQUEST_NOT_SUPPORTED";
        case RIL_E_CANCELLED: return "E_CANCELLED";
        case RIL_E_OP_NOT_ALLOWED_DURING_VOICE_CALL: return "E_OP_NOT_ALLOWED_DURING_VOICE_CALL";
        case RIL_E_OP_NOT_ALLOWED_BEFORE_REG_TO_NW: return "E_OP_NOT_ALLOWED_BEFORE_REG_TO_NW";
        case RIL_E_SMS_SEND_FAIL_RETRY: return "E_SMS_SEND_FAIL_RETRY";
        case RIL_E_SIM_ABSENT:return "E_SIM_ABSENT";
        case RIL_E_ILLEGAL_SIM_OR_ME:return "E_ILLEGAL_SIM_OR_ME";
#ifdef FEATURE_MULTIMODE_ANDROID
        case RIL_E_SUBSCRIPTION_NOT_AVAILABLE:return "E_SUBSCRIPTION_NOT_AVAILABLE";
        case RIL_E_MODE_NOT_SUPPORTED:return "E_MODE_NOT_SUPPORTED";
#endif
        case RIL_E_FDN_CHECK_FAILURE: return "E_FDN_CHECK_FAILURE";
        case RIL_E_MISSING_RESOURCE: return "E_MISSING_RESOURCE";
        case RIL_E_NO_SUCH_ELEMENT: return "E_NO_SUCH_ELEMENT";
        case RIL_E_DIAL_MODIFIED_TO_USSD: return "E_DIAL_MODIFIED_TO_USSD";
        case RIL_E_DIAL_MODIFIED_TO_SS: return "E_DIAL_MODIFIED_TO_SS";
        case RIL_E_DIAL_MODIFIED_TO_DIAL: return "E_DIAL_MODIFIED_TO_DIAL";
        case RIL_E_USSD_MODIFIED_TO_DIAL: return "E_USSD_MODIFIED_TO_DIAL";
        case RIL_E_USSD_MODIFIED_TO_SS: return "E_USSD_MODIFIED_TO_SS";
        case RIL_E_USSD_MODIFIED_TO_USSD: return "E_USSD_MODIFIED_TO_USSD";
        case RIL_E_SS_MODIFIED_TO_DIAL: return "E_SS_MODIFIED_TO_DIAL";
        case RIL_E_SS_MODIFIED_TO_USSD: return "E_SS_MODIFIED_TO_USSD";
        case RIL_E_SUBSCRIPTION_NOT_SUPPORTED: return "E_SUBSCRIPTION_NOT_SUPPORTED";
        case RIL_E_SS_MODIFIED_TO_SS: return "E_SS_MODIFIED_TO_SS";
        case RIL_E_LCE_NOT_SUPPORTED: return "E_LCE_NOT_SUPPORTED";
        case RIL_E_NO_MEMORY: return "E_NO_MEMORY";
        case RIL_E_INTERNAL_ERR: return "E_INTERNAL_ERR";
        case RIL_E_SYSTEM_ERR: return "E_SYSTEM_ERR";
        case RIL_E_MODEM_ERR: return "E_MODEM_ERR";
        case RIL_E_INVALID_STATE: return "E_INVALID_STATE";
        case RIL_E_NO_RESOURCES: return "E_NO_RESOURCES";
        case RIL_E_SIM_ERR: return "E_SIM_ERR";
        case RIL_E_INVALID_ARGUMENTS: return "E_INVALID_ARGUMENTS";
        case RIL_E_INVALID_SIM_STATE: return "E_INVALID_SIM_STATE";
        case RIL_E_INVALID_MODEM_STATE: return "E_INVALID_MODEM_STATE";
        case RIL_E_INVALID_CALL_ID: return "E_INVALID_CALL_ID";
        case RIL_E_NO_SMS_TO_ACK: return "E_NO_SMS_TO_ACK";
        case RIL_E_NETWORK_ERR: return "E_NETWORK_ERR";
        case RIL_E_REQUEST_RATE_LIMITED: return "E_REQUEST_RATE_LIMITED";
        case RIL_E_SIM_BUSY: return "E_SIM_BUSY";
        case RIL_E_SIM_FULL: return "E_SIM_FULL";
        case RIL_E_NETWORK_REJECT: return "E_NETWORK_REJECT";
        case RIL_E_OPERATION_NOT_ALLOWED: return "E_OPERATION_NOT_ALLOWED";
        case RIL_E_EMPTY_RECORD: return "E_EMPTY_RECORD";
        case RIL_E_INVALID_SMS_FORMAT: return "E_INVALID_SMS_FORMAT";
        case RIL_E_ENCODING_ERR: return "E_ENCODING_ERR";
        case RIL_E_INVALID_SMSC_ADDRESS: return "E_INVALID_SMSC_ADDRESS";
        case RIL_E_NO_SUCH_ENTRY: return "E_NO_SUCH_ENTRY";
        case RIL_E_NETWORK_NOT_READY: return "E_NETWORK_NOT_READY";
        case RIL_E_NOT_PROVISIONED: return "E_NOT_PROVISIONED";
        case RIL_E_NO_SUBSCRIPTION: return "E_NO_SUBSCRIPTION";
        case RIL_E_NO_NETWORK_FOUND: return "E_NO_NETWORK_FOUND";
        case RIL_E_DEVICE_IN_USE: return "E_DEVICE_IN_USE";
        case RIL_E_ABORTED: return "E_ABORTED";
        case RIL_E_INVALID_RESPONSE: return "INVALID_RESPONSE";
        case RIL_E_OEM_ERROR_1: return "E_OEM_ERROR_1";
        case RIL_E_OEM_ERROR_2: return "E_OEM_ERROR_2";
        case RIL_E_OEM_ERROR_3: return "E_OEM_ERROR_3";
        case RIL_E_OEM_ERROR_4: return "E_OEM_ERROR_4";
        case RIL_E_OEM_ERROR_5: return "E_OEM_ERROR_5";
        case RIL_E_OEM_ERROR_6: return "E_OEM_ERROR_6";
        case RIL_E_OEM_ERROR_7: return "E_OEM_ERROR_7";
        case RIL_E_OEM_ERROR_8: return "E_OEM_ERROR_8";
        case RIL_E_OEM_ERROR_9: return "E_OEM_ERROR_9";
        case RIL_E_OEM_ERROR_10: return "E_OEM_ERROR_10";
        case RIL_E_OEM_ERROR_11: return "E_OEM_ERROR_11";
        case RIL_E_OEM_ERROR_12: return "E_OEM_ERROR_12";
        case RIL_E_OEM_ERROR_13: return "E_OEM_ERROR_13";
        case RIL_E_OEM_ERROR_14: return "E_OEM_ERROR_14";
        case RIL_E_OEM_ERROR_15: return "E_OEM_ERROR_15";
        case RIL_E_OEM_ERROR_16: return "E_OEM_ERROR_16";
        case RIL_E_OEM_ERROR_17: return "E_OEM_ERROR_17";
        case RIL_E_OEM_ERROR_18: return "E_OEM_ERROR_18";
        case RIL_E_OEM_ERROR_19: return "E_OEM_ERROR_19";
        case RIL_E_OEM_ERROR_20: return "E_OEM_ERROR_20";
        case RIL_E_OEM_ERROR_21: return "E_OEM_ERROR_21";
        case RIL_E_OEM_ERROR_22: return "E_OEM_ERROR_22";
        case RIL_E_OEM_ERROR_23: return "E_OEM_ERROR_23";
        case RIL_E_OEM_ERROR_24: return "E_OEM_ERROR_24";
        case RIL_E_OEM_ERROR_25: return "E_OEM_ERROR_25";
        default: return "<unknown error>";
    }
}

const char *
radioStateToString(RIL_RadioState s) {
    switch(s) {
        case RADIO_STATE_OFF: return "RADIO_OFF";
        case RADIO_STATE_UNAVAILABLE: return "RADIO_UNAVAILABLE";
        case RADIO_STATE_ON:return"RADIO_ON";
        default: return "<unknown state>";
    }
}

const char *
callStateToString(RIL_CallState s) {
    switch(s) {
        case RIL_CALL_ACTIVE : return "ACTIVE";
        case RIL_CALL_HOLDING: return "HOLDING";
        case RIL_CALL_DIALING: return "DIALING";
        case RIL_CALL_ALERTING: return "ALERTING";
        case RIL_CALL_INCOMING: return "INCOMING";
        case RIL_CALL_WAITING: return "WAITING";
        default: return "<unknown state>";
    }
}

const char *
requestToString(int request) {
/*
 cat libs/telephony/ril_commands.h \
 | egrep "^ *{RIL_" \
 | sed -re 's/\{RIL_([^,]+),[^,]+,([^}]+).+/case RIL_\1: return "\1";/'


 cat libs/telephony/ril_unsol_commands.h \
 | egrep "^ *{RIL_" \
 | sed -re 's/\{RIL_([^,]+),([^}]+).+/case RIL_\1: return "\1";/'

*/
    switch(request) {
        case RIL_REQUEST_GET_SIM_STATUS: return "GET_SIM_STATUS";
        case RIL_REQUEST_ENTER_SIM_PIN: return "ENTER_SIM_PIN";
        case RIL_REQUEST_ENTER_SIM_PUK: return "ENTER_SIM_PUK";
        case RIL_REQUEST_ENTER_SIM_PIN2: return "ENTER_SIM_PIN2";
        case RIL_REQUEST_ENTER_SIM_PUK2: return "ENTER_SIM_PUK2";
        case RIL_REQUEST_CHANGE_SIM_PIN: return "CHANGE_SIM_PIN";
        case RIL_REQUEST_CHANGE_SIM_PIN2: return "CHANGE_SIM_PIN2";
        case RIL_REQUEST_ENTER_NETWORK_DEPERSONALIZATION: return "ENTER_NETWORK_DEPERSONALIZATION";
        case RIL_REQUEST_GET_CURRENT_CALLS: return "GET_CURRENT_CALLS";
        case RIL_REQUEST_DIAL: return "DIAL";
        case RIL_REQUEST_EMERGENCY_DIAL: return "EMERGENCY_DIAL";
        case RIL_REQUEST_GET_IMSI: return "GET_IMSI";
        case RIL_REQUEST_HANGUP: return "HANGUP";
        case RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND: return "HANGUP_WAITING_OR_BACKGROUND";
        case RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND: return "HANGUP_FOREGROUND_RESUME_BACKGROUND";
        case RIL_REQUEST_HANGUP_WITH_REASON: return "HANGUP_WITH_REASON";
        case RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE: return "SWITCH_WAITING_OR_HOLDING_AND_ACTIVE";
        case RIL_REQUEST_CONFERENCE: return "CONFERENCE";
        case RIL_REQUEST_UDUB: return "UDUB";
        case RIL_REQUEST_LAST_CALL_FAIL_CAUSE: return "LAST_CALL_FAIL_CAUSE";
        case RIL_REQUEST_SIGNAL_STRENGTH: return "SIGNAL_STRENGTH";
        case RIL_REQUEST_VOICE_REGISTRATION_STATE: return "VOICE_REGISTRATION_STATE";
        case RIL_REQUEST_DATA_REGISTRATION_STATE: return "DATA_REGISTRATION_STATE";
        case RIL_REQUEST_OPERATOR: return "OPERATOR";
        case RIL_REQUEST_RADIO_POWER: return "RADIO_POWER";
        case RIL_REQUEST_DTMF: return "DTMF";
        case RIL_REQUEST_SEND_SMS: return "SEND_SMS";
        case RIL_REQUEST_SEND_SMS_EXPECT_MORE: return "SEND_SMS_EXPECT_MORE";
        case RIL_REQUEST_SETUP_DATA_CALL: return "SETUP_DATA_CALL";
        case RIL_REQUEST_SIM_IO: return "SIM_IO";
        case RIL_REQUEST_SEND_USSD: return "SEND_USSD";
        case RIL_REQUEST_SEND_USSI: return "SEND_USSI";
        case RIL_REQUEST_CANCEL_USSD: return "CANCEL_USSD";
        case RIL_REQUEST_CANCEL_USSI: return "CANCEL_USSI";
        case RIL_REQUEST_GET_CLIR: return "GET_CLIR";
        case RIL_REQUEST_SET_CLIR: return "SET_CLIR";
        case RIL_REQUEST_QUERY_CALL_FORWARD_STATUS: return "QUERY_CALL_FORWARD_STATUS";
        case RIL_REQUEST_SET_CALL_FORWARD: return "SET_CALL_FORWARD";
        case RIL_REQUEST_QUERY_CALL_WAITING: return "QUERY_CALL_WAITING";
        case RIL_REQUEST_SET_CALL_WAITING: return "SET_CALL_WAITING";
        case RIL_REQUEST_SMS_ACKNOWLEDGE: return "SMS_ACKNOWLEDGE";
        case RIL_REQUEST_GET_IMEI: return "GET_IMEI";
        case RIL_REQUEST_GET_IMEISV: return "GET_IMEISV";
        case RIL_REQUEST_ANSWER: return "ANSWER";
        case RIL_REQUEST_DEACTIVATE_DATA_CALL: return "DEACTIVATE_DATA_CALL";
        case RIL_REQUEST_QUERY_FACILITY_LOCK: return "QUERY_FACILITY_LOCK";
        case RIL_REQUEST_SET_FACILITY_LOCK: return "SET_FACILITY_LOCK";
        case RIL_REQUEST_CHANGE_BARRING_PASSWORD: return "CHANGE_BARRING_PASSWORD";
        case RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE: return "QUERY_NETWORK_SELECTION_MODE";
        case RIL_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC: return "SET_NETWORK_SELECTION_AUTOMATIC";
        case RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL: return "SET_NETWORK_SELECTION_MANUAL";
        case RIL_REQUEST_QUERY_AVAILABLE_NETWORKS: return "QUERY_AVAILABLE_NETWORKS";
        case RIL_REQUEST_DTMF_START: return "DTMF_START";
        case RIL_REQUEST_DTMF_STOP: return "DTMF_STOP";
        case RIL_REQUEST_BASEBAND_VERSION: return "BASEBAND_VERSION";
        case RIL_REQUEST_SEPARATE_CONNECTION: return "SEPARATE_CONNECTION";
        case RIL_REQUEST_SET_MUTE: return "SET_MUTE";
        case RIL_REQUEST_GET_MUTE: return "GET_MUTE";
        case RIL_REQUEST_QUERY_CLIP: return "QUERY_CLIP";
        case RIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE: return "LAST_DATA_CALL_FAIL_CAUSE";
        case RIL_REQUEST_DATA_CALL_LIST: return "DATA_CALL_LIST";
        case RIL_REQUEST_RESET_RADIO: return "RESET_RADIO";
        case RIL_REQUEST_OEM_HOOK_RAW: return "OEM_HOOK_RAW";
        case RIL_REQUEST_OEM_HOOK_STRINGS: return "OEM_HOOK_STRINGS";
        case RIL_REQUEST_SCREEN_STATE: return "SCREEN_STATE";
        case RIL_REQUEST_SET_SUPP_SVC_NOTIFICATION: return "SET_SUPP_SVC_NOTIFICATION";
        case RIL_REQUEST_WRITE_SMS_TO_SIM: return "WRITE_SMS_TO_SIM";
        case RIL_REQUEST_DELETE_SMS_ON_SIM: return "DELETE_SMS_ON_SIM";
        case RIL_REQUEST_SET_BAND_MODE: return "SET_BAND_MODE";
        case RIL_REQUEST_QUERY_AVAILABLE_BAND_MODE: return "QUERY_AVAILABLE_BAND_MODE";
        case RIL_REQUEST_STK_GET_PROFILE: return "STK_GET_PROFILE";
        case RIL_REQUEST_STK_SET_PROFILE: return "STK_SET_PROFILE";
        case RIL_REQUEST_STK_SEND_ENVELOPE_COMMAND: return "STK_SEND_ENVELOPE_COMMAND";
        case RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE: return "STK_SEND_TERMINAL_RESPONSE";
        case RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM: return "STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM";
        case RIL_REQUEST_EXPLICIT_CALL_TRANSFER: return "EXPLICIT_CALL_TRANSFER";
        case RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE: return "SET_PREFERRED_NETWORK_TYPE";
        case RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE: return "GET_PREFERRED_NETWORK_TYPE";
        case RIL_REQUEST_GET_NEIGHBORING_CELL_IDS: return "GET_NEIGHBORING_CELL_IDS";
        case RIL_REQUEST_SET_LOCATION_UPDATES: return "SET_LOCATION_UPDATES";
        case RIL_REQUEST_CDMA_SET_SUBSCRIPTION_SOURCE: return "CDMA_SET_SUBSCRIPTION_SOURCE";
        case RIL_REQUEST_CDMA_SET_ROAMING_PREFERENCE: return "CDMA_SET_ROAMING_PREFERENCE";
        case RIL_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE: return "CDMA_QUERY_ROAMING_PREFERENCE";
        case RIL_REQUEST_SET_TTY_MODE: return "SET_TTY_MODE";
        case RIL_REQUEST_QUERY_TTY_MODE: return "QUERY_TTY_MODE";
        case RIL_REQUEST_CDMA_SET_PREFERRED_VOICE_PRIVACY_MODE: return "CDMA_SET_PREFERRED_VOICE_PRIVACY_MODE";
        case RIL_REQUEST_CDMA_QUERY_PREFERRED_VOICE_PRIVACY_MODE: return "CDMA_QUERY_PREFERRED_VOICE_PRIVACY_MODE";
        case RIL_REQUEST_CDMA_FLASH: return "CDMA_FLASH";
        case RIL_REQUEST_CDMA_BURST_DTMF: return "CDMA_BURST_DTMF";
        case RIL_REQUEST_CDMA_VALIDATE_AND_WRITE_AKEY: return "CDMA_VALIDATE_AND_WRITE_AKEY";
        case RIL_REQUEST_CDMA_SEND_SMS: return "CDMA_SEND_SMS";
        case RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE: return "CDMA_SMS_ACKNOWLEDGE";
        case RIL_REQUEST_GSM_GET_BROADCAST_SMS_CONFIG: return "GSM_GET_BROADCAST_SMS_CONFIG";
        case RIL_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG: return "GSM_SET_BROADCAST_SMS_CONFIG";
        case RIL_REQUEST_GSM_SMS_BROADCAST_ACTIVATION: return "GSM_SMS_BROADCAST_ACTIVATION";
        case RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG: return "CDMA_GET_BROADCAST_SMS_CONFIG";
        case RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG: return "CDMA_SET_BROADCAST_SMS_CONFIG";
        case RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION: return "CDMA_SMS_BROADCAST_ACTIVATION";
        case RIL_REQUEST_CDMA_SUBSCRIPTION: return "CDMA_SUBSCRIPTION";
        case RIL_REQUEST_CDMA_WRITE_SMS_TO_RUIM: return "CDMA_WRITE_SMS_TO_RUIM";
        case RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM: return "CDMA_DELETE_SMS_ON_RUIM";
        case RIL_REQUEST_DEVICE_IDENTITY: return "DEVICE_IDENTITY";
        case RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE: return "EXIT_EMERGENCY_CALLBACK_MODE";
        case RIL_REQUEST_GET_SMSC_ADDRESS: return "GET_SMSC_ADDRESS";
        case RIL_REQUEST_SET_SMSC_ADDRESS: return "SET_SMSC_ADDRESS";
        case RIL_REQUEST_REPORT_SMS_MEMORY_STATUS: return "REPORT_SMS_MEMORY_STATUS";
        case RIL_REQUEST_REPORT_STK_SERVICE_IS_RUNNING: return "REPORT_STK_SERVICE_IS_RUNNING";
        case RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE: return "CDMA_GET_SUBSCRIPTION_SOURCE";
        case RIL_REQUEST_ISIM_AUTHENTICATION: return "ISIM_AUTHENTICATION";
        case RIL_REQUEST_ACKNOWLEDGE_INCOMING_GSM_SMS_WITH_PDU: return "ACKNOWLEDGE_INCOMING_GSM_SMS_WITH_PDU";
        case RIL_REQUEST_STK_SEND_ENVELOPE_WITH_STATUS: return "STK_SEND_ENVELOPE_WITH_STATUS";
        case RIL_REQUEST_VOICE_RADIO_TECH: return "VOICE_RADIO_TECH";
        case RIL_REQUEST_GET_CELL_INFO_LIST: return "GET_CELL_INFO_LIST";
        case RIL_REQUEST_SET_UNSOL_CELL_INFO_LIST_RATE: return "SET_UNSOL_CELL_INFO_LIST_RATE";
        case RIL_REQUEST_SET_INITIAL_ATTACH_APN: return "SET_INITIAL_ATTACH_APN";
        case RIL_REQUEST_IMS_REGISTRATION_STATE: return "IMS_REGISTRATION_STATE";
        case RIL_REQUEST_IMS_SEND_SMS: return "IMS_SEND_SMS";
        case RIL_REQUEST_SIM_TRANSMIT_APDU_BASIC: return "SIM_TRANSMIT_APDU_BASIC";
        case RIL_REQUEST_SIM_OPEN_CHANNEL: return "SIM_OPEN_CHANNEL";
        case RIL_REQUEST_SIM_CLOSE_CHANNEL: return "SIM_CLOSE_CHANNEL";
        case RIL_REQUEST_SIM_TRANSMIT_APDU_CHANNEL: return "SIM_TRANSMIT_APDU_CHANNEL";
        case RIL_REQUEST_GET_SLOT_STATUS: return "REQUEST_GET_SLOT_STATUS";
        case RIL_REQUEST_SET_LOGICAL_TO_PHYSICAL_SLOT_MAPPING: return "REQUEST_SET_LOGICAL_TO_PHYSICAL_SLOT_MAPPING";
        case RIL_REQUEST_NV_READ_ITEM: return "NV_READ_ITEM";
        case RIL_REQUEST_NV_WRITE_ITEM: return "NV_WRITE_ITEM";
        case RIL_REQUEST_NV_WRITE_CDMA_PRL: return "NV_WRITE_CDMA_PRL";
        case RIL_REQUEST_NV_RESET_CONFIG: return "NV_RESET_CONFIG";
        case RIL_REQUEST_SET_UICC_SUBSCRIPTION: return "SET_UICC_SUBSCRIPTION";
        case RIL_REQUEST_ALLOW_DATA: return "ALLOW_DATA";
        case RIL_REQUEST_GET_HARDWARE_CONFIG: return "GET_HARDWARE_CONFIG";
        case RIL_REQUEST_SIM_AUTHENTICATION: return "SIM_AUTHENTICATION";
        case RIL_REQUEST_GET_DC_RT_INFO: return "GET_DC_RT_INFO";
        case RIL_REQUEST_SET_DC_RT_INFO_RATE: return "SET_DC_RT_INFO_RATE";
        case RIL_REQUEST_SET_DATA_PROFILE: return "SET_DATA_PROFILE";
        case RIL_REQUEST_SHUTDOWN: return "SHUTDOWN";
        case RIL_REQUEST_GET_RADIO_CAPABILITY: return "GET_RADIO_CAPABILITY";
        case RIL_REQUEST_SET_RADIO_CAPABILITY: return "SET_RADIO_CAPABILITY";
        case RIL_REQUEST_START_LCE: return "START_LCE";
        case RIL_REQUEST_STOP_LCE: return "STOP_LCE";
        case RIL_REQUEST_PULL_LCEDATA: return "PULL_LCEDATA";
        case RIL_REQUEST_GET_ACTIVITY_INFO: return "GET_ACTIVITY_INFO";
        case RIL_REQUEST_SET_ALLOWED_CARRIERS: return "SET_ALLOWED_CARRIERS";
        case RIL_REQUEST_GET_ALLOWED_CARRIERS: return "GET_ALLOWED_CARRIERS";
        case RIL_REQUEST_SET_CARRIER_INFO_IMSI_ENCRYPTION: return "SET_CARRIER_INFO_IMSI_ENCRYPTION";
        case RIL_REQUEST_SEND_DEVICE_STATE: return "SEND_DEVICE_STATE";
        case RIL_REQUEST_SET_UNSOLICITED_RESPONSE_FILTER: return "SET_UNSOLICITED_RESPONSE_FILTER";
        case RIL_REQUEST_SET_SIM_CARD_POWER: return "SET_SIM_CARD_POWER";
        case RIL_REQUEST_SET_SIM_POWER: return "SET_SIM_POWER";
        case RIL_REQUEST_START_NETWORK_SCAN: return "START_NETWORK_SCAN";
        case RIL_REQUEST_STOP_NETWORK_SCAN: return "STOP_NETWORK_SCAN";
        case RIL_REQUEST_SETPROP_IMS_HANDOVER: return "RIL_REQUEST_SETPROP_IMS_HANDOVER";
        case RIL_REQUEST_SET_LINK_CAPACITY_REPORTING_CRITERIA: return "SET_LINK_CAPACITY_REPORTING_CRITERIA";
        case RIL_REQUEST_SET_SS_PROPERTY: return "SET_SS_PROPERTY";
        case RIL_REQUEST_ACTIVATE_UICC_CARD: return "ACTIVATE_UICC_CARD";
        case RIL_REQUEST_DEACTIVATE_UICC_CARD: return "DEACTIVATE_UICC_CARD";
        case RIL_REQUEST_GET_CURRENT_UICC_CARD_PROVISIONING_STATUS: return "GET_CURRENT_UICC_CARD_PROVISIONING_STATUS";
        case RIL_RESPONSE_ACKNOWLEDGEMENT: return "RESPONSE_ACKNOWLEDGEMENT";
        case RIL_REQUEST_SET_PREFERRED_DATA_MODEM: return "RIL_REQUEST_SET_PREFERRED_DATA_MODEM";
        case RIL_REQUEST_ENABLE_DSDA_INDICATION: return "RIL_REQUEST_ENABLE_DSDA_INDICATION";
        case RIL_REQUEST_GET_DSDA_STATUS: return "RIL_REQUEST_GET_DSDA_STATUS";
        case RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED: return "UNSOL_RESPONSE_RADIO_STATE_CHANGED";
        case RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED: return "UNSOL_RESPONSE_CALL_STATE_CHANGED";
        case RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED: return "UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED";
        case RIL_UNSOL_RESPONSE_NEW_SMS: return "UNSOL_RESPONSE_NEW_SMS";
        case RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT: return "UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT";
        case RIL_UNSOL_RESPONSE_NEW_SMS_ON_SIM: return "UNSOL_RESPONSE_NEW_SMS_ON_SIM";
        case RIL_UNSOL_ON_USSD: return "UNSOL_ON_USSD";
        case RIL_UNSOL_ON_USSD_REQUEST: return "UNSOL_ON_USSD_REQUEST";
        case RIL_UNSOL_NITZ_TIME_RECEIVED: return "UNSOL_NITZ_TIME_RECEIVED";
        case RIL_UNSOL_SIGNAL_STRENGTH: return "UNSOL_SIGNAL_STRENGTH";
        case RIL_UNSOL_DATA_CALL_LIST_CHANGED: return "UNSOL_DATA_CALL_LIST_CHANGED";
        case RIL_UNSOL_SUPP_SVC_NOTIFICATION: return "UNSOL_SUPP_SVC_NOTIFICATION";
        case RIL_UNSOL_SUPP_SVC_NOTIFICATION_EX: return "UNSOL_SUPP_SVC_NOTIFICATION_EX";
        case RIL_UNSOL_STK_SESSION_END: return "UNSOL_STK_SESSION_END";
        case RIL_UNSOL_STK_PROACTIVE_COMMAND: return "UNSOL_STK_PROACTIVE_COMMAND";
        case RIL_UNSOL_STK_EVENT_NOTIFY: return "UNSOL_STK_EVENT_NOTIFY";
        case RIL_UNSOL_STK_CALL_SETUP: return "UNSOL_STK_CALL_SETUP";
        case RIL_UNSOL_SIM_SMS_STORAGE_FULL: return "UNSOL_SIM_SMS_STORAGE_FULL";
        case RIL_UNSOL_SIM_REFRESH: return "UNSOL_SIM_REFRESH";
        case RIL_UNSOL_CALL_RING: return "UNSOL_CALL_RING";
        case RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED: return "UNSOL_RESPONSE_SIM_STATUS_CHANGED";
        case RIL_UNSOL_ICC_SLOT_STATUS: return "UNSOL_ICC_SLOT_STATUS";
        case RIL_UNSOL_RESPONSE_CDMA_NEW_SMS: return "UNSOL_RESPONSE_CDMA_NEW_SMS";
        case RIL_UNSOL_RESPONSE_NEW_BROADCAST_SMS: return "UNSOL_RESPONSE_NEW_BROADCAST_SMS";
        case RIL_UNSOL_CDMA_RUIM_SMS_STORAGE_FULL: return "UNSOL_CDMA_RUIM_SMS_STORAGE_FULL";
        case RIL_UNSOL_RESTRICTED_STATE_CHANGED: return "UNSOL_RESTRICTED_STATE_CHANGED";
        case RIL_UNSOL_ENTER_EMERGENCY_CALLBACK_MODE: return "UNSOL_ENTER_EMERGENCY_CALLBACK_MODE";
        case RIL_UNSOL_CDMA_CALL_WAITING: return "UNSOL_CDMA_CALL_WAITING";
        case RIL_UNSOL_CDMA_OTA_PROVISION_STATUS: return "UNSOL_CDMA_OTA_PROVISION_STATUS";
        case RIL_UNSOL_CDMA_INFO_REC: return "UNSOL_CDMA_INFO_REC";
        case RIL_UNSOL_OEM_HOOK_RAW: return "UNSOL_OEM_HOOK_RAW";
        case RIL_UNSOL_RINGBACK_TONE: return "UNSOL_RINGBACK_TONE";
        case RIL_UNSOL_RESEND_INCALL_MUTE: return "UNSOL_RESEND_INCALL_MUTE";
        case RIL_UNSOL_CDMA_SUBSCRIPTION_SOURCE_CHANGED: return "UNSOL_CDMA_SUBSCRIPTION_SOURCE_CHANGED";
        case RIL_UNSOL_CDMA_PRL_CHANGED: return "UNSOL_CDMA_PRL_CHANGED";
        case RIL_UNSOL_EXIT_EMERGENCY_CALLBACK_MODE: return "UNSOL_EXIT_EMERGENCY_CALLBACK_MODE";
        case RIL_UNSOL_RIL_CONNECTED: return "UNSOL_RIL_CONNECTED";
        case RIL_UNSOL_VOICE_RADIO_TECH_CHANGED: return "UNSOL_VOICE_RADIO_TECH_CHANGED";
        case RIL_UNSOL_CELL_INFO_LIST: return "UNSOL_CELL_INFO_LIST";
        case RIL_UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED: return "UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED";
        case RIL_UNSOL_UICC_SUBSCRIPTION_STATUS_CHANGED: return "UNSOL_UICC_SUBSCRIPTION_STATUS_CHANGED";
        case RIL_UNSOL_SRVCC_STATE_NOTIFY: return "UNSOL_SRVCC_STATE_NOTIFY";
        case RIL_UNSOL_HARDWARE_CONFIG_CHANGED: return "UNSOL_HARDWARE_CONFIG_CHANGED";
        case RIL_UNSOL_DC_RT_INFO_CHANGED: return "UNSOL_DC_RT_INFO_CHANGED";
        case RIL_UNSOL_RADIO_CAPABILITY: return "UNSOL_RADIO_CAPABILITY";
        case RIL_UNSOL_MODEM_RESTART: return "UNSOL_MODEM_RESTART";
        case RIL_UNSOL_CARRIER_INFO_IMSI_ENCRYPTION: return "UNSOL_CARRIER_INFO_IMSI_ENCRYPTION";
        case RIL_UNSOL_ON_SS: return "UNSOL_ON_SS";
        case RIL_UNSOL_STK_CC_ALPHA_NOTIFY: return "UNSOL_STK_CC_ALPHA_NOTIFY";
        case RIL_UNSOL_LCEDATA_RECV: return "UNSOL_LCEDATA_RECV";
        case RIL_UNSOL_PCO_DATA: return "UNSOL_PCO_DATA";
        case RIL_UNSOL_PCO_DATA_AFTER_ATTACHED: return "UNSOL_PCO_DATA_AFTER_ATTACHED";
        case RIL_UNSOL_NETWORK_SCAN_RESULT: return "UNSOL_NETWORK_SCAN_RESULT";
        case RIL_UNSOL_NETWORK_REJECT_CAUSE: return "UNSOL_NETWORK_REJECT_CAUSE";
        case RIL_UNSOL_IMS_BEARER_STATE_NOTIFY: return "RIL_UNSOL_IMS_BEARER_STATE_NOTIFY";
        case RIL_UNSOL_IMS_DATA_INFO_NOTIFY: return "RIL_UNSOL_IMS_DATA_INFO_NOTIFY";
        case RIL_UNSOL_CARD_DETECTED_IND: return "UNSOL_CARD_DETECTED_IND";
        case RIL_UNSOL_ON_DSDA_CHANGED: return "RIL_UNSOL_ON_DSDA_CHANGED";
        case RIL_UNSOL_QUALIFIED_NETWORK_TYPES_CHANGED: return "UNSOL_QUALIFIED_NETWORK_TYPES_CHANGED";
        default: return mtkRequestToString(request);
    }
}

const char *
rilSocketIdToString(RIL_SOCKET_ID socket_id)
{
    switch(socket_id) {
        case RIL_SOCKET_1:
            return "RIL_SOCKET_1";
        case RIL_SOCKET_2:
            return "RIL_SOCKET_2";
        case RIL_SOCKET_3:
            return "RIL_SOCKET_3";
        case RIL_SOCKET_4:
            return "RIL_SOCKET_4";
        default:
            return "not a valid RIL";
    }
}

// M: ril proxy
bool cacheUrc(int unsolResponse, const void *data, size_t datalen, RIL_SOCKET_ID socket_id){
    ClientType targetClient = RADIO_AOSP;
    //Only the URC list we wanted.
    int division = CLIENT_RILJ;
    bool ret = true;
    // check input
    if(socket_id >= RIL_SOCKET_NUM) {
        mtkLogD(LOG_TAG, "incorrect input %d", socket_id);
        return false;
    }
    switch(unsolResponse) {
        // Need to cached RILJ URCs
        // Warning!
        // Warning!!
        // Warning!!!
        // If the data has pointer members which point to new buffer(it may be created by calloc or
        // malloc), this method will not cache these pointer members, so it will cause the native
        // error of "use after free" when you use these pointer members.
        // Warning!!!
        // Warning!!
        // Warning!
        case RIL_UNSOL_SMS_READY_NOTIFICATION:
        case RIL_UNSOL_SIM_SMS_STORAGE_FULL:
        case RIL_UNSOL_CDMA_RUIM_SMS_STORAGE_FULL:
        case RIL_UNSOL_PHB_READY_NOTIFICATION:
        case RIL_UNSOL_ME_SMS_STORAGE_FULL:
        case RIL_UNSOL_RADIO_CAPABILITY:
        case RIL_LOCAL_C2K_UNSOL_EF_ECC:
        case RIL_LOCAL_GSM_UNSOL_EF_ECC:
        case RIL_UNSOL_CDMA_CARD_INITIAL_ESN_OR_MEID:
        case RIL_UNSOL_VOICE_RADIO_TECH_CHANGED:
        case RIL_UNSOL_WORLD_MODE_CHANGED:
        case RIL_UNSOL_SIM_PLUG_IN:
        case RIL_UNSOL_SIM_PLUG_OUT:
        case RIL_UNSOL_CALL_FORWARDING:
        case RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED:
        case RIL_UNSOL_RESPONSE_PS_NETWORK_STATE_CHANGED:
        // External SIM [Start]
        case RIL_UNSOL_VSIM_OPERATION_INDICATION:
        // External SIM [End]
        case RIL_UNSOL_HARDWARE_CONFIG_CHANGED:
        case RIL_UNSOL_TX_POWER:
        case RIL_UNSOL_DSBP_STATE_CHANGED:
        case RIL_UNSOL_SIM_SLOT_LOCK_POLICY_NOTIFY:
        case RIL_UNSOL_SIM_POWER_CHANGED:
        case RIL_UNSOL_CARD_DETECTED_IND:
        case RIL_UNSOL_ECC_NUM:
        case RIL_UNSOL_EXIT_EMERGENCY_CALLBACK_MODE:
        case RIL_UNSOL_SRVCC_STATE_NOTIFY:
        case RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED:
            if (unsolResponse >= RIL_UNSOL_VENDOR_BASE) {
                targetClient = RADIO_MTK;
            } else {
                targetClient = RADIO_AOSP;
            }
            break;
        // Need to cached IMS URCs
        case RIL_UNSOL_IMS_REGISTRATION_INFO:
        case RIL_UNSOL_IMS_ENABLE_DONE:
        case RIL_UNSOL_IMS_DISABLE_DONE:
        case RIL_UNSOL_IMS_ENABLE_START:
        case RIL_UNSOL_IMS_DISABLE_START:
        case RIL_UNSOL_GET_PROVISION_DONE:
        case RIL_UNSOL_VOLTE_SETTING:
        case RIL_UNSOL_IMS_MULTIIMS_COUNT:
        case RIL_UNSOL_IMS_SUPPORT_ECC:
        case RIL_UNSOL_IMS_BEARER_INIT:
        case RIL_UNSOL_ON_VOLTE_SUBSCRIPTION:
            if (unsolResponse >= RIL_UNSOL_VENDOR_BASE) {
                targetClient = IMS_MTK;
            } else {
                targetClient = IMS_AOSP;
            }
            division = CLIENT_IMS;
            break;
        default:
            mtkLogD(LOG_TAG, "No need to cache the request");
            return false;
    }
    // check if we need to cache it
    AtResponseList* urcCur = pendedUrcList[socket_id][targetClient];
    AtResponseList* urcPrev = NULL;
    int pendedUrcCount = 0;
    pthread_mutex_lock(&(s_pendingUrcMutex[socket_id][targetClient]));
    if(bIsCallbackReady[socket_id][targetClient] == true) {
        mtkLogD(LOG_TAG, "already connected, no need to cache");
        ret = false;
        goto unlock;
    }
    while (urcCur != NULL) {
        mtkLogD(LOG_TAG, "Pended URC:%d, RILD:%s, :%s, client %d",
            pendedUrcCount, rilSocketIdToString(socket_id),
            requestToString(urcCur->id), targetClient);
        urcPrev = urcCur;
        urcCur = urcCur->pNext;
        pendedUrcCount++;
    }
    urcCur = (AtResponseList*)calloc(1, sizeof(AtResponseList));
    if (urcCur == NULL) {
        mtkLogE(LOG_TAG, "OOM");
        ret = false;
        goto unlock;
    }
    if (urcPrev != NULL) {
        urcPrev->pNext = urcCur;
    }
    urcCur->pNext = NULL;
    urcCur->id = unsolResponse;
    urcCur->datalen = datalen;
    urcCur->data = (char*)calloc(1, datalen + 1);
    if (urcCur->data == NULL) {
        mtkLogE(LOG_TAG, "OOM");
        free(urcCur);
        ret = false;
        goto unlock;
    }
    memcpy(urcCur->data, data, datalen);
    if (pendedUrcCount == 0) {
        pendedUrcList[socket_id][targetClient] = urcCur;
    }
    mtkLogD(LOG_TAG, "New pended URC:%d, RILD:%s, :%s, client %d",
            pendedUrcCount + 1, rilSocketIdToString(socket_id),
            requestToString(urcCur->id), targetClient);
unlock :
    pthread_mutex_unlock(&(s_pendingUrcMutex[socket_id][targetClient]));
    return ret;
}

void sendUrc(RIL_SOCKET_ID socket_id, AtResponseList** urcCached, android::ClientType clientType) {
    AtResponseList* urc = *urcCached;
    AtResponseList* urc_temp;

    pthread_mutex_lock(&(s_pendingUrcMutex[socket_id][clientType]));
    while (urc != NULL) {
        if (bIsCallbackReady[socket_id][clientType] == false) {
            mtkLogE(LOG_TAG, "CB is not ready, clientType %d, Socket %d", clientType, socket_id);
            break;
        }
        mtkLogI(LOG_TAG, "sendPendedUrcs %s, %s, client %d",
                rilSocketIdToString(socket_id), requestToString(urc->id), clientType);
    #if defined(ANDROID_MULTI_SIM)
        RIL_onUnsolicitedResponse(urc->id, urc->data, urc->datalen, socket_id);
    #else
        RIL_onUnsolicitedResponse(urc->id, urc->data, urc->datalen);
    #endif
        free(urc->data);
        urc_temp = urc;
        urc = urc->pNext;
        // update head of cache
        *urcCached = urc;
        free(urc_temp);
    }
    // clear the link list when finished
    *urcCached = urc;
    pthread_mutex_unlock(&(s_pendingUrcMutex[socket_id][clientType]));
}

void sendPendedUrcs(RIL_SOCKET_ID socket_id, android::ClientType clientType) {
    mtkLogD(LOG_TAG, "Ready to send pended URCs, socket:%s, clientType %d",
            rilSocketIdToString(socket_id), clientType);
    sendUrc(socket_id, &(pendedUrcList[socket_id][clientType]), clientType);
}

#ifdef HAVE_AEE_FEATURE
void triggerWarning(char *pErrMsg) {
    if (pErrMsg != NULL) {
        aee_system_warning("ril-proxy", NULL, DB_OPT_DEFAULT, pErrMsg);
        exit(0);
    } else {
        assert(0);
    }
}
#endif

/**
 * Convert divisions' slot Id to real SIM slot
 * @param slotId Divisions' slot Id
 * @retrun real slot Id
 */
extern "C" int toRealSlot(int slotId) {
    return slotId % MAX_SIM_COUNT;
}

/**
 * Convert real SIM slot to client Slot
 * @param slotId Real slot Id
 * @retrun client slot Id
 */
extern "C" int toClientSlot(int slotId, ClientId cliendId) {
    mtkLogD(LOG_TAG, "toClientSlot:%d to %d, client %d",
            slotId, (slotId % MAX_SIM_COUNT) + ((int)cliendId * MAX_SIM_COUNT), (int)cliendId);
    return (slotId % MAX_SIM_COUNT) + ((int)cliendId * MAX_SIM_COUNT);
}

/**
 * Get Division Id by slot Id
 * @param slotId Divisions' slot Id
 * @retrun Division Id
 */
extern "C" ClientId getClientBySlot(int slotId) {
    return (ClientId)(slotId / MAX_SIM_COUNT);
}

} /* namespace android */

