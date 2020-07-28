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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_V3_PIPELINE_IPIPELINECONTEXT_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_V3_PIPELINE_IPIPELINECONTEXT_H_
//
#include <utils/RefBase.h>
#include <utils/StrongPointer.h>
#include <utils/String8.h>
#include <utils/Vector.h>
//
#include <v3/pipeline/IPipelineNode.h>
#include <v3/pipeline/IPipelineBufferSetFrameControl.h>
#include <v3/utils/streambuf/StreamBuffers.h>
#include <v3/utils/streambuf/StreamBufferProvider.h>


/******************************************************************************
 *
 ******************************************************************************/
//
namespace NSCam {
namespace v3 {

//forward declaration
class IScenarioControl;

namespace NSPipelineContext {

typedef NSCam::v3::StreamId_T               StreamId_T;
typedef NSCam::v3::Pipeline_NodeId_T        NodeId_T;

enum StreamCategory
{
    // bit 0~3: behavior
    //    bit 3: app or hal
    eBehavior_HAL               = 0x0,
    eBehavior_APP               = 0x8,
    eCategory_Behavior_MASK     = 0x8,
    //    bit 0~2: hal subset
    eBehavior_HAL_POOL          = 0x1 | eBehavior_HAL,
    eBehavior_HAL_RUNTIME       = 0x2 | eBehavior_HAL,
    eBehavior_HAL_PROVIDER      = 0x3 | eBehavior_HAL,
    eCategory_Behavior_HAL_MASK = 0xF,
    //
    // bit 4: type
    eType_IMAGE                 = 0x0,
    eType_META                  = 0x10,
    eCategory_Type_MASK         = 0x10,
};

#define BehaviorOf(StreamType)      (StreamType & eCategory_Behavior_MASK)
#define HalBehaviorOf(StreamType)   (StreamType & eCategory_Behavior_HAL_MASK)
#define TypeOf(StreamType)          (StreamType & eCategory_Type_MASK)

enum eStreamType
{
    /* image */
    // always have streambuffer in request stage
    eStreamType_IMG_APP             = eType_IMAGE | eBehavior_APP,
    // allocate bufferpool in config stage, get streambuffer from pool
    eStreamType_IMG_HAL_POOL        = eType_IMAGE | eBehavior_HAL_POOL,
    // always no streambuffer in request stage, size may changed in run-time
    eStreamType_IMG_HAL_RUNTIME     = eType_IMAGE | eBehavior_HAL_RUNTIME,
    // hal stream with specified provider
    eStreamType_IMG_HAL_PROVIDER    = eType_IMAGE | eBehavior_HAL_PROVIDER,

