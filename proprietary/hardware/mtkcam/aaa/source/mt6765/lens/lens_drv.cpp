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

#define LOG_TAG "LensDrv"
#include <utils/Errors.h>
#include <fcntl.h>
#include <stdlib.h>  //memset
#include <stdio.h> //sprintf
#include <cutils/log.h>

#include "mcu_drv.h"
#include "lens_drv.h"

#define DEBUG_MCU_DRV
#ifdef DEBUG_MCU_DRV
#define DRV_DBG(fmt, arg...) ALOGD(fmt, ##arg)
#define DRV_ERR(fmt, arg...) ALOGE("Err: %5d:, " fmt, __LINE__, ##arg)
#else
#define DRV_DBG(a,...)
#define DRV_ERR(a,...)
#endif

static const char MCU_DRV_NAME[MCU_DEV_NUM][32] = {"/dev/MAINAF", "/dev/MAIN2AF", "/dev/SUBAF", "/dev/SUB2AF", "/dev/MAIN3AF"};
static const char DEV_NAME[MCU_DEV_NUM][32] = {"Main ", "Main2", "Sub  ", "Sub2 ", "Main3"};

/*******************************************************************************
*
********************************************************************************/
MCUDrv*
LensDrv::getInstance()
{
    static LensDrv singleton;
    return &singleton;
}

/*******************************************************************************
*
********************************************************************************/
void
LensDrv::destroyInstance()
{
}

/*******************************************************************************
*
********************************************************************************/
LensDrv::LensDrv()
    : MCUDrv()
{
    DRV_DBG("LensDrv()\n");

    for (int i = 0; i < MCU_DEV_NUM; i++)
    {
        m_LensDev[i].m_fdMCU   = -1;
        m_LensDev[i].m_userCnt = 0;
        m_LensDev[i].m_CurPos  = 0;
        m_LensDev[i].m_InitPos     = 0;
        m_LensDev[i].m_InitDone    = 0;
        m_LensDev[i].m_McuThreadEn = 0;
    }
}

/*******************************************************************************
*
********************************************************************************/
LensDrv::~LensDrv()
{
}

/*******************************************************************************
*
********************************************************************************/
int
LensDrv::init(unsigned int a_u4CurrSensorDev )
{
        Mutex::Autolock lock(mLock);

    unsigned int a_u4CurrLensIdx;

    unsigned int McuDevIdx = MCU_IDX_MAIN;

    switch(a_u4CurrSensorDev)
                {
        case MCU_DEV_MAIN:
            McuDevIdx = MCU_IDX_MAIN;
            break;
        case MCU_DEV_MAIN_2:
            McuDevIdx = MCU_IDX_MAIN_2;
            break;
        case MCU_DEV_SUB:
            McuDevIdx = MCU_IDX_SUB;
            break;
        case MCU_DEV_SUB_2:
            McuDevIdx = MCU_IDX_SUB_2;
            break;
        case MCU_DEV_MAIN_3:
            McuDevIdx = MCU_IDX_MAIN_3;
            break;
        default:
            McuDevIdx = MCU_IDX_MAIN;
            break;
    }

    a_u4CurrLensIdx = MCUDrv::m_u4CurrLensIdx[McuDevIdx];

    DRV_DBG("[%s][%s] : lens init() [m_userCnt] %d  +\n", DEV_NAME[McuDevIdx], __FUNCTION__, m_LensDev[McuDevIdx].m_userCnt);
    DRV_DBG("[%s][%s] : [Lens Driver] %s\n", DEV_NAME[McuDevIdx], __FUNCTION__, MCUDrv::m_LensInitFunc[McuDevIdx][a_u4CurrLensIdx].LensDrvName);

    if (m_LensDev[McuDevIdx].m_userCnt == 0)
    {
        if (m_LensDev[McuDevIdx].m_fdMCU == -1)
        {
            m_LensDev[McuDevIdx].m_fdMCU = open(MCU_DRV_NAME[McuDevIdx], O_RDWR);

            if (m_LensDev[McuDevIdx].m_fdMCU < 0)
            {
                if (a_u4CurrLensIdx == 0)
                {    // no error log for dummy lens
                    return MCUDrv::MCU_NO_ERROR;
                }
                else
                {
                    DRV_DBG("[%s][%s] : Lens error opening %s\n", DEV_NAME[McuDevIdx], __FUNCTION__, strerror(errno));
                    return MCUDrv::MCU_INVALID_DRIVER;
                }
            }

            mcuMotorName motorName;
            memcpy(motorName.uMotorName, MCUDrv::m_LensInitFunc[McuDevIdx][a_u4CurrLensIdx].LensDrvName, 32);

            int err = ioctl(m_LensDev[McuDevIdx].m_fdMCU, mcuIOC_S_SETDRVNAME, &motorName);
            if (err <= 0)
            {
                DRV_DBG("[%s][%s] : please check kernel driver\n", DEV_NAME[McuDevIdx], __FUNCTION__);
                close(m_LensDev[McuDevIdx].m_fdMCU);
                m_LensDev[McuDevIdx].m_fdMCU = -1;
                return MCUDrv::MCU_NO_ERROR;
            }
        }
    }
    m_LensDev[McuDevIdx].m_userCnt++;
    DRV_DBG("[%s][%s] : lens init() [m_userCnt]%d [fdMCU_main]%d - \n", DEV_NAME[McuDevIdx], __FUNCTION__, m_LensDev[McuDevIdx].m_userCnt, m_LensDev[McuDevIdx].m_fdMCU);

    return MCUDrv::MCU_NO_ERROR;
}

