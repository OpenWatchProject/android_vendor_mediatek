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
*  RTPSender.cpp
*
* Project:
* --------
*   Android
*
* Description:
* ------------
*   pack bistream to RTP packet(RFC6184)
*
* Author:
* -------
*   Haizhen.Wang(mtk80691)
*
****************************************************************************/

//#define LOG_NDEBUG 0
#include <utils/Log.h>

#include "RTPSender.h"
#include "RTPController.h"
//#include <MetaData.h>
#include <cutils/properties.h>
#include <sys/stat.h>
#include <fcntl.h>


#include <media/stagefright/foundation/avc_utils.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <inttypes.h>
#define ATRACE_TAG ATRACE_TAG_VIDEO
#include <utils/Trace.h>

#include "ImsSignal.h"
#undef LOG_TAG
#define LOG_TAG "[VT][RTP]RTPSender"

#include "IVcodecCap.h"
#include "VcodecCap.h"

using namespace android;
using android::status_t;

namespace imsma
{

//Need Check: whether need to release the memory for mpVideoCapParams,mpAudioCapParams

RTPSender::RTPSender(uint32_t ssrc,sp<SocketWrapper> spRTPSocketWrapper,sp<AMessage> notify, uint32_t simID,uint32_t operatorID)
{
    ALOGI("%s:always set front for test RJIL conference call issue",__FUNCTION__);

    if(!notify.get()) {
        ALOGE("%s,notify is NULL",__FUNCTION__);
    }

    mNotify = notify;
    mRTPSocketWrapper = spRTPSocketWrapper;

    mAdaInfo = new TxAdaptationInfo();

    //ToDo:
    //need dup mNotify before using mNotify

    //ToDo: get from modem
    mMTUSize = imsma_rtp_MTU_size;
    memset(&mConfigParam,0,sizeof(mConfigParam));

    m_videoRotation = 0; //in clockwise angle
    m_camera_facing = 0; //0: Front-facing camera;
    m_flipped = 0;
    mOperatorID = operatorID;
    mSimID = simID;

    m_extmap_CVO_supported = 0;
    m_extmap_CVO_id = 0;

    m_rtp_fix_header_len = 12;
    m_rtp_ext_header_len = 0;

    mSendRTPEventPending = false;
    mStarted = false;

    mRTPSeqNo = 0;//ToDo: change to random number
    //ToDo: need make this interval can config by property
    mSendIntervalMs = 10;//default send interval is 20ms

    mRTPGeneration = 0;

    //m_isFirstRTPPacket = true;
    mIsHoldOnEnable = false;

    mAccuReceived = 0;

    mSSRC = ssrc;

    mLastReduceSignal = ImsSignal::Signal_STRENGTH_NONE_OR_UNKNOWN;

#ifdef DEBUG_DUMP_PACKET
    mDumpUpLinkPacket = 0;// ToDo: 1 not work
    mRTPFd = -1;
    mAccuFd = -1;

    ALOGD("mDumpUpLinkPacket =%" PRId64 "",mDumpUpLinkPacket);

    char dump_param[PROPERTY_VALUE_MAX];
    memset(dump_param,0,sizeof(dump_param));

    //int64_t dump_value;
    if(property_get("vendor.vt.imsma.dump_uprtp_packet", dump_param, NULL)) {
        mDumpUpLinkPacket = atol(dump_param);
        ALOGD("dump uplink rtp packet =%" PRId64 "", mDumpUpLinkPacket);
    }

    if(mDumpUpLinkPacket > 0) {
        const char* rtp_filename = "/sdcard/uplink_rtp_data.raw";
        mRTPFd = open(rtp_filename, O_CREAT | O_LARGEFILE | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
        ALOGD("open %s,rtpFd(%d)",rtp_filename,mRTPFd);

        const char* accu_filename = "/sdcard/uplink_accu_data.raw";
        mAccuFd = open(accu_filename, O_CREAT | O_LARGEFILE | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
        ALOGD("open %s,AccuFd(%d)",accu_filename,mAccuFd);
    }

#endif
}
RTPSender::~RTPSender()
{
    ALOGI("%s",__FUNCTION__);

    delete mAdaInfo;

#ifdef DEBUG_DUMP_PACKET

    if(mRTPFd >= 0) {
        ALOGD("close rtpFd(%d)",mRTPFd);
        close(mRTPFd);
        mRTPFd = -1;
    }

    if(mAccuFd >= 0) {
        ALOGD("close AccuFd(%d)",mAccuFd);
        close(mAccuFd);
        mAccuFd = -1;
    }

#endif
}
status_t RTPSender::setConfigParams(rtp_rtcp_config_t* pRTPNegotiatedParams)
{
    ALOGI("%s",__FUNCTION__);
    sp<AMessage> msg = new AMessage(kWhatSetConfigParams, this);
    msg->setPointer("config_params",pRTPNegotiatedParams);

    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);
    return err;

}
status_t RTPSender::updateConfigParams(rtp_rtcp_config_t* pRTPNegotiatedParams)
{
    ALOGI("%s",__FUNCTION__);
    sp<AMessage> msg = new AMessage(kWhatUpdateConfigParams, this);
    msg->setPointer("config_params",pRTPNegotiatedParams);

    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);

    return err;
}


status_t RTPSender::start()
{
    //Mutex::Autolock autoLock(mLock);
    ALOGI("%s",__FUNCTION__);
    sp<AMessage> msg = new AMessage(kWhatStart, this);
    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);

    return err;
}
/*
status_t RTPSender::pause(){
    ALOGI("%s",__FUNCTION__);
    sp<AMessage> msg = new AMessage(kWhatPause, this);
    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);

    return err;
}
status_t RTPSender::resume(){
    ALOGI("%s",__FUNCTION__);
    sp<AMessage> msg = new AMessage(kWhatResume, this);
    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);

    return err;
}
*/
status_t RTPSender::stop()
{
    ALOGI("%s",__FUNCTION__);
    //ToDo:Whether stop will take too much time
    //if there are so many queueAccu or sentRTP AMessage in looper queue
    sp<AMessage> msg = new AMessage(kWhatStop, this);
    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);

    return err;
}

status_t RTPSender::queueAccessUnit(const sp<ABuffer> &accessUnit)
{
    sp<AMessage> msg = new AMessage(kWhatQueueAccessUnit, this);
    msg->setBuffer("buffer",accessUnit);
    msg->post();
    return OK;
}

bool RTPSender::isActive()
{
    ALOGI("%s",__FUNCTION__);

    sp<AMessage> msg = new AMessage(kWhatIsActive, this);
    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);

    int32_t isActive = 0;
    response->findInt32("isActive",&isActive);

    return isActive;
}

status_t RTPSender::setVideoOrientation(uint8_t rotation,uint8_t camera_facing,uint8_t flip)
{
    ALOGI("%s,rotation = %d,camera_facing=%d,flip=%d",__FUNCTION__,rotation,camera_facing,flip);

    sp<AMessage> msg = new AMessage(kWhatSetCVOinfo, this);
    msg->setInt32("rotation",rotation);
    msg->setInt32("face",camera_facing);
    msg->setInt32("flip",flip);

    msg->post();

    return OK;
}

void RTPSender::addSenderInfo(const sp<ABuffer> &buffer)
{
    ALOGV("%s ++",__FUNCTION__);
    sp<AMessage> msg = new AMessage(kWhatAddSenderInfo, this);
    msg->setBuffer("buffer",buffer);

    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);

    ALOGV("%s --",__FUNCTION__);
    return;

}
status_t RTPSender::processReportBlock(const sp<ABuffer> &packet)
{
    ALOGV("%s ++",__FUNCTION__);
    sp<AMessage> msg = new AMessage(kWhatProcessRecvReport, this);
    msg->setBuffer("buffer",packet);

    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);

    ALOGV("%s --",__FUNCTION__);
    return OK;
}

void RTPSender::processFIR(uint8_t seqNum)
{
    ALOGI("%s ++",__FUNCTION__);
    sp<AMessage> msg = new AMessage(kWhatProcessFIR, this);
    msg->setInt32("seqNum",seqNum);

    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);

    ALOGI("%s --",__FUNCTION__);
    return;

}
void RTPSender::processGenericNACKFB(sp<ABuffer> nack_fcis)
{
    ALOGI("%s ++",__FUNCTION__);
    sp<AMessage> msg = new AMessage(kWhatProcessNACK, this);
    msg->setBuffer("nack_fcis",nack_fcis);

    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);

    ALOGI("%s --",__FUNCTION__);
    return;
}
void RTPSender::processTMMBR(sp<ABuffer> tmmbr_fci)
{
    ALOGV("%s ++",__FUNCTION__);
    sp<AMessage> msg = new AMessage(kWhatProcessTMMBR, this);
    msg->setBuffer("tmmbr_fci",tmmbr_fci);
    msg->post();
    ALOGV("%s --",__FUNCTION__);
    return;
}

uint32_t RTPSender::getSSRC()
{
    ALOGI("%s ++",__FUNCTION__);
    sp<AMessage> msg = new AMessage(kWhatGetSSRC, this);
    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);
    uint32_t ssrc = 0;
    response->findInt32("ssrc", (int32_t*) &ssrc);
    ALOGI("%s --",__FUNCTION__);
    return ssrc;
}
void RTPSender::updateSSRC(uint32_t ssrc)
{
    ALOGI("%s",__FUNCTION__);
    sp<AMessage> msg = new AMessage(kWhatUpdateSSRC, this);
    msg->setInt32("ssrc",ssrc);

    sp<AMessage> response;
    msg->postAndAwaitResponse(&response);

    return;
}

