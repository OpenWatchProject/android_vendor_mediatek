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
// MTK Fusion Fwk include
#include "RfxStringsData.h"
#include "RmcOpCallControlUrcHandler.h"

#define RFX_LOG_TAG "RmcOpCCUrcHandler"

RFX_REGISTER_DATA_TO_URC_ID(RfxStringsData, RFX_MSG_UNSOL_QUERY_TRN);

// register handler to channel
RFX_IMPLEMENT_OP_HANDLER_CLASS(RmcOpCallControlUrcHandler, RIL_CMD_PROXY_URC);

RmcOpCallControlUrcHandler::RmcOpCallControlUrcHandler(
    int slot_id, int channel_id) : RfxBaseHandler(slot_id, channel_id) {
     const char* urc[] = {
        "+QUERYTRN",
    };

    registerToHandleURC(urc, sizeof(urc) / sizeof(char *));
}

RmcOpCallControlUrcHandler::~RmcOpCallControlUrcHandler() {
}

void RmcOpCallControlUrcHandler::onHandleUrc(const sp<RfxMclMessage>& msg) {
    char *urc = msg->getRawUrc()->getLine();
    //logD(RFX_LOG_TAG, "[onHandleUrc]%s", urc);
    if(strstr(urc, "+QUERYTRN") != NULL) {
        onQueryTrnIndication(msg);
    }
}

void RmcOpCallControlUrcHandler::onHandleTimer() {
    // do something
}


// For Modem to query TRN from AP during CSFB
void RmcOpCallControlUrcHandler::onQueryTrnIndication(const sp<RfxMclMessage>& msg) {
    /*
     * +QUERYTRN: <callId>,<toMsisdn>,<fromMsisdn>
     */
    int ret;
    int callId;
    char *number[2];
    RfxAtLine* line = msg->getRawUrc();

    line->atTokStart(&ret);
    if (ret < 0) { return; }

    callId = line->atTokNextint(&ret);
    if (ret < 0) {
        logE(RFX_LOG_TAG, "QUERYTRN: invalid callId");
        return;
    }
    getMclStatusManager()->setIntValue(RFX_STATUS_KEY_TRN_CALLID, callId);

    number[1] = line->atTokNextstr(&ret);
    if (ret < 0) {
        logE(RFX_LOG_TAG, "QUERYTRN: invalid to MSISDN");
        return;
    }

    number[0] = line->atTokNextstr(&ret);
    if (ret < 0) {
        logE(RFX_LOG_TAG, "QUERYTRN: invalid from MSISDN");
        return;
    }

    sp<RfxMclMessage> urc = RfxMclMessage::obtainUrc(RFX_MSG_UNSOL_QUERY_TRN,
                m_slot_id, RfxStringsData(number, 2));
    responseToTelCore(urc);
}
