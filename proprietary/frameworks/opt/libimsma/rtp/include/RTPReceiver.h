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
*  RTPReceiver.h
*
* Project:
* --------
*   Android
*
* Description:
* ------------
*   interface of RTPReceiver
*
* Author:
* -------
*   Haizhen.Wang(mtk80691)
*
****************************************************************************/

#ifndef _IMS_RTP_RECEIVER_H_
#define _IMS_RTP_RECEIVER_H_

#include "RTPBase.h"
#include <SocketWrapper.h>

#include <media/stagefright/foundation/AHandler.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/ALooper.h>

#include <utils/Vector.h>
#include <utils/List.h>

#include <utils/Errors.h>
#include "RTPSource.h"

using namespace android;
using android::status_t;


namespace imsma
{

/**
  *@ An object of this class facilitates receiving of media data on an RTP
  *@ channel.This class will not consider what kind of transport mode used,
  *@ it is only responsible for receiving and unpacking packet,
  *@ and assembling packets to Access Unit which codec can decode
  *@ Now only packetization schemes  of
  *@ "AVC/H.264 encapsulation as specified in RFC 3984 (non-interleaved mode)" supported
  */
class RTPReceiver:public AHandler
{

public:
    enum notify {
        kWhatStartRR            = 'stRR',
        kWhatGenericNACK        = 'nack',
        kWhatSendTMMBR          = 'tmbr',
        kWhatUpdateSSRC         = 'usrc',
        kWhatDropCall           = 'drop',
        kWhatUpdateDebugInfo    = 'upde',
        kWhatAdjustEncBitRate   = 'ajbr',
        kWhatNoRTP              = 'noda',
        kWhatTriggerPli         = 'tpli',
        kWhatTriggerFir         = 'tfir',
    };
    /**
      *@ Description: static function of get RTP module capability, RTP may supporte serveral media type
      *@                    and for each media type may be support serverl packetization mode
      *@                caller need allocate rtp_capability_t struct and set the pointer as the function param
      *@            This function will call static getCapability function of RTCP module to get rtcp capability
      *@ Parameters:
      *@        rtp_profile: a struct containing the capability of RTPSender
      *@ Return:
      *@    status_t type, OK indicate successful, otherwise error type will return
      */
    //static status_t getCapability(rtp_rtcp_capability_t* pRTPCapArray,uint8_t *pNumOfCap,uint8_t uiMediaType = VIDEO);


    /**
      *@ Description: RTPReceiver constructor,
      *@
      *@ Parameters:
      *@        looper: if caller want RTPReceiver run in caller thread, then set this param, otherwise set to NULL
      *@            then RTPReceiver will new a looper
      *@ Return:
      *@    no return
      */
    RTPReceiver(sp<AMessage> Notify, uint32_t simID,uint32_t operatorID);

    status_t addStream(rtp_rtcp_config_t*pRTPNegotiatedParams,sp<SocketWrapper> socketWrapper,sp<AMessage> accuNotify,int32_t trackIndex = IMSMA_RTP_VIDEO);
    status_t updateConfigParams(rtp_rtcp_config_t* pRTPNegotiatedParams,int32_t trackIndex = IMSMA_RTP_VIDEO);

    //status_t setNotify(sp<AMessage> accuNotify,sp<AMessage> rtpPacketNotify,int32_t trackIndex = IMSMA_RTP_VIDEO);


    /**
      *@ Description: start RTPReceiver.
      *@       RTPReceiver start to receive and unpack packet, and  assemble packet to access unit
      *@        RTPRecevier will check whether voice only depending on the track info
      *@ Parameters:
      *@        trackIndex: caller can only start one track. If set to NULL,RTPReceiver will start all added stream track
      *@ Return:
      *@    status_t type, OK indicate successful, otherwise error type will return
      */
    status_t start(int32_t trackIndex = IMSMA_RTP_VIDEO);



