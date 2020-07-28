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
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#define LOG_TAG "BaseCaptureRequestBuilder"
//
#include <mtkcam/middleware/v1/IShot.h>
#include <mtkcam/middleware/v1/camshot/BufferCallbackHandler.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/buffer/StereoSelector.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/buffer/StereoBufferPool.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam/middleware/v1/LegacyPipeline/IResourceContainer.h>
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include <mtkcam/middleware/v1/LegacyPipeline/NodeId.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam/middleware/v1/IParamsManager.h> // to get bokeh level
#include <mtkcam/middleware/v1/IParamsManagerV3.h>
//
#include <mtkcam/utils/std/Log.h>
// metadata tag
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h> // hal
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>// app
//
#include "../../../../../../common/CbImgBufMgr/CallbackImageBufferManager.h"
#include "../image/IImageShotCallback.h"
#include "../metadata/IMetaShotCallback.h"
#include "BaseCaptureRequestBuilder.h"
//
#include "../ImageShotCallbackFactory.h"
#include "../MetadataShotCallbackFactory.h"
//
#include "../processor/ShotCallbackProcessor.h"
using namespace android;
using namespace NSCam;
using namespace NSCam::v1::NSLegacyPipeline;
using namespace NSCam::v3::Utils;
/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)
/******************************************************************************
 *
 ******************************************************************************/
template <typename T>
inline MBOOL
tryGetMetadata(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T & rVal
)
{
    if( pMetadata == NULL ) {
        MY_LOGW("pMetadata == NULL");
        return MFALSE;
    }

    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if( !entry.isEmpty() ) {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }
    return MFALSE;
}
#define STEREO_FLOW_CAPTURE_REQUSET_NUM_START  (3000)
#define STEREO_FLOW_CAPTURE_REQUSET_NUM_END    (4000)
/******************************************************************************
 *
 ******************************************************************************/
BaseCaptureRequestBuilder::
BaseCaptureRequestBuilder(
) : ICaptureRequestBuilder()
{
    MY_LOGD("ctor(0x%p)", this);
}
/******************************************************************************
 *
 ******************************************************************************/
