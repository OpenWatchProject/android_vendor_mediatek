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
/*
 * rfx_ril_adapter.cpp
 *
 *  Created on: 2015/7/22
 *      Author: MTK08471
 *
 *  RIL Adapter prototype
 */

#include <sys/types.h>
#include <cutils/sockets.h>
#include <cutils/properties.h>
#include <log/log.h>
#include <stdlib.h>
#include "RfxRilAdapter.h"
#include <string.h>
#include "RfxRootController.h"
#include "util/RpFeatureOptionUtils.h"
#include "RfxDispatchThread.h"
#include "telephony/client/RpRilClientController.h"
#include <libmtkrilutils.h>
#include "modecontroller/RpCdmaLteModeController.h"

/*************************************************************
 * RfxRilAdapter Class Implementation
 *************************************************************/
RFX_IMPLEMENT_CLASS("RfxRilAdapter", RfxRilAdapter, RfxObject);
RFX_OBJ_IMPLEMENT_SINGLETON_CLASS(RfxRilAdapter);
#undef LOG_TAG
#define LOG_TAG "RfxRilAdapter"

RfxRilAdapter::RfxRilAdapter() {
    // TODO Auto-generated constructor stub

    socket_mutex = (pthread_mutex_t**)calloc(SIM_COUNT, sizeof(pthread_mutex_t*));
    socket_fds = (int**)calloc(SIM_COUNT, sizeof(int*));
    request_queue_gsm_mutex = (pthread_mutex_t*)calloc(SIM_COUNT, sizeof(pthread_mutex_t));
    request_queue_gsm = new Vector<sp<RfxMessage>>[SIM_COUNT];
    if (socket_mutex == NULL || socket_fds == NULL
            || request_queue_gsm_mutex == NULL || request_queue_gsm == NULL) {
        RFX_LOG_E(LOG_TAG, "Memory allocation failed for %s %s %s %s",
                (socket_mutex == NULL ? "socket_mutex" : ""),
                (socket_fds == NULL ? "socket_fds" : ""),
                (request_queue_gsm_mutex == NULL ? "request_queue_gsm_mutex" : ""),
                (request_queue_gsm == NULL ? "request_queue_gsm" : ""));
        goto mem_err_1;
    }
    for (int i = 0; i < SIM_COUNT; i++) {
        socket_mutex[i] = (pthread_mutex_t*)calloc(RADIO_TECH_GROUP_NUM, sizeof(pthread_mutex_t));
        if (socket_mutex[i] == NULL) {
            RFX_LOG_E(LOG_TAG, "Memory allocation failed for socket_mutex[%d]", i);
            goto mem_err_2;
        }
        socket_fds[i] = (int*)calloc(RADIO_TECH_GROUP_NUM, sizeof(int));
        if (socket_fds[i] == NULL) {
            RFX_LOG_E(LOG_TAG, "Memory allocation failed for socket_fds[%d]", i);
            goto mem_err_2;
        }
    }
    for (int i = 0; i < SIM_COUNT; i++) {
        for (int j = 0; j < RADIO_TECH_GROUP_NUM; j++) {
            socket_fds[i][j] = -1;
            pthread_mutex_init(&socket_mutex[i][j], NULL);
        }
        pthread_mutex_init(&request_queue_gsm_mutex[i], NULL);
    }
    return;

mem_err_2:
    for (int i = 0; i < SIM_COUNT; i++) {
        free(socket_mutex[i]);
        free(socket_fds[i]);
    }
mem_err_1:
    if (request_queue_gsm != NULL) {
        delete[] request_queue_gsm;
    }
    free(socket_mutex);
    free(socket_fds);
    free(request_queue_gsm_mutex);
    socket_mutex = NULL;
    socket_fds = NULL;
    request_queue_gsm_mutex = NULL;
    request_queue_gsm = NULL;
    return;
}

