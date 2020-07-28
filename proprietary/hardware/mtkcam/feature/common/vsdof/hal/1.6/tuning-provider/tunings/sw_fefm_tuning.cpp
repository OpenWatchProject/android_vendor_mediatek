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
#define LOG_TAG "StereoTuningProvider_SWFEFM"

#include "sw_fefm_tuning.h"

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

SW_FEFMTuning::SW_FEFMTuning(Value &document)
{
    _init(document);
}

SW_FEFMTuning::~SW_FEFMTuning()
{
}

bool
SW_FEFMTuning::retrieveTuningParams(TuningQuery_T &query)
{
    MTK_FEFM_TUNING_PARA_STRUCT *params = (MTK_FEFM_TUNING_PARA_STRUCT *)query.results[QUERY_KEY_SWFEFM_INFO];

    if(NULL == params) {
        MY_LOGE("Cannot get %s", QUERY_KEY_SWFEFM_INFO);
        return false;
    }

    *params = __tuningInfo;
    return true;
}

void
SW_FEFMTuning::_initDefaultValues()
{
    __tuningInfo.thr_cr          = 1 ;
    __tuningInfo.thr_grd         = 1 ;
    __tuningInfo.match_ratio     = 90 ;
    __tuningInfo.search_range_xL = 15 ;
    __tuningInfo.search_range_xR = 5 ;
    __tuningInfo.search_range_yT = 5 ;
    __tuningInfo.search_range_yD = 5 ;
}

void
SW_FEFMTuning::log(FastLogger &logger, bool inJSON)
{
    if(inJSON) {
        return StereoTuningBase::log(logger, inJSON);
    }

    logger
    .FastLogD("======= SW FEFM Parameters =======")
    .FastLogD("thr_cr %d", __tuningInfo.thr_cr)
    .FastLogD("thr_grd %d", __tuningInfo.thr_grd)
    .FastLogD("match_ratio %d", __tuningInfo.match_ratio)
    .FastLogD("search_range_xL %d", __tuningInfo.search_range_xL)
    .FastLogD("search_range_xR %d", __tuningInfo.search_range_xR)
    .FastLogD("search_range_yT %d", __tuningInfo.search_range_yT)
    .FastLogD("search_range_yD %d", __tuningInfo.search_range_yD)
    .print();
}

void
SW_FEFMTuning::_initDocument()
{
    // "SW_FEFM": {
    //     "thr_cr": 0,
    //     "thr_grd": 0,
    //     "match_ratio": 0,
    //     "search_range_xL": 0,
    //     "search_range_xR": 0,
    //     "search_range_yT": 0,
    //     "search_range_yD": 0
    // }

    Value swfefmValue(kObjectType);
    #define AddTuning(key, value) swfefmValue.AddMember(key, Value().SetInt(value), _allocator)
    AddTuning("thr_cr", __tuningInfo.thr_cr);
    AddTuning("thr_grd", __tuningInfo.thr_grd);
    AddTuning("match_ratio", __tuningInfo.match_ratio);
    AddTuning("search_range_xL", __tuningInfo.search_range_xL);
    AddTuning("search_range_xR", __tuningInfo.search_range_xR);
    AddTuning("search_range_yT", __tuningInfo.search_range_yT);
    AddTuning("search_range_yD", __tuningInfo.search_range_yD);

    Value key;
    key.SetString(StringRef(getTuningKey(), strlen(getTuningKey())));
    _document.AddMember(key, swfefmValue, _allocator);
}

void
SW_FEFMTuning::_loadValuesFromDocument()
{
    Value& swfefmValue = _document[getTuningKey()];
    __tuningInfo.thr_cr          = swfefmValue["thr_cr"].GetInt();
    __tuningInfo.thr_grd         = swfefmValue["thr_grd"].GetInt();
    __tuningInfo.match_ratio     = swfefmValue["match_ratio"].GetInt();
    __tuningInfo.search_range_xL = swfefmValue["search_range_xL"].GetInt();
    __tuningInfo.search_range_xR = swfefmValue["search_range_xR"].GetInt();
    __tuningInfo.search_range_yT = swfefmValue["search_range_yT"].GetInt();
    __tuningInfo.search_range_yD = swfefmValue["search_range_yD"].GetInt();
}
