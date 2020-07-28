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

#define LOG_TAG "mtkcam-AppRaw16Reprocessor"
//
#include "AppRaw16Reprocessor.h"
//
#include <mtkcam/utils/std/Aee.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam3/pipeline/hwnode/NodeId.h>
//
#include "MyUtils.h"

/******************************************************************************
 *
 ******************************************************************************/
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v3::pipeline::model;
using namespace NSCam::v3::pipeline::policy;
using namespace NSCam::v3::Utils;

#define ThisNamespace   AppRaw16Reprocessor


/******************************************************************************
 *
 ******************************************************************************/
template <class NodeIOMapSetT>
static auto
collectStreamId_P1NodeIOMapOutput(NodeIOMapSetT const& nodeIOMapSet) -> std::set<StreamId_T>
{
    auto insertStreamId = [](auto& set, auto const& streamSet) {
        for (size_t j = 0; j < streamSet.size(); j++) {
            auto streamId = streamSet[j];
                set.insert(streamId);
        }
    };

    //Output streams of P1 Node from IOMAP
    std::set<StreamId_T> set;
    for (auto const& v : nodeIOMapSet) {
        switch (v.first)
        {
        case eNODEID_P1Node:
        case eNODEID_P1Node_main2:
        case eNODEID_P1Node_main3:
            {
            auto const& iomapSet = v.second;
            for (size_t i = 0; i < iomapSet.size(); i++) {
                insertStreamId(set, iomapSet[i].vOut);
            }
            }break;
        default:
            break;
        }
    }
    return set;
}


/******************************************************************************
 *
 ******************************************************************************/
