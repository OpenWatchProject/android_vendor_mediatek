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

using namespace android;
using namespace NSCam;
using namespace NSCam::v3;

#define ThisNamespace   AppStreamMgr::CallbackHandler

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
#define MY_LOGW_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
static const int kDumpLockRetries = 50;
static const int kDumpLockSleep = 60000;

static bool tryLock(Mutex& mutex)
{
    bool locked = false;
    for (int i = 0; i < kDumpLockRetries; ++i) {
        if (mutex.tryLock() == NO_ERROR) {
            locked = true;
            break;
        }
        usleep(kDumpLockSleep);
    }
    return locked;
}


/******************************************************************************
 *
 ******************************************************************************/
static void convertToDebugString(AppStreamMgr::CallbackParcel const& cbParcel, android::Vector<android::String8>& out)
{
/*
    frame:80 errors#:10 shutter:10 o:meta#:10 i:image#:10 o:image#:10
        {ERROR_DEVICE/ERROR_REQUEST/ERROR_RESULT}
        {ERROR_BUFFER streamId:123 bufferId:456}
        OUT Meta  -
            partial#:123 "xxxxxxxxxx"
        IN Image -
            streamId:01 bufferId:04 OK
        OUT Image -
            streamId:02 bufferId:04 ERROR with releaseFence
*/
    String8 log = String8::format("frame:%u", cbParcel.frameNo);
    if  ( ! cbParcel.vError.isEmpty() ) {
        log += String8::format(" errors#:%zu", cbParcel.vError.size());
    }
    if  ( cbParcel.shutter != 0 ) {
        log += String8::format(" shutter:%" PRId64 "", cbParcel.shutter->timestamp);
    }
    if  ( ! cbParcel.vOutputMetaItem.isEmpty() ) {
        log += String8::format(" o:meta#:%zu", cbParcel.vOutputMetaItem.size());
    }
    if  ( ! cbParcel.vInputImageItem.isEmpty() ) {
        log += String8::format(" i:image#:%zu", cbParcel.vInputImageItem.size());
    }
    if  ( ! cbParcel.vOutputImageItem.isEmpty() ) {
        log += String8::format(" o:image#:%zu", cbParcel.vOutputImageItem.size());
    }
    out.push_back(log);
    //
    for (auto const& item : cbParcel.vError) {
        out.push_back(String8::format("    {%s %s}", toString(item.errorCode).c_str(), (item.stream == 0 ? "" : item.stream->getStreamName())));
    }
    //
    if ( ! cbParcel.vOutputMetaItem.isEmpty() ) {
        out.push_back(String8("    OUT Meta  -"));
        for (auto const& item: cbParcel.vOutputMetaItem) {
            out.push_back(String8::format("        streamId:%02" PRIx64 " partial#:%u %s",
                item.buffer->getStreamInfo()->getStreamId(), item.bufferNo, item.buffer->getName()));
        }
    }
    //
    if  ( ! cbParcel.vInputImageItem.isEmpty() ) {
        out.push_back(String8("     IN Image -"));
        for (auto const& item: cbParcel.vInputImageItem) {
            out.push_back(String8::format("        streamId:%02" PRIx64 " bufferId:%02" PRIu64 " %s %s",
                item.stream->getStreamId(), item.buffer->getBufferId(),
                (item.buffer->hasStatus(STREAM_BUFFER_STATUS::ERROR) ? "ERROR" : "OK"),
                (item.buffer->getReleaseFence()==-1 ? "" : "with releaseFence")
            ));
        }
    }
    //
    if  ( ! cbParcel.vOutputImageItem.isEmpty() ) {
        out.push_back(String8("    OUT Image -"));
        for (auto const& item: cbParcel.vOutputImageItem) {
            out.push_back(String8::format("        streamId:%02" PRIx64 " bufferId:%02" PRIu64 " %s %s",
                item.stream->getStreamId(), item.buffer->getBufferId(),
                (item.buffer->hasStatus(STREAM_BUFFER_STATUS::ERROR) ? "ERROR" : "OK"),
                (item.buffer->getReleaseFence()==-1 ? "" : "with releaseFence")
            ));
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
ThisNamespace::
CallbackHandler(
    std::shared_ptr<CommonInfo> pCommonInfo,
    std::shared_ptr<ResultMetadataQueue> pResultMetadataQueue
)
    : mInstanceName{std::to_string(pCommonInfo->mInstanceId) + "-CallbackHandler"}
    , mCommonInfo(pCommonInfo)
    , mResultMetadataQueue(pResultMetadataQueue)
{
    mStatistics.mStartTime_Output = systemTime(SYSTEM_TIME_PROCESS);
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
destroy() -> void
{
    mCommonInfo->mDebugPrinter->printLine("[destroy] mCallbackHandler->join +");
    this->requestExit();
    this->join();
    mCommonInfo->mDebugPrinter->printLine("[destroy] mCallbackHandler->join -");
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
dumpState(android::Printer& printer __unused, const std::vector<std::string>& options __unused) -> void
{
    auto logQueue = [&](auto const& queue) {
        for (auto const& item : queue) {
            Vector<String8> logs;
            convertToDebugString(item, logs);
            for (auto const& log : logs) {
                printer.printFormatLine("  %s", log.c_str());
            }
        }
    };

    printer.printLine(" *Pending callback results*");

    {
        Mutex::Autolock _l1(mQueue1Lock);
        logQueue(mQueue1);

        if  ( tryLock(mQueue2Lock) )
        {
            if ( ! mQueue1.empty() ) {
                printer.printLine("  ***");
            }
            logQueue(mQueue2);
            mQueue2Lock.unlock();
        }
    }

    printer.printLine("");
    printer.printLine(" *Statistics*");
    printer.printFormatLine(
        "  AvgShutterFps(x100):%" PRIu64 "",
        mStatistics.mAvgShutterFps.load()
    );

    {
        auto const partialResult = mStatistics.mLatestTimestamp_FinalPartialResultCallback.load();
        printer.printFormatLine(
            "  f%u : Latest Final-Partial-Result Callback (%s)",
            partialResult.frameNo, NSCam::Utils::LogTool::get()->convertToFormattedLogTime(&partialResult.ts).c_str()
        );
    }

    {
        auto const shutter = mStatistics.mLatestTimestamp_Shutter.load();
        printer.printFormatLine(
            "  f%u : Latest Shutter Timestamp:%" PRIu64 "",
            shutter.frameNo, shutter.timestamp
        );
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
        while ( ! exitPending() && ! queue.empty() )
        {
            err = cond.waitRelative(lock, timeoutToWait());
            if  ( OK != err ) {
                break;
            }
        }
        //
        if  ( queue.empty() ) { return OK; }
        if  ( exitPending() ) { return DEAD_OBJECT; }
        return err;
    };
    //
    //
    int err = OK;
    if  (   OK != (err = waitEmpty(mQueue1Lock, mQueue1Cond, mQueue1))
        ||  OK != (err = waitEmpty(mQueue2Lock, mQueue2Cond, mQueue2)))
    {
        MY_LOGW(
            "mQueue1:#%zu mQueue2:#%zu exitPending:%d timeout(ns):%" PRId64 " elapsed(ns):%" PRId64 " err:%d(%s)",
            mQueue1.size(), mQueue2.size(), exitPending(), timeout, (::systemTime() - startTime), err, ::strerror(-err)
        );
    }
    //
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
push(std::list<CallbackParcel>& item) -> void
{
    Mutex::Autolock _l(mQueue1Lock);

    //Transfers all elements from item to mQueue1.
    //After that, item is empty.
    mQueue1.splice(mQueue1.end(), item);

    mQueue1Cond.broadcast();
}


/******************************************************************************
 *
 ******************************************************************************/
// Good place to do one-time initializations
auto
ThisNamespace::
readyToRun() -> status_t
{
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
requestExit() -> void
{
    MY_LOGD("+ %s", mInstanceName.c_str());
    //
    {
        Mutex::Autolock _l1(mQueue1Lock);
        Mutex::Autolock _l2(mQueue2Lock);
        Thread::requestExit();
        mQueue1Cond.broadcast();
        mQueue2Cond.broadcast();
    }
    //
    MY_LOGD("- %s", mInstanceName.c_str());
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
threadLoop() -> bool
{
    if ( ! waitUntilQueue1NotEmpty() ) {
        MY_LOGD_IF(1, "Queue1 is empty");
        return true;
    }
    //
    {
        Mutex::Autolock _l1(mQueue1Lock);
        Mutex::Autolock _l2(mQueue2Lock);

        //Transfers all elements from mQueue1 to mQueue2.
        //After that, mQueue1 is empty.
        mQueue2.splice(mQueue2.end(), mQueue1);
        mQueue1Cond.broadcast();
    }
    //
    performCallback();
    //
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
waitUntilQueue1NotEmpty() -> bool
{
    Mutex::Autolock _l(mQueue1Lock);

    while ( ! exitPending() && mQueue1.empty() )
    {
        int err = mQueue1Cond.wait(mQueue1Lock);
        MY_LOGW_IF(
            OK != err,
            "exitPending:%d mQueue1#:%zu err:%d(%s)",
            exitPending(), mQueue1.size(), err, ::strerror(-err)
        );
    }

    return ! mQueue1.empty();
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
performCallback() -> void
{
    CAM_TRACE_CALL();
    std::vector<NotifyMsg> vNotifyMsg;
    std::vector<V3_4::CaptureResult> vCaptureResult;
    //
    {
        Mutex::Autolock _l2(mQueue2Lock);
        //
        beginStatistics();
        //
        for (auto const& cbParcel : mQueue2) {
            CAM_TRACE_BEGIN((!ATRACE_ENABLED()?"":String8::format(
                "Dev-%d:convert|request:%d", mCommonInfo->mInstanceId, cbParcel.frameNo).c_str()));

            //CallbackParcel::shutter
            convertShutterToHidl(cbParcel, vNotifyMsg);
            //
            //CallbackParcel::vError
            convertErrorToHidl(cbParcel, vNotifyMsg);
            //
            //CallbackParcel::vOutputMetaItem
            convertMetaToHidl(cbParcel, vCaptureResult);
            //
            //CallbackParcel::vOutputImageItem
            //CallbackParcel::vInputImageItem
            convertImageToHidl(cbParcel, vCaptureResult);

            if (CC_UNLIKELY(getLogLevel() >= 1)) {
                Vector<String8> logs;
                convertToDebugString(cbParcel, logs);
                for (auto const& l : logs) {
                    MY_LOGD("%s", l.c_str());
                }
            }

            CAM_TRACE_END();
        }
        //
        //  vCaptureResult[*].result -> mResultMetadataQueue
        if  ( auto pResultMetadataQueue = mResultMetadataQueue ) {
            for (auto& item : vCaptureResult) {
                if  ( item.v3_2.result.size() == 0 ) {
                    continue;
                }

                CAM_TRACE_BEGIN((!ATRACE_ENABLED()?"":String8::format(
                    "result metadata -> fmq|request:%d =fmqResultSize:%zu", item.v3_2.frameNumber, item.v3_2.result.size()).c_str()));
                if  ( pResultMetadataQueue->availableToWrite() > 0 ) {
                    if  ( CC_LIKELY(pResultMetadataQueue->write(item.v3_2.result.data(), item.v3_2.result.size())) ) {
                        item.v3_2.fmqResultSize = item.v3_2.result.size();
                        item.v3_2.result = hidl_vec<uint8_t>(); //resize(0)
                    }
                    else {
                        item.v3_2.fmqResultSize = 0;
                        MY_LOGW("fail to write to mResultMetadataQueue");
                    }
                }
                CAM_TRACE_END();
            }
        }
        //
        //  send callbacks
        {
            if  ( ! vNotifyMsg.empty() ) {
                hidl_vec<NotifyMsg> vecNotifyMsg;
                vecNotifyMsg.setToExternal(vNotifyMsg.data(), vNotifyMsg.size());
                auto ret1 = mCommonInfo->mDeviceCallback->notify(vecNotifyMsg);
                if (CC_UNLIKELY(!ret1.isOk())) {
                    MY_LOGE("Transaction error in ICameraDeviceCallback::notify: %s", ret1.description().c_str());
                }
            }
            //
            if  ( ! vCaptureResult.empty() ) {
                hidl_vec<V3_4::CaptureResult> vecCaptureResult;
                vecCaptureResult.setToExternal(vCaptureResult.data(), vCaptureResult.size());
                auto ret2 = mCommonInfo->mDeviceCallback->processCaptureResult_3_4(vecCaptureResult);
                if (CC_UNLIKELY(!ret2.isOk())) {
                    MY_LOGE("Transaction error in ICameraDeviceCallback::processCaptureResult: %s", ret2.description().c_str());
                }
            }
        }
        mQueue2.clear();
        mQueue2Cond.broadcast();    //inform anyone of empty mQueue2
        endStatistics();
        MY_LOGD_IF(getLogLevel() >= 1, "-");
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
convertShutterToHidl(CallbackParcel const& cbParcel, std::vector<NotifyMsg>& rvMsg) -> void
{
    CAM_TRACE_CALL();

    //CallbackParcel::shutter
    if  ( cbParcel.shutter != 0 )
    {
        MY_LOGI_IF(cbParcel.needIgnore,
                       "Ignore Shutter callback because error already occured, frame(%d)/timestamp(%llu)",
                       cbParcel.frameNo, cbParcel.shutter->timestamp);
        if  ( !cbParcel.needIgnore )
        {
            CAM_TRACE_BEGIN((!ATRACE_ENABLED()?"":String8::format(
                "Dev-%d:convertShutter|request:%d =timestamp(ns):%" PRId64 " needIgnore:%d",
                mCommonInfo->mInstanceId, cbParcel.frameNo, cbParcel.shutter->timestamp, cbParcel.needIgnore).c_str()));
            CAM_TRACE_END();

            rvMsg.push_back(NotifyMsg{
                .type = MsgType::SHUTTER,
                .msg  = {
                    .shutter = {
                        .frameNumber = cbParcel.frameNo,
                        .timestamp = cbParcel.shutter->timestamp,
                }}});
        }
        //
        updateStatistics_shutter(cbParcel.shutter->timestamp, cbParcel.frameNo);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
convertErrorToHidl(CallbackParcel const& cbParcel, std::vector<NotifyMsg>& rvMsg) -> void
{
    CAM_TRACE_CALL();

    //CallbackParcel::vError
    for (size_t i = 0; i < cbParcel.vError.size(); i++) {
        CallbackParcel::Error const& rError = cbParcel.vError[i];
        auto const errorStreamId = static_cast<int32_t>((rError.stream != 0) ? rError.stream->getStreamId() : -1);

        CAM_TRACE_BEGIN((!ATRACE_ENABLED()?"":String8::format(
            "Dev-%d:convertError|request:%d =errorCode:%d errorStreamId:%d",
            mCommonInfo->mInstanceId, cbParcel.frameNo, rError.errorCode, errorStreamId).c_str()));
        CAM_TRACE_END();

        rvMsg.push_back(NotifyMsg{
            .type = MsgType::ERROR,
            .msg  = {
                .error = {
                    .frameNumber = cbParcel.frameNo,
                    .errorCode = rError.errorCode,
                    .errorStreamId = errorStreamId,
            }}});
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
convertMetaToHidl(CallbackParcel const& cbParcel, std::vector<V3_4::CaptureResult>& rvResult) -> void
{
    CAM_TRACE_CALL();

    //CallbackParcel::vOutputMetaItem
    for (size_t i = 0; i < cbParcel.vOutputMetaItem.size(); i++) {
        auto const& rCbMetaItem = cbParcel.vOutputMetaItem[i];
        //
        hidl_vec<uint8_t> metadata;
        IMetadata* pMetadata = rCbMetaItem.buffer->tryReadLock(mInstanceName.c_str());
        {
            MY_LOGI_IF(cbParcel.needIgnore,
                       "Ignore Metadata callback because error already occured, frame(%d)/partialResult(%u)",
                       cbParcel.frameNo, rCbMetaItem.bufferNo);
            mCommonInfo->mMetadataConverter->convertToHidl(*pMetadata, &metadata);
            switch (getLogLevel())
            {
            case 3:
                mCommonInfo->mMetadataConverter->dumpAll(*pMetadata, cbParcel.frameNo);
                break;
            case 2:
                mCommonInfo->mMetadataConverter->dump(*pMetadata, cbParcel.frameNo);
                break;
            default:
                break;
            }
        }
        rCbMetaItem.buffer->unlock(mInstanceName.c_str(), pMetadata);
        //
        if  (!cbParcel.needIgnore) {
            CAM_TRACE_BEGIN((!ATRACE_ENABLED()?"":String8::format(
                "Dev-%d:convertMeta|request:%d =partialResult:%d size:%zu",
                mCommonInfo->mInstanceId, cbParcel.frameNo, rCbMetaItem.bufferNo, metadata.size()).c_str()));
            CAM_TRACE_END();

            rvResult.push_back(V3_4::CaptureResult{
                .v3_2.frameNumber    = cbParcel.frameNo,
                .v3_2.fmqResultSize  = 0,
                .v3_2.result         = std::move(metadata),
                // .v3_2.outputBuffers  = hidl_vec<StreamBuffer>(),
                .v3_2.inputBuffer    = {.streamId = -1},     // force assign -1 indicating no input buffer
                .v3_2.partialResult  = rCbMetaItem.bufferNo,
            });
        }
        //
        updateStatistics_finalPartialMetadata(rCbMetaItem.bufferNo == mCommonInfo->mAtMostMetaStreamCount, cbParcel.frameNo);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
convertImageToHidl(CallbackParcel const& cbParcel, std::vector<V3_4::CaptureResult>& rvResult) -> void
{
    CAM_TRACE_CALL();

    //CallbackParcel::vOutputImageItem
    //CallbackParcel::vInputImageItem
    //
    auto createNativeHandle = [](int dup_fd) -> native_handle_t* {
        if  ( -1 != dup_fd ) {
            auto handle = ::native_handle_create(/*numFds*/1, /*numInts*/0);
            if  ( CC_LIKELY(handle) ) {
                handle->data[0] = dup_fd;
                return handle;
            }
        }
        return nullptr;
    };
    //
    auto convert = [=](auto const& rCbImageItem, auto& rStreamBuffer){
        rStreamBuffer.streamId = rCbImageItem.stream->getStreamId();
        rStreamBuffer.bufferId = rCbImageItem.buffer->getBufferId();
        rStreamBuffer.buffer   = nullptr;
        rStreamBuffer.status   = rCbImageItem.buffer->hasStatus(STREAM_BUFFER_STATUS::ERROR)
                                    ? BufferStatus::ERROR
                                    : BufferStatus::OK
                                    ;
        rStreamBuffer.acquireFence = nullptr;
        rStreamBuffer.releaseFence.setTo(
            createNativeHandle(rCbImageItem.buffer->getReleaseFence()),
            true/*shouldOwn*/
        );

        CAM_TRACE_BEGIN((!ATRACE_ENABLED()?"":String8::format(
            "Dev-%d:convertImage|request:%d =%s",
            mCommonInfo->mInstanceId, cbParcel.frameNo, toString(rStreamBuffer).c_str()).c_str()));
        CAM_TRACE_END();
    };
    //
    if  ( ! cbParcel.vOutputImageItem.isEmpty() || ! cbParcel.vInputImageItem.empty() )
    {
        hidl_vec<StreamBuffer> vOutBuffers;
        vOutBuffers.resize(cbParcel.vOutputImageItem.size());

        hidl_vec<StreamBuffer> vInputBuffers;
        vInputBuffers.resize(cbParcel.vInputImageItem.size());

        //Output
        for (size_t i = 0; i < cbParcel.vOutputImageItem.size(); i++) {
            CallbackParcel::ImageItem const& rCbImageItem = cbParcel.vOutputImageItem[i];
            auto pImageBufferHeap = rCbImageItem.buffer->getImageBufferHeap();
            convert(rCbImageItem, vOutBuffers[i]);
            traceDisplayIf(cbParcel.frameNo, cbParcel.timestampShutter, rCbImageItem);
        }
        //
        //Input
        MY_LOGW_IF( cbParcel.vInputImageItem.size() > 1,
                    "input buffer should exceeds one; frmNumber:%u, vInputBuffers:%zu",
                    cbParcel.frameNo, cbParcel.vInputImageItem.size() );
        for (size_t i = 0; i < cbParcel.vInputImageItem.size(); i++) {
            CallbackParcel::ImageItem const& rCbImageItem = cbParcel.vInputImageItem[i];
            auto pImageBufferHeap = rCbImageItem.buffer->getImageBufferHeap();
            convert(rCbImageItem, vInputBuffers[i]);
        }
        //
        //
        rvResult.push_back(V3_4::CaptureResult{
            .v3_2.frameNumber    = cbParcel.frameNo,
            .v3_2.fmqResultSize  = 0,
            // .v3_2.result
            .v3_2.outputBuffers  = std::move(vOutBuffers),
            .v3_2.inputBuffer    = vInputBuffers.size() ? vInputBuffers[0] : StreamBuffer{.streamId = -1},
            .v3_2.partialResult  = 0,
        });
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
traceDisplayIf(
    uint32_t frameNo,
    uint64_t timestampShutter,
    const CallbackParcel::ImageItem& item
)   -> void
{
    if  ( ATRACE_ENABLED() )
    {
        String8 s8Trace_HwComposer, s8Trace_HwTexture;

        auto isTarget = [=](MUINT const usage) {
            return  0 != (item.stream->getUsageForConsumer() & usage)
                &&  0 == item.buffer->hasStatus(STREAM_BUFFER_STATUS::ERROR)
                && -1 == item.buffer->getReleaseFence()
                &&  0 != timestampShutter
                    ;
        };

        auto traceTarget = [=](char const* szPrefix, MUINT const usage, String8& str) {
            if  ( isTarget(usage) ) {
                str = String8::format(
                    "Cam:%d:Fwk:%s|timestamp(ns):%" PRId64 " duration(ns):%" PRId64 " u:%" PRIX64 " %dx%d request:%d",
                    mCommonInfo->mInstanceId, szPrefix, timestampShutter, ::systemTime()-timestampShutter,
                    item.stream->getUsageForConsumer(), item.stream->getImgSize().w, item.stream->getImgSize().h,
                    frameNo
                );
                return true;
            }
            return false;
        };

        //Trace display
        (void)(traceTarget("Hwc", GRALLOC_USAGE_HW_COMPOSER, s8Trace_HwComposer)
            || traceTarget("Gpu", GRALLOC_USAGE_HW_TEXTURE, s8Trace_HwTexture));

        if  ( ! s8Trace_HwComposer.isEmpty() ) {
            CAM_TRACE_BEGIN(s8Trace_HwComposer.c_str());
            CAM_TRACE_END();
        }
        else if  ( ! s8Trace_HwTexture.isEmpty() ) {
            CAM_TRACE_BEGIN(s8Trace_HwTexture.c_str());
            CAM_TRACE_END();
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
void
ThisNamespace::
updateStatistics_shutter(uint64_t timestamp, uint32_t const frameNo)
{
    if  (CC_UNLIKELY(timestamp < mStatistics.mLatestTimestamp_Shutter.load().timestamp) ) {
        auto const last = mStatistics.mLatestTimestamp_Shutter.load();
        MY_LOGE(
            "shutter:%" PRId64 " @f%u < shutter:%" PRId64 " @f%u",
            timestamp, frameNo, last.timestamp, last.frameNo
        );
        return;
    }

    mStatistics.mLatestTimestamp_Shutter.store({frameNo, timestamp});

    mStatistics.mFrameCounter_Shutter++;
    switch  (mStatistics.mFrameCounter_Shutter)
    {
    case 1:{
        mStatistics.mStartTime_Shutter = timestamp;
        }break;
    case 120:{
        auto const duration = timestamp - mStatistics.mStartTime_Shutter;
        if  (CC_LIKELY(duration > 0)) {
            uint64_t const fps = (100000000000ULL*mStatistics.mFrameCounter_Shutter) / duration;
            mStatistics.mAvgShutterFps.store(fps);
        }
        mStatistics.mStartTime_Shutter = 0;
        mStatistics.mFrameCounter_Shutter = 0;
        }break;
    default:
        break;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
void
ThisNamespace::
updateStatistics_finalPartialMetadata(bool isFinalPartialMetadata __unused, uint32_t const frameNo __unused)
{
    if  ( isFinalPartialMetadata ) {
        struct timespec ts;
        NSCam::Utils::LogTool::get()->getCurrentLogTime(&ts);
        mStatistics.mLatestTimestamp_FinalPartialResultCallback.store({frameNo, ts});
    }
}


/******************************************************************************
 *
 ******************************************************************************/
void
ThisNamespace::
endStatistics()
{
    auto st = systemTime(SYSTEM_TIME_PROCESS);
    if  ( st - mStatistics.mStartTime_Output >= mStatistics.mMaxDuration_Output ) {
        mStatistics.mStartTime_Output = st;

        std::string strDebugInfo{"[CallbackStatistics]"};
        //
        strDebugInfo += " AvgShutterFps(x100):" + std::to_string(mStatistics.mAvgShutterFps.load());
        //
        CAM_LOGD_IF(1, "%s", strDebugInfo.c_str());
    }
}

