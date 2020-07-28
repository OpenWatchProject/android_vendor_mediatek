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
 *  Image Format Interface
 ******************************************************************************/
class IImgFmt : public RefBase
{
public:     ////            Instantiation.
    virtual                 ~IImgFmt() {}

public:     ////            Attributes.
    virtual char const*     getName() const                                 = 0;
    virtual size_t          getPlaneCount() const                           = 0;
    virtual size_t          getPlaneWidthInPixels(size_t planeIndex, size_t widthInPixels)  const   = 0;
    virtual size_t          getPlaneHeightInPixels(size_t planeIndex, size_t heightInPixels)const   = 0;
    virtual int             getPlaneBitsPerPixel(size_t planeIndex) const   = 0;
    virtual int             getImageBitsPerPixel() const                    = 0;
    virtual bool            checkValidBufferInfo() const                    = 0;
};


/******************************************************************************
 *  Image Format Base
 ******************************************************************************/
class CBaseImgFmt : public IImgFmt
{
protected:  ////            Data Members.
    String8                 ms8About;           //  "About" this class
    String8                 ms8Name;
    EImageFormat            meImageFormat;
    size_t                  mPlaneCount;
    int                     mImageBitsPerPixel;
    int                     mPlaneBitsPerPixel[3];

public:     ////            Instantiation.
                            CBaseImgFmt(
                                char const*     szAbout,
                                char const*     szName,
                                EImageFormat    eImageFormat,
                                size_t          planeCount,
                                int const       imageBitsPerPixel,
                                int const       planeBitsPerPixel0,
                                int const       planeBitsPerPixel1 = 0,
                                int const       planeBitsPerPixel2 = 0
                            )
                                : ms8About(String8(szAbout))
                                , ms8Name(String8(szName))
                                , meImageFormat(eImageFormat)
                                , mPlaneCount(planeCount)
                                , mImageBitsPerPixel(imageBitsPerPixel)
                            {
                                mPlaneBitsPerPixel[0] = planeBitsPerPixel0;
                                mPlaneBitsPerPixel[1] = planeBitsPerPixel1;
                                mPlaneBitsPerPixel[2] = planeBitsPerPixel2;
                            }

public:     ////            Attributes.
    virtual char const*     getName() const                                     { return ms8Name.string(); }
    virtual size_t          getPlaneCount() const                               { return mPlaneCount; }
    virtual int             getPlaneBitsPerPixel(size_t planeIndex) const       { return mPlaneBitsPerPixel[planeIndex]; }
    virtual int             getImageBitsPerPixel() const                        { return mImageBitsPerPixel; }
    virtual bool            checkValidBufferInfo() const                        { return true; }
};


/******************************************************************************
 *  Image Format Class (1 Plane)
 ******************************************************************************/
class CBaseImgFmt_1Plane : public CBaseImgFmt
{
public:     ////            Instantiation.
                            CBaseImgFmt_1Plane(
                                char const*     szAbout,
                                char const*     szName,
                                EImageFormat    eImageFormat,
                                int const       imageBitsPerPixel
                            )
                                : CBaseImgFmt(szAbout, szName, eImageFormat, 1, imageBitsPerPixel, imageBitsPerPixel)
                            {
                            }
public:     ////            Attributes.
    virtual size_t          getPlaneWidthInPixels(size_t /*planeIndex*/, size_t widthInPixels)  const   { return widthInPixels; }
    virtual size_t          getPlaneHeightInPixels(size_t /*planeIndex*/, size_t heightInPixels)const   { return heightInPixels; }
};


#define DECLARE_FORMAT_1PLANE(_eImageFormat, _bitsPerPixel, _about) \
    struct CImgFmt##_eImageFormat : public CBaseImgFmt_1Plane \
    { \
        CImgFmt##_eImageFormat() \
            :   CBaseImgFmt_1Plane(\
                    _about, #_eImageFormat, eImgFmt_##_eImageFormat, \
                    _bitsPerPixel \
                ) \
        {} \
    }

//
//DECLARE_FORMAT_xxxx(IMPLEMENTATION_DEFINED,  0, "Implementation Defined");
//DECLARE_FORMAT_xxxx(UNKNOWN               ,  0, 0, "Unknown");
//
DECLARE_FORMAT_1PLANE(YUY2                  , 16, "YUY2, 1 plane (YUYV), 422i");
DECLARE_FORMAT_1PLANE(YVYU                  , 16, "YVYU, 1 plane (YVYU), 422");
DECLARE_FORMAT_1PLANE(UYVY                  , 16, "UYVY, 1 plane (UYVY), 422");
DECLARE_FORMAT_1PLANE(VYUY                  , 16, "VYUY, 1 plane (VYUY), 422");

DECLARE_FORMAT_1PLANE(YUYV_Y210             , 32, "YUYV, 1 plane (YUYV_Y210), 422");
DECLARE_FORMAT_1PLANE(YVYU_Y210             , 32, "YVYU, 1 plane (YVYU_Y210), 422");
DECLARE_FORMAT_1PLANE(UYVY_Y210             , 32, "UYVY, 1 plane (UYVY_Y210), 422");
DECLARE_FORMAT_1PLANE(VYUY_Y210             , 32, "VYUY, 1 plane (VYUY_Y210), 422");
DECLARE_FORMAT_1PLANE(MTK_YUYV_Y210         , 20, "YUYV, 1 plane (MTK_YUYV_Y210), 422");
DECLARE_FORMAT_1PLANE(MTK_YVYU_Y210         , 20, "YVYU, 1 plane (MTK_YVYU_Y210), 422");
DECLARE_FORMAT_1PLANE(MTK_UYVY_Y210         , 20, "UYVY, 1 plane (MTK_UYVY_Y210), 422");
DECLARE_FORMAT_1PLANE(MTK_VYUY_Y210         , 20, "VYUY, 1 plane (MTK_VYUY_Y210), 422");

