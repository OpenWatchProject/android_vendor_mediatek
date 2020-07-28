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
#define LOG_TAG "TestSuite_StereoPipeMgrPrvP1"
//
#include <gtest/gtest.h>
#include <sys/prctl.h>
//
#include <mtkcam/utils/std/Log.h>
//
#include <list>
#include <stdlib.h>
#include <utils/Errors.h>
#include <utils/List.h>
#include <utils/RefBase.h>
//
#include <utils/StrongPointer.h>
//
#include <mtkcam/drv/IHalSensor.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include <mtkcam/middleware/v1/LegacyPipeline/ILegacyPipeline.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/StereoLegacyPipelineDef.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/StereoLegacyPipelineBuilder.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/ContextBuilder/ImageStreamManager.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/StreamBufferProviderFactory.h>

#include <mtkcam/middleware/v1/LegacyPipeline/NodeId.h>
//
#include <camutils/IImgBufQueue.h>
#include "../../../../v1/adapter/buffer/ClientBufferPool.h"
//
#include <mtkcam/pipeline/hwnode/P1Node.h>
//
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metastore/ITemplateRequest.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/StereoPipelineData.h>
//
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
//
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v1::NSLegacyPipeline;
using namespace StereoPipelineMgrData::p1::prv;
using namespace StereoPipelineMgrData::p2::prv;
using namespace StereoPipelineMgrData::p2::cap;
using namespace StereoPipelineMgrData::p1::VdoRec;
using namespace StereoPipelineMgrData::p1::zsd;
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
#define FUNCTION_IN                 MY_LOGD_IF(1, "+")
#define FUNCTION_OUT                MY_LOGD_IF(1, "-")
//
namespace TestSuiteStereoLegacyPipelinePrvP1
{
    #define USER_NAME   "TestStereoLgcyPipe_PrvP1"
    //open sensor ID
    static MINT32 gSensorId = 0;
    static MINT32 gSensorId_Main2 = 2;

    P1Node::SensorParams gSensorParam;
    P1Node::SensorParams gSensorParam_Main2;

    MSize    gRrzoSize;
    MINT     gRrzoFormat = eImgFmt_FG_BAYER10;
    size_t   gRrzoStride;

    MSize    gImgoSize;
    MINT     gImgoFormat = eImgFmt_BAYER10;
    size_t   gImgoStride;

    IHalSensor* mpSensorHalObj;
    sp<ILegacyPipeline> gspLegacyPipeline = NULL;
    sp<ILegacyPipeline> gspLegacyPipeline_Main2 = NULL;
    sp<ILegacyPipeline> gspLegacyPipeline_P2 = NULL;

    sp<StereoLegacyPipelineBuilder> gspBuilder;

    sp<MetaStreamManager> gspMetaStrmManager = NULL;
    sp<ImageStreamManager> gspImgStrmManager = NULL;
    sp<NodeConfigDataManager> gspNodeConfigManager = NULL;

    sp<MetaStreamManager> gspMetaStrmManager_p2 = NULL;
    sp<ImageStreamManager> gspImgStrmManager_p2 = NULL;
    sp<NodeConfigDataManager> gspNodeConfigManager_p2 = NULL;

    enum {
        PipelineMode_PREVIEW,
        PipelineMode_RECORDING,
        PipelineMode_ZSD
    };

    MUINT32 gPipelineMode = PipelineMode_PREVIEW;
};

using namespace TestSuiteStereoLegacyPipelinePrvP1;

#define NS_PER_SEC  1000000000
#define NS_PER_MS   1000000
#define NS_PER_US   1000
static void Wait(int ms)
{
  long waitSec;
  long waitNSec;
  waitSec = (ms * NS_PER_MS) / NS_PER_SEC;
  waitNSec = (ms * NS_PER_MS) % NS_PER_SEC;
  struct timespec t;
  t.tv_sec = waitSec;
  t.tv_nsec = waitNSec;
  if( nanosleep(&t, NULL) != 0 )
  {

  }
}
//
Mutex           gLock;
Condition       gCond;
pthread_t               mThreadTimer;
pthread_t               mThreadTimer_Main2;
pthread_t               mThreadTimer_P2;
static void*    doThreadTimer(void* arg)
{
    ::prctl(PR_SET_NAME,"TimerThread", 0, 0, 0);

    if(arg != NULL)
    {
        StereoLegacyPipelineBuilder* ptr = reinterpret_cast<StereoLegacyPipelineBuilder*>(arg);
        gspLegacyPipeline = ptr->create(gspMetaStrmManager, gspImgStrmManager, gspNodeConfigManager);
    }
    Wait(1000);
    gCond.signal();
    return NULL;
}
static void*    doThreadTimer_Main2(void* arg)
{
    ::prctl(PR_SET_NAME,"TimerThreadMain2", 0, 0, 0);

    if(arg != NULL)
    {
        StereoLegacyPipelineBuilder* ptr = reinterpret_cast<StereoLegacyPipelineBuilder*>(arg);
        gspLegacyPipeline_Main2 = ptr->create(gspMetaStrmManager, gspImgStrmManager, gspNodeConfigManager);
    }
    Wait(1000);
    gCond.signal();
    return NULL;
}
static void*    doThreadTimer_P2(void* arg)
{
    ::prctl(PR_SET_NAME,"TimerThread", 0, 0, 0);

    if(arg != NULL)
    {
        StereoLegacyPipelineBuilder* ptr = reinterpret_cast<StereoLegacyPipelineBuilder*>(arg);
        gspLegacyPipeline_P2 = ptr->create(gspMetaStrmManager_p2, gspImgStrmManager_p2, gspNodeConfigManager_p2);
    }
    Wait(1000);
    gCond.signal();
    return NULL;
}