    /* meta */
    //eStreamType_META                = eType_META | eBehavior_APP,
    //
    // FIXME, workaroud here
    // Current implementation of hal metadata is going to be phased out.
    eStreamType_META_APP = eType_META | eBehavior_APP,
    eStreamType_META_HAL = eType_META | eBehavior_HAL,
};
//
//
template <typename T>
struct Set
    : public android::Vector<T>
{
    typedef typename android::Vector<T>::iterator           iter;
    typedef typename android::Vector<T>::const_iterator     const_iter;
    //
    Set&            add(T const& item) {
                        if( indexOf(item) < 0 ) // remove redundancy
                        this->push_back(item);
                        return *this;
                    }
    Set&            add(Set<T> const& set) {
                        typename android::Vector<T>::const_iterator iter = set.begin();
                        while( iter != set.end() ) {
                            Set::add(*iter); iter++;
                        }
                        return *this;
                    }
    ssize_t         indexOf(T const& item) {
                        for( size_t i = 0; i < this->size(); i++ ) {
                            if( item == this->itemAt(i) ) return i;
                        }
                        return -1;
                    }
};

typedef Set<NodeId_T>                   NodeSet;
typedef Set<StreamId_T>                 StreamSet;


/******************************************************************************
 *
 ******************************************************************************/
class INodeActor
    : public virtual android::RefBase
{
public:
    enum
    {
        //eNodeState_None,
        eNodeState_Create,
        eNodeState_Init,
        eNodeState_Config,
    };
    //
public:
    virtual                                 ~INodeActor() {}
protected:
                                            INodeActor(MUINT32 st) : muStatus(st) {}
    virtual void                            onLastStrongRef(const void* id);
public:
    MUINT32                                 getStatus() const;
public:
    MERROR                                  init();
    MERROR                                  config();
    MERROR                                  uninit();
protected: // template methods: to be implmented by derived class
    virtual MERROR                          onInit()                                = 0;
    virtual MERROR                          onConfig()                              = 0;
    virtual MERROR                          onUninit()                              = 0;
public:
    virtual IPipelineNode*                  getNode()                               = 0;
    //
private:
    mutable android::Mutex                  mLock;
    MUINT32                                 muStatus;
};


template <class _Node_>
class NodeActor
    : public INodeActor
{
public:
    typedef _Node_                          NodeT;
    typedef typename _Node_::InitParams     InitParamsT;
    typedef typename _Node_::ConfigParams   ConfigParamsT;
public:
                                            NodeActor(
                                                android::sp<NodeT> pNode,
                                                MUINT32 const rNodeState = eNodeState_Create
                                                )
                                                : INodeActor(rNodeState)
                                                , mpNode(pNode)
                                            { memset(&mInitParam,0,sizeof(InitParamsT)); }
protected:
    virtual MERROR                          onInit()    { return mpNode->init(mInitParam); }
    virtual MERROR                          onConfig()  { return mpNode->config(mConfigParam); }
    virtual MERROR                          onUninit()  { return mpNode->uninit(); }
public:
    virtual IPipelineNode*                  getNode() { return mpNode.get(); }
    virtual NodeT*                          getNodeImpl() { return mpNode.get(); }

public:
    MVOID                                   setInitParam(InitParamsT const& rParam) {
                                                mInitParam = rParam;
                                            }
    MVOID                                   setConfigParam(ConfigParamsT const& rParam) {
                                                mConfigParam = rParam;
                                            }
    MVOID                                   getInitParam(InitParamsT& rParam) {
                                                rParam = mInitParam;
                                            }
    MVOID                                   getConfigParam(ConfigParamsT& rParam) {
                                                rParam = mConfigParam;
                                            }
private:
    android::sp<NodeT>                      mpNode;
    InitParamsT                             mInitParam;
    ConfigParamsT                           mConfigParam;
};

class IDataCallback
    : public virtual android::RefBase
{
public:
    virtual MVOID                           onImageCallback(
                                                MUINT32                         requestNo,
                                                Pipeline_NodeId_T               nodeId,
                                                StreamId_T                      streamId,
                                                android::sp<IImageBufferHeap>&  rpImageBuffer,
                                                android::sp<IImageStreamInfo>&  rpStreamInfo,
                                                MBOOL                           errorResult = MFALSE
                                            )                                       = 0;
    virtual MVOID                           onMetaCallback(
                                                MUINT32              requestNo,
                                                Pipeline_NodeId_T    nodeId,
                                                StreamId_T           streamId,
                                                IMetadata const&     rMetaData,
                                                MBOOL                errorResult = MFALSE
                                            )                                       = 0;
};

class IDispatcher
    : public virtual IPipelineNodeCallback
{
public:
    virtual MVOID                           beginFlush()                                = 0;
    virtual MVOID                           endFlush()                                  = 0;

    virtual MERROR                          setDataCallback(
                                                 android::wp<IDataCallback> pCallback
                                            )                                           = 0;

public: ////    interface of IPipelineNodeCallback
    virtual MVOID                           onDispatchFrame(
                                                android::sp<IPipelineFrame> const& pFrame,
                                                Pipeline_NodeId_T nodeId
                                            )                                           = 0;
    virtual MVOID                           onEarlyCallback(
                                                MUINT32           requestNo,
                                                Pipeline_NodeId_T nodeId,
                                                StreamId_T        streamId,
                                                IMetadata const&  rMetaData,
                                                MBOOL             errorResult = MFALSE
                                            )                                           = 0;
};

class DispatcherBase
    : public virtual IDispatcher
{
public:
                                            DispatcherBase()
                                            {
                                                mInFlush = MFALSE;
                                            }

public: ////    interface of IDispatcher
    virtual MVOID                           beginFlush()
                                            {
                                                android::RWLock::AutoWLock _l(mFlushLock);
                                                mInFlush = MTRUE;
                                            }
    virtual MVOID                           endFlush()
                                            {
                                                android::RWLock::AutoWLock _l(mFlushLock);
                                                mInFlush = MFALSE;
                                            }
    MERROR                                  setDataCallback(
                                                 android::wp<IDataCallback> pCallback
                                            )
                                            {
                                                mpDataCallback = pCallback;
                                                return android::OK;
                                            }


public: ////    interface of IPipelineNodeCallback
    virtual MVOID                           onDispatchFrame(
                                                android::sp<IPipelineFrame> const& pFrame,
                                                Pipeline_NodeId_T nodeId
                                            )                                           = 0;



    MVOID                                   onEarlyCallback(
                                                MUINT32           requestNo,
                                                Pipeline_NodeId_T nodeId,
                                                StreamId_T        streamId,
                                                IMetadata const&  rMetaData,
                                                MBOOL             errorResult = MFALSE
                                            );
protected:
    mutable android::RWLock                 mFlushLock;
    MBOOL                                   mInFlush;

private:
    android::wp<IDataCallback>              mpDataCallback;
};

class PipelineContext
    : public virtual android::RefBase
{
    friend class StreamBuilder;
    friend class NodeBuilder;
    friend class PipelineBuilder;
    friend class RequestBuilder;

public:
    typedef NSCam::v3::Utils::                  HalImageStreamBuffer
                                                HalImageStreamBuffer;
    typedef HalImageStreamBuffer::              Allocator
                                                HalImageStreamBufferAllocatorT;
    typedef HalImageStreamBufferAllocatorT::    StreamBufferPoolT
                                                HalImageStreamBufferPoolT;
    //
public:
    static android::sp<PipelineContext>         create(char const* name);
    //
protected:
                                                PipelineContext(char const* name);
                                                ~PipelineContext();
    virtual void                                onLastStrongRef(const void* id);

public:
    char const*                                 getName() const;

public:
    MERROR                                      beginConfigure(
                                                    android::sp<PipelineContext> oldContext = NULL
                                                );
    MERROR                                      endConfigure(MBOOL asyncConfig = MFALSE);

public:
    MERROR                                      queue(
                                                    android::sp<IPipelineFrame> const &pFrame
                                                );
    MERROR                                      flush();
    MERROR                                      waitUntilDrained();
    MERROR                                      waitUntilNodeDrained(NodeId_T const nodeId);
    //
public:
    MERROR                                      setScenarioControl(
                                                    android::sp<IScenarioControl> pControl
                                                );
    android::sp<IScenarioControl>               getScenarioControl() const;
    //
    MERROR                                      setDispatcher(
                                                    android::wp<IDispatcher> pDispatcher
                                                );
    MERROR                                      setDataCallback(
                                                    android::wp<IDataCallback> pCallback
                                                );
    //
    template <typename _Node_>
    MERROR                                      queryNodeActor(
                                                    NodeId_T const nodeId,
                                                    android::sp< NodeActor<_Node_> >& pNodeActor
                                                )
                                                {
                                                    typedef NodeActor<_Node_>       NodeActorT;
                                                    typedef android::sp<INodeActor> SP_INodeActorT;
                                                    //
                                                    SP_INodeActorT pINodeActor = queryINodeActor(nodeId);
                                                    if( ! pINodeActor.get() ) return android::NAME_NOT_FOUND;
                                                    pNodeActor = static_cast<NodeActorT*>(pINodeActor.get());
                                                    return android::OK;
                                                }
    //
public: //FIXME: workaround to get pool
    android::sp<HalImageStreamBufferPoolT>      queryImageStreamPool(
                                                    StreamId_T const streamId
                                                ) const;

    android::sp<PipelineContext>                queryOldContext() const { return mpOldContext; }

    /**
     * Query a image stream info.
     *
     * @param[in] streamId: id of stream for quried.
     *
     * @param[in] pStreamInfo:
     *            caller set NULL;.callee must promise its value
     *
     * @return
     *      0 indicates success, no matter it exists or not;
     *      otherwise failure.
     */
    MERROR                                      queryStream(
                                                    StreamId_T const streamId,
                                                    android::sp<IImageStreamInfo>& pStreamInfo
                                                );

    /**
     * Reuse a image stream from old context StreamConfig.
     *
     * @param[in] pStreamInfo:
     *            caller must promise its value
     *
     * @return
     *      0 indicates success; otherwise failure.
     */
    MERROR                                      reuseStream(
                                                    android::sp<IImageStreamInfo>& pStreamInfo
                                                );

    MERROR                                      reuseNode(
                                                    NodeId_T const nodeId
                                                );

public:
    android::sp<INodeActor>                     queryINodeActor(
                                                    NodeId_T const nodeId
                                                ) const;

public:
    MVOID                                       dump();

protected:  //      Pipeline's status
    // TODO
    enum ContextState
    {
        eContextState_empty,
        eContextState_configuring,
        eContextState_configured,
    };
    MUINT32                                     getState() const { return 0; }

private:
    mutable android::Mutex                      mLock;
    android::sp<PipelineContext>                mpOldContext;

    class PipelineContextImpl;
    android::sp<PipelineContextImpl>            mpImpl;

private:
    PipelineContextImpl*                        getImpl() const;
};


class StreamBuilderImpl;
class StreamBuilder
{
    typedef NSCam::v3::Utils::                  IStreamBufferProvider
                                                IStreamBufferProviderT;
public:
                                                StreamBuilder(
                                                        eStreamType const type,
                                                        android::sp<IImageStreamInfo> pStreamInfo
                                                );

