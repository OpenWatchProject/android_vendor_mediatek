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
#include "RmcCdmaMoSms.h"
#include "RmcCdmaSmsConverter.h"


/*****************************************************************************
 * Register Data Class
 *****************************************************************************/
RFX_REGISTER_DATA_TO_REQUEST_ID(
        RmcCdmaMoSmsMessage, RmcCdmaMoSmsRsp, RFX_MSG_REQUEST_CDMA_SEND_SMS);

RFX_REGISTER_DATA_TO_REQUEST_ID(
        RmcCdmaMoSmsOverImsMessage, RmcCdmaMoSmsRsp, RFX_MSG_REQUEST_IMS_SEND_CDMA_SMS);

RFX_REGISTER_DATA_TO_REQUEST_ID(
        RmcCdmaMoSmsOverImsMessage, RmcCdmaMoSmsRsp, RFX_MSG_REQUEST_IMS_SEND_CDMA_SMS_EX);

/*****************************************************************************
 * Class RmcCdmaMoSmsMessage
 *****************************************************************************/
RFX_IMPLEMENT_DATA_CLASS(RmcCdmaMoSmsMessage);
RmcCdmaMoSmsMessage::RmcCdmaMoSmsMessage(void *data, int length):
        RmcSingleAtReq(data, length), m_msgRef(-1),  m_errCode(0){
    RFX_ASSERT(data != NULL);
    RFX_ASSERT(length == sizeof(m_msg));
    m_msg = *((RIL_CDMA_SMS_Message *)data);
    m_data = (void *)&m_msg;
    m_length = length;
}


RmcCdmaMoSmsMessage::~RmcCdmaMoSmsMessage() {
}


RmcCdmaMoSmsMessage::RmcCdmaMoSmsMessage(RIL_CDMA_SMS_Message *msg)
        : RmcSingleAtReq(NULL, 0), m_msgRef(-1),  m_errCode(0) {
    m_msg = *msg;
    m_data = &m_msg;
    m_length = sizeof(m_msg);
}



RmcAtSendInfo* RmcCdmaMoSmsMessage::onGetAtInfo(RfxBaseHandler *h) {
    RFX_UNUSED(h);
    String8 hexPdu;
    String8 number;
    preProcessMessage(h);
    if (!onGetPdu(hexPdu, number)) {
        this->setError(RIL_E_SYSTEM_ERR);
        return NULL;
    }
    String8 cmd = String8::format("AT+EC2KCMGS=%d,\"%s\",\"%s\"",
            (int)hexPdu.length() / 2, hexPdu.string(), number.string());
    String8 responsePrefix("+EC2KCMGS:");
    return new RmcSingleLineAtSendInfo(cmd, responsePrefix);
}


bool RmcCdmaMoSmsMessage::onGetPdu(String8 &hexPdu, String8 &address) {
    return RmcCdmaSmsConverter::toHexPdu(m_msg, hexPdu, address);
}


bool RmcCdmaMoSmsMessage::onHandleIntermediates(RfxAtLine * line, RfxBaseHandler * h) {
    RFX_UNUSED(h);
    int err;
    m_msgRef = line->atTokNextint(&err);
    m_errCode = 0;
    if (err < 0) {
        setError(RIL_E_SYSTEM_ERR);
        return false;
    }
    return true;
}

void RmcCdmaMoSmsMessage::onHandleFinalResponseForError(RfxAtLine * line,RfxBaseHandler * h) {
    RFX_UNUSED(h);
    int err;
    int errorClass = line->atTokNextint(&err);
    if (err < 0) {
        return;
    }
    m_errCode = line->atTokNextint(&err);
    if (err < 0) {
        return;
    }
    if (errorClass == ERROR_CLASS_TEMP) {
        if (ERROR_RUIM_ABSENT == m_errCode) {
            setError(RIL_E_SIM_ABSENT);
        }
    } else if ((errorClass == ERROR_CLASS_PERMANENT) && (m_errCode == ERROR_FDN_CHECK)) {
        setError(RIL_E_FDN_CHECK_FAILURE);
    }
    m_msgRef = 0;
}

void RmcCdmaMoSmsMessage::preProcessMessage(RfxBaseHandler * h) {
    if (isCtSimCard(h)) {
        /*
         * According to the spec of China Telecom, it needs the teleservice
         * id of long sms is TELESERVICE_WMT(0x1002). However, AOSP will use
         * TELESERVICE_WEMT(0x1005) as the teleservice id in SmsMessage.java.
         * In fact, most of China Telecom's network will work fine for 0x1005.
         * Only in Senzhen we will meet the problem that the message center cannot
         * handle the teleservice id 0x1005, so we need to convert the teleservice
         * id from 0x1005 to 0x1002 to make sure the long sms can work in all China
         * Telecom's network.
         */
        if (m_msg.uTeleserviceID == TELESERVICE_WEMT) {
            m_msg.uTeleserviceID = TELESERVICE_WMT;
        }
    }
}

bool RmcCdmaMoSmsMessage::isCtSimCard(RfxBaseHandler * h) {
    bool ret = false;
    int type = h->getMclStatusManager()->getIntValue(RFX_STATUS_KEY_CDMA_CARD_TYPE);
    if (type == CT_4G_UICC_CARD ||
            type == CT_UIM_SIM_CARD ||
            type == CT_3G_UIM_CARD) {
        ret = true;
    }
    return ret;
}


/*****************************************************************************
 * Class RmcCdmaMoSmsOverImsMessage
 *****************************************************************************/
RFX_IMPLEMENT_DATA_CLASS(RmcCdmaMoSmsOverImsMessage);
RmcCdmaMoSmsOverImsMessage::RmcCdmaMoSmsOverImsMessage(void *data, int length):
        RmcCdmaMoSmsMessage(data, length){
}


RmcCdmaMoSmsOverImsMessage::~RmcCdmaMoSmsOverImsMessage() {
}


bool RmcCdmaMoSmsOverImsMessage::onGetPdu(String8 &hexPdu, String8 &address) {
    return RmcCdmaSmsConverter::toHexPdu(
            const_cast<RIL_CDMA_SMS_Message &>(getMessage()), hexPdu, address, false);
}


/*****************************************************************************
 * Class RmcCdmaMoSmsRsp
 *****************************************************************************/
RFX_IMPLEMENT_DATA_CLASS(RmcCdmaMoSmsRsp);
RmcCdmaMoSmsRsp::RmcCdmaMoSmsRsp(void *data, int length):
        RmcVoidRsp(data, length) {
    RFX_ASSERT(data != NULL);
    RFX_ASSERT(length == sizeof(m_response));
    m_response = *((RIL_SMS_Response *)data);
    m_data = (void *)&m_response;
    m_length = length;
}


RmcCdmaMoSmsRsp::~RmcCdmaMoSmsRsp() {
}

RmcCdmaMoSmsRsp::RmcCdmaMoSmsRsp(int msgRef, int errCode, RIL_Errno e)
        : RmcVoidRsp(e) {
    m_response.messageRef = msgRef;
    m_response.ackPDU = NULL;
    m_response.errorCode = errCode;
    m_data = (void *)&m_response;
    m_length = sizeof(m_response);
}


/*****************************************************************************
 * Class RmcCdmaMoSmsHdlr
 *****************************************************************************/
RmcBaseRspData *RmcCdmaMoSmsHdlr::onGetRspData(RmcBaseReqData *req) {
    RmcCdmaMoSmsMessage *mo = (RmcCdmaMoSmsMessage *)req;
    return new RmcCdmaMoSmsRsp(
            mo->getMsgRef(),
            mo->getMsgErrCode(),
            req->getError());
}