void RTPSender::onMessageReceived(const sp<AMessage> &msg)
{
    ATRACE_CALL();

    switch(msg->what()) {
    case kWhatSetConfigParams: {
        ALOGI("kWhatSetConfigParams");
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));

        rtp_rtcp_config_t* pRTPNegotiatedParams = NULL;

        msg->findPointer("config_params", (void**) &pRTPNegotiatedParams);

        status_t err = onSetConfigParams(pRTPNegotiatedParams);

        sp<AMessage> response = new AMessage;
        response->setInt32("err", err);

        response->postReply(replyID);
        break;
    }
    case kWhatUpdateConfigParams: {
        ALOGI("kWhatUpdateConfigParams");
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));

        rtp_rtcp_config_t* pRTPNegotiatedParams = NULL;

        msg->findPointer("config_params", (void**) &pRTPNegotiatedParams);

        status_t err = onUpdateConfigParams(pRTPNegotiatedParams);

        sp<AMessage> response = new AMessage;
        response->setInt32("err", err);

        response->postReply(replyID);
        break;
    }
    case kWhatStart: {
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));

        //int32_t rtpPath = IMSMA_RTP_UP_DOWN_LINK;
        //int32_t trackIndex = IMSMA_RTP_VIDEO;
        status_t err = onStart();

        sp<AMessage> response = new AMessage;
        response->setInt32("err", err);

        response->postReply(replyID);
        break;
    }
    case kWhatStop: {
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));

        //int32_t rtpPath = IMSMA_RTP_UP_DOWN_LINK;
        //int32_t trackIndex = IMSMA_RTP_VIDEO;
        status_t err = onStop();

        sp<AMessage> response = new AMessage;
        response->setInt32("err", err);

        response->postReply(replyID);
        break;
    }
    /*
    case kWhatPause:
    {
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));

        //int32_t rtpPath = IMSMA_RTP_UP_DOWN_LINK;
        //int32_t trackIndex = IMSMA_RTP_VIDEO;
        status_t err = onPause();

        sp<AMessage> response = new AMessage;
        response->setInt32("err", err);

        response->postReply(replyID);
        break;
    }
    case kWhatResume:
    {
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));

        //int32_t rtpPath = IMSMA_RTP_UP_DOWN_LINK;
        //int32_t trackIndex = IMSMA_RTP_VIDEO;
        status_t err = onResume();

        sp<AMessage> response = new AMessage;
        response->setInt32("err", err);

        response->postReply(replyID);
        break;
    }
    */
    case kWhatQueueAccessUnit: {
        sp<ABuffer> accu;
        msg->findBuffer("buffer",&accu);
        onQueueAccessUnit(accu);
        break;
    }
    case kWhatIsActive: {
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));

        sp<AMessage> response = new AMessage;
        response->setInt32("err", OK);
        response->setInt32("isActive",mStarted);
        response->postReply(replyID);
        break;
    }
    case kWhatGetRTPSentCount: {
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));

        sp<AMessage> response = new AMessage;
        response->setInt32("err", OK);
        response->setInt32("RTPSentCount",mAdaInfo->getRTPSentCount());
        response->postReply(replyID);
        break;
    }
    case kWhatSendRTPPacket: {
        //int64_t bytesNeedSend = 0;
        //msg->findInt64("bytesNeedSend",&bytesNeedSend);
        if(mRTPPacketQueue.size() > 20){
            ALOGD("kWhatSendRTPPacket,mRTPPacketQueue size(%zu)",mRTPPacketQueue.size());
        }

        if(!mStarted) {
            ALOGW("kWhatSendRTPPacket,sender stopped,drop this packet ");
            //ToDo:how to use of mStarted and generation?
            return;
        }

        //Mutex::Autolock autoLock(mLock);

        mSendRTPEventPending = false;

        int32_t rtp_generation = 0;
        msg->findInt32("generation",&rtp_generation);
        bool loop_once = false;
        uint32_t firstSeq = 0;

        if(rtp_generation == mRTPGeneration) {
            while(!mRTPPacketQueue.empty()) {
                if(mConfigParam.rtp_packet_bandwidth == 0) {
                    //ToDo
                    //Need check whether UA will set right vaule to me
                    //otherwise should not drop rtp packet


                    //if b=AS:0, should stop sending RTP
                    //return;
                }

                sp<ABuffer> rtpPacket = *mRTPPacketQueue.begin();

                //find rtpPacket meta
                sp<AMessage> rtp_meta = rtpPacket->meta();
                size_t frame_start = 0;

                if((rtp_meta->findSize("frame_start",&frame_start)) && (frame_start > 0)) {
                    int32_t token = 0;
                    rtp_meta->findInt32("token",&token);
                    ATRACE_ASYNC_BEGIN("RTS:sending",token);
                    //ATRACE_INT("RTS:sending:pkSqN",seqNum);
                    //ATRACE_INT("MAS-RTS:EpkSeqN",seqNum);
                }

                int write_size = 0;
                write_size = mRTPSocketWrapper->writeSock(rtpPacket);

                if(write_size < 0) {
                    ALOGW("kWhatSendRTPPacket,writeSock fail err(%d)(%s)",write_size,strerror(-write_size));

                    if((write_size == -EAGAIN) || (write_size == -EWOULDBLOCK) || (write_size == -EPERM)) {
                        //ToDo: need check the delay time depending on socket queue length
                        msg->post(10000);    //delay 10ms
                        mSendRTPEventPending = true;

                        break;
                        //mSendRTPEventPending = true;
                    } else if((write_size == -ECONNREFUSED) || (write_size == -ENETUNREACH) || (write_size == -EINVAL)) {
                        ALOGW("ignore write error:%d,mLastRTPSeqNum:%u,mRTPSentCount:%u",\
                              write_size,mAdaInfo->getLastRTPSeqNum(),mAdaInfo->getRTPSentCount());
                    } else {
                        CHECK(write_size > 0);
                        break;
                    }
                } else if((uint32_t) write_size != (rtpPacket->size())) {
                    ALOGE("kWhatSendRTPPacket,writeSock write partial data:%d/%zu",\
                          write_size,rtpPacket->size());
                }

                int32_t seqNum = rtpPacket->int32Data();

                if(loop_once == false){
                    firstSeq = seqNum;
                    loop_once = true;
                }

                mAdaInfo->setLastRTPSeqNum(seqNum);
                mAdaInfo->selfIncFrameCount();

                size_t payload_size = 0;
                rtp_meta->findSize("payload_size",&payload_size);
                mAdaInfo->addPayloadSize(payload_size);

                //for adaptation
                updateStatisticInfo(rtpPacket);

                mRTPPacketQueue.erase(mRTPPacketQueue.begin());
                ALOGV("kWhatSendRTPPacket,dequeue rtp packet(seqNum:%d) from mRTPPacketQueue(%zu)",\
                      mAdaInfo->getLastRTPSeqNum(),mRTPPacketQueue.size());

                size_t frame_boundary = 0;

                if((rtp_meta->findSize("frame_boundary",&frame_boundary)) && (frame_boundary > 0)) {
                    int32_t token = 0;
                    rtp_meta->findInt32("token",&token);
                    ATRACE_ASYNC_END("RTS:sending",token);
                    //ATRACE_INT("RTS:sending:pkSqN",seqNum);
                    //ATRACE_INT("MAS-RTS:LpkSeqN",seqNum);
                    ATRACE_ASYNC_END("MAS-RTS",token);
                    ATRACE_INT("MAS-RTS:SeqNo",seqNum);
                }

#ifdef DEBUG_DUMP_PACKET

                if(mRTPFd >= 0) {
                    size_t real_write = write(mRTPFd,rtpPacket->data(),rtpPacket->size());
                    ALOGV("write to file,real_write(%zu)",real_write);
                }

#endif
            }

            ALOGD("kWhatSendRTPPacket,dequeue rtp packet(seqNum:%d-%d)", firstSeq, mAdaInfo->getLastRTPSeqNum());
        } else {
            ALOGW("kWhatSendRTPPacket overdue");
        }

        //ToDo: maybe needed in the future
        //msg->post(mSendIntervalMs * 1000);
        break;
    }
    case kWhatAddSenderInfo: {
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));

        sp<ABuffer> buffer;
        msg->findBuffer("buffer",&buffer);
        onAddSenderInfo(buffer);

        sp<AMessage> response = new AMessage;
        response->setInt32("err", OK);

        response->postReply(replyID);
        break;
    }
    case kWhatProcessRecvReport: {
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));

        sp<ABuffer> packet;
        msg->findBuffer("buffer",&packet);
        status_t err = onProcessReportBlock(packet);

        sp<AMessage> response = new AMessage;
        response->setInt32("err", err);

        response->postReply(replyID);
        break;
    }
    case kWhatProcessFIR: {
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));

        int32_t _seqNum = 0;
        uint8_t seqNum = 0;
        msg->findInt32("seqNum",&_seqNum);
        seqNum = (uint8_t) _seqNum;

        onProcessFIR(seqNum);

        sp<AMessage> response = new AMessage;
        response->setInt32("err", OK);

        response->postReply(replyID);
        break;
    }
    case kWhatProcessNACK: {
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));

        sp<ABuffer> nack_fcis;
        msg->findBuffer("nack_fcis",&nack_fcis);

        onProcessGenericNACK(nack_fcis);

        sp<AMessage> response = new AMessage;
        response->setInt32("err", OK);

        response->postReply(replyID);
        break;
    }
    case kWhatProcessTMMBR: {
        sp<ABuffer> tmmbr_fci;
        msg->findBuffer("tmmbr_fci",&tmmbr_fci);
        onProcessTMMBR(tmmbr_fci);

        break;
    }

    case kWhatSetCVOinfo: {
        int32_t rotation = 0;
        msg->findInt32("rotation",&rotation);

        int32_t camera_facing = 0;
        msg->findInt32("face",&camera_facing);

        int32_t flip = 0;
        msg->findInt32("flip",&flip);

        m_videoRotation = rotation; //in clockwise angle
        m_camera_facing = camera_facing; //0: Front-facing camera; 1:Back-facing camera
        m_flipped = flip;

        /*
        if(rotation != 0 && m_extmap_CVO_supported > 0){
            //if rotation!=0,need tranfer the rotation info in RTP extension header
            m_rtp_ext_header_len +
        }
        */

        break;
    }
    case kWhatGetSSRC: {
        ALOGI("%s,kWhatGetSSRC",__FUNCTION__);
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));

        sp<AMessage> response = new AMessage;

        response->setInt32("ssrc", mSSRC);
        response->setInt32("err", OK);

        response->postReply(replyID);

        break;
    }
    case kWhatUpdateSSRC: {
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));

        msg->findInt32("ssrc", (int32_t*) &mSSRC);

        sp<AMessage> response = new AMessage;

        response->setInt32("err", OK);

        response->postReply(replyID);

        break;
    }

    default:
        TRESPASS();
        break;
    }

}
status_t RTPSender::onSetConfigParams(rtp_rtcp_config_t* pRTPNegotiatedParams)
{
    ALOGD("%s,save video negotiated done params",__FUNCTION__);

    //check whether the negotiated params is supported
    //.....

    //save the negotiated done params of video
    mConfigParam.media_type = pRTPNegotiatedParams->media_type;
    mConfigParam.mime_Type = pRTPNegotiatedParams->mime_Type;
    mConfigParam.rtp_payload_type = pRTPNegotiatedParams->rtp_payload_type;
    mConfigParam.rtp_profile = pRTPNegotiatedParams->rtp_profile;
    mConfigParam.sample_rate = pRTPNegotiatedParams->sample_rate;

    mConfigParam.rtp_packet_bandwidth = pRTPNegotiatedParams->rtp_packet_bandwidth;//in kbps
    mAdaInfo->setRtpBW(mConfigParam.rtp_packet_bandwidth);
    mConfigParam.packetize_mode = pRTPNegotiatedParams->packetize_mode;

    //CVO
    //m_extmap_CVO_supported = 0;
    //m_extmap_CVO_id = 0;
    //m_rtp_ext_header_len = 0;

    mConfigParam.rtp_header_extension_num = pRTPNegotiatedParams->rtp_header_extension_num;

    if(mConfigParam.rtp_header_extension_num > 0) {
        m_rtp_ext_header_len = 4; // init m_rtp_ext_header_len to 4 bytes
    }

    uint8_t max_allow_ext_num = sizeof(mConfigParam.rtp_ext_map) /sizeof(rtp_ext_map_t);
    ALOGD("\t ext num:%d,max allow ext num:%d",\
          mConfigParam.rtp_header_extension_num,max_allow_ext_num);

    if(mConfigParam.rtp_header_extension_num > max_allow_ext_num) {
        mConfigParam.rtp_header_extension_num = max_allow_ext_num;
    }

    for(uint32_t j = 0; j < mConfigParam.rtp_header_extension_num; j++) {
        mConfigParam.rtp_ext_map[j].extension_id = (pRTPNegotiatedParams->rtp_ext_map[j]).extension_id;
        mConfigParam.rtp_ext_map[j].direction = (pRTPNegotiatedParams->rtp_ext_map[j]).direction;
        memcpy(mConfigParam.rtp_ext_map[j].extension_uri, (pRTPNegotiatedParams->rtp_ext_map[j]).extension_uri,\
               sizeof((pRTPNegotiatedParams->rtp_ext_map[j]).extension_uri));

        if(!strcmp("urn:3gpp:video-orientation",mConfigParam.rtp_ext_map[j].extension_uri)) {
            m_extmap_CVO_supported = 1;
            m_extmap_CVO_id = mConfigParam.rtp_ext_map[j].extension_id;
        }
    }

    mConfigParam.rtcp_sender_bandwidth = pRTPNegotiatedParams->rtcp_sender_bandwidth; //in bps
    mConfigParam.rtcp_receiver_bandwidth =  pRTPNegotiatedParams->rtcp_receiver_bandwidth; //in bps

    mConfigParam.rtcp_reduce_size = pRTPNegotiatedParams->rtcp_reduce_size;
    mConfigParam.rtcp_fb_param_num = pRTPNegotiatedParams->rtcp_fb_param_num;

    ALOGD("\t media(0x%x),mime(0x%x),PT(%d),profile(0x%x),sampleRate(%d)",\
          mConfigParam.media_type,mConfigParam.mime_Type,\
          mConfigParam.rtp_payload_type,mConfigParam.rtp_profile,mConfigParam.sample_rate);

    ALOGD("\t b=AS(%d),packetize_mode(%d),b=RS(%" PRId64 "),b=RR(%" PRId64 "),rtcp_rsize(%d)",\
          mConfigParam.rtp_packet_bandwidth,mConfigParam.packetize_mode,\
          mConfigParam.rtcp_sender_bandwidth,mConfigParam.rtcp_receiver_bandwidth,mConfigParam.rtcp_reduce_size);

    ALOGD("\t fb_num(%d) m_extmap_CVO_supported=%d",mConfigParam.rtcp_fb_param_num,m_extmap_CVO_supported);

    for(uint8_t i = 0; i< mConfigParam.rtcp_fb_param_num; i++) {
        mConfigParam.rtcp_fb_type[i].rtcp_fb_id= pRTPNegotiatedParams->rtcp_fb_type[i].rtcp_fb_id;
        mConfigParam.rtcp_fb_type[i].rtcp_fb_param = pRTPNegotiatedParams->rtcp_fb_type[i].rtcp_fb_param;

        //Need check, whether struct can assign data this way
        memcpy(mConfigParam.rtcp_fb_type[i].rtcp_fb_sub_param,pRTPNegotiatedParams->rtcp_fb_type[i].rtcp_fb_sub_param,sizeof(pRTPNegotiatedParams->rtcp_fb_type[i].rtcp_fb_sub_param));

        ALOGD("\t fb_id(%d),fb_param(%d),fb_sub_param(%s)",\
              mConfigParam.rtcp_fb_type[i].rtcp_fb_id,mConfigParam.rtcp_fb_type[i].rtcp_fb_param,\
              mConfigParam.rtcp_fb_type[i].rtcp_fb_sub_param);
    }

    mConfigParam.network_info.interface_type = (pRTPNegotiatedParams->network_info).interface_type;
    mConfigParam.network_info.MBR_UL = (pRTPNegotiatedParams->network_info).MBR_UL;
    mAdaInfo->setMBRUL(mConfigParam.network_info.MBR_UL);
    ALOGD("\t MBR_UL = %d kbps",mConfigParam.network_info.MBR_UL);

    ALOGI("mLooper check");
    return OK;

}
status_t RTPSender::onUpdateConfigParams(rtp_rtcp_config_t* pRTPNegotiatedParams)
{
    ALOGD("%s,update video negotiated done params",__FUNCTION__);
    //save the updated negotiated done params of video
    //Need Check whether need check which item updated
    mConfigParam.media_type = pRTPNegotiatedParams->media_type;
    mConfigParam.mime_Type = pRTPNegotiatedParams->mime_Type;
    mConfigParam.rtp_payload_type = pRTPNegotiatedParams->rtp_payload_type;
    mConfigParam.rtp_profile = pRTPNegotiatedParams->rtp_profile;
    mConfigParam.sample_rate = pRTPNegotiatedParams->sample_rate;

    mConfigParam.rtp_packet_bandwidth = pRTPNegotiatedParams->rtp_packet_bandwidth;
    mAdaInfo->setRtpBW(mConfigParam.rtp_packet_bandwidth);
    mConfigParam.packetize_mode = pRTPNegotiatedParams->packetize_mode;

    //CVO
    //m_extmap_CVO_supported = 0;
    //m_extmap_CVO_id = 0;
    //m_rtp_ext_header_len = 0;

    mConfigParam.rtp_header_extension_num = pRTPNegotiatedParams->rtp_header_extension_num;

    if(mConfigParam.rtp_header_extension_num > 0) {
        m_rtp_ext_header_len = 4; // init m_rtp_ext_header_len to 4 bytes
    } else {
        m_rtp_ext_header_len = 0;
    }

    uint8_t max_allow_ext_num = sizeof(mConfigParam.rtp_ext_map) /sizeof(rtp_ext_map_t);
    ALOGD("\t ext num:%d,max allow ext num:%d",\
          mConfigParam.rtp_header_extension_num,max_allow_ext_num);

    if(mConfigParam.rtp_header_extension_num > max_allow_ext_num) {
        mConfigParam.rtp_header_extension_num = max_allow_ext_num;
    }

    for(uint32_t j = 0; j < mConfigParam.rtp_header_extension_num; j++) {
        mConfigParam.rtp_ext_map[j].extension_id = (pRTPNegotiatedParams->rtp_ext_map[j]).extension_id;
        mConfigParam.rtp_ext_map[j].direction = (pRTPNegotiatedParams->rtp_ext_map[j]).direction;
        memcpy(mConfigParam.rtp_ext_map[j].extension_uri, (pRTPNegotiatedParams->rtp_ext_map[j]).extension_uri,\
               sizeof((pRTPNegotiatedParams->rtp_ext_map[j]).extension_uri));

        if(!strcmp("urn:3gpp:video-orientation",mConfigParam.rtp_ext_map[j].extension_uri)) {
            m_extmap_CVO_supported = 1;
            m_extmap_CVO_id = mConfigParam.rtp_ext_map[j].extension_id;
        }
    }

    mConfigParam.rtcp_sender_bandwidth = pRTPNegotiatedParams->rtcp_sender_bandwidth;
    mConfigParam.rtcp_receiver_bandwidth = pRTPNegotiatedParams->rtcp_receiver_bandwidth;

    mConfigParam.rtcp_reduce_size = pRTPNegotiatedParams->rtcp_reduce_size;
    mConfigParam.rtcp_fb_param_num = pRTPNegotiatedParams->rtcp_fb_param_num;

    ALOGD("\t media(0x%x),mime(0x%x),PT(%d),profile(0x%x),sampleRate(%d)",\
          mConfigParam.media_type,mConfigParam.mime_Type,\
          mConfigParam.rtp_payload_type,mConfigParam.rtp_profile,mConfigParam.sample_rate);

    ALOGD("\t b=AS(%d),packetize_mode(%d),b=RS(%" PRId64 "),b=RR(%" PRId64 "),rtcp_rsize(%d)",\
          mConfigParam.rtp_packet_bandwidth,mConfigParam.packetize_mode,\
          mConfigParam.rtcp_sender_bandwidth,mConfigParam.rtcp_receiver_bandwidth,mConfigParam.rtcp_reduce_size);

    ALOGD("\t fb_num(%d), m_extmap_CVO_supported=%d",mConfigParam.rtcp_fb_param_num,m_extmap_CVO_supported);

    for(uint8_t i = 0; i< mConfigParam.rtcp_fb_param_num; i++) {
        mConfigParam.rtcp_fb_type[i].rtcp_fb_id= pRTPNegotiatedParams->rtcp_fb_type[i].rtcp_fb_id;
        mConfigParam.rtcp_fb_type[i].rtcp_fb_param = pRTPNegotiatedParams->rtcp_fb_type[i].rtcp_fb_param;

        //Need check, whether struct can assign data this way
        memcpy(mConfigParam.rtcp_fb_type[i].rtcp_fb_sub_param,pRTPNegotiatedParams->rtcp_fb_type[i].rtcp_fb_sub_param,sizeof(pRTPNegotiatedParams->rtcp_fb_type[i].rtcp_fb_sub_param));

        ALOGD("\t fb_id(%d),fb_param(%d),fb_sub_param(%s)",\
              mConfigParam.rtcp_fb_type[i].rtcp_fb_id,mConfigParam.rtcp_fb_type[i].rtcp_fb_param,\
              mConfigParam.rtcp_fb_type[i].rtcp_fb_sub_param);
    }

    /*******MBR_UL*******/
    uint32_t uiLastMBR_UL = 0;
    uint32_t uiNewMBR_UL  = 0;

    uiLastMBR_UL = mConfigParam.network_info.MBR_DL;
    uiNewMBR_UL = (pRTPNegotiatedParams->network_info).MBR_DL;

    if(uiLastMBR_UL != 0 && uiNewMBR_UL != 0) {
        if(uiLastMBR_UL > uiNewMBR_UL) {
            ALOGD("\t uplink MBR reduce(%d kbps-> %d kbps)",uiLastMBR_UL,uiNewMBR_UL);
        } else if(uiLastMBR_UL < uiNewMBR_UL) {
            ALOGD("\t uplink MBR increase(%d kbps-> %d kbps)",uiLastMBR_UL,uiNewMBR_UL);
        }
    }

    mConfigParam.network_info.interface_type = (pRTPNegotiatedParams->network_info).interface_type;
    mConfigParam.network_info.MBR_UL = (pRTPNegotiatedParams->network_info).MBR_UL;
    mAdaInfo->setMBRUL(mConfigParam.network_info.MBR_UL);

    return OK;
}

