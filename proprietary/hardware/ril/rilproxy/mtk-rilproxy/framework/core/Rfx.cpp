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
 * File name:  rfx.cpp
 * Author: Jun Liu (MTK80064)
 * Description:
 * Implement interfaces to initialize RIL Proxy framework and enqueue messages from
 * external modules
 */


/*****************************************************************************
 * Include
 *****************************************************************************/

#include "Rfx.h"
#include "RfxMainThread.h"
#include "RfxRilAdapter.h"
#include "RfxDispatchThread.h"
#include "RfxSocketStateManager.h"
#include "RfxLog.h"
#include "SapSocketManager.h"
#include "RfxTransferUtils.h"
#include <libmtkrilutils.h>
#include <binder/ProcessState.h>

/*****************************************************************************
 * Implementation
 *****************************************************************************/
RfxDispatchThread *dispatchThread;

#define RFX_LOG_TAG "RFX"

int RFX_SLOT_COUNT = 2;

void rfx_init(void) {
    android::ProcessState::initWithDriver("/dev/vndbinder");
    RFX_SLOT_COUNT = getSimCount();
    RFX_LOG_D(RFX_LOG_TAG, "rfx_init (RFX_SLOT_COUNT=%d, SIM_COUNT=%d)", RFX_SLOT_COUNT, SIM_COUNT);
    RfxMainThread::init();
    dispatchThread = RfxDispatchThread::init();
}

void rfx_enqueue_request_message(int request, void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID socket_id) {
    dispatchThread->enqueueRequestMessage(request, data, datalen, t, socket_id);
}

void rfx_enqueue_request_message_old(int request, int slotId,
            RILD_RadioTechnology_Group dest, int token, void *data) {
    // RLOGD("[RilProxy] rfx_enqueue_request_message: request = %d, slotId = %d, dest = %d, token = %d"
    //        , request, slotId, dest, token);
    dispatchThread->enqueueRequestMessageOld(request, slotId, dest, token, data);
}

void rfx_enqueue_client_request_message(int request, int slotId,
            RILD_RadioTechnology_Group dest, int token, void *data, int clientId) {
    /* Log too much: Can check enqueueRequestMessage() in RfxDispatchThread.
    RFX_LOG_D(RFX_LOG_TAG, "rfx_enqueue_request_message: request = %d, slotId = %d, dest = %d,\
            token = %d, clienId = %d", request, slotId, dest, token, clientId);
    */
    //RfxDispatchThread::enqueueRequestMessage(request, slotId, dest, token, data);
    dispatchThread->enqueueRequestMessage(request, slotId, dest, token, data, clientId);
}

void rfx_process_raw_data(RILD_RadioTechnology_Group source, int slotId, void *data,
        int datalen) {
    //RLOGD("rfx_process_raw_data");
    Parcel *parcel = RfxRilAdapterUtils::extractResponse((uint8_t*) data, datalen);
    int32_t status = 0;
    int32_t type = 0;
    int32_t token = 0;
    int32_t error = 0;
    int32_t urcId = 0;

    status =  parcel->readInt32(&type);
    switch (type) {
        case RESPONSE_SOLICITED_ACK:
            status =  parcel->readInt32(&token);
            parcel->setDataPosition(0);
            // RFX_LOG_D(RFX_LOG_TAG, "rfx_process_raw_data: request ack: token = %d", token);
            dispatchThread->enqueueRequestAckMessage(source, slotId, token, parcel);
            break;
        case RESPONSE_SOLICITED:
        case RESPONSE_SOLICITED_ACK_EXP:
            status =  parcel->readInt32(&token);
            status =  parcel->readInt32(&error);
            parcel->setDataPosition(0);
            // RFX_LOG_D(RFX_LOG_TAG, "rfx_process_raw_data: response: token = %d", token);
            dispatchThread->enqueueResponseMessage(source, slotId, token, (RIL_Errno) error,
                    parcel);
            break;
        case RESPONSE_UNSOLICITED:
        case RESPONSE_UNSOLICITED_ACK_EXP:
            status = parcel->readInt32(&urcId);
            parcel->setDataPosition(0);
            // RFX_LOG_D(RFX_LOG_TAG, "rfx_process_raw_data: Urc: urcId = %d", urcId);
            dispatchThread->enqueueUrcMessage(source, slotId, urcId, parcel);
            break;
        default:
            RFX_LOG_D(RFX_LOG_TAG, "rfx_process_raw_data: Should not be here");
            break;
    }
}

void rfx_set_socket(RILD_RadioTechnology_Group group, int slotId, int fd) {
    RFX_LOG_D(RFX_LOG_TAG, "rfx_set_socket");
    RfxSocketStateManager::notifySocketState(group, slotId, fd, true);
}

void rfx_close_socket(RILD_RadioTechnology_Group group, int slotId) {
    RFX_LOG_D(RFX_LOG_TAG, "rfx_close_socket");
    RfxSocketStateManager::notifySocketState(group, slotId, -1, false);
}

void rfx_set_sap_socket(RIL_SOCKET_ID socketId, RILD_RadioTechnology_Group group, int fd) {
    SapSocketManager::setSapSocket(socketId, group, fd);
}

void rfx_sap_to_rild_socket(void *data, int datalen, RIL_SOCKET_ID socketId) {
    SapSocketManager::sendSapMessageToRild(data, datalen, socketId);
}

void rfx_update_connection_state(RIL_SOCKET_ID socketId, int isConnected) {
    dispatchThread->updateConnectionState(socketId, isConnected);
}