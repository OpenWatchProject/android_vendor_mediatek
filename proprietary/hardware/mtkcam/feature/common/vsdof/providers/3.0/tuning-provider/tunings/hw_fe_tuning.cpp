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
#define LOG_TAG "StereoTuningProvider_HWFE"

#include "hw_fe_tuning.h"

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

HW_FETuning::HW_FETuning(Value &document)
{
    __feinfoMap[8]  = FEInfo();
    __feinfoMap[16] = FEInfo();

    _init(document);
}

HW_FETuning::~HW_FETuning()
{
}

bool
HW_FETuning::retrieveTuningParams(TuningQuery_T &query)
{
    FEInfo *feInfo = (FEInfo *)query.results[QUERY_KEY_HWFE_INFO];

    if(NULL == feInfo) {
        MY_LOGE("Cannot get %s", QUERY_KEY_HWFE_INFO);
        return false;
    }

    bool result = true;
    int blockSize = query.intParams[QUERY_KEY_HWFE_BLOCKSIZE];
    if(__feinfoMap.find(blockSize) != __feinfoMap.end()) {
        ::memcpy(feInfo, &__feinfoMap[blockSize], sizeof(FEInfo));
    } else {
        MY_LOGE("Unknown block size: %d", blockSize);
        result = false;
    }

    return result;
}

void
HW_FETuning::_initDefaultValues()
{
    {
        FEInfo &feInfo = __feinfoMap[16];
        feInfo.mFEMODE      = 1;
        feInfo.mFEPARAM     = 4;
        feInfo.mFEFLT_EN    = 1;
        feInfo.mFETH_G      = 1;
        feInfo.mFETH_C      = 0;
        feInfo.mFEDSCR_SBIT = 3;
        feInfo.mFEXIDX      = 0;
        feInfo.mFEYIDX      = 0;
        feInfo.mFESTART_X   = 0;
        feInfo.mFESTART_Y   = 0;
        feInfo.mFEIN_HT     = 0;
        feInfo.mFEIN_WD     = 0;
    }

    {
        FEInfo &feInfo = __feinfoMap[8];
        feInfo.mFEMODE      = 2;
        feInfo.mFEPARAM     = 4;
        feInfo.mFEFLT_EN    = 1;
        feInfo.mFETH_G      = 1;
        feInfo.mFETH_C      = 0;
        feInfo.mFEDSCR_SBIT = 3;
        feInfo.mFEXIDX      = 0;
        feInfo.mFEYIDX      = 0;
        feInfo.mFESTART_X   = 0;
        feInfo.mFESTART_Y   = 0;
        feInfo.mFEIN_HT     = 0;
        feInfo.mFEIN_WD     = 0;
    }
}

void
HW_FETuning::log(FastLogger &logger, bool inJSON)
{
    if(inJSON) {
        return StereoTuningBase::log(logger, inJSON);
    }

    logger.FastLogD("======== FE Parameters ========");
    for (FE_MAP_T::iterator it = __feinfoMap.begin(); it != __feinfoMap.end(); ++it) {
        int blockSize = it->first;
        FEInfo &feInfo = it->second;

        logger
        .FastLogD("BlockSize: %d", blockSize)
        .FastLogD("mFEMODE:      %d", feInfo.mFEMODE)
        .FastLogD("mFEPARAM:     %d", feInfo.mFEPARAM)
        .FastLogD("mFEFLT_EN:    %d", feInfo.mFEFLT_EN)
        .FastLogD("mFETH_G:      %d", feInfo.mFETH_G)
        .FastLogD("mFETH_C:      %d", feInfo.mFETH_C)
        .FastLogD("mFEDSCR_SBIT: %d", feInfo.mFEDSCR_SBIT)
        .FastLogD("mFEXIDX:      %d", feInfo.mFEXIDX)
        .FastLogD("mFEYIDX:      %d", feInfo.mFEYIDX)
        .FastLogD("mFESTART_X:   %d", feInfo.mFESTART_X)
        .FastLogD("mFESTART_Y:   %d", feInfo.mFESTART_Y)
        .FastLogD("mFEIN_HT:     %d", feInfo.mFEIN_HT)
        .FastLogD("mFEIN_WD:     %d", feInfo.mFEIN_WD)
        .FastLogD("-------------------------------");
    }

    logger.print();
}

