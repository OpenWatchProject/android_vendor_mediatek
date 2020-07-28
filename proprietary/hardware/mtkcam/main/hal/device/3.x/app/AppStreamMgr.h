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

#ifndef _MTK_HARDWARE_MTKCAM_MAIN_HAL_DEVICE_3_X_APP_APPSTREAMMGR_H_
#define _MTK_HARDWARE_MTKCAM_MAIN_HAL_DEVICE_3_X_APP_APPSTREAMMGR_H_
//
#include <IAppStreamManager.h>
//
#include "AppStreamBuffers.h"
#include "Camera3ImageStreamInfo.h"
//
#include <mtkcam/pipeline/utils/streaminfo/MetaStreamInfo.h>
#include <mtkcam/utils/gralloc/IGrallocHelper.h>
#include <mtkcam/utils/debug/debug.h>
//
#include <atomic>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
//
#include <time.h>
//
#include <utils/BitSet.h>
#include <utils/Condition.h>
#include <utils/KeyedVector.h>
#include <utils/List.h>
#include <utils/Mutex.h>
#include <utils/RefBase.h>
#include <utils/StrongPointer.h>
#include <utils/Thread.h>
//
#include <fmq/MessageQueue.h>
#include <hidl/MQDescriptor.h>

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {

// Size of request metadata fast message queue. Change to 0 to always use hwbinder buffer.
static constexpr size_t CAMERA_REQUEST_METADATA_QUEUE_SIZE = 1 << 20 /* 1MB */;
// Size of result metadata fast message queue. Change to 0 to always use hwbinder buffer.
static constexpr size_t CAMERA_RESULT_METADATA_QUEUE_SIZE  = 1 << 20 /* 1MB */;

/**
 * An implementation of App stream manager.
 */
class AppStreamMgr
    : public IAppStreamManager
{
    friend  class IAppStreamManager;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Common Info.

    using   AppImageStreamBuffer = NSCam::v3::AppImageStreamBuffer;
    using   AppMetaStreamBuffer = NSCam::v3::AppMetaStreamBuffer;
    using   AppImageStreamInfo = Camera3ImageStreamInfo;
    using   AppMetaStreamInfo = NSCam::v3::Utils::MetaStreamInfo;


    struct  CommonInfo
    {
        int32_t                                     mLogLevel = 0;
        int32_t                                     mInstanceId = -1;
        std::shared_ptr<android::Printer>           mErrorPrinter = nullptr;
        std::shared_ptr<android::Printer>           mWarningPrinter = nullptr;
        std::shared_ptr<android::Printer>           mDebugPrinter = nullptr;
        android::sp<V3_4::ICameraDeviceCallback>    mDeviceCallback = nullptr;
        android::sp<IMetadataProvider>              mMetadataProvider = nullptr;
        android::sp<IMetadataConverter>             mMetadataConverter = nullptr;
        IGrallocHelper*                             mGrallocHelper = nullptr;
        size_t                                      mAtMostMetaStreamCount = 0;
    };


public:     ////    Debugging

    struct  MyDebuggee : public IDebuggee
    {
        static const std::string        mName;
        std::shared_ptr<IDebuggeeCookie>mCookie = nullptr;
        android::wp<AppStreamMgr>       mContext = nullptr;

                        MyDebuggee(AppStreamMgr* p) : mContext(p) {}
        virtual auto    debuggeeName() const -> std::string { return mName; }
        virtual auto    debug(
                            android::Printer& printer,
                            const std::vector<std::string>& options
                        ) -> void;
    };


public:     ////    Callback

    struct  CallbackParcel
    {
        struct  ImageItem
        {
            android::sp<AppImageStreamBuffer>       buffer;
            android::sp<AppImageStreamInfo>         stream;
        };

        struct  MetaItem
        {
            android::sp<IMetaStreamBuffer>          buffer;
            uint32_t                                bufferNo = 0; //partial_result
        };

        struct  Error
        {
            android::sp<AppImageStreamInfo>         stream;
            ErrorCode                               errorCode = ErrorCode::ERROR_REQUEST;
        };

        struct  Shutter
            : public android::LightRefBase<Shutter>
        {
            uint64_t                                timestamp = 0;
        };

        android::Vector<ImageItem>                  vInputImageItem;
        android::Vector<ImageItem>                  vOutputImageItem;
        android::Vector<MetaItem>                   vOutputMetaItem;
        android::Vector<Error>                      vError;
        android::sp<Shutter>                        shutter;
        uint64_t                                    timestampShutter = 0;
        uint32_t                                    frameNo = 0;
        bool                                        valid = false;
        bool                                        needIgnore = false;
        bool                                        needRemove = false;
    };


public:     ////    Result Queue

    struct  ResultItem
        : public android::LightRefBase<ResultItem>
    {
        uint32_t                                    frameNo = 0;
        bool                                        lastPartial = false; // partial metadata
        android::Vector<android::sp<IMetaStreamBuffer>>
                                                    buffer;
    };

    using   ResultQueueT = android::KeyedVector<uint32_t, android::sp<ResultItem>>;


public:     ////

    //             +---------------------------------------------------------+
    //             |                    Camera Frameworks                    |
    //             +---------------------------------------------------------+
    //                                          ^
    //                                 +-----------------+
    //                                 | CallbackHandler |
    //                                 +-----------------+
    //                                          ^
    //                                 +-----------------+
    //             +---------------+   |   BatchHandler  |   +----------------+
    //  [config] > | ConfigHandler | > |        ^        | < | RequestHandler | < [request]
    //             +---------------+   |   FrameHandler  |   +----------------+
    //                                 +-----------------+
    //                                          ^
    //                                 +----------------+
    //                                 |  ResultHandler |
    //                                 +----------------+
    //                                          ^
    //                                      [result]
    class   BufferHandleCache;
    class   ConfigHandler;
    class   RequestHandler;
    class   ResultHandler;
    class   FrameHandler;
    class   BatchHandler;
    class   CallbackHandler;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                Data Members.
    std::shared_ptr<CommonInfo> mCommonInfo = nullptr;
    std::string const           mInstanceName;
    std::shared_ptr<MyDebuggee> mDebuggee = nullptr;
    mutable android::Mutex      mInterfaceLock;

protected:  ////                Data Members (CONFIG/REQUEST/RESULT/FRAME/CALLBACK)
    android::sp<ConfigHandler>  mConfigHandler = nullptr;
    android::sp<RequestHandler> mRequestHandler = nullptr;
    android::sp<ResultHandler>  mResultHandler = nullptr;
    android::sp<FrameHandler>   mFrameHandler = nullptr;
    android::sp<BatchHandler>   mBatchHandler = nullptr;
    android::sp<CallbackHandler>mCallbackHandler = nullptr;

protected:  ////                Metadata Fast Message Queue (FMQ)
    using RequestMetadataQueue = ::android::hardware::MessageQueue<uint8_t, ::android::hardware::kSynchronizedReadWrite>;
    std::shared_ptr<RequestMetadataQueue>
                                mRequestMetadataQueue;

    using ResultMetadataQueue = ::android::hardware::MessageQueue<uint8_t, ::android::hardware::kSynchronizedReadWrite>;
    std::shared_ptr<ResultMetadataQueue>
                                mResultMetadataQueue;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Operations.
    auto            getLogLevel() const -> int32_t { return mCommonInfo != nullptr ? mCommonInfo->mLogLevel : 0; }
    auto            dumpStateLocked(android::Printer& printer, const std::vector<std::string>& options) -> void;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IAppStreamManager Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Interfaces.
                    AppStreamMgr(const CreationInfo& creationInfo);

    auto            initialize() -> bool;

public:     ////    IAppStreamManager Interfaces.
    virtual auto    destroy() -> void;

    virtual auto    dumpState(android::Printer& printer, const std::vector<std::string>& options) -> void;

    virtual auto    beginConfigureStreams(
                        const V3_4::StreamConfiguration& requestedConfiguration,
                        V3_4::HalStreamConfiguration& halConfiguration,
                        ConfigAppStreams& rStreams
                    )   -> int;

    virtual auto    endConfigureStreams(
                        V3_4::HalStreamConfiguration& halConfiguration
                    )   -> int;

    virtual auto    getCaptureRequestMetadataQueue() -> const ::android::hardware::MQDescriptorSync<uint8_t>&;

    virtual auto    getCaptureResultMetadataQueue() -> const ::android::hardware::MQDescriptorSync<uint8_t>&;

    virtual auto    flushRequest(
                        const hidl_vec<V3_4::CaptureRequest>& requests
                    )   -> void;

    virtual auto    removeBufferCache(
                        const hidl_vec<BufferCache>& cachesToRemove
                    )   -> void;

    virtual auto    submitRequest(
                        const hidl_vec<V3_4::CaptureRequest>& requests,
                        android::Vector<Request>& rRequests
                    )   -> int;

    virtual auto    updateResult(
                        uint32_t const frameNo,
                        intptr_t const userId,
                        android::Vector<android::sp<IMetaStreamBuffer>> resultMeta,
                        bool hasLastPartial
                    )   -> void;

    virtual auto    waitUntilDrained(nsecs_t const timeout) -> int;
};


/**
 * Buffer Handle Cache
 */
class AppStreamMgr::BufferHandleCache
{
protected:
    struct  BufferHandle
    {
        uint64_t        bufferId = 0;
        buffer_handle_t bufferHandle = nullptr;
        int32_t         ionFd = -1;
        struct timespec cachedTime;
    };

    // key: bufferId sent via HIDL interface
    // value: imported buffer_handle_t
    using BufferHandleMap = std::unordered_map<uint64_t, BufferHandle>;
    BufferHandleMap             mBufferHandleMap;
    std::string const           mInstanceName;
    std::shared_ptr<CommonInfo> mCommonInfo = nullptr;
    StreamId_T                  mStreamId = -1;

public:
                        ~BufferHandleCache();
                        BufferHandleCache(std::shared_ptr<CommonInfo> pCommonInfo, StreamId_T streamId);

    auto                dumpState(android::Printer& printer, uint32_t indent = 0) const -> void;

    auto                importBuffer(
                            uint64_t bufferId,
                            buffer_handle_t& importedBufferHandle
                        )   const -> int;

    auto                cacheBuffer(
                            uint64_t bufferId,
                            buffer_handle_t importedBufferHandle
                        )   -> int;

    auto                uncacheBuffer(uint64_t bufferId) -> void;

    auto                clear() -> void;

};


/**
 * Config Handler
 */
class AppStreamMgr::ConfigHandler
    : public android::RefBase
{
    friend  class AppStreamMgr;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                Data Members.
    std::string const           mInstanceName;
    std::shared_ptr<CommonInfo> mCommonInfo = nullptr;
    android::sp<FrameHandler>   mFrameHandler = nullptr;
    android::sp<BatchHandler>   mBatchHandler = nullptr;
    IMetadata::IEntry           mEntryMinDuration;
    IMetadata::IEntry           mEntryStallDuration;

protected:  ////    Operations.
    auto            getLogLevel() const -> int32_t { return mCommonInfo != nullptr ? mCommonInfo->mLogLevel : 0; }

    auto            checkStream(
                        const V3_4::Stream& rStream
                    )   const -> int;

    auto            checkStreams(
                        const hidl_vec<V3_4::Stream>& vStreamConfigured
                    )   const -> int;

    auto            createImageStreamInfo(
                        const V3_4::Stream& rStream,
                        V3_4::HalStream& rOutStream
                    )   const -> AppImageStreamInfo*;

    auto            createMetaStreamInfo(
                        StreamId_T suggestedStreamId
                    )   const -> AppMetaStreamInfo*;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Operations.
                    ConfigHandler(
                        std::shared_ptr<CommonInfo> pCommonInfo,
                        android::sp<FrameHandler> pFrameHandler,
                        android::sp<BatchHandler> pBatchHandler
                    );

    virtual auto    destroy() -> void;

    virtual auto    beginConfigureStreams(
                        const V3_4::StreamConfiguration& requestedConfiguration,
                        V3_4::HalStreamConfiguration& halConfiguration,
                        ConfigAppStreams& rStreams
                    )   -> int;

    virtual auto    endConfigureStreams(
                        V3_4::HalStreamConfiguration& halConfiguration
                    )   -> int;
};


/**
 * Request Handler
 */
class AppStreamMgr::RequestHandler
    : public android::RefBase
{
    friend  class AppStreamMgr;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    std::string const           mInstanceName;
    std::shared_ptr<CommonInfo> mCommonInfo = nullptr;
    android::sp<FrameHandler>   mFrameHandler = nullptr;
    android::sp<BatchHandler>   mBatchHandler = nullptr;

    std::shared_ptr<RequestMetadataQueue>
                                mRequestMetadataQueue = nullptr;
    IMetadata                   mLatestSettings;

protected:  ////

    auto            getLogLevel() const -> int32_t { return mCommonInfo != nullptr ? mCommonInfo->mLogLevel : 0; }

    auto            registerRequests(
                        const android::Vector<Request>& rRequests
                    )   -> int;

    auto            createRequests(
                        const hidl_vec<V3_4::CaptureRequest>& requests,
                        android::Vector<Request>& rRequests
                    )   -> int;

    auto            checkOneRequest(
                        const V3_4::CaptureRequest& request
                    )   const -> int;

    auto            createOneRequest(
                        const V3_4::CaptureRequest& request,
                        Request& rRequest
                    )   -> int;

    auto            convertStreamBuffer(
                        const std::string& bufferName,
                        const StreamBuffer& streamBuffer,
                        android::sp<AppImageStreamBuffer>& pStreamBuffer
                    )   const -> int;

    auto            importStreamBuffer(
                        const StreamBuffer& streamBuffer,
                        buffer_handle_t& bufferHandle,
                        int& acquire_fence
                    )   const -> int;

    auto            createImageStreamBuffer(
                        const std::string& bufferName,
                        const StreamBuffer& streamBuffer,
                        buffer_handle_t bufferHandle,
                        int const acquire_fence,
                        int const release_fence
                    )   const -> AppImageStreamBuffer*;

    auto            createMetaStreamBuffer(
                        android::sp<IMetaStreamInfo> pStreamInfo,
                        IMetadata const& rSettings,
                        bool const repeating
                    )   const -> AppMetaStreamBuffer*;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Operations.
                    RequestHandler(
                        std::shared_ptr<CommonInfo> pCommonInfo,
                        std::shared_ptr<RequestMetadataQueue> pRequestMetadataQueue,
                        android::sp<FrameHandler> pFrameHandler,
                        android::sp<BatchHandler> pBatchHandler
                    );

    virtual auto    destroy() -> void;

    virtual auto    reset() -> void;

    virtual auto    submitRequest(
                        const hidl_vec<V3_4::CaptureRequest>& requests,
                        android::Vector<Request>& rRequests
                    )   -> int;

};


/**
 * Result Handler
 */
class AppStreamMgr::ResultHandler
    : public android::Thread
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    std::string const           mInstanceName;
    std::shared_ptr<CommonInfo> mCommonInfo = nullptr;
    android::sp<FrameHandler>   mFrameHandler = nullptr;

protected:  ////
    mutable android::Mutex      mResultQueueLock;
    android::Condition          mResultQueueCond;
    ResultQueueT                mResultQueue;

protected:  ////

    auto            getLogLevel() const -> int32_t { return mCommonInfo != nullptr ? mCommonInfo->mLogLevel : 0; }
    auto            dequeResult(ResultQueueT& rvResult) -> int;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Thread Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    // Ask this object's thread to exit. This function is asynchronous, when the
    // function returns the thread might still be running. Of course, this
    // function can be called from a different thread.
    virtual auto    requestExit() -> void;

    // Good place to do one-time initializations
    virtual auto    readyToRun() -> android::status_t;

private:
    // Derived class must implement threadLoop(). The thread starts its life
    // here. There are two ways of using the Thread object:
    // 1) loop: if threadLoop() returns true, it will be called again if
    //          requestExit() wasn't called.
    // 2) once: if threadLoop() returns false, the thread will exit upon return.
    virtual auto    threadLoop() -> bool;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Operations.
                    ResultHandler(
                        std::shared_ptr<CommonInfo> pCommonInfo,
                        android::sp<FrameHandler> pFrameHandler
                    );

    virtual auto    destroy() -> void;

    auto            enqueResult(
                        uint32_t const frameNo,
                        intptr_t const userId,
                        android::Vector<android::sp<IMetaStreamBuffer>> resultMeta,
                        bool hasLastPartial
                    )   -> int;

};


