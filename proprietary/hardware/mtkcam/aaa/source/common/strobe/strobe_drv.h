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

#include "strobe_param.h"

/* strobe driver */
class StrobeDrv
{
    protected:
        virtual ~StrobeDrv() = 0;

    public:
        /***************************************
         * Available before init()
         **************************************/
        static StrobeDrv *getInstance(STROBE_DEVICE_ENUM device, int typeId, int ctId);

        /** part */
        virtual int getPartId(int *partId) = 0;

        /** has hw or not */
        virtual int hasFlashHw(int *hasHw) = 0;

        /** flash project parameter */
        virtual int setStrobeInfo(int dutyNum, int tabNum, int *tabId, int *timeOutMs, float *coolingTM) = 0;

        /** init/uninit */
        virtual int init() = 0;
        virtual int uninit() = 0;

        /***************************************
         * Available since init()
         **************************************/
        /** duty */
        virtual int setDuty(int duty) = 0;
        virtual int getDuty(int *duty) = 0;
        virtual int getMaxDuty(int *duty) = 0;
        virtual int getCurrentByDuty(int duty, int *current) = 0;
        virtual int getCurrentTable(short *ITab) = 0;

        /** pre-enable */
        virtual int setPreOn() = 0;
        virtual int getPreOnTimeMs(int *ms) = 0;
        virtual int getPreOnTimeMsDuty(int duty, int *ms) = 0;

        /** battery */
        virtual int getBattVol(int *battVol) = 0;
        virtual int isLowPower(int *battStatus) = 0;
        virtual int lowPowerDetectStart(int lowPowerDuty) = 0;
        virtual int lowPowerDetectEnd() = 0;

        /** charger */
        virtual int isChargerReady(int *chargerStatus) = 0;
        virtual int setCharger(int ready) = 0;

        /** enable/disable */
        virtual int isOn(int *isOn) = 0;
        virtual int setOnOff(int isOn) = 0;

        /** time */
        virtual int setTimeOutTime(int ms) = 0;
        virtual int getTimeOutTime(int duty, int *timeOut) = 0;
        virtual int getCoolTM(int duty, float *coolTM) = 0;

        /** fault */
        virtual int getHwFault(int *fault) = 0;
};

