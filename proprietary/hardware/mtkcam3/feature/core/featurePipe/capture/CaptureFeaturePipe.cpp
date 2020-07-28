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
//
#include <utility>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam3/feature/mfnr/MfllProperty.h>
#include "CaptureFeaturePipe.h"

#define PIPE_CLASS_TAG "Pipe"
#define PIPE_TRACE TRACE_CAPTURE_FEATURE_PIPE
#include <featurePipe/core/include/PipeLog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_CAPTURE);

#define NORMAL_STREAM_NAME "CaptureFeature"
#define THREAD_POSTFIX "@CapPipe"
#define APPEND_POSTFIX(name) (name THREAD_POSTFIX)
//
#define DUALCAM_NODE_THREAD_PRIORITY_OFFSET (-4)
//#define __DEBUG
using namespace NSCam::NSIoPipe::NSPostProc;
using namespace NSCam::NSPipelinePlugin;
using namespace android;
using namespace NSCam::Utils::ULog;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace NSCapture {

#define DEFAULT_CAPTURE_REQUEST_BOOST MTRUE

static MBOOL getIsDefaultBoost()
{
    static MBOOL ret = []()
    {
        const char* propName = "vendor.debug.camera.capture.enable.boost";
        const MBOOL propVal = property_get_int32(propName, DEFAULT_CAPTURE_REQUEST_BOOST);
        const MBOOL isSetProp = (propVal != DEFAULT_CAPTURE_REQUEST_BOOST);
        MY_LOGD("[getIsDefaultBoost] isSetProp:%d, isDefaultBoost:%d,", isSetProp, propVal);
        return propVal;
    }();
    return ret;
}

namespace { // begin anonymous namespace

struct CaptureFeatureNodeConfig
{
    MINT32      mPolicy;
    MINT32      mPriorit;
};

using CaptureFeatureNodeConfigTable = std::map<NodeID_T, const CaptureFeatureNodeConfig>;
const CaptureFeatureNodeConfigTable gCaptureFeatureNodeConfigTable =
{
    {NID_ROOT, {SCHED_NORMAL, DEFAULT_CAMTHREAD_PRIORITY}},
    {NID_RAW, {SCHED_NORMAL, DEFAULT_CAMTHREAD_PRIORITY}},
    {NID_P2A, {SCHED_NORMAL, DEFAULT_CAMTHREAD_PRIORITY}},
    {NID_FD, {SCHED_NORMAL, DEFAULT_CAMTHREAD_PRIORITY}},
    {NID_MULTIYUV, {SCHED_NORMAL, DEFAULT_CAMTHREAD_PRIORITY}},
    {NID_YUV, {SCHED_NORMAL, DEFAULT_CAMTHREAD_PRIORITY}},
    {NID_MDP, {SCHED_NORMAL, DEFAULT_CAMTHREAD_PRIORITY}},
    {NID_YUV2, {SCHED_NORMAL, DEFAULT_CAMTHREAD_PRIORITY}},
    {NID_DEPTH, {SCHED_NORMAL, DEFAULT_CAMTHREAD_PRIORITY + DUALCAM_NODE_THREAD_PRIORITY_OFFSET}},
    {NID_BOKEH, {SCHED_NORMAL, DEFAULT_CAMTHREAD_PRIORITY + DUALCAM_NODE_THREAD_PRIORITY_OFFSET}},
    {NID_FUSION, {SCHED_NORMAL, DEFAULT_CAMTHREAD_PRIORITY + DUALCAM_NODE_THREAD_PRIORITY_OFFSET}}
};

inline CaptureFeatureNodeConfig getCaptureFeatureNodeConfig(NodeID_T nodeId)
{
    auto foundItem = gCaptureFeatureNodeConfigTable.find(nodeId);
    if(foundItem == gCaptureFeatureNodeConfigTable.end()) {
        MY_LOGW("failed to get node config and use defaule, nodeId:%d", nodeId);
        return { .mPolicy  = SCHED_NORMAL,
                 .mPriorit = DEFAULT_CAMTHREAD_PRIORITY };
    }
    return foundItem->second;
}

template<typename TCaptureNode, typename... Ts>
inline TCaptureNode* CreateNode(NodeID_T nodeId, const char* name, Ts&&... params)
{
    CaptureFeatureNodeConfig config = getCaptureFeatureNodeConfig(nodeId);
    return new TCaptureNode(nodeId, name, config.mPolicy, config.mPriorit, std::forward<Ts>(params)...);
}

template<typename TCaptureNode>
inline TCaptureNode* CreateNode(NodeID_T nodeId, const char* name)
{
    CaptureFeatureNodeConfig config = getCaptureFeatureNodeConfig(nodeId);
    return new TCaptureNode(nodeId, name, config.mPolicy, config.mPriorit);
}

} // end anonymous namespace


CaptureFeaturePipe::CaptureFeaturePipe(const UsageHint &usageHint, const std::vector<MUINT32> sensorList)
    : CamPipe<CaptureFeatureNode>("CaptureFeaturePipe")
    , CamNodeULogHandler(Utils::ULog::MOD_FPIPE_CAPTURE)
{
    TRACE_FUNC_ENTER();

    (void) usageHint;
    mSensorList.assign(sensorList.begin(), sensorList.end());

    mLogLevel           = ::property_get_int32("vendor.debug.camera.capture.log", 0);
    mIsVSDOF            = ((sensorList.size() > 1) && (usageHint.mDualMode & v1::Stereo::E_STEREO_FEATURE_CAPTURE));
    mUsageHint          = usageHint;

    mpRootNode          = CreateNode<RootNode>(NID_ROOT, APPEND_POSTFIX("Root"));
    mpRAWNode           = CreateNode<RAWNode>(NID_RAW, APPEND_POSTFIX("Raw"));
    mpP2Node            = CreateNode<P2ANode>(NID_P2A, APPEND_POSTFIX("P2A"));
    mpFDNode            = CreateNode<FDNode>(NID_FD, APPEND_POSTFIX("FD"));
    mpMultiFrameNode    = CreateNode<MultiFrameNode>(NID_MULTIYUV, APPEND_POSTFIX("MF"));
    mpYUVNode           = CreateNode<YUVNode>(NID_YUV, APPEND_POSTFIX("YUV"), mIsVSDOF);
    mpMDPNode           = CreateNode<MDPNode>(NID_MDP, APPEND_POSTFIX("MDP"));

    // Dual
    if (mIsVSDOF) {
        mpYUV2Node      = CreateNode<YUVNode>(NID_YUV2, APPEND_POSTFIX("YUV2"), mIsVSDOF);
        mpDepthNode     = CreateNode<DepthNode>(NID_DEPTH, APPEND_POSTFIX("Depth"));
        mpBokehNode     = CreateNode<BokehNode>(NID_BOKEH, APPEND_POSTFIX("Bokeh"));
        mpFusionNode    = CreateNode<FusionNode>(NID_FUSION, APPEND_POSTFIX("Fusion"));
    } else {
        mpYUV2Node      = NULL;
        mpDepthNode     = NULL;
        mpBokehNode     = NULL;
        mpFusionNode    = NULL;
    }

    // Add node
    mpNodes.push_back(mpRootNode);
    mpNodes.push_back(mpRAWNode);
    mpNodes.push_back(mpP2Node);
    mpNodes.push_back(mpFDNode);
    mpNodes.push_back(mpMultiFrameNode);
    mpNodes.push_back(mpYUVNode);
    mpNodes.push_back(mpMDPNode);

    if (mIsVSDOF) {
        mpNodes.push_back(mpYUV2Node);
        mpNodes.push_back(mpFusionNode);
        mpNodes.push_back(mpDepthNode);
        mpNodes.push_back(mpBokehNode);
    }

    // Notice: the value of NodeID must follows the dependency of pipe flow
    mInference.addNode(NID_P2B, mpP2Node);
    mInference.addNode(NID_RAW, mpRAWNode);
    mInference.addNode(NID_MULTIRAW, mpMultiFrameNode);
    mInference.addNode(NID_P2A, mpP2Node);
    mInference.addNode(NID_FD, mpFDNode);
    mInference.addNode(NID_MULTIYUV, mpMultiFrameNode);
    mInference.addNode(NID_YUV, mpYUVNode);
    mInference.addNode(NID_MDP, mpMDPNode);
    if (mIsVSDOF) {
        mInference.addNode(NID_YUV2, mpYUV2Node);
        mInference.addNode(NID_FUSION, mpFusionNode);
        mInference.addNode(NID_DEPTH, mpDepthNode);
        mInference.addNode(NID_BOKEH, mpBokehNode);
        mInference.addNode(NID_MDP_B, mpMDPNode);
        mInference.addNode(NID_MDP_C, mpMDPNode);
    }

    mpCropCalculator = new CropCalculator(mSensorList, mLogLevel, usageHint.mDualMode);
    mpBufferPool = new CaptureBufferPool("CapturePipe", mLogLevel);
    mpTaskQueue = new CaptureTaskQueue(3);

    mDebuggee = std::make_shared<MyDebuggee>(this);
    if ( auto pDbgMgr = IDebuggeeManager::get() ) {
        mDebuggee->mCookie = pDbgMgr->attach(mDebuggee, 1);
    }
    MY_LOGD("this:%p UsageHint mDualMode=%d mIsHidlIsp=%d", this, usageHint.mDualMode, usageHint.mIsHidlIsp);
    TRACE_FUNC_EXIT();
}

CaptureFeaturePipe::~CaptureFeaturePipe()
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
    DELETE_NODE(mpRAWNode);
    DELETE_NODE(mpP2Node);
    DELETE_NODE(mpFDNode);
    DELETE_NODE(mpMultiFrameNode);
    DELETE_NODE(mpYUVNode);
    DELETE_NODE(mpMDPNode);
    if (mIsVSDOF) {
        DELETE_NODE(mpYUV2Node);
        DELETE_NODE(mpDepthNode);
        DELETE_NODE(mpBokehNode);
        DELETE_NODE(mpFusionNode);
    }
#undef DELETE_NODE

    if ( mDebuggee != nullptr ) {
        if ( auto pDbgMgr = IDebuggeeManager::get() ) {
            pDbgMgr->detach(mDebuggee->mCookie);
        }
        mDebuggee = nullptr;
    }

    // must call dispose to free CamGraph
    this->dispose();
    MY_LOGD("destroy pipe(%p)", this);
    TRACE_FUNC_EXIT();
}

