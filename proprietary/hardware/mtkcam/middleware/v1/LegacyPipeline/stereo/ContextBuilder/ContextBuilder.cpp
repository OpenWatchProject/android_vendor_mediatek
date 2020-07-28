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
#define SENSOR_ID 0
 
#define LOG_TAG "ContextBuilder"
#define CONTEXT_NAME "Stereo_P2"
//
#include <mtkcam/utils/std/Log.h>
//
#include <stdlib.h>
#include <utils/Errors.h>
#include <utils/List.h>
#include <utils/RefBase.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/ContextBuilder/ContextBuilder.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/NodeId.h>
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
//
#include <mtkcam/pipeline/utils/streaminfo/MetaStreamInfo.h>
#include <mtkcam/pipeline/utils/streaminfo/ImageStreamInfo.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/ContextBuilder/MetaStreamManager.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/ContextBuilder/ImageStreamManager.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/ContextBuilder/NodeConfigDataManager.h>
//
#include <mtkcam/utils/std/Trace.h>
using namespace NSCam;
using namespace v3;
using namespace NSCam::v3::Utils;
/*using namespace android;
using namespace NSCam::v3::NSPipelineContext;*/
/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
//
using namespace NSCam;
using namespace v3;
//
sp<ContextBuilder>
ContextBuilder::
create(const char* callerName,
       sp<MetaStreamManager>& MetaStreamManager,
       sp<ImageStreamManager>& ImageStreamManager,
       sp<NodeConfigDataManager>& nodeConfigManager)
{
    return new ContextBuilder(callerName,
                              MetaStreamManager,
                              ImageStreamManager,
                              nodeConfigManager);
}
//
void
ContextBuilder::
destroy()
{
    if(mpMetaStreamManager!=0)
        mpMetaStreamManager = NULL;
    if(mpImageStreamManager!=0)
        mpImageStreamManager = NULL;
    if(mpNodeConfigManager!=0)
        mpNodeConfigManager = NULL;
    if(mpContext!=0)
        mpContext = NULL;
}
//
ContextBuilder::
ContextBuilder(const char* Name,
               sp<MetaStreamManager>& metaStreamManager,
               sp<ImageStreamManager>& imageStreamManager,
               sp<NodeConfigDataManager>& nodeConfigManager)
    : mName(Name),
      mpMetaStreamManager(metaStreamManager),
      mpImageStreamManager(imageStreamManager),
      mpNodeConfigManager(nodeConfigManager)
{
}
//
ContextBuilder::
~ContextBuilder()
{
    MY_LOGD("dcot(%x)", this);
}
//
MERROR
ContextBuilder::
buildContext()
{
    if(mpMetaStreamManager == 0 || mpImageStreamManager == 0)
    {
        MY_LOGE("Manager is invalid");
        return UNKNOWN_ERROR;
    }
    if(mpContext!=NULL)
    {
        mpContext = NULL;
    }
    mpContext = NSCam::v3::NSPipelineContext::PipelineContext::create(mName);
    setupPipelineContext();
    return OK;
}
//
sp<NSCam::v3::NSPipelineContext::PipelineContext>&
ContextBuilder::
getContext()
{
    return mpContext;
}
//
void
ContextBuilder::
setUserParams(
    sp<NSCam::StereoBasicParameters>& userParams)
{
    mpUserParams = userParams;
}
//
MERROR
ContextBuilder::
setupPipelineContext()
{
    CAM_TRACE_NAME("setupPipelineContext");
    MUINT32 size = 0;

    mpContext->beginConfigure();
    // 1. setup stream
    // meta stream
    MY_LOGD("Set Meta Stream to pipeline context.");
    {
        CAM_TRACE_NAME("SetMetaStream");
        size = mpMetaStreamManager->getSize();
        if(size != 0)
        {
            for(MUINT32 i=0; i<size ; ++i)
            {
                StreamBuilder(mpMetaStreamManager->getPoolTypeByIndex(i),
                              mpMetaStreamManager->getStreamInfoByIndex(i))
                .build(mpContext);
            }
        }
    }
    // image stream
    MY_LOGD("Set Image Stream to pipeline context.");
    {
        CAM_TRACE_NAME("SetImageStream");
        size = mpImageStreamManager->getSize();
        if(size != 0)
        {
            for(MUINT32 i=0; i<size ; ++i)
            {
                if(mpImageStreamManager->getPoolTypeByIndex(i) == eStreamType_IMG_HAL_PROVIDER)
                {
                    StreamBuilder(mpImageStreamManager->getPoolTypeByIndex(i),
                                  mpImageStreamManager->getStreamInfoByIndex(i))
                        .setProvider(mpImageStreamManager->getBufProviderByIndex(i))
                        .build(mpContext);
                }
                else
                {
                    StreamBuilder(mpImageStreamManager->getPoolTypeByIndex(i),
                                  mpImageStreamManager->getStreamInfoByIndex(i))
                    .build(mpContext);
                }
            }
        }
    }

    MY_LOGD("Config node");
    {
        CAM_TRACE_NAME("ConfigNode");
        mpNodeConfigManager->configNode(mpMetaStreamManager, mpImageStreamManager, mpUserParams, mpContext);
    }
    // 2. Nodes
    {
        CAM_TRACE_NAME("BuildNode");
        size = mpNodeConfigManager->getSize();
        if(size != 0)
        {
            MERROR ret;
            NodeId_T nodeId;
            MUINT32 streamUsageSize = 0;
            DefaultKeyedVector<StreamId_T, MINT32 > vStreamUsage;
            for(MUINT32 i=0; i<size ; ++i)
            {
                nodeId = mpNodeConfigManager->getNodeId(i);
                sp<INodeActor> temp = mpNodeConfigManager->getNodeInstance(nodeId);
                MY_LOGD("NodeBuilder(%d)+", nodeId);
                NodeBuilder mb(nodeId, temp);
                mb.addStream(
                            NodeBuilder::eDirection_IN,
                            mpNodeConfigManager->getInStreamSet(nodeId)
                            );
                mb.addStream(
                            NodeBuilder::eDirection_OUT,
                            mpNodeConfigManager->getOutStreamSet(nodeId)
                            );
                MY_LOGD("NodeBuilder(%d)-", nodeId);
                // set stream usage
                vStreamUsage = mpNodeConfigManager->getStreamUsageSet(nodeId);
                for(MUINT32 j=0;j<vStreamUsage.size();++j)
                {
                    mb.setImageStreamUsage(vStreamUsage.keyAt(j), vStreamUsage.valueAt(j));
                }
                ret = mb.build(mpContext);
                if(OK != ret)
                {
                    MY_LOGE("Build Node fail. id(%d), error(%d)", nodeId, ret);
                    return UNKNOWN_ERROR;
                }
            }
        }
    }

    // PipelineContext
    MY_LOGD("Build pipeline");
    {
        CAM_TRACE_NAME("BuildPipeline");
        MERROR ret = PipelineBuilder().setRootNode(
                                                mpNodeConfigManager->getRootNodeId()
                                                )
                                      .setNodeEdges(
                                                mpNodeConfigManager->getNodeEdgeSet()
                                                )
                                      .build(mpContext);
        if( ret != OK ) {
            MY_LOGE("build pipeline error");
            return UNKNOWN_ERROR;
        }
    }

    mpContext->endConfigure(MTRUE);
    return OK;
}