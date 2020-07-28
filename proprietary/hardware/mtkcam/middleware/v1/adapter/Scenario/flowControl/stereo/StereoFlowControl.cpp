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

#define LOG_TAG "MtkCam/StereoFlowControl"
//
#include "../MyUtils.h"
//
#include <mtkcam/pipeline/utils/streaminfo/MetaStreamInfo.h>
#include <mtkcam/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam/pipeline/utils/streambuf/StreamBuffers.h>

#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>

#include <mtkcam/middleware/v1/camutils/CamInfo.h>
#include <mtkcam/middleware/v1/IParamsManager.h>
#include <mtkcam/middleware/v1/LegacyPipeline/request/IRequestController.h>
#include <mtkcam/middleware/v1/LegacyPipeline/ILegacyPipeline.h>

#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>

#include <mtkcam/utils/fwk/MtkCamera.h>

#include <bandwidth_control.h>

#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>

#include "StereoFlowControl.h"
#include <mtkcam/drv/iopipe/SImager/ISImagerDataTypes.h>
#include <mtkcam/middleware/v1/camshot/BufferCallbackHandler.h>
//#include <StereoLegacyPipeline.h>
//
#include <string>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/StreamBufferProviderFactory.h>
#include <mtkcam/middleware/v1/camshot/BufferCallbackHandler.h>
#include <mtkcam/middleware/v1/LegacyPipeline/IResourceContainer.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/BufferPoolImp.h>
//
#include <mtkcam/feature/effectHalBase/ICallbackClientMgr.h>
//
#include <mtkcam/middleware/v1/PostProc/IPostProc.h>
//
using namespace NSCam;
using namespace NSCam::v1;
using namespace NSCam::v1::NSLegacyPipeline;
using namespace android;
using namespace NSCam::v3;
using namespace NSCam::v3::Utils;

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

#define MY_LOGD1(...)               MY_LOGD_IF(1<=mLogLevel, __VA_ARGS__)
#define P2_OPEN_ID     (28285)

