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


#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/common.h>
#include <sys/stat.h>
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

#include <debug_exif/dbg_id_param.h>

#include <mtkcam/utils/exif/IBaseCamExif.h>
#include <debug/DebugUtil.h>

//#include <ISync3A.h>
#include <SttBufQ.h>

#include "isp_tuning_mgr.h"
#include <mtkcam/utils/TuningUtils/FileDumpNamingRule.h>
#include <mtkcam/utils/sys/IFileCache.h>

#include <lsc/ILscTbl.h>

#include <fstream>
#include <string>
#include <cstring>
#include <cmath>
//tuning utils
#include <mtkcam/utils/TuningUtils/FileReadRule.h>
#include <private/aaa_hal_private.h>
#include <array>

using namespace std;
using namespace NS3Av3;
using namespace NSCam;
using namespace NSCamHW;
using namespace NSIspTuning;
using namespace NSCam::TuningUtils;

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

#define HAL3A_REQ_PROC_KEY (2)
#define HAL3A_REQ_CAPACITY (HAL3A_REQ_PROC_KEY + 2)

/********************************************************************************************
 * ENABLE_OVERRIDE_SCENE_MODES_CONTROL:
 * [ON]: override scene mode control.
 * [OFF]: non-override scene mode control
 *******************************************************************************************/
#define ENABLE_OVERRIDE_SCENE_MODES_CONTROL MTRUE

/*******************************************************************************
* utilities
********************************************************************************/
#define MY_INST NS3Av3::INST_T<Hal3AAdapter3>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

MUINT32 Hal3AAdapter3::mu4LogEn = 0;
MUINT32 Hal3AAdapter3::mu4DbgLogWEn = 0;

MUINT32 Hal3AAdapter3::mu4ManualMode = 0;
MUINT32 Hal3AAdapter3::mu4ManualModeP2 = 0;
MUINT32 Hal3AAdapter3::mi4ForceDebugDump = 0;
map<MINT32,NSCamHW::HwMatrix> Hal3AAdapter3::mMapMat;
std::mutex Hal3AAdapter3::mMapMatLock;

MINT32 AAA_TRACE_LEVEL = 0;

