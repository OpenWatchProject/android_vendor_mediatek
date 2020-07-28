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

#define LOG_TAG "BayerAndBayerCaptureRequestBuilder"
//
#include <mtkcam/middleware/v1/IShot.h>
#include <mtkcam/middleware/v1/camshot/BufferCallbackHandler.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/buffer/StereoSelector.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/buffer/StereoBufferPool.h>
#include <mtkcam/middleware/v1/LegacyPipeline/IResourceContainer.h>
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include <mtkcam/middleware/v1/LegacyPipeline/NodeId.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam/middleware/v1/IParamsManager.h> // to get bokeh level
#include <mtkcam/middleware/v1/IParamsManagerV3.h>
// metadata tag
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h> // hal
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>// app
//
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
//
#include <mtkcam/drv/iopipe/SImager/ISImagerDataTypes.h>
//
#include "../buffer/CallbackImageBufferManager.h"
#include "../image/IImageShotCallback.h"
#include "../metadata/IMetaShotCallback.h"
#include "BayerAndBayerCaptureRequestBuilder.h"
//
#include "../ImageShotCallbackFactory.h"
#include "../MetadataShotCallbackFactory.h"
//
#include "../processor/ShotCallbackProcessor.h"
// B+B capture table
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/StereoPipelineData.Capture.h>
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
/******************************************************************************
 *
 ******************************************************************************/
BayerAndBayerCaptureRequestBuilder::
BayerAndBayerCaptureRequestBuilder(
) : ICaptureRequestBuilder()
{
    MY_LOGD("ctor(0x%x)", this);
}
/******************************************************************************
 *
 ******************************************************************************/
