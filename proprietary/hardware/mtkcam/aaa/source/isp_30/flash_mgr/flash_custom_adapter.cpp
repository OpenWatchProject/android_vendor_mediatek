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

#include <stddef.h>
#include <array>

/* aaa common headers */
#include "log_utils.h"
#include <private/aaa_utils.h>
#include <private/aaa_hal_private.h>

/* kernel headers */
#include "kd_camera_feature.h"

/* flash headers */
#include "flash_custom_adapter.h"
#include "flash_hal.h"
#include "flash_nvram.h"
#include "flash_duty.h"
#include "tools/flash_custom_utils.h"
#include "flash_utils.h"
#include "flash_cct.h"

/* custom headers */
#include "flash_tuning_custom.h"

FlashCustomAdapter::FlashCustomAdapter(int sensorDev)
    : mSensorDev(sensorDev)
    , mFacingSensor(0)
    , mp3ANvbuf(NULL)
    , mpStrobeNvbuf(NULL)
    , mpFlashCaliNvbuf(NULL)
    , mFlashAENVRAMIdx(0)
    , mFlashAWBNVRAMIdx(0)
    , mFlashCaliNVRAMIdx(0)
    , mPfPolicy(0)
    , mDutyNum(0)
    , mDutyNumLT(0)
    , mDutyMaskFunc(NULL)
{
    /* set debug */
    setDebug();

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
    /* get nvram 3A data */
    if (FlashNvram::nvRead3A(mp3ANvbuf, mSensorDev))
        logE("init(): failed to read nvram.");

    /* get nvram strobe data */
    if (FlashNvram::nvReadStrobe(mpStrobeNvbuf, mSensorDev))
        logE("init(): failed to read nvram.");

    /* get nvram flash calibration data */
    if (FlashNvram::nvReadFlashCali(mpFlashCaliNvbuf, mSensorDev))
        logE("init(): failed to read nvram.");

    /* get project parameters */
    FLASH_PROJECT_PARA prjPara;
    prjPara = getFlashProjectPara(LIB3A_AE_SCENE_AUTO, 0);

    mDutyNum = prjPara.dutyNum;
    mDutyNumLT = prjPara.dutyNumLT;

    /* update pre-flash flow policy */
    mPfPolicy = prjPara.tuningPara.cfgFlashPolicy;

    /* verify energy table */
    if (verifyYTab(mpFlashCaliNvbuf->yTab, mDutyNum, mDutyNumLT,
                cust_isDualFlashSupport(mFacingSensor)))
        logE("init(): invalid energy table.");

    FlashHal::getInstance(mSensorDev)->getCurrentTab(mITab1, mITab2);
    /* mask unavalibable energy table */
    if (cust_isDualFlashSupport(mFacingSensor)) {
        int i, j;
        int dutyIndex;
        mDutyMaskFunc = prjPara.dutyAvailableMaskFunc;
        for (j = -1; j < prjPara.dutyNumLT; j++)
            for (i = -1; i < prjPara.dutyNum; i++) {
                if (!mDutyMaskFunc(i, j)) {
                    FlashDuty::duty2ind(dutyIndex, i, j, mDutyNum, mDutyNumLT, 1);
                    mpFlashCaliNvbuf->yTab[dutyIndex] = -1;
                }
            }
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
        pFlashHalInfo[FLASH_HAL_SCENARIO_VIDEO_TORCH].duty = mpStrobeNvbuf->engLevel.torchDuty;
        pFlashHalInfo[FLASH_HAL_SCENARIO_VIDEO_TORCH].dutyLt = mpStrobeNvbuf->engLevelLT.torchDuty;
        pFlashHalInfo[FLASH_HAL_SCENARIO_AF_LAMP].duty = mpStrobeNvbuf->engLevel.afDuty;
        pFlashHalInfo[FLASH_HAL_SCENARIO_AF_LAMP].dutyLt = mpStrobeNvbuf->engLevelLT.afDuty;
        pFlashHalInfo[FLASH_HAL_SCENARIO_PRE_FLASH].duty = mpStrobeNvbuf->engLevel.pfDuty;
        pFlashHalInfo[FLASH_HAL_SCENARIO_PRE_FLASH].dutyLt = mpStrobeNvbuf->engLevelLT.pfDuty;
        pFlashHalInfo[FLASH_HAL_SCENARIO_LOW_POWER].duty = mpStrobeNvbuf->engLevel.mfDutyMaxL;
        pFlashHalInfo[FLASH_HAL_SCENARIO_LOW_POWER].dutyLt = mpStrobeNvbuf->engLevelLT.mfDutyMaxL;
    }
    return 0;
}