#define MY_LOGD(fmt, arg...) \
    do { \
        if (Hal3AAdapter3::mu4DbgLogWEn) { \
            CAM_ULOGW(Utils::ULog::MOD_3A_FRAMEWORK, fmt, ##arg); \
        } else { \
            CAM_ULOGD(Utils::ULog::MOD_3A_FRAMEWORK, fmt, ##arg); \
        } \
    }while(0)

#define MY_LOGD_IF(cond, ...) \
    do { \
        if (Hal3AAdapter3::mu4DbgLogWEn) { \
            if ( (cond) ){ CAM_ULOGW(Utils::ULog::MOD_3A_FRAMEWORK, __VA_ARGS__); } \
        } else { \
            if ( (cond) ){ CAM_ULOGD(Utils::ULog::MOD_3A_FRAMEWORK, __VA_ARGS__); } \
        } \
    }while(0)

#define MY_LOGW(fmt, arg...) \
    do { \
        CAM_ULOGW(Utils::ULog::MOD_3A_FRAMEWORK, fmt, ##arg); \
    }while(0)

#define MY_LOGE(fmt, arg...) \
    do { \
        CAM_ULOGE(Utils::ULog::MOD_3A_FRAMEWORK, fmt, ##arg); \
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

    MSize size(rArea.i4Right- rArea.i4Left,rArea.i4Bottom- rArea.i4Top);
    MPoint point(rArea.i4Left, rArea.i4Top);
    MRect input(point,size);
    MRect output;

    std::lock_guard<std::mutex> mapMatLock(Hal3AAdapter3::mMapMatLock);
    NSCamHW::HwMatrix &mat = Hal3AAdapter3::mMapMat[i4SensorIdx];
    mat.transform(input, output);

    rOut.i4Left   = output.p.x;
    rOut.i4Right  = output.p.x + output.s.w;
    rOut.i4Top    = output.p.y;
    rOut.i4Bottom = output.p.y + output.s.h;
    rOut.i4Weight = rArea.i4Weight;

    if (rOut.i4Left < 0) rOut.i4Left = 0;
    if (rOut.i4Right < 0) rOut.i4Right = 0;
    if (rOut.i4Top < 0) rOut.i4Top = 0;
    if (rOut.i4Bottom < 0) rOut.i4Bottom = 0;

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

    xOffset = 0;
    yOffset = 0;
    if(rOut.i4Right >= i4TgWidth)
        xOffset = (rOut.i4Right - i4TgWidth) + 1;
    if(rOut.i4Bottom >= i4TgHeight)
        yOffset = (rOut.i4Bottom - i4TgHeight) + 1;

    rOut.i4Left   = (rOut.i4Left >= xOffset) ? (rOut.i4Left - xOffset) : 0;
    rOut.i4Top    = (rOut.i4Top >= yOffset)  ? (rOut.i4Top - yOffset)  : 0;
    rOut.i4Right  = rOut.i4Right - xOffset;
    rOut.i4Bottom = rOut.i4Bottom - yOffset;

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
                MY_LOGD_IF(fgLogPf, "[%s] val(%ld)", __FUNCTION__, (long)val);
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


    MINT32 i4Magic = rP2Param.i4MagicNum;

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

    MINT32 i4Magic = rParam.i4MagicNum;

    if (u4Flag & _TEST_MANUAL_SENSOR)
    {
        MUINT32 u4Sensor = 0;
        MINT32 i4CamModeEnable = 0;
        GET_PROP("vendor.debug.hal3av3.sensor", 0, u4Sensor);
        GET_PROP("vendor.debug.camera.cammode", 0, i4CamModeEnable);
        if (i4CamModeEnable != 0)
        {
            MINT32 i4IsoSpeedMode = 0;
            GET_PROP("vendor.debug.camera.isospeed", 0, i4IsoSpeedMode);
            rParam.u4CamMode = i4CamModeEnable;
            rParam.i4IsoSpeedMode = i4IsoSpeedMode;
        }
        else
        {
            rParam.u4AeMode = MTK_CONTROL_AE_MODE_OFF;
            if (u4Sensor == 0)
            {
                rParam.i8ExposureTime = 3000000L + 3000000L * (i4Magic % 10);
                rParam.i4Sensitivity = 100;
                rParam.i8FrameDuration = 33000000L;
            }
            else if (u4Sensor == 1)
            {
                rParam.i8ExposureTime = 15000000L;
                rParam.i4Sensitivity = 100 + 100*(i4Magic%10);
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
                if (i4Magic & 0x1)
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
                rParam.i8FrameDuration = 3000000L + 3000000L * (i4Magic % 10);
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
        rParam.fColorCorrectGain[u4Channel] = (i4Magic % 2) ? 1.0f : 2.0f;
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

    #if HAL3A_TEST_OVERRIDE
    GET_PROP("vendor.debug.hal3av3.manual", 0, mu4ManualMode);
    GET_PROP("vendor.debug.hal3av3.testp2", 0, mu4ManualModeP2);
    #endif
    GET_PROP("vendor.debug.hal3av3.forcedump", 0, mi4ForceDebugDump);

    if (i4LogLevel) mu4LogEn |= HAL3AADAPTER3_LOG_PF;
    mu4DbgLogWEn = DebugUtil::getDebugLevel(DBG_3A);

    MY_LOGD("[%s] sensorIdx(%d) %s", __FUNCTION__, i4SensorIdx, strUser);

    if(i4SensorIdx >= SENSOR_IDX_MAX || i4SensorIdx < 0) {
        MY_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorIdx);
        return nullptr;
    }

    MY_INST& rSingleton = gMultiton[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<Hal3AAdapter3>(i4SensorIdx);
    } );
    (rSingleton.instance)->init(strUser);

    return rSingleton.instance.get();
}

Hal3AAdapter3::
Hal3AAdapter3(MINT32 const i4SensorIdx)
    : mpHal3aObj(NULL)
    , mpResultPoolObj(NULL)
    , mi4FrmId(-1)
    , mi4SensorIdx(i4SensorIdx)
    , mu4SensorDev(0)
    , mi4SensorMode(0)
    , mu4Counter(0)
    , mLock()
    , mLockIsp()
    , mLockResult()
    , mu1CapIntent(0)
    , mu1Precature(0)
    , mu1Start(0)
    , mu1FdEnable(0)
    , mu1HdrEnable(0)
    , mu4MetaResultQueueCapacity(0)
    , m_i4Presetkey(0) // Default value can't -1, because factory mode don't send this member
    , mi4PreTgWidth(0)
    , mi4PreTgHeight(0)
    , mi4AppfgCrop(0)
    , mLockConfigSend3ACtrl()
    , mSubSampleCount4Clear(0)
    , m_i4IsByPassCal(0)
    , m_u1LastAfTrig(0)
    , m_u1IsFirstParseMeta(0)
{
    ::memset(&m_rStaticInfo, 0, sizeof(m_rStaticInfo));
}

MVOID
Hal3AAdapter3::
destroyInstance(const char* strUser)
{
    MY_LOGD("[%s] sensorIdx(%d) User(%s)", __FUNCTION__, mi4SensorIdx, strUser);
    uninit(strUser);
}

MBOOL
Hal3AAdapter3::getStaticMeta()
{
    MBOOL bRet = MFALSE;
    // set staticInfo must do after init step, becuase MW updated static metadata after finish all init step - Muse.Sie W1633
    //sp<IMetadataProvider> pMetadataProvider = IMetadataProvider::create(mi4SensorIdx);
    android::sp<IMetadataProvider> pMetadataProvider = NSMetadataProviderManager::valueFor(mi4SensorIdx);
    if (pMetadataProvider != NULL)
    {
        mMetaStaticInfo = pMetadataProvider->getMtkStaticCharacteristics();
        bRet = MTRUE;
        //setupStaticInfo();
        // DEBUG
        // _printStaticMetadata(mMetaStaticInfo);
    }
    else
    {
        MY_LOGE("[%s] Fail to get static metadata: mi4SensorIdx(0x%04x)",
            __FUNCTION__, mi4SensorIdx);
    }
    MY_LOGD("[%s] Done - get Static Meta SensorIdx(%d)", __FUNCTION__, mi4SensorIdx);
    return bRet;
}

MINT32
Hal3AAdapter3::
config(const ConfigInfo_T& rConfigInfo)
{
    MY_LOGD("[%s]+ i4SubsampleCount(%d) i4RequestCount(%d)", __FUNCTION__, rConfigInfo.i4SubsampleCount, rConfigInfo.i4RequestCount);

    mParams.bIsFDReady = MTRUE;
    mParams.u1FaceDetectMode = 0;
    m_u1IsFirstParseMeta = 0;
    {
        std::lock_guard<std::mutex> lockConfigSend3ACtrl(mLockConfigSend3ACtrl);
        if (rConfigInfo.i4SubsampleCount > 1 && rConfigInfo.i4RequestCount > 1) //for SMVR
        {
            MY_LOGD("[%s] Uninit Normal 3A", __FUNCTION__);
            doUninit();
            m_rConfigInfo = rConfigInfo;
            mSubSampleCount4Clear = 0;
            MY_LOGD("[%s] Init SMVR 3A", __FUNCTION__);
            doInit();
            mParams.bIsFDReady = MTRUE;
        }
        else
        {
            if (m_rConfigInfo.i4SubsampleCount > 1 && m_rConfigInfo.i4RequestCount > 1)
            {
                MY_LOGD("[%s] Uninit SMVR 3A", __FUNCTION__);
                doUninit();
                m_rConfigInfo = rConfigInfo;
                mSubSampleCount4Clear = 0;
                MY_LOGD("[%s] Init Normal 3A", __FUNCTION__);
                doInit();
                mParams.bIsFDReady = MTRUE;
            }
        }
        MY_LOGD("[%s] lockConfigSend3ACtrl done", __FUNCTION__);
    }

    mi4PreTgWidth = 0;
    mi4PreTgHeight = 0;
    mi4AppfgCrop = 0;
    mbEnableOverride = ENABLE_OVERRIDE_SCENE_MODES_CONTROL;

    BASIC_CFG_INFO_T rBasicCfg;
    rBasicCfg.i4SensorIdx = mi4SensorIdx;
    rBasicCfg.i4SensorMode = mi4SensorMode;
    rBasicCfg.u4SensorDev = mu4SensorDev;
    mpResultPoolObj->config(rBasicCfg);

    // set staticInfo must do after init step, becuase MW updated static metadata after finish all init step - Muse.Sie W1633
    MBOOL bRet = getStaticMeta();

    // ResultPool - Set Meta Static Info
    if(bRet)
        mpResultPoolObj->setMetaStaticInfo(mMetaStaticInfo);


    setupStaticInfo();

    // Nelson Modified: MW pass new info via metadata in ConfigInfo_T, which has to be parsed
    ConfigInfo_T rConfigInfoOutPut;
    parseConfig(rConfigInfo, rConfigInfoOutPut);

    mpHal3aObj->setParams(mParams, MFALSE);

    mpHal3aObj->setSensorMode(mi4SensorMode);

    //
    MBOOL fgLog = (Hal3AAdapter3::mu4LogEn & HAL3AADAPTER3_LOG_AREA) ? MTRUE: MFALSE;
    HwTransHelper helper(mi4SensorIdx);
    NSCamHW::HwMatrix mat;
    if(!helper.getMatrixFromActive(mi4SensorMode, mat))
        MY_ERR("Get hw matrix failed");
    if(fgLog)
        mat.dump(__FUNCTION__);
    std::lock_guard<std::mutex> mapMatLock(Hal3AAdapter3::mMapMatLock);
    Hal3AAdapter3::mMapMat[mi4SensorIdx] = mat;
    //

    // ResultPool - Update magic #1 to resultpool for ISP HAL config flow
    MY_LOGD("[%s] Req(#%d) updateHistory/getValidateMetadata", __FUNCTION__, ConfigMagic);
    mpResultPoolObj->updateHistory(ConfigMagic, HistorySize);
    MBOOL ret = mpResultPoolObj->findValidateBuffer(ConfigMagic);

    MINT32 i4Ret = mpHal3aObj->config(rConfigInfoOutPut);
    MY_LOGD("[%s]- mMapMat size(%zu)", __FUNCTION__, mMapMat.size()); // unsigned long int
    return i4Ret;
}

MINT32
Hal3AAdapter3::
start(MINT32 /*i4StartNum*/)
{
    std::lock_guard<std::mutex> lock(mLock);
    MINT32 i4SensorPreviewDelay = 2;

    MY_LOGD("[%s]+ sensorDev(%d), sensorIdx(%d), mpHal3aObj(%p), mpResultPoolObj(%p)",
        __FUNCTION__, mu4SensorDev, mi4SensorIdx, mpHal3aObj, mpResultPoolObj);

#if 0
    mParams = Param_T();
    setupStaticInfo();
#endif
    mpHal3aObj->sendCommand(NS3Av3::ECmd_CameraPreviewStart);

    send3ACtrl(E3ACtrl_GetSensorPreviewDelay, reinterpret_cast<MINTPTR>(&i4SensorPreviewDelay), NULL);

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
    MY_LOGD("[%s]- %d Dummy Update Sent, sensorDev(%d), sensorIdx(%d),", __FUNCTION__, i4SensorPreviewDelay, mu4SensorDev, mi4SensorIdx);
    return 0;
}

MINT32
Hal3AAdapter3::
stop()
{
    std::lock_guard<std::mutex> lock(mLock);
    MY_LOGD("[%s]+ sensorDev(%d), sensorIdx(%d), mpHal3aObj(%p)", __FUNCTION__, mu4SensorDev, mi4SensorIdx, mpHal3aObj);

    if (mu1Start)
    {
        // ResultPool - Clear ResultPool Info
        mpResultPoolObj->clearAllResultPool();

#if CAM3_STEREO_FEATURE_EN
        MBOOL bIsActive = MFALSE;
        mpHal3aObj->send3ACtrl(E3ACtrl_Sync3A_IsActive, (MINTPTR)&bIsActive, 0);

        if (bIsActive)
        {
            Stereo_Param_T rStereoParam;
            rStereoParam.i4HwSyncMode = NS3Av3::E_HW_FRM_SYNC_MODE::E_HW_FRM_SYNC_MODE_OFF;
            rStereoParam.i4Sync2AMode= NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_NONE;
            rStereoParam.i4SyncAFMode= NS3Av3::E_SYNCAF_MODE::E_SYNCAF_MODE_OFF;
            rStereoParam.i4FrameNum = 0x7FFFFFFF;
            mpHal3aObj->send3ACtrl(E3ACtrl_SetStereoParams, (MINTPTR)&rStereoParam, 0);
        }
        else
        {
            //Notice: HWSync and Sync2A decouple for ISP60. So disable HwSync without isActive true.
            Stereo_Param_T rStereoParam;
            rStereoParam.i4HwSyncMode = NS3Av3::E_HW_FRM_SYNC_MODE::E_HW_FRM_SYNC_MODE_OFF;
            mpHal3aObj->send3ACtrl(E3ACtrl_SetStereoParams, (MINTPTR)&rStereoParam, 0);
        }

        mStereoParam.i4MasterIdx = 0;
        mStereoParam.i4SlaveIdx = 0;
        mStereoParam.i4HwSyncMode = NS3Av3::E_HW_FRM_SYNC_MODE::E_HW_FRM_SYNC_MODE_IDLE;
        mStereoParam.i4Sync2AMode= NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_IDLE;
        mStereoParam.i4SyncAFMode= NS3Av3::E_SYNCAF_MODE::E_SYNCAF_MODE_IDLE;
        mStereoParam.bIsByFrame = MFALSE;
        mStereoParam.bIsDummyFrame = MFALSE;
#endif
        m_u1LastAfTrig = 0;
        mpHal3aObj->sendCommand(NS3Av3::ECmd_CameraPreviewEnd);
        MY_LOGD("[%s]- sensorDev(%d), sensorIdx(%d), mpHal3aObj(%p)", __FUNCTION__, mu4SensorDev, mi4SensorIdx, mpHal3aObj);
        return 0;
    }
    else
    {
        MY_LOGE("[%s]- Not yet started! sensorDev(%d), sensorIdx(%d), mpHal3aObj(%p)",
            __FUNCTION__, mu4SensorDev, mi4SensorIdx, mpHal3aObj);
        return 1;
    }
}

MVOID
Hal3AAdapter3::
stopStt()
{
    std::lock_guard<std::mutex> lock(mLock);
    MY_LOGD("[%s]+ sensorDev(%d), sensorIdx(%d), mpHal3aObj(%p)", __FUNCTION__, mu4SensorDev, mi4SensorIdx, mpHal3aObj);

    if (mu1Start)
    {
        mpHal3aObj->stopStt();
        MY_LOGD("[%s]- sensorDev(%d), sensorIdx(%d), mpHal3aObj(%p)", __FUNCTION__, mu4SensorDev, mi4SensorIdx, mpHal3aObj);
        return;
    }
    else
    {
        MY_LOGE("[%s]- Not yet started! sensorDev(%d), sensorIdx(%d), mpHal3aObj(%p)",
            __FUNCTION__, mu4SensorDev, mi4SensorIdx, mpHal3aObj);
        return;
    }
}

MVOID
Hal3AAdapter3::
pause()
{
    MY_LOG("[%s] + sensorDev(%d), sensorIdx(%d)", __FUNCTION__, mu4SensorDev, mi4SensorIdx);
    mpHal3aObj->pause();
    MY_LOG("[%s] - sensorDev(%d), sensorIdx(%d)", __FUNCTION__, mu4SensorDev, mi4SensorIdx);
}

MVOID
Hal3AAdapter3::
resume(MINT32 MagicNum)
{
    MY_LOG("[%s] + sensorDev(%d), sensorIdx(%d)", __FUNCTION__, mu4SensorDev, mi4SensorIdx);
    mpHal3aObj->resume(MagicNum);
    MY_LOG("[%s] - sensorDev(%d), sensorIdx(%d)", __FUNCTION__, mu4SensorDev, mi4SensorIdx);
}


MBOOL
Hal3AAdapter3::
init(const char* strUser)
{
    std::lock_guard<std::mutex> lock(mLock);

    GET_PROP("vendor.debug.hal3av3.aaatrace", 0, AAA_TRACE_LEVEL);

    if (m_Users.size())
    {
        MY_LOGD("[%s] sensorDev(%d), sensorIdx(%d), Still %zu users", __FUNCTION__, mu4SensorDev, mi4SensorIdx, m_Users.size());
    }
    else
    {
        doInit();
    }

    m_Users[string(strUser)]++;

    MY_LOGD("[%s] sensorDev(%d), sensorIdx(%d), Done m_rConfigInfo.i4SubsampleCount(%d), User.count(%zu), User init(%s)", __FUNCTION__, mu4SensorDev, mi4SensorIdx, m_rConfigInfo.i4SubsampleCount, m_Users.size(), strUser);
    return MTRUE;
}

MBOOL
Hal3AAdapter3::
uninit(const char* strUser)
{
    std::lock_guard<std::mutex> lock(mLock);

    if (!m_Users[string(strUser)])  MY_LOGE("User(%s) did not create 3A!", strUser);
    else
    {
        m_Users[string(strUser)]--;
        if (!m_Users[string(strUser)])
        {
            m_Users.erase(string(strUser));
            if (!m_Users.size())    doUninit();
            else    MY_LOGD("[%s] sensorDev(%d), sensorIdx(%d), Still %zu users", __FUNCTION__, mu4SensorDev, mi4SensorIdx, m_Users.size());
        }
    }

    MY_LOGD("[%s] sensorDev(%d), sensorIdx(%d), Done m_rConfigInfo.i4SubsampleCount(%d), User.count(%zu), User uninit(%s)", __FUNCTION__, mu4SensorDev, mi4SensorIdx, m_rConfigInfo.i4SubsampleCount, m_Users.size(), strUser);
    return MTRUE;
}

MINT32
Hal3AAdapter3::
buildSceneModeOverride()
{
    MY_LOG("[%s] +", __FUNCTION__);
    // override
    const IMetadata::IEntry& entryAvailableScene = mMetaStaticInfo.entryFor(MTK_CONTROL_AVAILABLE_SCENE_MODES);
    if (entryAvailableScene.isEmpty())
    {
        MY_LOGE("[%s] MTK_CONTROL_AVAILABLE_SCENE_MODES are not defined", __FUNCTION__);
    }
    else
    {
        const IMetadata::IEntry& entryScnOvrd = mMetaStaticInfo.entryFor(MTK_CONTROL_SCENE_MODE_OVERRIDES);
        if (entryScnOvrd.isEmpty())
        {
            MY_LOGE("[%s] MTK_CONTROL_SCENE_MODE_OVERRIDES are not defined", __FUNCTION__);
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
                    //m_ScnModeOvrd.add(u1ScnMode, rMode3A);
                    m_ScnModeOvrd.insert(pair<MUINT8, Mode3A_T>(u1ScnMode, rMode3A));
                    MY_LOGD_IF(fgLog, "[%s] Scene mode(%d) overrides AE(%d), AWB(%d), AF(%d)", __FUNCTION__, i, rMode3A.u1AeMode, rMode3A.u1AwbMode, rMode3A.u1AfMode);
                }
            }
        }
    }
    MY_LOG("[%s] -", __FUNCTION__);
    return 0;
}

MINT32
Hal3AAdapter3::
setupStaticInfo()
{
    MY_LOG("[%s] +", __FUNCTION__);
    MBOOL fgLog = (mu4LogEn & HAL3AADAPTER3_LOG_PF) ? MTRUE : MFALSE;

    buildSceneModeOverride();

    // AE Comp Step
    MRational rStep;
    if (QUERY_ENTRY_SINGLE(mMetaStaticInfo, MTK_CONTROL_AE_COMPENSATION_STEP, rStep))
    {
        mParams.fExpCompStep = (MFLOAT) rStep.numerator / rStep.denominator;
        MY_LOGD_IF(fgLog, "[%s] ExpCompStep(%3.3f), (%d/%d)", __FUNCTION__, mParams.fExpCompStep, rStep.numerator, rStep.denominator);
    }

    MINT32 rRgn[3] = {};
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

    if ((m_rStaticInfo.u1RollingShutterSkew = GET_ENTRY_SINGLE_IN_ARRAY(mMetaStaticInfo, MTK_REQUEST_AVAILABLE_RESULT_KEYS, (MINT32)MTK_SENSOR_ROLLING_SHUTTER_SKEW)))
    {
        MY_LOGD_IF(fgLog, "[%s] RollingShutterSkew(%d)", __FUNCTION__, m_rStaticInfo.u1RollingShutterSkew);
        mParams.u1RollingShutterSkew = m_rStaticInfo.u1RollingShutterSkew;
    }
    else
    {
        mParams.u1RollingShutterSkew = 0;
    }

    MUINT8 u1SubFlashCustomization = 0;
    if (QUERY_ENTRY_SINGLE(mMetaStaticInfo, MTK_FLASH_FEATURE_CUSTOMIZATION_AVAILABLE, u1SubFlashCustomization))
    {
        MY_LOGD_IF(u1SubFlashCustomization, "[%s] custom panel flash support(%d)", __FUNCTION__, u1SubFlashCustomization);
        mParams.u1SubFlashCustomization = u1SubFlashCustomization;
    }

    MY_LOG("[%s] -", __FUNCTION__);
    return 0;
}

MINT32
Hal3AAdapter3::
startCapture(const List<MetaSet_T>& requestQ, MINT32 /*i4StartNum*/, MINT32 i4RequestQSize)
{
    MY_LOGD("[%s]+ Old startCapture", __FUNCTION__);

    vector<MetaSet_T*> transfer;
    MetaSet_T target;
    if (std::max(i4RequestQSize, (MINT32)requestQ.size()) >= 3)
    {
        List<MetaSet_T>::const_iterator it = requestQ.begin();
        it++; it++;
        target = *it;
        transfer.push_back(&target);
    }
    else
    {
        MY_LOGE("[%s] Start Capture Request Less Than 3 !", __FUNCTION__);
        return 0;
    }

    startCapture(transfer);
    MY_LOG("[%s]- ", __FUNCTION__);
    return MFALSE;
}

MINT32
Hal3AAdapter3::
startRequestQ(const android::List<MetaSet_T>& requestQ, MINT32 i4RequestQSize)
{
    MY_LOGD("[%s]+ ", __FUNCTION__);
    vector<MetaSet_T> local;
    for (auto& it : requestQ)
        local.push_back(it);

    vector<MetaSet_T*> transfer;
    for (auto& it : local)
        transfer.push_back(&it);


    startRequestQ(transfer);

    MY_LOGD("[%s]- ", __FUNCTION__);
    return MFALSE;
}

MINT32
Hal3AAdapter3::
set(const List<MetaSet_T>& requestQ, MINT32 i4RequestQSize)
{
    MY_LOGD_IF(mu4LogEn & HAL3AADAPTER3_LOG_SET_0, "[%s] Old set", __FUNCTION__);
    if (m_rConfigInfo.i4SubsampleCount > 1 && m_rConfigInfo.i4RequestCount > 1)  //Sorry for the following dumbass violence code, due to the constantness of every single elements in requestQ, we have no choice but to copy each of 'em
    {
        vector<MetaSet_T> local;
        for (auto& it : requestQ)
            local.push_back(it);

        vector<MetaSet_T*> transfer;
        for (auto& it : local)
            transfer.push_back(&it);

        setSMVR(transfer);
    }
    else
    {
        vector<MetaSet_T*> transfer;
        MetaSet_T target;
        if (std::max(i4RequestQSize, (MINT32)requestQ.size()) >= 3)
        {
            List<MetaSet_T>::const_iterator it = requestQ.begin();
            it++; it++;
            target = *it;
            transfer.push_back(&target);
        }
        else
        {
            target.Dummy = 1;
            transfer.push_back(&target);
        }

        setNormal(transfer);
    }

    return 0;
}

MINT32
Hal3AAdapter3::
setNormal(const List<MetaSet_T>& requestQ, MINT32 i4RequestQSize)
{
    return 0;
}

MINT32
Hal3AAdapter3::
setSMVR(const List<MetaSet_T>& requestQ, MINT32 i4RequestQSize)
{
    return 0;
}

MINT32
Hal3AAdapter3::
startCapture(const vector<MetaSet_T*>& requestQ, MINT32 /*i4StartNum*/)
{
    std::lock_guard<std::mutex> lock(mLock);
    MY_LOGD("[%s]+ sensorDev(%d), sensorIdx(%d), mpHal3aObj(%p)",
        __FUNCTION__, mu4SensorDev, mi4SensorIdx, mpHal3aObj);
    MINT32 i4Capture_type = ESTART_CAP_NORMAL;
    MUINT8 u1IsSkipHQC = 0;

    mpHal3aObj->sendCommand(NS3Av3::ECmd_CameraPreviewStart);
    {
        MUINT8 u1AeMode = 0;

        QUERY_ENTRY_SINGLE(requestQ[0]->appMeta, MTK_CONTROL_AE_MODE, u1AeMode);
        QUERY_ENTRY_SINGLE(requestQ[0]->halMeta, MTK_3A_SKIP_HIGH_QUALITY_CAPTURE, u1IsSkipHQC);
        MY_LOGD("[%s] MTK_CONTROL_AE_MODE(%d) MTK_3A_SKIP_HIGH_QUALITY_CAPTURE(%d)", __FUNCTION__, u1AeMode, u1IsSkipHQC);

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

            MY_LOGD("[%s]Dummy Update Sent, sensorDev(%d)", __FUNCTION__, mu4SensorDev);

            mParams.u1IsStartCapture = MTRUE;
            if (u1IsSkipHQC)
            {
                mParams.u1IsStartCapture = MFALSE;
            }
            MY_LOGD("[%s] IsStartCapture(%d)", __FUNCTION__, mParams.u1IsStartCapture);

            set(requestQ);
            mParams.u1IsStartCapture = MFALSE;
        }
        else
        {
#if CAM3_STEREO_FEATURE_EN
            if (m_i4IsByPassCal == MTRUE)
            {
                i4Capture_type = ESTART_CAP_NORMAL;
                set(requestQ);
            }
            else
#endif
            {
                MINT32 i4SensorModeDelay = 0;
                send3ACtrl(E3ACtrl_GetSensorSyncInfo, reinterpret_cast<MINTPTR>(&i4SensorModeDelay), NULL);
                MY_LOGD("[%s] i4SensorModeDelay(%d)", __FUNCTION__, i4SensorModeDelay);
                if (i4SensorModeDelay > 0)
                {
                    i4Capture_type = ESTART_CAP_SPECIAL;
                }
                i4SensorModeDelay = i4SensorModeDelay - 2;
                if (i4SensorModeDelay < 0)
                {
                    i4SensorModeDelay = 0;
                }
                RequestSet_T rRequestSet;
                NS3Av3::ParamIspProfile_T _3AProf(NSIspTuning::EIspProfile_Preview, 1, 0, MTRUE, NS3Av3::ParamIspProfile_T::EParamValidate_None, rRequestSet);
                for (int i = 0; i <= i4SensorModeDelay; i++)
                {
                    _3AProf.i4MagicNum = i;
                    rRequestSet.vNumberSet.clear();
                    rRequestSet.vNumberSet.push_back(i);
                    mpHal3aObj->sendCommand(NS3Av3::ECmd_Update, reinterpret_cast<MUINTPTR>(&_3AProf));
                }

                mParams.u1IsStartCapture = MTRUE;
                if (u1IsSkipHQC)
                {
                    mParams.u1IsStartCapture = MFALSE;
                }
                MY_LOGD("[%s] IsStartCapture(%d)", __FUNCTION__, mParams.u1IsStartCapture);

                set(requestQ);
                mParams.u1IsStartCapture = MFALSE;
            }
        }
    }
    mu1Start = 1;

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
        MY_LOGD("[%s]- ", __FUNCTION__);
        return ESTART_CAP_MANUAL;
    }
    else
    {
        MY_LOGD("[%s]- Capture_type %d", __FUNCTION__, i4Capture_type);
        return i4Capture_type;
    }
}

MINT32
Hal3AAdapter3::
startRequestQ(const std::vector<MetaSet_T*>& requestQ)
{
#define REQ_SIZE 3
    std::lock_guard<std::mutex> lock(mLock);

    MY_LOGD("[%s]+ sensorDev(%d), sensorIdx(%d), Clear mpResultPoolObj(%p)", __FUNCTION__, mu4SensorDev, mi4SensorIdx, mpResultPoolObj);

    /*************************************************************************************
       ***********  Jason will pass down several MetaSet_T where the last one contains the real setting  **********
       ***********    We have to extract real IspProfile along with CapIntent to Prevent Screen flikering    **********
       ************                                                                                                                              ***********
        *************************************************************************************/
    // Fetching Real Capture Intent
    MUINT8 CapIntent = 0;
    for (MINT32 i = 0; i < requestQ.back()->appMeta.count(); i++)
    {
        IMetadata::IEntry entry = requestQ.back()->appMeta.entryAt(i);
        mtk_camera_metadata_tag_t tag = (mtk_camera_metadata_tag_t)entry.tag();

        if (tag == MTK_CONTROL_CAPTURE_INTENT){
            CapIntent = entry.itemAt(0, Type2Type< MUINT8 >());
            break;
        }
    }

    // Determine Real IspProfile
    MUINT8 IspProfile = 0xFF;
    if (!QUERY_ENTRY_SINGLE(requestQ.back()->halMeta, MTK_3A_ISP_PROFILE, IspProfile))
    {
        switch (CapIntent)
        {
            case MTK_CONTROL_CAPTURE_INTENT_PREVIEW:
            case MTK_CONTROL_CAPTURE_INTENT_ZERO_SHUTTER_LAG:
                IspProfile = NSIspTuning::EIspProfile_Preview;
                break;
            case MTK_CONTROL_CAPTURE_INTENT_VIDEO_RECORD:
            case MTK_CONTROL_CAPTURE_INTENT_VIDEO_SNAPSHOT:
                IspProfile = NSIspTuning::EIspProfile_Video;
                break;
            default:
                IspProfile = NSIspTuning::EIspProfile_Preview;
                break;
        }
    }

    // Set IspProfile
    mpHal3aObj->send3ACtrl(E3ACtrl_SetIspProfile, (MINTPTR)IspProfile, 0);

    /********************************************************************************/

    mpHal3aObj->sendCommand(NS3Av3::ECmd_CameraPreviewStart);

    MINT32 i4SensorPreviewDelay = 2;
    send3ACtrl(E3ACtrl_GetSensorPreviewDelay, reinterpret_cast<MINTPTR>(&i4SensorPreviewDelay), NULL);

    // dummy request
    for(MINT32 i = 0; i < i4SensorPreviewDelay; i++)
    {
        RequestSet_T rRequestSet;
        rRequestSet.vNumberSet.clear();
        rRequestSet.vNumberSet.push_back(1);
        NS3Av3::ParamIspProfile_T _3AProf(NSIspTuning::EIspProfile_Preview, 1, 0, MTRUE, NS3Av3::ParamIspProfile_T::EParamValidate_None, rRequestSet);
        mpHal3aObj->sendCommand(NS3Av3::ECmd_Update, reinterpret_cast<MUINTPTR>(&_3AProf));
    }

    for (MINT32 i = 0; i < requestQ.size(); i++)
    {
        // always get the last metadata do set function
        // the other metadata do validate tuning and copy metadata to queue.
        if( i == (requestQ.size()-1) )
        {
            std::vector<MetaSet_T*> newRequestQ;
            MetaSet_T rMetaResult =(*requestQ[i]);
            rMetaResult.halMeta = requestQ[i]->halMeta;
            rMetaResult.appMeta = requestQ[i]->appMeta;
            UPDATE_ENTRY_SINGLE<MUINT8>(rMetaResult.halMeta, MTK_HAL_REQUEST_REPEAT, 0);

            // for debug
            MUINT8 u1AeMode = 0;
            MUINT8 uRepeatTag = 0;
            QUERY_ENTRY_SINGLE(rMetaResult.appMeta, MTK_CONTROL_AE_MODE, u1AeMode);
            QUERY_ENTRY_SINGLE(rMetaResult.halMeta, MTK_HAL_REQUEST_REPEAT, uRepeatTag);
            MY_LOGD("[%s] magic(%d), MTK_CONTROL_AE_MODE(%d), MTK_HAL_REQUEST_REPEAT(%d)", __FUNCTION__, rMetaResult.MagicNum, u1AeMode, uRepeatTag);
            newRequestQ.push_back(&rMetaResult);
            set(newRequestQ);
        }
        else
        {
            // ResultPool - Get validate buffer by request magic
            MBOOL ret = MTRUE;
            if(requestQ[i]->MagicNum != ConfigMagic)
                ret = mpResultPoolObj->findValidateBuffer(requestQ[i]->MagicNum);

            if(ret)
            {
                // ResultPool - Update config information
                RESULT_CFG_T rResultCfg;
                rResultCfg.i4ReqMagic = requestQ[i]->MagicNum;
                rResultCfg.i4StatisticMagic = InvalidValue;
                ret = mpResultPoolObj->updateResultCFG(rResultCfg);

                AllMetaResult_T* pMetadata = mpResultPoolObj->getMetadata(requestQ[i]->MagicNum);
                MetaSet_T* prMetaResult = NULL;
                if(pMetadata != NULL)
                {
                    std::lock_guard<std::mutex> lock(pMetadata->LockMetaResult);
                    prMetaResult = &(pMetadata->rMetaResult);
                    *prMetaResult = (*requestQ[i]);
                    UPDATE_ENTRY_SINGLE(prMetaResult->halMeta, MTK_3A_ISP_PROFILE, IspProfile);
                    mpHal3aObj->send3ACtrl(E3ACtrl_ValidatePass1, prMetaResult->MagicNum, (MINTPTR)IspProfile);
                }
            }
            else
                MY_LOGE("[%s] findValidateBuffer fail!", __FUNCTION__);
        }
    }
    mu1Start = 1;
    MY_LOGD("[%s]- i4SensorPreviewDelay(%d)", __FUNCTION__, i4SensorPreviewDelay);
    return MFALSE;
}

MINT32
Hal3AAdapter3::
set(const vector<MetaSet_T*>& requestQ)
{
    if (m_rConfigInfo.i4SubsampleCount > 1 && m_rConfigInfo.i4RequestCount > 1)  //Slow motion video recording
    {
        setSMVR(requestQ);
    }
    else
    {
        setNormal(requestQ);
    }

    return 0;
}

MINT32
Hal3AAdapter3::
preset(const vector<MetaSet_T*>& requestQ)
{
    MY_LOGD_IF(mu4LogEn & HAL3AADAPTER3_LOG_PF, "[%s] Preset has Key of %d", __FUNCTION__, requestQ[0]->PreSetKey);

    std::lock_guard<std::mutex> _lock(mutexParseMeta);    // Preset would contend this mutex with Vsync_Done Callback to ensure that there is no consecutive Preset taking place

    m_i4Presetkey = requestQ[0]->PreSetKey;     // Store Presetkey to determine whether the following setNormal should be processed or not ( if same, skip )
    mParams.u1Dummy = requestQ[0]->Dummy;

    mu1RepeatTag = parseMeta(requestQ);
    mParams.i4PresetKey = m_i4Presetkey;

#if HAL3A_TEST_OVERRIDE
    if(mu4ManualMode != 0) _test(mu4ManualMode, mParams);
#endif

    MY_LOGD_IF(mParams.bDummyAfterCapture, "[%s] DummyAfterCapture: %d i4ExpIndex: %d -> 0", __FUNCTION__, mParams.bDummyAfterCapture, mParams.i4ExpIndex);
    if (mParams.bDummyAfterCapture)
    {
        mParams.i4ExpIndex = 0;
    }

    // AF trigger start
    if (mAfParams.u1AfTrig == MTK_CONTROL_AF_TRIGGER_START)
    {
        MY_LOGD("[%s] Special flow to modify AEmode for setting AfTrig(%d) and AEmode(%d) the same frame", __FUNCTION__, mAfParams.u1AfTrig, mParams.u4AeMode);
        mpHal3aObj->send3ACtrl(E3ACtrl_SetParamAEMode, (MINTPTR)&(mParams.u4AeMode), NULL);
        mpHal3aObj->autoFocus();
    }
    // AF trigger cancel
    else if (mAfParams.u1AfTrig == MTK_CONTROL_AF_TRIGGER_CANCEL)
        mpHal3aObj->cancelAutoFocus();

    // AE Precapture start
    if ( (mParams.u1PrecapTrig == MTK_CONTROL_AE_PRECAPTURE_TRIGGER_START &&
        mParams.u4AeMode != MTK_CONTROL_AE_MODE_OFF) || mParams.u1PreCapStart)
    {
        MY_LOGD("[%s] Special flow to modify AEmode for setting PrecapTrig(%d) and AEmode(%d) the same frame", __FUNCTION__, mParams.u1PrecapTrig, mParams.u4AeMode);
        mpHal3aObj->send3ACtrl(E3ACtrl_SetParamAEMode, (MINTPTR)&(mParams.u4AeMode), NULL);
        mpHal3aObj->sendCommand(NS3Av3::ECmd_PrecaptureStart);
    }

    // AE Precapture End
    if (mParams.u1PrecapTrig == MTK_CONTROL_AE_PRECAPTURE_TRIGGER_CANCEL &&
        mParams.u4AeMode != MTK_CONTROL_AE_MODE_OFF)
        mpHal3aObj->sendCommand(NS3Av3::ECmd_PrecaptureEnd);

    mpHal3aObj->preset(mParams);

    return 0;
}

MUINT8
Hal3AAdapter3::
parseMeta(const vector<MetaSet_T*>& requestQ)
{
    AAA_TRACE_HAL(initialize);

#define OVERRIDE_AE  (1<<0)
#define OVERRIDE_AWB (1<<1)
#define OVERRIDE_AF  (1<<2)

    MINT32 i4ForceFace3A = 0;
    MINT32 i4DynamicSubsampleCount = 1;
    MUINT8 u1RepeatTag = 0;
    MUINT8 u1Override3A = (OVERRIDE_AE|OVERRIDE_AWB|OVERRIDE_AF);
    MUINT8 u1IspProfile = 0xFF;

    MBOOL fgLog    = mu4LogEn & HAL3AADAPTER3_LOG_PF;
    MBOOL fgLogEn0 = mu4LogEn & HAL3AADAPTER3_LOG_SET_0;
    MBOOL fgLogEn1 = mu4LogEn & HAL3AADAPTER3_LOG_SET_1;
    MBOOL fgLogEn2 = mu4LogEn & HAL3AADAPTER3_LOG_SET_2;

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

    MY_LOGD_IF(fgLog, "[%s] sensorDev(%d), sensorIdx(%d), mu4Counter(%d), TG size(%d,%d) Hbin size(%d,%d) Ratio(%d,%d)", __FUNCTION__,
        mu4SensorDev, mi4SensorIdx, mu4Counter++, i4TgWidth, i4TgHeight, i4HbinWidth, i4HbinHeight, i4Tg2HbinRatioW, i4Tg2HbinRatioH);

    AAA_TRACE_END_HAL;

    AAA_TRACE_HAL(ParseHalMeta);

    MRect rSclCropRect;
    MINT32 rSclCrop[4];
    MBOOL fgCrop = MFALSE;

    MetaSet_T* it = requestQ[0];
    const IMetadata& _appmeta = it->appMeta;
    const IMetadata& _halmeta = it->halMeta;

    // parsing repeat tag
    QUERY_ENTRY_SINGLE(_halmeta, MTK_HAL_REQUEST_REPEAT, u1RepeatTag);
    QUERY_ENTRY_SINGLE(_halmeta, MTK_3A_ISP_PROFILE, u1IspProfile);

    u1RepeatTag = (m_u1IsFirstParseMeta && u1RepeatTag);
    mu1RepeatTag = u1RepeatTag;

    MINT32 i4IsoSpeedMode = 0;
    if(QUERY_ENTRY_SINGLE(_halmeta, MTK_HAL_REQUEST_ISO_SPEED, i4IsoSpeedMode)) // Hal1
    {
        MY_LOGD_IF(mParams.i4IsoSpeedMode != i4IsoSpeedMode, "[%s] MTK_HAL_REQUEST_ISO_SPEED(%d -> %d)", __FUNCTION__, mParams.i4IsoSpeedMode, i4IsoSpeedMode);
        mParams.i4IsoSpeedMode = i4IsoSpeedMode;
    }
    else if(QUERY_ENTRY_SINGLE(_appmeta, MTK_3A_FEATURE_AE_REQUEST_ISO_SPEED, i4IsoSpeedMode)) // Hal3
    {
        MY_LOGD_IF(mParams.i4IsoSpeedMode != i4IsoSpeedMode, "[%s] MTK_3A_FEATURE_AE_REQUEST_ISO_SPEED(%d -> %d)", __FUNCTION__, mParams.i4IsoSpeedMode, i4IsoSpeedMode);
        mParams.i4IsoSpeedMode = i4IsoSpeedMode;
    }

    MINT32 i4AwbValue = 0;
    if(QUERY_ENTRY_SINGLE(_appmeta, MTK_3A_FEATURE_AWB_REQUEST_VALUE, i4AwbValue))
    {
        MY_LOGD_IF(mParams.i4AwbValue != i4AwbValue, "[%s] MTK_3A_FEATURE_AWB_REQUEST_VALUE(%d -> %d)", __FUNCTION__, mParams.i4AwbValue, i4AwbValue);
        mParams.i4AwbValue = i4AwbValue;
    }

    MUINT8 u1AeMeteringMode = 0;
    if(QUERY_ENTRY_SINGLE(_appmeta, MTK_3A_FEATURE_AE_REQUEST_METERING_MODE, u1AeMeteringMode))
    {
        MY_LOGD_IF(mParams.u4AeMeterMode != u1AeMeteringMode, "[%s] MTK_3A_FEATURE_AE_REQUEST_METERING_MODE(%d -> %d)", __FUNCTION__, mParams.u4AeMeterMode, u1AeMeteringMode);
        mParams.u4AeMeterMode = static_cast<MUINT32>(u1AeMeteringMode);
    }

    // flash calibration enable
    MINT32 i4FlashCalEn = 0;
    if(QUERY_ENTRY_SINGLE(_appmeta, MTK_FLASH_FEATURE_CALIBRATION_ENABLE, i4FlashCalEn))
    {
        MY_LOGD_IF(mParams.i4FlashCalEn != i4FlashCalEn, "[%s] MTK_FLASH_FEATURE_CALIBRATION_ENABLE(%d -> %d)", __FUNCTION__, mParams.i4FlashCalEn, i4FlashCalEn);
        mParams.i4FlashCalEn = i4FlashCalEn;
    }

    MINT32 i4ManualPlineIdx = 0;
    if(QUERY_ENTRY_SINGLE(_appmeta, MTK_3A_FEATURE_AE_PLINE_INDEX, i4ManualPlineIdx))
    {
        MY_LOGD_IF(mParams.i4ManualPlineIdx != i4ManualPlineIdx, "[%s] MTK_CONTROL_AE_PLINE_INDEX(%d)", __FUNCTION__, i4ManualPlineIdx);
        mParams.i4ManualPlineIdx = i4ManualPlineIdx;
    } else {
        mParams.i4ManualPlineIdx = 0;
        MY_LOGD("[%s] Set Manual Pline Index back to 0", __FUNCTION__);
    }

    MINT32 i4CustomPlineMode = 0;
    if(QUERY_ENTRY_SINGLE(_appmeta, MTK_3A_FEATURE_AE_CUSTOM_PLINE_MODE, i4CustomPlineMode))
    {
        MY_LOGD_IF(mParams.i4CustomPlineMode != i4CustomPlineMode, "[%s] MTK_3A_FEATURE_AE_CUSTOM_PLINE_MODE(%d)", __FUNCTION__, i4CustomPlineMode);
        mParams.i4CustomPlineMode = i4CustomPlineMode;
    } else {
        mParams.i4CustomPlineMode = 0;
        MY_LOGD("[%s] Set Custom Pline Mode back to 0", __FUNCTION__);
    }

    if (!QUERY_ENTRY_SINGLE(_halmeta, MTK_P1NODE_RAW_TYPE, mParams.i4RawType))
        mParams.i4RawType = NSIspTuning::ERawType_Proc;

    // for Dynamic subsample count
    if (!QUERY_ENTRY_SINGLE(_halmeta, MTK_3A_DYNAMIC_SUBSAMPLE_COUNT, i4DynamicSubsampleCount))
        mParams.i4DynamicSubsampleCount = i4DynamicSubsampleCount;

    // Dual PDAF support for EngMode
    QUERY_ENTRY_SINGLE(_halmeta, MTK_HAL_REQUEST_PASS1_DISABLE, mParams.i4DisableP1);

    // for HAL HDR
    if (!QUERY_ENTRY_SINGLE(_halmeta, MTK_3A_AE_CAP_PARAM, mParams.rcapParams))
        mParams.rcapParams = IMetadata::Memory();

    // parsing manual AWB ColorTemperature
    QUERY_ENTRY_SINGLE(_halmeta, MTK_3A_MANUAL_AWB_COLORTEMPERATURE, mParams.i4MWBColorTemperature);

    if (!QUERY_ENTRY_SINGLE(_halmeta, MTK_3A_AE_CAP_SINGLE_FRAME_HDR, mParams.u1IsSingleFrameHDR))
        mParams.u1IsSingleFrameHDR = 0;

    // for HAL to pause AF.
    if (!QUERY_ENTRY_SINGLE(_halmeta, MTK_FOCUS_PAUSE, mAfParams.u1AfPause))
        mAfParams.u1AfPause = 0;

    // for HAL to set MZ on/off.
    if (!QUERY_ENTRY_SINGLE(_halmeta, MTK_FOCUS_MZ_ON, mAfParams.u1MZOn))
        mAfParams.u1MZOn = 255;

    // only for capture intent, preview don't use
    if( !QUERY_ENTRY_SINGLE(_halmeta, MTK_3A_DUMMY_BEFORE_REQUEST_FRAME, mParams.bDummyBeforeCapture))
        mParams.bDummyBeforeCapture = MFALSE;
    if( !QUERY_ENTRY_SINGLE(_halmeta, MTK_3A_DUMMY_AFTER_REQUEST_FRAME, mParams.bDummyAfterCapture))
        mParams.bDummyAfterCapture = MFALSE;

    if (!QUERY_ENTRY_SINGLE(_halmeta, MTK_HAL_REQUEST_HIGH_QUALITY_CAP, mParams.u1HQCapEnable))
        mParams.u1HQCapEnable = 0;

    if (!QUERY_ENTRY_SINGLE(_halmeta, MTK_HAL_REQUEST_REQUIRE_EXIF, mParams.u1IsGetExif))
        mParams.u1IsGetExif = 0;

    MINT32 i4Cshot = 0;
    if (QUERY_ENTRY_SINGLE(_appmeta, MTK_CSHOT_FEATURE_CAPTURE, i4Cshot))
    {
        MY_LOGD_IF(mParams.i4Cshot != i4Cshot, "[%s] MTK_CSHOT_FEATURE_CAPTURE(%d -> %d)", __FUNCTION__, mParams.i4Cshot, i4Cshot);
        mParams.i4Cshot = i4Cshot;
    }
    else
    {
        mParams.i4Cshot = i4Cshot;
    }

    MUINT8 u1MtkHdrMode = static_cast<MUINT8>(HDRMode::OFF);
    QUERY_ENTRY_SINGLE(_halmeta, MTK_3A_HDR_MODE, u1MtkHdrMode);

    MY_LOGD_IF(fgLogEn0,"[%s] IsFirstParseMeta(%d) RepeatTag(%d), AeIsoSpeedMode(%d), IspProfile(%d), RawType(%d), DisableP1(%d), IsSingleFrameHDR(%d), MtkHdrMode(%d), MWBColorTemperature(%d), DummyAfterCapture(%d)",
                        __FUNCTION__, m_u1IsFirstParseMeta, u1RepeatTag, mParams.i4IsoSpeedMode, u1IspProfile, mParams.i4RawType, mParams.i4DisableP1,
                        mParams.u1IsSingleFrameHDR, u1MtkHdrMode, mParams.i4MWBColorTemperature, mParams.bDummyAfterCapture);

    if(!QUERY_ENTRY_SINGLE(_halmeta, MTK_HAL_REQUEST_PRECAPTURE_START, mParams.u1PreCapStart))
        mParams.u1PreCapStart = 0;
    if(!QUERY_ENTRY_SINGLE(_halmeta, MTK_HAL_REQUEST_AF_TRIGGER_START, mAfParams.u1AfTrigStart))
        mAfParams.u1AfTrigStart = 0;

    MY_LOGD_IF(fgLogEn0,"[%s] MTK_HAL_REQUEST_PRECAPTURE_START(%d), MTK_HAL_REQUEST_AF_TRIGGER_START(%d), AfPause(%d)",
                        __FUNCTION__, mParams.u1PreCapStart, mAfParams.u1AfTrigStart, mAfParams.u1AfPause);

    MUINT8 u1ZSDCaptureIntent = 0;
    QUERY_ENTRY_SINGLE(_halmeta, MTK_HAL_REQUEST_ZSD_CAPTURE_INTENT, u1ZSDCaptureIntent);

    mParams.u1ZSDCaptureIntent = u1ZSDCaptureIntent;
    mAfParams.u1ZSDCaptureIntent = u1ZSDCaptureIntent;

    // AE Auto HDR, 0:OFF 1:ON 2:AUTO
    switch(static_cast<HDRMode>(u1MtkHdrMode))
    {
        case HDRMode::VIDEO_ON:
             mParams.u1HdrMode = 1;
             break;
        case HDRMode::VIDEO_AUTO:
             mParams.u1HdrMode = 2;
             break;
        case HDRMode::ON:
        case HDRMode::OFF:
        case HDRMode::AUTO:
        default:
             mParams.u1HdrMode = 0;
             break;
    }

    if (QUERY_ENTRY_SINGLE(_halmeta, MTK_3A_PRV_CROP_REGION, rSclCropRect))
    {
        fgCrop = MTRUE;
        if((i4TgWidth != mi4PreTgWidth) || (i4TgHeight != mi4PreTgHeight)
            || (rSclCropRect.p.x != mPrvCropRegion.p.x)
            || (rSclCropRect.p.y != mPrvCropRegion.p.y)
            || (rSclCropRect.s.w != mPrvCropRegion.s.w)
            || (rSclCropRect.s.h != mPrvCropRegion.s.h))
        {
            mPrvCropRegion.p.x = rSclCropRect.p.x;
            mPrvCropRegion.p.y = rSclCropRect.p.y;
            mPrvCropRegion.s.w = rSclCropRect.s.w;
            mPrvCropRegion.s.h = rSclCropRect.s.h;

            // crop info for AE
            mParams.rScaleCropRect.i4Xoffset = rSclCropRect.p.x;
            mParams.rScaleCropRect.i4Yoffset = rSclCropRect.p.y;
            mParams.rScaleCropRect.i4Xwidth = rSclCropRect.s.w;
            mParams.rScaleCropRect.i4Yheight = rSclCropRect.s.h;

            // crop info for AF
            CameraArea_T& rArea = mAfParams.rScaleCropArea;
            rArea.i4Left   = rSclCropRect.p.x;
            rArea.i4Top    = rSclCropRect.p.y;
            rArea.i4Right  = rSclCropRect.p.x + rSclCropRect.s.w;
            rArea.i4Bottom = rSclCropRect.p.y + rSclCropRect.s.h;
            rArea = _transformArea(mi4SensorIdx,mi4SensorMode,rArea);
            rArea = _clipArea(i4TgWidth,i4TgHeight,rArea);

            MY_LOGD_IF(fgLogEn0, "[%s] AE SCL CROP(%d,%d,%d,%d) AF SCL CROP(%d,%d,%d,%d)",
                    __FUNCTION__, mParams.rScaleCropRect.i4Xoffset, mParams.rScaleCropRect.i4Yoffset, mParams.rScaleCropRect.i4Xwidth, mParams.rScaleCropRect.i4Yheight,
                    rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom);
            mi4PreTgWidth = i4TgWidth;
            mi4PreTgHeight = i4TgHeight;
        }
    }
    else
    {
        // 1. Pass1 and Pass2 MW send 3A preview crop region to meta with HAL1.
        // 2. Pass1 MW does not send 3A preview crop region to meta with HAL3.
        // 2. 3A HAL need get 3A preview crop region from ISP Pass2.
        P2Info_T rP2Info;
        mpResultPoolObj->getP2Info(rP2Info);

        mPrvCropRegion.p.x = rP2Info.rSclCropRectl.p.x;
        mPrvCropRegion.p.y = rP2Info.rSclCropRectl.p.y;
        mPrvCropRegion.s.w = rP2Info.rSclCropRectl.s.w;
        mPrvCropRegion.s.h = rP2Info.rSclCropRectl.s.h;

        // crop info for AE
        mParams.rScaleCropRect.i4Xoffset = rP2Info.rSclCropRectl.p.x;
        mParams.rScaleCropRect.i4Yoffset = rP2Info.rSclCropRectl.p.y;
        mParams.rScaleCropRect.i4Xwidth  = rP2Info.rSclCropRectl.s.w;
        mParams.rScaleCropRect.i4Yheight = rP2Info.rSclCropRectl.s.h;

        // crop info for AF
        CameraArea_T& rArea = mAfParams.rScaleCropArea;

        rArea.i4Left   = mPrvCropRegion.p.x;
        rArea.i4Top    = mPrvCropRegion.p.y;
        rArea.i4Right  = mPrvCropRegion.p.x + mPrvCropRegion.s.w;
        rArea.i4Bottom = mPrvCropRegion.p.y + mPrvCropRegion.s.h;
        rArea = _transformArea(mi4SensorIdx, mi4SensorMode, rArea);
        rArea = _clipArea(i4TgWidth, i4TgHeight, rArea);

        MY_LOGD_IF(fgLogEn0, "[%s] Pass2 info AE SCL CROP(%d,%d,%d,%d) AF SCL CROP(%d,%d,%d,%d)",
                __FUNCTION__, mParams.rScaleCropRect.i4Xoffset, mParams.rScaleCropRect.i4Yoffset, mParams.rScaleCropRect.i4Xwidth, mParams.rScaleCropRect.i4Yheight,
                rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom);
    }

    if(!u1RepeatTag)
    {
        mi4AppfgCrop = QUERY_ENTRY_SINGLE(_appmeta, MTK_SCALER_CROP_REGION, rSclCropRect);
        fgCrop = mi4AppfgCrop;
        mAppCropRegion.p.x = rSclCropRect.p.x;
        mAppCropRegion.p.y = rSclCropRect.p.y;
        mAppCropRegion.s.w = rSclCropRect.s.w;
        mAppCropRegion.s.h = rSclCropRect.s.h;
        rSclCrop[0] = rSclCropRect.p.x;
        rSclCrop[1] = rSclCropRect.p.y;
        rSclCrop[2] = rSclCropRect.p.x + rSclCropRect.s.w;
        rSclCrop[3] = rSclCropRect.p.y + rSclCropRect.s.h;
        MY_LOGD_IF(fgLogEn0, "[%s] SCL CROP(%d,%d,%d,%d)", __FUNCTION__, rSclCrop[0], rSclCrop[1], rSclCrop[2], rSclCrop[3]);
    }
    if(mi4AppfgCrop)
    {
        MINT32 i4ZoomRatio = 0;
        if( QUERY_ENTRY_SINGLE(_halmeta, MTK_DUALZOOM_ZOOMRATIO, i4ZoomRatio))
        {
            MY_LOGD_IF(fgLogEn0, "i4ZoomRatio(%d) from AP, SensorDev(%d)", i4ZoomRatio, mu4SensorDev);
        } else {
            MRect ActiveArrayCropRegionSize;
            if(QUERY_ENTRY_SINGLE(_halmeta, MTK_SENSOR_MODE_INFO_ACTIVE_ARRAY_CROP_REGION, ActiveArrayCropRegionSize))
            {
                MY_LOGD_IF(fgLogEn0, "[%s] ActiveCropRgn(%d,%d,%d,%d) PrvCropRegion(%d,%d,%d,%d)", __FUNCTION__, ActiveArrayCropRegionSize.p.x, ActiveArrayCropRegionSize.p.y, ActiveArrayCropRegionSize.s.w, ActiveArrayCropRegionSize.s.h,
                                     mPrvCropRegion.p.x, mPrvCropRegion.p.y, mPrvCropRegion.s.w, mPrvCropRegion.s.h);
                CameraArea_T ActiveArrayCropRegionSize2TG;
                ActiveArrayCropRegionSize2TG.i4Left   = ActiveArrayCropRegionSize.p.x;
                ActiveArrayCropRegionSize2TG.i4Top    = ActiveArrayCropRegionSize.p.y;
                ActiveArrayCropRegionSize2TG.i4Right  = ActiveArrayCropRegionSize.p.x + ActiveArrayCropRegionSize.s.w;
                ActiveArrayCropRegionSize2TG.i4Bottom = ActiveArrayCropRegionSize.p.y + ActiveArrayCropRegionSize.s.h;
                ActiveArrayCropRegionSize2TG = _transformArea(mi4SensorIdx, mi4SensorMode, ActiveArrayCropRegionSize2TG);
                ActiveArrayCropRegionSize2TG = _clipArea(i4TgWidth, i4TgHeight, ActiveArrayCropRegionSize2TG);

                MY_LOGD_IF(fgLogEn0, "[%s] ActiveCropRgn2TG(%d,%d,%d,%d)", __FUNCTION__, ActiveArrayCropRegionSize2TG.i4Left, ActiveArrayCropRegionSize2TG.i4Top, ActiveArrayCropRegionSize2TG.i4Right, ActiveArrayCropRegionSize2TG.i4Bottom);

                MINT32 i4X = ActiveArrayCropRegionSize2TG.i4Left;
                MINT32 i4Y = ActiveArrayCropRegionSize2TG.i4Top;
                MINT32 i4W = ActiveArrayCropRegionSize2TG.i4Right - ActiveArrayCropRegionSize2TG.i4Left;
                MINT32 i4H = ActiveArrayCropRegionSize2TG.i4Bottom - ActiveArrayCropRegionSize2TG.i4Top;
                i4ZoomRatio = (((i4W*100)/mPrvCropRegion.s.w) < ((i4H*100)/mPrvCropRegion.s.h)) ? ((i4W*100)/mPrvCropRegion.s.w) : ((i4H*100)/mPrvCropRegion.s.h);

                MY_LOGD_IF(fgLogEn0, "[%s] i4ZoomRatio(%d)", __FUNCTION__, i4ZoomRatio);
            }
            else
            {
                MY_LOGD_IF(fgLogEn0, "[%s] ActiveRgn(%d,%d,%d,%d) PrvCropRegion(%d,%d,%d,%d)", __FUNCTION__, mActiveArraySize.p.x, mActiveArraySize.p.y, mActiveArraySize.s.w, mActiveArraySize.s.h,
                                     mPrvCropRegion.p.x, mPrvCropRegion.p.y, mPrvCropRegion.s.w, mPrvCropRegion.s.h);
                CameraArea_T ActiveArraySize2TG;
                ActiveArraySize2TG.i4Left   = mActiveArraySize.p.x;
                ActiveArraySize2TG.i4Top    = mActiveArraySize.p.y;
                ActiveArraySize2TG.i4Right  = mActiveArraySize.p.x + mActiveArraySize.s.w;
                ActiveArraySize2TG.i4Bottom = mActiveArraySize.p.y + mActiveArraySize.s.h;
                ActiveArraySize2TG = _transformArea(mi4SensorIdx, mi4SensorMode, ActiveArraySize2TG);
                ActiveArraySize2TG = _clipArea(i4TgWidth, i4TgHeight, ActiveArraySize2TG);

                MY_LOGD_IF(fgLogEn0, "[%s] ActiveRgn2TG(%d,%d,%d,%d)", __FUNCTION__, ActiveArraySize2TG.i4Left, ActiveArraySize2TG.i4Top, ActiveArraySize2TG.i4Right, ActiveArraySize2TG.i4Bottom);

                MINT32 i4X = ActiveArraySize2TG.i4Left;
                MINT32 i4Y = ActiveArraySize2TG.i4Top;
                MINT32 i4W = ActiveArraySize2TG.i4Right - ActiveArraySize2TG.i4Left;
                MINT32 i4H = ActiveArraySize2TG.i4Bottom - ActiveArraySize2TG.i4Top;
                i4ZoomRatio = (((i4W*100)/mPrvCropRegion.s.w) < ((i4H*100)/mPrvCropRegion.s.h)) ? ((i4W*100)/mPrvCropRegion.s.w) : ((i4H*100)/mPrvCropRegion.s.h);

                MY_LOGD_IF(fgLogEn0, "[%s] i4ZoomRatio(%d)", __FUNCTION__, i4ZoomRatio);

            }
            MY_LOGD_IF(fgLogEn0, "i4ZoomRatio(%d) SensorDev(%d)", i4ZoomRatio, mu4SensorDev);
        }
        mParams.i4ZoomRatio = i4ZoomRatio;
    }

    // dummy request
    HAL3A_SETBIT(mParams.u4HalFlag, HAL_FLG_DUMMY2, (it->Dummy ? HAL_FLG_DUMMY2 : 0));

#if CAM3_STEREO_FEATURE_EN

    MINT32 i4DenoiseMode = 0;
    MINT32 i4Sync2AMode = 0;
    MINT32 i4SyncAFMode = 0;
    MINT32 i4HwSyncMode = 0;
    MBOOL  bIsByFrame = MFALSE;
    MBOOL  bIsDummyFrame = MFALSE;
    MINT32 i4Sync3ANOTIFY = 0;
    MINT32 i4Sync3ASwitchOn = NS3Av3::E_SYNC3A_NOTIFY::E_SYNC3A_NOTIFY_NONE;

    QUERY_ENTRY_SINGLE(_halmeta, MTK_STEREO_FEATURE_DENOISE_MODE, i4DenoiseMode);
    QUERY_ENTRY_SINGLE(_halmeta, MTK_STEREO_SYNC2A_MODE, i4Sync2AMode);
    QUERY_ENTRY_SINGLE(_halmeta, MTK_STEREO_SYNCAF_MODE, i4SyncAFMode);
    QUERY_ENTRY_SINGLE(_halmeta, MTK_STEREO_HW_FRM_SYNC_MODE, i4HwSyncMode);
    QUERY_ENTRY_SINGLE(_halmeta, MTK_STEREO_NOTIFY, i4Sync3ANOTIFY);
    i4Sync3ASwitchOn = ((i4Sync3ANOTIFY >> NS3Av3::E_SYNC3A_NOTIFY::E_SYNC3A_NOTIFY_SWITCH_ON) & 0x1);
    mStereoParam.i4ZoomRatio = mParams.i4ZoomRatio;
    mStereoParam.i4FrameNum = it->FrameNum;

    MINT32 StereoData[2];
    if(GET_ENTRY_ARRAY(_halmeta, MTK_STEREO_SYNC2A_MASTER_SLAVE, StereoData, 2))
    {
        mStereoParam.i4MasterIdx = StereoData[0];
        mStereoParam.i4SlaveIdx = StereoData[1];
    }

    MINT32 i4LogEnable = ( (mParams.i4DenoiseMode != i4DenoiseMode)||(mStereoParam.i4Sync2AMode != i4Sync2AMode)||(mStereoParam.i4SyncAFMode != i4SyncAFMode)||(mStereoParam.i4HwSyncMode != i4HwSyncMode)||(mStereoParam.i4Sync3ASwitchOn != i4Sync3ASwitchOn) );

    MY_LOGD_IF( (i4LogEnable || fgLog), "[%s] Dev(%d) PreSetKey(#%d) FrameNum(%d), StereoParams : DenoiseMode(%d -> %d), Sync2A(%d -> %d), SyncAF(%d -> %d), HwSync(%d -> %d), M&S Idx(%d,%d), bIsByFrame(%d -> %d), Sync3A(Notify,SwitchOn)(%d,%d -> %d)", __FUNCTION__,
        mu4SensorDev, it->PreSetKey, mStereoParam.i4FrameNum,
        mParams.i4DenoiseMode, i4DenoiseMode,
        mStereoParam.i4Sync2AMode, i4Sync2AMode,
        mStereoParam.i4SyncAFMode, i4SyncAFMode,
        mStereoParam.i4HwSyncMode, i4HwSyncMode,
        mStereoParam.i4MasterIdx,
        mStereoParam.i4SlaveIdx,
        mStereoParam.bIsByFrame, bIsByFrame,
        i4Sync3ANOTIFY,mStereoParam.i4Sync3ASwitchOn, i4Sync3ASwitchOn);

    if(i4Sync2AMode == NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_VSDOF_BY_FRAME ||
       i4Sync2AMode == NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_DENOISE_BY_FRAME ||
       i4Sync2AMode == NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_DUAL_ZOOM_BY_FRAME)
    {
        i4Sync2AMode = i4Sync2AMode - 3; // remapping to E_SYNC2A_MODE_VSDOF, E_SYNC2A_MODE_DENOISE, E_SYNC2A_MODE_DUAL_ZOOM
        bIsByFrame = MTRUE;
    }
    else if(i4Sync2AMode == NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_IDLE)
        bIsByFrame = mStereoParam.bIsByFrame;

    mParams.i4DenoiseMode = i4DenoiseMode;
    mStereoParam.i4Sync2AMode = i4Sync2AMode;
    mStereoParam.i4SyncAFMode = i4SyncAFMode;
    mStereoParam.i4HwSyncMode = i4HwSyncMode;
    mStereoParam.bIsByFrame = bIsByFrame;
    mStereoParam.bIsDummyFrame = bIsDummyFrame;
    mStereoParam.i4Sync3ASwitchOn = i4Sync3ASwitchOn;

    if (i4Sync3ASwitchOn == NS3Av3::E_SYNC3A_NOTIFY::E_SYNC3A_NOTIFY_SWITCH_ON)
    {
        mStereoParam.i4Sync2AMode = NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_IDLE;
        mStereoParam.i4SyncAFMode = NS3Av3::E_SYNCAF_MODE::E_SYNCAF_MODE_IDLE;
        mStereoParam.i4HwSyncMode = NS3Av3::E_HW_FRM_SYNC_MODE::E_HW_FRM_SYNC_MODE_IDLE;
    }

#endif

    MY_LOGD_IF(fgLog,
        "[%s] dummy(%d), hqc(%d), exif(%d), SCL CROP(%d,%d,%d,%d), AF SCL CROP(%d,%d,%d,%d)",
        __FUNCTION__, it->Dummy, mParams.u1HQCapEnable, mParams.u1IsGetExif,
        rSclCrop[0], rSclCrop[1], rSclCrop[2], rSclCrop[3],
        mAfParams.rScaleCropArea.i4Left, mAfParams.rScaleCropArea.i4Top, mAfParams.rScaleCropArea.i4Right, mAfParams.rScaleCropArea.i4Bottom);

    AAA_TRACE_END_HAL;

    if (!u1RepeatTag) // not repeating tag, parse app meta
    {
        AAA_TRACE_HAL(ParseAppMeta);
        mUpdateMetaResult.clear();
        for (MINT32 j = 0; j < _appmeta.count(); j++)
        {
            IMetadata::IEntry entry = _appmeta.entryAt(j);
            mtk_camera_metadata_tag_t tag = (mtk_camera_metadata_tag_t)entry.tag();

            // convert metadata tag into 3A settings.
            switch (tag)
            {
            case MTK_CONTROL_MODE:  // dynamic
                {
                    MUINT8 u1ControlMode = entry.itemAt(0, Type2Type< MUINT8 >());
                    MY_LOGD_IF(mParams.u1ControlMode != u1ControlMode , "[%s] MTK_CONTROL_MODE(%d -> %d)", __FUNCTION__, mParams.u1ControlMode, u1ControlMode);
                    mParams.u1ControlMode = u1ControlMode;
                }
                break;
            case MTK_CONTROL_CAPTURE_INTENT:
                {
                    MUINT8 u1CapIntent = entry.itemAt(0, Type2Type< MUINT8 >());
                    MY_LOGD_IF(mParams.u1CaptureIntent != u1CapIntent, "[%s] MTK_CONTROL_CAPTURE_INTENT(%d -> %d)", __FUNCTION__, mParams.u1CaptureIntent, u1CapIntent);
                    mParams.u1CaptureIntent = u1CapIntent;
                    mAfParams.u1CaptureIntent = u1CapIntent;
                }
                break;

            // AWB
            case MTK_CONTROL_AWB_LOCK:
                {
                    MUINT8 bIsAWBLock = entry.itemAt(0, Type2Type< MUINT8 >());
                    MY_LOGD_IF(mParams.bIsAWBLock != bIsAWBLock, "[%s] MTK_CONTROL_AWB_LOCK(%d -> %d)", __FUNCTION__, mParams.bIsAWBLock, bIsAWBLock);
                    mParams.bIsAWBLock = bIsAWBLock;
                }
                break;
            case MTK_CONTROL_AWB_MODE:  // dynamic
                {
                    MUINT8 u1Mode = entry.itemAt(0, Type2Type< MUINT8 >());
                    MY_LOGD_IF(mParams.u4AwbMode != u1Mode, "[%s] MTK_CONTROL_AWB_MODE(%d -> %d)", __FUNCTION__, mParams.u4AwbMode, u1Mode);
                    mParams.u4AwbMode = u1Mode;
                    //Enabling this disables control.aeMode, control.awbMode and control.afMode controls;
                    //the camera device will ignore those settings while USE_SCENE_MODE is active.
                    if(mbEnableOverride)
                        u1Override3A &= (~OVERRIDE_AWB);
                }
                break;
            case MTK_CONTROL_AWB_REGIONS:
                {
                    if (m_rStaticInfo.i4MaxRegionAwb != 0)
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
                                rArea.i4Left   = MIN(MAX(rArea.i4Left, rSclCrop[0]), rSclCrop[2]);
                                rArea.i4Top    = MIN(MAX(rArea.i4Top, rSclCrop[1]), rSclCrop[3]);
                                rArea.i4Right  = MAX(MIN(rArea.i4Right, rSclCrop[2]), rSclCrop[0]);
                                rArea.i4Bottom = MAX(MIN(rArea.i4Bottom, rSclCrop[3]), rSclCrop[1]);
                            }
                            MY_LOGD("[%s] Return to AP AWB_REGIONS L(%d) T(%d) R(%d) B(%d) W(%d)", __FUNCTION__, rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom, rArea.i4Weight);

                            entryNew.push_back(rArea.i4Left,   Type2Type<MINT32>());
                            entryNew.push_back(rArea.i4Top,    Type2Type<MINT32>());
                            entryNew.push_back(rArea.i4Right,  Type2Type<MINT32>());
                            entryNew.push_back(rArea.i4Bottom, Type2Type<MINT32>());
                            entryNew.push_back(rArea.i4Weight, Type2Type<MINT32>());
                            rArea = _transformArea(mi4SensorIdx,mi4SensorMode,rArea);
                            rArea = _clipArea(i4TgWidth,i4TgHeight,rArea);
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AWB_REGIONS mod L(%d) T(%d) R(%d) B(%d) W(%d)", __FUNCTION__, rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom, rArea.i4Weight);
                        }
                        mUpdateMetaResult.push_back({MTK_CONTROL_AWB_REGIONS, entryNew});
                    }
                }
                break;

            // AE
            case MTK_CONTROL_AE_ANTIBANDING_MODE:
                {
                    MINT32 i4Mode = entry.itemAt(0, Type2Type< MUINT8 >());
                    MY_LOGD_IF(mParams.u4AntiBandingMode != i4Mode, "[%s] MTK_CONTROL_AE_ANTIBANDING_MODE(%d -> %d)", __FUNCTION__, mParams.u4AntiBandingMode, i4Mode);
                    mParams.u4AntiBandingMode = i4Mode;
                }
                break;
            case MTK_CONTROL_AE_EXPOSURE_COMPENSATION:
                {
                    MINT32 i4ExpIdx = entry.itemAt(0, Type2Type< MINT32 >());
                    MY_LOGD_IF(mParams.i4ExpIndex != i4ExpIdx, "[%s] MTK_CONTROL_AE_EXPOSURE_COMPENSATION(%d -> %d)", __FUNCTION__, mParams.i4ExpIndex, i4ExpIdx);
                    mParams.i4ExpIndex = i4ExpIdx;
                }
                break;
            case MTK_CONTROL_AE_LOCK:
                {
                    MUINT8 u1bLock = entry.itemAt(0, Type2Type< MUINT8 >());
                    MY_LOGD_IF(mParams.bIsAELock != u1bLock, "[%s] MTK_CONTROL_AE_LOCK(%d -> %d)", __FUNCTION__, mParams.bIsAELock, u1bLock);
                    mParams.bIsAELock = u1bLock;
                }
                break;
            case MTK_CONTROL_AE_MODE:
                {
                    MUINT8 u1Mode = entry.itemAt(0, Type2Type< MUINT8 >());
                    MY_LOGD_IF(mParams.u4AeMode != u1Mode, "[%s] MTK_CONTROL_AE_MODE(%d -> %d)", __FUNCTION__, mParams.u4AeMode, u1Mode);
                    mParams.u4AeMode = u1Mode;
                    if(mbEnableOverride)
                        u1Override3A &= (~OVERRIDE_AE);
                }
                break;
            case MTK_CONTROL_AE_REGIONS:    // dynamic
                {
                    if (m_rStaticInfo.i4MaxRegionAe != 0)
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
                                rArea.i4Left   = MIN(MAX(rArea.i4Left, rSclCrop[0]), rSclCrop[2]);
                                rArea.i4Top    = MIN(MAX(rArea.i4Top, rSclCrop[1]), rSclCrop[3]);
                                rArea.i4Right  = MAX(MIN(rArea.i4Right, rSclCrop[2]), rSclCrop[0]);
                                rArea.i4Bottom = MAX(MIN(rArea.i4Bottom, rSclCrop[3]), rSclCrop[1]);
                            }
                            MY_LOGD("[%s] Return to AP AE_REGIONS L(%d) T(%d) R(%d) B(%d) W(%d)", __FUNCTION__, rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom, rArea.i4Weight);

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
                        mUpdateMetaResult.push_back({MTK_CONTROL_AE_REGIONS, entryNew});
                    }
                }
                break;
            case MTK_CONTROL_AE_TARGET_FPS_RANGE:
                {
                    mParams.i4MinFps = 1000 * entry.itemAt(0, Type2Type< MINT32 >());
                    mParams.i4MaxFps = 1000 * entry.itemAt(1, Type2Type< MINT32 >());
                    MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AE_TARGET_FPS_RANGE(%d, %d)", __FUNCTION__, mParams.i4MinFps/1000, mParams.i4MaxFps/1000);
                }
                break;
            case MTK_CONTROL_AE_PRECAPTURE_TRIGGER:
                {
                    //MUINT8 u1AePrecapTrig = MTK_CONTROL_AE_PRECAPTURE_TRIGGER_IDLE;
                    MUINT8 u1AePrecapTrig = entry.itemAt(0, Type2Type< MUINT8 >());
                    MY_LOGD_IF(mParams.u1PrecapTrig != u1AePrecapTrig, "[%s] MTK_CONTROL_AE_PRECAPTURE_TRIGGER(%d -> %d)", __FUNCTION__, mParams.u1PrecapTrig, u1AePrecapTrig);
                    mParams.u1PrecapTrig = u1AePrecapTrig;
                    mAfParams.u1PrecapTrig = u1AePrecapTrig;
                }
                break;
            case MTK_CONTROL_AE_PRECAPTURE_ID:
                {
                    MINT32 i4AePreCapId = 0;
                    i4AePreCapId = entry.itemAt(0, Type2Type< MINT32 >());
                    MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AE_PRECAPTURE_ID(%d)", __FUNCTION__, i4AePreCapId);
                }
                break;

            case MTK_FLASH_MODE:
                {
                    MUINT8 u1Mode = entry.itemAt(0, Type2Type< MUINT8 >());
                    MY_LOGD_IF(mParams.u4StrobeMode != u1Mode, "[%s] MTK_FLASH_MODE(%d -> %d)", __FUNCTION__, mParams.u4StrobeMode, u1Mode);
                    mParams.u4StrobeMode = u1Mode;
                }
                break;

            // Sensor
            case MTK_SENSOR_EXPOSURE_TIME:
                {
                    MINT64 i8AeExposureTime = entry.itemAt(0, Type2Type< MINT64 >());
                    MY_LOGD_IF(mParams.i8ExposureTime != i8AeExposureTime, "[%s] MTK_SENSOR_EXPOSURE_TIME(%lld -> %lld)", __FUNCTION__, (long long)mParams.i8ExposureTime, (long long)i8AeExposureTime);
                    mParams.i8ExposureTime = i8AeExposureTime;
                }
                break;
            case MTK_SENSOR_SENSITIVITY:
                {
                    MINT32 i4AeSensitivity = entry.itemAt(0, Type2Type< MINT32 >());
                    MY_LOGD_IF(mParams.i4Sensitivity != i4AeSensitivity, "[%s] MTK_SENSOR_SENSITIVITY(%d -> %d)", __FUNCTION__, mParams.i4Sensitivity, i4AeSensitivity);
                    mParams.i4Sensitivity = i4AeSensitivity;
                }
                break;
            case MTK_SENSOR_FRAME_DURATION:
                {
                    MINT64 i8FrameDuration = entry.itemAt(0, Type2Type< MINT64 >());
                    MY_LOGD_IF(mParams.i8FrameDuration != i8FrameDuration, "[%s] MTK_SENSOR_FRAME_DURATION(%lld -> %lld)", __FUNCTION__, (long long)mParams.i8FrameDuration, (long long)i8FrameDuration);
                    mParams.i8FrameDuration = i8FrameDuration;
                }
                break;
            case MTK_BLACK_LEVEL_LOCK:
                {
                    MUINT8 u1BlackLvlLock = entry.itemAt(0, Type2Type< MUINT8 >());
                    MY_LOGD_IF(mParams.u1BlackLvlLock != u1BlackLvlLock, "[%s] MTK_BLACK_LEVEL_LOCK(%d -> %d)", __FUNCTION__, mParams.u1BlackLvlLock, u1BlackLvlLock);
                    mParams.u1BlackLvlLock = u1BlackLvlLock;
                }
                break;

            // AF
            case MTK_CONTROL_AF_MODE:
                {
                    MUINT8 u1AfMode = entry.itemAt(0, Type2Type< MUINT8 >());
                    MY_LOGD_IF(mAfParams.u4AfMode != u1AfMode, "[%s] MTK_CONTROL_AF_MODE(%d -> %d)", __FUNCTION__, mAfParams.u4AfMode, u1AfMode);
                    mAfParams.u4AfMode = u1AfMode; //_convertAFMode(u1AfMode);
                    if(mbEnableOverride)
                        u1Override3A &= (~OVERRIDE_AF);
                }
                break;
            case MTK_CONTROL_AF_REGIONS:    // dynamic
                {
                    if (m_rStaticInfo.i4MaxRegionAf != 0)
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
                                rArea.i4Left   = MIN(MAX(rArea.i4Left, rSclCrop[0]), rSclCrop[2]);
                                rArea.i4Top    = MIN(MAX(rArea.i4Top, rSclCrop[1]), rSclCrop[3]);
                                rArea.i4Right  = MAX(MIN(rArea.i4Right, rSclCrop[2]), rSclCrop[0]);
                                rArea.i4Bottom = MAX(MIN(rArea.i4Bottom, rSclCrop[3]), rSclCrop[1]);
                            }
                            MY_LOGD("[%s] Return to AP AF_REGIONS L(%d) T(%d) R(%d) B(%d) W(%d)", __FUNCTION__, rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom, rArea.i4Weight);

                            entryNew.push_back(rArea.i4Left,   Type2Type<MINT32>());
                            entryNew.push_back(rArea.i4Top,    Type2Type<MINT32>());
                            entryNew.push_back(rArea.i4Right,  Type2Type<MINT32>());
                            entryNew.push_back(rArea.i4Bottom, Type2Type<MINT32>());
                            entryNew.push_back(rArea.i4Weight, Type2Type<MINT32>());
                            rArea = _transformArea(mi4SensorIdx,mi4SensorMode,rArea);
                            rArea = _clipArea(i4TgWidth,i4TgHeight,rArea);
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AF_REGIONS mod L(%d) T(%d) R(%d) B(%d) W(%d)", __FUNCTION__, rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom, rArea.i4Weight);
                        }
                        mUpdateMetaResult.push_back({MTK_CONTROL_AF_REGIONS, entryNew});
                    }
                }
                break;
            case MTK_CONTROL_AF_TRIGGER:
                {
                    MUINT8 AFTrigger = entry.itemAt(0, Type2Type< MUINT8 >());
                    MY_LOGD_IF(mAfParams.u1AfTrig != AFTrigger, "[%s] MTK_CONTROL_AF_TRIGGER(%d -> %d)", __FUNCTION__, mAfParams.u1AfTrig, AFTrigger);
                    mAfParams.u1AfTrig = AFTrigger;
                }
                break;
            case MTK_CONTROL_AF_TRIGGER_ID:
                {
                    MINT32 i4AfTrigId = entry.itemAt(0, Type2Type< MINT32 >());
                    MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AF_TRIGGER_ID(%d)", __FUNCTION__, i4AfTrigId);
                }
                break;

            // Lens
            case MTK_LENS_FOCUS_DISTANCE:
                {
                    MFLOAT fFocusDist = entry.itemAt(0, Type2Type< MFLOAT >());
                    MY_LOGD_IF(mAfParams.fFocusDistance != fFocusDist, "[%s] MTK_LENS_FOCUS_DISTANCE(%3.6f -> %3.6f)", __FUNCTION__, mAfParams.fFocusDistance, fFocusDist);
                    mAfParams.fFocusDistance = fFocusDist;
                }
                break;

            // ISP
            case MTK_CONTROL_EFFECT_MODE:
                {
                    MUINT8 u1EffectMode = entry.itemAt(0, Type2Type< MUINT8 >());
                    MY_LOGD_IF(mParams.u4EffectMode != u1EffectMode, "[%s] MTK_CONTROL_EFFECT_MODE(%d -> %d)", __FUNCTION__, mParams.u4EffectMode, u1EffectMode);
                    mParams.u4EffectMode = u1EffectMode;
                }
                break;
            case MTK_CONTROL_SCENE_MODE:
                {
                    MUINT32 u4SceneMode = entry.itemAt(0, Type2Type< MUINT8 >());
                    MY_LOGD_IF(mParams.u4SceneMode != u4SceneMode, "[%s] MTK_CONTROL_SCENE_MODE(%d -> %d)", __FUNCTION__, mParams.u4SceneMode, u4SceneMode);
                    mParams.u4SceneMode = u4SceneMode;
                }
                break;
            case MTK_EDGE_MODE:
                {
                    MUINT8 u1EdgeMode = entry.itemAt(0, Type2Type< MUINT8 >());
                    MY_LOGD_IF(mParams.u1EdgeMode != u1EdgeMode, "[%s] MTK_EDGE_MODE(%d -> %d)", __FUNCTION__, mParams.u1EdgeMode, u1EdgeMode);
                    mParams.u1EdgeMode = u1EdgeMode;
                }
                break;
            case MTK_NOISE_REDUCTION_MODE:
                {
                    MUINT8 u1NRMode = entry.itemAt(0, Type2Type< MUINT8 >());
                    MY_LOGD_IF(mParams.u1NRMode != u1NRMode, "[%s] MTK_NOISE_REDUCTION_MODE(%d -> %d)", __FUNCTION__, mParams.u1NRMode, u1NRMode);
                    mParams.u1NRMode = u1NRMode;
                }
                break;

            // Color correction
            case MTK_COLOR_CORRECTION_MODE:
                {
                    MUINT8 u1ColorCrctMode = entry.itemAt(0, Type2Type<MUINT8>());
                    MY_LOGD_IF(mParams.u1ColorCorrectMode != u1ColorCrctMode, "[%s] MTK_COLOR_CORRECTION_MODE(%d)", __FUNCTION__, mParams.u1ColorCorrectMode, u1ColorCrctMode);
                    mParams.u1ColorCorrectMode = u1ColorCrctMode;
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
                        MRational rMat = entry.itemAt(k, Type2Type<MRational>());
                        mParams.fColorCorrectMat[k] = (0.0f != rMat.denominator) ? (MFLOAT)rMat.numerator / rMat.denominator : 0.0f;
                        MY_LOGD_IF(fgLogEn1, "[%s] MTK_COLOR_CORRECTION_TRANSFORM rMat[%d]=(%d/%d)", __FUNCTION__, k, rMat.numerator, rMat.denominator);
                    }
                }
                break;
            case MTK_COLOR_CORRECTION_ABERRATION_MODE:
                {
                    MUINT8 u1ColorAberrationMode = entry.itemAt(0, Type2Type<MUINT8>());
                    MY_LOGD_IF(fgLogEn1, "[%s] MTK_COLOR_CORRECTION_ABERRATION_MODE(%d)", __FUNCTION__, u1ColorAberrationMode);
                }
                break;
            case MTK_HOT_PIXEL_MODE:
                {
                    MUINT8 u1HotPixelMode = entry.itemAt(0, Type2Type<MUINT8>());
                    MY_LOGD_IF(fgLogEn1, "[%s] MTK_HOT_PIXEL_MODE(%d)", __FUNCTION__, u1HotPixelMode);
                }
                break;
            case MTK_STATISTICS_HOT_PIXEL_MAP_MODE:
                {
                    MUINT8 u1HotPixelMap = entry.itemAt(0, Type2Type<MUINT8>());
                    MY_LOGD_IF(fgLogEn1, "[%s] MTK_STATISTICS_HOT_PIXEL_MAP_MODE(%d)", __FUNCTION__, u1HotPixelMap);
                }
                break;
            // Shading
            case MTK_SHADING_MODE:
                {
                    MUINT8 u1ShadingMode = entry.itemAt(0, Type2Type<MUINT8>());
                    MY_LOGD_IF(mParams.u1ShadingMode != u1ShadingMode, "[%s] MTK_SHADING_MODE(%d -> %d)", __FUNCTION__, mParams.u1ShadingMode, u1ShadingMode);
                    mParams.u1ShadingMode = u1ShadingMode;
                }
                break;
            case MTK_STATISTICS_LENS_SHADING_MAP_MODE:
                {
                    MUINT8 u1ShadingMapMode = entry.itemAt(0, Type2Type<MUINT8>());
                    MY_LOGD_IF(mParams.u1ShadingMapMode != u1ShadingMapMode, "[%s] MTK_STATISTICS_LENS_SHADING_MAP_MODE(%d)", __FUNCTION__, mParams.u1ShadingMapMode, u1ShadingMapMode);
                    mParams.u1ShadingMapMode = u1ShadingMapMode;
                }
                break;
            case MTK_FACE_FEATURE_FORCE_FACE_3A:
                {
                    MINT32 i4ForceFace3A = entry.itemAt(0, Type2Type<MINT32>());
                    MY_LOGD_IF(mParams.i4ForceFace3A != i4ForceFace3A, "[%s] MTK_FACE_FEATURE_FORCE_FACE_3A(%d -> %d)", __FUNCTION__, mParams.i4ForceFace3A, i4ForceFace3A);
                    mParams.i4ForceFace3A = i4ForceFace3A;
                }
                break;
            case MTK_STATISTICS_FACE_DETECT_MODE:
                {
                    MUINT8 u1FaceDetectMode = entry.itemAt(0, Type2Type<MUINT8>());
                    MY_LOGD_IF(mParams.u1FaceDetectMode != u1FaceDetectMode, "[%s] MTK_STATISTICS_FACE_DETECT_MODE(%d -> %d)", __FUNCTION__, mParams.u1FaceDetectMode, u1FaceDetectMode);
                    mParams.u1FaceDetectMode = u1FaceDetectMode;
                }
            default:
                break;
            }
        }
        AAA_TRACE_END_HAL;
    }
    else // repeat Tag
    {
        u1Override3A = 0;       // reduce AWB,AF,AE default value
    }

    AAA_TRACE_HAL(ControlMode);

    QUERY_ENTRY_SINGLE(_halmeta, MTK_3A_ISP_MDP_TARGET_SIZE, mParams.targetSize);
    MY_LOGD_IF(fgLog, "[%s] MTK_3A_ISP_MDP_TARGET_SIZE(%d, %d)", __FUNCTION__, mParams.targetSize.w, mParams.targetSize.h);

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
                if (!m_ScnModeOvrd.empty())
                {
                    Mode3A_T rOverride = m_ScnModeOvrd[(MUINT8)mParams.u4SceneMode];
                    mParams.u4AeMode  = (u1Override3A & OVERRIDE_AE) ? rOverride.u1AeMode : mParams.u4AeMode;
                    mParams.u4AwbMode = (u1Override3A & OVERRIDE_AWB) ? rOverride.u1AwbMode : mParams.u4AwbMode;
                    mAfParams.u4AfMode  = (u1Override3A & OVERRIDE_AF) ? rOverride.u1AfMode : mAfParams.u4AfMode; //_convertAFMode(rOverride.u1AfMode);
                    MY_LOGD_IF(fgLogEn2, "[%s] Scene mode(%d) overrides AE(%d), AWB(%d), AF(%d)", __FUNCTION__, mParams.u4SceneMode, mParams.u4AeMode, mParams.u4AwbMode, mAfParams.u4AfMode);
                }
                else
                {
                    MY_LOGE("[%s] Scene mode(%d) overrides Fails", __FUNCTION__, mParams.u4SceneMode);
                }
            }
            break;
        }
        break;
    default:
        break;
    }
    AAA_TRACE_END_HAL;

    AAA_TRACE_HAL(RemappingParam);
    // Restore m_Params Capture Commands
    if(u1RepeatTag)
    {
        if(mParams.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_STILL_CAPTURE)
            mParams.u1CaptureIntent = MTK_CONTROL_CAPTURE_INTENT_PREVIEW;
        else if(mParams.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_VIDEO_SNAPSHOT)
            mParams.u1CaptureIntent = MTK_CONTROL_CAPTURE_INTENT_VIDEO_RECORD;

        mParams.u1PrecapTrig        = MTK_CONTROL_AE_PRECAPTURE_TRIGGER_IDLE;
        mAfParams.u1PrecapTrig      = MTK_CONTROL_AE_PRECAPTURE_TRIGGER_IDLE;
        mAfParams.u1AfTrig          = MTK_CONTROL_AF_TRIGGER_IDLE;
    }

    if(mAfParams.u1AfTrigStart)
    {
        mAfParams.u1AfTrig = MTK_CONTROL_AF_TRIGGER_START;
    }
    if(mParams.u1PreCapStart)
    {
        mParams.u1PrecapTrig = MTK_CONTROL_AE_PRECAPTURE_TRIGGER_START;
        mAfParams.u1PrecapTrig = MTK_CONTROL_AE_PRECAPTURE_TRIGGER_START;
    }

    // check last and new AF trigger.
    // If set af trigger start continue, need to handle this case.
    if(mAfParams.u1AfTrig == MTK_CONTROL_AF_TRIGGER_START && m_u1LastAfTrig == MTK_CONTROL_AF_TRIGGER_START)
    {
        mAfParams.u1AfTrig = MTK_CONTROL_AF_TRIGGER_IDLE;
        CAM_LOGD("[%s] DEBUG : AfTrig=%d LastAfTrig=%d", __FUNCTION__, mAfParams.u1AfTrig, m_u1LastAfTrig);
    }

    m_u1LastAfTrig = mAfParams.u1AfTrig;

    // ISP profile
    if (u1IspProfile == 0xFF){
        switch (mParams.u1CaptureIntent)
        {
            case MTK_CONTROL_CAPTURE_INTENT_PREVIEW:
            case MTK_CONTROL_CAPTURE_INTENT_ZERO_SHUTTER_LAG:
                mParams.eIspProfile = NSIspTuning::EIspProfile_Preview;
                break;
            case MTK_CONTROL_CAPTURE_INTENT_VIDEO_RECORD:
            case MTK_CONTROL_CAPTURE_INTENT_VIDEO_SNAPSHOT:
                mParams.eIspProfile = NSIspTuning::EIspProfile_Video;
                break;
            default:
                mParams.eIspProfile = NSIspTuning::EIspProfile_Preview;
                break;
        }
    }
    else{

        NSIspTuning::EIspProfile_T eIspProfile = static_cast<NSIspTuning::EIspProfile_T>(u1IspProfile);
        MY_LOGD_IF(mParams.eIspProfile != eIspProfile, "[%s] eIspProfile %d -> %d", __FUNCTION__, mParams.eIspProfile, eIspProfile);
        mParams.eIspProfile = eIspProfile;
    }
    AAA_TRACE_END_HAL;

    if(m_u1IsFirstParseMeta == 0)
        m_u1IsFirstParseMeta++;

    return u1RepeatTag;
}