status_t RTPSender::onStart()
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

    //ToDo:
    //avoid sending out the overdue packet before hold on
    //no need pause->resume interface for hold on
    //when hold on-->stop, hold on back ->start

    //but RTPSender should be common, not only for VT
    //so we need add pause, resume
    mRTPPacketQueue.clear();

    ALOGD("sim=%d op %d  name = %s", mSimID,mOperatorID, getOperatorName(mOperatorID));

    //For FIR, need reset these parameters
    mAdaInfo->resetFirParam();

    mStarted = true;

    //other parameters keep the last value
    //such as:mRTPSentCount,mConfigParam

    return OK;
}
status_t RTPSender::onStop()
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

    //make all the send RTP message in queue to be overdue
    mRTPGeneration++;
    mStarted = false;


    /************reset some adaptation related params************/
    mAdaInfo->resetOnStop();

    return OK;
}

void RTPSender::recordSignal(){
    int32_t sigLong = 0 ;
    int32_t sigShort = 0;
    bool stable = false ;

    sp<ImsSignal> mSignal = getDefaultImsSignal();
    stable = mSignal->getSignal(mSimID,&sigLong,&sigShort);

    if(stable){
        mLastReduceSignal = sigLong;
    }else if(sigShort <= ImsSignal::Signal_STRENGTH_GOOD){
        mLastReduceSignal = sigShort;
    }else{
        mLastReduceSignal = ImsSignal::Signal_STRENGTH_NONE_OR_UNKNOWN;
    }

    ALOGD("RTPSender recordSignal %d", mLastReduceSignal);

    deleteDefaultImsSignal();
}

