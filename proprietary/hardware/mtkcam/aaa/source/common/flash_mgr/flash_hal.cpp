#define LOG_TAG "FlashHal"

/* camera headers */
#include "mtkcam/drv/IHalSensor.h"
/* aaa headers */
#include "strobe_drv.h"

/* aaa common headers */
#include "property_utils.h"
#include "time_utils.h"
#include "math_utils.h"
#include <private/aaa_utils.h>
#include <private/aaa_hal_private.h>

/* custom headers */
#include "flash_param.h"
#include "camera_custom_nvram.h"
#include "flash_tuning_custom.h"
#include "strobe_param.h"

/* flash headers */
#include "flash_hal.h"
#include "flash_mgr.h"
#include "flash_utils.h"
#include "flash_nvram.h"
#include "flash_custom_adapter.h"

/* kernel headers */
#include "kd_camera_feature.h"

#include <array>

using namespace NSCam;

/***********************************************************
 * Strobe handler
 **********************************************************/
STROBE_DEVICE_ENUM getStrobeDevice(int sensorDev)
{
    /*
     * typedef enum {
     *    STROBE_DEVICE_NONE = 0,
     *    STROBE_DEVICE_FLASHLIGHT,
     *    STROBE_DEVICE_LED,
     *    STROBE_DEVICE_XENON,
     *    STROBE_DEVICE_DISPLAY,
     *    STROBE_DEVICE_IR,
     * } STROBE_DEVICE_ENUM;
     */
    return cust_getStrobeDevice(sensorDev);
}

STROBE_TYPE_ENUM getStrobeType(int sensorDev)
{
    int facing = 0, num = 0;
    NS3Av3::mapSensorDevToFace(sensorDev, facing, num);

    if (facing == 1)
        return STROBE_TYPE_FRONT;

    return STROBE_TYPE_REAR;
}

int getStrobeCtNum(int typeId)
{
    /*
     * WARNING, be careful of mapping relationship between
     *
     * typedef enum {
     *    STROBE_TYPE_NONE = 0,
     *    STROBE_TYPE_REAR = 1,
     *    STROBE_TYPE_FRONT = 2,
     * } STROBE_TYPE_ENUM;
     *
     * typedef enum {
     *    DUAL_CAMERA_NONE_SENSOR = 0,
     *    DUAL_CAMERA_MAIN_SENSOR = 1,
     *    DUAL_CAMERA_SUB_SENSOR = 2,
     *    DUAL_CAMERA_MAIN_2_SENSOR = 4,
     *    DUAL_CAMERA_MAIN_SECOND_SENSOR = 4,
     *    DUAL_CAMERA_SUB_2_SENSOR   = 8,
     *    DUAL_CAMERA_SENSOR_MAX
     * } CAMERA_DUAL_CAMERA_SENSOR_ENUM;
     */
    if (cust_isDualFlashSupport(typeId))
        return 2;
    return 1;
}

/***********************************************************
 * Flash Hal
 **********************************************************/
IHalFlash *IHalFlash::getInstance(MINT32 const i4SensorOpenIdx)
{
    IHalSensorList *const pHalSensorList = MAKE_HalSensorList();
    int sensorDev = (!pHalSensorList) ? 0 : pHalSensorList->querySensorDevIdx(i4SensorOpenIdx);
    return FlashHal::getInstance(sensorDev);
}

