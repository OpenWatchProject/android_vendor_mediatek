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
#include <telephony/mtk_ril.h>

// MTK fusion include
#include "RfxVoidData.h"
#include "RfxIntsData.h"

// Local include
#include "RmcCallControlInterface.h"
#include "RmcCallControlSpecialRequestHandler.h"

#define RFX_LOG_TAG "RmcCCReqHandler"

// register handler to channel
RFX_IMPLEMENT_HANDLER_CLASS(RmcCallControlSpecialRequestHandler, RIL_CMD_PROXY_1);

RmcCallControlSpecialRequestHandler::RmcCallControlSpecialRequestHandler(int slot_id,
        int channel_id) : RmcCallControlBaseHandler(slot_id, channel_id) {
    const int requests[] = {
        RFX_MSG_REQUEST_FORCE_RELEASE_CALL,                           //AT+ECHUP
        RFX_MSG_REQUEST_SET_VOICE_DOMAIN_PREFERENCE,                  //AT+CEVDP
        RFX_MSG_REQUEST_GET_VOICE_DOMAIN_PREFERENCE,                  //AT+CEVDP?
    };

    registerToHandleRequest(requests, sizeof(requests)/sizeof(int));
}

RmcCallControlSpecialRequestHandler::~RmcCallControlSpecialRequestHandler() {
}

void RmcCallControlSpecialRequestHandler::onHandleRequest(const sp<RfxMclMessage>& msg) {
    logD(RFX_LOG_TAG, "onHandleRequest: %d", msg->getId());
    int request = msg->getId();
    switch(request) {
        case RFX_MSG_REQUEST_FORCE_RELEASE_CALL:
            requestForceReleaseCall(msg);
            break;
        case RFX_MSG_REQUEST_SET_VOICE_DOMAIN_PREFERENCE:
            requestSetVoiceDomainPreference(msg);
            break;
        case RFX_MSG_REQUEST_GET_VOICE_DOMAIN_PREFERENCE:
            requestGetVoiceDomainPreference(msg);
            break;
        default:
            logE(RFX_LOG_TAG, "Should not be here");
            break;
    }
}

void RmcCallControlSpecialRequestHandler::onHandleTimer() {
    // do something
}

void RmcCallControlSpecialRequestHandler::requestForceReleaseCall(const sp<RfxMclMessage>& msg) {
    /* +ECHUP=<call_id>
     * <call_id>: hangup call id
     */
    int *params = (int *)msg->getData()->getData();

    char* atCmd = AT_FORCE_RELEASE;
    int callId = params[0];

    String8 cmd = String8::format("%s=%d", atCmd, callId);
    handleCmdWithVoidResponse(msg, cmd);
}

void RmcCallControlSpecialRequestHandler::requestSetVoiceDomainPreference(const sp<RfxMclMessage>& msg) {
    int *params = (int *)msg->getData()->getData();
    char *cmd;
    sp<RfxAtResponse> p_response;
    RIL_Errno rilErrNo = RIL_E_SUCCESS;

    // send AT command
    // AT+CEVDP=<setting>
    //  1:  CS voice only
    //  2:  CS voice preferred, IMS PS voice as secondary
    //  3:  IMS PS voice preferred, CS voice as secondary
    //  4:  IMS PS voice only
    logD(RFX_LOG_TAG, "requestSetVoiceDomainPreference settings: %d", params[0]);
    p_response = atSendCommand(String8::format("AT+CEVDP=%d",params[0]));

    // set result
    if (p_response->getError() != 0 || p_response->getSuccess() != 1) {
        rilErrNo = RIL_E_GENERIC_FAILURE;
    }

    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), rilErrNo,
            RfxVoidData(), msg, false);

    // response to TeleCore
    responseToTelCore(response);
}

void RmcCallControlSpecialRequestHandler::requestGetVoiceDomainPreference(const sp<RfxMclMessage>& msg) {
    sp<RfxMclMessage> responseMsg;
    sp<RfxAtResponse> p_response = NULL;
    int err;
    int vps = 0;
    char *tmp;
    RfxAtLine *line = NULL;

    p_response = atSendCommandSingleline("AT+CEVDP?", "+CEVDP:");

    /* +CIREG: <vdp> */
    err = p_response->getError();
    if (err < 0 || p_response->getSuccess() == 0) {
        goto error;
    }

    line = p_response->getIntermediates();
    line->atTokStart(&err);
    if (err < 0) goto error;

    vps = line->atTokNextint(&err);
    if (err < 0) goto error;

    responseMsg = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxIntsData((void*)&vps, sizeof(int)), msg, false);
    // response to TeleCore
    responseToTelCore(responseMsg);
    return;
error:
    logE(RFX_LOG_TAG, "requestGetVoiceDomainPreference error");
    responseMsg = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
            RfxIntsData((void*)&vps, sizeof(int)), msg, false);
    // response to TeleCore
    responseToTelCore(responseMsg);
}