int
LensDrv::init(unsigned int a_u4CurrSensorDev, int a_i4InitPos)
{
    Mutex::Autolock lock(mLock);

    DRV_DBG("[%s] Dev : %d +\n", __FUNCTION__, a_u4CurrSensorDev);
    int ret = 0;

    switch(a_u4CurrSensorDev)
    {
        case MCU_DEV_MAIN:
            if (m_LensDev[MCU_IDX_MAIN].m_McuThreadEn == 0 && m_LensDev[MCU_IDX_MAIN].m_InitDone == 0)
            {
                m_LensDev[MCU_IDX_MAIN].m_InitPos     = a_i4InitPos;
                ret = pthread_create(&m_LensDev[MCU_IDX_MAIN].m_InitThread, NULL, initMCUThread<MCU_IDX_MAIN>, this);
                DRV_DBG("[%s][%s] : pthread_create(%d)\n", DEV_NAME[MCU_IDX_MAIN], __FUNCTION__, ret);
                if (ret)
                {
                    DRV_DBG("[%s][%s] : pthread_create fail\n", DEV_NAME[MCU_IDX_MAIN], __FUNCTION__);
                    m_LensDev[MCU_IDX_MAIN].m_McuThreadEn = 0;
                }
                else
                {
                    DRV_DBG("[%s][%s] : pthread_create success\n", DEV_NAME[MCU_IDX_MAIN], __FUNCTION__);
                    m_LensDev[MCU_IDX_MAIN].m_McuThreadEn = 1;
                }
            }
            break;
        case MCU_DEV_MAIN_2:
            if (m_LensDev[MCU_IDX_MAIN_2].m_McuThreadEn == 0 && m_LensDev[MCU_IDX_MAIN_2].m_InitDone == 0)
            {
                m_LensDev[MCU_IDX_MAIN_2].m_InitPos     = a_i4InitPos;
                ret = pthread_create(&m_LensDev[MCU_IDX_MAIN_2].m_InitThread, NULL, initMCUThread<MCU_IDX_MAIN_2>, this);
                DRV_DBG("[%s][%s] : pthread_create(%d)\n", DEV_NAME[MCU_IDX_MAIN_2], __FUNCTION__, ret);
                if (ret)
                {
                    DRV_DBG("[%s][%s] : pthread_create fail\n", DEV_NAME[MCU_IDX_MAIN_2], __FUNCTION__);
                    m_LensDev[MCU_IDX_MAIN_2].m_McuThreadEn = 0;
                }
                else
                {
                    DRV_DBG("[%s][%s] : pthread_create success\n", DEV_NAME[MCU_IDX_MAIN_2], __FUNCTION__);
                    m_LensDev[MCU_IDX_MAIN_2].m_McuThreadEn = 1;
                }
            }
            break;
        case MCU_DEV_SUB:
            if (m_LensDev[MCU_IDX_SUB].m_McuThreadEn == 0 && m_LensDev[MCU_IDX_SUB].m_InitDone == 0)
            {
                m_LensDev[MCU_IDX_SUB].m_InitPos     = a_i4InitPos;
                ret = pthread_create(&m_LensDev[MCU_IDX_SUB].m_InitThread, NULL, initMCUThread<MCU_IDX_SUB>, this);
                DRV_DBG("[%s][%s] : pthread_create(%d)\n", DEV_NAME[MCU_IDX_SUB], __FUNCTION__, ret);
                if (ret)
                {
                    DRV_DBG("[%s][%s] : pthread_create fail\n", DEV_NAME[MCU_IDX_SUB], __FUNCTION__);
                    m_LensDev[MCU_IDX_SUB].m_McuThreadEn = 0;
                }
                else
                {
                    DRV_DBG("[%s][%s] : pthread_create success\n", DEV_NAME[MCU_IDX_SUB], __FUNCTION__);
                    m_LensDev[MCU_IDX_SUB].m_McuThreadEn = 1;
                }
            }
            break;
        case MCU_DEV_SUB_2:
            if (m_LensDev[MCU_IDX_SUB_2].m_McuThreadEn == 0 && m_LensDev[MCU_IDX_SUB_2].m_InitDone == 0)
            {
                m_LensDev[MCU_IDX_SUB_2].m_InitPos     = a_i4InitPos;
                ret = pthread_create(&m_LensDev[MCU_IDX_SUB_2].m_InitThread, NULL, initMCUThread<MCU_IDX_SUB_2>, this);
                DRV_DBG("[%s][%s] : pthread_create(%d)\n", DEV_NAME[MCU_IDX_SUB_2], __FUNCTION__, ret);
                if (ret)
                {
                    DRV_DBG("[%s][%s] : pthread_create fail\n", DEV_NAME[MCU_IDX_SUB_2], __FUNCTION__);
                    m_LensDev[MCU_IDX_SUB_2].m_McuThreadEn = 0;
                }
                else
                {
                    DRV_DBG("[%s][%s] : pthread_create success\n", DEV_NAME[MCU_IDX_SUB_2], __FUNCTION__);
                    m_LensDev[MCU_IDX_SUB_2].m_McuThreadEn = 1;
                }
            }
            break;
        case MCU_DEV_MAIN_3:
            if (m_LensDev[MCU_IDX_MAIN_3].m_McuThreadEn == 0 && m_LensDev[MCU_IDX_MAIN_3].m_InitDone == 0)
            {
                m_LensDev[MCU_IDX_MAIN_3].m_InitPos     = a_i4InitPos;
                ret = pthread_create(&m_LensDev[MCU_IDX_MAIN_3].m_InitThread, NULL, initMCUThread<MCU_IDX_MAIN_3>, this);
                DRV_DBG("[%s][%s] : pthread_create(%d)\n", DEV_NAME[MCU_IDX_MAIN_3], __FUNCTION__, ret);
                if (ret)
                {
                    DRV_DBG("[%s][%s] : pthread_create fail\n", DEV_NAME[MCU_IDX_MAIN_3], __FUNCTION__);
                    m_LensDev[MCU_IDX_MAIN_3].m_McuThreadEn = 0;
                }
                else
                {
                    DRV_DBG("[%s][%s] : pthread_create success\n", DEV_NAME[MCU_IDX_MAIN_3], __FUNCTION__);
                    m_LensDev[MCU_IDX_MAIN_3].m_McuThreadEn = 1;
                }
            }
            break;
        default:
            DRV_DBG("[%s] : skip pthread_create\n", __FUNCTION__);
            break;
    }

    DRV_DBG("[%s] -\n", __FUNCTION__);

    return (ret == 0) ? 1 : 0;
}

