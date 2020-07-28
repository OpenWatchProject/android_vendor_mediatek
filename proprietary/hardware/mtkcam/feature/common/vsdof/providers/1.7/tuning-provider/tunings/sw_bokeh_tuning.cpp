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
#define LOG_TAG "StereoTuningProvider_SWBOKEH"

#include "sw_bokeh_tuning.h"

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

SW_BOKEHTuning::SW_BOKEHTuning(json &tuningJson)
{
    _init(tuningJson);
}

SW_BOKEHTuning::~SW_BOKEHTuning()
{
}

bool
SW_BOKEHTuning::retrieveTuningParams(TuningQuery_T &query)
{
    string queryName(query.strParams[QUERY_KEY_APP]);
    TUNING_PAIR_LIST_T *params = NULL;
    if(query.results.find(QUERY_KEY_TUNING_PARAMS) != query.results.end()) {
        params = (TUNING_PAIR_LIST_T *)query.results[QUERY_KEY_TUNING_PARAMS];
    }

    QUERY_INT_LIST_T *clearTable = NULL;
    if(query.results.find(QUERY_KEY_CLEAR_RANGE_TABLE) != query.results.end()) {
        clearTable = (QUERY_INT_LIST_T *)query.results[QUERY_KEY_CLEAR_RANGE_TABLE];
    }

    MUINT32 *dofValue = NULL;
    MUINT32 dofLevel = 0;
    if(query.results.find(QUERY_KEY_DOF_VALUE) != query.results.end()) {
        dofLevel = query.intParams[QUERY_KEY_DOF_LEVEL];
        dofValue = (MUINT32 *)query.results[QUERY_KEY_DOF_VALUE];
    }

    QUERY_INT_T tuningMap = __galleryTuning;
    if(queryName == QUERY_KEY_CAMERA) {
        if(clearTable) {
            *clearTable = (__cameraClearTable.size() > 0) ? __cameraClearTable : __galleryClearTable;
        }

        if(NULL != dofValue) {
            if(__cameraDofTable.size() > 0) {
                *dofValue = __getDofValue(dofLevel, __cameraDofTable);
            } else if(__galleryDofTable.size() > 0) {
                *dofValue = __getDofValue(dofLevel, __galleryDofTable);
            } else {
                *dofValue = dofLevel * 2;
            }
        }

        //Override gallery with camera params
        if(params) {
            for(auto &c : __cameraTuning) {
                tuningMap[c.first] = c.second;
            }
        }
    } else {
        if(clearTable) {
            *clearTable = __galleryClearTable;
        }

        if(NULL != dofValue) {
            if(__galleryDofTable.size() > 0) {
                *dofValue = __getDofValue(dofLevel, __galleryDofTable);
            } else {
                *dofValue = dofLevel * 2;
            }
        }
    }

    if(params) {
        params->clear();
        for(auto &t : tuningMap) {
            params->push_back({t.first, t.second});
        }
    }

    return true;
}

