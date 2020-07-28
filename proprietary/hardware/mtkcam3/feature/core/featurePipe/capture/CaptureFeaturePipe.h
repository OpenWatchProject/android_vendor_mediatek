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
 * MediaTek Inc. (C) 2018. All rights reserved.
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

#ifndef _MTK_CAMERA_CAPTURE_FEATURE_PIPE_CAPTURE_FEATURE_PIPE_H_
#define _MTK_CAMERA_CAPTURE_FEATURE_PIPE_CAPTURE_FEATURE_PIPE_H_

#include <list>
#include <utils/Mutex.h>

#include <featurePipe/core/include/CamPipe.h>
#include "CaptureFeatureNode.h"
#include "CaptureFeatureInference.h"

#include "buffer/CaptureBufferPool.h"

#include "nodes/RootNode.h"
#include "nodes/RAWNode.h"
#include "nodes/P2ANode.h"
#include "nodes/FDNode.h"
#include "nodes/FusionNode.h"
#include "nodes/MultiFrameNode.h"
#include "nodes/DepthNode.h"
#include "nodes/BokehNode.h"
#include "nodes/YUVNode.h"
#include "nodes/MDPNode.h"
#include <mtkcam/utils/std/ULog.h>
#include <featurePipe/core/include/CamNodeULog.h>

#include <mtkcam/utils/debug/debug.h>
#include <mtkcam3/3rdparty/plugin/PipelinePlugin.h>
#include <mtkcam3/3rdparty/plugin/PipelinePluginType.h>
#include <sstream>


using namespace android;


namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace NSCapture {

class CaptureFeaturePipe
    : public virtual CamPipe<CaptureFeatureNode>
    , public virtual CaptureFeatureNode::Handler_T
    , public virtual ICaptureFeaturePipe
    , public virtual CamNodeULogHandler
{
public:
    CaptureFeaturePipe(const UsageHint &usageHint, const std::vector<MUINT32> sensorList);
    virtual ~CaptureFeaturePipe();

    struct MyDebuggee : public IDebuggee
    {
        static const std::string			mName;
        std::shared_ptr<IDebuggeeCookie>	mCookie = nullptr;
        android::wp<CaptureFeaturePipe>     mContext = nullptr;

        MyDebuggee(CaptureFeaturePipe* p)
            : mContext(p)
        {}

        virtual ~MyDebuggee() { };
        virtual auto debuggeeName() const -> std::string { return mName; }
        virtual auto debug(android::Printer& printer,
                           const std::vector<std::string>& options) -> void;
   };

public:
    virtual MVOID init();
    virtual MVOID uninit();
    virtual MERROR enque(android::sp<ICaptureFeatureRequest> request);
    virtual MVOID abort(android::sp<ICaptureFeatureRequest> request);
    virtual MVOID setCallback(android::sp<RequestCallback>);
    virtual MBOOL flush();
    virtual std::string getStatus();

    virtual android::sp<ICaptureFeatureRequest> acquireRequest();
    virtual MVOID releaseRequest(android::sp<ICaptureFeatureRequest> request);

    virtual MVOID dumpPlugin(android::Printer& printer);
    virtual MVOID dumpRequest(android::Printer& printer);

protected:
    typedef CamPipe<CaptureFeatureNode> PARENT_PIPE;
    virtual MBOOL onInit();
    virtual MVOID onUninit();
    virtual MBOOL onData(DataID id, const RequestPtr&);

private:
    MERROR prepareNodeSetting();
    MERROR prepareNodeConnection();
    MVOID releaseNodeSetting();

    MUINT32         mLogLevel;
    MBOOL           mIsVSDOF;
    std::vector<MINT32> mSensorList;

    RootNode*       mpRootNode;
    RAWNode*        mpRAWNode;
    P2ANode*        mpP2Node;
    FDNode*         mpFDNode;
    DepthNode*      mpDepthNode;
    BokehNode*      mpBokehNode;
    FusionNode*     mpFusionNode;
    MultiFrameNode* mpMultiFrameNode;
    YUVNode*        mpYUVNode;
    YUVNode*        mpYUV2Node;
    MDPNode*        mpMDPNode;


    mutable Mutex                   mRequestLock;
    Vector<RequestPtr>              mRunningRequests;
    android::sp<CropCalculator>     mpCropCalculator;
    android::sp<CaptureBufferPool>  mpBufferPool;
    ICaptureFeaturePipe::UsageHint  mUsageHint;
    android::sp<CaptureTaskQueue>   mpTaskQueue;

    typedef std::list<CaptureFeatureNode*> NODE_LIST;
    NODE_LIST mpNodes;

    android::sp<RequestCallback> mpCallback;
    CaptureFeatureInference mInference;

    std::shared_ptr<MyDebuggee> mDebuggee = nullptr;

    typedef std::list<android::sp<CaptureFeatureRequest>> FINISH_CROSS_REQUEST_LIST;
    mutable Mutex                   mFinishRequestLock;
    FINISH_CROSS_REQUEST_LIST       mFinishCrossRequests;
};

class PipeBufferHandle : public BufferHandle
{
public:
    PipeBufferHandle(sp<CaptureBufferPool> pBufferPool, sp<CaptureTaskQueue> pTaskQueue, Format_T format, MSize& size, MSize& align);
    virtual ~PipeBufferHandle();

    virtual MERROR acquire(MINT usage = eBUFFER_USAGE_HW_CAMERA_READWRITE |
                                        eBUFFER_USAGE_SW_READ_OFTEN);

    virtual IImageBuffer* native();
    virtual MUINT32 getTransform();
    virtual MVOID release();
    virtual MVOID dump(std::ostream& os) const;

private:
    std::future<void>               mFuture;
    android::sp<IIBuffer>           mpSmartBuffer;
    Format_T                        mFormat;
    MSize                           mSize;
    MSize                           mAlign;
};


} // NSCapture
} // NSFeaturePipe
} // NSCamFeature
} // NSCam

#endif // _MTK_CAMERA_CAPTURE_FEATURE_PIPE_CAPTURE_FEATURE_PIPE_H_
