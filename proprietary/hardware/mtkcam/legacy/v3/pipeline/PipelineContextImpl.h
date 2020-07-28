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
#include "MyUtils.h"
#include <v3/pipeline/PipelineContext.h>
#include <v3/pipeline/IPipelineNodeMapControl.h>
#include <v3/pipeline/IScenarioControl.h>
#include "InFlightRequest.h"
#include "IPipelineFrameNumberGenerator.h"
//
#include <utils/RWLock.h>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace NSPipelineContext {
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
typedef NSCam::v3::Utils::          IStreamBufferProvider
                                    IStreamBufferProviderT;
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
                                            );
    virtual                                 ~ContextNode();

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
    eStreamType                                 mType;
    sp<IImageStreamInfo>                        mpImageStreamInfo;
    sp<IMetaStreamInfo>                         mpMetaStreamInfo;
    sp<IStreamBufferProviderT>                  mpProvider;
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
class RequestBuilderImpl
    : public android::VirtualLightRefBase
{
public:
    enum
    {
        //TODO: optimization
        FLAG_NO_CHANGE          = 0x0,
        FLAG_FIRSTTIME          = 0x1,
        FLAG_IOMAP_CHANGED      = 0x2,
        FLAG_NODEEDGE_CHANGED   = 0x4,
        FLAG_CALLBACK_CHANGED   = 0x8,
        FLAG_REPLACE_STREAMINFO = 0x16,
    };

public:
                                            RequestBuilderImpl()
                                                : mFlag(FLAG_FIRSTTIME)
                                            {}

public:
    MVOID                                   setFlag(MUINT32 flag) { mFlag |= flag; }
    MVOID                                   clearFlag() { mFlag = 0; }
    MBOOL                                   getFlag(MUINT32 const flag) const { return mFlag & flag; }

public:
    MVOID                                   onRequestConstructed();
    MVOID                                   dump(MUINT32 const reqNo, MUINT32 const frameNo) const;

public: // configured data
    MUINT32                                 mFlag;
    //
    NodeIOMaps                              mImageNodeIOMaps;
    NodeIOMaps                              mMetaNodeIOMaps;
    //
    NodeEdgeSet                             mNodeEdges;
    NodeSet                                 mRootNodes;
    //
    wp<AppCallbackT>                        mpCallback;
    //
    ImageStreamInfoMapT                     mReplacingInfos;
    //
    // one-shot, should be cleared after build a request.
    ImageStreamBufferMapsT                  mStreamBuffers_Image;
    HalImageStreamBufferMapsT               mStreamBuffers_HalImage;
    MetaStreamBufferMapsT                   mStreamBuffers_Meta;
    HalMetaStreamBufferMapsT                mStreamBuffers_HalMeta;

public: // intermediates
};


/******************************************************************************
 *
 ******************************************************************************/
