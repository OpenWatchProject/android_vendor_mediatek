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
#ifndef STEREO_TUNING_DEF_H_
#define STEREO_TUNING_DEF_H_

#include <vector>   //for std::vector
#include <utility>  //for std::pair

#include <camera_custom_stereo.h>
#include <mtkcam/feature/stereo/hal/stereo_common.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreorder"
#include <mtkcam/drv/def/dpecommon.h>
#include <mtkcam/drv/iopipe/PostProc/IHalPostProcPipe.h>
#include <drv/isp_reg.h>
#pragma GCC diagnostic pop

#include <mtkcam/feature/stereo/hal/pass2_size_data.h>
#include <DpDataType.h>

#define PERPERTY_TUNING_PROVIDER_LOG  "vendor.STEREO.log.tuning"

#define QUERY_KEY_SCENARIO        "Scenario"
#define QUERY_KEY_TUNING_PARAMS   "TuningParams"

#define QUERY_KEY_WMF_ROUND       "Round"
#define QUERY_KEY_WMFECTRL        "WMFECtrl"
#define QUERY_KEY_WMF_TABLE       "tblis"

#define QUERY_KEY_HWFE_BLOCKSIZE  "BlockSize"
#define QUERY_KEY_HWFE_INFO       "FEInfo"

#define QUERY_KEY_HWFM_DIRECTION  "Direction"
#define QUERY_KEY_HWFM_INFO       "FMInfo"

#define QUERY_KEY_HWBOKEH_STRENTH "Strength"
#define QUERY_KEY_HWBOKEH_INFO    "HWBokehInfo"

#define QUERY_KEY_SWGF_TABLE      "ClearRangeTable"

#define QUERY_KEY_CORE_NUMBER     "CoreNumber"

#define QUERY_KEY_MDP_PQ_NAME     "MDP_PQ_Name"
#define QUERY_KEY_MDP_PQ_INFO     "MDPPQInfo"

enum ENUM_CLEAR_REGION  //for GF
{
    E_CLEAR_REGION_SMALL  = 0,
    E_CLEAR_REGION_MEDIUM = 1,
    E_CLEAR_REGION_LARGE  = 2,
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

enum ENUM_STEREO_TUNING
{
    E_TUNING_HW_MDPPQ,

    E_TUNING_HW_FE,
    E_TUNING_HW_FM,
    E_TUNING_HW_DPE,
    E_TUNING_HW_WMF,
    E_TUNING_HW_BOKEH,

    E_TUNING_SW_GF,
    E_TUNING_SW_OCC,
    E_TUNING_SW_BOKEH,
    E_TUNING_SW_REFOCUS,
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

#endif