    /**
      *@ Description: pause RTPReceiver.
      *@       RTPReceiver pause receiving and unpacking packet,but RTCP channel will not pause
      *@        will not stop looper,just make rtp packet relate message out of date
      *@ Parameters:
      *@        trackIndex: caller can only pause one track. If set to NULL,RTPReceiver will pause all added stream track
      *@        isNeedFlushQueue: if need flush queue during pause, the default is true of hold on case
      *@ Return:
      *@    status_t type, OK indicate successful, otherwise error type will return
      */
    //status_t pause(bool isNeedFlushQueue = TRUE,uint8_t trackIndex = NULL);
    //pause change to holdOn function

    /**
    *@ Description: hold on RTPReceiver.
    *@       RTPReceiver pause receiving and unpacking packet,but RTCP channel will not pause
    *@        will not stop looper,just make rtp packet relate message out of date
    *@          But for voice only session, enable RTCP related function
    *@ Parameters:
    *@     trackIndex: caller can only pause one track. If set to NULL,RTPReceiver will pause all added stream track
    *@     isNeedFlushQueue: if need flush queue during pause, the default is true of hold on case
    *@ Return:
    *@     status_t type, OK indicate successful, otherwise error type will return
    */
    //status_t holdOn(bool isHoldOnEnable);
    //{
    /*
    Receiver know whether is voice only
    receive can control RTP and RTCP function of RTPSource separately
    if(only voice)
        find source
        pauseRTP
        startSendRR// Do we need parse the received RTCP packet during hold on
    else
        find source
        pauseRTP

    */
    //}



    /**
      *@ Description: resume RTPReceiver.
      *@       RTPReceiver resume to receive and unpack packet, and  assemble packet to access unit
      *@
      *@ Parameters:
      *@        trackIndex: caller can only resume one track. If set to NULL,RTPReceiver will resume all added stream track
      *@ Return:
      *@    status_t type, OK indicate successful, otherwise error type will return
      */
    //status_t resume(uint8_t trackIndex = NULL);
    // resume change to holdBack()

    //status_t pause(int32_t trackIndex = IMSMA_RTP_VIDEO);
    /**
    *@ Description: holdBack RTPReceiver.
    *@       RTPReceiver resume to receive and unpack packet, and  assemble packet to access unit
    *@             for voice only session, will disable RTCP related function
    *@ Parameters:
    *@     trackIndex: caller can only resume one track. If set to NULL,RTPReceiver will resume all added stream track
    *@ Return:
    *@     status_t type, OK indicate successful, otherwise error type will return
    */
    //status_t resume(int32_t trackIndex = IMSMA_RTP_VIDEO);


    /**
      *@ Description: stop  RTPReceiver .
      *@       RTPReceiver stop receiving and unpacking packet. RTPReceiver will stop ALooper thread
      *@ Parameters:
      *@        trackIndex: caller can only stop one track. If set to NULL,RTPReceiver will stop all added stream track
      *@ Return:
      *@    status_t type, OK indicate successful, otherwise error type will return
      */
    status_t stop(int32_t trackIndex = IMSMA_RTP_VIDEO);
    /**
      *@ Description: release RTPReceiver.
      *@       RTPReceiver will release the resources of this stream
      *@ Parameters:
      *@        trackIndex: caller can only release one track. If set to NULL,RTPReceiver will release all added stream track
      *@ Return:
      *@    status_t type, OK indicate successful, otherwise error type will return
      */
    //status_t release(uint8_t rtpPath, uint8_t trackIndex = VIDEO );

    /**
      *@ Description: QueuePacket to RTPReceiver.
      *@             caller can call QueuePacket interface to queue packet
      *@ Parameters:
      *@        trackIndex: which track this packet need to queue to
      *@        packet: packet buffer
      *@ Return:
      *@    status_t type, OK indicate successful, otherwise error type will return
      */
    //status_t queueRTCPPacket(sp<ABuffer> &packet,int32_t trackIndex = IMSMA_RTP_VIDEO);


    static  int  videoRTPPacketCallBack(void* cookie,const sp<ABuffer>& buffer);

    bool isActive(uint8_t trackIndex);

    //called by RTPSender and RTPReceiver
    //status_t Notify();

    //set CVO(Coordination of Video Orientation)
    //rotation: Clockwise angle
    //default value:
    //camera_facing = 0: Front-facing camera, flip=0: No flip operation
    //status_t setVideoOrienation(uint8_t rotation,uint8_t camera_facing = 0,uint8_t flip = 0);


