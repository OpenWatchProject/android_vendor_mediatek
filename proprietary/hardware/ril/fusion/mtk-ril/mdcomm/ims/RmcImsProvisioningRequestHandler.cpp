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

#include "RmcImsProvisioningRequestHandler.h"

static const int requests[] = {
    RFX_MSG_REQUEST_IMS_CONFIG_GET_MD_PROVISION, //AT+EPVSGET
    RFX_MSG_REQUEST_IMS_CONFIG_SET_PROVISION, //AT+EPVSSET
};

RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringData, RfxVoidData, RFX_MSG_REQUEST_IMS_CONFIG_GET_MD_PROVISION);


// register handler to channel
RFX_IMPLEMENT_HANDLER_CLASS(RmcImsProvisioningRequestHandler, RIL_CMD_PROXY_1);

RmcImsProvisioningRequestHandler::RmcImsProvisioningRequestHandler(int slot_id, int channel_id) :
        RfxBaseHandler(slot_id, channel_id) {
    logD(RFX_LOG_TAG, "RmcImsProvisioningRequestHandler constructor");
    registerToHandleRequest(requests, sizeof(requests)/sizeof(int));
}

RmcImsProvisioningRequestHandler::~RmcImsProvisioningRequestHandler() {
}

void RmcImsProvisioningRequestHandler::onHandleRequest(const sp<RfxMclMessage>& msg) {
    logD(RFX_LOG_TAG, "onHandleRequest: %d", msg->getId());
    int request = msg->getId();
    switch(request) {
        case RFX_MSG_REQUEST_IMS_CONFIG_GET_MD_PROVISION:
            requestGetProvisionValue(msg);
            break;
        case RFX_MSG_REQUEST_IMS_CONFIG_SET_PROVISION:
            requestSetProvisionValue(msg);
            break;
        default:
            logE(RFX_LOG_TAG, "Should not be here");
            break;
    }
}

void RmcImsProvisioningRequestHandler::requestGetProvisionValue(const sp<RfxMclMessage>& msg) {
    char *pvsString = (char *)msg->getData()->getData();
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> response;
    RIL_Errno rilErrNo = RIL_E_SUCCESS;
    // send AT command
    logD(RFX_LOG_TAG, "requestGetProvisionValue AT+EPVSGET=\"%s\"", pvsString);
    p_response = atSendCommand(String8::format("AT+EPVSGET=\"%s\"", pvsString));
    // set result
    if (p_response->getError() != 0 || p_response->getSuccess() != 1) {
        rilErrNo = RIL_E_GENERIC_FAILURE;

        if (p_response->getSuccess() != 1) {
            int err;
            RfxAtLine *p_cur;

            p_cur = p_response->getFinalResponse();
            if (p_cur->isFinalResponseErrorEx(m_channel_id)) {
                logD(RFX_LOG_TAG, "GetProvisionValue error =%s", p_response->getFinalResponse()->
                        getLine());
                if(strStartsWith(p_response->getFinalResponse()->getLine(), "+CME ERROR: 604")) {
                    rilErrNo = RIL_E_OEM_ERROR_24;
                }
            }
        }
    }
    response = RfxMclMessage::obtainResponse(msg->getId(), rilErrNo,
            RfxVoidData(), msg, false);
    // response to TeleCore
    responseToTelCore(response);
}

void RmcImsProvisioningRequestHandler::requestSetProvisionValue(const sp<RfxMclMessage>& msg) {
    const char **pvsStrings = (const char **)msg->getData()->getData();
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> response;
    RIL_Errno rilErrNo = RIL_E_SUCCESS;
    // send AT command
    logD(RFX_LOG_TAG, "requestSetProvisionValue AT+EPVSSET=\"%s\",\"%s\"",
            pvsStrings[0], pvsStrings[1]);
    if (pvsStrings[1] == NULL) {
        p_response = atSendCommand(String8::format("AT+EPVSSET=\"%s\",\"\"", pvsStrings[0]));
    } else {
        p_response = atSendCommand(String8::format("AT+EPVSSET=\"%s\",\"%s\"",
                pvsStrings[0], pvsStrings[1]));
    }
    // set result
    if (p_response->getError() != 0 || p_response->getSuccess() != 1) {
        rilErrNo = RIL_E_GENERIC_FAILURE;
    }
    response = RfxMclMessage::obtainResponse(msg->getId(), rilErrNo,
            RfxVoidData(), msg, false);
    // response to TeleCore
    responseToTelCore(response);
}
