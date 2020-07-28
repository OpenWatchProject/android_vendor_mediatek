/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
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

#include <telephony/mtk_rilop.h>

#include "RmcOpCallControlImsRequestHandler.h"
#include "RmcOpCallControlInterface.h"
#include "RfxIntsData.h"
#include "RfxStringsData.h"
#include "RfxVoidData.h"
#include "RfxDialFromData.h"

RFX_REGISTER_DATA_TO_REQUEST_ID(RfxDialFromData, RfxVoidData, RFX_MSG_REQUEST_DIAL_FROM);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_SET_EMERGENCY_CALL_CONFIG);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringsData, RfxVoidData, RFX_MSG_REQUEST_DEVICE_SWITCH);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxVoidData, RFX_MSG_REQUEST_CANCEL_DEVICE_SWITCH);

static const int requests[] = {
  RFX_MSG_REQUEST_DIAL_FROM,
  RFX_MSG_REQUEST_SET_EMERGENCY_CALL_CONFIG,
  RFX_MSG_REQUEST_DEVICE_SWITCH,
  RFX_MSG_REQUEST_CANCEL_DEVICE_SWITCH,
};

// register handler to channel
RFX_IMPLEMENT_OP_HANDLER_CLASS(RmcOpCallControlImsRequestHandler, RIL_CMD_PROXY_2);

RmcOpCallControlImsRequestHandler::RmcOpCallControlImsRequestHandler(int slot_id,
        int channel_id):RmcCallControlBaseHandler(slot_id, channel_id){
    registerToHandleRequest(requests, sizeof(requests)/sizeof(int));
}

RmcOpCallControlImsRequestHandler::~RmcOpCallControlImsRequestHandler() {
}

void RmcOpCallControlImsRequestHandler::onHandleTimer() {
}

void RmcOpCallControlImsRequestHandler::onHandleRequest(const sp<RfxMclMessage>& msg) {
    int requestId = msg->getId();
    logD(RFX_LOG_TAG, "onHandleRequest: %s", RFX_ID_TO_STR(requestId));
    switch(requestId) {
        case RFX_MSG_REQUEST_DIAL_FROM:
            requestDialFrom(msg);
            break;
        case RFX_MSG_REQUEST_SET_EMERGENCY_CALL_CONFIG:
            setEmergencyCallConfig(msg);
            break;
        case RFX_MSG_REQUEST_DEVICE_SWITCH:
            deviceSwitch(msg);
            break;
        case RFX_MSG_REQUEST_CANCEL_DEVICE_SWITCH:
            cancelDeviceSwitch(msg);
            break;
        default:
            break;
    }
}

void RmcOpCallControlImsRequestHandler::requestDialFrom(const sp<RfxMclMessage>& msg) {
    RIL_DialFrom *pDial = (RIL_DialFrom*)(msg->getData()->getData());
    String8 cmd;

    int clirMode = handleClirSpecial(false, pDial->clir, pDial->address);
    const char *clir = getClirPrefix(clirMode);
    String8 dialString = handleNumberWithPause(pDial->address);
    String8 dialFromString = handleNumberWithPause(pDial->fromAddress);

    char* atCmd = AT_DIAL_FROM;
    cmd = String8::format("%s\"%s%s\",\"%s\"", atCmd, clir, dialString.string(), dialFromString.string());

    if (pDial->isVideoCall) {
        cmd = String8::format("%s", cmd.string());
    } else {
        cmd = String8::format("%s;", cmd.string());
    }

    setVDSAuto(hasImsCall(msg->getSlotId()), false);
    handleCmdWithVoidResponse(msg, cmd);
}

void RmcOpCallControlImsRequestHandler::setEmergencyCallConfig(const sp<RfxMclMessage>& msg) {
    int *pInt = (int *)msg->getData()->getData();
    sp<RfxAtResponse> p_response;

    char* atCmd = AT_SET_ECC_CONFIG;
    String8 cmd = String8::format("%s=%d,%d", atCmd, pInt[0], pInt[1]);
    handleCmdWithVoidResponse(msg, cmd);
}

void RmcOpCallControlImsRequestHandler::deviceSwitch(const sp<RfxMclMessage>& msg) {
    /* +ECT=<type>,<number>,<deviceId>
     * <type>: 3 for device switch
     * <number>: target number, can be empty string
     * <deviceId>: target device id, can be empty string
     */
    char** params = (char**)msg->getData()->getData();
    char* number = params[0];
    char* deviceId = params[1];
    char* atCmd = AT_DEVICE_SWITCH;
    int msgType = MSG_TYPE_DEVICE_SWITCH;
    if (number == NULL) number = (char*)"";
    if (deviceId == NULL) deviceId = (char*)"";
    String8 cmd = String8::format("%s=%d,\"%s\",\"%s\"", atCmd, msgType, number, deviceId);
    handleCmdWithVoidResponse(msg, cmd);
}

void RmcOpCallControlImsRequestHandler::cancelDeviceSwitch(const sp<RfxMclMessage>& msg) {
    /* +ECT=<type>
     * <type>: 4 for cancel device switch
     */
    char* atCmd = AT_CANCEL_DEVICE_SWITCH;
    int msgType = MSG_TYPE_CANCEL_DEVICE_SWITCH;
    String8 cmd = String8::format("%s=%d", atCmd, msgType);
    handleCmdWithVoidResponse(msg, cmd);
}
