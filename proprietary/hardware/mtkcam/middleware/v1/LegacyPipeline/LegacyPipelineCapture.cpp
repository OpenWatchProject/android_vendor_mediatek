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

#define LOG_TAG "MtkCam/LPipelineCap"

//
#include "MyUtils.h"
#include "PipelineBuilderCapture.h"
#include "LegacyPipelineCapture.h"

#include <mtkcam/pipeline/utils/streambuf/StreamBuffers.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/pipeline/hwnode/P1Node.h>
#include <mtkcam/pipeline/hwnode/P2Node.h>
#include <mtkcam/pipeline/hwnode/JpegNode.h>

using namespace android;
using namespace NSCam;
using namespace NSCam::Utils;
using namespace NSCam::v3;
using namespace NSCam::v3::NSPipelineContext;
using namespace NSCam::v1;
using namespace NSCam::v1::NSLegacyPipeline;

typedef NSCam::v3::Utils::HalMetaStreamBuffer::Allocator  HalMetaStreamBufferAllocatorT;

/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)
//
#define MY_LOGD1(...)               MY_LOGD_IF((mLogLevel>=1),__VA_ARGS__)
#define MY_LOGD2(...)               MY_LOGD_IF((mLogLevel>=2),__VA_ARGS__)
#define MY_LOGD3(...)               MY_LOGD_IF((mLogLevel>=3),__VA_ARGS__)
//
#define FUNC_START                  MY_LOGD1("+")
#define FUNC_END                    MY_LOGD1("-")
//
/******************************************************************************
*
*******************************************************************************/
LegacyPipelineCapture::
LegacyPipelineCapture(MINT32 openId)
{
    mLogLevel = 1;
    mOpenId = openId;
}


/******************************************************************************
*
*******************************************************************************/
LegacyPipelineCapture::
~LegacyPipelineCapture()
{}


/******************************************************************************
*
*******************************************************************************/
MERROR
LegacyPipelineCapture::
submitSetting(
    MINT32    const requestNo,
    IMetadata& appMeta,
    IMetadata& halMeta,
    ResultSet* pResultSet
)
{
    CAM_TRACE_FMT_BEGIN("CaptureSubmitSetting No%d",requestNo);
    sp<PipelineContext> pPipelineContext;
    sp<IPipelineFrame> pFrame;
    {
    Mutex::Autolock _l(mLock);
    //
    pPipelineContext = mpPipelineContext;
    if( !pPipelineContext.get() )
    {
        MY_LOGW("pPipelineContext is nullptr, skip this time");
        CAM_TRACE_FMT_END();
        return OK;
    }
    //
    sp<RequestBuilder> pRequestBuilder = mpRequestBuilder;
    if( !pRequestBuilder.get() )
    {
        MY_LOGW("pRequestBuilder is nullptr, skip this time");
        CAM_TRACE_FMT_END();
        return OK;
    }
    //
    if( !trySetMetadata<MINT32>(halMeta, MTK_PIPELINE_UNIQUE_KEY, mTimestamp) )
        MY_LOGE("set unique key failed");

    sp<HalMetaStreamBuffer> pHalMetaStreamBuffer =
           HalMetaStreamBufferAllocatorT(
                queryMetaStreamInfo(eSTREAMID_META_HAL_CONTROL).get()
           )(halMeta);

    sp<IMetaStreamBuffer> pAppMetaStreamBuffer =
           HalMetaStreamBufferAllocatorT(
                queryMetaStreamInfo(eSTREAMID_META_APP_CONTROL).get()
           )(appMeta);

    pRequestBuilder->setMetaStreamBuffer(
                eSTREAMID_META_APP_CONTROL,
                pAppMetaStreamBuffer
                );
    pRequestBuilder->setMetaStreamBuffer(
                eSTREAMID_META_HAL_CONTROL,
                pHalMetaStreamBuffer
                );

    if( pResultSet )
    {
        for( size_t i = 0; i < pResultSet->size(); i++ )
        {
            StreamId_T const streamId = pResultSet->keyAt(i);
            sp<IMetaStreamBuffer> pMetaStreamBuffer =
                HalMetaStreamBufferAllocatorT(
                        queryMetaStreamInfo(streamId).get()
                        )(pResultSet->valueAt(i));
            //
            pRequestBuilder->setMetaStreamBuffer(
                    streamId, pMetaStreamBuffer
                    );
        }
    }

    pFrame = pRequestBuilder->build(requestNo, pPipelineContext);

    if( ! pFrame.get() ) {
        MY_LOGE("build request failed");
        CAM_TRACE_FMT_END();
        return UNKNOWN_ERROR;
    }
    }
    pPipelineContext->queue(pFrame);

    CAM_TRACE_FMT_END();
    return OK;
}

