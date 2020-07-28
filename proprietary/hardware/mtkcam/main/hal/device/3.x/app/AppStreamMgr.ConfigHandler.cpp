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
#include <mtkcam/pipeline/stream/StreamId.h>
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
    const V3_4::StreamConfiguration& requestedConfiguration,
    V3_4::HalStreamConfiguration& halConfiguration,
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
        StreamId_T streamId = srcStream.v3_2.id;
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
                    srcStream.v3_2.id, pStreamInfo->getStream().v3_2.streamType, srcStream.v3_2.streamType,
                    pStreamInfo->getStream().v3_2.width, pStreamInfo->getStream().v3_2.height, srcStream.v3_2.width, srcStream.v3_2.height,
                    pStreamInfo->getStream().v3_2.format, srcStream.v3_2.format,
                    pStreamInfo->getStream().v3_2.dataSpace, srcStream.v3_2.dataSpace,
                    pStreamInfo->getStream().v3_2.usage, srcStream.v3_2.usage
                );
            };

            MY_LOGI("stream re-configuration: %s", generateLogString().c_str());

            // refer to google default wrapper implementation:
            // width/height/format must not change, but usage/rotation might need to change
            bool check1 = (srcStream.v3_2.streamType == pStreamInfo->getStream().v3_2.streamType
                        && srcStream.v3_2.width      == pStreamInfo->getStream().v3_2.width
                        && srcStream.v3_2.height     == pStreamInfo->getStream().v3_2.height
                        // && srcStream.v3_2.dataSpace  == pStreamInfo->getStream().v3_2.dataSpace
                        );
            bool check2 = true || (srcStream.v3_2.format == pStreamInfo->getStream().v3_2.format
                        ||(srcStream.v3_2.format == (PixelFormat)pStreamInfo->getImgFormat() &&
                           PixelFormat::IMPLEMENTATION_DEFINED == pStreamInfo->getStream().v3_2.format)
                      //||(pStreamInfo->getStream().v3_2.format == real format of srcStream.v3_2.format &&
                      //   PixelFormat::IMPLEMENTATION_DEFINED == srcStream.v3_2.format)
                        );
            if ( ! check1 || ! check2 ) {
                MY_LOGE("stream configuration changed! %s", generateLogString().c_str());
                return -ENODEV;
            }

            // If usage is chaged, it implies that
            // the real format (flexible yuv/implementation_defined)
            // and the buffer layout may change.
            // In this case, should HAL and Frameworks have to cleanup the buffer handle cache?
            if ( pStreamInfo->getStream().v3_2.usage != srcStream.v3_2.usage ) {
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
                srcStream.v3_2.id, srcStream.v3_2.streamType, srcStream.v3_2.width, srcStream.v3_2.height,
                srcStream.v3_2.format, srcStream.v3_2.usage, srcStream.v3_2.dataSpace, srcStream.v3_2.rotation);
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
    V3_4::HalStreamConfiguration& halConfiguration
) -> int
{
    android::LogPrinter logPrinter(LOG_TAG, ANDROID_LOG_DEBUG, "[endConfigureStreams] ");
    //clear old BatchStreamId
    mBatchHandler->resetBatchStreamId();
    std::unordered_set<StreamId_T> usedStreamIds;
    usedStreamIds.reserve(halConfiguration.streams.size());
    for (size_t i = 0; i < halConfiguration.streams.size(); i++) {
        auto& halStream = halConfiguration.streams[i];
        StreamId_T const streamId = halStream.v3_3.v3_2.id;
        auto pStreamInfo = mFrameHandler->getConfigImageStream(streamId);
        if ( pStreamInfo == nullptr ) {
            MY_LOGE(
                "no image stream info for stream id %" PRId64 " - %zu/%zu",
                streamId, i, halConfiguration.streams.size()
            );
            return -ENODEV;
        }

        mBatchHandler->checkStreamUsageforBatchMode(pStreamInfo);
        usedStreamIds.insert(streamId);

        // a stream in demand ? => set its maxBuffers
        halStream.v3_3.v3_2.maxBuffers = pStreamInfo->getMaxBufNum();

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
checkStream(const V3_4::Stream& rStream) const -> int
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
    if ( Dataspace::DEPTH == (Dataspace)rStream.v3_2.dataSpace ) {
        MY_LOGE("Not support depth dataspace! %s", toString(rStream).c_str());
        return -EINVAL;
    } else if ( Dataspace::UNKNOWN != (Dataspace)rStream.v3_2.dataSpace ) {
        MY_LOGW("framework stream dataspace:0x%08x(%s) %s", rStream.v3_2.dataSpace, mCommonInfo->mGrallocHelper->queryDataspaceName(rStream.v3_2.dataSpace).c_str(), toString(rStream).c_str());
    }
    //
    switch ( rStream.v3_2.rotation )
    {
    case StreamRotation::ROTATION_0:
        break;
    case StreamRotation::ROTATION_90:
    case StreamRotation::ROTATION_180:
    case StreamRotation::ROTATION_270:
        MY_LOGI("%s", toString(rStream).c_str());
        if ( StreamType::INPUT == rStream.v3_2.streamType ) {
            MY_LOGE("input stream cannot support rotation");
            return -EINVAL;
        }

        switch (rStream.v3_2.format)
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
        MY_LOGE("rotation:%d out of bounds - %s", rStream.v3_2.rotation, toString(rStream).c_str());
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
    sp<AppImageStreamInfo> pInfo = mFrameHandler->getConfigImageStream(rStream.v3_2.id);
    for (MUINT i = 0; i < entryScaler.count(); i += 4 )
    {
        int32_t const format = entryScaler.itemAt(i, Type2Type< MINT32 >());
        if ( (int32_t)rStream.v3_2.format==format ||
             (pInfo.get() && pInfo->getOriImgFormat()==format ) )
        {
            MUINT32 scalerWidth  = entryScaler.itemAt(i + 1, Type2Type< MINT32 >());
            MUINT32 scalerHeight = entryScaler.itemAt(i + 2, Type2Type< MINT32 >());

            if ( ( rStream.v3_2.width == scalerWidth && rStream.v3_2.height == scalerHeight ) ||
                 ( rStream.v3_2.rotation&StreamRotation::ROTATION_90 &&
                   rStream.v3_2.width == scalerHeight && rStream.v3_2.height == scalerWidth ) )
            {
                return OK;
            }
        }
    }
    MY_LOGE("unsupported size %dx%d for format 0x%x/rotation:%d - %s",
            rStream.v3_2.width, rStream.v3_2.height, rStream.v3_2.format, rStream.v3_2.rotation, toString(rStream).c_str());
    return -EINVAL;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
checkStreams(const hidl_vec<V3_4::Stream>& vStreamConfigured) const -> int
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
            MY_LOGE("streams[id:%d] has a bad status: %d(%s)", stream.v3_2.id, err, ::strerror(-err));
            return err;
        }
        //
        typeNum.editValueFor(stream.v3_2.streamType)++;
        if ( StreamType::INPUT != stream.v3_2.streamType )
            outRotationNum.editValueFor(stream.v3_2.rotation)++;
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
    const V3_4::Stream& rStream,
    V3_4::HalStream& rOutStream
)   const -> AppImageStreamInfo*
{
    int err = OK;
    //
    MUINT64 const usageForHal = (GRALLOC_USAGE_SW_READ_OFTEN|GRALLOC_USAGE_SW_WRITE_OFTEN) |
                          GRALLOC1_PRODUCER_USAGE_CAMERA ;
    MUINT64 const usageForHalClient = rStream.v3_2.usage;
    MUINT64 const usageForAllocator = usageForHal | usageForHalClient;
    MINT32  const formatToAllocate  = static_cast<MINT32>(rStream.v3_2.format);
    //
    //
    IGrallocHelper* pGrallocHelper = mCommonInfo->mGrallocHelper;
    GrallocStaticInfo   grallocStaticInfo;
    GrallocRequest      grallocRequest;
    grallocRequest.usage  = usageForAllocator;
    grallocRequest.format = formatToAllocate;
    MY_LOGD("grallocRequest.format=%d, grallocRequest.usage = %#" PRIx64 " ", grallocRequest.format, grallocRequest.usage);

    if  ( HAL_PIXEL_FORMAT_BLOB == formatToAllocate ) {
        IMetadata::IEntry const& entry = mCommonInfo->mMetadataProvider->getMtkStaticCharacteristics().entryFor(MTK_JPEG_MAX_SIZE);
        if  ( entry.isEmpty() ) {
            MY_LOGE("no static JPEG_MAX_SIZE");
            grallocRequest.widthInPixels = rStream.v3_2.width * rStream.v3_2.height * 2;
        }
        else {
            grallocRequest.widthInPixels = entry.itemAt(0, Type2Type<MINT32>());
        }
        grallocRequest.heightInPixels = 1;
    }
    else {
        grallocRequest.widthInPixels  = rStream.v3_2.width;
        grallocRequest.heightInPixels = rStream.v3_2.height;
    }
    //
    err = pGrallocHelper->query(&grallocRequest, &grallocStaticInfo);
    if  ( OK != err ) {
        MY_LOGE("IGrallocHelper::query - err:%d(%s)", err, ::strerror(-err));
        return NULL;
    }
    //
    //  stream name = s<stream id>:d<device id>:App:<format>:<hal client usage>
    String8 s8StreamName = String8::format("s%d:d%d:App:", rStream.v3_2.id, mCommonInfo->mInstanceId);
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
        MY_LOGE("Unsupported format:0x%x(%s), grallocRequest.format=%d, grallocRequest.usage = %#" PRIx64 " ",
            grallocStaticInfo.format, s8FormatAllocated.c_str(), grallocRequest.format, grallocRequest.usage);
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
    rOutStream.v3_3.v3_2.id = rStream.v3_2.id;
    rOutStream.v3_3.v3_2.overrideFormat =
        (  PixelFormat::IMPLEMENTATION_DEFINED == rStream.v3_2.format
        //[ALPS03443045] Don't override it since there's a bug in API1 -> HAL3.
        //StreamingProcessor::recordingStreamNeedsUpdate always return true for video stream.
        && (GRALLOC_USAGE_HW_VIDEO_ENCODER & rStream.v3_2.usage) == 0
        //we don't have input stream's producer usage to determine the real format.
        && StreamType::OUTPUT == rStream.v3_2.streamType  )
            ? static_cast<PixelFormat>(grallocStaticInfo.format)
            : rStream.v3_2.format;
    rOutStream.v3_3.v3_2.producerUsage = (rStream.v3_2.streamType==StreamType::OUTPUT) ? usageForHal : 0;
    rOutStream.v3_3.v3_2.consumerUsage = (rStream.v3_2.streamType==StreamType::OUTPUT) ? 0 : usageForHal;
    rOutStream.v3_3.v3_2.maxBuffers    = 1;

    auto const& pStreamInfo = mFrameHandler->getConfigImageStream(rStream.v3_2.id);
    AppImageStreamInfo::CreationInfo creationInfo =
    {
        .mStreamName        = s8StreamName,
        .mvbufPlanes        = bufPlanes,
        .mImgFormat         = grallocStaticInfo.format,
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