MINT32
Hal3AAdapter3::
setNormal(const vector<MetaSet_T*>& requestQ)
{
    AAA_TRACE_HAL(setNormal);

    MBOOL fgLog = mu4LogEn & HAL3AADAPTER3_LOG_PF;

    std::lock_guard<std::mutex> lock_meta(mLockResult);

    MY_LOGD_IF(fgLog, "[%s] with Presetkey %d %d, MagicNum %d, FrameNum %d, Dummy(%d)", __FUNCTION__,requestQ[0]->PreSetKey, m_i4Presetkey, requestQ[0]->MagicNum, requestQ[0]->FrameNum, requestQ[0]->Dummy);

    MetaSet_T* it = requestQ[0];
    // if (it->MagicNum = NaN, has to fetch MagicNum in _halmeta) ---- for acdk and previous platform
    MINT32 i4FrmId = it->MagicNum;
    MINT32 i4FrameNum = it->FrameNum;
    if (i4FrmId <= 0)   QUERY_ENTRY_SINGLE(it->halMeta, MTK_P1NODE_PROCESSOR_MAGICNUM, i4FrmId);

    // reset AE dummy setting
    mParams.i8ExposureTimeDummy = 0;
    mParams.i8FrameDurationDummy = 0;
    mParams.i4SensitivityDummy = 0;
    mParams.u1Dummy = it->Dummy;
    // if (Presetkey != Magic Num ) // Re-parse
    // Don't forget to judge (!Dummy && FrmId > 0) to do the rest
    if (it->PreSetKey != m_i4Presetkey && !it->Dummy && i4FrmId > 0)
        parseMeta(requestQ);
    else if (it->Dummy || mParams.bDummyBeforeCapture || mParams.bDummyAfterCapture)
    {
        /************************************************************************************************************************************
         ***************************************      Why this ?    *************************************************************************
         **        After startCapture is called, MW will continuously set Dummy request where Hal3A should reset the Capture intent        **
         ************************************************************************************************************************************/

        if(mParams.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_STILL_CAPTURE)
            mParams.u1CaptureIntent = MTK_CONTROL_CAPTURE_INTENT_PREVIEW;
        else if(mParams.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_VIDEO_SNAPSHOT)
            mParams.u1CaptureIntent = MTK_CONTROL_CAPTURE_INTENT_VIDEO_RECORD;

        mParams.u1PrecapTrig        = MTK_CONTROL_AE_PRECAPTURE_TRIGGER_IDLE;
        mAfParams.u1PrecapTrig      = MTK_CONTROL_AE_PRECAPTURE_TRIGGER_IDLE;
        mAfParams.u1AfTrig          = MTK_CONTROL_AF_TRIGGER_IDLE;
        mParams.rcapParams          = IMetadata::Memory();

        if(mParams.bDummyBeforeCapture)
        {
            mParams.u4AeMode = MTK_CONTROL_AE_MODE_OFF;
            mParams.u4StrobeMode = MTK_FLASH_MODE_OFF;
        }
        else if(mParams.bDummyAfterCapture)
        {
            mParams.u4StrobeMode = MTK_FLASH_MODE_OFF;
            mParams.i4ExpIndex = 0;
        }

        if(mParams.u4AeMode == MTK_CONTROL_AE_MODE_OFF)
        {
            mParams.i8ExposureTimeDummy  = 10000000;
            mParams.i8FrameDurationDummy = 10000000;
            mParams.i4SensitivityDummy = 100;
        }
#if CAM3_STEREO_FEATURE_EN
        if(it->Dummy)
        {
            if(mStereoParam.bIsByFrame)
                mStereoParam.bIsDummyFrame = MTRUE;
            mStereoParam.i4Sync2AMode = NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_IDLE;
            mStereoParam.i4SyncAFMode = NS3Av3::E_SYNCAF_MODE::E_SYNCAF_MODE_IDLE;
            mStereoParam.i4HwSyncMode = NS3Av3::E_HW_FRM_SYNC_MODE::E_HW_FRM_SYNC_MODE_IDLE;
            mStereoParam.i4Sync3ASwitchOn = NS3Av3::E_SYNC3A_NOTIFY::E_SYNC3A_NOTIFY_NONE;
        }
        MY_LOGD("[%s] Dev(%d) Req(#%d) FrameNum(#%d) Dummy(%d) Sync2AMode(%d) SyncAF(%d) HwSync(%d) Sync3ASwitchOn(%d)", __FUNCTION__, mu4SensorDev, i4FrmId, i4FrameNum, it->Dummy, mStereoParam.i4Sync2AMode, mStereoParam.i4SyncAFMode, mStereoParam.i4HwSyncMode, mStereoParam.i4Sync3ASwitchOn);
#endif
        MY_LOGD("[%s] Dev(%d) Req(#%d) FrameNum(#%d) Dummy(%d/%d/%d) CapInt(%d) AE(M:%d/E:%lld/F:%lld/S:%d) FLASH(M:%d)", __FUNCTION__, mu4SensorDev, i4FrmId, i4FrameNum,
            it->Dummy, mParams.bDummyBeforeCapture, mParams.bDummyAfterCapture, mParams.u1CaptureIntent,
            mParams.u4AeMode, (long long)mParams.i8ExposureTimeDummy, (long long)mParams.i8FrameDurationDummy, mParams.i4SensitivityDummy,
            mParams.u4StrobeMode);
    }

    // Store in ResultPool
    mParams.i4MagicNum = i4FrmId;
    mParams.i4FrameNum = i4FrameNum;
    mAfParams.i4MagicNum = i4FrmId;

    // ResultPool - 1. Clear the new ResultPool of request magic and update request magic to history vector
    MY_LOGD_IF(mu4LogEn & HAL3AADAPTER3_LOG_SET_0, "[%s] Req(#%d) updateHistory/getValidateMetadata/clearOldestResultPool", __FUNCTION__, i4FrmId);
    mpResultPoolObj->updateHistory(i4FrmId, HistorySize);
    // ResultPool - 2. Get validate buffer by request magic
    MBOOL ret = mpResultPoolObj->findValidateBuffer(i4FrmId);

    // Result - 3. Update config information
    RESULT_CFG_T rResultCfg;
    rResultCfg.i4ReqMagic = i4FrmId;
    rResultCfg.i4PreSetKey = m_i4Presetkey;
    ret = mpResultPoolObj->updateResultCFG(rResultCfg);

    // Result - 4. Assign metadata data
    AllMetaResult_T* pMetadata = mpResultPoolObj->getMetadata(i4FrmId);
    if(pMetadata != NULL)
    {
        std::lock_guard<std::mutex> lock(pMetadata->LockMetaResult);
        MetaSet_T* prMetaResult = &(pMetadata->rMetaResult);

        prMetaResult->Dummy = it->Dummy;
        prMetaResult->MagicNum = i4FrmId;
        prMetaResult->halMeta = it->halMeta;
        prMetaResult->appMeta = it->appMeta;

        if(mParams.u1FaceDetectMode != MTK_STATISTICS_FACE_DETECT_MODE_OFF)
            prMetaResult->appMeta.remove(MTK_STATISTICS_FACE_DETECT_MODE);

        if (!mu1RepeatTag)
        {
            prMetaResult->appMeta = it->appMeta;
            prMetaResult->appMeta.remove(MTK_JPEG_THUMBNAIL_SIZE);
            prMetaResult->appMeta.remove(MTK_JPEG_ORIENTATION);
            // Update appMeta changed by 3A
            for(size_t k = 0; k < mUpdateMetaResult.size(); k++)
                prMetaResult->appMeta.update(mUpdateMetaResult[k].tag, mUpdateMetaResult[k].entry);
        }

        if ( mi4ForceDebugDump && (!(prMetaResult->Dummy)) )
        {
            mParams.u1IsGetExif = MTRUE;
            UPDATE_ENTRY_SINGLE<MUINT8>(prMetaResult->halMeta, MTK_HAL_REQUEST_REQUIRE_EXIF, 1);
        }
    }

    AAA_TRACE_HAL(face priority);
    // face priority
    if (mParams.u4SceneMode == MTK_CONTROL_SCENE_MODE_FACE_PRIORITY || mParams.i4ForceFace3A == 1 || mParams.u1FaceDetectMode != MTK_STATISTICS_FACE_DETECT_MODE_OFF)
    {
        if (!mu1FdEnable)
        {
            mpHal3aObj->setFDEnable(MTRUE);
            mu1FdEnable = MTRUE;
            mParams.bIsFDReady = MFALSE;
        }
    }
    else
    {
        if (mu1FdEnable)
        {
            MtkCameraFaceMetadata rFaceMeta;
            MtkCameraFaceMetadata rAFFaceMeta;
            mpHal3aObj->setFDInfo(&rFaceMeta, &rAFFaceMeta);
            mpHal3aObj->setFDEnable(MFALSE);
            mu1FdEnable = MFALSE;
        }
        mParams.bIsFDReady = MTRUE;
    }
    AAA_TRACE_END_HAL;

    MY_LOGD_IF(mu4LogEn, "[%s] Req(#%d) mu1FdEnable(%d) mParams.bIsFDReady(%d)", __FUNCTION__, i4FrmId, mu1FdEnable, mParams.bIsFDReady);

#if HAL3A_TEST_OVERRIDE
    if(mu4ManualMode != 0) _test(mu4ManualMode, mParams);
#endif
    AAA_TRACE_HAL(setAllParams);

    /************************************************************************************************
      * If (MagicNum is Natural Number, has to push back it in Request.vNumberset) - for TuningNode Searching Fail issue
      * If Magic Num truly has a number in it, we have to validateP1() ( push_back MagicNum in VRequest)
      * Addtionally, we shall prevent randomly generated garbage MagicNum (negative number)
      *************************************************************************************************/

    RequestSet_T rRequestSet;
    rRequestSet.vNumberSet.clear();
    rRequestSet.vNumberSet.push_back(std::max(0, i4FrmId));
    rRequestSet.fgDisableP1 = mParams.i4DisableP1;
    NS3Av3::ParamIspProfile_T _3AProf(mParams.eIspProfile, mParams.i4MagicNum, 0, MTRUE, NS3Av3::ParamIspProfile_T::EParamValidate_All, rRequestSet);

#if CAM3_STEREO_FEATURE_EN
    if (m_i4IsByPassCal == MTRUE)
    {
        mParams.bByPassStt = MTRUE;
    }
    else
    {
        mParams.bByPassStt = MFALSE;
    }

    MBOOL bAAOIsReady = mpHal3aObj->queryAaoIsReady();
    if (bAAOIsReady == MTRUE)
    {
        m_i4IsByPassCal = MFALSE;
    }

    mpHal3aObj->send3ACtrl(E3ACtrl_SetStereoParams, (MINTPTR)&mStereoParam, 0);
#endif

    mpHal3aObj->setParams(mParams);
    mpHal3aObj->setAfParams(mAfParams);

    AAA_TRACE_END_HAL;

    if (mParams.rcapParams.size() && mParams.u4SceneMode == MTK_CONTROL_SCENE_MODE_HDR)
    {
        // HAL HDR
        mu1HdrEnable = MTRUE;
        CaptureParam_T rCapParam;
        ::memcpy(&rCapParam, mParams.rcapParams.array(), sizeof(CaptureParam_T));
        mpHal3aObj->send3ACtrl(E3ACtrl_SetHalHdr, MTRUE, reinterpret_cast<MINTPTR>(&rCapParam));
        MY_LOGD_IF(fgLog, "[%s] HDR shot", __FUNCTION__);
    }
    else if(mu1HdrEnable != MFALSE)
    {
        // normal
        mu1HdrEnable = MFALSE;
        mpHal3aObj->send3ACtrl(E3ACtrl_SetHalHdr, MFALSE, NULL);
    }

    if(it->PreSetKey != m_i4Presetkey)
    {
        AAA_TRACE_HAL(AF_trigger);
        // AF trigger start
        if (mAfParams.u1AfTrig == MTK_CONTROL_AF_TRIGGER_START || mAfParams.u1AfTrigStart)
        {
            mpHal3aObj->autoFocus();
        }
        // AF trigger cancel
        else if (mAfParams.u1AfTrig == MTK_CONTROL_AF_TRIGGER_CANCEL)
        {
            mpHal3aObj->cancelAutoFocus();
        }
        AAA_TRACE_END_HAL;

        if ( (mParams.u1PrecapTrig == MTK_CONTROL_AE_PRECAPTURE_TRIGGER_START &&
            mParams.u4AeMode != MTK_CONTROL_AE_MODE_OFF) || mParams.u1PreCapStart)
        {
            MY_LOGD_IF(fgLog, "[%s] Precapture Trigger @ i4MagicNum(%d)", __FUNCTION__, mParams.i4MagicNum);
            mpHal3aObj->sendCommand(NS3Av3::ECmd_PrecaptureStart);
        }

        if (mParams.u1PrecapTrig == MTK_CONTROL_AE_PRECAPTURE_TRIGGER_CANCEL &&
            mParams.u4AeMode != MTK_CONTROL_AE_MODE_OFF)
        {
            MY_LOGD_IF(fgLog, "[%s] Precapture Cancel @ i4MagicNUm(%d)", __FUNCTION__, mParams.i4MagicNum);
            mpHal3aObj->sendCommand(NS3Av3::ECmd_PrecaptureEnd);
        }
    }
    m_i4Presetkey = 0;
    mpHal3aObj->sendCommand(NS3Av3::ECmd_Update, reinterpret_cast<MUINTPTR>(&_3AProf));


    AAA_TRACE_END_HAL;

    return 0;
}

