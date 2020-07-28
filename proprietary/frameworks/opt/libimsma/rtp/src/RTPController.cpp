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
 * MediaTek Inc. (C) 2014. All rights reserved.
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

/*****************************************************************************
*
* Filename:
* ---------
* RTPController.cpp
*
* Project:
* --------
*   Android
*
* Description:
* ------------
*   RTP module controller implement for ViLTE
*
* Author:
* -------
*   Haizhen.Wang(mtk80691)
*
****************************************************************************/
//#define LOG_NDEBUG 0
#include <utils/Log.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "RTPController.h"
#include <inttypes.h>
#define ATRACE_TAG ATRACETAG_VIDEO
#include <utils/Trace.h>

#include <arpa/inet.h>
#include <sys/socket.h>

#include <cutils/properties.h>
#undef LOG_TAG
#define LOG_TAG "[VT][RTP]RTPController"

using namespace imsma;
using android::status_t;

namespace android
{

//Need Check: whether need to release the memory for mpVideoCapParams,mpAudioCapParams
RTPController::RTPController(uint32_t simID,uint32_t operatorID)
{
//memset the following member to 0
    /*
        rtp_rtcp_capability_t mVideoCapParams;
        rtp_rtcp_capability_t mAudioCapParams;
        rtp_rtcp_capability_t mVideoConfigParam;
        rtp_rtcp_capability_t mAudioConfigParam;

    */
    ALOGI("%s",__FUNCTION__);

    mReflector = new AHandlerReflector<RTPController> (this);

    memset(&mVideoConfigParam,0,sizeof(mVideoConfigParam));
    memset(&mAudioConfigParam,0,sizeof(mAudioConfigParam));

    //mSSRC = imsma_sourceID;

    //use rand to produce a ssrc
    //we add a random variable address as offset for increase randomness
    //in fact this is a simple treatment, but it's enough in p2p
    srand(time(0));
    uint32_t rd_num = rand();
    uint32_t variable_offset;
    ALOGI("variable address=0x%p", &variable_offset);
    uint32_t sourceID = (uint32_t)((long)(&variable_offset) & 0xffffffff);
    mSSRC = rd_num + sourceID;
    ALOGI("rd=0x%x offset=0x%x ssrc=0x%x", rd_num, sourceID, mSSRC);

    mVideoPeerSSRC_set = false;
    mVideoPeerSSRC = 0;
    mHandoverState = 0;
    mDowngradeEnable = false;
    mRTCPTimer = false;
    mReceiveRTCPPacket = false;
    mRTCPTimerInterval = 20; //defualt 20s
    mSupportTMMBR = false;
    mNoRTPFlag = false;
    mSimID = simID;
    mOperatorID = operatorID;

    mRecLatency = false;
    mLastEncBitrate = 0;;
    mNoRTCPCount = 0;

    mMTUSize = imsma_rtp_MTU_size;

    //Video RTCP related params
    mLastRTCP_videoRTPSentCount = 0;
    m_Trr_interval = 1000000ll; //default control the regular rtcp send interval to 1s
    m_AllowEarlyRtcp = true;
    mRTCP_tp = 0; //tp; used for calcuate the next RTCP
    m_Trr_last = 0;
    m_avg_rtcp_size = 52;//initial value

    mStartedRTCPSchedule = false;
    mVideoSendRTCPGeneration = 0;

    mFIRLastSeqNum = 0; //ToDo:change to random value

    mActiveTrack = 0;
    mVideoAddedPath = 0;
    mAudioAddedPath = 0;

    mAddedTrack = 0;
    mVideoActivePath = 0;
    mAudioActivePath = 0;

    //for adaptation
    m_isWaitingTMMBN = false;
    //m_isNWIndication = false;
#ifdef DEBUG_ENABLE_ADAPTATION
    enable_adaptation = true;

    char adp_param[PROPERTY_VALUE_MAX];
    memset(adp_param,0,sizeof(adp_param));

    if(property_get("vendor.vt.imsma.adp_enable", adp_param, NULL)) {
        int _enable_adp = atoi(adp_param);

        if(_enable_adp > 0) {
            enable_adaptation = true;
        } else {
            enable_adaptation = false;
        }
    }

    ALOGD("enable_adaptation =%d", enable_adaptation);
#endif

    struct sockaddr_in addr;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    mCName.setTo("android@");
    mCName.append(inet_ntoa(addr.sin_addr));

#ifdef DEBUG_DUMP_RTCP_PACKET
    mDumpRTCPPacket = 0ll;// ToDo: 1 not work
    mRecvRTCPFd = -1;
    mSendRTCPFd = -1;
    ALOGD("mDumpRTCPPacket =%" PRId64 "",mDumpRTCPPacket);

    char dump_param[PROPERTY_VALUE_MAX];
    memset(dump_param,0,sizeof(dump_param));

    //int64_t dump_value;
    if(property_get("vendor.vt.imsma.dump_rtcp_packet", dump_param, NULL)) {
        mDumpRTCPPacket = atol(dump_param);
        ALOGD("dump rtcp packet =%" PRId64 "", mDumpRTCPPacket);
    }

    if(mDumpRTCPPacket > 0) {
        const char* recv_rtcp_filename = "/sdcard/recv_rtcp_data.raw";
        mRecvRTCPFd = open(recv_rtcp_filename, O_CREAT | O_LARGEFILE | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
        ALOGD("open %s,recv_rtcpFd(%d)",recv_rtcp_filename,mRecvRTCPFd);

        const char* send_rtcp_filename = "/sdcard/send_rtcp_data.raw";
        mSendRTCPFd = open(send_rtcp_filename, O_CREAT | O_LARGEFILE | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
        ALOGD("open %s,send_rtcpFd(%d)",send_rtcp_filename,mSendRTCPFd);
    }

#endif
}
RTPController::~RTPController()
{
    ALOGI("%s",__FUNCTION__);

    //SocketWrapper will has lock between setRxCallback and call videoRTPPacketCallBack
    // So RTPController can be released safely even there is a callback from SocketWrapper
    if(mVideoRTCPSocketWrapper.get()) {
        mVideoRTCPSocketWrapper->setRxCallBack(0,0);
        mVideoRTCPSocketWrapper = NULL;
    }

    if(mVideoRTPSocketWrapper.get()) {
        mVideoRTPSocketWrapper->setRxCallBack(0,0);
        mVideoRTPSocketWrapper = NULL;

    }

    //if looper hasn't unregisterHandler
    //the looper and AHandler may be can not be released even RTPController released
    if(mReceiverLooper.get()) {
        ALOGI("stop RTPReceiver looper");
        mReceiverLooper->stop();
        mReceiverLooper->unregisterHandler(mRTPReceiver->id());
        mReceiverLooper = NULL;
    }

    mRTPReceiver = NULL;

    if(mVideoSenderLooper.get()) {
        ALOGI("stop RTPSender_Video looper");
        mVideoSenderLooper->stop();
        mVideoSenderLooper->unregisterHandler(mVideoRTPSender->id());
        mVideoSenderLooper = NULL;
    }

    mVideoRTPSender = NULL;

    //ALooper ALooperRoster has lock
    //so the message handling in onMessagereceive will be complete
    //before unregisterhandler can return
    //so caller can release RTPController safely even there are messages receving from other thread
    if(mLooper.get()) {
        ALOGI("stop RTPController looper");
        mLooper->stop();
        mLooper->unregisterHandler(mReflector->id());
        mLooper = NULL;
    }

#ifdef DEBUG_DUMP_RTCP_PACKET

    if(mRecvRTCPFd >= 0) {
        ALOGD("close recv_rtcpFd(%d)",mRecvRTCPFd);
        close(mRecvRTCPFd);
        mRecvRTCPFd = -1;
    }

    if(mSendRTCPFd >= 0) {
        ALOGD("close send_rtcpFd(%d)",mSendRTCPFd);
        close(mSendRTCPFd);
        mSendRTCPFd = -1;
    }

#endif

}

status_t RTPController::setEventNotify(const sp<AMessage> &eventNotify)
{
    ALOGI("%s",__FUNCTION__);

    if(!mLooper.get()) {
        //new looper
        ALOGI("create & start RTPController looper");
        mLooper = new ALooper;
        mLooper->setName("RTPController");
        mLooper->registerHandler(mReflector);
        mLooper->start();
    }

    sp<AMessage> msg = new AMessage(kWhatSetEventNotify, mReflector);
    msg->setMessage("notify",eventNotify);

    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);

    return err;
}

#if 0
status_t RTPController::setSocketFds(Vector<uint32_t> fds)
{
    ALOGI("%s",__FUNCTION__);
    Mutex::Autolock autoLock(mSocketFdsLock);
    mSocketfds = fds;
}
#endif

status_t RTPController::setConfigParams(rtp_rtcp_config_t* pRTPNegotiatedParams,uint8_t uiMediaType)
{
    ALOGV("%s",__FUNCTION__);

    if(!mLooper.get()) {
        //new looper
        ALOGI("create & start RTPController looper");
        mLooper = new ALooper;
        mLooper->setName("RTPController");
        mLooper->registerHandler(mReflector);
        mLooper->start();
    }

    ALOGV("[setConfigParams],fd0:%d,fd1:%d",\
          (pRTPNegotiatedParams->network_info).socket_fds[0], (pRTPNegotiatedParams->network_info).socket_fds[1]);

    ALOGV("[setConfigParams],fd2:%d,fd3:%d",\
          (pRTPNegotiatedParams->network_info).socket_fds[2], (pRTPNegotiatedParams->network_info).socket_fds[3]);

    sp<AMessage> msg = new AMessage(kWhatSetConfigParams, mReflector);
    msg->setPointer("config_params",pRTPNegotiatedParams);
    msg->setInt32("media_type",uiMediaType);

    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);

    return err;
}

status_t RTPController::updateConfigParams(rtp_rtcp_config_t* pRTPNegotiatedParams,uint8_t uiMediaType)
{
    ALOGV("%s",__FUNCTION__);

    if(!mLooper.get()) {
        //new looper
        ALOGI("create & start RTPController looper");
        mLooper = new ALooper;
        mLooper->setName("RTPController");
        mLooper->registerHandler(mReflector);
        mLooper->start();
    }

    sp<AMessage> msg = new AMessage(kWhatUpdateConfigParmas, mReflector);
    msg->setPointer("config_params",pRTPNegotiatedParams);
    msg->setInt32("media_type",uiMediaType);

    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);

    return err;
}

status_t RTPController::setAccuNotify(const sp<AMessage>& accessUnitNotify,uint8_t uiMediaType)
{
    ALOGI("%s,MediaType = %d",__FUNCTION__,uiMediaType);


    if(!mLooper.get()) {
        //new looper
        ALOGI("create & start RTPController looper");
        mLooper = new ALooper;
        mLooper->setName("RTPController");
        mLooper->registerHandler(mReflector);
        mLooper->start();
    }

    sp<AMessage> msg = new AMessage(kWhatSetAccuNotify, mReflector);
    msg->setMessage("notify",accessUnitNotify);
    msg->setInt32("media_type",uiMediaType);

    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);

    return err;

}

status_t RTPController::addStream(uint8_t rtpPath, uint8_t trackIndex)
{
    if(!mLooper.get()) {
        //new looper
        ALOGI("create & start RTPController looper");
        mLooper = new ALooper;
        mLooper->setName("RTPController");
        mLooper->registerHandler(mReflector);
        mLooper->start();
    }

    //Mutex::Autolock autoLock(mLock);
    ALOGI("%s,path = %d, track = %d",__FUNCTION__,rtpPath ,trackIndex);
    sp<AMessage> msg = new AMessage(kWhatAddStream, mReflector);
    msg->setInt32("rtpPath", rtpPath);
    msg->setInt32("trackIndex",trackIndex);
    //msg->post();
    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);
    return err;
}
status_t RTPController::removeStream(uint8_t rtpPath, uint8_t trackIndex)
{
    ALOGI("%s,path = %d, track = %d",__FUNCTION__,rtpPath ,trackIndex);
    sp<AMessage> msg = new AMessage(kWhatRemoveStream, mReflector);
    msg->setInt32("rtpPath", rtpPath);
    msg->setInt32("trackIndex",trackIndex);
    //msg->post();
    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);
    return err;

}

//Need Check
//whether start,pause should be Asyn?
//because QueueAccu,queuePacket is runing in MA thread
//if MA start RTPController and queueAccu ,but may be RTPController has not started
//add wait start commpleted message?
status_t RTPController::start(uint8_t rtpPath, uint8_t trackIndex)
{
    ALOGI("%s,path = %d, track = %d",__FUNCTION__,rtpPath ,trackIndex);

    if(!mLooper.get()) {
        //new looper
        ALOGI("create & start RTPController looper");
        mLooper = new ALooper;
        mLooper->setName("RTPController");
        mLooper->registerHandler(mReflector);
        mLooper->start();
    }

    //Mutex::Autolock autoLock(mLock);
    sp<AMessage> msg = new AMessage(kWhatStart, mReflector);
    msg->setInt32("rtpPath", rtpPath);
    msg->setInt32("trackIndex",trackIndex);
    //msg->post();

    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);

    return err;
    //Need Check
    //wait response

}
status_t RTPController::peerPausedSendStream(uint8_t trackIndex)
{
    ALOGI("%s,track = %d",__FUNCTION__,trackIndex);

    sp<AMessage> msg = new AMessage(kWhatPeerPauseStream, mReflector);
    msg->setInt32("trackIndex",trackIndex);
    msg->post();

    return OK;
}
status_t RTPController::peerResumedSendStream(uint8_t trackIndex)
{
    ALOGI("%s,track = %d",__FUNCTION__,trackIndex);

    sp<AMessage> msg = new AMessage(kWhatPeerResumeStream, mReflector);
    msg->setInt32("trackIndex",trackIndex);
    msg->post();

    return OK;

}
status_t RTPController::setVideoOrientation(uint8_t rotation,uint8_t camera_facing,uint8_t flip)
{
    ALOGI("%s,rotation = %d,camera_facing=%d,flip=%d",__FUNCTION__,rotation,camera_facing,flip);

    sp<AMessage> msg = new AMessage(kWhatSetCVOinfo, mReflector);
    msg->setInt32("rotation",rotation);
    msg->setInt32("face",camera_facing);
    msg->setInt32("flip",flip);

    msg->post();

    return OK;
}

#if 0
status_t RTPController::holdOn(bool isHoldOnEnable)
{
    ALOGI("%s,isHoldOnEnable = %d",__FUNCTION__,isHoldOnEnable);
    sp<AMessage> msg = new AMessage(kWhatHoldOn, mReflector);
    msg->setInt32("isHoldOn", isHoldOnEnable);
    msg->post();
    return OK;
}
#endif
//ToDo: whether need change to sync call of pause
/*
status_t RTPController::pause(uint8_t rtpPath, uint8_t trackIndex){
    ALOGI("%s,path = %d, track = %d",__FUNCTION__,rtpPath ,trackIndex);
    sp<AMessage> msg = new AMessage(kWhatPause, mReflector);
    msg->setInt32("rtpPath", rtpPath);
    msg->setInt32("trackIndex",trackIndex);
    //msg->setInt32("keepRtcp",bKeepRTCP? 1 : 0);
    msg->post();
    return OK;

}

status_t RTPController::resume(uint8_t rtpPath, uint8_t trackIndex){
    ALOGI("%s,path = %d, track = %d",__FUNCTION__,rtpPath ,trackIndex);
    sp<AMessage> msg = new AMessage(kWhatResume, mReflector);
    msg->setInt32("rtpPath", rtpPath);
    msg->setInt32("trackIndex",trackIndex);
    msg->post();
    return OK;
}
*/
status_t RTPController::stop(uint8_t rtpPath, uint8_t trackIndex)
{
    ALOGI("%s,path = %d, track = %d",__FUNCTION__,rtpPath ,trackIndex);
    //Mutex::Autolock autoLock(mLock);
    sp<AMessage> msg = new AMessage(kWhatStop, mReflector);
    msg->setInt32("rtpPath", rtpPath);
    msg->setInt32("trackIndex",trackIndex);
    //msg->post();

    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);

    return err;

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

status_t RTPController::queueAccessUnit(const sp<ABuffer> &buffer,uint8_t trackIndex)
{
    if(!mLooper.get()) {
        ALOGE("%s,looper not start",__FUNCTION__);
    }

    sp<AMessage> msg = new AMessage(kWhatQueueAccu, mReflector);
    msg->setBuffer("accu",buffer);
    msg->setInt32("trackIndex",trackIndex);
    msg->post();
    return OK;
}

#if 0
void  RTPController::videoRTPPacketCallBack(void* cookie, int msg,const sp<ABuffer> buffer)
{

    ALOGI("%s,path = %d, track = %d",__FUNCTION__,rtpPath ,trackIndex);
    RTPController* rtpCon = static_cast<RTPController*>(cookie);

    if(rtpCon == NULL) {
        return;
    }

    sp<AMessage> msg = new AMessage(kWhatRTPPacket, rtpCon->id());
    msg->setInt32("", rtpPath);
    msg->setInt32("trackIndex",trackIndex);
    msg->post();
}

#endif
int  RTPController::videoRTCPPacketCallBack(void* cookie,const sp<ABuffer>& buffer)
{
    ALOGD("%s",__FUNCTION__);

    RTPController* rtpCon = static_cast<RTPController*>(cookie);

    if(rtpCon == NULL) {
        ALOGW("%s,cookie = NULL",__FUNCTION__);
        return UNKNOWN_ERROR;
    }

    sp<AMessage> msg = new AMessage(kWhatRTCPPacket, rtpCon->mReflector);
    msg->setInt32("trackIndex",IMSMA_RTP_VIDEO);
    // msg->setInt32("msg",msg);
    msg->setBuffer("packet",buffer);
    msg->post();
    return OK;
}

void RTPController::feedBackDecoderError(sp<AMessage> errInfo,uint8_t trackIndex)
{
    ALOGI("%s",__FUNCTION__);
    sp<AMessage> msg = new AMessage(kWhatFeedBackDecErr, mReflector);
    msg->setMessage("err_info",errInfo);
    msg->setInt32("trackIndex",trackIndex);
    msg->post();
}

#if 0
void RTPController::requestDecoderRefreshPoint(uint8_t trackIndex)
{
    ALOGI("%s",__FUNCTION__);
    sp<AMessage> msg = new AMessage(kWhatSendFIR, mReflector->id());
    msg->setInt32("trackIndex",trackIndex);
    msg->post();
}
#endif
void RTPController::recvDecoderRefreshPoint(uint8_t trackIndex)
{
    ALOGI("%s",__FUNCTION__);
    sp<AMessage> msg = new AMessage(kWhatRecvCSD, mReflector);
    msg->setInt32("trackIndex",trackIndex);
    msg->post();
}
#if 0
void RTPController::sliceLossIndication(uint16_t firstMB,uint16_t number,uint8_t pictureID)
{
    ALOGI("%s",__FUNCTION__);
    sp<AMessage> msg = new AMessage(kWhatSendSLI, mReflector->id());
    //msg->setInt32("trackIndex",trackIndex);
    msg->setInt32("firstMB",firstMB);
    msg->setInt32("number",number);
    msg->setInt32("pictureID",pictureID);
    msg->post();
}

void RTPController::sliceLossIndication(const Vector<sli_info>& sli_info)
{
    ALOGI("%s",__FUNCTION__);
    sp<AMessage> msg = new AMessage(kWhatSendSLI, mReflector->id());
    //msg->setInt32("trackIndex",trackIndex);
    //msg->setInt32("sli_info",&sli_info);
    //for build pass
    uint32_t size = sli_info.size();

    msg->post();
}
#endif

