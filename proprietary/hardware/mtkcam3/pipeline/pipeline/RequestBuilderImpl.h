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

#ifndef _MTK_HARDWARE_MTKCAM3_PIPELINE_PIPELINE_REQUESTBUILDERIMPL_H_
#define _MTK_HARDWARE_MTKCAM3_PIPELINE_PIPELINE_REQUESTBUILDERIMPL_H_
//
#include <mtkcam3/pipeline/pipeline/IPipelineContextBuilder.h>

#include <bitset>
#include <unordered_map>
#include <utility>

#include <utils/KeyedVector.h>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam::v3::pipeline::NSPipelineContext {


/******************************************************************************
 *
 ******************************************************************************/
class FrameBuilderImpl
    : public IFrameBuilder
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definition
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    using NodeIOMaps                = android::DefaultKeyedVector<NodeId_T, IOMapSet>;
    using ImageStreamInfoMapT       = android::DefaultKeyedVector<StreamId_T, android::sp<IImageStreamInfo>>;
    using ImageStreamBufferMapsT    = android::DefaultKeyedVector<StreamId_T, android::sp<IImageStreamBuffer>>;
    using HalImageStreamBufferMapsT = android::DefaultKeyedVector<StreamId_T, android::sp<HalImageStreamBuffer>>;
    using MetaStreamBufferMapsT     = android::DefaultKeyedVector<StreamId_T, android::sp<IMetaStreamBuffer>>;
    using HalMetaStreamBufferMapsT  = android::DefaultKeyedVector<StreamId_T, android::sp<HalMetaStreamBuffer>>;
    using AppImageStreamBufferMapsT = std::unordered_map<StreamId_T,
                                            std::pair<android::sp<IImageStreamInfo>, android::sp<IImageStreamBuffer>>>;

public:     ////
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

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    uint32_t                    mFlag{FLAG_FIRSTTIME};
    //
    uint32_t                    mRequestNo = 0;
    //
    NodeIOMaps                  mImageNodeIOMaps;
    NodeIOMaps                  mMetaNodeIOMaps;
    //
    NodeEdgeSet                 mNodeEdges;
    NodeSet                     mRootNodes;
    bool                        mbReprocessFrame{false};
    int64_t                     mReprocessSensorTimestamp{0};
    //
    android::wp<AppCallbackT>   mpCallback;
    //
    ImageStreamInfoMapT         mReplacingInfos;
    //
    // one-shot, should be cleared after build a request.
    AppImageStreamBufferMapsT   mStreamBuffers_AppImage;
    ImageStreamBufferMapsT      mStreamBuffers_HalImage;
    MetaStreamBufferMapsT       mStreamBuffers_AppMeta;
    MetaStreamBufferMapsT       mStreamBuffers_HalMeta;

    // physical camera setting
    std::vector<int32_t>        mvPhysicalCameraSetting;
    std::unordered_map<uint32_t, std::vector<StreamId_T> >
                                mvPhysicalAppStreamIds;

    std::shared_ptr<TrackFrameResultParams const>
                                mTrackFrameResultParams;

    std::bitset<ENABLED_FLAG_END>   mEnabledFlags{0};

    GroupFrameTypeT             mGroupFrameType{GroupFrameTypeT::MAIN};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Operations
    auto            setFlag(uint32_t flag) -> void { mFlag |= flag; }
    auto            clearFlag() -> void { mFlag = 0; }
    auto            getFlag(uint32_t flag) const -> bool { return mFlag & flag; }

public:     ////    Operations
    auto            onRequestConstructed() -> void;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IFrameBuilder Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Operations
    virtual auto    dumpState(
                        android::Printer& printer
                    ) -> void;

