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

#ifndef _MTK_CAMERA_STREAMING_FEATURE_PIPE_TPI_ASYNC_NODE_H_
#define _MTK_CAMERA_STREAMING_FEATURE_PIPE_TPI_ASYNC_NODE_H_

#include "StreamingFeatureNode.h"

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

class TPI_AsyncNode : public StreamingFeatureNode
{
private:
    enum class AsyncMode
    {
        NONE,
        FIX_RATE,
        WAITING,
        POLLING,
    };

    enum class WaitState
    {
        IDLE,
        WAITING,
        RUNNING,
    };

public:
    TPI_AsyncNode(const char* name);
    virtual ~TPI_AsyncNode();
    MVOID setTPIMgr(TPIMgr *tpiMgr);
    MBOOL queryFrameEnable();

public:
    virtual MBOOL onData(DataID id, const BasicImgData&);
    virtual IOPolicyType getIOPolicy(StreamType stream, const StreamingReqInfo &reqInfo) const;
    virtual MBOOL getInputBufferPool(const StreamingReqInfo &reqInfo, android::sp<IBufferPool> &pool, NextFullInfo &info);

protected:
    virtual MBOOL onInit();
    virtual MBOOL onUninit();
    virtual MBOOL onThreadStart();
    virtual MBOOL onThreadStop();
    virtual MBOOL onThreadLoop();

private:
    AsyncMode configAsyncMode() const;
    MBOOL initPlugin();
    MBOOL uninitPlugin();
    MBOOL checkNeedRun(const RequestPtr &request, const BasicImgData &data);
    MBOOL process(const RequestPtr &request, const BasicImgData &data);
    MVOID dumpLog(const RequestPtr &request, TPI_Meta meta[], unsigned metaCount, TPI_Image img[], unsigned imgCount);
    MVOID handleResultData(const RequestPtr &request);

    static const char* toStr(AsyncMode);

private:
    TPIUsage mTPIUsage;
    MBOOL mEnable = MFALSE;
    TPIO mTPINodeIO;
    TPIMgr *mTPIMgr = NULL;
    WaitQueue<BasicImgData> mData;
    MUINT32 mNumWorkingBuffer = 3;
    android::sp<ImageBufferPool> mBufferPool;

    AsyncMode mAsyncMode = AsyncMode::NONE;
    MUINT32 mExpectedMS = 0;
    MUINT32 mFixRate = 0;
    MUINT32 mFixRateCounter = 0;
    WaitState mWaitState = WaitState::IDLE;
    TPI_BufferInfo mInputInfo;
};

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

#endif // _MTK_CAMERA_STREAMING_FEATURE_PIPE_TPI_ASYNC_NODE_H_
