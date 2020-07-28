/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
#define __DEBUG // enable debug
// #define __SCOPE_TIMER // enable log of scope timer

#define LOG_TAG "ReqFrm"
static const char* __CALLERNAME__ = LOG_TAG;

#include "RequestFrame.h"

// MTKCAM
#include <mtkcam/utils/std/Log.h>

// AOSP
#include <cutils/compiler.h>
#include <cutils/properties.h>

// STL
#include <algorithm> // std::find

// ----------------------------------------------------------------------------
// macros
// ----------------------------------------------------------------------------
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_DBG_COND(level)          __builtin_expect( RequestFrame::m_dbgLevel >= level, false )
#define MY_LOGD3(...)               do { if ( MY_DBG_COND(3) ) MY_LOGD(__VA_ARGS__); } while(0)
// FUNCTION_SCOPE
#ifdef __DEBUG
#define FUNCTION_SCOPE          auto __scope_logger__ = create_scope_logger(__FUNCTION__)
#include <memory>
#include <functional>
static std::shared_ptr<char> create_scope_logger(const char* functionName)
{
    char* pText = const_cast<char*>(functionName);
    CAM_LOGD_IF(NSCam::plugin::RequestFrame::m_dbgLevel >= 3, "[%s] + ", pText);
    return std::shared_ptr<char>(pText, [](char* p){ CAM_LOGD_IF(NSCam::plugin::RequestFrame::m_dbgLevel >= 3, "[%s] -", p); });
}
#else
#define FUNCTION_SCOPE          do{}while(0)
#endif
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
#endif

using namespace android;
using namespace NSCam;
using namespace NSCam::plugin;

// ----------------------------------------------------------------------------
// RequestFrame::FrameImage
// ----------------------------------------------------------------------------
RequestFrame::FrameImage::FrameImage(eDirection inout)
    : reqId(eRequestImg_Unknown)
    , index(0)
    , bError(false)
    , inout(inout) // IN
    , callerName(nullptr)
    , orientation(0)
    , bBufferLocked(false)
{
}


RequestFrame::FrameImage::~FrameImage()
{
    MY_LOGD3("delete FrameImage idx:%zu reqId:%#x, callerName:%s",
            index, reqId, callerName);

    if (imageBuffer.get()) {
        // unlock image buffer if necessary
        if (bBufferLocked)
            imageBuffer->unlockBuf(callerName);
    }
    else {
        MY_LOGD3("destroying FrameImage, IImageBuffer is NULL");
    }

    if (handle.get() != nullptr) {
        if (bError) {
            handle->updateStatus(inout == eDirection_In
                    ? BUFFER_IN | BUFFER_READ_ERROR
                    : BUFFER_OUT | BUFFER_WRITE_ERROR
                    );
        }
        else {
            handle->updateStatus(inout == eDirection_In
                    ? BUFFER_IN | BUFFER_READ_OK
                    : BUFFER_OUT | BUFFER_WRITE_OK
                    );
        }
    }
}


// ----------------------------------------------------------------------------
// RequestFrame::FrameMetadata
// ----------------------------------------------------------------------------
RequestFrame::FrameMetadata::FrameMetadata(eDirection inout)
    : reqId(eRequestMeta_Unknown)
    , index(0)
    , bError(false)
    , inout(inout)
    , callerName(nullptr)
    , pMetadata(nullptr)
{
}


RequestFrame::FrameMetadata::~FrameMetadata()
{
    MY_LOGD3("delete FrameMetadata index:%zu reqId:%#x, callerName:%s",
            index, reqId, callerName);

    if (handle.get() != nullptr) {
        if (bError) {
            handle->updateStatus(inout == eDirection_In
                    ? BUFFER_IN | BUFFER_READ_ERROR
                    : BUFFER_OUT | BUFFER_WRITE_ERROR
                    );
        }
        else {
            handle->updateStatus(inout == eDirection_In
                    ? BUFFER_IN | BUFFER_READ_OK
                    : BUFFER_OUT | BUFFER_WRITE_OK
                    );
        }
    }
}


