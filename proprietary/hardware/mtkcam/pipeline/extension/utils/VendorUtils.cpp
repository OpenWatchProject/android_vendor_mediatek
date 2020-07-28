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

#define LOG_TAG "MtkCam/vmgr_utils"

#include "../MyUtils.h"
#include <mtkcam/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>

// EIS
#include <mtkcam/feature/eis/eis_ext.h>
#include <mtkcam/utils/hw/HwInfoHelper.h>

using namespace android;
using namespace NSCamHW;

/******************************************************************************
*
*******************************************************************************/
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
//
#define FUNC_START                  MY_LOGD("+")
#define FUNC_END                    MY_LOGD("-")
//
#define FUNCTION_SCOPE              auto __scope_logger__ = create_scope_logger(__FUNCTION__)
#include <memory>
static std::shared_ptr<char> create_scope_logger(const char* functionName)
{
    char* pText = const_cast<char*>(functionName);
    CAM_LOGD("[%s] + ", pText);
    return std::shared_ptr<char>(pText, [](char* p){ CAM_LOGD("[%s] -", p); });
}

/**
 *  The confidence threshold of GMV, which means if the confidence is not
 *  enough then the GMV won't be applied.
 */
#define MFC_GMV_CONFX_TH 25
#define MFC_GMV_CONFY_TH 25

using namespace NSCamHW;

