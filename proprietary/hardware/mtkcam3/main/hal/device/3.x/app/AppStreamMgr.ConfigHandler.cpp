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
#include <mtkcam3/pipeline/stream/StreamId.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/LogicalCam/IHalLogicalDeviceList.h>
//
#include <mtkcam/utils/std/TypeTraits.h>
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::Utils::ULog;
#define ThisNamespace   AppStreamMgr::ConfigHandler

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


static MINT findStreamSensorId(int32_t DevId, const V3_4::Stream& rStream, const NSCam::ImageBufferInfo& imgBufferInfo)
{

    auto pHalLogicalDeviceList = MAKE_HalLogicalDeviceList();
    if(CC_UNLIKELY(pHalLogicalDeviceList == nullptr)){
        return -1;
    }
    char const* sensorId = rStream.physicalCameraId.c_str();
    MINT deviceId;
    if(strlen(sensorId)>0){
        MINT vid = atoi(sensorId);
        deviceId = pHalLogicalDeviceList->getDeviceIdByVID(vid);
    }else{
        deviceId = -1;
    }
    // 0-length string is not a physical output stream
    CAM_ULOGMI("stream id len : %zu", strlen(sensorId));
    if (strlen(sensorId) != 0)
    {
        return deviceId;
    }
    return -1;
}

/******************************************************************************
 *
 ******************************************************************************/
