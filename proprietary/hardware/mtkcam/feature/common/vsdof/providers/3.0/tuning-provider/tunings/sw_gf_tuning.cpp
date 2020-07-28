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
    int *coreNumber = NULL;
    if(query.results.find(QUERY_KEY_CORE_NUMBER) != query.results.end()) {
        coreNumber = (int *)query.results[QUERY_KEY_CORE_NUMBER];
    }

    GF_TUNING_T *clearTable = NULL;
    if(query.results.find(QUERY_KEY_CLEAR_RANGE_TABLE) != query.results.end()) {
        clearTable = (GF_TUNING_T *)query.results[QUERY_KEY_CLEAR_RANGE_TABLE];
    }

    GF_TUNING_T *dispCtrlPoints = NULL;
    if(query.results.find(QUERY_KEY_DISPCTRLPOINTS) != query.results.end()) {
        dispCtrlPoints = (GF_TUNING_T *)query.results[QUERY_KEY_DISPCTRLPOINTS];
    }

    GF_TUNING_T *blurGainTable = NULL;
    if(query.results.find(QUERY_KEY_BLURGAINTABLE) != query.results.end()) {
        blurGainTable = (GF_TUNING_T *)query.results[QUERY_KEY_BLURGAINTABLE];
    }

    TUNING_PAIR_LIST_T *params = NULL;
    if(query.results.find(QUERY_KEY_TUNING_PARAMS) != query.results.end()) {
        params = (TUNING_PAIR_LIST_T *)query.results[QUERY_KEY_TUNING_PARAMS];
    }

    MUINT32 *dofValue = NULL;
    MUINT32 dofLevel = 0;
    if(query.intParams.find(QUERY_KEY_DOF_LEVEL) != query.intParams.end() &&
       query.results.find(QUERY_KEY_DOF_VALUE) != query.results.end())
    {
        dofLevel = query.intParams[QUERY_KEY_DOF_LEVEL];
        dofValue = (MUINT32 *)query.results[QUERY_KEY_DOF_VALUE];
    }

    bool result = true;
    int scenario = query.intParams[QUERY_KEY_SCENARIO];
    if(__gfMap.find(scenario) != __gfMap.end()) {
        SW_GF_TUNING_T &tuning = __gfMap[scenario];
        if(coreNumber) {
            *coreNumber = tuning.coreNumber;
        }

        if(clearTable) {
            *clearTable = tuning.clearTable;
        }

        if(dispCtrlPoints) {
            *dispCtrlPoints = tuning.dispCtrlPoints;
        }

        if(blurGainTable) {
            *blurGainTable = tuning.blurGainTable;
        }

        if(params) {
            *params = tuning.params;
        }

        if(dofValue) {
            *dofValue = __getDofValue(dofLevel, tuning.dofTable);
        }
    } else {
        MY_LOGE("Unknown scenario: %d", scenario);
        result = false;
    }

    return result;
}

MUINT32
SW_GFTuning::__getDofValue(MUINT32 dofLevel, vector<MUINT32> &table)
{
    if(0 == table.size()) {
        return dofLevel * 2;
    }

    if(dofLevel >= table.size()) {
        return table.back();
    }

    return table[dofLevel];
}

