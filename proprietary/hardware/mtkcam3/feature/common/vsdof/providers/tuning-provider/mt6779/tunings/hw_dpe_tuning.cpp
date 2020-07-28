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
#define LOG_TAG "StereoTuningProvider_HWDPE"

#include <mtkcam/utils/std/ULog.h>
#define CAM_ULOG_MODULE_ID MOD_MULTICAM_PROVIDER
#include "hw_dpe_tuning.h"

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

HW_DPETuning::HW_DPETuning(json &tuningJson)
    :   __dvpCtrlTuning(tuningJson),
        __dvsCtrlTuning(tuningJson),
        __dvsMeTuning(tuningJson),
        __dvsOccTuning(tuningJson)
{
}

bool
HW_DPETuning::retrieveTuningParams(TuningQuery_T &query)
{
    void *targetAddr = query.results[STEREO_TUNING_NAME[E_TUNING_HW_DPE]];

    if(NULL == targetAddr) {
        MY_LOGE("Cannot get target address");
        return false;
    }

    __dvpCtrlTuning.retrieveTuningParams(query);
    __dvsCtrlTuning.retrieveTuningParams(query);
    __dvsMeTuning.retrieveTuningParams(query);
    __dvsOccTuning.retrieveTuningParams(query);

    return true;
}

void
HW_DPETuning::log(FastLogger &logger, bool inJSON)
{
    if(inJSON) {
        return StereoTuningBase::log(logger, inJSON);
    }

    logger.FastLogD("===== DPE Parameters =====");
    __dvpCtrlTuning.log(logger, inJSON);
    __dvsCtrlTuning.log(logger, inJSON);
    __dvsMeTuning.log(logger, inJSON);
    __dvsOccTuning.log(logger, inJSON);
    logger.print();
}
