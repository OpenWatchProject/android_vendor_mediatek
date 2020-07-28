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

#define LOG_TAG "FlashAlgoAdapter"

#include <array>

/* aaa common headers */
#include "log_utils.h"
#include <private/aaa_utils.h>
#include <private/aaa_hal_private.h>
#include "flash_utils.h"

/* custom headers */
#include "flash_tuning_custom.h"
#include "camera_custom_nvram.h"

/* flash headers */
#include "flash_algo_adapter.h"
#include "flash_custom_adapter.h"
#include "flash_ae_general_type.h"
#include "flash_awb_general_type.h"
#include "flash_cal_general_type.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "flash_ae_interface_control.h"
#include "flash_awb_interface_control.h"
#include "flash_cal_interface_control.h"
#ifdef __cplusplus
}
#endif

/* kernel headers */
#include "kd_camera_feature.h"

FlashAlgoAdapter::FlashAlgoAdapter(int sensorDev)
    : mSensorDev(sensorDev)
{
    mSensorAlgo = NS3Av3::mapSensorDevToIdx(sensorDev);
    setDebug();
}

FlashAlgoAdapter::~FlashAlgoAdapter()
{
}

void FlashAlgoAdapter::init()
{
    FlashAEGroup groupIn;
    FlashAEGroup groupOut;

    logD("%s+", __func__);

    flashAEAlgoCtrl(MTK_FLASH_AE_INIT, &groupIn, &groupOut, mSensorAlgo);
}

void FlashAlgoAdapter::unInit()
{
    FlashAEGroup groupIn;
    FlashAEGroup groupOut;

    logD("%s+", __func__);

    flashAEAlgoCtrl(MTK_FLASH_AE_UNINIT, &groupIn, &groupOut, mSensorAlgo);
}

#define MY_INST NS3Av3::INST_T<FlashAlgoAdapter>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

FlashAlgoAdapter *FlashAlgoAdapter::getInstance(int sensorDev)
{
    int sensorOpenIndex = NS3Av3::mapSensorDevToIdx(sensorDev);

    if(sensorOpenIndex >= SENSOR_IDX_MAX || sensorOpenIndex < 0) {
        logE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, sensorOpenIndex);
        return NULL;
    }

    MY_INST& rSingleton = gMultiton[sensorOpenIndex];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<FlashAlgoAdapter>(sensorDev);
    } );

    return rSingleton.instance.get();
}

int FlashAlgoAdapter::Estimate(FlashAlgExpPara* exp, FlashAlgFacePos* pFaceInfo, int* isLowRef)
{
    FlashAEGroup groupIn;
    FlashAEGroup groupOut;

    FlashAEUnit unitIn;
    FlashAEUnit unitOut[2];

    FlashAEInput_FlashAlgFacePos faceInfoIn;
    FlashAEOutput_FlashAlgExpPara expOut;
    FlashAEOutput_IntPtr isLowRefOut;

    logD("%s+", __func__);

    faceInfoIn.input = pFaceInfo;
    expOut.output = exp;
    isLowRefOut.output = isLowRef;

    unitIn.type = FLASH_AE_INPUT_P_FACE_INFO;
    unitIn.data = &faceInfoIn;
    unitIn.size = sizeof(FlashAEInput_FlashAlgFacePos);

    unitOut[0].type = FLASH_AE_OUTPUT_P_EXP;
    unitOut[0].data = &expOut;
    unitOut[0].size = sizeof(FlashAEOutput_FlashAlgExpPara);

    unitOut[1].type = FLASH_AE_OUTPUT_P_IS_LOW_REF;
    unitOut[1].data = &isLowRefOut;
    unitOut[1].size = sizeof(FlashAEOutput_IntPtr);

    groupIn.number = 1;
    groupIn.units = &unitIn;

    groupOut.number = 2;
    groupOut.units = unitOut;

    flashAEAlgoCtrl(MTK_FLASH_AE_ESTIMATE, &groupIn, &groupOut, mSensorAlgo);
    logD("[Estimate out] %p %p %p %d", exp, pFaceInfo, isLowRef, mSensorAlgo);
    return 0;
}

