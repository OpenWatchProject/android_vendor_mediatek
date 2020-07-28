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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef STEREO_TUNING_COMMON_H_
#define STEREO_TUNING_COMMON_H_

#include <vector>   //for std::vector
#include <utility>  //for std::pair

#include <camera_custom_stereo.h>
#include <mtkcam3/feature/stereo/hal/stereo_common.h>
#include <mtkcam3/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam3/feature/stereo/hal/stereo_size_provider.h>
#include <mtkcam3/feature/stereo/hal/StereoArea.h>

#define PERPERTY_TUNING_PROVIDER_LOG    "vendor.STEREO.log.tuning"

// Common & SW part
#define QUERY_KEY_SCENARIO              "Scenario"
#define QUERY_KEY_TUNING_PARAMS         "TuningParams"

// OCC
#define QUERY_KEY_CORE_NUMBER           "CoreNumber"
#define QUERY_KEY_OCC_CORE_NUMBER       "OCC_CoreNumber"

// SW Bokeh
#define QUERY_KEY_APP                   "APP"
#define QUERY_KEY_CAMERA                "Camera"
#define QUERY_KEY_GALLERY               "Gallery"

// GF&SW Bokeh
#define QUERY_KEY_DOF_LEVEL             "DoFLevel"  //input
#define QUERY_KEY_DOF_VALUE             "DoFValue"  //output
#define QUERY_KEY_DOF_TABLE_PREFIX      "DoFTable"
#define QUERY_KEY_CLEAR_RANGE_TABLE     "ClearRangeTable"
#define QUERY_KEY_CLEAR_TABLE_PREFIX    "ClearTable"

// GF
#define QUERY_KEY_GF_CORE_NUMBER        "GF_CoreNumber"
#define QUERY_KEY_DISPCTRLPOINTS        "dispCtrlPoints"
#define QUERY_KEY_BLURGAINTABLE         "blurGainTable"

// OCC&WMF
#define DEPTH_MASK_VALUE (255)

// N3D
#define QUERY_KEY_MASK_MARGIN           "MaskMargin"
#define QUERY_KEY_LEFT                  "Left"
#define QUERY_KEY_RIGHT                 "Right"
#define QUERY_KEY_TOP                   "Top"
#define QUERY_KEY_BOTTOM                "Bottom"

namespace StereoHAL {

enum ENUM_STEREO_TUNING
{
    E_TUNING_HW_MDPPQ,

    E_TUNING_HW_FE,
    E_TUNING_HW_FM,
    E_TUNING_HW_DPE,
#if (0==HAS_HW_DPE)
#if (1==HAS_HW_OCC)
    E_TUNING_HW_OCC,
#endif  //(1==HAS_HW_OCC)
    E_TUNING_HW_WMF,
#else
    E_TUNING_HW_DPE_DVS_CTRL,
    E_TUNING_HW_DPE_DVP_CTRL,
    E_TUNING_HW_DPE_DVS_ME,
    E_TUNING_HW_DPE_DVS_OCC,
#endif  //(0==HAS_HW_DPE)
    E_TUNING_HW_BOKEH,
#if (1==HAS_ISP_FE_TUNING)
    E_TUNING_HW_ISP_FE,
#endif

    E_TUNING_SW_N3D,
    E_TUNING_SW_GF,
    E_TUNING_SW_OCC,
    E_TUNING_SW_BOKEH,
#if (1==HAS_AIDEPTH)
    E_TUNING_SW_AIDEPTH,
    E_TUNING_SW_VIDEO_AIDEPTH,
#endif
};

static const char *STEREO_TUNING_NAME[] =
{
    "HW_MDPPQ",

    "HW_FE",
    "HW_FM",
    "HW_DPE",
#if (0==HAS_HW_DPE)
#if (1==HAS_HW_OCC)
    "HW_OCC",
#endif  //(1==HAS_HW_OCC)
    "HW_WMF",
#else
    "HW_DPE_DVS_CTRL",
    "HW_DPE_DVP_CTRL",
    "HW_DPE_DVS_ME",
    "HW_DPE_DVS_OCC",
#endif  //(0==HAS_HW_DPE)
    "HW_BOKEH",
#if (1==HAS_ISP_FE_TUNING)
    "HW_ISP_FE",
#endif

    "SW_N3D",
    "SW_GF",
    "SW_OCC",
    "SW_BOKEH",
#if (1==HAS_AIDEPTH)
    "SW_AIDEPTH",
    "SW_VIDEO_AIDEPTH",
#endif
};

enum ENUM_BOKEH_STRENGTH
{
    E_BOKEH_STRENGTH_WEAK,
    E_BOKEH_STRENGTH_NORMAL,
    E_BOKEH_STRENGTH_STRONG,
    TOTAL_BOKEH_STRENGTH
};

enum ENUM_FM_DIRECTION
{
    E_FM_R_TO_L,
    E_FM_L_TO_R
};

enum ENUM_WMF_ROUND
{
    E_WMF_ROUND,
    E_HOLE_FILLING_ROUND_1,
    E_HOLE_FILLING_ROUND_2,
    TOTAL_WMF_ROUND
};

enum ENUM_CLEAR_REGION  //for GF
{
    E_CLEAR_REGION_SMALL  = 0,
    E_CLEAR_REGION_MEDIUM = 1,
    E_CLEAR_REGION_LARGE  = 2,
};

typedef std::map<std::string, int> QUERY_INT_T;
typedef std::map<std::string, const char *> QUERY_STR_T;
typedef std::map<std::string, void *> QUERY_BUFFER_T;
typedef std::vector<std::pair<std::string, int>> TUNING_PAIR_LIST_T;
typedef std::vector<int>    QUERY_INT_LIST_T;

struct TuningQuery_T
{
    QUERY_INT_T intParams;
    QUERY_STR_T strParams;
    QUERY_BUFFER_T  results;
};

typedef MUINT32 MARGIN_T;
#define DEFAULT_MARGIN (4)
struct StereoImageMargin
{
    MARGIN_T left   = DEFAULT_MARGIN;
    MARGIN_T top    = DEFAULT_MARGIN;
    MARGIN_T right  = DEFAULT_MARGIN;
    MARGIN_T bottom = DEFAULT_MARGIN;

    StereoImageMargin() {}

    StereoImageMargin(const MARGIN_T l, const MARGIN_T t, const MARGIN_T r, const MARGIN_T b)
    {
        left   = l;
        top    = t;
        right  = r;
        bottom = b;
    }

    StereoImageMargin(const MARGIN_T m)
    {
        left = top = right = bottom = m;
    }

    StereoImageMargin(const StereoArea &area)
    {
        left   = area.startPt.x + 1;
        top    = area.startPt.y + 1;
        right  = area.padding.w - left;
        bottom = area.padding.h - top;
    }

    StereoArea toStereoArea(const NSCam::MSize size) const
    {
        StereoArea area;
        area.size      = size;
        area.startPt.x = left;
        area.startPt.y = top;
        area.padding.w = left + right;
        area.padding.h = top + bottom;
        return area;
    }

    void applyToStereoArea(StereoArea &area) const
    {
        area.startPt.x += left;
        area.startPt.y += top;
        area.padding.w += (left + right);
        area.padding.h += (top + bottom);
    }
};

};  //namespace StereoHAL
#endif