void RTPController::onMessageReceived(const sp<AMessage> &msg)
{
    switch(msg->what()) {
    case kWhatSetEventNotify: {
        ALOGI("kWhatSetEventNotify");
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));

        sp<AMessage> eventNotify;
        msg->findMessage("notify", &eventNotify);

        mNotify = eventNotify;

        sp<AMessage> response = new AMessage;
        response->setInt32("err", OK);

        response->postReply(replyID);

        break;
    }
    case kWhatSetConfigParams: {
        ALOGI("kWhatSetConfigParams");
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));

        rtp_rtcp_config_t* pRTPNegotiatedParams = NULL;
        int32_t  uiMediaType = IMSMA_RTP_VIDEO;

        msg->findPointer("config_params", (void**) &pRTPNegotiatedParams);
        msg->findInt32("media_type",&uiMediaType);

        status_t err = onSetConfigParams(pRTPNegotiatedParams, (uint8_t) uiMediaType);

        sp<AMessage> response = new AMessage;
        response->setInt32("err", err);

        response->postReply(replyID);

        break;
    }
    case kWhatUpdateConfigParmas: {
        ALOGI("kWhatUpdateConfigParmas");
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));

        rtp_rtcp_config_t* pRTPNegotiatedParams = NULL;
        int32_t uiMediaType = IMSMA_RTP_VIDEO;

        msg->findPointer("config_params", (void**) &pRTPNegotiatedParams);
        msg->findInt32("media_type",&uiMediaType);

        status_t err = onUpdateConfigParams(pRTPNegotiatedParams, (uint8_t) uiMediaType);

        sp<AMessage> response = new AMessage;
        response->setInt32("err", err);

        response->postReply(replyID);
        break;
    }
    case kWhatSetAccuNotify: {
        ALOGI("kWhatSetAccuNotify");
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));

        sp<AMessage> accuNotify;
        msg->findMessage("notify", &accuNotify);
        CHECK(accuNotify.get());

        int32_t uiMediaType = IMSMA_RTP_VIDEO;
        msg->findInt32("media_type",&uiMediaType);


        if(uiMediaType == IMSMA_RTP_VIDEO) {
            mVideoAccessUnitNotify = accuNotify;
        } else if(uiMediaType == IMSMA_RTP_AUDIO) {
            mAudioAccessUnitNotify = accuNotify;

        } else {
            ALOGW("unsupported media 0x%x",uiMediaType);
        }

        sp<AMessage> response = new AMessage;
        response->setInt32("err", OK);

        response->postReply(replyID);
        break;
    }
    case kWhatAddStream: {
        ALOGI("kWhatAddStream");
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));

        int32_t rtpPath = IMSMA_RTP_UP_DOWN_LINK;
        int32_t trackIndex = IMSMA_RTP_VIDEO;
        msg->findInt32("rtpPath", &rtpPath);
        msg->findInt32("trackIndex",&trackIndex);

        status_t err = onAddStream(rtpPath,trackIndex);

        sp<AMessage> response = new AMessage;
        response->setInt32("err", err);

        response->postReply(replyID);

        break;
    }
    case kWhatRemoveStream: {
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));

        int32_t rtpPath = IMSMA_RTP_UP_DOWN_LINK;
        int32_t trackIndex = IMSMA_RTP_VIDEO;
        msg->findInt32("rtpPath", &rtpPath);
        msg->findInt32("trackIndex",&trackIndex);

        status_t err = onRemoveStream(rtpPath,trackIndex);

        sp<AMessage> response = new AMessage;
        response->setInt32("err", err);

        response->postReply(replyID);

        break;
    }
    case kWhatStart: {
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));

        int32_t rtpPath = IMSMA_RTP_UP_DOWN_LINK;
        int32_t trackIndex = IMSMA_RTP_VIDEO;
        msg->findInt32("rtpPath", &rtpPath);
        msg->findInt32("trackIndex",&trackIndex);
        status_t err = onStart(rtpPath,trackIndex);

        sp<AMessage> response = new AMessage;
        response->setInt32("err", err);

        response->postReply(replyID);
        break;
    }
    case kWhatStop: {
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));

        int32_t rtpPath = IMSMA_RTP_UP_DOWN_LINK;
        int32_t trackIndex = IMSMA_RTP_VIDEO;
        msg->findInt32("rtpPath", &rtpPath);
        msg->findInt32("trackIndex",&trackIndex);
        status_t err = onStop(rtpPath,trackIndex);
        //ToDo: AMessage without what?
        sp<AMessage> response = new AMessage;
        response->setInt32("err", err);

        response->postReply(replyID);
        break;
    }
    case kWhatSetHoState: {
        int32_t state = 0;
        msg->findInt32("state", &state);

        if(state != 0) {
            mHandoverState++;
        } else {
            mHandoverState--;
        }

        ALOGD("kWhatSetHoState state=%d mHandoverState=%d", state, mHandoverState);

        break;
    }
    /*
    case kWhatPause:
    {
        uint8_t rtpPath = IMSMA_RTP_UP_DOWN_LINK;
        uint8_t trackIndex = IMSMA_RTP_VIDEO;
        int32_t iKeepRTCP = 1;

        int32_t iRtpPath = IMSMA_RTP_UP_DOWN_LINK;
        int32_t iTrackIndex = IMSMA_RTP_VIDEO;
        msg->findInt32("rtpPath", &iRtpPath);
        msg->findInt32("trackIndex",&iTrackIndex);
        msg->findInt32("keepRtcp",&iKeepRTCP);

        rtpPath = (uint8_t)iRtpPath;
        trackIndex = (uint8_t)iTrackIndex;
        //onPause( iKeepRTCP >= 1 ? true:false,rtpPath,trackIndex);
        onPause(rtpPath,trackIndex);
        //ToDo:
        // Check the new ip address and b=rr,b=rs to decide whether to pause RTCP

        break;
    }
    case kWhatResume:
    {
        uint8_t rtpPath = IMSMA_RTP_UP_DOWN_LINK;
        uint8_t trackIndex = IMSMA_RTP_VIDEO;

        int32_t iRtpPath = IMSMA_RTP_UP_DOWN_LINK;
        int32_t iTrackIndex = IMSMA_RTP_VIDEO;
        msg->findInt32("rtpPath", &iRtpPath);
        msg->findInt32("trackIndex",&iTrackIndex);

        rtpPath = (uint8_t)iRtpPath;
        trackIndex = (uint8_t)iTrackIndex;
        onResume(rtpPath,trackIndex);
        break;
    }
    */
    case kWhatQueueAccu: {
        int32_t trackIndex = IMSMA_RTP_VIDEO;
        msg->findInt32("trackIndex",&trackIndex);
        sp<ABuffer> buffer;
        msg->findBuffer("accu",&buffer);

        if(trackIndex == IMSMA_RTP_VIDEO) {
            if((mActiveTrack & IMSMA_RTP_VIDEO) && (mVideoActivePath & IMSMA_RTP_UPLINK)) {
                if(mVideoRTPSender.get()) {
                    mVideoRTPSender->queueAccessUnit(buffer);
#if 0
                    mUPaccuCount++;

                    if(mUPaccuCount == 1) {
                        startScheduleRTCP(trackIndex);
                    }

#endif
                }
            } else {
                ALOGE("kWhatQueueAccu,uplink video not active!!!");
            }
        }

        break;
    }
    case kWhatPeerPauseStream: {
        int32_t trackIndex = IMSMA_RTP_VIDEO;
        msg->findInt32("trackIndex",&trackIndex);

        if(mRTPReceiver.get()) {
            mRTPReceiver->peerPausedSendStream(trackIndex);
        }

        break;
    }
    case kWhatPeerResumeStream: {
        int32_t trackIndex = IMSMA_RTP_VIDEO;
        msg->findInt32("trackIndex",&trackIndex);

        if(mRTPReceiver.get()) {
            mRTPReceiver->peerResumedSendStream(trackIndex);

            //we always send FIR for avoid resume late issue
            onSendFIR();
        }

        break;
    }
#if 0
    case kWhatHoldOn: {
        int32_t isHoldOnEnable = 0;
        msg->findInt32("isHoldOn", &isHoldOnEnable);
        onHoldOn(isHoldOnEnable);
    }
#endif
    case kWhatSenderNotify: {
        //ToDo:
        //handle video/audio track notify by trackIndex
        int32_t trackIndex = IMSMA_RTP_VIDEO;
        msg->findInt32("trackIndex",&trackIndex);

        int32_t what = 0;
        msg->findInt32("what",&what);

        if(trackIndex == IMSMA_RTP_VIDEO) {
            if(what == RTPSender::kWhatRefreshPointReq) {
                ALOGI("RTPSender::kWhatRefreshPointReq received");
                sp<AMessage> refreshPointReq = mNotify->dup();
                refreshPointReq->setInt32("what",kWhatFIR);
                refreshPointReq->post();
            }

            if(what == RTPSender::kWhatStartSR) {
                ALOGI("RTPSender::kWhatStartSR received");
                startScheduleRTCP(trackIndex);
            }

            if(what == RTPSender::kWhatForceIntraPicture) {
                ALOGI("RTPSender::kWhatForceIntraPicture received");
                //encoder handle this same as PLI
                //notify to MA
                sp<AMessage> notify = mNotify->dup();
                notify->setInt32("what",kWhatPLI);
                notify->post();
            }

            if(what == RTPSender::kWhatSendTMMBN) {
                ALOGI("RTPSender::kWhatSendTMMBN received");
                sp<ABuffer> tmmbn_fci;
                msg->findBuffer("tmmbn_fci",&tmmbn_fci);

                onSendTMMBN(tmmbn_fci);
            }

            if(what == RTPSender::kWhatAdjustEncBitRate) {
                int32_t uiEncBitRate = 0;
                msg->findInt32("netBitRate",&uiEncBitRate);
                ALOGD("RTPSender::kWhatAdjustEncBitRate,netBitRate = %d bps",uiEncBitRate);

                if(mHandoverState > 0) {
                    ALOGW("mHandoverState=%d we can't adjust bitrate", mHandoverState);
                    return;
                }

#ifdef DEBUG_ENABLE_ADAPTATION
                ALOGD("RTPSender::kWhatAdjustEncBitRate,enable_adaptation=%d",enable_adaptation);

                if(enable_adaptation) {
#endif
                    sp<AMessage> notify = mNotify->dup();
                    notify->setInt32("what",kWhatAdjustEncBitRate);
                    notify->setInt32("netBitRate",uiEncBitRate);
                    notify->post();
#ifdef DEBUG_ENABLE_ADAPTATION
                }

#endif
            }
        }

        //mVideoRtpPacketNotify

        break;
    }
    case kWhatReceiverNotify: {
        int32_t trackIndex = IMSMA_RTP_VIDEO;
        msg->findInt32("trackIndex",&trackIndex);

        int32_t what = 0;
        msg->findInt32("what",&what);

        if(what == RTPReceiver::kWhatStartRR) {
            ALOGI("RTPReceiver::kWhatStartRR received");
            startScheduleRTCP(trackIndex);
        } else if(what == RTPReceiver::kWhatGenericNACK) {
            if(mVideoConfigParam.rtp_profile == IMSMA_RTP_AVP) {
                ALOGW("kWhatReceiverNotify,AVP not support FBs");
                break;
            }

            sp<ABuffer> nack_fci;
            msg->findBuffer("nack_fci",&nack_fci);
            onSendGenericNack(nack_fci);
        } else if(what == RTPReceiver::kWhatTriggerPli) {
            if(mVideoConfigParam.rtp_profile == IMSMA_RTP_AVP) {
                ALOGW("kWhatReceiverNotify,AVP not support FBs");
                break;
            }

            onSendPLI();
        } else if(what == RTPReceiver::kWhatTriggerFir) {
            if(mVideoConfigParam.rtp_profile == IMSMA_RTP_AVP) {
                ALOGW("kWhatReceiverNotify,AVP not support FBs");
                break;
            }

            onSendFIR();
        }else if(what == RTPReceiver::kWhatSendTMMBR) {
            if(mVideoConfigParam.rtp_profile == IMSMA_RTP_AVP) {
                ALOGW("kWhatReceiverNotify,AVP not support TMMBR");
                break;
            }

            sp<ABuffer> tmmbr_fci;
            msg->findBuffer("tmmbr_fci",&tmmbr_fci);
            int32_t isReduce = true;
            msg->findInt32("isReduce",&isReduce);

            if(!tmmbr_fci.get()) {
                ALOGW("RTPReceiver::kWhatSendTMMBR,tmmbr_fci buffer is NULL !");
                break;
            }

            uint8_t* data = tmmbr_fci->data();
            ALOGD("RTPReceiver::kWhatSendTMMBR,isReduce=%d,tmmbr_fci=%p",\
                  isReduce, data);
#ifdef DEBUG_ENABLE_ADAPTATION
            ALOGD("RTPReceiver::kWhatSendTMMBR,enable_adaptation=%d",enable_adaptation);

            if(enable_adaptation) {
#endif
                onSendTMMBR(tmmbr_fci, (bool) isReduce);
#ifdef DEBUG_ENABLE_ADAPTATION
            }

#endif
        } else if(what == RTPReceiver::kWhatUpdateSSRC) {
            int32_t trackIndex =  IMSMA_RTP_VIDEO;
            msg->findInt32("trackIndex",&trackIndex);

            if(trackIndex ==  IMSMA_RTP_VIDEO) {
                mVideoPeerSSRC_set = true;
                msg->findInt32("peer_ssrc", (int32_t*) &mVideoPeerSSRC);
                ALOGD("RTPReceiver::kWhatUpdateSSRC,mVideoPeerSSRC = 0x%x",mVideoPeerSSRC);

                mDowngradeEnable = true;
                ALOGD("RTPReceiver::kWhatUpdateSSRC,set mDowngradeEnable true %d",mDowngradeEnable);
            }
        } else if(what == RTPReceiver::kWhatUpdateDebugInfo) {
            int32_t info = 0;
            msg->findInt32("info",&info);

            int32_t needNotify = 0;
            msg->findInt32("needNotify",&needNotify);
            mRecLatency = (needNotify == 0 ? false:true);

            sp<AMessage> notify = mNotify->dup();
            notify->setInt32("what",kWhatUpdateDebugInfo);
            notify->setInt32("info",info);
            notify->post();
        } else if(what == RTPReceiver::kWhatDropCall) {
            if(mHandoverState > 0) {
                ALOGD("RTPReceiver::kWhatDropCall mHandoverState=%d we can't drop call", mHandoverState);
            } else if(mDowngradeEnable == false)
                ALOGD("RTPReceiver::kWhatDropCall mDowngradeEnable=%d we can't drop call", mDowngradeEnable);
            else {
                sp<AMessage> notify = mNotify->dup();
                notify->setInt32("what",kWhatDropCall);
                notify->post();
            }
        } else if(what == RTPReceiver::kWhatNoRTP) {
            int32_t info = 0;
            msg->findInt32("info",&info);
            mNoRTPFlag = (info == 0 ? false:true);
            ALOGD("RTPReceiver::kWhatNoRTP,set mNoRTPFlag %d",mNoRTPFlag);
        } else if(what == RTPReceiver::kWhatAdjustEncBitRate) {
            int32_t uiEncBitRate = 0;
            msg->findInt32("netBitRate",&uiEncBitRate);
            ALOGD("RTPReceiver::kWhatAdjustEncBitRate,netBitRate = %d bps",uiEncBitRate);

            if(mHandoverState > 0) {
                ALOGW("mHandoverState=%d we can't adjust bitrate", mHandoverState);
                return;
            }

            if(mRecLatency == true && mLastEncBitrate != 0 && uiEncBitRate > mLastEncBitrate) {
                ALOGW("we can't adjust bitrate mRecLatency=%d new bitrate %d > %d",
                      mRecLatency, uiEncBitRate, mLastEncBitrate);
                return;
            }

#ifdef DEBUG_ENABLE_ADAPTATION
            ALOGD("RTPReceiver::kWhatAdjustEncBitRate,enable_adaptation=%d",enable_adaptation);

            if(enable_adaptation) {
#endif
                sp<AMessage> notify = mNotify->dup();
                notify->setInt32("what",kWhatAdjustEncBitRate);
                notify->setInt32("netBitRate",uiEncBitRate);
                notify->post();
                mLastEncBitrate = uiEncBitRate;
#ifdef DEBUG_ENABLE_ADAPTATION
            }

#endif
        }

        break;
    }
    //re-architecture
    case kWhatRTCPPacket: {
        int32_t trackIndex =  IMSMA_RTP_VIDEO;
        msg->findInt32("trackIndex",&trackIndex);

        if(trackIndex == IMSMA_RTP_VIDEO) {
            //int32_t msg_type = 0;
            //msg->findInt32("msg",&msg_type);

            //if(msg_type == packet){
            sp<ABuffer> packet;
            msg->findBuffer("packet",&packet);
            queueRTCPPacket(packet,IMSMA_RTP_VIDEO);
            //}
        }

        break;
    }
    case kWhatRTCPTimer: {
        if(mRTCPTimer == false) {
            ALOGD("RTCPTimer exit");
            break;
        }

        ALOGD("RTCPTimer mReceiveRTCPPacket=%d mNoRTPFlag=%d", mReceiveRTCPPacket, mNoRTPFlag);

        if(mReceiveRTCPPacket == false
                && (++mNoRTCPCount > mRTCPTimerInterval) && mNoRTPFlag) {
            if(mOperatorID != 8){
                ALOGE("RTCP Timer timeout %d s, we drop call", mRTCPTimerInterval);
                sp<AMessage> notify = mNotify->dup();
                notify->setInt32("what",kWhatDropCall);
                notify->post();
            }else{//TMO special handle
                ALOGE("RTCP Timer timeout %d s, mOperatorID=%d we don't drop call", mRTCPTimerInterval, mOperatorID);
            }

        } else {
            if(mReceiveRTCPPacket == true) {
                mNoRTCPCount = 0;
            }

            mReceiveRTCPPacket = false;
        }

        msg->post(1000000);

        break;
    }
    case kWhatSendRTCP: {
        int32_t generation = 0;
        msg->findInt32("generation",&generation);

        if(mVideoSendRTCPGeneration != generation) {
            ALOGD("overdue RTCP");
            break;
        }

        onSendRTCP(msg);
        break;
    }
    case kWhatFeedBackDecErr: {
        if(mVideoConfigParam.rtp_profile == IMSMA_RTP_AVP) {
            ALOGW("kWhatFeedBackDecErr,AVP not support FBs");
            break;
        }

        sp<AMessage> err_info;
        msg->findMessage("err_info",&err_info);

        int32_t err_type = 0;
        err_info->findInt32("err",&err_type);

        if(err_type == DecErr_PictureLossIndication) {
            onSendPLI();
        } else if(err_type == DecErr_SliceLossIndication) {
            sp<ABuffer> buffer;
            err_info->findBuffer("sli",&buffer);
            onSendSLI(buffer);
        } else if(err_type == DecErr_FullIntraRequest) {
            onSendFIR();
        } else {
            ALOGE("unknown err(%d)",err_type);
        }

        break;
    }
#if 0
    case kWhatSendFIR: {
        onSendFIR();
        break;
    }
#endif
    case kWhatRecvCSD: {
        int32_t trackIndex = IMSMA_RTP_VIDEO;
        msg->findInt32("trackIndex",&trackIndex);
        onReceiveCSD(trackIndex);

        break;
    }
#if 0
    case kWhatSendSLI: {
        //ToDo:
        //onSendSLI();
        break;
    }
#endif
    case kWhatSetCVOinfo: {
        int32_t rotation = 0;
        msg->findInt32("rotation",&rotation);

        int32_t camera_facing = 0;
        msg->findInt32("face",&camera_facing);

        int32_t flip = 0;
        msg->findInt32("flip",&flip);

        if(mVideoRTPSender.get()) {
            mVideoRTPSender->setVideoOrientation((uint8_t) rotation, (uint8_t) camera_facing, (uint8_t) flip);
        }

        break;
    }
    /*
        case kWhatSendTMMBR:
        {
            bool isNetworkIndication = true;
            int32_t mbr_dl;//kbps
            bool isReduce = true;

            //ToDo:Whether can get bool address this way
            msg->findInt32("isNW",&isNetworkIndication);
            msg->findInt32("mbr_dl",&mbr_dl);
            msg->findInt32("isReduce",&isReduce);

            sp<ABuffer> tmmbr_fci = new ABuffer(4);
            uint8_t* data = tmmbr_fci->data();
            //fit TMMBR with mbr_dl and overhead
            //........

            sp<AMessage> tmmbr_meta = tmmbr_fci->meta();
            tmmbr_meta->setInt32("isNW",isNetworkIndication);
            tmmbr_meta->setInt32("mbr_dl",mbr_dl);
            tmmbr_meta->setInt32("isReduce",isReduce);

            onSendTMMBR(tmmbr_fci);

            break;
        }*/
    default: {
        TRESPASS();
        break;
    }
    }

}

