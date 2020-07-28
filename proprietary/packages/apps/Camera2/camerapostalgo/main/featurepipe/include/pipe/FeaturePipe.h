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

#include <core/CamPipe.h>
#include <pipe/FeatureNode.h>
#include <pipe/FeatureInference.h>

#include "../../buffer/WorkingBufferPool.h"

#include "../../nodes/RootNode.h"
#include "../../nodes/MultiFrameNode.h"
#include "../../nodes/YUVNode.h"
//#include "nodes/MDPNode.h"

#include <plugin/PipelinePlugin.h>
#include <plugin/PipelinePluginType.h>
#include <sstream>


using namespace android;


namespace com {
namespace mediatek {
namespace campostalgo {
namespace NSFeaturePipe {




class FeaturePipe
    : public virtual CamPipe<FeatureNode>
    , public virtual FeatureNode::Handler_T
    , public virtual IFeaturePipe
{
public:
    FeaturePipe(const UsageHint &usageHint);
    virtual ~FeaturePipe();


public:
    virtual MVOID init();
    virtual MVOID uninit();
    virtual MERROR enque(android::sp<IFeatureRequest> request);
    virtual MVOID abort(android::sp<IFeatureRequest> request);
    virtual MVOID setCallback(android::sp<RequestCallback>);
    virtual sp<RequestCallback> getCallback();
    virtual MBOOL flush();
    virtual std::string getStatus();

    virtual android::sp<IFeatureRequest> acquireRequest();
    virtual MVOID releaseRequest(android::sp<IFeatureRequest> request);

    virtual MVOID dumpPlugin(android::Printer& printer);
    virtual MVOID dumpRequest(android::Printer& printer);

protected:
    typedef CamPipe<FeatureNode> PARENT_PIPE;
    virtual MBOOL onInit();
    virtual MVOID onUninit();
    virtual MBOOL onData(DataID id, const RequestPtr&);

private:
    MERROR prepareNodeSetting();
    MERROR prepareNodeConnection();
    MVOID releaseNodeSetting();
    MUINT32         mLogLevel;

    RootNode*       mpRootNode;
    MultiFrameNode* mpMultiFrameNode;
    YUVNode*        mpYUVNode;
    //MDPNode*        mpMDPNode;

    mutable Mutex                   mRequestLock;
    Vector<RequestPtr>              mRunningRequests;
    android::sp<WorkingBufferPool>  mpBufferPool;
    IFeaturePipe::UsageHint  mUsageHint;
    android::sp<FeatureTaskQueue>   mpTaskQueue;

    typedef std::list<FeatureNode*> NODE_LIST;
    NODE_LIST mpNodes;

    android::sp<NodeSignal> mNodeSignal;

    android::sp<RequestCallback> mpCallback;
    FeatureInference mInference;

    typedef std::list<android::sp<FeatureRequest>> FINISH_CROSS_REQUEST_LIST;
    mutable Mutex                   mFinishRequestLock;
    FINISH_CROSS_REQUEST_LIST       mFinishCrossRequests;
};

class PipeBufferHandle : public BufferHandle
{
public:
    PipeBufferHandle(sp<WorkingBufferPool> pBufferPool, sp<FeatureTaskQueue> pTaskQueue, Format_T format, MSize& size, MSize& align);
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

} // NSFeaturePipe
} // campostalgo
} // mediatek
} // com

#endif // _MTK_CAMERA_CAPTURE_FEATURE_PIPE_CAPTURE_FEATURE_PIPE_H_
