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

#define LOG_TAG "StrobeDrvFlashlight"

/***********************************************************
 * Headers
 **********************************************************/
#include <cutils/atomic.h>

/* custom headers */
//#include "flash_tuning_custom.h"

/* camera headers */
#include "mtkcam/def/BuiltinTypes.h"

/* aaa common headers */
#include "time_utils.h"
#include "interp_utils.h"
#include "math_utils.h"

/* strobe headers */
#include "strobe_drv_flashlight_fops.h"
#include "strobe_drv_flashlight.h"
#include "strobe_utils.h"

/* kernel headers */
#include "flashlight.h"


/***********************************************************
 * Macros
 **********************************************************/
#define BATTERY_STATUS_SYSFS_PATH "/sys/class/power_supply/battery/voltage_now"

#define MAX_NEED_PRE_ON_TIME 100

/***********************************************************
 * Member Functions
 **********************************************************/
StrobeDrvFlashlight::StrobeDrvFlashlight()
    : mUsers(0)
    , mSetCharger(0)
    , mTypeId(0)
    , mCtId(0)
    , mIsOn(0)
    , mDuty(-1)
    , mOnTime(-1)
    , mOffTime(-1)
    , mPreOnTime(-1)
    , mTimeOutTime(0)
    , mDutyNum(0)
    , mTabNum(0)
    , mLastQueryTime(-1)
    , mBattStatus(-1)
{
    logD("StrobeDrvFlashlight().");

    memset(mTabId, 0, sizeof(mTabId));
    memset(mTimeOutTab, 0, sizeof(mTimeOutTab));
    memset(mCoolTMTab, 0, sizeof(mCoolTMTab));
    memset(mCurrentTab, 0, sizeof(mCurrentTab));
}

StrobeDrvFlashlight::~StrobeDrvFlashlight()
{
    logD("~StrobeDrvFlashlight().");
}

StrobeDrvFlashlight *StrobeDrvFlashlight::getInstance(int typeId, int ctId)
{
	static StrobeDrvFlashlight singleton[FLASHLIGHT_TYPE_MAX][FLASHLIGHT_CT_MAX];

    /* verify type and ct */
    if (verifyTypeCt(typeId, ctId)) {
        logE("getInstance(): invalid type(%d), ct(%d).", typeId, ctId);
        return NULL;
    }

    /* get instance */
    logD("getInstance(): type(%d), ct(%d).", typeId, ctId);
	singleton[typeId - 1][ctId - 1].mTypeId = typeId;
	singleton[typeId - 1][ctId - 1].mCtId = ctId;

    return &singleton[typeId - 1][ctId - 1];
}

int StrobeDrvFlashlight::getPartId(int *partId)
{
    StrobeDrvFlashlightFops *pStrobeDrvFlashlightFops = StrobeDrvFlashlightFops::getInstance();
    if (pStrobeDrvFlashlightFops->openkd()) {
        logI("getPartId(): failed to open.");
        return -1;
    }
    if (pStrobeDrvFlashlightFops->sendCommandRet(
                FLASH_IOC_GET_PART_ID, mTypeId, mCtId, partId)) {
        logI("getPartId(): failed to ioctl.");
        pStrobeDrvFlashlightFops->closekd();
        return -1;
    }
    pStrobeDrvFlashlightFops->closekd();

    logI("getPartId(): partId(%d).", *partId);

    return 0;
}

int StrobeDrvFlashlight::hasFlashHw(int *hasHw)
{
    StrobeDrvFlashlightFops *pStrobeDrvFlashlightFops = StrobeDrvFlashlightFops::getInstance();
    if (pStrobeDrvFlashlightFops->openkd()) {
        logI("hasFlashHw(): failed to open.");
        return -1;
    }
    if (pStrobeDrvFlashlightFops->sendCommandRet(
                FLASH_IOC_IS_HARDWARE_READY, mTypeId, mCtId, hasHw)) {
        logI("hasFlashHw(): failed to ioctl.");
        pStrobeDrvFlashlightFops->closekd();
        return -1;
    }
    pStrobeDrvFlashlightFops->closekd();

    logI("hasFlashHw(): hasHw(%d).", *hasHw);

    return 0;
}

