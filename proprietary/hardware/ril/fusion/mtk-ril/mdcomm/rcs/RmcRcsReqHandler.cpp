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
#include "RfxVoidData.h"
#include "RfxIntsData.h"

#include "RmcRcsReqHandler.h"
#include "RmcRcsDataThreadController.h"
#include "RfxRcsSendMsgData.h"

// for send MCL request to myself
#include "RfxMclDispatcherThread.h"
#include "RfxMclMessage.h"
#include "RfxMessageId.h"
#include "rfx_properties.h"

// for socket / thread
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cutils/sockets.h>
#include <sys/socket.h>
#include <sys/un.h>

#define RFX_LOG_TAG "RCS RIL RMC"

RFX_REGISTER_DATA_TO_EVENT_ID(RfxRcsSendMsgData, RFX_MSG_EVENT_RCS_SEND_MSG);
/*****************************************************************************
 * Class RmcRcsReqHandler
 *****************************************************************************/
RFX_IMPLEMENT_HANDLER_CLASS(RmcRcsReqHandler, RIL_CMD_PROXY_IMS);

RmcRcsReqHandler::RmcRcsReqHandler(int slot_id, int channel_id)
: RfxBaseHandler(slot_id, channel_id) {

    RFX_LOG_I(RFX_LOG_TAG, "RmcRcsReqHandler create (slot_id = %d)", slot_id);

    if (!isImsSubmarineSupport()) {
        return;
    }

    if (!RmcRcsReqHandler::isRcsUaSupport()) {
        return;
    }

    //data to modem
    const int CommonEventList[] = {
        RFX_MSG_EVENT_RCS_SEND_MSG,
    };

    registerToHandleEvent(CommonEventList, sizeof(CommonEventList) / sizeof(int));

    //data from modem
    const int imsRcsEventList[] = {
        RFX_MSG_EVENT_IMS_DATA,
    };

    //RIL_IMS_Client_RCS
    registerToHandleEvent(RIL_IMS_Client_RCS, imsRcsEventList, sizeof(imsRcsEventList) / sizeof(int));

    // Notify RIL FWK that we are ready to recv message.
    // Sometimes remote side may be ready earler than us and send message firstly
    // so we need to tell FWK when we can recv message or the earlier message will lost.
    sendUserData(RIL_IMS_Client_RCS, RIL_IMS_CLIENT_READY, NULL, 0);

    // create thread controller
    if (slot_id == 0) {
        RmcRcsDataThreadController::init();
    }
}

RmcRcsReqHandler::~RmcRcsReqHandler() {
}

void RmcRcsReqHandler::onHandleRequest(const sp<RfxMclMessage>& msg) {

    int request = msg->getId();

    RFX_LOG_I(RFX_LOG_TAG, "request = %d", request);

    switch (request) {
        default:
            RFX_LOG_I(RFX_LOG_TAG, "unknown request, ignore!");
            break;
    }
}

void RmcRcsReqHandler::onHandleEvent(const sp<RfxMclMessage> & msg) {

    int event = msg->getId();

    RFX_LOG_I(RFX_LOG_TAG, "event = %d", event);

    switch (event) {
        case RFX_MSG_EVENT_RCS_SEND_MSG:
            handleEventRcsSendMsg(msg);
            break;
        case RFX_MSG_EVENT_IMS_DATA:
            handleEventRcsReceiveMsg(msg);
            break;
        default:
            RFX_LOG_I(RFX_LOG_TAG, "unknown event, ignore!");
            break;
    }
}

void RmcRcsReqHandler::handleEventRcsSendMsg(const sp<RfxMclMessage>& msg) {
    const char *pSendMsg = (const char *)msg->getData()->getData();
    int length = msg->getData()->getDataLength();

    RFX_LOG_I(RFX_LOG_TAG, "send data length = %d, slot = %d\n", length, m_slot_id);

    //debug trace
    if (1){
        int type,len,flag,acct,uaType;
        type = (* ((int*)pSendMsg));
        len = (* ((int*)pSendMsg + 1));
        flag = (* ((int*)pSendMsg + 2));
        acct = (* ((int*)pSendMsg + 3));
        uaType = (* ((int*)pSendMsg + 4));
        RFX_LOG_I(RFX_LOG_TAG, "send data type = %d, len = %d, flag = %d, acct = %d, uaType = %d\n",
            type, len, flag, acct, uaType);

        if (type == VOLTE_REQ_SIPTX_CREATE) {
            VoLTE_Stack_Channel_Info_t *info = (VoLTE_Stack_Channel_Info_t *)(pSendMsg + MD_CHANNEL_HEADER_LEN);
            RFX_LOG_I(RFX_LOG_TAG, "send data info->type = %d\n", info->type);
        }
    }

    // send to RIL FWK
    sendUserData(RIL_IMS_Client_RCS, (unsigned char*) pSendMsg, length);

    return;
}

void RmcRcsReqHandler::handleEventRcsReceiveMsg(const sp<RfxMclMessage>& msg) {
    const char *pRecvMsg = (const char *)msg->getData()->getData();
    int length = msg->getData()->getDataLength();

    RFX_LOG_I(RFX_LOG_TAG, "recv data length = %d, slot = %d\n", length, m_slot_id);

    RmcRcsDataThreadController::sendMsgToShareMemory((char*)pRecvMsg, length, "handleEventRcsReceiveMsg");
    return;
}

bool RmcRcsReqHandler::isImsSubmarineSupport() {
    char feature[] = "IMS SUBMARINE";
    int support = getFeatureVersion(feature);

    // IMS SUBMARINE, 0:not support, 1:support
    RFX_LOG_I(RFX_LOG_TAG, "isImsSubmarineSupport, %s = %d", feature, support);

    return (support == 1);
}

bool RmcRcsReqHandler::isRcsUaSupport() {
    //when mtk_rcs_ua_support set yes, ua deamon will be exist always,
    //so here use mtk_rcs_ua_support feature option to decide server running or not
    //property will be changed for cxp, so not use property here
    char property_value[MTK_PROPERTY_VALUE_MAX] = { 0 };
    int isRcsUaProxySupport = 0;
    rfx_property_get("persist.vendor.mtk_rcs_ua_support", property_value, "0");
    isRcsUaProxySupport = atoi(property_value);
    RFX_LOG_I(RFX_LOG_TAG, "isRcsUaProxySupport: %d", isRcsUaProxySupport);

    if (isRcsUaProxySupport == 1) {
       return true;
    }
    return false;
}
