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

#include "RtcRilClientController.h"
#include <string.h>
#include <telephony/mtk_ril.h>
#include <arpa/inet.h>
#include "RfxLog.h"
#include "rfx_properties.h"
#include "RilAtciClient.h"
#include "RilOemClient.h"
#include "RfxOpUtils.h"
#include "RilMttsClient.h"
#include "RfxRilUtils.h"
/*****************************************************************************
 * Class RfxController
 *****************************************************************************/

#define RFX_LOG_TAG "RtcRilClientController"

static ClientInformation client[] = {
    {CLIENT_ID_OEM, (char *) "rild-oem", CLIENT_TYPE_OEM},
    {CLIENT_ID_MTTS1, (char *) "rild-mtts1", CLIENT_TYPE_MTTS1},
    {CLIENT_ID_MTTS2, (char *) "rild-mtts2", CLIENT_TYPE_MTTS2},
    {CLIENT_ID_MTTS3, (char *) "rild-mtts3", CLIENT_TYPE_MTTS3},
    {CLIENT_ID_MTTS4, (char *) "rild-mtts4", CLIENT_TYPE_MTTS4},
    {CLIENT_ID_ATCI, (char *) "rild-atci", CLIENT_TYPE_ATCI},
};

RFX_IMPLEMENT_CLASS("RtcRilClientController", RtcRilClientController, RfxController);

RtcRilClientController::RtcRilClientController() {
}

RtcRilClientController::~RtcRilClientController() {
}

RilClientQueue* RtcRilClientController::clientHead = new RilClientQueue();

void RtcRilClientController::onInit() {
    logD(RFX_LOG_TAG, "RtcRilClientController init()");
    RfxController::onInit();
    initRilClient();
}