int StrobeDrvFlashlight::setStrobeInfo(
        int dutyNum, int tabNum, int *tabId, int *timeOutMs, float *coolingTM)
{
    int i;

    logI("setStrobeInfo(): table number(%d), duty number(%d).", tabNum, dutyNum);

    mDutyNum = dutyNum;
    mTabNum = tabNum;

    for (i = 0; i < tabNum; i++) {
        logI("setStrobeInfo(): table id(%d), timeout (%d ms), cooling time multiple(%.2f).",
                tabId[i], timeOutMs[i], coolingTM[i]);
        mTabId[i] = tabId[i];
        mTimeOutTab[i] = timeOutMs[i];
        mCoolTMTab[i] = coolingTM[i];
    }

    return 0;
}

int StrobeDrvFlashlight::init()
{
    Mutex::Autolock lock(mLock);

    if (!mUsers) {
        int maxDuty;
        /* init procedure */
        setDebug();

        StrobeDrvFlashlightFops *pStrobeDrvFlashlightFops = StrobeDrvFlashlightFops::getInstance();
        if (pStrobeDrvFlashlightFops->openkd()) {
            logI("init(): failed to open.");
            return -1;
        }
        if (pStrobeDrvFlashlightFops->sendCommand(FLASHLIGHTIOC_X_SET_DRIVER, mTypeId, mCtId, 1)) {
            logI("init(): failed to set driver.");
            pStrobeDrvFlashlightFops->closekd();
            return -1;
        }

        getMaxDuty(&maxDuty);
        logI("maxDuty:%d", maxDuty);

        for (int i = 0; i < maxDuty; i++) {
            getCurrentByDuty(i, &mCurrentTab[i]);
            logD("i:%d current:%d", i, mCurrentTab[i]);
        }

        /* clear member variables */
        mSetCharger = 0;

        mIsOn = 0;
        mDuty = -1;

        mOnTime = -1;
        mOffTime = -1;
        mPreOnTime = -1;
        mTimeOutTime = 0;
    }
    android_atomic_inc(&mUsers);

    logI("init(): mUsers(%d).", mUsers);

    return 0;
}

int StrobeDrvFlashlight::uninit()
{
    Mutex::Autolock lock(mLock);

    if (mUsers <= 0) {
        logI("uninit(): error mUsers(%d).", mUsers);
        return -1;
    }

    if (mUsers == 1) {
        /* uninit procedure */
        StrobeDrvFlashlightFops *pStrobeDrvFlashlightFops = StrobeDrvFlashlightFops::getInstance();
        pStrobeDrvFlashlightFops->sendCommand(FLASHLIGHTIOC_X_SET_DRIVER, mTypeId, mCtId, 0);
        pStrobeDrvFlashlightFops->closekd();
    }
    android_atomic_dec(&mUsers);

    logI("uninit(): mUsers(%d).", mUsers);

    return 0;
}

int StrobeDrvFlashlight::setDuty(int duty)
{
    logI("setDuty(): duty(%d).", duty);
    mDuty = duty;
    StrobeDrvFlashlightFops *pStrobeDrvFlashlightFops = StrobeDrvFlashlightFops::getInstance();
    return pStrobeDrvFlashlightFops->sendCommand(FLASH_IOC_SET_DUTY, mTypeId, mCtId, duty);
}

int StrobeDrvFlashlight::getDuty(int *duty)
{
    logD("getDuty(): duty(%d).", mDuty);
    *duty = mDuty;
    return 0;
}

int StrobeDrvFlashlight::getMaxDuty(int *duty)
{
    int ret;

    logD("getMaxDuty(): +");
    StrobeDrvFlashlightFops *pStrobeDrvFlashlightFops = StrobeDrvFlashlightFops::getInstance();
    ret = pStrobeDrvFlashlightFops->sendCommandRet(FLASH_IOC_GET_DUTY_NUMBER, mTypeId, mCtId, duty);
    if (ret != 0){
        *duty = 0;
        logE("getMaxDuty(): failed.");
    }
    logD("getMaxDuty(): duty(%d).", *duty);

    return ret;
}

