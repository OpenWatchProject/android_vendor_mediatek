/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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

/*****************************************************************************
 * Include
 *****************************************************************************/

#include <string.h>
#include <iostream>
#include <thread>
#include <cutils/sockets.h>
#include <cutils/jstring.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include "RilClient.h"
#include "Rfx.h"
#include "RtcRilClientController.h"
#include "RfxLog.h"
#include "rfx_properties.h"
#include <telephony/mtk_ril.h>
#include <libmtkrilutils.h>
#include "RilParcelUtils.h"
#include "RilCommandsTable.h"
/*****************************************************************************
 * Class RfxController
 *****************************************************************************/

#define RFX_LOG_TAG "RilClient"
#define NUM_ELEMS(a)  (sizeof (a) / sizeof (a)[0])

using namespace std;
using namespace android;

#if RILC_LOG
    static char printBuf[PRINTBUF_SIZE];
#endif

static Mutex sMutex;

RilClient::RilClient(int identity, char* socketName) {
    this->identity = identity;
    this->socketName = socketName;
    this->commandFd = -1;
    this->listenFd = -1;
    this->clientState = CLIENT_STATE_UNKNOWN;
    this->stream = NULL;
    activityThread = new StateActivityThread(this);
    activityThread->run("StateThread");
    setClientState(CLIENT_INITIALIZING);

    mPendingUrc = (UrcList**)calloc(1, sizeof(UrcList*)*getSimCount());
    if (mPendingUrc == NULL) {
        RFX_LOG_E(RFX_LOG_TAG,"OOM");
        return;
    }
    for (int i = 0; i < getSimCount(); i++) {
        mPendingUrc[i] = NULL;
    }
    RFX_LOG_D(RFX_LOG_TAG, "init done");
}

RilClient::~RilClient() {
    if (mPendingUrc != NULL) {
        for (int i = 0; i < getSimCount(); i++) {
            if (mPendingUrc[i] != NULL) {
                // free each data
                UrcList* urc = mPendingUrc[i];
                UrcList* urcTemp;
                while (urc != NULL) {
                    free(urc->data);
                    urcTemp = urc;
                    urc = urc->pNext;
                    free(urcTemp);
                }
                free(mPendingUrc[i]);
                mPendingUrc[i] = NULL;
            }
        }
        free(mPendingUrc);
    }
}

void RilClient::clientStateCallback() {
    RFX_LOG_D(RFX_LOG_TAG, "Enter callback %s", clientStateToString(clientState));
    switch(clientState) {
        case CLIENT_INITIALIZING:
            handleStateInitializing();
            break;
        case CLIENT_ACTIVE:
            handleStateActive();
            break;
        case CLIENT_DEACTIVE:
            handleStateDeactive();
            break;
        case CLIENT_CLOSED:
            handleStateClosed();
            break;
        default:
            break;
    }
}

void RilClient::handleStateInitializing() {

    int ret;
    char* socketName = this->socketName;
    struct sockaddr_un my_addr;
    struct sockaddr_un peer_addr;

    if (listenFd < 0) {
        listenFd = android_get_control_socket(socketName);
    }

    //do retry if init.rc didn't define socket
    if (listenFd < 0) {
        RFX_LOG_D(RFX_LOG_TAG, "init.rc didn't define, create socket manually");
        memset(&my_addr, 0, sizeof(struct sockaddr_un));
        my_addr.sun_family = AF_UNIX;

        char path[256];
        sprintf (path, "%s", socketName);
        RFX_LOG_D(RFX_LOG_TAG, "socketName is %s", path);
        my_addr.sun_path[0] = 0;
        strncpy(&(my_addr.sun_path[1]), path,
            sizeof(my_addr.sun_path) - 1);

        listenFd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (listenFd < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "manually listen fail, closed");
            setClientState(CLIENT_CLOSED);
            return;
        }

        int ret = ::bind(listenFd, (struct sockaddr *) &my_addr,
                sizeof(struct sockaddr_un));
        if (ret < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "bind fail, ret = %d, errno = %d, set state to close", ret, errno);
            close(listenFd);
            listenFd = -1;
        }
    }

    if (listenFd < 0) {
        RFX_LOG_D(RFX_LOG_TAG, "Failed to get socket %s, %s", socketName, strerror(errno));
        setClientState(CLIENT_CLOSED);
        return;
    }

    RFX_LOG_I(RFX_LOG_TAG, "start listen on fd: %d, socket name: %s", listenFd, socketName);
    ret = listen(listenFd, 4);
    if (ret < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "Failed to listen on control socket '%d': %s",
             listenFd, strerror(errno));
        close(listenFd);
        listenFd = -1;
        setClientState(CLIENT_CLOSED);
        return;
    }

    socklen_t socklen = sizeof (peer_addr);
    commandFd = accept (listenFd,  (sockaddr *) &peer_addr, &socklen);
    RFX_LOG_I(RFX_LOG_TAG, "initialize: commandFd is %d", commandFd);

    if (commandFd < 0 ) {
        RFX_LOG_D(RFX_LOG_TAG, "Error on accept() errno:%d", errno);
        close(listenFd);
        listenFd = -1;
        setClientState(CLIENT_CLOSED);
        return;
    }
    RFX_LOG_D(RFX_LOG_TAG, "set client state active");
    setClientState(CLIENT_ACTIVE);
}