namespace NSCam {
namespace plugin {

/******************************************************************************
 *
 ******************************************************************************/
sp<v3::IImageStreamInfo>
createImageStreamInfo(
    char const*         streamName,
    v3::StreamId_T      streamId,
    MUINT32             streamType,
    size_t              maxBufNum,
    size_t              minInitBufNum,
    MUINT               usageForAllocator,
    MINT                imgFormat,
    MSize const&        imgSize,
    MUINT32             transform
)
{
    v3::IImageStreamInfo::BufPlanes_t bufPlanes;
#define addBufPlane(planes, height, stride)                                          \
        do{                                                                          \
            size_t _height = (size_t)(height);                                       \
            size_t _stride = (size_t)(stride);                                       \
            v3::IImageStreamInfo::BufPlane bufPlane= { _height * _stride, _stride }; \
            planes.push_back(bufPlane);                                              \
        }while(0)
    switch( imgFormat ) {
        case eImgFmt_YV12:
            addBufPlane(bufPlanes , imgSize.h      , imgSize.w);
            addBufPlane(bufPlanes , imgSize.h >> 1 , imgSize.w >> 1);
            addBufPlane(bufPlanes , imgSize.h >> 1 , imgSize.w >> 1);
            break;
        case eImgFmt_I420:
        case eImgFmt_I422:
            addBufPlane(bufPlanes , imgSize.h      , imgSize.w);
            addBufPlane(bufPlanes , imgSize.h      , imgSize.w >> 1);
            addBufPlane(bufPlanes , imgSize.h      , imgSize.w >> 1);
            break;
        case eImgFmt_NV21:
            addBufPlane(bufPlanes , imgSize.h      , imgSize.w);
            addBufPlane(bufPlanes , imgSize.h >> 1 , imgSize.w);
            break;
        case eImgFmt_YUY2:
            addBufPlane(bufPlanes , imgSize.h      , imgSize.w << 1);
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
        case eImgFmt_Y8:
            addBufPlane(bufPlanes , 1              , imgSize.w);
            break;
        case eImgFmt_RGBA8888:
            addBufPlane(bufPlanes , imgSize.h      , imgSize.w<<2);
            break;
        default:
            MY_LOGE("format not support yet %d", imgFormat);
            break;
    }
#undef  addBufPlane

    sp<v3::Utils::ImageStreamInfo>
        pStreamInfo = new v3::Utils::ImageStreamInfo(
                streamName,
                streamId,
                streamType,
                maxBufNum, minInitBufNum,
                usageForAllocator, imgFormat, imgSize, bufPlanes, transform
                );

    if( pStreamInfo == NULL ) {
        MY_LOGE("create ImageStream failed, %s, %#" PRIx64,
                streamName, streamId);
    }

    return pStreamInfo;
}

/******************************************************************************
 *
 ******************************************************************************/
 void
 queryUFOStride(
    MINT const imgFormat,
    NSCam::MSize const imgSize,
    size_t stride[3]
)
{
    HwInfoHelper helper(0); //query UFO stride no need to fill real sensor id
    helper.queryUFOStride(imgFormat,imgSize,stride);
    MY_LOGD("fmt(%d), size(%dx%d), ret stride(%d, %d, %d)",imgFormat,imgSize.w,imgSize.h,stride[0], stride[1], stride[2]);
}

/******************************************************************************
 *
 ******************************************************************************/
sp<v3::IImageStreamInfo>
createRawImageStreamInfo(
    char const*         streamName,
    v3::StreamId_T      streamId,
    MUINT32             streamType,
    size_t              maxBufNum,
    size_t              minInitBufNum,
    MUINT               usageForAllocator,
    MINT                imgFormat,
    MSize const&        imgSize,
    size_t const        stride
)
{
    v3::IImageStreamInfo::BufPlanes_t bufPlanes;
    //
#define addBufPlane(planes, height, stride)                                          \
        do{                                                                          \
            size_t _height = (size_t)(height);                                       \
            size_t _stride = (size_t)(stride);                                       \
            v3::IImageStreamInfo::BufPlane bufPlane= { _height * _stride, _stride }; \
            planes.push_back(bufPlane);                                              \
        }while(0)
    switch( imgFormat ) {
        case eImgFmt_BAYER10:
        case eImgFmt_BAYER10_UNPAK:
        case eImgFmt_FG_BAYER10:
        case eImgFmt_BAYER8: // LCSO
        case eImgFmt_BAYER12:
        case eImgFmt_STA_2BYTE: // LCSO with LCE3.0
        case eImgFmt_FG_BAYER12:
            addBufPlane(bufPlanes , imgSize.h, stride);
            break;
        case eImgFmt_UFO_BAYER8:
        case eImgFmt_UFO_BAYER10:
        case eImgFmt_UFO_BAYER12:
        case eImgFmt_UFO_BAYER14:
        case eImgFmt_UFO_FG_BAYER8:
        case eImgFmt_UFO_FG_BAYER10:
        case eImgFmt_UFO_FG_BAYER12:
        case eImgFmt_UFO_FG_BAYER14:
            {
                size_t ufoStride[3];
                queryUFOStride(imgFormat, imgSize, ufoStride);
                addBufPlane(bufPlanes , imgSize.h, ufoStride[0]);
                addBufPlane(bufPlanes , imgSize.h, ufoStride[1]);
                addBufPlane(bufPlanes , imgSize.h, ufoStride[2]);
                break;
            }
        default:
            MY_LOGE("format not support yet %d", imgFormat);
            break;
    }
#undef  addBufPlane

    sp<v3::Utils::ImageStreamInfo>
        pStreamInfo = new v3::Utils::ImageStreamInfo(
                streamName,
                streamId,
                streamType,
                maxBufNum, minInitBufNum,
                usageForAllocator, imgFormat, imgSize, bufPlanes
                );

    if( pStreamInfo == NULL ) {
        MY_LOGE("create ImageStream failed, %s, %#" PRIx64,
                streamName, streamId);
    }

    return pStreamInfo;
}

MBOOL
retrieveScalerCropRgn(const IMetadata* pAppMetadata, const IMetadata* pHalMetadata, MRect& outRgn)
{
    if (!tryGetMetadata<MRect>(pHalMetadata, MTK_P1NODE_SENSOR_CROP_REGION, outRgn)) {
        MY_LOGW("try to query MTK_SCALER_CROP_REGION from App metadata...");
        if (!tryGetMetadata<MRect>(pAppMetadata, MTK_SCALER_CROP_REGION, outRgn)) {
            MY_LOGE("query MTK_SCALER_CROP_REGION from App metadata failed !");
            return MFALSE;
        }
    }

    return MTRUE;
}

MBOOL
retrieveScalerCropRgn(const IMetadata* pAppMetadata, MRect& outRgn)
{
    if (!tryGetMetadata<MRect>(pAppMetadata, MTK_SCALER_CROP_REGION, outRgn))
        return MFALSE;
    else
        return MTRUE;
}


MBOOL
retrieveGmvInfo(const IMetadata* pMetadata, int& x, int& y, MSize& size)
{
    MBOOL  ret = MTRUE;
    MSize  rzoSize;
    MUINT  entryCount = 0;
    IMetadata::IEntry entry;
    struct __confidence{
        MINT32 x;
        MINT32 y;
        __confidence() : x(0), y(0) {}
    } confidence;

    /* get size first */
    ret = tryGetMetadata(pMetadata, MTK_P1NODE_RESIZER_SIZE, rzoSize);
    if (ret != MTRUE) {
        MY_LOGE("%s: cannot get rzo size", __FUNCTION__);
        goto lbExit;
    }

    entry = pMetadata->entryFor(MTK_EIS_REGION);

    /* check if a valid EIS_REGION */
    entryCount = entry.count();
    if (EIS_REGION_INDEX_CONFX >= entryCount || EIS_REGION_INDEX_CONFY >= entryCount ||
        EIS_REGION_INDEX_GMVX >= entryCount || EIS_REGION_INDEX_GMVY >= entryCount)
    {
        MY_LOGE("%s: entry is not a valid EIS_REGION, size = %d",
                __FUNCTION__,
                entry.count());
        ret = MFALSE;
        goto lbExit;
    }

    /* read confidence */
    confidence.x = static_cast<MINT32>(entry.itemAt(EIS_REGION_INDEX_CONFX, Type2Type<MINT32>()));
    confidence.y = static_cast<MINT32>((MINT32)entry.itemAt(EIS_REGION_INDEX_CONFY, Type2Type<MINT32>()));

    /* to read GMV if confidence is enough */
    if (confidence.x > MFC_GMV_CONFX_TH) {
        x = entry.itemAt(EIS_REGION_INDEX_GMVX, Type2Type<MINT32>());
    }

    if (confidence.y > MFC_GMV_CONFY_TH) {
        y = entry.itemAt(EIS_REGION_INDEX_GMVY, Type2Type<MINT32>());
    }

    size = rzoSize;

    MY_LOGD("EIS info conf(x,y) = (%d, %d), gmv(x, y) = (%d, %d)",
            confidence.x, confidence.y, x, y);

lbExit:
    return ret;
}

MRect calCropRegin(const IMetadata* pAppMetadata, const IMetadata* pHalMetadata, MSize dstSize, int openId)
{
    // not indicate transform region size as input source buffer
    MSize srcSize = MSize(0, 0);
    return calCropRegin(pAppMetadata, pHalMetadata, srcSize, dstSize, openId);
}

MRect calCropRegin(const IMetadata* pAppMetadata, const IMetadata* pHalMetadata, MSize srcSize, MSize dstSize, int openId)
{
    // query active array size
    MY_LOGD("openId : %d", openId);

    sp<Cropper::CropInfo> pCropInfo = new Cropper::CropInfo;
    if (OK != Cropper::getCropInfo(pAppMetadata, pHalMetadata, false, *pCropInfo, openId)) {
        MY_LOGE("getCropInfo failed");
    }

    NSIoPipe::MCropRect cropRect;
    cropRect.p_fractional = {0, 0};
    cropRect.p_integral = {0, 0};
    cropRect.s = dstSize;

    Cropper::calcViewAngle(MTRUE, *pCropInfo, srcSize, dstSize, cropRect);

    MRect crop;
    crop.p = cropRect.p_integral;
    crop.s = cropRect.s;

    MY_LOGD("crop(x,y,w,h)=(%d,%d,%d,%d)",
            crop.p.x, crop.p.y,
            crop.s.w, crop.s.h);

    return crop;
}

MRect calCropRegin(const IMetadata* pAppMetadata, MSize dstSize, int openId)
{
    // query active array size
    MY_LOGD("openId : %d", openId);
    MRect activeArray;
    sp<IMetadataProvider> pMetadataProvider = NSMetadataProviderManager::valueFor(openId);
    IMetadata static_meta = pMetadataProvider->getMtkStaticCharacteristics();
    if (!IMetadata::getEntry<MRect>(
                &static_meta, MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION, activeArray)) {
        MY_LOGE("no active array size.");
        return activeArray;
    }

    MRect origCropRegion;
    if (!retrieveScalerCropRgn(pAppMetadata, origCropRegion)) {
        // set crop region to full size
        origCropRegion.p = MPoint(0, 0);
        origCropRegion.s = activeArray.s;
        MY_LOGW("no MTK_SCALER_CROP_REGION, set crop region to full size %dx%d",
        origCropRegion.s.w, origCropRegion.s.h);
    }

    simpleTransform tranActive2Sensor =
        simpleTransform(MPoint(0, 0), activeArray.s, dstSize);

    // apply transform
    MRect crop = transform(tranActive2Sensor, origCropRegion);

    return crop;
}

MRect calCrop(const MRect& rSrc, const MRect& rDst)
{
    #define ROUND_TO_2X(x) ((x) & (~0x1))
    MRect rCrop;

    // srcW/srcH < dstW/dstH
    if (rSrc.s.w * rDst.s.h < rDst.s.w * rSrc.s.h) {
        rCrop.s.w = rSrc.s.w;
        rCrop.s.h = rSrc.s.w * rDst.s.h / rDst.s.w;
    }
    // srcW/srcH > dstW/dstH
    else if (rSrc.s.w * rDst.s.h > rDst.s.w * rSrc.s.h) {
        rCrop.s.w = rSrc.s.h * rDst.s.w / rDst.s.h;
        rCrop.s.h = rSrc.s.h;
    }
    // srcW/srcH == dstW/dstH
    else {
        rCrop.s.w = rSrc.s.w;
        rCrop.s.h = rSrc.s.h;
    }

    rCrop.s.w =  ROUND_TO_2X(rCrop.s.w);
    rCrop.s.h =  ROUND_TO_2X(rCrop.s.h);

    rCrop.p.x = (rSrc.s.w - rCrop.s.w) / 2;
    rCrop.p.y = (rSrc.s.h - rCrop.s.h) / 2;

    rCrop.p.x += ROUND_TO_2X(rSrc.p.x);
    rCrop.p.y += ROUND_TO_2X(rSrc.p.y);

    #undef ROUND_TO_2X
    return rCrop;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
Cropper::
getCropInfo(
        const IMetadata* inApp,
        const IMetadata* inHal,
        MBOOL const isResized,
        CropInfo &cropInfo,
        int openId)
{
    if (!tryGetMetadata<MSize>(inHal, MTK_HAL_REQUEST_SENSOR_SIZE, cropInfo.sensor_size)) {
        MY_LOGE("cannot get MTK_HAL_REQUEST_SENSOR_SIZE");
        return BAD_VALUE;
    }

    sp<IMetadataProvider> pMetadataProvider = NSMetadataProviderManager::valueFor(openId);
    IMetadata static_meta = pMetadataProvider->getMtkStaticCharacteristics();

    if (tryGetMetadata<MRect>(&static_meta, MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION, cropInfo.activeArray)) {
        MY_LOGD("active array(%d, %d, %dx%d)",
                cropInfo.activeArray.p.x, cropInfo.activeArray.p.y,
                cropInfo.activeArray.s.w, cropInfo.activeArray.s.h);
    } else {
        MY_LOGE("no static info: MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION");
        return BAD_VALUE;
    }

    MSize const sensor = cropInfo.sensor_size;
    MSize const active = cropInfo.activeArray.s;

    cropInfo.isResized = isResized;
    // get current p1 buffer crop status
    if (!(tryGetMetadata<MRect>(inHal, MTK_P1NODE_SCALAR_CROP_REGION, cropInfo.crop_p1_sensor) &&
          tryGetMetadata<MSize>(inHal, MTK_P1NODE_RESIZER_SIZE, cropInfo.dstsize_resizer) &&
          tryGetMetadata<MRect>(inHal, MTK_P1NODE_DMA_CROP_REGION, cropInfo.crop_dma)))
    {
        MY_LOGW_IF(1, "[FIXME] should sync with p1 for rrz setting");

        cropInfo.crop_p1_sensor = MRect(MPoint(0, 0), sensor);
        cropInfo.dstsize_resizer = sensor;
        cropInfo.crop_dma = MRect(MPoint(0, 0), sensor);
    }

    MY_LOGD("SCALAR_CROP_REGION:(%d,%d)(%dx%d) RESIZER_SIZE:(%dx%d) DMA_CROP_REGION:(%d,%d)(%dx%d)",
           cropInfo.crop_p1_sensor.p.x, cropInfo.crop_p1_sensor.p.y,
           cropInfo.crop_p1_sensor.s.w, cropInfo.crop_p1_sensor.s.h,
           cropInfo.dstsize_resizer.w, cropInfo.dstsize_resizer.h,
           cropInfo.crop_dma.p.x, cropInfo.crop_dma.p.y,
           cropInfo.crop_dma.s.w, cropInfo.crop_dma.s.h);

    MINT32 sensorMode;
    if (!tryGetMetadata<MINT32>(inHal, MTK_P1NODE_SENSOR_MODE, sensorMode)) {
        MY_LOGE("cannot get MTK_P1NODE_SENSOR_MODE");
        return BAD_VALUE;
    }

    HwTransHelper hwTransHelper(openId);
    HwMatrix matToActive;
    if (!hwTransHelper.getMatrixToActive(sensorMode, cropInfo.matSensor2Active) ||
        !hwTransHelper.getMatrixFromActive(sensorMode, cropInfo.matActive2Sensor)) {
        MY_LOGE("get matrix fail");
        return UNKNOWN_ERROR;
    }

    cropInfo.tranSensor2Resized = simpleTransform(
            cropInfo.crop_p1_sensor.p,
            cropInfo.crop_p1_sensor.s,
            cropInfo.dstsize_resizer
    );

    cropInfo.isEisEabled = MFALSE;

    MRect cropRegion; //active array domain
    queryCropRegion(inApp, inHal, cropInfo.isEisEabled, cropRegion, openId);
    cropInfo.crop_a = cropRegion;

    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
Cropper::
queryCropRegion(
        const IMetadata* inApp,
        const IMetadata* inHal,
        MBOOL const isEisOn,
        MRect &cropRegion,
        int openId)
{
    if (!tryGetMetadata<MRect>(inApp, MTK_SCALER_CROP_REGION, cropRegion)) {
        sp<IMetadataProvider> pMetadataProvider = NSMetadataProviderManager::valueFor(openId);
        IMetadata static_meta = pMetadataProvider->getMtkStaticCharacteristics();
        MRect activeArray;
        if (tryGetMetadata<MRect>(&static_meta, MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION, activeArray)) {
            MY_LOGD("active array(%d, %d, %dx%d)",
                    activeArray.p.x, activeArray.p.y, activeArray.s.w, activeArray.s.h);
        }
        cropRegion.p = MPoint(0, 0);
        cropRegion.s = activeArray.s;
        MY_LOGW("no MTK_SCALER_CROP_REGION, crop full size %dx%d",
                   cropRegion.s.w, cropRegion.s.h);
    }
    MY_LOGD("control: cropRegion(%d, %d, %dx%d)",
               cropRegion.p.x, cropRegion.p.y, cropRegion.s.w, cropRegion.s.h);

#if SUPPORT_EIS
    if (isEisOn) {
        MINT64 eisPackedInfo = 0;
        if (!tryGetMetadata<MINT64>(inHal, MTK_EIS_INFO, eisPackedInfo)) {
            MY_LOGE("cannot get MTK_EIS_INFO, current eisPackedInfo = %" PRIi64 " ", eisPackedInfo);
        }
        MUINT32 eis_factor = EisInfo::getFactor(eisPackedInfo);
        cropRegion.p.x += (cropRegion.s.w * (eis_factor - 100) / 2 / eis_factor);
        cropRegion.p.y += (cropRegion.s.h * (eis_factor - 100) / 2 / eis_factor);
        cropRegion.s = cropRegion.s * 100 / eis_factor;
        MY_LOGD_IF(mbEnableLog, "EIS: factor %d, cropRegion(%d, %d, %dx%d)",
                   eis_factor, cropRegion.p.x, cropRegion.p.y, cropRegion.s.w, cropRegion.s.h);
    }
#endif
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
Cropper::
calcBufferCrop(
        MSize const &srcSize,
        MSize const &dstSize,
        MRect &viewCrop)
{
    // pillarbox
    if (srcSize.w * dstSize.h > srcSize.h * dstSize.w) {
        viewCrop.s.w = div_round(srcSize.h * dstSize.w, dstSize.h);
        viewCrop.s.h = srcSize.h;
        viewCrop.p.x = ((srcSize.w - viewCrop.s.w) >> 1);
        viewCrop.p.y = 0;
    }
    // letterbox
    else {
        viewCrop.s.w = srcSize.w;
        viewCrop.s.h = div_round(srcSize.w * dstSize.h, dstSize.w);
        viewCrop.p.x = 0;
        viewCrop.p.y = ((srcSize.h - viewCrop.s.h) >> 1);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
Cropper::
calcViewAngle(
        MBOOL bEnableLog,
        CropInfo const &cropInfo,
        MSize const &dstSize,
        NSIoPipe::MCropRect &result)
{
    // not indicate transform region size as input source buffer
    MSize srcSize = MSize(0, 0);
    return calcViewAngle(bEnableLog, cropInfo, srcSize, dstSize, result);
}

MVOID
Cropper::
calcViewAngle(
        MBOOL bEnableLog,
        CropInfo const &cropInfo,
        MSize const &srcSize,
        MSize const &dstSize,
        NSIoPipe::MCropRect &result)
{
    MBOOL const isResized = cropInfo.isResized;

    MRect s_crop;
    if ((srcSize.h > 0) && (srcSize.w > 0)) {
        MY_LOGD("Indicated source size as transform region, activeArray(%dx%d), srcSize(%dx%d)",
                cropInfo.activeArray.s.w, cropInfo.activeArray.s.h,
                srcSize.w, srcSize.h);

        // setup transform (active arrary -> source buffer)
        simpleTransform tranActive2Source =
            simpleTransform(MPoint(0, 0), cropInfo.activeArray.s, srcSize);

        // apply transform
        s_crop = transform(tranActive2Source, cropInfo.crop_a);
    }
    else {
        // setup transform (active arrary -> sensor)
        cropInfo.matActive2Sensor.transform(cropInfo.crop_a, s_crop);
    }

    MRect s_viewcrop;
    // pillarbox
    if (s_crop.s.w * dstSize.h > s_crop.s.h * dstSize.w) {
        s_viewcrop.s.w = div_round(s_crop.s.h * dstSize.w, dstSize.h);
        s_viewcrop.s.h = s_crop.s.h;
        s_viewcrop.p.x = s_crop.p.x + ((s_crop.s.w - s_viewcrop.s.w) >> 1);
        s_viewcrop.p.y = s_crop.p.y;
    }
    // letterbox
    else {
        s_viewcrop.s.w = s_crop.s.w;
        s_viewcrop.s.h = div_round(s_crop.s.w * dstSize.h, dstSize.w);
        s_viewcrop.p.x = s_crop.p.x;
        s_viewcrop.p.y = s_crop.p.y + ((s_crop.s.h - s_viewcrop.s.h) >> 1);
    }
    MY_LOGD_IF(bEnableLog, "s_cropRegion(%d, %d, %dx%d), dst %dx%d, view crop(%d, %d, %dx%d)",
           s_crop.p.x, s_crop.p.y,
           s_crop.s.w, s_crop.s.h,
           dstSize.w, dstSize.h,
           s_viewcrop.p.x, s_viewcrop.p.y,
           s_viewcrop.s.w, s_viewcrop.s.h
    );

    if (isResized) {
        MRect r_viewcrop = transform(cropInfo.tranSensor2Resized, s_viewcrop);
        result.s = r_viewcrop.s;
        result.p_integral = r_viewcrop.p + cropInfo.eis_mv_r.p;
        result.p_fractional = cropInfo.eis_mv_r.pf;

        // make sure hw limitation
        result.s.w &= ~(0x1);
        result.s.h &= ~(0x1);

        // check boundary
        if (refineBoundary(cropInfo.dstsize_resizer, result)) {
            MY_LOGE("[FIXME] need to check crop!");
            Cropper::dump(cropInfo);
        }
    }
    else {
        result.s = s_viewcrop.s;
        result.p_integral = s_viewcrop.p + cropInfo.eis_mv_s.p;
        result.p_fractional = cropInfo.eis_mv_s.pf;

        // make sure hw limitation
        result.s.w &= ~(0x1);
        result.s.h &= ~(0x1);

        // check boundary
        if (refineBoundary(cropInfo.sensor_size, result)) {
            MY_LOGE("[FIXME] need to check crop!");
            Cropper::dump(cropInfo);
        }
    }

    MY_LOGD_IF(bEnableLog, "resized %d, crop %d/%d, %d/%d, %dx%d",
           isResized,
           result.p_integral.x,
           result.p_integral.y,
           result.p_fractional.x,
           result.p_fractional.y,
           result.s.w,
           result.s.h
    );
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
Cropper::
refineBoundary(
        MSize const &bufSize,
        NSIoPipe::MCropRect &crop)
{
    MBOOL isRefined = MFALSE;
    NSIoPipe::MCropRect refined = crop;
    if (crop.p_integral.x < 0) {
        refined.p_integral.x = 0;
        isRefined = MTRUE;
    }
    if (crop.p_integral.y < 0) {
        refined.p_integral.y = 0;
        isRefined = MTRUE;
    }

    int const carry_x = (crop.p_fractional.x != 0) ? 1 : 0;
    if ((refined.p_integral.x + crop.s.w + carry_x) > bufSize.w) {
        refined.s.w = bufSize.w - refined.p_integral.x - carry_x;
        isRefined = MTRUE;
    }
    int const carry_y = (crop.p_fractional.y != 0) ? 1 : 0;
    if ((refined.p_integral.y + crop.s.h + carry_y) > bufSize.h) {
        refined.s.h = bufSize.h - refined.p_integral.y - carry_y;
        isRefined = MTRUE;
    }

    if (isRefined) {
        // make sure hw limitation
        refined.s.w &= ~(0x1);
        refined.s.h &= ~(0x1);

        MY_LOGE("buffer size:%dx%d, crop(%d/%d, %d/%d, %dx%d) -> refined crop(%d/%d, %d/%d, %dx%d)",
                bufSize.w, bufSize.h,
                crop.p_integral.x,
                crop.p_integral.y,
                crop.p_fractional.x,
                crop.p_fractional.y,
                crop.s.w,
                crop.s.h,
                refined.p_integral.x,
                refined.p_integral.y,
                refined.p_fractional.x,
                refined.p_fractional.y,
                refined.s.w,
                refined.s.h
        );
        crop = refined;
    }
    return isRefined;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
Cropper::
dump(CropInfo const &crop) {
    MY_LOGD("isResized %d", crop.isResized);
    MY_LOGD("p1 sensro crop(%d,%d,%dx%d), resizer size(%dx%d), crop dma(%d,%d,%dx%d)",
            crop.crop_p1_sensor.p.x,
            crop.crop_p1_sensor.p.y,
            crop.crop_p1_sensor.s.w,
            crop.crop_p1_sensor.s.h,
            crop.dstsize_resizer.w,
            crop.dstsize_resizer.h,
            crop.crop_dma.p.x,
            crop.crop_dma.p.y,
            crop.crop_dma.s.w,
            crop.crop_dma.s.h
    );
/*
    MY_LOGD("tran active to sensor o %d, %d, s %dx%d -> %dx%d",
        crop.tranActive2Sensor.tarOrigin.x,
        crop.tranActive2Sensor.tarOrigin.y,
        crop.tranActive2Sensor.oldScale.w,
        crop.tranActive2Sensor.oldScale.h,
        crop.tranActive2Sensor.newScale.w,
        crop.tranActive2Sensor.newScale.h
    );
*/
    MY_LOGD("tran sensor to resized o %d, %d, s %dx%d -> %dx%d",
            crop.tranSensor2Resized.tarOrigin.x,
            crop.tranSensor2Resized.tarOrigin.y,
            crop.tranSensor2Resized.oldScale.w,
            crop.tranSensor2Resized.oldScale.h,
            crop.tranSensor2Resized.newScale.w,
            crop.tranSensor2Resized.newScale.h
    );
    MY_LOGD("modified active crop %d, %d, %dx%d",
            crop.crop_a.p.x,
            crop.crop_a.p.y,
            crop.crop_a.s.w,
            crop.crop_a.s.h
    );
    MY_LOGD("isEisOn %d", crop.isEisEabled);
    MY_LOGD("mv in active %d/%d, %d/%d",
            crop.eis_mv_a.p.x, crop.eis_mv_a.pf.x,
            crop.eis_mv_a.p.y, crop.eis_mv_a.pf.y
    );
    MY_LOGD("mv in sensor %d/%d, %d/%d",
            crop.eis_mv_s.p.x, crop.eis_mv_s.pf.x,
            crop.eis_mv_s.p.y, crop.eis_mv_s.pf.y
    );
    MY_LOGD("mv in resized %d/%d, %d/%d",
            crop.eis_mv_r.p.x, crop.eis_mv_r.pf.x,
            crop.eis_mv_r.p.y, crop.eis_mv_r.pf.y
    );
}
}; //namespace plugin
}; //namespace NSCam
