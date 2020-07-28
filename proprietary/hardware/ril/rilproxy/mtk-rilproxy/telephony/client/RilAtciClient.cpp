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
#include <cutils/properties.h>
#include <sys/socket.h>
#include "RfxDispatchThread.h"
#include "RfxBasics.h"
#include "RfxLog.h"
#include "RilAtciClient.h"

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
    int request = RIL_REQUEST_OEM_HOOK_ATCI_INTERNAL;
    int token = 0xFFFFFFFF;
    char prop[PROPERTY_VALUE_MAX] = {0};
    property_get(ATCI_SIM_PROP, prop, "0");

    int slotId = atoi(prop);
    RFX_LOG_I(RFX_LOG_TAG, "slotId %d", slotId);
    if (slotId == 9) {
        memset(prop, 0, sizeof(prop));
        property_get("persist.vendor.radio.cdma_slot", prop, "1");
        int cdmaSlotId = atoi(prop) - 1;
        slotId = RIL_SOCKET_ID(cdmaSlotId);
    } else if (slotId < 0 || slotId >= RIL_SOCKET_NUM) {
        RFX_LOG_E(RFX_LOG_TAG, "unsupported slot number");
        return;
    }

    char *atBuffer = (char*)calloc(1, buflen + 1);
    memcpy(atBuffer, buffer, buflen);
    Parcel* p = new Parcel();
    p->writeInt32(request);
    p->writeInt32(token);
    p->writeInt32(buflen + 1);
    p->write((void*) atBuffer, (size_t) buflen + 1);
    p->setDataPosition(0);
    RFX_LOG_I(RFX_LOG_TAG, "buffer %s, length %d", atBuffer, buflen);
    rfx_enqueue_client_request_message(request, slotId, RADIO_TECH_GROUP_GSM, token, p, clientId);
    free(atBuffer);
}

void RilAtciClient::handleUnsolicited(int slotId, int unsolResponse, const void *data,
        size_t datalen) {
    RFX_UNUSED(slotId);
    if (RIL_UNSOL_ATCI_RESPONSE == unsolResponse) {
        RFX_LOG_I(RFX_LOG_TAG, "data is %s, %d", (char *)data, datalen);
        if (data != NULL && datalen > sizeof(int)) {
            String8 str = String8((char*) data + sizeof(int));

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