BaseCaptureRequestBuilder::
~BaseCaptureRequestBuilder()
{
    MY_LOGD("dtor(0x%p)", this);
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
BaseCaptureRequestBuilder::
onInit(
    CaptureRequestInitSetting &setting      __attribute__((unused))
)
{
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
BaseCaptureRequestBuilder::
onPrepareImageBufferProvider(
)
{
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
BaseCaptureRequestBuilder::
onDestory(
)
{
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
BaseCaptureRequestBuilder::
onUpdateMetadata(
    IMetadata* main1AppMeta     __attribute__((unused)),
    IMetadata* main1HalMeta     __attribute__((unused)),
    IMetadata* main2AppMeta     __attribute__((unused)),
    IMetadata* main2HalMeta     __attribute__((unused))
)
{
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
BaseCaptureRequestBuilder::
onSetImageCallback(
)
{
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
BaseCaptureRequestBuilder::
onSetMetaCallback(
)
{
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
BaseCaptureRequestBuilder::
init(
    CaptureRequestInitSetting &setting
)
{
    status_t ret = UNKNOWN_ERROR;
    miCaptureNo = setting.iCaptureNo;
    mShotParams = setting.shotParam;
    mpParamMgrV3 = setting.pParamMgr;
    mpILegacyPipeline = setting.pILegacyPipeline;
    mAppMetadata = setting.appMetadata;
    mHalMetadata = setting.halMetadata;
    mpUserData = setting.userData;
    mContent = setting.content;
    // get main1 & main2 sensor index.
    if(!StereoSettingProvider::getStereoSensorIndex(miMain1Id, miMain2Id))
    {
        MY_LOGE("Get sensor id fail");
        return ret;
    }
    if(setting.mCB == nullptr)
    {
        MY_LOGE("cb not set");
        return ret;
    }
    if(setting.mShotCallback == nullptr)
    {
        MY_LOGE("shot callback not set");
        return ret;
    }
    // prepare to allocate buffer
    miTargetPipelineId = setting.iTargetPipelineId;
    mpCallbackImageBufferManager = new CallbackImageBufferManager(miTargetPipelineId, setting.pImageStreamManager);
    mpShotCallbackProcessor = new ShotCallbackProcessor("Shot", setting);
    mpCallbackBufferHandler = new BufferCallbackHandler(0);
    mpCallbackBufferHandler->setImageCallback(mpShotCallbackProcessor);
    ret = prepareImageProvider();
    return onInit(setting);
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
BaseCaptureRequestBuilder::
doCapture(
    CaptureSetting &setting
)
{
    status_t ret = UNKNOWN_ERROR;
    // check init value first.
    // if init value not set, return error.
    std::vector< std::shared_ptr<BufferData> > vBufferDataList;
    if(mpCallbackBufferHandler == nullptr)
    {
        MY_LOGW("BufferCallbackHandler is uninit.");
        goto lbExit;
    }
    // step 1: build node config data manager
    {
        if(buildNodeConfigManager(mContent) != OK)
        {
            MY_LOGE("build node config manager fail");
            goto lbExit;
        }
    }
    // step 2: prepare p1 buffer
    {
        auto setP1Buffer = [&vBufferDataList, this](
                            MINT32 openId,
                            StreamId_T imgBufferId,
                            StreamId_T appMetadata,
                            StreamId_T halMetadata)
        {
            auto streamIdToString = [imgBufferId]()
            {
                if(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE == imgBufferId)
                {
                    return "eSTREAMID_IMAGE_PIPE_RAW_OPAQUE";
                }
                else if(eSTREAMID_IMAGE_PIPE_RAW_RESIZER == imgBufferId)
                {
                    return "eSTREAMID_IMAGE_PIPE_RAW_RESIZER";
                }
                else if(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01 == imgBufferId)
                {
                    return "eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01";
                }
                else if(eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01 == imgBufferId)
                {
                    return "eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01";
                }
                return "";
            };
            std::shared_ptr<BufferData> pBufferData = nullptr;
            auto selector = getSelector(openId, imgBufferId);
            if(selector == nullptr)
            {
                MY_LOGW("get selector fail (%s)", streamIdToString());
                return false;
            }
            pBufferData = getBufferDataFromSelect(selector, appMetadata, halMetadata);
            if(pBufferData != nullptr)
            {
                if(openId == miMain1Id && mMain1AppMetadata == nullptr)
                {
                    mMain1AppMetadata = &pBufferData->mAppMetadata;
                    MY_LOGD("- mMain1AppMetadata(%p)", mMain1AppMetadata);
                }
                if(openId == miMain1Id && mMain1HalMetadata == nullptr)
                {
                    mMain1HalMetadata = &pBufferData->mHalMetadata;
                    MY_LOGD("- mMain1HalMetadata(%p)", mMain1HalMetadata);
                }
                if(openId == miMain2Id && mMain2AppMetadata == nullptr)
                {
                    mMain2AppMetadata = &pBufferData->mAppMetadata;
                    MY_LOGD("- mMain2AppMetadata(%p)", mMain2AppMetadata);
                }
                if(openId == miMain2Id && mMain2HalMetadata == nullptr)
                {
                    mMain2HalMetadata = &pBufferData->mHalMetadata;
                    MY_LOGD("- mMain2HalMetadata(%p)", mMain2HalMetadata);
                }
                pBufferData->openId = openId;
                pBufferData->imgStreamId = imgBufferId;
                vBufferDataList.push_back(pBufferData);
                return true;
            }
            else
            {
                return false;
            }
        };
        // set main1 full raw and resize raw, lcso
        if(!setP1Buffer(
                        miMain1Id,
                        eSTREAMID_IMAGE_PIPE_RAW_OPAQUE,
                        eSTREAMID_META_APP_DYNAMIC_P1,
                        eSTREAMID_META_HAL_DYNAMIC_P1))
        {
            MY_LOGW("set eSTREAMID_IMAGE_PIPE_RAW_OPAQUE buffer fail.");
        }
        if(!setP1Buffer(
                        miMain1Id,
                        eSTREAMID_IMAGE_PIPE_RAW_RESIZER,
                        eSTREAMID_META_APP_DYNAMIC_P1,
                        eSTREAMID_META_HAL_DYNAMIC_P1))
        {
            MY_LOGW("set eSTREAMID_IMAGE_PIPE_RAW_RESIZER buffer fail.");
        }
        if(!setP1Buffer(
                        miMain1Id,
                        eSTREAMID_IMAGE_PIPE_RAW_LCSO,
                        eSTREAMID_META_APP_DYNAMIC_P1,
                        eSTREAMID_META_HAL_DYNAMIC_P1))
        {
            MY_LOGW("set eSTREAMID_IMAGE_PIPE_RAW_LCSO buffer fail.");
        }
        //
        if(!setP1Buffer(
                        miMain2Id,
                        eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01,
                        eSTREAMID_META_APP_DYNAMIC_P1_MAIN2,
                        eSTREAMID_META_HAL_DYNAMIC_P1_MAIN2))
        {
            MY_LOGW("set eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01 buffer fail.");
        }
        if(!setP1Buffer(
                        miMain2Id,
                        eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01,
                        eSTREAMID_META_APP_DYNAMIC_P1_MAIN2,
                        eSTREAMID_META_HAL_DYNAMIC_P1_MAIN2))
        {
            MY_LOGW("set eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01 buffer fail.");
        }
        if(!setP1Buffer(
                        miMain2Id,
                        eSTREAMID_IMAGE_PIPE_RAW_LCSO_01,
                        eSTREAMID_META_APP_DYNAMIC_P1_MAIN2,
                        eSTREAMID_META_HAL_DYNAMIC_P1_MAIN2))
        {
            MY_LOGW("set eSTREAMID_IMAGE_PIPE_RAW_LCSO_01 buffer fail.");
        }
    }
    // step 4: prepare metadata
    // pack main1 and main2 metadata and add more metadata.
    {
        MY_LOGD("mMain1AppMetadata(%p)", mMain1AppMetadata);
        MY_LOGD("mMain1HalMetadata(%p)", mMain1HalMetadata);
        MY_LOGD("mMain2AppMetadata(%p)", mMain2AppMetadata);
        MY_LOGD("mMain2HalMetadata(%p)", mMain2HalMetadata);
        // get crop size
        {
            if( ! tryGetMetadata<MRect>(mMain1AppMetadata, MTK_SCALER_CROP_REGION, mCropSize_Main1) )
            {
                MY_LOGE("cannot get MTK_SCALER_CROP_REGION after updating request");
            }
        }
        // set hal metadate to main1 hal
        mHalMetadata = *mMain1HalMetadata;
        if(prepareMetadata(&mAppMetadata, &mHalMetadata, mMain2AppMetadata, mMain2HalMetadata)!=OK)
        {
            MY_LOGE("prepare metadata fail");
            goto lbExit;
        }
        // for stereo, it has to update main1 crop info for main1 app
        // otherwise, it will use original crop size that set in paramMgr.
        // And, stereo flow does not set new crop size to paramMgr.
        // update main1 crop size
        /*{
            IMetadata::IEntry entry(MTK_SCALER_CROP_REGION);
            entry.push_back(mCropSize_Main1, Type2Type<MRect>());
            mAppMetadata.update(MTK_SCALER_CROP_REGION, entry);
        }*/
    }
    // step 5: get timestamp and send it to provider
    {
        if(getTimestamp(mMain1AppMetadata, miTimeStamp)!=OK)
        {
            MY_LOGE("get timestamp fail");
            goto lbExit;
        }
        if(sendTimestampToProvider(miCaptureNo, miTimeStamp) != OK)
        {
            MY_LOGE("send timestamp to provider fail");
            goto lbExit;
        }
    }
    // step 6: set image callback and metadata callback
    {
        setImageCallback();
        setMetadataCallback();
    }
    // step x: push image buffer to pipeline entrance.
    {
        auto setBufferToPool = [&vBufferDataList](
                        std::shared_ptr<BufferPoolInfo> poolInfo)
        {
            for(auto itr : vBufferDataList)
            {
                if(poolInfo->openId == itr->openId &&
                   poolInfo->poolStreamId == itr->imgStreamId)
                {
                    MY_LOGD("set buffer to pool (%s)", poolInfo->name.string());
                    poolInfo->mpPool->setBuffer(poolInfo->name.string(), itr->mBufferHeap);
                }
            }
        };
        for(auto itr:setting.mvBufferPool)
        {
            setBufferToPool(itr);
        }
    }
    // send to legacy pipeline
    {
        sp<ILegacyPipeline> pPipeline = mpILegacyPipeline.promote();
        // do timestamp to p1
        auto sendTimestampToP2Input = [&pPipeline, this](
                                        StreamId_T streamId)
        {
            sp<StreamBufferProvider> pProvider = pPipeline->queryProvider(streamId);
            if(pProvider != nullptr)
            {
                pProvider->doTimestampCallback(miCaptureNo, MFALSE, miTimeStamp);
            }
            else
            {
                MY_LOGW("get provider fail %#" PRIx64 ".", streamId);
            }
        };
        sendTimestampToP2Input(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE);
        sendTimestampToP2Input(eSTREAMID_IMAGE_PIPE_RAW_RESIZER);
        sendTimestampToP2Input(eSTREAMID_IMAGE_PIPE_RAW_LCSO);
        sendTimestampToP2Input(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01);
        sendTimestampToP2Input(eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01);
        sendTimestampToP2Input(eSTREAMID_IMAGE_PIPE_RAW_LCSO_01);
        //register callback to result processor
        MY_LOGD("set result processor");
        {
            sp<ResultProcessor> pResultProcessor = pPipeline->getResultProcessor().promote();
            if(pResultProcessor == nullptr)
            {
                MY_LOGE("can not get result processor");
                goto lbExit;
            }
            //partial
            pResultProcessor->registerListener(
                miCaptureNo,
                miCaptureNo,
                true,
                mpShotCallbackProcessor
            );
            //full
            pResultProcessor->registerListener(
                miCaptureNo,
                miCaptureNo,
                false,
                mpShotCallbackProcessor
            );
        }
        if(pPipeline!= nullptr)
        {
            if(mvDstStreams.size() == 0)
            {
                MY_LOGW("vDstStreams is empty");
            }

            // for PostProcessing
            mpShotCallbackProcessor->addPostProcInfo(eSTREAMID_META_APP_CONTROL, mAppMetadata, miTimeStamp, mShotParams);

            pPipeline->submitRequest(miCaptureNo, mAppMetadata, mHalMetadata, mvDstStreams);
        }
    }
    ret = OK;
lbExit:
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
BaseCaptureRequestBuilder::
destroy(
)
{
    status_t ret = UNKNOWN_ERROR;
    mpShotCallbackProcessor->destroy();
    if(mpCallbackImageBufferManager!=nullptr)
    {
        ret = mpCallbackImageBufferManager->destory();
    }
    mpCallbackImageBufferManager = nullptr;
    mpCallbackBufferHandler = nullptr;
    mvMetadataCallbackList.clear();
    mvImageCallbackList.clear();
    mpNodeConfigDataManager = nullptr;
    ret = OK;
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
BaseCaptureRequestBuilder::
buildNodeConfigManager(
    ContextBuilderContent content
)
{
    status_t ret = UNKNOWN_ERROR;
    sp<MetaStreamManager> pMetaStreamMgr = nullptr;
    sp<ImageStreamManager> pImageStreamMgr = nullptr;
    sp<NodeConfigDataManager> pNodeConfigDataManager = nullptr;
    sp<NSCam::StereoBasicParameters> userParams = nullptr;
    sp<PipelineContext> pipelineContext = nullptr;
    const NodeConfigDataManager::NodeConfigMgrSetting cfgSetting =
                                        {
                                            (MUINT32)miMain1Id,
                                            (MUINT32)miMain2Id,
                                            content.nodeConfigData
                                        };
    pMetaStreamMgr = MetaStreamManager::create(content.metaTable);
    if(pMetaStreamMgr == nullptr)
    {
        MY_LOGE("pMetaStreamMgr is null");
        goto lbExit;
    }
    pImageStreamMgr = ImageStreamManager::create(content.imageTable);
    if(pImageStreamMgr == nullptr)
    {
        MY_LOGE("pImageStreamMgr is null");
        goto lbExit;
    }
    //
    mpNodeConfigDataManager =
            NodeConfigDataManager::create("BBCapture", &cfgSetting);
    if(mpNodeConfigDataManager == nullptr)
    {
        MY_LOGE("mpNodeConfigDataManager is null");
        goto lbExit;
    }
    // send image table and image table to node config data manager to build
    // IOMap for this capture request.
    mpNodeConfigDataManager->configNode(
                                pMetaStreamMgr,
                                pImageStreamMgr,
                                userParams,
                                pipelineContext);
    ret = OK;
    return ret;
lbExit:
    pMetaStreamMgr = nullptr;
    pImageStreamMgr = nullptr;
    mpNodeConfigDataManager = nullptr;
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
sp<ISelector>
BaseCaptureRequestBuilder::
getSelector(
    MUINT32 openId,
    StreamId_T streamId
)
{
    sp<StreamBufferProvider> pBufferProvider = nullptr;
    sp<IResourceContainer> pResourceContainer =
                        IResourceContainer::getInstance(openId);
    if(pResourceContainer == nullptr)
    {
        MY_LOGW("get resource container fail");
        goto lbExit;
    }
    pBufferProvider = pResourceContainer->queryConsumer(streamId);
    if(pBufferProvider == nullptr)
    {
        MY_LOGW("pBufferProvider is null");
        goto lbExit;
    }
    return pBufferProvider->querySelector();
lbExit:
    pResourceContainer = nullptr;
    pBufferProvider = nullptr;
    return nullptr;
}
/******************************************************************************
 *
 ******************************************************************************/
std::shared_ptr<ICaptureRequestBuilder::BufferData>
BaseCaptureRequestBuilder::
getBufferDataFromSelect(
    sp<ISelector> selector,
    MINT32 const iAppMetadataStreamId,
    MINT32 const iHalMetadataStreamId
)
{
    if(selector == nullptr)
    {
        MY_LOGE("selector is nullptr");
        return nullptr;
    }
    std::shared_ptr<ICaptureRequestBuilder::BufferData> bufferData =
                        std::shared_ptr<ICaptureRequestBuilder::BufferData>(
                            new BufferData(),
                            [&](auto *p) -> void
                            {
                                MY_LOGD("release buffer data");
                                if(p!=nullptr)
                                    delete p;
                                p = nullptr;
                            }
                        );
    Vector<ISelector::MetaItemSet> tempMetadata;
    MINT32 tempReqNo = -1;
    MBOOL hasAppDynamicData = MFALSE;
    MBOOL hasHalDynamicData = MFALSE;
    //
    if(((StereoSelector*)selector.get())->getResultCapture(tempReqNo, tempMetadata, bufferData->mBufferHeap) != OK){
        MY_LOGE("mSelector->getResultCapture failed!");
        goto lbExit;
    }
    //
    for ( auto& metadata : tempMetadata )
    {
        if(metadata.id == iAppMetadataStreamId)
        {
            bufferData->mAppMetadata = metadata.meta;
            hasAppDynamicData = MTRUE;
        }
        else if(metadata.id == iHalMetadataStreamId)
        {
            bufferData->mHalMetadata = metadata.meta;
            hasHalDynamicData = MTRUE;
        }
        else
        {
            MY_LOGE("unexpected meta stream from %s:%d", ((StereoSelector*)selector.get())->getUserName(), metadata.id);
        }
    }
    if(!hasAppDynamicData || !hasHalDynamicData)
    {
        MY_LOGE("some meta stream from %s is not correctly set!", ((StereoSelector*)selector.get())->getUserName());
        goto lbExit;
    }
    return bufferData;
lbExit:
    return nullptr;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
BaseCaptureRequestBuilder::
prepareMetadata(
    IMetadata* appMeta,
    IMetadata* halMeta,
    IMetadata* main2AppMeta,
    IMetadata* main2HalMeta
)
{
    return onUpdateMetadata(appMeta, halMeta, main2AppMeta, main2HalMeta);
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
BaseCaptureRequestBuilder::
prepareImageProvider(
)
{
    return onPrepareImageBufferProvider();
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
BaseCaptureRequestBuilder::
getTimestamp(
    IMetadata *metadata,
    MINT64 &timestamp
)
{
    status_t ret = UNKNOWN_ERROR;
    if(metadata == nullptr)
    {
        MY_LOGW("metadata is nullptr, set to 0");
        timestamp = 0;
        goto lbExit;
    }
    if(tryGetMetadata<MINT64>(metadata, MTK_SENSOR_TIMESTAMP, timestamp))
    {
        MY_LOGD("timestamp_P1:%lld", timestamp);
    }
    else
    {
        MY_LOGW("Can't get timestamp from metadata, set to 0");
        timestamp = 0;
    }
    ret = OK;
lbExit:
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
BaseCaptureRequestBuilder::
sendTimestampToProvider(
    MUINT32 reqNo,
    MINT64 timestamp
)
{
    status_t ret = UNKNOWN_ERROR;
    if(mpCallbackImageBufferManager != nullptr)
    {
        ret = mpCallbackImageBufferManager->setTimestamp(reqNo, timestamp);
    }
    else
    {
        MY_LOGE("mpCallbackImageBufferManager is null");
    }
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
BaseCaptureRequestBuilder::
setImageCallback(
)
{
    status_t ret = UNKNOWN_ERROR;
    if(mpShotCallbackProcessor == nullptr)
    {
        MY_LOGE("mpShotCallbackProcessor is null");
        return ret;
    }
    ret = onSetImageCallback();
    if(ret!=OK)
    {
        MY_LOGD("setImageCallback fail, reset shot callback processor");
        mpShotCallbackProcessor->resetProcessor();
    }
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
BaseCaptureRequestBuilder::
setMetadataCallback(
)
{
    status_t ret = UNKNOWN_ERROR;
    if(mpShotCallbackProcessor == nullptr)
    {
        MY_LOGE("mpShotCallbackProcessor is null");
        return ret;
    }
    ret = onSetMetaCallback();
    if(ret!=OK)
    {
        MY_LOGD("onSetMetaCallback fail, reset shot callback processor");
        mpShotCallbackProcessor->resetProcessor();
    }
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
BaseCaptureRequestBuilder::
onLastStrongRef(
    const void*
)
{
    //
    if(mpCallbackBufferHandler != nullptr)
    {
        mpCallbackBufferHandler->setImageCallback(nullptr);
        mpCallbackBufferHandler.clear();
    }
    // remove result processor
    {
        sp<ILegacyPipeline> pPipeline = mpILegacyPipeline.promote();
        //register callback to result processor
        MY_LOGD("remove result processor");
        {
            sp<ResultProcessor> pResultProcessor = pPipeline->getResultProcessor().promote();
            if(pResultProcessor == nullptr)
            {
                MY_LOGE("can not get result processor");
                goto lbExit;
            }
            //partial
            pResultProcessor->removeListener(
                miCaptureNo,
                miCaptureNo,
                true,
                mpShotCallbackProcessor
            );
            //full
            pResultProcessor->removeListener(
                miCaptureNo,
                miCaptureNo,
                false,
                mpShotCallbackProcessor
            );
        }
    }
lbExit:
    return;
}