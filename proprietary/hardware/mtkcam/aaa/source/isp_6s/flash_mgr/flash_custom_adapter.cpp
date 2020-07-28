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

#define LOG_TAG "FlashCustomAdapter"

#include <array>

/* aaa common headers */
#include "log_utils.h"
#include <private/aaa_utils.h>
#include <private/aaa_hal_private.h>

/* kernel headers */
#include "kd_camera_feature.h"

/* custom headers */
#include "flash_tuning_custom.h"

/* flash headers */
#include "flash_custom_adapter.h"
#include "flash_hal.h"
#include "flash_nvram.h"
#include "flash_duty.h"
#include "tools/flash_custom_utils.h"
#include "flash_utils.h"
#include "flash_cct.h"

#include <stddef.h>

#include <mtkcam/drv/mem/cam_cal_drv.h>

#define FLASH_TRANSFORM_CALI_DATA_PATH "/data/vendor/flash/transformFlashCaliData.txt"

/***********************************************************
 * Copy Project Parameters From NVRAM
 **********************************************************/
void copyTuningPara(FLASH_TUNING_PARA* p, NVRAM_FLASH_TUNING_PARA* nv_p)
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
    p->yDecreEvWeight = nv_p->yDecreEvWeight;
    p->yFaceTarget = nv_p->yFaceTarget;
    p->cfgFlashPolicy = nv_p->cfgFlashPolicy;
    p->faceMixedWeight = nv_p->faceMixedWeight;
    p->enablePreflashAE = 0;

    ALOGD("%s(): yTarget(%d), policy(%d).", __FUNCTION__, p->yTarget, p->cfgFlashPolicy);
}

void copyTuningParaDualFlash(FLASH_TUNING_PARA* p, NVRAM_DUAL_FLASH_TUNING_PARA* nv)
{
    p->dualFlashPref.toleranceEV_pos = nv->toleranceEV_pos;
    p->dualFlashPref.toleranceEV_neg = nv->toleranceEV_neg;
    p->dualFlashPref.XYWeighting = nv->XYWeighting;
    p->dualFlashPref.useAwbPreferenceGain = nv->useAwbPreferenceGain;
    for (int i = 0; i < 4; i++) {
        p->dualFlashPref.envOffsetIndex[i] = nv->envOffsetIndex[i];
        p->dualFlashPref.envXrOffsetValue[i] = nv->envXrOffsetValue[i];
        p->dualFlashPref.envYrOffsetValue[i] = nv->envYrOffsetValue[i];
    }
    p->dualFlashPref.VarianceTolerance = nv->VarianceTolerance;
    if (nv->ChooseColdOrWarm == FLASH_CHOOSE_WARM)
        p->dualFlashPref.ChooseColdOrWarm = FLASH_ALGO_CHOOSE_WARM;
    else
        p->dualFlashPref.ChooseColdOrWarm = FLASH_ALGO_CHOOSE_COLD;
}

void copyTuningParaFlashAwb(FLASH_AWB_ALGO_TUNING_PARAM_T* p,
        FLASH_AWB_TUNING_PARAM_T* nv)
{
    p->ForeGroundPercentage = nv->ForeGroundPercentage;
    p->BackGroundPercentage = nv->BackGroundPercentage;

    p->FgPercentage_Th1 = nv->FgPercentage_Th1;
    p->FgPercentage_Th2 = nv->FgPercentage_Th2;
    p->FgPercentage_Th3 = nv->FgPercentage_Th3;
    p->FgPercentage_Th4 = nv->FgPercentage_Th4;
    p->FgPercentage_Th1_Val = nv->FgPercentage_Th1_Val;
    p->FgPercentage_Th2_Val = nv->FgPercentage_Th2_Val;
    p->FgPercentage_Th3_Val = nv->FgPercentage_Th3_Val;
    p->FgPercentage_Th4_Val = nv->FgPercentage_Th4_Val;

    p->location_map_th1 = nv->location_map_th1;
    p->location_map_th2 = nv->location_map_th2;
    p->location_map_th3 = nv->location_map_th3;
    p->location_map_th4 = nv->location_map_th4;
    p->location_map_val1 = nv->location_map_val1;
    p->location_map_val2 = nv->location_map_val2;
    p->location_map_val3 = nv->location_map_val3;
    p->location_map_val4 = nv->location_map_val4;

    p->SelfTuningFbBgWeightTbl = nv->SelfTuningFbBgWeightTbl;
    p->FgBgTbl_Y0 = nv->FgBgTbl_Y0;
    p->FgBgTbl_Y1 = nv->FgBgTbl_Y1;
    p->FgBgTbl_Y2 = nv->FgBgTbl_Y2;
    p->FgBgTbl_Y3 = nv->FgBgTbl_Y3;
    p->FgBgTbl_Y4 = nv->FgBgTbl_Y4;
    p->FgBgTbl_Y5 = nv->FgBgTbl_Y5;

    for(int i=0; i<5; i++)
        p->YPrimeWeightTh[i] = nv->YPrimeWeightTh[i];

    for(int i=0; i<4; i++)
        p->YPrimeWeight[i] = nv->YPrimeWeight[i];

    for(int i=0; i<FLASH_LV_INDEX_NUM; i++)
        p->FlashPreferenceGain[i] = nv->FlashPreferenceGain[i];
}

int dumpFlashAwb(const char *fname, FLASH_AWB_GAIN_N_T *gain, int len, int isDual, int dutyNum, int dutyNumLt)
{
    /* verify arguments */
    if (!fname || !gain)
        return -1;

    FILE *fp = fopen(fname, "wt");
    if (!fp)
        return -1;

    fprintf(fp, "{{\n");

    int i;
    for (i = 0; i < len; i++) {
        if (isDual) {
            int duty = 0;
            int dutyLt = 0;
            FlashDuty::ind2duty(i, duty, dutyLt, dutyNum, dutyNumLt, isDual);
            fprintf(fp, "    {%4d,%4d},  //duty=%d, dutyLt=%d\n", gain[i].i2R, gain[i].i2B, duty, dutyLt);
        } else
            fprintf(fp, "    {%4d,%4d},\n", gain[i].i2R, gain[i].i2B);
    }
    fprintf(fp, "}}\n");
    fclose(fp);

    return 0;
}

