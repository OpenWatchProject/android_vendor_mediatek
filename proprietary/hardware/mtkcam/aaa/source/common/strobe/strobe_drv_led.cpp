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

#define LOG_TAG "StrobeDrvLed"

/***********************************************************
 * Headers
 **********************************************************/
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
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
#include "strobe_drv_led.h"
#include "strobe_utils.h"


/***********************************************************
 * Macros
 **********************************************************/
#define STROBE_SYSFS_FLASH_BRIGHTNESS     "/sys/class/leds/rt5081/flash_brightness" // TODO: replace driver IC
#define STROBE_SYSFS_MAX_FLASH_BRIGHTNESS "/sys/class/leds/rt5081/max_flash_brightness"
#define STROBE_SYSFS_FLASH_TIMEOUT        "/sys/class/leds/rt5081/flash_timeout"
#define STROBE_SYSFS_MAX_FLASH_TIMEOUT    "/sys/class/leds/rt5081/max_flash_timeout"
#define STROBE_SYSFS_FLASH_STROBE         "/sys/class/leds/rt5081/flash_strobe"
#define STROBE_SYSFS_FLASH_FAULT          "/sys/class/leds/rt5081/flash_fault"

#define BATTERY_STATUS_SYSFS_PATH "/sys/class/power_supply/battery/voltage_now"

/***********************************************************
 * Member Functions
 **********************************************************/
StrobeDrvLed::StrobeDrvLed()
    : mStrobeHandle(-1)
    , mUsers(0)
    , mSetCharger(0)
    , mTypeId(0)
    , mCtId(0)
    , mIsOn(0)
    , mDuty(-1)
    , mOnTime(-1)
    , mOffTime(-1)
    , mTimeOutTime(0)
    , mDutyNum(0)
    , mTabNum(0)
{
    logD("StrobeDrvLed().");

    memset(mTabId, 0, sizeof(mTabId));
    memset(mTimeOutTab, 0, sizeof(mTimeOutTab));
    memset(mCoolTMTab, 0, sizeof(mCoolTMTab));
}

StrobeDrvLed::~StrobeDrvLed()
{
    logD("~StrobeDrvLed().");
}

StrobeDrvLed *StrobeDrvLed::getInstance(int typeId, int ctId)
{
    StrobeDrvLed *instance = NULL;

    /* verify type and ct */
    if (verifyTypeCt(typeId, ctId)) {
        logE("getInstance(): error type/ct(%d,%d).", typeId, ctId);
        return instance;
    }

    logD("getInstance(): type(%d), ct(%d).", typeId, ctId);

    /* get instance */
    if (typeId == 1) {
        if (ctId == 1) {
            static StrobeDrvLed singleton1;
            instance = &singleton1;
        } else if (ctId == 2) {
            static StrobeDrvLed singleton2;
            instance = &singleton2;
        }
    } else if (typeId == 2) {
        if (ctId == 1) {
            static StrobeDrvLed singleton3;
            instance = &singleton3;
        } else if (ctId == 2) {
            static StrobeDrvLed singleton4;
            instance = &singleton4;
        }
    }

    if (instance != NULL){
        instance->mTypeId = typeId;
        instance->mCtId = ctId;
    }

    return instance;
}

int StrobeDrvLed::getPartId(int *partId)
{
    *partId = 1; // TODO: not support part
    logI("getPartId(): partId(%d).", *partId);
    return 0;
}

int StrobeDrvLed::hasFlashHw(int *hasHw)
{
    struct stat strobeStat;
    if (stat(STROBE_SYSFS_FLASH_BRIGHTNESS, &strobeStat))
        *hasHw = 0;
    else
        *hasHw = 1;

    logI("hasFlashHw(): hasHw(%d).", *hasHw);

    return 0;
}