int StrobeDrvFlashlight::getCurrentByDuty(int duty, int *current)
{
    int ret;

    logD("getCurrentByDuty(): +");
    StrobeDrvFlashlightFops *pStrobeDrvFlashlightFops = StrobeDrvFlashlightFops::getInstance();
    ret = pStrobeDrvFlashlightFops->sendCommandRet(FLASH_IOC_GET_DUTY_CURRENT, mTypeId, mCtId, &duty);
    if (ret != 0){
        *current = -1;
        logE("getCurrentByDuty(): failed");
    } else {
        *current = duty;
    }
    logD("getCurrentByDuty(): current(%d).", *current);

    return ret;
}

int StrobeDrvFlashlight::getCurrentTable(short *ITab)
{
    int maxDuty = 0;

    logD("getCurrentTable(): +");
    getMaxDuty(&maxDuty);
    for (int i = 0; i < maxDuty; i++)
        ITab[i] = mCurrentTab[i];
    return 0;
}

/* for boosting voltage and current */
int StrobeDrvFlashlight::setPreOn()
{
    logD("setPreOn().");
    mPreOnTime = getMs();
    StrobeDrvFlashlightFops *pStrobeDrvFlashlightFops = StrobeDrvFlashlightFops::getInstance();
    return pStrobeDrvFlashlightFops->sendCommand(FLASH_IOC_PRE_ON, mTypeId, mCtId, 0);
}

int StrobeDrvFlashlight::getPreOnTimeMs(int *ms)
{
    int ret;

    logD("getPreOnTimeMs(): +");
    StrobeDrvFlashlightFops *pStrobeDrvFlashlightFops = StrobeDrvFlashlightFops::getInstance();
    ret = pStrobeDrvFlashlightFops->sendCommandRet(FLASH_IOC_GET_PRE_ON_TIME_MS, mTypeId, mCtId, ms);
    logD("getPreOnTimeMs(): ms(%d).", *ms);

    return ret;
}

int StrobeDrvFlashlight::getPreOnTimeMsDuty(int duty, int *ms)
{
    int ret = 0;

    logD("getPreOnTimeMsDuty(): +.");
    StrobeDrvFlashlightFops *pStrobeDrvFlashlightFops = StrobeDrvFlashlightFops::getInstance();
    ret |= pStrobeDrvFlashlightFops->sendCommand(FLASH_IOC_GET_PRE_ON_TIME_MS_DUTY, mTypeId, mCtId, duty);
    ret |= pStrobeDrvFlashlightFops->sendCommandRet(FLASH_IOC_GET_PRE_ON_TIME_MS, mTypeId, mCtId, ms);
    logD("getPreOnTimeMsDuty(): duty(%d), ms(%d).", duty, *ms);

    return ret;
}

int StrobeDrvFlashlight::getBattVol(int *battVol)
{
    int ret;
    int value;
    FILE *fp;

    /* defined in [KERNEL]/drivers/power/power_supply_sysfs.c */
    fp = fopen(BATTERY_STATUS_SYSFS_PATH, "rb");
    if (!fp) {
        logE("getBattVol(): failed to open(%s).", BATTERY_STATUS_SYSFS_PATH);
        return -1;
    }

    ret = fscanf(fp, "%d", &value);
    if (ret != 1) {
        logE("getBattVol(): failed to read battery voltage.");
        fclose(fp);
        return -1;
    }

    *battVol = value/1000;
    fclose(fp);

    logI("getBattVol(): battVol(%d).", *battVol);

    return 0;
}

