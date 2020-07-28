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
#include <mtkcam3/pipeline/pipeline/IPipelineContextBuilder.h>
#include <mtkcam/utils/std/ULog.h>
#include "MyUtils.h"
#include "PipelineContextImpl.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_CONTEXT);
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v3::pipeline;
using namespace NSCam::v3::pipeline::NSPipelineContext;


/******************************************************************************
 *
 ******************************************************************************/
#if 0
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
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
    AttributeT attribute,
    sp<IImageStreamInfo> pStreamInfo
)
    : mpImpl( new StreamBuilderImpl() )
{
    mpImpl->mAttribute        = attribute;
    mpImpl->mpImageStreamInfo = pStreamInfo;
}


/******************************************************************************
 *
 ******************************************************************************/
StreamBuilder::
StreamBuilder(
    AttributeT attribute,
    sp<IMetaStreamInfo> pStreamInfo
)
    : mpImpl( new StreamBuilderImpl() )
{
    mpImpl->mAttribute       = attribute;
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
    IImageStreamBufferProviderT pProvider
)
{
    mpImpl->mpProvider = pProvider;
    return *this;
}

/******************************************************************************
 *
 ******************************************************************************/
StreamBuilder&
StreamBuilder::
setSharedImgStream(
    android::sp<IImageStreamInfo> pStreamInfo
)
{
    mpImpl->mpSharedImageStreamInfo = pStreamInfo;
    return *this;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
StreamBuilder::
build(
    sp<IPipelineContext> pPipelineContext
) const
{
    auto pContext = PipelineContext::cast(pPipelineContext.get());
    if (CC_UNLIKELY( pContext == nullptr )) {
        MY_LOGE("Bad context");
        return UNKNOWN_ERROR;
    }

    typedef PipelineContext::PipelineContextImpl        PipelineContextImplT;
    PipelineContextImplT* pContextImpl = pContext->getImpl();
    if (CC_UNLIKELY( pContextImpl == nullptr )) {
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
    sp<IPipelineContext> pPipelineContext
) const
{
    auto pContext = PipelineContext::cast(pPipelineContext.get());
    if (CC_UNLIKELY( pContext == nullptr )) {
        MY_LOGE("Bad context");
        return UNKNOWN_ERROR;
    }

    typedef PipelineContext::PipelineContextImpl        PipelineContextImplT;
    PipelineContextImplT* pContextImpl = pContext->getImpl();
    if (CC_UNLIKELY( pContextImpl == nullptr )) {
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
    sp<IPipelineContext> pPipelineContext
) const
{
    auto pContext = PipelineContext::cast(pPipelineContext.get());
    if (CC_UNLIKELY( pContext == nullptr )) {
        MY_LOGE("Bad context");
        return UNKNOWN_ERROR;
    }

    typedef PipelineContext::PipelineContextImpl        PipelineContextImplT;
    PipelineContextImplT* pContextImpl = pContext->getImpl();
    if (CC_UNLIKELY( pContextImpl == nullptr )) {
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

