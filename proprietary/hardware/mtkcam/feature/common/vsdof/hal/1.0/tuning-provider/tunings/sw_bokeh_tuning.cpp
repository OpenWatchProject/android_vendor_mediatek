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

SW_BOKEHTuning::SW_BOKEHTuning(Value &document, SW_REFOCUSTuning *refocusTuning)
{
    __refocusTuning = refocusTuning;
    _init(document);
}

SW_BOKEHTuning::~SW_BOKEHTuning()
{
}

bool
SW_BOKEHTuning::retrieveTuningParams(TuningQuery_T &query)
{
    TUNING_PAIR_LIST_T *params = (TUNING_PAIR_LIST_T *)query.results[QUERY_KEY_TUNING_PARAMS];

    if(NULL == params) {
        MY_LOGE("Cannot get %s", QUERY_KEY_TUNING_PARAMS);
        return false;
    }
    *params = __mergedTuning;

    QUERY_INT_LIST_T *clearTable = (QUERY_INT_LIST_T *)query.results[QUERY_KEY_SWGF_TABLE];
    if(NULL == clearTable) {
        MY_LOGE("Cannot get %s", QUERY_KEY_SWGF_TABLE);
        return false;
    }
    *clearTable = (__clearTable.size() > 0) ? __clearTable : __refocusTuning->__clearTable;

    return true;
}

void
SW_BOKEHTuning::_initDefaultValues()
{
    // __clearTable = { 3, 3, 3, 4, 6, 8, 12, 16, 20, 24, 28, 32, 32, 32, 32, 32, 32 };
    __bokehTuning =
    {
        //Force to re-generate bokeh
        {"refocus.ruf",             1 },

        //gendepth
        // {"refocus.srh",           255 },
        // {"refocus.srv",            24 },
        // {"refocus.gain",            4 },
        // {"refocus.intplmode",       2 },
        // {"refocus.intplwt",         3 },
        // {"refocus.intplpref",       0 },
        // {"refocus.wmifull",         0 },
        // {"refocus.ada_gain_en",     1 },
        // {"refocus.gain_value",     80 },
        // {"refocus.min_depth",     128 },

        //refocus
        // {"refocus.bk_max_wt",      10 },
        // {"refocus.cr",             32 },
        // {"refocus.cr_gain",         4 },

        // {"refocus.weighty",        16 },
        // {"refocus.dofn",            6 },
        // {"refocus.dofm",            8 },
        // {"refocus.dofa",            4 },
        // {"refocus.dofb",            3 },
        // {"refocus.sigmak1",        20 },
        // {"refocus.sigmak2",      1000 },
        // {"refocus.sigmad",         20 },
        // {"refocus.diabnd1",         0 },
        // {"refocus.diabnd2",         0 },
        // {"refocus.diabnd3",         0 },
        // {"refocus.diayth",         32 },
        // {"refocus.bkyth",         220 },
        // {"refocus.filtmode1",       1 },
        // {"refocus.filtmode2",       1 },
        // {"refocus.sigma_r1",      200 },
        // {"refocus.sigma_s1",       30 },
        // {"refocus.sigma_r2",      200 },
        // {"refocus.sigma_s2",       15 },
        // {"refocus.filttype1",       0 },
        // {"refocus.filttype2",       0 },
        // {"refocus.enable_dia",      0 },
        // {"refocus.wessiter1",       3 },
        // {"refocus.wessiter2",       3 },
        // {"refocus.dtfiter",         1 },
        // {"refocus.gfr1",            3 },
        // {"refocus.gfr2",            3 },
        // {"refocus.bok_weight",     32 },
        // {"refocus.bok_dof_div",    16 },
        // {"refocus.bk_single_side",  0 },
        // {"refocus.max_strength",   13 },
        // {"refocus.bg_mode",         1 },
        // {"refocus.depth_preproc",   0 },
        // {"refocus.bok_backward",    1 },

        //common
        // {"refocus.lpboxen",         1 },
        // {"refocus.printdata",       0 },
        // {"refocus.wmi2x",           0 },
        // {"refocus.gf_en",           1 },
        // {"refocus.debug_level",     0 },

        //imgseg
        // {"refocus.seg_mode",      255 },
        // {"refocus.flmk_th",        94 },
        // {"refocus.seg_fd_th",       9 },
        // {"refocus.seg_fg_cof",    384 },
        // {"refocus.seg_bg_cof",    640 },

        // //REFOCUS_BLURMAP_CURVE
        // {"refocus.bmap_curve",      0 },

        //bokeh
        // {"refocus.minrange",        4 },
        // {"refocus.mingain",        28 },
        // {"refocus.bweight",       350 },
        // {"refocus.bndratioi",       0 },
        // {"refocus.bndratioh",       0 },
        // {"refocus.bndratiov",       0 },

        // fbs
        // {"refocus.fbs_sigma_x",        8 },
        // {"refocus.fbs_sigma_y",        8 },
        // {"refocus.fbs_sigma_luma",     8 },
        // {"refocus.fbs_sigma_chroma",   8 },
        // {"refocus.fbs_lambda",       128 },
        // {"refocus.fbs_cg_tol",         1 },
        // {"refocus.fbs_cg_maxiter",    25 },
        // {"refocus.fbs_max_mem",       50 },
        // {"refocus.fbs_use_confidence", 0 },

        // {"refocus.blur_max_fg",          128 },
        // {"refocus.blur_max_bg",          255 },
        // {"refocus.bok_img_erosion",        0 },
        // {"refocus.blend_blurmap_filter",   0 },
        // {"refocus.outlier_tolerance",      3 },
        // {"refocus.outlier_percent",        3 },
    };

    __mergeTuning();
}