int StrobeDrvLed::setStrobeInfo(
        int dutyNum, int tabNum, int *tabId, int *timeOutMs, float *coolingTM)
{
    int i;

    logI("setStrobeInfo(): table number(%d), duty number(%d).", tabNum, dutyNum);

    mDutyNum = dutyNum;
    mTabNum = tabNum;

    for (i = 0; i < tabNum; i++) {
        logI("setStrobeInfo(): table id(%d), timeout (%d ms), cooling time multiple(%f).",
                tabId[i], timeOutMs[i], coolingTM[i]);
        mTabId[i] = tabId[i];
        mTimeOutTab[i] = timeOutMs[i];
        mCoolTMTab[i] = coolingTM[i];
    }

    return 0;
}

int StrobeDrvLed::init()
{
    Mutex::Autolock lock(mLock);

    if (!mUsers) {
        /* init procedure */
        setDebug();

        mStrobeHandle = -1;

        /* clear member variables */
        mSetCharger = 0;

        mIsOn = 0;
        mDuty = -1;

        mOnTime = -1;
        mOffTime = -1;
        mTimeOutTime = 0;
    }
    android_atomic_inc(&mUsers);

    logI("init(): mUsers(%d).", mUsers);

    return 0;
}

int StrobeDrvLed::uninit()
{
    Mutex::Autolock lock(mLock);

    if (mUsers <= 0) {
        logI("uninit(): error mUsers(%d).", mUsers);
        return -1;
    }
    android_atomic_dec(&mUsers);

    logI("uninit(): mUsers(%d).", mUsers);

    return 0;
}

int StrobeDrvLed::setDuty(int duty)
{
    int ret;
    char brightness[4];

    // TODO: check max brightness level

    logI("setDuty(): duty(%d).", duty);

    mDuty = duty;
    snprintf(brightness, sizeof(brightness), "%d", duty);

    mStrobeHandle = open(STROBE_SYSFS_FLASH_BRIGHTNESS, O_RDWR);
    if (mStrobeHandle < 0) {
        logE("setDuty(): failed to open device file(%s) error(%s).",
                STROBE_SYSFS_FLASH_BRIGHTNESS, strerror(errno));
        return -1;
    }
    ret = write(mStrobeHandle, brightness, sizeof(brightness));
    close(mStrobeHandle);

    return ret;
}

int StrobeDrvLed::getDuty(int *duty)
{
    logD("getDuty(): duty(%d).", mDuty);
    *duty = mDuty;
    return 0;
}

int StrobeDrvLed::getMaxDuty(int *duty)
{
    (void)duty;
    logD("getMaxDuty().");
    return 0;
}

int StrobeDrvLed::getCurrentByDuty(int duty, int *current)
{
    (void)duty;
    (void)current;
    logD("getCurrentByDuty().");
    return 0;
}

int StrobeDrvLed::getCurrentTable(short *ITab)
{
    (void)ITab;
    logD("getCurrentTable().");
    return 0;
}

int StrobeDrvLed::setPreOn()
{
    logD("setPreOn().");
    return 0;
}

int StrobeDrvLed::getPreOnTimeMs(int *ms)
{
    (void)ms;
    logD("getPreOnTimeMs().");
    return 0;
}

int StrobeDrvLed::getPreOnTimeMsDuty(int duty, int *ms)
{
    (void)duty;
    (void)ms;
    logD("getPreOnTimeMsDuty().");
    return 0;
}

int StrobeDrvLed::getBattVol(int *battVol)
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

int StrobeDrvLed::isLowPower(int *battStatus)
{
    int ret = 0;

    // TODO: use pmic driver interface
    *battStatus = 0;
    logD("isLowPower(): battery status(%d).", *battStatus);

    return ret;
}

int StrobeDrvLed::lowPowerDetectStart(int lowPowerDuty)
{
    logI("lowPowerDetectStart(): duty(%d).", lowPowerDuty);
    return 0;
}

int StrobeDrvLed::lowPowerDetectEnd()
{
    logI("lowPowerDetectEnd().");
    return 0;
}

