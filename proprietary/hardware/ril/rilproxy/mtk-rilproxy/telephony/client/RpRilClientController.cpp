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

#include "RpRilClientController.h"
#include "RilClientConstants.h"
#include "RfxSocketState.h"
#include <cutils/properties.h>
#include <string.h>
#include <telephony/mtk_ril.h>
#include <arpa/inet.h>
#include "RfxLog.h"

#include "RilAtciClient.h"
#include "RilMalClient.h"
/*****************************************************************************
 * Class RfxController
 *****************************************************************************/

#define RFX_LOG_TAG "RpRilClientCtrl"
#define NUM_ELEMS(a)  (sizeof (a) / sizeof (a)[0])

static ClientInformation client[] = {
    {CLIENT_ID_MAL, (char *) "rilproxy-mal", CLIENT_TYPE_MAL},
    {CLIENT_ID_ATCI, (char *) "rilproxy-atci", CLIENT_TYPE_ATCI},
};

RFX_IMPLEMENT_CLASS("RpRilClientController", RpRilClientController, RfxController);

RpRilClientController::RpRilClientController() {
}

RpRilClientController::~RpRilClientController() {
}

RilClientQueue* RpRilClientController::clientHead = new RilClientQueue();

void RpRilClientController::onInit() {
    RFX_LOG_D(RFX_LOG_TAG, "RpRilClientController init()");
    RfxController::onInit();
    initRilClient();
}

void RpRilClientController::initRilClient() {
    RFX_LOG_D(RFX_LOG_TAG, "init client number: %lu", NUM_ELEMS(client));
    for (unsigned int i = 0; i < NUM_ELEMS(client); i++) {
        ClientInformation information = client[i];
        RilClient* client;
        switch(information.type) {
            case CLIENT_TYPE_DEFAULT:
                client = new RilClient(information.identity, information.socketName);
                break;
            case CLIENT_TYPE_MAL:
                client = new RilMalClient(information.identity, information.socketName);
                break;
            case CLIENT_TYPE_ATCI:
                client = new RilAtciClient(information.identity, information.socketName);
                break;
            default:
                client = new RilClient(information.identity, information.socketName);
                break;
        }
        registerRilClient(client);
    }
}


void RpRilClientController::registerRilClient(RilClient* client) {
    RilClientQueue* queue = clientHead;
    RilClientQueue* clientBeingRegistered = new RilClientQueue(client);
    while(true) {
        if (queue -> nextClient == NULL) {
            queue -> nextClient = clientBeingRegistered;
            clientBeingRegistered -> prevClient = queue;
            break;
        } else {
            queue = queue -> nextClient;
        }
    }
}

RilClient* RpRilClientController::findClientWithId(int clientId) {
    RilClientQueue* queue = clientHead -> nextClient;
    RilClient* matchedClient;
    while(true) {
        if (queue == NULL) {
            matchedClient = NULL;
            break;
        }
        RilClient* client = queue -> client;
        if (client == NULL) {
            LOG_ALWAYS_FATAL("should not have client null here");
            break;
        }

        if (client -> identity == clientId) {
            // RFX_LOG_D(RFX_LOG_TAG, "Find client with client id: %d", clientId);
            matchedClient = client;
            break;
        } else {
            queue = queue -> nextClient;
        }
    }
    return matchedClient;
}

int RpRilClientController::queryFileDescriptor(int clientId) {
    RilClient* client = findClientWithId(clientId);
    int matchedFileDescriptor;
    if (client == NULL) {
        matchedFileDescriptor = -1;
    } else {
        matchedFileDescriptor = client -> commandFd;
    }
    return matchedFileDescriptor;
}

// Implement this here because we want to force everyone send response back
bool RpRilClientController::onClientRequestComplete(int slotId, int token, RIL_Errno e, void *response, size_t responselen,
                                                    int clientId) {
    // RFX_LOG_D(RFX_LOG_TAG, "Enter onClientRequestComplete");
    int ret;
    RilClient* client = findClientWithId(clientId);
    int fd = queryFileDescriptor(clientId);
    //RFX_LOG_D(RFX_LOG_TAG, "onClientRequestComplete: fd is %d", fd);
    size_t errorOffset;

    Parcel p;
    client->addHeaderToResponse(&p, slotId);

    p.writeInt32 (RESPONSE_SOLICITED);
    p.writeInt32 (token);
    errorOffset = p.dataPosition();
    p.writeInt32 (e);

    if (response != NULL) {
        ret = p.write(response, responselen);

        if (ret != 0) {
            RFX_LOG_D(RFX_LOG_TAG, "responseFunction error, ret %d", ret);
            p.setDataPosition(errorOffset);
            p.writeInt32 (ret);
        }
    }

    if (e != RIL_E_SUCCESS) {
        RFX_LOG_D(RFX_LOG_TAG, "fails by %d", e);
    }

    if (fd < 0) {
        RFX_LOG_D(RFX_LOG_TAG, "RIL onRequestComplete: Command channel closed");
        return false;
    }

    sendResponse(p, fd);

    return true;
}

int RpRilClientController::sendResponse (Parcel& p, int fd) {
    uint32_t header;
    int ret;
    header = htonl(p.dataSize());
    ret = blockingWrite(fd, (void *)&header, sizeof(header));

    if (ret < 0) {
        RFX_LOG_D(RFX_LOG_TAG, "sendResponse: ret %d", ret);
        return ret;
    }

    ret = blockingWrite(fd, p.data(), p.dataSize());

    if (ret < 0) {
        RFX_LOG_D(RFX_LOG_TAG, "sendResponse: ret %d", ret);
        return ret;
    }

    return ret;
}

bool RpRilClientController::onClientUnsolicitedResponse(int slotId, int urcId, void *response,
                                                        size_t responseLen) {
    RilClientQueue* queue = clientHead -> nextClient;

    // go through every client and ask them to handle unsolicited
    while(true) {
        if (queue == NULL) {
            break;
        }
        RilClient* client = queue -> client;
        if (client == NULL) {
            LOG_ALWAYS_FATAL("should not have client null here");
        } else {
            // RFX_LOG_D(RFX_LOG_TAG, "client %d handle unsolictied", client -> identity);
            client -> handleUnsolicited(slotId, urcId, response, responseLen);
        }
        queue = queue -> nextClient;
    }
    return true;
}

int RpRilClientController::blockingWrite(int fd, const void *buffer, size_t len) {
    size_t writeOffset = 0;
    const uint8_t *toWrite;

    toWrite = (const uint8_t *)buffer;

    while (writeOffset < len) {
        ssize_t written;
        do {
            written = write (fd, toWrite + writeOffset,
                                len - writeOffset);
        } while (written < 0 && ((errno == EINTR) || (errno == EAGAIN)));

        if (written >= 0) {
            writeOffset += written;
        } else {
            RFX_LOG_D(RFX_LOG_TAG, "RIL Response: unexpected error on write errno:%d", errno);
            // avoid to double close (RilClient would close, too)
            // close(fd);
            return -1;
        }
    }
    // RFX_LOG_D(RFX_LOG_TAG, "leave blocking write");
    return 0;
}

int RpRilClientController::getCdmaSlotId() {
    char tempstr[PROPERTY_VALUE_MAX];
    memset(tempstr, 0, sizeof(tempstr));
    property_get("persist.vendor.radio.cdma_slot", tempstr, "1");
    int cdmaSlotId = atoi(tempstr) - 1;
    RFX_LOG_D(RFX_LOG_TAG, "CDMA slot Id is %d", cdmaSlotId);
    return cdmaSlotId;
}
