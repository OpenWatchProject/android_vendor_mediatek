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
#define LOG_TAG "ContextBuilder/DepthNodeData"
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/pipeline/hwnode/DepthMapNode.h>
//
#include <utils/StrongPointer.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/ContextBuilder/MetaStreamManager.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/ContextBuilder/ImageStreamManager.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/ContextBuilder/StereoBasicParameters.h>
//
#include <mtkcam/pipeline/pipeline/PipelineContext.h>
//
#include <mtkcam/pipeline/hwnode/DepthMapNode.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
//
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
//
#include <mtkcam/drv/IHalSensor.h>
//
#include <mtkcam/feature/stereo/StereoCamEnum.h>
//
#include "DepthMapNodeConfigData.h"

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
typedef DepthMapNode                    NodeT;
typedef NodeActor< NodeT >              MyNodeActorT;
//
DepthMapNodeConfigData::
DepthMapNodeConfigData::
DepthMapNodeConfigData(
    MINT32 openId,
    NodeId_T nodeId,
    char const* nodeName)
    : INodeConfigBase(openId, nodeId, nodeName)
{
    initParam.openId = mOpenId;
    initParam.openId_main2 = 0;
    initParam.nodeId = mNodeId;
    initParam.nodeName = mNodeName;
    initParam.featureMode = E_INIT_MODE;
    initParam.moduleType = INIT_TYPE;
}
//
DepthMapNodeConfigData::
~DepthMapNodeConfigData()
{
    MY_LOGD("dcot(%x)", this);
}
//
sp<INodeActor>
DepthMapNodeConfigData::
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
DepthMapNodeConfigData::
configNode(
    MetaStreamManager* metaManager,
    ImageStreamManager* imageManager,
    StereoBasicParameters* userParams,
    PipelineContext* pipeContext)
{
    MY_LOGD("Config DepthMapNode stream. +");

    if(userParams != NULL) {
        // get main2 openId.
        initParam.moduleType = (SeneorModuleType)userParams->getInt(NSCam::StereoBasicParameters::KEY_STEREO_CAM_MODULE_TYPE);
        initParam.featureMode = (StereoFeatureMode)userParams->getInt(NSCam::StereoBasicParameters::KEY_STEREO_CAM_FEATURE_TYPE);
        initParam.openId       = userParams->getInt(StereoBasicParameters::KEY_VSDOF_DEPTHMAP_NODE_MAIN1ID);
        initParam.openId_main2 = userParams->getInt(StereoBasicParameters::KEY_VSDOF_DEPTHMAP_NODE_MAIN2ID);
        MY_LOGD("get openIds from userParams (%d,%d)", initParam.openId, initParam.openId_main2);
        MY_LOGD("module type (%d) feature type (%d)", initParam.moduleType, initParam.featureMode);
    }

    MY_LOGD("BM-vsdof:%d, BB-vsdof:%d, tk-depth(3rd):%d, denoise:%d",
        STEREO_BM_PRV_CAP_REC == (initParam.moduleType|initParam.featureMode),
        STEREO_BB_PRV_CAP_REC == (initParam.moduleType|initParam.featureMode),
        E_STEREO_FEATURE_MTK_DEPTHMAP == initParam.featureMode,
        STEREO_BM_DENOISE == (initParam.moduleType|initParam.featureMode));

    // for vsdof
    if(     STEREO_BM_PRV_CAP_REC == (initParam.moduleType|initParam.featureMode)
        ||  STEREO_BB_PRV_CAP_REC == (initParam.moduleType|initParam.featureMode))
    {

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
            addStream(cfgParam.pInHalMeta_Main2, MTRUE);
            //
            cfgParam.pOutAppMeta = metaManager->getStreamInfoByStreamId(eSTREAMID_META_APP_DYNAMIC_DEPTH);
            addStream(cfgParam.pOutAppMeta, MFALSE);
            //
            cfgParam.pOutHalMeta = metaManager->getStreamInfoByStreamId(eSTREAMID_META_HAL_DYNAMIC_DEPTH);
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
            // decide whether main2 uses IMGO
            if(::supportMain2FullRaw())
            {
                MY_LOGD("cfgParam.pInFullRaw_Main2 added");
                cfgParam.pInFullRaw_Main2 = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01);
                addStream(  cfgParam.pInFullRaw_Main2, MTRUE);
            }
            //
            cfgParam.pInResizedRaw = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_RAW_RESIZER);
            addStream(  cfgParam.pInResizedRaw, MTRUE);
            //
            cfgParam.pInResizedRaw_Main2 = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01);
            addStream(  cfgParam.pInResizedRaw_Main2, MTRUE);
            //
            cfgParam.pInLcso = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_RAW_LCSO);
            addStream(  cfgParam.pInLcso, MTRUE);
            //
            cfgParam.pInLcso_Main2 = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_RAW_LCSO_01);
            addStream(  cfgParam.pInLcso_Main2, MTRUE);
            //
            cfgParam.pAppImageDepthMap = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_DEPTHMAPYUV);
            addStream(  cfgParam.pAppImageDepthMap, MFALSE);
            //
            cfgParam.pHalImageMainImg = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_MAINIMAGEYUV);
            addStream(  cfgParam.pHalImageMainImg, MFALSE);
            //
            cfgParam.pHalImageMainImg_Cap = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_MAINIMAGE_CAPYUV);
            addStream(  cfgParam.pHalImageMainImg_Cap, MFALSE);
            //
            cfgParam.pHalImageDMBG = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_DMBGYUV);
            addStream(  cfgParam.pHalImageDMBG, MFALSE);
            //
            cfgParam.pHalImageJPSMain1 = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_JPSMAIN1YUV);
            addStream(  cfgParam.pHalImageJPSMain1, MFALSE);
            //
            cfgParam.pHalImageJPSMain2 = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_JPSMAIN2YUV);
            addStream(  cfgParam.pHalImageJPSMain2, MFALSE);
            //
            cfgParam.pHalImageFD = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_FDYUV);
            addStream(  cfgParam.pHalImageFD, MFALSE);
            //
            cfgParam.pAppImageExtraData = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_STEREO_DBG);
            addStream(  cfgParam.pAppImageExtraData, MFALSE);
            //
            cfgParam.pAppImageLDC = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_STEREO_DBG_LDC);
            addStream(  cfgParam.pAppImageLDC, MFALSE);
            //
            cfgParam.pHalImageDepthWrapper = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_HAL_DEPTHWAPPER);
            addStream(  cfgParam.pHalImageDepthWrapper, MFALSE);
            //
            cfgParam.pAppImageN3DDebug = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_STEREO_N3D_DEBUG);
            addStream(  cfgParam.pAppImageN3DDebug, MFALSE);
            //
            cfgParam.pAppImagePostview = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_DUALYUVNODE_THUMBNAIL);
            addStream(  cfgParam.pAppImagePostview, MFALSE);
            //
            cfgParam.pAppImageTuning_Main1 = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_STEREO_P2_TUNING);
            addStream(  cfgParam.pAppImageTuning_Main1, MFALSE);
            addStreamEnd(MFALSE);

        }
    }
    // for denoise
    if(STEREO_BM_DENOISE == (initParam.moduleType|initParam.featureMode))
    {
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
            addStream(cfgParam.pInHalMeta_Main2, MTRUE);
            //
            cfgParam.pOutAppMeta = metaManager->getStreamInfoByStreamId(eSTREAMID_META_APP_DYNAMIC_DEPTH);
            addStream(cfgParam.pOutAppMeta, MFALSE);
            //
            cfgParam.pOutHalMeta = metaManager->getStreamInfoByStreamId(eSTREAMID_META_HAL_DYNAMIC_DEPTH);
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
            // decide whether main2 uses IMGO
            if(::supportMain2FullRaw())
            {
                MY_LOGD("cfgParam.pInFullRaw_Main2 added");
                cfgParam.pInFullRaw_Main2 = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01);
                addStream(  cfgParam.pInFullRaw_Main2, MTRUE);
            }
            //
            cfgParam.pInResizedRaw = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_RAW_RESIZER);
            addStream(  cfgParam.pInResizedRaw, MTRUE);
            //
            cfgParam.pInResizedRaw_Main2 = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01);
            addStream(  cfgParam.pInResizedRaw_Main2, MTRUE);
            //
            cfgParam.pHalImageDisparityMap_Left = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_DISPARITY_L);
            addStream(  cfgParam.pHalImageDisparityMap_Left, MFALSE);
            //
            cfgParam.pHalImageDisparityMap_Right = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_DISPARITY_R);
            addStream(  cfgParam.pHalImageDisparityMap_Right, MFALSE);
            //
            cfgParam.pHalImageWarpingMatrix = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_BMDENOISE_WAPING_MATRIX);
            addStream(  cfgParam.pHalImageWarpingMatrix, MFALSE);
            //
            cfgParam.pHalImageSceneInfo = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_BMDENOISE_SCENE_INFO);
            addStream(  cfgParam.pHalImageSceneInfo, MFALSE);
            //
            cfgParam.pAppImageExtraData = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_STEREO_DBG);
            addStream(  cfgParam.pAppImageExtraData, MFALSE);
            addStreamEnd(MFALSE);
        }
    }
    // for tk-depth + 3rd bokeh
    if(E_STEREO_FEATURE_MTK_DEPTHMAP == initParam.featureMode)
    {
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
            addStream(cfgParam.pInHalMeta_Main2, MTRUE);
            //
            cfgParam.pOutAppMeta = metaManager->getStreamInfoByStreamId(eSTREAMID_META_APP_DYNAMIC_DEPTH);
            addStream(cfgParam.pOutAppMeta, MFALSE);
            //
            cfgParam.pOutHalMeta = metaManager->getStreamInfoByStreamId(eSTREAMID_META_HAL_DYNAMIC_DEPTH);
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
            if(::supportMain2FullRaw()){
                MY_LOGD("cfgParam.pInFullRaw_Main2 added");
                cfgParam.pInFullRaw_Main2 = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01);
                addStream(  cfgParam.pInFullRaw_Main2, MTRUE);
            }else{
                MY_LOGD("cfgParam.pInFullRaw_Main2 not added");
            }
            //
            cfgParam.pInResizedRaw = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_RAW_RESIZER);
            addStream(  cfgParam.pInResizedRaw, MTRUE);
            //
            cfgParam.pInResizedRaw_Main2 = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01);
            addStream(  cfgParam.pInResizedRaw_Main2, MTRUE);
            //
            cfgParam.pInLcso = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_RAW_LCSO);
            addStream(  cfgParam.pInLcso, MTRUE);
            //
            cfgParam.pInLcso_Main2 = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_RAW_LCSO_01);
            addStream(  cfgParam.pInLcso_Main2, MTRUE);
            //
            cfgParam.pHalImageMainImg = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_MAINIMAGEYUV);
            addStream(  cfgParam.pHalImageMainImg, MFALSE);
            //
            cfgParam.pHalImageDepthMap = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_DEPTHMAPYUV);
            addStream(  cfgParam.pHalImageDepthMap, MFALSE);
            //
            cfgParam.pHalImageFD = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_FDYUV);
            addStream(  cfgParam.pHalImageFD, MFALSE);
            addStreamEnd(MFALSE);
        }
    }
    MY_LOGD("Config DepthMapNode stream. -");
}
//
void
DepthMapNodeConfigData::
dump()
{

}
//
void
DepthMapNodeConfigData::
destroy()
{
    cfgParam.pInAppMeta = NULL;
    cfgParam.pInHalMeta = NULL;
    cfgParam.pInHalMeta_Main2 = NULL;
    cfgParam.pOutAppMeta = NULL;
    cfgParam.pOutHalMeta = NULL;
    //
    cfgParam.pInFullRaw = NULL;
    cfgParam.pInFullRaw_Main2 = NULL;
    cfgParam.pInResizedRaw = NULL;
    cfgParam.pInResizedRaw_Main2 = NULL;
    cfgParam.pHalImageDisparityMap_Left = NULL;
    cfgParam.pHalImageDisparityMap_Right = NULL;
    cfgParam.pHalImageWarpingMatrix = NULL;

    mvInStreamSet.clear();
    mvOutStreamSet.clear();
    mvStreamUsageSet.clear();
}