void stopPipeline()
{
    FUNCTION_IN;

    if(gspLegacyPipeline != NULL) {
        gspLegacyPipeline->flush();
        gspLegacyPipeline->waitUntilDrained();
        gspLegacyPipeline = NULL;
    }

    if(gspLegacyPipeline_Main2 != NULL){
        gspLegacyPipeline_Main2->flush();
        gspLegacyPipeline_Main2->waitUntilDrained();
        gspLegacyPipeline_Main2 = NULL;
    }

    if(gspLegacyPipeline_P2 != NULL) {
        gspLegacyPipeline_P2->flush();
        gspLegacyPipeline_P2->waitUntilDrained();
        gspLegacyPipeline_P2 = NULL;
    }
    FUNCTION_OUT;
}

void resetGlobalVars()
{
    FUNCTION_IN;

    if(mpSensorHalObj != NULL) {
        mpSensorHalObj->destroyInstance(USER_NAME);
        mpSensorHalObj = NULL;
    }

    if(gspLegacyPipeline != NULL) {
        gspLegacyPipeline->flush();
        gspLegacyPipeline->waitUntilDrained();
        gspLegacyPipeline = NULL;
    }

    if(gspLegacyPipeline_Main2 != NULL){
        gspLegacyPipeline_Main2->flush();
        gspLegacyPipeline_Main2->waitUntilDrained();
        gspLegacyPipeline_Main2 = NULL;
    }

    if(gspLegacyPipeline_P2 != NULL) {
        gspLegacyPipeline_P2->flush();
        gspLegacyPipeline_P2->waitUntilDrained();
        gspLegacyPipeline_P2 = NULL;
    }

    if(gspBuilder != NULL) {
        gspBuilder = NULL;
    }

    if(gspMetaStrmManager != NULL) {
        gspMetaStrmManager->destroy();
        gspMetaStrmManager = NULL;
    }
    if(gspImgStrmManager != NULL) {
        gspImgStrmManager->destroy();
        gspImgStrmManager = NULL;
    }
    if(gspNodeConfigManager != NULL) {
        gspNodeConfigManager->destroy();
        gspNodeConfigManager = NULL;
    }

    if(mpSensorHalObj != NULL){
        mpSensorHalObj->destroyInstance(USER_NAME);
        mpSensorHalObj = NULL;
    }

    FUNCTION_OUT;
}

void closeSensor(MUINT32 sensorId)
{
    FUNCTION_IN;
    //
    MUINT32    sensorArray[1] = {sensorId};
    mpSensorHalObj->powerOff(USER_NAME, 1, &sensorArray[0]);

    FUNCTION_OUT;
}

void powerOnSensor(MUINT32 sensorId){
    FUNCTION_IN;

    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
    pHalSensorList->searchSensors();

    MUINT32    sensorArray[1] = {sensorId};
    mpSensorHalObj = pHalSensorList->createSensor(
                                        USER_NAME,
                                        1,
                                        sensorArray);
    if( ! mpSensorHalObj ) {
        MY_LOGE("create sensor(%d) failed", sensorId);
        return;
    }

    if( !mpSensorHalObj->powerOn(USER_NAME, 1, &sensorArray[0]) )
    {
        MY_LOGE("sensor power on failed: %d", sensorId);
        return;
    }

    FUNCTION_OUT;
}

void powerOnSensorDual(MUINT32 sensorId, MUINT32 sensorId_main2){
    FUNCTION_IN;

    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
    pHalSensorList->searchSensors();

    MUINT32    sensorArray[2] = {sensorId, sensorId_main2};

    MY_LOGD("powerOnSensorDual create sensor");

    mpSensorHalObj = pHalSensorList->createSensor(
                                        USER_NAME,
                                        2,
                                        sensorArray);
    if( ! mpSensorHalObj ) {
        MY_LOGE("create sensor(%d/%d) failed", sensorId, sensorId_main2);
        return;
    }

    // MY_LOGD("powerOnSensorDual powerOn:%d", sensorArray[0]);

    // if( !mpSensorHalObj->powerOn(USER_NAME, 1, &sensorArray[0]) )
    // {
    //     MY_LOGE("sensor power on failed: %d", sensorArray[0]);
    //     return;
    // }

    // MY_LOGD("powerOnSensorDual powerOn:%d", sensorArray[1]);

    // if( !mpSensorHalObj->powerOn(USER_NAME, 1, &sensorArray[1]) )
    // {
    //     MY_LOGE("sensor power on failed: %d", sensorArray[1]);
    //     return;
    // }

    MY_LOGD("powerOnSensorDual powerOn:{%d/%d}", sensorArray[0], sensorArray[1]);

    if( !mpSensorHalObj->powerOn(USER_NAME, 2, sensorArray) )
    {
        MY_LOGE("sensor power on failed:{%d/%d}", sensorArray[0], sensorArray[1]);
        return;
    }

    FUNCTION_OUT;
}

void powerOffSensorDual(MUINT32 sensorId, MUINT32 sensorId_main2){
    FUNCTION_IN;

    if(mpSensorHalObj)
    {
        MUINT pIndex[2] = { (MUINT)sensorId, (MUINT)sensorId_main2 };
        mpSensorHalObj->powerOff(USER_NAME, 2, pIndex);
        mpSensorHalObj->destroyInstance(USER_NAME);
    }else{
        MY_LOGE("power off sensor without mpSensorHalObj!");
    }

    FUNCTION_OUT;
}