DECLARE_FORMAT_1PLANE(Y16                   , 16, "16-bit Y plane, 1 plane");
DECLARE_FORMAT_1PLANE(Y8                    ,  8, " 8-bit Y plane, 1 plane");

DECLARE_FORMAT_1PLANE(RGB565                , 16, "RGB565, 1 plane");
DECLARE_FORMAT_1PLANE(RGB888                , 24, "RGB888, 1 plane, LSB:R, MSB:B");
DECLARE_FORMAT_1PLANE(RGBA8888              , 32, "RGBA8888, 1 plane, LSB:R, MSB:A");
DECLARE_FORMAT_1PLANE(RGBX8888              , 32, "RGBX8888, 1 plane, LSB:R, MSB:X");
DECLARE_FORMAT_1PLANE(BGRA8888              , 32, "BGRA8888, 1 plane, LSB:B, MSB:A");
DECLARE_FORMAT_1PLANE(ARGB8888              , 32, "ARGB8888, 1 plane, A:LSB, B:MSB");
DECLARE_FORMAT_1PLANE(RGB48                 , 48, "RGB48, 1 plane, LSB:R, MSB:B");

DECLARE_FORMAT_1PLANE(BLOB                  ,  8, "BLOB (width=size, height=1)");

DECLARE_FORMAT_1PLANE(JPEG                  ,  8, "JPEG");


DECLARE_FORMAT_1PLANE(BAYER8                ,  8, "Bayer8");
DECLARE_FORMAT_1PLANE(BAYER10               , 10, "Bayer10");
DECLARE_FORMAT_1PLANE(BAYER12               , 12, "Bayer12");
DECLARE_FORMAT_1PLANE(BAYER14               , 14, "Bayer14");


DECLARE_FORMAT_1PLANE(BAYER8_UNPAK          , 16, "Bayer8 unpak, 16bits");
DECLARE_FORMAT_1PLANE(BAYER10_UNPAK         , 16, "Bayer10 unpak, 16bits");
DECLARE_FORMAT_1PLANE(BAYER12_UNPAK         , 16, "Bayer12 unpak, 16bits");
DECLARE_FORMAT_1PLANE(BAYER14_UNPAK         , 16, "Bayer14 unpak, 16bits");
DECLARE_FORMAT_1PLANE(BAYER15_UNPAK         , 16, "Bayer15 unpak, 16bits");
DECLARE_FORMAT_1PLANE(BAYER16_APPLY_LSC     , 16, "Bayer16 apply LSC, 16-bit");

//DECLARE_FORMAT_1PLANE(RAW_SENSOR            , 16, "RAW SENSOR");

DECLARE_FORMAT_1PLANE(RAW16                    , 16, "RAW16");
DECLARE_FORMAT_1PLANE(FG_BAYER8             ,  8*3/2, "FG_Bayer8");
DECLARE_FORMAT_1PLANE(FG_BAYER10            , 10*3/2, "FG_Bayer10");
DECLARE_FORMAT_1PLANE(FG_BAYER12            , 12*3/2, "FG_Bayer12");
DECLARE_FORMAT_1PLANE(FG_BAYER14            , 14*3/2, "FG_Bayer14");

DECLARE_FORMAT_1PLANE(STA_BYTE              ,  8, "statistic format, 8-bit");
DECLARE_FORMAT_1PLANE(STA_2BYTE             , 16, "statistic format, 16-bit");
DECLARE_FORMAT_1PLANE(STA_4BYTE             , 32, "statistic format, 32-bit");
DECLARE_FORMAT_1PLANE(STA_10BIT             , 10, "statistic format, 10-bit");
DECLARE_FORMAT_1PLANE(STA_12BIT             , 12, "statistic format, 12-bit");
DECLARE_FORMAT_1PLANE(CAMERA_OPAQUE         , 10, "Opaque Reprocessing");
DECLARE_FORMAT_1PLANE(UFO_FG                ,  8, "ufo length table, 8-bit");
DECLARE_FORMAT_1PLANE(BAYER10_MIPI          , 10, "Bayer10 Mipi");

/******************************************************************************
 *  Image Format Class (YUV420 2/3 Plane, 12-bit)
 ******************************************************************************/
class CBaseImgFmt_YUV420_2_3_Plane_12Bits : public CBaseImgFmt
{
public:     ////            Instantiation.
                            CBaseImgFmt_YUV420_2_3_Plane_12Bits(
                                char const*     szAbout,
                                char const*     szName,
                                EImageFormat    eImageFormat,
                                size_t          planeCount,
                                int const       planeBitsPerPixel0,
                                int const       planeBitsPerPixel1,
                                int const       planeBitsPerPixel2 = 0
                            )
                                :   CBaseImgFmt(
                                        szAbout, szName, eImageFormat, planeCount,
                                        12, planeBitsPerPixel0, planeBitsPerPixel1, planeBitsPerPixel2
                                    )
                            {
                            }
public:     ////            Attributes.
    virtual size_t          getPlaneWidthInPixels(size_t planeIndex, size_t widthInPixels)  const
                            {
                                return  (0 == planeIndex)
                                            ? (widthInPixels)
                                            : (widthInPixels >> 1)
                                            ;
                            }

    virtual size_t          getPlaneHeightInPixels(size_t planeIndex, size_t heightInPixels)const
                            {
                                return  (0 == planeIndex)
                                            ? (heightInPixels)
                                            : (heightInPixels >> 1)
                                            ;
                            }
};

#define DECLARE_FORMAT_YUV420_2PLANE_12BITS(_eImageFormat, _planeBitsPerPixel0, _planeBitsPerPixel1, _about) \
    struct CImgFmt##_eImageFormat : public CBaseImgFmt_YUV420_2_3_Plane_12Bits \
    { \
        CImgFmt##_eImageFormat() \
            :   CBaseImgFmt_YUV420_2_3_Plane_12Bits(\
                    _about, #_eImageFormat, eImgFmt_##_eImageFormat, \
                    2, _planeBitsPerPixel0, _planeBitsPerPixel1 \
                ) \
        {} \
    }

