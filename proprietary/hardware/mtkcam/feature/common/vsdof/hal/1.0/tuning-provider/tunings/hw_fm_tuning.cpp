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
#define LOG_TAG "StereoTuningProvider_HWFM"

#include "hw_fm_tuning.h"

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

const char *QUERY_KEY_FM_DIRECTION_NAMES[] =
{
    "Right to Left",
    "Left to Right"
};

HW_FMTuning::HW_FMTuning(Value &document)
{
    __fminfoMap[E_FM_R_TO_L] = FMInfo();
    __fminfoMap[E_FM_L_TO_R] = FMInfo();

    _init(document);
}

HW_FMTuning::~HW_FMTuning()
{
}

bool
HW_FMTuning::retrieveTuningParams(TuningQuery_T &query)
{
    FMInfo *fmInfo = (FMInfo *)query.results[QUERY_KEY_HWFM_INFO];

    if(NULL == fmInfo) {
        MY_LOGE("Cannot get %s", QUERY_KEY_HWFM_INFO);
        return false;
    }

    bool result = true;
    int direction = query.intParams[QUERY_KEY_HWFM_DIRECTION];
    if(__fminfoMap.find(direction) != __fminfoMap.end()) {
        ::memcpy(fmInfo, &__fminfoMap[direction], sizeof(FMInfo));
    } else {
        MY_LOGE("Unknown direction: %d", direction);
        result = false;
    }

    return result;
}

void
HW_FMTuning::_initDefaultValues()
{
    {
        FMInfo &fmInfo = __fminfoMap[E_FM_R_TO_L];
        fmInfo.mFMOFFSET_X  = 20;
        fmInfo.mFMOFFSET_Y  = 16;
        fmInfo.mFMSR_TYPE   = 0;
        fmInfo.mFMMIN_RATIO = 90;
        fmInfo.mFMSAD_TH    = 1023;
        fmInfo.mFMRES_TH    = 0;
    }

    {
        FMInfo &fmInfo = __fminfoMap[E_FM_L_TO_R];
        fmInfo.mFMOFFSET_X  = 12;
        fmInfo.mFMOFFSET_Y  = 16;
        fmInfo.mFMSR_TYPE   = 0;
        fmInfo.mFMMIN_RATIO = 90;
        fmInfo.mFMSAD_TH    = 1023;
        fmInfo.mFMRES_TH    = 0;
    }
}

void
HW_FMTuning::log(FastLogger &logger, bool inJSON)
{
    if(inJSON) {
        return StereoTuningBase::log(logger, inJSON);
    }

    logger.FastLogD("======== FM Parameters ========");
    for (FM_MAP_T::iterator it = __fminfoMap.begin(); it != __fminfoMap.end(); ++it) {
        int direction = it->first;
        FMInfo &fmInfo = it->second;

        logger
        .FastLogD("Direction: %s", QUERY_KEY_FM_DIRECTION_NAMES[direction])
        .FastLogD("mFMOFFSET_X:  %d", fmInfo.mFMOFFSET_X)
        .FastLogD("mFMOFFSET_Y:  %d", fmInfo.mFMOFFSET_Y)
        .FastLogD("mFMSR_TYPE:   %d", fmInfo.mFMSR_TYPE)
        .FastLogD("mFMMIN_RATIO: %d", fmInfo.mFMMIN_RATIO)
        .FastLogD("mFMSAD_TH:    %d", fmInfo.mFMSAD_TH)
        .FastLogD("mFMRES_TH:    %d", fmInfo.mFMRES_TH)
        .FastLogD("-------------------------------");
    }

    logger.print();
}

