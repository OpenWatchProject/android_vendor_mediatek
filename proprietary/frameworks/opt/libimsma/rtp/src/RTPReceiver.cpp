/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
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
*  RTPReceiver.cpp
*
* Project:
* --------
*   Android
*
* Description:
* ------------
*   Receiver RTP packet
*
* Author:
* -------
*   Haizhen.Wang(mtk80691)
*
****************************************************************************/

//#define LOG_NDEBUG 0
#define LOG_TAG "[VT][RTP]RTPReceiver"
#include <utils/Log.h>

#include "RTPController.h"
#include "RTPAssembler.h"

#include "RTPReceiver.h"
#include <media/stagefright/foundation/avc_utils.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <inttypes.h>
#include <cutils/properties.h>

#include "IVcodecCap.h"
#include "VcodecCap.h"

#define ATRACE_TAG ATRACE_TAG_VIDEO
#include <utils/Trace.h>

using namespace android;
using android::status_t;

namespace imsma
{
static const int64_t kMaxUint32bitValue = 0x0100000000LL;

//Need Check: whether need to release the memory for mpVideoCapParams,mpAudioCapParams

RTPReceiver::RTPReceiver(sp<AMessage> notify, uint32_t simID,uint32_t operatorID)
{
    ALOGI("%s",__FUNCTION__);

    if(!notify.get()) {
        ALOGE("notify is NULL");
    }

    //ToDo,need dup mNotify before use mNotify
    mNotify = notify;
    //ToDo: need make this interval can config by property

    //mTrackNotify = new AMessage(kWhatTrackNotify,id());
    mLastCVOinfo = 0;

    mLastSeqN = 0;
    mSimID = simID;
    mOperatorID = operatorID;

    mVideoRTPPending = false;
    mMsgDebugEnable = false;
    mSsrcInOtherQueue = 0;
    mPreviousSsrc = 0;

    mVideoPathDelayUs = 50000;//50ms, the delay from RTP to Render, need tune

    char videoPathDelay[PROPERTY_VALUE_MAX];
    memset(videoPathDelay,0,sizeof(videoPathDelay));
    int _ivideo_path_delay = 0;

    if(property_get("vendor.vt.dl_Adp.video_path_delay", videoPathDelay, NULL)) {
        _ivideo_path_delay = atoi(videoPathDelay);

        if(_ivideo_path_delay >= 0) {
            mVideoPathDelayUs = (int64_t) _ivideo_path_delay;
            ALOGD("%s,estimate video path delay =%" PRId64" us",__FUNCTION__,mVideoPathDelayUs);
        }
    }
}
RTPReceiver::~RTPReceiver()
{
    ALOGI("%s",__FUNCTION__);
}
status_t RTPReceiver::addStream(rtp_rtcp_config_t*pRTPNegotiatedParams,sp<SocketWrapper> socketWrapper,sp<AMessage> accuNotify,int32_t trackIndex)
{
    sp<AMessage> msg = new AMessage(kWhatAddStream, this);
    msg->setPointer("param", pRTPNegotiatedParams);
    msg->setObject("socketWrapper",socketWrapper);
    msg->setMessage("accuNotify",accuNotify);
    msg->setInt32("trackIndex",trackIndex);

    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);

    return err;
}

status_t RTPReceiver::removeStream(int32_t trackIndex)
{
    sp<AMessage> msg = new AMessage(kWhatRemoveStream, this);
    msg->setInt32("trackIndex",trackIndex);

    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);

    return err;
}


status_t RTPReceiver::updateConfigParams(rtp_rtcp_config_t* pRTPNegotiatedParams,int32_t trackIndex)
{
    ALOGD("%s,track %d save negotiated done params %p",__FUNCTION__,trackIndex, pRTPNegotiatedParams);

    sp<AMessage> msg = new AMessage(kWhatUpdateConfigParams, this);
    msg->setPointer("param", pRTPNegotiatedParams);
    msg->setInt32("trackIndex",trackIndex);
    msg->post();

    return OK;
}

status_t RTPReceiver::start(int32_t trackIndex)
{
    ALOGI("%s,track(%d)",__FUNCTION__,trackIndex);

    //ToDo, AMessage must create after registerHandler ,
    //because before registerHandler the id() will be 0, invalid id
    //mTrackNotify = new AMessage(kWhatTrackNotify,id());

    //Mutex::Autolock autoLock(mLock);
    sp<AMessage> msg = new AMessage(kWhatStart, this);
    msg->setInt32("trackIndex",trackIndex);
    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);

    //for debug
    if(mMsgDebugEnable == false) {
        sp<AMessage> msg_debug = new AMessage(kWhatDebugInfo, this);
        msg_debug->post();
        mMsgDebugEnable = true;
    }

    return err;

}
/*
status_t RTPReceiver::pause(int32_t trackIndex){
    ALOGI("%s,track(%d)",__FUNCTION__,trackIndex);
    //ToDo: implement pause--onPause
    sp<AMessage> msg = new AMessage(kWhatPause, this);
    msg->setInt32("trackIndex",trackIndex);
    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);

    return err;


}

status_t RTPReceiver::resume(int32_t trackIndex){
    ALOGI("%s,track(%d)",__FUNCTION__,trackIndex);
    //ToDo: implement resume--onResume
    sp<AMessage> msg = new AMessage(kWhatResume, this);
    msg->setInt32("trackIndex",trackIndex);
    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);

    return err;
}
*/
status_t RTPReceiver::stop(int32_t trackIndex)
{
    ALOGI("%s,track(%d)",__FUNCTION__,trackIndex);
    //ToDo: implement stop--onStop
    sp<AMessage> msg = new AMessage(kWhatStop, this);
    msg->setInt32("trackIndex",trackIndex);
    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);

    return err;


}
bool RTPReceiver::isActive(uint8_t trackIndex)
{
    ALOGI("%s",__FUNCTION__);

    sp<AMessage> msg = new AMessage(kWhatIsActive, this);
    msg->setInt32("trackIndex",trackIndex);
    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);

    if(err != OK) {
        ALOGE("%s,return err(%d)",__FUNCTION__,err);
        return false;
    }

    int32_t isActive = false;
    response->findInt32("isActive",&isActive);

    return isActive;
}

uint8_t RTPReceiver::addReceiveReportBlocks(const sp<ABuffer> &buffer, uint8_t trackIndex)
{
    ALOGD("%s,track(%d) ++",__FUNCTION__,trackIndex);
    //ToDo: implement pause--onPause
    sp<AMessage> msg = new AMessage(kWhatAddReportBlock, this);
    msg->setInt32("trackIndex",trackIndex);
    msg->setBuffer("buffer",buffer);

    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);
    uint8_t RC = 0;
    int32_t iRC = 0;
    response->findInt32("rc",&iRC);
    RC = (uint8_t) iRC;
    ALOGI("%s,track(%d) --,RC(%d)",__FUNCTION__,trackIndex,RC);
    return RC;
}

#if 0
status_t RTPReceiver::getSrcSSRC(uint32_t* ssrc,uint8_t trackIndex)
{
    ALOGD("%s,++",__FUNCTION__);
    sp<AMessage> msg = new AMessage(kWhatGetSSRC, this);
    msg->setInt32("trackIndex",trackIndex);

    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);
    uint32_t ui_ssrc = 0;
    response->findInt32("ssrc", (int32_t*) &ui_ssrc);
    *ssrc = ui_ssrc;
    ALOGI("%s,-- track(%d),ssrc(%d)",__FUNCTION__,trackIndex,ui_ssrc);
    return err;
}
#endif
status_t RTPReceiver::peerPausedSendStream(uint8_t trackIndex)
{
    ALOGD("%s",__FUNCTION__);
    sp<AMessage> msg = new AMessage(kWhatPeerPauseStream, this);
    msg->setInt32("trackIndex",trackIndex);
    msg->post();
    return OK;

}

status_t RTPReceiver::peerResumedSendStream(uint8_t trackIndex)
{
    ALOGD("%s",__FUNCTION__);
    sp<AMessage> msg = new AMessage(kWhatPeerResumeStream, this);
    msg->setInt32("trackIndex",trackIndex);
    msg->post();
    return OK;
}

static uint16_t u16at(const uint8_t *data)
{
    return data[0] << 8 | data[1];
}

static uint32_t u32at(const uint8_t *data)
{
    return u16at(data) << 16 | u16at(&data[2]);
}

static uint64_t u64at(const uint8_t *data)
{
    return (uint64_t)(u32at(data)) << 32 | u32at(&data[4]);
}

