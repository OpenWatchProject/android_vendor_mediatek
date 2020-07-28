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

#ifndef _MTK_HARDWARE_MTKCAM_V3_PIPELINE_PIPELINECONTEXTIMPL_H_
#define _MTK_HARDWARE_MTKCAM_V3_PIPELINE_PIPELINECONTEXTIMPL_H_
//
#include <mtkcam3/pipeline/pipeline/IPipelineContext.h>
#include "PipelineContext.h"

#include <atomic>
#include <future>

#include "MyUtils.h"
#include "IPipelineNodeMapControl.h"
#include "InFlightRequest.h"
#include "RequestBuilderImpl.h"
#include "IPipelineFrameNumberGenerator.h"

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam::v3::pipeline::NSPipelineContext {
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
/******************************************************************************
 *  Definitions.
 ******************************************************************************/
typedef NSCam::v3::                 StreamId_T
                                    StreamId_T;
typedef NSCam::v3::Utils::          HalImageStreamBuffer
                                    HalImageStreamBuffer;
typedef NSCam::v3::Utils::          HalMetaStreamBuffer
                                    HalMetaStreamBuffer;
typedef NSCam::v3::Utils::          IStreamInfoSetControl
                                    IStreamInfoSetControl;
typedef HalImageStreamBuffer::      Allocator
                                    HalImageStreamBufferAllocatorT;
typedef HalImageStreamBufferAllocatorT::StreamBufferPoolT
                                    HalImageStreamBufferPoolT;
typedef HalMetaStreamBuffer::       Allocator
                                    HalMetaStreamBufferAllocatorT;
//
typedef DefaultKeyedVector<StreamId_T, MUINT>
                                    StreamUsageMap;
typedef DefaultKeyedVector<NodeId_T, StreamUsageMap>
                                    NodeStreamUsageMaps;
//
typedef DefaultKeyedVector<NodeId_T, IOMapSet>
                                    NodeIOMaps;
typedef IPipelineFrame::InfoIOMapSet
                                    InfoIOMapSet;
typedef IPipelineFrameNodeMapControl
                                    FrameNodeMapT; //FIXME, remove this!

using AppImageStreamBufferMapsT = FrameBuilderImpl::AppImageStreamBufferMapsT;
typedef DefaultKeyedVector<StreamId_T, sp<IImageStreamBuffer> >
                                    ImageStreamBufferMapsT;
typedef DefaultKeyedVector<StreamId_T, sp<HalImageStreamBuffer> >
                                    HalImageStreamBufferMapsT;
typedef DefaultKeyedVector<StreamId_T, sp<IMetaStreamBuffer> >
                                    MetaStreamBufferMapsT;
typedef DefaultKeyedVector<StreamId_T, sp<HalMetaStreamBuffer> >
                                    HalMetaStreamBufferMapsT;
typedef IPipelineBufferSetFrameControl::IAppCallback
                                    AppCallbackT;
typedef DefaultKeyedVector<StreamId_T, sp<IImageStreamInfo> >
                                    ImageStreamInfoMapT;
typedef DefaultKeyedVector<StreamId_T, sp<IMetaStreamInfo> >
                                    MetaStreamInfoMapT;
typedef DefaultKeyedVector<StreamId_T, MUINT32 >
                                    StreamTypeMapT;


/******************************************************************************
 *
 ******************************************************************************/
class ContextNode
    : public virtual android::VirtualLightRefBase
{
public:
                                            ContextNode(
                                                NodeId_T const nodeId,
                                                android::sp<INodeActor> pNode
                                            )
                                                : mNodeId(nodeId)
                                                , mpNode(pNode)
                                            {}
    virtual                                 ~ContextNode() = default;

public:
    NodeId_T                                getNodeId() const { return mNodeId; }
    android::sp<IPipelineNode>              getNode() const { return mpNode->getNode(); }

public:
    MVOID                                   setInStreams(android::sp<IStreamInfoSetControl> pStreams) {
                                                mpInStreams = pStreams;
                                            }
    MVOID                                   setOutStreams(android::sp<IStreamInfoSetControl> pStreams) {
                                                mpOutStreams = pStreams;
                                            };
    android::sp<INodeActor>                 getNodeActor() const { return mpNode; }
    android::sp<const IStreamInfoSetControl> const
                                            getInStreams() { return mpInStreams; }
    android::sp<const IStreamInfoSetControl> const
                                            getOutStreams() { return mpOutStreams; }

protected:
    NodeId_T const                          mNodeId;
    android::sp<INodeActor> const           mpNode;
    //
    android::sp<IStreamInfoSetControl>      mpInStreams;
    android::sp<IStreamInfoSetControl>      mpOutStreams;
};
typedef DefaultKeyedVector<NodeId_T, sp<ContextNode> >  ContextNodeMapT;


/******************************************************************************
 *
 ******************************************************************************/
class NodeBuilderImpl
    : public android::VirtualLightRefBase
{
public:
                                                NodeBuilderImpl(
                                                    NodeId_T const nodeId,
                                                    android::sp<INodeActor> pNode
                                                )
                                                    : mpContextNode( new ContextNode(nodeId, pNode) )
                                                {}
public:
    sp<ContextNode>                             mpContextNode;
    //
    StreamSet                                   mInStreamSet;
    StreamSet                                   mOutStreamSet;
    StreamUsageMap                              mUsageMap;
};


/******************************************************************************
 *
 ******************************************************************************/
class StreamBuilderImpl
    : public android::VirtualLightRefBase
{
public:
    using AttributeT = StreamBuilder::AttributeT;
    using IImageStreamBufferProviderT = StreamBuilder::IImageStreamBufferProviderT;

    AttributeT                                  mAttribute;
    sp<IImageStreamInfo>                        mpImageStreamInfo;
    sp<IImageStreamInfo>                        mpSharedImageStreamInfo = nullptr;
    sp<IMetaStreamInfo>                         mpMetaStreamInfo;
    IImageStreamBufferProviderT                 mpProvider = nullptr;
};


/******************************************************************************
 *
 ******************************************************************************/
class PipelineBuilderImpl
    : public android::VirtualLightRefBase
{
public:
    NodeSet                                     mRootNodes;
    NodeEdgeSet                                 mNodeEdges;
};


/******************************************************************************
 *
 ******************************************************************************/
class StreamConfig
    : public android::VirtualLightRefBase
{
public:
    //
    struct ItemImageStream : public VirtualLightRefBase
    {
        sp<IImageStreamInfo>            pInfo;
        MUINT32                         type;
                                        ItemImageStream(
                                                sp<IImageStreamInfo> rpInfo,
                                                MUINT32 const rType
                                        ) : pInfo(rpInfo), type(rType)
                                        {}
    };

    struct ItemMetaStream : public VirtualLightRefBase
    {
        sp<IMetaStreamInfo>             pInfo;
        MUINT32                         type;

                                        ItemMetaStream(
                                                sp<IMetaStreamInfo> rpInfo,
                                                MUINT32 const rType
                                        ) : pInfo(rpInfo), type(rType)
                                        {}
    };
    //
private:
    typedef DefaultKeyedVector<StreamId_T, sp<ItemImageStream> >
                                                ItemMapImageT;
    typedef DefaultKeyedVector<StreamId_T, sp<ItemMetaStream> >
                                                ItemMapMetaT;
public:
                                        ~StreamConfig();

public:
    MERROR                              add(sp<ItemImageStream> pItem) {
                                            RWLock::AutoWLock _l(mRWLock);
                                            mStreamMap_Image.add(pItem->pInfo->getStreamId(), pItem);
                                            return OK;
                                        }
    MERROR                              add(sp<ItemMetaStream> pItem) {
                                            RWLock::AutoWLock _l(mRWLock);
                                            mStreamMap_Meta.add(pItem->pInfo->getStreamId(), pItem);
                                            return OK;
                                        }

public:
    sp<ItemImageStream>                 queryImage(StreamId_T const streamId) const {
                                            RWLock::AutoRLock _l(mRWLock);
                                            return mStreamMap_Image.valueFor(streamId);
                                        }
    sp<ItemMetaStream>                  queryMeta(StreamId_T const streamId) const {
                                            RWLock::AutoRLock _l(mRWLock);
                                            return mStreamMap_Meta.valueFor(streamId);
                                        }

public:
    void                                dumpState(android::Printer& printer) const;
    MVOID                               dump() const;

private:
    mutable android::RWLock             mRWLock;
    ItemMapImageT                       mStreamMap_Image;
    ItemMapMetaT                        mStreamMap_Meta;
};


/******************************************************************************
 *
 ******************************************************************************/
class NodeConfig
    : public android::VirtualLightRefBase
{

public:
    MVOID                                   addNode(
                                                NodeId_T const nodeId,
                                                sp<ContextNode> pNode
                                            );

    MVOID                                   setImageStreamUsage(
                                                NodeId_T const nodeId,
                                                StreamUsageMap const& usgMap
                                            );

    StreamUsageMap const&                   getImageStreamUsage(
                                                NodeId_T const nodeId
                                            ) const { return mNodeImageStreamUsage.valueFor(nodeId); }

public:     // query
    sp<ContextNode> const                   queryNode(
                                                NodeId_T const nodeId
                                            ) const;

    MUINT                                   queryMinimalUsage(
                                                NodeId_T const nodeId,
                                                StreamId_T const streamId
                                            ) const;
public:     // no lock, since caller should guarantee the calling sequence.
    ContextNodeMapT const&                  getContextNodeMap() const { return mConfig_NodeMap; }

public:
    void                                    dumpState(android::Printer& printer) const;

private:
    mutable android::RWLock                 mRWLock;
    ContextNodeMapT                         mConfig_NodeMap;
    NodeStreamUsageMaps                     mNodeImageStreamUsage;
};


/******************************************************************************
 *
 ******************************************************************************/
class PipelineConfig
    : public android::VirtualLightRefBase
{
public:
    MVOID                                   setRootNode(NodeSet const& roots) { mRootNodes = roots; }
    MVOID                                   setNodeEdges(NodeEdgeSet const& edges) { mNodeEdges = edges; }
    NodeSet const&                          getRootNode() const { return mRootNodes; }
    NodeEdgeSet const&                      getNodeEdges() const { return mNodeEdges; }
    void                                    dumpState(android::Printer& printer) const;
private:
    NodeEdgeSet                             mNodeEdges;
    NodeSet                                 mRootNodes;
};


/******************************************************************************
 *
 ******************************************************************************/
class PipelineContext::PipelineContextImpl
    : public INodeCallbackToPipeline
{
public:
    using IImageStreamBufferPoolT = IPipelineContext::IImageStreamBufferPoolT;

    class InitFrameHandler;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    const std::string                       mName;

private:
    mutable android::RWLock                 mRWLock; //FIXME, use this?

private:    ////                            Configuration
    android::sp<StreamConfig>               mpStreamConfig;
    android::sp<NodeConfig>                 mpNodeConfig;
    android::sp<PipelineConfig>             mpPipelineConfig;
    //
    android::sp<IPipelineFrameNumberGenerator>
                                            mpFrameNumberGenerator;
    android::sp<IPipelineDAG>               mpPipelineDAG;
    android::sp<IPipelineNodeMapControl>    mpPipelineNodeMap;
    android::wp<IDispatcher>                mpDispatcher;
    android::sp<IDispatcher>                mpDispatcher_Default;
    android::wp<IDataCallback>              mpDataCallback;
    android::sp<InFlightRequest>            mpInFlightRequest;
    //
    std::shared_ptr<IFrameBufferManager>    mFrameBufferManager;
    std::shared_ptr<IPipelineFrameStreamBufferProvider>
                                            mFrameStreamBufferProvider;
    std::shared_ptr<IPipelineFrameHalResultProcessor>
                                            mFrameHalResultProcessor;
    //
private:    ////                            Async. Configuration
    mutable android::Mutex                  mAsyncConfigLock;
    std::vector<std::future<int>>           mAsyncConfigFutures;

private:    ////                            Build Frames
    std::mutex                              mBuildFrameLock;

private:    ////                            Init. Frames
    std::atomic_uint32_t                    mConfigInitFrameCount{0};

    std::timed_mutex                        mInitFrameHandlerLock;
    std::shared_ptr<InitFrameHandler>       mInitFrameHandler;

private:    ////                            Frames

    std::timed_mutex                        mSubmitFrameLock;

    /**
     * Count up when every IPipelineContext::queue() is called.
     */
    std::atomic_uint32_t                    mFrameCountQueued{0};

    mutable android::Mutex                  mEnqueLock;
    android::Condition                      mCondEnque;
    MBOOL                                   mEnqueReady = MFALSE;

    mutable android::Mutex                  mKickLock;

    mutable android::Mutex                  mLastFrameLock;
    android::wp<IPipelineFrame>             mpLastFrame;

private:    ////                            Request
    static constexpr uint32_t               BAD_REQUEST_NO = static_cast<uint32_t>(-1);

    std::atomic_uint32_t                    m1stRequestNo{BAD_REQUEST_NO};
    std::atomic_uint32_t                    mLastRequestNo{BAD_REQUEST_NO};

private:    ////                            Flush
    /**
     *                     beginFlush           endFlush
     *                         |                   |
     *                         v                   v
     * requestNo: 0  1  2 | 3  3  3 | 4  4  4 | 5  5  5 |  6  7
     *   frameNo: 0  1  2 | 3  4  5 | 6  7  8 | 9 10 11 | 12 13
     *
     *            All requestNo <= 5 must be flushed.
     */
    mutable android::RWLock                 mFlushLock; // useless?
    std::atomic_uint32_t                    mFlushRequestNo{BAD_REQUEST_NO};
    std::atomic_bool                        mIsFlushing{false};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Configuration
    auto            getFrameBufferManager() const -> std::shared_ptr<IFrameBufferManager>;

protected:  ////    Async. Configuration
    auto            waitUntilConfigDone(const char* szCallName) -> int;

protected:  ////    Build Frames
    auto            constructRequest(
                        FrameBuilderImpl* pBuilder
                    ) -> android::sp<IPipelineFrame>;

protected:  ////    Init. Frames
    auto            waitUntilInitFrameDrained() -> void;
    auto            waitUntilInitFrameDrainedAndDestroyInitFrameHandler() -> void;

protected:  ////    Flush.
    auto            isNeededToFlush(
                        android::sp<IPipelineFrame> const& pFrame
                    ) const -> bool;

    auto            kickRootNodes() -> void;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Instanciaion.
    virtual void    onLastStrongRef(const void* id);

public:     ////    Instanciaion.
                    ~PipelineContextImpl();
                    PipelineContextImpl(char const* name);

public:     ////    Attributes
    auto            getName() const -> char const* { return mName.c_str(); }

    auto            queryNode(NodeId_T const nodeId) const -> android::sp<INodeActor>;

public:     ////    Operations

    auto            updateConfig(NodeBuilderImpl* pBuilder) -> int;
    auto            updateConfig(StreamBuilderImpl* pBuilder) -> int;
    auto            updateConfig(PipelineBuilderImpl* pBuilder) -> int;
    auto            reuseStream(android::sp<StreamConfig::ItemImageStream> pItem) -> int;
    auto            reuseNode(
                        NodeId_T const nodeId,
                        android::sp<ContextNode> pNode,
                        StreamUsageMap const& usgMap
                    ) -> int;
    virtual auto    config(
                        PipelineContextImpl* pOldContext,
                        MBOOL const isAsync,
                        MBOOL const isMultiThread
                    ) -> int;

    auto            buildFrame(
                        std::vector<FrameBuilderImpl*>const& builders
                    ) -> std::vector<android::sp<IPipelineFrame>>;

    auto            buildFrame(
                        FrameBuilderImpl* builder
                    ) -> android::sp<IPipelineFrame>;

    auto            sendFrame(android::sp<IPipelineFrame>const& pFrame) -> int;
    auto            sendFrameToRootNodes(android::sp<IPipelineFrame>const& pFrame,
                                         Vector<sp<IPipelineNode>> &vspPipelineNode) -> int;
    auto            waitEnqueReady( size_t NotEnqueSize ) -> bool;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  INodeCallbackToPipeline Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    auto            onCallback(CallBackParams param) -> MVOID;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineContext Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Debugging
    virtual auto    dumpState(
                        android::Printer& printer,
                        const std::vector<std::string>& options
                    ) -> void;

public:     ////    Configuration

    virtual auto    setDataCallback(
                        android::wp<IDataCallback> pCallback
                    ) -> int;

    virtual auto    setInitFrameCount(uint32_t count) -> void;

public:     ////    Operations
    virtual auto    submitFrame(
                        std::vector<android::sp<IPipelineFrame>>const& frames
                    ) -> int;

    virtual auto    submitOneFrame(
                        android::sp<IPipelineFrame>const &pFrame
                    ) -> int;

    virtual auto    kick(
                        android::sp<IPipelineFrame>const &pFrame
                    ) -> int;

    virtual auto    beginFlush() -> int;
    virtual auto    endFlush() -> int;

    virtual auto    waitUntilDrained() -> int;
    virtual auto    waitUntilRootNodeDrained() -> int;
    virtual auto    waitUntilNodeDrained(NodeId_T const nodeId) -> int;

};


/******************************************************************************
 *
 ******************************************************************************/
struct config_pipeline
{
    struct Params
    {
        // In
        NodeConfig const*               pNodeConfig;
        PipelineConfig const*           pPipelineConfig;
        // Out
        IPipelineDAG*                   pDAG;
        IPipelineNodeMapControl*        pNodeMap;
    };

    MERROR      operator()(Params& rParams);
};


/******************************************************************************
 *
 ******************************************************************************/
struct collect_from_NodeIOMaps
{
    // collect information(StreamSet or NodeSet) from NodeIOMaps.
    MVOID       getStreamSet(
                        NodeIOMaps const& nodeIOMap,
                        StreamSet& collected
                );
#if 0
    MVOID       getNodeSet(
                        NodeIOMaps const& nodeIOMap,
                        NodeSet& collected
                );
#endif
};


android::sp<IPipelineDAG>           constructDAG(
                                        IPipelineDAG const* pConfigDAG,
                                        NodeSet const& rootNodes,
                                        NodeEdgeSet const& edges
                                    );


struct set_streaminfoset_from_config
{
    struct Params
    {
        StreamSet const*        pStreamSet;
        StreamConfig const*     pStreamConfig;
        IStreamInfoSetControl*  pSetControl;
    };
    //
    MERROR  operator() (Params& rParams);
};


            //
struct collect_from_stream_config
{
    struct Params
    {
        /********** in *********/
        StreamConfig const*             pStreamConfig;
        StreamSet const*                pvImageStreamSet;
        StreamSet const*                pvMetaStreamSet;

        /********** out *********/
        ImageStreamInfoMapT*             pvAppImageStreamInfo;
        ImageStreamInfoMapT*             pvHalImageStreamInfo;
        MetaStreamInfoMapT*              pvAppMetaStreamInfo;
        MetaStreamInfoMapT*              pvHalMetaStreamInfo;
    };
    //
    MERROR                  operator()(Params& rParams);
};

#define FRAME_STREAMINFO_DEBUG_ENABLE       (0)
struct update_streaminfo_to_set
{
    struct Params
    {
        // in
        ImageStreamInfoMapT const*       pvAppImageStreamInfo;
        ImageStreamInfoMapT const*       pvHalImageStreamInfo;
        MetaStreamInfoMapT const*        pvAppMetaStreamInfo;
        MetaStreamInfoMapT const*        pvHalMetaStreamInfo;
        // out
        IStreamInfoSetControl*          pSetControl;
    };
    // update each IImageStreamInfo in InfoMap to IStreamInfoSetControl
    MERROR                  operator() (Params& rParams);
};

#define FRAMENODEMAP_DEBUG_ENABLE           (0)
struct construct_FrameNodeMapControl
{
    struct Params
    {
        // in
        NodeIOMaps const*               pImageNodeIOMaps;
        NodeIOMaps const*               pMetaNodeIOMaps;
        IPipelineDAG const* const       pReqDAG;
        IStreamInfoSet const* const     pReqStreamInfoSet;
        // out
        IPipelineFrameNodeMapControl*   pMapControl;
    };
    //
    MERROR                  operator() (Params& rParams);
};

#define FRAMEE_STREAMBUFFER_DEBUG_ENABLE    (0)
struct update_streambuffers_to_frame
{
    using PipelineFrameT = IPipelineBufferSetFrameControl;

    // Image:
    //      App: should have StreamBuffer
    //      Hal: could get StreamBuffer in later stage
    // Meta:
    //      App: control: should have StreamBuffer
    //           result: allocate here
    MERROR  updateAppMetaSB(
                MetaStreamInfoMapT const& vStreamInfo,
                MetaStreamBufferMapsT const& vSBuffers,
                PipelineFrameT* pFrame
            ) const;
    MERROR  updateHalMetaSB(
                MetaStreamInfoMapT const& vStreamInfo,
                MetaStreamBufferMapsT const& vSBuffers,
                PipelineFrameT* pFrame
            ) const;
    MERROR  updateAppImageSB(
                ImageStreamInfoMapT const& vStreamInfo,
                AppImageStreamBufferMapsT const& vSBuffers,
                PipelineFrameT* pFrame
            ) const;
    MERROR  updateHalImageSB(
                ImageStreamInfoMapT const& vStreamInfo,
                ImageStreamBufferMapsT const& vSBuffers,
                PipelineFrameT* pFrame
            ) const;
};


struct evaluate_buffer_users
{
    struct Imp;
    // to evaluate the userGraph of each StreamBuffer
    struct Params
    {
        // in
        NodeConfig const*               pProvider;
        IPipelineDAG const*             pPipelineDAG;
        IPipelineFrameNodeMapControl const*
                                        pNodeMap;
        // out
        IPipelineBufferSetControl*      pBufferSet;
    };
    MERROR  operator() (Params& rParams);
};


};  //namespace NSCam::v3::pipeline::NSPipelineContext
#endif  //_MTK_HARDWARE_MTKCAM_V3_PIPELINE_PIPELINECONTEXTIMPL_H_

