/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
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

#include "RmcSuppServRequestNRTHandler.h"
#include "RfxMessageId.h"

#include <telephony/mtk_ril.h>
#include <mtkconfigutils.h>

RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxVoidData, RFX_MSG_REQUEST_GET_XCAP_STATUS);

static const int requests[] = {
    RFX_MSG_REQUEST_GET_XCAP_STATUS
};

// register handler to non real time channel
RFX_IMPLEMENT_HANDLER_CLASS(RmcSuppServRequestNRTHandler, RIL_CMD_PROXY_8);

RmcSuppServRequestNRTHandler::RmcSuppServRequestNRTHandler(int slot_id, int channel_id) :
    RfxBaseHandler(slot_id, channel_id) {
    registerToHandleRequest(requests, sizeof(requests)/sizeof(int));
}

RmcSuppServRequestNRTHandler::~RmcSuppServRequestNRTHandler() {
}

void RmcSuppServRequestNRTHandler::onHandleRequest(const sp<RfxMclMessage>& msg) {
    logD(TAG, "onHandleRequest: %d", msg->getId());
    int request = msg->getId();
    switch(request) {
        case RFX_MSG_REQUEST_GET_XCAP_STATUS:
            requestGetXcapStatus(msg);
            break;
        default:
            logE(TAG, "Should not be here");
            break;
    }
}

void RmcSuppServRequestNRTHandler::requestGetXcapStatus(const sp<RfxMclMessage>& msg) {
    sp<RfxAtResponse> p_response;
    int err;
    char* cmd = NULL;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;

    asprintf(&cmd, "AT+ECUSD=1,1,\"*#21#\",,,1");

    p_response = atSendCommandSingleline(cmd, "+CCFC:");
    free(cmd);

    err = p_response->getError();

    if (err < 0 || p_response == NULL) {
        logD(TAG, "requestGetXcapStatus failed.");
        goto error;
    }

    switch (p_response->atGetCmeError()) {
        case CME_SUCCESS:
            break;
        case CME_CALL_BARRED:
        case CME_OPR_DTR_BARRING:
            ret = RIL_E_CALL_BARRED;
            goto error;
            break;
        case CME_PHB_FDN_BLOCKED:
            ret = RIL_E_FDN_CHECK_FAILURE;
            goto error;
            break;
        case CME_403_FORBIDDEN:
            ret = RIL_E_UT_XCAP_403_FORBIDDEN;
            goto error;
            break;
        case CME_404_NOT_FOUND:
            ret = RIL_E_404_NOT_FOUND;
            goto error;
            break;
        case CME_409_CONFLICT: {
            ret = RIL_E_409_CONFLICT;
            goto error;
            break;
        }
        case CME_412_PRECONDITION_FAILED:
            ret = RIL_E_412_PRECONDITION_FAILED;
            goto error;
            break;
        case CME_NETWORK_TIMEOUT:
            ret = RIL_E_UT_UNKNOWN_HOST;
            goto error;
            break;
        case CME_OPERATION_NOT_SUPPORTED:
            ret = RIL_E_REQUEST_NOT_SUPPORTED;
            goto error;
            break;
        case CME_844_UNPROVISIONED:
            ret = RIL_E_REQUEST_NOT_SUPPORTED;
            goto error;
            break;
        case CME_845_NO_DATA_CONNECTION:
            ret = RIL_E_845_NO_DATA_CONNECTION;
            goto error;
            break;
        default:
            goto error;
    }

    /* return success here */
    ret = RIL_E_SUCCESS;

error:
    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), ret,
            RfxVoidData(), msg, false);

    // response to TeleCore
    responseToTelCore(response);
}