void dumpFwbGain(FLASH_AWB_GAIN_N_T *flashWBGain, int dutyNum, int dutyNumLt)
{
    if (!flashWBGain)
        return;

    logI("dumpFwbGain():");

    for (int j = 0; j < dutyNumLt; j++)
        for (int i = 0; i < dutyNum; i++)
            logI("\t(%d,%d): %d %d", i, j,
                    flashWBGain[i + j * dutyNumLt].i2R,
                    flashWBGain[i + j * dutyNumLt].i2B);
}


FlashCustomAdapter::FlashCustomAdapter(int sensorDev)
    : mSensorDev(sensorDev)
    , mFacingSensor(0)
    , mpStrobeNvbuf(NULL)
    , mpFlashCaliNvbuf(NULL)
    , mFlashAENVRAMIdx(0)
    , mFlashAWBNVRAMIdx(0)
    , mFlashCaliNVRAMIdx(0)
    , mPfPolicy(0)
    , mDutyNum(0)
    , mDutyNumLT(0)
    , mCurrentMaskFunc(NULL)
{
    setDebug();
    memset(&mFlashAwbTuning, 0, sizeof(mFlashAwbTuning));
    memset(&mFlashWBGain, 0, sizeof(mFlashWBGain));

    int facing = 0, num = 0, facingMain = 0;
    NS3Av3::mapSensorDevToFace(sensorDev, facing, num);
    NS3Av3::mapSensorDevToFace(DUAL_CAMERA_MAIN_SENSOR, facingMain, num);
    if (facing == facingMain)
        mFacingSensor = DUAL_CAMERA_MAIN_SENSOR;
    else
        mFacingSensor = DUAL_CAMERA_SUB_SENSOR;
}

FlashCustomAdapter::~FlashCustomAdapter()
{
}

int FlashCustomAdapter::init()
{
    /* get nvram strobe data */
    if (FlashNvram::nvReadStrobe(mpStrobeNvbuf, mSensorDev))
        logE("init(): failed to read nvram.");

    /* get nvram flash calibration data */
    if (FlashNvram::nvReadFlashCali(mpFlashCaliNvbuf, mSensorDev))
        logE("init(): failed to read nvram.");

    /* get project parameters */
    FLASH_PROJECT_PARA prjPara;
    prjPara = getFlashProjectPara(LIB3A_AE_SCENE_AUTO, 0);

    mCurrentMaskFunc = prjPara.currentAvailableMaskFunc;
    mDutyNum = prjPara.dutyNum;
    mDutyNumLT = prjPara.dutyNumLT;

    /* update pre-flash flow policy */
    mPfPolicy = prjPara.tuningPara.cfgFlashPolicy;

    /* verify energy table */
    if (verifyYTab(mpFlashCaliNvbuf->Flash_Calibration[mFlashCaliNVRAMIdx].engTab.yTab, mDutyNum, mDutyNumLT,
                cust_isDualFlashSupport(mFacingSensor)))
        logE("init(): invalid energy table.");

    FlashHal::getInstance(mSensorDev)->getCurrentTab(mITab1, mITab2);
    /* mask unavalibable energy table */
    if (cust_isDualFlashSupport(mFacingSensor)) {
        int i, j;
        int dutyIndex;

        FlashHal::getInstance(mSensorDev)->getCurrentTab(mITab1, mITab2);
        for (j = -1; j < mDutyNumLT; j++) {
            int current, currentLt;
            if (j != -1)
                currentLt = mITab2[j];
            else
                currentLt = 0;
            for (i = -1; i < mDutyNum; i++) {
                if (i != -1)
                    current = mITab1[i];
                else
                    current = 0;
                if (!mCurrentMaskFunc(current, currentLt)) {
                    FlashDuty::duty2ind(dutyIndex, i, j, mDutyNum, mDutyNumLT, 1);
                    mpFlashCaliNvbuf->Flash_Calibration[mFlashCaliNVRAMIdx].engTab.yTab[dutyIndex] = -1;
                }
            }
        }
    }

    copyTuningParaFlashAwb(&mFlashAwbTuning,
            &mpStrobeNvbuf->Flash_AWB[mFlashAWBNVRAMIdx].rTuningParam);

    for(int i=0; i<FLASH_CUSTOM_MAX_DUTY_NUM; i++){
        mFlashWBGain[i].i2R =
                mpFlashCaliNvbuf->Flash_Calibration[mFlashCaliNVRAMIdx].flashWBGain[i].i2R;
        mFlashWBGain[i].i2B =
                mpFlashCaliNvbuf->Flash_Calibration[mFlashCaliNVRAMIdx].flashWBGain[i].i2B;
    }

    return 0;
}

int FlashCustomAdapter::uninit()
{
    return 0;
}

#define MY_INST NS3Av3::INST_T<FlashCustomAdapter>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

FlashCustomAdapter *FlashCustomAdapter::getInstance(int sensorDev)
{
    int sensorOpenIndex = NS3Av3::mapSensorDevToIdx(sensorDev);
    if(sensorOpenIndex >= SENSOR_IDX_MAX || sensorOpenIndex < 0) {
        logE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, sensorOpenIndex);
        return NULL;
    }

    MY_INST& rSingleton = gMultiton[sensorOpenIndex];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<FlashCustomAdapter>(sensorDev);
    } );

    return rSingleton.instance.get();
}

int FlashCustomAdapter::getPfPolicy()
{
    return mPfPolicy;
}

int FlashCustomAdapter::getDutyNum()
{
    return mDutyNum;
}

int FlashCustomAdapter::getDutyNumLt()
{
    return mDutyNumLT;
}

