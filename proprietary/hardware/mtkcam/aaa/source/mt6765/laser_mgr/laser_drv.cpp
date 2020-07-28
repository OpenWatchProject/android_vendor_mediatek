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

#define LOG_TAG "LaserDrv"
#include <utils/Errors.h>
#include <fcntl.h>
#include <stdlib.h>  //memset
#include <stdio.h> //sprintf
#include <cutils/log.h>

#include "MediaTypes.h"
#include <mtkcam/aaa/drv/laser_drv.h>

#include <isp_tuning.h>
#include <camera_custom_nvram.h>
#include <nvbuf_util.h>


#define DEBUG_LASER_DRV
#ifdef DEBUG_LASER_DRV
#define DRV_DBG(fmt, arg...) ALOGD(fmt, ##arg)
#define DRV_ERR(fmt, arg...) ALOGE("Err: %5d:, " fmt, __LINE__, ##arg)
#else
#define DRV_DBG(a,...)
#define DRV_ERR(a,...)
#endif

#define LASER_THREAD
//#define LASER_CALIB_TO_NVRAM


/*******************************************************************************
*
********************************************************************************/
LaserDrv&
LaserDrv::getInstance()
{
    static LaserDrv singleton;
    return singleton;
}

/*******************************************************************************
*
********************************************************************************/
void
LaserDrv::destroyInstance()
{
}

/*******************************************************************************
*
********************************************************************************/
LaserDrv::LaserDrv()
{
    m_fd_Laser = -1;

    memset(m_LensDistDac, 0, sizeof(m_LensDistDac));
    memset(m_LaserDistRV, 0, sizeof(m_LaserDistRV));
    memset(&m_LaserInfo,  0, sizeof(m_LaserInfo));
    m_LaserInfo.u4LaserStatus = -1;
    m_LaserCurPos_Status      = -1;
    m_LensMarcoPosDAC         = 0;
    m_LensMidPosDAC           = 0;
    m_bLaserThreadLoop        = 0;
    m_LaserGoldenTableNum     = LASER_GOLDEN_TABLE_LEVEL;
    m_LaserMaxRainingDistance = LASER_MAX_RAINING_DISTANCE;
    m_AFStartSearchPos_RV     = LASER_MAX_RAINING_DISTANCE;
    m_AFStartSearchPos_DAC    = 0;
    m_LaserCurPos_DAC         = 0;
    m_LaserCurPos_RV          = 0;
    m_LaserCurPos_Status      = 0;
    m_LaserCurPos_DMAX        = 0;

    DRV_DBG("LaserDrv()\n");
}

/*******************************************************************************
*
********************************************************************************/
LaserDrv::~LaserDrv()
{
}

/*******************************************************************************
*
********************************************************************************/
int
LaserDrv::init(void)
{
    Mutex::Autolock lock(mLock);

    if( m_fd_Laser == -1 )
    {
        m_fd_Laser = open("/dev/laser",O_RDWR);

        if (m_fd_Laser < 0)
        {
            DRV_DBG("Device error opening : %s", strerror(errno));

            return 0;
        }

#ifdef LASER_THREAD
        EnableLaserThread(1);
#endif
    }

    return 1;
}

/*******************************************************************************
*
********************************************************************************/
int
LaserDrv::uninit(void)
{
    Mutex::Autolock lock(mLock);

    if( m_fd_Laser > 0 )
    {
#ifdef LASER_THREAD
        EnableLaserThread(0);
#endif

        close(m_fd_Laser);
        m_fd_Laser = -1;
    }

    return 1;
}

/*******************************************************************************
*
********************************************************************************/
int
LaserDrv::checkHwSetting(void)
{
    if (ioctl(m_fd_Laser, LASER_IOCTL_INIT , NULL) < 0)
    {
        DRV_DBG("Laser Error: Could not perform VL6180_IOCTL_INIT");

        return 0;
    }

    return 1;
}

