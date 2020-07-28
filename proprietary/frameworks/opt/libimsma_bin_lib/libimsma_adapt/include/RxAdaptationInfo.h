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
*  RxAdaptationInfo.h
*
* Project:
* --------
*   Android
*
* Description:
* ------------
*   Interface of TxAdaptationInfo
*
* Author:
* -------
*   Zhiming.jiang(mtk11041)
*
****************************************************************************/

#ifndef _IMS_RXADA_H_
#define _IMS_RXADA_H_

#include <sys/time.h>
#include <time.h>
#include <list>

struct RTPDebugNotifyInfo {
    int kWhatInfo;
    int32_t item1;
    int32_t item2;
};

enum {
    kWhatTimeUpdate = 'tmup',
    kWhatSendTMMBR  = 'tmbr',
    kWhatDropCall   = 'drop',
    kWhatUpdateDebugInfo = 'upde',
    kWhatNoRTP      = 'noda',
};


/**
  *@ An object of this class facilitates TX Adaptation info calculate
  *@ it contains informations related to RTCP or TMMBR
  */
class RxAdaptationInfo
{

public:
    RxAdaptationInfo();
    bool updateStatisticInfo(int32_t iRtpOH, int32_t iRtpSize, uint32_t uiNetSize, uint32_t lostcount, int32_t iRtpTime);
    void calculateArrivalJitter(uint32_t uiRtpTimeStamp, int64_t iPacketRecvTimeUs, uint32_t ClockRate);
    int processSenderInfo(uint8_t *data, uint64_t *ntpTime, uint32_t *rtpTime);
    int addReceiverReportBlock(uint32_t mID, uint8_t *data, uint32_t HighestSeqNumber, uint32_t FirstPacketSeqNum, uint32_t ClockRate, bool *isTrigger);
    void processTMMBN(uint8_t* data);
    bool updateInfo(uint32_t rtp_packet_bandwidth, uint32_t MBR_DL, uint32_t sample);
    int fillTMMBRbuffer(uint8_t* data, bool *isReduce);
    bool GetdebugInfo(bool needNotify, int32_t *uiEncBitRate, uint32_t lostcount, uint32_t IDamageCount, RTPDebugNotifyInfo *NotifyInfo, int *count, bool *isTrigger, int Operator);
    void EstimateTSDelay(int64_t mediaTimeUs);
    bool pollingCheckTSDelay();
    void clearTSDelayInfo();

    void peerPausedSendStream();
    void peerResumedSendStream();
    void resetParamOnstart();
    void resetParamOnstop();

    ~RxAdaptationInfo();

    //scattered info
    void setReceivePacketFlag(void) {
        mReceivePacket = true;
    };
    void selfIncFrameCount(void) {
        if(mNumBuffersReceived == 0){
            mFirstPacketRecvTimeUs = GetNowUs();
        }
        mNumBuffersReceived++;
    };
    int32_t getFrameCount(void) {
        return mNumBuffersReceived;
    };

private:

    enum {
        IMSMA_IP_IPSec_UDP_HEADER = 200, //200bytes
    };

    int64_t GetNowUs() {
        struct timespec t;
        t.tv_sec = t.tv_nsec = 0;
        clock_gettime(CLOCK_MONOTONIC, &t);
        return ((long long)(t.tv_sec)*1000000000LL + t.tv_nsec) / 1000ll;
    }
    void initAdaptationParams();
    bool estimateNewBitRate(bool *canDowngrade = NULL);
    uint8_t getFractionLost(uint32_t HighestSeqNumber);
    int32_t getCumulativeLost(uint32_t HighestSeqNumber, uint32_t FirstPacketSeqNum);
    uint32_t tuneNetBitRate(uint32_t uiNetBitRate);
    void CheckAndTriggerTMMBR(bool isReduce,uint32_t uiNewBitRate,uint16_t uiOverHead);
    bool checkAllowIncrEncBR();
    uint32_t GetMaxBR();
    void updateLastSecBitrate();

    int32_t mNumBuffersReceived;
    int64_t mFirstPacketRecvTimeUs;

    bool mNotifyDebugInfoEnable;
    int mRTPMap;

    uint32_t mAS; //kbps
    uint32_t mMBR_DL;//kbps
    uint32_t mSample;

    //for debug
    int32_t mDebugLastLostCount;
    int32_t mDebugLastnumBuffer;
    int32_t mDebugLastIDamageNum;

    //for TS delay
    int64_t mLastMediaTU;
    int64_t mFirstDelta;
    int64_t mAllDelta;
    bool mHaveEarly;
    uint32_t mFrameCount;