int FlashCustomAdapter::getFlashHalInfo(FlashHalInfo *pFlashHalInfo)
{
    int ret = FlashNvram::nvReadStrobe(mpStrobeNvbuf, mSensorDev);
    if (ret){
        logE("FlashHal(): failed to read nvram(%d).", ret);
        mpStrobeNvbuf = NULL;
    }

    if (mpStrobeNvbuf) {
        pFlashHalInfo[FLASH_HAL_SCENARIO_VIDEO_TORCH].duty = mpStrobeNvbuf->Flash_AE[0].engLevel.torchDuty;
        pFlashHalInfo[FLASH_HAL_SCENARIO_VIDEO_TORCH].dutyLt = mpStrobeNvbuf->Flash_AE[0].engLevelLT.torchDuty;
        pFlashHalInfo[FLASH_HAL_SCENARIO_AF_LAMP].duty = mpStrobeNvbuf->Flash_AE[0].engLevel.afDuty;
        pFlashHalInfo[FLASH_HAL_SCENARIO_AF_LAMP].dutyLt = mpStrobeNvbuf->Flash_AE[0].engLevelLT.afDuty;
        pFlashHalInfo[FLASH_HAL_SCENARIO_PRE_FLASH].duty = mpStrobeNvbuf->Flash_AE[0].engLevel.pfDuty;
        pFlashHalInfo[FLASH_HAL_SCENARIO_PRE_FLASH].dutyLt = mpStrobeNvbuf->Flash_AE[0].engLevelLT.pfDuty;
        pFlashHalInfo[FLASH_HAL_SCENARIO_LOW_POWER].duty = mpStrobeNvbuf->Flash_AE[0].engLevel.mfDutyMaxL;
        pFlashHalInfo[FLASH_HAL_SCENARIO_LOW_POWER].dutyLt = mpStrobeNvbuf->Flash_AE[0].engLevelLT.mfDutyMaxL;
    }

    return 0;
}

int FlashCustomAdapter::getPfEng()
{
    int dutyIndex = 0;
    FlashDuty::duty2ind(dutyIndex,
            mpStrobeNvbuf->Flash_AE[mFlashAENVRAMIdx].engLevel.pfDuty,
            mpStrobeNvbuf->Flash_AE[mFlashAENVRAMIdx].engLevelLT.pfDuty,
            mDutyNum, mDutyNumLT,
            cust_isDualFlashSupport(mFacingSensor));
    return  mpFlashCaliNvbuf->Flash_Calibration[mFlashCaliNVRAMIdx].engTab.yTab[dutyIndex];
}

FLASH_AWB_ALGO_GAIN_N_T *FlashCustomAdapter::getFlashWBGain()
{
    return mFlashWBGain;
}

int FlashCustomAdapter::getDecideMfDutyIdx()
{
    int dutyIndex = 0;
    if (cust_isDualFlashSupport(mFacingSensor)==1)
    {
        FlashDuty::duty2ind(dutyIndex,
                mpStrobeNvbuf->Flash_AE[mFlashAENVRAMIdx].engLevel.mfDutyMin,
                mpStrobeNvbuf->Flash_AE[mFlashAENVRAMIdx].engLevelLT.mfDutyMin,
                mDutyNum, mDutyNumLT,
                cust_isDualFlashSupport(mFacingSensor));
    } else {
        dutyIndex = mpStrobeNvbuf->Flash_AE[mFlashAENVRAMIdx].engLevel.mfDutyMax;
    }
    return dutyIndex;
}

int FlashCustomAdapter::getMfDutyMax()
{
    return mpStrobeNvbuf->Flash_AE[mFlashAENVRAMIdx].engLevel.mfDutyMax;
}

int FlashCustomAdapter::getMfDutyMaxLt()
{
    return mpStrobeNvbuf->Flash_AE[mFlashAENVRAMIdx].engLevelLT.mfDutyMax;
}

int FlashCustomAdapter::getMfDutyMin(FlashCapInfo *pCapInfo)
{
    if (pCapInfo->isBurst == 1)
        return mpStrobeNvbuf->Flash_AE[mFlashAENVRAMIdx].engLevel.mfDutyMinB;
    else if (pCapInfo->isBatLow == 1)
        return mpStrobeNvbuf->Flash_AE[mFlashAENVRAMIdx].engLevel.mfDutyMinL;
    else if (!pCapInfo->isChargerReady)
        return mpStrobeNvbuf->Flash_AE[mFlashAENVRAMIdx].engLevel.mfDutyMinL;
    else
        return mpStrobeNvbuf->Flash_AE[mFlashAENVRAMIdx].engLevel.mfDutyMin;
}

int FlashCustomAdapter::getMfDutyMax(FlashCapInfo *pCapInfo)
{
    if (pCapInfo->isBurst == 1)
        return mpStrobeNvbuf->Flash_AE[mFlashAENVRAMIdx].engLevel.mfDutyMaxB;
    else if (pCapInfo->isBatLow == 1)
        return mpStrobeNvbuf->Flash_AE[mFlashAENVRAMIdx].engLevel.mfDutyMaxL;
    else if (!pCapInfo->isChargerReady)
        return mpStrobeNvbuf->Flash_AE[mFlashAENVRAMIdx].engLevel.mfDutyMaxL;
    else
        return mpStrobeNvbuf->Flash_AE[mFlashAENVRAMIdx].engLevel.mfDutyMax;
}

int FlashCustomAdapter::getMfDutyMinLt(FlashCapInfo *pCapInfo)
{
    if (pCapInfo->isBurst == 1)
        return mpStrobeNvbuf->Flash_AE[mFlashAENVRAMIdx].engLevelLT.mfDutyMinB;
    else if (pCapInfo->isBatLow == 1)
        return mpStrobeNvbuf->Flash_AE[mFlashAENVRAMIdx].engLevelLT.mfDutyMinL;
    else if (!pCapInfo->isChargerReady)
        return mpStrobeNvbuf->Flash_AE[mFlashAENVRAMIdx].engLevelLT.mfDutyMinL;
    else
        return mpStrobeNvbuf->Flash_AE[mFlashAENVRAMIdx].engLevelLT.mfDutyMin;
}