int FlashAlgoAdapter::setFlashType(int type)
{
    FlashAEGroup groupIn;
    FlashAEGroup groupOut;

    FlashAEUnit unitIn;

    FlashAEInput_Int flashType;
    flashType.input = type;

    unitIn.type = FLASH_AE_INPUT_FLASH_TYPE;
    unitIn.data = &flashType;
    unitIn.size = sizeof(FlashAEInput_Int);

    groupIn.number = 1;
    groupIn.units = &unitIn;

    logD("%s+", __func__);

    flashAEAlgoCtrl(MTK_FLASH_AE_SET_FLASH_TYPE, &groupIn, &groupOut, mSensorAlgo);
    return 0;
}

int FlashAlgoAdapter::Reset()
{
    FlashAEGroup groupIn;
    FlashAEGroup groupOut;

    logD("%s+", __func__);

    flashAEAlgoCtrl(MTK_FLASH_AE_RESET, &groupIn, &groupOut, mSensorAlgo);
    return 0;
}

int FlashAlgoAdapter::AddStaData10(FlashAlgStaData* pData, FlashAlgExpPara* pExp,
        int* pIsNeedNext, FlashAlgExpPara* pExpNext, int last)
{
    FlashAEGroup groupIn;
    FlashAEGroup groupOut;
    FlashAEUnit unitIn[3];
    FlashAEUnit unitOut[2];
    FlashAEInput_FlashAlgStaData staData;
    FlashAEInput_FlashAlgExpPara expPara;
    FlashAEOutput_FlashAlgExpPara expParaNext;
    FlashAEOutput_IntPtr isNeedNext;
    FlashAEInput_Int isLast;

    logD("%s+", __func__);

    staData.input = pData;
    expPara.input = pExp;
    isNeedNext.output = pIsNeedNext;
    isLast.input = last;
    expParaNext.output = pExpNext;

    unitIn[0].type = FLASH_AE_INPUT_P_STA_DATA;
    unitIn[0].data = &staData;
    unitIn[0].size = sizeof(FlashAEInput_FlashAlgStaData);

    unitIn[1].type = FLASH_AE_INPUT_P_EXP;
    unitIn[1].data = &expPara;
    unitIn[1].size = sizeof(FlashAEInput_FlashAlgExpPara);

    unitIn[2].type = FLASH_AE_INPUT_P_LAST_FRAME;
    unitIn[2].data = &isLast;
    unitIn[2].size = sizeof(FlashAEInput_Int);

    unitOut[0].type = FLASH_AE_OUTPUT_P_IS_NEED_NEXT;
    unitOut[0].data = &isNeedNext;
    unitOut[0].size = sizeof(FlashAEOutput_IntPtr);

    unitOut[1].type = FLASH_AE_OUTPUT_P_EXP_NEXT;
    unitOut[1].data = &expParaNext;
    unitOut[1].size = sizeof(FlashAEOutput_FlashAlgExpPara);

    groupIn.number = 3;
    groupIn.units = unitIn;

    groupOut.number = 2;
    groupOut.units = unitOut;

    flashAEAlgoCtrl(MTK_FLASH_AE_ADDSTADATA10, &groupIn, &groupOut, mSensorAlgo);
    logD("[AddStaData10 out] %p %p %d %p %d",
            pData, pExp, *pIsNeedNext, pExpNext, last);

    return 0;
}

int FlashAlgoAdapter::CalFirstEquAEPara(FlashAlgExpPara* exp, FlashAlgExpPara* EquExp)
{
    FlashAEGroup groupIn;
    FlashAEGroup groupOut;

    FlashAEUnit unitIn;
    FlashAEUnit unitOut;

    FlashAEInput_FlashAlgExpPara expIn;
    FlashAEOutput_FlashAlgExpPara expOut;

    logD("%s+", __func__);
    expIn.input = exp;
    expOut.output = EquExp;

    unitIn.type = FLASH_AE_INPUT_P_EXP;
    unitIn.data = &expIn;
    unitIn.size = sizeof(FlashAEOutput_FlashAlgExpPara);

    unitOut.type = FLASH_AE_OUTPUT_P_EXP;
    unitOut.data = &expOut;
    unitOut.size = sizeof(FlashAEOutput_FlashAlgExpPara);

    groupIn.number = 1;
    groupIn.units = &unitIn;

    groupOut.number = 1;
    groupOut.units = &unitOut;

    flashAEAlgoCtrl(MTK_FLASH_AE_CAL_FIRST_EQU_AE_PARA, &groupIn, &groupOut, mSensorAlgo);
    logD("[CalFirstEquAEPara out] %p %p", exp, EquExp);
    return 0;
}

