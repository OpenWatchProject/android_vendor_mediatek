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

#define LOG_TAG "ThirdPartyCapture"

#include "ThirdPartyCaptureRequestBuilder.h"
// mtkcam
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include <mtkcam/middleware/v1/LegacyPipeline/NodeId.h>
#include <mtkcam/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam/utils/metadata/IMetadata.h>
// size related
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam/middleware/v1/IParamsManager.h> // to get bokeh level
#include <mtkcam/middleware/v1/IParamsManagerV3.h>
// metadata tag
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h> // hal
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>// app
//
#include <mtkcam/drv/iopipe/SImager/ISImagerDataTypes.h>
#include <mtkcam/feature/stereo/hal/stereo_common.h>
//
#include <mtkcam/utils/std/Log.h>
//
//#include "../buffer/CallbackImageBufferManager.h"
#include "../../../../../../common/CbImgBufMgr/CallbackImageBufferManager.h"
#include "../image/IImageShotCallback.h"
#include "../metadata/IMetaShotCallback.h"
//
#include "../ImageShotCallbackFactory.h"
#include "../MetadataShotCallbackFactory.h"
//
#include "../processor/ShotCallbackProcessor.h"
//
#include <isp_tuning/isp_tuning.h>
using namespace android;
using namespace NSCam;
using namespace NSCam::v1::NSLegacyPipeline;
/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
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
template <typename T>
inline MBOOL
tryGetMetadata(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T & rVal
)
{
    if( pMetadata == NULL ) {
        MY_LOGW("pMetadata == NULL");
        return MFALSE;
    }

    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if( !entry.isEmpty() ) {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }
    return MFALSE;
}
/******************************************************************************
 *
 ******************************************************************************/