MUINT32
SW_BOKEHTuning::__getDofValue(MUINT32 dofLevel, vector<MUINT32> &table)
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
SW_BOKEHTuning::_initDefaultValues()
{
    __galleryClearTable = __DEFAULT_CLEAR_TABLE;
    __galleryTuning =
    {
        { "refocus.ada_gain_en", 1 },
        { "refocus.gain_value", 80 },
        { "refocus.min_depth", 128 },
        { "refocus.bk_max_wt", 8 },
        { "refocus.cr", 32 },
        { "refocus.cr_gain", 4 },
        { "refocus.weighty", 224 },
        { "refocus.dofn", 8 },
        { "refocus.dofm", 8 },
        { "refocus.dofa", 2 },
        { "refocus.dofb", 4 },
        { "refocus.sigmak1", 20 },
        { "refocus.sigmak2", 1000 },
        { "refocus.sigmad", 20 },
        { "refocus.bkyth", 220 },
        { "refocus.filtmode1", 1 },
        { "refocus.filtmode2", 1 },
        { "refocus.enable_dia", 0 },
        { "refocus.wessiter1", 3 },
        { "refocus.wessiter2", 3 },
        { "refocus.gfr1", 3 },
        { "refocus.gfr2", 3 },
        { "refocus.bok_weight", 32 },
        { "refocus.bok_dof_div", 16 },
        { "refocus.bk_single_side", 0 },
        { "refocus.bk_ifocus", 0 },
        { "refocus.bk_ifocus_touch", 0 },
        { "refocus.bk_ifocus_ratio", 0 },
        { "refocus.max_strength", 13 },
        { "refocus.bg_mode", 1 },
        { "refocus.depth_preproc", 0 },
        { "refocus.blur_kernel", 5 },
        { "refocus.bok_circle_size", 3 },
        { "refocus.bok_circle_strength", 9 },
        { "refocus.bok_high_contrast_gain", 2 },
        { "refocus.bok_circle_min_layer", 6 },
        { "refocus.bok_circle_layer_num", 3 },
        { "refocus.bok_area_ratio_weight", 2 },
        { "refocus.bok_area_size_weight", 5 },
        { "refocus.bok_safe_layer_num", 2 },
        { "refocus.bok_suppress_table_option", 0 },
        { "refocus.debug_level", 0 },
        { "refocus.seg_mode", 255 },
        { "refocus.ruf", 0 },
        { "refocus.minrange", 4 },
        { "refocus.mingain", 80 },
        { "refocus.bweight", 1000 },
        { "refocus.fbs_sigma_x", 8 },
        { "refocus.fbs_sigma_y", 8 },
        { "refocus.fbs_sigma_luma", 8 },
        { "refocus.fbs_sigma_chroma", 8 },
        { "refocus.fbs_lambda", 32 },
        { "refocus.fbs_cg_tol", 1 },
        { "refocus.fbs_cg_maxiter", 25 },
        { "refocus.fbs_max_mem", 50 },
        { "refocus.fbs_use_confidence", 0 },
        { "refocus.fbs_mp_num", 4 },
        { "refocus.blur_max_fg", 128 },
        { "refocus.blur_max_bg", 255 },
        { "refocus.bok_img_erosion", 1 },
        { "refocus.blend_blurmap_filter", 0 },
        { "refocus.outlier_tolerance", 3 },
        { "refocus.outlier_percent", 3 },
        { "refocus.force_saveas", 0 },
    };

    __cameraTuning =
    {
        { "refocus.ruf", 1 }
    };
}

void
SW_BOKEHTuning::log(FastLogger &logger, bool inJSON)
{
    if(inJSON) {
        return StereoTuningBase::log(logger, inJSON);
    }

    logger.FastLogD("======== SW BOKEH Parameters ========");
    logger.FastLogD("-------- Gallery --------");
    std::ostringstream oss;
    for(auto i : __galleryClearTable) {
        oss << i << " ";
    }
    logger.FastLogD("Clear Table(%zu): %s", __galleryClearTable.size(), oss.str().c_str());

    if(__galleryDofTable.size() > 0) {
        oss.clear();
        oss.str("");
        for(size_t i = 0; i < __galleryDofTable.size(); ++i) {
            oss << __galleryDofTable[i] << " ";
        }
        logger.FastLogD("DoF Mapping(%zu): %s", __galleryDofTable.size(), oss.str().c_str());
    }

    for(auto &param : __galleryTuning) {
        logger.FastLogD("\"%s\": %d", param.first.c_str(), param.second);
    }

    logger.FastLogD("-------- Camera --------");
    oss.clear();
    oss.str("");
    if(__cameraClearTable.size() > 0) {
        for(auto i : __cameraClearTable) {
            oss << i << " ";
        }
        logger.FastLogD("Clear Table(%zu): %s", __cameraClearTable.size(), oss.str().c_str());
    }

    if(__cameraDofTable.size() > 0) {
        oss.clear();
        oss.str("");
        for(size_t i = 0; i < __cameraDofTable.size(); ++i) {
            oss << __cameraDofTable[i] << " ";
        }
        logger.FastLogD("DoF Mapping(%zu): %s", __cameraDofTable.size(), oss.str().c_str());
    }

    for(auto &param : __cameraTuning) {
        logger.FastLogD("\"%s\": %d", param.first.c_str(), param.second);
    }
    logger.FastLogD("=====================================");

    logger.print();
}