#define MAX_COMMAND_BYTES (8 * 1024)
void RilClient::handleStateActive() {
    // send pending URC first
    sendPendedUrcs(commandFd);

    RecordStream *p_rs;
    void *p_record;
    size_t recordlen;
    int ret;

    if (stream == NULL) {
        RFX_LOG_D(RFX_LOG_TAG, "Create new stream first time enter active");
        p_rs = record_stream_new(commandFd, MAX_COMMAND_BYTES);
        stream = p_rs;
    } else {
        RFX_LOG_D(RFX_LOG_TAG, "Already have a stream");
        p_rs = stream;
    }
    RFX_LOG_D(RFX_LOG_TAG, "command Fd active is %d", commandFd);
    while(clientState == CLIENT_ACTIVE) {
        /* loop until EAGAIN/EINTR, end of stream, or other error */
        ret = record_stream_get_next(p_rs, &p_record, &recordlen);

        if (ret == 0 && p_record == NULL) {
            /* end-of-stream */
            break;
        } else if (ret < 0) {
            break;
        } else if (ret == 0) { /* && p_record != NULL */
            processCommands(p_record, recordlen, identity);
        }
    }

    if (ret == 0 || !(errno == EAGAIN || errno == EINTR)) {
        RFX_LOG_D(RFX_LOG_TAG, "socket read: ret: %d, errno: %d", ret, errno);
        /* fatal error or end-of-stream */
        if (ret != 0) {
            RFX_LOG_D(RFX_LOG_TAG, "error on reading command socket errno:%d\n", errno);
        } else {
            RFX_LOG_D(RFX_LOG_TAG, "EOS.  Closing command socket.");
        }
        /// Close and reset fd immediately, make sure the others have no chance using it
        /// before StateActivityThread update it.
        close(commandFd);
        commandFd = -1;
        setClientState(CLIENT_DEACTIVE);
    }  else {
        RFX_LOG_D(RFX_LOG_TAG, "socket read: ret: %d, errno: %d, keep reading", ret, errno);
        setClientState(CLIENT_ACTIVE);
    }
}

void RilClient::handleStateDeactive() {
    if (commandFd != -1) {
        RFX_LOG_D(RFX_LOG_TAG, "clear Fd Command %d", commandFd);
        close(commandFd);
        commandFd = -1;
    } else {
        RFX_LOG_D(RFX_LOG_TAG, "commandFd alread -1");
    }

    if (stream != NULL) {
        RFX_LOG_D(RFX_LOG_TAG, "clear stream because stream exists");
        record_stream_free(stream);
        stream = NULL;
    } else {
        RFX_LOG_D(RFX_LOG_TAG, "stream null here");
    }

    setClientState(CLIENT_INITIALIZING);
}

void RilClient::handleStateClosed() {
    if (commandFd != -1) {
        RFX_LOG_D(RFX_LOG_TAG, "closed: clear Fd Command %d", commandFd);
        close(commandFd);
        commandFd = -1;
    } else {
        RFX_LOG_D(RFX_LOG_TAG, "closed: commandFd alread -1");
    }

    if (stream != NULL) {
        RFX_LOG_D(RFX_LOG_TAG, "closed: clear stream because stream exists");
        record_stream_free(stream);
        stream = NULL;
    } else {
        RFX_LOG_D(RFX_LOG_TAG, "closed: stream null here");
    }
}

void RilClient::setClientState(RilClientState state) {
    if (clientState != state) {
        RFX_LOG_D(RFX_LOG_TAG, "set client state %s with old state %s",
            clientStateToString(state), clientStateToString(clientState));
        clientState = state;
    } else {
        RFX_LOG_D(RFX_LOG_TAG, "client state is already %s", clientStateToString(state));
    }
}

