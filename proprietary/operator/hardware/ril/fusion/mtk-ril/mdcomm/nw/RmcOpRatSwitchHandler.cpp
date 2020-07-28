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

#include "RmcOpRatSwitchHandler.h"

static const int request[] = {
    RFX_MSG_REQUEST_SET_DISABLE_2G,
    RFX_MSG_REQUEST_GET_DISABLE_2G
};

// register handler to channel
RFX_IMPLEMENT_OP_HANDLER_CLASS(RmcOpRatSwitchHandler, RIL_CMD_PROXY_9);

RmcOpRatSwitchHandler::RmcOpRatSwitchHandler(int slot_id, int channel_id) :
        RmcNetworkHandler(slot_id, channel_id) {
    registerToHandleRequest(request, sizeof(request)/sizeof(int));
    logV(LOG_TAG, "[RmcOpRatSwitchHandler] init");
}

RmcOpRatSwitchHandler::~RmcOpRatSwitchHandler() {
}

void RmcOpRatSwitchHandler::onHandleRequest(const sp<RfxMclMessage>& msg) {
    logD(LOG_TAG, "[onHandleRequest] %s", RFX_ID_TO_STR(msg->getId()));
    int request = msg->getId();
    switch(request) {
        case RFX_MSG_REQUEST_SET_DISABLE_2G:
            requestSetDisable2G(msg);
            break;
        case RFX_MSG_REQUEST_GET_DISABLE_2G:
            requestGetDisable2G(msg);
            break;
        default:
            logE(LOG_TAG, "Should not be here");
            break;
    }
}

void RmcOpRatSwitchHandler::requestSetDisable2G(const sp<RfxMclMessage>& msg) {
    RIL_Errno ril_errno = RIL_E_MODE_NOT_SUPPORTED;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> resp;
    int mode = 0;

    int *pInt = (int *)msg->getData()->getData();
    mode = pInt[0];
    if (mode == 1 || mode == 0) {
        if (isOp07Support()) {
            logD(LOG_TAG, "[OP07] requestSetDisable2G %d", mode);
            p_response = atSendCommand(String8::format("AT+EDRAT=%d", mode));
            if (p_response->getError() >= 0 && p_response->getSuccess() != 0) {
                rfx_property_set("persist.vendor.radio.disable.2g",
                        String8::format("%d", mode).string());
                ril_errno = RIL_E_SUCCESS;
            }
        } else if (isOp08Support()) {
            logD(LOG_TAG, "[OP08] requestSetDisable2G %d", mode);
            p_response = atSendCommand(String8::format("AT+EGMC=1,\"rat_disable_with_ecc\",%d", mode));
            if (p_response->getError() >= 0 && p_response->getSuccess() != 0) {
                ril_errno = RIL_E_SUCCESS;
            }
        } else {
            logD(LOG_TAG, "NOT SUPPORT, requestSetDisable2G %d", mode);
        }
    }
    p_response = NULL;

error:
    resp = RfxMclMessage::obtainResponse(msg->getId(), ril_errno,
            RfxVoidData(), msg, false);
    responseToTelCore(resp);
}

void RmcOpRatSwitchHandler::requestGetDisable2G(const sp<RfxMclMessage>& msg) {
    RIL_Errno ril_errno = RIL_E_MODE_NOT_SUPPORTED;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> resp;
    RfxAtLine* line;
    int err=0, mode=0;
    char *type=NULL;
    char property_value[RFX_PROPERTY_VALUE_MAX] = {0};

    if (isOp07Support()) {
        rfx_property_get("persist.vendor.radio.disable.2g", property_value, "0");
        mode = atoi(property_value);
        logD(LOG_TAG, "[OP07] requestGetDisable2G %d", mode);
        ril_errno = RIL_E_SUCCESS;
    } else if (isOp08Support()) {
        p_response = atSendCommandSingleline("AT+EGMC=0,\"rat_disable_with_ecc\"", "+EGMC:");

        err = p_response->getError();
        if (err < 0 || p_response->getSuccess() == 0)
            goto error;

        line = p_response->getIntermediates();

        // go to start position
        line->atTokStart(&err);
        if(err < 0) goto error;

        // get "type"
        type = line->atTokNextstr(&err);
        if(err < 0) goto error;

        if (strncmp(type, "rat_disable_with_ecc", strlen("rat_disable_with_ecc")) != 0) {
            goto error;
        }

        // get <mode>
        mode = line->atTokNextint(&err);
        if(err < 0) goto error;

        logD(LOG_TAG, "[OP08] requestGetDisable2G %d", mode);
        ril_errno = RIL_E_SUCCESS;
    } else {
        logD(LOG_TAG, "NOT SUPPORT, requestGetDisable2G %d", mode);
    }

error:
    resp = RfxMclMessage::obtainResponse(msg->getId(), ril_errno,
            RfxIntsData(&mode, 1), msg, false);
    responseToTelCore(resp);
}