class StreamConfig
    : public IPipelineStreamBufferProvider
{
public:
    //
    struct ItemImageStream : public VirtualLightRefBase
    {
        sp<IImageStreamInfo>            pInfo;
        MUINT32             type;
        sp<HalImageStreamBufferPoolT>   pPool;
        sp<IStreamBufferProviderT>      pProvider;
        //
                                        ItemImageStream(
                                                sp<IImageStreamInfo> rpInfo,
                                                MUINT32 const rType
                                        ) : pInfo(rpInfo), type(rType)
                                        {}
                                        ~ItemImageStream() {
                                            if( pPool.get() )
                                                pPool->uninitPool(LOG_TAG);
                                        }
    };

    struct ItemMetaStream : public VirtualLightRefBase
    {
        sp<IMetaStreamInfo>             pInfo;
        MUINT32                         type;
    //
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

public:     ////    interface of IPipelineStreamBufferProvider
    virtual MERROR                      acquireHalStreamBuffer(
                                            // MUINT32 const requestNo,
                                            sp<IImageStreamInfo> const pStreamInfo,
                                            sp<HalImageStreamBuffer>& rpStreamBuffer
                                        )   const;
public:
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
    MVOID                                   dump();

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
    MVOID                                   dump() const;
private:
    NodeEdgeSet                             mNodeEdges;
    NodeSet                                 mRootNodes;
};


/******************************************************************************
 *
 ******************************************************************************/
class DefaultDispatcher
    : public DispatcherBase
{
public:
    static
    sp<DefaultDispatcher>           create() { return new DefaultDispatcher(); }

public:
    virtual MVOID                   onDispatchFrame(
                                        android::sp<IPipelineFrame> const& pFrame,
                                        Pipeline_NodeId_T nodeId
                                    );
};


/******************************************************************************
 *
 ******************************************************************************/
class PipelineContext::PipelineContextImpl
    : public virtual RefBase
{
public:
                                            PipelineContextImpl(char const* name);
                                            ~PipelineContextImpl();
protected:
    virtual void                            onLastStrongRef(const void* id);
    //
public:
    MERROR                                  updateConfig(NodeBuilderImpl* pBuilder);
    MERROR                                  updateConfig(StreamBuilderImpl* pBuilder);
    MERROR                                  updateConfig(PipelineBuilderImpl* pBuilder);
    MERROR                                  reuseStream(android::sp<StreamConfig::ItemImageStream> pItem);
    MERROR                                  reuseNode(
                                                NodeId_T const nodeId,
                                                android::sp<ContextNode> pNode,
                                                StreamUsageMap const& usgMap
                                            );
    sp<IPipelineFrame>                      constructRequest(
                                                RequestBuilderImpl* pBuilder,
                                                MUINT32 const requestNo
                                            );

public:
    MERROR                                  config(
                                                PipelineContextImpl* pOldContext,
                                                MBOOL const isAsync
                                            );
    MERROR                                  queue(
                                                sp<IPipelineFrame> const& pFrame
                                            );
    MERROR                                  waitUntilDrained();
    MERROR                                  waitUntilNodeDrained(NodeId_T const nodeId);
    MERROR                                  beginFlush();
    MERROR                                  endFlush();
    MERROR                                  setScenarioControl(
                                                sp<IScenarioControl> pControl
                                            );
    sp<IScenarioControl>                    getScenarioControl() const { return mpScenarioControl; }
    MERROR                                  setDispatcher(
                                                wp<IDispatcher> pDispatcher
                                            );
    MERROR                                  setDataCallback(
                                                android::wp<IDataCallback> pCallback
                                            );

public:
    android::sp<HalImageStreamBufferPoolT>  queryImageStreamPool(
                                                StreamId_T const streamId
                                            ) const;

    sp<StreamConfig::ItemImageStream>       queryImage(StreamId_T const streamId) const {
                                                return mpStreamConfig->queryImage(streamId);
                                            }

    sp<INodeActor>                          queryNode(NodeId_T const nodeId) const;

public:
    char const*                             getName() const { return mName.string(); }
    MVOID                                   dump() const;
    sp<NodeConfig>                          getNodeConfig() const { return mpNodeConfig; }

private:
    const String8                           mName;

private:
    mutable android::RWLock                 mRWLock; //FIXME, use this?

private:
    sp<StreamConfig>                        mpStreamConfig;
    sp<NodeConfig>                          mpNodeConfig;
    sp<PipelineConfig>                      mpPipelineConfig;
    //
    sp<IScenarioControl>                    mpScenarioControl;
    sp<IPipelineFrameNumberGenerator>       mpFrameNumberGenerator;
    // FIXME: seems not necessary
    //sp<IStreamInfoSetControl>               mpStreamInfoSet;
    sp<IPipelineDAG>                        mpPipelineDAG;
    sp<IPipelineNodeMapControl>             mpPipelineNodeMap;
    wp<IDispatcher>                         mpDispatcher;
    sp<IDispatcher>                         mpDispatcher_Default;
    wp<IDataCallback>                       mpDataCallback;
    sp<InFlightRequest>                     mpInFlightRequest;
    //
private:
    mutable android::RWLock                 mFlushLock;
    MBOOL                                   mInFlush;
};


/******************************************************************************
 *
 ******************************************************************************/
struct config_pipeline
{
    struct Params
    {
        // In
        StreamConfig const*             pStreamConfig;
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
String8             dump(IOMap const& rIomap);

/******************************************************************************
 *
 ******************************************************************************/
sp<HalImageStreamBufferPoolT>               createHalStreamBufferPool(
                                                const char* username,
                                                sp<IImageStreamInfo> pStreamInfo
                                            );
/******************************************************************************
 *
 ******************************************************************************/
struct Log
{
    static String8
    StreamInfo(sp<IStreamInfo> const& pStreamInfo)
    {
        return String8::format( "stream %#" PRIx64 ":%s",
                pStreamInfo->getStreamId(), pStreamInfo->getStreamName()
                );
    }

    static String8
    Info(sp<IMetaStreamInfo> const& pStreamInfo)
    {
        return Log::StreamInfo(pStreamInfo);
    }

    static String8
    Info(sp<IImageStreamInfo> const& pStreamInfo)
    {
        return String8::format( "%s, format:0x%x size:%dx%d",
                Log::StreamInfo(pStreamInfo).string(),
                pStreamInfo->getImgFormat(),
                pStreamInfo->getImgSize().w, pStreamInfo->getImgSize().h
                );
    }
};


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
    typedef IPipelineBufferSetFrameControl          PipelineFrameT;
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
                HalMetaStreamBufferMapsT const& vSBuffers,
                PipelineFrameT* pFrame
            ) const;
    MERROR  updateAppImageSB(
                ImageStreamInfoMapT const& vStreamInfo,
                ImageStreamBufferMapsT const& vSBuffers,
                PipelineFrameT* pFrame
            ) const;
    MERROR  updateHalImageSB(
                ImageStreamInfoMapT const& vStreamInfo,
                HalImageStreamBufferMapsT const& vSBuffers,
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


};  //namespace NSPipelineContext
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_V3_PIPELINE_PIPELINECONTEXTIMPL_H_

