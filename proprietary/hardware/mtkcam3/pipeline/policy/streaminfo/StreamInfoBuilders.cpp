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

#define LOG_TAG "mtkcam-StreamInfoBuilders"

#include "StreamInfoBuilders.h"
//
#include <mtkcam/utils/hw/HwInfoHelper.h>
//
#include "MyUtils.h"


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE  ("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA  ("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF  ("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if (            (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if (            (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if (            (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam::v3::pipeline::policy {


/******************************************************************************
 *
 ******************************************************************************/
auto
DefaultYuvImageStreamInfoBuilder::
build() const -> android::sp<IImageStreamInfo>
{
    auto& self = *const_cast<DefaultYuvImageStreamInfoBuilder*>(this);

    auto imgFormat = getImgFormat();
    auto imgSize   = getImgSize();

    IImageStreamInfo::BufPlanes_t bufPlanes;
    #define addBufPlane(idx, planes, height, stride)                             \
        do{                                                                      \
            size_t _height = (size_t)(height);                                   \
            size_t _stride = (size_t)(stride);                                   \
            planes[idx] = { _height * _stride, _stride };                        \
        }while(0)

    size_t w = imgSize.w >= 0 ? imgSize.w : 0;
    size_t h = imgSize.h >= 0 ? imgSize.h : 0;
    switch( imgFormat ) {
        case eImgFmt_YV12:
            bufPlanes.count = 3;
            addBufPlane(0, bufPlanes.planes, h     , w);
            addBufPlane(1, bufPlanes.planes, h >> 1, w >> 1);
            addBufPlane(2, bufPlanes.planes, h >> 1, w >> 1);
            break;
        case eImgFmt_NV21:
            bufPlanes.count = 2;
            addBufPlane(0, bufPlanes.planes, h     , w);
            addBufPlane(1, bufPlanes.planes, h >> 1, w);
            break;
        case eImgFmt_YUY2:
            bufPlanes.count = 1;
            addBufPlane(0, bufPlanes.planes, h     , w << 1);
            break;
        case eImgFmt_Y16:
            bufPlanes.count = 1;
            addBufPlane(0, bufPlanes.planes, h     , w << 1);
            break;
        case eImgFmt_Y8:
            bufPlanes.count = 1;
            addBufPlane(0, bufPlanes.planes, h     , w);
            break;
        default:
            MY_LOGE("unsupported format:%#x", imgFormat);
            break;
    }
   #undef  addBufPlane

    self.setAllocBufPlanes(bufPlanes);
    self.setBufPlanes(bufPlanes);
    self.setBufCount(1);
    self.setStartOffset(0);
    self.setBufStep(0);

    return ImageStreamInfoBuilder::build();
}


/******************************************************************************
 *
 ******************************************************************************/
auto
JpegYuvImageStreamInfoBuilder::
build() const -> android::sp<IImageStreamInfo>
{
    auto& self = *const_cast<JpegYuvImageStreamInfoBuilder*>(this);

    auto imgFormat = getImgFormat();
    auto imgSize   = getImgSize();

    IImageStreamInfo::BufPlanes_t bufPlanes;
    bool ret = IHwInfoHelperManager::get()->getDefaultBufPlanes_JpegYuv(bufPlanes, imgFormat, imgSize);
    MY_LOGF_IF(!ret, "getDefaultBufPlanes_JpegYuv - format:%#x %dx%d", imgFormat, imgSize.w, imgSize.h);

    self.setAllocBufPlanes(bufPlanes);
    self.setBufPlanes(bufPlanes);
    self.setBufCount(1);
    self.setStartOffset(0);
    self.setBufStep(0);

    return ImageStreamInfoBuilder::build();
}


};  //namespace NSCam::v3::pipeline::policy