char* RilClient::clientStateToString(RilClientState state) {
    char* ret;
    switch(state) {
        case CLIENT_INITIALIZING:
            ret =  (char *) "CLIENT_INITIALIZING";
            break;
        case CLIENT_ACTIVE:
            ret =  (char *) "CLIENT_ACTIVE";
            break;
        case CLIENT_DEACTIVE:
            ret =  (char *) "CLIENT_DEACTIVE";
            break;
        case CLIENT_CLOSED:
            ret = (char *) "CLIENT_CLOSED";
            break;
        default:
            ret = (char *) "NO_SUCH_STATE";
            break;
    }
    return ret;
}

RilClient::StateActivityThread::StateActivityThread (RilClient* client){
    this->client = client;
    RFX_LOG_D(RFX_LOG_TAG, "Consctruct Activity thread");
}

RilClient::StateActivityThread::~StateActivityThread() {
    RFX_LOG_D(RFX_LOG_TAG, "Desctruct Activity thread");
}

bool RilClient::StateActivityThread::threadLoop() {
    while(1) {
        client->clientStateCallback();
        if (client->clientState == CLIENT_CLOSED) {
            return false;
        }
    }
}


void RilClient::processCommands(void *buffer, size_t buflen, int clientId) {
    Parcel p;
    status_t status;
    int32_t request = 0;
    int32_t token = 0;
    RfxRequestInfo *pRI;

    p.setData((uint8_t *) buffer, buflen);
    status = p.readInt32(&request);
    status = p.readInt32 (&token);

    char prop_value[RFX_PROPERTY_VALUE_MAX] = {0};
    rfx_property_get(PROPERTY_3G_SIM, prop_value, "1");
    int capabilitySim = atoi(prop_value) - 1;

    RFX_LOG_D(RFX_LOG_TAG, "enqueue request id %d with token %d for client %d to slot = %d",
            request, token, clientId, capabilitySim);

    pRI = (RfxRequestInfo *)calloc(1, sizeof(RfxRequestInfo));
    if (pRI == NULL) {
        RFX_LOG_E(RFX_LOG_TAG,"OOM");
        return;
    }
    pRI->socket_id = (RIL_SOCKET_ID) capabilitySim;
    pRI->token = token;
    pRI->clientId = (ClientId) clientId;
    for (unsigned int i = 0; i < sCommands_size; i++) {
        if (request == sCommands[i].requestNumber) {
            RFX_LOG_D(RFX_LOG_TAG, "find entry! request = %d", request);
            pRI->pCI = &(sCommands[i]);
            pRI->pCI->dispatchFunction(&rfx_enqueue_request_message_client, p, pRI);
            return;
        }
    }
    free(pRI);
    RFX_LOG_E(RFX_LOG_TAG, "Didn't find any entry, please check ril_client_commands.h");
}

void RilClient::handleUnsolicited(int slotId, int unsolResponse, void *data,
        size_t datalen, UrcDispatchRule rule) {
    RFX_UNUSED(rule);

    int ret;
    RfxUnsolResponseInfo *pUI = NULL;
    char prop_value[RFX_PROPERTY_VALUE_MAX] = {0};
    rfx_property_get(PROPERTY_3G_SIM, prop_value, "1");
    int capabilitySim = atoi(prop_value) - 1;
    RFX_LOG_D(RFX_LOG_TAG, "capabilitySim = %d", capabilitySim);

    if(capabilitySim != slotId) {
        RFX_LOG_D(RFX_LOG_TAG, "only handle capabilitySim");
        return;
    }

    if (commandFd == -1) {
        RFX_LOG_D(RFX_LOG_TAG, "command Fd not ready here");
        // try to cache URC
        cacheUrc(unsolResponse, data, datalen, rule, slotId);
        return;
    }

    for (unsigned int i = 0; i < sUnsolCommands_size; i++) {
        if (unsolResponse == sUnsolCommands[i].requestNumber) {
            pUI = &(sUnsolCommands[i]);
            break;
        }
    }
    if (pUI == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "didn't find unsolResposnInfo");
        return;
    }

    Parcel p;
    p.writeInt32(RESPONSE_UNSOLICITED);
    p.writeInt32(unsolResponse);
    ret = pUI->responseFunction(p, data, datalen);

    if (ret != 0) {
        RFX_LOG_D(RFX_LOG_TAG, "ret = %d, just return", ret);
        return;
    }

    RtcRilClientController::sendResponse(p, commandFd);
}