#define DECLARE_FORMAT_YUV420_3PLANE_12BITS(_eImageFormat, _planeBitsPerPixel0, _planeBitsPerPixel1, _planeBitsPerPixel2, _about) \
    struct CImgFmt##_eImageFormat : public CBaseImgFmt_YUV420_2_3_Plane_12Bits \
    { \
        CImgFmt##_eImageFormat() \
            :   CBaseImgFmt_YUV420_2_3_Plane_12Bits(\
                    _about, #_eImageFormat, eImgFmt_##_eImageFormat, \
                    3, _planeBitsPerPixel0, _planeBitsPerPixel1, _planeBitsPerPixel2 \
                ) \
        {} \
    }

DECLARE_FORMAT_YUV420_2PLANE_12BITS(NV21    ,  8, 16,       "NV21, 2 plane (Y)(VU), 420sp");
DECLARE_FORMAT_YUV420_2PLANE_12BITS(NV12    ,  8, 16,       "NV12, 2 plane (Y)(UV), 420");
DECLARE_FORMAT_YUV420_2PLANE_12BITS(NV21_BLK,  8, 16,       "NV21 block, 2 plane (Y)(VU), 420");
DECLARE_FORMAT_YUV420_2PLANE_12BITS(NV12_BLK,  8, 16,       "NV12 block, 2 plane (Y)(UV), 420");
//
DECLARE_FORMAT_YUV420_3PLANE_12BITS(YV12    ,  8,  8,  8,   "YV12, 3 plane (Y)(V)(U), 420p");
DECLARE_FORMAT_YUV420_3PLANE_12BITS(I420    ,  8,  8,  8,   "I420, 3 plane (Y)(U)(V)");


/******************************************************************************
 *  Image Format Class (YUV420 2/3 Plane, 24-bit)
 ******************************************************************************/
class CBaseImgFmt_YUV420_2_3_Plane_24Bits : public CBaseImgFmt
{
public:     ////            Instantiation.
                            CBaseImgFmt_YUV420_2_3_Plane_24Bits(
                                char const*     szAbout,
                                char const*     szName,
                                EImageFormat    eImageFormat,
                                size_t          planeCount,
                                int const       planeBitsPerPixel0,
                                int const       planeBitsPerPixel1,
                                int const       planeBitsPerPixel2 = 0
                            )
                                :   CBaseImgFmt(
                                        szAbout, szName, eImageFormat, planeCount,
                                        24, planeBitsPerPixel0, planeBitsPerPixel1, planeBitsPerPixel2
                                    )
                            {
                            }
public:     ////            Attributes.
    virtual size_t          getPlaneWidthInPixels(size_t planeIndex, size_t widthInPixels)  const
                            {
                                return  (0 == planeIndex)
                                            ? (widthInPixels)
                                            : (widthInPixels >> 1)
                                            ;
                            }

    virtual size_t          getPlaneHeightInPixels(size_t planeIndex, size_t heightInPixels)const
                            {
                                return  (0 == planeIndex)
                                            ? (heightInPixels)
                                            : (heightInPixels >> 1)
                                            ;
                            }
};


#define DECLARE_FORMAT_YUV420_2PLANE_24BITS(_eImageFormat, _planeBitsPerPixel0, _planeBitsPerPixel1, _about) \
    struct CImgFmt##_eImageFormat : public CBaseImgFmt_YUV420_2_3_Plane_24Bits \
    { \
        CImgFmt##_eImageFormat() \
            :   CBaseImgFmt_YUV420_2_3_Plane_24Bits(\
                    _about, #_eImageFormat, eImgFmt_##_eImageFormat, \
                    2, _planeBitsPerPixel0, _planeBitsPerPixel1 \
                ) \
        {} \
    }

#define DECLARE_FORMAT_YUV420_3PLANE_24BITS(_eImageFormat, _planeBitsPerPixel0, _planeBitsPerPixel1, _planeBitsPerPixel2, _about) \
    struct CImgFmt##_eImageFormat : public CBaseImgFmt_YUV420_2_3_Plane_24Bits \
    { \
        CImgFmt##_eImageFormat() \
            :   CBaseImgFmt_YUV420_2_3_Plane_24Bits(\
                    _about, #_eImageFormat, eImgFmt_##_eImageFormat, \
                    3, _planeBitsPerPixel0, _planeBitsPerPixel1, _planeBitsPerPixel2 \
                ) \
        {} \
    }

DECLARE_FORMAT_YUV420_2PLANE_24BITS(YUV_P010          ,  16, 32,       "YUV_P010, 2 plane (Y),(UV) = P010");
DECLARE_FORMAT_YUV420_2PLANE_24BITS(YVU_P010          ,  16, 32,       "YVU_P010, 2 plane (Y),(VU)");
DECLARE_FORMAT_YUV420_3PLANE_24BITS(YUV_P010_3PLANE   ,  16, 16, 16,   "YUV_P010_3PLANE, 3 plane (Y),(U),(V)");


/******************************************************************************
 *  Image Format Class (YUV420 2/3 Plane, 15-bit)
 ******************************************************************************/
class CBaseImgFmt_YUV420_2_3_Plane_15Bits : public CBaseImgFmt
{
public:     ////            Instantiation.
                            CBaseImgFmt_YUV420_2_3_Plane_15Bits(
                                char const*     szAbout,
                                char const*     szName,
                                EImageFormat    eImageFormat,
                                size_t          planeCount,
                                int const       planeBitsPerPixel0,
                                int const       planeBitsPerPixel1,
                                int const       planeBitsPerPixel2 = 0
                            )
                                :   CBaseImgFmt(
                                        szAbout, szName, eImageFormat, planeCount,
                                        15, planeBitsPerPixel0, planeBitsPerPixel1, planeBitsPerPixel2
                                    )
                            {
                            }
public:     ////            Attributes.
    virtual size_t          getPlaneWidthInPixels(size_t planeIndex, size_t widthInPixels)  const
                            {
                                return  (0 == planeIndex)
                                            ? (widthInPixels)
                                            : (widthInPixels >> 1)
                                            ;
                            }

