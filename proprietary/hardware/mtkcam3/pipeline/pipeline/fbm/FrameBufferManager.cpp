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

#define LOG_TAG "mtkcam-ppl_fbm"

#include "FrameBufferManager.h"
//
#include <algorithm>
#include <chrono>
#include <functional>
//
#include <cutils/compiler.h>
#include <cutils/properties.h>
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/utils/std/Aee.h>
//
#include <mtkcam/utils/imgbuf/ISecureImageBufferHeap.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_CONTEXT);

using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v3::pipeline;
using namespace NSCam::v3::pipeline::NSPipelineContext;

using HalImageStreamBufferT = NSCam::v3::Utils::HalImageStreamBuffer;
using AllocFuncT = BufferProducer::AllocFuncT;
using CallbackFuncT = BufferProducer::CallbackFuncT;

#define ThisNamespace   FrameBufferManagerImpl

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if (            (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if (            (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if (            (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

/******************************************************************************
 *
 ******************************************************************************/
static inline std::string toString_ConfigImageStream(const ThisNamespace::ConfigImageStream& o)
{
    std::string os;

    os += toString(o.attribute);

    if (CC_LIKELY( o.pStreamInfo != nullptr )) {
        os += o.pStreamInfo->toString();
    }

    return os;
};


/******************************************************************************
 *
 ******************************************************************************/
static inline std::string toString_ConfigMetaStream(const ThisNamespace::ConfigMetaStream& o)
{
    std::string os;

    os += toString(o.attribute);

    if (CC_LIKELY( o.pStreamInfo != nullptr )) {
        os += o.pStreamInfo->toString();
    }

    return os;
};


/******************************************************************************
 *
 ******************************************************************************/
static auto
generateImgParam(
    IImageStreamInfo const* pStreamInfo
) -> IImageBufferAllocator::ImgParam
{
    IImageStreamInfo::BufPlanes_t const& bufPlanes = pStreamInfo->getAllocBufPlanes();
    switch (pStreamInfo->getAllocImgFormat())
    {
    case eImgFmt_BLOB:{
        return  IImageBufferAllocator::ImgParam(
                    bufPlanes.planes[0].sizeInBytes,
                    0
                );
        }break;

    case eImgFmt_JPEG:{
        return  IImageBufferAllocator::ImgParam(
                    pStreamInfo->getImgSize(),
                    bufPlanes.planes[0].sizeInBytes,
                    0
                );
        }break;

    default:
        break;
    }

    size_t bufStridesInBytes[3] = {0};
    size_t bufBoundaryInBytes[3]= {0};
    size_t bufCustomSizeInBytes[3] = {0};
    for (size_t i = 0; i < bufPlanes.count; i++) {
        bufStridesInBytes[i] = bufPlanes.planes[i].rowStrideInBytes;
        bufCustomSizeInBytes[i] = bufPlanes.planes[i].sizeInBytes;
    }
    return  IImageBufferAllocator::ImgParam(
                pStreamInfo->getAllocImgFormat(),
                pStreamInfo->getImgSize(),
                bufStridesInBytes, bufBoundaryInBytes,
                bufCustomSizeInBytes,
                bufPlanes.count
            );
}


/******************************************************************************
 *
 ******************************************************************************/
static bool isSameAllocLayout(IImageStreamInfo const* s1, IImageStreamInfo const* s2)
{
    //CAM_ULOGME("<CONFIG > %s", s1->toString().c_str());
    //CAM_ULOGME("<REQUEST> %s", s2->toString().c_str());

    if ( s1 == s2 )
        return true;

    if ( s1->getAllocImgFormat() != s2->getAllocImgFormat() )
        return false;

    auto const& bp1 = s1->getAllocBufPlanes();
    auto const& bp2 = s2->getAllocBufPlanes();
    if ( bp1.count != bp2.count )
        return false;

    for (size_t i = 0; i < bp1.count; i++) {
        if ( bp1.planes[i].sizeInBytes != bp2.planes[i].sizeInBytes
          || bp1.planes[i].rowStrideInBytes != bp2.planes[i].rowStrideInBytes ) {
            return false;
        }
    }

    return true;
}


/******************************************************************************
 *
 ******************************************************************************/
 namespace
 {
    struct Allocator
    {
        ////    Data Members.
        android::sp<IImageStreamInfo>           mpStreamInfo;
        IImageBufferAllocator::ImgParam         mAllocImgParam;

        ////    Operations.
        Allocator(
            IImageStreamInfo* pStreamInfo,
            IImageBufferAllocator::ImgParam&& rAllocImgParam
        ) : mpStreamInfo(pStreamInfo), mAllocImgParam(rAllocImgParam) {}

        ////    Operator.
        IImageBufferHeap* operator()(const std::shared_ptr<IIonDevice>& pIonDevice = nullptr)
        {
            IImageBufferHeap* pImageBufferHeap = NULL;
            if (mpStreamInfo->isSecure())
            {
                pImageBufferHeap = ISecureImageBufferHeap::create(
                                        mpStreamInfo->getStreamName(),
                                        mAllocImgParam,
                                        ISecureImageBufferHeap::AllocExtraParam(0,1,0,MFALSE,mpStreamInfo->getSecureInfo().type),
                                        MFALSE
                                    );
                //
                if (CC_UNLIKELY(pImageBufferHeap == nullptr)) {
                    MY_LOGE("ISecureImageBufferHeap::create: %s", mpStreamInfo->toString().c_str());
                    return nullptr;
                }
            }
            else
            {
                pImageBufferHeap =
                    IIonImageBufferHeap::create(
                        mpStreamInfo->getStreamName(),
                        mAllocImgParam,
                        [&pIonDevice](){
                            IIonImageBufferHeap::AllocExtraParam tmp;
                            tmp.ionDevice = pIonDevice;
                            return tmp;
                        }(),
                        MFALSE
                    );
                //
                if (CC_UNLIKELY(pImageBufferHeap == nullptr)) {
                    MY_LOGE("IIonImageBufferHeap::create: %s", mpStreamInfo->toString().c_str());
                    return nullptr;
                }
            }
            return pImageBufferHeap;
        }
    };
 }


/******************************************************************************
 *
 ******************************************************************************/
static auto
allocateHalImageStreamBufferRuntime(
    IImageStreamInfo* pStreamInfo,
    android::sp<IImageStreamBuffer>& rpStreamBuffer
) -> int
{
    rpStreamBuffer = new HalImageStreamBufferT(
                            pStreamInfo,
                            nullptr,
                            Allocator(pStreamInfo, generateImgParam(pStreamInfo))()
                        );

    if (CC_UNLIKELY(rpStreamBuffer == nullptr)) {
        MY_LOGE("<REQUEST> %s", pStreamInfo->toString().c_str());
        return UNKNOWN_ERROR;
    }
    //MY_LOGD("[%s] runtime alloc imageSB (%p)", pStreamInfo->toString().c_str(), rpStreamBuffer.get());  //@@
    return OK;
};


/******************************************************************************
 *
 ******************************************************************************/
static auto
createHalStreamBufferPool(
    const char* username,
    android::sp<IImageStreamInfo>const& pStreamInfo,
    CallbackFuncT const& callbackFunc
) -> android::sp<BufferProducer>
{
    std::string const name = std::string(username) + ":" + pStreamInfo->getStreamName();

    sp<BufferProducer> pPool = new BufferProducer(pStreamInfo, callbackFunc);  // implment w/ Heap_Pool + SB_Producer

    if  (CC_UNLIKELY( pPool == nullptr )) {
        MY_LOGE("[%s] Failed to create a pool for %s", pStreamInfo->toString().c_str(), name.c_str());
        return nullptr;
    }

    int err = pPool->initProducer(
                        pStreamInfo->getStreamName(),
                        pStreamInfo->getMaxBufNum(),
                        pStreamInfo->getMinInitBufNum(),
                        Allocator(
                            pStreamInfo.get(),
                            generateImgParam(pStreamInfo.get())
                        )
                    );

    if  (CC_UNLIKELY( 0 != err )) {
        MY_LOGE("[%s] (%p) failed on initPool - err:%d(%s)", pStreamInfo->toString().c_str(), pPool.get(), err, ::strerror(-err));
        return nullptr;
    }

    return pPool;
}


/******************************************************************************
 *
 ******************************************************************************/
template <class T>
void releaseBuffer(T/*android::sp<IImageStreamBuffer>*/& rpBuffer)
{
    rpBuffer->releaseBuffer();  // return to the pool
    rpBuffer = nullptr;
}

template <>
void releaseBuffer(android::sp<IMetaStreamBuffer>& rpBuffer)
{
    rpBuffer = nullptr;
}

/******************************************************************************
 *
 ******************************************************************************/
template <typename T> inline
std::string toString_BuffersStreamId(const T& vBuffers)
{
    std::ostringstream oss;
    oss << std::showbase << std::hex;
    for (auto const& buf : vBuffers) {
        oss << " (" << buf->getStreamInfo()->getStreamId() << ") ";
    }
    oss << std::dec;
    return oss.str();
}


/******************************************************************************
 *
 ******************************************************************************/
extern "C" void
createFrameBufferManager(
    std::shared_ptr<IFrameBufferManager>* out,
    char const* name
)
{
    if (CC_UNLIKELY( out == nullptr )) {
        return;
    }

    auto pInst = std::make_shared<FrameBufferManagerImpl>(name);
    if (CC_UNLIKELY( pInst == nullptr )) {
        return;
    }

    bool ret = pInst->initialize(pInst);
    if (CC_UNLIKELY( ! ret )) {
        return;
    }

    *out = pInst;
}


/******************************************************************************
 *
 ******************************************************************************/
ThisNamespace::
FrameBufferManagerImpl(char const* name)
    : mName(name)
    , mLogLevel(::property_get_int32("vendor.debug.camera.log.pipeline.fbm", 0))
    , mAcquireTimeoutMS(::property_get_int32("vendor.debug.camera.pipeline.fbm.acquire.timeout", 1500))
{
}


/******************************************************************************
 *
 ******************************************************************************/
ThisNamespace::
~FrameBufferManagerImpl()
{
    MY_LOGI("+ mDebuggeeCookie:%p", mDebuggeeCookie.get());
    if (auto pDbgMgr = IDebuggeeManager::get()) {
        pDbgMgr->detach(mDebuggeeCookie);
    }
    mDebuggeeCookie = nullptr;
    MY_LOGI("-");
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
initialize(std::shared_ptr<ThisT> sp) -> bool
{
    if (CC_LIKELY(sp != nullptr)) {
        if (auto pDbgMgr = IDebuggeeManager::get()) {
            mDebuggeeCookie = pDbgMgr->attach(sp, 0);
        }
    }
    //
    mWeakThis = sp;
    return true;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
dumpState(
    android::Printer& printer
) -> void
{
    dumpConfigStreams(printer);
    dumpBufferState(printer);
    std::string memberStr = "mInSelect(" + std::to_string(mInSelect.load()) + ") " +
                            "mPreOrderAmount(" + std::to_string(mPreOrderAmount.load()) + ") ";
    printer.printLine(memberStr.c_str());
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
dumpBufferState(
    android::Printer& printer
) -> void
{
    {
        std::unique_lock _li(mInFlightLock, std::defer_lock);
        if (_li.try_lock_for(std::chrono::milliseconds(500))) {
            dumpInFlightListLocked(printer);
        } else {
            printer.printLine("Timeout waiting for mInFlightLock");
        }
    }
    {
        std::unique_lock _lr(mReadyLock, std::defer_lock);
        if (_lr.try_lock_for(std::chrono::milliseconds(500))) {
            dumpReadyListLocked(printer);
        } else {
            printer.printLine("Timeout waiting for mReadyLock");
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
dumpConfigStreams(
    android::Printer& printer
) -> void
{
    {
        printer.printLine("<image streams>");
        auto tmp = [this](){ android::RWLock::AutoRLock _l(mConfigLock); return mConfigImageStreams; }();
        for (auto const& v : tmp) {
            if ( auto p = v.second.get() ) {
                printer.printFormatLine("    %s", toString_ConfigImageStream(*p).c_str());
            }
        }
    }
    {
        printer.printLine("<meta streams>");
        auto tmp = [this](){ android::RWLock::AutoRLock _l(mConfigLock); return mConfigMetaStreams; }();
        for (auto const& v : tmp) {
            if ( auto p = v.second.get() ) {
                printer.printFormatLine("    %s", toString_ConfigMetaStream(*p).c_str());
            }
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
dumpInFlightListLocked(
    android::Printer& printer
) -> void
{
    printer.printLine("<InFlightList>");
    for (auto const& fbs : mInFlightList) {
        printer.printFormatLine("    [R%u F%u]TS:%" PRIu64 " #%zu left {%s} {%s} {%s}",
                                fbs.requestNo, fbs.frameNo, fbs.sensorTimestamp, fbs.trackCnt,
                                toString_BuffersStreamId(fbs.vpHalImageStreamBuffers).c_str(),
                                toString_BuffersStreamId(fbs.vpHalMetaStreamBuffers).c_str(),
                                toString_BuffersStreamId(fbs.vpAppMetaStreamBuffers).c_str());
        //TODO: TrackFrameResultParams??
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
dumpReadyListLocked(
    android::Printer& printer
) -> void
{
    printer.printLine("<ReadyList>");
    for (auto const& hbs : mReadyList) {
        printer.printFormatLine("    [R%u F%u]TS:%" PRIu64 " {%s} {%s} {%s}",
                                hbs.requestNo, hbs.frameNo, hbs.sensorTimestamp,
                                toString_BuffersStreamId(hbs.vpHalImageStreamBuffers).c_str(),
                                toString_BuffersStreamId(hbs.vpHalMetaStreamBuffers).c_str(),
                                toString_BuffersStreamId(hbs.vpAppMetaStreamBuffers).c_str());
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
beginConfigure() -> void
{
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
endConfigure() -> int
{
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
addImageStream(AddImageStream const& arg) -> int
{
    auto pStreamInfo = arg.pStreamInfo;

    auto pConfigStream = std::make_shared<ConfigImageStream>();
    if (CC_UNLIKELY( pConfigStream == nullptr )) {
        MY_LOGE("[%s] Failed on make_shared<ConfigImageStream>", pStreamInfo->toString().c_str());
        return NO_MEMORY;
    }

    pConfigStream->pStreamInfo = arg.pStreamInfo;
    pConfigStream->attribute = arg.attribute;

    auto const bufferSource = ( (uint32_t)(arg.attribute)
                              & (uint32_t)(StreamAttribute::BUFFER_SOURCE_MASK) );
    switch (static_cast<IFrameBufferManager::Attribute>(bufferSource))
    {
    case IFrameBufferManager::Attribute::BUFFER_SOURCE_POOL:{

        auto pPool = createHalStreamBufferPool(getName(), pStreamInfo,
                        [this](StreamId_T streamId, bool isFirstRetry){ tryReleaseOneBufferSet(streamId, isFirstRetry); });
        if (CC_UNLIKELY( pPool == nullptr )) {
            MY_LOGE("[%s] Failed on createHalStreamBufferPool", pStreamInfo->toString().c_str());
            return UNKNOWN_ERROR;
        }
        //
        pConfigStream->pPool = pPool;
        }break;

    case IFrameBufferManager::Attribute::BUFFER_SOURCE_PROVIDER:{
        if (CC_UNLIKELY( arg.pProvider == nullptr )) {
            MY_LOGE("[%s] Bad provider", pStreamInfo->toString().c_str());
            return BAD_VALUE;
        }
        //
        pConfigStream->pProvider = arg.pProvider;
        }break;

    default:{
        }break;
    }

    {
        android::RWLock::AutoWLock _l(mConfigLock);
        mConfigImageStreams[pStreamInfo->getStreamId()] = pConfigStream;
    }

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
addMetaStream(AddMetaStream const& arg) -> int
{
    auto pStreamInfo = arg.pStreamInfo;

    auto pConfigStream = std::make_shared<ConfigMetaStream>();
    if (CC_UNLIKELY( pConfigStream == nullptr )) {
        MY_LOGE("[%s] Failed on make_shared<ConfigMetaStream>", pStreamInfo->toString().c_str());
        return NO_MEMORY;
    }

    pConfigStream->pStreamInfo = arg.pStreamInfo;
    pConfigStream->attribute = arg.attribute;

    {
        android::RWLock::AutoWLock _l(mConfigLock);
        mConfigMetaStreams[pStreamInfo->getStreamId()] = pConfigStream;
    }

    return 0;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
queryConfigImageStream(
    StreamId_T streamId
) const -> std::shared_ptr<ConfigImageStream>
{
    android::RWLock::AutoRLock _l(mConfigLock);

    auto it = mConfigImageStreams.find(streamId);
    if (CC_LIKELY( it != mConfigImageStreams.end() )) {
        return it->second;
    }

    MY_LOGW("couldn't find streamId:%#" PRIx64 "", streamId);
    return nullptr;
}


/******************************************************************************
 *
 ******************************************************************************/
template <typename T> inline
auto targetLikelyFront(const T& List, uint32_t const frameNo)
{
    auto it = List.begin();
    for (; it != List.end(); ++it) {
        if (frameNo < it->frameNo) break;
    }
    return it;
}

template <typename T> inline
auto targetLikelyBack(const T& List, uint32_t const frameNo)
{
    auto rit = List.rbegin();
    for (; rit != List.rend(); ++rit) {
        if (frameNo > rit->frameNo) break;
    }
    if (rit == List.rend()) {
        return List.begin();
    }
    return rit.base();
}


/******************************************************************************
 *
 ******************************************************************************/
template <typename T>
auto
ThisNamespace::
releaseOneBufferSet(T& bufferSet) -> void
{
    auto releaseBuffers = [](auto& vBuffers) {
        for (auto& buf : vBuffers) {
            releaseBuffer(buf);
        }
    };
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    releaseBuffers(bufferSet.vpHalImageStreamBuffers);
    releaseBuffers(bufferSet.vpHalMetaStreamBuffers);
    releaseBuffers(bufferSet.vpAppMetaStreamBuffers);
    MY_LOGD_IF((mLogLevel > 1), "release [R%u F%u]", bufferSet.requestNo, bufferSet.frameNo);
}


/******************************************************************************
 *
 ******************************************************************************/
template <typename T>
auto
ThisNamespace::
isPreOrderedBuffer(T const& bufferSet) -> bool
{
    for (auto const& pHalImage : bufferSet.vpHalImageStreamBuffers) {
        if ((queryConfigImageStream(pHalImage->getStreamInfo()->getStreamId())->pPool)->isInAcquire()) {
            return true;
        }
    }
    return false;
};


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
tryReleaseOneBufferSet(StreamId_T streamId __unused, bool isFirstRetry) -> void
{
    MY_LOGD_IF((mLogLevel > 1), "stream(%#" PRIx64 ")", streamId);
    std::unique_lock _lr(mReadyLock);
    if (!mReadyList.empty()) {
        auto tmp = std::move(mReadyList.front());
        mReadyList.pop_front();
        _lr.unlock();
        releaseOneBufferSet(tmp);
    } else {
        _lr.unlock();
        if (isFirstRetry) {
            mPreOrderAmount++;
            MY_LOGD_IF((mLogLevel > 1), "mPreOrderAmount++ %d", mPreOrderAmount.load());
        } else {
            MY_LOGD_IF((mLogLevel > 1), "mPreOrderAmount = %d", mPreOrderAmount.load());
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
requestStreamBuffer(
    android::sp<IImageStreamBuffer>& rpImageStreamBuffer,
    RequestStreamBuffer const& arg
) -> int
{
    auto trace = [](auto const& arg, char const* title){
        CAM_TRACE_NAME(!ATRACE_ENABLED()?"":
            (std::string(title) + " requestStreamBuffer " + arg.streamInfo->getStreamName() +
            "|request:" + std::to_string(arg.requestNo) + " frame:" + std::to_string(arg.frameNo)).c_str()
        );
    };

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    int err = UNKNOWN_ERROR;
    auto pStreamInfo = const_cast<IImageStreamInfo*>(arg.streamInfo);
    auto const streamId = pStreamInfo->getStreamId();

    auto pConfigImageStream = queryConfigImageStream(streamId);
    if (CC_UNLIKELY( pConfigImageStream == nullptr )) {
        MY_LOGE("[requestNo:%u frameNo:%u] Couldn't find %s",
            arg.requestNo, arg.frameNo, toString_ConfigImageStream(*pConfigImageStream).c_str());
        return err;
    }

    switch (pConfigImageStream->attribute)
    {
    case IFrameBufferManager::Attribute::APP_IMAGE_PROVIDER:{
            auto pProvider = pConfigImageStream->pProvider;
            {
                trace(arg, "APP_IMAGE_PROVIDER");
                err = pProvider->requestStreamBuffer(
                        rpImageStreamBuffer,
                        IImageStreamBufferProvider::RequestStreamBuffer{
                            .pStreamInfo = pStreamInfo,
                            .timeout = UINT64_MAX,
                            .requestNo = arg.requestNo,
                        });
            }
        }break;

    case IFrameBufferManager::Attribute::HAL_IMAGE_PROVIDER:{
        if ( ! isSameAllocLayout(pConfigImageStream->pStreamInfo.get(), pStreamInfo) )
        {
            MY_LOGD("[requestNo:%u frameNo:%u] PROVIDER -> RUNTIME due to alloc layout change", arg.requestNo, arg.frameNo);
            {
                trace(arg, "PROVIDER -> RUNTIME");
                err = allocateHalImageStreamBufferRuntime(pStreamInfo, rpImageStreamBuffer);
            }
        }
        else
        {
            auto pProvider = pConfigImageStream->pProvider;
            {
                trace(arg, "HAL_IMAGE_PROVIDER");
                err = pProvider->requestStreamBuffer(
                        rpImageStreamBuffer,
                        IImageStreamBufferProvider::RequestStreamBuffer{
                            .pStreamInfo = pStreamInfo,
                            .timeout = UINT64_MAX,
                            .requestNo = arg.requestNo,
                        });
            }
            //
            if  (   CC_LIKELY(rpImageStreamBuffer != nullptr)
                && (rpImageStreamBuffer->getStreamInfo() != pStreamInfo) )
            {
                rpImageStreamBuffer->replaceStreamInfo(pStreamInfo);
            }
        }
        }break;

    case IFrameBufferManager::Attribute::HAL_IMAGE_POOL:{
        if ( ! isSameAllocLayout(pConfigImageStream->pStreamInfo.get(), pStreamInfo) )
        {
            MY_LOGD("[requestNo:%u frameNo:%u] POOL -> RUNTIME due to alloc layout change", arg.requestNo, arg.frameNo);
            {
                trace(arg, "POOL -> RUNTIME");
                err = allocateHalImageStreamBufferRuntime(pStreamInfo, rpImageStreamBuffer);
            }
        }
        else
        {
            auto pPool = pConfigImageStream->pPool;
            {
                trace(arg, "POOL");
                MY_LOGD_IF((mLogLevel > 1), "[requestNo:%u frameNo:%u] acquireFromPool (%s)", arg.requestNo, arg.frameNo, pPool->poolName());
                err = pPool->acquireFromPool(pPool->poolName(),
                                             rpImageStreamBuffer,
                                             ::ms2ns(mAcquireTimeoutMS));
                if ((mPreOrderAmount > 0) &&  (err == 0)) {
                    mPreOrderAmount--;
                    MY_LOGD_IF((mLogLevel > 1), "mPreOrderAmount-- %d", mPreOrderAmount.load());
                }
            }
            //
            if  (   CC_LIKELY(rpImageStreamBuffer != nullptr)
                && (rpImageStreamBuffer->getStreamInfo() != pStreamInfo) )
            {
                rpImageStreamBuffer->replaceStreamInfo(pStreamInfo);
            }
        }
        }break;

    case IFrameBufferManager::Attribute::HAL_IMAGE_RUNTIME:{
        {
            trace(arg, "RUNTIME");
            err = allocateHalImageStreamBufferRuntime(pStreamInfo, rpImageStreamBuffer);
        }
        }break;

    default:{
        MY_LOGE("[requestNo:%u frameNo:%u] Unsupported attribute:%s for %s",
            arg.requestNo, arg.frameNo,
            toString(pConfigImageStream->attribute).c_str(),
            toString_ConfigImageStream(*pConfigImageStream).c_str());
        }break;
    }
    //
    MY_LOGE_IF(0!=err || rpImageStreamBuffer==nullptr,
        "[requestNo:%u frameNo:%u] err:%d(%s) %s",
        arg.requestNo, arg.frameNo, err, ::strerror(-err),
        toString_ConfigImageStream(*pConfigImageStream).c_str()
    );
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
returnResult(
    ReturnResult const& arg
) -> void
{
    std::ostringstream oss, *pOss = ((mLogLevel > 1) ? &oss : nullptr);

    if (pOss) {
        (*pOss) << "[R" << arg.requestNo << " F" << arg.frameNo << "]"
                << "TS:" << arg.sensorTimestamp
                << ' ' << (arg.isFrameDestroyed ? 'X' : ' ');
    }
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Lambda Functions
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    auto releaseBuffers = [pOss](auto pBuffers) {
        if (pBuffers != nullptr) {
            for (auto& buf : *pBuffers) {
                if (buf != nullptr) {
                    if (pOss) {
                        (*pOss) << ' ' << buf->getName() << "(X) ";
                    }
                    releaseBuffer(buf);
                }
            }
            pBuffers->clear();
        }
    };
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    bool addToReadyList = false;
    InFlightBufferSet tmp;
    {
        std::scoped_lock _li(mInFlightLock);
        auto pInFlight = find_if(mInFlightList.begin(), mInFlightList.end(),
                                 [&arg](auto const& fbs) { return fbs.frameNo == arg.frameNo; });

        if (pInFlight != mInFlightList.end()) {
            /**
             * Save buffers which are in the trackParams, and release the others.
             */
            auto saveBuffers = [pOss, pInFlight](auto pSrcList, auto& destVec, auto const& checkList)
            {
                if (pSrcList != nullptr) {
                    for (auto it = (*pSrcList).begin(); it != (*pSrcList).end(); ) {
                        auto const streamId = (*it)->getStreamInfo()->getStreamId();
                        if (checkList.find(streamId) != checkList.end()) {
                            if (pOss) {
                                (*pOss) << ' ' << (*it)->getName() << "(O) ";
                            }
                            destVec.push_back(std::move(*it));
                            (pInFlight->trackCnt)--;
                            it = (*pSrcList).erase(it);
                        } else {
                            ++it;
                        }
                    }
                }
            };
            //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            if (pInFlight->sensorTimestamp == 0)    pInFlight->sensorTimestamp = arg.sensorTimestamp;
            saveBuffers(arg.pHalImageStreamBuffers, pInFlight->vpHalImageStreamBuffers, pInFlight->trackParams->imageStreams);
            saveBuffers(arg.pHalMetaStreamBuffers, pInFlight->vpHalMetaStreamBuffers, pInFlight->trackParams->metaStreams);
            saveBuffers(arg.pAppMetaStreamBuffers, pInFlight->vpAppMetaStreamBuffers, pInFlight->trackParams->metaStreams);

            if (arg.isFrameDestroyed && (pInFlight->trackCnt > 0)) {
                MY_LOGE("[requestNo:%u frameNo:%u] isFrameDestroyed(%d), but still have #%zu left",
                    arg.requestNo, arg.frameNo, arg.isFrameDestroyed, pInFlight->trackCnt);
                pInFlight->sensorTimestamp = 0;  // mark as error frame, releaseOneBufferSet in addToReadyList
                pInFlight->trackCnt = 0;  // redirect to move to addToReadyList
            }

            if (pInFlight->trackCnt == 0) {  // move to ReadyList
                tmp = std::move(*pInFlight);
                mInFlightList.erase(pInFlight);
                addToReadyList = true;
            }
        }
    }

    releaseBuffers(arg.pHalImageStreamBuffers);
    releaseBuffers(arg.pHalMetaStreamBuffers);
    releaseBuffers(arg.pAppMetaStreamBuffers);

    if (addToReadyList) {
        if (tmp.sensorTimestamp == 0 /*error frame*/||
            ((mPreOrderAmount > 0) && isPreOrderedBuffer(tmp)))
        {
            if (pOss) {
                (*pOss) << " => release bufferSet (TS: " << tmp.sensorTimestamp << ")";
            }
            releaseOneBufferSet(tmp);
        }
        else
        {
            std::scoped_lock _lr(mReadyLock);
            mReadyList.insert(targetLikelyBack(mReadyList, tmp.frameNo),
                              HistoryBufferSet{
                                  .frameNo = tmp.frameNo,
                                  .requestNo = tmp.requestNo,
                                  .sensorTimestamp = tmp.sensorTimestamp,
                                  .vpHalImageStreamBuffers = std::move(tmp.vpHalImageStreamBuffers),
                                  .vpHalMetaStreamBuffers = std::move(tmp.vpHalMetaStreamBuffers),
                                  .vpAppMetaStreamBuffers = std::move(tmp.vpAppMetaStreamBuffers)
                                });

            if (pOss) {
                (*pOss) << " => move to ReadyList";
            }
        }
    }

    MY_LOGD_IF(pOss, "%s", (*pOss).str().c_str());
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
trackFrameResult(TrackFrameResult const& arg) -> int
{
    if (arg.trackParams == nullptr ||
        (arg.trackParams->imageStreams.empty() && arg.trackParams->metaStreams.empty()))
    {
        MY_LOGD_IF((mLogLevel > 1), "[R%u F%u] Nothing to track, no need to create FBS", arg.requestNo, arg.frameNo);
    }
    else
    {
        {
            std::scoped_lock _li(mInFlightLock);
            mInFlightList.emplace(targetLikelyBack(mInFlightList, arg.frameNo),
                                  arg.frameNo, arg.requestNo, arg.trackParams);
        }

        // dump trackParams
        if (mLogLevel > 1) {
            std::ostringstream oss;
            oss << "[R" << arg.requestNo << " F" << arg.frameNo << "] create FBS, Track: images{ "
                << std::showbase << std::hex;
            for (auto const& streamId : arg.trackParams->imageStreams)
                oss << streamId << " ";
            oss << "}, metas { ";
            for (auto const& streamId : arg.trackParams->metaStreams)
                oss << streamId << " ";
            oss << "}" << std::dec;
            MY_LOGD("%s", oss.str().c_str());
        }
    }

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
enableFrameResultRecyclable(
    uint32_t frameNo,
    bool forceToRecycle
) -> void
{
    MY_LOGE_IF((mLogLevel > 1), "TODO - frameNo:%u forceToRecycle:%d", frameNo, forceToRecycle);
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
beginSelect() -> std::optional<std::list<HistoryBufferSet>>
{
    mInSelect = true;
    std::list<HistoryBufferSet> destList;
    {
        std::scoped_lock _lr(mReadyLock);
        destList.splice(destList.begin(), mReadyList);
    }
    return destList.size()? std::make_optional(std::move(destList)) : std::nullopt;
}

/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
endSelect() -> void
{
    mInSelect = false;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
returnUnselectedSet(
    ReturnUnselectedSet&& arg
) -> void
{
    MY_LOGD_IF((mLogLevel > 1), "[R%u F%u] welcome back, %s", arg.hbs.requestNo, arg.hbs.frameNo, arg.keep ? "keep" : "release");
    if ((arg.keep == false) ||
        ((mPreOrderAmount > 0) && isPreOrderedBuffer(arg.hbs))) {
        releaseOneBufferSet(arg.hbs);
    } else {
        std::scoped_lock _lr(mReadyLock);
        mReadyList.insert(targetLikelyFront(mReadyList, arg.hbs.frameNo), std::move(arg.hbs));
    }
}


/******************************************************************************
 *
 ******************************************************************************/
const std::string ThisNamespace::mDebuggeeName{"NSCam::v3::NSPipelineContext::IFrameBufferManager"};

auto
ThisNamespace::
debug(
    android::Printer& printer,
    const std::vector<std::string>& options __unused
) -> void
{
    //@TODO
    dumpState(printer);
}