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

#define LOG_TAG "MtkCam/StereoLPipelinePrv"
//
#include "MyUtils.h"
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Trace.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/ContextBuilder/ImageStreamManager.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/ContextBuilder/MetaStreamManager.h>
#include "StereoLegacyPipeline.h"
#include <mtkcam/pipeline/utils/streambuf/StreamBuffers.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/postprocessor/TableDefine.h>
#include <mtkcam/middleware/v1/LegacyPipeline/postprocessor/ThirdParty/table_data_set.h>
//
using namespace android;
using namespace NSCam;
using namespace NSCam::Utils;
using namespace NSCam::v3;
using namespace NSCam::v3::NSPipelineContext;
using namespace NSCam::v1;
using namespace NSCam::v1::NSLegacyPipeline;

typedef NSCam::v3::Utils::HalMetaStreamBuffer::Allocator  HalMetaStreamBufferAllocatorT;

std::vector<android::NSPostProc::FrameDataTableSet> android::NSPostProc::gFrameDataTableSet;
/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)

#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#define FUNC_START                  MY_LOGD("+")
#define FUNC_END                    MY_LOGD("-")
/******************************************************************************
*
*******************************************************************************/
StereoLegacyPipeline::
StereoLegacyPipeline(
    MINT32 const openId,
    MINT32 const mode
) : miOpenId(openId),
    miMode(mode)
{
    mTimestamp = TimeTool::getReadableTime();
}
/******************************************************************************
*
*******************************************************************************/
StereoLegacyPipeline::
~StereoLegacyPipeline()
{}
/******************************************************************************
*
*******************************************************************************/
sp<StereoLegacyPipeline>
StereoLegacyPipeline::
createInstance(
    MINT32 const openId,
    MINT32 const mode
)
{
    return new StereoLegacyPipeline(openId, mode);
}
/******************************************************************************
*
*******************************************************************************/
MVOID
StereoLegacyPipeline::
onLastStrongRef( const void* /*id*/)
{
    FUNC_START;

    if ( mpPipelineContext != 0 ){
        mpPipelineContext->flush();
        mpPipelineContext->waitUntilDrained();
        mpPipelineContext = NULL;
    }
    if(mpResultProcessor != nullptr)
    {
        mpResultProcessor->flush();
    }
    //
    mpPipelineContext       = NULL;
    mpResultProcessor       = NULL;
    mpTimestampProcessor    = NULL;
    mpRequestBuilder        = NULL;
    mpImageStreamManager    = NULL;
    mpMetaStreamManager     = NULL;
    mpNodeConfigDataManager = NULL;
    //

    FUNC_END;
}

