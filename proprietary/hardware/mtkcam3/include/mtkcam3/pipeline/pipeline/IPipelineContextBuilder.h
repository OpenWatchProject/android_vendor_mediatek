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

#ifndef _MTK_HARDWARE_MTKCAM3_INCLUDE_MTKCAM3_PIPELINE_PIPELINE_IPIPELINECONTEXTBUILDER_H_
#define _MTK_HARDWARE_MTKCAM3_INCLUDE_MTKCAM3_PIPELINE_PIPELINE_IPIPELINECONTEXTBUILDER_H_
//
#include "types.h"
#include "IPipelineContext.h"
#include "IPipelineBufferSetFrameControl.h"
//
#include <memory>
#include <vector>

#include <utils/Printer.h>

namespace NSCam::v3::pipeline::NSPipelineContext {


/******************************************************************************
 *
 ******************************************************************************/
class StreamBuilderImpl;
class StreamBuilder
{
public:
    using IImageStreamBufferProviderT = std::shared_ptr<IImageStreamBufferProvider>;
    using AttributeT = uint32_t;

public:
                    ~StreamBuilder();
                    StreamBuilder(
                            AttributeT attribute,
                            android::sp<IImageStreamInfo> pStreamInfo
                    );
                    StreamBuilder(
                            AttributeT attribute,
                            android::sp<IMetaStreamInfo> pStreamInfo
                    );
                    StreamBuilder(const StreamBuilder& builder);

public:
    auto            build(
                        android::sp<IPipelineContext> pContext
                    ) const -> int;

    auto            setProvider(
                        IImageStreamBufferProviderT pProvider
                    ) -> StreamBuilder&;

    auto            setSharedImgStream(
                        android::sp<IImageStreamInfo> pStreamInfo
                    ) -> StreamBuilder&;

private:
    android::sp<StreamBuilderImpl>      mpImpl;
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
                    ~NodeBuilder();
                    NodeBuilder(
                        NodeId_T const nodeId,
                        android::sp<INodeActor> pNode
                    );
                    NodeBuilder(const NodeBuilder& builder);

public:
    auto            build(
                        android::sp<IPipelineContext> pContext
                    ) const -> int;

public:
    auto            addStream(
                        eDirection const direction,
                        StreamSet const& streams
                    ) -> NodeBuilder&;

    auto            setImageStreamUsage(
                        StreamId_T const streamId,
                        MUINT const bufUsage
                    ) -> NodeBuilder&;

private:
    android::sp<NodeBuilderImpl>        mpImpl;
};


/******************************************************************************
 *
 ******************************************************************************/
class PipelineBuilderImpl;
class PipelineBuilder
{
public:
                    ~PipelineBuilder();
                    PipelineBuilder();
                    PipelineBuilder(const PipelineBuilder& builder);

public:
    auto            build(
                        android::sp<IPipelineContext> pContext
                    ) const -> int;

public:
    auto            setRootNode(
                        NodeSet const& roots
                    ) -> PipelineBuilder&;

    auto            setNodeEdges(
                        NodeEdgeSet const& edges
                    ) -> PipelineBuilder&;

private:
    android::sp<PipelineBuilderImpl>    mpImpl;
};


/******************************************************************************
 *
 ******************************************************************************/
class IFrameBuilder
{
public:
    using AppCallbackT          = NSCam::v3::pipeline::IPipelineBufferSetFrameControl::IAppCallback;
    using GroupFrameTypeT       = NSCam::v3::IPipelineFrame::GroupFrameTypeT;
    using HalImageStreamBuffer  = NSCam::v3::Utils::HalImageStreamBuffer;
    using HalMetaStreamBuffer   = NSCam::v3::Utils::HalMetaStreamBuffer;

public:
    virtual         ~IFrameBuilder() = default;

    static  auto    make() -> std::shared_ptr<IFrameBuilder>;

public:     ////    Operations
    virtual auto    dumpState(
                        android::Printer& printer
                    ) -> void                                               = 0;

public:     ////    Access
    virtual auto    getRequestNo() const -> uint32_t                        = 0;
    virtual auto    setRequestNo(uint32_t requestNo) -> void                = 0;

    virtual auto    getReprocessFrame() const -> bool                       = 0;
    virtual auto    setReprocessFrame(
                        MBOOL const bReprocessFrame
                    ) -> void                                               = 0;

    virtual auto    getReprocessSensorTimestamp() const -> int64_t          = 0;
    virtual auto    setReprocessSensorTimestamp(
                        int64_t reprocessSensorTimestamp
                    ) -> void                                               = 0;