/******************************************************************************
*
*******************************************************************************/
static
MERROR
wait_and_get_buffer(
    MINT32 const             requestNo,
    sp<StreamBufferProvider> pProvider,
    RequestBuilder&          builder
)
{
    if ( pProvider == nullptr ) {
        CAM_LOGE("Null provider.");
        return UNKNOWN_ERROR;
    }
    //
    sp<HalImageStreamBuffer> buffer;
    pProvider->dequeStreamBuffer(
        requestNo,
        pProvider->queryImageStreamInfo(),
        buffer
    );

    if ( buffer == nullptr ) {
        sp<IImageStreamInfo> pImgStreamInfo = pProvider->queryImageStreamInfo();
        CAM_LOGW("Null HalImageStreamBuffer name(%s) streamId(0x%x).",
                    pImgStreamInfo->getStreamName(),
                    pImgStreamInfo->getStreamId());
        return UNKNOWN_ERROR;
    }
    builder.setImageStreamBuffer(
            pProvider->queryImageStreamInfo()->getStreamId(),
            buffer
        );
    //
    return OK;
}

/******************************************************************************
*
*******************************************************************************/
MERROR
LegacyPipelineCapture::
submitRequest(
    MINT32    const   requestNo,
    IMetadata&        appMeta,
    IMetadata&        halMeta,
    Vector<BufferSet> vDstStreams,
    ResultSet* pResultSet
)
{
    MY_LOGD_IF( 1, "submitRequest %d vDstStreams:%zu +", requestNo, vDstStreams.size());
    CAM_TRACE_FMT_BEGIN("LP:[Cap]submitRequest No%d",requestNo);
    sp<PipelineContext> pPipelineContext;
    sp<IPipelineFrame> pFrame;
    {
    Mutex::Autolock _l(mLock);
    //
    pPipelineContext = mpPipelineContext;
    if( !pPipelineContext.get() )
    {
        MY_LOGW("pPipelineContext is nullptr, skip this time");
        CAM_TRACE_FMT_END();
        return OK;
    }
    //
    RequestBuilder aRequestBuilder;
    sp<HalMetaStreamBuffer> pHalMetaStreamBuffer =
           HalMetaStreamBufferAllocatorT(
                queryMetaStreamInfo(eSTREAMID_META_HAL_CONTROL).get()
           )(halMeta);

    sp<IMetaStreamBuffer> pAppMetaStreamBuffer =
           HalMetaStreamBufferAllocatorT(
                queryMetaStreamInfo(eSTREAMID_META_APP_CONTROL).get()
           )(appMeta);

    // p1 node
    android::sp< NodeActor<NSCam::v3::P1Node> > nodeActor = NULL;
    MERROR err = pPipelineContext->queryNodeActor(eNODEID_P1Node, nodeActor);
    MBOOL aNeedP1 = (err == OK);
    if ( aNeedP1 ) {
        IOMap p1_Image_IOMap;
        for ( size_t i = 0; i < vDstStreams.size(); ++i ) {
            switch( vDstStreams[i].streamId ) {
                case eSTREAMID_IMAGE_PIPE_RAW_OPAQUE:
                case eSTREAMID_IMAGE_PIPE_RAW_RESIZER:
                case eSTREAMID_IMAGE_PIPE_RAW_LCSO:
                    aNeedP1 = true;
                    p1_Image_IOMap.addOut(vDstStreams[i].streamId);
                    break;
                default:
                    break;
            };
        }
        if ( aNeedP1 ) {
            aRequestBuilder.setIOMap(
                    eNODEID_P1Node,
                    IOMapSet().add(
                        p1_Image_IOMap
                        ),
                    IOMapSet().add(IOMap()
                        .addIn(mConfiguration.pControlMeta_App->getStreamId())
                        .addIn(mConfiguration.pControlMeta_Hal->getStreamId())
                        .addOut(mConfiguration.pResultMeta_P1_App->getStreamId())
                        .addOut(mConfiguration.pResultMeta_P1_Hal->getStreamId())
                        )
                    );
            aRequestBuilder.setRootNode(
                NodeSet().add(eNODEID_P1Node)
                );
        } else {
            MY_LOGE("request:%d No P1 node. preview pipeline does not support memory in.", requestNo);
            CAM_TRACE_FMT_END();
            return BAD_VALUE;
        }
    }
    // p2 node
    MBOOL aNeedP2       = false;
    {
        MBOOL aHasResizeRaw = false;
        MBOOL aHasLcso      = false;
        MBOOL aHasFullRaw   = false;
        IOMap p2_Image_IOMap;
        for ( size_t i = 0; i < vDstStreams.size(); ++i ) {
            switch( vDstStreams[i].streamId ) {
                case eSTREAMID_IMAGE_PIPE_YUV_JPEG:
                case eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL:
                case eSTREAMID_IMAGE_PIPE_YUV_00:
                case eSTREAMID_IMAGE_PIPE_YUV_01:
                case eSTREAMID_IMAGE_YUV_FD:
                    aNeedP2 = true;
                    p2_Image_IOMap.addOut(vDstStreams[i].streamId);
                    break;
                case eSTREAMID_IMAGE_PIPE_RAW_RESIZER:
                    aNeedP2 = true;
                    aHasResizeRaw = true;
                    break;
                case eSTREAMID_IMAGE_PIPE_RAW_LCSO:
                    aNeedP2 = true;
                    aHasLcso = true;
                    break;
                case eSTREAMID_IMAGE_PIPE_RAW_OPAQUE:
                    aNeedP2 = true;
                    aHasFullRaw = true;
                    break;
                default:
                    break;
            };
        }
        if ( aNeedP2 ) {
            if ( aHasLcso ) p2_Image_IOMap.addIn(eSTREAMID_IMAGE_PIPE_RAW_LCSO);
            if ( aHasResizeRaw ) p2_Image_IOMap.addIn(eSTREAMID_IMAGE_PIPE_RAW_RESIZER);
            if ( aHasFullRaw )   p2_Image_IOMap.addIn(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE);

            if (!aHasFullRaw && !aHasResizeRaw)
            {
                MY_LOGW("request:%d No resized and Full raw as p2 input, something wrong ?", requestNo);
            }

            aRequestBuilder.setIOMap(
                eNODEID_P2Node,
                IOMapSet().add(p2_Image_IOMap),
                IOMapSet().add(IOMap()
                    .addIn(mConfiguration.pControlMeta_App->getStreamId())
                    .addIn(mConfiguration.pResultMeta_P1_App->getStreamId())
                    .addIn(mConfiguration.pResultMeta_P1_Hal->getStreamId())
                    .addOut(mConfiguration.pResultMeta_P2_App->getStreamId())
                    .addOut(mConfiguration.pResultMeta_P2_Hal->getStreamId())
                    )
                );
            if ( aNeedP1 )
                aRequestBuilder.setNodeEdges(
                    NodeEdgeSet().addEdge(eNODEID_P1Node, eNODEID_P2Node)
                    );
            else
                aRequestBuilder.setRootNode(
                    NodeSet().add(eNODEID_P2Node)
                    );
        } else {
            MY_LOGD("request:%d No P2 node", requestNo);
        }
    }
    // Jpeg node
    if (aNeedP2)
    {
        MBOOL aHasYuvJpeg      = false;
        MBOOL aHasYuvThumbnail = false;
        MBOOL aHasJpeg         = false;
        IOMap jpeg_Image_IOMap;
        for ( size_t i = 0; i < vDstStreams.size(); ++i ) {
            switch( vDstStreams[i].streamId ) {
                case eSTREAMID_IMAGE_JPEG:
                    aHasJpeg = true;
                    break;
                case eSTREAMID_IMAGE_PIPE_YUV_JPEG:
                    aHasYuvJpeg = true;
                    break;
                case eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL:
                    aHasYuvThumbnail = true;
                    break;
                default:
                    break;
            };
        }
        if ( aHasJpeg && aHasYuvJpeg ) {
            if ( aHasYuvJpeg )      jpeg_Image_IOMap.addIn(eSTREAMID_IMAGE_PIPE_YUV_JPEG);
            if ( aHasYuvThumbnail ) jpeg_Image_IOMap.addIn(eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL);
            if ( aHasJpeg )         jpeg_Image_IOMap.addOut(eSTREAMID_IMAGE_JPEG);

            aRequestBuilder.setIOMap(
                eNODEID_JpegNode,
                IOMapSet().add(jpeg_Image_IOMap),
                IOMapSet().add(IOMap()
                    .addIn(mConfiguration.pControlMeta_App->getStreamId())
                    .addIn(mConfiguration.pResultMeta_P2_Hal->getStreamId())
                    .addOut(mConfiguration.pResultMeta_Jpeg_App->getStreamId())
                    )
                );

            NodeEdgeSet edges;
            if( aNeedP1 ) edges.addEdge(eNODEID_P1Node, eNODEID_P2Node);
            if( aNeedP2 ) edges.addEdge(eNODEID_P2Node, eNODEID_JpegNode);
            aRequestBuilder.setNodeEdges(edges);
        } else {
            MY_LOGD("request:%d No need Jpeg node", requestNo);
        }
    }

    // get critical buffer when building request
    for ( auto &e : vDstStreams) {
        if ( e.criticalBuffer) {
            wait_and_get_buffer( requestNo, e.provider.promote(), aRequestBuilder );
        }
    }

    if( pResultSet )
    {
        for( size_t i = 0; i < pResultSet->size(); i++ )
        {
            StreamId_T const streamId = pResultSet->keyAt(i);
            sp<IMetaStreamBuffer> pMetaStreamBuffer =
                HalMetaStreamBufferAllocatorT(
                        queryMetaStreamInfo(streamId).get()
                        )(pResultSet->valueAt(i));
            //
            aRequestBuilder.setMetaStreamBuffer(
                    streamId, pMetaStreamBuffer
                    );
        }
    }
    sp<ResultProcessor> pResultProcessor = mpResultProcessor;
    aRequestBuilder.updateFrameCallback(pResultProcessor);
    if( !pResultProcessor.get() ) {
        MY_LOGW("get ResultProcessor failed!");
    }

    pFrame =
        aRequestBuilder
        .setMetaStreamBuffer(
                eSTREAMID_META_APP_CONTROL,
                pAppMetaStreamBuffer
                )
        .setMetaStreamBuffer(
                eSTREAMID_META_HAL_CONTROL,
                pHalMetaStreamBuffer
                )
        .build(requestNo, pPipelineContext);

    if( ! pFrame.get() ) {
        MY_LOGE("build request failed");
        CAM_TRACE_FMT_END();
        return UNKNOWN_ERROR;
    }
    }
    pPipelineContext->queue(pFrame);
    //
    MY_LOGD_IF( 1, "submitRequest %d vDstStreams:%zu -", requestNo, vDstStreams.size());
    CAM_TRACE_FMT_END();
    return OK;
}


