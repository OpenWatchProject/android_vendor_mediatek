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
#include "RilMalClient.h"
#include "Rfx.h"
#include <telephony/mtk_ril.h>
#include "RpRilClientController.h"
#include "RfxDefaultDestUtils.h"
#include <mal.h>
#include <libmtkrilutils.h>
#include "RfxLog.h"
/*****************************************************************************
 * Class RfxController
 *****************************************************************************/

#define RFX_LOG_TAG "RilMalClient"
#define ARRAY_LENGTH(array) (sizeof(array)/sizeof(array[0]))

using namespace std;
using namespace android;

// Define Supported Feature Bitmap number
#define FEATURE_SLOT_ID 1

typedef struct {
    int mal;
    int ril;
} MalRequestInfo;

MalRequestInfo requestCommandMap[] = {
#include "mal_ril_commands.h"
};

MalRequestInfo unsolCommandMap[] = {
#include "mal_ril_unsol_commands.h"
};

RilMalClient::RilMalClient(int identity, char* socketName):RilClient(identity, socketName){
    version = 1;
    subVersion = 1;
    RFX_LOG_D(RFX_LOG_TAG, "Init MAL client version %d.%d", version, subVersion);
}

RilMalClient::~RilMalClient() {

}

void RilMalClient::addHeaderToResponse(Parcel* p, int slotId) {
    // Default: we have 4 bytes imply supported feature bit map
    int32_t headerSize = sizeof(int32_t);
    int32_t supportedFeatureBitMap = 0;
    int id;

    if (isMultipleImsSupport() || isMultipleImsSupport()) {
        id = slotId;
    } else {
        char prop_value[PROPERTY_VALUE_MAX] = {0};
        property_get(PROPERTY_3G_SIM, prop_value, "1");
        int capabilitySim = atoi(prop_value) - 1;
        id = capabilitySim;
    }
    RFX_LOG_D(RFX_LOG_TAG, "Add Header for MAL Solicited Response, id = %d", id);
    // Feature: slot id
    headerSize = headerSize + sizeof(id);
    supportedFeatureBitMap = supportedFeatureBitMap + FEATURE_SLOT_ID;

    p->writeInt32(headerSize);
    p->writeInt32 (supportedFeatureBitMap);
    p->writeInt32(id);
    return;
}

void RilMalClient::handleUnsolicited(int slotId, int unsolResponse, const void *data,
                                size_t datalen)
{

    int ret;
    // Default: we have 4 bytes imply supported feature bit map
    int32_t headerSize = 4;
    int32_t supportedFeatureBitMap = 0;

    // Feature: slot id
    headerSize = headerSize + sizeof(slotId);
    supportedFeatureBitMap = supportedFeatureBitMap + FEATURE_SLOT_ID;

    if (commandFd == -1) {
        RFX_LOG_E(RFX_LOG_TAG, "command Fd not ready here");
        return;
    }

    // if dual ims or dula ps => send each unsolicited to MAL
    // or send to MAL if and only if main capability SIM
    char prop_value[PROPERTY_VALUE_MAX] = {0};
    property_get(PROPERTY_3G_SIM, prop_value, "1");
    int capabilitySim = atoi(prop_value) - 1;
    // RFX_LOG_D(RFX_LOG_TAG, "capabilitySim = %d", capabilitySim);
    if (!isMultipleImsSupport() &&
            !isMultiPsAttachSupport() && capabilitySim != slotId) {
        RFX_LOG_D(RFX_LOG_TAG, "only handle capabilitySim");
        return;
    }

    Parcel p;
    p.writeInt32 (headerSize);
    p.writeInt32 (supportedFeatureBitMap);
    p.writeInt32 (slotId);
    p.writeInt32 (RESPONSE_UNSOLICITED);
    unsolResponse = remappingCommands(UNSOL_TO_MAL, unsolResponse);
    if (unsolResponse == -1) {
        RFX_LOG_D(RFX_LOG_TAG, "UNSOL_TO_MAL: unsol mapping fail, return");
        return;
    }
    RFX_LOG_D(RFX_LOG_TAG, "UNSOL_TO_MAL: remap to unsol resp(%d)", unsolResponse);
    p.writeInt32 (unsolResponse);
    ret = p.write(data, datalen);

    if (ret != 0) {
        RFX_LOG_E(RFX_LOG_TAG, "ret = %d, just return", ret);
    }

    RpRilClientController::sendResponse(p, commandFd);
    return;
}

