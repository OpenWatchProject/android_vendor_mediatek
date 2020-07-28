/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "Hal3Av3"


#include <mtkcam/utils/std/Log.h>
#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/common.h>
#include <utils/Atomic.h>
#include <cutils/properties.h>
//#include <camera_feature.h>
#include <faces.h>
#include "Hal3AAdapter3.h"

#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metadata/getDumpFilenamePrefix.h>
#include <mtkcam/aaa/IDngInfo.h>
#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>

#include <mtkcam/utils/hw/HwTransform.h>

#include <debug_exif/ver2/dbg_id_param.h>

#include <mtkcam/utils/exif/IBaseCamExif.h>
#include <debug/DebugUtil.h>

#include <ISync3A.h>

using namespace std;
using namespace android;
using namespace NS3Av3;
using namespace NSCam;
using namespace NSCamHW;


#define GET_PROP(prop, dft, val)\
{\
   val = property_get_int32(prop,dft);\
}

#define MAX(a,b) ((a) >= (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

#define HAL3A_TEST_OVERRIDE (1)

#define HAL3AADAPTER3_LOG_SET_0 (1<<0)
#define HAL3AADAPTER3_LOG_SET_1 (1<<1)
#define HAL3AADAPTER3_LOG_SET_2 (1<<2)
#define HAL3AADAPTER3_LOG_GET_0 (1<<3)
#define HAL3AADAPTER3_LOG_GET_1 (1<<4)
#define HAL3AADAPTER3_LOG_GET_2 (1<<5)
#define HAL3AADAPTER3_LOG_GET_3 (1<<6)
#define HAL3AADAPTER3_LOG_GET_4 (1<<7)
#define HAL3AADAPTER3_LOG_PF    (1<<8)
#define HAL3AADAPTER3_LOG_USERS (1<<9)
#define HAL3AADAPTER3_LOG_AREA  (1<<10)
#define HAL3AADAPTER3_LOG_PREFIXP2 (1<<11)

#define HAL3A_REQ_PROC_KEY (2)
#define HAL3A_REQ_CAPACITY (HAL3A_REQ_PROC_KEY + 2)

#define LCS_OUT_INFO_QUEUE_SIZE (15)

/*******************************************************************************
* utilities
********************************************************************************/
MUINT32 Hal3AAdapter3::mu4LogEn = 0;
MUINT32 Hal3AAdapter3::mu4DbgLogWEn = 0;

int AAA_TRACE_LEVEL = 0;

