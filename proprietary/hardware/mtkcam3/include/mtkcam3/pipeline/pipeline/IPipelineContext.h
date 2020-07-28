/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#ifndef _MTK_HARDWARE_MTKCAM3_INCLUDE_MTKCAM3_PIPELINE_PIPELINE_IPIPELINECONTEXT_H_
#define _MTK_HARDWARE_MTKCAM3_INCLUDE_MTKCAM3_PIPELINE_PIPELINE_IPIPELINECONTEXT_H_
//
#include <memory>
#include <string>
#include <vector>

#include <utils/Printer.h>
#include <utils/RefBase.h>
#include <utils/StrongPointer.h>

#include "types.h"
#include "IPipelineNode.h"
#include "ISessionLifeHolder.h"

#include <mtkcam3/pipeline/stream/IStreamBufferProvider.h>
#include <mtkcam3/pipeline/utils/streambuf/StreamBuffers.h>
#include <mtkcam3/pipeline/utils/SyncHelper/ISyncHelper.h>
#include <mtkcam3/pipeline/pipeline/IFrameBufferManager.h>

namespace NSCam::v3::pipeline::NSPipelineContext {


/******************************************************************************
 *
 ******************************************************************************/
namespace implementation {
class INodeOpsActor
{
public:
    virtual         ~INodeOpsActor() = default;

    virtual auto    getNode() const -> IPipelineNode*                           = 0;
    virtual auto    getConfigParam() const -> void const*                       = 0;
    virtual auto    setConfigParam(void const* pConfigParams) -> void           = 0;

    virtual auto    config(void const* pConfigParams) -> int                    = 0;

};


template <class _NodeT_>
class NodeOpsActor : public INodeOpsActor
{
public:
    using NodeT = _NodeT_;
    using ConfigParamsT = typename NodeT::ConfigParams;

public:
    android::sp<NodeT>  mpNode;
    ConfigParamsT       mConfigParams;

public:
                    NodeOpsActor(android::sp<NodeT> pNode) : mpNode(pNode) {}

    virtual auto    getNode() const -> IPipelineNode* { return mpNode.get(); }
    virtual auto    getConfigParam() const -> void const* { return &mConfigParams; }
    virtual auto    setConfigParam(void const* pConfigParams) -> void
                    {
                        mConfigParams = *reinterpret_cast<ConfigParamsT const*>(pConfigParams);
                    }