    //if MA recevie the message of peer has paused sending Stream
    //trackIndex=video/audio default is video
    //for example: peer camera off, mute event
    status_t peerPausedSendStream(uint8_t trackIndex = IMSMA_RTP_VIDEO);

    //if MA recevie the message of peer has resume sending Stream
    //trackIndex=video/audio default is video
    //for example: peer camera re-on, un-mute event
    status_t peerResumedSendStream(uint8_t trackIndex = IMSMA_RTP_VIDEO);

    status_t removeStream(int32_t trackIndex = IMSMA_RTP_VIDEO);

    status_t processSenderInfo(const sp<ABuffer> &buffer,uint32_t uSSRC,uint8_t trackIndex = IMSMA_RTP_VIDEO);

    uint8_t addReceiveReportBlocks(const sp<ABuffer> &buffer, uint8_t trackIndex = IMSMA_RTP_VIDEO);

    //status_t getSrcSSRC(uint32_t* ssrc,uint8_t trackIndex = IMSMA_RTP_VIDEO);

    Vector<sp<ABuffer> > *video_queue() {
        return &mVideoRTPQueue;
    }

    bool mVideoRTPPending;

    //for adaptation
    const sp<ABuffer> getNewTMMBRInfo();
    status_t processTMMBN(sp<ABuffer> tmmbn_fci);

protected:
    virtual ~RTPReceiver();

    virtual void onMessageReceived(const sp<AMessage> &msg);
private:
    status_t postTimeUpdate(uint32_t rtpTime,uint64_t ntpTime, int32_t trackIndex = IMSMA_RTP_VIDEO);

    status_t onUpdateConfigParams(rtp_rtcp_config_t* pRTPNegotiatedParams,int32_t trackIndex);
    status_t onAddStream(rtp_rtcp_config_t*pRTPNegotiatedParams,sp<SocketWrapper> socketWrapper,sp<AMessage> accuNotify,int32_t trackIndex);

    //ToDo--need change to message
    status_t onStart(int32_t trackIndex);
    status_t onStop(int32_t trackIndex);
    //status_t onPause(int32_t trackIndex);
    //status_t onResume(int32_t trackIndex);
    //status_t onHoldOn(int32_t isHoldOnEnable);

    status_t onRemoveStream(int32_t trackIndex);
    /**
      *@ Description: QueueBuffer to RTPSender.
      *@
      *@ Parameters:
      *@        buffer: Abuffer sp
      *@ Return:
      *@    status_t type, OK indicate successful, otherwise error type will return
      */
    status_t queueRTPPacket(sp<ABuffer> &buffer,int32_t trackIndex = IMSMA_RTP_VIDEO, uint32_t *newSsrc = 0);
    status_t changeSSRC(sp<ABuffer> &packet,int32_t trackIndex, uint32_t newSsrc);

    status_t onProcessSenderInfo(const sp<ABuffer> &buffer,uint32_t uSSRC,uint8_t trackIndex);
    status_t onPeerPausedSendStream(uint8_t trackIndex);
    status_t onPeerResumedSendStream(uint8_t trackIndex);
    //status_t onGetSrcSSRC(uint32_t* ssrc,uint8_t trackIndex);

    uint8_t onAddReceiveReportBlocks(const sp<ABuffer> &buffer, uint8_t trackIndex = IMSMA_RTP_VIDEO);
    status_t copyConfigParams(rtp_rtcp_config_t* srcParams,rtp_rtcp_config_t* dstParams);

    sp<AMessage> mNotify;
    sp<AMessage> mTrackNotify;
    //mutable Mutex mLock; //Need Check
    //List<sp<RTPSource>> mRTPSources;

    struct TrackInfo:public RefBase {
        int32_t mTrackIndex;
        rtp_rtcp_config_t mConfigParam;

        sp<RTPSource> mRTPSource; //ToDo: only one RTPSource for one track? whether need to change to vector
        bool mSSRCset;
        uint32_t mSSRCid;

        sp<AMessage> mAccuNotify;

        //List<sp<ABuffer>> mAccuList;
        bool mTimeMapped;
        int32_t mSRGeneration;