int FlashAlgoAdapter::setFlashProfile(FlashAlgStrobeProfile* pr)
{
    FlashAEGroup groupIn;
    FlashAEGroup groupOut;

    FlashAEUnit unitIn;

    FlashAEInput_FlashAlgStrobeProfile prIn;

    logD("%s+", __func__);
    prIn.input = pr;

    unitIn.type = FLASH_AE_INPUT_P_STROBE_PROFILE;
    unitIn.data = &prIn;
    unitIn.size = sizeof(FlashAEInput_FlashAlgStrobeProfile);

    groupIn.number = 1;
    groupIn.units = &unitIn;

    flashAEAlgoCtrl(MTK_FLASH_AE_SET_FLASH_PROFILE, &groupIn, &groupOut, mSensorAlgo);
    logD("[%s out] %p", __func__, pr);
    return 0;
}

int FlashAlgoAdapter::setCapturePLine(PLine* p, int isoAtGain1x)
{
    FlashAEGroup groupIn;
    FlashAEGroup groupOut;

    FlashAEUnit unitIn[2];

    FlashAEInput_PLine pIn;
    FlashAEInput_Int isoAtGain1xIn;
    logD("%s+", __func__);

    pIn.input = p;
    isoAtGain1xIn.input = isoAtGain1x;

    unitIn[0].type = FLASH_AE_INPUT_P_PLINE;
    unitIn[0].data = &pIn;
    unitIn[0].size = sizeof(FlashAEInput_FlashAlgStrobeProfile);

    unitIn[1].type = FLASH_AE_INPUT_ISO_AT_GAIN1X;
    unitIn[1].data = &isoAtGain1xIn;
    unitIn[1].size = sizeof(FlashAEInput_Int);

    groupIn.number = 2;
    groupIn.units = unitIn;

    flashAEAlgoCtrl(MTK_FLASH_AE_SET_CAPTURE_PLINE, &groupIn, &groupOut, mSensorAlgo);
    logD("[%s out] %p %d", __func__, p, isoAtGain1x);
    return 0;
}

int FlashAlgoAdapter::setPreflashPLine(PLine* p, int isoAtGain1x)
{
    FlashAEGroup groupIn;
    FlashAEGroup groupOut;

    FlashAEUnit unitIn[2];

    FlashAEInput_PLine pIn;
    FlashAEInput_Int isoAtGain1xIn;
    logD("%s+", __func__);

    pIn.input = p;
    isoAtGain1xIn.input = isoAtGain1x;

    unitIn[0].type = FLASH_AE_INPUT_P_PLINE;
    unitIn[0].data = &pIn;
    unitIn[0].size = sizeof(FlashAEInput_FlashAlgStrobeProfile);

    unitIn[1].type = FLASH_AE_INPUT_ISO_AT_GAIN1X;
    unitIn[1].data = &isoAtGain1xIn;
    unitIn[1].size = sizeof(FlashAEInput_Int);

    groupIn.number = 2;
    groupIn.units = unitIn;

    flashAEAlgoCtrl(MTK_FLASH_AE_SET_PREFLASH_PLINE, &groupIn, &groupOut, mSensorAlgo);
    logD("[%s out] %p %d", __func__, p, isoAtGain1x);
    return 0;
}

int FlashAlgoAdapter::setEVCompEx(float ev_comp, float tar, float evLevel)
{
    FlashAEGroup groupIn;
    FlashAEGroup groupOut;

    FlashAEUnit unitIn[3];
    FlashAEInput_Float ev_compIn, tarIn, evLevelIn;
    logD("%s+", __func__);

    ev_compIn.input = ev_comp;
    tarIn.input = tar;
    evLevelIn.input = evLevel;

    unitIn[0].type = FLASH_AE_INPUT_EV_COMP;
    unitIn[0].data = &ev_compIn;
    unitIn[0].size = sizeof(FlashAEInput_Float);

    unitIn[1].type = FLASH_AE_INPUT_TAR;
    unitIn[1].data = &tarIn;
    unitIn[1].size = sizeof(FlashAEInput_Float);

    unitIn[2].type = FLASH_AE_INPUT_EV_LEVEL;
    unitIn[2].data = &evLevelIn;
    unitIn[2].size = sizeof(FlashAEInput_Float);

    groupIn.number = 3;
    groupIn.units = unitIn;

    flashAEAlgoCtrl(MTK_FLASH_AE_SET_EV_COMP_EX, &groupIn, &groupOut, mSensorAlgo);
    logD("[%s out] %f %f %f", __func__, ev_comp, tar, evLevel);
    return 0;
}