ThirdPartyCaptureRequestBuilder::
~ThirdPartyCaptureRequestBuilder()
{
    MY_LOGD("dctor(%p)", this);
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
ThirdPartyCaptureRequestBuilder::
onInit(
    CaptureRequestInitSetting &setting
)
{
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
ThirdPartyCaptureRequestBuilder::
onPrepareImageBufferProvider()
{
    status_t ret = UNKNOWN_ERROR;
    if(mpCallbackBufferHandler == nullptr)
    {
        MY_LOGE("BufferCallbackHandler promote fail");
        return ret;
    }
    if(mpCallbackImageBufferManager == nullptr)
    {
        MY_LOGE("mpCallbackImageBufferManager is nullptr");
        return ret;
    }
    auto createImageStreamInfo = [](
                char const* streamName,
                StreamId_T streamId,
                MUINT32 streamType,
                size_t maxBufNum,
                size_t minInitBufNum,
                MUINT usageForAllocator,
                MINT imgFormat,
                MSize const& imgSize,
                MUINT32 transform)
    {
        IImageStreamInfo::BufPlanes_t bufPlanes;
    #define addBufPlane(planes, height, stride)                                      \
            do{                                                                      \
                size_t _height = (size_t)(height);                                   \
                size_t _stride = (size_t)(stride);                                   \
                IImageStreamInfo::BufPlane bufPlane= { _height * _stride, _stride }; \
                planes.push_back(bufPlane);                                          \
            }while(0)
        switch( imgFormat ) {
            case eImgFmt_YV12:
                addBufPlane(bufPlanes , imgSize.h      , imgSize.w);
                addBufPlane(bufPlanes , imgSize.h >> 1 , imgSize.w >> 1);
                addBufPlane(bufPlanes , imgSize.h >> 1 , imgSize.w >> 1);
                break;
            case eImgFmt_NV21:
                addBufPlane(bufPlanes , imgSize.h      , imgSize.w);
                addBufPlane(bufPlanes , imgSize.h >> 1 , imgSize.w);
                break;
            case eImgFmt_RAW16:
            case eImgFmt_YUY2:
                addBufPlane(bufPlanes , imgSize.h      , imgSize.w << 1);
                break;
            case eImgFmt_Y8:
            case eImgFmt_STA_BYTE:
                addBufPlane(bufPlanes , imgSize.h      , imgSize.w);
                break;
            case eImgFmt_RGBA8888:
                addBufPlane(bufPlanes , imgSize.h      , imgSize.w<<2);
                break;
            case eImgFmt_BLOB:
                /*
                add 328448 for image size
                standard exif: 1280 bytes
                4 APPn for debug exif: 0xFF80*4 = 65408*4 bytes
                max thumbnail size: 64K bytes
                */
                addBufPlane(bufPlanes , 1              , (imgSize.w * imgSize.h * 12 / 10) + 328448); //328448 = 64K+1280+65408*4
                break;
            default:
                MY_LOGE("format not support yet %p", imgFormat);
                break;
        }
    #undef  addBufPlane

        sp<ImageStreamInfo>
            pStreamInfo = new ImageStreamInfo(
                    streamName,
                    streamId,
                    streamType,
                    maxBufNum, minInitBufNum,
                    usageForAllocator, imgFormat, imgSize, bufPlanes, transform
                    );

        if( pStreamInfo == NULL ) {
            MY_LOGE("create ImageStream failed, %s, %#" PRIxPTR ,
                    streamName, streamId);
        }

        return pStreamInfo;
    };
    // full yuv
    sp<IImageStreamInfo> pFullYuvStreamInfo = nullptr;
    // full yuv main2
    sp<IImageStreamInfo> pFullYuvMain2StreamInfo = nullptr;
    // resize yuv
    sp<IImageStreamInfo> pResizeYuvStreamInfo = nullptr;
    // resize yuv main2
    sp<IImageStreamInfo> pResizeYuvMain2StreamInfo = nullptr;
    // rrzo main1 postview
    sp<IImageStreamInfo> pPostViewStreamInfo = nullptr;

    // get stereo image size from StereoSizeProvider.
    StereoSizeProvider* pStereoSizeProvider = StereoSizeProvider::getInstance();

    MSize finalPictureSize_imgo_m1, finalPictureSize_rrzo_m1;
    MSize finalPictureSize_imgo_m2, finalPictureSize_rrzo_m2;
    MSize postviewSize;
    /*
    MSize finalPictureSize = MSize(mShotParams.mShotParam.mi4PictureWidth, mShotParams.mShotParam.mi4PictureHeight);
    MSize CaptureSize = finalPictureSize;*/
    MSize finalThumbnailSize = MSize(mShotParams.mJpegParam.mi4JpegThumbWidth, mShotParams.mJpegParam.mi4JpegThumbHeight);

    postviewSize = pStereoSizeProvider->postViewSize(StereoSettingProvider::imageRatio());
    if(mShotParams.mShotParam.mu4Transform & eTransform_ROT_90){
        //finalPictureSize = MSize(mShotParams.mShotParam.mi4PictureHeight, mShotParams.mShotParam.mi4PictureWidth);
        postviewSize = MSize(postviewSize.h, postviewSize.w);
    }
    MY_LOGD("[Postview] size: %dx%d", postviewSize.w, postviewSize.h);

    size_t  stride;
    MSize   imageSize;
    MRect   imageCrop;

    bool bRet = MTRUE;

    // main1 imgo
    bRet = pStereoSizeProvider->getcustomYUVSize(
                                    StereoHAL::eSTEREO_SENSOR_MAIN1, //:(StereoHAL::eSTEREO_SENSOR_MAIN2),
                                    NSImageio::NSIspio::EPortIndex_IMGO,
                                    finalPictureSize_imgo_m1);
    // bRet = pStereoSizeProvider->getPass1Size(
    //                     StereoHAL::eSTEREO_SENSOR_MAIN1, //:(StereoHAL::eSTEREO_SENSOR_MAIN2),
    //                     (EImageFormat)eImgFmt_BAYER10,  // eImgFmt_BAYER10 is for IMGO, eImgFmt_FG_BAYER10 is for RRZO
    //                     NSImageio::NSIspio::EPortIndex_IMGO,
    //                     (StereoHAL::ENUM_STEREO_SCENARIO)eSTEREO_SCENARIO_CAPTURE,
    //                     (MRect&)imageCrop,
    //                     (MSize&)imageSize,
    //                     (MUINT32&)stride);
    if(!bRet) {
        MY_LOGE("[Main1 IMGO] Get Pass1 Size Fail.");
        return ret;
    }

    //finalPictureSize_imgo_m1 = imageSize;
    //if(mShotParams.mShotParam.mu4Transform & eTransform_ROT_90){
    //    finalPictureSize_imgo_m1 = MSize(imageSize.h, imageSize.w);
    //}
    MY_LOGD("[Main1 IMGO] final capture size: picture:%dx%d",
        finalPictureSize_imgo_m1.w,
        finalPictureSize_imgo_m1.h
    );

    // main2 imgo
    bRet = pStereoSizeProvider->getcustomYUVSize(
                                    StereoHAL::eSTEREO_SENSOR_MAIN2, //:(StereoHAL::eSTEREO_SENSOR_MAIN2),
                                    NSImageio::NSIspio::EPortIndex_IMGO,
                                    finalPictureSize_imgo_m2);
    // bRet = pStereoSizeProvider->getPass1Size(
    //                     StereoHAL::eSTEREO_SENSOR_MAIN2,
    //                     (EImageFormat)eImgFmt_BAYER10,
    //                     NSImageio::NSIspio::EPortIndex_IMGO,
    //                     (StereoHAL::ENUM_STEREO_SCENARIO)eSTEREO_SCENARIO_CAPTURE,
    //                     (MRect&)imageCrop,
    //                     (MSize&)imageSize,
    //                     (MUINT32&)stride);
    if(!bRet) {
        MY_LOGE("[Main2 IMGO] Get Pass1 Size Fail.");
        return ret;
    }

    //finalPictureSize_imgo_m2 = imageSize;
    //if(mShotParams.mShotParam.mu4Transform & eTransform_ROT_90){
    //    finalPictureSize_imgo_m2 = MSize(imageSize.h, imageSize.w);
    //}
    MY_LOGD("[Main2 IMGO] final capture size: picture:%dx%d",
        finalPictureSize_imgo_m2.w,
        finalPictureSize_imgo_m2.h
    );

    // main1 rrzo
    bRet = pStereoSizeProvider->getcustomYUVSize(
                                    StereoHAL::eSTEREO_SENSOR_MAIN1, //:(StereoHAL::eSTEREO_SENSOR_MAIN2),
                                    NSImageio::NSIspio::EPortIndex_RRZO,
                                    finalPictureSize_rrzo_m1);
    // bRet = pStereoSizeProvider->getPass1Size(
    //                     StereoHAL::eSTEREO_SENSOR_MAIN1, //:(StereoHAL::eSTEREO_SENSOR_MAIN2),
    //                     (EImageFormat)eImgFmt_FG_BAYER10,  // eImgFmt_BAYER10 is for IMGO, eImgFmt_FG_BAYER10 is for RRZO
    //                     NSImageio::NSIspio::EPortIndex_RRZO,
    //                     (StereoHAL::ENUM_STEREO_SCENARIO)eSTEREO_SCENARIO_CAPTURE,
    //                     (MRect&)imageCrop,
    //                     (MSize&)imageSize,
    //                     (MUINT32&)stride);
    if(!bRet) {
        MY_LOGE("[Main1 RRZO] Get Pass1 Size Fail.");
        return ret;
    }

    //finalPictureSize_rrzo_m1 = imageSize;
    //if(mShotParams.mShotParam.mu4Transform & eTransform_ROT_90){
    //    finalPictureSize_imgo_m1 = MSize(imageSize.h, imageSize.w);
    //}
    MY_LOGD("[Main1 RRZO] final capture size: picture:%dx%d",
        finalPictureSize_rrzo_m1.w,
        finalPictureSize_rrzo_m1.h
    );

    // main2 rrzo
    bRet = pStereoSizeProvider->getcustomYUVSize(
                                    StereoHAL::eSTEREO_SENSOR_MAIN2, //:(StereoHAL::eSTEREO_SENSOR_MAIN2),
                                    NSImageio::NSIspio::EPortIndex_RRZO,
                                    finalPictureSize_rrzo_m2);
    // bRet = pStereoSizeProvider->getPass1Size(
    //                     StereoHAL::eSTEREO_SENSOR_MAIN2,
    //                     (EImageFormat)eImgFmt_FG_BAYER10,
    //                     NSImageio::NSIspio::EPortIndex_RRZO,
    //                     (StereoHAL::ENUM_STEREO_SCENARIO)eSTEREO_SCENARIO_CAPTURE,
    //                     (MRect&)imageCrop,
    //                     (MSize&)imageSize,
    //                     (MUINT32&)stride);
    if(!bRet) {
        MY_LOGE("[Main2 RRZO] Get Pass1 Size Fail.");
        return ret;
    }

    //finalPictureSize_rrzo_m2 = imageSize;
    //if(mShotParams.mShotParam.mu4Transform & eTransform_ROT_90){
    //    finalPictureSize_imgo_m2 = MSize(imageSize.h, imageSize.w);
    //}
    MY_LOGD("[Main2 RRZO] final capture size: picture:%dx%d",
        finalPictureSize_rrzo_m2.w,
        finalPictureSize_rrzo_m2.h
    );

    //------------------------------------------------------------------------------

    // full yuv
    {
        pFullYuvStreamInfo =
                        createImageStreamInfo((std::string("Hal:Image:FullYuv")+std::to_string(miCaptureNo)).c_str(),
                                              eSTREAMID_IMAGE_PIPE_THIRDPARTY_FULLSIZE_YUV_0,
                                              eSTREAMTYPE_IMAGE_INOUT,
                                              1,
                                              1,
                                              eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE |eBUFFER_USAGE_SW_WRITE_OFTEN,
                                              eImgFmt_NV21,
                                              finalPictureSize_imgo_m1,
                                              0);
    }
    // full yuv main2
    {
        pFullYuvMain2StreamInfo =
                        createImageStreamInfo((std::string("Hal:Image:FullYuvMain2")+std::to_string(miCaptureNo)).c_str(),
                                              eSTREAMID_IMAGE_PIPE_THIRDPARTY_FULLSIZE_YUV_1,
                                              eSTREAMTYPE_IMAGE_INOUT,
                                              1,
                                              1,
                                              eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE |eBUFFER_USAGE_SW_WRITE_OFTEN,
                                              eImgFmt_NV21,
                                              finalPictureSize_imgo_m2,
                                              0);
    }
    // resize yuv
    {
        pResizeYuvStreamInfo =
                        createImageStreamInfo((std::string("Hal:Image:ResizeYuv")+std::to_string(miCaptureNo)).c_str(),
                                              eSTREAMID_IMAGE_PIPE_THIRDPARTY_RESIZE_YUV_0,
                                              eSTREAMTYPE_IMAGE_INOUT,
                                              1,
                                              1,
                                              eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE |eBUFFER_USAGE_SW_WRITE_OFTEN,
                                              eImgFmt_NV21,
                                              finalPictureSize_rrzo_m1,
                                              0);
    }
    // resize yuv main2
    {
        pResizeYuvMain2StreamInfo =
                        createImageStreamInfo((std::string("Hal:Image:ResizeYuvMain2")+std::to_string(miCaptureNo)).c_str(),
                                              eSTREAMID_IMAGE_PIPE_THIRDPARTY_RESIZE_YUV_1,
                                              eSTREAMTYPE_IMAGE_INOUT,
                                              1,
                                              1,
                                              eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE |eBUFFER_USAGE_SW_WRITE_OFTEN,
                                              eImgFmt_NV21,
                                              finalPictureSize_rrzo_m2,
                                              0);
    }
    // rrzo main1 postview
    {
        pPostViewStreamInfo =
                        createImageStreamInfo((std::string("Hal:Image:PostView")+std::to_string(miCaptureNo)).c_str(),
                                              eSTREAMID_IMAGE_PIPE_DUALYUVNODE_THUMBNAIL,
                                              eSTREAMTYPE_IMAGE_INOUT,
                                              1,
                                              1,
                                              eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN,
                                              mShotParams.mShotParam.miPostviewThumbFmt,
                                              MSize(postviewSize.w, postviewSize.h),
                                              mShotParams.mShotParam.mu4Transform);
    }

    // update dst stream
    auto addDstStreams = [this](MINT32 streamId, MBOOL bNeedProvider)
    {
        mvDstStreams.push_back(
            BufferSet{
                .streamId       = streamId,
                .criticalBuffer = bNeedProvider,
            }
        );
    };
    // metadata
    addDstStreams(eSTREAMID_META_HAL_DYNAMIC_P1, MFALSE);
    addDstStreams(eSTREAMID_META_HAL_DYNAMIC_P1_MAIN2, MFALSE);
    // image
    addDstStreams(eSTREAMID_IMAGE_PIPE_THIRDPARTY_FULLSIZE_YUV_0, MTRUE);
    addDstStreams(eSTREAMID_IMAGE_PIPE_THIRDPARTY_FULLSIZE_YUV_1, MTRUE);
    addDstStreams(eSTREAMID_IMAGE_PIPE_THIRDPARTY_RESIZE_YUV_0, MTRUE);
    addDstStreams(eSTREAMID_IMAGE_PIPE_THIRDPARTY_RESIZE_YUV_1, MTRUE);
    addDstStreams(eSTREAMID_IMAGE_PIPE_DUALYUVNODE_THUMBNAIL, MTRUE);

    // allocate buffer, no need to callback
    ret = mpCallbackImageBufferManager->allocBuffer(pFullYuvStreamInfo         ,mpCallbackBufferHandler);
    ret |= mpCallbackImageBufferManager->allocBuffer(pFullYuvMain2StreamInfo   ,mpCallbackBufferHandler);
    ret |= mpCallbackImageBufferManager->allocBuffer(pResizeYuvStreamInfo      ,mpCallbackBufferHandler);
    ret |= mpCallbackImageBufferManager->allocBuffer(pResizeYuvMain2StreamInfo ,mpCallbackBufferHandler);
    ret |= mpCallbackImageBufferManager->allocBuffer(pPostViewStreamInfo ,mpCallbackBufferHandler);
lbExit:
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
ThirdPartyCaptureRequestBuilder::
onDestory(
)
{
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
ThirdPartyCaptureRequestBuilder::
onUpdateMetadata(
    IMetadata* appMeta,
    IMetadata* halMeta,
    IMetadata* main2AppMeta,
    IMetadata* main2HalMeta
)
{
    status_t ret = UNKNOWN_ERROR;
    if(appMeta == nullptr ||
       halMeta == nullptr ||
       main2AppMeta == nullptr ||
       main2HalMeta == nullptr)
    {
        MY_LOGE("metadata is nullptr");
        return ret;
    }
    // add bokeh mode to p2node metadata (main1)
    {
        IMetadata::IEntry entry(MTK_P2NODE_BOKEH_ISP_PROFILE);
        entry.push_back(NSIspTuning::EIspProfile_N3D_Capture, Type2Type< MUINT8 >());
        halMeta->update(entry.tag(), entry);
    }
    // add bokeh mode to p2node metadata (main2)
    {
        IMetadata::IEntry entry(MTK_P2NODE_BOKEH_ISP_PROFILE);
        entry.push_back(NSIspTuning::EIspProfile_N3D_Capture, Type2Type< MUINT8 >());
        main2HalMeta->update(entry.tag(), entry);
    }
    // pack main2 metadata to main1
    {
        IMetadata::IEntry entry_meta(MTK_P1NODE_MAIN2_HAL_META);
        entry_meta.push_back(*main2HalMeta, Type2Type< IMetadata >());
        halMeta->update(entry_meta.tag(), entry_meta);
    }
    // patch main2 app metadata to main1 hal metadata
    {
        MY_LOGD("push main2 app metadata");
        IMetadata::IEntry entry_meta(MTK_P1NODE_MAIN2_APP_META);
        entry_meta.push_back(*main2AppMeta, Type2Type< IMetadata >());
        halMeta->update(entry_meta.tag(), entry_meta);
    }
    // force using hw to encode all jpeg
    {
        IMetadata::IEntry entry(MTK_JPG_ENCODE_TYPE);
        entry.push_back(NSCam::NSIoPipe::NSSImager::JPEGENC_HW_ONLY, Type2Type< MUINT8 >());
        halMeta->update(entry.tag(), entry);
    }
    // for capture, force turn off 3dnr.
    {
        IMetadata::IEntry entry(MTK_NR_FEATURE_3DNR_MODE);
        entry.push_back(MFALSE, Type2Type< MINT32 >());
        appMeta->update(entry.tag(), entry);
    }
    // add capture intent
    {
        IMetadata::IEntry entry(MTK_CONTROL_CAPTURE_INTENT);
        entry.push_back(MTK_CONTROL_CAPTURE_INTENT_ZERO_SHUTTER_LAG, Type2Type< MUINT8 >());
        appMeta->update(entry.tag(), entry);
    }
    {
        sp<IParamsManagerV3> paramMgr = mpParamMgrV3.promote();
        if(paramMgr == nullptr)
        {
            MY_LOGE("get paramMgr fail");
            return UNKNOWN_ERROR;
        }
        {
            // set dof level to metadata
            MINT32 bokehLevel = paramMgr->getParamsMgr()->getInt(MtkCameraParameters::KEY_STEREO_DOF_LEVEL);
            int DoFLevel_HAL = bokehLevel*2;
            int DoFMin_HAL = 0;
            int DoFMax_HAL = 30;

            DoFLevel_HAL = std::max(DoFMin_HAL, std::min(DoFLevel_HAL, DoFMax_HAL));

            MY_LOGD("Capture Bokeh level(%d)", DoFLevel_HAL);
            IMetadata::IEntry entry(MTK_STEREO_FEATURE_DOF_LEVEL);
            entry.push_back(DoFLevel_HAL, Type2Type< MINT32 >());
            appMeta->update(entry.tag(), entry);
        }
        {
            // update touch info for ff lens
            String8 sString = paramMgr->getParamsMgr()->getStr(MtkCameraParameters::KEY_STEREO_TOUCH_POSITION);
            // String8 sString = String8("-110,120"); for test
            const char *strPosX = sString.string();
            const char *strPosY = strPosX ? ::strchr(strPosX, ',') : NULL;
            MINT32  PosX = 0, PosY = 0;
            if(strPosX != NULL && strPosY != NULL) {
                PosX = ::atoi(strPosX);
                PosY = ::atoi(strPosY+1);
                if(PosX < -1000 || PosX > 1000 || PosY < -1000 || PosY > 1000) {
                    PosX = 0;
                    PosY = 0;
                }
            }
            MY_LOGD("Get touch pos from parameter(x, y): %d, %d", PosX, PosY);
            IMetadata::IEntry entry(MTK_STEREO_FEATURE_TOUCH_POSITION);
            entry.push_back(PosX, Type2Type< MINT32 >());
            entry.push_back(PosY, Type2Type< MINT32 >());
            appMeta->update(entry.tag(), entry);
        }
    }
#if 1
    // Debug Log
    {
        MSize tempSize;
        if( ! tryGetMetadata<MSize>(const_cast<IMetadata*>(halMeta), MTK_HAL_REQUEST_SENSOR_SIZE, tempSize) ){
            MY_LOGE("cannot get MTK_HAL_REQUEST_SENSOR_SIZE after updating request");
        }else{
            MY_LOGD("MTK_HAL_REQUEST_SENSOR_SIZE:(%dx%d)", tempSize.w, tempSize.h);
        }
        IMetadata tempMetadata;
        if( ! tryGetMetadata<IMetadata>(const_cast<IMetadata*>(halMeta), MTK_P1NODE_MAIN2_HAL_META, tempMetadata) ){
            MY_LOGE("cannot get MTK_P1NODE_MAIN2_HAL_META after updating request");
        }else{
            MY_LOGD("MTK_P1NODE_MAIN2_HAL_META");
        }
        MINT32 tempLevel;
        if( ! tryGetMetadata<MINT32>(const_cast<IMetadata*>(appMeta), MTK_STEREO_FEATURE_DOF_LEVEL, tempLevel) ){
            MY_LOGE("cannot get MTK_STEREO_FEATURE_DOF_LEVEL after updating request");
        }else{
            MY_LOGD("MTK_STEREO_FEATURE_DOF_LEVEL:%d", tempLevel);
        }
        MINT32 tempTransform;
        if( ! tryGetMetadata<MINT32>(const_cast<IMetadata*>(appMeta), MTK_JPEG_ORIENTATION, tempTransform) ){
            MY_LOGE("cannot get MTK_JPEG_ORIENTATION after updating request");
        }else{
            MY_LOGD("MTK_JPEG_ORIENTATION:%d", tempTransform);
        }
    }
#endif
    ret = OK;
lbExit:
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
ThirdPartyCaptureRequestBuilder::
onSetImageCallback()
{
    MY_LOGD("---- start ----");
    status_t ret = UNKNOWN_ERROR;
    if(mpShotCallbackProcessor == nullptr)
    {
        MY_LOGE("mpShotCallbackProcessor is null");
        return ret;
    }
    auto createCallback = [this](MUINT32 callbackType, IImageShotCallback::ImageCallbackParams params)
    {
        sp<IImageShotCallback> pImgCallback =
                ImageShotCallbackFactory::createCallback(
                                        callbackType,
                                        params);
        if(pImgCallback!=nullptr)
        {
            mpShotCallbackProcessor->addCallback(pImgCallback);
        }
        else
        {
            MY_LOGE("create callback fail. streamId(0x%x)", params.streamId);
            return UNKNOWN_ERROR;
        }
        return OK;
    };

    IImageShotCallback::ImageCallbackParams params;
    // create full yuv image callback
    {
        MY_LOGD("---- create callback for full yuv 0 ----");
        params.streamId = eSTREAMID_IMAGE_PIPE_THIRDPARTY_FULLSIZE_YUV_0;
        params.streamName = "Full_YUV_0__";
        params.fileExtension = ".yuv";
        params.callbackIndex = 0;
        params.msgType = MTK_CAMERA_MSG_EXT_DATA_STEREO_MAIN1_YUV;
        params.slot = 0;
        ret = createCallback(ImageShotCallbackFactory::BGYUV, params);
    }
    // create full yuv main2 image callback
    if(::supportMain2FullRaw())
    {
        MY_LOGD("---- create callback for full yuv 1 ----");
        params.streamId = eSTREAMID_IMAGE_PIPE_THIRDPARTY_FULLSIZE_YUV_1;
        params.streamName = "Full_YUV_1__";
        params.fileExtension = ".yuv";
        params.callbackIndex = 1;
        params.msgType = MTK_CAMERA_MSG_EXT_DATA_STEREO_MAIN2_YUV;
        params.slot = 1;
        ret |= createCallback(ImageShotCallbackFactory::BGYUV, params);
    }
    // create resize yuv image callback
    {
        MY_LOGD("---- create callback for resize yuv 0 ----");
        params.streamId = eSTREAMID_IMAGE_PIPE_THIRDPARTY_RESIZE_YUV_0;
        params.streamName = "Resize_YUV_0__";
        params.fileExtension = ".yuv";
        params.callbackIndex = 2;
        params.msgType = MTK_CAMERA_MSG_EXT_DATA_STEREO_MAIN1_YUV;
        params.slot = 0;
        ret |= createCallback(ImageShotCallbackFactory::BGYUV, params);
    }
    // create resize yuv main2 image callback
    {
        MY_LOGD("---- create callback for resize yuv 1 ----");
        params.streamId = eSTREAMID_IMAGE_PIPE_THIRDPARTY_RESIZE_YUV_1;
        params.streamName = "Resize_YUV_1__";
        params.fileExtension = ".yuv";
        params.callbackIndex = 3;
        params.msgType = MTK_CAMERA_MSG_EXT_DATA_STEREO_MAIN2_YUV;
        params.slot = 1;
        ret |= createCallback(ImageShotCallbackFactory::BGYUV, params);
    }
    // create rrzo main1 postview image callback
    {
        params.streamId = eSTREAMID_IMAGE_PIPE_DUALYUVNODE_THUMBNAIL;
        params.streamName = "Postview";
        params.fileExtension = ".yuv";
        params.callbackIndex = 4;
        params.msgType = 0;
        params.mbNeedCaptureDoneCb = MFALSE;
        params.mbNeedToCountForCapture = MFALSE;
        params.slot = 0; // reset
        ret |= createCallback(ImageShotCallbackFactory::POSTVIEW, params);
    }
    MY_LOGD("---- end ----");
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
ThirdPartyCaptureRequestBuilder::
onSetMetaCallback()
{
    status_t ret = UNKNOWN_ERROR;
    if(mpShotCallbackProcessor == nullptr)
    {
        MY_LOGE("mpShotCallbackProcessor is null");
        return ret;
    }
    sp<IMetaShotCallback> pMetaCallback =
                MetadataShotCallbackFactory::createCallback(
                                    MetadataShotCallbackFactory::COLLECT,
                                    eSTREAMID_META_HAL_DYNAMIC_P2,
                                    *mMain1AppMetadata,
                                    0);
    {
            MRect tempSize;
        
        if( tryGetMetadata<MRect>(mMain1AppMetadata, MTK_SCALER_CROP_REGION, tempSize) )
        {
            MY_LOGD("crop size (%d, %d, %d, %d)", tempSize.p.x, tempSize.p.y, tempSize.s.w, tempSize.s.h);
        }
        else
        {
            MY_LOGE("can not get crop size");
        }
    }
    if(pMetaCallback!=nullptr)
    {
        mpShotCallbackProcessor->addCallback(pMetaCallback);
    }
    else
    {
        MY_LOGE("create callback fail. streamId(0x%x)", eSTREAMID_META_HAL_DYNAMIC_P2);
        return UNKNOWN_ERROR;
    }
    pMetaCallback =
                MetadataShotCallbackFactory::createCallback(
                                    MetadataShotCallbackFactory::COLLECT,
                                    eSTREAMID_META_HAL_DYNAMIC_P2_MAIN2,
                                    *mMain2AppMetadata,
                                    1);
    {
            MRect tempSize;
        if( tryGetMetadata<MRect>(mMain2AppMetadata, MTK_SCALER_CROP_REGION, tempSize) )
        {
            MY_LOGD("crop size (%d, %d, %d, %d)", tempSize.p.x, tempSize.p.y, tempSize.s.w, tempSize.s.h);
        }
        else
        {
            MY_LOGE("can not get crop size");
        }
    }
    if(pMetaCallback!=nullptr)
    {
        mpShotCallbackProcessor->addCallback(pMetaCallback);
    }
    else
    {
        MY_LOGE("create callback fail. streamId(0x%x)", eSTREAMID_META_HAL_DYNAMIC_P2_MAIN2);
        return UNKNOWN_ERROR;
    }
    ret = OK;
    return ret;
}