FlashHal::FlashHal(int sensorDev)
    : mSensorDev(sensorDev)
    , mTorchStatus(0)
    , mDriverFault(0)
    , mMaxDuty(0)
    , mMaxDutyLT(0)
    , mHasHw(0)
    , mInCharge(1)
    , mSubFlashCustomization(0)
    , mpStrobe(NULL)
    , mpStrobe2(NULL)
    , mStrobeDevice(STROBE_DEVICE_NONE)
    , mStrobeTypeId(STROBE_TYPE_NONE)
    , mStrobeCtNum(0)
    , mStrobePartId(1)
    , mTorchLevel(-1)
{
    /* set debug */
    setDebug();

    logI("FlashHal(): sensorDev(%d).", mSensorDev);

    /* clear data structure */
    memset(&mFlashHalTimeInfo, 0, sizeof(mFlashHalTimeInfo));
    memset(&mFlashHalInfo, 0, sizeof(mFlashHalInfo));
    memset(&mPrjPara, 0, sizeof(FLASH_PROJECT_PARA));

    /* setup strobe hardware */
    mStrobeDevice = getStrobeDevice(mSensorDev);
    mStrobeTypeId = getStrobeType(mSensorDev);
    mStrobeCtNum = getStrobeCtNum(mStrobeTypeId);
    mpStrobe = StrobeDrv::getInstance(mStrobeDevice, mStrobeTypeId, 1);
    if (!mpStrobe) {
        logE("FlashHal(): mpStrobe NULL, use default");
        mpStrobe = StrobeDrv::getInstance(STROBE_DEVICE_FLASHLIGHT, 1, 1);
    }

    if (mStrobeCtNum > 1) {
        mpStrobe2 = StrobeDrv::getInstance(mStrobeDevice, mStrobeTypeId, 2);
        if (!mpStrobe2) {
            logE("FlashHal(): mpStrobe2 NULL, use default");
            mpStrobe2 = StrobeDrv::getInstance(STROBE_DEVICE_FLASHLIGHT, 1, 2);
        }
    }

    mpStrobe->getPartId(&mStrobePartId);
    if (mStrobePartId < 1 || mStrobePartId > 2) {
        logE("FlashHal(): invalid part id.");
        mStrobePartId = 1;
    }
    cust_setFlashPartId(mStrobeTypeId, mStrobePartId); /* WARNING, sernsorDev in custom */
    mpStrobe->hasFlashHw(&mHasHw);

    /* get nvram strobe data */
    int ret;
    NVRAM_CAMERA_STROBE_STRUCT *pNvram = NULL;
    ret = FlashNvram::nvReadStrobe(pNvram, mSensorDev);
    if (ret){
        logE("FlashHal(): failed to read nvram(%d).", ret);
        pNvram = NULL;
    }

    /*
     * Get project parameters.
     * It's fine if failed to get NVRAM. (pNvram is NULL).
     */
    mPrjPara = cust_getFlashProjectPara_V3(mStrobeTypeId, LIB3A_AE_SCENE_AUTO, 0, pNvram);

    /* set strobe info */
    mpStrobe->setStrobeInfo(mPrjPara.dutyNum,
            mPrjPara.coolTimeOutPara.tabNum, mPrjPara.coolTimeOutPara.tabId,
            mPrjPara.coolTimeOutPara.timOutMs, mPrjPara.coolTimeOutPara.coolingTM);
    if (mStrobeCtNum > 1)
        mpStrobe2->setStrobeInfo(mPrjPara.dutyNumLT,
                mPrjPara.coolTimeOutParaLT.tabNum, mPrjPara.coolTimeOutParaLT.tabId,
                mPrjPara.coolTimeOutParaLT.timOutMs, mPrjPara.coolTimeOutParaLT.coolingTM);

    /* setup flash hal info */
    int duty = 0;
    int dutyLt = 0;
    cust_getFlashHalTorchDuty(mStrobeTypeId, &duty, &dutyLt); /* WARNING, sernsorDev in custom */
    mFlashHalInfo[FLASH_HAL_SCENARIO_TORCH].duty = duty;
    mFlashHalInfo[FLASH_HAL_SCENARIO_TORCH].dutyLt = dutyLt;
    /*For its pass. The parameter should be obtained from NVRAM*/
    mFlashHalInfo[FLASH_HAL_SCENARIO_MAIN_FLASH].duty = duty;
    mFlashHalInfo[FLASH_HAL_SCENARIO_MAIN_FLASH].dutyLt = dutyLt;

    FlashCustomAdapter::getInstance(mSensorDev)->getFlashHalInfo(mFlashHalInfo);

    /* show attribute */
    show();
}

#define MY_INST NS3Av3::INST_T<FlashHal>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