int FlashAlgoAdapter::setDefaultPreferences()
{
    FlashAEGroup groupIn;
    FlashAEGroup groupOut;

    logD("%s+", __func__);

    flashAEAlgoCtrl(MTK_FLASH_AE_SET_DEFAULT_PREFERENCES, &groupIn, &groupOut, mSensorAlgo);
    return 0;
}

int FlashAlgoAdapter::setTuningPreferences(FLASH_TUNING_PARA *tuning_p, int lv)
{
    FlashAEGroup groupIn;
    FlashAEGroup groupOut;

    FlashAEUnit unitIn[2];

    FlashAEInput_FLASH_TUNING_PARA tuningIn;
    FlashAEInput_Int lvIn;

    logD("%s+", __func__);

    tuningIn.input = tuning_p;
    lvIn.input = lv;

    unitIn[0].type = FLASH_AE_INPUT_P_TUNING;
    unitIn[0].data = &tuningIn;
    unitIn[0].size = sizeof(FlashAEInput_FLASH_TUNING_PARA);

    unitIn[1].type = FLASH_AE_INPUT_LV;
    unitIn[1].data = &lvIn;
    unitIn[1].size = sizeof(FlashAEInput_Int);

    groupIn.number = 2;
    groupIn.units = unitIn;

    flashAEAlgoCtrl(MTK_FLASH_AE_SET_TUNING_PREFERENCES, &groupIn, &groupOut, mSensorAlgo);
    logD("[%s out] %p %d", __func__, tuning_p, lv);
    return 0;
}

int FlashAlgoAdapter::setFlashInfo(FlashAlgInputInfoStruct *pFlashInfo)
{
    FlashAWBGroup groupIn;
    FlashAWBGroup groupOut;

    FlashAWBUnit unitIn;

    FlashAWBInput_FlashAlgInputInfoStruct flashInfoIn;
    logD("%s+", __func__);

    flashInfoIn.input = pFlashInfo;

    unitIn.type = FLASH_AWB_INPUT_P_FLASH_INFO;
    unitIn.data = &flashInfoIn;
    unitIn.size = sizeof(FlashAWBInput_FlashAlgInputInfoStruct);

    groupIn.number = 1;
    groupIn.units = &unitIn;

    flashAWBAlgoCtrl(mSensorAlgo, MTK_FLASH_AWB_SET_FLASH_INFO, &groupIn, &groupOut);
    logD("[%s out] %p", __func__, pFlashInfo);
    return 0;

}

int FlashAlgoAdapter::Flash_Awb_Init(FLASH_AWB_INIT_T &FlashAwbInit)
{
    FlashAWBGroup groupIn;
    FlashAWBGroup groupOut;

    FlashAWBUnit unitIn;

    FlashAWBInput_FLASH_AWB_INIT_T flashAwbInitIn;

    logD("%s+", __func__);

    flashAwbInitIn.input = &FlashAwbInit;

    unitIn.type = FLASH_AWB_INPUT_FLASH_AWB_INIT_T;
    unitIn.data = &flashAwbInitIn;
    unitIn.size = sizeof(FlashAWBInput_FLASH_AWB_INIT_T);

    groupIn.number = 1;
    groupIn.units = &unitIn;

    flashAWBAlgoCtrl(mSensorAlgo, MTK_FLASH_AWB_INIT, &groupIn, &groupOut);
    logD("[%s out] %p", __func__, &FlashAwbInit);
    return 0;
}