status_t RTPController::onSetConfigParams(rtp_rtcp_config_t* pRTPNegotiatedParams,uint8_t uiMediaType)
{
    if(uiMediaType == IMSMA_RTP_VIDEO) {
        ALOGD("%s,save video negotiated done params",__FUNCTION__);
        //save the negotiated done params of video
        mVideoConfigParam.media_type = pRTPNegotiatedParams->media_type;
        mVideoConfigParam.mime_Type = pRTPNegotiatedParams->mime_Type;
        mVideoConfigParam.rtp_payload_type = pRTPNegotiatedParams->rtp_payload_type;
        mVideoConfigParam.rtp_profile = pRTPNegotiatedParams->rtp_profile;
        mVideoConfigParam.sample_rate = pRTPNegotiatedParams->sample_rate;

        mVideoConfigParam.rtp_packet_bandwidth = pRTPNegotiatedParams->rtp_packet_bandwidth;
        mVideoConfigParam.packetize_mode = pRTPNegotiatedParams->packetize_mode;

        mVideoConfigParam.rtp_header_extension_num = pRTPNegotiatedParams->rtp_header_extension_num;
        uint8_t max_allow_ext_num = sizeof(mVideoConfigParam.rtp_ext_map) /sizeof(rtp_ext_map_t);
        ALOGD("\t ext num:%d,max allow ext num:%d",\
              mVideoConfigParam.rtp_header_extension_num,max_allow_ext_num);

        if(mVideoConfigParam.rtp_header_extension_num > max_allow_ext_num) {
            mVideoConfigParam.rtp_header_extension_num = max_allow_ext_num;
        }

        for(uint32_t j = 0; j < mVideoConfigParam.rtp_header_extension_num; j++) {
            mVideoConfigParam.rtp_ext_map[j].extension_id = (pRTPNegotiatedParams->rtp_ext_map[j]).extension_id;
            mVideoConfigParam.rtp_ext_map[j].direction = (pRTPNegotiatedParams->rtp_ext_map[j]).direction;
            memcpy(mVideoConfigParam.rtp_ext_map[j].extension_uri, (pRTPNegotiatedParams->rtp_ext_map[j]).extension_uri,\
                   sizeof((pRTPNegotiatedParams->rtp_ext_map[j]).extension_uri));

            ALOGD("\t ext id:%d,direction:%d,uri:%s",\
                  mVideoConfigParam.rtp_ext_map[j].extension_id,mVideoConfigParam.rtp_ext_map[j].direction,mVideoConfigParam.rtp_ext_map[j].extension_uri);
        }

        mVideoConfigParam.rtcp_sender_bandwidth = pRTPNegotiatedParams->rtcp_sender_bandwidth;
        mVideoConfigParam.rtcp_receiver_bandwidth = pRTPNegotiatedParams->rtcp_receiver_bandwidth;

        mVideoConfigParam.rtcp_reduce_size = pRTPNegotiatedParams->rtcp_reduce_size;
        mVideoConfigParam.rtcp_fb_param_num = pRTPNegotiatedParams->rtcp_fb_param_num;

        ALOGD("\t media(0x%x),mime(0x%x),PT(%d),profile(0x%x),sampleRate(%d)",\
              mVideoConfigParam.media_type,mVideoConfigParam.mime_Type,\
              mVideoConfigParam.rtp_payload_type,mVideoConfigParam.rtp_profile,mVideoConfigParam.sample_rate);

        ALOGD("\t b=AS(%d),packetize_mode(%d),b=RS(%" PRId64 "),b=RR(%" PRId64 "),rtcp_rsize(%d)",\
              mVideoConfigParam.rtp_packet_bandwidth,mVideoConfigParam.packetize_mode,\
              mVideoConfigParam.rtcp_sender_bandwidth,mVideoConfigParam.rtcp_receiver_bandwidth,mVideoConfigParam.rtcp_reduce_size);

        ALOGD("\t fb_num(%d)",mVideoConfigParam.rtcp_fb_param_num);

        m_Trr_interval = 1000000ll;

        bool bSupportTMMBR = false;

        for(uint8_t i = 0; i< mVideoConfigParam.rtcp_fb_param_num; i++) {
            mVideoConfigParam.rtcp_fb_type[i].rtcp_fb_id= pRTPNegotiatedParams->rtcp_fb_type[i].rtcp_fb_id;
            mVideoConfigParam.rtcp_fb_type[i].rtcp_fb_param = pRTPNegotiatedParams->rtcp_fb_type[i].rtcp_fb_param;

            memcpy(mVideoConfigParam.rtcp_fb_type[i].rtcp_fb_sub_param,pRTPNegotiatedParams->rtcp_fb_type[i].rtcp_fb_sub_param,sizeof(pRTPNegotiatedParams->rtcp_fb_type[i].rtcp_fb_sub_param));

            ALOGV("\t Feedback[%d]:fb_id(%d),fb_param(%d),fb_sub_param(%s)",\
                  i,mVideoConfigParam.rtcp_fb_type[i].rtcp_fb_id,mVideoConfigParam.rtcp_fb_type[i].rtcp_fb_param,\
                  mVideoConfigParam.rtcp_fb_type[i].rtcp_fb_sub_param);

            if(mVideoConfigParam.rtcp_fb_type[i].rtcp_fb_id == IMSMA_TRR_INT) {
                m_Trr_interval = mVideoConfigParam.rtcp_fb_type[i].rtcp_fb_param * 1000;//in Us
            }

            if(mVideoConfigParam.rtcp_fb_type[i].rtcp_fb_id == IMSMA_CCM && mVideoConfigParam.rtcp_fb_type[i].rtcp_fb_param == IMSMA_TMMBR) {
                bSupportTMMBR = true;
            }
        }

        if(bSupportTMMBR == true) {
            mSupportTMMBR =true;
        }

        ALOGI("\t m_Trr_interval=%" PRId64 "us  SupportTMMBR=%d : %d",m_Trr_interval, bSupportTMMBR, mSupportTMMBR);


        mVideoConfigParam.network_info.ebi = (pRTPNegotiatedParams->network_info).ebi;
        mVideoConfigParam.network_info.interface_type = (pRTPNegotiatedParams->network_info).interface_type;
        mVideoConfigParam.network_info.dscp= (pRTPNegotiatedParams->network_info).dscp;
        mVideoConfigParam.network_info.soc_priority= (pRTPNegotiatedParams->network_info).soc_priority;

        mVideoConfigParam.network_info.remote_rtp_port = (pRTPNegotiatedParams->network_info).remote_rtp_port;
        mVideoConfigParam.network_info.remote_rtcp_port = (pRTPNegotiatedParams->network_info).remote_rtcp_port;
        mVideoConfigParam.network_info.local_rtp_port = (pRTPNegotiatedParams->network_info).local_rtp_port;
        mVideoConfigParam.network_info.local_rtcp_port  = (pRTPNegotiatedParams->network_info).local_rtcp_port;

        ALOGI("\t NetworkInfo:ebi[%x],interface_type[%x],dscp:[%d],priority=[%d]",\
              mVideoConfigParam.network_info.ebi,mVideoConfigParam.network_info.interface_type,\
              mVideoConfigParam.network_info.dscp,mVideoConfigParam.network_info.soc_priority);

        ALOGI("\t NetworkInfo:remote port:[rtp:%d][rtcp:%d],local port:[rtp:%d][rtcp:%d]",\
              mVideoConfigParam.network_info.remote_rtp_port,mVideoConfigParam.network_info.remote_rtcp_port,\
              mVideoConfigParam.network_info.local_rtp_port,mVideoConfigParam.network_info.local_rtcp_port);

        memcpy(mVideoConfigParam.network_info.remote_rtp_address, (pRTPNegotiatedParams->network_info).remote_rtp_address,\
               sizeof((pRTPNegotiatedParams->network_info).remote_rtp_address));
        memcpy(mVideoConfigParam.network_info.remote_rtcp_address, (pRTPNegotiatedParams->network_info).remote_rtcp_address,\
               sizeof((pRTPNegotiatedParams->network_info).remote_rtcp_address));

        memcpy(mVideoConfigParam.network_info.local_rtp_address, (pRTPNegotiatedParams->network_info).local_rtp_address,\
               sizeof((pRTPNegotiatedParams->network_info).local_rtp_address));

        memcpy(mVideoConfigParam.network_info.local_rtcp_address, (pRTPNegotiatedParams->network_info).local_rtcp_address,\
               sizeof((pRTPNegotiatedParams->network_info).local_rtcp_address));

        mVideoConfigParam.network_info.remote_addr_type = (pRTPNegotiatedParams->network_info).remote_addr_type;
        mVideoConfigParam.network_info.local_addr_type  = (pRTPNegotiatedParams->network_info).local_addr_type;

        ALOGI("\t NetworkInfo:remote address type:%x",mVideoConfigParam.network_info.remote_addr_type);

        //printf remote ip address value
        if(mVideoConfigParam.network_info.remote_addr_type == ViLTE_IPv4) {
            ALOGI("\t NetworkInfo:remote rtp address IPv4:%d.%d.%d.%d",\
                  mVideoConfigParam.network_info.remote_rtp_address[0],mVideoConfigParam.network_info.remote_rtp_address[1],\
                  mVideoConfigParam.network_info.remote_rtp_address[2],mVideoConfigParam.network_info.remote_rtp_address[3]);

            ALOGI("\t NetworkInfo:remote rtcp address IPv4:%d.%d.%d.%d",\
                  mVideoConfigParam.network_info.remote_rtcp_address[0],mVideoConfigParam.network_info.remote_rtcp_address[1],\
                  mVideoConfigParam.network_info.remote_rtcp_address[2],mVideoConfigParam.network_info.remote_rtcp_address[3]);

        } else if(mVideoConfigParam.network_info.remote_addr_type == ViLTE_IPv6) {
            char rtp_addr_string[256];
            sprintf(rtp_addr_string,"\t NetworkInfo:remote rtp address IPv6:");

            for(uint8_t i = 0; i < 8; i++) {
                char temp_addr[8];
                sprintf(temp_addr,"%x:",mVideoConfigParam.network_info.remote_rtp_address[i*2]<< 8 | mVideoConfigParam.network_info.remote_rtp_address[i*2+1]);
                strncat(rtp_addr_string,temp_addr,8);
            }

            ALOGI("%s",rtp_addr_string);

            char rtcp_addr_string[256];
            sprintf(rtcp_addr_string,"\t NetworkInfo:remote rtcp address IPv6:");

            for(uint8_t i = 0; i < 8; i++) {
                char temp_addr[8];
                sprintf(temp_addr,"%x:",mVideoConfigParam.network_info.remote_rtp_address[i*2]<< 8 | mVideoConfigParam.network_info.remote_rtp_address[i*2+1]);
                strncat(rtcp_addr_string,temp_addr,8);
            }

            ALOGI("%s",rtcp_addr_string);
        }

        //printf local ip address value
        if(mVideoConfigParam.network_info.local_addr_type == ViLTE_IPv4) {
            ALOGI("\t NetworkInfo:local rtp address IPv4:%d.%d.%d.%d",\
                  mVideoConfigParam.network_info.local_rtp_address[0],mVideoConfigParam.network_info.local_rtp_address[1],\
                  mVideoConfigParam.network_info.local_rtp_address[2],mVideoConfigParam.network_info.local_rtp_address[3]);

            ALOGI("\t NetworkInfo:local rtcp address IPv4:%d.%d.%d.%d",\
                  mVideoConfigParam.network_info.local_rtcp_address[0],mVideoConfigParam.network_info.local_rtcp_address[1],\
                  mVideoConfigParam.network_info.local_rtcp_address[2],mVideoConfigParam.network_info.local_rtcp_address[3]);

        } else if(mVideoConfigParam.network_info.local_addr_type == ViLTE_IPv6) {
            char rtp_addr_string[256];
            sprintf(rtp_addr_string,"\t NetworkInfo:local rtp address IPv6:");

            for(uint8_t i = 0; i < 8; i++) {
                char temp_addr[8];
                sprintf(temp_addr,"%x:",mVideoConfigParam.network_info.local_rtp_address[i*2]<< 8 | mVideoConfigParam.network_info.local_rtp_address[i*2+1]);
                strncat(rtp_addr_string,temp_addr,8);
            }

            ALOGI("%s",rtp_addr_string);

            char rtcp_addr_string[256];
            sprintf(rtcp_addr_string,"\t NetworkInfo:local rtcp address IPv6:");

            for(uint8_t i = 0; i < 8; i++) {
                char temp_addr[8];
                sprintf(temp_addr,"%x:",mVideoConfigParam.network_info.local_rtcp_address[i*2]<< 8 | mVideoConfigParam.network_info.local_rtcp_address[i*2+1]);
                strncat(rtcp_addr_string,temp_addr,8);
            }

            ALOGI("%s",rtcp_addr_string);
        }


        mVideoConfigParam.network_info.network_id   = (pRTPNegotiatedParams->network_info).network_id;
        strncpy(mVideoConfigParam.network_info.ifname, (pRTPNegotiatedParams->network_info).ifname, 16);
        mVideoConfigParam.network_info.ifname[15] = '\0';
        mVideoConfigParam.network_info.uid  = (pRTPNegotiatedParams->network_info).uid;
        mVideoConfigParam.network_info.rtp_direction    = (pRTPNegotiatedParams->network_info).rtp_direction;
        mVideoConfigParam.network_info.tag  = (pRTPNegotiatedParams->network_info).tag;

        ALOGI("\t NetworkInfo:network_id[%d], uid=%d, rtp_direction[%d]  ifname size[%zu][%s]",\
              mVideoConfigParam.network_info.network_id,mVideoConfigParam.network_info.uid,
              mVideoConfigParam.network_info.rtp_direction, strlen(mVideoConfigParam.network_info.ifname),
              mVideoConfigParam.network_info.ifname);

        memcpy(mVideoConfigParam.network_info.socket_fds, (pRTPNegotiatedParams->network_info).socket_fds,\
               sizeof((pRTPNegotiatedParams->network_info).socket_fds));

        //reduce log
        for(uint8_t ifd = 0; ifd < 2 /*VILTE_BIND_FD_NUMBER*/; ifd++) {
            ALOGI("\t NetworkInfo:socket_fds[%d] = %d",ifd, (mVideoConfigParam.network_info.socket_fds) [ifd]);
        }

        mVideoConfigParam.network_info.MBR_DL = (pRTPNegotiatedParams->network_info).MBR_DL;
        mVideoConfigParam.network_info.MBR_UL = (pRTPNegotiatedParams->network_info).MBR_UL;

        ALOGI("\t NetworkInfo:MBR_DL = %d kbps, MBR_UL = %d kbps",\
              mVideoConfigParam.network_info.MBR_DL,mVideoConfigParam.network_info.MBR_UL);

        return OK;
    } else if(uiMediaType == IMSMA_RTP_AUDIO) {
        ALOGE("%s,Audio not support now",__FUNCTION__);
        return UNKNOWN_ERROR;
    } else {
        ALOGE("%s,unknown media type 0x%x",__FUNCTION__,uiMediaType);
        return UNKNOWN_ERROR;
    }
}