#define MY_LOGD(fmt, arg...) \
    do { \
        if (Hal3AAdapter3::mu4DbgLogWEn) { \
            CAM_LOGW(fmt, ##arg); \
        } else { \
            CAM_LOGD(fmt, ##arg); \
        } \
    }while(0)

#define MY_LOGD_IF(cond, ...) \
    do { \
        if (Hal3AAdapter3::mu4DbgLogWEn) { \
            CAM_LOGW_IF(cond, __VA_ARGS__); \
        } else { \
            if ( (cond) ){ CAM_LOGD(__VA_ARGS__); } \
        } \
    }while(0)


#if 0
inline static
MINT32 _convertAFMode(MINT32 i4Cam3Mode)
{
    MINT32 i4Cam1Mode;
    switch (i4Cam3Mode)
    {
    case MTK_CONTROL_AF_MODE_AUTO: // AF-Single Shot Mode
        i4Cam1Mode = NSFeature::AF_MODE_AFS;
        break;
    case MTK_CONTROL_AF_MODE_CONTINUOUS_PICTURE: // AF-Continuous Mode
        i4Cam1Mode = NSFeature::AF_MODE_AFC;
        break;
    case MTK_CONTROL_AF_MODE_CONTINUOUS_VIDEO: // AF-Continuous Mode (Video)
        i4Cam1Mode = NSFeature::AF_MODE_AFC_VIDEO;
        break;
    case MTK_CONTROL_AF_MODE_MACRO: // AF Macro Mode
        i4Cam1Mode = NSFeature::AF_MODE_MACRO;
        break;
    case MTK_CONTROL_AF_MODE_OFF: // Focus is set at infinity
        i4Cam1Mode = NSFeature::AF_MODE_INFINITY;
        break;
    default:
        i4Cam1Mode = NSFeature::AF_MODE_AFS;
        break;
    }
    return i4Cam1Mode;
}
#endif

inline static
CameraArea_T _transformArea(const MUINT32 i4SensorIdx, const MINT32 i4SensorMode, const CameraArea_T& rArea)
{
    if (rArea.i4Left == 0 && rArea.i4Top == 0 && rArea.i4Right == 0 && rArea.i4Bottom == 0)
    {
        return rArea;
    }
    MBOOL fgLog = (Hal3AAdapter3::mu4LogEn & HAL3AADAPTER3_LOG_AREA) ? MTRUE: MFALSE;
    MY_LOGD_IF(fgLog,"[%s] i4SensorIdx(%d), i4SensorMode(%d)", __FUNCTION__, i4SensorIdx, i4SensorMode);
    CameraArea_T rOut;

    HwTransHelper helper(i4SensorIdx);
    HwMatrix mat;
    if(!helper.getMatrixFromActive(i4SensorMode, mat))
        MY_ERR("Get hw matrix failed");

    if(fgLog)
        mat.dump(__FUNCTION__);

    MSize size(rArea.i4Right- rArea.i4Left,rArea.i4Bottom- rArea.i4Top);
    MPoint point(rArea.i4Left, rArea.i4Top);
    MRect input(point,size);
    MRect output;
    mat.transform(input, output);

    rOut.i4Left   = output.p.x;
    rOut.i4Right  = output.p.x + output.s.w;
    rOut.i4Top    = output.p.y;
    rOut.i4Bottom = output.p.y + output.s.h;
    rOut.i4Weight = rArea.i4Weight;

    MY_LOGD_IF(fgLog, "[%s] in(%d,%d,%d,%d), out(%d,%d,%d,%d)", __FUNCTION__,
        input.p.x, input.p.y, input.s.w, input.s.h,
        output.p.x, output.p.y, output.s.w, output.s.h);
    MY_LOGD_IF(fgLog, "[%s] rArea(%d,%d,%d,%d), rOut(%d,%d,%d,%d)", __FUNCTION__,
        rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom,
        rOut.i4Left, rOut.i4Top, rOut.i4Right, rOut.i4Bottom);
    return rOut;
}

inline static
CameraArea_T _transformArea2Active(const MUINT32 i4SensorIdx, const MINT32 i4SensorMode, const CameraArea_T& rArea)
{
    if (rArea.i4Left == 0 && rArea.i4Top == 0 && rArea.i4Right == 0 && rArea.i4Bottom == 0)
    {
        return rArea;
    }
    MBOOL fgLog = (Hal3AAdapter3::mu4LogEn & HAL3AADAPTER3_LOG_AREA) ? MTRUE: MFALSE;
    CameraArea_T rOut;

    HwTransHelper helper(i4SensorIdx);
    HwMatrix mat;
    if(!helper.getMatrixToActive(i4SensorMode, mat))
        MY_ERR("Get hw matrix failed");
    if(fgLog)
        mat.dump(__FUNCTION__);

    MSize size(rArea.i4Right- rArea.i4Left,rArea.i4Bottom- rArea.i4Top);
    MPoint point(rArea.i4Left, rArea.i4Top);
    MRect input(point,size);
    MRect output;
    mat.transform(input, output);

    rOut.i4Left   = output.p.x;
    rOut.i4Right  = output.p.x + output.s.w;
    rOut.i4Top    = output.p.y;
    rOut.i4Bottom = output.p.y + output.s.h;
    rOut.i4Weight = rArea.i4Weight;

    MY_LOGD_IF(fgLog, "[%s] in(%d,%d,%d,%d), out(%d,%d,%d,%d)", __FUNCTION__,
              input.p.x, input.p.y, input.s.w, input.s.h,
              output.p.x, output.p.y, output.s.w, output.s.h);
    MY_LOGD_IF(fgLog, "[%s] rArea(%d,%d,%d,%d), rOut(%d,%d,%d,%d)", __FUNCTION__,
              rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom,
              rOut.i4Left, rOut.i4Top, rOut.i4Right, rOut.i4Bottom);
    return rOut;
}


inline static
CameraArea_T _clipArea(const MINT32 i4TgWidth, const MINT32 i4TgHeight, const CameraArea_T& rArea)
{
    if (rArea.i4Left == 0 && rArea.i4Top == 0 && rArea.i4Right == 0 && rArea.i4Bottom == 0)
    {
        return rArea;
    }
    if (i4TgWidth == 0 && i4TgHeight == 0)
    {
        return rArea;
    }
    CameraArea_T rOut;
    MINT32 i4AreaWidth  = rArea.i4Right - rArea.i4Left;
    MINT32 i4AreaHeight = rArea.i4Bottom - rArea.i4Top;
    MINT32 xOffset = 0;
    MINT32 yOffset = 0;

    if(i4AreaWidth > i4TgWidth)
        xOffset = (i4AreaWidth - i4TgWidth) / 2;
    if(i4AreaHeight > i4TgHeight)
        yOffset = (i4AreaHeight - i4TgHeight) / 2;

    rOut.i4Left   = rArea.i4Left + xOffset;
    rOut.i4Top    = rArea.i4Top + yOffset;
    rOut.i4Right  = rArea.i4Right - xOffset;
    rOut.i4Bottom = rArea.i4Bottom - yOffset;
    rOut.i4Weight = rArea.i4Weight;

    MBOOL fgLog = (Hal3AAdapter3::mu4LogEn & HAL3AADAPTER3_LOG_AREA) ? MTRUE: MFALSE;
    MY_LOGD_IF(fgLog, "[%s] rArea(%d,%d,%d,%d), rOut(%d,%d,%d,%d) offset(%d,%d)", __FUNCTION__,
        rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom,
        rOut.i4Left, rOut.i4Top, rOut.i4Right, rOut.i4Bottom,
        xOffset, yOffset);
    return rOut;
}


inline static
CameraArea_T _normalizeArea(const CameraArea_T& rArea, const MRect& activeSize)
{
    CameraArea_T rOut;
    if (rArea.i4Left == 0 && rArea.i4Top == 0 && rArea.i4Right == 0 && rArea.i4Bottom == 0)
    {
        return rArea;
    }
    rOut.i4Left = (2000 * (rArea.i4Left - activeSize.p.x) / activeSize.s.w) - 1000;
    rOut.i4Right = (2000 * (rArea.i4Right - activeSize.p.x) / activeSize.s.w) - 1000;
    rOut.i4Top = (2000 * (rArea.i4Top - activeSize.p.y) / activeSize.s.h) - 1000;
    rOut.i4Bottom = (2000 * (rArea.i4Bottom - activeSize.p.y) / activeSize.s.h) - 1000;
    rOut.i4Weight = rArea.i4Weight;
    #if 0
    MY_LOGD_IF(m_fgLogEn, "[%s] in(%d,%d,%d,%d), out(%d,%d,%d,%d)", __FUNCTION__,
        rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom,
        rOut.i4Left, rOut.i4Top, rOut.i4Right, rOut.i4Bottom);
    #endif
    return rOut;
}

inline static
MVOID _updateMetadata(const IMetadata& src, IMetadata& dest)
{
    dest += src;
}

inline static
MVOID _printStaticMetadata(const IMetadata& src)
{
    MUINT32 i;
    MBOOL fgLogPf = (Hal3AAdapter3::mu4LogEn & HAL3AADAPTER3_LOG_PF) ? MTRUE: MFALSE;
    for (i = 0; i < src.count(); i++)
    {
        MUINT32 j;
        const IMetadata::IEntry& entry = src.entryAt(i);
        MY_LOGD_IF(fgLogPf, "[%s] Tag(0x%08x)", __FUNCTION__, entry.tag());
        switch (entry.tag())
        {
        case MTK_CONTROL_MAX_REGIONS:
        case MTK_SENSOR_INFO_SENSITIVITY_RANGE:
        case MTK_SENSOR_MAX_ANALOG_SENSITIVITY:
        case MTK_SENSOR_INFO_ORIENTATION:
        case MTK_CONTROL_AE_AVAILABLE_TARGET_FPS_RANGES:
        case MTK_CONTROL_AE_COMPENSATION_RANGE:
            // MINT32
            for (j = 0; j < entry.count(); j++)
            {
                MINT32 val = entry.itemAt(j, Type2Type<MINT32>());
                MY_LOGD_IF(fgLogPf, "[%s] val(%d)", __FUNCTION__, val);
            }
            break;
        case MTK_SENSOR_INFO_EXPOSURE_TIME_RANGE:
        case MTK_SENSOR_INFO_MAX_FRAME_DURATION:
            // MINT64
            for (j = 0; j < entry.count(); j++)
            {
                MINT64 val = entry.itemAt(j, Type2Type<MINT64>());
                MY_LOGD_IF(fgLogPf, "[%s] val(%lld)", __FUNCTION__, val);
            }
            break;
        case MTK_SENSOR_INFO_PHYSICAL_SIZE:
        case MTK_LENS_INFO_MINIMUM_FOCUS_DISTANCE:
        case MTK_LENS_INFO_AVAILABLE_APERTURES:
        case MTK_LENS_INFO_AVAILABLE_FILTER_DENSITIES:
        case MTK_LENS_INFO_HYPERFOCAL_DISTANCE:
        case MTK_LENS_INFO_AVAILABLE_FOCAL_LENGTHS:
            // MFLOAT
            for (j = 0; j < entry.count(); j++)
            {
                MFLOAT val = entry.itemAt(j, Type2Type<MFLOAT>());
                MY_LOGD_IF(fgLogPf, "[%s] val(%f)", __FUNCTION__, val);
            }
            break;
        case MTK_CONTROL_AE_AVAILABLE_ANTIBANDING_MODES:
        case MTK_CONTROL_AE_AVAILABLE_MODES:
        case MTK_CONTROL_AF_AVAILABLE_MODES:
        case MTK_CONTROL_AWB_AVAILABLE_MODES:
        case MTK_CONTROL_AVAILABLE_EFFECTS:
        case MTK_CONTROL_AVAILABLE_SCENE_MODES:
        case MTK_CONTROL_SCENE_MODE_OVERRIDES:
        case MTK_SENSOR_INFO_FACING:
        case MTK_LENS_INFO_AVAILABLE_OPTICAL_STABILIZATION:
        case MTK_INFO_SUPPORTED_HARDWARE_LEVEL:
            // MUINT8
            for (j = 0; j < entry.count(); j++)
            {
                MUINT8 val = entry.itemAt(j, Type2Type<MUINT8>());
                MY_LOGD_IF(fgLogPf, "[%s] val(%d)", __FUNCTION__, val);
            }
            break;
        case MTK_CONTROL_AE_COMPENSATION_STEP:
        case MTK_SENSOR_BASE_GAIN_FACTOR:
            // MRational
            for (j = 0; j < entry.count(); j++)
            {
                MRational val = entry.itemAt(j, Type2Type<MRational>());
                MY_LOGD_IF(fgLogPf, "[%s] val(%d/%d)", __FUNCTION__, val.numerator, val.denominator);
            }
            break;
        }
    }
}


inline static
MVOID _test_p2(MUINT32 u4Flag, Param_T& rParam, P2Param_T& rP2Param)
{
#define _TEST_TONEMAP       (1<<10)
#define _TEST_TONEMAP2      (1<<11)


    MUINT32 u4Magic = rP2Param.i4MagicNum;

    if (u4Flag & _TEST_TONEMAP)
    {
        const MINT32 i4Cnt = 32;
        static MFLOAT fCurve15[i4Cnt] =
        {
            0.0000f, 0.0000f, 0.0667f, 0.2920f, 0.1333f, 0.4002f, 0.2000f, 0.4812f,
            0.2667f, 0.5484f, 0.3333f, 0.6069f, 0.4000f, 0.6594f, 0.4667f, 0.7072f,
            0.5333f, 0.7515f, 0.6000f, 0.7928f, 0.6667f, 0.8317f, 0.7333f, 0.8685f,
            0.8000f, 0.9035f, 0.8667f, 0.9370f, 0.9333f, 0.9691f, 1.0000f, 1.0000f
        };

        static MFLOAT fCurve0[i4Cnt] =
        {
            0.0000f, 0.0000f, 0.0625f, 0.0625f, 0.1250f, 0.1250f, 0.2500f, 0.2500f,
            0.3125f, 0.3125f, 0.3750f, 0.3750f, 0.4375f, 0.4375f, 0.5000f, 0.5000f,
            0.5625f, 0.5625f, 0.6250f, 0.6250f, 0.6875f, 0.6875f, 0.7500f, 0.7500f,
            0.8125f, 0.8125f, 0.8750f, 0.8750f, 0.9375f, 0.9375f, 1.0000f, 1.0000f
        };

        MFLOAT fCurve[i4Cnt];

        MUINT32 u4Tonemap = 0;
        MUINT32 u4Idx = 0;
        GET_PROP("vendor.debug.hal3av3.tonemap", 0, u4Tonemap);
        if (u4Tonemap == 16)
        {
            u4Idx = rP2Param.i4MagicNum % 16;
            rParam.u1TonemapMode = u4Tonemap = MTK_TONEMAP_MODE_CONTRAST_CURVE;
        }
        else if (u4Tonemap >= 17)
        {
            rParam.u1TonemapMode = MTK_TONEMAP_MODE_FAST;
        }
        else
        {
            u4Idx = u4Tonemap;
            rParam.u1TonemapMode = u4Tonemap = MTK_TONEMAP_MODE_CONTRAST_CURVE;
        }

        if (u4Tonemap == MTK_TONEMAP_MODE_CONTRAST_CURVE)
        {
            MFLOAT fScale = (MFLOAT) u4Idx / 15.0f;
            for (MUINT32 i = 0; i < i4Cnt; i++)
            {
                fCurve[i] = ((1.0f-fScale)*fCurve0[i] + fScale*fCurve15[i]);
            }
            rParam.vecTonemapCurveBlue.resize(i4Cnt);
            rParam.vecTonemapCurveGreen.resize(i4Cnt);
            rParam.vecTonemapCurveRed.resize(i4Cnt);
            ::memcpy(&(rParam.vecTonemapCurveBlue[0]), fCurve, sizeof(MFLOAT)*i4Cnt);
            ::memcpy(&(rParam.vecTonemapCurveGreen[0]), fCurve, sizeof(MFLOAT)*i4Cnt);
            ::memcpy(&(rParam.vecTonemapCurveRed[0]), fCurve, sizeof(MFLOAT)*i4Cnt);
        }

        {
            rP2Param.u1TonemapMode = rParam.u1TonemapMode;
            if (rParam.u1TonemapMode == MTK_TONEMAP_MODE_CONTRAST_CURVE)
            {
                rP2Param.pTonemapCurveRed = &(rParam.vecTonemapCurveRed[0]);
                rP2Param.pTonemapCurveGreen = &(rParam.vecTonemapCurveGreen[0]);
                rP2Param.pTonemapCurveBlue = &(rParam.vecTonemapCurveBlue[0]);
                rP2Param.u4TonemapCurveRedSize = rParam.vecTonemapCurveRed.size();
                rP2Param.u4TonemapCurveGreenSize = rParam.vecTonemapCurveGreen.size();
                rP2Param.u4TonemapCurveBlueSize = rParam.vecTonemapCurveBlue.size();
            }
        }


    }

    if (u4Flag & _TEST_TONEMAP2)
    {
        MFLOAT fCurve[] =
        {
            0.000000,0.000000,
            0.032258,0.064516,
            0.064516,0.129032,
            0.096774,0.193548,
            0.129032,0.258065,
            0.161290,0.322581,
            0.193548,0.387097,
            0.225806,0.451613,
            0.258065,0.516129,
            0.290323,0.580645,
            0.322581,0.645161,
            0.354839,0.709677,
            0.387097,0.774194,
            0.419355,0.838710,
            0.451613,0.903226,
            0.483871,0.967742,
            0.516129,1.000000,
            0.548387,1.000000,
            0.580645,1.000000,
            0.612903,1.000000,
            0.645161,1.000000,
            0.677419,1.000000,
            0.709677,1.000000,
            0.741935,1.000000,
            0.774194,1.000000,
            0.806452,1.000000,
            0.838710,1.000000,
            0.870968,1.000000,
            0.903226,1.000000,
            0.935484,1.000000,
            0.967742,1.000000,
            1.000000,1.000000
        };

        MUINT32 u4Tonemap = 0;

        GET_PROP("vendor.debug.hal3av3.tonemap2", 0, u4Tonemap);

        if (u4Tonemap == 0)
        {
            rParam.u1TonemapMode = MTK_TONEMAP_MODE_CONTRAST_CURVE;
            rParam.vecTonemapCurveBlue.resize(64);
            rParam.vecTonemapCurveGreen.resize(64);
            rParam.vecTonemapCurveRed.resize(64);
            ::memcpy(&(rParam.vecTonemapCurveBlue[0]), fCurve, sizeof(MFLOAT)*64);
            ::memcpy(&(rParam.vecTonemapCurveGreen[0]), fCurve, sizeof(MFLOAT)*64);
            ::memcpy(&(rParam.vecTonemapCurveRed[0]), fCurve, sizeof(MFLOAT)*64);
        }
        else if (u4Tonemap == 1)
        {
            MFLOAT fLinearCurve[] = {0.0f, 1.0f, 1.0f, 0.0f};
            rParam.u1TonemapMode = MTK_TONEMAP_MODE_CONTRAST_CURVE;
            rParam.vecTonemapCurveBlue.resize(4);
            rParam.vecTonemapCurveGreen.resize(4);
            rParam.vecTonemapCurveRed.resize(4);
            ::memcpy(&(rParam.vecTonemapCurveBlue[0]), fLinearCurve, sizeof(MFLOAT)*4);
            ::memcpy(&(rParam.vecTonemapCurveGreen[0]), fLinearCurve, sizeof(MFLOAT)*4);
            ::memcpy(&(rParam.vecTonemapCurveRed[0]), fLinearCurve, sizeof(MFLOAT)*4);
        }
        else if (u4Tonemap == 2)
        {
            rParam.u1TonemapMode = MTK_TONEMAP_MODE_CONTRAST_CURVE;
            rParam.vecTonemapCurveBlue.resize(64);
            rParam.vecTonemapCurveGreen.resize(64);
            rParam.vecTonemapCurveRed.resize(64);
            for (MINT32 i = 1; i < 64; i +=2 )
            {
                fCurve[i] = 1.0f - fCurve[i];
            }
            ::memcpy(&(rParam.vecTonemapCurveBlue[0]), fCurve, sizeof(MFLOAT)*64);
            ::memcpy(&(rParam.vecTonemapCurveGreen[0]), fCurve, sizeof(MFLOAT)*64);
            ::memcpy(&(rParam.vecTonemapCurveRed[0]), fCurve, sizeof(MFLOAT)*64);
        }
        else if (u4Tonemap == 3)
        {
            MFLOAT fLinearCurve[] = {0.0f, 0.0f, 1.0f, 0.0f};
            rParam.u1TonemapMode = MTK_TONEMAP_MODE_CONTRAST_CURVE;
            rParam.vecTonemapCurveBlue.resize(4);
            rParam.vecTonemapCurveGreen.resize(64);
            rParam.vecTonemapCurveRed.resize(64);
            for (MINT32 i = 1; i < 64; i +=2 )
            {
                fCurve[i] = 1.0f - fCurve[i];
            }
            ::memcpy(&(rParam.vecTonemapCurveBlue[0]), fLinearCurve, sizeof(MFLOAT)*4);
            ::memcpy(&(rParam.vecTonemapCurveGreen[0]), fCurve, sizeof(MFLOAT)*64);
            ::memcpy(&(rParam.vecTonemapCurveRed[0]), fCurve, sizeof(MFLOAT)*64);
        }
        else if (u4Tonemap == 4)
        {
            MFLOAT fLinearCurve[] = {0.0f, 0.0f, 1.0f, 0.0f};
            rParam.u1TonemapMode = MTK_TONEMAP_MODE_CONTRAST_CURVE;
            rParam.vecTonemapCurveBlue.resize(64);
            rParam.vecTonemapCurveGreen.resize(4);
            rParam.vecTonemapCurveRed.resize(64);
            for (MINT32 i = 1; i < 64; i +=2 )
            {
                fCurve[i] = 1.0f - fCurve[i];
            }
            ::memcpy(&(rParam.vecTonemapCurveBlue[0]), fCurve, sizeof(MFLOAT)*64);
            ::memcpy(&(rParam.vecTonemapCurveGreen[0]), fLinearCurve, sizeof(MFLOAT)*4);
            ::memcpy(&(rParam.vecTonemapCurveRed[0]), fCurve, sizeof(MFLOAT)*64);
        }
        else if (u4Tonemap == 5)
        {
            MFLOAT fLinearCurve[] = {0.0f, 0.0f, 1.0f, 0.0f};
            rParam.u1TonemapMode = MTK_TONEMAP_MODE_CONTRAST_CURVE;
            rParam.vecTonemapCurveBlue.resize(64);
            rParam.vecTonemapCurveGreen.resize(64);
            rParam.vecTonemapCurveRed.resize(4);
            for (MINT32 i = 1; i < 64; i +=2 )
            {
                fCurve[i] = 1.0f - fCurve[i];
            }
            ::memcpy(&(rParam.vecTonemapCurveBlue[0]), fCurve, sizeof(MFLOAT)*64);
            ::memcpy(&(rParam.vecTonemapCurveGreen[0]), fCurve, sizeof(MFLOAT)*64);
            ::memcpy(&(rParam.vecTonemapCurveRed[0]), fLinearCurve, sizeof(MFLOAT)*4);
        }
        else if (u4Tonemap == 6)
        {
            MFLOAT fLinearCurve[] = {0.0f, 0.0f, 1.0f, 0.0f};
            rParam.u1TonemapMode = MTK_TONEMAP_MODE_CONTRAST_CURVE;
            rParam.vecTonemapCurveBlue.resize(4);
            rParam.vecTonemapCurveGreen.resize(4);
            rParam.vecTonemapCurveRed.resize(64);
            for (MINT32 i = 1; i < 64; i +=2 )
            {
                fCurve[i] = 1.0f - fCurve[i];
            }
            ::memcpy(&(rParam.vecTonemapCurveBlue[0]), fLinearCurve, sizeof(MFLOAT)*4);
            ::memcpy(&(rParam.vecTonemapCurveGreen[0]), fLinearCurve, sizeof(MFLOAT)*4);
            ::memcpy(&(rParam.vecTonemapCurveRed[0]), fCurve, sizeof(MFLOAT)*64);
        }
        else if (u4Tonemap == 7)
        {
            MFLOAT fLinearCurve[] = {0.0f, 0.0f, 1.0f, 0.0f};
            rParam.u1TonemapMode = MTK_TONEMAP_MODE_CONTRAST_CURVE;
            rParam.vecTonemapCurveBlue.resize(4);
            rParam.vecTonemapCurveGreen.resize(64);
            rParam.vecTonemapCurveRed.resize(4);
            for (MINT32 i = 1; i < 64; i +=2 )
            {
                fCurve[i] = 1.0f - fCurve[i];
            }
            ::memcpy(&(rParam.vecTonemapCurveBlue[0]), fLinearCurve, sizeof(MFLOAT)*4);
            ::memcpy(&(rParam.vecTonemapCurveGreen[0]), fCurve, sizeof(MFLOAT)*64);
            ::memcpy(&(rParam.vecTonemapCurveRed[0]), fLinearCurve, sizeof(MFLOAT)*4);
        }
        else if (u4Tonemap == 8)
        {
            MFLOAT fLinearCurve[] = {0.0f, 0.0f, 1.0f, 0.0f};
            rParam.u1TonemapMode = MTK_TONEMAP_MODE_CONTRAST_CURVE;
            rParam.vecTonemapCurveBlue.resize(64);
            rParam.vecTonemapCurveGreen.resize(4);
            rParam.vecTonemapCurveRed.resize(4);
            for (MINT32 i = 1; i < 64; i +=2 )
            {
                fCurve[i] = 1.0f - fCurve[i];
            }
            ::memcpy(&(rParam.vecTonemapCurveBlue[0]), fCurve, sizeof(MFLOAT)*64);
            ::memcpy(&(rParam.vecTonemapCurveGreen[0]), fLinearCurve, sizeof(MFLOAT)*4);
            ::memcpy(&(rParam.vecTonemapCurveRed[0]), fLinearCurve, sizeof(MFLOAT)*4);
        }
        else if (u4Tonemap == 9)
        {
            MFLOAT fLinearCurve[] = {0.0f, 0.0f, 1.0f, 0.0f};
            rParam.u1TonemapMode = MTK_TONEMAP_MODE_CONTRAST_CURVE;
            rParam.vecTonemapCurveBlue.resize(4);
            rParam.vecTonemapCurveGreen.resize(4);
            rParam.vecTonemapCurveRed.resize(64);
            ::memcpy(&(rParam.vecTonemapCurveBlue[0]), fLinearCurve, sizeof(MFLOAT)*4);
            ::memcpy(&(rParam.vecTonemapCurveGreen[0]), fLinearCurve, sizeof(MFLOAT)*4);
            ::memcpy(&(rParam.vecTonemapCurveRed[0]), fCurve, sizeof(MFLOAT)*64);
        }
        else if (u4Tonemap == 10)
        {
            MFLOAT fLinearCurve[] = {0.0f, 0.0f, 1.0f, 0.0f};
            rParam.u1TonemapMode = MTK_TONEMAP_MODE_CONTRAST_CURVE;
            rParam.vecTonemapCurveBlue.resize(4);
            rParam.vecTonemapCurveGreen.resize(64);
            rParam.vecTonemapCurveRed.resize(4);
            ::memcpy(&(rParam.vecTonemapCurveBlue[0]), fLinearCurve, sizeof(MFLOAT)*4);
            ::memcpy(&(rParam.vecTonemapCurveGreen[0]), fCurve, sizeof(MFLOAT)*64);
            ::memcpy(&(rParam.vecTonemapCurveRed[0]), fLinearCurve, sizeof(MFLOAT)*4);
        }
        else if (u4Tonemap == 11)
        {
            MFLOAT fLinearCurve[] = {0.0f, 0.0f, 1.0f, 0.0f};
            rParam.u1TonemapMode = MTK_TONEMAP_MODE_CONTRAST_CURVE;
            rParam.vecTonemapCurveBlue.resize(64);
            rParam.vecTonemapCurveGreen.resize(4);
            rParam.vecTonemapCurveRed.resize(4);
            ::memcpy(&(rParam.vecTonemapCurveBlue[0]), fCurve, sizeof(MFLOAT)*64);
            ::memcpy(&(rParam.vecTonemapCurveGreen[0]), fLinearCurve, sizeof(MFLOAT)*4);
            ::memcpy(&(rParam.vecTonemapCurveRed[0]), fLinearCurve, sizeof(MFLOAT)*4);
        }
        else
        {
            rParam.u1TonemapMode = MTK_TONEMAP_MODE_FAST;
        }

        {
            rP2Param.u1TonemapMode = rParam.u1TonemapMode;
            if (rParam.u1TonemapMode == MTK_TONEMAP_MODE_CONTRAST_CURVE)
            {
                rP2Param.pTonemapCurveRed = &(rParam.vecTonemapCurveRed[0]);
                rP2Param.pTonemapCurveGreen = &(rParam.vecTonemapCurveGreen[0]);
                rP2Param.pTonemapCurveBlue = &(rParam.vecTonemapCurveBlue[0]);
                rP2Param.u4TonemapCurveRedSize = rParam.vecTonemapCurveRed.size();
                rP2Param.u4TonemapCurveGreenSize = rParam.vecTonemapCurveGreen.size();
                rP2Param.u4TonemapCurveBlueSize = rParam.vecTonemapCurveBlue.size();
            }
        }
    }

}
#if HAL3A_TEST_OVERRIDE
inline static
MVOID _test(MUINT32 u4Flag, Param_T& rParam)
{
#define _TEST_MANUAL_SENSOR (1<<0)
#define _TEST_MANUAL_LENS   (1<<1)
#define _TEST_MANUAL_WB     (1<<2)
#define _TEST_EDGE_MODE     (1<<3)
#define _TEST_NR_MODE       (1<<4)
#define _TEST_SHADING       (1<<5)
#define _TEST_TORCH         (1<<6)
#define _TEST_FLK           (1<<7)
#define _TEST_AALOCK        (1<<8)
#define _TEST_BLK_LOCK      (1<<9)
#define _TEST_TONEMAP       (1<<10)
#define _TEST_TONEMAP2      (1<<11)
#define _TEST_CAP_SINGLE    (1<<14)
#define _TEST_EFFECT_MODE   (1<<15)
#define _TEST_SCENE_MODE    (1<<16)

    MUINT32 u4Magic = rParam.i4MagicNum;

    if (u4Flag & _TEST_MANUAL_SENSOR)
    {
        MUINT32 u4Sensor = 0;
        MINT32 i4CamModeEnable = 0;
        GET_PROP("vendor.debug.hal3av3.sensor", 0, u4Sensor);
        GET_PROP("vendor.debug.camera.cammode", 0, i4CamModeEnable);
        if (i4CamModeEnable != 0)
        {
            MUINT32 u4IsoSpeedMode = 0;
            GET_PROP("vendor.debug.camera.isospeed", 0, u4IsoSpeedMode);
            rParam.u4CamMode = i4CamModeEnable;
            rParam.i4IsoSpeedMode = u4IsoSpeedMode;
        }
        else
        {
            rParam.u4AeMode = MTK_CONTROL_AE_MODE_OFF;
            if (u4Sensor == 0)
            {
                rParam.i8ExposureTime = 3000000L + 3000000L * (u4Magic % 10);
                rParam.i4Sensitivity = 100;
                rParam.i8FrameDuration = 33000000L;
            }
            else if (u4Sensor == 1)
            {
                rParam.i8ExposureTime = 15000000L;
                rParam.i4Sensitivity = 100 + 100*(u4Magic%10);
                rParam.i8FrameDuration = 33000000L;
            }
            else if (u4Sensor == 2)
            {
                MINT32 iso = 100;
                GET_PROP("vendor.debug.hal3av3.iso", 100, iso);
                rParam.i8ExposureTime = 15000000L;
                rParam.i4Sensitivity = iso;
                rParam.i8FrameDuration = 33000000L;
            }
            else if (u4Sensor == 3)
            {
                if (u4Magic & 0x1)
                {
                    rParam.i8ExposureTime = 15000000L;
                    rParam.i4Sensitivity = 100;
                }
                else
                {
                    MINT32 iso = 100;
                    GET_PROP("vendor.debug.hal3av3.iso", 200, iso);
                    rParam.i8ExposureTime = 7500000L;
                    rParam.i4Sensitivity = iso;
                }
                rParam.i8FrameDuration = 33000000L;
            }
            else
            {
                rParam.i8ExposureTime = 3000000L;
                rParam.i4Sensitivity = 400;
                rParam.i8FrameDuration = 3000000L + 3000000L * (u4Magic % 10);
            }
        }
    }

    // Fix Me
    /*if (u4Flag & _TEST_MANUAL_LENS)
    {
        static MFLOAT fDist[] = {50, 100, 150, 200, 400, 800, 1200, 2000, 3000};
        rParam.u4AfMode = MTK_CONTROL_AF_MODE_OFF;
        rParam.fFocusDistance = (MFLOAT) 1000.0f / fDist[u4Magic%9];
    }*/

    if (u4Flag & _TEST_MANUAL_WB)
    {
        MUINT32 u4Wb = 0;
        MUINT32 u4Channel = 0;
        GET_PROP("vendor.debug.hal3av3.wb", 0, u4Wb);
        rParam.u4AwbMode = MTRUE ? MTK_CONTROL_AWB_MODE_OFF : rParam.u4AwbMode;
        rParam.u1ColorCorrectMode = MTRUE ? MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX : rParam.u1ColorCorrectMode;
        u4Channel = u4Wb & 0x3;
        rParam.fColorCorrectGain[u4Channel] = (u4Magic % 2) ? 1.0f : 2.0f;
        rParam.fColorCorrectGain[(u4Channel+1)&0x3] = 1.0f;
        rParam.fColorCorrectGain[(u4Channel+2)&0x3] = 1.0f;
        rParam.fColorCorrectGain[(u4Channel+3)&0x3] = 1.0f;
        rParam.fColorCorrectMat[0] = 1.0f;
        rParam.fColorCorrectMat[1] = 0.0f;
        rParam.fColorCorrectMat[2] = 0.0f;
        rParam.fColorCorrectMat[3] = 0.0f;
        rParam.fColorCorrectMat[4] = 1.0f;
        rParam.fColorCorrectMat[5] = 0.0f;
        rParam.fColorCorrectMat[6] = 0.0f;
        rParam.fColorCorrectMat[7] = 0.0f;
        rParam.fColorCorrectMat[8] = 1.0f;
    }


    if (u4Flag & _TEST_EDGE_MODE)
    {
        MUINT32 u4Edge = 0;
        GET_PROP("vendor.debug.hal3av3.edge", 0, u4Edge);
        rParam.u1EdgeMode = u4Edge;
    }

    if (u4Flag & _TEST_NR_MODE)
    {
        MUINT32 u4NR = 0;
        GET_PROP("vendor.debug.hal3av3.nr", 0, u4NR);
        rParam.u1NRMode = u4NR;
    }

    if (u4Flag & _TEST_SHADING)
    {
        MUINT32 u4Shading = 0;
        GET_PROP("vendor.debug.hal3av3.shading", 0, u4Shading);
        rParam.u1ShadingMode = u4Shading;
    }

    if (u4Flag & _TEST_TORCH)
    {
        rParam.u4AeMode = MTK_CONTROL_AE_MODE_ON;
        rParam.u4StrobeMode = MTK_FLASH_MODE_TORCH;
    }

    if (u4Flag & _TEST_FLK)
    {
        MUINT32 u4Flk = 0;
        GET_PROP("vendor.debug.hal3av3.flk", 0, u4Flk);
        rParam.u4AntiBandingMode = u4Flk;
    }

    if (u4Flag & _TEST_AALOCK)
    {
        MUINT32 u4AALock = 0;
        GET_PROP("vendor.debug.hal3av3.aalock", 0, u4AALock);
        rParam.bIsAELock = (u4AALock&0x1) ? MTRUE : MFALSE;
        rParam.bIsAWBLock = (u4AALock&0x2) ? MTRUE : MFALSE;
    }

    if (u4Flag & _TEST_BLK_LOCK)
    {
        MUINT32 u4Lock = 0;
        GET_PROP("vendor.debug.hal3av3.blklock", 0, u4Lock);
        rParam.u1BlackLvlLock = u4Lock ? MTRUE : MFALSE;
    }
/*
    if (u4Flag & _TEST_TONEMAP)
    {
        const MINT32 i4Cnt = 32;
        static MFLOAT fCurve15[i4Cnt] =
        {
            0.0000f, 0.0000f, 0.0667f, 0.2920f, 0.1333f, 0.4002f, 0.2000f, 0.4812f,
            0.2667f, 0.5484f, 0.3333f, 0.6069f, 0.4000f, 0.6594f, 0.4667f, 0.7072f,
            0.5333f, 0.7515f, 0.6000f, 0.7928f, 0.6667f, 0.8317f, 0.7333f, 0.8685f,
            0.8000f, 0.9035f, 0.8667f, 0.9370f, 0.9333f, 0.9691f, 1.0000f, 1.0000f
        };

        static MFLOAT fCurve0[i4Cnt] =
        {
            0.0000f, 0.0000f, 0.0625f, 0.0625f, 0.1250f, 0.1250f, 0.2500f, 0.2500f,
            0.3125f, 0.3125f, 0.3750f, 0.3750f, 0.4375f, 0.4375f, 0.5000f, 0.5000f,
            0.5625f, 0.5625f, 0.6250f, 0.6250f, 0.6875f, 0.6875f, 0.7500f, 0.7500f,
            0.8125f, 0.8125f, 0.8750f, 0.8750f, 0.9375f, 0.9375f, 1.0000f, 1.0000f
        };

        MFLOAT fCurve[i4Cnt];

        MUINT32 u4Tonemap = 0;
        MUINT32 u4Idx = 0;
        GET_PROP("vendor.debug.hal3av3.tonemap", 0, u4Tonemap);
        if (u4Tonemap == 16)
        {
            u4Idx = u4Magic % 16;
            rParam.u1TonemapMode = u4Tonemap = MTK_TONEMAP_MODE_CONTRAST_CURVE;
        }
        else if (u4Tonemap >= 17)
        {
            rParam.u1TonemapMode = MTK_TONEMAP_MODE_FAST;
        }
        else
        {
            u4Idx = u4Tonemap;
            rParam.u1TonemapMode = u4Tonemap = MTK_TONEMAP_MODE_CONTRAST_CURVE;
        }

        if (u4Tonemap == MTK_TONEMAP_MODE_CONTRAST_CURVE)
        {
            MFLOAT fScale = (MFLOAT) u4Idx / 15.0f;
            for (MUINT32 i = 0; i < i4Cnt; i++)
            {
                fCurve[i] = ((1.0f-fScale)*fCurve0[i] + fScale*fCurve15[i]);
            }
            rParam.vecTonemapCurveBlue.resize(i4Cnt);
            rParam.vecTonemapCurveGreen.resize(i4Cnt);
            rParam.vecTonemapCurveRed.resize(i4Cnt);
            ::memcpy(rParam.vecTonemapCurveBlue.editArray(), fCurve, sizeof(MFLOAT)*i4Cnt);
            ::memcpy(rParam.vecTonemapCurveGreen.editArray(), fCurve, sizeof(MFLOAT)*i4Cnt);
            ::memcpy(rParam.vecTonemapCurveRed.editArray(), fCurve, sizeof(MFLOAT)*i4Cnt);
        }
    }

    if (u4Flag & _TEST_TONEMAP2)
    {
        MFLOAT fCurve[] =
        {
            0.000000,0.000000,
            0.032258,0.064516,
            0.064516,0.129032,
            0.096774,0.193548,
            0.129032,0.258065,
            0.161290,0.322581,
            0.193548,0.387097,
            0.225806,0.451613,
            0.258065,0.516129,
            0.290323,0.580645,
            0.322581,0.645161,
            0.354839,0.709677,
            0.387097,0.774194,
            0.419355,0.838710,
            0.451613,0.903226,
            0.483871,0.967742,
            0.516129,1.000000,
            0.548387,1.000000,
            0.580645,1.000000,
            0.612903,1.000000,
            0.645161,1.000000,
            0.677419,1.000000,
            0.709677,1.000000,
            0.741935,1.000000,
            0.774194,1.000000,
            0.806452,1.000000,
            0.838710,1.000000,
            0.870968,1.000000,
            0.903226,1.000000,
            0.935484,1.000000,
            0.967742,1.000000,
            1.000000,1.000000
        };

        MUINT32 u4Tonemap = 0;

        GET_PROP("vendor.debug.hal3av3.tonemap2", 0, u4Tonemap);

        if (u4Tonemap == 0)
        {
            rParam.u1TonemapMode = MTK_TONEMAP_MODE_CONTRAST_CURVE;
            rParam.vecTonemapCurveBlue.resize(64);
            rParam.vecTonemapCurveGreen.resize(64);
            rParam.vecTonemapCurveRed.resize(64);
            ::memcpy(rParam.vecTonemapCurveBlue.editArray(), fCurve, sizeof(MFLOAT)*64);
            ::memcpy(rParam.vecTonemapCurveGreen.editArray(), fCurve, sizeof(MFLOAT)*64);
            ::memcpy(rParam.vecTonemapCurveRed.editArray(), fCurve, sizeof(MFLOAT)*64);
        }
        else if (u4Tonemap == 1)
        {
            MFLOAT fLinearCurve[] = {0.0f, 1.0f, 1.0f, 0.0f};
        rParam.u1TonemapMode = MTK_TONEMAP_MODE_CONTRAST_CURVE;
        rParam.vecTonemapCurveBlue.resize(4);
        rParam.vecTonemapCurveGreen.resize(4);
        rParam.vecTonemapCurveRed.resize(4);
            ::memcpy(rParam.vecTonemapCurveBlue.editArray(), fLinearCurve, sizeof(MFLOAT)*4);
            ::memcpy(rParam.vecTonemapCurveGreen.editArray(), fLinearCurve, sizeof(MFLOAT)*4);
            ::memcpy(rParam.vecTonemapCurveRed.editArray(), fLinearCurve, sizeof(MFLOAT)*4);
        }
        else if (u4Tonemap == 2)
        {
            rParam.u1TonemapMode = MTK_TONEMAP_MODE_CONTRAST_CURVE;
            rParam.vecTonemapCurveBlue.resize(64);
            rParam.vecTonemapCurveGreen.resize(64);
            rParam.vecTonemapCurveRed.resize(64);
            for (MINT32 i = 1; i < 64; i +=2 )
            {
                fCurve[i] = 1.0f - fCurve[i];
            }
            ::memcpy(rParam.vecTonemapCurveBlue.editArray(), fCurve, sizeof(MFLOAT)*64);
            ::memcpy(rParam.vecTonemapCurveGreen.editArray(), fCurve, sizeof(MFLOAT)*64);
            ::memcpy(rParam.vecTonemapCurveRed.editArray(), fCurve, sizeof(MFLOAT)*64);
        }
        else if (u4Tonemap == 3)
        {
            MFLOAT fLinearCurve[] = {0.0f, 0.0f, 1.0f, 0.0f};
            rParam.u1TonemapMode = MTK_TONEMAP_MODE_CONTRAST_CURVE;
            rParam.vecTonemapCurveBlue.resize(4);
            rParam.vecTonemapCurveGreen.resize(64);
            rParam.vecTonemapCurveRed.resize(64);
            for (MINT32 i = 1; i < 64; i +=2 )
            {
                fCurve[i] = 1.0f - fCurve[i];
            }
            ::memcpy(rParam.vecTonemapCurveBlue.editArray(), fLinearCurve, sizeof(MFLOAT)*4);
            ::memcpy(rParam.vecTonemapCurveGreen.editArray(), fCurve, sizeof(MFLOAT)*64);
            ::memcpy(rParam.vecTonemapCurveRed.editArray(), fCurve, sizeof(MFLOAT)*64);
        }
        else if (u4Tonemap == 4)
        {
            MFLOAT fLinearCurve[] = {0.0f, 0.0f, 1.0f, 0.0f};
            rParam.u1TonemapMode = MTK_TONEMAP_MODE_CONTRAST_CURVE;
            rParam.vecTonemapCurveBlue.resize(64);
            rParam.vecTonemapCurveGreen.resize(4);
            rParam.vecTonemapCurveRed.resize(64);
            for (MINT32 i = 1; i < 64; i +=2 )
            {
                fCurve[i] = 1.0f - fCurve[i];
            }
            ::memcpy(rParam.vecTonemapCurveBlue.editArray(), fCurve, sizeof(MFLOAT)*64);
            ::memcpy(rParam.vecTonemapCurveGreen.editArray(), fLinearCurve, sizeof(MFLOAT)*4);
            ::memcpy(rParam.vecTonemapCurveRed.editArray(), fCurve, sizeof(MFLOAT)*64);
        }
        else if (u4Tonemap == 5)
        {
            MFLOAT fLinearCurve[] = {0.0f, 0.0f, 1.0f, 0.0f};
            rParam.u1TonemapMode = MTK_TONEMAP_MODE_CONTRAST_CURVE;
            rParam.vecTonemapCurveBlue.resize(64);
            rParam.vecTonemapCurveGreen.resize(64);
            rParam.vecTonemapCurveRed.resize(4);
            for (MINT32 i = 1; i < 64; i +=2 )
            {
                fCurve[i] = 1.0f - fCurve[i];
            }
            ::memcpy(rParam.vecTonemapCurveBlue.editArray(), fCurve, sizeof(MFLOAT)*64);
            ::memcpy(rParam.vecTonemapCurveGreen.editArray(), fCurve, sizeof(MFLOAT)*64);
            ::memcpy(rParam.vecTonemapCurveRed.editArray(), fLinearCurve, sizeof(MFLOAT)*4);
        }
        else if (u4Tonemap == 6)
        {
            MFLOAT fLinearCurve[] = {0.0f, 0.0f, 1.0f, 0.0f};
            rParam.u1TonemapMode = MTK_TONEMAP_MODE_CONTRAST_CURVE;
            rParam.vecTonemapCurveBlue.resize(4);
            rParam.vecTonemapCurveGreen.resize(4);
            rParam.vecTonemapCurveRed.resize(64);
            for (MINT32 i = 1; i < 64; i +=2 )
            {
                fCurve[i] = 1.0f - fCurve[i];
            }
            ::memcpy(rParam.vecTonemapCurveBlue.editArray(), fLinearCurve, sizeof(MFLOAT)*4);
            ::memcpy(rParam.vecTonemapCurveGreen.editArray(), fLinearCurve, sizeof(MFLOAT)*4);
            ::memcpy(rParam.vecTonemapCurveRed.editArray(), fCurve, sizeof(MFLOAT)*64);
        }
        else if (u4Tonemap == 7)
        {
            MFLOAT fLinearCurve[] = {0.0f, 0.0f, 1.0f, 0.0f};
        rParam.u1TonemapMode = MTK_TONEMAP_MODE_CONTRAST_CURVE;
        rParam.vecTonemapCurveBlue.resize(4);
            rParam.vecTonemapCurveGreen.resize(64);
            rParam.vecTonemapCurveRed.resize(4);
            for (MINT32 i = 1; i < 64; i +=2 )
            {
                fCurve[i] = 1.0f - fCurve[i];
            }
            ::memcpy(rParam.vecTonemapCurveBlue.editArray(), fLinearCurve, sizeof(MFLOAT)*4);
            ::memcpy(rParam.vecTonemapCurveGreen.editArray(), fCurve, sizeof(MFLOAT)*64);
            ::memcpy(rParam.vecTonemapCurveRed.editArray(), fLinearCurve, sizeof(MFLOAT)*4);
        }
        else if (u4Tonemap == 8)
        {
            MFLOAT fLinearCurve[] = {0.0f, 0.0f, 1.0f, 0.0f};
            rParam.u1TonemapMode = MTK_TONEMAP_MODE_CONTRAST_CURVE;
            rParam.vecTonemapCurveBlue.resize(64);
        rParam.vecTonemapCurveGreen.resize(4);
        rParam.vecTonemapCurveRed.resize(4);
            for (MINT32 i = 1; i < 64; i +=2 )
            {
                fCurve[i] = 1.0f - fCurve[i];
            }
            ::memcpy(rParam.vecTonemapCurveBlue.editArray(), fCurve, sizeof(MFLOAT)*64);
            ::memcpy(rParam.vecTonemapCurveGreen.editArray(), fLinearCurve, sizeof(MFLOAT)*4);
            ::memcpy(rParam.vecTonemapCurveRed.editArray(), fLinearCurve, sizeof(MFLOAT)*4);
        }
        else if (u4Tonemap == 9)
        {
            MFLOAT fLinearCurve[] = {0.0f, 0.0f, 1.0f, 0.0f};
        rParam.u1TonemapMode = MTK_TONEMAP_MODE_CONTRAST_CURVE;
        rParam.vecTonemapCurveBlue.resize(4);
        rParam.vecTonemapCurveGreen.resize(4);
            rParam.vecTonemapCurveRed.resize(64);
            ::memcpy(rParam.vecTonemapCurveBlue.editArray(), fLinearCurve, sizeof(MFLOAT)*4);
            ::memcpy(rParam.vecTonemapCurveGreen.editArray(), fLinearCurve, sizeof(MFLOAT)*4);
            ::memcpy(rParam.vecTonemapCurveRed.editArray(), fCurve, sizeof(MFLOAT)*64);
        }
        else if (u4Tonemap == 10)
        {
            MFLOAT fLinearCurve[] = {0.0f, 0.0f, 1.0f, 0.0f};
            rParam.u1TonemapMode = MTK_TONEMAP_MODE_CONTRAST_CURVE;
            rParam.vecTonemapCurveBlue.resize(4);
            rParam.vecTonemapCurveGreen.resize(64);
        rParam.vecTonemapCurveRed.resize(4);
            ::memcpy(rParam.vecTonemapCurveBlue.editArray(), fLinearCurve, sizeof(MFLOAT)*4);
            ::memcpy(rParam.vecTonemapCurveGreen.editArray(), fCurve, sizeof(MFLOAT)*64);
            ::memcpy(rParam.vecTonemapCurveRed.editArray(), fLinearCurve, sizeof(MFLOAT)*4);
        }
        else if (u4Tonemap == 11)
        {
            MFLOAT fLinearCurve[] = {0.0f, 0.0f, 1.0f, 0.0f};
            rParam.u1TonemapMode = MTK_TONEMAP_MODE_CONTRAST_CURVE;
            rParam.vecTonemapCurveBlue.resize(64);
            rParam.vecTonemapCurveGreen.resize(4);
            rParam.vecTonemapCurveRed.resize(4);
            ::memcpy(rParam.vecTonemapCurveBlue.editArray(), fCurve, sizeof(MFLOAT)*64);
            ::memcpy(rParam.vecTonemapCurveGreen.editArray(), fLinearCurve, sizeof(MFLOAT)*4);
            ::memcpy(rParam.vecTonemapCurveRed.editArray(), fLinearCurve, sizeof(MFLOAT)*4);
        }
        else
        {
            rParam.u1TonemapMode = MTK_TONEMAP_MODE_FAST;
        }
    }
*/
    if (u4Flag & _TEST_CAP_SINGLE)
    {
        //rParam.u4AeMode = MTK_CONTROL_AE_MODE_ON_ALWAYS_FLASH;
        MUINT32 u4Flash = 0;
        GET_PROP("vendor.debug.hal3av3.flash", 0, u4Flash);
        switch (u4Flash)
        {
        case 0:
            rParam.u4StrobeMode = MTK_FLASH_MODE_OFF;
            break;
        case 1:
            //if ((u4Magic % 5) == 0)
            {
                rParam.u4AeMode = MTK_CONTROL_AE_MODE_ON;
                rParam.u4StrobeMode = MTK_FLASH_MODE_SINGLE;
            }
            break;
        case 2:
            {
                rParam.u4AeMode = MTK_CONTROL_AE_MODE_ON;
                rParam.u4StrobeMode = MTK_FLASH_MODE_TORCH;
            }
            break;
        }
    }

    if (u4Flag & _TEST_SCENE_MODE)
    {
        MUINT32 u4Scene = 0;
        GET_PROP("vendor.debug.hal3av3.scene", 0, u4Scene);
        rParam.u1ControlMode = MTK_CONTROL_MODE_USE_SCENE_MODE;
        rParam.u4SceneMode = u4Scene;
    }

    if (u4Flag & _TEST_EFFECT_MODE)
    {
        MUINT32 u4Efct = 0;
        GET_PROP("vendor.debug.hal3av3.effect", 0, u4Efct);
        rParam.u4EffectMode = u4Efct;
    }
}
#endif

/*******************************************************************************
* implementations
********************************************************************************/
Hal3AAdapter3*
Hal3AAdapter3::
createInstance(MINT32 const i4SensorIdx, const char* strUser)
{
    MINT32 i4LogLevel = 0;
    GET_PROP("vendor.debug.camera.log", 0, i4LogLevel);
    GET_PROP("vendor.debug.hal3av3.log", 0, mu4LogEn);
    if (i4LogLevel) mu4LogEn |= HAL3AADAPTER3_LOG_PF;
    mu4DbgLogWEn = DebugUtil::getDebugLevel(DBG_3A);

    MY_LOGD_IF((mu4LogEn & HAL3AADAPTER3_LOG_PF), "[%s] sensorIdx(%d) %s", __FUNCTION__, i4SensorIdx, strUser);
    switch (i4SensorIdx)
    {
    case 0:
        {
            static Hal3AAdapter3 _singleton(0);
            _singleton.init(strUser);
            return &_singleton;
        }
    case 1:
        {
            static Hal3AAdapter3 _singleton(1);
            _singleton.init(strUser);
            return &_singleton;
        }
    case 2:
        {
            static Hal3AAdapter3 _singleton(2);
            _singleton.init(strUser);
            return &_singleton;
        }
    case 3:
        {
            static Hal3AAdapter3 _singleton(3);
            _singleton.init(strUser);
            return &_singleton;
        }
    default:
        return NULL;
    }
}

Hal3AAdapter3::
Hal3AAdapter3(MINT32 const i4SensorIdx)
    : mpHal3aObj(NULL)
    , mi4FrmId(-1)
    , mi4SensorIdx(i4SensorIdx)
    , mi4User(0)
    , mu4Counter(0)
    , mLock()
    , mLockIsp()
    , mu1Start(0)
    //, mu4MetaResultQueueCapacity(6)
    , m_i4SubsampleCount(1)
    , mpTuning(NULL)
    , mu4SensorDev(0)
    , mi4SensorMode(0)
    , mu1CapIntent(0)
    , mu1Precature(0)
    , mu1FdEnable(0)
    , mu4MetaResultQueueCapacity(0)
    , m_u4LCSOCapacity(LCS_OUT_INFO_QUEUE_SIZE)
    , m_LCSO_Lock_Out()
{
/*
    if (! buildStaticInfo(mMetaStaticInfo))
    {
        CAM_LOGE("[%s] Static Info load error.", __FUNCTION__);
    }
*/
    ::memset(&m_rStaticInfo, 0, sizeof(m_rStaticInfo));
    MY_LOGD_IF((mu4LogEn & HAL3AADAPTER3_LOG_PF),
        "[%s] sensorIdx(0x%04x)", __FUNCTION__, i4SensorIdx);
}

MVOID
Hal3AAdapter3::
destroyInstance(const char* strUser)
{
    MY_LOGD_IF((mu4LogEn & HAL3AADAPTER3_LOG_PF), "[%s] sensorIdx(%d) %s", __FUNCTION__, mi4SensorIdx, strUser);
    uninit(strUser);
}

MINT32
Hal3AAdapter3::
config(const ConfigInfo_T& rConfigInfo)
{
    MY_LOGD("[%s] i4SubsampleCount(%d)", __FUNCTION__, rConfigInfo.i4SubsampleCount);

    if (rConfigInfo.i4SubsampleCount > 1) //for SMVR
    {
        MY_LOGD("[SMVR] uninit 3A");
        doUninit();
        m_i4SubsampleCount = rConfigInfo.i4SubsampleCount;
        MY_LOGD("[SMVR] init 3A");
        doInit();
    }
    else
    {
        if (m_i4SubsampleCount > 1)
        {
            MY_LOGD("[SMVR] uninit 3A");
            doUninit();
            m_i4SubsampleCount = rConfigInfo.i4SubsampleCount;
            MY_LOGD("[SMVR] init 3A");
            doInit();
        }
    }

    // set staticInfo must do after init step, becuase MW updated static metadata after finish all init step - Muse.Sie W1633
    //sp<IMetadataProvider> pMetadataProvider = IMetadataProvider::create(mi4SensorIdx);
    sp<IMetadataProvider> pMetadataProvider = NSMetadataProviderManager::valueFor(mi4SensorIdx);
    if (pMetadataProvider != NULL)
    {
        mMetaStaticInfo = pMetadataProvider->getMtkStaticCharacteristics();
        //setupStaticInfo();
        // DEBUG
        // _printStaticMetadata(mMetaStaticInfo);
    }
    else
    {
        CAM_LOGE("[%s] Fail to get static metadata: mi4SensorIdx(0x%04x)",
            __FUNCTION__, mi4SensorIdx);
    }
    setupStaticInfo();

    mpHal3aObj->setParams(mParams, MFALSE);
    //

    mpHal3aObj->setSensorMode(mi4SensorMode);

    MY_LOGD("[%s]", __FUNCTION__);
    return mpHal3aObj->config();
}

MINT32
Hal3AAdapter3::
start(MINT32 i4StartNum)
{
    Mutex::Autolock lock(mLock);
    MINT32 i4SensorPreviewDelay;

    MY_LOGD("[%s]+ sensorDev(%d), sensorIdx(%d), mpHal3aObj(%p)",
        __FUNCTION__, mu4SensorDev, mi4SensorIdx, mpHal3aObj);

    MBOOL ret = doInitMetaResultQ();
    if(!ret)
        CAM_LOGE("[%s]- Fail to metaResultQueue init");

#if 0
    mParams = Param_T();
    setupStaticInfo();
#endif
    mpHal3aObj->sendCommand(NS3Av3::ECmd_CameraPreviewStart);

    send3ACtrl(E3ACtrl_GetSensorPreviewDelay, reinterpret_cast<MINTPTR>(&i4SensorPreviewDelay), NULL);
    i4SensorPreviewDelay = i4SensorPreviewDelay + 2;//shutter delay = 2
    RequestSet_T rRequestSet;
    NS3Av3::ParamIspProfile_T _3AProf(NSIspTuning::EIspProfile_Preview, 1, 0, MTRUE, NS3Av3::ParamIspProfile_T::EParamValidate_None, rRequestSet);
    for (int i = 1; i <= i4SensorPreviewDelay; i++)
    {
        _3AProf.i4MagicNum = i;
    rRequestSet.vNumberSet.clear();
        rRequestSet.vNumberSet.push_back(i);
    mpHal3aObj->sendCommand(NS3Av3::ECmd_Update, reinterpret_cast<MUINTPTR>(&_3AProf));
    }
    mu1Start = 1;
    MY_LOGD("[%s]- %d Dummy Update Sent, sensorDev(%d)", __FUNCTION__, i4SensorPreviewDelay, mu4SensorDev);
    return 0;
}

MINT32
Hal3AAdapter3::
stop()
{
    Mutex::Autolock lock(mLock);

    if (mu1Start)
    {
#if CAM3_STEREO_FEATURE_EN
        Stereo_Param_T rStereoParam;
        rStereoParam.i4HwSyncMode = NS3Av3::E_HW_FRM_SYNC_MODE::E_HW_FRM_SYNC_MODE_OFF;
        rStereoParam.i4Sync2AMode= NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_NONE;
        rStereoParam.i4SyncAFMode= NS3Av3::E_SYNCAF_MODE::E_SYNCAF_MODE_OFF;
        mpHal3aObj->send3ACtrl(E3ACtrl_SetStereoParams, (MINTPTR)&rStereoParam, 0);
#endif

        MY_LOGD("[%s] sensorDev(%d), sensorIdx(%d), mpHal3aObj(%p)",
            __FUNCTION__, mu4SensorDev, mi4SensorIdx, mpHal3aObj);
        mpHal3aObj->sendCommand(NS3Av3::ECmd_CameraPreviewEnd);
        return 0;
    }
    else
    {
        CAM_LOGE("[%s] Not yet started! sensorDev(%d), sensorIdx(%d), mpHal3aObj(%p)",
            __FUNCTION__, mu4SensorDev, mi4SensorIdx, mpHal3aObj);
        return 1;
    }
}

MVOID
Hal3AAdapter3::
stopStt()
{
    Mutex::Autolock lock(mLock);

    if (mu1Start)
    {
        MY_LOGD("[%s] sensorDev(%d), sensorIdx(%d), mpHal3aObj(%p)",
            __FUNCTION__, mu4SensorDev, mi4SensorIdx, mpHal3aObj);
        mpHal3aObj->stopStt();
        return;
    }
    else
    {
        CAM_LOGE("[%s] Not yet started! sensorDev(%d), sensorIdx(%d), mpHal3aObj(%p)",
            __FUNCTION__, mu4SensorDev, mi4SensorIdx, mpHal3aObj);
        return;
    }
}

MVOID
Hal3AAdapter3::
pause()
{
    MY_LOG("[%s] +", __FUNCTION__);
    mpHal3aObj->pause();
    MY_LOG("[%s] -", __FUNCTION__);
}

MVOID
Hal3AAdapter3::
resume(MINT32 MagicNum)
{
    MY_LOG("[%s] +", __FUNCTION__);
    mpHal3aObj->resume();
    MY_LOG("[%s] -", __FUNCTION__);
}


MBOOL
Hal3AAdapter3::
init(const char* strUser)
{
    Mutex::Autolock lock(mLock);
    MY_LOGD_IF((HAL3AADAPTER3_LOG_USERS & mu4LogEn), "[%s] m_i4SubsampleCount(%d), mi4User(%d)", __FUNCTION__, m_i4SubsampleCount, mi4User);

    GET_PROP("vendor.debug.hal3av3.aaatrace", 0, AAA_TRACE_LEVEL);

    std::string strName = std::string(strUser);
    ssize_t idxKey = m_Users.indexOfKey(strName);
    if (idxKey >= 0)
    {
        m_Users.editValueAt(idxKey)++;
    }
    else
    {
        m_Users.add(strName, 1);
    }

    if (mi4User > 0)
    {
        MY_LOGD_IF((HAL3AADAPTER3_LOG_USERS & mu4LogEn),
            "[%s] Still %d users", __FUNCTION__, mi4User);
    }
    else
    {
        doInit();
    }
    android_atomic_inc(&mi4User);

    return MTRUE;
}

MBOOL
Hal3AAdapter3::
uninit(const char* strUser)
{
    MY_LOGD_IF((HAL3AADAPTER3_LOG_USERS & mu4LogEn), "[%s+] m_i4SubsampleCount(%d), mi4User(%d)", __FUNCTION__, m_i4SubsampleCount, mi4User);
    Mutex::Autolock lock(mLock);

    if (mi4User > 0)
    {
        // More than one user, so decrease one User.
        android_atomic_dec(&mi4User);

        if (mi4User == 0) // There is no more User after decrease one User
        {
            doUninit();
        }
        else    // There are still some users.
        {
            MY_LOGD_IF((HAL3AADAPTER3_LOG_USERS & mu4LogEn),
                "[%s] Still %d users", __FUNCTION__, mi4User);
        }
    }

    std::string strName = std::string(strUser);
    ssize_t idxKey = m_Users.indexOfKey(strName);
    if (idxKey >= 0)
    {
        m_Users.editValueAt(idxKey)--;
    }
    else
    {
        CAM_LOGE("User(%s) did not create 3A!", strName.c_str());
    }

    return MTRUE;
}

MBOOL
Hal3AAdapter3::
doInitMetaResultQ()
{
    MY_LOGD("[%s] MetaResultQueueCapacity(%d) +", __FUNCTION__,mu4MetaResultQueueCapacity);

    MetaSet_T rInitMetaResult;
    mMetaResultQueue.clear();
    for (MUINT32 i = 0; i < mu4MetaResultQueueCapacity; i++)
        mMetaResultQueue.push_back(rInitMetaResult);

    if(mMetaResultQueue.empty())
        return MFALSE;

    MY_LOGD("[%s] +", __FUNCTION__);
    return MTRUE;
}

#if 0
const IMetadata*
Hal3AAdapter3::
queryStaticInfo() const
{
    return &mMetaStaticInfo;
}
#endif

MINT32
Hal3AAdapter3::
buildSceneModeOverride()
{
    // override
    const IMetadata::IEntry& entryAvailableScene = mMetaStaticInfo.entryFor(MTK_CONTROL_AVAILABLE_SCENE_MODES);
    if (entryAvailableScene.isEmpty())
    {
        CAM_LOGE("[%s] MTK_CONTROL_AVAILABLE_SCENE_MODES are not defined", __FUNCTION__);
    }
    else
    {
        const IMetadata::IEntry& entryScnOvrd = mMetaStaticInfo.entryFor(MTK_CONTROL_SCENE_MODE_OVERRIDES);
        if (entryScnOvrd.isEmpty())
        {
            CAM_LOGE("[%s] MTK_CONTROL_SCENE_MODE_OVERRIDES are not defined", __FUNCTION__);
        }
        else
        {
            MUINT32 i;
            MBOOL fgLog = (mu4LogEn & HAL3AADAPTER3_LOG_PF) ? MTRUE : MFALSE;
            for (i = 0; i < entryAvailableScene.count(); i++)
            {
                MUINT8 u1ScnMode = entryAvailableScene.itemAt(i, Type2Type< MUINT8 >());
                if (entryScnOvrd.count() >= 3*i)
                {
                    // override
                    Mode3A_T rMode3A;
                    rMode3A.u1AeMode  = entryScnOvrd.itemAt(3*i,   Type2Type< MUINT8 >());
                    rMode3A.u1AwbMode = entryScnOvrd.itemAt(3*i+1, Type2Type< MUINT8 >());
                    rMode3A.u1AfMode  = entryScnOvrd.itemAt(3*i+2, Type2Type< MUINT8 >());
                    m_ScnModeOvrd.add(u1ScnMode, rMode3A);
                    MY_LOGD_IF(fgLog, "[%s] Scene mode(%d) overrides AE(%d), AWB(%d), AF(%d)", __FUNCTION__, i, rMode3A.u1AeMode, rMode3A.u1AwbMode, rMode3A.u1AfMode);
                }
            }
        }
    }

    return 0;
}

MINT32
Hal3AAdapter3::
setupStaticInfo()
{
    MBOOL fgLog = (mu4LogEn & HAL3AADAPTER3_LOG_PF) ? MTRUE : MFALSE;

    buildSceneModeOverride();

    // AE Comp Step
    MRational rStep;
    if (QUERY_ENTRY_SINGLE(mMetaStaticInfo, MTK_CONTROL_AE_COMPENSATION_STEP, rStep))
    {
        mParams.fExpCompStep = (MFLOAT) rStep.numerator / rStep.denominator;
        MY_LOGD_IF(fgLog, "[%s] ExpCompStep(%3.3f), (%d/%d)", __FUNCTION__, mParams.fExpCompStep, rStep.numerator, rStep.denominator);
    }

    MINT32 rRgn[3];
    if (GET_ENTRY_ARRAY(mMetaStaticInfo, MTK_CONTROL_MAX_REGIONS, rRgn, 3))
    {
        m_rStaticInfo.i4MaxRegionAe  = rRgn[0];
        m_rStaticInfo.i4MaxRegionAwb = rRgn[1];
        m_rStaticInfo.i4MaxRegionAf  = rRgn[2];
        MY_LOGD_IF(fgLog, "[%s] Max Regions AE(%d) AWB(%d) AF(%d)", __FUNCTION__, rRgn[0], rRgn[1], rRgn[2]);
    }

    // active array size for normalizing
    if (QUERY_ENTRY_SINGLE(mMetaStaticInfo, MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION, mActiveArraySize))
    {
        MY_LOGD_IF(fgLog, "[%s] ActiveRgn(%d,%d,%d,%d)", __FUNCTION__, mActiveArraySize.p.x, mActiveArraySize.p.y, mActiveArraySize.s.w, mActiveArraySize.s.h);
    }
    // shading gain map size
    MSize rSdgmSize;
    if (QUERY_ENTRY_SINGLE(mMetaStaticInfo, MTK_LENS_INFO_SHADING_MAP_SIZE, rSdgmSize))
    {
        MY_LOGD_IF(fgLog, "[%s] gain map size (%dx%d)", __FUNCTION__, rSdgmSize.w, rSdgmSize.h);
        mParams.u1ShadingMapXGrid = rSdgmSize.w;
        mParams.u1ShadingMapYGrid = rSdgmSize.h;
    }
    // rolling shutter skew
        /*
    IMetadata::IEntry entry = metadata.entryFor(MTK_REQUEST_AVAILABLE_RESULT_KEYS);
    if(entry.tag() != IMetadata::IEntry::BAD_TAG
    {
        MUINT32 cnt = entry.count();
        for (MUINT32 i = 0; i < cnt; i++)
        {
            if (MTK_SENSOR_ROLLING_SHUTTER_SKEW == entry.itemAt(i, Type2Type<MINT32>())
            {
                MY_LOGD_IF(fgLog, "[%s] RollingShutterSkew(%d)", __FUNCTION__, m_u1RollingShutterSkew);
            }
        }
    }*/

    if (m_rStaticInfo.u1RollingShutterSkew = GET_ENTRY_SINGLE_IN_ARRAY(mMetaStaticInfo, MTK_REQUEST_AVAILABLE_RESULT_KEYS, (MINT32)MTK_SENSOR_ROLLING_SHUTTER_SKEW))
    {
        MY_LOGD_IF(fgLog, "[%s] RollingShutterSkew(%d)", __FUNCTION__, m_rStaticInfo.u1RollingShutterSkew);
        mParams.u1RollingShutterSkew = m_rStaticInfo.u1RollingShutterSkew;
    }
    else
    {
        mParams.u1RollingShutterSkew = 0;
    }
    return 0;
}

MINT32
Hal3AAdapter3::
convertResultToMeta(const Result_T& rResult, MetaSet_T& rMetaResult) const
{
    MUINT32 u4SensorDev = mu4SensorDev;
    MUINT32 i4SensorIdx = mi4SensorIdx;
    MINT32 i4SensorMode = mi4SensorMode;
    const StaticInfo_T& rstaticInfo = m_rStaticInfo;
    const IMetadata& staticMeta = mMetaStaticInfo;

    MBOOL fgLogPf = (mu4LogEn & HAL3AADAPTER3_LOG_PF) ? MTRUE: MFALSE;
    MBOOL fgLog = (mu4LogEn & HAL3AADAPTER3_LOG_GET_3) ? MTRUE : MFALSE;
    MBOOL fgLogMap = (mu4LogEn & HAL3AADAPTER3_LOG_GET_4) ? MTRUE : MFALSE;

    // convert result into metadata
    UPDATE_ENTRY_SINGLE(rMetaResult.appMeta, MTK_CONTROL_SCENE_MODE, rResult.u1SceneMode);
    UPDATE_ENTRY_SINGLE(rMetaResult.appMeta, MTK_CONTROL_AWB_STATE, rResult.u1AwbState);
    UPDATE_ENTRY_SINGLE(rMetaResult.appMeta, MTK_CONTROL_AE_STATE, rResult.u1AeState);
    UPDATE_ENTRY_SINGLE(rMetaResult.appMeta, MTK_CONTROL_AF_STATE, rResult.u1AfState);
    UPDATE_ENTRY_SINGLE(rMetaResult.appMeta, MTK_LENS_STATE, rResult.u1LensState);
    UPDATE_ENTRY_SINGLE(rMetaResult.appMeta, MTK_FLASH_STATE, rResult.u1FlashState);
    UPDATE_ENTRY_SINGLE(rMetaResult.appMeta, MTK_STATISTICS_SCENE_FLICKER, rResult.u1SceneFlk);
    UPDATE_ENTRY_SINGLE(rMetaResult.appMeta, MTK_HDR_FEATURE_HDR_DETECTION_RESULT, rResult.i4AutoHdrResult);
    UPDATE_ENTRY_SINGLE(rMetaResult.halMeta, MTK_3A_AE_BV_TRIGGER, rResult.fgAeBvTrigger);
    MY_LOGD_IF(fgLogPf, "[%s] #(%d) AWB(%d) AE(%d) AF(%d) Lens(%d) Flash(%d) Flk(%d), AutoHdr(%d), AE_BV_TRIGGER(%d)", __FUNCTION__,
        rResult.i4FrmId, rResult.u1AwbState, rResult.u1AeState, rResult.u1AfState, rResult.u1LensState, rResult.u1FlashState, rResult.u1SceneFlk, rResult.i4AutoHdrResult, rResult.fgAeBvTrigger);

    // sensor
    IDngInfo* pDngInfo = MAKE_DngInfo(LOG_TAG, i4SensorIdx);
    IMetadata rMetaDngDynNoiseProfile = pDngInfo->getDynamicNoiseProfile(rResult.i4SensorSensitivity);
    UPDATE_ENTRY_SINGLE(rMetaResult.appMeta, MTK_SENSOR_EXPOSURE_TIME, rResult.i8SensorExposureTime);
    UPDATE_ENTRY_SINGLE(rMetaResult.appMeta, MTK_SENSOR_FRAME_DURATION, rResult.i8SensorFrameDuration);
    UPDATE_ENTRY_SINGLE(rMetaResult.appMeta, MTK_SENSOR_SENSITIVITY, rResult.i4SensorSensitivity);
    if(rstaticInfo.u1RollingShutterSkew)
    {
        MY_LOGD_IF(fgLog, "[%s] RollingShutterSkew(%lld)", __FUNCTION__,rResult.i8SensorRollingShutterSkew);
        MY_LOGD_IF(fgLogPf, "[%s] ExpTime(%lld) FrmDuration(%lld) ISO(%d) RSS(%lld)", __FUNCTION__,
        rResult.i8SensorExposureTime, rResult.i8SensorFrameDuration, rResult.i4SensorSensitivity, rResult.i8SensorRollingShutterSkew);
    }
    else
    {
        MY_LOGD_IF(fgLogPf, "[%s] ExpTime(%lld) FrmDuration(%lld) ISO(%d)", __FUNCTION__,
        rResult.i8SensorExposureTime, rResult.i8SensorFrameDuration, rResult.i4SensorSensitivity);
    }
    UPDATE_ENTRY_SINGLE(rMetaResult.appMeta, MTK_SENSOR_GREEN_SPLIT, (MFLOAT) 0.0f);
    rMetaResult.appMeta += rMetaDngDynNoiseProfile;
    // lens
    MFLOAT fFNum, fFocusLength;
    if (QUERY_ENTRY_SINGLE(staticMeta, MTK_LENS_INFO_AVAILABLE_APERTURES, fFNum))
    {
        UPDATE_ENTRY_SINGLE(rMetaResult.appMeta, MTK_LENS_APERTURE, fFNum);
    }
    if (QUERY_ENTRY_SINGLE(staticMeta, MTK_LENS_INFO_AVAILABLE_FOCAL_LENGTHS, fFocusLength))
    {
        UPDATE_ENTRY_SINGLE(rMetaResult.appMeta, MTK_LENS_FOCAL_LENGTH, fFocusLength);
    }
    UPDATE_ENTRY_SINGLE(rMetaResult.appMeta, MTK_LENS_FOCUS_DISTANCE, rResult.fLensFocusDistance);
    UPDATE_ENTRY_ARRAY(rMetaResult.appMeta, MTK_LENS_FOCUS_RANGE, rResult.fLensFocusRange, 2);
    MY_LOGD_IF(fgLog, "[%s] FocusDist(%f), FocusRange(%f, %f)", __FUNCTION__, rResult.fLensFocusDistance, rResult.fLensFocusRange[0], rResult.fLensFocusRange[1]);

    // wb
    MRational NeutralColorPt[3];
    for (MINT32 k = 0; k < 3; k++)
    {
        NeutralColorPt[k].denominator = rResult.i4AwbGain[k];
        NeutralColorPt[k].numerator = rResult.i4AwbGainScaleUint;
    }
    UPDATE_ENTRY_ARRAY(rMetaResult.appMeta, MTK_SENSOR_NEUTRAL_COLOR_POINT, NeutralColorPt, 3);
    UPDATE_ENTRY_SINGLE(rMetaResult.appMeta, MTK_COLOR_CORRECTION_ABERRATION_MODE, (MUINT8) MTK_COLOR_CORRECTION_ABERRATION_MODE_OFF);
    UPDATE_ENTRY_ARRAY(rMetaResult.appMeta, MTK_COLOR_CORRECTION_GAINS, rResult.fColorCorrectGain, 4);
    MY_LOGD_IF(fgLog, "[%s] Awb Gain(%3.6f, %3.6f, %3.6f, %3.6f)", __FUNCTION__,
            rResult.fColorCorrectGain[0], rResult.fColorCorrectGain[1], rResult.fColorCorrectGain[2], rResult.fColorCorrectGain[3]);

    // color correction matrix
    if (rResult.vecColorCorrectMat.size())
    {
        const MFLOAT* pfMat = rResult.vecColorCorrectMat.array();
        IMetadata::IEntry entry(MTK_COLOR_CORRECTION_TRANSFORM);
        for (MINT32 k = 0; k < 9; k++)
        {
            MRational rMat;
            MFLOAT fVal = *pfMat++;
            rMat.numerator = fVal*512;
            rMat.denominator = 512;
            entry.push_back(rMat, Type2Type<MRational>());
            MY_LOGD_IF(fgLog, "[%s] Mat[%d] = (%3.6f, %d)", __FUNCTION__, k, fVal, rMat.numerator);
        }
        rMetaResult.appMeta.update(MTK_COLOR_CORRECTION_TRANSFORM, entry);
    }

    // shading
    MINT32 i4Size = rResult.vecLscData.size();
    if (i4Size)
    {
        IMetadata::Memory rLscData;
        rLscData.appendVector(rResult.vecLscData);
        UPDATE_ENTRY_SINGLE(rMetaResult.halMeta, MTK_LSC_TBL_DATA, rLscData);
        if (rResult.fgShadingMapOn)
        {
            MUINT8 u1ShadingMode = 0;
            QUERY_ENTRY_SINGLE(rMetaResult.appMeta, MTK_SHADING_MODE, u1ShadingMode);
            rMetaResult.appMeta += pDngInfo->getShadingMapFromMem(u1ShadingMode, rLscData);
        }
    }

    // tuning
    UPDATE_MEMORY(rMetaResult.halMeta, MTK_PROCESSOR_CAMINFO, rResult.rCamInfo);
    //UPDATE_ENTRY_ARRAY(rMetaResult.halMeta, MTK_PROCESSOR_CAMINFO, rResult.rCamInfo.data, sizeof(NSIspTuning::RAWIspCamInfo_U));

    // Exif
    if (rResult.vecExifInfo.size())
    {
        IMetadata metaExif;
        const EXIF_3A_INFO_T& rExifInfo = rResult.vecExifInfo[0];
        UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_FNUMBER,              fFNum*10/*rExifInfo.u4FNumber*/);
        UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_FOCAL_LENGTH,         fFocusLength*1000/*rExifInfo.u4FocalLength*/);
        //UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_SCENE_MODE,           rExifInfo.u4SceneMode);
        UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_AWB_MODE,             rExifInfo.u4AWBMode);
        UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_LIGHT_SOURCE,         rExifInfo.u4LightSource);
        UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_EXP_PROGRAM,          rExifInfo.u4ExpProgram);
        UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_SCENE_CAP_TYPE,       rExifInfo.u4SceneCapType);
        UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_FLASH_LIGHT_TIME_US,  rExifInfo.u4FlashLightTimeus);
        UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_AE_METER_MODE,        rExifInfo.u4AEMeterMode);
        UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_AE_EXP_BIAS,          rExifInfo.i4AEExpBias);
        UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_CAP_EXPOSURE_TIME,    rExifInfo.u4CapExposureTime);
        UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_AE_ISO_SPEED,         rExifInfo.u4AEISOSpeed);
        //UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_REAL_ISO_VALUE,       rExifInfo.u4RealISOValue);

        // debug info
        if (rResult.vecDbg3AInfo.size() && rResult.vecDbgShadTbl.size() && rResult.vecDbgIspInfo.size())
        {
            MY_LOGD_IF(fgLogMap, "[%s] DebugInfo #(%d)", __FUNCTION__, rResult.i4FrmId);
            UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_DBGINFO_AAA_KEY, AAA_DEBUG_KEYID);
            UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_DBGINFO_SDINFO_KEY, DEBUG_SHAD_TABLE_KEYID);
            UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_DBGINFO_ISP_KEY, ISP_DEBUG_KEYID);
            IMetadata::Memory dbg3A;
            IMetadata::Memory dbgIspP1;
            dbg3A.appendVector(rResult.vecDbg3AInfo);
            dbgIspP1.appendVector(rResult.vecDbgIspInfo);
            UPDATE_ENTRY_SINGLE(metaExif, MTK_3A_EXIF_DBGINFO_AAA_DATA, dbg3A);
            UPDATE_ENTRY_SINGLE(metaExif, MTK_3A_EXIF_DBGINFO_ISP_DATA, dbgIspP1);
        }

        // debug info for N3D
        if(rResult.vecDbgN3DInfo.size())
        {
            MY_LOGD_IF(fgLogMap, "[%s] DebugInfo #(%d) for N3D", __FUNCTION__, rResult.i4FrmId);
            UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_N3D_EXIF_DBGINFO_KEY, DEBUG_EXIF_MID_CAM_N3D);
            IMetadata::Memory dbgN3D;
            dbgN3D.appendVector(rResult.vecDbgN3DInfo);
            UPDATE_ENTRY_SINGLE(metaExif, MTK_N3D_EXIF_DBGINFO_DATA, dbgN3D);
        }

        UPDATE_ENTRY_SINGLE(rMetaResult.halMeta, MTK_3A_EXIF_METADATA, metaExif);
        UPDATE_ENTRY_SINGLE<MUINT8>(rMetaResult.halMeta, MTK_HAL_REQUEST_REQUIRE_EXIF, 1);
        UPDATE_ENTRY_SINGLE(rMetaResult.halMeta, MTK_ANALOG_GAIN, (MINT32)rResult.rCamInfo.rAEInfo.u4AfeGain);
        UPDATE_ENTRY_SINGLE(rMetaResult.halMeta, MTK_AWB_RGAIN, (MINT32)rResult.i4AwbGain[0]);
        UPDATE_ENTRY_SINGLE(rMetaResult.halMeta, MTK_AWB_GGAIN, (MINT32)rResult.i4AwbGain[1]);
        UPDATE_ENTRY_SINGLE(rMetaResult.halMeta, MTK_AWB_BGAIN, (MINT32)rResult.i4AwbGain[2]);
    }

    // log of control
    MINT32 rRgn[5];
    MUINT8 u1Mode;
    if (GET_ENTRY_ARRAY(rMetaResult.appMeta, MTK_CONTROL_AE_REGIONS, rRgn, 5))
    {
        MY_LOGD_IF(fgLog, "[%s] MTK_CONTROL_AE_REGIONS(%d,%d,%d,%d,%d)", __FUNCTION__,
            rRgn[0], rRgn[1], rRgn[2], rRgn[3], rRgn[4]);
    }
    if (GET_ENTRY_ARRAY(rMetaResult.appMeta, MTK_CONTROL_AF_REGIONS, rRgn, 5))
    {
        MY_LOGD_IF(fgLog, "[%s] MTK_CONTROL_AF_REGIONS(%d,%d,%d,%d,%d)", __FUNCTION__,
            rRgn[0], rRgn[1], rRgn[2], rRgn[3], rRgn[4]);
    }
    if (QUERY_ENTRY_SINGLE(rMetaResult.appMeta, MTK_COLOR_CORRECTION_ABERRATION_MODE, u1Mode))
    {
        MY_LOGD_IF(fgLog, "[%s] MTK_COLOR_CORRECTION_ABERRATION_MODE(%d)", __FUNCTION__, u1Mode);
    }
    if (QUERY_ENTRY_SINGLE(rMetaResult.appMeta, MTK_NOISE_REDUCTION_MODE, u1Mode))
    {
        MY_LOGD_IF(fgLog, "[%s] MTK_NOISE_REDUCTION_MODE(%d)", __FUNCTION__, u1Mode);
    }
    double noise_profile[8];
    if (GET_ENTRY_ARRAY(rMetaResult.appMeta, MTK_SENSOR_NOISE_PROFILE, noise_profile, 8))
    {
        MY_LOGD_IF(fgLog, "[%s] MTK_SENSOR_NOISE_PROFILE (%f,%f,%f,%f,%f,%f,%f,%f)", __FUNCTION__,
            noise_profile[0], noise_profile[1], noise_profile[2], noise_profile[3],
            noise_profile[4], noise_profile[5], noise_profile[6], noise_profile[7]);
    }

    //Focus area result.
    if( rResult.vecFocusAreaPos.size()!=0)
    {
        MINT32  szW = rResult.i4FocusAreaSz[0];
        MINT32  szH = rResult.i4FocusAreaSz[1];
        const MINT32 *ptrPos = rResult.vecFocusAreaPos.array();
        MINT32  cvtX = 0, cvtY = 0, cvtW = 0, cvtH = 0;
        android::Vector<MINT32> vecCvtPos = rResult.vecFocusAreaPos;
        MINT32 *ptrCvtPos = vecCvtPos.editArray();
        szW = szW/2;
        szH = szH/2;
        for( MUINT32 i=0; i<rResult.vecFocusAreaPos.size(); i+=2)
        {
            CameraArea_T pos;
            MINT32 X = *ptrPos++;
            MINT32 Y = *ptrPos++;

            pos.i4Left   = X - szW;
            pos.i4Top    = Y - szH;
            pos.i4Right  = X + szW;
            pos.i4Bottom = Y + szH;
            pos.i4Weight = 0;

            pos = _transformArea2Active(i4SensorIdx, i4SensorMode, pos);
            cvtX = (pos.i4Left + pos.i4Right )/2;
            cvtY = (pos.i4Top  + pos.i4Bottom)/2;
            *ptrCvtPos++ = cvtX;
            *ptrCvtPos++ = cvtY;

            cvtW = pos.i4Right  -  pos.i4Left;
            cvtH = pos.i4Bottom -  pos.i4Top;
        }
        if( fgLogPf)
        {
            for( MUINT32 i=0; i<vecCvtPos.size(); i++)
            {
                MY_LOGD_IF(fgLogPf, "Pos %d(%d)", rResult.vecFocusAreaPos[i], vecCvtPos[i]);
            }

            for( MUINT32 i=0; i<rResult.vecFocusAreaRes.size(); i++)
            {
                MY_LOGD_IF(fgLogPf, "Res (%d)", rResult.vecFocusAreaRes[i]);
            }
            MY_LOGD_IF(fgLogPf, "W %d(%d), H %d(%d)", rResult.i4FocusAreaSz[0], cvtW, rResult.i4FocusAreaSz[1], cvtH);
        }
        MSize areaCvtSZ(cvtW, cvtH);
        UPDATE_ENTRY_SINGLE(rMetaResult.halMeta, MTK_FOCUS_AREA_SIZE, areaCvtSZ);
        UPDATE_ENTRY_ARRAY(rMetaResult.halMeta, MTK_FOCUS_AREA_POSITION, vecCvtPos.array(), vecCvtPos.size());
        UPDATE_ENTRY_ARRAY(rMetaResult.halMeta, MTK_FOCUS_AREA_RESULT, rResult.vecFocusAreaRes.array(), rResult.vecFocusAreaRes.size());
    }

    // stereo warning message
    UPDATE_ENTRY_SINGLE(rMetaResult.appMeta, MTK_STEREO_FEATURE_WARNING, rResult.i4StereoWarning);
    return 0;
}

