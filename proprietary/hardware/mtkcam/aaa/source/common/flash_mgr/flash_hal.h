#pragma once

#include <utils/threads.h>
#include "mtkcam/aaa/IHalFlash.h"
#include "strobe_drv.h"
#include "flash_param.h"

using namespace android;
using namespace NSCam;

/** flash hal scenario */
typedef enum {
    FLASH_HAL_SCENARIO_TORCH,
    FLASH_HAL_SCENARIO_VIDEO_TORCH,
    FLASH_HAL_SCENARIO_AF_LAMP,
    FLASH_HAL_SCENARIO_PRE_FLASH,
    FLASH_HAL_SCENARIO_MAIN_FLASH,
    FLASH_HAL_SCENARIO_LOW_POWER,
    FLASH_HAL_SCENARIO_NUM,
} FLASH_HAL_SCENARIO_ENUM;

/** flash hal info */
typedef struct FlashHalInfo {
    int duty;
    int dutyLt;
    int timeout;
    int timeoutLt;
} FlashHalInfo;

/** flash hal time info */
typedef struct FlashHalTimeInfo {
    int mfStartTime;
    int mfEndTime;
    int mfTimeout;
    int mfTimeoutLt;
    int mfIsTimeout;
} FlashHalTimeInfo;

class FlashHal : public IHalFlash
{
    public:
        /***************************************
         * Available before init()
         **************************************/
        FlashHal(int sensorDev);
        ~FlashHal() {};

        static FlashHal *getInstance(MINT32 const i4SensorOpenIdx);
        void destroyInstance();

        int init();
        int uninit();

        int setInfo(FLASH_HAL_SCENARIO_ENUM scenario, FlashHalInfo info);
        int setInfoDuty(FLASH_HAL_SCENARIO_ENUM scenario, FlashHalInfo info);
        int getMaxDuty(int *duty, int *dutyLT);
        int getCurrentByDuty(int duty);
        int getCurrentTab(short *ITabHt, short *ITabLt);
        int getFlashMaxIDuty(int dutyNum, int dutyNumLt, int *duty, int *dutyLt);

        FlashHalTimeInfo getTimeInfo();
        int getDriverFault();

        int hasHw(int &hasHw);
        int getPartId();

        /**
         * @brief get/set in charge
         *
         * For Dual camera, 2 sensor mapping to 1 flashlight.
         * Needs to maintain the control right.
         */
        int getInCharge();
        int setInCharge(int inCharge);

        /**
         * @brief is instance avalible for operation
         * @return available status
         */
        int isAvailable();

        /***************************************
         * Available since init()
         **************************************/
        void show();
        int getBattVol(int *battVol);
        int isLowPower(int *battStatus);
        int isNeedWaitCooling(int curMs, int *ms);

        /**
         * @brief Set charger
         *
         * Some flashlight driver IC (RT5081)
         * need time to switch fast-charge mode to normal mode.
         */
        int isChargerReady(int *chargerStatus);
		int setCharger(int ready);

        /**
         * @brief Check if flash is on/off.
         */
        int isFlashOn();
        int isAFLampOn();

        /**
         * @brief Set pre-on
         *
         * Some old style flashlight driver IC (MT6332)
         * need time to burst large currnet.
         */
        int setPreOn();

        /**
         * @brief Turn on/off flash.
         */
        int setFlashOn(FlashHalInfo info);
        int setFlashOff();
        int setOnOff(int enable, FlashHalInfo info);
        int setOnOff(int enable, FLASH_HAL_SCENARIO_ENUM scenario);

        /**
         * @brief Turn on/off scenario flash.
         */
        int getTorchStatus();
        int setTorchOnOff(MBOOL en);
        int setVideoTorchOnOff(int enable);
        int setAfLampOnOff(int enable);
        int setPfOnOff(int enable);
        int setCaptureFlashOnOff(int enable);

        int setTorchDuty(int level);

        /**
         * @brief Set/Get to support customized panel flash
         */
        int setSubFlashCustomization(MBOOL en);
        int getSubFlashCustomization();

        /************************************************************
         * Engineer mode related function
         ***********************************************************/
        int egGetDutyRange(int *start, int *end);
        int egGetStepRange(int *start, int *end);
        int egSetMfDutyStep(int duty, int step);

    private:
        mutable Mutex mLock;

        int mSensorDev;
        int mTorchStatus;
        FlashHalTimeInfo mFlashHalTimeInfo;
        FlashHalInfo mFlashHalInfo[FLASH_HAL_SCENARIO_NUM];
        int mDriverFault;
        int mMaxDuty;
        int mMaxDutyLT;

        int mHasHw;
        int mInCharge;
        int mSubFlashCustomization;

        /** flash tuning parameters */
        FLASH_PROJECT_PARA mPrjPara;

        /** strobe handler */
        StrobeDrv *mpStrobe;
        StrobeDrv *mpStrobe2;
        STROBE_DEVICE_ENUM mStrobeDevice;
        STROBE_TYPE_ENUM mStrobeTypeId;
        int mStrobeCtNum;
        int mStrobePartId;
        int mTorchLevel;
};