// ----------------------------------------------------------------------------
// RequestFrame
// ----------------------------------------------------------------------------
int RequestFrame::m_dbgLevel = ::property_get_int32("persist.mtk.camera.log_level", 0);
RequestFrame::RequestFrame(
        const MINT32                        openId,
        const char*                         callerName,
        const IVendor::BufferParam&         bufParam,
        const IVendor::MetaParam&           metaParam,
        const wp<IVendor::IDataCallback>&   cb,
        const StreamIdMap_Img&              streamIdMapImg,
        const StreamIdMap_Meta&             streamIdMapMeta,
        MUINT32                             requestNo /* = 0 */,
        size_t                              index /* = 0 */
    )
    : m_bBroken(false)
    , m_requestNo(requestNo)
    , m_index(index)
    , m_openId(openId)
    , m_pDataCallback(cb)
{
    FUNCTION_SCOPE;
    SCOPE_TIMER(__t0, __FUNCTION__);

    // Step 1: parse image buffers, source part
    for (size_t i = 0; i < bufParam.vIn.size(); i++) {
        if (bufParam.vIn[i] == nullptr) {
            MY_LOGE("got buffer handle(in), but it's NULL. i=%zu, index=%zu",
                    i, index);
            continue;
        }

        bool bBroken = !parseImageBuffer(
                callerName,
                streamIdMapImg,
                bufParam.vIn[i],
                eDirection_In,
                index);

        if (bBroken) {
            m_bBroken = true;
        }
    }


    // Step 2: parse image buffers, output part
    for (size_t i = 0; i < bufParam.vOut.size(); i++) {
        if (bufParam.vOut[i] == nullptr) {
            MY_LOGE("got buffer handle(in), but it's NULL. i=%zu, index=%zu",
                    i, index);
            continue;
        }

        bool bBroken = !parseImageBuffer(
                callerName,
                streamIdMapImg,
                bufParam.vOut[i],
                eDirection_Out,
                index);

        if (bBroken) {
            m_bBroken = true;
        }
    }

    // Step 3: parse metadata, input part
    if (!parseMetadata(
                callerName,
                streamIdMapMeta,
                metaParam.appCtrl,
                eDirection_In,
                eRequestMeta_InAppRequest,
                index))
    {
        MY_LOGE("input app ctrl metadata is wrong, mark as a broken frame" \
                "index=%zu", m_index);
        m_bBroken = true;
    }

    if (!parseMetadata(
                callerName,
                streamIdMapMeta,
                metaParam.halCtrl,
                eDirection_In,
                eRequestMeta_InHalP1,
                index))
    {
        MY_LOGE("input hal ctrl metadata is wrong, mark as a broken frame" \
                "index=%zu", m_index);
        m_bBroken = true;
    }

    if (metaParam.vIn.size() > 0) { // TODO: p1Result should has stream ID or something else.
        parseMetadata(
                callerName,
                streamIdMapMeta,
                metaParam.vIn[0],
                eDirection_In,
                eRequestMeta_InFullRaw,
                index);
    }

    // Step 4: parse metadata, output part
    parseMetadata(
            callerName,
            streamIdMapMeta,
            metaParam.appResult,
            eDirection_Out,
            eRequestMeta_OutAppResult,
            index);

    parseMetadata(
            callerName,
            streamIdMapMeta,
            metaParam.halResult,
            eDirection_Out,
            eRequestMeta_OutHalResult,
            index);


lbExit:
    if (m_bBroken) {
        m_frameImages.clear();
        m_frameMetadatas.clear();
    }
}


RequestFrame::~RequestFrame()
{
    RequestFrame::dispatch();
}

// ----------------------------------------------------------------------------
// Methods
// ----------------------------------------------------------------------------

bool RequestFrame::dispatch()
{
    FUNCTION_SCOPE;

    MY_LOGD("clear image buffers [+]");
    // unlock all image buffer first
    unlockImageBuffer( eRequestImg_All );
    // clear image buffer
    m_frameImages.clear();
    m_frameMetadatas.clear();
    MY_LOGD("clear image buffers [-]");
    auto spCb = m_pDataCallback.promote();
    m_pDataCallback = nullptr;
    if (spCb.get()) {
        spCb->onDispatch();
        return true;
    }
    return false;
}


size_t RequestFrame::getFrameIndex() const
{
    return m_index;
}