int
LaserDrv::EnableLaserThread(int a_bEnable)
{
#ifdef LASER_THREAD
    if( a_bEnable )
    {
        if( m_bLaserThreadLoop == 0 )
        {
            DRV_DBG("[LaserThread] Create");

            m_bLaserThreadLoop = 1;

            sem_init(&m_SemFinish, 0, 0);
            pthread_create(&m_LaserThread, NULL, LaserThreadFunc, this);
        }
    }
    else
    {
        if( m_bLaserThreadLoop == 1 )
        {
            m_bLaserThreadLoop = 0;

            sem_post(&m_SemFinish);
            pthread_join(m_LaserThread, NULL);

            DRV_DBG("[LaserThread] Delete");
        }
    }
#endif

    return 0;
}

void*
LaserDrv::LaserThreadFunc(void* arg)
{
#ifdef LASER_THREAD
    LaserDrv* _this = reinterpret_cast<LaserDrv*>(arg);

    int RangeValue;
    int LaserStatus;
    int DMAX;
    int DacValue;

    sem_wait(&_this->m_SemFinish);

    while( _this->m_bLaserThreadLoop )
    {
        if( _this->getLaserInfo() )
        {
            RangeValue   = _this->m_LaserInfo.u4LaserCurPos;
            DMAX         = _this->m_LaserInfo.u4LaserDMAX;
            LaserStatus  = _this->m_LaserInfo.u4LaserStatus;
        }
        else
        {
            RangeValue   = 0;
            DMAX         = 0;
            LaserStatus  = -1;
        }

        DacValue = _this->transformDistanceToDAC(RangeValue);

        if( DacValue == 0 && LaserStatus == STATUS_RANGING_VALID )
            LaserStatus  = STATUS_NOT_REFERENCE;

        {
            Mutex::Autolock autoLock(_this->m_LaserMtx);

            //DRV_DBG("[LaserInfo] RV(%d) DMAX(%d) DAC(%d) Status(%d)", RangeValue, DMAX, DacValue, LaserStatus);

            if( (_this->m_LaserCurPos_Status != LaserStatus && LaserStatus != -1) ||
                    (_this->m_LaserCurPos_DMAX   != DMAX        && LaserStatus == STATUS_MOVE_DMAX) )
            {
                if(LaserStatus == STATUS_MOVE_DMAX)
                {
                    _this->m_AFStartSearchPos_RV  = DMAX;
                    _this->m_AFStartSearchPos_DAC = _this->transformDistanceToDAC(DMAX);
                }
                else
                {
                    _this->m_AFStartSearchPos_RV  = _this->m_LaserMaxRainingDistance;

                    if( _this->m_LensMidPosDAC > 0 )
                    {
                        _this->m_AFStartSearchPos_DAC = _this->m_LensMidPosDAC;
                    }
                    else
                    {
                        _this->m_AFStartSearchPos_DAC = _this->transformDistanceToDAC(_this->m_LaserMaxRainingDistance);
                    }
                }
            }

            _this->m_LaserCurPos_DAC     = DacValue;
            _this->m_LaserCurPos_RV      = RangeValue;
            _this->m_LaserCurPos_Status  = LaserStatus;
            _this->m_LaserCurPos_DMAX    = DMAX;
        }
        sem_wait(&_this->m_SemFinish);
    }
#endif
    return NULL;
}

/*******************************************************************************
*
********************************************************************************/
void
LaserDrv::setLaserGoldenTable(unsigned int *TableValue, int LaserTableNum, int LaserMaxDistance)
{
    int i;

    unsigned int Data;

    unsigned int *Value;

    if( LaserTableNum > LASER_GOLDEN_TABLE_LEVEL )
    {
        m_LaserGoldenTableNum = LaserTableNum;
    }

    if( LaserMaxDistance > LASER_MAX_RAINING_DISTANCE )
    {
        m_LaserMaxRainingDistance = LaserMaxDistance;
    }

    DRV_DBG("Laser LaserMaxRV(%d) GTNum(%d)\n", m_LaserMaxRainingDistance, m_LaserGoldenTableNum);

    if( TableValue[0] > 50 && TableValue[m_LaserGoldenTableNum-1] > 50 )
    {
        Value = TableValue;

        for( i = 0; i < m_LaserGoldenTableNum; i++ )
        {
            //Value = DAC * 10000 + RV
            Data = (unsigned int)Value[i];
            m_LensDistDac[i] = Value[i] / 10000;
            m_LaserDistRV[i] = Value[i] - m_LensDistDac[i] * 10000;
            DRV_DBG("Laser GT[%d] : %d - %d\n", i, m_LaserDistRV[i], m_LensDistDac[i] );
        }
    }
    else
    {
        DRV_DBG("Laser Golden Table Err\n");
    }

    sem_post(&m_SemFinish);
}


