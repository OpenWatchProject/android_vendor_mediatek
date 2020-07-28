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

#ifndef WPFA_DRIVER_CCCI_DATA_ENCODER_H
#define WPFA_DRIVER_CCCI_DATA_ENCODER_H

// #include "utils/String8.h"
// #include "utils/RefBase.h"
#include "String8.h"
#include "RefBase.h"
#include "WpfaDriverMessage.h"

#include <stdint.h>
#include <mtk_log.h>

using ::android::String8;

class WpfaCcciDataHeader {
    public:
        WpfaCcciDataHeader() :
                mMsgId(0),
                mTid(0),
                mMsgType(0),
                mParam16bit(0){
        }

        WpfaCcciDataHeader(uint16_t msgId, uint16_t tId,
                uint16_t msgType, uint16_t para) :
                mMsgId(msgId),
                mTid(tId),
                mMsgType(msgType),
                mParam16bit(para){
        }

        uint16_t getMsgId() const {
            return mMsgId;
        }

        uint16_t getTid() const {
            return mTid;
        }

        uint16_t getMsgType() const {
            return mMsgType;
        }

        uint16_t getParams() const {
            return mParam16bit;
        }

        String8 toString() const;

    private:
        uint16_t mMsgId;
        uint16_t mTid;
        uint16_t mMsgType;
        uint16_t mParam16bit;
};

class WpfaCcciDataHeaderEncoder {

    public:
        static const size_t HEADER_SIZE = 8;

    private:

        WpfaCcciDataHeaderEncoder();

    public:

        static void init();
        static WpfaCcciDataHeader decodeHeader(ccci_msg_hdr_t *header);
        static void encodeHeader(WpfaCcciDataHeader data, ccci_msg_hdr_t *header);


    private:
        static WpfaCcciDataHeaderEncoder *sSelf;
};
#endif