MINT32
Hal3AAdapter3::
setSMVR(const vector<MetaSet_T*>& requestQ)
{
#define OVERRIDE_AE  (1<<0)
#define OVERRIDE_AWB (1<<1)
#define OVERRIDE_AF  (1<<2)

    CAM_TRACE_CALL();

    MINT32 i4FrmId = 0;
    MINT32 i4FrmIdCur = 0;
    MUINT8 u1AfTrig = MTK_CONTROL_AF_TRIGGER_IDLE;
    MUINT8 u1AfTrigSMVR = MTK_CONTROL_AF_TRIGGER_IDLE;
    MUINT8 u1AePrecapTrig = MTK_CONTROL_AE_PRECAPTURE_TRIGGER_IDLE;
    MUINT8 u1Override3A = (OVERRIDE_AE|OVERRIDE_AWB|OVERRIDE_AF);
    MUINT8 u1IspProfile = 0xFF;

    MBOOL fgLog = (mu4LogEn & HAL3AADAPTER3_LOG_PF) ? MTRUE : MFALSE;
    MBOOL fgLogEn0 = (mu4LogEn & HAL3AADAPTER3_LOG_SET_0) ? MTRUE : MFALSE;
    MBOOL fgLogEn1 = (mu4LogEn & HAL3AADAPTER3_LOG_SET_1) ? MTRUE : MFALSE;
    MBOOL fgLogEn2 = (mu4LogEn & HAL3AADAPTER3_LOG_SET_2) ? MTRUE : MFALSE;
    RequestSet_T rRequestSet;
    rRequestSet.vNumberSet.clear();
    for (MINT32 i4InitCount = 0; i4InitCount < m_rConfigInfo.i4SubsampleCount; i4InitCount++) //avoid dummy request
        rRequestSet.vNumberSet.push_back(0);


    MINT32 i4TgWidth = 0;
    MINT32 i4TgHeight = 0;
    MINT32 i4HbinWidth = 0;
    MINT32 i4HbinHeight = 0;
    mpHal3aObj->queryTgSize(i4TgWidth,i4TgHeight);
    mpHal3aObj->queryHbinSize(i4HbinWidth,i4HbinHeight);
    MINT32 i4Tg2HbinRatioW = i4TgWidth / i4HbinWidth;
    MINT32 i4Tg2HbinRatioH = i4TgHeight / i4HbinHeight;

    MY_LOGD_IF(fgLog, "[%s] sensorDev(%d), sensorIdx(%d), mu4Counter(%d), TG size(%d,%d) Hbin size(%d,%d) Ratio(%d,%d)", __FUNCTION__,
        mu4SensorDev, mi4SensorIdx, mu4Counter++, i4TgWidth, i4TgHeight, i4HbinWidth, i4HbinHeight, i4Tg2HbinRatioW, i4Tg2HbinRatioH);

    // reset
    mParams.u1IsGetExif = 0;
    mParams.u1HQCapEnable = 0;
    mParams.i4RawType = NSIspTuning::ERawType_Proc;
    mAfParams.u1AfTrig = 0;

    mLockResult.lock();

    // ResultPool - update 4 reqest magic, because convert 4 metadata use
    MINT32 rConvertMagic4SMVR[ConvertNum4SMVR] = {0,0,0,0};

    for (MINT32 i = 0; i < requestQ.size(); i++)
    {
        MetaSet_T* it = requestQ[i];
        MRect rSclCropRect;
        MINT32 rSclCrop[4];
        MBOOL fgCrop = MFALSE;
        const IMetadata& _appmeta = it->appMeta;

        const IMetadata& _halmeta = it->halMeta;

        MBOOL fgOK = QUERY_ENTRY_SINGLE(_halmeta, MTK_P1NODE_PROCESSOR_MAGICNUM, i4FrmId);

        MUINT8 uRepeatTag = 0;
        IMetadata::IEntry repeatEntry = _halmeta.entryFor(MTK_HAL_REQUEST_REPEAT);
        if (!repeatEntry.isEmpty())
        {
            QUERY_ENTRY_SINGLE(_halmeta, MTK_HAL_REQUEST_REPEAT, uRepeatTag);
            MY_LOGD_IF(fgLogEn1,"[%s] uRepeatTag(%d) i4FrmId(%d)", __FUNCTION__, uRepeatTag, i4FrmId);
        }

        // ResultPool - 1. Clear the new ResultPool of request magic
        MY_LOGD_IF(fgLog, "[%s] Req(#%d) getValidateMetadata/clearOldestResultPool", __FUNCTION__, i4FrmId);
        // ResultPool - 2. Get validate buffer by request magic
        MBOOL ret = mpResultPoolObj->findValidateBuffer(i4FrmId);

        // Result - 3. Update config information
        RESULT_CFG_T rResultCfg;
        rResultCfg.i4ReqMagic = i4FrmId;
        rResultCfg.i4PreSetKey = i4FrmId;
        ret = mpResultPoolObj->updateResultCFG(rResultCfg);

        // Result - 4. Assign metadata data
        AllMetaResult_T* pMetadata = mpResultPoolObj->getMetadata(i4FrmId);
        MetaSet_T* prMetaResult = NULL;
        if(pMetadata != NULL)
        {
            std::lock_guard<std::mutex> lock(pMetadata->LockMetaResult);
            prMetaResult = &(pMetadata->rMetaResult);
            prMetaResult->Dummy = it->Dummy;
            prMetaResult->MagicNum = i4FrmId;
            if (fgOK) {
                if (mi4ForceDebugDump) {
                    mParams.u1IsGetExif = MTRUE;
                    UPDATE_ENTRY_SINGLE<MUINT8>(prMetaResult->halMeta, MTK_HAL_REQUEST_REQUIRE_EXIF, 1);
                }
                prMetaResult->halMeta = _halmeta;
            }
        }

        rConvertMagic4SMVR[(i % m_rConfigInfo.i4SubsampleCount)] = i4FrmId;

        QUERY_ENTRY_SINGLE(_halmeta, MTK_3A_ISP_PROFILE, u1IspProfile);
        QUERY_ENTRY_SINGLE(_halmeta, MTK_P1NODE_RAW_TYPE, mParams.i4RawType);

        if (QUERY_ENTRY_SINGLE(_halmeta, MTK_3A_PRV_CROP_REGION, rSclCropRect))
        {
            fgCrop = MTRUE;
            mPrvCropRegion.p.x = rSclCropRect.p.x;
            mPrvCropRegion.p.y = rSclCropRect.p.y;
            mPrvCropRegion.s.w = rSclCropRect.s.w;
            mPrvCropRegion.s.h = rSclCropRect.s.h;

            // crop info for AE
            mParams.rScaleCropRect.i4Xoffset = rSclCropRect.p.x;
            mParams.rScaleCropRect.i4Yoffset = rSclCropRect.p.y;
            mParams.rScaleCropRect.i4Xwidth = rSclCropRect.s.w;
            mParams.rScaleCropRect.i4Yheight = rSclCropRect.s.h;

            // crop info for AF
            CameraArea_T& rArea = mAfParams.rScaleCropArea;
            rArea.i4Left   = rSclCropRect.p.x;
            rArea.i4Top    = rSclCropRect.p.y;
            rArea.i4Right  = rSclCropRect.p.x + rSclCropRect.s.w;
            rArea.i4Bottom = rSclCropRect.p.y + rSclCropRect.s.h;
            rArea = _transformArea(mi4SensorIdx,mi4SensorMode,rArea);
            rArea = _clipArea(i4TgWidth,i4TgHeight,rArea);

            MY_LOGD_IF(fgLogEn0, "[%s] AE SCL CROP(%d,%d,%d,%d) AF SCL CROP(%d,%d,%d,%d)",
                        __FUNCTION__, mParams.rScaleCropRect.i4Xoffset, mParams.rScaleCropRect.i4Yoffset, mParams.rScaleCropRect.i4Xwidth, mParams.rScaleCropRect.i4Yheight,
                        rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom);
        }
        else
        {
            // 1. Pass1 and Pass2 MW send 3A preview crop region to meta with HAL1.
            // 2. Pass1 MW does not send 3A preview crop region to meta with HAL3.
            // 2. 3A HAL need get 3A preview crop region from ISP Pass2.
            P2Info_T rP2Info;
            mpResultPoolObj->getP2Info(rP2Info);

            mPrvCropRegion.p.x = rP2Info.rSclCropRectl.p.x;
            mPrvCropRegion.p.y = rP2Info.rSclCropRectl.p.y;
            mPrvCropRegion.s.w = rP2Info.rSclCropRectl.s.w;
            mPrvCropRegion.s.h = rP2Info.rSclCropRectl.s.h;

            // crop info for AE
            mParams.rScaleCropRect.i4Xoffset = rP2Info.rSclCropRectl.p.x;
            mParams.rScaleCropRect.i4Yoffset = rP2Info.rSclCropRectl.p.y;
            mParams.rScaleCropRect.i4Xwidth  = rP2Info.rSclCropRectl.s.w;
            mParams.rScaleCropRect.i4Yheight = rP2Info.rSclCropRectl.s.h;

            // crop info for AF
            CameraArea_T& rArea = mAfParams.rScaleCropArea;

            rArea.i4Left   = mPrvCropRegion.p.x;
            rArea.i4Top    = mPrvCropRegion.p.y;
            rArea.i4Right  = mPrvCropRegion.p.x + mPrvCropRegion.s.w;
            rArea.i4Bottom = mPrvCropRegion.p.y + mPrvCropRegion.s.h;
            rArea = _transformArea(mi4SensorIdx, mi4SensorMode, rArea);
            rArea = _clipArea(i4TgWidth, i4TgHeight, rArea);

            MY_LOGD_IF(fgLogEn0, "[%s] Pass2 info AE SCL CROP(%d,%d,%d,%d) AF SCL CROP(%d,%d,%d,%d)",
                    __FUNCTION__, mParams.rScaleCropRect.i4Xoffset, mParams.rScaleCropRect.i4Yoffset, mParams.rScaleCropRect.i4Xwidth, mParams.rScaleCropRect.i4Yheight,
                    rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom);

        }

        if(QUERY_ENTRY_SINGLE(_appmeta, MTK_SCALER_CROP_REGION, rSclCropRect))
        {
            fgCrop = MTRUE;
            mAppCropRegion.p.x = rSclCropRect.p.x;
            mAppCropRegion.p.y = rSclCropRect.p.y;
            mAppCropRegion.s.w = rSclCropRect.s.w;
            mAppCropRegion.s.h = rSclCropRect.s.h;
            rSclCrop[0] = rSclCropRect.p.x;
            rSclCrop[1] = rSclCropRect.p.y;
            rSclCrop[2] = rSclCropRect.p.x + rSclCropRect.s.w;
            rSclCrop[3] = rSclCropRect.p.y + rSclCropRect.s.h;
            MY_LOGD_IF(fgLogEn0, "[%s] SCL CROP(%d,%d,%d,%d)", __FUNCTION__, rSclCrop[0], rSclCrop[1], rSclCrop[2], rSclCrop[3]);

            MINT32 i4ZoomRatio = 0;
            if( QUERY_ENTRY_SINGLE(_halmeta, MTK_DUALZOOM_ZOOMRATIO, i4ZoomRatio))
            {
                MY_LOGD_IF(fgLogEn0, "i4ZoomRatio(%d) from AP, SensorDev(%d)", i4ZoomRatio, mu4SensorDev);
            } else {
                MRect ActiveArrayCropRegionSize;
                if(QUERY_ENTRY_SINGLE(_halmeta, MTK_SENSOR_MODE_INFO_ACTIVE_ARRAY_CROP_REGION, ActiveArrayCropRegionSize))
                {
                    MY_LOGD_IF(fgLogEn0, "[%s] ActiveCropRgn(%d,%d,%d,%d) PrvCropRegion(%d,%d,%d,%d)", __FUNCTION__, ActiveArrayCropRegionSize.p.x, ActiveArrayCropRegionSize.p.y, ActiveArrayCropRegionSize.s.w, ActiveArrayCropRegionSize.s.h,
                                     mPrvCropRegion.p.x, mPrvCropRegion.p.y, mPrvCropRegion.s.w, mPrvCropRegion.s.h);
                    CameraArea_T ActiveArrayCropRegionSize2TG;
                    ActiveArrayCropRegionSize2TG.i4Left   = ActiveArrayCropRegionSize.p.x;
                    ActiveArrayCropRegionSize2TG.i4Top    = ActiveArrayCropRegionSize.p.y;
                    ActiveArrayCropRegionSize2TG.i4Right  = ActiveArrayCropRegionSize.p.x + ActiveArrayCropRegionSize.s.w;
                    ActiveArrayCropRegionSize2TG.i4Bottom = ActiveArrayCropRegionSize.p.y + ActiveArrayCropRegionSize.s.h;
                    ActiveArrayCropRegionSize2TG = _transformArea(mi4SensorIdx, mi4SensorMode, ActiveArrayCropRegionSize2TG);
                    ActiveArrayCropRegionSize2TG = _clipArea(i4TgWidth, i4TgHeight, ActiveArrayCropRegionSize2TG);

                    MY_LOGD_IF(fgLogEn0, "[%s] ActiveCropRgn2TG(%d,%d,%d,%d)", __FUNCTION__, ActiveArrayCropRegionSize2TG.i4Left, ActiveArrayCropRegionSize2TG.i4Top, ActiveArrayCropRegionSize2TG.i4Right, ActiveArrayCropRegionSize2TG.i4Bottom);

                    MINT32 i4X = ActiveArrayCropRegionSize2TG.i4Left;
                    MINT32 i4Y = ActiveArrayCropRegionSize2TG.i4Top;
                    MINT32 i4W = ActiveArrayCropRegionSize2TG.i4Right - ActiveArrayCropRegionSize2TG.i4Left;
                    MINT32 i4H = ActiveArrayCropRegionSize2TG.i4Bottom - ActiveArrayCropRegionSize2TG.i4Top;
                    i4ZoomRatio = (((i4W*100)/mPrvCropRegion.s.w) < ((i4H*100)/mPrvCropRegion.s.h)) ? ((i4W*100)/mPrvCropRegion.s.w) : ((i4H*100)/mPrvCropRegion.s.h);

                    MY_LOGD_IF(fgLogEn0, "[%s] i4ZoomRatio(%d)", __FUNCTION__, i4ZoomRatio);
                }
                else
                {
                    MY_LOGD_IF(fgLogEn0, "[%s] ActiveRgn(%d,%d,%d,%d) PrvCropRegion(%d,%d,%d,%d)", __FUNCTION__, mActiveArraySize.p.x, mActiveArraySize.p.y, mActiveArraySize.s.w, mActiveArraySize.s.h,
                                     mPrvCropRegion.p.x, mPrvCropRegion.p.y, mPrvCropRegion.s.w, mPrvCropRegion.s.h);
                    CameraArea_T ActiveArraySize2TG;
                    ActiveArraySize2TG.i4Left   = mActiveArraySize.p.x;
                    ActiveArraySize2TG.i4Top    = mActiveArraySize.p.y;
                    ActiveArraySize2TG.i4Right  = mActiveArraySize.p.x + mActiveArraySize.s.w;
                    ActiveArraySize2TG.i4Bottom = mActiveArraySize.p.y + mActiveArraySize.s.h;
                    ActiveArraySize2TG = _transformArea(mi4SensorIdx, mi4SensorMode, ActiveArraySize2TG);
                    ActiveArraySize2TG = _clipArea(i4TgWidth, i4TgHeight, ActiveArraySize2TG);

                    MY_LOGD_IF(fgLogEn0, "[%s] ActiveRgn2TG(%d,%d,%d,%d)", __FUNCTION__, ActiveArraySize2TG.i4Left, ActiveArraySize2TG.i4Top, ActiveArraySize2TG.i4Right, ActiveArraySize2TG.i4Bottom);

                    MINT32 i4X = ActiveArraySize2TG.i4Left;
                    MINT32 i4Y = ActiveArraySize2TG.i4Top;
                    MINT32 i4W = ActiveArraySize2TG.i4Right - ActiveArraySize2TG.i4Left;
                    MINT32 i4H = ActiveArraySize2TG.i4Bottom - ActiveArraySize2TG.i4Top;
                    i4ZoomRatio = (((i4W*100)/mPrvCropRegion.s.w) < ((i4H*100)/mPrvCropRegion.s.h)) ? ((i4W*100)/mPrvCropRegion.s.w) : ((i4H*100)/mPrvCropRegion.s.h);

                    MY_LOGD_IF(fgLogEn0, "[%s] i4ZoomRatio(%d)", __FUNCTION__, i4ZoomRatio);
                }
                MY_LOGD_IF(fgLogEn0, "i4ZoomRatio(%d) SensorDev(%d)", i4ZoomRatio, mu4SensorDev);
            }
            mParams.i4ZoomRatio = i4ZoomRatio;

        }

        if (fgOK)   // && (i4requestSize >= m_rConfigInfo.i4SubsampleCount *(HAL3A_REQ_PROC_KEY + 1)))      // Probably not in use anymore
        {
            //mMetaResult = *it;
            if(i == 0)
            {
                mParams.i4MagicNum = i4FrmId;
                mAfParams.i4MagicNum = i4FrmId;
                rRequestSet.vNumberSet.clear();
            }
            if (!uRepeatTag) //not repeating tag, parse app meta
            {
                mUpdateMetaResult.clear();
                for (MINT32 j = 0; j < _appmeta.count(); j++)
                {
                    IMetadata::IEntry entry = _appmeta.entryAt(j);
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
                            MY_LOGD_IF(mParams.u1ControlMode != u1ControlMode, "[%s] MTK_CONTROL_MODE(%d -> %d)", __FUNCTION__, mParams.u1ControlMode, u1ControlMode);
                            mParams.u1ControlMode = u1ControlMode;
                        }
                        break;
                    case MTK_CONTROL_CAPTURE_INTENT:
                        {
                            MUINT8 u1CapIntent = entry.itemAt(0, Type2Type< MUINT8 >());
                            MY_LOGD_IF(mParams.u1CaptureIntent != u1CapIntent, "[%s] MTK_CONTROL_CAPTURE_INTENT(%d -> %d)", __FUNCTION__, mParams.u1CaptureIntent, u1CapIntent);
                            mParams.u1CaptureIntent = u1CapIntent;
                            mAfParams.u1CaptureIntent = u1CapIntent;
                        }
                        break;

                    // AWB
                    case MTK_CONTROL_AWB_LOCK:
                        {
                            MUINT8 bLock = entry.itemAt(0, Type2Type< MUINT8 >());
                            MY_LOGD_IF(mParams.bIsAWBLock != bLock, "[%s] MTK_CONTROL_AWB_LOCK(%d -> %d)", __FUNCTION__, mParams.bIsAWBLock, bLock);
                            mParams.bIsAWBLock = bLock;
                        }
                        break;
                    case MTK_CONTROL_AWB_MODE:  // dynamic
                        {
                            MUINT8 u1Mode = entry.itemAt(0, Type2Type< MUINT8 >());
                            MY_LOGD_IF(mParams.u4AwbMode != u1Mode, "[%s] MTK_CONTROL_AWB_MODE(%d -> %d)", __FUNCTION__, mParams.u4AwbMode, u1Mode);
                            mParams.u4AwbMode = u1Mode;
                            if(mbEnableOverride)
                                u1Override3A &= (~OVERRIDE_AWB);
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
                                        rArea.i4Left   = MIN(MAX(rArea.i4Left, rSclCrop[0]), rSclCrop[2]);
                                        rArea.i4Top    = MIN(MAX(rArea.i4Top, rSclCrop[1]), rSclCrop[3]);
                                        rArea.i4Right  = MAX(MIN(rArea.i4Right, rSclCrop[2]), rSclCrop[0]);
                                        rArea.i4Bottom = MAX(MIN(rArea.i4Bottom, rSclCrop[3]), rSclCrop[1]);
                                    }
                                    MY_LOGD("[%s] Return to AP AWB_REGIONS L(%d) T(%d) R(%d) B(%d) W(%d)", __FUNCTION__, rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom, rArea.i4Weight);

                                    entryNew.push_back(rArea.i4Left,   Type2Type<MINT32>());
                                    entryNew.push_back(rArea.i4Top,    Type2Type<MINT32>());
                                    entryNew.push_back(rArea.i4Right,  Type2Type<MINT32>());
                                    entryNew.push_back(rArea.i4Bottom, Type2Type<MINT32>());
                                    entryNew.push_back(rArea.i4Weight, Type2Type<MINT32>());
                                    rArea = _transformArea(mi4SensorIdx,mi4SensorMode,rArea);
                                    rArea = _clipArea(i4TgWidth,i4TgHeight,rArea);
                                    MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AWB_REGIONS mod L(%d) T(%d) R(%d) B(%d) W(%d)", __FUNCTION__, rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom, rArea.i4Weight);
                                }
                                mUpdateMetaResult.push_back({MTK_CONTROL_AWB_REGIONS, entryNew});
                            }
                        }
                        break;

                    // AE
                    case MTK_CONTROL_AE_ANTIBANDING_MODE:
                        {
                            MINT32 i4Mode = entry.itemAt(0, Type2Type< MUINT8 >());
                            MY_LOGD_IF(mParams.u4AntiBandingMode != i4Mode, "[%s] MTK_CONTROL_AE_ANTIBANDING_MODE(%d -> %d)", __FUNCTION__, mParams.u4AntiBandingMode, i4Mode);
                            mParams.u4AntiBandingMode = i4Mode;
                        }
                        break;
                    case MTK_CONTROL_AE_EXPOSURE_COMPENSATION:
                        {
                            MINT32 i4ExpIdx = entry.itemAt(0, Type2Type< MINT32 >());
                            MY_LOGD_IF(mParams.i4ExpIndex != i4ExpIdx, "[%s] MTK_CONTROL_AE_EXPOSURE_COMPENSATION(%d -> %d)", __FUNCTION__, mParams.i4ExpIndex, i4ExpIdx);
                            mParams.i4ExpIndex = i4ExpIdx;
                        }
                        break;
                    case MTK_CONTROL_AE_LOCK:
                        {
                            MUINT8 bLock = entry.itemAt(0, Type2Type< MUINT8 >());
                            MY_LOGD_IF(mParams.bIsAELock != bLock, "[%s] MTK_CONTROL_AE_LOCK(%d -> %d)", __FUNCTION__, mParams.bIsAELock, bLock);
                            mParams.bIsAELock = bLock;
                        }
                        break;
                    case MTK_CONTROL_AE_MODE:
                        {
                            MUINT8 u1Mode = entry.itemAt(0, Type2Type< MUINT8 >());
                            MY_LOGD_IF(mParams.u4AeMode != u1Mode, "[%s] MTK_CONTROL_AE_MODE(%d -> %d)", __FUNCTION__, mParams.u4AeMode, u1Mode);
                            mParams.u4AeMode = u1Mode;
                            if(mbEnableOverride)
                                u1Override3A &= (~OVERRIDE_AE);
                        }
                        break;
                    case MTK_CONTROL_AE_REGIONS:    // dynamic
                        {
                            if ((m_rStaticInfo.i4MaxRegionAe == 0) || (u1AfTrigSMVR == MTK_CONTROL_AF_TRIGGER_START))
                            {
                                if ((u1AfTrig != MTK_CONTROL_AF_TRIGGER_IDLE))
                                    MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AE_REGIONS u1AfTrig(%d)", __FUNCTION__, u1AfTrig);
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
                                        rArea.i4Left   = MIN(MAX(rArea.i4Left, rSclCrop[0]), rSclCrop[2]);
                                        rArea.i4Top    = MIN(MAX(rArea.i4Top, rSclCrop[1]), rSclCrop[3]);
                                        rArea.i4Right  = MAX(MIN(rArea.i4Right, rSclCrop[2]), rSclCrop[0]);
                                        rArea.i4Bottom = MAX(MIN(rArea.i4Bottom, rSclCrop[3]), rSclCrop[1]);
                                    }
                                    MY_LOGD("[%s] Return to AP AE_REGIONS L(%d) T(%d) R(%d) B(%d) W(%d)", __FUNCTION__, rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom, rArea.i4Weight);

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
                                mUpdateMetaResult.push_back({MTK_CONTROL_AE_REGIONS, entryNew});
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
                            MUINT8 u1Ae = entry.itemAt(0, Type2Type< MUINT8 >());
                            u1AePrecapTrig = std::max(u1AePrecapTrig, u1Ae);
                            MY_LOGD_IF(mParams.u1PrecapTrig != u1AePrecapTrig, "[%s] MTK_CONTROL_AE_PRECAPTURE_TRIGGER(%d -> %d)", __FUNCTION__, mParams.u1PrecapTrig, u1AePrecapTrig);
                            mParams.u1PrecapTrig = u1AePrecapTrig;
                        }
                        break;
                    case MTK_CONTROL_AE_PRECAPTURE_ID:
                        {
                            MINT32 i4AePreCapId = entry.itemAt(0, Type2Type< MINT32 >());
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AE_PRECAPTURE_ID(%d)", __FUNCTION__, i4AePreCapId);
                        }
                        break;
                    case MTK_FLASH_MODE:
                        {
                            MUINT8 u1Mode = entry.itemAt(0, Type2Type< MUINT8 >());
                            MY_LOGD_IF(mParams.u4StrobeMode != u1Mode, "[%s] MTK_FLASH_MODE(%d -> %d)", __FUNCTION__, mParams.u4StrobeMode, u1Mode);
                            mParams.u4StrobeMode = u1Mode;
                        }
                    break;

                    // AF
                    case MTK_CONTROL_AF_MODE:
                        {
                            MUINT8 u1AfMode = entry.itemAt(0, Type2Type< MUINT8 >());
                            MY_LOGD_IF(mAfParams.u4AfMode != u1AfMode, "[%s] MTK_CONTROL_AF_MODE(%d -> %d)", __FUNCTION__, mAfParams.u4AfMode, u1AfMode);
                            mAfParams.u4AfMode = u1AfMode; //_convertAFMode(u1AfMode);
                            if(mbEnableOverride)
                                u1Override3A &= (~OVERRIDE_AF);
                        }
                        break;
                    case MTK_CONTROL_AF_REGIONS:    // dynamic
                        {
                            if ((m_rStaticInfo.i4MaxRegionAf == 0) || (u1AfTrigSMVR == MTK_CONTROL_AF_TRIGGER_START))
                            {
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
                                        rArea.i4Left   = MIN(MAX(rArea.i4Left, rSclCrop[0]), rSclCrop[2]);
                                        rArea.i4Top    = MIN(MAX(rArea.i4Top, rSclCrop[1]), rSclCrop[3]);
                                        rArea.i4Right  = MAX(MIN(rArea.i4Right, rSclCrop[2]), rSclCrop[0]);
                                        rArea.i4Bottom = MAX(MIN(rArea.i4Bottom, rSclCrop[3]), rSclCrop[1]);
                                    }
                                    MY_LOGD("[%s] Return to AP AF_REGIONS L(%d) T(%d) R(%d) B(%d) W(%d)", __FUNCTION__, rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom, rArea.i4Weight);

                                    entryNew.push_back(rArea.i4Left,   Type2Type<MINT32>());
                                    entryNew.push_back(rArea.i4Top,    Type2Type<MINT32>());
                                    entryNew.push_back(rArea.i4Right,  Type2Type<MINT32>());
                                    entryNew.push_back(rArea.i4Bottom, Type2Type<MINT32>());
                                    entryNew.push_back(rArea.i4Weight, Type2Type<MINT32>());
                                    rArea = _transformArea(mi4SensorIdx,mi4SensorMode,rArea);
                                    rArea = _clipArea(i4TgWidth,i4TgHeight,rArea);
                                    MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AF_REGIONS mod L(%d) T(%d) R(%d) B(%d) W(%d)", __FUNCTION__, rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom, rArea.i4Weight);
                                }
                                mUpdateMetaResult.push_back({MTK_CONTROL_AF_REGIONS, entryNew});
                            }
                        }
                        break;
                    case MTK_CONTROL_AF_TRIGGER:
                        {
                            MUINT8 u1Af = entry.itemAt(0, Type2Type< MUINT8 >());
                            u1AfTrig = (u1Af != MTK_CONTROL_AF_TRIGGER_IDLE) ? u1Af : u1AfTrig;
                            MY_LOGD_IF(mAfParams.u1AfTrig != u1AfTrig, "[%s] MTK_CONTROL_AF_TRIGGER(%d -> %d)", __FUNCTION__, mAfParams.u1AfTrig, u1AfTrig);
                            mAfParams.u1AfTrig = u1AfTrig;
                        }
                        break;
                    case MTK_CONTROL_AF_TRIGGER_ID:
                        {
                            MINT32 i4AfTrigId = entry.itemAt(0, Type2Type< MINT32 >());
                            MY_LOGD_IF(fgLogEn1, "[%s] MTK_CONTROL_AF_TRIGGER_ID(%d)", __FUNCTION__, i4AfTrigId);
                        }
                        break;
                    // ISP
                    case MTK_CONTROL_EFFECT_MODE:
                        {
                            MUINT8 u1EffectMode = entry.itemAt(0, Type2Type< MUINT8 >());
                            MY_LOGD_IF(mParams.u4EffectMode != u1EffectMode, "[%s] MTK_CONTROL_EFFECT_MODE(%d -> %d)", __FUNCTION__, mParams.u4EffectMode, u1EffectMode);
                            mParams.u4EffectMode = u1EffectMode;
                        }
                        break;
                    case MTK_CONTROL_SCENE_MODE:
                        {
                            MUINT8 u1SceneMode = entry.itemAt(0, Type2Type< MUINT8 >());
                            MY_LOGD_IF(mParams.u4SceneMode != u1SceneMode, "[%s] MTK_CONTROL_SCENE_MODE(%d -> %d)", __FUNCTION__, mParams.u4SceneMode, u1SceneMode);
                            mParams.u4SceneMode = u1SceneMode;
                        }
                        break;
                    default:
                        break;
                    }
                }
            }//not repeating tag, parse app meta
            CAM_TRACE_BEGIN("copy appMeta");
            if (prMetaResult)
            {
                prMetaResult->appMeta = (it->appMeta);
                prMetaResult->appMeta.remove(MTK_JPEG_THUMBNAIL_SIZE);
                prMetaResult->appMeta.remove(MTK_JPEG_ORIENTATION);

                // Update appMeta changed by 3A
                for(size_t k = 0; k < mUpdateMetaResult.size(); k++)
                {
                    prMetaResult->appMeta.update(mUpdateMetaResult[k].tag, mUpdateMetaResult[k].entry);
                }
            }
            if (u1AfTrig == MTK_CONTROL_AF_TRIGGER_START)
                u1AfTrigSMVR = MTK_CONTROL_AF_TRIGGER_START;
            CAM_TRACE_END();
            MY_LOGD_IF(fgLogEn0, "[%s] magic(%d)", __FUNCTION__, i4FrmId);
            rRequestSet.vNumberSet.push_back(i4FrmId);
        }
        else
        {
            MY_LOGD("Warning!! fail to get magic(#%d)", i4FrmId);
            break;
        }

    }

    // ResultPool - Copy rConvertMagic4SMVR to resultPool

    MY_LOGD_IF(fgLog, "[%s] update ConvertMagic to ResutlPool(Req, Req4SMVR0, Req4SMVR1, Req4SMVR2, Req4SMVR3) = (#%d, #%d, #%d, #%d, #%d)"
        , __FUNCTION__, mParams.i4MagicNum, rConvertMagic4SMVR[0], rConvertMagic4SMVR[1], rConvertMagic4SMVR[2], rConvertMagic4SMVR[3]);

    for(MINT32 i = 0; i < ConvertNum4SMVR; i++)
    {
        AllResult_T* pAllResutl = mpResultPoolObj->getAllResult(rConvertMagic4SMVR[i]);
        if(pAllResutl != NULL)
            ::memcpy(pAllResutl->rOld3AInfo.i4ConvertMagic, rConvertMagic4SMVR, sizeof(rConvertMagic4SMVR));
    }

    mLockResult.unlock();

    // AF trigger cancel
    if (u1AfTrig == MTK_CONTROL_AF_TRIGGER_CANCEL)
    {
        mpHal3aObj->cancelAutoFocus();
    }

    // ISP profile

    if ( u1IspProfile == 0xFF){
        switch (mParams.u1CaptureIntent)
        {
            case MTK_CONTROL_CAPTURE_INTENT_PREVIEW:
            case MTK_CONTROL_CAPTURE_INTENT_ZERO_SHUTTER_LAG:
                mParams.eIspProfile = NSIspTuning::EIspProfile_Preview;
                break;
            case MTK_CONTROL_CAPTURE_INTENT_VIDEO_RECORD:
            case MTK_CONTROL_CAPTURE_INTENT_VIDEO_SNAPSHOT:
                mParams.eIspProfile = NSIspTuning::EIspProfile_Video;
                break;
            default:
                mParams.eIspProfile = NSIspTuning::EIspProfile_Preview;
            break;
        }
    }
    else{
        mParams.eIspProfile = static_cast<NSIspTuning::EIspProfile_T>(u1IspProfile);
    }

    MY_LOGD_IF(fgLog,"[%s] Update @ i4FrmId(%d), dummy(%d), IspProfile(%d)", __FUNCTION__, i4FrmId, (mParams.u4HalFlag & HAL_FLG_DUMMY2) ? 1 : 0, mParams.eIspProfile);
    NS3Av3::ParamIspProfile_T _3AProf(mParams.eIspProfile, i4FrmId, i4FrmIdCur, MTRUE, NS3Av3::ParamIspProfile_T::EParamValidate_All, rRequestSet);

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

