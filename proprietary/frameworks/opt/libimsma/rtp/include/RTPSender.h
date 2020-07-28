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
*  RTPSender.h
*
* Project:
* --------
*   Android
*
* Description:
* ------------
*   Interface of RTPSender
*
* Author:
* -------
*   Haizhen.Wang(mtk80691)
*
****************************************************************************/

#ifndef _IMS_RTP_SENDER_H_
#define _IMS_RTP_SENDER_H_

#include "RTPBase.h"
#include "TxAdaptationInfo.h"
#include <SocketWrapper.h>

#include <media/stagefright/foundation/AHandler.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/ALooper.h>


#include <utils/Errors.h>

#define DEBUG_DUMP_PACKET

using namespace android;
using android::status_t;

class TxAdaptationInfo;

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
class RTPSender: public AHandler
{

public:
    enum notify {
        kWhatRefreshPointReq    = 'rfrp',
        kWhatForceIntraPicture  = 'fitp',
        kWhatStartSR            = 'stsr',
        kWhatSendTMMBN          = 'stmn',
        kWhatAdjustEncBitRate   = 'ajbr',
    };

    /**
      *@ Description: RTPSender constructor,
      *@
      *@ Parameters:
      *@        looper: if caller want RTPReceiver run in caller thread, then set this param, otherwise set to NULL
      *@            then RTPReceiver will new a looper
      *@ Return:
      *@    no return
      */
    RTPSender(uint32_t ssrc,sp<SocketWrapper> spRTPSocketWrapper,sp<AMessage> Notify, uint32_t simID,uint32_t operatorID);


    status_t setConfigParams(rtp_rtcp_config_t* pRTPNegotiatedParams);
    status_t updateConfigParams(rtp_rtcp_config_t* pRTPNegotiatedParams);


    /**
      *@ Description: start RTPSender.
      *@       RTPReceiver start to receive and unpack packet, and  assemble packet to access unit
      *@        RTPRecevier will check whether voice only depending on the track info
      *@ Parameters:
      *@        trackIndex: caller can only start one track. If set to NULL,RTPReceiver will start all added stream track
      *@ Return:
      *@    status_t type, OK indicate successful, otherwise error type will return
      */
    status_t start();



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

    //status_t pause();
    /**
    *@ Description: holdBack RTPReceiver.
    *@       RTPReceiver resume to receive and unpack packet, and  assemble packet to access unit
    *@             for voice only session, will disable RTCP related function
    *@ Parameters:
    *@     trackIndex: caller can only resume one track. If set to NULL,RTPReceiver will resume all added stream track
    *@ Return:
    *@     status_t type, OK indicate successful, otherwise error type will return
    */
    //status_t resume();


    /**
      *@ Description: stop  RTPReceiver .
      *@       RTPReceiver stop receiving and unpacking packet. RTPReceiver will stop ALooper thread
      *@ Parameters:
      *@        trackIndex: caller can only stop one track. If set to NULL,RTPReceiver will stop all added stream track
      *@ Return:
      *@    status_t type, OK indicate successful, otherwise error type will return
      */
    status_t stop();

    /**
      *@ Description: QueueBuffer to RTPSender.
      *@
      *@ Parameters:
      *@        buffer: Abuffer sp
      *@ Return:
      *@    status_t type, OK indicate successful, otherwise error type will return
      */
    status_t queueAccessUnit(const sp<ABuffer> &buffer);

    bool isActive();

    //called by RTPSender and RTPReceiver
    //status_t Notify();

    //set CVO(Coordination of Video Orientation)
    //rotation: Clockwise angle
    //default value:
    //camera_facing = 0: Front-facing camera, flip=0: No flip operation
    status_t setVideoOrientation(uint8_t rotation,uint8_t camera_facing = 0,uint8_t flip = 0);

    uint32_t getRTPSentCount();
    void addSenderInfo(const sp<ABuffer> &buffer);

    status_t processReportBlock(const sp<ABuffer> &packet);
    //for FIR
    void processFIR(uint8_t seqNum);
    void processGenericNACKFB(sp<ABuffer> nack_fcis);

    //for adaptation
    void processTMMBR(sp<ABuffer> tmmbr_fci);

    //ToDo: maybe not usable
    //RTPSender should keep align with RTPController's ssrc
    uint32_t getSSRC();
    void updateSSRC(uint32_t ssrc);


protected:
    virtual ~RTPSender();

    virtual void onMessageReceived(const sp<AMessage> &msg);
private:

    status_t onSetConfigParams(rtp_rtcp_config_t* pRTPNegotiatedParams);
    status_t onUpdateConfigParams(rtp_rtcp_config_t* pRTPNegotiatedParams);