void
HW_FMTuning::_initDocument()
{
    // "HW_FM": [
    //     {
    //         "Parameters": {
    //             "Direction": "Left->Right"
    //         },
    //         "Values": {
    //             "mFMOFFSET_X": 0,
    //             "mFMOFFSET_Y": 0,
    //             "mFMSR_TYPE": 0,
    //             "mFMMIN_RATIO": 0,
    //             "mFMSAD_TH": 0,
    //             "mFMRES_TH": 0
    //         }
    //     },
    //     ...
    // ]

    Value fmValue(kArrayType);

    for (FM_MAP_T::iterator it=__fminfoMap.begin(); it!=__fminfoMap.end(); ++it) {
        int direction = it->first;
        FMInfo &fmInfo = it->second;

        Value resultValue(kObjectType);
        // "Parameters": {
        //     "Direction": "Right->Left"
        // },
        Value parameters(kObjectType);
        parameters.AddMember(QUERY_KEY_HWFM_DIRECTION, StringRef(QUERY_KEY_FM_DIRECTION_NAMES[direction]), _allocator);
        resultValue.AddMember(VALUE_KEY_PARAMETERS, parameters, _allocator);

        // "Values": {
        //     "mFMOFFSET_X": 0,
        //     "mFMOFFSET_Y": 0,
        //     "mFMSR_TYPE": 0,
        //     "mFMMIN_RATIO": 0,
        //     "mFMSAD_TH": 0,
        //     "mFMRES_TH": 0
        // }
        Value tuningValue(kObjectType);
        #define AddTuning(key, value) tuningValue.AddMember(key, Value().SetInt(value), _allocator)
        AddTuning("mFMOFFSET_X", fmInfo.mFMOFFSET_X);
        AddTuning("mFMOFFSET_Y", fmInfo.mFMOFFSET_Y);
        AddTuning("mFMSR_TYPE", fmInfo.mFMSR_TYPE);
        AddTuning("mFMMIN_RATIO", fmInfo.mFMMIN_RATIO);
        AddTuning("mFMSAD_TH", fmInfo.mFMSAD_TH);
        AddTuning("mFMRES_TH", fmInfo.mFMRES_TH);
        resultValue.AddMember(VALUE_KEY_VALUES, tuningValue.Move(), _allocator);

        fmValue.PushBack(resultValue.Move(), _allocator);
    }

    Value key;
    key.SetString(StringRef(getTuningKey(), strlen(getTuningKey())));
    _document.AddMember(key, fmValue, _allocator);
}

void
HW_FMTuning::_loadValuesFromDocument()
{
    Value& feValues = _document[getTuningKey()];

    for(SizeType i = 0; i < feValues.Size(); i++) {
        const Value &value = feValues[i];
        const char *dirName = value[VALUE_KEY_PARAMETERS][QUERY_KEY_HWFM_DIRECTION].GetString();
        int direction = 0;
        if(!strncmp(QUERY_KEY_FM_DIRECTION_NAMES[1],
                    value[VALUE_KEY_PARAMETERS][QUERY_KEY_HWFM_DIRECTION].GetString(),
                    strlen(QUERY_KEY_FM_DIRECTION_NAMES[1])))
        {
            direction = 1;
        }

        if(__fminfoMap.find(direction) == __fminfoMap.end()) {
            MY_LOGE("Unkown direction %d", direction);
            continue;
        }

        FMInfo &fmInfo = __fminfoMap[direction];
        const Value &tuningValues = value[VALUE_KEY_VALUES];
        fmInfo.mFMOFFSET_X  = tuningValues["mFMOFFSET_X"].GetInt();
        fmInfo.mFMOFFSET_Y  = tuningValues["mFMOFFSET_Y"].GetInt();
        fmInfo.mFMSR_TYPE   = tuningValues["mFMSR_TYPE"].GetInt();
        fmInfo.mFMMIN_RATIO = tuningValues["mFMMIN_RATIO"].GetInt();
        fmInfo.mFMSAD_TH    = tuningValues["mFMSAD_TH"].GetInt();
        fmInfo.mFMRES_TH    = tuningValues["mFMRES_TH"].GetInt();
    }
}
