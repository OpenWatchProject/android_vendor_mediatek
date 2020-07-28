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
#ifndef __RMC_CDMA_MT_SMS_H__
#define __RMC_CDMA_MT_SMS_H__
/*****************************************************************************
 * Include
 *****************************************************************************/
#include "common/RmcData.h"
#include "common/RmcMessageHandler.h"

/*****************************************************************************
 * Class RmcCdmaMtSmsMessage
 *****************************************************************************/
class RmcCdmaMtSmsMessage : public RmcVoidUrsp {
    RFX_DECLARE_DATA_CLASS(RmcCdmaMtSmsMessage);
public:
    int getReplySeqNo() const {
        return m_replySeqNo;
    }

    const RIL_CDMA_SMS_Address *getAddress() const {
        return &m_msg.sAddress;
    }

public:
    RmcCdmaMtSmsMessage(int urcId, int slotId, String8 &hexPdu);

private:
    RIL_CDMA_SMS_Message m_msg;
    int m_replySeqNo;
};


/*****************************************************************************
 * Class RmcCdmaMtSmsHdlr
 *****************************************************************************/
class RmcCdmaMtSmsHdlr : public RmcBaseUrcHandler {
// Constructor / Destructor
public:
    RmcCdmaMtSmsHdlr(RfxBaseHandler *h, int type):RmcBaseUrcHandler(h), m_type(type), m_data(-1) {}
    virtual ~RmcCdmaMtSmsHdlr() {}

public:
    virtual bool onHandleRawUrc(RfxAtLine * line);
    virtual RmcBaseUrspData *onGetUrcData(int slotId);

private:
    int m_type;
    int m_data;
    String8 m_hexPdu;
};


#endif /* __RMC_CDMA_MT_SMS_H__ */