int  RTPReceiver::videoRTPPacketCallBack(void* cookie,const sp<ABuffer>& buffer)
{
    ALOGV("%s",__FUNCTION__);

    RTPReceiver* rtpRecv = static_cast<RTPReceiver*>(cookie);

    if(rtpRecv == NULL) {
        ALOGW("%s,cookie = NULL",__FUNCTION__);
        return UNKNOWN_ERROR;
    }

    Mutex::Autolock autoLock(rtpRecv->mVideoRTPQueueLock);
    Vector<sp<ABuffer> > *videoRTPQueue = rtpRecv->video_queue();
    videoRTPQueue->push_back(buffer);

    if(rtpRecv->mVideoRTPPending) {
        return OK;
    }

    sp<AMessage> msg = new AMessage(kWhatRTPPacket, rtpRecv);
    msg->setInt32("trackIndex",IMSMA_RTP_VIDEO);
    msg->post();
    rtpRecv->mVideoRTPPending = true;
    return OK;

}


status_t RTPReceiver::processSenderInfo(const sp<ABuffer> &buffer,uint32_t uSSRC,uint8_t trackIndex)
{
    sp<AMessage> msg = new AMessage(kWhatProcessSR, this);
    msg->setBuffer("buffer",buffer);
    msg->setInt32("ssrc",uSSRC);
    msg->setInt32("trackIndex",trackIndex);
    msg->post();
    return OK;

}

status_t RTPReceiver::postTimeUpdate(uint32_t rtpTime, uint64_t ntpTime, int32_t trackIndex)
{
    sp<AMessage> msg = new AMessage(kWhatTimeUpdate,this);
    msg->setInt32("trackIndex",trackIndex);
    msg->setInt32("rtpTime",rtpTime);
    msg->setInt64("ntpTime",ntpTime);
    msg->post();
    return OK;
}

status_t RTPReceiver::processTMMBN(sp<ABuffer> tmmbn_fci)
{
    sp<AMessage> msg = new AMessage(kWhatProcessTMMBN, this);
    msg->setBuffer("buffer",tmmbn_fci);
    msg->post();
    return OK;
}

const sp<ABuffer> RTPReceiver::getNewTMMBRInfo()
{
    ALOGD("%s,++",__FUNCTION__);
    sp<AMessage> msg = new AMessage(kWhatGetNewTMMBRInfo, this);

    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);
    sp<ABuffer> tmmbr_fci;
    response->findBuffer("tmmbr_fci",&tmmbr_fci);

    ALOGI("%s,--",__FUNCTION__);
    return tmmbr_fci;
}