                                                StreamBuilder(
                                                        eStreamType const type,
                                                        android::sp<IMetaStreamInfo> pStreamInfo
                                                );

                                                StreamBuilder(const StreamBuilder& builder);

                                                ~StreamBuilder();

public:
    StreamBuilder&                              setProvider(
                                                    android::sp<IStreamBufferProviderT> pProvider
                                                );

    MERROR                                      build(
                                                    android::sp<PipelineContext> pContext
                                                ) const;

private:
    android::sp<StreamBuilderImpl>              mpImpl;
};


/******************************************************************************
 *
 ******************************************************************************/
class NodeBuilderImpl;
class NodeBuilder
{
public:
    typedef enum Direction
    {
        // direction
        eDirection_IN    = 0x0,
        eDirection_OUT   = 0x1,
    } eDirection;

public:
                                                NodeBuilder(
                                                    NodeId_T const nodeId,
                                                    android::sp<INodeActor> pNode
                                                    );
                                                NodeBuilder(const NodeBuilder& builder);
                                                ~NodeBuilder();

public:
    NodeBuilder&                                addStream(
                                                    eDirection const direction,
                                                    StreamSet const& streams
                                                );
    NodeBuilder&                                setImageStreamUsage(
                                                    StreamId_T const streamId,
                                                    MUINT const bufUsage
                                                );
    MERROR                                      build(
                                                    android::sp<PipelineContext> pContext
                                                ) const;
private:
    android::sp<NodeBuilderImpl>                mpImpl;
};


/******************************************************************************
 *
 ******************************************************************************/
struct NodeEdge {
    NodeId_T src;
    NodeId_T dst;
};

inline bool operator== (NodeEdge const& lhs, NodeEdge const& rhs) {
    return lhs.src == rhs.src && lhs.dst == rhs.dst;
}

class NodeEdgeSet
    : public Set<NodeEdge>
{
public:
    typedef android::Vector<NodeEdge>::iterator          iterator;
    typedef android::Vector<NodeEdge>::const_iterator    const_iterator;

public:
    NodeEdgeSet&                                addEdge(
                                                    NodeId_T const src,
                                                    NodeId_T const dst
                                                ) {
                                                    struct NodeEdge e = {src, dst};
                                                    Set<NodeEdge>::add(e);
                                                    return *this;
                                                }
};


/******************************************************************************
 *
 ******************************************************************************/
class PipelineBuilderImpl;
class PipelineBuilder
{
public:
                                                PipelineBuilder();
                                                PipelineBuilder(const PipelineBuilder& builder);
                                                ~PipelineBuilder();

public:
    MERROR                                      build(
                                                    android::sp<PipelineContext> pContext
                                                ) const;

public:
    PipelineBuilder&                            setRootNode(
                                                    NodeSet const& roots
                                                );

