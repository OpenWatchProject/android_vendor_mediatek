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
// #if (MTKCAM_HAVE_AEE_FEATURE == 1)
#if 0
#include <aee.h>
#define AEE_ASSERT(fmt, arg...) \
    do { \
        android::String8 const str = android::String8::format(fmt, ##arg); \
        CAM_LOGE("ASSERT(%s) fail", str.string()); \
        aee_system_exception( \
            "mtkcam/Metadata", \
            NULL, \
            DB_OPT_DEFAULT, \
            str.string()); \
    } while(0)
#else
#define AEE_ASSERT(fmt, arg...)
#endif
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;

#define ThisNamespace   AppStreamMgr::FrameHandler

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


/******************************************************************************
 *
 ******************************************************************************/
static auto stateToText(ThisNamespace::State::T state) -> android::String8
{
    switch (state) {
    case ThisNamespace::State::IN_FLIGHT:   return String8("in-flight");
    case ThisNamespace::State::PRE_RELEASE: return String8("pre-release");
    case ThisNamespace::State::VALID:       return String8("valid");
    case ThisNamespace::State::ERROR:       return String8("error");
    }
    return String8("never happen");
};


/******************************************************************************
 *
 ******************************************************************************/
static auto historyToText (android::BitSet32 v) -> android::String8
{
    String8 s("");
    if (v.hasBit(ThisNamespace::HistoryBit::RETURNED)) {
        s += "returned";
    }
    if (v.hasBit(ThisNamespace::HistoryBit::ERROR_SENT_FRAME)) {
        s += "error-sent-frame";
    }
    if (v.hasBit(ThisNamespace::HistoryBit::ERROR_SENT_META)) {
        s += "error-sent-meta";
    }
    if (v.hasBit(ThisNamespace::HistoryBit::ERROR_SENT_IMAGE)) {
        s += "error-sent-image";
    }
    return s;
};


/******************************************************************************
 *
 ******************************************************************************/
ThisNamespace::
FrameHandler(std::shared_ptr<CommonInfo> pCommonInfo, android::sp<BatchHandler> pBatchHandler)
    : RefBase()
    , mInstanceName{std::to_string(pCommonInfo->mInstanceId) + "-FrameHandler"}
    , mCommonInfo(pCommonInfo)
    , mBatchHandler(pBatchHandler)
{
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
destroy() -> void
{
    mBatchHandler = nullptr;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
setOperationMode(uint32_t operationMode) -> void
{
    if ( mOperationMode != operationMode ) {
        MY_LOGI("operationMode change: %#x -> %#x", mOperationMode, operationMode);
        mOperationMode = operationMode;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
removeUnusedConfigStream(std::unordered_set<StreamId_T>const& usedStreamIds) -> void
{
    for (ssize_t i = mImageConfigMap.size() - 1; i >= 0; i--)
    {
        auto const streamId = mImageConfigMap.keyAt(i);
        auto const it = usedStreamIds.find(streamId);
        if ( it == usedStreamIds.end() ) {
            //  remove unsued stream
            MY_LOGD("remove unused streamId:%02" PRIu64 "", streamId);
            mImageConfigMap.removeItemsAt(i);
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
addConfigStream(AppImageStreamInfo* pStreamInfo, bool keepBufferCache) -> void
{
    ssize_t const index = mImageConfigMap.indexOfKey(pStreamInfo->getStreamId());
    if  ( index >= 0 ) {
        auto& item = mImageConfigMap.editValueAt(index);
        if  ( keepBufferCache ) {
            item.pStreamInfo = pStreamInfo;
            item.vItemFrameQueue.clear();
        }
        else {
            item.pStreamInfo = pStreamInfo;
            item.vItemFrameQueue.clear();

            MY_LOGF_IF(item.pBufferHandleCache==nullptr, "streamId:%#" PRIx64 " has no buffer handle cache", pStreamInfo->getStreamId());
            item.pBufferHandleCache->clear();
        }
    }
    else {
        mImageConfigMap.add(
            pStreamInfo->getStreamId(),
            ImageConfigItem{
                .pStreamInfo = pStreamInfo,
                .pBufferHandleCache = std::make_shared<BufferHandleCache>(mCommonInfo, pStreamInfo->getStreamId()),
        });
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
addConfigStream(AppMetaStreamInfo* pStreamInfo) -> void
{
    MetaConfigItem item;
    item.pStreamInfo = pStreamInfo;
    mMetaConfigMap.add(pStreamInfo->getStreamId(), item);
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
getConfigImageStream(StreamId_T streamId) const -> android::sp<AppImageStreamInfo>
{
    ssize_t const index = mImageConfigMap.indexOfKey(streamId);
    if  ( 0 <= index ) {
        return mImageConfigMap.valueAt(index).pStreamInfo;
    }
    return nullptr;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
getConfigMetaStream(size_t index) const -> sp<AppStreamMgr::AppMetaStreamInfo>
{
    return mMetaConfigMap[index].pStreamInfo;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
importBuffer(StreamId_T streamId, uint64_t bufferId, buffer_handle_t& importedBufferHandle) -> int
{
    ssize_t const index = mImageConfigMap.indexOfKey(streamId);
    if  ( 0 > index ) {
        MY_LOGE("bad streamId:%#" PRIx64 ": bufferId:%" PRIu64 " buffer handle:%p", streamId, bufferId, importedBufferHandle);
        return NAME_NOT_FOUND;
    }

    if ( auto pBufferHandleCache = mImageConfigMap.valueAt(index).pBufferHandleCache ) {
        return pBufferHandleCache->importBuffer(bufferId, importedBufferHandle);
    }

    MY_LOGE("BufferHandleCache==nullptr - streamId:%#" PRIx64 ": bufferId:%" PRIu64 " buffer handle:%p", streamId, bufferId, importedBufferHandle);
    return -ENODEV;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
uncacheBuffer(StreamId_T streamId, uint64_t bufferId) -> void
{
    ssize_t const index = mImageConfigMap.indexOfKey(streamId);
    if  ( 0 > index ) {
        MY_LOGE("bad streamId:%#" PRIx64 ": bufferId:%" PRIu64, streamId, bufferId);
        return;
    }

    if ( auto pBufferHandleCache = mImageConfigMap.valueAt(index).pBufferHandleCache ) {
        pBufferHandleCache->uncacheBuffer(bufferId);
    }
    else {
        MY_LOGE("bad streamId:%#" PRIx64 " has no buffer handle cache - bufferId:%" PRIu64, streamId, bufferId);
    }
}



/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
cacheBuffer(StreamId_T streamId, uint64_t bufferId, buffer_handle_t& handle) -> void
{
    ssize_t const index = mImageConfigMap.indexOfKey(streamId);
    if  ( 0 > index ) {
        MY_LOGE("bad streamId:%#" PRIx64 ": bufferId:%" PRIu64, streamId, bufferId);
        return;
    }

    if ( auto pBufferHandleCache = mImageConfigMap.valueAt(index).pBufferHandleCache ) {
        pBufferHandleCache->cacheBuffer(bufferId, handle);
    }
    else {
        MY_LOGE("bad streamId:%#" PRIx64 " has no buffer handle cache - bufferId:%" PRIu64, streamId, bufferId);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
registerFrame(Request const& rRequest) -> int
{
    sp<FrameParcel> pFrame = new FrameParcel;
    mFrameQueue.push_back(pFrame);
    pFrame->frameNo = rRequest.frameNo;
    if (auto pTool = NSCam::Utils::LogTool::get()) {
        pTool->getCurrentLogTime(&pFrame->requestTimestamp);
    }
    //
    //  Request::vInputImageBuffers
    //  Request::vOutputImageBuffers
    {
        registerStreamBuffers(rRequest.vOutputImageBuffers, pFrame, &pFrame->vOutputImageItem);
        registerStreamBuffers(rRequest.vInputImageBuffers,  pFrame, &pFrame->vInputImageItem);
    }
    //
    //  Request::vInputMetaBuffers (Needn't register)
    //  Request::vOutputMetaBuffers
    {
        //registerStreamBuffers(rRequest.vOutputMetaBuffers, pFrame, &pFrame->vOutputMetaItem);
        registerStreamBuffers(rRequest.vInputMetaBuffers,  pFrame, &pFrame->vInputMetaItem);
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
registerStreamBuffers(
    android::KeyedVector<
        StreamId_T,
        android::sp<IImageStreamBuffer>
                        > const& buffers,
    android::sp<FrameParcel> const pFrame,
    ImageItemSet*const pItemSet
)   -> int
{
    //  Request::v[Input|Output]ImageBuffers
    //  -> FrameParcel
    //  -> mImageConfigMap::vItemFrameQueue
    //  -> mImageConfigMap::pBufferHandleCache
    //
    for (size_t i = 0; i < buffers.size(); i++)
    {
        sp<AppImageStreamBuffer> const pBuffer = static_cast<AppImageStreamBuffer*>(buffers[i].get());
        //
        StreamId_T const streamId = buffers.keyAt(i);
        //
        ssize_t const index = mImageConfigMap.indexOfKey(streamId);
        if  ( 0 > index ) {
            MY_LOGE("[frameNo:%u] bad streamId:%#" PRIx64, pFrame->frameNo, streamId);
            return NAME_NOT_FOUND;
        }

        {// cache buffer handle if needed.
            cacheBuffer(streamId, pBuffer->getBufferId(), *pBuffer->getImageBufferHeap()->getBufferHandlePtr());
        }

        {// add inflight ImageItem
            ImageItemFrameQueue& rItemFrameQueue = mImageConfigMap.editValueAt(index).vItemFrameQueue;
            //
            sp<ImageItem> pItem = new ImageItem;
            //
            rItemFrameQueue.push_back(pItem);
            //
            pItem->pFrame = pFrame.get();
            pItem->pItemSet = pItemSet;
            pItem->buffer = pBuffer;
            pItem->iter = --rItemFrameQueue.end();
            //
            pItemSet->add(streamId, pItem);
        }
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
registerStreamBuffers(
    android::KeyedVector<
        StreamId_T,
        android::sp<IMetaStreamBuffer>
                        > const& buffers,
    android::sp<FrameParcel> const pFrame,
    MetaItemSet*const pItemSet
)   -> int
{
    //  Request::v[Input|Output]MetaBuffers
    //  -> FrameParcel
    //
    for (size_t i = 0; i < buffers.size(); i++)
    {
        sp<IMetaStreamBuffer> const pBuffer = buffers[i];
        //
        StreamId_T const streamId = buffers.keyAt(i);
        //
        sp<MetaItem> pItem = new MetaItem;
        pItem->pFrame = pFrame.get();
        pItem->pItemSet = pItemSet;
        pItem->buffer = pBuffer;
        //
        pItemSet->add(streamId, pItem);
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MINT
ThisNamespace::
checkRequestError(FrameParcel const& frame)
{
    /**
     * @return
     *      ==0: uncertain
     *      > 0: it is indeed a request error
     *      < 0: it is indeed NOT a request error
     */
    //  It's impossible to be a request error if:
    //  1) any valid output image streams exist, or
    //  2) all valid output meta streams exist
    //
    //[NOT a request error]
    //
    if  (
            frame.vOutputImageItem.numValidStreams > 0
        ||  (frame.vOutputMetaItem.numValidStreams == frame.vOutputMetaItem.size()
        &&   frame.vOutputMetaItem.hasLastPartial)
        )
    {
        return -1;
    }
    //
    //[A request error]
    //
    if  (
            frame.vOutputImageItem.numErrorStreams == frame.vOutputImageItem.size()
        &&  frame.vOutputMetaItem.numErrorStreams > 0
        )
    {
        return 1;
    }
    //
    //[uncertain]
    return 0;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
prepareErrorFrame(
    CallbackParcel& rCbParcel,
    android::sp<FrameParcel> const& pFrame
)   -> void
{
    rCbParcel.valid = MTRUE;
    //
    {
        CallbackParcel::Error error;
        error.errorCode = ErrorCode::ERROR_REQUEST;
        //
        rCbParcel.vError.add(error);
        //
    }
    //
    //Note:
    //FrameParcel::vInputImageItem
    //We're not sure whether input image streams are returned or not.
    //
    //FrameParcel::vOutputImageItem
    for (size_t i = 0; i < pFrame->vOutputImageItem.size(); i++) {
        prepareReturnImage(rCbParcel, pFrame->vOutputImageItem.valueAt(i));
    }
    //
    pFrame->errors.markBit(HistoryBit::ERROR_SENT_FRAME);
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
prepareErrorMetaIfPossible(
    CallbackParcel& rCbParcel,
    android::sp<MetaItem> const& pItem
)   -> void
{
    sp<FrameParcel> const pFrame = pItem->pFrame;
    if  ( ! pFrame->errors.hasBit(HistoryBit::ERROR_SENT_META) ) {
        pFrame->errors.markBit(HistoryBit::ERROR_SENT_META);
        //
        CallbackParcel::Error error;
        error.errorCode = ErrorCode::ERROR_RESULT;
        //
        rCbParcel.vError.add(error);
        rCbParcel.valid = MTRUE;
    }
    //
    //  Actually, this item will be set to NULL, and it is not needed for
    //  the following statements.
    //
    pItem->history.markBit(HistoryBit::ERROR_SENT_META);
    //
    if  ( 0 == pFrame->shutterTimestamp ) {
        MY_LOGW("[frameNo:%u] CAMERA3_MSG_ERROR_RESULT with shutter timestamp = 0", pFrame->frameNo);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
prepareErrorImage(
    CallbackParcel& rCbParcel,
    android::sp<ImageItem> const& pItem
)   -> void
{
    auto const& pStreamInfo = pItem->buffer->getStreamInfo();
    if ( pStreamInfo == nullptr ) {
        MY_LOGE("pItem->buffer->getStreamInfo() return nullptr");
        return;
    }

    rCbParcel.valid = MTRUE;
    //
    {
        StreamId_T const streamId = pStreamInfo->getStreamId();
        ImageConfigItem const& rConfigItem = mImageConfigMap.valueFor(streamId);
        //
        CallbackParcel::Error error;
        error.errorCode = ErrorCode::ERROR_BUFFER;
        error.stream = rConfigItem.pStreamInfo;
        //
        rCbParcel.vError.add(error);
        MY_LOGW_IF(1, "(Error Status) streamId:%#" PRIx64 "(%s)", streamId, rConfigItem.pStreamInfo->getStreamName());
    }
    //
    //  Actually, this item will be set to NULL, and it is not needed for
    //  the following statements.
    //
    pItem->history.markBit(HistoryBit::ERROR_SENT_IMAGE);
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
prepareShutterNotificationIfPossible(
    CallbackParcel& rCbParcel,
    android::sp<MetaItem> const& pItem
)   -> bool
{
    sp<FrameParcel> const pFrame = pItem->pFrame;
    if  ( 0 == pFrame->shutterTimestamp ) {
        IMetadata* pMetadata = pItem->buffer->tryReadLock(LOG_TAG);
        if ( pMetadata ) {
            IMetadata::IEntry const entry = pMetadata->entryFor(MTK_SENSOR_TIMESTAMP);
            pItem->buffer->unlock(LOG_TAG, pMetadata);
            //
            if  ( ! entry.isEmpty() && entry.tag() == MTK_SENSOR_TIMESTAMP ) {
                MINT64 const timestamp = entry.itemAt(0, Type2Type<MINT64>());
                //
                pFrame->shutterTimestamp = timestamp;
                pFrame->bShutterCallbacked = true;
                //
                rCbParcel.shutter = new CallbackParcel::Shutter;
                rCbParcel.shutter->timestamp = timestamp;
                rCbParcel.valid = MTRUE;
                return MTRUE;
            }
        }
    }
    return MFALSE;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
prepareReturnMeta(
    CallbackParcel& rCbParcel,
    android::sp<MetaItem> const& pItem
)   -> void
{
    rCbParcel.valid = MTRUE;
    //
    {
        pItem->history.markBit(HistoryBit::RETURNED);
        pItem->pItemSet->numReturnedStreams++;
        //
        Vector<CallbackParcel::MetaItem>* pvCbItem = &rCbParcel.vOutputMetaItem;
        CallbackParcel::MetaItem& rCbItem = pvCbItem->editItemAt(pvCbItem->add());
        rCbItem.buffer = pItem->buffer;

        if  ( pItem->bufferNo == mCommonInfo->mAtMostMetaStreamCount ) {
            rCbItem.bufferNo = mCommonInfo->mAtMostMetaStreamCount;
            //
//#warning "[FIXME] hardcode: REQUEST_PIPELINE_DEPTH=4"
            IMetadata::IEntry entry(MTK_REQUEST_PIPELINE_DEPTH);
            entry.push_back(4, Type2Type<MUINT8>());
            //
            if ( IMetadata* pMetadata = rCbItem.buffer->tryWriteLock(LOG_TAG) ) {
                pMetadata->update(MTK_REQUEST_PIPELINE_DEPTH, entry);
                rCbItem.buffer->unlock(LOG_TAG, pMetadata);
            }
        }
        else {
            rCbItem.bufferNo = pItem->pItemSet->numReturnedStreams;
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
isShutterReturnable(
    android::sp<MetaItem> const& pItem
)   const -> bool
{
    // check this shutter can return or not
    auto itFrame = mFrameQueue.begin();
    while ( itFrame != mFrameQueue.end() ) {
        if ( (*itFrame)->frameNo != pItem->pFrame->frameNo &&
             (*itFrame)->bShutterCallbacked == false ) {
            MY_LOGW("previous shutter is not ready for frame(%u:%p)", (*itFrame)->frameNo, (*itFrame).get());
            return MFALSE;
        } else if ( (*itFrame)->frameNo == pItem->pFrame->frameNo ) {
            break;
        }
        ++itFrame;
    }
    //
    return MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
isReturnable(
    android::sp<MetaItem> const& pItem
)   const -> bool
{
    if (pItem->bufferNo == mCommonInfo->mAtMostMetaStreamCount) {
        //the final meta result to return must keep the order submitted.

        FrameQueue::const_iterator itFrame = mFrameQueue.begin();
        while (1)
        {
            sp<FrameParcel> const& pFrame = *itFrame;
            if  ( pFrame == pItem->pFrame ) {
                break;
            }
            //
//#warning "FIXME pFrame->vOutputMetaItem.isEmpty() in isReturnable()"
            if ( pFrame->vOutputMetaItem.isEmpty() ) {
                    MY_LOGW("[%d/%d] vOutputMetaItem:%zu", pFrame->frameNo, pItem->pFrame->frameNo, pFrame->vOutputMetaItem.size());
                    dumpLocked();
                    AEE_ASSERT("Skip frame %d.", pFrame->frameNo);
                return  MFALSE;
            }
            android::sp<MetaItem> const& pMetaItem = pFrame->vOutputMetaItem.valueAt(pFrame->vOutputMetaItem.size() - 1);
            if( (  pFrame->vOutputMetaItem.hasLastPartial && pMetaItem != NULL ) ||
                ( !pFrame->vOutputMetaItem.hasLastPartial/* && pMetaItem == NULL */))
            {
                MY_LOGW(
                    "Block to return the final meta of frameNo:%u since frameNo:%u (%zu|%zu) partial:%d isNULL:%d",
                    pItem->pFrame->frameNo,
                    pFrame->frameNo,
                    pFrame->vOutputMetaItem.numReturnedStreams,
                    pFrame->vOutputMetaItem.size(),
                    pFrame->vOutputMetaItem.hasLastPartial,
                    pMetaItem == NULL
                );
                return  MFALSE;
            }
            //
            ++itFrame;
        }
    }
    //
    return MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
prepareReturnImage(
    CallbackParcel& rCbParcel,
    android::sp<ImageItem> const& pItem
)   -> void
{
    auto const& pStreamInfo = pItem->buffer->getStreamInfo();
    if ( pStreamInfo == nullptr ) {
        MY_LOGE("pItem->buffer->getStreamInfo() return nullptr");
        return;
    }

    rCbParcel.valid = MTRUE;
    //
    if ( ! pItem->history.hasBit(HistoryBit::RETURNED) ) {
        pItem->history.markBit(HistoryBit::RETURNED);
        pItem->pItemSet->numReturnedStreams++;
        //
        StreamId_T const streamId = pStreamInfo->getStreamId();
        ImageConfigItem& rConfigItem = mImageConfigMap.editValueFor(streamId);
        rConfigItem.vItemFrameQueue.erase(pItem->iter);
        //
        Vector<CallbackParcel::ImageItem>* pvCbItem = ( pItem->pItemSet->asInput )
                                                    ? &rCbParcel.vInputImageItem
                                                    : &rCbParcel.vOutputImageItem
                                                    ;
        CallbackParcel::ImageItem& rCbItem = pvCbItem->editItemAt(pvCbItem->add());
        rCbItem.buffer = pItem->buffer;
        rCbItem.stream = rConfigItem.pStreamInfo;

        //CameraBlob
        if ( PixelFormat::BLOB == rCbItem.stream->getStream().format
        && ! rCbItem.buffer->hasStatus(STREAM_BUFFER_STATUS::ERROR) )
        {
            auto buffer = rCbItem.buffer;
            auto pImageBufferHeap = buffer->getImageBufferHeap();

            std::string name{std::to_string(mCommonInfo->mInstanceId) + ":AppStreamMgr:FrameHandler"};

            GrallocStaticInfo staticInfo;
            int rc = IGrallocHelper::singleton()->query(*pImageBufferHeap->getBufferHandlePtr(), &staticInfo, NULL);
            if  ( OK == rc && pImageBufferHeap->lockBuf(name.c_str(), GRALLOC_USAGE_SW_WRITE_OFTEN|GRALLOC_USAGE_SW_READ_OFTEN) ) {
                MINTPTR jpegBuf = pImageBufferHeap->getBufVA(0);
                size_t jpegDataSize = pImageBufferHeap->getBitstreamSize();
                size_t jpegBufSize = staticInfo.widthInPixels;
                CameraBlob* pTransport = reinterpret_cast<CameraBlob*>(jpegBuf + jpegBufSize - sizeof(CameraBlob));
                pTransport->blobId = CameraBlobId::JPEG;
                pTransport->blobSize = jpegDataSize;
                if ( ! pImageBufferHeap->unlockBuf(name.c_str()) ) {
                    MY_LOGE("failed on pImageBufferHeap->unlockBuf");
                }
                MY_LOGD("CameraBlob added: bufferId:%" PRIu64 " jpegBuf:%#" PRIxPTR " bufsize:%zu datasize:%zu", buffer->getBufferId(), jpegBuf, jpegBufSize, jpegDataSize);
            }
            else {
                MY_LOGE("Fail to lock jpeg buffer - rc:%d", rc);
            }
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
isReturnable(
    android::sp<ImageItem> const& pItem
)   const -> bool
{
    auto const& pStreamInfo = pItem->buffer->getStreamInfo();
    if ( pStreamInfo == nullptr ) {
        MY_LOGE("pItem->buffer->getStreamInfo() return nullptr");
        return false;
    }

    StreamId_T const streamId = pStreamInfo->getStreamId();
    ImageItemFrameQueue const& rItemFrameQueue = mImageConfigMap.valueFor(streamId).vItemFrameQueue;
    //
    ImageItemFrameQueue::const_iterator it = rItemFrameQueue.begin();
    for (; it != pItem->iter; it++) {
        if  ( State::IN_FLIGHT == (*it)->state ) {
            return false;
        }
    }
    //
    return true;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
isFrameRemovable(
    android::sp<FrameParcel> const& pFrame
)   const -> bool
{
    //Not all output image streams have been returned.
    if  ( pFrame->vOutputImageItem.size() != pFrame->vOutputImageItem.numReturnedStreams ) {
        return MFALSE;
    }
    //
    //Not all input image streams have been returned.
    if  ( pFrame->vInputImageItem.size() != pFrame->vInputImageItem.numReturnedStreams ) {
        return MFALSE;
    }
    //
    //
    if  ( pFrame->errors.hasBit(HistoryBit::ERROR_SENT_FRAME) ) {
        //frame error was sent.
        return MTRUE;
    }
    else
    if  ( pFrame->errors.hasBit(HistoryBit::ERROR_SENT_META) ) {
        //meta error was sent.
        if  ( 0 == pFrame->shutterTimestamp ) {
            MY_LOGW("[frameNo:%u] shutter not sent with meta error", pFrame->frameNo);
        }
    }
    else {
        //Not all meta streams have been returned.
        android::sp<MetaItem> const& pItem = pFrame->vOutputMetaItem.valueAt(pFrame->vOutputMetaItem.size() - 1);
        if( !pFrame->vOutputMetaItem.hasLastPartial || pItem != NULL ) {
            return MFALSE;
        }
        //
        //No shutter timestamp has been sent.
        if  ( 0 == pFrame->shutterTimestamp ) {
            MY_LOGW("[frameNo:%u] shutter not sent @ no meta error", pFrame->frameNo);
            return MFALSE;
        }
    }
    //
    return MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
prepareCallbackIfPossible(
    CallbackParcel& rCbParcel,
    MetaItemSet& rItemSet
)   -> bool
{
    MBOOL anyUpdate = MFALSE;
    //
    for ( size_t i = 0; i < rItemSet.size(); i++ )
    {
        sp<MetaItem> pItem = rItemSet[i];
        if  ( pItem == 0 ) {
            continue;
        }
        //
        sp<FrameParcel> const pFrame = pItem->pFrame;
        //
        switch  ( pItem->state )
        {
        case State::VALID:{
            //Valid Buffer but Not Returned
            if  ( ! pItem->history.hasBit(HistoryBit::RETURNED) ) {
                // separate shutter and metadata
                if ( isShutterReturnable(pItem) &&
                     prepareShutterNotificationIfPossible(rCbParcel, pItem) ) {
                    anyUpdate = MTRUE;
                }
                if  ( isReturnable(pItem) ) {
                    prepareReturnMeta(rCbParcel, pItem);
                    anyUpdate = MTRUE;
                }
            }
            }break;
            //
        case State::ERROR:{
            //Error Buffer but Not Error Sent Yet
            if  ( ! pItem->history.hasBit(HistoryBit::ERROR_SENT_META) ) {
                if  ( checkRequestError(*pFrame) < 0 ) {
                    //Not a request error
                    prepareErrorMetaIfPossible(rCbParcel, pItem);
                    anyUpdate = MTRUE;
                }
                else {
                    MY_LOGD("frameNo:%u Result Error Pending", pFrame->frameNo);
                }
            }
            }break;
            //
        default:
            break;
        }
        //
        MBOOL const needRelease =
              ( pItem->buffer->haveAllUsersReleased() == OK )
            &&( pItem->history.hasBit(HistoryBit::RETURNED)
            ||  pItem->history.hasBit(HistoryBit::ERROR_SENT_FRAME)
            ||  pItem->history.hasBit(HistoryBit::ERROR_SENT_META)
            ||  pItem->history.hasBit(HistoryBit::ERROR_SENT_IMAGE)
              ) ;
        if  ( needRelease ) {
            rItemSet.editValueAt(i) = NULL;
        }
    }
    //
    return anyUpdate;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
prepareCallbackIfPossible(
    CallbackParcel& rCbParcel,
    ImageItemSet& rItemSet
)   -> bool
{
    MBOOL anyUpdate = MFALSE;
    //
    for ( size_t i = 0; i < rItemSet.size(); i++ )
    {
        sp<ImageItem> pItem = rItemSet[i];
        if  ( pItem == 0 ) {
            continue;
        }
        //
        sp<FrameParcel> const pFrame = pItem->pFrame;
        //
        switch  ( pItem->state )
        {
        case State::PRE_RELEASE:{
            //Pre-Release but Not Returned
            if  ( ! pItem->history.hasBit(HistoryBit::RETURNED) ) {
                if  ( isReturnable(pItem) ) {
                    prepareReturnImage(rCbParcel, pItem);
                    anyUpdate = MTRUE;
                }
            }
            }break;
            //
        case State::VALID:{
            //Valid Buffer but Not Returned
            if  ( ! pItem->history.hasBit(HistoryBit::RETURNED) ) {
                if  ( isReturnable(pItem) ) {
                    prepareReturnImage(rCbParcel, pItem);
                    anyUpdate = MTRUE;
                }
            }
            }break;
            //
        case State::ERROR:{
            //Error Buffer but Not Error Sent Yet
            if  ( ! pItem->history.hasBit(HistoryBit::ERROR_SENT_IMAGE) ) {
                if  ( checkRequestError(*pFrame) < 0 ) {
                    //Not a request error
                    prepareErrorImage(rCbParcel, pItem);
                    if  ( ! pItem->history.hasBit(HistoryBit::RETURNED) ) {
                        prepareReturnImage(rCbParcel, pItem);
                    }
                    anyUpdate = MTRUE;
                }
                else {
                    MY_LOGV("frameNo:%u Buffer Error Pending, streamId:%#" PRIx64, pFrame->frameNo, pItem->buffer->getStreamInfo()->getStreamId());
                }
            }
            }break;
            //
        default:
            break;
        }
        //
        MBOOL const needRelease =
              ( pItem->buffer->haveAllUsersReleased() == OK )
            &&( pItem->history.hasBit(HistoryBit::RETURNED)
            ||  pItem->history.hasBit(HistoryBit::ERROR_SENT_FRAME)
            ||  pItem->history.hasBit(HistoryBit::ERROR_SENT_META)
            ||  pItem->history.hasBit(HistoryBit::ERROR_SENT_IMAGE)
              ) ;
        if  ( needRelease ) {
            rItemSet.editValueAt(i) = NULL;
        }
    }
    //
    return anyUpdate;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
updateItemSet(MetaItemSet& rItemSet) -> void
{
    auto pReleaseHandler = [=](
            StreamId_T const        streamId,
            MetaItem* const         pItem,
            IMetaStreamBuffer*      pStreamBuffer
        )
        {
            MBOOL const isError = pStreamBuffer->hasStatus(STREAM_BUFFER_STATUS::ERROR);
            if  ( isError ) {
                // RELEASE & ERROR BUFFER
                pItem->state = State::ERROR;
                pItem->pItemSet->numErrorStreams++;
                MY_LOGW(
                    "[Meta Stream Buffer] Error happens..."
                    " - frameNo:%u streamId:%#" PRIx64 " %s",
                    pItem->pFrame->frameNo, streamId, pStreamBuffer->getName()
                );
            }
            else {
                // RELEASE & VALID BUFFER
                pItem->state = State::VALID;
                pItem->pItemSet->numValidStreams++;
            }
        };

    for (size_t i = 0; i < rItemSet.size(); i++)
    {
        StreamId_T const streamId = rItemSet.keyAt(i);
        sp<MetaItem> pItem = rItemSet.valueAt(i);
        if  ( pItem == 0 ) {
            //MY_LOGV("Meta streamId:%#" PRIx64 " NULL MetaItem", streamId);
            continue;
        }
        //
        if  ( State::VALID != pItem->state && State::ERROR != pItem->state )
        {
            sp<IMetaStreamBuffer> pStreamBuffer = pItem->buffer;
            //
            if  (
                    pStreamBuffer->getStreamInfo()->getStreamType() != eSTREAMTYPE_META_IN
                &&  pStreamBuffer->haveAllProducerUsersReleased() == OK
                )
            {
                pReleaseHandler(streamId, pItem.get(), pStreamBuffer.get());
            }
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
updateItemSet(ImageItemSet& rItemSet) -> void
{
    auto pReleaseHandler = [=](
            StreamId_T const        streamId,
            ImageItem* const        pItem,
            AppImageStreamBuffer*   pStreamBuffer
        )
        {
            MBOOL const isError = pStreamBuffer->hasStatus(STREAM_BUFFER_STATUS::ERROR);
            if  ( isError ) {
                MY_LOGW(
                    "[Image Stream Buffer] Error happens and all users release"
                    " - frameNo:%u streamId:%#" PRIx64 " %s state:%s->ERROR",
                    pItem->pFrame->frameNo, streamId, pStreamBuffer->getName(), (( State::IN_FLIGHT == pItem->state ) ? "IN-FLIGHT" : "PRE-RELEASE")
                );
                //We should:
                //  RF = ( ACQUIRE ) ? -1 : AF
                //For simplity, however, no matter if acquire_fence was
                //waited on or not, we just:
                //  RF = AF
                MINT AF = pStreamBuffer->createAcquireFence();
                pStreamBuffer->setReleaseFence(AF);
                pStreamBuffer->setAcquireFence(-1);
                //
                pItem->state = State::ERROR;
                pItem->pItemSet->numErrorStreams++;
            }
            else {
                pStreamBuffer->setReleaseFence(-1);
                pStreamBuffer->setAcquireFence(-1);
                //
                pItem->state = State::VALID;
                pItem->pItemSet->numValidStreams++;
            }
        };

    auto pPreReleaseHandler = [=](
            StreamId_T const        streamId,
            ImageItem* const        pItem,
            AppImageStreamBuffer*   pStreamBuffer
        )
        {
            //Here the buffer status is uncertain, but its default should be OK.
            MINT RF = pStreamBuffer->createReleaseFence();
            MY_LOGW_IF(
                -1 == RF,
                "[Image Stream Buffer] pre-release but release_fence=-1 !! "
                " - frameNo:%u streamId:%#" PRIx64 " %s state:IN-FLIGHT->PRE-RELEASE",
                pItem->pFrame->frameNo, streamId, pStreamBuffer->getName()
            );
            pStreamBuffer->setReleaseFence(RF);
            pStreamBuffer->setAcquireFence(-1);
            //
            pItem->state = State::PRE_RELEASE;
        };

    for (size_t i = 0; i < rItemSet.size(); i++)
    {
        StreamId_T const streamId = rItemSet.keyAt(i);
        sp<ImageItem> pItem = rItemSet.valueAt(i);
        if  ( pItem == 0 ) {
            //MY_LOGV("Image streamId:%#" PRIx64 " NULL ImageItem", streamId);
            continue;
        }
        //
        switch  (pItem->state)
        {
        case State::IN_FLIGHT:{
            MUINT32 allUsersStatus = pItem->buffer->getAllUsersStatus();
            //
            //  IN_FLIGHT && all users release ==> VALID/ERROR
            if ( allUsersStatus == IUsersManager::UserStatus::RELEASE )
            {
                pReleaseHandler(streamId, pItem.get(), pItem->buffer.get());
            }
            //
            //  IN-IN_FLIGHT && all users release or pre-release ==> PRE_RELEASE
            else
            if ( allUsersStatus == IUsersManager::UserStatus::PRE_RELEASE )
            {
                pPreReleaseHandler(streamId, pItem.get(), pItem->buffer.get());
            }
            }break;
            //
        case State::PRE_RELEASE:{
            //  PRE_RELEASE && all users release ==> VALID/ERROR
            if  ( OK == pItem->buffer->haveAllUsersReleased() )
            {
                pReleaseHandler(streamId, pItem.get(), pItem->buffer.get());
            }
            }break;
            //
        default:
            break;
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
updateResult(ResultQueueT const& rvResult) -> void
{
    if  ( mFrameQueue.empty() ) {
        MY_LOGD("Empty FrameQueue:%p %p", &mFrameQueue, this);
        return;
    }
    //
    FrameQueue::iterator itFrame = mFrameQueue.begin();
    for (size_t iResult = 0; iResult < rvResult.size(); iResult++)
    {
        MUINT32 const frameNo = rvResult.valueAt(iResult)->frameNo;
        for (; itFrame != mFrameQueue.end(); itFrame++) {
            //
            sp<FrameParcel>& pFrame = *itFrame;
            if  ( frameNo != pFrame->frameNo ) {
                continue;
            }
            // put output meta into vOutputMetaItem
            sp<MetaItem> pMetaItem = NULL; // last partial metadata
            MetaItemSet* pItemSet = &pFrame->vOutputMetaItem;
            Vector< sp<IMetaStreamBuffer> >::iterator it = rvResult.valueAt(iResult)->buffer.begin();
            for(; it != rvResult.valueAt(iResult)->buffer.end(); it++)
            {
                sp<IMetaStreamBuffer> const pBuffer = *it;
                //
                StreamId_T const streamId = pBuffer->getStreamInfo()->getStreamId();
                //
                sp<MetaItem> pItem = new MetaItem;
                pItem->pFrame = pFrame.get();
                pItem->pItemSet = pItemSet;
                pItem->buffer = pBuffer;
                pItem->bufferNo = pItemSet->size() + 1;
                pMetaItem = pItem;
                //
                pItemSet->add(streamId, pItem);
            }

            if  ( pMetaItem == 0 ) {
                //MY_LOGV("frameNo:%u NULL MetaItem", frameNo);
            } else if(rvResult.valueAt(iResult)->lastPartial) {
                pMetaItem->bufferNo = mCommonInfo->mAtMostMetaStreamCount;
                pItemSet->hasLastPartial = true;
            }
            //
            updateItemSet(pFrame->vOutputMetaItem);
            updateItemSet(pFrame->vOutputImageItem);
            updateItemSet(pFrame->vInputImageItem);
            break;
        }
        //
        if  ( itFrame == mFrameQueue.end() ) {
            {
                String8 log = String8::format("frameNo:%u is not in FrameQueue; FrameQueue:", frameNo);
                for (auto const& v : mFrameQueue) {
                    log += String8::format(" %u", v->frameNo);
                }
                log += String8("; ResultQueue:");
                for (size_t v = 0; v < rvResult.size(); v++) {
                    log += String8::format(" %u", rvResult.valueAt(v)->frameNo);
                }
                MY_LOGW("%s", log.c_str());
            }
            itFrame = mFrameQueue.begin();
        }
    }
    //
    MUINT32 const latestResultFrameNo = rvResult.valueAt(rvResult.size() - 1)->frameNo;
    if  (0 < (MINT32)(latestResultFrameNo - mFrameQueue.latestResultFrameNo) ) {
        mFrameQueue.latestResultFrameNo = latestResultFrameNo;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
updateCallback(std::list<CallbackParcel>& rCbList) -> void
{
    FrameQueue::iterator itFrame = mFrameQueue.begin();
    while ( itFrame != mFrameQueue.end() )
    {
        MUINT32 const frameNo = (*itFrame)->frameNo;
        if  ( 0 < (MINT32)(frameNo - mFrameQueue.latestResultFrameNo) ) {
            MY_LOGV("stop updating frame => frameNo: this(%u) > latest(%u) ", frameNo, mFrameQueue.latestResultFrameNo);
            break;
        }
        //
        CallbackParcel cbParcel;
        cbParcel.valid = MFALSE;
        cbParcel.frameNo = frameNo;
        cbParcel.timestampShutter = (*itFrame)->shutterTimestamp;
        //
        if  ( checkRequestError(**itFrame) > 0 ) {
            //It's a request error
            //Here we're still not sure that the input image stream is returned or not.
            MY_LOGW("frameNo:%u Request Error", (*itFrame)->frameNo);
            prepareErrorFrame(cbParcel, *itFrame);
        }
        else {
            prepareCallbackIfPossible(cbParcel, (*itFrame)->vOutputMetaItem);
            prepareCallbackIfPossible(cbParcel, (*itFrame)->vOutputImageItem);
        }
        prepareCallbackIfPossible(cbParcel, (*itFrame)->vInputImageItem);
        //
        if  ( cbParcel.valid ) {
            rCbList.push_back(cbParcel);
        }
        //
        if  ( isFrameRemovable(*itFrame) ) {
            //remove this frame from the frame queue.
            itFrame = mFrameQueue.erase(itFrame);
        }
        else {
            //next iteration
            itFrame++;
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
update(ResultQueueT const& rvResult) -> void
{
    Mutex::Autolock _l(mFrameQueueLock);

    std::list<CallbackParcel> cbList;
    updateResult(rvResult);
    updateCallback(cbList);
    mBatchHandler->push(cbList);

    if  ( mFrameQueue.empty() ) {
        mFrameQueueCond.broadcast();
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
waitUntilDrained(nsecs_t const timeout) -> int
{
    nsecs_t const startTime = ::systemTime();
    //
    auto timeoutToWait = [=](){
        nsecs_t const elapsedInterval = (::systemTime() - startTime);
        nsecs_t const timeoutToWait = (timeout > elapsedInterval)
                                    ? (timeout - elapsedInterval)
                                    :   0
                                    ;
        return timeoutToWait;
    };
    //
    auto waitEmpty = [=](Mutex& lock, Condition& cond, auto const& queue) -> int {
        int err = OK;
        Mutex::Autolock _l(lock);
        while ( ! queue.empty() )
        {
            err = cond.waitRelative(lock, timeoutToWait());
            if  ( OK != err ) {
                break;
            }
        }
        //
        if  ( queue.empty() ) { return OK; }
        return err;
    };
    //
    //
    int err = OK;
    if  ( OK != (err = waitEmpty(mFrameQueueLock, mFrameQueueCond, mFrameQueue)))
    {
        MY_LOGW(
            "mFrameQueue:#%zu timeout(ns):%" PRId64 " elapsed(ns):%" PRId64 " err:%d(%s)",
            mFrameQueue.size(), timeout, (::systemTime() - startTime), err, ::strerror(-err)
        );
        dump();
    }
    //
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
dumpStateLocked(android::Printer& printer) const -> void
{
    auto logImage = [&](char const* tag, ImageItemSet const& rItems) {
        String8 strCaption = String8::format("      %s - return#:%zu valid#:%zu error#:%zu",
            tag, rItems.numReturnedStreams, rItems.numValidStreams, rItems.numErrorStreams);
        printer.printLine(strCaption.c_str());
        for (size_t i = 0; i < rItems.size(); i++) {
            StreamId_T const streamId = rItems.keyAt(i);
            ImageItem* pItem = rItems.valueAt(i).get();
            String8 str = String8::format("          streamId:%02" PRIx64 " ", streamId);
            if ( pItem ) {
                str += String8::format("bufferId:%02" PRIu64 " ", pItem->buffer->getBufferId());
                str += stateToText(pItem->state) + " " + historyToText(pItem->history);
            }
            printer.printLine(str.c_str());
        }
    };

    auto logMeta = [&](char const* tag, MetaItemSet const& rItems) {
        String8 strCaption = String8::format("      %s - ", tag);
        if (rItems.asInput) {
            strCaption += (rItems.valueAt(0)->buffer->isRepeating() ? "REPEAT" : "CHANGE");
        }
        else {
            strCaption += String8::format("return#:%zu valid#:%zu error#:%zu", rItems.numReturnedStreams, rItems.numValidStreams, rItems.numErrorStreams);
        }
        printer.printLine(strCaption.c_str());

        if (rItems.asInput) { return; }

        for (size_t i = 0; i < rItems.size(); i++) {
            StreamId_T const streamId = rItems.keyAt(i);
            MetaItem* pItem = rItems.valueAt(i).get();
            String8 str = String8::format("          streamId:%02" PRIx64 " ", streamId);
            if ( pItem ) {
                str += stateToText(pItem->state) + " " + historyToText(pItem->history);
            }
            printer.printLine(str.c_str());
        }
    };


    printer.printLine(" *Stream Configuration*");
    printer.printFormatLine("  operationMode:%#x", mOperationMode);
    for (size_t i = 0; i < mMetaConfigMap.size(); i++) {
        auto const& item = mMetaConfigMap.valueAt(i);
        printer.printFormatLine("  %#" PRIx64 " INPUT %s", item.pStreamInfo->getStreamId(), item.pStreamInfo->getStreamName());
    }
    for (size_t i = 0; i < mImageConfigMap.size(); i++) {
        auto const& item = mImageConfigMap.valueAt(i);
        item.pStreamInfo->dumpState(printer, 2);
        item.pBufferHandleCache->dumpState(printer, 6);
    }

    printer.printLine("");
    printer.printLine(" *In-flight requests*");
    FrameQueue::const_iterator itFrame = mFrameQueue.begin();
    for (; itFrame != mFrameQueue.end(); itFrame++)
    {
        auto const& item = (*itFrame);
        String8 caption = String8::format("  frame:%u", item->frameNo);
        if (auto pTool = NSCam::Utils::LogTool::get()) {
            caption += String8::format(" %s :", pTool->convertToFormattedLogTime(&item->requestTimestamp).c_str());
        }
        if (0 != item->shutterTimestamp) {
            caption += String8::format(" shutter:%" PRId64 "", item->shutterTimestamp);
        }
        if (0 != item->errors.value) {
            caption += String8::format(" errors:%#x", item->errors.value);
        }
        printer.printLine(caption.c_str());
        //
        //  Input Meta
        {
            logMeta(" IN Meta ", item->vInputMetaItem);
        }
        //
        //  Output Meta
        {
            logMeta("OUT Meta ", item->vOutputMetaItem);
        }
        //
        //  Input Image
        if ( 0 < item->vInputImageItem.size() )
        {
            logImage(" IN Image", item->vInputImageItem);
        }
        //
        //  Output Image
        {
            logImage("OUT Image", item->vOutputImageItem);
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
dumpLocked() const -> void
{
    android::LogPrinter p(LOG_TAG);
    dumpStateLocked(p);
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
dump() const -> void
{
    Mutex::Autolock _l(mFrameQueueLock);
    dumpLocked();
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
dumpState(android::Printer& printer, const std::vector<std::string>& /*options*/) -> void
{
    Mutex::Autolock _l(mFrameQueueLock);
    dumpStateLocked(printer);
}

