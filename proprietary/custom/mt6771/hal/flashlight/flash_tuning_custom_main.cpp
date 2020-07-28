#define LOG_TAG "flash_tuning_custom_main.cpp"
#define MTK_LOG_ENABLE 1

#include <cutils/log.h>
#include "flash_param.h"
#include "camera_custom_nvram.h"
#include "custom/aaa/AEPlinetable.h"
#include "flash_tuning_custom.h"

#if FLASH_NVRAM
#else

/***********************************************************
 * Static Method
 **********************************************************/
static void copyTuningPara_main(FLASH_TUNING_PARA* p, NVRAM_FLASH_TUNING_PARA* nv_p)
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
    p->enablePreflashAE = 0;
    ALOGD("%s(): yTarget(%d).", __FUNCTION__, p->yTarget);
}

static void copyTuningParaDualFlash_main(FLASH_TUNING_PARA* p, NVRAM_CAMERA_STROBE_STRUCT* nv)
{
    (void)p;
    (void)nv;


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

static int FlashIMapFunc_main(int duty, int dutyLt)
{
    // TODO: setup duty current
    int dutyI[] = {
        22, 46, 70, 92, 116, 139, 163, 200, 250, 300, 350, 400, 450, 500, 550, 600, 650, 700, 750, 800, 850, 900, 950, 1000, 1050, 1100
    };
    // TODO: setup duty current
    int dutyLtI[] = {
        22, 46, 70, 92, 116, 139, 163, 200, 250, 300, 350, 400, 450, 500, 550, 600, 650, 700, 750, 800, 850, 900, 950, 1000, 1050, 1100
    };
    int mA = 0;
    int mALt = 0;
    if (duty >= 0 && duty < (int)(sizeof(dutyI) / sizeof(int)))
        mA = dutyI[duty];
    if (dutyLt >= 0 && dutyLt < (int)(sizeof(dutyLtI) / sizeof(int)))
        mALt = dutyLtI[dutyLt];
    return mA + mALt;
}

static int dutyMaskFunc_main(int d, int dLt)
{
    // TODO: setup mask current
    if (FlashIMapFunc_main(d, dLt) > 1100)
        return 0;
    return 1;
}


/***********************************************************
 * Public Method
 **********************************************************/
void cust_getFlashQuick2CalibrationExp_main(int* exp, int* afe, int* isp)
{
    // TODO: setup normal calibration exposure condition
    *exp = 10000;
    *afe = 1024;
    *isp = 1024;
}

void cust_getFlashHalTorchDuty_main(int* duty, int* dutyLt)
{
    // TODO: setup flashlight torch duty
    *duty = 6;
    *dutyLt = 0;
}

FlashIMapFP cust_getFlashIMapFunc_main()
{
    return FlashIMapFunc_main;
}

void cust_getFlashITab1_main(short* ITab1)
{
    (void)ITab1;
    // TODO: setup duty current
    int dutyI[] = {
        22, 46, 70, 92, 116, 139, 163, 200, 250, 300, 350, 400, 450, 500, 550, 600, 650, 700, 750, 800, 850, 900, 950, 1000, 1050, 1100
    };
    int sz = sizeof(dutyI) / sizeof(int);
    for (int i = 0; i < sz; i++)
        ITab1[i] = dutyI[i];
}

void cust_getFlashITab2_main(short* ITab2)
{
    (void)ITab2;
    // TODO: setup duty current
    int dutyLtI[] = {
        22, 46, 70, 92, 116, 139, 163, 200, 250, 300, 350, 400, 450, 500, 550, 600, 650, 700, 750, 800, 850, 900, 950, 1000, 1050, 1100
    };
    int sz = sizeof(dutyLtI) / sizeof(int);
    for (int i = 0; i < sz; i++)
        ITab2[i] = dutyLtI[i];
}

void cust_getFlashMaxIDuty_main(int dutyNum, int dutyNumLt, int* duty, int* dutyLt)
{
    int max_duty = 0;
    int max_dutyLt = 0;
    int max_I = 0;
    //short ITab1[dutyNum];  /* fix build warning */
    //short ITab2[dutyNumLt];  /* fix build warning */

    max_I = FlashIMapFunc_main(max_duty, max_dutyLt);

    for (int i = -1; i < dutyNum; i++) {
        for (int j = -1; j < dutyNumLt; j++) {
            if (dutyMaskFunc_main(i,j) == 1)
                if (FlashIMapFunc_main(i,j) >= max_I) {
                    max_I = FlashIMapFunc_main(i,j);
                    max_duty = i;
                    max_dutyLt = j;
                }
        }
    }
    *duty = max_duty;
    *dutyLt = max_dutyLt;
}

FLASH_PROJECT_PARA& cust_getFlashProjectPara_main(int aeScene, int isForceFlash, NVRAM_CAMERA_STROBE_STRUCT* nvrame)
{
    (void)aeScene;
    (void)isForceFlash;
    static FLASH_PROJECT_PARA para;

    /* from NVRAM */
    if (nvrame) {
#if FLASH_NVRAM
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

        copyTuningPara_main(&para.tuningPara, &nvrame->tuningPara[ind]);
        copyTuningParaDualFlash_main(&para.tuningPara, nvrame);
#endif
    }

    // TODO: setup duty number
    para.dutyNum = 26;
    para.dutyNumLT = 26;

    // TODO: setup cooling time muliple and timeout
    para.coolTimeOutPara.tabNum = 5;
    para.coolTimeOutPara.tabId[0] = 0;
    para.coolTimeOutPara.tabId[1] = 6;
    para.coolTimeOutPara.tabId[2] = 12;
    para.coolTimeOutPara.tabId[3] = 17;
    para.coolTimeOutPara.tabId[4] = 25;
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
    para.coolTimeOutParaLT.tabId[2] = 12;
    para.coolTimeOutParaLT.tabId[3] = 17;
    para.coolTimeOutParaLT.tabId[4] = 25;
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

    para.dutyAvailableMaskFunc = dutyMaskFunc_main;

    return para;
}