int FlashCustomAdapter::getMfDutyMaxLt(FlashCapInfo *pCapInfo)
{
    if (pCapInfo->isBurst == 1)
        return mpStrobeNvbuf->Flash_AE[mFlashAENVRAMIdx].engLevelLT.mfDutyMaxB;
    else if (pCapInfo->isBatLow == 1)
        return mpStrobeNvbuf->Flash_AE[mFlashAENVRAMIdx].engLevelLT.mfDutyMaxL;
    else if (!pCapInfo->isChargerReady)
        return mpStrobeNvbuf->Flash_AE[mFlashAENVRAMIdx].engLevelLT.mfDutyMaxL;
    else
        return mpStrobeNvbuf->Flash_AE[mFlashAENVRAMIdx].engLevelLT.mfDutyMax;
}

int FlashCustomAdapter::getPfDuty(FlashCapInfo *pCapInfo)
{
    if (pCapInfo->vBat < mpStrobeNvbuf->Flash_AE[mFlashAENVRAMIdx].engLevel.vBatL &&
            mpStrobeNvbuf->Flash_AE[mFlashAENVRAMIdx].engLevel.IChangeByVBatEn) {
        return  mpStrobeNvbuf->Flash_AE[mFlashAENVRAMIdx].engLevel.pfDutyL;
    } else if (pCapInfo->isBurst == 1 &&
                    mpStrobeNvbuf->Flash_AE[mFlashAENVRAMIdx].engLevel.IChangeByBurstEn) {
        return mpStrobeNvbuf->Flash_AE[mFlashAENVRAMIdx].engLevel.pfDutyB;
    } else {
        return mpStrobeNvbuf->Flash_AE[mFlashAENVRAMIdx].engLevel.pfDuty;
    }
}

int FlashCustomAdapter::getPfDutyLt(FlashCapInfo *pCapInfo)
{
    if (pCapInfo->vBat < mpStrobeNvbuf->Flash_AE[mFlashAENVRAMIdx].engLevel.vBatL &&
            mpStrobeNvbuf->Flash_AE[mFlashAENVRAMIdx].engLevel.IChangeByVBatEn) {
        return mpStrobeNvbuf->Flash_AE[mFlashAENVRAMIdx].engLevelLT.pfDutyL;
    } else if (pCapInfo->isBurst == 1 &&
                    mpStrobeNvbuf->Flash_AE[mFlashAENVRAMIdx].engLevel.IChangeByBurstEn) {
        return mpStrobeNvbuf->Flash_AE[mFlashAENVRAMIdx].engLevelLT.pfDutyB;
    } else {
        return mpStrobeNvbuf->Flash_AE[mFlashAENVRAMIdx].engLevelLT.pfDuty;
    }
}

int FlashCustomAdapter::getFlashMaxIDuty(int *duty, int *dutyLt)
{
    int max_duty = 0;
    int max_dutyLt = 0;
    int max_I = 0;

    FlashHal::getInstance(mSensorDev)->getCurrentTab(mITab1, mITab2);
    max_I = mITab1[max_duty]+ mITab2[max_dutyLt];

    for (int i = 0; i < mDutyNum; i++) {
        for (int j = 0; j < mDutyNumLT; j++) {
            if (mCurrentMaskFunc(mITab1[i],mITab2[j]) == 1)
                if (mITab1[i]+mITab2[j] >= max_I) {
                    max_I = mITab1[i]+mITab2[j];
                    max_duty = i;
                    max_dutyLt = j;
                }
        }
    }
    *duty = max_duty;
    *dutyLt = max_dutyLt;

    return 0;
}

int FlashCustomAdapter::isValidDuty(int duty, int dutyLt)
{
    int isValidDutyIndex = 0;
    if (!cust_isDualFlashSupport(mFacingSensor)){
        if (duty == -1)
            isValidDutyIndex = 1;
        else if(mCurrentMaskFunc(mITab1[duty], 0))
            isValidDutyIndex = 1;
    }
    else if (duty == -1 && dutyLt == -1)
        isValidDutyIndex = 1;
    else if (duty == -1 && mCurrentMaskFunc(0, mITab2[dutyLt]))
        isValidDutyIndex = 1;
    else if (dutyLt == -1 && mCurrentMaskFunc(mITab1[duty], 0))
        isValidDutyIndex = 1;
    else if (mCurrentMaskFunc(mITab1[duty], mITab2[dutyLt]))
        isValidDutyIndex = 1;
    return isValidDutyIndex;
}

int FlashCustomAdapter::getYTabByDutyIndex(int dutyIndex)
{
    return mpFlashCaliNvbuf->Flash_Calibration[mFlashCaliNVRAMIdx].engTab.yTab[dutyIndex];
}

int FlashCustomAdapter::setYTabByDutyIndex(int dutyIndex, int y)
{
    mpFlashCaliNvbuf->Flash_Calibration[mFlashCaliNVRAMIdx].engTab.yTab[dutyIndex] = y;
    return 0;
}

int FlashCustomAdapter::setWBGainByDutyIndex(int dutyIndex, double r, double g, double b)
{
    (void)g;
    mpFlashCaliNvbuf->Flash_Calibration[mFlashCaliNVRAMIdx].flashWBGain[dutyIndex].i2R = r;
    mpFlashCaliNvbuf->Flash_Calibration[mFlashCaliNVRAMIdx].flashWBGain[dutyIndex].i2B = b;
    return 0;
}

int FlashCustomAdapter::getWBRGainByDutyIndex(int dutyIndex)
{
    return mpFlashCaliNvbuf->Flash_Calibration[mFlashCaliNVRAMIdx].flashWBGain[dutyIndex].i2R;
}

int FlashCustomAdapter::getWBGGainByDutyIndex(int dutyIndex)
{
    (void)dutyIndex;
    return 512;
}

int FlashCustomAdapter::getWBBGainByDutyIndex(int dutyIndex)
{
    return mpFlashCaliNvbuf->Flash_Calibration[mFlashCaliNVRAMIdx].flashWBGain[dutyIndex].i2B;
}

int FlashCustomAdapter::clearYTab()
{
    for (int i = 0; i < FLASH_CUSTOM_MAX_DUTY_NUM; i++)
        mpFlashCaliNvbuf->Flash_Calibration[mFlashCaliNVRAMIdx].engTab.yTab[i] = 0;
    return 0;
}

