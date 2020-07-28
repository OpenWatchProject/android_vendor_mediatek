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
 * MediaTek Inc. (C) 2017. All rights reserved.
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
#define __DEBUG // enable debug
// #define __SCOPE_TIMER // enable log of scope timer
// #define __DUMP_STREAM_INFO // enable dump stream info for debug
// #define __DUMP_PIPELINE_CONFIG // enable dump stream info for debug
// #define __DUMP_USAGE // enable pipeline usage for debug

#define LOG_TAG "MtkCam/CommonCapturePipeline"
static const char* __CALLERNAME__ = LOG_TAG;

// AOSP
#include <utils/String8.h>
#include <cutils/properties.h>
#include <utils/KeyedVector.h>

// STL
#include <string.h>
#include <chrono>
#include <future>
#include <cassert> // assert

// MTKCAM
#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Trace.h>
#include <mtkcam/utils/hw/HwInfoHelper.h>
#include <mtkcam/middleware/v1/IParamsManager.h>
#include <mtkcam/middleware/v1/IParamsManagerV3.h>
#include <mtkcam/aaa/IIspMgr.h>
#include "CommonCapturePipeline.h"

using namespace android;
using namespace NSCam;
using namespace NSCam::Utils;
using namespace NSCam::v1;
using namespace NSCam::v1::NSLegacyPipeline;
using namespace NSCamHW;
using namespace NS3Av3;

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#define CHECK_OBJECT(x)  do{                                    \
    if (x == nullptr) { MY_LOGE("Null %s Object", #x); return;} \
} while(0)

#define CHECK_OBJECT_RETURN(x, error)  do{                                    \
    if (x == nullptr) { MY_LOGE("Null %s Object", #x); return error;} \
} while(0)

// FUNCTION_SCOPE
#ifdef __DEBUG
#define FUNCTION_DONE       CAM_LOGD("[%s] done", __FUNCTION__);
#define FUNCTION_SCOPE      auto __scope_logger__ = create_scope_logger(" ", __FUNCTION__)
#define LOG_SCOPE(log)      auto __scope_logger__ = create_scope_logger(log, __FUNCTION__)
#include <memory>
#include <functional>
static std::shared_ptr<char> create_scope_logger(const char* log, const char* functionName)
{
    char* pText = const_cast<char*>(functionName);
    CAM_LOGD("[%s] %s + ", pText, log);
    return std::shared_ptr<char>(pText, [log](char* p){ CAM_LOGD("[%s] %s -", p, log); });
}
#else
#define FUNCTION_DONE           do{}while(0)
#define FUNCTION_SCOPE          do{}while(0)
#define LOG_SCOPE               do{}while(0)
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

#define DEFAULT_MIN_RAW_FRAME_NUM       1 // for non-zsd
#define DEFAULT_MAX_RAW_FRAME_NUM       6 // for non-zsd
#define DEFAULT_MIN_YUV_FRAME_NUM       3
#define DEFAULT_MAX_YUV_FRAME_NUM       8
#define DEFAULT_MIN_JPEG_FRAME_NUM      1
#define DEFAULT_MAX_JPEG_FRAME_NUM      3

/******************************************************************************
 *
 ******************************************************************************/
Mutex gMapLock;
DefaultKeyedVector< MINT32, sp<ICommonCapturePipeline> > gPipelineMap; //(openId, pipeline)
DefaultKeyedVector< MINT32, String8 > gUserMap; // (openId, userName)

Mutex gCommonPipelineLock;

/******************************************************************************
 *
 ******************************************************************************/
sp<ICommonCapturePipeline>
ICommonCapturePipeline::
createCommonCapturePipeline(
    MINT32 openId,
    sp<IParamsManagerV3> pParamsManagerV3
)
{
    CAM_TRACE_CALL();
    FUNCTION_SCOPE;

    PipelineConfig config;
    if (getDefaultConfig(openId, pParamsManagerV3, config)){
        MY_LOGW("query default config for openId(%d) failed!", openId);
    }

    return queryCommonCapturePipeline(config);
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
ICommonCapturePipeline::
removeCommonCapturePipeline(
    MINT32 openId
)
{
    Mutex::Autolock _l(gCommonPipelineLock);
    return removeCommonCapturePipelineNoLock(openId);
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
ICommonCapturePipeline::
removeCommonCapturePipelineNoLock(
    MINT32 openId
)
{
    CAM_TRACE_CALL();
    FUNCTION_SCOPE;

    sp<ICommonCapturePipeline> pCCPipeline = nullptr;
    pCCPipeline = getPipelineFromMap(openId);
    dumpUsage();

    if (pCCPipeline.get()) {
        MY_LOGD("wait for previous capture behavior(openId:%d)", openId);
        pCCPipeline->waitUntilDrained();
        pCCPipeline->flush(MTRUE);
        MY_LOGD("previous capture behavior done (openId:%d)", openId);
    }

    removePipelineFromMap(openId);
    removeUserFromMap(openId);

    dumpUsage();
    //
    FUNCTION_DONE;
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
ICommonCapturePipeline::
getDefaultConfig
(
    MINT32 openId,
    sp<IParamsManagerV3> pParamsManagerV3,
    PipelineConfig &config
)
{
    CAM_TRACE_CALL();
    FUNCTION_SCOPE;

    sp<IParamsManager> pParamsMgr = pParamsManagerV3->getParamsMgr();
    CHECK_OBJECT_RETURN(pParamsMgr, UNKNOWN_ERROR);

    HwInfoHelper helper(openId);
    if( ! helper.updateInfos() ) {
        MY_LOGE("cannot properly update infos");
        return UNKNOWN_ERROR;
    }

    config.userName = LOG_TAG;
    config.openId = openId;

    // isZsdMode
    String8 s8AppMode = PARAMSMANAGER_MAP_INST(eMapAppMode)->stringFor(pParamsMgr->getHalAppMode());
    if (s8AppMode == MtkCameraParameters::APP_MODE_NAME_MTK_ZSD ||
        s8AppMode == MtkCameraParameters::APP_MODE_NAME_MTK_STEREO) {
        config.isZsdMode = MTRUE;
    }
    else {
        config.isZsdMode = MFALSE;
    }

    wp<StreamBufferProvider> pConsumer = nullptr;
    sp<StreamBufferProvider> pProvider = nullptr;
    if (config.isZsdMode) {
        MY_LOGD("Query Consumer OpenID(%d) StreamID(%llx)", openId, eSTREAMID_IMAGE_PIPE_RAW_OPAQUE);
        pConsumer = IResourceContainer::getInstance(openId)->queryConsumer(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE);
        pProvider = pConsumer.promote();
        CHECK_OBJECT_RETURN(pProvider, UNKNOWN_ERROR);
    }

    MUINT32 sensorMode = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;

    MSize sensorSize, postviewSize, pictureSize, thumbnailSize, jpegSize;
    MINT32 sensorFps;
    MUINT32 pixelMode;

    if(! helper.getSensorSize(sensorMode, sensorSize) ||
       ! helper.getSensorFps(sensorMode, sensorFps)   ||
       ! helper.queryPixelMode(sensorMode, sensorFps, pixelMode) ){
        MY_LOGE("cannot get params about sensor");
        return UNKNOWN_ERROR;
    }

    MBOOL bEnableLtm = MFALSE;
#ifdef MTKCAM_LTM_SUPPORT
    if(MTKCAM_LTM_SUPPORT == 0) {
        bEnableLtm = MFALSE;
    }
    else if( helper.isYuv() ) {
        MY_LOGD("yuv sensor disable lcso");
        bEnableLtm = MFALSE;
    }
    bEnableLtm = MTRUE;
#endif

    MINT32 bitDepth = 10;
    helper.getRecommendRawBitDepth(bitDepth);

    // convert rotation to transform
    MUINT32 transform = 0;
    MINT32 rotation = pParamsMgr->getInt(CameraParameters::KEY_ROTATION);
    switch(rotation)
    {
        case 0:
            transform = 0;
            break;
        case 90:
            transform = eTransform_ROT_90;
            break;
        case 180:
            transform = eTransform_ROT_180;
            break;
        case 270:
            transform = eTransform_ROT_270;
            break;
        default:
            break;
    }
    MY_LOGD("rotation:%d, transform:%d", rotation, transform);
    //
    int postviewClientFormat    = MtkCameraParameters::queryImageFormat(pParamsMgr->getStr(MtkCameraParameters::KEY_POST_VIEW_FMT));
    // if postview YUV format is unknown, set NV21 as default
    if (postviewClientFormat == eImgFmt_UNKNOWN) {
        postviewClientFormat = eImgFmt_NV21;
    }
    //
    MBOOL bEnablePostview = (postviewClientFormat != eImgFmt_UNKNOWN) ? MTRUE : MFALSE;
    MINT postviewFmt = static_cast<EImageFormat>(postviewClientFormat);
    pParamsMgr->getPreviewSize(&postviewSize.w, &postviewSize.h);
    if (sensorSize.w != 0 && sensorSize.h != 0) {
        pictureSize = sensorSize;
    }
    else {
        pParamsMgr->getPictureSize(&pictureSize.w, &pictureSize.h);
    }

    MINT yuvfmt_main      = eImgFmt_I422;
    // thumbnail
    MINT yuvfmt_thumbnail = eImgFmt_YUY2;
    thumbnailSize.w = pParamsMgr->getInt(CameraParameters::KEY_JPEG_THUMBNAIL_WIDTH);
    thumbnailSize.h = pParamsMgr->getInt(CameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT);

    jpegSize = (transform & eTransform_ROT_90) ?
        MSize(pictureSize.h, pictureSize.w):
        MSize(pictureSize.w, pictureSize.h);

    // provide sp<IImageStreamInfo>
    if(config.isZsdMode) {
        config.pInfo_ResizedRaw = nullptr;
        Vector<sp<IImageStreamInfo>> rawInputInfos;
        pProvider->querySelector()->queryCollectImageStreamInfo(rawInputInfos, MTRUE);
        for(size_t i = 0; i < rawInputInfos.size() ; i++)
        {
            if(rawInputInfos[i]->getStreamId() == eSTREAMID_IMAGE_PIPE_RAW_LCSO) {
                config.pInfo_LcsoRaw = rawInputInfos[i];
                config.LPBConfigParams.enableLCS = MTRUE;
            }
            else if(rawInputInfos[i]->getStreamId() == eSTREAMID_IMAGE_PIPE_RAW_OPAQUE) {
                config.pInfo_FullRaw = rawInputInfos[i];
            }
            else if(rawInputInfos[i]->getStreamId() == eSTREAMID_IMAGE_PIPE_RAW_RESIZER) {
                config.pInfo_ResizedRaw = rawInputInfos[i];
            }
        }
    }
    else {
        // Fullsize Raw
        {
            MSize size = sensorSize;
            MINT format;
            size_t stride;
            MUINT const usage = 0; //not necessary here
            if( ! helper.getImgoFmt(bitDepth, format) ||
                ! helper.alignPass1HwLimitation(pixelMode, format, true, size, stride) )
            {
                MY_LOGE("wrong params about imgo");
                return UNKNOWN_ERROR;
            }
            //
            sp<IImageStreamInfo>
                pStreamInfo = createRawImageStreamInfo(
                        "Common:Fullraw",
                        eSTREAMID_IMAGE_PIPE_RAW_OPAQUE,
                        eSTREAMTYPE_IMAGE_INOUT,
                        DEFAULT_MAX_RAW_FRAME_NUM,
                        DEFAULT_MIN_RAW_FRAME_NUM,
                        usage, format, size, stride
                        );
            if( pStreamInfo == nullptr ) {
                return MFALSE;
            }
            config.pInfo_FullRaw = pStreamInfo;
        }

        // Resized raw (for disable frontal binning use-case)
        if (helper.isRaw() && !!postviewSize)
        {
            MSize size;
            MINT format;
            size_t stride;
            MUINT const usage = eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE;

            if(!helper.getRrzoFmt(bitDepth, format) ||
               !helper.alignRrzoHwLimitation(postviewSize, sensorSize, size) ||
               !helper.alignPass1HwLimitation(pixelMode, format, false, size, stride))
            {
                MY_LOGE("wrong parameter for RRZO");
                return UNKNOWN_ERROR;
            }

            sp<IImageStreamInfo>
                pStreamInfo = createRawImageStreamInfo(
                    "Common:ResizedRaw",
                    eSTREAMID_IMAGE_PIPE_RAW_RESIZER,
                    eSTREAMTYPE_IMAGE_INOUT,
                    DEFAULT_MAX_RAW_FRAME_NUM,
                    DEFAULT_MIN_RAW_FRAME_NUM,
                    usage, format, size, stride);

            if( pStreamInfo == nullptr ) {
                MY_LOGE("create ImageStreamInfo ResizedRaw failed");
                return UNKNOWN_ERROR;
            }
            MY_LOGD("create ImageStreamInfo ResizedRaw done");
            config.pInfo_ResizedRaw = pStreamInfo;
        }

        // Lcso Raw
        if(bEnableLtm)
        {
            NS3Av3::LCSO_Param lcsoParam;
            if ( auto pIspMgr = MAKE_IspMgr() ) {
                pIspMgr->queryLCSOParams(lcsoParam);
            }

            MUINT const usage = eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE; //not necessary here
            //
            sp<IImageStreamInfo>
                pStreamInfo = createRawImageStreamInfo(
                        "Common:LCSraw",
                        eSTREAMID_IMAGE_PIPE_RAW_LCSO,
                        eSTREAMTYPE_IMAGE_INOUT,
                        DEFAULT_MAX_RAW_FRAME_NUM,
                        DEFAULT_MIN_RAW_FRAME_NUM,
                        usage, lcsoParam.format, lcsoParam.size, lcsoParam.stride
                        );
            if( pStreamInfo == nullptr ) {
                MY_LOGE("create ImageStreamInfo LcsoRaw failed");
                return UNKNOWN_ERROR;
            }
            MY_LOGD("create ImageStreamInfo LcsoRaw done");
            config.pInfo_LcsoRaw = pStreamInfo;
        }
    }

    // postview YUV
    if (bEnablePostview)
    {
        MSize size        = postviewSize;
        MINT format       = postviewFmt;
        MUINT const usage = 0; //not necessary here
        MUINT32 transform = 0;
        MBOOL isReusable = MTRUE;
        sp<IImageStreamInfo>
            pStreamInfo = createImageStreamInfo(
                    "Common:Postview",
                    eSTREAMID_IMAGE_PIPE_YUV_00,
                    eSTREAMTYPE_IMAGE_INOUT,
                    DEFAULT_MAX_YUV_FRAME_NUM,
                    DEFAULT_MIN_YUV_FRAME_NUM,
                    usage, format, size, transform,
                    MSize(1,1),
                    isReusable
                    );
        if( pStreamInfo == nullptr ) {
            MY_LOGE("create ImageStreamInfo Postview Yuv failed");
            return UNKNOWN_ERROR;
        }
        MY_LOGD("create ImageStreamInfo Postview Yuv done");
        config.pInfo_YuvPostview = pStreamInfo;
    }

    // Yuv
    {
        MSize size        = jpegSize;
        MINT format       = yuvfmt_main;
        MUINT const usage = 0; //not necessary here
        MUINT32 transform = transform;
        MBOOL isReusable = MTRUE;
        sp<IImageStreamInfo>
            pStreamInfo = createImageStreamInfo(
                    "Common:MainYuv",
                    eSTREAMID_IMAGE_PIPE_YUV_JPEG,
                    eSTREAMTYPE_IMAGE_INOUT,
                    DEFAULT_MAX_YUV_FRAME_NUM,
                    DEFAULT_MIN_YUV_FRAME_NUM,
                    usage, format, size, transform,
                    MSize(16, 16),
                    isReusable
                    );
        if( pStreamInfo == nullptr ) {
            MY_LOGE("create ImageStreamInfo Main Yuv failed");
            return UNKNOWN_ERROR;
        }
        MY_LOGD("create ImageStreamInfo Main Yuv done");
        config.pInfo_Yuv = pStreamInfo;
    }
    //
    // Thumbnail Yuv
    {
        MSize size        = thumbnailSize;
        MINT format       = yuvfmt_thumbnail;
        MUINT const usage = 0; //not necessary here
        MUINT32 transform = 0;
        MBOOL isReusable = MTRUE;
        sp<IImageStreamInfo>
            pStreamInfo = createImageStreamInfo(
                    "Common:ThumbnailYuv",
                    eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL,
                    eSTREAMTYPE_IMAGE_INOUT,
                    DEFAULT_MAX_YUV_FRAME_NUM,
                    DEFAULT_MIN_YUV_FRAME_NUM,
                    usage, format, size, transform,
                    MSize(1,1),
                    isReusable
                    );
        if( pStreamInfo == nullptr ) {
            MY_LOGE("create ImageStreamInfo Thumbnail Yuv failed");
            return UNKNOWN_ERROR;
        }
        MY_LOGD("create ImageStreamInfo Thumbnail Yuv done");
        config.pInfo_YuvThumbnail = pStreamInfo;
    }
    //
    // Jpeg
    {
        MSize size        = jpegSize;
        MINT format       = eImgFmt_BLOB;
        MUINT const usage = 0; //not necessary here
        MUINT32 transform = 0;
        MBOOL isReusable = MTRUE;
        sp<IImageStreamInfo>
            pStreamInfo = createImageStreamInfo(
                    "Common:Jpeg",
                    eSTREAMID_IMAGE_JPEG,
                    eSTREAMTYPE_IMAGE_INOUT,
                    DEFAULT_MAX_JPEG_FRAME_NUM,
                    DEFAULT_MIN_JPEG_FRAME_NUM,
                    usage, format, size, transform,
                    MSize(1,1),
                    isReusable
                    );
        if( pStreamInfo == nullptr ) {
            MY_LOGE("create ImageStreamInfo Jpeg failed");
            return UNKNOWN_ERROR;
        }
        MY_LOGD("create ImageStreamInfo Jpeg done");
        config.pInfo_Jpeg = pStreamInfo;
    }


    // LegacyPipelineBuilder::ConfigParams
    config.LPBConfigParams.mode = LegacyPipelineMode_T::PipelineMode_Capture;
    config.LPBConfigParams.enableEIS = MFALSE;
    config.LPBConfigParams.enableLCS = bEnableLtm;
    //TODO: check this is compatible for all shot user
    config.LPBConfigParams.pluginUser = plugin::CALLER_SHOT_SMART;
    if (helper.getDualPDAFSupported(sensorMode)) {
        config.LPBConfigParams.enableDualPD = MTRUE;
    }
    //
    MUINT32 pipeBit;
    MINT ImgFmt = eImgFmt_BAYER12;
    if (helper.getLpModeSupportBitDepthFormat(ImgFmt, pipeBit)) {
        using namespace NSCam::NSIoPipe::NSCamIOPipe;
        if(pipeBit & CAM_Pipeline_14BITS) {
            config.LPBConfigParams.pipeBit = CAM_Pipeline_14BITS;
        }
        else {
            config.LPBConfigParams.pipeBit = CAM_Pipeline_12BITS;
        }
    }
    //
    // no need to provide pCallbackListener (this will be register by shot developer during capture)
    config.pCallbackListener = nullptr;

    if (!config.isZsdMode) { // non-ZSD mode
        // query VHDR sensor mode
        MUINT32 vhdrMode = SENSOR_VHDR_MODE_NONE;
        // query from feature table to decide whether
        // single-frame HDR or multi-frame HDR capture is supported
        bool isSingleFrameCaptureHDR = pParamsMgr->getSingleFrameCaptureHDR();
        // query current VHDR mode
        uint32_t __vhdrMode = pParamsMgr->getVHdr();
        if (isSingleFrameCaptureHDR) {
            switch (__vhdrMode) {
            case SENSOR_VHDR_MODE_ZVHDR:
                vhdrMode = SENSOR_VHDR_MODE_ZVHDR;
                break;
            case SENSOR_VHDR_MODE_IVHDR:
                vhdrMode = SENSOR_VHDR_MODE_IVHDR;
                break;
            default:;
            }
        }

        // provide sensorParam (PipelineSensorParam,
        config.sensorParam.mode = sensorMode;
        config.sensorParam.rawType = 0x0000;
        config.sensorParam.size = sensorSize;
        config.sensorParam.fps = (MUINT)sensorFps;
        config.sensorParam.pixelMode = pixelMode;
        config.sensorParam.vhdrMode = vhdrMode;

        // provide IScenarioControl::ControlParam
        config.scenarioControlParam.scenario   = IScenarioControl::Scenario_Capture;
        config.scenarioControlParam.sensorSize = sensorSize;
        config.scenarioControlParam.sensorFps  = sensorFps;
        if(config.LPBConfigParams.enableDualPD) {
            FEATURE_CFG_ENABLE_MASK(config.scenarioControlParam.featureFlag, IScenarioControl::FEATURE_DUAL_PD);
        }
    }

    FUNCTION_DONE;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
sp<ICommonCapturePipeline>
ICommonCapturePipeline::
queryCommonCapturePipeline(
    PipelineConfig &config
)
{
    CAM_TRACE_CALL();
    FUNCTION_SCOPE;

    Mutex::Autolock _l(gCommonPipelineLock);
    dumpUsage();

    sp<ICommonCapturePipeline> pCCPipeline = nullptr;
    MINT32 openId = config.openId;
    String8 userName = config.userName;

    pCCPipeline = getPipelineFromMap(openId);

    // Check pipeline status and config setting
    if ( pCCPipeline.get() ) { // common capture pipeline has been created
        if ( CC_UNLIKELY(pCCPipeline->isNeedToReconstruct(config)) ) { //Pipeline config is changed!
            removeCommonCapturePipelineNoLock(openId);
            pCCPipeline = nullptr;

            MY_LOGI("create a new capture pipeline for openId(%d), due to pipeline setting incompatible", openId);
            pCCPipeline = new CommonCapturePipeline(config);
        }
        else { // Pipeline config is the same, no need to reconstruct it.
            MY_LOGI("reconfig pipeline, already have pipeline with open id %d", openId);
            pCCPipeline->reconfigPipeline(config);
        }
    }
    else { // common capture pipeline not exist
        MY_LOGI("create a new capture pipeline for openId(%d), due to pipeline exist", openId);
        pCCPipeline = new CommonCapturePipeline(config);
    }

    if( pCCPipeline.get() &&
        pCCPipeline->isPipelineExist() ){
        addPipelineToMap(openId, pCCPipeline);
        if (strcmp(userName.string(), LOG_TAG)) {
            addUserToMap(openId, userName);
        }
        MY_LOGD("create/get pipeline done");
    }
    else{
        pCCPipeline = nullptr;
        MY_LOGE("cannot create/get pipeline properly");
    }

    dumpUsage();
    return pCCPipeline;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
ICommonCapturePipeline::
returnCommonCapturePipeline(
    MINT32 openId
)
{
    CAM_TRACE_CALL();
    FUNCTION_SCOPE;
    MERROR ret = OK;

    Mutex::Autolock _l(gCommonPipelineLock);

    sp<ICommonCapturePipeline> pCCPipeline = nullptr;
    pCCPipeline = getPipelineFromMap(openId);

    if (pCCPipeline.get()) {
        //pCCPipeline->waitUntilDrained();
        //pCCPipeline->flush();
        //pCCPipeline->unregistAndReturnBufferToPool();
    }

    removeUserFromMap(openId);
    dumpUsage();

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
sp<ICommonCapturePipeline>
ICommonCapturePipeline::
getPipelineFromMap
(
    MINT32 openId
)
{
    Mutex::Autolock _l(gMapLock);

    sp<ICommonCapturePipeline> pCCPipeline = nullptr;

    if (gPipelineMap.size() > 0) {
        ssize_t index = gPipelineMap.indexOfKey(openId);
        if ( index >= 0 ){
            pCCPipeline = gPipelineMap.valueAt(index);
        }
    }

    if (pCCPipeline.get() == nullptr) {
        MY_LOGD("cannot find common capture pipeline (openId:%d)", openId);
    }

    return pCCPipeline;
}

/******************************************************************************
 *
 ******************************************************************************/
void
ICommonCapturePipeline::
addPipelineToMap
(
    MINT32 openId,
    sp<ICommonCapturePipeline> pCCPipeline
)
{
    Mutex::Autolock _l(gMapLock);
    gPipelineMap.add(openId, pCCPipeline);
    MY_LOGD("add (openId:%d, pipeline:0x%X) to map", openId, pCCPipeline.get());

    return;
}

/******************************************************************************
 *
 ******************************************************************************/
void
ICommonCapturePipeline::
addUserToMap
(
    MINT32 openId,
    String8 userName
)
{
    Mutex::Autolock _l(gMapLock);
    gUserMap.add(openId, userName);
    MY_LOGD("add (openId:%d, user:%s) to map", openId, userName.string());

    return;
}

/******************************************************************************
 *
 ******************************************************************************/
void
ICommonCapturePipeline::
removePipelineFromMap
(
    MINT32 openId
)
{
    Mutex::Autolock _l(gMapLock);

    if (gPipelineMap.size() > 0) {
        ssize_t index = gPipelineMap.indexOfKey(openId);
        if (index >= 0) {
            gPipelineMap.removeItemsAt(index);
        }
    }

    return;
}

/******************************************************************************
 *
 ******************************************************************************/
void
ICommonCapturePipeline::
removeUserFromMap
(
    MINT32 openId
)
{
    Mutex::Autolock _l(gMapLock);

    if (gUserMap.size() > 0) {
        ssize_t index = gUserMap.indexOfKey(openId);
        if (index >= 0) {
            String8 userName = gUserMap.valueAt(index);
            MY_LOGD("remove (openId:%d, user:%s) from map", openId, userName.string());
            gUserMap.removeItemsAt(index);
        }
    }

    return;
}

/******************************************************************************
 *
 ******************************************************************************/
void
ICommonCapturePipeline::
dumpUsage()
{
#ifdef __DUMP_USAGE
    Mutex::Autolock _l(gMapLock);

    size_t pipelineNum = gPipelineMap.size();
    size_t userNum = gUserMap.size();
    MY_LOGD("%zu user use %d pipeline", userNum, pipelineNum);
    for (size_t i = 0; i < userNum; i++) {
        MINT32 openId = gUserMap.keyAt(i);
        String8 userName = gUserMap.valueAt(i);
        MY_LOGD("(userName, openId) = (%s, %d)", userName.string(), openId);
    }
#endif
    return;
}

/******************************************************************************
 *
 ******************************************************************************/
void
ICommonCapturePipeline::
dumpPipelineConfig
(
    PipelineConfig &config
)
{
#ifdef __DUMP_PIPELINE_CONFIG // enable dump stream info for d
    MY_LOGD("=================================================================");
    MY_LOGD("userName:%s", config.userName.string());
    MY_LOGD("openId:%d", config.openId);
    MY_LOGD("isZsdMode:%d", config.isZsdMode);

    MY_LOGD("-----------------PipelineSensorParam-----------------");
    MY_LOGD("mode:%d", config.sensorParam.mode);
    MY_LOGD("rawType:%d", config.sensorParam.rawType);
    MY_LOGD("size:(%d,%d)", config.sensorParam.size.w,
                            config.sensorParam.size.h);
    MY_LOGD("fps:%d", config.sensorParam.fps);
    MY_LOGD("pixelMode:%d", config.sensorParam.pixelMode);
    MY_LOGD("vhdrMode:%d", config.sensorParam.vhdrMode);

    MY_LOGD("-----------------scenarioControlParam-----------------");
    MY_LOGD("scenario:%d", config.scenarioControlParam.scenario);
    MY_LOGD("sensorFps:%d", config.scenarioControlParam.sensorFps);
    MY_LOGD("featureFlag:%d", config.scenarioControlParam.featureFlag);
    MY_LOGD("enableBWCControl:%d", config.scenarioControlParam.enableBWCControl);
    MY_LOGD("videoSize:(%d.%d)", config.scenarioControlParam.videoSize.w,
                                 config.scenarioControlParam.videoSize.h);
    MY_LOGD("camMode:%d", config.scenarioControlParam.camMode);
    MY_LOGD("supportCustomOption:%d", config.scenarioControlParam.supportCustomOption);

    MY_LOGD("-------------------ImageStreamInfo-------------------");
    dumpImageStreamInfo(config.pInfo_FullRaw);
    dumpImageStreamInfo(config.pInfo_ResizedRaw);
    dumpImageStreamInfo(config.pInfo_LcsoRaw);
    dumpImageStreamInfo(config.pInfo_Yuv);
    dumpImageStreamInfo(config.pInfo_YuvPostview);
    dumpImageStreamInfo(config.pInfo_YuvThumbnail);
    dumpImageStreamInfo(config.pInfo_Jpeg);

    MY_LOGD("------------------pCallbackListener------------------");
    MY_LOGD("pCallbackListener:0x%X", config.pCallbackListener);

    MY_LOGD("-------------------LPBConfigParams-------------------");
    MY_LOGD("mode:%d", config.LPBConfigParams.mode);
    MY_LOGD("enableEIS:%d", config.LPBConfigParams.enableEIS);
    MY_LOGD("enableLCS:%d", config.LPBConfigParams.enableLCS);
    MY_LOGD("enableRSS:%d", config.LPBConfigParams.enableRSS);
    MY_LOGD("enableDualPD:%d", config.LPBConfigParams.enableDualPD);
    MY_LOGD("eisMode:%d", config.LPBConfigParams.eisMode);
    MY_LOGD("enableBurstNum:%d", config.LPBConfigParams.enableBurstNum);
    MY_LOGD("optimizedCaptureFlow:%d", config.LPBConfigParams.optimizedCaptureFlow);
    MY_LOGD("disableFrontalBinning:%d", config.LPBConfigParams.disableFrontalBinning);
    MY_LOGD("enableUNI:%d", config.LPBConfigParams.enableUNI);
    MY_LOGD("dualcamMode:%d", config.LPBConfigParams.dualcamMode);
    MY_LOGD("streamingSize:(%d,%d)", config.LPBConfigParams.streamingSize.w,
                                     config.LPBConfigParams.streamingSize.h);
    MY_LOGD("receiveMode:%d", config.LPBConfigParams.receiveMode);
    MY_LOGD("pluginUser:0x%X", config.LPBConfigParams.pluginUser);
    MY_LOGD("disableHLR:%d", config.LPBConfigParams.disableHLR);
    MY_LOGD("pipeBit:%d", config.LPBConfigParams.pipeBit);
    MY_LOGD("initRequest:%d", config.LPBConfigParams.initRequest);
    MY_LOGD("support4cellsensor:%d", config.LPBConfigParams.support4cellsensor);
    MY_LOGD("enableRrzoCapture:%d", config.LPBConfigParams.enableRrzoCapture);
    MY_LOGD("enableQuadCode:%d", config.LPBConfigParams.enableQuadCode);
    MY_LOGD("=================================================================");
#endif
    return;
}

/******************************************************************************
 *
 ******************************************************************************/
void
ICommonCapturePipeline::
dumpImageStreamInfo
(
    sp<IImageStreamInfo> pInfo,
    const char* pTxt
)
{
#ifdef __DUMP_STREAM_INFO
    if (pInfo.get()) {
        MY_LOGD("-----ImageStreamInfo(%s)-----", pInfo->getStreamName());
        if(pTxt) {
            MY_LOGD("<<< debug code tag: %s >>>", pTxt);
        }
        MY_LOGD("streamId:%#" PRIxPTR "", pInfo->getStreamId());
        MY_LOGD("streamType:%d", pInfo->getStreamType());
        MY_LOGD("maxBufNum:%d", pInfo->getMaxBufNum());
        MY_LOGD("minInitBufNum:%d", pInfo->getMinInitBufNum());
        MY_LOGD("usageForAllocator:0x%X", pInfo->getUsageForAllocator());
        MY_LOGD("imgFormat:0x%X", pInfo->getImgFormat());
        MY_LOGD("imgSize:(%d,%d)", pInfo->getImgSize().w, pInfo->getImgSize().h);
        MY_LOGD("bufPlanes:%zu", pInfo->getBufPlanes().size());
        if (pInfo->getBufPlanes().size() > 0) {
            for (size_t i = 0; i < pInfo->getBufPlanes().size(); i++) {
                MY_LOGD("BufPlanes[%d](sizeInBytes,rowStrideInBytes):(%d,%d)",
                        i,
                        pInfo->getBufPlanes()[i].sizeInBytes, pInfo->getBufPlanes()[i].rowStrideInBytes);
                }
        }
        MY_LOGD("transform:%d", pInfo->getTransform());
    }
    else {
        MY_LOGD("------------------------------");;
        if(pTxt) {
            MY_LOGD("<<< debug code tag: %s >>>", pTxt);
        }
        MY_LOGD("ImageStreamInfo is nullptr, no need to dump");
    }
    MY_LOGD("------------------------------");;
#endif
    return;
}

/******************************************************************************
 *
 ******************************************************************************/
CommonCapturePipeline::
CommonCapturePipeline(PipelineConfig &config)
    : mCurPipelineConfig(config)
    , mpPipeline(nullptr)
    , mpFactory(nullptr)
    , mpCallbackHandler(nullptr)
    , mpImageCallback(nullptr)
    , mpMetadataListener(nullptr)
    , mpMetadataListenerFull(nullptr)
    , mpPool_FullRaw(nullptr)
    , mpPool_ResizedRaw(nullptr)
    , mpPool_LcsoRaw(nullptr)
    , mpPool_Yuv(nullptr)
    , mpPool_YuvPostview(nullptr)
    , mpPool_YuvThumbnail(nullptr)
    , mpPool_Jpeg(nullptr)
    , mDebug(0)
{
    CAM_TRACE_CALL();
    FUNCTION_SCOPE;

    mDebug = ::property_get_int32("debug.camera.commonpipeline.debug", 0);

    MY_LOGD("isZsdMode:%d", config.isZsdMode);

    if (config.pCallbackListener) {
        mpImageCallback = new SImageCallback(config.pCallbackListener, 0);
        CHECK_OBJECT(mpImageCallback);
        mpCallbackHandler = new BufferCallbackHandler(config.openId);
        CHECK_OBJECT(mpCallbackHandler);
        mpCallbackHandler->setImageCallback(mpImageCallback);
    }
    else {
        mpCallbackHandler = nullptr;
        MY_LOGI("config pCallbackListener is nullptr, not ready to create BufferCallbackHandler");
    }

    MUINT32 openId = mCurPipelineConfig.openId;
    String8 userName = mCurPipelineConfig.userName;

    wp<StreamBufferProvider> pConsumer = nullptr;
    sp<StreamBufferProvider> pProvider = nullptr;
    if (config.isZsdMode) {
        MY_LOGD_IF(mDebug, "Query Consumer OpenID(%d) StreamID(%llx)", openId, eSTREAMID_IMAGE_PIPE_RAW_OPAQUE);
        pConsumer = IResourceContainer::getInstance(openId)->queryConsumer(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE);
        pProvider = pConsumer.promote();
        CHECK_OBJECT(pProvider);
    }
    else {
        MY_LOGD_IF(mDebug, "It's non-ZSD mode, no need to query Consumer OpenID(%d) StreamID(%llx)",
                   openId, eSTREAMID_IMAGE_PIPE_RAW_OPAQUE);
    }

    sp<LegacyPipelineBuilder> pBuilder = LegacyPipelineBuilder::createInstance(
                                            openId,
                                            config.userName.string(),
                                            config.LPBConfigParams);
    CHECK_OBJECT(pBuilder);

    mpFactory = StreamBufferProviderFactory::createInstance();
    CHECK_OBJECT(mpFactory);

    Vector<PipelineImageParam> vImageParam;

    MY_LOGD_IF(mDebug, "createPipeline for %s", userName.string());
    if ( mCurPipelineConfig.isZsdMode ) {
        //
        Vector<sp<IImageStreamInfo>> rawInputInfos;
        pProvider->querySelector()->queryCollectImageStreamInfo(rawInputInfos, MTRUE);

        Vector<PipelineImageParam> vImgSrcParams;
        sp<IImageStreamInfo> pStreamInfo = NULL;

        for(size_t i = 0; i < rawInputInfos.size(); i++) {
            pStreamInfo = rawInputInfos[i];
            sp<CallbackBufferPool> pPool = new CallbackBufferPool(pStreamInfo);
            //pPool->addBuffer(pBuf); //TODO: ZSD, SmartShot is not the same
            if (mpCallbackHandler.get()) {
                mpCallbackHandler->setBufferPool(pPool);
            }
            else{
                MY_LOGI("CallbackHandler is nullptr, not ready to setBufferPool for CallbackHandler");
            }
            //
            mpFactory->setImageStreamInfo(pStreamInfo);
            mpFactory->setUsersPool(pPool);
            PipelineImageParam imgParam = {
                .pInfo     = pStreamInfo,
                .pProvider = mpFactory->create(false),
                .usage     = 0
            };
            vImgSrcParams.push_back(imgParam);

            // store buffer pool for pipeline reuse
            if (pStreamInfo->getStreamId() == eSTREAMID_IMAGE_PIPE_RAW_OPAQUE) {
                mpPool_FullRaw = pPool;
            }
            else if (pStreamInfo->getStreamId() == eSTREAMID_IMAGE_PIPE_RAW_RESIZER) {
                mpPool_ResizedRaw = pPool;
            }
            else if (pStreamInfo->getStreamId() == eSTREAMID_IMAGE_PIPE_RAW_LCSO) {
                mpPool_LcsoRaw = pPool;
            }
        }
        //
        if( OK != pBuilder->setSrc(vImgSrcParams) ) {
            MY_LOGE("setSrc failed");
            return;
        }
    }
    else { // non-zsd config
        //
        MY_LOGD_IF(mDebug, "vhdrMode(0x%X)", mCurPipelineConfig.sensorParam.vhdrMode);
        if( OK != pBuilder->setSrc(mCurPipelineConfig.sensorParam) ) {
            MY_LOGE("setSrc failed");
            return;
        }

        //TODO: move to correct timing
        sp<IScenarioControl> pScenarioCtrl = IScenarioControl::create(openId);
        if( pScenarioCtrl.get() == NULL )
        {
            MY_LOGE("get Scenario Control fail");
            return;
        }

        pScenarioCtrl->enterScenario(mCurPipelineConfig.scenarioControlParam);
        pBuilder->setScenarioControl(pScenarioCtrl);

        // fullraw
        if( mCurPipelineConfig.pInfo_FullRaw.get() )
        {
            sp<IImageStreamInfo> pStreamInfo = mCurPipelineConfig.pInfo_FullRaw;
            //
            sp<CallbackBufferPool> pPool = new CallbackBufferPool(pStreamInfo);
            {
                CAM_TRACE_NAME("allocate Buffer");
                pPool->allocateBuffer(
                                  pStreamInfo->getStreamName(),
                                  pStreamInfo->getMaxBufNum(),
                                  pStreamInfo->getMinInitBufNum()
                                  );
            }
            if (mpCallbackHandler.get()) {
                mpCallbackHandler->setBufferPool(pPool);
            }
            else{
                MY_LOGI("CallbackHandler is nullptr, not ready to setBufferPool for CallbackHandler");
            }
            //
            mpFactory->setImageStreamInfo(pStreamInfo);
            mpFactory->setUsersPool(pPool);
            //
            PipelineImageParam imgParam = {
                .pInfo     = pStreamInfo,
                .pProvider = mpFactory->create(false),
                .usage     = 0
            };
            vImageParam.push_back(imgParam);

            // store buffer pool for pipeline reuse
            mpPool_FullRaw = pPool;
        }
        // resized raw
        if( mCurPipelineConfig.pInfo_ResizedRaw.get() )
        {
            sp<IImageStreamInfo> pStreamInfo = mCurPipelineConfig.pInfo_ResizedRaw;
            //
            PipelineImageParam imgParam = {
                .pInfo     = pStreamInfo,
                .pProvider = nullptr,
                .usage     = 0
            };
            vImageParam.push_back(imgParam);
        }
        // resized raw
        if( mCurPipelineConfig.pInfo_LcsoRaw.get() )
        {
            sp<IImageStreamInfo> pStreamInfo = mCurPipelineConfig.pInfo_LcsoRaw;
            //
            PipelineImageParam imgParam = {
                .pInfo     = pStreamInfo,
                .pProvider = nullptr,
                .usage     = 0
            };
            vImageParam.push_back(imgParam);
        }
    }

    //
    if( mCurPipelineConfig.pInfo_Yuv.get() )
    {
        CAM_TRACE_NAME("Set StreamInfo: YUV");
        sp<IImageStreamInfo> pStreamInfo = mCurPipelineConfig.pInfo_Yuv;
        //
        sp<CallbackBufferPool> pPool = new CallbackBufferPool(pStreamInfo);
        {
            CAM_TRACE_NAME("allocate Buffer");
            pPool->allocateBuffer(
                              pStreamInfo->getStreamName(),
                              pStreamInfo->getMaxBufNum(),
                              pStreamInfo->getMinInitBufNum()
                              );
        }
        //
        if (mpCallbackHandler.get()) {
            mpCallbackHandler->setBufferPool(pPool);
        }
        else{
            MY_LOGI("CallbackHandler is nullptr, not ready to setBufferPool for CallbackHandler");
        }
        mpFactory->setImageStreamInfo(pStreamInfo);
        mpFactory->setUsersPool(pPool);
        //
        PipelineImageParam imgParam = {
            .pInfo     = pStreamInfo,
            .pProvider = mpFactory->create(false),
            .usage     = 0
        };
        vImageParam.push_back(imgParam);

        // store buffer pool for pipeline reuse
        mpPool_Yuv = pPool;
    }
    //
    if( mCurPipelineConfig.pInfo_YuvPostview.get() )
    {
        CAM_TRACE_NAME("Set StreamInfo: YUV Postview");
        sp<IImageStreamInfo> pStreamInfo = mCurPipelineConfig.pInfo_YuvPostview;
        //
        sp<CallbackBufferPool> pPool = new CallbackBufferPool(pStreamInfo);
        {
            CAM_TRACE_NAME("allocate Buffer");
            pPool->allocateBuffer(
                              pStreamInfo->getStreamName(),
                              pStreamInfo->getMaxBufNum(),
                              pStreamInfo->getMinInitBufNum()
                              );
        }
        if (mpCallbackHandler.get()) {
            mpCallbackHandler->setBufferPool(pPool);
        }
        else{
            MY_LOGI("CallbackHandler is nullptr, not ready to setBufferPool for CallbackHandler");
        }
        //
        mpFactory->setImageStreamInfo(pStreamInfo);
        mpFactory->setUsersPool(pPool);
        //
        PipelineImageParam imgParam = {
            .pInfo     = pStreamInfo,
            .pProvider = mpFactory->create(false),
            .usage     = 0
        };
        vImageParam.push_back(imgParam);

        // store buffer pool for pipeline reuse
        mpPool_YuvPostview = pPool;
    }
    //
    if( mCurPipelineConfig.pInfo_YuvThumbnail.get() )
    {
        CAM_TRACE_NAME("Set StreamInfo: YUV Thumbnail");
        sp<IImageStreamInfo> pStreamInfo = mCurPipelineConfig.pInfo_YuvThumbnail;
        //
        sp<CallbackBufferPool> pPool = new CallbackBufferPool(pStreamInfo);
        {
            CAM_TRACE_NAME("allocate Buffer");
            pPool->allocateBuffer(
                              pStreamInfo->getStreamName(),
                              pStreamInfo->getMaxBufNum(),
                              pStreamInfo->getMinInitBufNum()
                              );
        }
        if (mpCallbackHandler.get()) {
            mpCallbackHandler->setBufferPool(pPool);
        }
        else{
            MY_LOGI("CallbackHandler is nullptr, not ready to setBufferPool for CallbackHandler");
        }
        //
        mpFactory->setImageStreamInfo(pStreamInfo);
        mpFactory->setUsersPool(pPool);
        //
        PipelineImageParam imgParam = {
            .pInfo     = pStreamInfo,
            .pProvider = mpFactory->create(false),
            .usage     = 0
        };
        vImageParam.push_back(imgParam);

        // store buffer pool for pipeline reuse
        mpPool_YuvThumbnail = pPool;
    }
    //
    if( mCurPipelineConfig.pInfo_Jpeg.get() )
    {
        CAM_TRACE_NAME("Set StreamInfo: JPEG");
        sp<IImageStreamInfo> pStreamInfo = mCurPipelineConfig.pInfo_Jpeg;
        //
        sp<CallbackBufferPool> pPool = new CallbackBufferPool(pStreamInfo);
        {
            CAM_TRACE_NAME("allocate Buffer");
            pPool->allocateBuffer(
                              pStreamInfo->getStreamName(),
                              pStreamInfo->getMaxBufNum(),
                              pStreamInfo->getMinInitBufNum()
                              );
        }
        if (mpCallbackHandler.get()) {
            mpCallbackHandler->setBufferPool(pPool);
        }
        else{
            MY_LOGI("CallbackHandler is nullptr, not ready to setBufferPool for CallbackHandler");
        }
        //
        mpFactory->setImageStreamInfo(pStreamInfo);
        mpFactory->setUsersPool(pPool);
        //
        PipelineImageParam imgParam = {
            .pInfo     = pStreamInfo,
            .pProvider = mpFactory->create(false),
            .usage     = 0
        };
        vImageParam.push_back(imgParam);

        // store buffer pool for pipeline reuse
        mpPool_Jpeg = pPool;
    }
    //
    if( OK != pBuilder->setDst(vImageParam) ) {
        MY_LOGE("setDst failed");
        return;
    }
    //
    mpPipeline = pBuilder->create();
    CHECK_OBJECT(mpPipeline);

    // metadata Listener
    sp<ResultProcessor> pResultProcessor = mpPipeline->getResultProcessor().promote();
    CHECK_OBJECT(pResultProcessor);

    if (config.pCallbackListener) {
        mpMetadataListener = new SMetadataListener(config.pCallbackListener);
        pResultProcessor->registerListener( 0, UINT_MAX, true, mpMetadataListener);

        mpMetadataListenerFull = new SMetadataListener(config.pCallbackListener);
        pResultProcessor->registerListener( 0, UINT_MAX, false, mpMetadataListenerFull);
    }
    else{
        mpMetadataListener = nullptr;
        mpMetadataListenerFull = nullptr;
        MY_LOGI("config pCallbackListener is nullptr, not ready to create MetaDataListener");
    }

    FUNCTION_DONE;
}

/******************************************************************************
 *
 ******************************************************************************/
void
CommonCapturePipeline::
onLastStrongRef(const void* /*id*/)
{
    CAM_TRACE_CALL();
    FUNCTION_SCOPE;
    if (mpPool_FullRaw.get()){
        mpPool_FullRaw->flush();
    }
    if (mpPool_ResizedRaw.get()){
        mpPool_ResizedRaw->flush();
    }
    if (mpPool_LcsoRaw.get()){
        mpPool_LcsoRaw->flush();
    }
    if (mpPool_Yuv.get()){
        mpPool_Yuv->flush();
    }
    if (mpPool_YuvPostview.get()){
        mpPool_YuvPostview->flush();
    }
    if (mpPool_YuvThumbnail.get()){
        mpPool_YuvThumbnail->flush();
    }
    if (mpPool_Jpeg.get()){
        mpPool_Jpeg->flush();
    }
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
CommonCapturePipeline::
reconfigStreamInfo(
    IStreamInfo::StreamId_T streamId,
    PipelineConfig &config
)
{
    CAM_TRACE_CALL();
    FUNCTION_SCOPE;
    MBOOL ret = OK;

    //update buffer pool callback
    if (mpCallbackHandler.get() == nullptr) {
        MY_LOGW("mpCallbackHandler is invalid nullptr");
        ret = UNKNOWN_ERROR;
    }
    else { //reconfig pipeline buffer pool callback
        bool isRaw = false;
        sp<CallbackBufferPool> pPool = nullptr;
        sp<IImageStreamInfo>   pInfo_old = nullptr;
        sp<IImageStreamInfo>   pInfo_new = nullptr;
        switch (streamId) {
            case eSTREAMID_IMAGE_PIPE_RAW_OPAQUE:
                pPool = mpPool_FullRaw;
                pInfo_old = mCurPipelineConfig.pInfo_FullRaw;
                pInfo_new = config.pInfo_FullRaw;
                isRaw = true;
                break;
            case eSTREAMID_IMAGE_PIPE_RAW_RESIZER:
                pPool = mpPool_ResizedRaw;
                pInfo_old = mCurPipelineConfig.pInfo_ResizedRaw;
                pInfo_new = config.pInfo_ResizedRaw;
                isRaw = true;
                break;
            case eSTREAMID_IMAGE_PIPE_RAW_LCSO:
                pPool = mpPool_LcsoRaw;
                pInfo_old = mCurPipelineConfig.pInfo_LcsoRaw;
                pInfo_new = config.pInfo_LcsoRaw;
                isRaw = true;
                break;
            case eSTREAMID_IMAGE_PIPE_YUV_JPEG:
                pPool = mpPool_Yuv;
                pInfo_old = mCurPipelineConfig.pInfo_Yuv;
                pInfo_new = config.pInfo_Yuv;
                isRaw = false;
                break;
            case eSTREAMID_IMAGE_PIPE_YUV_00:
                pPool = mpPool_YuvPostview;
                pInfo_old = mCurPipelineConfig.pInfo_YuvPostview;
                pInfo_new = config.pInfo_YuvPostview;
                isRaw = false;
                break;
            case eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL:
                pPool = mpPool_YuvThumbnail;
                pInfo_old = mCurPipelineConfig.pInfo_YuvThumbnail;
                pInfo_new = config.pInfo_YuvThumbnail;
                isRaw = false;
                break;
            case eSTREAMID_IMAGE_JPEG:
                pPool = mpPool_Jpeg;
                pInfo_old = mCurPipelineConfig.pInfo_Jpeg;
                pInfo_new = config.pInfo_Jpeg;
                isRaw = false;
                break;
            default:
                MY_LOGW("Unknow stream info!!");
                ret = UNKNOWN_ERROR;
                break;
        }

        if (ret != UNKNOWN_ERROR) {
            dumpImageStreamInfo(pInfo_old, "before: Old");
            dumpImageStreamInfo(pInfo_new, "before: New");
            ResultType checkResult = checkAndUpdateImageStreamInfo(pInfo_old, pInfo_new, isRaw);
            dumpImageStreamInfo(pInfo_old, "after: Old");
            dumpImageStreamInfo(pInfo_new, "after: New");

            // update config for reconfig buffer pool
            if (pInfo_new.get() && checkResult != RESULT_UNKNOWN_ERROR) {
                MY_LOGD_IF(mDebug, "update config for streamId(%#" PRIx64 ")", pInfo_new->getStreamId());
                switch (streamId) {
                    case eSTREAMID_IMAGE_PIPE_RAW_OPAQUE:
                        config.pInfo_FullRaw = pInfo_new;
                        break;
                    case eSTREAMID_IMAGE_PIPE_RAW_RESIZER:
                        config.pInfo_ResizedRaw = pInfo_new;
                        break;
                    case eSTREAMID_IMAGE_PIPE_RAW_LCSO:
                        config.pInfo_LcsoRaw = pInfo_new;
                        break;
                    case eSTREAMID_IMAGE_PIPE_YUV_JPEG:
                        config.pInfo_Yuv = pInfo_new;
                        break;
                    case eSTREAMID_IMAGE_PIPE_YUV_00:
                        config.pInfo_YuvPostview = pInfo_new;
                        break;
                    case eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL:
                        config.pInfo_YuvThumbnail = pInfo_new;
                        break;
                    case eSTREAMID_IMAGE_JPEG:
                        config.pInfo_Jpeg = pInfo_new;
                        break;
                    default:
                        MY_LOGW("Unknow stream info!!");
                        ret = UNKNOWN_ERROR;
                        break;
                }

                if (checkResult == RESULT_ADD_BUFFER ||
                    checkResult == RESULT_RELOAD_ALL_BUFFER ||
                    checkResult == RESULT_RECONFIG_STREAM_INFO ){
                    MY_LOGI("pInfo(streamId:0x%llx) is changed!", streamId);

                    // wait previous shot instance done before reconfig buffer pool
                    waitUntilDrained();

                    if (!config.isZsdMode){
                        // non-zsd
                        reconfigBufferPool(streamId, config, checkResult);
                        MY_LOGD_IF(mDebug, "reconfigBufferPool for (streamId:0x%llx, isZSDMode:%d, checkResult:%d)",
                                   streamId, config.isZsdMode, checkResult);
                    }
                    else if(streamId != eSTREAMID_IMAGE_PIPE_RAW_OPAQUE  &&
                            streamId != eSTREAMID_IMAGE_PIPE_RAW_RESIZER &&
                            streamId != eSTREAMID_IMAGE_PIPE_RAW_LCSO    ){
                        // zsd & not raw
                        reconfigBufferPool(streamId, config, checkResult);
                        MY_LOGD_IF(mDebug, "reconfigBufferPool for (streamId:0x%llx, isZSDMode:%d, checkResult:%d)",
                                   streamId, config.isZsdMode, checkResult);
                    }
                    else {
                        MY_LOGD_IF(mDebug, "no need to reconfigBufferPool for (streamId:0x%llx, isZSDMode:%d, checkResult:%d)",
                                   streamId, config.isZsdMode, checkResult);
                    }
                }
                else {
                    MY_LOGD("no need to reconfigBufferPool for (streamId:0x%llx, isZSDMode:%d, checkResult:%d)",
                            streamId, config.isZsdMode, checkResult);
                }

                switch (streamId) {
                    case eSTREAMID_IMAGE_PIPE_RAW_OPAQUE:
                        pPool = mpPool_FullRaw;
                        break;
                    case eSTREAMID_IMAGE_PIPE_RAW_RESIZER:
                        pPool = mpPool_ResizedRaw;
                        break;
                    case eSTREAMID_IMAGE_PIPE_RAW_LCSO:
                        pPool = mpPool_LcsoRaw;
                        break;
                    case eSTREAMID_IMAGE_PIPE_YUV_JPEG:
                        pPool = mpPool_Yuv;
                        break;
                    case eSTREAMID_IMAGE_PIPE_YUV_00:
                        pPool = mpPool_YuvPostview;
                        break;
                    case eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL:
                        pPool = mpPool_YuvThumbnail;
                        break;
                    case eSTREAMID_IMAGE_JPEG:
                        pPool = mpPool_Jpeg;
                        break;
                    default:
                        MY_LOGW("Unknow stream info!!");
                        ret = UNKNOWN_ERROR;
                        break;
                }

                if (pInfo_new.get() && pPool.get()) {
                    if (mpCallbackHandler.get()) {
                        mpCallbackHandler->setBufferPool(pPool);
                    }
                    else {
                        MY_LOGW("CallbackHandler is nullptr, not ready to setBufferPool for CallbackHandler");
                    }
                    mpFactory->setImageStreamInfo(pInfo_new);
                    mpFactory->setUsersPool(pPool);
                    mpFactory->create(false);
                    MY_LOGD_IF(mDebug, "reconfig image stream for streamId(%#" PRIx64 ")", pInfo_new->getStreamId());
                    ret = OK;
                }
                else {
                    MY_LOGW("pInfo_new(0x%X) or pPool(0x%X) is nullptr", pInfo_new.get(), pPool.get());
                    ret = UNKNOWN_ERROR;
                }
            }
        }
    }

    MY_LOGD_IF(mDebug, "ret:%d", ret);
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
ICommonCapturePipeline::ResultType
CommonCapturePipeline::
reconfigBufferPool(
    IStreamInfo::StreamId_T streamId,
    PipelineConfig &config,
    ResultType checkResult
)
{
    CAM_TRACE_CALL();
    FUNCTION_SCOPE;

    ResultType ret = RESULT_OK;
    sp<CallbackBufferPool> pPool = nullptr;
    sp<IImageStreamInfo>   pInfo_old = nullptr;
    sp<IImageStreamInfo>   pInfo_new = nullptr;

    switch (streamId) {
        case eSTREAMID_IMAGE_PIPE_RAW_OPAQUE:
            pPool = mpPool_FullRaw;
            pInfo_old = mCurPipelineConfig.pInfo_FullRaw;
            pInfo_new = config.pInfo_FullRaw;
            break;
        case eSTREAMID_IMAGE_PIPE_RAW_RESIZER:
            pPool = mpPool_ResizedRaw;
            pInfo_old = mCurPipelineConfig.pInfo_ResizedRaw;
            pInfo_new = config.pInfo_ResizedRaw;
            break;
        case eSTREAMID_IMAGE_PIPE_RAW_LCSO:
            pPool = mpPool_LcsoRaw;
            pInfo_old = mCurPipelineConfig.pInfo_LcsoRaw;
            pInfo_new = config.pInfo_LcsoRaw;
            break;
        case eSTREAMID_IMAGE_PIPE_YUV_JPEG:
            pPool = mpPool_Yuv;
            pInfo_old = mCurPipelineConfig.pInfo_Yuv;
            pInfo_new = config.pInfo_Yuv;
            break;
        case eSTREAMID_IMAGE_PIPE_YUV_00:
            pPool = mpPool_YuvPostview;
            pInfo_old = mCurPipelineConfig.pInfo_YuvPostview;
            pInfo_new = config.pInfo_YuvPostview;
            break;
        case eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL:
            pPool = mpPool_YuvThumbnail;
            pInfo_old = mCurPipelineConfig.pInfo_YuvThumbnail;
            pInfo_new = config.pInfo_YuvThumbnail;
            break;
        case eSTREAMID_IMAGE_JPEG:
            pPool = mpPool_Jpeg;
            pInfo_old = mCurPipelineConfig.pInfo_Jpeg;
            pInfo_new = config.pInfo_Jpeg;
            break;
        default:
            MY_LOGW("Unknow stream info!!");
            ret = RESULT_UNKNOWN_ERROR;
            break;
    }
    dumpImageStreamInfo(pInfo_new, "reconfigBufferPool: New");

    if (checkResult == RESULT_RELOAD_ALL_BUFFER) {
        if (pPool.get()) {
            CAM_TRACE_NAME("allocate all buffer");
            MY_LOGD_IF(mDebug, "flush unused and incompatible buffer pool for streamId(%#" PRIx64 ")", streamId);
            pPool->flush();
            MY_LOGD_IF(mDebug, "create new buffer pool for streamId(%#" PRIx64 ")", streamId);
            pPool->updateStreamInfo(pInfo_new);
            pPool->allocateBuffer(
                              pInfo_new->getStreamName(),
                              pInfo_new->getMaxBufNum(),
                              pInfo_new->getMinInitBufNum()
                              );
            MY_LOGD_IF(mDebug, "allocate all new buffers done");
        }
        else {
            MY_LOGE("create new pPool failed!");
            ret = RESULT_UNKNOWN_ERROR;
        }
    }
    else if (checkResult == RESULT_ADD_BUFFER) {
        if (pPool.get()) {
            size_t addNumberOfBuffers = pInfo_new->getMaxBufNum() - pInfo_old->getMaxBufNum();

            CAM_TRACE_NAME("allocate added buffer");
            MY_LOGD_IF(mDebug, "number of buffer (old:%zu, new:%zu, add:%zu)",
                    pInfo_old->getMaxBufNum(), pInfo_new->getMaxBufNum(), addNumberOfBuffers);

            if (addNumberOfBuffers > 0) {
                pPool->addAllocateBuffer(addNumberOfBuffers);
                MY_LOGD_IF(mDebug, "allocate added new buffer done");
            }
        }
        else {
            MY_LOGE("create new pPool failed!");
            ret = RESULT_UNKNOWN_ERROR;
        }
    }
    else if (checkResult == RESULT_RECONFIG_STREAM_INFO) {
        if (pPool.get()) {
            CAM_TRACE_NAME("update stream info");
            MY_LOGD_IF(mDebug, "update stream info for streamId(%#" PRIx64 ")", streamId);
            pPool->updateStreamInfo(pInfo_new);
            MY_LOGD_IF(mDebug, "update stream info done");
        }
        else {
            MY_LOGE("query pPool failed!");
            ret = RESULT_UNKNOWN_ERROR;
        }
    }

    if (pPool.get()) {
        switch (streamId) {
            case eSTREAMID_IMAGE_PIPE_RAW_OPAQUE:
                mpPool_FullRaw = pPool;
                break;
            case eSTREAMID_IMAGE_PIPE_RAW_RESIZER:
                mpPool_ResizedRaw = pPool;
                break;
            case eSTREAMID_IMAGE_PIPE_RAW_LCSO:
                mpPool_LcsoRaw = pPool;
                break;
            case eSTREAMID_IMAGE_PIPE_YUV_JPEG:
                mpPool_Yuv = pPool;
                break;
            case eSTREAMID_IMAGE_PIPE_YUV_00:
                mpPool_YuvPostview = pPool;
                break;
            case eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL:
                mpPool_YuvThumbnail = pPool;
                break;
            case eSTREAMID_IMAGE_JPEG:
                mpPool_Jpeg = pPool;
                break;
            default:
                MY_LOGW("Unknow stream info!!");
                ret = RESULT_UNKNOWN_ERROR;
                break;
        }
    }

    MY_LOGD_IF(mDebug, "ret:%d", ret);
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
ICommonCapturePipeline::ResultType
CommonCapturePipeline::
reconfigCallback(
    PipelineConfig &config
)
{
    CAM_TRACE_CALL();
    FUNCTION_SCOPE;

    ResultType ret = RESULT_UNKNOWN_ERROR;

    //update buffer callback
    if (config.pCallbackListener == nullptr) {
        MY_LOGW("config.pCallbackListener is invalid nullptr (user:%s)", config.userName.string());
        return RESULT_UNKNOWN_ERROR;
    }
    else {

        MY_LOGD_IF(mDebug, "callback listener: old:0x%X, new:0x%X",
                   mCurPipelineConfig.pCallbackListener, config.pCallbackListener);
        if (config.pCallbackListener == mCurPipelineConfig.pCallbackListener &&
            mpImageCallback != nullptr &&
            mpCallbackHandler != nullptr &&
            mpMetadataListener != nullptr &&
            mpMetadataListenerFull != nullptr
        ) {
            MY_LOGD("reuse shot instance(0x%X) and no need to reconfig callback handler.",
                    config.pCallbackListener);
            return RESULT_NO_NEED_CHANGE;
        }

        // wait previous shot instance done
        waitUntilDrained();

        mpImageCallback = new SImageCallback(config.pCallbackListener, 0);
        mpCallbackHandler = new BufferCallbackHandler(config.openId);
        if (mpImageCallback.get() && mpCallbackHandler.get()) {
            mpCallbackHandler->setImageCallback(mpImageCallback);
        }
        else {
            MY_LOGE("mpImageCallback(0x%X) or mpCallbackHandler(0x%X) is invalid nullptr",
                    mpImageCallback.get(), mpCallbackHandler.get());
            return RESULT_UNKNOWN_ERROR;
        }

        // metadata Listener
        sp<ResultProcessor> pResultProcessor = mpPipeline->getResultProcessor().promote();
        if (pResultProcessor.get()) {
            mpMetadataListener = new SMetadataListener(config.pCallbackListener);
            if (mpMetadataListener.get()) {
                pResultProcessor->registerListener( 0, UINT_MAX, true, mpMetadataListener);
            }
            else {
                MY_LOGE("regist MetadataListener failed");
                return RESULT_UNKNOWN_ERROR;
            }
            mpMetadataListenerFull = new SMetadataListener(config.pCallbackListener);
            if (mpMetadataListenerFull.get()) {
                pResultProcessor->registerListener( 0, UINT_MAX, false, mpMetadataListenerFull);
            }
            else {
                MY_LOGE("regist MetadataListenerFull failed");
                return RESULT_UNKNOWN_ERROR;
            }
            sp<TimestampProcessor> pTimestampProcessor = mpPipeline->getTimestampProcessor().promote();
            if (pTimestampProcessor.get()) {
                pResultProcessor->registerListener(eSTREAMID_META_APP_DYNAMIC_P1, pTimestampProcessor);
            }
            else {
                MY_LOGE("regist TimestampProcessor failed");
                return RESULT_UNKNOWN_ERROR;
            }
        }
        else{
            MY_LOGE("pResultProcessor is invalid nullptr");
            return RESULT_UNKNOWN_ERROR;
        }
        ret = RESULT_RECONFIG_CALLBACK;
    }

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
CommonCapturePipeline::
reconfigPipeline(
    PipelineConfig &config
)
{
    CAM_TRACE_CALL();
    FUNCTION_SCOPE;
    MERROR ret = OK;

    // update callback
    if(reconfigCallback(config) != RESULT_UNKNOWN_ERROR) {
        // update buffer pool callback
        ret |= reconfigStreamInfo(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE, config);    // pInfo_FullRaw
        ret |= reconfigStreamInfo(eSTREAMID_IMAGE_PIPE_RAW_RESIZER, config);   // pInfo_ResizedRaw
        ret |= reconfigStreamInfo(eSTREAMID_IMAGE_PIPE_RAW_LCSO, config);      // pInfo_LcsoRaw
        ret |= reconfigStreamInfo(eSTREAMID_IMAGE_PIPE_YUV_JPEG, config);      // pInfo_Yuv
        ret |= reconfigStreamInfo(eSTREAMID_IMAGE_PIPE_YUV_00, config);        // pInfo_YuvPostview
        ret |= reconfigStreamInfo(eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL, config); // pInfo_YuvThumbnail
        ret |= reconfigStreamInfo(eSTREAMID_IMAGE_JPEG, config);               // pInfo_Jpeg
    }
    else {
        ret = UNKNOWN_ERROR;
    }

    mCurPipelineConfig = config;
    dumpPipelineConfig(mCurPipelineConfig);

    MY_LOGD_IF(mDebug, "ret:%d", ret);
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
CommonCapturePipeline::
isNeedToReconstruct(
    PipelineConfig &config
)
{
    CAM_TRACE_CALL();
    FUNCTION_SCOPE;
    MBOOL ret = MFALSE;

    MY_LOGD_IF(mDebug, "userName: old(%s), new(%s)",
            mCurPipelineConfig.userName.string(), config.userName.string());
    if(mCurPipelineConfig.isZsdMode != config.isZsdMode) {
        MY_LOGD("isZSDMode: old(%d), new(%d)", mCurPipelineConfig.isZsdMode, config.isZsdMode);
        ret = MTRUE;
    }

    if(!mCurPipelineConfig.isZsdMode) { // non-zsd requirement
        MY_LOGD("Must reconstruct capture pipeline for non-ZSD mode");
        ret = MTRUE;
    }

    // check LegacyPipelineBuilder::ConfigParams LPBConfigParams;
    if (checkLPBConfigParams(config)) {
        MY_LOGW("LPBConfigParams is changed!");
        ret = MTRUE;
    }

    // check sp<BufferCallbackHandler> pBufferCallbackHandler;
    if(mCurPipelineConfig.pCallbackListener != config.pCallbackListener) {
        MY_LOGD_IF(mDebug, "buffer callback is changed! old(0x%X), new(0x%X)",
                   mCurPipelineConfig.pCallbackListener, config.pCallbackListener);
    }

    if (ret) {
        MY_LOGI("reconstruct pipeline(%d)", config.openId);
    }
    else {
        MY_LOGI("reuse and reconfig pipeline", config.openId);
    }

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
ICommonCapturePipeline::ResultType
CommonCapturePipeline::
checkAndUpdateImageStreamInfo(
    sp<IImageStreamInfo>      pInfo_old,
    sp<IImageStreamInfo>&     pInfo_new,
    bool isRaw
)
{
    CAM_TRACE_CALL();
    FUNCTION_SCOPE;
    ResultType ret = RESULT_NO_NEED_CHANGE;

    if (pInfo_new.get() == nullptr) {
        MY_LOGI("new ImageStreamInfo is nullptr, no need to reconfig");
        pInfo_new = pInfo_old;
        ret = RESULT_NO_NEED_CHANGE;
    }
    else if (pInfo_old.get() == nullptr) {
        MY_LOGI("old ImageStreamInfo is nullptr, must reconfig");
        ret = RESULT_RELOAD_ALL_BUFFER;
    }
    else {
        size_t updateMaxBufNum = 0;
        size_t updateMinInitBufNum = 0;
        size_t updateStride = 0;
        size_t updateSizeInBytes = 0;
        size_t bufPlanesScanCount = 0;

        // check if RESULT_RECONFIG_STREAM_INFO
        if (pInfo_old->getUsageForAllocator() != pInfo_new->getUsageForAllocator()) {
            MY_LOGW("usage for allocator is changed! old(0x%X), new(0x%X)",
                    pInfo_old->getUsageForAllocator(), pInfo_new->getUsageForAllocator());
            ret = RESULT_RECONFIG_STREAM_INFO;
        }

        if (pInfo_old->getImgFormat() != pInfo_new->getImgFormat()) {
            MY_LOGW("image format is changed! old(0x%X), new(ox%X)",
                    pInfo_old->getImgFormat(), pInfo_new->getImgFormat());
            ret = RESULT_RECONFIG_STREAM_INFO;
        }

        if ((pInfo_old->getImgSize()).w != (pInfo_new->getImgSize()).w ||
                 (pInfo_old->getImgSize()).h != (pInfo_new->getImgSize()).h ){
            MY_LOGW("image size is changed! old(%d,%d), new(%d,%d)",
                    pInfo_old->getImgSize().w, pInfo_old->getImgSize().h,
                    pInfo_new->getImgSize().w, pInfo_new->getImgSize().h);
            ret = RESULT_RECONFIG_STREAM_INFO;
        }

        if (pInfo_old->getTransform() != pInfo_new->getTransform()) {
            MY_LOGW("transform is changed! old(%d), new(%d)",
                    pInfo_old->getTransform(), pInfo_new->getTransform());
            ret = RESULT_RECONFIG_STREAM_INFO;
        }

        if (pInfo_old->getMinInitBufNum() < pInfo_new->getMinInitBufNum()) {
            MY_LOGW("min buffer num is not enough! old(%d), new(%d)",
                    pInfo_old->getMinInitBufNum(), pInfo_new->getMinInitBufNum());
            updateMinInitBufNum = pInfo_new->getMinInitBufNum();
            ret = RESULT_RECONFIG_STREAM_INFO;
        }
        else {
            updateMinInitBufNum = pInfo_old->getMinInitBufNum();
        }

        if (pInfo_old->getBufPlanes().size() < pInfo_new->getBufPlanes().size()) {
            bufPlanesScanCount = pInfo_old->getBufPlanes().size();
        }
        else {
            bufPlanesScanCount = pInfo_new->getBufPlanes().size();
        }

        if (!isRaw) {
            for (size_t i = 0; i < bufPlanesScanCount; i++) {
                if (pInfo_old->getBufPlanes()[i].rowStrideInBytes != pInfo_new->getBufPlanes()[i].rowStrideInBytes) {
                        updateStride = pInfo_new->getBufPlanes()[0].rowStrideInBytes;
                        ret = RESULT_RECONFIG_STREAM_INFO;

                        MY_LOGW("BufPlanes[%d](rowStrideInBytes) is changed! old(%zu), new(%zu), updateStride(%zu)",
                                i, pInfo_old->getBufPlanes()[i].rowStrideInBytes, pInfo_new->getBufPlanes()[i].rowStrideInBytes, updateStride);
                }
            }
        }
        else{
            updateStride = pInfo_new->getBufPlanes()[0].rowStrideInBytes;
        }

        // check if RESULT_ADD_BUFFER
        if (pInfo_old->getMaxBufNum() < pInfo_new->getMaxBufNum()) {
            MY_LOGW("max buffer num is not enough! old(%d), new(%d)",
                    pInfo_old->getMaxBufNum(), pInfo_new->getMaxBufNum());
            updateMaxBufNum = pInfo_new->getMaxBufNum();
            ret = RESULT_ADD_BUFFER;
        }
        else {
            updateMaxBufNum = pInfo_old->getMaxBufNum();
        }

        // check if RESULT_RELOAD_ALL_BUFFER
        if (pInfo_old->getBufPlanes().size() < pInfo_new->getBufPlanes().size()) {
            MY_LOGW("buffer planes number is changed and not enough! old(%zu), new(%zu)",
                    pInfo_old->getBufPlanes().size(), pInfo_new->getBufPlanes().size());
            ret = RESULT_RELOAD_ALL_BUFFER;
        }

        if (!isRaw) {
            for (size_t i = 0; i < bufPlanesScanCount; i++) {
                MY_LOGD_IF(mDebug, "bufPlanes[%d]: old(size:%zu, reusable:%zu), new(size:%zu, reusable:%zu)",
                           i,
                           pInfo_old->getBufPlanes()[i].sizeInBytes, pInfo_old->getBufPlanes()[i].reusableSizeInBytes,
                           pInfo_new->getBufPlanes()[i].sizeInBytes, pInfo_new->getBufPlanes()[i].reusableSizeInBytes);
                if (pInfo_old->getBufPlanes()[i].reusableSizeInBytes < pInfo_new->getBufPlanes()[i].reusableSizeInBytes) {
                        updateSizeInBytes = (updateSizeInBytes > pInfo_new->getBufPlanes()[i].reusableSizeInBytes) ?
                                             updateSizeInBytes : pInfo_new->getBufPlanes()[i].reusableSizeInBytes;

                        ret = RESULT_RELOAD_ALL_BUFFER;

                        MY_LOGW("BufPlanes[%d](reusableSizeInBytes) is changed and not enough! old(size:%zu, reusable:%zu), new(size:%zu, reusable:%zu)",
                                i,
                                pInfo_old->getBufPlanes()[i].sizeInBytes, pInfo_old->getBufPlanes()[i].reusableSizeInBytes,
                                pInfo_new->getBufPlanes()[i].sizeInBytes, pInfo_new->getBufPlanes()[i].reusableSizeInBytes);
                }
            }
        }

        if (pInfo_old->getStreamType() != pInfo_new->getStreamType()) {
            MY_LOGE("stream type changed and not maping! old(0x%X), new(0x%X)",
                    pInfo_old->getStreamType(), pInfo_new->getStreamType());
            ret = RESULT_RELOAD_ALL_BUFFER;
        }

        if (pInfo_old->getStreamId() != pInfo_new->getStreamId()) {
            MY_LOGE("stream id changed and not maping! old(%#" PRIxPTR "), new(%#" PRIxPTR ")",
                    pInfo_old->getStreamId(), pInfo_new->getStreamId());
            ret = RESULT_UNKNOWN_ERROR;
        }

        if (updateMaxBufNum < updateMinInitBufNum) {
            updateMaxBufNum = updateMinInitBufNum;
        }

        if (ret == RESULT_ADD_BUFFER ||
            ret == RESULT_RELOAD_ALL_BUFFER ) {
            MY_LOGD("ImageStreamInfo is changed! must reconfig buffer");
        }
        else if (ret == RESULT_RECONFIG_STREAM_INFO) {
            MY_LOGD("ImageStreamInfo is changed! must reconfig stream info");
        }

        sp<IImageStreamInfo> pNewStreamInfo = nullptr;
        if (isRaw) {
            pNewStreamInfo = createRawImageStreamInfo(
                    pInfo_new->getStreamName(),        // streamName
                    pInfo_new->getStreamId(),          // streamId
                    pInfo_new->getStreamType(),        // streamType
                    updateMaxBufNum,                   // maxBufNum
                    updateMinInitBufNum,               // minInitBufNum
                    pInfo_new->getUsageForAllocator(), // usageForAllocator
                    pInfo_new->getImgFormat(),         // imgFormat
                    pInfo_new->getImgSize(),           // imgSize
                    updateStride                       // stride
                    );
        }
        else {
            pNewStreamInfo = createImageStreamInfo(
                    pInfo_new->getStreamName(),        // streamName
                    pInfo_new->getStreamId(),          // streamId
                    pInfo_new->getStreamType(),        // streamType
                    updateMaxBufNum,                   // maxBufNum
                    updateMinInitBufNum,               // minInitBufNum
                    pInfo_new->getUsageForAllocator(), // usageForAllocator
                    pInfo_new->getImgFormat(),         // imgFormat
                    pInfo_new->getImgSize(),           // imgSize
                    pInfo_new->getTransform(),         // transform
                    MSize(1,1),                        // align
                    MTRUE,                             // isReusable
                    updateSizeInBytes                  // foecedSizeInBytes
                    );
        }

        if (pNewStreamInfo.get()) {
            dumpImageStreamInfo(pNewStreamInfo, __FUNCTION__);
            pInfo_new = pNewStreamInfo;
        }
        else{
            MY_LOGW("create new ImageStreamInfo failed! (streamId:%#" PRIx64 ")", pInfo_new->getStreamName());
            ret = RESULT_UNKNOWN_ERROR;
        }
    }
    MY_LOGD_IF(mDebug, "ret: %d", ret);
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
CommonCapturePipeline::
checkAndUpdateSensorParams(
    PipelineConfig &config
)
{
    CAM_TRACE_CALL();
    FUNCTION_SCOPE;
    MBOOL ret = MFALSE;

    if (mCurPipelineConfig.sensorParam.mode != config.sensorParam.mode) {
        MY_LOGW("pipeline sensor param (mode) is changed! old(%d), new(%d)",
                mCurPipelineConfig.sensorParam.rawType, config.sensorParam.rawType);
        ret = MTRUE;
    }
    else if (mCurPipelineConfig.sensorParam.rawType != config.sensorParam.rawType) {
        MY_LOGW("pipeline sensor param (rawType) is changed! old(0x%X), new(0x%X)",
                mCurPipelineConfig.sensorParam.rawType, config.sensorParam.rawType);
        ret = MTRUE;
    }
    else if ((mCurPipelineConfig.sensorParam.size.w != config.sensorParam.size.w) ||
             (mCurPipelineConfig.sensorParam.size.w != config.sensorParam.size.w) ){
        MY_LOGW("pipeline sensor param (size) is changed! old(%d,%d), new(%d,%d)",
                mCurPipelineConfig.sensorParam.size.w, mCurPipelineConfig.sensorParam.size.h,
                config.sensorParam.size.w, config.sensorParam.size.h);
        ret = MTRUE;
    }
    else if (mCurPipelineConfig.sensorParam.fps != config.sensorParam.fps) {
        MY_LOGW("pipeline sensor param (fps) is changed! old(0x%X), new(0x%X)",
                mCurPipelineConfig.sensorParam.fps, config.sensorParam.fps);
        ret = MTRUE;
    }
    else if (mCurPipelineConfig.sensorParam.pixelMode != config.sensorParam.pixelMode) {
        MY_LOGW("pipeline sensor param (pixelMode) is changed! old(0x%d), new(0x%X)",
                mCurPipelineConfig.sensorParam.pixelMode, config.sensorParam.pixelMode);
        ret = MTRUE;
    }
    else if (mCurPipelineConfig.sensorParam.vhdrMode != config.sensorParam.vhdrMode) {
        MY_LOGW("pipeline sensor param (vhdrMode) is changed! old(0x%d), new(0x%X)",
                mCurPipelineConfig.sensorParam.vhdrMode, config.sensorParam.vhdrMode);
        ret = MTRUE;
    }
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
CommonCapturePipeline::
checkScenarioControlParams(
    PipelineConfig &config
)
{
    CAM_TRACE_CALL();
    FUNCTION_SCOPE;
    MBOOL ret = MFALSE;

    if (mCurPipelineConfig.scenarioControlParam.scenario != config.scenarioControlParam.scenario) {
        MY_LOGW("scenarioControlParam (scenario) is changed! old(%d), new(%d)",
                mCurPipelineConfig.scenarioControlParam.scenario, config.scenarioControlParam.scenario);
        ret = MTRUE;
    }
    else if (mCurPipelineConfig.scenarioControlParam.sensorSize != config.scenarioControlParam.sensorSize) {
        MY_LOGW("scenarioControlParam (sensorSize) is changed! old(%d), new(%d)",
                mCurPipelineConfig.scenarioControlParam.sensorSize, config.scenarioControlParam.sensorSize);
        ret = MTRUE;
    }
    else if (mCurPipelineConfig.scenarioControlParam.sensorFps != config.scenarioControlParam.sensorFps) {
        MY_LOGW("scenarioControlParam (sensorFps) is changed! old(%d), new(%d)",
                mCurPipelineConfig.scenarioControlParam.sensorFps, config.scenarioControlParam.sensorFps);
        ret = MTRUE;
    }
    else if (mCurPipelineConfig.scenarioControlParam.featureFlag != config.scenarioControlParam.featureFlag) {
        MY_LOGW("scenarioControlParam (featureFlag) is changed! old(%d), new(%d)",
                mCurPipelineConfig.scenarioControlParam.featureFlag, config.scenarioControlParam.featureFlag);
        ret = MTRUE;
    }
    else if (mCurPipelineConfig.scenarioControlParam.enableBWCControl != config.scenarioControlParam.enableBWCControl) {
        MY_LOGW("scenarioControlParam (enableBWCControl) is changed! old(%d), new(%d)",
                mCurPipelineConfig.scenarioControlParam.enableBWCControl, config.scenarioControlParam.enableBWCControl);
        ret = MTRUE;
    }
    else if ((mCurPipelineConfig.scenarioControlParam.videoSize.w != config.scenarioControlParam.videoSize.w) ||
             (mCurPipelineConfig.scenarioControlParam.videoSize.h != config.scenarioControlParam.videoSize.h) ){
        MY_LOGW("scenarioControlParam (videoSize) is changed! old(%d,%d), new(%d,%d)",
                mCurPipelineConfig.scenarioControlParam.videoSize.w, mCurPipelineConfig.scenarioControlParam.videoSize.h,
                config.scenarioControlParam.videoSize.w, config.scenarioControlParam.videoSize.h);
        ret = MTRUE;
    }
    else if (mCurPipelineConfig.scenarioControlParam.camMode != config.scenarioControlParam.camMode) {
        MY_LOGW("scenarioControlParam (camMode) is changed! old(%d), new(%d)",
                mCurPipelineConfig.scenarioControlParam.camMode, config.scenarioControlParam.camMode);
        ret = MTRUE;
    }
    else if (mCurPipelineConfig.scenarioControlParam.supportCustomOption != config.scenarioControlParam.supportCustomOption) {
        MY_LOGW("scenarioControlParam (supportCustomOption) is changed! old(%d), new(%d)",
                mCurPipelineConfig.scenarioControlParam.supportCustomOption, config.scenarioControlParam.supportCustomOption);
        ret = MTRUE;
    }
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
CommonCapturePipeline::
checkLPBConfigParams(
    PipelineConfig &config
)
{
    CAM_TRACE_CALL();
    FUNCTION_SCOPE;
    MBOOL ret = MFALSE;

    if (mCurPipelineConfig.LPBConfigParams.mode != config.LPBConfigParams.mode) {
        MY_LOGW("mode is changed! old(0x%X), new(0x%X)",
                mCurPipelineConfig.LPBConfigParams.mode, config.LPBConfigParams.mode);
        ret = MTRUE;
    }
    else if (mCurPipelineConfig.LPBConfigParams.enableEIS != config.LPBConfigParams.enableEIS) {
        MY_LOGW("enableEIS is changed! old(%d), new(%d)",
                mCurPipelineConfig.LPBConfigParams.enableEIS, config.LPBConfigParams.enableEIS);
        ret = MTRUE;
    }
    else if (mCurPipelineConfig.LPBConfigParams.enableLCS != config.LPBConfigParams.enableLCS) {
        MY_LOGW("enableLCS is changed! old(%d), new(%d)",
                mCurPipelineConfig.LPBConfigParams.enableLCS, config.LPBConfigParams.enableLCS);
        ret = MTRUE;
    }
    else if (mCurPipelineConfig.LPBConfigParams.enableRSS != config.LPBConfigParams.enableRSS) {
        MY_LOGW("enableRSS is changed! old(%d), new(%d)",
                mCurPipelineConfig.LPBConfigParams.enableRSS, config.LPBConfigParams.enableRSS);
        ret = MTRUE;
    }
    else if (mCurPipelineConfig.LPBConfigParams.enableDualPD != config.LPBConfigParams.enableDualPD) {
        MY_LOGW("enableDualPD is changed! old(%d), new(%d)",
                mCurPipelineConfig.LPBConfigParams.enableDualPD, config.LPBConfigParams.enableDualPD);
        ret = MTRUE;
    }
    else if (mCurPipelineConfig.LPBConfigParams.eisMode != config.LPBConfigParams.eisMode) {
        MY_LOGW("eisMode is changed! old(%d), new(%d)",
                mCurPipelineConfig.LPBConfigParams.eisMode, config.LPBConfigParams.eisMode);
        ret = MTRUE;
    }
    else if (mCurPipelineConfig.LPBConfigParams.enableBurstNum != config.LPBConfigParams.enableBurstNum) {
        MY_LOGW("enableBurstNum is changed! old(%d), new(%d)",
                mCurPipelineConfig.LPBConfigParams.enableBurstNum, config.LPBConfigParams.enableBurstNum);
        ret = MTRUE;
    }
    else if (mCurPipelineConfig.LPBConfigParams.optimizedCaptureFlow != config.LPBConfigParams.optimizedCaptureFlow) {
        MY_LOGW("optimizedCaptureFlow is changed! old(%d), new(%d)",
                mCurPipelineConfig.LPBConfigParams.optimizedCaptureFlow, config.LPBConfigParams.optimizedCaptureFlow);
        ret = MTRUE;
    }
    else if (mCurPipelineConfig.LPBConfigParams.disableFrontalBinning != config.LPBConfigParams.disableFrontalBinning) {
        MY_LOGW("disableFrontalBinning is changed! old(%d), new(%d)",
                mCurPipelineConfig.LPBConfigParams.disableFrontalBinning, config.LPBConfigParams.disableFrontalBinning);
        ret = MTRUE;
    }
    else if (mCurPipelineConfig.LPBConfigParams.enableUNI != config.LPBConfigParams.enableUNI) {
        MY_LOGW("enableUNI is changed! old(%d), new(%d)",
                mCurPipelineConfig.LPBConfigParams.enableUNI, config.LPBConfigParams.enableUNI);
        ret = MTRUE;
    }
    else if (mCurPipelineConfig.LPBConfigParams.dualcamMode != config.LPBConfigParams.dualcamMode) {
        MY_LOGW("dualcamMode is changed! old(%d), new(%d)",
                mCurPipelineConfig.LPBConfigParams.dualcamMode, config.LPBConfigParams.dualcamMode);
        ret = MTRUE;
    }
    else if (mCurPipelineConfig.LPBConfigParams.receiveMode != config.LPBConfigParams.receiveMode) {
        MY_LOGW("receiveMode is changed! old(%d), new(%d)",
                mCurPipelineConfig.LPBConfigParams.receiveMode, config.LPBConfigParams.receiveMode);
        ret = MTRUE;
    }
    else if (mCurPipelineConfig.LPBConfigParams.pluginUser != config.LPBConfigParams.pluginUser) {
        MY_LOGW("pluginUser is changed! old(0x%X), new(0x%X)",
                mCurPipelineConfig.LPBConfigParams.pluginUser, config.LPBConfigParams.pluginUser);
        //ret = MTRUE;
    }
    else if (mCurPipelineConfig.LPBConfigParams.disableHLR != config.LPBConfigParams.disableHLR) {
        MY_LOGW("disableHLR is changed! old(%d), new(%d)",
                mCurPipelineConfig.LPBConfigParams.disableHLR, config.LPBConfigParams.disableHLR);
        ret = MTRUE;
    }
    else if (mCurPipelineConfig.LPBConfigParams.pipeBit != config.LPBConfigParams.pipeBit) {
        MY_LOGW("pipeBit is changed! old(0x%X), new(0x%X)",
                mCurPipelineConfig.LPBConfigParams.pipeBit, config.LPBConfigParams.pipeBit);
        ret = MTRUE;
    }
    else if (mCurPipelineConfig.LPBConfigParams.initRequest != config.LPBConfigParams.initRequest) {
        MY_LOGW("initRequest is changed! old(%d), new(%d)",
                mCurPipelineConfig.LPBConfigParams.initRequest, config.LPBConfigParams.initRequest);
        ret = MTRUE;
    }
    else if (mCurPipelineConfig.LPBConfigParams.support4cellsensor != config.LPBConfigParams.support4cellsensor) {
        MY_LOGW("support4cellsensor is changed! old(%d), new(%d)",
                mCurPipelineConfig.LPBConfigParams.support4cellsensor, config.LPBConfigParams.support4cellsensor);
        ret = MTRUE;
    }
    else if (mCurPipelineConfig.LPBConfigParams.enableRrzoCapture != config.LPBConfigParams.enableRrzoCapture) {
        MY_LOGW("enableRrzoCapture is changed! old(%d), new(%d)",
                mCurPipelineConfig.LPBConfigParams.enableRrzoCapture, config.LPBConfigParams.enableRrzoCapture);
        ret = MTRUE;
    }
    else if (mCurPipelineConfig.LPBConfigParams.enableQuadCode != config.LPBConfigParams.enableQuadCode) {
        MY_LOGW("enableQuadCode is changed! old(%d), new(%d)",
                mCurPipelineConfig.LPBConfigParams.enableQuadCode, config.LPBConfigParams.enableQuadCode);
        ret = MTRUE;
    }

    return ret;
}

/******************************************************************************
 * Interfaces for pipeline control
 ******************************************************************************/
MBOOL
CommonCapturePipeline::
isPipelineExist()
{
    if (mpPipeline.get()) {
        return MTRUE;
    }
    return MFALSE;
}

MERROR
CommonCapturePipeline::
unregistAndReturnBufferToPool()
{
    CAM_TRACE_CALL();
    FUNCTION_SCOPE;

    MERROR ret = OK;
    MINT32 openId = mCurPipelineConfig.openId;
    String8 userName = mCurPipelineConfig.userName;

    // free unused callback handler to return buffer to pool
    mpCallbackHandler = nullptr;
    mpImageCallback = nullptr;
    mpMetadataListener = nullptr;
    mpMetadataListenerFull = nullptr;
    mCurPipelineConfig.pCallbackListener = nullptr;

    return ret;
}

MERROR
CommonCapturePipeline::
flush(MBOOL cleanRss)
{
    FUNCTION_SCOPE;
    MERROR ret = UNKNOWN_ERROR;
    if (mpPipeline.get()) {
        if (CC_UNLIKELY(cleanRss))
            return mpPipeline->flush();
        else
            return mpPipeline->flushForReuse();
    }
    return UNKNOWN_ERROR;
}

MERROR
CommonCapturePipeline::
waitUntilDrained()
{
    FUNCTION_SCOPE;
    if (mpPipeline.get()) {
        return mpPipeline->waitUntilDrained();
    }
    return UNKNOWN_ERROR;
}

MERROR
CommonCapturePipeline::
waitUntilDrainedAndFlush()
{
    FUNCTION_SCOPE;
    if (mpPipeline.get()) {
        return mpPipeline->waitUntilDrainedAndFlush();
    }
    return UNKNOWN_ERROR;
}

MERROR
CommonCapturePipeline::
waitUntilP1DrainedAndFlush()
{
    FUNCTION_SCOPE;
    if (mpPipeline.get()) {
        return mpPipeline->waitUntilP1DrainedAndFlush();
    }
    return UNKNOWN_ERROR;
}

MERROR
CommonCapturePipeline::
waitUntilP2JpegDrainedAndFlush()
{
    FUNCTION_SCOPE;
    if (mpPipeline.get()) {
        return mpPipeline->waitUntilP2JpegDrainedAndFlush();
    }
    return UNKNOWN_ERROR;
}


wp<ResultProcessor>
CommonCapturePipeline::
getResultProcessor()
{
    FUNCTION_SCOPE;
    if (mpPipeline.get()) {
        return mpPipeline->getResultProcessor();
    }
    return nullptr;
}

wp<TimestampProcessor>
CommonCapturePipeline::
getTimestampProcessor()
{
    if (mpPipeline.get()) {
        return mpPipeline->getTimestampProcessor();
    }
    return nullptr;
}

MERROR
CommonCapturePipeline::
submitSetting(
    MINT32    const requestNo,
    IMetadata& appMeta,
    IMetadata& halMeta,
    ILegacyPipeline::ResultSet* pResultSet
)
{
    if (mpPipeline.get()) {
        return mpPipeline->submitSetting(
                requestNo, appMeta, halMeta, pResultSet);
    }
    return UNKNOWN_ERROR;
}

MERROR
CommonCapturePipeline::
submitRequest(
    MINT32    const   requestNo,
    IMetadata&        appMeta,
    IMetadata&        halMeta,
    Vector<BufferSet> vDstStreams,
    ILegacyPipeline::ResultSet* pResultSet
)
{
    if (mpPipeline.get()) {
        return mpPipeline->submitRequest(
                requestNo, appMeta, halMeta, vDstStreams, pResultSet);
    }
    return UNKNOWN_ERROR;
}

sp<IImageStreamInfo>
CommonCapturePipeline::
queryImageStreamInfo(
    StreamId const streamId
)
{
    if (mpPipeline.get()) {
        return mpPipeline->queryImageStreamInfo(streamId);
    }
    return nullptr;
}

sp<IMetaStreamInfo>
CommonCapturePipeline::
queryMetaStreamInfo(
    StreamId const streamId
)
{
    if (mpPipeline.get()) {
        return mpPipeline->queryMetaStreamInfo(streamId);
    }
    return nullptr;
}

sp<StreamBufferProvider>
CommonCapturePipeline::
queryProvider(
    StreamId const streamId
)
{
    if (mpPipeline.get()) {
        return mpPipeline->queryProvider(streamId);
    }
    return nullptr;
}

MVOID
CommonCapturePipeline::
setRequestBuilder(
    sp<RequestBuilder> const pRequestBuilder
)
{
    if (mpPipeline.get()) {
        mpPipeline->setRequestBuilder(pRequestBuilder);
    }
    return;
}

MVOID
CommonCapturePipeline::
getMetaIOMapInfo(
    KeyedVector<NSCam::v3::Pipeline_NodeId_T,
    NSCam::v3::NSPipelineContext::IOMapSet >& vInfo
)
{
    if (mpPipeline.get()) {
        mpPipeline->getMetaIOMapInfo(vInfo);
    }
    return;
}

MERROR
CommonCapturePipeline::
getFrameNo(
    MINT32 &frameNo
)
{
    if (mpPipeline.get()) {
        return mpPipeline->getFrameNo(frameNo);
    }
    return UNKNOWN_ERROR;
}

/******************************************************************************
 * Interfaces for CallbackBufferPool
 ******************************************************************************/
android::sp<CallbackBufferPool>
CommonCapturePipeline::
queryBufferPool(
    StreamId_T id
)
{
    if (mpCallbackHandler.get()) {
        return mpCallbackHandler->queryBufferPool(id);
    }
    return nullptr;
}
