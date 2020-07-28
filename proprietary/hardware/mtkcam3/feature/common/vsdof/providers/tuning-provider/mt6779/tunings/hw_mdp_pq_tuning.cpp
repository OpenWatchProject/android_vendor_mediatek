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
#define LOG_TAG "StereoTuningProvider_HW_MDPPQ"

#include <mtkcam/utils/std/ULog.h>
#define CAM_ULOG_MODULE_ID MOD_MULTICAM_PROVIDER
#include "hw_mdp_pq_tuning.h"

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

HW_MDPPQTuning::HW_MDPPQTuning(json &tuningJson)
{
    _init(tuningJson);
}

HW_MDPPQTuning::~HW_MDPPQTuning()
{
}

bool
HW_MDPPQTuning::retrieveTuningParams(TuningQuery_T &query)
{
    VSDOFParam *params = (VSDOFParam *)query.results[QUERY_KEY_MDP_PQ_INFO];

    if(NULL == params) {
        MY_LOGE("Cannot get %s", QUERY_KEY_MDP_PQ_INFO);
        return false;
    }

    *params = __mdpPQParam;
    return true;
}

void
HW_MDPPQTuning::_initDefaultValues()
{
    __mdpPQParam.isRefocus            = 1;
    __mdpPQParam.defaultUpTable       = 7;
    __mdpPQParam.defaultDownTable     = 17;
    __mdpPQParam.IBSEGain             = 0;
    __mdpPQParam.switchRatio6Tap6nTap = 1;
    __mdpPQParam.switchRatio6nTapAcc  = 1;
}

void
HW_MDPPQTuning::log(FastLogger &logger, bool inJSON)
{
    if(inJSON) {
        return StereoTuningBase::log(logger, inJSON);
    }

    logger.FastLogD("====== MDP PQ Parameters ======")
          .FastLogD("isRefocus:            %d", __mdpPQParam.isRefocus)
          .FastLogD("defaultUpTable:       %d", __mdpPQParam.defaultUpTable)
          .FastLogD("defaultDownTable:     %d", __mdpPQParam.defaultDownTable)
          .FastLogD("IBSEGain:             %d", __mdpPQParam.IBSEGain)
          .FastLogD("switchRatio6Tap6nTap: %d", __mdpPQParam.switchRatio6Tap6nTap)
          .FastLogD("switchRatio6nTapAcc:  %d", __mdpPQParam.switchRatio6nTapAcc)
          .FastLogD("-------------------------------")
          .print();
}

void
HW_MDPPQTuning::_loadValuesFromDocument(const json& mdpPQValues)
{
    for(auto &mdpPQValue : mdpPQValues) {
        const json &tuningValues = mdpPQValue[VALUE_KEY_VALUES];
        __mdpPQParam.isRefocus            = _getInt(tuningValues, "isRefocus");
        __mdpPQParam.defaultUpTable       = _getInt(tuningValues, "defaultUpTable");
        __mdpPQParam.defaultDownTable     = _getInt(tuningValues, "defaultDownTable");
        __mdpPQParam.IBSEGain             = _getInt(tuningValues, "IBSEGain");
        __mdpPQParam.switchRatio6Tap6nTap = _getInt(tuningValues, "switchRatio6Tap6nTap");
        __mdpPQParam.switchRatio6nTapAcc  = _getInt(tuningValues, "switchRatio6nTapAcc");
        break;
    }
}