/******************************************************************************
*
*******************************************************************************/
MVOID
LegacyPipelineCapture::
setConfiguration(
    Configuration const& config
)
{
    CAM_TRACE_NAME("CaptureSetConfig");
    mConfiguration = config;
    //
    struct update_stream
    {
        MVOID   operator()(LegacyPipelineCapture* lp, sp<IMetaStreamInfo> pInfo) {
                    if( pInfo != 0 ) { lp->setMetaStreamInfo(pInfo); }
                }
        MVOID   operator()(LegacyPipelineCapture* lp, sp<IImageStreamInfo> pInfo) {
                    if( pInfo != 0 ) { lp->setImageStreamInfo(pInfo); }
                }
    };
    // in
    update_stream()(this, config.pControlMeta_App);
    update_stream()(this, config.pControlMeta_Hal);
    update_stream()(this, config.pImage_ImgoRawSrc);

    // out
    update_stream()(this, config.pResultMeta_P1_App);
    update_stream()(this, config.pResultMeta_P1_Hal);
    update_stream()(this, config.pResultMeta_P2_App);
    update_stream()(this, config.pResultMeta_P2_Hal);
    update_stream()(this, config.pResultMeta_Jpeg_App);

    update_stream()(this, config.pImage_RrzoRaw);
    update_stream()(this, config.pImage_ImgoRaw);
    update_stream()(this, config.pImage_LcsoRaw);
    update_stream()(this, config.pImage_Yuv00);
    update_stream()(this, config.pImage_YuvJpeg);
    update_stream()(this, config.pImage_YuvThumbnail);
    update_stream()(this, config.pImage_Jpeg);
}