MINT32
Hal3AAdapter3::
convertP2ResultToMeta(const ResultP2_T& rResultP2, MetaSet_T* pResult) const
{
    MBOOL fgLog = (mu4LogEn & HAL3AADAPTER3_LOG_GET_4) ? MTRUE : MFALSE;
    MINT32 i4Size = 0;
    if (pResult != NULL)
    {
        // tonemap
        i4Size = rResultP2.vecTonemapCurveBlue.size();
        if (i4Size)
        {
            const MFLOAT* pCurve = rResultP2.vecTonemapCurveBlue.array();
            UPDATE_ENTRY_ARRAY(pResult->appMeta, MTK_TONEMAP_CURVE_BLUE, pCurve, i4Size);
            MY_LOGD_IF(fgLog, "[%s] B size(%d), P0(%f,%f), P_end(%f,%f)", __FUNCTION__, i4Size, pCurve[0], pCurve[1], pCurve[i4Size-2], pCurve[i4Size-1]);
        }
        i4Size = rResultP2.vecTonemapCurveGreen.size();
        if (i4Size)
        {
            const MFLOAT* pCurve = rResultP2.vecTonemapCurveGreen.array();
            UPDATE_ENTRY_ARRAY(pResult->appMeta, MTK_TONEMAP_CURVE_GREEN, pCurve, i4Size);
            MY_LOGD_IF(fgLog, "[%s] G size(%d), P0(%f,%f), P_end(%f,%f)", __FUNCTION__, i4Size, pCurve[0], pCurve[1], pCurve[i4Size-2], pCurve[i4Size-1]);
        }
        i4Size = rResultP2.vecTonemapCurveRed.size();
        if (i4Size)
        {
            const MFLOAT* pCurve = rResultP2.vecTonemapCurveRed.array();
            UPDATE_ENTRY_ARRAY(pResult->appMeta, MTK_TONEMAP_CURVE_RED, pCurve, i4Size);
            MY_LOGD_IF(fgLog, "[%s] R size(%d), P0(%f,%f), P_end(%f,%f)", __FUNCTION__, i4Size, pCurve[0], pCurve[1], pCurve[i4Size-2], pCurve[i4Size-1]);
        }
    }
    return 0;
}

MINT32
Hal3AAdapter3::
startCapture(const vector<MetaSet_T*>& requestQ, MINT32 i4StartNum)
{
    return 0;
}

MINT32
Hal3AAdapter3::
startRequestQ(const vector<MetaSet_T*>& requestQ)
{
    return 0;
}

MINT32
Hal3AAdapter3::
set(const vector<MetaSet_T*>& requestQ)
{
    return 0;
}

MINT32
Hal3AAdapter3::
preset(const std::vector<MetaSet_T*>& requestQ)
{
    return 0;
}

MINT32
Hal3AAdapter3::
setNormal(const vector<MetaSet_T*>& requestQ)
{
    return 0;
}

MINT32
Hal3AAdapter3::
setSMVR(const vector<MetaSet_T*>& requestQ)
{
    return 0;
}

MINT32
Hal3AAdapter3::
startCapture(const List<MetaSet_T>& requestQ, MINT32 /*i4StartNum*/, MINT32 i4RequestQSize)
{
    Mutex::Autolock lock(mLock);
    MY_LOGD("[%s]+ sensorDev(%d), sensorIdx(%d), mpHal3aObj(%p), RequestQSize(%d)",
        __FUNCTION__, mu4SensorDev, mi4SensorIdx, mpHal3aObj, i4RequestQSize);
    MINT32 i4Capture_type = ESTART_CAP_NORMAL;
    MINT32 i4RequestSize = 0;

    MBOOL ret = doInitMetaResultQ();
    if(!ret)
        CAM_LOGE("[%s]- Fail to metaResultQueue init", __FUNCTION__);

    mpHal3aObj->sendCommand(NS3Av3::ECmd_CameraPreviewStart);
    {
        // 3A can't operate requestQ, because will happen racing condition.
        // Let MW set requestQ size to 3A.
        // Reference CR ALPS03350011
        if(i4RequestQSize == -1)
            i4RequestSize = requestQ.size();
        else
            i4RequestSize = i4RequestQSize;

        MetaSetList_T::const_iterator it = requestQ.begin();
        MUINT8 u1AeMode = 0;

        for (int i = 0; i < getCapacity() && i < i4RequestSize; i++, it++)
        {
            if (i == HAL3A_REQ_PROC_KEY)
            {
                QUERY_ENTRY_SINGLE(it->appMeta, MTK_CONTROL_AE_MODE, u1AeMode);
                MY_LOGD("[%s] MTK_CONTROL_AE_MODE(%d)", __FUNCTION__, u1AeMode);
             }
        }
        if (u1AeMode == 0)
        {
            RequestSet_T rRequestSet;
            rRequestSet.vNumberSet.clear();
            rRequestSet.vNumberSet.push_back(1);
            NS3Av3::ParamIspProfile_T _3AProf(NSIspTuning::EIspProfile_Preview, 1, 0, MTRUE, NS3Av3::ParamIspProfile_T::EParamValidate_None, rRequestSet);
            mpHal3aObj->sendCommand(NS3Av3::ECmd_Update, reinterpret_cast<MUINTPTR>(&_3AProf));
            _3AProf.i4MagicNum = 2;
            _3AProf.rRequestSet.vNumberSet.clear();
            _3AProf.rRequestSet.vNumberSet.push_back(2);
            mpHal3aObj->sendCommand(NS3Av3::ECmd_Update, reinterpret_cast<MUINTPTR>(&_3AProf));
            MY_LOGD("[%s]- Dummy Update Sent, sensorDev(%d)", __FUNCTION__, mu4SensorDev);
        }
        else
        {
            MINT32 i4SensorModeDelay = 0;
            send3ACtrl(E3ACtrl_GetSensorSyncInfo, reinterpret_cast<MINTPTR>(&i4SensorModeDelay), NULL);
            MY_LOGD("[%s] i4SensorModeDelay(%d)", __FUNCTION__, i4SensorModeDelay);
            if (i4SensorModeDelay > 0)
                i4Capture_type = ESTART_CAP_SPECIAL;
            i4SensorModeDelay = i4SensorModeDelay - 2;
            if (i4SensorModeDelay < 0) i4SensorModeDelay = 0;
            RequestSet_T rRequestSet;
            NS3Av3::ParamIspProfile_T _3AProf(NSIspTuning::EIspProfile_Preview, 1, 0, MTRUE, NS3Av3::ParamIspProfile_T::EParamValidate_None, rRequestSet);
            for (int i = 1; i <= i4SensorModeDelay; i++)
            {
                _3AProf.i4MagicNum = i;
                rRequestSet.vNumberSet.clear();
                rRequestSet.vNumberSet.push_back(i);
                mpHal3aObj->sendCommand(NS3Av3::ECmd_Update, reinterpret_cast<MUINTPTR>(&_3AProf));
            }
        }
    }
    mu1Start = 1;

    mParams.u1IsStartCapture = MTRUE;
    set(requestQ, i4RequestSize);
    mParams.u1IsStartCapture = MFALSE;
    if (mParams.u4AeMode == 0)
    {
        MY_LOGD("[%s] manual capture", __FUNCTION__);
        for(int i = 3; i <= 5; i++)
        {
            RequestSet_T rRequestSet;
            rRequestSet.vNumberSet.clear();
            rRequestSet.vNumberSet.push_back(i);
            NS3Av3::ParamIspProfile_T _3AProf(NSIspTuning::EIspProfile_Preview, 1, 0, MTRUE, NS3Av3::ParamIspProfile_T::EParamValidate_None, rRequestSet);
            mpHal3aObj->sendCommand(NS3Av3::ECmd_Update, reinterpret_cast<MUINTPTR>(&_3AProf));
        }
        return ESTART_CAP_MANUAL;
    }
    else return i4Capture_type;
}

MINT32
Hal3AAdapter3::
startRequestQ(const android::List<MetaSet_T>& requestQ, MINT32 i4RequestQSize)
{
    Mutex::Autolock lock(mLock);

    MY_LOGD("[%s]+ sensorDev(%d), sensorIdx(%d), RequestQSize(%d)", __FUNCTION__, mu4SensorDev, mi4SensorIdx, i4RequestQSize);

    MBOOL ret = doInitMetaResultQ(); //start() don't call this
    if(!ret)
        CAM_LOGE("[%s]- Fail to metaResultQueue init", __FUNCTION__);

    MINT32 i4UpdatePreMetaRQIdx = 0;

    // 3A can't operate requestQ, because will happen racing condition.
    // Let MW set requestQ size to 3A.
    // Reference CR ALPS03350011
    MINT32 i4RequestSize = 0;
    if(i4RequestQSize == -1)
        i4RequestSize = requestQ.size();
    else
        i4RequestSize = i4RequestQSize;

    MetaSetList_T::const_iterator it = requestQ.begin();
    for (size_t i = 0; i < i4RequestSize; i++, it++)
    {
        if(i == (i4RequestSize-1) )
            set(requestQ, i4RequestSize);
        else
        {
            MY_LOGD("[%s] Ring buffer - MetaResult size(%d) Capacity(%d)", __FUNCTION__, mMetaResultQueue.size(), mu4MetaResultQueueCapacity);
            MetaSet_T& rMetaResult = getValidBuf(i4UpdatePreMetaRQIdx);
            MY_LOGD("[%s] Ring buffer - MetaResult(%p) Previous MetaResultQueue Idx(%d)", __FUNCTION__, &rMetaResult, i4UpdatePreMetaRQIdx);

            rMetaResult = (*it);
            // assign value to mMetaResult
            mMetaResult = rMetaResult;
        }
    }
    return MFALSE;
}

