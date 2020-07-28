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
#include "RmcRuimSmsMem.h"

/*****************************************************************************
 * Register Data Class
 *****************************************************************************/

RFX_REGISTER_DATA_TO_REQUEST_ID(
        RmcCdmaGetRuimSmsMemReq, RmcCdmaGetRuimSmsMemRsp, RFX_MSG_REQUEST_GET_SMS_RUIM_MEM_STATUS);

/*****************************************************************************
 * Class RmcCdmaGetRuimSmsMemReq
 *****************************************************************************/
RFX_IMPLEMENT_DATA_CLASS(RmcCdmaGetRuimSmsMemReq);
RmcCdmaGetRuimSmsMemReq::RmcCdmaGetRuimSmsMemReq(void *data, int length):
        RmcSingleAtReq(data, length), m_used(0), m_total(0) {
}


RmcCdmaGetRuimSmsMemReq::~RmcCdmaGetRuimSmsMemReq() {
}


RmcAtSendInfo* RmcCdmaGetRuimSmsMemReq::onGetAtInfo(RfxBaseHandler *h) {
    RFX_UNUSED(h);
    String8 cmd("AT+EC2KCPMS?");
    String8 responsePrefix("+EC2KCPMS:");
    return new RmcSingleLineAtSendInfo(cmd, responsePrefix);
}

bool RmcCdmaGetRuimSmsMemReq::onHandleIntermediates(RfxAtLine * line,RfxBaseHandler * h) {
    RFX_UNUSED(h);
    int err;
    line->atTokNextstr(&err);
    if (err < 0) {
        setError(RIL_E_SYSTEM_ERR);
        return false;
    }
    m_used = line->atTokNextint(&err);
    if (err < 0) {
        return false;
    }
    m_total = line->atTokNextint(&err);
    if (err < 0) {
        return false;
    }
    return true;
}


/*****************************************************************************
 * Class RmcCdmaGetRuimSmsMemRsp
 *****************************************************************************/
RFX_IMPLEMENT_DATA_CLASS(RmcCdmaGetRuimSmsMemRsp);
RmcCdmaGetRuimSmsMemRsp::RmcCdmaGetRuimSmsMemRsp(void *data, int length):
        RmcVoidRsp(data, length) {
    if (data != NULL) {
        m_info = *((MemInfo *)data);
        m_data = (void *)&m_info;
        m_length = length;
    }
}

RmcCdmaGetRuimSmsMemRsp::RmcCdmaGetRuimSmsMemRsp(int total, int used, RIL_Errno e)
        :RmcVoidRsp(e), m_info(total, used) {
    if (e == RIL_E_SUCCESS) {
        m_data = (void *)&m_info;
        m_length = sizeof(m_info);
    }
}


RmcCdmaGetRuimSmsMemRsp::~RmcCdmaGetRuimSmsMemRsp() {
}

/*****************************************************************************
 * Class RmcCdmaGetRuimSmsMemHdlr
 *****************************************************************************/
RmcBaseRspData *RmcCdmaGetRuimSmsMemHdlr::onGetRspData(RmcBaseReqData* req) {
    RmcCdmaGetRuimSmsMemReq * memReq = (RmcCdmaGetRuimSmsMemReq *)req;
    return new RmcCdmaGetRuimSmsMemRsp(
            memReq->getTotal(),
            memReq->getUsed(),
            req->getError());
}



