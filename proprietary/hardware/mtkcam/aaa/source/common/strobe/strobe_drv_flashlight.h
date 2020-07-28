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

#pragma once

#include <utils/threads.h>

/* kernel headers */
#include "flashlight.h"

#include "strobe_drv.h"

using namespace android;

class StrobeDrvFlashlight : public StrobeDrv
{
    private:
        mutable Mutex mLock;
        int mUsers;            /** init count */
        int mSetCharger;       /** set charger count */

        int mTypeId;
        int mCtId;

        int mIsOn;             /** current status: on, off */
        int mDuty;             /** current duty */

        long int mOnTime;      /** time at turning on */
        long int mOffTime;     /** time at turning off */
        long int mPreOnTime;   /** time at pre-on */
        long int mTimeOutTime; /** current timeout */

        /** flash project parameters from custom */
        int mDutyNum;          /** duty number not inclued off status */
        int mTabNum;
        int mTabId[10];
        int mTimeOutTab[10];
        float mCoolTMTab[10];
        int mCurrentTab[40];

        int mLastQueryTime;
        int mBattStatus;

    protected:
        StrobeDrvFlashlight();
        virtual ~StrobeDrvFlashlight();

    public:
        /***************************************
         * Available before init()
         **************************************/
        static StrobeDrvFlashlight *getInstance(int typeId, int ctId);

        /** part */
        virtual int getPartId(int *partId);

        /** has hw or not */
        virtual int hasFlashHw(int *hasHw);

        /** flash project parameter */
        virtual int setStrobeInfo(int dutyNum, int tabNum, int *tabId, int *timeOutMs, float *coolingTM);

        /** init/uninit */
        virtual int init();
        virtual int uninit();

        /***************************************
         * Available since init()
         **************************************/
        /** duty */
        virtual int setDuty(int duty);
        virtual int getDuty(int *duty);
        virtual int getMaxDuty(int *duty);
        virtual int getCurrentByDuty(int duty, int *current);
        virtual int getCurrentTable(short *ITab);

        /** pre-enable */
        virtual int setPreOn();
        virtual int getPreOnTimeMs(int *ms);
        virtual int getPreOnTimeMsDuty(int duty, int *ms);

        /** battery */
        virtual int getBattVol(int *battVol);
        virtual int isLowPower(int *battStatus);
        virtual int lowPowerDetectStart(int lowPowerDuty);
        virtual int lowPowerDetectEnd();

        /** charger */
        virtual int isChargerReady(int *chargerStatus);
        virtual int setCharger(int ready);

        /** enable/disable */
        virtual int isOn(int *isOn);
        virtual int setOnOff(int isOn);

        /** time */
        virtual int setTimeOutTime(int ms);
        virtual int getTimeOutTime(int duty, int *timeOut);
        virtual int getCoolTM(int duty, float *coolTM);

        /** fault */
        virtual int getHwFault(int *fault);

        /** check type and ct */
        static int verifyTypeCt(int typeId, int ctId)
        {
            if (typeId < 1 || typeId > FLASHLIGHT_TYPE_MAX)
                return -1;
            if (ctId < 1 || ctId > FLASHLIGHT_CT_MAX)
                return -1;
            return 0;
        }
};