void
SW_BOKEHTuning::_loadValuesFromDocument(const json& refocusValues)
{
    const char *TUNING_PREFIX = "refocus.";
    const size_t TUNING_PREFIX_SIZE = strlen(TUNING_PREFIX);

    QUERY_INT_T *pTunings = NULL;
    QUERY_INT_LIST_T *pClearTable = NULL;
    vector<MUINT32> *pDofTable = NULL;

    for(auto &refocusValue : refocusValues) {
        if(LogicalCamJSONUtil::HasMember(refocusValue, VALUE_KEY_PARAMETERS) &&
           LogicalCamJSONUtil::HasMember(refocusValue[VALUE_KEY_PARAMETERS], QUERY_KEY_APP))
        {
            const json &tuningValues = refocusValue[VALUE_KEY_VALUES];
            if(QUERY_KEY_GALLERY == refocusValue[VALUE_KEY_PARAMETERS][QUERY_KEY_APP]) {
                pTunings = &__galleryTuning;
                pClearTable = &__galleryClearTable;
                pDofTable = &__galleryDofTable;
            } else {
                pTunings = &__cameraTuning;
                pClearTable = &__cameraClearTable;
                pDofTable = &__cameraDofTable;
            }

            //Extract tuning params
            pClearTable->clear();
            pDofTable->clear();

            bool hasClearTableArray = false;
            bool hasDofTableArray   = false;
            int v = 0;
            if(_isArray(tuningValues, QUERY_KEY_CLEAR_TABLE_PREFIX)) {
                hasClearTableArray = true;
                *pClearTable = tuningValues[QUERY_KEY_CLEAR_TABLE_PREFIX].get<QUERY_INT_LIST_T>();
            } else {
                int clearTableIndex  = 0;
                string clearTableKey = QUERY_KEY_CLEAR_TABLE_PREFIX+std::to_string(clearTableIndex);
                while(LogicalCamJSONUtil::HasMember(tuningValues, clearTableKey)) {
                    v = tuningValues[clearTableKey].get<int>();
                    if(v > 0) {
                        pClearTable->push_back(v);
                    } else {
                        MY_LOGW("Ignore invalid clear range(%d) for index %d", v, clearTableIndex);
                    }

                    clearTableIndex++;
                    clearTableKey = QUERY_KEY_CLEAR_TABLE_PREFIX+std::to_string(clearTableIndex);
                }
            }

            if(_isArray(tuningValues, QUERY_KEY_DOF_TABLE_PREFIX)) {
                hasDofTableArray = true;
                *pDofTable = tuningValues[QUERY_KEY_DOF_TABLE_PREFIX].get<vector<MUINT32>>();
            } else {
                int dofTableIndex  = 0;
                string doFTableKey = QUERY_KEY_DOF_TABLE_PREFIX+std::to_string(dofTableIndex);
                while(LogicalCamJSONUtil::HasMember(tuningValues, doFTableKey)) {
                    v = tuningValues[doFTableKey].get<int>();
                    if(v >= 0) {
                        pDofTable->push_back(v);
                    } else {
                        MY_LOGW("Ignore invalid DOF Value(%d) for index %d", v, dofTableIndex);
                    }

                    dofTableIndex++;
                    doFTableKey = QUERY_KEY_DOF_TABLE_PREFIX+std::to_string(dofTableIndex);
                }
            }

            pTunings->clear();
            for(json::const_iterator it = tuningValues.begin(); it != tuningValues.end(); ++it) {
                //Extract "refocus.xxxx" parameters
                if(it.key().length() > TUNING_PREFIX_SIZE &&
                   !strncmp(it.key().c_str(), TUNING_PREFIX, TUNING_PREFIX_SIZE))
                {
                    (*pTunings)[it.key()] = it.value().get<int>();
                }
            }

            if(pClearTable->size() == 0 &&
               pClearTable != &__cameraClearTable)
            {
               *pClearTable = __DEFAULT_CLEAR_TABLE;
            } else {
                std::sort(pClearTable->begin(), pClearTable->end());
            }

            auto last = std::unique(pDofTable->begin(), pDofTable->end());
            if(last != pDofTable->end()) {
                MY_LOGW("%zu duplicated %s found, please check \"%s\"s in %s",
                        pDofTable->end()-last, QUERY_KEY_DOF_TABLE_PREFIX, QUERY_KEY_DOF_TABLE_PREFIX, getTuningKey());
                pDofTable->erase(last, pDofTable->end());
            }
            std::sort(pDofTable->begin(), pDofTable->end());
        }
    }
}