int FlashCustomAdapter::getPfEng()
{
    int dutyIndex = 0;
    FlashDuty::duty2ind(dutyIndex,
            mpStrobeNvbuf->engLevel.pfDuty,
            mpStrobeNvbuf->engLevelLT.pfDuty,
            mDutyNum, mDutyNumLT,
            cust_isDualFlashSupport(mFacingSensor));
    return  mpFlashCaliNvbuf->yTab[dutyIndex];
}

AWB_GAIN_T *FlashCustomAdapter::getFlashWBGain()
{
    return mpFlashCaliNvbuf->flashWBGain;
}

int FlashCustomAdapter::getDecideMfDutyIdx()
{
    int dutyIndex = 0;
    if (cust_isDualFlashSupport(mFacingSensor)==1)
    {
        FlashDuty::duty2ind(dutyIndex,
                mpStrobeNvbuf->engLevel.mfDutyMin,
                mpStrobeNvbuf->engLevelLT.mfDutyMin,
                mDutyNum, mDutyNumLT,
                cust_isDualFlashSupport(mFacingSensor));
    } else {
        dutyIndex = mpStrobeNvbuf->engLevel.mfDutyMax;
    }
    return dutyIndex;
}

int FlashCustomAdapter::getMfDutyMax()
{
    return mpStrobeNvbuf->engLevel.mfDutyMax;
}

int FlashCustomAdapter::getMfDutyMaxLt()
{
    return mpStrobeNvbuf->engLevelLT.mfDutyMax;
}

int FlashCustomAdapter::getMfDutyMin(FlashCapInfo *pCapInfo)
{
    if (pCapInfo->isBurst == 1)
        return mpStrobeNvbuf->engLevel.mfDutyMinB;
    else if (pCapInfo->isBatLow == 1)
        return mpStrobeNvbuf->engLevel.mfDutyMinL;
    else if (!pCapInfo->isChargerReady)
        return mpStrobeNvbuf->engLevel.mfDutyMinL;
    else
        return mpStrobeNvbuf->engLevel.mfDutyMin;
}

int FlashCustomAdapter::getMfDutyMax(FlashCapInfo *pCapInfo)
{
    if (pCapInfo->isBurst == 1)
        return mpStrobeNvbuf->engLevel.mfDutyMaxB;
    else if (pCapInfo->isBatLow == 1)
        return mpStrobeNvbuf->engLevel.mfDutyMaxL;
    else if (!pCapInfo->isChargerReady)
        return mpStrobeNvbuf->engLevel.mfDutyMaxL;
    else
        return mpStrobeNvbuf->engLevel.mfDutyMax;
}

int FlashCustomAdapter::getMfDutyMinLt(FlashCapInfo *pCapInfo)
{
    if (pCapInfo->isBurst == 1)
        return mpStrobeNvbuf->engLevelLT.mfDutyMinB;
    else if (pCapInfo->isBatLow == 1)
        return mpStrobeNvbuf->engLevelLT.mfDutyMinL;
    else if (!pCapInfo->isChargerReady)
        return mpStrobeNvbuf->engLevelLT.mfDutyMinL;
    else
        return mpStrobeNvbuf->engLevelLT.mfDutyMin;
}

