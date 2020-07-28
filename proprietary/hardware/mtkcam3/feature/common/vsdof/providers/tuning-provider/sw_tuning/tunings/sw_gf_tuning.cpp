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

#include <mtkcam/utils/std/ULog.h>
#define CAM_ULOG_MODULE_ID MOD_MULTICAM_PROVIDER
#include "sw_gf_tuning.h"

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

SW_GFTuning::SW_GFTuning(json &tuningJson)
{
    __gfMap[eSTEREO_SCENARIO_PREVIEW] = SW_GF_TUNING_T();
    __gfMap[eSTEREO_SCENARIO_RECORD]  = SW_GF_TUNING_T();
    __gfMap[eSTEREO_SCENARIO_CAPTURE] = SW_GF_TUNING_T();

    _init(tuningJson);
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
SW_GFTuning::_loadValuesFromDocument(const json& gfValues)
{
    const char *TUNING_PREFIX = "gf.";
    const size_t TUNING_PREFIX_SIZE = strlen(TUNING_PREFIX);

    for(auto &gfValue : gfValues) {
        std::string SCENARIO_NAME = gfValue[VALUE_KEY_PARAMETERS][QUERY_KEY_SCENARIO].get<string>();
        int scenario = 1;
        for(int s = 1; s < TOTAL_STEREO_SCENARIO; s++) {
            if(gfValue[VALUE_KEY_PARAMETERS][QUERY_KEY_SCENARIO] == SCENARIO_NAMES[s]) {
                scenario = s;
                break;
            }
        }

        if(__gfMap.find(scenario) == __gfMap.end()) {
            MY_LOGE("Unkown scenario %s", SCENARIO_NAME.c_str());
            continue;
        }

        SW_GF_TUNING_T &tuning = __gfMap[scenario];
        const json &tuningValues = gfValue[VALUE_KEY_VALUES];
        tuning.coreNumber = tuningValues[QUERY_KEY_GF_CORE_NUMBER].get<int>();
        if(tuning.coreNumber < 1) {
            tuning.coreNumber = 1;
        }

        //Extract tuning params
        bool hasClearTableArray     = false;
        bool hasDofTableArray       = false;
        bool hasDispCtrlPointsArray = false;
        bool hasBlurGainTableArray  = false;
        tuning.clearTable.clear();
        tuning.dofTable.clear();
        tuning.dispCtrlPoints.clear();
        tuning.blurGainTable.clear();

        int v = 0;
        if(_isArray(tuningValues, QUERY_KEY_CLEAR_TABLE_PREFIX)) {
            hasClearTableArray = true;
            tuning.clearTable = tuningValues[QUERY_KEY_CLEAR_TABLE_PREFIX].get<GF_TUNING_T>();
        } else {
            int clearTableIndex  = 0;
            string clearTableKey = QUERY_KEY_CLEAR_TABLE_PREFIX+std::to_string(clearTableIndex);
            while(LogicalCamJSONUtil::HasMember(tuningValues, clearTableKey)) {
                v = tuningValues[clearTableKey].get<int>();
                if(v > 0) {
                    tuning.clearTable.push_back(v);
                } else {
                    MY_LOGW("Ignore invalid clear range(%d) for index %d", v, clearTableIndex);
                }

                clearTableIndex++;
                clearTableKey = QUERY_KEY_CLEAR_TABLE_PREFIX+std::to_string(clearTableIndex);
            }
        }

        if(_isArray(tuningValues, QUERY_KEY_DOF_TABLE_PREFIX)) {
            hasDofTableArray = true;
            tuning.dofTable = tuningValues[QUERY_KEY_DOF_TABLE_PREFIX].get<std::vector<MUINT32>>();
        } else {
            int dofTableIndex  = 0;
            string doFTableKey = QUERY_KEY_DOF_TABLE_PREFIX+std::to_string(dofTableIndex);
            while(LogicalCamJSONUtil::HasMember(tuningValues, doFTableKey)) {
                v = tuningValues[doFTableKey].get<int>();
                if(v >= 0) {
                    tuning.dofTable.push_back(v);
                } else {
                    MY_LOGW("Ignore invalid DOF Value(%d) for index %d", v, dofTableIndex);
                }

                dofTableIndex++;
                doFTableKey = QUERY_KEY_DOF_TABLE_PREFIX+std::to_string(dofTableIndex);
            }
        }

        if(_isArray(tuningValues, QUERY_KEY_DISPCTRLPOINTS)) {
            hasDispCtrlPointsArray = true;
            tuning.dispCtrlPoints = tuningValues[QUERY_KEY_DISPCTRLPOINTS].get<GF_TUNING_T>();
        } else {
            int dispCtrlPointsIndex  = 0;
            string dispCtrlPointsKey = QUERY_KEY_DISPCTRLPOINTS+std::to_string(dispCtrlPointsIndex);
            while(LogicalCamJSONUtil::HasMember(tuningValues, dispCtrlPointsKey)) {
                v = tuningValues[dispCtrlPointsKey.c_str()].get<int>();
                tuning.dispCtrlPoints.push_back(v);

                dispCtrlPointsIndex++;
                dispCtrlPointsKey = QUERY_KEY_DISPCTRLPOINTS+std::to_string(dispCtrlPointsIndex);
            }
        }

        if(_isArray(tuningValues, QUERY_KEY_BLURGAINTABLE)) {
            hasBlurGainTableArray = true;
            tuning.blurGainTable = tuningValues[QUERY_KEY_BLURGAINTABLE].get<GF_TUNING_T>();
        } else {
            int blurGainTableIndex  = 0;
            string blurGainTableKey = QUERY_KEY_BLURGAINTABLE+std::to_string(blurGainTableIndex);
            while(LogicalCamJSONUtil::HasMember(tuningValues, blurGainTableKey)) {
                v = tuningValues[blurGainTableKey].get<int>();
                tuning.blurGainTable.push_back(v);

                blurGainTableIndex++;
                blurGainTableKey = QUERY_KEY_BLURGAINTABLE+std::to_string(blurGainTableIndex);
            }
        }

        tuning.params.clear();
        for(json::const_iterator it = tuningValues.begin(); it != tuningValues.end(); ++it) {
            //Extract "gf.xxxx" parameters
            if(it.key().length() > TUNING_PREFIX_SIZE &&
               !strncmp(it.key().c_str(), TUNING_PREFIX, TUNING_PREFIX_SIZE))
            {
                tuning.params.push_back({it.key(), it.value().get<int>()});
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