    virtual size_t          getPlaneHeightInPixels(size_t planeIndex, size_t heightInPixels)const
                            {
                                return  (0 == planeIndex)
                                            ? (heightInPixels)
                                            : (heightInPixels >> 1)
                                            ;
                            }
};

#define DECLARE_FORMAT_YUV420_2PLANE_15BITS(_eImageFormat, _planeBitsPerPixel0, _planeBitsPerPixel1, _about) \
    struct CImgFmt##_eImageFormat : public CBaseImgFmt_YUV420_2_3_Plane_15Bits \
    { \
        CImgFmt##_eImageFormat() \
            :   CBaseImgFmt_YUV420_2_3_Plane_15Bits(\
                    _about, #_eImageFormat, eImgFmt_##_eImageFormat, \
                    2, _planeBitsPerPixel0, _planeBitsPerPixel1 \
                ) \
        {} \
    }

#define DECLARE_FORMAT_YUV420_3PLANE_15BITS(_eImageFormat, _planeBitsPerPixel0, _planeBitsPerPixel1, _planeBitsPerPixel2, _about) \
    struct CImgFmt##_eImageFormat : public CBaseImgFmt_YUV420_2_3_Plane_15Bits \
    { \
        CImgFmt##_eImageFormat() \
            :   CBaseImgFmt_YUV420_2_3_Plane_15Bits(\
                    _about, #_eImageFormat, eImgFmt_##_eImageFormat, \
                    3, _planeBitsPerPixel0, _planeBitsPerPixel1, _planeBitsPerPixel2 \
                ) \
        {} \
    }

DECLARE_FORMAT_YUV420_2PLANE_15BITS(MTK_YUV_P010          ,  10, 20,       "MTK_YUV_P010, 2 plane (Y),(UV) = P010");
DECLARE_FORMAT_YUV420_2PLANE_15BITS(MTK_YVU_P010          ,  10, 20,       "MTK_YVU_P010, 2 plane (Y),(VU)");
DECLARE_FORMAT_YUV420_3PLANE_15BITS(MTK_YUV_P010_3PLANE   ,  10, 10, 10,   "MTK_YUV_P010_3PLANE, 3 plane (Y),(U),(V)");


/******************************************************************************
 *  Image Format Class (YUV422 2/3 Plane, 20-bit)
 ******************************************************************************/
class CBaseImgFmt_YUV422_2_3_Plane_20Bits : public CBaseImgFmt
{
public:     ////            Instantiation.
                            CBaseImgFmt_YUV422_2_3_Plane_20Bits(
                                char const*     szAbout,
                                char const*     szName,
                                EImageFormat    eImageFormat,
                                size_t          planeCount,
                                int const       planeBitsPerPixel0,
                                int const       planeBitsPerPixel1,
                                int const       planeBitsPerPixel2 = 0
                            )
                                :   CBaseImgFmt(
                                        szAbout, szName, eImageFormat, planeCount,
                                        20, planeBitsPerPixel0, planeBitsPerPixel1, planeBitsPerPixel2
                                    )
                            {
                            }
public:     ////            Attributes.
    virtual size_t          getPlaneWidthInPixels(size_t planeIndex, size_t widthInPixels)  const
                            {
                                return  (0 == planeIndex)
                                            ? (widthInPixels)
                                            : (widthInPixels >> 1)
                                            ;
                            }

    virtual size_t          getPlaneHeightInPixels(size_t /*planeIndex*/, size_t heightInPixels)const
                            {
                                return  (heightInPixels);
                            }
};

#define DECLARE_FORMAT_YUV422_2PLANE_20BITS(_eImageFormat, _planeBitsPerPixel0, _planeBitsPerPixel1, _about) \
    struct CImgFmt##_eImageFormat : public CBaseImgFmt_YUV422_2_3_Plane_20Bits \
    { \
        CImgFmt##_eImageFormat() \
            :   CBaseImgFmt_YUV422_2_3_Plane_20Bits(\
                    _about, #_eImageFormat, eImgFmt_##_eImageFormat, \
                    2, _planeBitsPerPixel0, _planeBitsPerPixel1 \
                ) \
        {} \
    }

#define DECLARE_FORMAT_YUV422_3PLANE_20BITS(_eImageFormat, _planeBitsPerPixel0, _planeBitsPerPixel1, _planeBitsPerPixel2, _about) \
    struct CImgFmt##_eImageFormat : public CBaseImgFmt_YUV422_2_3_Plane_20Bits \
    { \
        CImgFmt##_eImageFormat() \
            :   CBaseImgFmt_YUV422_2_3_Plane_20Bits(\
                    _about, #_eImageFormat, eImgFmt_##_eImageFormat, \
                    3, _planeBitsPerPixel0, _planeBitsPerPixel1, _planeBitsPerPixel2 \
                ) \
        {} \
    }

DECLARE_FORMAT_YUV422_2PLANE_20BITS(MTK_YUV_P210          ,  10, 20,       "MTK_YUV_P210, 2 plane (Y),(UV) = P210");
DECLARE_FORMAT_YUV422_2PLANE_20BITS(MTK_YVU_P210          ,  10, 20,       "MTK_YVU_P210, 2 plane (Y),(VU)");
DECLARE_FORMAT_YUV422_3PLANE_20BITS(MTK_YUV_P210_3PLANE   ,  10, 10, 10,   "MTK_YUV_P210_3PLANE, 3 plane (Y),(U),(V)");


/******************************************************************************
 *  Image Format Class (YUV422 2/3 Plane, 16-bit)
 ******************************************************************************/