/*
status_t RTPSender::onPause(){
    mRTPPacketQueue.clear();
    mStarted = false;
    return OK;

}
status_t RTPSender::onResume(){
    mStarted = true;
    return OK;
}
*/
//ToDo:we don't support this case:
//if encoder send data by NAL, we can't aggregate several NAL to one RTPPacket
//so if encoder can support multi-slice, encoder need send video by frame
status_t RTPSender::onQueueAccessUnit(const sp<ABuffer> &accessUnit)
{
    //check the mimetype of buffer
    //ALOGD("%s",__FUNCTION__);

    ATRACE_CALL();

    if(!accessUnit.get()) {
        ALOGE("%s,accu is NULL",__FUNCTION__);
        return UNKNOWN_ERROR;
    }

    if(!mStarted) {
        ALOGW("%s,RTPSender not started",__FUNCTION__);
        //ToDo:how to use of mStarted and generation?
        return OK;
    }

    sp<AMessage> accu_meta = accessUnit->meta();

    //for systrace
    int32_t token = 0;

    if(accu_meta->findInt32("token",&token)) {
        ATRACE_ASYNC_BEGIN("MAS-RTS",token);
        ATRACE_ASYNC_BEGIN("RTS:packeting",token);
    }

    //record begin signal
    int32_t Signal = 0;
    if(accu_meta->findInt32("begin_signal",&Signal)){
        mLastReduceSignal = Signal;
    }

    //For FIR
    int32_t isCSD = false;
    accu_meta->findInt32("csd", &isCSD);

    if(true == isCSD) {
        mAdaInfo->setFreshTimeUs();
    }

    mAccuReceived ++;

    //for adapation
    //estimate encoding bit rate,fps
    updateEncBitRate(accessUnit);

#ifdef DEBUG_DUMP_PACKET

//dump raw accu
    if(mAccuFd >= 0) {
        size_t real_write = write(mAccuFd,accessUnit->data(),accessUnit->size());
        //ALOGD("write to file,real_write(%d)",real_write);
    }

#endif

    if(mConfigParam.media_type == IMSMA_RTP_VIDEO && mConfigParam.mime_Type == IMSMA_RTP_VIDEO_H264) {
        if(mConfigParam.packetize_mode == IMSMA_SINGLE_NAL_MODE) {
            queueSingleNALUnitPacket(accessUnit);
        } else if(mConfigParam.packetize_mode == IMSMA_NON_INTERLEAVED_MODE) {
            queueFUorSTAPPacket(accessUnit);
        }

        if(accu_meta->findInt32("token",&token)) {
            ATRACE_ASYNC_END("RTS:packeting",token);
        }

        postSendRTPMessage();
    }//if avc
    else if(mConfigParam.media_type == IMSMA_RTP_VIDEO && mConfigParam.mime_Type == IMSMA_RTP_VIDEO_HEVC) {
        //ALOGD("rtp support h265");
        queueFUorSTAPPacket_HEVC(accessUnit);

        postSendRTPMessage();
    } else {
        ALOGE("not supported mime type");
    }

    return OK;
}

status_t RTPSender::queueSingleNALUnitPacket(const sp<ABuffer> accessUnit)
{
    ALOGV("%s",__FUNCTION__);

    const uint8_t *data = accessUnit->data();
    size_t size = accessUnit->size();
    const uint8_t *nalStart = NULL;
    size_t nalSize = 0;

    int32_t packetCount = 1;

    int64_t timeUs = 0;
    sp<AMessage> accu_meta = accessUnit->meta();
    CHECK(accu_meta->findInt64("timeUs", &timeUs));

    int32_t ccw_rotation = 0; //counter clockwise rotation
    int32_t camera_facing = IMSMA_CAMERA_FACING_UNKNOW;
    int32_t flip = IMSMA_CAMERA_NO_FLIP;

    if(accu_meta.get()) {
        accu_meta->findInt32("ccw_rotation",&ccw_rotation);
        accu_meta->findInt32("camera_facing",&camera_facing);
        accu_meta->findInt32("flip",&flip);
    }

    //TODO: need change to timeUs*mConfigParam.sample_rate/1000000ll
    //uint32_t rtpTime = (timeUs/1000000ll * mConfigParam.sample_rate);
    uint32_t rtpTime = ((timeUs * mConfigParam.sample_rate) /1000000ll);
    ALOGD("%s,accu num(%d),accu size(%zu),timeUs(%" PRId64 " us),rtpTime(%u),rotation(%d),face(%d),flip(%d)",\
          __FUNCTION__,mAccuReceived,size,timeUs,rtpTime,ccw_rotation,camera_facing,flip);


    //only single NAL in one RTP packet support
    while(getNextNALUnit(
                &data, &size, &nalStart, &nalSize,
                true /* startCodeFollows */) == OK) {

        //ToDo:
        //check NAL type
        //if net work is bad, we can drop NAL with low priority
        //..........


        //we create a RTP packet as large as nal
        //12 is RTP header
        //16 reserver for RTP extension header
        //if extension is larger need change the size
        sp<ABuffer> out = new ABuffer(nalSize + 12 + 16);
        //ToDo:
        //add memset to 0 for out
        memset(out->data(),0,out->size());
        out->setRange(0,0);

        //copy some meta from accu to rtp packet
        sp<AMessage> rtp_meta = out->meta();
        rtp_meta->setInt64("timeUs",timeUs);
        rtp_meta->setInt32("ccw_rotation",ccw_rotation);
        rtp_meta->setInt32("camera_facing",camera_facing);
        rtp_meta->setInt32("flip",flip);

        addRTPFixHeader(out);    //ToDo:if packetize_mode=0, we should set PT to another valuse
        addRTPExtHeader(out);

        memcpy(out->data() + out->size(),nalStart,nalSize);
        out->setRange(out->offset(),out->size() + nalSize);

        //revise offset to rtp header
        int32_t rtpOffset = 0;
        rtp_meta->findInt32("rtpOffet",&rtpOffset);
        out->setRange(rtpOffset,out->size() - rtpOffset);

        rtp_meta->setSize("payload_size",nalSize);
        // out->meta()->setInt32("token",count);//profile by qian

        int32_t seqNum = out->int32Data();

        if(packetCount == 1) {
            ALOGD("%s,first nal of the frame",__FUNCTION__);
            rtp_meta->setSize("frame_start",1);

            int32_t token = 0;

            if(accu_meta->findInt32("token",&token)) {
                rtp_meta->setInt32("token",token);
            }

            //ATRACE_INT("RTS:Packeting:pkSqN",seqNum);
            //ATRACE_INT("MAS-RTS:EpkSeqN",seqNum);
            ATRACE_INT("MAS-RTS:SeqNo",seqNum);
        }

        if(size <= 0) {
            ALOGD("%s,last nal of the frame,total packets(%d)",__FUNCTION__,packetCount);
            rtp_meta->setSize("frame_boundary",1);

            int32_t token = 0;

            if(accu_meta->findInt32("token",&token)) {
                rtp_meta->setInt32("token",token);
            }

            //ATRACE_INT("RTS:Packeting:pkSqN",seqNum);
            //ATRACE_INT("MAS-RTS:LpkSeqN",seqNum);
            ATRACE_INT("MAS-RTS::Packeting:pkCnt",packetCount);
        }

        queueRTPPacket(out);

        packetCount++;
    }

    return OK;


}
status_t RTPSender::queueFUorSTAPPacket(const sp<ABuffer> accessUnit)
{
    const uint8_t *data = accessUnit->data();
    size_t size = accessUnit->size();
    const uint8_t *nalStart = NULL;
    size_t nalSize = 0;
    int32_t packetCount = 1;

    int64_t timeUs = 0;
    sp<AMessage> accu_meta = accessUnit->meta();
    CHECK(accu_meta->findInt64("timeUs", &timeUs));

    int32_t ccw_rotation = 0; //counter clockwise rotation
    int32_t camera_facing = IMSMA_CAMERA_FACING_UNKNOW;
    int32_t flip = IMSMA_CAMERA_NO_FLIP;

    if(accu_meta.get()) {
        accu_meta->findInt32("ccw_rotation",&ccw_rotation);
        accu_meta->findInt32("camera_facing",&camera_facing);
        accu_meta->findInt32("flip",&flip);
    }

    //TODO: need change to timeUs*mConfigParam.sample_rate/1000000ll
    //uint32_t rtpTime = (timeUs/1000000ll * mConfigParam.sample_rate);
    uint32_t rtpTime = ((timeUs * mConfigParam.sample_rate) /1000000ll);
    ALOGD("%s,accu num(%d),accu size(%zu),timeUs(%" PRId64 " us),rtpTime(%u),rotation(%d),face(%d),flip(%d)",\
          __FUNCTION__,mAccuReceived,size,timeUs,rtpTime,ccw_rotation,camera_facing,flip);

    //create RTP packet with fixed size of MTU
    sp<ABuffer> out;
    bool isFirstNAL = true;
    size_t outBytesUsed = 0;
    sp<AMessage> rtp_meta;
    size_t payload_size = 0;

    while(getNextNALUnit(
                &data, &size, &nalStart, &nalSize,
                true /* startCodeFollows */) == OK) {
        //ToDo:
        //check NAL type
        //if net work is bad, we can drop NAL with low priority
        //..........
        ALOGV("%s,get one NAL,offset(%p),size(%zu)",__FUNCTION__, nalStart,nalSize);

        if(!out.get()) {
            //ToDo:
            //how to know this the last NAL in this frame
            //so set Market bit of rtp packer fixed header to 1
            // to avoid the receiver know this is the last rtp pakcet until receiving the next rtp header

            //create next RTP packet
            out = new ABuffer(mMTUSize);
            memset(out->data(),0,out->size());
            out->setRange(0,0);

            //copy some meta from accu to rtp packet
            rtp_meta = out->meta();
            rtp_meta->setInt64("timeUs",timeUs);

            rtp_meta->setInt32("ccw_rotation",ccw_rotation);
            rtp_meta->setInt32("camera_facing",camera_facing);
            rtp_meta->setInt32("flip",flip);

            addRTPFixHeader(out);
            addRTPExtHeader(out);
            payload_size = 0;
            outBytesUsed = out->size();
        }

        size_t bytesNeeded = nalSize + 2;

        if(outBytesUsed + bytesNeeded > out->capacity()) {
            bool emitSingleNALPacket = false;

            if(isFirstNAL
                    && outBytesUsed + nalSize <= out->capacity()) {
                // We haven't emitted anything into the current packet yet and
                // this NAL unit fits into a single-NAL-unit-packet while
                // it wouldn't have fit as part of a STAP-A packet.

                memcpy(out->data() + out->size(), nalStart, nalSize);

                payload_size += nalSize;
                outBytesUsed += nalSize;
                isFirstNAL = false;
                emitSingleNALPacket = true;
            }

            if(!isFirstNAL) {
                out->setRange(out->offset(), outBytesUsed);

                //revise offset to rtp header
                int32_t rtpOffset = 0;
                rtp_meta->findInt32("rtpOffet",&rtpOffset);
                out->setRange(rtpOffset,out->size() - rtpOffset);

                rtp_meta->setSize("payload_size",payload_size);

                int32_t seqNum = out->int32Data();

                if(packetCount == 1) {
                    ALOGV("%s,first packet of the frame",__FUNCTION__);
                    rtp_meta->setSize("frame_start",1);

                    int32_t token = 0;

                    if(accu_meta->findInt32("token",&token)) {
                        rtp_meta->setInt32("token",token);
                    }

                    //ATRACE_INT("MAS-RTS:EpkSeqN",seqNum);
                    ATRACE_INT("MAS-RTS:SeqNo",seqNum);
                    //ATRACE_INT("RTS:Packeting:pkSqN",seqNum);
                }

                if(size <= 0 && emitSingleNALPacket) {
                    ALOGD("%s,last packet of the frame,total packets(%d)",__FUNCTION__,packetCount);
                    rtp_meta->setSize("frame_boundary",1);

                    int32_t token = 0;

                    if(accu_meta->findInt32("token",&token)) {
                        rtp_meta->setInt32("token",token);
                    }

                    //ATRACE_INT("RTS:Packeting:pkSqN",seqNum);
                    //ATRACE_INT("MAS-RTS:LpkSeqN",seqNum);
                    ATRACE_INT("MAS-RTS:Packeting:pkcnt",packetCount);
                }

                queueRTPPacket(out);
                packetCount++;

                isFirstNAL = true;
                out = NULL;
            }

            if(emitSingleNALPacket) {
                continue;
            }
        }

        if(!out.get()) {
            //ToDo:
            //how to know this the last NAL in this frame
            //so set Market bit of rtp packer fixed header to 1
            // to avoid the receiver know this is the last rtp pakcet until receiving the next rtp header

            //create next RTP packet
            out = new ABuffer(mMTUSize);
            memset(out->data(),0,out->size());
            out->setRange(0,0);

            //copy some meta from accu to rtp packet
            rtp_meta = out->meta();
            rtp_meta->setInt64("timeUs",timeUs);
            rtp_meta->setInt32("ccw_rotation",ccw_rotation);
            rtp_meta->setInt32("camera_facing",camera_facing);
            rtp_meta->setInt32("flip",flip);

            addRTPFixHeader(out);
            addRTPExtHeader(out);
            payload_size = 0;
            outBytesUsed = out->size();
        }

        if(isFirstNAL) {
            ++bytesNeeded;
        }

        if(outBytesUsed + bytesNeeded <= out->capacity()) {
            uint8_t *dst = out->data() + out->size(); // + outBytesUsed;

            if(isFirstNAL) {
                *dst++ = 24;  // STAP-A header
                isFirstNAL = false;
            }

            *dst++ = (nalSize >> 8) & 0xff;
            *dst++ = nalSize & 0xff;
            memcpy(dst, nalStart, nalSize);

            payload_size += nalSize;
            out->setRange(out->offset(), out->size() + bytesNeeded);
            outBytesUsed += bytesNeeded;

            continue;
        }

        // This single NAL unit does not fit into a single RTP packet,
        // we need to emit an FU-A.
        queueFragmentNALUnitPacket(accessUnit,size,nalStart,nalSize,out,&packetCount);
    }

    if(out.get()) {
        // out->meta()->setInt32("token",count);

        rtp_meta->setSize("payload_size",payload_size);
        ALOGV("%s,no more nal,queue last packet(offset(%zu),size(%zu))",\
              __FUNCTION__,out->offset(),out->size());
        //revise offset to rtp header
        int32_t rtpOffset = 0;
        rtp_meta->findInt32("rtpOffet",&rtpOffset);
        out->setRange(rtpOffset,out->size() - rtpOffset);

        int32_t seqNum = out->int32Data();

        if(packetCount == 1) {
            ALOGV("%s,first packet of the frame",__FUNCTION__);
            rtp_meta->setSize("frame_start",1);

            int32_t token = 0;

            if(accu_meta->findInt32("token",&token)) {
                rtp_meta->setInt32("token",token);
            }

            //ATRACE_INT("RTS:Packeting:pkSqN",seqNum);
            //ATRACE_INT("MAS-RTS:EpkSeqN",seqNum);
            ATRACE_INT("MAS-RTS:SeqNo",seqNum);
        }

        if(size <= 0) {
            ALOGV("%s,last packet of the frame,total packets(%d)",__FUNCTION__,packetCount);
            rtp_meta->setSize("frame_boundary",1);

            int32_t token = 0;

            if(accu_meta->findInt32("token",&token)) {
                rtp_meta->setInt32("token",token);
            }

            //ATRACE_INT("RTS:Packeting:pkSqN",seqNum);
            //ATRACE_INT("MAS-RTS:LpkSeqN",seqNum);
            ATRACE_INT("MAS-RTS:Packeting:pkcnt",packetCount);
        }

        queueRTPPacket(out);
        //packetCount++;
        out = NULL;
    }

    return OK;
}

