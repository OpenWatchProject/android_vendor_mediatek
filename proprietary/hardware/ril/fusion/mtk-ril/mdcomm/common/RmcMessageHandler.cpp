/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2016. All rights reserved.
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
#include "RmcMessageHandler.h"

/*****************************************************************************
 * Class RmcBaseRequestHandler
 *****************************************************************************/
RmcBaseRequestHandler::~RmcBaseRequestHandler() {
    Vector<RmcAtRspInfo *>::iterator it;
    for (it = m_atRspInfos.begin(); it != m_atRspInfos.end();) {
        delete (*it);
        it = m_atRspInfos.erase(it);
    }
}


/*****************************************************************************
 * Class RmcBaseRequestHandler
 *****************************************************************************/
void RmcBaseRequestHandler::onHandleMessage(const sp<RfxMclMessage> &msg) {
    RmcBaseReqData * req = (RmcBaseReqData *)msg->getData();
    sendAtCmds(req);
    RmcBaseRspData *rsp = getRspData(req);
    getHandler()->responseToTelCore(rsp->toMessage(msg));
    releaseRspData(rsp);
    onAfterResponse();
}

void RmcBaseRequestHandler::sendAtCmds(RmcBaseReqData *data) {
    RmcAtSendInfo* info = data->getFirstAt(getHandler());
    if (info != NULL) {
        RmcAtRspInfo* rspInfo;
        do {
            sp<RfxAtResponse> rsp = info->sendAt(getHandler());
            rspInfo = new RmcAtRspInfo(info->getCmd(), rsp);
            m_atRspInfos.push(rspInfo);
        } while ((info = data->getNextAt(rspInfo, getHandler())) != NULL);
    }
}

RmcBaseRspData *RmcBaseRequestHandler::getRspData(RmcBaseReqData *data) {
    return onGetRspData(data);
}

RmcBaseRspData *RmcBaseRequestHandler::onGetRspData(RmcBaseReqData *data) {
    return new RmcVoidRsp(data->getError());
}


void RmcBaseRequestHandler::releaseRspData(RmcBaseRspData *data) {
    if (data != NULL) {
        delete data;
    }
}

/*****************************************************************************
 * Class RmcBaseUrcHandler
 *****************************************************************************/
void RmcBaseUrcHandler::onHandleMessage(const sp<RfxMclMessage> &msg) {
    RmcBaseUrspData *uRsp = getUrspData(msg);
    if (uRsp != NULL) {
        getHandler()->responseToTelCore(uRsp->toMessage());
        releaseUrspData(uRsp);
    }
}

RmcBaseUrspData *RmcBaseUrcHandler::getUrspData(const sp<RfxMclMessage> &msg) {
    int err;
    RfxAtLine *urc = msg->getRawUrc();
    urc->atTokStart(&err);
    if (err < 0) {
        return NULL;
    }
    if (!onHandleRawUrc(urc)) {
        return NULL;
    }
    RfxAtLine *urc2 = msg->getRawUrc2();
    if (urc2 != NULL) {
        urc2->atTokStart(&err);
        if (err < 0) {
            return NULL;
        }
        if (!onHandleRawUrc2(urc2)) {
            return NULL;
        }
    }
    return onGetUrcData(msg->getSlotId());
};

void RmcBaseUrcHandler::releaseUrspData(RmcBaseUrspData * data) {
    if (data != NULL) {
        delete data;
    }
}
