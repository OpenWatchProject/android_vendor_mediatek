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

#define LOG_TAG "MtkCam/ppl_builder"
//
#include "MyUtils.h"
#include "PipelineContextImpl.h"
#include <v3/pipeline/PipelineContext.h>
#include <v3/pipeline/IPipelineNodeMapControl.h>
#include "IPipelineFrameNumberGenerator.h"
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v3::NSPipelineContext;


/******************************************************************************
 *
 ******************************************************************************/
#if 0
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
#else
#define NO_OP                       ((void)0)
#define MY_LOGV(fmt, arg...)        NO_OP
#define MY_LOGD(fmt, arg...)        NO_OP
#define MY_LOGI(fmt, arg...)        NO_OP
#define MY_LOGW(fmt, arg...)        NO_OP
#define MY_LOGE(fmt, arg...)        NO_OP
#define MY_LOGA(fmt, arg...)        NO_OP
#define MY_LOGF(fmt, arg...)        NO_OP
#endif
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


#define FUNC_START     MY_LOGD("+")
#define FUNC_END       MY_LOGD("-")
/******************************************************************************
 *
 ******************************************************************************/
StreamBuilder::
StreamBuilder(
    eStreamType const type,
    sp<IImageStreamInfo> pStreamInfo
)
    : mpImpl( new StreamBuilderImpl() )
{
    mpImpl->mType             = type;
    mpImpl->mpImageStreamInfo = pStreamInfo;
}


/******************************************************************************
 *
 ******************************************************************************/
StreamBuilder::
StreamBuilder(
    eStreamType const type,
    sp<IMetaStreamInfo> pStreamInfo
)
    : mpImpl( new StreamBuilderImpl() )
{
    mpImpl->mType            = type;
    mpImpl->mpMetaStreamInfo = pStreamInfo;
}


/******************************************************************************
 *
 ******************************************************************************/
StreamBuilder::
StreamBuilder(const StreamBuilder& builder)
{
    mpImpl = builder.mpImpl;
}


/******************************************************************************
 *
 ******************************************************************************/
StreamBuilder::
~StreamBuilder()
{}


/******************************************************************************
 *
 ******************************************************************************/