int FlashAlgoAdapter::Flash_Awb_Algo(int Exp, int Iso, MUINT32 *FlashResultWeight)
{
    FlashAWBGroup groupIn;
    FlashAWBGroup groupOut;

    FlashAWBUnit unitIn[2];
    FlashAWBUnit unitOut;
    FlashAWBInput_Int exp,iso;
    FlashAWBOutput_RESULT_WEIGHT flashResultWeightOut;

    logD("%s+", __func__);
    exp.input = Exp;
    iso.input = Iso;
    flashResultWeightOut.output = FlashResultWeight;

    unitIn[0].type = FLASH_AWB_INPUT_EXP;
    unitIn[0].data = &exp;
    unitIn[0].size = sizeof(FlashAWBInput_Int);
    unitIn[1].type = FLASH_AWB_INPUT_ISO;
    unitIn[1].data = &iso;
    unitIn[1].size = sizeof(FlashAWBInput_Int);

    unitOut.type = FLASH_AWB_OUTPUT_P_RESULT_WEIGHT;
    unitOut.data = &flashResultWeightOut;
    unitOut.size = sizeof(FlashAWBOutput_RESULT_WEIGHT);

    groupIn.number = 2;
    groupIn.units = unitIn;
    groupOut.number = 1;
    groupOut.units = &unitOut;

    flashAWBAlgoCtrl(mSensorAlgo, MTK_FLASH_AWB_ALGO, &groupIn, &groupOut);
    logD("[%s out] %d %d %p", __func__, Exp, Iso, FlashResultWeight);
    return 0;
}

MRESULT FlashAlgoAdapter::DecideCalFlashComb(int CalNum,
        NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT *pNvCali, int totalStep,
        int totalStepLT, ChooseResult *pChoose, int dualFlashEnable, int caliNvIdx)
{
    FlashCalGroup groupIn;
    FlashCalGroup groupOut;

    FlashCalUnit unitIn[5];
    FlashCalUnit unitOut;

    FlashCalInput_Int calNumIn, totalStepIn, totalStepLTIn;
    FlashCalInput_ShortPtr yTabIn;
    FlashCalInput_Int dualFlashEnableIn;
    FlashCalOutput_ChooseResult chooseOut;

    logD("%s+", __func__);

    calNumIn.input = CalNum;
    yTabIn.input = pNvCali->Flash_Calibration[caliNvIdx].engTab.yTab;
    totalStepIn.input = totalStep;
    totalStepLTIn.input = totalStepLT;
    dualFlashEnableIn.input = dualFlashEnable;
    chooseOut.output = pChoose;

    unitIn[0].type = FLASH_CAL_INPUT_CAL_NUM;
    unitIn[0].data = &calNumIn;
    unitIn[0].size = sizeof(FlashCalInput_Int);

    unitIn[1].type = FLASH_CAL_INPUT_P_YTAB;
    unitIn[1].data = &yTabIn;
    unitIn[1].size = sizeof(FlashCalInput_ShortPtr);

    unitIn[2].type = FLASH_CAL_INPUT_TOTAL_STEP;
    unitIn[2].data = &totalStepIn;
    unitIn[2].size = sizeof(FlashCalInput_Int);

    unitIn[3].type = FLASH_CAL_INPUT_TOTAL_STEP_LT;
    unitIn[3].data = &totalStepLTIn;
    unitIn[3].size = sizeof(FlashCalInput_Int);

    unitIn[4].type = FLASH_CAL_INPUT_DUAL_FLASH_ENABLE;
    unitIn[4].data = &dualFlashEnableIn;
    unitIn[4].size = sizeof(FlashCalInput_Int);

    unitOut.type = FLASH_CAL_OUTPUT_P_CHOOSE;
    unitOut.data = &chooseOut;
    unitOut.size = sizeof(FlashCalOutput_ChooseResult);

    groupIn.number = 5;
    groupIn.units = unitIn;

    groupOut.number = 1;
    groupOut.units = &unitOut;

    logD("[%s out] %d %p %d %d %p %d", __func__, CalNum, yTabIn.input,
            totalStep, totalStepLT, pChoose, dualFlashEnable);
    return flashCalAlgoCtrl(mSensorAlgo, MTK_FLASH_CAL_DECIDE_CAL_FLASH_COMB,
            &groupIn, &groupOut);
}

