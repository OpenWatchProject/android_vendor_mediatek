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
*  RTPAssebler.cpp
*
* Project:
* --------
*   Android
*
* Description:
* ------------
*   baisc class for assemble bitstream to RTP packet
*
* Author:
* -------
*   Haizhen.Wang(mtk80691)
*
****************************************************************************/

//#define LOG_NDEBUG 0
#define LOG_TAG "[VT][RTP]RTPAssembler"
#include <utils/Log.h>

#include "RTPAssembler.h"

#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/foundation/AMessage.h>

#include <stdint.h>

namespace imsma
{

RTPAssembler::RTPAssembler()
    : mFirstFailureTimeUs(-1)
{

}

//test
//RTPAssembler::RTPAssembler(const RTPAssembler & src){
//  mFirstFailureTimeUs = src.mFirstFailureTimeUs;
//}

void RTPAssembler::onPacketReceived(const sp<RTPSource> &source)
{
    //test:
    //RTPAssembler dest(*this);

    AssemblyStatus status;

    for(;;) {
        status = assembleMore(source);

        if(status == WRONG_SEQUENCE_NUMBER) {
            if(mFirstFailureTimeUs >= 0) {
                if(ALooper::GetNowUs() - mFirstFailureTimeUs > 5000ll) {

                    mFirstFailureTimeUs = -1;

                    // LOG(VERBOSE) << "waited too long for packet.";
                    packetLostRegister();
                    continue;
                }
            } else {
                mFirstFailureTimeUs = ALooper::GetNowUs();
            }

            break;
        } else if(status == LARGE_SEQUENCE_GAP) {
            mFirstFailureTimeUs = -1;
            packetLostRegister();
            continue;

        } else {
            mFirstFailureTimeUs = -1;

            if(status == NOT_ENOUGH_DATA) {
                break;
            }
        }
    }

    //maybe we have some packetlost updata
    packetLost();
}

// static
void RTPAssembler::CopyMetas(const sp<ABuffer> &to, const sp<ABuffer> &from)
{
    sp<AMessage> to_meta = to->meta();
    sp<AMessage> from_meta = from->meta();

    //copy rtp-time meta
    uint32_t rtpTime;
    CHECK(from_meta->findInt32("rtp-time", (int32_t *) &rtpTime));
    to_meta->setInt32("rtp-time", rtpTime);

    // Copy the seq number.
    to->setInt32Data(from->int32Data());

    //copy the rotation,facing,flip info
    int32_t ccw_rotation = 0;
    int32_t camera_facing = IMSMA_CAMERA_FACING_UNKNOW;
    int32_t flip = IMSMA_CAMERA_NO_FLIP;

    from_meta->findInt32("ccw_rotation",&ccw_rotation);
    from_meta->findInt32("camera_facing",&camera_facing);
    from_meta->findInt32("flip",&flip);

    to_meta->setInt32("ccw_rotation",ccw_rotation);
    to_meta->setInt32("camera_facing",camera_facing);
    to_meta->setInt32("flip",flip);

    //copy maker info
    int32_t marker_bit = 0;
    from_meta->findInt32("M", &marker_bit);

    if(marker_bit > 0) {
        ALOGV("%s,last accu of frame",__FUNCTION__);
        to_meta->setInt32("M",marker_bit);
    }
}

// static
#if 0
sp<ABuffer> ARTPAssembler::MakeADTSCompoundFromAACFrames(
    unsigned profile,
    unsigned samplingFreqIndex,
    unsigned channelConfig,
    const List<sp<ABuffer> > &frames)
{
    size_t totalSize = 0;

    for(List<sp<ABuffer> >::const_iterator it = frames.begin();
            it != frames.end(); ++it) {
        // Each frame is prefixed by a 7 byte ADTS header
        totalSize += (*it)->size() + 7;
    }

    sp<ABuffer> accessUnit = new ABuffer(totalSize);
    size_t offset = 0;

    for(List<sp<ABuffer> >::const_iterator it = frames.begin();
            it != frames.end(); ++it) {
        sp<ABuffer> nal = *it;
        uint8_t *dst = accessUnit->data() + offset;

        static const unsigned kADTSId = 0;
        static const unsigned kADTSLayer = 0;
        static const unsigned kADTSProtectionAbsent = 1;

        unsigned frameLength = nal->size() + 7;

        dst[0] = 0xff;

        dst[1] =
            0xf0 | (kADTSId << 3) | (kADTSLayer << 1) | kADTSProtectionAbsent;

        dst[2] = (profile << 6)
                 | (samplingFreqIndex << 2)
                 | (channelConfig >> 2);

        dst[3] = ((channelConfig & 3) << 6) | (frameLength >> 11);

        dst[4] = (frameLength >> 3) & 0xff;
        dst[5] = (frameLength & 7) << 5;
        dst[6] = 0x00;

        memcpy(dst + 7, nal->data(), nal->size());
        offset += nal->size() + 7;
    }

    CopyTimes(accessUnit, *frames.begin());

    return accessUnit;
}

// static
sp<ABuffer> ARTPAssembler::MakeCompoundFromPackets(
    const List<sp<ABuffer> > &packets)
{
    size_t totalSize = 0;

    for(List<sp<ABuffer> >::const_iterator it = packets.begin();
            it != packets.end(); ++it) {
        totalSize += (*it)->size();
    }

    sp<ABuffer> accessUnit = new ABuffer(totalSize);
    size_t offset = 0;

    for(List<sp<ABuffer> >::const_iterator it = packets.begin();
            it != packets.end(); ++it) {
        sp<ABuffer> nal = *it;
        memcpy(accessUnit->data() + offset, nal->data(), nal->size());
        offset += nal->size();
    }

    CopyTimes(accessUnit, *packets.begin());

    return accessUnit;
}

#ifdef MTK_AOSP_ENHANCEMENT
void ARTPAssembler::setFlush(bool flush)
{
    mbFlush = flush;
}

void ARTPAssembler::updatePacketReceived(const sp<ARTPSource> &source,
        const sp<ABuffer> &buffer)
{
    evaluateDuration(source, buffer);
}
#endif // #ifdef MTK_AOSP_ENHANCEMENT
#endif

}  // namespace imsma