    virtual auto    setIOMap(
                        NodeId_T const nodeId,
                        IOMapSet const& imageIOMap,
                        IOMapSet const& metaIOMap
                    ) -> void                                               = 0;

    virtual auto    getRootNode() const -> NodeSet const&                   = 0;
    virtual auto    setRootNode(
                        NodeSet const& roots
                    ) -> void                                               = 0;

    virtual auto    getNodeEdges() const -> NodeEdgeSet const&              = 0;
    virtual auto    setNodeEdges(
                        NodeEdgeSet const& edges
                    ) -> void                                               = 0;

    /* provide new IImageStreamInfo to overwrite the previously configured one. */
    virtual auto    replaceStreamInfo(
                        StreamId_T const streamId,
                        android::sp<IImageStreamInfo> pStreamInfo
                    ) -> void                                               = 0;

    virtual auto    setAppImageStreamBuffer(
                        StreamId_T const streamId,
                        android::sp<IImageStreamInfo> pStreamInfo,
                        android::sp<IImageStreamBuffer> buffer = nullptr
                    ) -> void                                               = 0;

    virtual auto    setAppImageStreamBuffer(
                        StreamId_T const streamId,
                        android::sp<IImageStreamBuffer> buffer
                    ) -> void                                               = 0;

    virtual auto    setHalImageStreamBuffer(
                        StreamId_T const streamId,
                        android::sp<IImageStreamBuffer> buffer
                    ) -> void                                               = 0;

    virtual auto    getAppMetaStreamBuffers(
                        std::vector<android::sp<IMetaStreamBuffer>>& out
                    ) const -> void                                         = 0;;
    virtual auto    setAppMetaStreamBuffer(
                        StreamId_T const streamId,
                        android::sp<IMetaStreamBuffer> buffer
                    ) -> void                                               = 0;

    virtual auto    getHalMetaStreamBuffers(
                        std::vector<android::sp<IMetaStreamBuffer>>& out
                    ) const -> void                                         = 0;
    virtual auto    setHalMetaStreamBuffer(
                        StreamId_T const streamId,
                        android::sp<IMetaStreamBuffer> buffer
                    ) -> void                                               = 0;

    virtual auto    getFrameCallback() const -> android::wp<AppCallbackT>   = 0;
    virtual auto    updateFrameCallback(
                        android::wp<AppCallbackT> pCallback
                    ) -> void                                               = 0;

    // update physical app metadata stream id list.
    virtual auto    setPhysicalAppMetaStreamIds(
                        std::unordered_map<uint32_t, std::vector<StreamId_T> > const*
                                                physicalAppMetaStreamIds
                    ) -> void                                               = 0;

    virtual auto    setTrackFrameResultParams(
                        std::shared_ptr<TrackFrameResultParams const>const& arg
                    ) -> void                                               = 0;

    enum
    {
        /**
         * true indicates this frame is expected to be successfully processed
         * and unexpected to be aborted even during IPipelineNode::flush().
         */
        ENABLED_FLAG_UNEXPECTED_TO_ABORT = 0,

        ENABLED_FLAG_END,
    };
    virtual auto    getEnabled(uint32_t which) const -> bool                = 0;
    virtual auto    setEnabled(uint32_t which, bool enabled) -> void        = 0;

    virtual auto    getGroupFrameType() const -> GroupFrameTypeT            = 0;
    virtual auto    setGroupFrameType(GroupFrameTypeT type) -> void         = 0;
};


/******************************************************************************
 *
 ******************************************************************************/
class IRequestBuilder
{
public:     ////    Definitions.
    using FrameBuilderT = std::shared_ptr<IFrameBuilder>;
    using PipelineFrameT = android::sp<IPipelineFrame>;

public:     ////    Operations
    virtual         ~IRequestBuilder() = default;

public:     ////    Operations

    static  auto    make() -> std::shared_ptr<IRequestBuilder>;

    virtual auto    build(
                        android::sp<IPipelineContext> pPipelineContext,
                        std::vector<FrameBuilderT>const& builders
                    ) -> std::vector<PipelineFrameT>                        = 0;

    virtual auto    build(
                        android::sp<IPipelineContext> pPipelineContext,
                        FrameBuilderT const& builder
                    ) -> PipelineFrameT                                     = 0;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSCam::v3::pipeline::NSPipelineContext
#endif  //_MTK_HARDWARE_MTKCAM3_INCLUDE_MTKCAM3_PIPELINE_PIPELINE_IPIPELINECONTEXTBUILDER_H_