MINT32
Hal3AAdapter3::
set(const List<MetaSet_T>& requestQ, MINT32 i4RequestQSize)
{
    MetaSetList_T::const_iterator it = requestQ.begin();
    if (m_i4SubsampleCount > 1)  //Slow motion video recording
    {
        setSMVR(requestQ, i4RequestQSize);
        return 0;
    }
    else
    {
        setNormal(requestQ, i4RequestQSize);
        return 0;
    }
}

MINT32
Hal3AAdapter3::
setNormal(const List<MetaSet_T>& requestQ, MINT32 i4RequestQSize)
{
    AAA_TRACE_NG(setNormal);
    AAA_TRACE_L(setNormal);
    AAA_TRACE_NS(setNormal);

#define OVERRIDE_AE  (1<<0)
#define OVERRIDE_AWB (1<<1)
#define OVERRIDE_AF  (1<<2)


    MINT32 i;
    MINT32 i4FrmId = 0;
    MINT32 i4FrmIdCur = 0;
    MINT32 i4AfTrigId = 0;
    MINT32 i4AePreCapId = 0;
    MINT32 i4AeIsoSpeedMode = 0; // MTK AE feature
    MINT32 i4ForceFace3A = 0;
    MINT64 i8MinFrmDuration = 0;
    MUINT8 u1PauseAF = 0;
    MUINT8 u1MZOn = 255;
    MUINT8 u1AfTrig = MTK_CONTROL_AF_TRIGGER_IDLE;
    MUINT8 u1AePrecapTrig = MTK_CONTROL_AE_PRECAPTURE_TRIGGER_IDLE;
    MUINT8 u1CapIntent = MTK_CONTROL_CAPTURE_INTENT_PREVIEW;
    MUINT8 u1Override3A = (OVERRIDE_AE|OVERRIDE_AWB|OVERRIDE_AF);
    MUINT8 u1IspProfile = 0xFF;
    MUINT8 u1MtkHdrMode = static_cast<MUINT8>(HDRMode::OFF);

    MBOOL fgLog = (mu4LogEn & HAL3AADAPTER3_LOG_PF) ? MTRUE : MFALSE;
    MBOOL fgLogEn0 = (mu4LogEn & HAL3AADAPTER3_LOG_SET_0) ? MTRUE : MFALSE;
    MBOOL fgLogEn1 = (mu4LogEn & HAL3AADAPTER3_LOG_SET_1) ? MTRUE : MFALSE;
    MBOOL fgLogEn2 = (mu4LogEn & HAL3AADAPTER3_LOG_SET_2) ? MTRUE : MFALSE;
    MUINT32 u4ManualMode = 0;
    IMetadata::Memory capParams;
    RequestSet_T rRequestSet;
    rRequestSet.vNumberSet.clear();
    rRequestSet.vNumberSet.push_back(0);

    MINT32 i4TgWidth = 0;
    MINT32 i4TgHeight = 0;
    MINT32 i4HbinWidth = 0;
    MINT32 i4HbinHeight = 0;
    MINT32 i4Tg2HbinRatioW = 1;
    MINT32 i4Tg2HbinRatioH = 1;
    mpHal3aObj->queryTgSize(i4TgWidth,i4TgHeight);
    mpHal3aObj->queryHbinSize(i4HbinWidth,i4HbinHeight);
    i4Tg2HbinRatioW = i4TgWidth / i4HbinWidth;
    i4Tg2HbinRatioH = i4TgHeight / i4HbinHeight;

    // 3A can't operate requestQ, because will happen racing condition.
    // Let MW set requestQ size to 3A.
    // Reference CR ALPS03350011
    MINT32 i4RequestSize = 0;
    if(i4RequestQSize == -1)
        i4RequestSize = requestQ.size();
    else
        i4RequestSize = i4RequestQSize;

    MY_LOGD_IF(fgLog, "[%s] sensorDev(%d), sensorIdx(%d), mu4Counter(%d), queueSize(%d), RequestSize(%d) TG size(%d,%d) Hbin size(%d,%d) Ratio(%d,%d)", __FUNCTION__,
        mu4SensorDev, mi4SensorIdx, mu4Counter++, i4RequestQSize, i4RequestSize, i4TgWidth, i4TgHeight, i4HbinWidth, i4HbinHeight, i4Tg2HbinRatioW, i4Tg2HbinRatioH);

#if CAM3_STEREO_FEATURE_EN
    Stereo_Param_T rStereoParam;
#endif

    // reset
    mParams.u1IsGetExif = 0;
    mParams.u1HQCapEnable = 0;
    mParams.i4RawType = NSIspTuning::ERawType_Proc;
    mAfParams.u1AfTrig = 0;
    /*MINT32 i4BrightnessMode = 1;
    MINT32 i4ContrastMode = 1;
    MINT32 i4HueMode = 1;
    MINT32 i4SaturationMode = 1;
    MINT32 i4EdgeMode = 1;*/
    mParams.u1IsSingleFrameHDR = 0;
    //mParams.i8ExposureTime = 33000000;
    mParams.u1AeFakePreCap = MFALSE;
    MINT32 i4UpdatePreMetaRQIdx = 0;
    MUINT8 uRepeatTag = 0;

    mLockResult.lock();
    // startCapture init 6 buffer, cap request magic#(3) will put first Q-Idx, dummy will put next Q-Idx(Q = resultQ)
    // (magic(Q-idx))ex: 3(0) 0(1) 0(2) 0(3) 0(4) 0(5), dummy always put Q-idx(1) address
    MY_LOGD_IF(fgLogEn0, "[%s] Ring buffer - MetaResult size(%d) Capacity(%d)", __FUNCTION__, mMetaResultQueue.size(), mu4MetaResultQueueCapacity);
    MetaSet_T& rMetaResult = getValidBuf(i4UpdatePreMetaRQIdx);
    MY_LOGD_IF(fgLogEn0, "[%s] Ring buffer - Current MetaResult : (Idx, Addr)(%d, %p)", __FUNCTION__, i4UpdatePreMetaRQIdx, &rMetaResult);

    MetaSetList_T::const_iterator it = requestQ.begin();
    for (i = 0; i < getCapacity() && i < i4RequestSize; i++, it++)
    {
        MINT32 j, cnt;
        MRect rSclCropRect;
        MINT32 rSclCrop[4];
        MBOOL fgCrop = MFALSE;
        const IMetadata& metadata = it->appMeta;

        cnt = metadata.count();
        MY_LOGD_IF(fgLogEn0, "[%s] metadata[%d], count(%d)", __FUNCTION__, i, cnt);

        if (i == HAL3A_REQ_PROC_KEY)
        {
            AAA_TRACE_S(i==HAL3AKEY);
            MBOOL fgOK = QUERY_ENTRY_SINGLE(it->halMeta, MTK_P1NODE_PROCESSOR_MAGICNUM, i4FrmId);
            mParams.i4MagicNum = i4FrmId;
            mAfParams.i4MagicNum = i4FrmId;
            mAfParams.i4MagicNumCur = i4FrmIdCur;
            rRequestSet.vNumberSet.clear();
            rRequestSet.vNumberSet.push_back(((MINT32)(i4FrmId)));
            MY_LOGD_IF(fgLogEn0, "rRequestSet.vNumberSet[0]:%d",rRequestSet.vNumberSet[0]);
            if (fgOK)
            {
                //mMetaResult = *it;
                rMetaResult.halMeta = it->halMeta;
                MY_LOGD_IF(fgLogEn0, "[%s] magic(%d)", __FUNCTION__, i4FrmId);
            }
            else
            {
                CAM_LOGE("fail to get magic(%d)", i4FrmId);
            }

            //parsing repeat tag
            QUERY_ENTRY_SINGLE(it->halMeta, MTK_HAL_REQUEST_REPEAT, uRepeatTag);
            MY_LOGD_IF(fgLogEn1,"[%s] uRepeatTag(%d) i4FrmId(%d)", __FUNCTION__, uRepeatTag, i4FrmId);

            QUERY_ENTRY_SINGLE(it->halMeta, MTK_P1NODE_MIN_FRM_DURATION, i8MinFrmDuration);
            QUERY_ENTRY_SINGLE(it->halMeta, MTK_HAL_REQUEST_ISO_SPEED, i4AeIsoSpeedMode);

            MY_LOGD_IF(fgLogEn1,"[%s] i4AeIsoSpeedMode(%d)", __FUNCTION__, i4AeIsoSpeedMode);
            mParams.i4IsoSpeedMode = i4AeIsoSpeedMode;

/*            QUERY_ENTRY_SINGLE(it->halMeta, MTK_HAL_REQUEST_BRIGHTNESS_MODE, i4BrightnessMode);
            QUERY_ENTRY_SINGLE(it->halMeta, MTK_HAL_REQUEST_CONTRAST_MODE, i4ContrastMode);
            QUERY_ENTRY_SINGLE(it->halMeta, MTK_HAL_REQUEST_HUE_MODE, i4HueMode);
            QUERY_ENTRY_SINGLE(it->halMeta, MTK_HAL_REQUEST_SATURATION_MODE, i4SaturationMode);
            QUERY_ENTRY_SINGLE(it->halMeta, MTK_HAL_REQUEST_EDGE_MODE, i4EdgeMode); */
            QUERY_ENTRY_SINGLE(it->halMeta, MTK_3A_ISP_PROFILE, u1IspProfile);
            QUERY_ENTRY_SINGLE(it->halMeta, MTK_P1NODE_RAW_TYPE, mParams.i4RawType);

            MY_LOG_IF(fgLogEn0, "[%s] Dual PDAF support for EngMode", __FUNCTION__);
            QUERY_ENTRY_SINGLE(it->halMeta, MTK_HAL_REQUEST_PASS1_DISABLE, rRequestSet.fgDisableP1);
            MY_LOG_IF(fgLogEn0, "[%s] MTK_HAL_REQUEST_PASS1_DISABLE(%d)", __FUNCTION__, rRequestSet.fgDisableP1);

            // for HAL HDR
            QUERY_ENTRY_SINGLE(it->halMeta, MTK_3A_AE_CAP_PARAM, capParams);
            QUERY_ENTRY_SINGLE(it->halMeta, MTK_3A_AE_CAP_SINGLE_FRAME_HDR, mParams.u1IsSingleFrameHDR);
            QUERY_ENTRY_SINGLE(it->halMeta, MTK_3A_HDR_MODE, u1MtkHdrMode);
            MY_LOGD_IF(fgLogEn0, "[%s] u1MtkHdrMode(%d)", __FUNCTION__, u1MtkHdrMode);
            // AE Auto HDR, 0:OFF 1:ON 2:AUTO
            switch(static_cast<HDRMode>(u1MtkHdrMode))
            {
                case HDRMode::OFF:
                     mParams.u1HdrMode = 0;
                     break;
                case HDRMode::ON:
                case HDRMode::VIDEO_ON:
                     mParams.u1HdrMode = 1;
                     break;
                case HDRMode::AUTO:
                case HDRMode::VIDEO_AUTO:
                     mParams.u1HdrMode = 2;
                     break;
                default:
                     mParams.u1HdrMode = 0;
            }

            // for HAL to pause AF.
            if( QUERY_ENTRY_SINGLE(it->halMeta, MTK_FOCUS_PAUSE, u1PauseAF))
            {
                mAfParams.u1AfPause = u1PauseAF;
            }
            // for HAL to set MZ on/off.
            if( QUERY_ENTRY_SINGLE(it->halMeta, MTK_FOCUS_MZ_ON, u1MZOn))
            {
                mAfParams.u1MZOn = u1MZOn;
            }

            MUINT8 fgHQCapEnable = 0;
            fgOK = QUERY_ENTRY_SINGLE(it->halMeta, MTK_HAL_REQUEST_HIGH_QUALITY_CAP, fgHQCapEnable);
            mParams.u1HQCapEnable = fgHQCapEnable;
            MY_LOGD_IF(fgOK && fgLogEn0, "[%s] MTK_HAL_REQUEST_HIGH_QUALITY_CAP(%d)", __FUNCTION__, mParams.u1HQCapEnable);

            fgOK = QUERY_ENTRY_SINGLE(it->halMeta, MTK_HAL_REQUEST_REQUIRE_EXIF, mParams.u1IsGetExif);
            MY_LOGD_IF(fgOK && fgLogEn0, "[%s] Request EXIF(%d)", __FUNCTION__, mParams.u1IsGetExif);

            fgCrop = QUERY_ENTRY_SINGLE(metadata, MTK_SCALER_CROP_REGION, rSclCropRect);
            if (fgCrop)
            {
                mCropRegion.p.x = rSclCropRect.p.x;
                mCropRegion.p.y = rSclCropRect.p.y;
                mCropRegion.s.w = rSclCropRect.s.w;
                mCropRegion.s.h = rSclCropRect.s.h;
                rSclCrop[0] = rSclCropRect.p.x;
                rSclCrop[1] = rSclCropRect.p.y;
                rSclCrop[2] = rSclCropRect.p.x + rSclCropRect.s.w;
                rSclCrop[3] = rSclCropRect.p.y + rSclCropRect.s.h;
                MY_LOGD_IF(fgLogEn0, "[%s] SCL CROP(%d,%d,%d,%d)", __FUNCTION__, rSclCrop[0], rSclCrop[1], rSclCrop[2], rSclCrop[3]);

                // crop info for AE
                mParams.rScaleCropRect.i4Xoffset = rSclCropRect.p.x;
                mParams.rScaleCropRect.i4Yoffset = rSclCropRect.p.y;
                mParams.rScaleCropRect.i4Xwidth = rSclCropRect.s.w;
                mParams.rScaleCropRect.i4Yheight = rSclCropRect.s.h;

                // crop info for AF
                CameraArea_T& rArea = mAfParams.rScaleCropArea;
                rArea.i4Left   = rSclCrop[0];
                rArea.i4Top    = rSclCrop[1];
                rArea.i4Right  = rSclCrop[2];
                rArea.i4Bottom = rSclCrop[3];
                rArea = _transformArea(mi4SensorIdx,mi4SensorMode,rArea);
                rArea = _clipArea(i4TgWidth,i4TgHeight,rArea);

                MY_LOGD_IF(fgLogEn0, "[%s] AF SCL CROP(%d,%d,%d,%d)", __FUNCTION__, rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom);

                MINT32 i4ZoomRatio = 0;
                if( QUERY_ENTRY_SINGLE(it->halMeta, MTK_DUALZOOM_ZOOMRATIO, i4ZoomRatio))
                {
                    MY_LOGD_IF(fgLogEn0, "i4ZoomRatio(%d) from AP, SensorDev(%d)", i4ZoomRatio, mu4SensorDev);
                }
                else
                {
                    i4ZoomRatio = (((mActiveArraySize.s.w*100)/mCropRegion.s.w) < ((mActiveArraySize.s.h*100)/mCropRegion.s.h))?
                    ((mActiveArraySize.s.w*100)/mCropRegion.s.w):((mActiveArraySize.s.h*100)/mCropRegion.s.h);
                    MY_LOGD_IF(fgLogEn0, "i4ZoomRatio(%d) SensorDev(%d)", i4ZoomRatio, mu4SensorDev);
                }
                mParams.i4ZoomRatio = i4ZoomRatio;
            }

            // dummy request
            MUINT8 u1Dummy = 0;
            fgOK = QUERY_ENTRY_SINGLE(it->halMeta, MTK_HAL_REQUEST_DUMMY, u1Dummy);
            HAL3A_SETBIT(mParams.u4HalFlag, HAL_FLG_DUMMY2, (u1Dummy ? HAL_FLG_DUMMY2 : 0));

#if CAM3_STEREO_FEATURE_EN
            QUERY_ENTRY_SINGLE(it->halMeta, MTK_STEREO_FEATURE_DENOISE_MODE, mParams.i4DenoiseMode);
            QUERY_ENTRY_SINGLE(it->halMeta, MTK_STEREO_SYNC2A_MODE, rStereoParam.i4Sync2AMode);
            QUERY_ENTRY_SINGLE(it->halMeta, MTK_STEREO_SYNCAF_MODE, rStereoParam.i4SyncAFMode);
            QUERY_ENTRY_SINGLE(it->halMeta, MTK_STEREO_HW_FRM_SYNC_MODE, rStereoParam.i4HwSyncMode);
            MINT32 StereoData[2];
            if(GET_ENTRY_ARRAY(it->halMeta, MTK_STEREO_SYNC2A_MASTER_SLAVE, StereoData, 2))
            {
                rStereoParam.i4MasterIdx = StereoData[0];
                rStereoParam.i4SlaveIdx = StereoData[1];
            }
            MY_LOGD_IF(fgLog, "[%s] StereoParams : Sync2A(%d), SyncAF(%d), HwSync(%d), M&S Idx(%d,%d)", __FUNCTION__,
                rStereoParam.i4Sync2AMode,
                rStereoParam.i4SyncAFMode,
                rStereoParam.i4HwSyncMode,
                rStereoParam.i4MasterIdx,
                rStereoParam.i4SlaveIdx);
#endif

            MY_LOGD_IF(fgLog,
                "[%s] #(%d), dummy(%d), hqc(%d), exif(%d), SCL CROP(%d,%d,%d,%d), AF SCL CROP(%d,%d,%d,%d)",
                __FUNCTION__, i4FrmId, u1Dummy, mParams.u1HQCapEnable, mParams.u1IsGetExif,
                rSclCrop[0], rSclCrop[1], rSclCrop[2], rSclCrop[3],
                mAfParams.rScaleCropArea.i4Left, mAfParams.rScaleCropArea.i4Top, mAfParams.rScaleCropArea.i4Right, mAfParams.rScaleCropArea.i4Bottom);
            AAA_TRACE_END_S;
        }
        else if (i == 0)
        {
            AAA_TRACE_S(!ihalMeta);
            MBOOL fgOK = QUERY_ENTRY_SINGLE(it->halMeta, MTK_P1NODE_PROCESSOR_MAGICNUM, i4FrmIdCur);
            if (fgOK)
            {
                MY_LOGD_IF(fgLog, "[%s] current magic(%d)", __FUNCTION__, i4FrmIdCur);
            }
            else
            {
                CAM_LOGE("fail to get current magic(%d)", i4FrmIdCur);
            }

            MUINT8 u1Dummy = 0;
            fgOK = QUERY_ENTRY_SINGLE(it->halMeta, MTK_HAL_REQUEST_DUMMY, u1Dummy);
            MY_LOGD_IF(fgOK && u1Dummy, "[%s] current #(%d) is dummy", __FUNCTION__, i4FrmIdCur);
            HAL3A_SETBIT(mParams.u4HalFlag, HAL_FLG_DUMMY, (u1Dummy ? HAL_FLG_DUMMY : 0));
            AAA_TRACE_END_S;
            continue;
        }
        else
        {
            continue;
        }

        if (i == HAL3A_REQ_PROC_KEY)
        {
            if (!uRepeatTag) // not repeating tag, parse app meta
            {
                AAA_TRACE_S(i==HAL3AKEY-jLoop);
                mUpdateMetaResult.clear();
                for (j = 0; j < cnt; j++)
                {
                    IMetadata::IEntry entry = metadata.entryAt(j);
                    mtk_camera_metadata_tag_t tag = (mtk_camera_metadata_tag_t)entry.tag();
                    //MBOOL needUpdate = MTRUE;
#if 0
                    // for control only, ignore; for control+dynamic, not ignore.
                    MBOOL fgControlOnly = MFALSE;

                    // Only update 3A setting when delay matches.
                    if (i != getDelay(tag)) continue;
#endif

                    // convert metadata tag into 3A settings.
                    switch (tag)
                    {
                    case MTK_CONTROL_MODE:  // dynamic
                        {
                            MUINT8 u1ControlMode = entry.itemAt(0, Type2Type< MUINT8 >());
                            mParams.u1ControlMode = u1ControlMode;
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_MODE(%d)", __FUNCTION__, u1ControlMode);
                        }
                        break;
                    case MTK_CONTROL_CAPTURE_INTENT:
                        {
                            u1CapIntent = entry.itemAt(0, Type2Type< MUINT8 >());
                            mParams.u1CaptureIntent = u1CapIntent;
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_CAPTURE_INTENT(%d)", __FUNCTION__, u1CapIntent);
                        }
                        break;

                    // AWB
                    case MTK_CONTROL_AWB_LOCK:
                        {
                            MUINT8 bLock = entry.itemAt(0, Type2Type< MUINT8 >());
                            mParams.bIsAWBLock = (bLock==MTK_CONTROL_AWB_LOCK_ON) ? MTRUE : MFALSE;
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AWB_LOCK(%d)", __FUNCTION__, bLock);
                        }
                        break;
                    case MTK_CONTROL_AWB_MODE:  // dynamic
                        {
                            MUINT8 u1Mode = entry.itemAt(0, Type2Type< MUINT8 >());
                            mParams.u4AwbMode = u1Mode;
                            u1Override3A &= (~OVERRIDE_AWB);
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AWB_MODE(%d)", __FUNCTION__, u1Mode);
                        }
                        break;
                    case MTK_CONTROL_AWB_REGIONS:
                        {
                            if (m_rStaticInfo.i4MaxRegionAwb == 0)
                            {
                                //mMetaResult.appMeta.remove(MTK_CONTROL_AWB_REGIONS);
                            }
                            else
                            {
                                IMetadata::IEntry entryNew(MTK_CONTROL_AWB_REGIONS);
                                MINT32 numRgns = entry.count() / 5;
                                MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AWB_REGIONS(%d)", __FUNCTION__, numRgns);
                                for (MINT32 k = 0; k < numRgns; k++)
                                {
                                    CameraArea_T rArea;
                                    rArea.i4Left     = entry.itemAt(k*5+0, Type2Type< MINT32 >());
                                    rArea.i4Top      = entry.itemAt(k*5+1, Type2Type< MINT32 >());
                                    rArea.i4Right    = entry.itemAt(k*5+2, Type2Type< MINT32 >());
                                    rArea.i4Bottom   = entry.itemAt(k*5+3, Type2Type< MINT32 >());
                                    rArea.i4Weight   = entry.itemAt(k*5+4, Type2Type< MINT32 >());
                                    MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AWB_REGIONS L(%d) T(%d) R(%d) B(%d) W(%d)", __FUNCTION__, rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom, rArea.i4Weight);
                                    if (fgCrop)
                                    {
                                        rArea.i4Left   = MAX(rArea.i4Left, rSclCrop[0]);
                                        rArea.i4Top    = MAX(rArea.i4Top, rSclCrop[1]);
                                        rArea.i4Right  = MAX(MIN(rArea.i4Right, rSclCrop[2]), rSclCrop[0]);
                                        rArea.i4Bottom = MAX(MIN(rArea.i4Bottom, rSclCrop[3]), rSclCrop[1]);
                                    }
                                    entryNew.push_back(rArea.i4Left,   Type2Type<MINT32>());
                                    entryNew.push_back(rArea.i4Top,    Type2Type<MINT32>());
                                    entryNew.push_back(rArea.i4Right,  Type2Type<MINT32>());
                                    entryNew.push_back(rArea.i4Bottom, Type2Type<MINT32>());
                                    entryNew.push_back(rArea.i4Weight, Type2Type<MINT32>());
                                    rArea = _transformArea(mi4SensorIdx,mi4SensorMode,rArea);
                                    rArea = _clipArea(i4TgWidth,i4TgHeight,rArea);
                                    MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AWB_REGIONS mod L(%d) T(%d) R(%d) B(%d) W(%d)", __FUNCTION__, rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom, rArea.i4Weight);
                                }
                                mUpdateMetaResult.push({MTK_CONTROL_AWB_REGIONS, entryNew});
                            }
                        }
                        break;

                    // AE
                    case MTK_CONTROL_AE_ANTIBANDING_MODE:
                        {
                            MINT32 i4Mode = entry.itemAt(0, Type2Type< MUINT8 >());
                            mParams.u4AntiBandingMode = i4Mode;
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AE_ANTIBANDING_MODE(%d)", __FUNCTION__, i4Mode);
                        }
                        break;
                    case MTK_CONTROL_AE_EXPOSURE_COMPENSATION:
                        {
                            MINT32 i4ExpIdx = entry.itemAt(0, Type2Type< MINT32 >());
                            mParams.i4ExpIndex = i4ExpIdx;
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AE_EXPOSURE_COMPENSATION(%d)", __FUNCTION__, i4ExpIdx);
                        }
                        break;
                    case MTK_CONTROL_AE_LOCK:
                        {
                            MUINT8 bLock = entry.itemAt(0, Type2Type< MUINT8 >());
                            mParams.bIsAELock = (bLock==MTK_CONTROL_AE_LOCK_ON) ? MTRUE : MFALSE;
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AE_LOCK(%d)", __FUNCTION__, bLock);
                        }
                        break;
                    case MTK_CONTROL_AE_MODE:
                        {
                            MUINT8 u1Mode = entry.itemAt(0, Type2Type< MUINT8 >());
                            mParams.u4AeMode = u1Mode;
                            u1Override3A &= (~OVERRIDE_AE);
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AE_MODE(%d)", __FUNCTION__, u1Mode);
                        }
                        break;
                    case MTK_CONTROL_AE_REGIONS:    // dynamic
                        {
                            if (m_rStaticInfo.i4MaxRegionAe == 0)
                            {
                                //mMetaResult.appMeta.remove(MTK_CONTROL_AE_REGIONS);
                            }
                            else
                            {
                                IMetadata::IEntry entryNew(MTK_CONTROL_AE_REGIONS);
                                MINT32 numRgns = entry.count() / 5;
                                mParams.rMeteringAreas.u4Count = numRgns;
                                MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AE_REGIONS(%d)", __FUNCTION__, numRgns);
                                for (MINT32 k = 0; k < numRgns; k++)
                                {
                                    CameraArea_T& rArea = mParams.rMeteringAreas.rAreas[k];
                                    rArea.i4Left     = entry.itemAt(k*5+0, Type2Type< MINT32 >());
                                    rArea.i4Top      = entry.itemAt(k*5+1, Type2Type< MINT32 >());
                                    rArea.i4Right    = entry.itemAt(k*5+2, Type2Type< MINT32 >());
                                    rArea.i4Bottom   = entry.itemAt(k*5+3, Type2Type< MINT32 >());
                                    rArea.i4Weight   = entry.itemAt(k*5+4, Type2Type< MINT32 >());
                                    MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AE_REGIONS L(%d) T(%d) R(%d) B(%d) W(%d)", __FUNCTION__, rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom, rArea.i4Weight);
                                    if (fgCrop)
                                    {
                                        rArea.i4Left   = MAX(rArea.i4Left, rSclCrop[0]);
                                        rArea.i4Top    = MAX(rArea.i4Top, rSclCrop[1]);
                                        rArea.i4Right  = MAX(MIN(rArea.i4Right, rSclCrop[2]), rSclCrop[0]);
                                        rArea.i4Bottom = MAX(MIN(rArea.i4Bottom, rSclCrop[3]), rSclCrop[1]);
                                    }
                                    entryNew.push_back(rArea.i4Left,   Type2Type<MINT32>());
                                    entryNew.push_back(rArea.i4Top,    Type2Type<MINT32>());
                                    entryNew.push_back(rArea.i4Right,  Type2Type<MINT32>());
                                    entryNew.push_back(rArea.i4Bottom, Type2Type<MINT32>());
                                    entryNew.push_back(rArea.i4Weight, Type2Type<MINT32>());
                                    rArea = _transformArea(mi4SensorIdx,mi4SensorMode,rArea);
                                    rArea = _clipArea(i4TgWidth,i4TgHeight,rArea);
                                    MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AE_REGIONS mod L(%d) T(%d) R(%d) B(%d) W(%d)", __FUNCTION__, rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom, rArea.i4Weight);
                                    rArea.i4Left = rArea.i4Left       /i4Tg2HbinRatioW;
                                    rArea.i4Right= rArea.i4Right      /i4Tg2HbinRatioW;
                                    rArea.i4Top  = rArea.i4Top        /i4Tg2HbinRatioH;
                                    rArea.i4Bottom  = rArea.i4Bottom  /i4Tg2HbinRatioH;
                                    MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AE_REGIONS mod2 L(%d) T(%d) R(%d) B(%d) W(%d)", __FUNCTION__, rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom, rArea.i4Weight);
                                }
                                mUpdateMetaResult.push({MTK_CONTROL_AE_REGIONS, entryNew});
                            }
                        }
                        break;
                    case MTK_CONTROL_AE_TARGET_FPS_RANGE:
                        {
                            MINT32 i4MinFps = entry.itemAt(0, Type2Type< MINT32 >());
                            MINT32 i4MaxFps = entry.itemAt(1, Type2Type< MINT32 >());
                            mParams.i4MinFps = i4MinFps*1000;
                            mParams.i4MaxFps = i4MaxFps*1000;
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AE_TARGET_FPS_RANGE(%d,%d)", __FUNCTION__, i4MinFps, i4MaxFps);
                        }
                        break;
                    case MTK_CONTROL_AE_PRECAPTURE_TRIGGER:
                        {
                            u1AePrecapTrig = entry.itemAt(0, Type2Type< MUINT8 >());
                            if(u1AePrecapTrig == MTK_CONTROL_AE_PRECAPTURE_TRIGGER_START)
                                mParams.u1AeFakePreCap = MTRUE;
                            mParams.u1PrecapTrig = u1AePrecapTrig;
                            mAfParams.u1PrecapTrig = u1AePrecapTrig;
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AE_PRECAPTURE_TRIGGER(%d)", __FUNCTION__, u1AePrecapTrig);
                        }
                        break;
                    case MTK_CONTROL_AE_PRECAPTURE_ID:
                        {
                            i4AePreCapId = entry.itemAt(0, Type2Type< MINT32 >());
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AE_PRECAPTURE_ID(%d)", __FUNCTION__, i4AePreCapId);
                        }
                        break;

                    case MTK_FLASH_MODE:
                        {
                            MUINT8 u1Mode = entry.itemAt(0, Type2Type< MUINT8 >());
                            mParams.u4StrobeMode = u1Mode;
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_FLASH_MODE(%d)", __FUNCTION__, u1Mode);
                        }
                        break;

                    // Sensor
                    case MTK_SENSOR_EXPOSURE_TIME:
                        {
                            MINT64 i8AeExposureTime = entry.itemAt(0, Type2Type< MINT64 >());
                            mParams.i8ExposureTime = i8AeExposureTime;
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_SENSOR_EXPOSURE_TIME(%lld)", __FUNCTION__, i8AeExposureTime);
                        }
                        break;
                    case MTK_SENSOR_SENSITIVITY:
                        {
                            MINT32 i4AeSensitivity = entry.itemAt(0, Type2Type< MINT32 >());
                            mParams.i4Sensitivity = i4AeSensitivity;
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_SENSOR_SENSITIVITY(%d)", __FUNCTION__, i4AeSensitivity);
                        }
                        break;
                    case MTK_SENSOR_FRAME_DURATION:
                        {
                            MINT64 i8FrameDuration = entry.itemAt(0, Type2Type< MINT64 >());
                            mParams.i8FrameDuration = i8FrameDuration;
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_SENSOR_FRAME_DURATION(%lld)", __FUNCTION__, i8FrameDuration);
                        }
                        break;
                    case MTK_BLACK_LEVEL_LOCK:
                        {
                            MUINT8 u1BlackLvlLock = entry.itemAt(0, Type2Type< MUINT8 >());
                            mParams.u1BlackLvlLock = u1BlackLvlLock;
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_BLACK_LEVEL_LOCK(%d)", __FUNCTION__, u1BlackLvlLock);
                        }
                        break;

                    // AF
                    case MTK_CONTROL_AF_MODE:
                        {
                            MUINT8 u1AfMode = entry.itemAt(0, Type2Type< MUINT8 >());
                            mAfParams.u4AfMode = u1AfMode; //_convertAFMode(u1AfMode);
                            u1Override3A &= (~OVERRIDE_AF);
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AF_MODE(%d)", __FUNCTION__, u1AfMode);
                        }
                        break;
                    case MTK_CONTROL_AF_REGIONS:    // dynamic
                        {
                            if (m_rStaticInfo.i4MaxRegionAf == 0)
                            {
                                //mMetaResult.appMeta.remove(MTK_CONTROL_AF_REGIONS);
                            }
                            else
                            {
                                IMetadata::IEntry entryNew(MTK_CONTROL_AF_REGIONS);
                                MINT32 numRgns = entry.count() / 5;
                                mAfParams.rFocusAreas.u4Count = numRgns;
                                MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AF_REGIONS(%d)", __FUNCTION__, numRgns);
                                for (MINT32 k = 0; k < numRgns; k++)
                                {
                                    CameraArea_T& rArea = mAfParams.rFocusAreas.rAreas[k];
                                    rArea.i4Left     = entry.itemAt(k*5+0, Type2Type< MINT32 >());
                                    rArea.i4Top      = entry.itemAt(k*5+1, Type2Type< MINT32 >());
                                    rArea.i4Right    = entry.itemAt(k*5+2, Type2Type< MINT32 >());
                                    rArea.i4Bottom   = entry.itemAt(k*5+3, Type2Type< MINT32 >());
                                    rArea.i4Weight   = entry.itemAt(k*5+4, Type2Type< MINT32 >());
                                    MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AF_REGIONS L(%d) T(%d) R(%d) B(%d) W(%d)", __FUNCTION__, rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom, rArea.i4Weight);
                                    if (fgCrop)
                                    {
                                        rArea.i4Left   = MAX(rArea.i4Left, rSclCrop[0]);
                                        rArea.i4Top    = MAX(rArea.i4Top, rSclCrop[1]);
                                        rArea.i4Right  = MAX(MIN(rArea.i4Right, rSclCrop[2]), rSclCrop[0]);
                                        rArea.i4Bottom = MAX(MIN(rArea.i4Bottom, rSclCrop[3]), rSclCrop[1]);
                                    }
                                    entryNew.push_back(rArea.i4Left,   Type2Type<MINT32>());
                                    entryNew.push_back(rArea.i4Top,    Type2Type<MINT32>());
                                    entryNew.push_back(rArea.i4Right,  Type2Type<MINT32>());
                                    entryNew.push_back(rArea.i4Bottom, Type2Type<MINT32>());
                                    entryNew.push_back(rArea.i4Weight, Type2Type<MINT32>());
                                    rArea = _transformArea(mi4SensorIdx,mi4SensorMode,rArea);
                                    rArea = _clipArea(i4TgWidth,i4TgHeight,rArea);
                                    MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AF_REGIONS mod L(%d) T(%d) R(%d) B(%d) W(%d)", __FUNCTION__, rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom, rArea.i4Weight);
                                }
                                mUpdateMetaResult.push({MTK_CONTROL_AF_REGIONS, entryNew});
                            }
                        }
                        break;
                    case MTK_CONTROL_AF_TRIGGER:
                        {
                            u1AfTrig = entry.itemAt(0, Type2Type< MUINT8 >());
                            mAfParams.u1AfTrig = u1AfTrig;
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AF_TRIGGER(%d)", __FUNCTION__, u1AfTrig);
                        }
                        break;
                    case MTK_CONTROL_AF_TRIGGER_ID:
                        {
                            i4AfTrigId = entry.itemAt(0, Type2Type< MINT32 >());
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AF_TRIGGER_ID(%d)", __FUNCTION__, i4AfTrigId);
                        }
                        break;

                    // Lens
                    case MTK_LENS_FOCUS_DISTANCE:
                        {
                            MFLOAT fFocusDist = entry.itemAt(0, Type2Type< MFLOAT >());
                            mAfParams.fFocusDistance = fFocusDist;
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_LENS_FOCUS_DISTANCE(%3.6f)", __FUNCTION__, fFocusDist);
                        }
                        break;

                    // ISP
                    case MTK_CONTROL_EFFECT_MODE:
                        {
                            MUINT8 u1EffectMode = entry.itemAt(0, Type2Type< MUINT8 >());
                            mParams.u4EffectMode = u1EffectMode;
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_EFFECT_MODE(%d)", __FUNCTION__, u1EffectMode);
                        }
                        break;
                    case MTK_CONTROL_SCENE_MODE:
                        {
                            MUINT8 u1SceneMode = entry.itemAt(0, Type2Type< MUINT8 >());
                            mParams.u4SceneMode = u1SceneMode;
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_SCENE_MODE(%d)", __FUNCTION__, u1SceneMode);
                        }
                        break;
                    case MTK_EDGE_MODE:
                        {
                            MUINT8 u1EdgeMode = entry.itemAt(0, Type2Type< MUINT8 >());
                            mParams.u1EdgeMode = u1EdgeMode;
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_EDGE_MODE(%d)", __FUNCTION__, u1EdgeMode);
                        }
                        break;
                    case MTK_NOISE_REDUCTION_MODE:
                        {
                            MUINT8 u1NRMode = entry.itemAt(0, Type2Type< MUINT8 >());
                            mParams.u1NRMode = u1NRMode;
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_NOISE_REDUCTION_MODE(%d)", __FUNCTION__, u1NRMode);
                        }
                        break;

                    // Color correction
                    case MTK_COLOR_CORRECTION_MODE:
                        {
                            MUINT8 u1ColorCrctMode = entry.itemAt(0, Type2Type<MUINT8>());
                            mParams.u1ColorCorrectMode = u1ColorCrctMode;
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_COLOR_CORRECTION_MODE(%d)", __FUNCTION__, u1ColorCrctMode);
                        }
                        break;
                    case MTK_COLOR_CORRECTION_GAINS:
                        {
                            // [R G_even G_odd B]
                            MFLOAT fGains[4];
                            mParams.fColorCorrectGain[0] = fGains[0] = entry.itemAt(0, Type2Type<MFLOAT>());
                            mParams.fColorCorrectGain[1] = fGains[1] = entry.itemAt(1, Type2Type<MFLOAT>());
                            mParams.fColorCorrectGain[2] = fGains[2] = entry.itemAt(2, Type2Type<MFLOAT>());
                            mParams.fColorCorrectGain[3] = fGains[3] = entry.itemAt(3, Type2Type<MFLOAT>());
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_COLOR_CORRECTION_GAINS(%f, %f, %f, %f)", __FUNCTION__, fGains[0], fGains[1], fGains[2], fGains[3]);
                        }
                        break;
                    case MTK_COLOR_CORRECTION_TRANSFORM:
                        {
                            for (MINT32 k = 0; k < 9; k++)
                            {
                                MRational rMat;
                                MFLOAT fVal = 0.0f;
                                rMat = entry.itemAt(k, Type2Type<MRational>());
                                mParams.fColorCorrectMat[k] = fVal = rMat.denominator ? (MFLOAT)rMat.numerator / rMat.denominator : 0.0f;
                                MY_LOGD_IF(fgLogEn1, "[%s] MTK_COLOR_CORRECTION_TRANSFORM rMat[%d]=(%d/%d)=(%3.6f)", __FUNCTION__, k, rMat.numerator, rMat.denominator, fVal);
                            }
                        }
                        break;
                    case MTK_COLOR_CORRECTION_ABERRATION_MODE:
                        {
                            MUINT8 u1ColorAberrationMode = entry.itemAt(0, Type2Type<MUINT8>());
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_COLOR_CORRECTION_ABERRATION_MODE(%d)", __FUNCTION__, u1ColorAberrationMode);
                        }
                        break;
/*
                    // Tonemap
                    case MTK_TONEMAP_MODE:
                        {
                            MUINT8 u1TonemapMode = entry.itemAt(0, Type2Type<MUINT8>());
                            mParams.u1TonemapMode = u1TonemapMode;
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_TONEMAP_MODE(%d)", __FUNCTION__, u1TonemapMode);
                        }
                        break;
                    case MTK_TONEMAP_CURVE_BLUE:
                        {
                            MUINT32 u4Cnt = entry.count();
                            mParams.vecTonemapCurveBlue.resize(u4Cnt);
                            MFLOAT* fVec = mParams.vecTonemapCurveBlue.editArray();
                            for (MUINT32 i = 0; i < u4Cnt; i++)
                            {
                                *fVec++ = entry.itemAt(i, Type2Type< MFLOAT >());
                            }
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_TONEMAP_CURVE_BLUE(%d)", __FUNCTION__, u4Cnt);
                        }
                        break;
                    case MTK_TONEMAP_CURVE_GREEN:
                        {
                            MUINT32 u4Cnt = entry.count();
                            mParams.vecTonemapCurveGreen.resize(u4Cnt);
                            MFLOAT* fVec = mParams.vecTonemapCurveGreen.editArray();
                            for (MUINT32 i = 0; i < u4Cnt; i++)
                            {
                                *fVec++ = entry.itemAt(i, Type2Type< MFLOAT >());
                            }
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_TONEMAP_CURVE_GREEN(%d)", __FUNCTION__, u4Cnt);
                        }
                        break;
                    case MTK_TONEMAP_CURVE_RED:
                        {
                            MUINT32 u4Cnt = entry.count();
                            mParams.vecTonemapCurveRed.resize(u4Cnt);
                            MFLOAT* fVec = mParams.vecTonemapCurveRed.editArray();
                            for (MUINT32 i = 0; i < u4Cnt; i++)
                            {
                                *fVec++ = entry.itemAt(i, Type2Type< MFLOAT >());
                            }
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_TONEMAP_CURVE_RED(%d)", __FUNCTION__, u4Cnt);
                        }
                        break;
*/
                    // Shading
                    case MTK_SHADING_MODE:
                        {
                            MUINT8 u1ShadingMode = entry.itemAt(0, Type2Type<MUINT8>());
                            mParams.u1ShadingMode = u1ShadingMode;
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_SHADING_MODE(%d)", __FUNCTION__, u1ShadingMode);
                        }
                        break;
                    case MTK_STATISTICS_LENS_SHADING_MAP_MODE:
                        {
                            MUINT8 u1ShadingMapMode = entry.itemAt(0, Type2Type<MUINT8>());
                            mParams.u1ShadingMapMode = u1ShadingMapMode;
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_STATISTICS_LENS_SHADING_MAP_MODE(%d)", __FUNCTION__, u1ShadingMapMode);
                        }
                        break;
                    case MTK_FACE_FEATURE_FORCE_FACE_3A:
                        {
                            i4ForceFace3A = entry.itemAt(0, Type2Type<MINT32>());
                            MY_LOGD_IF(1, "[%s] MTK_FACE_FEATURE_FORCE_FACE_3A(%d)", __FUNCTION__, i4ForceFace3A);
                        }
                        break;
/*
                    case MTK_JPEG_THUMBNAIL_SIZE:
                    case MTK_JPEG_ORIENTATION:
                        needUpdate = MFALSE; // the tags do not update
                        break;
*/
                    default:
                        break;
                    }
/*
                    //if (!fgControlOnly && (HAL3A_REQ_PROC_KEY == i))
                    {
                        // update result for control/dynamic, including MTK_REQUEST_FRAME_COUNT
                        if (needUpdate)
                        rMetaResult.appMeta.update(tag, entry);
                    }
*/
                }
                AAA_TRACE_END_S;
                // face priority
                if (mParams.u4SceneMode == MTK_CONTROL_SCENE_MODE_FACE_PRIORITY || i4ForceFace3A == 1)
                {
                    if (mu1FdEnable != MTRUE)
                    {
                        mpHal3aObj->setFDEnable(MTRUE);
                        mu1FdEnable = MTRUE;
                    }
                }
                else
                {
                    if (mu1FdEnable != MFALSE)
                    {
                        MtkCameraFaceMetadata rFaceMeta;
                        MtkCameraFaceMetadata rAFFaceMeta;
                        rFaceMeta.number_of_faces = 0;
                        rAFFaceMeta.number_of_faces = 0;
                        mpHal3aObj->setFDInfo(&rFaceMeta, &rAFFaceMeta);
                        mpHal3aObj->setFDEnable(MFALSE);
                        mu1FdEnable = MFALSE;
                    }
                }
                rMetaResult.appMeta = (it->appMeta);
                rMetaResult.appMeta.remove(MTK_JPEG_THUMBNAIL_SIZE);
                rMetaResult.appMeta.remove(MTK_JPEG_ORIENTATION);
                // Update appMeta changed by 3A
                for(MINT32 k = 0; k < mUpdateMetaResult.size(); k++)
                {
                    rMetaResult.appMeta.update(mUpdateMetaResult[k].tag, mUpdateMetaResult[k].entry);
                }
            }
            else
            {
                u1Override3A = 0;//reduce AWB,AF,AE default value
                MetaSetList_T::iterator Preit = mMetaResultQueue.begin();
                for (MINT32 k = 0; Preit != mMetaResultQueue.end(); k++, Preit++)
                {
                    if(k == mi4PreMetaResultQueueIdx)
                    {
                        rMetaResult.appMeta = (Preit->appMeta);
                        break;
                    }
                }
                MY_LOGD_IF(fgLogEn0, "[%s] Set previous Idx(%d), current Addr(%p) size(%d)", __FUNCTION__, mi4PreMetaResultQueueIdx, &rMetaResult, rMetaResult.appMeta.count());
            }
            // assign value to mMetaResult
            mMetaResult = rMetaResult;

            mi4PreMetaResultQueueIdx = i4UpdatePreMetaRQIdx;
            MY_LOGD_IF(fgLogEn0, "[%s] Update Previous MetaResultQueue idx(%d)", __FUNCTION__, mi4PreMetaResultQueueIdx);
        }
    }

    MINT32 i4ForceDebugDump = 0;
    GET_PROP("vendor.debug.hal3av3.forcedump", 0, i4ForceDebugDump);
    if (i4ForceDebugDump)
    {
        mParams.u1IsGetExif = MTRUE;
        UPDATE_ENTRY_SINGLE<MUINT8>(rMetaResult.halMeta, MTK_HAL_REQUEST_REQUIRE_EXIF, 1);
    }

    mLockResult.unlock();

#if HAL3A_TEST_OVERRIDE
    GET_PROP("vendor.debug.hal3av3.manual", 0, u4ManualMode);
    _test(u4ManualMode, mParams);
#endif

    if (i4FrmId != 0)
    {
        // control mode and scene mode
        switch (mParams.u1ControlMode)
        {
        case MTK_CONTROL_MODE_OFF:
            mParams.u4AeMode  = MTK_CONTROL_AE_MODE_OFF;
            mParams.u4AwbMode = MTK_CONTROL_AWB_MODE_OFF;
            mParams.u4EffectMode = MTK_CONTROL_EFFECT_MODE_OFF;
            mAfParams.u4AfMode  = MTK_CONTROL_AF_MODE_OFF;
            break;
        case MTK_CONTROL_MODE_USE_SCENE_MODE:
            switch (mParams.u4SceneMode)
            {
            case MTK_CONTROL_SCENE_MODE_DISABLED:
            case MTK_CONTROL_SCENE_MODE_FACE_PRIORITY:
            case MTK_CONTROL_SCENE_MODE_NORMAL:
                break;
            default:
                {
                    if (!m_ScnModeOvrd.isEmpty())
                    {
                        Mode3A_T rOverride = m_ScnModeOvrd.valueFor((MUINT8)mParams.u4SceneMode);
                        mParams.u4AeMode  = (u1Override3A & OVERRIDE_AE) ? rOverride.u1AeMode : mParams.u4AeMode;
                        mParams.u4AwbMode = (u1Override3A & OVERRIDE_AWB) ? rOverride.u1AwbMode : mParams.u4AwbMode;
                        mAfParams.u4AfMode  = (u1Override3A & OVERRIDE_AF) ? rOverride.u1AfMode : mAfParams.u4AfMode; //_convertAFMode(rOverride.u1AfMode);
                        MY_LOGD_IF(fgLogEn2, "[%s] Scene mode(%d) overrides AE(%d), AWB(%d), AF(%d)", __FUNCTION__, mParams.u4SceneMode, mParams.u4AeMode, mParams.u4AwbMode, mAfParams.u4AfMode);
                    }
                    else
                    {
                        CAM_LOGE("[%s] Scene mode(%d) overrides Fails", __FUNCTION__, mParams.u4SceneMode);
                    }
                }
                break;
            }
            break;
        default:
            break;
        }
    }

    // AF trigger cancel
    if (u1AfTrig == MTK_CONTROL_AF_TRIGGER_CANCEL)
    {
        mpHal3aObj->cancelAutoFocus();
    }

    // workaround for p1 frame rate
    if (i8MinFrmDuration > 0)
    {
        MINT64 i4MaxFps = (1000000000LL*1000LL) / i8MinFrmDuration;
        mParams.i4MaxFps = mParams.i4MaxFps >= i4MaxFps ? i4MaxFps : mParams.i4MaxFps;
        if( mParams.i4MinFps > mParams.i4MaxFps )
            mParams.i4MinFps = mParams.i4MaxFps;
        mParams.i8FrameDuration = mParams.i8FrameDuration < i8MinFrmDuration ? i8MinFrmDuration : mParams.i8FrameDuration;
    }

    // set m_Params
    if(i4FrmId == 0 || uRepeatTag)
    {
        if(mParams.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_STILL_CAPTURE)
            mParams.u1CaptureIntent = MTK_CONTROL_CAPTURE_INTENT_PREVIEW;
        else if(mParams.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_VIDEO_SNAPSHOT)
            mParams.u1CaptureIntent = MTK_CONTROL_CAPTURE_INTENT_VIDEO_RECORD;
    }
    AAA_TRACE_S(setParams);

    // ISP profile
    NSIspTuning::EIspProfile_T prof = NSIspTuning::EIspProfile_Preview;

    if ( u1IspProfile == 0xFF){
        switch (mParams.u1CaptureIntent)
        {
        case MTK_CONTROL_CAPTURE_INTENT_PREVIEW:
            case MTK_CONTROL_CAPTURE_INTENT_ZERO_SHUTTER_LAG:
            prof = NSIspTuning::EIspProfile_Preview;
            break;
            case MTK_CONTROL_CAPTURE_INTENT_VIDEO_RECORD:
        case MTK_CONTROL_CAPTURE_INTENT_VIDEO_SNAPSHOT:
            prof = NSIspTuning::EIspProfile_Video;
            break;
            default:
                prof = NSIspTuning::EIspProfile_Preview;
            break;
        }
    }
    else{
        prof = static_cast<NSIspTuning::EIspProfile_T>(u1IspProfile);
    }

    MY_LOGD_IF(fgLog,"[%s] Update @ i4FrmId(%d), dummy(%d), IspProfile(%d)", __FUNCTION__, i4FrmId, (mParams.u4HalFlag & HAL_FLG_DUMMY2) ? 1 : 0, prof);
    NS3Av3::ParamIspProfile_T _3AProf(prof, i4FrmId, i4FrmIdCur, MTRUE, NS3Av3::ParamIspProfile_T::EParamValidate_All, rRequestSet);

    mParams.eIspProfile = prof;

    mpHal3aObj->setParams(mParams);
    mpHal3aObj->setAfParams(mAfParams);
#if CAM3_STEREO_FEATURE_EN
    mpHal3aObj->send3ACtrl(E3ACtrl_SetStereoParams, (MINTPTR)&rStereoParam, 0);
#endif
    AAA_TRACE_END_S;

    if (capParams.size() && mParams.u4SceneMode == MTK_CONTROL_SCENE_MODE_HDR)
    {
        // HAL HDR
        CaptureParam_T rCapParam;
        ::memcpy(&rCapParam, capParams.array(), sizeof(CaptureParam_T));
        mpHal3aObj->send3ACtrl(E3ACtrl_SetHalHdr, MTRUE, reinterpret_cast<MINTPTR>(&rCapParam));
        MY_LOGD_IF(fgLog, "[%s] HDR shot", __FUNCTION__);
    }
    else
    {
        // normal
        mpHal3aObj->send3ACtrl(E3ACtrl_SetHalHdr, MFALSE, NULL);
    }

    // AF trigger start
    if (u1AfTrig == MTK_CONTROL_AF_TRIGGER_START)
    {
        mpHal3aObj->autoFocus();
    }

    AAA_TRACE_S(SendCommands);
    if (u1AePrecapTrig == MTK_CONTROL_AE_PRECAPTURE_TRIGGER_START &&
        mParams.u4AeMode != MTK_CONTROL_AE_MODE_OFF)
    {
        MY_LOGD_IF(fgLog, "[%s] Precapture Trigger @ i4FrmId(%d)", __FUNCTION__, i4FrmId);
        mpHal3aObj->sendCommand(NS3Av3::ECmd_PrecaptureStart);
    } else if (u1AePrecapTrig == MTK_CONTROL_AE_PRECAPTURE_TRIGGER_CANCEL)
    {
        MY_LOGD_IF(fgLog, "[%s] Precapture Trigger Cancel @ i4FrmId(%d)", __FUNCTION__, i4FrmId);
        mpHal3aObj->sendCommand(NS3Av3::ECmd_PrecaptureEnd);
    }

    /*if (u1CapIntent == MTK_CONTROL_CAPTURE_INTENT_STILL_CAPTURE)
    {
        MY_LOGD("[%s] Capture Start @ i4FrmId(%d)", __FUNCTION__, i4FrmId);
        NS3Av3::ParamIspProfile_T _3AProf(NS3Av3::EIspProfile_Capture, i4FrmId, MTRUE, NS3Av3::ParamIspProfile_T::EParamValidate_All);
        mpHal3aObj->sendCommand(NS3Av3::ECmd_CaptureStart, reinterpret_cast<MINT32>(&_3AProf));
    }
    else*/
    {
        mpHal3aObj->sendCommand(NS3Av3::ECmd_Update, reinterpret_cast<MUINTPTR>(&_3AProf));

    }
    AAA_TRACE_END_S;

    AAA_TRACE_END_NG;
    AAA_TRACE_END_L;
    AAA_TRACE_END_NS;
    return 0;
}

