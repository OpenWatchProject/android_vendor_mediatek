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
#define LOG_TAG "StereoTuningProvider_SWGF"

#include "sw_gf_tuning.h"

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

SW_GFTuning::SW_GFTuning(Value &document)
{
    __gfMap[eSTEREO_SCENARIO_PREVIEW] = SW_GF_TUNING_T();
    __gfMap[eSTEREO_SCENARIO_RECORD]  = SW_GF_TUNING_T();
    __gfMap[eSTEREO_SCENARIO_CAPTURE] = SW_GF_TUNING_T();

    _init(document);
}

SW_GFTuning::~SW_GFTuning()
{
}

bool
SW_GFTuning::retrieveTuningParams(TuningQuery_T &query)
{
    int *coreNumber = (int *)query.results[QUERY_KEY_CORE_NUMBER];
    GF_TUNING_T *clearTable = (GF_TUNING_T *)query.results[QUERY_KEY_SWGF_TABLE];
    TUNING_PAIR_LIST_T *params = (TUNING_PAIR_LIST_T *)query.results[QUERY_KEY_TUNING_PARAMS];

    if(NULL == clearTable) {
        MY_LOGE("Cannot get %s", QUERY_KEY_SWGF_TABLE);
        return false;
    }

    bool result = true;
    int scenario = query.intParams[QUERY_KEY_SCENARIO];
    if(__gfMap.find(scenario) != __gfMap.end()) {
        *coreNumber = __gfMap[scenario].coreNumber;
        *clearTable = __gfMap[scenario].clearTable;
        *params = __gfMap[scenario].params;
    } else {
        MY_LOGE("Unknown scenario: %d", scenario);
        result = false;
    }

    return result;
}

void
SW_GFTuning::_initDefaultValues()
{
    for(int i = 1; i < TOTAL_STEREO_SCENARIO; i++) {
        SW_GF_TUNING_T &gfTuning = __gfMap[i];
        gfTuning.coreNumber = 1;
        gfTuning.clearTable = { 3, 4, 6, 9, 12, 16, 20, 24, 28, 32, 36, 39, 42, 46, 49, 52, 55 };
        gfTuning.params =
        {
            { "gf.logLevel", 0 },
            { "gf.debugLevel", 0 },
            { "gf.sigma", 3 },
            { "gf.sigmat", 50 },
            { "gf.tempbase", 800 },
            { "gf.alpha", 100 },
            { "gf.wessiter", 1 },
            { "gf.dofptl", 6 },
            { "gf.dofpth", 8 },
            { "gf.bk_single_side", 0 },
            { "gf.bk_ifocus", 10 },
            { "gf.bk_ifocus_ratio", 20 },
            { "gf.be_lbound_ratio", 0 },
            { "gf.be_tbound_ratio", 0 },
            { "gf.be_rbound_ratio", 0 },
            { "gf.be_bbound_ratio", 0 },
            { "gf.be_hist_ratio", 25 },
            { "gf.be_depth_limit", 128 },
            { "gf.be_occ_dia_radius", -1 },
            { "gf.be_weight_spatial", 1 },
            { "gf.be_weight_color", 1 },
        };
    }
}

void
SW_GFTuning::log(FastLogger &logger, bool inJSON)
{
    if(inJSON) {
        return StereoTuningBase::log(logger, inJSON);
    }

    logger.FastLogD("======== GF Parameters ========");
    for (GF_MAP_T::iterator it = __gfMap.begin(); it != __gfMap.end(); ++it) {
        int scenario = it->first;
        SW_GF_TUNING_T &gfTuning = it->second;

        logger
        .FastLogD("Scenario: %s", SCENARIO_NAMES[scenario])
        .FastLogD("CoreNumber: %d", gfTuning.coreNumber);

        std::ostringstream oss;
        for(size_t i = 0; i < gfTuning.clearTable.size(); ++i) {
            oss << gfTuning.clearTable[i] << " ";
        }
        logger.FastLogD("Clear Table: %s", oss.str().c_str());

        for(auto &param : gfTuning.params) {
            logger.FastLogD("\"%s\": %d", param.first.c_str(), param.second);
        }

        logger.FastLogD("-------------------------------");
    }

    logger.print();
}