MINT32
Hal3AAdapter3::
get(MUINT32 frmId, MetaSet_T& result)
{
    AAA_TRACE_D("AdapterGet");
    AAA_TRACE_HAL(AdapterGet);

    MBOOL fgLog = mu4LogEn & HAL3AADAPTER3_LOG_PF;
    MBOOL fgLogEn0 = mu4LogEn & HAL3AADAPTER3_LOG_GET_0;
    MBOOL fgLogEn1 = mu4LogEn & HAL3AADAPTER3_LOG_GET_1;
    MINT32 i4Ret = MTRUE;
    MBOOL bEnMetaClear = MTRUE; //If MW get Current MetaResult. 3A don't clear MetaResult.
    MINT32 i4IsConvert4MetaResult = E_NON_CONVERT;
    MY_LOGD_IF(fgLog, "[%s] sensorDev(%d), sensorIdx(%d) R(%d)", __FUNCTION__, mu4SensorDev, mi4SensorIdx, frmId);

    /*****************************
     *     get metedata from pool
     *****************************/
    std::lock_guard<std::mutex> lock(mLock);
    std::lock_guard<std::mutex> lock_meta(mLockResult);

    AAA_TRACE_HAL(getMetadata);
    // ResultPool - Get the specified result
    AllMetaResult_T* pMetaResult = mpResultPoolObj->getMetadata(frmId);
    AllResult_T *pAllResult = mpResultPoolObj->getAllResult(frmId);

    if(pMetaResult == NULL)
    {
        MY_LOGW("[%s] Fail to get the specified metadata(#%d), return -1", __FUNCTION__, frmId);
        AAA_TRACE_END_HAL;
        AAA_TRACE_END_HAL;
        AAA_TRACE_END_D;
        return (-1);
    }

    // TODO : check this flow is working or not.
    {
        std::lock_guard<std::mutex> lock(pMetaResult->LockMetaResult);
        i4IsConvert4MetaResult = pMetaResult->i4IsConverted;
        UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.halMeta, MTK_3A_REPEAT_RESULT, (MUINT8)MFALSE);
    }
    AAA_TRACE_END_HAL;

    /*****************************
     *     check bad picture
     *****************************/
    AAA_TRACE_HAL(checkBadPicture);
    HALResultToMeta_T* pHalResult = (HALResultToMeta_T*)mpResultPoolObj->getResult(frmId,E_HAL_RESULTTOMETA, __FUNCTION__);
    if(pHalResult != NULL)
    {
        if(pHalResult->fgBadPicture)
            MY_LOGD("[%s] Bad Picture #(%d), fgBadPicture(%d)", __FUNCTION__, frmId, pHalResult->fgBadPicture);
        {
            std::lock_guard<std::mutex> lock(pMetaResult->LockMetaResult);
            UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.halMeta, MTK_3A_SKIP_BAD_FRAME, (MBOOL)pHalResult->fgBadPicture);
        }
    }
    AAA_TRACE_END_HAL;

    /*****************************
     *     update metedata to MW
     *****************************/
    AAA_TRACE_HAL(updateMetadata);
    MINT32 i4ConvertType = 0;
    if(!(i4IsConvert4MetaResult & E_2A_CONVERT))
        i4ConvertType |= E_2A_CONVERT;
    if(!(i4IsConvert4MetaResult & E_AF_CONVERT))
        i4ConvertType |= E_AF_CONVERT;
    if(i4ConvertType != 0)
        mpResultPoolObj->convertToMetadataFlow(frmId, i4ConvertType, pAllResult, pMetaResult);

    {
        std::lock_guard<std::mutex> lock(pMetaResult->LockMetaResult);
        result = pMetaResult->rMetaResult;
    }
    //TODO : fetch focal length in result !
    AAA_TRACE_END_HAL;

    MUINT8 u1CaptureIntent = 0;
    MUINT8 u1AwbState = 0;
    MUINT8 u1AeState = 0;
    MUINT8 u1FlashState = 0;
    MUINT8 u1AfState = 0;
    MINT64 i8AeExposureTime = 0;
    MINT32 i4AeSensitivity = 0;
    MINT64 i8FrameDuration = 0;
    MBOOL ret = QUERY_ENTRY_SINGLE(result.appMeta, MTK_CONTROL_CAPTURE_INTENT, u1CaptureIntent);
    QUERY_ENTRY_SINGLE(result.appMeta, MTK_CONTROL_AWB_STATE, u1AwbState);
    QUERY_ENTRY_SINGLE(result.appMeta, MTK_CONTROL_AE_STATE, u1AeState);
    QUERY_ENTRY_SINGLE(result.appMeta, MTK_FLASH_STATE, u1FlashState);
    QUERY_ENTRY_SINGLE(result.appMeta, MTK_CONTROL_AF_STATE, u1AfState);
    QUERY_ENTRY_SINGLE(result.appMeta, MTK_SENSOR_EXPOSURE_TIME, i8AeExposureTime);
    QUERY_ENTRY_SINGLE(result.appMeta, MTK_SENSOR_SENSITIVITY, i4AeSensitivity);
    QUERY_ENTRY_SINGLE(result.appMeta, MTK_SENSOR_FRAME_DURATION, i8FrameDuration);
    MY_LOGD("[%s] frmId(%d), IsConvert4MetaResult(%d), ret(%d), u1CaptureIntent(%d), count(%d, %d), AwbState(%d) AeState(%d) FlashState(%d) AfState(%d) Exp(%lld) ISO(%d) FrameDuration(%lld)", __FUNCTION__,
                frmId, i4IsConvert4MetaResult, ret, u1CaptureIntent, result.appMeta.count(), result.halMeta.count(),
                u1AwbState, u1AeState, u1FlashState, u1AfState, (long long)i8AeExposureTime, i4AeSensitivity, (long long)i8FrameDuration);
    if(!ret)
    {
        MY_LOGW("[%s] Fail to get MTK_CONTROL_CAPTURE_INTENT in result(#%d)", __FUNCTION__, frmId);
        AAA_TRACE_END_HAL;
        AAA_TRACE_END_D;
        //return (-1);
    }

    /*****************************
     *     clear metedata of pool
     *****************************/
    AAA_TRACE_HAL(clearMetadata);
    if(m_rConfigInfo.i4SubsampleCount > 1)
        mSubSampleCount4Clear++;

    if(bEnMetaClear && m_rConfigInfo.i4SubsampleCount <= 1)
    {
        std::lock_guard<std::mutex> lock(pMetaResult->LockMetaResult);
        pMetaResult->rMetaResult.halMeta.clear();
        pMetaResult->rMetaResult.appMeta.clear();
    }
    else if(bEnMetaClear && m_rConfigInfo.i4SubsampleCount > 1 && mSubSampleCount4Clear == m_rConfigInfo.i4SubsampleCount)
    {
        std::lock_guard<std::mutex> lock(pMetaResult->LockMetaResult);
        pMetaResult->rMetaResult.halMeta.clear();
        pMetaResult->rMetaResult.appMeta.clear();
        mSubSampleCount4Clear = 0;
    }

    AAA_TRACE_END_HAL;

    MY_LOGD_IF(fgLog, "[%s] - pMetaResult:%p, clear metaResult(%d) SubSampleCount4Clear(%d)", __FUNCTION__, pMetaResult, bEnMetaClear, mSubSampleCount4Clear);
    AAA_TRACE_END_HAL;
    AAA_TRACE_END_D;
    return i4Ret;
}