IImageBuffer* RequestFrame::getImageBuffer(eRequestImg reqId) const
{
    const auto& f = m_frameImages[reqId];
    return f.get() ? f->imageBuffer.get() : nullptr;
}


IMetadata* RequestFrame::getMetadata(eRequestMeta reqId) const
{
    const auto& f = m_frameMetadatas[reqId];
    return f.get() ? f->pMetadata : nullptr;
}


MUINT32 RequestFrame::getOrientation(eRequestImg reqId) const
{
    const auto& f = m_frameImages[reqId];
    return f.get() ? f->orientation : 0;
}


MUINT32 RequestFrame::getRequestNo() const
{
    return m_requestNo;
}


bool RequestFrame::isError(eRequestImg reqId) const
{
    if (reqId == eRequestImg_All) {
        for (auto&& itr : m_frameImages)
            if (itr.get() && itr->bError)
                return true;

        return false;
    }

    const auto& f = m_frameImages[reqId];
    return f.get() ? f->bError : false;
}


bool RequestFrame::isError(eRequestMeta reqId) const
{
    if (reqId == eRequestMeta_All) {
        for (auto&& itr : m_frameMetadatas)
            if (itr.get() && itr->bError)
                return true;

        return false;
    }

    const auto& f = m_frameMetadatas[reqId];
    return f.get() ? f->bError: false;
}


bool RequestFrame::isImageBufferLocked(eRequestImg reqId) const
{
    if (CC_UNLIKELY( reqId == eRequestImg_All )) {
        for (auto&& itr : m_frameImages)
            if (itr.get() && itr->bBufferLocked)
                return true;

        return false;
    }

    const auto& f = m_frameImages[reqId];
    return f.get() ? f->bBufferLocked : false;
}


bool RequestFrame::lockImageBuffer(eRequestImg reqId, MINT usage)
{
    MBOOL bResult = MTRUE;

    if (reqId == eRequestImg_All) {
        for (auto&& itr : m_frameImages) {
            if (itr.get() && itr->imageBuffer.get() && (!itr->bBufferLocked)) {
                auto __result = itr->imageBuffer->lockBuf( itr->callerName, usage );
                if (CC_LIKELY( __result )) {
                    itr->bBufferLocked = true;
                }
                bResult = bResult && __result;
            }
        }
        return bResult;
    }

    auto& f = m_frameImages[reqId];
    if (CC_LIKELY( f.get() && f->imageBuffer.get() && (!f->bBufferLocked) )) {
        auto __result = f->imageBuffer->lockBuf( f->callerName, usage );
        if (CC_LIKELY( __result )) {
            f->bBufferLocked = true;
        }
        bResult = __result;
    }
    return bResult;
}


bool RequestFrame::unlockImageBuffer(eRequestImg reqId)
{
    MBOOL bResult = MTRUE;;

    if (reqId == eRequestImg_All) {
        for (auto&& itr : m_frameImages) {
            if (itr.get() && itr->imageBuffer.get() && itr->bBufferLocked) {
                auto __result = itr->imageBuffer->unlockBuf( itr->callerName );
                if (CC_LIKELY( __result )) {
                    itr->bBufferLocked = false;
                }
                bResult = bResult && __result;
            }
        }
        return bResult;
    }

    auto& f = m_frameImages[reqId];
    if (CC_LIKELY( f.get() && f->imageBuffer.get() && f->bBufferLocked )) {
        auto __result = f->imageBuffer->unlockBuf( f->callerName );
        if (CC_LIKELY( __result )) {
            f->bBufferLocked = false;
        }
        bResult = __result;
    }
    return bResult;
}


void RequestFrame::releaseImageBuffer(eRequestImg reqId)
{
    if (CC_UNLIKELY(reqId == eRequestImg_All))
        m_frameImages.clear();
    else
        m_frameImages[reqId] = nullptr;
}


void RequestFrame::releaseMetadata(eRequestMeta reqId)
{
    if (CC_UNLIKELY(reqId == eRequestMeta_All))
        m_frameMetadatas.clear();
    else
        m_frameMetadatas[reqId] = nullptr;
}


void RequestFrame::markError(eRequestImg reqId, bool bError /* = true */)
{
    if(reqId == eRequestImg_All) {
        for (auto&& itr : m_frameImages){
            if (itr.get())
                itr->bError = bError;
        }
        return;
    }
    auto pFrame = m_frameImages[reqId];
    if (pFrame.get()) {
        pFrame->bError = bError;
    }
}