void RTPReceiver::onMessageReceived(const sp<AMessage> &msg)
{
    //ALOGD("%s,what(0x%x)",__FUNCTION__,msg->what());
    ATRACE_CALL();

    switch(msg->what()) {
    case kWhatStart: {
        int32_t trackIndex = IMSMA_RTP_VIDEO;
        msg->findInt32("trackIndex",&trackIndex);
        ALOGD("kWhatStart,track(%d)",trackIndex);

        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));
        status_t err = onStart(trackIndex);
        sp<AMessage> response = new AMessage;
        response->setInt32("err", err);

        response->postReply(replyID);
        break;
    }
    case kWhatStop: {
        int32_t trackIndex = IMSMA_RTP_VIDEO;
        msg->findInt32("trackIndex",&trackIndex);
        ALOGD("kWhatStop,track(%d)",trackIndex);

        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));


        status_t err = onStop(trackIndex);

        sp<AMessage> response = new AMessage;
        response->setInt32("err", err);

        response->postReply(replyID);
        break;
    }
    case kWhatIsActive: {
        int32_t trackIndex = IMSMA_RTP_VIDEO;
        msg->findInt32("trackIndex",&trackIndex);
        ALOGD("kWhatIsActive,track(%d)",trackIndex);

        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));

        sp<AMessage> response = new AMessage;

        Vector<sp<TrackInfo> >::iterator i = mpTrackInfos.begin();
        sp<TrackInfo> pTrack;

        while(i != mpTrackInfos.end()) {
            if((*i)->mTrackIndex == trackIndex) {
                pTrack = *i;
                ALOGD("kWhatIsActive,track(%d))",trackIndex);
                break;
            }

            i++;
        }

        if(i == mpTrackInfos.end()) {
            ALOGW("kWhatIsActive,track(%d) not added",trackIndex);
            response->setInt32("err", UNKNOWN_ERROR);
            response->setInt32("isActive",false);
            response->postReply(replyID);
            break;
        }

        response->setInt32("err", OK);
        response->setInt32("isActive",pTrack->mStarted);

        response->postReply(replyID);
        break;
    }

    case kWhatDebugInfo: {
        int32_t uiEncBitRate = 0;

        Vector<sp<TrackInfo> >::iterator i = mpTrackInfos.begin();
        sp<TrackInfo> pTrack;

        while(i != mpTrackInfos.end()) {
            pTrack = *i;

            if((pTrack->mRTPSource).get()) {
                //statistics framerate <--> rtp timestamp express
                bool needNotify = pTrack->mRTPSource->pollingCheckTSDelay();

                pTrack->mRTPSource->GetdebugInfo(needNotify, &uiEncBitRate, mOperatorID);
            }

            //i++;
            break;
        }

        //maybe real net no need
        /*if(needNotify == true && uiEncBitRate != 0){
            ALOGD("delta: we need nodify enc bitrate to %d", uiEncBitRate*9/10);
            sp<AMessage> notify = mNotify->dup();
            notify->setInt32("what",kWhatAdjustEncBitRate);
            notify->setInt32("netBitRate",uiEncBitRate*9/10);
            notify->post();
        }*/


        //ALOGD("kWhatDebugInfo,timeIntervel=%lld",2000000);
        msg->post(1000000);    //1s

        break;
    }

    /*
    case kWhatPause:
    {
        int32_t trackIndex = IMSMA_RTP_VIDEO;
        msg->findInt32("trackIndex",&trackIndex);
        ALOGD("kWhatStop,track(%d)",trackIndex);

        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));


        status_t err = onPause(trackIndex);

        sp<AMessage> response = new AMessage;
        response->setInt32("err", err);

        response->postReply(replyID);
        break;
    }
    case kWhatResume:
    {
        int32_t trackIndex = IMSMA_RTP_VIDEO;
        msg->findInt32("trackIndex",&trackIndex);
        ALOGD("kWhatStop,track(%d)",trackIndex);

        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));


        status_t err = onResume(trackIndex);

        sp<AMessage> response = new AMessage;
        response->setInt32("err", err);

        response->postReply(replyID);
        break;
    }
    */
    case kWhatAddStream: {
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));

        rtp_rtcp_config_t*pRTPNegotiatedParams = NULL;
        msg->findPointer("param", (void**) &pRTPNegotiatedParams);

        sp<SocketWrapper> socketWrapper;
        msg->findObject("socketWrapper", (sp<RefBase> *) &socketWrapper);

        sp<AMessage> accuNotify;
        msg->findMessage("accuNotify",&accuNotify);

        int32_t trackIndex = IMSMA_RTP_VIDEO;
        msg->findInt32("trackIndex",&trackIndex);

        status_t err = onAddStream(pRTPNegotiatedParams,socketWrapper,accuNotify,trackIndex);

        sp<AMessage> response = new AMessage;
        response->setInt32("err", err);

        response->postReply(replyID);

        break;
    }
    case kWhatRemoveStream: {
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));


        int32_t trackIndex = IMSMA_RTP_VIDEO;
        msg->findInt32("trackIndex",&trackIndex);

        status_t err = onRemoveStream(trackIndex);

        sp<AMessage> response = new AMessage;
        response->setInt32("err", err);

        response->postReply(replyID);

        break;
    }
    case kWhatUpdateConfigParams: {
        rtp_rtcp_config_t*pRTPNegotiatedParams = NULL;
        msg->findPointer("param", (void**) &pRTPNegotiatedParams);

        int32_t trackIndex = IMSMA_RTP_VIDEO;
        msg->findInt32("trackIndex",&trackIndex);

        onUpdateConfigParams(pRTPNegotiatedParams,trackIndex);

        break;
    }
    case kWhatPeerPauseStream: {
        int32_t trackIndex = IMSMA_RTP_VIDEO;
        msg->findInt32("trackIndex",&trackIndex);
        onPeerPausedSendStream(trackIndex);
        break;
    }
    case kWhatPeerResumeStream: {
        int32_t trackIndex = IMSMA_RTP_VIDEO;
        msg->findInt32("trackIndex",&trackIndex);
        onPeerResumedSendStream(trackIndex);
        break;
    }
    case kWhatRTPPacket: {
        int32_t trackIndex = IMSMA_RTP_VIDEO;
        msg->findInt32("trackIndex",&trackIndex);

        sp<ABuffer> packet;

        if(IMSMA_RTP_VIDEO == trackIndex) {
            Mutex::Autolock autoLock(mVideoRTPQueueLock);
            size_t size =  mVideoRTPQueue.size();
            //ALOGD("kWhatRTPPacket,video RTP queue size = %d",size);

            while(!mVideoRTPQueue.empty()) {
                packet = *mVideoRTPQueue.begin();
                mVideoRTPQueue.erase(mVideoRTPQueue.begin());

                mVideoRTPPending = false;

                uint32_t newSsrc = 0;

                queueRTPPacket(packet,trackIndex, &newSsrc);

                if(newSsrc != 0) {
                    changeSSRC(packet,trackIndex, newSsrc);
                }
            }
        }

        break;
    }

    case kWhatTrackNotify: {
        int32_t trackIndex = IMSMA_RTP_VIDEO;
        msg->findInt32("trackIndex",&trackIndex);
        ALOGV("kWhatTrackNotify,trackIndex(%d),trackInfos num(%zu)",trackIndex,mpTrackInfos.size());

        sp<TrackInfo> pTrack;
        Vector<sp<TrackInfo> >::iterator i = mpTrackInfos.begin();

        while(i != mpTrackInfos.end()) {
            if((*i)->mTrackIndex == trackIndex) {
                ALOGV("kWhatTrackNotify,find track(%d) in TrackInfos",trackIndex);
                pTrack = *i;
                break;
            }

            i++;
        }

        if(i == mpTrackInfos.end()) {
            ALOGE("kWhatTrackNotify,track %d not found",trackIndex);
            return;
        }

        sp<AMessage> accuNotify;
        accuNotify = (pTrack->mAccuNotify)->dup();


        int32_t what = 0;
        msg->findInt32("what",&what);

        if(what == RTPSource::kWhatTimeUpdate) {
            int32_t sr_generation = 0;
            msg->findInt32("sr_generation",&sr_generation);

            if(sr_generation != pTrack->mSRGeneration) {
                ALOGI("RTPSource::kWhatTimeUpdate,timeout should drop");
                return;
            }

            if(pTrack->mIsFirstAccu) {
                ALOGI("RTPSource::kWhatTimeUpdate,not accu receive, not update time");
                return;
            }


            uint32_t rtpTime = 0;
            uint64_t ntpTime = 0;
            msg->findInt32("rtpTime", (int32_t*) &rtpTime);
            msg->findInt64("ntpTime", (int64_t*) &ntpTime);
            int64_t ntpTimeUs = (int64_t)(ntpTime * 1E6 / (1ll << 32));
            //int64_t ntpTimeUs = (ntpTime >> 32)*1E6 + (double)(ntpTime & 0xffffffff) * 1E6 / (1ll << 32);
            //int64_t ntpTimeUsTemp = (int64_t)(((ntpTime >> 32) + (double)(ntpTime & 0xffffffff) / (1ll << 32))*1E6);

            ALOGI("%s,kWhatTimeUpdate track(%d)(rtpTime:%u,ntpTime:%" PRId64 "us",\
                  __FUNCTION__,trackIndex,rtpTime,ntpTimeUs);  //,ntpTimeUsTemp);

            if(pTrack->mTimeMapped) {
                //update the rtp->ntp mapping
                //check whether rtp time in SR overflow
                if(rtpTime < pTrack->mRtpTimeAnchor) {
                    if((pTrack->mRtpTimeAnchor - rtpTime) > 0x40000000) {
                        pTrack->mSRRtpTimeCycles++;
                        ALOGI("%s,kWhatTimeUpdate,rtp time overflow times( %d )( %d -> %d )",__FUNCTION__,\
                              pTrack->mSRRtpTimeCycles,pTrack->mRtpTimeAnchor,rtpTime);
                    }
                }

            }

            pTrack->mExtenedRtpTimeAnchor = (int64_t) rtpTime + kMaxUint32bitValue * (int64_t) pTrack->mSRRtpTimeCycles;
            pTrack->mNtpTimeAnchor = ntpTimeUs;
            pTrack->mRtpTimeAnchor = rtpTime;
            pTrack->mTimeMapped = true;
            pTrack->mLastNTPTimeUpdateUs = ALooper::GetNowUs();

            ALOGI("kWhatTimeUpdate:track(%d),sys_time(%" PRId64 "us)",\
                  trackIndex,pTrack->mLastNTPTimeUpdateUs);

            ALOGI("kWhatTimeUpdate:mRtpTimeAnchor:%u tick,mExtenedRtpTimeAnchor:%" PRId64 " tick,mNtpTimeAnchor:%" PRId64 "us",\
                  pTrack->mRtpTimeAnchor,pTrack->mExtenedRtpTimeAnchor,pTrack->mNtpTimeAnchor);

            break;
        } else if(what == RTPSource::kWhatSendTMMBR) {
            sp<ABuffer> tmmbr_fci;
            msg->findBuffer("tmmbr_fci",&tmmbr_fci);
            int32_t isReduce = true;
            msg->findInt32("isReduce",&isReduce);

            sp<AMessage> notify = mNotify->dup();
            notify->setInt32("what",kWhatSendTMMBR);
            notify->setBuffer("tmmbr_fci",tmmbr_fci);
            notify->setInt32("isReduce",isReduce);
            notify->post();
        } else if(what == RTPSource::kWhatUpdateDebugInfo) {
            int32_t info = true;
            msg->findInt32("info",&info);
            int32_t needNotify = 0;
            msg->findInt32("needNotify",&needNotify);

            sp<AMessage> notify = mNotify->dup();
            notify->setInt32("what",kWhatUpdateDebugInfo);
            notify->setInt32("info",info);
            notify->setInt32("needNotify",needNotify);
            notify->post();
        } else if(what == RTPSource::kWhatDropCall) {
            sp<AMessage> notify = mNotify->dup();
            notify->setInt32("what",kWhatDropCall);
            notify->post();
        } else if(what == RTPSource::kWhatNoRTP) {
            int32_t info = 0;
            msg->findInt32("info",&info);

            sp<AMessage> notify = mNotify->dup();
            notify->setInt32("what",kWhatNoRTP);
            notify->setInt32("info",info);
            notify->post();
        } else if(what == RTPAssembler::kWhatAccessUnit) {

            if(!pTrack->mStarted) {
                break;
            }

            //ToDo: rtp->ntp mapping
            // Whether need maintain one accu queue,
            //queue accu to accu queue
            //1, if rtp->ntp mapping has not create, then queue the accu to queue
            //2, need queue accu to Low Water Mark to accommodate network jitter
            //3, release one accu after has received the response of decoder
            //    we can evaluate the decoding speed according the reponses and the accumulated accu queue
            //3GVT no queue--maybe different with 4G VT, 3GVT is circuit switch with fix bandwidth
            //but 4GVT is packet switch without guarantee network
            //but the other hand: VT is real time application... so first we didn't queue the accu
            sp<ABuffer> accu;

            if((trackIndex == IMSMA_RTP_VIDEO) && msg->findBuffer("access-unit", &accu) && accu.get()) {
                //ToDo: maybe we can check mAccuOwnByDecList status before notify new accu
                // maybe can discard some new accu with importance =0
                sp<AMessage> accu_meta =  accu->meta();
                uint32_t rtp_time = 0;
                //ToDo: how to handle uint and int assign
                accu_meta->findInt32("rtp-time", (int32_t*) &rtp_time);

                int32_t early_seq = 0;
                accu_meta->findInt32("FirstPacket_token", (int32_t*) &early_seq);

                int32_t end_seq = 0;
                accu_meta->findInt32("latestPacekt_token", (int32_t*) &end_seq);

                int32_t damagedAu= 0;
                accu_meta->findInt32("damaged",&damagedAu);

                ALOGV("EarliestPacket_token=%d latestPacekt_token=%d damagedAu=%d", early_seq, end_seq, damagedAu);

                if(pTrack->mLastDamageFlag == false && pTrack->mLastEndSeq != -1 && (pTrack->mLastEndSeq + 1 != early_seq)) {
                    if(pTrack->mLastEndSeq != early_seq) {
                        accu_meta->setInt32("lostfull", 1);
                        ALOGD("mLastDamageFlag=%d mLastEndSeq=%d early_seq=%d",
                              pTrack->mLastDamageFlag, pTrack->mLastEndSeq, early_seq);
                    }
                }

                if(damagedAu) {
                    pTrack->mLastDamageFlag = true;
                } else {
                    pTrack->mLastDamageFlag = false;
                }

                pTrack->mLastEndSeq = end_seq;

                uint32_t timeScale = pTrack->mConfigParam.sample_rate;

                if(pTrack->mIsFirstAccu) {
                    //uint32_t rtp_time = 0;
                    pTrack->mFirstAccuRtpTime = rtp_time;
                    pTrack->mLastAccuRtpTime = rtp_time;
                    ALOGI("receive first accu,rtp-time = %d",rtp_time);
                    pTrack->mIsFirstAccu = false;

                    if((pTrack->mRTPSource).get()) {
                      bool isCsd = pTrack->mRTPSource->isCSD(accu);
                      if(isCsd == false){
                         ALOGD("first not csd info, we need send FIR");
                         sp<AMessage> msg = mNotify->dup();
                         msg->setInt32("trackIndex",trackIndex);
                         msg->setInt32("what",kWhatTriggerFir);
                         msg->post();
                      }
                    }
                }

                int64_t iExtenedRtpTime = 0;

                if(rtp_time < pTrack->mLastAccuRtpTime) {
                    if(pTrack->mLastAccuRtpTime - rtp_time > 0x40000000) {
                        pTrack->mRtpTimeCycles++;
                        ALOGI("vido track notify,rtp time overflow times(%d)( %d -> %d )",\
                              pTrack->mRtpTimeCycles,pTrack->mLastAccuRtpTime,rtp_time);
                    }
                }

                iExtenedRtpTime = (int64_t) rtp_time + kMaxUint32bitValue * (int64_t) pTrack->mRtpTimeCycles;
                //int64_t mediaTimeUs = (int64_t)(rtp_time - (*i)->mFirstAccuRtpTime) * 1000000l / timeScale;
                int64_t mediaTimeUs = (iExtenedRtpTime - (int64_t) pTrack->mFirstAccuRtpTime) * 1000000l / timeScale;

                accu_meta->setInt64("timeUs", mediaTimeUs);    //mediaTime calcuated from rtp time

                //statistics framerate <--> rtp timestamp express
                if((pTrack->mRTPSource).get()) {
                    pTrack->mRTPSource->EstimateTSDelay(mediaTimeUs);
                }

                //ToDo: if multislice case, we should find the frame boundary
                if(!(*i)->mTimeMapped) {
                    ALOGV("video track notify, rtp<->ntp mapping not established  accu timeUs(rtpTime) (%" PRId64 "us),rtp(%u tick),extRtp(%" PRId64 "tick),cycle(%d)",\
                          mediaTimeUs,rtp_time,iExtenedRtpTime,pTrack->mRtpTimeCycles);
                    accu_meta->setInt64("ntp-time",0);    //ToDo: check whether need revise to -1

                } else {
                    ALOGV("rtp<->ntp mapping has established");
                    int64_t ntp_time = 0;
                    ntp_time = (iExtenedRtpTime - pTrack->mExtenedRtpTimeAnchor) *1000000l / timeScale + pTrack->mNtpTimeAnchor;
                    accu_meta->setInt64("ntp-time",ntp_time);
                    ALOGV("video track notify, timeUs(ntpTime) (%" PRId64 "us) accu timeUs(rtpTime) (%" PRId64 "us),rtp(%u tick),extRtp(%" PRId64 "tick),cycle(%d)",\
                          ntp_time, mediaTimeUs,rtp_time,iExtenedRtpTime,pTrack->mRtpTimeCycles);

                }


                pTrack->mLastAccuRtpTime = rtp_time;

                accuNotify->setBuffer("access-unit",accu);
                accuNotify->post();

                int32_t iAccuCount = 0;
                accu_meta->findInt32("token",&iAccuCount);
                //ATRACE_INT("RTR:Recv:accu",iAccuCount);
                ATRACE_ASYNC_END("RTR-MAR",iAccuCount);
            } else {
                bool findAccu = msg->findBuffer("access-unit", &accu);
                ALOGE("Should not be here(track(%d),findAccu(%d))",trackIndex,findAccu);
            }
        } else if(what == RTPAssembler::kWhatPacketLost) {
            int32_t LostCount = 0;
            sp<ABuffer> TmBuf;

            //uint32_t packetLostOrigSeqNum = 0;
            msg->findInt32("lostcount",&LostCount);
            msg->findBuffer("lostpointer",&TmBuf);
            //msg->findInt32("orig_seqNum",&packetLostOrigSeqNum);
            ALOGI("assembler detect packet lost, Num:%d pli limit: 1", LostCount);

            //trigger pli
            if(LostCount >= 1){
                sp<AMessage> msg = mNotify->dup();
                msg->setInt32("trackIndex",trackIndex);
                msg->setInt32("what",kWhatTriggerPli);
                msg->post();
            }

            uint8_t *pTmBuf = TmBuf->base();

            for(int32_t i = 0; i < LostCount; i++) {
                ALOGI("kWhatPacketLost, count:%d seq=%d", i, ((uint32_t *) pTmBuf) [i]);

                pTrack->mLostPacketSeqNumList.push_back(((uint32_t *) pTmBuf) [i]);

                for(List<uint32_t>::iterator j = (pTrack->mLostPacketSeqNumList).begin(); j != (pTrack->mLostPacketSeqNumList).end();) {
                    if((((uint32_t *) pTmBuf) [i] - (*j)) > 16) {
                        j = (pTrack->mLostPacketSeqNumList).erase(j);
                    } else {
                        break;
                    }
                }
            }


            uint8_t list_size = (pTrack->mLostPacketSeqNumList).size();

            //not feedback nack too frequent,
            //only many continues packet lost,we trigger nack
            if(list_size >= 8) {
                ALOGI("more than 8 continues packet lost");
                sp<ABuffer> nack_fci = new ABuffer(4);
                memset(nack_fci->data(),0,nack_fci->size());

                uint8_t *data = nack_fci->data();
                List<uint32_t>::iterator it = (pTrack->mLostPacketSeqNumList).begin();
                uint32_t u32firstSeqNum =  *it;
                uint16_t u16firstSeqNum = (uint16_t) u32firstSeqNum;

                data[0] = (u16firstSeqNum >> 8) & 0x00FF;
                data[1] = u16firstSeqNum & 0x00FF;

                ALOGD("packet lost: first seqN = %d", u16firstSeqNum);

                (pTrack->mLostPacketSeqNumList).erase(it);

                //uint16_t* blp_data =(uint16_t*)&data[2];
                uint16_t blp_data = 0;

                for(List<uint32_t>::iterator i = (pTrack->mLostPacketSeqNumList).begin(); i != (pTrack->mLostPacketSeqNumList).end(); i++) {
                    uint8_t seqNumDelta = (*i) - u32firstSeqNum;

                    if(seqNumDelta >= 1) {
                        blp_data |= (0x0001 << (seqNumDelta - 1));
                    }
                }

                data[2] = (blp_data >> 8) & 0xff;
                data[3] = blp_data & 0xff;

                ALOGI("following packet lost seqNums:0x%x",blp_data);
                (pTrack->mLostPacketSeqNumList).clear();

                sp<AMessage> msg = mNotify->dup();
                msg->setInt32("trackIndex",trackIndex);
                msg->setInt32("what",kWhatGenericNACK);
                msg->setBuffer("nack_fci",nack_fci);
                msg->post();
            }
        }

        break;
    }

    case kWhatProcessSR: {
        sp<ABuffer> buffer;
        msg->findBuffer("buffer",&buffer);

        uint32_t uSSRC = 0;
        msg->findInt32("ssrc", (int32_t*) &uSSRC);

        int32_t trackIndex = IMSMA_RTP_VIDEO;
        msg->findInt32("trackIndex",&trackIndex);

        onProcessSenderInfo(buffer,uSSRC,trackIndex);
        break;
    }
    case kWhatProcessTMMBN: {
        int32_t trackIndex = IMSMA_RTP_VIDEO;
        sp<TrackInfo> pTrack;
        Vector<sp<TrackInfo> >::iterator i = mpTrackInfos.begin();

        while(i != mpTrackInfos.end()) {
            if((*i)->mTrackIndex == trackIndex) {
                pTrack = *i;
                break;
            }

            i++;
        }

        if(i == mpTrackInfos.end()) {
            ALOGE("kWhatProcessTMMBN,no related track");
            break;
        }

        sp<ABuffer> tmmbn_fci;
        msg->findBuffer("buffer",&tmmbn_fci);

        if(!tmmbn_fci.get()) {
            ALOGE("kWhatProcessTMMBN,tmmbn_fci is null");
            break;
        }

        int32_t ssrc = 0;
        (tmmbn_fci->meta())->findInt32("ssrc_media",&ssrc);

        //if(pTrack->mSSRCid == (uint32_t)ssrc){
        if((pTrack->mRTPSource).get()) {
            (pTrack->mRTPSource)->processTMMBN(tmmbn_fci);
        }

        //}

        break;
    }
    case kWhatGetNewTMMBRInfo: {
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));

        sp<ABuffer> tmmbr_fci;

        int32_t trackIndex = IMSMA_RTP_VIDEO;
        sp<TrackInfo> pTrack;
        Vector<sp<TrackInfo> >::iterator i = mpTrackInfos.begin();

        while(i != mpTrackInfos.end()) {
            if((*i)->mTrackIndex == trackIndex) {
                pTrack = *i;
                break;
            }

            i++;
        }

        if(pTrack.get() && (pTrack->mRTPSource).get()) {
            tmmbr_fci  = (pTrack->mRTPSource)->getNewTMMBRInfo();
        }

        sp<AMessage> response = new AMessage;
        response->setBuffer("tmmbr_fci",tmmbr_fci);
        response->setInt32("err", tmmbr_fci.get() ? OK : UNKNOWN_ERROR);
        response->postReply(replyID);

        break;
    }
    case kWhatAddReportBlock: {

        int32_t trackIndex = IMSMA_RTP_VIDEO;
        msg->findInt32("trackIndex",&trackIndex);
        ALOGD("kWhatAddReportBlock,track(%d)",trackIndex);

        sp<ABuffer> buffer;
        msg->findBuffer("buffer",&buffer);

        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));

        uint8_t RC = onAddReceiveReportBlocks(buffer,trackIndex);

        sp<AMessage> response = new AMessage;
        response->setInt32("err", OK);
        response->setInt32("rc",RC);
        response->postReply(replyID);

        break;
    }
