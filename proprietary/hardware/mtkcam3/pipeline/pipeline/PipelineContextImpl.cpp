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

#define LOG_TAG "MtkCam/ppl_context"
//
#include "MyUtils.h"
#include "Dispatcher.h"
#include "PipelineContextImpl.h"
#include "InitFrameHandler.h"
//
#include <chrono>
#include <functional>
#include <iomanip>
#include <list>
#include <map>
#include <sstream>
#include <thread>
#include <vector>
//
#if (MTKCAM_HAVE_AEE_FEATURE == 1)
#include <aee.h>
#endif
//
#include <mtkcam3/pipeline/stream/StreamId.h>
#include <mtkcam3/pipeline/hwnode/NodeId.h>
#include <mtkcam/utils/std/ULog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_CONTEXT);

//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v3::pipeline;
using namespace NSCam::v3::pipeline::NSPipelineContext;
using namespace NSCam::Utils::ULog;


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if (            (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if (            (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if (            (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


#if 1
#define FUNC_START     MY_LOGD("+")
#define FUNC_END       MY_LOGD("-")
#else
#define FUNC_START
#define FUNC_END
#endif


/******************************************************************************
 *
 ******************************************************************************/
static inline android::String8 toString_ItemImageStream(const StreamConfig::ItemImageStream& o)
{
    android::String8 os;

    os += toString(static_cast<StreamAttribute>(o.type)).c_str();
    if (CC_LIKELY( o.pInfo != nullptr )) {
        os += o.pInfo->toString();
    }

    return os;
};


/******************************************************************************
 *
 ******************************************************************************/
static inline android::String8 toString_ItemMetaStream(const StreamConfig::ItemMetaStream& o)
{
    android::String8 os;

    os += toString(static_cast<StreamAttribute>(o.type)).c_str();
    if (CC_LIKELY( o.pInfo != nullptr )) {
        os += o.pInfo->toString();
    }

    return os;
};


/******************************************************************************
 *
 ******************************************************************************/
PipelineContext::PipelineContextImpl::
PipelineContextImpl(char const* name)
    : mName(name)
    //
    , mpStreamConfig(new StreamConfig())
    , mpNodeConfig( new NodeConfig() )
    , mpPipelineConfig( new PipelineConfig() )
    //
{
    createFrameBufferManager(&mFrameBufferManager, name);
    MY_LOGF_IF(mFrameBufferManager==nullptr, "createFrameBufferManager(%s)", name);
    mFrameBufferManager->beginConfigure();
}


/******************************************************************************
 *
 ******************************************************************************/
PipelineContext::PipelineContextImpl::
~PipelineContextImpl()
{
}


/******************************************************************************
 *
 ******************************************************************************/
void
PipelineContext::PipelineContextImpl::
onLastStrongRef(const void* /*id*/)
{
    waitUntilInitFrameDrainedAndDestroyInitFrameHandler();
    //
    MY_LOGD("wait drained before destroy +");
    waitUntilDrained();
    MY_LOGD("wait drained before destroy -");
    //
    mpPipelineDAG     = NULL;
    mpPipelineNodeMap = NULL;
    mpDispatcher      = NULL;
    mpDataCallback    = NULL;
    mpInFlightRequest = NULL;
    //
    mpPipelineConfig = NULL;
    mpNodeConfig     = NULL;
    mpStreamConfig   = NULL;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineContext::PipelineContextImpl::
waitUntilConfigDone(const char* szCallName) -> int
{
    int err = OK;

    if ( ! mAsyncConfigFutures.empty() )
    {
        MY_LOGI("%s: wait for pipeline config done", szCallName);
        android::Mutex::Autolock _l(mAsyncConfigLock);
        if ( ! mAsyncConfigFutures.empty() )
        {
            for (auto& f : mAsyncConfigFutures) {
                int result = f.get();
                if (CC_UNLIKELY( OK != result )) {
                    MY_LOGE("fail to configure - err:%d", err);
                    err = result;
                }
            }
            mAsyncConfigFutures.clear();
        }
        MY_LOGI("%s: pipeline config done - err:%d", szCallName, err);
    }

    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineContext::PipelineContextImpl::
updateConfig(NodeBuilderImpl* pBuilder)
{
    RWLock::AutoWLock _l(mRWLock);
    //
    sp<ContextNode>&    pNode        = pBuilder->mpContextNode;
    StreamSet&          inStreamSet  = pBuilder->mInStreamSet;
    StreamSet&          outStreamSet = pBuilder->mOutStreamSet;
    StreamUsageMap&     usgMap       = pBuilder->mUsageMap;
    //
    if( inStreamSet.size() == 0 && outStreamSet.size() == 0)
    {
        MY_LOGE("should set in/out stream to node");
        return BAD_VALUE;
    }
    //
    sp<IStreamInfoSetControl> pInStreams = IStreamInfoSetControl::create();
    sp<IStreamInfoSetControl> pOutStreams = IStreamInfoSetControl::create();
    MERROR err;
    {
        set_streaminfoset_from_config::Params param =
        {
        .pStreamSet    = &inStreamSet,
        .pStreamConfig = mpStreamConfig.get(),
        .pSetControl   = pInStreams.get()
        };
        if( OK != (err = set_streaminfoset_from_config()(param)) ) {
            MY_LOGE("set_streaminfoset_from_config err:%d(%s)",
                    err, ::strerror(-err));
            return err;
        }
    }
    //
    {
        set_streaminfoset_from_config::Params param =
        {
        .pStreamSet    = &outStreamSet,
        .pStreamConfig = mpStreamConfig.get(),
        .pSetControl   = pOutStreams.get()
        };
        if( OK != (err = set_streaminfoset_from_config()(param)) ) {
            MY_LOGE("set_streaminfoset_from_config err:%d(%s)",
                    err, ::strerror(-err));
            return err;
        }
    }
    //
    pNode->setInStreams(pInStreams);
    pNode->setOutStreams(pOutStreams);

    // update to NodeConfig
    NodeId_T const nodeId = pNode->getNodeId();
    mpNodeConfig->addNode(nodeId, pNode);
    mpNodeConfig->setImageStreamUsage(nodeId, usgMap);
    //
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineContext::PipelineContextImpl::
reuseNode(
    NodeId_T const nodeId,
    android::sp<ContextNode> pNode,
    StreamUsageMap const& usgMap
)
{
    RWLock::AutoWLock _l(mRWLock);
    //
    if ( !pNode.get() )
        return BAD_VALUE;
    //
    MY_LOGD_IF(1, "Reuse node(%" PRIdPTR "): %s", nodeId, pNode->getNode()->getNodeName());
    mpNodeConfig->addNode(nodeId, pNode);
    mpNodeConfig->setImageStreamUsage(nodeId, usgMap);
    //
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineContext::PipelineContextImpl::
updateConfig(StreamBuilderImpl* pBuilder)
{
    auto attr = pBuilder->mAttribute;
    //
    {
        auto pFrameBufferManager = getFrameBufferManager();
        if (CC_LIKELY( pFrameBufferManager != nullptr ))
        {
            if( auto pStreamInfo = pBuilder->mpImageStreamInfo.get() )
            {
                pFrameBufferManager->addImageStream(
                    IFrameBufferManager::AddImageStream{
                        .pStreamInfo = pStreamInfo,
                        .attribute = static_cast<StreamAttribute>(attr),
                        .pProvider = pBuilder->mpProvider,
                });
            }
            else if( auto pStreamInfo = pBuilder->mpMetaStreamInfo.get() )
            {
                pFrameBufferManager->addMetaStream(
                    IFrameBufferManager::AddMetaStream{
                        .pStreamInfo = pStreamInfo,
                        .attribute = static_cast<StreamAttribute>(attr),
                });
            }
        }
    }

    RWLock::AutoWLock _l(mRWLock);
    //
    if( auto pStreamInfo = pBuilder->mpImageStreamInfo.get() )
    {
        // 1. check if this stream is already marked as reuse
        // TODO
        // 2. add <stream, pool or provider> to context
        sp<StreamConfig::ItemImageStream> pItem = new StreamConfig::ItemImageStream(pStreamInfo, attr);
        //

        // 4. add <stream, attribute> to context
        //MY_LOGD_IF(1, "New image stream: attribute 0x%x, %s", attr, pStreamInfo->toString().c_str());
        return mpStreamConfig->add(pItem);
    }
    else if( auto pStreamInfo = pBuilder->mpMetaStreamInfo.get() )
    {
        //MY_LOGD_IF(1, "New meta stream: attribute 0x%x, %s", attr, pStreamInfo->toString().c_str());
        sp<StreamConfig::ItemMetaStream> pItem = new StreamConfig::ItemMetaStream(pStreamInfo, attr);
        return mpStreamConfig->add(pItem);
    }
    MY_LOGE("not supported attribute 0x%x", attr);
    return UNKNOWN_ERROR;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineContext::PipelineContextImpl::
reuseStream(
    android::sp<StreamConfig::ItemImageStream> pItem)
{
    RWLock::AutoWLock _l(mRWLock);
    //
    if ( !pItem.get() )
        return BAD_VALUE;
    //
    MY_LOGD_IF( 1, "Reuse image stream: type 0x%x, %s", pItem->type, pItem->pInfo->toString().c_str());
    return mpStreamConfig->add(pItem);
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineContext::PipelineContextImpl::
updateConfig(PipelineBuilderImpl* pBuilder)
{
    RWLock::AutoWLock _l(mRWLock);
    //
    NodeSet const& rootNodes = pBuilder->mRootNodes;
    NodeEdgeSet const& edges = pBuilder->mNodeEdges;
    //
    MY_LOGD("root node size = %zu", rootNodes.size());
    //
    // check if nodes exist
    NodeConfig const* pNodeConfig = mpNodeConfig.get();
    for( size_t i = 0; i < edges.size(); i++ ) {
        NodeId_T const src = edges[i].src;
        NodeId_T const dst = edges[i].dst;
        if( pNodeConfig->queryNode(src) == NULL ) {
            MY_LOGE("cannot find node %#" PRIxPTR " from configuration", src);
            return NAME_NOT_FOUND;
        }
        if( pNodeConfig->queryNode(dst) == NULL ) {
            MY_LOGE("cannot find node %#" PRIxPTR " from configuration", dst);
            return NAME_NOT_FOUND;
        }
    }
    // update to context
    mpPipelineConfig->setRootNode(rootNodes);
    mpPipelineConfig->setNodeEdges(edges);
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineContext::PipelineContextImpl::
buildFrame(
    std::vector<FrameBuilderImpl*>const& builders
) -> std::vector<android::sp<IPipelineFrame>>
{
    std::vector<android::sp<IPipelineFrame>> out;
    out.reserve(builders.size());

    std::lock_guard<std::mutex> _l(mBuildFrameLock);

    for (auto& builder : builders) {
        auto pFrame = constructRequest(builder);
        if (CC_UNLIKELY( pFrame == nullptr )) {
            MY_LOGE("Failed to build %zu frames", builders.size());
            out.clear();
            break;
        }
        out.push_back( pFrame );
    }
    return out;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineContext::PipelineContextImpl::
buildFrame(
    FrameBuilderImpl* builder
) -> android::sp<IPipelineFrame>
{
    std::lock_guard<std::mutex> _l(mBuildFrameLock);

    return constructRequest(builder);
}


/******************************************************************************
 *
 ******************************************************************************/
namespace {
struct OverrideImageStreams
{
    int logLevel;

    // To override
    ImageStreamInfoMapT const&  aStreamInfosToOverride;

    // To be overridden (Hal)
    ImageStreamInfoMapT&        rStreamInfos_HalImage;

    // To be overridden (App)
    ImageStreamInfoMapT&        rStreamInfos_AppImage;
    AppImageStreamBufferMapsT&  rStreamBuffers_AppImage;
};
bool overrideImageStreams(OverrideImageStreams&& arg)
{
    auto overrideImageStreamInfo = [](auto& asIs, auto const& toBe){
            auto const& newStreamInfo = toBe;
            auto const streamId = newStreamInfo->getStreamId();
            ssize_t idx = asIs.indexOfKey(streamId);
            if ( idx < 0 ) {
                return false;
            }
            asIs.replaceValueAt(idx, newStreamInfo);
            return true;
        };

    auto overrideHalImageStream =
            [=, &rStreamInfoMap=arg.rStreamInfos_HalImage]
            (auto const& newStreamInfo) {

            bool ret = overrideImageStreamInfo(rStreamInfoMap, newStreamInfo);

            return ret;
        };

    auto overrideAppImageStream =
            [=, &rStreamInfoMap=arg.rStreamInfos_AppImage, &rBufferMap=arg.rStreamBuffers_AppImage]
            (auto const& newStreamInfo) {

            bool ret = overrideImageStreamInfo(rStreamInfoMap, newStreamInfo);

            if ( ret ) {//Override pBuilder->mStreamBuffers_AppImage
                auto found = rBufferMap.find(newStreamInfo->getStreamId());
                if ( found != rBufferMap.end() ) {
                    found->second.first = newStreamInfo;
                    found->second.second->replaceStreamInfo(newStreamInfo);//IImageStreamBuffer::replaceStreamInfo
                }
            }
            return ret;
        };


    auto const& aStreamInfosToOverride = arg.aStreamInfosToOverride;
    for (size_t i = 0; i < aStreamInfosToOverride.size(); i++)
    {
        auto const& newStreamInfo = aStreamInfosToOverride.valueAt(i);
        auto const streamId = newStreamInfo->getStreamId();

        bool ret = false;
        if ( streamId >= eSTREAMID_BEGIN_OF_INTERNAL ) {
            // Should most likely be HalImage streams; could still be AppImage streams.
            ret = overrideHalImageStream(newStreamInfo);
        }
        if ( ! ret /* If not HalImage, then try AppImage */ ) {
            // Must be AppImage streams if streamId is within [0, eSTREAMID_END_OF_APP)
            ret = overrideAppImageStream(newStreamInfo);
        }

        if (!ret) {
            MY_LOGE("Cannot find stream %s when trying to override it", newStreamInfo->getStreamName());
            return false;
        }

        MY_LOGD_IF(arg.logLevel>0,
            "Override image stream %#" PRIx64 "(%p) due to new stream info",
            streamId, newStreamInfo.get());
    }

    return true;
}
} //namespace


/******************************************************************************
 *
 ******************************************************************************/
sp<IPipelineFrame>
PipelineContext::PipelineContextImpl::
constructRequest(FrameBuilderImpl* pBuilder)
{
    // to make sure onRequestConstructed() can be called when this function
    // returns
    class scopedVar
    {
        public:
            scopedVar(FrameBuilderImpl* rpBuilder) : mpBuilder(rpBuilder) {}
            ~scopedVar() { mpBuilder->onRequestConstructed(); }
        private:
            FrameBuilderImpl* const   mpBuilder;
    } _localVar(pBuilder);
    //
    RWLock::AutoRLock _l(mRWLock);
    //
    typedef IPipelineBufferSetFrameControl          PipelineFrameT;
    //TODO: check context's status first!
    //TODO: check valid request
    //
    uint32_t const requestNo        = pBuilder->mRequestNo;
    MBOOL const& bReprocessFrame = pBuilder->mbReprocessFrame;
    NodeIOMaps const& aImageIOMaps  = pBuilder->mImageNodeIOMaps;
    NodeIOMaps const& aMetaIOMaps   = pBuilder->mMetaNodeIOMaps;
    NodeEdgeSet const& aEdges       = pBuilder->mNodeEdges;
    NodeSet const& aRoots           = pBuilder->mRootNodes;
    //
    wp<AppCallbackT> const& aAppCallback
                                    = pBuilder->mpCallback;
    //
    if( ! mpFrameNumberGenerator.get() ) { //TODO: can be removed after check context's status
        MY_LOGE("cannot get frameNo generator");
        return NULL;
    }
    //
    uint32_t const frameNo = mpFrameNumberGenerator->generateFrameNo();
    //
    #define MY_FUNC_ASSERT(expected, _ret_) \
    do{                                     \
        MERROR ret = (_ret_);               \
        if ( ret != expected ) {            \
            MY_LOGE("ret %d", ret);         \
            return NULL;                    \
        }                                   \
    } while(0)
    //
    //
    sp<PipelineFrameT> pFrame =
        PipelineFrameT::create(
        PipelineFrameT::CreationParams{
            .requestNo = requestNo,
            .frameNo = frameNo,
            .isReprocessFrame = static_cast<bool>(bReprocessFrame),
            .groupFrameType = pBuilder->mGroupFrameType,
            .pAppCallback = aAppCallback, // IAppCallback
            .pNodeCallback = mpDispatcher, // IPipelineNodeCallback
            .pStreamBufferProvider = mFrameStreamBufferProvider,
            .pHalResultProcessor = mFrameHalResultProcessor,
        });
    //
    if( ! pFrame.get() ) {
        MY_LOGE("cannot create PipelineFrame");
        return NULL;
    }
    //
    pFrame->startConfiguration();
    //
    // collect all used nodes/streams from NodeIOMaps
    //NodeSet reqNodes; //not necessary now
    StreamSet reqImgStreams;
    StreamSet reqMetaStreams;
    //
    collect_from_NodeIOMaps().getStreamSet(aImageIOMaps, reqImgStreams);
    collect_from_NodeIOMaps().getStreamSet(aMetaIOMaps, reqMetaStreams);
    //
    //collect_from_NodeIOMaps().getNodeSet(aImageIOMaps, reqNodes);
    //collect_from_NodeIOMaps().getNodeSet(aMetaIOMaps, reqNodes);
    //
    // get StreamId <-> type & (IImageStreamInfo or IMetaStreamInfo)
    struct
    {
        ImageStreamInfoMapT     vAppImageStreamInfo;
        ImageStreamInfoMapT     vHalImageStreamInfo;
        MetaStreamInfoMapT      vAppMetaStreamInfo;
        MetaStreamInfoMapT      vHalMetaStreamInfo;
    } aRequestData;
    //
    {
        collect_from_stream_config::Params params = {
        .pStreamConfig        = mpStreamConfig.get(),
        .pvImageStreamSet     = &reqImgStreams,
        .pvMetaStreamSet      = &reqMetaStreams,
        .pvAppImageStreamInfo = &aRequestData.vAppImageStreamInfo,
        .pvHalImageStreamInfo = &aRequestData.vHalImageStreamInfo,
        .pvAppMetaStreamInfo  = &aRequestData.vAppMetaStreamInfo,
        .pvHalMetaStreamInfo  = &aRequestData.vHalMetaStreamInfo
        };
        MY_FUNC_ASSERT(OK, collect_from_stream_config()(params));
    }

    // Override App image stream(s) based on App image stream buffers.
    //
    //      pBuilder->mStreamBuffers_AppImage --> aRequestData.vAppImageStreamInfo
    //
    for (auto const& v : pBuilder->mStreamBuffers_AppImage)
    {
        sp<IImageStreamInfo> pInfo = v.second.first;
        ssize_t idx = aRequestData.vAppImageStreamInfo.indexOfKey(pInfo->getStreamId());
        if( idx < 0 )
        {
            MY_LOGE("cannot replace APP IImageStreamInfo for stream %#" PRIx64,
            pInfo->getStreamId());
            return NULL;
        }
        MY_LOGD_IF(1, "replace APP stream %#" PRIx64, pInfo->getStreamId() );
        aRequestData.vAppImageStreamInfo.replaceValueAt(idx, pInfo);
    }

    // Override App / Hal image stream(s) based on new stream info(s) if needed.
    //
    //      pBuilder->mReplacingInfos --> aRequestData.vHalImageStreamInfo
    //  or  pBuilder->mReplacingInfos --> aRequestData.vAppImageStreamInfo, pBuilder->mStreamBuffers_AppImage
    //
    {
        bool ret =
            overrideImageStreams(OverrideImageStreams{
                .logLevel = 1,

                // To override
                .aStreamInfosToOverride = pBuilder->mReplacingInfos,

                // To be overridden
                .rStreamInfos_HalImage  = aRequestData.vHalImageStreamInfo,
                .rStreamInfos_AppImage  = aRequestData.vAppImageStreamInfo,
                .rStreamBuffers_AppImage= pBuilder->mStreamBuffers_AppImage,
            });
        if (!ret) {
            return nullptr;
        }
    }

    //
    sp<IPipelineDAG> pReqDAG = constructDAG(
            mpPipelineDAG.get(),
            aRoots,
            aEdges
            );
    if( ! pReqDAG.get() ) {
        return NULL;
    }
    //
    //
    sp<IStreamInfoSetControl> pReqStreamInfoSet;
    {
        sp<IStreamInfoSetControl> pStreamInfoSet = IStreamInfoSetControl::create();
        //
        update_streaminfo_to_set::Params params = {
        .pvAppImageStreamInfo = &aRequestData.vAppImageStreamInfo,
        .pvHalImageStreamInfo = &aRequestData.vHalImageStreamInfo,
        .pvAppMetaStreamInfo  = &aRequestData.vAppMetaStreamInfo,
        .pvHalMetaStreamInfo  = &aRequestData.vHalMetaStreamInfo,
        .pSetControl          = pStreamInfoSet.get()
        };
        MY_FUNC_ASSERT(OK, update_streaminfo_to_set()(params));
        //
        pReqStreamInfoSet = pStreamInfoSet;
    }
    //
    //
    sp<IPipelineFrameNodeMapControl> pReqFrameNodeMap;
    {
        sp<IPipelineFrameNodeMapControl> pFrameNodeMap = IPipelineFrameNodeMapControl::create();
        construct_FrameNodeMapControl::Params params = {
        .pImageNodeIOMaps  = &aImageIOMaps,
        .pMetaNodeIOMaps   = &aMetaIOMaps,
        .pReqDAG           = pReqDAG.get(),
        .pReqStreamInfoSet = pReqStreamInfoSet.get(),
        .pMapControl       = pFrameNodeMap.get()
        };
        MY_FUNC_ASSERT(OK, construct_FrameNodeMapControl()(params));
        //
        pReqFrameNodeMap = pFrameNodeMap;
    }
    //
    // update stream buffer
    MY_FUNC_ASSERT(OK, update_streambuffers_to_frame().
            updateAppMetaSB(
                aRequestData.vAppMetaStreamInfo, pBuilder->mStreamBuffers_AppMeta,
                pFrame.get()
                )
            );
    MY_FUNC_ASSERT(OK, update_streambuffers_to_frame().
            updateHalMetaSB(
                aRequestData.vHalMetaStreamInfo, pBuilder->mStreamBuffers_HalMeta,
                pFrame.get()
                )
            );
    MY_FUNC_ASSERT(OK, update_streambuffers_to_frame().
            updateAppImageSB(
                aRequestData.vAppImageStreamInfo, pBuilder->mStreamBuffers_AppImage,
                pFrame.get()
                )
            );
    MY_FUNC_ASSERT(OK, update_streambuffers_to_frame().
            updateHalImageSB(
                aRequestData.vHalImageStreamInfo, pBuilder->mStreamBuffers_HalImage,
                pFrame.get()
                )
            );
    //
    // userGraph of each stream buffer
    {
        evaluate_buffer_users::Params params = {
        .pProvider    = mpNodeConfig.get(),
        .pPipelineDAG = pReqDAG.get(),
        .pNodeMap     = pReqFrameNodeMap.get(),
        .pBufferSet   = pFrame.get()
        };
        MY_FUNC_ASSERT(OK, evaluate_buffer_users()(params));
    }
    //
    pFrame->setPipelineNodeMap (mpPipelineNodeMap.get());
    pFrame->setNodeMap         (pReqFrameNodeMap);
    pFrame->setPipelineDAG     (pReqDAG);
    pFrame->setStreamInfoSet   (pReqStreamInfoSet);
    // set physical camera setting
    pFrame->setPhysicalAppMetaStreamIds(pBuilder->mvPhysicalAppStreamIds);
    //
    if ( 0 != pBuilder->mReprocessSensorTimestamp ) {
        MY_LOGV("[requestNo:%u frameNo:%u] reprocess SensorTimestamp:%" PRIu64 "",
            requestNo, frameNo, pBuilder->mReprocessSensorTimestamp);
        pFrame->setSensorTimestamp(pBuilder->mReprocessSensorTimestamp, LOG_TAG);
    }
    //
    pFrame->setTrackFrameResultParams(pBuilder->mTrackFrameResultParams);
    pFrame->setUnexpectedToAbort(pBuilder->getEnabled(IFrameBuilder::ENABLED_FLAG_UNEXPECTED_TO_ABORT));
    //
    pFrame->finishConfiguration();
    //
    // TODO: performance optimization
    //
    return pFrame;
#undef MY_FUNC_ASSERT
}


/******************************************************************************
 *
 ******************************************************************************/
namespace {
struct LaunchOneNode
{
    using TimeoutHandlerT = std::function<void(android::sp<INodeActor>)>;

    android::sp<INodeActor> pNodeActor;
    TimeoutHandlerT         pTimeoutHandler;
    std::future<int>        future;

};
}


/******************************************************************************
 *
 ******************************************************************************/
static void handleLaunchNodeTimeout(android::sp<INodeActor> pNodeActor)
{
    std::string nodeName = pNodeActor->getNodeName();
    std::string msg = nodeName + "::init()/config() didn't return and timed out"
                    + "(" + std::to_string(pNodeActor->getInitTimeout()) + "ns)!"
                    + " Please firstly check the callstack of launchOneNode()."
                        ;

    MY_LOGE("%s", msg.c_str());

    if ( auto pNode = pNodeActor->getNode() ) {

        // For example:
        // "P1Node::init()/config() didn't return and timed out(10000000000ns)!"
        // " Please firstly check the callstack of launchOneNode()."
        // "\nCRDISPATCH_KEY:PipelineNode=P1Node"
        std::string crkey{"\nCRDISPATCH_KEY:PipelineNode=" + nodeName};
        pNode->triggerdb(IPipelineNode::TriggerDB{
            .msg = (msg + crkey).c_str(),
            .err = -ETIMEDOUT,
            .needDumpCallstack = true,
        });

    }
}


/******************************************************************************
 *
 ******************************************************************************/
static
auto
waitForNodesLaunched(std::list<LaunchOneNode>& list) -> int
{
    auto waitOneNodeDone = [](auto& x){

        if (CC_LIKELY( x.future.valid() )) {

            int64_t timeout = x.pNodeActor->getInitTimeout();
            if ( timeout >= 0 ) {
                auto status = x.future.wait_for(std::chrono::nanoseconds(timeout));
                if ( status == std::future_status::timeout ) {
                    if ( x.pTimeoutHandler ) {
                        x.pTimeoutHandler(x.pNodeActor);
                    }
                }
            }

            int result = x.future.get();
            return result;
        }

        return 0;
    };


    int err = 0;
    while ( ! list.empty() )
    {
        {
            int result = waitOneNodeDone(list.front());
            if (result != 0) {
                err = result;
            }
        }
        list.pop_front();
    }
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
static
auto
launchOneNode(
    android::sp<INodeActor> pIActor,
    android::sp<INodeCallbackToPipeline> pNodeCallback
) -> int
{
    CAM_TRACE_NAME((std::string("launchOneNode: ") + pIActor->getNodeName()).c_str());
    auto startTime = ::systemTime();

    auto triggerAEE = [=](char const* opsName, MERROR err) {
        std::string msg = std::string{"::"} + opsName + "() failed (called by PipelineContext)";

        if ( pIActor != nullptr && pIActor->getNode() != nullptr ) {
            auto pNode = pIActor->getNode();
            std::string nodeName = pIActor->getNodeName();
            // For example:
            // "P1Node::config() failed (called by PipelineContext)"
            // "\nCRDISPATCH_KEY:PipelineNode=P1Node"
            std::string crkey{"\nCRDISPATCH_KEY:PipelineNode=" + nodeName};
            pNode->triggerdb(IPipelineNode::TriggerDB{
                .msg = (nodeName + msg + crkey).c_str(),
                .err = err,
            });
        }
        else {
            #if (MTKCAM_HAVE_AEE_FEATURE == 1)
            aee_system_warning(
                LOG_TAG, NULL, DB_OPT_DEFAULT,
                msg.c_str());
            #endif
        }
    };

    MERROR err;
    err = pIActor->init();
    if (CC_UNLIKELY( OK != err )) {
        triggerAEE("init", err);
        return err;
    }

    err = pIActor->config();
    if (CC_UNLIKELY( OK != err )) {
        triggerAEE("config", err);
        return err;
    }

    auto pNode = pIActor->getNode();
    if (CC_UNLIKELY( pNode == nullptr )) {
        err = NO_INIT;
        triggerAEE("bad pIActor->getNode", err);
        return err;
    }
    err = pNode->setNodeCallBack(pNodeCallback);
    if (CC_UNLIKELY( OK != err )) {
        triggerAEE("setNodeCallBack", err);
        return err;
    }

    auto elapsedTime = ::systemTime()-startTime;
    pIActor->setInitElapsedTime(elapsedTime);
    MY_LOGD("%3" PRId64 "ms: %s", ns2ms(elapsedTime), pIActor->getNodeName());
    return OK;
};


/******************************************************************************
 *
 ******************************************************************************/
static
auto
launchNodes(
    std::list<LaunchOneNode>& list/*in/out*/,
    android::sp<INodeCallbackToPipeline> pNodeCallback,
    bool isAsync
) -> void
{
    for (auto& v : list) {
        v.future =
            std::async(
                isAsync ? std::launch::async : std::launch::deferred,
                launchOneNode,
                v.pNodeActor,
                pNodeCallback
            );
    }
}


/******************************************************************************
 *
 ******************************************************************************/
static
auto
configureNodes(
    android::sp<IPipelineDAG> pPipelineDAG,
    android::sp<NodeConfig> pNodeConfig,
    android::sp<INodeCallbackToPipeline> pNodeCallback,
    bool isAsync,
    std::function<void(int)> notifyConfigDone
) -> int
{
    using LaunchOneNodeSetT = std::list<LaunchOneNode>;
    auto toString = [](const LaunchOneNodeSetT& o){
        std::string os;
        os += "{ ";
        for (auto const& v : o) {
            os += v.pNodeActor->getNodeName();
            os += " ";
        }
        os += "}";
        return os;
    };

    MY_LOGD("+");
    int err = OK;

    LaunchOneNodeSetT aLaunchNodes_freeRunOrder;                        //init-order = -1
    std::map<int32_t/*init order*/, LaunchOneNodeSetT> aLaunchNodesMap; //init-order >= 0
    /**
     *  The nodes with init-order = -1 will be launched concurrently (without any dependency).
     *  The nodes with init-order >= 0 will be launched in order
     *  (The smaller value, the higher priority order).
     *
     *  Take the following case for example, the init/config order will be
     *  {A,B} -> {C} -> {D,E,F},
     *  where
     *      A and B will be launched concurrently, and
     *      D, E, and F will be launched concurrently.
     *  ____________________________________________________________________
     *  init order | nodes which can be launched concurrently
     *      0      |     A B
     *      1      |     C
     *      2      |     D E F
     *     ...     |     ...
     */

    Vector<IPipelineDAG::NodeObj_T> const& rToposort = pPipelineDAG->getToposort();
    Vector<IPipelineDAG::NodeObj_T>::const_iterator it = rToposort.begin();
    for (; it != rToposort.end(); it++)
    {
        auto nodeId = it->id;
        sp<ContextNode> pContextNode = pNodeConfig->queryNode(nodeId);
        sp<INodeActor> pIActor = pContextNode.get() ? pContextNode->getNodeActor() : NULL;
        if  (CC_UNLIKELY( pIActor == nullptr )) {
            MY_LOGE("cannot find node %#" PRIxPTR " from Node Config", nodeId);
            return UNKNOWN_ERROR;
        }
        //
        int32_t order = pIActor->getInitOrder();
        if ( isAsync && order >= 0 ) {
            //Returns a pair consisting of an iterator to the inserted element,
            //or the already-existing element if no insertion happened,
            //and a bool denoting whether the insertion took place.
            auto ret = aLaunchNodesMap.emplace(order, LaunchOneNodeSetT());
            LaunchOneNodeSetT& aNodes = (*ret.first).second;
            aNodes.push_back(
                LaunchOneNode{
                    .pNodeActor = pIActor,
                    .pTimeoutHandler = handleLaunchNodeTimeout,
                });
        }
        else {
            aLaunchNodes_freeRunOrder.push_back(
                LaunchOneNode{
                    .pNodeActor = pIActor,
                    .pTimeoutHandler = handleLaunchNodeTimeout,
                });
        }
    }

    // [Debug] Dump the init/config order of aLaunchNodes_freeRunOrder & aLaunchNodesMap
    MY_LOGD("init-order:-1 %s", toString(aLaunchNodes_freeRunOrder).c_str());
    for (auto const& v : aLaunchNodesMap) {
        MY_LOGD("init-order:%2d %s", v.first, toString(v.second).c_str());
    }


    // Launch the nodes with free-run order (init-order = -1).
    launchNodes(aLaunchNodes_freeRunOrder, pNodeCallback, isAsync);

    // Launch the nodes in order (init-order >= 0; The smaller value, the higher priority order).
    for (auto& v : aLaunchNodesMap) {
        // The nodes with the same init-order are launched concurrently.
        // The nodes with lower priority order cannot be launched until all higher priority nodes finish.
        launchNodes(v.second, pNodeCallback, isAsync);
        auto e = waitForNodesLaunched(v.second);
        if (CC_UNLIKELY( e != OK )) {
            err = e;
        }
    }

    // Wait for the finish of launching the nodes with free-run order.
    err = waitForNodesLaunched(aLaunchNodes_freeRunOrder);

    // Notify the completion of configuration.
    if ( notifyConfigDone ) {
        notifyConfigDone(err);
    }

    MY_LOGD("- err:%d", err);
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineContext::PipelineContextImpl::
config(
    PipelineContextImpl* pOldContext,
    MBOOL const isAsync,
    MBOOL const isMultiThread
)
{
    {
        auto tmp = getFrameBufferManager();
        if (CC_LIKELY( tmp != nullptr )) {
            if (CC_UNLIKELY( 0 != tmp->endConfigure() )) {
                MY_LOGE("[%s] Failed on IFrameBufferManager::endConfigure()", mName.c_str());
                return UNKNOWN_ERROR;
            }
            mFrameStreamBufferProvider = tmp->getPipelineFrameBufferProvider();
            mFrameHalResultProcessor   = tmp->getPipelineFrameResultProcessor();
        }
    }

    std::shared_ptr<InitFrameHandler> pInitFrameHandler;
    if ( mConfigInitFrameCount.load() > 0 ) {
        std::lock_guard<std::timed_mutex> _lg(mInitFrameHandlerLock);
        mInitFrameHandler = InitFrameHandler::make(mConfigInitFrameCount.load(), this);
        pInitFrameHandler = mInitFrameHandler;
    }

    RWLock::AutoWLock _l(mRWLock);
    //
    MERROR err = OK;
    // get FrameNumberGenerator only in node reuse case
    if( pOldContext != NULL )
        mpFrameNumberGenerator = pOldContext->mpFrameNumberGenerator;
    MY_LOGD_IF(mpFrameNumberGenerator.get(), "FrameNumberGenerator(%p)", mpFrameNumberGenerator.get());
    if( ! mpFrameNumberGenerator.get() )
        mpFrameNumberGenerator = IPipelineFrameNumberGenerator::create();
    //
    {
        sp<IPipelineDAG> pDAG                = IPipelineDAG::create();
        sp<IPipelineNodeMapControl> pNodeMap = IPipelineNodeMapControl::create();
        //
        config_pipeline::Params param =
        {
        .pNodeConfig     = mpNodeConfig.get(),
        .pPipelineConfig = mpPipelineConfig.get(),
        .pDAG            = pDAG.get(),
        .pNodeMap        = pNodeMap.get()
        };
        if( OK != (err = config_pipeline()(param)) ) {
            MY_LOGE("config_pipeline err:%d(%s)", err, ::strerror(-err));
            return err;
        }
        //
        mpPipelineDAG     = pDAG;
        mpPipelineNodeMap = pNodeMap;
    }
    // in-flight
    if( ! mpDispatcher.unsafe_get() ) {
        mpDispatcher_Default = DefaultDispatcher::create();
        mpDispatcher = mpDispatcher_Default;
        mpDispatcher_Default->setDataCallback(mpDataCallback);
    }
    //
    mpInFlightRequest = new InFlightRequest();
    //
    // config each node
    {
        {
            android::Mutex::Autolock _l(mAsyncConfigLock);
            mAsyncConfigFutures.push_back(
                std::async(isMultiThread ? std::launch::async : std::launch::deferred,
                    configureNodes,
                    mpPipelineDAG,
                    mpNodeConfig,
                    this,
                    isAsync,
                    [pInitFrameHandler](int err){
                        if ( pInitFrameHandler != nullptr ) {
                            pInitFrameHandler->setConfigDone(err);
                        }
                    }
                )
            );
        }
        //
        if (!isMultiThread)
        {
            err = waitUntilConfigDone(__FUNCTION__);
        }
    }
    //
    // TODO: should keep error status to avoid requesting.
    //
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineContext::PipelineContextImpl::
submitFrame(std::vector<android::sp<IPipelineFrame>>const& frames) -> int
{
    int err = OK;

    {
        std::lock_guard<std::timed_mutex> _l(mSubmitFrameLock);

        for (auto const& f : frames) {

            int e = submitOneFrame(f);

            if (CC_UNLIKELY( e != OK )) {
                MY_LOGE("[requestNo:%u frameNo:%u] err:%d(%s)",
                    f->getRequestNo(), f->getFrameNo(),
                    e, ::strerror(-e));
                err = e;
            }

        }
    }

    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineContext::PipelineContextImpl::
submitOneFrame(android::sp<IPipelineFrame>const& pFrame) -> int
{
    mFrameCountQueued++;

    ///////////////////////////////////////////////////////
    // [1] Not use init. frames
    if ( mConfigInitFrameCount.load() == 0 ) {
        // Send it to the pipeline root node(s).
        return sendFrame(pFrame);
    }

    ///////////////////////////////////////////////////////
    // [2] Use init. frames

    // [2.1]
    // We don't have enough init. frames.
    // Put it into the queue of init. frame handler. (non-blocking call)
    if ( mConfigInitFrameCount.load() > mFrameCountQueued.load() ) {
        {
            std::lock_guard<std::timed_mutex> _lg(mInitFrameHandlerLock);
            if (CC_LIKELY( mInitFrameHandler != nullptr )) {
                // Do enqueue inside the lock to avoid that a frame is enqueued
                // "after" waitUntilInitFrameDrainedAndDestroyInitFrameHandler().
                return mInitFrameHandler->enqueue(pFrame); //non-blocking call
            }
        }
        MY_LOGW(
            "Bad InitFrameHandler! "
            "It might be destroyed (due to flush?) before all init frames were received "
            "- mConfigInitFrameCount:%u mFrameCountQueued:%u",
            mConfigInitFrameCount.load(), mFrameCountQueued.load()
        );
    }

    // [2.1]
    // We had received enough init. frames already.
    {
        // Wait until all init. frames are sent to the pipeline root node(s).
        // Then destory the handler.
        waitUntilInitFrameDrainedAndDestroyInitFrameHandler();

        // Send it to the pipeline root node(s).
        return sendFrame(pFrame);
    }

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineContext::PipelineContextImpl::
waitUntilInitFrameDrained() -> void
{
    {
        std::lock_guard<std::timed_mutex> _lg(mInitFrameHandlerLock);

        if ( mInitFrameHandler != nullptr ) {
            MY_LOGI("+");
            mInitFrameHandler->waitUntilDrained();
            MY_LOGI("-");
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineContext::PipelineContextImpl::
waitUntilInitFrameDrainedAndDestroyInitFrameHandler() -> void
{
    {
        std::lock_guard<std::timed_mutex> _lg(mInitFrameHandlerLock);

        if ( mInitFrameHandler != nullptr ) {
            MY_LOGI("+");
            mInitFrameHandler->deactivateAndWaitUntilDrained();
            mInitFrameHandler = nullptr;
            MY_LOGI("-");
        }
    }
}

/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineContext::PipelineContextImpl::
waitEnqueReady( size_t NotEnqueSize ) -> bool
{
    if (NotEnqueSize != 0)
    {
        Mutex::Autolock lock(mEnqueLock);
        if(!mEnqueReady)
        {
            mCondEnque.waitRelative(mEnqueLock,33000000);
        }
        mEnqueReady = MFALSE;
    }
    else
    {
        return false;
    }
    return true;
}

/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineContext::PipelineContextImpl::
sendFrameToRootNodes(android::sp<IPipelineFrame>const& pFrame,
                     Vector<sp<IPipelineNode>> &vspPipelineNode) -> int
{
    #define FRAME_SENDING (1)
    #define FRAME_FLUSHING (2)
    int32_t sendStatus = 0;
    int32_t tempStatus = 0;
    MERROR err = OK;
    auto pPipelineBufferSetFrameControl = IPipelineBufferSetFrameControl::castFrom(pFrame.get());
    Mutex::Autolock _l(mKickLock);
    do
    {
        /**
         * If we do CAM_ULOG_ENTER "after" queue() a frame, the order of ulog may reverse.
         * Thus, what we can do is to do CAM_ULOG_ENTER "before" queue() a frame, no matter
         * whether queue() succeed or not. However, the timestamp of ulog may happen much
         * earlier than queue().
         * In addition, do not CAM_ULOG_ENTER the same frame more than once.
         */
        // wake up and enque to root node which can not be enqued last time
        RWLock::AutoRLock _l(mFlushLock);
        Vector<sp<IPipelineNode>>::iterator it = vspPipelineNode.begin();
        while(it != vspPipelineNode.end())
        {
            if  ( isNeededToFlush(pFrame) && sendStatus != FRAME_SENDING ) {
                AutoLogDispatch(pPipelineBufferSetFrameControl, eNODEID_UNKNOWN, (*it)->getNodeId(), "flush");
                pPipelineBufferSetFrameControl->abort();
                err = (*it)->flush(pFrame);
                tempStatus = FRAME_FLUSHING;
            }
            else {
                err = (*it)->queue(pFrame);
                tempStatus = FRAME_SENDING;
            }
            switch (err)
            {
            case OK:
                {
                    MY_LOGD("r%d frameNo:%d (X) -> 0x%" PRIxPTR " success; erase record",
                        pFrame->getRequestNo(), pFrame->getFrameNo(), (*it)->getNodeId());
                    logDispatch(pPipelineBufferSetFrameControl, eNODEID_UNKNOWN, (*it)->getNodeId());
                    it = vspPipelineNode.erase(it);
                    if (sendStatus == 0)
                    {
                        sendStatus = tempStatus;
                    }
                }
                break;
            case FAILED_TRANSACTION:
                {
                    std::string str = (std::ostringstream() << "(X) -> 0x" << std::hex << (*it)->getNodeId() << " BUSY" ).str();
                    MY_LOGD("r%d frameNo:%d %s", pFrame->getRequestNo(), pFrame->getFrameNo(), str.c_str());
                    pPipelineBufferSetFrameControl->logDebugInfo(std::move(str));
                    it++;
                }
                break;
            default:
                {
                    std::string str = (std::ostringstream() << "(X) -> 0x" << std::hex << (*it)->getNodeId() << " error:" << std::dec << err ).str();
                    MY_LOGE("r%d frameNo:%d %s", pFrame->getRequestNo(), pFrame->getFrameNo(), str.c_str());
                    pPipelineBufferSetFrameControl->logDebugInfo(std::move(str));
                    return err;
                }
                break;
            }
        }
    } while ( sendStatus != 0 && waitEnqueReady(vspPipelineNode.size()) );
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineContext::PipelineContextImpl::
sendFrame(android::sp<IPipelineFrame>const& pFrame) -> int
{
    CAM_TRACE_NAME(!ATRACE_ENABLED()?"":(
        std::string("queue|") +
        "requestNo:" + std::to_string(pFrame->getRequestNo()) +
        " frameNo:" + std::to_string(pFrame->getFrameNo())
    ).c_str());

    auto pPipelineBufferSetFrameControl = IPipelineBufferSetFrameControl::castFrom(pFrame.get());
    if (CC_LIKELY( pPipelineBufferSetFrameControl != nullptr )) {
        auto tmp = getFrameBufferManager();
        if (CC_LIKELY( tmp != nullptr )) {
            tmp->trackFrameResult(
                IFrameBufferManager::TrackFrameResult{
                    .frameNo = pFrame->getFrameNo(),
                    .requestNo = pFrame->getRequestNo(),
                    .trackParams = pPipelineBufferSetFrameControl->getTrackFrameResultParams(),
                });
        }
    }

    mLastRequestNo = pFrame->getRequestNo();
    if ( m1stRequestNo == BAD_REQUEST_NO ) {
        m1stRequestNo = pFrame->getRequestNo();
    }
    //
    RWLock::AutoRLock _l(mRWLock);
    if  (CC_UNLIKELY( ! mpInFlightRequest.get() || ! mpDispatcher.unsafe_get() )) { //TODO, may remove this check
        MY_LOGE("not configured yet!");
        return UNKNOWN_ERROR;
    }

    int errConfig = waitUntilConfigDone(__FUNCTION__);
    MY_LOGE_IF(OK!=errConfig, "fail to configure - err:%d", errConfig);
    MY_LOGD_IF( 0, "regist inflight request(%p) %d", this, pFrame->getFrameNo());
    mpInFlightRequest->registerRequest(pFrame);
    //
    sp<IPipelineNodeMap const> pPipelineNodeMap = pFrame->getPipelineNodeMap();
    if  (CC_UNLIKELY( pPipelineNodeMap == 0 || pPipelineNodeMap->isEmpty() )) {
        MY_LOGE("[frameNo:%d] Bad PipelineNodeMap:%p", pFrame->getFrameNo(), pPipelineNodeMap.get());
        return DEAD_OBJECT;
    }
    //
    {
        Mutex::Autolock _l(mLastFrameLock);
        mpLastFrame = pFrame;
    }
    //
    {   // only wait for the regular kick-processing
        Mutex::Autolock _l(mKickLock);
    }
    //
    MERROR err = OK;
    Vector<IPipelineDAG::NodeObj_T> const RootNodeObjSet = pFrame->getPipelineDAG().getRootNode();
    Vector<sp<IPipelineNode>> vspPipelineNode;
    for(size_t i=0; i<RootNodeObjSet.size();i++)
    {

        sp<IPipelineNode> pNode = pPipelineNodeMap->nodeAt(RootNodeObjSet[i].val);
        if  ( pNode == 0 ) {
            MY_LOGE("[frameNo:%d] Bad root node", pFrame->getFrameNo());
            return DEAD_OBJECT;
        }
        vspPipelineNode.push_back(pNode);
    }

    do
    {
        if ( (err = sendFrameToRootNodes(pFrame, vspPipelineNode)) != OK )
        {
            return err;
        }
    } while ( waitEnqueReady(vspPipelineNode.size()) );

    //
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineContext::PipelineContextImpl::
kick(
    sp<IPipelineFrame> const& pFrame
)
{
    RWLock::AutoRLock _l(mRWLock);
    sp<IPipelineNodeMap const> pPipelineNodeMap = pFrame->getPipelineNodeMap();
    if (CC_UNLIKELY( pPipelineNodeMap == 0 || pPipelineNodeMap->isEmpty() )) {
        MY_LOGE("[requestNo:%u frameNo:%u] Bad PipelineNodeMap:%p", pFrame->getRequestNo(), pFrame->getFrameNo(), pPipelineNodeMap.get());
        return DEAD_OBJECT;
    }
    //
    Vector<IPipelineDAG::NodeObj_T> const RootNodeObjSet = pFrame->getPipelineDAG().getRootNode();
    //
    MERROR err = OK;
    {
        Mutex::Autolock _l(mKickLock);
        {
            RWLock::AutoRLock _l(mFlushLock);
            if  ( isNeededToFlush(pFrame) ) {
                // do-nothing
                MY_LOGI("[requestNo:%u frameNo:%u] kick but flushing", pFrame->getRequestNo(), pFrame->getFrameNo());
            }
            else
            {
                for(size_t i=0;i<RootNodeObjSet.size();i++)
                {
                    sp<IPipelineNode> pNode = pPipelineNodeMap->nodeAt(RootNodeObjSet[i].val);
                    if (CC_UNLIKELY( pNode == 0 )) {
                        MY_LOGE("[requestNo:%u frameNo:%u] Bad root node", pFrame->getRequestNo(), pFrame->getFrameNo());
                        return DEAD_OBJECT;
                    }

                    MY_LOGI("[requestNo:%u frameNo:%u] kick begin", pFrame->getRequestNo(), pFrame->getFrameNo());
                    err = pNode->kick();
                    MY_LOGI("[requestNo:%u frameNo:%u] kick end. err:%d", pFrame->getRequestNo(), pFrame->getFrameNo(), err);
                }
            }
        }
    }
    //
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineContext::PipelineContextImpl::
waitUntilDrained()
{
    waitUntilInitFrameDrained();

    RWLock::AutoRLock _l(mRWLock);
    if( mpInFlightRequest.get() )
        mpInFlightRequest->waitUntilDrained();
    else
        MY_LOGD("may not configured yet");
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineContext::PipelineContextImpl::
waitUntilRootNodeDrained() -> int
{
    FUNC_START;

    int err = 0;

    auto roots = [this](){
        android::RWLock::AutoRLock _l(mRWLock);
        return mpPipelineConfig->getRootNode();
    }();

    for (size_t i = 0; i < roots.size(); i++) {
        int e = waitUntilNodeDrained(roots[i]);
        if (CC_UNLIKELY( e != 0 )) {
            MY_LOGE("nodeId:%#" PRIxPTR " err:%d", roots[i], e);
            err = e;
        }
    }

    FUNC_END;
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineContext::PipelineContextImpl::
waitUntilNodeDrained(NodeId_T const nodeId)
{
    int errConfig = waitUntilConfigDone(__FUNCTION__);
    MY_LOGE_IF(OK!=errConfig, "fail to configure - err:%d", errConfig);

    waitUntilInitFrameDrained();

    RWLock::AutoRLock _l(mRWLock);
    if( mpInFlightRequest.get() ) {
        mpInFlightRequest->waitUntilNodeDrained(nodeId);
    }
    else {
        MY_LOGD("may not configured yet");
    }
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineContext::PipelineContextImpl::
isNeededToFlush(
    android::sp<IPipelineFrame> const& pFrame
) const -> bool
{
    if ( mIsFlushing.load() /*atomic*/ ) {
        return true;
    }

    if ( mFlushRequestNo != BAD_REQUEST_NO ) {

        auto requestNo = pFrame->getRequestNo();

        bool ret = false;
        if (CC_LIKELY( mFlushRequestNo - m1stRequestNo >= 0 )) {
            ret = (requestNo <= mFlushRequestNo) && CC_LIKELY(m1stRequestNo <= requestNo);
            MY_LOGD("ret:%d requestNo:%d in [%d %d]", ret, requestNo, m1stRequestNo.load(), mFlushRequestNo.load());
        }
        else {
            ret = (requestNo <= mFlushRequestNo) || (m1stRequestNo <= requestNo);
            MY_LOGD("ret:%d requestNo:%d in [%d %d]", ret, requestNo, mFlushRequestNo.load(), m1stRequestNo.load());
        }
        return ret;
    }

    return false;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineContext::PipelineContextImpl::
kickRootNodes() -> void
{
    {
        android::Mutex::Autolock _l(mLastFrameLock);
        android::sp<IPipelineFrame> pLastFrame = mpLastFrame.promote();
        if (pLastFrame != nullptr) {
            MY_LOGI("[requestNo:%u frameNo:%u] last frame",
                pLastFrame->getRequestNo(), pLastFrame->getFrameNo());
        }
    }

    {
        android::RWLock::AutoRLock _l(mRWLock);
        auto roots = mpPipelineConfig->getRootNode();
        {
            android::Mutex::Autolock _l(mKickLock);
            for (size_t i = 0; i < roots.size(); i++) {
                android::sp<IPipelineNode> pNode = mpPipelineNodeMap->nodeFor(roots[i]);
                if (CC_LIKELY( pNode != nullptr )) {
                    MY_LOGI("kick %#" PRIxPTR " +", roots[i]);
                    int err = pNode->kick();
                    MY_LOGI("kick %#" PRIxPTR " - err:%d", roots[i], err);
                }
            }
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineContext::PipelineContextImpl::
beginFlush()
{
    FUNC_START;
    //
    int errConfig = waitUntilConfigDone(__FUNCTION__);
    MY_LOGE_IF(OK!=errConfig, "fail to configure - err:%d", errConfig);

    // Mark every frame aborted in a decrementing order of frame number
    {
        {
            RWLock::AutoWLock _l(mFlushLock);
            mIsFlushing = true;
        }
        {
            android::sp<IDispatcher> pDispatcher = mpDispatcher.promote();
            if( pDispatcher != nullptr ) {
                pDispatcher->beginFlush();
            }
            else {
                MY_LOGW("cannot promote dispatcher");
            }
        }
        {
            RWLock::AutoRLock _l(mRWLock);
            mpInFlightRequest->abort();
        }
    }

    // Kick root nodes.
    kickRootNodes();

    // Flush every nodes in toposort order.
    {
        RWLock::AutoRLock _l(mRWLock);
        //
        sp<IPipelineNodeMap> pPipelineNodeMap = mpPipelineNodeMap;
        Vector<IPipelineDAG::NodeObj_T> const& rToposort = mpPipelineDAG->getToposort();
        Vector<IPipelineDAG::NodeObj_T>::const_iterator it = rToposort.begin();
        for (; it != rToposort.end(); it++)
        {
            sp<IPipelineNode> pNode = pPipelineNodeMap->nodeAt(it->val);
            if  (CC_UNLIKELY( pNode == 0 )) {
                MY_LOGE("NULL node (id:%" PRIxPTR ")", it->id);
                continue;
            }
            //
            if (CC_UNLIKELY( pNode->flush() != OK ))
            {
                MY_LOGE("Fail to flush node (id:%" PRIxPTR ")", it->id);
            }
        }
    }
    //
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineContext::PipelineContextImpl::
endFlush()
{
    FUNC_START;
    //
    {
        sp<IDispatcher> pDispatcher = mpDispatcher.promote();
        if( pDispatcher.get() )
            pDispatcher->endFlush();
        else
            MY_LOGW("cannot promote dispatcher");
    }
    //
    {
        RWLock::AutoWLock _l(mFlushLock);
        mFlushRequestNo.store(mLastRequestNo.load());
        mIsFlushing = false;
    }
    //
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineContext::PipelineContextImpl::
setDataCallback(
    wp<IDataCallback> pCallback
)
{
    MY_LOGD("set DataCallback %p", pCallback.unsafe_get());
    sp<IDispatcher> pDispatcher = mpDispatcher.promote();
    if( pDispatcher.get() )
    {
        pDispatcher->setDataCallback(pCallback);
    }
    mpDataCallback = pCallback;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineContext::PipelineContextImpl::
setInitFrameCount(uint32_t count) -> void
{
    android::RWLock::AutoWLock _l(mRWLock);
    mConfigInitFrameCount = count;
    MY_LOGD("%u", count);
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineContext::PipelineContextImpl::
getFrameBufferManager() const -> std::shared_ptr<IFrameBufferManager>
{
    android::RWLock::AutoRLock _l(mRWLock);
    return mFrameBufferManager;
}


/******************************************************************************
 *
 ******************************************************************************/
sp<INodeActor>
PipelineContext::PipelineContextImpl::
queryNode(NodeId_T const nodeId) const
{
    RWLock::AutoRLock _l(mRWLock);
    sp<ContextNode> pContextNode = mpNodeConfig->queryNode(nodeId);
    return pContextNode.get() ? pContextNode->getNodeActor() : NULL;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineContext::PipelineContextImpl::
dumpState(
    android::Printer& printer __unused,
    const std::vector<std::string>& options __unused
) -> void
{
    printer.printFormatLine("<%s>", mName.c_str());

    sp<InFlightRequest> pInFlightRequest;
    {
        RWLock::AutoRLock _l(mRWLock);
        //
        mpPipelineConfig->dumpState(printer);
        mpNodeConfig->dumpState(printer);

        pInFlightRequest = mpInFlightRequest;
    }

    {
        auto tmp = getFrameBufferManager();
        if (CC_LIKELY( tmp != nullptr )) {
            tmp->dumpState(printer);
        }
    }

    printer.printLine("");
    printer.printFormatLine("mConfigInitFrameCount=%u", mConfigInitFrameCount.load());
    {
        auto pInitFrameHandler = [&, this](){
            std::unique_lock<std::timed_mutex> _l(mInitFrameHandlerLock, std::defer_lock);
            if ( _l.try_lock_for(std::chrono::milliseconds(50)) == false ) {
                printer.printFormatLine("Timed out on lock. Try to unsafely dump mInitFrameHandler:%p", mInitFrameHandler.get());
            }
            return mInitFrameHandler;
        }();
        if ( pInitFrameHandler != nullptr ) {
            android::PrefixPrinter prefixPrinter(printer, " ");
            pInitFrameHandler->dumpState(prefixPrinter);
            printer.printLine("");
        }
    }

    printer.printFormatLine("mFrameCountQueued=%u", mFrameCountQueued.load());
    printer.printFormatLine("RequestNo:[%d,%d] (being) queued!",
        m1stRequestNo.load(), mLastRequestNo.load());
    printer.printFormatLine("RequestNo<=%d (being) flushed! %s",
        mFlushRequestNo.load(), (mIsFlushing.load() ? "Flushing" : "No flush"));
    if ( pInFlightRequest != nullptr ) {
        printer.printLine("");
        android::PrefixPrinter prefixPrinter(printer, " ");
        pInFlightRequest->dumpState(prefixPrinter, options);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
PipelineContext::PipelineContextImpl::
onCallback(CallBackParams param)
{
    Mutex::Autolock lock(mEnqueLock);
    //
    MY_LOGD("param: (id:%" PRIxPTR "), (lastFrameNum = %d), (eNoticeType = %d)",
        param.nodeId, param.lastFrameNum,param.noticeType);
    if(param.noticeType == eNotice_ReadyToEnque)
    {
        mCondEnque.signal();
        mEnqueReady = MTRUE;
    }
    return;
}


MERROR
config_pipeline::
operator()(Params& rParams)
{
    MERROR err = OK;
    //
    NodeConfig const* pNodeConfig         = rParams.pNodeConfig;
    PipelineConfig const* pPipelineConfig = rParams.pPipelineConfig;
    IPipelineDAG* pDAG                    = rParams.pDAG;
    IPipelineNodeMapControl* pNodeMap     = rParams.pNodeMap;
    //
    if( !pNodeConfig || !pPipelineConfig || !pDAG || !pNodeMap ) {
        MY_LOGE("NULL in params");
        return UNKNOWN_ERROR;
    }
    //
    ContextNodeMapT const& configNodeMap = pNodeConfig->getContextNodeMap();
    //
    pNodeMap->setCapacity( configNodeMap.size() );
    //
    // nodes
    for ( size_t i = 0; i < configNodeMap.size(); i++ )
    {
        struct copy_IStreamInfoSetControl
        {
            MVOID operator() (
                    sp<IStreamInfoSetControl const> const& src,
                    sp<IStreamInfoSetControl> const& dst
                    )
            {
                dst->editAppMeta() = src->getAppMeta();
                dst->editHalMeta() = src->getHalMeta();
                dst->editAppImage() = src->getAppImage();
                dst->editHalImage() = src->getHalImage();
            }
        };
        //
        sp<ContextNode> pNode = configNodeMap.valueAt(i);
        //
        pDAG->addNode(pNode->getNodeId(), i);
        //
        sp<IPipelineNodeMapControl::INode> const pINode =
            pNodeMap->getNodeAt( pNodeMap->add(pNode->getNodeId(), pNode->getNode()) );
        // in/out
        copy_IStreamInfoSetControl()(pNode->getInStreams(), pINode->editInStreams());
        copy_IStreamInfoSetControl()(pNode->getOutStreams(), pINode->editOutStreams());
    }
    {
        NodeSet const& roots = pPipelineConfig->getRootNode();
        NodeEdgeSet const& nodeEdges = pPipelineConfig->getNodeEdges();
        // edge
        Vector<NodeEdge>::const_iterator iter = nodeEdges.begin();
        for(; iter != nodeEdges.end(); iter++) {
            err = pDAG->addEdge(iter->src, iter->dst);
            if( err != OK ) return err;
        }
        // root
        if  ( roots.size() )
            pDAG->setRootNode(roots);
        else {
            MY_LOGE("No RootNode!");
            return INVALID_OPERATION;
        }
    }
    //
    if  ( pDAG->getToposort().isEmpty() ) {
        MY_LOGE("It seems that the connection of nodes cannot from a DAG...");
        err = UNKNOWN_ERROR;
    }
    //
    //  dump nodes.
    if( 0 )
    {
        for (size_t i = 0; i < pNodeMap->size(); i++) {
            IPipelineNodeMap::NodePtrT const& pNode = pNodeMap->nodeAt(i);
            MY_LOGD("%#" PRIxPTR ":%s", pNode->getNodeId(), pNode->getNodeName());
        }
        //
        Vector<String8> logs;
        pDAG->dump(logs);
        for (size_t i = 0; i < logs.size(); i++) {
            MY_LOGD("%s", logs[i].string());
        }
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
collect_from_NodeIOMaps::
getStreamSet(
    NodeIOMaps const& nodeIOMap,
    StreamSet& collected
    )
{
    for( size_t i = 0; i < nodeIOMap.size(); i++ )
    {
        IOMapSet const& IOMapSet = nodeIOMap.valueAt(i);
        IOMapSet::const_iterator pIOMap = IOMapSet.begin();
        for (; pIOMap != IOMapSet.end(); pIOMap++)
        {
            collected.add(pIOMap->vIn);
            collected.add(pIOMap->vOut);
        }
    }
};


#if 0
/******************************************************************************
 *
 ******************************************************************************/
MVOID
collect_from_NodeIOMaps::
getNodeSet(
    NodeIOMaps const& nodeIOMap,
    NodeSet& collected
    )
{
    for( size_t i = 0; i < nodeIOMap.size(); i++ ) {
        collected.add(nodeIOMap.keyAt(i));
    }
};
#endif


/******************************************************************************
 *
 ******************************************************************************/
sp<IPipelineDAG>
NSPipelineContext::
constructDAG(
    IPipelineDAG const* pConfigDAG,
    NodeSet const& rootNodes,
    NodeEdgeSet const& edges
)
{
    NodeSet requestNodeSet;
    {
        NodeEdgeSet::const_iterator iter = edges.begin();
        for (; iter != edges.end(); iter++)
        {
            requestNodeSet.add(iter->src);
            requestNodeSet.add(iter->dst);
        }
        //
        NodeSet::const_iterator iterNode = rootNodes.begin();
        for (; iterNode != rootNodes.end(); iterNode++)
            requestNodeSet.add(*iterNode);
    }

    //
    sp<IPipelineDAG> pDAG = IPipelineDAG::create();
    for( size_t i = 0; i < requestNodeSet.size(); i++ )
    {
        NodeId_T const nodeId = requestNodeSet[i];
        IPipelineDAG::NodeObj_T obj = pConfigDAG->getNode(nodeId);
        if( obj.val == -1 ) { // invalid
            MY_LOGE("cannot find node %#" PRIxPTR, nodeId);
            return NULL;
        }
        pDAG->addNode(nodeId, obj.val);
    }
    // set root
    if( OK != pDAG->setRootNode(rootNodes) ) {
        MY_LOGE("set root node failed");
        return NULL;
    }
    // set edges
    {
        Vector<NodeEdge>::const_iterator iter = edges.begin();
        for(; iter != edges.end(); iter++ ) {
            if( OK != pDAG->addEdge(iter->src, iter->dst) )
                return NULL;
        }
    }
    //
    if  ( pDAG->getToposort().isEmpty() ) {
        MY_LOGE("It seems that the connection of nodes cannot from a DAG...");
        return NULL;
    }
    //
    // dump
    if( 0 ) {
        Vector<String8> logs;
        pDAG->dump(logs);
        for (size_t i = 0; i < logs.size(); i++) {
            MY_LOGD("%s", logs[i].string());
        }
    }
    return pDAG;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
set_streaminfoset_from_config::
operator() (Params& rParams)
{
    StreamSet const*        pStreamSet    = rParams.pStreamSet;
    //
    for( auto streamId : *pStreamSet )
    {
        auto search_then_add = [
            streamId,
            pStreamConfig = rParams.pStreamConfig,
            pSetControl = rParams.pSetControl
        ]
        (
            auto queryAPI,      // query[Image|Meta]
            auto editAppAPI,    // editApp[Image|Meta]
            auto editHalAPI     // editHal[Image|Meta]
        )
        {
            auto pItem = std::invoke(queryAPI, pStreamConfig, streamId);
            if ( pItem != nullptr ) {
                switch ( (uint32_t)(pItem->type) & (uint32_t)(StreamAttribute::BEHAVIOR_MASK) )
                {
                case (uint32_t)(StreamAttribute::BEHAVIOR_APP):
                    std::invoke(editAppAPI, pSetControl).addStream(pItem->pInfo);
                    return true;
                case (uint32_t)(StreamAttribute::BEHAVIOR_HAL):
                    std::invoke(editHalAPI, pSetControl).addStream(pItem->pInfo);
                    return true;
                default:
                    CAM_ULOGME("unknown app or hal streamId:%#" PRIx64 "", streamId);
                    break;
                }
            }
            return false;
        };

        // search from configured images, then from configured meta
        bool ret =
                search_then_add(
                    &StreamConfig::queryImage,
                    &IStreamInfoSetControl::editAppImage,
                    &IStreamInfoSetControl::editHalImage
                )
            ||  search_then_add(
                    &StreamConfig::queryMeta,
                    &IStreamInfoSetControl::editAppMeta,
                    &IStreamInfoSetControl::editHalMeta
                );
        if (CC_LIKELY( ret ))
            continue;
        //
        MY_LOGE("cannot find stream(%#" PRIx64 ") from configuration", streamId);
        MY_LOGW("=== dump configuration begin ===");
        rParams.pStreamConfig->dump();
        MY_LOGW("=== dump configuration end ===");
        return NAME_NOT_FOUND;
    }
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
collect_from_stream_config::
operator()(Params& rParams)
{
    auto classifyStreams = [pStreamConfig = rParams.pStreamConfig](
        char const* typeName,   // "image" / "meta"
        auto queryAPI,          // query[Image|Meta]
        auto pStreams,          // given: a set of stream id
        auto pvAppInfos,        // result: a set of app streams
        auto pvHalInfos         // result: a set of hal streams
    )
    {
        for (auto streamId : *pStreams) {
            auto pItem = std::invoke(queryAPI, pStreamConfig, streamId);
            if (CC_UNLIKELY( pItem == nullptr )) {
                CAM_ULOGME("cannot find %s streamId:%#" PRIx64 "", typeName, streamId);
                return BAD_VALUE;
            }

            switch ( (uint32_t)(pItem->type) & (uint32_t)(StreamAttribute::BEHAVIOR_MASK) )
            {
            case (uint32_t)(StreamAttribute::BEHAVIOR_APP):{
                pvAppInfos->add(streamId, pItem->pInfo);
                }break;
            case (uint32_t)(StreamAttribute::BEHAVIOR_HAL):{
                pvHalInfos->add(streamId, pItem->pInfo);
                }break;
            default:
                CAM_ULOGME("unknown app or hal %s streamId:%#" PRIx64 "", typeName, streamId);
                return UNKNOWN_ERROR;
            }
        }
        return OK;
    };

    if (CC_UNLIKELY( rParams.pStreamConfig == nullptr )) {
        MY_LOGE("Bad StreamConfig");
        return UNKNOWN_ERROR;
    }
    //
    int err = OK;
    err = classifyStreams(
                    "image",
                    &StreamConfig::queryImage,
                    rParams.pvImageStreamSet,
                    rParams.pvAppImageStreamInfo, rParams.pvHalImageStreamInfo
                );
    if( err != OK ) return err;
    //
    err = classifyStreams(
                    "meta",
                    &StreamConfig::queryMeta,
                    rParams.pvMetaStreamSet,
                    rParams.pvAppMetaStreamInfo, rParams.pvHalMetaStreamInfo
                );

    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
update_streaminfo_to_set::
operator() (
    Params& rParams
    )
{
#define updateInfo(_name_, _type_, pStreamInfoMap)                   \
do {                                                         \
    IStreamInfoSetControl::Map<I##_type_##StreamInfo>& map = \
        pSetControl->edit##_name_##_type_();                 \
    map.setCapacity(pStreamInfoMap->size());                 \
    for( size_t i = 0; i < pStreamInfoMap->size(); i++ )     \
    {                                                        \
        map.addStream(pStreamInfoMap->valueAt(i));           \
            if ( FRAME_STREAMINFO_DEBUG_ENABLE )                     \
            {                                                        \
                String8 str = pStreamInfoMap->valueAt(i)->toString();\
                MY_LOGD("update info: %s", str.string());            \
            }                                                        \
    }                                                        \
} while(0)

    IStreamInfoSetControl* pSetControl = rParams.pSetControl;
    if( ! pSetControl ) return UNKNOWN_ERROR;

    updateInfo(App, Image, rParams.pvAppImageStreamInfo);
    updateInfo(Hal, Image, rParams.pvHalImageStreamInfo);
    updateInfo(App, Meta , rParams.pvAppMetaStreamInfo);
    updateInfo(Hal, Meta , rParams.pvHalMetaStreamInfo);

#undef updateInfo
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
update_streambuffers_to_frame::
updateAppMetaSB(
        MetaStreamInfoMapT const& rvStreamInfo,
        MetaStreamBufferMapsT const& rvSBuffers,
        PipelineFrameT* pFrame
        ) const
{
    typedef IMetaStreamBuffer SBufferT;
    //
    Vector< sp<SBufferT> > vStreamBuffers;
    for( size_t i = 0; i < rvStreamInfo.size(); i++ )
    {
        StreamId_T const streamId = rvStreamInfo.keyAt(i);
        auto SBuffer = rvSBuffers.valueFor(streamId);
        if( ! SBuffer.get() ) { // allocate here
            sp<IMetaStreamInfo> pStreamInfo = rvStreamInfo.valueAt(i);
            SBuffer = HalMetaStreamBufferAllocatorT(pStreamInfo.get())();
        }
        //
        vStreamBuffers.push_back(SBuffer);
    }
    //
    auto pBufMap = pFrame->editMap_AppMeta();
    pBufMap->setCapacity( vStreamBuffers.size() );
    for( size_t i = 0; i < vStreamBuffers.size(); i++ ) {
        MY_LOGD_IF(FRAMEE_STREAMBUFFER_DEBUG_ENABLE,
                "stream %#" PRIx64,
                vStreamBuffers[i]->getStreamInfo()->getStreamId());
        pBufMap->add(vStreamBuffers[i]);
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
update_streambuffers_to_frame::
updateHalMetaSB(
        MetaStreamInfoMapT const& rvStreamInfo,
        MetaStreamBufferMapsT const& rvSBuffers,
        PipelineFrameT* pFrame
        ) const
{
    typedef IMetaStreamBuffer SBufferT;
    //
    Vector< sp<SBufferT> > vStreamBuffers;
    for( size_t i = 0; i < rvStreamInfo.size(); i++ )
    {
        StreamId_T const streamId = rvStreamInfo.keyAt(i);
        auto SBuffer = rvSBuffers.valueFor(streamId);
        if( ! SBuffer.get() ) { // allocate here
            sp<IMetaStreamInfo> pStreamInfo = rvStreamInfo.valueAt(i);
            SBuffer = HalMetaStreamBufferAllocatorT(pStreamInfo.get())();
        }
        vStreamBuffers.push_back(SBuffer);
    }
    //
    auto pBufMap = pFrame->editMap_HalMeta();
    pBufMap->setCapacity( vStreamBuffers.size() );
    for( size_t i = 0; i < vStreamBuffers.size(); i++ ) {
        MY_LOGD_IF(FRAMEE_STREAMBUFFER_DEBUG_ENABLE,
                "stream %#" PRIx64,
                vStreamBuffers[i]->getStreamInfo()->getStreamId());
        pBufMap->add(vStreamBuffers[i]);
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
update_streambuffers_to_frame::
updateAppImageSB(
        ImageStreamInfoMapT const& rvStreamInfo,
        AppImageStreamBufferMapsT const& rvSBuffers,
        PipelineFrameT* pFrame
        ) const
{
    if( rvStreamInfo.size() != rvSBuffers.size() ) {
        MY_LOGE("Inconsistent streams from IOMap and IFrameBuilder");
        MY_LOGW("<Streams from IOMap> #%zu", rvStreamInfo.size());
        for (size_t i = 0; i < rvStreamInfo.size(); i++) {
            if ( auto p = rvStreamInfo[i].get() ) {
                MY_LOGW("    %s", p->toString().c_str());
            }
        }
        MY_LOGW("<Streams from IFrameBuilder> #%zu", rvSBuffers.size());
        for (auto const& v : rvSBuffers) {
            if ( auto p = v.second.first.get() ) {
                MY_LOGW("    %s%s", p->toString().c_str(), (v.second.second==nullptr ? "(NO BUFFER)" : ""));
            }
        }
        return BAD_VALUE;
    }
    //
    auto pBufMap = pFrame->editMap_AppImage();
    //
    pBufMap->setCapacity(rvSBuffers.size());
    for (auto const& v : rvSBuffers)
    {
        auto const  streamId = v.first;
        auto const& pStreamInfo = v.second.first;
        auto const& pStreamBuffer = v.second.second;
        MY_LOGD_IF(FRAMEE_STREAMBUFFER_DEBUG_ENABLE, "%s%s",
            pStreamInfo->toString().c_str(),
            (pStreamBuffer==nullptr ? "(NO BUFFER)" : ""));

        if ( pStreamBuffer != nullptr ) {
            pBufMap->add(pStreamBuffer);
        }
        else {
            pBufMap->add(pStreamInfo, nullptr);
        }
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
update_streambuffers_to_frame::
updateHalImageSB(
    ImageStreamInfoMapT const& rvStreamInfo,
    ImageStreamBufferMapsT const& vSBuffers,
    PipelineFrameT* pFrame
    ) const
{
    auto pBufMap = pFrame->editMap_HalImage();
    //
    pBufMap->setCapacity( rvStreamInfo.size() );
    for( size_t i = 0; i < rvStreamInfo.size(); i++ )
    {
        MY_LOGD_IF(FRAMEE_STREAMBUFFER_DEBUG_ENABLE,
                "stream %#" PRIx64,
                rvStreamInfo.valueAt(i)->getStreamId());
        auto pBuffer =
            vSBuffers.valueFor(rvStreamInfo.valueAt(i)->getStreamId());
        if( pBuffer.get() )
            pBufMap->add(pBuffer);
        else
            pBufMap->add(rvStreamInfo.valueAt(i), NULL);
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
construct_FrameNodeMapControl::
operator() (Params& rParams)
{
    typedef IPipelineFrameNodeMapControl    FrameNodeMapT;
    //
    struct setINodeIOStreams
    {
        MVOID   operator()  (
                IOMapSet const& rImageIOMapSet,
                IOMapSet const& rMetaIOMapSet,
                IStreamInfoSet const* const pReqStreamInfoSet,
                FrameNodeMapT::INode* pNode
                )
        {
            typedef NSCam::v3::Utils::SimpleStreamInfoSetControl StreamInfoSetT;
            sp<StreamInfoSetT> pIStreams = new StreamInfoSetT;
            sp<StreamInfoSetT> pOStreams = new StreamInfoSetT;
            pNode->setIStreams(pIStreams);
            pNode->setOStreams(pOStreams);

#define setINodeIO(type, rIOMapSet)                                         \
            do {                                                            \
                IOMapSet::const_iterator it = rIOMapSet.begin();            \
                for (; it != rIOMapSet.end(); it++)                         \
                {                                                           \
                    IPipelineFrame::type##InfoIOMap map;                    \
                    for (size_t i = 0; i < it->vIn.size(); i++)             \
                    {                                                       \
                        StreamId_T const streamId = it->vIn[i];             \
                        sp<I##type##StreamInfo> pStreamInfo =               \
                        pReqStreamInfoSet->get##type##InfoFor(streamId);    \
                        map.vIn.add(streamId, pStreamInfo);                 \
                        pIStreams->edit##type().add(streamId, pStreamInfo); \
                    }                                                       \
                    for (size_t i = 0; i < it->vOut.size(); i++)            \
                    {                                                       \
                        StreamId_T const streamId = it->vOut[i];            \
                        sp<I##type##StreamInfo> pStreamInfo =               \
                        pReqStreamInfoSet->get##type##InfoFor(streamId);    \
                        map.vOut.add(streamId, pStreamInfo);                \
                        pOStreams->edit##type().add(streamId, pStreamInfo); \
                    }                                                       \
                    pNode->editInfoIOMapSet()                               \
                    .m##type##InfoIOMapSet.push_back(map);                  \
                }                                                           \
            } while(0)

            setINodeIO(Image, rImageIOMapSet);
            setINodeIO(Meta, rMetaIOMapSet);

#undef setINodeIO
        }
        //
        MVOID   dumpINodeIO(
                FrameNodeMapT::INode* pNode
                )
        {
            MY_LOGD("nodeId %#" PRIxPTR , pNode->getNodeId() );
            InfoIOMapSet const& aIOMapSet = pNode->getInfoIOMapSet();
#define dump( type, rIOMapSet )                                                                \
            do {                                                                               \
                for( size_t idx = 0; idx < rIOMapSet.size(); idx++ )                           \
                {                                                                              \
                    IPipelineFrame::type##InfoIOMap const& aIOMap = rIOMapSet[idx];            \
                    String8 inStream, outStream;                                               \
                    for( size_t i = 0; i < aIOMap.vIn.size(); i++ ) {                          \
                        inStream += String8::format("(%#" PRIx64 ")", aIOMap.vIn.keyAt(i));   \
                    }                                                                          \
                    for( size_t i = 0; i < aIOMap.vOut.size(); i++ ) {                         \
                        outStream += String8::format("(%#" PRIx64 ")", aIOMap.vOut.keyAt(i)); \
                    }                                                                          \
                    MY_LOGD("%s #%zu", #type, idx);                                             \
                    MY_LOGD("    In : %s", inStream.string());                                      \
                    MY_LOGD("    Out: %s", outStream.string());                                    \
                }                                                                              \
            } while(0)
            dump(Image, aIOMapSet.mImageInfoIOMapSet);
            dump(Meta, aIOMapSet.mMetaInfoIOMapSet);
#undef dump
        }
    };
    //
    FrameNodeMapT* pNodeMap = rParams.pMapControl;
    pNodeMap->setCapacity( rParams.pReqDAG->getNumOfNodes() );
    //
    Vector<IPipelineDAG::NodeObj_T> const& rToposort   = rParams.pReqDAG->getToposort();
    Vector<IPipelineDAG::NodeObj_T>::const_iterator it = rToposort.begin();
    for (; it != rToposort.end(); it++)
    {
        NodeId_T const nodeId = it->id;
        //
        FrameNodeMapT::INode* pNode =
            pNodeMap->getNodeAt( pNodeMap->addNode(nodeId) );
        //
        setINodeIOStreams() (
                rParams.pImageNodeIOMaps->valueFor(nodeId),
                rParams.pMetaNodeIOMaps->valueFor(nodeId),
                rParams.pReqStreamInfoSet,
                pNode
                );
        //
        // debug
        if( FRAMENODEMAP_DEBUG_ENABLE ) {
            setINodeIOStreams().dumpINodeIO(pNode);
        }
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
struct evaluate_buffer_users::Imp
{
    typedef IPipelineFrameNodeMapControl    FrameNodeMapT;
    typedef Vector<IPipelineDAG::NodeObj_T> ToposortT;
    IPipelineDAG const*                     mpPipelineDAG;
    Vector<IPipelineDAG::Edge>              mvEdges;

    MERROR
    operator()(Params& rParams)
    {
        CAM_TRACE_NAME("evaluate_request_buffer_users");
        mpPipelineDAG   = rParams.pPipelineDAG;
        mvEdges.clear();
        rParams.pPipelineDAG->getEdges(mvEdges);
        //
        evaluateAppImage(rParams);
        evaluateHalImage(rParams);
        evaluateAppMeta (rParams);
        evaluateHalMeta (rParams);
        //
        return  OK;
    }

#define _IMP_EVALUATE_(_NAME_, _TYPE_) \
    MERROR \
    evaluate##_NAME_##_TYPE_(Params& rParams) \
    { \
        MakeUser_##_NAME_##_TYPE_ makeUser(rParams.pProvider, rParams.pNodeMap); \
        doIt(makeUser, rParams.pBufferSet->editMap_##_NAME_##_TYPE_()); \
        return OK; \
    }

    _IMP_EVALUATE_(App, Image);
    _IMP_EVALUATE_(App, Meta);
    _IMP_EVALUATE_(Hal, Image);
    _IMP_EVALUATE_(Hal, Meta);

#undef  _IMP_EVALUATE_

    template <class MakeUserT, class MapT>
    MVOID
    doIt(
        MakeUserT const& makeUser,
        MapT pBufMap
    )
    {
        ToposortT const& rToposort = mpPipelineDAG->getToposort();
        for (size_t i = 0; i < pBufMap->size(); i++)
        {
            sp<IUsersManager>const& pUsersManager = pBufMap->usersManagerAt(i);

            //User graph of current buffer
            sp<IUsersManager::IUserGraph> userGraph = pUsersManager->createGraph();

            //Add users
            ToposortT::const_iterator user = rToposort.begin();
            do
            {
                if(pBufMap->streamInfoAt(i))
                {
                    userGraph->addUser(makeUser(pBufMap->streamInfoAt(i), user->id));
                }
                //
                user++;
            }  while ( user != rToposort.end() );

            //Add edges
            for (size_t j = 0; j < mvEdges.size(); j++) {
                userGraph->addEdge(mvEdges.itemAt(j).src, mvEdges.itemAt(j).dst);
            }

            //Enqueue graph
            pUsersManager->enqueUserGraph(userGraph);
            pUsersManager->finishUserSetup();
        }
    }

    struct  MakeUserBase
    {
        NodeConfig const*                   mpProvider;
        FrameNodeMapT const*                mpNodeMap;

        IUsersManager::User
        makeImageUser(
            IImageStreamInfo const* pStreamInfo,
            NodeId_T const nodeId
        )   const
        {
            StreamId_T const streamId = pStreamInfo->getStreamId();
            //
            IUsersManager::User user;
            user.mUserId = nodeId;
            //
            FrameNodeMapT::INode* pNode = mpNodeMap->getNodeFor(nodeId);
            refineUser(
                user, streamId,
                pNode->getOStreams()->getImageInfoMap(),
                pNode->getIStreams()->getImageInfoMap()
            );
            if  ( IUsersManager::Category::NONE != user.mCategory ) {
                user.mUsage = mpProvider->queryMinimalUsage(nodeId, pStreamInfo->getStreamId());
            }
            //
            return user;
        }

        IUsersManager::User
        makeMetaUser(
            IMetaStreamInfo const* pStreamInfo,
            NodeId_T const nodeId
        )   const
        {
            StreamId_T const streamId = pStreamInfo->getStreamId();
            //
            IUsersManager::User user;
            user.mUserId = nodeId;
            //
            FrameNodeMapT::INode* pNode = mpNodeMap->getNodeFor(nodeId);
            refineUser(
                user, streamId,
                pNode->getOStreams()->getMetaInfoMap(),
                pNode->getIStreams()->getMetaInfoMap()
            );
            //
            return user;
        }

        template <class StreamsT>
        MVOID
        refineUser(
            IUsersManager::User& rUser,
            StreamId_T const streamId,
            StreamsT const& pOStreams,
            StreamsT const& pIStreams
        )   const
        {
            if  ( pOStreams != 0 && pIStreams != 0 )
            {
                if  ( 0 <= pOStreams->indexOfKey(streamId) )
                {
                    rUser.mCategory = IUsersManager::Category::PRODUCER;
                    return;
                }
                //
                if  ( 0 <= pIStreams->indexOfKey(streamId) )
                {
                    rUser.mCategory = IUsersManager::Category::CONSUMER;
                    return;
                }
                //
                MY_LOGD_IF(
                    0,
                    "streamId:%#" PRIx64 " nodeId:%#" PRIxPTR ": not found in IO streams",
                    streamId, rUser.mUserId
                );
            }
            else
            {
                MY_LOGW_IF(
                    1,
                    "streamId:%#" PRIx64 " nodeId:%#" PRIxPTR ": no IO streams(%p,%p)",
                    streamId, rUser.mUserId, pIStreams.get(), pOStreams.get()
                );
            }
            //
            rUser.mCategory = IUsersManager::Category::NONE;
            rUser.mReleaseFence = rUser.mAcquireFence;
        }
    };

#define _DEFINE_MAKEUSER_(_NAME_, _TYPE_)                                   \
    struct  MakeUser_##_NAME_##_TYPE_                                       \
        : public MakeUserBase                                               \
    {                                                                       \
        MakeUser_##_NAME_##_TYPE_(                                          \
            NodeConfig const* pProvider,                                    \
            FrameNodeMapT const* pNodeMap                                   \
        )                                                                   \
        {                                                                   \
            mpProvider = pProvider;                                         \
            mpNodeMap  = pNodeMap;                                          \
        }                                                                   \
                                                                            \
        IUsersManager::User                                                 \
        operator()(                                                         \
            I##_TYPE_##StreamInfo const* pStreamInfo,                       \
            NodeId_T const nodeId                                           \
        )   const                                                           \
        {                                                                   \
            return make##_TYPE_##User(                                      \
                pStreamInfo,                                                \
                nodeId                                                      \
            );                                                              \
        }                                                                   \
    };

    _DEFINE_MAKEUSER_(App, Image);
    _DEFINE_MAKEUSER_(App, Meta);
    _DEFINE_MAKEUSER_(Hal, Image);
    _DEFINE_MAKEUSER_(Hal, Meta);

#undef _DEFINE_MAKEUSER_

};


/******************************************************************************
 *
 ******************************************************************************/
MERROR
evaluate_buffer_users::
operator() (Params& rParams)
{
    return Imp()(rParams);
    // TODO:
    // pseudo code
    // for each stream in BufferSetControl
    //     parse users via DAG & IPipelineFrameNodeMapControl
    //     contruct a list of users (WRRRWRRR...)
    //     create graph
};


/******************************************************************************
 *
 ******************************************************************************/
StreamConfig::
~StreamConfig()
{
}


/******************************************************************************
 *
 ******************************************************************************/
void
StreamConfig::
dumpState(android::Printer& printer __unused) const
{
    RWLock::AutoRLock _l(mRWLock);

    printer.printLine("<image streams>");
    for( size_t i = 0; i < mStreamMap_Image.size(); i++ )
    {
        sp<ItemImageStream> pItem = mStreamMap_Image.valueAt(i);
        printer.printFormatLine("    %s", toString_ItemImageStream(*pItem).c_str());
    }
    printer.printLine("<meta streams>");
    for( size_t i = 0; i < mStreamMap_Meta.size(); i++ )
    {
        sp<ItemMetaStream> pItem = mStreamMap_Meta.valueAt(i);
        printer.printFormatLine("    %s", toString_ItemMetaStream(*pItem).c_str());
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
StreamConfig::
dump() const
{
    ULogPrinter logPrinter(__ULOG_MODULE_ID, LOG_TAG, DetailsType::DETAILS_INFO, "[dump] ");
    dumpState(logPrinter);
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
NodeConfig::
addNode(
    NodeId_T const nodeId,
    sp<ContextNode> pNode
)
{
    RWLock::AutoWLock _l(mRWLock);
    mConfig_NodeMap.add(nodeId, pNode);
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
NodeConfig::
setImageStreamUsage(
    NodeId_T const nodeId,
    StreamUsageMap const& usgMap
)
{
    RWLock::AutoWLock _l(mRWLock);
    mNodeImageStreamUsage.add(nodeId, usgMap);
}


/******************************************************************************
 *
 ******************************************************************************/
sp<ContextNode> const
NodeConfig::
queryNode(
    NodeId_T const nodeId
) const
{
    RWLock::AutoRLock _l(mRWLock);
    return mConfig_NodeMap.valueFor(nodeId);
}


/******************************************************************************
 *
 ******************************************************************************/
MUINT
NodeConfig::
queryMinimalUsage(
    NodeId_T const nodeId,
    StreamId_T const streamId
) const
{
    RWLock::AutoRLock _l(mRWLock);
    ssize_t index_node = mNodeImageStreamUsage.indexOfKey(nodeId);
    if( index_node < 0 ) {
        MY_LOGW("cannot find usage for (NodeId %#" PRIxPTR ", streamId %#" PRIx64 ")",
                nodeId, streamId );
        return 0;
    }
    //
    StreamUsageMap const& pStreamUsgMap = mNodeImageStreamUsage.valueAt(index_node);
    ssize_t index_stream = pStreamUsgMap.indexOfKey(streamId);
    if( index_stream < 0 ) {
        MY_LOGW("cannot find usage for (NodeId %#" PRIxPTR ", streamId %#" PRIx64 ")",
                nodeId, streamId );
        return 0;
    }
    //
    return pStreamUsgMap.valueAt(index_stream);

}


/******************************************************************************
 *
 ******************************************************************************/
void
NodeConfig::
dumpState(android::Printer& printer) const
{
    struct dump
    {
        static android::String8
            StreamInfo(android::KeyedVector<StreamId_T, android::sp<IImageStreamInfo>> const& vector) {
                android::String8 os;
                for( size_t i = 0; i < vector.size(); i++ ) {
                    auto const streamId = vector.keyAt(i);
                    os += android::String8::format("%#" PRIx64 " ", streamId);
                }
                return os;
            }
        static android::String8
            StreamInfo(android::KeyedVector<StreamId_T, android::sp<IMetaStreamInfo>> const& vector) {
                android::String8 os;
                for( size_t i = 0; i < vector.size(); i++ ) {
                    auto const streamId = vector.keyAt(i);
                    os += android::String8::format("%#" PRIx64 " ", streamId);
                }
                return os;
            }
        static android::String8
            StreamInfoSetControl(android::sp<const IStreamInfoSetControl> s) {
                android::String8 os;
                os += " .AppImage={ ";
                os += dump::StreamInfo(s->getAppImage());
                os += "}";
                os += " .HalImage={ ";
                os += dump::StreamInfo(s->getHalImage());
                os += "}";
                os += " .AppMeta={ ";
                os += dump::StreamInfo(s->getAppMeta());
                os += "}";
                os += " .HalMeta={ ";
                os += dump::StreamInfo(s->getHalMeta());
                os += "}";
                return os;
            }
    };

    RWLock::AutoRLock _l(mRWLock);
    for( size_t i = 0; i < mConfig_NodeMap.size(); i++ )
    {
        android::sp<ContextNode> pNode = mConfig_NodeMap.valueAt(i);
        android::sp<INodeActor> pNodeActor = pNode->getNodeActor();
        if (CC_LIKELY( pNode != nullptr && pNodeActor != nullptr )) {
            printer.printFormatLine("<%s>", pNodeActor->toString().c_str());
            printer.printFormatLine("    In:%s", dump::StreamInfoSetControl(pNode->getInStreams()).c_str());
            printer.printFormatLine("   Out:%s", dump::StreamInfoSetControl(pNode->getOutStreams()).c_str());
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
void
PipelineConfig::
dumpState(android::Printer& printer) const
{
    android::String8 os;
    os += ".root={";
    for( size_t i = 0; i < mRootNodes.size(); i++ ) {
        os += android::String8::format(" %#" PRIxPTR " ", mRootNodes[i]);
    }
    os += "}";

    os += ", .edges={";
    for( size_t i = 0; i < mNodeEdges.size(); i++ ) {
        os += android::String8::format("(%#" PRIxPTR " -> %#" PRIxPTR ")",
            mNodeEdges[i].src, mNodeEdges[i].dst);
    }
    os += "}";
    printer.printLine(os.c_str());
}