int StrobeDrvFlashlight::isLowPower(int *battStatus)
{
    int ret = 0;
    int currMs = getMs();

    logD("isLowPower(): +.");
    /* 10s update */
    if ( mLastQueryTime == -1 || (currMs - mLastQueryTime > 10000) ) {
        StrobeDrvFlashlightFops *pStrobeDrvFlashlightFops = StrobeDrvFlashlightFops::getInstance();
        ret = pStrobeDrvFlashlightFops->sendCommandRet(
            FLASH_IOC_IS_LOW_POWER, mTypeId, mCtId, battStatus);
        logD("isLowPower(): battStatus(%d).", *battStatus);
        if ( ret == 0 ) {
            mLastQueryTime = currMs;
            mBattStatus = *battStatus;
        }
    } else {
        *battStatus = mBattStatus;
    }

    return ret;
}

int StrobeDrvFlashlight::lowPowerDetectStart(int lowPowerDuty)
{
    logI("lowPowerDetectStart(): duty(%d).", lowPowerDuty);
    StrobeDrvFlashlightFops *pStrobeDrvFlashlightFops = StrobeDrvFlashlightFops::getInstance();
    return pStrobeDrvFlashlightFops->sendCommand(
            FLASH_IOC_LOW_POWER_DETECT_START, mTypeId, mCtId, lowPowerDuty);
}

int StrobeDrvFlashlight::lowPowerDetectEnd()
{
    logI("lowPowerDetectEnd().");
    StrobeDrvFlashlightFops *pStrobeDrvFlashlightFops = StrobeDrvFlashlightFops::getInstance();
    return pStrobeDrvFlashlightFops->sendCommand(
            FLASH_IOC_LOW_POWER_DETECT_END, mTypeId, mCtId, 0);
}

int StrobeDrvFlashlight::isChargerReady(int *chargerStatus)
{
    int ret;

    StrobeDrvFlashlightFops *pStrobeDrvFlashlightFops = StrobeDrvFlashlightFops::getInstance();
    ret = pStrobeDrvFlashlightFops->sendCommandRet(
            FLASH_IOC_IS_CHARGER_READY, mTypeId, mCtId, chargerStatus);
    logI("isChargerReady(): charger status(%d).", *chargerStatus);

    return ret;
}

int StrobeDrvFlashlight::setCharger(int ready)
{
    Mutex::Autolock lock(mLock);

    logI("setCharger(): ready(%d).", ready);

    int scenario;
    if (ready) {
        scenario = FLASHLIGHT_SCENARIO_CAMERA;
        android_atomic_inc(&mSetCharger);
    } else {
        scenario = FLASHLIGHT_SCENARIO_FLASHLIGHT;
        android_atomic_dec(&mSetCharger);
    }

    int ret = 0;
    if ((ready && mSetCharger == 1) ||
            (!ready && mSetCharger == 0)) {
        StrobeDrvFlashlightFops *pStrobeDrvFlashlightFops = StrobeDrvFlashlightFops::getInstance();
        ret = pStrobeDrvFlashlightFops->sendCommand(FLASH_IOC_SET_SCENARIO, mTypeId, mCtId, scenario);
        logI("setCharger(): time(%ld).", getMs());
    }

    return ret;
}

int StrobeDrvFlashlight::isOn(int *isOn)
{
    *isOn = mIsOn;
    return 0;
}

