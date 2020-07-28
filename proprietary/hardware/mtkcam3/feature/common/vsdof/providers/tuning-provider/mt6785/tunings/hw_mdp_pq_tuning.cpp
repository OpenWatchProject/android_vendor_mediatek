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

#include "hw_mdp_pq_tuning.h"

#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s]" fmt, __func__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s]" fmt, __func__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

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
    string queryName(query.strParams[QUERY_KEY_MDP_PQ_NAME]);
    VSDOFParam *params = (VSDOFParam *)query.results[QUERY_KEY_MDP_PQ_INFO];

    if(NULL == params) {
        MY_LOGE("Cannot get %s", QUERY_KEY_MDP_PQ_INFO);
        return false;
    }

    bool result = true;
    if(__mdpPQParams.find(queryName) != __mdpPQParams.end()) {
        *params = __mdpPQParams[queryName];
    } else {
        MY_LOGW("Cannot find MDP PQ for %s, use default", queryName.c_str());
        params->isRefocus            = 1;
        params->defaultUpTable       = 9;
        params->defaultDownTable     = 17;
        params->IBSEGain             = 0;
        params->switchRatio6Tap6nTap = 1;
        params->switchRatio6nTapAcc  = 1;

        __mdpPQParams[queryName] = *params;
    }

    return result;
}

void
HW_MDPPQTuning::_initDefaultValues()
{
    for(int i = 0; i < PASS2_ROUND_COUNT; ++i) {
        __mdpPQParams[PASS2_ROUND_NAME[i]].isRefocus            = 1;
        __mdpPQParams[PASS2_ROUND_NAME[i]].defaultUpTable       = 9;
        __mdpPQParams[PASS2_ROUND_NAME[i]].defaultDownTable     = 17;
        __mdpPQParams[PASS2_ROUND_NAME[i]].IBSEGain             = 0;
        __mdpPQParams[PASS2_ROUND_NAME[i]].switchRatio6Tap6nTap = 1;
        __mdpPQParams[PASS2_ROUND_NAME[i]].switchRatio6nTapAcc  = 1;
    }
}

void
HW_MDPPQTuning::log(FastLogger &logger, bool inJSON)
{
    if(inJSON) {
        return StereoTuningBase::log(logger, inJSON);
    }

    logger.FastLogD("====== MDP PQ Parameters ======");
    for(auto &param : __mdpPQParams) {
        VSDOFParam &tuning = param.second;

        logger.FastLogD("Name: %s", param.first.c_str())
              .FastLogD("isRefocus:            %d", tuning.isRefocus)
              .FastLogD("defaultUpTable:       %d", tuning.defaultUpTable)
              .FastLogD("defaultDownTable:     %d", tuning.defaultDownTable)
              .FastLogD("IBSEGain:             %d", tuning.IBSEGain)
              .FastLogD("switchRatio6Tap6nTap: %d", tuning.switchRatio6Tap6nTap)
              .FastLogD("switchRatio6nTapAcc:  %d", tuning.switchRatio6nTapAcc)
              .FastLogD("-------------------------------");
    }

    logger.print();
}

void
HW_MDPPQTuning::_loadValuesFromDocument(const json& mdpPQValues)
{
    __mdpPQParams.clear();
    for(auto &mdpPQValue : mdpPQValues) {
        string name = mdpPQValue[VALUE_KEY_PARAMETERS][QUERY_KEY_MDP_PQ_NAME].get<string>();

        const json &tuningValues = mdpPQValue[VALUE_KEY_VALUES];
        VSDOFParam tuning;
        tuning.isRefocus            = _getInt(tuningValues, "isRefocus");
        tuning.defaultUpTable       = _getInt(tuningValues, "defaultUpTable");
        tuning.defaultDownTable     = _getInt(tuningValues, "defaultDownTable");
        tuning.IBSEGain             = _getInt(tuningValues, "IBSEGain");
        tuning.switchRatio6Tap6nTap = _getInt(tuningValues, "switchRatio6Tap6nTap");
        tuning.switchRatio6nTapAcc  = _getInt(tuningValues, "switchRatio6nTapAcc");
        __mdpPQParams[name] = std::move(tuning);
    }
}