void RtcRilClientController::initRilClient() {
    logD(RFX_LOG_TAG, "init client number: %u", NUM_ELEMS(client));
    char property_value[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get("persist.vendor.ril.test_mode", property_value, "0");
    for (unsigned int i = 0; i < NUM_ELEMS(client); i++) {
        ClientInformation information = client[i];
        RilClient* client;
        switch(information.type) {
            case CLIENT_TYPE_DEFAULT:
                client = new RilClient(information.identity, information.socketName);
                break;
            case CLIENT_TYPE_OEM:
                if (RfxOpUtils::getOpHandler() == NULL) {
                    logD(RFX_LOG_TAG, "RtcRilClientController CLIENT_TYPE_OEM");
                    client = new RilOemClient(information.identity, information.socketName);
                    break;
                } else {
                    continue;
                }
            case CLIENT_TYPE_ATCI:
                logD(RFX_LOG_TAG, "RtcRilClientController CLIENT_TYPE_ATCI");
                client = new RilAtciClient(information.identity, information.socketName);
                break;
            case CLIENT_TYPE_MTTS4:
                if (atoi(property_value) == 0) {
                    continue;
                }
                if (RfxRilUtils::rfxGetSimCount() < 4) {
                    continue;
                }
                logD(RFX_LOG_TAG, "RtcRilClientController CLIENT_TYPE_MTTS4");
                client = new RilMttsClient(information.identity, information.socketName);
                break;
            case CLIENT_TYPE_MTTS3:
                if (atoi(property_value) == 0) {
                    continue;
                }
                if (RfxRilUtils::rfxGetSimCount() < 3) {
                    continue;
                }
                logD(RFX_LOG_TAG, "RtcRilClientController CLIENT_TYPE_MTTS3");
                client = new RilMttsClient(information.identity, information.socketName);
                break;
            case CLIENT_TYPE_MTTS2:
                if (atoi(property_value) == 0) {
                    continue;
                }
                if (RfxRilUtils::rfxGetSimCount() < 2) {
                    continue;
                }
                logD(RFX_LOG_TAG, "RtcRilClientController CLIENT_TYPE_MTTS2");
                client = new RilMttsClient(information.identity, information.socketName);
                break;
            case CLIENT_TYPE_MTTS1:
                if (atoi(property_value) == 0) {
                    continue;
                }
                logD(RFX_LOG_TAG, "RtcRilClientController CLIENT_TYPE_MTTS1");
                client = new RilMttsClient(information.identity, information.socketName);
                break;
            default:
                client = new RilClient(information.identity, information.socketName);
                break;
        }
        registerRilClient(client);
    }
}


void RtcRilClientController::registerRilClient(RilClient* client) {
    RilClientQueue* queue = clientHead;
    RilClientQueue* clientBeingRegistered = new RilClientQueue(client);
    while(true) {
        if (queue->nextClient == NULL) {
            queue->nextClient = clientBeingRegistered;
            clientBeingRegistered->prevClient = queue;
            break;
        } else {
            queue = queue->nextClient;
        }
    }
}

RilClient* RtcRilClientController::findClientWithId(int clientId) {
    RilClientQueue* queue = clientHead -> nextClient;
    RilClient* matchedClient;
    while(true) {
        if (queue == NULL) {
            matchedClient = NULL;
            break;
        }
        RilClient* client = queue -> client;
        if (client == NULL) {
            RFX_LOG_E(RFX_LOG_TAG, "should not have client null here");
            RFX_ASSERT(0);
            break;
        }

        if (client->identity == clientId) {
            matchedClient = client;
            break;
        } else {
            queue = queue->nextClient;
        }
    }
    return matchedClient;
}

int RtcRilClientController::queryFileDescriptor(int clientId) {
    RilClient* client = findClientWithId(clientId);
    int matchedFileDescriptor;
    if (client == NULL) {
        matchedFileDescriptor = -1;
    } else {
        matchedFileDescriptor = client->commandFd;
    }
    return matchedFileDescriptor;
}

// Implement this here because we want to force everyone send response back
bool RtcRilClientController::onClientRequestComplete(RIL_Token token, RIL_Errno e,
        void *response, size_t responselen, int clientId) {
    RilClient* client = findClientWithId(clientId);
    if (client == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "should not have client null here");
        RFX_ASSERT(0);
    } else {
        RFX_LOG_D(RFX_LOG_TAG, "client %d handle requestComplete", client->identity);
        client->requestComplete(token, e, response, responselen);
    }
    return true;
}

int RtcRilClientController::sendResponse (Parcel& p, int fd) {
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

bool RtcRilClientController::onClientUnsolicitedResponse(int slotId, int urcId,
        void *response, size_t responseLen, UrcDispatchRule rule) {
    RilClientQueue* queue = clientHead->nextClient;

    // go through every client and ask them to handle unsolicited
    while(true) {
        if (queue == NULL) {
            break;
        }
        RilClient* client = queue->client;
        if (client == NULL) {
            RFX_LOG_E(RFX_LOG_TAG, "should not have client null here");
            RFX_ASSERT(0);
        } else {
            client->handleUnsolicited(slotId, urcId, response, responseLen, rule);
        }
        queue = queue->nextClient;
    }
    return true;
}

int RtcRilClientController::blockingWrite(int fd, const void *buffer, size_t len) {
    size_t writeOffset = 0;
    const uint8_t *toWrite;

    toWrite = (const uint8_t *)buffer;

    while (writeOffset < len) {
        ssize_t written;
        do {
            written = write(fd, toWrite + writeOffset,
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
    return 0;
}

int RtcRilClientController::getCdmaSlotId() {
    char tempstr[RFX_PROPERTY_VALUE_MAX];
    memset(tempstr, 0, sizeof(tempstr));
    rfx_property_get("persist.vendor.radio.cdma_slot", tempstr, "1");
    int cdmaSlotId = atoi(tempstr) - 1;
    RFX_LOG_D(RFX_LOG_TAG, "CDMA slot Id is %d", cdmaSlotId);
    return cdmaSlotId;
}
