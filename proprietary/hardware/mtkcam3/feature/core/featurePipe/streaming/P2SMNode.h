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
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#ifndef _MTK_CAMERA_STREAMING_FEATURE_PIPE_P2SM_NODE_H_
#define _MTK_CAMERA_STREAMING_FEATURE_PIPE_P2SM_NODE_H_

#include "StreamingFeatureNode.h"
#include "DIPStreamBase.h"

#include <mtkcam3/feature/utils/p2/P2Util.h>
#include <mtkcam/drv/iopipe/PostProc/IHalPostProcPipe.h>
#include <mtkcam/aaa/IHalISP.h>
#include <mtkcam3/feature/utils/p2/DIPStream.h>

#define NEED_EXTRA_P2_TUNING 1

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

class P2SMEnqueRunData : public virtual android::RefBase
{
public:
    SmartTuningBuffer mTuningBuffer;
    NSCam::Feature::P2Util::P2Obj mP2Obj;
};

class P2SMEnqueData
{
public:
    RequestPtr            mRequest;
    ImgBuffer             mFullImg;
    std::list<sp<P2SMEnqueRunData>> mRunData;
    MUINT32 mRecordRunCount = 0;
    MUINT32 mDropCount = 0;
};

class P2SMNode : public StreamingFeatureNode, public DIPStreamBase<P2SMEnqueData>
{
private:
    using P2Pack = NSCam::Feature::P2Util::P2Pack;
    using P2IOPack = NSCam::Feature::P2Util::P2IOPack;

    class MainData : public virtual android::RefBase
    {
    public:
        MainData(const P2Pack &pack) : mP2Pack(pack) {}
    public:
        P2Pack mP2Pack;
        SmartTuningBuffer mTuningBuffer;
        TuningParam mTuningParam;
        MUINT32 mReqCount = 1;
        MBOOL mIsRecord = MFALSE;
        MRectF mRecordCrop;
    };

    class SubData
    {
    public:
        SubData() {}
        SubData(const sp<MainData> &main, IImageBuffer *rrzo, IImageBuffer *lcso, IImageBuffer *lcsho)
            : mMainData(main), mRRZO(rrzo), mLCSO(lcso), mLCSHO(lcsho)
        {}
    public:
        sp<MainData> mMainData;
        IImageBuffer *mRRZO = NULL;
        IImageBuffer *mLCSO = NULL;
        IImageBuffer *mLCSHO = NULL;
    };

public:
    P2SMNode(const char *name);
    virtual ~P2SMNode();
    MVOID setNormalStream(Feature::P2Util::DIPStream *stream);
    MVOID flush();

public:
    virtual MBOOL onData(DataID id, const RequestPtr &data);
    virtual IOPolicyType getIOPolicy(StreamType stream, const StreamingReqInfo &reqInfo) const;

protected:
    virtual MBOOL onInit();
    virtual MBOOL onUninit();
    virtual MBOOL onThreadStart();
    virtual MBOOL onThreadStop();
    virtual MBOOL onThreadLoop();

protected:
    virtual MVOID onDIPStreamBaseCB(const Feature::P2Util::DIPParams &params, const P2SMEnqueData &request);

private:
    MVOID handleResultData(const RequestPtr &request, const P2SMEnqueData &data);
    MBOOL initP2();
    MVOID uninitP2();
    MBOOL processP2SM(const RequestPtr &request);
    MBOOL prepareDrop(const RequestPtr &request, P2SMEnqueData &data, const sp<MainData> &mainData);
    MBOOL prepareTuning(const RequestPtr &request, const sp<MainData> &mainData);
    MVOID updateLCSO(const RequestPtr &request, const sp<MainData> &mainData);
    MBOOL prepareIO(Feature::P2Util::DIPParams &params, const RequestPtr &request, P2SMEnqueData &data, const sp<MainData> &mainData);
    MBOOL prepareMainRun(Feature::P2Util::DIPParams &params, const RequestPtr &request, P2SMEnqueData &data, const sp<MainData> &mainData, const std::vector<P2IO> &recordOut, MBOOL optimizeFirstRecord);
    MBOOL prepareRecordRun(Feature::P2Util::DIPParams &params, const RequestPtr &request, P2SMEnqueData &data, const std::vector<P2IO> &recordOut, MBOOL optimizeFirstRecord);
    MVOID setRecordTimestamp(IImageBuffer *buffer, MINT64 timestamp, const sp<MainData> &mainData);
    MUINT32 fillIOPack(P2IOPack &io, const P2IO &output);
    MUINT32 fillIOPack(P2IOPack &io, const std::vector<P2IO> &outputs, MUINT32 index);
    sp<P2SMEnqueRunData> addRunData(const RequestPtr &request, P2SMEnqueData &data, const sp<MainData> &mainData);
    MBOOL addRun(Feature::P2Util::DIPParams &params, const RequestPtr &request, const sp<P2SMEnqueRunData> &runData, const sp<MainData> &mainData, const P2IOPack &io);

    #if NEED_EXTRA_P2_TUNING
    MVOID copyRunTuning(const sp<P2SMEnqueRunData> &runData, const sp<MainData> &mainData, MUINT32 mask);
    #endif // NEED_EXTRA_P2_TUNING

    MVOID enqueFeatureStream(Feature::P2Util::DIPParams &params, P2SMEnqueData &data);
    MVOID tryAllocateTuningBuffer();

private:
    MBOOL initHalISP();
    MVOID uninitHalISP();

private:
    const MUINT32 DEBUG_SINGLE_DISPLAY_RUN  = 1<<0;
    const MUINT32 DEBUG_COPY_DISP_TUNING    = 1<<1;
    const MUINT32 DEBUG_COPY_REC_TUNING     = 1<<2;

private:
    WaitQueue<RequestPtr> mRequests;

    Feature::P2Util::DIPStream *mDIPStream = NULL;
    NS3Av3::IHalISP *mHalISP = NULL;
    android::sp<TuningBufferPool> mTuningPool;
    android::sp<ImageBufferPool> mFullImgPool;

    MUINT32 mTuningCount = 0;

    std::queue<SubData> mSubQueue;
    ENormalStreamTag mSMVRTag = NSIoPipe::NSPostProc::ENormalStreamTag_Normal;
    MUINT32 mSMVRSpeed = 1;
    MUINT32 mExpectMS = 32;
    MUINT32 mBurst = 480;
    MUINT32 mRecover = 30;

    MUINT32 mSimulate = 0;
    MUINT32 mSingle = 0;
    MBOOL mSkip = MFALSE;
    MBOOL mOptimizeFirst = MTRUE;
};

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

#endif // _MTK_CAMERA_STREAMING_FEATURE_PIPE_P2A_NODE_H_