    virtual auto    config(void const* pConfigParams) -> int
                    {
                        auto const& params = *reinterpret_cast<ConfigParamsT const*>(pConfigParams);
                        return mpNode->config(params);
                    }

};
}; //namespace implementation


/******************************************************************************
 *
 ******************************************************************************/
class INodeActor;
extern "C" INodeActor* createNodeActor(std::shared_ptr<implementation::INodeOpsActor> pNodeOps);

template <class NodeT>
static auto makeINodeActor(android::sp<NodeT> pNode) -> android::sp<INodeActor>
{
    return createNodeActor(std::make_shared<implementation::NodeOpsActor<NodeT>>(pNode));
}

class INodeActor
    : public virtual android::RefBase
{
public:
    using InitParamsT = IPipelineNode::InitParams;

public:
    virtual         ~INodeActor() = default;

public:
    virtual auto    init() -> int                                               = 0;
    virtual auto    config() -> int                                             = 0;
    virtual auto    uninit() -> int                                             = 0;
    virtual auto    reconfig() -> int                                           = 0;

public:
    virtual auto    toString() const -> std::string                             = 0;

public:
    virtual auto    getNode() -> IPipelineNode*                                 = 0;
    virtual auto    getNodeId() const -> NodeId_T                               = 0;
    virtual auto    getNodeName() const -> char const*                          = 0;
    virtual auto    getInitOrder() const -> int32_t                             = 0;
    virtual auto    setInitOrder(int32_t order) -> void                         = 0;

    /**
     * >=0: a timeout in nanoseconds
     * < 0: no timeout (by default)
     */
    virtual auto    getInitTimeout() const -> int64_t                           = 0;
    virtual auto    setInitTimeout(int64_t timeout) -> void                     = 0;

    virtual auto    setInitParam(InitParamsT const& rParam) -> void             = 0;
    virtual auto    setConfigParam(void const* pConfigParams) -> void           = 0;

public:
    /**
     * Set the elapsed time, in nanoseconds, of init() and config().
     */
    virtual auto    setInitElapsedTime(int64_t t) -> void                       = 0;

};


/******************************************************************************
 *
 ******************************************************************************/
class IDataCallback
    : public virtual android::RefBase
{
public:

    /**
     * This method is called when some (but not all) result metadata are available.
     *
     * For each frame, some result metadata might be available earlier than others.
     * For performance-oriented use-cases, applications should query the metadata they need to make
     * forward progress from the partial results and avoid waiting for the completed result.
     */
    using MetaResultAvailable = IPipelineNodeCallback::MetaResultAvailable;
    virtual auto    onMetaResultAvailable(
                      MetaResultAvailable&& arg __unused
                    ) -> void
                    {
                    }

    /**
     *
     */
    virtual MVOID   onMetaCallback(
                        MUINT32              requestNo __unused,
                        Pipeline_NodeId_T    nodeId __unused,
                        StreamId_T           streamId __unused,
                        IMetadata const&     rMetaData __unused,
                        MBOOL                errorResult __unused
                    )
                    {
                    }

    /**
     *
     */
    virtual MVOID   onNextCaptureCallBack(
                         MUINT32             requestNo __unused,
                         Pipeline_NodeId_T   nodeId __unused,
                         MUINT32             requestCnt __unused,
                         MBOOL               bSkipCheck __unused
                    )
                    {
                    }

    /**
     *
     */
    virtual MBOOL   isCtrlSetting() { return MFALSE; };
    virtual MVOID   onCtrlSetting(
                        MUINT32             requestNo __unused,
                        Pipeline_NodeId_T   nodeId __unused,
                        StreamId_T const    metaAppStreamId __unused,
                        IMetadata&          rAppMetaData __unused,
                        StreamId_T const    metaHalStreamId __unused,
                        IMetadata&          rHalMetaData __unused,
                        MBOOL&              rIsChanged __unused
                    )
                    {
                    }

    /**
     *
     */
    virtual MBOOL   isCtrlSync() { return MFALSE; };
    virtual MVOID   onCtrlSync(
                        MUINT32             requestNo __unused,
                        Pipeline_NodeId_T   nodeId __unused,
                        MUINT32             index __unused,
                        MUINT32             type __unused,
                        MINT64              duration __unused
                    )
                    {
                    }

    /**
     *
     */
    virtual MBOOL   isCtrlResize() { return MFALSE; };
    virtual MVOID   onCtrlResize(
                        MUINT32             requestNo __unused,
                        Pipeline_NodeId_T   nodeId __unused,
                        StreamId_T const    metaAppStreamId __unused,
                        IMetadata&          rAppMetaData __unused,
                        StreamId_T const    metaHalStreamId __unused,
                        IMetadata&          rHalMetaData __unused,
                        MBOOL&              rIsChanged __unused
                    )
                    {
                    }

    /**
     *
     */
    virtual MBOOL   isCtrlReadout() { return MFALSE; };
    virtual MVOID   onCtrlReadout(
                        MUINT32             requestNo __unused,
                        Pipeline_NodeId_T   nodeId __unused,
                        StreamId_T const    metaAppStreamId __unused,
                        IMetadata&          rAppMetaData __unused,
                        StreamId_T const    metaHalStreamId __unused,
                        IMetadata&          rHalMetaData __unused,
                        MBOOL&              rIsChanged __unused
                    )
                    {
                    }

};
struct DataCallbackBase : public virtual IDataCallback {}; //[TODO] remove this.


/******************************************************************************
 *
 ******************************************************************************/
class IDispatcher
    : public virtual IPipelineNodeCallback
{
public:
    virtual int     setDataCallback(android::wp<IDataCallback> pCallback)   = 0;

    virtual MVOID   beginFlush()                                            = 0;
    virtual MVOID   endFlush()                                              = 0;

};


/******************************************************************************
 *
 ******************************************************************************/
class IPipelineContext
    : public virtual android::RefBase
{
public:     ////    Definitions.
    using HalImageStreamBufferT = NSCam::v3::Utils::HalImageStreamBuffer;
    using HalImageStreamBufferAllocatorT = HalImageStreamBufferT::Allocator;
    using HalImageStreamBufferPoolT = HalImageStreamBufferAllocatorT::StreamBufferPoolT;
    using IImageStreamBufferPoolT = NSCam::v3::Utils::IStreamBufferPool<IImageStreamBuffer>;

    using MultiCamSyncHelperT = NSCam::v3::Utils::Imp::ISyncHelper;

public:
    virtual         ~IPipelineContext() = default;

public:     ////    Debugging
    virtual auto    dumpState(
                        android::Printer& printer,
                        const std::vector<std::string>& options
                    ) -> void                                                   = 0;

public:     ////    Configuration

    virtual auto    setDataCallback(
                        android::wp<IDataCallback> pCallback
                    ) -> int                                                    = 0;

    virtual auto    setInitFrameCount(uint32_t count) -> void                   = 0;

    virtual auto    beginConfigure(
                        android::sp<IPipelineContext> oldContext = nullptr
                    ) -> int                                                    = 0;
    virtual auto    endConfigure(
                        MBOOL asyncConfig = MFALSE,
                        MBOOL isMultiThread = MFALSE
                    ) -> int                                                    = 0;

    virtual auto    getMultiCamSyncHelper() -> android::sp<MultiCamSyncHelperT> = 0;
    virtual auto    setMultiCamSyncHelper(
                        android::sp<MultiCamSyncHelperT>& helper
                    ) -> void = 0;

    virtual auto    getSessionLifeHolder() const -> std::shared_ptr<ISessionLifeHolder> = 0;
    virtual auto    setSessionLifeHolder(
                        std::shared_ptr<ISessionLifeHolder> const& mgr) -> void = 0;

public:     ////    Attributes
    virtual auto    getNativeName() const -> std::string                        = 0;
    virtual auto    getNativeHandle() -> void*                                  = 0;

    virtual auto    queryINodeActor(
                        NodeId_T const nodeId
                    ) const -> android::sp<INodeActor>                          = 0;

public:     ////    Operations
    virtual auto    queue(
                        android::sp<IPipelineFrame>const& frame
                    ) -> int                                                    = 0;

    virtual auto    queue(
                        std::vector<android::sp<IPipelineFrame>>const& frames
                    ) -> int                                                    = 0;

    virtual auto    kick(
                        android::sp<IPipelineFrame>const &pFrame
                    ) -> int                                                    = 0;

    virtual auto    beginFlush() -> int                                         = 0;
    virtual auto    endFlush() -> int                                           = 0;

    virtual auto    waitUntilDrained() -> int                                   = 0;
    virtual auto    waitUntilRootNodeDrained() -> int                           = 0;
    virtual auto    waitUntilNodeDrained(NodeId_T const nodeId) -> int          = 0;

public:     ////    fbm
    virtual auto    getHistoryBufferProvider() const
                        -> std::shared_ptr<IHistoryBufferProvider>              = 0;
};


/******************************************************************************
 *
 ******************************************************************************/
class IPipelineContextManager
{
public:     ////    Interfaces.

    virtual         ~IPipelineContextManager() = default;

    /**
     * Get the singleton instance of IPipelineContextManager.
     */
    static  auto    get() -> std::shared_ptr<IPipelineContextManager>;

    /**
     * Create an instance of pipeline context.
     *
     * @param[in] name: the name of pipeline context.
     */
    virtual auto    create(char const* name) -> android::sp<IPipelineContext>   = 0;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSCam::v3::pipeline::NSPipelineContext
#endif  //_MTK_HARDWARE_MTKCAM3_INCLUDE_MTKCAM3_PIPELINE_PIPELINE_IPIPELINECONTEXT_H_