/******************************************************************************
*
*******************************************************************************/
MERROR
StereoLegacyPipeline::
submitSetting(
    MINT32    const requestNo,
    IMetadata& appMeta,
    IMetadata& halMeta,
    ResultSet* /*pResultSet*/
)
{
    MINT32 uniqueKey = -1;
    if( IMetadata::getEntry<MINT32>(&halMeta, MTK_PIPELINE_UNIQUE_KEY, uniqueKey) ){
        MY_LOGD_IF(0, "unique key already assigned(%d)", uniqueKey);
    }else{
        MY_LOGD_IF(0, "set unique key(%d)", mTimestamp);
        if( !trySetMetadata<MINT32>(halMeta, MTK_PIPELINE_UNIQUE_KEY, mTimestamp) ){
           MY_LOGE("set unique key failed");
        }
    }

    sp<HalMetaStreamBuffer> pHalMetaStreamBuffer =
           HalMetaStreamBufferAllocatorT(
                queryMetaStreamInfo(mHalMetaStreamId).get()
           )(halMeta);

    sp<IMetaStreamBuffer> pAppMetaStreamBuffer =
           HalMetaStreamBufferAllocatorT(
                queryMetaStreamInfo(mAppMetaStreamId).get()
           )(appMeta);

    mpRequestBuilder->setMetaStreamBuffer(
                mAppMetaStreamId,
                pAppMetaStreamBuffer
                );
    mpRequestBuilder->setMetaStreamBuffer(
                mHalMetaStreamId,
                pHalMetaStreamBuffer
                );
    sp<IPipelineFrame> pFrame =
        mpRequestBuilder->build(requestNo, mpPipelineContext);

    if( ! pFrame.get() ) {
        MY_LOGE("build request failed");
        return UNKNOWN_ERROR;
    }

    mpPipelineContext->queue(pFrame);

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
StereoLegacyPipeline::
submitRequest(
    MINT32    const     requestNo,
    IMetadata&          appMeta,
    IMetadata&          halMeta,
    Vector<BufferSet>   vDstStreams,
    ResultSet* pResultSet
)
{
    MY_LOGD("submitRequest reqId(%d) +", requestNo);
    CAM_TRACE_FMT_BEGIN("LP:[Prv]submitRequest No%d",requestNo);

    MINT32 uniqueKey = -1;
    if( IMetadata::getEntry<MINT32>(&halMeta, MTK_PIPELINE_UNIQUE_KEY, uniqueKey) ){
        MY_LOGD_IF(1, "unique key already assigned(%d)", uniqueKey);
    }else{
        MY_LOGD_IF(1, "set unique key(%d)", mTimestamp);
        if( !trySetMetadata<MINT32>(halMeta, MTK_PIPELINE_UNIQUE_KEY, mTimestamp) ){
           MY_LOGE("set unique key failed");
        }
    }

    RequestBuilder aRequestBuilder;

    sp<HalMetaStreamBuffer> pHalMetaStreamBuffer =
        HalMetaStreamBufferAllocatorT(
            queryMetaStreamInfo(mHalMetaStreamId).get()
        )(halMeta);

    sp<IMetaStreamBuffer> pAppMetaStreamBuffer =
        HalMetaStreamBufferAllocatorT(
            queryMetaStreamInfo(mAppMetaStreamId).get()
        )(appMeta);

    if(mvNodeConfigDataManagers.size() > 0){
        bool isRaw16Cap = false;
        // Decide whether is DNG capture
        for ( size_t i = 0; i < vDstStreams.size(); ++i ) {
            if(vDstStreams[i].streamId == eSTREAMID_IMAGE_PIPE_RAW16){
                isRaw16Cap = true;
                break;
            }
        }

        // identify NodeConfigDataManagers
        auto isContainNode = [](sp<NodeConfigDataManager> mgr, NodeId_T id)
        {
            for(size_t i=0; i<mgr->getSize(); ++i){
                NodeId_T nodeId = mgr->getNodeId(i);
                if(id == mgr->getNodeId(i)){
                    return true;
                }
            }
            return false;
        };
        sp<NodeConfigDataManager> aNodeConfigDataManager_cap = nullptr;
        sp<NodeConfigDataManager> aNodeConfigDataManager_dng_cap = nullptr;
        for ( size_t i = 0; i < mvNodeConfigDataManagers.size(); ++i ) {
            if(isContainNode(mvNodeConfigDataManagers[i], eNODEID_RAW16Out)){
                aNodeConfigDataManager_dng_cap = mvNodeConfigDataManagers[i];
            }
            else{
                aNodeConfigDataManager_cap = mvNodeConfigDataManagers[i];
            }
        }

        // choose the one to create RequestBuilder
        sp<NodeConfigDataManager> aNodeConfigDataManager_chosen = nullptr;
        if(isRaw16Cap){
            MY_LOGD("create a new stereo DNG capture request");
            aNodeConfigDataManager_chosen = aNodeConfigDataManager_dng_cap;
        }
        else{
            MY_LOGD("create a new stereo capture request");
            aNodeConfigDataManager_chosen = aNodeConfigDataManager_cap;
        }

        for(size_t i=0; i<aNodeConfigDataManager_chosen->getSize(); i++){
            NodeId_T nodeId = aNodeConfigDataManager_chosen->getNodeId(i);
            MY_LOGD("nodeId(0x%x)", nodeId);
            aRequestBuilder.setIOMap(
                nodeId,
                aNodeConfigDataManager_chosen->getImageIOMap(nodeId),
                aNodeConfigDataManager_chosen->getMetaIOMap(nodeId)
            );
        }

        aRequestBuilder.setRootNode(
            aNodeConfigDataManager_chosen->getRootNodeId()
        );
        aRequestBuilder.setNodeEdges(
            aNodeConfigDataManager_chosen->getNodeEdgeSet()
        );
        aRequestBuilder.updateFrameCallback(mpResultProcessor);

    }else{
        MY_LOGE("extra request types are not set!");
        return UNKNOWN_ERROR;
    }

    // check whether the stream is critical(need to setImageStreamBuffer here)
    MBOOL bP1Request = MFALSE;
    IOMap p1_Image_IOMap;
    for ( size_t i = 0; i < vDstStreams.size(); ++i ) {
        if ( vDstStreams[i].criticalBuffer ) {
            wait_and_get_buffer( requestNo, queryProvider(vDstStreams[i].streamId), aRequestBuilder );
        }
        if( vDstStreams[i].streamId == eSTREAMID_IMAGE_PIPE_RAW_OPAQUE ||
            vDstStreams[i].streamId == eSTREAMID_IMAGE_PIPE_RAW_RESIZER){
            bP1Request = MTRUE;
            p1_Image_IOMap.addOut(vDstStreams[i].streamId);
        }
    }

    if(bP1Request){
        MY_LOGD("bP1Request relace original request");
        aRequestBuilder.setIOMap(
            eNODEID_P1Node,
            IOMapSet().add(
                p1_Image_IOMap
            ),
            vIOMapInfo.valueFor(eNODEID_P1Node)
        );
    }

    sp<IPipelineFrame> pFrame =
        aRequestBuilder
        .setMetaStreamBuffer(
                mAppMetaStreamId,
                pAppMetaStreamBuffer
                )
        .setMetaStreamBuffer(
                mHalMetaStreamId,
                pHalMetaStreamBuffer
                )
        .build(requestNo, mpPipelineContext);

    if( ! pFrame.get() ) {
        MY_LOGE("build request failed");
        CAM_TRACE_FMT_END();
        return UNKNOWN_ERROR;
    }

    mpPipelineContext->queue(pFrame);
    //
    MY_LOGD("submitRequest -");
    MY_LOGD_IF( 1, "submitRequest %d vDstStreams:%d -", requestNo, vDstStreams.size());
    CAM_TRACE_FMT_END();

    return OK;
}

/******************************************************************************
*
*******************************************************************************/
MERROR
StereoLegacyPipeline::
submitRequest(
    MINT32    const     requestNo,
    IMetadata&          appMeta,
    IMetadata&          halMeta,
    Vector<BufferSet>   vDstStreams,
    MINT32 type
)
{
    MY_LOGD("submitRequest reqId(%d) +", requestNo);
    CAM_TRACE_FMT_BEGIN("LP:[Prv]submitRequest No%d",requestNo);

    MINT32 uniqueKey = -1;
    if( IMetadata::getEntry<MINT32>(&halMeta, MTK_PIPELINE_UNIQUE_KEY, uniqueKey) ){
        MY_LOGD_IF(1, "unique key already assigned(%d)", uniqueKey);
    }else{
        MY_LOGD_IF(1, "set unique key(%d)", mTimestamp);
        if( !trySetMetadata<MINT32>(halMeta, MTK_PIPELINE_UNIQUE_KEY, mTimestamp) ){
           MY_LOGE("set unique key failed");
        }
    }

    RequestBuilder aRequestBuilder;

    sp<HalMetaStreamBuffer> pHalMetaStreamBuffer =
        HalMetaStreamBufferAllocatorT(
            queryMetaStreamInfo(mHalMetaStreamId).get()
        )(halMeta);

    sp<IMetaStreamBuffer> pAppMetaStreamBuffer =
        HalMetaStreamBufferAllocatorT(
            queryMetaStreamInfo(mAppMetaStreamId).get()
        )(appMeta);

    // choose the one to create RequestBuilder
    sp<NodeConfigDataManager> aNodeConfigDataManager_chosen = nullptr;
    // todo: may can add type to node config data. and just check this value.
    for(MUINT32 i = 0;i < android::NSPostProc::gFrameDataTableSet.size(); i++)
    {
        if((MINT32)android::NSPostProc::gFrameDataTableSet[i].type == type)
        {
            for(MUINT32 j = 0;j < mvNodeConfigDataManagers.size(); j++)
            {
                if(mvNodeConfigDataManagers[j]->getName() == android::NSPostProc::gFrameDataTableSet[i].name)
                {
                    aNodeConfigDataManager_chosen = mvNodeConfigDataManagers[j];
                    MY_LOGD("tbl name(%s)", android::NSPostProc::gFrameDataTableSet[i].name.c_str());
                }
            }
        }
    }
    if(aNodeConfigDataManager_chosen == nullptr)
    {
        MY_LOGE("should not happened! tpye(%d)", type);
        return UNKNOWN_ERROR;
    }

    for(size_t i=0; i<aNodeConfigDataManager_chosen->getSize(); i++){
        NodeId_T nodeId = aNodeConfigDataManager_chosen->getNodeId(i);
        MY_LOGD("nodeId(0x%x)", nodeId);
        aRequestBuilder.setIOMap(
            nodeId,
            aNodeConfigDataManager_chosen->getImageIOMap(nodeId),
            aNodeConfigDataManager_chosen->getMetaIOMap(nodeId)
        );
    }

    aRequestBuilder.setRootNode(
        aNodeConfigDataManager_chosen->getRootNodeId()
    );
    aRequestBuilder.setNodeEdges(
        aNodeConfigDataManager_chosen->getNodeEdgeSet()
    );
    aRequestBuilder.updateFrameCallback(mpResultProcessor);

    // check whether the stream is critical(need to setImageStreamBuffer here)
    for ( size_t i = 0; i < vDstStreams.size(); ++i ) {
        if ( vDstStreams[i].criticalBuffer ) {
            wait_and_get_buffer( requestNo, queryProvider(vDstStreams[i].streamId), aRequestBuilder );
        }
    }

    sp<IPipelineFrame> pFrame =
        aRequestBuilder
        .setMetaStreamBuffer(
                mAppMetaStreamId,
                pAppMetaStreamBuffer
                )
        .setMetaStreamBuffer(
                mHalMetaStreamId,
                pHalMetaStreamBuffer
                )
        .build(requestNo, mpPipelineContext);

    if( ! pFrame.get() ) {
        MY_LOGE("build request failed");
        CAM_TRACE_FMT_END();
        return UNKNOWN_ERROR;
    }

    mpPipelineContext->queue(pFrame);
    //
    MY_LOGD("submitRequest -");
    MY_LOGD_IF( 1, "submitRequest %d vDstStreams:%d -", requestNo, vDstStreams.size());
    CAM_TRACE_FMT_END();

    return OK;
}

/******************************************************************************
*
*******************************************************************************/
sp<IImageStreamInfo>
StereoLegacyPipeline::
queryImageStreamInfo(StreamId const streamId)
{
    if(mpImageStreamManager == NULL)
    {
        MY_LOGE("ImageStreamManager is NULL.");
        return NULL;
    }

    return mpImageStreamManager->getStreamInfoByStreamId(streamId);
}

/******************************************************************************
*
*******************************************************************************/
sp<IMetaStreamInfo>
StereoLegacyPipeline::
queryMetaStreamInfo(StreamId const streamId)
{
    if(mpMetaStreamManager == NULL)
    {
        MY_LOGE("MetaStreamManager is NULL.");
        return NULL;
    }

    return mpMetaStreamManager->getStreamInfoByStreamId(streamId);
}

/******************************************************************************
*
*******************************************************************************/
sp<StreamBufferProvider>
StereoLegacyPipeline::
queryProvider(StreamId const streamId)
{
    if(mpImageStreamManager == NULL)
    {
        MY_LOGE("ImageStreamManager is NULL.");
        return NULL;
    }

    return mpImageStreamManager->getBufProviderByStreamId(streamId);
}
