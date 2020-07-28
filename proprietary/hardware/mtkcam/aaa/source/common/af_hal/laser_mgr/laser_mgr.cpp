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

#define LOG_TAG "LaserMgr"

#include <fcntl.h>
#include <utils/threads.h>  // For android::Mutex::Autolock.
#include <aaa_types.h>       /* DataType, Ex : MINT32 */
#include <mtkcam/utils/std/Log.h>
#include <isp_tuning.h>
#include <linux/ioctl.h>

#include "laser_mgr.h"

#define LASER_GOLDEN_TABLE_LEVEL      2
#define LASER_MAX_RAINING_DISTANCE  1201
#define LASER_MIN_RAINING_DISTANCE    60

typedef struct
{
    //current position
    int u4LaserCurPos;
    //laser status
    int u4i4LaserStatus;
    //i4DMAX
    int u4LaserDMAX;
} LaserInfo;

static android::Mutex g_LaserDrvLock;
static MINT32  g_fdLaserDrv = -1;
static MINT32  g_LaserDrvUser = 0;
static MINT32  g_IsLaserDrvExist = -1;
static LaserInfo g_LaserDrvInfo;
static int g_LaserOffsetCaliData = 0;
static int g_LaserXTalkCaliData = 0;

#define LASER_MAGIC 'A'

#define LASER_IOCTL_INIT             _IO(LASER_MAGIC, 0x01)
#define LASER_IOCTL_GETOFFCALB      _IOR(LASER_MAGIC, 0x02, int)
#define LASER_IOCTL_GETXTALKCALB    _IOR(LASER_MAGIC, 0X03, int)
#define LASER_IOCTL_SETOFFCALB      _IOW(LASER_MAGIC, 0x04, int)
#define LASER_IOCTL_SETXTALKCALB    _IOW(LASER_MAGIC, 0x05, int)
#define LASER_IOCTL_GETDATA         _IOR(LASER_MAGIC, 0x0a, LaserInfo)

//------------------------------------------------------------------------------
MVOID LaserDrv_LaserDrv()
{
    android::Mutex::Autolock lock(g_LaserDrvLock);
    g_fdLaserDrv = -1;
    g_LaserDrvUser = 0;
    g_IsLaserDrvExist = -1;
    memset(&g_LaserDrvInfo, 0, sizeof(LaserInfo));
}

MINT32 LaserDrv_init()
{
    android::Mutex::Autolock lock(g_LaserDrvLock);

    if (g_LaserDrvUser == 0)
    {
        if (g_fdLaserDrv == -1)
        {
            CAM_LOGD("[%s]", __FUNCTION__);
            g_fdLaserDrv = open("/dev/laser",O_RDWR);
            if (g_fdLaserDrv < 0)
            {
                CAM_LOGD("[%s] Device error opening : %s", __FUNCTION__, strerror(errno));
                return 0;
            }
        }
    }

    g_LaserDrvUser++;

    return (g_fdLaserDrv > 0) ? 1 : 0;
}

MINT32 LaserDrv_uninit()
{
    android::Mutex::Autolock lock(g_LaserDrvLock);

    g_LaserDrvUser--;

    if (g_LaserDrvUser == 0)
    {
        if (g_fdLaserDrv > 0)
        {
            CAM_LOGD("[%s]", __FUNCTION__);
            close(g_fdLaserDrv);
            g_fdLaserDrv = -1;
            g_IsLaserDrvExist = -1;
        }
    }

    return 1;
}

MINT32 LaserDrv_checkHwSetting()
{
    android::Mutex::Autolock lock(g_LaserDrvLock);

    if (g_fdLaserDrv > 0)
    {
        if (g_IsLaserDrvExist == -1)
        {
            g_IsLaserDrvExist = 1;
            if (ioctl(g_fdLaserDrv, LASER_IOCTL_INIT , NULL) < 0)
            {
                CAM_LOGD("[%s] fail", __FUNCTION__);
                g_IsLaserDrvExist = 0;
            }
        }
    }

    return (g_IsLaserDrvExist > 0) ? 1 : 0;
}

