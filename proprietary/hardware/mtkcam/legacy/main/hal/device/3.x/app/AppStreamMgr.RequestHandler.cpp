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

#define ThisNamespace   AppStreamMgr::RequestHandler

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
    const hidl_vec<CaptureRequest>& captureRequests,
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
    CAM_TRACE_CALL();
    for ( const auto& rRequest : rRequests ) {
        int err = mFrameHandler->registerFrame(rRequest);
        if ( err != OK )
            return err;
    }
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
createRequests(
    const hidl_vec<CaptureRequest>& requests,
    android::Vector<Request>& rRequests
)   -> int
{
    CAM_TRACE_CALL();

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
            requests[i].frameNumber, i, requests.size());
        return err;
    }

    return  OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
checkOneRequest(const CaptureRequest& request) const -> int
{
    //  there are 0 output buffers
    if ( request.outputBuffers.size() == 0 )
    {
        MY_LOGE("[frameNo:%u] outputBuffers.size()==0", request.frameNumber);
        return  -EINVAL;
    }
    //
    //  not allowed if the fmq size is zero and settings are NULL w/o the lastest setting.
    if ( request.fmqSettingsSize == 0 &&
         request.settings.size() == 0 && mLatestSettings.isEmpty() )
    {
        MY_LOGE("[frameNo:%u] NULL request settings; "
                "however most-recently submitted request is also NULL after configureStreams",
                request.frameNumber);
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
    const CaptureRequest& request,
    Request& rRequest
)   -> int
{
    auto queryBufferName = [=](auto const& buffer) -> std::string {
        return ("r"+std::to_string(request.frameNumber)+":b"+std::to_string(buffer.bufferId));
    };

    CAM_TRACE_CALL();
    //
    rRequest.frameNo = request.frameNumber;
    //
    //  vOutputImageBuffers <- request.outputBuffers
    rRequest.vOutputImageBuffers.setCapacity(request.outputBuffers.size());
    for ( const auto& buffer : request.outputBuffers )
    {
        sp<AppImageStreamBuffer> pStreamBuffer;
        int err = convertStreamBuffer(queryBufferName(buffer), buffer, pStreamBuffer);
        if ( OK != err )
            return err;
        rRequest.vOutputImageBuffers.add(pStreamBuffer->getStreamInfo()->getStreamId(), pStreamBuffer);
    }
    //
    //  vInputImageBuffers <- request.inputBuffer
    bool hasInputBuf = (request.inputBuffer.streamId != -1 &&
                        request.inputBuffer.bufferId != 0);
    if  ( hasInputBuf )
    {
        sp<AppImageStreamBuffer> pStreamBuffer;
        int err = convertStreamBuffer(queryBufferName(request.inputBuffer), request.inputBuffer, pStreamBuffer);
        if ( OK != err )
            return err;
        rRequest.vInputImageBuffers.add(pStreamBuffer->getStreamInfo()->getStreamId(), pStreamBuffer);
    }
    //
    //  vInputMetaBuffers <- request.settings
    {
        sp<IMetaStreamInfo> pStreamInfo = mFrameHandler->getConfigMetaStream(0);
        bool isRepeating = false;
        //
        if ( request.fmqSettingsSize > 0 ) {        // read settings from FMQ.
            isRepeating = false;
            mLatestSettings.clear();
            CameraMetadata settingsFmq;
            settingsFmq.resize(request.fmqSettingsSize);
            // non-blocking read; client must write metadata before calling
            bool read = mRequestMetadataQueue->read(settingsFmq.data(), request.fmqSettingsSize);
            if ( !read ) {
                MY_LOGE("frameNo:%u capture request settings metadata couldn't be read from fmq!", request.frameNumber);
                return -ENODEV;
            }
            CAM_TRACE_BEGIN("convertFromHidl: settingsFmq -> mLatestSettings");
            if ( ! mCommonInfo->mMetadataConverter->convertFromHidl(&settingsFmq, mLatestSettings) ) {
                MY_LOGE("frameNo:%u convertFromHidl: settingsFmq -> mLatestSettings", request.frameNumber);
                return -ENODEV;
            }
            CAM_TRACE_END();
        }
        else if ( request.settings.size() > 0 ) {   // read settings from metadata settings.
            isRepeating = false;
            mLatestSettings.clear();
            CAM_TRACE_BEGIN("convertFromHidl: request.settings -> mLatestSettings");
            if ( ! mCommonInfo->mMetadataConverter->convertFromHidl(&request.settings, mLatestSettings) ) {
                MY_LOGE("frameNo:%u convertFromHidl: request.settings -> mLatestSettings", request.frameNumber);
                return -ENODEV;
            }
            CAM_TRACE_END();
        }
        else {
            /**
             * As a special case, a NULL settings buffer indicates that the
             * settings are identical to the most-recently submitted capture request. A
             * NULL buffer cannot be used as the first submitted request after a
             * configure_streams() call.
             */
            isRepeating = true;
            MY_LOGD_IF(getLogLevel() >= 1,
                "frameNo:%u NULL settings -> most-recently submitted capture request",
                request.frameNumber);
        }

        sp<AppMetaStreamBuffer> pStreamBuffer = createMetaStreamBuffer(pStreamInfo, mLatestSettings, isRepeating);
        rRequest.vInputMetaBuffers.add(pStreamBuffer->getStreamInfo()->getStreamId(), pStreamBuffer);

        // to debug
        if ( ! isRepeating ) {

            if (getLogLevel() >= 2) {
                mCommonInfo->mMetadataConverter->dumpAll(mLatestSettings, request.frameNumber);
            } else if(getLogLevel() >= 1) {
                mCommonInfo->mMetadataConverter->dump(mLatestSettings, request.frameNumber);
            }

            IMetadata::IEntry const& e1 = mLatestSettings.entryFor(MTK_CONTROL_AF_TRIGGER);
            if ( ! e1.isEmpty() ) {
                MUINT8 af_trigger = e1.itemAt(0, Type2Type< MUINT8 >());
                if ( af_trigger==MTK_CONTROL_AF_TRIGGER_START )
                {
                    CAM_TRACE_FMT_BEGIN("AF_state: %d", af_trigger);
                    MY_LOGD_IF(getLogLevel() >= 1, "AF_state: %d", af_trigger);
                    CAM_TRACE_END();
                }
            }
            //
            IMetadata::IEntry const& e2 = mLatestSettings.entryFor(MTK_CONTROL_AE_PRECAPTURE_TRIGGER);
            if ( ! e2.isEmpty() ) {
                MUINT8 ae_pretrigger = e2.itemAt(0, Type2Type< MUINT8 >());
                if ( ae_pretrigger==MTK_CONTROL_AE_PRECAPTURE_TRIGGER_START )
                {
                    CAM_TRACE_FMT_BEGIN("ae precap: %d", ae_pretrigger);
                    MY_LOGD_IF(getLogLevel() >= 1, "ae precap: %d", ae_pretrigger);
                    CAM_TRACE_END();
                }
            }
            //
            IMetadata::IEntry const& e3 = mLatestSettings.entryFor(MTK_CONTROL_CAPTURE_INTENT);
            if ( ! e3.isEmpty() ) {
                MUINT8 capture_intent = e3.itemAt(0, Type2Type< MUINT8 >());
                CAM_TRACE_FMT_BEGIN("capture intent: %d", capture_intent);
                //MY_LOGD_IF(getLogLevel() >= 1, "capture intent: %d", capture_intent);
                CAM_LOGI("capture intent: %d", capture_intent);
                CAM_TRACE_END();
            }
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
    android::sp<AppImageStreamBuffer>& pStreamBuffer
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
    int acquire_fence = -1;
    err = importStreamBuffer(streamBuffer, bufferHandle, acquire_fence);
    if  ( OK != err ) {
        android::LogPrinter logPrinter(LOG_TAG, ANDROID_LOG_DEBUG, "importStreamBuffer-Fail");
        mFrameHandler->dumpState(logPrinter, std::vector<std::string>());
        return err;
    }

    pStreamBuffer = createImageStreamBuffer(bufferName, streamBuffer, bufferHandle, acquire_fence, -1/*release_fence*/);
    if  ( pStreamBuffer == nullptr ) {
        android::LogPrinter logPrinter(LOG_TAG, ANDROID_LOG_DEBUG, "createImageStreamBuffer-Fail");
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
    int err = mFrameHandler->importBuffer(streamBuffer.streamId, streamBuffer.bufferId, bufferHandle);
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
auto
ThisNamespace::
createImageStreamBuffer(
    const std::string& bufferName,
    const StreamBuffer& streamBuffer,
    buffer_handle_t bufferHandle,
    int const acquire_fence,
    int const release_fence
)   const -> AppImageStreamBuffer*
{
    sp<IImageStreamInfo> pStreamInfo = mFrameHandler->getConfigImageStream(streamBuffer.streamId);
	
	int format = pStreamInfo->getImgFormat();
	MY_LOGD("MYL format = %d", format);
	if(format == eImgFmt_JPEG)
		MY_LOGD("MYL format = eImgFmt_JPEG");

	size_t rowStrideInBytes = pStreamInfo->getBufPlanes()[0].rowStrideInBytes;
	MY_LOGD("MYL format = %zu", rowStrideInBytes);
	
    sp<IGraphicImageBufferHeap> pImageBufferHeap =
		  IGraphicImageBufferHeap::create(
            (bufferName + ":" + pStreamInfo->getStreamName()).c_str(),
            pStreamInfo->getUsageForAllocator(),
            pStreamInfo->getImgFormat() == eImgFmt_JPEG ? MSize(rowStrideInBytes , 1) : pStreamInfo->getImgSize(),
            format,
            &bufferHandle,
            acquire_fence,
            release_fence
        );

    AppImageStreamBuffer* pStreamBuffer =
    AppImageStreamBuffer::Allocator(pStreamInfo.get())(streamBuffer.bufferId, pImageBufferHeap.get());
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
    pStreamBuffer->setRepeating(repeating);
    //
    return pStreamBuffer;
}