void RilMalClient::processCommands(void *buffer, size_t buflen, int clientId) {
    Parcel* p =  new Parcel();
    Parcel* parcelToSend = new Parcel();
    status_t status;
    int32_t request = 0;
    int32_t token = 0;
    int32_t headerSize = 0;
    int32_t slotId = 0;

    p-> setData((uint8_t *) buffer, buflen);
    status = p -> readInt32(&headerSize);
    status = p -> readInt32(&slotId);
    p-> setDataPosition(headerSize + sizeof(int32_t));
    status = p -> readInt32(&request);
    status = p -> readInt32(&token);

    // start copy Parcel
    request = remappingCommands(MAL_TO_RIL, request);
    RFX_LOG_D(RFX_LOG_TAG, "MAL_TO_RIL: remap to ril request(%d)", request);
    if (request == -1) {
        RpRilClientController::onClientRequestComplete(slotId, token, RIL_E_REQUEST_NOT_SUPPORTED,
                NULL, 0, CLIENT_ID_MAL);
        RFX_LOG_E(RFX_LOG_TAG, "Rild does not support this RIL_REQUEST from MAL, fail cause=%d",
                RIL_E_REQUEST_NOT_SUPPORTED);
    } else {
        parcelToSend -> writeInt32(request);
        p->setDataPosition(headerSize + sizeof(int32_t) + sizeof(request));
        int remainSize = p -> dataAvail();
        char remainData[remainSize];
        status = p -> read(remainData, remainSize);
        status = parcelToSend -> write(remainData, remainSize);

        RFX_LOG_D(RFX_LOG_TAG, "enqueue request id %d with token %d for client %d", request, token,
                clientId);

        int unsolResponseIndex;
        int ret;
        RILD_RadioTechnology_Group dest = RfxDefaultDestUtils::getDefaultDest(request);

        if (dest == RADIO_TECH_GROUP_C2K) {
            int cdmaSlotId = RpRilClientController::getCdmaSlotId();
            RFX_LOG_D(RFX_LOG_TAG, "Default destination is cdma, enqueue to cdma slot %d",
                    cdmaSlotId);
            rfx_enqueue_client_request_message(request, cdmaSlotId, dest, token, parcelToSend, clientId);
        } else {
            RFX_LOG_D(RFX_LOG_TAG, "Default destination is gsm, enqueue to slot %d", slotId);
            rfx_enqueue_client_request_message(request, slotId, dest, token, parcelToSend, clientId);
        }
        if (p != NULL) {
            delete(p);
            p = NULL;
        }
    }
}

int32_t RilMalClient::remappingCommands(MappingSelection selection , int32_t cmd) {

    if (selection == MAL_TO_RIL) {
        int length = (int)(sizeof(requestCommandMap) / sizeof(MalRequestInfo));
        RFX_LOG_D(RFX_LOG_TAG, "MAL_TO_RIL: request command map length: %d, req: %d", length, cmd);
        for (int i = 0 ; i < length ; ++i) {
            if (cmd == requestCommandMap[i].mal) {
                return requestCommandMap[i].ril;
            }
        }
        RFX_LOG_D(RFX_LOG_TAG, "MAL_TO_RIL: request mapping fail");
    } else if (selection == UNSOL_TO_MAL) {
        int length = (int)(sizeof(unsolCommandMap) / sizeof(MalRequestInfo));
        // RFX_LOG_D(RFX_LOG_TAG, "UNSOL_TO_MAL: unsol command map length: %d, unsol: %d", length, cmd);
        for (int i = 0 ; i < length ; ++i) {
            if (cmd == unsolCommandMap[i].ril) {
                return unsolCommandMap[i].mal;
            }
        }
    }
    return -1;
}