void prepareSensorConfig(MUINT32 sensorId, P1Node::SensorParams* pSensorParam)
{
    FUNCTION_IN;

    // Sensor Information Query
    {
        IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
        MUINT32 sensorDev = pHalSensorList->querySensorDevIdx(sensorId);
        SensorStaticInfo sensorStaticInfo;
        memset(&sensorStaticInfo, 0, sizeof(SensorStaticInfo));
        pHalSensorList->querySensorStaticInfo(sensorDev, &sensorStaticInfo);

        MY_LOGD("querySensorStaticInfo: prv(%dx%d@%d)", sensorStaticInfo.previewWidth, sensorStaticInfo.previewHeight, sensorStaticInfo.previewFrameRate/10);
        MY_LOGD("querySensorStaticInfo: cap(%dx%d@%d)", sensorStaticInfo.captureWidth, sensorStaticInfo.captureHeight, sensorStaticInfo.captureFrameRate/10);
        MY_LOGD("querySensorStaticInfo: vdo(%dx%d@%d)", sensorStaticInfo.videoWidth, sensorStaticInfo.videoHeight, sensorStaticInfo.videoFrameRate/10);

        //
        switch(gPipelineMode)
        {
        case PipelineMode_PREVIEW:
            (*pSensorParam).mode = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
            (*pSensorParam).size = MSize(sensorStaticInfo.previewWidth, sensorStaticInfo.previewHeight);
            (*pSensorParam).fps = sensorStaticInfo.previewFrameRate/10;
            break;
        case PipelineMode_ZSD:
            if(sensorId == gSensorId_Main2) //for Main2, use Preview Scenario (FIX)
            {
                (*pSensorParam).mode = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
                (*pSensorParam).size = MSize(sensorStaticInfo.previewWidth, sensorStaticInfo.previewHeight);
                (*pSensorParam).fps = sensorStaticInfo.previewFrameRate/10;
            }
            else {
                (*pSensorParam).mode = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
                (*pSensorParam).size = MSize(sensorStaticInfo.captureWidth, sensorStaticInfo.captureHeight);
                (*pSensorParam).fps = sensorStaticInfo.captureFrameRate/10;
            }
            break;
        case PipelineMode_RECORDING:
            (*pSensorParam).mode = SENSOR_SCENARIO_ID_NORMAL_VIDEO;
            (*pSensorParam).size = MSize(sensorStaticInfo.videoWidth, sensorStaticInfo.videoHeight);
            (*pSensorParam).fps = sensorStaticInfo.videoFrameRate/10;
            break;
        }
        //(*pSensorParam).fps = 15;
        (*pSensorParam).pixelMode = 0;
        mpSensorHalObj->sendCommand(
                pHalSensorList->querySensorDevIdx(sensorId),
                SENSOR_CMD_GET_SENSOR_PIXELMODE,
                (MUINTPTR)(&(*pSensorParam).mode),
                (MUINTPTR)(&(*pSensorParam).fps),
                (MUINTPTR)(&(*pSensorParam).pixelMode));
    }
    MY_LOGD("Sensor mode(%d), fps(%d), size(%d,%d), pixelMode(%d)",
            (*pSensorParam).mode,
            (*pSensorParam).fps,
            (*pSensorParam).size.w, (*pSensorParam).size.h,
            (*pSensorParam).pixelMode);
    FUNCTION_OUT;
}

void prepareConfiguration(MBOOL bMain1, MUINT32 pipelineMode)
{
    MUINT32 stereoMode = StereoHAL::eSTEREO_SCENARIO_PREVIEW;
    switch(pipelineMode)
    {
    default:                        MY_LOGE("unknown pipeline mode(%d)!!", pipelineMode);  break;
    case PipelineMode_PREVIEW:      stereoMode = StereoHAL::eSTEREO_SCENARIO_PREVIEW;       break;
    case PipelineMode_ZSD:          stereoMode = StereoHAL::eSTEREO_SCENARIO_CAPTURE;       break;
    case PipelineMode_RECORDING:    stereoMode = StereoHAL::eSTEREO_SCENARIO_RECORD;       break;
    }

    //RRZO
    {
        StereoSizeProvider::getInstance()->getPass1Size(
                            (bMain1)?(StereoHAL::eSTEREO_SENSOR_MAIN1):(StereoHAL::eSTEREO_SENSOR_MAIN2),
                            (EImageFormat)gRrzoFormat,
                            NSImageio::NSIspio::EPortIndex_RRZO,
                            (StereoHAL::ENUM_STEREO_SCENARIO)stereoMode,
                            gRrzoSize,
                            (MUINT32&)gRrzoStride);

        MY_LOGD("rrzo format 0x%x size %dx%d, stride %d",
                gRrzoFormat,
                gRrzoSize.w, gRrzoSize.h,
                gRrzoStride);
    }
    //IMGO
    {
        StereoSizeProvider::getInstance()->getPass1Size(
                            (bMain1)?(StereoHAL::eSTEREO_SENSOR_MAIN1):(StereoHAL::eSTEREO_SENSOR_MAIN2),
                            (EImageFormat)gImgoFormat,
                            NSImageio::NSIspio::EPortIndex_IMGO,
                            (StereoHAL::ENUM_STEREO_SCENARIO)stereoMode,
                            gImgoSize,
                            (MUINT32&)gImgoStride);

        MY_LOGD("imgo format 0x%x size %dx%d, stride %d",
                gImgoFormat,
                gImgoSize.w, gImgoSize.h,
                gImgoStride);
    }
}