RfxRilAdapter::~RfxRilAdapter() {
    unregisterSocketStateListener();

    for (int i = 0; i < SIM_COUNT; i++) {
        for (int j = 0; j < RADIO_TECH_GROUP_NUM; j++) {
            pthread_mutex_destroy(&socket_mutex[i][j]);
        }
        pthread_mutex_destroy(&request_queue_gsm_mutex[i]);
    }
    for (int i = 0; i < SIM_COUNT; i++) {
        free(socket_mutex[i]);
        free(socket_fds[i]);
    }
    if (request_queue_gsm != NULL) {
        delete[] request_queue_gsm;
    }
    free(socket_mutex);
    free(socket_fds);
    free(request_queue_gsm_mutex);

    socket_mutex = NULL;
    socket_fds = NULL;
    request_queue_gsm_mutex = NULL;
    request_queue_gsm = NULL;
}

bool RfxRilAdapter::setSocket(int slotId, RILD_RadioTechnology_Group group, int fd) {

    if ( (slotId >= SIM_COUNT) ||
            (group >= RADIO_TECH_GROUP_NUM) ) {
        RFX_LOG_E(LOG_TAG, "setSocket param error (slotId=%d, group=%d, fd=%d)",
                slotId, group, fd);
        return false;
    }

    pthread_mutex_lock(&socket_mutex[slotId][group]);
    socket_fds[slotId][group] = fd;
    pthread_mutex_unlock(&socket_mutex[slotId][group]);
    RFX_LOG_I(LOG_TAG, "setSocket (slotId=%d, group=%d, fd=%d)", slotId, group, fd);

    return true;
}

bool RfxRilAdapter::switchC2kSocket(int targetSlotId) {
    bool ret = false;
    if (targetSlotId >= SIM_COUNT) {
        RFX_LOG_E(LOG_TAG, "switchC2kSocket param error (targetSlotId=%d)", targetSlotId);
        return false;
    }

    RFX_LOG_D(LOG_TAG, "switchC2kSocket - start all fds mutex");
    for (int i = 0; i < SIM_COUNT; i++) {
        pthread_mutex_lock(&socket_mutex[i][RADIO_TECH_GROUP_C2K]);
    }

    int c2kFd = -1;
    int c2kSlotId = -1;
    for (int i = 0; i < SIM_COUNT; i++) {
        if (socket_fds[i][RADIO_TECH_GROUP_C2K] != -1) {
            c2kFd = socket_fds[i][RADIO_TECH_GROUP_C2K];
            c2kSlotId = i;
            break;
        }
    }
    RFX_LOG_D(LOG_TAG,
            "switchC2kSocket check current c2k fd (c2kSlotId=%d, c2kFd=%d, targetSlotId=%d)",
            c2kSlotId, c2kFd, targetSlotId);

    // check current condition
    int doSwitch = 0;
    if (c2kSlotId == -1) {
        RFX_LOG_E(LOG_TAG, "switchC2kSocket status error (c2k fd not exist)");
        doSwitch = 0;
        ret = false;
    } else if (c2kSlotId == targetSlotId) {
        RFX_LOG_I(LOG_TAG, "switchC2kSocket c2k already at target slot (c2kSlotId=%d, c2kFd=%d)",
                c2kSlotId, c2kFd);
        doSwitch = 0;
        ret = true;
    } else {
        doSwitch = 1;
        ret = true;
    }

    if (doSwitch == 1) {
        for (int i = 0; i < SIM_COUNT; i++) {
            if (i == targetSlotId) {
                socket_fds[i][RADIO_TECH_GROUP_C2K] = c2kFd;
            } else {
                socket_fds[i][RADIO_TECH_GROUP_C2K] = -1;
            }
        }
    }

    for (int i = (SIM_COUNT - 1); i >= 0; i--) {
        pthread_mutex_unlock(&socket_mutex[i][RADIO_TECH_GROUP_C2K]);
    }
    RFX_LOG_D(LOG_TAG, "switchC2kSocket - release all fds mutex (doSwitch=%d)", doSwitch);

    if (doSwitch) {
        clearRequestQueue(c2kSlotId, RADIO_TECH_GROUP_C2K);
    }

    if (SIM_COUNT >= 2) {
        for (int i = 0; i < SIM_COUNT; i++) {
            if (i != targetSlotId && socket_fds[i][RADIO_TECH_GROUP_C2K] != -1) {
                RFX_LOG_E(LOG_TAG, "switchC2kSocket error fds[%d]=%d", i,
                        socket_fds[i][RADIO_TECH_GROUP_C2K]);
            }
        }
        RFX_LOG_I(LOG_TAG, "switchC2kSocket done (targetSlotId=%d, fds[%d]=%d)", targetSlotId,
                targetSlotId, socket_fds[targetSlotId][RADIO_TECH_GROUP_C2K]);
    } else {
        RFX_LOG_D(LOG_TAG, "switchC2kSocket should not work when SIM COUNT=1 (fds: %d)",
                socket_fds[0][RADIO_TECH_GROUP_C2K]);
    }
    return ret;
}