class CBaseImgFmt_YUV422_2_3_Plane_16Bits : public CBaseImgFmt
{
public:     ////            Instantiation.
                            CBaseImgFmt_YUV422_2_3_Plane_16Bits(
                                char const*     szAbout,
                                char const*     szName,
                                EImageFormat    eImageFormat,
                                size_t          planeCount,
                                int const       planeBitsPerPixel0,
                                int const       planeBitsPerPixel1,
                                int const       planeBitsPerPixel2 = 0
                            )
                                :   CBaseImgFmt(
                                        szAbout, szName, eImageFormat, planeCount,
                                        16, planeBitsPerPixel0, planeBitsPerPixel1, planeBitsPerPixel2
                                    )
                            {
                            }
public:     ////            Attributes.
    virtual size_t          getPlaneWidthInPixels(size_t planeIndex, size_t widthInPixels)  const
                            {
                                return  (0 == planeIndex)
                                            ? (widthInPixels)
                                            : (widthInPixels >> 1)
                                            ;
                            }

    virtual size_t          getPlaneHeightInPixels(size_t /*planeIndex*/, size_t heightInPixels)const
                            {
                                return  (heightInPixels);
                            }
};

#define DECLARE_FORMAT_YUV422_2PLANE_16BITS(_eImageFormat, _planeBitsPerPixel0, _planeBitsPerPixel1, _about) \
    struct CImgFmt##_eImageFormat : public CBaseImgFmt_YUV422_2_3_Plane_16Bits \
    { \
        CImgFmt##_eImageFormat() \
            :   CBaseImgFmt_YUV422_2_3_Plane_16Bits(\
                    _about, #_eImageFormat, eImgFmt_##_eImageFormat, \
                    2, _planeBitsPerPixel0, _planeBitsPerPixel1 \
                ) \
        {} \
    }

#define DECLARE_FORMAT_YUV422_3PLANE_16BITS(_eImageFormat, _planeBitsPerPixel0, _planeBitsPerPixel1, _planeBitsPerPixel2, _about) \
    struct CImgFmt##_eImageFormat : public CBaseImgFmt_YUV422_2_3_Plane_16Bits \
    { \
        CImgFmt##_eImageFormat() \
            :   CBaseImgFmt_YUV422_2_3_Plane_16Bits(\
                    _about, #_eImageFormat, eImgFmt_##_eImageFormat, \
                    3, _planeBitsPerPixel0, _planeBitsPerPixel1, _planeBitsPerPixel2 \
                ) \
        {} \
    }

DECLARE_FORMAT_YUV422_2PLANE_16BITS(NV61    ,  8, 16,       "NV61, 2 plane (Y)(VU), 422");
DECLARE_FORMAT_YUV422_2PLANE_16BITS(NV16    ,  8, 16,       "NV16, 2 plane (Y)(UV), 422sp");
//
DECLARE_FORMAT_YUV422_3PLANE_16BITS(YV16    ,  8,  8,  8,   "YV16, 3 plane (Y)(V)(U), 422");
//
DECLARE_FORMAT_YUV422_3PLANE_16BITS(I422    ,  8,  8,  8,   "I422, 3 plane (Y)(V)(U), 422");


/******************************************************************************
 *  Image Format Class (YUV422 2/3 Plane, 32-bit)
 ******************************************************************************/
class CBaseImgFmt_YUV422_2_3_Plane_32Bits : public CBaseImgFmt
{
public:     ////            Instantiation.
                            CBaseImgFmt_YUV422_2_3_Plane_32Bits(
                                char const*     szAbout,
                                char const*     szName,
                                EImageFormat    eImageFormat,
                                size_t          planeCount,
                                int const       planeBitsPerPixel0,
                                int const       planeBitsPerPixel1,
                                int const       planeBitsPerPixel2 = 0
                            )
                                :   CBaseImgFmt(
                                        szAbout, szName, eImageFormat, planeCount,
                                        32, planeBitsPerPixel0, planeBitsPerPixel1, planeBitsPerPixel2
                                    )
                            {
                            }
public:     ////            Attributes.
    virtual size_t          getPlaneWidthInPixels(size_t planeIndex, size_t widthInPixels)  const
                            {
                                return  (0 == planeIndex)
                                            ? (widthInPixels)
                                            : (widthInPixels >> 1)
                                            ;
                            }

    virtual size_t          getPlaneHeightInPixels(size_t /*planeIndex*/, size_t heightInPixels)const
                            {
                                return  (heightInPixels);
                            }
};

#define DECLARE_FORMAT_YUV422_2PLANE_32BITS(_eImageFormat, _planeBitsPerPixel0, _planeBitsPerPixel1, _about) \
    struct CImgFmt##_eImageFormat : public CBaseImgFmt_YUV422_2_3_Plane_32Bits \
    { \
        CImgFmt##_eImageFormat() \
            :   CBaseImgFmt_YUV422_2_3_Plane_32Bits(\
                    _about, #_eImageFormat, eImgFmt_##_eImageFormat, \
                    2, _planeBitsPerPixel0, _planeBitsPerPixel1 \
                ) \
        {} \
    }

#define DECLARE_FORMAT_YUV422_3PLANE_32BITS(_eImageFormat, _planeBitsPerPixel0, _planeBitsPerPixel1, _planeBitsPerPixel2, _about) \
    struct CImgFmt##_eImageFormat : public CBaseImgFmt_YUV422_2_3_Plane_32Bits \
    { \
        CImgFmt##_eImageFormat() \
            :   CBaseImgFmt_YUV422_2_3_Plane_32Bits(\
                    _about, #_eImageFormat, eImgFmt_##_eImageFormat, \
                    3, _planeBitsPerPixel0, _planeBitsPerPixel1, _planeBitsPerPixel2 \
                ) \
        {} \
    }

DECLARE_FORMAT_YUV422_2PLANE_32BITS(YUV_P210          ,  16, 32,       "YUV_P210, 2 plane (Y),(UV) = P210");
DECLARE_FORMAT_YUV422_2PLANE_32BITS(YVU_P210          ,  16, 32,       "YVU_P210, 2 plane (Y),(VU)");
DECLARE_FORMAT_YUV422_3PLANE_32BITS(YUV_P210_3PLANE   ,  16, 16, 16,   "YUV_P210_3PLANE, 3 plane (Y),(U),(V)");

