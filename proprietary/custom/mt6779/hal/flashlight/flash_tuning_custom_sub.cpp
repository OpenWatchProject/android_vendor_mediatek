#define LOG_TAG "flash_tuning_custom_sub.cpp"
#define MTK_LOG_ENABLE 1

#include <cutils/log.h>
#include "flash_param.h"
#include "camera_custom_nvram.h"
#include "custom/aaa/AEPlinetable.h"
#include "flash_tuning_custom.h"


/***********************************************************
 * Static Method
 **********************************************************/
#if FLASH_NVRAM
#else
static void copyTuningPara_sub(FLASH_TUNING_PARA* p, NVRAM_FLASH_TUNING_PARA* nv_p)
{
    p->yTarget = nv_p->yTarget;
    p->fgWIncreaseLevelbySize = nv_p->fgWIncreaseLevelbySize;
    p->fgWIncreaseLevelbyRef = nv_p->fgWIncreaseLevelbyRef;
    p->ambientRefAccuracyRatio = nv_p->ambientRefAccuracyRatio;
    p->flashRefAccuracyRatio = nv_p->flashRefAccuracyRatio;
    p->backlightAccuracyRatio = nv_p->backlightAccuracyRatio;
    p->backlightUnderY = nv_p->backlightUnderY;
    p->backlightWeakRefRatio = nv_p->backlightWeakRefRatio;
    p->safetyExp = nv_p->safetyExp;
    p->maxUsableISO = nv_p->maxUsableISO;
    p->yTargetWeight = nv_p->yTargetWeight;
    p->lowReflectanceThreshold = nv_p->lowReflectanceThreshold;
    p->flashReflectanceWeight = nv_p->flashReflectanceWeight;
    p->bgSuppressMaxDecreaseEV = nv_p->bgSuppressMaxDecreaseEV;
    p->bgSuppressMaxOverExpRatio = nv_p->bgSuppressMaxOverExpRatio;
    p->fgEnhanceMaxIncreaseEV = nv_p->fgEnhanceMaxIncreaseEV;
    p->fgEnhanceMaxOverExpRatio = nv_p->fgEnhanceMaxOverExpRatio;
    p->isFollowCapPline = nv_p->isFollowCapPline;
    p->histStretchMaxFgYTarget = nv_p->histStretchMaxFgYTarget;
    p->histStretchBrightestYTarget = nv_p->histStretchBrightestYTarget;
    p->fgSizeShiftRatio = nv_p->fgSizeShiftRatio;
    p->backlitPreflashTriggerLV = nv_p->backlitPreflashTriggerLV;
    p->backlitMinYTarget = nv_p->backlitMinYTarget;
    p->minstameanpass = nv_p->minstameanpass;
	p->faceMixedWeight = nv_p->faceMixedWeight;
    p->enablePreflashAE = 0;
    ALOGD("%s(): yTarget(%d).", __FUNCTION__, p->yTarget);
}

static void copyTuningParaDualFlash_sub(FLASH_TUNING_PARA* p, NVRAM_CAMERA_STROBE_STRUCT* nv)
{
    p->dualFlashPref.toleranceEV_pos = nv->dualTuningPara.toleranceEV_pos;
    p->dualFlashPref.toleranceEV_neg = nv->dualTuningPara.toleranceEV_neg;
    p->dualFlashPref.XYWeighting = nv->dualTuningPara.XYWeighting;
    p->dualFlashPref.useAwbPreferenceGain = nv->dualTuningPara.useAwbPreferenceGain;
    for (int i = 0; i < 4; i++) {
        p->dualFlashPref.envOffsetIndex[i] = nv->dualTuningPara.envOffsetIndex[i];
        p->dualFlashPref.envXrOffsetValue[i] = nv->dualTuningPara.envXrOffsetValue[i];
        p->dualFlashPref.envYrOffsetValue[i] = nv->dualTuningPara.envYrOffsetValue[i];
    }
    p->dualFlashPref.VarianceTolerance = nv->dualTuningPara.VarianceTolerance;
    p->dualFlashPref.ChooseColdOrWarm = nv->dualTuningPara.ChooseColdOrWarm;
}
#endif

static int currentMaskFunc_sub(int c, int cLt)
{
    // TODO: setup mask current
    if ((c + cLt) > 1100)
        return 0;
    return 1;
}

/***********************************************************
 * Public Method
 **********************************************************/
void cust_getFlashQuick2CalibrationExp_sub(int* exp, int* afe, int* isp)
{
    // TODO: setup normal calibration exposure condition
    *exp = 10000;
    *afe = 1024;
    *isp = 1024;
}

void cust_getFlashHalTorchDuty_sub(int* duty, int* dutyLt)
{
    // TODO: setup flashlight torch duty
    *duty = 0;
    *dutyLt = 0;
}

