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
#define LOG_TAG "StereoTuningProvider_SWN3D"

#include <mtkcam/utils/std/ULog.h>
#define CAM_ULOG_MODULE_ID MOD_MULTICAM_PROVIDER
#include "sw_n3d_tuning.h"

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

SW_N3DTuning::SW_N3DTuning(json &tuningJson)
{
    __n3dMap[eSTEREO_SCENARIO_PREVIEW] = SW_N3D_TUNING_T();
    __n3dMap[eSTEREO_SCENARIO_CAPTURE] = SW_N3D_TUNING_T();

    _init(tuningJson);
}

SW_N3DTuning::~SW_N3DTuning()
{
}

bool
SW_N3DTuning::retrieveTuningParams(TuningQuery_T &query)
{
    StereoImageMargin *maskMargin = NULL;
    TUNING_PAIR_LIST_T *params    = NULL;
    if(query.results.find(QUERY_KEY_MASK_MARGIN) != query.results.end()) {
        maskMargin = (StereoImageMargin *)query.results[QUERY_KEY_MASK_MARGIN];
    }

    if(query.results.find(QUERY_KEY_TUNING_PARAMS) != query.results.end()) {
        params = (TUNING_PAIR_LIST_T *)query.results[QUERY_KEY_TUNING_PARAMS];
    }

    bool result = true;
    int scenario = query.intParams[QUERY_KEY_SCENARIO];
    if(__n3dMap.find(scenario) != __n3dMap.end()) {
        if(maskMargin) {
            *maskMargin = __n3dMap[scenario].maskMargin;
        }

        if(params) {
            *params = __n3dMap[scenario].params;
        }
    } else {
        MY_LOGD("Unknown scenario: %d", scenario);
        result = false;
    }

    return result;
}

void
SW_N3DTuning::_initDefaultValues()
{
    // for (N3D_MAP_T::iterator it = __n3dMap.begin(); it != __n3dMap.end(); ++it) {
    //     int scenario = it->first;
    //     SW_N3D_TUNING_T &n3dTuning = it->second;
    // }
}

void
SW_N3DTuning::log(FastLogger &logger, bool inJSON)
{
    if(inJSON) {
        return StereoTuningBase::log(logger, inJSON);
    }

    logger.FastLogD("======== N3D Parameters ========");
    for (N3D_MAP_T::iterator it = __n3dMap.begin(); it != __n3dMap.end(); ++it) {
        int scenario = it->first;
        SW_N3D_TUNING_T &n3dTuning = it->second;

        logger
        .FastLogD("Scenario:    %s", SCENARIO_NAMES[scenario])
        .FastLogD("Mask Margin: Left %d Top %d Right %d Bottom %d",
                    n3dTuning.maskMargin.left,
                    n3dTuning.maskMargin.top,
                    n3dTuning.maskMargin.right,
                    n3dTuning.maskMargin.bottom);

        for(auto &param : n3dTuning.params) {
            std::string s = std::bitset<32>(param.second).to_string();
            logger.FastLogD("\"%s\": %s(%u)", param.first.c_str(), s.c_str(), param.second);
        }

        logger.FastLogD("-------------------------------");
    }

    logger.print();
}

void
SW_N3DTuning::_loadValuesFromDocument(const json& n3dValues)
{
    const char *TUNING_PREFIX = "n3d.";
    const size_t TUNING_PREFIX_SIZE = strlen(TUNING_PREFIX);

    for(auto &n3dValue : n3dValues) {
        int scenario = 1;
        for(int s = 1; s < TOTAL_STEREO_SCENARIO; s++) {
            if(n3dValue[VALUE_KEY_PARAMETERS][QUERY_KEY_SCENARIO] == SCENARIO_NAMES[s]) {
                scenario = s;
                break;
            }
        }

        if(__n3dMap.find(scenario) == __n3dMap.end()) {
            std::string SCENARIO_NAME = n3dValue[VALUE_KEY_PARAMETERS][QUERY_KEY_SCENARIO].get<string>();
            MY_LOGD("Unkown scenario %s", SCENARIO_NAME.c_str());
            continue;
        }

        SW_N3D_TUNING_T &tuning = __n3dMap[scenario];
        const json &tuningValues = n3dValue[VALUE_KEY_VALUES];

        // Get mask margin
        if(LogicalCamJSONUtil::HasMember(tuningValues, QUERY_KEY_MASK_MARGIN)) {
            const json &maskTuningObj = tuningValues[QUERY_KEY_MASK_MARGIN];
            if(maskTuningObj.type() == json::value_t::object) {
                // "MaskMargin": {
                //     "Left": 4,
                //     "Top": 4,
                //     "Right": 4,
                //     "Bottom": 4
                // }
                if(LogicalCamJSONUtil::HasMember(maskTuningObj, QUERY_KEY_LEFT)) {
                    tuning.maskMargin.left = maskTuningObj[QUERY_KEY_LEFT].get<int>();
                }
                if(LogicalCamJSONUtil::HasMember(maskTuningObj, QUERY_KEY_TOP)) {
                    tuning.maskMargin.top = maskTuningObj[QUERY_KEY_TOP].get<int>();
                }
                if(LogicalCamJSONUtil::HasMember(maskTuningObj, QUERY_KEY_RIGHT)) {
                    tuning.maskMargin.right = maskTuningObj[QUERY_KEY_RIGHT].get<int>();
                }
                if(LogicalCamJSONUtil::HasMember(maskTuningObj, QUERY_KEY_BOTTOM)) {
                    tuning.maskMargin.bottom = maskTuningObj[QUERY_KEY_BOTTOM].get<int>();
                }
            } else {
                // "MaskMargin": 4
                int margin = maskTuningObj.get<int>();
                tuning.maskMargin = StereoImageMargin(margin);
            }
        }

        //Extract tuning params
        tuning.params.clear();
        for(json::const_iterator it = tuningValues.begin(); it != tuningValues.end(); ++it) {
            if(it.key().length() > TUNING_PREFIX_SIZE &&
               !strncmp(it.key().c_str(), TUNING_PREFIX, TUNING_PREFIX_SIZE))
            {
                tuning.params.push_back({it.key(), it.value().get<int>()});
            }
        }
    }
}
