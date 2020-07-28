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

#include "RmcImsRttControlUrcHandler.h"
#include <telephony/mtk_ril.h>

#include "rfx_properties.h"

// register handler to channel
RFX_IMPLEMENT_HANDLER_CLASS(RmcImsRttControlUrcHandler, RIL_CMD_PROXY_URC);

// register data
RFX_REGISTER_DATA_TO_URC_ID(RfxIntsData, RFX_MSG_UNSOL_RTT_MODIFY_RESPONSE);
RFX_REGISTER_DATA_TO_URC_ID(RfxStringsData, RFX_MSG_UNSOL_RTT_TEXT_RECEIVE);
RFX_REGISTER_DATA_TO_URC_ID(RfxIntsData, RFX_MSG_UNSOL_RTT_CAPABILITY_INDICATION);
RFX_REGISTER_DATA_TO_URC_ID(RfxIntsData, RFX_MSG_UNSOL_RTT_MODIFY_REQUEST_RECEIVE);
RFX_REGISTER_DATA_TO_URC_ID(RfxIntsData, RFX_MSG_UNSOL_AUDIO_INDICATION);

RmcImsRttControlUrcHandler::RmcImsRttControlUrcHandler(int slot_id, int channel_id) :
        RfxBaseHandler(slot_id, channel_id) {
    logD(RFX_LOG_TAG, "RmcImsRttControlUrcHandler constructor");

    int m_slot_id = slot_id;
    const char* urc[] = {
        "+RTTCALL",
        "+ERTTSTR",
        "+EIMSTCAP",
        "+PRTTCALL",
        "+EIMSAUDIOSID",
    };

    registerToHandleURC(urc, sizeof(urc)/sizeof(char *));
}

RmcImsRttControlUrcHandler::~RmcImsRttControlUrcHandler() {
}

void RmcImsRttControlUrcHandler::onHandleUrc(const sp<RfxMclMessage>& msg) {
    if (strStartsWith(msg->getRawUrc()->getLine(), "+RTTCALL")) {
        logD(RFX_LOG_TAG, "onHandleUrc: %s", msg->getRawUrc()->getLine());
        onRttModifyResponse(msg);
    } else if (strStartsWith(msg->getRawUrc()->getLine(), "+ERTTSTR")) {
        logD(RFX_LOG_TAG, "onHandleUrc: +ERTTSTR");
        onRttTextReceive(msg);
    } else if (strStartsWith(msg->getRawUrc()->getLine(), "+EIMSTCAP")) {
        logD(RFX_LOG_TAG, "onHandleUrc: %s", msg->getRawUrc()->getLine());
        onRttCapabilityIndication(msg);
    } else if (strStartsWith(msg->getRawUrc()->getLine(), "+PRTTCALL")) {
        logD(RFX_LOG_TAG, "onHandleUrc: %s", msg->getRawUrc()->getLine());
        onRttModifyRequestReceive(msg);
    } else if (strStartsWith(msg->getRawUrc()->getLine(), "+EIMSAUDIOSID")) {
        logD(RFX_LOG_TAG, "onHandleUrc: %s", msg->getRawUrc()->getLine());
        onAudioIndication(msg);
    }
}
void RmcImsRttControlUrcHandler::onRttModifyResponse(const sp<RfxMclMessage>& msg) {
    /*
     * +RTTCALL: <call id>,<result>
     * <call id>: call id
     * <result>: 0:success; 1:fail;
     */
    const int maxLen = 2;
    bool appendPhoneId = true;
    int rfxMsg = RFX_MSG_UNSOL_RTT_MODIFY_RESPONSE;

    notifyIntsDataToTcl(msg, rfxMsg, maxLen, appendPhoneId);
}

void RmcImsRttControlUrcHandler::onRttTextReceive(const sp<RfxMclMessage>& msg) {

    // +ERTTSTR= =<callid>, <len>, <text>
    const int maxLen = 3;
    bool appendPhoneId = true;
    int rfxMsg = RFX_MSG_UNSOL_RTT_TEXT_RECEIVE;

    notifyStringsDataToTcl(msg, rfxMsg, maxLen, appendPhoneId);
}

void RmcImsRttControlUrcHandler::onRttCapabilityIndication(const sp<RfxMclMessage>& msg) {
    // +EIMSTCAP:<call id>,<local text capability>,<remote text capability>
    // <local text status>,<real remote text capability>

    const int maxLen = 5;
    bool appendPhoneId = true;
    int rfxMsg = RFX_MSG_UNSOL_RTT_CAPABILITY_INDICATION;

    notifyIntsDataToTcl(msg, rfxMsg, maxLen, appendPhoneId);
}

void RmcImsRttControlUrcHandler::onRttModifyRequestReceive(const sp<RfxMclMessage>& msg) {
    //+PRTTCALL= <call id>, <op>
    const int maxLen = 2;
    bool appendPhoneId = true;
    int rfxMsg = RFX_MSG_UNSOL_RTT_MODIFY_REQUEST_RECEIVE;

    notifyIntsDataToTcl(msg, rfxMsg, maxLen, appendPhoneId);
}

void RmcImsRttControlUrcHandler::onAudioIndication(const sp<RfxMclMessage>& msg) {
    /*
     * +EIMSAUDIOSID:<call id>, <audio>
     * <call id>: call id
     * <audio>: 0:silence; 1:speech;
     */

    const int maxLen = 2;
    bool appendPhoneId = true;
    int rfxMsg = RFX_MSG_UNSOL_AUDIO_INDICATION;

    notifyIntsDataToTcl(msg, rfxMsg, maxLen, appendPhoneId);
}