StreamBuilder&
StreamBuilder::
setProvider(
    sp<IStreamBufferProviderT> pProvider
)
{
    mpImpl->mpProvider = pProvider;
    return *this;
}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
StreamBuilder::
build(
    sp<PipelineContext> pContext
) const
{
    typedef PipelineContext::PipelineContextImpl        PipelineContextImplT;
    PipelineContextImplT* pContextImpl = pContext->getImpl();
    if( ! pContext.get() || ! (pContextImpl = pContext->getImpl()) ) {
        MY_LOGE("cannot get context");
        return UNKNOWN_ERROR;
    }
    //
    MERROR err;
    if( OK != (err = pContextImpl->updateConfig(mpImpl.get()) ) )
        return err;
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
NodeBuilder::
NodeBuilder(
    NodeId_T const nodeId,
    android::sp<INodeActor> pNode
)
    : mpImpl( new NodeBuilderImpl(nodeId, pNode) )
{
}


/******************************************************************************
 *
 ******************************************************************************/
NodeBuilder::
NodeBuilder(const NodeBuilder& builder)
{
    mpImpl = builder.mpImpl;
}


/******************************************************************************
 *
 ******************************************************************************/
NodeBuilder::
~NodeBuilder()
{
}


/******************************************************************************
 *
 ******************************************************************************/
NodeBuilder&
NodeBuilder::
addStream(
    eDirection const direction,
    StreamSet const& streams
)
{
    if( direction == eDirection_IN )
        mpImpl->mInStreamSet.add(streams);
    else if ( direction == eDirection_OUT )
        mpImpl->mOutStreamSet.add(streams);
    return *this;
}


/******************************************************************************
 *
 ******************************************************************************/
NodeBuilder&
NodeBuilder::
setImageStreamUsage(
    StreamId_T const streamId,
    MUINT const bufUsage
)
{
    mpImpl->mUsageMap.add(streamId, bufUsage);
    return *this;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
NodeBuilder::
build(
    sp<PipelineContext> pContext
) const
{
    typedef PipelineContext::PipelineContextImpl        PipelineContextImplT;
    PipelineContextImplT* pContextImpl = pContext->getImpl();
    if( ! pContext.get() || ! (pContextImpl = pContext->getImpl()) ) {
        MY_LOGE("cannot get context");
        return UNKNOWN_ERROR;
    }
    // 1. check if this Node is already marked as reuse
    // TODO
    // 2. create Node if not existed
    MERROR err;
    if( OK != (err = pContextImpl->updateConfig(mpImpl.get()) ) )
        return err;
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
PipelineBuilder::
PipelineBuilder()
    : mpImpl( new PipelineBuilderImpl() )
{
}


/******************************************************************************
 *
 ******************************************************************************/
PipelineBuilder::
PipelineBuilder(const PipelineBuilder& builder)
{
    mpImpl = builder.mpImpl;
}

/******************************************************************************
 *
 ******************************************************************************/
PipelineBuilder::
~PipelineBuilder()
{
}


/******************************************************************************
 *
 ******************************************************************************/
PipelineBuilder&
PipelineBuilder::
setRootNode(
    NodeSet const& roots
)
{
    mpImpl->mRootNodes.add(roots);
    return *this;
}


/******************************************************************************
 *
 ******************************************************************************/
PipelineBuilder&
PipelineBuilder::
setNodeEdges(
    NodeEdgeSet const& edges
)
{
    mpImpl->mNodeEdges = edges;
    return *this;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineBuilder::
build(
    sp<PipelineContext> pContext
) const
{
    typedef PipelineContext::PipelineContextImpl        PipelineContextImplT;
    PipelineContextImplT* pContextImpl = pContext->getImpl();
    if( ! pContext.get() || ! (pContextImpl = pContext->getImpl()) ) {
        MY_LOGE("cannot get context");
        return UNKNOWN_ERROR;
    }
    //
    MERROR err;
    if( OK != (err = pContextImpl->updateConfig(mpImpl.get()) ) )
        return err;
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
RequestBuilder::
RequestBuilder()
    : mpImpl(new RequestBuilderImpl())
{
}


/******************************************************************************
 *
 ******************************************************************************/
RequestBuilder::
~RequestBuilder()
{
}


/******************************************************************************
 *
 ******************************************************************************/
RequestBuilder&
RequestBuilder::
setIOMap(
    NodeId_T const nodeId,
    IOMapSet const& imageIOMap,
    IOMapSet const& metaIOMap
)
{
    if( ! mpImpl->getFlag(RequestBuilderImpl::FLAG_IOMAP_CHANGED) )
    {
        mpImpl->mImageNodeIOMaps.clear();
        mpImpl->mMetaNodeIOMaps.clear();
        //
        mpImpl->setFlag(RequestBuilderImpl::FLAG_IOMAP_CHANGED);
    }
    mpImpl->mImageNodeIOMaps.add(nodeId, imageIOMap);
    mpImpl->mMetaNodeIOMaps.add(nodeId, metaIOMap);
    return *this;
}


/******************************************************************************
 *
 ******************************************************************************/
RequestBuilder&
RequestBuilder::
setRootNode(
    NodeSet const& roots
)
{
    mpImpl->setFlag(RequestBuilderImpl::FLAG_NODEEDGE_CHANGED);
    mpImpl->mRootNodes = roots;
    return *this;
}


/******************************************************************************
 *
 ******************************************************************************/
RequestBuilder&
RequestBuilder::
setNodeEdges(
    NodeEdgeSet const& edges
)
{
    mpImpl->setFlag(RequestBuilderImpl::FLAG_NODEEDGE_CHANGED);
    mpImpl->mNodeEdges = edges;
    return *this;
}


/******************************************************************************
 *
 ******************************************************************************/
RequestBuilder&
RequestBuilder::
replaceStreamInfo(
    StreamId_T const streamId,
    android::sp<IImageStreamInfo> pStreamInfo
)
{
    mpImpl->setFlag(RequestBuilderImpl::FLAG_REPLACE_STREAMINFO);
    mpImpl->mReplacingInfos.add(streamId, pStreamInfo);
    return *this;
}


/******************************************************************************
 *
 ******************************************************************************/
RequestBuilder&
RequestBuilder::
setImageStreamBuffer(
    StreamId_T const streamId,
    android::sp<IImageStreamBuffer> buffer
)
{
    mpImpl->mStreamBuffers_Image.add(streamId, buffer);
    return *this;
}


/******************************************************************************
 *
 ******************************************************************************/
RequestBuilder&
RequestBuilder::
setImageStreamBuffer(
    StreamId_T const streamId,
    android::sp<HalImageStreamBuffer> buffer
)
{
    mpImpl->mStreamBuffers_HalImage.add(streamId, buffer);
    return *this;
}


/******************************************************************************
 *
 ******************************************************************************/
RequestBuilder&
RequestBuilder::
setMetaStreamBuffer(
    StreamId_T const streamId,
    android::sp<IMetaStreamBuffer> buffer
)
{
    mpImpl->mStreamBuffers_Meta.add(streamId, buffer);
    return *this;
}


/******************************************************************************
 *
 ******************************************************************************/
RequestBuilder&
RequestBuilder::
setMetaStreamBuffer(
    StreamId_T const streamId,
    android::sp<HalMetaStreamBuffer> buffer
)
{
    mpImpl->mStreamBuffers_HalMeta.add(streamId, buffer);
    return *this;
}


/******************************************************************************
 *
 ******************************************************************************/
RequestBuilder&
RequestBuilder::
updateFrameCallback(
    wp<AppCallbackT> pCallback
)
{
    mpImpl->setFlag(RequestBuilderImpl::FLAG_CALLBACK_CHANGED);
    mpImpl->mpCallback = pCallback;
    return *this;
}


/******************************************************************************
 *
 ******************************************************************************/
android::sp<IPipelineFrame>
RequestBuilder::
build(
    MUINT32 const requestNo,
    sp<PipelineContext> pContext
)
{
    FUNC_START;
    typedef PipelineContext::PipelineContextImpl        PipelineContextImplT;
    PipelineContextImplT* pContextImpl = pContext->getImpl();
    if( ! pContext.get() || ! (pContextImpl = pContext->getImpl()) ) {
        MY_LOGE("cannot get context");
        return NULL;
    }
    //
    MY_LOGD("build requestNo %d", requestNo);
    sp<IPipelineFrame> pFrame = pContextImpl->constructRequest(mpImpl.get(), requestNo);
    if( ! pFrame.get() )
        MY_LOGE("constructRequest failed");
    //
    FUNC_END;
    return pFrame;
}


/******************************************************************************
 *
 ******************************************************************************/
