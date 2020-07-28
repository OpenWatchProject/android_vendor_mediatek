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

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxVoidData.h"
#include "RfxIntsData.h"

#include "RmcVtCommonReqHandler.h"

#include "RfxVtSendMsgData.h"
#include "RmcVtMsgParser.h"

// for send MCL request to myself
#include "RfxMclDispatcherThread.h"
#include "RfxMclMessage.h"
#include "RfxMessageId.h"

// for socket / thread
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cutils/properties.h>
#include <cutils/sockets.h>
#include <sys/socket.h>
#include <sys/un.h>

#define RFX_LOG_TAG "VT RIL CMN RMC"

/*****************************************************************************
 * Class RmcVtCommonReqHandler
 *****************************************************************************/
RFX_IMPLEMENT_HANDLER_CLASS(RmcVtCommonReqHandler, RIL_CMD_PROXY_2);

RmcVtCommonReqHandler::RmcVtCommonReqHandler(int slot_id, int channel_id)
: RfxBaseHandler(slot_id, channel_id) {

    RFX_LOG_I(RFX_LOG_TAG, "[RMC VT REQ HDLR] RmcVtCommonReqHandler create (slot_id = %d)", slot_id);

    const int CommonEventList[] = {
        RFX_MSG_EVENT_REPORT_ANBR,
    };

    registerToHandleEvent(CommonEventList, sizeof(CommonEventList) / sizeof(int));

    const int requestList[] = {
        RFX_MSG_REQUEST_ENABLE_ANBR,
    };

    registerToHandleRequest(requestList, sizeof(requestList)/sizeof(int));
}

RmcVtCommonReqHandler::~RmcVtCommonReqHandler() {
}

void RmcVtCommonReqHandler::onHandleRequest(const sp<RfxMclMessage>& msg) {

    int request = msg->getId();

    RFX_LOG_I(RFX_LOG_TAG, "[Handle REQ] request = %d", request);

    switch (request) {
        case RFX_MSG_REQUEST_ENABLE_ANBR:
            RFX_LOG_I(RFX_LOG_TAG, "[Handle REQ] RFX_MSG_REQUEST_ENABLE_ANBR");
            handleEnableANBR(msg);
            break;
        default:
            RFX_LOG_I(RFX_LOG_TAG, "[Handle REQ] unknown request, ignore!");
            break;
    }
}

void RmcVtCommonReqHandler::onHandleEvent(const sp<RfxMclMessage> & msg) {

    int event = msg->getId();

    RFX_LOG_I(RFX_LOG_TAG, "[Handle EVT] event = %d", event);

    switch (event) {
        case RFX_MSG_EVENT_REPORT_ANBR:
            RFX_LOG_I(RFX_LOG_TAG, "[Handle EVT] RFX_MSG_EVENT_REPORT_ANBR");
            handleReportANBR(msg);
            break;

        default:
            RFX_LOG_I(RFX_LOG_TAG, "[Handle EVT] unknown event, ignore!");
            break;
    }
}

void RmcVtCommonReqHandler::handleReportANBR(const sp<RfxMclMessage>& msg) {

    char* cmd = NULL;
    sp<RfxAtResponse> p_response;

    int *pInt = (int *)msg->getData()->getData();
    int is_ul = pInt[0];
    int ebi = pInt[1];
    int bitrate = pInt[2];
    int bearer_id = pInt[3];
    int pdu_session_id = pInt[4];
    int ext_param = pInt[5];; //for future usage

    RFX_LOG_I(RFX_LOG_TAG, "[handleReportANBR] is_ul = %d, ebi = %d, bitrate = %d, bearer_id = %d, pdu_session_id = %d, ext_param = %d\n",
            is_ul, ebi, bitrate, bearer_id, pdu_session_id, ext_param);

    asprintf(&cmd, "AT+EANBR=2,%d,%d,%d,%d,%d,%d", ebi, is_ul, bitrate, bearer_id, pdu_session_id, ext_param);
    //test
    //asprintf(&cmd, "ATI");

    // send AT command
    p_response = atSendCommand(cmd);

    free(cmd);
    cmd = NULL;

    if (p_response->getSuccess() != 1) {
        RFX_LOG_I(RFX_LOG_TAG, "[handleReportANBR] response error");
    }

}

void RmcVtCommonReqHandler::handleEnableANBR(const sp<RfxMclMessage>& msg) {

    char* cmd = NULL;
    sp<RfxAtResponse> p_response;

    int *pInt = (int *)msg->getData()->getData();
    int enable = pInt[0];

    RFX_LOG_I(RFX_LOG_TAG, "[handleEnableANBR] enable = %d\n", enable);

    asprintf(&cmd, "AT+EANBR=%d", enable);

    // send AT command
    p_response = atSendCommand(cmd);

    free(cmd);
    cmd = NULL;

    if (p_response->getSuccess() != 1) {
        RFX_LOG_I(RFX_LOG_TAG, "[handleEnableANBR] response error");
    }
}