int StrobeDrvFlashlight::setOnOff(int enable)
{
    int ret = 0;

    /* verify arguments */
    if (verifyTypeCt(mTypeId, mCtId)) {
        logE("setOnOff(): error type/ct(%d,%d).", mTypeId, mCtId);
        return -1;
    }
    if (enable != MTRUE && enable != MFALSE) {
        logE("setOnOff(): error enable(%d).", enable);
        return -1;
    }

    logI("setOnOff(): enable(%d).", enable);

    /* get strobe fops handler */
    StrobeDrvFlashlightFops *pStrobeDrvFlashlightFops = StrobeDrvFlashlightFops::getInstance();

    if (enable == MTRUE) {
        /* record time at turning on */
        if (mIsOn == MFALSE)
            mOnTime = getMs();

        /* get pre-on time from driver */
        int needPreOnTime = 0;
        getPreOnTimeMsDuty(mDuty, &needPreOnTime);
        needPreOnTime = min(needPreOnTime, MAX_NEED_PRE_ON_TIME);

        /* sleep for pre-on */
        if (needPreOnTime > 0) {
            if (mPreOnTime == -1) {
                setPreOn();
                logI("setOnOff(): needs pre-on time(%d ms).", needPreOnTime);
                sleepMs(needPreOnTime);
            } else {
                long int curTime = getMs();
                int sleepTimeMs = needPreOnTime - (curTime - mPreOnTime);
                sleepTimeMs = min(sleepTimeMs, needPreOnTime);
                if (sleepTimeMs > 0) {
                    logI("setOnOff(): still needs pre-on time(%d ms).", sleepTimeMs);
                    sleepMs(sleepTimeMs);
                }
            }
        }

        /* enable strobe */
        ret = pStrobeDrvFlashlightFops->sendCommand(FLASH_IOC_SET_ONOFF, mTypeId, mCtId, MTRUE);
        logI("setOnOff(): on time(%ld).", mOnTime);

    } else if (enable == MFALSE) {
        if (mIsOn == MTRUE) {
            mOffTime = getMs();
            if (mTimeOutTime && (mOffTime - mOnTime > mTimeOutTime))
                logI("setOnOff(): timeout.");
        }

        /* clear set pre-on time */
        mPreOnTime = -1;

        /* disable strobe */
        ret = pStrobeDrvFlashlightFops->sendCommand(FLASH_IOC_SET_ONOFF, mTypeId, mCtId, MFALSE);
        logI("setOnOff(): off time(%ld).", mOffTime);
    }

    if (!ret)
        mIsOn = enable;
    else
        logE("setOnOff(): failed to ioctl.");

    return ret;
}

int StrobeDrvFlashlight::setTimeOutTime(int ms)
{
    logI("setTimeOutTime(): (%d ms).", ms);

    mTimeOutTime = ms;
    StrobeDrvFlashlightFops *pStrobeDrvFlashlightFops = StrobeDrvFlashlightFops::getInstance();
    return pStrobeDrvFlashlightFops->sendCommand(FLASH_IOC_SET_TIME_OUT_TIME_MS, mTypeId, mCtId, ms);
}

int StrobeDrvFlashlight::getTimeOutTime(int duty, int *timeOut)
{
    if (duty < 0)
        *timeOut = 0;
    else
        *timeOut = interpTable(mTabNum, mTabId, mTimeOutTab, duty);

    logI("getTimeOutTime(): timeout(%d).", *timeOut);

    return 0;
}

int StrobeDrvFlashlight::getCoolTM(int duty, float *coolTM)
{
    if (duty < 0)
        *coolTM = 0;
    else
        *coolTM = interpTable(mTabNum, mTabId, mCoolTMTab, duty);

    logI("getCoolTM(): cooling time multiple(%f).", *coolTM);

    return 0;
}

int StrobeDrvFlashlight::getHwFault(int *fault)
{
    int flag1=0, flag2=0, ret=0;
    StrobeDrvFlashlightFops *pStrobeDrvFlashlightFops = StrobeDrvFlashlightFops::getInstance();

    ret |= pStrobeDrvFlashlightFops->sendCommandRet(FLASH_IOC_GET_HW_FAULT, mTypeId, mCtId, &flag1);
    logI("getHwFault(): flag1(%d) ret(%d).", flag1, ret);
    if (ret == -1)
        flag1 = 0;
    ret |= pStrobeDrvFlashlightFops->sendCommandRet(FLASH_IOC_GET_HW_FAULT2, mTypeId, mCtId, &flag2);
    logI("getHwFault(): flag2(%d) ret(%d).", flag2, ret);
    if (ret == -1)
        flag2 = 0;

    *fault = (flag1 & 0xFF) |  ( (flag2 & 0xFF) << 8);
    if(*fault != 0)
        logI("getHwFault(): fault(%d).", *fault);

    return 0;
}