BayerAndBayerCaptureRequestBuilder::
~BayerAndBayerCaptureRequestBuilder()
{
    MY_LOGD("dtor(0x%x)", this);
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
BayerAndBayerCaptureRequestBuilder::
init(
    CaptureRequestInitSetting setting
)
{
    status_t ret = UNKNOWN_ERROR;
    if(setting.pBufferHandler == nullptr)
    {
        MY_LOGE("BufferCallbackHandler is null");
        return ret;
    }
    mpCallbackBufferHandler = setting.pBufferHandler;
    miCaptureNo = setting.iCaptureNo;
    mpParamMgrV3 = setting.pParamMgr;
    // shotParam needs to set before prepareImageProvider
    mShotParams = setting.shotParam;
    // prepare to allocate buffer
    ret = prepareImageProvider();
    // get main1 & main2 sensor index.
    if(!StereoSettingProvider::getStereoSensorIndex(miMain1Id, miMain2Id))
    {
        MY_LOGE("Get sensor id fail");
        return ret;
    }
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
BayerAndBayerCaptureRequestBuilder::
doCapture(
    CapturePipelineFrameSetting setting
)
{
    status_t ret = UNKNOWN_ERROR;
    // check init value first.
    // if init value not set, return error.
    if(mpCallbackBufferHandler == nullptr || mpParamMgrV3 == nullptr)
    {
        MY_LOGE("Init fail!");
        return ret;
    }
    if(setting.vStereoBufferPoolList.size() != 3)
    {
        MY_LOGE("B+B buffer pool is not equal to 3");
        return ret;
    }
    sp<ISelector> pSelector_Main1_Opaque = nullptr;
    sp<ISelector> pSelector_Main1_Resize = nullptr;
    sp<ISelector> pSelector_Main2_Resize = nullptr;
    BufferData* pBufferData_Main1_Opaque = nullptr;
    BufferData* pBufferData_Main1_Resize = nullptr;
    BufferData* pBufferData_Main2_Resize = nullptr;
    Vector<BufferData*> vBufferDataList;
    RequestBuilder aRequestBuilder;
    // step 1: build node config data manager
    {
        ContextBuilderContent contextBuilderContent;
        contextBuilderContent.metaTable = 
                                          StereoPipelineMgrData::
                                          p2::
                                          cap::
                                          gStereoMetaTbl_P2Cap;
        contextBuilderContent.imageTable = 
                                          StereoPipelineMgrData::
                                          p2::
                                          cap::
                                          gStereoImgStreamTbl_P2Cap;
        contextBuilderContent.nodeConfigData = 
                                          StereoPipelineMgrData::
                                          p2::
                                          cap::
                                          gStereoP2ConnectSetting_P2Cap;
        if(buildNodeConfigManager(contextBuilderContent) != OK)
        {
            MY_LOGE("build node config manager fail");
            goto lbExit;
        }
    }
    // step 2: get selector
    {
        pSelector_Main1_Opaque = getSelector(miMain1Id, eSTREAMID_IMAGE_PIPE_RAW_OPAQUE);
        if(pSelector_Main1_Opaque == nullptr)
        {
            MY_LOGE("get selector fail (eSTREAMID_IMAGE_PIPE_RAW_OPAQUE)");
            goto lbExit;
        }
        pSelector_Main1_Resize = getSelector(miMain1Id, eSTREAMID_IMAGE_PIPE_RAW_RESIZER);
        if(pSelector_Main1_Resize == nullptr)
        {
            MY_LOGE("get selector fail (eSTREAMID_IMAGE_PIPE_RAW_RESIZER)");
            goto lbExit;
        }
        pSelector_Main2_Resize = getSelector(miMain1Id, eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01);
        if(pSelector_Main2_Resize == nullptr)
        {
            MY_LOGE("get selector fail (eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01)");
            goto lbExit;
        }
    }
    // step 3: get buffer data from selector
    {
        pBufferData_Main1_Opaque = getBufferDataFromSelect(
                                                    pSelector_Main1_Opaque,
                                                    eSTREAMID_META_APP_DYNAMIC_P1,
                                                    eSTREAMID_META_HAL_DYNAMIC_P1);
        pBufferData_Main1_Resize = getBufferDataFromSelect(
                                                    pSelector_Main1_Resize,
                                                    eSTREAMID_META_APP_DYNAMIC_P1,
                                                    eSTREAMID_META_HAL_DYNAMIC_P1);
        pBufferData_Main2_Resize = getBufferDataFromSelect(
                                                    pSelector_Main2_Resize,
                                                    eSTREAMID_META_APP_DYNAMIC_P1_MAIN2,
                                                    eSTREAMID_META_HAL_DYNAMIC_P1_MAIN2);
    }
    // step 4: prepare metadata
    // pack main1 and main2 metadata and add more metadata.
    {
        vBufferDataList.add(pBufferData_Main1_Opaque);
        vBufferDataList.add(pBufferData_Main2_Resize);
        if(prepareMetadata(vBufferDataList)!=OK)
        {
            MY_LOGE("prepare metadata fail");
            goto lbExit;
        }
    }
    // step 5: get timestamp and send it to provider
    {
        if(getTimestamp(vBufferDataList[0]->mAppMetadata)!=OK)
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
        setting.vStereoBufferPoolList[0]->setBuffer("spHeap_full", pBufferData_Main1_Opaque->mBufferHeap);
        setting.vStereoBufferPoolList[1]->setBuffer("spHeap_resized", pBufferData_Main1_Resize->mBufferHeap);
        setting.vStereoBufferPoolList[2]->setBuffer("spHeap_resized_main2", pBufferData_Main2_Resize->mBufferHeap);
    }
    // send to legacy pipeline
    {
        if(setting.pILegacyPipeline!= nullptr)
        {
            //ret = ((StereoLegacyPipeline*)setting.pILegacyPipeline)->submitRequest(miCaptureNo,
            //                                                                 vBufferDataList[0].mHalMetadata,
            //                                                                 vBufferDataList[1].mAppMetadata,
            //                                                                 mpNodeConfigDataManager,
            //                                                                 mpCallbackImageBufferManager.getBufferProviderPool());
        }
    }
lbExit:
    vBufferDataList.clear();
    if(pBufferData_Main1_Opaque!=nullptr)
    {
        if(ret != OK && pBufferData_Main1_Opaque->mBufferHeap != nullptr)
        {
            pSelector_Main1_Opaque->returnBuffer(pBufferData_Main1_Opaque->mBufferHeap);
        }
        pBufferData_Main1_Opaque->mBufferHeap = nullptr;
        delete pBufferData_Main1_Opaque;
        pBufferData_Main1_Opaque = nullptr;
    }
    if(pBufferData_Main1_Resize!=nullptr)
    {
        if(ret != OK && pBufferData_Main1_Resize->mBufferHeap != nullptr)
        {
            pSelector_Main1_Resize->returnBuffer(pBufferData_Main1_Resize->mBufferHeap);
        }
        pBufferData_Main1_Resize->mBufferHeap = nullptr;
        delete pBufferData_Main1_Resize;
        pBufferData_Main1_Resize = nullptr;
    }
    if(pBufferData_Main2_Resize!=nullptr)
    {
        if(ret != OK && pBufferData_Main2_Resize->mBufferHeap != nullptr)
        {
            pSelector_Main2_Resize->returnBuffer(pBufferData_Main2_Resize->mBufferHeap);
        }
        pBufferData_Main2_Resize->mBufferHeap = nullptr;
        delete pBufferData_Main2_Resize;
        pBufferData_Main2_Resize = nullptr;
    }
    pSelector_Main1_Opaque = nullptr;
    pSelector_Main1_Resize = nullptr;
    pSelector_Main2_Resize = nullptr;
    return UNKNOWN_ERROR;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
BayerAndBayerCaptureRequestBuilder::
destroy(
)
{
    status_t ret = UNKNOWN_ERROR;
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
BayerAndBayerCaptureRequestBuilder::
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
                                            miMain1Id,
                                            miMain2Id,
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
BayerAndBayerCaptureRequestBuilder::
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
        MY_LOGE("get resource container fail");
        goto lbExit;
    }
    pBufferProvider = pResourceContainer->queryConsumer(streamId);
    if(pBufferProvider == nullptr)
    {
        MY_LOGE("pBufferProvider is null");
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
ICaptureRequestBuilder::BufferData*
BayerAndBayerCaptureRequestBuilder::
getBufferDataFromSelect(
    sp<ISelector> selector,
    MINT32 iAppMetadataStreamId,
    MINT32 iHalMetadataStreamId
)
{
    if(selector == nullptr)
    {
        MY_LOGE("selector is nullptr");
        return nullptr;
    }
    BufferData *bufferData = new BufferData;
    Vector<ISelector::MetaItemSet> tempMetadata;
    MINT32 tempReqNo = -1;
    MBOOL hasAppDynamicData = MFALSE;
    MBOOL hasHalDynamicData = MFALSE;
    //
    if(((StereoSelector*)selector.get())->getResultZSL(tempReqNo, tempMetadata, bufferData->mBufferHeap) != OK){
        MY_LOGE("mSelector->getResultZSL failed!");
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
    bufferData->mBufferHeap = nullptr;
    delete bufferData;
    bufferData = nullptr;
    return nullptr;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
BayerAndBayerCaptureRequestBuilder::
prepareMetadata(
    Vector<BufferData*> vBufferDataList
)
{
    status_t ret = UNKNOWN_ERROR;
    // for dual cam use, vBufferDataList.size() is 2.
    // first element is main1 buffer data.
    // second element is main2 buffer data.
    if(vBufferDataList.size() != 2)
    {
        MY_LOGE("B+B capture builder needs two buffer data.");
        return ret;
    }
    // push main2 hal metadata to main1 hal metadata.
    IMetadata::IEntry entry_meta(MTK_P1NODE_MAIN2_HAL_META);
    entry_meta.push_back(vBufferDataList[1]->mHalMetadata, Type2Type< IMetadata >());
    vBufferDataList[0]->mHalMetadata.update(entry_meta.tag(), entry_meta);
    // set DOF level.
    {
        sp<IParamsManager> mParam = nullptr;
        if (mpParamMgrV3 != 0) {
            mParam = mpParamMgrV3->getParamsMgr();
        }
        else
        {
            MY_LOGE("get parameter manager fail");
            return UNKNOWN_ERROR;
        }
        MINT32 bokehLevel = mParam->getInt(MtkCameraParameters::KEY_STEREO_DOF_LEVEL);
        int DoFLevel_HAL = bokehLevel*2;
        int DoFMin_HAL = 0;
        int DoFMax_HAL = 30;

        DoFLevel_HAL = std::max(DoFMin_HAL, std::min(DoFLevel_HAL, DoFMax_HAL));

        MY_LOGD("Bokeh level(%d)", DoFLevel_HAL);
        IMetadata::IEntry entry(MTK_STEREO_FEATURE_DOF_LEVEL);
        entry.push_back(DoFLevel_HAL, Type2Type< MINT32 >());
        vBufferDataList[0]->mAppMetadata.update(entry.tag(), entry);
    }
    // for capture, force turn off 3dnr.
    {
        IMetadata::IEntry entry(MTK_NR_FEATURE_3DNR_MODE);
        entry.push_back(MFALSE, Type2Type< MINT32 >());
        vBufferDataList[0]->mAppMetadata.update(entry.tag(), entry);
    }
    // add capture intent
    {
        IMetadata::IEntry entry(MTK_CONTROL_CAPTURE_INTENT);
        entry.push_back(MTK_CONTROL_CAPTURE_INTENT_ZERO_SHUTTER_LAG, Type2Type< MUINT8 >());
        vBufferDataList[0]->mAppMetadata.update(entry.tag(), entry);
    }
    // force using hw to encode all jpeg
    {
        IMetadata::IEntry entry(MTK_JPG_ENCODE_TYPE);
        entry.push_back(NSCam::NSIoPipe::NSSImager::JPEGENC_HW_ONLY, Type2Type< MUINT8 >());
        vBufferDataList[0]->mHalMetadata.update(entry.tag(), entry);
    }

#if 1
    // Debug Log
    {
        MSize tempSize;
        if( ! tryGetMetadata<MSize>(&vBufferDataList[0]->mHalMetadata, MTK_HAL_REQUEST_SENSOR_SIZE, tempSize) ){
            MY_LOGE("cannot get MTK_HAL_REQUEST_SENSOR_SIZE after updating request");
        }else{
            MY_LOGD("MTK_HAL_REQUEST_SENSOR_SIZE:(%dx%d)", tempSize.w, tempSize.h);
        }
        IMetadata tempMetadata;
        if( ! tryGetMetadata<IMetadata>(&vBufferDataList[0]->mHalMetadata, MTK_P1NODE_MAIN2_HAL_META, tempMetadata) ){
            MY_LOGE("cannot get MTK_P1NODE_MAIN2_HAL_META after updating request");
        }else{
            MY_LOGD("MTK_P1NODE_MAIN2_HAL_META");
        }
        MINT32 tempLevel;
        if( ! tryGetMetadata<MINT32>(&vBufferDataList[0]->mAppMetadata, MTK_STEREO_FEATURE_DOF_LEVEL, tempLevel) ){
            MY_LOGE("cannot get MTK_STEREO_FEATURE_DOF_LEVEL after updating request");
        }else{
            MY_LOGD("MTK_STEREO_FEATURE_DOF_LEVEL:%d", tempLevel);
        }
        MINT32 tempTransform;
        if( ! tryGetMetadata<MINT32>(&vBufferDataList[0]->mAppMetadata, MTK_JPEG_ORIENTATION, tempTransform) ){
            MY_LOGE("cannot get MTK_JPEG_ORIENTATION after updating request");
        }else{
            MY_LOGD("MTK_JPEG_ORIENTATION:%d", tempTransform);
        }

        MINT64 timestamp_main1 = -1;
        MINT64 timestamp_main2 = -1;
        int timestamp_main1_ms = -1;
        int timestamp_main2_ms = -1;
        int timestamp_diff = -1;
        if( ! tryGetMetadata<MINT64>(&vBufferDataList[0]->mAppMetadata, MTK_SENSOR_TIMESTAMP, timestamp_main1)){
            MY_LOGE("cannot get timestamp_main1 after updating request");
        }else{
            // MY_LOGD("capture timestamp_main1:%lld", timestamp_main1);
            timestamp_main1_ms = timestamp_main1/1000000;
        }
        if( ! tryGetMetadata<MINT64>(&vBufferDataList[0]->mAppMetadata, MTK_SENSOR_TIMESTAMP, timestamp_main2)){
            MY_LOGE("cannot get timestamp_main2 after updating request");
        }else{
            // MY_LOGD("capture timestamp_main2:%lld", timestamp_main2);
            timestamp_main2_ms = timestamp_main2/1000000;
        }
        timestamp_diff     = timestamp_main1_ms - timestamp_main2_ms;
        MY_LOGD("capture_ts_diff:(main1/main2/diff)(%09d/%09d/%09d)(ms)",
            timestamp_main1_ms,
            timestamp_main2_ms,
            timestamp_diff
        );
    }
#endif
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
BayerAndBayerCaptureRequestBuilder::
pushBufferToPipelineEntryPool(
    sp<StereoBufferPool> pool,
    sp<IImageBufferHeap> bufferHeap,
    const char* name
)
{
    status_t ret = UNKNOWN_ERROR;
    if(pool == nullptr)
    {
        MY_LOGE("pool is null");
        return ret;
    }
    if(bufferHeap == nullptr)
    {
        MY_LOGE("bufferHeap is null");
        return ret;
    }
    pool->setBuffer(name, bufferHeap);
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
BayerAndBayerCaptureRequestBuilder::
prepareImageProvider(
)
{
    status_t ret = UNKNOWN_ERROR;
    if(mpCallbackBufferHandler == nullptr)
    {
        MY_LOGE("mpCallbackBufferHandler is nullptr");
        return ret;
    }
    if(mpCallbackImageBufferManager == nullptr)
    {
        MY_LOGE("mpCallbackImageBufferManager is nullptr");
        return ret;
    }
    auto createImageStreamInfo = [](
                char const* streamName,
                StreamId_T streamId,
                MUINT32 streamType,
                size_t maxBufNum,
                size_t minInitBufNum,
                MUINT usageForAllocator,
                MINT imgFormat,
                MSize const& imgSize,
                MUINT32 transform)
    {
        IImageStreamInfo::BufPlanes_t bufPlanes;
    #define addBufPlane(planes, height, stride)                                      \
            do{                                                                      \
                size_t _height = (size_t)(height);                                   \
                size_t _stride = (size_t)(stride);                                   \
                IImageStreamInfo::BufPlane bufPlane= { _height * _stride, _stride }; \
                planes.push_back(bufPlane);                                          \
            }while(0)
        switch( imgFormat ) {
            case eImgFmt_YV12:
                addBufPlane(bufPlanes , imgSize.h      , imgSize.w);
                addBufPlane(bufPlanes , imgSize.h >> 1 , imgSize.w >> 1);
                addBufPlane(bufPlanes , imgSize.h >> 1 , imgSize.w >> 1);
                break;
            case eImgFmt_NV21:
                addBufPlane(bufPlanes , imgSize.h      , imgSize.w);
                addBufPlane(bufPlanes , imgSize.h >> 1 , imgSize.w);
                break;
            case eImgFmt_RAW16:
            case eImgFmt_YUY2:
                addBufPlane(bufPlanes , imgSize.h      , imgSize.w << 1);
                break;
            case eImgFmt_Y8:
            case eImgFmt_STA_BYTE:
                addBufPlane(bufPlanes , imgSize.h      , imgSize.w);
                break;
            case eImgFmt_RGBA8888:
                addBufPlane(bufPlanes , imgSize.h      , imgSize.w<<2);
                break;
            case eImgFmt_BLOB:
                        /*
                        add 328448 for image size
                        standard exif: 1280 bytes
                        4 APPn for debug exif: 0xFF80*4 = 65408*4 bytes
                        max thumbnail size: 64K bytes
                        */
                addBufPlane(bufPlanes , 1              , (imgSize.w * imgSize.h * 12 / 10) + 328448); //328448 = 64K+1280+65408*4
                break;
            default:
                MY_LOGE("format not support yet %p", imgFormat);
                break;
        }
    #undef  addBufPlane

        sp<ImageStreamInfo>
            pStreamInfo = new ImageStreamInfo(
                    streamName,
                    streamId,
                    streamType,
                    maxBufNum, minInitBufNum,
                    usageForAllocator, imgFormat, imgSize, bufPlanes, transform
                    );

        if( pStreamInfo == NULL ) {
            MY_LOGE("create ImageStream failed, %s, %#"PRIx64,
                    streamName, streamId);
        }

        return pStreamInfo;
    };
    sp<IImageStreamInfo> extraDebugStreamInfo = nullptr;
    sp<IImageStreamInfo> depthStreamInfo = nullptr;
    sp<IImageStreamInfo> jpsStreamInfo = nullptr;
    sp<IImageStreamInfo> bokehResultStreamInfo = nullptr;
    sp<IImageStreamInfo> bokehCleanImageStreamInfo = nullptr;
    sp<IImageStreamInfo> jpgBokehImageStreamInfo = nullptr;
    sp<IImageStreamInfo> jpgCleanImageStreamInfo = nullptr;
    sp<IImageStreamInfo> thumbImageStreamInfo = nullptr;
    //
    StereoSizeProvider* pStereoSizeProvider = StereoSizeProvider::getInstance();
    StereoArea depthMap = pStereoSizeProvider->getBufferSize(
                                    E_DEPTH_MAP,
                                    eSTEREO_SCENARIO_CAPTURE);

    MSize finalDepthMapSize = MSize(depthMap.size.w, depthMap.size.h);
    MSize finalPictureSize = MSize(mShotParams.mShotParam.mi4PictureWidth, mShotParams.mShotParam.mi4PictureHeight);
    if(mShotParams.mShotParam.mu4Transform & eTransform_ROT_90){
        finalDepthMapSize = MSize(depthMap.size.h, depthMap.size.w);
        finalPictureSize = MSize(mShotParams.mShotParam.mi4PictureHeight, mShotParams.mShotParam.mi4PictureWidth);
    }
    MY_LOGD("final capture size: depthmap%dx%d, picture:%dx%d",
        finalDepthMapSize.w,
        finalDepthMapSize.h,
        finalPictureSize.w,
        finalPictureSize.h
    );
    // ExtraDebug
    {
        extraDebugStreamInfo =
                        createImageStreamInfo((std::string("Hal:Image:ExtraData")+std::to_string(miCaptureNo)).c_str(),
                                              eSTREAMID_IMAGE_PIPE_STEREO_DBG,
                                              eSTREAMTYPE_IMAGE_INOUT,
                                              1,
                                              1,
                                              eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN,
                                              eImgFmt_STA_BYTE,
                                              MSize(StereoSettingProvider::getExtraDataBufferSizeInBytes(), 1),
                                              0);
    }
    // JPS
    {
        jpsStreamInfo =
                        createImageStreamInfo((std::string("Hal:Image:JpegEnc_JPS")+std::to_string(miCaptureNo)).c_str(),
                                              eSTREAMID_IMAGE_PIPE_JPG_JPS,
                                              eSTREAMTYPE_IMAGE_INOUT,
                                              1,
                                              1,
                                              eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN,
                                              eImgFmt_BLOB,
                                              pStereoSizeProvider->getSBSImageSize(),
                                              0);
    }
    // thumbnail
    {
        thumbImageStreamInfo =
                        createImageStreamInfo((std::string("Hal:Image:Bokeh_Thumb")+std::to_string(miCaptureNo)).c_str(),
                                              eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL,
                                              eSTREAMTYPE_IMAGE_INOUT,
                                              1,
                                              1,
                                              eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN,
                                              eImgFmt_YV12,
                                              MSize(mShotParams.mJpegParam.mi4JpegThumbWidth, mShotParams.mJpegParam.mi4JpegThumbHeight),
                                              0);
    }
    // depthmap
    {
        depthStreamInfo =
                createImageStreamInfo((std::string("Hal:Image:DepthMap")+std::to_string(miCaptureNo)).c_str(),
                                      eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_DEPTHMAPYUV,
                                      eSTREAMTYPE_IMAGE_INOUT,
                                      1,
                                      1,
                                      eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE,
                                      eImgFmt_STA_BYTE,
                                      finalDepthMapSize,
                                      0);
    }
    // bokehResult
    {
        bokehResultStreamInfo =
                        createImageStreamInfo((std::string("Hal:Image:BokehResult")+std::to_string(miCaptureNo)).c_str(),
                                              eSTREAMID_IMAGE_PIPE_BOKEHNODE_RESULTYUV,
                                              eSTREAMTYPE_IMAGE_INOUT,
                                              1,
                                              1,
                                              eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE,
                                              eImgFmt_NV21,
                                              finalPictureSize,
                                              0);
    }
    // bokehClean
    {
        bokehCleanImageStreamInfo =
                        createImageStreamInfo((std::string("Hal:Image:CleanImage")+std::to_string(miCaptureNo)).c_str(),
                                              eSTREAMID_IMAGE_PIPE_BOKEHNODE_CLEANIMAGEYUV,
                                              eSTREAMTYPE_IMAGE_INOUT,
                                              1,
                                              1,
                                              eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE,
                                              eImgFmt_NV21,
                                              finalPictureSize,
                                              0);
    }
    // jpgBokeh
    {
        jpgBokehImageStreamInfo =
                        createImageStreamInfo((std::string("Hal:Image:JpegEnc_Bokeh")+std::to_string(miCaptureNo)).c_str(),
                                              eSTREAMID_IMAGE_PIPE_JPG_Bokeh,
                                              eSTREAMTYPE_IMAGE_INOUT,
                                              1,
                                              1,
                                              eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN,
                                              eImgFmt_BLOB,
                                              finalPictureSize,
                                              0);
    }
    // jpgClean
    {
        jpgCleanImageStreamInfo =
                        createImageStreamInfo((std::string("Hal:Image:JpegEnc_Clean")+std::to_string(miCaptureNo)).c_str(),
                                              eSTREAMID_IMAGE_PIPE_JPG_CleanMainImg,
                                              eSTREAMTYPE_IMAGE_INOUT,
                                              1,
                                              1,
                                              eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN,
                                              eImgFmt_BLOB,
                                              finalPictureSize,
                                              0);
    }
    // allocate buffer
    ret = mpCallbackImageBufferManager->allocBuffer(extraDebugStreamInfo,       mpCallbackBufferHandler);
    ret &= mpCallbackImageBufferManager->allocBuffer(jpsStreamInfo,              mpCallbackBufferHandler);
    ret &= mpCallbackImageBufferManager->allocBuffer(depthStreamInfo,            mpCallbackBufferHandler);
    ret &= mpCallbackImageBufferManager->allocBuffer(jpgBokehImageStreamInfo,    mpCallbackBufferHandler);
    ret &= mpCallbackImageBufferManager->allocBuffer(jpgCleanImageStreamInfo,    mpCallbackBufferHandler);
    ret &= mpCallbackImageBufferManager->allocBuffer(thumbImageStreamInfo);
    ret &= mpCallbackImageBufferManager->allocBuffer(bokehResultStreamInfo);
    ret &= mpCallbackImageBufferManager->allocBuffer(bokehCleanImageStreamInfo);
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
BayerAndBayerCaptureRequestBuilder::
getTimestamp(
    IMetadata metadata
)
{
    status_t ret = UNKNOWN_ERROR;
    if(tryGetMetadata<MINT64>(&metadata, MTK_SENSOR_TIMESTAMP, miTimeStamp))
    {
        MY_LOGD("timestamp_P1:%lld", miTimeStamp);
    }
    else
    {
        MY_LOGW("Can't get timestamp from metadata, set to 0");
        miTimeStamp = 0;
    }
    ret = OK;
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
BayerAndBayerCaptureRequestBuilder::
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
BayerAndBayerCaptureRequestBuilder::
setImageCallback(
)
{
    status_t ret = UNKNOWN_ERROR;
    if(mpShotCallbackProcessor == nullptr)
    {
        MY_LOGE("mpShotCallbackProcessor is null");
        return ret;
    }
    auto createCallback = [this](MUINT32 callbackType, IImageShotCallback::ImageCallbackParams params)
    {
        sp<IImageShotCallback> pImgCallback = 
                ImageShotCallbackFactory::createCallback(
                                        callbackType,
                                        params);
        if(pImgCallback!=nullptr)
        {
            mpShotCallbackProcessor->addCallback(pImgCallback);
        }
        else
        {
            MY_LOGE("create callback fail. streamId(0x%x)", params.streamId);
            return UNKNOWN_ERROR;
        }
        return OK;
    };
    IImageShotCallback::ImageCallbackParams params;
    // create bokeh result image callback
    {
        params.streamId = eSTREAMID_IMAGE_PIPE_JPG_Bokeh;
        params.streamName = "JPEG_BOKEH";
        params.fileExtension = ".jpg";
        params.callbackIndex = 0;
        params.msgType = MTK_CAMERA_MSG_EXT_DATA_COMPRESSED_IMAGE;
        ret = createCallback(ImageShotCallbackFactory::JPEG, params);
    }
    // create jps result image callback
    {
        params.streamId = eSTREAMID_IMAGE_PIPE_JPG_JPS;
        params.streamName = "JPEG_JPS";
        params.fileExtension = ".jpg";
        params.callbackIndex = 1;
        params.msgType = MTK_CAMERA_MSG_EXT_DATA_JPS;
        ret &= createCallback(ImageShotCallbackFactory::JPEG, params);
    }
    // create clear result image callback
    {
        params.streamId = eSTREAMID_IMAGE_PIPE_JPG_CleanMainImg;
        params.streamName = "JPEG_CLEAR";
        params.fileExtension = ".jpg";
        params.callbackIndex = 2;
        params.msgType = MTK_CAMERA_MSG_EXT_DATA_STEREO_CLEAR_IMAGE;
        ret &= createCallback(ImageShotCallbackFactory::JPEG, params);
    }
    // create extra debug image callback
    {
        params.streamId = eSTREAMID_IMAGE_PIPE_STEREO_DBG;
        params.streamName = "ExtraDebug";
        params.fileExtension = ".dbg";
        params.callbackIndex = 3;
        params.msgType = MTK_CAMERA_MSG_EXT_DATA_STEREO_DBG;
        ret &= createCallback(ImageShotCallbackFactory::EXTRADATA, params);
    }
    // create depthmap image callback
    {
        params.streamId = eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_DEPTHMAPYUV;
        params.streamName = "depthMap";
        params.fileExtension = ".yuv";
        params.callbackIndex = 4;
        params.msgType = MTK_CAMERA_MSG_EXT_DATA_DEPTHMAP;
        ret &= createCallback(ImageShotCallbackFactory::DEPTHMAP, params);
    }
    if(ret!=OK)
    {
        mpShotCallbackProcessor->resetProcessor();
    }
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
BayerAndBayerCaptureRequestBuilder::
setMetadataCallback(
)
{
    status_t ret = UNKNOWN_ERROR;
    if(mpShotCallbackProcessor == nullptr)
    {
        MY_LOGE("mpShotCallbackProcessor is null");
        return ret;
    }
    IMetadata metadata;
    sp<IMetaShotCallback> pMetaCallback =
                MetadataShotCallbackFactory::createCallback(
                                    MetadataShotCallbackFactory::SHUTTER,
                                    eSTREAMID_META_APP_DYNAMIC_DEPTH,
                                    metadata);
    ret = OK;
    return ret;
}