MBOOL constructPipeline_P1(
    MINT32 sensorId,
    char* const name,
    MINT32 mode,
    const MetaStreamManager::metadata_info_setting* pMetaInfoTbl,
    ImageStreamManager::image_stream_info_pool_setting* pStrmInfoTbl,
    const MINT32* pNodeTbl,
    P1Node::SensorParams* pSensorParam
    )
{
    FUNCTION_IN;
    Mutex::Autolock _l(gLock);

    sp<StereoLegacyPipelineBuilder> pBuilder = new StereoLegacyPipelineBuilder(sensorId, name, mode);

    if ( pBuilder == 0 ) {
        MY_LOGE("Cannot create LegacyPipelineBuilder.");
        return MFALSE;
    }
    StereoPipelineSensorParam sensorParam;
    sensorParam.mode = (*pSensorParam).mode;
    sensorParam.size = MSize((*pSensorParam).size.w, (*pSensorParam).size.h);
    sensorParam.fps = (*pSensorParam).fps;
    sensorParam.pixelMode = (*pSensorParam).pixelMode;
    //
    MY_LOGD("sensor mode:%d size:%dx%d, fps:%d pixel:%d",
        sensorParam.mode,
        sensorParam.size.w, sensorParam.size.h,
        sensorParam.fps,
        sensorParam.pixelMode);
    pBuilder->setSrc(sensorParam);

    prepareConfiguration((sensorId==gSensorId)?(MTRUE):(MFALSE), gPipelineMode);

    // Image
    {
        // TODO:
        if(    pStrmInfoTbl[0].streamId == eSTREAMID_IMAGE_PIPE_RAW_RESIZER
           ||  pStrmInfoTbl[0].streamId == eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01
          )
        {
            pStrmInfoTbl[0].imgFormat = gRrzoFormat;
            pStrmInfoTbl[0].imgSize = MSize(gRrzoSize.w, gRrzoSize.h);
            pStrmInfoTbl[0].stride = gRrzoStride;
        }
        else {
            MY_LOGE("RRZO Error.");
            return MFALSE;
        }
        if(    pStrmInfoTbl[1].streamId == eSTREAMID_IMAGE_PIPE_RAW_OPAQUE
           ||  pStrmInfoTbl[1].streamId == eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01
          )
        {
            pStrmInfoTbl[1].imgFormat = gImgoFormat;
            pStrmInfoTbl[1].imgSize = MSize(gImgoSize.w, gImgoSize.h);
            pStrmInfoTbl[1].stride = gImgoStride;
        }
        gspMetaStrmManager = MetaStreamManager::create(pMetaInfoTbl);
        gspImgStrmManager = ImageStreamManager::create(pStrmInfoTbl);

        const NodeConfigDataManager::NodeConfigMgrSetting cfgSetting =
                                        {
                                            gSensorId,
                                            gSensorId_Main2,
                                            pNodeTbl
                                        };
        gspNodeConfigManager = NodeConfigDataManager::create(name, &cfgSetting);
        //
        MY_LOGD("spImgStrmManager->getSize() = %d", gspImgStrmManager->getSize());
        //
        Vector<StreamId_T > vMetaStreamInfoSet;
        //Add APP_CONTROL first (MUST BE)
        vMetaStreamInfoSet.add(eSTREAMID_META_APP_CONTROL);
        if(sensorId == gSensorId_Main2)
        {
            vMetaStreamInfoSet.add(eSTREAMID_META_APP_DYNAMIC_P1_MAIN2);
            vMetaStreamInfoSet.add(eSTREAMID_META_HAL_DYNAMIC_P1_MAIN2);
            pBuilder->setMetaStreamId(
                        eSTREAMID_META_HAL_CONTROL_MAIN2,
                        eSTREAMID_META_APP_CONTROL_MAIN2);
        }
        else {
            vMetaStreamInfoSet.add(eSTREAMID_META_APP_DYNAMIC_P1);
            vMetaStreamInfoSet.add(eSTREAMID_META_HAL_DYNAMIC_P1);
            pBuilder->setMetaStreamId(
                        eSTREAMID_META_HAL_CONTROL,
                        eSTREAMID_META_APP_CONTROL);
        }

        for(MUINT32 i=0; i<gspImgStrmManager->getSize(); i++)
        {
            sp<IImageStreamInfo> spStrmInfo = gspImgStrmManager->getStreamInfoByIndex(i);
            if(gspImgStrmManager->getPoolTypeByIndex(i) == NSPipelineContext::eStreamType_IMG_HAL_PROVIDER)
            {
                // create pairMetadata
                sp<PairMetadata> pPair;
                pPair = PairMetadata::createInstance(spStrmInfo->getStreamName());

                //Create Provider if need
                sp<StreamBufferProviderFactory> pFactory = StreamBufferProviderFactory::createInstance();
                pFactory->setImageStreamInfo(spStrmInfo);
                pFactory->setPairRule(pPair, vMetaStreamInfoSet.size());
                sp<StreamBufferProvider> pProducer = pFactory->create(sensorId, MTRUE);
                //
                gspImgStrmManager->updateBufProvider(
                                            spStrmInfo->getStreamId(),
                                            pProducer,
                                            vMetaStreamInfoSet);
            }
            MY_LOGD("idx(%d) StreamId(%d), Name(%s) BufferType(%d), BufProvider(0x%X)",
                        i,
                        spStrmInfo->getStreamId(),
                        spStrmInfo->getStreamName(),
                        gspImgStrmManager->getPoolTypeByIndex(i),
                        gspImgStrmManager->getBufProviderByIndex(i).get()
                        );
        }
    }


    if(sensorId == gSensorId_Main2){
        if( pthread_create(&mThreadTimer_Main2, NULL, doThreadTimer_Main2, pBuilder.get()) != 0 )
        {
            MY_LOGE("mThreadTimer_Main2 create failed");
        }
    }else{
        if( pthread_create(&mThreadTimer, NULL, doThreadTimer, pBuilder.get()) != 0 )
        {
            MY_LOGE("mThreadTimer create failed");
        }
    }

    //
    MY_LOGD("wait_s");
    gCond.wait(gLock);
    MY_LOGD("wait_e");

    FUNCTION_OUT;
    return MTRUE;
}