#if 0
    case kWhatGetSSRC: {
        int32_t trackIndex = IMSMA_RTP_VIDEO;
        msg->findInt32("trackIndex",&trackIndex);

        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));

        status_t err = OK;
        uint32_t ssrc = 0;
        err = onGetSrcSSRC(&ssrc,trackIndex);

        sp<AMessage> response = new AMessage;
        response->setInt32("err", err);
        response->setInt32("ssrc",ssrc);
        response->postReply(replyID);

        break;
    }
#endif
#if 0
    case kWhatHoldOn: {
        uint32_t replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));

        int32_t isHoldOnEnable = 0;
        msg->findInt32("isHoldOn",&isHoldOnEnable);
        status_t err = onHoldOn(isHoldOnEnable);

        sp<AMessage> response = new AMessage;
        response->setInt32("err", err);

        response->postReply(replyID);
        break;
    }
#endif
    default: {
        ALOGE("Should not be here,(%d)",msg->what());
        break;
    }
    }

}


status_t RTPReceiver::onAddStream(rtp_rtcp_config_t*pRTPNegotiatedParams,sp<SocketWrapper> socketWrapper,sp<AMessage> accuNotify,int32_t trackIndex)
{
    //ToDo--need change to message
    ALOGI("%s,track(%d)",__FUNCTION__,trackIndex);
    sp<TrackInfo> pTrack = new TrackInfo;
    pTrack->mTrackIndex = trackIndex;
    //pTrack->mConfigParam = *pRTPNegotiatedParams;//ToDo: can we assign struct this way
    copyConfigParams(pRTPNegotiatedParams,& (pTrack->mConfigParam));
    pTrack->mRTPSource = NULL;
    pTrack->mSSRCset = false;
    pTrack->mSSRCid = 0;
    pTrack->mTimeMapped = false;
    pTrack->mIsFirstAccu = true;
    pTrack->mSocketWrapper = socketWrapper;
    pTrack->mAccuNotify = accuNotify;
    mpTrackInfos.push(pTrack);

    return OK;
}
status_t RTPReceiver::copyConfigParams(rtp_rtcp_config_t* srcParams,rtp_rtcp_config_t* dstParams)
{
    ALOGI("%s",__FUNCTION__);

    dstParams->media_type = srcParams->media_type;
    dstParams->mime_Type = srcParams->mime_Type;
    dstParams->rtp_payload_type = srcParams->rtp_payload_type;
    dstParams->rtp_profile = srcParams->rtp_profile;
    dstParams->sample_rate = srcParams->sample_rate;

    dstParams->rtp_packet_bandwidth = srcParams->rtp_packet_bandwidth;
    dstParams->packetize_mode = srcParams->packetize_mode;

    dstParams->rtp_header_extension_num = srcParams->rtp_header_extension_num;

    for(uint32_t j = 0; j < srcParams->rtp_header_extension_num; j++) {
        ((dstParams->rtp_ext_map) [j]).extension_id = ((srcParams->rtp_ext_map) [j]).extension_id;
        ((dstParams->rtp_ext_map) [j]).direction = ((srcParams->rtp_ext_map) [j]).direction;
        memcpy(((dstParams->rtp_ext_map) [j]).extension_uri, ((srcParams->rtp_ext_map) [j]).extension_uri,\
               sizeof(((srcParams->rtp_ext_map) [j]).extension_uri));
    }

    dstParams->rtcp_sender_bandwidth = srcParams->rtcp_sender_bandwidth;
    dstParams->rtcp_receiver_bandwidth = srcParams->rtcp_receiver_bandwidth;

    dstParams->rtcp_reduce_size = srcParams->rtcp_reduce_size;
    dstParams->rtcp_fb_param_num = srcParams->rtcp_fb_param_num;

    for(uint8_t i = 0; i< dstParams->rtcp_fb_param_num; i++) {
        (dstParams->rtcp_fb_type) [i].rtcp_fb_id= srcParams->rtcp_fb_type[i].rtcp_fb_id;
        (dstParams->rtcp_fb_type) [i].rtcp_fb_param = srcParams->rtcp_fb_type[i].rtcp_fb_param;

        //Need check, whether struct can assign data this way
        memcpy((dstParams->rtcp_fb_type) [i].rtcp_fb_sub_param,srcParams->rtcp_fb_type[i].rtcp_fb_sub_param,\
               sizeof(((srcParams->rtcp_fb_type) [i]).rtcp_fb_sub_param));
    }

    //get network info
    (dstParams->network_info).ebi = (srcParams->network_info).ebi;
    (dstParams->network_info).interface_type = (srcParams->network_info).interface_type;
    (dstParams->network_info).dscp= (srcParams->network_info).dscp;
    (dstParams->network_info).soc_priority= (srcParams->network_info).soc_priority;

    (dstParams->network_info).remote_rtp_port = (srcParams->network_info).remote_rtp_port;
    (dstParams->network_info).remote_rtcp_port = (srcParams->network_info).remote_rtcp_port;
    (dstParams->network_info).local_rtp_port = (srcParams->network_info).local_rtp_port;
    (dstParams->network_info).local_rtcp_port   = (srcParams->network_info).local_rtcp_port;

    memcpy((dstParams->network_info).remote_rtp_address, (srcParams->network_info).remote_rtp_address,\
           sizeof((srcParams->network_info).remote_rtp_address));
    memcpy((dstParams->network_info).remote_rtcp_address, (srcParams->network_info).remote_rtcp_address,\
           sizeof((srcParams->network_info).remote_rtcp_address));

    memcpy((dstParams->network_info).local_rtp_address, (srcParams->network_info).local_rtp_address,\
           sizeof((srcParams->network_info).local_rtp_address));

    memcpy((dstParams->network_info).local_rtcp_address, (srcParams->network_info).local_rtcp_address,\
           sizeof((srcParams->network_info).local_rtcp_address));

    (dstParams->network_info).remote_addr_type  = (srcParams->network_info).remote_addr_type;
    (dstParams->network_info).local_addr_type   = (srcParams->network_info).local_addr_type;

    (dstParams->network_info).network_id    = (srcParams->network_info).network_id;
    (dstParams->network_info).rtp_direction = (srcParams->network_info).rtp_direction;

    memcpy((dstParams->network_info).socket_fds, (srcParams->network_info).socket_fds,\
           sizeof((srcParams->network_info).socket_fds));

    (dstParams->network_info).MBR_DL    = (srcParams->network_info).MBR_DL;
    (dstParams->network_info).MBR_UL    = (srcParams->network_info).MBR_UL;

    return OK;
}

