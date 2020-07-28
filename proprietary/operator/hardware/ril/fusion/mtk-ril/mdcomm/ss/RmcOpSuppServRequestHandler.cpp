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

#include "RmcOpSuppServRequestHandler.h"
#include "RfxMessageId.h"
#include <telephony/mtk_ril.h>

#include <string.h>
#include <dlfcn.h>

RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringsData, RfxVoidData, RFX_MSG_REQUEST_SEND_USSI_FROM);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringData,  RfxVoidData, RFX_MSG_REQUEST_CANCEL_USSI_FROM);

static const int requests[] = {
    RFX_MSG_REQUEST_SEND_USSI_FROM,
    RFX_MSG_REQUEST_CANCEL_USSI_FROM,
};

// register handler to channel
RFX_IMPLEMENT_OP_HANDLER_CLASS(RmcOpSuppServRequestHandler, RIL_CMD_PROXY_2);

RmcOpSuppServRequestHandler::RmcOpSuppServRequestHandler(int slot_id,
        int channel_id):RfxBaseHandler(slot_id, channel_id){
    registerToHandleRequest(requests, sizeof(requests)/sizeof(int));
}

RmcOpSuppServRequestHandler::~RmcOpSuppServRequestHandler() {
}

void RmcOpSuppServRequestHandler::onHandleRequest(const sp<RfxMclMessage>& msg) {
    logD(TAG, "onHandleRequest: %d", msg->getId());
    int request = msg->getId();
    switch(request) {
        case RFX_MSG_REQUEST_SEND_USSI_FROM:
            requestSendUssiFrom(msg);
            break;

        case RFX_MSG_REQUEST_CANCEL_USSI_FROM:
            requestCancelUssiFrom(msg);
            break;

        default:
            logE(TAG, "Should not be here");
            break;
    }
}

void RmcOpSuppServRequestHandler::onHandleTimer() {
    // do something
}

void RmcOpSuppServRequestHandler::requestSendUssiFrom(const sp<RfxMclMessage>& msg) {

    const char** strings = (const char**) (msg->getData()->getData());
    sp<RfxAtResponse> p_response;
    const char* from = strings[0];
    int action = atoi(strings[1]);
    const char* ussi = strings[2];
    int err;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;

    logD(TAG, "requestSendUssiFrom: from=%s, action=%d, ussi=%s", from, action, ussi);

    p_response = atSendCommand(String8::format("AT+EIUSD=2,1,%d,\"%s\",\"en\",0,\"%s\"",
            action, ussi, from));

    err = p_response->getError();
    if (err < 0 || p_response->getSuccess() == 0) {
        logE(TAG, "requestSendUssiFrom Fail");
    } else {
        ret = RIL_E_SUCCESS;
    }
    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), ret,
            RfxVoidData(), msg, false);
    // response to TeleCore
    responseToTelCore(response);
}

void RmcOpSuppServRequestHandler::requestCancelUssiFrom(const sp<RfxMclMessage>& msg) {

    const char** strings = (const char**) (msg->getData()->getData());
    sp<RfxAtResponse> p_response;
    const char* from = strings[0];
    int err;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;

    logD(TAG, "requestCancelUssiFrom: from=%s", from);

    p_response = atSendCommand(String8::format("AT+EIUSD=2,2,2,\"\",\"en\",0,\"%s\"", from));

    err = p_response->getError();
    if (err < 0 || p_response->getSuccess() == 0) {
        logD(TAG, "requestCancelUssiFrom failed.");
    } else {
        ret = RIL_E_SUCCESS;
    }
    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), ret,
            RfxVoidData(), msg, false);
    // response to TeleCore
    responseToTelCore(response);
}

