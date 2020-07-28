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
#include "RmcCdmaMtSms.h"
#include "RmcCdmaSmsConverter.h"
#include "RmcCdmaSmsAck.h"
#include "RfxStringData.h"

/*****************************************************************************
 * Register Data Class
 *****************************************************************************/

RFX_REGISTER_DATA_TO_URC_ID(
        RmcCdmaMtSmsMessage, RFX_MSG_URC_CDMA_NEW_SMS);
RFX_REGISTER_DATA_TO_URC_ID(
        RmcCdmaMtSmsMessage, RFX_MSG_URC_CDMA_NEW_SMS_EX);


/*****************************************************************************
 * Class RmcCdmaMtSmsMessage
 *****************************************************************************/
RFX_IMPLEMENT_DATA_CLASS(RmcCdmaMtSmsMessage);
RmcCdmaMtSmsMessage::RmcCdmaMtSmsMessage(void *data, int length):
        RmcVoidUrsp(data, length), m_replySeqNo(-1){
    RFX_ASSERT(data != NULL);
    RFX_ASSERT(length == sizeof(m_msg));
    m_msg = *((RIL_CDMA_SMS_Message *)data);
    m_data = (void *)&m_msg;
    m_length = length;
}

RmcCdmaMtSmsMessage::RmcCdmaMtSmsMessage(int urcId, int slotId, String8 &hexPdu) :
        RmcVoidUrsp(urcId, slotId), m_replySeqNo(-1) {
    RmcCdmaSmsConverter::toMessage(&m_msg, hexPdu, &m_replySeqNo);
    m_data = (void *)&m_msg;
    m_length = sizeof(m_msg);
}

RmcCdmaMtSmsMessage::~RmcCdmaMtSmsMessage() {
}


/*****************************************************************************
 * Class RmcCdmaMtSmsHdlr
 *****************************************************************************/
bool RmcCdmaMtSmsHdlr::onHandleRawUrc(RfxAtLine * line) {
    int err;
    m_data = line->atTokNextint(&err);
    if (err < 0) {
        return false;
    }
    if (m_type == CDMA_SMS_INBOUND_VMI) {
        int num = m_data;
        if (num > 99) {
            num = 99;
        }
        num = ((num / 10) << 4) | (num % 10);
        m_hexPdu.appendFormat("000002100302020040080c0003100000010210000B01%02x", num);
    } else {
        const char *pdu = line->atTokNextstr(&err);
        if (err < 0 || pdu == NULL) {
            return false;
        }
        m_hexPdu.setTo(pdu);
    }
    return true;
}

RmcBaseUrspData *RmcCdmaMtSmsHdlr::onGetUrcData(int slotId) {
    RmcCdmaMtSmsMessage* pMtMsg =  new RmcCdmaMtSmsMessage(
            RFX_MSG_URC_CDMA_NEW_SMS, slotId,
            m_hexPdu);
    int mtType = getHandler()->getMclStatusManager()->getIntValue(
        RFX_STATUS_KEY_CDMA_INBOUND_SMS_TYPE,
        CDMA_SMS_INBOUND_NONE);
    if (mtType != CDMA_SMS_INBOUND_NONE) {
        if (m_type == CDMA_SMS_INBOUND_VMI) {
            getHandler()->getMclStatusManager()->setIntValue(
                RFX_STATUS_KEY_CDMA_PENDING_VMI, m_data);
        } else {
            RIL_CDMA_SMS_Ack ack;
            ack.uErrorClass = (RIL_CDMA_SMS_ErrorClass)2;
            ack.uSMSCauseCode = 33;
            RmcCdmaSmsAck errorAck(m_type, pMtMsg->getReplySeqNo(), ack, pMtMsg->getAddress());
            getHandler()->sendEvent(RFX_MSG_EVENT_CDMA_SMS_NEW_SMS_ERR_ACK,
                RfxStringData((void *)errorAck.getCmd().string(), errorAck.getCmd().length()),
                    RIL_CMD_PROXY_7, slotId);
        }
        delete pMtMsg;
        return NULL;
    }

    getHandler()->getMclStatusManager()->setIntValue(
        RFX_STATUS_KEY_CDMA_INBOUND_SMS_TYPE,
        m_type);
    if (m_type == CDMA_SMS_INBOUND_VMI) {
        getHandler()->getMclStatusManager()->setIntValue(
            RFX_STATUS_KEY_CDMA_PENDING_VMI, -1);
    } else {
        getHandler()->getMclStatusManager()->setIntValue(
                RFX_STATUS_KEY_CDMA_SMS_REPLY_SEQ_NO,
                pMtMsg->getReplySeqNo());
        Vector<char> addr;
        addr.appendArray((const char *)pMtMsg->getAddress(), sizeof(RIL_CDMA_SMS_Address));
        getHandler()->getMclStatusManager()->setValue(
                RFX_STATUS_KEY_CDMA_SMS_ADDR,
                RfxVariant(addr));
    }
    return pMtMsg;
}


