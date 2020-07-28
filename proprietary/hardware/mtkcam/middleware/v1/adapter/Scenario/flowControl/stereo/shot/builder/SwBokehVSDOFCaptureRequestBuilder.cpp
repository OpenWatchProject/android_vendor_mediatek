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
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#define LOG_TAG "SwBokehCapture"

#include "SwBokehVSDOFCaptureRequestBuilder.h"
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
//
#include <mtkcam/utils/std/Log.h>
//
#include "../../../../../../common/CbImgBufMgr/CallbackImageBufferManager.h"
#include "../image/IImageShotCallback.h"
#include "../metadata/IMetaShotCallback.h"
//
#include "../ImageShotCallbackFactory.h"
#include "../MetadataShotCallbackFactory.h"
//
#include "../processor/ShotCallbackProcessor.h"
//
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
SwBokehVSDOFCaptureRequestBuilder::
~SwBokehVSDOFCaptureRequestBuilder()
{
    MY_LOGD("dctor(%p)", this);
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
SwBokehVSDOFCaptureRequestBuilder::
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
SwBokehVSDOFCaptureRequestBuilder::
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
            MY_LOGE("create ImageStream failed, %s, %#" PRIx64 ,
                    streamName, streamId);
        }

        return pStreamInfo;
    };
    // depthmapnode provider
    sp<IImageStreamInfo> pMainImage_Cap = nullptr;
    sp<IImageStreamInfo> pHalDepthWrapperStreamInfo = nullptr;
    sp<IImageStreamInfo> pLDCDataStreamInfo = nullptr;
    sp<IImageStreamInfo> pExtraDataStreamInfo = nullptr;
    sp<IImageStreamInfo> pN3DDebugStreamInfo = nullptr;
    sp<IImageStreamInfo> pAppExtraDataStreamInfo = nullptr;
    // sw bokeh provider
    sp<IImageStreamInfo> bokehResultStreamInfo = nullptr;
    sp<IImageStreamInfo> bokehCleanImageStreamInfo = nullptr;
    sp<IImageStreamInfo> thumbImageStreamInfo = nullptr;
    sp<IImageStreamInfo> pDepthMapWrapper = nullptr;
    // jpeg - bokeh
    sp<IImageStreamInfo> pSwBokehJpegStreamInfo = nullptr;
    // jpeg - clean image
    sp<IImageStreamInfo> pCleanJpegStreamInfo = nullptr;
    // postview
    sp<IImageStreamInfo> pPostViewStreamInfo = nullptr;
    // get stereo image size from StereoSizeProvider.
    StereoSizeProvider* pStereoSizeProvider = StereoSizeProvider::getInstance();
    MSize depthMapWrapper =
                        pStereoSizeProvider->getBufferSize(
                                    E_BOKEH_PACKED_BUFFER,
                                    eSTEREO_SCENARIO_CAPTURE).size;
    MSize ldcSize = pStereoSizeProvider->getBufferSize(E_LDC, eSTEREO_SCENARIO_CAPTURE).size;
    MUINT32 extraDataSize = StereoSettingProvider::getExtraDataBufferSizeInBytes();
    MUINT32 n3dDebugSize = StereoSettingProvider::getMaxN3DDebugBufferSizeInBytes();

    MSize finalPictureSize = MSize(mShotParams.mShotParam.mi4PictureWidth, mShotParams.mShotParam.mi4PictureHeight);
    MSize CaptureSize = finalPictureSize;
    MSize finalThumbnailSize = MSize(mShotParams.mJpegParam.mi4JpegThumbWidth, mShotParams.mJpegParam.mi4JpegThumbHeight);
    MSize finalPostViewSize  = MSize(mShotParams.mShotParam.mi4PostviewWidth, mShotParams.mShotParam.mi4PostviewHeight);
    if(mShotParams.mShotParam.mu4Transform & eTransform_ROT_90){
        finalPictureSize   = MSize(mShotParams.mShotParam.mi4PictureHeight, mShotParams.mShotParam.mi4PictureWidth);
        finalPostViewSize  = MSize(mShotParams.mShotParam.mi4PostviewHeight, mShotParams.mShotParam.mi4PostviewWidth);
    }
    MY_LOGD("depWrap:%dx%d ldc:%dx%d extra:%d n3d:%d",
                depthMapWrapper.w, depthMapWrapper.h,
                ldcSize.w, ldcSize.h,
                extraDataSize,
                n3dDebugSize);
    MY_LOGD("final capture size: picture:%dx%d",
        finalPictureSize.w,
        finalPictureSize.h
    );
    // pMainImage_Cap
    {
        pMainImage_Cap = createImageStreamInfo(
                                        (std::string("Hal:Image:Main_Cap")+std::to_string(miCaptureNo)).c_str(),
                                        eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_MAINIMAGE_CAPYUV,
                                        eSTREAMTYPE_IMAGE_INOUT,
                                        1,
                                        1,
                                        eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN,
                                        eImgFmt_NV21,
                                        CaptureSize,
                                        0);
    }
    // pHalDepthWrapperStreamInfo
    {
        pHalDepthWrapperStreamInfo = createImageStreamInfo(
                                        (std::string("Hal:Image:HalDepthWrapper")+std::to_string(miCaptureNo)).c_str(),
                                        eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_HAL_DEPTHWAPPER,
                                        eSTREAMTYPE_IMAGE_INOUT,
                                        1,
                                        1,
                                        eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN,
                                        eImgFmt_STA_BYTE,
                                        depthMapWrapper,
                                        0);
    }
    // pLDCDataStreamInfo
    {
        pLDCDataStreamInfo = createImageStreamInfo(
                                        (std::string("Hal:Buf:LDC")+std::to_string(miCaptureNo)).c_str(),
                                        eSTREAMID_IMAGE_PIPE_STEREO_DBG_LDC,
                                        eSTREAMTYPE_IMAGE_INOUT,
                                        1,
                                        1,
                                        eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN,
                                        eImgFmt_STA_BYTE,
                                        ldcSize,
                                        0);
    }
    // pExtraDataStreamInfo
    {
        pExtraDataStreamInfo = createImageStreamInfo(
                                        (std::string("Hal:Buf:ExtraData")+std::to_string(miCaptureNo)).c_str(),
                                        eSTREAMID_IMAGE_PIPE_STEREO_DBG,
                                        eSTREAMTYPE_IMAGE_INOUT,
                                        1,
                                        1,
                                        eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN,
                                        eImgFmt_STA_BYTE,
                                        MSize(extraDataSize, 1),
                                        0);
    }
    // pAppExtraDataStreamInfo
    {
        pAppExtraDataStreamInfo = createImageStreamInfo(
                                        (std::string("App:Buf:ExtraData")+std::to_string(miCaptureNo)).c_str(),
                                        eSTREAMID_IMAGE_PIPE_STEREO_APP_EXTRA_DATA,
                                        eSTREAMTYPE_IMAGE_INOUT,
                                        1,
                                        1,
                                        eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN,
                                        eImgFmt_STA_BYTE,
                                        MSize(extraDataSize, 1),
                                        0);
    }
    // pN3DDebugStreamInfo
    {
        pN3DDebugStreamInfo = createImageStreamInfo(
                                        (std::string("Hal:Buf:N3DDebug")+std::to_string(miCaptureNo)).c_str(),
                                        eSTREAMID_IMAGE_PIPE_STEREO_N3D_DEBUG,
                                        eSTREAMTYPE_IMAGE_INOUT,
                                        1,
                                        1,
                                        eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN,
                                        eImgFmt_STA_BYTE,
                                        MSize(n3dDebugSize, 1),
                                        0);
    }
    // pDepthMapWrapper
    {
        pDepthMapWrapper = createImageStreamInfo(
                                        (std::string("App:Buf:DepthWrapper")+std::to_string(miCaptureNo)).c_str(),
                                        eSTREAMID_IMAGE_PIPE_STEREO_DEPTHWRAPPER,
                                        eSTREAMTYPE_IMAGE_INOUT,
                                        1,
                                        1,
                                        eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN,
                                        eImgFmt_STA_BYTE,
                                        depthMapWrapper,
                                        0);
    }
    // thumbnail
    {
        thumbImageStreamInfo =
                        createImageStreamInfo((std::string("Hal:Image:Bokeh_Thumb")+std::to_string(miCaptureNo)).c_str(),
                                              eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL,
                                              eSTREAMTYPE_IMAGE_INOUT,
                                              1,
                                              1,
                                              eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN,
                                              eImgFmt_YV12,
                                              finalThumbnailSize,
                                              0);
    }
    // bokehResult
    {
        bokehResultStreamInfo =
                        createImageStreamInfo((std::string("Hal:Image:BokehResult")+std::to_string(miCaptureNo)).c_str(),
                                              eSTREAMID_IMAGE_PIPE_BOKEHNODE_RESULTYUV,
                                              eSTREAMTYPE_IMAGE_INOUT,
                                              1,
                                              1,
                                              eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE,
                                              eImgFmt_NV21,
                                              finalPictureSize,
                                              mShotParams.mShotParam.mu4Transform);
    }

    // bokehClean
    {
        bokehCleanImageStreamInfo =
                        createImageStreamInfo((std::string("Hal:Image:CleanImage")+std::to_string(miCaptureNo)).c_str(),
                                              eSTREAMID_IMAGE_PIPE_BOKEHNODE_CLEANIMAGEYUV,
                                              eSTREAMTYPE_IMAGE_INOUT,
                                              1,
                                              1,
                                              eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE,
                                              eImgFmt_NV21,
                                              finalPictureSize,
                                              mShotParams.mShotParam.mu4Transform);
    }
    // jpgBokeh
    {
        pSwBokehJpegStreamInfo =
                        createImageStreamInfo((std::string("Hal:Image:JpegEnc_Bokeh")+std::to_string(miCaptureNo)).c_str(),
                                              eSTREAMID_IMAGE_PIPE_JPG_Bokeh,
                                              eSTREAMTYPE_IMAGE_INOUT,
                                              1,
                                              1,
                                              eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN,
                                              eImgFmt_BLOB,
                                              finalPictureSize,
                                              0);
    }
    // jpgClean
    {
        pCleanJpegStreamInfo =
                        createImageStreamInfo((std::string("Hal:Image:JpegEnc_Clean")+std::to_string(miCaptureNo)).c_str(),
                                              eSTREAMID_IMAGE_PIPE_JPG_CleanMainImg,
                                              eSTREAMTYPE_IMAGE_INOUT,
                                              1,
                                              1,
                                              eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN,
                                              eImgFmt_BLOB,
                                              finalPictureSize,
                                              0);
    }
    // postview
    {
        pPostViewStreamInfo =
                        createImageStreamInfo((std::string("APP:Image:postview")+std::to_string(miCaptureNo)).c_str(),
                                              eSTREAMID_IMAGE_PIPE_DUALYUVNODE_THUMBNAIL,
                                              eSTREAMTYPE_IMAGE_INOUT,
                                              1,
                                              1,
                                              eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN,
                                              mShotParams.mShotParam.miPostviewThumbFmt,
                                              finalPostViewSize,
                                              0);
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
    addDstStreams(eSTREAMID_META_APP_DYNAMIC_DEPTH, MFALSE);
    addDstStreams(eSTREAMID_META_HAL_DYNAMIC_DEPTH, MFALSE);
    addDstStreams(eSTREAMID_META_HAL_DYNAMIC_BOKEH, MFALSE);
    addDstStreams(eSTREAMID_META_APP_DYNAMIC_BOKEH, MFALSE);
    addDstStreams(eSTREAMID_META_APP_DYNAMIC_DUALIT, MFALSE);
    addDstStreams(eSTREAMID_META_APP_DYNAMIC_BOKEH_JPG, MFALSE);
    addDstStreams(eSTREAMID_META_APP_DYNAMIC_JPEG, MFALSE);
    // image
    addDstStreams(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_MAINIMAGE_CAPYUV, MTRUE);
    addDstStreams(eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_HAL_DEPTHWAPPER, MTRUE);
    addDstStreams(eSTREAMID_IMAGE_PIPE_STEREO_DBG_LDC, MTRUE);
    addDstStreams(eSTREAMID_IMAGE_PIPE_STEREO_DBG, MTRUE);
    addDstStreams(eSTREAMID_IMAGE_PIPE_STEREO_N3D_DEBUG, MTRUE);
    addDstStreams(eSTREAMID_IMAGE_PIPE_BOKEHNODE_CLEANIMAGEYUV, MTRUE);
    addDstStreams(eSTREAMID_IMAGE_PIPE_BOKEHNODE_RESULTYUV, MTRUE);
    addDstStreams(eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL, MTRUE);
    addDstStreams(eSTREAMID_IMAGE_PIPE_STEREO_DEPTHWRAPPER, MTRUE);
    addDstStreams(eSTREAMID_IMAGE_PIPE_STEREO_APP_EXTRA_DATA, MTRUE);
    addDstStreams(eSTREAMID_IMAGE_PIPE_JPG_Bokeh, MTRUE);
    addDstStreams(eSTREAMID_IMAGE_PIPE_JPG_CleanMainImg, MTRUE);
    addDstStreams(eSTREAMID_IMAGE_PIPE_DUALYUVNODE_THUMBNAIL, MTRUE);

    // allocate buffer
    ret = mpCallbackImageBufferManager->allocBuffer(pAppExtraDataStreamInfo,mpCallbackBufferHandler);
    ret |= mpCallbackImageBufferManager->allocBuffer(pLDCDataStreamInfo,    mpCallbackBufferHandler);
    ret |= mpCallbackImageBufferManager->allocBuffer(pN3DDebugStreamInfo,   mpCallbackBufferHandler);
    ret |= mpCallbackImageBufferManager->allocBuffer(pDepthMapWrapper,      mpCallbackBufferHandler);
    ret |= mpCallbackImageBufferManager->allocBuffer(pSwBokehJpegStreamInfo,mpCallbackBufferHandler);
    ret |= mpCallbackImageBufferManager->allocBuffer(pCleanJpegStreamInfo,  mpCallbackBufferHandler);
    ret |= mpCallbackImageBufferManager->allocBuffer(pPostViewStreamInfo,  mpCallbackBufferHandler);
    ret |= mpCallbackImageBufferManager->allocBuffer(pMainImage_Cap);
    ret |= mpCallbackImageBufferManager->allocBuffer(pHalDepthWrapperStreamInfo);
    ret |= mpCallbackImageBufferManager->allocBuffer(pExtraDataStreamInfo);
    ret |= mpCallbackImageBufferManager->allocBuffer(bokehResultStreamInfo);
    ret |= mpCallbackImageBufferManager->allocBuffer(bokehCleanImageStreamInfo);
    ret |= mpCallbackImageBufferManager->allocBuffer(thumbImageStreamInfo);
lbExit:
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
SwBokehVSDOFCaptureRequestBuilder::
onDestory(
)
{
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
SwBokehVSDOFCaptureRequestBuilder::
onUpdateMetadata(
    IMetadata* appMeta,
    IMetadata* halMeta,
    IMetadata* main2AppMeta,
    IMetadata* main2HalMeta
)
{
    status_t ret = UNKNOWN_ERROR;
    MINT32 vFNumber[] = {110, 100, 90, 80, 72, 63, 56, 45, 36, 28, 22, 18, 14, 12, 10, 8};
    if(appMeta == nullptr ||
       halMeta == nullptr ||
       main2AppMeta == nullptr ||
       main2HalMeta == nullptr)
    {
        MY_LOGE("metadata is nullptr");
        return ret;
    }
    // pack main2 metadata to main1
    {
        IMetadata::IEntry entry_meta(MTK_P1NODE_MAIN2_HAL_META);
        entry_meta.push_back(*main2HalMeta, Type2Type< IMetadata >());
        halMeta->update(entry_meta.tag(), entry_meta);
    }
    // set dof level to metadata
    {
        sp<IParamsManagerV3> paramMgr = mpParamMgrV3.promote();
        if(paramMgr == nullptr)
        {
            MY_LOGE("get paramMgr fail");
            return UNKNOWN_ERROR;
        }
        MINT32 bokehLevel = paramMgr->getParamsMgr()->getInt(MtkCameraParameters::KEY_STEREO_DOF_LEVEL);
        int DoFLevel_HAL = bokehLevel*2;
        int DoFMin_HAL = 0;
        int DoFMax_HAL = 30;

        DoFLevel_HAL = std::max(DoFMin_HAL, std::min(DoFLevel_HAL, DoFMax_HAL));

        MY_LOGD("Capture Bokeh level(%d)", DoFLevel_HAL);
        IMetadata::IEntry entry(MTK_STEREO_FEATURE_DOF_LEVEL);
        entry.push_back(DoFLevel_HAL, Type2Type< MINT32 >());
        appMeta->update(entry.tag(), entry);
        // update F number to 3a exif
        {
            IMetadata exifMeta;
            if( tryGetMetadata<IMetadata>(halMeta, MTK_3A_EXIF_METADATA, exifMeta) )
            {
                MY_LOGD("update bokeh level");
                IMetadata::IEntry entry_1(MTK_3A_EXIF_FNUMBER);
                entry_1.push_back(vFNumber[bokehLevel], Type2Type< MINT32 >());
                exifMeta.update(entry_1.tag(), entry_1);
                // update to exif metadata
                IMetadata::IEntry entry_2(MTK_3A_EXIF_METADATA);
                entry_2.push_back(exifMeta, Type2Type< IMetadata >());
                halMeta->update(entry_2.tag(), entry_2);
            }
            else
            {
                MY_LOGW("no tag: MTK_3A_EXIF_METADATA");
            }
        }
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
    // update touch info for ff lens
    {
        sp<IParamsManagerV3> paramMgr = mpParamMgrV3.promote();
        if(paramMgr == nullptr)
        {
            MY_LOGE("get paramMgr fail");
            return UNKNOWN_ERROR;
        }
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
SwBokehVSDOFCaptureRequestBuilder::
onSetImageCallback()
{
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
    // create bokeh result image callback
    {
        params.streamId = eSTREAMID_IMAGE_PIPE_JPG_Bokeh;
        params.streamName = "JPEG_BOKEH";
        params.fileExtension = ".jpg";
        params.callbackIndex = 0;
        params.msgType = MTK_CAMERA_MSG_EXT_DATA_COMPRESSED_IMAGE;
        ret = createCallback(ImageShotCallbackFactory::JPEG, params);
    }
    // create clear result image callback
    {
        params.streamId = eSTREAMID_IMAGE_PIPE_JPG_CleanMainImg;
        params.streamName = "JPEG_CLEAN";
        params.fileExtension = ".jpg";
        params.callbackIndex = 1;
        params.msgType = MTK_CAMERA_MSG_EXT_DATA_STEREO_CLEAR_IMAGE;
        ret |= createCallback(ImageShotCallbackFactory::JPEG, params);
    }
    // create extra debug image callback
    {
        params.streamId = eSTREAMID_IMAGE_PIPE_STEREO_APP_EXTRA_DATA;
        params.streamName = "ExtraDebug";
        params.fileExtension = ".dbg";
        params.callbackIndex = 2;
        params.msgType = MTK_CAMERA_MSG_EXT_DATA_STEREO_DBG;
        ret |= createCallback(ImageShotCallbackFactory::DEPTHMAP, params);
    }
    // create ldc image callback
    {
        params.streamId = eSTREAMID_IMAGE_PIPE_STEREO_DBG_LDC;
        params.streamName = "Ldc";
        params.fileExtension = ".dbg";
        params.callbackIndex = 3;
        params.msgType = MTK_CAMERA_MSG_EXT_DATA_STEREO_LDC;
        ret |= createCallback(ImageShotCallbackFactory::DEPTHMAP, params);
    }
    // create n3d image callback
    {
        params.streamId = eSTREAMID_IMAGE_PIPE_STEREO_N3D_DEBUG;
        params.streamName = "N3DDebug";
        params.fileExtension = ".dbg";
        params.callbackIndex = 4;
        params.msgType = MTK_CAMERA_MSG_EXT_DATA_STEREO_N3D_DEBUG;
        ret |= createCallback(ImageShotCallbackFactory::DEPTHMAP, params);
    }
    // create ldc image callback
    {
        params.streamId = eSTREAMID_IMAGE_PIPE_STEREO_DEPTHWRAPPER;
        params.streamName = "DepthWrapper";
        params.fileExtension = ".yuv";
        params.callbackIndex = 5;
        params.msgType = MTK_CAMERA_MSG_EXT_DATA_STEREO_DEPTHWRAPPER;
        ret |= createCallback(ImageShotCallbackFactory::DEPTHMAP, params);
    }
    // create postview image callback
    {
        params.streamId = eSTREAMID_IMAGE_PIPE_DUALYUVNODE_THUMBNAIL;
        params.streamName = "Postview";
        params.fileExtension = ".yuv";
        params.callbackIndex = 6;
        params.msgType = 0;
        params.mbNeedCaptureDoneCb = MFALSE;
        params.mbNeedToCountForCapture = MFALSE;
        ret |= createCallback(ImageShotCallbackFactory::POSTVIEW, params);
    }
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
SwBokehVSDOFCaptureRequestBuilder::
onSetMetaCallback()
{
    status_t ret = UNKNOWN_ERROR;
    if(mpShotCallbackProcessor == nullptr)
    {
        MY_LOGE("mpShotCallbackProcessor is null");
        return ret;
    }
    // when take picture, shutter sound already be trigger.
    /*IMetadata metadata;
    sp<IMetaShotCallback> pMetaCallback =
                MetadataShotCallbackFactory::createCallback(
                                    MetadataShotCallbackFactory::SHUTTER,
                                    eSTREAMID_META_APP_DYNAMIC_DEPTH,
                                    metadata);*/
    ret = OK;
    return ret;
}
