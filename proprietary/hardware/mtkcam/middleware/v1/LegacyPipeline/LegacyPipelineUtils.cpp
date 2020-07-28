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

#define LOG_TAG "MtkCam/LegacyPipelineUtils"

#include "MyUtils.h"
#include <math.h>
#include <mtkcam/middleware/v1/LegacyPipeline/LegacyPipelineUtils.h>
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
namespace NSCam {
namespace v1 {
namespace NSLegacyPipeline {

void
queryUFOStride(
    MINT const imgFormat,
    NSCam::MSize const imgSize,
    size_t stride[3]
)
{
    HwInfoHelper helper(0); //query UFO stride no need to fill real sensor id
    helper.queryUFOStride(imgFormat,imgSize,stride);
    MY_LOGD("fmt(%d), size(%dx%d), ret stride(%d, %d, %d)",
        imgFormat, imgSize.w, imgSize.h, stride[0], stride[1], stride[2]);
}


/******************************************************************************
 *
 ******************************************************************************/
sp<IImageStreamInfo>
createImageStreamInfo(
    char const*         streamName,
    StreamId_T          streamId,
    MUINT32             streamType,
    size_t              maxBufNum,
    size_t              minInitBufNum,
    MUINT               usageForAllocator,
    MINT                imgFormat,
    MSize const&        imgSize,
    MUINT32             transform,
    MSize const&        align,
    MBOOL               isReusable,
    size_t              reusableSizeInBytes,
    MBOOL               secure
)
{
    IImageStreamInfo::BufPlanes_t bufPlanes;
    MSize alignSize = imgSize;
    if( imgSize.w > 0 && imgSize.h > 0 )
    {
        alignSize.w = ceil((double)imgSize.w/align.w)*align.w;
        alignSize.h = ceil((double)imgSize.h/align.h)*align.h;
    }
    MY_LOGI("in(%d,%d), align(%d,%d), out(%d,%d)", imgSize.w, imgSize.h, align.w, align.h, alignSize.w, alignSize.h);

#define addBufPlane(planes, height, stride)                                                                               \
        do{                                                                                                               \
            size_t _planeIndex = planes.size();                                                                           \
            size_t _height = (size_t)(height);                                                                            \
            size_t _stride = (size_t)(stride);                                                                            \
            size_t _sizeInBytes = (size_t)( _height * _stride );                                                          \
            size_t _reusableSizeInBytes = (reusableSizeInBytes > _sizeInBytes) ? reusableSizeInBytes : _sizeInBytes;      \
                                                                                                                          \
            size_t ori_height = alignSize.h;                                                                              \
            size_t ori_width  = alignSize.w;                                                                              \
            size_t _longLength = (ori_height > ori_width) ? ori_height : ori_width;                                       \
            size_t _shortLength = (ori_height > ori_width) ? ori_width : ori_height;                                      \
                                                                                                                          \
            if (isReusable &&                                                                                             \
                imgFormat != eImgFmt_BLOB &&                                                                              \
                imgFormat != eImgFmt_RGBA8888 ){                                                                          \
                size_t _maxSizeInBytes = _reusableSizeInBytes;                                                            \
                if (_planeIndex == 0) {                                                                                   \
                    _maxSizeInBytes = (size_t)( _shortLength * (_longLength << 1) );                                      \
                }                                                                                                         \
                else {                                                                                                    \
                    _maxSizeInBytes = (size_t)( _shortLength * (_longLength >> 1) );                                      \
                }                                                                                                         \
                _maxSizeInBytes = (_maxSizeInBytes > _sizeInBytes) ? _maxSizeInBytes : _sizeInBytes;                      \
                _reusableSizeInBytes = (_reusableSizeInBytes > _maxSizeInBytes) ? _reusableSizeInBytes : _maxSizeInBytes; \
                                                                                                                          \
                MY_LOGD("use max buffer heap for pipeline buffer reuse, (w:%zu, h:%zu), (_maxSizeInBytes:%zu)",           \
                            ori_width, ori_height, _maxSizeInBytes);                                                      \
            }                                                                                                             \
                                                                                                                          \
            IImageStreamInfo::BufPlane bufPlane= { _sizeInBytes, _stride, _reusableSizeInBytes};                          \
            planes.push_back(bufPlane);                                                                                   \
            MY_LOGD("bufPlanes[%zu](sizeInBytes:%zu, stride:%zu, reusableSizeInBytes:%zu)",                               \
                    _planeIndex, bufPlane.sizeInBytes, bufPlane.rowStrideInBytes, bufPlane.reusableSizeInBytes);          \
        }while(0)

    switch( imgFormat ) {
        case eImgFmt_YV12:
            addBufPlane(bufPlanes , alignSize.h      , alignSize.w);
            addBufPlane(bufPlanes , alignSize.h >> 1 , alignSize.w >> 1);
            addBufPlane(bufPlanes , alignSize.h >> 1 , alignSize.w >> 1);
            break;
        case eImgFmt_I422:
            addBufPlane(bufPlanes , alignSize.h      , alignSize.w);
            addBufPlane(bufPlanes , alignSize.h      , alignSize.w >> 1);
            addBufPlane(bufPlanes , alignSize.h      , alignSize.w >> 1);
            break;
        case eImgFmt_I420:
            addBufPlane(bufPlanes , alignSize.h      , alignSize.w);
            addBufPlane(bufPlanes , alignSize.h >> 1 , alignSize.w >> 1);
            addBufPlane(bufPlanes , alignSize.h >> 1 , alignSize.w >> 1);
            break;
        case eImgFmt_NV21:
        case eImgFmt_NV12:
            addBufPlane(bufPlanes , alignSize.h      , alignSize.w);
            addBufPlane(bufPlanes , alignSize.h >> 1 , alignSize.w);
            if (isReusable) { // dummy for reuse by others format
                addBufPlane(bufPlanes , alignSize.h, alignSize.w >> 1);
            }
            break;
        case eImgFmt_YUY2:
            addBufPlane(bufPlanes , alignSize.h      , alignSize.w << 1);
            if (isReusable) { // dummy for reuse by others format
                addBufPlane(bufPlanes , alignSize.h, alignSize.w >> 1);
                addBufPlane(bufPlanes , alignSize.h, alignSize.w >> 1);
            }
            break;
        case eImgFmt_BLOB:
            /*
            add 328448 for image size
            standard exif: 1280 bytes
            4 APPn for debug exif: 0xFF80*4 = 65408*4 bytes
            max thumbnail size: 64K bytes
            */
            addBufPlane(bufPlanes , 1 , (imgSize.w * imgSize.h * 12 / 10) + 328448); //328448 = 64K+1280+65408*4
            break;
        case eImgFmt_RGBA8888:
            addBufPlane(bufPlanes , alignSize.h, alignSize.w << 2);
            break;
        case eImgFmt_Y8:
            addBufPlane(bufPlanes , imgSize.h      , imgSize.w);
            break;
        case eImgFmt_STA_BYTE:
            addBufPlane(bufPlanes , imgSize.h      , imgSize.w);
            break;
        default:
            MY_LOGE("format not support yet %d", imgFormat);
            break;
    }
#undef  addBufPlane

    sp<ImageStreamInfo>
        pStreamInfo = new ImageStreamInfo(
                streamName,
                streamId,
                streamType,
                maxBufNum, minInitBufNum,
                usageForAllocator, imgFormat, imgSize, bufPlanes, transform, 0/*dataSpace*/, secure
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
sp<IImageStreamInfo>
createRawImageStreamInfo(
    char const*         streamName,
    StreamId_T          streamId,
    MUINT32             streamType,
    size_t              maxBufNum,
    size_t              minInitBufNum,
    MUINT               usageForAllocator,
    MINT                imgFormat,
    MSize const&        imgSize,
    size_t const        stride,
    MBOOL               secure
)
{
    IImageStreamInfo::BufPlanes_t bufPlanes;
    //
#define addBufPlane(planes, height, stride)                                      \
        do{                                                                      \
            size_t _height = (size_t)(height);                                   \
            size_t _stride = (size_t)(stride);                                   \
            IImageStreamInfo::BufPlane bufPlane= { _height * _stride, _stride }; \
            planes.push_back(bufPlane);                                          \
        }while(0)
    switch( imgFormat ) {
        case eImgFmt_BAYER10:
        case eImgFmt_BAYER10_UNPAK:
        case eImgFmt_FG_BAYER10:
        case eImgFmt_BAYER8: // LCSO
        case eImgFmt_BAYER12:
        case eImgFmt_STA_BYTE:
        case eImgFmt_STA_2BYTE: // LCSO with LCE3.0
        case eImgFmt_FG_BAYER12:
        case eImgFmt_YUY2:
        case eImgFmt_UYVY:
        case eImgFmt_VYUY:
        case eImgFmt_YVYU:
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

    sp<ImageStreamInfo>
        pStreamInfo = new ImageStreamInfo(
                streamName,
                streamId,
                streamType,
                maxBufNum, minInitBufNum,
                usageForAllocator, imgFormat, imgSize, bufPlanes, 0/*transform*/, 0/*dataSpace*/, secure
                );

    if( pStreamInfo == NULL ) {
        MY_LOGE("create ImageStream failed, %s, %#" PRIx64,
                streamName, streamId);
    }

    return pStreamInfo;
}

}; //namespace NSLegacyPipeline
}; //namespace v1
}; //namespace NSCam