MRESULT FlashAlgoAdapter::InterpolateCalData(int CalNum, short *dutyI, short *dutyLtI,
        CalData *pCalData, NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT *pNvCali,
        int totalStep, int totalStepLT, AWB_GAIN_T *outWB, short *outYTab,
        int dualFlashEnable, int caliNvIdx)
{
    FlashCalGroup groupIn;
    FlashCalGroup groupOut;

    FlashCalUnit unitIn[9];
    FlashCalUnit unitOut[2];

    FlashCalInput_Int calNumIn, totalStepIn, totalStepLTIn;
    FlashCalInput_ShortPtr dutyIIn, dutyLtIIn, yTabIn;
    FlashCalInput_CalData calDataIn;
    FlashCalInput_FLASH_AWB_ALGO_GAIN_N_T goldenWBGainIn;
    FlashCalInput_Int dualFlashEnableIn;
    FlashCalOutput_AWB_GAIN_T outWBOut;
    FlashCalOutput_ShortPtr outYTabOut;

    FLASH_AWB_ALGO_GAIN_N_T *pFlashWBGain;
    logD("%s+", __func__);

    pFlashWBGain = FlashCustomAdapter::getInstance(mSensorDev)->getFlashWBGain();

    calNumIn.input = CalNum;
    dutyIIn.input = dutyI;
    dutyLtIIn.input = dutyLtI;
    calDataIn.input = pCalData;
    yTabIn.input = pNvCali->Flash_Calibration[caliNvIdx].engTab.yTab;
    goldenWBGainIn.input = pFlashWBGain;
    totalStepIn.input = totalStep;
    totalStepLTIn.input = totalStepLT;
    dualFlashEnableIn.input = dualFlashEnable;

    outWBOut.output = outWB;
    outYTabOut.output = outYTab;

    unitIn[0].type = FLASH_CAL_INPUT_CAL_NUM;
    unitIn[0].data = &calNumIn;
    unitIn[0].size = sizeof(FlashCalInput_Int);

    unitIn[1].type = FLASH_CAL_INPUT_P_DUTY_I;
    unitIn[1].data = &dutyIIn;
    unitIn[1].size = sizeof(FlashCalInput_ShortPtr);

    unitIn[2].type = FLASH_CAL_INPUT_P_DUTY_LTI;
    unitIn[2].data = &dutyLtIIn;
    unitIn[2].size = sizeof(FlashCalInput_ShortPtr);

    unitIn[3].type = FLASH_CAL_INPUT_P_CAL_DATA;
    unitIn[3].data = &calDataIn;
    unitIn[3].size = sizeof(FlashCalInput_CalData);

    unitIn[4].type = FLASH_CAL_INPUT_P_YTAB;
    unitIn[4].data = &yTabIn;
    unitIn[4].size = sizeof(FlashCalInput_ShortPtr);

    unitIn[5].type = FLASH_CAL_INPUT_P_GOLDEN_WB_GAIN;
    unitIn[5].data = &goldenWBGainIn;
    unitIn[5].size = sizeof(FlashCalInput_FLASH_AWB_ALGO_GAIN_N_T);

    unitIn[6].type = FLASH_CAL_INPUT_TOTAL_STEP;
    unitIn[6].data = &totalStepIn;
    unitIn[6].size = sizeof(FlashCalInput_Int);

    unitIn[7].type = FLASH_CAL_INPUT_TOTAL_STEP_LT;
    unitIn[7].data = &totalStepLTIn;
    unitIn[7].size = sizeof(FlashCalInput_Int);

    unitIn[8].type = FLASH_CAL_INPUT_DUAL_FLASH_ENABLE;
    unitIn[8].data = &dualFlashEnableIn;
    unitIn[8].size = sizeof(FlashCalInput_Int);

    unitOut[0].type = FLASH_CAL_OUTPUT_P_OUT_WB;
    unitOut[0].data = &outWBOut;
    unitOut[0].size = sizeof(FlashCalOutput_AWB_GAIN_T);

    unitOut[1].type = FLASH_CAL_OUTPUT_P_OUT_YTAB;
    unitOut[1].data = &outYTabOut;
    unitOut[1].size = sizeof(FlashCalOutput_ShortPtr);

    groupIn.number = 9;
    groupIn.units = unitIn;

    groupOut.number = 2;
    groupOut.units = unitOut;

    logD("[%s out] %d %p %p %p %p %p %d %d %p %p %d",
            __func__, CalNum, dutyI, dutyLtI, pCalData, yTabIn.input,
            pFlashWBGain, totalStep, totalStepLT, outWB, outYTab, dualFlashEnable);
    return flashCalAlgoCtrl(mSensorAlgo, MTK_FLASH_CAL_INTERPOLATE_CAL_DATA,
            &groupIn, &groupOut);
}

