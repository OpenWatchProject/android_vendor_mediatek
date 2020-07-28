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
#include "PipelineContextImpl.h"
//
#include <future>
#include <vector>
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v3::NSPipelineContext;


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


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
MVOID
RequestBuilderImpl::
onRequestConstructed()
{
    // clear one-shot data
    mStreamBuffers_Image.clear();
    mStreamBuffers_HalImage.clear();
    mStreamBuffers_Meta.clear();
    mStreamBuffers_HalMeta.clear();
    //
    mFlag = FLAG_NO_CHANGE;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
RequestBuilderImpl::
dump(MUINT32 const reqNo, MUINT32 const frameNo) const
{
    MY_LOGD("dump frameNo req %d frameNo %d +", reqNo, frameNo);
    MY_LOGD("Image IOMap:");
    for( size_t i_node = 0; i_node < mImageNodeIOMaps.size(); i_node++ )
    {
        NodeId_T const nodeId = mImageNodeIOMaps.keyAt(i_node);
        IOMapSet const& mapSet = mImageNodeIOMaps.valueAt(i_node);
        for( size_t i_map = 0; i_map < mapSet.size(); i_map++ )
        {
            String8 dumpLog = NSPipelineContext::dump(mapSet[i_map]);
            MY_LOGD("nodeId %#" PRIxPTR " #%zu: %s",
                    nodeId, i_map, dumpLog.string());
        }
    }
    //
    MY_LOGD("Meta IOMap:");
    for( size_t i_node = 0; i_node < mMetaNodeIOMaps.size(); i_node++ )
    {
        NodeId_T const nodeId = mImageNodeIOMaps.keyAt(i_node);
        IOMapSet const& mapSet = mMetaNodeIOMaps.valueAt(i_node);
        for( size_t i_map = 0; i_map < mapSet.size(); i_map++ )
        {
            String8 dumpLog = NSPipelineContext::dump(mapSet[i_map]);
            MY_LOGD("nodeId %#" PRIxPTR " #%zu: %s",
                    nodeId, i_map, dumpLog.string());
        }
    }
    //
    MY_LOGD("Node edge:");
    for( size_t i = 0; i < mNodeEdges.size(); i++ )
    {
        MY_LOGD("nodeId %#" PRIxPTR " -> %#" PRIxPTR,
                mNodeEdges[i].src, mNodeEdges[i].dst);
    }
    //
    MY_LOGD_IF(mpCallback.unsafe_get(), "callback is set(%p)", mpCallback.unsafe_get());
    //
    for( size_t i = 0; i < mReplacingInfos.size(); i++ )
    {
        MY_LOGD("replacing stream %#" PRIx64,
                mReplacingInfos[i]->getStreamId());
    }
    //
#define sb_dump( sbmap, str )                  \
    for( size_t i = 0; i < sbmap.size(); i++ ) \
    {                                          \
        MY_LOGD("%s %#" PRIx64,               \
                str, sbmap.keyAt(i));          \
    }
    sb_dump(mStreamBuffers_Image,       "StreamBuffer(Image):");
    sb_dump(mStreamBuffers_HalImage,    "StreamBuffer(HalImage):");
    sb_dump(mStreamBuffers_Meta,        "StreamBuffer(Meta):");
    sb_dump(mStreamBuffers_HalMeta,     "StreamBuffer(HalMeta):");
#undef sb_dump
    MY_LOGD("dump frameNo req %d frameNo %d -", reqNo, frameNo);
}


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
    , mInFlush(MFALSE)
{}


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
    RWLock::AutoWLock _l(mRWLock);
    //
    MUINT32 const type = pBuilder->mType;
    //
    if( TypeOf(type) == eType_IMAGE )
    {
        sp<IImageStreamInfo> pStreamInfo = pBuilder->mpImageStreamInfo;
        if( ! pStreamInfo.get() ) {
            MY_LOGE("inconsistent type 0x%x", type);
            return BAD_VALUE;
        }
        StreamId_T const streamId = pStreamInfo->getStreamId();
        // 1. check if this stream is already marked as reuse
        // TODO
        // 2. add <stream, pool or provider> to context
        sp<StreamConfig::ItemImageStream> pItem = new StreamConfig::ItemImageStream(pStreamInfo, type);
        //
        if( type == eStreamType_IMG_HAL_POOL )
        {
            // create pool
            sp<HalImageStreamBufferPoolT> pPool =
                createHalStreamBufferPool(getName(), pStreamInfo);
            if( ! pPool.get() ) {
                MY_LOGE("create pool failed: %s", Log::Info(pStreamInfo).string());
                return DEAD_OBJECT;
            }
            //
            pItem->pPool = pPool;
        }
        else if ( type == eStreamType_IMG_HAL_PROVIDER )
            {
            // get SB Provider set by user
            sp<IStreamBufferProviderT> const pProvider = pBuilder->mpProvider;
            if ( ! pProvider.get() ) {
                MY_LOGE("get provider failed: %s", Log::Info(pStreamInfo).string());
                return DEAD_OBJECT;
            }
            //
            pItem->pProvider = pProvider;
        }

        // 4. add <stream, type> to context
        //MY_LOGD_IF(1, "New image stream: type 0x%x, %s", type, Log::Info(pStreamInfo).string());
        return mpStreamConfig->add(pItem);
    }
    else if( TypeOf(type) == eType_META )
    {
        sp<IMetaStreamInfo> pStreamInfo = pBuilder->mpMetaStreamInfo;
        if( ! pStreamInfo.get() ) {
            MY_LOGE("inconsistent type 0x%x", type);
            return BAD_VALUE;
        }
        //
        //MY_LOGD_IF(1, "New meta stream: type 0x%x, %s", type, Log::Info(pStreamInfo).string());
        sp<StreamConfig::ItemMetaStream> pItem = new StreamConfig::ItemMetaStream(pStreamInfo, type);
        return mpStreamConfig->add(pItem);
    }
    MY_LOGE("not supported type 0x%x", type);
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
    MY_LOGD_IF( 1, "Reuse image stream: type 0x%x, %s", pItem->type, Log::Info(pItem->pInfo).string());
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
sp<IPipelineFrame>
PipelineContext::PipelineContextImpl::
constructRequest(RequestBuilderImpl* pBuilder, MUINT32 const requestNo)
{
    // to make sure onRequestConstructed() can be called when this function
    // returns
    class scopedVar
    {
        public:
            scopedVar(RequestBuilderImpl* rpBuilder) : mpBuilder(rpBuilder) {}
            ~scopedVar() { mpBuilder->onRequestConstructed(); }
        private:
            RequestBuilderImpl* const   mpBuilder;
    } _localVar(pBuilder);
    //
    RWLock::AutoRLock _l(mRWLock);
    //
    typedef IPipelineBufferSetFrameControl          PipelineFrameT;
    //TODO: check context's status first!
    //TODO: check valid request
    //
    NodeIOMaps const& aImageIOMaps  = pBuilder->mImageNodeIOMaps;
    NodeIOMaps const& aMetaIOMaps   = pBuilder->mMetaNodeIOMaps;
    NodeEdgeSet const& aEdges       = pBuilder->mNodeEdges;
    NodeSet const& aRoots           = pBuilder->mRootNodes;
    //
    wp<AppCallbackT> const& aAppCallback
                                    = pBuilder->mpCallback;
    ImageStreamInfoMapT const& aReplacingInfos
                                    =  pBuilder->mReplacingInfos;
    //
    if( ! mpFrameNumberGenerator.get() ) { //TODO: can be removed after check context's status
        MY_LOGE("cannot get frameNo generator");
        return NULL;
    }
    //
    uint32_t const frameNo = mpFrameNumberGenerator->generateFrameNo();
    //
    if( 0 ) {
        pBuilder->dump(requestNo, frameNo);
    }
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
    sp<PipelineFrameT> pFrame = PipelineFrameT::create(
        requestNo,
        frameNo,
        aAppCallback, // IAppCallback
        mpStreamConfig.get(), //  IPipelineStreamBufferProvider
        mpDispatcher // IPipelineNodeCallback
    );
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
    // replace IImageStreamInfo:
    //      update run-time modified IStreamInfo to this request IStreamInfoSet.
    //      Then, following operations could query IStreamInfo from this if necessary.
    for( size_t i = 0; i < aReplacingInfos.size(); i++ )
    {
        sp<IImageStreamInfo> pInfo = aReplacingInfos.valueAt(i);
        ssize_t idx = aRequestData.vHalImageStreamInfo.indexOfKey(pInfo->getStreamId());
        if( idx < 0 )
    {
            MY_LOGE("cannot replace IImageStreamInfo for stream %#" PRIx64,
                    pInfo->getStreamId());
            return NULL;
                }
        MY_LOGD_IF(1, "replace stream %#" PRIx64, pInfo->getStreamId() );
        aRequestData.vHalImageStreamInfo.replaceValueAt(idx, pInfo);
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
                aRequestData.vAppMetaStreamInfo, pBuilder->mStreamBuffers_Meta,
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
                aRequestData.vAppImageStreamInfo, pBuilder->mStreamBuffers_Image,
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
MERROR
PipelineContext::PipelineContextImpl::
config(
    PipelineContextImpl* pOldContext,
    MBOOL const isAsync
)
{
    RWLock::AutoWLock _l(mRWLock);
    //
    MERROR err;
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
        .pStreamConfig   = mpStreamConfig.get(),
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
        std::vector< std::future<MERROR> > vFutures;
        //
        Vector<IPipelineDAG::NodeObj_T> const& rToposort = mpPipelineDAG->getToposort();
        Vector<IPipelineDAG::NodeObj_T>::const_iterator it = rToposort.begin();
        for (; it != rToposort.end(); it++)
        {
            sp<ContextNode> pContextNode = mpNodeConfig->queryNode(it->id);
            sp<INodeActor> pIActor = pContextNode.get() ? pContextNode->getNodeActor() : NULL;
            if( ! pIActor.get() )
            {
                MY_LOGE("cannnot find node %#" PRIxPTR " from Node Config", it->id);
                return UNKNOWN_ERROR;
            }
            //
            struct job
            {
                static
                    MERROR      execute(sp<INodeActor> pIActor) {
                                    MERROR err;
                                    if( OK != (err = pIActor->init() ) )
                                        return err;
                                    err = pIActor->config();
                                    return err;
                                }
            };
            //
            vFutures.push_back(
                    std::async(isAsync ? std::launch::async : std::launch::deferred,
                        &job::execute, pIActor)
                    );
        }
        //
        for( auto &fut : vFutures ) {
            MERROR result = fut.get();
            if( result != OK ) {
                err = result;
            }
        }
        //
        sp<IPipelineNodeMap> pPipelineNodeMap = mpPipelineNodeMap;
        sp<INodeCallbackToPipeline> pCallback = this;
        it = rToposort.begin();
        for (; it != rToposort.end(); it++)
        {
            sp<IPipelineNode> pNode = pPipelineNodeMap->nodeAt(it->val);
            if  ( pNode == 0 ) {
                MY_LOGE("NULL node (id:%" PRIxPTR ")", it->id);
                continue;
            }
            //
            if (pNode->setNodeCallBack(pCallback) != OK)
            {
                MY_LOGE("Fail to setcallback to node (id:%" PRIxPTR ")", it->id);
            }
        }
    }
    //
    // TODO: should keep error status to avoid requesting.
    //
    return err;
};

/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineContext::PipelineContextImpl::
queue(
    sp<IPipelineFrame> const& pFrame
)
{
    RWLock::AutoRLock _l(mRWLock);
    if( ! mpInFlightRequest.get() || ! mpDispatcher.unsafe_get() ) { //TODO, may remove this check
        MY_LOGE("not configured yet!");
        return UNKNOWN_ERROR;
    }
    MY_LOGD_IF( 0, "regist inflight request(%p) %d", this, pFrame->getFrameNo());
    mpInFlightRequest->registerRequest(pFrame);
    //
    sp<IPipelineNodeMap const> pPipelineNodeMap = pFrame->getPipelineNodeMap();
    if  ( pPipelineNodeMap == 0 || pPipelineNodeMap->isEmpty() ) {
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
    {
        RWLock::AutoRLock _l(mFlushLock);
        for(size_t i=0; i<RootNodeObjSet.size();i++)
        {
            sp<IPipelineNode> pNode = pPipelineNodeMap->nodeAt(RootNodeObjSet[i].val);
            if  ( pNode == 0 ) {
                MY_LOGE("[frameNo:%d] Bad root node", pFrame->getFrameNo());
                return DEAD_OBJECT;
            }
            if( mInFlush )
                err = pNode->flush(pFrame);
            else
            {
                // check enque need pipeline blocking or not
                err = pNode->queue(pFrame);
                if(err == FAILED_TRANSACTION)
                {
                    vspPipelineNode.push_back(pNode);
                    MY_LOGD("[frameNo:%d] enque to root node: %" PRIxPTR " need blocking",
                        pFrame->getFrameNo(),pNode->getNodeId());
                }
                else if(err != OK)
                {
                    MY_LOGE("[frameNo:%d] enque to root node: %" PRIxPTR " fail(%d)",
                        pFrame->getFrameNo(),pNode->getNodeId(),err);
                    return err;
                }
            }
        }
    }

    while(vspPipelineNode.size() != 0)
    {
        {
            // blocking and wait event to enque (wait 33ms and check again)
            Mutex::Autolock lock(mEnqueLock);
            mCondEnque.waitRelative(mEnqueLock,33000000);
        }

        {
            // wake up and enque to root node which can not be enqued last time
            RWLock::AutoRLock _l(mFlushLock);
            Vector<sp<IPipelineNode>>::iterator it = vspPipelineNode.begin();
            while(it != vspPipelineNode.end())
            {
                if( mInFlush )
                    err = (*it)->flush(pFrame);
                else
                    err = (*it)->queue(pFrame);
                if(err == FAILED_TRANSACTION)
                {
                    MY_LOGD("[frameNo:%d] enque to root node: %" PRIxPTR " need blocking",
                        pFrame->getFrameNo(),(*it)->getNodeId());
                    it++;
                }
                else if(err != OK)
                {
                    MY_LOGE("[frameNo:%d] enque to root node: %" PRIxPTR " fail(%d)",
                        pFrame->getFrameNo(),(*it)->getNodeId(),err);
                    return err;
                }
                else
                {
                    MY_LOGD("[frameNo:%d] enque success, erase root node record: id = %d" PRIxPTR " ",
                        pFrame->getFrameNo(),(*it)->getNodeId());
                    it = vspPipelineNode.erase(it);
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
kick(
    sp<IPipelineFrame> const& pFrame
)
{
    RWLock::AutoRLock _l(mRWLock);
    sp<IPipelineNodeMap const> pPipelineNodeMap = pFrame->getPipelineNodeMap();
    if  ( pPipelineNodeMap == 0 || pPipelineNodeMap->isEmpty() ) {
        MY_LOGE("[frameNo:%d] Bad PipelineNodeMap:%p", pFrame->getFrameNo(), pPipelineNodeMap.get());
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
            if( mInFlush ) { // do-nothing
                MY_LOGD("[frameNo:%d] kick but flushing", pFrame->getFrameNo());
            }
            else
            {
                for(size_t i=0;i<RootNodeObjSet.size();i++)
                {
                    sp<IPipelineNode> pNode = pPipelineNodeMap->nodeAt(RootNodeObjSet[i].val);
                    if  ( pNode == 0 ) {
                        MY_LOGE("[frameNo:%d] Bad root node", pFrame->getFrameNo());
                        return DEAD_OBJECT;
                    }

                    MY_LOGD("[frameNo:%d] kick begin", pFrame->getFrameNo());
                    err = pNode->kick();
                    MY_LOGD("[frameNo:%d] kick end", pFrame->getFrameNo());
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
MERROR
PipelineContext::PipelineContextImpl::
waitUntilNodeDrained(NodeId_T const nodeId)
{
    RWLock::AutoRLock _l(mRWLock);
    if( mpInFlightRequest.get() )
        mpInFlightRequest->waitUntilNodeDrained(nodeId);
    else
        MY_LOGD("may not configured yet");
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineContext::PipelineContextImpl::
beginFlush()
{
    FUNC_START;
    {
        Mutex::Autolock _l(mLastFrameLock);
        sp<IPipelineFrame> pLastFrame = mpLastFrame.promote();
        if (pLastFrame.get() != NULL) {
            kick(pLastFrame);
        } else {
            MY_LOGW("cannot promote LastFrame");
        }
    }
    //
    {
        RWLock::AutoWLock _l(mFlushLock);
        mInFlush = MTRUE;
    }
    //
    {
        sp<IDispatcher> pDispatcher = mpDispatcher.promote();
        if( pDispatcher.get() )
            pDispatcher->beginFlush();
        else
            MY_LOGW("cannot promote dispatcher");
    }
    //
    {
    RWLock::AutoRLock _l(mRWLock);
    //
    sp<IPipelineNodeMap> pPipelineNodeMap = mpPipelineNodeMap;
    Vector<IPipelineDAG::NodeObj_T> const& rToposort = mpPipelineDAG->getToposort();
    Vector<IPipelineDAG::NodeObj_T>::const_iterator it = rToposort.begin();
    for (; it != rToposort.end(); it++)
    {
        sp<IPipelineNode> pNode = pPipelineNodeMap->nodeAt(it->val);
        if  ( pNode == 0 ) {
            MY_LOGE("NULL node (id:%" PRIxPTR ")", it->id);
            continue;
        }
        //
        if (pNode->flush() != OK)
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
        mInFlush = MFALSE;
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
setScenarioControl(
    sp<IScenarioControl> pControl
)
{
    if( mpScenarioControl.get() )
    {
        MY_LOGW("mpScenarioControl already existed");
    }
    mpScenarioControl = pControl;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineContext::PipelineContextImpl::
setDispatcher(
    wp<IDispatcher> pDispatcher
)
{
    MY_LOGD("set dispatcher %p", pDispatcher.unsafe_get());
    mpDispatcher = pDispatcher;
    sp<IDispatcher> spDispatcher = mpDispatcher.promote();
    if (spDispatcher.get())
    {
        spDispatcher->setDataCallback(mpDataCallback);
    }
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
sp<HalImageStreamBufferPoolT>
PipelineContext::PipelineContextImpl::
queryImageStreamPool(
    StreamId_T const streamId
) const
{
    RWLock::AutoRLock _l(mRWLock);
    return mpStreamConfig->queryImage(streamId)->pPool;
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
uint32_t
PipelineContext::PipelineContextImpl::
getFrameNo()
{
    RWLock::AutoRLock _l(mRWLock);
    //
    uint32_t frameNo = 0;
    if( ! mpFrameNumberGenerator.get() ) {
        MY_LOGE("cannot get frameNo generator");
    }
    else {
       frameNo =  mpFrameNumberGenerator->getFrameNo();
       MY_LOGD("frameNo:%d", frameNo);
    }
    //
    return frameNo;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
PipelineContext::PipelineContextImpl::
dump() const
{
    RWLock::AutoRLock _l(mRWLock);
    //
    MY_LOGD("dump pipeline(%s) begin", getName());
    //
    mpStreamConfig->dump();
    mpNodeConfig->dump();
    mpPipelineConfig->dump();
    //
    MY_LOGD("dump pipeline(%s) end", getName());
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
    }
    return;
}


MERROR
config_pipeline::
operator()(Params& rParams)
{
    MERROR err = OK;
    //
    StreamConfig const* pStreamConfig     = rParams.pStreamConfig;
    NodeConfig const* pNodeConfig         = rParams.pNodeConfig;
    PipelineConfig const* pPipelineConfig = rParams.pPipelineConfig;
    IPipelineDAG* pDAG                    = rParams.pDAG;
    IPipelineNodeMapControl* pNodeMap     = rParams.pNodeMap;
    //
    if( !pStreamConfig || !pNodeConfig || !pPipelineConfig || !pDAG || !pNodeMap ) {
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
sp<HalImageStreamBufferPoolT>
NSPipelineContext::
createHalStreamBufferPool(
    const char* username,
    sp<IImageStreamInfo> pStreamInfo
)
{
    String8 const name = String8::format("%s:%s", username, pStreamInfo->getStreamName());
    //
    IImageStreamInfo::BufPlanes_t const& bufPlanes = pStreamInfo->getBufPlanes();
    size_t bufStridesInBytes[3] = {0};
    size_t bufBoundaryInBytes[3]= {0};
    size_t bufCustomSizeInBytes[3] = {0};
    size_t bufReusableSizeInBytes[3] = {0};
    for (size_t i = 0; i < bufPlanes.size(); i++) {
        bufStridesInBytes[i] = bufPlanes[i].rowStrideInBytes;
        bufCustomSizeInBytes[i] = bufPlanes[i].sizeInBytes;
        bufReusableSizeInBytes[i] = bufPlanes[i].reusableSizeInBytes;
    }
    sp<HalImageStreamBufferPoolT>         pPool;

    if(pStreamInfo->getImgFormat()  == eImgFmt_BLOB)
    {
         IImageBufferAllocator::ImgParam const imgParam(
                bufStridesInBytes[0], bufBoundaryInBytes[0]);

         pPool = new HalImageStreamBufferPoolT(
             name.string(),
             HalImageStreamBufferAllocatorT(pStreamInfo.get(), imgParam)
         );

    }
    else {
    IImageBufferAllocator::ImgParam const imgParam(
        pStreamInfo->getImgFormat(),
        pStreamInfo->getImgSize(),
        bufStridesInBytes, bufBoundaryInBytes,
        bufCustomSizeInBytes, bufReusableSizeInBytes,
        bufPlanes.size()
    );

    pPool = new HalImageStreamBufferPoolT(
        name.string(),
        HalImageStreamBufferAllocatorT(pStreamInfo.get(), imgParam)
    );

    }

    if  ( pPool == 0 ) {
        MY_LOGE("Fail to new a image pool:%s", name.string());
        return NULL;
    }
    //
    MERROR err = pPool->initPool(pStreamInfo->getStreamName(), pStreamInfo->getMaxBufNum(), pStreamInfo->getMinInitBufNum());
    if  ( OK != err ) {
        MY_LOGE("%s: initPool err:%d(%s)", name.string(), err, ::strerror(-err));
        return NULL;
    }
    if  ( OK != pPool->commitPool(username) ) {
        MY_LOGE("%s: commitPool err:%d(%s)", name.string(), err, ::strerror(-err));
        return NULL;
    }
    //
    return pPool;
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
    StreamConfig const*     pStreamConfig = rParams.pStreamConfig;
    IStreamInfoSetControl*  pSetControl   = rParams.pSetControl;
    //
    for( size_t i = 0; i < pStreamSet->size(); i++ )
    {
        StreamId_T const streamId = pStreamSet->itemAt(i);

#define search_then_add(_type_)                                                              \
        {                                                                                    \
            sp<StreamConfig::Item##_type_##Stream> item                    \
                = pStreamConfig->query##_type_(streamId);                  \
            if( item.get() )                                                                 \
            {                                                                                \
                if( BehaviorOf(item->type) == eBehavior_HAL ) {                              \
                    pSetControl->editHal##_type_().addStream(item->pInfo);                   \
                } else {                                                                     \
                    pSetControl->editApp##_type_().addStream(item->pInfo);                   \
                }                                                                            \
                continue;                                                                    \
            }                                                                                \
        }

        // search from configured images, then from configured meta
        search_then_add(Image);
        search_then_add(Meta);
#undef search_then_add
        //
        MY_LOGE("cannot find stream(%#" PRIx64 ") from configuration", streamId);
        MY_LOGW("=== dump configuration begin ===");
        pStreamConfig->dump();
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
    struct impl
    {
        #define     impl_query(_type_)                                                 \
        MERROR              query(                                                     \
                                StreamConfig const* rpStreamConfig,                    \
                                StreamSet const* pStreams,                             \
                                _type_##StreamInfoMapT* pvAppInfos,                    \
                                _type_##StreamInfoMapT* pvHalInfos                     \
                            )                                                          \
        {                                                                              \
            if( rpStreamConfig == NULL ) return UNKNOWN_ERROR;                         \
            for( size_t i = 0; i < pStreams->size(); i++ )                             \
            {                                                                          \
                StreamId_T const streamId = pStreams->itemAt(i);                       \
                sp<StreamConfig::Item##_type_##Stream> pItem                           \
                    = rpStreamConfig->query##_type_(streamId);                         \
                if( ! pItem.get() ) {                                                  \
                    MY_LOGE("cannot find %s stream %#" PRIx64 "", #_type_, streamId); \
                    return BAD_VALUE;                                                  \
                }                                                                      \
                if( BehaviorOf(pItem->type) == eBehavior_APP )                         \
            {                                                                         \
                    pvAppInfos->add(streamId, pItem->pInfo);                           \
                } else                                                                \
                if( BehaviorOf(pItem->type) == eBehavior_HAL )                         \
                {                                                                     \
                    pvHalInfos->add(streamId, pItem->pInfo);                           \
                } else                                                                \
                {                                                                     \
                    MY_LOGE("should not happen");                                     \
                    return UNKNOWN_ERROR;                                             \
                }                                                                     \
            }                                                                         \
            return OK;                                                                \
        }
        //
        impl_query(Image)
        impl_query(Meta)
#undef impl_query
    };
    //
    MERROR err = OK;
    err = impl().query(
            rParams.pStreamConfig, rParams.pvImageStreamSet,
                    rParams.pvAppImageStreamInfo, rParams.pvHalImageStreamInfo
                );
    if( err != OK ) return err;
    //
    err = impl().query(
            rParams.pStreamConfig, rParams.pvMetaStreamSet,
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
                String8 str = Log::Info(pStreamInfoMap->valueAt(i)); \
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
        sp<SBufferT> SBuffer = rvSBuffers.valueFor(streamId);
        if( ! SBuffer.get() ) { // allocate here
            sp<IMetaStreamInfo> pStreamInfo = rvStreamInfo.valueAt(i);
            SBuffer = HalMetaStreamBufferAllocatorT(pStreamInfo.get())();
        }
        //
        vStreamBuffers.push_back(SBuffer);
    }
    //
    sp<PipelineFrameT::IMap<SBufferT> > pBufMap = pFrame->editMap_AppMeta();
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
        HalMetaStreamBufferMapsT const& rvSBuffers,
        PipelineFrameT* pFrame
        ) const
{
    typedef HalMetaStreamBuffer SBufferT;
    //
    Vector< sp<SBufferT> > vStreamBuffers;
    for( size_t i = 0; i < rvStreamInfo.size(); i++ )
    {
        StreamId_T const streamId = rvStreamInfo.keyAt(i);
        sp<SBufferT> SBuffer = rvSBuffers.valueFor(streamId);
        if( ! SBuffer.get() ) { // allocate here
            sp<IMetaStreamInfo> pStreamInfo = rvStreamInfo.valueAt(i);
            SBuffer = HalMetaStreamBufferAllocatorT(pStreamInfo.get())();
        }
        vStreamBuffers.push_back(SBuffer);
    }
    //
    sp<PipelineFrameT::IMap<SBufferT> > pBufMap = pFrame->editMap_HalMeta();
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
        ImageStreamBufferMapsT const& rvSBuffers,
        PipelineFrameT* pFrame
        ) const
{
    if( rvStreamInfo.size() != rvSBuffers.size() ) {
        MY_LOGE("collect rvStreamInfo size %zu != SB size %zu",
                rvStreamInfo.size(), rvSBuffers.size() );
        return BAD_VALUE;
    }
    //
    sp<PipelineFrameT::IMap<IImageStreamBuffer> > pBufMap = pFrame->editMap_AppImage();
    //
    pBufMap->setCapacity( rvSBuffers.size() );
    for( size_t i = 0; i < rvSBuffers.size(); i++ )
    {
        MY_LOGD_IF(FRAMEE_STREAMBUFFER_DEBUG_ENABLE,
                "stream %#" PRIx64,
                rvSBuffers[i]->getStreamInfo()->getStreamId());
        pBufMap->add(rvSBuffers[i]);
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
    HalImageStreamBufferMapsT const& vSBuffers,
    PipelineFrameT* pFrame
    ) const
{
    sp<PipelineFrameT::IMap<HalImageStreamBuffer> > pBufMap = pFrame->editMap_HalImage();
    //
    pBufMap->setCapacity( rvStreamInfo.size() );
    for( size_t i = 0; i < rvStreamInfo.size(); i++ )
    {
        MY_LOGD_IF(FRAMEE_STREAMBUFFER_DEBUG_ENABLE,
                "stream %#" PRIx64,
                rvStreamInfo.valueAt(i)->getStreamId());
        sp<HalImageStreamBuffer> pBuffer =
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
                userGraph->addUser(makeUser(pBufMap->streamInfoAt(i), user->id));
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
MERROR
StreamConfig::
acquireHalStreamBuffer(
    MUINT32 const requestNo,
    sp<IImageStreamInfo> const pStreamInfo,
    sp<HalImageStreamBuffer>& rpStreamBuffer
)   const
{
    MERROR err = UNKNOWN_ERROR;
    StreamId_T const streamId = pStreamInfo->getStreamId();
    sp<ItemImageStream> item = queryImage(streamId);
    switch ( HalBehaviorOf(item->type) )
    {
        case eBehavior_HAL_POOL:
            {
                sp<HalImageStreamBufferPoolT> pPool = item->pPool;
                //
                MY_LOGE_IF(pPool == 0, "NULL HalImageStreamBufferPool - stream:%#" PRIx64 "(%s)",
                        streamId, pStreamInfo->getStreamName());
                //
                err = pPool == 0 ?
                    UNKNOWN_ERROR : pPool->acquireFromPool(pPool->poolName(), rpStreamBuffer, ::s2ns(10));
                MY_LOGE_IF( OK!=err || rpStreamBuffer==0,
                        "[acquireFromPool] err:%d(%s) pStreamBuffer:%p stream:%#" PRIx64 "(%s)",
                        err, ::strerror(-err), rpStreamBuffer.get(),
                        streamId, pStreamInfo->getStreamName()
                        );
            }
            //MY_LOGD_IF(err == OK, "stream %#" PRIx64 " buffer %p from pool",
            //        streamId, rpStreamBuffer.get());
            break;
        case eBehavior_HAL_PROVIDER:
            {
                sp<IStreamBufferProviderT> pProvider = item->pProvider;
                //
                MY_LOGE_IF(pProvider == 0, "NULL HalImageStreamBufferProvider - stream:%#" PRIx64 "(%s)",
                        streamId, pStreamInfo->getStreamName());
                //
                sp<HalImageStreamBuffer> pStreamBuffer;
                err = pProvider == 0 ?
                    UNKNOWN_ERROR : pProvider->dequeStreamBuffer(requestNo, pStreamInfo, pStreamBuffer);
                rpStreamBuffer = pStreamBuffer;
                //
                MY_LOGW_IF( 0 /*OK!=err || rpStreamBuffer==0*/,
                        "[acquireFromProvider] err:%d(%s) pStreamBuffer:%p stream:%#" PRIx64 "(%s)",
                        err, ::strerror(-err), rpStreamBuffer.get(),
                        streamId, pStreamInfo->getStreamName()
                        );
            }
            break;
        case eBehavior_HAL_RUNTIME:
            {
                String8 const str = String8::format(
                        "%s StreamId:%#" PRIx64 " %dx%d %p %p",
                        pStreamInfo->getStreamName(),
                        pStreamInfo->getStreamId(),
                        pStreamInfo->getImgSize().w,
                        pStreamInfo->getImgSize().h,
                        pStreamInfo.get(),
                        item->pInfo.get()
                        );
                IImageStreamInfo::BufPlanes_t const& bufPlanes = pStreamInfo->getBufPlanes();
                size_t bufStridesInBytes[3] = {0};
                size_t bufBoundaryInBytes[3]= {0};
                for (size_t i = 0; i < bufPlanes.size(); i++) {
                    bufStridesInBytes[i] = bufPlanes[i].rowStrideInBytes;
                }
                IImageBufferAllocator::ImgParam const imgParam(
                        pStreamInfo->getImgFormat(),
                        pStreamInfo->getImgSize(),
                        bufStridesInBytes, bufBoundaryInBytes,
                        bufPlanes.size()
                        );
                //
                rpStreamBuffer = HalImageStreamBufferAllocatorT(pStreamInfo.get(), imgParam)();
                err = rpStreamBuffer.get() ? OK : UNKNOWN_ERROR;
                if  ( err != OK ) {
                    MY_LOGE("Fail to allocate - %s", str.string());
                }
            }
            break;
        default:
            MY_LOGW("not supported type 0x%x stream:%#" PRIx64 "(%s)",
                    item->type, streamId, pStreamInfo->getStreamName());
    };

    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
StreamConfig::
dump() const
{
    RWLock::AutoRLock _l(mRWLock);
    //
    MY_LOGD("configured streams +");
    for( size_t i = 0; i < mStreamMap_Image.size(); i++ )
    {
        sp<ItemImageStream> pItem = mStreamMap_Image.valueAt(i);
        MY_LOGD("image: type 0x%x, %s", pItem->type, Log::Info(pItem->pInfo).string());
    }
    for( size_t i = 0; i < mStreamMap_Meta.size(); i++ )
    {
        sp<ItemMetaStream> pItem = mStreamMap_Meta.valueAt(i);
        MY_LOGD("meta: type 0x%x, %s", pItem->type, Log::Info(pItem->pInfo).string());
    }
    MY_LOGD("configured streams -");
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
MVOID
NodeConfig::
dump()
{
    RWLock::AutoRLock _l(mRWLock);
    MY_LOGD("configured node +");
    for( size_t i = 0; i < mConfig_NodeMap.size(); i++ )
    {
        sp<ContextNode> pNode = mConfig_NodeMap.valueAt(i);
        sp<const IStreamInfoSetControl> const pInStream = pNode->getInStreams();
        sp<const IStreamInfoSetControl> const pOutStream = pNode->getOutStreams();
        MY_LOGD("<nodeId %#" PRIxPTR ">", pNode->getNodeId() );
        //
        struct dump
        {
            static String8
                StreamInfo(KeyedVector<StreamId_T, sp<IImageStreamInfo> > const& vector) {
                    String8 ret;
                    for( size_t i = 0; i < vector.size(); i++ ) {
                        sp<IStreamInfo> pStreamInfo = vector.valueAt(i);
                        ret += String8::format("%#" PRIx64 ",", pStreamInfo->getStreamId() );
                    }
                    return ret;
                }
            static String8
                StreamInfo(KeyedVector<StreamId_T, sp<IMetaStreamInfo> > const& vector) {
                    String8 ret;
                    for( size_t i = 0; i < vector.size(); i++ ) {
                        sp<IStreamInfo> pStreamInfo = vector.valueAt(i);
                        ret += String8::format("%#" PRIx64 ",", pStreamInfo->getStreamId() );
                    }
                    return ret;
                }
        };
        //
        MY_LOGD("\tIn: AppImage: %s, HalImage: %s, AppMeta: %s, HalMeta: %s" ,dump::StreamInfo(pInStream->getAppImage()).string()
                                                                             ,dump::StreamInfo(pInStream->getHalImage()).string()
                                                                             ,dump::StreamInfo(pInStream->getAppMeta()).string()
                                                                             ,dump::StreamInfo(pInStream->getHalMeta()).string());
        MY_LOGD("\tOut: AppImage: %s, HalImage: %s, AppMeta: %s, HalMeta: %s",dump::StreamInfo(pOutStream->getAppImage()).string()
                                                                             ,dump::StreamInfo(pOutStream->getHalImage()).string()
                                                                             ,dump::StreamInfo(pOutStream->getAppMeta()).string()
                                                                             ,dump::StreamInfo(pOutStream->getHalMeta()).string());
    }
    MY_LOGD("configured node -");
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
PipelineConfig::
dump() const
{
    MY_LOGD("configured pipeline +");
    // dump edges & roots
    String8 ret("");
    for( size_t i = 0; i < mRootNodes.size(); i++ ) {
        ret += String8::format("root: %#" PRIxPTR, mRootNodes[i]);
    }
    //
    for( size_t i = 0; i < mNodeEdges.size(); i++ ) {
        ret += String8::format(", edge: %#" PRIxPTR " -> %#" PRIxPTR,
                                mNodeEdges[i].src, mNodeEdges[i].dst
                              );
    }
    MY_LOGD("%s", ret.string());

    MY_LOGD("configured pipeline -");
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
DefaultDispatcher::
onDispatchFrame(
    android::sp<IPipelineFrame> const& pFrame,
    Pipeline_NodeId_T nodeId
)
{
    sp<IPipelineNodeMap const> pPipelineNodeMap = pFrame->getPipelineNodeMap();
    if  ( pPipelineNodeMap == NULL || pPipelineNodeMap->isEmpty() ) {
        MY_LOGE("[frameNo:%d] Bad PipelineNodeMap:%p", pFrame->getFrameNo(), pPipelineNodeMap.get());
        return;
    }
    //
    IPipelineDAG::NodeObjSet_T nextNodes;
    MERROR err = pFrame->getPipelineDAG().getOutAdjacentNodes(nodeId, nextNodes);
    if  ( ! err && ! nextNodes.empty() )
    {
        for (size_t i = 0; i < nextNodes.size(); i++)
        {
            sp<IPipelineNode> pNextNode = pPipelineNodeMap->nodeAt(nextNodes[i].val);
            if  ( pNextNode != NULL )
            {
                RWLock::AutoRLock _l(mFlushLock);
                Pipeline_NodeId_T nextNode_id = pNextNode->getNodeId();
                MUINT32 enqueue_count;
                IPipelineDAG::NodeObjSet_T InAdjacentNodes;

                // get Input Adjacent nodes of next node
                err = pFrame->getPipelineDAG().getInAdjacentNodes(nextNode_id, InAdjacentNodes);
                if(err)
                {
                    MY_LOGE("Get InAdjacentNodes of next node error (%d)", err);
                    return;
                }
                // get In request counter of next node
                err = pFrame->getPipelineDAGSp()->addInAdjacentNodesReqCnt(nextNode_id);
                if(err)
                {
                    MY_LOGE("Get InAdjacentNodes of next node error (%d)", err);
                    return;
                }
                err = pFrame->getPipelineDAG().getInAdjacentNodesReqCnt(nextNode_id, enqueue_count);
                if(err)
                {
                    MY_LOGE("Get InAdjacentNodes of next node error (%d)", err);
                    return;
                }

                // after next node receive all of requests, enqueue to next node
                if((size_t)enqueue_count >= InAdjacentNodes.size())
                {
                    MY_LOGD("[frameNo:%d] node: %#" PRIxPTR "-> %#" PRIxPTR ", enqueue to next node (%d/%d)",
                        pFrame->getFrameNo(), nodeId, nextNode_id, enqueue_count, InAdjacentNodes.size());
                    if (mInFlush == MTRUE)
                        pNextNode->flush(pFrame);
                    else
                        pNextNode->queue(pFrame);
                }
                else
                {
                    MY_LOGD("[frameNo:%d] node: %#" PRIxPTR "-> %#" PRIxPTR ", not enqueue to next node yet (%d/%d)",
                            pFrame->getFrameNo(), nodeId, nextNode_id, enqueue_count, InAdjacentNodes.size());
                }
            }
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
String8
NSPipelineContext::
dump(IOMap const& rIomap)
{
    struct dumpStreamSet
    {
        MVOID   operator() (const char* str, StreamSet const& set, String8& log)
        {
            for( size_t i = 0; i < set.size(); i++ ) {
                if( i == 0 ) log += String8::format("%s: stream ", str);
                log += String8::format("(%#" PRIx64 ")", set[i]);
            }
        }
    };
    String8 ret;
    dumpStreamSet()("In", rIomap.vIn, ret);
    dumpStreamSet()("Out", rIomap.vOut, ret);
    //
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
