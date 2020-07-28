/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#include "WpfaCcciDataHeaderEncoder.h"
//#include "RfxRilUtils.h"

#define WPFA_D_LOG_TAG "WpfaCcciDataHeaderEncoder"

String8 WpfaCcciDataHeader::toString() const {
    return String8::format("mMsgId: %d, mTid: %d, mMsgType: %d, mParam16bit: %d",
            mMsgId, mTid, mMsgType, mParam16bit);
}

WpfaCcciDataHeaderEncoder *WpfaCcciDataHeaderEncoder::sSelf = NULL;

WpfaCcciDataHeaderEncoder::WpfaCcciDataHeaderEncoder() {
}

void WpfaCcciDataHeaderEncoder::init() {
    sSelf = new WpfaCcciDataHeaderEncoder();
}

void WpfaCcciDataHeaderEncoder::encodeHeader(WpfaCcciDataHeader data, ccci_msg_hdr_t *header) {
    header->msg_id = data.getMsgId();
    header->t_id = data.getTid();
    header->msg_type = data.getMsgType();
    header->param_16bit = data.getParams();
    mtkLogD(WPFA_D_LOG_TAG, " encodeHeader msg_id: %d, t_id:%d, msg_type:%d, mPara: %d",
            header->msg_id, header->t_id, header->msg_type, header->param_16bit);
}

WpfaCcciDataHeader WpfaCcciDataHeaderEncoder::decodeHeader(ccci_msg_hdr_t *header)
{
    uint16_t msgId = header->msg_id;
    uint16_t mTid = header->t_id;
    uint16_t mMsgType = header->msg_type;
    uint16_t mPara = header->param_16bit;

    mtkLogD(WPFA_D_LOG_TAG, " decodeHeader msg_id: %d, t_id:%d, msg_type:%d, mPara: %d",
            msgId, mTid, mMsgType, mPara);
    WpfaCcciDataHeader data(msgId, mTid, mMsgType, mPara);
    return data;
}