MINT32
Hal3AAdapter3::
setSMVR(const List<MetaSet_T>& requestQ, MINT32 i4RequestQSize)
{
#define OVERRIDE_AE  (1<<0)
#define OVERRIDE_AWB (1<<1)
#define OVERRIDE_AF  (1<<2)

    CAM_TRACE_CALL();

    MINT32 i;
    MINT32 i4FrmId = 0;
    MINT32 i4FrmIdCur = 0;
    MINT32 i4AfTrigId = 0;
    MINT32 i4AePreCapId = 0;
    MINT32 i4AeIsoSpeedMode = 0; // MTK AE feature
    MINT64 i8MinFrmDuration = 0;
    MUINT8 u1AfTrig = MTK_CONTROL_AF_TRIGGER_IDLE;
    MUINT8 u1AfTrigSMVR = MTK_CONTROL_AF_TRIGGER_IDLE;
    MUINT8 u1AePrecapTrig = MTK_CONTROL_AE_PRECAPTURE_TRIGGER_IDLE;
    MUINT8 u1CapIntent =  MTK_CONTROL_CAPTURE_INTENT_VIDEO_RECORD;;
    MUINT8 u1Override3A = (OVERRIDE_AE|OVERRIDE_AWB|OVERRIDE_AF);
    MUINT8 u1IspProfile = 0xFF;

    MBOOL fgLog = (mu4LogEn & HAL3AADAPTER3_LOG_PF) ? MTRUE : MFALSE;
    MBOOL fgLogEn0 = (mu4LogEn & HAL3AADAPTER3_LOG_SET_0) ? MTRUE : MFALSE;
    MBOOL fgLogEn1 = (mu4LogEn & HAL3AADAPTER3_LOG_SET_1) ? MTRUE : MFALSE;
    MBOOL fgLogEn2 = (mu4LogEn & HAL3AADAPTER3_LOG_SET_2) ? MTRUE : MFALSE;
    MUINT32 u4ManualMode = 0;
    IMetadata::Memory capParams;
    RequestSet_T rRequestSet;
    rRequestSet.vNumberSet.clear();
    for (MINT32 i4InitCount = 0; i4InitCount < m_i4SubsampleCount; i4InitCount++) //avoid dummy request
        rRequestSet.vNumberSet.push_back(0);


    MINT32 i4TgWidth = 0;
    MINT32 i4TgHeight = 0;
    MINT32 i4HbinWidth = 0;
    MINT32 i4HbinHeight = 0;
    MINT32 i4Tg2HbinRatioW = 1;
    MINT32 i4Tg2HbinRatioH = 1;
    mpHal3aObj->queryTgSize(i4TgWidth,i4TgHeight);
    mpHal3aObj->queryHbinSize(i4HbinWidth,i4HbinHeight);
    i4Tg2HbinRatioW = i4TgWidth / i4HbinWidth;
    i4Tg2HbinRatioH = i4TgHeight / i4HbinHeight;
    // 3A can't operate requestQ, because will happen racing condition.
    // Let MW set requestQ size to 3A.
    // Reference CR ALPS03350011
    MINT32 i4requestSize = 0;
    //i4requestSize = requestQ.size();
    if(i4RequestQSize == -1)
        i4requestSize = requestQ.size();
    else
        i4requestSize = i4RequestQSize;

    MY_LOGD_IF(fgLog, "[%s] sensorDev(%d), sensorIdx(%d), mu4Counter(%d), queueSize(%d), RequestSize(%d), TG size(%d,%d) Hbin size(%d,%d) Ratio(%d,%d)", __FUNCTION__,
        mu4SensorDev, mi4SensorIdx, mu4Counter++, i4RequestQSize, i4requestSize, i4TgWidth, i4TgHeight, i4HbinWidth, i4HbinHeight, i4Tg2HbinRatioW, i4Tg2HbinRatioH);

    // reset
    mParams.u1IsGetExif = 0;
    mParams.u1HQCapEnable = 0;
    mParams.i4RawType = NSIspTuning::ERawType_Proc;
    mAfParams.u1AfTrig = 0;

    MY_LOGD_IF(fgLogEn0, "[%s]+", __FUNCTION__);

    MINT32 i4UpdatePreMetaRQIdx = 0;
    mLockResult.lock();

    MetaSetList_T::const_iterator it = requestQ.begin();
    for (MINT32 j = 0; j < (m_i4SubsampleCount * HAL3A_REQ_PROC_KEY); j++) it++;
    for (i = (m_i4SubsampleCount * HAL3A_REQ_PROC_KEY); i < ((m_i4SubsampleCount * HAL3A_REQ_PROC_KEY) + m_i4SubsampleCount) && i < i4requestSize; i++, it++)
    {
        MINT32 j, cnt;
        MRect rSclCropRect;
        MINT32 rSclCrop[4];
        MBOOL fgCrop = MFALSE;
        const IMetadata& metadata = it->appMeta;
        cnt = metadata.count();

        MBOOL fgOK = QUERY_ENTRY_SINGLE(it->halMeta, MTK_P1NODE_PROCESSOR_MAGICNUM, i4FrmId);

        const IMetadata& halMetadata = it->halMeta;
        MUINT8 uRepeatTag = 0;
        IMetadata::IEntry repeatEntry = halMetadata.entryFor(MTK_HAL_REQUEST_REPEAT);
        if (!repeatEntry.isEmpty())
        {
            QUERY_ENTRY_SINGLE(it->halMeta, MTK_HAL_REQUEST_REPEAT, uRepeatTag);
            MY_LOGD_IF(fgLogEn1,"[%s] uRepeatTag(%d) i4FrmId(%d)", __FUNCTION__, uRepeatTag, i4FrmId);
        }


        // find small magic ID address and idx
        MY_LOGD_IF(fgLogEn0, "[%s] Ring buffer - MetaResult size(%d) Capacity(%d)", __FUNCTION__, mMetaResultQueue.size(), mu4MetaResultQueueCapacity);
        MetaSet_T& rMetaResult = getValidBuf(i4UpdatePreMetaRQIdx);
        MY_LOGD_IF(fgLogEn0, "[%s] Ring buffer - rMetaResult(%p) Previous MetaResultQueue Idx(%d)", __FUNCTION__, &rMetaResult, i4UpdatePreMetaRQIdx);

        QUERY_ENTRY_SINGLE(it->halMeta, MTK_P1NODE_MIN_FRM_DURATION, i8MinFrmDuration);
        QUERY_ENTRY_SINGLE(it->halMeta, MTK_HAL_REQUEST_ISO_SPEED, i4AeIsoSpeedMode);
/*        QUERY_ENTRY_SINGLE(it->halMeta, MTK_HAL_REQUEST_BRIGHTNESS_MODE, mParams.i4BrightnessMode);
        QUERY_ENTRY_SINGLE(it->halMeta, MTK_HAL_REQUEST_CONTRAST_MODE, mParams.i4ContrastMode);
        QUERY_ENTRY_SINGLE(it->halMeta, MTK_HAL_REQUEST_HUE_MODE, mParams.i4HueMode);
        QUERY_ENTRY_SINGLE(it->halMeta, MTK_HAL_REQUEST_SATURATION_MODE, mParams.i4SaturationMode);
        QUERY_ENTRY_SINGLE(it->halMeta, MTK_HAL_REQUEST_EDGE_MODE, mParams.i4EdgeMode);*/
        QUERY_ENTRY_SINGLE(it->halMeta, MTK_3A_ISP_PROFILE, u1IspProfile);
        QUERY_ENTRY_SINGLE(it->halMeta, MTK_P1NODE_RAW_TYPE, mParams.i4RawType);
        fgCrop = QUERY_ENTRY_SINGLE(metadata, MTK_SCALER_CROP_REGION, rSclCropRect);
        if (fgCrop)
        {
            mCropRegion.p.x = rSclCropRect.p.x;
            mCropRegion.p.y = rSclCropRect.p.y;
            mCropRegion.s.w = rSclCropRect.s.w;
            mCropRegion.s.h = rSclCropRect.s.h;
            rSclCrop[0] = rSclCropRect.p.x;
            rSclCrop[1] = rSclCropRect.p.y;
            rSclCrop[2] = rSclCropRect.p.x + rSclCropRect.s.w;
            rSclCrop[3] = rSclCropRect.p.y + rSclCropRect.s.h;
            MY_LOGD_IF(fgLogEn0, "[%s] SCL CROP(%d,%d,%d,%d)", __FUNCTION__, rSclCrop[0], rSclCrop[1], rSclCrop[2], rSclCrop[3]);

            // crop info for AE
            mParams.rScaleCropRect.i4Xoffset = rSclCropRect.p.x;
            mParams.rScaleCropRect.i4Yoffset = rSclCropRect.p.y;
            mParams.rScaleCropRect.i4Xwidth = rSclCropRect.s.w;
            mParams.rScaleCropRect.i4Yheight = rSclCropRect.s.h;

            // crop info for AF
            CameraArea_T& rArea = mAfParams.rScaleCropArea;
            rArea.i4Left   = rSclCrop[0];
            rArea.i4Top    = rSclCrop[1];
            rArea.i4Right  = rSclCrop[2];
            rArea.i4Bottom = rSclCrop[3];
            rArea = _transformArea(mi4SensorIdx,mi4SensorMode,rArea);
            rArea = _clipArea(i4TgWidth,i4TgHeight,rArea);

            MY_LOGD_IF(fgLogEn0, "[%s] AF SCL CROP(%d,%d,%d,%d)", __FUNCTION__, rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom);

            MINT32 i4ZoomRatio = 0;
            if( QUERY_ENTRY_SINGLE(it->halMeta, MTK_DUALZOOM_ZOOMRATIO, i4ZoomRatio))
            {
                MY_LOGD_IF(fgLogEn0, "i4ZoomRatio(%d) from AP, SensorDev(%d)", i4ZoomRatio, mu4SensorDev);
            }
            else
            {
                i4ZoomRatio = (((mActiveArraySize.s.w*100)/mCropRegion.s.w) < ((mActiveArraySize.s.h*100)/mCropRegion.s.h))?
                ((mActiveArraySize.s.w*100)/mCropRegion.s.w):((mActiveArraySize.s.h*100)/mCropRegion.s.h);
                 MY_LOGD_IF(fgLogEn0, "i4ZoomRatio(%d) SensorDev(%d)", i4ZoomRatio, mu4SensorDev);
            }
            mParams.i4ZoomRatio = i4ZoomRatio;
        }

        if (fgOK && (i4requestSize >= m_i4SubsampleCount *(HAL3A_REQ_PROC_KEY + 1)))
        {
            //mMetaResult = *it;
            if(i == (m_i4SubsampleCount * HAL3A_REQ_PROC_KEY))
            {
                mParams.i4MagicNum = i4FrmId;
                mAfParams.i4MagicNum = i4FrmId;
                mAfParams.i4MagicNumCur = (i4FrmId > 2) ? i4FrmId - 2 : 0;
                rRequestSet.vNumberSet.clear();
            }
            if (!uRepeatTag) //not repeating tag, parse app meta
            {
                mUpdateMetaResult.clear();
                for (j = 0; j < cnt; j++)
                {
                    IMetadata::IEntry entry = metadata.entryAt(j);
                    MUINT32 tag = entry.tag();
                    //MBOOL needUpdate = MTRUE;
#if 0
                    // for control only, ignore; for control+dynamic, not ignore.
                    MBOOL fgControlOnly = MFALSE;

                    // Only update 3A setting when delay matches.
                    if (i != getDelay(tag)) continue;
#endif

                    // convert metadata tag into 3A settings.
                    switch (tag)
                    {
                    case MTK_CONTROL_MODE:  // dynamic
                        {
                            MUINT8 u1ControlMode = entry.itemAt(0, Type2Type< MUINT8 >());
                            mParams.u1ControlMode = u1ControlMode;
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_MODE(%d)", __FUNCTION__, u1ControlMode);
                        }
                        break;
                    case MTK_CONTROL_CAPTURE_INTENT:
                        {
                            u1CapIntent = entry.itemAt(0, Type2Type< MUINT8 >());
                            mParams.u1CaptureIntent = u1CapIntent;
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_CAPTURE_INTENT(%d)", __FUNCTION__, u1CapIntent);
                        }
                        break;

                    // AWB
                    case MTK_CONTROL_AWB_LOCK:
                        {
                            MUINT8 bLock = entry.itemAt(0, Type2Type< MUINT8 >());
                            mParams.bIsAWBLock = (bLock==MTK_CONTROL_AWB_LOCK_ON) ? MTRUE : MFALSE;
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AWB_LOCK(%d)", __FUNCTION__, bLock);
                        }
                        break;
                    case MTK_CONTROL_AWB_MODE:  // dynamic
                        {
                            MUINT8 u1Mode = entry.itemAt(0, Type2Type< MUINT8 >());
                            mParams.u4AwbMode = u1Mode;
                            u1Override3A &= (~OVERRIDE_AWB);
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AWB_MODE(%d)", __FUNCTION__, u1Mode);
                        }
                        break;
                    case MTK_CONTROL_AWB_REGIONS:
                        {
                            if ((m_rStaticInfo.i4MaxRegionAwb == 0) || (u1AfTrigSMVR == MTK_CONTROL_AF_TRIGGER_START))
                            {
                                if ((u1AfTrig != MTK_CONTROL_AF_TRIGGER_IDLE))
                                    MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AWB_REGIONS u1AfTrig(%d)", __FUNCTION__, u1AfTrig);
                                //mMetaResult.appMeta.remove(MTK_CONTROL_AWB_REGIONS);
                            }
                            else
                            {
                                IMetadata::IEntry entryNew(MTK_CONTROL_AWB_REGIONS);
                                MINT32 numRgns = entry.count() / 5;
                                MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AWB_REGIONS(%d)", __FUNCTION__, numRgns);
                                for (MINT32 k = 0; k < numRgns; k++)
                                {
                                    CameraArea_T rArea;
                                    rArea.i4Left     = entry.itemAt(k*5+0, Type2Type< MINT32 >());
                                    rArea.i4Top      = entry.itemAt(k*5+1, Type2Type< MINT32 >());
                                    rArea.i4Right    = entry.itemAt(k*5+2, Type2Type< MINT32 >());
                                    rArea.i4Bottom   = entry.itemAt(k*5+3, Type2Type< MINT32 >());
                                    rArea.i4Weight   = entry.itemAt(k*5+4, Type2Type< MINT32 >());
                                    MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AWB_REGIONS L(%d) T(%d) R(%d) B(%d) W(%d)", __FUNCTION__, rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom, rArea.i4Weight);
                                    if (fgCrop)
                                    {
                                        rArea.i4Left   = MAX(rArea.i4Left, rSclCrop[0]);
                                        rArea.i4Top    = MAX(rArea.i4Top, rSclCrop[1]);
                                        rArea.i4Right  = MAX(MIN(rArea.i4Right, rSclCrop[2]), rSclCrop[0]);
                                        rArea.i4Bottom = MAX(MIN(rArea.i4Bottom, rSclCrop[3]), rSclCrop[1]);
                                    }
                                    entryNew.push_back(rArea.i4Left,   Type2Type<MINT32>());
                                    entryNew.push_back(rArea.i4Top,    Type2Type<MINT32>());
                                    entryNew.push_back(rArea.i4Right,  Type2Type<MINT32>());
                                    entryNew.push_back(rArea.i4Bottom, Type2Type<MINT32>());
                                    entryNew.push_back(rArea.i4Weight, Type2Type<MINT32>());
                                    rArea = _transformArea(mi4SensorIdx,mi4SensorMode,rArea);
                                    rArea = _clipArea(i4TgWidth,i4TgHeight,rArea);
                                    MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AWB_REGIONS mod L(%d) T(%d) R(%d) B(%d) W(%d)", __FUNCTION__, rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom, rArea.i4Weight);
                                }
                                mUpdateMetaResult.push({MTK_CONTROL_AWB_REGIONS, entryNew});
                            }
                        }
                        break;

                    // AE
                    case MTK_CONTROL_AE_ANTIBANDING_MODE:
                        {
                            MINT32 i4Mode = entry.itemAt(0, Type2Type< MUINT8 >());
                            mParams.u4AntiBandingMode = i4Mode;
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AE_ANTIBANDING_MODE(%d)", __FUNCTION__, i4Mode);
                        }
                        break;
                    case MTK_CONTROL_AE_EXPOSURE_COMPENSATION:
                        {
                            MINT32 i4ExpIdx = entry.itemAt(0, Type2Type< MINT32 >());
                            mParams.i4ExpIndex = i4ExpIdx;
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AE_EXPOSURE_COMPENSATION(%d)", __FUNCTION__, i4ExpIdx);
                        }
                        break;
                    case MTK_CONTROL_AE_LOCK:
                        {
                            MUINT8 bLock = entry.itemAt(0, Type2Type< MUINT8 >());
                            mParams.bIsAELock = (bLock==MTK_CONTROL_AE_LOCK_ON) ? MTRUE : MFALSE;
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AE_LOCK(%d)", __FUNCTION__, bLock);
                        }
                        break;
                    case MTK_CONTROL_AE_MODE:
                        {
                            MUINT8 u1Mode = entry.itemAt(0, Type2Type< MUINT8 >());
                            mParams.u4AeMode = u1Mode;
                            u1Override3A &= (~OVERRIDE_AE);
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AE_MODE(%d)", __FUNCTION__, u1Mode);
                        }
                        break;
                    case MTK_CONTROL_AE_REGIONS:    // dynamic
                        {
                            if ((m_rStaticInfo.i4MaxRegionAe == 0) || (u1AfTrigSMVR == MTK_CONTROL_AF_TRIGGER_START))
                            {
                                if ((u1AfTrig != MTK_CONTROL_AF_TRIGGER_IDLE))
                                    MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AE_REGIONS u1AfTrig(%d)", __FUNCTION__, u1AfTrig);
                                //mMetaResult.appMeta.remove(MTK_CONTROL_AE_REGIONS);
                            }
                            else
                            {
                                IMetadata::IEntry entryNew(MTK_CONTROL_AE_REGIONS);
                                MINT32 numRgns = entry.count() / 5;
                                mParams.rMeteringAreas.u4Count = numRgns;
                                MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AE_REGIONS(%d)", __FUNCTION__, numRgns);
                                for (MINT32 k = 0; k < numRgns; k++)
                                {
                                    CameraArea_T& rArea = mParams.rMeteringAreas.rAreas[k];
                                    rArea.i4Left     = entry.itemAt(k*5+0, Type2Type< MINT32 >());
                                    rArea.i4Top      = entry.itemAt(k*5+1, Type2Type< MINT32 >());
                                    rArea.i4Right    = entry.itemAt(k*5+2, Type2Type< MINT32 >());
                                    rArea.i4Bottom   = entry.itemAt(k*5+3, Type2Type< MINT32 >());
                                    rArea.i4Weight   = entry.itemAt(k*5+4, Type2Type< MINT32 >());
                                    MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AE_REGIONS L(%d) T(%d) R(%d) B(%d) W(%d)", __FUNCTION__, rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom, rArea.i4Weight);
                                    if (fgCrop)
                                    {
                                        rArea.i4Left   = MAX(rArea.i4Left, rSclCrop[0]);
                                        rArea.i4Top    = MAX(rArea.i4Top, rSclCrop[1]);
                                        rArea.i4Right  = MAX(MIN(rArea.i4Right, rSclCrop[2]), rSclCrop[0]);
                                        rArea.i4Bottom = MAX(MIN(rArea.i4Bottom, rSclCrop[3]), rSclCrop[1]);
                                    }
                                    entryNew.push_back(rArea.i4Left,   Type2Type<MINT32>());
                                    entryNew.push_back(rArea.i4Top,    Type2Type<MINT32>());
                                    entryNew.push_back(rArea.i4Right,  Type2Type<MINT32>());
                                    entryNew.push_back(rArea.i4Bottom, Type2Type<MINT32>());
                                    entryNew.push_back(rArea.i4Weight, Type2Type<MINT32>());
                                    rArea = _transformArea(mi4SensorIdx,mi4SensorMode,rArea);
                                    rArea = _clipArea(i4TgWidth,i4TgHeight,rArea);
                                    MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AE_REGIONS mod L(%d) T(%d) R(%d) B(%d) W(%d)", __FUNCTION__, rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom, rArea.i4Weight);
                                    rArea.i4Left = rArea.i4Left       /i4Tg2HbinRatioW;
                                    rArea.i4Right= rArea.i4Right      /i4Tg2HbinRatioW;
                                    rArea.i4Top  = rArea.i4Top        /i4Tg2HbinRatioH;
                                    rArea.i4Bottom  = rArea.i4Bottom  /i4Tg2HbinRatioH;
                                    MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AE_REGIONS mod2 L(%d) T(%d) R(%d) B(%d) W(%d)", __FUNCTION__, rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom, rArea.i4Weight);
                                }
                                mUpdateMetaResult.push({MTK_CONTROL_AE_REGIONS, entryNew});
                            }
                        }
                        break;
                    case MTK_CONTROL_AE_TARGET_FPS_RANGE:
                        {
                            MINT32 i4MinFps = entry.itemAt(0, Type2Type< MINT32 >());
                            MINT32 i4MaxFps = entry.itemAt(1, Type2Type< MINT32 >());
                            mParams.i4MinFps = i4MinFps*1000;
                            mParams.i4MaxFps = i4MaxFps*1000;
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AE_TARGET_FPS_RANGE(%d,%d)", __FUNCTION__, i4MinFps, i4MaxFps);
                        }
                        break;
                    case MTK_CONTROL_AE_PRECAPTURE_TRIGGER:
                        {
                            MUINT8 u1Ae = 0;
                            u1Ae  = entry.itemAt(0, Type2Type< MUINT8 >());
                            if (u1AePrecapTrig < u1Ae)
                                u1AePrecapTrig = u1Ae;
                            mParams.u1PrecapTrig = u1AePrecapTrig;
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AE_PRECAPTURE_TRIGGER(%d)", __FUNCTION__, u1Ae);
                        }
                        break;
                    case MTK_CONTROL_AE_PRECAPTURE_ID:
                        {
                            i4AePreCapId = entry.itemAt(0, Type2Type< MINT32 >());
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AE_PRECAPTURE_ID(%d)", __FUNCTION__, i4AePreCapId);
                        }
                        break;
                    case MTK_FLASH_MODE:
                        {
                            MUINT8 u1Mode = entry.itemAt(0, Type2Type< MUINT8 >());
                            mParams.u4StrobeMode = u1Mode;
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_FLASH_MODE(%d)", __FUNCTION__, u1Mode);
                        }
                    break;

                    // AF
                    case MTK_CONTROL_AF_MODE:
                        {
                            MUINT8 u1AfMode = entry.itemAt(0, Type2Type< MUINT8 >());
                            mAfParams.u4AfMode = u1AfMode; //_convertAFMode(u1AfMode);
                            u1Override3A &= (~OVERRIDE_AF);
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AF_MODE(%d)", __FUNCTION__, u1AfMode);
                        }
                        break;
                    case MTK_CONTROL_AF_REGIONS:    // dynamic
                        {
                            if ((m_rStaticInfo.i4MaxRegionAf == 0) || (u1AfTrigSMVR == MTK_CONTROL_AF_TRIGGER_START))
                            {
                                //mMetaResult.appMeta.remove(MTK_CONTROL_AF_REGIONS);
                                if ((u1AfTrig != MTK_CONTROL_AF_TRIGGER_IDLE))
                                    MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AF_REGIONS u1AfTrig(%d)", __FUNCTION__, u1AfTrig);
                            }
                            else
                            {
                                IMetadata::IEntry entryNew(MTK_CONTROL_AF_REGIONS);
                                MINT32 numRgns = entry.count() / 5;
                                mAfParams.rFocusAreas.u4Count = numRgns;
                                MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AF_REGIONS(%d)", __FUNCTION__, numRgns);
                                for (MINT32 k = 0; k < numRgns; k++)
                                {
                                    CameraArea_T& rArea = mAfParams.rFocusAreas.rAreas[k];
                                    rArea.i4Left     = entry.itemAt(k*5+0, Type2Type< MINT32 >());
                                    rArea.i4Top      = entry.itemAt(k*5+1, Type2Type< MINT32 >());
                                    rArea.i4Right    = entry.itemAt(k*5+2, Type2Type< MINT32 >());
                                    rArea.i4Bottom   = entry.itemAt(k*5+3, Type2Type< MINT32 >());
                                    rArea.i4Weight   = entry.itemAt(k*5+4, Type2Type< MINT32 >());
                                    MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AF_REGIONS L(%d) T(%d) R(%d) B(%d) W(%d)", __FUNCTION__, rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom, rArea.i4Weight);
                                    if (fgCrop)
                                    {
                                        rArea.i4Left   = MAX(rArea.i4Left, rSclCrop[0]);
                                        rArea.i4Top    = MAX(rArea.i4Top, rSclCrop[1]);
                                        rArea.i4Right  = MAX(MIN(rArea.i4Right, rSclCrop[2]), rSclCrop[0]);
                                        rArea.i4Bottom = MAX(MIN(rArea.i4Bottom, rSclCrop[3]), rSclCrop[1]);
                                    }
                                    entryNew.push_back(rArea.i4Left,   Type2Type<MINT32>());
                                    entryNew.push_back(rArea.i4Top,    Type2Type<MINT32>());
                                    entryNew.push_back(rArea.i4Right,  Type2Type<MINT32>());
                                    entryNew.push_back(rArea.i4Bottom, Type2Type<MINT32>());
                                    entryNew.push_back(rArea.i4Weight, Type2Type<MINT32>());
                                    rArea = _transformArea(mi4SensorIdx,mi4SensorMode,rArea);
                                    rArea = _clipArea(i4TgWidth,i4TgHeight,rArea);
                                    MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AF_REGIONS mod L(%d) T(%d) R(%d) B(%d) W(%d)", __FUNCTION__, rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom, rArea.i4Weight);
                                }
                                mUpdateMetaResult.push({MTK_CONTROL_AF_REGIONS, entryNew});
                            }
                        }
                        break;
                    case MTK_CONTROL_AF_TRIGGER:
                        {
                            MUINT8 u1Af = 0;
                            u1Af  = entry.itemAt(0, Type2Type< MUINT8 >());
                            if (u1Af != MTK_CONTROL_AF_TRIGGER_IDLE)
                                u1AfTrig = u1Af;
                            mAfParams.u1AfTrig = u1AfTrig;
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AF_TRIGGER(%d)", __FUNCTION__, u1Af);
                        }
                        break;
                    case MTK_CONTROL_AF_TRIGGER_ID:
                        {
                            i4AfTrigId = entry.itemAt(0, Type2Type< MINT32 >());
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AF_TRIGGER_ID(%d)", __FUNCTION__, i4AfTrigId);
                        }
                        break;
                    // ISP
                    case MTK_CONTROL_EFFECT_MODE:
                        {
                            MUINT8 u1EffectMode = entry.itemAt(0, Type2Type< MUINT8 >());
                            mParams.u4EffectMode = u1EffectMode;
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_EFFECT_MODE(%d)", __FUNCTION__, u1EffectMode);
                        }
                        break;
                    case MTK_CONTROL_SCENE_MODE:
                        {
                            MUINT8 u1SceneMode = entry.itemAt(0, Type2Type< MUINT8 >());
                            mParams.u4SceneMode = u1SceneMode;
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_SCENE_MODE(%d)", __FUNCTION__, u1SceneMode);
                        }
                        break;
/*
                    case MTK_JPEG_THUMBNAIL_SIZE:
                    case MTK_JPEG_ORIENTATION:
                        needUpdate = MFALSE; // the tags do not update
                        break;
*/
                    default:
                        break;
                    }
                }
                CAM_TRACE_BEGIN("copy appMeta");
                rMetaResult.appMeta = (it->appMeta);
                rMetaResult.appMeta.remove(MTK_JPEG_THUMBNAIL_SIZE);
                rMetaResult.appMeta.remove(MTK_JPEG_ORIENTATION);
                if ((u1AfTrig == MTK_CONTROL_AF_TRIGGER_START))
                    u1AfTrigSMVR = MTK_CONTROL_AF_TRIGGER_START;
                // Update appMeta changed by 3A
                for(MINT32 k = 0; k < mUpdateMetaResult.size(); k++)
                {
                    rMetaResult.appMeta.update(mUpdateMetaResult[k].tag, mUpdateMetaResult[k].entry);
                }
                CAM_TRACE_END();
            }//not repeating tag, parse app meta

            MINT32 i4ForceDebugDump = 0;
            GET_PROP("vendor.debug.hal3av3.forcedump", 0, i4ForceDebugDump);
            if (i4ForceDebugDump)
            {
                mParams.u1IsGetExif = MTRUE;
                UPDATE_ENTRY_SINGLE<MUINT8>(rMetaResult.halMeta, MTK_HAL_REQUEST_REQUIRE_EXIF, 1);
            }

            rMetaResult.halMeta = it->halMeta;
            MY_LOGD_IF(fgLogEn0, "[%s] magic(%d)", __FUNCTION__, i4FrmId);
            //updateResult(i4FrmId, rMetaResult);
            // assign value to mMetaResult
            mMetaResult = rMetaResult;
            rRequestSet.vNumberSet.push_back(i4FrmId);
        }
        else
        {
            MY_LOGD("Warning!! fail to get magic(%d), check requestSize(%d)", i4FrmId, i4requestSize);
            break;
        }

    }

    mLockResult.unlock();

    // AF trigger cancel
    if (u1AfTrig == MTK_CONTROL_AF_TRIGGER_CANCEL)
    {
        mpHal3aObj->cancelAutoFocus();
    }

    // workaround for p1 frame rate
    if (i8MinFrmDuration > 0)
    {
        MINT64 i4MaxFps = (1000000000LL*1000LL) / i8MinFrmDuration;
        // mParams.i4MaxFps = mParams.i4MaxFps >= i4MaxFps ? i4MaxFps : mParams.i4MaxFps;
        if( mParams.i4MinFps > mParams.i4MaxFps )
            mParams.i4MinFps = mParams.i4MaxFps;
        mParams.i8FrameDuration = mParams.i8FrameDuration < i8MinFrmDuration ? i8MinFrmDuration : mParams.i8FrameDuration;
    }

    // ISP profile
    NSIspTuning::EIspProfile_T prof = NSIspTuning::EIspProfile_Preview;

    if ( u1IspProfile == 0xFF){
        switch (mParams.u1CaptureIntent)
        {
        case MTK_CONTROL_CAPTURE_INTENT_PREVIEW:
        case MTK_CONTROL_CAPTURE_INTENT_ZERO_SHUTTER_LAG:
            prof = NSIspTuning::EIspProfile_Preview;
            break;
            case MTK_CONTROL_CAPTURE_INTENT_VIDEO_RECORD:
        case MTK_CONTROL_CAPTURE_INTENT_VIDEO_SNAPSHOT:
            prof = NSIspTuning::EIspProfile_Video;
            break;
            default:
                prof = NSIspTuning::EIspProfile_Preview;
            break;
        }
    }
    else{
        prof = static_cast<NSIspTuning::EIspProfile_T>(u1IspProfile);
    }

    CAM_LOGE("Choo.Hal3A Prof: %d", prof);


    MY_LOGD_IF(fgLog,"[%s] Update @ i4FrmId(%d), dummy(%d), IspProfile(%d)", __FUNCTION__, i4FrmId, (mParams.u4HalFlag & HAL_FLG_DUMMY2) ? 1 : 0, prof);
    NS3Av3::ParamIspProfile_T _3AProf(prof, i4FrmId, i4FrmIdCur, MTRUE, NS3Av3::ParamIspProfile_T::EParamValidate_All, rRequestSet);

    mParams.eIspProfile = prof;


    mpHal3aObj->setParams(mParams); // set m_Params
    mpHal3aObj->setAfParams(mAfParams);


    // AF trigger start
    if (u1AfTrig == MTK_CONTROL_AF_TRIGGER_START)
    {
        mpHal3aObj->autoFocus();
    }

    if (u1AePrecapTrig == MTK_CONTROL_AE_PRECAPTURE_TRIGGER_START &&
        mParams.u4AeMode != MTK_CONTROL_AE_MODE_OFF)
    {
        MY_LOGD_IF(fgLog, "[%s] Precapture Trigger @ i4FrmId(%d)", __FUNCTION__, i4FrmId);
        mpHal3aObj->sendCommand(NS3Av3::ECmd_PrecaptureStart);
    }

    MY_LOGD_IF(fgLog,"[%s] Update @ i4FrmId(%d), dummy(%d)", __FUNCTION__, mParams.i4MagicNum, (mParams.u4HalFlag & HAL_FLG_DUMMY2) ? 1 : 0);
    mpHal3aObj->sendCommand(NS3Av3::ECmd_Update, reinterpret_cast<MUINTPTR>(&_3AProf));
    return 0;
}