MINT32 LaserDrv_getLaserInfo()
{
    android::Mutex::Autolock lock(g_LaserDrvLock);

    MINT32 res = 1;

    if (g_fdLaserDrv > 0)
    {
        if (ioctl(g_fdLaserDrv, LASER_IOCTL_GETDATA, &g_LaserDrvInfo) < 0)
        {
            CAM_LOGD("[%s] fail", __FUNCTION__);
            res = 0;
        }
    }

    return res;
}

MINT32 LaserDrv_getLaserOffsetCaliData()
{
    android::Mutex::Autolock lock(g_LaserDrvLock);

    MINT32 res = 1;

    if (g_fdLaserDrv > 0)
    {
        if (ioctl(g_fdLaserDrv, LASER_IOCTL_GETOFFCALB, &g_LaserOffsetCaliData) < 0)
        {
            CAM_LOGD("[%s] fail", __FUNCTION__);
            res = 0;
        }
    }

    return res;
}

MINT32 LaserDrv_setLaserOffsetCaliData(MINT32 a_i4Data)
{
    android::Mutex::Autolock lock(g_LaserDrvLock);

    MINT32 res = 1;
    int data = (int)a_i4Data;

    if (g_fdLaserDrv > 0)
    {
        if (ioctl(g_fdLaserDrv, LASER_IOCTL_SETOFFCALB, data) < 0)
        {
            CAM_LOGD("[%s] fail", __FUNCTION__);
            res = 0;
        }
    }

    return res;
}

MINT32 LaserDrv_getLaserXTalkCaliData()
{
    android::Mutex::Autolock lock(g_LaserDrvLock);

    MINT32 res = 1;

    if (g_fdLaserDrv > 0)
    {
        if (ioctl(g_fdLaserDrv, LASER_IOCTL_GETXTALKCALB, &g_LaserXTalkCaliData) < 0)
        {
            CAM_LOGD("[%s] fail", __FUNCTION__);
            res = 0;
        }
    }

    return res;
}