void RequestFrame::markError(eRequestMeta reqId, bool bError /* = true */)
{
    if(reqId == eRequestMeta_All) {
        for (auto&& itr : m_frameMetadatas){
            if (itr.get())
                itr->bError = bError;
        }
        return;
    }
    auto pFrame = m_frameMetadatas[reqId];
    if (pFrame.get()) {
        pFrame->bError = bError;
    }
}


void RequestFrame::markBroken(bool bBroken /* = true */)
{
    m_bBroken = bBroken;
}


bool RequestFrame::isBroken() const
{
    return m_bBroken;
}


MINT32 RequestFrame::getOpenId() const
{
    return m_openId;
}


IImageBuffer* RequestFrame::operator [] (eRequestImg reqId)
{
    return getImageBuffer(reqId);
}


const IImageBuffer* RequestFrame::operator [] (eRequestImg reqId) const
{
    return getImageBuffer(reqId);
}


IMetadata* RequestFrame::operator [] (eRequestMeta reqId)
{
    return getMetadata(reqId);
}


const IMetadata* RequestFrame::operator [] (eRequestMeta reqId) const
{
    return getMetadata(reqId);
}


RequestFrame& RequestFrame::operator = (RequestFrame&& other)
{
    if (CC_LIKELY(this != &other)) {
        m_bBroken           = other.m_bBroken;
        m_requestNo         = other.m_requestNo;
        m_index             = other.m_index;
        m_frameImages       = std::move(other.m_frameImages);
        m_frameMetadatas    = std::move(other.m_frameMetadatas);
        m_pDataCallback     = other.m_pDataCallback;
        other.m_pDataCallback = nullptr;
    }

    return *this;
}


