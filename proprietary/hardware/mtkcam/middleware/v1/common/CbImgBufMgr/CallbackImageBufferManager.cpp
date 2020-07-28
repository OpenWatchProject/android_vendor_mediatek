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

#define LOG_TAG "MtkCam/CallbackImageBufferManager"
//
#include "CallbackImageBufferManager.h"
//
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/StreamBufferProviderFactory.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/BufferPoolImp.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/StreamBufferProvider.h>
//
#include <mtkcam/middleware/v1/camshot/BufferCallbackHandler.h>
//
#include <mtkcam/utils/std/Log.h>
//
using namespace android;
using namespace NSCam::v1;
using namespace NSCam::v1::NSLegacyPipeline;
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
CallbackImageBufferManager::
CallbackImageBufferManager(
    MUINT32 openId,
    sp<ImageStreamManager> pImageStreamManager
)
{
    miOpenId = openId;
    mpImageStreamManager = pImageStreamManager;
    MY_LOGD("ctor(%p) openId(%d) image(%p)", this, miOpenId, mpImageStreamManager.promote().get());
}
/******************************************************************************
 *
 ******************************************************************************/
CallbackImageBufferManager::
~CallbackImageBufferManager()
{
    MY_LOGD("dtor(%p)", this);
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
CallbackImageBufferManager::
allocBuffer(
    sp<IImageStreamInfo> pImgStreamInfo
)
{
    status_t ret = UNKNOWN_ERROR;
    //
    MY_LOGD("name(%s) streamId(0x%lx) size(%dx%d)",
            pImgStreamInfo->getStreamName(),
            pImgStreamInfo->getStreamId(),
            pImgStreamInfo->getImgSize().w,
            pImgStreamInfo->getImgSize().h);
    //
    if(pImgStreamInfo!=nullptr){
        sp<StreamBufferProviderFactory> pFactory = StreamBufferProviderFactory::createInstance();
        sp<BufferPoolImp> pPool = new BufferPoolImp(pImgStreamInfo);
        pPool->allocateBuffer(
            pImgStreamInfo->getStreamName(),
            pImgStreamInfo->getMaxBufNum(),
            pImgStreamInfo->getMinInitBufNum());
        pFactory->setImageStreamInfo(pImgStreamInfo);
        pFactory->setUsersPool(pPool);
        //
        sp<StreamBufferProvider> provider = pFactory->create(miOpenId, MTRUE);
        ret = addToPool(pImgStreamInfo->getStreamId(), provider);
        sp<ImageStreamManager> pImageStreamManager = mpImageStreamManager.promote();
        if(pImageStreamManager != nullptr)
        {
            pImageStreamManager->updateBufProvider(
                                            pImgStreamInfo->getStreamId(),
                                            provider,
                                            Vector<StreamId_T>());
        }
    }
    //
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
CallbackImageBufferManager::
allocBuffer(
    sp<IImageStreamInfo> pImgStreamInfo,
    sp<BufferCallbackHandler> pCallbackHandler
)
{
    status_t ret = UNKNOWN_ERROR;
    //
    MY_LOGD("name(%s) streamId(0x%lx) size(%dx%d)",
            pImgStreamInfo->getStreamName(),
            pImgStreamInfo->getStreamId(),
            pImgStreamInfo->getImgSize().w,
            pImgStreamInfo->getImgSize().h);
    //
    if(pImgStreamInfo!=nullptr){
        sp<StreamBufferProviderFactory> pFactory = StreamBufferProviderFactory::createInstance();
        sp<CallbackBufferPool> pPool = new CallbackBufferPool(pImgStreamInfo);
        pPool->allocateBuffer(
            pImgStreamInfo->getStreamName(),
            pImgStreamInfo->getMaxBufNum(),
            pImgStreamInfo->getMinInitBufNum());
        pCallbackHandler->setBufferPool(pPool);
        pFactory->setImageStreamInfo(pImgStreamInfo);
        pFactory->setUsersPool(pPool);
        //
        sp<StreamBufferProvider> provider = pFactory->create(miOpenId, MTRUE);
        ret = addToPool(pImgStreamInfo->getStreamId(), provider);
        sp<ImageStreamManager> pImageStreamManager = mpImageStreamManager.promote();
        if(pImageStreamManager != nullptr)
        {
            pImageStreamManager->updateBufProvider(
                                            pImgStreamInfo->getStreamId(),
                                            provider,
                                            Vector<StreamId_T>());
        }
    }
    //
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
sp<StreamBufferProvider>
CallbackImageBufferManager::
getBufferProvider(
    StreamId_T const streamId
)
{
    ssize_t index = mvBufferProviderPool.indexOfKey(streamId);
    if(index >= 0)
    {
        return mvBufferProviderPool.valueAt(index);
    }
    MY_LOGW("Can not find buffer provider. streamId(0x%lx)", streamId);
    return nullptr;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
CallbackImageBufferManager::
setTimestamp(
    MUINT32 requestNo,
    MINT64 timestamp
)
{
    status_t ret = UNKNOWN_ERROR;
    for(size_t i = 0;i<mvBufferProviderPool.size();++i)
    {
        mvBufferProviderPool.editValueAt(i)->doTimestampCallback(requestNo, MFALSE, timestamp);
    }
    ret = OK;
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
CallbackImageBufferManager::
destory(
)
{
    status_t ret = UNKNOWN_ERROR;
    mvBufferProviderPool.clear();
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
CallbackImageBufferManager::
addToPool(
    StreamId_T streamId,
    sp<StreamBufferProvider> provider
)
{
    // add to pool
    ssize_t index = mvBufferProviderPool.indexOfKey(streamId);
    if(index >= 0)
    {
        MY_LOGD("edit buffer provider (0x%lx)", streamId);
        mvBufferProviderPool.replaceValueAt(index, provider);
    }
    else
    {
        MY_LOGD("add new buffer provider (0x%lx)", streamId);
        mvBufferProviderPool.add(streamId, provider);
    }
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
DefaultKeyedVector<StreamId_T, sp<StreamBufferProvider> >
CallbackImageBufferManager::
getBufferProviderPool(
)
{
    return mvBufferProviderPool;
}