FlashHal *FlashHal::getInstance(int sensorDev)
{
    int sensorOpenIndex = NS3Av3::mapSensorDevToIdx(sensorDev);
    if(sensorOpenIndex >= SENSOR_IDX_MAX || sensorOpenIndex < 0) {
        logE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, sensorOpenIndex);
        return NULL;
    }

    MY_INST& rSingleton = gMultiton[sensorOpenIndex];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<FlashHal>(sensorDev);
    } );

    return rSingleton.instance.get();
}

void FlashHal::destroyInstance()
{
}


/***********************************************************
 * Misc
 **********************************************************/
static int verifyScenario(FLASH_HAL_SCENARIO_ENUM scenario)
{
    if (scenario < FLASH_HAL_SCENARIO_TORCH || scenario >= FLASH_HAL_SCENARIO_NUM)
        return -1;
    return 0;
}

void FlashHal::show()
{
    logI("Device: (%d), Type: (%d), CT number: (%d), Part ID: (%d), HW(%d), in charge(%d).",
            mStrobeDevice, mStrobeTypeId, mStrobeCtNum, mStrobePartId, mHasHw, mInCharge);
}

/***********************************************************
 * Life cycle
 **********************************************************/
int FlashHal::init()
{
    logI("init().");

    mpStrobe->init();
    mpStrobe->getMaxDuty(&mMaxDuty);
    mpStrobe->lowPowerDetectStart(mFlashHalInfo[FLASH_HAL_SCENARIO_LOW_POWER].duty);
    if (mStrobeCtNum > 1) {
        mpStrobe2->init();
        mpStrobe2->getMaxDuty(&mMaxDutyLT);
        mpStrobe2->lowPowerDetectStart(mFlashHalInfo[FLASH_HAL_SCENARIO_LOW_POWER].dutyLt);
    }

    return 0;
}

int FlashHal::uninit()
{
    logI("uninit().");

    mpStrobe->lowPowerDetectEnd();
    mpStrobe->uninit();
    if (mStrobeCtNum > 1) {
        mpStrobe2->lowPowerDetectEnd();
        mpStrobe2->uninit();
    }

    return 0;
}

/***********************************************************
 * Attribute
 **********************************************************/
int FlashHal::setInfo(FLASH_HAL_SCENARIO_ENUM scenario, FlashHalInfo info)
{
    /* verify arguments */
    if (verifyScenario(scenario)) {
        logE("setInfo(): invalid arguments.");
        return -1;
    }

    logI("setInfo(): scenario(%d).", scenario);
    mFlashHalInfo[scenario] = info;

    return 0;
}

int FlashHal::setInfoDuty(FLASH_HAL_SCENARIO_ENUM scenario, FlashHalInfo info)
{
    /* verify arguments */
    if (verifyScenario(scenario)) {
        logE("setInfoDuty(): invalid arguments.");
        return -1;
    }

    logI("setInfoDuty(): scenario(%d).", scenario);

    int timeout;
    mpStrobe->getTimeOutTime(info.duty, &timeout);
    if (timeout == ENUM_FLASH_TIME_NO_TIME_OUT)
        timeout = 0;
    info.timeout = timeout;

    if (mStrobeCtNum > 1) {
        mpStrobe2->getTimeOutTime(info.dutyLt, &timeout);
        if (timeout == ENUM_FLASH_TIME_NO_TIME_OUT)
            timeout = 0;
        info.timeoutLt = timeout;
    }
    mFlashHalInfo[scenario] = info;

    return 0;
}

FlashHalTimeInfo FlashHal::getTimeInfo()
{
    logD("getTimeInfo(): main flash period(%d,%d), timeout(%d,%d), is timeout(%d).",
            mFlashHalTimeInfo.mfStartTime, mFlashHalTimeInfo.mfEndTime,
            mFlashHalTimeInfo.mfTimeout, mFlashHalTimeInfo.mfTimeoutLt,
            mFlashHalTimeInfo.mfIsTimeout);
    return mFlashHalTimeInfo;
}

int FlashHal::getDriverFault()
{
    return mDriverFault;
}