    PipelineBuilder&                            setNodeEdges(
                                                    NodeEdgeSet const& edges
                                                );

private:
    android::sp<PipelineBuilderImpl>            mpImpl;
};


/******************************************************************************
 *
 ******************************************************************************/
struct IOMap
{
    StreamSet                       vIn;
    StreamSet                       vOut;
    //
    MBOOL                           isEmpty() const {
                                        return vIn.size() == 0 && vOut.size() == 0;
                                    }
    //
    IOMap&                          addIn(StreamId_T const stream) {
                                        vIn.add(stream); return *this;
                                    }
    IOMap&                          addOut(StreamId_T const stream) {
                                        vOut.add(stream); return *this;
                                    }
    IOMap&                          addIn(StreamSet const& stream) {
                                        vIn.add(stream); return *this;
                                    }
    IOMap&                          addOut(StreamSet const& stream) {
                                        vOut.add(stream); return *this;
                                    }
    size_t                          sizeIn()  { return vIn.size(); }
    size_t                          sizeOut() { return vOut.size(); }
};

struct IOMapSet
: public android::Vector<IOMap>
{
    typedef typename android::Vector<IOMap>::iterator       iterator;
    typedef typename android::Vector<IOMap>::const_iterator const_iterator;
    //
    IOMapSet&                       add(IOMap const& map) {
                                        this->push_back(map); return *this;
                                    }

    static const IOMapSet&          empty();
};


/******************************************************************************
 *
 ******************************************************************************/
class RequestBuilderImpl;
class RequestBuilder
    : public virtual android::RefBase
{
public:
    typedef IPipelineBufferSetFrameControl::IAppCallback
                                                AppCallbackT;

    typedef NSCam::v3::Utils::                  HalImageStreamBuffer
                                                HalImageStreamBuffer;
    typedef NSCam::v3::Utils::                  HalMetaStreamBuffer
                                                HalMetaStreamBuffer;
public:
                                                RequestBuilder();
                                                ~RequestBuilder();

public:
    android::sp<IPipelineFrame>                 build(
                                                    MUINT32 const requestNo,
                                                    android::sp<PipelineContext> pContext
                                                );

public:
    RequestBuilder&                             setIOMap(
                                                    NodeId_T const nodeId,
                                                    IOMapSet const& imageIOMap,
                                                    IOMapSet const& metaIOMap
                                                );

    RequestBuilder&                             setRootNode(
                                                    NodeSet const& roots
                                                );

    RequestBuilder&                             setNodeEdges(
                                                    NodeEdgeSet const& edges
                                                );

    /* provide new IImageStreamInfo to overwrite the previously configured one. */
    RequestBuilder&                             replaceStreamInfo(
                                                    StreamId_T const streamId,
                                                    android::sp<IImageStreamInfo> pStreamInfo
                                                );

    /* provide stream buffer if existed */
    RequestBuilder&                             setImageStreamBuffer(
                                                    StreamId_T const streamId,
                                                    android::sp<IImageStreamBuffer> buffer
                                                );
    // FIXME: workaround. Should not use Hal...
    RequestBuilder&                             setImageStreamBuffer(
                                                    StreamId_T const streamId,
                                                    android::sp<HalImageStreamBuffer> buffer
                                                );
    RequestBuilder&                             setMetaStreamBuffer(
                                                    StreamId_T const streamId,
                                                    android::sp<IMetaStreamBuffer> buffer
                                                );
    // FIXME: workaround. Should not use Hal...
    RequestBuilder&                             setMetaStreamBuffer(
                                                    StreamId_T const streamId,
                                                    android::sp<HalMetaStreamBuffer> buffer
                                                );

    // the callback that implements
    //     virtual MVOID           updateFrame(
    //                                 MUINT32 const requestNo,
    //                                 MINTPTR const userId,
    //                                 ssize_t const nOutMetaLeft,
    //                                 android::Vector<android::sp<IMetaStreamBuffer> > vOutMeta
    //                             )                                           = 0;
    //
    RequestBuilder&                             updateFrameCallback(
                                                    android::wp<AppCallbackT> pCallback
                                                );
private:
    android::sp<RequestBuilderImpl>             mpImpl;
};



/******************************************************************************
*
******************************************************************************/
};  //namespace NSPipelineContext
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_V3_PIPELINE_IPIPELINECONTEXT_H_

