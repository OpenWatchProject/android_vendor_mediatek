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
* RTPController.h
*
* Project:
* --------
*   Android
*
* Description:
* ------------
*   RTP module controller interface for ViLTE
*
* Author:
* -------
*   Haizhen.Wang(mtk80691)
*
****************************************************************************/
#ifndef _IMS_RTP_CONTROLLER_H_
#define _IMS_RTP_CONTROLLER_H_

#include "RTPBase.h"
#include "IRTPController.h"
#include "RTPSender.h"
#include "RTPReceiver.h"
#include <SocketWrapper.h>
#include <utils/List.h>
#include <utils/Vector.h>

#include <media/stagefright/foundation/AHandler.h>
#include <media/stagefright/foundation/AHandlerReflector.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/ALooper.h>


#include <utils/Errors.h>

using namespace imsma;
using android::status_t;

#define DEBUG_DUMP_RTCP_PACKET
#define UT_PCAP_TEST
#define DEBUG_ENABLE_ADAPTATION

//namespace imsma{
namespace android
{

/**
  *@ An object of this class facilitates receiving of media data on an RTP
  *@ channel.This class will not consider what kind of transport mode used,
  *@ it is only responsible for receiving and unpacking packet,
  *@ and assembling packets to Access Unit which codec can decode
  *@ Now only packetization schemes  of
  *@ "AVC/H.264 encapsulation as specified in RFC 3984 (non-interleaved mode)" supported
  */
class RTPController:public IRTPController
{

public:
    /**************interface of IRTPController*****************************/

    virtual status_t setEventNotify(const sp<AMessage> &eventNotify);
    virtual status_t setConfigParams(rtp_rtcp_config_t* pRTPNegotiatedParams,uint8_t uiMediaType = IMSMA_RTP_VIDEO);
    virtual status_t updateConfigParams(rtp_rtcp_config_t* pRTPNegotiatedParams,uint8_t uiMediaType = IMSMA_RTP_VIDEO);

    virtual status_t setAccuNotify(const sp<AMessage> &accessUnitNotify,uint8_t uiMediaType = IMSMA_RTP_VIDEO);

    virtual status_t addStream(uint8_t rtpPath = IMSMA_RTP_UP_DOWN_LINK, uint8_t trackIndex = IMSMA_RTP_VIDEO);

    virtual status_t removeStream(uint8_t rtpPath = IMSMA_RTP_UP_DOWN_LINK, uint8_t trackIndex = IMSMA_RTP_VIDEO);

    virtual status_t start(uint8_t rtpPath = IMSMA_RTP_UP_DOWN_LINK, uint8_t trackIndex = IMSMA_RTP_VIDEO);

    //virtual status_t pause(uint8_t rtpPath = IMSMA_RTP_UP_DOWN_LINK, uint8_t trackIndex = IMSMA_RTP_VIDEO );

    //virtual status_t resume(uint8_t rtpPath = IMSMA_RTP_UP_DOWN_LINK, uint8_t trackIndex = IMSMA_RTP_VIDEO );

    virtual status_t stop(uint8_t rtpPath = IMSMA_RTP_UP_DOWN_LINK, uint8_t trackIndex = IMSMA_RTP_VIDEO);

    virtual status_t queueAccessUnit(const sp<ABuffer> &buffer,uint8_t trackIndex = IMSMA_RTP_VIDEO);

    virtual status_t peerPausedSendStream(uint8_t trackIndex = IMSMA_RTP_VIDEO);

    virtual status_t peerResumedSendStream(uint8_t trackIndex = IMSMA_RTP_VIDEO);

    virtual status_t setVideoOrientation(uint8_t rotation,uint8_t camera_facing = 0,uint8_t flip = 0);



    /*******************interface related to SocketWrapper*************************/
    //re-architecture
    //two packetcallback respective for RTP and RTCP
    //need cookie to konw RTPController object
    //need msg type: one for buffer, others may be for socket error/info
    //static  void  videoRTPPacketCallBack(void* cookie, int msg,const sp<ABuffer> buffer);