ThisNamespace::
ConfigHandler(
    std::shared_ptr<CommonInfo> pCommonInfo,
    android::sp<FrameHandler> pFrameHandler,
    android::sp<BatchHandler> pBatchHandler,
    std::shared_ptr<ResultMetadataQueue> pResultMetadataQueue
)
    : mInstanceName{std::to_string(pCommonInfo->mInstanceId) + "-ConfigHandler"}
    , mCommonInfo(pCommonInfo)
    , mFrameHandler(pFrameHandler)
    , mBatchHandler(pBatchHandler)
    , mResultMetadataQueue(pResultMetadataQueue)
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
    mCurrentFMQSize = CAMERA_RESULT_METADATA_QUEUE_SIZE;
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
std::shared_ptr<ThisNamespace::ParsedSMVRBatchInfo>
ThisNamespace::extractSMVRBatchInfo(const V3_4::StreamConfiguration& requestedConfiguration)
{
    IMetadata metaSessionParams;
    if ( ! mCommonInfo->mMetadataConverter->convertFromHidl(&(requestedConfiguration.sessionParams), metaSessionParams) )
    {
        MY_LOGE("SMVRBatch::Bad Session parameters");
        return nullptr;
    }
    const MUINT32 operationMode = (const MUINT32) requestedConfiguration.operationMode;

    MINT32 customP2BatchNum = 1;
    MINT32 p2IspBatchNum = 1;
    std::shared_ptr<ParsedSMVRBatchInfo> pParsedSMVRBatchInfo = nullptr;
    int isFromApMeta = 0;

    IMetadata::IEntry const& entry = metaSessionParams.entryFor(MTK_SMVR_FEATURE_SMVR_MODE);

    if (getLogLevel()>=2)
    {
        MY_LOGD("SMVRBatch: chk metaSessionParams.count()=%d, MTK_SMVR_FEATURE_SMVR_MODE: count: %d", metaSessionParams.count(), entry.count());
        metaSessionParams.dump();
    }

    if  ( !entry.isEmpty() && entry.count() >= 2 )
    {
        isFromApMeta = 1;
        pParsedSMVRBatchInfo = std::make_shared<ParsedSMVRBatchInfo>();
        if  ( CC_UNLIKELY(pParsedSMVRBatchInfo == nullptr) ) {
            CAM_LOGE("[%s] Fail on make_shared<pParsedSMVRBatchInfo>", __FUNCTION__);
            return nullptr;
        }
        // get image w/h
        for ( size_t i = 0; i < requestedConfiguration.streams.size(); i++ )
        {
            const auto& srcStream = requestedConfiguration.streams[i];
    //        if (usage & GRALLOC_USAGE_HW_VIDEO_ENCODER)
            if (pParsedSMVRBatchInfo->imgW == 0) // !!NOTES: assume previewSize = videoOutSize
            {
                // found
                pParsedSMVRBatchInfo->imgW = srcStream.v3_2.width;
                pParsedSMVRBatchInfo->imgH = srcStream.v3_2.height;
//                break;
            }
            MY_LOGD("SMVRBatch: vImageStreams[%zu]=%dx%d, isVideo=%#" PRIx64 "", i, srcStream.v3_2.width, srcStream.v3_2.height, (srcStream.v3_2.usage & GRALLOC_USAGE_HW_VIDEO_ENCODER));
        }

        if  ( !entry.isEmpty() && entry.count() >= 2)
        {
            pParsedSMVRBatchInfo->maxFps = entry.itemAt(0, Type2Type< MINT32 >()); // meta[0]: LmaxFps
            customP2BatchNum = entry.itemAt(1, Type2Type< MINT32 >());             // meta[1]: customP2BatchNum
        }
        pParsedSMVRBatchInfo->maxFps = ::property_get_int32("vendor.debug.smvrb.maxFps", pParsedSMVRBatchInfo->maxFps);
//        if (pParsedSMVRBatchInfo->maxFps <= 120)
//        {
//            MY_LOGE("SMVRBatch: !!err: only support slow motion more than 120fps: curr-maxFps=%d", pParsedSMVRBatchInfo->maxFps);
//            return nullptr;
//        }
        // determine final P2BatchNum
        #define min(a,b)  ((a) < (b) ? (a) : (b))
        MUINT32 vOutSize = pParsedSMVRBatchInfo->imgW * pParsedSMVRBatchInfo->imgH;
        if (vOutSize <= 640*480) // vga
        {
            p2IspBatchNum = ::property_get_int32("ro.vendor.smvr.p2batch.vga", 1);
        }
        else if (vOutSize <= 1280*736) // hd
        {
            p2IspBatchNum = ::property_get_int32("ro.vendor.smvr.p2batch.hd", 1);
        }
        else if (vOutSize <= 1920*1088) // fhd
        {
            p2IspBatchNum = ::property_get_int32("ro.vendor.smvr.p2batch.fhd", 1);
        }
        else
        {
           p2IspBatchNum = 1;
        }
        // change p2IspBatchNum by debug adb if necessary
        p2IspBatchNum = ::property_get_int32("vendor.debug.smvrb.P2BatchNum", p2IspBatchNum);
        // final P2BatchNum
        pParsedSMVRBatchInfo->p2BatchNum = min(p2IspBatchNum, customP2BatchNum);
        #undef min

        // P1BatchNum
        pParsedSMVRBatchInfo->p1BatchNum = pParsedSMVRBatchInfo->maxFps/30;
        // operatioin mode
        pParsedSMVRBatchInfo->opMode = operationMode;

        // log level
        pParsedSMVRBatchInfo->logLevel = ::property_get_int32("vendor.debug.smvrb.loglevel", 0);
    }
    else
    {
        MINT32 propSmvrBatchEnable = ::property_get_int32("vendor.debug.smvrb.enable", 0);
        if (propSmvrBatchEnable)
        {
            pParsedSMVRBatchInfo = std::make_shared<ParsedSMVRBatchInfo>();
            if  ( CC_UNLIKELY(pParsedSMVRBatchInfo == nullptr) ) {
                CAM_LOGE("[%s] Fail on make_shared<pParsedSMVRBatchInfo>", __FUNCTION__);
                return nullptr;
            }

            // get image w/h
            for ( size_t i = 0; i < requestedConfiguration.streams.size(); i++ )
            {
                const auto& srcStream = requestedConfiguration.streams[i];
        //        if (usage & GRALLOC_USAGE_HW_VIDEO_ENCODER)
                if (pParsedSMVRBatchInfo->imgW == 0) // !!NOTES: assume previewSize = videoOutSize
                {
                    // found
                    pParsedSMVRBatchInfo->imgW = srcStream.v3_2.width;
                    pParsedSMVRBatchInfo->imgH = srcStream.v3_2.height;
    //                break;
                }
                MY_LOGD("SMVRBatch: vImageStreams[%zu]=%dx%d, isVideo=%" PRIx64 "", i, srcStream.v3_2.width, srcStream.v3_2.height, (srcStream.v3_2.usage & GRALLOC_USAGE_HW_VIDEO_ENCODER));
            }

            pParsedSMVRBatchInfo->maxFps = ::property_get_int32("vendor.debug.smvrb.maxFps", pParsedSMVRBatchInfo->maxFps);
//            if (pParsedSMVRBatchInfo->maxFps <= 120)
//            {
//                MY_LOGE("SMVRBatch: !!err: only support slow motion more than 120fps: curr-maxFps=%d", pParsedSMVRBatchInfo->maxFps);
//                return nullptr;
//            }

            pParsedSMVRBatchInfo->p2BatchNum = ::property_get_int32("vendor.debug.smvrb.P2BatchNum", 1);
            pParsedSMVRBatchInfo->p1BatchNum = ::property_get_int32("vendor.debug.smvrb.P1BatchNum", 1);
            pParsedSMVRBatchInfo->opMode     = operationMode;
            pParsedSMVRBatchInfo->logLevel   = ::property_get_int32("vendor.debug.smvrb.loglevel", 0);
        }
    }

    if (pParsedSMVRBatchInfo != nullptr)
    {
         MY_LOGD("SMVRBatch: isFromApMeta=%d, vOutImg=%dx%d, meta-info(maxFps=%d, customP2BatchNum=%d), p2IspBatchNum=%d, final-P2BatchNum=%d, p1BatchNum=%d, opMode=%d, logLevel=%d",
             isFromApMeta,
             pParsedSMVRBatchInfo->imgW, pParsedSMVRBatchInfo->imgH,
             pParsedSMVRBatchInfo->maxFps, customP2BatchNum,
             p2IspBatchNum, pParsedSMVRBatchInfo->p2BatchNum,
             pParsedSMVRBatchInfo->p1BatchNum,
             pParsedSMVRBatchInfo->opMode,
             pParsedSMVRBatchInfo->logLevel
         );

    }
    else
    {
         MY_LOGD("SMVRBatch: no need");
    }

    return pParsedSMVRBatchInfo;
}
/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
beginConfigureStreams(
    const V3_5::StreamConfiguration& requestedConfiguration,
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
    err = checkStreams(requestedConfiguration.v3_4.streams);
    if  ( OK != err ) {
        MY_LOGE("checkStreams failed - StreamConfiguration=%s", toString(requestedConfiguration.v3_4).c_str());
        return err;
    }
    //
    mFrameHandler->setOperationMode((uint32_t)requestedConfiguration.v3_4.operationMode);
    //
    {
        StreamId_T const streamId = eSTREAMID_END_OF_FWK;
        std::string const streamName = "Meta:App:Control";
        auto pStreamInfo = createMetaStreamInfo(streamName.c_str(), streamId);
        mFrameHandler->addConfigStream(pStreamInfo);

        rStreams.vMetaStreams.add(streamId, pStreamInfo);
    }

    //
    mspParsedSMVRBatchInfo = extractSMVRBatchInfo(requestedConfiguration.v3_4);

    //
    if(!checkResizeFMQ(requestedConfiguration.v3_4))
    {
        MY_LOGW("Failed to resize FMQ");
    }
    //
    halConfiguration.streams.resize(requestedConfiguration.v3_4.streams.size());
    rStreams.vImageStreams.setCapacity(requestedConfiguration.v3_4.streams.size());
    for ( size_t i = 0; i < requestedConfiguration.v3_4.streams.size(); i++ )
    {
        const auto& srcStream = requestedConfiguration.v3_4.streams[i];
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

        // check hidl_stream contain physic id or not.
        if(srcStream.physicalCameraId.size() != 0)
        {
            auto& sensorList = rStreams.vPhysicCameras;
            MINT32 vid = std::stoi((std::string)srcStream.physicalCameraId);
            auto pcId = MAKE_HalLogicalDeviceList()->getDeviceIdByVID(vid);
            MY_LOGD("pcid(%d:%s)", pcId, ((std::string)srcStream.physicalCameraId).c_str());
            auto iter = std::find(
                                    sensorList.begin(),
                                    sensorList.end(),
                                    pcId);
            if(iter == sensorList.end())
            {
                sensorList.push_back(pcId);
                //physical settings
                String8 const streamName = String8::format("Meta:App:Physical_%s", ((std::string)srcStream.physicalCameraId).c_str());
                StreamId_T const streamId = eSTREAMID_BEGIN_OF_PHYSIC_ID + (int64_t)pcId;
                auto pMetaStreamInfo = createMetaStreamInfo(streamName.c_str(), streamId, pcId);
                mFrameHandler->addConfigStream(pMetaStreamInfo);
                rStreams.vMetaStreams_physical.add(pcId, pMetaStreamInfo);
            }
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
bool
ThisNamespace::
checkResizeFMQ(const V3_4::StreamConfiguration& requestedConfiguration)
{
    // enlarge RequestMetadataQueue size if needed (CES)
    MBOOL bNeedChangeFMQSize = MFALSE;
    IMetadata metaSessionParams;
    if(auto pResultMetadataQueue = mResultMetadataQueue)
    {
        if ( ! mCommonInfo->mMetadataConverter->convertFromHidl(&(requestedConfiguration.sessionParams), metaSessionParams) )
        {
            MY_LOGE("Resize FMQ, Bad Session parameters");
            return false;
        }
        // check need change FMQ size or not
        IMetadata::IEntry const& entry = metaSessionParams.entryFor(MTK_CONTROL_CAPTURE_ISP_TUNING_DATA_ENABLE);
        if( entry.isEmpty() )
        {
            if(mCurrentFMQSize == CAMERA_RESULT_METADATA_QUEUE_SIZE_CES)
            {
                bNeedChangeFMQSize = MTRUE;
                mCurrentFMQSize    = CAMERA_RESULT_METADATA_QUEUE_SIZE;
            }
        }
        else
        {
            MUINT8 iIspTuningEnable = entry.itemAt(0, Type2Type< MUINT8 >());
            MY_LOGI("query tag MTK_CONTROL_CAPTURE_ISP_TUNING_DATA_ENABLE success, %u", iIspTuningEnable);
            if(iIspTuningEnable == 1 && mCurrentFMQSize == CAMERA_RESULT_METADATA_QUEUE_SIZE)
            {
                bNeedChangeFMQSize = MTRUE;
                mCurrentFMQSize    = CAMERA_RESULT_METADATA_QUEUE_SIZE_CES;
            }
        }
        // resize result metadata queue
        if(bNeedChangeFMQSize)
        {
            MY_LOGI("Need Resize FMQ, request size : %zu, result size : %zu", CAMERA_REQUEST_METADATA_QUEUE_SIZE, mCurrentFMQSize);
            pResultMetadataQueue = std::make_shared<RequestMetadataQueue>(
                mCurrentFMQSize, false /* non blocking */);
            if  ( !pResultMetadataQueue || !pResultMetadataQueue->isValid() ) {
                MY_LOGE("invalid result fmq");
                return false;
            }
        }
    }
    else
    {
        MY_LOGW("Can not access FMQ, stop change FMQ size");
        return false;
    }
    return true;
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
    ULogPrinter logPrinter(__ULOG_MODULE_ID, LOG_TAG, DetailsType::DETAILS_DEBUG, "[endConfigureStreams] ");
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
    sp<IMetadataProvider> currentMetadataProvider = nullptr;
    if (rStream.physicalCameraId.size() != 0){
        MUINT32 vid = (MUINT32)std::stoi((std::string)rStream.physicalCameraId);
        MY_LOGI("Check the setting of physical camera %u instead of logical camera for streamId %d", vid, rStream.v3_2.id);
        auto search = mCommonInfo->mPhysicalMetadataProviders.find(vid);
        if (search != mCommonInfo->mPhysicalMetadataProviders.end()){
            currentMetadataProvider = search->second;
        }
        else{
            MY_LOGE("Couldn't find corresponding metadataprovider of physical cam %u", vid);
            return -EINVAL;
        }
    }
    else{
        currentMetadataProvider = mCommonInfo->mMetadataProvider;
    }
    IMetadata::IEntry const& entryScaler = currentMetadataProvider->getMtkStaticCharacteristics().entryFor(MTK_SCALER_AVAILABLE_STREAM_CONFIGURATIONS);
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
    CAM_ULOGM_FUNCLIFE();
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
    const bool isSecureCameraDevice = ((MAKE_HalLogicalDeviceList()->getSupportedFeature(
                mCommonInfo->mInstanceId) & DEVICE_FEATURE_SECURE_CAMERA) &
                DEVICE_FEATURE_SECURE_CAMERA)>0;
    const bool isSecureUsage = ((rStream.v3_2.usage & GRALLOC_USAGE_PROTECTED) & GRALLOC_USAGE_PROTECTED)>0;
    const bool isSecureFlow = isSecureCameraDevice && isSecureUsage;
    if (isSecureCameraDevice && !isSecureUsage)
    {
        MY_LOGE("Not support insecure stream for a secure camera device",
                toString(rStream).c_str());
        return nullptr;
    }
    //
    // For secure camera device, the HAL client is expected to set
    // GRALLOC1_PRODUCER_USAGE_PROTECTED, meaning that the buffer is protected from
    // direct CPU access outside the isolated execution environment
    // (e.g. TrustZone or Hypervisor-based solution) or
    // being read by non-secure hardware.
    //
    // Moreover, this flag is incompatible with CPU read and write flags.
    MUINT64 const usageForHal = GRALLOC1_PRODUCER_USAGE_CAMERA |
        (isSecureFlow ?
        (GRALLOC_USAGE_SW_READ_NEVER|GRALLOC_USAGE_SW_WRITE_NEVER) :
        (GRALLOC_USAGE_SW_READ_OFTEN|GRALLOC_USAGE_SW_WRITE_OFTEN));
    MUINT64 const usageForHalClient = rStream.v3_2.usage;
    MUINT64 usageForAllocator = usageForHal | usageForHalClient;
    MY_LOGD("isSecureCameraDevice=%d usageForHal=0x%#" PRIx64 " usageForHalClient=0x%#" PRIx64 "",
            isSecureCameraDevice, usageForHal, usageForHalClient);
    MINT32  const formatToAllocate  = static_cast<MINT32>(rStream.v3_2.format);
    //
    usageForAllocator = (rStream.v3_2.streamType==StreamType::OUTPUT) ? usageForAllocator : usageForAllocator | GRALLOC_USAGE_HW_CAMERA_ZSL;
    //
    IGrallocHelper* pGrallocHelper = mCommonInfo->mGrallocHelper;
    GrallocStaticInfo   grallocStaticInfo;
    GrallocRequest      grallocRequest;
    grallocRequest.usage  = usageForAllocator;
    if (mspParsedSMVRBatchInfo != nullptr)
    {
       if (rStream.v3_2.usage == GRALLOC_USAGE_HW_VIDEO_ENCODER)
       {
           grallocRequest.usage |= GRALLOC1_USAGE_SMVR;
       }
    }
    grallocRequest.format = formatToAllocate;
    MY_LOGD("grallocRequest.format=%d, grallocRequest.usage = 0x%x ", grallocRequest.format, grallocRequest.usage);

    if  ( HAL_PIXEL_FORMAT_BLOB == formatToAllocate ) {
        auto const dataspace = (Dataspace)rStream.v3_2.dataSpace;
        auto const bufferSz  = rStream.bufferSize;
        // For BLOB format with dataSpace Dataspace::DEPTH, this must be zero and and HAL must
        // determine the buffer size based on ANDROID_DEPTH_MAX_DEPTH_SAMPLES.
        if ( Dataspace::DEPTH == dataspace ) {
            // should be return in checkStream.
            MY_LOGE("Not support depth dataspace %s", toString(rStream).c_str());
            return nullptr;
        }
        // For BLOB format with dataSpace Dataspace::JFIF, this must be non-zero and represent the
        // maximal size HAL can lock using android.hardware.graphics.mapper lock API.
        else if ( Dataspace::V0_JFIF == dataspace ) {
            if ( CC_UNLIKELY(bufferSz==0) ) {
                MY_LOGW("V0_JFIF with bufferSize(0)");
                IMetadata::IEntry const& entry = mCommonInfo->mMetadataProvider->getMtkStaticCharacteristics().entryFor(MTK_JPEG_MAX_SIZE);
                if  ( entry.isEmpty() ) {
                    MY_LOGW("no static JPEG_MAX_SIZE");
                    grallocRequest.widthInPixels = rStream.v3_2.width * rStream.v3_2.height * 2;
                }
                else {
                    grallocRequest.widthInPixels = entry.itemAt(0, Type2Type<MINT32>());
                }
            } else {
                grallocRequest.widthInPixels = bufferSz;
            }
            grallocRequest.heightInPixels = 1;
            MY_LOGI("BLOB with widthInPixels(%d), heightInPixels(%d), bufferSize(%u)",
                    grallocRequest.widthInPixels, grallocRequest.heightInPixels, rStream.bufferSize);
        }
        else {
            if ( bufferSz!=0 )
                grallocRequest.widthInPixels = bufferSz;
            else
                grallocRequest.widthInPixels = rStream.v3_2.width * rStream.v3_2.height * 2;
            grallocRequest.heightInPixels = 1;
            MY_LOGW("undefined dataspace(0x%x) with bufferSize(%u) in BLOB format -> %dx%d",
                    static_cast<int>(dataspace), bufferSz, grallocRequest.widthInPixels, grallocRequest.heightInPixels);
        }
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
    case HAL_PIXEL_FORMAT_NV12:
    case HAL_PIXEL_FORMAT_RGB_888:
    case HAL_PIXEL_FORMAT_RAW16:
    case HAL_PIXEL_FORMAT_RAW_OPAQUE:
    case HAL_PIXEL_FORMAT_CAMERA_OPAQUE:
        s8StreamName += s8FormatAllocated;
        break;
    case HAL_PIXEL_FORMAT_Y16:
    default:
        MY_LOGE("Unsupported format:0x%x(%s), grallocRequest.format=%d, grallocRequest.usage = %d ",
            grallocStaticInfo.format, s8FormatAllocated.c_str(), grallocRequest.format, grallocRequest.usage);
        return NULL;
    }
    //
    s8StreamName += ":";
    s8StreamName += pGrallocHelper->queryGrallocUsageName(usageForHalClient);
    //
    IImageStreamInfo::BufPlanes_t bufPlanes;
    bufPlanes.count = grallocStaticInfo.planes.size();
    for (size_t i = 0; i < bufPlanes.count; i++)
    {
        IImageStreamInfo::BufPlane& plane = bufPlanes.planes[i];
        plane.sizeInBytes      = grallocStaticInfo.planes[i].sizeInBytes;
        plane.rowStrideInBytes = grallocStaticInfo.planes[i].rowStrideInBytes;
    }
    //
    rOutStream.v3_3.v3_2.id = rStream.v3_2.id;
    rOutStream.physicalCameraId = rStream.physicalCameraId;
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
    // SMVRBatch: producer usage
    if (mspParsedSMVRBatchInfo != nullptr)
    {
       if (rStream.v3_2.usage == GRALLOC_USAGE_HW_VIDEO_ENCODER)
       {
           rOutStream.v3_3.v3_2.producerUsage |= GRALLOC1_USAGE_SMVR;
       }
    }
    rOutStream.v3_3.v3_2.consumerUsage = (rStream.v3_2.streamType==StreamType::OUTPUT) ? 0 : usageForHal;
    rOutStream.v3_3.v3_2.maxBuffers    = 1;
    rOutStream.v3_3.overrideDataSpace = rStream.v3_2.dataSpace;

    auto const& pStreamInfo = mFrameHandler->getConfigImageStream(rStream.v3_2.id);
    MINT imgFormat = (grallocStaticInfo.format == HAL_PIXEL_FORMAT_BLOB
                    && rStream.v3_2.dataSpace == static_cast<int32_t>(Dataspace::V0_JFIF)) ?
                    eImgFmt_JPEG : grallocStaticInfo.format;
    imgFormat = ( grallocStaticInfo.format == HAL_PIXEL_FORMAT_RAW16 ? eImgFmt_BAYER12_UNPAK : imgFormat );
    MINT allocImgFormat = static_cast<MINT>(grallocStaticInfo.format);
    IImageStreamInfo::BufPlanes_t allocBufPlanes = bufPlanes;

    // SMVRBatch:: handle blob layout
    uint32_t oneImgTotalSizeInBytes_32align = 0;
    if (mspParsedSMVRBatchInfo != nullptr)
    {
        if (rStream.v3_2.usage == GRALLOC_USAGE_HW_VIDEO_ENCODER)
        {
            uint32_t oneImgTotalSizeInBytes = 0;
            uint32_t oneImgTotalStrideInBytes = 0;

            allocBufPlanes.count = 1;
            allocImgFormat = static_cast<MINT>(eImgFmt_BLOB); // for smvr-batch mode

            for (size_t i = 0; i < bufPlanes.count; i++)
            {
                MY_LOGD("SMVRBatch: idx=%zu, (sizeInBytes, rowStrideInBytes)=(%zu,%zu)", i, grallocStaticInfo.planes[i].sizeInBytes, grallocStaticInfo.planes[i].rowStrideInBytes);
                oneImgTotalSizeInBytes += grallocStaticInfo.planes[i].sizeInBytes;
    //                oneImgTotalStrideInBytes += grallocStaticInfo.planes[i].rowStrideInBytes;
                oneImgTotalStrideInBytes = oneImgTotalSizeInBytes;
            }
            oneImgTotalSizeInBytes_32align = (((oneImgTotalSizeInBytes-1)>>5)+1)<<5;
            allocBufPlanes.planes[0].sizeInBytes = oneImgTotalSizeInBytes_32align  * mspParsedSMVRBatchInfo->p2BatchNum;
            allocBufPlanes.planes[0].rowStrideInBytes = allocBufPlanes.planes[0].sizeInBytes;

            // debug message
            MY_LOGD_IF(mspParsedSMVRBatchInfo->logLevel >= 2, "SMVRBatch: %s: isVideo=%d \n"
                "\t rStream-Info(sid=0x%x, format=0x%x, usage=0x%#" PRIx64 ", StreamType::OUTPUT=0x%x, streamType=0x%x) \n"
                "\t grallocStaticInfo(format=0x%x) \n"
                "\t HalStream-info(producerUsage= %#" PRIx64 ",  consumerUsage= %#" PRIx64 ", overrideFormat=0x%x ) \n"
                "\t Blob-info(imgFmt=0x%x, allocImgFmt=0x%x, vOutBurstNum=%d, oneImgTotalSizeInBytes(%d, 32align-%d ), oneImgTotalStrideInBytes=%d, allocBufPlanes(size=%zu, sizeInBytes=%zu, rowStrideInBytes=%zu) \n"
                "\t Misc-info(usageForAllocator=%#" PRIx64 ", GRALLOC1_USAGE_SMVR=%#" PRIx64 ") \n"
                , __FUNCTION__, (rStream.v3_2.usage == GRALLOC_USAGE_HW_VIDEO_ENCODER)
                , rStream.v3_2.id, rStream.v3_2.format, rStream.v3_2.usage, StreamType::OUTPUT, rStream.v3_2.streamType
                , static_cast<PixelFormat>(grallocStaticInfo.format)
                , rOutStream.v3_3.v3_2.producerUsage, rOutStream.v3_3.v3_2.consumerUsage, rOutStream.v3_3.v3_2.overrideFormat
                , imgFormat, allocImgFormat, mspParsedSMVRBatchInfo->p2BatchNum, oneImgTotalSizeInBytes, oneImgTotalSizeInBytes_32align, oneImgTotalStrideInBytes, allocBufPlanes.count, allocBufPlanes.planes[0].sizeInBytes, allocBufPlanes.planes[0].rowStrideInBytes
                , usageForAllocator, GRALLOC1_USAGE_SMVR
                );
        }
    } else if (grallocStaticInfo.format == HAL_PIXEL_FORMAT_RAW16) { // Alloc Format: Raw16 -> Blob
        allocBufPlanes.count = 1;
        allocImgFormat = static_cast<MINT>(eImgFmt_BLOB);
        allocBufPlanes.planes[0].sizeInBytes      = grallocStaticInfo.planes[0].sizeInBytes;
        allocBufPlanes.planes[0].rowStrideInBytes = grallocStaticInfo.planes[0].sizeInBytes;
        MY_LOGD("Raw16: (sizeInBytes, rowStrideInBytes)=(%zu,%zu)", allocBufPlanes.planes[0].sizeInBytes, allocBufPlanes.planes[0].rowStrideInBytes);
    }

    NSCam::ImageBufferInfo imgBufferInfo;
    imgBufferInfo.count   = 1;
    imgBufferInfo.bufStep = 0;
    imgBufferInfo.startOffset = 0;
    if (mspParsedSMVRBatchInfo != nullptr)
    {
        // SMVRBatch:: buffer offset setting
        if (rStream.v3_2.usage == GRALLOC_USAGE_HW_VIDEO_ENCODER)
        {
            imgBufferInfo.count   = mspParsedSMVRBatchInfo->p2BatchNum;
            imgBufferInfo.bufStep = oneImgTotalSizeInBytes_32align;
        }
    }

    // !!NOTES: bufPlanes, imgFormat should be maintained as original format, ref: Camera3ImageStreamInfo.cpp
    imgBufferInfo.bufPlanes = bufPlanes;
    imgBufferInfo.imgFormat = imgFormat;

    imgBufferInfo.imgWidth  = rStream.v3_2.width;
    imgBufferInfo.imgHeight = rStream.v3_2.height;

    MINT sensorId = findStreamSensorId(mCommonInfo->mInstanceId, rStream, imgBufferInfo);

    AppImageStreamInfo::CreationInfo creationInfo =
    {
        .mStreamName        = s8StreamName,
        .mImgFormat         = allocImgFormat,                 /* alloc stage, TBD if it's YUV format for batch mode SMVR */
        .mOriImgFormat      = (pStreamInfo.get())? pStreamInfo->getOriImgFormat() : formatToAllocate,
        .mRealAllocFormat   = grallocStaticInfo.format,
        .mStream            = rStream,
        .mHalStream         = rOutStream,
        .mImageBufferInfo   = imgBufferInfo,
        .mSensorId          = sensorId,
        .mSecureInfo        = SecureInfo{
            isSecureFlow ? SecType::mem_protected : SecType::mem_normal }
    };
    /* alloc stage, TBD if it's YUV format for batch mode SMVR */
    creationInfo.mvbufPlanes = allocBufPlanes;

    //fill in the secure info if and only if it's a secure camera device
    // debug message
    MY_LOGD("rStream.v_32.usage(0x%#" PRIx64 ") \n"
            "\t rStream-Info(sid=0x%x, format=0x%x, usage=0x%#" PRIx64 ", StreamType::OUTPUT=0x%x, streamType=0x%x) \n"
            "\t grallocStaticInfo(format=0x%x) \n"
            "\t HalStream-info(producerUsage= %#" PRIx64 ",  consumerUsage= %#" PRIx64 ", overrideFormat=0x%x ) \n"
            "\t Blob-info(imgFmt=0x%x, allocImgFmt=0x%x, allocBufPlanes(size=%zu, sizeInBytes=%zu, rowStrideInBytes=%zu) \n"
            "\t Misc-info(usageForAllocator=%#" PRIx64 ", GRALLOC1_USAGE_PROT=%#" PRIx64 ") GRALLOC1_USAGE_SECURE_CAMERA=%#" PRIx64 ")\n"
            "\t creationInfo(secureInfo=0x%x)\n"
            , rStream.v3_2.usage
            , rStream.v3_2.id, rStream.v3_2.format, rStream.v3_2.usage
            , StreamType::OUTPUT, rStream.v3_2.streamType
            , static_cast<PixelFormat>(grallocStaticInfo.format)
            , rOutStream.v3_3.v3_2.producerUsage
            , rOutStream.v3_3.v3_2.consumerUsage
            , rOutStream.v3_3.v3_2.overrideFormat
            , imgFormat, allocImgFormat, allocBufPlanes.count, allocBufPlanes.planes[0].sizeInBytes
            , allocBufPlanes.planes[0].rowStrideInBytes
            , usageForAllocator, GRALLOC1_USAGE_PROT, GRALLOC1_USAGE_SECURE_CAMERA
            , toLiteral(creationInfo.mSecureInfo.type)
            );

    AppImageStreamInfo* pStream = new AppImageStreamInfo(creationInfo);
    return pStream;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
createMetaStreamInfo(
    char const* metaName,
    StreamId_T  suggestedStreamId,
    MINT        physicalId
)   const -> AppMetaStreamInfo*
{
    return new AppMetaStreamInfo(
        metaName,
        suggestedStreamId,
        eSTREAMTYPE_META_IN,
        0,
        0,
        physicalId
    );
}

