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

#define LOG_TAG "mtkcam-CaptureStreamUpdaterPolicy"

#include <mtkcam3/pipeline/policy/ICaptureStreamUpdaterPolicy.h>

#include <algorithm>

#include <mtkcam3/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/hw/HwInfoHelper.h>
#include "MyUtils.h"
#include <cutils/properties.h>
#include <mtkcam/utils/std/ULog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_POLICY);

/******************************************************************************
 *
 ******************************************************************************/
using namespace android;


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)

#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)
#define ABS(x,y) ((x)>(y)?(x)-(y):(y)-(x))
#define ASPECT_TOLERANCE 0.03
using namespace NSCam::v3::Utils;
using namespace NSCam::v3::pipeline::policy::capturestreamupdater;

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace policy {


/******************************************************************************
 *
 ******************************************************************************/
static auto
createBufPlanes_ThumbnailYuv(
    IImageStreamInfo::BufPlanes_t& bufPlanes,
    int             imgFormat,
    MSize const&    imgSize
) -> bool
{
    auto addBufPlane = [](size_t idx, auto& planes, size_t height, size_t stride) {
        planes[idx] = IImageStreamInfo::BufPlane{
            .sizeInBytes = (height * stride),
            .rowStrideInBytes = stride,
        };
    };

    switch( imgFormat )
    {
    case eImgFmt_NV21:
        bufPlanes.count = 2;
        addBufPlane(0, bufPlanes.planes , imgSize.h      , imgSize.w);
        addBufPlane(1, bufPlanes.planes , imgSize.h >> 1 , imgSize.w);
        break;
    case eImgFmt_YUY2:
        bufPlanes.count = 1;
        addBufPlane(0, bufPlanes.planes , imgSize.h      , imgSize.w << 1);
        break;
    default:
        MY_LOGE("unsupported format:%#x", imgFormat);
        break;
    }

    return true;
}


/******************************************************************************
 *
 ******************************************************************************/
static auto
createImageStreamInfo_JpegYuv(
    android::sp<IImageStreamInfo>   pTemplateStreamInfo,
    MINT                            imgFormat,
    MSize const&                    imgSize,
    MUINT32                         transform
) -> sp<IImageStreamInfo>
{
    ImageBufferInfo imageBufferInfo;
    bool ret = IHwInfoHelperManager::get()->getDefaultBufPlanes_JpegYuv(imageBufferInfo.bufPlanes, imgFormat, imgSize);
    MY_LOGF_IF(!ret, "getDefaultBufPlanes_JpegYuv with format:%#x %dx%d - template:%s",
        imgFormat, imgSize.w, imgSize.h, pTemplateStreamInfo->toString().c_str());

    imageBufferInfo.count     = 1;
    imageBufferInfo.bufStep   = 0;
    imageBufferInfo.startOffset = 0;
    imageBufferInfo.imgFormat = imgFormat;
    imageBufferInfo.imgWidth  = imgSize.w;
    imageBufferInfo.imgHeight = imgSize.h;

    android::sp<IImageStreamInfo> pStreamInfo =
        ImageStreamInfoBuilder(pTemplateStreamInfo.get())
        .setAllocImgFormat(imgFormat)
        .setAllocBufPlanes(imageBufferInfo.bufPlanes)
        .setImageBufferInfo(imageBufferInfo)
        .setTransform(transform)
        .build();

    MY_LOGF_IF(pStreamInfo==nullptr,
        "Fail to build a new IImageStreamInfo - template:%s",
        pTemplateStreamInfo->toString().c_str());
    return pStreamInfo;
}


/******************************************************************************
 *
 ******************************************************************************/
static auto calcThumbnailYuvSize(
    MSize const rPicSize,
    MSize const rThumbnailsize
) -> MSize
{
#define align2(x) (((x) + 1) & (~0x1))
    MSize size;
    MUINT32 const val0 = rPicSize.w * rThumbnailsize.h;
    MUINT32 const val1 = rPicSize.h * rThumbnailsize.w;
    if( val0 > val1 ) {
        size.w = align2(val0/rPicSize.h);
        size.h = rThumbnailsize.h;
    }
    else if( val0 < val1 ) {
        size.w = align2(rThumbnailsize.w);
        size.h = align2(val1/rPicSize.w);
    }
    else {
        size = rThumbnailsize;
    }
#undef align2
    MY_LOGD_IF(1, "thumb %dx%d, pic %dx%d -> yuv for thumb %dx%d",
            rThumbnailsize.w, rThumbnailsize.h,
            rPicSize.w, rPicSize.h,
            size.w, size.h
            );
    return size;
}


static auto createStreamInfoLocked_Thumbnail_YUV(
    RequestOutputParams& out __unused,
    RequestInputParams const& in __unused
) -> int
{
    auto const pMetadata    = in.pRequest_AppControl;
    auto const pCfgThumbYUV  = *(in.pConfiguration_HalImage_Thumbnail_YUV);
    auto const pCfgJpegYUV  = *(in.pConfiguration_HalImage_Jpeg_YUV);
    if ( CC_UNLIKELY( ! pMetadata || ! pCfgThumbYUV.get() ) ) {
        MY_LOGE("invlaid input params");
        return NO_INIT;
    }

    IMetadata::IEntry const& entryThumbnailSize = pMetadata->entryFor(MTK_JPEG_THUMBNAIL_SIZE);
    MSize thumbnailSize;
    if  ( entryThumbnailSize.isEmpty() ) {
        int sensorID = in.sensorID;
        sp<IMetadataProvider> pMetadataProvider = NSMetadataProviderManager::valueFor(sensorID);
        IMetadata static_meta = pMetadataProvider->getMtkStaticCharacteristics();
        IMetadata::IEntry const& entryAvaliableSize = static_meta.entryFor(MTK_JPEG_AVAILABLE_THUMBNAIL_SIZES);
        if (entryAvaliableSize.count() == 0) {
            MY_LOGW("No tag: MTK_JPEG_AVAILABLE_THUMBNAIL_SIZES");
            return OK;
        }
        MSize jpegSize = pCfgJpegYUV->getImgSize();
        MY_LOGW("Select thumbnail size from MTK_JPEG_AVAILABLE_THUMBNAIL_SIZES, sersorID:%d, JpegSize size: w x h: %dx%d",
                    sensorID, jpegSize.w, jpegSize.h);
        MUINT32 thumbnailWidth = 0;
        MUINT32 thumbnailHeight = 0;
        if (jpegSize.w != 0 && jpegSize.h != 0) {
            double picAspectRatio = (double)jpegSize.w/(double)jpegSize.h;
            for (uint32_t index = 0; index < entryAvaliableSize.count(); index++) {
                MSize size = entryAvaliableSize.itemAt(index, Type2Type<MSize>());
                   MY_LOGD("thumbnail index: %u, size: wxh: %dx%d",index, size.w, size.h);
                if ( size.h == 0) {
                    continue;
                }
                double thumbnailAspectRatio = (double)size.w/(double)size.h;
                double dis = ABS(picAspectRatio, thumbnailAspectRatio);
                if (dis > ASPECT_TOLERANCE) {
                    continue;
                }
                if ((uint32_t)size.w > thumbnailWidth) {
                    thumbnailWidth = size.w;
                    thumbnailHeight = size.h;
                }
            }
            if ((0 != thumbnailWidth) && (0 != thumbnailHeight)) {
                thumbnailSize.w = thumbnailWidth;
                thumbnailSize.h = thumbnailHeight;
            } else {
                MY_LOGW("Bad thumbnail size: %dx%d", thumbnailSize.w, thumbnailSize.h);
                return OK;
            }
        } else {
            MY_LOGW("Bad jpegSize: %dx%d",  jpegSize.w, jpegSize.h);
            return OK;
        }
    } else {
        thumbnailSize = entryThumbnailSize.itemAt(0, Type2Type<MSize>());
        if ( !thumbnailSize )
        {
            MY_LOGW("Bad thumbnail size: %dx%d",  thumbnailSize.w, thumbnailSize.h);
            return OK;
        }
    }
    MY_LOGD_IF( 1, "thumbnail size from metadata: %dx%d", thumbnailSize.w, thumbnailSize.h);
    //
    IMetadata::IEntry const& entryJpegOrientation = pMetadata->entryFor(MTK_JPEG_ORIENTATION);
    if  ( entryJpegOrientation.isEmpty() ) {
        MY_LOGW("No tag: MTK_JPEG_ORIENTATION");
        return OK;
    }
    //
    MSize const yuvthumbnailsize = calcThumbnailYuvSize(
                                        pCfgJpegYUV->getImgSize(),
                                        thumbnailSize
                                        );
    //
    MINT32  jpegOrientation = 0;
    MUINT32 jpegTransform   = 0;
    MSize   thunmbSize      = yuvthumbnailsize; // default thumbnail size
    //
    MINT const format = pCfgThumbYUV->getImgFormat();
    //
    ImageBufferInfo imageBufferInfo;
    createBufPlanes_ThumbnailYuv(imageBufferInfo.bufPlanes, format, yuvthumbnailsize);
    imageBufferInfo.count     = 1;
    imageBufferInfo.bufStep   = 0;
    imageBufferInfo.startOffset = 0;
    imageBufferInfo.imgFormat = format;
    imageBufferInfo.imgWidth  = thunmbSize.w;
    imageBufferInfo.imgHeight = thunmbSize.h;

    android::sp<IImageStreamInfo> pStreamInfo =
        ImageStreamInfoBuilder(pCfgThumbYUV.get())
        .setAllocImgFormat(format)
        .setAllocBufPlanes(imageBufferInfo.bufPlanes)
        .setImageBufferInfo(imageBufferInfo)
        .setTransform(jpegTransform)
        .build();

    if  ( CC_UNLIKELY( ! pStreamInfo.get() ) ) {
        MY_LOGE( "fail to new thumbnail ImageStreamInfo: %s %#" PRIx64,
                 pCfgThumbYUV->getStreamName(), pCfgThumbYUV->getStreamId() );
        return NO_MEMORY;
    }

    *out.pHalImage_Thumbnail_YUV = pStreamInfo;

    MY_LOGD_IF( 1, "streamId:%#" PRIx64 " name(%s) req(%p) cfg(%p) yuvthumbnailsize:%dx%d jpegOrientation:%d",
        pStreamInfo->getStreamId(),
        pStreamInfo->getStreamName(),
        pStreamInfo.get(), pCfgThumbYUV.get(),
        thunmbSize.w, thunmbSize.h, jpegOrientation
    );

    return OK;
}


static auto createRotationStreamInfoLocked_Main_YUV(
    RequestOutputParams& out __unused,
    RequestInputParams const& in __unused
) -> int
{
    auto const pMetadata    = in.pRequest_AppControl;
    auto const pCfgMainYUV  = *(in.pConfiguration_HalImage_Jpeg_YUV);
    if ( CC_UNLIKELY( ! pMetadata || ! pCfgMainYUV.get() ) ) {
        MY_LOGE("invlaid input params");
        return NO_INIT;
    }

    IMetadata::IEntry const& entryJpegOrientation = pMetadata->entryFor(MTK_JPEG_ORIENTATION);
    if  ( entryJpegOrientation.isEmpty() ) {
        MY_LOGW("No tag: MTK_JPEG_ORIENTATION");
        return -EINVAL;
    }

    int32_t jpegFlip = 0;
    IMetadata::IEntry const& entryJpegFlip = pMetadata->entryFor(MTK_CONTROL_CAPTURE_JPEG_FLIP_MODE);
    if (entryJpegFlip.isEmpty()) {
        MY_LOGD("No tag: MTK_CONTROL_CAPTURE_JPEG_FLIP_MODE");
    } else {
        jpegFlip = entryJpegFlip.itemAt(0, Type2Type<MINT32>());
    }

    int32_t jpegFlipProp = ::property_get_int32("vendor.debug.camera.Jpeg.flip", 0);

    int32_t const jpegOrientation = (in.isSupportJpegPack) ? 0 :
                                    entryJpegOrientation.itemAt(0, Type2Type<MINT32>());
    uint32_t      reqTransform   = 0;

    if (jpegFlip || jpegFlipProp) {
        if ( 0==jpegOrientation )
            reqTransform = eTransform_FLIP_H;
        else if ( 90==jpegOrientation )
            reqTransform = eTransform_ROT_90 | eTransform_FLIP_V;
        else if ( 180==jpegOrientation )
            reqTransform = eTransform_FLIP_V;
        else if ( 270==jpegOrientation )
            reqTransform = eTransform_ROT_90 | eTransform_FLIP_H;
        else
            MY_LOGW("Invalid Jpeg Orientation value: %d", jpegOrientation);
    } else {
        if ( 0==jpegOrientation )
            reqTransform = 0;
        else if ( 90==jpegOrientation )
            reqTransform = eTransform_ROT_90;
        else if ( 180==jpegOrientation )
            reqTransform = eTransform_ROT_180;
        else if ( 270==jpegOrientation )
            reqTransform = eTransform_ROT_270;
        else
            MY_LOGW("Invalid Jpeg Orientation value: %d", jpegOrientation);
    }

    uint32_t const cfgTransform   = pCfgMainYUV->getTransform();
    MY_LOGD_IF( 1, "Jpeg orientation metadata: %d degrees; transform request(%d) & config(%d) & flip(%d) pack(%d)",
                jpegOrientation, reqTransform, cfgTransform, jpegFlip, in.isSupportJpegPack);
    if ( reqTransform == cfgTransform ) {
        *out.pHalImage_Jpeg_YUV      = nullptr;
        if(out.pHalImage_Jpeg_Sub_YUV != nullptr)
            *out.pHalImage_Jpeg_Sub_YUV      = nullptr;
        return OK;
    }

    MSize size, cfgSize(pCfgMainYUV->getImgSize().w, pCfgMainYUV->getImgSize().h);
    if ( reqTransform&eTransform_ROT_90 ) {
        size.w = std::min(cfgSize.w, cfgSize.h);
        size.h = std::max(cfgSize.w, cfgSize.h);
    } else
        size = cfgSize;

    sp<IImageStreamInfo>
        pStreamInfo = createImageStreamInfo_JpegYuv(
                            pCfgMainYUV,
                            pCfgMainYUV->getImgFormat(),
                            size, reqTransform
                        );
    if ( CC_UNLIKELY( ! pStreamInfo.get() ) ) {
        MY_LOGE( "fail to new ImageStreamInfo: %s %#" PRIx64,
                 pCfgMainYUV->getStreamName(), pCfgMainYUV->getStreamId() );
        return NO_MEMORY;
    }
    // [Jpeg pack]
    if(in.pConfiguration_HalImage_Jpeg_Sub_YUV != nullptr)
    {
        auto const pCfgSubYUV   = *(in.pConfiguration_HalImage_Jpeg_Sub_YUV);
        if(pCfgSubYUV != nullptr)
        {
            // pack jpeg image will contain same propert to MainYuv.
            sp<IImageStreamInfo>
                pStreamInfo = createImageStreamInfo_JpegYuv(
                                    pCfgSubYUV,
                                    pCfgSubYUV->getImgFormat(),
                                    size, reqTransform
                                );
            if ( CC_UNLIKELY( ! pStreamInfo.get() ) ) {
                MY_LOGE( "fail to new ImageStreamInfo: %s %#" PRIx64,
                         pCfgSubYUV->getStreamName(), pCfgSubYUV->getStreamId() );
                return NO_MEMORY;
            }
            *out.pHalImage_Jpeg_Sub_YUV = pStreamInfo;
        }
    }

    *out.pHalImage_Jpeg_YUV = pStreamInfo;

    MY_LOGD_IF( 1, "streamId:%#" PRIx64 " name(%s) req(%p) cfg(%p) yuvsize(%dx%d) jpegOrientation(%d)",
                pStreamInfo->getStreamId(), pStreamInfo->getStreamName(),
                pStreamInfo.get(), pCfgMainYUV.get(),
                pStreamInfo->getImgSize().w, pStreamInfo->getImgSize().h, jpegOrientation );

    return OK;
}


/**
 * Make a function target as a policy - default version
 */
FunctionType_CaptureStreamUpdaterPolicy makePolicy_CaptureStreamUpdater_Default()
{
    return [](
        RequestOutputParams& out __unused,
        RequestInputParams const& in __unused
    ) -> int
    {
        int err = createStreamInfoLocked_Thumbnail_YUV(out, in);
        if ( CC_UNLIKELY(OK != err) ) {
            MY_LOGW("err:%d(%s)", err, ::strerror(-err));
        }

        if ( ! in.isJpegRotationSupported ) {
            *out.pHalImage_Jpeg_YUV = nullptr;
        } else {
            err = createRotationStreamInfoLocked_Main_YUV(out, in);
        }

        return err;
    };
}


};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam

