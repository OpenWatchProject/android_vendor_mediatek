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
#ifndef __RMC_RUIM_SMS_WRITE_H__
#define __RMC_RUIM_SMS_WRITE_H__
/*****************************************************************************
 * Include
 *****************************************************************************/
#include "common/RmcData.h"
#include "common/RmcMessageHandler.h"

/*****************************************************************************
 * Class RmcCdmaWriteRuimSmsReq
 *****************************************************************************/
class RmcCdmaWriteRuimSmsReq: RmcSingleAtReq {
    RFX_DECLARE_DATA_CLASS(RmcCdmaWriteRuimSmsReq);
public:
    virtual RmcAtSendInfo* onGetAtInfo(RfxBaseHandler *h);

    virtual bool onHandleIntermediates(RfxAtLine * line, RfxBaseHandler * h);

    int getIndex() const {
        return m_index;
    }

private:
    RIL_CDMA_SMS_WriteArgs m_writeArgs;
    int m_index;
};



/*****************************************************************************
 * Class RmcCdmaWriteRuimSmsRsp
 *****************************************************************************/
class RmcCdmaWriteRuimSmsRsp: public RmcVoidRsp {
    RFX_DECLARE_DATA_CLASS(RmcCdmaWriteRuimSmsRsp);
public:
    RmcCdmaWriteRuimSmsRsp(int index, RIL_Errno e);

private:
    int m_index;
};


/*****************************************************************************
 * Class RmcCdmaWriteRuimSmsHdlr
 *****************************************************************************/
class RmcCdmaWriteRuimSmsHdlr : public RmcBaseRequestHandler {
public:
    RmcCdmaWriteRuimSmsHdlr(RfxBaseHandler *h) :RmcBaseRequestHandler(h){}
    virtual ~RmcCdmaWriteRuimSmsHdlr() {}
public:
    virtual RmcBaseRspData *onGetRspData(RmcBaseReqData *req);
};



#endif /* __RMC_RUIM_SMS_WRITE_H__ */
