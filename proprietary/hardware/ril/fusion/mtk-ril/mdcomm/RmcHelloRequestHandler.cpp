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

#include "RmcHelloRequestHandler.h"
#include <telephony/mtk_ril.h>

// register handler to channel
RFX_IMPLEMENT_HANDLER_CLASS(RmcHelloRequestHandler, RIL_CMD_PROXY_1);

RmcHelloRequestHandler::RmcHelloRequestHandler(int slot_id, int channel_id) :
        RfxBaseHandler(slot_id, channel_id) {
    const int request1[] = {
        RIL_REQUEST_RADIO_POWER,
    };
    const int event[] = {
        RFX_MSG_EVENT_HELLO_NOTIFY_MODEM_STATE,
    };

    registerToHandleRequest(request1, sizeof(request1)/sizeof(int));
    registerToHandleEvent(event, sizeof(event)/sizeof(int));
}

RmcHelloRequestHandler::~RmcHelloRequestHandler() {
}

void RmcHelloRequestHandler::onHandleRequest(const sp<RfxMclMessage>& msg) {
    logD(LOG_TAG, "onHandleRequest: %d", msg->getId());
    int request = msg->getId();
    switch(request) {
        case RIL_REQUEST_RADIO_POWER:
            requestRadioPower(msg);
            break;
        case RIL_REQUEST_GET_SIM_STATUS:
            // do something
            break;
        default:
            logE(LOG_TAG, "Should not be here");
            break;
    }
}

void RmcHelloRequestHandler::requestRadioPower(const sp<RfxMclMessage>& msg) {
    int requestId = msg->getId();
    int *pInt = (int *)msg->getData()->getData();
    bool power = pInt[0]? true: false;
    int err;
    sp<RfxAtResponse> p_response;

    // send AT command
    // Case1: parameter is constant
    if (power) {
        p_response = atSendCommand("AT+EFUN=3");
    } else {
        p_response = atSendCommand("AT+EFUN=0");
    }
    // Case2: parameter is varible
    // framework provides other interface: atSendCommand(const String8 &command)
    // If we want to send comand with varible, we can use this API and avoid to manage memory (use asprintf)
    p_response = atSendCommand(String8::format("AT+EFUN = %d", 0));

    // check error
    err = p_response->getError();
    if (err != 0) {
        // do something
        // for example, error handle
    }

    RfxAtLine* line1 = p_response->getIntermediates();
    // go to start position
    line1->atTokStart(&err);
    int dataLength = line1->atTokNextint(&err);
    // second line
    RfxAtLine* line2 = p_response->getIntermediates()->getNext();
    line2->atTokStart(&err);
    const char* dataContent = line2->atTokChar(&err);

    const RfxAtLine* final = p_response->getFinalResponse();

    // NOTE: This response should correspending to response function.
    //           Please use RfxBaseData as parameter. This can avoid to manage memory.
    // Assume we want to send Strings to Telcore
    char *data = "foo";
    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxStringData(data), msg, true);

    // response to TeleCore
    responseToTelCore(response);

    // other case. data structure is RIL_Foo
    // RIL_Foo foo;
    // p_response->getIntermediates()->atTokStart(&err);
    // foo.timeout = p_response->getIntermediates()->atTokNextint(&err);
    // foo.power = p_response->getIntermediates()->atTokNextint(&err);
    // sp<RfxMclMessage> response_foo = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
    //         RfxFooData(foo.timeout, foo.power), msg, true);

    // Assuem module want to reply something error
    sp<RfxMclMessage> response_err = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_CANCELLED,
            RfxIntsData(), msg, true);

}

void RmcHelloRequestHandler::modemStateChange(const sp<RfxMclMessage>& msg) {
    int modemState = ((int *) msg->getData()->getData())[0];

    // do something
}

void RmcHelloRequestHandler::onHandleTimer() {
    // do something
}

void RmcHelloRequestHandler::onHandleEvent(const sp<RfxMclMessage>& msg) {
    int id = msg->getId();
    switch(id) {
        case RFX_MSG_EVENT_HELLO_NOTIFY_MODEM_STATE:
            modemStateChange(msg);
            break;
        default:
            logE(LOG_TAG, "should not be here");
            break;
    }
}
