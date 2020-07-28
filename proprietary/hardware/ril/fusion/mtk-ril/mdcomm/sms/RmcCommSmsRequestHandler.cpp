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
#include "RfxMessageId.h"
#include "RmcCommSmsRequestHandler.h"
#include <telephony/mtk_ril.h>
#include "RfxStatusDefs.h"
#include "RfxVoidData.h"

#define ENABLE_GEO_FENCING "ro.vendor.enable.geo.fencing"

using ::android::String8;

static const int event_list[] = {
    RFX_MSG_EVENT_SMS_NOTIFY_SMS_READY,
};

// Register handler
RFX_IMPLEMENT_HANDLER_CLASS(RmcCommSmsRequestHandler, RIL_CMD_PROXY_1);

// Register unsolicited message
RFX_REGISTER_DATA_TO_URC_ID(RfxVoidData, RFX_MSG_URC_SMS_READY_NOTIFICATION);

// Register event
RFX_REGISTER_DATA_TO_EVENT_ID(RfxVoidData, RFX_MSG_EVENT_SMS_NOTIFY_SMS_READY);

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/
RmcCommSmsRequestHandler::RmcCommSmsRequestHandler(int slot_id, int channel_id) :
        RfxBaseHandler(slot_id, channel_id) {
        setTag(String8("RmcCommSmsReq"));
        registerToHandleEvent(event_list, sizeof(event_list)/sizeof(int));
}

void RmcCommSmsRequestHandler::onHandleEvent(const sp < RfxMclMessage > & msg) {
    int event = msg->getId();

    switch(event) {
        case RFX_MSG_EVENT_SMS_NOTIFY_SMS_READY:
            notifySmsReady(msg);
            break;
        default:
            logE(mTag, "Not support the event!");
            break;
    }
}

void RmcCommSmsRequestHandler::notifySmsReady(const sp<RfxMclMessage>& msg) {
    logD(mTag, "notifySmsReady");

    RFX_UNUSED(msg);
    /*
     * If modem turns off, RILD should not send any AT command to modem.
     * If RILD sends the at commands to Modem(off), it will block the
     * MUXD/CCCI channels
     */
    if (getNonSlotMclStatusManager()->getBoolValue(RFX_STATUS_KEY_MODEM_POWER_OFF, false)) {
        logD(mTag, "Modem turns off");
        return;
    }

    sp<RfxMclMessage> unsol = RfxMclMessage::obtainUrc(RFX_MSG_URC_SMS_READY_NOTIFICATION,
            m_slot_id, RfxVoidData());
    responseToTelCore(unsol);

    // To set PDU mode must be when SMS task is ready in modem.
    atSendCommand("AT+CMGF=0");
    /*
     * Always send SMS messages directly to the TE
     *
     * mode = 1 // discard when link is reserved (link should never be
     *             reserved)
     * mt = 2   // most messages routed to TE
     * bm = 2   // new cell BM's routed to TE
     * ds = 1   // Status reports routed to TE
     * bfr = 1  // flush buffer
     */
    atSendCommandSingleline("AT+CSMS=1", "+CSMS:");

    atSendCommand("AT+CNMI=1,2,2,1,1");

    atSendCommand("AT+CPMS=\"SM\", \"SM\", \"SM\"");
    atSendCommand("AT+EC2KCPMS=\"SM\", \"SM\", \"SM\"");

    // Enable WEA 3.0.
    char value[RFX_PROPERTY_VALUE_MAX] = {0};
    int ret = 0;
    rfx_property_get(ENABLE_GEO_FENCING, value, "0");
    ret = atoi(value);
    if (ret == 1) {
        atSendCommand("AT+EWAC=1");
    }
}