    static  int  videoRTCPPacketCallBack(void* cookie,const sp<ABuffer>& buffer);

    //status_t setSocketFds(Vector<int> fds);


    /************************ AVPF **************************************
    ********************************************************************/
    virtual void feedBackDecoderError(sp<AMessage> errInfo,uint8_t trackIndex = IMSMA_RTP_VIDEO);

    virtual void recvDecoderRefreshPoint(uint8_t trackIndex = IMSMA_RTP_VIDEO);

    virtual status_t setHandoverState(int32_t state);


#if 0
    //for FIR request
    //if decoder need refresh point from peer(sps+pps+IDR for H.264)

    void requestDecoderRefreshPoint(uint8_t trackIndex = IMSMA_RTP_VIDEO);
#endif


    //for SLI requesst
#if 0
    struct sli_info {
        uint16_t mFirstMB;
        uint16_t mNumberMB;
        uint8_t mPictureID;
    };
    sli_info mSLIinfo;

    void sliceLossIndication(uint16_t firstMB,uint16_t number,uint8_t pictureID);

    void sliceLossIndication(const Vector<sli_info>& sli_info);

#endif

#ifdef UT_PCAP_TEST
    virtual sp<SocketWrapper> getVideoRTPSocketWrapper() {
        return mVideoRTPSocketWrapper;
    }
#endif
    RTPController(uint32_t simID,uint32_t operatorID);

protected:
    virtual ~RTPController();

    virtual void onMessageReceived(const sp<AMessage> &msg);
private:
    friend struct AHandlerReflector<RTPController>;
    sp<AHandlerReflector<RTPController> > mReflector;

    rtp_rtcp_config_t mVideoConfigParam;
    rtp_rtcp_config_t mAudioConfigParam;


    sp<AMessage> mVideoAccessUnitNotify;
    sp<AMessage> mAudioAccessUnitNotify;

    //mutable Mutex mLock; //Need Check
    sp<AMessage> mNotify;
    sp<RTPSender> mVideoRTPSender;
    sp<RTPSender> mAudioRTPSender;
    sp<ALooper> mVideoSenderLooper;

    sp<RTPReceiver> mRTPReceiver;
    sp<ALooper> mReceiverLooper;

    sp<ALooper> mLooper;

    uint32_t mSSRC;
    bool mVideoPeerSSRC_set;
    uint32_t mVideoPeerSSRC;
    int32_t mMTUSize;

    bool mRecLatency;
    int32_t mLastEncBitrate;

    /****video related rtcp parameters start******/
    enum rtcp_mode {
        IMMEDIATE_MODE,
        EARLY_MODE,
        REGULAR_MODE,
    };
    //send RTCP related members
    //only video track now supported
    struct rtcp_info {
        int64_t mRTCPScheduleTimeUs;
        uint8_t mRTCPMode;
        uint32_t mFeedBackFlag;
        //uint32_t mRTPSentCount;
        rtcp_info() {
            mRTCPScheduleTimeUs = 0;
            mRTCPMode = REGULAR_MODE;
            mFeedBackFlag = 0;
            //mRTPSentCount = 0;
        }
    };
    //rtcp_info mLastSentRTCPinfo;
    rtcp_info mNextScheduleRTCPinfo;
    uint32_t mLastRTCP_videoRTPSentCount;

    uint64_t m_Trr_interval; //in us, the minimum interval between two regular RTCP
    //equal "trr-int" * 1000 of SDP
    bool m_AllowEarlyRtcp;
    uint64_t mRTCP_tp; //tp; used for calcuate the next RTCP
    uint64_t m_Trr_last; //scheduled time of last regular RTCP (not containing FBs)

    uint32_t m_avg_rtcp_size;