/*******************************************************************************
*
********************************************************************************/
void
LaserDrv::setLensCalibrationData(int LensDAC_Marco, int LensDAC_Middle)
{
    m_LensMarcoPosDAC = LensDAC_Marco;
    m_LensMidPosDAC   = LensDAC_Middle;
    DRV_DBG("Lens Calib Data : %d - %d\n", m_LensMarcoPosDAC, m_LensMidPosDAC );
}


/*******************************************************************************
*
********************************************************************************/
int
LaserDrv:: getLaserInfo(void)
{
    int err;

    if( m_fd_Laser > 0 )
    {
        err = ioctl(m_fd_Laser, LASER_IOCTL_GETDATA, &m_LaserInfo);

        if (err < 0)
        {
            DRV_DBG("[getLaserRangingData] ioctl - LASER_IOCTL_GETDATA");

            return 0;
        }
    }

    return 1;
}

/*******************************************************************************
*
********************************************************************************/
int
LaserDrv::transformDistanceToDAC(int Distance)
{
    int DacValue                = 0;
    int LaserDistIdx            = 0;
    int LaserGoldenTableNum     = m_LaserGoldenTableNum;
    int LaserMaxRainingDistance = m_LaserMaxRainingDistance;

    if( ( Distance > LASER_MIN_RAINING_DISTANCE ) && ( Distance <= LaserMaxRainingDistance ) )
    {
        for( LaserDistIdx = 0; LaserDistIdx < LaserGoldenTableNum; LaserDistIdx++ )
        {
            if( Distance < m_LaserDistRV[LaserDistIdx]  )
            {
                break;
            }
        }

        // Transform Laser Range Value into Lens DAC
        if( LaserDistIdx > 0 )
        {
            if( LaserDistIdx == LaserGoldenTableNum )
            {
                if( (m_LaserDistRV[LaserGoldenTableNum - 1] - m_LaserDistRV[LaserGoldenTableNum - 2]) != 0 )
                {
                    DacValue = ( m_LensDistDac[LaserGoldenTableNum - 2] * ( m_LaserDistRV[LaserGoldenTableNum - 1] - Distance ) +
                                 m_LensDistDac[LaserGoldenTableNum - 1] * ( Distance - m_LaserDistRV[LaserGoldenTableNum - 2] ) ) /
                               ( m_LaserDistRV[LaserGoldenTableNum - 1] - m_LaserDistRV[LaserGoldenTableNum - 2] );
                }
                else
                {
                    DacValue = 0;
                }
            }
            else
            {
                if( (m_LaserDistRV[LaserDistIdx] - m_LaserDistRV[LaserDistIdx - 1]) != 0 )
                {
                    DacValue = ( m_LensDistDac[LaserDistIdx - 1] * ( m_LaserDistRV[LaserDistIdx] - Distance ) +
                                 m_LensDistDac[LaserDistIdx] * ( Distance - m_LaserDistRV[LaserDistIdx - 1] ) ) /
                               ( m_LaserDistRV[LaserDistIdx] - m_LaserDistRV[LaserDistIdx - 1] );
                }
                else
                {
                    DacValue = 0;
                }
            }
        }
        else
        {
            DacValue = m_LensDistDac[0];
        }

        // Remapping Golden DAC Table to Current Lens DAC Table
        if( DacValue > 0 )
        {
            int DacMarcoGT = m_LensDistDac[0];
            int DacMidGT = m_LensDistDac[LaserGoldenTableNum - 1];

            if( m_LensMarcoPosDAC > 0 && ( DacMidGT - DacMarcoGT ) != 0 )
            {
                DacValue = ( m_LensMarcoPosDAC * ( DacMidGT - DacValue ) +
                             m_LensMidPosDAC * ( DacValue - DacMarcoGT ) ) /
                           ( DacMidGT - DacMarcoGT );
            }
        }
    }
    else
    {
        DacValue = 0;
    }

    return DacValue;
}

