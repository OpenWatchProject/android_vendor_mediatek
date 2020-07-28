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
*  TxAdaptationInfo.h
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

#ifndef _IMS_TXADA_H_
#define _IMS_TXADA_H_

#include <sys/time.h>
#include <time.h>
#include <list>

typedef struct{
    bool *needRecord;
    bool *active;
    bool canExpand;
    int32_t ratio;
    uint32_t maxBitrate;
}signalInfo;

/**
  *@ An object of this class facilitates TX Adaptation info calculate
  *@ it contains informations related to RTCP or TMMBR
  */
class TxAdaptationInfo
{

public:
    TxAdaptationInfo();

    void addSenderInfo(uint8_t *data);

    int processReportBlock(uint8_t *data, int32_t contain_TMMBR, signalInfo sigInfo, uint32_t *target, bool *needForceI);

    //for FIR
    bool processFIR(uint8_t seqNum);
    void setFreshTimeUs() {
        mLastRefreshPointSentTimeUs = GetNowUs();
    };
    bool processGenericNACK(uint8_t* data_fci, uint32_t fci_num);
    void resetFirParam();

    //for adaptation
    bool processTMMBR(uint8_t* data, uint32_t *target, uint32_t durationUs, signalInfo sigInfo);
    bool checkFallBack(uint32_t *target, uint32_t durationUs);
    void resetOnStop();
    void updateStatisticInfo(int64_t timeUs, size_t payload_size, size_t size);
    void updateEncBitRate(int64_t timeUs, uint32_t size);

    //scattered info
    void setLastTimeUs(int64_t param) {
        mLastTimeUs = param;
    };
    void setLastRtpTime(uint32_t param) {
        mLastRtpTime = param;
    };
    void setLastRTPSeqNum(uint32_t param) {
        mLastRTPSeqNum = param;
    };
    void setMBRUL(uint32_t param) {
        mMBRUL = param;
    };
    void setRtpBW(uint32_t param) {
        mRtpPacketBandWidth = param;
    };
    void selfIncFrameCount(void) {
        mRTPSentCount++;
    };
    void addPayloadSize(uint64_t param) {
        mLastPayloadSize += param;
    };
    uint32_t getRTPSentCount(void) {
        return mRTPSentCount;
    };
    uint32_t getLastRTPSeqNum(void) {
        return mLastRTPSeqNum;
    };
    uint32_t getRTPQueueNumHighWater(void) {
        return mRTPQueueNumHighWater;
    };

    ~TxAdaptationInfo();

private:

    enum {
        imsma_rtp_ReservedHeaderSize = 0,//16,
        imsma_rtp_MTU_size = 1200,
    };

    int64_t GetNowUs() {
        struct timespec t;
        t.tv_sec = t.tv_nsec = 0;
        clock_gettime(CLOCK_MONOTONIC, &t);
        return ((long long)(t.tv_sec)*1000000000LL + t.tv_nsec) / 1000ll;
    }

    bool updateSendingRate();
    bool checkAllowIncrEncBR(uint32_t durationUs);
    uint32_t tuneNetBitRate(uint32_t uiNetBitRate);
    void notifyAdjustEncBR(uint32_t uiNewBR,bool immediately = false);
    void initAdaptationParams();


    //for FIR
    //if hold on ->resume, need reset these elements
    int64_t mLastRefreshPointSentTimeUs;
    int32_t mLastFIRSeqNum;

    /******for adaptation start**************/
    struct PacketEntry {
        int64_t timeUs;
        int64_t send_timeUs;
        uint32_t iRtpSize;//bytes
        uint32_t iNetSize;//bytes
    };
    std::list<PacketEntry> mPacketList;

    uint32_t mMBRUL;
    uint32_t mRtpPacketBandWidth;

    uint32_t mSendRateSlidWindowUs;
    uint32_t mSendNetBitRate;
    uint32_t mSendRTPBitRate;
    uint32_t mSendPacketRate;
    uint32_t mAvgPayloadSize;


    int64_t mLastTimeUs;
    uint32_t mLastRtpTime;
    uint32_t mRTPSentCount;
    uint64_t mLastPayloadSize;
    uint32_t mLastRTPSeqNum;

    struct FrameEntry {
        int64_t recv_timeUs;
        int64_t frame_timeUs;
        uint32_t iframeSize;//bytes
    };
    std::list<FrameEntry> mFrameList;
    uint32_t mFramesTotalSize;

    uint32_t mEncRateSlidWindowUs;
    uint32_t mLastEstimateEncBR;
    uint32_t mLastEstimateFPS;


    double mDAvgRTPOH;
    uint32_t mUIAvgRTPOH;

    uint32_t mLastTMMBRTotalBitRate;//bps
    uint32_t mLastTMMBRNetBitRate; //bps
    int64_t mLastTMMBRRecvTimeUs;

    //how much bigger the bit rate in TMMBR
    //than the current encoding bitrate, we should trigger increase bit rate
    uint32_t mIncEncBRThreshold; //bps  defalut > 10000bps

    //how much littler the bit rate in TMMBR
    //than the current encoding bitrate, we should trigger reduce bit rate
    uint32_t mReduceEncBRThreshold; //bps default > 5000bps

    //monitor rtp packet queue
    uint32_t mRTPQueueNumHighWater;
    uint32_t mRTPQueueDurHighWaterUs;
    uint32_t mRTPQueueDurLowWaterUs;

    //reception report info
    //int64_t mLastRRTimeUs;

    struct ReceptionReport {
        uint8_t fraction_lost;
        int32_t cumulativeNumPacketLost;
        uint32_t highestSeqNumRecv;
        uint32_t interarrival_jitter;
        int32_t iDiffJitterUs;
        int64_t LSR_timeMs;
        int64_t DLSR_timeMs;
        int32_t iDiffBR;
        int32_t iDiffPR;
        int64_t recv_timeUs;
        uint32_t sentRTPCount;
        uint64_t sentPayloadSize;
    };
    ReceptionReport mLastRCInfo;

    uint32_t mLastExcess_bit;

    //threshold for Reception report to adjust enc bit rate
    uint8_t m_flst_threshold;
    uint32_t m_diffJitterUs_Thrsd;
    int32_t m_diffBR_Thrsd;
    int32_t m_diffPR_Thrsd;

    int32_t m_diffBR_LowWater;//low water for increase encodig bit rate
    int32_t m_diffPR_LowWater;

    uint32_t mLastAdjustEncBitRate;
    int64_t mLastAdjustEncBitRateTimeUs;

    uint32_t mAdjustIntervalUs;
    uint32_t mRRLostForceI;

    struct SenderInfo {
        uint32_t ntptime_first32;
        uint32_t ntptime_last32;
        uint32_t rtpTime;
        uint32_t rtpCount;
        uint32_t payloadOctets;

        int64_t send_timeUs;
    };
    std::list<SenderInfo> mSenderInfoList;

    uint32_t mLast_RTT_ms;

    void riseBitrate(uint32_t *bitRate, signalInfo sigInfo);
    bool mBackUpIncrease;
    uint32_t mRRCount;

    /******for adaptation end**************/

};
#endif // _IMS_TXADA_H_