status_t RTPReceiver::onUpdateConfigParams(rtp_rtcp_config_t* pRTPNegotiatedParams,int32_t trackIndex)
{
    ALOGI("%s,param(%p)track(%d)",__FUNCTION__, pRTPNegotiatedParams,trackIndex);

    sp<TrackInfo> pTrack;
    Vector<sp<TrackInfo> >::iterator i = mpTrackInfos.begin();

    while(i != mpTrackInfos.end()) {
        if((*i)->mTrackIndex == trackIndex) {
            ALOGV("kWhatTrackNotify,find track(%d) in TrackInfos",trackIndex);
            pTrack = *i;
            break;
        }

        i++;
    }

    if(i == mpTrackInfos.end()) {
        ALOGE("kWhatTrackNotify,track %d not found",trackIndex);
        return UNKNOWN_ERROR;
    }

    copyConfigParams(pRTPNegotiatedParams,& (pTrack->mConfigParam));

    //no need update to RTPSource
    //RTPSource will not use the params except mime_type
    //and RTPSource not support update mime during ViLTE now

    //need update to RTPSource for adaptation
    if((pTrack->mRTPSource).get()) {
        (pTrack->mRTPSource)->updateConfigParams(pRTPNegotiatedParams);
    }

    return OK;
}

status_t RTPReceiver::onStart(int32_t trackIndex)
{
    //ToDo:
    //each call start,need re-random the first rtp time


    //define the send interval dpending on the mConfigParam.rtp_packet_bandwidth
    //each start need re-calculate the send rate depending on the new bandwidth

    //calculte the bytes can sending in each interval
    //uint32_t uiBytesSendEachInterval = 0;
    //uiBytesSendEachInterval = mConfigParam.rtp_packet_bandwidth * mSendIntervalMs;


    //sp<AMessage> msg = new AMessage(kWhatSendRTPPacket,id());
    //ToDo: check whether can assign unit32 to int64
    //ToDo: for send evenly
    //msg->setInt64("bytesNeedSend",uiBytesSendEachInterval);

    //msg->post();
    //start post send RTP packet AMessage


    //find the related trackInfo
    Vector<sp<TrackInfo> >::iterator i = mpTrackInfos.begin();
    sp<TrackInfo> pTrack;

    while(i != mpTrackInfos.end()) {
        if((*i)->mTrackIndex == trackIndex) {
            pTrack = *i;
            ALOGD("%s,track(%d))",__FUNCTION__,trackIndex);
            break;
        }

        i++;
    }

    if(i == mpTrackInfos.end()) {
        ALOGW("%s,track(%d) not added",__FUNCTION__,trackIndex);
        return UNKNOWN_ERROR;
    }

    if((pTrack->mRTPSource).get()) {
        pTrack->mRTPSource->start();
    }

    pTrack->mStarted = true;

    ALOGD("sim=%d op id %d,name = %s", mSimID ,mOperatorID, getOperatorName(mOperatorID));

    //before listen new RTP packet,clear the old RTP packets
    {
        Mutex::Autolock autoLock(mVideoRTPQueueLock);
        mVideoRTPQueue.clear();
        mVideoOtherSSRCQueue.clear();
        mSsrcInOtherQueue = 0;
        mPreviousSsrc = 0;
    }
    //start listen packet from socketWrapper
    sp<SocketWrapper> socketWrapper = pTrack->mSocketWrapper;

    if(trackIndex == IMSMA_RTP_VIDEO) {
        socketWrapper->setRxCallBack(this,videoRTPPacketCallBack);
    }

    ALOGD("%s,track(%d)",__FUNCTION__,trackIndex);
    return OK;
}


