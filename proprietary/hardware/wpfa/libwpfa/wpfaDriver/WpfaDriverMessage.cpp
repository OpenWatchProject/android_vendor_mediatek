/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#include "WpfaDriverMessage.h"


using ::android::String8;
using ::android::RefBase;
using ::android::sp;

#define WPFA_D_LOG_TAG "WpfaDriverMessage"

WpfaDriverMessage::WpfaDriverMessage() :
        mMsgId(0),
        mTid(0),
        mType(0),
        mParams(0),
        mData(NULL) {

}

WpfaDriverMessage::~WpfaDriverMessage() {
    if (mData != NULL) {
        delete(mData);
    }
}

sp<WpfaDriverMessage> WpfaDriverMessage::obtainMessage(uint16_t msgId, uint16_t tId,
            uint16_t type, uint16_t params, const WpfaDriverBaseData &data) {
    sp<WpfaDriverMessage> msg = new WpfaDriverMessage();
    msg->mMsgId = msgId;
    msg->mTid = tId;
    msg->mType = type;
    msg->mParams = params;
    msg->mData = copyData(msgId, &data);
    return msg;
}

sp<WpfaDriverMessage> WpfaDriverMessage::obtainMessage(uint16_t msgId, uint16_t tId,
            uint16_t type, uint16_t params) {
    sp<WpfaDriverMessage> msg = new WpfaDriverMessage();
    msg->mMsgId = msgId;
    msg->mTid = tId;
    msg->mType = type;
    msg->mParams = params;
    msg->mData = NULL;
    return msg;
}

int WpfaDriverMessage::getCcciMsgBodySize(uint16_t msgId) {
    switch (msgId) {
        case MSG_M2A_REG_DL_FILTER:
            return CCCI_BODY_REG_FILTER_SIZE;
            break;

        case MSG_M2A_DEREG_DL_FILTER:
            return CCCI_BODY_DEREG_FILTER_SIZE;
            break;

        case MSG_A2M_REG_REPLY:
            return CCCI_BODY_REG_ACCEPT_SIZE;
            break;

        case MSG_A2M_DEREG_REPLY:
            return CCCI_BODY_DEREG_ACCEPT_SIZE;
            break;

        case MSG_A2M_WPFA_VERSION:
        case MSG_M2A_WPFA_VERSION:
            return CCCI_BODY_FILTER_VER_SIZE;
            break;

        default:
            return 0;
    }
}

WpfaDriverBaseData* WpfaDriverMessage::copyData(uint16_t msgId, const WpfaDriverBaseData *data) {
    switch (msgId) {
        case MSG_M2A_REG_DL_FILTER:
            return WpfaDriverRegFilterData::copyDataByObj(data);
            break;

        case MSG_M2A_DEREG_DL_FILTER:
            return WpfaDriverDeRegFilterData::copyDataByObj(data);
            break;

        case MSG_A2M_REG_REPLY:
        case MSG_A2M_DEREG_REPLY:
            return WpfaDriverAcceptData::copyDataByObj(data);
            break;

        case MSG_A2M_WPFA_VERSION:
        case MSG_M2A_WPFA_VERSION:
            return WpfaDriverVersionData::copyDataByObj(data);
            break;

        case MSG_M2A_UL_IP_PKT:
            return WpfaDriverULIpPkt::copyDataByObj(data);
            break;

        default:
            return NULL;
    }
}

String8 WpfaDriverMessage::toString() const {
    return String8::format(
            "MsgId=%d, Tid=%d, Type=%d, Params=%d",
            mMsgId, mTid, mType, mParams);
}