void
SW_GFTuning::_initDefaultValues()
{
    for(int i = 1; i < TOTAL_STEREO_SCENARIO; i++) {
        SW_GF_TUNING_T &gfTuning = __gfMap[i];
        gfTuning.coreNumber     = 1;
        gfTuning.clearTable     = __DEFAULT_CLEAR_TABLE;
        gfTuning.dispCtrlPoints = __DEFAULT_DISP_CTRL_POINTS;
        gfTuning.blurGainTable  = __DEFAULT_BLUR_GAIN_TABLE;
        gfTuning.params =
        {
            { "gf.logLevel", 0 },
            { "gf.debugLevel", 0 },
            { "gf.DPP_output_resize", 1 },
            { "gf.mode_dl", 1 },
            { "gf.sigma", 3 },
            { "gf.sigmat", 50 },
            { "gf.tempbase", 800 },
            { "gf.alpha", 100 },
            { "gf.wessiter", 1 },
            { "gf.dofptl", 6 },
            { "gf.dofpth", 8 },
            { "gf.max_blur_level", 8 },
            { "gf.bk_single_side", 0 },
            { "gf.bk_ifocus", 0 },
            { "gf.bk_ifocus_touch", 0 },
            { "gf.bk_ifocus_ratio", 0 },
            { "gf.be_lbound_ratio", 0 },
            { "gf.be_tbound_ratio", 0 },
            { "gf.be_rbound_ratio", 0 },
            { "gf.be_bbound_ratio", 0 },
            { "gf.be_hist_ratio", 25 },
            { "gf.be_depth_limit", 128 },
            { "gf.be_occ_dia_radius", -1 },
            { "gf.be_weight_spatial", 1 },
            { "gf.be_weight_color", 1 },
            { "gf.calibrate_en", 0 },
            { "gf.offset_en", 0 },
            { "gf.focus_abs_depth_max", 128 },
            { "gf.blur_mapping_mode", 1 },
            { "gf.disp_offset", 9 },
            { "gf.true_min_dac", 407 },
            { "gf.dac_scale", 998 },
            { "gf.clr_curve_type", 1 },
            { "gf.depth_domain_clr", 300 },
            { "gf.min_clr", 3 },
            { "gf.gain_curve_type", 1 },
            { "gf.max_blur_gain", 32 },
            { "gf.min_depth_to_gain", 200 },
            { "gf.gain_bezier_x", 100 },
            { "gf.gain_bezier_y", 100 },
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
        logger.FastLogD("Clear Table(%zu): %s", gfTuning.clearTable.size(), oss.str().c_str());

        oss.clear();
        oss.str("");
        for(size_t i = 0; i < gfTuning.dispCtrlPoints.size(); ++i) {
            oss << gfTuning.dispCtrlPoints[i] << " ";
        }
        logger.FastLogD("DispCtrlPoints(%zu): %s", gfTuning.dispCtrlPoints.size(), oss.str().c_str());

        oss.clear();
        oss.str("");
        for(size_t i = 0; i < gfTuning.blurGainTable.size(); ++i) {
            oss << gfTuning.blurGainTable[i] << " ";
        }
        logger.FastLogD("BlurGainTable(%zu): %s", gfTuning.blurGainTable.size(), oss.str().c_str());

        if(gfTuning.dofTable.size() > 0) {
            oss.clear();
            oss.str("");
            for(size_t i = 0; i < gfTuning.dofTable.size(); ++i) {
                oss << gfTuning.dofTable[i] << " ";
            }
            logger.FastLogD("DoF Mapping(%zu): %s", gfTuning.dofTable.size(), oss.str().c_str());
        }

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
    //             "GF_CoreNumber": 0,
    //             "ClearTableSize": 17,
    //             "ClearTable0": 3,
    //             "ClearTable1": 3,
    //             "ClearTable2": 3,
    //             "ClearTable3": 4,
    //             ...
    //             "ClearTable16": 32,
    //             "gf.logLevel": 0,
    //             "gf.debugLevel": 0,
    //             "gf.sigma": 3,
    //             "gf.sigmat": 50,
    //             "gf.tempbase": 800,
    //             "gf.alpha": 100,
    //             "gf.wessiter": 1,
    //             "gf.dofptl": 12,
    //             "gf.dofpth": 24,
    //             "gf.bk_single_side": 1
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
        //     "GF_CoreNumber": 0,
        //     "ClearTable0": 3,
        //     "ClearTable1": 3,
        //     "ClearTable2": 3,
        //     "ClearTable3": 4,
        //     ...
        //     "ClearTable16": 32,
        //     "gf.logLevel": 0,
        //     "gf.debugLevel": 0,
        //     "gf.sigma": 3,
        //     "gf.sigmat": 50,
        //     "gf.tempbase": 800,
        //     "gf.alpha": 100,
        //     "gf.wessiter": 1,
        //     "gf.dofptl": 12,
        //     "gf.dofpth": 24,
        //     "gf.bk_single_side": 1
        // }
        Value tuningValue(kObjectType);
        tuningValue.AddMember(QUERY_KEY_GF_CORE_NUMBER, gfTuning.coreNumber, _allocator);

        //ClearRangeTable
        size_t tableSize = gfTuning.clearTable.size();
        if(__clearTableNames.size() < tableSize) {
            for(size_t t = __clearTableNames.size(); t < tableSize; ++t) {
                __clearTableNames.push_back(std::string(QUERY_KEY_CLEAR_TABLE_PREFIX)+std::to_string(t));
            }
        }

        for(size_t t = 0; t < tableSize; ++t) {
            tuningValue.AddMember(StringRef(__clearTableNames[t].c_str()), gfTuning.clearTable[t], _allocator);
        }

        //dispCtrlPoints
        tableSize = gfTuning.dispCtrlPoints.size();
        if(__dispCtrlPointsNames.size() < tableSize) {
            for(size_t t = __dispCtrlPointsNames.size(); t < tableSize; ++t) {
                __dispCtrlPointsNames.push_back(std::string(QUERY_KEY_DISPCTRLPOINTS)+std::to_string(t));
            }
        }

        for(size_t t = 0; t < tableSize; ++t) {
            tuningValue.AddMember(StringRef(__dispCtrlPointsNames[t].c_str()), gfTuning.dispCtrlPoints[t], _allocator);
        }

        //blurGainTable
        tableSize = gfTuning.blurGainTable.size();
        if(__blurGainTableNames.size() < tableSize) {
            for(size_t t = __blurGainTableNames.size(); t < tableSize; ++t) {
                __blurGainTableNames.push_back(std::string(QUERY_KEY_BLURGAINTABLE)+std::to_string(t));
            }
        }

        for(size_t t = 0; t < tableSize; ++t) {
            tuningValue.AddMember(StringRef(__blurGainTableNames[t].c_str()), gfTuning.blurGainTable[t], _allocator);
        }

        //TuningParams
        for(auto &param : gfTuning.params) {
            tuningValue.AddMember(StringRef(param.first.c_str()), param.second, _allocator);
        }

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

    const char *TUNING_PREFIX = "gf.";
    const size_t TUNING_PREFIX_SIZE = strlen(TUNING_PREFIX);

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
            MY_LOGE("Unkown scenario %s", SCENARIO_NAME);
            continue;
        }

        SW_GF_TUNING_T &tuning = __gfMap[scenario];
        const Value &tuningValues = value[VALUE_KEY_VALUES];
        tuning.coreNumber = tuningValues[QUERY_KEY_GF_CORE_NUMBER].GetInt();
        if(tuning.coreNumber < 1) {
            tuning.coreNumber = 1;
        }

        //Extract tuning params
        tuning.clearTable.clear();
        tuning.dofTable.clear();
        int v = 0;
        int dofTableIndex    = 0;
        string doFTableKey   = QUERY_KEY_DOF_TABLE_PREFIX+std::to_string(dofTableIndex);
        int clearTableIndex  = 0;
        string clearTableKey = QUERY_KEY_CLEAR_TABLE_PREFIX+std::to_string(clearTableIndex);
        int dispCtrlPointsIndex  = 0;
        string dispCtrlPointsKey = QUERY_KEY_DISPCTRLPOINTS+std::to_string(dispCtrlPointsIndex);
        int blurGainTableIndex  = 0;
        string blurGainTableKey = QUERY_KEY_BLURGAINTABLE+std::to_string(blurGainTableIndex);

        tuning.params.clear();
        for(auto &m : tuningValues.GetObject()) {
            //Extract clear table
            if(tuningValues.HasMember(clearTableKey.c_str())) {
                __clearTableNames.push_back(clearTableKey);

                v = tuningValues[clearTableKey.c_str()].GetInt();
                tuning.clearTable.push_back(v);

                clearTableIndex++;
                clearTableKey = QUERY_KEY_CLEAR_TABLE_PREFIX+std::to_string(clearTableIndex);
            //Extract dispCtrlPoints
            } else if(tuningValues.HasMember(dispCtrlPointsKey.c_str())) {
                __dispCtrlPointsNames.push_back(dispCtrlPointsKey);

                v = tuningValues[dispCtrlPointsKey.c_str()].GetInt();
                tuning.dispCtrlPoints.push_back(v);

                dispCtrlPointsIndex++;
                dispCtrlPointsKey = QUERY_KEY_DISPCTRLPOINTS+std::to_string(dispCtrlPointsIndex);
            //Extract blurGainTable
            } else if(tuningValues.HasMember(blurGainTableKey.c_str())) {
                __blurGainTableNames.push_back(blurGainTableKey);

                v = tuningValues[blurGainTableKey.c_str()].GetInt();
                tuning.blurGainTable.push_back(v);

                blurGainTableIndex++;
                blurGainTableKey = QUERY_KEY_BLURGAINTABLE+std::to_string(blurGainTableIndex);
            //Extract DoF table
            } else if(tuningValues.HasMember(doFTableKey.c_str())) {
                __dofTableNames.push_back(doFTableKey);

                v = tuningValues[doFTableKey.c_str()].GetInt();
                tuning.dofTable.push_back(v);

                dofTableIndex++;
                doFTableKey   = QUERY_KEY_DOF_TABLE_PREFIX+std::to_string(dofTableIndex);
            //Extract "gf.xxxx" parameters
            } else {
                if(m.name.Size() > TUNING_PREFIX_SIZE &&
                   !strncmp(m.name.GetString(), TUNING_PREFIX, TUNING_PREFIX_SIZE))
                {
                    tuning.params.push_back({m.name.GetString(), m.value.GetInt()});
                }
            }
        }

        if(tuning.clearTable.size() == 0) {
           tuning.clearTable = __DEFAULT_CLEAR_TABLE;
        } else {
            std::sort(tuning.clearTable.begin(), tuning.clearTable.end());
        }

        auto last = std::unique(tuning.dofTable.begin(), tuning.dofTable.end());
        if(last != tuning.dofTable.end()) {
            MY_LOGW("%zu duplicated %s found, please check \"%s\"s in %s",
                    tuning.dofTable.end()-last, QUERY_KEY_DOF_TABLE_PREFIX, QUERY_KEY_DOF_TABLE_PREFIX, getTuningKey());
            tuning.dofTable.erase(last, tuning.dofTable.end());
        }
        std::sort(tuning.dofTable.begin(), tuning.dofTable.end());
    }
}
