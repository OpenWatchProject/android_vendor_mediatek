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

#include "RmcNrSwitchHandler.h"

static const int request[] = {
    RFX_MSG_REQUEST_NR_SWITCH
};

#define MAX_NR_SWITCH_RETRY_TIME 40

// register data
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_NR_SWITCH);

// register handler to channel
RFX_IMPLEMENT_HANDLER_CLASS(RmcNrSwitchHandler, RIL_CMD_PROXY_9);

RmcNrSwitchHandler::RmcNrSwitchHandler(int slot_id, int channel_id) :
        RfxBaseHandler(slot_id, channel_id) {
    m_slot_id = slot_id;
    m_channel_id = channel_id;
    mRetryCount = 1;
    registerToHandleRequest(request, sizeof(request)/sizeof(int));
}

RmcNrSwitchHandler::~RmcNrSwitchHandler() {
}

void RmcNrSwitchHandler::onHandleRequest(const sp<RfxMclMessage>& msg) {
    // logD(LOG_TAG, "[onHandleRequest] %s", RFX_ID_TO_STR(msg->getId()));
    int request = msg->getId();
    switch(request) {
        case RFX_MSG_REQUEST_NR_SWITCH:
            requestNrSwitch(msg);
            break;
        default:
            logE(LOG_TAG, "Should not be here");
            break;
    }
}

void RmcNrSwitchHandler::requestNrSwitch(const sp<RfxMclMessage>& msg) {
    RIL_Errno ril_errno = RIL_E_SUCCESS;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> resp;
    int slot;

    int *pInt = (int *)msg->getData()->getData();
    slot = pInt[0];
    slot++;
    p_response = atSendCommand(String8::format("AT+EPSCONFIG=1,\"nr_ps\",%d", slot));
    if (p_response->getError() >= 0 && p_response->getSuccess() != 0) {
        logV(LOG_TAG, "requestNrSwitch ok");
        rfx_property_set("persist.vendor.radio.nrslot", String8::format("%d", slot).string());
        ril_errno = RIL_E_SUCCESS;
    } else {
        logE(LOG_TAG, "requestNrSwitch fail, count=%d, error=%d",
                mRetryCount, p_response->atGetCmeError());
        if (p_response->atGetCmeError() == CME_OPERATION_NOT_ALLOWED_ERR) {
            ril_errno = RIL_E_GENERIC_FAILURE;
            logE(LOG_TAG, "requestNrSwitch: fail, modem report no retry");
        } else {  // retry 40 times if Nr switch fail.
            if (mRetryCount < MAX_NR_SWITCH_RETRY_TIME) {
                mRetryCount++;
                sp<RfxMclMessage> mclMessage = RfxMclMessage::obtainRequest(msg->getId(),
                        msg->getData(), msg->getSlotId(), msg->getToken(),
                        msg->getSendToMainProtocol(), msg->getRilToken(), ms2ns(100),
                        msg->getTimeStamp(), msg->getAddAtFront());
                RfxMclDispatcherThread::enqueueMclMessageDelay(mclMessage);
                return;
            } else {
                char pErrMsg[] = "requestNrSwitch: retry max asserion!";
                mtkAssert(pErrMsg);
            }
        }
    }
    p_response = NULL;

finish:
    mRetryCount = 1;
    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), ril_errno,
            RfxVoidData(), msg, false);
    // response to TeleCore
    responseToTelCore(response);
}