MetaSet_T&
Hal3AAdapter3::
getValidBuf(MINT32& i4UpdatePreMetaRQIdx)
{
    //Mutex::Autolock lock(mLockResult);
    MINT32 i;
    MINT32 i4OldMagicNum = 0;
    MINT32 i4OldMagicNumIdx = 0;

    // use first Frame ID of metaResultQueue to find old magic number and index
    MetaSetList_T::iterator it = mMetaResultQueue.begin();
    QUERY_ENTRY_SINGLE(it->halMeta, MTK_P1NODE_PROCESSOR_MAGICNUM, i4OldMagicNum);

    for (i = 0; it != mMetaResultQueue.end(); i++, it++)
    {
        MINT32 u4Id = 0;
        QUERY_ENTRY_SINGLE(it->halMeta, MTK_P1NODE_PROCESSOR_MAGICNUM, u4Id);

        MY_LOGD_IF(mu4LogEn, "[%s] i4OldMagicNum(%d) u4Id(%d) i(%d) it(%p)",__FUNCTION__, i4OldMagicNum, u4Id, i, &(*(it)));

        if(i4OldMagicNum > u4Id)
        {
            i4OldMagicNumIdx = i;
            i4OldMagicNum = u4Id;//find small magic num
        }
    }

    // find old magic number address
    i4UpdatePreMetaRQIdx = i4OldMagicNumIdx;

    it = mMetaResultQueue.begin();
    for (i = 0; i < i4UpdatePreMetaRQIdx; i++)
    {
        it++;
    }

    MY_LOGD_IF(mu4LogEn, "[%s] ValidBuf (Idx, Addr)(%d, %p)", __FUNCTION__, i4UpdatePreMetaRQIdx, &(*(it)));
    return (*(it));
}


MINT32
Hal3AAdapter3::
updateResult(MINT32 i4FrmId, const MetaSet_T& rMetaResult)
{
    Mutex::Autolock lock(mLockResult);

    MINT32 i4Ret = -1;
    MINT32 i4Pos = 0;
    MINT32 i4Size = mMetaResultQueue.size();
    MetaSetList_T::iterator it = mMetaResultQueue.begin();
    for (; it != mMetaResultQueue.end(); it++, i4Pos++)
    {
        MINT32 u4Id = 0;
        QUERY_ENTRY_SINGLE(it->halMeta, MTK_P1NODE_PROCESSOR_MAGICNUM, u4Id);
        if (i4FrmId == u4Id)
        {
            *it = rMetaResult;
            i4Ret = 1;
            break;
        }
    }

    if (i4Pos == i4Size)
    {
        // does not exist, add
        mMetaResultQueue.push_back(rMetaResult);
        i4Ret = 0;
    }

    // remove item
    if (mMetaResultQueue.size() > mu4MetaResultQueueCapacity)
    {
        mMetaResultQueue.erase(mMetaResultQueue.begin());
    }

    mMetaResult = rMetaResult;

    return i4Ret;
}

MINT32
Hal3AAdapter3::
get(MUINT32 frmId, MetaSet_T& result)
{
    //Mutex::Autolock lock(mLock);
    Result_T rResult;
    MBOOL fgLog = mu4LogEn & HAL3AADAPTER3_LOG_PF;
    MBOOL fgLogEn0 = mu4LogEn & HAL3AADAPTER3_LOG_GET_0;
    MBOOL fgLogEn1 = mu4LogEn & HAL3AADAPTER3_LOG_GET_1;

    MY_LOGD_IF(fgLog, "[%s] sensorDev(%d), sensorIdx(%d) R(%d)", __FUNCTION__, mu4SensorDev, mi4SensorIdx, frmId);

    AAA_TRACE_D("getResult(%d)", frmId);
    // 1. get result (dynamic) of x from 3AMgr
    if (-1 == mpHal3aObj->getResult(frmId, rResult))
    {
        CAM_LOGW("[%s] Fail to get R[%d], current result will be obtained.", __FUNCTION__, frmId);
        return (-1);
    }
    AAA_TRACE_END_D;

    Mutex::Autolock lock(mLock);
    Mutex::Autolock lock_meta(mLockResult);
    // 2. copy result at x to MW
    MetaSetList_T::iterator it = mMetaResultQueue.begin();
    for (; it != mMetaResultQueue.end(); it++)
    {
        MINT32 u4Id = 0;
        QUERY_ENTRY_SINGLE(it->halMeta, MTK_P1NODE_PROCESSOR_MAGICNUM, u4Id);
        if (frmId == u4Id)
        {
            MINT32 i4ret = mpHal3aObj->queryRepeatQueue(frmId);
            if (i4ret != MTRUE) //should update non repeat metadata list
            {
                UPDATE_ENTRY_SINGLE(it->halMeta, MTK_3A_REPEAT_RESULT, (MUINT8)MFALSE);
                AAA_TRACE_D("_updateMeta(%d)", frmId);
                _updateMetadata(it->appMeta, result.appMeta);
                _updateMetadata(it->halMeta, result.halMeta);
                AAA_TRACE_END_D;
                AAA_TRACE_D("convertR2M(%d)", frmId);
                convertResultToMeta(rResult, result);
                AAA_TRACE_END_D;
                mMetaSet = result;
            }
            else {
                UPDATE_ENTRY_SINGLE(mMetaSet.halMeta, MTK_P1NODE_PROCESSOR_MAGICNUM, u4Id);
                UPDATE_ENTRY_SINGLE(mMetaSet.halMeta, MTK_3A_REPEAT_RESULT, (MUINT8)MTRUE);
                result = mMetaSet;
                MY_LOGD_IF(fgLog, "[%s] copy last metadata list(%d)", __FUNCTION__, u4Id);
            }
            break;
        }
    }

    // 3. fail to get the specified result, use current.
    if (it == mMetaResultQueue.end())
    {
        MINT32 i4Id = 0;
        UPDATE_ENTRY_SINGLE(mMetaResult.halMeta, MTK_3A_REPEAT_RESULT, (MUINT8)MFALSE);
        QUERY_ENTRY_SINGLE(mMetaResult.halMeta, MTK_P1NODE_PROCESSOR_MAGICNUM, i4Id);
        CAM_LOGW("[%s] fail to get the specified results, use current. i4Id(%d)", __FUNCTION__, i4Id);
        _updateMetadata(mMetaResult.appMeta, result.appMeta);
        _updateMetadata(mMetaResult.halMeta, result.halMeta);
        convertResultToMeta(rResult, result);
    }

    return 0;
}

MINT32
Hal3AAdapter3::
getCur(MUINT32 frmId, MetaSet_T& result)
{
    //Mutex::Autolock lock(mLock);

    Result_T rResult;
    MBOOL fgLog = mu4LogEn & HAL3AADAPTER3_LOG_PF;
    MBOOL fgLogEn0 = mu4LogEn & HAL3AADAPTER3_LOG_GET_0;
    MBOOL fgLogEn1 = mu4LogEn & HAL3AADAPTER3_LOG_GET_1;

    MY_LOGD_IF(fgLog, "[%s] sensorDev(%d), sensorIdx(%d) R(%d)", __FUNCTION__, mu4SensorDev, mi4SensorIdx, frmId);

    // 1. get result (dynamic) of x from 3AMgr
    if (-1 == mpHal3aObj->getResultCur(frmId, rResult))
    {
        CAM_LOGW("[%s] Fail to get R[%d], current result will be obtained.", __FUNCTION__, frmId);
    }

    Mutex::Autolock lock(mLock);
    Mutex::Autolock lock_meta(mLockResult);
    // 2. copy result at x to MW
    MetaSetList_T::iterator it = mMetaResultQueue.begin();
    for (; it != mMetaResultQueue.end(); it++)
    {
        MINT32 u4Id = 0;
        QUERY_ENTRY_SINGLE(it->halMeta, MTK_P1NODE_PROCESSOR_MAGICNUM, u4Id);
        if (frmId == u4Id)
        {
            _updateMetadata(it->appMeta, result.appMeta);
            _updateMetadata(it->halMeta, result.halMeta);
            convertResultToMeta(rResult, result);
            break;
        }
    }

    // 3. fail to get the specified result, use current.
    if (it == mMetaResultQueue.end())
    {
        MINT32 i4Id = 0;
        QUERY_ENTRY_SINGLE(mMetaResult.halMeta, MTK_P1NODE_PROCESSOR_MAGICNUM, i4Id);
        CAM_LOGW("[%s] fail to get the specified results, use current. i4Id(%d)", __FUNCTION__, i4Id);
        _updateMetadata(mMetaResult.appMeta, result.appMeta);
        _updateMetadata(mMetaResult.halMeta, result.halMeta);
        convertResultToMeta(rResult, result);
    }

    return 0;
}

