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

#ifndef _MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_UTILS_INCLUDE_IMPL_PIPELINEFRAMEBUILDER_H_
#define _MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_UTILS_INCLUDE_IMPL_PIPELINEFRAMEBUILDER_H_

#include <mtkcam3/pipeline/pipeline/IPipelineContextBuilder.h>

#include <impl/types.h>

#include <unordered_map>
#include <vector>

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace model {


/**
 * Unsed on the call buildPipelineFrame().
 */
struct BuildPipelineFrameInputParams
{
    using IOMapSet = NSCam::v3::pipeline::NSPipelineContext::IOMapSet;

    /**
     * Request no.
     */
    uint32_t const                              requestNo = 0;

    /**
     * Reprocess frame or not.
     */
    MBOOL const                                 bReprocessFrame = MFALSE;

    /**
     * Reprocess sensor timestamp.
     *
     * Definition:
     * https://developer.android.com/reference/android/hardware/camera2/CaptureResult#SENSOR_TIMESTAMP
     */
    int64_t                                     reprocessSensorTimestamp = 0;

    /**
     * App image stream info.
     */
    ParsedAppImageStreamInfo const*             pAppImageStreamInfo = nullptr;

    /**
     * App image stream buffers.
     */
    ParsedAppImageStreamBuffers const*          pAppImageStreamBuffers = nullptr;

    /**
     * App meta stream buffers.
     */
    std::vector<android::sp<IMetaStreamBuffer>> const*
                                                pAppMetaStreamBuffers = nullptr;

    /**
     * Hal image stream buffers.
     */
    std::vector<android::sp<IImageStreamBuffer>> const*
                                                pHalImageStreamBuffers = nullptr;

    /**
     * Hal meta stream buffers.
     */
    std::vector<android::sp<IMetaStreamBuffer>> const*
                                                pHalMetaStreamBuffers = nullptr;

    /**
     * Updated image stream info.
     */
    std::unordered_map<StreamId_T, android::sp<IImageStreamInfo>> const*
                                                pvUpdatedImageStreamInfo = nullptr;

    /**
     * IOMapSet for all pipeline nodes.
     */
    std::vector<NodeId_T> const*                pnodeSet = nullptr;
    std::unordered_map<NodeId_T, IOMapSet>const*pnodeIOMapImage = nullptr;
    std::unordered_map<NodeId_T, IOMapSet>const*pnodeIOMapMeta = nullptr;

    /**
     * The root nodes of a pipeline.
     */
    NSPipelineContext::NodeSet const*           pRootNodes = nullptr;

    /**
     * The edges to connect pipeline nodes.
     */
    NSPipelineContext::NodeEdgeSet const*       pEdges = nullptr;

    /**
     * IFrameBuilder's callback.
     */
    android::wp<NSPipelineContext::IFrameBuilder::AppCallbackT> const
                                                pCallback = nullptr;

    /**
     * Physical metadata stream id list.
     */
    std::unordered_map<uint32_t, std::vector<StreamId_T> > const*
                                                physicalMetaStreamIds;

    /**
     * Pipeline context (deprecated)
     */
    android::sp<NSPipelineContext::IPipelineContext> const
                                                pPipelineContext = nullptr;

    /**
     * Pipeline context
     */
    std::vector<int32_t> const*                 vPhysicalCameraSetting;

};


/**
 * Generate a frame builder based on the given input.
 *
 * @param[in] in: input parameters.
 *
 * @return
 *      A non-nullptr frame builder indicates success; otherwise failure.
 */
auto makeFrameBuilder(
    BuildPipelineFrameInputParams const& in
) -> std::shared_ptr<NSPipelineContext::IFrameBuilder>;


/**
 * Generate a pipeline frames.
 *
 * @param[out] out: a new-created pipeline frame.
 *
 * @param[in] in: a given frame builder.
 *
 * @param[in] pPipelineContext: the pipeline context which all successfully-built
 *  pipeline frames are assocaited with.
 *
 * @return
 *      0 indicates success; otherwise failure.
 */
auto makePipelineFrame(
    android::sp<IPipelineFrame>& out,
    std::shared_ptr<NSPipelineContext::IFrameBuilder> const& in,
    android::sp<NSPipelineContext::IPipelineContext> pPipelineContext
) -> int;


/**
 * Generate a set of pipeline frames with contiguous frame numbers.
 *
 * @param[out] out: a set of new-created pipeline frames.
 *
 * @param[in] in: a given set of frame builders.
 *
 * @param[in] pPipelineContext: the pipeline context which all successfully-built
 *  pipeline frames are assocaited with.
 *
 * @return
 *      0 indicates success; otherwise failure.
 */
auto makePipelineFrame(
    std::vector<android::sp<IPipelineFrame>>& out,
    std::vector<std::shared_ptr<NSPipelineContext::IFrameBuilder>>const& in,
    android::sp<NSPipelineContext::IPipelineContext> pPipelineContext
) -> int;


/**
 * Generate a new pipeline frame.
 *
 * @param[out] out: a new-created pipeline frame.
 *
 * @param[in] in: input parameters.
 *
 * @param[in] pPipelineContext: the pipeline context which the successfully-built
 *  pipeline frame are assocaited with.
 *
 * @return
 *      0 indicates success; otherwise failure.
 */
auto buildPipelineFrame(
    android::sp<IPipelineFrame>& out,
    BuildPipelineFrameInputParams const& in,
    android::sp<NSPipelineContext::IPipelineContext> pPipelineContext = nullptr
) -> int;


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace model
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_UTILS_INCLUDE_IMPL_PIPELINEFRAMEBUILDER_H_

