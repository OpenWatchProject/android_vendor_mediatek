/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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
#include "RmcCdmaSmsHandler.h"
#include "RmcCdmaMoSms.h"
#include "RmcCdmaMtSms.h"
#include "RmcCdmaSmsAck.h"
#include "RmcRuimSmsWrite.h"
#include "RmcRuimSmsDelete.h"
#include "RmcRuimSmsMem.h"
#include "RmcCdmaSmsMemFull.h"
#include "RmcCdmaEsnMeid.h"
#include "RmcCdmaBcActivate.h"
#include "RmcCdmaBcRangeParser.h"
#include "RmcCdmaBcConfigSet.h"
#include "RmcCdmaBcConfigGet.h"
#include "RfxStringData.h"
#include "RfxIntsData.h"


/*****************************************************************************
 * Const Value
 *****************************************************************************/
const char *memFul = "+ESMFULL";
const char *esnMeid = "+ECARDESNME";
const char *mt = "+EC2KCMT";
const char *vm = "+EVMI";


RFX_REGISTER_DATA_TO_EVENT_ID(RfxStringData, RFX_MSG_EVENT_CDMA_SMS_NEW_SMS_ERR_ACK);
RFX_REGISTER_DATA_TO_EVENT_ID(RfxIntsData, RFX_MSG_EVENT_CDMA_SMS_PENDING_VMI);

/*****************************************************************************
 * Class RmcCdmaSmsReqHandler
 *****************************************************************************/
RFX_IMPLEMENT_HANDLER_CLASS(RmcCdmaSmsReqHandler, RIL_CMD_PROXY_1);

RmcCdmaSmsReqHandler::RmcCdmaSmsReqHandler(int slotId,int channelId) :
    RmcBaseHandler(slotId, channelId) {
    const int request[] = {
        RFX_MSG_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION,
        RFX_MSG_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG,
        RFX_MSG_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG,
        RFX_MSG_REQUEST_CDMA_DELETE_SMS_ON_RUIM,
        RFX_MSG_REQUEST_CDMA_WRITE_SMS_TO_RUIM,
        RFX_MSG_REQUEST_GET_SMS_RUIM_MEM_STATUS,
    };
    registerToHandleRequest(request, sizeof(request) / sizeof(int));
}

RmcCdmaSmsReqHandler::~RmcCdmaSmsReqHandler() {
}

RmcMessageHandler* RmcCdmaSmsReqHandler::onCreateReqHandler(
        const sp<RfxMclMessage> & msg) {
    RmcMessageHandler* handler = NULL;
    switch (msg->getId()) {
        case RFX_MSG_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION: {
            handler = new RmcBaseRequestHandler(this);
            break;
        }

        case RFX_MSG_REQUEST_CDMA_DELETE_SMS_ON_RUIM: {
            handler = new RmcBaseRequestHandler(this);
            break;
        }

        case RFX_MSG_REQUEST_CDMA_WRITE_SMS_TO_RUIM: {
            handler = new RmcCdmaWriteRuimSmsHdlr(this);
            break;
        }

        case RFX_MSG_REQUEST_GET_SMS_RUIM_MEM_STATUS: {
            handler = new RmcCdmaGetRuimSmsMemHdlr(this);
            break;
        }

        case RFX_MSG_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG: {
            handler = new RmcCdmaBcGetConfigHdlr(this);
            break;
        }

        case RFX_MSG_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG: {
            handler = new RmcBaseRequestHandler(this);
            break;
        }

        default:
            RFX_ASSERT(0);
    }
    return handler;
}


void RmcCdmaSmsReqHandler::onHandleEvent(const sp<RfxMclMessage> & msg) {
    RfxBaseHandler::onHandleEvent(msg);
}


/*****************************************************************************
 * Class RmcCdmaMoSmsReqHandler
 *****************************************************************************/
RFX_IMPLEMENT_HANDLER_CLASS(RmcCdmaMoSmsReqHandler, RIL_CMD_PROXY_8);

RmcCdmaMoSmsReqHandler::RmcCdmaMoSmsReqHandler(int slotId,int channelId) :
    RmcBaseHandler(slotId, channelId) {
    const int request[] = {
            RFX_MSG_REQUEST_CDMA_SEND_SMS,
            RFX_MSG_REQUEST_IMS_SEND_CDMA_SMS,
            RFX_MSG_REQUEST_IMS_SEND_CDMA_SMS_EX
    };
    registerToHandleRequest(request, sizeof(request) / sizeof(int));
}

RmcCdmaMoSmsReqHandler::~RmcCdmaMoSmsReqHandler() {
}


RmcMessageHandler* RmcCdmaMoSmsReqHandler::onCreateReqHandler(
        const sp<RfxMclMessage> & msg) {
    RmcMessageHandler* handler = NULL;
    int id = msg->getId();
    RFX_ASSERT((id == RFX_MSG_REQUEST_CDMA_SEND_SMS) ||
            (id == RFX_MSG_REQUEST_IMS_SEND_CDMA_SMS) ||
            (id == RFX_MSG_REQUEST_IMS_SEND_CDMA_SMS_EX));
    handler = new RmcCdmaMoSmsHdlr(this);
    return handler;
}