status_t RTPController::onUpdateConfigParams(rtp_rtcp_config_t* pRTPNegotiatedParams,uint8_t uiMediaType)
{
    if(uiMediaType == IMSMA_RTP_VIDEO) {
        ALOGD("%s,update video negotiated done params",__FUNCTION__);

        //if video hasn't added,assign new parameters directly
        if(!(mAddedTrack & IMSMA_RTP_VIDEO)) {
            onSetConfigParams(pRTPNegotiatedParams,IMSMA_RTP_VIDEO);
            return OK;
        }

        //save the updated negotiated done params of video
        //Need Check whether need check which item updated

        //now not support update mediatype and mimetype
        //ToDo, need support this if add support for H.265
        if(mVideoConfigParam.media_type != pRTPNegotiatedParams->media_type ||
                mVideoConfigParam.mime_Type != pRTPNegotiatedParams->mime_Type) {
            ALOGE("%s, not support update mime(%d,%d->%d,%d)",\
                  __FUNCTION__,mVideoConfigParam.media_type,mVideoConfigParam.mime_Type,\
                  pRTPNegotiatedParams->media_type,pRTPNegotiatedParams->mime_Type);
            return UNKNOWN_ERROR;
        }

        mVideoConfigParam.media_type = pRTPNegotiatedParams->media_type;
        mVideoConfigParam.mime_Type = pRTPNegotiatedParams->mime_Type;
        mVideoConfigParam.rtp_payload_type = pRTPNegotiatedParams->rtp_payload_type;
        mVideoConfigParam.rtp_profile = pRTPNegotiatedParams->rtp_profile;
        mVideoConfigParam.sample_rate = pRTPNegotiatedParams->sample_rate;

        mVideoConfigParam.rtp_packet_bandwidth = pRTPNegotiatedParams->rtp_packet_bandwidth;
        mVideoConfigParam.packetize_mode = pRTPNegotiatedParams->packetize_mode;

        mVideoConfigParam.rtp_header_extension_num = pRTPNegotiatedParams->rtp_header_extension_num;
        uint8_t max_allow_ext_num = sizeof(mVideoConfigParam.rtp_ext_map) /sizeof(rtp_ext_map_t);
        ALOGD("\t ext num:%d,max allow ext num:%d",\
              mVideoConfigParam.rtp_header_extension_num,max_allow_ext_num);

        if(mVideoConfigParam.rtp_header_extension_num > max_allow_ext_num) {
            mVideoConfigParam.rtp_header_extension_num = max_allow_ext_num;
        }

        for(uint32_t j = 0; j < mVideoConfigParam.rtp_header_extension_num; j++) {
            mVideoConfigParam.rtp_ext_map[j].extension_id = (pRTPNegotiatedParams->rtp_ext_map[j]).extension_id;
            mVideoConfigParam.rtp_ext_map[j].direction = (pRTPNegotiatedParams->rtp_ext_map[j]).direction;
            memcpy(mVideoConfigParam.rtp_ext_map[j].extension_uri, (pRTPNegotiatedParams->rtp_ext_map[j]).extension_uri,\
                   sizeof((pRTPNegotiatedParams->rtp_ext_map[j]).extension_uri));

            ALOGD("\t ext id:%d,uri:%s",\
                  mVideoConfigParam.rtp_ext_map[j].extension_id,mVideoConfigParam.rtp_ext_map[j].extension_uri);
        }

        //need handle special case for rr=0, rs=0
        if(mVideoConfigParam.rtcp_sender_bandwidth == 0 && mVideoConfigParam.rtcp_receiver_bandwidth == 0) {
            if(pRTPNegotiatedParams->rtcp_sender_bandwidth != 0 || pRTPNegotiatedParams->rtcp_receiver_bandwidth != 0) {
                ALOGI("%s,rtcp recover",__FUNCTION__);
                startScheduleRTCP(IMSMA_RTP_VIDEO);
            }
        } else {
            if(pRTPNegotiatedParams->rtcp_sender_bandwidth == 0 &&  pRTPNegotiatedParams->rtcp_receiver_bandwidth == 0) {
                ALOGI("%s,rtcp stop",__FUNCTION__);
                stopScheduleRTCP(IMSMA_RTP_VIDEO);
            }
        }

        mVideoConfigParam.rtcp_sender_bandwidth = pRTPNegotiatedParams->rtcp_sender_bandwidth;
        mVideoConfigParam.rtcp_receiver_bandwidth = pRTPNegotiatedParams->rtcp_receiver_bandwidth;

        mVideoConfigParam.rtcp_reduce_size = pRTPNegotiatedParams->rtcp_reduce_size;
        mVideoConfigParam.rtcp_fb_param_num = pRTPNegotiatedParams->rtcp_fb_param_num;

        ALOGD("\t media(0x%x),mime(0x%x),PT(%d),profile(0x%x),sampleRate(%d)",\
              mVideoConfigParam.media_type,mVideoConfigParam.mime_Type,\
              mVideoConfigParam.rtp_payload_type,mVideoConfigParam.rtp_profile,mVideoConfigParam.sample_rate);

        ALOGD("\t b=AS(%d),packetize_mode(%d),b=RS(%" PRId64 "),b=RR(%" PRId64 "),rtcp_rsize(%d)",\
              mVideoConfigParam.rtp_packet_bandwidth,mVideoConfigParam.packetize_mode,\
              mVideoConfigParam.rtcp_sender_bandwidth,mVideoConfigParam.rtcp_receiver_bandwidth,mVideoConfigParam.rtcp_reduce_size);

        ALOGD("\t fb_num(%d)",mVideoConfigParam.rtcp_fb_param_num);

        m_Trr_interval = 1000000ll;
        bool bSupportTMMBR = false;

        for(uint8_t i = 0; i< mVideoConfigParam.rtcp_fb_param_num; i++) {
            mVideoConfigParam.rtcp_fb_type[i].rtcp_fb_id= pRTPNegotiatedParams->rtcp_fb_type[i].rtcp_fb_id;
            mVideoConfigParam.rtcp_fb_type[i].rtcp_fb_param = pRTPNegotiatedParams->rtcp_fb_type[i].rtcp_fb_param;

            //Need check, whether struct can assign data this way
            memcpy(mVideoConfigParam.rtcp_fb_type[i].rtcp_fb_sub_param,pRTPNegotiatedParams->rtcp_fb_type[i].rtcp_fb_sub_param,sizeof(pRTPNegotiatedParams->rtcp_fb_type[i].rtcp_fb_sub_param));

            ALOGV("\t Feedback[%d]:fb_id(%d),fb_param(%d),fb_sub_param(%s)",\
                  i,mVideoConfigParam.rtcp_fb_type[i].rtcp_fb_id,mVideoConfigParam.rtcp_fb_type[i].rtcp_fb_param,\
                  mVideoConfigParam.rtcp_fb_type[i].rtcp_fb_sub_param);

            if(mVideoConfigParam.rtcp_fb_type[i].rtcp_fb_id == IMSMA_TRR_INT) {
                m_Trr_interval = mVideoConfigParam.rtcp_fb_type[i].rtcp_fb_param * 1000;//in Us
            }

            if(mVideoConfigParam.rtcp_fb_type[i].rtcp_fb_id == IMSMA_CCM && mVideoConfigParam.rtcp_fb_type[i].rtcp_fb_param == IMSMA_TMMBR) {
                bSupportTMMBR = true;
            }
        }

        if(bSupportTMMBR == true) {
            mSupportTMMBR =true;
        }

        ALOGI("\t m_Trr_interval=%" PRId64 "us, bSupportTMMBR = %d : %d",m_Trr_interval,bSupportTMMBR,mSupportTMMBR);

        //get network info
        mVideoConfigParam.network_info.ebi = (pRTPNegotiatedParams->network_info).ebi;
        mVideoConfigParam.network_info.interface_type = (pRTPNegotiatedParams->network_info).interface_type;
        mVideoConfigParam.network_info.dscp= (pRTPNegotiatedParams->network_info).dscp;
        mVideoConfigParam.network_info.soc_priority= (pRTPNegotiatedParams->network_info).soc_priority;

        //handle special case for port num 0
        if(mVideoConfigParam.network_info.remote_rtcp_port == 0) {
            if((pRTPNegotiatedParams->network_info).remote_rtcp_port != 0) {
                ALOGI("%s,rtcp recover,port available",__FUNCTION__);
                startScheduleRTCP(IMSMA_RTP_VIDEO);
            }
        } else {
            if((pRTPNegotiatedParams->network_info).remote_rtcp_port == 0) {
                ALOGI("%s,rtcp stop,port not available",__FUNCTION__);
                stopScheduleRTCP(IMSMA_RTP_VIDEO);
            }
        }

        //ToDo:
        //need handle ip address or port update during ViLTE
        //need inform SocketWrapper
        //or new a new SocketWrapper and keep the old SoKetWrapper to not drop the old packets
        bool isPortChange = false;

        if(mVideoConfigParam.network_info.remote_rtp_port != (pRTPNegotiatedParams->network_info).remote_rtp_port) {
            ALOGI("\t NetworkInfo remote port change:before port:[rtp:%d][rtcp:%d],now port:[rtp:%d][rtcp:%d]",\
                  mVideoConfigParam.network_info.remote_rtp_port, mVideoConfigParam.network_info.remote_rtcp_port,
                  (pRTPNegotiatedParams->network_info).remote_rtp_port, (pRTPNegotiatedParams->network_info).remote_rtcp_port);
            isPortChange = true;
        }

        if(mVideoConfigParam.network_info.local_rtp_port != (pRTPNegotiatedParams->network_info).local_rtp_port) {
            ALOGI("\t NetworkInfo local port change:before port:[rtp:%d][rtcp:%d],now port:[rtp:%d][rtcp:%d]",\
                  mVideoConfigParam.network_info.local_rtp_port, mVideoConfigParam.network_info.local_rtcp_port,
                  (pRTPNegotiatedParams->network_info).local_rtp_port, (pRTPNegotiatedParams->network_info).local_rtcp_port);
            isPortChange = true;
        }


        mVideoConfigParam.network_info.remote_rtp_port = (pRTPNegotiatedParams->network_info).remote_rtp_port;
        mVideoConfigParam.network_info.remote_rtcp_port = (pRTPNegotiatedParams->network_info).remote_rtcp_port;
        mVideoConfigParam.network_info.local_rtp_port = (pRTPNegotiatedParams->network_info).local_rtp_port;
        mVideoConfigParam.network_info.local_rtcp_port  = (pRTPNegotiatedParams->network_info).local_rtcp_port;

        //for datausage tag change
        bool isTagChange = false;

        if(mVideoConfigParam.network_info.tag != (pRTPNegotiatedParams->network_info).tag) {
            ALOGI("\t NetworkInfo tag before tag[0x%x], now tag[0x%x]",\
                  mVideoConfigParam.network_info.tag, (pRTPNegotiatedParams->network_info).tag);
            isTagChange = true;
        }

        mVideoConfigParam.network_info.tag = (pRTPNegotiatedParams->network_info).tag;

        ALOGI("\t NetworkInfo:ebi[%x],interface_type[%x],dscp:[%d],priority=[%d]",\
              mVideoConfigParam.network_info.ebi,mVideoConfigParam.network_info.interface_type,\
              mVideoConfigParam.network_info.dscp,mVideoConfigParam.network_info.soc_priority);

        ALOGI("\t NetworkInfo:remote port:[rtp:%d][rtcp:%d],local port:[rtp:%d][rtcp:%d]",\
              mVideoConfigParam.network_info.remote_rtp_port,mVideoConfigParam.network_info.remote_rtcp_port,\
              mVideoConfigParam.network_info.local_rtp_port,mVideoConfigParam.network_info.local_rtcp_port);

        //ToDo:
        //handle special case for ip address = 0.0.0.0

        memcpy(mVideoConfigParam.network_info.remote_rtp_address, (pRTPNegotiatedParams->network_info).remote_rtp_address,\
               sizeof((pRTPNegotiatedParams->network_info).remote_rtp_address));
        memcpy(mVideoConfigParam.network_info.remote_rtcp_address, (pRTPNegotiatedParams->network_info).remote_rtcp_address,\
               sizeof((pRTPNegotiatedParams->network_info).remote_rtcp_address));

        memcpy(mVideoConfigParam.network_info.local_rtp_address, (pRTPNegotiatedParams->network_info).local_rtp_address,\
               sizeof((pRTPNegotiatedParams->network_info).local_rtp_address));

        memcpy(mVideoConfigParam.network_info.local_rtcp_address, (pRTPNegotiatedParams->network_info).local_rtcp_address,\
               sizeof((pRTPNegotiatedParams->network_info).local_rtcp_address));

        mVideoConfigParam.network_info.remote_addr_type = (pRTPNegotiatedParams->network_info).remote_addr_type;
        mVideoConfigParam.network_info.local_addr_type  = (pRTPNegotiatedParams->network_info).local_addr_type;

        ALOGI("\t NetworkInfo:remote address type:%x",mVideoConfigParam.network_info.remote_addr_type);

        //printf remote ip address value
        if(mVideoConfigParam.network_info.remote_addr_type == ViLTE_IPv4) {
            ALOGI("\t NetworkInfo:remote rtp address IPv4:%d.%d.%d.%d",\
                  mVideoConfigParam.network_info.remote_rtp_address[0],mVideoConfigParam.network_info.remote_rtp_address[1],\
                  mVideoConfigParam.network_info.remote_rtp_address[2],mVideoConfigParam.network_info.remote_rtp_address[3]);

            ALOGI("\t NetworkInfo:remote rtcp address IPv4:%d.%d.%d.%d",\
                  mVideoConfigParam.network_info.remote_rtcp_address[0],mVideoConfigParam.network_info.remote_rtcp_address[1],\
                  mVideoConfigParam.network_info.remote_rtcp_address[2],mVideoConfigParam.network_info.remote_rtcp_address[3]);

        } else if(mVideoConfigParam.network_info.remote_addr_type == ViLTE_IPv6) {
            char rtp_addr_string[256];
            sprintf(rtp_addr_string,"\t NetworkInfo:remote rtp address IPv6:");

            for(uint8_t i = 0; i < 8; i++) {
                char temp_addr[8];
                sprintf(temp_addr,"%x:",mVideoConfigParam.network_info.remote_rtp_address[i*2]<< 8 | mVideoConfigParam.network_info.remote_rtp_address[i*2+1]);
                strncat(rtp_addr_string,temp_addr,8);
            }

            ALOGI("%s",rtp_addr_string);

            char rtcp_addr_string[256];
            sprintf(rtcp_addr_string,"\t NetworkInfo:remote rtcp address IPv6:");

            for(uint8_t i = 0; i < 8; i++) {
                char temp_addr[8];
                sprintf(temp_addr,"%x:",mVideoConfigParam.network_info.remote_rtp_address[i*2]<< 8 | mVideoConfigParam.network_info.remote_rtp_address[i*2+1]);
                strncat(rtcp_addr_string,temp_addr,8);
            }

            ALOGI("%s",rtcp_addr_string);
        }

        //printf local ip address value
        if(mVideoConfigParam.network_info.local_addr_type == ViLTE_IPv4) {
            ALOGI("\t NetworkInfo:local rtp address IPv4:%d.%d.%d.%d",\
                  mVideoConfigParam.network_info.local_rtp_address[0],mVideoConfigParam.network_info.local_rtp_address[1],\
                  mVideoConfigParam.network_info.local_rtp_address[2],mVideoConfigParam.network_info.local_rtp_address[3]);

            ALOGI("\t NetworkInfo:local rtcp address IPv4:%d.%d.%d.%d",\
                  mVideoConfigParam.network_info.local_rtcp_address[0],mVideoConfigParam.network_info.local_rtcp_address[1],\
                  mVideoConfigParam.network_info.local_rtcp_address[2],mVideoConfigParam.network_info.local_rtcp_address[3]);

        } else if(mVideoConfigParam.network_info.local_addr_type == ViLTE_IPv6) {
            char rtp_addr_string[256];
            sprintf(rtp_addr_string,"\t NetworkInfo:local rtp address IPv6:");

            for(uint8_t i = 0; i < 8; i++) {
                char temp_addr[8];
                sprintf(temp_addr,"%x:",mVideoConfigParam.network_info.local_rtp_address[i*2]<< 8 | mVideoConfigParam.network_info.local_rtp_address[i*2+1]);
                strncat(rtp_addr_string,temp_addr,8);
            }

            ALOGI("%s",rtp_addr_string);

            char rtcp_addr_string[256];
            sprintf(rtcp_addr_string,"\t NetworkInfo:local rtcp address IPv6:");

            for(uint8_t i = 0; i < 8; i++) {
                char temp_addr[8];
                sprintf(temp_addr,"%x:",mVideoConfigParam.network_info.local_rtcp_address[i*2]<< 8 | mVideoConfigParam.network_info.local_rtcp_address[i*2+1]);
                strncat(rtcp_addr_string,temp_addr,8);
            }

            ALOGI("%s",rtcp_addr_string);
        }


        mVideoConfigParam.network_info.network_id   = (pRTPNegotiatedParams->network_info).network_id;
        strncpy(mVideoConfigParam.network_info.ifname, (pRTPNegotiatedParams->network_info).ifname, 16);
        mVideoConfigParam.network_info.ifname[15] = '\0';
        mVideoConfigParam.network_info.uid  = (pRTPNegotiatedParams->network_info).uid;
        mVideoConfigParam.network_info.rtp_direction    = (pRTPNegotiatedParams->network_info).rtp_direction;

        ALOGI("\t NetworkInfo:network_id[%d], uid=%d, rtp_direction[%d] ifname size[%zu][%s]",\
              mVideoConfigParam.network_info.network_id,mVideoConfigParam.network_info.uid,
              mVideoConfigParam.network_info.rtp_direction,strlen(mVideoConfigParam.network_info.ifname),
              mVideoConfigParam.network_info.ifname);

        memcpy(mVideoConfigParam.network_info.socket_fds, (pRTPNegotiatedParams->network_info).socket_fds,\
               sizeof((pRTPNegotiatedParams->network_info).socket_fds));

        //reduce log
        for(uint8_t ifd = 0; ifd < 2 /*VILTE_BIND_FD_NUMBER*/; ifd++) {
            ALOGI("\t NetworkInfo:socket_fds[%d] = %d",ifd, (mVideoConfigParam.network_info.socket_fds) [ifd]);
        }

        //notify socketwapper port change
        if(isPortChange == true || isTagChange == true) {
            if(!mVideoRTCPSocketWrapper.get()) {
                mVideoRTCPSocketWrapper = new SocketWrapper();
            }

            Sock_param_t param_rtcp;
            param_rtcp.protocol_version = mVideoConfigParam.network_info.remote_addr_type;
            param_rtcp.sockfd= -1;

            param_rtcp.isBlock = false;
            param_rtcp.sendBufferSize = 512 * 1024;
            param_rtcp.receiveBufferSize = 512 * 1024;
            param_rtcp.dscp = mVideoConfigParam.network_info.dscp;
            param_rtcp.priority = mVideoConfigParam.network_info.soc_priority;
            param_rtcp.network_id =  mVideoConfigParam.network_info.network_id;
            strncpy(param_rtcp.ifname, mVideoConfigParam.network_info.ifname,16);
            param_rtcp.ifname[15] = '\0';
            param_rtcp.uid =     mVideoConfigParam.network_info.uid;
            memcpy(param_rtcp.local_address,mVideoConfigParam.network_info.local_rtcp_address,sizeof(mVideoConfigParam.network_info.local_rtcp_address));
            param_rtcp.local_port = mVideoConfigParam.network_info.local_rtcp_port;
            param_rtcp.tag = mVideoConfigParam.network_info.tag;

            memcpy(param_rtcp.peer_address,mVideoConfigParam.network_info.remote_rtcp_address,sizeof(mVideoConfigParam.network_info.remote_rtcp_address));
            param_rtcp.peer_port = mVideoConfigParam.network_info.remote_rtcp_port;

            ALOGI("\t NetworkInfo notify RTCP socketwapper param change");

            mVideoRTCPSocketWrapper->setParam(param_rtcp);

            if(!mVideoRTPSocketWrapper.get()) {
                mVideoRTPSocketWrapper = new SocketWrapper();
            }

            Sock_param_t param_rtp;
            param_rtp.protocol_version = mVideoConfigParam.network_info.remote_addr_type;
            param_rtp.sockfd= -1;

            param_rtp.isBlock = false;
            param_rtp.sendBufferSize = 2048 * 1024;
            param_rtp.receiveBufferSize = 2048 * 1024;
            param_rtp.dscp = mVideoConfigParam.network_info.dscp;
            param_rtp.priority = mVideoConfigParam.network_info.soc_priority;
            param_rtp.network_id =  mVideoConfigParam.network_info.network_id;
            strncpy(param_rtp.ifname, mVideoConfigParam.network_info.ifname, 16);
            param_rtp.ifname[15] = '\0';
            param_rtp.uid =     mVideoConfigParam.network_info.uid;
            memcpy(param_rtp.local_address,mVideoConfigParam.network_info.local_rtp_address,sizeof(mVideoConfigParam.network_info.local_rtp_address));
            param_rtp.local_port = mVideoConfigParam.network_info.local_rtp_port;
            param_rtp.tag = mVideoConfigParam.network_info.tag;

            memcpy(param_rtp.peer_address,mVideoConfigParam.network_info.remote_rtp_address,sizeof(mVideoConfigParam.network_info.remote_rtp_address));
            param_rtp.peer_port = mVideoConfigParam.network_info.remote_rtp_port;

            ALOGI("\t NetworkInfo notify RTP socketwapper param change");

            mVideoRTPSocketWrapper->setParam(param_rtp);
        }

        /*****************MBR ************************/
        uint32_t uiLastMBR_DL = 0;
        uint32_t uiNewMBR_DL  = 0;

        uiLastMBR_DL = mVideoConfigParam.network_info.MBR_DL;
        uiNewMBR_DL = (pRTPNegotiatedParams->network_info).MBR_DL;

        if(uiLastMBR_DL != 0 && uiNewMBR_DL != 0) {
            //ToDo: whether need set a threshold,
            //do not trigger sip update too frequently
            if(uiLastMBR_DL < uiNewMBR_DL) {
                //downlink MBR increase
                /*
                sp<AMessage> notify = mNotify->dup();
                notify->setInt32("what",kWhatUpdateVB);
                notify->setInt32("mbr_dl",uiNewMBR_DL);//Kbps
                notify->post();
                */

            } else if(uiLastMBR_DL > uiNewMBR_DL) {
                if(uiNewMBR_DL < mVideoConfigParam.rtp_packet_bandwidth) {
                    ALOGI("\t NetworkInfo:uiLastMBR_DL=%dkbps,uiNewMBR_DL=%dkbps rtp_packet_bandwidth=%d kWhatUpdateVB",\
                          uiLastMBR_DL, uiNewMBR_DL, mVideoConfigParam.rtp_packet_bandwidth);

                    if(!mSupportTMMBR) {
                        sp<AMessage> notify = mNotify->dup();
                        notify->setInt32("what",kWhatUpdateVB);
                        notify->setInt32("mbr_dl",uiNewMBR_DL);    //Kbps
                        notify->post();
                    } else {
                        ALOGI("\t NetworkInfo:bSupportTMMBR[%d:%d] not notify UPFATE", bSupportTMMBR,mSupportTMMBR);
                    }
                }

                /*
                //downlink MBR reduce
                //check whether support tmmbr
                if(!bSupportTMMBR){
                    sp<AMessage> notify = mNotify->dup();
                    notify->setInt32("what",kWhatUpdateVB);
                    notify->setInt32("mbr_dl",uiNewMBR_DL);//Kbps
                    notify->post();
                }else{
                    //ToDo:as spec we should trigger sip update after receivng TMMBN
                    //but we don't think it's necessary, we should trigger sip update as soon as possible
                    //or post with a delayUs?
                    //comibine the operations
                    sp<AMessage> notify = mNotify->dup();
                    notify->setInt32("what",kWhatUpdateVB);
                    notify->setInt32("mbr_dl",uiNewMBR_DL);//Kbps
                    notify->post();
                }
                */
            }
        }

        mVideoConfigParam.network_info.MBR_DL = (pRTPNegotiatedParams->network_info).MBR_DL;
        mVideoConfigParam.network_info.MBR_UL = (pRTPNegotiatedParams->network_info).MBR_UL;

        ALOGI("\t NetworkInfo:MBR_DL = %d kbps, MBR_UL = %d kbps",\
              mVideoConfigParam.network_info.MBR_DL,mVideoConfigParam.network_info.MBR_UL);

        if(mVideoRTPSender.get()) {
            mVideoRTPSender->updateConfigParams(&mVideoConfigParam);
        }

        if(mRTPReceiver.get()) {
            mRTPReceiver->updateConfigParams(&mVideoConfigParam,IMSMA_RTP_VIDEO);
        }

        return OK;
    } else if(uiMediaType == IMSMA_RTP_AUDIO) {
        ALOGE("%s,Audio not support now",__FUNCTION__);
        return UNKNOWN_ERROR;
    } else {
        ALOGE("%s,unknown media type 0x%x",__FUNCTION__,uiMediaType);
        return UNKNOWN_ERROR;
    }

}

