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

#include "RmcRadioRelatedRequestHandler.h"
#include "RfxStringsData.h"
#include "RfxVoidData.h"

#define RFX_LOG_TAG "RmcRadioRelReqHandler"

// register handler to channel
RFX_IMPLEMENT_HANDLER_CLASS(RmcRadioRelatedRequestHandler, RIL_CMD_PROXY_9);

RmcRadioRelatedRequestHandler::RmcRadioRelatedRequestHandler(int slot_id, int channel_id) :
        RfxBaseHandler(slot_id, channel_id) {
    logD(RFX_LOG_TAG, "RmcRadioRelatedRequestHandler constructor");
    // For commands that need to send on same channel as EFUN, usally before EFUN=1
    const int requests[] = {
        RFX_MSG_REQUEST_SET_ECC_MODE,            //AT+EMCS
    };

    registerToHandleRequest(requests, sizeof(requests)/sizeof(int));
}

RmcRadioRelatedRequestHandler::~RmcRadioRelatedRequestHandler() {
}

void RmcRadioRelatedRequestHandler::onHandleRequest(const sp<RfxMclMessage>& msg) {
    logD(RFX_LOG_TAG, "onHandleRequest: %d", msg->getId());
    int request = msg->getId();
    switch (request) {
        case RFX_MSG_REQUEST_SET_ECC_MODE:
            requestSetEccMode(msg);
            break;

        default:
            logE(RFX_LOG_TAG, "Should not be here");
            break;
    }
}

void RmcRadioRelatedRequestHandler::requestSetEccMode(const sp<RfxMclMessage>& msg) {
    sp<RfxAtResponse> p_response;
    RIL_Errno rilErrNo = RIL_E_SUCCESS;

    RfxStringsData* data = (RfxStringsData*)msg->getData();
    char** params = (char**)data->getData();
    int enable = atoi(params[1]);
    int airplaneMode = atoi(params[2]);
    //int imsReg = atoi(params[3]);
    /*
    AT+EMCS:<emc_session>,<airplane_mode>
    <emc_session> :  1 : emergency session begin
    <airplaine_mode> : 0 : off, 1 : on
    */

    bool currentMode = getNonSlotMclStatusManager()->getBoolValue(
            RFX_STATUS_KEY_EMERGENCY_MODE_IN_FLIGHT_MODE, false);
    logD(RFX_LOG_TAG, "currentMode=%d, enable=%d, airplane=%d", currentMode, enable, airplaneMode);

    if (airplaneMode == 0 || (enable != 0 && enable != 1)) {
        logD(RFX_LOG_TAG, "invalid param");
        rilErrNo = RIL_E_INTERNAL_ERR;
    } else if ((currentMode && enable == 1) || (!currentMode && enable == 0)) {
        logD(RFX_LOG_TAG, "duplicate request");
        rilErrNo = RIL_E_INTERNAL_ERR;
    } else {
        if (enable == 1) {
            if (RfxRilUtils::isEmciSupport()) {
                p_response = atSendCommand(String8::format("AT+EMCI=1"));
            } else {
                p_response = atSendCommand(String8::format("AT+EMCS=1,%d", airplaneMode));
            }
            getNonSlotMclStatusManager()->setBoolValue(
                    RFX_STATUS_KEY_EMERGENCY_MODE_IN_FLIGHT_MODE, true);
        } else {
            if (RfxRilUtils::isEmciSupport()) {
                p_response = atSendCommand(String8::format("AT+EMCI=0"));
            } else {
                p_response = atSendCommand(String8::format("AT+EMCS=0"));
            }
            getNonSlotMclStatusManager()->setBoolValue(
                    RFX_STATUS_KEY_EMERGENCY_MODE_IN_FLIGHT_MODE, false);
        }
        // set result
        if (p_response->getError() != 0 || p_response->getSuccess() != 1) {
            rilErrNo = RIL_E_INTERNAL_ERR;
        }
    }

    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), rilErrNo,
            RfxVoidData(), msg);
    responseToTelCore(response);
}