/******************************************************************************
*
*******************************************************************************/
MERROR
LegacyPipelineCapture::
waitUntilDrainedAndFlush()
{
    MERROR err = OK;
    sp<PipelineContext> pPipelineContext = mpPipelineContext;
    if( !pPipelineContext.get() )
    {
        MY_LOGW("get pPipelineContext fail");
        return UNKNOWN_ERROR;
    }

    //P1
    {
        android::sp< NodeActor<NSCam::v3::P1Node> > nodeActor = NULL;
        err = waitUntilNodeDrainedAndFlush(eNODEID_P1Node, nodeActor);
        if( err != OK )
        {
            MY_LOGW("get wait until node(%d) drained and flush fail", eNODEID_P1Node);
            return err;
        }
    }
    //P2
    {
        android::sp< NodeActor<NSCam::v3::P2Node> > nodeActor = NULL;
        err = waitUntilNodeDrainedAndFlush(eNODEID_P2Node, nodeActor);
        if( err != OK )
        {
            MY_LOGW("get wait until node(%d) drained and flush fail", eNODEID_P2Node);
            return err;
        }
    }
    //Jpeg
    {
        android::sp< NodeActor<NSCam::v3::JpegNode> > nodeActor = NULL;
        err = waitUntilNodeDrainedAndFlush(eNODEID_JpegNode, nodeActor);
        if( err != OK )
        {
            MY_LOGW("get wait until node(%d) drained and flush fail", eNODEID_JpegNode);
            return err;
        }
    }

    return err;
}