bool RfxRilAdapter::closeSocket(int slotId, RILD_RadioTechnology_Group group) {

    RFX_LOG_I(LOG_TAG, "closeSocket (slotId=%d, group=%d)", slotId, group);

    if ( (slotId >= SIM_COUNT) ||
            (group >= RADIO_TECH_GROUP_NUM) ) {
        return false;
    }

    pthread_mutex_lock(&socket_mutex[slotId][group]);
    socket_fds[slotId][group] = -1;
    pthread_mutex_unlock(&socket_mutex[slotId][group]);

    return true;
}

#define MAX_COMMAND_BYTES (20 * 1024)

void RfxRilAdapter::requestToRild(const sp<RfxMessage>& message) {
    // arg check
    if (message == NULL || message.get() == NULL) {
        RFX_LOG_E(LOG_TAG, "(w)requestToRild arg check fail");
        return;
    }

    // type check
    if (message->getType() != RFX_MESSAGE_TYPE::REQUEST) {
        RFX_LOG_E(LOG_TAG, "(w)requestToRild type check fail (%d)", message->getType());
        return;
    }

    // fd check
    int slotId = message->getSlotId();
    if (slotId >= SIM_COUNT) {
        RFX_LOG_E(LOG_TAG, "requestToRild target slot id error. SIM_COUNT: %d, target slot id: %d",
                SIM_COUNT, slotId);
        return;
    }

    int dest = message->getDest();
    int targetFd = socket_fds[slotId][dest];

    // External SIM [Start]
    if ((message->getId() == RIL_REQUEST_VSIM_NOTIFICATION ||
            message->getId() == RIL_REQUEST_VSIM_OPERATION ||
            dest == RADIO_TECH_GROUP_VSIM)) {
        targetFd = vsim_fd[slotId];
        RFX_LOG_E(LOG_TAG, "requestToRild change to vsim fd (slotId=%d, dest=%d, targetFd=%d)",
            slotId, dest, targetFd);
    }
    // External SIM [End]

    int currC2kSlotId = RpCdmaLteModeController::getCdmaSocketSlotId();
    if (slotId == currC2kSlotId) {
        message->setSentOnCdmaCapabilitySlot(C_SLOT_STATUS_IS_CURRENT_SLOT);
    } else {
        message->setSentOnCdmaCapabilitySlot(C_SLOT_STATUS_NOT_CURRENT_SLOT);
    }
    if (targetFd == -1) {
        if (dest == RADIO_TECH_GROUP_C2K && (slotId != currC2kSlotId ||
                    RpFeatureOptionUtils::isC2kSupport() != 1)) {
            RFX_LOG_E(LOG_TAG,
                    "(w)requestToRild incorrect c2k socket (slotId=%d, dest=%d, targetFd=%d) (c2kslotId=%d)",
                    slotId, dest, targetFd, currC2kSlotId);
            dispatchErrorResponse(message);
            return;
        }

        RFX_LOG_E(LOG_TAG, "(w)requestToRild socket not ready (slotId=%d, dest=%d, targetFd=%d)",
                slotId, dest, targetFd);
        addRequestQueue(message->getSlotId(), message->getDest(), message);
        return;
    }

    if (!isRequestQueueEmpty(message->getSlotId(), message->getDest())) {
        RFX_LOG_I(LOG_TAG,
                "(w)requestToRild request queue not empty, dispatch first. (slotId=%d, dest=%d, targetFd=%d)",
                slotId, dest, targetFd);
        dispatchRequestQueue(message->getSlotId(), message->getDest());
    }

    requestToRildX(message);
    return;
}

bool RfxRilAdapter::isDisableRequestLog(int reqId) {
    int ret = false;
    if (reqId == RIL_RESPONSE_ACKNOWLEDGEMENT) {
        ret = true;
    }
    return ret;
}

