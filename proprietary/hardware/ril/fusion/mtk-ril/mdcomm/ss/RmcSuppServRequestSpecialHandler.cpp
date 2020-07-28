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

#include "RmcSuppServRequestSpecialHandler.h"
#include "GsmUtil.h"
#include "SSUtil.h"
#include "RfxMessageId.h"

#include <telephony/mtk_ril.h>
#include <mtkconfigutils.h>

RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxIntsData, RFX_MSG_REQUEST_QUERY_CALL_WAITING);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxCallForwardInfoData, RfxCallForwardInfosData, RFX_MSG_REQUEST_QUERY_CALL_FORWARD_STATUS);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxCallForwardInfoExData, RfxCallForwardInfosExData, RFX_MSG_REQUEST_QUERY_CALL_FORWARD_IN_TIME_SLOT);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData,    RfxIntsData, RFX_MSG_REQUEST_GET_CLIR);

RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringsData, RfxIntsData, RFX_MSG_REQUEST_QUERY_CALL_BARRING);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringsData, RfxIntsData, RFX_MSG_REQUEST_SET_CALL_BARRING);

static const int requests[] = {
    RFX_MSG_REQUEST_QUERY_CALL_FORWARD_STATUS,
    RFX_MSG_REQUEST_QUERY_CALL_FORWARD_IN_TIME_SLOT,
    RFX_MSG_REQUEST_QUERY_CALL_WAITING,
    RFX_MSG_REQUEST_GET_CLIR,
    RFX_MSG_REQUEST_QUERY_CALL_BARRING,
    RFX_MSG_REQUEST_SET_CALL_BARRING
};

// register handler to channel
RFX_IMPLEMENT_HANDLER_CLASS(RmcSuppServRequestSpecialHandler, RIL_CMD_PROXY_6);

RmcSuppServRequestSpecialHandler::RmcSuppServRequestSpecialHandler(int slot_id,
        int channel_id):RmcSuppServRequestBaseHandler(slot_id, channel_id){
    registerToHandleRequest(requests, sizeof(requests)/sizeof(int));
}

RmcSuppServRequestSpecialHandler::~RmcSuppServRequestSpecialHandler() {
}

void RmcSuppServRequestSpecialHandler::onHandleRequest(const sp<RfxMclMessage>& msg) {
    logD(TAG, "onHandleRequest: %d", msg->getId());
    int request = msg->getId();
    switch(request) {
        case RFX_MSG_REQUEST_QUERY_CALL_FORWARD_STATUS:
            requestQueryCallForwardStatus(msg);
            break;
        case RFX_MSG_REQUEST_QUERY_CALL_FORWARD_IN_TIME_SLOT:
            requestQueryCallForwardInTimeSlotStatus(msg);
            break;
        case RFX_MSG_REQUEST_GET_CLIR:
            requestGetClir(msg);
            break;
        case RFX_MSG_REQUEST_QUERY_CALL_WAITING:
            requestQueryCallWaiting(msg);
            break;
        case RFX_MSG_REQUEST_QUERY_CALL_BARRING:
            requestQueryCallBarring(msg);
            break;
        case RFX_MSG_REQUEST_SET_CALL_BARRING:
            requestSetCallBarring(msg);
            break;
        default:
            logE(TAG, "Should not be here");
            break;
    }
}

void RmcSuppServRequestSpecialHandler::onHandleTimer() {
    // do something
}

void RmcSuppServRequestSpecialHandler::requestQueryCallForwardStatus(const sp<RfxMclMessage>& msg) {
    requestCallForwardOperation(msg, CCFC_E_QUERY);
}

void RmcSuppServRequestSpecialHandler::requestQueryCallForwardInTimeSlotStatus(const sp<RfxMclMessage>& msg) {
    requestCallForwardExOperation(msg, CCFC_E_QUERY);
}

void RmcSuppServRequestSpecialHandler::requestGetClir(const sp<RfxMclMessage>& msg) {
    requestClirOperation(msg);
}

void RmcSuppServRequestSpecialHandler::requestQueryCallWaiting(const sp<RfxMclMessage>& msg) {
    requestCallWaitingOperation(msg, CW_E_QUERY);
}

void RmcSuppServRequestSpecialHandler::requestQueryCallBarring(const sp<RfxMclMessage>& msg) {
    requestCallBarring(msg, CB_E_QUERY);
}

void RmcSuppServRequestSpecialHandler::requestSetCallBarring(const sp<RfxMclMessage>& msg) {
    requestCallBarring(msg, CB_E_SET);
}