status_t RTPController::onAddStream(int32_t rtpPath,int32_t trackIndex)
{
    ALOGI("%s,rtpPath(0x%x),track(%d)",__FUNCTION__,rtpPath,trackIndex);

    //re-architecture
    if(trackIndex & IMSMA_RTP_VIDEO) {
        if(!mVideoRTCPSocketWrapper.get()) {
            mVideoRTCPSocketWrapper = new SocketWrapper();

            Sock_param_t param;
            param.protocol_version = mVideoConfigParam.network_info.remote_addr_type;
            param.sockfd= -1;
            //get param from mConfigParam
            // find the fd related to port number
            //fill to param
            int32_t socket_fd = findSocketFd(mVideoConfigParam.network_info.local_rtcp_port);
            ALOGI("%s,findSocketFd,rtcp local port:%d,fd:%d",__FUNCTION__,mVideoConfigParam.network_info.local_rtcp_port,socket_fd);

            if(socket_fd != -1) {
                param.sockfd = socket_fd;
            }

            param.isBlock = false;
            param.sendBufferSize = 512 * 1024;
            param.receiveBufferSize = 512 * 1024;

            param.dscp = mVideoConfigParam.network_info.dscp;
            param.priority = mVideoConfigParam.network_info.soc_priority;
            param.network_id =  mVideoConfigParam.network_info.network_id;
            strncpy(param.ifname, mVideoConfigParam.network_info.ifname, 16);
            param.ifname[15] = '\0';
            param.uid =     mVideoConfigParam.network_info.uid;
            memcpy(param.local_address,mVideoConfigParam.network_info.local_rtcp_address,sizeof(mVideoConfigParam.network_info.local_rtcp_address));
            param.local_port = mVideoConfigParam.network_info.local_rtcp_port;
            param.tag = mVideoConfigParam.network_info.tag;

            memcpy(param.peer_address,mVideoConfigParam.network_info.remote_rtcp_address,sizeof(mVideoConfigParam.network_info.remote_rtcp_address));
            param.peer_port = mVideoConfigParam.network_info.remote_rtcp_port;

            mVideoRTCPSocketWrapper->setParam(param);

            //config socket attributes: like blocking/non-blocking, socket buffer queue size

            //start listen RTCP callback from SocketWrapper
            //bring start listen RTCP forward
            mVideoRTCPSocketWrapper->setRxCallBack(this,videoRTCPPacketCallBack);
        }

        if(!mVideoRTPSocketWrapper.get()) {
            mVideoRTPSocketWrapper = new SocketWrapper();
            Sock_param_t param;
            param.protocol_version = mVideoConfigParam.network_info.remote_addr_type;
            param.sockfd= -1;
            //get param from mConfigParam
            // find the fd related to port number
            //fill to param
            int32_t socket_fd = findSocketFd(mVideoConfigParam.network_info.local_rtp_port);
            ALOGI("%s,findSocketFd,rtp local port:%d,fd:%d",__FUNCTION__,mVideoConfigParam.network_info.local_rtp_port,socket_fd);

            if(socket_fd != -1) {
                param.sockfd = socket_fd;
            }

            param.isBlock = false;
            param.sendBufferSize = 2048 * 1024;
            param.receiveBufferSize = 2048 * 1024;
            ALOGI("%s,isBlock=%d,sendBufferSize:%d,receiveBufferSize:%d",__FUNCTION__, param.isBlock, param.sendBufferSize, param.receiveBufferSize);
            param.dscp = mVideoConfigParam.network_info.dscp;
            param.priority = mVideoConfigParam.network_info.soc_priority;
            param.network_id =  mVideoConfigParam.network_info.network_id;
            strncpy(param.ifname, mVideoConfigParam.network_info.ifname,16);
            param.ifname[15] = '\0';
            param.uid =     mVideoConfigParam.network_info.uid;
            memcpy(param.local_address,mVideoConfigParam.network_info.local_rtp_address,sizeof(mVideoConfigParam.network_info.local_rtp_address));
            param.local_port = mVideoConfigParam.network_info.local_rtp_port;
            param.tag = mVideoConfigParam.network_info.tag;

            memcpy(param.peer_address,mVideoConfigParam.network_info.remote_rtp_address,sizeof(mVideoConfigParam.network_info.remote_rtp_address));
            param.peer_port = mVideoConfigParam.network_info.remote_rtp_port;

            mVideoRTPSocketWrapper->setParam(param);

            //config socket attributes: like blocking/non-blocking, socket buffer queue size
        }

        if((rtpPath & IMSMA_RTP_UPLINK) && !(mVideoAddedPath & IMSMA_RTP_UPLINK)) {
            //add RTPSender for Video Track
            ALOGI("new RTPSender for video track");
            sp<AMessage> notify = new AMessage(kWhatSenderNotify, mReflector);
            notify->setInt32("trackIndex",IMSMA_RTP_VIDEO);
            mVideoRTPSender = new RTPSender(mSSRC,mVideoRTPSocketWrapper,notify, mSimID,mOperatorID);

            ALOGI("new RTPSender_Video looper");
            mVideoSenderLooper = new ALooper;
            mVideoSenderLooper->setName("RTPSender_Video");
            mVideoSenderLooper->registerHandler(mVideoRTPSender);
            mVideoSenderLooper->start();

            mVideoRTPSender->setConfigParams(&mVideoConfigParam);

            mAddedTrack |= IMSMA_RTP_VIDEO;
            mVideoAddedPath |= IMSMA_RTP_UPLINK;

        }

        if((rtpPath & IMSMA_RTP_DOWNLINK) && !(mVideoAddedPath & IMSMA_RTP_DOWNLINK)) {
            if(!mRTPReceiver.get()) {
                sp<AMessage> notify = new AMessage(kWhatReceiverNotify, mReflector);
                mRTPReceiver = new RTPReceiver(notify, mSimID,mOperatorID);

                ALOGI("new RTPReceiver looper");
                mReceiverLooper = new ALooper;
                mReceiverLooper->setName("RTPReceiver");
                mReceiverLooper->registerHandler(mRTPReceiver);
                mReceiverLooper->start();
            }

            ALOGI("add video track to RTPReceiver");
            mRTPReceiver->addStream(&mVideoConfigParam,mVideoRTPSocketWrapper,mVideoAccessUnitNotify,IMSMA_RTP_VIDEO);
            //mRTPReceiver->setNotify(mVideoAccessUnitNotify,IMSMA_RTP_VIDEO);
            mAddedTrack |= IMSMA_RTP_VIDEO;
            mVideoAddedPath |= IMSMA_RTP_DOWNLINK;
        }
    }

    ALOGI("%s,added track:0x%x, video added path:0x%x",__FUNCTION__,mAddedTrack,mVideoAddedPath);
    return OK;
}
status_t RTPController::onRemoveStream(int32_t rtpPath,int32_t trackIndex)
{
    ALOGI("%s,rtpPath(0x%x),track(%d)",__FUNCTION__,rtpPath,trackIndex);

    if((trackIndex & IMSMA_RTP_VIDEO) && (mAddedTrack & IMSMA_RTP_VIDEO)) {
        if((rtpPath & IMSMA_RTP_UPLINK) && (mVideoAddedPath & IMSMA_RTP_UPLINK)) {
            if(mVideoRTPSender.get()) {
                ALOGI("remove Video RTPSender");

                if(mVideoActivePath & IMSMA_RTP_UPLINK) {
                    mVideoRTPSender->stop();
                    mVideoActivePath &= ~IMSMA_RTP_UPLINK;
                }

                if(mVideoSenderLooper.get()) {
                    ALOGI("stop RTPSender_Video looper");
                    mVideoSenderLooper->stop();
                    mVideoSenderLooper->unregisterHandler(mVideoRTPSender->id());
                    mVideoSenderLooper = NULL;
                }

                mVideoRTPSender = NULL;

                mVideoAddedPath &= ~IMSMA_RTP_UPLINK;
            }
        }

        if((rtpPath & IMSMA_RTP_DOWNLINK) && (mVideoAddedPath & IMSMA_RTP_DOWNLINK)) {
            if(mRTPReceiver.get()) {
                ALOGI("remove video track of RTPReceiver");

                if(mVideoActivePath & IMSMA_RTP_DOWNLINK) {
                    mRTPReceiver->stop(IMSMA_RTP_VIDEO);
                }

                mRTPReceiver->removeStream(IMSMA_RTP_VIDEO);

                mVideoAddedPath &= ~IMSMA_RTP_DOWNLINK;
            }
        }

        //if  video uplink and downlink all removed
        if(mVideoAddedPath == 0) {
            mAddedTrack &= ~IMSMA_RTP_VIDEO;

            //close video RTP  SocketWrapper
            mVideoRTPSocketWrapper = NULL;

            //stop video rtcp
            stopScheduleRTCP(IMSMA_RTP_VIDEO);

            //SocketWrapper will has lock between setRxCallback and call videoRTPPacketCallBack
            // So RTPController can be released safely even there is a callback from SocketWrapper
            if(mVideoRTCPSocketWrapper.get()) {
                mVideoRTCPSocketWrapper->setRxCallBack(0,0);
                mVideoRTCPSocketWrapper = NULL;
            }
        }
    }

    //add audio track remove here
    //.....

    //release RTPReceiver
    if(!(mVideoAddedPath & IMSMA_RTP_DOWNLINK) && !(mAudioAddedPath & IMSMA_RTP_DOWNLINK)) {
        if(mReceiverLooper.get()) {
            ALOGI("stop RTPReceiver looper");
            mReceiverLooper->stop();
            mReceiverLooper->unregisterHandler(mRTPReceiver->id());
            mReceiverLooper = NULL;
        }

        mRTPReceiver = NULL;
    }

    /*
        //if video and audio track all removed
        //stop RTPController looper
        if(mAddedTrack == 0){
            //ALooper ALooperRoster has lock
            //so the message handling in onMessagereceive will be complete
            //before unregisterhandler can return
            //so caller can release RTPController safely even there are messages receving from other thread
            if(mLooper.get()){
                ALOGI("stop RTPController looper");
                mLooper->stop();
                mLooper->unregisterHandler(mReflector->id());
                mLooper = NULL;
            }
        }
    */

    ALOGI("%s,added track:0x%x, video added path:0x%x",__FUNCTION__,mAddedTrack,mVideoAddedPath);
    return OK;
}

status_t RTPController::onStart(int32_t rtpPath,int32_t trackIndex)
{
    ALOGI("%s,rtpPath(0x%x),track(%d)",__FUNCTION__,rtpPath,trackIndex);
    status_t err = OK;

    if(trackIndex & IMSMA_RTP_VIDEO) {
        if(!mVideoRTPSocketWrapper.get()) {
            mVideoRTPSocketWrapper = new SocketWrapper();
            Sock_param_t param;
            param.protocol_version = mVideoConfigParam.network_info.remote_addr_type;
            param.sockfd= -1;
            //get param from mConfigParam
            // find the fd related to port number
            //fill to param
            int32_t socket_fd = findSocketFd(mVideoConfigParam.network_info.local_rtp_port);
            ALOGI("%s,findSocketFd,rtp local port:%d,fd:%d",\
                  __FUNCTION__,mVideoConfigParam.network_info.local_rtp_port,socket_fd);

            if(socket_fd != -1) {
                param.sockfd = socket_fd;
            }

            param.isBlock = false;
            param.sendBufferSize = 2048 * 1024;
            param.receiveBufferSize = 2048 * 1024;
            ALOGI("%s,isBlock=%d,sendBufferSize:%d,receiveBufferSize:%d",__FUNCTION__, param.isBlock, param.sendBufferSize, param.receiveBufferSize);
            param.dscp = mVideoConfigParam.network_info.dscp;
            param.priority = mVideoConfigParam.network_info.soc_priority;
            param.network_id =  mVideoConfigParam.network_info.network_id;
            strncpy(param.ifname, mVideoConfigParam.network_info.ifname,16);
            param.ifname[15] = '\0';
            param.uid =     mVideoConfigParam.network_info.uid;
            memcpy(param.local_address,mVideoConfigParam.network_info.local_rtp_address,sizeof(mVideoConfigParam.network_info.local_rtp_address));
            param.local_port = mVideoConfigParam.network_info.local_rtp_port;
            param.tag = mVideoConfigParam.network_info.tag;

            memcpy(param.peer_address,mVideoConfigParam.network_info.remote_rtp_address,sizeof(mVideoConfigParam.network_info.remote_rtp_address));
            param.peer_port = mVideoConfigParam.network_info.remote_rtp_port;

            mVideoRTPSocketWrapper->setParam(param);

            //config socket attributes: like blocking/non-blocking, socket buffer queue size
        }

        if((rtpPath & IMSMA_RTP_UPLINK) && !(mVideoActivePath & IMSMA_RTP_UPLINK)) {
            if(mVideoRTPSender.get()) {
                err = mVideoRTPSender->start();
            } else {
                sp<AMessage> notify = new AMessage(kWhatSenderNotify, mReflector);
                notify->setInt32("trackIndex",IMSMA_RTP_VIDEO);
                mVideoRTPSender = new RTPSender(mSSRC,mVideoRTPSocketWrapper,notify, mSimID,mOperatorID);
                ALOGI("new RTPSender_Video looper");
                mVideoSenderLooper = new ALooper;
                mVideoSenderLooper->setName("RTPSender_Video");
                mVideoSenderLooper->registerHandler(mVideoRTPSender);
                mVideoSenderLooper->start();
                //mVideoRTPSender->setNotify(mVideoRtpPacketNotify);
                mVideoRTPSender->setConfigParams(&mVideoConfigParam);
                err = mVideoRTPSender->start();
            }

            if(err != OK) {
                ALOGE("%s,uplink video track start Fail! err(%d)",__FUNCTION__,err);
                return err;
            }

            mActiveTrack |= IMSMA_RTP_VIDEO;
            mVideoActivePath |= IMSMA_RTP_UPLINK;
        }

        if((rtpPath & IMSMA_RTP_DOWNLINK) && !(mVideoActivePath & IMSMA_RTP_DOWNLINK)) {
            if(mRTPReceiver.get()) {
                //ToDo: RTPReceiver start by index
                err = mRTPReceiver->start(IMSMA_RTP_VIDEO);
            } else {
                sp<AMessage> notify = new AMessage(kWhatReceiverNotify, mReflector);
                mRTPReceiver = new RTPReceiver(notify, mSimID,mOperatorID);
                ALOGI("new RTPReceiver looper");
                mReceiverLooper = new ALooper;
                mReceiverLooper->setName("RTPReceiver");
                mReceiverLooper->registerHandler(mRTPReceiver);
                mReceiverLooper->start();

                mRTPReceiver->addStream(&mVideoConfigParam,mVideoRTPSocketWrapper,mVideoAccessUnitNotify,IMSMA_RTP_VIDEO);
                //mRTPReceiver->setNotify(mVideoAccessUnitNotify,mVideoRtpPacketNotify);
                //ToDo: need addStream ?
                //mRTPReceiver->setConfigParams(&mVideoConfigParam);
                err = mRTPReceiver->start(IMSMA_RTP_VIDEO);
            }

            if(err != OK) {
                ALOGE("%s,downlink video track start Fail! err(%d)",__FUNCTION__,err);
                return err;
            }

            mActiveTrack |= IMSMA_RTP_VIDEO;
            mVideoActivePath |= IMSMA_RTP_DOWNLINK;
#if 0
            //test FIR+
            requestDecoderRefreshPoint();
            //test FIR-
#endif

        }


        if(!mVideoRTCPSocketWrapper.get()) {
            mVideoRTCPSocketWrapper = new SocketWrapper();

            Sock_param_t param;
            param.protocol_version = mVideoConfigParam.network_info.remote_addr_type;
            param.sockfd= -1;
            //get param from mConfigParam
            // find the fd related to port number
            //fill to param
            int32_t socket_fd = findSocketFd(mVideoConfigParam.network_info.local_rtcp_port);
            ALOGI("%s,findSocketFd,rtcp local port:%d,fd:%d",\
                  __FUNCTION__,mVideoConfigParam.network_info.local_rtcp_port,socket_fd);

            if(socket_fd != -1) {
                param.sockfd = socket_fd;
            }

            param.isBlock = false;
            param.sendBufferSize = 512 * 1024;
            param.receiveBufferSize = 512 * 1024;

            param.dscp = mVideoConfigParam.network_info.dscp;
            param.priority = mVideoConfigParam.network_info.soc_priority;
            param.network_id =  mVideoConfigParam.network_info.network_id;
            strncpy(param.ifname, mVideoConfigParam.network_info.ifname, 16);
            param.ifname[15] = '\0';
            param.uid =     mVideoConfigParam.network_info.uid;
            memcpy(param.local_address,mVideoConfigParam.network_info.local_rtcp_address,sizeof(mVideoConfigParam.network_info.local_rtcp_address));
            param.local_port = mVideoConfigParam.network_info.local_rtcp_port;
            param.tag = mVideoConfigParam.network_info.tag;

            memcpy(param.peer_address,mVideoConfigParam.network_info.remote_rtcp_address,sizeof(mVideoConfigParam.network_info.remote_rtcp_address));
            param.peer_port = mVideoConfigParam.network_info.remote_rtcp_port;

            mVideoRTCPSocketWrapper->setParam(param);
            //config socket attributes: like blocking/non-blocking, socket buffer queue size
            //....

            //start listen RTCP callback from SocketWrapper
            mVideoRTCPSocketWrapper->setRxCallBack(this,videoRTCPPacketCallBack);
        }

        //ToDo...
        //whether need add a delay
        //avoid sending empty RR first
        //which cause the first Sender Info will be sent in the next SR
        // too late
        startScheduleRTCP(trackIndex);
    }

    ALOGI("%s,active track:0x%x, video active path:0x%x",__FUNCTION__,mActiveTrack,mVideoActivePath);

    return err;
}
status_t RTPController::onStop(int32_t rtpPath,int32_t trackIndex)
{
    ALOGI("%s,path:%d,track:%d",__FUNCTION__,rtpPath,trackIndex);

    if((trackIndex & IMSMA_RTP_VIDEO) && (mActiveTrack & IMSMA_RTP_VIDEO)) {
        if((rtpPath & IMSMA_RTP_UPLINK) && (mVideoActivePath & IMSMA_RTP_UPLINK)) {
            if(mVideoRTPSender.get()) {
                mVideoRTPSender->stop();
                mVideoActivePath &= ~IMSMA_RTP_UPLINK;
            }
        }

        if((rtpPath & IMSMA_RTP_DOWNLINK) && (mVideoActivePath & IMSMA_RTP_DOWNLINK)) {
            if(mRTPReceiver.get()) {
                mRTPReceiver->stop(IMSMA_RTP_VIDEO);
                mVideoActivePath &= ~IMSMA_RTP_DOWNLINK;
            }
        }

        if(mVideoActivePath == 0) {
            mActiveTrack &= ~IMSMA_RTP_VIDEO;
        }
    }

    ALOGI("%s,active track:0x%x, video active path:0x%x",__FUNCTION__,mActiveTrack,mVideoActivePath);

    return OK;
}
/*
status_t RTPController::onPause(uint8_t rtpPath,uint8_t trackIndex){
    ALOGI("%s,rtpPath(%d),track(%d)",__FUNCTION__,rtpPath,trackIndex);
    if(trackIndex & IMSMA_RTP_VIDEO){
        //ToDo: How to handle rtcp
        //ToDo: whether active path ,active track need refresh
#if 0
        if(!bKeepRTCP){
            stopScheduleRTCP(trackIndex);
#endif
        if(rtpPath & IMSMA_RTP_UPLINK){
            if(mVideoRTPSender.get()){
                mVideoRTPSender->pause();
            }
        }
        if(rtpPath & IMSMA_RTP_DOWNLINK){
            if(mRTPReceiver.get()){
                mRTPReceiver->pause(IMSMA_RTP_VIDEO);
            }
        }
    }
    return OK;
}
status_t RTPController::onResume(uint8_t rtpPath,uint8_t trackIndex){
    ALOGI("%s,rtpPath(%d),track(%d)",__FUNCTION__,rtpPath,trackIndex);
    if(trackIndex & IMSMA_RTP_VIDEO){
        if(rtpPath & IMSMA_RTP_UPLINK){
            if(mVideoRTPSender.get()){
                mVideoRTPSender->resume();
            }
        }
        if(rtpPath & IMSMA_RTP_DOWNLINK){
            if(mRTPReceiver.get()){
                mRTPReceiver->resume(IMSMA_RTP_VIDEO);
            }
        }
        //how to resume RTCP
        //ToDo: whether need take the last rtcp schedule time during hold on into consider
        // to congestion control,whether need postpone starting the new RTCP
        mVideoSendRTCPGeneration++; //To make the RTCP message during hold on over due
        startSendRTCPPacket(0,trackIndex); // peer will need the new RTCP info with SR to sync multi-track as soon as possible
        mStartedRTCPSchedule = true;
    }
    return OK;
}
*/
#if 0
status_t RTPController::onHoldOn(int32_t isHoldOnEnable)
{
    if(mVideoRTPSender.get()) {
        mVideoRTPSender->holdOn(isHoldOnEnable);
    }

    if(mAudioRTPSender.get()) {
        mAudioRTPSender->holdOn(isHoldOnEnable);
    }

    //ToDo:hold on will align to audio and video track simultaneously
    if(mRTPReceiver.get()) {
        mRTPReceiver->holdOn(isHoldOnEnable);
    }

    return OK;
}
#endif
#if 0
status_t RTPController::startScheduleRTCP(uint8_t trackIndex)
{
    ALOGI("%s,track(%d)",__FUNCTION__,trackIndex);
    sp<AMessage> msg = new AMessage(kWhatStartScheduleRTCP, id());

    msg->setInt32("trackIndex",trackIndex);

    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);

    return err;
}
status_t RTPController::stopScheduleRTCP(uint8_t trackIndex)
{
    ALOGI("%s,track(%d)",__FUNCTION__,trackIndex);
    sp<AMessage> msg = new AMessage(kWhatStopScheduleRTCP, id());

    msg->setInt32("trackIndex",trackIndex);

    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);

    return err;
}
#endif

int32_t RTPController::findSocketFd(uint16_t port)
{
    uint8_t fd_num = sizeof(mVideoConfigParam.network_info.socket_fds) /sizeof(uint32_t);

    for(uint8_t i = 0; i < fd_num; i++) {
        //find the port for each fd
        uint16_t fd_port = -1;
        int32_t fd = mVideoConfigParam.network_info.socket_fds[i];

        if(fd != -1) {
            struct sockaddr_in sock_Addr;
            socklen_t sock_AddrLen = sizeof(sock_Addr);

            int res = getsockname(
                          fd, (struct sockaddr *) &sock_Addr, (socklen_t*) &sock_AddrLen);
            ALOGD("%s,fd=%d",__FUNCTION__,fd);

            CHECK_GE(res, 0);

            fd_port = u16at((uint8_t*) &sock_Addr.sin_port);

            ALOGD("%s,fd_port=%d,port=%d",__FUNCTION__,fd_port,port);

            if(fd_port == port) {
                return fd;
            }
        }

    }

    //ToDo: What should return to represent
    return -1;
}

