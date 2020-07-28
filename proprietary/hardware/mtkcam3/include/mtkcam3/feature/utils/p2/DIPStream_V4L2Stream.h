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
 * MediaTek Inc. (C) 2019. All rights reserved.
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

#ifndef _MTKCAM_FEATURE_UTILS_P2_V4L2_STREAM_H_
#define _MTKCAM_FEATURE_UTILS_P2_V4L2_STREAM_H_

#include "DIPStream.h"

#include <queue>
#include <atomic>
#include <utils/Mutex.h>
#include <utils/Condition.h>
#include <utils/Thread.h>
#include <mtkcam/drv/def/Dip_Notify_datatype.h>
#include <mtkcam/drv/def/IPostProcDef.h>

namespace NSCam {
namespace Feature {
namespace P2Util {
#if MTK_V4L2_READY
typedef img_ipi_frameparam frame_param;
typedef dip_param ext_param;
#endif //MTK_V4L2_READY
class DIPStream_V4L2Stream
    : public DIPStream
{
public:
    DIPStream_V4L2Stream(MUINT32 sensorIndex);
    virtual ~DIPStream_V4L2Stream();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    MBOOL init(char const* szCallerName,
                NSCam::NSIoPipe::EStreamPipeID pipeID=NSCam::NSIoPipe::EStreamPipeID_Normal,
                MUINT32 secTag=0) override;
    MBOOL uninit(char const* szCallerName = "") override;
    MBOOL enque(const DIPParams &dipParams) override;
    static MUINT32 getRegTableSize();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Internal Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
#if MTK_V4L2_READY
    MVOID reserveFrameParams(size_t curBatchNum);
    MVOID toV4_Pack(const DIPParams &dipParams, frame_param_pack &pack);
    MVOID toV4_FrameParam(const DIPFrameParams &dipFrameParam, frame_param &frameParams);
    MVOID toV4_DipParam(const DIPFrameParams &dipFrameParam, ext_param &dipParams);
    MVOID toV4_Input(const NSIoPipe::Input &in, img_input &input);
    MVOID toV4_Output(const NSIoPipe::Output &out, img_output &output);
    MVOID toV4_Transform(const MINT32 transform, img_output &output);
    MVOID toV4_CrpRsInfo(const NSIoPipe::MCrpRsInfo &crpRsInfo, img_output &output);
    MVOID toV4_SrzConfig(const NSIoPipe::ModuleInfo &moduleData, srz_config &srzConfig);
    MVOID toV4_ExtraParam(const NSIoPipe::ExtraParam &srcExtraParam, extra_param &dstExtraParam);
    //static DIPParams convertToDIPParams(const frame_param_pack &param);
    MVOID dumpLog(const frame_param_pack &param);
    MVOID dumpLog(const img_input &input, const uint32_t i);
    MVOID dumpLog(const img_output &output, const uint32_t i);
    MVOID dumpLog(const ext_param &dipParams);
    int open_media_device(const char *match_name);
    int open_device(int media_fd, const char *match_name, int non_block);
    int open_device(const char *dev_name, int non_block);
    void close_device(int fd);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Convert struct
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    class OutPortI
    {
    public:
        OutPortI() {};
        ~OutPortI() {};
    public:
        MBOOL setOutPortI(NSImageio::NSIspio::EPortIndex portID, int i);
        MINT32 getOutPortI(MINT32 groupID);
    private:
        MINT32 mImg2oId = -1;
        MINT32 mWdmaoId = -1;
        MINT32 mWrotoId = -1;
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    int   mMediaFd = -1;
    int   mHubFd = -1;
    int   mVideoFd = -1;
    MBOOL mInitState = MFALSE;
    std::atomic<uint64_t> mSeqNum = {0};
    MBOOL mBypassV4L2 = MFALSE;
    frame_param *mFrameParams = NULL;
    size_t mFrameParamsSize = 0;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  V4L2 deque thread class
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    class DequeThread : public android::Thread
    {
    public:
        DequeThread(int videoFd, MBOOL logEn, MBOOL bypassV4L2);
        virtual ~DequeThread();
        MVOID signalDeque(const DIPParams &dipParams);
        MVOID signalStop();

    public:
        android::status_t readyToRun();
        bool threadLoop();

    private:
        MBOOL waitParam(DIPParams &dipParams);
        MVOID processParam(DIPParams &dipParams);
        MBOOL checkSeqNum(const uint64_t enqueNum, const uint64_t dequeNum);

    private:
        int mVideoFd = -1;
        std::queue<DIPParams> mParamQueue;
        android::Mutex mThreadMutex;
        android::Condition mThreadCondition;
        MBOOL mStop = MFALSE;
        MBOOL mLogEn = MFALSE;
        MBOOL mBypassV4L2 = MFALSE;
    };
    android::sp<DequeThread> mDequeThread;
#endif //MTK_V4L2_READY
};
} // namespace P2Util
} // namespace Feature
} // namespace NSCam

#endif // _MTKCAM_FEATURE_UTILS_P2_V4L2_STREAM_H_
