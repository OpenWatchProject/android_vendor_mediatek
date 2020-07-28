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
#define LOG_TAG "StereoTuningProvider_SWADP"

#include "sw_adp_tuning.h"

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

SW_ADPTuning::SW_ADPTuning(Value &document)
{
    _init(document);
}

SW_ADPTuning::~SW_ADPTuning()
{
}

bool
SW_ADPTuning::retrieveTuningParams(TuningQuery_T &query)
{
    if(query.results.find(QUERY_KEY_TUNING_PARAMS) != query.results.end()) {
        TUNING_PAIR_LIST_T *params = (TUNING_PAIR_LIST_T *)query.results[QUERY_KEY_TUNING_PARAMS];

        if(params) {
            params->clear();
            for(auto &t : __tunings) {
                params->push_back({t.first, t.second});
            }

            return true;
        }
    }

    return false;
}

void
SW_ADPTuning::_initDefaultValues()
{
    __tunings = {
        { "adp.DvLsbShift", 0 },
        { "adp.DisparityShift", 511 },
        { "adp.DisparityShift4Color", 0 },
        { "adp.DisparityRatio4Color", 1250 },
        { "adp.DebugLevel", 0 }
    };
}

void
SW_ADPTuning::log(FastLogger &logger, bool inJSON)
{
    if(inJSON) {
        return StereoTuningBase::log(logger, inJSON);
    }

    logger.FastLogD("======== SW ADP Parameters ========");
    for(auto &param : __tunings) {
        logger.FastLogD("\"%s\": %d", param.first.c_str(), param.second);
    }
    logger.FastLogD("===================================");
    logger.print();
}

void
SW_ADPTuning::_initDocument()
{
    // "SW_ADP": [{
    //     "Parameters": {
    //         "APP": "Gallery"
    //     },
    //     "Values": {
    //         "refocus.srh": 255,
    //         ...

    Value adpValue(kArrayType);
    for(int i = 0; i < 2; ++i) {
        Value resultValue(kObjectType);
        // "Parameters": {
        // },
        Value parameters(kObjectType);
        resultValue.AddMember(VALUE_KEY_PARAMETERS, parameters, _allocator);

        // "Values": {
        //     ...
        // }
        Value tuningValue(kObjectType);

        //Tuning params
        for(auto &param : __tunings) {
            tuningValue.AddMember(StringRef(param.first.c_str()), param.second, _allocator);
        }

        adpValue.PushBack(resultValue.Move(), _allocator);
    }

    Value key;
    key.SetString(StringRef(getTuningKey(), strlen(getTuningKey())));
    _document.AddMember(key, adpValue, _allocator);
}

void
SW_ADPTuning::_loadValuesFromDocument()
{
    Value& adpValues = _document[getTuningKey()];

    for(SizeType i = 0; i < adpValues.Size(); i++) {
        const Value &value = adpValues[i];
        {
            const Value &tuningValues = value[VALUE_KEY_VALUES];
            //Extract tuning params
            int v = 0;
            __tunings.clear();
            for(auto &m : tuningValues.GetObject()) {
                __tunings[m.name.GetString()] = m.value.GetInt();
            }
        }
    }
}