    status_t onStart();
    status_t onStop();
    //status_t onPause();
    //status_t onResume();

    status_t onQueueAccessUnit(const sp<ABuffer> &accessUnit);
    status_t queueSingleNALUnitPacket(const sp<ABuffer> accessUnit);
    status_t queueFUorSTAPPacket(const sp<ABuffer> accessUnit);
    status_t queueFUorSTAPPacket_HEVC(const sp<ABuffer> accessUnit);
    status_t queueFragmentNALUnitPacket(
        const sp<ABuffer> accessUnit,size_t remain_size,const uint8_t* nalStart,const size_t nalSize,sp<ABuffer>& out,int32_t* packetCount);
    status_t queueFragmentNALUnitPacket_HEVC(
        const sp<ABuffer> accessUnit,size_t remain_size,const uint8_t* nalStart,const size_t nalSize,sp<ABuffer>& out,int32_t* packetCount);

    void queueRTPPacket(sp<ABuffer> rtpPacket);
    status_t addRTPFixHeader(sp<ABuffer> rtpPacket);
    status_t addRTPExtHeader(sp<ABuffer> rtpPacket);
    void postSendRTPMessage();

    void onAddSenderInfo(const sp<ABuffer> &buffer);
    status_t onProcessReportBlock(const sp<ABuffer> &packet);
    void onProcessFIR(uint8_t seqNum);
    void onProcessGenericNACK(const sp<ABuffer> nack_fcis);

    /*******for adaptation start*****/
    void onProcessTMMBR(const sp<ABuffer> tmmbr_fci);
    void updateStatisticInfo(sp<ABuffer> rtpPacket);
    void updateEncBitRate(const sp<ABuffer> accessUnit);
    bool checkAllowIncrEncBR();
    void notifyAdjustEncBR(uint32_t uiNewBR);
    uint32_t requestRTPQueueDuration();
    /*******for adaptation end *****/

    int32_t mLastReduceSignal;
    void recordSignal();
    bool ifRiseBitrateBaseSignal(int32_t *ratio);
    uint32_t getMaxBitrate();

    int32_t mMTUSize; // Need check whether can get from modem or other network AL
    rtp_rtcp_config_t mConfigParam;

    sp<AMessage> mNotify;
    sp<SocketWrapper> mRTPSocketWrapper;
    mutable Mutex mLock; //Need Check
    bool mSendRTPEventPending;
    bool mStarted;
    List<sp<ABuffer> > mRTPPacketQueue;

    uint32_t mRTPSeqNo;

    //bool m_isFirstRTPPacket;

    int32_t mRTPGeneration;
    uint32_t mSendIntervalMs;//in ms

    //CVO info
    uint32_t m_videoRotation; //in clockwise angle
    uint8_t m_camera_facing; //0: Front-facing camera; 1:Back-facing camera
    uint8_t m_flipped; //0: No flip operation; 1: Horizontal flip operation

    uint8_t m_extmap_CVO_supported;
    uint8_t m_extmap_CVO_id;

    uint8_t m_rtp_fix_header_len;
    uint8_t m_rtp_ext_header_len;


    bool mIsHoldOnEnable;
    uint32_t mAccuReceived;

    uint32_t mSSRC;

    TxAdaptationInfo *mAdaInfo;
    uint32_t mSimID;
    uint32_t mOperatorID;
#ifdef DEBUG_DUMP_PACKET
    int64_t mDumpUpLinkPacket;
    int mRTPFd;
    int mAccuFd;
#endif
    enum {

        kWhatSetConfigParams    = 'sCoP',
        kWhatUpdateConfigParams = 'uCoP',

        kWhatStart              = 'star',
        kWhatStop               = 'stop',

        //kWhatPause                = 'paus',
        //kWhatResume               = 'resm',


        kWhatQueueAccessUnit    = 'accu',

        kWhatIsActive           = 'iAct',

        kWhatSendRTPPacket      = 'sdpk',

        kWhatGetRTPSentCount    = 'RtpC',
        kWhatSetCVOinfo         = 'scvo',

        kWhatAddSenderInfo      = 'asif',
        kWhatProcessRecvReport  = 'prrb',
        kWhatProcessFIR         = 'pfir',
        kWhatProcessNACK        = 'pnak',
        kWhatProcessTMMBR       = 'ptmr',

        kWhatUpdateSSRC         = 'upsr',
        kWhatGetSSRC            = 'gtsr',
    };
};//RTPSender
}//namespace imsma
#endif // _IMS_RTP_SENDER_H_

