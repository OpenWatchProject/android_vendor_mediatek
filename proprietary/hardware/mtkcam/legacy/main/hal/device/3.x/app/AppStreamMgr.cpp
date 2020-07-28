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

#include "AppStreamMgr.h"
#include "MyUtils.h"
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;


/******************************************************************************
 *
 ******************************************************************************/
#define MY_DEBUG(level, fmt, arg...) \
    do { \
        CAM_LOG##level("[%s] " fmt, __FUNCTION__, ##arg); \
        mCommonInfo->mDebugPrinter->printFormatLine(#level" [%s] " fmt, __FUNCTION__, ##arg); \
    } while(0)

#define MY_WARN(level, fmt, arg...) \
    do { \
        CAM_LOG##level("[%s] " fmt, __FUNCTION__, ##arg); \
        mCommonInfo->mWarningPrinter->printFormatLine(#level" [%s] " fmt, __FUNCTION__, ##arg); \
    } while(0)

#define MY_ERROR(level, fmt, arg...) \
    do { \
        CAM_LOG##level("[%s] " fmt, __FUNCTION__, ##arg); \
        mCommonInfo->mErrorPrinter->printFormatLine(#level" [%s] " fmt, __FUNCTION__, ##arg); \
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
const std::string AppStreamMgr::MyDebuggee::mName{"NSCam::v3::IAppStreamManager"};
auto AppStreamMgr::MyDebuggee::debug(android::Printer& printer, const std::vector<std::string>& options) -> void
{
    auto p = mContext.promote();
    if ( p != nullptr ) {
        p->dumpState(printer, options);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
IAppStreamManager::
create(const CreationInfo& creationInfo) -> IAppStreamManager*
{
    auto pInstance = new AppStreamMgr(creationInfo);
    if  ( ! pInstance || ! pInstance->initialize() ) {
        delete pInstance;
        return nullptr;
    }
    return pInstance;
}


/******************************************************************************
 *
 ******************************************************************************/
AppStreamMgr::
AppStreamMgr(const CreationInfo& creationInfo)
    : mCommonInfo(std::make_shared<CommonInfo>())
    , mInstanceName{std::to_string(creationInfo.mInstanceId) + ":AppStreamMgr"}
{
    if ( creationInfo.mErrorPrinter == nullptr
      || creationInfo.mWarningPrinter == nullptr
      || creationInfo.mDebugPrinter == nullptr
      || creationInfo.mCameraDeviceCallback == nullptr
      || creationInfo.mMetadataProvider == nullptr
      || creationInfo.mMetadataConverter == nullptr )
    {
        MY_LOGE("mErrorPrinter:%p mWarningPrinter:%p mDebugPrinter:%p mCameraDeviceCallback:%p mMetadataProvider:%p mMetadataConverter:%p",
            creationInfo.mErrorPrinter.get(),
            creationInfo.mWarningPrinter.get(),
            creationInfo.mDebugPrinter.get(),
            creationInfo.mCameraDeviceCallback.get(),
            creationInfo.mMetadataProvider.get(),
            creationInfo.mMetadataConverter.get());
        mCommonInfo = nullptr;
        return;
    }

    IGrallocHelper* pGrallocHelper = IGrallocHelper::singleton();
    if ( pGrallocHelper == nullptr ) {
        MY_LOGE("IGrallocHelper::singleton(): nullptr");
        mCommonInfo = nullptr;
        return;
    }

    if ( mCommonInfo != nullptr )
    {
        size_t aAtMostMetaStreamCount = 1;
        IMetadata::IEntry const& entry = creationInfo.mMetadataProvider->getMtkStaticCharacteristics().entryFor(MTK_REQUEST_PARTIAL_RESULT_COUNT);
        if ( entry.isEmpty() ) {
            MY_LOGE("no static REQUEST_PARTIAL_RESULT_COUNT");
            aAtMostMetaStreamCount = 1;
        }
        else {
            aAtMostMetaStreamCount = entry.itemAt(0, Type2Type<MINT32>());
        }
        //
        int32_t loglevel = ::property_get_int32("debug.camera.log", 0);
        if ( loglevel == 0 ) {
            loglevel = ::property_get_int32("debug.camera.log.AppStreamMgr", 0);
        }
        //
        //
        mCommonInfo->mLogLevel = loglevel;
        mCommonInfo->mInstanceId = creationInfo.mInstanceId;
        mCommonInfo->mErrorPrinter = creationInfo.mErrorPrinter;
        mCommonInfo->mWarningPrinter = creationInfo.mWarningPrinter;
        mCommonInfo->mDebugPrinter = creationInfo.mDebugPrinter;
        mCommonInfo->mDeviceCallback = creationInfo.mCameraDeviceCallback;
        mCommonInfo->mMetadataProvider = creationInfo.mMetadataProvider;
        mCommonInfo->mMetadataConverter = creationInfo.mMetadataConverter;
        mCommonInfo->mGrallocHelper = pGrallocHelper;
        mCommonInfo->mAtMostMetaStreamCount = aAtMostMetaStreamCount;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
AppStreamMgr::
initialize() -> bool
{
    status_t status = OK;
    //
    if  ( mCommonInfo == nullptr ) {
        MY_LOGE("Bad mCommonInfo");
        return false;
    }
    //
    {
        mRequestMetadataQueue = std::make_shared<RequestMetadataQueue>(
            CAMERA_REQUEST_METADATA_QUEUE_SIZE, false /* non blocking */);
        if  ( !mRequestMetadataQueue || !mRequestMetadataQueue->isValid() ) {
            MY_LOGE("invalid request fmq");
            return false;
        }
        mResultMetadataQueue = std::make_shared<RequestMetadataQueue>(
            CAMERA_RESULT_METADATA_QUEUE_SIZE, false /* non blocking */);
        if  ( !mResultMetadataQueue || !mResultMetadataQueue->isValid() ) {
            MY_LOGE("invalid result fmq");
            return false;
        }
    }
    //
    {
        mCallbackHandler = new CallbackHandler(mCommonInfo, mResultMetadataQueue);
        if  ( mCallbackHandler == nullptr ) {
            MY_LOGE("Bad mCallbackHandler");
            return false;
        }
        else {
            const std::string threadName{std::to_string(mCommonInfo->mInstanceId)+":AppMgr-CbHdl"};
            status = mCallbackHandler->run(threadName.c_str());
            if  ( OK != status ) {
                MY_LOGE("Fail to run the thread %s - status:%d(%s)", threadName.c_str(), status, ::strerror(-status));
                return false;
            }
        }
        //
        mBatchHandler = new BatchHandler(mCommonInfo, mCallbackHandler);
        if  ( mBatchHandler == nullptr ) {
            MY_LOGE("Bad mBatchHandler");
            return false;
        }
        //
        mFrameHandler = new FrameHandler(mCommonInfo, mBatchHandler);
        if  ( mFrameHandler == nullptr ) {
            MY_LOGE("Bad mFrameHandler");
            return false;
        }
        //
        mResultHandler = new ResultHandler(mCommonInfo, mFrameHandler);
        if  ( mResultHandler == nullptr ) {
            MY_LOGE("Bad mResultHandler");
            return false;
        }
        else {
            const std::string threadName{std::to_string(mCommonInfo->mInstanceId)+":AppMgr-RstHdl"};
            status = mResultHandler->run(threadName.c_str());
            if  ( OK != status ) {
                MY_LOGE("Fail to run the thread %s - status:%d(%s)", threadName.c_str(), status, ::strerror(-status));
                return false;
            }
        }
        //
        mRequestHandler = new RequestHandler(mCommonInfo, mRequestMetadataQueue, mFrameHandler, mBatchHandler);
        if  ( mRequestHandler == nullptr ) {
            MY_LOGE("Bad mRequestHandler");
            return false;
        }
        //
        mConfigHandler = new ConfigHandler(mCommonInfo, mFrameHandler, mBatchHandler);
        if  ( mConfigHandler == nullptr ) {
            MY_LOGE("Bad mConfigHandler");
            return false;
        }
        //
    }
    //
    mDebuggee = std::make_shared<MyDebuggee>(this);
    if ( auto pDbgMgr = IDebuggeeManager::get() ) {
        mDebuggee->mCookie = pDbgMgr->attach(mDebuggee, 1);
    }
    //
    return true;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
AppStreamMgr::
destroy() -> void
{
    auto resLock = mInterfaceLock.timedLock(::ms2ns(1000));
    if  ( OK != resLock ) {
        MY_LOGW("timedLock failed; still go on to destroy");
    }


    mConfigHandler->destroy();
    mConfigHandler = nullptr;

    mRequestHandler->destroy();
    mRequestHandler = nullptr;

    mResultHandler->destroy();
    mResultHandler = nullptr;

    mFrameHandler->destroy();
    mFrameHandler = nullptr;

    mBatchHandler->destroy();
    mBatchHandler = nullptr;

    mCallbackHandler->destroy();
    mCallbackHandler = nullptr;

    if ( mDebuggee != nullptr ) {
        if ( auto pDbgMgr = IDebuggeeManager::get() ) {
            pDbgMgr->detach(mDebuggee->mCookie);
        }
        mDebuggee = nullptr;
    }


    if  ( OK == resLock ) {
        mInterfaceLock.unlock();
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
AppStreamMgr::
dumpState(android::Printer& printer __unused, const std::vector<std::string>& options __unused) -> void
{
    printer.printFormatLine("## App Stream Manager  [%u]\n", mCommonInfo->mInstanceId);

    if  ( OK == mInterfaceLock.timedLock(::ms2ns(500)) ) {

        dumpStateLocked(printer, options);

        mInterfaceLock.unlock();
    }
    else {
        printer.printLine("mInterfaceLock.timedLock timeout");
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
AppStreamMgr::
dumpStateLocked(android::Printer& printer, const std::vector<std::string>& options) -> void
{
    if ( mFrameHandler != nullptr ) {
        mFrameHandler->dumpState(printer, options);
    }
    if ( mBatchHandler != nullptr ) {
        printer.printLine(" ");
        mBatchHandler->dumpState(printer, options);
    }
    if ( mCallbackHandler != nullptr ) {
        printer.printLine(" ");
        mCallbackHandler->dumpState(printer, options);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
AppStreamMgr::
beginConfigureStreams(
    const StreamConfiguration& requestedConfiguration,
    HalStreamConfiguration& halConfiguration,
    ConfigAppStreams& rStreams
)   -> int
{
    CAM_TRACE_CALL();

    MY_LOGD("StreamConfiguration=%s", toString(requestedConfiguration).c_str());

    Mutex::Autolock _l(mInterfaceLock);
    //
    auto err = mConfigHandler->beginConfigureStreams(requestedConfiguration, halConfiguration, rStreams);
    //
    //  An emtpy settings buffer cannot be used as the first submitted request
    //  after a configure_streams() call.
    if ( OK == err ) {
        mRequestHandler->reset();
    }
    //
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
AppStreamMgr::
endConfigureStreams(
    HalStreamConfiguration& halConfiguration
) -> int
{
    CAM_TRACE_CALL();

    Mutex::Autolock _l(mInterfaceLock);

    auto err = mConfigHandler->endConfigureStreams(halConfiguration);

    MY_LOGD("HalStreamConfiguration=%s", toString(halConfiguration).c_str());
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
AppStreamMgr::
getCaptureRequestMetadataQueue() -> const ::android::hardware::MQDescriptorSync<uint8_t>&
{
    Mutex::Autolock _l(mInterfaceLock);
    return *mRequestMetadataQueue->getDesc();
}


/******************************************************************************
 *
 ******************************************************************************/
auto
AppStreamMgr::
getCaptureResultMetadataQueue() -> const ::android::hardware::MQDescriptorSync<uint8_t>&
{
    Mutex::Autolock _l(mInterfaceLock);
    return *mResultMetadataQueue->getDesc();
}


/******************************************************************************
 *
 ******************************************************************************/
auto
AppStreamMgr::
flushRequest(
    const hidl_vec<CaptureRequest>& requests
)   -> void
{
    auto prepareBuffer = [this](StreamBuffer const& buffer, auto& rvImageItem) {
        if  ( -1 != buffer.streamId && 0 != buffer.bufferId ) {
            auto const& pStreamInfo = mFrameHandler->getConfigImageStream(buffer.streamId);
            rvImageItem.push_back(
                CallbackParcel::ImageItem{
                    .buffer = AppErrorImageStreamBuffer::Allocator()(
                                buffer.bufferId,
                                pStreamInfo.get(),
                                (buffer.acquireFence != nullptr ? ::dup(buffer.acquireFence->data[0]) : -1)),
                    .stream = pStreamInfo,
                }
            );
            if ( buffer.buffer!=nullptr ) {
                buffer_handle_t bufferHandle = nullptr;
                int acquire_fence = -1;
                mRequestHandler->importStreamBuffer(buffer, bufferHandle, acquire_fence);
                mFrameHandler->cacheBuffer(buffer.streamId, buffer.bufferId, bufferHandle);
                MY_LOGW("we should keep this map - stream:%d bufferId:%" PRId64 " bufferFd:%d bufferHandle(%p)",
                        buffer.streamId, buffer.bufferId, buffer.buffer->data[0], bufferHandle);
            }
        }
    };

    Mutex::Autolock _l(mInterfaceLock);
    std::list<CallbackParcel> cbList;
    for (auto const& req : requests) {
        CallbackParcel cbParcel = {
            .shutter = nullptr,
            .timestampShutter = 0,
            .frameNo = req.frameNumber,
            .valid = MTRUE,
        };

        //  cbParcel.vError
        cbParcel.vError.push_back(
            CallbackParcel::Error{
                . errorCode = ErrorCode::ERROR_REQUEST,
            }
        );

        //  cbParcel.vInputImageItem <- req.inputBuffer
        prepareBuffer(req.inputBuffer, cbParcel.vInputImageItem);

        //  cbParcel.vOutputImageItem <- req.outputBuffers
        for (auto const& buffer : req.outputBuffers) {
            prepareBuffer(buffer, cbParcel.vOutputImageItem);
        }
        cbList.push_back(std::move(cbParcel));
    }
    mBatchHandler->push(cbList);
    // clear request metadata queue
    if ( mRequestMetadataQueue->availableToRead() ) {
        CameraMetadata dummySettingsFmq;
        dummySettingsFmq.resize(mRequestMetadataQueue->availableToRead());
        bool read = mRequestMetadataQueue->read(dummySettingsFmq.data(), mRequestMetadataQueue->availableToRead());
        if ( !read ) {
            MY_LOGW("cannot clear request fmq during flush!");
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
AppStreamMgr::
removeBufferCache(
    const hidl_vec<BufferCache>& cachesToRemove
)   -> void
{
    Mutex::Autolock _l(mInterfaceLock);
    for ( auto& cache : cachesToRemove ) {
        mFrameHandler->uncacheBuffer(cache.streamId, cache.bufferId);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
AppStreamMgr::
submitRequest(
    const hidl_vec<CaptureRequest>& captureRequests,
    android::Vector<Request>& rRequests
) -> int
{
    Mutex::Autolock _l(mInterfaceLock);
    int err = mRequestHandler->submitRequest(captureRequests, rRequests);
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
AppStreamMgr::
updateResult(
    uint32_t const frameNo,
    intptr_t const userId,
    android::Vector<android::sp<IMetaStreamBuffer>> resultMeta,
    bool hasLastPartial
)   -> void
{
    auto pHandler = mResultHandler;
    if ( pHandler != nullptr ) {
        pHandler->enqueResult(frameNo, userId, resultMeta, hasLastPartial);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
AppStreamMgr::
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
    //
    int err = OK;
    Mutex::Autolock _l(mInterfaceLock);

    (void)((OK == (err = mFrameHandler->waitUntilDrained(timeout)))
        && (OK == (err = mBatchHandler->waitUntilDrained(timeoutToWait())))
        && (OK == (err = mCallbackHandler->waitUntilDrained(timeoutToWait()))))
            ;

    if (OK != err) {
        MY_LOGW("timeout(ns):%" PRId64 " err:%d(%s)", timeout, -err, ::strerror(-err));
        android::LogPrinter logPrinter(LOG_TAG, ANDROID_LOG_DEBUG, "[waitUntilDrained] ");
        dumpStateLocked(logPrinter, {});
    }

    return err;
}