/******************************************************************************
 *  Image Format Class (3 Plane, UFEO)
 ******************************************************************************/
class CBaseImgFmt_UFO_3Plane : public CBaseImgFmt
{
public:     ////            Instantiation.
                            CBaseImgFmt_UFO_3Plane(
                                char const*     szAbout,
                                char const*     szName,
                                EImageFormat    eImageFormat,
                                int const       imageBitsPerPixel
                            )
                                : CBaseImgFmt(szAbout, szName, eImageFormat, 3, imageBitsPerPixel, imageBitsPerPixel, imageBitsPerPixel)
                            {
                            }
public:     ////            Attributes.
    virtual size_t          getPlaneWidthInPixels(size_t /*planeIndex*/, size_t widthInPixels)  const   { return widthInPixels; }
    virtual size_t          getPlaneHeightInPixels(size_t /*planeIndex*/, size_t heightInPixels)const   { return heightInPixels; }
    virtual bool            checkValidBufferInfo() const                        { return false; }
};


#define DECLARE_FORMAT_UFO_3PLANE(_eImageFormat, _bitsPerPixel, _about) \
    struct CImgFmt##_eImageFormat : public CBaseImgFmt_UFO_3Plane \
    { \
        CImgFmt##_eImageFormat() \
            :   CBaseImgFmt_UFO_3Plane(\
                    _about, #_eImageFormat, eImgFmt_##_eImageFormat, \
                    _bitsPerPixel \
                ) \
        {} \
    }

DECLARE_FORMAT_UFO_3PLANE(UFO_BAYER8             ,  8*3/2, "UFO_BAYER8");
DECLARE_FORMAT_UFO_3PLANE(UFO_BAYER10            , 10*3/2, "UFO_BAYER10");
DECLARE_FORMAT_UFO_3PLANE(UFO_BAYER12            , 12*3/2, "UFO_BAYER12");
DECLARE_FORMAT_UFO_3PLANE(UFO_BAYER14            , 14*3/2, "UFO_BAYER14");

DECLARE_FORMAT_UFO_3PLANE(UFO_FG_BAYER8             ,  8*3/2, "UFO_FG_BAYER8");
DECLARE_FORMAT_UFO_3PLANE(UFO_FG_BAYER10            , 10*3/2, "UFO_FG_BAYER10");
DECLARE_FORMAT_UFO_3PLANE(UFO_FG_BAYER12            , 12*3/2, "UFO_FG_BAYER12");
DECLARE_FORMAT_UFO_3PLANE(UFO_FG_BAYER14            , 14*3/2, "UFO_FG_BAYER14");

/******************************************************************************
 *  Image Format Class (3 Plane, WARP)
 ******************************************************************************/
class CBaseImgFmt_WARP_3PLANE : public CBaseImgFmt
{
public:     ////            Instantiation.
                            CBaseImgFmt_WARP_3PLANE(
                                char const*     szAbout,
                                char const*     szName,
                                EImageFormat    eImageFormat,
                                size_t          planeCount,
                                int const       imageBitsPerPixel,
                                int const       planeBitsPerPixel0,
                                int const       planeBitsPerPixel1 = 0,
                                int const       planeBitsPerPixel2 = 0
                            )
                                : CBaseImgFmt(szAbout, szName, eImageFormat, planeCount, imageBitsPerPixel, planeBitsPerPixel0,  planeBitsPerPixel1,  planeBitsPerPixel2)
                            {
                            }
public:     ////            Attributes.
    virtual size_t          getPlaneWidthInPixels(size_t /*planeIndex*/, size_t widthInPixels)  const   { return widthInPixels; }
    virtual size_t          getPlaneHeightInPixels(size_t /*planeIndex*/, size_t heightInPixels)const   { return heightInPixels; }
};



#define DECLARE_FORMAT_WARP_1PLANE(_eImageFormat, _imageBitsPerPixel, _planeBitsPerPixel0, _about) \
    struct CImgFmt##_eImageFormat : public CBaseImgFmt_WARP_3PLANE \
    { \
        CImgFmt##_eImageFormat() \
            :   CBaseImgFmt_WARP_3PLANE(\
                    _about, #_eImageFormat, eImgFmt_##_eImageFormat, \
                    1, _imageBitsPerPixel,  _planeBitsPerPixel0 \
                ) \
        {} \
    }

#define DECLARE_FORMAT_WARP_2PLANE(_eImageFormat, _imageBitsPerPixel, _planeBitsPerPixel0, _planeBitsPerPixel1, _about) \
    struct CImgFmt##_eImageFormat : public CBaseImgFmt_WARP_3PLANE \
    { \
        CImgFmt##_eImageFormat() \
            :   CBaseImgFmt_WARP_3PLANE(\
                    _about, #_eImageFormat, eImgFmt_##_eImageFormat, \
                    2, _imageBitsPerPixel,  _planeBitsPerPixel0, _planeBitsPerPixel1 \
                ) \
        {} \
    }

#define DECLARE_FORMAT_WARP_3PLANE(_eImageFormat, _imageBitsPerPixel, _planeBitsPerPixel0, _planeBitsPerPixel1, _planeBitsPerPixel2, _about) \
    struct CImgFmt##_eImageFormat : public CBaseImgFmt_WARP_3PLANE \
    { \
        CImgFmt##_eImageFormat() \
            :   CBaseImgFmt_WARP_3PLANE(\
                    _about, #_eImageFormat, eImgFmt_##_eImageFormat, \
                    3, _imageBitsPerPixel, _planeBitsPerPixel0, _planeBitsPerPixel1, _planeBitsPerPixel2 \
                ) \
        {} \
    }


DECLARE_FORMAT_WARP_1PLANE(WARP_1PLANE, 32, 32,         "WARP_1PLANE");
DECLARE_FORMAT_WARP_2PLANE(WARP_2PLANE, 64, 32, 32,     "WARP_2PLANE");
DECLARE_FORMAT_WARP_3PLANE(WARP_3PLANE, 96, 32, 32, 32, "WARP_3PLANE");