MINT32
Hal3AAdapter3::
getCur(MUINT32 frmId, MetaSet_T& result)
{
    AAA_TRACE_HAL(AdapterGetCur);

    MBOOL fgLog = mu4LogEn & HAL3AADAPTER3_LOG_PF;
    MBOOL fgLogEn0 = mu4LogEn & HAL3AADAPTER3_LOG_GET_0;
    MBOOL fgLogEn1 = mu4LogEn & HAL3AADAPTER3_LOG_GET_1;

    MY_LOGD("[%s] sensorDev(%d), sensorIdx(%d) R(%d)", __FUNCTION__, mu4SensorDev, mi4SensorIdx, frmId);
    MINT32 i4Ret = 0;
    AllResult_T *pAllResult = NULL;
    MINT32 i4Validate= 0;
    MINT32 i4ReqMagic= -1;

    /*****************************
     *     get 2A CurResult from ResultPool
     *****************************/
    // ResultPool - 1. get result (dynamic) of x from 3AMgr
    i4Ret = mpHal3aObj->getResultCur(frmId);//req/stt:5/2
    // ResultPool - 2. Use sttMagic to get ResultCur
    if(1 == i4Ret)
    {
        i4ReqMagic = frmId;
        MY_LOGD("[%s] get R[%d]", __FUNCTION__, frmId);
        pAllResult = mpResultPoolObj->getAllResultCur(frmId);
        i4Validate = (mpResultPoolObj->isValidateCur(frmId, E_HAL_RESULTTOMETA) && mpResultPoolObj->isValidateCur(frmId, E_AE_RESULTTOMETA) /*&&
                      mpResultPoolObj->isValidateCur(frmId, E_AF_RESULTTOMETA)*/ && mpResultPoolObj->isValidateCur(frmId, E_AWB_RESULTTOMETA) &&
                      mpResultPoolObj->isValidateCur(frmId, E_LSC_RESULTTOMETA) && mpResultPoolObj->isValidateCur(frmId, E_FLASH_RESULTTOMETA) &&
                      mpResultPoolObj->isValidateCur(frmId, E_FLK_RESULTTOMETA) /*&& mpResultPoolObj->isValidateCur(frmId, E_ISP_RESULTTOMETA)*/);
    }

    /*****************************
     *     get special result
     *****************************/
    if(-1 == i4Ret)
    {
        // ResultPool - Get History
        MINT32 rHistoryReqMagic[HistorySize] = {0,0,0};
        mpResultPoolObj->getHistory(rHistoryReqMagic);
        i4ReqMagic = rHistoryReqMagic[2];
        MY_LOGW("[%s] History (Req0, Req1, Req2) = (#%d, #%d, #%d), Fail to get R[%d], current result will be obtained(%d).", __FUNCTION__, rHistoryReqMagic[0], rHistoryReqMagic[1], rHistoryReqMagic[2], frmId, rHistoryReqMagic[2]);
        pAllResult = mpResultPoolObj->getAllResultLastCur(rHistoryReqMagic[2]);//get the last request EX: req/stt:4/1
        i4Validate = (mpResultPoolObj->isValidate(rHistoryReqMagic[2], E_HAL_RESULTTOMETA) && mpResultPoolObj->isValidate(rHistoryReqMagic[2], E_AE_RESULTTOMETA) /*&&
                      mpResultPoolObj->isValidate(rHistoryReqMagic[2], E_AF_RESULTTOMETA)*/ && mpResultPoolObj->isValidate(rHistoryReqMagic[2], E_AWB_RESULTTOMETA) &&
                      mpResultPoolObj->isValidate(rHistoryReqMagic[2], E_LSC_RESULTTOMETA) && mpResultPoolObj->isValidate(rHistoryReqMagic[2], E_FLASH_RESULTTOMETA) &&
                      mpResultPoolObj->isValidate(rHistoryReqMagic[2], E_FLK_RESULTTOMETA) /*&& mpResultPoolObj->isValidate(rHistoryReqMagic[2], E_ISP_RESULTTOMETA)*/);
    }
    if(-2 == i4Ret)
    {
        i4ReqMagic = frmId;
        MY_LOGW("[%s] Fail to get R[%d], result will be obtained.", __FUNCTION__, frmId);
        pAllResult = mpResultPoolObj->getAllResult(frmId);//req/stt:2/X
        i4Validate = (mpResultPoolObj->isValidate(frmId, E_HAL_RESULTTOMETA) && mpResultPoolObj->isValidate(frmId, E_AE_RESULTTOMETA) /*&&
                      mpResultPoolObj->isValidate(frmId, E_AF_RESULTTOMETA)*/ && mpResultPoolObj->isValidate(frmId, E_AWB_RESULTTOMETA) &&
                      mpResultPoolObj->isValidate(frmId, E_LSC_RESULTTOMETA) && mpResultPoolObj->isValidate(frmId, E_FLASH_RESULTTOMETA) &&
                      mpResultPoolObj->isValidate(frmId, E_FLK_RESULTTOMETA) /*&& mpResultPoolObj->isValidate(frmId, E_ISP_RESULTTOMETA)*/);
    }

    /*****************************
     *     convert result to metadata and update metedata to MW
     *****************************/
    std::lock_guard<std::mutex> lock(mLock);
    std::lock_guard<std::mutex> lock_meta(mLockResult);
    if(i4Validate == MTRUE)
    {
        AllMetaResult_T *pMetaResult = mpResultPoolObj->getMetadata(frmId);
        if(pMetaResult == NULL)
            MY_LOGE("[%s] pMetaResult is NULL Req(#%d)", __FUNCTION__, frmId);
        else
        {
            mpResultPoolObj->convertToMetadata42A(pAllResult, pMetaResult);
            mpResultPoolObj->convertToMetadata4AF(pAllResult, pMetaResult);
            result = pMetaResult->rMetaResult;

            {
                std::lock_guard<std::mutex> lock(pMetaResult->LockMetaResult);
                MUINT8 u1CaptureIntent = 0;
                MUINT8 u1AwbState = 0;
                MUINT8 u1AeState = 0;
                MUINT8 u1FlashState = 0;
                MUINT8 u1AfState = 0;
                MBOOL ret = QUERY_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_CONTROL_CAPTURE_INTENT, u1CaptureIntent);
                QUERY_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_CONTROL_AWB_STATE, u1AwbState);
                QUERY_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_CONTROL_AE_STATE, u1AeState);
                QUERY_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_FLASH_STATE, u1FlashState);
                QUERY_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_CONTROL_AF_STATE, u1AfState);
                MY_LOGD("[%s] frmId(%d, %d), ret(%d), u1CaptureIntent(%d), count(%d, %d), AwbState(%d) AeState(%d) FlashState(%d) AfState(%d)", __FUNCTION__,
                            frmId, i4ReqMagic, ret, u1CaptureIntent, pMetaResult->rMetaResult.appMeta.count(), pMetaResult->rMetaResult.halMeta.count(),
                            u1AwbState, u1AeState, u1FlashState, u1AfState);
                if(!ret)
                {
                    MY_LOGW("[%s] Fail to get MTK_CONTROL_CAPTURE_INTENT in result(#%d)", __FUNCTION__, frmId);
                }
            }
        }
    }


    MY_LOGD("[%s] - Validate:%d", __FUNCTION__, i4Validate);
    AAA_TRACE_END_HAL;
    return 0;
}