status_t RTPSender::queueFUorSTAPPacket_HEVC(const sp<ABuffer> accessUnit)
{
    const uint8_t *data = accessUnit->data();
    size_t size = accessUnit->size();
    const uint8_t *nalStart = NULL;
    size_t nalSize = 0;
    int32_t packetCount = 1;

    int64_t timeUs = 0;
    sp<AMessage> accu_meta = accessUnit->meta();
    CHECK(accu_meta->findInt64("timeUs", &timeUs));

    int32_t ccw_rotation = 0; //counter clockwise rotation
    int32_t camera_facing = IMSMA_CAMERA_FACING_UNKNOW;
    int32_t flip = IMSMA_CAMERA_NO_FLIP;

    if(accu_meta.get()) {
        accu_meta->findInt32("ccw_rotation",&ccw_rotation);
        accu_meta->findInt32("camera_facing",&camera_facing);
        accu_meta->findInt32("flip",&flip);
    }

    //TODO: need change to timeUs*mConfigParam.sample_rate/1000000ll
    //uint32_t rtpTime = (timeUs/1000000ll * mConfigParam.sample_rate);
    uint32_t rtpTime = ((timeUs * mConfigParam.sample_rate) /1000000ll);
    ALOGD("%s,accu num(%d),accu size(%zu),timeUs(%" PRId64 " us),rtpTime(%u),rotation(%d),face(%d),flip(%d)",\
          __FUNCTION__,mAccuReceived,size,timeUs,rtpTime,ccw_rotation,camera_facing,flip);

    //create RTP packet with fixed size of MTU
    sp<ABuffer> out;
    bool isFirstNAL = true;
    size_t outBytesUsed = 0;
    sp<AMessage> rtp_meta;
    size_t payload_size = 0;

    while(getNextNALUnit(
                &data, &size, &nalStart, &nalSize,
                true /* startCodeFollows */) == OK) {
        //ToDo:
        //check NAL type
        //if net work is bad, we can drop NAL with low priority
        //..........
        ALOGV("%s,get one NAL,offset(%p),size(%zu)",__FUNCTION__, nalStart,nalSize);

        if(!out.get()) {
            //ToDo:
            //how to know this the last NAL in this frame
            //so set Market bit of rtp packer fixed header to 1
            // to avoid the receiver know this is the last rtp pakcet until receiving the next rtp header

            //create next RTP packet
            out = new ABuffer(mMTUSize);
            memset(out->data(),0,out->size());
            out->setRange(0,0);

            //copy some meta from accu to rtp packet
            rtp_meta = out->meta();
            rtp_meta->setInt64("timeUs",timeUs);

            rtp_meta->setInt32("ccw_rotation",ccw_rotation);
            rtp_meta->setInt32("camera_facing",camera_facing);
            rtp_meta->setInt32("flip",flip);

            addRTPFixHeader(out);
            addRTPExtHeader(out);
            payload_size = 0;
            outBytesUsed = out->size();
        }

        size_t bytesNeeded = nalSize + 2;

        if(outBytesUsed + bytesNeeded > out->capacity()) {
            bool emitSingleNALPacket = false;

            if(isFirstNAL
                    && outBytesUsed + nalSize <= out->capacity()) {
                // We haven't emitted anything into the current packet yet and
                // this NAL unit fits into a single-NAL-unit-packet while
                // it wouldn't have fit as part of a STAP-A packet.

                memcpy(out->data() + out->size(), nalStart, nalSize);

                payload_size += nalSize;
                outBytesUsed += nalSize;
                isFirstNAL = false;
                emitSingleNALPacket = true;
            }

            if(!isFirstNAL) {
                out->setRange(out->offset(), outBytesUsed);

                //revise offset to rtp header
                int32_t rtpOffset = 0;
                rtp_meta->findInt32("rtpOffet",&rtpOffset);
                out->setRange(rtpOffset,out->size() - rtpOffset);

                rtp_meta->setSize("payload_size",payload_size);

                int32_t seqNum = out->int32Data();

                if(packetCount == 1) {
                    ALOGV("%s,first packet of the frame",__FUNCTION__);
                    rtp_meta->setSize("frame_start",1);

                    int32_t token = 0;

                    if(accu_meta->findInt32("token",&token)) {
                        rtp_meta->setInt32("token",token);
                    }

                    //ATRACE_INT("MAS-RTS:EpkSeqN",seqNum);
                    ATRACE_INT("MAS-RTS:SeqNo",seqNum);
                    //ATRACE_INT("RTS:Packeting:pkSqN",seqNum);
                }

                if(size <= 0 && emitSingleNALPacket) {
                    ALOGV("%s,last packet of the frame,total packets(%d)",__FUNCTION__,packetCount);
                    rtp_meta->setSize("frame_boundary",1);

                    int32_t token = 0;

                    if(accu_meta->findInt32("token",&token)) {
                        rtp_meta->setInt32("token",token);
                    }

                    //ATRACE_INT("RTS:Packeting:pkSqN",seqNum);
                    //ATRACE_INT("MAS-RTS:LpkSeqN",seqNum);
                    ATRACE_INT("MAS-RTS:Packeting:pkcnt",packetCount);
                }

                queueRTPPacket(out);
                packetCount++;

                isFirstNAL = true;
                out = NULL;
            }

            if(emitSingleNALPacket) {
                continue;
            }
        }

        if(!out.get()) {
            //ToDo:
            //how to know this the last NAL in this frame
            //so set Market bit of rtp packer fixed header to 1
            // to avoid the receiver know this is the last rtp pakcet until receiving the next rtp header

            //create next RTP packet
            out = new ABuffer(mMTUSize);
            memset(out->data(),0,out->size());
            out->setRange(0,0);

            //copy some meta from accu to rtp packet
            rtp_meta = out->meta();
            rtp_meta->setInt64("timeUs",timeUs);
            rtp_meta->setInt32("ccw_rotation",ccw_rotation);
            rtp_meta->setInt32("camera_facing",camera_facing);
            rtp_meta->setInt32("flip",flip);

            addRTPFixHeader(out);
            addRTPExtHeader(out);
            payload_size = 0;
            outBytesUsed = out->size();
        }

        if(isFirstNAL) {
            bytesNeeded += 2;
        }

        if(outBytesUsed + bytesNeeded <= out->capacity()) {
            uint8_t *dst = out->data() + out->size(); // + outBytesUsed;

            if(isFirstNAL) {
                *dst++ = 48 << 1;  // STAP-A header
                *dst++ = 0x1;
                isFirstNAL = false;
            }

            *dst++ = (nalSize >> 8) & 0xff;
            *dst++ = nalSize & 0xff;
            memcpy(dst, nalStart, nalSize);

            payload_size += nalSize;
            out->setRange(out->offset(), out->size() + bytesNeeded);
            outBytesUsed += bytesNeeded;

            continue;
        }

        // This single NAL unit does not fit into a single RTP packet,
        // we need to emit an FU-A.
        queueFragmentNALUnitPacket_HEVC(accessUnit,size,nalStart,nalSize,out,&packetCount);
    }

    if(out.get()) {
        // out->meta()->setInt32("token",count);

        rtp_meta->setSize("payload_size",payload_size);
        ALOGV("%s,no more nal,queue last packet(offset(%zu),size(%zu))",\
              __FUNCTION__,out->offset(),out->size());
        //revise offset to rtp header
        int32_t rtpOffset = 0;
        rtp_meta->findInt32("rtpOffet",&rtpOffset);
        out->setRange(rtpOffset,out->size() - rtpOffset);

        int32_t seqNum = out->int32Data();

        if(packetCount == 1) {
            ALOGV("%s,first packet of the frame",__FUNCTION__);
            rtp_meta->setSize("frame_start",1);

            int32_t token = 0;

            if(accu_meta->findInt32("token",&token)) {
                rtp_meta->setInt32("token",token);
            }

            //ATRACE_INT("RTS:Packeting:pkSqN",seqNum);
            //ATRACE_INT("MAS-RTS:EpkSeqN",seqNum);
            ATRACE_INT("MAS-RTS:SeqNo",seqNum);
        }

        if(size <= 0) {
            ALOGV("%s,last packet of the frame,total packets(%d)",__FUNCTION__,packetCount);
            rtp_meta->setSize("frame_boundary",1);

            int32_t token = 0;

            if(accu_meta->findInt32("token",&token)) {
                rtp_meta->setInt32("token",token);
            }

            //ATRACE_INT("RTS:Packeting:pkSqN",seqNum);
            //ATRACE_INT("MAS-RTS:LpkSeqN",seqNum);
            ATRACE_INT("MAS-RTS:Packeting:pkcnt",packetCount);
        }

        queueRTPPacket(out);
        //packetCount++;
        out = NULL;
    }

    return OK;
}

