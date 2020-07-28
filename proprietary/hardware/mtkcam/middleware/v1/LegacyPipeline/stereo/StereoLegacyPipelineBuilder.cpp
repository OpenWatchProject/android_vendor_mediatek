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

#define LOG_TAG "MtkCam/StereoLgcyPipeBuilder"

#include <mtkcam/utils/std/Log.h>
#include <utils/RefBase.h>
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include <mtkcam/middleware/v1/LegacyPipeline/NodeId.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/StereoLegacyPipelineDef.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/StereoLegacyPipelineBuilder.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/ContextBuilder/ContextBuilder.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/ContextBuilder/StereoBasicParameters.h>
//
#include "StereoLegacyPipeline.h"
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
//
using namespace NSCam::v1::NSLegacyPipeline;
using namespace android;
using namespace NSCam::v3::NSPipelineContext;

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
#define FUNC_NAME                   MY_LOGD("")

/******************************************************************************
*
*******************************************************************************/
StereoLegacyPipelineBuilder::
StereoLegacyPipelineBuilder(
    MINT32 const openId,
    char* const pipeName,
    MINT32 const mode
)
    : android::RefBase()
    , mOpenId(openId)
    , mPipeName(pipeName)
    , mMode(mode)
    , mbIsSensor(MFALSE)
    , mspMetaStreamManager(NULL)
    , mspImageStreamManager(NULL)
    , mspNodeConfigDataManager(NULL)
    , mHalMetaStreamId(0)
    , mAppMetaStreamId(0)
{
}

StereoLegacyPipelineBuilder::
~StereoLegacyPipelineBuilder()
{
    FUNC_NAME;
    if(mspMetaStreamManager != NULL)
    {
        mspMetaStreamManager->destroy();
        mspMetaStreamManager = NULL;
    }
    if(mspImageStreamManager != NULL)
    {
        mspImageStreamManager->destroy();
        mspImageStreamManager = NULL;
    }
    if(mspNodeConfigDataManager != NULL)
    {
        mspNodeConfigDataManager->destroy();
        mspNodeConfigDataManager = NULL;
    }
    mHalMetaStreamId = 0;
    mAppMetaStreamId = 0;
}