    bool mStartedRTCPSchedule;
    int32_t mVideoSendRTCPGeneration;
    //int64_t mVideoSendRTCPIntervalUs;//in Us

    enum feed_back {
        kKeyFIR = 0x01,
        kKeySLI = 0x02,
        kKeyTMMBR = 0x04,
        kKeyTMMBN = 0x08,
        kKeyPLI = 0x10,
        kKeyGNACK = 0x20,
    };
    //uint32_t mFeedBackFlag;
    uint8_t mFIRLastSeqNum;

    sp<ABuffer> mSliBuffer;
    sp<ABuffer> mGenericNACKBuffer;

    //for adaptation
    //bool m_isNWIndication;
    sp<ABuffer> mTMMBRBuffer;
    bool m_isWaitingTMMBN;
    sp<ABuffer> mTMMBNBuffer;
    bool mSupportTMMBR;

#ifdef DEBUG_ENABLE_ADAPTATION
    bool enable_adaptation;
#endif

    /****video related rtcp parameters end******/

    //audio rtcp maybe need other parameters

#if 0
    uint64_t mUPaccuCount;
    uint64_t mDLpacketCount;
#endif

    //re-architecture
    sp<SocketWrapper> mVideoRTPSocketWrapper;
    sp<SocketWrapper> mVideoRTCPSocketWrapper;

    int32_t mActiveTrack; //IMSMA_RTP_VIDEO,IMSMA_RTP_AUDIO
    int32_t mVideoActivePath; //IMSMA_RTP_UPLINK,IMSMA_RTP_DOWNLINK
    int32_t mAudioActivePath;

    int32_t mAddedTrack;
    int32_t mVideoAddedPath;
    int32_t mAudioAddedPath;
#if 0
    Vector<uint32_t> mSocketfds;
    mutable Mutex mSocketFdsLock; //Need Check
#endif

    AString mCName;

#ifdef DEBUG_DUMP_RTCP_PACKET
    int64_t mDumpRTCPPacket;// ToDo: 1 not work
    int mRecvRTCPFd;
    int mSendRTCPFd;
#endif

    //for RTCP timer
    bool mRTCPTimer;
    int32_t mHandoverState; //0-no handover >0 handover on going
    bool mDowngradeEnable; //we don't trigger downgrade when TMO audio early-media (no date receive)
    bool mReceiveRTCPPacket;
    bool mNoRTPFlag;
    uint32_t mRTCPTimerInterval;
    uint32_t mNoRTCPCount;
    uint32_t mSimID;
    uint32_t mOperatorID;
    void startRtcpTimer();
    void stopRtcpTimer();

    status_t onSetConfigParams(rtp_rtcp_config_t* pRTPNegotiatedParams,uint8_t uiMediaType);
    status_t onUpdateConfigParams(rtp_rtcp_config_t* pRTPNegotiatedParams,uint8_t uiMediaType);

    status_t onAddStream(int32_t rtpPath,int32_t trackIndex);
    status_t onRemoveStream(int32_t rtpPath,int32_t trackIndex);
    status_t onStart(int32_t rtpPath,int32_t trackIndex);
    status_t onStop(int32_t rtpPath,int32_t trackIndex);

    //status_t onPause(uint8_t rtpPath = IMSMA_RTP_UP_DOWN_LINK,uint8_t trackIndex = IMSMA_RTP_VIDEO);
    //status_t onResume(uint8_t rtpPath,uint8_t trackIndex);
    //status_t onHoldOn(int32_t isHoldOnEnable);

    int32_t findSocketFd(uint16_t port);

    status_t queueRTCPPacket(sp<ABuffer> &packet,uint8_t trackIndex = IMSMA_RTP_VIDEO);
    //status_t processReportBlocks(uint8_t RC,uint32_t length,uint8_t trackIndex = IMSMA_RTP_VIDEO);