MINT32
Hal3AAdapter3::
dumpIsp(MINT32 /*flowType*/, const MetaSet_T& control, TuningParam* pTuningBuf, MetaSet_T* /*pResult*/)
{
    MY_LOGE("[%s] HAL3A not support ISP API, need to use HALISP", __FUNCTION__);
    return MFALSE;
}

MINT32
Hal3AAdapter3::
setIsp(MINT32 flowType, const MetaSet_T& control, TuningParam* pTuningBuf, MetaSet_T* pResult)
{
    MY_LOGE("[%s] HAL3A not support ISP API, need to use HALISP", __FUNCTION__);
    return MFALSE;
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
getDelay(IMetadata& /*delay_info*/) const
{
    return 0;
}

MINT32
Hal3AAdapter3::
getDelay(MUINT32 /*tag*/) const
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
    MY_LOGD("[%s] mi4SensorMode = %d", __FUNCTION__, mi4SensorMode);
}

MVOID
Hal3AAdapter3::
notifyP1Done(MINT32 i4MagicNum, MVOID* pvArg)
{
    mpHal3aObj->notifyP1Done(i4MagicNum, pvArg);
}

MBOOL
Hal3AAdapter3::
notifyPwrOn()
{
    MY_LOGD("[%s]+ sensorDev(%d), sensorIdx(%d)", __FUNCTION__, mu4SensorDev, mi4SensorIdx);
    MBOOL i4Ret = mpHal3aObj->notifyPwrOn();
    MY_LOGD("[%s]- sensorDev(%d), sensorIdx(%d)", __FUNCTION__, mu4SensorDev, mi4SensorIdx);
    return i4Ret;
}


MBOOL
Hal3AAdapter3::
notifyPwrOff()
{
    std::lock_guard<std::mutex> lock(mLock);
    MY_LOGD("[%s]+ sensorDev(%d), sensorIdx(%d)", __FUNCTION__, mu4SensorDev, mi4SensorIdx);
    for (auto& it : m_Users)
    {
        MY_LOGD("[%s] sensor(%d)(%s)", __FUNCTION__, mu4SensorDev, it.first.c_str());
    }

/*        std::string strName = m_Users.keyAt(i);
        MUINT8 value = m_Users.valueAt(i);
        MY_LOGD_IF(value > 0, "[%s] sensor(%d)(%s, %d)", __FUNCTION__, mu4SensorDev, strName.c_str(), value);
    }*/

    //MY_LOGD_IF(value > 0, "[%s] sensor(%d)(%s, %d)", __FUNCTION__, mu4SensorDev, strName.c_str(), value);
    MBOOL i4Ret = mpHal3aObj->notifyPwrOff();
    MY_LOGD("[%s]- sensorDev(%d), sensorIdx(%d)", __FUNCTION__, mu4SensorDev, mi4SensorIdx);
    return i4Ret;
}

MBOOL
Hal3AAdapter3::
notifyP1PwrOn()
{
    MY_LOGD("[%s]+ notifyP1PwrOn sensorDev(%d), sensorIdx(%d)", __FUNCTION__, mu4SensorDev, mi4SensorIdx);
    MBOOL i4Ret = mpHal3aObj->notifyP1PwrOn();
    MY_LOGD("[%s]- sensorDev(%d), sensorIdx(%d)", __FUNCTION__, mu4SensorDev, mi4SensorIdx);
    return i4Ret;
}

MBOOL
Hal3AAdapter3::
notifyP1PwrOff()
{
    MY_LOGD("[%s]+ notifyP1PwrOff sensorDev(%d), sensorIdx(%d)", __FUNCTION__, mu4SensorDev, mi4SensorIdx);
    MBOOL i4Ret = mpHal3aObj->notifyP1PwrOff();
    MY_LOGD("[%s]- sensorDev(%d), sensorIdx(%d)", __FUNCTION__, mu4SensorDev, mi4SensorIdx);
    return i4Ret;
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
    std::lock_guard<std::mutex> lock(mLock);
    mpHal3aObj->setFDEnable(fgEnable);
}