MBOOL constructPipeline_P2(
    MINT32 sensorId,
    char* const name,
    MINT32 mode,
    const MetaStreamManager::metadata_info_setting* pMetaInfoTbl,
    ImageStreamManager::image_stream_info_pool_setting* pStrmInfoTbl,
    const MINT32* pNodeTbl
    )
{
#if 0
    FUNCTION_IN;
    // create legacy pipeline builder
    sp<StereoLegacyPipelineBuilder> pBuilder =
            new StereoLegacyPipelineBuilder(
                                        sensorId,
                                        "P2_Pipeline",
                                        StereoLegacyPipelineMode_T::STPipelineMode_P2);
    if ( pBuilder == 0 ) {
        MY_LOGE("Cannot create LegacyPipelineBuilder.");
        return MFALSE;
    }
    gspMetaStrmManager_p2 = MetaStreamManager::create(pMetaInfoTbl);
    if(gspMetaStrmManager_p2 == NULL) {
        MY_LOGE("Create MetaStreamManager fail.");
        return NULL;
    }
    //
    gspImgStrmManager_p2 = ImageStreamManager::create(pStrmInfoTbl);
    if(gspImgStrmManager_p2 == NULL) {
        MY_LOGE("Create ImageStreamManager fail.");
        return NULL;
    }
    //
    const NodeConfigDataManager::NodeConfigMgrSetting cfgSetting =
                                    {
                                        gSensorId,
                                        gSensorId_Main2,
                                        pNodeTbl
                                    };
    gspNodeConfigManager_p2 = NodeConfigDataManager::create("PrvP2", &cfgSetting);
    if(gspNodeConfigManager_p2 == NULL) {
        MY_LOGE("Create NodeConfigDataManager fail.");
        return NULL;
    }

    // Set provider
    // 1. set Full raw main1
    {
        android::sp<StreamBufferProvider> pImgoProvider =
                ConsumerContainer::getInstance()->queryConsumer(
                                        sensorId,
                                        eSTREAMID_IMAGE_PIPE_RAW_OPAQUE);
        if(pImgoProvider == NULL)
        {
            MY_LOGE("query main1 imgo consumer fail");
            //return NULL;
        }
        gspImgStrmManager_p2->updateBufProvider(
                                        eSTREAMID_IMAGE_PIPE_RAW_OPAQUE,
                                        pImgoProvider,
                                        NULL,
                                        Vector<StreamId_T>());
    }
    // 2. set resized raw main1
    {
        android::sp<StreamBufferProvider> pRrzoProvider =
                ConsumerContainer::getInstance()->queryConsumer(
                                        sensorId,
                                        eSTREAMID_IMAGE_PIPE_RAW_RESIZER);
        if(pRrzoProvider == NULL)
        {
            MY_LOGE("query main1 rrzo consumer fail");
            //return NULL;
        }
        gspImgStrmManager_p2->updateBufProvider(
                                        eSTREAMID_IMAGE_PIPE_RAW_RESIZER,
                                        pRrzoProvider,
                                        NULL,
                                        Vector<StreamId_T>());
    }
    // 3. set resized raw main2
    {
        android::sp<StreamBufferProvider> pImgoProvider =
                ConsumerContainer::getInstance()->queryConsumer(
                                        sensorId,
                                        eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01);
        if(pImgoProvider == NULL)
        {
            MY_LOGE("query main2 rrzp consumer fail");
            //return NULL;
        }
        gspImgStrmManager_p2->updateBufProvider(
                                        eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01,
                                        pImgoProvider,
                                        NULL,
                                        Vector<StreamId_T>());
    }
    // 4. set CamClient
    {
        Vector<StreamId_T> vMetaStreamInfoSet;
        vMetaStreamInfoSet.add(eSTREAMID_IMAGE_PIPE_BOKEHNODE_PREVIEWYUV);
        //
        sp<StreamBufferProviderFactory> pFactory = StreamBufferProviderFactory::createInstance();
        sp<IImageStreamInfo> pImageInfo = gspImgStrmManager_p2->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_BOKEHNODE_PREVIEWYUV);
//        sp<ClientBufferPool> pClient = new ClientBufferPool();
/*        {
            pClient->setCamClient(
                            pImageInfo->getStreamName(),
                            mpImgBufProvidersMgr,
                            IImgBufProvider::eID_DISPLAY);
        }*/
        pFactory->setName(pImageInfo->getStreamName());
        pFactory->setImageStreamInfo(pImageInfo);
//        pFactory->setUsersPool(pClient);
        gspImgStrmManager_p2->updateBufProvider(
                                        pImageInfo->getStreamId(),
                                        pFactory->create(sensorId),
                                        NULL,
                                        vMetaStreamInfoSet);
    }
    // 5. set FD
    {
        Vector<StreamId_T> vMetaStreamInfoSet;
        vMetaStreamInfoSet.add(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_FDYUV);
        //
        sp<StreamBufferProviderFactory> pFactory = StreamBufferProviderFactory::createInstance();
        sp<IImageStreamInfo> pImageInfo = gspImgStrmManager_p2->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_FDYUV);
//        sp<ClientBufferPool> pClient = new ClientBufferPool();
        /*{
            pClient->setCamClient(
                            pImageInfo->getStreamName(),
                            mpImgBufProvidersMgr,
                            IImgBufProvider::eID_FD);
        }*/
        pFactory->setName(pImageInfo->getStreamName());
        pFactory->setImageStreamInfo(pImageInfo);
//        pFactory->setUsersPool(pClient);
        gspImgStrmManager_p2->updateBufProvider(
                                        pImageInfo->getStreamId(),
                                        pFactory->create(sensorId),
                                        NULL,
                                        vMetaStreamInfoSet);
    }

    pBuilder->setMetaStreamId(
                eSTREAMID_META_HAL_DYNAMIC_P1,
                eSTREAMID_META_APP_CONTROL);

    if( pthread_create(&mThreadTimer_P2, NULL, doThreadTimer_P2, pBuilder.get()) != 0 )
    {
        MY_LOGE("pthread create failed");
    }
    //
    MY_LOGD("wait_s");
    gCond.wait(gLock);
    MY_LOGD("wait_e");

    FUNCTION_OUT;
#endif
    return MTRUE;
}