status_t RTPReceiver::onStop(int32_t trackIndex)
{
    //ToDo:
    //each call start,need re-random the first rtp time


    //define the send interval dpending on the mConfigParam.rtp_packet_bandwidth
    //each start need re-calculate the send rate depending on the new bandwidth

    //calculte the bytes can sending in each interval
    //uint32_t uiBytesSendEachInterval = 0;
    //uiBytesSendEachInterval = mConfigParam.rtp_packet_bandwidth * mSendIntervalMs;


    //sp<AMessage> msg = new AMessage(kWhatSendRTPPacket,id());
    //ToDo: check whether can assign unit32 to int64
    //ToDo: for send evenly
    //msg->setInt64("bytesNeedSend",uiBytesSendEachInterval);

    //msg->post();
    //start post send RTP packet AMessage

    //find the related trackInfo
    Vector<sp<TrackInfo> >::iterator i = mpTrackInfos.begin();
    sp<TrackInfo> pTrack;

    while(i != mpTrackInfos.end()) {
        if((*i)->mTrackIndex == trackIndex) {
            pTrack = *i;
            ALOGD("%s,track(%d))",__FUNCTION__,trackIndex);
            break;
        }

        i++;
    }

    if(i == mpTrackInfos.end()) {
        ALOGW("%s,track(%d) not added",__FUNCTION__,trackIndex);
        return UNKNOWN_ERROR;
    }

    if((pTrack->mRTPSource).get()) {
        pTrack->mRTPSource->stop();
        pTrack->mRTPSource->clearTSDelayInfo();
    }

    //make all the SRs still in looper message queue and received before stop invalid
    //but make sure not ignore the new SR after stop
    //this for we should drop the SR before hold on
    //but the new valid SR maybe received before start called
    //so we shouldn't increase this param when start
    pTrack->mSRGeneration++;

    //avoid re-use the SR before stop(such as hold on, one-way video)
    //peer may reset rtp time to increase from another random value after hold back
    //so we can't re-use the old SR
    //but can make use of the SR receiving earlier than start
    //so we should not reset this param when start
    pTrack->mTimeMapped = false;
    pTrack->mExtenedRtpTimeAnchor = -1;
    pTrack->mNtpTimeAnchor = -1;
    pTrack->mLastNTPTimeUpdateUs = 0;

    //RTP related params
    pTrack->mIsFirstAccu = true;
    pTrack->mRtpTimeCycles = 0;

    pTrack->mLostPacketSeqNumList.clear();
    pTrack->mSRRtpTimeCycles = 0;

    //stop listen rtp packet
    sp<SocketWrapper> socketWrapper = pTrack->mSocketWrapper;
    socketWrapper->setRxCallBack(0,0);

    //after setRxCallback, videoRTPPacketCallBack will not be called
    //mVideoRTPQueue will not be changed
    {
        Mutex::Autolock autoLock(mVideoRTPQueueLock);
        mVideoRTPQueue.clear();
        mVideoOtherSSRCQueue.clear();
        mSsrcInOtherQueue = 0;
        mPreviousSsrc = 0;
    }

    pTrack->mStarted = false;

    ALOGD("%s,track(%d)",__FUNCTION__,trackIndex);
    return OK;
}
/*
status_t RTPReceiver::onPause(int32_t trackIndex){

    //find the related trackInfo
    Vector<sp<TrackInfo> >::iterator i = mpTrackInfos.begin();
    sp<TrackInfo> pTrack;
    while(i != mpTrackInfos.end()){
        if((*i)->mTrackIndex == trackIndex){
            pTrack = *i;
            ALOGD("%s,track(%d))",__FUNCTION__,trackIndex);
            break;
        }
        i++;
    }
    if(i == mpTrackInfos.end()){
        ALOGW("%s,track(%d) not added",__FUNCTION__,trackIndex);
        return UNKNOWN_ERROR;
    }
    pTrack->mStarted = false;

    sp<RTPSource> source = pTrack->mRTPSource;
    source->flushQueue();
    ALOGD("%s,track(%d)",__FUNCTION__,trackIndex);

    return OK;
}
status_t RTPReceiver::onResume(int32_t trackIndex){
    //find the related trackInfo
    Vector<sp<TrackInfo> >::iterator i = mpTrackInfos.begin();
    sp<TrackInfo> pTrack;
    while(i != mpTrackInfos.end()){
        if((*i)->mTrackIndex == trackIndex){
            pTrack = *i;
            ALOGD("%s,track(%d))",__FUNCTION__,trackIndex);
            break;
        }
        i++;
    }
    if(i == mpTrackInfos.end()){
        ALOGW("%s,track(%d) not added",__FUNCTION__,trackIndex);
        return UNKNOWN_ERROR;
    }
    pTrack->mStarted = true;
    ALOGD("%s,track(%d)",__FUNCTION__,trackIndex);

    return OK;
}
*/
status_t RTPReceiver::onRemoveStream(int32_t trackIndex)
{
    //ToDo--need change to message
    //Mutex::Autolock autoLock(mLock);
    ALOGI("%s,track(%d)",__FUNCTION__,trackIndex);
    Vector<sp<TrackInfo> >::iterator i = mpTrackInfos.begin();

    while(i != mpTrackInfos.end()) {
        if((*i)->mTrackIndex == trackIndex) {
            ALOGD("%s,track(%d))",__FUNCTION__,trackIndex);
            mpTrackInfos.erase(i);
            break;
        }

        i++;
    }

    if(i == mpTrackInfos.end()) {
        ALOGE("%s,not found track(%d)",__FUNCTION__,trackIndex);
        return UNKNOWN_ERROR;
    }

    return OK;
}

status_t RTPReceiver::changeSSRC(sp<ABuffer> &packet,int32_t trackIndex, uint32_t newSsrc)
{
    if(newSsrc == mPreviousSsrc) {
        ALOGD("%s,PreviousSsrc ignore track(%d),newssrc(%d)",__FUNCTION__, trackIndex, newSsrc);
        return OK;
    }

    if(!mVideoOtherSSRCQueue.empty() && (mSsrcInOtherQueue != newSsrc)) {
        ALOGD("%s,3th mSsrcInOtherQueue 0x%x, newssrc(%d)",__FUNCTION__, mSsrcInOtherQueue, newSsrc);
        mVideoOtherSSRCQueue.clear();
    }

    mSsrcInOtherQueue = newSsrc;
    mVideoOtherSSRCQueue.push_back(packet);

    if(mVideoOtherSSRCQueue.size() >= 10) {
        ALOGD("%s,now we change ssrc 0x%x > 0x%x",__FUNCTION__, mPreviousSsrc, mSsrcInOtherQueue);
        sp<TrackInfo> pTrack;
        Vector<sp<TrackInfo> >::iterator i = mpTrackInfos.begin();

        while(i != mpTrackInfos.end()) {
            if((*i)->mTrackIndex == trackIndex) {
                pTrack = (*i);
                break;
            }

            i++;
        }

        if(i == mpTrackInfos.end()) {
            ALOGE("%s,not find pTrack",__FUNCTION__);
            return -1;
        }

        if(pTrack.get() && !(pTrack->mStarted)) {
            ALOGD("%s,track(%d) isn't start",__FUNCTION__,trackIndex);
            return OK;
        }

        mLastSeqN = 0;

        sp<RTPSource> source;
        source = pTrack->mRTPSource;

        //reset RTPsource
        source->reset();

        //set new ssrc
        source->setSsrc(mSsrcInOtherQueue);
        pTrack->mSSRCset = false;

        //reset TrackInfo
        pTrack->mTimeMapped = false;
        pTrack->mSRGeneration = 0;
        pTrack->mFirstAccuRtpTime = 0;
        pTrack->mLastAccuRtpTime = 0;
        pTrack->mRtpTimeCycles  = 0;

        pTrack->mLostPacketSeqNumList.clear();

        pTrack->mIsFirstAccu = true;
        pTrack->mRtpTimeAnchor = 0;
        pTrack->mExtenedRtpTimeAnchor = -1;
        pTrack->mNtpTimeAnchor = -1;
        pTrack->mLastNTPTimeUpdateUs = 0;
        pTrack->mSRRtpTimeCycles = 0;

        ALOGD("%s,reset CVO info",__FUNCTION__);
        mLastCVOinfo = 0;

        //clear TS-map info
        pTrack->mRTPSource->clearTSDelayInfo();

        //queue buffer
        sp<ABuffer> packetOtherQueue;

        while(!mVideoOtherSSRCQueue.empty()) {
            packetOtherQueue = *mVideoOtherSSRCQueue.begin();
            mVideoOtherSSRCQueue.erase(mVideoOtherSSRCQueue.begin());

            mVideoRTPPending = false;

            uint32_t newSsrc = 0;

            queueRTPPacket(packetOtherQueue,trackIndex, &newSsrc);
        }
    }

    return OK;
}

