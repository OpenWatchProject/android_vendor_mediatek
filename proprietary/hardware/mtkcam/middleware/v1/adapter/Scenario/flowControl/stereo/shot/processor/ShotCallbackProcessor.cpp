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

#define LOG_TAG "ShotCallbackProcessor"
//
#include <mtkcam/def/BuiltinTypes.h>
#include <mtkcam/def/Errors.h>
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Misc.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/IResourceContainer.h>
//
#include "../image/IImageShotCallback.h"
#include "../metadata/IMetaShotCallback.h"
#include "ShotCallbackProcessor.h"
#include "../MetadataShotCallbackFactory.h"
//
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
//
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
/******************************************************************************
 *
 ******************************************************************************/
#include <chrono>
#include <string>
/******************************************************************************
 *
 ******************************************************************************/
#define CHECK_OBJECT(x)  do{                                        \
    if (x == nullptr) { MY_LOGE("Null %s Object", #x); return MFALSE;} \
} while(0)
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

#define FUNC_START  MY_LOGD("+")
#define FUNC_END    MY_LOGD("-")
#define FUNC_NAME   MY_LOGD("")
#define WRITE_PERMISSION 0660
/******************************************************************************
 *
 ******************************************************************************/
using namespace NSCam;
using namespace NSCam::v1::NSLegacyPipeline;
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
ShotCallbackProcessor::
ShotCallbackProcessor(
    const char* pszShotName,
    CaptureRequestInitSetting &setting
)
{
    mShotName = pszShotName;
    mpShotCallback = setting.mShotCallback;
    char cLogLevel[PROPERTY_VALUE_MAX];
    ::property_get("vendor.di.stereo.dumpcapturedata", cLogLevel, "0");
    MINT32 value = ::atoi(cLogLevel);
    if(value > 0)
    {
        mbDumpResult = MTRUE;
        // regenerate filename if need.
        resetProcessor();
    }
    mpCb = setting.mCB;
    miCaptureNo = setting.iCaptureNo;
    mbSupportPostProcessor = (setting.mPostProcType != android::NSPostProc::PostProcessorType::NONE)?MTRUE:MFALSE;
    if (mbSupportPostProcessor)
    {
        mPostProcess_DataQueue = new ImagePostProcessData();
        mPostProcess_DataQueue->mpCb = setting.mPostProcCB;
        mPostProcess_DataQueue->mProcessType = setting.mPostProcType;
        InputData data;
        mPostProcess_DataQueue->mvInputData.push_back(data);
    }
    MY_LOGD("ctor(%p) cb(%p) dump(%d) cap(%d) post(%d)"
                    , this
                    , mpCb.promote().get()
                    , mbDumpResult
                    , miCaptureNo
                    , mbSupportPostProcessor);
}
/******************************************************************************
 *
 ******************************************************************************/
ShotCallbackProcessor::
~ShotCallbackProcessor()
{
    MY_LOGD("dctor(0x%p)", this);
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
ShotCallbackProcessor::
addCallback(
    sp<IImageShotCallback>& imageCallback
)
{
    if(imageCallback == nullptr)
    {
        return BAD_VALUE;
    }
    mvImageCallbackPool.addCallback(imageCallback);
    if(mbDumpResult)
    {
        imageCallback->setDumpInfo(MTRUE, msFilename);
    }
    if(imageCallback->isNeedImageCount())
        miTotalImgCount++;
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
ShotCallbackProcessor::
addCallback(
    sp<IMetaShotCallback>& metaCallback
)
{
    if(metaCallback == nullptr)
    {
        return BAD_VALUE;
    }
    mvMetaCallbackPool.addCallback(metaCallback);

    mPostProcess_DataQueue->mvInputData[0].mAppMetadataQueue.push_back(IMetadata());
    mPostProcess_DataQueue->mvInputData[0].mHalMetadataQueue.push_back(IMetadata());
    miTotalMetaCount++;
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
ShotCallbackProcessor::
resetProcessor()
{
    if(mbDumpResult)
    {
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        msFilename = std::string("/sdcard/stereo/Cap/")+std::to_string(millis);
        MY_LOGD("filename(%s)", msFilename.c_str());
        NSCam::Utils::makePath(msFilename.c_str(), WRITE_PERMISSION);
    }
    mvImageCallbackPool.clearPool();
    mvMetaCallbackPool.clearPool();
    miImgCount = 0;
    miTotalImgCount = 0;
    miMetaCount = 0;
    miTotalMetaCount = 0;
    mbDepthMeta = MFALSE;
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
ShotCallbackProcessor::
destroy()
{
    Mutex::Autolock _l(mImgResultLock);
    mpShotCallback = nullptr;
    mImgBufferQue.clear();
    mImgCallbackQue.clear();
    mvImageCallbackPool.clearPool();
    mvMetaCallbackPool.clearPool();
    return OK;
}
status_t
ShotCallbackProcessor::
addPostProcInfo(StreamId_T const streamId, IMetadata  const result, MINT64 const timestamp, StereoShotParam const shotParams)
{
    FUNC_START;
    // for post 3rd party
    mPostProcess_DataQueue->mShotParam = shotParams.mShotParam;
    mPostProcess_DataQueue->mJpegParam = shotParams.mJpegParam;
    mPostProcess_DataQueue->miTimeStamp = timestamp;
    appMetadata = result;
    FUNC_END;
    return OK;
}

/******************************************************************************
 * metadata event handler
 ******************************************************************************/
void
ShotCallbackProcessor::
onResultReceived(
    MUINT32    const requestNo,
    StreamId_T const streamId,
    MBOOL      const errorResult,
    IMetadata  const result)
{
    
    Mutex::Autolock _l(mImgResultLock);
    FUNC_START;
    MY_LOGD("[%d]requestNo %d, stream %#" PRIx64, errorResult, requestNo, streamId);

    sp<IMetaShotCallback> callback = nullptr;
    if(mvMetaCallbackPool.getCallback(streamId, callback))
    {
        if(callback==nullptr)
        {
            MY_LOGE("callback is null");
            return;
        }

        // for post 3rd party
        mPostProcess_DataQueue->mRequestNo = requestNo;
        if(callback->getType() == MetadataShotCallbackFactory::COLLECT)
        {
            MY_LOGD("[%d] add metadata", callback->getSlot());

            IMetadata metadata = appMetadata;
            IMetadata tempMetadata = callback->getMetadata();
            metadata += callback->getMetadata();
            MRect cropSize;
            if( tryGetMetadata<MRect>(&tempMetadata, MTK_SCALER_CROP_REGION, cropSize) )
            {
                MY_LOGD("[%d]crop size (%d, %d, %d, %d)", callback->getSlot(), cropSize.p.x, cropSize.p.y, cropSize.s.w, cropSize.s.h);
            }
            else
            {
                MY_LOGE("can not get crop size");
            }
            {
                IMetadata::IEntry entry(MTK_SCALER_CROP_REGION);
                entry.push_back(cropSize, Type2Type<MRect>());
                metadata.update(MTK_SCALER_CROP_REGION, entry);
            }
            {
                // get af roi
                IMetadata::IEntry entry = tempMetadata.entryFor(MTK_3A_FEATURE_AF_ROI);
                MINT32 af_head1 = 0;
                MINT32 afroinum = 0;
                MINT32 afTopLeftX     = 0;
                MINT32 afTopLeftY     = 0;
                MINT32 afBottomRightX = 0;
                MINT32 afBottomRightY = 0;
                MINT32 aftype = 0;
                if(!entry.isEmpty())
                {
                    af_head1         = entry.itemAt(0, Type2Type<MINT32>());
                    afroinum         = entry.itemAt(1, Type2Type<MINT32>());
                    afTopLeftX       = entry.itemAt(2, Type2Type<MINT32>());
                    afTopLeftY       = entry.itemAt(3, Type2Type<MINT32>());
                    afBottomRightX   = entry.itemAt(4, Type2Type<MINT32>());
                    afBottomRightY   = entry.itemAt(5, Type2Type<MINT32>());
                    aftype           = entry.itemAt(6, Type2Type<MINT32>());

                    MY_LOGD("preview merr af_head1 = %d, afroinum = %d, afTopLeftX(%d), afTopLeftY(%d), afBottomRightX(%d), afBottomRightY(%d), aftype(%d)",
                            af_head1, afroinum, afTopLeftX, afTopLeftY, afBottomRightX, afBottomRightY, aftype);
                    metadata.update(entry.tag(), entry);
                }
                else
                {
                    MY_LOGW("Get AF roi fail preview, set to 0");
                    IMetadata::IEntry entry(MTK_3A_FEATURE_AF_ROI);
                    for(int i=0 ; i < 6 ; i ++)
                    {
                        entry.push_back(0, Type2Type< MINT32 >());
                    }
                    metadata.update(entry.tag(), entry);
                }
                MUINT8 afstate = 0;
                if(tryGetMetadata<MUINT8>(&tempMetadata, MTK_CONTROL_AF_STATE, afstate) )
                {
                    IMetadata::IEntry entry(MTK_CONTROL_AF_STATE);
                    entry.push_back(afstate, Type2Type<MUINT8>());
                    metadata.update(entry.tag(), entry);
                    MY_LOGD("preview merr afstate = %d", afstate);
                }
                //get af distance
                MFLOAT afdistance = 0;
                if(tryGetMetadata<MFLOAT>(&tempMetadata, MTK_LENS_FOCUS_DISTANCE, afdistance) )
                {
                    IMetadata::IEntry entry(MTK_LENS_FOCUS_DISTANCE);
                    entry.push_back(afdistance, Type2Type<MFLOAT>());
                    metadata.update(entry.tag(), entry);
                    MY_LOGD("preview afdistance = %f", afdistance);
                }
            }
            IMetadata halMetadata = result;
            {
                // set open id to hal metadata
                MINT32 main1Id, main2Id;
                if(!StereoSettingProvider::getStereoSensorIndex(main1Id, main2Id))
                {
                    MY_LOGE("cannot get sensor id");
                }
                MINT32 value = (callback->getSlot() == 0) ? main1Id : main2Id;
                IMetadata::IEntry entry(MTK_STEREO_FEATURE_OPEN_ID);
                entry.push_back(value, Type2Type<MINT32>());
                halMetadata.update(MTK_STEREO_FEATURE_OPEN_ID, entry);
            }
            mPostProcess_DataQueue->mvInputData[0].mAppMetadataQueue[callback->getSlot()] = metadata;
            mPostProcess_DataQueue->mvInputData[0].mHalMetadataQueue[callback->getSlot()] = halMetadata;
        }
        // notify to flow control
        sp<ICaptureRequestCB> pCB = mpCb.promote();
        if(pCB != nullptr)
        {
                pCB->onEvent(
                            miCaptureNo,
                            callback->getCBType(),
                            streamId,
                            errorResult);
        }
        if(errorResult)
        {
            callback->processFailData(requestNo, streamId, result);
        }
        else
        {
            callback->sendCallback(mpShotCallback, streamId, result);
        }
        miMetaCount++;

        mbMetaPushDone = (miMetaCount >= miTotalMetaCount);
        MY_LOGD("miMetaCount(%d) miTotalMetaCount(%d)", miMetaCount, miTotalMetaCount);

        if(mbMetaPushDone)
        {
            MY_LOGD("metadata ready for post processor");
            readyToPostProcessor();
            if(mbEnqueDone)
                sendDoneCallback();
        }
    }
    FUNC_END;
}
/******************************************************************************
 * image event handler
 ******************************************************************************/
MERROR
ShotCallbackProcessor::
onResultReceived(
    MUINT32    const requestNo,
    StreamId_T const streamId,
    MBOOL      const errorBuffer,
    android::sp<IImageBuffer>& pBuffer
)
{
    Mutex::Autolock _l(mImgResultLock);
    FUNC_START;
    MY_LOGD("[%d] image request(%d) streamID(%llx)", errorBuffer, requestNo, streamId);
    CHECK_OBJECT(pBuffer);
    //
    sp<IImageShotCallback> callback = nullptr;
    if(mvImageCallbackPool.getCallback(streamId, callback))
    {
        if(callback == nullptr)
            return UNKNOWN_ERROR;
        // check need to count image.
        MBOOL isFinalImage = MFALSE;
        if(callback->isNeedImageCount())
        {
            miImgCount++;
            MY_LOGD("miImgCount = %d, miTotalImgCount = %d", miImgCount, miTotalImgCount);
            if(miImgCount == miTotalImgCount)
            {
                isFinalImage = MTRUE;
            }
        }
        //
        callback->setImgBufferErrorType(errorBuffer);
        //
        if(callback->isNeedCaptureDoneCb())
        {
            mImgCallbackQue.push_back(callback);
            mImgBufferQue.push_back(pBuffer);
            if(isFinalImage)
            {
                // notify to flow control
                sp<ICaptureRequestCB> pCB = mpCb.promote();
                while(!mImgCallbackQue.empty())
                {
                    sp<IImageShotCallback> pImgShotCb = mImgCallbackQue.front();
                    android::sp<IImageBuffer> pImgBuf = mImgBufferQue.front();
                    sendImageCallback(pImgShotCb, pImgBuf);
                    // push image to post processor image queue
                    if(mPostProcess_DataQueue->mProcessType != PostProcessorType::NONE)
                    {
                        pushToPostProcQueue(pImgShotCb);
                        if(mImgCallbackQue.size() == 1)
                        {
                            mbImgPushDone = MTRUE;
                            MY_LOGD("image buffer ready for post processor");
                            // add for post 3rd party
                            mPostProcess_DataQueue->mRequestNo = requestNo;
                            MY_LOGD("Frame size = %d", mPostProcess_DataQueue->mvInputData[0].mImageQueue.size());
                            for(MUINT32 i=0; i<mPostProcess_DataQueue->mvInputData[0].mImageQueue.size() ; i++)
                            {
                                MY_LOGD("Frame [%d].size() = %d", i, mPostProcess_DataQueue->mvInputData[0].mImageQueue[i].size());
                            }
                            readyToPostProcessor();
                            sendEvent(streamId, errorBuffer, mbEnqueDone, callback);
                            MY_LOGD("last image frame streamId(%llx) err(%d)",
                                                    pImgShotCb->getStreamId(),
                                                    pImgShotCb->getImgBufferErrorType());
                        }
                    }
                    else
                    {
                        MY_LOGD("send done callback");
                        sendEvent(streamId, errorBuffer, (MBOOL)(mImgCallbackQue.size() == 1), callback);
                    }
                    mImgCallbackQue.pop_front();
                    mImgBufferQue.pop_front();
                }
            }
        }
        else
        {
            sendEvent(streamId, errorBuffer, isFinalImage, callback);
            callbackImage(requestNo, streamId, errorBuffer, pBuffer, isFinalImage, callback);
        }
    }
    FUNC_END;
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
String8
ShotCallbackProcessor::
getUserName()
{
    return String8(LOG_TAG);
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
ShotCallbackProcessor::
readyToPostProcessor()
{
    Mutex::Autolock _l(mPostProcessorLock);
    FUNC_START;
    if(!mbEnqueDone && mbImgPushDone && mbMetaPushDone)
    {
        MY_LOGD("---- start ----");
        for(auto &meta : mPostProcess_DataQueue->mvInputData[0].mHalMetadataQueue)
        {
             IMetadata::IEntry entry(MTK_STEREO_FEATURE_SENSOR_PROFILE);
             entry.push_back(StereoSettingProvider::stereoProfile(), Type2Type< MINT32 >());
             meta.update(MTK_STEREO_FEATURE_SENSOR_PROFILE, entry);
        }

        // for post 3rdparty
        IImagePostProcessManager* pIIPPManager = IImagePostProcessManager::getInstance();
        if(pIIPPManager != nullptr)
        {
            MY_LOGD("before enque");
            pIIPPManager->enque(mPostProcess_DataQueue);
            MY_LOGD("after enque");
        }
        else
        {
            MY_LOGE("Cannot get IImagePostProcessManager instance.");
        }
        mbEnqueDone = MTRUE;
        MY_LOGD("---- done ----");
    }
    FUNC_END;
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
void
ShotCallbackProcessor::
prepareMetaCheckSet(
)
{
    appMetaDefinitionSet.clear();
    halMetaDefinitionSet.clear();

    halMetaDefinitionSet.insert(eSTREAMID_META_HAL_BEGIN);
    halMetaDefinitionSet.insert(eSTREAMID_META_HAL_CONTROL);
    halMetaDefinitionSet.insert(eSTREAMID_META_HAL_DYNAMIC_P1);
    halMetaDefinitionSet.insert(eSTREAMID_META_HAL_DYNAMIC_P2);
    halMetaDefinitionSet.insert(eSTREAMID_META_HAL_DYNAMIC_HDR);
    halMetaDefinitionSet.insert(eSTREAMID_META_HAL_CONTROL_MAIN2);
    halMetaDefinitionSet.insert(eSTREAMID_META_HAL_DYNAMIC_P1_MAIN2);
    halMetaDefinitionSet.insert(eSTREAMID_META_HAL_DYNAMIC_P2_MAIN2);
    halMetaDefinitionSet.insert(eSTREAMID_META_HAL_DYNAMIC_DEPTH);
    halMetaDefinitionSet.insert(eSTREAMID_META_HAL_DYNAMIC_BOKEH);
    halMetaDefinitionSet.insert(eSTREAMID_META_HAL_DYNAMIC_BMDENOISE);
    halMetaDefinitionSet.insert(eSTREAMID_META_HAL_DYNAMIC_BMPREPROCESS);
    halMetaDefinitionSet.insert(eSTREAMID_META_HAL_DYNAMIC_STEREOROOT);
    halMetaDefinitionSet.insert(eSTREAMID_META_HAL_DYNAMIC_DUALYUV);
    halMetaDefinitionSet.insert(eSTREAMID_META_HAL_DYNAMIC_TP);
    halMetaDefinitionSet.insert(eSTREAMID_META_HAL_FULL);
    halMetaDefinitionSet.insert(eSTREAMID_META_HAL_DYNAMIC_MFLL);

    appMetaDefinitionSet.insert(eSTREAMID_META_APP_BEGIN);
    appMetaDefinitionSet.insert(eSTREAMID_META_APP_CONTROL);
    appMetaDefinitionSet.insert(eSTREAMID_META_APP_DYNAMIC_P1);
    appMetaDefinitionSet.insert(eSTREAMID_META_APP_DYNAMIC_P2);
    appMetaDefinitionSet.insert(eSTREAMID_META_APP_DYNAMIC_FD);
    appMetaDefinitionSet.insert(eSTREAMID_META_APP_DYNAMIC_JPEG);
    appMetaDefinitionSet.insert(eSTREAMID_META_APP_DYNAMIC_HDR);
    appMetaDefinitionSet.insert(eSTREAMID_META_APP_CONTROL_MAIN2);
    appMetaDefinitionSet.insert(eSTREAMID_META_APP_DYNAMIC_P1_MAIN2);
    appMetaDefinitionSet.insert(eSTREAMID_META_APP_DYNAMIC_P2_MAIN2);
    appMetaDefinitionSet.insert(eSTREAMID_META_APP_DYNAMIC_DEPTH);
    appMetaDefinitionSet.insert(eSTREAMID_META_APP_DYNAMIC_FDYUV);
    appMetaDefinitionSet.insert(eSTREAMID_META_APP_DYNAMIC_BOKEH);
    appMetaDefinitionSet.insert(eSTREAMID_META_APP_DYNAMIC_DUALIT);
    appMetaDefinitionSet.insert(eSTREAMID_META_APP_DYNAMIC_BMDENOISE);
    appMetaDefinitionSet.insert(eSTREAMID_META_APP_DYNAMIC_BMPREPROCESS);
    appMetaDefinitionSet.insert(eSTREAMID_META_APP_DYNAMIC_BOKEH_JPG);
    appMetaDefinitionSet.insert(eSTREAMID_META_APP_DYNAMIC_JPS);
    appMetaDefinitionSet.insert(eSTREAMID_META_APP_DYNAMIC_DUALYUV);
    appMetaDefinitionSet.insert(eSTREAMID_META_APP_DYNAMIC_TP);
    appMetaDefinitionSet.insert(eSTREAMID_META_APP_DYNAMIC_TP_JPEG0);
    appMetaDefinitionSet.insert(eSTREAMID_META_APP_FULL);
    appMetaDefinitionSet.insert(eSTREAMID_META_APP_DYNAMIC_MFLL);

    return;
}
/******************************************************************************
 *
 ******************************************************************************/
void
ShotCallbackProcessor::
checkMetadataType(
    MINT32 streamId,
    MBOOL& isAppMeta
)
{
    MY_LOGD("streamId = %llx", streamId);
    auto search = this->appMetaDefinitionSet.find(streamId);
    if(search != this->appMetaDefinitionSet.end())
    {
        MY_LOGD("isAppMeta = true");
        isAppMeta = MTRUE;
    }
    else
    {
        MY_LOGD("isAppMeta = false");
        isAppMeta = MFALSE;
    }
}
/******************************************************************************
 *
 ******************************************************************************/
void
ShotCallbackProcessor::
sendImageCallback(
    sp<IImageShotCallback> &pImgShotCb,
    android::sp<IImageBuffer> &pImgBuf
)
{
    if(pImgShotCb == nullptr || pImgBuf == nullptr)
    {
        MY_LOGE("pImgShotCb or pImgBuf may nullptr");
        return;
    }
    if(pImgShotCb->getImgBufferErrorType())
    {
        pImgShotCb->processFailData(
                    miCaptureNo,
                    pImgShotCb->getStreamId(),
                    pImgBuf);
    }
    else
    {
        pImgShotCb->sendCallback(
                    mpShotCallback,
                    pImgShotCb->getStreamId(),
                    pImgBuf,
                    (mImgCallbackQue.size() == 1));
    }
}
/******************************************************************************
 *
 ******************************************************************************/
void
ShotCallbackProcessor::
pushToPostProcQueue(
    sp<IImageShotCallback> &pImgShotCb
)
{
    if((MINT32)pImgShotCb->getSlot() > ((MINT32)mPostProcess_DataQueue->mvInputData[0].mImageQueue.size() - 1))
    {
        for(MUINT32 j=mPostProcess_DataQueue->mvInputData[0].mImageQueue.size();j<=pImgShotCb->getSlot();j++)
        {
            MY_LOGD("create new slot frame(%d)", j);
            android::DefaultKeyedVector<MINT32, android::sp<IImageBuffer>> queue;
            mPostProcess_DataQueue->mvInputData[0].mImageQueue.push_back(queue);
        }
    }
    MY_LOGD("slot[%d] streamid[%llx] buffer[%x] heap[%x]",
                                            pImgShotCb->getSlot(),
                                            pImgShotCb->getStreamId(),
                                            pImgShotCb->getBuffer().get(),
                                            pImgShotCb->getBuffer()->getImageBufferHeap());
    mPostProcess_DataQueue->mvInputData[0].mImageQueue[pImgShotCb->getSlot()].add(
                                                        pImgShotCb->getStreamId(),
                                                        pImgShotCb->getBuffer());
}
/******************************************************************************
 *
 ******************************************************************************/
void
ShotCallbackProcessor::
sendEvent(
    StreamId_T const &streamId,
    MBOOL      const &errorBuffer,
    MBOOL isFinalImage,
    sp<IImageShotCallback> &pImgShotCb
)
{
    if(isFinalImage)
    {
        sendDoneCallback();
    }
    else
    {
        sp<ICaptureRequestCB> pCB = mpCb.promote();
        if(pCB != nullptr)
        {
            pCB->onEvent(
                        miCaptureNo,
                        pImgShotCb->getCBType(),
                        pImgShotCb->getStreamId(),
                        pImgShotCb->getImgBufferErrorType());
        }
    }
}
/******************************************************************************
 *
 ******************************************************************************/
void
ShotCallbackProcessor::
sendDoneCallback()
{
    MY_LOGD("+");
    sp<ICaptureRequestCB> pCB = mpCb.promote();
    if(pCB != nullptr)
    {
        pCB->onEvent(
                    miCaptureNo,
                    CaptureRequestCB::CAPTURE_DONE,
                    0,
                    0);
    }
    MY_LOGD("-");
}
/******************************************************************************
 *
 ******************************************************************************/
void
ShotCallbackProcessor::
callbackImage(
    MUINT32    const &requestNo,
    StreamId_T const &streamId,
    MBOOL      const &errorBuffer,
    android::sp<IImageBuffer>& pBuffer,
    MBOOL isFinalImage,
    sp<IImageShotCallback> &callback
)
{
    if(callback != nullptr)
    {
        if(errorBuffer)
        {
            callback->processFailData(requestNo, streamId, pBuffer);
        }
        else
        {
            callback->sendCallback(mpShotCallback, streamId, pBuffer, isFinalImage);
        }
    }
    else
    {
        MY_LOGW("image shot callback is nullptr");
    }
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
ImageCallbackPool::
addCallback(
    sp<IImageShotCallback>& callback
)
{
    ssize_t index = mvImageShotCallbackPool.indexOfKey(callback->getStreamId());
    if(index >= 0)
    {
        mvImageShotCallbackPool.replaceValueAt(index, callback);
    }
    else
    {
        mvImageShotCallbackPool.add(callback->getStreamId(), callback);
    }
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
ImageCallbackPool::
clearPool(
)
{
    mvImageShotCallbackPool.clear();
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
ImageCallbackPool::
getCallback(
    MINT32 streamId,
    sp<IImageShotCallback>& callback
)
{
    ssize_t index = mvImageShotCallbackPool.indexOfKey(streamId);
    if(index >= 0)
    {
        callback = mvImageShotCallbackPool.valueAt(index);
        return MTRUE;
    }
    else
    {
        return MFALSE;
    }
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
MetaCallbackPool::
addCallback(
    sp<IMetaShotCallback>& callback
)
{
    ssize_t index = mvMetaShotCallbackPool.indexOfKey(callback->getStreamId());
    if(index >= 0)
    {
        mvMetaShotCallbackPool.replaceValueAt(index, callback);
    }
    else
    {
        mvMetaShotCallbackPool.add(callback->getStreamId(), callback);
    }
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
MetaCallbackPool::
clearPool(
)
{
    mvMetaShotCallbackPool.clear();
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
MetaCallbackPool::
getCallback(
    MINT32 streamId,
    sp<IMetaShotCallback>& callback
)
{
    ssize_t index = mvMetaShotCallbackPool.indexOfKey(streamId);
    if(index >= 0)
    {
        callback = mvMetaShotCallbackPool.valueAt(index);
        return MTRUE;
    }
    else
    {
        return MFALSE;
    }
}