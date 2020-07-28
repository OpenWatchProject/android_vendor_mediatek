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
#include "RmcOpSCBMRequestHandler.h"
#include <telephony/mtk_ril.h>
#include "RfxStatusDefs.h"
#include "RfxVoidData.h"


using ::android::String8;

static const int reqList[] = {
    RFX_MSG_REQUEST_EXIT_SCBM
};


// Register handler
RFX_IMPLEMENT_HANDLER_CLASS(RmcOpSCBMRequestHandler, RIL_CMD_PROXY_1);

RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxVoidData,
        RFX_MSG_REQUEST_EXIT_SCBM);


/*****************************************************************************
 * Class RfxController
 *****************************************************************************/
RmcOpSCBMRequestHandler::RmcOpSCBMRequestHandler(int slot_id, int channel_id) :
        RfxBaseHandler(slot_id, channel_id) {
    setTag(String8("RmcOpSCBMRequest"));
    registerToHandleRequest(reqList, sizeof(reqList)/sizeof(int));
    logD(mTag, "RmcOpSCBMRequestHandler: constructor");
}

RmcOpSCBMRequestHandler::~RmcOpSCBMRequestHandler() {
}

void RmcOpSCBMRequestHandler::onHandleRequest(const sp<RfxMclMessage>& msg) {
    int id = msg->getId();
    logD(mTag, "onHandleRequest: %s(%d)", idToString(id), id);
    switch(id) {
        case RFX_MSG_REQUEST_EXIT_SCBM:
            requestExitSCBM(msg);
            break;
        default:
            logE(mTag, "Should not be here");
            break;
    }
}

void RmcOpSCBMRequestHandler::requestExitSCBM(const sp<RfxMclMessage>& msg) {
    // When MD is off, respond success directly.
    if (getNonSlotMclStatusManager()->getBoolValue(RFX_STATUS_KEY_MODEM_POWER_OFF, false)) {
        logE(mTag, "%s MD off, just return success!", __FUNCTION__);
        sp<RfxMclMessage> mclResponse =
        RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS, RfxVoidData(), msg);
        responseToTelCore(mclResponse);
        return;
    }
    handleCmdWithVoidResponse(msg, String8("AT+SMEMEXIT=1"));
}