status_t RTPReceiver::queueRTPPacket(sp<ABuffer> &packet,int32_t trackIndex, uint32_t *newSsrc)
{
    ATRACE_CALL();
    *newSsrc = 0;

    /////start of "check whether packet is valid and find the offset for payload"
    if(!packet.get()) {
        ALOGE("%s,packet is NULL",__FUNCTION__);
        return -1;
    }

    if(packet->size() < 4) {
        ALOGE("%s,rtp packet too short(%zu)",__FUNCTION__,packet->size());
        return -1;
    }

    size_t size = packet->size();
    ALOGV("%s,track(%d),packet size(%zu)",__FUNCTION__,trackIndex,size);

    if(size < 12) {
        // Too short to be a valid RTP header.
        ALOGE("%s,packet size < 12 %zu",__FUNCTION__,size);
        return -1;
    }

    const uint8_t *data = packet->data();

    int32_t seqN = (int32_t) u16at(&data[2]);

    int64_t time_startUs = ALooper::GetNowUs();

    ALOGV("%s,track(%d),seqN(%d),size(%zu)",__FUNCTION__,trackIndex,seqN,size);

    if(seqN - mLastSeqN != 1) {
        ALOGD("%s,seq not serial track(%d),seqN(%d),mLastSeqN(%d)",__FUNCTION__,trackIndex,seqN,mLastSeqN);
    }

    mLastSeqN = seqN;

    //ATRACE_INT("RTR:Recv:queSeqN",seqN);
    ATRACE_ASYNC_BEGIN("RTR-MAR:SeqN",seqN);

    if((data[0] >> 6) != 2) {
        // Unsupported version.
        ALOGE("%s,unsupported version %d",__FUNCTION__,data[0]);
        return -1;
    }

    if(data[0] & 0x20) {
        // Padding present.
        size_t paddingLength = data[size - 1];

        if(paddingLength + 12 > size) {
            // If we removed this much padding we'd end up with something
            // that's too short to be a valid RTP header.
            ALOGE("%s,padding %zu %zu",__FUNCTION__,paddingLength, size);
            return -1;
        }

        size -= paddingLength;
    }

    int numCSRCs = data[0] & 0x0f;

    size_t payloadOffset = 12 + 4 * numCSRCs;

    if(size < payloadOffset) {
        // Not enough data to fit the basic header and all the CSRC entries.
        ALOGE("%s,offset %zu %zu",__FUNCTION__,payloadOffset, size);
        return -1;
    }

    uint8_t pt = data[1] & 0x7f;

    sp<TrackInfo> pTrack;
    //rtp_rtcp_capability * pConfigParam = NULL;
    Vector<sp<TrackInfo> >::iterator i = mpTrackInfos.begin();

    while(i != mpTrackInfos.end()) {
        if(((*i)->mTrackIndex == trackIndex) && ((*i)->mConfigParam.rtp_payload_type == pt)) {
            //pConfigParam = &(*i).mConfigParam;
            pTrack = (*i);
            break;
        }
        i++;
    }

    if(i == mpTrackInfos.end()) {
        ALOGE("%s,not expected pt %d",__FUNCTION__,pt);
        return -1;
    }

    if(pTrack.get() && !(pTrack->mStarted)) {
        ALOGD("%s,track(%d) isn't start",__FUNCTION__,trackIndex);
        return OK;
    }

    uint8_t rotation = 0;

    if(data[0] & 0x10) {
        // Header eXtension present.
        if(size < payloadOffset + 4) {
            // Not enough data to fit the basic header, all CSRC entries
            // and the first 4 bytes of the extension header.
            ALOGE("offset %zu %zu", payloadOffset, size);
            return -1;
        }

        const uint8_t *extensionData = &data[payloadOffset];

        size_t extensionLength =
            4 * (extensionData[2] << 8 | extensionData[3]);

        if(size < payloadOffset + 4 + extensionLength) {
            ALOGE("%s,extensionLength %zu %zu %zu",__FUNCTION__,extensionLength, payloadOffset, size);
            return -1;
        }

        //now only support one byte form, because cvo use one-byte form,
        //now onlu support arse cvo extension-->camera_facing,flip,rotation
        if((extensionData[0] == 0xBE) && (extensionData[1] == 0xDE)) {
            //one -byte header
            uint32_t offset = 0;
            const uint8_t* _data = &extensionData[4];

            while(offset < extensionLength) {
                //parse one extension
                uint8_t extension_id = (_data[offset] >> 4) & 0x0F;

                if(extension_id == 0 || extension_id == 15) {
                    break;
                }

                uint8_t extension_length = (_data[offset] & 0x0F) + 1;
                offset++;

                uint8_t extension_data[extension_length];
                memset(&extension_data,0,extension_length);
                memcpy(&extension_data,&_data[offset],extension_length);
                offset += extension_length;

                //find extetion name according extension id
                //ToDo:we know we support sendrecv for CVO
                //avoid peer set wrong direction capability, we will not filter cvo info as direction
                //for(uint32_t j = 0; j < pTrack->mConfigParam.rtp_header_extension_num; j++){
                //  if(extension_id == (pTrack->mConfigParam.rtp_ext_map[j]).extension_id){
                //      if(!strcmp("urn:3gpp:video-orientation",pTrack->mConfigParam.rtp_ext_map[j].extension_uri)){
                mLastCVOinfo = extension_data[0]; //only one byte for rotation

                //ALOGD("%s,CVO =0x%x",__FUNCTION__,mLastCVOinfo);
                //          break;
                //      }
                //    }
                // }
                //skip padding between extensions
                while(_data[offset] == 0) {
                    offset++;
                }
            }
        }

        //parse whether extension--rotation supported
        payloadOffset += 4 + extensionLength;
    }

    if(pt == 73) {
        ALOGW("playload type is 73(Reserved for RTCP conflict avoidance), not supported, ignore it");
        return OK;
    }

    /////end of "check whether packet is valid and find the offset for payload"

    uint32_t srcId = u32at(&data[8]);

    //ToDo: whether ssrc will change during VT or conference
    //How to handle this
    //in rtsp, the ssrc can get from "setup", may be sip can get this info?
    if(!(pTrack->mSSRCset)) {
        pTrack->mSSRCid = srcId;
        pTrack->mSSRCset = true;

        //notify ssrc to RTPController
        sp<AMessage> msg = mNotify->dup();
        msg->setInt32("trackIndex",trackIndex);
        msg->setInt32("what",kWhatUpdateSSRC);
        msg->setInt32("peer_ssrc",srcId);
        msg->post();

        if(pTrack->mRTPSource.get()) {
            pTrack->mRTPSource->setSsrc(srcId);
            ALOGI("%s,renew RTPSource ssrc=0x%x",__FUNCTION__,srcId);
        }

        ALOGI("%s,ssrc=0x%x",__FUNCTION__,srcId);
    } else if(srcId != pTrack->mSSRCid) {
        ALOGW("%s,Other ssrc %x, original %x",__FUNCTION__, srcId, pTrack->mSSRCid);
        *newSsrc = srcId;
        return -2;
    }

    sp<RTPSource> source;

    if(!pTrack->mRTPSource.get()) {
        sp<AMessage> trackNotify = new AMessage(kWhatTrackNotify,this);
        trackNotify->setInt32("trackIndex",pTrack->mTrackIndex);
        source = new RTPSource(srcId,& (pTrack->mConfigParam),pTrack->mTrackIndex,trackNotify);
        source->start();
        pTrack->mRTPSource = source;
    } else {
        source = pTrack->mRTPSource;
    }

    uint32_t rtpTime = u32at(&data[4]);

    sp<AMessage> meta = packet->meta();
    meta->setInt32("ssrc", srcId);
    meta->setInt32("rtp-time", rtpTime);    //ToDo: uint32_t set to int32,whill drop data?
    meta->setInt32("PT", pt);
    meta->setInt32("M", data[1] >> 7);

    if(trackIndex == IMSMA_RTP_VIDEO) {
        int32_t ccw_rotation = 0; //counter clockwise rotation
        int32_t camera_facing = IMSMA_CAMERA_FACING_UNKNOW;
        int32_t flip = IMSMA_CAMERA_NO_FLIP;

        if(mLastCVOinfo != 0) {
            uint8_t rotation = mLastCVOinfo & 0x03;

            switch(rotation) {
            case 0:
                ccw_rotation = 0;
                break;
            case 1:
                ccw_rotation = 90;
                break;
            case 2:
                ccw_rotation = 180;
                break;
            case 3:
                ccw_rotation = 270;
                break;
            }

            camera_facing = ((mLastCVOinfo & 0x08) == 0) ? 0 : 1;
            flip = mLastCVOinfo & 0x04;
        }

        meta->setInt32("ccw_rotation",ccw_rotation);
        meta->setInt32("camera_facing",camera_facing);
        meta->setInt32("flip",flip);
    }

    //int32_t seqNum = (int32_t)u16at(&data[2]);
    meta->setInt32("token",seqN);

    //for adaptation
    int32_t rtp_whole_size = packet->size();
    meta->setInt32("rtp_size",rtp_whole_size);
    meta->setInt32("rtp_overhead",rtp_whole_size- (size-payloadOffset));

    packet->setInt32Data(seqN);
    ALOGV("%s,payloadOffset:%zu",__FUNCTION__,payloadOffset);
    packet->setRange(packet->offset() + payloadOffset, size - payloadOffset);

    source->processRTPPacket(packet);

    int64_t time_endUs = ALooper::GetNowUs();

    ALOGV("%s -- ,track(%d),seqN(%d),consume time =%" PRId64 "Us",\
          __FUNCTION__,trackIndex,seqN,time_endUs-time_startUs);

    return OK;
}

