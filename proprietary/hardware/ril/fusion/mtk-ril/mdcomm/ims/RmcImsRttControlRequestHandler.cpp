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

#include "RmcImsRttControlRequestHandler.h"
#include "rfx_properties.h"
#include <telephony/mtk_ril.h>
#include <string>
#include <string.h>
#include "ConfigUtil.h"

static const int requests[] = {
    RFX_MSG_REQUEST_SEND_RTT_MODIFY_REQUEST,    // AT+RTTCALL
    RFX_MSG_REQUEST_SEND_RTT_TEXT,              // AT+RTTSTR
    RFX_MSG_REQUEST_RTT_MODIFY_REQUEST_RESPONSE,  // AT+PRTTCALL
    RFX_MSG_REQUEST_TOGGLE_RTT_AUDIO_INDICATION,  // AT+EIMSAUDIOSID
};

// register data
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_SEND_RTT_MODIFY_REQUEST);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringsData, RfxVoidData, RFX_MSG_REQUEST_SEND_RTT_TEXT);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_RTT_MODIFY_REQUEST_RESPONSE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_TOGGLE_RTT_AUDIO_INDICATION);

// register handler to channel
RFX_IMPLEMENT_HANDLER_CLASS(RmcImsRttControlRequestHandler, RIL_CMD_PROXY_1);

RmcImsRttControlRequestHandler::RmcImsRttControlRequestHandler(int slot_id, int channel_id) :
        RfxBaseHandler(slot_id, channel_id) {
    logD(RFX_LOG_TAG, "RmcImsRttControlRequestHandler constructor");
    registerToHandleRequest(requests, sizeof(requests)/sizeof(int));
}

RmcImsRttControlRequestHandler::~RmcImsRttControlRequestHandler() {
}

void RmcImsRttControlRequestHandler::onHandleRequest(const sp<RfxMclMessage>& msg) {
    //logD(RFX_LOG_TAG, "onHandleRequest: %d", msg->getId());
    int request = msg->getId();
    switch(request) {
        case RFX_MSG_REQUEST_SEND_RTT_MODIFY_REQUEST:
            sendRttModifyRequest(msg);
            break;
        case RFX_MSG_REQUEST_SEND_RTT_TEXT:
            sendRttText(msg);
            break;
        case RFX_MSG_REQUEST_RTT_MODIFY_REQUEST_RESPONSE:
            rttModifyRequestResponse(msg);
            break;
        case RFX_MSG_REQUEST_TOGGLE_RTT_AUDIO_INDICATION:
            toggleRttAudioIndication(msg);
            break;
        default:
            logE(RFX_LOG_TAG, "Should not be here");
            break;
    }
}

void RmcImsRttControlRequestHandler::sendRttModifyRequest(const sp<RfxMclMessage>& msg) {
    /* AT+RTTCALL=<call id>,<op>
     * <call id>: call id
     * <op>: 0 = downgrade; 1 = upgrade;
     */
    int *params = (int *)msg->getData()->getData();
    int callId = params[0];
    int op = params[1];

    logD(RFX_LOG_TAG, "sendRttModifyRequest callId:%d, op:%d", callId, op);

    // send AT command
    String8 cmd = String8::format("AT+RTTCALL=%d,%d", callId, op);

    handleCmdWithVoidResponse(msg, cmd);
}

void RmcImsRttControlRequestHandler::sendRttText(const sp<RfxMclMessage>& msg) {
    //AT+ERTTSTR= =<callid>, <len>, <text>
    char** params = (char**)msg->getData()->getData();
    char* callId = params[0];
    char* len = params[1];
    char* text = params[2];

    logD(RFX_LOG_TAG, "sendRttText callId:%s", callId);

    String8 cmd = String8::format("AT+ERTTSTR=%s,%s,\"%s\"", callId, len, text);

    handleCmdWithVoidResponse(msg, cmd);
}

void RmcImsRttControlRequestHandler::rttModifyRequestResponse(const sp<RfxMclMessage>& msg) {

    /* AT+PRTTCALL=<call id>, <result>
     */
    int *params = (int *)msg->getData()->getData();
    int callId = params[0];
    int result = params[1];

    logD(RFX_LOG_TAG, "rttModifyRequestResponse callId:%d, result: %d", callId, result);

    String8 cmd = String8::format("AT+PRTTCALL=%d,%d", callId, result);

    handleCmdWithVoidResponse(msg, cmd);
}

void RmcImsRttControlRequestHandler::toggleRttAudioIndication(const sp<RfxMclMessage>& msg) {
    /* AT+EIMSAUDIOSID=<call id>,<enable>
     * <call id>: call id
     * <enable>:
     *      0 = disable audio indication;
     *      1 = enable audio indication;
     */
    int *params = (int *)msg->getData()->getData();
    int callId = params[0];
    int enable = params[1];

    logD(RFX_LOG_TAG, "toggleRttAudioIndication callId:%d, enable:%d", callId, enable);

    // send AT command
    String8 cmd = String8::format("AT+EIMSAUDIOSID=%d,%d", callId, enable);

    handleCmdWithVoidResponse(msg, cmd);
}