void
SW_BOKEHTuning::log(FastLogger &logger, bool inJSON)
{
    if(inJSON) {
        return StereoTuningBase::log(logger, inJSON);
    }

    logger.FastLogD("======== SW BOKEH Parameters ========\nDiff to Refocus:");
    if(__clearTable.size() > 0) {
        std::ostringstream oss;
        for(auto i : __clearTable) {
            oss << i << " ";
        }
        logger.FastLogD("Clear Table(%d): %s", __clearTable.size(), oss.str().c_str());
    }

    for(auto &param : __bokehTuning) {
        logger.FastLogD("\"%s\": %d", param.first.c_str(), param.second);
    }
    logger.FastLogD("---------------------------------------------");
    logger.print();
}

void
SW_BOKEHTuning::_initDocument()
{
    // "SW_BOKEH": {
    //     "refocus.ruf": 1,
    //     ...
    // }

    Value bokehValue(kObjectType);
    for(auto &param : __bokehTuning) {
        if(param.first != QUERY_KEY_SWGF_TABLE) {
            bokehValue.AddMember(StringRef(param.first.c_str()), param.second, _allocator);
        }
    }

    if(__clearTable.size() > 0) {
        Value clearTable(kArrayType);
        #define AddToClearTable(value) clearTable.PushBack(Value().SetInt(value), _allocator)
        for(auto v : __clearTable) {
            AddToClearTable(v);
        }
        bokehValue.AddMember(QUERY_KEY_SWGF_TABLE, clearTable.Move(), _allocator);
    }

    Value key;
    key.SetString(StringRef(getTuningKey(), strlen(getTuningKey())));
    _document.AddMember(key, bokehValue, _allocator);
}

void
SW_BOKEHTuning::_loadValuesFromDocument()
{
    Value& bokehValues = _document[getTuningKey()];
    __bokehTuning.clear();
    for(auto &m : bokehValues.GetObject()) {
        if(m.name != QUERY_KEY_SWGF_TABLE) {
            __bokehTuning.push_back({m.name.GetString(), m.value.GetInt()});
        }
    }

    __clearTable.clear();
    if(bokehValues.HasMember(QUERY_KEY_SWGF_TABLE)) {
        const Value &clearTableValue = bokehValues[QUERY_KEY_SWGF_TABLE];
        for(auto &v : clearTableValue.GetArray()) {
            __clearTable.push_back(v.GetInt());
        }
    }

    __mergeTuning();
}

void
SW_BOKEHTuning::__mergeTuning()
{
    for(auto &param : __refocusTuning->__refocusTuning) {
        auto it = std::find_if( __bokehTuning.begin(), __bokehTuning.end(),
                  [param](const std::pair<std::string, int>& p) { return p.first == param.first;} );
        if(it == __bokehTuning.end()) {
            __mergedTuning.push_back(param);
        } else {
            __mergedTuning.push_back(*it);
        }
    }
}