void RmcCdmaMoSmsReqHandler::onHandleEvent(const sp<RfxMclMessage> & msg) {
    RfxBaseHandler::onHandleEvent(msg);
}

/*****************************************************************************
 * Class RmcCdmaMtSmsAckReqHandler
 *****************************************************************************/
RFX_IMPLEMENT_HANDLER_CLASS(RmcCdmaMtSmsAckReqHandler, RIL_CMD_PROXY_7);

RmcCdmaMtSmsAckReqHandler::RmcCdmaMtSmsAckReqHandler(int slotId,int channelId) :
    RmcBaseHandler(slotId, channelId) {
    const int request[] = {
        RFX_MSG_REQUEST_CDMA_SMS_ACKNOWLEDGE,
        RFX_MSG_REQUEST_CDMA_SMS_ACKNOWLEDGE_EX,
        RFX_MSG_REQUEST_CDMA_SMS_ACKNOWLEDGE_INTERNAL
    };
    registerToHandleRequest(request, sizeof(request) / sizeof(int));
    const int event[] = {
       RFX_MSG_EVENT_CDMA_SMS_NEW_SMS_ERR_ACK
    };
    registerToHandleEvent(event, sizeof(event) / sizeof(int));
}

RmcCdmaMtSmsAckReqHandler::~RmcCdmaMtSmsAckReqHandler() {
}


RmcMessageHandler* RmcCdmaMtSmsAckReqHandler::onCreateReqHandler(
        const sp<RfxMclMessage> & msg) {
    RmcMessageHandler* handler = NULL;
    int id = msg->getId();
    RFX_ASSERT(id == RFX_MSG_REQUEST_CDMA_SMS_ACKNOWLEDGE ||
            id == RFX_MSG_REQUEST_CDMA_SMS_ACKNOWLEDGE_EX ||
            id == RFX_MSG_REQUEST_CDMA_SMS_ACKNOWLEDGE_INTERNAL);
    handler = new RmcCdmaSmsAckHdlr(this);
    return handler;
}

void RmcCdmaMtSmsAckReqHandler::onHandleEvent(const sp<RfxMclMessage> & msg) {
    if (msg->getId() == RFX_MSG_EVENT_CDMA_SMS_NEW_SMS_ERR_ACK) {
        String8 at((const char* )msg->getData()->getData(), msg->getData()->getDataLength());
        atSendCommand(at);
        return;
    }
    RfxBaseHandler::onHandleEvent(msg);
}

/*****************************************************************************
 * Class RmcCdmaSmsUrcHandler
 *****************************************************************************/
RFX_IMPLEMENT_HANDLER_CLASS(RmcCdmaSmsUrcHandler, RIL_CMD_PROXY_URC);

RmcCdmaSmsUrcHandler::RmcCdmaSmsUrcHandler(int slotId,int channelId) :
    RmcBaseHandler(slotId, channelId) {
    const char *urc[] = {mt, memFul, esnMeid, vm};
    registerToHandleURC(urc, sizeof(urc)/ sizeof(urc[0]));
    const int event[] = {
       RFX_MSG_EVENT_CDMA_SMS_PENDING_VMI
    };
    registerToHandleEvent(event, sizeof(event) / sizeof(int));
}

RmcCdmaSmsUrcHandler::~RmcCdmaSmsUrcHandler() {
}


RmcMessageHandler* RmcCdmaSmsUrcHandler::onCreateUrcHandler(
        const sp<RfxMclMessage> & msg) {
    RmcMessageHandler* handler = NULL;

    const char *urc = msg->getRawUrc()->getLine();
    if (strStartsWith(urc, mt)) {
        handler = new RmcCdmaMtSmsHdlr(this, CDMA_SMS_INBOUND_COMM);
    } else if (strStartsWith(urc, vm)) {
        handler = new RmcCdmaMtSmsHdlr(this, CDMA_SMS_INBOUND_VMI);
    } else if (strStartsWith(urc, memFul)) {
        handler = new RmcCdmaSmsMemFullHdlr(this);
    } else if (strStartsWith(urc, esnMeid)) {
        handler = new RmcCdmaCardEsnMeidHdlr(this);
    }
    return handler;
}




void RmcCdmaSmsUrcHandler::onHandleEvent(const sp<RfxMclMessage> & msg) {
    if (msg->getId() == RFX_MSG_EVENT_CDMA_SMS_PENDING_VMI) {
        int vmi = *((int*)msg->getData()->getData());
        String8 urc = String8::format("+EVMI:%d", vmi);
        RfxAtLine* atLine = new RfxAtLine(urc.string(), NULL);
        sp<RfxMclMessage> message = RfxMclMessage::obtainRawUrc(
            msg->getSlotId() * RIL_CHANNEL_OFFSET, atLine, NULL);
        RmcCdmaSmsUrcHandler::onHandleUrc(message);
        return;
    }
    RfxBaseHandler::onHandleEvent(msg);
}

