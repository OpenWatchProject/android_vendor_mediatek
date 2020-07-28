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
 *  RTPSource.cpp
 *
 * Project:
 * --------
 *   Andriod
 *
 * Description:
 * ------------
 *   parse RTP/RTCP packet and jitter buffer hanlde
 *
 * Author:
 * -------
 *   Haizhen.Wang(mtk80691)
 *
 ****************************************************************************/

//#define LOG_NDEBUG 0
#define LOG_TAG "[VT][RTP]RTPSource"
#include <utils/Log.h>

#include "RTPSource.h"

#include "AVCAssembler.h"
#include "HEVCAssembler.h"

#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <inttypes.h>

#include <cutils/properties.h>

#define ATRACE_TAG ATRACE_TAG_VIDEO
#include <utils/Trace.h>

namespace imsma
{

//static const uint32_t kSourceID = 0xdeadbeef;
//ToDo: whether all trackIndex change to uint32_t
RTPSource::RTPSource(
    uint32_t srcId,
    rtp_rtcp_config_t* pConfigPram ,int32_t iTrackIndex,
    sp<AMessage> &notify)
{

    ALOGI("%s,ssrc(0x%x),track(%d)",__FUNCTION__,srcId,iTrackIndex);
    mID = srcId;
    mHighestSeqNumber = 0;
    mFirstPacketSeqNum = 0;
    //mIssueFIRRequests(false),
    //mLastFIRRequestUs(-1),
    //mNextFIRSeqNo((rand() * 256.0) / RAND_MAX),
    mNotify = notify->dup();
    mHighestSeqNumberSet = false;
    //mConfigParam = *pConfigPram;// ToDo: whether can assign struct this way
    mWaitingTMMBN = false;


    mAdaInfo = new RxAdaptationInfo();

    mTrackIndex = iTrackIndex;

    if(IMSMA_RTP_VIDEO == pConfigPram->media_type) {
        if(IMSMA_RTP_VIDEO_H264 == pConfigPram->mime_Type) {
            mAssembler = new AVCAssembler(notify);
        } else if(IMSMA_RTP_VIDEO_HEVC == pConfigPram->mime_Type) {
            mAssembler = new HEVCAssembler(notify);
        }
    }

    mClockRate = pConfigPram->sample_rate;
    //mRRintervalUs = 5000000l;
    //ToDo:need set mRRintervalUs according pConfigParam

    mFirstPacketRecvTimeUs = 0;

    struct sockaddr_in addr;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    mCName.setTo("android@");
    mCName.append(inet_ntoa(addr.sin_addr));


    /*****adaptation related parameters*******/
    mAdaInfo->updateInfo(pConfigPram->rtp_packet_bandwidth, pConfigPram->network_info.MBR_DL, pConfigPram->sample_rate);

    mSupportTMMBR = false;

    for(uint8_t i = 0; i< pConfigPram->rtcp_fb_param_num; i++) {
        uint16_t fb_id = (pConfigPram->rtcp_fb_type[i]).rtcp_fb_id;
        uint16_t fb_param = (pConfigPram->rtcp_fb_type[i]).rtcp_fb_param;

        if((fb_id == IMSMA_CCM) && (fb_param == IMSMA_TMMBR)) {
            mSupportTMMBR = true;
            break;
        }
    }

    ALOGD("%s,mAS =%d kbps,mMBR_DL=%d kbps,mSupportTMMBR=%d",\
          __FUNCTION__,pConfigPram->rtp_packet_bandwidth,pConfigPram->network_info.MBR_DL,mSupportTMMBR);


}
RTPSource::~RTPSource()
{
    ALOGI("%s",__FUNCTION__);

    delete mAdaInfo;
}
static uint32_t AbsDiff(uint32_t seq1, uint32_t seq2)
{
    return seq1 > seq2 ? seq1 - seq2 : seq2 - seq1;
}

void RTPSource::processRTPPacket(const sp<ABuffer> &buffer)
{
    ALOGV("%s",__FUNCTION__);

    if(queuePacket(buffer) && mAssembler != NULL) {
        mAssembler->onPacketReceived(this);
    }
}

/*
void RTPSource::timeUpdate(uint32_t rtpTime, uint64_t ntpTime) {
    mLastNTPTime = ntpTime;
    mLastNTPTimeUpdateUs = ALooper::GetNowUs();

    sp<AMessage> notify = mNotify->dup();
    notify->setInt32("time-update", true);
    notify->setInt32("rtp-time", rtpTime);
    notify->setInt64("ntp-time", ntpTime);
    notify->post();
}
*/

bool RTPSource::GetdebugInfo(bool needNotify, int32_t *uiEncBitRate, uint32_t Operator)
{

    RTPDebugNotifyInfo NotifyInfo[10];
    memset(NotifyInfo, 0, sizeof(RTPDebugNotifyInfo)*10);

    int count = 10;
    bool isTrigger = false;
    uint32_t lostcount = getLostCount();
    uint32_t IDamageCount = getIDamageCount();
    bool ret = mAdaInfo->GetdebugInfo(needNotify, uiEncBitRate, lostcount, IDamageCount, NotifyInfo, &count, &isTrigger, Operator);

    if(ret == true) {
        ALOGD("%s NotifyInfo size=%d isTrigger=%d",__FUNCTION__, count, isTrigger);

        if(isTrigger == true) {
            NotifyTMMBR();
        }

        if(count < 0 || count > 10) {
            ALOGE("error: GetdebugInfo notify msg count %d", count);
        } else {
            for(int i = 0; i < count; i++) {
                switch(NotifyInfo[i].kWhatInfo) {
                case kWhatNoRTP: {
                    ALOGD("%s kWhatNoRTP info=%d",__FUNCTION__, NotifyInfo[i].item1);
                    sp<AMessage> notify = mNotify->dup();
                    notify->setInt32("what",kWhatNoRTP);
                    notify->setInt32("info",NotifyInfo[i].item1);
                    notify->post();
                    break;
                }

            case kWhatDropCall: {
                ALOGD("%s kWhatDropCall ",__FUNCTION__);
                sp<AMessage> notify = mNotify->dup();
                notify->setInt32("what",kWhatDropCall);
                notify->post();
                break;
            }

                case kWhatUpdateDebugInfo: {
                    ALOGD("%s kWhatUpdateDebugInfo info=%d",__FUNCTION__, NotifyInfo[i].item1);
                    sp<AMessage> notify = mNotify->dup();
                    notify->setInt32("what",kWhatUpdateDebugInfo);
                    notify->setInt32("info",NotifyInfo[i].item1);
                    notify->setInt32("needNotify", (needNotify == true ? 1 : 0));
                    notify->post();
                    break;
                }

                default:
                    ALOGE("%s invaild msg %x",__FUNCTION__, NotifyInfo[i].kWhatInfo);
                    break;
                }
            }
        }
    }

    return ret;
}

bool RTPSource::queuePacket(const sp<ABuffer> &buffer)
{
    uint32_t orig_seqNum = (uint32_t) buffer->int32Data();

    updateStatisticInfo(buffer);
    mAdaInfo->setReceivePacketFlag();

    if(mAdaInfo->getFrameCount() == 0) {
        mFirstPacketRecvTimeUs = ALooper::GetNowUs();

        if(!mHighestSeqNumberSet) {
            mHighestSeqNumber = orig_seqNum;
        }

        mFirstPacketSeqNum = orig_seqNum;

        mAdaInfo->selfIncFrameCount();
        mQueue.push_back(buffer);
        ALOGI("%s,first recv packet seqNum:%u",__FUNCTION__,orig_seqNum);
        return false;
    }

    mAdaInfo->selfIncFrameCount();

    uint32_t seqNum = extendSeqNumber(orig_seqNum, mHighestSeqNumber);

    if(seqNum > mHighestSeqNumber) {
        mHighestSeqNumber = seqNum;
    }

    buffer->setInt32Data(seqNum);

    ATRACE_INT64("RTR:Src:queExtSeqN", (int64_t) seqNum);

    List<sp<ABuffer> >::iterator it = mQueue.begin();

    while(it != mQueue.end() && (uint32_t)(*it)->int32Data() < seqNum) {
        ++it;
    }

    if(it != mQueue.end() && (uint32_t)(*it)->int32Data() == seqNum) {
        ALOGW("Discarding duplicate buffer");
        return false;
    }

    mQueue.insert(it, buffer);
    /*ALOGD("%s,SeqNum(orig:%d,extended:%d),jitter buf size(%d)",\
        __FUNCTION__,orig_seqNum,seqNum,mQueue.size());*/

    //ToDo:
    //wait for 50ms for the right first seqNum
    //need tune on real network to adjust this timeout value
    //or change to mNumBuffersReceived < threshold
    int64_t iPacketRecvTimeUs = ALooper::GetNowUs();

    if((iPacketRecvTimeUs - mFirstPacketRecvTimeUs) < 50000) {
        if(seqNum < mFirstPacketSeqNum) {
            mFirstPacketSeqNum = seqNum;
        }

        ALOGD("waiting(%" PRId64 " us) for the least seq:%ud",\
              iPacketRecvTimeUs - mFirstPacketRecvTimeUs,mFirstPacketSeqNum);
        return false;
    }

    return true;
}

void RTPSource::updateStatisticInfo(const sp<ABuffer> buffer)
{
    sp<AMessage> meta = buffer->meta();

    int32_t iRtpTime = 0;
    meta->findInt32("rtp-time", &iRtpTime);

    calculateArrivalJitter(buffer);

    int32_t iRtpOH = 0;
    int32_t iRtpSize = 0;
    meta->findInt32("rtp_size",&iRtpSize);
    meta->findInt32("rtp_overhead",&iRtpOH);

    uint32_t lostcount = getLostCount();
    uint32_t uiNetSize = buffer->size();
    bool isTrigger = mAdaInfo->updateStatisticInfo(iRtpOH, iRtpSize, uiNetSize, lostcount, iRtpTime);

    if(isTrigger == true) {
        NotifyTMMBR();
    }
}

void RTPSource::reset()
{
    peerResumedSendStream();
    flushQueue();
}

status_t RTPSource::peerPausedSendStream()
{
    ALOGI("%s",__FUNCTION__);
    //ToDo: cancel check whether alive

    mAdaInfo->peerPausedSendStream();

    return OK;
}
status_t RTPSource::peerResumedSendStream()
{
    ALOGI("%s",__FUNCTION__);

    mHighestSeqNumberSet = 0;

    if(mAssembler != NULL) {
        mAssembler->reset();
    }

    mWaitingTMMBN = false;

    mAdaInfo->peerResumedSendStream();

    //ToDo: re-start check whether alive
    return OK;
}

//void RTPSource::byeReceived() {
//    mAssembler->onByeReceived();
//}

status_t RTPSource::processSenderInfo(const sp<ABuffer> &buffer)
{
    ALOGI("%s",__FUNCTION__);

    if(!buffer.get() || (buffer->size() < 20)) {
        ALOGE("%s,buffer is not valid",__FUNCTION__);
        return UNKNOWN_ERROR;
    }

    uint8_t *data = buffer->data();

    uint64_t ntpTime = 0;
    uint32_t rtpTime = 0;

    mAdaInfo->processSenderInfo(data, &ntpTime, &rtpTime);

    ALOGI("XXX timeUpdate: ssrc=0x%08x, rtpTime %u tick == ntpTime %.6f s",
          mID,
          rtpTime,
          (ntpTime >> 32) + (double)(ntpTime & 0xffffffff) / (1ll << 32));

    //notify timeupdate to RTPReceiver
    sp<AMessage> meta = buffer->meta();
    int32_t sr_generation = 0;
    meta->findInt32("sr_generation",&sr_generation);

    sp<AMessage> notify = mNotify->dup();
    notify->setInt32("what",kWhatTimeUpdate);
    notify->setInt32("rtpTime",rtpTime);
    notify->setInt64("ntpTime",ntpTime);
    notify->setInt32("sr_generation",sr_generation);
    notify->post();

    return OK;
}

status_t RTPSource::addReceiverReportBlock(const sp<ABuffer> &buffer)
{
    //ToDo, check whether the state, if hold on no packets received

    if(buffer->size() + 24 > buffer->capacity()) {
        ALOGW("RTCP buffer too small to accomodate RR.");
        return UNKNOWN_ERROR;
    }

    bool isTrigger = false;
    uint8_t *data = buffer->data() + buffer->size();
    int ret = mAdaInfo->addReceiverReportBlock(mID, data, mHighestSeqNumber, mFirstPacketSeqNum, mClockRate, &isTrigger);

    if(ret != 0) {
        return UNKNOWN_ERROR;
    }

    if(isTrigger == true) {
        NotifyTMMBR();
    }

    buffer->setRange(buffer->offset(), buffer->size() + 24);
    return OK;
}

//ToDo: wrong value will happen if rtptime overflow
void RTPSource::calculateArrivalJitter(const sp<ABuffer> &buffer)
{
    //calculate interarrival jitter
    int32_t iArrivalJitter = 0;

    uint32_t uiRtpTimeStamp = 0;
    sp<AMessage> meta_pack = buffer->meta();
    CHECK(meta_pack->findInt32("rtp-time", (int32_t *) &uiRtpTimeStamp));

    int64_t iPacketRecvTimeUs = ALooper::GetNowUs();
    meta_pack->setInt64("recv-time",iPacketRecvTimeUs);

    mAdaInfo->calculateArrivalJitter(uiRtpTimeStamp, iPacketRecvTimeUs, mClockRate);

    return;
}
//ToDo: can not handle if overflow and discorder happen simultaneously
uint32_t RTPSource::extendSeqNumber(uint32_t seqNum, uint32_t mHighestSeqNumber)
{
    uint32_t seq1 = seqNum | (mHighestSeqNumber & 0xffff0000);
    uint32_t seq2 = seqNum | ((mHighestSeqNumber & 0xffff0000) + 0x10000);
    uint32_t seq3 = seqNum | ((mHighestSeqNumber & 0xffff0000) - 0x10000);
    uint32_t diff1 = AbsDiff(seq1, mHighestSeqNumber);
    uint32_t diff2 = AbsDiff(seq2, mHighestSeqNumber);
    uint32_t diff3 = AbsDiff(seq3, mHighestSeqNumber);

    if(diff1 < diff2) {
        if(diff1 < diff3) {
            // diff1 < diff2 ^ diff1 < diff3
            seqNum = seq1;
        } else {
            // diff3 <= diff1 < diff2
            seqNum = seq3;
        }
    } else if(diff2 < diff3) {
        // diff2 <= diff1 ^ diff2 < diff3
        seqNum = seq2;
    } else {
        // diff3 <= diff2 <= diff1
        seqNum = seq3;
    }

    return seqNum;
}

//ToDo
#if 0
void ARTPSource::updateExpectedTimeoutUs(const int32_t& samples)
{
    int64_t duration = samples * 1000000LL / mClockRate;
    return updateExpectedTimeoutUs(duration);
}

void ARTPSource::updateExpectedTimeoutUs(const int64_t& duration)
{
    if(duration < kAccessUnitTimeoutUs)
        return;

    int64_t timeUs = ALooper::GetNowUs() + duration;

    if(timeUs > mExpectedTimeoutUs)
        mExpectedTimeoutUs = timeUs;
}
#endif

void RTPSource::flushQueue()
{
    mQueue.clear();

    if(mAssembler != NULL) {
        mAssembler->flushQueue();
    }
}
void RTPSource::start()
{

    mAdaInfo->resetParamOnstart();

    mHighestSeqNumberSet = false;

    flushQueue();

    if(mAssembler != NULL) {
        mAssembler->reset();
    }

    mWaitingTMMBN = false;
}
void RTPSource::stop()
{
    //mStarted = false;

    //avoid re-use the SR before stop(such as hold on, one-way video)
    //peer may reset rtp time to increase from another random value after hold back
    //so we can't re-use the old SR
    //but can make use of the SR receiving earlier than start
    //so we should not reset this param when start
    //mLastSRntp = 0;

    mAdaInfo->resetParamOnstop();

    flushQueue();
    mWaitingTMMBN = false;
}

void RTPSource::NotifyTMMBR()
{
    if(!mSupportTMMBR) {
        ALOGW("%s,not support TMMBR",__FUNCTION__);
        return;
    }

    if(mWaitingTMMBN) {
        return;
    }

    bool isReduce = true;
    sp<ABuffer> tmmbr_fci = new ABuffer(4);
    uint8_t* data = tmmbr_fci->data();

    if(OK != mAdaInfo->fillTMMBRbuffer(data, &isReduce)) {
        ALOGW("%s,fillTMMBRbuffer fail !",__FUNCTION__);
        return;
    }

    sp<AMessage> notify = mNotify->dup();
    notify->setInt32("what",kWhatSendTMMBR);
    notify->setBuffer("tmmbr_fci",tmmbr_fci);
    notify->setInt32("isReduce",isReduce);
    notify->post();

    mWaitingTMMBN = true;

}

void RTPSource::processTMMBN(sp<ABuffer> tmmbn_fci)
{
    ALOGD("%s ++",__FUNCTION__);

    if(!mWaitingTMMBN) {
        ALOGW("%s,not waiting TMMBN",__FUNCTION__);
        //ToDo, if peer send TMMBN actively
        //whether need more operation
    }

    uint8_t* data = tmmbn_fci->data();

    mAdaInfo->processTMMBN(data);
    mWaitingTMMBN = false;
}

void RTPSource::updateConfigParams(rtp_rtcp_config_t* pConfigPram)
{
    ALOGD("%s",__FUNCTION__);
    mClockRate = pConfigPram->sample_rate;

    mSupportTMMBR = false;

    for(uint8_t i = 0; i < (pConfigPram->rtcp_fb_param_num); i++) {
        uint16_t fb_id = (pConfigPram->rtcp_fb_type[i]).rtcp_fb_id;
        uint16_t fb_param = (pConfigPram->rtcp_fb_type[i]).rtcp_fb_param;

        if((fb_id == IMSMA_CCM) && (fb_param == IMSMA_TMMBR)) {
            mSupportTMMBR = true;
            break;
        }
    }

    bool isTrigger = mAdaInfo->updateInfo(pConfigPram->rtp_packet_bandwidth, pConfigPram->network_info.MBR_DL, pConfigPram->sample_rate);

    if(isTrigger == true) {
        NotifyTMMBR();
    }

    ALOGD("%s,mSupportTMMBR=%d",__FUNCTION__,mSupportTMMBR);
}


const sp<ABuffer> RTPSource::getNewTMMBRInfo()
{
    sp<ABuffer> tmmbr_fci = new ABuffer(4);
    uint8_t* data = tmmbr_fci->data();

    bool isReduce = true;

    if(0 != mAdaInfo->fillTMMBRbuffer(data, &isReduce)) {
        ALOGE("%s,fill tmmbr fail",__FUNCTION__);
        return NULL;
    }

    //ToDo: maybe no use
    sp<AMessage> tmmbr_meta = tmmbr_fci->meta();
    tmmbr_meta->setInt32("isReduce",isReduce);

    return tmmbr_fci;
}

uint32_t RTPSource::getLostCount()
{
    if(mAssembler != NULL) {
        return mAssembler->getLostCount();
    } else {
        ALOGD("getLostCount 0 mAssembler is NULL");
        return 0;
    }
}

uint32_t RTPSource::getIDamageCount()
{
    if(mAssembler != NULL) {
        return mAssembler->getIDamageCount();
    } else {
        ALOGD("getIDamageCount 0 mAssembler is NULL");
        return 0;
    }
}

void RTPSource::EstimateTSDelay(int64_t mediaTimeUs)
{
    mAdaInfo->EstimateTSDelay(mediaTimeUs);
}

bool RTPSource::pollingCheckTSDelay()
{
    return mAdaInfo->pollingCheckTSDelay();
}

void RTPSource::clearTSDelayInfo()
{
    return mAdaInfo->clearTSDelayInfo();
}

bool RTPSource::isCSD(const sp<ABuffer>& accessUnit) {
    bool ret = false;
    if(mAssembler != NULL) {
        ret = mAssembler->isCSD(accessUnit);
    }
    return ret;
}

}  // namespace android