bool RfxRilAdapter::requestToRildX(const sp<RfxMessage>& message) {
    bool sendResult = false;

    uint8_t dataLength[4];

    // arg check
    if (message == NULL || message.get() == NULL) {
        RFX_LOG_E(LOG_TAG, "requestToRild arg check fail");
        return false;
    }

    // type check
    if (message->getType() != RFX_MESSAGE_TYPE::REQUEST) {
        RFX_LOG_E(LOG_TAG, "requestToRild type check fail (%d)", message->getType());
        return false;
    }

    // fd check
    int slotId = message->getSlotId();
    int dest = message->getDest();
    int targetFd = socket_fds[slotId][dest];

    // External SIM [Start]
    if ((message->getId() == RIL_REQUEST_VSIM_NOTIFICATION ||
            message->getId() == RIL_REQUEST_VSIM_OPERATION ||
            dest == RADIO_TECH_GROUP_VSIM)) {
        targetFd = vsim_fd[slotId];
        RFX_LOG_E(LOG_TAG, "requestToRild change to vsim fd (slotId=%d, dest=%d, targetFd=%d)",
            slotId, dest, targetFd);
    }
    // External SIM [End]

    if (targetFd == -1) {
        RFX_LOG_E(LOG_TAG, "requestToRild socket not ready (slotId=%d, dest=%d, targetFd=%d)",
                slotId, dest, targetFd);
        return false;
    }

    int reqId = message->getId();
    // check data obj
    // Parcel* parcel = message->getParcel();
    Parcel *parcel = NULL;
    if (message->getDataObj() != NULL) {
        // RFX_LOG_D(LOG_TAG, "requestToRild: use data obj");
        message->getDataObj()->rfxData2Parcel(parcel);
    } else {
        // RFX_LOG_D(LOG_TAG, "requestToRild: use parcel");
        parcel = message->getParcel();
    }

    // payload check
    if (parcel == NULL) {
        RFX_LOG_E(LOG_TAG, "requestToRild parcel is NULL");
        return false;
    }
    int dataSize = parcel->dataSize();
    const uint8_t* data = parcel->data();
    if (dataSize > MAX_COMMAND_BYTES) {
        RFX_LOG_E(LOG_TAG, "requestToRild parcel dataSize check fail %d (>%d)",
                dataSize, MAX_COMMAND_BYTES);
        return false;
    }

    // parcel length in big endian
    dataLength[0] = dataLength[1] = 0;
    dataLength[2] = ((dataSize >> 8) & 0xff);
    dataLength[3] = ((dataSize) & 0xff);

    pthread_mutex_lock(&socket_mutex[slotId][dest]);

    sendResult = true;

    // Send Data
    ssize_t sent = 0;
    if (message->getId() != RIL_RESPONSE_ACKNOWLEDGEMENT) {
        RfxDispatchThread::addMessageToPendingQueue(message);
    }
    sent = send(targetFd , dataLength , 4, 0);
    if( sent < 0 ) {
        RFX_LOG_E(LOG_TAG,
                "requestToRild send datalen fail (sent=%zu, err=%d)(%d, %d, %d)(%d, %d, %d, %d)",
                sent, errno, slotId, dest, targetFd, dataLength[0], dataLength[1], dataLength[2],
                dataLength[3]);
        sendResult = false;
        RfxDispatchThread::removeMessageFromPendingQueue(message->getToken());
    }
    if (sendResult) {
        sent += send(targetFd , data , dataSize, 0);
        if( sent < 0 ) {
            RFX_LOG_E(LOG_TAG,
                    "requestToRild send data fail (sent=%zu, err=%d)(%d, %d, %d, dataSize=%d)",
                    sent, errno, slotId, dest, targetFd, dataSize);
            sendResult = false;
            RfxDispatchThread::removeMessageFromPendingQueue(message->getToken());
        }
    }

    pthread_mutex_unlock(&socket_mutex[slotId][dest]);
    // RfxDispatchThread::addMessageToPendingQueue(message);

    RFX_LOG_D(LOG_TAG,
            "requestToRild send request done (slotId=%d, dest=%d, reqId=%d, %s, dataSize=%d, %zu)",
            slotId, dest, reqId, (sendResult == true ? "true" : "false"), dataSize, sent);
    return sendResult;
}