bool RequestFrame::parseImageBuffer(
        const char*                         callerName,
        const StreamIdMap_Img&              streamIdMapImg,
        const sp<IVendorManager::BufferHandle>  bufHandle,
        eDirection                          inout,
        size_t                              index)
{
    if (CC_UNLIKELY(bufHandle == nullptr)) {
        MY_LOGE("buffer handle is NULL, index=%zu", index);
        return false;
    }

    // chekc if buffer has marked as error.
    // if yes, mark the FrameImage as an error image too.
    bool bMarkError = false;
    if ((bufHandle->getStatus() & BUFFER_WRITE_ERROR)
        ||
        (bufHandle->getStatus() & BUFFER_READ_ERROR))
    {
        MY_LOGW("got buffer which is marked as error, index=%zu", index);
        bMarkError = true;
    }

    std::shared_ptr<FrameImage> pFrame(new FrameImage(inout));

    // check stream info
    auto streamInfo = bufHandle->getStreamInfo();
    if (CC_UNLIKELY(streamInfo.get() == nullptr)) {
        MY_LOGE("stream info is NULL from a buffer handle, index=%zu", index);
        return false;
    }

    auto streamId = streamInfo->getStreamId();
    auto usage = streamInfo->getUsageForAllocator();
    auto orientation = streamInfo->getTransform();

    // find matched StreamID
    eRequestImg  reqId = eRequestImg_Unknown;
    const auto itr = std::find(streamIdMapImg.begin(), streamIdMapImg.end(),
            streamId);
    if (CC_LIKELY(itr != streamIdMapImg.end())) {
        reqId = static_cast<eRequestImg>(itr - streamIdMapImg.begin());
    }
    else {
        MY_LOGW("Retrieve stream ID failed, stream ID=%#x," \
                "and this frame won't be kept in RequestFrame",
                static_cast<unsigned int>(streamId));
        // do not return directly, we need parse it as a RequestFrame and
        // destroys it.
    }

    // assign attributes
    pFrame->reqId = reqId;
    pFrame->index = index;
    pFrame->callerName = callerName;
    pFrame->imageBufferHeap = bufHandle->getHeap();
    pFrame->handle = bufHandle;
    pFrame->orientation = orientation;
    pFrame->bError = bMarkError;

    // create image buffer
    if (CC_UNLIKELY(pFrame->imageBufferHeap.get() == nullptr)) {
        MY_LOGE("ImageBufferHeap is NULL");
        return false;
    }


    MINT32 formatHeap = pFrame->imageBufferHeap->getImgFormat();
    MINT32 formatStream = streamInfo->getImgFormat();
    MSize sizeHeap = MSize(pFrame->imageBufferHeap->getImgSize().w, pFrame->imageBufferHeap->getImgSize().h);
    MSize sizeStream = MSize(streamInfo->getImgSize().w, streamInfo->getImgSize().h);

#ifdef __DEBUG
    if (__builtin_expect( RequestFrame::m_dbgLevel >= 3, false )) {
        String8 bufferName = String8::format("%s:s%dx%d:f%d_%dx%d:f%d",
          "VendorWB", sizeHeap.w, sizeHeap.h, formatHeap, sizeStream.w, sizeStream.h, formatStream);
        MY_LOGD("%s", bufferName.string());
    }
#endif

    if (formatHeap == eImgFmt_BLOB && formatHeap != formatStream)
    {
        size_t strideInBytes[3] = {0};
        MUINT32 plane = NSCam::Utils::Format::queryPlaneCount(formatStream);
        for (MUINT32 i = 0; i < plane; i++)
        {
            strideInBytes[i] = (Utils::Format::queryPlaneWidthInPixels(formatStream, i, sizeStream.w) *
                Utils::Format::queryPlaneBitsPerPixel(formatStream, i)) >> 3;
#ifdef __DEBUG
            MY_LOGD3("strideInBytes[%d]:%d", i, strideInBytes[i]);
#endif
        }

        pFrame->imageBuffer = pFrame->imageBufferHeap->createImageBuffer_FromBlobHeap((size_t)0,
          (MINT32)formatStream, sizeStream, strideInBytes);
    }
    else
    {
        pFrame->imageBuffer = pFrame->imageBufferHeap->createImageBuffer();
    }

    if (CC_UNLIKELY(pFrame->imageBuffer.get() == nullptr)) {
        MY_LOGE("get image buffer failed. inout=%d, index=%zu",
                eDirection_In, index);
        return false;
    }

    // lock it for using
#define BUFFER_USAGE    (eBUFFER_USAGE_SW_READ_OFTEN | \
        eBUFFER_USAGE_SW_WRITE_OFTEN | \
        eBUFFER_USAGE_HW_CAMERA_READWRITE)
    usage = BUFFER_USAGE; // The usage is never correct, always use this value.
    if (CC_UNLIKELY(!pFrame->imageBuffer->lockBuf(callerName, usage))) {
        MY_LOGE("lockBuf returns fail, caller=%s, usage=%#x",
                callerName, static_cast<unsigned int>(usage));
        return false;
    }

    pFrame->bBufferLocked = true; // mark as locked

    // add frame to set if the eRequestImg is ok, or directly drop it.
    if (CC_LIKELY(reqId != eRequestImg_Unknown))
        m_frameImages[reqId] = pFrame;

    return true;
}


bool RequestFrame::parseMetadata(
        const char*                 callerName,
        const StreamIdMap_Meta&     streamIdMapMeta     __attribute__((unused)),
        const sp<IVendorManager::MetaHandle> metaHandle,
        eDirection                  inout,
        eRequestMeta                reqId,
        size_t                      index)
{
    if (CC_UNLIKELY(metaHandle.get() == nullptr))
        return false;

    if (CC_UNLIKELY(metaHandle->getMetadata() == nullptr))
        return false;

    // chekc if metadata has marked as error
    bool bMarkError = false;
    if ((metaHandle->getStatus() & BUFFER_WRITE_ERROR)
        ||
        (metaHandle->getStatus() & BUFFER_READ_ERROR))
    {
        MY_LOGW("got metadata marked as error. reqId=%d", reqId);
        bMarkError = true;
    }

    std::shared_ptr<FrameMetadata> metadata(new FrameMetadata(inout));
    metadata->reqId = reqId;
    metadata->index = index;
    metadata->callerName = callerName;
    metadata->handle = metaHandle;
    metadata->pMetadata = metaHandle->getMetadata();
    metadata->bError = bMarkError;

    // add to set
    m_frameMetadatas[reqId] = metadata;
    return true;
}