void RilClient::addHeaderToResponse(Parcel* p) {
    RFX_UNUSED(p);
    RFX_LOG_D(RFX_LOG_TAG, "Add nothing under default behaviour");
    return;
}

void RilClient::requestComplete(RIL_Token token, RIL_Errno e, void *response,
        size_t responselen) {
    if (commandFd < 0) {
        RFX_LOG_D(RFX_LOG_TAG, "command Fd not ready here");
        return;
    }

    int ret;
    size_t errorOffset;
    RfxRequestInfo *pRI = (RfxRequestInfo *) token;

    Parcel p;
    p.writeInt32(RESPONSE_SOLICITED);
    p.writeInt32(pRI->token);
    errorOffset = p.dataPosition();
    p.writeInt32 (e);

    if (response != NULL) {
        // ret = p.write(response, responselen);
        ret = pRI->pCI->responseFunction(p, response, responselen);

        if (ret != 0) {
            RFX_LOG_D(RFX_LOG_TAG, "responseFunction error, ret %d", ret);
            p.setDataPosition(errorOffset);
            p.writeInt32(ret);
        }
    }

    if (e != RIL_E_SUCCESS) {
        RFX_LOG_D(RFX_LOG_TAG, "fails by %d", e);
    }

    RtcRilClientController::sendResponse(p, commandFd);
    free(pRI);
}

void RilClient::cacheUrc(int unsolResponse, const void *data, size_t datalen,
         UrcDispatchRule rule, int slotId) {
    Mutex::Autolock autoLock(sMutex);
    //Only the URC list we wanted.
    if (!isNeedToCache(unsolResponse)) {
        RFX_LOG_I(RFX_LOG_TAG, "Don't need to cache the request %d", unsolResponse);
        return;
    }
    UrcList* urcCur = mPendingUrc[slotId];
    UrcList* urcPrev = NULL;
    int pendedUrcCount = 0;

    while (urcCur != NULL) {
        RFX_LOG_D(RFX_LOG_TAG, "Pended Vsim URC:%d, slot:%d, :%d",
            pendedUrcCount,
            slotId,
            urcCur->id);
        urcPrev = urcCur;
        urcCur = urcCur->pNext;
        pendedUrcCount++;
    }
    urcCur = (UrcList*)calloc(1, sizeof(UrcList));
    if (urcCur == NULL) {
        RFX_LOG_E(RFX_LOG_TAG,"OOM");
        return;
    }
    if (urcPrev != NULL)
        urcPrev->pNext = urcCur;
    urcCur->pNext = NULL;
    urcCur->id = unsolResponse;
    urcCur->datalen = datalen;
    urcCur->data = (char*)calloc(1, datalen + 1);
    if (urcCur->data == NULL) {
        RFX_LOG_E(RFX_LOG_TAG,"OOM");
        free(urcCur);
        return;
    }
    urcCur->data[datalen] = 0x0;
    memcpy(urcCur->data, data, datalen);
    urcCur->rule = rule;
    if (pendedUrcCount == 0) {
        mPendingUrc[slotId] = urcCur;
    }
    RFX_LOG_D(RFX_LOG_TAG, "[Slot %d] Current pendedVsimUrcCount = %d", slotId, pendedUrcCount + 1);
}

void RilClient::sendUrc(int slotId, UrcList* urcCached) {
    UrcList* urc = urcCached;
    UrcList* urc_temp;
    while (urc != NULL) {
        RFX_LOG_D(RFX_LOG_TAG, "sendVsimPendedUrcs RIL%d, %d", slotId, urc->id);
        handleUnsolicited(slotId, urc->id, urc->data, urc->datalen, urc->rule);
        free(urc->data);
        urc_temp = urc;
        urc = urc->pNext;
        free(urc_temp);
    }
}

void RilClient::sendPendedUrcs(int fdCommand) {
    Mutex::Autolock autoLock(sMutex);
    RFX_LOG_D(RFX_LOG_TAG, "Ready to send pended Vsim URCs, fdCommand:%d", fdCommand);
    if (fdCommand != -1) {
        for (int i = 0; i < getSimCount(); i++) {
            sendUrc(i, mPendingUrc[i]);
            mPendingUrc[i] = NULL;
        }
    }
}

bool RilClient::isNeedToCache(int unsolResponse __unused) {
    return false;
}

