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
#include "RmcCdmaSmsConverter.h"
#include "RmcCdmaSmsAck.h"
#include "RfxIntsData.h"

/*****************************************************************************
 * Register Data Class
 *****************************************************************************/
RFX_REGISTER_DATA_TO_REQUEST_ID(
        RmcCdmaSmsAck, RmcVoidRsp, RFX_MSG_REQUEST_CDMA_SMS_ACKNOWLEDGE);

RFX_REGISTER_DATA_TO_REQUEST_ID(
        RmcCdmaSmsAck, RmcVoidRsp, RFX_MSG_REQUEST_CDMA_SMS_ACKNOWLEDGE_EX);

RFX_REGISTER_DATA_TO_REQUEST_ID(
        RmcCdmaSmsAck, RmcVoidRsp, RFX_MSG_REQUEST_CDMA_SMS_ACKNOWLEDGE_INTERNAL);

/*****************************************************************************
 * Class RmcCdmaSmsAck
 *****************************************************************************/
RFX_IMPLEMENT_DATA_CLASS(RmcCdmaSmsAck);
RmcCdmaSmsAck::RmcCdmaSmsAck(void *data, int length) :
        RmcSingleAtReq(data, length){
    RFX_ASSERT(data != NULL);
    RFX_ASSERT(length == sizeof(m_ack));
    m_ack = *((RIL_CDMA_SMS_Ack *)data);
    m_data = (void *)&m_ack;
    m_length = length;
}

RmcCdmaSmsAck::~RmcCdmaSmsAck() {
}

RmcCdmaSmsAck::RmcCdmaSmsAck(int type, int replyReqNo, RIL_CDMA_SMS_Ack ack,
        const RIL_CDMA_SMS_Address *pAddr) :
    RmcSingleAtReq(NULL, 0), m_ack(ack){
    setCmd(type, replyReqNo, pAddr);
}

RmcAtSendInfo* RmcCdmaSmsAck::onGetAtInfo(RfxBaseHandler *h) {
    int mtType = h->getMclStatusManager()->getIntValue(
            RFX_STATUS_KEY_CDMA_INBOUND_SMS_TYPE,
            CDMA_SMS_INBOUND_NONE);

    String8 hexPdu;
    int replySeqNo = h->getMclStatusManager()->getIntValue(
            RFX_STATUS_KEY_CDMA_SMS_REPLY_SEQ_NO,
            -1);
    if (replySeqNo == -1) {
        setError(RIL_E_NO_SMS_TO_ACK);
        return NULL;
    }
    const RfxVariant & addr = h->getMclStatusManager()->getValue(
        RFX_STATUS_KEY_CDMA_SMS_ADDR);
    RIL_CDMA_SMS_Address address;
    RFX_ASSERT(sizeof(address) == addr.asCharVector().size());
    memcpy(&address, addr.asCharVector().array(), sizeof(address));
    if (setCmd(mtType, replySeqNo, &address)) {
        return new RmcNoLineAtSendInfo(m_cmd);
    } else {
        setError(RIL_E_NO_SMS_TO_ACK);
        return NULL;
    }
}

bool RmcCdmaSmsAck::setCmd(int type, int replySeqNo, const RIL_CDMA_SMS_Address *pAddr) {
    switch (type) {
        case CDMA_SMS_INBOUND_COMM:
            m_cmd.setTo("AT+EC2KCNMA=");
            break;

        case CDMA_SMS_INBOUND_VMI:

        default:
            return false;
        }

    String8 hexPdu;
    RmcCdmaSmsConverter::toHexPdu(
            &m_ack,
            replySeqNo,
            (RIL_CDMA_SMS_Address *)pAddr,
            hexPdu);
    m_cmd.appendFormat("%d,\"%s\"", (int)hexPdu.length() / 2, hexPdu.string());
    return true;
}


/*****************************************************************************
 * Class RmcCdmaSmsAckHdlr
 *****************************************************************************/
void RmcCdmaSmsAckHdlr::onAfterResponse() {
    getHandler()->getMclStatusManager()->setIntValue(
            RFX_STATUS_KEY_CDMA_INBOUND_SMS_TYPE,
            CDMA_SMS_INBOUND_NONE);
    int vmi = getHandler()->getMclStatusManager()->getIntValue(
                RFX_STATUS_KEY_CDMA_PENDING_VMI, -1);
    if (vmi != -1){
        int slotId = getHandler()->getMclStatusManager()->getSlotId();
        getHandler()->sendEvent(
            RFX_MSG_EVENT_CDMA_SMS_PENDING_VMI,
            RfxIntsData(&vmi, 1), RIL_CMD_PROXY_URC, slotId, -1, -1, 100);
    }
}