MVOID CaptureFeaturePipe::init()
{
    TRACE_FUNC_ENTER();
    MBOOL ret;
    ret = PARENT_PIPE::init();
    TRACE_FUNC_EXIT();
}

MVOID CaptureFeaturePipe::uninit()
{
    TRACE_FUNC_ENTER();
    MBOOL ret;
    ret = PARENT_PIPE::uninit();
    TRACE_FUNC_EXIT();
}

MERROR CaptureFeaturePipe::enque(sp<ICaptureFeatureRequest> request)
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
        sp<CaptureFeatureRequest> pRequest = static_cast<CaptureFeatureRequest*>(request.get());
        // For the event of next capture
        pRequest->mpCallback = mpCallback;

        // boost
        if ( getIsDefaultBoost() ) {
            std::stringstream ss;
            ss << "req#:" << pRequest->getRequestNo();
            pRequest->setBooster(IBooster::createInstance(ss.str()));
            pRequest->enableBoost();
        }

        function<void()> fnInference = [=]() {
            if(mInference.evaluate(pRequest)!=OK)
            {
                AEE_ASSERT("R/F Num: %d/%d, Inference evaluation failed!\nCRDISPATCH_KEY:CaptureFeaturePipe",
                        pRequest->getRequestNo(), pRequest->getFrameNo());
            }
            CaptureFeatureRequest& rRequest = *pRequest;

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

            if (mLogLevel > 1)
                pRequest->dump();
            return true;
        };
