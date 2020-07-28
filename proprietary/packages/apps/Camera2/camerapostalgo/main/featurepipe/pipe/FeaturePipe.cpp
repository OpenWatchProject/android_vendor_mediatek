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

#include <thread>
#include <chrono>
#include <utility>

#include "core/DebugControl.h"
#define PIPE_CLASS_TAG "Pipe"
#define PIPE_TRACE TRACE_CAPTURE_FEATURE_PIPE
#include <core/PipeLog.h>

#define NORMAL_STREAM_NAME "CaptureFeature"
#define THREAD_POSTFIX "@CapPipe"
#define APPEND_POSTFIX(name) (name THREAD_POSTFIX)
//
//#define __DEBUG
//using namespace NSCam::NSIoPipe::NSPostProc;
//using namespace NSCam::NSPipelinePlugin;

#include <pipe/FeaturePipe.h>

namespace com {
namespace mediatek {
namespace campostalgo {
namespace NSFeaturePipe {


namespace {
// begin anonymous namespace

struct FeatureNodeConfig
{
    MINT32      mPolicy;
    MINT32      mPriorit;
};

using FeatureNodeConfigTable = std::map<NodeID_T, const FeatureNodeConfig>;
const FeatureNodeConfigTable gFeatureNodeConfigTable =
{
    {NID_ROOT, {SCHED_NORMAL, DEFAULT_CAMTHREAD_PRIORITY}},
    {NID_MULTIYUV, {SCHED_NORMAL, DEFAULT_CAMTHREAD_PRIORITY}},
    {NID_YUV, {SCHED_NORMAL, DEFAULT_CAMTHREAD_PRIORITY}},
    {NID_MDP, {SCHED_NORMAL, DEFAULT_CAMTHREAD_PRIORITY}},
};

inline FeatureNodeConfig getFeatureNodeConfig(NodeID_T nodeId)
{
    auto foundItem = gFeatureNodeConfigTable.find(nodeId);
    if(foundItem == gFeatureNodeConfigTable.end()) {
        MY_LOGW("failed to get node config and use defaule, nodeId:%d", nodeId);
        return { .mPolicy  = SCHED_NORMAL,
                 .mPriorit = DEFAULT_CAMTHREAD_PRIORITY };
    }
    return foundItem->second;
}

template<typename TFeaturePipeNode, typename... Ts>
inline TFeaturePipeNode* CreateNode(NodeID_T nodeId, const char* name, Ts&&... params)
{
    FeatureNodeConfig config = getFeatureNodeConfig(nodeId);
    return new TFeaturePipeNode(nodeId, name, config.mPolicy, config.mPriorit, std::forward<Ts>(params)...);
}

template<typename TFeaturePipeNode>
inline TFeaturePipeNode* CreateNode(NodeID_T nodeId, const char* name)
{
    FeatureNodeConfig config = getFeatureNodeConfig(nodeId);
    return new TFeaturePipeNode(nodeId, name, config.mPolicy, config.mPriorit);
}

} // end anonymous namespace


FeaturePipe::FeaturePipe(const UsageHint &usageHint)
    : CamPipe<FeatureNode>("FeaturePipe")
{
    TRACE_FUNC_ENTER();

    (void) usageHint;

    mLogLevel           = ::property_get_int32("vendor.debug.postalgo.featurepipe.log", 1);
    mUsageHint          = usageHint;

    mpRootNode          = CreateNode<RootNode>(NID_ROOT, APPEND_POSTFIX("Root"));
    mpMultiFrameNode    = CreateNode<MultiFrameNode>(NID_MULTIYUV, APPEND_POSTFIX("MF"));
    mpYUVNode           = CreateNode<YUVNode>(NID_YUV, APPEND_POSTFIX("YUV"));
//    mpMDPNode           = CreateNode<MDPNode>(NID_MDP, APPEND_POSTFIX("MDP"));

    // Add node
    mpNodes.push_back(mpRootNode);
    mpNodes.push_back(mpMultiFrameNode);
    mpNodes.push_back(mpYUVNode);
//    mpNodes.push_back(mpMDPNode);

    // Notice: the value of NodeID must follows the dependency of pipe flow
    mInference.addNode(NID_MULTIYUV, mpMultiFrameNode);
    mInference.addNode(NID_YUV, mpYUVNode);
//    mInference.addNode(NID_MDP, mpMDPNode);

    mpBufferPool = new WorkingBufferPool("CapturePipe");
    mpTaskQueue = new FeatureTaskQueue(3);

    mNodeSignal = new NodeSignal();
    if (mNodeSignal == NULL) {
        MY_LOGE("cannot create NodeSignal");
    }

    MY_LOGD("construct pipe(%p), uniqueKey:%d",this, mUsageHint.mPluginUniqueKey);

    TRACE_FUNC_EXIT();
}

FeaturePipe::~FeaturePipe()
{
    TRACE_FUNC_ENTER();
#define DELETE_NODE(node)       \
    do {                        \
        if (node == NULL)       \
            break;              \
        delete node;            \
        node = NULL;            \
    } while(0)

    DELETE_NODE(mpRootNode);
    DELETE_NODE(mpMultiFrameNode);
    DELETE_NODE(mpYUVNode);
//    DELETE_NODE(mpMDPNode);
#undef DELETE_NODE

    // must call dispose to free CamGraph
    this->dispose();
    MY_LOGI("destroy pipe(%p), uniqueKey:%d", this, mUsageHint.mPluginUniqueKey);
    TRACE_FUNC_EXIT();
}


MVOID FeaturePipe::init()
{
    TRACE_FUNC_ENTER();
    MBOOL ret;
    ret = PARENT_PIPE::init();
    TRACE_FUNC_EXIT();
}

MVOID FeaturePipe::uninit()
{
    TRACE_FUNC_ENTER();
    ALOGD("FeaturePipe uninit!");
    MBOOL ret;
    ret = PARENT_PIPE::uninit();
    TRACE_FUNC_EXIT();
}

MERROR FeaturePipe::enque(sp<IFeatureRequest> request)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    if (request == NULL)
    {
        MY_LOGE("enque an empty request!");
        TRACE_FUNC_EXIT();
        return BAD_VALUE;
    }
    else
    {
        sp<FeatureRequest> pRequest = static_cast<FeatureRequest*>(request.get());
        // For the event of next capture
        pRequest->mpCallback = mpCallback;

        MY_LOGD("start inference");
        function<void()> fnInference = [=]() {
            if(mInference.evaluate(pRequest)!=OK)
            {
                AEE_ASSERT("R: %d/Timestamp:%d, Inference evaluation failed!",
                        pRequest->getRequestNo(), pRequest->getTimestamp());
            }

            /*
            FeatureRequest& rRequest = *pRequest;
            // Unnecessary to allocate buffers if BSS drop
            if (!rRequest.hasParameter(PID_DROPPED_FRAME)) {
                for (size_t i = 0; i < rRequest.mBufferItems.size(); i++) {
                    auto& rItem = rRequest.mBufferItems.editValueAt(i);
                    auto bufID  = rRequest.mBufferItems.keyAt(i);
                    MY_LOGD_IF(mLogLevel, "Check to create working buffer, bufID=%d, cond: mCreated=%d Size=%d",
                                bufID, !rItem.mCreated, !!rItem.mSize);
                    if (!rItem.mCreated && !!rItem.mSize) {
                        rItem.mpHandle = new PipeBufferHandle(
                                    mpBufferPool,
                                    mpTaskQueue,
                                    rItem.mFormat,
                                    rItem.mSize,
                                    rItem.mAlign);
                        rItem.mCreated = MTRUE;
                    }
                }
            }
            */

            if (mLogLevel > 1)
                pRequest->dump();
            return true;
        };
        fnInference();
        MY_LOGD("end inference");

        mNodeSignal->clearStatus(NodeSignal::STATUS_IN_FLUSH);
        {
            Mutex::Autolock _l(mRequestLock);
            mRunningRequests.push_back(pRequest);
        }
        MY_LOGD("enque CamPipe start");
        ret = CamPipe::enque(PID_ENQUE, pRequest);
        MY_LOGD("enque CamPipe end");
    }
    TRACE_FUNC_EXIT();
    return ret ? OK : UNKNOWN_ERROR ;
}

MVOID FeaturePipe::abort(sp<IFeatureRequest> request)
{
    TRACE_FUNC_ENTER();
    if (request == NULL) {
        MY_LOGE("enque an empty request!");
    } else {
        sp<FeatureRequest> pRequest = static_cast<FeatureRequest*>(request.get());
        pRequest->addParameter(PID_ABORTED, 1);
        // Broadcast
        for (auto& pNode : mpNodes) {
            if (!pNode->onAbort(pRequest)) {
                MY_LOGI("Stop aborting on Node(%d), R: %d/Timestamp:%d",
                        pNode->getNodeID(),
                        pRequest->getRequestNo(),
                        pRequest->getTimestamp());
                break;
            }
        }
    }
    TRACE_FUNC_EXIT();
}

MBOOL FeaturePipe::flush()
{
    TRACE_FUNC_ENTER();
    MY_LOGD("Trigger flush");
    mNodeSignal->setStatus(NodeSignal::STATUS_IN_FLUSH);
    CamPipe::sync();
    mNodeSignal->clearStatus(NodeSignal::STATUS_IN_FLUSH);
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL FeaturePipe::onInit()
{
    TRACE_FUNC_ENTER();
    MBOOL ret;

    mpBufferPool->init(Vector<BufferConfig>());

    ret = this->prepareNodeSetting() &&
          this->prepareNodeConnection();

    TRACE_FUNC_EXIT();
    return ret;
}

MVOID FeaturePipe::onUninit()
{
    TRACE_FUNC_ENTER();
    this->releaseNodeSetting();
    TRACE_FUNC_EXIT();
}

MBOOL FeaturePipe::onData(DataID id, const RequestPtr& pRequest)
{
    (void) id;
    TRACE_FUNC_ENTER();
    mpCallback->onCompleted(pRequest, OK);

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL FeaturePipe::prepareNodeSetting()
{
    TRACE_FUNC_ENTER();
    NODE_LIST::iterator it, end;
    for( it = mpNodes.begin(), end = mpNodes.end(); it != end; ++it )
    {
        (*it)->setNodeSignal(mNodeSignal);
        (*it)->setUsageHint(mUsageHint);
        (*it)->setTaskQueue(mpTaskQueue);
        if (mLogLevel > 0)
            (*it)->setLogLevel(mLogLevel);
    }

    mpYUVNode->setBufferPool(mpBufferPool);

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL FeaturePipe::prepareNodeConnection()
{
    TRACE_FUNC_ENTER();

    // TODO: construct a data table to describe the connecting edges
    this->connectData(PID_ROOT_TO_YUV, *mpRootNode, *mpYUVNode);
    this->connectData(PID_ROOT_TO_MULTIFRAME, *mpRootNode, *mpMultiFrameNode);
    //this->connectData(PID_ROOT_TO_MDP, *mpRootNode, *mpMDPNode);
    this->connectData(PID_MULTIFRAME_TO_YUV, *mpMultiFrameNode, *mpYUVNode);
    //this->connectData(PID_YUV_TO_MDP, *mpYUVNode, *mpMDPNode);
    //this->connectData(PID_MULTIFRAME_TO_MDP, *mpMultiFrameNode, *mpMDPNode);

    NODE_LIST::iterator it, end;
    for(it = mpNodes.begin(), end = mpNodes.end(); it != end; ++it) {
        this->connectData(PID_DEQUE, PID_DEQUE, **it, this);
    }

    this->setRootNode(mpRootNode);
    mpRootNode->registerInputDataID(PID_ENQUE);

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MVOID FeaturePipe::releaseNodeSetting()
{
    TRACE_FUNC_ENTER();

    mpYUVNode->setBufferPool(NULL);
    this->disconnect();
    TRACE_FUNC_EXIT();
}

android::sp<IFeatureRequest>
FeaturePipe::acquireRequest()
{
    sp<FeatureRequest> pRequest = new FeatureRequest();
    return pRequest;
}

MVOID FeaturePipe::releaseRequest(sp<IFeatureRequest>)
{
}

MVOID FeaturePipe::setCallback(sp<RequestCallback> pCallback)
{
    mpCallback = pCallback;
}

sp<RequestCallback> FeaturePipe::getCallback()
{
    return mpCallback;
}

MVOID FeaturePipe::dumpPlugin(android::Printer& printer)
{
#if 0
    #define DUMP_PLUGIN(type)                                                                                               \
    do {                                                                                                                    \
        type##Plugin::Ptr plugin = type##Plugin::getInstance(mUsageHint.mPluginUniqueKey, {mSensorIndex, mSensorIndex2});   \
        std::stringstream stream;                                                                                           \
        stream << "== " << #type << "Plugin Dump ==" << std::endl;                                                          \
        plugin->dump(stream);                                                                                               \
        printer.printLine(stream.str().c_str());                                                                            \
    } while(0)

    DUMP_PLUGIN(MultiFrame);
    DUMP_PLUGIN(Yuv);
#endif
}

MVOID FeaturePipe::dumpRequest(android::Printer& printer)
{
    printer.printLine("== Running Request Dump ==");
    Mutex::Autolock _l(mRequestLock);
    for (const auto& request :mRunningRequests) {
        request->dump(printer);
    }
}

std::string FeaturePipe::getStatus()
{
    std::string strRet;
#if 0
    #define DUMP_PLUGIN_STATUS(type)                                                                                        \
    do {                                                                                                                    \
        type##Plugin::Ptr plugin = type##Plugin::getInstance(mUsageHint.mPluginUniqueKey, {mSensorIndex, mSensorIndex2});   \
        std::stringstream stream;                                                                                           \
        MY_LOGI("== %s Plugin Dump ==", #type);                                                                             \
        plugin->dump(stream);                                                                                               \
        MY_LOGI("%s", stream.str().c_str());                                                                                \
    } while(0)

    DUMP_PLUGIN_STATUS(MultiFrame);
    DUMP_PLUGIN_STATUS(Yuv);

    Mutex::Autolock _l(mRequestLock);
    std::string strDispatch;
    std::string strRet;
    if (mRunningRequests.size() > 0) {
        MY_LOGI("== Not Finish Request Dump ==");
        auto it = mRunningRequests.begin();

        std::string strProvider;
        std::string str;
        if (mpMultiFrameNode) {
            strProvider += mpMultiFrameNode->getStatus(strDispatch);
        }
        if (mpYUVNode) {
            strProvider += mpYUVNode->getStatus(strDispatch);
        }

        if (strProvider.length() > 0) {
            str = "[NOT Finish Provider:" + strProvider + "]";
        }
        strRet = (*it)->getStatus(strDispatch) + str;
    }
#if 1
    MY_LOGI("%s", strRet.c_str());
    if (strDispatch.size() == 0) {
        return "\nCRDISPATCH_KEY:JpegNode Not Finish";
    } else {
        return ("\nCRDISPATCH_KEY:P2C" + strDispatch);
    }
#else
    return (strRet + "\nCRDISPATCH_KEY:P2C" + strDispatch);
#endif
#endif
return strRet;
}

PipeBufferHandle::PipeBufferHandle(
    sp<WorkingBufferPool> pBufferPool, sp<FeatureTaskQueue> pTaskQueue, Format_T format, MSize& size, MSize& align)
    : mpSmartBuffer(NULL)
    , mFormat(format)
    , mSize(size)
    , mAlign(align)
{
    std::function<void()> func = [&,pBufferPool]() {
        CAM_TRACE_FMT_BEGIN("cfp::allocate|%dx%d", mSize.w, mSize.h);

        MY_LOGD("allocate pipe buffer(%dx%d) format(%d) +",
             mSize.w, mSize.h, mFormat);

        mpSmartBuffer = pBufferPool->getImageBuffer(mSize, mFormat, mAlign);

        MY_LOGD("allocate pipe buffer(%dx%d) format(%d) -",
             mSize.w, mSize.h, mFormat);

        CAM_TRACE_FMT_END();
    };

    mFuture = pTaskQueue->addFutureTask(func);
}

PipeBufferHandle::~PipeBufferHandle()
{
    mFuture.wait();
}

MERROR PipeBufferHandle::acquire(MINT usage)
{
    (void) usage;

    mFuture.wait();

    MY_LOGD("acquired %p:  buffer(%dx%d) format(%d) ",
            mpSmartBuffer.get(),
            mSize.w, mSize.h, mFormat);
    return OK;
}

IImageBuffer* PipeBufferHandle::native()
{
    if (mpSmartBuffer == NULL)
        return NULL;

    return mpSmartBuffer->getImageBufferPtr();
}

MVOID PipeBufferHandle::release()
{
    if (mpSmartBuffer == NULL) {
        MY_LOGW("released a not acquired buffer");
        return;
    }

    MY_LOGD("released %p: buffer(%dx%d) format(%d) ",
            mpSmartBuffer.get(),
            mSize.w, mSize.h, mFormat);

    mpSmartBuffer = NULL;
}

MVOID PipeBufferHandle::dump(std::ostream& os) const
{
    (void) os;
}


MUINT32 PipeBufferHandle::getTransform()
{
    return 0;
}


} // NSFeaturePipe
} // campostalgo
} // mediatek
} // com
