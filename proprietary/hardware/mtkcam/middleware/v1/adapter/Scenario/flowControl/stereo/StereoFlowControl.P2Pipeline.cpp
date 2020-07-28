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

#define LOG_TAG "MtkCam/StereoFlowControl"
//
#include "MyUtils.h"
//
#include <mtkcam/pipeline/utils/streaminfo/MetaStreamInfo.h>
#include <mtkcam/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam/pipeline/utils/streambuf/StreamBuffers.h>

#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include <mtkcam/middleware/v1/LegacyPipeline/NodeId.h>
#include <mtkcam/middleware/v1/IParamsManager.h>
#include <mtkcam/middleware/v1/LegacyPipeline/ILegacyPipeline.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/StereoLegacyPipelineDef.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/StereoLegacyPipelineBuilder.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/StreamBufferProviderFactory.h>

#include <mtkcam/middleware/v1/camutils/IImgBufQueue.h>
#include "../../../buffer/ClientBufferPool.h"

#include "StereoFlowControl.h"
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
//

using namespace NSCam;
using namespace NSCam::v1;
using namespace NSCam::v1::NSLegacyPipeline;
using namespace android;
using namespace NSCam::v3;
using namespace NSCamHW;
using namespace StereoHAL;

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
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
 * create p2 prv pipeline
 ******************************************************************************/
sp<NodeConfigDataManager>
StereoFlowControl::
buildNodeConfigDataManager(
    ContextBuilderContent content
)
{
    sp<MetaStreamManager> pMetaStreamManager =
            MetaStreamManager::create(content.metaTable);
    if(pMetaStreamManager == NULL)
    {
        MY_LOGE("Create MetaStreamManager fail.");
        return NULL;
    }
    //
    sp<ImageStreamManager> pImageStreamManager =
            ImageStreamManager::create(content.imageTable);
    if(pImageStreamManager == NULL)
    {
        MY_LOGE("Create ImageStreamManager fail.");
        return NULL;
    }
    //
    const NodeConfigDataManager::NodeConfigMgrSetting cfgSetting =
                                        {
                                            (MUINT32)mOpenId,
                                            (MUINT32)mOpenId_main2,
                                            content.nodeConfigData
                                        };
    sp<NodeConfigDataManager> pNodeConfigDataManager =
            NodeConfigDataManager::create("Prv", &cfgSetting);
    if(pNodeConfigDataManager == NULL)
    {
        MY_LOGE("Create NodeConfigDataManager fail.");
        return NULL;
    }
    sp<NSCam::StereoBasicParameters> userParams = new NSCam::StereoBasicParameters();
    userParams->set(NSCam::StereoBasicParameters::KEY_STEREO_CAM_MODULE_TYPE, mCurrentSensorModuleType);
    userParams->set(NSCam::StereoBasicParameters::KEY_STEREO_CAM_FEATURE_TYPE, mCurrentStereoMode);
    sp<PipelineContext> pipelineContext = nullptr;
    pNodeConfigDataManager->configNode(
                                pMetaStreamManager,
                                pImageStreamManager,
                                userParams,
                                pipelineContext);
    pMetaStreamManager = nullptr;
    pImageStreamManager = nullptr;
    return pNodeConfigDataManager;
}
/******************************************************************************
 * create p2 prv pipeline
 ******************************************************************************/