void
HW_FETuning::_initDocument()
{
    // "HW_FE": [
    //     {
    //         "Parameters": {
    //             "BlockSize": 16
    //         },
    //         "Values": {
    //             "mFEMODE": 0,
    //             "mFEPARAM": 0,
    //             "mFEFLT_EN": 0,
    //             "mFETH_G": 0,
    //             "mFETH_C": 0,
    //             "mFEDSCR_SBIT": 0,
    //             "mFEXIDX": 0,
    //             "mFEYIDX": 0,
    //             "mFESTART_X": 0,
    //             "mFESTART_Y": 0,
    //             "mFEIN_HT": 0,
    //             "mFEIN_WD": 0
    //         }
    //     },
    //     ...
    // ]

    Value feValue(kArrayType);

    for (FE_MAP_T::iterator it=__feinfoMap.begin(); it!=__feinfoMap.end(); ++it) {
        int blockSize = it->first;
        FEInfo &feInfo = it->second;

        Value resultValue(kObjectType);
        // "Parameters": {
        //     "BlockSize": 16
        // },
        Value parameters(kObjectType);
        parameters.AddMember(QUERY_KEY_HWFE_BLOCKSIZE, blockSize, _allocator);
        resultValue.AddMember(VALUE_KEY_PARAMETERS, parameters, _allocator);

        // "Values": {
        //     "mFEMODE": 0,
        //     "mFEPARAM": 0,
        //     "mFEFLT_EN": 0,
        //     "mFETH_G": 0,
        //     "mFETH_C": 0,
        //     "mFEDSCR_SBIT": 0,
        //     "mFEXIDX": 0,
        //     "mFEYIDX": 0,
        //     "mFESTART_X": 0,
        //     "mFESTART_Y": 0,
        //     "mFEIN_HT": 0,
        //     "mFEIN_WD": 0
        // }
        Value tuningValue(kObjectType);
        #define AddTuning(key, value) tuningValue.AddMember(key, Value().SetInt(value), _allocator)
        AddTuning("mFEMODE", feInfo.mFEMODE);
        AddTuning("mFEPARAM", feInfo.mFEPARAM);
        AddTuning("mFEFLT_EN", feInfo.mFEFLT_EN);
        AddTuning("mFETH_G", feInfo.mFETH_G);
        AddTuning("mFETH_C", feInfo.mFETH_C);
        AddTuning("mFEDSCR_SBIT", feInfo.mFEDSCR_SBIT);
        AddTuning("mFEXIDX", feInfo.mFEXIDX);
        AddTuning("mFEYIDX", feInfo.mFEYIDX);
        AddTuning("mFESTART_X", feInfo.mFESTART_X);
        AddTuning("mFESTART_Y", feInfo.mFESTART_Y);
        AddTuning("mFEIN_HT", feInfo.mFEIN_HT);
        AddTuning("mFEIN_WD", feInfo.mFEIN_WD);
        resultValue.AddMember(VALUE_KEY_VALUES, tuningValue.Move(), _allocator);

        feValue.PushBack(resultValue.Move(), _allocator);
    }

    Value key;
    key.SetString(StringRef(getTuningKey(), strlen(getTuningKey())));
    _document.AddMember(key, feValue, _allocator);
}

void
HW_FETuning::_loadValuesFromDocument()
{
    Value& feValues = _document[getTuningKey()];

    for(SizeType i = 0; i < feValues.Size(); i++) {
        const Value &value = feValues[i];
        int blockSize = value[VALUE_KEY_PARAMETERS][QUERY_KEY_HWFE_BLOCKSIZE].GetInt();
        if(__feinfoMap.find(blockSize) == __feinfoMap.end()) {
            MY_LOGE("Unkown block size %d", blockSize);
            continue;
        }

        FEInfo &feInfo = __feinfoMap[blockSize];
        const Value &tuningValues = value[VALUE_KEY_VALUES];
        feInfo.mFEMODE      = tuningValues["mFEMODE"].GetInt();
        feInfo.mFEPARAM     = tuningValues["mFEPARAM"].GetInt();
        feInfo.mFEFLT_EN    = tuningValues["mFEFLT_EN"].GetInt();
        feInfo.mFETH_G      = tuningValues["mFETH_G"].GetInt();
        feInfo.mFETH_C      = tuningValues["mFETH_C"].GetInt();
        feInfo.mFEDSCR_SBIT = tuningValues["mFEDSCR_SBIT"].GetInt();
        feInfo.mFEXIDX      = tuningValues["mFEXIDX"].GetInt();
        feInfo.mFEYIDX      = tuningValues["mFEYIDX"].GetInt();
        feInfo.mFESTART_X   = tuningValues["mFESTART_X"].GetInt();
        feInfo.mFESTART_Y   = tuningValues["mFESTART_Y"].GetInt();
        feInfo.mFEIN_HT     = tuningValues["mFEIN_HT"].GetInt();
        feInfo.mFEIN_WD     = tuningValues["mFEIN_WD"].GetInt();
    }
}
