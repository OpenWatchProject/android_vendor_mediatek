/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#include <cutils/properties.h>
#include <string.h>
#include <iostream>
#include <thread>
#include <cutils/sockets.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include "RilClient.h"
#include "Rfx.h"
#include <telephony/mtk_ril.h>
#include "RpRilClientController.h"
#include "RfxDefaultDestUtils.h"
#include "RfxLog.h"

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/

#define RFX_LOG_TAG "RilClient"
#define ARRAY_LENGTH(array) (sizeof(array)/sizeof(array[0]))

using namespace std;
using namespace android;

RilClient::RilClient(int identity, char* socketName) {
    this -> identity = identity;
    this -> socketName = socketName;
    this -> commandFd = -1;
    this -> listenFd = -1;
    this -> clientState = CLIENT_STATE_UNKNOWN;
    this -> stream = NULL;
    setClientState(CLIENT_INITIALIZING);
    RFX_LOG_D(RFX_LOG_TAG, "init done");
}

RilClient::~RilClient() {

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
    char* socketName = this -> socketName;
    struct sockaddr_un my_addr;
    struct sockaddr_un peer_addr;

    if (listenFd < 0) {
        listenFd = android_get_control_socket(socketName);
    }

    /* some trial to manually create socket, will work if permission is added
    if (listenFd < 0) {
        RFX_LOG_D(RFX_LOG_TAG, "init.rc didnt define, create socket manually");
        //do retry if init.rc didn't define socket
        memset(&my_addr, 0, sizeof(struct sockaddr_un));
        my_addr.sun_family = AF_UNIX;

        char path[256];
        sprintf (path, "/data/%s", socketName);
        RFX_LOG_D(RFX_LOG_TAG, "socketName is %s", path);
        strncpy(my_addr.sun_path, path,
            sizeof(my_addr.sun_path) - 1);

        listenFd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (listenFd < 0) {
            RFX_LOG_D(RFX_LOG_TAG, "manually listen fail, closed");
            setClientState(CLIENT_CLOSED);
        }

        int ret = ::bind(listenFd, (struct sockaddr *) &my_addr,
                sizeof(struct sockaddr_un));
        if (ret < 0) {
            RFX_LOG_D(RFX_LOG_TAG, "bind fail, ret = %d, errno = %d, set state to close", ret, errno);
            listenFd = -1;
            setClientState(CLIENT_CLOSED);
        }
    }
    */

    if (listenFd < 0) {
        RFX_LOG_D(RFX_LOG_TAG, "Failed to get socket %s", socketName);
        setClientState(CLIENT_CLOSED);
        return;
    }

    RFX_LOG_D(RFX_LOG_TAG, "start listen on fd: %d", listenFd);
    ret = listen(listenFd, 4);
    if (ret < 0) {
        RFX_LOG_D(RFX_LOG_TAG, "Failed to listen on control socket '%d': %s",
             listenFd, strerror(errno));
        setClientState(CLIENT_CLOSED);
        return;
    }

    socklen_t socklen = sizeof (peer_addr);
    commandFd = accept (listenFd,  (sockaddr *) &peer_addr, &socklen);
    RFX_LOG_D(RFX_LOG_TAG, "initialize: commandFd is %d", commandFd);

    if (commandFd < 0 ) {
        RFX_LOG_D(RFX_LOG_TAG, "Error on accept() errno:%d", errno);
        setClientState(CLIENT_CLOSED);
        return;
    }
    RFX_LOG_D(RFX_LOG_TAG, "set client state active");
    setClientState(CLIENT_ACTIVE);
}

#define MAX_COMMAND_BYTES (8 * 1024)
void RilClient::handleStateActive() {
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
    activityThread = new StateActivityThread(this);
    activityThread -> run("StateThread");
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
    this -> client = client;
    RFX_LOG_D(RFX_LOG_TAG, "Consctruct Activity thread");
}

RilClient::StateActivityThread::~StateActivityThread() {
    RFX_LOG_D(RFX_LOG_TAG, "Desctruct Activity thread");
}

bool RilClient::StateActivityThread::threadLoop() {
    client -> clientStateCallback();
    return false;
}


void RilClient::processCommands(void *buffer, size_t buflen, int clientId) {
    Parcel* p =  new Parcel();
    status_t status;
    int32_t request = 0;
    int32_t token = 0;

    p -> setData((uint8_t *) buffer, buflen);
    status = p -> readInt32(&request);
    status = p -> readInt32 (&token);
    p->setDataPosition(0);

    //TODO: get 3g capacity protocol
    RFX_LOG_D(RFX_LOG_TAG, "enqueue request id %d with token %d for client %d",
            request, token, clientId);

    int unsolResponseIndex;
    int ret;
    char prop_value[PROPERTY_VALUE_MAX] = {0};
    property_get(PROPERTY_3G_SIM, prop_value, "1");
    int capabilitySim = atoi(prop_value) - 1;
    RFX_LOG_D(RFX_LOG_TAG, "capabilitySim = %d", capabilitySim);
    RILD_RadioTechnology_Group dest = RfxDefaultDestUtils::getDefaultDest(request);

    if (dest == RADIO_TECH_GROUP_C2K) {
        int slotId = RpRilClientController::getCdmaSlotId();
        RFX_LOG_D(RFX_LOG_TAG, "Default destination is cdma, enqueue to slot %d", slotId);
        rfx_enqueue_client_request_message(request, slotId, dest, token, p, clientId);
    } else {
        RFX_LOG_D(RFX_LOG_TAG, "Default destination is gsm, enqueue to slot %d", capabilitySim);
        rfx_enqueue_client_request_message(request, capabilitySim, dest, token, p, clientId);
    }
}

void RilClient::handleUnsolicited(int slotId, int unsolResponse, const void *data,
                                size_t datalen)
{

    int ret;
    char prop_value[PROPERTY_VALUE_MAX] = {0};
    property_get(PROPERTY_3G_SIM, prop_value, "1");
    int capabilitySim = atoi(prop_value) - 1;
    RFX_LOG_D(RFX_LOG_TAG, "capabilitySim = %d", capabilitySim);

    if(capabilitySim != slotId) {
        RFX_LOG_D(RFX_LOG_TAG, "only handle capabilitySim");
        return;
    }


    if (commandFd == -1) {
        RFX_LOG_D(RFX_LOG_TAG, "command Fd not ready here");
        return;
    }

    Parcel p;

    p.writeInt32 (RESPONSE_UNSOLICITED);
    p.writeInt32 (unsolResponse);
    ret = p.write(data, datalen);

    if (ret != 0) {
        RFX_LOG_D(RFX_LOG_TAG, "ret = %d, just return", ret);
    }

    RpRilClientController::sendResponse(p, commandFd);
    return;
}

void RilClient::addHeaderToResponse(Parcel* p __unused, int slotId __unused) {
    RFX_LOG_D(RFX_LOG_TAG, "Add nothing under default behaviour");
    return;
}