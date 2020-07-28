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
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "StereoTuningProvider"

#include <sys/stat.h>
#include <unistd.h>

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#define CAM_ULOG_MODULE_ID MOD_MULTICAM_PROVIDER
CAM_ULOG_DECLARE_MODULE_ID(CAM_ULOG_MODULE_ID);
#include <stereo_tuning_provider.h>
#include <mtkcam3/feature/stereo/hal/stereo_common.h>
#include <mtkcam3/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam3/feature/stereo/hal/stereo_size_provider.h>
#include "stereo_tuning_provider_kernel.h"

using namespace NSCam;
using namespace NSCam::NSIoPipe;
using namespace StereoHAL;

#define STEREO_TUNING_PROVIDER_DEBUG

#ifdef STEREO_TUNING_PROVIDER_DEBUG    // Enable debug log.

#undef __func__
#define __func__ __FUNCTION__

#define MY_LOGD(fmt, arg...)    CAM_ULOGMD("[%s]" fmt, __func__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_ULOGMI("[%s]" fmt, __func__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_ULOGMW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_ULOGME("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

#else   // Disable debug log.
#define MY_LOGD(a,...)
#define MY_LOGI(a,...)
#define MY_LOGW(a,...)
#define MY_LOGE(a,...)
#endif  // STEREO_TUNING_PROVIDER_DEBUG

#define MY_LOGV_IF(cond, arg...)    if (cond) { MY_LOGV(arg); }
#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }

bool
StereoTuningProvider::getGFTuningInfo(MUINT32 &coreNumber,
                                      GF_TUNING_T &tuningTable,
                                      GF_TUNING_T &dispCtrlPoints,
                                      GF_TUNING_T &blurGainTable,
                                      TUNING_PAIR_LIST_T &tuningParams,
                                      ENUM_STEREO_SCENARIO eScenario)
{
    TuningQuery_T query;
    query.intParams[QUERY_KEY_SCENARIO] = eScenario;

    query.results[QUERY_KEY_CORE_NUMBER]       = &coreNumber;
    query.results[QUERY_KEY_CLEAR_RANGE_TABLE] = &tuningTable;
    query.results[QUERY_KEY_DISPCTRLPOINTS]    = &dispCtrlPoints;
    query.results[QUERY_KEY_BLURGAINTABLE]     = &blurGainTable;
    query.results[QUERY_KEY_TUNING_PARAMS]     = &tuningParams;

    return StereoTuningProviderKernel::getInstance()->getTuningParams(E_TUNING_SW_GF, query);
}

MUINT32
StereoTuningProvider::getGFDoFValue(const MUINT32 dofLevel, ENUM_STEREO_SCENARIO eScenario)
{
    TuningQuery_T query;
    int dofValue = dofLevel * 2;    //default

    //Input
    query.intParams[QUERY_KEY_SCENARIO] = eScenario;
    query.intParams[QUERY_KEY_DOF_LEVEL] = dofLevel;
    //Output
    query.results[QUERY_KEY_DOF_VALUE] = &dofValue;

    StereoTuningProviderKernel::getInstance()->getTuningParams(E_TUNING_SW_GF, query);
    return dofValue;
}

bool
StereoTuningProvider::getOCCTuningInfo(MUINT32 &coreNumber, TUNING_PAIR_LIST_T &tuningParams, ENUM_STEREO_SCENARIO eScenario)
{
    TuningQuery_T query;
    query.intParams[QUERY_KEY_SCENARIO] = eScenario;

    query.results[QUERY_KEY_CORE_NUMBER]   = &coreNumber;
    query.results[QUERY_KEY_TUNING_PARAMS] = &tuningParams;
    return StereoTuningProviderKernel::getInstance()->getTuningParams(E_TUNING_SW_OCC, query);
}

bool
StereoTuningProvider::getSWBokehTuningInfo(TUNING_PAIR_LIST_T &tuningParams, std::vector<int> &clearTable)
{
    TuningQuery_T query;
    query.strParams[QUERY_KEY_APP]             = QUERY_KEY_CAMERA;
    query.results[QUERY_KEY_TUNING_PARAMS]     = &tuningParams;
    query.results[QUERY_KEY_CLEAR_RANGE_TABLE] = &clearTable;

    return StereoTuningProviderKernel::getInstance()->getTuningParams(E_TUNING_SW_BOKEH, query);
}

MUINT32
StereoTuningProvider::getSWBokehDoFValue(const MUINT32 dofLevel)
{
    TuningQuery_T query;
    int dofValue = dofLevel * 2;    //default
    query.intParams[QUERY_KEY_DOF_LEVEL] = dofLevel;
    query.results[QUERY_KEY_DOF_VALUE] = &dofValue;
    StereoTuningProviderKernel::getInstance()->getTuningParams(E_TUNING_SW_BOKEH, query);
    return dofValue;
}

bool
StereoTuningProvider::getN3DTuningInfo(ENUM_STEREO_SCENARIO eScenario, TUNING_PAIR_LIST_T &tuningParams)
{
    TuningQuery_T query;
    query.intParams[QUERY_KEY_SCENARIO] = eScenario;

    query.results[QUERY_KEY_TUNING_PARAMS] = &tuningParams;

    return StereoTuningProviderKernel::getInstance()->getTuningParams(E_TUNING_SW_N3D, query);
}

bool
StereoTuningProvider::getN3DMaskMargin(ENUM_STEREO_SCENARIO eScenario, StereoImageMargin &maskMargin)
{
    TuningQuery_T query;
    query.intParams[QUERY_KEY_SCENARIO]  = eScenario;

    query.results[QUERY_KEY_MASK_MARGIN] = &maskMargin;

    return StereoTuningProviderKernel::getInstance()->getTuningParams(E_TUNING_SW_N3D, query);
}

#if (1==HAS_AIDEPTH)
bool
StereoTuningProvider::getAIDepthTuningInfo(MUINT32 &coreNumber, TUNING_PAIR_LIST_T &tuningParams)
{
    TuningQuery_T query;
    query.results[QUERY_KEY_CORE_NUMBER]   = &coreNumber;
    query.results[QUERY_KEY_TUNING_PARAMS] = &tuningParams;
    return StereoTuningProviderKernel::getInstance()->getTuningParams(E_TUNING_SW_AIDEPTH, query);
}

bool
StereoTuningProvider::getVideoAIDepthTuningInfo(MUINT32 &coreNumber, TUNING_PAIR_LIST_T &tuningParams)
{
    TuningQuery_T query;
    query.results[QUERY_KEY_CORE_NUMBER]   = &coreNumber;
    query.results[QUERY_KEY_TUNING_PARAMS] = &tuningParams;
    return StereoTuningProviderKernel::getInstance()->getTuningParams(E_TUNING_SW_VIDEO_AIDEPTH, query);
}
#endif
