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
#include "RilMttsClient.h"
#include "RfxLog.h"
#include "Rfx.h"
#include "RfxDispatchThread.h"
#include "rfx_properties.h"
#include <telephony/mtk_ril.h>
#include <libmtkrilutils.h>
#include "RilParcelUtils.h"
#include "RtcRilClientController.h"
#include "RilCommandsTable.h"

/*****************************************************************************
 * Class RilMttsClient
 *****************************************************************************/

#define RFX_LOG_TAG "RilMttsClient"

#define NUM_ELEMS(a)  (sizeof (a) / sizeof (a)[0])

RilMttsClient::RilMttsClient(int identity, char* socketName) : RilClient(identity, socketName) {
    RFX_LOG_D(RFX_LOG_TAG, "Init MTTS client : %s", socketName);
}

void RilMttsClient::processCommands(void *buffer, size_t buflen, int clientId) {
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
    if (clientId == CLIENT_ID_MTTS4) {
        pRI->socket_id = RIL_SOCKET_4;
    } else if (clientId == CLIENT_ID_MTTS3) {
        pRI->socket_id = RIL_SOCKET_3;
    } else if (clientId == CLIENT_ID_MTTS2) {
        pRI->socket_id = RIL_SOCKET_2;
    } else { //if (clientId == CLIENT_ID_MTTS1) {
        pRI->socket_id = RIL_SOCKET_1;
    }
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

void RilMttsClient::handleUnsolicited(int slotId, int unsolResponse, void *data,
        size_t datalen, UrcDispatchRule rule) {
    RFX_UNUSED(rule);

    int ret;
    RfxUnsolResponseInfo *pUI = NULL;

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
    p.writeInt32 (RESPONSE_UNSOLICITED);
    p.writeInt32 (unsolResponse);
    ret = pUI->responseFunction(p, data, datalen);

    if (ret != 0) {
        RFX_LOG_D(RFX_LOG_TAG, "ret = %d, just return", ret);
        return;
    }

    RtcRilClientController::sendResponse(p, commandFd);
}

bool RilMttsClient::isNeedToCache(int unsolResponse __unused) {
    return false;
}