int FlashAlgoAdapter::setIsSaveSimBinFile(int isSaveBin)
{
    FlashAEGroup groupIn;
    FlashAEGroup groupOut;

    FlashAEUnit unitIn;

    FlashAEInput_Int isSaveBinIn;

    logD("%s+", __func__);

    isSaveBinIn.input = isSaveBin;

    unitIn.type = FLASH_AE_INPUT_IS_SAVE_BIN;
    unitIn.data = &isSaveBinIn;
    unitIn.size = sizeof(FlashAEInput_Int);

    groupIn.number = 1;
    groupIn.units = &unitIn;

    flashAEAlgoCtrl(MTK_FLASH_AE_SET_IS_SAVE_BIN_FILE, &groupIn, &groupOut, mSensorAlgo);
    logD("[%s out] %d", __func__, isSaveBin);
    return 0;
}

int FlashAlgoAdapter::setDebugDir(const char *DirName, const char *PrjName)
{
    FlashAEGroup groupIn;
    FlashAEGroup groupOut;

    FlashAEUnit unitIn[2];

    FlashAEInput_ConstCharPtr dirNameIn, prjNameIn;

    logD("%s+", __func__);

    dirNameIn.input = DirName;
    prjNameIn.input = PrjName;

    unitIn[0].type = FLASH_AE_INPUT_P_DIR_NAME;
    unitIn[0].data = &dirNameIn;
    unitIn[0].size = sizeof(FlashAEInput_ConstCharPtr);

    unitIn[1].type = FLASH_AE_INPUT_P_PRJ_NAME;
    unitIn[1].data = &prjNameIn;
    unitIn[1].size = sizeof(FlashAEInput_ConstCharPtr);

    groupIn.number = 2;
    groupIn.units = unitIn;

    flashAEAlgoCtrl(MTK_FLASH_AE_SET_DEBUG_DIR, &groupIn, &groupOut, mSensorAlgo);
    logD("[%s out] %p %p", __func__, DirName, PrjName);
    return 0;
}

int FlashAlgoAdapter::getDbgData(FLASH_DEBUG_INFO_T *pDebugInfo)
{
    FlashAEGroup groupIn;
    FlashAEGroup groupOut;

    FlashAEUnit unitOut;

    FlashAEOutput_FLASH_ALGO_DEBUG_INFO_T debugInfoOut;

    logD("%s+", __func__);
    FLASH_ALGO_DEBUG_INFO_T debugInfo;

    debugInfoOut.output = &debugInfo;

    unitOut.type = FLASH_AE_OUTPUT_P_DEBUG_INFO;
    unitOut.data = &debugInfoOut;
    unitOut.size = sizeof(FlashAEOutput_FLASH_ALGO_DEBUG_INFO_T);

    groupOut.number = 1;
    groupOut.units = &unitOut;
    flashAEAlgoCtrl(MTK_FLASH_AE_GET_DBG_DATA, &groupIn, &groupOut, mSensorAlgo);
    for(int i=0; i<FLASH_TAG_NUM-FLASH_DEBUG_TAG_ALG_START ; i++) {
        FlashAEUnit* unitIn1 = groupOut.units;
        FLASH_ALGO_DEBUG_INFO_T *pDebugInfoOut = ((FlashAEOutput_FLASH_ALGO_DEBUG_INFO_T*) unitIn1->data)->output;
        setDebugTag(*pDebugInfo, i + FLASH_DEBUG_TAG_ALG_START, pDebugInfoOut->Tag[i].u4FieldValue);
    }
    logD("[%s out] %p", __func__, &debugInfo);
    return 0;
}

int FlashAlgoAdapter::checkInputParaError(int* num, int* errBuf)
{
    FlashAEGroup groupIn;
    FlashAEGroup groupOut;

    FlashAEUnit unitIn[2];

    FlashAEInput_IntPtr numIn, errBufIn;
    logD("%s+", __func__);

    numIn.input = num;
    errBufIn.input = errBuf;

    unitIn[0].type = FLASH_AE_INPUT_P_NUM;
    unitIn[0].data = &numIn;
    unitIn[0].size = sizeof(FlashAEInput_IntPtr);

    unitIn[1].type = FLASH_AE_INPUT_P_ERR_BUF;
    unitIn[1].data = &errBufIn;
    unitIn[1].size = sizeof(FlashAEInput_IntPtr);

    groupIn.number = 2;
    groupIn.units = unitIn;

    logD("[%s out] %p %p", __func__, num, errBuf);
    return flashAEAlgoCtrl(MTK_FLASH_AE_CHECK_INPUT_PARA_ERROR, &groupIn, &groupOut, mSensorAlgo);
}