int FlashCustomAdapter::getMfDutyMaxLt(FlashCapInfo *pCapInfo)
{
    if (pCapInfo->isBurst == 1)
        return mpStrobeNvbuf->engLevelLT.mfDutyMaxB;
    else if (pCapInfo->isBatLow == 1)
        return mpStrobeNvbuf->engLevelLT.mfDutyMaxL;
    else if (!pCapInfo->isChargerReady)
        return mpStrobeNvbuf->engLevelLT.mfDutyMaxL;
    else
        return mpStrobeNvbuf->engLevelLT.mfDutyMax;
}

int FlashCustomAdapter::getPfDuty(FlashCapInfo *pCapInfo)
{
    if (pCapInfo->vBat < mpStrobeNvbuf->engLevel.vBatL &&
            mpStrobeNvbuf->engLevel.IChangeByVBatEn) {
        return  mpStrobeNvbuf->engLevel.pfDutyL;
    } else if (pCapInfo->isBurst == 1 &&
                    mpStrobeNvbuf->engLevel.IChangeByBurstEn) {
        return mpStrobeNvbuf->engLevel.pfDutyB;
    } else {
        return mpStrobeNvbuf->engLevel.pfDuty;
    }
}

int FlashCustomAdapter::getPfDutyLt(FlashCapInfo *pCapInfo)
{
    if (pCapInfo->vBat < mpStrobeNvbuf->engLevel.vBatL &&
            mpStrobeNvbuf->engLevel.IChangeByVBatEn) {
        return mpStrobeNvbuf->engLevelLT.pfDutyL;
    } else if (pCapInfo->isBurst == 1 &&
                    mpStrobeNvbuf->engLevel.IChangeByBurstEn) {
        return mpStrobeNvbuf->engLevelLT.pfDutyB;
    } else {
        return mpStrobeNvbuf->engLevelLT.pfDuty;
    }
}

int FlashCustomAdapter::getFlashMaxIDuty(int *duty, int *dutyLt)
{
    cust_getFlashMaxIDuty(mFacingSensor, mDutyNum, mDutyNumLT, duty, dutyLt);
    return 0;
}

int FlashCustomAdapter::isValidDuty(int duty, int dutyLt)
{
    int isValidDutyIndex = 0;
    if (!cust_isDualFlashSupport(mFacingSensor))
        isValidDutyIndex = 1;
    else if (duty == -1 && dutyLt == -1)
        isValidDutyIndex = 1;
    else if (mDutyMaskFunc(duty, dutyLt))
        isValidDutyIndex = 1;
    return isValidDutyIndex;
}

int FlashCustomAdapter::getYTabByDutyIndex(int dutyIndex)
{
    return mpFlashCaliNvbuf->yTab[dutyIndex];
}

int FlashCustomAdapter::setYTabByDutyIndex(int dutyIndex, int y)
{
    mpFlashCaliNvbuf->yTab[dutyIndex] = y;
    return 0;
}

int FlashCustomAdapter::setWBGainByDutyIndex(int dutyIndex, double r, double g, double b)
{
    logI("Index:%d %lf %lf", dutyIndex, r, b);
    mpFlashCaliNvbuf->flashWBGain[dutyIndex].i4R = r;
    mpFlashCaliNvbuf->flashWBGain[dutyIndex].i4G = g;
    mpFlashCaliNvbuf->flashWBGain[dutyIndex].i4B = b;
    return 0;
}

int FlashCustomAdapter::getWBRGainByDutyIndex(int dutyIndex)
{
    return mpFlashCaliNvbuf->flashWBGain[dutyIndex].i4R;
}

int FlashCustomAdapter::getWBGGainByDutyIndex(int dutyIndex)
{
    return mpFlashCaliNvbuf->flashWBGain[dutyIndex].i4G;
}

int FlashCustomAdapter::getWBBGainByDutyIndex(int dutyIndex)
{
    return mpFlashCaliNvbuf->flashWBGain[dutyIndex].i4B;
}

int FlashCustomAdapter::clearYTab()
{
    for (int i = 0; i < FLASH_CUSTOM_MAX_DUTY_NUM; i++)
        mpFlashCaliNvbuf->yTab[i] = 0;
    return 0;
}