static
inline MVOID
_reprocess(const MetaSet_T& control, MINT32 i4IspProfile, NSIspTuning::ISP_INFO_T& rReprocInput)
{
    MUINT8 u1PGN = MFALSE;
    QUERY_ENTRY_SINGLE(control.halMeta, MTK_3A_PGN_ENABLE, u1PGN);
    rReprocInput.rCamInfo.fgRPGEnable = !u1PGN;

    if (i4IspProfile != -1)
    {
        rReprocInput.rCamInfo.eIspProfile = (NSIspTuning::EIspProfile_T)i4IspProfile;
    }
    else
    {
        MUINT8 u1CapIntent = MTK_CONTROL_CAPTURE_INTENT_PREVIEW;
        QUERY_ENTRY_SINGLE(control.appMeta, MTK_CONTROL_CAPTURE_INTENT, u1CapIntent);

        switch (u1CapIntent)
        {
        case MTK_CONTROL_CAPTURE_INTENT_VIDEO_RECORD:
        case MTK_CONTROL_CAPTURE_INTENT_VIDEO_SNAPSHOT:
            rReprocInput.rCamInfo.eIspProfile = NSIspTuning::EIspProfile_Video;
            break;
        case MTK_CONTROL_CAPTURE_INTENT_PREVIEW:
        case MTK_CONTROL_CAPTURE_INTENT_ZERO_SHUTTER_LAG:
            rReprocInput.rCamInfo.eIspProfile = NSIspTuning::EIspProfile_Preview;
            break;
        case MTK_CONTROL_CAPTURE_INTENT_STILL_CAPTURE:
            if(rReprocInput.rCamInfo.rFlashInfo.isFlash){
                rReprocInput.rCamInfo.eIspProfile = NSIspTuning::EIspProfile_Flash_Capture;
            }else{
                rReprocInput.rCamInfo.eIspProfile = NSIspTuning::EIspProfile_Capture;
            }
            break;
        }
    }

    MINT32 i4RawType = NSIspTuning::ERawType_Proc;
    QUERY_ENTRY_SINGLE(control.halMeta, MTK_P1NODE_RAW_TYPE, i4RawType);
    rReprocInput.rCamInfo.i4RawType = i4RawType;

    IMetadata::Memory rLscData;
    if ((!rReprocInput.rCamInfo.fgRPGEnable) && (i4RawType == NSIspTuning::ERawType_Pure) && QUERY_ENTRY_SINGLE(control.halMeta, MTK_LSC_TBL_DATA, rLscData))
    {
        rReprocInput.rLscData = std::vector<MUINT8>(rLscData.array(), rLscData.array()+rLscData.size());
    }

    MINT32 i4UniqueKey=0;
    QUERY_ENTRY_SINGLE(control.halMeta, MTK_PIPELINE_UNIQUE_KEY, i4UniqueKey);
    rReprocInput.i4UniqueKey = i4UniqueKey;

    MRect rP1Crop;
    MSize rRzSize;
    if (QUERY_ENTRY_SINGLE(control.halMeta, MTK_P1NODE_SCALAR_CROP_REGION, rP1Crop) &&
        QUERY_ENTRY_SINGLE(control.halMeta, MTK_P1NODE_RESIZER_SIZE, rRzSize) &&
        (rReprocInput.rCamInfo.fgRPGEnable /* || rReprocInput.rCamInfo.eIspProfile == NSIspTuning::EIspProfile_N3D_Denoise */))
    {
        rReprocInput.rCamInfo.rCropRzInfo.i4OfstX    = rP1Crop.p.x;
        rReprocInput.rCamInfo.rCropRzInfo.i4OfstY    = rP1Crop.p.y;
        rReprocInput.rCamInfo.rCropRzInfo.i4Width    = rP1Crop.s.w;
        rReprocInput.rCamInfo.rCropRzInfo.i4Height   = rP1Crop.s.h;
        rReprocInput.rCamInfo.rCropRzInfo.i4RzWidth  = rRzSize.w;
        rReprocInput.rCamInfo.rCropRzInfo.i4RzHeight = rRzSize.h;
        rReprocInput.rCamInfo.rCropRzInfo.fgOnOff    = MTRUE;
    }
    else
    {
        rReprocInput.rCamInfo.rCropRzInfo.i4OfstX    = 0;
        rReprocInput.rCamInfo.rCropRzInfo.i4OfstY    = 0;
        rReprocInput.rCamInfo.rCropRzInfo.i4Width    = rReprocInput.rCamInfo.rCropRzInfo.i4FullW;
        rReprocInput.rCamInfo.rCropRzInfo.i4Height   = rReprocInput.rCamInfo.rCropRzInfo.i4FullH;
        rReprocInput.rCamInfo.rCropRzInfo.i4RzWidth  = rReprocInput.rCamInfo.rCropRzInfo.i4FullW;
        rReprocInput.rCamInfo.rCropRzInfo.i4RzHeight = rReprocInput.rCamInfo.rCropRzInfo.i4FullH;
        rReprocInput.rCamInfo.rCropRzInfo.fgOnOff = MFALSE;
    }

    MSize rP2OriginSize;
    MRect rP2CropRgn;
    MSize rP2RzSize;
    if (QUERY_ENTRY_SINGLE(control.halMeta, MTK_ISP_P2_ORIGINAL_SIZE, rP2OriginSize) &&
        QUERY_ENTRY_SINGLE(control.halMeta, MTK_ISP_P2_CROP_REGION, rP2CropRgn) &&
        QUERY_ENTRY_SINGLE(control.halMeta, MTK_ISP_P2_RESIZER_SIZE, rP2RzSize))
    {
        rReprocInput.rIspP2CropInfo.i4FullW     = rP2OriginSize.w;
        rReprocInput.rIspP2CropInfo.i4FullH     = rP2OriginSize.h;
        rReprocInput.rIspP2CropInfo.i4OfstX     = rP2CropRgn.p.x;
        rReprocInput.rIspP2CropInfo.i4OfstY     = rP2CropRgn.p.y;
        rReprocInput.rIspP2CropInfo.i4Width     = rP2CropRgn.s.w;
        rReprocInput.rIspP2CropInfo.i4Height    = rP2CropRgn.s.h;
        rReprocInput.rIspP2CropInfo.i4RzWidth   = rP2RzSize.w;
        rReprocInput.rIspP2CropInfo.i4RzHeight  = rP2RzSize.h;
        rReprocInput.rIspP2CropInfo.fgOnOff     = MTRUE;
    }
    else
    {
        rReprocInput.rIspP2CropInfo.i4FullW     = rReprocInput.rCamInfo.rCropRzInfo.i4FullW;
        rReprocInput.rIspP2CropInfo.i4FullH     = rReprocInput.rCamInfo.rCropRzInfo.i4FullH;
        rReprocInput.rIspP2CropInfo.i4OfstX     = 0;
        rReprocInput.rIspP2CropInfo.i4OfstY     = 0;
        rReprocInput.rIspP2CropInfo.i4Width     = rReprocInput.rCamInfo.rCropRzInfo.i4FullW;
        rReprocInput.rIspP2CropInfo.i4Height    = rReprocInput.rCamInfo.rCropRzInfo.i4FullH;
        rReprocInput.rIspP2CropInfo.i4RzWidth   = rReprocInput.rCamInfo.rCropRzInfo.i4FullW;
        rReprocInput.rIspP2CropInfo.i4RzHeight  = rReprocInput.rCamInfo.rCropRzInfo.i4FullH;
        rReprocInput.rIspP2CropInfo.fgOnOff     = MFALSE;
    }

    MUINT8 u1EdgeMode;
    if (QUERY_ENTRY_SINGLE(control.appMeta, MTK_EDGE_MODE, u1EdgeMode))
    {
        rReprocInput.rCamInfo.eEdgeMode = (mtk_camera_metadata_enum_android_edge_mode_t)u1EdgeMode;
    }

    MUINT8 u1NrMode;
    if (QUERY_ENTRY_SINGLE(control.appMeta, MTK_NOISE_REDUCTION_MODE, u1NrMode))
    {
        rReprocInput.rCamInfo.eNRMode = (mtk_camera_metadata_enum_android_noise_reduction_mode_t)u1NrMode;
    }

    MINT32 u4ISO;
    if (rReprocInput.rCamInfo.eIspProfile == NSIspTuning::EIspProfile_YUV_Reprocess)
    {
        QUERY_ENTRY_SINGLE(control.appMeta, MTK_SENSOR_SENSITIVITY, u4ISO);
        rReprocInput.rCamInfo.rAEInfo.u4RealISOValue= u4ISO;
        rReprocInput.rCamInfo.eIdx_Scene = (NSIspTuning::EIndex_Scene_T)0;  //MTK_CONTROL_SCENE_MODE_DISABLED
        rReprocInput.rCamInfo.eSensorMode = NSIspTuning::ESensorMode_Capture;
    }
}

IMetadata
Hal3AAdapter3::
getReprocStdExif(const MetaSet_T& control)
{
    IMetadata rMetaExif;

    MFLOAT fFNum = 0.0f;
    MFLOAT fFocusLength = 0.0f;
    MUINT8 u1AWBMode = 0;
    MINT32 u4LightSource = 0;
    MUINT8 u1SceneMode = 0;
    MINT32 u4ExpProgram = 0;
    MINT32 u4SceneCapType = 0;
    MUINT8 u1FlashState = 0;
    MINT32 u4FlashLightTimeus = 0;
    MINT32 u4AEComp = 0;
    MINT32 i4AEExpBias = 0;
    MINT32 u4AEISOSpeed = 0;
    MINT64 u8CapExposureTime = 0;
    QUERY_ENTRY_SINGLE(control.appMeta, MTK_LENS_APERTURE, fFNum);
    QUERY_ENTRY_SINGLE(control.appMeta, MTK_LENS_FOCAL_LENGTH , fFocusLength);
    QUERY_ENTRY_SINGLE(control.appMeta, MTK_CONTROL_AWB_MODE, u1AWBMode);

    //LightSource
    switch (u1AWBMode)
    {
        case MTK_CONTROL_AWB_MODE_AUTO:
        case MTK_CONTROL_AWB_MODE_WARM_FLUORESCENT:
        case MTK_CONTROL_AWB_MODE_TWILIGHT:
        case MTK_CONTROL_AWB_MODE_INCANDESCENT:
            u4LightSource = eLightSourceId_Other;
            break;
        case MTK_CONTROL_AWB_MODE_DAYLIGHT:
            u4LightSource = eLightSourceId_Daylight;
            break;
        case MTK_CONTROL_AWB_MODE_FLUORESCENT:
            u4LightSource = eLightSourceId_Fluorescent;
            break;
#if 0
        case MTK_CONTROL_AWB_MODE_TUNGSTEN:
            u4LightSource = eLightSourceId_Tungsten;
            break;
#endif
        case MTK_CONTROL_AWB_MODE_CLOUDY_DAYLIGHT:
            u4LightSource = eLightSourceId_Cloudy;
            break;
        case MTK_CONTROL_AWB_MODE_SHADE:
            u4LightSource = eLightSourceId_Shade;
            break;
        default:
            u4LightSource = eLightSourceId_Other;
            break;
        }

    QUERY_ENTRY_SINGLE(control.appMeta, MTK_CONTROL_SCENE_MODE, u1SceneMode);

    //EXP_Program
    switch (u1SceneMode)
    {
        case MTK_CONTROL_SCENE_MODE_PORTRAIT:
            u4ExpProgram = eExpProgramId_Portrait;
            break;
        case MTK_CONTROL_SCENE_MODE_LANDSCAPE:
            u4ExpProgram = eExpProgramId_Landscape;
            break;
        default:
            u4ExpProgram = eExpProgramId_NotDefined;
            break;
    }

    //SCENE_CAP_TYPE
    switch (u1SceneMode)
    {
        case MTK_CONTROL_SCENE_MODE_DISABLED:
        case MTK_CONTROL_SCENE_MODE_NORMAL:
        case MTK_CONTROL_SCENE_MODE_NIGHT_PORTRAIT:
        case MTK_CONTROL_SCENE_MODE_THEATRE:
        case MTK_CONTROL_SCENE_MODE_BEACH:
        case MTK_CONTROL_SCENE_MODE_SNOW:
        case MTK_CONTROL_SCENE_MODE_SUNSET:
        case MTK_CONTROL_SCENE_MODE_STEADYPHOTO:
        case MTK_CONTROL_SCENE_MODE_FIREWORKS:
        case MTK_CONTROL_SCENE_MODE_SPORTS:
        case MTK_CONTROL_SCENE_MODE_PARTY:
        case MTK_CONTROL_SCENE_MODE_CANDLELIGHT:
            u4SceneCapType = eCapTypeId_Standard;
            break;
        case MTK_CONTROL_SCENE_MODE_PORTRAIT:
            u4SceneCapType = eCapTypeId_Portrait;
            break;
        case MTK_CONTROL_SCENE_MODE_LANDSCAPE:
            u4SceneCapType = eCapTypeId_Landscape;
            break;
        case MTK_CONTROL_SCENE_MODE_NIGHT:
            u4SceneCapType = eCapTypeId_Night;
            break;
        default:
            u4SceneCapType = eCapTypeId_Standard;
            break;
    }

    //FlashTimeUs
    QUERY_ENTRY_SINGLE(control.appMeta, MTK_FLASH_STATE, u1FlashState);
    if (u1FlashState == MTK_FLASH_STATE_FIRED){
        u4FlashLightTimeus = 30000;
    }

    //AE_EXP_BIAS
    QUERY_ENTRY_SINGLE(control.appMeta, MTK_CONTROL_AE_EXPOSURE_COMPENSATION, u4AEComp );
    i4AEExpBias = mParams.fExpCompStep*u4AEComp*10;

    QUERY_ENTRY_SINGLE(control.appMeta, MTK_SENSOR_EXPOSURE_TIME, u8CapExposureTime);
    QUERY_ENTRY_SINGLE(control.appMeta, MTK_SENSOR_SENSITIVITY, u4AEISOSpeed);

    UPDATE_ENTRY_SINGLE<MINT32>(rMetaExif, MTK_3A_EXIF_FNUMBER,              fFNum*10/*rExifInfo.u4FNumber*/);
    UPDATE_ENTRY_SINGLE<MINT32>(rMetaExif, MTK_3A_EXIF_FOCAL_LENGTH,         fFocusLength*1000/*rExifInfo.u4FocalLength*/);
    UPDATE_ENTRY_SINGLE<MINT32>(rMetaExif, MTK_3A_EXIF_AWB_MODE,             u1AWBMode);
    UPDATE_ENTRY_SINGLE<MINT32>(rMetaExif, MTK_3A_EXIF_LIGHT_SOURCE,         u4LightSource);
    UPDATE_ENTRY_SINGLE<MINT32>(rMetaExif, MTK_3A_EXIF_EXP_PROGRAM,          u4ExpProgram);
    UPDATE_ENTRY_SINGLE<MINT32>(rMetaExif, MTK_3A_EXIF_SCENE_CAP_TYPE,       u4SceneCapType);
    UPDATE_ENTRY_SINGLE<MINT32>(rMetaExif, MTK_3A_EXIF_FLASH_LIGHT_TIME_US,  u4FlashLightTimeus);
    UPDATE_ENTRY_SINGLE<MINT32>(rMetaExif, MTK_3A_EXIF_AE_METER_MODE,        (MINT32)eMeteringMode_Average);
    UPDATE_ENTRY_SINGLE<MINT32>(rMetaExif, MTK_3A_EXIF_AE_EXP_BIAS,          i4AEExpBias);
    UPDATE_ENTRY_SINGLE<MINT32>(rMetaExif, MTK_3A_EXIF_CAP_EXPOSURE_TIME,    u8CapExposureTime/1000);
    UPDATE_ENTRY_SINGLE<MINT32>(rMetaExif, MTK_3A_EXIF_AE_ISO_SPEED,         u4AEISOSpeed);

    return rMetaExif;
}

static
inline MVOID
_reprocResult(NSIspTuning::ISP_INFO_T& rIspInfo, MetaSet_T* pResult)
{
    if (pResult == NULL)
        return;

    MUINT8 u1EdgeMode = rIspInfo.rCamInfo.eEdgeMode;
    UPDATE_ENTRY_SINGLE(pResult->appMeta, MTK_EDGE_MODE, u1EdgeMode);

    MUINT8 u1NrMode = rIspInfo.rCamInfo.eNRMode;
    UPDATE_ENTRY_SINGLE(pResult->appMeta, MTK_NOISE_REDUCTION_MODE, u1NrMode);
}

static
inline MBOOL
_dumpDebugInfo(const char* filename, const IMetadata& metaExif)
{
    FILE* fid = fopen(filename, "wb");
    if (fid)
    {
        IMetadata::Memory p3ADbg;
        if (IMetadata::getEntry<IMetadata::Memory>(&metaExif, MTK_3A_EXIF_DBGINFO_AAA_DATA, p3ADbg))
        {
            MY_LOGD("[%s] %s, 3A(%p, %d)", __FUNCTION__, filename, p3ADbg->array(), p3ADbg->size());
            MUINT8 hdr[6] = {0, 0, 0xFF, 0xE6, 0, 0};
            MUINT16 size = (MUINT16)(p3ADbg->size()+2);
            hdr[4] = (size >> 8); // big endian
            hdr[5] = size & 0xFF;
            fwrite(hdr, 6, 1, fid);
            fwrite(p3ADbg->array(), p3ADbg->size(), 1, fid);
        }
        IMetadata::Memory pIspDbg;
        if (IMetadata::getEntry<IMetadata::Memory>(&metaExif, MTK_3A_EXIF_DBGINFO_ISP_DATA, pIspDbg))
        {
            MY_LOGD("[%s] %s, ISP(%p, %d)", __FUNCTION__, filename, pIspDbg->array(), pIspDbg->size());
            MUINT8 hdr[4] = {0xFF, 0xE7, 0, 0};
            MUINT16 size = (MUINT16)(pIspDbg->size()+2);
            hdr[2] = (size >> 8);
            hdr[3] = size & 0xFF;
            fwrite(hdr, 4, 1, fid);
            fwrite(pIspDbg->array(), pIspDbg->size(), 1, fid);
        }
        fclose(fid);
        return MTRUE;
    }
    return MFALSE;
}

MBOOL
Hal3AAdapter3::
setP2Params(P2Param_T &rNewP2Param, const MetaSet_T& control,ResultP2_T* pResultP2, NSIspTuning::ISP_INFO_T& rReprocInput)
{
    MBOOL fgLog = (mu4LogEn & HAL3AADAPTER3_LOG_PF) ? MTRUE : MFALSE;
    MBOOL fgLogEn0 = (mu4LogEn & HAL3AADAPTER3_LOG_SET_0) ? MTRUE : MFALSE;
    MBOOL fgLogEn1 = (mu4LogEn & HAL3AADAPTER3_LOG_SET_1) ? MTRUE : MFALSE;
    MBOOL fgLogEn2 = (mu4LogEn & HAL3AADAPTER3_LOG_SET_2) ? MTRUE : MFALSE;

    MY_LOGD_IF(fgLog, "[%s] +", __FUNCTION__);


    MINT32 i4Id;
    MUINT32 u4ManualMode = 0;
    MINT32 i4BrightnessMode = 1;
    MINT32 i4ContrastMode = 1;
    MINT32 i4HueMode = 1;
    MINT32 i4SaturationMode = 1;
    MINT32 i4EdgeMode = 1;
    MUINT8 u1ToneMapMode = 1;


    //HAL metadata
    QUERY_ENTRY_SINGLE(control.halMeta, MTK_P1NODE_PROCESSOR_MAGICNUM, i4Id);
    rNewP2Param.i4MagicNum = i4Id;

    if (QUERY_ENTRY_SINGLE(control.halMeta, MTK_HAL_REQUEST_BRIGHTNESS_MODE, rNewP2Param.i4BrightnessMode))
    {
        rReprocInput.rCamInfo.rIspUsrSelectLevel.eIdx_Bright = (EIndex_Isp_Brightness_T)rNewP2Param.i4BrightnessMode;
    }
    if (QUERY_ENTRY_SINGLE(control.halMeta, MTK_HAL_REQUEST_CONTRAST_MODE, rNewP2Param.i4ContrastMode))
    {
        rReprocInput.rCamInfo.rIspUsrSelectLevel.eIdx_Contrast = (EIndex_Isp_Contrast_T)rNewP2Param.i4ContrastMode;
    }
    if (QUERY_ENTRY_SINGLE(control.halMeta, MTK_HAL_REQUEST_HUE_MODE, rNewP2Param.i4HueMode))
    {
        rReprocInput.rCamInfo.rIspUsrSelectLevel.eIdx_Hue = (EIndex_Isp_Hue_T)rNewP2Param.i4HueMode;
    }
    if (QUERY_ENTRY_SINGLE(control.halMeta, MTK_HAL_REQUEST_SATURATION_MODE, rNewP2Param.i4SaturationMode))
    {
        rReprocInput.rCamInfo.rIspUsrSelectLevel.eIdx_Sat = (EIndex_Isp_Saturation_T)rNewP2Param.i4SaturationMode;
    }
    if (QUERY_ENTRY_SINGLE(control.halMeta, MTK_HAL_REQUEST_EDGE_MODE, rNewP2Param.i4EdgeMode))
    {
        rReprocInput.rCamInfo.eEdgeMode = (mtk_camera_metadata_enum_android_edge_mode_t)rNewP2Param.i4EdgeMode;
    }
    MY_LOGD_IF((mu4LogEn & HAL3AADAPTER3_LOG_PF),"[%s] FrameId(%d), HueMode(%d),BrightnessMode(%d), ContrastMode(%d), SaturationMode(%d), i4EdgeMode(%d)"
        , __FUNCTION__, i4Id, rNewP2Param.i4HueMode, rNewP2Param.i4BrightnessMode, rNewP2Param.i4ContrastMode, rNewP2Param.i4SaturationMode, rNewP2Param.i4EdgeMode);

    //tonemap
    if (QUERY_ENTRY_SINGLE(control.appMeta, MTK_TONEMAP_MODE, u1ToneMapMode))
    {
        rReprocInput.rCamInfo.eToneMapMode = (mtk_camera_metadata_enum_android_tonemap_mode_t)u1ToneMapMode;
        rNewP2Param.u1TonemapMode = u1ToneMapMode;
    }

    {
        IMetadata::IEntry entryRed = control.appMeta.entryFor(MTK_TONEMAP_CURVE_RED);
        if (entryRed.tag() != IMetadata::IEntry::BAD_TAG)
        {
            rNewP2Param.u4TonemapCurveRedSize = entryRed.count();
            rNewP2Param.pTonemapCurveRed = (const MFLOAT*)entryRed.data();
            //rNewP2Param.pTonemapCurveRed = pTonemapRed;
        }

         IMetadata::IEntry entryGreen = control.appMeta.entryFor(MTK_TONEMAP_CURVE_GREEN);
        if (entryGreen.tag() != IMetadata::IEntry::BAD_TAG)
        {
            rNewP2Param.u4TonemapCurveGreenSize = entryGreen.count();
            rNewP2Param.pTonemapCurveGreen = (const MFLOAT*)entryGreen.data();
            //rNewP2Param.pTonemapCurveGreen = pTonemapGreen;
        }

        IMetadata::IEntry entryBlue = control.appMeta.entryFor(MTK_TONEMAP_CURVE_BLUE);
        if (entryBlue.tag() != IMetadata::IEntry::BAD_TAG)
        {
            rNewP2Param.u4TonemapCurveBlueSize = entryBlue.count();
            rNewP2Param.pTonemapCurveBlue = (const MFLOAT*)entryBlue.data();
            //rNewP2Param.pTonemapCurveBlue = pTonemapBlue;
        }
    }

#if HAL3A_TEST_OVERRIDE
    GET_PROP("vendor.debug.hal3av3.testp2", 0, u4ManualMode);
    _test_p2(u4ManualMode ,mParams, rNewP2Param);
#endif

    MY_LOGD_IF((mu4LogEn & HAL3AADAPTER3_LOG_PF), "[%s] MTK_TONEMAP_MODE(%d), MagicNum(%d) Rsize(%d) Gsize(%d) Bsize(%d)",
    __FUNCTION__, rNewP2Param.u1TonemapMode, i4Id,
    rNewP2Param.u4TonemapCurveRedSize, rNewP2Param.u4TonemapCurveGreenSize, rNewP2Param.u4TonemapCurveBlueSize);

    mpHal3aObj->setP2Params(rNewP2Param, pResultP2);
    MY_LOGD_IF(fgLogEn1, "[%s](%d)-", __FUNCTION__, i4Id);
    return MTRUE;
}

MVOID
Hal3AAdapter3::
clearLCSOList_Out()
{
    Mutex::Autolock lock(m_LCSO_Lock_Out);
    CAM_LOGD("[%s] Size(%d)", __FUNCTION__, m_rLCS_OUT_info_Queue.size());
    m_rLCS_OUT_info_Queue.clear();
}

MINT32
Hal3AAdapter3::
updateLCSList_Out(const ISP_LCS_OUT_INFO_T& rLCSinfo)
{
    Mutex::Autolock lock(m_LCSO_Lock_Out);

    MINT32 i4Ret = -1;
    MINT32 i4Pos = 0;
    MINT32 i4Size = m_rLCS_OUT_info_Queue.size();
    LCS_OUT_info_List_T::iterator it = m_rLCS_OUT_info_Queue.begin();
#if 0
    for (; it != m_rResultQueue.end(); it++)
    {
        CAM_LOGD("[%s] #(%d)", __FUNCTION__, it->i4FrmId);
    }
#endif
    for (it = m_rLCS_OUT_info_Queue.begin(); it != m_rLCS_OUT_info_Queue.end(); it++, i4Pos++)
    {
        if (it->i4FrmId == rLCSinfo.i4FrmId)
        {
            CAM_LOGW("overwirte LCSList_Out FrmID: %d", rLCSinfo.i4FrmId);
            *it = rLCSinfo;
            i4Ret = 1;
            break;
        }
    }

    if (i4Pos == i4Size)
    {
        m_rLCS_OUT_info_Queue.push_back(rLCSinfo);
        i4Ret = 0;
    }

    // remove item
    if (m_rLCS_OUT_info_Queue.size() > m_u4LCSOCapacity)
    {
        m_rLCS_OUT_info_Queue.erase(m_rLCS_OUT_info_Queue.begin());
    }

    return i4Ret;
}

MINT32
Hal3AAdapter3::
getLCSList_info_Out(MINT32 i4FrmId, ISP_LCS_OUT_INFO_T& rLCSinfo)
{
    Mutex::Autolock lock(m_LCSO_Lock_Out);

    MINT32 i4Ret = 0;
    MINT32 i4Pos = 0;
    MINT32 i4Size = m_rLCS_OUT_info_Queue.size();
    LCS_OUT_info_List_T::iterator it = m_rLCS_OUT_info_Queue.begin();
    for (; it != m_rLCS_OUT_info_Queue.end(); it++, i4Pos++)
    {
        if (it->i4FrmId == i4FrmId)
        {
            rLCSinfo = *it;
            //CAM_LOGD("[%s] OK i4Pos(%d)", __FUNCTION__, i4Pos);
            break;
        }
    }

    if (i4Pos == i4Size)
    {
        // does not exist
        CAM_LOGD("[%s] NG i4Pos(%d)", __FUNCTION__, i4Pos);
        i4Ret = -1;
    }

    return i4Ret;
}

MINT32
Hal3AAdapter3::
getLCSListLast_Out(ISP_LCS_OUT_INFO_T& rLCSinfo)
{
    Mutex::Autolock lock(m_LCSO_Lock_Out);

    if (!m_rLCS_OUT_info_Queue.empty())
    {
        LCS_OUT_info_List_T::iterator it = m_rLCS_OUT_info_Queue.end();
        it--;
        rLCSinfo = *it;
        return 0;
    }

    return -1;
}

ISP_LCS_OUT_INFO_T*
Hal3AAdapter3::getLCSList_info_Out(MINT32 i4FrmId)
{
    Mutex::Autolock lock(m_LCSO_Lock_Out);

    ISP_LCS_OUT_INFO_T* pBuf = NULL;
    MINT32 i4Pos = 0;
    MINT32 i4Size = m_rLCS_OUT_info_Queue.size();
    LCS_OUT_info_List_T::iterator it = m_rLCS_OUT_info_Queue.begin();
    for (; it != m_rLCS_OUT_info_Queue.end(); it++, i4Pos++)
    {
        if (it->i4FrmId == i4FrmId)
        {
            pBuf = &(*it);
            //CAM_LOGD("[%s] OK i4Pos(%d)", __FUNCTION__, i4Pos);
            break;
        }
    }

    if (i4Pos == i4Size)
    {
        // does not exist
        CAM_LOGD("[%s] Ref NG i4Pos(%d)", __FUNCTION__, i4Pos);
    }

    return pBuf;
}

MINT32
Hal3AAdapter3::
dumpIsp(MINT32 /*flowType*/, const MetaSet_T& /*control*/, TuningParam* /*pTuningBuf*/, MetaSet_T* /*pResult*/)
{
    return 0;
}

MBOOL
Hal3AAdapter3::
_readDump(TuningParam* /*pTuningBuf*/, const MetaSet_T& /*control*/, MetaSet_T* /*pResult*/, ISP_INFO_T* /*pIspInfo*/, MINT32 /*i4Format*/)
{
    return 0;
}

MINT32
Hal3AAdapter3::
setIsp(MINT32 flowType, const MetaSet_T& control, TuningParam* pTuningBuf, MetaSet_T* pResult)
{
    MINT32 i4P2En = 0;
    GET_PROP("vendor.debug.hal3av3.p2", 1, i4P2En);

if (i4P2En == 0 || pTuningBuf == NULL)
{
    MY_LOGD_IF((mu4LogEn & HAL3AADAPTER3_LOG_PF), "[%s] IT: flowType(%d), pTuningBuf(%p)", __FUNCTION__, flowType, pTuningBuf);
    return -1;
}
else
{
    MBOOL fgLog = mu4LogEn & HAL3AADAPTER3_LOG_PF;
    MBOOL fgGenPrefix = mu4LogEn & HAL3AADAPTER3_LOG_PREFIXP2;

    MINT32 i4Ret = -1;
    MINT32 u4Id = 0;
    ResultP2_T rResultP2;
    P2Param_T rNewP2Param;

    MUINT8 u1Exif = MFALSE;
    MUINT8 u1DumpExif = MFALSE;
    MUINT8 u1RepeatResult = MFALSE;
    NSIspTuning::ISP_INFO_T rIspInfo;
    QUERY_ENTRY_SINGLE(control.halMeta, MTK_HAL_REQUEST_REQUIRE_EXIF, u1Exif);
    QUERY_ENTRY_SINGLE(control.halMeta, MTK_HAL_REQUEST_DUMP_EXIF, u1DumpExif);
    QUERY_ENTRY_SINGLE(control.halMeta, MTK_P1NODE_PROCESSOR_MAGICNUM, u4Id);
    QUERY_ENTRY_SINGLE(control.halMeta, MTK_STEREO_FEATURE_DENOISE_MODE, rNewP2Param.i4DenoiseMode);

    IMetadata::IEntry repeatEntry = control.halMeta.entryFor(MTK_3A_REPEAT_RESULT);
    if (!repeatEntry.isEmpty())
        QUERY_ENTRY_SINGLE(control.halMeta, MTK_3A_REPEAT_RESULT, u1RepeatResult);
    MY_LOGD_IF((mu4LogEn & HAL3AADAPTER3_LOG_PF), "[%s] u1RepeatResult(%d):(%d)", __FUNCTION__, u1RepeatResult, u4Id);

#if 1
    if (u1RepeatResult)
    {
        ::memcpy(pTuningBuf->pRegBuf, mpTuning, queryTuningSize());
        if (pResult)
        {
            Mutex::Autolock lock(mLockIsp);
            *pResult = mMetaSetISP;
        }
        return 0;
    }
#endif

    const IMetadata::Memory* pCaminfoBuf = QUERY_ENTRY_REF_SINGLE<IMetadata::Memory>(control.halMeta, MTK_PROCESSOR_CAMINFO);

    MINT32 i4IspProfile = -1;
    MUINT8 u1IspProfile = NSIspTuning::EIspProfile_Preview;
    if (QUERY_ENTRY_SINGLE(control.halMeta, MTK_3A_ISP_PROFILE, u1IspProfile)){
        i4IspProfile = u1IspProfile;
    }

    MBOOL bBypassLCE = MFALSE;
    QUERY_ENTRY_SINGLE(control.halMeta, MTK_3A_ISP_BYPASS_LCE, bBypassLCE);

    MINT32 NR3D_Data[8];
    if(!GET_ENTRY_ARRAY(control.halMeta, MTK_3A_ISP_NR3D_SW_PARAMS, NR3D_Data, 8))
    {
        MY_LOG_IF(fgLog, "[%s]+ NR3D params get fail",__FUNCTION__);
    }

    if (pCaminfoBuf)
    {
                // Generate file name prefix
        rIspInfo.strFileNamePrefix = "";
        if(fgGenPrefix)
        {
            MINT32 i4Size = 512;
            char FilenamePrefix[512] = {'\0'};
            getDumpFilenamePrefix(FilenamePrefix, i4Size, &control.appMeta, &control.halMeta);
            rIspInfo.strFileNamePrefix = FilenamePrefix;
        }

        ::memcpy(&rIspInfo.rCamInfo, pCaminfoBuf->array(), sizeof(NSIspTuning::RAWIspCamInfo));
        rIspInfo.rCamInfo.bBypassLCE = bBypassLCE;
        rIspInfo.rCamInfo.NR3D_Data.GMVX    = NR3D_Data[0];
        rIspInfo.rCamInfo.NR3D_Data.GMVY    = NR3D_Data[1];
        rIspInfo.rCamInfo.NR3D_Data.confX   = NR3D_Data[2];
        rIspInfo.rCamInfo.NR3D_Data.confY   = NR3D_Data[3];
        rIspInfo.rCamInfo.NR3D_Data.MAX_GMV = NR3D_Data[4];
        rIspInfo.rCamInfo.NR3D_Data.frameReset = NR3D_Data[5];
        rIspInfo.rCamInfo.NR3D_Data.GMV_Status =NR3D_Data[6];
        rIspInfo.rCamInfo.NR3D_Data.ISO_cutoff =NR3D_Data[7];
        rIspInfo.rCamInfo.NR3D_Data.isGyroValid =NR3D_Data[8];
        rIspInfo.rCamInfo.NR3D_Data.gyroXAccelX1000 =NR3D_Data[9];
        rIspInfo.rCamInfo.NR3D_Data.gyroYAccelX1000 =NR3D_Data[10];
        rIspInfo.rCamInfo.NR3D_Data.gyroZAccelX1000 =NR3D_Data[11];
        rIspInfo.rCamInfo.NR3D_Data.gyroTimeStampHigh =NR3D_Data[12];
        rIspInfo.rCamInfo.NR3D_Data.gyroTimeStampLow =NR3D_Data[13];
        //LCE callback (get LCSO parameter)
        if(getLCSList_info_Out(u4Id, rIspInfo.rCamInfo.rLCS_Info.rOutSetting) == (-1) ){
            CAM_LOGE("[-Fail to get LCS Out tuning ] FrmId(%d)", u4Id);

            if(getLCSListLast_Out(rIspInfo.rCamInfo.rLCS_Info.rOutSetting) == (-1)){
                 CAM_LOGE("[-Fail to get LCS Out Queue ]");
                 rIspInfo.rCamInfo.bBypassLCE = MTRUE;
            }
        }

        setP2Params(rNewP2Param, control, &rResultP2, rIspInfo); // parse p2 params & set to ISP

#if CAM3_STEREO_FEATURE_EN
        MUINT32 u4AEISOSpeed = 0;
        if(rNewP2Param.i4DenoiseMode > 0)
        {
            // update ISO , IspGain to ISP Caminfo
            mpHal3aObj->send3ACtrl(E3ACtrl_GetAEStereoDenoiseInfo, (MINTPTR)&rIspInfo, (MINTPTR)&u4AEISOSpeed);
        }
#endif
        AAA_TRACE_D("setIsp#(%d)", rIspInfo.rCamInfo.u4Id);
        _reprocess(control, i4IspProfile, rIspInfo);

        MY_LOGD_IF(fgLog, "[%s]+ sensorDev(%d), key(%09d), #(%d), flow(%d), ispProfile(%d), rpg(%d), pTuningBuf(%p)",
            __FUNCTION__, mu4SensorDev, rIspInfo.i4UniqueKey, rIspInfo.rCamInfo.u4Id, flowType, rIspInfo.rCamInfo.eIspProfile, rIspInfo.rCamInfo.fgRPGEnable, pTuningBuf);

        if (u1Exif)
        {
            IMetadata exifMeta;

            if(rIspInfo.rCamInfo.eIspProfile != EIspProfile_Capture_MultiPass_HWNR)
            {
                // after got p1 debug info
                if (QUERY_ENTRY_SINGLE(control.halMeta, MTK_3A_EXIF_METADATA, exifMeta))
                {
                    IMetadata::Memory dbgIsp;
                    QUERY_ENTRY_SINGLE(exifMeta, MTK_3A_EXIF_DBGINFO_ISP_DATA, dbgIsp);
                    rResultP2.vecDbgIspP2.appendVector(dbgIsp);
                }
            }
            // generate P2 tuning and get result including P2 debug info
            mpHal3aObj->setIspPass2(flowType, rIspInfo, pTuningBuf, &rResultP2);

            if (pResult)
            {
                // after got p1 debug info, append p2 debug info, and then put to result
                if (rResultP2.vecDbgIspP2.size())
                {
                    UPDATE_ENTRY_SINGLE<MINT32>(exifMeta, MTK_3A_EXIF_DBGINFO_ISP_KEY, ISP_DEBUG_KEYID);
                    IMetadata::Memory dbgIsp;
                    dbgIsp.appendVector(rResultP2.vecDbgIspP2);
                    UPDATE_ENTRY_SINGLE(exifMeta, MTK_3A_EXIF_DBGINFO_ISP_DATA, dbgIsp);
                }

                // multi-pass NR debug info
                if (rResultP2.vecDbgIspP2_MultiP.size())
                {
                    QUERY_ENTRY_SINGLE(pResult->halMeta, MTK_3A_EXIF_METADATA, exifMeta);
                    UPDATE_ENTRY_SINGLE<MINT32>(exifMeta, MTK_POSTNR_EXIF_DBGINFO_NR_KEY, DEBUG_EXIF_MID_CAM_RESERVE1);
                    IMetadata::Memory dbgIsp;
                    dbgIsp.appendVector(rResultP2.vecDbgIspP2_MultiP);
                    UPDATE_ENTRY_SINGLE(exifMeta, MTK_POSTNR_EXIF_DBGINFO_NR_DATA, dbgIsp);
                }

#if CAM3_STEREO_FEATURE_EN
                if(rNewP2Param.i4DenoiseMode > 0)
                {
                    // update ISO to EXIF
                    if(u4AEISOSpeed != 0)
                        UPDATE_ENTRY_SINGLE<MINT32>(exifMeta, MTK_3A_EXIF_AE_ISO_SPEED, u4AEISOSpeed);
                }
#endif

                UPDATE_ENTRY_SINGLE(pResult->halMeta, MTK_3A_EXIF_METADATA, exifMeta);

                MINT32 i4FrmNo=0, i4ReqNo=0;
                QUERY_ENTRY_SINGLE(control.halMeta, MTK_PIPELINE_FRAME_NUMBER, i4FrmNo);
                QUERY_ENTRY_SINGLE(control.halMeta, MTK_PIPELINE_REQUEST_NUMBER, i4ReqNo);

                MUINT32 u4DebugInfo = 0;
                GET_PROP("vendor.debug.camera.dump.p2.debuginfo", 0, u4DebugInfo);
                if (u4DebugInfo != 0 || u1DumpExif)
                {
                    char strFile[512] = {'\0'};
                    sprintf(strFile, "/sdcard/debug/debuginfo-%09d-%04d-%04d-%d-%04d-%d.bin", rIspInfo.i4UniqueKey, i4FrmNo, i4ReqNo, mu4SensorDev, rIspInfo.rCamInfo.u4Id, rIspInfo.rCamInfo.eIspProfile);
                    _dumpDebugInfo(strFile, exifMeta);
                }
            }
        }
        else
        {
            // generate P2 tuning only
            mpHal3aObj->setIspPass2(flowType, rIspInfo, pTuningBuf, NULL);
        }
        _reprocResult(rIspInfo, pResult);

        AAA_TRACE_END_D;
        MY_LOGD_IF(fgLog, "[%s]- OK(%p, %p)", __FUNCTION__, pTuningBuf->pRegBuf, pTuningBuf->pLsc2Buf);
        i4Ret = 0;
    }
    else if(u1IspProfile == EIspProfile_YUV_Reprocess){
        rIspInfo.rCamInfo.bBypassLCE = MTRUE;
        _reprocess(control, i4IspProfile, rIspInfo);
        mpHal3aObj->setIspPass2(flowType, rIspInfo, pTuningBuf, NULL);
        _reprocResult(rIspInfo, pResult);

        if (pResult)
        {
            IMetadata rStdExif = getReprocStdExif(control);
            UPDATE_ENTRY_SINGLE(pResult->halMeta, MTK_3A_EXIF_METADATA, rStdExif);
        }
        i4Ret = 0;
    }
    else
    {
        CAM_LOGE("[%s] NG (no caminfo)", __FUNCTION__);
        i4Ret = -1;
    }

    {
        MY_LOGD_IF(fgLog, "mpTuning(%x) pTuningBuf(%x)", mpTuning, pTuningBuf);
        ::memcpy(mpTuning, pTuningBuf->pRegBuf, queryTuningSize());
        if (pResult != NULL)
        {
            Mutex::Autolock lock(mLockIsp);
            mMetaSetISP = *pResult;
        }
    }
    mpHal3aObj->getP2Result(rNewP2Param, &rResultP2);
    convertP2ResultToMeta(rResultP2, pResult);
    return i4Ret;
}
}