MINT32 LaserDrv_setLaserXTalkCaliData(MINT32 a_i4Data)
{
    android::Mutex::Autolock lock(g_LaserDrvLock);

    MINT32 res = 1;
    int data = (int)a_i4Data;

    if (g_fdLaserDrv > 0)
    {
        if (ioctl(g_fdLaserDrv, LASER_IOCTL_SETXTALKCALB, data) < 0)
        {
            CAM_LOGD("[%s] fail", __FUNCTION__);
            res = 0;
        }
    }

    return res;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
LaserMgr::LaserMgr(MINT32 const eSensorDev)
{
    m_u4SensorDev = eSensorDev;
    m_i4LaserThreadLoop = 0;
    m_i4TransformRule   = 0;
    m_i4AfMarcoDAC      = 0;
    m_i4AfInfDAC        = 0;
    m_i4LaserMaxDAC     = 0;
    m_i4AfMarcoDistance = 100;
    m_i4AfInfDistance   = 5000;

    m_i4LaserCurPos_DAC    = 0;
    m_i4LaserCurPos_RV     = 0;
    m_i4LaserCurPos_Status = 0;
    m_i4LaserCurPos_DMAX   = 0;
    m_i4LaserMaxPos_DAC    = 0;
    m_i4LaserMaxPos_RV     = 0;

    memset(m_aLensDistDac, 0, sizeof(MINT32)*32);
    memset(m_aLaserDistRV, 0, sizeof(MINT32)*32);
    m_i4LaserGoldenTableNum = 0;
    m_i4LaserMaxDistance    = LASER_MAX_RAINING_DISTANCE;
    m_i4LaserMaxDistRatio   = 0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
LaserMgr* LaserMgr::getInstance(MINT32 const eSensorDev)
{
    switch (eSensorDev)
    {
    case NSIspTuning::ESensorDev_Main: //  Main Sensor
        return  LaserMgrDev<NSIspTuning::ESensorDev_Main>::getInstance();
    case NSIspTuning::ESensorDev_MainSecond: //  Main Second Sensor
        return  LaserMgrDev<NSIspTuning::ESensorDev_MainSecond>::getInstance();
    default:
        CAM_LOGE("eSensorDev = %d", eSensorDev);
        return  NULL;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 LaserMgr::init()
{
    CAM_LOGD("[%s] dev(%d)", __FUNCTION__, m_u4SensorDev);

    MINT32 i4Res = LaserDrv_init();

    enableLaserThread(1);

    return i4Res;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 LaserMgr::uninit()
{
    CAM_LOGD("[%s] dev(%d)", __FUNCTION__, m_u4SensorDev);

    LaserDrv_uninit();

    enableLaserThread(0);
    return 0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 LaserMgr::checkHwSetting()
{
    CAM_LOGD("[%s] dev(%d)", __FUNCTION__, m_u4SensorDev);

    return LaserDrv_checkHwSetting();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID LaserMgr::enableLaserThread(MINT32 a_i4Enable)
{
    CAM_LOGD("[%s] dev(%d)", __FUNCTION__, m_u4SensorDev);

    if (a_i4Enable)
    {
        if (m_i4LaserThreadLoop == 0)
        {
            CAM_LOGD("[%s] dev(%d) Create", __FUNCTION__, m_u4SensorDev);

            m_i4LaserThreadLoop = 1;

            sem_init(&m_SemFinish, 0, 0);
            pthread_create(&m_LaserThread, NULL, LaserThreadFunc, this);
        }
    }
    else
    {
        if (m_i4LaserThreadLoop == 1)
        {
            m_i4LaserThreadLoop = 0;

            sem_post(&m_SemFinish);
            pthread_join(m_LaserThread, NULL);

            CAM_LOGD("[%s] dev(%d) Join", __FUNCTION__, m_u4SensorDev);
        }
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void* LaserMgr::LaserThreadFunc(void* arg)
{
    LaserMgr* _this = reinterpret_cast<LaserMgr*>(arg);

    MINT32 i4RangeValue;
    MINT32 i4LaserStatus;
    MINT32 i4DMAX;
    MINT32 i4DacValue;

    sem_wait(&_this->m_SemFinish);

    while (_this->m_i4LaserThreadLoop)
    {
        if (LaserDrv_getLaserInfo())
        {
            android::Mutex::Autolock lock(g_LaserDrvLock);
            i4RangeValue   = g_LaserDrvInfo.u4LaserCurPos;
            i4DMAX         = g_LaserDrvInfo.u4LaserDMAX;
            i4LaserStatus  = g_LaserDrvInfo.u4i4LaserStatus;
        }
        else
        {
            i4RangeValue   = 0;
            i4DMAX         = 0;
            i4LaserStatus  = -1;
        }

        i4DacValue = _this->transformDistanceToDAC(i4RangeValue);

        if (i4DacValue == 0/* && i4LaserStatus == STATUS_RANGING_VALID*/)
        {
            i4LaserStatus  = STATUS_NOT_REFERENCE;
        }

        {
            android::Mutex::Autolock autoLock(_this->m_LaserMtx);

            CAM_LOGD("[LaserInfo] RV(%d) i4DMAX(%d) DAC(%d) Status(%d)", i4RangeValue, i4DMAX, i4DacValue, i4LaserStatus);

            if ((_this->m_i4LaserCurPos_Status != i4LaserStatus && i4LaserStatus != -1) ||
                (_this->m_i4LaserCurPos_DMAX   != i4DMAX        && i4LaserStatus == STATUS_MOVE_DMAX))
            {
                if (i4LaserStatus == STATUS_MOVE_DMAX)
                {
                    _this->m_i4LaserMaxPos_RV  = i4DMAX;
                    _this->m_i4LaserMaxPos_DAC = _this->transformDistanceToDAC(i4DMAX);
                }
                else
                {
                    _this->m_i4LaserMaxPos_RV  = _this->m_i4LaserMaxDistance;

                    if( _this->m_i4LaserMaxDAC > 0 )
                    {
                        _this->m_i4LaserMaxPos_DAC = _this->m_i4LaserMaxDAC;
                    }
                    else
                    {
                        _this->m_i4LaserMaxPos_DAC = _this->transformDistanceToDAC(_this->m_i4LaserMaxDistance);
                    }
                }
            }

            _this->m_i4LaserCurPos_DAC     = i4DacValue;
            _this->m_i4LaserCurPos_RV      = i4RangeValue;
            _this->m_i4LaserCurPos_Status  = i4LaserStatus;
            _this->m_i4LaserCurPos_DMAX    = i4DMAX;
        }
        sem_wait(&_this->m_SemFinish);
    }

    return NULL;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 LaserMgr::transformDistanceToDAC(MINT32 a_i4Distance)
{
    if (m_i4TransformRule == 1)
    {
        MINT32 i4ResultDac = 0;

        if (a_i4Distance <= LASER_MIN_RAINING_DISTANCE)
        {
            return m_i4AfMarcoDAC;
        }

        if (a_i4Distance >= m_i4AfInfDistance)
        {
            a_i4Distance = LASER_MAX_RAINING_DISTANCE;
        }

        MFLOAT fCurDistance   = 1000.0 / a_i4Distance;
        MFLOAT fInfDistance   = 1000.0 / m_i4AfInfDistance;
        MFLOAT fMacroDistance = 1000.0 / m_i4AfMarcoDistance;

        // focusDistance -> DAC
        i4ResultDac = (MINT32)(m_i4AfMarcoDAC + ((fCurDistance - fMacroDistance) / (fInfDistance - fMacroDistance) * (m_i4AfInfDAC - m_i4AfMarcoDAC)));

        return i4ResultDac;
    }

    MINT32 i4DacValue                = 0;
    MINT32 i4LaserDistIdx            = 0;
    MINT32 i4LaserGoldenTableNum     = m_i4LaserGoldenTableNum;

    for (i4LaserDistIdx = 0; i4LaserDistIdx < i4LaserGoldenTableNum; i4LaserDistIdx++)
    {
        if (a_i4Distance < m_aLaserDistRV[i4LaserDistIdx])
        {
            // CAM_LOGD("[LaserInfo] i4LaserDistIdx(%d), a_i4Distance(%d)", i4LaserDistIdx, a_i4Distance);
            break;
        }
    }

    // Transform Laser Range a_pTableValue into Lens DAC
    if (i4LaserDistIdx > 0)
    {
        if (i4LaserDistIdx == i4LaserGoldenTableNum)
        {
            if (m_aLaserDistRV[i4LaserGoldenTableNum-1] - m_aLaserDistRV[i4LaserGoldenTableNum-2] != 0)
            {
                i4DacValue = (m_aLensDistDac[i4LaserGoldenTableNum-2] * (m_aLaserDistRV[i4LaserGoldenTableNum-1] - a_i4Distance) +
                              m_aLensDistDac[i4LaserGoldenTableNum-1] * (a_i4Distance - m_aLaserDistRV[i4LaserGoldenTableNum-2])) /
                             (m_aLaserDistRV[i4LaserGoldenTableNum-1] - m_aLaserDistRV[i4LaserGoldenTableNum-2]);
                /* CAM_LOGD("[LaserInfo] i4DacValue(%d), Distance(%d, %d), DAC(%d, %d)",
                              i4DacValue, m_aLaserDistRV[i4LaserGoldenTableNum-1], m_aLaserDistRV[i4LaserGoldenTableNum-2],
                              m_aLensDistDac[i4LaserGoldenTableNum-1], m_aLensDistDac[i4LaserGoldenTableNum-2]); */
            }
            else
            {
                i4DacValue = 0;
            }
        }
        else
        {
            if (m_aLaserDistRV[i4LaserDistIdx] - m_aLaserDistRV[i4LaserDistIdx-1] != 0)
            {
                i4DacValue = (m_aLensDistDac[i4LaserDistIdx-1] * (m_aLaserDistRV[i4LaserDistIdx] - a_i4Distance) +
                              m_aLensDistDac[i4LaserDistIdx]   * (a_i4Distance - m_aLaserDistRV[i4LaserDistIdx-1])) /
                             (m_aLaserDistRV[i4LaserDistIdx]   - m_aLaserDistRV[i4LaserDistIdx-1]);
                /* CAM_LOGD("[LaserInfo] i4DacValue(%d), Distance(%d, %d), DAC(%d, %d)",
                              i4DacValue, m_aLaserDistRV[i4LaserGoldenTableNum-1], m_aLaserDistRV[i4LaserGoldenTableNum-2],
                              m_aLensDistDac[i4LaserGoldenTableNum-1], m_aLensDistDac[i4LaserGoldenTableNum-2]); */
            }
            else
            {
                i4DacValue = 0;
            }
        }
    }
    else
    {
        i4DacValue = m_aLensDistDac[0];
    }

    // Remapping Golden DAC Table to Current Lens DAC Table
    if (i4DacValue > 0)
    {
        MINT32 i4DacMarcoGT    = m_aLensDistDac[0];
        MINT32 i4DacLaserMaxGT = m_aLensDistDac[i4LaserGoldenTableNum-1];

        if (m_i4LaserMaxDAC > 0 && (i4DacLaserMaxGT - i4DacMarcoGT) != 0)
        {
            i4DacValue = (m_i4AfMarcoDAC * (i4DacLaserMaxGT - i4DacValue) +
                          m_i4LaserMaxDAC * (i4DacValue - i4DacMarcoGT)) /
                         (i4DacLaserMaxGT - i4DacMarcoGT);
            /* CAM_LOGD("[LaserInfo] i4DacValue(%d), LensDistDac(%d, %d), DAC(%d, %d)",
                         i4DacValue, i4DacMarcoGT, i4DacLaserMaxGT,
                         m_i4AfMarcoDAC, m_i4LaserMaxDAC); */
        }
    }

    return i4DacValue;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID LaserMgr::setLaserGoldenTable(MINT32 *a_pTableValue, MINT32 a_i4LaserTableNum, MINT32 a_i4LaserMaxDistance, MINT32 a_i4LaserMaxDistRatio)
{
    if (a_i4LaserTableNum > LASER_GOLDEN_TABLE_LEVEL)
    {
        m_i4LaserGoldenTableNum = a_i4LaserTableNum;
    }

    if (a_i4LaserMaxDistance > LASER_MAX_RAINING_DISTANCE)
    {
        m_i4LaserMaxDistance = a_i4LaserMaxDistance;
    }

    m_i4LaserMaxDistRatio = a_i4LaserMaxDistRatio;

    CAM_LOGD("[%s] dev(%d) LaserMaxRV(%d) GTNum(%d) LaserMaxDistRatio(%d)\n", __FUNCTION__, m_u4SensorDev, m_i4LaserMaxDistance, m_i4LaserGoldenTableNum, m_i4LaserMaxDistRatio);

    if (a_pTableValue[0] > 50 && a_pTableValue[m_i4LaserGoldenTableNum-1] > 50)
    {
        for (MUINT32 i = 0; i < m_i4LaserGoldenTableNum; i++ )
        {
            //a_pTableValue = DAC * 10000 + RV
            m_aLensDistDac[i] = a_pTableValue[i] / 10000;
            m_aLaserDistRV[i] = a_pTableValue[i] - m_aLensDistDac[i] * 10000;
            CAM_LOGD("[%s] dev(%d) Laser GT[%d] : %d - %d\n", __FUNCTION__, m_u4SensorDev, i, m_aLaserDistRV[i], m_aLensDistDac[i]);
        }
        m_i4TransformRule = 0;
    }
    else
    {
        CAM_LOGD("[%s] dev(%d) Laser Golden Table Err\n", __FUNCTION__, m_u4SensorDev);
        m_i4TransformRule = 1;
    }

    sem_post(&m_SemFinish);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID LaserMgr::setAFCaliData(MINT32 a_i4InfDac, MINT32 a_i4InfDist, MINT32 a_i4MacroDac, MINT32 a_i4MacroDist, MINT32 a_i4LaserMaxDac, MINT32 a_i4LaserMaxDist)
{
    m_i4AfMarcoDAC  = a_i4MacroDac;
    m_i4AfInfDAC    = a_i4InfDac;
    m_i4LaserMaxDAC = a_i4LaserMaxDac;

    if (a_i4MacroDist > 0)
        m_i4AfMarcoDistance = a_i4MacroDist;

    if (a_i4InfDist > 0)
        m_i4AfInfDistance   = a_i4InfDist;

    if (m_i4LaserMaxDAC == 0)
    {
        CAM_LOGD("[%s] dev(%d) no laser calibration DAC\n", __FUNCTION__, m_u4SensorDev);

        if (m_i4LaserMaxDistRatio > 0)
        {
            m_i4LaserMaxDAC = m_i4AfInfDAC + (m_i4AfMarcoDAC - m_i4AfInfDAC) * m_i4LaserMaxDistRatio / 1000;
            CAM_LOGD("[%s] dev(%d) LaserMaxDAC(%d) Ratio(%d)\n", __FUNCTION__, m_u4SensorDev, m_i4LaserMaxDAC, m_i4LaserMaxDistRatio);
        }
        else
       	{
            CAM_LOGD("[%s] dev(%d) Please check m_i4LaserMaxDistRatio timing\n", __FUNCTION__, m_u4SensorDev);
        }
    }

    if (a_i4LaserMaxDist > 0)
        m_i4LaserMaxDistance = a_i4LaserMaxDist;

    CAM_LOGD("[%s] dev(%d) AfMarcoDAC(%d) AfInfDAC(%d) LaserMaxDAC(%d) AfMarcoDistance(%d) AfInfDistance(%d) LaserMaxDistance(%d)\n",
               __FUNCTION__, m_u4SensorDev,
               m_i4AfMarcoDAC, m_i4AfInfDAC, m_i4LaserMaxDAC,
               m_i4AfMarcoDistance, m_i4AfInfDistance, m_i4LaserMaxDistance);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID LaserMgr::setLaserCaliData(MINT32 a_i4Offset, MINT32 a_i4XTalk)
{
    MINT32 i4Enable;
    MINT32 i4Data;

    i4Enable = a_i4Offset / 10000;
    i4Data = a_i4Offset - i4Enable * 10000;
    if (i4Enable == 1)
    {
        CAM_LOGD("[%s] dev(%d) NVRAM Laser Offset Cali Data : %d\n", __FUNCTION__, m_u4SensorDev, i4Data);
        setLaserOffsetCalib(i4Data);
    }

    i4Enable = a_i4XTalk / 10000;
    i4Data = a_i4XTalk - i4Enable * 10000;
    if (i4Enable == 1)
    {
        CAM_LOGD("[%s] dev(%d) NVRAM Laser XTalk Cali Data : %d\n", __FUNCTION__, m_u4SensorDev, i4Data);
        setLaserXTalkCalib(i4Data);
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 LaserMgr::getLaserCurDist()
{
    {
        MINT32 i4Val;
        sem_getvalue(&m_SemFinish, &i4Val);
        if (i4Val == 0)
        {
            sem_post(&m_SemFinish);
        }
    }

    android::Mutex::Autolock autoLock(m_LaserMtx);

    MINT32 i4Data = m_i4LaserCurPos_RV;

    return i4Data;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 LaserMgr::getLaserCurDac()
{
    android::Mutex::Autolock autoLock(m_LaserMtx);

    MINT32 i4Data = m_i4LaserCurPos_DAC;

    return i4Data;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 LaserMgr::getLaserCurStatus()
{
    android::Mutex::Autolock autoLock(m_LaserMtx);

    MINT32 i4Data = m_i4LaserCurPos_Status;

    return i4Data;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 LaserMgr::getLaserCurDMAX()
{
    android::Mutex::Autolock autoLock(m_LaserMtx);

    MINT32 i4Data = m_i4LaserCurPos_DMAX;

    return i4Data;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 LaserMgr::predictAFStartPosDac()
{
    {
        MINT32 i4Val;
        sem_getvalue(&m_SemFinish, &i4Val);
        if (i4Val == 0)
        {
            sem_post(&m_SemFinish);
        }
    }

    android::Mutex::Autolock autoLock(m_LaserMtx);

    MINT32 i4Data = m_i4LaserMaxPos_DAC;

    return i4Data;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 LaserMgr::predictAFStartPosDist()
{
    android::Mutex::Autolock autoLock(m_LaserMtx);

    MINT32 i4Data = m_i4LaserMaxPos_RV;

    return i4Data;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 LaserMgr::getLaserOffsetCalib()
{
    MINT32 i4Data = -1;

    if (LaserDrv_getLaserOffsetCaliData())
    {
        android::Mutex::Autolock lock(g_LaserDrvLock);
        i4Data = g_LaserOffsetCaliData;
    }

    return i4Data;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 LaserMgr::setLaserOffsetCalib(MINT32 a_i4Data)
{
    return LaserDrv_setLaserOffsetCaliData(a_i4Data);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 LaserMgr::getLaserXTalkCalib()
{
    MINT32 i4Data = -1;

    if (LaserDrv_getLaserXTalkCaliData())
    {
        android::Mutex::Autolock lock(g_LaserDrvLock);
        i4Data = g_LaserXTalkCaliData;
    }

    return i4Data;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 LaserMgr::setLaserXTalkCalib(MINT32 a_i4Data)
{
    return LaserDrv_setLaserXTalkCaliData(a_i4Data);
}
