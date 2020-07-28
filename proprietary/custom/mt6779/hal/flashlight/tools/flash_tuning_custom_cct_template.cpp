#define LOG_TAG "flash_tuning_custom_cct_template.cpp"
#define MTK_LOG_ENABLE 1

#include <string.h>
#include "camera_custom_nvram.h"


/***********************************************************
 * Energy and FlashWB Gain
 **********************************************************/
// TODO: setup energy table
static short engTab[] = {
    FIXME_engTab
};

// TODO: setup flash WB gain table
static AWB_GAIN_T flashWBGain[] = {
    FIXME_flashWBGain
};


/***********************************************************
 * Public Method
 **********************************************************/
int cust_fillDefaultStrobeNVRam_template(void* data)
{
    NVRAM_CAMERA_STROBE_STRUCT* p = (NVRAM_CAMERA_STROBE_STRUCT*)data;
    memcpy(p->engTab.yTab, engTab, sizeof(engTab));

    // TODO: setup NVRAM strobe file version 
    p->u4Version = FIXME_nvramStrobeVersion;

    // TODO: setup tuning parameters
    for (int i = 0; i < 8; i++) {
        p->tuningPara[i].yTarget = FIXME_yTarget_x;
        p->tuningPara[i].fgWIncreaseLevelbySize = FIXME_fgWIncreaseLevelbySize_x;
        p->tuningPara[i].fgWIncreaseLevelbyRef = FIXME_fgWIncreaseLevelbyRef_x;
        p->tuningPara[i].ambientRefAccuracyRatio = FIXME_ambientRefAccuracyRatio_x;
        p->tuningPara[i].flashRefAccuracyRatio = FIXME_flashRefAccuracyRatio_x;
        p->tuningPara[i].backlightAccuracyRatio = FIXME_backlightAccuracyRatio_x;
        p->tuningPara[i].backlightUnderY = FIXME_backlightUnderY_x;
        p->tuningPara[i].backlightWeakRefRatio = FIXME_backlightWeakRefRatio_x;
        p->tuningPara[i].safetyExp = FIXME_safetyExp_x;
        p->tuningPara[i].maxUsableISO = FIXME_maxUsableISO_x;
        p->tuningPara[i].yTargetWeight = FIXME_yTargetWeight_x;
        p->tuningPara[i].lowReflectanceThreshold = FIXME_lowReflectanceThreshold_x;
        p->tuningPara[i].flashReflectanceWeight = FIXME_flashReflectanceWeight_x;
        p->tuningPara[i].bgSuppressMaxDecreaseEV = FIXME_bgSuppressMaxDecreaseEV_x;
        p->tuningPara[i].bgSuppressMaxOverExpRatio = FIXME_bgSuppressMaxOverExpRatio_x;
        p->tuningPara[i].fgEnhanceMaxIncreaseEV = FIXME_fgEnhanceMaxIncreaseEV_x;
        p->tuningPara[i].fgEnhanceMaxOverExpRatio = FIXME_fgEnhanceMaxOverExpRatio_x;
        p->tuningPara[i].isFollowCapPline = FIXME_isFollowCapPline_x;
        p->tuningPara[i].histStretchMaxFgYTarget = FIXME_histStretchMaxFgYTarget_x;
        p->tuningPara[i].histStretchBrightestYTarget = FIXME_histStretchBrightestYTarget_x;
        p->tuningPara[i].fgSizeShiftRatio = FIXME_fgSizeShiftRatio_x;
        p->tuningPara[i].backlitPreflashTriggerLV = FIXME_backlitPreflashTriggerLV_x;
        p->tuningPara[i].backlitMinYTarget = FIXME_backlitMinYTarget_x;
        p->tuningPara[i].minstameanpass = FIXME_minstameanpass_x;
    }
    p->tuningPara[0].isFollowCapPline = FIXME_isFollowCapPline_0;

    // TODO: setup tuning scene index for force on
    p->paraIdxForceOn[0]  = FIXME_paraIdxForceOn_00; //default
    p->paraIdxForceOn[1]  = FIXME_paraIdxForceOn_01; //LIB3A_AE_SCENE_OFF
    p->paraIdxForceOn[2]  = FIXME_paraIdxForceOn_02; //LIB3A_AE_SCENE_AUTO
    p->paraIdxForceOn[3]  = FIXME_paraIdxForceOn_03; //LIB3A_AE_SCENE_NIGHT
    p->paraIdxForceOn[4]  = FIXME_paraIdxForceOn_04; //LIB3A_AE_SCENE_ACTION
    p->paraIdxForceOn[5]  = FIXME_paraIdxForceOn_05; //LIB3A_AE_SCENE_BEACH
    p->paraIdxForceOn[6]  = FIXME_paraIdxForceOn_06; //LIB3A_AE_SCENE_CANDLELIGHT
    p->paraIdxForceOn[7]  = FIXME_paraIdxForceOn_07; //LIB3A_AE_SCENE_FIREWORKS
    p->paraIdxForceOn[8]  = FIXME_paraIdxForceOn_08; //LIB3A_AE_SCENE_LANDSCAPE
    p->paraIdxForceOn[9]  = FIXME_paraIdxForceOn_09; //LIB3A_AE_SCENE_PORTRAIT
    p->paraIdxForceOn[10] = FIXME_paraIdxForceOn_10; //LIB3A_AE_SCENE_NIGHT_PORTRAIT
    p->paraIdxForceOn[11] = FIXME_paraIdxForceOn_11; //LIB3A_AE_SCENE_PARTY
    p->paraIdxForceOn[12] = FIXME_paraIdxForceOn_12; //LIB3A_AE_SCENE_SNOW
    p->paraIdxForceOn[13] = FIXME_paraIdxForceOn_13; //LIB3A_AE_SCENE_SPORTS
    p->paraIdxForceOn[14] = FIXME_paraIdxForceOn_14; //LIB3A_AE_SCENE_STEADYPHOTO
    p->paraIdxForceOn[15] = FIXME_paraIdxForceOn_15; //LIB3A_AE_SCENE_SUNSET
    p->paraIdxForceOn[16] = FIXME_paraIdxForceOn_16; //LIB3A_AE_SCENE_THEATRE
    p->paraIdxForceOn[17] = FIXME_paraIdxForceOn_17; //LIB3A_AE_SCENE_ISO_ANTI_SHAKE
    p->paraIdxForceOn[18] = FIXME_paraIdxForceOn_18; //LIB3A_AE_SCENE_BACKLIGHT

    // TODO: setup tuning scene index for auto
    p->paraIdxAuto[0]  = FIXME_paraIdxAuto_00; //default
    p->paraIdxAuto[1]  = FIXME_paraIdxAuto_01; //LIB3A_AE_SCENE_OFF
    p->paraIdxAuto[2]  = FIXME_paraIdxAuto_02; //LIB3A_AE_SCENE_AUTO
    p->paraIdxAuto[3]  = FIXME_paraIdxAuto_03; //LIB3A_AE_SCENE_NIGHT
    p->paraIdxAuto[4]  = FIXME_paraIdxAuto_04; //LIB3A_AE_SCENE_ACTION
    p->paraIdxAuto[5]  = FIXME_paraIdxAuto_05; //LIB3A_AE_SCENE_BEACH
    p->paraIdxAuto[6]  = FIXME_paraIdxAuto_06; //LIB3A_AE_SCENE_CANDLELIGHT
    p->paraIdxAuto[7]  = FIXME_paraIdxAuto_07; //LIB3A_AE_SCENE_FIREWORKS
    p->paraIdxAuto[8]  = FIXME_paraIdxAuto_08; //LIB3A_AE_SCENE_LANDSCAPE
    p->paraIdxAuto[9]  = FIXME_paraIdxAuto_09; //LIB3A_AE_SCENE_PORTRAIT
    p->paraIdxAuto[10] = FIXME_paraIdxAuto_10; //LIB3A_AE_SCENE_NIGHT_PORTRAIT
    p->paraIdxAuto[11] = FIXME_paraIdxAuto_11; //LIB3A_AE_SCENE_PARTY
    p->paraIdxAuto[12] = FIXME_paraIdxAuto_12; //LIB3A_AE_SCENE_SNOW
    p->paraIdxAuto[13] = FIXME_paraIdxAuto_13; //LIB3A_AE_SCENE_SPORTS
    p->paraIdxAuto[14] = FIXME_paraIdxAuto_14; //LIB3A_AE_SCENE_STEADYPHOTO
    p->paraIdxAuto[15] = FIXME_paraIdxAuto_15; //LIB3A_AE_SCENE_SUNSET
    p->paraIdxAuto[16] = FIXME_paraIdxAuto_16; //LIB3A_AE_SCENE_THEATRE
    p->paraIdxAuto[17] = FIXME_paraIdxAuto_17; //LIB3A_AE_SCENE_ISO_ANTI_SHAKE
    p->paraIdxAuto[18] = FIXME_paraIdxAuto_18; //LIB3A_AE_SCENE_BACKLIGHT

    // TODO: setup HT torch duty
    p->engLevel.torchDuty = FIXME_engLevelTorchDuty;
    // TODO: setup HT AF lamp duty
    p->engLevel.afDuty = FIXME_engLevelAfDuty;
    // TODO: setup HT pre-flash duty
    p->engLevel.pfDuty = FIXME_engLevelPfDutyN;
    // TODO: setup HT main flash duty range
    p->engLevel.mfDutyMax = FIXME_engLevelMfDutyMaxN;
    p->engLevel.mfDutyMin = FIXME_engLevelMfDutyMinN;
    // TODO: setup HT low battery
    p->engLevel.IChangeByVBatEn = FIXME_engLeveliChangeByVBatEn;
    p->engLevel.vBatL = FIXME_engLevelVBatL; //mv
    p->engLevel.pfDutyL = FIXME_engLevelPfDutyL;
    p->engLevel.mfDutyMaxL = FIXME_engLevelMfDutyMaxL;
    p->engLevel.mfDutyMinL = FIXME_engLevelMfDutyMinL;
    // TODO: setup HT burst shot
    p->engLevel.IChangeByBurstEn = FIXME_engLeveliChangeByBurstEn;
    p->engLevel.pfDutyB = FIXME_engLevelPfDutyB;
    p->engLevel.mfDutyMaxB = FIXME_engLevelMfDutyMaxB;
    p->engLevel.mfDutyMinB = FIXME_engLevelMfDutyMinB;
    // TODO: setup HT high current
    p->engLevel.decSysIAtHighEn = FIXME_engLevelDecSysIAtHighEn;
    p->engLevel.dutyH = FIXME_engLevelDutyH;

    // TODO: setup LT torch duty
    p->engLevelLT.torchDuty = FIXME_engLevelLTTorchDuty;
    // TODO: setup LT AF lamp duty
    p->engLevelLT.afDuty = FIXME_engLevelLTAfDuty;
    // TODO: setup LT pre-flash duty
    p->engLevelLT.pfDuty = FIXME_engLevelLTPfDutyN;
    // TODO: setup LT main flash duty range
    p->engLevelLT.mfDutyMax = FIXME_engLevelLTMfDutyMaxN;
    p->engLevelLT.mfDutyMin = FIXME_engLevelLTMfDutyMinN;
    // TODO: setup HT low battery
    p->engLevelLT.pfDutyL = FIXME_engLevelLTPfDutyL;
    p->engLevelLT.mfDutyMaxL = FIXME_engLevelLTMfDutyMaxL;
    p->engLevelLT.mfDutyMinL = FIXME_engLevelLTMfDutyMinL;
    // TODO: setup HT burst shot
    p->engLevelLT.pfDutyB = FIXME_engLevelLTPfDutyB;
    p->engLevelLT.mfDutyMaxB = FIXME_engLevelLTMfDutyMaxB;
    p->engLevelLT.mfDutyMinB = FIXME_engLevelLTMfDutyMinB;

    // TODO: setup dual flash tuning parameters
    p->dualTuningPara.toleranceEV_pos = FIXME_toleranceEV_pos;
    p->dualTuningPara.toleranceEV_neg = FIXME_toleranceEV_neg;

    p->dualTuningPara.XYWeighting = FIXME_xyWeighting;
    p->dualTuningPara.useAwbPreferenceGain = FIXME_useAwbPreferenceGain;
    p->dualTuningPara.envOffsetIndex[0] = FIXME_envOffsetIndex_0;
    p->dualTuningPara.envOffsetIndex[1] = FIXME_envOffsetIndex_1;
    p->dualTuningPara.envOffsetIndex[2] = FIXME_envOffsetIndex_2;
    p->dualTuningPara.envOffsetIndex[3] = FIXME_envOffsetIndex_3;

    p->dualTuningPara.envXrOffsetValue[0] = FIXME_envXrOffsetValue_0;
    p->dualTuningPara.envXrOffsetValue[1] = FIXME_envXrOffsetValue_1;
    p->dualTuningPara.envXrOffsetValue[2] = FIXME_envXrOffsetValue_2;
    p->dualTuningPara.envXrOffsetValue[3] = FIXME_envXrOffsetValue_3;

    p->dualTuningPara.envYrOffsetValue[0] = FIXME_envYrOffsetValue_0;
    p->dualTuningPara.envYrOffsetValue[1] = FIXME_envYrOffsetValue_1;
    p->dualTuningPara.envYrOffsetValue[2] = FIXME_envYrOffsetValue_2;
    p->dualTuningPara.envYrOffsetValue[3] = FIXME_envYrOffsetValue_3;

    p->dualTuningPara.VarianceTolerance = FIXME_varianceTolerance;
    p->dualTuningPara.ChooseColdOrWarm = FIXME_chooseColdOrWarm;

    return 0;
}

int cust_fillDefaultFlashCalibrationNVRam_template(void* data)
{
    NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT* p = (NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT*)data;
    memcpy(p->yTab, engTab, sizeof(engTab));
    memcpy(p->flashWBGain, flashWBGain, sizeof(flashWBGain));

    return 0;
}