/******************************************************************************
 *  Image Format Map Manager
 ******************************************************************************/
struct MapMgr
{
public:     ////            Data Members.
    typedef DefaultKeyedVector<int, sp<IImgFmt> >  MapType;
    MapType                 mvMapper;
public:     ////            Instantiation.
                            MapMgr()
                                : mvMapper(NULL)
                            {
                                mvMapper.clear();
                                //
#define DO_MAP(_eImageFormat)    \
            do { \
                mvMapper.add(eImgFmt_##_eImageFormat, new CImgFmt##_eImageFormat());\
            } while (0)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//DO_MAP(IMPLEMENTATION_DEFINED);
//DO_MAP(UNKNOWN);
//
DO_MAP(YV12);
DO_MAP(I420);
DO_MAP(NV21);
DO_MAP(NV12);
DO_MAP(NV12_BLK);
DO_MAP(NV21_BLK);
//
DO_MAP(YV16);
DO_MAP(NV61);
DO_MAP(NV16);
DO_MAP(I422);
//
DO_MAP(YUY2);
DO_MAP(YVYU);
DO_MAP(UYVY);
DO_MAP(VYUY);
//
DO_MAP(YUYV_Y210);
DO_MAP(YVYU_Y210);
DO_MAP(UYVY_Y210);
DO_MAP(VYUY_Y210);
DO_MAP(YUV_P210);
DO_MAP(YVU_P210);
DO_MAP(YUV_P210_3PLANE);
DO_MAP(YUV_P010);
DO_MAP(YVU_P010);
DO_MAP(YUV_P010_3PLANE);
DO_MAP(MTK_YUYV_Y210);
DO_MAP(MTK_YVYU_Y210);
DO_MAP(MTK_UYVY_Y210);
DO_MAP(MTK_VYUY_Y210);
DO_MAP(MTK_YUV_P210);
DO_MAP(MTK_YVU_P210);
DO_MAP(MTK_YUV_P210_3PLANE);
DO_MAP(MTK_YUV_P010);
DO_MAP(MTK_YVU_P010);
DO_MAP(MTK_YUV_P010_3PLANE);
//
DO_MAP(Y16);
DO_MAP(Y8);
//
DO_MAP(RGB565);
DO_MAP(RGB888);
DO_MAP(RGBA8888);
DO_MAP(RGBX8888);
DO_MAP(BGRA8888);
DO_MAP(ARGB8888);
DO_MAP(RGB48);
//
DO_MAP(BLOB);
//
DO_MAP(JPEG);
//
DO_MAP(BAYER8);
DO_MAP(BAYER10);
DO_MAP(BAYER12);
DO_MAP(BAYER14);
//
DO_MAP(WARP_1PLANE);
DO_MAP(WARP_2PLANE);
DO_MAP(WARP_3PLANE);
//

//DO_MAP(RAW_SENSOR);
//
DO_MAP(RAW16);
DO_MAP(FG_BAYER8);
DO_MAP(FG_BAYER10);
DO_MAP(FG_BAYER12);
DO_MAP(FG_BAYER14);
//
DO_MAP(UFO_BAYER8);
DO_MAP(UFO_BAYER10);
DO_MAP(UFO_BAYER12);
DO_MAP(UFO_BAYER14);
//
DO_MAP(UFO_FG_BAYER8);
DO_MAP(UFO_FG_BAYER10);
DO_MAP(UFO_FG_BAYER12);
DO_MAP(UFO_FG_BAYER14);
//
DO_MAP(BAYER8_UNPAK);
DO_MAP(BAYER10_UNPAK);
DO_MAP(BAYER12_UNPAK);
DO_MAP(BAYER14_UNPAK);
DO_MAP(BAYER15_UNPAK);
DO_MAP(BAYER16_APPLY_LSC);
//
DO_MAP(STA_BYTE);
DO_MAP(STA_2BYTE);
DO_MAP(STA_4BYTE);
DO_MAP(STA_10BIT);
DO_MAP(STA_12BIT);
DO_MAP(CAMERA_OPAQUE);
DO_MAP(UFO_FG);
DO_MAP(BAYER10_MIPI);
//------------------------------------------------------------------------------
#undef  DO_MAP
                            }
};

static MapMgr mapMgrInstance;

MapMgr&
getMapMgr()
{
    return mapMgrInstance;
}


MapMgr::MapType
getMapper()
{
    return  getMapMgr().mvMapper;
}

bool
checkValidFormat(
    int const imageFormat
)
{
    sp<IImgFmt> p = getMapper().valueFor(imageFormat);
    if  ( p != 0 )
    {
        return true;
    }
    else
    {
        dumpMapInformation();
        MY_LOGW("Unsupported Image Format: %#x", imageFormat);
        return false;
    }
}


/*****************************************************************************
 * @brief Query the name of a specified format.
 *
 * @details Given a format of type EImageFormat, return its readable name.
 *
 * @note
 *
 * @param[in] imageFormat: A format of type EImageFormat.
 *
 * @return the name of image format.
 *
 ******************************************************************************/
std::string
queryImageFormatName(
    int const imageFormat
)
{
    sp<IImgFmt> p = getMapper().valueFor(imageFormat);
    if  ( p != 0 ) {
        return std::string(p->getName());
    }
    //
    dumpMapInformation();
    MY_LOGW("Unsupported Image Format: %#x", imageFormat);
    return std::string("unknown");
}


/*****************************************************************************
 * @brief Query the plane count.
 *
 * @details Given a format of type EImageFormat, return its corresponding
 * plane count.
 *
 * @note
 *
 * @param[in] imageFormat: A format of type EImageFormat.
 *
 * @return its corresponding plane count.
 *
 ******************************************************************************/
size_t
queryPlaneCount(
    int const imageFormat
)
{
    sp<IImgFmt> p = getMapper().valueFor(imageFormat);
    if  ( p != 0 )
    {
        size_t const PlaneCount = p->getPlaneCount();
        MY_LOGV("[%s] plane count %zu", p->getName(), PlaneCount);
        return  PlaneCount;
    }
    //
    dumpMapInformation();
    MY_LOGW("Unsupported Image Format: %#x", imageFormat);
    return  0;
}


/*****************************************************************************
 * @brief Query the width in pixels of a specific plane.
 *
 * @details Given a format of type EImageFormat, a plane index, and the width in
 * in pixels of the 0-th plane, return the width in pixels of the given plane.
 *
 * @note
 *
 * @param[in] imageFormat: A format of type EImageFormat.
 * @param[in] planeIndex: a specific plane index.
 * @param[in] widthInPixels: the width in pixels of the 0-th plane.
 *
 * @return the width in pixels of the given plane.
 *
 ******************************************************************************/
size_t
queryPlaneWidthInPixels(
    int const imageFormat,
    size_t planeIndex,
    size_t widthInPixels
)
{
    sp<IImgFmt> p = getMapper().valueFor(imageFormat);
    if  ( p != 0 )
    {
        size_t const pixels = p->getPlaneWidthInPixels(planeIndex, widthInPixels);
        MY_LOGV("[%s] Width in pixels %zu @ %zu-th plane", p->getName(), pixels, planeIndex);
        return  pixels;
    }
    //
    dumpMapInformation();
    MY_LOGW("Unsupported Image Format: %#x", imageFormat);
    return  0;
}


/*****************************************************************************
 * @brief Query the height in pixels of a specific plane.
 *
 * @details Given a format of type EImageFormat, a plane index, and the height
 * in pixels of the 0-th plane, return the height in pixels of the given plane.
 *
 * @note
 *
 * @param[in] imageFormat: A format of type EImageFormat.
 * @param[in] planeIndex: a specific plane index.
 * @param[in] heightInPixels: the height in pixels of the 0-th plane.
 *
 * @return the height in pixels of the given plane.
 *
 ******************************************************************************/
size_t
queryPlaneHeightInPixels(
    int const imageFormat,
    size_t planeIndex,
    size_t heightInPixels
)
{
    sp<IImgFmt> p = getMapper().valueFor(imageFormat);
    if  ( p != 0 )
    {
        size_t const pixels = p->getPlaneHeightInPixels(planeIndex, heightInPixels);
        MY_LOGV("[%s] Height in pixels %zu @ %zu-th plane", p->getName(), pixels, planeIndex);
        return  pixels;
    }
    //
    dumpMapInformation();
    MY_LOGW("Unsupported Image Format: %#x", imageFormat);
    return  0;
}


/*****************************************************************************
 * @brief Query the bits per pixel of a specific plane.
 *
 * @details Given a format of type EImageFormat and a plane index, return
 * the bits per pixel of the given plane.
 *
 * @note
 *
 * @param[in] imageFormat: A format of type EImageFormat.
 * @param[in] planeIndex: a specific plane index.
 *
 * @return the bits per pixel of the given plane.
 *
 ******************************************************************************/
int
queryPlaneBitsPerPixel(
    int const imageFormat,
    size_t planeIndex
)
{
    sp<IImgFmt> p = getMapper().valueFor(imageFormat);
    if  ( p != 0 )
    {
        int const bitsPerPixel = p->getPlaneBitsPerPixel(planeIndex);
        MY_LOGV("[%s] bits per pixel %d @ %zu-th plane", p->getName(), bitsPerPixel, planeIndex);
        return  bitsPerPixel;
    }
    //
    dumpMapInformation();
    MY_LOGW("Unsupported Image Format: %#x", imageFormat);
    return  0;
}


/*****************************************************************************
 * @brief Query the bits per pixel of a specific plane.
 *
 * @details Given a format of type EImageFormat and a plane index, return
 * the bits per pixel of the given plane.
 *
 * @note
 *
 * @param[in] imageFormat: A format of type EImageFormat.
 * @param[in] planeIndex: a specific plane index.
 *
 * @return the bits per pixel of the given plane.
 *
 ******************************************************************************/
int
queryImageBitsPerPixel(
    int const imageFormat
)
{
    sp<IImgFmt> p = getMapper().valueFor(imageFormat);
    if  ( p != 0 )
    {
        int const bitsPerPixel = p->getImageBitsPerPixel();
        MY_LOGV("[%s] bits per pixel %d", p->getName(), bitsPerPixel);
        return  bitsPerPixel;
    }
    //
    dumpMapInformation();
    MY_LOGW("Unsupported Image Format: %#x", imageFormat);
    return  0;
}

/*****************************************************************************
 * @Dump the information of Image map.
 ******************************************************************************/
void
dumpMapInformation()
{
    size_t mapSize = getMapper().size();
    if ( mapSize == 0 )
    {
        MY_LOGW("map size is 0");
    }
    else
    {
        int key;
        sp<IImgFmt> keyValue;
        MY_LOGW("map size is %zu", mapSize);
        for(size_t i = 0; i<mapSize; i++)
        {
            key = getMapper().keyAt(i);
            keyValue = getMapper().valueAt(i);
            MY_LOGW("Index (%zu): Key = %#x, value = %s", i, key, keyValue->getName());
        }
    }
}

/*****************************************************************************
 * @brief decide if need to check the correction of buffer info
 *
 * @details Given a format of type EImageFormat, return true/false.
 *
 * @note
 *
 * @param[in] imageFormat: A format of type EImageFormat.
 *
 * @return true if need to be checked
 ******************************************************************************/
bool
checkValidBufferInfo(
    int const imageFormat
)
{
    sp<IImgFmt> p = getMapper().valueFor(imageFormat);
    if  ( p != 0 )
    {
        return p->checkValidBufferInfo();
    }
    //
    dumpMapInformation();
    MY_LOGW("Unsupported Image Format: %#x", imageFormat);
    return  false;

}

};  // namespace Format
};  // namespace Utils
};  // namespace NSCam