sp<ILegacyPipeline>
StereoLegacyPipelineBuilder::
create(
    sp<MetaStreamManager> spMetaStreamMng,
    sp<ImageStreamManager> spImageStreamMng,
    sp<NodeConfigDataManager> spNodeConfigDataMng
)
{
    FUNC_START;

    //
    // 0. Config Check **************
    //
    if(    spMetaStreamMng == NULL
        || spImageStreamMng == NULL
        || spNodeConfigDataMng == NULL)
    {
        MY_LOGE("set managers error!!");
        return NULL;
    }
    if(mHalMetaStreamId == 0 || mAppMetaStreamId == 0) {
        MY_LOGE("APP/HAL meta stream is NULL!!");
        return NULL;
    }
    sp<StereoLegacyPipeline> spLegacyPipeline = StereoLegacyPipeline::createInstance(getOpenId(), mMode);
    //
    // 1. Context Builder **************
    //
    sp<ContextBuilder> pContextBlder = ContextBuilder::create(mPipeName
                                                    , spMetaStreamMng
                                                    , spImageStreamMng
                                                    , spNodeConfigDataMng);
    //
    StereoPipelineSensorParam aParam;
    sp<StereoBasicParameters> pUserParams = new StereoBasicParameters();
    MINT32 sensorId = 0, sensorId_main2 = 1;
    StereoSettingProvider::getStereoSensorIndex(sensorId, sensorId_main2);
    if ( !querySrcParam(aParam) )
    {
        pUserParams->set(StereoBasicParameters::KEY_VSDOF_DEPTHMAP_NODE_MAIN1ID, sensorId);
        pUserParams->set(StereoBasicParameters::KEY_VSDOF_DEPTHMAP_NODE_MAIN2ID, sensorId_main2);
    }
    else
    {
        //Set SensorParams to ContextBuilder for P1node configuration
        pUserParams->set(StereoBasicParameters::KEY_P1NODE_SENSOR_CONFIG_MODE, aParam.mode);
        pUserParams->set(StereoBasicParameters::KEY_IMAGE_WIDTH, aParam.size.w);
        pUserParams->set(StereoBasicParameters::KEY_IMAGE_HEIGHT, aParam.size.h);
        pUserParams->set(StereoBasicParameters::KEY_P1NODE_SENSOR_CONFIG_FPS, aParam.fps);
        pUserParams->set(StereoBasicParameters::KEY_P1NODE_SENSOR_CONFIG_PIXELMODE, aParam.pixelMode);
    }
    // set module type and feature type
    MINT32 moduleType = StereoSettingProvider::getStereoModuleType();
    MINT32 featureType = StereoSettingProvider::getStereoFeatureMode();
    pUserParams->set(StereoBasicParameters::KEY_STEREO_CAM_MODULE_TYPE, moduleType);
    pUserParams->set(StereoBasicParameters::KEY_STEREO_CAM_FEATURE_TYPE, featureType);
    pContextBlder->setUserParams(pUserParams);

    //
    //Build Context
    pContextBlder->buildContext();
    //
    sp<PipelineContext> pPipelineContext = pContextBlder->getContext();
    if( !pPipelineContext.get() ) {
        MY_LOGE("cannot create context");
        return NULL;
    }
    //
    // 2. Request Builder **************
    //
    sp<ResultProcessor> pResultProcessor = ResultProcessor::createInstance();
    sp<RequestBuilder> pRequestBuilder = new RequestBuilder();
    MY_LOGD("NodeCfgMnger->getSize() = %d", spNodeConfigDataMng->getSize());
    for(size_t i=0; i<spNodeConfigDataMng->getSize(); i++)
    {
        NodeId_T nodeId = spNodeConfigDataMng->getNodeId(i);
        pRequestBuilder->setIOMap(
                        nodeId,
                        spNodeConfigDataMng->getImageIOMap(nodeId),
                        spNodeConfigDataMng->getMetaIOMap(nodeId)
                        );
        spLegacyPipeline->setMetaIOMap(
                        nodeId,
                        spNodeConfigDataMng->getMetaIOMap(nodeId)
                        );
    }
    pRequestBuilder->setRootNode(
                    spNodeConfigDataMng->getRootNodeId()
                    );
    pRequestBuilder->setNodeEdges(
                    spNodeConfigDataMng->getNodeEdgeSet()
                    );
    pRequestBuilder->updateFrameCallback(pResultProcessor);
    //
    //
    // 3. Register Listener ****************
    //
    sp<TimestampProcessor> pTimestampProcessor = TimestampProcessor::createInstance(getOpenId());
    MY_LOGD("sensorId = %d, sensorId_main2 = %d, getOpenId() = %d", sensorId, sensorId_main2, getOpenId());
    //for main1
    if(sensorId == getOpenId()){
        #if MTK_CAM_DISPAY_FRAME_CONTROL_ON
        pResultProcessor->registerListener(
                            eSTREAMID_META_HAL_DYNAMIC_P1,
                            pTimestampProcessor);
        #else
        pResultProcessor->registerListener(
                            eSTREAMID_META_APP_DYNAMIC_P1,
                            pTimestampProcessor);
        #endif
    }
    else { // for main2
        #if MTK_CAM_DISPAY_FRAME_CONTROL_ON
        pResultProcessor->registerListener(
                            eSTREAMID_META_HAL_DYNAMIC_P1_MAIN2,
                            pTimestampProcessor);
        #else
        pResultProcessor->registerListener(
                            eSTREAMID_META_APP_DYNAMIC_P1_MAIN2,
                            pTimestampProcessor);
        #endif
    }
    for( size_t idx=0; idx<spImageStreamMng->getSize(); idx++)
    {
        sp<ImageStreamManager::BufProviderMetaInfoSet> spSet =
                    spImageStreamMng->getBufProviderMetaInfoSet(idx);
        if ( spSet->bufProvider != NULL )
        {
            for ( size_t i = 0; i < spSet->vMetaStream.size(); ++i ) {
                pTimestampProcessor->registerCB(spSet->bufProvider);
            }
        }
    }

    //
    //
    // 4. Legacy Pipeline **************
    //
    MY_LOGD("Legacy Pipeline +");
    // add 3a early callback to main1
    if(mOpenId == sensorId)
    {
        MY_LOGD("add 3a early callback to main1(%d)", mOpenId);
        pPipelineContext->setDataCallback(pResultProcessor);
    }
    spLegacyPipeline->setPipelineContext(pPipelineContext);
    spLegacyPipeline->setRequestBuilder(pRequestBuilder);
    spLegacyPipeline->setResultProcessor(pResultProcessor);
    spLegacyPipeline->setTimestampProcessor(pTimestampProcessor);
    //set Managers
    spLegacyPipeline->setMetaStreamManager(spMetaStreamMng);
    spLegacyPipeline->setImageStreamManager(spImageStreamMng);
    spLegacyPipeline->setNodeConfigDataManager(spNodeConfigDataMng);
    //
    spLegacyPipeline->setMetaStreamId(mHalMetaStreamId, mAppMetaStreamId);

    //
    //
    // 5. Set default request builder to StereoLegacyPipeline **************
    //
    if(mpNodeConfigDataManager_default != nullptr){
        MY_LOGD("substitute default request builder, openId=%d", getOpenId());
        sp<RequestBuilder> aRequestBuilder = new RequestBuilder();
        for(size_t i=0; i<mpNodeConfigDataManager_default->getSize(); i++){
            NodeId_T nodeId = mpNodeConfigDataManager_default->getNodeId(i);
            aRequestBuilder->setIOMap(
                nodeId,
                mpNodeConfigDataManager_default->getImageIOMap(nodeId),
                mpNodeConfigDataManager_default->getMetaIOMap(nodeId)
            );
        }

        aRequestBuilder->setRootNode(
            mpNodeConfigDataManager_default->getRootNodeId()
        );
        aRequestBuilder->setNodeEdges(
            mpNodeConfigDataManager_default->getNodeEdgeSet()
        );
        aRequestBuilder->updateFrameCallback(pResultProcessor);

        spLegacyPipeline->setRequestBuilder(aRequestBuilder);
    }

    //
    //
    // 6. Set more node configs(for creating request builder) to StereoLegacyPipeline **************
    //
    if(mvNodeConfigDataManager_requests.size() > 0){
        MY_LOGD("add more node data mgr to stereo legacy pipeline: %d", mvNodeConfigDataManager_requests.size());
        spLegacyPipeline->setNodeConfigDataManagers(mvNodeConfigDataManager_requests);
    }
    MY_LOGD("Legacy Pipeline -");

    return spLegacyPipeline;
}


