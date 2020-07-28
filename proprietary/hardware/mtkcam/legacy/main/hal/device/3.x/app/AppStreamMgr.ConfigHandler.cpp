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
#include <mtkcam/v3/app/StreamId.h>
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;

#define ThisNamespace   AppStreamMgr::ConfigHandler

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
ConfigHandler(
    std::shared_ptr<CommonInfo> pCommonInfo,
    android::sp<FrameHandler> pFrameHandler,
    android::sp<BatchHandler> pBatchHandler
)
    : mInstanceName{std::to_string(pCommonInfo->mInstanceId) + "-ConfigHandler"}
    , mCommonInfo(pCommonInfo)
    , mFrameHandler(pFrameHandler)
    , mBatchHandler(pBatchHandler)
{
    mEntryMinDuration =
        mCommonInfo->mMetadataProvider->getMtkStaticCharacteristics().entryFor(MTK_SCALER_AVAILABLE_MIN_FRAME_DURATIONS);
    if  ( mEntryMinDuration.isEmpty() ) {
        MY_LOGE("no static MTK_SCALER_AVAILABLE_MIN_FRAME_DURATIONS");
    }
    mEntryStallDuration =
        mCommonInfo->mMetadataProvider->getMtkStaticCharacteristics().entryFor(MTK_SCALER_AVAILABLE_STALL_DURATIONS);
    if  ( mEntryStallDuration.isEmpty() ) {
        MY_LOGE("no static MTK_SCALER_AVAILABLE_STALL_DURATIONS");
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
beginConfigureStreams(
    const StreamConfiguration& requestedConfiguration,
    HalStreamConfiguration& halConfiguration,
    ConfigAppStreams& rStreams
)   -> int
{
    auto addFrameDuration = [this](auto& rStreams, auto const pStreamInfo) {
        for (size_t j = 0; j < mEntryMinDuration.count(); j+=4) {
            if (mEntryMinDuration.itemAt(j    , Type2Type<MINT64>()) == (MINT64)pStreamInfo->getOriImgFormat() &&
                mEntryMinDuration.itemAt(j + 1, Type2Type<MINT64>()) == (MINT64)pStreamInfo->getLandscapeSize().w &&
                mEntryMinDuration.itemAt(j + 2, Type2Type<MINT64>()) == (MINT64)pStreamInfo->getLandscapeSize().h)
            {
                rStreams.vMinFrameDuration.add(
                    pStreamInfo->getStreamId(),
                    mEntryMinDuration.itemAt(j + 3, Type2Type<MINT64>())
                );
                rStreams.vStallFrameDuration.add(
                    pStreamInfo->getStreamId(),
                    mEntryStallDuration.itemAt(j + 3, Type2Type<MINT64>())
                );
                MY_LOGI("[addFrameDuration] format:%" PRId64 " size:%" PRId64 "x%" PRId64 " min_duration:%" PRId64 ", stall_duration:%" PRId64 ,
                    mEntryMinDuration.itemAt(j, Type2Type<MINT64>()),
                    mEntryMinDuration.itemAt(j + 1, Type2Type<MINT64>()),
                    mEntryMinDuration.itemAt(j + 2, Type2Type<MINT64>()),
                    mEntryMinDuration.itemAt(j + 3, Type2Type<MINT64>()),
                    mEntryStallDuration.itemAt(j + 3, Type2Type<MINT64>()));
                break;
            }
        }
        return;
    };
    //
    //
    int err = OK;
    err = checkStreams(requestedConfiguration.streams);
    if  ( OK != err ) {
        MY_LOGE("checkStreams failed - StreamConfiguration=%s", toString(requestedConfiguration).c_str());
        return err;
    }
    //
    mFrameHandler->setOperationMode((uint32_t)requestedConfiguration.operationMode);
    //
    {
        StreamId_T const streamId = eSTREAMID_END_OF_FWK;
        auto pStreamInfo = createMetaStreamInfo(streamId);
        mFrameHandler->addConfigStream(pStreamInfo);

        rStreams.vMetaStreams.add(streamId, pStreamInfo);
    }
    //
    halConfiguration.streams.resize(requestedConfiguration.streams.size());
    rStreams.vImageStreams.setCapacity(requestedConfiguration.streams.size());
    for ( size_t i = 0; i < requestedConfiguration.streams.size(); i++ )
    {
        const auto& srcStream = requestedConfiguration.streams[i];
              auto& dstStream = halConfiguration.streams[i];
        StreamId_T streamId = srcStream.id;
        //
        sp<AppImageStreamInfo> pStreamInfo = mFrameHandler->getConfigImageStream(streamId);
        if ( pStreamInfo == nullptr )
        {
            pStreamInfo = createImageStreamInfo(srcStream, dstStream);
            if ( pStreamInfo == nullptr ) {
                MY_LOGE("createImageStreamInfo failed - Stream=%s", toString(srcStream).c_str());
                return -ENODEV;
            }
            mFrameHandler->addConfigStream(pStreamInfo.get(), false/*keepBufferCache*/);
        }
        else
        {
            auto generateLogString = [=]() {
                return String8::format("streamId:%d type(%d:%d) "
                    "size(%dx%d:%dx%d) format(%d:%d) dataSpace(%d:%d) "
                    "usage(%#" PRIx64 ":%#" PRIx64 ")",
                    srcStream.id, pStreamInfo->getStream().streamType, srcStream.streamType,
                    pStreamInfo->getStream().width, pStreamInfo->getStream().height, srcStream.width, srcStream.height,
                    pStreamInfo->getStream().format, srcStream.format,
                    pStreamInfo->getStream().dataSpace, srcStream.dataSpace,
                    pStreamInfo->getStream().usage, srcStream.usage
                );
            };

            MY_LOGI("stream re-configuration: %s", generateLogString().c_str());

            // refer to google default wrapper implementation:
            // width/height/format must not change, but usage/rotation might need to change
            bool check1 = (srcStream.streamType == pStreamInfo->getStream().streamType
                        && srcStream.width      == pStreamInfo->getStream().width
                        && srcStream.height     == pStreamInfo->getStream().height
                        && srcStream.dataSpace  == pStreamInfo->getStream().dataSpace
                        );
            bool check2 = (srcStream.format == pStreamInfo->getStream().format
                        ||(srcStream.format == (PixelFormat)pStreamInfo->getImgFormat() &&
                           PixelFormat::IMPLEMENTATION_DEFINED == pStreamInfo->getStream().format)
                      //||(pStreamInfo->getStream().format == real format of srcStream.format &&
                      //   PixelFormat::IMPLEMENTATION_DEFINED == srcStream.format)
                        );
            if ( ! check1 || ! check2 ) {
                MY_LOGE("stream configuration changed! %s", generateLogString().c_str());
                return -ENODEV;
            }

            // If usage is chaged, it implies that
            // the real format (flexible yuv/implementation_defined)
            // and the buffer layout may change.
            // In this case, should HAL and Frameworks have to cleanup the buffer handle cache?
            if ( pStreamInfo->getStream().usage != srcStream.usage ) {
                MY_LOGW("stream usage changed! %s", generateLogString().c_str());
                MY_LOGW("shall HAL and Frameworks have to clear buffer handle cache?");
            }

            // Create a new stream to override the old one, since usage/rotation might need to change.
            pStreamInfo = createImageStreamInfo(srcStream, dstStream);
            mFrameHandler->addConfigStream(pStreamInfo.get(), true/*keepBufferCache*/);
        }

        rStreams.vImageStreams.add(streamId, pStreamInfo);
        addFrameDuration(rStreams, pStreamInfo);

        MY_LOGD_IF(getLogLevel()>=2, "Stream: id:%d streamType:%d %dx%d format:0x%x usage:0x%" PRIx64 " dataSpace:0x%x rotation:%d",
                srcStream.id, srcStream.streamType, srcStream.width, srcStream.height,
                srcStream.format, srcStream.usage, srcStream.dataSpace, srcStream.rotation);
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
endConfigureStreams(
    HalStreamConfiguration& halConfiguration
) -> int
{
    android::LogPrinter logPrinter(LOG_TAG, ANDROID_LOG_DEBUG, "[endConfigureStreams] ");

    std::unordered_set<StreamId_T> usedStreamIds;
    usedStreamIds.reserve(halConfiguration.streams.size());
    for (size_t i = 0; i < halConfiguration.streams.size(); i++) {
        auto& halStream = halConfiguration.streams[i];
        StreamId_T const streamId = halStream.id;
        auto pStreamInfo = mFrameHandler->getConfigImageStream(streamId);
        if ( pStreamInfo == nullptr ) {
            MY_LOGE(
                "no image stream info for stream id %" PRId64 " - %zu/%zu",
                streamId, i, halConfiguration.streams.size()
            );
            return -ENODEV;
        }

        usedStreamIds.insert(streamId);

        // a stream in demand ? => set its maxBuffers
        halStream.maxBuffers = pStreamInfo->getMaxBufNum();

        pStreamInfo->dumpState(logPrinter);
    }
    mFrameHandler->removeUnusedConfigStream(usedStreamIds);

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
checkStream(const Stream& rStream) const -> int
{
    //
    /**
     * Return values:
     *
     *  0:      On successful stream configuration
     *
     * -EINVAL: If the requested stream configuration is invalid. Some examples
     *          of invalid stream configurations include:
     *
     *          - Including streams with unsupported formats, or an unsupported
     *            size for that format.
     *
     *          ....
     *
     *          - Unsupported rotation configuration (only applies to
     *            devices with version >= CAMERA_DEVICE_API_VERSION_3_3)
     */
    if ( Dataspace::DEPTH == (Dataspace)rStream.dataSpace ) {
        MY_LOGE("Not support depth dataspace! %s", toString(rStream).c_str());
        return -EINVAL;
    } else if ( Dataspace::UNKNOWN != (Dataspace)rStream.dataSpace ) {
        MY_LOGW("framework stream dataspace:0x%08x(%s) %s", rStream.dataSpace, mCommonInfo->mGrallocHelper->queryDataspaceName(rStream.dataSpace).c_str(), toString(rStream).c_str());
    }
    //
    switch ( rStream.rotation )
    {
    case StreamRotation::ROTATION_0:
        break;
    case StreamRotation::ROTATION_90:
    case StreamRotation::ROTATION_180:
    case StreamRotation::ROTATION_270:
        MY_LOGI("%s", toString(rStream).c_str());
        if ( StreamType::INPUT == rStream.streamType ) {
            MY_LOGE("input stream cannot support rotation");
            return -EINVAL;
        }

        switch (rStream.format)
        {
        case PixelFormat::RAW16:
        case PixelFormat::RAW_OPAQUE:
            MY_LOGE("raw stream cannot support rotation");
            return -EINVAL;
        default:
            break;
        }

        break;
    default:
        MY_LOGE("rotation:%d out of bounds - %s", rStream.rotation, toString(rStream).c_str());
        return -EINVAL;
    }
    //
    IMetadata::IEntry const& entryScaler = mCommonInfo->mMetadataProvider->getMtkStaticCharacteristics().entryFor(MTK_SCALER_AVAILABLE_STREAM_CONFIGURATIONS);
    if ( entryScaler.isEmpty() )
    {
        MY_LOGE("no static MTK_SCALER_AVAILABLE_STREAM_CONFIGURATIONS");
        return -EINVAL;
    }

    // android.scaler.availableStreamConfigurations
    // int32 x n x 4
    sp<AppImageStreamInfo> pInfo = mFrameHandler->getConfigImageStream(rStream.id);
    for (MUINT i = 0; i < entryScaler.count(); i += 4 )
    {
        int32_t const format = entryScaler.itemAt(i, Type2Type< MINT32 >());
        if ( (int32_t)rStream.format==format ||
             (pInfo.get() && pInfo->getOriImgFormat()==format ) )
        {
            MUINT32 scalerWidth  = entryScaler.itemAt(i + 1, Type2Type< MINT32 >());
            MUINT32 scalerHeight = entryScaler.itemAt(i + 2, Type2Type< MINT32 >());

            if ( ( rStream.width == scalerWidth && rStream.height == scalerHeight ) ||
                 ( rStream.rotation&StreamRotation::ROTATION_90 &&
                   rStream.width == scalerHeight && rStream.height == scalerWidth ) )
            {
                return OK;
            }
        }
    }
    MY_LOGE("unsupported size %dx%d for format 0x%x/rotation:%d - %s",
            rStream.width, rStream.height, rStream.format, rStream.rotation, toString(rStream).c_str());
    return -EINVAL;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
checkStreams(const hidl_vec<Stream>& vStreamConfigured) const -> int
{
    CAM_TRACE_CALL();
    if  ( 0 == vStreamConfigured.size() ) {
        MY_LOGE("StreamConfiguration.streams.size() = 0");
        return -EINVAL;
    }
    //
    //
    KeyedVector<StreamType, size_t> typeNum;
    typeNum.add(StreamType::OUTPUT,          0);
    typeNum.add(StreamType::INPUT,           0);
    //
    KeyedVector<StreamRotation, size_t> outRotationNum;
    outRotationNum.add(StreamRotation::ROTATION_0,      0);
    outRotationNum.add(StreamRotation::ROTATION_90,     0);
    outRotationNum.add(StreamRotation::ROTATION_180,    0);
    outRotationNum.add(StreamRotation::ROTATION_270,    0);
    //
    for (const auto& stream : vStreamConfigured) {
        //
        auto err = checkStream(stream);
        if  ( OK != err ) {
            MY_LOGE("streams[id:%d] has a bad status: %d(%s)", stream.id, err, ::strerror(-err));
            return err;
        }
        //
        typeNum.editValueFor(stream.streamType)++;
        if ( StreamType::INPUT != stream.streamType )
            outRotationNum.editValueFor(stream.rotation)++;
    }

    /**
     * At most one input-capable stream may be defined (INPUT or BIDIRECTIONAL)
     * in a single configuration.
     *
     * At least one output-capable stream must be defined (OUTPUT or
     * BIDIRECTIONAL).
     */
    /*
     *
     * Return values:
     *
     *  0:      On successful stream configuration
     *
     * -EINVAL: If the requested stream configuration is invalid. Some examples
     *          of invalid stream configurations include:
     *
     *          - Including more than 1 input-capable stream (INPUT or
     *            BIDIRECTIONAL)
     *
     *          - Not including any output-capable streams (OUTPUT or
     *            BIDIRECTIONAL)
     *
     *          - Including too many output streams of a certain format.
     *
     *          - Unsupported rotation configuration (only applies to
     *            devices with version >= CAMERA_DEVICE_API_VERSION_3_3)
     */
    if  ( (typeNum[(int)StreamType::INPUT] > 1) ||  (typeNum[(int)StreamType::OUTPUT] == 0) )
    {
        MY_LOGE( "bad stream count: (out, in)=(%zu, %zu)",
                 typeNum[(int)StreamType::OUTPUT], typeNum[(int)StreamType::INPUT] );
        return  -EINVAL;
    }
    //
    size_t const num_rotation_not0 = outRotationNum[(int)StreamRotation::ROTATION_90] +
                                     outRotationNum[(int)StreamRotation::ROTATION_180] +
                                     outRotationNum[(int)StreamRotation::ROTATION_270];
    if ( num_rotation_not0 > 1 )
    {
        MY_LOGW("more than one output streams need to rotation: (0, 90, 180, 270)=(%zu,%zu,%zu,%zu)",
                outRotationNum[(int)StreamRotation::ROTATION_0],   outRotationNum[(int)StreamRotation::ROTATION_90],
                outRotationNum[(int)StreamRotation::ROTATION_180], outRotationNum[(int)StreamRotation::ROTATION_270]);
        return -EINVAL;
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
createImageStreamInfo(
    const Stream& rStream,
    HalStream& rOutStream
)   const -> AppImageStreamInfo*
{
    CAM_TRACE_CALL();

    int err = OK;
    //
#if 1
    MUINT64 const usageForHal =
                    (GRALLOC_USAGE_SW_READ_OFTEN|GRALLOC_USAGE_SW_WRITE_OFTEN)
                  | (GRALLOC_USAGE_HW_CAMERA_READ|GRALLOC_USAGE_HW_CAMERA_WRITE)
                    ;
#else
    MUINT64 const usageForHal =
                    (GRALLOC_USAGE_SW_READ_OFTEN|GRALLOC_USAGE_SW_WRITE_OFTEN)
                  | (GRALLOC_USAGE_HW_CAMERA_WRITE | (rStream.streamTtype == StreamType::INPUT ? GRALLOC_USAGE_HW_CAMERA_READ : 0))
                  | (GRALLOC_USAGE_CAMERA)
                    ;
#endif
    MUINT64 const usageForHalClient = rStream.usage;
    MUINT64 const usageForAllocator = usageForHal | usageForHalClient;
    MINT32  const formatToAllocate  = static_cast<MINT32>(rStream.format);
    //
    //
    IGrallocHelper* pGrallocHelper = mCommonInfo->mGrallocHelper;
    GrallocStaticInfo   grallocStaticInfo;
    GrallocRequest      grallocRequest;
    grallocRequest.usage  = usageForAllocator;
    grallocRequest.format = formatToAllocate;
    if  ( HAL_PIXEL_FORMAT_BLOB == formatToAllocate ) {
        IMetadata::IEntry const& entry = mCommonInfo->mMetadataProvider->getMtkStaticCharacteristics().entryFor(MTK_JPEG_MAX_SIZE);
        if  ( entry.isEmpty() ) {
            MY_LOGE("no static JPEG_MAX_SIZE");
            grallocRequest.widthInPixels = rStream.width * rStream.height * 2;
        }
        else {
            grallocRequest.widthInPixels = entry.itemAt(0, Type2Type<MINT32>());
        }
        grallocRequest.heightInPixels = 1;
    }
    else {
        grallocRequest.widthInPixels  = rStream.width;
        grallocRequest.heightInPixels = rStream.height;
    }
    //
    err = pGrallocHelper->query(&grallocRequest, &grallocStaticInfo);
    if  ( OK != err ) {
        MY_LOGE("IGrallocHelper::query - err:%d(%s)", err, ::strerror(-err));
        return NULL;
    }
    //
    //  stream name = s<stream id>:d<device id>:App:<format>:<hal client usage>
    String8 s8StreamName = String8::format("s%d:d%d:App:", rStream.id, mCommonInfo->mInstanceId);
    String8 const s8FormatAllocated  = pGrallocHelper->queryPixelFormatName(grallocStaticInfo.format);
    switch  (grallocStaticInfo.format)
    {
    case HAL_PIXEL_FORMAT_BLOB:
    case HAL_PIXEL_FORMAT_YV12:
    case HAL_PIXEL_FORMAT_YCRCB_420_SP:
    case HAL_PIXEL_FORMAT_YCBCR_422_I:
    case HAL_PIXEL_FORMAT_RAW16:
    case HAL_PIXEL_FORMAT_RAW_OPAQUE:
    case HAL_PIXEL_FORMAT_CAMERA_OPAQUE:
        s8StreamName += s8FormatAllocated;
        break;
    case HAL_PIXEL_FORMAT_Y16:
    default:
        MY_LOGE("Unsupported format:0x%x(%s)", grallocStaticInfo.format, s8FormatAllocated.c_str());
        return NULL;
    }
    //
    s8StreamName += ":";
    s8StreamName += pGrallocHelper->queryGrallocUsageName(usageForHalClient);
    //
    IImageStreamInfo::BufPlanes_t bufPlanes;
    bufPlanes.resize(grallocStaticInfo.planes.size());
    for (size_t i = 0; i < bufPlanes.size(); i++)
    {
        IImageStreamInfo::BufPlane& plane = bufPlanes.editItemAt(i);
        plane.sizeInBytes      = grallocStaticInfo.planes[i].sizeInBytes;
        plane.rowStrideInBytes = grallocStaticInfo.planes[i].rowStrideInBytes;
    }
    //
    rOutStream.id = rStream.id;
    rOutStream.overrideFormat =
        (  PixelFormat::IMPLEMENTATION_DEFINED == rStream.format
        //[ALPS03443045] Don't override it since there's a bug in API1 -> HAL3.
        //StreamingProcessor::recordingStreamNeedsUpdate always return true for video stream.
        && (GRALLOC_USAGE_HW_VIDEO_ENCODER & rStream.usage) == 0
        //we don't have input stream's producer usage to determine the real format.
        && StreamType::OUTPUT == rStream.streamType  )
            ? static_cast<PixelFormat>(grallocStaticInfo.format)
            : rStream.format;
    rOutStream.producerUsage = (rStream.streamType==StreamType::OUTPUT) ? usageForHal : 0;
    rOutStream.consumerUsage = (rStream.streamType==StreamType::OUTPUT) ? 0 : usageForHal;
    rOutStream.maxBuffers    = 1;

    auto const& pStreamInfo = mFrameHandler->getConfigImageStream(rStream.id);
	MINT imgFormat = (grallocStaticInfo.format == HAL_PIXEL_FORMAT_BLOB
		&& rStream.dataSpace == static_cast<int32_t>(Dataspace::V0_JFIF)) ?
		eImgFmt_JPEG : grallocStaticInfo.format;
    AppImageStreamInfo::CreationInfo creationInfo =
    {
        .mStreamName        = s8StreamName,
        .mvbufPlanes        = bufPlanes,
        .mImgFormat         = imgFormat,
        .mOriImgFormat      = (pStreamInfo.get())? pStreamInfo->getOriImgFormat() : formatToAllocate,
        .mStream            = rStream,
        .mHalStream         = rOutStream,
    };
    AppImageStreamInfo* pStream = new AppImageStreamInfo(creationInfo);
    return pStream;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
createMetaStreamInfo(
    StreamId_T suggestedStreamId
)   const -> AppMetaStreamInfo*
{
    return new AppMetaStreamInfo(
        "Meta:App:Control",
        suggestedStreamId,
        eSTREAMTYPE_META_IN,
        0
    );
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
overrideOutputFormat(
    const int32_t  inFormat,
    const uint64_t inUsage,
    int32_t &outFormat
)   const -> void
{
    // might be moved to custom folder
    if ( inFormat==HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED )
    {
        int32_t defaultFormat = HAL_PIXEL_FORMAT_RGBA_8888;
        const int u_video = GRALLOC_USAGE_HW_VIDEO_ENCODER;
        const int u_camera = GRALLOC1_USAGE_CAMERA;
        const int u_camera_texture  = GRALLOC_USAGE_HW_TEXTURE | GRALLOC1_USAGE_CAMERA;
        const int u_camera_composer = GRALLOC_USAGE_HW_COMPOSER | GRALLOC1_USAGE_CAMERA;
        const int u_camera_zsl      = GRALLOC_USAGE_HW_CAMERA_ZSL | GRALLOC1_USAGE_CAMERA;

        if ((inUsage & u_video) == u_video)
            outFormat = HAL_PIXEL_FORMAT_YV12;
        else if ((inUsage & u_camera_composer) == u_camera_composer)
            outFormat = HAL_PIXEL_FORMAT_YCBCR_422_I;
        else if ((inUsage & u_camera_texture) == u_camera_texture)
            outFormat = HAL_PIXEL_FORMAT_YV12;
        else if ((inUsage & u_camera_zsl) == u_camera_zsl)
            outFormat = HAL_PIXEL_FORMAT_CAMERA_OPAQUE;
        else if ((inUsage & u_camera) == u_camera)
            outFormat = HAL_PIXEL_FORMAT_YCBCR_422_I;
    }
    else {
        outFormat = inFormat;
    }
    //
    MY_LOGD("inFormat:0x%x usage:0x%" PRIX64 " -> outFormat:0x%x", inFormat, inUsage, outFormat);
}