/***************************************************
//parse the  Sender Info to RTPReceiver
//parse out ,report block to RTPSender
//parse the SDES
//parse out the following Feedback message

//need check whether has report block info in SR
//if has report block,


//for SR, send to RTPReceiver
*****************************************************/

status_t RTPController::queueRTCPPacket(sp<ABuffer> &packet,uint8_t trackIndex)
{
    ALOGV("%s",__FUNCTION__);

    if(!packet.get()) {
        ALOGE("%s,packet is NULL",__FUNCTION__);
        return UNKNOWN_ERROR;
    }

    if(packet->size() < 4) {
        ALOGE("%s,rtcp packet too short(%zu)",__FUNCTION__,packet->size());
        return UNKNOWN_ERROR;
    }

#ifdef DEBUG_DUMP_RTCP_PACKET

    if(mRecvRTCPFd >= 0) {
        size_t real_write = write(mRecvRTCPFd,packet->data(),packet->size());
    }

#endif
    mReceiveRTCPPacket = true;

    //update m_avg_rtcp_size
    size_t rtcp_size = packet->size();
    m_avg_rtcp_size = rtcp_size /16 + 15 * m_avg_rtcp_size /16;
    ALOGI("%s,rtcp_size = %zu,new m_avg_rtcp_size = %d",\
          __FUNCTION__,rtcp_size,m_avg_rtcp_size);

    //for adaptation
    //scan whether has TMMBR
    bool contain_TMMBR = scanTMMBR(packet);

    //parse what kind of RTCP
    //iterate all the RTCP type
    uint32_t offset = 0;

    while(offset < packet->size()) {
        uint8_t* data = packet->data() + offset;
        uint32_t length = (u16at(&data[2]) + 1) * 4;

        if(packet->size()- offset < length) {
            ALOGE("%s,invalid rtcp packet(size:%zu,offset:%u,length:%u)",\
                  __FUNCTION__,packet->size(),offset,length);
            return UNKNOWN_ERROR;
        }

        uint8_t pt = data[1];
        ALOGV("%s,pt =%d",__FUNCTION__,pt);

        if(pt == 201) {
            uint8_t RC = data[0] & 0x1F; // 5 bit for RC
            ALOGV("RR,RC = %u",RC);

            uint8_t* rp_data = &data[8];
            uint32_t rp_offset = 0;

            for(uint8_t i = 0; i < RC; i++) {
                if(8 + rp_offset >= length) {
                    ALOGW("%s invalid RC: %u",__FUNCTION__,RC);
                    break;
                }

                uint8_t* rpblock_data = rp_data + rp_offset;
                //parse reception report
                uint32_t uSSRC = u32at(&rpblock_data[0]);

                if(mSSRC == uSSRC) {
                    //queue reception report to  RTPSender
                    sp<ABuffer> reportBlockBuffer = new ABuffer(20);
                    memset(reportBlockBuffer->data(),0,reportBlockBuffer->size());
                    memcpy(reportBlockBuffer->data(),&rpblock_data[4],reportBlockBuffer->size());
                    reportBlockBuffer->setRange(0,reportBlockBuffer->size());

                    if(trackIndex == IMSMA_RTP_VIDEO) {
                        if(mVideoRTPSender.get()) {
                            sp<AMessage> meta = reportBlockBuffer->meta();
                            meta->setInt32("contain_tmmbr",contain_TMMBR);
                            mVideoRTPSender->processReportBlock(reportBlockBuffer);
                        }
                    }
                }

                rp_offset += 24;
            }
        } else if(pt == 200) {
            uint32_t uSSRC = u32at(&data[4]);

            ALOGV("queue SenderInfo from remote to RTPReceiver");

            sp<ABuffer> senderInfoBuffer = new ABuffer(20);
            memset(senderInfoBuffer->data(),0,senderInfoBuffer->size());
            memcpy(senderInfoBuffer->data(),&data[8],senderInfoBuffer->size());
            senderInfoBuffer->setRange(0,senderInfoBuffer->size());

            if(mRTPReceiver.get()) {
                mRTPReceiver->processSenderInfo(senderInfoBuffer,uSSRC,trackIndex);
            }

            uint8_t RC = data[0] & 0x1F; // 5 bit for RC
            ALOGV("SR,RC = %u",RC);

            uint8_t* rp_data = &data[28];
            uint32_t rp_offset = 0;

            for(uint8_t i = 0; i < RC; i++) {
                if(28 + rp_offset >= length) {
                    ALOGW("%s invalid RC: %u",__FUNCTION__,RC);
                    break;
                }

                uint8_t* rpblock_data = rp_data + rp_offset;
                //parse reception report
                uint32_t uSSRC = u32at(&rpblock_data[0]);

                if(mSSRC == uSSRC) {
                    //queue reception report to  RTPSender
                    sp<ABuffer> reportBlockBuffer = new ABuffer(20);
                    memset(reportBlockBuffer->data(),0,reportBlockBuffer->size());
                    memcpy(reportBlockBuffer->data(),&rpblock_data[4],reportBlockBuffer->size());
                    reportBlockBuffer->setRange(0,reportBlockBuffer->size());

                    if(trackIndex == IMSMA_RTP_VIDEO) {
                        if(mVideoRTPSender.get()) {
                            sp<AMessage> meta = reportBlockBuffer->meta();
                            meta->setInt32("contain_tmmbr",contain_TMMBR);
                            mVideoRTPSender->processReportBlock(reportBlockBuffer);
                        }
                    }
                }

                rp_offset += 24;
            }
        } else if(pt == 206) {
            uint8_t FMT = data[0] & 0x1F;
            ALOGW("receive payload-specific FB message(%d) from peer",FMT);
            sp<ABuffer> psfb = new ABuffer(data,length);

            switch(FMT) {
            case 1: //PLI
                onReceivePLI(psfb);
                break;
            case 2://SLI
                onReceiveSLI(psfb);
                break;
            case 3://RPSI
                break;
            case 4://FIR
                //ToDo
                //for FIR notify to MA to encode refresh point(sps+pps+idr for H.264)
                onReceiveFIR(psfb);
                break;
            default:
                break;
            }
        } else if(pt == 205) {
            uint8_t FMT = data[0] & 0x1F;
            ALOGW("receive transport layer FB message(%d) from peer",FMT);
            sp<ABuffer> tsfb = new ABuffer(data,length);

            switch(FMT) {
            case 1: //Generic NACK
                onReceiveGenericNACK(tsfb);
                break;
            case 3: //TMMBR
                onReceiveTMMBR(tsfb);
                break;
            case 4: //TMMBN
                onReceiveTMMBN(tsfb);
                break;
            default:
                break;
            }
        } else if(pt == 202) {    //SDES
            //ToDo
        } else {
            ALOGD("unsupported PT: %d",pt);

        }

        offset += length;
    }

    //ToDo:
    //need check whether the packet size == sum of all the rtcp length
    //if not, it's not valid rtcp packet
    if(offset != packet->size()) {
        ALOGE("%s,invalid rtcp packet,offset(%u),packet size(%zu)",\
              __FUNCTION__,offset,packet->size());
    }

    return OK;
}

void RTPController::startRtcpTimer()
{
    if(mRTCPTimer == true) {
        return ;
    }

    mRTCPTimer = true;
    mReceiveRTCPPacket = false;
    mNoRTCPCount = 0;
    mRTCPTimerInterval = 19; //defualt value

    char adp_param[PROPERTY_VALUE_MAX];
    memset(adp_param,0,sizeof(adp_param));

    if(property_get("persist.vendor.vt.rtcp.timer", adp_param, NULL)) {
        mRTCPTimerInterval = atoi(adp_param);
    }

    ALOGI("%s,delayUs(%d s)",__FUNCTION__, mRTCPTimerInterval);

    sp<AMessage> msg = new AMessage(kWhatRTCPTimer,mReflector);
    msg->post(1000000);
}

void RTPController::stopRtcpTimer()
{
    mRTCPTimer = false;
}

status_t RTPController::setHandoverState(int32_t state)
{
    ALOGI("%s,state(%d)",__FUNCTION__,state);

    sp<AMessage> msg = new AMessage(kWhatSetHoState, mReflector);
    msg->setInt32("state", state);

    if(state != 0) {
        msg->post();
    } else {
        msg->post(2000000);
    }

    return OK;
}


status_t RTPController::startScheduleRTCP(uint8_t trackIndex)
{
    ALOGI("%s,track(%d)",__FUNCTION__,trackIndex);
    startRtcpTimer();

    //ToDo:need change mStartedRTCPSchedule to track parameter
    if(!mStartedRTCPSchedule) {
        //init mNextScheduleRTCPinfo
        mNextScheduleRTCPinfo.mFeedBackFlag = 0;
        mNextScheduleRTCPinfo.mRTCPMode = REGULAR_MODE;
        int64_t nowUs = ALooper::GetNowUs();
        mNextScheduleRTCPinfo.mRTCPScheduleTimeUs = nowUs;

        m_Trr_last = 0;
        m_AllowEarlyRtcp = true;
        mRTCP_tp = nowUs;
        mLastRTCP_videoRTPSentCount = 0;

        m_avg_rtcp_size = 52;//initial value

        startSendRTCPPacket(0,trackIndex);
        mStartedRTCPSchedule = true;

/*
        //always send FIR at begining
        sp<AMessage> errInfo = new AMessage;
        errInfo->setInt32("err", IRTPController::DecErr_FullIntraRequest);
        sp<AMessage> msg = new AMessage(kWhatFeedBackDecErr, mReflector);
        msg->setMessage("err_info",errInfo);
        msg->post();
*/
    }

    return OK;
}
status_t RTPController::stopScheduleRTCP(uint8_t trackIndex)
{
    ALOGI("%s,track(%d)",__FUNCTION__,trackIndex);
    stopRtcpTimer();

    if(mStartedRTCPSchedule) {
        mVideoSendRTCPGeneration++;
        mStartedRTCPSchedule = false;

        //clear AVPF related buffer
        mSliBuffer = NULL;
        mGenericNACKBuffer = NULL;
    }

    return OK;
}

status_t RTPController::startSendRTCPPacket(int64_t delayUs,uint8_t trackIndex)
{
    ALOGI("%s,delayUs(%lld)",__FUNCTION__, (long long) delayUs);
    sp<AMessage> msg = new AMessage(kWhatSendRTCP,mReflector);
    msg->setInt32("generation",mVideoSendRTCPGeneration);
    msg->setInt32("trackIndex",trackIndex);
    msg->post(delayUs);
    return OK;
}
/*
status_t RTPController::onSendRTCP(const sp<AMessage> &msg){
    ALOGI("%s ++",__FUNCTION__);
    int32_t trackIndex = IMSMA_RTP_VIDEO;
    msg->findInt32("trackIndex",&trackIndex);
    if(trackIndex & IMSMA_RTP_VIDEO){
        sp<ABuffer> buffer = new ABuffer(mMTUSize);
        memset(buffer->data(),0,buffer->size());
        buffer->setRange(0,0);
        //add common fixed header
        //if there is no data transmission and reception to report
        //we still need a emtpy RR(RC=0,type=201)
        uint8_t* data = buffer->data() + buffer->size();
        data[0] = 0x80; //V=2,P=0,RC=0
        data[1] = 201; //default is RR,

        data[4] = mSSRC >> 24;
        data[5] = (mSSRC >> 16) & 0xff;
        data[6] = (mSSRC >> 8)& 0xff;
        data[7] = mSSRC & 0xff;

        buffer->setRange(buffer->offset(),buffer->size() + 8);

        //ToDo: need check whether packets sent out during this interval
        uint32_t uRTPSentCount = 0;
        //ToDo: whether need check RTPSender status,
        //if stopped/paused, no need get the RTPSentCount
        if(mVideoRTPSender.get()){
            uRTPSentCount = mVideoRTPSender->getRTPSentCount();
            if(uRTPSentCount > mLastRTCPinfo.mRTPSentCount){
                //if has rtp packets sent during the interval
                data[1] = 200; //chage to SR
                if(mVideoRTPSender.get()){
                    mVideoRTPSender->addSenderInfo(buffer);
                }
            }
        }
        uint8_t RC = 0;
        if(mRTPReceiver.get()){
             RC = mRTPReceiver->addReceiveReportBlocks(buffer,trackIndex);
        }

        //update the RC
        data[0] = data[0] | (RC & 0x1f);

        uint16_t length = buffer->size() / 4 - 1;

        //update length
        data[2] = (length >> 8) & 0xff;
        data[3] = length & 0xff;


        addSDES(mCName,buffer);

        //ToDo:
        //add FB message on demand
        if(mFeedBackFlag & kKeyFIR){
            addFIR(buffer);
        }
        if(mFeedBackFlag & kKeySLI){
            addSLI(buffer);
        }
        if(mFeedBackFlag & kKeyPLI){
            addPLI(buffer);
        }
        if(mFeedBackFlag & kKeyGNACK ){
            addNACK(buffer);
        }

        mLastRTCPinfo.mRTCPSentTimeUs = ALooper::GetNowUs();
        mLastRTCPinfo.mRTPSentCount = uRTPSentCount;

        sp<AMessage> rtcp_meta = buffer->meta();

        //need notify to RTPController first,
        //to check whether receiver has work
        //if so ,need add receive report block to SR
        //if send-only, RTPController will notify the SR directly to SocketWrapper

        //rtcp socket suppose with little sendrequest
        //should not send fail
        //if send fail happen, drop this RTCP packet directly, waiting for next RTCP scheduling
        int write_size = 0;
        if(mVideoRTCPSocketWrapper.get()){
            write_size = mVideoRTCPSocketWrapper->writeSock(buffer);
        }
        if(write_size < 0){
            ALOGE("%s,packet send fail",__FUNCTION__);
            //no need re-send for rtcp
        }else if((uint32_t)write_size != (buffer->size())){
            ALOGE("%s,writeSock write partial data:%d/%d",\
                __FUNCTION__,write_size,buffer->size());
        }

#ifdef DEBUG_DUMP_RTCP_PACKET
        if(mSendRTCPFd >= 0){
            size_t real_write = write(mSendRTCPFd,buffer->data(),buffer->size());
        }
#endif
        //next rtcp
        //ToDo need change the next rtcp schedule time if has fb
        msg->post(mVideoSendRTCPIntervalUs);

    }
    ALOGI("%s --",__FUNCTION__);
    return OK;
}
*/
int64_t RTPController::calculateNextInterval()
{
    ALOGV("%s ++",__FUNCTION__);

    int64_t t_rr = 0;
    uint64_t rs = mVideoConfigParam.rtcp_sender_bandwidth; //bps
    uint64_t rr = mVideoConfigParam.rtcp_receiver_bandwidth; //bps

    int64_t rtcp_bandwidth = 0;

    uint8_t uTotalMember = 2;
    uint8_t uSenderNum = 2;
    uint8_t uReceiverNum = 0;

    bool we_sent = true;

    if((rs == 0) && (rr == 0)) {
        ALOGI("%s,rs=0,rr=0,not allow sending RTCP",__FUNCTION__);
        t_rr = -1;
        return t_rr;
    }

    if(mVideoActivePath == IMSMA_RTP_UP_DOWN_LINK) {
        uSenderNum = 2;
        uReceiverNum = 0;
        we_sent = true;
    } else if(mVideoActivePath == IMSMA_RTP_UPLINK) {
        uSenderNum = 1;
        uReceiverNum = 1;
        we_sent = true;
        //we are sender
    } else if(mVideoActivePath == IMSMA_RTP_DOWNLINK) {
        uSenderNum = 1;
        uReceiverNum = 1;
        we_sent = false;
        //we are receiver
    } else {
        ALOGI("%s,no active path",__FUNCTION__);
        //ToDo need check whether is OK
        uSenderNum = 0;
        uReceiverNum = 0;
        we_sent = false;
    }

    double dSenderNumProp = (double) uSenderNum/uTotalMember;
    double dSenderBwProp = (double) rs/ (rs+rr);

    if(dSenderNumProp < dSenderBwProp) {
        if(uSenderNum == 0 || uReceiverNum == 0) {
            rtcp_bandwidth = (rs + rr) / uTotalMember;
        } else {
            if(we_sent) {
                rtcp_bandwidth = rs/uSenderNum;
            } else {
                rtcp_bandwidth = rr/uReceiverNum;
            }
        }
    } else {
        rtcp_bandwidth = (rs + rr) / uTotalMember;
    }

    if(rtcp_bandwidth <= 0) {
        ALOGE("%s,rtcp bw <= 0",__FUNCTION__);
        return -1;
    }

    //ToDo, the rr,rs contain udp & ip header
    //but m_avg_rtcp_size without udp & ip header
    //so the rr, rs need sub the udp ip hedder size
    //estimate 200 bytes for upd+ip+ipSec header
    //8bytes for udp;20bytes for ipv4/40bytes for ipv6
    t_rr = (int64_t) 8 * (m_avg_rtcp_size + IMSMA_IP_IPSec_UDP_HEADER) * 1000000ll /rtcp_bandwidth;     //us

    ALOGI("%s --,t_rr=%" PRId64 "us,rtcp bw=%" PRId64 "bps",__FUNCTION__,t_rr,rtcp_bandwidth);
    return t_rr;
}