/******************************************************************************
*
*******************************************************************************/
MERROR
LegacyPipelineCapture::
waitUntilP1DrainedAndFlush()
{
    MERROR err = OK;
    sp<PipelineContext> pPipelineContext = mpPipelineContext;
    if( !pPipelineContext.get() )
    {
        MY_LOGW("get pPipelineContext fail");
        return UNKNOWN_ERROR;
    }
    MY_LOGD("waitUntilP1DrainedAndFlush");

    //P1
    {
        android::sp< NodeActor<NSCam::v3::P1Node> > nodeActor = NULL;
        err = waitUntilNodeDrainedAndFlush(eNODEID_P1Node, nodeActor);
        if( err != OK )
        {
            MY_LOGW("get wait until node(%d) drained and flush fail", eNODEID_P1Node);
            return err;
        }
    }

    return err;
}

/******************************************************************************
*
*******************************************************************************/
MERROR
LegacyPipelineCapture::
waitUntilP2JpegDrainedAndFlush()
{
    MERROR err = OK;
    sp<PipelineContext> pPipelineContext = mpPipelineContext;
    if( !pPipelineContext.get() )
    {
        MY_LOGW("get pPipelineContext fail");
        return UNKNOWN_ERROR;
    }
    MY_LOGD("waitUntilP2JpegDrainedAndFlush");

    //P2
    {
        android::sp< NodeActor<NSCam::v3::P2Node> > nodeActor = NULL;
        err = waitUntilNodeDrainedAndFlush(eNODEID_P2Node, nodeActor);
        if( err != OK )
        {
            MY_LOGW("get wait until node(%d) drained and flush fail", eNODEID_P2Node);
            return err;
        }
    }
    //Jpeg
    {
        android::sp< NodeActor<NSCam::v3::JpegNode> > nodeActor = NULL;
        err = waitUntilNodeDrainedAndFlush(eNODEID_JpegNode, nodeActor);
        if( err != OK )
        {
            MY_LOGW("get wait until node(%d) drained and flush fail", eNODEID_JpegNode);
            return err;
        }
    }

    return err;
}


/******************************************************************************
*
*******************************************************************************/
template <typename _Node_>
MERROR
LegacyPipelineCapture::
waitUntilNodeDrainedAndFlush(
    NodeId_T const nodeId,
    android::sp< NodeActor<_Node_> >& pNodeActor)
{
    sp<PipelineContext> pPipelineContext = mpPipelineContext;
    if( !pPipelineContext.get() )
    {
        MY_LOGW("get pPipelineContext fail");
        return UNKNOWN_ERROR;
    }

    MERROR err = pPipelineContext->queryNodeActor(nodeId, pNodeActor);
    if( err != NAME_NOT_FOUND )
    {
        if( err != OK || pNodeActor == NULL )
        {
            MY_LOGW("get NodeActor(%" PRIdPTR ") fail", nodeId);
            return err;
        }
        //
        err = pPipelineContext->waitUntilNodeDrained(nodeId);
        if( err != OK )
        {
            MY_LOGW("wait until node(%" PRIdPTR ") drained fail", nodeId);
            return err;
        }
        //
        IPipelineNode* node = pNodeActor->getNode();
        if( node == NULL )
        {
            MY_LOGW("get node(%" PRIdPTR ") fail", nodeId);
            return UNKNOWN_ERROR;
        }
        //
        err = node->flush();
        if( err != OK )
        {
            MY_LOGW("flush node(%" PRIdPTR ") fail", nodeId);
            return err;
        }
    }
    return OK;
}