status_t RTPReceiver::onProcessSenderInfo(const sp<ABuffer> &buffer,uint32_t uSSRC,uint8_t trackIndex)
{
    //find related track and RTPSource
    sp<TrackInfo> pTrack;
    //rtp_rtcp_capability * pConfigParam = NULL;
    Vector<sp<TrackInfo> >::iterator i = mpTrackInfos.begin();

    while(i != mpTrackInfos.end()) {
        if((*i)->mTrackIndex == trackIndex) {
            //pConfigParam = &(*i).mConfigParam;
            pTrack = (*i);
            break;
        }

        i++;
    }

    if(i == mpTrackInfos.end()) {
        ALOGE("%s,can't find track(%d)",__FUNCTION__,trackIndex);
        return -1;
    }

    //check the ssrc info,maybe sr receive early than the RTP packet
    //only one source for one track now
    if(!(pTrack->mSSRCset)) {
        pTrack->mSSRCid = uSSRC;
        ALOGI("%s,ssrc=0x%x",__FUNCTION__,uSSRC);
    } else if(uSSRC != pTrack->mSSRCid) {
        ALOGW("ignore invalid ssrc %x, expect %x", uSSRC, pTrack->mSSRCid);
        return OK;
    }

    sp<RTPSource> source;

    if(!pTrack->mRTPSource.get()) {
        sp<AMessage> trackNotify = new AMessage(kWhatTrackNotify,this);
        trackNotify->setInt32("trackIndex",pTrack->mTrackIndex);
        source = new RTPSource(uSSRC,& (pTrack->mConfigParam),pTrack->mTrackIndex,trackNotify);
        source->start();
        pTrack->mRTPSource = source;
    } else {
        source = pTrack->mRTPSource;
    }

    sp<AMessage> meta = buffer->meta();
    meta->setInt32("sr_generation",pTrack->mSRGeneration);
    return source->processSenderInfo(buffer);
}

#if 0
status_t RTPReceiver::onHoldOn(int32_t isHoldOnEnable)
{
    ALOGI("%s,holdOn(%d)",__FUNCTION__,isHoldOnEnable);

    if(isHoldOnEnable > 0) {
        //TODO:
        //stop receiver packet and stop notify accu
        //flush buffer queue

        //keeep RTCP sending

    } else {
        //ToDo:
        //re-rtp->ntp maping
        //rtp number re-calculate

        //resume receive packet and notify accu
    }

    return OK;
}
#endif

status_t RTPReceiver::onPeerPausedSendStream(uint8_t trackIndex)
{
    sp<TrackInfo> pTrack;
    //rtp_rtcp_capability * pConfigParam = NULL;
    Vector<sp<TrackInfo> >::iterator i = mpTrackInfos.begin();

    while(i != mpTrackInfos.end()) {
        if((*i)->mTrackIndex == trackIndex) {
            //pConfigParam = &(*i).mConfigParam;
            pTrack = (*i);
            break;
        }

        i++;
    }

    if(i == mpTrackInfos.end()) {
        ALOGE("%s,track %d not found in TrackInfos",__FUNCTION__,trackIndex);
        return -1;
    }

    sp<RTPSource> source;

    if(!pTrack->mRTPSource.get()) {
        ALOGE("%s,track %d related RTPSource is not found",__FUNCTION__,trackIndex);
    } else {
        source = pTrack->mRTPSource;
        source->peerPausedSendStream();
    }

    return OK;
}

status_t RTPReceiver::onPeerResumedSendStream(uint8_t trackIndex)
{
    sp<TrackInfo> pTrack;
    //rtp_rtcp_capability * pConfigParam = NULL;
    //ToDo: why need space after TrackInfo
    Vector<sp<TrackInfo> >::iterator i = mpTrackInfos.begin();

    while(i != mpTrackInfos.end()) {
        if((*i)->mTrackIndex == trackIndex) {
            //pConfigParam = &(*i)->mConfigParam;
            pTrack = (*i);
            break;
        }

        i++;
    }

    if(i == mpTrackInfos.end()) {
        ALOGE("%s,track %d not found in TrackInfos",__FUNCTION__,trackIndex);
        return -1;
    }

    ALOGD("%s,reset CVO info",__FUNCTION__);
    mLastCVOinfo = 0;

    sp<RTPSource> source;

    if(!(pTrack->mRTPSource).get()) {
        ALOGE("%s,track %d related RTPSource is not found",__FUNCTION__,trackIndex);
        //mTrackNotify->setInt32("trackIndex",pTrack->mTrackIndex);
        //source = new RTPSource(srcId,pTrack->mConfigParam,pTrack->mTrackIndex,mTrackNotify);
    } else {
        source = pTrack->mRTPSource;
        source->peerResumedSendStream();
        pTrack->mRTPSource->clearTSDelayInfo();
    }

    return OK;
}

#if 0
status_t RTPReceiver::onGetSrcSSRC(uint32_t* ssrc,uint8_t trackIndex)
{
//uint32_t RTPReceiver::getSrcSSRC(uint8_t trackIndex){
    *ssrc = 0;
    status_t err = OK;

    sp<TrackInfo> pTrack;
    //rtp_rtcp_capability * pConfigParam = NULL;
    Vector<sp<TrackInfo> >::iterator i = mpTrackInfos.begin();

    while(i != mpTrackInfos.end()) {
        if((*i)->mTrackIndex == trackIndex) {
            //pConfigParam = &(*i).mConfigParam;
            pTrack = (*i);
            break;
        }

        i++;
    }

    if(i == mpTrackInfos.end()) {
        ALOGE("%s,can't find track(%d)",__FUNCTION__,trackIndex);
        err = UNKNOWN_ERROR;
        return err;
    }

    if((pTrack.get()) && (pTrack->mSSRCset)) {
        *ssrc = pTrack->mSSRCid;
    } else {
        ALOGW("ssrc hasn't set");
        err = UNKNOWN_ERROR;
    }

    return err;
}
#endif
uint8_t RTPReceiver::onAddReceiveReportBlocks(const sp<ABuffer> &buffer, uint8_t trackIndex)
{
    ALOGI("%s,track(%d)",__FUNCTION__,trackIndex);
    uint8_t RC = 0;
    //find related RTPSource
    sp<TrackInfo> pTrack;
    //rtp_rtcp_capability * pConfigParam = NULL;
    Vector<sp<TrackInfo> >::iterator i = mpTrackInfos.begin();

    while(i != mpTrackInfos.end()) {
        if((*i)->mTrackIndex == trackIndex) {
            pTrack = (*i);
            break;
        }

        i++;
    }

    if(i == mpTrackInfos.end()) {
        ALOGE("%s,no packet received for track %d",__FUNCTION__,trackIndex);
        return RC;
    }

    //uint8_t* data = buffer->data() + buffer->size();

    //if there is multi-RTPSource for one track
    //need add each report block to RR
    sp<RTPSource> source = pTrack->mRTPSource;

    //if hold on,or send-only, will no report block
    if((source.get()) && (OK == source->addReceiverReportBlock(buffer))) {
        RC++;
    }

    return RC;
}

}