#ifdef SUPPORT_MFNR
        auto isApplyBss = [this](const RequestPtr& pRequest) -> bool {
            if (pRequest->getPipelineFrameCount() <= 1)
                return false;

            auto pInMetaHal = pRequest->getMetadata(MID_MAN_IN_HAL);
            IMetadata* pHalMeta = pInMetaHal->native();
            MINT32 selectedBssCount = 0;
            if (!IMetadata::getEntry<MINT32>(pHalMeta, MTK_FEATURE_BSS_SELECTED_FRAME_COUNT, selectedBssCount)) {
                MY_LOGW("get MTK_FEATURE_BSS_SELECTED_FRAME_COUNT failed, set to 0");
            }

            if (selectedBssCount < 1)
                return false;

            if (CC_UNLIKELY(mfll::MfllProperty::getBss() <= 0)) {
                MY_LOGD("%s: Bypass bss due to force disable by property", __FUNCTION__);
                return false;
            }
            return true;
        };
        MY_LOGD("(%p)R/F:%d/%d  isApplyBss=%d", this, pRequest->getRequestNo(), pRequest->getFrameNo(), isApplyBss(pRequest));
        // if need BSS, need delay inference
        if (isApplyBss(pRequest)) {
            pRequest->setDelayInference(fnInference);
        } else