status_t RTPSender::queueFragmentNALUnitPacket(
    const sp<ABuffer> accessUnit,size_t remain_size,const uint8_t* nalStart,const size_t nalSize,sp<ABuffer>& out,int32_t* packetCount)
{
    uint8_t nalType = nalStart[0] & 0x1f;
    uint8_t nri = (nalStart[0] >> 5) & 3;

    size_t srcOffset = 1;
    size_t num_nal_FU_A = 0; //for debug
    bool isEndFU = false;
    size_t payload_size = 0;
    size_t outBytesUsed = 0;

    sp<AMessage> rtp_meta;

    if(out.get()) {
        outBytesUsed = out->size();
        rtp_meta = out->meta();

    }

    //sp<ABuffer> out;

    int64_t timeUs = 0;
    sp<AMessage> accu_meta = accessUnit->meta();
    CHECK(accu_meta->findInt64("timeUs", &timeUs));

    int32_t ccw_rotation = 0; //counter clockwise rotation
    int32_t camera_facing = IMSMA_CAMERA_FACING_UNKNOW;
    int32_t flip = IMSMA_CAMERA_NO_FLIP;

    if(accu_meta.get()) {
        accu_meta->findInt32("ccw_rotation",&ccw_rotation);
        accu_meta->findInt32("camera_facing",&camera_facing);
        accu_meta->findInt32("flip",&flip);
    }

    while(srcOffset < nalSize) {
        if(!out.get()) {
            //create next RTP packet
            out = new ABuffer(mMTUSize);
            memset(out->data(),0,out->size());
            out->setRange(0,0);

            //copy some meta from accu to rtp packet
            rtp_meta = out->meta();
            rtp_meta->setInt64("timeUs",timeUs);

            rtp_meta->setInt32("ccw_rotation",ccw_rotation);
            rtp_meta->setInt32("camera_facing",camera_facing);
            rtp_meta->setInt32("flip",flip);

            addRTPFixHeader(out);
            addRTPExtHeader(out);
            num_nal_FU_A ++;
            payload_size = 0;
            outBytesUsed = out->size();
        }

        size_t copy = out->capacity() - outBytesUsed - 2;

        if(copy > nalSize - srcOffset) {
            copy = nalSize - srcOffset;
        }

        uint8_t *dst = out->data() + out->size();// + outBytesUsed;
        ALOGV("%s,header size(%zu)",__FUNCTION__,out->size());
        dst[0] = (nri << 5) | 28;

        dst[1] = nalType;

        if(srcOffset == 1) {
            ALOGV("the first FU-A");
            dst[1] |= 0x80;
        }

        if(srcOffset + copy == nalSize) {
            ALOGV("the end FU-A");
            dst[1] |= 0x40;
            isEndFU = true;
        }

        memcpy(&dst[2], nalStart + srcOffset, copy);
        payload_size += copy;
        srcOffset += copy;

        out->setRange(out->offset(), out->size() + copy + 2);
        ALOGV("FU-A(%zu),packet offset(%zu),size(%zu)",num_nal_FU_A,out->offset(),out->size());
        //revise offset to rtp header
        int32_t rtpOffset = 0;
        rtp_meta->findInt32("rtpOffet",&rtpOffset);
        out->setRange(rtpOffset,out->size() - rtpOffset);

        rtp_meta->setSize("payload_size",payload_size);

        int32_t seqNum = out->int32Data();

        if(*packetCount == 1) {
            ALOGV("%s,first packet of the frame",__FUNCTION__);
            rtp_meta->setSize("frame_start",1);

            int32_t token = 0;

            if(accu_meta->findInt32("token",&token)) {
                rtp_meta->setInt32("token",token);
            }

            //ATRACE_INT("MAS-RTS:EpkSeqN",seqNum);
            ATRACE_INT("MAS-RTS:SeqNo",seqNum);
            //ATRACE_INT("RTS:Packeting:pkSqN",seqNum);
        }

        if((remain_size <= 0) && (isEndFU)) {
            ALOGV("%s,last packet of the frame,total packets(%d)",__FUNCTION__,*packetCount);
            rtp_meta->setSize("frame_boundary",1);

            int32_t token = 0;

            if(accu_meta->findInt32("token",&token)) {
                rtp_meta->setInt32("token",token);
            }

            //ATRACE_INT("RTS:Packeting:pkSqN",seqNum);
            //ATRACE_INT("MAS-RTS:LpkSeqN",seqNum);
            ATRACE_INT("MAS-RTS:Packeting:pkcnt",*packetCount);
        }

        queueRTPPacket(out);
        (*packetCount) ++;

        out = NULL;
    }

    return OK;

}

status_t RTPSender::queueFragmentNALUnitPacket_HEVC(
    const sp<ABuffer> accessUnit,size_t remain_size,const uint8_t* nalStart,const size_t nalSize,sp<ABuffer>& out,int32_t* packetCount)
{
    uint8_t nalType = (nalStart[0] & 0x7E) >> 1;
    uint8_t nalheader_1 = nalStart[0];
    uint8_t nalheader_2 = nalStart[1];

    size_t srcOffset = 2;
    size_t num_nal_FU_A = 0; //for debug
    bool isEndFU = false;
    size_t payload_size = 0;
    size_t outBytesUsed = 0;

    sp<AMessage> rtp_meta;

    if(out.get()) {
        outBytesUsed = out->size();
        rtp_meta = out->meta();
    }

    //sp<ABuffer> out;

    int64_t timeUs = 0;
    sp<AMessage> accu_meta = accessUnit->meta();
    CHECK(accu_meta->findInt64("timeUs", &timeUs));

    int32_t ccw_rotation = 0; //counter clockwise rotation
    int32_t camera_facing = IMSMA_CAMERA_FACING_UNKNOW;
    int32_t flip = IMSMA_CAMERA_NO_FLIP;

    if(accu_meta.get()) {
        accu_meta->findInt32("ccw_rotation",&ccw_rotation);
        accu_meta->findInt32("camera_facing",&camera_facing);
        accu_meta->findInt32("flip",&flip);
    }

    while(srcOffset < nalSize) {
        if(!out.get()) {
            //create next RTP packet
            out = new ABuffer(mMTUSize);
            memset(out->data(),0,out->size());
            out->setRange(0,0);

            //copy some meta from accu to rtp packet
            rtp_meta = out->meta();
            rtp_meta->setInt64("timeUs",timeUs);

            rtp_meta->setInt32("ccw_rotation",ccw_rotation);
            rtp_meta->setInt32("camera_facing",camera_facing);
            rtp_meta->setInt32("flip",flip);

            addRTPFixHeader(out);
            addRTPExtHeader(out);
            num_nal_FU_A ++;
            payload_size = 0;
            outBytesUsed = out->size();
        }

        size_t copy = out->capacity() - outBytesUsed - 3;

        if(copy > nalSize - srcOffset) {
            copy = nalSize - srcOffset;
        }

        uint8_t *dst = out->data() + out->size();// + outBytesUsed;
        dst[0] = nalheader_1;
        dst[0] &= 0x81;
        dst[0] |= (49 << 1);
        dst[1] = nalheader_2;

        ALOGD("%s,header size(%zu) type(%d) nalheader_1=0x%x nalheader_2=0x%x sdt0=0x%x",
              __FUNCTION__,out->size(),nalType, nalheader_1, nalheader_2, dst[0]);

        dst[2] = nalType;

        if(srcOffset == 2) {
            ALOGD("the first FU-A");
            dst[2] |= 0x80;
        }

        if(srcOffset + copy == nalSize) {
            ALOGD("the end FU-A");
            dst[2] |= 0x40;
            isEndFU = true;
        }

        memcpy(&dst[3], nalStart + srcOffset, copy);
        payload_size += copy;
        srcOffset += copy;

        out->setRange(out->offset(), out->size() + copy + 3);
        ALOGD("FU-A(%zu),packet offset(%zu),size(%zu)",num_nal_FU_A,out->offset(),out->size());
        //revise offset to rtp header
        int32_t rtpOffset = 0;
        rtp_meta->findInt32("rtpOffet",&rtpOffset);
        out->setRange(rtpOffset,out->size() - rtpOffset);

        rtp_meta->setSize("payload_size",payload_size);

        int32_t seqNum = out->int32Data();

        if(*packetCount == 1) {
            ALOGD("%s,first packet of the frame",__FUNCTION__);
            rtp_meta->setSize("frame_start",1);

            int32_t token = 0;

            if(accu_meta->findInt32("token",&token)) {
                rtp_meta->setInt32("token",token);
            }

            //ATRACE_INT("MAS-RTS:EpkSeqN",seqNum);
            ATRACE_INT("MAS-RTS:SeqNo",seqNum);
            //ATRACE_INT("RTS:Packeting:pkSqN",seqNum);
        }

        if((remain_size <= 0) && (isEndFU)) {
            ALOGD("%s,last packet of the frame,total packets(%d)",__FUNCTION__,*packetCount);
            rtp_meta->setSize("frame_boundary",1);

            int32_t token = 0;

            if(accu_meta->findInt32("token",&token)) {
                rtp_meta->setInt32("token",token);
            }

            //ATRACE_INT("RTS:Packeting:pkSqN",seqNum);
            //ATRACE_INT("MAS-RTS:LpkSeqN",seqNum);
            ATRACE_INT("MAS-RTS:Packeting:pkcnt",*packetCount);
        }

        queueRTPPacket(out);
        (*packetCount) ++;

        out = NULL;
    }

    return OK;
}