status_t RTPController::onSendRTCP(const sp<AMessage> &msg)
{
    ALOGV("%s ++",__FUNCTION__);

    int32_t trackIndex = IMSMA_RTP_VIDEO;
    msg->findInt32("trackIndex",&trackIndex);

    if(trackIndex & IMSMA_RTP_VIDEO) {
        bool bDrop = false;
        int64_t tc = ALooper::GetNowUs();
        int64_t tn = ALooper::GetNowUs();
        //mFeedBackFlag = mNextScheduleRTCPinfo.mFeedBackFlag;
        //calculate t_rr
        int64_t t_rr = calculateNextInterval();

        if(t_rr < 0) {
            ALOGI("%s,t_rr < 0, not allow RTCP, stop schedule rtcp",__FUNCTION__);
            stopScheduleRTCP(IMSMA_RTP_VIDEO);
            return OK;
        }

        if(mNextScheduleRTCPinfo.mRTCPMode == REGULAR_MODE) {
            if(!mNextScheduleRTCPinfo.mFeedBackFlag) {
                //if is a regular RTCP not containing any FBs
                //"trr-int" will influence whether can send this RTCP
                if((m_Trr_interval > 0) && (tc - m_Trr_last < m_Trr_interval)) {
                    //need cancel this RTCP
                    bDrop = true;
                    //keep m_Trr_last not change
                    //change allow early RTCP
                    m_AllowEarlyRtcp = true;
                    ALOGD("%s,regular,no fb,discard by trr-int(c:%" PRId64 "us,last:%" PRId64 "us,trr-int:%" PRId64 "us)",\
                          __FUNCTION__,tc,m_Trr_last,m_Trr_interval);
                } else {
                    bDrop = false;
                    m_Trr_last = tc;
                    m_AllowEarlyRtcp = true;
                    ALOGD("%s,regular,no fb",__FUNCTION__);
                }
            } else {
                //will not influence by trr-int
                bDrop = false;
                m_AllowEarlyRtcp = true;
                ALOGD("%s,regular,contain fb,ignore trr-int",__FUNCTION__);
            }

            //even though the RTCP is canceled
            //should change tp to tc
            mRTCP_tp = tc;
            //update the item for the next RTCP
            tn = mRTCP_tp + t_rr;
        } else {

            bDrop = false;
            m_AllowEarlyRtcp = false;
            //keep mRTCP_tp not change
            //keep m_Trr_last not change

            //update the item for the next RTCP
            //ToDo:even spec define the tn = last rtcp + 2*t_rr
            //but it maybe with little difference
            //so we use the following fromula without record the last rtcp info
            tn = mRTCP_tp + t_rr;

            //tn = mLastRTCPinfo.mRTCPScheduleTimeUs+ 2*t_rr;
            ALOGD("%s,immediately",__FUNCTION__);
        }

        if(!bDrop) {
            //need update mNextScheduleRTCPinfo.mRTPSentCount in makeRTCP
            //need update mLastSentRTCPinfo in makeRTCP
            makeAndsendRTCP(trackIndex);
        }

        //update next RTCP
        mNextScheduleRTCPinfo.mRTCPScheduleTimeUs = tn;
        mNextScheduleRTCPinfo.mRTCPMode = REGULAR_MODE;
        //mNextScheduleRTCPinfo.mFeedBackFlag = 0;

        //schedule the next RTCP
        uint64_t delayUs = mNextScheduleRTCPinfo.mRTCPScheduleTimeUs - tc;

        //next rtcp
        //ToDo need change the next rtcp schedule time if has fb
        msg->post(delayUs);
        ALOGI("%s,next RTCP delayUs=%" PRId64 "Us",__FUNCTION__,delayUs);
    }

    ALOGV("%s --",__FUNCTION__);
    return OK;
}
status_t RTPController::makeAndsendRTCP(uint8_t trackIndex)
{
    if(trackIndex & IMSMA_RTP_VIDEO) {
        sp<ABuffer> buffer = new ABuffer(mMTUSize);
        memset(buffer->data(),0,buffer->size());
        buffer->setRange(0,0);
        //add common fixed header
        //if there is no data transmission and reception to report
        //we still need a emtpy RR(RC=0,type=201)
        uint8_t* data = buffer->data() + buffer->size();
        data[0] = 0x80; //V=2,P=0,RC=0
        data[1] = 201; //default is RR,

        data[4] = mSSRC >> 24;
        data[5] = (mSSRC >> 16) & 0xff;
        data[6] = (mSSRC >> 8) & 0xff;
        data[7] = mSSRC & 0xff;

        buffer->setRange(buffer->offset(),buffer->size() + 8);

        //ToDo: need check whether packets sent out during this interval
        uint32_t uRTPSentCount = 0;

        //ToDo: whether need check RTPSender status,
        //if stopped/paused, no need get the RTPSentCount
        if(mVideoRTPSender.get() && (mVideoActivePath & IMSMA_RTP_UPLINK)) {
            uRTPSentCount = mVideoRTPSender->getRTPSentCount();

            if(uRTPSentCount > mLastRTCP_videoRTPSentCount) {
                //if has rtp packets sent during the interval
                data[1] = 200; //chage to SR

                if(mVideoRTPSender.get()) {
                    mVideoRTPSender->addSenderInfo(buffer);
                }
            }
        }

        uint8_t RC = 0;

        if(mRTPReceiver.get() && (mVideoActivePath & IMSMA_RTP_DOWNLINK)) {
            RC = mRTPReceiver->addReceiveReportBlocks(buffer,IMSMA_RTP_VIDEO);
        }

        //update the RC
        data[0] = data[0] | (RC & 0x1f);

        uint16_t length = buffer->size() / 4 - 1;

        //update length
        data[2] = (length >> 8) & 0xff;
        data[3] = length & 0xff;

        addSDES(mCName,buffer);

        //ToDo:
        //add FB message on demand
        //Whether can combine the AVPF messages?
        uint32_t uFeedBackFlag = mNextScheduleRTCPinfo.mFeedBackFlag;

        if(mRTPReceiver.get() && (mVideoActivePath & IMSMA_RTP_DOWNLINK)) {
            //if downlink path stopped, no need handle the old feeback request

            if(uFeedBackFlag & kKeyFIR) {
                addFIR(buffer);
            }

            if(uFeedBackFlag & kKeySLI) {
                addSLI(buffer);
            }

            if(uFeedBackFlag & kKeyPLI) {
                addPLI(buffer);
            }

            if(uFeedBackFlag & kKeyGNACK) {
                addNACK(buffer);
            }

            if(uFeedBackFlag & kKeyTMMBR) {
                addTMMBR(buffer);
            }
        }

        if(uFeedBackFlag & kKeyTMMBN) {
            addTMMBN(buffer);

        }

        mLastRTCP_videoRTPSentCount = uRTPSentCount;
        //mLastSentRTCPinfo.mRTCPSentTimeUs = ALooper::GetNowUs();

        //update m_avg_rtcp_size
        size_t rtcp_size = buffer->size();
        m_avg_rtcp_size = rtcp_size /16 + 15 * m_avg_rtcp_size /16;
        ALOGI("%s,rtcp_size = %zu,new m_avg_rtcp_size = %d",\
              __FUNCTION__,rtcp_size,m_avg_rtcp_size);

        sp<AMessage> rtcp_meta = buffer->meta();

        //need notify to RTPController first,
        //to check whether receiver has work
        //if so ,need add receive report block to SR
        //if send-only, RTPController will notify the SR directly to SocketWrapper

        //rtcp socket suppose with little sendrequest
        //should not send fail
        //if send fail happen, drop this RTCP packet directly, waiting for next RTCP scheduling
        int write_size = 0;

        if(mVideoRTCPSocketWrapper.get()) {
            write_size = mVideoRTCPSocketWrapper->writeSock(buffer);
        }

        if(write_size < 0) {
            ALOGE("%s,packet send fail",__FUNCTION__);
            //no need re-send for rtcp
        } else if((uint32_t) write_size != (buffer->size())) {
            ALOGE("%s,writeSock write partial data:%d/%zu",\
                  __FUNCTION__,write_size,buffer->size());
        }

#ifdef DEBUG_DUMP_RTCP_PACKET

        if(mSendRTCPFd >= 0) {
            size_t real_write = write(mSendRTCPFd,buffer->data(),buffer->size());
        }

#endif
    } else {
        ALOGE("%s,track(%d) not support",__FUNCTION__,trackIndex);
    }

    return OK;

}

void RTPController::addSDES(const AString& cname, const sp<ABuffer> &buffer)
{
    ALOGI("%s",__FUNCTION__);
    int32_t size = cname.size() + 1 + 2 + 8; // 8:header, 1:CANME,1:length, 1: '/0'

    size = (size + 3) & (uint32_t)(-4);

    if(buffer->size() + size > buffer->capacity()) {
        ALOGW("RTCP buffer too small to accomodate SDES.");
        return;
    }

    uint8_t *data = buffer->data() + buffer->size();
    data[0] = 0x80 | 1;
    data[1] = 202;  // SDES

    size_t numWords = (size / 4) - 1;
    data[2] = numWords >> 8;
    data[3] = numWords & 0xff;

    data[4] = mSSRC >> 24;
    data[5] = (mSSRC >> 16) & 0xff;
    data[6] = (mSSRC >> 8) & 0xff;
    data[7] = mSSRC & 0xff;

    int32_t offset = 8;

    data[offset++] = 1;  // CNAME
    data[offset++] = cname.size();

    memcpy(&data[offset], cname.c_str(), cname.size());
    offset += cname.size();

    while(offset < size) {
        data[offset++] = 0;
    }

    buffer->setRange(buffer->offset(), buffer->size() + size);
}
void RTPController::addFIR(const sp<ABuffer> &buffer)
{
    ALOGI("%s",__FUNCTION__);

    if(buffer->size() + 20 > buffer->capacity()) {
        ALOGW("RTCP buffer too small to accomodate FIR.");
        return;
    }

    if((!mRTPReceiver.get()) || !mVideoPeerSSRC_set) {
        ALOGW("no packet received,ssrc hasn't set");
        return;
    }

    uint8_t *data = buffer->data() + buffer->size();

    data[0] = 0x80 | 4; // FMT=4
    data[1] = 206;  // PSFB
    data[2] = 0; //length
    data[3] = 4;


    data[4] = mSSRC >> 24;
    data[5] = (mSSRC >> 16) & 0xff;
    data[6] = (mSSRC >> 8) & 0xff;
    data[7] = mSSRC & 0xff;

    data[8] = 0x00;  // SSRC of media source (unused)
    data[9] = 0x00;
    data[10] = 0x00;
    data[11] = 0x00;


    data[12] = mVideoPeerSSRC >> 24;
    data[13] = (mVideoPeerSSRC >> 16) & 0xff;
    data[14] = (mVideoPeerSSRC >> 8) & 0xff;
    data[15] = mVideoPeerSSRC & 0xff;

    data[16] = mFIRLastSeqNum;  // Seq Nr.

    data[17] = 0x00;  // Reserved
    data[18] = 0x00;
    data[19] = 0x00;

    //if there are multi FIR request for each src
    //combine the FCI here
    //and update length field


    buffer->setRange(buffer->offset(), buffer->size() + 20);

    //now we just send once
    mNextScheduleRTCPinfo.mFeedBackFlag = mNextScheduleRTCPinfo.mFeedBackFlag & (~kKeyFIR);
}
void RTPController::addSLI(const sp<ABuffer> &buffer)
{
    ALOGI("%s ++",__FUNCTION__);

    if((buffer->size() + 12 + mSliBuffer->size()) > buffer->capacity()) {
        ALOGW("RTCP buffer too small to accomodate SLI.");
        return;
    }

    if((!mRTPReceiver.get()) || !mVideoPeerSSRC_set) {
        ALOGW("no packet received,ssrc hasn't set");
        return;
    }

    uint8_t *data = buffer->data() + buffer->size();

    uint8_t sli_fci_num = mSliBuffer->size() /4;
    uint16_t length = 2 + sli_fci_num;

    data[0] = 0x80 | 2; // FMT=2
    data[1] = 206;  // PSFB

    data[2] = (length >> 8) & 0x00ff;
    data[3] = length & 0x00ff;
    /*
        data[2] = 0; //length
        data[3] = 3;
    */

    data[4] = mSSRC >> 24;
    data[5] = (mSSRC >> 16) & 0xff;
    data[6] = (mSSRC >> 8) & 0xff;
    data[7] = mSSRC & 0xff;


    // SSRC of media source
    data[8] = mVideoPeerSSRC >> 24;
    data[9] = (mVideoPeerSSRC >> 16) & 0xff;
    data[10] = (mVideoPeerSSRC >> 8) & 0xff;
    data[11] = mVideoPeerSSRC & 0xff;

    /*
    //first:13bit, number:13bit, pictureID:6bit
    uint16_t first = mSLIinfo.mFirstMB;
    uint16_t number = mSLIinfo.mNumberMB;
    uint8_t pictureID = mSLIinfo.mPictureID;

    first = first & 0x1FFF; //extract the last 13 bit
    number = number & 0X1FFF; //extract the last 13bit
    pictureID = pictureID & 0x3F;

    data[12] = first >> 5;
    data[13] = (first & 0x001F) << 3;
    data[13] = data[13]| (number >> 10);
    data[14] = (number >> 2) & 0x00FF;
    data[15] = (number & 0x0003) << 6;
    data[15] = data[15] | pictureID;
    */

    //decoder notify SLI FCI buffer directly
    memcpy((void*) &data[12], (void*) mSliBuffer->data(),mSliBuffer->size());

    //ALOGI("%s --,sli fci(0x%x)",__FUNCTION__,u32at(&data[12]));
    buffer->setRange(buffer->offset(), buffer->size() + 12 + mSliBuffer->size());

    mNextScheduleRTCPinfo.mFeedBackFlag &= (~kKeySLI);

    mSliBuffer.clear();
    ALOGI("%s --,sli fci num(%d)",__FUNCTION__,sli_fci_num);
}

void RTPController::addPLI(const sp<ABuffer> &buffer)
{
    ALOGI("%s ++",__FUNCTION__);

    if(buffer->size() + 12 > buffer->capacity()) {
        ALOGW("RTCP buffer too small to accomodate SLI.");
        return;
    }

    if((!mRTPReceiver.get()) || !mVideoPeerSSRC_set) {
        ALOGW("no packet received,ssrc hasn't set");
        return;
    }

    uint8_t *data = buffer->data() + buffer->size();

    data[0] = 0x80 | 1; // FMT=1
    data[1] = 206;  // PSFB
    data[2] = 0; //length
    data[3] = 2;

    data[4] = mSSRC >> 24;
    data[5] = (mSSRC >> 16) & 0xff;
    data[6] = (mSSRC >> 8) & 0xff;
    data[7] = mSSRC & 0xff;

    // SSRC of media source
    data[8] = mVideoPeerSSRC >> 24;
    data[9] = (mVideoPeerSSRC >> 16) & 0xff;
    data[10] = (mVideoPeerSSRC >> 8) & 0xff;
    data[11] = mVideoPeerSSRC & 0xff;

    buffer->setRange(buffer->offset(), buffer->size() + 12);

    mNextScheduleRTCPinfo.mFeedBackFlag &= (~kKeyPLI);

    ALOGI("%s --",__FUNCTION__);
}
void RTPController::addNACK(const sp<ABuffer> &buffer)
{
    ALOGI("%s ++",__FUNCTION__);

    if((buffer->size() + 12 + mGenericNACKBuffer->size()) > buffer->capacity()) {
        ALOGW("RTCP buffer too small to accomodate NACK.");
        return;
    }

    if((!mRTPReceiver.get()) || !mVideoPeerSSRC_set) {
        ALOGW("no packet received,ssrc hasn't set");
        return;
    }

    uint8_t *data = buffer->data() + buffer->size();

    uint8_t nack_fci_num = mGenericNACKBuffer->size() /4;
    uint16_t length = 2 + nack_fci_num;

    data[0] = 0x80 | 1; // FMT=2
    data[1] = 205;  // RTPFB

    data[2] = (length >> 8) & 0x00ff;
    data[3] = length & 0x00ff;
    /*
        data[2] = 0; //length
        data[3] = 3;
    */

    data[4] = mSSRC >> 24;
    data[5] = (mSSRC >> 16) & 0xff;
    data[6] = (mSSRC >> 8) & 0xff;
    data[7] = mSSRC & 0xff;


    // SSRC of media source
    data[8] = mVideoPeerSSRC >> 24;
    data[9] = (mVideoPeerSSRC >> 16) & 0xff;
    data[10] = (mVideoPeerSSRC >> 8) & 0xff;
    data[11] = mVideoPeerSSRC & 0xff;


    //add NACK FCI buffer
    memcpy((void*) &data[12], (void*) mGenericNACKBuffer->data(),mGenericNACKBuffer->size());

    //ALOGI("%s --,sli fci(0x%x)",__FUNCTION__,u32at(&data[12]));
    buffer->setRange(buffer->offset(), buffer->size() + 12 + mGenericNACKBuffer->size());

    mNextScheduleRTCPinfo.mFeedBackFlag &= (~kKeyGNACK);
    mGenericNACKBuffer.clear();

    ALOGI("%s --,nack fci num(%d)",__FUNCTION__,nack_fci_num);


}

void RTPController::addTMMBR(const sp<ABuffer> &buffer)
{
    ALOGI("%s ++",__FUNCTION__);

    if(buffer->size() + 20 > buffer->capacity()) {
        ALOGW("RTCP buffer too small to accomodate TMMBR.");
        return;
    }

    if((!mRTPReceiver.get()) || !mVideoPeerSSRC_set) {
        ALOGW("no packet received,ssrc hasn't set");
        return;
    }

    uint8_t *data = buffer->data() + buffer->size();

    data[0] = 0x80 | 3; // FMT=3
    data[1] = 205;  // PTPFB
    data[2] = 0; //length
    data[3] = 4;


    data[4] = mSSRC >> 24;
    data[5] = (mSSRC >> 16) & 0xff;
    data[6] = (mSSRC >> 8) & 0xff;
    data[7] = mSSRC & 0xff;

    data[8] = 0x00;  // SSRC of media source (unused)
    data[9] = 0x00;
    data[10] = 0x00;
    data[11] = 0x00;


    data[12] = mVideoPeerSSRC >> 24;
    data[13] = (mVideoPeerSSRC >> 16) & 0xff;
    data[14] = (mVideoPeerSSRC >> 8) & 0xff;
    data[15] = mVideoPeerSSRC & 0xff;

    //ToDo:
    //if repeat TMMBR in regular compound RTCP
    // need use new tmmbr info
    if(m_isWaitingTMMBN) {
        //get new tmmbr info
        const sp<ABuffer> _newTMMBRbuffer = mRTPReceiver->getNewTMMBRInfo();

        if(_newTMMBRbuffer.get()) {
            mTMMBRBuffer = _newTMMBRbuffer;
        }
    }

    memcpy((void*) &data[16], (void*) mTMMBRBuffer->data(),mTMMBRBuffer->size());

    //only consider one tmmbr_fci case
    buffer->setRange(buffer->offset(), buffer->size() + 20);

    m_isWaitingTMMBN = true;
    ALOGI("%s --: feeback_flag = 0x%x",__FUNCTION__,mNextScheduleRTCPinfo.mFeedBackFlag);
}

void RTPController::addTMMBN(const sp<ABuffer> &buffer)
{
    ALOGI("%s ++",__FUNCTION__);

    if(buffer->size() + 20 > buffer->capacity()) {
        ALOGW("RTCP buffer too small to accomodate TMMBN.");
        return;
    }

    uint8_t *data = buffer->data() + buffer->size();

    data[0] = 0x80 | 4; // FMT=4
    data[1] = 205;  // PTPFB
    data[2] = 0; //length
    data[3] = 4;


    data[4] = mSSRC >> 24;
    data[5] = (mSSRC >> 16) & 0xff;
    data[6] = (mSSRC >> 8) & 0xff;
    data[7] = mSSRC & 0xff;

    data[8] = 0x00;  // SSRC of media source (unused)
    data[9] = 0x00;
    data[10] = 0x00;
    data[11] = 0x00;


    data[12] = mSSRC >> 24;
    data[13] = (mSSRC >> 16) & 0xff;
    data[14] = (mSSRC >> 8) & 0xff;
    data[15] = mSSRC & 0xff;


    memcpy((void*) &data[16], (void*) mTMMBNBuffer->data(),mTMMBNBuffer->size());

    //only consider one tmmbn_fci case
    buffer->setRange(buffer->offset(), buffer->size() + 20);

    mNextScheduleRTCPinfo.mFeedBackFlag &= (~kKeyTMMBN);
    mTMMBNBuffer.clear();

    ALOGI("%s --: feeback_flag = 0x%x",__FUNCTION__,mNextScheduleRTCPinfo.mFeedBackFlag);
}


void RTPController::updateSSRC(uint32_t newSSRC)
{
    ALOGI("%s",__FUNCTION__);
    mSSRC = newSSRC;

    if(mVideoRTPSender.get()) {
        mVideoRTPSender->updateSSRC(newSSRC);
    }
}

void RTPController::onReceiveFIR(const sp<ABuffer> buffer)
{
    ALOGI("%s",__FUNCTION__);
    uint8_t* data = buffer->data();
    uint8_t FMT = data[0] & 0x1F;
    uint8_t pt = data[1];

    if((FMT != 4) || (pt != 206)) {
        ALOGE("%s,not valid FIR(FMT:%d,pt:%d)",__FUNCTION__,FMT,pt);
        return;
    }

    //there are maybe many FIR request for different sender
    //we just need handle the related fir request with the same ssrc of sender
    uint32_t offset = 12;//seek to FCI
    uint32_t ssrc = 0;
    uint8_t seqNum = 0;

    while(offset < buffer->size()) {
        uint8_t* fci_data = data + offset;
        ssrc = u32at(fci_data);

        //if(ssrc == mVideoRTPSender->getSSRC()){
        if(ssrc == mSSRC) {
            seqNum = fci_data[4];

            if(mVideoRTPSender.get()) {
                mVideoRTPSender->processFIR(seqNum);
            }

            return;
        }

        offset += 8;
    }

    return;
}
void RTPController::onReceivePLI(const sp<ABuffer> buffer)
{
    ALOGI("%s",__FUNCTION__);

    if(buffer->size() != 12) {
        ALOGE("Invalid pli buffer");
        return;
    }

    uint8_t* data = buffer->data();
    uint8_t FMT = data[0] & 0x1F;
    uint8_t pt = data[1];

    if((FMT != 1) || (pt != 206)) {
        ALOGE("%s,not valid PLI(FMT:%d,pt:%d)",__FUNCTION__,FMT,pt);
        return;
    }

    uint32_t ssrc = u32at(&data[8]);

    //if(ssrc == mSSRC) {
        //notify to MA
        sp<AMessage> notify = mNotify->dup();
        notify->setInt32("what",kWhatPLI);
        notify->post();
    //}

    return;
}
void RTPController::onReceiveSLI(const sp<ABuffer> buffer)
{
    ALOGI("%s",__FUNCTION__);

    if(buffer->size() < 16) {
        ALOGE("Invalid sli buffer");
        return;
    }

    uint8_t* data = buffer->data();
    uint8_t FMT = data[0] & 0x1F;
    uint8_t pt = data[1];

    if((FMT != 2) || (pt != 206)) {
        ALOGE("%s,not valid SLI(FMT:%d,pt:%d)",__FUNCTION__,FMT,pt);
        return;
    }

    uint32_t ssrc = u32at(&data[8]);

    if(ssrc == mSSRC) {
        uint32_t sli_length = buffer->size() - 12;
        sp<ABuffer> sli_buffer = new ABuffer(sli_length);
        memcpy(sli_buffer->data(),buffer->data() + 12,sli_length);

        //notify to MA
        sp<AMessage> notify = mNotify->dup();
        notify->setInt32("what",kWhatSLI);
        notify->setBuffer("sli",sli_buffer);
        notify->post();
    }

    return;
}

