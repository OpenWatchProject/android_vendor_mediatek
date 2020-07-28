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
#ifndef __RMC_CDMA_MO_SMS_H__
#define __RMC_CDMA_MO_SMS_H__
/*****************************************************************************
 * Include
 *****************************************************************************/
#include "common/RmcData.h"
#include "common/RmcMessageHandler.h"

/*****************************************************************************
 * Class RmcCdmaMoSmsMessage
 *****************************************************************************/
class RmcCdmaMoSmsMessage : public RmcSingleAtReq {
    RFX_DECLARE_DATA_CLASS(RmcCdmaMoSmsMessage);

public:
    int getMsgRef() const {
        return m_msgRef;
    }

    int getMsgErrCode() const {
        return m_errCode;
    }

    const RIL_CDMA_SMS_Message& getMessage() const {
        return m_msg;
    }

public:
    RmcCdmaMoSmsMessage(RIL_CDMA_SMS_Message *msg);

protected:
    virtual RmcAtSendInfo* onGetAtInfo(RfxBaseHandler *h);
    virtual bool onHandleIntermediates(RfxAtLine * line, RfxBaseHandler * h);
    virtual void onHandleFinalResponseForError(RfxAtLine * line,RfxBaseHandler * h);

// Overridable
protected:
    virtual bool onGetPdu(String8 &hexPdu, String8 &address);

private:
    RIL_CDMA_SMS_Message m_msg;
    int m_msgRef;
    int m_errCode;

// Implement
private:
    const int TELESERVICE_WMT       = 0x1002;
    const int TELESERVICE_WEMT      = 0x1005;
    const int ERROR_RUIM_ABSENT     = 107;
    const int ERROR_FDN_CHECK       = 14;
    const int ERROR_CLASS_TEMP      = 2;
    const int ERROR_CLASS_PERMANENT = 3;

    bool isCtSimCard(RfxBaseHandler * h);
    void preProcessMessage(RfxBaseHandler * h);

};


/*****************************************************************************
 * Class RmcCdmaMoSmsOverImsMessage
 *****************************************************************************/
class RmcCdmaMoSmsOverImsMessage: public RmcCdmaMoSmsMessage {
    RFX_DECLARE_DATA_CLASS(RmcCdmaMoSmsOverImsMessage);
protected:
    virtual bool onGetPdu(String8 &hexPdu, String8 &address);
};


/*****************************************************************************
 * Class RmcCdmaMoSmsRsp
 *****************************************************************************/
class RmcCdmaMoSmsRsp : public RmcVoidRsp {
    RFX_DECLARE_DATA_CLASS(RmcCdmaMoSmsRsp);
public:
    RmcCdmaMoSmsRsp(int msgRef, int errCode, RIL_Errno e);

private:
    RIL_SMS_Response m_response;
};


/*****************************************************************************
 * Class RmcCdmaMoSmsHdlr
 *****************************************************************************/
class RmcCdmaMoSmsHdlr : public RmcBaseRequestHandler {
public:
    RmcCdmaMoSmsHdlr(RfxBaseHandler *h) :RmcBaseRequestHandler(h){}
    virtual ~RmcCdmaMoSmsHdlr() {}
public:
    virtual RmcBaseRspData *onGetRspData(RmcBaseReqData *req);
};

#endif /* __RMC_CDMA_MO_SMS_H__ */
