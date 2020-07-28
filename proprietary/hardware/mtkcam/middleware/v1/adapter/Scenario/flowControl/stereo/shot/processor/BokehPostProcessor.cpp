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

#define LOG_TAG "BokehPostProcessor"
//
#include <mtkcam/def/BuiltinTypes.h>
#include <mtkcam/def/Errors.h>
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Misc.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/NodeId.h>
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
//
using namespace NSCam;
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/PostProcessor/bokeh/BokehPostProcessorData.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/StereoLegacyPipelineDef.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/StereoLegacyPipelineBuilder.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/StreamBufferProviderFactory.h>
#include <mtkcam/middleware/v1/camshot/BufferCallbackHandler.h>
//
#include "BokehPostProcessor.h"
//
#include <mtkcam/feature/effectHalBase/ICallbackClientMgr.h>
//
#include "../builder/SwBokehPostProcessRequestBuilder.h"
/******************************************************************************
 *
 ******************************************************************************/
#include <chrono>
#include <string>
#include <sys/resource.h>
/******************************************************************************
 *
 ******************************************************************************/
#define CHECK_OBJECT(x)  do{                                        \
    if (x == nullptr) { MY_LOGE("Null %s Object", #x); return MFALSE;} \
} while(0)
/******************************************************************************
 *
 ******************************************************************************/
#define BOKEHPOSTPROCESSOR_NAME       ("Cam@BokehPostProcessor")
#define BOKEHPOSTPROCESSOR_POLICY     (SCHED_OTHER)
#define BOKEHPOSTPROCESSOR_PRIORITY   (0)
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

#define OPEN_ID 22
#define METADATA_TBL        BokehPostProcessorData::full::gBokehMetaTbl_full
#define IMAGEDATA_TBL       BokehPostProcessorData::full::gBokehImgTbl_full
#define NODEDATA_TBL        BokehPostProcessorData::full::gBokehConnectSetting_full
using namespace NSCam::v1::NSLegacyPipeline;

/******************************************************************************
 *
 ******************************************************************************/
void
BokehPostProcessor::
destroyPostProcessor()
{
    FUNCTION_SCOPE;
    sp<ImagePostProcessor> pIPP = IResourceContainer::queryImagePostProcessor();
    if(pIPP != nullptr)
    {
        pIPP->flush();
    }
    IResourceContainer::clearImagePostProcessor();
}

/******************************************************************************
 *
 ******************************************************************************/
BokehPostProcessor::
BokehPostProcessor()
{
    FUNCTION_SCOPE;
    init();
}
/******************************************************************************
 *
 ******************************************************************************/
BokehPostProcessor::
~BokehPostProcessor()
{
    FUNCTION_SCOPE;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
BokehPostProcessor::
enque(
    sp<ImagePostProcessorData> data
)
{
    FUNCTION_SCOPE;
    Mutex::Autolock _l(mQueueLock);
    mQueue.push_back(data);
    mQueueCond.signal();
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
BokehPostProcessor::
flush()
{
    FUNCTION_SCOPE;
    Mutex::Autolock _l(mQueueLock);
    mQueue.clear();
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
BokehPostProcessor::
waitUntilDrain()
{
    FUNCTION_SCOPE;
    if(mpPipeline != nullptr)  mpPipeline->waitUntilDrained();
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
BokehPostProcessor::
destroy()
{
    FUNCTION_SCOPE;
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
MINT32
BokehPostProcessor::
size()
{
    MINT32 size1, size2;
    FUNCTION_SCOPE;
    {
        Mutex::Autolock _l(mQueueLock);
        size1 = mQueue.size() + (mbOnProcess == MTRUE ? 1 : 0);
    }
    {
        Mutex::Autolock _l(mRequestLock);
        size2 = mvPostProcRequests.size();
    }
    MY_LOGD("size: Queue(%d), OnProcess(%d)", size1, size2);
    return (size1+size2);
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
BokehPostProcessor::
setCameraStatus(CameraMode mode)
{
    FUNCTION_SCOPE;
    Mutex::Autolock _l(mInitLock);
    mCameraMode = mode;
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
BokehPostProcessor::
setCallback(sp<ICaptureRequestCB> pCb)
{
    FUNCTION_SCOPE;
    Mutex::Autolock _l(mCBLock);
    mpCb = pCb;
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
BokehPostProcessor::
init()
{
    Mutex::Autolock _l(mInitLock);
    if (mbInit) return;
    if(!StereoSettingProvider::getStereoSensorIndex(mSensorId, mSensorIdMain2)){
        MY_LOGE("Cannot get sensor ids from StereoSettingProvider! (%d,%d)", mSensorId, mSensorIdMain2);
        return;
    }
    if(mpProcessThread == nullptr)
    {
      mpProcessThread = new ProcessThread(this);
      if(mpProcessThread->run(LOG_TAG) != OK)
      {
          MY_LOGE("create post process thread fail.");
          return;
      }
    }
    ICallbackClientMgr::getInstance()->registerCB(destroyPostProcessor);
    mbOnProcess = MFALSE;
    mbInit = MTRUE;
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
BokehPostProcessor::
unInit()
{
    Mutex::Autolock _l(mInitLock);
    if (!mbInit) return;
    mbInit = MFALSE;
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
BokehPostProcessor::
onLastStrongRef(
    const void* /* id */
)
{
    FUNCTION_SCOPE;
    flush();
    waitUntilDrain();
    if(mpProcessThread != nullptr)
    {
        mpProcessThread->requestExit();
        mQueueCond.signal();
        mpProcessThread->join();
    }
    mvPostProcRequests.clear();
    exitPerformanceScenario();
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
BokehPostProcessor::
clearPipeline()
{
    Mutex::Autolock _l(mInitLock);
    if (mCameraMode == CameraMode::CloseCamera)
    {
        mpPipeline            = nullptr;
        mpPool_Main           = nullptr;
        mpPool_DepthWrapper   = nullptr;
        mpPool_Dbg            = nullptr;
        mpImageStreamManager  = nullptr;
    }
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
BokehPostProcessor::
buildPipeline()
{
    FUNCTION_SCOPE;

    if (mpPipeline != nullptr)
    {
        MY_LOGE("Pipeline already be created");
        return OK;
    }
    // create legacy pipeline builder
    sp<StereoLegacyPipelineBuilder> pBuilder =
            new StereoLegacyPipelineBuilder((MINT32)PostImgProcessorType::BOKEH,
                                            (char *)"BokehPostPipeline",
                                            StereoLegacyPipelineMode_T::STPipelineMode_P2);
    if ( pBuilder == 0 ) {
        MY_LOGE("Cannot create LegacyPipelineBuilder.");
        return UNKNOWN_ERROR;
    }
    // create MetaStreamInfo
    sp<MetaStreamManager> pMetaStreamManager =
            MetaStreamManager::create(METADATA_TBL);
    if(pMetaStreamManager == nullptr)
    {
        MY_LOGE("Create MetaStreamManager fail.");
        return UNKNOWN_ERROR;
    }
    // create ImageStreamInfo
    sp<ImageStreamManager> pImageStreamManager =
            ImageStreamManager::create(IMAGEDATA_TBL);
    if(pImageStreamManager == nullptr)
    {
        MY_LOGE("Create ImageStreamManager fail.");
        return UNKNOWN_ERROR;
    }
    //
    const NodeConfigDataManager::NodeConfigMgrSetting cfgSetting =
                                        {
                                            (MUINT32)mSensorId,
                                            (MUINT32)mSensorIdMain2,
                                            NODEDATA_TBL
                                        };
    // create Node & Edge
    sp<NodeConfigDataManager> pNodeConfigDataManager =
            NodeConfigDataManager::create("postprocessor", &cfgSetting);
    if(pNodeConfigDataManager == nullptr)
    {
        MY_LOGE("Create NodeConfigDataManager fail.");
        return UNKNOWN_ERROR;
    }
    // update tbl
    {
        auto setProvider = [this, &pImageStreamManager](MINT32 streamId, char const* name)
        {
            sp<IImageStreamInfo> pImageInfo = pImageStreamManager->getStreamInfoByStreamId(streamId);
            if(pImageInfo != nullptr)
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
                if (streamId == eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_MAINIMAGE_CAPYUV)
                {
                    mpPool_Main = pPool;
                }
                else if (streamId == eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_HAL_DEPTHWAPPER)
                {
                    mpPool_DepthWrapper = pPool;
                }
                else if (streamId == eSTREAMID_IMAGE_PIPE_STEREO_DBG)
                {
                    mpPool_Dbg = pPool;
                }

                android::sp<StreamBufferProvider> pProvider = pFactory->create(OPEN_ID, MTRUE);

                pImageStreamManager->updateBufProvider(
                                                pImageInfo->getStreamId(),
                                                pProvider,
                                                Vector<StreamId_T>());
            }
            else {
                MY_LOGD("%s did not config in ImageStreamMnger!!", name);
            }
        };
        // depthmap, the input of SwBokeh Node
        setProvider(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_MAINIMAGE_CAPYUV, "capture yuv image");
        setProvider(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_HAL_DEPTHWAPPER , "hal depth wrapper");
        setProvider(eSTREAMID_IMAGE_PIPE_STEREO_DBG                   , "extra debug");
        // the output of SwBokehNode
        setProvider(eSTREAMID_IMAGE_PIPE_STEREO_APP_EXTRA_DATA        , "app extra data");
        setProvider(eSTREAMID_IMAGE_PIPE_STEREO_DEPTHWRAPPER          , "depthmap wrapper");
        setProvider(eSTREAMID_IMAGE_PIPE_BOKEHNODE_CLEANIMAGEYUV      , "clean yuv image");
        setProvider(eSTREAMID_IMAGE_PIPE_BOKEHNODE_RESULTYUV          , "bokeh result image");
        setProvider(eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL                , "thumbnail");

        // the output of JpegNode
        setProvider(eSTREAMID_IMAGE_PIPE_JPG_CleanMainImg             , "clean image");
        setProvider(eSTREAMID_IMAGE_PIPE_JPG_Bokeh                    , "bokeh image");

        if (mpPool_Main.get()) mpPool_Main->setNotifier(this);
        if (mpPool_DepthWrapper.get()) mpPool_DepthWrapper->setNotifier(this);
        if (mpPool_Dbg.get()) mpPool_Dbg->setNotifier(this);
    }
    //
    pBuilder->setMetaStreamId(
                eSTREAMID_META_HAL_DYNAMIC_DEPTH,
                eSTREAMID_META_APP_CONTROL);

    // Tell StereoLegacyPipelineBuilder we need to substitute default requestBuilder
    {
        sp<PipelineContext> pipelineContext = nullptr;
        sp<NSCam::StereoBasicParameters> userParams = new NSCam::StereoBasicParameters();
        pNodeConfigDataManager->configNode(
                                    pMetaStreamManager,
                                    pImageStreamManager,
                                    userParams,
                                    pipelineContext);

        Vector< sp<NodeConfigDataManager> > vRequests;
        vRequests.push_back(pNodeConfigDataManager);

        pBuilder->setRequestTypes(pNodeConfigDataManager, vRequests);
    }

    mpImageStreamManager = pImageStreamManager;

    mpPipeline = pBuilder->create(pMetaStreamManager, pImageStreamManager, pNodeConfigDataManager);
    if(mpPipeline == nullptr){
        MY_LOGE("pBuilder->create PostProcPipeline failed");
        return UNKNOWN_ERROR;
    }
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
BokehPostProcessor::
onDequeData(
    android::sp<ImagePostProcessorData>& data
)
{
    FUNCTION_SCOPE;
    MBOOL bPostProcAvail;
    {
        Mutex::Autolock _l(mRequestLock);
        bPostProcAvail = mvPostProcRequests.size() < MAX_POSTPROC_REQUSET_CNT;
    }
    Mutex::Autolock _l(mQueueLock);
    //  Wait until the queue is not empty or not going exit
    while ((mQueue.empty() || !bPostProcAvail) && !mbExit)
    {
        status_t status = mQueueCond.wait(mQueueLock);
        if  ( OK != status ) {
            MY_LOGW(
                "wait status:%d:%s, mQueue.size:%zu",
                status, ::strerror(-status), mQueue.size()
            );
        }
        {
            Mutex::Autolock _l(mRequestLock);
            bPostProcAvail = mvPostProcRequests.size() < MAX_POSTPROC_REQUSET_CNT;
            if (!bPostProcAvail && !mQueue.empty())
            {
              MY_LOGD("Wait PostProc size(%d), Queue size(%d)", mvPostProcRequests.size(), mQueue.size());
            }
            if ((mvPostProcRequests.size() + mQueue.size()) == 0)
            {
                exitPerformanceScenario();
                clearPipeline();
            }
        }
    }
    //
    if  ( mbExit ) {
        MY_LOGW_IF(!mQueue.empty(), "[flush] mQueue.size:%zu", mQueue.size());
        return DEAD_OBJECT;
    }

    if (!mQueue.empty() && bPostProcAvail)
    {
        data = *mQueue.begin();
        mQueue.erase(mQueue.begin());
        mbOnProcess = MTRUE;
    }
    else
    {
        MY_LOGE("shouldn't go here!!");
    }

    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
BokehPostProcessor::
onResultReceived(
    MUINT32    const requestNo,
    StreamId_T const streamId,
    MBOOL      const errorBuffer,
    android::sp<IImageBuffer>& pBuffer
)
{
    MINT64 timeStamp = -1;
    sp<ICaptureRequestCB> pCB = nullptr;
    MINT32          captureNo = -1;
    MBOOL           done = MFALSE;
    {
        Mutex::Autolock _l(mRequestLock);
        ssize_t idx = mvPostProcRequests.indexOfKey(requestNo);
        if (idx >= 0)
        {
            PostProcRequestInfo *info = &mvPostProcRequests.editValueFor(requestNo);
            timeStamp = info->timeStamp;
            info->cbCountDown--;
            if (info->cbCountDown == 0)
            {
                done = MTRUE;
                pCB       = info->pCb.promote();
                captureNo = info->captureNo;
                if (pCB == nullptr)
                {
                    Mutex::Autolock _l(mCBLock);
                    pCB       = mpCb.promote();
                }
            }
        }
    }

    pBuffer->lockBuf(LOG_TAG, eBUFFER_USAGE_SW_READ_MASK);
    MUINT32 msgType = streamId == eSTREAMID_IMAGE_PIPE_STEREO_APP_EXTRA_DATA  ? MTK_CAMERA_MSG_EXT_DATA_STEREO_DBG :
                      streamId == eSTREAMID_IMAGE_PIPE_STEREO_DEPTHWRAPPER    ? MTK_CAMERA_MSG_EXT_DATA_STEREO_DEPTHWRAPPER :
                      streamId == eSTREAMID_IMAGE_PIPE_JPG_CleanMainImg       ? MTK_CAMERA_MSG_EXT_DATA_STEREO_CLEAR_IMAGE :
                                                                                MTK_CAMERA_MSG_EXT_DATA_COMPRESSED_IMAGE;
    MUINT32 u4Size = (streamId == eSTREAMID_IMAGE_PIPE_JPG_CleanMainImg || streamId == eSTREAMID_IMAGE_PIPE_JPG_Bokeh)
                     ? pBuffer->getBitstreamSize() : pBuffer->getBufSizeInBytes(0);

    uint8_t const* puBuf = (uint8_t const*)pBuffer->getBufVA(0);
    MY_LOGD("requestNo %d, msgType %d, u4Size %d, pBuf(%p), pHeap(%p)", requestNo, msgType, u4Size, puBuf, pBuffer->getImageBufferHeap());
    if (errorBuffer == MFALSE)
    {
        ICallbackClientMgr::getInstance()->onCB_CompressedImage_packed(
                                                timeStamp,
                                                u4Size,
                                                puBuf,
                                                msgType,
                                                0,
                                                msgType,
                                                MTK_CAMERA_CALLBACK_CLIENT_ID_DUAL);
    }
    else
    {
        MY_LOGE("Error Buffer !!");
    }
    pBuffer->unlockBuf(LOG_TAG);

    /* callback to StereoFlowControl */
    if (pCB != nullptr)
    {
        pCB->onEvent(
            captureNo,
            CaptureRequestCB::POSTPROC_DONE,
            0,
            0);
    }
    {
        Mutex::Autolock _l(mRequestLock);
        if (done)
        {
            mvPostProcRequests.removeItem(requestNo);
            mQueueCond.signal();
            MY_LOGD("requestNo %d done, size %zu", requestNo, mvPostProcRequests.size());
        }
    }
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
BokehPostProcessor::
onBufferAcquired(
    MINT32           rRequestNo,
    StreamId_T       rStreamId
)
{
  return;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
BokehPostProcessor::
onBufferReturned(
    MINT32                         /*rRequestNo*/,
    StreamId_T                     /*rStreamId*/,
    bool                           /*bErrorBuffer*/,
    android::sp<IImageBufferHeap>& /*rpBuffer */
)
{
    return true;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
BokehPostProcessor::
onProcessData(
    android::sp<ImagePostProcessorData> const& data
)
{
    FUNCTION_SCOPE;
    status_t ret = UNKNOWN_ERROR;
    MSize depthMapWrapper;
    MUINT32 extraDataSize = 0;

    MY_LOGD("Type[%d] Img[%zu] meta[%zu]",
                data->mProcessType,
                data->mImageQueue.size(),
                data->mMetadataQueue.size());
    if(mpPipeline == nullptr)
    {
        buildPipeline();
    }
#define CALLBACK_BUFFER(streamId, msgType) \
    do\
    {\
        ssize_t index = data->mImageQueue.indexOfKey(streamId);\
        if(index >= 0)\
        {\
        sp<IImageBuffer> pBuffer = data->mImageQueue.valueAt(index);\
        pBuffer->lockBuf(LOG_TAG, eBUFFER_USAGE_SW_READ_MASK);\
        MUINT32 u4Size = pBuffer->getBufSizeInBytes(0);\
        MY_LOGD("msgType %d, u4Size %d", msgType, u4Size);\
        uint8_t const* puBuf = (uint8_t const*)pBuffer->getBufVA(0);\
        ICallbackClientMgr::getInstance()->onCB_CompressedImage_packed(\
                                                data->miTimeStamp,\
                                                u4Size,\
                                                puBuf,\
                                                msgType,\
                                                0,\
                                                msgType,\
                                                MTK_CAMERA_CALLBACK_CLIENT_ID_DUAL);\
        pBuffer->unlockBuf(LOG_TAG);\
        }\
        else\
        {\
            MY_LOGE("Should not happened. miss -" #streamId);\
        }\
    }while(0);
    CALLBACK_BUFFER(eSTREAMID_IMAGE_PIPE_STEREO_DBG_LDC, MTK_CAMERA_MSG_EXT_DATA_STEREO_LDC);
    CALLBACK_BUFFER(eSTREAMID_IMAGE_PIPE_STEREO_N3D_DEBUG, MTK_CAMERA_MSG_EXT_DATA_STEREO_N3D_DEBUG);

#define SET_INPUT_BUFFER(streamId, bufferPool, name) \
    do\
    {\
        ssize_t index = data->mImageQueue.indexOfKey(streamId);\
        if(index >= 0)\
        {\
            if(bufferPool != nullptr)\
            {\
                sp<IImageBuffer> pBuffer = data->mImageQueue.valueAt(index);\
                bufferPool->addBuffer(pBuffer);\
                if (streamId == eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_HAL_DEPTHWAPPER)\
                {\
                    depthMapWrapper = pBuffer->getImgSize();\
                }\
                else if (streamId == eSTREAMID_IMAGE_PIPE_STEREO_DBG)\
                {\
                    extraDataSize = pBuffer->getBufSizeInBytes(0);\
                }\
            }\
            else\
            {\
                MY_LOGE(#bufferPool " is null");\
            }\
        }\
        else\
        {\
            MY_LOGE("Should not happened. miss -" #streamId);\
        }\
    }while(0);
    SET_INPUT_BUFFER(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_MAINIMAGE_CAPYUV, mpPool_Main, "Cap_Clean_Main");
    SET_INPUT_BUFFER(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_HAL_DEPTHWAPPER, mpPool_DepthWrapper, "Cap_DepthWrapper");
    SET_INPUT_BUFFER(eSTREAMID_IMAGE_PIPE_STEREO_DBG, mpPool_Dbg, "Cap_Dbg");
#undef SET_INPUT_BUFFER

    if(mpPipeline != nullptr)
    {
        auto sendTimestampToInput = [this](StreamId_T streamId, MINT64 timestamp)
        {
            sp<StreamBufferProvider> pProvider = mpPipeline->queryProvider(streamId);
            if(pProvider != nullptr)
            {
                pProvider->doTimestampCallback(miPostProcCounter, MFALSE, timestamp);
            }
            else
            {
                MY_LOGE("get provider fail");
            }
        };
        sendTimestampToInput(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_MAINIMAGE_CAPYUV, data->miTimeStamp);
        sendTimestampToInput(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_HAL_DEPTHWAPPER , data->miTimeStamp);
        sendTimestampToInput(eSTREAMID_IMAGE_PIPE_STEREO_DBG                   , data->miTimeStamp);

        sp<IPostProcessRequestBuilder>  pPostProcRequest = nullptr;
        PostProcRequestInitSetting      initSetting;

        {
            pPostProcRequest = new SwBokehPostProcessRequestBuilder();
            {
              Mutex::Autolock _l(mRequestLock);
              PostProcRequestInfo info;
              info.cbCountDown  = 4;
              info.timeStamp    = data->miTimeStamp;
              info.pRequest     = pPostProcRequest;
              info.pCb          = data->mpCb;
              info.captureNo    = data->mCaptureNo;
              MY_LOGD("add Request %d, timestamp %" PRId64 "",miPostProcCounter, info.timeStamp);
              mvPostProcRequests.add(miPostProcCounter, info);
              {
                  Mutex::Autolock _l(mQueueLock);
                  mbOnProcess = MFALSE;
              }

              if (mpScenarioCtrl == nullptr) enterPerformanceScenario();
            }
            if(pPostProcRequest == nullptr)
            {
                MY_LOGE("create PostProc request builder fail");
                goto lbExit;
            }

            // prepare init config
            initSetting.iTargetPipelineId     = OPEN_ID;
            initSetting.iRequestNo            = miPostProcCounter;
            initSetting.timestamp             = data->miTimeStamp;
            initSetting.shotParam             = data->mShotParam;
            initSetting.jpegParam             = data->mJpegParam;
            initSetting.depthMapWrapper       = depthMapWrapper;
            initSetting.extraDataSize         = extraDataSize;
            initSetting.pILegacyPipeline      = mpPipeline;
            initSetting.appMetadata           = data->mMetadataQueue.valueFor(eSTREAMID_META_APP_CONTROL);
            initSetting.halMetadata           = data->mMetadataQueue.valueFor(eSTREAMID_META_HAL_DYNAMIC_DEPTH);
            initSetting.pImageCallback        = this;
            initSetting.pImageStreamManager   = mpImageStreamManager;
            if(pPostProcRequest->init(initSetting) != OK)
            {
                MY_LOGE("some error occuer, please check");
                goto lbExit;
            }
            if(pPostProcRequest->doPostProc() != OK)
            {
                MY_LOGE("some error occur, please check");
                goto lbExit;
            }
        }
    }
    else
    {
        MY_LOGE("pipeline is not exist");
    }
    ret = OK;
lbExit:

    if (ret != OK)
    {
        Mutex::Autolock _l(mRequestLock);
        ssize_t idx = mvPostProcRequests.indexOfKey(miPostProcCounter);
        if (idx >= 0) mvPostProcRequests.removeItem(idx);
    }
    if(++miPostProcCounter > STEREO_FLOW_POSTPROC_REQUSET_NUM_END)
    {
        miPostProcCounter = STEREO_FLOW_POSTPROC_REQUSET_NUM_START;
    }
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
BokehPostProcessor::
enterPerformanceScenario()
{
    FUNCTION_SCOPE;
    if (mpScenarioCtrl != nullptr) return;

    IScenarioControl::ControlParam controlParam;
    controlParam.scenario         = IScenarioControl::Scenario_Capture;
    controlParam.sensorSize       = MSize(0,0);
    controlParam.sensorFps        = 0;
    controlParam.featureFlag      = IScenarioControl::FEATURE_STEREO_CAPTURE;
    controlParam.enableBWCControl = MFALSE;

    mpScenarioCtrl = IScenarioControl::create(OPEN_ID);
    if( mpScenarioCtrl == nullptr )
    {
        MY_LOGE("get Scenario Control fail");
        return;
    }

    mpScenarioCtrl->enterScenario(controlParam);
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
BokehPostProcessor::
exitPerformanceScenario()
{
    FUNCTION_SCOPE;
    if (mpScenarioCtrl == nullptr) return;

    mpScenarioCtrl->exitScenario();
    mpScenarioCtrl = nullptr;

}
/******************************************************************************
 *
 ******************************************************************************/
void
BokehPostProcessor::
ProcessThread::
requestExit()
{
    FUNCTION_SCOPE;
    Thread::requestExit();
    mpProcessImp->mbExit = MTRUE;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
BokehPostProcessor::
ProcessThread::
readyToRun()
{
    //
    //  thread policy & priority
    //  Notes:
    //      Even if pthread_create() with SCHED_OTHER policy, a newly-created thread
    //      may inherit the non-SCHED_OTHER policy & priority of the thread creator.
    //      And thus, we must set the expected policy & priority after a thread creation.
    MINT tid;
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    if (BOKEHPOSTPROCESSOR_POLICY == SCHED_OTHER) {
        sched_p.sched_priority = 0;
        ::sched_setscheduler(0, BOKEHPOSTPROCESSOR_POLICY, &sched_p);
        ::setpriority(PRIO_PROCESS, 0, BOKEHPOSTPROCESSOR_PRIORITY);   //  Note: "priority" is nice value.
    } else {
        sched_p.sched_priority = BOKEHPOSTPROCESSOR_PRIORITY;          //  Note: "priority" is real-time priority.
        ::sched_setscheduler(0, BOKEHPOSTPROCESSOR_POLICY, &sched_p);
    }

    MY_LOGD("tid(%d) policy(%d) priority(%d)", ::gettid(), BOKEHPOSTPROCESSOR_POLICY, BOKEHPOSTPROCESSOR_PRIORITY);

    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
BokehPostProcessor::
ProcessThread::
threadLoop()
{
    android::sp<ImagePostProcessorData> pIPPData = nullptr;
    if  (
            !exitPending()
        &&  OK == mpProcessImp->onDequeData(pIPPData)
        &&  pIPPData != 0
        )
    {
        mpProcessImp->onProcessData(pIPPData);
        return true;
    }

    MY_LOGD("exit unpack thread");
    return  false;
}