#endif
        {
            fnInference();
        }

        {
            Mutex::Autolock _l(mRequestLock);
            mRunningRequests.push_back(pRequest);
        }
        // Append Hal metadata
        auto inHalMetaHandle = pRequest->getMetadata(MID_MAN_IN_HAL);
        auto outHalMetaHandle = pRequest->getMetadata(MID_MAN_OUT_HAL);
        if(inHalMetaHandle != NULL && outHalMetaHandle != NULL)
        {
            IMetadata* pOutHalMeta = outHalMetaHandle->native();
            IMetadata* pInHalMeta = inHalMetaHandle->native();
            (*pOutHalMeta) = (*pInHalMeta) + (*pOutHalMeta);
            MY_LOGD("R/F:%d/%d, append Hal metadata.", pRequest->getRequestNo(), pRequest->getFrameNo());
        }
        else
        {
            MY_LOGD("R/F:%d/%d Missing meta handle, inHal=%p outHal=%p", pRequest->getRequestNo(), pRequest->getFrameNo(), inHalMetaHandle.get(), outHalMetaHandle.get());
        }
        ret = CamPipe::enque(PID_ENQUE, pRequest);
    }
    TRACE_FUNC_EXIT();
    return ret ? OK : UNKNOWN_ERROR ;
}

MVOID CaptureFeaturePipe::abort(sp<ICaptureFeatureRequest> request)
{
    TRACE_FUNC_ENTER();
    if (request == NULL) {
        MY_LOGE("enque an empty request!");
    } else {
        sp<CaptureFeatureRequest> pRequest = static_cast<CaptureFeatureRequest*>(request.get());
        pRequest->addParameter(PID_ABORTED, 1);
        // Broadcast
        for (auto& pNode : mpNodes) {
            if (!pNode->onAbort(pRequest)) {
                MY_LOGI("Stop aborting on Node(%d), R/F Num: %d/%d",
                        pNode->getNodeID(),
                        pRequest->getRequestNo(),
                        pRequest->getFrameNo());
                break;
            }
        }
    }
    TRACE_FUNC_EXIT();
}