int FlashCustomAdapter::showAWBTuningPara()
{
    logD("pfStart(): ForeGroundPercentage(%u), BackGroundPercentage(%u).",
        mpStrobeNvbuf->Flash_AWB[mFlashAWBNVRAMIdx].rTuningParam.ForeGroundPercentage,
        mpStrobeNvbuf->Flash_AWB[mFlashAWBNVRAMIdx].rTuningParam.BackGroundPercentage);
    logD("pfStart(): FgPercentage_Th(%u %u %u %u).",
        mpStrobeNvbuf->Flash_AWB[mFlashAWBNVRAMIdx].rTuningParam.FgPercentage_Th1,
        mpStrobeNvbuf->Flash_AWB[mFlashAWBNVRAMIdx].rTuningParam.FgPercentage_Th2,
        mpStrobeNvbuf->Flash_AWB[mFlashAWBNVRAMIdx].rTuningParam.FgPercentage_Th3,
        mpStrobeNvbuf->Flash_AWB[mFlashAWBNVRAMIdx].rTuningParam.FgPercentage_Th4);
    return 0;
}

int FlashCustomAdapter::flashAWBInit(FLASH_AWB_INIT_T *pFlashAwbInitParam)
{
    copyTuningParaFlashAwb(&mFlashAwbTuning,
            &mpStrobeNvbuf->Flash_AWB[mFlashAWBNVRAMIdx].rTuningParam);

    pFlashAwbInitParam->flash_awb_tuning_param = mFlashAwbTuning;
    return 0;
}

int FlashCustomAdapter::getAWBRotationMatrix(FlashAlgInputInfoStruct *pFlashAlgoInfo)
{
    NVRAM_CAMERA_3A_STRUCT *buf;
    if (FlashNvram::nvRead3A(buf, mSensorDev))
        logE("pfStart(): failed to get nvram.");

    pFlashAlgoInfo->AwbRotationMatrix.i4Cos =
        buf->AWB[AWBNVRAMMapping[CAM_SCENARIO_PREVIEW]].rAlgoCalParam.rRotationMatrix.i4Cos;
    pFlashAlgoInfo->AwbRotationMatrix.i4Sin =
        buf->AWB[AWBNVRAMMapping[CAM_SCENARIO_PREVIEW]].rAlgoCalParam.rRotationMatrix.i4Sin;

    return 0;
}

int FlashCustomAdapter::isVBatLow(FlashCapInfo *pCapInfo)
{
    if (mpStrobeNvbuf->Flash_AE[mFlashAENVRAMIdx].engLevel.IChangeByVBatEn == 1 &&
            pCapInfo->vBat < mpStrobeNvbuf->Flash_AE[mFlashAENVRAMIdx].engLevel.vBatL)
        return 1;
    return 0;
}

int FlashCustomAdapter::dumpNvEngTab()
{
    if (cust_isDualFlashSupport(mFacingSensor))
        dumpEngTab(&mpFlashCaliNvbuf->Flash_Calibration[mFlashCaliNVRAMIdx].engTab,
                mDutyNum + 1, mDutyNumLT + 1);
    else
        dumpEngTab(&mpFlashCaliNvbuf->Flash_Calibration[mFlashCaliNVRAMIdx].engTab,
                mDutyNum, mDutyNumLT);
    return 0;
}

int FlashCustomAdapter::dumpNvFlashAe(const char *fname)
{
    if (cust_isDualFlashSupport(mFacingSensor))
        dumpFlashAe(fname,
                mpFlashCaliNvbuf->Flash_Calibration[mFlashCaliNVRAMIdx].engTab.yTab,
                (mDutyNumLT + 1) * (mDutyNum + 1), mDutyNum + 1);
    else
        dumpFlashAe(fname,
                mpFlashCaliNvbuf->Flash_Calibration[mFlashCaliNVRAMIdx].engTab.yTab,
                mDutyNum, mDutyNum);
    return 0;
}

int FlashCustomAdapter::dumpNvFlashAwb(const char *fname, int len)
{
    if (cust_isDualFlashSupport(mFacingSensor))
        dumpFlashAwb(fname,
                mpFlashCaliNvbuf->Flash_Calibration[mFlashCaliNVRAMIdx].flashWBGain,
                len, 1, mDutyNum, mDutyNumLT);
    else
        dumpFlashAwb(fname,
                mpFlashCaliNvbuf->Flash_Calibration[mFlashCaliNVRAMIdx].flashWBGain,
                len, 0, mDutyNum, mDutyNumLT);
    return 0;

}

int FlashCustomAdapter::getFastKExp(int *pExpBase, int *pAfeBase, int *pIspBase)
{
    *pExpBase = mpFlashCaliNvbuf->Flash_Calibration[mFlashCaliNVRAMIdx].engTab.exp;
    *pAfeBase = mpFlashCaliNvbuf->Flash_Calibration[mFlashCaliNVRAMIdx].engTab.afe_gain;
    *pIspBase = mpFlashCaliNvbuf->Flash_Calibration[mFlashCaliNVRAMIdx].engTab.isp_gain;

    return 0;
}

/***********************************************************
 * Flash attributes from custom
 **********************************************************/
int FlashCustomAdapter::getFlashModeStyle(int sensorDev, int flashMode)
{
    return cust_getFlashModeStyle(sensorDev, flashMode);
}

FLASH_PROJECT_PARA &FlashCustomAdapter::getFlashProjectPara(int aeScene, int isForceFlash)
{
    FLASH_PROJECT_PARA &prjPara = cust_getFlashProjectPara_V3(mFacingSensor, aeScene, isForceFlash, mpStrobeNvbuf);
    copyTuningPara(&prjPara.tuningPara, &mpStrobeNvbuf->Flash_AE[mFlashAENVRAMIdx].tuningPara);
    copyTuningParaDualFlash(&prjPara.tuningPara, &mpStrobeNvbuf->Flash_AE[mFlashAENVRAMIdx].dualTuningPara);
    logD("getFlashProjectPara(): sensorDev(%d), aeMode(%d).", mSensorDev, aeScene);
    return prjPara;
}

