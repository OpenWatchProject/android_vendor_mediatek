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
#include <mtkcam/utils/hw/HwInfoHelper.h>
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam3/pipeline/stream/StreamId.h>
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::Utils::ULog;
#define ThisNamespace   AppStreamMgr::RequestHandler

/******************************************************************************
 *
 ******************************************************************************/
CAM_ULOG_DECLARE_MODULE_ID(MOD_CAMERA_DEVICE);
#define MY_DEBUG(level, fmt, arg...) \
    do { \
        CAM_ULOGM##level("[%s::%s] " fmt, mInstanceName.c_str(), __FUNCTION__, ##arg); \
        mCommonInfo->mDebugPrinter->printFormatLine(#level" [%s::%s] " fmt, mInstanceName.c_str(), __FUNCTION__, ##arg); \
    } while(0)

#define MY_WARN(level, fmt, arg...) \
    do { \
        CAM_ULOGM##level("[%s::%s] " fmt, mInstanceName.c_str(), __FUNCTION__, ##arg); \
        mCommonInfo->mWarningPrinter->printFormatLine(#level" [%s::%s] " fmt, mInstanceName.c_str(), __FUNCTION__, ##arg); \
    } while(0)

#define MY_ERROR_ULOG(level, fmt, arg...) \
    do { \
        CAM_ULOGM##level("[%s::%s] " fmt, mInstanceName.c_str(), __FUNCTION__, ##arg); \
        mCommonInfo->mErrorPrinter->printFormatLine(#level" [%s::%s] " fmt, mInstanceName.c_str(), __FUNCTION__, ##arg); \
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
#define MY_LOGE(...)                MY_ERROR_ULOG(E, __VA_ARGS__)
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
ThisNamespace::
RequestHandler(
    std::shared_ptr<CommonInfo> pCommonInfo,
    std::shared_ptr<RequestMetadataQueue> pRequestMetadataQueue,
    android::sp<FrameHandler> pFrameHandler,
    android::sp<BatchHandler> pBatchHandler
)
    : mInstanceName{std::to_string(pCommonInfo->mInstanceId) + "-RequestHandler"}
    , mCommonInfo(pCommonInfo)
    , mFrameHandler(pFrameHandler)
    , mBatchHandler(pBatchHandler)
    , mRequestMetadataQueue(pRequestMetadataQueue)
    , mLatestSettings()
{
    mIonDevice = IIonDeviceProvider::get()->makeIonDevice("AppStreamMgr.RequestHandler", 0);
    if ( CC_UNLIKELY( mIonDevice == nullptr) ) {
        MY_LOGE("fail to make IonDevice");
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
destroy() -> void
{
    mFrameHandler = nullptr;
    mBatchHandler = nullptr;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
reset() -> void
{
    //  An emtpy settings buffer cannot be used as the first submitted request
    //  after a configure_streams() call.
    mLatestSettings.clear();
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
submitRequest(
    const hidl_vec<V3_4::CaptureRequest>& captureRequests,
    android::Vector<Request>& rRequests
)   -> int
{
    int err = 0;

    err = createRequests(captureRequests, rRequests);
    if  ( OK != err ) {
        return err;
    }
    //
    err = registerRequests(rRequests);
    if  ( OK != err ) {
        return err;
    }

    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
registerRequests(
    const android::Vector<Request>& rRequests
)   -> int
{
    CAM_ULOGM_FUNCLIFE();
    for ( const auto& rRequest : rRequests ) {
        int err = mFrameHandler->registerFrame(rRequest);
        if ( err != OK )
            return err;
    }
    if(rRequests.size() > 1)
        mBatchHandler->registerBatch(rRequests);
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
createRequests(
    const hidl_vec<V3_4::CaptureRequest>& requests,
    android::Vector<Request>& rRequests
)   -> int
{
    CAM_ULOGM_FUNCLIFE();

    if  ( requests.size() == 0 ) {
        MY_LOGE("empty requests list");
        return  -EINVAL;
    }

    int err = OK;
    rRequests.resize(requests.size());
    size_t i = 0;
    for (; i < requests.size(); i++) {
        err = checkOneRequest(requests[i]);
        if  ( OK != err ) {
            break;
        }

        err = createOneRequest(requests[i], rRequests.editItemAt(i));
        if  ( OK != err ) {
            break;
        }
    }

    if  ( OK != err ) {
        MY_LOGE("something happened - frameNo:%u request:%zu/%zu",
            requests[i].v3_2.frameNumber, i, requests.size());
        return err;
    }

    return  OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
checkOneRequest(const V3_4::CaptureRequest& request) const -> int
{
    //  there are 0 output buffers
    if ( request.v3_2.outputBuffers.size() == 0 )
    {
        MY_LOGE("[frameNo:%u] outputBuffers.size()==0", request.v3_2.frameNumber);
        return  -EINVAL;
    }
    //
    //  not allowed if the fmq size is zero and settings are NULL w/o the lastest setting.
    if ( request.v3_2.fmqSettingsSize == 0 &&
         request.v3_2.settings.size() == 0 && mLatestSettings.isEmpty() )
    {
        MY_LOGE("[frameNo:%u] NULL request settings; "
                "however most-recently submitted request is also NULL after configureStreams",
                request.v3_2.frameNumber);
        return  -EINVAL;
    }
    //
    return  OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
createOneRequest(
    const V3_4::CaptureRequest& request,
    Request& rRequest
)   -> int
{
    auto queryBufferName = [=](auto const& buffer) -> std::string {
        return ("r"+std::to_string(request.v3_2.frameNumber)+":b"+std::to_string(buffer.bufferId));
    };

    CAM_ULOGM_FUNCLIFE();
    //
    rRequest.frameNo = request.v3_2.frameNumber;
    //
    //  vInputMetaBuffers <- request.v3_2.settings
    {
        sp<IMetaStreamInfo> pStreamInfo = mFrameHandler->getConfigMetaStream(eSTREAMID_END_OF_FWK);
        bool isRepeating = false;
        //
        if ( request.v3_2.fmqSettingsSize > 0 ) {        // read settings from FMQ.
            isRepeating = false;
            mLatestSettings.clear();
            CameraMetadata settingsFmq;
            settingsFmq.resize(request.v3_2.fmqSettingsSize);
            // non-blocking read; client must write metadata before calling
            bool read = mRequestMetadataQueue->read(settingsFmq.data(), request.v3_2.fmqSettingsSize);
            if ( !read ) {
                MY_LOGE("frameNo:%u capture request settings metadata couldn't be read from fmq!", request.v3_2.frameNumber);
                return -ENODEV;
            }
            CAM_ULOGM_TAG_BEGIN("convertFromHidl: settingsFmq -> mLatestSettings");
            if ( ! mCommonInfo->mMetadataConverter->convertFromHidl(&settingsFmq, mLatestSettings) ) {
                MY_LOGE("frameNo:%u convertFromHidl: settingsFmq -> mLatestSettings", request.v3_2.frameNumber);
                return -ENODEV;
            }
            CAM_ULOGM_TAG_END();
        }
        else if ( request.v3_2.settings.size() > 0 ) {   // read settings from metadata settings.
            isRepeating = false;
            mLatestSettings.clear();
            CAM_ULOGM_TAG_BEGIN("convertFromHidl: request.v3_2.settings -> mLatestSettings");
            if ( ! mCommonInfo->mMetadataConverter->convertFromHidl(&request.v3_2.settings, mLatestSettings) ) {
                MY_LOGE("frameNo:%u convertFromHidl: request.v3_2.settings -> mLatestSettings", request.v3_2.frameNumber);
                return -ENODEV;
            }
            CAM_ULOGM_TAG_END();
        }
        else {
            /**
             * As a special case, a NULL settings buffer indicates that the
             * settings are identical to the most-recently submitted capture request.v3_2. A
             * NULL buffer cannot be used as the first submitted request after a
             * configure_streams() call.
             */
            isRepeating = true;
            MY_LOGD_IF(getLogLevel() >= 1,
                "frameNo:%u NULL settings -> most-recently submitted capture request",
                request.v3_2.frameNumber);
        }

        sp<AppMetaStreamBuffer> pStreamBuffer = createMetaStreamBuffer(pStreamInfo, mLatestSettings, isRepeating);
        if ( CC_LIKELY(pStreamBuffer != nullptr && pStreamBuffer->getStreamInfo() != nullptr) ){
            rRequest.vInputMetaBuffers.add(pStreamBuffer->getStreamInfo()->getStreamId(), pStreamBuffer);
        }
        // to debug
        if ( ! isRepeating ) {

            if (getLogLevel() >= 2) {
                mCommonInfo->mMetadataConverter->dumpAll(mLatestSettings, request.v3_2.frameNumber);
            } else if(getLogLevel() >= 1) {
                mCommonInfo->mMetadataConverter->dump(mLatestSettings, request.v3_2.frameNumber);
            }

            IMetadata::IEntry const& e1 = mLatestSettings.entryFor(MTK_CONTROL_AF_TRIGGER);
            if ( ! e1.isEmpty() ) {
                MUINT8 af_trigger = e1.itemAt(0, Type2Type< MUINT8 >());
                if ( af_trigger==MTK_CONTROL_AF_TRIGGER_START )
                {
                    CAM_ULOGM_DTAG_BEGIN(true, "AF_state: %d", af_trigger);
                    MY_LOGD_IF(getLogLevel() >= 1, "AF_state: %d", af_trigger);
                    CAM_ULOGM_DTAG_END();
                }
            }
            //
            IMetadata::IEntry const& e2 = mLatestSettings.entryFor(MTK_CONTROL_AE_PRECAPTURE_TRIGGER);
            if ( ! e2.isEmpty() ) {
                MUINT8 ae_pretrigger = e2.itemAt(0, Type2Type< MUINT8 >());
                if ( ae_pretrigger==MTK_CONTROL_AE_PRECAPTURE_TRIGGER_START )
                {
                    CAM_ULOGM_DTAG_BEGIN(true, "ae precap: %d", ae_pretrigger);
                    MY_LOGD_IF(getLogLevel() >= 1, "ae precap: %d", ae_pretrigger);
                    CAM_ULOGM_DTAG_END();
                }
            }
            //
            IMetadata::IEntry const& e3 = mLatestSettings.entryFor(MTK_CONTROL_CAPTURE_INTENT);
            if ( ! e3.isEmpty() ) {
                MUINT8 capture_intent = e3.itemAt(0, Type2Type< MUINT8 >());
                CAM_ULOGM_DTAG_BEGIN(true, "capture intent: %d", capture_intent);
                MY_LOGD_IF(getLogLevel() >= 1, "capture intent: %d", capture_intent);
                CAM_ULOGM_DTAG_END();
            }
        }
    }
    //
    //  vInputMetaBuffers <- request.physicalCameraSettings
    {
        for ( const auto& physicSetting : request.physicalCameraSettings ){
            //convert physical metadata
            IMetadata physicMeta;
            if ( physicSetting.fmqSettingsSize > 0 ) {        // read settings from FMQ.
                CameraMetadata settingsFmq;
                settingsFmq.resize(physicSetting.fmqSettingsSize);
                // non-blocking read; client must write metadata before calling
                bool read = mRequestMetadataQueue->read(settingsFmq.data(), physicSetting.fmqSettingsSize);
                if ( !read ) {
                    MY_LOGE("frameNo:%u capture request physic settings metadata couldn't be read from fmq!", request.v3_2.frameNumber);
                    return -ENODEV;
                }
                CAM_TRACE_BEGIN("convertFromHidl: settingsFmq -> physicMeta");
                if ( ! mCommonInfo->mMetadataConverter->convertFromHidl(&settingsFmq, physicMeta) ) {
                    MY_LOGE("frameNo:%u convertFromHidl: settingsFmq -> physicMeta", request.v3_2.frameNumber);
                    return -ENODEV;
                }
                CAM_TRACE_END();
            }
            else if ( request.v3_2.settings.size() > 0 ) {   // read settings from metadata settings.
                CAM_TRACE_BEGIN("convertFromHidl: request.physicalCameraSettings.settings -> physicMeta");
                if ( ! mCommonInfo->mMetadataConverter->convertFromHidl(&physicSetting.settings, physicMeta) ) {
                    MY_LOGE("frameNo:%u convertFromHidl: request.physicalCameraSettings.settings -> mLatestSettings", request.v3_2.frameNumber);
                    return -ENODEV;
                }
                CAM_TRACE_END();
            }
            else {
                MY_LOGE("frameNo:%u invalid physical metadata settings!", request.v3_2.frameNumber);
            }

            StreamId_T const streamId = eSTREAMID_BEGIN_OF_PHYSIC_ID + (int64_t)(std::stoi((std::string)physicSetting.physicalCameraId));
            sp<IMetaStreamInfo> pMetaStreamInfo = mFrameHandler->getConfigMetaStream(streamId);
            if ( pMetaStreamInfo != nullptr ){
                sp<AppMetaStreamBuffer> pMetaStreamBuffer = createMetaStreamBuffer(pMetaStreamInfo, physicMeta, false /*isRepeating*/);
                rRequest.vInputMetaBuffers.add(pMetaStreamInfo->getStreamId(), pMetaStreamBuffer);
                //
                MY_LOGD("Duck: create physical meta frameNo:%u physicId:%d streamId:%#" PRIx64 "",
                    request.v3_2.frameNumber, std::stoi((std::string)physicSetting.physicalCameraId), streamId);
            }
            //
            // to debug
            if (getLogLevel() >= 2) {
                MY_LOGI("Dump all physical metadata settings frameNo:%u", request.v3_2.frameNumber);
                mCommonInfo->mMetadataConverter->dumpAll(physicMeta, request.v3_2.frameNumber);
            } else if(getLogLevel() >= 1) {
                MY_LOGI("Dump partial physical metadata settings frameNo:%u", request.v3_2.frameNumber);
                mCommonInfo->mMetadataConverter->dump(physicMeta, request.v3_2.frameNumber);
            }
        }
    }

    // get some metadata for output raw buffer format decision
    RawBufferInfo rawInfo = {0, 0, 12} ;
    {
        IMetadata::IEntry const& e1 = mLatestSettings.entryFor(MTK_CONTROL_CAPTURE_PACKED_RAW_ENABLE);
        if ( ! e1.isEmpty() ) {
            rawInfo.packedRaw = e1.itemAt(0, Type2Type<MINT32>());
            MY_LOGD("packedRaw: %d", rawInfo.packedRaw);
        }

        IMetadata::IEntry const& e2 = mLatestSettings.entryFor(MTK_CONTROL_CAPTURE_RAW_BPP);
        if ( ! e2.isEmpty() ) {
            rawInfo.bpp = e2.itemAt(0, Type2Type<MINT32>());
            MY_LOGD("bppOfRaw: %d", rawInfo.bpp);
        }

        IMetadata::IEntry const& e3 = mLatestSettings.entryFor(MTK_CONTROL_CAPTURE_PROCESS_RAW_ENABLE);
        if ( ! e3.isEmpty() ) {
            rawInfo.processed = e3.itemAt(0, Type2Type<MINT32>());
            MY_LOGD("processedRaw: %d", rawInfo.processed);
        }
    }
    //
    //  vOutputImageBuffers <- request.v3_2.outputBuffers
    rRequest.vOutputImageBuffers.setCapacity(request.v3_2.outputBuffers.size());
    for ( const auto& buffer : request.v3_2.outputBuffers )
    {
        sp<AppImageStreamBuffer> pStreamBuffer;
        int err = convertStreamBuffer(queryBufferName(buffer), buffer, pStreamBuffer, rawInfo);
        if ( OK != err )
            return err;
        if ( CC_LIKELY(pStreamBuffer != nullptr && pStreamBuffer->getStreamInfo() != nullptr) ){
            rRequest.vOutputImageBuffers.add(pStreamBuffer->getStreamInfo()->getStreamId(), pStreamBuffer);
        }
    }
    //
    //  vInputImageBuffers <- request.v3_2.inputBuffer
    bool hasInputBuf = (request.v3_2.inputBuffer.streamId != -1 &&
                        request.v3_2.inputBuffer.bufferId != 0);
    if  ( hasInputBuf )
    {
        sp<AppImageStreamBuffer> pStreamBuffer;
        int err = convertStreamBuffer(queryBufferName(request.v3_2.inputBuffer), request.v3_2.inputBuffer, pStreamBuffer, rawInfo);
        if ( OK != err )
            return err;
        if ( CC_LIKELY(pStreamBuffer != nullptr && pStreamBuffer->getStreamInfo() != nullptr) ){
            rRequest.vInputImageBuffers.add(pStreamBuffer->getStreamInfo()->getStreamId(), pStreamBuffer);
        }
    }
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
convertStreamBuffer(
    const std::string& bufferName,
    const StreamBuffer& streamBuffer,
    android::sp<AppImageStreamBuffer>& pStreamBuffer,
    const RawBufferInfo& rawInfo
)   const -> int
{
    MY_LOGD_IF(
        getLogLevel() >= 1,
        "StreamBuffer:%p streamId:%d bufferId:%" PRIu64 " status:%d "
        "handle:%p acquireFence:%p releaseFence:%p",
        &streamBuffer, streamBuffer.streamId, streamBuffer.bufferId, streamBuffer.status,
        streamBuffer.buffer.getNativeHandle(),
        streamBuffer.acquireFence.getNativeHandle(),
        streamBuffer.releaseFence.getNativeHandle()
    );

    if  ( streamBuffer.streamId == -1 || streamBuffer.bufferId == 0 ) {
        CAM_LOGE("invalid streamBuffer streamId:%d bufferId:%" PRIu64 " handle:%p",
            streamBuffer.streamId, streamBuffer.bufferId, streamBuffer.buffer.getNativeHandle());
        return -EINVAL;
    }

    int err = OK;
    buffer_handle_t bufferHandle = nullptr;
    std::shared_ptr<AppStreamBufferHandle> appBufferHandle = nullptr;
    int acquire_fence = -1;
    err = importStreamBuffer(streamBuffer, bufferHandle, appBufferHandle, acquire_fence);
    if  ( OK != err ) {
        ULogPrinter logPrinter(__ULOG_MODULE_ID, LOG_TAG, DetailsType::DETAILS_DEBUG, "importStreamBuffer-Fail");
        mFrameHandler->dumpState(logPrinter, std::vector<std::string>());
        return err;
    }

    if ( appBufferHandle == nullptr ){
        //first time using on this buffer (no record on the BufferHandleCacheMap)
        appBufferHandle = std::make_shared<AppStreamBufferHandle>(bufferHandle);
    }
    pStreamBuffer = createImageStreamBuffer(bufferName, streamBuffer, bufferHandle, appBufferHandle, acquire_fence, -1/*release_fence*/, rawInfo);
    if  ( pStreamBuffer == nullptr ) {
        ULogPrinter logPrinter(__ULOG_MODULE_ID, LOG_TAG, DetailsType::DETAILS_DEBUG, "createImageStreamBuffer-Fail");
        mFrameHandler->dumpState(logPrinter, std::vector<std::string>());
        MY_LOGE("fail to create AppImageStreamBuffer - streamId:%u bufferId:%" PRIu64 " ", streamBuffer.streamId, streamBuffer.bufferId);
        return -ENODEV;
    }
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
importStreamBuffer(
    const StreamBuffer& streamBuffer,
    buffer_handle_t& bufferHandle,
    std::shared_ptr<AppStreamBufferHandle>& appBufferHandle,
    int& acquire_fence
)   const -> int
{
    /**
     * The unique ID of the buffer within this StreamBuffer.
     * 0 indicates this StreamBuffer contains no buffer.
     * For StreamBuffers sent to the HAL in a CaptureRequest, this ID uniquely
     * identifies a buffer. When a buffer is sent to HAL for the first time,
     * both bufferId and buffer handle must be filled. HAL must keep track of
     * the mapping between bufferId and corresponding buffer until the
     * corresponding stream is removed from stream configuration or until camera
     * device session is closed. After the first time a buffer is introduced to
     * HAL, in the future camera service must refer to the same buffer using
     * only bufferId, and keep the buffer handle null.
     */
    bufferHandle = streamBuffer.buffer;
    int err = mFrameHandler->importBuffer(streamBuffer.streamId, streamBuffer.bufferId, bufferHandle, appBufferHandle);
    if  ( OK != err ) {
        return -EINVAL;
    }


    auto duplicateFenceFD = [=](buffer_handle_t handle, int& fd)
    {
        if (handle == nullptr || handle->numFds == 0) {
            fd = -1;
            return true;
        }
        if (handle->numFds != 1) {
            MY_LOGE("invalid fence handle with %d file descriptors", handle->numFds);
            fd = -1;
            return false;
        }
        if (handle->data[0] < 0) {
            fd = -1;
            return true;
        }
        fd = ::dup(handle->data[0]);
        if (fd < 0) {
            MY_LOGE("failed to dup fence fd %d", handle->data[0]);
            return false;
        }
        return true;
    };

    if ( ! duplicateFenceFD(streamBuffer.acquireFence.getNativeHandle(), acquire_fence) ) {
        MY_LOGE("streamId:%d bufferId:%" PRIu64 " acquire fence is invalid", streamBuffer.streamId, streamBuffer.bufferId);
        return -EINVAL;
    }

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
static auto convertToSensorColorOrder(uint8_t colorFilterArrangement) -> int32_t
{
    switch (colorFilterArrangement)
    {
    case MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT_RGGB:
        return (int32_t)SENSOR_FORMAT_ORDER_RAW_R;
    case MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT_GRBG:
        return (int32_t)SENSOR_FORMAT_ORDER_RAW_Gr;
    case MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT_GBRG:
        return (int32_t)SENSOR_FORMAT_ORDER_RAW_Gb;
    case MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT_BGGR:
        return (int32_t)SENSOR_FORMAT_ORDER_RAW_B;
    default:
        break;
    }
    CAM_LOGE("Unsupported Color Filter Arrangement:%d", colorFilterArrangement);
    return -1;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
createImageStreamBuffer(
    const std::string& bufferName,
    const StreamBuffer& streamBuffer,
    buffer_handle_t bufferHandle,
    std::shared_ptr<AppStreamBufferHandle> appBufferHandle,
    int const acquire_fence,
    int const release_fence,
    const RawBufferInfo& rawInfo
)   const -> AppImageStreamBuffer*
{
    sp<IImageStreamInfo> pStreamInfo = mFrameHandler->getConfigImageStream(streamBuffer.streamId);

    int heapFormat = pStreamInfo->getAllocImgFormat();
    int reqFormat  = pStreamInfo->getAllocImgFormat();
    int stride = 0;
    // hack for customer, create packed raw image heap or change unpacked format by bpp
    if (pStreamInfo->getOriImgFormat() == static_cast<int>(HAL_PIXEL_FORMAT_RAW16) && (rawInfo.packedRaw || rawInfo.bpp != 12)) {
        if (rawInfo.packedRaw) {
            reqFormat = eImgFmt_BAYER10;
            stride    = (((pStreamInfo->getImgSize().w) + 0x3F) & ~0x3F)*10/8; // align 64
        } else if (rawInfo.bpp == 10) {
            reqFormat = eImgFmt_BAYER10_UNPAK;
        }

        sp<IImageStreamInfo> pPackedRawStreamInfo = new AppImageStreamInfo(*(static_cast<AppImageStreamInfo*>(pStreamInfo.get())), reqFormat, stride);
        pStreamInfo = pPackedRawStreamInfo; // replace request streamInfo

        MY_LOGD("change raw16 to format:%x, allocFormat:%x", pStreamInfo->getImgFormat(), pStreamInfo->getAllocImgFormat());
    }

    if (heapFormat == static_cast<int>(eImgFmt_BLOB) && pStreamInfo->getImgFormat() == static_cast<int>(eImgFmt_JPEG)) {
        heapFormat = pStreamInfo->getImgFormat();
    }

    sp<IGraphicImageBufferHeap> pImageBufferHeap =
        IGraphicImageBufferHeap::create(
            (bufferName + ":" + pStreamInfo->getStreamName()).c_str(),
            pStreamInfo->getUsageForAllocator(),
            pStreamInfo->getAllocImgFormat() == eImgFmt_BLOB ? MSize(pStreamInfo->getAllocBufPlanes().planes[0].rowStrideInBytes, 1): pStreamInfo->getImgSize(),
            heapFormat,
            &bufferHandle,
            acquire_fence,
            release_fence,
            stride,
            pStreamInfo->getSecureInfo().type,
            mIonDevice
        );
    if ( pImageBufferHeap == nullptr ){
        ULogPrinter logPrinter(__ULOG_MODULE_ID, LOG_TAG, DetailsType::DETAILS_DEBUG, "createIGraphicImageBufferHeap-Fail");
        mFrameHandler->dumpState(logPrinter, std::vector<std::string>());
        MY_LOGF("IGraphicImageBufferHeap::create \"%s:%s\", handle: %p, fd: %d",
                 bufferName.c_str(), pStreamInfo->getStreamName(), bufferHandle, bufferHandle->data[0]);
    }

    if ( pStreamInfo->getAllocImgFormat() == HAL_PIXEL_FORMAT_RAW16
      && pStreamInfo->getStreamType() == eSTREAMTYPE_IMAGE_IN )
    {
        uint8_t colorFilterArrangement = 0;
        IMetadata const& staticMetadata = mCommonInfo->mMetadataProvider->getMtkStaticCharacteristics();
        bool ret = IMetadata::getEntry(&staticMetadata, MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT, colorFilterArrangement);
        MY_LOGF_IF(!ret, "no static info: MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT");
        auto sensorColorOrder = convertToSensorColorOrder(colorFilterArrangement);
        pImageBufferHeap->setColorArrangement(sensorColorOrder);
    }

    AppImageStreamBuffer* pStreamBuffer =
    AppImageStreamBuffer::Allocator(pStreamInfo.get())(streamBuffer.bufferId, pImageBufferHeap.get(), bufferHandle, appBufferHandle);
    return pStreamBuffer;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
createMetaStreamBuffer(
    android::sp<IMetaStreamInfo> pStreamInfo,
    IMetadata const& rSettings,
    bool const repeating
)   const -> AppMetaStreamBuffer*
{
    AppMetaStreamBuffer* pStreamBuffer =
    AppMetaStreamBuffer::Allocator(pStreamInfo.get())(rSettings);
    //
    if ( CC_LIKELY(pStreamBuffer != nullptr) ){
        pStreamBuffer->setRepeating(repeating);
    }
    //
    return pStreamBuffer;
}