sp<ILegacyPipeline>
StereoFlowControl::
constructP2Pipeline_PrvCap_BB(
    StereoModeContextBuilderContent content,
    sp<ImageStreamManager>& spImageStreamManager
)
{
    FUNC_START;
    // create legacy pipeline builder
    sp<StereoLegacyPipelineBuilder> pBuilder =
            new StereoLegacyPipelineBuilder(
                                        getOpenId_P2Prv(),
                                        "P2_Pipeline",
                                        StereoLegacyPipelineMode_T::STPipelineMode_P2);
    if ( pBuilder == 0 ) {
        MY_LOGE("Cannot create LegacyPipelineBuilder.");
        return MFALSE;
    }
    sp<MetaStreamManager> pMetaStreamManager =
            MetaStreamManager::create(content.mFullContent.metaTable);
    if(pMetaStreamManager == NULL)
    {
        MY_LOGE("Create MetaStreamManager fail.");
        return NULL;
    }
    //Update P1 information to image stream table
    {
#define updateItem(streamId, rcContainer) \
        do {\
            sp<StreamBufferProvider> pProviderPass1 = rcContainer->queryConsumer(streamId); \
            if(pProviderPass1 != NULL){ \
                sp<IImageStreamInfo> streamInfo = pProviderPass1->queryImageStreamInfo(); \
                MINT32 index = getImageStreamIndex( content.mFullContent.imageTable, streamId); \
                if(index == -1) { \
                    MY_LOGW("Stream do not exist in table!! %#" PRIx64 " .", streamId); \
                } \
                else { \
                    content.mFullContent.imageTable[index].imgSize.w = streamInfo->getImgSize().w; \
                    content.mFullContent.imageTable[index].imgSize.h = streamInfo->getImgSize().h; \
                    content.mFullContent.imageTable[index].stride = (streamInfo->getBufPlanes()[0]).rowStrideInBytes; \
                    MY_LOGD("update streamId %#" PRIx64 "for P2 PV/VR's table %dx%d %d", \
                        streamId, \
                        content.mFullContent.imageTable[index].imgSize.w, \
                        content.mFullContent.imageTable[index].imgSize.h, \
                        content.mFullContent.imageTable[index].stride ); \
                } \
            } \
        } while(0);

        updateItem(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE, mpResourceContainier);
        updateItem(eSTREAMID_IMAGE_PIPE_RAW_RESIZER, mpResourceContainier);
        updateItem(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01, mpResourceContainierMain2);
        updateItem(eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01, mpResourceContainierMain2);
        // update lcso
        updateItem(eSTREAMID_IMAGE_PIPE_RAW_LCSO, mpResourceContainier);
        updateItem(eSTREAMID_IMAGE_PIPE_RAW_LCSO_01, mpResourceContainierMain2);
#undef updateItem
    }
    //Update MainImage/Preview/Record buffer size from SizeProvider
    {
#define updateItem(streamId, scenario) \
        do { \
            Pass2SizeInfo pass2SizeInfo; \
            StereoSizeProvider::getInstance()->getPass2SizeInfo(PASS2A, scenario, pass2SizeInfo); \
            MINT32 index = getImageStreamIndex( content.mFullContent.imageTable, streamId); \
            if(index == -1) { \
                MY_LOGW("Stream do not exist in table!! %#" PRIx64 " .", streamId); \
            } \
            else { \
                content.mFullContent.imageTable[index].imgSize.w = pass2SizeInfo.areaWDMA.size.w; \
                content.mFullContent.imageTable[index].imgSize.h = pass2SizeInfo.areaWDMA.size.h; \
                MY_LOGD("update streamId %#" PRIx64 "for P2 PV/VR's table %dx%d", \
                    streamId,content.mFullContent.imageTable[index].imgSize.w, content.mFullContent.imageTable[index].imgSize.h); \
            } \
        }while(0);

        //Record Scenario
        if(getImageStreamIndex( content.mFullContent.imageTable, eSTREAMID_IMAGE_PIPE_BOKEHNODE_RECORDYUV) != -1) {
            updateItem(eSTREAMID_IMAGE_PIPE_BOKEHNODE_PREVIEWYUV, StereoHAL::eSTEREO_SCENARIO_PREVIEW);
            updateItem(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_MAINIMAGEYUV, StereoHAL::eSTEREO_SCENARIO_RECORD);
            updateItem(eSTREAMID_IMAGE_PIPE_BOKEHNODE_RECORDYUV, StereoHAL::eSTEREO_SCENARIO_RECORD);
        }
        else {
            updateItem(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_MAINIMAGEYUV, StereoHAL::eSTEREO_SCENARIO_PREVIEW);
            updateItem(eSTREAMID_IMAGE_PIPE_BOKEHNODE_PREVIEWYUV, StereoHAL::eSTEREO_SCENARIO_PREVIEW);
        }

#undef updateItem
    }
    // update dmbg, capture main image size
    {
#define updateItem(streamId, enum, scenario) \
        do { \
            StereoArea imgSize; \
            imgSize = StereoSizeProvider::getInstance()->getBufferSize(enum, scenario); \
            MINT32 index = getImageStreamIndex( content.mFullContent.imageTable, streamId); \
            if(index == -1) { \
                MY_LOGW("Stream do not exist in table!! %#" PRIx64 " .", streamId); \
            } \
            else { \
                content.mFullContent.imageTable[index].imgSize.w = imgSize.size.w; \
                content.mFullContent.imageTable[index].imgSize.h = imgSize.size.h; \
                MY_LOGD("update streamId %#" PRIx64 " for P2 PV/VR's table %dx%d", \
                    streamId,content.mFullContent.imageTable[index].imgSize.w, content.mFullContent.imageTable[index].imgSize.h); \
            } \
        }while(0);
        updateItem(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_DMBGYUV, E_DMBG, StereoHAL::eSTEREO_SCENARIO_PREVIEW);
        updateItem(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_DEPTHMAPYUV, E_DEPTH_MAP, StereoHAL::eSTEREO_SCENARIO_PREVIEW);
        updateItem(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_MAINIMAGE_CAPYUV, E_BOKEH_WDMA, StereoHAL::eSTEREO_SCENARIO_CAPTURE);
#undef updateItem
    }
    // update p2 tuning buffer size
    {
        MINT32 index = getImageStreamIndex( content.mFullContent.imageTable, eSTREAMID_IMAGE_PIPE_STEREO_P2_TUNING);
        if(index == -1)
        {
            MY_LOGW("Stream do not exist in table!! %#" PRIx64 " .", eSTREAMID_IMAGE_PIPE_STEREO_P2_TUNING);
        }
        else
        {
            content.mFullContent.imageTable[index].imgSize.w = NSIoPipe::NSPostProc::INormalStream::getRegTableSize();
            content.mFullContent.imageTable[index].imgSize.h = 1;
            MY_LOGD("update streamId %#" PRIx64 " for P2 PV/VR's table %dx%d",
                eSTREAMID_IMAGE_PIPE_STEREO_P2_TUNING,
                content.mFullContent.imageTable[index].imgSize.w,
                content.mFullContent.imageTable[index].imgSize.h);
        }
    }
    //
    sp<ImageStreamManager> pImageStreamManager =
            ImageStreamManager::create(content.mFullContent.imageTable);
    if(pImageStreamManager == NULL)
    {
        MY_LOGE("Create ImageStreamManager fail.");
        return NULL;
    }
    spImageStreamManager = pImageStreamManager;
    //
    const NodeConfigDataManager::NodeConfigMgrSetting cfgSetting =
                                        {
                                            (MUINT32)mOpenId,
                                            (MUINT32)mOpenId_main2,
                                            content.mFullContent.nodeConfigData
                                        };
    sp<NodeConfigDataManager> pNodeConfigDataManager =
            NodeConfigDataManager::create("Prv", &cfgSetting);
    if(pNodeConfigDataManager == NULL)
    {
        MY_LOGE("Create NodeConfigDataManager fail.");
        return NULL;
    }

#define SET_INPUT_PROVIDER(streamId, selector, bufferPool) \
do\
{\
    sp<IImageStreamInfo> streamInfo = pImageStreamManager->getStreamInfoByStreamId(streamId);\
    if(selector != nullptr && streamInfo != nullptr)\
    {\
        bufferPool = StereoBufferPool::createInstance(streamInfo->getStreamName(), streamInfo);\
        bufferPool->setSelector(selector);\
\
        sp<StreamBufferProviderFactory> pFactory = StreamBufferProviderFactory::createInstance();\
        pFactory->setImageStreamInfo(streamInfo);\
        pFactory->setUsersPool(bufferPool);\
\
        android::sp<StreamBufferProvider> pProvider = pFactory->create(getOpenId_P2Prv(), MTRUE);\
\
        if(pProvider == NULL)\
        {\
            MY_LOGE("create " #bufferPool " failed");\
            return NULL;\
        }\
        pImageStreamManager->updateBufProvider(\
                                        streamId,\
                                        pProvider,\
                                        Vector<StreamId_T>());\
    }\
    else\
    {\
        MY_LOGE_IF(mLogLevel > 0, "get " #streamId " failed");\
    }\
}while(0);
    // Set provider
    // 1. set Full raw main1
    SET_INPUT_PROVIDER(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE, mpStereoSelector_OPAQUE, mpStereoBufferPool_OPAQUE);
    // 2. set resized raw main1
    SET_INPUT_PROVIDER(eSTREAMID_IMAGE_PIPE_RAW_RESIZER, mpStereoSelector_RESIZER, mpStereoBufferPool_RESIZER);
    // 3.1 set FULL raw main2
    SET_INPUT_PROVIDER(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01, mpStereoSelector_OPAQUE_MAIN2, mpStereoBufferPool_OPAQUE_MAIN2);
    // 3.2 set resized raw main2
    SET_INPUT_PROVIDER(eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01, mpStereoSelector_RESIZER_MAIN2, mpStereoBufferPool_RESIZER_MAIN2);
    // 3.3 set lcso for main1
    SET_INPUT_PROVIDER(eSTREAMID_IMAGE_PIPE_RAW_LCSO, mpStereoSelector_LCS ,mpStereoBufferPool_LCS);
    // 3.3 set lcso for main2
    SET_INPUT_PROVIDER(eSTREAMID_IMAGE_PIPE_RAW_LCSO_01, mpStereoSelector_LCS_MAIN2 ,mpStereoBufferPool_LCS_MAIN2);
#undef SET_INPUT_PROVIDER

    // Set CamClient
#define SET_CAM_CLIENT(streamId, providerTypeList)\
do{\
    sp<IImageStreamInfo> pImageInfo = pImageStreamManager->getStreamInfoByStreamId(streamId);\
    if(pImageInfo != nullptr && providerTypeList.size() > 0)\
    {\
        sp<StreamBufferProviderFactory> pFactory = StreamBufferProviderFactory::createInstance();\
\
        sp<ClientBufferPool> pClient = new ClientBufferPool(getOpenId_P2Prv());\
        {\
            for(auto item:providerTypeList)\
            {\
                if(IImgBufProvider::eID_GENERIC == item)\
                {\
                    pClient->setCamClient(item);\
                }\
                else\
                {\
                    pClient->setCamClient(\
                                    pImageInfo->getStreamName(),\
                                    mpImgBufProvidersMgr,\
                                    item);\
                }\
            }\
        }\
        pFactory->setImageStreamInfo(pImageInfo);\
        pFactory->setUsersPool(pClient);\
\
        android::sp<StreamBufferProvider> pProvider = pFactory->create(getOpenId_P2Prv(), MTRUE);\
\
        pImageStreamManager->updateBufProvider(\
                                        pImageInfo->getStreamId(),\
                                        pProvider,\
                                        Vector<StreamId_T>());\
    }\
    else\
    {\
        MY_LOGW_IF(mLogLevel > 0, "set " #streamId " cam client fail");\
    }\
}while(0);
    // 4-1 set display CamClient
    Vector<MINT32> lProviderType;
    lProviderType.clear();
    lProviderType.push_back(IImgBufProvider::eID_DISPLAY);
    SET_CAM_CLIENT(eSTREAMID_IMAGE_PIPE_BOKEHNODE_PREVIEWYUV, lProviderType);
    // 4-2 set preview callback
    lProviderType.clear();
    lProviderType.push_back(IImgBufProvider::eID_PRV_CB);
    lProviderType.push_back(IImgBufProvider::eID_GENERIC);
    SET_CAM_CLIENT(eSTREAMID_IMAGE_PIPE_YUV_01, lProviderType);
    // 4-3 set record CamClient
    lProviderType.clear();
    lProviderType.push_back(IImgBufProvider::eID_REC_CB);
    SET_CAM_CLIENT(eSTREAMID_IMAGE_PIPE_BOKEHNODE_RECORDYUV, lProviderType);
    // 5. set FD
    lProviderType.clear();
    lProviderType.push_back(IImgBufProvider::eID_FD);
    SET_CAM_CLIENT(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_FDYUV, lProviderType);
#undef SET_CAM_CLIENT
    // 6. set extradebug provider
    {
        auto setProvider = [this, &pImageStreamManager](MINT32 streamId, char const* name)
        {
            sp<IImageStreamInfo> pImageInfo = pImageStreamManager->getStreamInfoByStreamId(streamId);
            if(pImageInfo != NULL)
            {
                MY_LOGD("Prepare BufferProvider for %s", name);
                sp<StreamBufferProviderFactory> pFactory = StreamBufferProviderFactory::createInstance();
                sp<CallbackBufferPool> pPool = new CallbackBufferPool(pImageInfo);
                pPool->allocateBuffer(
                        pImageInfo->getStreamName(),
                        pImageInfo->getMaxBufNum(),
                        pImageInfo->getMinInitBufNum());
                pFactory->setImageStreamInfo(pImageInfo);
                pFactory->setUsersPool(pPool);

                android::sp<StreamBufferProvider> pProvider = pFactory->create(getOpenId_P2Prv(), MTRUE);

                pImageStreamManager->updateBufProvider(
                                                pImageInfo->getStreamId(),
                                                pProvider,
                                                Vector<StreamId_T>());
            }
            else {
                MY_LOGD("%s did not config in ImageStreamMnger!!", name);
            }
        };
        if(mCaptureModeType == 0) // capture use tk flow
        {
            // raw 16
            setProvider(eSTREAMID_IMAGE_PIPE_RAW16, "raw16");
            // depthmap
            setProvider(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_MAINIMAGE_CAPYUV, "capture image");
            setProvider(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_HAL_DEPTHWAPPER, "hal depth wrapper");
            setProvider(eSTREAMID_IMAGE_PIPE_STEREO_DBG, "extra debug");
            setProvider(eSTREAMID_IMAGE_PIPE_STEREO_DBG_LDC, "ldc data");
            setProvider(eSTREAMID_IMAGE_PIPE_STEREO_N3D_DEBUG, "n3d debug");
            if(!mbCanSupportBackgroundService)
            {
                // bokeh node
                setProvider(eSTREAMID_IMAGE_PIPE_JPG_CleanMainImg, "clean image");
                setProvider(eSTREAMID_IMAGE_PIPE_JPG_Bokeh, "bokeh image");
                setProvider(eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL, "thumbnail");
                setProvider(eSTREAMID_IMAGE_PIPE_STEREO_DEPTHWRAPPER, "depthmap wrapper");
                setProvider(eSTREAMID_IMAGE_PIPE_STEREO_APP_EXTRA_DATA, "app extra data");
                // Jpeg
                setProvider(eSTREAMID_IMAGE_PIPE_BOKEHNODE_CLEANIMAGEYUV, "clean image");
                setProvider(eSTREAMID_IMAGE_PIPE_BOKEHNODE_RESULTYUV, "bokeh result image");
            }
            setProvider(eSTREAMID_IMAGE_PIPE_DUALYUVNODE_THUMBNAIL, "postview");
        }
        else if(mCaptureModeType == 1) // capture use pure 3rd flow
        {
            if(!mbCanSupportBackgroundService)
            {
                setProvider(eSTREAMID_IMAGE_PIPE_RAW16, "raw16");
                setProvider(eSTREAMID_IMAGE_PIPE_YUV_JPEG, "yuv");
                setProvider(eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL, "thumbnail yuv");
            }

            // 3rd party
            setProvider(eSTREAMID_IMAGE_PIPE_THIRDPARTY_RESIZE_YUV_0, "resize yuv");
            setProvider(eSTREAMID_IMAGE_PIPE_THIRDPARTY_RESIZE_YUV_1, "resize yuv main2");
            setProvider(eSTREAMID_IMAGE_PIPE_THIRDPARTY_FULLSIZE_YUV_0, "fullsize yuv");
            setProvider(eSTREAMID_IMAGE_PIPE_THIRDPARTY_FULLSIZE_YUV_1, "fullsize yuv main2");
            setProvider(eSTREAMID_IMAGE_PIPE_DUALYUVNODE_THUMBNAIL, "postview");
        }
        else
        {
            MY_LOGA("not support, please check config");
            return NULL;
        }
    }
    //
    pBuilder->setMetaStreamId(
                eSTREAMID_META_HAL_DYNAMIC_P1,
                eSTREAMID_META_APP_CONTROL);

    // Tell StereoLegacyPipelineBuilder we need to substitute default requestBuilder
    {
        MY_LOGD("Create a default request builder for p2 pipeline");
        sp<NodeConfigDataManager> pNodeConfigDataManager_prvRequest =
                                    buildNodeConfigDataManager(content.mPrvContent);
        sp<NodeConfigDataManager> pNodeConfigDataManager_capRequest =
                                    buildNodeConfigDataManager(content.mCapContent);
        //sp<NodeConfigDataManager> pNodeConfigDataManager_dngCapRequest =
        //                            buildNodeConfigDataManager(content.mDngCapContent);

        Vector< sp<NodeConfigDataManager> > vRequests;
        vRequests.push_back(pNodeConfigDataManager_capRequest);
        //vRequests.push_back(pNodeConfigDataManager_dngCapRequest);

        pBuilder->setRequestTypes(pNodeConfigDataManager_prvRequest, vRequests);
    }

    sp<ILegacyPipeline> pPipeline = pBuilder->create(pMetaStreamManager, pImageStreamManager, pNodeConfigDataManager);
    if(pPipeline == NULL){
        MY_LOGE("pBuilder->create P2Pipeline failed");
        return NULL;
    }
    FUNC_END;
    return pPipeline;
}
/******************************************************************************
 * create p2 rec pipeline
 ******************************************************************************/
sp<ILegacyPipeline>
StereoFlowControl::
constructP2Pipeline_Rec_BB(
    StereoModeContextBuilderContent content
)
{
    FUNC_START;
    // create legacy pipeline builder
    sp<StereoLegacyPipelineBuilder> pBuilder =
            new StereoLegacyPipelineBuilder(
                                        getOpenId_P2Prv(),
                                        "P2_Pipeline",
                                        StereoLegacyPipelineMode_T::STPipelineMode_P2);
    if ( pBuilder == 0 ) {
        MY_LOGE("Cannot create LegacyPipelineBuilder.");
        return MFALSE;
    }
    sp<MetaStreamManager> pMetaStreamManager =
            MetaStreamManager::create(content.mFullContent.metaTable);
    if(pMetaStreamManager == NULL)
    {
        MY_LOGE("Create MetaStreamManager fail.");
        return NULL;
    }
    //Update P1 information to image stream table
    {
#define updateItem(streamId, rcContainer) \
        do {\
            sp<StreamBufferProvider> pProviderPass1 = rcContainer->queryConsumer(streamId); \
            if(pProviderPass1 != NULL){ \
                sp<IImageStreamInfo> streamInfo = pProviderPass1->queryImageStreamInfo(); \
                MINT32 index = getImageStreamIndex( content.mFullContent.imageTable, streamId); \
                if(index == -1) { \
                    MY_LOGW("Stream do not exist in table!! %#" PRIx64 " .", streamId); \
                } \
                else { \
                    content.mFullContent.imageTable[index].imgSize.w = streamInfo->getImgSize().w; \
                    content.mFullContent.imageTable[index].imgSize.h = streamInfo->getImgSize().h; \
                    content.mFullContent.imageTable[index].stride = (streamInfo->getBufPlanes()[0]).rowStrideInBytes; \
                    MY_LOGD("update streamId %#" PRIx64 "for P2 PV/VR's table %dx%d %d", \
                        streamId, \
                        content.mFullContent.imageTable[index].imgSize.w, \
                        content.mFullContent.imageTable[index].imgSize.h, \
                        content.mFullContent.imageTable[index].stride ); \
                } \
            } \
        } while(0);

        updateItem(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE, mpResourceContainier);
        updateItem(eSTREAMID_IMAGE_PIPE_RAW_RESIZER, mpResourceContainier);
        updateItem(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01, mpResourceContainierMain2);
        updateItem(eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01, mpResourceContainierMain2);
        // update lcso
        updateItem(eSTREAMID_IMAGE_PIPE_RAW_LCSO, mpResourceContainier);
        updateItem(eSTREAMID_IMAGE_PIPE_RAW_LCSO_01, mpResourceContainierMain2);
#undef updateItem
    }
    //Update MainImage/Preview/Record buffer size from SizeProvider
    {
#define updateItem(streamId, scenario) \
        do { \
            Pass2SizeInfo pass2SizeInfo; \
            StereoSizeProvider::getInstance()->getPass2SizeInfo(PASS2A, scenario, pass2SizeInfo); \
            MINT32 index = getImageStreamIndex( content.mFullContent.imageTable, streamId); \
            if(index == -1) { \
                MY_LOGW("Stream do not exist in table!! %#" PRIx64 " .", streamId); \
            } \
            else { \
                content.mFullContent.imageTable[index].imgSize.w = pass2SizeInfo.areaWDMA.size.w; \
                content.mFullContent.imageTable[index].imgSize.h = pass2SizeInfo.areaWDMA.size.h; \
                MY_LOGD("update streamId %#" PRIx64 "for P2 PV/VR's table %dx%d", \
                    streamId,content.mFullContent.imageTable[index].imgSize.w, content.mFullContent.imageTable[index].imgSize.h); \
            } \
        }while(0);

        //Record Scenario
        if(getImageStreamIndex( content.mFullContent.imageTable, eSTREAMID_IMAGE_PIPE_BOKEHNODE_RECORDYUV) != -1) {
            updateItem(eSTREAMID_IMAGE_PIPE_BOKEHNODE_PREVIEWYUV, StereoHAL::eSTEREO_SCENARIO_PREVIEW);
            updateItem(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_MAINIMAGEYUV, StereoHAL::eSTEREO_SCENARIO_RECORD);
            updateItem(eSTREAMID_IMAGE_PIPE_BOKEHNODE_RECORDYUV, StereoHAL::eSTEREO_SCENARIO_RECORD);
        }
        else {
            updateItem(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_MAINIMAGEYUV, StereoHAL::eSTEREO_SCENARIO_PREVIEW);
            updateItem(eSTREAMID_IMAGE_PIPE_BOKEHNODE_PREVIEWYUV, StereoHAL::eSTEREO_SCENARIO_PREVIEW);
        }

#undef updateItem
    }
    // update dmbg, capture main image size
    {
#define updateItem(streamId, enum, scenario) \
        do { \
            StereoArea imgSize; \
            imgSize = StereoSizeProvider::getInstance()->getBufferSize(enum, scenario); \
            MINT32 index = getImageStreamIndex( content.mFullContent.imageTable, streamId); \
            if(index == -1) { \
                MY_LOGW("Stream do not exist in table!! %#" PRIx64 " .", streamId); \
            } \
            else { \
                content.mFullContent.imageTable[index].imgSize.w = imgSize.size.w; \
                content.mFullContent.imageTable[index].imgSize.h = imgSize.size.h; \
                MY_LOGD("update streamId %#" PRIx64 "for P2 PV/VR's table %dx%d", \
                    streamId,content.mFullContent.imageTable[index].imgSize.w, content.mFullContent.imageTable[index].imgSize.h); \
            } \
        }while(0);
        updateItem(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_DMBGYUV, E_DMBG, StereoHAL::eSTEREO_SCENARIO_PREVIEW);
#undef updateItem
    }
    // update p2 tuning buffer size
    {
        MINT32 index = getImageStreamIndex( content.mFullContent.imageTable, eSTREAMID_IMAGE_PIPE_STEREO_P2_TUNING);
        if(index == -1)
        {
            MY_LOGW("Stream do not exist in table!! %#" PRIx64 " .", eSTREAMID_IMAGE_PIPE_STEREO_P2_TUNING);
        }
        else
        {
            content.mFullContent.imageTable[index].imgSize.w = NSIoPipe::NSPostProc::INormalStream::getRegTableSize();
            content.mFullContent.imageTable[index].imgSize.h = 1;
            MY_LOGD("update streamId %#" PRIx64 " for P2 PV/VR's table %dx%d",
                eSTREAMID_IMAGE_PIPE_STEREO_P2_TUNING,
                content.mFullContent.imageTable[index].imgSize.w,
                content.mFullContent.imageTable[index].imgSize.h);
        }
    }
    //
    sp<ImageStreamManager> pImageStreamManager =
            ImageStreamManager::create(content.mFullContent.imageTable);
    if(pImageStreamManager == NULL)
    {
        MY_LOGE("Create ImageStreamManager fail.");
        return NULL;
    }
    //
    const NodeConfigDataManager::NodeConfigMgrSetting cfgSetting =
                                        {
                                            (MUINT32)mOpenId,
                                            (MUINT32)mOpenId_main2,
                                            content.mFullContent.nodeConfigData
                                        };
    sp<NodeConfigDataManager> pNodeConfigDataManager =
            NodeConfigDataManager::create("Prv", &cfgSetting);
    if(pNodeConfigDataManager == NULL)
    {
        MY_LOGE("Create NodeConfigDataManager fail.");
        return NULL;
    }

#define SET_INPUT_PROVIDER(streamId, selector, bufferPool) \
do\
{\
    sp<IImageStreamInfo> streamInfo = pImageStreamManager->getStreamInfoByStreamId(streamId);\
    if(selector != nullptr && streamInfo != nullptr)\
    {\
        bufferPool = StereoBufferPool::createInstance(streamInfo->getStreamName(), streamInfo);\
        bufferPool->setSelector(selector);\
\
        sp<StreamBufferProviderFactory> pFactory = StreamBufferProviderFactory::createInstance();\
        pFactory->setImageStreamInfo(streamInfo);\
        pFactory->setUsersPool(bufferPool);\
\
        android::sp<StreamBufferProvider> pProvider = pFactory->create(getOpenId_P2Prv(), MTRUE);\
\
        if(pProvider == NULL)\
        {\
            MY_LOGE("create " #bufferPool " failed");\
            return NULL;\
        }\
        pImageStreamManager->updateBufProvider(\
                                        streamId,\
                                        pProvider,\
                                        Vector<StreamId_T>());\
    }\
}while(0);
    // Set provider
    // 1. set Full raw main1
    SET_INPUT_PROVIDER(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE, mpStereoSelector_OPAQUE, mpStereoBufferPool_OPAQUE);
    // 2. set resized raw main1
    SET_INPUT_PROVIDER(eSTREAMID_IMAGE_PIPE_RAW_RESIZER, mpStereoSelector_RESIZER, mpStereoBufferPool_RESIZER);
    // 3.1 set FULL raw main2
    SET_INPUT_PROVIDER(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01, mpStereoSelector_OPAQUE_MAIN2, mpStereoBufferPool_OPAQUE_MAIN2);
    // 3.2 set resized raw main2
    SET_INPUT_PROVIDER(eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01, mpStereoSelector_RESIZER_MAIN2, mpStereoBufferPool_RESIZER_MAIN2);
    // 3.3 set lcso for main1
    SET_INPUT_PROVIDER(eSTREAMID_IMAGE_PIPE_RAW_LCSO, mpStereoSelector_LCS ,mpStereoBufferPool_LCS);
    // 3.3 set lcso for main2
    SET_INPUT_PROVIDER(eSTREAMID_IMAGE_PIPE_RAW_LCSO_01, mpStereoSelector_LCS_MAIN2 ,mpStereoBufferPool_LCS_MAIN2);
#undef SET_INPUT_PROVIDER
    // Set CamClient
#define SET_CAM_CLIENT(streamId, providerTypeList)\
do{\
    sp<IImageStreamInfo> pImageInfo = pImageStreamManager->getStreamInfoByStreamId(streamId);\
    if(pImageInfo != nullptr && providerTypeList.size() > 0)\
    {\
        sp<StreamBufferProviderFactory> pFactory = StreamBufferProviderFactory::createInstance();\
\
        sp<ClientBufferPool> pClient = new ClientBufferPool(getOpenId_P2Prv());\
        {\
            for(auto item:providerTypeList)\
            {\
                if(IImgBufProvider::eID_GENERIC == item)\
                {\
                    pClient->setCamClient(item);\
                }\
                else\
                {\
                    pClient->setCamClient(\
                                    pImageInfo->getStreamName(),\
                                    mpImgBufProvidersMgr,\
                                    item);\
                }\
            }\
        }\
        pFactory->setImageStreamInfo(pImageInfo);\
        pFactory->setUsersPool(pClient);\
\
        android::sp<StreamBufferProvider> pProvider = pFactory->create(getOpenId_P2Prv(), MTRUE);\
\
        pImageStreamManager->updateBufProvider(\
                                        pImageInfo->getStreamId(),\
                                        pProvider,\
                                        Vector<StreamId_T>());\
    }\
    else\
    {\
        MY_LOGW("set " #streamId " cam client fail");\
    }\
}while(0);
    // 4-1 set display CamClient
    Vector<MINT32> lProviderType;
    lProviderType.clear();
    lProviderType.push_back(IImgBufProvider::eID_DISPLAY);
    SET_CAM_CLIENT(eSTREAMID_IMAGE_PIPE_BOKEHNODE_PREVIEWYUV, lProviderType);
    // 4-2 set preview callback
    lProviderType.clear();
    lProviderType.push_back(IImgBufProvider::eID_PRV_CB);
    lProviderType.push_back(IImgBufProvider::eID_GENERIC);
    SET_CAM_CLIENT(eSTREAMID_IMAGE_PIPE_YUV_01, lProviderType);
    // 4-3 set record CamClient
    lProviderType.clear();
    lProviderType.push_back(IImgBufProvider::eID_REC_CB);
    SET_CAM_CLIENT(eSTREAMID_IMAGE_PIPE_BOKEHNODE_RECORDYUV, lProviderType);
    // 5. set FD
    lProviderType.clear();
    lProviderType.push_back(IImgBufProvider::eID_FD);
    SET_CAM_CLIENT(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_FDYUV, lProviderType);
#undef SET_CAM_CLIENT
    //
    pBuilder->setMetaStreamId(
                eSTREAMID_META_HAL_DYNAMIC_P1,
                eSTREAMID_META_APP_CONTROL);

    sp<ILegacyPipeline> pPipeline = pBuilder->create(pMetaStreamManager, pImageStreamManager, pNodeConfigDataManager);
    if(pPipeline == NULL){
        MY_LOGE("pBuilder->create P2Pipeline failed");
        return NULL;
    }
    FUNC_END;
    return pPipeline;
}



/******************************************************************************
 * create 3rd party p2 prv pipeline
 ******************************************************************************/
sp<ILegacyPipeline>
StereoFlowControl::
constructP2Pipeline_PrvCap_3rdParty(
    StereoModeContextBuilderContent content,
    sp<ImageStreamManager>& spImageStreamManager
)
{
    FUNC_START;
    // create legacy pipeline builder
    sp<StereoLegacyPipelineBuilder> pBuilder =
            new StereoLegacyPipelineBuilder(
                                        getOpenId_P2Prv(),
                                        "P2_3rdParty_Pipeline",
                                        StereoLegacyPipelineMode_T::STPipelineMode_P2);
    if ( pBuilder == 0 ) {
        MY_LOGE("Cannot create LegacyPipelineBuilder.");
        return MFALSE;
    }
    sp<MetaStreamManager> pMetaStreamManager =
            MetaStreamManager::create(content.mFullContent.metaTable);
    if(pMetaStreamManager == NULL)
    {
        MY_LOGE("Create MetaStreamManager fail.");
        return NULL;
    }
    //Update P1 information to image stream table
    {
#define updateItem(streamId, rcContainer) \
        do {\
            sp<StreamBufferProvider> pProviderPass1 = rcContainer->queryConsumer(streamId); \
            if(pProviderPass1 != NULL){ \
                sp<IImageStreamInfo> streamInfo = pProviderPass1->queryImageStreamInfo(); \
                MINT32 index = getImageStreamIndex( content.mFullContent.imageTable, streamId); \
                if(index == -1) { \
                    MY_LOGW("Stream do not exist in table!! %#" PRIx64 " .", streamId); \
                } \
                else { \
                    content.mFullContent.imageTable[index].imgSize.w = streamInfo->getImgSize().w; \
                    content.mFullContent.imageTable[index].imgSize.h = streamInfo->getImgSize().h; \
                    content.mFullContent.imageTable[index].stride = (streamInfo->getBufPlanes()[0]).rowStrideInBytes; \
                    MY_LOGD("update streamId %#" PRIx64 "for P2 PV/VR's table %dx%d %d", \
                        streamId, \
                        content.mFullContent.imageTable[index].imgSize.w, \
                        content.mFullContent.imageTable[index].imgSize.h, \
                        content.mFullContent.imageTable[index].stride ); \
                } \
            } \
        } while(0);

        updateItem(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE, mpResourceContainier);
        updateItem(eSTREAMID_IMAGE_PIPE_RAW_RESIZER, mpResourceContainier);
        updateItem(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01, mpResourceContainierMain2);
        updateItem(eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01, mpResourceContainierMain2);
        // update lcso
        updateItem(eSTREAMID_IMAGE_PIPE_RAW_LCSO, mpResourceContainier);
        updateItem(eSTREAMID_IMAGE_PIPE_RAW_LCSO_01, mpResourceContainierMain2);
#undef updateItem
    }
    //Update MainImage/Preview/Record buffer size from SizeProvider
    {
#define updateItem(streamId, scenario) \
        do { \
            Pass2SizeInfo pass2SizeInfo; \
            StereoSizeProvider::getInstance()->getPass2SizeInfo(PASS2A, scenario, pass2SizeInfo); \
            MINT32 index = getImageStreamIndex( content.mFullContent.imageTable, streamId); \
            if(index == -1) { \
                MY_LOGW("Stream do not exist in table!! %#" PRIx64 " .", streamId); \
            } \
            else { \
                content.mFullContent.imageTable[index].imgSize.w = pass2SizeInfo.areaWDMA.size.w; \
                content.mFullContent.imageTable[index].imgSize.h = pass2SizeInfo.areaWDMA.size.h; \
                MY_LOGD("update streamId %#" PRIx64 "for P2 PV/VR's table %dx%d", \
                    streamId,content.mFullContent.imageTable[index].imgSize.w, content.mFullContent.imageTable[index].imgSize.h); \
            } \
        }while(0);
        updateItem(eSTREAMID_IMAGE_PIPE_YUV_00, StereoHAL::eSTEREO_SCENARIO_PREVIEW);

#undef updateItem
    }
    //
    sp<ImageStreamManager> pImageStreamManager =
            ImageStreamManager::create(content.mFullContent.imageTable);
    if(pImageStreamManager == NULL)
    {
        MY_LOGE("Create ImageStreamManager fail.");
        return NULL;
    }
    spImageStreamManager = pImageStreamManager;
    //
    const NodeConfigDataManager::NodeConfigMgrSetting cfgSetting =
                                        {
                                            (MUINT32)mOpenId,
                                            (MUINT32)mOpenId_main2,
                                            content.mFullContent.nodeConfigData
                                        };
    sp<NodeConfigDataManager> pNodeConfigDataManager =
            NodeConfigDataManager::create("Prv", &cfgSetting);
    if(pNodeConfigDataManager == NULL)
    {
        MY_LOGE("Create NodeConfigDataManager fail.");
        return NULL;
    }

#define SET_INPUT_PROVIDER(streamId, selector, bufferPool) \
do\
{\
    sp<IImageStreamInfo> streamInfo = pImageStreamManager->getStreamInfoByStreamId(streamId);\
    if(selector != nullptr && streamInfo != nullptr)\
    {\
        bufferPool = StereoBufferPool::createInstance(streamInfo->getStreamName(), streamInfo);\
        bufferPool->setSelector(selector);\
\
        sp<StreamBufferProviderFactory> pFactory = StreamBufferProviderFactory::createInstance();\
        pFactory->setImageStreamInfo(streamInfo);\
        pFactory->setUsersPool(bufferPool);\
\
        android::sp<StreamBufferProvider> pProvider = pFactory->create(getOpenId_P2Prv(), MTRUE);\
\
        if(pProvider == NULL)\
        {\
            MY_LOGE("create " #bufferPool " failed");\
            return NULL;\
        }\
        pImageStreamManager->updateBufProvider(\
                                        streamId,\
                                        pProvider,\
                                        Vector<StreamId_T>());\
    }\
}while(0);
    // Set provider
    // 1. set Full raw main1
    SET_INPUT_PROVIDER(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE, mpStereoSelector_OPAQUE, mpStereoBufferPool_OPAQUE);
    // 2. set resized raw main1
    SET_INPUT_PROVIDER(eSTREAMID_IMAGE_PIPE_RAW_RESIZER, mpStereoSelector_RESIZER, mpStereoBufferPool_RESIZER);
    // 3.1 set FULL raw main2
    SET_INPUT_PROVIDER(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01, mpStereoSelector_OPAQUE_MAIN2, mpStereoBufferPool_OPAQUE_MAIN2);
    // 3.2 set resized raw main2
    SET_INPUT_PROVIDER(eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01, mpStereoSelector_RESIZER_MAIN2, mpStereoBufferPool_RESIZER_MAIN2);
    // 3.3 set lcso for main1
    SET_INPUT_PROVIDER(eSTREAMID_IMAGE_PIPE_RAW_LCSO, mpStereoSelector_LCS ,mpStereoBufferPool_LCS);
    // 3.3 set lcso for main2
    SET_INPUT_PROVIDER(eSTREAMID_IMAGE_PIPE_RAW_LCSO_01, mpStereoSelector_LCS_MAIN2 ,mpStereoBufferPool_LCS_MAIN2);
#undef SET_INPUT_PROVIDER

    // Set CamClient
#define SET_CAM_CLIENT(streamId, providerTypeList)\
do{\
    sp<IImageStreamInfo> pImageInfo = pImageStreamManager->getStreamInfoByStreamId(streamId);\
    if(pImageInfo != nullptr && providerTypeList.size() > 0)\
    {\
        sp<StreamBufferProviderFactory> pFactory = StreamBufferProviderFactory::createInstance();\
\
        sp<ClientBufferPool> pClient = new ClientBufferPool(getOpenId_P2Prv());\
        {\
            for(auto item:providerTypeList)\
            {\
                if(IImgBufProvider::eID_GENERIC == item)\
                {\
                    pClient->setCamClient(item);\
                }\
                else\
                {\
                    pClient->setCamClient(\
                                    pImageInfo->getStreamName(),\
                                    mpImgBufProvidersMgr,\
                                    item);\
                }\
            }\
        }\
        pFactory->setImageStreamInfo(pImageInfo);\
        pFactory->setUsersPool(pClient);\
\
        android::sp<StreamBufferProvider> pProvider = pFactory->create(getOpenId_P2Prv(), MTRUE);\
\
        pImageStreamManager->updateBufProvider(\
                                        pImageInfo->getStreamId(),\
                                        pProvider,\
                                        Vector<StreamId_T>());\
    }\
    else\
    {\
        MY_LOGW("set " #streamId " cam client fail");\
    }\
}while(0);
    // 4-1 set display CamClient
    Vector<MINT32> lProviderType;
    lProviderType.clear();
    lProviderType.push_back(IImgBufProvider::eID_DISPLAY);
    SET_CAM_CLIENT(eSTREAMID_IMAGE_PIPE_YUV_00, lProviderType);
    // 4-2 set preview callback
    lProviderType.clear();
    lProviderType.push_back(IImgBufProvider::eID_PRV_CB);
    lProviderType.push_back(IImgBufProvider::eID_GENERIC);
    SET_CAM_CLIENT(eSTREAMID_IMAGE_PIPE_YUV_01, lProviderType);
    // 4-3 set record CamClient
    //lProviderType.clear();
    //lProviderType.push_back(IImgBufProvider::eID_REC_CB);
    //SET_CAM_CLIENT(eSTREAMID_IMAGE_PIPE_BOKEHNODE_RECORDYUV, lProviderType);
    // 5. set FD
    lProviderType.clear();
    lProviderType.push_back(IImgBufProvider::eID_FD);
    SET_CAM_CLIENT(eSTREAMID_IMAGE_YUV_FD, lProviderType);
#undef SET_CAM_CLIENT
    // 6. set extradebug provider
    {
        auto setProvider = [this, &pImageStreamManager](MINT32 streamId, char const* name)
        {
            sp<IImageStreamInfo> pImageInfo = pImageStreamManager->getStreamInfoByStreamId(streamId);
            if(pImageInfo != NULL)
            {
                MY_LOGD("Prepare BufferProvider for %s", name);
                sp<StreamBufferProviderFactory> pFactory = StreamBufferProviderFactory::createInstance();
                sp<CallbackBufferPool> pPool = new CallbackBufferPool(pImageInfo);
                pPool->allocateBuffer(
                        pImageInfo->getStreamName(),
                        pImageInfo->getMaxBufNum(),
                        pImageInfo->getMinInitBufNum());
                pFactory->setImageStreamInfo(pImageInfo);
                pFactory->setUsersPool(pPool);

                android::sp<StreamBufferProvider> pProvider = pFactory->create(getOpenId_P2Prv(), MTRUE);

                pImageStreamManager->updateBufProvider(
                                                pImageInfo->getStreamId(),
                                                pProvider,
                                                Vector<StreamId_T>());
            }
            else {
                MY_LOGD("%s did not config in ImageStreamMnger!!", name);
            }
        };
        if(!mbCanSupportBackgroundService)
        {
            setProvider(eSTREAMID_IMAGE_PIPE_RAW16, "raw16");
            setProvider(eSTREAMID_IMAGE_PIPE_YUV_JPEG, "yuv");
            setProvider(eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL, "thumbnail yuv");
        }

        // 3rd party
        setProvider(eSTREAMID_IMAGE_PIPE_THIRDPARTY_RESIZE_YUV_0, "resize yuv");
        setProvider(eSTREAMID_IMAGE_PIPE_THIRDPARTY_RESIZE_YUV_1, "resize yuv main2");
        setProvider(eSTREAMID_IMAGE_PIPE_THIRDPARTY_FULLSIZE_YUV_0, "fullsize yuv");
        setProvider(eSTREAMID_IMAGE_PIPE_THIRDPARTY_FULLSIZE_YUV_1, "fullsize yuv main2");
        setProvider(eSTREAMID_IMAGE_PIPE_DUALYUVNODE_THUMBNAIL, "postview");
    }
    //
    pBuilder->setMetaStreamId(
                eSTREAMID_META_HAL_DYNAMIC_P1,
                eSTREAMID_META_APP_CONTROL);

    // Tell StereoLegacyPipelineBuilder we need to substitute default requestBuilder
    {
        MY_LOGD("Create a default request builder for p2 pipeline");
        sp<NodeConfigDataManager> pNodeConfigDataManager_prvRequest =
                                    buildNodeConfigDataManager(content.mPrvContent);
        sp<NodeConfigDataManager> pNodeConfigDataManager_capRequest =
                                    buildNodeConfigDataManager(content.mCapContent);
        sp<NodeConfigDataManager> pNodeConfigDataManager_dngCapRequest =
                                    buildNodeConfigDataManager(content.mDngCapContent);

        Vector< sp<NodeConfigDataManager> > vRequests;
        vRequests.push_back(pNodeConfigDataManager_capRequest);
        vRequests.push_back(pNodeConfigDataManager_dngCapRequest);

        pBuilder->setRequestTypes(pNodeConfigDataManager_prvRequest, vRequests);
    }


    sp<ILegacyPipeline> pPipeline = pBuilder->create(pMetaStreamManager, pImageStreamManager, pNodeConfigDataManager);
    if(pPipeline == NULL){
        MY_LOGE("pBuilder->create P2Pipeline failed");
        return NULL;
    }
    FUNC_END;
    return pPipeline;
}