FLASH_PROJECT_PARA& cust_getFlashProjectPara_sub(int aeScene, int isForceFlash, NVRAM_CAMERA_STROBE_STRUCT* nvrame)
{
    static FLASH_PROJECT_PARA para;

    /* from NVRAM */
    if (nvrame) {
#if FLASH_NVRAM
    (void)aeScene;
    (void)isForceFlash;
#else
        int ind = 0;
        int aeSceneInd = -1;
        switch (aeScene) {
        case LIB3A_AE_SCENE_OFF:
            aeSceneInd = 1;
            break;
        case LIB3A_AE_SCENE_AUTO:
            aeSceneInd = 2;
            break;
        case LIB3A_AE_SCENE_NIGHT:
            aeSceneInd = 3;
            break;
        case LIB3A_AE_SCENE_ACTION:
            aeSceneInd = 4;
            break;
        case LIB3A_AE_SCENE_BEACH:
            aeSceneInd = 5;
            break;
        case LIB3A_AE_SCENE_CANDLELIGHT:
            aeSceneInd = 6;
            break;
        case LIB3A_AE_SCENE_FIREWORKS:
            aeSceneInd = 7;
            break;
        case LIB3A_AE_SCENE_LANDSCAPE:
            aeSceneInd = 8;
            break;
        case LIB3A_AE_SCENE_PORTRAIT:
            aeSceneInd = 9;
            break;
        case LIB3A_AE_SCENE_NIGHT_PORTRAIT:
            aeSceneInd = 10;
            break;
        case LIB3A_AE_SCENE_PARTY:
            aeSceneInd = 11;
            break;
        case LIB3A_AE_SCENE_SNOW:
            aeSceneInd = 12;
            break;
        case LIB3A_AE_SCENE_SPORTS:
            aeSceneInd = 13;
            break;
        case LIB3A_AE_SCENE_STEADYPHOTO:
            aeSceneInd = 14;
            break;
        case LIB3A_AE_SCENE_SUNSET:
            aeSceneInd = 15;
            break;
        case LIB3A_AE_SCENE_THEATRE:
            aeSceneInd = 16;
            break;
        case LIB3A_AE_SCENE_ISO_ANTI_SHAKE:
            aeSceneInd = 17;
            break;
        case LIB3A_AE_SCENE_BACKLIGHT:
            aeSceneInd = 18;
            break;
        default:
            aeSceneInd = 0;
            break;
        }

        if (isForceFlash == 1)
            ind = nvrame->paraIdxForceOn[aeSceneInd];
        else
            ind = nvrame->paraIdxAuto[aeSceneInd];

        ALOGD("%s(): paraIdx(%d), aeSceneInd(%d).", __FUNCTION__, ind, aeSceneInd);

        copyTuningPara_sub(&para.tuningPara, &nvrame->tuningPara[ind]);
        copyTuningParaDualFlash_sub(&para.tuningPara, nvrame);
#endif
    }

    // TODO: setup duty number
    para.dutyNum = 30;
    para.dutyNumLT = 30;

    // TODO: setup cooling time muliple and timeout
    para.coolTimeOutPara.tabNum = 5;
    para.coolTimeOutPara.tabId[0] = 0;
    para.coolTimeOutPara.tabId[1] = 6;
    para.coolTimeOutPara.tabId[2] = 16;
    para.coolTimeOutPara.tabId[3] = 21;
    para.coolTimeOutPara.tabId[4] = 29;
    para.coolTimeOutPara.coolingTM[0] = 0;
    para.coolTimeOutPara.coolingTM[1] = 0;
    para.coolTimeOutPara.coolingTM[2] = 2;
    para.coolTimeOutPara.coolingTM[3] = 4;
    para.coolTimeOutPara.coolingTM[4] = 9;
    para.coolTimeOutPara.timOutMs[0] = ENUM_FLASH_TIME_NO_TIME_OUT;
    para.coolTimeOutPara.timOutMs[1] = ENUM_FLASH_TIME_NO_TIME_OUT;
    para.coolTimeOutPara.timOutMs[2] = 600;
    para.coolTimeOutPara.timOutMs[3] = 500;
    para.coolTimeOutPara.timOutMs[4] = 300;

    para.coolTimeOutParaLT.tabNum = 5;
    para.coolTimeOutParaLT.tabId[0] = 0;
    para.coolTimeOutParaLT.tabId[1] = 6;
    para.coolTimeOutParaLT.tabId[2] = 16;
    para.coolTimeOutParaLT.tabId[3] = 21;
    para.coolTimeOutParaLT.tabId[4] = 29;
    para.coolTimeOutParaLT.coolingTM[0] = 0;
    para.coolTimeOutParaLT.coolingTM[1] = 0;
    para.coolTimeOutParaLT.coolingTM[2] = 2;
    para.coolTimeOutParaLT.coolingTM[3] = 4;
    para.coolTimeOutParaLT.coolingTM[4] = 9;
    para.coolTimeOutParaLT.timOutMs[0] = ENUM_FLASH_TIME_NO_TIME_OUT;
    para.coolTimeOutParaLT.timOutMs[1] = ENUM_FLASH_TIME_NO_TIME_OUT;
    para.coolTimeOutParaLT.timOutMs[2] = 600;
    para.coolTimeOutParaLT.timOutMs[3] = 500;
    para.coolTimeOutParaLT.timOutMs[4] = 300;

    // TODO: setup max capture exposure time
    para.maxCapExpTimeUs = 66000;

    para.currentAvailableMaskFunc = currentMaskFunc_sub;

    return para;
}