    //TMMBR message related parameters
    int64_t mTriggerNewTMMBRDelayUs;
    uint32_t mLastAllowedNetBitRate;// default 0,bps
    uint32_t mLastTMMBRNetBitRate; //bps
    bool mLastTMMBR_isReduce;
    bool mHaveReduce;

    //TMMBN message related parameters
    //ToDo: maybe no use, can be delelted
    uint32_t mLastTMMBNNetBitRate; //bps
    uint32_t mLastTMMBNTotalBitRate;//bps
    int64_t mLastTMMBNRecvTimeUs;
    uint16_t mLastTMMBNOverhead;//bytes


    //make use of SR info to trigger adaptation
    int32_t mLastDiffPR;
    int32_t mLastDiffBR;
    int32_t mDiffPacketRateTd;//10,need can be changed by property
    int32_t mDiffBitRateTd; //5000,5kbps,need can be changed by property
    int32_t m_diffBR_LowWater;//low water for increase encodig bit rate,need can be adjust by property
    int32_t m_diffPR_LowWater;//need can be adjust by property

    uint32_t mNumLastSRPackRecv;
    uint64_t mNetSizeLastSRPackRecv;
    uint32_t mPeerRealBitRate;

    //make use of RRinfo to trigger adaptation
    uint32_t mPacketLostThreshold;//default 2,mean(2/256),need can be adjust by property
    uint32_t m_diffJitterUs_Thrsd; //in Us ,50000 ,50ms,need can be changed by property
    uint32_t mLastRRJitter;//0
    int64_t mLastRRtimeUs;
    uint8_t mLastFractionLost;
    int32_t mLastRRdiffJitterUs;

    //make use playout delay info  to trigger adaptation
    int64_t mAvgPlayOutDelayUs;//0 default
    int64_t mPlayOutDelayLowWater;//-15000,-15ms,low water for increase bit rate
    int64_t mPlayOutDelayHighWater; //50000,50ms high water for reduce bit rate
    uint32_t mPlayOutDelaySlidWindow;//How many frame need to check

    //work for each trigger
    uint32_t mIncEncBRThreshold;//default 10000,10kbps,,need can be changed by property
    uint32_t mReduceEncBRThreshold;//bps default > 5000bps,need can be changed by property

    //record the last trigger info
    bool mLastBWR_isReduce;
    uint32_t mLastBWR_BitRate;
    uint16_t mLastBWR_OverHead;


    //calculate the receiving rate
    struct PacketEntry {
        uint32_t rtp_time;
        int64_t recv_timeUs;
        uint32_t iRtpSize;//bytes
        uint32_t iNetSize;//bytes
    };
    std::list<PacketEntry> mPacketList;

    struct PacketDebugInfo {
        int32_t mDebugPacketLostCount;
        int32_t mDebugPacketnumBuffer;
        int32_t mDebugPacketIDamageNum;
    };
    std::list<PacketDebugInfo> mPacketDebugList;
    uint32_t mRecvRateSlidWindowUs;//2s,need can be changed by property
    uint32_t mRecvPacketRate;
    uint32_t mRecvNetBitRate;
    uint32_t mRecvRTPBitRate;
    uint32_t mMaxRecvNetBR;
    uint32_t mLastLostCount;
    uint32_t mLast3LostCount;
    uint32_t mRecvNetBitRateLastSec;
    uint32_t mDowngradePacketLostTH;
    uint32_t mDowngradeCountTH;
    uint32_t mRTPTimerSec;

    uint64_t mLastCheckIncTimeUs;
    uint64_t m_IncBRUs_Thrsd;
    uint32_t mLowestTMMBRValue;

    double mDAvgRTPOH;
    uint32_t mUIAvgRTPOH;

    uint64_t mLastNetSizeRecv;

    uint32_t mLastPacketRtpTime;
    int64_t mLastPacketRecvTimeUs; //in RTP timestamp units

    uint32_t mUIInterarrivalJitter;
    double mDInterarrivalJitter;

    uint32_t mNumLastRRPackRecv;
    uint32_t mLastRRPackRecvSeqNum;

    //SenderInfo received from peer
    uint64_t mLastSRntp; //
    int64_t mLastSRRecvTimeUs;
    uint32_t mLastSRsenderPacketCount;
    uint32_t mLastSRsenderOctetCount;

    bool mReceivePacket;
    uint32_t mNoPacketCount;
    uint32_t mLowestBitrateCount;   //AT&T need 10s to downgrade
    bool isStopStatus;
    bool isPeerPauseStatus;
    int mDebugLoopCount;

};
#endif // _IMS_TXADA_H_