        uint32_t mFirstAccuRtpTime;
        uint32_t mLastAccuRtpTime;
        uint32_t mRtpTimeCycles;
        //int64_t mNtpTimeUs;
        List<uint32_t> mLostPacketSeqNumList;

        bool mIsFirstAccu;
        uint32_t mRtpTimeAnchor;
        int64_t mExtenedRtpTimeAnchor;
        int64_t mNtpTimeAnchor;
        int64_t mLastNTPTimeUpdateUs;
        uint32_t mSRRtpTimeCycles;

        //int64_t mAccuNumReceived;
        //KeyedVector<uint8_t,uint8_t*> extension_header;
        bool mStarted;

        bool mLastDamageFlag;
        int mLastEndSeq;

        sp<SocketWrapper> mSocketWrapper;

        TrackInfo() {
            mTrackIndex = 0;
            memset(&mConfigParam,0,sizeof(mConfigParam));
            mSSRCset = false;
            mSSRCid = 0;

            mTimeMapped = false;
            mSRGeneration = 0;
            mFirstAccuRtpTime = 0;
            mLastAccuRtpTime = 0;
            mRtpTimeCycles  = 0;

            mLostPacketSeqNumList.clear();

            mIsFirstAccu = true;
            mRtpTimeAnchor = 0;
            mExtenedRtpTimeAnchor = -1;
            mNtpTimeAnchor = -1;
            mLastNTPTimeUpdateUs = 0;
            mSRRtpTimeCycles = 0;
            mStarted = false;
            mLastDamageFlag = false;
            mLastEndSeq = -1;
            //mAccuNumReceived = 0;

            mLastDamageFlag = false;
            mLastEndSeq = -1;
        }

    protected:
        ~TrackInfo() {
            //if(extension_header)
            mLostPacketSeqNumList.clear();
        }
    };
    Vector<sp<TrackInfo> > mpTrackInfos;

    sp<AMessage> mVideoAccuNotify;
    sp<AMessage> mVideoRtpPacketNotify;

    sp<ALooper> mLooper;

    sp<SocketWrapper> mSocketWrapper;

    uint8_t mLastCVOinfo;

    mutable Mutex mVideoRTPQueueLock;
    Vector<sp<ABuffer> > mVideoRTPQueue;
    Vector<sp<ABuffer> > mVideoOtherSSRCQueue;
    uint32_t mSsrcInOtherQueue;
    uint32_t mPreviousSsrc;
    uint32_t mSimID;
    uint32_t mOperatorID;

    //for video adaptation
    int64_t mVideoPathDelayUs;

    //for debug
    bool mMsgDebugEnable;
    int32_t mLastSeqN;

    enum {
        kWhatTrackNotify    = 'tknf',
        kWhatNotifyAccu     = 'ntau',
        kWhatSetConfigParams    = 'sCoP',
        kWhatUpdateConfigParams = 'uCoP',

        kWhatTimeUpdate = 'tmup',

        kWhatStart              = 'star',
        kWhatStop               = 'stop',

        kWhatIsActive           = 'iAct',

        //kWhatPause                = 'paus',
        //kWhatResume               = 'resm',

        kWhatAddStream          = 'adds',
        kWhatRemoveStream       = 'revs',
        kWhatRTPPacket          = 'RTPp',

        kWhatProcessSR          = 'pssr',
        kWhatProcessTMMBN       = 'tmbn',

        kWhatHoldOn             = 'hold',

        kWhatPeerPauseStream    = 'pPau',
        kWhatPeerResumeStream   = 'pRes',

        kWhatSenderNotify       = 'sNtf',
        kWhatReceiverNotify     = 'rNtf',

        kWhatSendRTPPacket      = 'sdpk',
        kWhatRTPsent            = 'pkst',

        kWhatSendSR             = 'sdSR',


        kWhatAddReportBlock     = 'arpb',
        //kWhatGetSSRC          = 'ssrc',

        kWhatGetNewTMMBRInfo    = 'gtmb',

        kWhatDebugInfo          = 'deIn',
    };
};//RTPReceiver
}//namespace imsma
#endif // _IMS_RTP_RECEIVER_H_