#define CHECK_OBJECT(x)  do{                                        \
    if (x == nullptr) { MY_LOGE("Null %s Object", #x); return MFALSE;} \
} while(0)

// ----------------------------------------------------------------------------
// function utility
// ----------------------------------------------------------------------------
// function scope
#define __DEBUG
#define __SCOPE_TIMER
#ifdef __DEBUG
#define FUNCTION_SCOPE      auto __scope_logger__ = create_scope_logger(__FUNCTION__)
#include <memory>
static std::shared_ptr<char> create_scope_logger(const char* functionName)
{
    char* pText = const_cast<char*>(functionName);
    CAM_LOGD("[%s] +",pText);
    return std::shared_ptr<char>(pText, [](char* p){ CAM_LOGD("[%s] -", p); });
}
#else
#define FUNCTION_SCOPE
#endif // function scope
// SCOPE_TIMER
#ifdef __SCOPE_TIMER
#define SCOPE_TIMER(VAR, TEXT)  auto VAR = create_scope_timer(TEXT)
#include <memory>
#include <functional>
#include <utils/SystemClock.h> // elapsedRealtime(), uptimeMillis()
static std::unique_ptr<const char, std::function<void(const char*)> >
create_scope_timer(const char* text)
{
    auto t1 = android::elapsedRealtimeNano();
    return std::unique_ptr<const char, std::function<void(const char*)> >
        (
            text,
            [t1](const char* t)->void
            {
                auto t2 = android::elapsedRealtimeNano();
                MY_LOGD("%s --> duration(ns): %" PRId64 "", t, (t2 -t1));
            }
        );
}
#else
#define SCOPE_TIMER(VAR, TEXT)  do{}while(0)
#endif // SCOPE_TIMER
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
StereoFlowControl::
StereoFlowControl(
    char const*                 pcszName,
    MINT32 const                i4OpenId,
    sp<IParamsManagerV3>          pParamsManagerV3,
    sp<ImgBufProvidersManager>  pImgBufProvidersManager,
    sp<INotifyCallback>         pCamMsgCbInfo
)
    : mpParamsManagerV3(pParamsManagerV3)
    , mpImgBufProvidersMgr(pImgBufProvidersManager)
    , mName(const_cast<char*>(pcszName))
    , mpCamMsgCbInfo(pCamMsgCbInfo)
    , mOpenId_P2Prv(P2_OPEN_ID)
{
    FUNCTION_SCOPE;

    char cProperty[PROPERTY_VALUE_MAX];

    // it has to check module type and mode first.
    mCurrentStereoMode = StereoSettingProvider::getStereoFeatureMode();
    if(mCurrentStereoMode == 0)
    {
        MY_LOGD("get stereo mode for parameter.");
        mCurrentStereoMode = getStereoMode();
        StereoSettingProvider::setStereoFeatureMode(mCurrentStereoMode);
    }

    if(!StereoSettingProvider::getStereoSensorIndex(mOpenId, mOpenId_main2)){
        MY_LOGE("Cannot get sensor ids from StereoSettingProvider! (%d,%d)", mOpenId, mOpenId_main2);
        return;
    }

    if(getOpenId() != i4OpenId &&
       !StereoSettingProvider::isWideTeleVSDoF()){
        MY_LOGE("mOpenId(%d) != i4OpenId(%d), should not have happened!", getOpenId(), i4OpenId);
        return;
    }


    mpResourceContainier = IResourceContainer::getInstance(getOpenId());
    mpResourceContainierMain2 = IResourceContainer::getInstance(getOpenId_Main2());
    mpResourceContainierP2Prv = IResourceContainer::getInstance(getOpenId_P2Prv()); // Use this magic number to create consumer container for P2 Prv

    mCurrentSensorModuleType = getSensorModuleType();

    StereoSettingProvider::setStereoModuleType(mCurrentSensorModuleType);

    MY_LOGD("mCurrentSensorModuleType=%d, mCurrentStereoMode=%d", mCurrentSensorModuleType, mCurrentStereoMode);

    memset(cProperty, 0, PROPERTY_VALUE_MAX);
    ::property_get("vendor.camera.log", cProperty, "0");
    mLogLevel = ::atoi(cProperty);
    if ( 0 == mLogLevel ) {
        ::property_get("vendor.camera.log.stereoflow", cProperty, "0");
        mLogLevel = ::atoi(cProperty);
    }

    MY_LOGD("StereoFlowControl => mOpenId(%d), mOpenId_main2(%d), mOpenId_P2Prv(%d), mLogLevel(%d)",
        getOpenId(),
        getOpenId_Main2(),
        getOpenId_P2Prv(),
        mLogLevel
    );
    // get preview size
    int preview_width = 0;
    int preview_height = 0;
    mpParamsManagerV3->getParamsMgr()->getPreviewSize(&preview_width, &preview_height);
    MY_LOGD("width(%d) height(%d)", preview_width, preview_height);
    double ratio_4_3 = 4.0/3.0;
    double preview_ratio = ((double)preview_width) / ((double)preview_height);
    if(preview_ratio == ratio_4_3)
    {
        MY_LOGD("set to 4:3");
        StereoSettingProvider::setImageRatio(eRatio_4_3);
    }
    else
    {
        MY_LOGD("set to 16:9");
        StereoSettingProvider::setImageRatio(eRatio_16_9);
    }

    int capture_width = 0;
    int capture_height = 0;
    mpParamsManagerV3->getParamsMgr()->getBokehPictureSize(capture_width, capture_height);
    StereoSizeProvider::getInstance()->setCaptureImageSize(capture_width, capture_height);

    // for AP to read
    onSetStereoBufferSize();

    mpRequestListener = new StereoFlowControlListener(this);
    //
    if(::supportMain2FullRaw())
    {
        MY_LOGD("main2 will output imgo");
        mbMain2Full = MTRUE;
    }
    mbCanSupportBackgroundService = ICallbackClientMgr::getInstance()->canSupportBGService();
    //
    mCaptureModeType = ::property_get_int32("vendor.camera.log.scapmode", -1);
    if(mCaptureModeType == -1)
    {
        mCaptureModeType = ::getStereoCaptureModeType();
    }
    MY_LOGD("mbCanSupportBackgroundService(%d) mCaptureModeType(%d)",
                                        mbCanSupportBackgroundService,
                                        mCaptureModeType);
}

/******************************************************************************
 *
 ******************************************************************************/
char const*
StereoFlowControl::
getName()   const
{
    return mName;
}

/******************************************************************************
 *
 ******************************************************************************/
int32_t
StereoFlowControl::
getOpenId() const
{
    return mOpenId;
}

/******************************************************************************
 *
 ******************************************************************************/
int32_t
StereoFlowControl::
getOpenId_Main2() const
{
    return mOpenId_main2;
}

/******************************************************************************
 *
 ******************************************************************************/
int32_t
StereoFlowControl::
getOpenId_P2Prv() const
{
    return mOpenId_P2Prv;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
StereoFlowControl::
startPreview()
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(__t0, "startpreview");
    status_t ret = UNKNOWN_ERROR;
    // Scenario Control: CPU Core
    // for init time, set max power.
    enterMMDVFSScenario();
    enterPerformanceScenario(ScenarioMode::CAPTURE); // exit current scenario before entering the next one

    mPipelineMode = PipelineMode_ZSD;
    mIspProfile = NSIspTuning::EIspProfile_N3D_Preview;
    MY_LOGD("mPipelineMode=%d, mIspProfile=%d", mPipelineMode, mIspProfile);

    mpResourceContainier->setFeatureFlowControl(this);

    // get pass1 active array crop
    if(queryPass1ActiveArrayCrop() != OK)
    {
        ret = BAD_VALUE;
        goto lbExit;
    }

    // create request controllers
    if(createStereoRequestController() != OK)
    {
        ret = UNKNOWN_ERROR;
        goto lbExit;
    }

    // create pipelines
    MY_LOGD("create pipelines");

    //Turn on stereo flag for af mgr
    initAFMgr();
    setAFSyncMode();

    // get context builder table
    if(buildStereoPipeline(
                        mCurrentStereoMode,
                        mCurrentSensorModuleType,
                        mPipelineMode)!=OK)
    {
        MY_LOGE("get context builder content fail");
        ret = INVALID_OPERATION;
        goto lbExit;
    }

    if ( mpPipeline_P1 == 0 || mpPipeline_P1_Main2 == 0 || mpPipeline_P2 == 0) {
        MY_LOGE("Cannot get pipeline. start preview fail.");
        MY_LOGE_IF(mpPipeline_P1 == 0, "mpPipeline_P1 == 0");
        MY_LOGE_IF(mpPipeline_P1_Main2 == 0, "mpPipeline_P1_Main2 == 0");
        MY_LOGE_IF(mpPipeline_P2 == 0, "mpPipeline_P2 == 0");
        ret = BAD_VALUE;
        goto lbExit;
    }

    // create stereoSynchronizer
    if(setAndStartStereoSynchronizer() != OK)
    {
        ret = UNKNOWN_ERROR;
        goto lbExit;
    }

    // set status
    if(mpStereoRequestUpdater_P2 != nullptr)
    {
        mpStereoRequestUpdater_P2->setStereoFeatureStatus(MTK_STEREO_FEATURE_STATUS_PREVIEW);
    }

    // start pipeline
    if(startStereoPipeline(
                        STEREO_FLOW_PREVIEW_REQUSET_NUM_START,
                        STEREO_FLOW_PREVIEW_REQUSET_NUM_END) != OK)
    {
        ret = UNKNOWN_ERROR;
        goto lbExit;
    }

    #ifdef ENABLE_TEMP_MONITOR
    // temperature monitor
    mpTemperatureMonitor = ITemperatureMonitor::create();
    if(mpTemperatureMonitor != nullptr)
    {
        MINT32 dev_main1 = -1, dev_main2 = -1;
        if(!StereoSettingProvider::getStereoSensorDevIndex(dev_main1, dev_main2)){
            MY_LOGE("Cannot get sensor dev ids from StereoSettingProvider! (%d,%d)", dev_main1, dev_main2);
            ret = UNKNOWN_ERROR;
            goto lbExit;
        }
        mpTemperatureMonitor->addToObserve(mOpenId, dev_main1);
        mpTemperatureMonitor->addToObserve(mOpenId_main2, dev_main2);
        mpTemperatureMonitor->init();
        mpTemperatureMonitor->run("TemperatureMonitor");
    }
    #endif
    enterPerformanceScenario(ScenarioMode::PREVIEW); // exit current scenario before entering the next one
    isStopPreview = MFALSE;

    mbPreviewEnable = MTRUE;
    ret = OK;
lbExit:
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
StereoFlowControl::
stopPreview()
{
    FUNCTION_SCOPE;
    {
        Mutex::Autolock _l(mCaptureLock);
        isStopPreview = MTRUE;
        mCondCaptureLock.signal();
    }

    {
        Mutex::Autolock _l(mCaptureQueueLock);
        while(!mvCaptureQueue.empty())
        {
            MY_LOGD("Capture queue size(%d)", mvCaptureQueue.size());
                status_t status = mCondCaptureLock.waitRelative(mCaptureQueueLock, 3000000000LL); // wait for most 3 secs

            if(status != OK && !mvCaptureQueue.empty()){
                MY_LOGW("still capture request in queue but already timeout!");
                    std::queue< sp<ICaptureRequestBuilder>> empty;
                    std::swap( mvCaptureQueue, empty );
                break;
            }
        }
    }
    Mutex::Autolock _l(mCaptureLock);
    //
    uninitPipelineAndRelatedResource();

    // set status
    if(mpStereoRequestUpdater_P2 != nullptr)
    {
        mpStereoRequestUpdater_P2->setStereoFeatureStatus(MTK_STEREO_FEATURE_STATUS_OFF);
    }

    uninitAFMgr();
    resetAFSyncMode();

    #ifdef ENABLE_TEMP_MONITOR
    // uninit temperator monitor
    if(mpTemperatureMonitor!=nullptr)
    {
        mpTemperatureMonitor->requestExit();
        mpTemperatureMonitor->join();
        mpTemperatureMonitor->uninit();
        mpTemperatureMonitor = nullptr;
    }
    #endif

    exitPerformanceScenario();
    exitMMDVFSScenario();

    // release member object
    mpPipeline_P2 = nullptr;
    mpPipeline_P1 = nullptr;
    mpPipeline_P1_Main2 = nullptr;
    if(mpImageStreamManager != nullptr)
    {
        mpImageStreamManager->destroy();
        mpImageStreamManager=nullptr;
    }
    mpShotCb = nullptr;
    mpParamsManagerV3 = nullptr;
    mpImgBufProvidersMgr = nullptr;
    mpCamMsgCbInfo = nullptr;
    mpResourceContainier = nullptr;
    mpResourceContainierMain2 = nullptr;
    mpResourceContainierP2Prv = nullptr;
    mpStereoBufferSynchronizer = nullptr;
    IResourceContainer::getInstance(getOpenId())->clearBufferProviders();
    IResourceContainer::getInstance(getOpenId_Main2())->clearBufferProviders();
    IResourceContainer::getInstance(getOpenId_P2Prv())->clearBufferProviders();
    IResourceContainer::getInstance(getOpenId())->clearPipelineResource();
    IResourceContainer::getInstance(getOpenId_Main2())->clearPipelineResource();
    IResourceContainer::getInstance(getOpenId_P2Prv())->clearPipelineResource();
    mbPreviewEnable = MFALSE;

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
StereoFlowControl::
pausePreview(
    MBOOL stopPipeline
)
{
    MY_LOGE("no implementation!");
    return UNKNOWN_ERROR;
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
StereoFlowControl::
highQualityZoom()
{
    MY_LOGD("no implementation");
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
StereoFlowControl::
resumePreview()
{
    MY_LOGE("no implementation!");
    return UNKNOWN_ERROR;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
StereoFlowControl::
startRecording()
{
    FUNCTION_SCOPE;
    if(mPipelineMode == PipelineMode_RECORDING)
    {
        MY_LOGD("Pipeline alread exist");
        return OK;
    }

    mPipelineMode = PipelineMode_RECORDING;
    mIspProfile = NSIspTuning::EIspProfile_N3D_Video;
    MY_LOGD("mPipelineMode=%d, mIspProfile=%d", mPipelineMode, mIspProfile);

    // get pass1 active array crop
    if(queryPass1ActiveArrayCrop() != OK)
    {
        return BAD_VALUE;
    }

    // create request controllers
    if(createStereoRequestController() != OK)
    {
        return UNKNOWN_ERROR;
    }

    setRequsetTypeForAllPipelines(MTK_CONTROL_CAPTURE_INTENT_VIDEO_RECORD);

    // create pipelines
    MY_LOGD("create pipelines");

    //Turn on stereo flag for af mgr
    initAFMgr();
    setAFSyncMode();

    // get context builder table
    if(buildStereoPipeline(
                        mCurrentStereoMode,
                        mCurrentSensorModuleType,
                        mPipelineMode)!=OK)
    {
        MY_LOGE("get context builder content fail");
        return INVALID_OPERATION;
    }

    if ( mpPipeline_P1 == 0 || mpPipeline_P1_Main2 == 0 || mpPipeline_P2 == 0) {
        MY_LOGE("Cannot get pipeline. start recording fail.");
        MY_LOGE_IF(mpPipeline_P1 == 0, "mpPipeline_P1 == 0");
        MY_LOGE_IF(mpPipeline_P1_Main2 == 0, "mpPipeline_P1_Main2 == 0");
        MY_LOGE_IF(mpPipeline_P2 == 0, "mpPipeline_P2 == 0");
        return BAD_VALUE;
    }

    // Scenario Control: CPU Core
    enterMMDVFSScenario();
    enterPerformanceScenario(ScenarioMode::PREVIEW);

    // create stereoSynchronizer
    if(setAndStartStereoSynchronizer() != OK)
    {
        return UNKNOWN_ERROR;
    }

    // set status
    if(mpStereoRequestUpdater_P2 != nullptr)
    {
        mpStereoRequestUpdater_P2->setStereoFeatureStatus(MTK_STEREO_FEATURE_STATUS_RECORD);
    }

    // start pipeline
    if(startStereoPipeline(
                        STEREO_FLOW_PREVIEW_REQUSET_NUM_START,
                        STEREO_FLOW_PREVIEW_REQUSET_NUM_END) != OK)
    {
        return UNKNOWN_ERROR;
    }


    #ifdef ENABLE_TEMP_MONITOR
    // temperature monitor
    mpTemperatureMonitor = ITemperatureMonitor::create();
    if(mpTemperatureMonitor != nullptr)
    {
        MINT32 dev_main1 = -1, dev_main2 = -1;
        if(!StereoSettingProvider::getStereoSensorDevIndex(dev_main1, dev_main2)){
            MY_LOGE("Cannot get sensor dev ids from StereoSettingProvider! (%d,%d)", dev_main1, dev_main2);
            goto lbExit;
        }
        mpTemperatureMonitor->addToObserve(mOpenId, dev_main1);
        mpTemperatureMonitor->addToObserve(mOpenId_main2, dev_main2);
        mpTemperatureMonitor->init();
        mpTemperatureMonitor->run("TemperatureMonitor");
    }
    #endif
    mbPreviewEnable = MTRUE;
lbExit:
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
StereoFlowControl::
stopRecording()
{
    FUNCTION_SCOPE;
    // do nothing to reduce stopRecord time
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
StereoFlowControl::
autoFocus()
{
    // also send auto focus to P2 pipeline for touch point update
    if(mpRequestController_P2 != 0){
        if(mpRequestController_P2->autoFocus() != OK){
            MY_LOGE("mpRequestController_P2->autoFocus failed!");
        }
    }

    // send auto focus to P1_main2 pipeline
    if(mpRequestController_P1_Main2 != 0){
        if(mpRequestController_P1_Main2->autoFocus() != OK){
            MY_LOGE("mpRequestController_P1_Main2->autoFocus failed!");
        }
    }

    // send auto focus to P1 pipeline
    return (mpRequestController_P1 != 0) ?
        mpRequestController_P1->autoFocus() : OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
StereoFlowControl::
cancelAutoFocus()
{
    if(mpRequestController_P2 != 0){
        if(mpRequestController_P2->cancelAutoFocus() != OK){
            MY_LOGE("mpRequestController_P2->cancelAutoFocus failed!");
        }
    }

    if(mpRequestController_P1_Main2 != 0){
        if(mpRequestController_P1_Main2->cancelAutoFocus() != OK){
            MY_LOGE("mpRequestController_P1_Main2->cancelAutoFocus failed!");
        }
    }

    return (mpRequestController_P1 != 0) ?
        mpRequestController_P1->cancelAutoFocus() : OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
StereoFlowControl::
precapture(int& flashRequired)
{
    return (mpRequestController_P1 != 0) ?
        mpRequestController_P1->precapture(flashRequired) : OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
StereoFlowControl::
takePicture()
{
    FUNCTION_SCOPE;
    MY_LOGE("not implementation");
    return UNKNOWN_ERROR;
}

/******************************************************************************
 * Take Picture with StereoShotParam
 ******************************************************************************/
status_t
StereoFlowControl::
takePicture(StereoShotParam shotParam)
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(__t1, "takepicture");
    status_t ret = UNKNOWN_ERROR;
    {
        // init mbIsCancelRequest value.
        // each capture request needs to reset this value.
        Mutex::Autolock _l(mCancelRequestLock);
        mbIsCancelRequest = MFALSE;
    }

    MINT32 reqNo = miCapCounter;
    CaptureBufferData bufferData;
    IMetadata appMeta = shotParam.mShotParam.mAppSetting;
    IMetadata halMeta;
    BufferList vDstStreams;
    sp<ICaptureRequestBuilder> pCaptureRequest = nullptr;
    CaptureRequestInitSetting initSetting;
    CaptureSetting captureSetting;
    //
    {// check capture count
        Mutex::Autolock _l(mCaptureQueueLock);
        android::NSPostProc::IImagePostProcessManager* pIPP =
                                android::NSPostProc::IImagePostProcessManager::getInstance();
        MINT32 postProcCnt = 0;
        if(pIPP != nullptr)
        {
            postProcCnt = pIPP->size();
        }
        if((mvCaptureQueue.size()+postProcCnt) > ::get_vsdof_cap_queue_size())
        {
            MY_LOGD("wait capture +");
            mCondCaptureLock.wait(mCaptureQueueLock);
            MY_LOGD("wait capture -");
        }
    }
    // deal with stoppreview case.
    // stoppreview will call cancelTakePicture first.
    // to avoid wait in mCondCaptureLock.
    {
        Mutex::Autolock _l(mCancelRequestLock);
        if(mbIsCancelRequest)
        {
            MY_LOGD("capture request is be canceled.");
            return OK;
        }
    }
    {
        Mutex::Autolock _l(mCaptureLock);
        //Turn on stereo flag for af mgr
        initAFMgr();
        setAFSyncMode();
        auto setBufferPool = [&captureSetting](
                                MINT32 openId,
                                StreamId_T poolStreamId,
                                sp<StereoBufferPool>& mpPool,
                                String8 name)
        {
            if(mpPool == nullptr)
                return;
            std::shared_ptr<BufferPoolInfo> pPoolInfo =
                    std::shared_ptr<BufferPoolInfo>(
                        new BufferPoolInfo(),
                        [&](auto *p)->void{
                            if(p!=nullptr)
                            {
                                MY_LOGD("release pool [%d:%ld]",
                                            p->openId,
                                            p->poolStreamId);
                                p->mpPool = nullptr;
                                delete p;
                                p = nullptr;
                            }
                        });
            pPoolInfo->openId = openId;
            pPoolInfo->poolStreamId = poolStreamId;
            pPoolInfo->mpPool = mpPool;
            pPoolInfo->name = name;
            captureSetting.mvBufferPool.push_back(pPoolInfo);
            // print available buffer infomation

            MY_LOGD("[%s]  after preview drain buffer size (%d/%d)",
                name.string(),
                mpPool->getCurrentAvailableBufSize(),
                mpPool->getInUseBufSize()
            );
            return;
        };
        if(isDNGEnable)
        {
            MY_LOGD("Dng is enabled, but vsdof is not support");
        }
        // create capture request builder
        if(mbCanSupportBackgroundService)
        {
            if(Stereo::STEREO_3RDPARTY == mCurrentStereoMode ||
               Stereo::STEREO_TK_DEPTH_3RD_PRV)
            {
                pCaptureRequest = CaptureRequestBuilder::
                                        createCaptureRequestBuilder(
                                            CaptureRequestBuilder::BuilderMode::DUAL_3RD_CAPTURE);
            }
            else
            {
                pCaptureRequest = CaptureRequestBuilder::
                                        createCaptureRequestBuilder(
                                            CaptureRequestBuilder::BuilderMode::DEPTH_RESULT);
            }
        }
        else{
            if(Stereo::STEREO_3RDPARTY == mCurrentStereoMode ||
               Stereo::STEREO_TK_DEPTH_3RD_PRV)
            {
                pCaptureRequest = CaptureRequestBuilder::
                                        createCaptureRequestBuilder(
                                            CaptureRequestBuilder::BuilderMode::DUAL_3RD_CAPTURE);
            }
            else
            {
                pCaptureRequest = CaptureRequestBuilder::
                                        createCaptureRequestBuilder(
                                            CaptureRequestBuilder::BuilderMode::SW_VSDOF);
            }
        }
        if(pCaptureRequest == nullptr)
        {
            MY_LOGE("create capture request builder fail");
            goto lbExit;
        }
        {
            Mutex::Autolock _l(mCaptureQueueLock);
            mvCaptureQueue.push(pCaptureRequest);

            // set status
            if(mpStereoRequestUpdater_P2 != nullptr)
            {
                mpStereoRequestUpdater_P2->setStereoFeatureStatus(MTK_STEREO_FEATURE_STATUS_CAPTURE);
            }
        }

        MY_LOGD("waitAndLock synchronizer");
        mpStereoBufferSynchronizer->waitAndLockCapture();

        {
            MY_LOGD("pause P2 pipeline");
            vDstStreams.clear();
            mpRequestController_P2->pausePipeline(vDstStreams);
            mbPreviewEnable = MFALSE;
            MY_LOGD("preview enable(%d)", mbPreviewEnable);
        }

        // switch to high performance mode for capture
        enterPerformanceScenario(ScenarioMode::CAPTURE);
        // prepare init config
        initSetting.iTargetPipelineId = getOpenId_P2Prv();
        initSetting.iCaptureNo = miCapCounter;
        initSetting.shotParam = shotParam;
        initSetting.pParamMgr = mpParamsManagerV3;
        initSetting.pILegacyPipeline = mpPipeline_P2;
        initSetting.appMetadata = shotParam.mShotParam.mAppSetting;
        initSetting.halMetadata = shotParam.mShotParam.mHalSetting;
        initSetting.userData = nullptr;
        initSetting.content = mCurrentPipelineContent.mCapContent;
        initSetting.mCB = mpRequestListener;
        sp<IShotCallback> pShotCb = mpShotCb.promote();
        initSetting.mShotCallback = pShotCb;
        initSetting.pImageStreamManager = mpImageStreamManager;
        initSetting.mPostProcCB = mpRequestListener;
        if(mbCanSupportBackgroundService && mCaptureModeType == 0)
        {
            initSetting.mPostProcType = android::NSPostProc::PostProcessorType::BOKEH;
        }
        else if(mCaptureModeType == 1)
        {
            initSetting.mPostProcType = android::NSPostProc::PostProcessorType::THIRDPARTY_BOKEH;
        }
        if(pCaptureRequest->init(initSetting) != OK)
        {
            MY_LOGE("some error occuer, please check");
            goto lbExit;
        }
        // prepare capture setting
        setBufferPool(getOpenId(), eSTREAMID_IMAGE_PIPE_RAW_OPAQUE, mpStereoBufferPool_OPAQUE, String8("M1_F_POOL"));
        setBufferPool(getOpenId(), eSTREAMID_IMAGE_PIPE_RAW_RESIZER, mpStereoBufferPool_RESIZER, String8("M1_R_POOL"));
        setBufferPool(getOpenId(), eSTREAMID_IMAGE_PIPE_RAW_LCSO, mpStereoBufferPool_LCS, String8("M1_L_POOL"));
        setBufferPool(getOpenId_Main2(), eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01, mpStereoBufferPool_OPAQUE_MAIN2, String8("M2_F_POOL"));
        setBufferPool(getOpenId_Main2(), eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01, mpStereoBufferPool_RESIZER_MAIN2, String8("M2_R_POOL"));
        setBufferPool(getOpenId_Main2(), eSTREAMID_IMAGE_PIPE_RAW_LCSO_01, mpStereoBufferPool_LCS_MAIN2, String8("M2_L_POOL"));
        if(pCaptureRequest->doCapture(captureSetting) != OK)
        {
            MY_LOGE("some error occuer, please check");
            goto lbExit;
        }
        mpStereoBufferSynchronizer->unlockCapture();

        miCapCounter++;
        if(miCapCounter > STEREO_FLOW_CAPTURE_REQUSET_NUM_END)
        {
            miCapCounter = STEREO_FLOW_CAPTURE_REQUSET_NUM_START;
        }
    }
    ret = OK;
lbExit:
    if(ret != OK)
    {
        Mutex::Autolock _l(mCaptureQueueLock);
        mvCaptureQueue.pop();

        // set status
        if(mpStereoRequestUpdater_P2 != nullptr)
        {
            mpStereoRequestUpdater_P2->setStereoFeatureStatus(MTK_STEREO_FEATURE_STATUS_PREVIEW);
        }
        }
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
StereoFlowControl::
cancelTakePicture()
{
    FUNCTION_SCOPE;
    Mutex::Autolock _l(mCancelRequestLock);
    mbIsCancelRequest = MTRUE;
    mCondCaptureLock.signal();
    return OK;
}

/******************************************************************************
 * set callback function
 ******************************************************************************/
MBOOL
StereoFlowControl::
setCallbacks(sp<IShotCallback> pShotCb)
{
    if(pShotCb!=nullptr)
    {
        mpShotCb = pShotCb;
        // if background is disable, use AOSP flow
        if(!mbCanSupportBackgroundService)
        {
            android::NSPostProc::IImagePostProcessManager* pIPP =
                                        android::NSPostProc::IImagePostProcessManager::getInstance();
            if(pIPP!=nullptr)
            {
                pIPP->setShotCallback(mpShotCb.promote());
            }
        }
        return MTRUE;
    }
    else
    {
        MY_LOGE("pShotCb is null");
        return MFALSE;
    }
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
StereoFlowControl::
setParameters()
{
    FUNCTION_SCOPE;
    MERROR ret = OK;

    if(mpRequestController_P1 != 0){
        ret = mpRequestController_P1->setParameters( this );
        MY_LOGE_IF(ret != OK, "mpRequestController_P1->setParameters failed!");
    }
    if(mpRequestController_P1_Main2 != 0){
        ret = ret & mpRequestController_P1_Main2->setParameters( mpStereoRequestUpdater_P1_Main2 );
        MY_LOGE_IF(ret != OK, "mpRequestController_P1_Main2->setParameters failed!");
    }
    if(mpRequestController_P2 != 0){
        ret = ret & mpRequestController_P2->setParameters( mpStereoRequestUpdater_P2 );
        MY_LOGE_IF(ret != OK, "mpRequestController_P2->setParameters failed!");
    }

    // Check if set stereo buffer size to parameter manager again
    if (mpParamsManagerV3->getParamsMgr()->getInt(MtkCameraParameters::KEY_STEREO_LDC_SIZE) < 0)
    {
        onSetStereoBufferSize();
    }
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
StereoFlowControl::
sendCommand(
    int32_t cmd,
    int32_t arg1,
    int32_t arg2
)
{
    MERROR ret = INVALID_OPERATION;
    switch(cmd)
    {
        case CONTROL_DNG_FLAG:
            isDNGEnable = arg1;
            break;
        default:
            break;
    }

    if(mpRequestController_P1_Main2 != 0){
        ret = ret & mpRequestController_P1_Main2->sendCommand( cmd, arg1, arg2 );
    }
    if(mpRequestController_P2 != 0){
        ret = ret & mpRequestController_P2->sendCommand( cmd, arg1, arg2 );
    }

    return (mpRequestController_P1 != 0) ?
        mpRequestController_P1->sendCommand( cmd, arg1, arg2 ) : INVALID_OPERATION;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
StereoFlowControl::
dump(
    int fd,
    Vector<String8>const& args
)
{
#warning "TODO"
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
StereoFlowControl::
updateParametersCommon(
    IMetadata* setting,
    sp<IParamsManagerV3> pParamsMgrV3,
    StereoPipelineSensorParam& sensorParam
)
{
    MERROR ret = UNKNOWN_ERROR;

    sp<IParamsManager> paramsMgr = pParamsMgrV3->getParamsMgr();
    if(paramsMgr == NULL){
        MY_LOGE("paramsMgr == NULL");
        return UNKNOWN_ERROR;
    }

    // auto-flicker check
    {
        MtkCameraParameters newParams;

        newParams.unflatten(paramsMgr->flatten());

        const char *abMode = newParams.get(CameraParameters::KEY_ANTIBANDING);

        if(strcmp(abMode, CameraParameters::ANTIBANDING_AUTO) == 0){
            MY_LOGD1("Should not use auto-flicker in stereo mode, force switch to OFF!");
            paramsMgr->set(CameraParameters::KEY_ANTIBANDING, CameraParameters::ANTIBANDING_OFF);
        }
    }

    // EIS Metadata setting : Preview
    if( mPipelineMode == PipelineMode_ZSD)
    {
        pParamsMgrV3->updateRequestPreview(setting);
    }
    // Record
    else if(mPipelineMode == PipelineMode_RECORDING)
    {
        pParamsMgrV3->updateRequestRecord(setting);
    }

    ret = pParamsMgrV3->updateRequest(setting, sensorParam.mode);

    const char* value = paramsMgr->getStr(MtkCameraParameters::KEY_STEREO_PREVIEW_ENABLE);
    if(::strcmp(value, "") != 0)
    {
        // check if p2 pipeline is enable
        bool isP2Enable = paramsMgr->isEnabled(MtkCameraParameters::KEY_STEREO_PREVIEW_ENABLE);
        if(isP2Enable)
        {
            MY_LOGD("Enable preview");
            if(mpRequestController_P2 != nullptr)
            {
                mpStereoBufferSynchronizer->flushCaptureQueue();
                mpRequestController_P2->resumePipeline();
            }
        }
        else
        {
            MY_LOGD("Disable preview");
            if(mpRequestController_P2 != nullptr)
            {
                BufferList vDstStreams;
                vDstStreams.clear();
                mpRequestController_P2->pausePipeline(vDstStreams);
            }
        }
    }

    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
StereoFlowControl::
updateParameters(
    IMetadata* setting
)
{
    MERROR ret = UNKNOWN_ERROR;
    if( mpParamsManagerV3 != 0 )
    {

        ret = updateParametersCommon(setting, getParamsMgrV3(), mSensorParam);

        #if 0
        IMetadata::IEntry entry(MTK_STATISTICS_LENS_SHADING_MAP_MODE);
        entry.push_back(MTK_STATISTICS_LENS_SHADING_MAP_MODE_ON, Type2Type< MUINT8 >());
        setting->update(entry.tag(), entry);
        #endif
    }
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
StereoFlowControl::
updateRequestSetting(
    IMetadata* appSetting,
    IMetadata* halSetting
)
{
    // update app control
    {
        IMetadata::IEntry entry(MTK_SCALER_CROP_REGION);
        entry.push_back(this->getActiveArrayCrop(), Type2Type<MRect>());
        appSetting->update(MTK_SCALER_CROP_REGION, entry);

        // add 3DNR flag
        MINT32 mode3DNR = MTK_NR_FEATURE_3DNR_MODE_OFF;
        if(::strcmp(mpParamsManagerV3->getParamsMgr()->getStr
                    (MtkCameraParameters::KEY_3DNR_MODE),
                    "on") == 0 )
        {
            MY_LOGD1("3DNR on");
            mode3DNR = MTK_NR_FEATURE_3DNR_MODE_ON;
        }
        IMetadata::IEntry entry2(MTK_NR_FEATURE_3DNR_MODE);
        entry2.push_back(mode3DNR, Type2Type< MINT32 >());
        appSetting->update(entry2.tag(), entry2);
    }

    // update hal control
    {
        IMetadata::IEntry entry(MTK_HAL_REQUEST_SENSOR_SIZE);
        entry.push_back(mSensorParam.size, Type2Type< MSize >());
        halSetting->update(entry.tag(), entry);

        IMetadata::IEntry entry2(MTK_P1NODE_SENSOR_CROP_REGION);
        entry2.push_back(this->getSensorDomainCrop(), Type2Type<MRect>());
        halSetting->update(MTK_P1NODE_SENSOR_CROP_REGION, entry2);

        // Always request exif since stereo is zsd flow
        IMetadata::IEntry entry3(MTK_HAL_REQUEST_REQUIRE_EXIF);
        entry3.push_back(true, Type2Type< MUINT8 >());
        halSetting->update(entry3.tag(), entry3);

        IMetadata::IEntry entry4(MTK_3A_ISP_PROFILE);
        entry4.push_back(this->getIspProfile(), Type2Type< MUINT8 >());
        halSetting->update(entry4.tag(), entry4);

        if(::strcmp(getParamsMgr()->getStr(MtkCameraParameters::KEY_STEREO_PREVIEW_ENABLE),"") != 0 )
        {
            bool isP2Enable = getParamsMgr()->isEnabled(
                                MtkCameraParameters::KEY_STEREO_PREVIEW_ENABLE);
            IMetadata::IEntry entry(MTK_FOCUS_PAUSE);
            entry.push_back(isP2Enable!=true, Type2Type< MUINT8 >());
            halSetting->update(entry.tag(), entry);
        }

        // use MTK_STEREO_SYNCAF_MODE to enable AF sync
        IMetadata::IEntry entry6(MTK_STEREO_SYNCAF_MODE);
        entry6.push_back(1, Type2Type<MINT32>());
        halSetting->update(MTK_STEREO_SYNCAF_MODE, entry6);

        // use MTK_STEREO_HW_FRM_SYNC_MODE to enable HW sync
        IMetadata::IEntry entry7(MTK_STEREO_HW_FRM_SYNC_MODE);
        entry7.push_back(1, Type2Type<MINT32>());
        halSetting->update(MTK_STEREO_HW_FRM_SYNC_MODE,entry7);

        // add main1 and main2 openid to metadata
        IMetadata::IEntry tag(MTK_STEREO_SYNC2A_MASTER_SLAVE);
        tag.push_back(getOpenId(), Type2Type<MINT32>());
        tag.push_back(getOpenId_Main2(), Type2Type<MINT32>());
        halSetting->update(MTK_STEREO_SYNC2A_MASTER_SLAVE, tag);

    }

    mpParamsManagerV3->updateRequestHal(halSetting,mSensorParam.mode);

    MY_LOGD1("P1 main1 udpate request sensorSize:(%dx%d)  sensorCrop:(%d,%d,%d,%d)",
        mSensorParam.size.w,
        mSensorParam.size.h,
        this->getSensorDomainCrop().p.x,
        this->getSensorDomainCrop().p.y,
        this->getSensorDomainCrop().s.w,
        this->getSensorDomainCrop().s.h
    );

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
StereoFlowControl::
submitRequest(
    Vector< SettingSet > rvSettings,
    BufferList           rvDstStreams,
    Vector< MINT32 >&    rvRequestNo
)
{
    status_t ret = UNKNOWN_ERROR;
    if( mpRequestController_P2 == NULL)
    {
        MY_LOGE("mpRequestController_P2 is NULL");
        return UNKNOWN_ERROR;
    }
    //
    Vector< BufferList >  vDstStreams;
    for (size_t i = 0; i < rvSettings.size(); ++i) {
        vDstStreams.push_back(rvDstStreams);
    }
    ret = mpRequestController_P2->submitRequest( rvSettings, vDstStreams, rvRequestNo );

    if( ret != OK)
    {
        MY_LOGE("submitRequest Fail!");
        return UNKNOWN_ERROR;
    }
    //
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
StereoFlowControl::
submitRequest(
    Vector< SettingSet > rvSettings,
    Vector< BufferList > rvDstStreams,
    Vector< MINT32 >&    rvRequestNo
)
{
    MY_LOGE("No implementation!");
    return UNKNOWN_ERROR;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
StereoFlowControl::
onLastStrongRef( const void* /*id*/)
{
    FUNCTION_SCOPE;
    mpRequestController_P1 = NULL;
    if ( mpPipeline_P1 != 0 ) {
        mpPipeline_P1->flush();
        mpPipeline_P1->waitUntilDrained();
        mpPipeline_P1 = NULL;
    }
    mpRequestController_P1_Main2 = NULL;
    if ( mpPipeline_P1_Main2 != 0 ) {
        mpPipeline_P1_Main2->flush();
        mpPipeline_P1_Main2->waitUntilDrained();
        mpPipeline_P1_Main2 = NULL;
    }
    mpRequestController_P2 = NULL;
    if ( mpPipeline_P2 != 0 ) {
        mpPipeline_P2->flush();
        mpPipeline_P2->waitUntilDrained();
        mpPipeline_P2 = NULL;
    }
}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
StereoFlowControl::
checkNotifyCallback(
    IMetadata* meta
)
{
    MY_LOGD1("+");

    // check stereo warning
    {
        MINT32 newResult = -1;
        if(tryGetMetadata<MINT32>(meta, MTK_STEREO_FEATURE_WARNING, newResult)){
            MY_LOGD1("check cb MTK_STEREO_FEATURE_WARNING: %d  ignore:%d", newResult, mbIgnoreStereoWarning);
            if(newResult != mCurrentStereoWarning){
                // to adapter
                if(mbIgnoreStereoWarning){
                    // do nothing
                }else{
                    MY_LOGD("do cb MTK_STEREO_FEATURE_WARNING: %d", newResult);
                    mpCamMsgCbInfo->doNotifyCallback(
                        MTK_CAMERA_MSG_EXT_NOTIFY,
                        MTK_CAMERA_MSG_EXT_NOTIFY_STEREO_WARNING,
                        newResult
                    );
                }
            }

            mCurrentStereoWarning = newResult;
        }
    }

    MY_LOGD1("-");
    return OK;
}

/******************************************************************************
 * for developing purpose, we can block P2 pipeline by this function
 ******************************************************************************/
MBOOL
StereoFlowControl::
waitP2PrvReady()
{
    MY_LOGD1("+");

    // MY_LOGW("temp block waitP2PrvReady");
    // mbEnableP2Prv = MFALSE;

    Mutex::Autolock _l(mP2PrvLock);
    while(!mbEnableP2Prv){
        MY_LOGD1("wait for mbEnableP2Prv");
        mCondP2PrvLock.wait(mP2PrvLock);
    }

    MY_LOGD1("-");
    return MTRUE;
}
/******************************************************************************
 * Enter scenario control with stereo flag on
 ******************************************************************************/
MERROR
StereoFlowControl::
enterPerformanceScenario(ScenarioMode newScenario)
{
    FUNCTION_SCOPE;
    Mutex::Autolock _l(mScenarioControlLock);

    if(mpScenarioCtrl != 0){
        if(mCurrentScenarioMode == newScenario){
            MY_LOGD("enterPerformanceScenario twice with same scenario, ignored");
            return OK;
        }else{
            MY_LOGD("enterPerformanceScenario newScenario scenario, exitPerformanceScenario first");
            mpScenarioCtrl->exitScenario();
            mpScenarioCtrl = NULL;
        }
    }

    if(newScenario == NONE){
        MY_LOGE("not supproting this scenarioMode:%d", newScenario);
        return OK;
    }

    MUINT featureFlagStereo = 0;
    if(newScenario == ScenarioMode::CAPTURE){
        MY_LOGD("vsdof mode + high perf");
        FEATURE_CFG_ENABLE_MASK(featureFlagStereo, IScenarioControl::FEATURE_STEREO_CAPTURE);
    }else if(newScenario == ScenarioMode::PREVIEW){
        MY_LOGD("vsdof/3rd party preview mode");
        FEATURE_CFG_ENABLE_MASK(featureFlagStereo, IScenarioControl::FEATURE_VSDOF_PREVIEW);
    }
    else{
        MY_LOGD("vsdof/3rd party record mode");
        FEATURE_CFG_ENABLE_MASK(featureFlagStereo, IScenarioControl::FEATURE_VSDOF_RECORD);
    }

    IScenarioControl::ControlParam controlParam;
    controlParam.scenario = getScenario();
    controlParam.sensorSize = mSensorParam.size;
    controlParam.sensorFps = mSensorParam.fps;
    controlParam.featureFlag = featureFlagStereo;
    controlParam.enableBWCControl = MFALSE;
    if(newScenario == ScenarioMode::CAPTURE)
    {
        controlParam.enableDramClkControl = ::supportDramControl();
        controlParam.dramOPPLevel = 0;
    }
    else
    {
        controlParam.enableDramClkControl = false;
    }
    mpScenarioCtrl = IScenarioControl::create(getOpenId());
    if( mpScenarioCtrl == NULL )
    {
        MY_LOGE("get Scenario Control fail");
        return UNKNOWN_ERROR;
    }

    mpScenarioCtrl->enterScenario(controlParam);
    mCurrentScenarioMode = newScenario;

    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
StereoFlowControl::
exitPerformanceScenario()
{
    FUNCTION_SCOPE;
    Mutex::Autolock _l(mScenarioControlLock);

    if(mpScenarioCtrl != 0){
        mpScenarioCtrl->exitScenario();
        mpScenarioCtrl = NULL;
    }else{
        MY_LOGE("Can't get scenario control when exitPerformanceScenario? should not have happened!");
    }
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
StereoFlowControl::
setBufferPoolsReturnMode(MINT32 returnMode)
{
    MY_LOGD("returnMode:%d", returnMode);
    if(mpStereoBufferPool_RESIZER != nullptr){
        mpStereoBufferPool_RESIZER->setReturnMode(returnMode);
    }
    if(mpStereoBufferPool_RESIZER_MAIN2 != nullptr){
        mpStereoBufferPool_RESIZER_MAIN2->setReturnMode(returnMode);
    }
    if(mpStereoBufferPool_OPAQUE != nullptr){
        mpStereoBufferPool_OPAQUE->setReturnMode(returnMode);
    }
    if(mpStereoBufferPool_OPAQUE_MAIN2 != nullptr){
        mpStereoBufferPool_OPAQUE_MAIN2->setReturnMode(returnMode);
    }
    if(mpStereoBufferPool_LCS != nullptr){
        mpStereoBufferPool_LCS->setReturnMode(returnMode);
    }
    if(mpStereoBufferPool_LCS_MAIN2 != nullptr){
        mpStereoBufferPool_LCS_MAIN2->setReturnMode(returnMode);
    }
    return OK;
}

/******************************************************************************
*
*******************************************************************************/
MERROR
StereoFlowControl::
uninitPipelineAndRelatedResource()
{
    SCOPE_TIMER(__t0, "uninit resource");
    while(!mvThreadInitQue.empty())
    {
        if(mvThreadInitQue.front().valid())
        {
            mvThreadInitQue.front().get();
        }
        mvThreadInitQue.pop_front();
    }
    std::vector<std::future<MBOOL> > vThread;
    uninitP2Pipeline();
#define UNINIT_PIPELINE(JOB_NAME, EXECTUE_FUNC_NAME)\
    do{\
        struct JOB_NAME\
        {\
            static\
                MBOOL      execute(StereoFlowControl* pSelf) {\
                                return pSelf->EXECTUE_FUNC_NAME();\
                            }\
        };\
        vThread.push_back(std::async(std::launch::async, &JOB_NAME::execute, this));\
    }while(0);
    UNINIT_PIPELINE(job_releaseP2, releaseP2Resource);
    UNINIT_PIPELINE(job_stopP1, uninitP1Main1Pipeline);
    UNINIT_PIPELINE(job_stopP1Main2, uninitP1Main2Pipeline);
    if(mbCanSupportBackgroundService)
        UNINIT_PIPELINE(job_stopPostProcessor, releasePostProcessor);
#undef UNINIT_PIPELINE
    //
    for(int i = 0 ;i<vThread.size();++i)
    {
        vThread[i].wait();
    }

    if(mpStereoBufferSynchronizer != 0)
    {
        mpStereoBufferSynchronizer->flush();
    }
    if(mpImageStreamManager != nullptr)
    {
        mpImageStreamManager->destroy();
        mpImageStreamManager=nullptr;
    }
    if ( mpPipeline_P1 != 0 ){
        mpPipeline_P1 = NULL;

        mpStereoSelector_RESIZER = NULL;
        mpStereoSelector_OPAQUE = NULL;
    }
    if ( mpPipeline_P1_Main2 != 0 ){
        mpPipeline_P1_Main2 = NULL;

        mpStereoSelector_RESIZER_MAIN2 = NULL;
        mpStereoSelector_OPAQUE_MAIN2 = NULL;

        mpStereoRequestUpdater_P1_Main2 = NULL;
    }

    return OK;
}

/******************************************************************************
*
*******************************************************************************/
MERROR
StereoFlowControl::
doSynchronizerBinding()
{
    FUNCTION_SCOPE;

    Vector<StreamId_T> streamFromMain;
    Vector<StreamId_T> streamFromAux;

    if(mpStereoSelector_RESIZER != nullptr){
        MY_LOGD("bind stream/selector : eSTREAMID_IMAGE_PIPE_RAW_RESIZER");
        sp<StreamBufferProvider> provider = mpResourceContainier->queryConsumer(eSTREAMID_IMAGE_PIPE_RAW_RESIZER);

        if(provider == nullptr){
            MY_LOGE("provider is nullptr!");
            return BAD_VALUE;
        }

        provider->setSelector(mpStereoSelector_RESIZER);
        mpStereoBufferSynchronizer->addStream(eSTREAMID_IMAGE_PIPE_RAW_RESIZER,     mpStereoSelector_RESIZER);
        mpStereoRequestUpdater_P2->addSelector(mpStereoSelector_RESIZER,            StereoRequestUpdater::StreamType::RESIZED);
        mpStereoRequestUpdater_P2->addPool(mpStereoBufferPool_RESIZER,              StereoRequestUpdater::StreamType::RESIZED);
        mpStereoBufferPool_RESIZER->setSynchronizer(mpStereoBufferSynchronizer);

        streamFromMain.push_back(eSTREAMID_IMAGE_PIPE_RAW_RESIZER);
        mProviderStreamIdList.push_back(eSTREAMID_IMAGE_PIPE_RAW_RESIZER);
    }

    if(mpStereoSelector_OPAQUE != nullptr){
        MY_LOGD("bind stream/selector : eSTREAMID_IMAGE_PIPE_RAW_OPAQUE");
        sp<StreamBufferProvider> provider = mpResourceContainier->queryConsumer(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE);

        if(provider == nullptr){
            MY_LOGE("provider is nullptr!");
            return BAD_VALUE;
        }

        provider->setSelector(mpStereoSelector_OPAQUE);
        mpStereoBufferSynchronizer->addStream(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE,      mpStereoSelector_OPAQUE);
        mpStereoRequestUpdater_P2->addSelector(mpStereoSelector_OPAQUE,             StereoRequestUpdater::StreamType::FULL);
        mpStereoRequestUpdater_P2->addPool(mpStereoBufferPool_OPAQUE,               StereoRequestUpdater::StreamType::FULL);
        mpStereoBufferPool_OPAQUE->setSynchronizer(mpStereoBufferSynchronizer);

        streamFromMain.push_back(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE);
        mProviderStreamIdList.push_back(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE);
    }

    if(mpStereoSelector_LCS != nullptr)
    {
        MY_LOGD("bind stream/selector : eSTREAMID_IMAGE_PIPE_RAW_LCSO");
        sp<StreamBufferProvider> provider = mpResourceContainier->queryConsumer(eSTREAMID_IMAGE_PIPE_RAW_LCSO);

        if(provider == nullptr){
            MY_LOGE("provider is nullptr!");
            return BAD_VALUE;
        }
        provider->setSelector(mpStereoSelector_LCS);
        mpStereoBufferSynchronizer->addStream(eSTREAMID_IMAGE_PIPE_RAW_LCSO,        mpStereoSelector_LCS);
        mpStereoRequestUpdater_P2->addSelector(mpStereoSelector_LCS,                StereoRequestUpdater::StreamType::LCS);
        mpStereoRequestUpdater_P2->addPool(mpStereoBufferPool_LCS,                  StereoRequestUpdater::StreamType::LCS);
        mpStereoBufferPool_LCS->setSynchronizer(mpStereoBufferSynchronizer);

        streamFromMain.push_back(eSTREAMID_IMAGE_PIPE_RAW_LCSO);
        mProviderStreamIdList.push_back(eSTREAMID_IMAGE_PIPE_RAW_LCSO);
    }

    if(mpStereoSelector_RESIZER_MAIN2 != nullptr){
        MY_LOGD("bind stream/selector : eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01");
        sp<StreamBufferProvider> provider        = mpResourceContainierMain2->queryConsumer(eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01);

        if(provider == nullptr){
            MY_LOGE("provider is nullptr!");
            return BAD_VALUE;
        }

        provider->setSelector(mpStereoSelector_RESIZER_MAIN2);
        mpStereoBufferSynchronizer->addStream(eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01,      mpStereoSelector_RESIZER_MAIN2);
        mpStereoRequestUpdater_P2->addSelector(mpStereoSelector_RESIZER_MAIN2,          StereoRequestUpdater::StreamType::RESIZED_MAIN2);
        mpStereoRequestUpdater_P2->addPool(mpStereoBufferPool_RESIZER_MAIN2,            StereoRequestUpdater::StreamType::RESIZED_MAIN2);
        mpStereoBufferPool_RESIZER_MAIN2->setSynchronizer(mpStereoBufferSynchronizer);

        streamFromAux.push_back(eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01);
        mProviderStreamIdList.push_back(eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01);
    }

    if(mpStereoSelector_OPAQUE_MAIN2 != nullptr){
        MY_LOGD("bind stream/selector : eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01");
        sp<StreamBufferProvider> provider        = mpResourceContainierMain2->queryConsumer(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01);

        if(provider == nullptr){
            MY_LOGE("provider is nullptr!");
            return BAD_VALUE;
        }

        provider->setSelector(mpStereoSelector_OPAQUE_MAIN2);
        mpStereoBufferSynchronizer->addStream(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01,    mpStereoSelector_OPAQUE_MAIN2);
        mpStereoRequestUpdater_P2->addSelector(mpStereoSelector_OPAQUE_MAIN2,        StereoRequestUpdater::StreamType::FULL_MAIN2);
        mpStereoRequestUpdater_P2->addPool(mpStereoBufferPool_OPAQUE_MAIN2,            StereoRequestUpdater::StreamType::FULL_MAIN2);
        mpStereoBufferPool_OPAQUE_MAIN2->setSynchronizer(mpStereoBufferSynchronizer);

        streamFromAux.push_back(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01);
        mProviderStreamIdList.push_back(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01);
    }

    if(mpStereoSelector_LCS_MAIN2 != nullptr)
    {
        MY_LOGD("bind stream/selector : eSTREAMID_IMAGE_PIPE_RAW_LCSO_01");
        sp<StreamBufferProvider> provider = mpResourceContainierMain2->queryConsumer(eSTREAMID_IMAGE_PIPE_RAW_LCSO_01);

        if(provider == nullptr){
            MY_LOGE("provider is nullptr!");
            return BAD_VALUE;
        }
        provider->setSelector(mpStereoSelector_LCS_MAIN2);
        mpStereoBufferSynchronizer->addStream(eSTREAMID_IMAGE_PIPE_RAW_LCSO_01,        mpStereoSelector_LCS_MAIN2);
        mpStereoRequestUpdater_P2->addSelector(mpStereoSelector_LCS_MAIN2,             StereoRequestUpdater::StreamType::LCS_MAIN2);
        mpStereoRequestUpdater_P2->addPool(mpStereoBufferPool_LCS_MAIN2,               StereoRequestUpdater::StreamType::LCS_MAIN2);
        mpStereoBufferPool_LCS_MAIN2->setSynchronizer(mpStereoBufferSynchronizer);

        streamFromAux.push_back(eSTREAMID_IMAGE_PIPE_RAW_LCSO_01);
        mProviderStreamIdList.push_back(eSTREAMID_IMAGE_PIPE_RAW_LCSO_01);
    }

    mpStereoBufferSynchronizer->setPreviewBufferUser(mpStereoRequestUpdater_P2);
    mpStereoBufferSynchronizer->requestEnable(streamFromMain, streamFromAux);

    return OK;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
StereoFlowControl::
setRequsetTypeForAllPipelines(MINT32 type)
{
    MY_LOGD("new requestType:%d", type);

    if( mpRequestController_P2 != 0 ){
        mpRequestController_P2->setRequestType(type);
    }

    if( mpRequestController_P1 != 0 ){
        mpRequestController_P1->setRequestType(type);
    }

    if( mpRequestController_P1_Main2 != 0 ){
        mpRequestController_P1_Main2->setRequestType(type);
    }
    return OK;
}

/******************************************************************************
*
 ******************************************************************************/
sp<ImageStreamInfo>
StereoFlowControl::
createImageStreamInfo(
    char const*         streamName,
    StreamId_T          streamId,
    MUINT32             streamType,
    size_t              maxBufNum,
    size_t              minInitBufNum,
    MUINT               usageForAllocator,
    MINT                imgFormat,
    MSize const&        imgSize,
    MUINT32             transform
)
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
                addBufPlane(bufPlanes , imgSize.h      , imgSize.w);
                break;
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
            MY_LOGE("create ImageStream failed, %s, %#" PRIx64,
                    streamName, streamId);
        }

        return pStreamInfo;
}

/******************************************************************************
 *
 ******************************************************************************/
MINT32
StereoFlowControl::
getSensorModuleType(
)
{
    MINT32 value = 0;
    // get main1 sensor format
    MUINT sensorRawFmtType_Main1 = StereoSettingProvider::getSensorRawFormat(getOpenId());
    // get main2 sensor format
    MUINT sensorRawFmtType_Main2 = StereoSettingProvider::getSensorRawFormat(getOpenId_Main2());
    if(sensorRawFmtType_Main2 == SENSOR_RAW_MONO)
    {
        MY_LOGD("Sensor module is B+M");
        value = Stereo::BAYER_AND_MONO;
    }
    else
    {
        MY_LOGD("Sensor module is B+B");
        value = Stereo::BAYER_AND_BAYER;
    }
    return value;
}

/******************************************************************************
 *
 ******************************************************************************/
MINT32
StereoFlowControl::
getStereoMode(
)
{
    MINT32 value = 0;
    if(::strcmp(mpParamsManagerV3->getParamsMgr()->getStr(
            MtkCameraParameters::KEY_STEREO_REFOCUS_MODE), MtkCameraParameters::ON) == 0)
    {
        MY_LOGD("enable stereo capture");
        value |= Stereo::E_STEREO_FEATURE_CAPTURE;
    }
    if(::strcmp(mpParamsManagerV3->getParamsMgr()->getStr(
            MtkCameraParameters::KEY_STEREO_VSDOF_MODE), MtkCameraParameters::ON) == 0)
    {
        MY_LOGD("enable vsdof");
        value |= Stereo::E_STEREO_FEATURE_VSDOF;
    }
    if(::strcmp(mpParamsManagerV3->getParamsMgr()->getStr(
            MtkCameraParameters::KEY_STEREO_3RDPARTY_MODE), MtkCameraParameters::ON) == 0)
    {
        MY_LOGD("enable 3party");
        value |= Stereo::E_STEREO_FEATURE_THIRD_PARTY;
    }
    MY_LOGD("Stereo mode(%d)", value);
    return value;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
StereoFlowControl::
buildStereoPipeline(
    MINT32 stereoMode,
    MINT32 sensorModuleType,
    MINT32 pipelineMode
)
{
    SCOPE_TIMER(__t0, "buildStereoPipeline");
    status_t ret = UNKNOWN_ERROR;
    if(sensorModuleType == Stereo::UNSUPPORTED_SENSOR_MODULE)
    {
        MY_LOGE("invalid operation.");
        goto lbExit;
    }
    if(mPipelineMode == PipelineMode_RECORDING)
    {
        // main1
        mpStereoSelector_RESIZER            = new StereoSelector("RESIZER");
        mpStereoSelector_LCS                = new StereoSelector("LCS");
        // main2
        if(mbMain2Full)
        {
            mpStereoSelector_OPAQUE_MAIN2 = new StereoSelector("OPAQUE_01");
            mpStereoSelector_LCS_MAIN2    = new StereoSelector("LCS_01");
        }
        else
        {
            mpStereoSelector_RESIZER_MAIN2 = new StereoSelector("RESIZER_01");
        }
    }
    else
    {
        // main1
        mpStereoSelector_RESIZER            = new StereoSelector("RESIZER");
        mpStereoSelector_OPAQUE             = new StereoSelector("OPAQUE");
        mpStereoSelector_LCS                = new StereoSelector("LCS");
        // main2
        mpStereoSelector_RESIZER_MAIN2 = new StereoSelector("RESIZER_01");

        if(mbMain2Full)
        {
            mpStereoSelector_OPAQUE_MAIN2 = new StereoSelector("OPAQUE_01");
            mpStereoSelector_LCS_MAIN2    = new StereoSelector("LCS_01");
        }
    }

    StereoSettingProvider::getSensorScenario(stereoMode,
                                             sensorModuleType,
                                             pipelineMode,
                                             mSensorParam.mode,
                                             mSensorParam_main2.mode);
    StereoSettingProvider::updateSensorScenario(mSensorParam.mode,
                                                mSensorParam_main2.mode);

    //
    if(Stereo::STEREO_3RDPARTY == (stereoMode) &&
       pipelineMode == PipelineMode_ZSD)
    {
        MY_LOGD("STEREO_3RDPARTY: get 3rd party prv_cap table");
        ret = get_3rdParty_Content(mCurrentPipelineContent);
    }
    else if(Stereo::STEREO_TK_DEPTH_3RD_PRV == (stereoMode) &&
       pipelineMode == PipelineMode_ZSD)
    {
        MY_LOGD("STEREO_TK_DEPTH_3RD_PRV (temp use pure 3rd flow)");
        ret = get_Tk_Depth_3rd_Cap_Content(mCurrentPipelineContent);
    }
    else if(Stereo::STEREO_BB_PRV_CAP_REC == (stereoMode|sensorModuleType) &&
       pipelineMode == PipelineMode_ZSD)
    {
        if(mCaptureModeType == 0)
        {
            MY_LOGD("get bb prv_cap table");
            ret = get_BB_Prv_Cap_Content(mCurrentPipelineContent);
        }
        else if(mCaptureModeType == 1)
        {
            MY_LOGD("get bb tk prv 3rd cap table");
            ret = get_Tk_Prv_3rd_Cap_Content(mCurrentPipelineContent);
        }
    }
    else if(Stereo::STEREO_BB_PRV_CAP_REC == (stereoMode|sensorModuleType) &&
       pipelineMode == PipelineMode_RECORDING)
    {
        MY_LOGD("get bb rec table");
        ret = get_BB_Rec_Content(mCurrentPipelineContent);
    }
    else if(Stereo::STEREO_BM_PRV_CAP_REC == (stereoMode|sensorModuleType) &&
       pipelineMode == PipelineMode_ZSD)
    {
        MY_LOGD("get bm prv_cap table");
        ret = get_BM_VSDOF_Content(mCurrentPipelineContent);
    }
    else if(Stereo::STEREO_BM_PRV_CAP_REC == (stereoMode|sensorModuleType) &&
       pipelineMode == PipelineMode_RECORDING)
    {
        MY_LOGD("get bm rec table");
        ret = get_BM_VSDOF_Rec_Content(mCurrentPipelineContent);
    }
    else
    {
        MY_LOGE("should not happened!");
        goto lbExit;
    }
    // after get table finish, init flow
    mvThreadInitQue.push_back(
        std::async(
            std::launch::async,
            [this]()
            {
                buildStereoP1Pipeline_Main1();
            }
        )
    );
    mvThreadInitQue.push_back(
        std::async(
            std::launch::async,
            [this]()
            {
                buildStereoP1Pipeline_Main2();
            }
        )
    );
    while(!mvThreadInitQue.empty())
    {
        if(mvThreadInitQue.front().valid())
        {
            mvThreadInitQue.front().get();
        }
        mvThreadInitQue.pop_front();
    }
    buildStereoP2Pipeline();

lbExit:
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
StereoFlowControl::
get_BB_Prv_Cap_Content(
    StereoModeContextBuilderContent &content
)
{
    // pass1 main1 content
    content.mP1Main1Content.metaTable = StereoPipelineMgrData::p1::zsd::gStereoMetaTbl_ZsdP1;
    content.mP1Main1Content.nodeConfigData = StereoPipelineMgrData::p1::zsd::gStereoConnectSetting_ZsdP1;
    content.mP1Main1Content.imageTable = StereoPipelineMgrData::p1::zsd::gStereoImgStreamTbl_ZsdP1;

    // pass1 main2 content
    content.mP1Main2Content.metaTable = StereoPipelineMgrData::p1::zsd::gStereoMetaTbl_ZsdP1Main2;
    content.mP1Main2Content.nodeConfigData = StereoPipelineMgrData::p1::zsd::gStereoConnectSetting_ZsdP1Main2;
    if(mbMain2Full)
    {
        content.mP1Main2Content.imageTable = StereoPipelineMgrData::p1::zsd::gStereoImgStreamTbl_ZsdP1Main2_BOTH;
    }
    else
    {
        content.mP1Main2Content.imageTable = StereoPipelineMgrData::p1::zsd::gStereoImgStreamTbl_ZsdP1Main2_RRZO;
    }

    // pass2 contents
#define SET_P2_TBL(prvcap_name, prv_name, cap_name, dcap_name) \
    do{\
        content.mFullContent.metaTable = StereoPipelineMgrData::p2::prvcap_name::gStereoMetaTbl_P2PrvCap;\
        content.mFullContent.nodeConfigData = StereoPipelineMgrData::p2::prvcap_name::gStereoP2ConnectSetting_P2PrvCap;\
        if(mbMain2Full)\
        {\
            content.mFullContent.imageTable = StereoPipelineMgrData::p2::prvcap_name::gStereoImgStreamTbl_P2PrvCap_Full;\
        }\
        else\
        {\
            content.mFullContent.imageTable = StereoPipelineMgrData::p2::prvcap_name::gStereoImgStreamTbl_P2PrvCap;\
        }\
        content.mPrvContent.metaTable = StereoPipelineMgrData::p2::prv_name::gStereoMetaTbl_P2Prv;\
        content.mPrvContent.nodeConfigData = StereoPipelineMgrData::p2::prv_name::gStereoP2ConnectSetting_P2Prv;\
        if(mbMain2Full)\
        {\
            content.mPrvContent.imageTable = StereoPipelineMgrData::p2::prv_name::gStereoImgStreamTbl_P2Prv_Main2_Full;\
        }\
        else\
        {\
            content.mPrvContent.imageTable = StereoPipelineMgrData::p2::prv_name::gStereoImgStreamTbl_P2Prv_Main2;\
        }\
        content.mCapContent.metaTable = StereoPipelineMgrData::p2::cap_name::gStereoMetaTbl_P2Cap;\
        content.mCapContent.nodeConfigData = StereoPipelineMgrData::p2::cap_name::gStereoP2ConnectSetting_P2Cap;\
        if(mbMain2Full)\
        {\
            content.mCapContent.imageTable = StereoPipelineMgrData::p2::cap_name::gStereoImgStreamTbl_P2Cap_Main2_Full;\
        }\
        else\
        {\
            content.mCapContent.imageTable = StereoPipelineMgrData::p2::cap_name::gStereoImgStreamTbl_P2Cap_Main2;\
        }\
        content.mDngCapContent.metaTable = StereoPipelineMgrData::p2::dcap_name::gStereoMetaTbl_P2DNGCap;\
        content.mDngCapContent.nodeConfigData = StereoPipelineMgrData::p2::dcap_name::gStereoP2ConnectSetting_P2DNGCap;\
        if(mbMain2Full)\
        {\
            content.mCapContent.imageTable = StereoPipelineMgrData::p2::dcap_name::gStereoImgStreamTbl_P2DNGCap_Full;\
        }\
        else\
        {\
            content.mCapContent.imageTable = StereoPipelineMgrData::p2::dcap_name::gStereoImgStreamTbl_P2DNGCap;\
        }\
    }while(0);
    if(mbCanSupportBackgroundService)
    {
        SET_P2_TBL(BM_VSDOF_V2_PREVIEW_CAPTURE, BM_VSDOF_V2_PREVIEW, BM_VSDOF_V2_CAPTURE, BM_VSDOF_V2_DNG_CAPTURE);
    }
    else
    {
        SET_P2_TBL(PrvCap, prv, cap, dngCap);
    }
#undef SET_P2_TBL
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
StereoFlowControl::
get_BB_Rec_Content(
    StereoModeContextBuilderContent &content
)
{
    content.mP1Main1Content.metaTable = StereoPipelineMgrData::p1::VdoRec::gStereoMetaTbl_VdoP1;
    content.mP1Main1Content.nodeConfigData = StereoPipelineMgrData::p1::VdoRec::gStereoConnectSetting_VdoP1;
    content.mP1Main1Content.imageTable = StereoPipelineMgrData::p1::VdoRec::gStereoImgStreamTbl_VdoP1;

    content.mP1Main2Content.metaTable = StereoPipelineMgrData::p1::VdoRec::gStereoMetaTbl_VdoP1Main2;
    content.mP1Main2Content.nodeConfigData = StereoPipelineMgrData::p1::VdoRec::gStereoConnectSetting_VdoP1Main2;
    content.mP1Main2Content.imageTable = StereoPipelineMgrData::p1::VdoRec::gStereoImgStreamTbl_VdoP1Main2_RRZO;

    content.mFullContent.metaTable = StereoPipelineMgrData::p2::VdoRec::gStereoMetaTbl_P2VdoRec;
    content.mFullContent.nodeConfigData = StereoPipelineMgrData::p2::VdoRec::gStereoP2ConnectSetting_P2VdoRec;
    content.mFullContent.imageTable = StereoPipelineMgrData::p2::VdoRec::gStereoImgStreamTbl_P2VdoRec;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
StereoFlowControl::
get_BM_VSDOF_Content(
    StereoModeContextBuilderContent &content
)
{
    // pass1 main1 content
    content.mP1Main1Content.metaTable = StereoPipelineMgrData::p1::BM_VSDOF_ZSD::gStereoMetaTbl_ZsdP1;
    content.mP1Main1Content.nodeConfigData = StereoPipelineMgrData::p1::BM_VSDOF_ZSD::gStereoConnectSetting_ZsdP1;
    content.mP1Main1Content.imageTable = StereoPipelineMgrData::p1::BM_VSDOF_ZSD::gStereoImgStreamTbl_ZsdP1;

    // pass1 main2 content
    content.mP1Main2Content.metaTable = StereoPipelineMgrData::p1::BM_VSDOF_ZSD::gStereoMetaTbl_ZsdP1Main2;
    content.mP1Main2Content.nodeConfigData = StereoPipelineMgrData::p1::BM_VSDOF_ZSD::gStereoConnectSetting_ZsdP1Main2;
    content.mP1Main2Content.imageTable = StereoPipelineMgrData::p1::BM_VSDOF_ZSD::gStereoImgStreamTbl_ZsdP1Main2_RRZO;

    // pass2 contents
#define SET_P2_TBL(prvcap_name, prv_name, cap_name, dcap_name) \
    do{\
        content.mFullContent.metaTable = StereoPipelineMgrData::p2::prvcap_name::gStereoMetaTbl_P2PrvCap;\
        content.mFullContent.nodeConfigData = StereoPipelineMgrData::p2::prvcap_name::gStereoP2ConnectSetting_P2PrvCap;\
        content.mFullContent.imageTable = StereoPipelineMgrData::p2::prvcap_name::gStereoImgStreamTbl_P2PrvCap;\
        content.mPrvContent.metaTable = StereoPipelineMgrData::p2::prv_name::gStereoMetaTbl_P2Prv;\
        content.mPrvContent.nodeConfigData = StereoPipelineMgrData::p2::prv_name::gStereoP2ConnectSetting_P2Prv;\
        content.mPrvContent.imageTable = StereoPipelineMgrData::p2::prv_name::gStereoImgStreamTbl_P2Prv_Main2;\
        content.mCapContent.metaTable = StereoPipelineMgrData::p2::cap_name::gStereoMetaTbl_P2Cap;\
        content.mCapContent.nodeConfigData = StereoPipelineMgrData::p2::cap_name::gStereoP2ConnectSetting_P2Cap;\
        content.mCapContent.imageTable = StereoPipelineMgrData::p2::cap_name::gStereoImgStreamTbl_P2Cap_Main2;\
        content.mDngCapContent.metaTable = StereoPipelineMgrData::p2::dcap_name::gStereoMetaTbl_P2DNGCap;\
        content.mDngCapContent.nodeConfigData = StereoPipelineMgrData::p2::dcap_name::gStereoP2ConnectSetting_P2DNGCap;\
        content.mDngCapContent.imageTable = StereoPipelineMgrData::p2::dcap_name::gStereoImgStreamTbl_P2DNGCap;\
    }while(0);
    if(mbCanSupportBackgroundService)
    {
        SET_P2_TBL(BM_VSDOF_V2_PREVIEW_CAPTURE, BM_VSDOF_V2_PREVIEW, BM_VSDOF_V2_CAPTURE, BM_VSDOF_V2_DNG_CAPTURE);
    }
    else
    {
        SET_P2_TBL(BM_VSDOF_PREVIEW_CAPTURE, BM_VSDOF_PREVIEW, BM_VSDOF_CAPTURE, BM_VSDOF_DNG_CAPTURE);
    }
#undef SET_P2_TBL
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
StereoFlowControl::
get_BM_VSDOF_Rec_Content(
    StereoModeContextBuilderContent &content
)
{
    content.mP1Main1Content.metaTable = StereoPipelineMgrData::p1::BM_VSDOF_RECORD::gStereoMetaTbl_VdoP1;
    content.mP1Main1Content.nodeConfigData = StereoPipelineMgrData::p1::BM_VSDOF_RECORD::gStereoConnectSetting_VdoP1;
    content.mP1Main1Content.imageTable = StereoPipelineMgrData::p1::BM_VSDOF_RECORD::gStereoImgStreamTbl_VdoP1;

    content.mP1Main2Content.metaTable = StereoPipelineMgrData::p1::BM_VSDOF_RECORD::gStereoMetaTbl_VdoP1Main2;
    content.mP1Main2Content.nodeConfigData = StereoPipelineMgrData::p1::BM_VSDOF_RECORD::gStereoConnectSetting_VdoP1Main2;
    content.mP1Main2Content.imageTable = StereoPipelineMgrData::p1::BM_VSDOF_RECORD::gStereoImgStreamTbl_VdoP1Main2_RRZO;

    content.mFullContent.metaTable = StereoPipelineMgrData::p2::BM_VSDOF_RECORD::gStereoMetaTbl_P2VdoRec;
    content.mFullContent.nodeConfigData = StereoPipelineMgrData::p2::BM_VSDOF_RECORD::gStereoP2ConnectSetting_P2VdoRec;
    content.mFullContent.imageTable = StereoPipelineMgrData::p2::BM_VSDOF_RECORD::gStereoImgStreamTbl_P2VdoRec;
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
StereoFlowControl::
get_3rdParty_Content(
    StereoModeContextBuilderContent &content
)
{
    // pass1 main1 content
    content.mP1Main1Content.metaTable = StereoPipelineMgrData::p1::ZSD_3rdParty_Async::gStereoMetaTbl_ZsdP1;
    content.mP1Main1Content.nodeConfigData = StereoPipelineMgrData::p1::ZSD_3rdParty_Async::gStereoConnectSetting_ZsdP1;
    content.mP1Main1Content.imageTable = StereoPipelineMgrData::p1::ZSD_3rdParty_Async::gStereoImgStreamTbl_ZsdP1;

    // pass1 main2 content
    content.mP1Main2Content.metaTable = StereoPipelineMgrData::p1::ZSD_3rdParty_Async::gStereoMetaTbl_ZsdP1Main2;
    content.mP1Main2Content.nodeConfigData = StereoPipelineMgrData::p1::ZSD_3rdParty_Async::gStereoConnectSetting_ZsdP1Main2;
    if(mbMain2Full)
        content.mP1Main2Content.imageTable = StereoPipelineMgrData::p1::ZSD_3rdParty_Async::gStereoImgStreamTbl_ZsdP1Main2_BOTH;
    else
        content.mP1Main2Content.imageTable = StereoPipelineMgrData::p1::ZSD_3rdParty_Async::gStereoImgStreamTbl_ZsdP1Main2_RRZO;

    // pass2 contents
    content.mFullContent.metaTable = StereoPipelineMgrData::p2::PrvCap_3rdParty_Async::gStereoMetaTbl_P2PrvCap;
    content.mFullContent.nodeConfigData = StereoPipelineMgrData::p2::PrvCap_3rdParty_Async::gStereoP2ConnectSetting_P2PrvCap;
    if(mbMain2Full)
        content.mFullContent.imageTable = StereoPipelineMgrData::p2::PrvCap_3rdParty_Async::gStereoImgStreamTbl_P2PrvCap;
    else
        content.mFullContent.imageTable = StereoPipelineMgrData::p2::PrvCap_3rdParty_Async::gStereoImgStreamTbl_P2PrvCap_wo_Main2_Full;

    content.mPrvContent.metaTable = StereoPipelineMgrData::p2::Prv_3rdParty_Async::gStereoMetaTbl_P2Prv;
    content.mPrvContent.nodeConfigData = StereoPipelineMgrData::p2::Prv_3rdParty_Async::gStereoP2ConnectSetting_P2Prv;
    if(mbMain2Full)
        content.mPrvContent.imageTable = StereoPipelineMgrData::p2::Prv_3rdParty_Async::gStereoImgStreamTbl_P2Prv;
    else
        content.mPrvContent.imageTable = StereoPipelineMgrData::p2::Prv_3rdParty_Async::gStereoImgStreamTbl_P2Prv_wo_Main2_Full;

    content.mCapContent.metaTable = StereoPipelineMgrData::p2::Cap_3rdParty_Async::gStereoMetaTbl_P2Cap;
    content.mCapContent.nodeConfigData = StereoPipelineMgrData::p2::Cap_3rdParty_Async::gStereoP2ConnectSetting_P2Cap;
    if(mbMain2Full)
        content.mCapContent.imageTable = StereoPipelineMgrData::p2::Cap_3rdParty_Async::gStereoImgStreamTbl_P2Cap;
    else
        content.mCapContent.imageTable = StereoPipelineMgrData::p2::Cap_3rdParty_Async::gStereoImgStreamTbl_P2Cap_wo_Main2_Full;

    content.mDngCapContent.metaTable = StereoPipelineMgrData::p2::DngCap_3rdParty_Async::gStereoMetaTbl_P2DNGCap;
    content.mDngCapContent.nodeConfigData = StereoPipelineMgrData::p2::DngCap_3rdParty_Async::gStereoP2ConnectSetting_P2DNGCap;
    if(mbMain2Full)
        content.mDngCapContent.imageTable = StereoPipelineMgrData::p2::DngCap_3rdParty_Async::gStereoImgStreamTbl_P2DNGCap;
    else
        content.mDngCapContent.imageTable = StereoPipelineMgrData::p2::DngCap_3rdParty_Async::gStereoImgStreamTbl_P2DNGCap_wo_Main2_Full;
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
StereoFlowControl::
get_Tk_Prv_3rd_Cap_Content(
    StereoModeContextBuilderContent &content
)
{
    // pass1 main1 content
    content.mP1Main1Content.metaTable = StereoPipelineMgrData::p1::zsd::gStereoMetaTbl_ZsdP1;
    content.mP1Main1Content.nodeConfigData = StereoPipelineMgrData::p1::zsd::gStereoConnectSetting_ZsdP1;
    content.mP1Main1Content.imageTable = StereoPipelineMgrData::p1::zsd::gStereoImgStreamTbl_ZsdP1;

    // pass1 main2 content
    content.mP1Main2Content.metaTable = StereoPipelineMgrData::p1::zsd::gStereoMetaTbl_ZsdP1Main2;
    content.mP1Main2Content.nodeConfigData = StereoPipelineMgrData::p1::zsd::gStereoConnectSetting_ZsdP1Main2;
    if(mbMain2Full)
    {
        content.mP1Main2Content.imageTable = StereoPipelineMgrData::p1::zsd::gStereoImgStreamTbl_ZsdP1Main2_BOTH;
    }
    else
    {
        content.mP1Main2Content.imageTable = StereoPipelineMgrData::p1::zsd::gStereoImgStreamTbl_ZsdP1Main2_RRZO;
    }
    // p2
    content.mFullContent.metaTable = StereoPipelineMgrData::p2_vsdof_3rd::PrvCap::gStereoMetaTbl_P2PrvCap;
    content.mFullContent.nodeConfigData = StereoPipelineMgrData::p2_vsdof_3rd::PrvCap::gStereoP2ConnectSetting_P2PrvCap;

    content.mPrvContent.metaTable = StereoPipelineMgrData::p2_vsdof_3rd::prv::gStereoMetaTbl_P2Prv;
    content.mPrvContent.nodeConfigData = StereoPipelineMgrData::p2_vsdof_3rd::prv::gStereoP2ConnectSetting_P2Prv;

    content.mCapContent.metaTable = StereoPipelineMgrData::p2_vsdof_3rd::cap::gStereoMetaTbl_P2Cap;
    content.mCapContent.nodeConfigData = StereoPipelineMgrData::p2_vsdof_3rd::cap::gStereoP2ConnectSetting_P2Cap;

    content.mDngCapContent.metaTable = StereoPipelineMgrData::p2_vsdof_3rd::dngCap::gStereoMetaTbl_P2DNGCap;
    content.mDngCapContent.nodeConfigData = StereoPipelineMgrData::p2_vsdof_3rd::dngCap::gStereoP2ConnectSetting_P2DNGCap;

    if(mbMain2Full)
    {
        content.mFullContent.imageTable = StereoPipelineMgrData::p2_vsdof_3rd::PrvCap::gStereoImgStreamTbl_P2PrvCap_Full;
        content.mPrvContent.imageTable = StereoPipelineMgrData::p2_vsdof_3rd::prv::gStereoImgStreamTbl_P2Prv_Main2_Full;
        content.mCapContent.imageTable = StereoPipelineMgrData::p2_vsdof_3rd::cap::gStereoImgStreamTbl_P2Cap_Main2_Full;
        content.mCapContent.imageTable = StereoPipelineMgrData::p2_vsdof_3rd::dngCap::gStereoImgStreamTbl_P2DNGCap_Full;
    }
    else
    {
        content.mFullContent.imageTable = StereoPipelineMgrData::p2_vsdof_3rd::PrvCap::gStereoImgStreamTbl_P2PrvCap;
        content.mPrvContent.imageTable = StereoPipelineMgrData::p2_vsdof_3rd::prv::gStereoImgStreamTbl_P2Prv_Main2;
        content.mCapContent.imageTable = StereoPipelineMgrData::p2_vsdof_3rd::cap::gStereoImgStreamTbl_P2Cap_Main2;
        content.mCapContent.imageTable = StereoPipelineMgrData::p2_vsdof_3rd::dngCap::gStereoImgStreamTbl_P2DNGCap;
    }
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
StereoFlowControl::
get_Tk_Depth_3rd_Cap_Content(
    StereoModeContextBuilderContent &content
)
{
    content.mFullContent.nodeConfigData = StereoPipelineMgrData::p2_vsdof_3rd::PrvCap::gStereoP2ConnectSetting_P2PrvCap_Vendor;
    // pass1 main1 content
    content.mP1Main1Content.metaTable = StereoPipelineMgrData::p1::zsd::gStereoMetaTbl_ZsdP1;
    content.mP1Main1Content.nodeConfigData = StereoPipelineMgrData::p1::zsd::gStereoConnectSetting_ZsdP1;
    content.mP1Main1Content.imageTable = StereoPipelineMgrData::p1::zsd::gStereoImgStreamTbl_ZsdP1;

    // pass1 main2 content
    content.mP1Main2Content.metaTable = StereoPipelineMgrData::p1::zsd::gStereoMetaTbl_ZsdP1Main2;
    content.mP1Main2Content.nodeConfigData = StereoPipelineMgrData::p1::zsd::gStereoConnectSetting_ZsdP1Main2;
    if(mbMain2Full)
    {
        content.mP1Main2Content.imageTable = StereoPipelineMgrData::p1::zsd::gStereoImgStreamTbl_ZsdP1Main2_BOTH;
    }
    else
    {
        content.mP1Main2Content.imageTable = StereoPipelineMgrData::p1::zsd::gStereoImgStreamTbl_ZsdP1Main2_RRZO;
    }
    // p2
    content.mFullContent.metaTable = StereoPipelineMgrData::p2_vsdof_3rd::PrvCap::gStereoMetaTbl_P2PrvCap;
    content.mFullContent.nodeConfigData = StereoPipelineMgrData::p2_vsdof_3rd::PrvCap::gStereoP2ConnectSetting_P2PrvCap_Vendor;

    content.mPrvContent.metaTable = StereoPipelineMgrData::p2_vsdof_3rd::prv::gStereoMetaTbl_P2Prv;
    content.mPrvContent.nodeConfigData = StereoPipelineMgrData::p2_vsdof_3rd::prv::gStereoP2ConnectSetting_P2Prv_Vendor;

    content.mCapContent.metaTable = StereoPipelineMgrData::p2_vsdof_3rd::cap::gStereoMetaTbl_P2Cap;
    content.mCapContent.nodeConfigData = StereoPipelineMgrData::p2_vsdof_3rd::cap::gStereoP2ConnectSetting_P2Cap;

    content.mDngCapContent.metaTable = StereoPipelineMgrData::p2_vsdof_3rd::dngCap::gStereoMetaTbl_P2DNGCap;
    content.mDngCapContent.nodeConfigData = StereoPipelineMgrData::p2_vsdof_3rd::dngCap::gStereoP2ConnectSetting_P2DNGCap;

    if(mbMain2Full)
    {
        content.mFullContent.imageTable = StereoPipelineMgrData::p2_vsdof_3rd::PrvCap::gStereoImgStreamTbl_P2PrvCap_Full_TkDep;
        content.mPrvContent.imageTable = StereoPipelineMgrData::p2_vsdof_3rd::prv::gStereoImgStreamTbl_P2Prv_Main2_Full_TkDep;
        content.mCapContent.imageTable = StereoPipelineMgrData::p2_vsdof_3rd::cap::gStereoImgStreamTbl_P2Cap_Main2_Full;
        content.mCapContent.imageTable = StereoPipelineMgrData::p2_vsdof_3rd::dngCap::gStereoImgStreamTbl_P2DNGCap_Full;
    }
    else
    {
        content.mFullContent.imageTable = StereoPipelineMgrData::p2_vsdof_3rd::PrvCap::gStereoImgStreamTbl_P2PrvCap_TkDep;
        content.mPrvContent.imageTable = StereoPipelineMgrData::p2_vsdof_3rd::prv::gStereoImgStreamTbl_P2Prv_Main2_TkDep;
        content.mCapContent.imageTable = StereoPipelineMgrData::p2_vsdof_3rd::cap::gStereoImgStreamTbl_P2Cap_Main2;
        content.mCapContent.imageTable = StereoPipelineMgrData::p2_vsdof_3rd::dngCap::gStereoImgStreamTbl_P2DNGCap;
    }
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
StereoFlowControl::
queryPass1ActiveArrayCrop()
{
    if(!StereoSizeProvider::getInstance()->getPass1ActiveArrayCrop(StereoHAL::eSTEREO_SENSOR_MAIN1, mActiveArrayCrop)){
        MY_LOGE("can't get active arrayCrop from StereoSizeProvider for eSTEREO_SENSOR_MAIN1");
        return BAD_VALUE;
    }
    if(!StereoSizeProvider::getInstance()->getPass1ActiveArrayCrop(StereoHAL::eSTEREO_SENSOR_MAIN2, mActiveArrayCrop_Main2)){
        MY_LOGE("can't get active arrayCrop from StereoSizeProvider for eSTEREO_SENSOR_MAIN2");
        return BAD_VALUE;
    }

    MY_LOGD("StereoSizeProvider => active array crop main1(%d,%d,%dx%d), main2(%d,%d,%dx%d)",
        mActiveArrayCrop.p.x,
        mActiveArrayCrop.p.y,
        mActiveArrayCrop.s.w,
        mActiveArrayCrop.s.h,
        mActiveArrayCrop_Main2.p.x,
        mActiveArrayCrop_Main2.p.y,
        mActiveArrayCrop_Main2.s.w,
        mActiveArrayCrop_Main2.s.h
    );
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
StereoFlowControl::
createStereoRequestController()
{
    FUNCTION_SCOPE;
    status_t ret = UNKNOWN_ERROR;
    mpRequestController_P1 = IRequestController::createInstance(
                                                "mpRequestController_P1",
                                                getOpenId(),
                                                mpCamMsgCbInfo,
                                                mpParamsManagerV3
                                            );
    if(mpRequestController_P1 == nullptr)
    {
        MY_LOGE("mpRequestController_P1 create fail.");
        goto lbExit;
    }
    mpRequestController_P1_Main2 = IRequestController::createInstance(
                                            "mpRequestController_P1_Main2",
                                            getOpenId_Main2(),
                                            mpCamMsgCbInfo,
                                            mpParamsManagerV3
                                        );
    if(mpRequestController_P1_Main2 == nullptr)
    {
        MY_LOGE("mpRequestController_P1_Main2 create fail.");
        goto lbExit;
    }
    mpRequestController_P2 = IRequestController::createInstance(
                                            "mpRequestController",
                                            getOpenId(), // dont use getOpenId_P2Prv() or RequestSettingBuilder will go wrong
                                            mpCamMsgCbInfo,
                                            mpParamsManagerV3
                                        );
    if(mpRequestController_P2 == nullptr)
    {
        MY_LOGE("mpRequestController_P2 create fail.");
        goto lbExit;
    }
    MY_LOGD("mpRequestController_P1=%p", mpRequestController_P1.get());
    MY_LOGD("mpRequestController_P1_Main2=%p", mpRequestController_P1_Main2.get());
    MY_LOGD("mpRequestController_P2=%p", mpRequestController_P2.get());

    ret = OK;
lbExit:
    if(ret != OK)
    {
    mpRequestController_P1 = nullptr;
    mpRequestController_P1_Main2 = nullptr;
    mpRequestController_P2 = nullptr;
    }
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
StereoFlowControl::
setAndStartStereoSynchronizer()
{
    FUNCTION_SCOPE;

    if(getPipelineMode() == PipelineMode_ZSD)
    {
        mpStereoBufferSynchronizer = StereoBufferSynchronizer::createInstance("Synchronizer", get_stereo_zsd_cap_stored_frame_cnt());
    }
    else
    {
        mpStereoBufferSynchronizer = StereoBufferSynchronizer::createInstance("Synchronizer", 0);
    }

    // create stereoRequestUpdater
    MY_LOGD("create stereoRequestUpdater");
    mpStereoRequestUpdater_P2 = new StereoRequestUpdater(this, mpStereoBufferSynchronizer, mpParamsManagerV3);
    mpStereoRequestUpdater_P1_Main2 = new StereoMain2RequestUpdater(this, mpStereoBufferSynchronizer, mpParamsManagerV3);

    mpStereoRequestUpdater_P2->setSensorParams(mSensorParam, mSensorParam_main2);
    if(Stereo::STEREO_3RDPARTY == mCurrentStereoMode && getPipelineMode() == PipelineMode_ZSD)
    {
        mpStereoRequestUpdater_P2->setPreviewStreamId(eSTREAMID_IMAGE_PIPE_YUV_00);
        mpStereoRequestUpdater_P2->setFdStreamId(eSTREAMID_IMAGE_YUV_FD);
        mProviderStreamIdList.push_back(eSTREAMID_IMAGE_PIPE_YUV_00);
        mProviderStreamIdList.push_back(eSTREAMID_IMAGE_YUV_FD);
    }
    else
    {
        mpStereoRequestUpdater_P2->setPreviewStreamId(eSTREAMID_IMAGE_PIPE_BOKEHNODE_PREVIEWYUV);
        mProviderStreamIdList.push_back(eSTREAMID_IMAGE_PIPE_BOKEHNODE_PREVIEWYUV);
        // FD just work in preview mode
        if(getPipelineMode() == PipelineMode_ZSD)
        {
        mpStereoRequestUpdater_P2->setFdStreamId(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_FDYUV);
            mProviderStreamIdList.push_back(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_FDYUV);
        }
        // if pipeline is in record mode, add record stream
        if(getPipelineMode() == PipelineMode_RECORDING)
        {
            mProviderStreamIdList.push_back(eSTREAMID_IMAGE_PIPE_BOKEHNODE_RECORDYUV);
        }
    }
    mpStereoRequestUpdater_P1_Main2->setSensorParams(mSensorParam, mSensorParam_main2);

    // bind stream/selector pair to synchronizer
    if(doSynchronizerBinding() != OK){
        MY_LOGE("Cannot doSynchronizerBinding. start preview fail.");
        return BAD_VALUE;
    }

    // set stereo buffer pool return mode
    setBufferPoolsReturnMode(StereoBufferPool::RETURN_MODE::TO_SYNCHRONIZER);

    // need to add preview callback
    mProviderStreamIdList.push_back(eSTREAMID_IMAGE_PIPE_YUV_01);
    // set provider stream list to p2 flowcontrol
    mpStereoRequestUpdater_P2->setProviderStreamList(mProviderStreamIdList);
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
StereoFlowControl::
startStereoPipeline(
    MINT32 request_num_start,
    MINT32 request_num_end
)
{
    FUNCTION_SCOPE;
    MERROR startPipelineResult = OK;
    {
        MERROR ret = mpRequestController_P1->startPipeline(
                                    request_num_start,
                                    request_num_end,
                                    mpPipeline_P1,
                                    this
                                );
        if(ret != OK){
            MY_LOGE("startPipeline mpRequestController_P1 failed!");
            startPipelineResult = UNKNOWN_ERROR;
        }
    }
    {
        MERROR ret = mpRequestController_P1_Main2->startPipeline(
                                    request_num_start,
                                    request_num_end,
                                    mpPipeline_P1_Main2,
                                    mpStereoRequestUpdater_P1_Main2
                                );
        if(ret != OK){
            MY_LOGE("startPipeline mpRequestController_P1_Main2 failed!");
            startPipelineResult = UNKNOWN_ERROR;
        }
    }
    {
        MERROR ret = mpRequestController_P2->startPipeline(
                                    request_num_start,
                                    request_num_end,
                                    mpPipeline_P2,
                                    mpStereoRequestUpdater_P2,
                                    LegacyPipelineMode_T::PipelineMode_Feature_StereoZSD
                                );
        if(ret != OK){
            MY_LOGE("startPipeline mpRequestController_P2 failed!");
            startPipelineResult = UNKNOWN_ERROR;
        }
    }
    return startPipelineResult;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
StereoFlowControl::
buildStereoP1Pipeline_Main1(
)
{
    FUNCTION_SCOPE;
    mpPipeline_P1 = constructP1Pipeline_Main1(
        mCurrentPipelineContent.mP1Main1Content
    );
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
StereoFlowControl::
buildStereoP1Pipeline_Main2(
)
{
    FUNCTION_SCOPE;
    mpPipeline_P1_Main2 = constructP1Pipeline_Main2(
        mCurrentPipelineContent.mP1Main2Content
    );
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
StereoFlowControl::
buildStereoP2Pipeline()
{
    FUNCTION_SCOPE;
    status_t ret = UNKNOWN_ERROR;
    MUINT32 stereoMode = mCurrentStereoMode;
    MUINT32 pipelineMode = mPipelineMode;
    MUINT32 sensorModuleType = mCurrentSensorModuleType;
    //
    if((Stereo::STEREO_3RDPARTY == (stereoMode)) &&
        pipelineMode == PipelineMode_ZSD)
    {
        MY_LOGD("get pure 3rd table");
        mpPipeline_P2 = constructP2Pipeline_PrvCap_3rdParty(
            mCurrentPipelineContent,
            mpImageStreamManager
        );
    }
    else if((Stereo::STEREO_TK_DEPTH_3RD_PRV == (stereoMode)) &&
        pipelineMode == PipelineMode_ZSD)
    {
        MY_LOGD("get tkbokeh+3rd table");
        mpPipeline_P2 = constructP2Pipeline_PrvCap_BB(
                            mCurrentPipelineContent,
                            mpImageStreamManager
                     );
    }
    else if(Stereo::STEREO_BB_PRV_CAP_REC == (stereoMode|sensorModuleType) &&
       pipelineMode == PipelineMode_ZSD)
    {
        MY_LOGD("get bb prv_cap table");
        mpPipeline_P2 = constructP2Pipeline_PrvCap_BB(
                            mCurrentPipelineContent,
                            mpImageStreamManager
                     );
    }
    else if(Stereo::STEREO_BB_PRV_CAP_REC == (stereoMode|sensorModuleType) &&
       pipelineMode == PipelineMode_RECORDING)
    {
        MY_LOGD("get bb rec table");
        mpPipeline_P2 = constructP2Pipeline_Rec_BB(
                            mCurrentPipelineContent
                     );
    }
    else if(Stereo::STEREO_BM_PRV_CAP_REC == (stereoMode|sensorModuleType) &&
       pipelineMode == PipelineMode_ZSD)
    {
        MY_LOGW("get bm prv_cap table");
        mpPipeline_P2 = constructP2Pipeline_PrvCap_BB(
            mCurrentPipelineContent,
            mpImageStreamManager
        );
    }
    else if(Stereo::STEREO_BM_PRV_CAP_REC == (stereoMode|sensorModuleType) &&
       pipelineMode == PipelineMode_RECORDING)
    {
        MY_LOGD("get bm rec table");
        mpPipeline_P2 = constructP2Pipeline_Rec_BB(
                            mCurrentPipelineContent
                     );
    }
    else
    {
        MY_LOGE("should not happened!");
        goto lbExit;
    }

    ret = OK;
lbExit:
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/

MBOOL
StereoFlowControl::
initAFMgr()
{
    MY_LOGD("Init stereo for af_mgr");
    MBOOL ret = MTRUE;
    int32_t main1Idx, main2Idx;
    StereoSettingProvider::getStereoSensorIndex(main1Idx, main2Idx);
    NS3Av3::IHal3A *pHal3A = MAKE_Hal3A(main1Idx, LOG_TAG);
    bool isAF = false;
    if(NULL == pHal3A) {
        MY_LOGE("Cannot get 3A HAL of sensor %d", main1Idx);
        ret = MFALSE;
    } else {
        NS3Av3::FeatureParam_T rFeatureParam;
        if(pHal3A->send3ACtrl(NS3Av3::E3ACtrl_GetSupportedInfo, (MUINTPTR)&rFeatureParam, 0)) {
            isAF = (rFeatureParam.u4MaxFocusAreaNum > 0);
            MY_LOGD("Is AF: %d", isAF);
        } else {
            MY_LOGW("Cannot query AF ability from 3A");
        }

        if(isAF) {
            if(NULL == mpAFTable) {
                pHal3A->send3ACtrl(NS3Av3::E3ACtrl_GetAFDAFTable, (MUINTPTR)&mpAFTable, 0);
            }

            if(mpAFTable) {
                MY_LOGD("af_mgr: enable is_daf_run");
                mpAFTable->is_daf_run |= NS3Av3::E_DAF_RUN_STEREO;
            } else {
                MY_LOGE("[AF] Cannot get AF table of sensor %d", main1Idx);
                ret = MFALSE;
            }
        }

        pHal3A->destroyInstance(LOG_TAG);
    }
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
StereoFlowControl::
uninitAFMgr()
{
    if(mpAFTable) {
        MY_LOGD("Uninit stereo for af_mgr");
        mpAFTable->is_daf_run &= ~NS3Av3::E_DAF_RUN_STEREO;
        mpAFTable = NULL;
    } else {
        MY_LOGE("AF table is NULL");
    }
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
StereoFlowControl::
setAFSyncMode()
{
    int mode = 0;
    if(mCurrentStereoMode & Stereo::E_STEREO_FEATURE_VSDOF)
    {
        mode = 1;
    }

    MY_LOGD("Set AFSyncMode %d", mode);

}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
StereoFlowControl::
resetAFSyncMode()
{
    MY_LOGD("Reset AFSyncMode");
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
StereoFlowControl::
uninitP2Pipeline()
{
    FUNCTION_SCOPE;
    // stop P2's pipeline
    MY_LOGD("mpRequestController_P2->stopPipeline");
    if( mpRequestController_P2 != 0 ){
        mpRequestController_P2->stopPipeline();
        mpRequestController_P2 = NULL;
    }
    MY_LOGD("mpPipeline_P2->stopPipeline");
    if(mpPipeline_P2!=nullptr){
        mpPipeline_P2->flush();
        mpPipeline_P2->waitUntilDrained();
    }
    return MTRUE;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
StereoFlowControl::
releaseP2Resource()
{
    FUNCTION_SCOPE;
    if ( mpPipeline_P2 != 0 ){
        mpPipeline_P2 = NULL;
        // no one will use these buffer pool anymore
        mpStereoBufferPool_RESIZER = NULL;
        mpStereoBufferPool_RESIZER_MAIN2 = NULL;
        mpStereoBufferPool_OPAQUE = NULL;
        mpStereoBufferPool_OPAQUE_MAIN2 = NULL;

        mpStereoRequestUpdater_P2 = NULL;
    }
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
StereoFlowControl::
uninitP1Main1Pipeline()
{
    FUNCTION_SCOPE;
    // stop P1's pipeline
    MY_LOGD("mpRequestController_P1->stopPipeline");
    if( mpRequestController_P1 != 0 ){
        mpRequestController_P1->stopPipeline();
        mpRequestController_P1 = NULL;
    }
    MY_LOGD("mpPipeline_P1->stopPipeline");
    if ( mpPipeline_P1 != 0 ){
        mpPipeline_P1->flush();
        mpPipeline_P1->waitUntilDrained();
    }
    return MTRUE;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
StereoFlowControl::
uninitP1Main2Pipeline()
{
    FUNCTION_SCOPE;
    // stop P1_main2's pipeline
    MY_LOGD("mpRequestController_P1_Main2->stopPipeline");
    if( mpRequestController_P1_Main2 != 0 ){
        mpRequestController_P1_Main2->stopPipeline();
        mpRequestController_P1_Main2 = NULL;
    }
    MY_LOGD("mpPipeline_P1_Main2->stopPipeline");
    if ( mpPipeline_P1_Main2 != 0 ){
        mpPipeline_P1_Main2->flush();
        mpPipeline_P1_Main2->waitUntilDrained();
    }
    return MTRUE;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
StereoFlowControl::
releasePostProcessor()
{
    FUNCTION_SCOPE;
    sp<ImagePostProcessor> pIPP = IResourceContainer::queryImagePostProcessor();
    if(pIPP != nullptr)
    {
        pIPP->setCameraStatus(CameraMode::CloseCamera);
        if(pIPP->size() == 0)
        {
            MY_LOGD("no capture photo in queue, do release");
            IResourceContainer::clearImagePostProcessor();
        }
    }
    else
    {
        MY_LOGD("image processor is already release");
    }
    return MTRUE;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
StereoFlowControl::
enterMMDVFSScenario()
{
    FUNCTION_SCOPE;
    BWC BwcIns;
    BwcIns.Profile_Change(BWCPT_CAMERA_ICFP,true);
    mmdvfs_set(
        BWCPT_CAMERA_ICFP, // for stereo, it has use this.
        MMDVFS_CAMERA_MODE_STEREO,      MTRUE,
        MMDVFS_PARAMETER_EOF);
    mbDVFSLevel = true;
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
StereoFlowControl::
exitMMDVFSScenario()
{
    FUNCTION_SCOPE;
    BWC BwcIns;
    BwcIns.Profile_Change(BWCPT_CAMERA_ICFP,false);
    mbDVFSLevel = false;
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
StereoFlowControl::
onP2Done()
{
    if(mpRequestController_P2!=nullptr && !mbPreviewEnable)
    {
        Mutex::Autolock _l(mCaptureLock);
        mpStereoBufferSynchronizer->flushCaptureQueue();
        mpRequestController_P2->resumePipeline();
        mbPreviewEnable = MTRUE;
        MY_LOGD("preview enable(%d)", mbPreviewEnable);
    }
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
StereoFlowControl::
onCaptureDone(
)
{
    Mutex::Autolock _l(mCaptureQueueLock);
    MINT32 postProcCnt = 0;
    MBOOL isCaptureAvailable = MFALSE;
    //if (mbCanSupportBackgroundService)
    {

        android::NSPostProc::IImagePostProcessManager* pIPP =
                                    android::NSPostProc::IImagePostProcessManager::getInstance();
        if(pIPP != nullptr)
        {
            isCaptureAvailable = pIPP->isAvailable();
            postProcCnt = pIPP->size();
        }

        // Notify Capture Done to enable face detection
        mpCamMsgCbInfo->doExtCallback(
                        MTK_CAMERA_MSG_EXT_NOTIFY_CAPTURE_DONE,
                        0,
                        0
                    );
    }
    onP2Done();
    MUINT32 size = mvCaptureQueue.size();
    if(mvCaptureQueue.size() != 0)
    {
        mvCaptureQueue.pop();
    }
    MY_LOGD("current capture queue size(%d), PostProcCnt(%d), CapAvailable(%d)",
                                        size,
                                        postProcCnt,
                                        isCaptureAvailable);
    if(mvCaptureQueue.size() == 0)
    {
        MY_LOGD("post capture done");
        mCondCaptureLock.signal();
    }
    if(mvCaptureQueue.size() + postProcCnt == 0)
    {
        enterPerformanceScenario(ScenarioMode::PREVIEW);

        // set status
        if(mpStereoRequestUpdater_P2 != nullptr)
        {
            mpStereoRequestUpdater_P2->setStereoFeatureStatus(MTK_STEREO_FEATURE_STATUS_PREVIEW);
        }
    }
    if((mvCaptureQueue.size() + postProcCnt) <= ::get_vsdof_cap_queue_size())
    {
        MY_LOGD("unlock");
        mCondCaptureLock.signal();
    }
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
StereoFlowControl::
onPostProcDone()
{
    Mutex::Autolock _l(mCaptureQueueLock);
    FUNCTION_SCOPE;
    MINT32 postProcCnt = 0;
    MBOOL isCaptureAvailable = MFALSE;
    //if (mbCanSupportBackgroundService)
    {
        android::NSPostProc::IImagePostProcessManager* pIPP =
                                android::NSPostProc::IImagePostProcessManager::getInstance();
        if(pIPP != nullptr)
        {
            isCaptureAvailable = pIPP->isAvailable();
            postProcCnt = pIPP->size();
        }
    }
    MUINT32 size = mvCaptureQueue.size();
    MY_LOGD("current capture queue size(%d), PostProcCnt(%d), CapAvailable(%d)",
                                        size,
                                        postProcCnt,
                                        isCaptureAvailable);
    if(size + postProcCnt == 0)
    {
        enterPerformanceScenario(ScenarioMode::PREVIEW);
        // set status
        if(mpStereoRequestUpdater_P2 != nullptr)
        {
            mpStereoRequestUpdater_P2->setStereoFeatureStatus(MTK_STEREO_FEATURE_STATUS_PREVIEW);
        }
    }
    if((mvCaptureQueue.size() + postProcCnt) <= ::get_vsdof_cap_queue_size())
    {
        MY_LOGD("unlock");
        mCondCaptureLock.signal();
    }
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
StereoFlowControl::
onEvent(
    MINT32  capNo,
    CaptureRequestCB callbackType,
    MUINT32 streamId,
    MBOOL bError,
    void* params1,
    void* params2
)
{
    auto cbTypeToString = [&callbackType]()
    {
        switch(callbackType)
        {
            case CaptureRequestCB::NONE:
                return "NONE";
            case CaptureRequestCB::DNG:
                return "DNG";
            case CaptureRequestCB::STRAME_ID:
                return "STRAME_ID";
            case CaptureRequestCB::SHUTTER:
                return "SHUTTER";
            case CaptureRequestCB::P2_DONE:
                return "P2_DONE";
            case CaptureRequestCB::CAPTURE_DONE:
                return "CAPTURE_DONE";
            case CaptureRequestCB::EXTRA_DATA:
                return "EXTRA_DATA";
            case CaptureRequestCB::DEPTH_MAP:
                return "DEPTH_MAP";
            case CaptureRequestCB::JPEG:
                return "JPEG";
            case CaptureRequestCB::RAW16:
                return "RAW16";
            case CaptureRequestCB::POSTVIEW:
                return "POSTVIEW";
            case CaptureRequestCB::POSTPROC_DONE:
                return "POSTPROC_DONE";
            case CaptureRequestCB::BGYUV:
                return "BGYUV";
            default:
                return "";
        }
        return "";
    };
    MY_LOGD("cap(%d) cb[%s] streamId(%x) bError(%d)", capNo, cbTypeToString(), streamId, bError);
    switch(callbackType)
    {
            case CaptureRequestCB::DNG:
            case CaptureRequestCB::STRAME_ID:
            case CaptureRequestCB::EXTRA_DATA:
            case CaptureRequestCB::DEPTH_MAP:
            case CaptureRequestCB::JPEG:
            case CaptureRequestCB::RAW16:
            case CaptureRequestCB::SHUTTER:
            case CaptureRequestCB::BGYUV:
                break;
            case CaptureRequestCB::POSTVIEW:
            case CaptureRequestCB::P2_DONE:
                if(!mbPreviewEnable)
                {
                    onP2Done();
                }
                break;
            case CaptureRequestCB::CAPTURE_DONE:
                if(!mbPreviewEnable)
                {
                    // resume preview manually.
                    onP2Done();
                }
                onCaptureDone();
                break;
            case CaptureRequestCB::POSTPROC_DONE:
                onPostProcDone();
                break;
    }
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
StereoFlowControl::
onSetStereoBufferSize()
{
    // for AP to read
    int size;
    StereoArea area;
    area = StereoSizeProvider::getInstance()->getBufferSize(E_LDC, eSTEREO_SCENARIO_CAPTURE);
    size = area.size.w*area.size.h;
    MY_LOGD("set LDC buffer: size %d, w  %d, h %d", size, area.size.w, area.size.h);
    mpParamsManagerV3->getParamsMgr()->set(MtkCameraParameters::KEY_STEREO_LDC_SIZE, size);
    size = StereoSettingProvider::getMaxN3DDebugBufferSizeInBytes();
    MY_LOGD("set N3D buffer: size %d", size);
    mpParamsManagerV3->getParamsMgr()->set(MtkCameraParameters::KEY_STEREO_N3D_SIZE, size);
    size = StereoSettingProvider::getExtraDataBufferSizeInBytes();
    MY_LOGD("set Extra buffer: size %d", size);
    mpParamsManagerV3->getParamsMgr()->set(MtkCameraParameters::KEY_STEREO_EXTRA_SIZE, size);
    area.size = StereoSizeProvider::getInstance()->thirdPartyDepthmapSize(StereoSettingProvider::stereoProfile(), StereoSettingProvider::imageRatio());
    size = area.size.w*area.size.h;
    MY_LOGD("set Depth buffer: size %d, w  %d, h %d", size, area.size.w, area.size.h);
    mpParamsManagerV3->getParamsMgr()->set(MtkCameraParameters::KEY_STEREO_DEPTH_SIZE, size);

    return OK;
}