/*******************************************************************************
*
********************************************************************************/
int
LaserDrv:: getLaserCurDist(void)
{
    int Value;

#ifdef LASER_THREAD
    {
        Mutex::Autolock autoLock(m_LaserMtx);
        Value = m_LaserCurPos_RV;
    }
#endif

    {
        int Val;
        sem_getvalue(&m_SemFinish, &Val);
        if (Val == 0)
        {
            sem_post(&m_SemFinish);
        }
    }

    return Value;
}

int
LaserDrv:: getLaserCurDac(void)
{
    int Value;

#ifdef LASER_THREAD
    {
        Mutex::Autolock autoLock(m_LaserMtx);
        Value = m_LaserCurPos_DAC;
    }
#endif

    return Value;
}

int
LaserDrv:: getLaserCurStatus(void)
{
    int Value;

#ifdef LASER_THREAD
    {
        Mutex::Autolock autoLock(m_LaserMtx);
        Value = m_LaserCurPos_Status;
    }
#endif

    return Value;
}

int
LaserDrv:: getLaserCurDMAX(void)
{
    int Value;

#ifdef LASER_THREAD
    {
        Mutex::Autolock autoLock(m_LaserMtx);
        Value = m_LaserCurPos_DMAX;
    }
#endif

    return Value;
}

int
LaserDrv:: predictAFStartPosDac(void)
{
    int Value;

#ifdef LASER_THREAD
    {
        Mutex::Autolock autoLock(m_LaserMtx);
        Value = m_AFStartSearchPos_DAC;
    }
#endif

    {
        int Val;
        sem_getvalue(&m_SemFinish, &Val);
        if (Val == 0)
        {
            sem_post(&m_SemFinish);
        }
    }

    return Value;
}

int
LaserDrv:: predictAFStartPosDist(void)
{
    int Value;

#ifdef LASER_THREAD
    {
        Mutex::Autolock autoLock(m_LaserMtx);
        Value = m_AFStartSearchPos_RV;
    }
#endif

    return Value;
}

/*******************************************************************************
*
********************************************************************************/
void
LaserDrv::setLaserCalibrationData(unsigned int OffsetData, unsigned int XTalkData)
{
    int ReadOffset;
    int ReadXTalk;
    int Enable;
    int CalibData;

    ReadOffset = 0;
    ReadXTalk = 0;

#ifdef LASER_CALIB_TO_NVRAM
    NVRAM_AF_PARA_STRUCT* pNVRAM_LASER;

    int err = NvBufUtil::getInstance().getBufAndReadNoDefault(CAMERA_NVRAM_DATA_AF, 0x01, (void*&)pNVRAM_LASER);
    int val;

    if( err == 0 )
    {
        if (pNVRAM_LASER->LaserOffsetCalEn == 1)
        {
            val = pNVRAM_LASER->LaserOffsetCalVal;
            DRV_DBG("Laser Offset Calib Data : %d", val);
            setLaserOffsetCalib(val);
            ReadOffset = 1;
        }
        if (pNVRAM_LASER->LaserXTalkCalEn == 1)
        {
            val = pNVRAM_LASER->LaserXTalkCalVal;
            DRV_DBG("Laser XTalk Calib Data : %d", val);
            setLaserXTalkCalib(val);
            ReadXTalk = 1;
        }
    }
#endif

    Enable = 0;
    CalibData = 0;

    if( ReadOffset == 0 )
    {
        Enable = OffsetData / 10000;
        CalibData = OffsetData - Enable * 10000;
        if( Enable == 1 )
        {
            DRV_DBG("NVRAM Laser Offset Calib Data : %d", CalibData);
            setLaserOffsetCalib(CalibData);
        }
    }

    Enable = 0;
    CalibData = 0;

    if( ReadXTalk == 0 )
    {
        Enable = XTalkData / 10000;
        CalibData = XTalkData - Enable * 10000;
        if( Enable == 1 )
        {
            DRV_DBG("NVRAM Laser XTalk Calib Data : %d", CalibData);
            setLaserXTalkCalib(CalibData);
        }
    }
}

