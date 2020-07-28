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

#include "MyUtils.h"
#include "AppStreamMgr.h"
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;

#define ThisNamespace   AppStreamMgr::BufferHandleCache

/******************************************************************************
 *
 ******************************************************************************/
#define MY_DEBUG(level, fmt, arg...) \
    do { \
        CAM_LOG##level("[%s::%s] " fmt, mInstanceName.c_str(), __FUNCTION__, ##arg); \
        mCommonInfo->mDebugPrinter->printFormatLine(#level" [%s::%s] " fmt, mInstanceName.c_str(), __FUNCTION__, ##arg); \
    } while(0)

#define MY_WARN(level, fmt, arg...) \
    do { \
        CAM_LOG##level("[%s::%s] " fmt, mInstanceName.c_str(), __FUNCTION__, ##arg); \
        mCommonInfo->mWarningPrinter->printFormatLine(#level" [%s::%s] " fmt, mInstanceName.c_str(), __FUNCTION__, ##arg); \
    } while(0)

#define MY_ERROR(level, fmt, arg...) \
    do { \
        CAM_LOG##level("[%s::%s] " fmt, mInstanceName.c_str(), __FUNCTION__, ##arg); \
        mCommonInfo->mErrorPrinter->printFormatLine(#level" [%s::%s] " fmt, mInstanceName.c_str(), __FUNCTION__, ##arg); \
    } while(0)

#define MY_LOGV(...)                MY_DEBUG(V, __VA_ARGS__)
#define MY_LOGD(...)                MY_DEBUG(D, __VA_ARGS__)
#define MY_LOGI(...)                MY_DEBUG(I, __VA_ARGS__)
#define MY_LOGW(...)                MY_WARN (W, __VA_ARGS__)
#define MY_LOGE(...)                MY_ERROR(E, __VA_ARGS__)
#define MY_LOGA(...)                MY_ERROR(A, __VA_ARGS__)
#define MY_LOGF(...)                MY_ERROR(F, __VA_ARGS__)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#if 0
#define FUNC_START     MY_LOGD("+")
#define FUNC_END       MY_LOGD("-")
#else
#define FUNC_START
#define FUNC_END
#endif


/******************************************************************************
 *
 ******************************************************************************/
static
auto
queryIonFd(buffer_handle_t bufferHandle) -> int
{
    GrallocStaticInfo staticInfo;
    GrallocDynamicInfo dynamicInfo;
    auto helper = IGrallocHelper::singleton();
    if  ( ! helper || 0 != helper->query(bufferHandle, &staticInfo, &dynamicInfo) || dynamicInfo.ionFds.empty() )
    {
        return -1;
    }

    return dynamicInfo.ionFds[0];
}


/******************************************************************************
 *
 ******************************************************************************/
static
auto
mapBufferHandle(buffer_handle_t& bufferHandle) -> void
{
    auto helper = IGrallocHelper::singleton();
    if  ( ! helper || ! helper->importBuffer(bufferHandle) || bufferHandle == nullptr )
    {
        bufferHandle = nullptr;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
static
auto
unmapBufferHandle(buffer_handle_t& bufferHandle) -> void
{
    if  ( bufferHandle != nullptr )
    {
        if  ( auto helper = IGrallocHelper::singleton() ) {
            helper->freeBuffer(bufferHandle);
        }
        bufferHandle = nullptr;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
ThisNamespace::
BufferHandleCache(std::shared_ptr<CommonInfo> pCommonInfo, StreamId_T streamId)
    : mInstanceName{std::to_string(pCommonInfo->mInstanceId) + "-BufferHandleCache"}
    , mCommonInfo(pCommonInfo)
    , mStreamId(streamId)
{
}


/******************************************************************************
 *
 ******************************************************************************/
ThisNamespace::
~BufferHandleCache()
{
    try {
        FUNC_START;
        clear();
        FUNC_END;
    }
    catch (std::exception&) {
        CAM_LOGW("BufferHandleCache::clear() throws exception");
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
dumpState(android::Printer& printer, uint32_t indent) const -> void
{
    for (auto const& pair : mBufferHandleMap) {
        printer.printFormatLine("%-*cbufferId:%3" PRIu64 " -> handle:%p %3d ionFd:%3d  (%s)",
            indent, ' ', pair.second.bufferId, pair.second.bufferHandle, pair.second.bufferHandle->data[0], pair.second.ionFd,
            NSCam::Utils::LogTool::get()->convertToFormattedLogTime(&pair.second.cachedTime).c_str()
        );
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
importBuffer(
    uint64_t bufferId,
    buffer_handle_t& importedBufferHandle
)   const -> int
{
    FUNC_START;
    auto it = mBufferHandleMap.find(bufferId);
    if ( it == mBufferHandleMap.end() ) {
        if ( importedBufferHandle == nullptr ) {
            MY_LOGE("bufferId %" PRIu64 " has null buffer handle!", bufferId);
            return -EINVAL;
        }

        mapBufferHandle(importedBufferHandle);
    }
    else if ( importedBufferHandle != nullptr ) {
        MY_LOGE("bufferId %" PRIu64 ": has a cached buffer handle:%p != %p",
            bufferId, it->second.bufferHandle, importedBufferHandle);
        return -ENODEV;
    }
    else {
        importedBufferHandle = it->second.bufferHandle;
    }
    //
    if ( importedBufferHandle == nullptr ) {
        MY_LOGE("bufferId %" PRIu64 ": fail to import", bufferId);
        return -ENODEV;
    }
    // MY_LOGD("bufferId %" PRIu64 ": %p", bufferId, importedBufferHandle);
    //
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
cacheBuffer(
    uint64_t bufferId,
    buffer_handle_t importedBufferHandle
)   -> int
{
    FUNC_START;
    auto it = mBufferHandleMap.find(bufferId);
    if ( it == mBufferHandleMap.end() ) {
        mBufferHandleMap[bufferId] = {
            .bufferId = bufferId,
            .bufferHandle = importedBufferHandle,
            .ionFd = queryIonFd(importedBufferHandle),
        };
        NSCam::Utils::LogTool::get()->getCurrentLogTime(&mBufferHandleMap[bufferId].cachedTime);
    }
    else if (it->second.bufferHandle != importedBufferHandle) {
        MY_LOGE("bufferId %" PRIu64 ": has a cached buffer handle:%p != %p",
            bufferId, it->second.bufferHandle, importedBufferHandle);
        return -ENODEV;
    }
    //MY_LOGD("bufferId %-3" PRIu64 " -> %p %3d", bufferId, importedBufferHandle, importedBufferHandle->data[0]);
    //
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
uncacheBuffer(uint64_t bufferId) -> void
{
    FUNC_START;
    auto it = mBufferHandleMap.find(bufferId);
    if ( it == mBufferHandleMap.end() ) {
        MY_LOGW("bufferId:%" PRIu64 ": has not been cached!", bufferId);
    }
    else {
        //MY_LOGD("bufferId %-3" PRIu64 " -> %p %3d", bufferId, it->second.bufferHandle, it->second.bufferHandle->data[0]);
        unmapBufferHandle(it->second.bufferHandle);
        mBufferHandleMap.erase(it);
    }
    FUNC_END;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
clear() -> void
{
    FUNC_START;
    MY_LOGD("streamId:%02" PRIu64 " + mBufferHandleMap.size#:%zu", mStreamId, mBufferHandleMap.size());
    for (auto& pair : mBufferHandleMap) {
        //MY_LOGD("bufferId %-3" PRIu64 " -> %p %3d", pair.first, pair.second.bufferHandle, pair.second.bufferHandle->data[0]);
        unmapBufferHandle(pair.second.bufferHandle);
    }
    mBufferHandleMap.clear();
    FUNC_END;
}