int StrobeDrvLed::isChargerReady(int *chargerStatus)
{
    int ret = 0;

    // TODO: use pmic driver interface
    *chargerStatus = 1;
    logI("isChargerReady(): charger status(%d).", *chargerStatus);

    return ret;
}

int StrobeDrvLed::setCharger(int ready)
{
    Mutex::Autolock lock(mLock);

    logI("setCharger(): ready(%d).", ready);

    if (ready) {
        android_atomic_inc(&mSetCharger);
    } else {
        android_atomic_dec(&mSetCharger);
    }

    int ret = 0;
    if (ready && mSetCharger == 1) {
        // TODO: use pmic driver interface
        ;
    } else if (!ready && mSetCharger == 0) {
        // TODO: use pmic driver interface
        ;
    }
    logI("setCharger(): time(%ld).", getMs());

    return ret;
}

int StrobeDrvLed::isOn(int *isOn)
{
    *isOn = mIsOn;
    return 0;
}

int StrobeDrvLed::setOnOff(int enable)
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

    if (enable == MTRUE) {
        /* record time at turning on */
        if (mIsOn == MFALSE)
            mOnTime = getMs();

        /* enable strobe */
        mStrobeHandle = open(STROBE_SYSFS_FLASH_STROBE, O_RDWR);
        if (mStrobeHandle < 0) {
            logE("setOnOff(): failed to open device file(%s) error(%s).",
                    STROBE_SYSFS_FLASH_STROBE, strerror(errno));
            return -1;
        }
        ret = write(mStrobeHandle, "1", 1);
        close(mStrobeHandle);
        logI("setOnOff(): on time(%ld).", mOnTime);

    } else if (enable == MFALSE) {
        if (mIsOn == MTRUE) {
            mOffTime = getMs();
            if (mTimeOutTime && (mOffTime - mOnTime > mTimeOutTime))
                logI("setOnOff(): timeout.");
        }

        /* disable strobe */
        mStrobeHandle = open(STROBE_SYSFS_FLASH_STROBE, O_RDWR);
        if (mStrobeHandle < 0) {
            logE("setOnOff(): failed to open device file(%s) error(%s).",
                    STROBE_SYSFS_FLASH_STROBE, strerror(errno));
            return -1;
        }
        ret = write(mStrobeHandle, "0", 1);
        close(mStrobeHandle);
        logI("setOnOff(): off time(%ld).", mOffTime);
    }

    if (!ret)
        mIsOn = enable;
    else
        logE("setOnOff(): failed to ioctl.");

    return ret;
}

int StrobeDrvLed::setTimeOutTime(int ms)
{
    int ret;
    char timeout[8];

    // TODO: check max timeout

    mTimeOutTime = ms;
    snprintf(timeout, sizeof(timeout), "%d", ms);

    mStrobeHandle = open(STROBE_SYSFS_FLASH_TIMEOUT, O_RDWR);
    if (mStrobeHandle < 0) {
        logE("setTimeOutTime(): failed to open device file(%s) error(%s).",
                STROBE_SYSFS_FLASH_TIMEOUT, strerror(errno));
        return -1;
    }
    ret = write(mStrobeHandle, timeout, sizeof(timeout));
    close(mStrobeHandle);

    logI("setTimeOutTime(): (%d ms).", ms);

    return ret;
}

int StrobeDrvLed::getTimeOutTime(int duty, int *timeOut)
{
    if (duty < 0)
        *timeOut = 0;
    else
        *timeOut = interpTable(mTabNum, mTabId, mTimeOutTab, duty);

    logI("getTimeOutTime(): timeout(%d).", *timeOut);

    return 0;
}

int StrobeDrvLed::getCoolTM(int duty, float *coolTM)
{
    if (duty < 0)
        *coolTM = 0;
    else
        *coolTM = interpTable(mTabNum, mTabId, mCoolTMTab, duty);

    logI("getCoolTM(): cooling time multiple(%f).", *coolTM);

    return 0;
}

int StrobeDrvLed::getHwFault(int *fault __unused)
{
    *fault = 0;

    return 0;
}