MBOOL
Hal3AAdapter3::
setFDInfo(MVOID* prFaces)
{
    std::lock_guard<std::mutex> lock(mLock);
    if (!prFaces)   return MFALSE;
    MBOOL fdLogEn0 = (mu4LogEn & HAL3AADAPTER3_LOG_SET_0) ? MTRUE : MFALSE;
    MtkCameraFaceMetadata* rFaceMeta = (MtkCameraFaceMetadata *)prFaces;
    mAFFaceMeta.number_of_faces = 0;
    mAFFaceMeta.timestamp = rFaceMeta->timestamp;
    mAFFaceMeta.CNNFaces = rFaceMeta->CNNFaces;
    if(!mParams.bIsFDReady)
    {
        mParams.bIsFDReady = MTRUE;
        MY_LOGD("[%s] bIsFDReady(%d)", __FUNCTION__, mParams.bIsFDReady);
    }
    if(mPrvCropRegion.s.w != 0 && mPrvCropRegion.s.h != 0 && rFaceMeta->number_of_faces != 0) {
        // calculate face TG size
        int i = 0, j = 0;
        int img_w = rFaceMeta->ImgWidth;
        int img_h = rFaceMeta->ImgHeight;
        MRect ImgCrop;
        MINT32 i4TgWidth = 0;
        MINT32 i4TgHeight = 0;
        CameraArea_T rArea;
        mpHal3aObj->queryTgSize(i4TgWidth,i4TgHeight);

        if((mPrvCropRegion.s.w * img_h) > (mPrvCropRegion.s.h * img_w)) { // pillarbox
            ImgCrop.s.w = mPrvCropRegion.s.h * img_w / img_h;
            ImgCrop.s.h = mPrvCropRegion.s.h;
            ImgCrop.p.x = mPrvCropRegion.p.x + ((mPrvCropRegion.s.w - ImgCrop.s.w) >> 1);
            ImgCrop.p.y = mPrvCropRegion.p.y;
        } else if((mPrvCropRegion.s.w * img_h) < (mPrvCropRegion.s.h * img_w)) { // letterbox
            ImgCrop.s.w = mPrvCropRegion.s.w;
            ImgCrop.s.h = mPrvCropRegion.s.w * img_h / img_w;
            ImgCrop.p.x = mPrvCropRegion.p.x;
            ImgCrop.p.y = mPrvCropRegion.p.y + ((mPrvCropRegion.s.h - ImgCrop.s.h) >> 1);

        } else {
            ImgCrop.p.x = mPrvCropRegion.p.x;
            ImgCrop.p.y = mPrvCropRegion.p.y;
            ImgCrop.s.w = mPrvCropRegion.s.w;
            ImgCrop.s.h = mPrvCropRegion.s.h;
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

                // left eye up/down
                if(rFaceMeta->leyeux[i] == -1000 && rFaceMeta->leyeuy[i] == -1000
                    && rFaceMeta->leyedx[i] == -1000 && rFaceMeta->leyedy[i] == -1000)
                {
                    rArea.i4Left = rArea.i4Top = rArea.i4Right = rArea.i4Bottom = 0;
                }
                else
                {
                    rArea.i4Left = ((rFaceMeta->leyeux[i]+1000) * ImgCrop.s.w/2000) + ImgCrop.p.x;  //Left
                    rArea.i4Top = ((rFaceMeta->leyeuy[i]+1000) * ImgCrop.s.h/2000) + ImgCrop.p.y;  //Top
                    rArea.i4Right = ((rFaceMeta->leyedx[i]+1000) * ImgCrop.s.w/2000) + ImgCrop.p.x;  //Right
                    rArea.i4Bottom = ((rFaceMeta->leyedy[i]+1000) * ImgCrop.s.h/2000) + ImgCrop.p.y;  //Bottom
                    rArea = _transformArea(mi4SensorIdx,mi4SensorMode,rArea);
                    rArea = _clipArea(i4TgWidth,i4TgHeight,rArea);
                }
                mAFFaceMeta.leyeux[i] = rArea.i4Left;
                mAFFaceMeta.leyeuy[i] = rArea.i4Top;
                mAFFaceMeta.leyedx[i] = rArea.i4Right;
                mAFFaceMeta.leyedy[i] = rArea.i4Bottom;

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

                // right eye up/down
                if(rFaceMeta->reyeux[i] == -1000 && rFaceMeta->reyeuy[i] == -1000
                    && rFaceMeta->reyedx[i] == -1000 && rFaceMeta->reyedy[i] == -1000)
                {
                    rArea.i4Left = rArea.i4Top = rArea.i4Right = rArea.i4Bottom = 0;
                }
                else
                {
                    rArea.i4Left = ((rFaceMeta->reyeux[i]+1000) * ImgCrop.s.w/2000) + ImgCrop.p.x;  //Left
                    rArea.i4Top = ((rFaceMeta->reyeuy[i]+1000) * ImgCrop.s.h/2000) + ImgCrop.p.y;  //Top
                    rArea.i4Right = ((rFaceMeta->reyedx[i]+1000) * ImgCrop.s.w/2000) + ImgCrop.p.x;  //Right
                    rArea.i4Bottom = ((rFaceMeta->reyedy[i]+1000) * ImgCrop.s.h/2000) + ImgCrop.p.y;  //Bottom
                    rArea = _transformArea(mi4SensorIdx,mi4SensorMode,rArea);
                    rArea = _clipArea(i4TgWidth,i4TgHeight,rArea);
                }
                mAFFaceMeta.reyeux[i] = rArea.i4Left;
                mAFFaceMeta.reyeuy[i] = rArea.i4Top;
                mAFFaceMeta.reyedx[i] = rArea.i4Right;
                mAFFaceMeta.reyedy[i] = rArea.i4Bottom;

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
                // nose
                rArea.i4Left = ((rFaceMeta->nosex[i]+1000) * ImgCrop.s.w/2000) + ImgCrop.p.x;  //Left
                rArea.i4Top = ((rFaceMeta->nosey[i]+1000) * ImgCrop.s.h/2000) + ImgCrop.p.y;  //Top
                rArea.i4Right = ((rFaceMeta->nosex[i]+1000) * ImgCrop.s.w/2000) + ImgCrop.p.x;  //Right
                rArea.i4Bottom = ((rFaceMeta->nosey[i]+1000) * ImgCrop.s.h/2000) + ImgCrop.p.y;  //Bottom
                rArea = _transformArea(mi4SensorIdx,mi4SensorMode,rArea);
                rArea = _clipArea(i4TgWidth,i4TgHeight,rArea);
                mAFFaceMeta.nosex[i] = rArea.i4Left;
                mAFFaceMeta.nosey[i] = rArea.i4Top;
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
setFDInfoOnActiveArray(MVOID* prFaces)
{
    std::lock_guard<std::mutex> lock(mLock);
    if (!prFaces)   return MFALSE;
    MBOOL fdLogEn0 = (mu4LogEn & HAL3AADAPTER3_LOG_SET_0) ? MTRUE : MFALSE;
    MtkCameraFaceMetadata* rFaceMeta = (MtkCameraFaceMetadata *)prFaces;
    mAFFaceMeta.number_of_faces = 0;
    mAFFaceMeta.timestamp = rFaceMeta->timestamp;
    mAFFaceMeta.CNNFaces = rFaceMeta->CNNFaces;
    if(!mParams.bIsFDReady)
    {
        mParams.bIsFDReady = MTRUE;
        MY_LOGD("[%s] bIsFDReady(%d)", __FUNCTION__, mParams.bIsFDReady);
    }
    if (rFaceMeta->number_of_faces != 0)
    {
        // calculate face TG size
        int i = 0;
        MINT32 i4TgWidth = 0;
        MINT32 i4TgHeight = 0;
        CameraArea_T rArea;
        mpHal3aObj->queryTgSize(i4TgWidth,i4TgHeight);

        mAFFaceMeta.number_of_faces = rFaceMeta->number_of_faces;
        mAFFaceMeta.landmarkNum = rFaceMeta->landmarkNum;
        mAFFaceMeta.genderNum = rFaceMeta->genderNum;
        mAFFaceMeta.poseNum = rFaceMeta->poseNum;
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

                // left eye up/down
                if(rFaceMeta->leyeux[i] == -1000 && rFaceMeta->leyeuy[i] == -1000
                    && rFaceMeta->leyedx[i] == -1000 && rFaceMeta->leyedy[i] == -1000)
                {
                    rArea.i4Left = rArea.i4Top = rArea.i4Right = rArea.i4Bottom = 0;
                }
                else
                {
                    rArea.i4Left = rFaceMeta->leyeux[i];   //Left
                    rArea.i4Top = rFaceMeta->leyeuy[i];    //Top
                    rArea.i4Right = rFaceMeta->leyedx[i];  //Right
                    rArea.i4Bottom = rFaceMeta->leyedy[i]; //Bottom
                    rArea = _transformArea(mi4SensorIdx,mi4SensorMode,rArea);
                    rArea = _clipArea(i4TgWidth,i4TgHeight,rArea);
                }
                mAFFaceMeta.leyeux[i] = rArea.i4Left;
                mAFFaceMeta.leyeuy[i] = rArea.i4Top;
                mAFFaceMeta.leyedx[i] = rArea.i4Right;
                mAFFaceMeta.leyedy[i] = rArea.i4Bottom;

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

                // right eye up/down
                if(rFaceMeta->reyeux[i] == -1000 && rFaceMeta->reyeuy[i] == -1000
                    && rFaceMeta->reyedx[i] == -1000 && rFaceMeta->reyedy[i] == -1000)
                {
                    rArea.i4Left = rArea.i4Top = rArea.i4Right = rArea.i4Bottom = 0;
                }
                else
                {
                    rArea.i4Left = rFaceMeta->reyeux[i];   //Left
                    rArea.i4Top = rFaceMeta->reyeuy[i];    //Top
                    rArea.i4Right = rFaceMeta->reyedx[i];  //Right
                    rArea.i4Bottom = rFaceMeta->reyedy[i]; //Bottom
                    rArea = _transformArea(mi4SensorIdx,mi4SensorMode,rArea);
                    rArea = _clipArea(i4TgWidth,i4TgHeight,rArea);
                }
                mAFFaceMeta.reyeux[i] = rArea.i4Left;
                mAFFaceMeta.reyeuy[i] = rArea.i4Top;
                mAFFaceMeta.reyedx[i] = rArea.i4Right;
                mAFFaceMeta.reyedy[i] = rArea.i4Bottom;


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

                // nose
                rArea.i4Left = rFaceMeta->nosex[i];    //Left
                rArea.i4Top = rFaceMeta->nosey[i];     //Top
                rArea.i4Right = rFaceMeta->nosex[i];   //Right
                rArea.i4Bottom = rFaceMeta->nosey[i];  //Bottom
                rArea = _transformArea(mi4SensorIdx,mi4SensorMode,rArea);
                rArea = _clipArea(i4TgWidth,i4TgHeight,rArea);
                mAFFaceMeta.nosex[i] = rArea.i4Left;
                mAFFaceMeta.nosey[i] = rArea.i4Top;
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
            mAFFaceMeta.fld_GenderInfo[i]  = rFaceMeta->fld_GenderInfo[i];
            mAFFaceMeta.GenderLabel[i]     = rFaceMeta->GenderLabel[i];
            mAFFaceMeta.oGenderLabel[i]    = rFaceMeta->oGenderLabel[i];
            mAFFaceMeta.GenderCV[i]        = rFaceMeta->GenderCV[i];
            mAFFaceMeta.RaceLabel[i]       = rFaceMeta->RaceLabel[i];

            MY_LOGD_IF(fdLogEn0, "[%s] face[%d] rect info(%d,%d,%d,%d)", __FUNCTION__, i, rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom);
        }
    }
    memcpy(&(mAFFaceMeta.YUVsts), &(rFaceMeta->YUVsts), sizeof(mAFFaceMeta.YUVsts));
    memcpy(&(mAFFaceMeta.RaceCV), &(rFaceMeta->RaceCV), sizeof(mAFFaceMeta.RaceCV));
    memcpy(&(mAFFaceMeta.poseinfo), &(rFaceMeta->poseinfo), sizeof(mAFFaceMeta.poseinfo));
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
    std::lock_guard<std::mutex> lock(mLock);
    MBOOL fdLogEn0 = (mu4LogEn & HAL3AADAPTER3_LOG_SET_0) ? MTRUE : MFALSE;
    MtkCameraFaceMetadata* rOTMeta = (MtkCameraFaceMetadata *)prOT;
    mAFOTMeta.number_of_faces = 0;
    if(mPrvCropRegion.s.w != 0 && mPrvCropRegion.s.h != 0 && rOTMeta->number_of_faces != 0) {
        // calculate face TG size
        int i = 0;
        int img_w = rOTMeta->ImgWidth;
        int img_h = rOTMeta->ImgHeight;
        MRect ImgCrop;
        MINT32 i4TgWidth = 0;
        MINT32 i4TgHeight = 0;
        CameraArea_T rArea;
        mpHal3aObj->queryTgSize(i4TgWidth,i4TgHeight);
        if((mPrvCropRegion.s.w * img_h) > (mPrvCropRegion.s.h * img_w)) { // pillarbox
            ImgCrop.s.w = mPrvCropRegion.s.h * img_w / img_h;
            ImgCrop.s.h = mPrvCropRegion.s.h;
            ImgCrop.p.x = mPrvCropRegion.p.x + ((mPrvCropRegion.s.w - ImgCrop.s.w) >> 1);
            ImgCrop.p.y = mPrvCropRegion.p.y;
        } else if((mPrvCropRegion.s.w * img_h) < (mPrvCropRegion.s.h * img_w)) { // letterbox
            ImgCrop.s.w = mPrvCropRegion.s.w;
            ImgCrop.s.h = mPrvCropRegion.s.w * img_h / img_w;
            ImgCrop.p.x = mPrvCropRegion.p.x;
            ImgCrop.p.y = mPrvCropRegion.p.y + ((mPrvCropRegion.s.h - ImgCrop.s.h) >> 1);

        } else {
            ImgCrop.p.x = mPrvCropRegion.p.x;
            ImgCrop.p.y = mPrvCropRegion.p.y;
            ImgCrop.s.w = mPrvCropRegion.s.w;
            ImgCrop.s.h = mPrvCropRegion.s.h;
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
    if(e3ACtrl == E3ACtrl_GetSupportedInfo)
    {
        MINT32 i4Ret = MFALSE;
        //get FeatureParam from NVRAM
        {
            std::lock_guard<std::mutex> lockConfigSend3ACtrl(mLockConfigSend3ACtrl);
            MY_LOGD("[%s] GetSupportedInfo mpHal3aObj(%p)", __FUNCTION__, mpHal3aObj);
            i4Ret = mpHal3aObj->send3ACtrl(e3ACtrl, i4Arg1, i4Arg2);
        }

        // re-modify FocusLength from statistic metadata
        MBOOL bRet = getStaticMeta();
        MFLOAT fFocusLength = 0.0f;
        if(bRet)
        {
            if(!QUERY_ENTRY_SINGLE(mMetaStaticInfo, MTK_LENS_INFO_AVAILABLE_FOCAL_LENGTHS , fFocusLength))
                MY_LOGD("[%s] FocusLength QUERY fail", __FUNCTION__);
        }
        FeatureParam_T* rFeatureParam;
        rFeatureParam = reinterpret_cast<FeatureParam_T*>(i4Arg1);
        // Format: sensor:338=>FL 4.52f, FL multiplication 100, because MW will divide 100.
        // Format: sensor:4e6=>FL 2.37f, FL multiplication 100, because MW will divide 100. => use roundf, because 2.37*100 = 236.9999999
        rFeatureParam->u4FocusLength_100x = (MUINT32)(std::roundf(fFocusLength*100));

        MY_LOGD("[%s] (i4Ret, bRet)=(%d, %d) fFocusLength = %f, %d", __FUNCTION__, i4Ret, bRet, fFocusLength, rFeatureParam->u4FocusLength_100x);
        return i4Ret;
    }
    else if(e3ACtrl == E3ACtrl_GetFlashCapDummyCnt)
    {
        rHAL3AFlashCapDummyInfo_T* pFlashCapDummyInfo = reinterpret_cast<rHAL3AFlashCapDummyInfo_T*>(i4Arg1);

        MUINT32 u4AeMode = 0;
        MUINT32 u4StrobeMode = 0;
        MUINT32 u4CaptureIntent = 1;
        MUINT32 cntBefore= 0;
        MUINT32 cntAfter = 0;
        MUINT8 u1SubFlashCustomization = 0;
        MINT32 i4FlashOnCapture = 0;
        if (pFlashCapDummyInfo)
        {
            u4AeMode = pFlashCapDummyInfo->u4AeMode;
            u4StrobeMode = pFlashCapDummyInfo->u4StrobeMode;
            u4CaptureIntent = pFlashCapDummyInfo->u4CaptureIntent;
        }
        mpHal3aObj->send3ACtrl(E3ACtrl_GetIsFlashOnCapture, reinterpret_cast<MINTPTR>(&i4FlashOnCapture), NULL);

        if ((u4StrobeMode == MTK_FLASH_MODE_SINGLE ||
           u4AeMode == MTK_CONTROL_AE_MODE_ON_ALWAYS_FLASH ||
           (u4AeMode == MTK_CONTROL_AE_MODE_ON_AUTO_FLASH && i4FlashOnCapture == 1)) && u4CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_STILL_CAPTURE)
        {
            #ifdef CUST_FLASH_DUMMY_CNT_BEFORE
                cntBefore = static_cast<MUINT32>(CUST_FLASH_DUMMY_CNT_BEFORE);
            #else
                cntBefore = 2;
            #endif

            #ifdef CUST_FLASH_DUMMY_CNT_AFTER
                cntAfter = static_cast<MUINT32>(CUST_FLASH_DUMMY_CNT_AFTER);
            #else
                cntAfter = 3;
            #endif
        }

        MY_LOGD("[%s] E3ACtrl_FlashCapDummyCnt cntBefore(%d), cntAfter(%d) ", __FUNCTION__, cntBefore, cntAfter);

        if (QUERY_ENTRY_SINGLE(mMetaStaticInfo, MTK_FLASH_FEATURE_CUSTOMIZATION_AVAILABLE, u1SubFlashCustomization)){
            if(u1SubFlashCustomization == 1 && (mu4SensorDev == ESensorDev_Sub || mu4SensorDev == ESensorDev_SubSecond)){
                cntBefore = 0;
                cntAfter = 0;
                MY_LOGW("[%s] sensordev(%d) no dummy for customized panel flash.", __FUNCTION__, mu4SensorDev);
            }
        }

        if (i4Arg2)
        {
            rHAL3AFlashCapDummyCnt_T* pFlashCapDummyCnt = reinterpret_cast<rHAL3AFlashCapDummyCnt_T*>(i4Arg2);
            pFlashCapDummyCnt->u4CntBefore = cntBefore;
            pFlashCapDummyCnt->u4CntAfter = cntAfter;
        }

        return 0;
    }
#if CAM3_STEREO_FEATURE_EN
    else if(e3ACtrl == E3ACtrl_GetAAOIsReady)
    {
        MBOOL bAAOIsReady = mpHal3aObj->queryAaoIsReady();
        CAM_LOGD("[%s] sensordev(%d) bAAOIsReady(%d).", __FUNCTION__, mu4SensorDev, bAAOIsReady);
        *(reinterpret_cast<MBOOL*>(i4Arg1)) = bAAOIsReady;
        return 0;
    }
    else if (e3ACtrl == E3ACtrl_SetSync3ADevDoSwitch)
    {
        MINT32 i4Sync3ADevDoSwitch = static_cast<MINT32>(i4Arg1);
        CAM_LOGW("[%s] Dev(%u) Sync3ADevDoSwitch(%d)", __FUNCTION__, mu4SensorDev, i4Sync3ADevDoSwitch);
        if (i4Sync3ADevDoSwitch)
        {
            m_i4IsByPassCal = MTRUE;
        }

        mpHal3aObj->send3ACtrl(e3ACtrl, i4Arg1, i4Arg2);
        return 0;
    }
#endif
    else
    {
        MINT32 i4Ret = MFALSE;
        if(mpHal3aObj)
            i4Ret = mpHal3aObj->send3ACtrl(e3ACtrl, i4Arg1, i4Arg2);
        return i4Ret;
    }
}

void
Hal3AAdapter3::
doNotifyCb(MINT32 _msgType, MINTPTR _ext1, MINTPTR _ext2, MINTPTR _ext3)
{
    switch (_msgType)
    {
    case eID_NOTIFY_3APROC_FINISH:
        MY_LOGD_IF((mu4LogEn & HAL3AADAPTER3_LOG_PF), "[%s] _msgType(%d), _ext1(%d), _ext2(%d), _ext3(%d)", __FUNCTION__, _msgType, (MINT32)_ext1, (MINT32)_ext2, (MINT32)_ext3);
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
    {
        std::lock_guard<std::mutex> _lock(mutexParseMeta);
        m_CbSet[IHal3ACb::eID_NOTIFY_VSYNC_DONE].doNotifyCb(IHal3ACb::eID_NOTIFY_VSYNC_DONE, _ext1, _ext2, _ext3);
    }
        break;
    case eID_NOTIFY_LCS_ISP_PARAMS:
        m_CbSet[IHal3ACb::eID_NOTIFY_LCS_ISP_PARAMS].doNotifyCb(IHal3ACb::eID_NOTIFY_LCS_ISP_PARAMS, _ext1, _ext2, _ext3);
        break;
    case eID_NOTIFY_AF_FSC_INFO:
        m_CbSet[IHal3ACb::eID_NOTIFY_AF_FSC_INFO].doNotifyCb(IHal3ACb::eID_NOTIFY_AF_FSC_INFO, _ext1, _ext2, _ext3);
        break;
    default:
        break;
    }
}

void
Hal3AAdapter3::
doDataCb(int32_t /*_msgType*/, void* /*_data*/, uint32_t /*_size*/)
{
    return ;
}

MVOID
Hal3AAdapter3::
parseConfig(const ConfigInfo_T& rConfigInfo, ConfigInfo_T& rConfigInfoOutPut)
{
    rConfigInfoOutPut = rConfigInfo;
    MUINT8 u1MtkHdrMode = static_cast<MUINT8>(HDRMode::OFF);
    QUERY_ENTRY_SINGLE(rConfigInfo.CfgHalMeta, MTK_3A_HDR_MODE, u1MtkHdrMode);

    // AE Auto HDR, 0:OFF 1:ON 2:AUTO
    switch(static_cast<HDRMode>(u1MtkHdrMode))
    {
        case HDRMode::VIDEO_ON:
             mParams.u1HdrMode = 1;
             break;
        case HDRMode::VIDEO_AUTO:
             mParams.u1HdrMode = 2;
             break;
        case HDRMode::ON:
        case HDRMode::OFF:
        case HDRMode::AUTO:
        default:
             mParams.u1HdrMode = 0;
             break;
    }

    MUINT8 u1IspProfile = 0x0;
    QUERY_ENTRY_SINGLE(rConfigInfo.CfgHalMeta, MTK_3A_ISP_PROFILE, u1IspProfile);    // NSIspTuning::EIspProfile_Preview
    mParams.eIspProfile = static_cast<NSIspTuning::EIspProfile_T>(u1IspProfile);

    NSCam::IMetadata::IEntry entry = rConfigInfo.CfgAppMeta.entryFor(MTK_CONTROL_AE_TARGET_FPS_RANGE);
    if (entry.tag() != NSCam::IMetadata::IEntry::BAD_TAG)
    {
        mParams.i4MinFps = entry.itemAt(0, NSCam::Type2Type< MINT32 >()) * 1000;
        mParams.i4MaxFps = entry.itemAt(1, NSCam::Type2Type< MINT32 >()) * 1000;
    }

#if CAM3_STEREO_FEATURE_EN
    QUERY_ENTRY_SINGLE(rConfigInfo.CfgAppMeta, MTK_MULTI_CAM_FEATURE_MODE, rConfigInfoOutPut.i4CamFeatureMode);
#endif

    MY_LOGD("[%s] Config Params: HDR mode %d, IspProf %d, MinFps %d, MaxFps %d CamFeatureMode %d"
        , __FUNCTION__, mParams.u1HdrMode, mParams.eIspProfile, mParams.i4MinFps, mParams.i4MaxFps, rConfigInfoOutPut.i4CamFeatureMode);

    return ;
}

//====================
Hal3ACbSet::
Hal3ACbSet()
    : m_Mutex()
{
    std::lock_guard<std::mutex> autoLock(m_Mutex);

    m_CallBacks.clear();
}

Hal3ACbSet::
~Hal3ACbSet()
{
    std::lock_guard<std::mutex> autoLock(m_Mutex);

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
    std::lock_guard<std::mutex> autoLock(m_Mutex);

    //MY_LOGD("[Hal3ACbSet::%s] _msgType(%d), _ext1(%d), _ext2(%d), _ext3(%d)", __FUNCTION__, _msgType, _ext1, _ext2, _ext3);
    for (std::list<IHal3ACb*>::iterator it = m_CallBacks.begin(); it != m_CallBacks.end(); it++)
    {
        IHal3ACb* pCb = *it;
        //MY_LOGD("[%s] pCb(%p)", __FUNCTION__, pCb);

        // No need to verify, due to null check has been performed previously
        if(pCb)
            pCb->doNotifyCb(_msgType, _ext1, _ext2, _ext3);
        else
            MY_LOGW("Hal3ACbSet::[%s] pCb(%p)", __FUNCTION__, pCb);
    }
}

MINT32
Hal3ACbSet::
addCallback(IHal3ACb* cb)
{
    std::lock_guard<std::mutex> autoLock(m_Mutex);

    if (!cb)
    {
        MY_LOGE("[%s] NULL callback!", __FUNCTION__);
        return -m_CallBacks.size();
    }

    for (std::list<IHal3ACb*>::iterator it = m_CallBacks.begin(); it != m_CallBacks.end(); it++)
    {
        if (cb == *it)
        {
            MY_LOGD("[%s] Callback already exists!", __FUNCTION__);
            return m_CallBacks.size();
        }
    }

    m_CallBacks.push_back(cb);
    return m_CallBacks.size();

}

MINT32
Hal3ACbSet::
removeCallback(IHal3ACb* cb)
{
    std::lock_guard<std::mutex> autoLock(m_Mutex);

    if (!cb)
    {
        MY_LOGE("[%s] NULL callback!", __FUNCTION__);
        return -m_CallBacks.size();
    }

    for (std::list<IHal3ACb*>::iterator it = m_CallBacks.begin(); it != m_CallBacks.end(); it++)
    {
        if (cb == *it)
        {
            m_CallBacks.erase(it);
            return m_CallBacks.size();
        }
    }

    // cannot be found
    MY_LOGE("[%s] No such callback, remove failed", __FUNCTION__);
    return -m_CallBacks.size();
}

