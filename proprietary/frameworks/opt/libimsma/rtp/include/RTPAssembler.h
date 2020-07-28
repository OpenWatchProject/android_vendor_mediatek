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
*  RTPAssebler.h
*
* Project:
* --------
*   Android
*
* Description:
* ------------
*   Interface of baisc class for assemble bitstream to RTP packet
*
* Author:
* -------
*   Haizhen.Wang(mtk80691)
*
****************************************************************************/

#ifndef _IMS_RTP_ASSEMBLER_H_

#define _IMS_RTP_ASSEMBLER_H_

#include <media/stagefright/foundation/ABase.h>
#include <utils/List.h>
#include <utils/StrongPointer.h>
#include <utils/RefBase.h>

#include <utils/List.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/AString.h>
#include <media/stagefright/foundation/AHandler.h>
#include <media/stagefright/foundation/ALooper.h>
#include "RTPBase.h"
#include "RTPSource.h"
using namespace android;
using android::status_t;

namespace imsma
{

struct RTPAssembler : public RefBase {
    enum AssemblyStatus {
        MALFORMED_PACKET,
        WRONG_SEQUENCE_NUMBER,
        LARGE_SEQUENCE_GAP,
        NOT_ENOUGH_DATA,
        SKIP_MISS_PACKET,
        OK
    };
    enum {
        kWhatAccessUnit = 'accu',
        kWhatPacketLost = 'plst',
    };

    RTPAssembler();

    void onPacketReceived(const sp<RTPSource> &source);
    virtual void flushQueue() = 0;
    virtual void reset() = 0;
    virtual uint32_t getLostCount() = 0;
    virtual uint32_t getIDamageCount() = 0;
    virtual bool isCSD(const sp<ABuffer>& accessUnit) = 0;

    //virtual void onByeReceived() = 0;
protected:
    virtual AssemblyStatus assembleMore(const sp<RTPSource> source) = 0;
    virtual void packetLost() = 0;
    virtual void packetLostRegister() = 0;

    /*
    static void CopyTimes(const sp<ABuffer> &to, const sp<ABuffer> &from);

    static sp<ABuffer> MakeADTSCompoundFromAACFrames(
            unsigned profile,
            unsigned samplingFreqIndex,
            unsigned channelConfig,
            const List<sp<ABuffer> > &frames);

    static sp<ABuffer> MakeCompoundFromPackets(
            const List<sp<ABuffer> > &frames);
    */
private:
    int64_t mFirstFailureTimeUs;
    static const uint32_t kLargeSequenceGap = 20;

    DISALLOW_EVIL_CONSTRUCTORS(RTPAssembler);


    // do something before time established
    /*
    virtual void updatePacketReceived(const sp<RTPSource> &source,
            const sp<ABuffer> &buffer);
    virtual void setNextExpectedSeqNo(uint32_t rtpSeq) {rtpSeq++ ;return; };
    */

protected:
    static void CopyMetas(const sp<ABuffer> &to, const sp<ABuffer> &from);
    // notify ARTPSource to updateExpectedTimeoutUs, mainly for audio
    /*
    virtual void evaluateDuration(const sp<RTPSource> &source,
            const sp<ABuffer> &buffer) {
                  if(source.get()== NULL ||  buffer.get()==NULL)
                        return;

        }*/
    AssemblyStatus getAssembleStatus(List<sp<ABuffer> > *queue,
                                     uint32_t nextExpectedSeq) {
        sp<ABuffer> buffer = *--queue->end();
        uint32_t seq = buffer->int32Data();
        return seq - nextExpectedSeq > kLargeSequenceGap ?
               LARGE_SEQUENCE_GAP : WRONG_SEQUENCE_NUMBER;
    }

};

}  // namespace android

#endif  // _IMS_RTP_ASSEMBLER_H_
