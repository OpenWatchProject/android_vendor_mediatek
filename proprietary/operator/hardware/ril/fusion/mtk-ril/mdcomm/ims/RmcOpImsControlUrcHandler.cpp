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
// Std include
#include <vector>
#include <string>

// MTK Fusion Fwk include
#include "RfxStringsData.h"
#include "RfxStringData.h"
#include "RfxIntsData.h"
#include "RfxVoidData.h"
#include "RfxMessageId.h"

// Ims Op Control Local include
#include "RmcOpImsControlInterface.h"
#include "RmcOpImsControlUrcHandler.h"

#include "cc/RmcOpCallControlCommonRequestHandler.h"

#define RFX_LOG_TAG "RmcOpImsControl"

// register handler to channel
RFX_IMPLEMENT_OP_HANDLER_CLASS(RmcOpImsControlUrcHandler, RIL_CMD_PROXY_URC);

RFX_REGISTER_DATA_TO_URC_ID(RfxStringsData, RFX_MSG_UNSOL_DIGITS_LINE_INDICATION);
RFX_REGISTER_DATA_TO_URC_ID(RfxStringData, RFX_MSG_UNSOL_RCS_DIGITS_LINE_INFO);

static const char* urc[] = {
    URC_DIGITS_LINE_INDICATION,
};

RmcOpImsControlUrcHandler::RmcOpImsControlUrcHandler(
    int slot_id, int channel_id) : RfxBaseHandler(slot_id, channel_id) {

    registerToHandleURC(urc, sizeof(urc) / sizeof(char *));
}

RmcOpImsControlUrcHandler::~RmcOpImsControlUrcHandler() {
}

void RmcOpImsControlUrcHandler::onHandleUrc(const sp<RfxMclMessage>& msg) {
    char *urc = msg->getRawUrc()->getLine();
    logD(RFX_LOG_TAG, "[onHandleUrc]%s", urc);
    if (strStartsWith(urc, URC_DIGITS_LINE_INDICATION)) {
        onDigitsLineIndication(msg);
    }
}

void RmcOpImsControlUrcHandler::onHandleTimer() {
    // do something
}

void RmcOpImsControlUrcHandler::onDigitsLineIndication(const sp<RfxMclMessage>& msg) {
    /*
     * +DIGITLINE=<account_id>,<serial_number>,<msisdn_num>,
       "<msisdn>",<is_active>,"<msisdn>",<is_active>,"<msisdn>",<is_active>,"<msisdn>",<is_active>,"<msisdn>",<is_active>,
       "<msisdn>",<is_active>,"<msisdn>",<is_active>,"<msisdn>",<is_active>,"<msisdn>",<is_active>,"<msisdn>",<is_active>,
     * <account_id>:    0:SIM1=0 ; 2:SIM2=2 ; 4;SIM3
     * <serial_number>: A sequence number copy from AT command
     * <msisdn_num>: the number of msisdn
     * <msisdn>: msisdn
     * <Is_active>: 0: This line registers fail; 1: The line register succeesfully
     */

    const int maxLen = 23;
    bool appendPhoneId = true;
    int rfxMsg = RFX_MSG_UNSOL_DIGITS_LINE_INDICATION;
    notifyStringsDataToTcl(msg, rfxMsg, maxLen, appendPhoneId);
}

void RmcOpImsControlUrcHandler::onRcsDigitsLineInfo(const sp<RfxMclMessage>& msg) {

    const int maxLen = 2;  // TODO: wait confirm
    bool appendPhoneId = true;
    int rfxMsg = RFX_MSG_UNSOL_RCS_DIGITS_LINE_INFO;
    notifyIntsDataToTcl(msg, rfxMsg, maxLen, appendPhoneId);
}