MERROR createDefaultRequest(
        MINT32 sensorId,
        int templateId,
        IMetadata &request /*out*/
)
{

    ITemplateRequest* obj = NSTemplateRequestManager::valueFor(sensorId);
    if(obj == NULL) {
        obj = ITemplateRequest::getInstance(sensorId);
        NSTemplateRequestManager::add(sensorId, obj);
    }
    request = obj->getMtkData(templateId);

    return OK;
}


TEST(StereoLegacyPiplinePrvP1, test01)
{
    MY_LOGD("+");
    gPipelineMode = PipelineMode_PREVIEW;
    int templateID = MTK_CONTROL_CAPTURE_INTENT_PREVIEW;
    IMetadata appSetting;
    IMetadata halSetting;

    prepareSensorConfig(gSensorId, &gSensorParam);
    //
    sp<IMetadataProvider> pMetadataProvider = IMetadataProvider::create(gSensorId);
    NSMetadataProviderManager::add(gSensorId, pMetadataProvider.get());
    //
    constructPipeline_P1(
                gSensorId,
                "PrvP1",
                StereoLegacyPipelineMode_T::STPipelineMode_P1,
                gStereoMetaTbl_PrvP1,
                gStereoImgStreamTbl_PrvP1,
                gStereoConnectSetting_PrvP1,
                &gSensorParam);
    //
    EXPECT_FALSE(gspLegacyPipeline == NULL) << "LegacyPipelie is NULL";
    if(gspLegacyPipeline == NULL) {
        MY_LOGE("gspLegacyPipeline is NULL!!");
        goto lbExit;
    }

    //Queue one request to PipelineContext to avoid P1Node wait problem
    {
        createDefaultRequest(gSensorId, templateID, appSetting);
        gspLegacyPipeline->submitSetting(0, appSetting, halSetting);
        Wait(100);
    }

    // Buffer Provider
    EXPECT_FALSE(gspLegacyPipeline->queryProvider(eSTREAMID_IMAGE_PIPE_RAW_RESIZER) == NULL) << "Resize provider is NULL!!";
    // Image stream
    EXPECT_FALSE(gspLegacyPipeline->queryImageStreamInfo(eSTREAMID_IMAGE_PIPE_RAW_RESIZER) == NULL) << "Resize StreamInfo is NULL!!";
    //EXPECT_FALSE(gspLegacyPipeline->queryImageStreamInfo(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE) == NULL) << "Fullsize StreamInfo is NULL!!";
    // Meta Stream
    EXPECT_FALSE(gspLegacyPipeline->queryMetaStreamInfo(eSTREAMID_META_APP_CONTROL) == NULL) << "AppControl MetaStreamInfo is NULL!!";
    EXPECT_FALSE(gspLegacyPipeline->queryMetaStreamInfo(eSTREAMID_META_HAL_CONTROL) == NULL) << "HalControl MetaStreamInfo is NULL!!";

lbExit:
    stopPipeline();
    closeSensor(gSensorId);
    //
    resetGlobalVars();
    MY_LOGD("-");
}

TEST(StereoLegacyPiplineZsdP1, test01)
{
    MY_LOGD("+");

    gPipelineMode = PipelineMode_ZSD;
    int templateID = MTK_CONTROL_CAPTURE_INTENT_PREVIEW;
    IMetadata appSetting;
    IMetadata halSetting;
    //
    prepareSensorConfig(gSensorId, &gSensorParam);
    //
    sp<IMetadataProvider> pMetadataProvider = IMetadataProvider::create(gSensorId);
    NSMetadataProviderManager::add(gSensorId, pMetadataProvider.get());
    //
    constructPipeline_P1(
                gSensorId,
                "ZsdP1",
                StereoLegacyPipelineMode_T::STPipelineMode_P1,
                gStereoMetaTbl_PrvP1,
                gStereoImgStreamTbl_ZsdP1,
                gStereoConnectSetting_PrvP1,
                &gSensorParam_Main2);
    //
    EXPECT_FALSE(gspLegacyPipeline == NULL) << "LegacyPipelie is NULL";
    if(gspLegacyPipeline == NULL) {
        MY_LOGE("gspLegacyPipeline is NULL!!");
        goto lbExit;
    }

    //Queue one request to PipelineContext to avoid P1Node wait problem
    {
        createDefaultRequest(gSensorId, templateID, appSetting);
        gspLegacyPipeline->submitSetting(0, appSetting, halSetting);
        Wait(100);
    }

    // Buffer Provider
    EXPECT_FALSE(gspLegacyPipeline->queryProvider(eSTREAMID_IMAGE_PIPE_RAW_RESIZER) == NULL) << "Resize provider is NULL!!";
    EXPECT_FALSE(gspLegacyPipeline->queryProvider(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE) == NULL) << "Fullsize provider is NULL!!";
    // Image Stream
    EXPECT_FALSE(gspLegacyPipeline->queryImageStreamInfo(eSTREAMID_IMAGE_PIPE_RAW_RESIZER) == NULL) << "Resize StreamInfo is NULL!!";
    EXPECT_FALSE(gspLegacyPipeline->queryImageStreamInfo(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE) == NULL) << "Fullsize StreamInfo is NULL!!";
    // Meta Stream
    EXPECT_FALSE(gspLegacyPipeline->queryMetaStreamInfo(eSTREAMID_META_APP_CONTROL) == NULL) << "AppControl MetaStreamInfo is NULL!!";
    EXPECT_FALSE(gspLegacyPipeline->queryMetaStreamInfo(eSTREAMID_META_HAL_CONTROL) == NULL) << "HalControl MetaStreamInfo is NULL!!";

lbExit:
    stopPipeline();
    closeSensor(gSensorId);
    //
    resetGlobalVars();
    MY_LOGD("-");
}