public:     ////    Access
    virtual auto    getRequestNo() const -> uint32_t { return mRequestNo; }
    virtual auto    setRequestNo(uint32_t requestNo) -> void;

    virtual auto    getReprocessFrame() const -> bool { return mbReprocessFrame; }
    virtual auto    setReprocessFrame(
                        MBOOL const bReprocessFrame
                    ) -> void;

    virtual auto    getReprocessSensorTimestamp() const -> int64_t { return mReprocessSensorTimestamp; }
    virtual auto    setReprocessSensorTimestamp(
                        int64_t reprocessSensorTimestamp
                    ) -> void;

    virtual auto    setIOMap(
                        NodeId_T const nodeId,
                        IOMapSet const& imageIOMap,
                        IOMapSet const& metaIOMap
                    ) -> void;

    virtual auto    getRootNode() const -> NodeSet const& { return mRootNodes; }
    virtual auto    setRootNode(
                        NodeSet const& roots
                    ) -> void;

    virtual auto    getNodeEdges() const -> NodeEdgeSet const& { return mNodeEdges; }
    virtual auto    setNodeEdges(
                        NodeEdgeSet const& edges
                    ) -> void;

    /* provide new IImageStreamInfo to overwrite the previously configured one. */
    virtual auto    replaceStreamInfo(
                        StreamId_T const streamId,
                        android::sp<IImageStreamInfo> pStreamInfo
                    ) -> void;

    /* provide stream buffer if existed */
    virtual auto    setAppImageStreamBuffer(
                        StreamId_T const streamId,
                        android::sp<IImageStreamInfo> pStreamInfo,
                        android::sp<IImageStreamBuffer> buffer
                    ) -> void;

    virtual auto    setAppImageStreamBuffer(
                        StreamId_T const streamId,
                        android::sp<IImageStreamBuffer> buffer
                    ) -> void;

    virtual auto    setHalImageStreamBuffer(
                        StreamId_T const streamId,
                        android::sp<IImageStreamBuffer> buffer
                    ) -> void;

    virtual auto    getAppMetaStreamBuffers(
                        std::vector<android::sp<IMetaStreamBuffer>>& out
                    ) const -> void;
    virtual auto    setAppMetaStreamBuffer(
                        StreamId_T const streamId,
                        android::sp<IMetaStreamBuffer> buffer
                    ) -> void;

    virtual auto    getHalMetaStreamBuffers(
                        std::vector<android::sp<IMetaStreamBuffer>>& out
                    ) const -> void;
    virtual auto    setHalMetaStreamBuffer(
                        StreamId_T const streamId,
                        android::sp<IMetaStreamBuffer> buffer
                    ) -> void;

    virtual auto    getFrameCallback() const -> android::wp<AppCallbackT> { return mpCallback; }
    virtual auto    updateFrameCallback(
                        android::wp<AppCallbackT> pCallback
                    ) -> void;

    // update physical camera setting.
    virtual auto    setPhysicalCameraSetting(
                        std::vector<int32_t> const& physicalCameraSetting
                    ) -> void;

    // update physical app metadata stream id list.
    virtual auto    setPhysicalAppMetaStreamIds(
                        std::unordered_map<uint32_t, std::vector<StreamId_T> > const*
                                                physicalAppMetaStreamIds
                    ) -> void;

    virtual auto    getEnabled(uint32_t which) const -> bool            { return mEnabledFlags.test(which); }
    virtual auto    setEnabled(uint32_t which, bool enabled) -> void    { mEnabledFlags.set(which, enabled); }

    virtual auto    getGroupFrameType() const -> GroupFrameTypeT        { return mGroupFrameType; }
    virtual auto    setGroupFrameType(GroupFrameTypeT type) -> void     { mGroupFrameType = type; }

    virtual auto    setTrackFrameResultParams(
                        std::shared_ptr<TrackFrameResultParams const>const& arg
                    ) -> void;

};


/******************************************************************************
 *
 ******************************************************************************/
class RequestBuilderImpl : public IRequestBuilder
{
public:     ////    Operations

    virtual auto    build(
                        android::sp<IPipelineContext> pPipelineContext,
                        std::vector<FrameBuilderT>const& builders
                    ) -> std::vector<PipelineFrameT>;

    virtual auto    build(
                        android::sp<IPipelineContext> pPipelineContext,
                        FrameBuilderT const& builder
                    ) -> PipelineFrameT;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSCam::v3::pipeline::NSPipelineContext
#endif  //_MTK_HARDWARE_MTKCAM3_PIPELINE_PIPELINE_REQUESTBUILDERIMPL_H_