MBOOL CaptureFeaturePipe::flush()
{
    TRACE_FUNC_ENTER();
    MY_LOGD("Trigger flush");
    CamPipe::sync();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL CaptureFeaturePipe::onInit()
{
    TRACE_FUNC_ENTER();
    MBOOL ret;

    mpBufferPool->init(Vector<BufferConfig>());

    ret = this->prepareNodeSetting() &&
          this->prepareNodeConnection();

    TRACE_FUNC_EXIT();
    return ret;
}

MVOID CaptureFeaturePipe::onUninit()
{
    TRACE_FUNC_ENTER();
    this->releaseNodeSetting();
    TRACE_FUNC_EXIT();
}

MBOOL CaptureFeaturePipe::onData(DataID id, const RequestPtr& pRequest)
{
    (void) id;
    TRACE_FUNC_ENTER();
    // NOTE: if this function caller is P2ANode, that will hang the P2 callback thread
    const MINT64 sleepTime = ::property_get_int32("vendor.debug.camera.p2capture.sleeptime", 0);
    if(sleepTime > 0)
    {
        auto tmp = std::chrono::milliseconds(sleepTime);
        MY_LOGD("sleep %lld ms +, R/F Num: %d/%d", tmp.count(), pRequest->getRequestNo(), pRequest->getFrameNo());
        std::this_thread::sleep_for(tmp);
        MY_LOGD("sleep %lld ms -, R/F Num: %d/%d", tmp.count(), pRequest->getRequestNo(), pRequest->getFrameNo());
    }

    //
    MY_LOGI("R/F Num: %d/%d - dataid=%s, Finished Abort(%d)/Fail(%d)",
            pRequest->getRequestNo(),
            pRequest->getFrameNo(),
            PathID2Name(id),
            pRequest->hasParameter(PID_ABORTED),
            pRequest->hasParameter(PID_FAILURE));
    {
        Mutex::Autolock _l(mRequestLock);

        auto it = mRunningRequests.begin();
        for (; it != mRunningRequests.end(); it++) {
            if (*it == pRequest) {
                mRunningRequests.erase(it);
                break;
            }
        }
    }
    pRequest->disableBoost();
    CAM_ULOG_EXIT(MOD_FPIPE_CAPTURE, REQ_CAP_FPIPE_REQUEST, pRequest->getFrameNo());

    if (mLogLevel > 2)
        pRequest->dump();

    {
        #define REQUEST_CALLBACK(pRequest, pCallback) \
        pRequest->detachResourceItems();\
        if (pRequest->hasParameter(PID_ABORTED) && !pRequest->hasParameter(PID_RESTORED)) \
            pCallback->onAborted(pRequest); \
        else if (pRequest->hasParameter(PID_FAILURE)) \
            pCallback->onCompleted(pRequest, UNKNOWN_ERROR); \
        else \
            pCallback->onCompleted(pRequest, OK);

        // check cross request
        sp<CaptureFeatureRequest> pCrossRequest = pRequest->getCrossRequest();
        if (pCrossRequest) {
            Mutex::Autolock _l(mFinishRequestLock);
            MBOOL bMainCrossRequest = MFALSE;
            if (!mFinishCrossRequests.empty()) {
                FINISH_CROSS_REQUEST_LIST::iterator iter = mFinishCrossRequests.begin();
                for(;iter != mFinishCrossRequests.end();) {
                    if (*iter == pCrossRequest) {
                        iter   = mFinishCrossRequests.erase(iter);
                        bMainCrossRequest = MTRUE;
                        break;
                    } else {
                        ++iter;
                    }
                }
            }

            if (bMainCrossRequest) {  // callback cross requests at the same time
                MY_LOGD("Callback cross request: R/F Num: %d/%d - %d/%d",
                pCrossRequest->getRequestNo(),
                pCrossRequest->getFrameNo(),
                pRequest->getRequestNo(),
                pRequest->getFrameNo());
                REQUEST_CALLBACK(pCrossRequest, mpCallback);
                REQUEST_CALLBACK(pRequest, mpCallback);
            } else {  // keep first cross request
                mFinishCrossRequests.push_front(pRequest);
                MY_LOGD("Keep cross request: R/F Num: %d/%d, count(%zu)",
                pRequest->getRequestNo(),
                pRequest->getFrameNo(),
                mFinishCrossRequests.size());
            }
        } else {
            REQUEST_CALLBACK(pRequest, mpCallback);
        }
        #undef REQUEST_CALLBACK
    }

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL CaptureFeaturePipe::prepareNodeSetting()
{
    TRACE_FUNC_ENTER();
    sp<FovCalculator> pFovCalculator = new FovCalculator(mSensorList, mIsVSDOF);
    NODE_LIST::iterator it, end;
    for( it = mpNodes.begin(), end = mpNodes.end(); it != end; ++it )
    {
        (*it)->setSensorList(mSensorList);
        (*it)->setCropCalculator(mpCropCalculator);
        (*it)->setFovCalculator(pFovCalculator);
        (*it)->setUsageHint(mUsageHint);
        (*it)->setTaskQueue(mpTaskQueue);
        if (mLogLevel > 0)
            (*it)->setLogLevel(mLogLevel);
    }

    mpP2Node->setBufferPool(mpBufferPool);
    mpYUVNode->setBufferPool(mpBufferPool);
    if (mIsVSDOF) {
        mpDepthNode->setBufferPool(mpBufferPool);
        mpYUV2Node->setBufferPool(mpBufferPool);
    }

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL CaptureFeaturePipe::prepareNodeConnection()
{
    TRACE_FUNC_ENTER();

    // TODO: construct a data table to describe the connecting edges
#if 0
    for (size_t i = 0; i < NUM_OF_PATH; i++) {

    }
#endif
    this->connectData(PID_ROOT_TO_RAW, *mpRootNode, *mpRAWNode);
    this->connectData(PID_ROOT_TO_P2A, *mpRootNode, *mpP2Node);
    this->connectData(PID_ROOT_TO_P2B, *mpRootNode, *mpP2Node);
    this->connectData(PID_ROOT_TO_MULTIRAW, *mpRootNode, *mpMultiFrameNode);
    this->connectData(PID_RAW_TO_P2A, *mpRAWNode, *mpP2Node);
    this->connectData(PID_MULTIRAW_TO_P2A, *mpMultiFrameNode, *mpP2Node);
    this->connectData(PID_P2A_TO_MULTIYUV, *mpP2Node, *mpMultiFrameNode);
    this->connectData(PID_P2A_TO_YUV, *mpP2Node, *mpYUVNode);
    this->connectData(PID_FD_TO_MULTIFRAME, *mpFDNode, *mpMultiFrameNode);
    this->connectData(PID_FD_TO_YUV, *mpFDNode, *mpYUVNode);
    this->connectData(PID_P2A_TO_MDP, *mpP2Node, *mpMDPNode);
    this->connectData(PID_P2A_TO_FD, *mpP2Node, *mpFDNode);
    this->connectData(PID_P2B_TO_YUV, *mpP2Node, *mpYUVNode);
    this->connectData(PID_P2B_TO_P2A, *mpP2Node, *mpP2Node);
    this->connectData(PID_MULTIFRAME_TO_YUV, *mpMultiFrameNode, *mpYUVNode);
    this->connectData(PID_MULTIFRAME_TO_MDP, *mpMultiFrameNode, *mpMDPNode);
    this->connectData(PID_YUV_TO_MDP, *mpYUVNode, *mpMDPNode);
    if (mIsVSDOF) {
        this->connectData(PID_P2A_TO_DEPTH, *mpP2Node, *mpDepthNode);
        this->connectData(PID_P2A_TO_FUSION, *mpP2Node, *mpFusionNode);
        this->connectData(PID_P2A_TO_YUV2, *mpP2Node, *mpYUV2Node);
        this->connectData(PID_FD_TO_DEPTH, *mpFDNode, *mpDepthNode);
        this->connectData(PID_FD_TO_FUSION, *mpFDNode, *mpFusionNode);
        this->connectData(PID_FD_TO_YUV2, *mpFDNode, *mpYUV2Node);
        this->connectData(PID_MULTIFRAME_TO_MDP_C, *mpMultiFrameNode, *mpMDPNode);
        this->connectData(PID_MULTIFRAME_TO_BOKEH, *mpMultiFrameNode, *mpBokehNode);
        this->connectData(PID_MULTIFRAME_TO_YUV2, *mpMultiFrameNode, *mpYUV2Node);
        this->connectData(PID_FUSION_TO_YUV, *mpFusionNode, *mpYUVNode);
        this->connectData(PID_FUSION_TO_YUV2, *mpFusionNode, *mpYUV2Node);
        this->connectData(PID_FUSION_TO_MDP, *mpFusionNode, *mpMDPNode);
        this->connectData(PID_DEPTH_TO_BOKEH, *mpDepthNode, *mpBokehNode);
        this->connectData(PID_YUV_TO_MDP_C, *mpYUVNode, *mpMDPNode);
        this->connectData(PID_YUV_TO_BOKEH, *mpYUVNode, *mpBokehNode);
        this->connectData(PID_YUV_TO_YUV2, *mpYUVNode, *mpYUV2Node);
        this->connectData(PID_BOKEH_TO_MDP_B, *mpBokehNode, *mpMDPNode);
        this->connectData(PID_BOKEH_TO_YUV2, *mpBokehNode, *mpYUV2Node);
        this->connectData(PID_BOKEH_TO_MDP, *mpBokehNode, *mpMDPNode);
        this->connectData(PID_YUV2_TO_MDP, *mpYUV2Node, *mpMDPNode);
    }

    NODE_LIST::iterator it, end;
    for(it = mpNodes.begin(), end = mpNodes.end(); it != end; ++it) {
        this->connectData(PID_DEQUE, PID_DEQUE, **it, this);
    }

    this->setRootNode(mpRootNode);
    mpRootNode->registerInputDataID(PID_ENQUE);

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MVOID CaptureFeaturePipe::releaseNodeSetting()
{
    TRACE_FUNC_ENTER();

    mpP2Node->setBufferPool(NULL);
    mpYUVNode->setBufferPool(NULL);
    if (mIsVSDOF) {
        mpDepthNode->setBufferPool(NULL);
        mpYUV2Node->setBufferPool(NULL);
    }

    this->disconnect();
    TRACE_FUNC_EXIT();
}

android::sp<ICaptureFeatureRequest>
CaptureFeaturePipe::acquireRequest()
{
    sp<CaptureFeatureRequest> pRequest = new CaptureFeatureRequest();
    return pRequest;
}

MVOID CaptureFeaturePipe::releaseRequest(sp<ICaptureFeatureRequest>)
{
}

MVOID CaptureFeaturePipe::setCallback(sp<RequestCallback> pCallback)
{
    mpCallback = pCallback;
}

const string CaptureFeaturePipe::MyDebuggee::mName {"NSCam::NSCamFeature::NSFeaturePipe::NSCapture::CaptureFeaturePipe"};

MVOID CaptureFeaturePipe::MyDebuggee::debug(android::Printer& printer, const std::vector<std::string>& options __unused)
{
    auto p = mContext.promote();
    if (CC_LIKELY(p != nullptr)) {
        p->dumpPlugin(printer);
        p->dumpRequest(printer);
    }
}

MVOID CaptureFeaturePipe::dumpPlugin(android::Printer& printer)
{
    #define DUMP_PLUGIN(type)                                                                                               \
    do {                                                                                                                    \
        type##Plugin::Ptr plugin = type##Plugin::getInstance(mUsageHint.mPluginUniqueKey);\
        std::stringstream stream;                                                                                           \
        stream << "== " << #type << "Plugin Dump ==" << std::endl;                                                          \
        plugin->dump(stream);                                                                                               \
        printer.printLine(stream.str().c_str());                                                                            \
    } while(0)

    DUMP_PLUGIN(Raw);
    DUMP_PLUGIN(MultiFrame);
    DUMP_PLUGIN(Yuv);

    if (mIsVSDOF) {
        DUMP_PLUGIN(Fusion);
        DUMP_PLUGIN(Bokeh);
        DUMP_PLUGIN(Depth);
    }
}

MVOID CaptureFeaturePipe::dumpRequest(android::Printer& printer)
{
    printer.printLine("== Running Request Dump ==");
    Mutex::Autolock _l(mRequestLock);
    for (const auto& request :mRunningRequests) {
        request->dump(printer);
    }
}

std::string CaptureFeaturePipe::getStatus()
{
    #define DUMP_PLUGIN_STATUS(type)                                                                                        \
    do {                                                                                                                    \
        type##Plugin::Ptr plugin = type##Plugin::getInstance(mUsageHint.mPluginUniqueKey);   \
        std::stringstream stream;                                                                                           \
        MY_LOGI("== %s Plugin Dump ==", #type);                                                                             \
        plugin->dump(stream);                                                                                               \
        MY_LOGI("%s", stream.str().c_str());                                                                                \
    } while(0)

    DUMP_PLUGIN_STATUS(Raw);
    DUMP_PLUGIN_STATUS(MultiFrame);
    DUMP_PLUGIN_STATUS(Yuv);

    if (mIsVSDOF) {
        DUMP_PLUGIN_STATUS(Fusion);
        DUMP_PLUGIN_STATUS(Bokeh);
        DUMP_PLUGIN_STATUS(Depth);
    }

    Mutex::Autolock _l(mRequestLock);
    std::string strDispatch;
    std::string strRet;
    if (mRunningRequests.size() > 0) {
        MY_LOGI("== Not Finish Request Dump ==");
        auto it = mRunningRequests.begin();

        std::string strProvider;
        std::string str;
        if (mpRAWNode) {
            strProvider += mpRAWNode->getStatus(strDispatch);
        }
        if (mpMultiFrameNode) {
            strProvider += mpMultiFrameNode->getStatus(strDispatch);
        }
        if (mpYUVNode) {
            strProvider += mpYUVNode->getStatus(strDispatch);
        }
        if (mpYUV2Node) {
            strProvider += mpYUV2Node->getStatus(strDispatch);
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
}

PipeBufferHandle::PipeBufferHandle(
    sp<CaptureBufferPool> pBufferPool, sp<CaptureTaskQueue> pTaskQueue, Format_T format, MSize& size, MSize& align)
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
    try {
        mFuture.wait();
    }
    catch (std::exception&) {
        MY_LOGW("future's wait throws exception");
    }
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


} // NSCapture
} // NSFeaturePipe
} // NSCamFeature
} // NSCam