int FlashHal::hasHw(int &hasHw)
{
    hasHw = mHasHw;
    if (!hasHw)
        logD("hasHw(): hasHw(%d).", hasHw);

    return 0;
}

int FlashHal::getPartId()
{
    return mStrobePartId;
}

int FlashHal::getInCharge()
{
    return mInCharge;
}

int FlashHal::setInCharge(int inCharge)
{
    mInCharge = inCharge;
    logI("setInCharge(): sensorDev(%d), inCharge(%d).",
            mSensorDev, mInCharge);
    return 0;
}

int FlashHal::isAvailable()
{
    int isAvailable = mHasHw && mInCharge;

    if (!isAvailable)
        logD("isAvailable(): hasHw(%d), inCharge(%d).",
                mHasHw, mInCharge);

    return isAvailable;
}

int FlashHal::getBattVol(int *battVol)
{
    return mpStrobe->getBattVol(battVol);
}

int FlashHal::isLowPower(int *battStatus)
{
    return mpStrobe->isLowPower(battStatus);
}

int FlashHal::isNeedWaitCooling(int curMs, int *waitTimeMs)
{
    int isNeedWait = 1;
    float coolTM;
    float coolTMLt;
    int waitTime;
    int waitTimeLt;

    /* get wait time */
    mpStrobe->getCoolTM(mFlashHalInfo[FLASH_HAL_SCENARIO_MAIN_FLASH].duty, &coolTM);
    waitTime = mFlashHalTimeInfo.mfEndTime +
        (mFlashHalTimeInfo.mfEndTime - mFlashHalTimeInfo.mfStartTime) * coolTM -
        (curMs + 300);
    if (mStrobeCtNum > 1) {
        mpStrobe2->getCoolTM(mFlashHalInfo[FLASH_HAL_SCENARIO_MAIN_FLASH].dutyLt, &coolTMLt);
        waitTimeLt = mFlashHalTimeInfo.mfEndTime +
            (mFlashHalTimeInfo.mfEndTime - mFlashHalTimeInfo.mfStartTime) * coolTMLt -
            (curMs + 300);
        waitTime = max(waitTime, waitTimeLt);
    }

    /* need to wait or not */
    if (waitTime > 5000)
        waitTime = 5000;
    else if (waitTime < 0) {
        isNeedWait = 0;
        waitTime = 0;
    }
    *waitTimeMs = waitTime;

    return isNeedWait;
}

int FlashHal::getMaxDuty(int *duty, int *dutyLT)
{
    *duty = mMaxDuty;
    *dutyLT = mMaxDutyLT;
    return 0;
}

int FlashHal::getCurrentByDuty(int duty)
{
    int current;

    mpStrobe->getCurrentByDuty(duty, &current);

    return current;
}

int FlashHal::getCurrentTab(short *ITabHt, short *ITabLt)
{
    mpStrobe->getCurrentTable(ITabHt);
    if (mStrobeCtNum > 1)
        mpStrobe2->getCurrentTable(ITabLt);
    return 0;
}

/***********************************************************
 * On/Off function
 **********************************************************/
int FlashHal::isChargerReady(int *chargerStatus)
{
    return mpStrobe->isChargerReady(chargerStatus);
}

int FlashHal::setCharger(int ready)
{
    mpStrobe->setCharger(ready);
    if (mStrobeCtNum > 1)
        mpStrobe2->setCharger(ready);

    return 0;
}

int FlashHal::isFlashOn()
{
    int on = 0;
    int onLt = 0;
    mpStrobe->isOn(&on);
    if (mStrobeCtNum > 1) {
        mpStrobe2->isOn(&onLt);
    }

    return (on || onLt);
}

int FlashHal::isAFLampOn()
{
    if (!isAvailable()) {
        logD("isAFLampOn(): sensorDev(%d) not available.", mSensorDev);
        return 0;
    }

    int on = isFlashOn();
    logD("isAFLampOn(): (%d).", on);
    return on;
}

int FlashHal::setPreOn()
{
    int ret = 0;

    ret |= mpStrobe->setPreOn();
    if (mStrobeCtNum > 1)
        ret |= mpStrobe2->setPreOn();

    return ret;
}

