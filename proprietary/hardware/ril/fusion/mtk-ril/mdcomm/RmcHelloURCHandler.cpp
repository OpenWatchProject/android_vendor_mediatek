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

#include "RmcHelloURCHandler.h"
#include <telephony/mtk_ril.h>

RFX_IMPLEMENT_HANDLER_CLASS(RmcHelloURCHandler, RIL_CMD_PROXY_5);

// register data
RFX_REGISTER_DATA_TO_EVENT_ID(RfxIntsData, RFX_MSG_EVENT_HELLO_NOTIFY_MODEM_STATE);

RmcHelloURCHandler::RmcHelloURCHandler(int slot_id, int channel_id) :
        RfxBaseHandler(slot_id, channel_id) {
       const char* urc[] = {
    //     "+EIND:128",
    //     "+ECSRA",
           "+EUSIM:"
       };

    registerToHandleURC(urc, sizeof(urc)/sizeof(char *));
}

RmcHelloURCHandler::~RmcHelloURCHandler() {
}

void RmcHelloURCHandler::onHandleUrc(const sp<RfxMclMessage>& msg) {
    if(strStartsWith(msg->getRawUrc()->getLine(), "+EIND:128")) {
        handleModemState(msg);
    } else if (strStartsWith(msg->getRawUrc()->getLine(), "+EUSIM:")) {
        sp<RfxMclMessage> urc = RfxMclMessage::obtainUrc(RIL_UNSOL_TEST,
                m_slot_id, RfxVoidData());
        responseToTelCore(urc);
    }
}

void RmcHelloURCHandler::handleModemState(const sp<RfxMclMessage>& msg) {
    // do something

    int modemState[1];
    // NOTE: sendEvent with RfxBaseData
    modemState[0] = 1;
    sendEvent(RFX_MSG_EVENT_HELLO_NOTIFY_MODEM_STATE, RfxIntsData(modemState, 1),
            RIL_CMD_PROXY_1, RFX_SLOT_ID_0);

    sp<RfxMclMessage> urc = RfxMclMessage::obtainUrc(RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED, m_slot_id, RfxVoidData());
    responseToTelCore(urc);
}

void RmcHelloURCHandler::onHandleTimer() {
    // do something
}

void RmcHelloURCHandler::onHandleEvent(const sp<RfxMclMessage>& msg) {
    // handle event
}