// External SIM [Start]
void RfxRilAdapter::setVsimSocket(int slotId, int fd) {
    vsim_fd[slotId] = fd;
    RFX_LOG_I(LOG_TAG, "setVsimSocket (slotId=%d, fd=%d)", slotId, fd);
}
// External SIM [End]

extern "C" void
RIL_onRequestAck(RIL_Token t);

void RfxRilAdapter::requestAckToRilj(const sp<RfxMessage>& message) {
    // arg check
    if (message == NULL || message.get() == NULL) {
        RFX_LOG_E(LOG_TAG, "requestAckToRilj arg check fail");
        return;
    }

    RFX_MESSAGE_TYPE type = message->getType();

    if (message->getClientId() != -1) {
        RFX_LOG_E(LOG_TAG, "requestAckToRilj not support message with client Id: %d",
                message->getClientId());
        return;
    }

    RIL_onRequestAck(message->getRilToken());
    RFX_LOG_D(LOG_TAG, "requestAckToRilj send done (pId=%d, pToken=%d, id=%d, token=%d)",
            message->getPId(), message->getPToken(), message->getId(), message->getToken());
}

bool RfxRilAdapter::responseToRilj(const sp<RfxMessage>& message) {

    bool ret = false;

    void* data = NULL;
    int datalen = 0;

    // arg check
    if (message == NULL || message.get() == NULL) {
        RFX_LOG_E(LOG_TAG, "requestToRilj arg check fail");
        return false;
    }

    int slotId = message->getSlotId();
    RFX_MESSAGE_TYPE type = message->getType();

    // type check
    if (type != RFX_MESSAGE_TYPE::RESPONSE && type != RFX_MESSAGE_TYPE::URC) {
        RFX_LOG_E(LOG_TAG, "requestToRilj type check fail (%d)", type);
        return false;
    }

    // payload check
    Parcel* parcel = message->getParcel();
    if (parcel == NULL) {
        RFX_LOG_E(LOG_TAG, "requestToRilj parcel is NULL");
        return false;
    }

    RfxDispatchThread::RequestInfo *pRI =
            (RfxDispatchThread::RequestInfo *)message->getRilToken();

    if (pRI!= NULL && (pRI->token != message->getPToken())) {
        RFX_LOG_I(LOG_TAG, "module response request twice, reqId:%d", message->getId());
        return false;
    }

    int dataSize = parcel->dataSize();
    if (type == RFX_MESSAGE_TYPE::RESPONSE) {
        if (message->getClientId() != -1) {
            // for Ril Client
            void* clientData = NULL;
            size_t clientDatalen = 0;
            parcel->setDataPosition(sizeof(uint32_t)*3);
            clientDatalen = (size_t)parcel->dataAvail();
            clientData = (void*)(parcel->readInplace(datalen));
            ret = RpRilClientController::
                    onClientRequestComplete(slotId, message->getPToken(), message->getError(),
                    clientData, clientDatalen, message -> getClientId());
            clientData = NULL;
        } else {
            RFX_LOG_D(LOG_TAG, "message->getRilToken() = %p", message->getRilToken());
            RfxTransferUtils::parcelToDataResponse(message->getRilToken(), message->getError(),
                    message->getPId(), parcel, slotId);
        }
        RFX_LOG_D(LOG_TAG,
                "responseToRilj send Response done (slotId=%d, pToken=%d, pId=%d, dataSize=%d)",
                slotId, message->getPToken(), message->getPId(), dataSize);
    } else if (type == RFX_MESSAGE_TYPE::URC) {
        // for Ril Client
        void* clientData = NULL;
        size_t clientDatalen = 0;
        parcel->setDataPosition(sizeof(uint32_t)*2);
        clientDatalen = (size_t)parcel->dataAvail();
        clientData = (void*)(parcel->readInplace(datalen));
        ret = RpRilClientController::
                onClientUnsolicitedResponse(slotId, message->getPId(), clientData, clientDatalen);
        clientData = NULL;

        RfxTransferUtils::parcelToDataUrc(message->getPId(), parcel, slotId);
        RFX_LOG_D(LOG_TAG, "responseToRilj send Urc done (slotId=%d, PId=%d, dataSize=%d)", slotId,
                message->getPId(), dataSize);
    }

    return ret;
}