int FlashHal::setFlashOn(FlashHalInfo info)
{
    Mutex::Autolock lock(mLock);

    logI("setFlashOn(): duty(%d), timeout(%d), lt duty(%d), lt timeout(%d).",
            info.duty, info.timeout, info.dutyLt, info.timeoutLt);

    int isLow = 0;
    isLowPower(&isLow);
    if (isLow) {
        logI("setFlashOn(): is low power.");
        return 0;
    }

    /* set duty */
    int dutyCur;
    int dutyCurLt;
    {
        mpStrobe->getDuty(&dutyCur);
        if (dutyCur != info.duty)
            mpStrobe->setDuty(info.duty);
    }
    if (mStrobeCtNum > 1) {
        mpStrobe2->getDuty(&dutyCurLt);
        if (dutyCurLt != info.dutyLt)
            mpStrobe2->setDuty(info.dutyLt);
    }

    /* set on/off */
    int bOn;
    {
        mpStrobe->isOn(&bOn);
        if (info.duty < 0) {
            mpStrobe->setOnOff(0);
        } else {
            mpStrobe->setTimeOutTime(info.timeout);
            mpStrobe->setOnOff(1);
        }
    }
    if (mStrobeCtNum > 1) {
        mpStrobe2->isOn(&bOn);
        if (info.dutyLt < 0) {
            mpStrobe2->setOnOff(0);
        } else {
            mpStrobe2->setTimeOutTime(info.timeoutLt);
            mpStrobe2->setOnOff(1);
        }
    }

    return 0;
}

int FlashHal::setFlashOff()
{
    Mutex::Autolock lock(mLock);

    logI("setFlashOff().");

    mpStrobe->setOnOff(0);
    if (mStrobeCtNum > 1)
        mpStrobe2->setOnOff(0);

    mpStrobe->getHwFault(&mDriverFault);
    mTorchLevel = -1;

    return 0;
}

int FlashHal::setOnOff(int enable, FlashHalInfo info)
{
    logI("setOnOff(): type(%d), enable(%d).", mStrobeTypeId, enable);

    if (!isAvailable()) {
        logI("setOnOff(): sensorDev(%d) not available.", mSensorDev);
        return -1;
    }

    if (enable)
        return setFlashOn(info);
    else
        return setFlashOff();
}

int FlashHal::setOnOff(int enable, FLASH_HAL_SCENARIO_ENUM scenario)
{
    logI("setOnOff(): type(%d), enable(%d), scenario(%d).", mStrobeTypeId, enable, scenario);

    /* verify arguments */
    if (verifyScenario(scenario)) {
        logE("setOnOff(): invalid arguments.");
        return -1;
    }

    if (!isAvailable()) {
        logI("setOnOff(): sensorDev(%d) not available.", mSensorDev);
        return -1;
    }

    if (scenario == FLASH_HAL_SCENARIO_MAIN_FLASH) {
        /* get timeout */
        int currentTime = getMs();
        if (enable) {
            mFlashHalTimeInfo.mfStartTime = currentTime;
            mFlashHalTimeInfo.mfIsTimeout = 0;
            mFlashHalTimeInfo.mfTimeout = mFlashHalInfo[FLASH_HAL_SCENARIO_MAIN_FLASH].timeout;
            if (mStrobeCtNum > 1)
                mFlashHalTimeInfo.mfTimeoutLt = mFlashHalInfo[FLASH_HAL_SCENARIO_MAIN_FLASH].timeoutLt;

        } else {
            mFlashHalTimeInfo.mfEndTime = currentTime;

            int flashOnPeriod = mFlashHalTimeInfo.mfEndTime - mFlashHalTimeInfo.mfStartTime;
            if (flashOnPeriod > mFlashHalTimeInfo.mfTimeout &&
                    mFlashHalTimeInfo.mfTimeout)
                mFlashHalTimeInfo.mfIsTimeout = 1;
            if (mStrobeCtNum > 1)
                if (flashOnPeriod > mFlashHalTimeInfo.mfTimeoutLt &&
                        mFlashHalTimeInfo.mfTimeoutLt)
                    mFlashHalTimeInfo.mfIsTimeout = 1;
        }
    } else if (scenario == FLASH_HAL_SCENARIO_VIDEO_TORCH) {
        if (mTorchLevel != -1) {
            FlashHalInfo torchInfo;
            int duty=0, dutyLt=0;

            FlashCustomAdapter::getInstance(mSensorDev)->getTorchDuty(mTorchLevel, &duty, &dutyLt);
            torchInfo.duty = duty;
            torchInfo.dutyLt = dutyLt;
            torchInfo.timeout = 0;
            torchInfo.timeoutLt = 0;
            return setFlashOn(torchInfo);
        }
    }

    if (enable)
        return setFlashOn(mFlashHalInfo[scenario]);
    else
        return setFlashOff();
}

