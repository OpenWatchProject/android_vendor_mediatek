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

#define LOG_TAG "MtkCam/Utils"
//
#include "MyUtils.h"
#include <mtkcam/utils/fwk/MtkCameraParameters.h>
//
using namespace android;


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


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace Utils {
namespace Format {


/******************************************************************************
 *  String-Enum Format Map Manager
 ******************************************************************************/
struct  StrEnumMapMgr
{
public:     ////            Data Members.
    typedef DefaultKeyedVector<String8, EImageFormat>   MapType;
    MapType                 mvMapper;
    mutable Mutex           mLock;

public:     ////            Instantiation.
                            StrEnumMapMgr()
                                : mvMapper(eImgFmt_UNKNOWN)
                                , mLock()
                            {
                                mvMapper.clear();
                                Mutex::Autolock _l(mLock);
                                //
#define DO_MAP(_format_, _eImageFormat) \
            do { \
                mvMapper.add(String8(MtkCameraParameters::PIXEL_FORMAT_##_format_), eImgFmt_##_eImageFormat); \
            } while (0)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            DO_MAP(YUV420SP         , NV21);
                            DO_MAP(YUV420SP_NV12    , NV12);
                            DO_MAP(YUV420P          , YV12);
                            DO_MAP(YV12_GPU         , YV12);
                            DO_MAP(YUV420I          , I420);
                            DO_MAP(YUV422SP         , NV16);
                            DO_MAP(YUV422I          , YUY2);
                            DO_MAP(YUV422I_YVYU     , YVYU);
                            DO_MAP(YUV422I_UYVY     , UYVY);
                            DO_MAP(YUV422I_VYUY     , VYUY);
                            DO_MAP(RGB565           , RGB565);
                            DO_MAP(RGBA8888         , RGBA8888);
                            DO_MAP(BAYER8           , BAYER8);
                            DO_MAP(BAYER10          , BAYER10);
                            DO_MAP(JPEG             , JPEG);
//------------------------------------------------------------------------------
#undef  DO_MAP
                            }
};
static  StrEnumMapMgr       gStrEnumMap;


/*****************************************************************************
 * @brief Query the image format constant.
 *
 * @details Given a MtkCameraParameters::PIXEL_FORMAT_xxx, return its
 * corresponding image format constant.
 *
 * @note
 *
 * @param[in] szPixelFormat: A null-terminated string for pixel format (i.e.
 * MtkCameraParameters::PIXEL_FORMAT_xxx)
 *
 * @return its corresponding image format.
 *
 ******************************************************************************/
int
queryImageFormat(
    char const* szPixelFormat
)
{
    MY_LOGF_IF(NULL==szPixelFormat, "NULL==szPixelFormat");
    String8 const s8Format(szPixelFormat);
    int format = gStrEnumMap.mvMapper.valueFor(s8Format);
    MY_LOGW_IF(
        eImgFmt_UNKNOWN==format,
        "Unsupported MtkCameraParameters::PIXEL_FORMAT_xxx (%s)", szPixelFormat
    );
    return  format;
}


};  // namespace Format
};  // namespace Utils
};  // namespace NSCam