void RTPSender::postSendRTPMessage()
{
    /* it's not necessary after re-architecture, because we will handle these packets by accu*/
    if(mSendRTPEventPending) {
        //avoid too many sendRTPPacket message in queue
        //ToDo: how to make sure the last packet in queue also can be overdue after stop
        //avoid keep the old packet in queue,
        return;
    }

    sp<AMessage> msg = new AMessage(kWhatSendRTPPacket,this);
    msg->setInt32("generation",mRTPGeneration);
    msg->post();
    mSendRTPEventPending = true;
}

void RTPSender::queueRTPPacket(sp<ABuffer> rtpPacket)
{
    ATRACE_CALL();
    //if(m_isFirstRTPPacket){
    //  m_isFirstRTPPacket = false;
    //}

    //update some rtp header here
    sp<AMessage> rtp_meta = rtpPacket->meta();
    uint8_t* data = rtpPacket->data();

    //update marker bit
    size_t frame_boundary = 0;
    rtp_meta->findSize("frame_boundary",&frame_boundary);

    if(frame_boundary > 0) {
        data[1] |= 1 << 7;  // M-bit
    }

    size_t frame_start = 0;
    rtp_meta->findSize("frame_start",&frame_start);
    //update other rtp header bit
    //...


    //ToDo:
    //statistic the packet number
    //Mutex::Autolock autoLock(mLock);

    mRTPPacketQueue.push_back(rtpPacket);

    //for adaptation
    if((frame_start || frame_boundary) && mRTPPacketQueue.size() > 10) {
        ALOGD("%s,rtp queue size(%zu),frame_start =%zu,frame_boundary=%zu",\
              __FUNCTION__,mRTPPacketQueue.size(),frame_start,frame_boundary);
        uint32_t target = 0;
        uint32_t durationUs = requestRTPQueueDuration();
        bool isAdjust = mAdaInfo->checkFallBack(&target, durationUs);

        if(isAdjust == true) {
            notifyAdjustEncBR(target);
        }
    }

    //postSendRTPMessage();
}
status_t RTPSender::addRTPFixHeader(sp<ABuffer> rtpPacket)
{
    int64_t timeUs = 0;
    sp<AMessage> rtp_meta = rtpPacket->meta();

    CHECK(rtp_meta->findInt64("timeUs", &timeUs));

    //TODO: need change to timeUs*mConfigParam.sample_rate/1000000ll
    uint32_t rtpTime = ((timeUs * mConfigParam.sample_rate) /1000000ll);
    ALOGV("%s,timeUs(%" PRId64 " us),rtpTime(%u)",__FUNCTION__,timeUs,rtpTime);
    //ToDo:
    //Need calculate rtpTime use initRTPTime + delta(timeUs-lastTimeUs)
    //and should consider:if hold on back,the timeUs will change,but the rtpTime whether need change
    //whether need re-map(rtp->ntp) after hold on back
    //whether the inital rtp time need new rand()

    //ToDo: AMessage no set Unit32
    //rtp_meta->setInt64("rtptime",rtpTime);

    mAdaInfo->setLastTimeUs(timeUs);
    mAdaInfo->setLastRtpTime(rtpTime);
    //mLastRTPSeqNum = mRTPSeqNo;

    //ToDO:
    //need handle rtptime overflow
    //need make the first rtptime to random value
    //each call start,need re-random the first rtp time

    //uint32_t rtpTime = (timeUs * 9 / 100ll);

    rtpPacket->setInt32Data(mRTPSeqNo);

    uint8_t *dst = rtpPacket->data() + rtpPacket->size();
    size_t rtpOffset =  rtpPacket->offset() + rtpPacket->size();
    rtp_meta->setInt32("rtpOffet",rtpOffset);


    dst[0] = 0x80;
    //ToDo: the rtp_payload_type is the remote payload type ,should not be used for
    //sender
    dst[1] = mConfigParam.rtp_payload_type;//7bit PT, 1bit M-bit = 0
    ALOGV("%s,rtp_payload_type=%d",__FUNCTION__, mConfigParam.rtp_payload_type);

    //ToDo:
    size_t frame_boundary = 0;
    rtp_meta->findSize("frame_boundary",&frame_boundary);

    if(frame_boundary) {
        dst[1] |= 1 << 7;  // M-bit
    }

    dst[2] = (mRTPSeqNo >> 8) & 0xff;
    dst[3] = mRTPSeqNo & 0xff;
    ++mRTPSeqNo;

    dst[4] = rtpTime >> 24;
    dst[5] = (rtpTime >> 16) & 0xff;
    dst[6] = (rtpTime >> 8) & 0xff;
    dst[7] = rtpTime & 0xff;
    dst[8] = mSSRC >> 24;
    dst[9] = (mSSRC >> 16) & 0xff;
    dst[10] = (mSSRC >> 8) & 0xff;
    dst[11] = mSSRC & 0xff;

    ALOGV("%s,before setRange,offset(%zu),size(%zu)",__FUNCTION__,rtpPacket->offset(),rtpPacket->size());
    rtpPacket->setRange(rtpPacket->offset(),rtpPacket->size() + 12);
    ALOGV("%s,after setRange,offset(%zu),size(%zu)",__FUNCTION__,rtpPacket->offset(),rtpPacket->size());

    return OK;
}
status_t RTPSender::addRTPExtHeader(sp<ABuffer> rtpPacket)
{
    uint16_t rtp_ext_header_len = 0; //in bytes
    uint16_t rtp_ext_with_padding = 0;//in bytes

    enum {
        ONE_BYTE_HEADER = 1,
        TWO_BYTE_HEADER = 2,
    };
    uint8_t header_byte_type = 0; // one-byte header, two-byte header

    sp<AMessage> meta = rtpPacket->meta();

    //ToDo:
    //as spec of TS26.114 7.4.5 describe
    //we only need set CVO info to the last packet of I frame
    //and only if cvo changed, we can set CVO info to P frame
    //but for simple we set CVO info for each packet
    //as spec method, peer may keep the last CVO info if no CVO set for new packet
    //so we also need set cvo info to RTP packet even rotation change to 0

    if(m_extmap_CVO_supported > 0) {
        if(rtp_ext_header_len == 0) {
            //if the first extension element,need add the extension header first
            rtp_ext_header_len += 4; //extension header
        }

        rtp_ext_header_len += 2; //extension element:CVO
        header_byte_type = ONE_BYTE_HEADER;
    }

    //other extension element: such as gps
    /*
    size_t gps = 0;
    meta->findSize("frame_gps",&gps);
    if(gps_supported && gps != 0 ){
        rtp_ext_header_len + 2;
    }*/

    if(rtp_ext_header_len == 0) {
        //no extension element needed
        //no need add RTP extension header
        return OK;
    } else if(rtp_ext_header_len > 0) {
        uint8_t *dst = rtpPacket->data() + rtpPacket->size();

        //update X tag of RTP fixed header
        //uint8_t *base = rtpPacket->base();
        int32_t rtpOffset = 0;
        meta->findInt32("rtpOffet",&rtpOffset);
        uint8_t *base = rtpPacket->base() + rtpOffset;

        base[0] = base[0] | 0x10; //set X tag to 1

        if(header_byte_type == ONE_BYTE_HEADER) {
            dst[0] = 0xBE;
            dst[1] = 0XDE;
        }

        //ToDo:we know we support sendrecv for CVO
        //avoid peer set wrong direction capability, we will not filter cvo info as direction
        //if CVO not support by peer, it can just ignore this extension
        if(m_extmap_CVO_supported > 0) {

            int32_t ccw_rotation = 0; //counter clockwise rotation
            int32_t camera_facing = IMSMA_CAMERA_FACING_UNKNOW;
            int32_t flip = IMSMA_CAMERA_NO_FLIP;

            meta->findInt32("ccw_rotation",&ccw_rotation);
            meta->findInt32("camera_facing",&camera_facing);
            meta->findInt32("flip",&flip);

            //always set front for test RJIL conference call issue
            if(mOperatorID == 18) {
                camera_facing = IMSMA_CAMERA_FACING_FRONT;
            }

            dst[4] = m_extmap_CVO_id << 4 | 0x00; //id =m_extmap_CVO_id; L= 0

            uint8_t cvo_element = 0;
            //uint32_t final_rotation = frame_rotation != 0 ? frame_rotation : m_videoRotation;
            cvo_element = camera_facing << 3 | flip << 2;

            switch(ccw_rotation) {
            case 0:
                cvo_element = cvo_element | 0x00; //R1=0,R0=0
                break;
            case 90:
                cvo_element = cvo_element | 0x01; //R1=0,R0=1
                break;
            case 180:
                cvo_element = cvo_element | 0x02; //R1=1,R0=0
                break;
            case 270:
                cvo_element = cvo_element | 0x03; //R1=1,R0=1
                break;
            default:
                break;
            }

            cvo_element = cvo_element & 0x0F; //set the first four bit to 0

            dst[5] = cvo_element;
        }

        //if has other extension element
        //add here ...

        //need align the rtp_ext_header_len to 32-bit words, 4 bytes
        rtp_ext_with_padding = (rtp_ext_header_len + 3) & 0xFFFC;

        uint16_t rtp_ext_element_len = (rtp_ext_with_padding - 4) / 4;   //subtract header of 4 bytes and change to in 32-bit words
        //ToCheck need change to network byte order(big-endian)
        //set the whole extension len excluding the 4 bytes extension header
        dst[2] = (rtp_ext_element_len >> 8) & 0x00FF;
        dst[3] = rtp_ext_element_len & 0x00FF;
        //(uint16_t*)dst[2] = htons(rtp_ext_element_len);

        //add padding with 0
        for(int i = 0; i < (rtp_ext_with_padding - rtp_ext_header_len); i++) {
            dst[rtp_ext_with_padding-i-1] = 0x00;
        }

        //update the offset
        rtpPacket->setRange(rtpPacket->offset(),rtpPacket->size() + rtp_ext_with_padding);
        ALOGV("%s,offset(%zu),size(%zu)",__FUNCTION__,rtpPacket->offset(),rtpPacket->size());
        return OK;
    }

    return OK;
}