int FlashHal::getTorchStatus()
{
    logI("getTorchStatus(): torch status(%d).", mTorchStatus);
    return mTorchStatus;
}

int FlashHal::setTorchOnOff(MBOOL enable)
{
    logI("setTorchOnOff(): type(%d), enable(%d).", mStrobeTypeId, enable);

    if (mStrobeTypeId == STROBE_TYPE_FRONT && !cust_isSubFlashSupport())
        return 1;

    /*
     * WARNING, only torch API is different from others.
     * The only control zoon for life cycle is in the APP site.
     * It should call init/uninit before using setTorchMode() API.
     *
     * So this is a workaround that life cycle is followed by torch on/off.
     */
    if (enable == 1) {
        init();
        setOnOff(enable, FLASH_HAL_SCENARIO_TORCH);
        mTorchStatus = 1;
    } else {
        setOnOff(enable, FLASH_HAL_SCENARIO_TORCH);
        uninit();
        mTorchStatus = 0;
    }

	return 0;
}

int FlashHal::setVideoTorchOnOff(int enable)
{
    logI("setVideoTorchOnOff(): enable(%d).", enable);
    setOnOff(enable, FLASH_HAL_SCENARIO_VIDEO_TORCH);
    return 0;
}

int FlashHal::setAfLampOnOff(int enable)
{
    logI("setAfLampOnOff(): enable(%d).", enable);
    setOnOff(enable, FLASH_HAL_SCENARIO_AF_LAMP);
    return 0;
}

int FlashHal::setPfOnOff(int enable)
{
    logI("setPfOnOff(): enable(%d).", enable);
    setOnOff(enable, FLASH_HAL_SCENARIO_PRE_FLASH);
    return 0;
}

int FlashHal::setCaptureFlashOnOff(int enable)
{
    logI("setCaptureFlashOnOff(): enable(%d).", enable);
    setOnOff(enable, FLASH_HAL_SCENARIO_MAIN_FLASH);
    return 0;
}

int FlashHal::setTorchDuty(int level)
{
    int duty = 0, dutyLt = 0;

    logI("setTorchLevel(): level(%d).", level);

    mTorchLevel = level;

    FlashCustomAdapter::getInstance(mSensorDev)->getTorchDuty(level, &duty, &dutyLt);
    mpStrobe->setDuty(duty);
    if (mStrobeCtNum > 1) {
        mpStrobe2->setDuty(dutyLt);
    }
    return 0;
}

int FlashHal::setSubFlashCustomization(MBOOL en)
{
    logI("setSubFlashCustomization(): enable(%d).", en);
    mSubFlashCustomization = en;
    return 0;
}

int FlashHal::getSubFlashCustomization()
{
    return mSubFlashCustomization;
}
/***********************************************************
 * Engineer mode related function
 **********************************************************/
int FlashHal::egGetDutyRange(int *start, int *end)
{
#if 1
    *start = 0;
    *end = 1;
#else
    *start = 0;
    *end = mPrjPara.dutyNum - 1;
#endif
    return 0;
}

int FlashHal::egGetStepRange(int *start, int *end)
{
    *start = 0;
    *end = 0;
    return 0;
}

int FlashHal::egSetMfDutyStep(int duty, int step)
{
    (void)duty;
    (void)step;
    return 0;
}

