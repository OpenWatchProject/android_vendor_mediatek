/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

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
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
/*
** $Log: lens_drv.h $
 *
*/

#ifndef _LASER_MGR_H_
#define _LASER_MGR_H_

#include <utils/Mutex.h>
#include <semaphore.h>

typedef enum
{
    STATUS_RANGING_VALID             = 0x0,  // reference laser ranging distance
    STATUS_MOVE_DMAX                 = 0x1,  // Search range [DMAX  : infinity]
    STATUS_MOVE_MAX_RANGING_DIST     = 0x2,  // Search range [xx cm : infinity], according to the laser max ranging distance
    STATUS_NOT_REFERENCE             = 0x3
} LASER_STATUS_T;

class LaserMgr
{
private:
    MUINT32 m_u4SensorDev;

    MINT32  m_i4TransformRule;
    MINT32  m_i4AfMarcoDAC;
    MINT32  m_i4AfInfDAC;
    MINT32  m_i4LaserMaxDAC;
    MINT32  m_i4AfMarcoDistance;
    MINT32  m_i4AfInfDistance;

    MINT32  m_i4LaserCurPos_DAC;
    MINT32  m_i4LaserCurPos_RV;
    MINT32  m_i4LaserCurPos_Status;
    MINT32  m_i4LaserCurPos_DMAX;
    MINT32  m_i4LaserMaxPos_DAC;
    MINT32  m_i4LaserMaxPos_RV;

    MINT32  m_aLensDistDac[32];
    MINT32  m_aLaserDistRV[32];
    MINT32  m_i4LaserGoldenTableNum;
    MINT32  m_i4LaserMaxDistance;
    MINT32  m_i4LaserMaxDistRatio; // use the ratio to calculate LaserMaxPos_DAC if no LaserMaxPos_DAC calibration data

    // thread
protected:
    MINT32  m_i4LaserThreadLoop;
    pthread_t   m_LaserThread;
    sem_t       m_SemFinish;
    android::Mutex m_LaserMtx;

protected:
    LaserMgr(MINT32 const eSensorDev);

    virtual ~LaserMgr() {}

public:
    static LaserMgr* getInstance(MINT32 const eSensorDev);

public: //Interfaces
    MINT32 init();
    MINT32 uninit();
    MINT32 checkHwSetting();
    MVOID setLaserGoldenTable(MINT32 *a_pTableValue, MINT32 a_i4LaserTableNum, MINT32 a_i4LaserMaxDistance, MINT32 a_i4LaserMaxDistRatio);
    MVOID setAFCaliData(MINT32 a_i4InfDac, MINT32 a_i4InfDist, MINT32 a_i4MacroDac, MINT32 a_i4MacroDist, MINT32 a_i4LaserMaxDac, MINT32 a_i4LaserMaxDist);
    MVOID setLaserCaliData(MINT32 a_i4Offset, MINT32 a_i4XTalk);

    MINT32 getLaserCurDist();
    MINT32 getLaserCurDac();
    MINT32 getLaserCurStatus();
    MINT32 getLaserCurDMAX();

    MINT32 predictAFStartPosDac();
    MINT32 predictAFStartPosDist();

private:
    /**
    * @brief AF thread execution function
    */
    static void* LaserThreadFunc(void *arg);
    MVOID enableLaserThread(MINT32 i4Enable);
    MINT32 transformDistanceToDAC(MINT32 i4Distance);

    // Calibration Process
    MINT32 getLaserOffsetCalib();
    MINT32 setLaserOffsetCalib(MINT32 a_i4Data);
    MINT32 getLaserXTalkCalib();
    MINT32 setLaserXTalkCalib(MINT32 a_i4Data);
};

template <MINT32 const eSensorDev>
class LaserMgrDev : public LaserMgr
{
public:
    static LaserMgr* getInstance()
    {
        static LaserMgrDev<eSensorDev> singleton;
        return &singleton;
    }

    virtual MVOID destroyInstance() {}

    LaserMgrDev() : LaserMgr(eSensorDev) {}

    virtual ~LaserMgrDev() {}
};

#endif  //  _LASER_MGR_H_

