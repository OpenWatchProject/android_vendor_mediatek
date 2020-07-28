/*
* Copyright (C) 2014 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#define __STDC_LIMIT_MACROS
#include <stdint.h>
#define RIL_SHLIB
#include "telephony/mtk_ril.h"
#include "RilSapSocket.h"
#include "pb_decode.h"
#include "pb_encode.h"
#undef LOG_TAG
#define LOG_TAG "RIL_UIM_SOCKET"
#include <arpa/inet.h>
#include <errno.h>
#include <sap_service.h>
#include <mtk_log.h>

static RilSapSocket::RilSapSocketList *head = NULL;

extern "C" void
RIL_requestTimedCallback (RIL_TimedCallback callback, void *param,
        const struct timeval *relativeTime);

struct RIL_Env RilSapSocket::uimRilEnv = {
        .OnRequestComplete = RilSapSocket::sOnRequestComplete,
        .OnUnsolicitedResponse = RilSapSocket::sOnUnsolicitedResponse,
        .RequestTimedCallback = RIL_requestTimedCallback
};

void RilSapSocket::sOnRequestComplete (RIL_Token t,
        RIL_Errno e,
        void *response,
        size_t responselen) {
    RilSapSocket *sap_socket;
    SapSocketRequest *request = (SapSocketRequest*) t;

    mtkLogD(LOG_TAG, "Socket id:%d", request->socketId);

    sap_socket = getSocketById(request->socketId);

    if (sap_socket) {
        sap_socket->onRequestComplete(t,e,response,responselen);
    } else {
        mtkLogE(LOG_TAG, "Invalid socket id");
        if (request->curr) {
            free(request->curr);
        }
        free(request);
    }
}

#if defined(ANDROID_MULTI_SIM)
void RilSapSocket::sOnUnsolicitedResponse(int unsolResponse,
        const void *data,
        size_t datalen,
        RIL_SOCKET_ID socketId) {
    RilSapSocket *sap_socket = getSocketById(socketId);
    if (sap_socket) {
        sap_socket->onUnsolicitedResponse(unsolResponse, (void *)data, datalen);
    }
}
#else
void RilSapSocket::sOnUnsolicitedResponse(int unsolResponse,
       const void *data,
       size_t datalen) {
    RilSapSocket *sap_socket = getSocketById(RIL_SOCKET_1);
    if(sap_socket){
        sap_socket->onUnsolicitedResponse(unsolResponse, (void *)data, datalen);
    }
}
#endif

void RilSapSocket::printList() {
    RilSapSocketList *current = head;
    mtkLogD(LOG_TAG, "Printing socket list");
    while(NULL != current) {
        mtkLogD(LOG_TAG, "SocketName:%s",current->socket->name);
        mtkLogD(LOG_TAG, "Socket id:%d",current->socket->id);
        current = current->next;
    }
}

RilSapSocket *RilSapSocket::getSocketById(RIL_SOCKET_ID socketId) {
    RilSapSocket *sap_socket;
    RilSapSocketList *current = head;

    mtkLogD(LOG_TAG, "Entered getSocketById");
    printList();

    while(NULL != current) {
        if(socketId == current->socket->id) {
            sap_socket = current->socket;
            return sap_socket;
        }
        current = current->next;
    }
    return NULL;
}

void RilSapSocket::initSapSocket(const char *socketName,
        const RIL_RadioFunctions *uimFuncs) {

    if (strcmp(socketName, RIL1_SERVICE_NAME) == 0) {
        if(!SocketExists(socketName)) {
            addSocketToList(socketName, RIL_SOCKET_1, uimFuncs);
        }
    }

    if (strcmp(socketName, RIL2_SERVICE_NAME) == 0) {
        if(!SocketExists(socketName)) {
            addSocketToList(socketName, RIL_SOCKET_2, uimFuncs);
        }
    }

    if (strcmp(socketName, RIL3_SERVICE_NAME) == 0) {
        if(!SocketExists(socketName)) {
            addSocketToList(socketName, RIL_SOCKET_3, uimFuncs);
        }
    }

    if (strcmp(socketName, RIL4_SERVICE_NAME) == 0) {
        if(!SocketExists(socketName)) {
            addSocketToList(socketName, RIL_SOCKET_4, uimFuncs);
        }
    }
}

void RilSapSocket::addSocketToList(const char *socketName, RIL_SOCKET_ID socketid,
        const RIL_RadioFunctions *uimFuncs) {
    RilSapSocket* socket = NULL;
    RilSapSocketList *current;

    if(!SocketExists(socketName)) {
        socket = new RilSapSocket(socketName, socketid, uimFuncs);
        RilSapSocketList* listItem = (RilSapSocketList*)malloc(sizeof(RilSapSocketList));
        if (!listItem) {
            mtkLogE(LOG_TAG, "addSocketToList: OOM");
            delete socket;
            return;
        }
        listItem->socket = socket;
        listItem->next = NULL;

        mtkLogD(LOG_TAG, "Adding socket with id: %d", socket->id);

        if(NULL == head) {
            head = listItem;
            head->next = NULL;
        }
        else {
            current = head;
            while(NULL != current->next) {
                current = current->next;
            }
            current->next = listItem;
        }
    }
}

bool RilSapSocket::SocketExists(const char *socketName) {
    RilSapSocketList* current = head;

    while(NULL != current) {
        if(strcmp(current->socket->name, socketName) == 0) {
            return true;
        }
        current = current->next;
    }
    return false;
}

RilSapSocket::RilSapSocket(const char *socketName,
        RIL_SOCKET_ID socketId,
        const RIL_RadioFunctions *inputUimFuncs):
        RilSocket(socketName, socketId) {
    if (inputUimFuncs) {
        uimFuncs = inputUimFuncs;
    }
}

void RilSapSocket::dispatchRequest(MsgHeader *req) {
    // SapSocketRequest will be deallocated in onRequestComplete()
    SapSocketRequest* currRequest=(SapSocketRequest*)malloc(sizeof(SapSocketRequest));
    if (!currRequest) {
        mtkLogE(LOG_TAG, "dispatchRequest: OOM");
        // Free MsgHeader allocated in pushRecord()
        free(req);
        return;
    }
    currRequest->token = req->token;
    currRequest->curr = req;
    currRequest->p_next = NULL;
    currRequest->socketId = id;

    pendingResponseQueue.enqueue(currRequest);

    if (uimFuncs) {
        mtkLogI(LOG_TAG, "RilSapSocket::dispatchRequest [%d] > SAP REQUEST type: %d. id: %d. error: %d, \
                token 0x%p",
                req->token,
                req->type,
                req->id,
                req->error,
                currRequest );

#if defined(ANDROID_MULTI_SIM)
        uimFuncs->onRequest(req->id, req->payload->bytes, req->payload->size, currRequest, id);
#else
        uimFuncs->onRequest(req->id, req->payload->bytes, req->payload->size, currRequest);
#endif
    }
}

void RilSapSocket::onRequestComplete(RIL_Token t, RIL_Errno e, void *response,
        size_t response_len) {
    SapSocketRequest* request= (SapSocketRequest*)t;

    if (!request || !request->curr) {
        mtkLogE(LOG_TAG, "RilSapSocket::onRequestComplete: request/request->curr is NULL");
        return;
    }

    MsgHeader *hdr = request->curr;

    MsgHeader rsp;
    rsp.token = request->curr->token;
    rsp.type = MsgType_RESPONSE;
    rsp.id = request->curr->id;
    rsp.error = (Error)e;
    rsp.payload = (pb_bytes_array_t *)calloc(1, sizeof(pb_bytes_array_t) + response_len);
    if (!rsp.payload) {
        mtkLogE(LOG_TAG, "onRequestComplete: OOM");
    } else {
        if (response && response_len > 0) {
            memcpy(rsp.payload->bytes, response, response_len);
            rsp.payload->size = response_len;
        } else {
            rsp.payload->size = 0;
        }

        mtkLogE(LOG_TAG, "RilSapSocket::onRequestComplete: Token:%d, MessageId:%d ril token 0x%p",
                hdr->token, hdr->id, t);

        sap::processResponse(&rsp, this);
        free(rsp.payload);
    }

    // Deallocate SapSocketRequest
    if(!pendingResponseQueue.checkAndDequeue(hdr->id, hdr->token)) {
        mtkLogE(LOG_TAG, "Token:%d, MessageId:%d", hdr->token, hdr->id);
        mtkLogE(LOG_TAG, "RilSapSocket::onRequestComplete: invalid Token or Message Id");
    }

    // Deallocate MsgHeader
    free(hdr);
}

void RilSapSocket::onUnsolicitedResponse(int unsolResponse, void *data, size_t datalen) {
    if (data && datalen > 0) {
        pb_bytes_array_t *payload = (pb_bytes_array_t *)calloc(1,
                sizeof(pb_bytes_array_t) + datalen);
        if (!payload) {
            mtkLogE(LOG_TAG, "onUnsolicitedResponse: OOM");
            return;
        }
        memcpy(payload->bytes, data, datalen);
        payload->size = datalen;
        MsgHeader rsp;
        rsp.payload = payload;
        rsp.type = MsgType_UNSOL_RESPONSE;
        rsp.id = (MsgId)unsolResponse;
        rsp.error = Error_RIL_E_SUCCESS;
        rsp.token = -1;
        sap::processUnsolResponse(&rsp, this);
        free(payload);
    }
}