/**
 * Frame Handler
 */
class AppStreamMgr::FrameHandler
    : public android::RefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions:
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    struct  FrameParcel;
    struct  MetaItem;
    struct  MetaItemSet;
    struct  ImageItem;
    struct  ImageItemSet;

    /**
     * IN_FLIGHT    -> PRE_RELEASE
     * IN_FLIGHT    -> VALID
     * IN_FLIGHT    -> ERROR
     * PRE_RELEASE  -> VALID
     * PRE_RELEASE  -> ERROR
     */
    struct  State
    {
        enum T
        {
            IN_FLIGHT,
            PRE_RELEASE,
            VALID,
            ERROR,
        };
    };

    struct  HistoryBit
    {
        enum
        {
            RETURNED,
            ERROR_SENT_FRAME,
            ERROR_SENT_META,
            ERROR_SENT_IMAGE,
        };
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions: Meta Stream
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    struct  MetaItem
        : public android::RefBase
    {
        FrameParcel*                        pFrame = nullptr;
        MetaItemSet*                        pItemSet = nullptr;
        State::T                            state = State::IN_FLIGHT;
        android::BitSet32                   history;//HistoryBit::xxx
        android::sp<IMetaStreamBuffer>      buffer;
        MUINT32                             bufferNo = 0;//partial_result
    };

    struct  MetaItemSet
        : public android::DefaultKeyedVector<StreamId_T, android::sp<MetaItem> >
    {
        bool                                asInput;
        bool                                hasLastPartial;
        size_t                              numReturnedStreams;
        size_t                              numValidStreams;
        size_t                              numErrorStreams;
                                            //
                                            MetaItemSet(MBOOL _asInput)
                                                : asInput(_asInput)
                                                , hasLastPartial(false)
                                                , numReturnedStreams(0)
                                                , numValidStreams(0)
                                                , numErrorStreams(0)
                                            {}
    };

    struct  MetaConfigItem
    {
        android::sp<AppMetaStreamInfo>      pStreamInfo;
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions: Image Stream
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    struct  ImageItemFrameQueue
        : public android::List<android::sp<ImageItem> >
    {
                                            ImageItemFrameQueue()
                                            {}
    };

    struct  ImageItem
        : public android::RefBase
    {
        FrameParcel*                        pFrame = nullptr;
        ImageItemSet*                       pItemSet = nullptr;
        State::T                            state = State::IN_FLIGHT;
        android::BitSet32                   history;//HistoryBit::xxx
        android::sp<AppImageStreamBuffer>   buffer;
        ImageItemFrameQueue::iterator       iter;
    };

    struct  ImageItemSet
        : public android::DefaultKeyedVector<StreamId_T, android::sp<ImageItem> >
    {
        MBOOL                               asInput;
        size_t                              numReturnedStreams;
        size_t                              numValidStreams;
        size_t                              numErrorStreams;
                                            //
                                            ImageItemSet(MBOOL _asInput)
                                                : asInput(_asInput)
                                                , numReturnedStreams(0)
                                                , numValidStreams(0)
                                                , numErrorStreams(0)
                                            {}
    };

    struct  ImageConfigItem
    {
        android::sp<AppImageStreamInfo>     pStreamInfo;
        ImageItemFrameQueue                 vItemFrameQueue;
        std::shared_ptr<BufferHandleCache>  pBufferHandleCache;
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions: Frame Parcel & Queue
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    struct  FrameParcel
        : public android::RefBase
    {
        ImageItemSet                        vOutputImageItem = MFALSE;
        ImageItemSet                        vInputImageItem = MTRUE;
        MetaItemSet                         vOutputMetaItem = MFALSE;
        MetaItemSet                         vInputMetaItem = MTRUE;
        MUINT32                             frameNo = 0;
        android::BitSet32                   errors; //HistoryBit::ERROR_SENT_xxx
        MUINT64                             shutterTimestamp = 0;
        bool                                bShutterCallbacked = false;
        timespec                            requestTimestamp{0, 0};
    };

    struct  FrameQueue
        : public android::List<android::sp<FrameParcel> >
    {
        MUINT32                             latestResultFrameNo = 0;
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Data Members.
    std::string const                                   mInstanceName;
    std::shared_ptr<CommonInfo>                         mCommonInfo = nullptr;
    android::sp<BatchHandler>                           mBatchHandler = nullptr;

protected:  ////    Data Members (CONFIG/REQUEST)
    mutable android::Mutex                              mFrameQueueLock;
    android::Condition                                  mFrameQueueCond;
    FrameQueue                                          mFrameQueue;
    android::KeyedVector<StreamId_T, ImageConfigItem>   mImageConfigMap;
    android::KeyedVector<StreamId_T, MetaConfigItem>    mMetaConfigMap;
    uint32_t                                            mOperationMode = 0xffffffff;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations:
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Operations.
    auto            dumpStateLocked(android::Printer& printer) const -> void;
    auto            dumpLocked() const -> void;
    auto            dump() const -> void;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations: Request
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Operations.
    virtual auto    registerStreamBuffers(
                        android::KeyedVector<
                            StreamId_T,
                            android::sp<IImageStreamBuffer>
                                            > const& buffers,
                        android::sp<FrameParcel> const pFrame,
                        ImageItemSet*const pItemSet
                    )   -> int;

    virtual auto    registerStreamBuffers(
                        android::KeyedVector<
                            StreamId_T,
                            android::sp<IMetaStreamBuffer>
                                            > const& buffers,
                        android::sp<FrameParcel> const pFrame,
                        MetaItemSet*const pItemSet
                    )   -> int;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations: Result
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Operations.

    /**
     * @param[in] frame: a given frame to check.
     *
     * @return
     *      ==0: uncertain
     *      > 0: it is indeed a request error
     *      < 0: it is indeed NOT a request error
     */
    static  MINT    checkRequestError(FrameParcel const& frame);

protected:  ////    Operations.
    virtual auto    prepareErrorFrame(
                        CallbackParcel& rCbParcel,
                        android::sp<FrameParcel> const& pFrame
                    )   -> void;

    virtual auto    prepareErrorMetaIfPossible(
                        CallbackParcel& rCbParcel,
                        android::sp<MetaItem> const& pItem
                    )   -> void;

    virtual auto    prepareErrorImage(
                        CallbackParcel& rCbParcel,
                        android::sp<ImageItem> const& pItem
                    )   -> void;

protected:  ////    Operations.
    virtual auto    isShutterReturnable(
                        android::sp<MetaItem> const& pItem
                    )   const -> bool;

    virtual auto    prepareShutterNotificationIfPossible(
                        CallbackParcel& rCbParcel,
                        android::sp<MetaItem> const& pItem
                    )   -> bool;

    virtual auto    prepareReturnMeta(
                        CallbackParcel& rCbParcel,
                        android::sp<MetaItem> const& pItem
                    )   -> void;

    virtual auto    isReturnable(
                        android::sp<MetaItem> const& pItem
                    )   const -> bool;

protected:  ////    Operations.
    virtual auto    prepareReturnImage(
                        CallbackParcel& rCbParcel,
                        android::sp<ImageItem> const& pItem
                    )   -> void;

    virtual auto    isReturnable(
                        android::sp<ImageItem> const& pItem
                    )   const -> bool;

protected:  ////    Operations.
    virtual auto    isFrameRemovable(
                        android::sp<FrameParcel> const& pFrame
                    )   const -> bool;

    virtual auto    prepareCallbackIfPossible(
                        CallbackParcel& rCbParcel,
                        MetaItemSet& rItemSet
                    )   -> bool;

    virtual auto    prepareCallbackIfPossible(
                        CallbackParcel& rCbParcel,
                        ImageItemSet& rItemSet
                    )   -> bool;

protected:  ////    Operations.
    virtual auto    updateItemSet(MetaItemSet& rItemSet) -> void;
    virtual auto    updateItemSet(ImageItemSet& rItemSet) -> void;
    virtual auto    updateResult(ResultQueueT const& rvResult) -> void;
    virtual auto    updateCallback(std::list<CallbackParcel>& rCbList) -> void;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Operations.
                    FrameHandler(
                        std::shared_ptr<CommonInfo> pCommonInfo,
                        android::sp<BatchHandler> pBatchHandler
                    );

    virtual auto    destroy() -> void;

    virtual auto    dumpState(android::Printer& printer, const std::vector<std::string>& options) -> void;

    virtual auto    setOperationMode(uint32_t operationMode) -> void;

    virtual auto    removeUnusedConfigStream(std::unordered_set<StreamId_T>const& usedStreamIds) -> void;
    virtual auto    addConfigStream(AppImageStreamInfo* pStreamInfo, bool keepBufferCache) -> void;
    virtual auto    addConfigStream(AppMetaStreamInfo* pStreamInfo) -> void;
    virtual auto    getConfigImageStream(StreamId_T streamId) const -> android::sp<AppImageStreamInfo>;
    virtual auto    getConfigMetaStream(size_t index) const -> android::sp<AppMetaStreamInfo>;

    virtual auto    importBuffer(StreamId_T streamId, uint64_t bufferId, buffer_handle_t& importedBufferHandle) -> int;
    virtual auto    uncacheBuffer(StreamId_T streamId, uint64_t bufferId) -> void;
    virtual auto    cacheBuffer(StreamId_T streamId, uint64_t bufferId, buffer_handle_t& handle) -> void;

    virtual auto    registerFrame(Request const& rRequest) -> int;

    virtual auto    update(ResultQueueT const& rvResult) -> void;

    virtual auto    waitUntilDrained(nsecs_t const timeout) -> int;

};


/**
 * Batch Handler
 */
class AppStreamMgr::BatchHandler
    : public android::RefBase
{
    friend  class AppStreamMgr;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
        struct InFlightBatch {
            // Protect access to entire struct. Acquire this lock before read/write any data or
            // calling any methods. processCaptureResult and notify will compete for this lock
            // HIDL IPCs might be issued while the lock is held
            mutable android::Mutex  mLock;

            uint32_t                mBatchNo;
            uint32_t                mFirstFrame;
            uint32_t                mLastFrame;
            uint32_t                mBatchSize;
            std::vector<uint32_t>   mHasLastPartial;
            std::vector<uint32_t>   mShutterReturned;
            std::vector<uint32_t>   mFrameRemoved;
            //bool                    mRemoved = false;
            uint32_t                mFrameHasMetaResult = 0;
            uint32_t                mFrameHasImageResult = 0;
            uint32_t                mFrameRemovedCount = 0;
       };

protected:  ////
    std::string const               mInstanceName;
    std::shared_ptr<CommonInfo>     mCommonInfo = nullptr;
    android::sp<CallbackHandler>    mCallbackHandler = nullptr;

    // to record id of batched streams.
    std::vector<uint32_t>           mBatchedStreams;
    uint32_t                        mBatchCounter;
    mutable android::Mutex          mLock;
    std::vector<std::shared_ptr<InFlightBatch>>
                                    mInFlightBatches;
    mutable android::Mutex          mMergedParcelLock;
    std::list<CallbackParcel>       mMergedParcels;

public:
    enum { NOT_BATCHED = -1 };

protected:  ////

    auto            getLogLevel() const -> int32_t { return mCommonInfo != nullptr ? mCommonInfo->mLogLevel : 0; }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Operations.
                    BatchHandler(
                        std::shared_ptr<CommonInfo> pCommonInfo,
                        android::sp<CallbackHandler> pCallbackHandler
                    );

    virtual auto    destroy() -> void;

    virtual auto    dumpState(android::Printer& printer, const std::vector<std::string>& options) -> void;

    virtual auto    dumpStateLocked(android::Printer& printer) const -> void;

    virtual auto    waitUntilDrained(nsecs_t const timeout) -> int;

    virtual auto    push(std::list<CallbackParcel>& item) -> void;

    // Operations to be modified.
    virtual auto resetBatchStreamId() -> void;

    virtual auto checkStreamUsageforBatchMode(const android::sp<AppImageStreamInfo>) -> bool;

    virtual auto registerBatch(const android::Vector<Request>) -> int;

    virtual auto getBatchLocked(uint32_t frameNumber) -> std::shared_ptr<InFlightBatch>;

    virtual auto removeBatchLocked(uint32_t batchNum) -> void;

    virtual auto appendParcel(CallbackParcel srcParcel,CallbackParcel &dstParcel) -> void;

    virtual auto updateCallback(std::list<CallbackParcel> cbParcels, std::list<CallbackParcel>& rUpdatedList) -> void;
};


/**
 * Callback Handler
 */
class AppStreamMgr::CallbackHandler
    : public android::Thread
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions:
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    struct  CallbackQueue
        : public std::list<CallbackParcel>
    {
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Data Members.
    std::string const                       mInstanceName;
    std::shared_ptr<CommonInfo>             mCommonInfo = nullptr;
    std::shared_ptr<ResultMetadataQueue>    mResultMetadataQueue = nullptr;

    mutable android::Mutex                  mQueue1Lock;
    android::Condition                      mQueue1Cond;
    CallbackQueue                           mQueue1;

    mutable android::Mutex                  mQueue2Lock;
    android::Condition                      mQueue2Cond;
    CallbackQueue                           mQueue2;

protected:  ////    Operations.
    auto            getLogLevel() const -> int32_t { return mCommonInfo != nullptr ? mCommonInfo->mLogLevel : 0; }

    auto            waitUntilQueue1NotEmpty() -> bool;

    auto            performCallback() -> void;

    auto            convertShutterToHidl(CallbackParcel const& cbParcel, std::vector<NotifyMsg>& rvMsg) -> void;
    auto            convertErrorToHidl(CallbackParcel const& cbParcel, std::vector<NotifyMsg>& rvMsg) -> void;
    auto            convertMetaToHidl(CallbackParcel const& cbParcel, std::vector<V3_4::CaptureResult>& rvResult) -> void;
    auto            convertImageToHidl(CallbackParcel const& cbParcel, std::vector<V3_4::CaptureResult>& rvResult) -> void;

protected:  ////    Operations (Trace)
    auto            traceDisplayIf(
                        uint32_t frameNo,
                        uint64_t timestampShutter,
                        const CallbackParcel::ImageItem& item
                    )   -> void;

protected:  ////    Debug (Statistics)

    struct  Statistics
    {
        //  fps / duration
        static int64_t const        mMaxDuration_Output = 3000000000UL;
        int64_t                     mStartTime_Output = 0;
        int64_t                     mStartTime_Shutter = 0;
        uint32_t                    mFrameCounter_Shutter = 0;
        std::atomic_uint64_t        mAvgShutterFps{0};

        //  shutter timestamp
        struct Shutter
        {
            uint32_t    frameNo;
            uint64_t    timestamp;
        };
        std::atomic<Shutter>        mLatestTimestamp_Shutter{Shutter{0, 0}};

        //  partial result
        struct PartialResult
        {
            uint32_t    frameNo;
            timespec    ts;
        };
        std::atomic<PartialResult>  mLatestTimestamp_FinalPartialResultCallback{PartialResult{0, timespec{0, 0}}};

    }               mStatistics;

    auto            beginStatistics() -> void {}
    auto            endStatistics() -> void;
    auto            updateStatistics_shutter(uint64_t timestamp, uint32_t const frameNo) -> void ;
    auto            updateStatistics_finalPartialMetadata(bool isFinalPartialMetadata, uint32_t const frameNo) -> void ;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Thread Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    // Ask this object's thread to exit. This function is asynchronous, when the
    // function returns the thread might still be running. Of course, this
    // function can be called from a different thread.
    virtual auto    requestExit() -> void;

    // Good place to do one-time initializations
    virtual auto    readyToRun() -> android::status_t;

private:
    // Derived class must implement threadLoop(). The thread starts its life
    // here. There are two ways of using the Thread object:
    // 1) loop: if threadLoop() returns true, it will be called again if
    //          requestExit() wasn't called.
    // 2) once: if threadLoop() returns false, the thread will exit upon return.
    virtual auto    threadLoop() -> bool;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Operations.

                    CallbackHandler(
                        std::shared_ptr<CommonInfo> pCommonInfo,
                        std::shared_ptr<ResultMetadataQueue> pResultMetadataQueue
                    );

    virtual auto    destroy() -> void;

    virtual auto    dumpState(android::Printer& printer, const std::vector<std::string>& options) -> void;

    virtual auto    waitUntilDrained(nsecs_t const timeout) -> int;

    virtual auto    push(std::list<CallbackParcel>& item) -> void;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_MAIN_HAL_DEVICE_3_X_APP_APPSTREAMMGR_H_