/*******************************************************************************
*
********************************************************************************/
int
LaserDrv::getLaserOffsetCalib(void)
{
    int err;
    int Value = 0;

    if( m_fd_Laser > 0 )
    {
        err = ioctl(m_fd_Laser, LASER_IOCTL_GETOFFCALB, &Value);

        if (err < 0)
        {
            DRV_DBG("[getLaserOffsetCalib] ioctl - LASER_IOCTL_GETOFFCALB");

            return 0;
        }
    }

#ifdef LASER_CALIB_TO_NVRAM
    NVRAM_AF_PARA_STRUCT* pNVRAM_LASER;

    err = NvBufUtil::getInstance().getBufAndReadNoDefault(CAMERA_NVRAM_DATA_AF, 0x01, (void*&)pNVRAM_LASER);

    DRV_DBG("[getLaserOffsetCalib] LaserCali : %d(%d)", err, Value);

    if( err == 0 )
    {
        pNVRAM_LASER->LaserOffsetCalEn = 1;
        pNVRAM_LASER->LaserOffsetCalVal = Value;
        NvBufUtil::getInstance().write(CAMERA_NVRAM_DATA_AF, 0x01);
    }
#endif

    return Value;
}

int
LaserDrv::setLaserOffsetCalib(int Value)
{
    int err;

    if( m_fd_Laser > 0 )
    {
        DRV_DBG("Laser Offset Calib Data : %d \n", Value);

        err = ioctl(m_fd_Laser, LASER_IOCTL_SETOFFCALB, Value);

        if (err < 0)
        {
            DRV_DBG("[setLaserOffsetCalib] ioctl - LASER_IOCTL_SETOFFCALB");

            return 0;
        }
    }

    return 1;
}

int
LaserDrv::getLaserXTalkCalib(void)
{
    int err;
    int Value = 0;

    if( m_fd_Laser > 0 )
    {
        err = ioctl(m_fd_Laser, LASER_IOCTL_GETXTALKCALB, &Value);

        if (err < 0)
        {
            DRV_DBG("[getLaserXTalkCalib] ioctl - LASER_IOCTL_GETXTALKCALB");

            return 0;
        }
    }

#ifdef LASER_CALIB_TO_NVRAM
    NVRAM_AF_PARA_STRUCT* pNVRAM_LASER;

    err = NvBufUtil::getInstance().getBufAndReadNoDefault(CAMERA_NVRAM_DATA_AF, 0x01, (void*&)pNVRAM_LASER);

    DRV_DBG("[getLaserXTalkCalib] LaserCali : %d(%d)", err, Value);

    if( err == 0 )
    {
        pNVRAM_LASER->LaserXTalkCalEn = 1;
        pNVRAM_LASER->LaserXTalkCalVal = Value;
        NvBufUtil::getInstance().write(CAMERA_NVRAM_DATA_AF, 0x01);
    }
#endif

    return Value;
}

int
LaserDrv::setLaserXTalkCalib(int Value)
{
    int err;

    if( m_fd_Laser > 0 )
    {
        DRV_DBG("Laser XTalk Calib Data : %d \n", Value);

        err = ioctl(m_fd_Laser, LASER_IOCTL_SETXTALKCALB, Value);

        if (err < 0)
        {
            DRV_DBG("[setLaserXTalkCalib] ioctl - LASER_IOCTL_SETXTALKCALB");

            return 0;
        }
    }

    return 1;
}