#if 0
void RTPController::onReceiveSLI(const sp<ABuffer> buffer)
{
    ALOGI("%s",__FUNCTION__);
    uint8_t* data = buffer->data();
    uint8_t FMT = data[0] & 0x1F;
    uint8_t pt = data[1];

    if((FMT != 2) || (pt != 206)) {
        ALOGE("%s,not valid SLI(FMT:%d,pt:%d)",__FUNCTION__,FMT,pt);
        return;
    }

    //there are maybe many SLI request for different sender

    uint32_t offset = 12;//seek to FCI
    uint32_t ssrc = 0;
    uint8_t seqNum = 0;

    Vector<sli_info> sliFciVector;
    uint16_t first = 0;
    uint16_t number = 0;
    uint8_t pictureID = 0;

    while(offset < buffer->size()) {
        uint8_t* fci_data = data + offset;

        sli_info slifci;
        slifci.mFirstMB = (fci_data[0] << 5) | (fci_data[1] >> 3);
        slifci.mNumberMB = ((fci_data[1] & 0x07) << 10) | (fci_data[2] << 2) | (fci_data[3] >> 6);
        slifci.mPictureID = fci_data[3] &0x03F;

        sliFciVector.push_back(slifci);
        offset += 4;
    }

    //notify to MA
    sp<AMessage> notify = mNotify->dup();

    notify->setInt32("what",kWhatSLI);
    notify->setPointer("sli_infos",&sliFciVector);
    notify->post();
    return;
}
#endif

void RTPController::onReceiveGenericNACK(const sp<ABuffer> buffer)
{
    ALOGI("%s",__FUNCTION__);

    if(buffer->size() < 16) {
        ALOGE("Invalid Generic NACK buffer");
        return;
    }

    uint8_t* data = buffer->data();
    uint8_t FMT = data[0] & 0x1F;
    uint8_t pt = data[1];

    if((FMT != 1) || (pt != 205)) {
        ALOGE("%s,not valid Generic NACK(FMT:%d,pt:%d)",__FUNCTION__,FMT,pt);
        return;
    }

    uint32_t ssrc = u32at(&data[8]);

    if(ssrc == mSSRC) {
        sp<ABuffer> nack_fcis = new ABuffer(buffer->size() - 12);
        memcpy(nack_fcis->data(),buffer->data() + 12, buffer->size() - 12);

        if(mVideoRTPSender.get()) {
            mVideoRTPSender->processGenericNACKFB(nack_fcis);
        }
    }

    return;
}
void RTPController::onReceiveTMMBN(const sp<ABuffer> buffer)
{
    ALOGI("%s ++",__FUNCTION__);

    if(!m_isWaitingTMMBN) {
        ALOGW("%s,not waiting TMMBN",__FUNCTION__);
    }

    if(buffer->size() < 20) {
        ALOGE("Invalid TMMBN buffer");
        return;
    }

    uint8_t* data = buffer->data();
    uint8_t FMT = data[0] & 0x1F;
    uint8_t pt = data[1];

    if((FMT != 4) || (pt != 205)) {
        ALOGE("%s,not valid TMMBN(FMT:%d,pt:%d)",__FUNCTION__,FMT,pt);
        return;
    }

    uint32_t ssrc = u32at(&data[12]);
    ALOGD("%s,skip check ssrc=0x%x,mVideoPeerSSRC=0x%x",__FUNCTION__,ssrc,mVideoPeerSSRC);

    //if(mVideoPeerSSRC_set && (ssrc == mVideoPeerSSRC)){
    sp<ABuffer> tmmbn_fci = new ABuffer(4);
    memcpy(tmmbn_fci->data(),buffer->data() + 16, 4);
    (tmmbn_fci->meta())->setInt32("ssrc_media",ssrc);
    //parse bitrate and overhead

    //no need repeat TMMBR
    mNextScheduleRTCPinfo.mFeedBackFlag &= (~kKeyTMMBR);

    if(mRTPReceiver.get()) {    //&& !m_isNWIndication){
        mRTPReceiver->processTMMBN(tmmbn_fci);
    }

    //}
    //ToDo: need check whether need
    /*
        if(m_isWaitingTMMBN && m_isNWIndication){
            //notify need sip update b=AS
            sp<AMessage> notify = mNotify->dup();
            notify->setInt32("what",kWhatUpdateVB);
            notify->setInt32("mbr_dl",mbr_dl);
            notify->post();
        }
        */
    m_isWaitingTMMBN = false;
    ALOGI("%s --,feeback_flag = 0x%x",__FUNCTION__,mNextScheduleRTCPinfo.mFeedBackFlag);

}
void RTPController::onReceiveTMMBR(const sp<ABuffer> buffer)
{
    ALOGI("%s",__FUNCTION__);

    if(buffer->size() < 20) {
        ALOGE("Invalid TMMBR buffer");
        return;
    }

    uint8_t* data = buffer->data();
    uint8_t FMT = data[0] & 0x1F;
    uint8_t pt = data[1];

    if((FMT != 3) || (pt != 205)) {
        ALOGE("%s,not valid TMMBR(FMT:%d,pt:%d)",__FUNCTION__,FMT,pt);
        return;
    }

    uint32_t ssrc = u32at(&data[12]);
    ALOGD("%s,skip check ssrc=0x%x,mSSRC=0x%x",__FUNCTION__,ssrc,mSSRC);
    //if(ssrc == mSSRC){
    sp<ABuffer> tmmbr_fci = new ABuffer(4);
    memcpy(tmmbr_fci->data(),buffer->data() + 16, 4);

    if(mVideoRTPSender.get()) {
        mVideoRTPSender->processTMMBR(tmmbr_fci);
    }

    //}
    return;
}

void RTPController::onSendFIR()
{
    ALOGI("%s",__FUNCTION__);
    //check whether FIR supported
    bool bSupportFIR = false;

    for(uint8_t i = 0; i< mVideoConfigParam.rtcp_fb_param_num; i++) {
        uint16_t fb_id = mVideoConfigParam.rtcp_fb_type[i].rtcp_fb_id;
        uint16_t fb_param = mVideoConfigParam.rtcp_fb_type[i].rtcp_fb_param;

        if((fb_id == IMSMA_CCM) && (fb_param == IMSMA_FIR)) {
            bSupportFIR = true;
            break;
        }
    }

    if(!bSupportFIR) {
        ALOGW("%s,ccm fir not supported",__FUNCTION__);
        //test
        return;
    }

    mFIRLastSeqNum = (mFIRLastSeqNum + 1) % 256;

    checkAndAddFB(kKeyFIR,-1);    //300000);
}
void RTPController::onReceiveCSD(int32_t trackIndex)
{
    ALOGI("%s,stop sending FIR for track(%d)",__FUNCTION__,trackIndex);
    mNextScheduleRTCPinfo.mFeedBackFlag = mNextScheduleRTCPinfo.mFeedBackFlag & (~kKeyFIR);
    return;
}
#if 0
void RTPController::onSendSLI(uint16_t firstMB,uint16_t number,uint8_t pictureID)
{
    ALOGI("%s:first(%d),number(%d),pictureID(%d)",__FUNCTION__,firstMB,number,pictureID);
    //check whether FIR supported
    bool bSupportSLI = false;

    for(uint8_t i = 0; i< mVideoConfigParam.rtcp_fb_param_num; i++) {
        uint16_t fb_id = mVideoConfigParam.rtcp_fb_type[i].rtcp_fb_id;
        uint16_t fb_param = mVideoConfigParam.rtcp_fb_type[i].rtcp_fb_param;

        if((fb_id == IMSMA_NACK) && (fb_param == IMSMA_SLI)) {
            bSupportSLI = true;
            break;
        }
    }

    if(!bSupportSLI) {
        ALOGW("%s,SLI not supported",__FUNCTION__);
        //test
        return;
    }

    mSLIinfo.mFirstMB = firstMB;
    mSLIinfo.mNumberMB = number;
    mSLIinfo.mPictureID = pictureID;

    mFeedBackFlag = mFeedBackFlag | kKeySLI;

    //if last RTCP containing immediately fb, the next RTCP must be regular
    //we should not change the scheduled time of next RTCP
    if(mLastRTCPinfo.mRTCPMode == (IMMEDIATE_MODE|EARLY_MODE)) {
        mLastRTCPinfo.mRTCPMode = REGULAR_MODE;
        return;
    }

    //make the scheduled next RTCP out of date
    mVideoSendRTCPGeneration ++;

    int64_t nowUs = ALooper::GetNowUs();
    int64_t deltaUs = nowUs - mLastRTCPinfo.mRTCPSentTimeUs;
    int64_t delayUs = 500000l; //ToDo: need check the ealry mode delay

    if(deltaUs < 100000l) {
        mLastRTCPinfo.mRTCPMode = EARLY_MODE;
        startSendRTCPPacket(delayUs);
        return;
    }

    mLastRTCPinfo.mRTCPMode = IMMEDIATE_MODE;
    startSendRTCPPacket(0);    //immediate fb

    return;
}
#endif
void RTPController::onSendSLI(sp<ABuffer> buffer)
{
    if(!buffer.get()) {
        ALOGE("%s,sli buffer is NULL",__FUNCTION__);
        return;
    }

    //check whether FIR supported
    bool bSupportSLI = false;

    for(uint8_t i = 0; i< mVideoConfigParam.rtcp_fb_param_num; i++) {
        uint16_t fb_id = mVideoConfigParam.rtcp_fb_type[i].rtcp_fb_id;
        uint16_t fb_param = mVideoConfigParam.rtcp_fb_type[i].rtcp_fb_param;

        if((fb_id == IMSMA_NACK) && (fb_param == IMSMA_SLI)) {
            bSupportSLI = true;
            break;
        }
    }

    if(!bSupportSLI) {
        ALOGW("%s,SLI not supported",__FUNCTION__);
        return;
    }

    mSliBuffer = buffer;

    checkAndAddFB(kKeySLI,100000);

}
void RTPController::checkAndAddFB(uint32_t fb_type,int64_t max_delayUs)
{
    ALOGI("%s",__FUNCTION__);
    //check whether can or need send FBs
    int64_t tc = ALooper::GetNowUs();

    if(mNextScheduleRTCPinfo.mFeedBackFlag != 0) {
        ALOGI("%s,combine this FB(0x%x) with the next RTCP(already contain FB)",__FUNCTION__,fb_type);
        //if next RTCP has FBs
        //rather than produce a new RTCP,we need contain the new FBs to the next RTCP

        //check whether it's still worth to sending this FBs
        //if combine into the next RTCP
        //int64_t tc = ALooper::GetNowUs();
        int64_t delayUs = mNextScheduleRTCPinfo.mRTCPScheduleTimeUs - tc;

        //ToDo: the max delay need re-check with codec owner
        if((max_delayUs > 0) && (delayUs >  max_delayUs)) {
            ALOGW("%s,drop this FB(0x%x),too late for the FB",__FUNCTION__,fb_type);
            return;
        }

        mNextScheduleRTCPinfo.mFeedBackFlag = mNextScheduleRTCPinfo.mFeedBackFlag | fb_type;
        return;
    } else {
        if(m_AllowEarlyRtcp) {
            //ALOGI("%s,allow early rtcp",__FUNCTION__);

            //ToDo: Whether need check tc with the last RTCP sent time
            //if too close to the last rtcp,whether need delay this FB too early_mode
            /*
            int64_t nowUs = ALooper::GetNowUs();
            int64_t deltaUs = nowUs - mLastRTCPinfo.mRTCPSentTimeUs;
            int64_t delayUs = 500000l; //ToDo: need check the ealry mode delay

            if(deltaUs < 100000l){
                mLastRTCPinfo.mRTCPMode = EARLY_MODE;
                startSendRTCPPacket(delayUs);
                return;
            }
            */

            //timeout the original next rtcp
            mVideoSendRTCPGeneration++;

            //keep the last tp equal to the original next rtcp,even though it cancelled
            mRTCP_tp = mNextScheduleRTCPinfo.mRTCPScheduleTimeUs;
            ALOGI("%s,allow early rtcp,keep original mRtcp_tp:%" PRId64 "",\
                  __FUNCTION__,mRTCP_tp);


            //update the next RTCP params
            m_AllowEarlyRtcp = false;
            mNextScheduleRTCPinfo.mFeedBackFlag = mNextScheduleRTCPinfo.mFeedBackFlag | fb_type;
            mNextScheduleRTCPinfo.mRTCPMode = IMMEDIATE_MODE;
            mNextScheduleRTCPinfo.mRTCPScheduleTimeUs = tc;

            startSendRTCPPacket(0,IMSMA_RTP_VIDEO);
            return;
        } else {
            ALOGI("%s,not allow ealry RTCP,combine this FB(0x%x) with next regular RTCP",\
                  __FUNCTION__,fb_type);
            //check whether it's still worth to sending this FBs
            //if combine into the next RTCP
            //int64_t tc = ALooper::GetNowUs();
            int64_t delayUs = mNextScheduleRTCPinfo.mRTCPScheduleTimeUs - tc;

            //ToDo: the max delay need re-check with codec owner
            if((max_delayUs > 0) && (delayUs >  max_delayUs)) {
                ALOGW("%s,drop this FB(0x%x),too late for the FB",__FUNCTION__,fb_type);
                return;
            }

            mNextScheduleRTCPinfo.mFeedBackFlag = mNextScheduleRTCPinfo.mFeedBackFlag | fb_type;
            return;
        }
    }
}
void RTPController::onSendPLI()
{
    ALOGI("%s",__FUNCTION__);
    //check whether pli supported
    bool bSupporPLI = false;

    for(uint8_t i = 0; i< mVideoConfigParam.rtcp_fb_param_num; i++) {
        uint16_t fb_id = mVideoConfigParam.rtcp_fb_type[i].rtcp_fb_id;
        uint16_t fb_param = mVideoConfigParam.rtcp_fb_type[i].rtcp_fb_param;

        if((fb_id == IMSMA_NACK) && (fb_param == IMSMA_PLI)) {
            bSupporPLI = true;
            break;
        }
    }

    if(!bSupporPLI) {
        ALOGW("%s,ccm pli not supported",__FUNCTION__);
        return;
    }

    //usually 1s one I frame
    checkAndAddFB(kKeyPLI,-1);

}
void RTPController::onSendGenericNack(sp<ABuffer> buffer)
{
    if(!buffer.get()) {
        ALOGE("%s,nack fci buffer is NULL",__FUNCTION__);
        return;
    }

    //check whether FIR supported
    bool bSupportGenericNack = false;

    for(uint8_t i = 0; i< mVideoConfigParam.rtcp_fb_param_num; i++) {
        uint16_t fb_id = mVideoConfigParam.rtcp_fb_type[i].rtcp_fb_id;
        uint16_t fb_param = mVideoConfigParam.rtcp_fb_type[i].rtcp_fb_param;

        //ToDo: need check if "a=rtcp-fb:xx nack ",without fb_param set
        // UA will set fb_param= IMSMA_NONE or IMASMA_GENERIC_NACK
        if((fb_id == IMSMA_NACK) && ((fb_param == IMSMA_GENERIC_NACK) || (fb_param == IMSMA_NONE))) {
            bSupportGenericNack = true;
            break;
        }
    }

    if(!bSupportGenericNack) {
        ALOGW("%s,Generic NACK not supported",__FUNCTION__);
        return;
    }

    mGenericNACKBuffer = buffer;

    checkAndAddFB(kKeyGNACK,100000);
}

void RTPController::onSendTMMBR(sp<ABuffer> buffer,bool isReduce)
{
    ALOGI("%s ++: buffer(%p),isReduce=%d",__FUNCTION__,buffer.get(),isReduce);

    if(!buffer.get()) {
        ALOGE("%s,tmmbr fci buffer is NULL",__FUNCTION__);
        return;
    }

    if(mHandoverState > 0) {
        ALOGW("mHandoverState=%d we can't send TMMBR", mHandoverState);
        return;
    }

#if 0
    bool isNetworkIndication = true;
    int32_t mbr_dl;//kbps
    bool isReduce = true;

    sp<AMessage> tmmbr_meta = buffer->meta();

    //ToDo:Whether can get bool address this way
    tmmbr_meta->findInt32("isNW",&isNetworkIndication);
    tmmbr_meta->findInt32("mbr_dl",&mbr_dl);
    tmmbr_meta->findInt32("isReduce",&isReduce);
#endif
    //check whether TMMBR supported
    bool bSupportTMMBR = false;

    for(uint8_t i = 0; i< mVideoConfigParam.rtcp_fb_param_num; i++) {
        uint16_t fb_id = mVideoConfigParam.rtcp_fb_type[i].rtcp_fb_id;
        uint16_t fb_param = mVideoConfigParam.rtcp_fb_type[i].rtcp_fb_param;

        if((fb_id == IMSMA_CCM) && (fb_param == IMSMA_TMMBR)) {
            bSupportTMMBR = true;
            break;
        }
    }

    if(bSupportTMMBR == true) {
        mSupportTMMBR =true;
    }

    if(!mSupportTMMBR) {
        ALOGW("%s,TMMBR/TMMBN not supported",__FUNCTION__);
#if 0

        if(isNetworkIndication) {
            //notify need sip update b=AS
            sp<AMessage> notify = mNotify->dup();
            notify->setInt32("what",kWhatUpdateVB);
            notify->setInt32("mbr_dl",mbr_dl);
            notify->post();
        }

#endif
        return;
    }

    mTMMBRBuffer = buffer;

    //if request increase sending rate from peer
    //schedule TMMBR on regular compound RTCP
    if(!isReduce) {
        mNextScheduleRTCPinfo.mFeedBackFlag |= kKeyTMMBR;
        ALOGI("%s --: feeback_flag = 0x%x",__FUNCTION__,mNextScheduleRTCPinfo.mFeedBackFlag);
        return;
    }

    //if request reduce sending rate from peer
    //can adopt immediate/early mode accoring timing rule

    checkAndAddFB(kKeyTMMBR,-1);
    ALOGI("%s --: feeback_flag = 0x%x",__FUNCTION__,mNextScheduleRTCPinfo.mFeedBackFlag);
}

void RTPController::onSendTMMBN(sp<ABuffer> buffer)
{
    ALOGI("%s ++: ",__FUNCTION__);

    if(!buffer.get()) {
        ALOGE("%s,tmmbr fci buffer is NULL",__FUNCTION__);
        return;
    }

    //check whether TMMBR supported
    bool bSupportTMMBR = false;

    for(uint8_t i = 0; i< mVideoConfigParam.rtcp_fb_param_num; i++) {
        uint16_t fb_id = mVideoConfigParam.rtcp_fb_type[i].rtcp_fb_id;
        uint16_t fb_param = mVideoConfigParam.rtcp_fb_type[i].rtcp_fb_param;

        if((fb_id == IMSMA_CCM) && (fb_param == IMSMA_TMMBR)) {
            bSupportTMMBR = true;
            break;
        }
    }

    if(bSupportTMMBR == true) {
        mSupportTMMBR =true;
    }

    if(!mSupportTMMBR) {
        ALOGW("%s,TMMBR/TMMBN not supported",__FUNCTION__);
        return;
    }

    mTMMBNBuffer = buffer;

    checkAndAddFB(kKeyTMMBN,-1);
    ALOGI("%s --: feeback_flag = 0x%x",__FUNCTION__,mNextScheduleRTCPinfo.mFeedBackFlag);
}

bool RTPController::scanTMMBR(sp<ABuffer> rtcp_packet)
{
    uint32_t offset = 0;

    while(offset < rtcp_packet->size()) {
        uint8_t* data = rtcp_packet->data() + offset;
        uint32_t length = (u16at(&data[2]) + 1) * 4;

        if(rtcp_packet->size()- offset < length) {
            ALOGE("%s,invalid rtcp packet(size:%zu,offset:%u,length:%u)",\
                  __FUNCTION__,rtcp_packet->size(),offset,length);
            return false;
        }

        uint8_t pt = data[1];

        if(pt == 205) {
            uint8_t FMT = data[0] & 0x1F;

            if(FMT == 3) {
                ALOGD("%s,has TMMBR",__FUNCTION__);
                return true;
            }
        }

        offset += length;
    }

    return false;
}


/*
void RTPController::postSendTMMBRMessage(bool isNetworkIndication,int32_t mbr_dl,bool isReduce ){

       sp<AMessage> msg = new AMessage(kWhatSendTMMBR,mReflector);
       msg->setInt32("isNW",isNetworkIndication);
       msg->setInt32("mbr_dl",mbr_dl);
       msg->setInt32("isReduce",isReduce);
       msg->post();

}
*/
//
//}
} //namespace