void
SW_GFTuning::_initDocument()
{
    // "SW_GF": [
    //     {
    //         "Parameters": {
    //             "scenario": "Preview"
    //         },
    //         "Values": {
    //             "CoreNumber": 0,
    //             "ClearRangeTable": [3, 3, 3, 4, 6, 8, 12, 16, 20, 24, 28, 32, 32, 32, 32, 32, 32],
    //             "TuningParams": {
    //                 "gf.logLevel": 0,
    //                 "gf.debugLevel": 0,
    //                 "gf.sigma": 3,
    //                 "gf.sigmat": 50,
    //                 "gf.tempbase": 800,
    //                 "gf.alpha": 100,
    //                 "gf.wessiter": 1,
    //                 "gf.dofptl": 12,
    //                 "gf.dofpth": 24,
    //                 "gf.bk_single_side": 1
    //             }
    //         }
    //     },
    //     ...
    // ]

    Value gfValue(kArrayType);

    for (GF_MAP_T::iterator it=__gfMap.begin(); it!=__gfMap.end(); ++it) {
        int scenario = it->first;
        SW_GF_TUNING_T &gfTuning = it->second;

        Value resultValue(kObjectType);
        // "Parameters": {
        //     "scenario": "Preview"
        // },
        Value parameters(kObjectType);
        parameters.AddMember(QUERY_KEY_SCENARIO, StringRef(SCENARIO_NAMES[scenario]), _allocator);
        resultValue.AddMember(VALUE_KEY_PARAMETERS, parameters, _allocator);

        // "Values": {
        //     "CoreNumber": 0,
        //     "ClearRangeTable": [3, 3, 3, 4, 6, 8, 12, 16, 20, 24, 28, 32, 32, 32, 32, 32, 32],
        //     "TuningParams": {
        //         "gf.logLevel": 0,
        //         "gf.debugLevel": 0,
        //         "gf.sigma": 3,
        //         "gf.sigmat": 50,
        //         "gf.tempbase": 800,
        //         "gf.alpha": 100,
        //         "gf.wessiter": 1,
        //         "gf.dofptl": 12,
        //         "gf.dofpth": 24,
        //         "gf.bk_single_side": 1
        //     }
        // }
        Value tuningValue(kObjectType);
        tuningValue.AddMember(QUERY_KEY_CORE_NUMBER, gfTuning.coreNumber, _allocator);

        //ClearRangeTable
        Value clearTable(kArrayType);
        #define AddToClearTable(value) clearTable.PushBack(Value().SetInt(value), _allocator)
        for(auto v : gfTuning.clearTable) {
            AddToClearTable(v);
        }
        tuningValue.AddMember(QUERY_KEY_SWGF_TABLE, clearTable.Move(), _allocator);

        //TuningParams
        Value paramsTable(kObjectType);
        for(auto &param : gfTuning.params) {
            paramsTable.AddMember(StringRef(param.first.c_str()), param.second, _allocator);
        }

        tuningValue.AddMember(QUERY_KEY_TUNING_PARAMS, paramsTable.Move(), _allocator);
        resultValue.AddMember(VALUE_KEY_VALUES, tuningValue.Move(), _allocator);

        gfValue.PushBack(resultValue.Move(), _allocator);
    }

    Value key;
    key.SetString(StringRef(getTuningKey(), strlen(getTuningKey())));
    _document.AddMember(key, gfValue, _allocator);
}

void
SW_GFTuning::_loadValuesFromDocument()
{
    Value& gfValues = _document[getTuningKey()];

    for(SizeType i = 0; i < gfValues.Size(); i++) {
        const Value &value = gfValues[i];
        const char *SCENARIO_NAME = value[VALUE_KEY_PARAMETERS][QUERY_KEY_SCENARIO].GetString();
        int scenario = 1;
        for(int s = 1; s < TOTAL_STEREO_SCENARIO; s++) {
            if(!strncmp(SCENARIO_NAMES[s], SCENARIO_NAME, strlen(SCENARIO_NAMES[s]))) {
                scenario = s;
                break;
            }
        }

        if(__gfMap.find(scenario) == __gfMap.end()) {
            MY_LOGE("Unkown scenario %d", scenario);
            continue;
        }

        SW_GF_TUNING_T &gfTuning = __gfMap[scenario];
        const Value &tuningValues = value[VALUE_KEY_VALUES];
        gfTuning.coreNumber = tuningValues[QUERY_KEY_CORE_NUMBER].GetInt();

        //Extract clear table
        const Value &clearTableValue = tuningValues[QUERY_KEY_SWGF_TABLE];
        gfTuning.clearTable.clear();
        for(auto &v : clearTableValue.GetArray()) {
            gfTuning.clearTable.push_back(v.GetInt());
        }

        //Extract tuning params
        gfTuning.params.clear();
        const Value &tuningParams = tuningValues[QUERY_KEY_TUNING_PARAMS];
        for(auto &m : tuningParams.GetObject()) {
            gfTuning.params.push_back({m.name.GetString(), m.value.GetInt()});
        }
    }
}
