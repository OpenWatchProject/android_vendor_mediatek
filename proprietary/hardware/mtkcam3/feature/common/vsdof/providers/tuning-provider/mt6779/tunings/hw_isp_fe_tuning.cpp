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
#define LOG_TAG "StereoTuningProvider_HWISPFE"

#include <mtkcam/utils/std/ULog.h>
#define CAM_ULOG_MODULE_ID MOD_MULTICAM_PROVIDER
#include "hw_isp_fe_tuning.h"

#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#define MY_LOGD(fmt, arg...)    CAM_ULOGMD("[%s]" fmt, __func__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_ULOGMI("[%s]" fmt, __func__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_ULOGMW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_ULOGME("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }

HW_ISP_FETuning::HW_ISP_FETuning(json &tuningJson)
{
    _init(tuningJson);
}

HW_ISP_FETuning::~HW_ISP_FETuning()
{
}

bool
HW_ISP_FETuning::retrieveTuningParams(TuningQuery_T &query)
{
    dip_x_reg_t *tuningInfo = (dip_x_reg_t *)query.results[QUERY_KEY_ISP_FE_INFO];

    if(NULL == tuningInfo) {
        MY_LOGE("Cannot get %s", QUERY_KEY_ISP_FE_INFO);
        return false;
    }

    tuningInfo->GGM_D3A_GGM_CTRL.Bits.GGM_LNR                    = __tuningInfo.GGM_D3A_GGM_CTRL.Bits.GGM_LNR;
    tuningInfo->DIPCTL_D1A_DIPCTL_YUV_EN1.Bits.DIPCTL_GGM_D3_EN  = __tuningInfo.DIPCTL_D1A_DIPCTL_YUV_EN1.Bits.DIPCTL_GGM_D3_EN;
    tuningInfo->IGGM_D1A_IGGM_CTRL.Bits.IGGM_LNR                 = __tuningInfo.IGGM_D1A_IGGM_CTRL.Bits.IGGM_LNR;
    tuningInfo->DIPCTL_D1A_DIPCTL_YUV_EN1.Bits.DIPCTL_IGGM_D1_EN = __tuningInfo.DIPCTL_D1A_DIPCTL_YUV_EN1.Bits.DIPCTL_IGGM_D1_EN;

    return true;
}

void
HW_ISP_FETuning::_initDefaultValues()
{
    __tuningInfo.GGM_D3A_GGM_CTRL.Bits.GGM_LNR                    = 1;
    __tuningInfo.DIPCTL_D1A_DIPCTL_YUV_EN1.Bits.DIPCTL_GGM_D3_EN  = 1;
    __tuningInfo.IGGM_D1A_IGGM_CTRL.Bits.IGGM_LNR                 = 1;
    __tuningInfo.DIPCTL_D1A_DIPCTL_YUV_EN1.Bits.DIPCTL_IGGM_D1_EN = 1;
}

void
HW_ISP_FETuning::log(FastLogger &logger, bool inJSON)
{
    if(inJSON) {
        return StereoTuningBase::log(logger, inJSON);
    }

    logger
    .FastLogD("============ HW ISP FE Parameters ============")
    .FastLogD("GGM_D3A_GGM_CTRL.Bits.GGM_LNR                    %d", __tuningInfo.GGM_D3A_GGM_CTRL.Bits.GGM_LNR)
    .FastLogD("DIPCTL_D1A_DIPCTL_YUV_EN1.Bits.DIPCTL_GGM_D3_EN  %d", __tuningInfo.DIPCTL_D1A_DIPCTL_YUV_EN1.Bits.DIPCTL_GGM_D3_EN)
    .FastLogD("IGGM_D1A_IGGM_CTRL.Bits.IGGM_LNR                 %d", __tuningInfo.IGGM_D1A_IGGM_CTRL.Bits.IGGM_LNR)
    .FastLogD("DIPCTL_D1A_DIPCTL_YUV_EN1.Bits.DIPCTL_IGGM_D1_EN %d", __tuningInfo.DIPCTL_D1A_DIPCTL_YUV_EN1.Bits.DIPCTL_IGGM_D1_EN)
    .FastLogD("----------------------------------------------")
    .print();
}

void
HW_ISP_FETuning::_loadValuesFromDocument(const json& tuningValues)
{
    for(auto &tuning : tuningValues)
    {
        const json &tuningValues = tuning[VALUE_KEY_VALUES];
        __tuningInfo.GGM_D3A_GGM_CTRL.Bits.GGM_LNR                    = _getInt(tuningValues, "GGM_LNR");
        __tuningInfo.DIPCTL_D1A_DIPCTL_YUV_EN1.Bits.DIPCTL_GGM_D3_EN  = _getInt(tuningValues, "DIPCTL_GGM_D3_EN");
        __tuningInfo.IGGM_D1A_IGGM_CTRL.Bits.IGGM_LNR                 = _getInt(tuningValues, "IGGM_LNR");
        __tuningInfo.DIPCTL_D1A_DIPCTL_YUV_EN1.Bits.DIPCTL_IGGM_D1_EN = _getInt(tuningValues, "DIPCTL_IGGM_D1_EN");
    }
}
