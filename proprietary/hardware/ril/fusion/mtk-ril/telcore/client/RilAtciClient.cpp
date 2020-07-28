/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2018. All rights reserved.
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
#include <sys/socket.h>
#include "RfxDispatchThread.h"
#include "RfxBasics.h"
#include "RfxLog.h"
#include "RfxMessageId.h"
#include "RilAtciClient.h"
#include "rfx_properties.h"

/*****************************************************************************
 * Class RilAtciClient
 *****************************************************************************/

#define RFX_LOG_TAG "RilAtciClient"
#define ATCI_SIM_PROP "persist.vendor.service.atci.sim"

RilAtciClient::RilAtciClient(int identity, char* socketName) : RilClient(identity, socketName) {
}

RilAtciClient::~RilAtciClient() {
}

void RilAtciClient::processCommands(void *buffer, size_t buflen, int clientId) {
    RFX_UNUSED(clientId);
    int request = RFX_MSG_REQUEST_OEM_HOOK_ATCI_INTERNAL;
    int token = 0xFFFFFFFF;
    char prop[RFX_PROPERTY_VALUE_MAX] = {0};
    rfx_property_get(ATCI_SIM_PROP, prop, "0");

    int slotId = atoi(prop);
    RFX_LOG_I(RFX_LOG_TAG, "slotId %d", slotId);
    if (slotId >= 0 && slotId < RIL_SOCKET_NUM) {
        RfxRequestInfo *pRI = (RfxRequestInfo *)calloc(1, sizeof(RfxRequestInfo));
        if (pRI == NULL) {
            RFX_LOG_E(RFX_LOG_TAG, "OOM");
            return;
        }
        pRI->socket_id = (RIL_SOCKET_ID) slotId;
        pRI->token = token;
        pRI->clientId = (ClientId) CLIENT_ID_ATCI;
        pRI->request = request;
        char *atBuffer = (char*)calloc(1, buflen + 1);
        if (atBuffer == NULL) {
            RFX_LOG_E(RFX_LOG_TAG, "fail to calloc buffer");
            free(pRI);
            return;
        }
        memcpy(atBuffer, buffer, buflen);
        RFX_LOG_I(RFX_LOG_TAG, "buffer %s, length %d", atBuffer, (int)buflen);
        rfx_enqueue_request_message_client(request, atBuffer, buflen, pRI, (RIL_SOCKET_ID) slotId);
        free(atBuffer);
    } else {
        RFX_LOG_E(RFX_LOG_TAG, "unsupported slot number");
    }
}

void RilAtciClient::requestComplete(RIL_Token token, RIL_Errno e, void *response,
        size_t responselen) {
    RFX_UNUSED(e);
    RfxRequestInfo *info = (RfxRequestInfo *) token;
    if (info != NULL && RFX_MSG_REQUEST_OEM_HOOK_ATCI_INTERNAL == info->request) {
        String8 strResult;
        if (response != NULL && responselen > 0) {
            strResult = String8((char*) response);
        } else {
            strResult = String8((char*) "\r\nERROR\r\n");
        }

        if (commandFd >= 0) {
            RFX_LOG_I(RFX_LOG_TAG, "commandFd is valid, strResult is %s", strResult.string());
            size_t len = strResult.size();
            ssize_t ret = send(commandFd, strResult, len, 0);
            if (ret != (ssize_t) len) {
                RFX_LOG_E(RFX_LOG_TAG, "lose data when send response.");
            }
        } else {
            RFX_LOG_E(RFX_LOG_TAG, "commandFd < 0");
        }
        free(info);
    }
}

void RilAtciClient::handleUnsolicited(int slotId, int unsolResponse, void *data,
        size_t datalen, UrcDispatchRule rule) {
    RFX_UNUSED(slotId);
    RFX_UNUSED(rule);
    if (RIL_UNSOL_ATCI_RESPONSE == unsolResponse) {
        if (data != NULL && datalen > 0) {
            String8 str = String8((char*) data);

            if (commandFd >= 0) {
                RFX_LOG_I(RFX_LOG_TAG, "commandFd is valid, str is %s", str.string());
                size_t len = str.size();
                ssize_t ret = send(commandFd, str, len, 0);
                if (ret != (ssize_t) len) {
                    RFX_LOG_E(RFX_LOG_TAG, "lose data when send response.");
                }
            } else {
                RFX_LOG_E(RFX_LOG_TAG, "commandFd < 0");
            }
        }
    }
}