template <EMcuDev_T eMcuDev> void* LensDrv::initMCUThread(void* arg)
{
    LensDrv* _this = reinterpret_cast<LensDrv*>(arg);

    _this->initMCU<eMcuDev>();

    return NULL;
}

template <EMcuDev_T eMcuDev> int LensDrv::initMCU()
{
    unsigned int a_u4CurrLensIdx;

    int fdMCU = 0;

    unsigned int McuDevIdx = eMcuDev;

    a_u4CurrLensIdx = MCUDrv::m_u4CurrLensIdx[McuDevIdx];

    DRV_DBG("[%s][%s] : lens init() [m_userCnt] %d  +\n", DEV_NAME[McuDevIdx], __FUNCTION__, m_LensDev[McuDevIdx].m_userCnt);
    DRV_DBG("[%s][%s] : [Lens Driver] %s\n", DEV_NAME[McuDevIdx], __FUNCTION__, MCUDrv::m_LensInitFunc[McuDevIdx][a_u4CurrLensIdx].LensDrvName);

    if (m_LensDev[McuDevIdx].m_userCnt == 0)
    {
        if (m_LensDev[McuDevIdx].m_fdMCU == -1)
        {
            m_LensDev[McuDevIdx].m_fdMCU = fdMCU = open(MCU_DRV_NAME[McuDevIdx], O_RDWR);

            if (m_LensDev[McuDevIdx].m_fdMCU < 0)
            {
                m_LensDev[McuDevIdx].m_CurPos = 0;
                m_LensDev[McuDevIdx].m_InitDone = 1;

                if (a_u4CurrLensIdx == 0)
                {
                    // no error log for dummy lens
                    DRV_DBG("[%s][%s] : dummy lens %d - %d\n", DEV_NAME[McuDevIdx], __FUNCTION__, m_LensDev[McuDevIdx].m_McuThreadEn, m_LensDev[McuDevIdx].m_InitDone);
                    return MCUDrv::MCU_NO_ERROR;
                }
                else
                {
                    DRV_DBG("[%s][%s] : Lens error opening %d - %d, %s\n", DEV_NAME[McuDevIdx], __FUNCTION__, m_LensDev[McuDevIdx].m_McuThreadEn, m_LensDev[McuDevIdx].m_InitDone, strerror(errno));
                    return MCUDrv::MCU_INVALID_DRIVER;
                }
            }

            mcuMotorName motorName;
            memcpy(motorName.uMotorName, MCUDrv::m_LensInitFunc[McuDevIdx][a_u4CurrLensIdx].LensDrvName, 32);

            int err = ioctl(m_LensDev[McuDevIdx].m_fdMCU, mcuIOC_S_SETDRVNAME, &motorName);
            if (err <= 0)
            {
                m_LensDev[McuDevIdx].m_CurPos = 0;
                m_LensDev[McuDevIdx].m_InitDone = 1;
                DRV_DBG("[%s][%s] : please check kernel driver %d - %d\n", DEV_NAME[McuDevIdx], __FUNCTION__, m_LensDev[McuDevIdx].m_McuThreadEn, m_LensDev[McuDevIdx].m_InitDone);
                close(m_LensDev[McuDevIdx].m_fdMCU);
                m_LensDev[McuDevIdx].m_fdMCU = -1;
                return MCUDrv::MCU_UNKNOWN_ERROR;
            }

            err = ioctl(m_LensDev[McuDevIdx].m_fdMCU, mcuIOC_T_MOVETO, (unsigned long)m_LensDev[McuDevIdx].m_InitPos);

            if (err < 0) {
                m_LensDev[McuDevIdx].m_CurPos = 0;
                m_LensDev[McuDevIdx].m_InitDone = 1;
                DRV_DBG("[%s][%s] : ioctl - mcuIOC_T_MOVETO, %d - %d, error %s\n", DEV_NAME[McuDevIdx], __FUNCTION__, m_LensDev[McuDevIdx].m_McuThreadEn, m_LensDev[McuDevIdx].m_InitDone, strerror(errno));
                return MCUDrv::MCU_UNKNOWN_ERROR;
            }

            m_LensDev[McuDevIdx].m_CurPos = m_LensDev[McuDevIdx].m_InitPos;
            m_LensDev[McuDevIdx].m_InitDone = 1;
            DRV_DBG("[%s][%s] : Init Done %d - %d\n", DEV_NAME[McuDevIdx], __FUNCTION__, m_LensDev[McuDevIdx].m_McuThreadEn, m_LensDev[McuDevIdx].m_InitDone);
        }
    }
    m_LensDev[McuDevIdx].m_userCnt++;
    m_LensDev[McuDevIdx].m_InitDone = 1;

    if (m_LensDev[McuDevIdx].m_fdMCU == 0)
    {
        if (fdMCU > 0)
            m_LensDev[McuDevIdx].m_fdMCU = fdMCU;

        DRV_DBG("[%s][%s] : [fdMCU_main]%d (%d)\n", DEV_NAME[McuDevIdx], __FUNCTION__, m_LensDev[McuDevIdx].m_fdMCU, fdMCU);
    }

    DRV_DBG("[%s][%s] : lens init() [m_userCnt]%d [fdMCU_main]%d [Status]%d - %d -\n", DEV_NAME[McuDevIdx], __FUNCTION__, m_LensDev[McuDevIdx].m_userCnt, m_LensDev[McuDevIdx].m_fdMCU, m_LensDev[McuDevIdx].m_McuThreadEn, m_LensDev[McuDevIdx].m_InitDone);

    return MCUDrv::MCU_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
int
LensDrv::uninit(unsigned int a_u4CurrSensorDev )
{
    Mutex::Autolock lock(mLock);

    unsigned int McuDevIdx = MCU_IDX_MAIN;

    switch(a_u4CurrSensorDev)
    {
        case MCU_DEV_MAIN:
            McuDevIdx = MCU_IDX_MAIN;
            break;
        case MCU_DEV_MAIN_2:
            McuDevIdx = MCU_IDX_MAIN_2;
            break;
        case MCU_DEV_SUB:
            McuDevIdx = MCU_IDX_SUB;
            break;
        case MCU_DEV_SUB_2:
            McuDevIdx = MCU_IDX_SUB_2;
            break;
        case MCU_DEV_MAIN_3:
            McuDevIdx = MCU_IDX_MAIN_3;
            break;
        default:
            McuDevIdx = MCU_IDX_MAIN;
            break;
    }

    if (m_LensDev[McuDevIdx].m_McuThreadEn == 1)
    {
        DRV_DBG( "[%s] pthread_join", __FUNCTION__);
        pthread_join(m_LensDev[McuDevIdx].m_InitThread, NULL);
    }

    m_LensDev[McuDevIdx].m_McuThreadEn = 0;
    m_LensDev[McuDevIdx].m_InitDone = 0;
    m_LensDev[McuDevIdx].m_CurPos = 0;

    DRV_DBG("[%s][%s] : lens uninit() [m_userCnt]%d [fdMCU_main]%d +\n", DEV_NAME[McuDevIdx], __FUNCTION__, m_LensDev[McuDevIdx].m_userCnt, m_LensDev[McuDevIdx].m_fdMCU);

    if (m_LensDev[McuDevIdx].m_userCnt == 1)
    {
        if (m_LensDev[McuDevIdx].m_fdMCU > 0)
            close(m_LensDev[McuDevIdx].m_fdMCU);

        m_LensDev[McuDevIdx].m_fdMCU = -1;
    }
    m_LensDev[McuDevIdx].m_userCnt--;

    if (m_LensDev[McuDevIdx].m_userCnt < 0)
        m_LensDev[McuDevIdx].m_userCnt = 0;

    DRV_DBG("[%s][%s] : lens uninit() [fdMCU_main] %d  -\n", DEV_NAME[McuDevIdx], __FUNCTION__, m_LensDev[McuDevIdx].m_fdMCU);

    return MCUDrv::MCU_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
int
LensDrv::moveMCU(int a_i4FocusPos,unsigned int a_u4CurrSensorDev )
{
    Mutex::Autolock lock(mLock);

    //DRV_DBG("moveMCU() - pos = %d \n", a_i4FocusPos);
    int err, a_fdMCU, a_u4CurrLensIdx;

    unsigned int McuDevIdx = MCU_IDX_MAIN;

    switch(a_u4CurrSensorDev)
    {
        case MCU_DEV_MAIN:
            McuDevIdx = MCU_IDX_MAIN;
            break;
        case MCU_DEV_MAIN_2:
            McuDevIdx = MCU_IDX_MAIN_2;
            break;
        case MCU_DEV_SUB:
            McuDevIdx = MCU_IDX_SUB;
            break;
        case MCU_DEV_SUB_2:
            McuDevIdx = MCU_IDX_SUB_2;
            break;
        case MCU_DEV_MAIN_3:
            McuDevIdx = MCU_IDX_MAIN_3;
            break;
        default:
            McuDevIdx = MCU_IDX_MAIN;
            break;
    }

    a_fdMCU = m_LensDev[McuDevIdx].m_fdMCU;
    a_u4CurrLensIdx = MCUDrv::m_u4CurrLensIdx[McuDevIdx];

    if (m_LensDev[McuDevIdx].m_InitDone == 0)
    {
        DRV_DBG("[%s][%s] : driver init(%d:%d) - %d, %d\n", DEV_NAME[McuDevIdx], __FUNCTION__,
                a_u4CurrSensorDev, McuDevIdx, m_LensDev[McuDevIdx].m_McuThreadEn, m_LensDev[McuDevIdx].m_InitDone);
        return 0;
    }

    if (a_fdMCU < 0)
    {
        if (a_u4CurrLensIdx == 0)
        {  // no error log for dummy lens
            return 1;
        }
        else
        {
            DRV_DBG("[%s][%s] : invalid m_fdMCU = %d\n", DEV_NAME[McuDevIdx], __FUNCTION__, a_fdMCU);
            return 1;
        }
    }

    m_LensDev[McuDevIdx].m_CurPos = a_i4FocusPos;

    err = ioctl(a_fdMCU, mcuIOC_T_MOVETO, (unsigned long)a_i4FocusPos);

    if (err < 0) {
        DRV_DBG("[%s][%s] : ioctl - mcuIOC_T_MOVETO, error %s\n", DEV_NAME[McuDevIdx], __FUNCTION__, strerror(errno));
        return 1;
    }

    //DRV_DBG("[%s][%s] : DAC = %d\n", DEV_NAME[McuDevIdx], __FUNCTION__, m_LensDev[McuDevIdx].m_CurPos);

    return 1;
}

/*******************************************************************************
*
********************************************************************************/
int
LensDrv::getMCUInfo(mcuMotorInfo *a_pMotorInfo, unsigned int a_u4CurrSensorDev )
{
    Mutex::Autolock lock(mLock);

    //DRV_DBG("getMCUInfo() - E \n");
#if 0
    int err, a_fdMCU, a_u4CurrLensIdx;
#endif

    unsigned int McuDevIdx = MCU_IDX_MAIN;

    switch(a_u4CurrSensorDev)
    {
        case MCU_DEV_MAIN:
            McuDevIdx = MCU_IDX_MAIN;
            break;
        case MCU_DEV_MAIN_2:
            McuDevIdx = MCU_IDX_MAIN_2;
            break;
        case MCU_DEV_SUB:
            McuDevIdx = MCU_IDX_SUB;
            break;
        case MCU_DEV_SUB_2:
            McuDevIdx = MCU_IDX_SUB_2;
            break;
        case MCU_DEV_MAIN_3:
            McuDevIdx = MCU_IDX_MAIN_3;
            break;
        default:
            McuDevIdx = MCU_IDX_MAIN;
            break;
    }

#if 0
    a_fdMCU = m_LensDev[McuDevIdx].m_fdMCU;
    a_u4CurrLensIdx = MCUDrv::m_u4CurrLensIdx[McuDevIdx];

    mcuMotorInfo motorInfo;
    memset(&motorInfo, 0, sizeof(mcuMotorInfo));

    if (a_fdMCU < 0)
    {
        if (a_u4CurrLensIdx == 0)
        {  // no error log for dummy lens
            a_pMotorInfo->bIsMotorOpen = 0;
            return MCUDrv::MCU_NO_ERROR;
        }
        else
        {
            DRV_DBG("[%s][%s] : invalid m_fdMCU = %d\n", DEV_NAME[McuDevIdx], __FUNCTION__, a_fdMCU);
            a_pMotorInfo->bIsMotorOpen = 0;
            return MCUDrv::MCU_INVALID_DRIVER;
        }
    }

    err = ioctl(a_fdMCU, mcuIOC_G_MOTORINFO, &motorInfo);

    if (err < 0)
    {
        DRV_DBG("[%s][%s] : ioctl - mcuIOC_G_MOTORINFO, error %s\n", DEV_NAME[McuDevIdx], __FUNCTION__, strerror(errno));
        return err;
    }

    a_pMotorInfo->bIsMotorOpen = 1;
    a_pMotorInfo->bIsMotorMoving = motorInfo.bIsMotorMoving;
    a_pMotorInfo->u4CurrentPosition = motorInfo.u4CurrentPosition;
    a_pMotorInfo->u4MacroPosition = motorInfo.u4MacroPosition;
    a_pMotorInfo->u4InfPosition = motorInfo.u4InfPosition;
    a_pMotorInfo->bIsSupportSR = motorInfo.bIsSupportSR;
#else
    a_pMotorInfo->bIsMotorOpen      = m_LensDev[McuDevIdx].m_InitDone;
    a_pMotorInfo->bIsMotorMoving    = 1;
    a_pMotorInfo->u4CurrentPosition = m_LensDev[McuDevIdx].m_CurPos;
    a_pMotorInfo->u4MacroPosition   = 1023;
    a_pMotorInfo->u4InfPosition     = 0;
    a_pMotorInfo->bIsSupportSR      = 1;

    //DRV_DBG("[%s][%s] : DAC = %d\n", DEV_NAME[McuDevIdx], __FUNCTION__, a_pMotorInfo->u4CurrentPosition);
#endif

    return MCUDrv::MCU_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
int
LensDrv::setMCUInfPos(int a_i4FocusPos,unsigned int a_u4CurrSensorDev )
{
    int err, a_fdMCU, a_u4CurrLensIdx;

    unsigned int McuDevIdx = MCU_IDX_MAIN;

    switch(a_u4CurrSensorDev)
    {
        case MCU_DEV_MAIN:
            McuDevIdx = MCU_IDX_MAIN;
            break;
        case MCU_DEV_MAIN_2:
            McuDevIdx = MCU_IDX_MAIN_2;
            break;
        case MCU_DEV_SUB:
            McuDevIdx = MCU_IDX_SUB;
            break;
        case MCU_DEV_SUB_2:
            McuDevIdx = MCU_IDX_SUB_2;
            break;
        case MCU_DEV_MAIN_3:
            McuDevIdx = MCU_IDX_MAIN_3;
            break;
        default:
            McuDevIdx = MCU_IDX_MAIN;
            break;
    }

    DRV_DBG("[%s][%s] : pos = %d\n", DEV_NAME[McuDevIdx], __FUNCTION__, a_i4FocusPos);

    a_fdMCU = m_LensDev[McuDevIdx].m_fdMCU;
    a_u4CurrLensIdx = MCUDrv::m_u4CurrLensIdx[McuDevIdx];

    if (a_fdMCU < 0)
    {
        if (a_u4CurrLensIdx == 0)
        {  // no error log for dummy lens
            return MCUDrv::MCU_NO_ERROR;
        }
        else
        {
            DRV_DBG("[%s][%s] : invalid m_fdMCU = %d\n", DEV_NAME[McuDevIdx], __FUNCTION__, a_fdMCU);
            return MCUDrv::MCU_INVALID_DRIVER;
        }
    }

    err = ioctl(a_fdMCU, mcuIOC_T_SETINFPOS,(unsigned long)a_i4FocusPos);
    if (err  < 0)
    {
        DRV_DBG("[%s][%s] : ioctl - mcuIOC_T_SETINFPOS, error %s\n", DEV_NAME[McuDevIdx], __FUNCTION__, strerror(errno));
        return err;
    }

    return MCUDrv::MCU_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
int
LensDrv::setMCUMacroPos(int a_i4FocusPos,unsigned int a_u4CurrSensorDev )
{
    int err, a_fdMCU,a_u4CurrLensIdx;

    unsigned int McuDevIdx = MCU_IDX_MAIN;

    switch(a_u4CurrSensorDev)
    {
        case MCU_DEV_MAIN:
            McuDevIdx = MCU_IDX_MAIN;
            break;
        case MCU_DEV_MAIN_2:
            McuDevIdx = MCU_IDX_MAIN_2;
            break;
        case MCU_DEV_SUB:
            McuDevIdx = MCU_IDX_SUB;
            break;
        case MCU_DEV_SUB_2:
            McuDevIdx = MCU_IDX_SUB_2;
            break;
        case MCU_DEV_MAIN_3:
            McuDevIdx = MCU_IDX_MAIN_3;
            break;
        default:
            McuDevIdx = MCU_IDX_MAIN;
            break;
    }

    DRV_DBG("[%s][%s] : pos = %d\n", DEV_NAME[McuDevIdx], __FUNCTION__, a_i4FocusPos);

    a_fdMCU = m_LensDev[McuDevIdx].m_fdMCU;
    a_u4CurrLensIdx = MCUDrv::m_u4CurrLensIdx[McuDevIdx];

    if (a_fdMCU < 0)
    {
        if (a_u4CurrLensIdx == 0)
        {  // no error log for dummy lens
            return MCUDrv::MCU_NO_ERROR;
        }
        else
        {
            DRV_DBG("[%s][%s] : invalid m_fdMCU = %d\n", DEV_NAME[McuDevIdx], __FUNCTION__, a_fdMCU);
            return MCUDrv::MCU_INVALID_DRIVER;
        }
    }

    err = ioctl(a_fdMCU,mcuIOC_T_SETMACROPOS,(unsigned long)a_i4FocusPos);
    if (err < 0)
    {
        DRV_DBG("[%s][%s] : ioctl - mcuIOC_T_SETMACROPOS, error %s\n", DEV_NAME[McuDevIdx], __FUNCTION__, strerror(errno));
        return err;
    }

    return MCUDrv::MCU_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
int
LensDrv::getMCUCalPos(mcuMotorCalPos *a_pMotorCalPos,unsigned int a_u4CurrSensorDev)
{
    int err, a_fdMCU, a_u4CurrLensIdx;

    unsigned int McuDevIdx = MCU_IDX_MAIN;

    switch(a_u4CurrSensorDev)
    {
        case MCU_DEV_MAIN:
            McuDevIdx = MCU_IDX_MAIN;
            break;
        case MCU_DEV_MAIN_2:
            McuDevIdx = MCU_IDX_MAIN_2;
            break;
        case MCU_DEV_SUB:
            McuDevIdx = MCU_IDX_SUB;
            break;
        case MCU_DEV_SUB_2:
            McuDevIdx = MCU_IDX_SUB_2;
            break;
        case MCU_DEV_MAIN_3:
            McuDevIdx = MCU_IDX_MAIN_3;
            break;
        default:
            McuDevIdx = MCU_IDX_MAIN;
            break;
    }

    a_fdMCU = m_LensDev[McuDevIdx].m_fdMCU;
    a_u4CurrLensIdx = MCUDrv::m_u4CurrLensIdx[McuDevIdx];

    mcuMotorCalPos motorCalPos;
    memset(&motorCalPos, 0, sizeof(motorCalPos));

    a_pMotorCalPos->u4MacroPos = 0;
    a_pMotorCalPos->u4InfPos = 0;

    if (a_fdMCU < 0)
    {
        if (a_u4CurrLensIdx == 0)
            return MCUDrv::MCU_NO_ERROR;
        else
            return MCUDrv::MCU_INVALID_DRIVER;
    }

    err = ioctl(a_fdMCU, mcuIOC_G_MOTORCALPOS, &motorCalPos);

    if (err < 0) {
        DRV_DBG("[%s][%s] : ioctl - mcuIOC_G_MOTORCALPOS, error %s\n", DEV_NAME[McuDevIdx], __FUNCTION__, strerror(errno));
        return err;
    }

    a_pMotorCalPos->u4MacroPos = motorCalPos.u4MacroPos;
    a_pMotorCalPos->u4InfPos = motorCalPos.u4InfPos;

    return MCUDrv::MCU_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
int
LensDrv::setMCUParam(int a_CmdId, int a_Param,unsigned int a_u4CurrSensorDev )
{
    int err, a_fdMCU,a_u4CurrLensIdx;

    unsigned int McuDevIdx = MCU_IDX_MAIN;

    switch(a_u4CurrSensorDev)
    {
        case MCU_DEV_MAIN:
            McuDevIdx = MCU_IDX_MAIN;
            break;
        case MCU_DEV_MAIN_2:
            McuDevIdx = MCU_IDX_MAIN_2;
            break;
        case MCU_DEV_SUB:
            McuDevIdx = MCU_IDX_SUB;
            break;
        case MCU_DEV_SUB_2:
            McuDevIdx = MCU_IDX_SUB_2;
            break;
        case MCU_DEV_MAIN_3:
            McuDevIdx = MCU_IDX_MAIN_3;
            break;
        default:
            McuDevIdx = MCU_IDX_MAIN;
            break;
    }

    // DRV_DBG("[%s][%s] : a_CmdId = %d\n", DEV_NAME[McuDevIdx], __FUNCTION__, a_CmdId);
    // DRV_DBG("[%s][%s] : a_Param = %d\n", DEV_NAME[McuDevIdx], __FUNCTION__, a_Param);

    a_fdMCU = m_LensDev[McuDevIdx].m_fdMCU;
    a_u4CurrLensIdx = MCUDrv::m_u4CurrLensIdx[McuDevIdx];

    if (a_fdMCU < 0)
    {
        if (a_u4CurrLensIdx == 0)
        {   // no error log for dummy lens
            return MCUDrv::MCU_NO_ERROR;
        }
        else
        {
            DRV_DBG("[%s][%s] : invalid m_fdMCU = %d\n", DEV_NAME[McuDevIdx], __FUNCTION__, a_fdMCU);
            return MCUDrv::MCU_INVALID_DRIVER;
        }
    }

    mcuMotorCmd motorCmd;

    motorCmd.u4CmdID = (unsigned int)a_CmdId;
    motorCmd.u4Param = (unsigned int)a_Param;

    err = ioctl(a_fdMCU, mcuIOC_S_SETPARA, &motorCmd);
    if (err < 0) {
        // DRV_DBG("[%s][%s] : ioctl - mcuIOC_S_SETPARA, error %s\n", DEV_NAME[McuDevIdx], __FUNCTION__, strerror(errno));
        return err;
    }

    return MCUDrv::MCU_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
int
LensDrv::getMCUOISInfo(mcuMotorOISInfo *a_pMotorOISInfo, unsigned int a_u4CurrSensorDev)
{
    //DRV_DBG("getMCUInfo() - E \n");
    int err, a_fdMCU,a_u4CurrLensIdx;

    unsigned int McuDevIdx = MCU_IDX_MAIN;

    switch(a_u4CurrSensorDev)
    {
        case MCU_DEV_MAIN:
            McuDevIdx = MCU_IDX_MAIN;
            break;
        case MCU_DEV_MAIN_2:
            McuDevIdx = MCU_IDX_MAIN_2;
            break;
        case MCU_DEV_SUB:
            McuDevIdx = MCU_IDX_SUB;
            break;
        case MCU_DEV_SUB_2:
            McuDevIdx = MCU_IDX_SUB_2;
            break;
        case MCU_DEV_MAIN_3:
            McuDevIdx = MCU_IDX_MAIN_3;
            break;
        default:
            McuDevIdx = MCU_IDX_MAIN;
            break;
    }

    a_fdMCU = m_LensDev[McuDevIdx].m_fdMCU;
    a_u4CurrLensIdx = MCUDrv::m_u4CurrLensIdx[McuDevIdx];

    mcuMotorOISInfo motorOISInfo;
    memset(&motorOISInfo, 0, sizeof(motorOISInfo));

    a_pMotorOISInfo->i4OISHallPosXum  = 0;
    a_pMotorOISInfo->i4OISHallPosYum  = 0;
    a_pMotorOISInfo->i4OISHallFactorX = 0;
    a_pMotorOISInfo->i4OISHallFactorY = 0;

    if (a_fdMCU < 0)
    {
        return MCUDrv::MCU_INVALID_DRIVER;
    }

    err = ioctl(a_fdMCU, mcuIOC_G_MOTOROISINFO, &motorOISInfo);
    if (err < 0) {
        //DRV_DBG("[getMCUInfo] ioctl - mcuIOC_G_MOTOROISINFO, error %s",  strerror(errno));
        return err;
    }

    a_pMotorOISInfo->i4OISHallPosXum  = motorOISInfo.i4OISHallPosXum;
    a_pMotorOISInfo->i4OISHallPosYum  = motorOISInfo.i4OISHallPosYum;
    a_pMotorOISInfo->i4OISHallFactorX = motorOISInfo.i4OISHallFactorX;
    a_pMotorOISInfo->i4OISHallFactorY = motorOISInfo.i4OISHallFactorY;

    return MCUDrv::MCU_NO_ERROR;
}
/*******************************************************************************
*
********************************************************************************/
int
LensDrv::getOISPosInfo(mcuOISPosInfo *a_pOISPosInfo,unsigned int a_u4CurrSensorDev)
{
    //DRV_DBG("getMCUInfo() - E \n");
    int err, a_fdMCU,a_u4CurrLensIdx;

    unsigned int McuDevIdx = MCU_IDX_MAIN;

    switch(a_u4CurrSensorDev)
    {
        case MCU_DEV_MAIN:
            McuDevIdx = MCU_IDX_MAIN;
            break;
        case MCU_DEV_MAIN_2:
            McuDevIdx = MCU_IDX_MAIN_2;
            break;
        case MCU_DEV_SUB:
            McuDevIdx = MCU_IDX_SUB;
            break;
        case MCU_DEV_SUB_2:
            McuDevIdx = MCU_IDX_SUB_2;
            break;
        case MCU_DEV_MAIN_3:
            McuDevIdx = MCU_IDX_MAIN_3;
            break;
        default:
            McuDevIdx = MCU_IDX_MAIN;
            break;
    }

    a_fdMCU = m_LensDev[McuDevIdx].m_fdMCU;
    a_u4CurrLensIdx = MCUDrv::m_u4CurrLensIdx[McuDevIdx];

    memset(a_pOISPosInfo, 0, sizeof(mcuOISPosInfo));

    if (a_fdMCU < 0)
    {
        return MCUDrv::MCU_INVALID_DRIVER;
    }

    err = ioctl(a_fdMCU, mcuIOC_G_OISPOSINFO, a_pOISPosInfo);
    if (err < 0) {
        memset(a_pOISPosInfo, 0, sizeof(mcuOISPosInfo));
        return err;
    }

    return MCUDrv::MCU_NO_ERROR;
}