int FlashCustomAdapter::setNVRAMIndex(FLASH_NVRAM_ENUM eNVRAM, MUINT32 a_eNVRAMIndex)
{
    switch (eNVRAM) {
    case FLASH_NVRAM_AE:
        if(a_eNVRAMIndex >= FlASH_AE_NUM_2) {
            logI("setNVRAMIndex(): F_AE Index(%d) exceed defined maximum(%d)", a_eNVRAMIndex, FlASH_AE_NUM_2);
        } else if(mFlashAENVRAMIdx != (MINT32)a_eNVRAMIndex) {
            mFlashAENVRAMIdx = a_eNVRAMIndex;
            logD("setNVRAMIndex(): FLASH_NVRAM_ENUM(%d), Index(%d)", eNVRAM, a_eNVRAMIndex);
        }
        break;
    case FLASH_NVRAM_AWB:
        if(a_eNVRAMIndex >= FlASH_AWB_NUM_2) {
            logI("setNVRAMIndex(): F_AWB Index(%d) exceed defined maximum(%d)", a_eNVRAMIndex, FlASH_AWB_NUM_2);
        } else if(mFlashAWBNVRAMIdx != (MINT32)a_eNVRAMIndex) {
            mFlashAWBNVRAMIdx = a_eNVRAMIndex;
            logD("setNVRAMIndex(): FLASH_NVRAM_ENUM(%d), Index(%d)", eNVRAM, a_eNVRAMIndex);
        }
        break;
    case FLASH_NVRAM_CALIBRATION:
        if(a_eNVRAMIndex >= FlASH_CALIBRATION_NUM_2) {
            logI("setNVRAMIndex(): F_Cali Index(%d) exceed defined maximum(%d)", a_eNVRAMIndex, FlASH_CALIBRATION_NUM_2);
        } else if(mFlashCaliNVRAMIdx != (MINT32)a_eNVRAMIndex) {
            mFlashCaliNVRAMIdx = a_eNVRAMIndex;
            logD("setNVRAMIndex(): FLASH_NVRAM_ENUM(%d), Index(%d)", eNVRAM, a_eNVRAMIndex);
        }
        break;
    default:
        logE("setNVRAMIndex(): invalid FLASH_NVRAM_ENUM(%d).", eNVRAM);
        return MFALSE;
    }
    return 0;
}

int FlashCustomAdapter::getFlashCaliNvIdx()
{
    return mFlashCaliNVRAMIdx;
}

int FlashCustomAdapter::getFaceFlashSupport()
{
    return cust_isFaceFlashSupport(mFacingSensor);
}

void FlashCustomAdapter::dumpProjectPara(FLASH_PROJECT_PARA *pp)
{
    /* verify arguments */
    if (!pp) {
        logE("dumpProjectPara(): error arguments.");
        return;
    }

    /* project parameters */
    logI("dumpProjectPara():");
    logI("dumpProjectPara(): dutyNum(%d).", pp->dutyNum);
    logI("dumpProjectPara(): maxCapExpTimeUs(%d us).", pp->maxCapExpTimeUs);

    /* tuning parameters */
    FLASH_TUNING_PARA *pt = &pp->tuningPara;
    logI("tuning parameters(%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d).",
            pt->yTarget,                     // 188 (10bit)
            pt->fgWIncreaseLevelbySize,      // 10
            pt->fgWIncreaseLevelbyRef,       // 5
            pt->ambientRefAccuracyRatio,     // 5 (5/256=2%)
            pt->flashRefAccuracyRatio,       // 0 (0/256=0%)
            pt->backlightAccuracyRatio,      // 18 (18/256=7%)
            pt->backlightUnderY,             // 40
            pt->backlightWeakRefRatio,       // 32
            pt->safetyExp,                   // 666444
            pt->maxUsableISO,                // 1200
            pt->yTargetWeight,               // 0 (base:256)
            pt->lowReflectanceThreshold,     // 13 (13/256=5%)
            pt->flashReflectanceWeight,      // 0 (base:256)
            pt->bgSuppressMaxDecreaseEV,     // 20
            pt->bgSuppressMaxOverExpRatio,   // 6 (6/256=2%)
            pt->fgEnhanceMaxIncreaseEV,      // 50
            pt->fgEnhanceMaxOverExpRatio,    // 2
            pt->isFollowCapPline,            // 1
            pt->histStretchMaxFgYTarget,     // 300 (10bit)
            pt->histStretchBrightestYTarget, // 480 (10bit)
            pt->fgSizeShiftRatio,            // 0
            pt->backlitPreflashTriggerLV,    // 90
            pt->backlitMinYTarget,           // 90
            pt->minstameanpass,              // 80
            pt->yDecreEvWeight,              // 188
            pt->yFaceTarget,                 // 188
            pt->cfgFlashPolicy               // 5

                );

    /* cooling time and timeout parameters */
    FLASH_COOL_TIMEOUT_PARA *pct = &pp->coolTimeOutPara;
    logI("dumpProjectPara(): tabNum(%d).", pct->tabNum);

    logI("dumpProjectPara(): tabId(%d %d %d %d %d %d %d %d %d %d).",
            pct->tabId[0], pct->tabId[1], pct->tabId[2], pct->tabId[3],
            pct->tabId[4], pct->tabId[5], pct->tabId[6], pct->tabId[7],
            pct->tabId[8], pct->tabId[9]);

    logI("dumpProjectPara(): coolingTM(%f %f %f %f %f %f %f %f %f %f).",
            pct->coolingTM[0], pct->coolingTM[1], pct->coolingTM[2], pct->coolingTM[3],
            pct->coolingTM[4], pct->coolingTM[5], pct->coolingTM[6], pct->coolingTM[7],
            pct->coolingTM[8], pct->coolingTM[9]);

    logI("dumpProjectPara(): timOutMs(%d %d %d %d %d %d %d %d %d %d).",
            pct->timOutMs[0], pct->timOutMs[1], pct->timOutMs[2], pct->timOutMs[3],
            pct->timOutMs[4], pct->timOutMs[5], pct->timOutMs[6], pct->timOutMs[7],
            pct->timOutMs[8], pct->timOutMs[9]);
}