template <class NodeIOMapSetT>
static auto
collectStreamId_P2NodeIOMapInput(NodeIOMapSetT const& nodeIOMapSet) -> std::set<StreamId_T>
{
    auto insertStreamId = [](auto& set, auto const& streamSet) {
        for (size_t j = 0; j < streamSet.size(); j++) {
            auto streamId = streamSet[j];
                set.insert(streamId);
        }
    };

    //Input streams of P2 Node from IOMAP
    std::set<StreamId_T> set;
    for (auto const& v : nodeIOMapSet) {
        switch (v.first)
        {
        case eNODEID_P2CaptureNode:
        case eNODEID_P2StreamNode:
            {
            auto const& iomapSet = v.second;
            for (size_t i = 0; i < iomapSet.size(); i++) {
                insertStreamId(set, iomapSet[i].vIn);
            }
            }break;
        default:
            break;
        }
    }
    return set;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
IAppRaw16Reprocessor::
makeInstance(
    std::string const& name,
    CtorParams const& rCtorParams
) -> android::sp<IAppRaw16Reprocessor>
{
    android::sp<ThisNamespace> pInstance = new ThisNamespace(name, rCtorParams);
    if  ( CC_UNLIKELY(pInstance==nullptr) ) {
        CAM_LOGE("[%s] Bad IAppRaw16Reprocessor instance: %s", __FUNCTION__, name.c_str());
        return nullptr;
    }

    return pInstance;
}


/******************************************************************************
 *
 ******************************************************************************/
ThisNamespace::
ThisNamespace(
    std::string const& name,
    CtorParams const& rCtorParams
)
    : IAppRaw16Reprocessor()
    , mName(name)
    , mCtorParams(rCtorParams)
{
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
dumpState(android::Printer& printer __unused) -> void
{
    printer.printLine("\n== AppRaw16Reprocessor ==");
    android::PrefixPrinter prefixPrinter(printer, "  ");
    {
        prefixPrinter.printFormatLine("Flushing:%d", mFlushing.load());
    }
    {
        auto reprocessRequestNo = [=](){
            std::lock_guard<std::mutex> _l(mReprocessRequestNoLock);
            return mReprocessRequestNo;
        }();
        prefixPrinter.printFormatLine("Reprocess Request: %s",
                ((reprocessRequestNo==BAD_REQUEST_NO)
                    ? "N/A" : std::to_string(reprocessRequestNo).c_str())
            );
    }
    {
        std::lock_guard<std::mutex> _l(mHistoryFrameContainerLock);
        prefixPrinter.printFormatLine("History Frame: #%zu", mHistoryFrameContainer.size());
        for (auto const& historyFrame : mHistoryFrameContainer) {
            if (historyFrame == nullptr)
                continue;
            auto pFrameControl = IPipelineBufferSetFrameControl::castFrom(historyFrame.get());
            if (pFrameControl == nullptr)
                continue;

            auto const historyTimestamp = pFrameControl->tryGetSensorTimestamp();
            prefixPrinter.printFormatLine("History Frame(requestNo:%u frameNo:%u): timestamp==%" PRId64 "",
                historyFrame->getRequestNo(), historyFrame->getFrameNo(), historyTimestamp);
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
setReprocessRequestNo(uint32_t requestNo) -> void
{
    std::lock_guard<std::mutex> _l(mReprocessRequestNoLock);
    mReprocessRequestNo = requestNo;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
resetReprocessRequestNo(uint32_t requestNo) -> void
{
    std::lock_guard<std::mutex> _l(mReprocessRequestNoLock);
    if ( requestNo == mReprocessRequestNo ) {
        mReprocessRequestNo = BAD_REQUEST_NO;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
acquireAllHistoryFrames() -> HistoryFrameContainerT
{
    HistoryFrameContainerT history;
    {
        std::lock_guard<std::mutex> _l(mHistoryFrameContainerLock);
        //All elements are transfered to the local variable "history".
        //mHistoryFrameContainer is empty after the splice.
        history.splice(history.end(), mHistoryFrameContainer);
    }
    return history;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
reset() -> void
{
    // Flush all history frames
    {
        HistoryFrameContainerT history = acquireAllHistoryFrames();
        if ( ! history.empty() ) {
            MY_LOGI("Flush all history frames: #%zu in total", history.size());
            for (auto& frame : history) {
                if (CC_LIKELY( frame != nullptr )) {
                    MY_LOGI("[requestNo:%u frameNo:%u] flush...", frame->getRequestNo(), frame->getFrameNo());
                    frame.clear();
                }
            }
            history.clear();
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
beginFlush() -> void
{
    {
        std::lock_guard<std::mutex> _l(mFlushLock);
        mFlushing = true;

        reset();
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
endFlush() -> void
{
    {
        std::lock_guard<std::mutex> _l(mFlushLock);
        mFlushing = false;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
keepFrameIfNeeded(
    KeepFrameIfNeededParams const& params
) -> bool
{
    bool isKeepForReprocess = (params.pReqResult->needKeepP1BuffersForAppReprocess);
    if ( ! isKeepForReprocess ) {
        return false;
    }


    std::lock_guard<std::mutex> _l(mFlushLock);
    if ( mFlushing == true ) {
        MY_LOGI(
            "[requestNo:%u frameNo:%u] shouldn't keep a frame during flushing",
            params.pPipelineFrame->getRequestNo(),
            params.pPipelineFrame->getFrameNo());
        return false;
    }

    MY_LOGF_IF((params.pAppRequest->pParsedAppImageStreamInfo->vAppImage_Output_RAW16.empty()),
        "[requestNo:%u frameNo:%u] shouldn't keep a frame WITHOUT App Output RAW16 buffer existing",
        params.pPipelineFrame->getRequestNo(),
        params.pPipelineFrame->getFrameNo());

    MY_LOGF_IF(( ! params.isMainFrame ),
        "[requestNo:%u frameNo:%u] shouldn't keep a NON-main frame",
        params.pPipelineFrame->getRequestNo(),
        params.pPipelineFrame->getFrameNo());

    MY_LOGF_IF(params.isReprocessRequest,
        "[requestNo:%u frameNo:%u] shouldn't keep a reprocess request",
        params.pPipelineFrame->getRequestNo(),
        params.pPipelineFrame->getFrameNo());

    // Keep this frame before queuing it to pipeline.
    keepFrameForReprocess(params);
    return true;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
keepFrameForReprocess(KeepFrameIfNeededParams const& params) -> void
{
    auto& pPipelineFrame = params.pPipelineFrame;

    MY_LOGI("[requestNo:%u frameNo:%u] kept for reprocess",
            pPipelineFrame->getRequestNo(), pPipelineFrame->getFrameNo());

    auto pFrameControl = IPipelineBufferSetFrameControl::castFrom(pPipelineFrame.get());
    MY_LOGF_IF(pFrameControl==nullptr,
            "[requestNo:%u frameNo:%u] IPipelineBufferSetFrameControl::castFrom",
            pPipelineFrame->getRequestNo(), pPipelineFrame->getFrameNo());

    /**
     *  How to keep the output Hal stream buffers of P1Node?
     *  - For this preview frame, output streams of P1Node from IOMAP are the candidates.
     *  - For simplicity, both Hal and App output stream IDs of P1Node are collected from IOMAP.
     */
    pFrameControl->configureInformationKeeping(
        IPipelineBufferSetFrameControl::
        ConfigureInformationKeepingParams{
            .keepTimestamp  = true,
            .keepHalImage   = true,
            .keepHalMeta    = true,
            .keepAppMeta    = false,
            .targetToKeep_HalImage
                = collectStreamId_P1NodeIOMapOutput(params.pReqResult->nodeIOMapImage),
            .targetToKeep_HalMeta
                = collectStreamId_P1NodeIOMapOutput(params.pReqResult->nodeIOMapMeta),
        });
    {
        std::lock_guard<std::mutex> _l(mHistoryFrameContainerLock);
        mHistoryFrameContainer.push_back(pPipelineFrame);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
isReprocessFrame(IsReprocessFrameParams const& params) const -> bool
{
    if ( params.pAppRequest->pParsedAppImageStreamInfo->pAppImage_Input_RAW16 == nullptr ) {
        return false;
    }

    MY_LOGF_IF(( ! params.isMainFrame),
        "[requestNo:%u] Only main frame is expected for RAW16 reprocess request",
        params.pAppRequest->requestNo);

    return true;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
handleReprocessFrame(HandleReprocessFrameParams const& params) -> void
{
    std::lock_guard<std::mutex> _l(mFlushLock);//Don't reset history frames during preparing the reprocess frame.
    if ( mFlushing == true ) {
        MY_LOGW("[requestNo:%u] Handle the reprocess frame during flushing!", params.pAppRequest->requestNo);
        MY_LOGW("[requestNo:%u] Do nothing since kept Hal buffers have been reset", params.pAppRequest->requestNo);
        return;
    }

    setReprocessRequestNo(params.pAppRequest->requestNo);
    prepareReprocessHalBuffers(params);
    checkJpegControlMetadata(params.pAppMetaControl.get());
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
prepareReprocessHalBuffers(HandleReprocessFrameParams const& params) -> void
{
    auto requestNo = params.pAppRequest->requestNo;

    // For reprocessing, pick up one history frame which has the same timestamp as the request's.
    //
    // 1) the timestamp from request.
    int64_t requestTimestamp = 0;
    bool ret = IMetadata::getEntry(params.pAppMetaControl.get(), MTK_SENSOR_TIMESTAMP, requestTimestamp);
    MY_LOGF_IF(!ret, "[requestNo:%u] reprocess request has no android.sensor.timestamp", requestNo);
    //MY_LOGI("[requestNo:%u] reprocess request SENSOR_TIMESTAMP:%" PRId64 "", requestNo, requestTimestamp);

    // 2) find the history frame with the same timestamp.
    android::sp<IPipelineBufferSetFrameControl> pTargetHistoryFrameControl = nullptr;
    HistoryFrameContainerT historyFrames;
    selectHistoryFrameForReprocess(pTargetHistoryFrameControl, historyFrames, requestTimestamp, requestNo);

    // 3) [transfer] pHistoryFrameControl -> vHalMeta, vHalImage
    if ( pTargetHistoryFrameControl != nullptr ) {
        transferBuffersFromFrame(TransferBuffersFromFrameParams{
            .pFrameBuilder          = params.pFrameBuilder,
            .pHistoryFrameControl   = pTargetHistoryFrameControl,
            .pReqResult             = params.pReqResult,
            .pAppRequest            = params.pAppRequest,
            .pConfigStreamInfo_P1   = params.pConfigStreamInfo_P1,
        });
    }

    // 4) Clean up all history frames.
    historyFrames.clear();
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
selectHistoryFrameForReprocess(
    android::sp<IPipelineBufferSetFrameControl>& pTargetFrameControl,
    HistoryFrameContainerT& historyFrames,
    int64_t requestTimestamp,
    uint32_t requestNo
) -> void
{
    // 1) get all history frames.
    historyFrames = acquireAllHistoryFrames();
    MY_LOGI("[requestNo:%u] History frames:#%zu", requestNo, historyFrames.size());
    if (CC_UNLIKELY( historyFrames.empty() )) {
        std::string msg =
                std::string("[requestNo:") + std::to_string(requestNo) + "] "
              + "Empty history container!\n"
              + "App didn't request to keep any frame before reprocessing."
                ;
        NSCam::Utils::triggerAeeException(LOG_TAG, NULL, msg.c_str());
        //MY_LOGF("%s", msg.c_str());
    }

    // 2) find the history frame with the same timestamp.
    for (auto const& historyFrame : historyFrames)
    {
        MY_LOGF_IF(historyFrame==nullptr,
            "[requestNo:%u] nullptr historyFrame", requestNo);
        auto pFrameControl = IPipelineBufferSetFrameControl::castFrom(historyFrame.get());
        MY_LOGF_IF(pFrameControl==nullptr,
            "[requestNo:%u] IPipelineBufferSetFrameControl::castFrom() history frame(requestNo:%u frameNo:%u)",
            requestNo, historyFrame->getRequestNo(), historyFrame->getFrameNo());

        auto const historyTimestamp = pFrameControl->tryGetSensorTimestamp();
        if ( historyTimestamp == requestTimestamp ) {
            pTargetFrameControl = pFrameControl;
            break;//hit
        }

        MY_LOGW("History Frame(requestNo:%u frameNo:%u): timestamp==%" PRId64 "",
            historyFrame->getRequestNo(), historyFrame->getFrameNo(), historyTimestamp);
    }

    if (CC_LIKELY( pTargetFrameControl != nullptr )) {
        MY_LOGI("[requestNo:%u SENSOR_TIMESTAMP:%" PRId64 "] "
                "hit the History-Frame(requestNo:%u frameNo:%u) for reprocessing",
                requestNo, requestTimestamp,
                pTargetFrameControl->getRequestNo(),
                pTargetFrameControl->getFrameNo());
    }
    else {
        MY_LOGW("[requestNo:%u SENSOR_TIMESTAMP:%" PRId64 "] "
                "cannot hit any history frame for reprocessing", requestNo, requestTimestamp);

        char const* key = "vendor.debug.camera3.AppRaw16Reprocessor.forceSelect";
        auto forceSelect = ::property_get_bool(key, 0);
        if ( forceSelect ) {
            pTargetFrameControl = IPipelineBufferSetFrameControl::castFrom(historyFrames.front().get());
            MY_LOGW("%s=true", key);
            MY_LOGW("[workaround] "
                    "force to select the 1st kept History-Frame(requestNo:%u frameNo:%u SENSOR_TIMESTAMP:%" PRId64 ")",
                    pTargetFrameControl->getRequestNo(),
                    pTargetFrameControl->getFrameNo(),
                    pTargetFrameControl->tryGetSensorTimestamp());
        }

        if (CC_UNLIKELY( pTargetFrameControl == nullptr )) {
            std::string msg =
                    std::string("[requestNo:") + std::to_string(requestNo) + "] "
                  + "Cannot hit any history frame for reprocessing!\n"
                  + "App is sending a reprocessing frame"
                  + " with SENSOR_TIMESTAMP:" + std::to_string(requestTimestamp) +
                  + " different from the previous kept frame."
                    ;
            NSCam::Utils::triggerAeeException(LOG_TAG, NULL, msg.c_str());
            //MY_LOGF("%s", msg.c_str());
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
transferBuffersFromFrame(TransferBuffersFromFrameParams const& params) -> void
{
    auto& pReqResult = (params.pReqResult);
    auto& pHistoryFrameControl = params.pHistoryFrameControl;
    auto& pFrameBuilder = params.pFrameBuilder;

    /**
     *  P1 output stream buffers contain:
     *   (1) App meta
     *   (2) App image
     *   (3) Hal meta
     *   (4) Hal image
     *
     *  For reprocess frames:
     *  - (1) and (2) are not needed to keep in HAL during preview since applications will
     *    send them to HAL via createReprocessCaptureRequest(TotalCaptureResult inputResult).
     *  - (3) and (4) may be needed for raw-to-yuv tuning.
     *
     *  How to get the stream IDs of (3) and (4)?
     *  - IOMap shows all streams needed for this reprocess frame.
     *  - Input streams of P2 Node from IOMAP are the candidates (Don't use P1 IOMAP).
     *  - P1 HAL meta control streams must be ignored.
     */

    // Transfer stream buffers from the given frame (i.e. "pFrameControl") to "out".
    auto transferBuffers = [](
        auto bufferType,
        auto attachBuffer,
        auto ignoreBuffer,
        auto const& pFrameControl,                      // from
        std::set<StreamId_T> const& targetStreamIdSet,  // target stream id
        std::set<StreamId_T> const& ignoredStreamIdSet  // ignored stream id
    )
    {
        using android_sp_StreamBufferT = decltype(bufferType);
        android::Vector<android_sp_StreamBufferT> vStreamBuffer;

        pFrameControl->transferPendingReleaseBuffers(vStreamBuffer);

        for (size_t i = 0; i < vStreamBuffer.size(); i++) {
            auto pBuffer = vStreamBuffer[i].get();
            if (CC_LIKELY( pBuffer != nullptr )) {
                auto streamId = pBuffer->getStreamInfo()->getStreamId();
                if (  ignoredStreamIdSet.find(streamId) == ignoredStreamIdSet.end() //not ignored
                  &&   targetStreamIdSet.find(streamId) != targetStreamIdSet.end()  //target
                   )
                {
                    CAM_LOGI("Attach HAL stream: %#" PRIx64 " %s", streamId, pBuffer->getName());
                    attachBuffer(pBuffer);
                }
                else
                {
                    CAM_LOGI("Ignore HAL stream: %#" PRIx64 " %s", streamId, pBuffer->getName());
                    ignoreBuffer(pBuffer);
                }
            }
        }
    };

    // [transfer] History P1 HAL Meta (from pTargetFrameControl) -> vHalMeta
    transferBuffers(
        android::sp<IMetaStreamBuffer>(),
        /*attachBuffer*/[=](auto b){ pFrameBuilder->setHalMetaStreamBuffer(b->getStreamInfo()->getStreamId(), b); },
        /*ignoreBuffer*/[ ](auto b){ (void)b;/*fix build warning*/ },
        pHistoryFrameControl,
        collectStreamId_P2NodeIOMapInput(pReqResult->nodeIOMapMeta),
        {}
    );

    // [transfer] History P1 HAL Image (from pTargetFrameControl) -> vHalImage
    transferBuffers(
        android::sp<IImageStreamBuffer>(),
        /*attachBuffer*/[=](auto b){ pFrameBuilder->setHalImageStreamBuffer(b->getStreamInfo()->getStreamId(), b); },
        /*ignoreBuffer*/[ ](auto b){ b->releaseBuffer(); },
        pHistoryFrameControl,
        collectStreamId_P2NodeIOMapInput(pReqResult->nodeIOMapImage),
        {}
    );

}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
checkJpegControlMetadata(IMetadata* pMetadata) const -> void
{
#define JPEG_ENTRY(_tag_)                               \
    do {                                                \
        auto const& entry = pMetadata->entryFor(_tag_); \
        if (CC_UNLIKELY( entry.isEmpty() )) {           \
            MY_LOGW("NO " #_tag_ );                      \
          /*pMetadata->update(_tag_, _entryToUpdate_);*/\
        }                                               \
    } while (0)

    if (CC_LIKELY( pMetadata != nullptr ))
    {
        JPEG_ENTRY( MTK_JPEG_GPS_COORDINATES );
        JPEG_ENTRY( MTK_JPEG_GPS_PROCESSING_METHOD );
        JPEG_ENTRY( MTK_JPEG_GPS_TIMESTAMP );
        JPEG_ENTRY( MTK_JPEG_ORIENTATION );
        JPEG_ENTRY( MTK_JPEG_QUALITY );
        JPEG_ENTRY( MTK_JPEG_THUMBNAIL_QUALITY );
        JPEG_ENTRY( MTK_JPEG_THUMBNAIL_SIZE );
    }

#undef JPEG_ENTRY
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
notifyRequestDone(uint32_t requestNo) -> void
{
    resetReprocessRequestNo(requestNo);
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
debugRequestResult(
    DebugRequestResultParams const& params
) const -> void
{
    bool isReprocessRequest = [=](){
        std::lock_guard<std::mutex> _l(mReprocessRequestNoLock);
        return ( params.requestNo == mReprocessRequestNo );
    }();

    if ( isReprocessRequest )
    {
        auto const requestNo = params.requestNo;
        auto const& vAppOutMeta = *params.pvAppOutMeta;
        for ( size_t i = 0; i < vAppOutMeta.size(); i++ ) {
            if ( auto pStreamBuffer = vAppOutMeta[i].get() ) {
                if ( auto pMetadata = pStreamBuffer->tryReadLock(LOG_TAG) ) {
                    int64_t timestamp = 0;
                    if ( IMetadata::getEntry(pMetadata, MTK_SENSOR_TIMESTAMP, timestamp) ) {
                        MY_LOGI("[requestNo:%u] reprocess request has a callback SENSOR_TIMESTAMP:%" PRId64 "", requestNo, timestamp);
                    }
                    pStreamBuffer->unlock(LOG_TAG, pMetadata);
                }
            }
        }
    }
}

