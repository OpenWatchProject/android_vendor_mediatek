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
 * MediaTek Inc. (C) 2017. All rights reserved.
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
#define LOG_TAG "ContextBuilder/Dual3rdPartyNodeData"
//
#include <mtkcam/utils/std/Log.h>
#include <utils/StrongPointer.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/ContextBuilder/MetaStreamManager.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/ContextBuilder/ImageStreamManager.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/ContextBuilder/StereoBasicParameters.h>
//
#include <mtkcam/pipeline/pipeline/PipelineContext.h>
//
#include <mtkcam/pipeline/hwnode/Dual3rdPartyNode.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
//
#include "Dual3rdPartyNodeConfigData.h"
//
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
using namespace android;
using namespace NSCam;
using namespace v3;
using namespace NSCam::v3::NSPipelineContext;
//
typedef Dual3rdPartyNode          NodeT;
typedef NodeActor< NodeT >              MyNodeActorT;
//
Dual3rdPartyNodeConfigData::
Dual3rdPartyNodeConfigData(
    MINT32 openId,
    NodeId_T nodeId,
    char const* nodeName)
    : INodeConfigBase(openId, nodeId, nodeName)
{
    initParam.openId = mOpenId;
    initParam.nodeId = mNodeId;
    initParam.nodeName = mNodeName;
}
//
Dual3rdPartyNodeConfigData::
~Dual3rdPartyNodeConfigData()
{
    MY_LOGD("dcot(%x)", this);
}
//
sp<INodeActor>
Dual3rdPartyNodeConfigData::
getNode()
{
    MY_LOGD("+");
    sp<MyNodeActorT> pNode = new MyNodeActorT( NodeT::createInstance() );
    //
    pNode->setInitParam(initParam);
    pNode->setConfigParam(cfgParam);
    //
    MY_LOGD("-");
    return pNode;
}
//
void
Dual3rdPartyNodeConfigData::
configNode(
    MetaStreamManager* metaManager,
    ImageStreamManager* imageManager,
    StereoBasicParameters* userParams,
    PipelineContext* pipeContext)
{
    if(userParams != NULL) {
        // get main2 openId.
        initParam.openId       = userParams->getInt(StereoBasicParameters::KEY_VSDOF_DEPTHMAP_NODE_MAIN1ID);
        initParam.openId_main2 = userParams->getInt(StereoBasicParameters::KEY_VSDOF_DEPTHMAP_NODE_MAIN2ID);
        MY_LOGD("get openIds from userParams (%d,%d)", initParam.openId, initParam.openId_main2);
    }
    //
    // Get meta stream info
    //
    if(metaManager != NULL)
    {
        addStreamBegin(MTRUE);
        cfgParam.pInAppMeta = metaManager->getStreamInfoByStreamId(eSTREAMID_META_APP_CONTROL);
        addStream(cfgParam.pInAppMeta, MTRUE);
        //
        cfgParam.pInHalMeta = metaManager->getStreamInfoByStreamId(eSTREAMID_META_HAL_DYNAMIC_P1);
        addStream(cfgParam.pInHalMeta, MTRUE);
        //
        cfgParam.pInHalMeta_Main2 = metaManager->getStreamInfoByStreamId(eSTREAMID_META_HAL_DYNAMIC_P1_MAIN2);
        addStream(cfgParam.pInHalMeta, MTRUE);
        //
        cfgParam.pOutAppMeta = metaManager->getStreamInfoByStreamId(eSTREAMID_META_APP_DYNAMIC_DUALYUV);
        addStream(cfgParam.pOutAppMeta, MFALSE);
        //
        cfgParam.pOutHalMeta = metaManager->getStreamInfoByStreamId(eSTREAMID_META_HAL_DYNAMIC_DUALYUV);
        addStream(cfgParam.pOutHalMeta, MFALSE);
        addStreamEnd(MTRUE);
    }
    //
    // Get image stream info
    //
    if(imageManager != NULL)
    {
        addStreamBegin(MFALSE);
        cfgParam.pInFullRaw = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE);
        addStream(  cfgParam.pInFullRaw, MTRUE);
        // input stream
        cfgParam.pInFullRaw_Main2 = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01);
        addStream(  cfgParam.pInFullRaw_Main2, MTRUE);
        //
        cfgParam.pInResizedRaw = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_RAW_RESIZER);
        addStream(  cfgParam.pInResizedRaw, MTRUE);
        //
        cfgParam.pInResizedRaw_Main2 = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01);
        addStream(  cfgParam.pInResizedRaw_Main2, MTRUE);
        //
        cfgParam.pInLCSO = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_RAW_LCSO);
        addStream(  cfgParam.pInLCSO, MTRUE);
        //
        cfgParam.pInLCSO_Main2 = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_RAW_LCSO_01);
        addStream(  cfgParam.pInLCSO_Main2, MTRUE);
        //
        cfgParam.pInResize_YUV = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_THIRDPARTY_RESIZE_YUV_0);
        addStream(  cfgParam.pInResize_YUV, MTRUE);
        //
        cfgParam.pInResize_YUV_Main2 = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_THIRDPARTY_RESIZE_YUV_1);
        addStream(  cfgParam.pInResize_YUV_Main2, MTRUE);
        //
        cfgParam.pInFullSize_YUV = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_THIRDPARTY_FULLSIZE_YUV_0);
        addStream(  cfgParam.pInFullSize_YUV, MTRUE);
        //
        cfgParam.pInFullSize_YUV_Main2 = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_THIRDPARTY_FULLSIZE_YUV_1);
        addStream(  cfgParam.pInFullSize_YUV_Main2, MTRUE);
        // output stream
        cfgParam.pOutAppPrvImg = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_YUV_00);
        addStream(  cfgParam.pOutAppPrvImg, MFALSE);
        // preview cb
        cfgParam.pOutAppPrvCB = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_YUV_01);
        addStream(  cfgParam.pOutAppPrvCB, MFALSE);
        //
        cfgParam.pOutHalCapBokehResult = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_YUV_JPEG);
        addStream(  cfgParam.pOutHalCapBokehResult, MFALSE);
        //
        cfgParam.pOutAppCapDepthResult = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_STEREO_DEPTHWRAPPER);
        addStream(  cfgParam.pOutAppCapDepthResult, MFALSE);
        //
        cfgParam.pOutHalThumbnailImg_Cap = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL);
        addStream(  cfgParam.pOutHalThumbnailImg_Cap, MFALSE);
        //
        cfgParam.pOutAppPrvFD = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_YUV_FD);
        addStream(  cfgParam.pOutAppPrvFD, MFALSE);
        addStreamEnd(MFALSE);
    }
}
//
void
Dual3rdPartyNodeConfigData::
dump()
{

}
//
void
Dual3rdPartyNodeConfigData::
destroy()
{
    //cfgParam.pInHalMetadata = NULL;
    cfgParam.pInAppMeta = nullptr;
    cfgParam.pOutAppMeta = nullptr;
    cfgParam.pInHalMeta = nullptr;
    cfgParam.pOutHalMeta = nullptr;
    cfgParam.pInFullRaw = nullptr;
    cfgParam.pInFullRaw_Main2 = nullptr;
    cfgParam.pInResizedRaw = nullptr;
    cfgParam.pInResizedRaw_Main2 = nullptr;
    cfgParam.pInResize_YUV = nullptr;
    cfgParam.pInResize_YUV_Main2 = nullptr;
    cfgParam.pInFullSize_YUV = nullptr;
    cfgParam.pInFullSize_YUV_Main2 = nullptr;
    cfgParam.pInLCSO = nullptr;
    cfgParam.pInLCSO_Main2 = nullptr;
    cfgParam.pOutAppPrvImg = nullptr;
    cfgParam.pOutAppPrvCB = nullptr;
    cfgParam.pOutHalCapBokehResult = nullptr;
    cfgParam.pOutAppCapDepthResult = nullptr;
    cfgParam.pOutHalThumbnailImg_Cap = nullptr;
    mvInStreamSet.clear();
    mvOutStreamSet.clear();
    mvStreamUsageSet.clear();
}
