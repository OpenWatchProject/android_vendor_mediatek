/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
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

#include "RmcGwsdRequestHandler.h"
#include "RfxGwsdUtils.h"

static const int requests[] = {
    RFX_MSG_REQUEST_SET_GWSD_MODE,
    RFX_MSG_REQUEST_SET_GWSD_CALL_VALID,
    RFX_MSG_REQUEST_SET_GWSD_IGNORE_CALL_INTERVAL,
    RFX_MSG_REQUEST_SET_GWSD_KEEP_ALIVE_PDCP,
    RFX_MSG_REQUEST_SET_GWSD_KEEP_ALIVE_IPDATA,
};

// register data
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringsData, RfxVoidData, RFX_MSG_REQUEST_SET_GWSD_MODE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_SET_GWSD_CALL_VALID);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_SET_GWSD_IGNORE_CALL_INTERVAL);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringData, RfxVoidData, RFX_MSG_REQUEST_SET_GWSD_KEEP_ALIVE_PDCP);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringData, RfxVoidData, RFX_MSG_REQUEST_SET_GWSD_KEEP_ALIVE_IPDATA);

// register handler to channel
RFX_IMPLEMENT_HANDLER_CLASS(RmcGwsdRequestHandler, RIL_CMD_PROXY_1);

RmcGwsdRequestHandler::RmcGwsdRequestHandler(int slot_id, int channel_id) :
        RfxBaseHandler(slot_id, channel_id) {
    logD(RFX_LOG_TAG, "RmcGwsdRequestHandler constructor");
    registerToHandleRequest(requests, sizeof(requests)/sizeof(int));
}

RmcGwsdRequestHandler::~RmcGwsdRequestHandler() {
}

void RmcGwsdRequestHandler::onHandleRequest(const sp<RfxMclMessage>& msg) {
    logD(RFX_LOG_TAG, "onHandleRequest: %d", msg->getId());
    int request = msg->getId();
    switch(request) {
        case RFX_MSG_REQUEST_SET_GWSD_MODE:
            requestSetGwsdMode(msg);
            break;
        case RFX_MSG_REQUEST_SET_GWSD_CALL_VALID:
            requestSetCallValidTimer(msg);
            break;
        case RFX_MSG_REQUEST_SET_GWSD_IGNORE_CALL_INTERVAL:
            requestSetIgnoreSameNumberInterval(msg);
            break;
        case RFX_MSG_REQUEST_SET_GWSD_KEEP_ALIVE_PDCP:
            requestSetKeepAliveByPDCPCtrlPDU(msg);
            break;
        case RFX_MSG_REQUEST_SET_GWSD_KEEP_ALIVE_IPDATA:
            requestSetKeepAliveByIpData(msg);
            break;
        default:
            logE(RFX_LOG_TAG, "Should not be here");
            break;
    }
}

void RmcGwsdRequestHandler::requestSetGwsdMode(const sp<RfxMclMessage>& msg) {
    RIL_Errno ril_errno = RIL_E_MODE_NOT_SUPPORTED;
    sp<RfxMclMessage> resp;

     char** params   = (char**)msg->getData()->getData();
     logD(RFX_LOG_TAG, "requestSetGwsdMode, mode:%s,kaMode:%s,kaCycle:%s", params[0], params[1], params[2]);

    if (RfxGwsdUtils::getSettingHandler() == NULL) {
        logD(RFX_LOG_TAG, "not support gwsd");
        goto error;
    }

    ril_errno = RfxGwsdUtils::getSettingHandler()
                ->requestSetGwsdMode(this, atoi(params[0]), params[1], params[2]);

error:
    resp = RfxMclMessage::obtainResponse(ril_errno, RfxVoidData(), msg);
    responseToTelCore(resp);
}

void RmcGwsdRequestHandler::requestSetCallValidTimer(const sp<RfxMclMessage>& msg) {
    RIL_Errno ril_errno = RIL_E_MODE_NOT_SUPPORTED;
    sp<RfxMclMessage> resp;

     int *timer = (int*)msg->getData()->getData();
     logD(RFX_LOG_TAG, "requestSetCallValidTimer, timer: %d", timer[0]);

    if (RfxGwsdUtils::getSettingHandler() == NULL) {
        logD(RFX_LOG_TAG, "not support gwsd");
        goto error;
    }

    ril_errno = RfxGwsdUtils::getSettingHandler()->requestSetCallValidTimer(this, timer[0]);

error:
    resp = RfxMclMessage::obtainResponse(ril_errno, RfxVoidData(), msg);
    responseToTelCore(resp);
}

void RmcGwsdRequestHandler::requestSetIgnoreSameNumberInterval(
        const sp<RfxMclMessage>& msg) {
    RIL_Errno ril_errno = RIL_E_MODE_NOT_SUPPORTED;
    sp<RfxMclMessage> resp;

     int *interval = (int*)msg->getData()->getData();
     logD(RFX_LOG_TAG, "requestSetIgnoreSameNumberInterval, interval: %d", interval[0]);

    if (RfxGwsdUtils::getSettingHandler() == NULL) {
        logD(RFX_LOG_TAG, "not support gwsd");
        goto error;
    }

    ril_errno = RfxGwsdUtils::getSettingHandler()->
            requestSetIgnoreSameNumberInterval(this, interval[0]);

error:
    resp = RfxMclMessage::obtainResponse(ril_errno, RfxVoidData(), msg);
    responseToTelCore(resp);
}

void RmcGwsdRequestHandler::requestSetKeepAliveByPDCPCtrlPDU(
        const sp<RfxMclMessage>& msg) {
    RIL_Errno ril_errno = RIL_E_MODE_NOT_SUPPORTED;
    sp<RfxMclMessage> resp;

     char *pData = (char*)(msg->getData()->getData());
     logD(RFX_LOG_TAG, "%s, KeepAlive: %s", __FUNCTION__, pData);

    if (RfxGwsdUtils::getSettingHandler() == NULL) {
        logD(RFX_LOG_TAG, "not support gwsd");
        goto error;
    }

    ril_errno = RfxGwsdUtils::getSettingHandler()->
            requestSetKeepAliveByPDCPCtrlPDU(this, pData);

error:
    resp = RfxMclMessage::obtainResponse(ril_errno, RfxVoidData(), msg);
    responseToTelCore(resp);
}

void RmcGwsdRequestHandler::requestSetKeepAliveByIpData(
        const sp<RfxMclMessage>& msg) {
    RIL_Errno ril_errno = RIL_E_MODE_NOT_SUPPORTED;
    sp<RfxMclMessage> resp;

     char *pData = (char*)(msg->getData()->getData());
     logD(RFX_LOG_TAG, "%s, KeepAlive: %s", __FUNCTION__, pData);

    if (RfxGwsdUtils::getSettingHandler() == NULL) {
        logD(RFX_LOG_TAG, "not support gwsd");
        goto error;
    }

    ril_errno = RfxGwsdUtils::getSettingHandler()->
            requestSetKeepAliveByIpData(this, pData);

error:
    resp = RfxMclMessage::obtainResponse(ril_errno, RfxVoidData(), msg);
    responseToTelCore(resp);
}