TEST(StereoLegacyPiplinePrvP1Main2, test01)
{
    MY_LOGD("+");
    gPipelineMode = PipelineMode_PREVIEW;
    int templateID = MTK_CONTROL_CAPTURE_INTENT_PREVIEW;
    IMetadata appSetting;
    IMetadata halSetting;
    //
    prepareSensorConfig(gSensorId_Main2, &gSensorParam_Main2);
    //
    sp<IMetadataProvider> pMetadataProvider = IMetadataProvider::create(gSensorId_Main2);
    NSMetadataProviderManager::add(gSensorId_Main2, pMetadataProvider.get());
    //
    constructPipeline_P1(
                gSensorId_Main2,
                "PrvP1Main2",
                StereoLegacyPipelineMode_T::STPipelineMode_P1Main2,
                gStereoMetaTbl_PrvP1Main2,
                gStereoImgStreamTbl_PrvP1Main2,
                gStereoConnectSetting_PrvP1Main2,
                &gSensorParam_Main2);
    //
    EXPECT_FALSE(gspLegacyPipeline == NULL) << "LegacyPipelie is NULL";
    if(gspLegacyPipeline == NULL) {
        MY_LOGE("gspLegacyPipeline is NULL!!");
        goto lbExit;
    }

    //Queue one request to PipelineContext to avoid P1Node wait problem
    {
        createDefaultRequest(gSensorId_Main2, templateID, appSetting);
        gspLegacyPipeline->submitSetting(0, appSetting, halSetting);
        Wait(100);
    }

    // Buffer Provider
    EXPECT_FALSE(gspLegacyPipeline->queryProvider(eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01) == NULL) << "ResizeM2 provider is NULL!!";
    // Image Stream
    EXPECT_FALSE(gspLegacyPipeline->queryImageStreamInfo(eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01) == NULL) << "ResizeM2 StreamInfo is NULL!!";
    // Meta Stream
    EXPECT_FALSE(gspLegacyPipeline->queryMetaStreamInfo(eSTREAMID_META_APP_CONTROL_MAIN2) == NULL) << "AppControl_M2 MetaStreamInfo is NULL!!";
    EXPECT_FALSE(gspLegacyPipeline->queryMetaStreamInfo(eSTREAMID_META_HAL_CONTROL_MAIN2) == NULL) << "HalControl_M2 MetaStreamInfo is NULL!!";

lbExit:
    //
    stopPipeline();
    closeSensor(gSensorId_Main2);
    //
    resetGlobalVars();
    MY_LOGD("-");
}

TEST(StereoLegacyPiplinePrvDualP1, test00)
{
    MY_LOGD("StereoLegacyPiplinePrvDualP1 + Shane");

    // gPipelineMode = PipelineMode_PREVIEW;
    gPipelineMode = PipelineMode_ZSD;
    gSensorParam.mode       = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
    gSensorParam_Main2.mode = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
    int templateID = MTK_CONTROL_CAPTURE_INTENT_PREVIEW;
    //
    // powerOnSensor(gSensorId);
    // powerOnSensor(gSensorId_Main2);
    powerOnSensorDual(gSensorId, gSensorId_Main2);
    //
    //

    // start Main1 pipeline
    {
        MY_LOGD("start pipeline Main1 +");

        prepareSensorConfig(gSensorId, &gSensorParam);

        IMetadata appSetting;
        IMetadata halSetting;
        //
        sp<IMetadataProvider> pMetadataProvider = IMetadataProvider::create(gSensorId);
        NSMetadataProviderManager::add(gSensorId, pMetadataProvider.get());
        //
        constructPipeline_P1(
                    gSensorId,
                    "PrvP1",
                    StereoLegacyPipelineMode_T::STPipelineMode_P1,
                    gStereoMetaTbl_PrvP1,
                    gStereoImgStreamTbl_PrvP1,
                    gStereoConnectSetting_PrvP1,
                    &gSensorParam);
        //
        EXPECT_FALSE(gspLegacyPipeline == NULL) << "LegacyPipelie is NULL";
        if(gspLegacyPipeline == NULL) {
            MY_LOGE("gspLegacyPipeline is NULL!!");
            goto lbExit;
        }

        //Queue one request to PipelineContext to avoid P1Node wait problem
        {
            createDefaultRequest(gSensorId, templateID, appSetting);
            gspLegacyPipeline->submitSetting(0, appSetting, halSetting);
            Wait(100);
        }

        // Buffer Provider
        EXPECT_FALSE(gspLegacyPipeline->queryProvider(eSTREAMID_IMAGE_PIPE_RAW_RESIZER) == NULL) << "Resize provider is NULL!!";
        // Image stream
        EXPECT_FALSE(gspLegacyPipeline->queryImageStreamInfo(eSTREAMID_IMAGE_PIPE_RAW_RESIZER) == NULL) << "Resize StreamInfo is NULL!!";
        //EXPECT_FALSE(gspLegacyPipeline->queryImageStreamInfo(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE) == NULL) << "Fullsize StreamInfo is NULL!!";
        // Meta Stream
        EXPECT_FALSE(gspLegacyPipeline->queryMetaStreamInfo(eSTREAMID_META_APP_CONTROL) == NULL) << "AppControl MetaStreamInfo is NULL!!";
        EXPECT_FALSE(gspLegacyPipeline->queryMetaStreamInfo(eSTREAMID_META_HAL_CONTROL) == NULL) << "HalControl MetaStreamInfo is NULL!!";
        MY_LOGD("start pipeline Main1 -");
    }

    // start Main2 pipeline
    {
        MY_LOGD("start pipeline Main2 +");

        prepareSensorConfig(gSensorId_Main2, &gSensorParam_Main2);

        IMetadata appSetting;
        IMetadata halSetting;
        sp<IMetadataProvider> pMetadataProvider = IMetadataProvider::create(gSensorId_Main2);
        NSMetadataProviderManager::add(gSensorId_Main2, pMetadataProvider.get());
        //
        constructPipeline_P1(
                    gSensorId_Main2,
                    "PrvP1Main2",
                    StereoLegacyPipelineMode_T::STPipelineMode_P1Main2,
                    gStereoMetaTbl_PrvP1Main2,
                    gStereoImgStreamTbl_PrvP1Main2,
                    gStereoConnectSetting_PrvP1Main2,
                    &gSensorParam_Main2);
        //
        EXPECT_FALSE(gspLegacyPipeline_Main2 == NULL) << "LegacyPipelie is NULL";
        if(gspLegacyPipeline_Main2 == NULL) {
            MY_LOGE("gspLegacyPipeline is NULL!!");
            goto lbExit;
        }

        //Queue one request to PipelineContext to avoid P1Node wait problem
        {
            createDefaultRequest(gSensorId_Main2, templateID, appSetting);
            gspLegacyPipeline_Main2->submitSetting(0, appSetting, halSetting);
            Wait(100);
        }

        // Buffer Provider
        EXPECT_FALSE(gspLegacyPipeline_Main2->queryProvider(eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01) == NULL) << "ResizeM2 provider is NULL!!";
        // Image Stream
        EXPECT_FALSE(gspLegacyPipeline_Main2->queryImageStreamInfo(eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01) == NULL) << "ResizeM2 StreamInfo is NULL!!";
        // Meta Stream
        EXPECT_FALSE(gspLegacyPipeline_Main2->queryMetaStreamInfo(eSTREAMID_META_APP_CONTROL_MAIN2) == NULL) << "AppControl_M2 MetaStreamInfo is NULL!!";
        EXPECT_FALSE(gspLegacyPipeline_Main2->queryMetaStreamInfo(eSTREAMID_META_HAL_CONTROL_MAIN2) == NULL) << "HalControl_M2 MetaStreamInfo is NULL!!";
        MY_LOGD("start pipeline Main2 -");
    }

lbExit:
    //
    stopPipeline();
    // closeSensor(gSensorId);
    // closeSensor(gSensorId_Main2);
    powerOffSensorDual(gSensorId, gSensorId_Main2);
    //
    resetGlobalVars();
    MY_LOGD("StereoLegacyPiplinePrvDualP1 -");
}