extern "C" void RIL_onRequestComplete(RIL_Token t, RIL_Errno e, void *response, size_t
        responselen);
#if defined(ANDROID_MULTI_SIM)
extern "C" void RIL_onUnsolicitedResponse(int unsolResponse, const void *data,
        size_t datalen, RIL_SOCKET_ID socketId);
#else
extern "C" void RIL_onUnsolicitedResponse(int unsolResponse, const void *data,
        size_t datalen);
#endif

void RfxRilAdapter::responseToRilj(RIL_Token t, RIL_Errno e, void *response, int responselen) {
    RIL_onRequestComplete(t,e,response,responselen);
}

#if defined(ANDROID_MULTI_SIM)
void RfxRilAdapter::responseToRilj(int unsolResponse, void* data, int datalen, RIL_SOCKET_ID socketId) {
#else
void RfxRilAdapter::responseToRilj(int unsolResponse, void* data, int datalen) {
#endif

#if defined(ANDROID_MULTI_SIM)
    RIL_onUnsolicitedResponse(unsolResponse, data, datalen, socketId);
#else
    RIL_onUnsolicitedResponse(unsolResponse, data, datalen);
#endif
}

Parcel* RfxRilAdapterUtils::extractResponse(uint8_t* buf, int buflen) {
    Parcel* parcel = NULL;

    if (buf != NULL) {
        parcel = new Parcel();
        parcel->setData(buf, buflen); // p.setData((uint8_t *) buffer, buflen);
        parcel->setDataPosition(0);
    }

    return parcel;
}

bool RfxRilAdapter::addRequestQueue(int slotId, RILD_RadioTechnology_Group group,
        const sp<RfxMessage>& message) {
    bool ret = false;
    Vector<sp<RfxMessage>>* rQueue = NULL;

    if (group == RADIO_TECH_GROUP_GSM) {
        pthread_mutex_lock(&request_queue_gsm_mutex[slotId]);
        rQueue = &request_queue_gsm[slotId];
    } else if (group == RADIO_TECH_GROUP_C2K) {
        pthread_mutex_lock(&request_queue_c2k_mutex);
        rQueue = &request_queue_c2k;
    }

    if (rQueue !=NULL) {
        rQueue->add(message);
        ret = true;
    }

    if (group == RADIO_TECH_GROUP_GSM) {
        pthread_mutex_unlock(&request_queue_gsm_mutex[slotId]);
    } else if (group == RADIO_TECH_GROUP_C2K) {
        pthread_mutex_unlock(&request_queue_c2k_mutex);
    }

    RFX_LOG_D(LOG_TAG, "addRequestQueue slotId=%d, group=%d, id=%d, type=%d, ret=%s",
            slotId, group, message->getId(), message->getType(), (ret ? "t" : "f"));
    return ret;
}

bool RfxRilAdapter::isRequestQueueEmpty(int slotId, RILD_RadioTechnology_Group group) {
    bool ret = true;

    Vector<sp<RfxMessage>>* rQueue = NULL;

    if (group == RADIO_TECH_GROUP_GSM) {
        pthread_mutex_lock(&request_queue_gsm_mutex[slotId]);
        rQueue = &request_queue_gsm[slotId];
    } else if (group == RADIO_TECH_GROUP_C2K) {
        pthread_mutex_lock(&request_queue_c2k_mutex);
        rQueue = &request_queue_c2k;
    }

    if (rQueue !=NULL) {
        ret = rQueue->isEmpty();
    }

    if (group == RADIO_TECH_GROUP_GSM) {
        pthread_mutex_unlock(&request_queue_gsm_mutex[slotId]);
    } else if (group == RADIO_TECH_GROUP_C2K) {
        pthread_mutex_unlock(&request_queue_c2k_mutex);
    }

    if (ret == false) {
        RFX_LOG_D(LOG_TAG, "isRequestQueueEmpty slotId=%d, group=%d, ret=%s", slotId, group,
                (ret ? "t" : "f"));
    }
    return ret;
}

void RfxRilAdapter::onSocketStateChanged(RfxStatusKeyEnum key, RfxVariant old_value,
        RfxVariant value) {
    RFX_UNUSED(key);
    RfxSocketState old_socket_state = old_value.asSocketState();
    RfxSocketState socket_state = value.asSocketState();
    int slotId = socket_state.getSlotId();
    bool stateValue = false, stateValueOld = false;
    // check gsm
    stateValue = socket_state.getSocketState(RfxSocketState::SOCKET_GSM);
    stateValueOld = old_socket_state.getSocketState(RfxSocketState::SOCKET_GSM);
    if (stateValue != stateValueOld) {
        RFX_LOG_I(LOG_TAG, "onSocketStateChanged, slotId=%d, gsm, stateValue: %s -> %s", slotId,
                (stateValueOld ? "t" : "f"), (stateValue ? "t" : "f"));
        if (!stateValueOld && stateValue) {
            // disconnect -> connect, check queue & dispatch
            bool reqQueueEmpty = isRequestQueueEmpty(slotId, RADIO_TECH_GROUP_GSM);
            if (!reqQueueEmpty) {
                dispatchRequestQueue(slotId, RADIO_TECH_GROUP_GSM);
            }
        } else if (stateValueOld && !stateValue) {
            clearRequestQueue(slotId, RADIO_TECH_GROUP_GSM);
        }
    }
    // check c2k
    bool isCdmaSlot = socket_state.getIsCdmaSlot();
    if (isCdmaSlot) {
        stateValue = socket_state.getSocketState(RfxSocketState::SOCKET_C2K);
        stateValueOld = old_socket_state.getSocketState(RfxSocketState::SOCKET_C2K);
        if (stateValue != stateValueOld) {
            RFX_LOG_I(LOG_TAG, "onSocketStateChanged, slotId=%d, c2k, stateValue: %s -> %s", slotId,
                    (stateValueOld ? "t" : "f"), (stateValue ? "t" : "f"));
            if (!stateValueOld && stateValue) {
                // disconnect -> connect, check queue & dispatch
                bool reqQueueEmpty = isRequestQueueEmpty(slotId, RADIO_TECH_GROUP_C2K);
                if (!reqQueueEmpty) {
                    dispatchRequestQueue(slotId, RADIO_TECH_GROUP_C2K);
                }
            } else if (stateValueOld && !stateValue) {
                clearRequestQueue(slotId, RADIO_TECH_GROUP_C2K);
            }
        }
    }
    RFX_LOG_D(LOG_TAG, "onSocketStateChanged, done");
}

void RfxRilAdapter::registerSocketStateListener() {
    RFX_LOG_D(LOG_TAG, "registerSocketStateListener");

    for (int i=0; i < RpFeatureOptionUtils::getSimCount(); i++) {
        RfxStatusManager *slotStatusMgr = RFX_OBJ_GET_INSTANCE(RfxRootController)->getStatusManager(i);
        if (slotStatusMgr != NULL) {
            slotStatusMgr->registerStatusChanged(RFX_STATUS_KEY_SOCKET_STATE,
                    RfxStatusChangeCallback(this, &RfxRilAdapter::onSocketStateChanged));
        } else {
            RFX_LOG_E(LOG_TAG, "registerSocketStateListener slotStatusManager(%d) NULL", i);
        }
    }
}

void RfxRilAdapter::unregisterSocketStateListener() {
    RFX_LOG_D(LOG_TAG, "unregisterSocketStateListener");

    for (int i=0; i < RpFeatureOptionUtils::getSimCount(); i++) {
        RfxStatusManager *slotStatusMgr = RFX_OBJ_GET_INSTANCE(RfxRootController)->getStatusManager(i);
        if (slotStatusMgr != NULL) {
            slotStatusMgr->unRegisterStatusChanged(RFX_STATUS_KEY_SOCKET_STATE,
                    RfxStatusChangeCallback(this, &RfxRilAdapter::onSocketStateChanged));
        } else {
            RFX_LOG_E(LOG_TAG, "unregisterSocketStateListener slotStatusManager(%d) NULL", i);
        }
    }
}

bool RfxRilAdapter::dispatchRequestQueue(int slotId, RILD_RadioTechnology_Group group) {
    bool ret = false;

    int dispatchCount = 0;
    Vector<sp<RfxMessage>>* rQueue = NULL;
    if (group == RADIO_TECH_GROUP_GSM) {
        pthread_mutex_lock(&request_queue_gsm_mutex[slotId]);
        rQueue = &request_queue_gsm[slotId];
    } else if (group == RADIO_TECH_GROUP_C2K) {
        pthread_mutex_lock(&request_queue_c2k_mutex);
        rQueue = &request_queue_c2k;
    }

    if (rQueue !=NULL) {
        if (!rQueue->isEmpty()) {
            for (size_t i = 0; i < rQueue->size(); i++) {
                sp<RfxMessage> msg = rQueue->itemAt(i);
                requestToRildX(msg);
                dispatchCount++;
            }
            rQueue->clear();
        }
        ret = true;
    }

    if (group == RADIO_TECH_GROUP_GSM) {
        pthread_mutex_unlock(&request_queue_gsm_mutex[slotId]);
    } else if (group == RADIO_TECH_GROUP_C2K) {
        pthread_mutex_unlock(&request_queue_c2k_mutex);
    }

    RFX_LOG_I(LOG_TAG, "dispatchRequestQueue slotId=%d, group=%d, ret=%s, dispatchCount=%d", slotId,
            group, (ret ? "t" : "f"), dispatchCount);
    return ret;
}

void RfxRilAdapter::clearRequestQueue(int slotId, RILD_RadioTechnology_Group group) {
    int clearCount = 0;
    Vector<sp<RfxMessage>>* rQueue = NULL;

    if (slotId <0 || slotId >= RpFeatureOptionUtils::getSimCount()) {
        RFX_LOG_E(LOG_TAG, "clearRequestQueue param error (slotId=%d, group=%d", slotId, group);
        return;
    }

    if (group == RADIO_TECH_GROUP_GSM) {
        pthread_mutex_lock(&request_queue_gsm_mutex[slotId]);
        rQueue = &request_queue_gsm[slotId];
    } else if (group == RADIO_TECH_GROUP_C2K) {
        pthread_mutex_lock(&request_queue_c2k_mutex);
        rQueue = &request_queue_c2k;
    }

    if (rQueue !=NULL) {
        for (size_t i = 0; i < rQueue->size(); i++) {
            sp<RfxMessage> msg = rQueue->itemAt(i);
            RFX_LOG_D(LOG_TAG, "clearRequestQueue slotId=%d, group=%d, queue[%zu]: id=%d", slotId,
                    group, i, msg->getId());
            clearCount++;
        }
        rQueue->clear();
    }

    if (group == RADIO_TECH_GROUP_GSM) {
        pthread_mutex_unlock(&request_queue_gsm_mutex[slotId]);
    } else if (group == RADIO_TECH_GROUP_C2K) {
        pthread_mutex_unlock(&request_queue_c2k_mutex);
    }

    RFX_LOG_I(LOG_TAG, "clearRequestQueue slotId=%d, group=%d, clearCount=%d", slotId, group,
            clearCount);
}

void RfxRilAdapter::initSocketNotifyByFwk() {
    registerSocketStateListener();
}

void RfxRilAdapter::dispatchErrorResponse(const sp<RfxMessage>& req) {
    if (req == NULL) {
        RFX_LOG_E(LOG_TAG, "dispatchErrorResponse fail (req NULL)");
        return;
    }
    if (req->getType() != RFX_MESSAGE_TYPE::REQUEST) {
        RFX_LOG_E(LOG_TAG, "dispatchErrorResponse fail (req type %d incorrect)(slotId=%d, dest=%d, id=%d)",
                req->getType(), req->getSlotId(), req->getDest(), req->getId());
        return;
    } else {
        RFX_LOG_I(LOG_TAG, "dispatchErrorResponse req (slotId=%d, dest=%d, id=%d)",
                req->getSlotId(), req->getDest(), req->getId());
    }

    sp<RfxMessage> errorResponse = RfxMessage::obtainResponse(req->getSlotId(), req->getDest(),
            RIL_Errno::RIL_E_RADIO_NOT_AVAILABLE, req, true);
    RfxMainThread::waitLooper();
    RfxMainThread::enqueueMessage(errorResponse);
}

void RfxRilAdapter::clearAllRequestQueue() {
    RFX_LOG_I(LOG_TAG, "clearAllRequestQueue");
    for (int i = 0; i < SIM_COUNT; i++) {
        clearRequestQueue(i, RADIO_TECH_GROUP_GSM);
        clearRequestQueue(i, RADIO_TECH_GROUP_C2K);
    }
}