int FlashCustomAdapter::showAWBTuningPara()
{
    logD("pfStart(): ForeGroundPercentage(%u), BackGroundPercentage(%u).",
        mp3ANvbuf->rFlashAWBNVRAM.rTuningParam.ForeGroundPercentage,
        mp3ANvbuf->rFlashAWBNVRAM.rTuningParam.BackGroundPercentage);
    logD("pfStart(): FgPercentage_Th(%u %u %u %u).",
        mp3ANvbuf->rFlashAWBNVRAM.rTuningParam.FgPercentage_Th1,
        mp3ANvbuf->rFlashAWBNVRAM.rTuningParam.FgPercentage_Th2,
        mp3ANvbuf->rFlashAWBNVRAM.rTuningParam.FgPercentage_Th3,
        mp3ANvbuf->rFlashAWBNVRAM.rTuningParam.FgPercentage_Th4);
    return 0;
}

int FlashCustomAdapter::flashAWBInit(FLASH_AWB_INIT_T *pFlashAwbInitParam)
{

    pFlashAwbInitParam->flash_awb_tuning_param =
        mp3ANvbuf->rFlashAWBNVRAM.rTuningParam;
    return 0;
}

int FlashCustomAdapter::getAWBRotationMatrix(FlashAlgInputInfoStruct *pFlashAlgoInfo)
{
    pFlashAlgoInfo->AwbRotationMatrix.i4Cos =
        mp3ANvbuf->rAWBNVRAM[AWBNVRAMMapping[CAM_SCENARIO_PREVIEW]].rAlgoCalParam.rRotationMatrix.i4Cos;
    pFlashAlgoInfo->AwbRotationMatrix.i4Sin =
        mp3ANvbuf->rAWBNVRAM[AWBNVRAMMapping[CAM_SCENARIO_PREVIEW]].rAlgoCalParam.rRotationMatrix.i4Sin;

    return 0;
}

int FlashCustomAdapter::isVBatLow(FlashCapInfo *pCapInfo)
{
    if (mpStrobeNvbuf->engLevel.IChangeByVBatEn == 1 &&
            pCapInfo->vBat < mpStrobeNvbuf->engLevel.vBatL)
        return 1;
    return 0;
}

int FlashCustomAdapter::dumpNvEngTab()
{
    return 0;
}

int FlashCustomAdapter::dumpNvFlashAe(const char *fname)
{
    if (cust_isDualFlashSupport(mFacingSensor))
        dumpFlashAe(fname,
                mpFlashCaliNvbuf->yTab,
                (mDutyNumLT + 1) * (mDutyNum + 1), mDutyNum + 1);
    else
        dumpFlashAe(fname,
                mpFlashCaliNvbuf->yTab,
                mDutyNum, mDutyNum);
    return 0;
}

int FlashCustomAdapter::dumpNvFlashAwb(const char *fname, int len)
{
    if (cust_isDualFlashSupport(mFacingSensor))
        dumpFlashAwb(fname,
                mpFlashCaliNvbuf->flashWBGain,
                len, 1, mDutyNum, mDutyNumLT);
    else
        dumpFlashAwb(fname,
                mpFlashCaliNvbuf->flashWBGain,
                len, 0, mDutyNum, mDutyNumLT);
    return 0;

}

int FlashCustomAdapter::getFastKExp(int *pExpBase, int *pAfeBase, int *pIspBase)
{
    cust_getFlashQuick2CalibrationExp(mFacingSensor, pExpBase, pAfeBase, pIspBase);
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
    return cust_getFlashProjectPara_V3(mFacingSensor, aeScene, isForceFlash, mpStrobeNvbuf);
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
    //isp40 not support face flash.
    return 0;
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
            pt->yDecreEVTarget,              // 188
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
    cust_getTorchDuty(mSensorDev, level, duty, dutyLt);
    return 0;
}

void FlashCustomAdapter::transformFlashCaliData()
{
}
