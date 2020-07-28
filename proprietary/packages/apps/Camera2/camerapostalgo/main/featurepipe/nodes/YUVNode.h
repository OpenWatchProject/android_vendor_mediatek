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

#ifndef _MTK_CAMERA_CAPTURE_FEATURE_PIPE_YUV_NODE_H_
#define _MTK_CAMERA_CAPTURE_FEATURE_PIPE_YUV_NODE_H_

#include <pipe/FeatureNode.h>

#include <plugin/PipelinePlugin.h>
#include <plugin/PipelinePluginType.h>
#include "../buffer/WorkingBufferPool.h"
#include <future>

namespace com {
namespace mediatek {
namespace campostalgo {
namespace NSFeaturePipe {


class YUVNode : public FeatureNode
{
public:
    typedef NSCam::NSPipelinePlugin::YuvPlugin YuvPlugin;
    typedef YuvPlugin::IProvider::Ptr ProviderPtr;
    typedef YuvPlugin::IInterface::Ptr InterfacePtr;
    typedef YuvPlugin::Selection Selection;
    typedef YuvPlugin::RequestCallback::Ptr RequestCallbackPtr;

public:
    YUVNode(NodeID_T nid, const char *name, MINT32 policy = SCHED_NORMAL, MINT32 priority = DEFAULT_CAMTHREAD_PRIORITY, MBOOL hasTwinNodes = MFALSE);
    virtual ~YUVNode();
    MVOID setBufferPool(const android::sp<WorkingBufferPool> &pool);

public:
    typedef NSCam::NSPipelinePlugin::YuvPlugin::Request::Ptr PluginRequestPtr;

    virtual MBOOL onData(DataID id, const RequestPtr& pRequest);
    virtual MERROR evaluate(NodeID_T nodeId, FeatureInferenceData& rInference);
    virtual RequestPtr findRequest(PluginRequestPtr&);
    virtual MBOOL onRequestRepeat(RequestPtr&);
    virtual MBOOL onRequestProcess(RequestPtr&);
    virtual MBOOL onRequestFinish(RequestPtr&);
    virtual std::string getStatus(std::string& strDispatch);

protected:
    virtual MBOOL onInit();
    virtual MBOOL onUninit();
    virtual MBOOL onThreadStart();
    virtual MBOOL onThreadStop();
    virtual MBOOL onThreadLoop();


private:
    class NegotiatedCacher;

private:
    typedef std::future<void> InitT;
    typedef std::map<FeatureID_T,InitT> InitMapT;
    typedef std::shared_ptr<NegotiatedCacher> NegotiatedCacherPtr;
    struct RequestPair {
        RequestPtr           mPipe;
        PluginRequestPtr     mPlugin;
    };

    struct ProviderPair {
        ProviderPtr          mpProvider;
        FeatureID_T          mFeatureId;
    };

    android::sp<WorkingBufferPool>              mpBufferPool;

    YuvPlugin::Ptr                              mPlugin;
    InterfacePtr                                mpInterface;
    Vector<ProviderPair>                        mProviderPairs;
    RequestCallbackPtr                          mpCallback;
    InitMapT                                    mInitMap;
    android::BitSet64                           mInitFeatures;
    NegotiatedCacherPtr                         mpNegotiatedCacher;

    WaitQueue<RequestPtr>                       mRequests;
    Vector<RequestPair>                         mRequestPairs;

    ProviderPtr                                 mpCurProvider;

    MBOOL                                       mHasTwinNodes;
    mutable Mutex                               mPairLock;
};

} // NSFeaturePipe
} // campostalgo
} // mediatek
} // com

#endif // _MTK_CAMERA_CAPTURE_FEATURE_PIPE_YUV_NODE_H_