MINT32
Hal3AAdapter3::
attachCb(IHal3ACb::ECb_T eId, IHal3ACb* pCb)
{
    MY_LOGD_IF((mu4LogEn & HAL3AADAPTER3_LOG_PF), "[%s] eId(%d), pCb(%p)", __FUNCTION__, eId, pCb);
    return m_CbSet[eId].addCallback(pCb);
}

MINT32
Hal3AAdapter3::
detachCb(IHal3ACb::ECb_T eId, IHal3ACb* pCb)
{
    MY_LOGD_IF((mu4LogEn & HAL3AADAPTER3_LOG_PF), "[%s] eId(%d), pCb(%p)", __FUNCTION__, eId, pCb);
    return m_CbSet[eId].removeCallback(pCb);
}

MINT32
Hal3AAdapter3::
getDelay(IMetadata& delay_info) const
{
    return 0;
}

MINT32
Hal3AAdapter3::
getDelay(MUINT32 tag) const
{
    // must be >= HAL3A_MIN_PIPE_LATENCY
    MINT32 i4Delay = 2;
    #if 0
    // temp
    switch (tag)
    {
    case MTK_REQUEST_FRAME_COUNT:
        i4Delay = 1;
        break;
    case MTK_CONTROL_AE_REGIONS:
        i4Delay = 1;
        break;
    case MTK_CONTROL_AF_MODE:
        i4Delay = 1;
        break;
    case MTK_CONTROL_AF_REGIONS:
        i4Delay = 3;
        break;
    //case MTK_CONTROL_AF_APERTURE:
    //    i4Delay = 1;
    //    break;
    //case MTK_CONTROL_AF_FOCALLENGTH:
    //    i4Delay = 1;
    //    break;
    //case MTK_CONTROL_AF_FOCUSDISTANCE:
    //    i4Delay = 1;
    //    break;
    //case MTK_CONTROL_AF_OIS:
    //    i4Delay = 1;
    //    break;
    //case MTK_CONTROL_AF_SHARPNESSMAPMODE:
    //    i4Delay = 1;
    //    break;

    case MTK_CONTROL_AWB_REGIONS:
        i4Delay = 1;
        break;
    case MTK_CONTROL_AWB_MODE:
        i4Delay = 3;
        break;
    }
    #endif
    return i4Delay;
}

MINT32
Hal3AAdapter3::
getCapacity() const
{
    return HAL3A_REQ_CAPACITY;
}

MVOID
Hal3AAdapter3::
setSensorMode(MINT32 i4SensorMode)
{
    mpHal3aObj->setSensorMode(i4SensorMode);
    mi4SensorMode = i4SensorMode;
    MY_LOGD_IF((mu4LogEn & HAL3AADAPTER3_LOG_PF), "[%s] mi4SensorMode = %d", __FUNCTION__, mi4SensorMode);
}

MVOID
Hal3AAdapter3::
notifyP1Done(MUINT32 u4MagicNum, MVOID* pvArg)
{
    //MY_LOGD("[%s] u4MagicNum(%d)", __FUNCTION__, u4MagicNum);
    mpHal3aObj->notifyP1Done(u4MagicNum, pvArg);
}

MBOOL
Hal3AAdapter3::
notifyPwrOn()
{
    return mpHal3aObj->notifyPwrOn();
}


MBOOL
Hal3AAdapter3::
notifyPwrOff()
{
    Mutex::Autolock lock(mLock);
    size_t iUsers = m_Users.size();
    for (size_t i = 0; i < iUsers; i++)
    {
        std::string strName = m_Users.keyAt(i);
        MUINT8 value = m_Users.valueAt(i);
        MY_LOGD_IF(value > 0, "[%s] sensor(%d)(%s, %d)", __FUNCTION__, mu4SensorDev, strName.c_str(), value);
    }
    return mpHal3aObj->notifyPwrOff();
}

MBOOL
Hal3AAdapter3::
notifyP1PwrOn()
{
    CAM_LOGD("[%s] notifyP1PwrOn", __FUNCTION__);
    return MFALSE;
}

MBOOL
Hal3AAdapter3::
notifyP1PwrOff()
{
    CAM_LOGD("[%s] notifyP1PwrOff", __FUNCTION__);
    return MFALSE;
}

MBOOL
Hal3AAdapter3::
checkCapFlash()
{
    return mpHal3aObj->checkCapFlash();
}

MVOID
Hal3AAdapter3::
setFDEnable(MBOOL fgEnable)
{
    Mutex::Autolock lock(mLock);
    mpHal3aObj->setFDEnable(fgEnable);
}

MBOOL
Hal3AAdapter3::
setFDInfo(MVOID* prFaces)
{
    Mutex::Autolock lock(mLock);
    MBOOL fdLogEn0 = (mu4LogEn & HAL3AADAPTER3_LOG_SET_0) ? MTRUE : MFALSE;
    MtkCameraFaceMetadata* rFaceMeta = (MtkCameraFaceMetadata *)prFaces;
    mAFFaceMeta.number_of_faces = 0;
    if(mCropRegion.s.w != 0 && mCropRegion.s.h != 0 && rFaceMeta->number_of_faces != 0) {
        // calculate face TG size
        int i = 0;
        int img_w = rFaceMeta->ImgWidth;
        int img_h = rFaceMeta->ImgHeight;
        MRect ImgCrop;
        MINT32 i4TgWidth = 0;
        MINT32 i4TgHeight = 0;
        CameraArea_T rArea;
        mpHal3aObj->queryTgSize(i4TgWidth,i4TgHeight);
        if((mCropRegion.s.w * img_h) > (mCropRegion.s.h * img_w)) { // pillarbox
            ImgCrop.s.w = mCropRegion.s.h * img_w / img_h;
            ImgCrop.s.h = mCropRegion.s.h;
            ImgCrop.p.x = mCropRegion.p.x + ((mCropRegion.s.w - ImgCrop.s.w) >> 1);
            ImgCrop.p.y = mCropRegion.p.y;
        } else if((mCropRegion.s.w * img_h) < (mCropRegion.s.h * img_w)) { // letterbox
            ImgCrop.s.w = mCropRegion.s.w;
            ImgCrop.s.h = mCropRegion.s.w * img_h / img_w;
            ImgCrop.p.x = mCropRegion.p.x;
            ImgCrop.p.y = mCropRegion.p.y + ((mCropRegion.s.h - ImgCrop.s.h) >> 1);

        } else {
            ImgCrop.p.x = mCropRegion.p.x;
            ImgCrop.p.y = mCropRegion.p.y;
            ImgCrop.s.w = mCropRegion.s.w;
            ImgCrop.s.h = mCropRegion.s.h;
        }
        mAFFaceMeta.number_of_faces = rFaceMeta->number_of_faces;
        for(i = 0; i < rFaceMeta->number_of_faces; i++) {
            // face
            rArea.i4Left = ((rFaceMeta->faces[i].rect[0]+1000) * ImgCrop.s.w/2000) + ImgCrop.p.x;  //Left
            rArea.i4Top = ((rFaceMeta->faces[i].rect[1]+1000) * ImgCrop.s.h/2000) + ImgCrop.p.y;  //Top
            rArea.i4Right = ((rFaceMeta->faces[i].rect[2]+1000) * ImgCrop.s.w/2000) + ImgCrop.p.x;  //Right
            rArea.i4Bottom = ((rFaceMeta->faces[i].rect[3]+1000) * ImgCrop.s.h/2000) + ImgCrop.p.y;  //Bottom
            rArea.i4Weight = 0;
            rArea = _transformArea(mi4SensorIdx,mi4SensorMode,rArea);
            rArea = _clipArea(i4TgWidth,i4TgHeight,rArea);
            mAFFaceMeta.faces[i].rect[0] = rArea.i4Left;
            mAFFaceMeta.faces[i].rect[1] = rArea.i4Top;
            mAFFaceMeta.faces[i].rect[2] = rArea.i4Right;
            mAFFaceMeta.faces[i].rect[3] = rArea.i4Bottom;
            if(rFaceMeta->fa_cv[i] > 0) // 0 is invalid value
            {
                // left eye
                rArea.i4Left = ((rFaceMeta->leyex0[i]+1000) * ImgCrop.s.w/2000) + ImgCrop.p.x;  //Left
                rArea.i4Top = ((rFaceMeta->leyey0[i]+1000) * ImgCrop.s.h/2000) + ImgCrop.p.y;  //Top
                rArea.i4Right = ((rFaceMeta->leyex1[i]+1000) * ImgCrop.s.w/2000) + ImgCrop.p.x;  //Right
                rArea.i4Bottom = ((rFaceMeta->leyey1[i]+1000) * ImgCrop.s.h/2000) + ImgCrop.p.y;  //Bottom
                rArea = _transformArea(mi4SensorIdx,mi4SensorMode,rArea);
                rArea = _clipArea(i4TgWidth,i4TgHeight,rArea);
                mAFFaceMeta.leyex0[i] = rArea.i4Left;
                mAFFaceMeta.leyey0[i] = rArea.i4Top;
                mAFFaceMeta.leyex1[i] = rArea.i4Right;
                mAFFaceMeta.leyey1[i] = rArea.i4Bottom;
                // right eye
                rArea.i4Left = ((rFaceMeta->reyex0[i]+1000) * ImgCrop.s.w/2000) + ImgCrop.p.x;  //Left
                rArea.i4Top = ((rFaceMeta->reyey0[i]+1000) * ImgCrop.s.h/2000) + ImgCrop.p.y;  //Top
                rArea.i4Right = ((rFaceMeta->reyex1[i]+1000) * ImgCrop.s.w/2000) + ImgCrop.p.x;  //Right
                rArea.i4Bottom = ((rFaceMeta->reyey1[i]+1000) * ImgCrop.s.h/2000) + ImgCrop.p.y;  //Bottom
                rArea = _transformArea(mi4SensorIdx,mi4SensorMode,rArea);
                rArea = _clipArea(i4TgWidth,i4TgHeight,rArea);
                mAFFaceMeta.reyex0[i] = rArea.i4Left;
                mAFFaceMeta.reyey0[i] = rArea.i4Top;
                mAFFaceMeta.reyex1[i] = rArea.i4Right;
                mAFFaceMeta.reyey1[i] = rArea.i4Bottom;
                // mouth
                rArea.i4Left = ((rFaceMeta->mouthx0[i]+1000) * ImgCrop.s.w/2000) + ImgCrop.p.x;  //Left
                rArea.i4Top = ((rFaceMeta->mouthy0[i]+1000) * ImgCrop.s.h/2000) + ImgCrop.p.y;  //Top
                rArea.i4Right = ((rFaceMeta->mouthx1[i]+1000) * ImgCrop.s.w/2000) + ImgCrop.p.x;  //Right
                rArea.i4Bottom = ((rFaceMeta->mouthy1[i]+1000) * ImgCrop.s.h/2000) + ImgCrop.p.y;  //Bottom
                rArea = _transformArea(mi4SensorIdx,mi4SensorMode,rArea);
                rArea = _clipArea(i4TgWidth,i4TgHeight,rArea);
                mAFFaceMeta.mouthx0[i] = rArea.i4Left;
                mAFFaceMeta.mouthy0[i] = rArea.i4Top;
                mAFFaceMeta.mouthx1[i] = rArea.i4Right;
                mAFFaceMeta.mouthy1[i] = rArea.i4Bottom;
            }
            // other data
            mAFFaceMeta.faces[i].score   = rFaceMeta->faces[i].score;
            mAFFaceMeta.faces[i].id      = rFaceMeta->faces[i].id;
            mAFFaceMeta.faces_type[i]    = rFaceMeta->faces_type[i];
            mAFFaceMeta.motion[i][0]     = rFaceMeta->motion[i][0];
            mAFFaceMeta.motion[i][1]     = rFaceMeta->motion[i][1];
            mAFFaceMeta.fa_cv[i]         = rFaceMeta->fa_cv[i];
            mAFFaceMeta.posInfo[i].rip_dir = rFaceMeta->posInfo[i].rip_dir;
            mAFFaceMeta.posInfo[i].rop_dir = rFaceMeta->posInfo[i].rop_dir;
            MY_LOGD_IF(fdLogEn0, "[%s] face[%d] rect info(%d,%d,%d,%d)", __FUNCTION__, i, rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom);
        }
    }
    return mpHal3aObj->setFDInfo(&mAFFaceMeta, &mAFFaceMeta);
}

MBOOL
Hal3AAdapter3::
setFDInfoOnActiveArray(MVOID* prFaces)
{
    Mutex::Autolock lock(mLock);
    if (!prFaces)   return MFALSE;
    MBOOL fdLogEn0 = (mu4LogEn & HAL3AADAPTER3_LOG_SET_0) ? MTRUE : MFALSE;
    MtkCameraFaceMetadata* rFaceMeta = (MtkCameraFaceMetadata *)prFaces;
    mAFFaceMeta.number_of_faces = 0;
    mAFFaceMeta.timestamp = rFaceMeta->timestamp;
    mAFFaceMeta.CNNFaces = rFaceMeta->CNNFaces;
    if (rFaceMeta->number_of_faces != 0)
    {
        // calculate face TG size
        int i = 0;
        MINT32 i4TgWidth = 0;
        MINT32 i4TgHeight = 0;
        CameraArea_T rArea;
        mpHal3aObj->queryTgSize(i4TgWidth,i4TgHeight);

        mAFFaceMeta.number_of_faces = rFaceMeta->number_of_faces;
        for(i = 0; i < rFaceMeta->number_of_faces; i++) {
            // face
            rArea.i4Left   = rFaceMeta->faces[i].rect[0];  //Left
            rArea.i4Top    = rFaceMeta->faces[i].rect[1];  //Top
            rArea.i4Right  = rFaceMeta->faces[i].rect[2];  //Right
            rArea.i4Bottom = rFaceMeta->faces[i].rect[3];  //Bottom
            rArea.i4Weight = 0;
            rArea = _transformArea(mi4SensorIdx,mi4SensorMode,rArea);
            rArea = _clipArea(i4TgWidth,i4TgHeight,rArea);
            mAFFaceMeta.faces[i].rect[0] = rArea.i4Left;
            mAFFaceMeta.faces[i].rect[1] = rArea.i4Top;
            mAFFaceMeta.faces[i].rect[2] = rArea.i4Right;
            mAFFaceMeta.faces[i].rect[3] = rArea.i4Bottom;
            if(rFaceMeta->fa_cv[i] > 0) // 0 is invalid value
            {
                // left eye
                rArea.i4Left   = rFaceMeta->leyex0[i];  //Left
                rArea.i4Top    = rFaceMeta->leyey0[i];  //Top
                rArea.i4Right  = rFaceMeta->leyex1[i];  //Right
                rArea.i4Bottom = rFaceMeta->leyey1[i];  //Bottom
                rArea = _transformArea(mi4SensorIdx,mi4SensorMode,rArea);
                rArea = _clipArea(i4TgWidth,i4TgHeight,rArea);
                mAFFaceMeta.leyex0[i] = rArea.i4Left;
                mAFFaceMeta.leyey0[i] = rArea.i4Top;
                mAFFaceMeta.leyex1[i] = rArea.i4Right;
                mAFFaceMeta.leyey1[i] = rArea.i4Bottom;
                // right eye
                rArea.i4Left   = rFaceMeta->reyex0[i];  //Left
                rArea.i4Top    = rFaceMeta->reyey0[i];  //Top
                rArea.i4Right  = rFaceMeta->reyex1[i];  //Right
                rArea.i4Bottom = rFaceMeta->reyey1[i];  //Bottom
                rArea = _transformArea(mi4SensorIdx,mi4SensorMode,rArea);
                rArea = _clipArea(i4TgWidth,i4TgHeight,rArea);
                mAFFaceMeta.reyex0[i] = rArea.i4Left;
                mAFFaceMeta.reyey0[i] = rArea.i4Top;
                mAFFaceMeta.reyex1[i] = rArea.i4Right;
                mAFFaceMeta.reyey1[i] = rArea.i4Bottom;
                // mouth
                rArea.i4Left   = rFaceMeta->mouthx0[i];  //Left
                rArea.i4Top    = rFaceMeta->mouthy0[i];  //Top
                rArea.i4Right  = rFaceMeta->mouthx1[i];  //Right
                rArea.i4Bottom = rFaceMeta->mouthy1[i];  //Bottom
                rArea = _transformArea(mi4SensorIdx,mi4SensorMode,rArea);
                rArea = _clipArea(i4TgWidth,i4TgHeight,rArea);
                mAFFaceMeta.mouthx0[i] = rArea.i4Left;
                mAFFaceMeta.mouthy0[i] = rArea.i4Top;
                mAFFaceMeta.mouthx1[i] = rArea.i4Right;
                mAFFaceMeta.mouthy1[i] = rArea.i4Bottom;
            }
            // other data
            mAFFaceMeta.faces[i].score   = rFaceMeta->faces[i].score;
            mAFFaceMeta.faces[i].id      = rFaceMeta->faces[i].id;
            mAFFaceMeta.faces_type[i]    = rFaceMeta->faces_type[i];
            mAFFaceMeta.motion[i][0]     = rFaceMeta->motion[i][0];
            mAFFaceMeta.motion[i][1]     = rFaceMeta->motion[i][1];
            mAFFaceMeta.fa_cv[i]         = rFaceMeta->fa_cv[i];
            mAFFaceMeta.fld_rip[i]       = rFaceMeta->fld_rip[i];
            mAFFaceMeta.fld_rop[i]       = rFaceMeta->fld_rop[i];
            mAFFaceMeta.posInfo[i].rip_dir = rFaceMeta->posInfo[i].rip_dir;
            mAFFaceMeta.posInfo[i].rop_dir = rFaceMeta->posInfo[i].rop_dir;
            mAFFaceMeta.fld_GenderLabel[i] = rFaceMeta->fld_GenderLabel[i];
            mAFFaceMeta.fld_GenderInfo[i] = rFaceMeta->fld_GenderInfo[i];
            MY_LOGD_IF(fdLogEn0, "[%s] face[%d] rect info(%d,%d,%d,%d)", __FUNCTION__, i, rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom);
        }
    }
    memcpy(&(mAFFaceMeta.YUVsts), &(rFaceMeta->YUVsts), sizeof(mAFFaceMeta.YUVsts));
#if 0
    for(int i = 0; i < mAFFaceMeta.number_of_faces; i++) {
        for(int j = 0; j < 6 ; j++) {
            MY_LOGD_IF(fdLogEn0, "[%s] rFaceMeta->YUVsts[%d][%d] : %d", __FUNCTION__, i, j, rFaceMeta->YUVsts[i][j]);
            MY_LOGD_IF(fdLogEn0, "[%s] mAFFaceMeta.YUVsts[%d][%d] : %d", __FUNCTION__, i, j, mAFFaceMeta.YUVsts[i][j]);
        }
    }
#endif
    return mpHal3aObj->setFDInfo(&mAFFaceMeta, &mAFFaceMeta);
}

MBOOL
Hal3AAdapter3::
setOTInfo(MVOID* prOT)
{
    Mutex::Autolock lock(mLock);
    MBOOL fdLogEn0 = (mu4LogEn & HAL3AADAPTER3_LOG_SET_0) ? MTRUE : MFALSE;
    MtkCameraFaceMetadata* rOTMeta = (MtkCameraFaceMetadata *)prOT;
    mAFOTMeta.number_of_faces = 0;
    if(mCropRegion.s.w != 0 && mCropRegion.s.h != 0 && rOTMeta->number_of_faces != 0) {
        // calculate face TG size
        int i = 0;
        int img_w = rOTMeta->ImgWidth;
        int img_h = rOTMeta->ImgHeight;
        MRect ImgCrop;
        MINT32 i4TgWidth = 0;
        MINT32 i4TgHeight = 0;
        CameraArea_T rArea;
        mpHal3aObj->queryTgSize(i4TgWidth,i4TgHeight);
        if((mCropRegion.s.w * img_h) > (mCropRegion.s.h * img_w)) { // pillarbox
            ImgCrop.s.w = mCropRegion.s.h * img_w / img_h;
            ImgCrop.s.h = mCropRegion.s.h;
            ImgCrop.p.x = mCropRegion.p.x + ((mCropRegion.s.w - ImgCrop.s.w) >> 1);
            ImgCrop.p.y = mCropRegion.p.y;
        } else if((mCropRegion.s.w * img_h) < (mCropRegion.s.h * img_w)) { // letterbox
            ImgCrop.s.w = mCropRegion.s.w;
            ImgCrop.s.h = mCropRegion.s.w * img_h / img_w;
            ImgCrop.p.x = mCropRegion.p.x;
            ImgCrop.p.y = mCropRegion.p.y + ((mCropRegion.s.h - ImgCrop.s.h) >> 1);

        } else {
            ImgCrop.p.x = mCropRegion.p.x;
            ImgCrop.p.y = mCropRegion.p.y;
            ImgCrop.s.w = mCropRegion.s.w;
            ImgCrop.s.h = mCropRegion.s.h;
        }
        mAFOTMeta.number_of_faces = rOTMeta->number_of_faces;
        for(i = 0; i < rOTMeta->number_of_faces; i++) {
            rArea.i4Left = ((rOTMeta->faces[i].rect[0]+1000) * ImgCrop.s.w/2000) + ImgCrop.p.x;  //Left
            rArea.i4Top = ((rOTMeta->faces[i].rect[1]+1000) * ImgCrop.s.h/2000) + ImgCrop.p.y;  //Top
            rArea.i4Right = ((rOTMeta->faces[i].rect[2]+1000) * ImgCrop.s.w/2000) + ImgCrop.p.x;  //Right
            rArea.i4Bottom = ((rOTMeta->faces[i].rect[3]+1000) * ImgCrop.s.h/2000) + ImgCrop.p.y;  //Bottom
            rArea.i4Weight = 0;
            rArea = _transformArea(mi4SensorIdx,mi4SensorMode,rArea);
            rArea = _clipArea(i4TgWidth,i4TgHeight,rArea);
            mAFOTMeta.faces[i].rect[0] = rArea.i4Left;
            mAFOTMeta.faces[i].rect[1] = rArea.i4Top;
            mAFOTMeta.faces[i].rect[2] = rArea.i4Right;
            mAFOTMeta.faces[i].rect[3] = rArea.i4Bottom;
            mAFOTMeta.faces[i].score   = rOTMeta->faces[i].score;
            MY_LOGD_IF(fdLogEn0, "[%s] face[%d] rect info(%d,%d,%d,%d)", __FUNCTION__, i, rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom);
        }
    }
    return mpHal3aObj->setOTInfo(prOT, &mAFOTMeta);
}

MINT32
Hal3AAdapter3::
send3ACtrl(E3ACtrl_T e3ACtrl, MINTPTR i4Arg1, MINTPTR i4Arg2)
{
    if(e3ACtrl == E3ACtrl_SetLcsoParam){
        MINT32 err = NO_ERROR;
        updateLCSList_Out(*(ISP_LCS_OUT_INFO_T*)i4Arg1);
        return 0;
    }
    else
    return mpHal3aObj->send3ACtrl(e3ACtrl, i4Arg1, i4Arg2);
}

void
Hal3AAdapter3::
doNotifyCb(MINT32 _msgType, MINTPTR _ext1, MINTPTR _ext2, MINTPTR _ext3)
{
    switch (_msgType)
    {
    case eID_NOTIFY_3APROC_FINISH:
        MY_LOGD_IF((mu4LogEn & HAL3AADAPTER3_LOG_PF), "[%s] _msgType(%d), _ext1(%d), _ext2(%d), _ext3(%d)", __FUNCTION__, _msgType, _ext1, _ext2, _ext3);
        m_CbSet[IHal3ACb::eID_NOTIFY_3APROC_FINISH].doNotifyCb(IHal3ACb::eID_NOTIFY_3APROC_FINISH, _ext1, _ext2, _ext3);
        break;
    case eID_NOTIFY_READY2CAP:
        m_CbSet[IHal3ACb::eID_NOTIFY_READY2CAP].doNotifyCb(IHal3ACb::eID_NOTIFY_READY2CAP, _ext1, _ext2, _ext3);
        break;
    case eID_NOTIFY_CURR_RESULT:
        m_CbSet[IHal3ACb::eID_NOTIFY_CURR_RESULT].doNotifyCb(IHal3ACb::eID_NOTIFY_CURR_RESULT, _ext1, _ext2, _ext3);
        break;
    case eID_NOTIFY_AE_RT_PARAMS:
        m_CbSet[IHal3ACb::eID_NOTIFY_AE_RT_PARAMS].doNotifyCb(IHal3ACb::eID_NOTIFY_AE_RT_PARAMS, _ext1, _ext2, _ext3);
        break;
    case eID_NOTIFY_HDRD_RESULT:
        m_CbSet[IHal3ACb::eID_NOTIFY_HDRD_RESULT].doNotifyCb(IHal3ACb::eID_NOTIFY_HDRD_RESULT, _ext1, _ext2, _ext3);
        break;
    case eID_NOTIFY_VSYNC_DONE:
        m_CbSet[IHal3ACb::eID_NOTIFY_VSYNC_DONE].doNotifyCb(IHal3ACb::eID_NOTIFY_VSYNC_DONE, _ext1, _ext2, _ext3);
        break;
    case eID_NOTIFY_LCS_ISP_PARAMS:
        m_CbSet[IHal3ACb::eID_NOTIFY_LCS_ISP_PARAMS].doNotifyCb(IHal3ACb::eID_NOTIFY_LCS_ISP_PARAMS, _ext1, _ext2, _ext3);
        break;
    default:
        break;
    }
}

void
Hal3AAdapter3::
doDataCb(int32_t _msgType, void* _data, uint32_t _size)
{}

//====================
Hal3ACbSet::
Hal3ACbSet()
    : m_Mutex()
{
    Mutex::Autolock autoLock(m_Mutex);

    m_CallBacks.clear();
}

Hal3ACbSet::
~Hal3ACbSet()
{
    Mutex::Autolock autoLock(m_Mutex);

    m_CallBacks.clear();
}

void
Hal3ACbSet::
doNotifyCb (
    MINT32  _msgType,
    MINTPTR _ext1,
    MINTPTR _ext2,
    MINTPTR _ext3
)
{
    Mutex::Autolock autoLock(m_Mutex);

    //MY_LOGD("[Hal3ACbSet::%s] _msgType(%d), _ext1(%d), _ext2(%d), _ext3(%d)", __FUNCTION__, _msgType, _ext1, _ext2, _ext3);
    List<IHal3ACb*>::iterator it;
    for (it = m_CallBacks.begin(); it != m_CallBacks.end(); it++)
    {
        IHal3ACb* pCb = *it;
        //MY_LOGD("[%s] pCb(%p)", __FUNCTION__, pCb);
        if (pCb)
        {
            pCb->doNotifyCb(_msgType, _ext1, _ext2, _ext3);
        }
    }
}

MINT32
Hal3ACbSet::
addCallback(IHal3ACb* cb)
{
    Mutex::Autolock autoLock(m_Mutex);

    MINT32 i4Cnt = 0;

    if (cb == NULL)
    {
        CAM_LOGE("[%s] NULL callback!", __FUNCTION__);
        return -m_CallBacks.size();
    }

    List<IHal3ACb*>::iterator it;
    for (it = m_CallBacks.begin(); it != m_CallBacks.end(); it++, i4Cnt++)
    {
        if (cb == *it)
            break;
    }

    if (i4Cnt == (MINT32) m_CallBacks.size())
    {
        // not exist, add
        m_CallBacks.push_back(cb);
        return m_CallBacks.size();
    }
    else
    {
        // already exists
        return -m_CallBacks.size();
    }
}

MINT32
Hal3ACbSet::
removeCallback(IHal3ACb* cb)
{
    Mutex::Autolock autoLock(m_Mutex);

    MINT32 i4Cnt = 0;
    MINT32 i4Size = m_CallBacks.size();

    List<IHal3ACb*>::iterator it;
    for (it = m_CallBacks.begin(); it != m_CallBacks.end(); it++, i4Cnt++)
    {
        if (cb == *it)
        {
            m_CallBacks.erase(it);
            // successfully removed
            return m_CallBacks.size();
        }
    }

    // cannot be found
    return -i4Size;
}