TEST(ImageStreamManagerTest, updateBufProvider)
{
    MY_LOGD("+");
#if 0
    sp<MetaStreamManager> spMetaStrmManager = MetaStreamManager::create(gStereoMetaTbl_PrvP1);
    sp<ImageStreamManager> spImgStrmManager = ImageStreamManager::create(gStereoImgStreamTbl_PrvP1);

    MY_LOGD("spImgStrmManager->getSize() = %d", spImgStrmManager->getSize());
    for(MUINT32 i=0; i<spImgStrmManager->getSize(); i++)
    {
        sp<IImageStreamInfo> spStrmInfo = spImgStrmManager->getStreamInfoByIndex(i);
        if(spImgStrmManager->getPoolTypeByIndex(i) == NSPipelineContext::eStreamType_IMG_HAL_PROVIDER)
        {
            //Create Provider if need
            sp<StreamBufferProviderFactory> pFactory = StreamBufferProviderFactory::createInstance();
            pFactory->setImageStreamInfo(spStrmInfo);
            //
            Vector<StreamId_T > vMetaStreamInfoSet;
            vMetaStreamInfoSet.add(eSTREAMID_META_APP_DYNAMIC_P1);
            vMetaStreamInfoSet.add(eSTREAMID_META_HAL_DYNAMIC_P1);
            spImgStrmManager->updateBufProvider(
                                        spStrmInfo->getStreamId(),
                                        pFactory->create(0),
                                        NULL,
                                        vMetaStreamInfoSet);
        }
        MY_LOGD("idx(%d) StreamId(%d), Name(%s) BufferType(%d), BufProvider(0x%X)",
                    i,
                    spStrmInfo->getStreamId(),
                    spStrmInfo->getStreamName(),
                    spImgStrmManager->getPoolTypeByIndex(i),
                    spImgStrmManager->getBufProviderByIndex(i).get()
                    );
    }

    spImgStrmManager->dump();
    // wait by another thread for buffer allocation, avoid NE
    if( pthread_create(&mThreadTimer, NULL, doThreadTimer, NULL))
    {
        MY_LOGE("pthread create failed");
    }
    //
    MY_LOGD("wait_s");
    gCond.wait(gLock);
    MY_LOGD("wait_e");
#endif
    MY_LOGD("-");
}

TEST(StereoLegacyPiplinePrvP2, test01)
{
    MY_LOGD("+");
#if 0
    gPipelineMode = PipelineMode_PREVIEW;
    prepareSensorConfig(gSensorId);
    //
    sp<IMetadataProvider> pMetadataProvider = IMetadataProvider::create(gSensorId);
    NSMetadataProviderManager::add(gSensorId, pMetadataProvider.get());
    //
    constructPipeline_P1(
                gSensorId,
                "PrvP1",
                StereoLegacyPipelineMode_T::STPipelineMode_P1,
                gStereoMetaTbl_PrvP1,
                gStereoImgStreamTbl_PrvP1,
                gStereoConnectSetting_PrvP1);
    //
    constructPipeline_P2(
                gSensorId,
                "PrvP2",
                StereoLegacyPipelineMode_T::STPipelineMode_P2,
                gStereoMetaTbl_P2Prv,
                gStereoImgStreamTbl_P2Prv,
                gStereoP2ConnectSetting_P2Prv);
    //
    EXPECT_FALSE(gspLegacyPipeline_P2 == NULL) << "LegacyPipelie is NULL";
    if(gspLegacyPipeline_P2 == NULL) {
        MY_LOGE("gspLegacyPipeline is NULL!!");
        goto lbExit;
    }

lbExit:
    stopPipeline();
    closeSensor(gSensorId);
    //
    resetGlobalVars();
#endif
    MY_LOGD("-");
}

