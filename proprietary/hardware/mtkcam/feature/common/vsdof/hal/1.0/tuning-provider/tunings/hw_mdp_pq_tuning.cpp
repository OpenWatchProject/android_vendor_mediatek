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

HW_MDPPQTuning::HW_MDPPQTuning(Value &document)
{
    _init(document);
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
        params->isRefocus        = 1;
        params->defaultUpTable   = 0;
        params->defaultDownTable = 0;
        params->IBSEGain         = 0;
        __mdpPQParams[queryName] = *params;
    }

    return result;
}

void
HW_MDPPQTuning::_initDefaultValues()
{
    for(int i = 0; i < PASS2_ROUND_COUNT; ++i) {
        __mdpPQParams[PASS2_ROUND_NAME[i]].isRefocus        = 1;
        __mdpPQParams[PASS2_ROUND_NAME[i]].defaultUpTable   = 0;
        __mdpPQParams[PASS2_ROUND_NAME[i]].defaultDownTable = 0;
        __mdpPQParams[PASS2_ROUND_NAME[i]].IBSEGain         = 0;
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
              .FastLogD("isRefocus:        %d", tuning.isRefocus)
              .FastLogD("defaultUpTable:   %d", tuning.defaultUpTable)
              .FastLogD("defaultDownTable: %d", tuning.defaultDownTable)
              .FastLogD("IBSEGain:         %d", tuning.IBSEGain)
              .FastLogD("-------------------------------");
    }

    logger.print();
}

void
HW_MDPPQTuning::_initDocument()
{
    // "HW_PASS2_MDP": [
    //     {
    //         "Parameters": {
    //             "MDP_PQ_Name": "PASS2A"
    //         },
    //         "Values": {
    //             "isRefocus": 1,
    //             "defaultUpTable": 0,
    //             "defaultDownTable": 0,
    //             "IBSEGain": 0
    //         }
    //     },
    //     ...
    // ]

    Value pass2MDPValue(kArrayType);
    for(auto &param : __mdpPQParams) {
        VSDOFParam &tuning = param.second;

        Value resultValue(kObjectType);
        // "Parameters": {
        //     "MDP_PQ_Name": "PASS2A"
        // },
        Value parameters(kObjectType);
        parameters.AddMember(QUERY_KEY_MDP_PQ_NAME, StringRef(param.first.c_str()), _allocator);
        resultValue.AddMember(VALUE_KEY_PARAMETERS, parameters, _allocator);

        // "Values": {
        //     "isRefocus": 1,
        //     "defaultUpTable": 0,
        //     "defaultDownTable": 0,
        //     "IBSEGain": 0
        // }
        Value tuningValue(kObjectType);
        #define AddTuning(key, value) tuningValue.AddMember(key, Value().SetInt(value), _allocator)
        AddTuning("isRefocus", tuning.isRefocus);
        AddTuning("defaultUpTable", tuning.defaultUpTable);
        AddTuning("defaultDownTable", tuning.defaultDownTable);
        AddTuning("IBSEGain", tuning.IBSEGain);
        resultValue.AddMember(VALUE_KEY_VALUES, tuningValue.Move(), _allocator);

        pass2MDPValue.PushBack(resultValue.Move(), _allocator);
    }

    Value key;
    key.SetString(StringRef(getTuningKey(), strlen(getTuningKey())));
    _document.AddMember(key, pass2MDPValue, _allocator);
}

void
HW_MDPPQTuning::_loadValuesFromDocument()
{
    Value& mdpPQValues = _document[getTuningKey()];

    __mdpPQParams.clear();
    for(SizeType i = 0; i < mdpPQValues.Size(); i++) {
        const Value &value = mdpPQValues[i];
        string name(value[VALUE_KEY_PARAMETERS][QUERY_KEY_MDP_PQ_NAME].GetString());

        const Value &tuningValues = value[VALUE_KEY_VALUES];
        VSDOFParam tuning;
        tuning.isRefocus        = tuningValues["isRefocus"].GetInt();
        tuning.defaultUpTable   = tuningValues["defaultUpTable"].GetInt();
        tuning.defaultDownTable = tuningValues["defaultDownTable"].GetInt();
        tuning.IBSEGain         = tuningValues["IBSEGain"].GetInt();
        __mdpPQParams[name] = tuning;
    }
}
