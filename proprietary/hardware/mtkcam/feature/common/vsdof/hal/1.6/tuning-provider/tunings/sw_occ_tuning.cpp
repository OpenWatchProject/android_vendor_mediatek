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
#define LOG_TAG "StereoTuningProvider_SWOCC"

#include "sw_occ_tuning.h"

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

SW_OCCTuning::SW_OCCTuning(Value &document)
{
    __occMap[eSTEREO_SCENARIO_PREVIEW] = SW_OCC_TUNING_T();
    __occMap[eSTEREO_SCENARIO_RECORD]  = SW_OCC_TUNING_T();
    __occMap[eSTEREO_SCENARIO_CAPTURE] = SW_OCC_TUNING_T();

    _init(document);
}

SW_OCCTuning::~SW_OCCTuning()
{
}

bool
SW_OCCTuning::retrieveTuningParams(TuningQuery_T &query)
{
    int *coreNumber = (int *)query.results[QUERY_KEY_CORE_NUMBER];
    TUNING_PAIR_LIST_T *params = (TUNING_PAIR_LIST_T *)query.results[QUERY_KEY_TUNING_PARAMS];

    if(NULL == params) {
        MY_LOGE("Cannot get %s", QUERY_KEY_TUNING_PARAMS);
        return false;
    }

    bool result = true;
    int scenario = query.intParams[QUERY_KEY_SCENARIO];
    if(__occMap.find(scenario) != __occMap.end()) {
        *coreNumber = __occMap[scenario].coreNumber;
        *params = __occMap[scenario].params;
    } else {
        MY_LOGE("Unknown scenario: %d", scenario);
        result = false;
    }

    return result;
}

void
SW_OCCTuning::_initDefaultValues()
{
    for(int i = 1; i < TOTAL_STEREO_SCENARIO; i++) {
        SW_OCC_TUNING_T &occTuning = __occMap[i];
        occTuning.coreNumber = 1;
        occTuning.params =
        {
            {"occ.debugLevel",  0},
            {"occ.logLevel",    0},
            {"occ.calibration_en", 0},
            {"occ.offset_en", 0},
        };

        if(i == eSTEREO_SCENARIO_CAPTURE) {
            occTuning.params.push_back({"occ.holeFilling", 1});
        }
    }
}

void
SW_OCCTuning::log(FastLogger &logger, bool inJSON)
{
    if(inJSON) {
        return StereoTuningBase::log(logger, inJSON);
    }

    logger.FastLogD("======== OCC Parameters ========");
    for (OCC_MAP_T::iterator it = __occMap.begin(); it != __occMap.end(); ++it) {
        int scenario = it->first;
        SW_OCC_TUNING_T &occTuning = it->second;

        logger
        .FastLogD("Scenario: %s", SCENARIO_NAMES[scenario])
        .FastLogD("CoreNumber: %d", occTuning.coreNumber);

        for(auto &param : occTuning.params) {
            logger.FastLogD("\"%s\": %d", param.first.c_str(), param.second);
        }

        logger.FastLogD("-------------------------------");
    }

    logger.print();
}

void
SW_OCCTuning::_initDocument()
{
    // "SW_OCC": [
    //     {
    //         "Parameters": {
    //             "scenario": "Preview"
    //         },
    //         "Values": {
    //             "CoreNumber": 1,
    //             "TuningParams": {
    //                 "occ.logLevel": 0,
    //                 "occ.debugLevel": 0
    //             }
    //         }
    //     },
    //     ...
    // ]

    Value occValue(kArrayType);

    for (OCC_MAP_T::iterator it=__occMap.begin(); it!=__occMap.end(); ++it) {
        int scenario = it->first;
        SW_OCC_TUNING_T &occTuning = it->second;

        Value resultValue(kObjectType);
        // "Parameters": {
        //     "scenario": "Preview"
        // },
        Value parameters(kObjectType);
        parameters.AddMember(QUERY_KEY_SCENARIO, StringRef(SCENARIO_NAMES[scenario]), _allocator);
        resultValue.AddMember(VALUE_KEY_PARAMETERS, parameters, _allocator);

        // "Values": {
        //     "CoreNumber": 1,
        //     "TuningParams": {
        //        "occ.logLevel": 0,
        //        "occ.debugLevel": 0
        //    }
        // }
        Value tuningValue(kObjectType);
        tuningValue.AddMember(QUERY_KEY_CORE_NUMBER, occTuning.coreNumber, _allocator);

        //TuningParams
        Value paramsTable(kObjectType);
        for(auto &param : occTuning.params) {
            paramsTable.AddMember(StringRef(param.first.c_str()), param.second, _allocator);
        }

        tuningValue.AddMember(QUERY_KEY_TUNING_PARAMS, paramsTable.Move(), _allocator);
        resultValue.AddMember(VALUE_KEY_VALUES, tuningValue.Move(), _allocator);

        occValue.PushBack(resultValue.Move(), _allocator);
    }

    Value key;
    key.SetString(StringRef(getTuningKey(), strlen(getTuningKey())));
    _document.AddMember(key, occValue, _allocator);
}

void
SW_OCCTuning::_loadValuesFromDocument()
{
    Value& occValues = _document[getTuningKey()];

    for(SizeType i = 0; i < occValues.Size(); i++) {
        const Value &value = occValues[i];
        const char *SCENARIO_NAME = value[VALUE_KEY_PARAMETERS][QUERY_KEY_SCENARIO].GetString();
        int scenario = 1;
        for(int s = 1; s < TOTAL_STEREO_SCENARIO; s++) {
            if(!strncmp(SCENARIO_NAMES[s], SCENARIO_NAME, strlen(SCENARIO_NAMES[s]))) {
                scenario = s;
                break;
            }
        }

        if(__occMap.find(scenario) == __occMap.end()) {
            MY_LOGE("Unkown scenario %d", scenario);
            continue;
        }

        SW_OCC_TUNING_T &occTuning = __occMap[scenario];
        const Value &tuningValues = value[VALUE_KEY_VALUES];
        occTuning.coreNumber = tuningValues[QUERY_KEY_CORE_NUMBER].GetInt();

        //Extract tuning params
        occTuning.params.clear();
        const Value &tuningParams = tuningValues[QUERY_KEY_TUNING_PARAMS];
        for(auto &m : tuningParams.GetObject()) {
            occTuning.params.push_back({m.name.GetString(), m.value.GetInt()});
        }
    }
}
