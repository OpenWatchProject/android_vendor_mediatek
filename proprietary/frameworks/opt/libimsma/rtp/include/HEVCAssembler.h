/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
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

/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*****************************************************************************
 *
 * Filename:
 * ---------
 *  HEVCAssembler.h
 *
 * Project:
 * --------
 *   Android
 *
 * Description:
 * ------------
 *   Assemble RTP packet to HEVC NAL class interface
 *
 * Author:
 * -------
 *   Haizhen.Wang(mtk80691)
 *
 ****************************************************************************/

#ifndef _IMS_HEVC_ASSEMBLER_H_

#define _IMS_HEVC_ASSEMBLER_H_

#include "RTPAssembler.h"

#include <utils/List.h>
#include <utils/RefBase.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/AString.h>
#include <media/stagefright/foundation/AHandler.h>
#include <media/stagefright/foundation/ALooper.h>
using namespace android;
using android::status_t;
#define DEBUG_DUMP_ACCU
#define PACKETLOSTRECORDNUM 16
namespace imsma
{


struct HEVCAssembler : public RTPAssembler {
    //ToDo: why shouldn't we define const sp<AMessage> & notify as parameter
    //struct NALFragMentsInfo;

    HEVCAssembler(const sp<AMessage> notify);

    uint32_t getLostCount() {
        return mLostCount;
    }

    uint32_t getIDamageCount() {
        return mIDamageNum;
    }

    bool isCSD(const sp<ABuffer>& /*accessUnit*/){
        //To do implementation
        return true;
    }

protected:
    virtual ~HEVCAssembler();

private:
    sp<AMessage> mNotifyMsg;

    //uint32_t mAccessUnitRTPTime;
    bool mNextExpectedSeqNoValid;
    uint32_t mNextExpectedSeqNo;
    bool mAccessUnitDamaged;

    List<uint32_t> mPackLostList; //at present, we only register 16 packets
    int64_t mLastLost;

    int32_t mAccuCount;
    int32_t mLostCount;

    int32_t mIDamageNum;

    struct NALFragMentsInfo: public RefBase {
        NALFragMentsInfo() {
            mIsCompleted = false;
            mIsDamaged = false;
            mStartSeqNum = 0;
            mStopSeqNum = 0;
            mTotalCount = 0;
            mNALSize = 0;
            mNALType = 0;
            mNRI = 1;
        }

    protected:
        ~NALFragMentsInfo() {
            if(!mNALFragments.empty()) {
                mNALFragments.clear();
            }
        }

    public:
        bool mIsCompleted;
        bool mIsDamaged;
        uint32_t mStartSeqNum;
        uint32_t mStopSeqNum;
        uint32_t mTotalCount;
        uint32_t mNALSize;
        uint32_t mNALType;
        uint32_t mNRI;
        List<sp<ABuffer> > mNALFragments;

    };

    sp<NALFragMentsInfo> mpNALFragmentInfo;

public:
    virtual AssemblyStatus assembleMore(const sp<RTPSource> source);
    //virtual void onByeReceived();
    virtual void packetLost();
    virtual void packetLostRegister();
    virtual void reset();
    virtual void flushQueue();

private:
    AssemblyStatus addNALUnit(const sp<RTPSource> &source);
    void addSingleNALUnit(const sp<ABuffer> &buffer);
    AssemblyStatus addFragmentedNALUnit(List<sp<ABuffer> > *queue);
    bool addSingleTimeAggregationPacket(const sp<ABuffer> &buffer);
    sp<ABuffer> assembleToNAL(sp<NALFragMentsInfo> nalFragmentInfo);
    void submitAccessUnit(const sp<ABuffer>& accessUnit);

#ifdef DEBUG_DUMP_ACCU
    int64_t mDumpAcuu;
    int mAccuFd;
#endif
    DISALLOW_EVIL_CONSTRUCTORS(HEVCAssembler);
};

}  // namespace android

#endif  // _IMS_AVC_ASSEMBLER_H_
