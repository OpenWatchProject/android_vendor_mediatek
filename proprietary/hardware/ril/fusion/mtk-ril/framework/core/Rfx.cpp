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
#include "RfxLog.h"
#include "SapSocketManager.h"
#include "RfxMclDispatcherThread.h"
#include "RfxDataCloneManager.h"
#include "RfxMclMessage.h"
#include "RfxViaUtils.h"
#include <binder/ProcessState.h>
#include <libmtkrilutils.h>
#include "RfxOpUtils.h"
#include "RfxGwsdUtils.h"


/*****************************************************************************
 * Implementation
 *****************************************************************************/
RfxDispatchThread *dispatchThread;

#define RFX_LOG_TAG "RFX"

int RFX_SLOT_COUNT = 2;

void rfx_init(void) {
#ifdef MTK_USE_HIDL
    android::ProcessState::initWithDriver("/dev/vndbinder");
#endif
    RFX_SLOT_COUNT = getSimCount();
    RFX_LOG_D(RFX_LOG_TAG, "rfx_init (RFX_SLOT_COUNT=%d, SIM_COUNT=%d)", RFX_SLOT_COUNT, SIM_COUNT);

    // init operator library for operator software decouple
    RfxOpUtils::initOpLibrary();
    // init ViaHandler
    RfxViaUtils::initViaHandler();
    // init MdComm layer
    RfxMclDispatcherThread::init();
    // init TelCore layer
    RfxMainThread::init();
    dispatchThread = RfxDispatchThread::init();
    // init Gwsd
    RfxGwsdUtils::initHandler();
}

void rfx_enqueue_request_message(int request, void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID socketId) {
    char property_value[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get("persist.vendor.ril.test_mode", property_value, "0");
    if (atoi(property_value) == 1) {
        RFX_LOG_D(RFX_LOG_TAG, "test mode on, return not support : %d", request);
        RfxRequestInfo *requestInfo = (RfxRequestInfo *)t;
        sp<RfxMessage> resMsg = RfxMessage::obtainResponse(socketId, request, requestInfo->token,
                INVALID_ID, -1, RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0, t);
        MessageObj *obj = createMessageObj(resMsg);
        dispatchResponseQueue.enqueue(obj);
        return;
    }
    dispatchThread->enqueueRequestMessage(request, data, datalen, t, socketId);
}

void rfx_enqueue_request_message_client(int request, void *data, size_t datalen,
        RIL_Token t, RIL_SOCKET_ID socketId) {
    char property_value[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get("persist.vendor.ril.test_mode", property_value, "0");
    RfxRequestInfo *requestInfo = (RfxRequestInfo *)t;
    int clientId = requestInfo->clientId;
    if (atoi(property_value) == 1 && (clientId != CLIENT_ID_MTTS1
            && clientId != CLIENT_ID_MTTS2 && clientId != CLIENT_ID_MTTS3
            && clientId != CLIENT_ID_MTTS4)) {
        RFX_LOG_D(RFX_LOG_TAG, "test mode but not MTTS request, return not support : %d", request);
        sp<RfxMessage> resMsg = RfxMessage::obtainResponse(socketId, request, requestInfo->token,
                INVALID_ID, -1, RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0, t);
        MessageObj *obj = createMessageObj(resMsg);
        dispatchResponseQueue.enqueue(obj);
        return;
    }
    dispatchThread->enqueueRequestMessage(request, data, datalen, t, socketId, clientId);
}

void rfx_enqueue_sap_request_message(int request, void *data, size_t datalen,
        RIL_Token t, RIL_SOCKET_ID socketId) {
   char property_value[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get("persist.vendor.ril.test_mode", property_value, "0");
    if (atoi(property_value) == 1) {
        RFX_LOG_D(RFX_LOG_TAG, "test mode on, return not support : %d", request);
        RfxRequestInfo *requestInfo = (RfxRequestInfo *)t;
        sp<RfxMessage> resMsg = RfxMessage::obtainResponse(socketId, request, requestInfo->token,
                INVALID_ID, -1, RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0, t);
        MessageObj *obj = createMessageObj(resMsg);
        dispatchResponseQueue.enqueue(obj);
        return;
    }
   dispatchThread->enqueueSapRequestMessage(request, data, datalen, t, socketId);
}

void rfx_update_connection_state(RIL_SOCKET_ID socketId, int isConnected) {
    dispatchThread->updateConnectionState(socketId, isConnected);
}