int FlashCustomAdapter::getTorchDuty(int level, int *duty, int *dutyLt)
{
    (void)level;
    (void)*duty;
    (void)*dutyLt;
    return 0;
}

void FlashCustomAdapter::transformFlashCaliData()
{
    if(cust_transformFlashCaliData(mSensorDev, DUAL_CAMERA_MAIN_2_SENSOR))
    {
        transformFlashCaliData(mSensorDev, DUAL_CAMERA_MAIN_2_SENSOR);
    }
    if(cust_transformFlashCaliData(mSensorDev, DUAL_CAMERA_MAIN_3_SENSOR))
    {
        transformFlashCaliData(mSensorDev, DUAL_CAMERA_MAIN_3_SENSOR);
    }
}

void FlashCustomAdapter::transformFlashCaliData(int srcSensorDev, int dstSensorDev)
{
    FILE* fp;
    fp = fopen(FLASH_TRANSFORM_CALI_DATA_PATH, "a");

    if(fp != NULL)
    {
        logD("transformFlashCalibrationData(): source sensor ID(%d), destination sensor ID(%d).", srcSensorDev, dstSensorDev);
        fprintf(fp, "source sensor ID(%d), destination sensor ID(%d).\n", srcSensorDev, dstSensorDev);

        CAM_CAL_DATA_STRUCT GetCamCalData;
        CamCalDrvBase *pCamCalDrvObj = CamCalDrvBase::createInstance();
        CAMERA_CAM_CAL_TYPE_ENUM enCamCalEnum = CAMERA_CAM_CAL_DATA_3A_GAIN;
        pCamCalDrvObj->GetCamCalCalData(srcSensorDev, enCamCalEnum, (void *)&GetCamCalData);
        AWB_GAIN_T srcGoldGain;
        srcGoldGain.i4R = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rGoldGainu4R);
        srcGoldGain.i4G = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rGoldGainu4G);
        srcGoldGain.i4B = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rGoldGainu4B);
        AWB_GAIN_T srcUnitGain;
        srcUnitGain.i4R = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rUnitGainu4R);
        srcUnitGain.i4G = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rUnitGainu4G);
        srcUnitGain.i4B = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rUnitGainu4B);
        AWB_GAIN_T srcAwbPreGain;
        srcAwbPreGain.i4R = srcUnitGain.i4R * srcGoldGain.i4G * 512 / srcGoldGain.i4R / srcUnitGain.i4G;
        srcAwbPreGain.i4G = 512;
        srcAwbPreGain.i4B = srcUnitGain.i4B * srcGoldGain.i4G * 512 / srcGoldGain.i4B / srcUnitGain.i4G;
        logD("transformFlashCalibrationData(): source gold gain(%d,%d,%d), source unit gain(%d,%d,%d), source pregain(%d,%d,%d).",
            srcGoldGain.i4R, srcGoldGain.i4G, srcGoldGain.i4B,
            srcUnitGain.i4R, srcUnitGain.i4G, srcUnitGain.i4B,
            srcAwbPreGain.i4R, srcAwbPreGain.i4G, srcAwbPreGain.i4B);
        fprintf(fp, "source gold gain(%d,%d,%d), source unit gain(%d,%d,%d), source pregain(%d,%d,%d).\n",
            srcGoldGain.i4R, srcGoldGain.i4G, srcGoldGain.i4B,
            srcUnitGain.i4R, srcUnitGain.i4G, srcUnitGain.i4B,
            srcAwbPreGain.i4R, srcAwbPreGain.i4G, srcAwbPreGain.i4B);

        pCamCalDrvObj->GetCamCalCalData(dstSensorDev, enCamCalEnum, (void *)&GetCamCalData);
        AWB_GAIN_T dstGoldGain;
        dstGoldGain.i4R = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rGoldGainu4R);
        dstGoldGain.i4G = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rGoldGainu4G);
        dstGoldGain.i4B = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rGoldGainu4B);
        AWB_GAIN_T dstUnitGain;
        dstUnitGain.i4R = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rUnitGainu4R);
        dstUnitGain.i4G = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rUnitGainu4G);
        dstUnitGain.i4B = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rUnitGainu4B);
        AWB_GAIN_T dstAwbPreGain;
        dstAwbPreGain.i4R = dstUnitGain.i4R * dstGoldGain.i4G * 512 / dstGoldGain.i4R / dstUnitGain.i4G;
        dstAwbPreGain.i4G = 512;
        dstAwbPreGain.i4B = dstUnitGain.i4B * dstGoldGain.i4G * 512 / dstGoldGain.i4B / dstUnitGain.i4G;
        logD("transformFlashCalibrationData(): destination gold gain(%d,%d,%d), destination unit gain(%d,%d,%d), destination pregain(%d,%d,%d).",
            dstGoldGain.i4R, dstGoldGain.i4G, dstGoldGain.i4B,
            dstUnitGain.i4R, dstUnitGain.i4G, dstUnitGain.i4B,
            dstAwbPreGain.i4R, dstAwbPreGain.i4G, dstAwbPreGain.i4B);
        fprintf(fp, "destination gold gain(%d,%d,%d), destination unit gain(%d,%d,%d), destination pregain(%d,%d,%d).\n",
            dstGoldGain.i4R, dstGoldGain.i4G, dstGoldGain.i4B,
            dstUnitGain.i4R, dstUnitGain.i4G, dstUnitGain.i4B,
            dstAwbPreGain.i4R, dstAwbPreGain.i4G, dstAwbPreGain.i4B);

        NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT *temp;

        if (FlashNvram::nvReadFlashCaliDefault(temp, srcSensorDev))
            logE("transformFlashCalibrationData(): failed to read source default nvram.");
        NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT srcDefaultFlashCaliNvbuf;
        for(int i = 0; i < FLASH_CUSTOM_MAX_DUTY_NUM; i++)
        {
            srcDefaultFlashCaliNvbuf.Flash_Calibration[mFlashCaliNVRAMIdx].flashWBGain[i].i2R = temp->Flash_Calibration[mFlashCaliNVRAMIdx].flashWBGain[i].i2R;
            srcDefaultFlashCaliNvbuf.Flash_Calibration[mFlashCaliNVRAMIdx].flashWBGain[i].i2B = temp->Flash_Calibration[mFlashCaliNVRAMIdx].flashWBGain[i].i2B;
        }

        if (FlashNvram::nvReadFlashCaliDefault(temp, dstSensorDev))
            logE("transformFlashCalibrationData(): failed to read destination default nvram.");
        NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT dstDefaultFlashCaliNvbuf;
        for(int i = 0; i < FLASH_CUSTOM_MAX_DUTY_NUM; i++)
        {
            dstDefaultFlashCaliNvbuf.Flash_Calibration[mFlashCaliNVRAMIdx].flashWBGain[i].i2R = temp->Flash_Calibration[mFlashCaliNVRAMIdx].flashWBGain[i].i2R;
            dstDefaultFlashCaliNvbuf.Flash_Calibration[mFlashCaliNVRAMIdx].flashWBGain[i].i2B = temp->Flash_Calibration[mFlashCaliNVRAMIdx].flashWBGain[i].i2B;
        }

        NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT *srcFlashCaliNvbuf;
        if (FlashNvram::nvReadFlashCali(srcFlashCaliNvbuf, srcSensorDev, 1))
            logE("transformFlashCalibrationData(): failed to read source nvram.");

        NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT *dstFlashCaliNvbuf;
        if (FlashNvram::nvReadFlashCali(dstFlashCaliNvbuf, dstSensorDev, 1))
            logE("transformFlashCalibrationData(): failed to read destination nvram.");

        for(int i = 0; i < FLASH_CUSTOM_MAX_DUTY_NUM; i++)
        {
           dstFlashCaliNvbuf->Flash_Calibration[mFlashCaliNVRAMIdx].flashWBGain[i].i2R = dstDefaultFlashCaliNvbuf.Flash_Calibration[mFlashCaliNVRAMIdx].flashWBGain[i].i2R * srcFlashCaliNvbuf->Flash_Calibration[mFlashCaliNVRAMIdx].flashWBGain[i].i2R * dstAwbPreGain.i4R / srcAwbPreGain.i4R / srcDefaultFlashCaliNvbuf.Flash_Calibration[mFlashCaliNVRAMIdx].flashWBGain[i].i2R;
           dstFlashCaliNvbuf->Flash_Calibration[mFlashCaliNVRAMIdx].flashWBGain[i].i2B = dstDefaultFlashCaliNvbuf.Flash_Calibration[mFlashCaliNVRAMIdx].flashWBGain[i].i2B * srcFlashCaliNvbuf->Flash_Calibration[mFlashCaliNVRAMIdx].flashWBGain[i].i2B * dstAwbPreGain.i4B / srcAwbPreGain.i4B / srcDefaultFlashCaliNvbuf.Flash_Calibration[mFlashCaliNVRAMIdx].flashWBGain[i].i2B;
           logD("transformFlashCalibrationData(): source pregain(%d,%d), destination pregain(%d,%d), source flash golden gain(%d,%d), source flash unit gain(%d,%d),  destination Flash golden gain(%d,%d), destination flash unit gain(%d,%d).",
               srcAwbPreGain.i4R, srcAwbPreGain.i4B,
               dstAwbPreGain.i4R, dstAwbPreGain.i4B,
               srcDefaultFlashCaliNvbuf.Flash_Calibration[mFlashCaliNVRAMIdx].flashWBGain[i].i2R, srcDefaultFlashCaliNvbuf.Flash_Calibration[mFlashCaliNVRAMIdx].flashWBGain[i].i2B,
               srcFlashCaliNvbuf->Flash_Calibration[mFlashCaliNVRAMIdx].flashWBGain[i].i2R, srcFlashCaliNvbuf->Flash_Calibration[mFlashCaliNVRAMIdx].flashWBGain[i].i2B,
               dstDefaultFlashCaliNvbuf.Flash_Calibration[mFlashCaliNVRAMIdx].flashWBGain[i].i2R, dstDefaultFlashCaliNvbuf.Flash_Calibration[mFlashCaliNVRAMIdx].flashWBGain[i].i2B,
               dstFlashCaliNvbuf->Flash_Calibration[mFlashCaliNVRAMIdx].flashWBGain[i].i2R, dstFlashCaliNvbuf->Flash_Calibration[mFlashCaliNVRAMIdx].flashWBGain[i].i2B);
           fprintf(fp, "source pregain(%d,%d), destination pregain(%d,%d), source flash golden gain(%d,%d), source flash unit gain(%d,%d),  destination Flash golden gain(%d,%d), destination flash unit gain(%d,%d).\n",
               srcAwbPreGain.i4R, srcAwbPreGain.i4B,
               dstAwbPreGain.i4R, dstAwbPreGain.i4B,
               srcDefaultFlashCaliNvbuf.Flash_Calibration[mFlashCaliNVRAMIdx].flashWBGain[i].i2R, srcDefaultFlashCaliNvbuf.Flash_Calibration[mFlashCaliNVRAMIdx].flashWBGain[i].i2B,
               srcFlashCaliNvbuf->Flash_Calibration[mFlashCaliNVRAMIdx].flashWBGain[i].i2R, srcFlashCaliNvbuf->Flash_Calibration[mFlashCaliNVRAMIdx].flashWBGain[i].i2B,
               dstDefaultFlashCaliNvbuf.Flash_Calibration[mFlashCaliNVRAMIdx].flashWBGain[i].i2R, dstDefaultFlashCaliNvbuf.Flash_Calibration[mFlashCaliNVRAMIdx].flashWBGain[i].i2B,
               dstFlashCaliNvbuf->Flash_Calibration[mFlashCaliNVRAMIdx].flashWBGain[i].i2R, dstFlashCaliNvbuf->Flash_Calibration[mFlashCaliNVRAMIdx].flashWBGain[i].i2B);
        }

        FlashNvram::nvWriteFlashCali(dstSensorDev);

        fclose(fp);
    }
}