    status_t startScheduleRTCP(uint8_t trackIndex = IMSMA_RTP_VIDEO);
    status_t stopScheduleRTCP(uint8_t trackIndex = IMSMA_RTP_VIDEO);
    status_t startSendRTCPPacket(int64_t delayUs,uint8_t trackIndex = IMSMA_RTP_VIDEO);
    status_t onSendRTCP(const sp<AMessage> &msg);
    int64_t calculateNextInterval();
    status_t makeAndsendRTCP(uint8_t trackIndex);
    void checkAndAddFB(uint32_t fb_type,int64_t max_delayUs);
    void addFIR(const sp<ABuffer> &buffer);
    void addSDES(const AString& cname, const sp<ABuffer> &buffer);
    void addSLI(const sp<ABuffer> &buffer);
    void addPLI(const sp<ABuffer> &buffer);
    void addNACK(const sp<ABuffer> &buffer);

    //for adaptation
    void addTMMBR(const sp<ABuffer> &buffer);
    void addTMMBN(const sp<ABuffer> &buffer);


    // this function must called in looper message,
    // it's the caller's responsible to lock this operation
    // so no need lock,
    void updateSSRC(uint32_t newSSRC);
    void onReceiveFIR(const sp<ABuffer> buffer);
    void onReceiveSLI(const sp<ABuffer> buffer);
    void onReceivePLI(const sp<ABuffer> buffer);
    void onReceiveGenericNACK(const sp<ABuffer> buffer);

    //for adaptation
    void onReceiveTMMBN(const sp<ABuffer> buffer);
    void onReceiveTMMBR(const sp<ABuffer> buffer);

    void onSendFIR();
    void onReceiveCSD(int32_t trackIndex = IMSMA_RTP_VIDEO);
    void onSendSLI(uint16_t firstMB,uint16_t number,uint8_t pictureID);

    void onSendSLI(sp<ABuffer> buffer);
    void onSendPLI();
    void onSendGenericNack(sp<ABuffer> buffer);

    //for adaptation
    void onSendTMMBR(sp<ABuffer> buffer,bool isReduce);
    void onSendTMMBN(sp<ABuffer> buffer);
    bool scanTMMBR(sp<ABuffer> rtcp_packet);

    enum {
        kWhatSetEventNotify     = 'enof',
        kWhatSetConfigParams    = 'scfp',
        kWhatUpdateConfigParmas = 'ucfp',

        kWhatAddStream          = 'addS',
        kWhatRemoveStream       = 'remS',

        kWhatSetAccuNotify      = 'aunf',
        //need copy the params immediately
        //so should not change to Asyn
        //kWhatSetConfigParams    = 'sCoP',
        //kWhatUpdateConfigParams = 'uCoP',

        kWhatStart              = 'star',
        kWhatStop               = 'stop',

        //kWhatPause                = 'paus',
        //kWhatResume               = 'resm',

        kWhatQueueAccu          = 'accu',

        kWhatHoldOn             = 'hold',

        kWhatPeerPauseStream    = 'pPau',
        kWhatPeerResumeStream   = 'pRes',

        kWhatSetCVOinfo         = 'scvo',

        kWhatSenderNotify       = 'sNtf',
        kWhatReceiverNotify     = 'rNtf',

        kWhatRTPPacket          = 'rtpb',
        kWhatRTCPPacket         = 'rtcp',

        kWhatStartScheduleRTCP  = 'sscd',
        kWhatStopScheduleRTCP   = 'pscd',
        kWhatSendRTCP           = 'sdrc',
        kWhatRTCPTimer          = 'timr',

        kWhatFeedBackDecErr     = 'derr',
#if 0
        kWhatSendFIR            = 'drfp',
        kWhatSendSLI            = 'dsli',
#endif

        kWhatRecvCSD            = 'rCSD',

        kWhatSendTMMBR          = 'tmbr',

        kWhatSetHoState         = 'shos',
    };
};//RTPController
}//namespace imsma
#endif // _IMS_RTP_CONTROLLER_H_