void RTPSender::onAddSenderInfo(const sp<ABuffer> &buffer)
{
    ALOGV("%s ++",__FUNCTION__);

    if(buffer->size() + 20 > buffer->capacity()) {
        ALOGW("RTCP buffer too small to accomodate Sender info.");
        return;
    }

    uint8_t *data = buffer->data() + buffer->size();

    mAdaInfo->addSenderInfo(data);

    buffer->setRange(buffer->offset(), buffer->size() + 20);

    return;
}

status_t RTPSender::onProcessReportBlock(const sp<ABuffer> &packet)
{
    if(!packet.get() || (packet->size() < 20)) {
        ALOGE("%s,buffer is not valid",__FUNCTION__);
        return UNKNOWN_ERROR;
    }

    uint8_t *data = packet->data();
    sp<AMessage> meta = packet->meta();
    int32_t contain_TMMBR = false;
    meta->findInt32("contain_tmmbr",&contain_TMMBR);

    signalInfo sigInfo;
    sigInfo.ratio = 100;
    sigInfo.needRecord = NULL;
    sigInfo.maxBitrate = getMaxBitrate();
    sigInfo.active = NULL;

    if(mConfigParam.network_info.interface_type){
        sigInfo.canExpand = false;
        ALOGD("%s, not over LTE %d, not expand", __FUNCTION__, mConfigParam.network_info.interface_type);
    }else{
        sigInfo.canExpand = ifRiseBitrateBaseSignal(&sigInfo.ratio);
    }

    uint32_t target = 0;
    bool needForceI = false;
    mAdaInfo->processReportBlock(data, contain_TMMBR, sigInfo, &target, &needForceI);
    if(target != 0 && sigInfo.canExpand == true){
        notifyAdjustEncBR(target);
        mLastReduceSignal = ImsSignal::Signal_STRENGTH_NONE_OR_UNKNOWN;
    }

    if(needForceI == true){
        sp<AMessage> refreshPointNotify = mNotify->dup();
        refreshPointNotify->setInt32("what",kWhatForceIntraPicture);
        refreshPointNotify->post();
    }

    return OK;
}

void RTPSender::onProcessFIR(uint8_t seqNum)
{
    ALOGI("%s +",__FUNCTION__);

    if(!mStarted) {
        ALOGW("%s,RTPSender not started",__FUNCTION__);
        return;
    }

    if(mAdaInfo->processFIR(seqNum) == false) {
        return;
    }

    //if no refresh point has sent before
    //Refresh point need sending immediately
    sp<AMessage> refreshPointNotify = mNotify->dup();
    refreshPointNotify->setInt32("what",kWhatRefreshPointReq);
    refreshPointNotify->post();
    ALOGI("%s -",__FUNCTION__);
    return;
}
void RTPSender::onProcessGenericNACK(const sp<ABuffer> nack_fcis)
{
    ALOGI("%s +",__FUNCTION__);

    if(!mStarted) {
        ALOGW("%s,RTPSender not started",__FUNCTION__);
        return;
    }

    if(mAdaInfo->processGenericNACK(nack_fcis->data(), nack_fcis->size()) == true) {
        sp<AMessage> refreshPointNotify = mNotify->dup();
        refreshPointNotify->setInt32("what",kWhatForceIntraPicture);
        refreshPointNotify->post();
    }

    return;
}

void RTPSender::onProcessTMMBR(const sp<ABuffer> tmmbr_fci)
{
    ALOGV("%s +",__FUNCTION__);

    uint32_t target = 0;
    bool nRecord = false;
    bool active = false;
    uint32_t durationUs = requestRTPQueueDuration();

    signalInfo sigInfo;
    sigInfo.ratio = 100;
    sigInfo.needRecord = &nRecord;
    sigInfo.maxBitrate = getMaxBitrate();
    sigInfo.active = &active;

    //property map
    char value[PROPERTY_VALUE_MAX];
    int RTPMap = 0;
    int TestMode = 0;

    if(property_get("persist.vendor.vt.OPTest_MM", value, NULL)) {
        TestMode = atoi(value);
    }

    memset(value, 0, sizeof(value));
    if(property_get("persist.vendor.vt.OPTest_RTP", value, NULL)) {
        RTPMap = strtol(value,NULL,16);
    }

    if((TestMode && (RTPMap & 0x1)) || mConfigParam.network_info.interface_type){
        sigInfo.canExpand = false;
        ALOGD("%s, not over LTE %d, or test mode 0x1=%d:%d not expand",
              __FUNCTION__, mConfigParam.network_info.interface_type, TestMode, (RTPMap & 0x1));
    }else{
        sigInfo.canExpand = ifRiseBitrateBaseSignal(&sigInfo.ratio);
    }

    bool isAdjust = mAdaInfo->processTMMBR(tmmbr_fci->data(), &target, durationUs, sigInfo);

    if(isAdjust == true) {
        notifyAdjustEncBR(target);
        if(*sigInfo.active == true){
            mLastReduceSignal = ImsSignal::Signal_STRENGTH_NONE_OR_UNKNOWN;
        }
    }

    if(nRecord == true){
        recordSignal();
    }

    //react TMMBN
    sp<ABuffer> tmmbn_fci = new ABuffer(4);
    //cmcc spec need tmmbn exp,mantissa equal with tmmbr
    //ToDo,need check whether need change the item in tmmbn_fci
    //ToDo:if we didn't really adjust the encoding bit rate, whether we should not response TMMBN
    memset(tmmbn_fci->data(),0,4);
    memcpy(tmmbn_fci->data(),tmmbr_fci->data(),4);

    sp<AMessage> tmmbn_notify = mNotify->dup();
    tmmbn_notify->setInt32("what",kWhatSendTMMBN);
    tmmbn_notify->setBuffer("tmmbn_fci",tmmbn_fci);
    tmmbn_notify->post();

    ALOGV("%s -",__FUNCTION__);
    return;
}


void RTPSender::updateStatisticInfo(sp<ABuffer> rtpPacket)
{
    sp<AMessage> rtp_meta = rtpPacket->meta();

    int64_t timeUs = 0;
    rtp_meta->findInt64("timeUs",&timeUs);

    size_t payload_size = 0;
    rtp_meta->findSize("payload_size",&payload_size);

    mAdaInfo->updateStatisticInfo(timeUs, payload_size, rtpPacket->size());

    return;
}

void RTPSender::updateEncBitRate(const sp<ABuffer> accessUnit)
{
    sp<AMessage> accu_meta = accessUnit->meta();

    int64_t timeUs = 0;
    accu_meta->findInt64("timeUs",&timeUs);
    uint32_t size = accessUnit->size();
    mAdaInfo->updateEncBitRate(timeUs, size);

    return;
}

void RTPSender::notifyAdjustEncBR(uint32_t uiNewBR)
{
    if(!mStarted) {
        ALOGW("%s,Not Started!",__FUNCTION__);
        return;
    }

    sp<AMessage> notify = mNotify->dup();
    notify->setInt32("what",kWhatAdjustEncBitRate);
    notify->setInt32("netBitRate",uiNewBR);
    notify->post();

    return;
}

uint32_t RTPSender::requestRTPQueueDuration()
{
    uint32_t durationUs = 0;

    uint32_t minNum = mAdaInfo->getRTPQueueNumHighWater();

    if(mRTPPacketQueue.size() > minNum) {
        sp<ABuffer> beginRTP = * (mRTPPacketQueue.begin());
        sp<ABuffer> endRTP = * (--mRTPPacketQueue.end());

        int64_t beginTimeUs = 0;
        int64_t endTimeUs = 0;
        (beginRTP->meta())->findInt64("timeUs",&beginTimeUs);
        (endRTP->meta())->findInt64("timeUs",&endTimeUs);

        durationUs = endTimeUs - beginTimeUs;
    }

    ALOGD("%s,rtp queue size(%zu) durationUs(%d)",__FUNCTION__,mRTPPacketQueue.size(), durationUs);

    return durationUs;
}

uint32_t RTPSender::getRTPSentCount()
{
    ALOGV("%s",__FUNCTION__);

    sp<AMessage> msg = new AMessage(kWhatGetRTPSentCount, this);
    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);

    uint32_t uRTPSentCount = 0;
    response->findInt32("RTPSentCount", (int32_t*) &uRTPSentCount);

    return uRTPSentCount;
}

uint32_t RTPSender::getMaxBitrate(){
    uint32_t uiLast_MBR_UL_NetBR = (uint64_t)mConfigParam.network_info.MBR_UL * 1000 * 95 / 100;
    uint32_t uiLast_AS_NetBR = (uint64_t)mConfigParam.rtp_packet_bandwidth *1000;

    return (uiLast_MBR_UL_NetBR > uiLast_AS_NetBR ? uiLast_AS_NetBR : uiLast_MBR_UL_NetBR);
}

bool RTPSender::ifRiseBitrateBaseSignal(int32_t *ratio){
    *ratio = 100;

    bool expand = false;
    bool stable = false;
    int32_t sigLong = ImsSignal::Signal_STRENGTH_NONE_OR_UNKNOWN;
    int32_t sigShort = ImsSignal::Signal_STRENGTH_NONE_OR_UNKNOWN;

    sp<ImsSignal> mSignal = getDefaultImsSignal();
    stable = mSignal->getSignal(mSimID, &sigLong,&sigShort);

    if(stable){
        if(sigLong != ImsSignal::Signal_STRENGTH_NONE_OR_UNKNOWN
            && sigLong > mLastReduceSignal
            && sigLong >= ImsSignal::Signal_STRENGTH_GOOD
            && mLastReduceSignal != ImsSignal::Signal_STRENGTH_NONE_OR_UNKNOWN){
            *ratio =  mSignal->signalToBitrateRatio(sigLong);
            expand = true;
        }
    }

    ALOGD("ifRiseBitrateBaseSignal stable=%d ratio=%d expand=%d mLastReduceSignal=%d sigLong=%d",
          stable, *ratio, expand, mLastReduceSignal, sigLong);

    deleteDefaultImsSignal();

    return expand;
}


} //namespace


