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

#define LOG_TAG "LensMCU"
#include <utils/Errors.h>
#include <fcntl.h>
#include <stdlib.h>  //memset
#include <stdio.h> //sprintf
#include <cutils/log.h>
//

#include <mcu_drv.h>
#include "lens_drv.h"

#define DEBUG_MCU_DRV
#ifdef DEBUG_MCU_DRV
#define MCU_DRV_DBG(fmt, arg...) ALOGD(fmt, ##arg)
#define MCU_DRV_ERR(fmt, arg...) ALOGE("Err: %5d:, " fmt, __LINE__, ##arg)
#else
#define MCU_DRV_DBG(a,...)
#define MCU_DRV_ERR(a,...)
#endif

MSDK_LENS_INIT_FUNCTION_STRUCT MCUDrv::m_LensInitFunc[MCU_DEV_NUM][MAX_NUM_OF_SUPPORT_LENS] =
{
    {{0,0,0,"",NULL},{0,0,0,"",NULL},{0,0,0,"",NULL},{0,0,0,"",NULL}},
    {{0,0,0,"",NULL},{0,0,0,"",NULL},{0,0,0,"",NULL},{0,0,0,"",NULL}},
    {{0,0,0,"",NULL},{0,0,0,"",NULL},{0,0,0,"",NULL},{0,0,0,"",NULL}},
    {{0,0,0,"",NULL},{0,0,0,"",NULL},{0,0,0,"",NULL},{0,0,0,"",NULL}}
};

unsigned int  MCUDrv::m_u4CurrLensIdx[MCU_DEV_NUM] = {0, 0, 0, 0};

//extern function in camera_custom_msdk.h
extern unsigned int LensCustomInit(unsigned int a_u4CurrSensorDev);
extern unsigned int LensCustomSetIndex(unsigned int a_u4CurrIdx);
extern unsigned int LensCustomGetInitFunc(MSDK_LENS_INIT_FUNCTION_STRUCT *a_pLensInitFunc);
static Mutex  gLensDevLock;
static const char MCU_DRV_NAME[MCU_DEV_NUM][32] = {"/dev/MAINAF", "/dev/MAIN2AF", "/dev/SUBAF", "/dev/SUB2AF"};

static const char DEV_NAME[MCU_DEV_NUM][32] = {"Main ", "Main2", "Sub  ", "Sub2 "};

/*******************************************************************************
*
********************************************************************************/
MCUDrv::~MCUDrv()
{
}

/*******************************************************************************
*
********************************************************************************/
MCUDrv*
MCUDrv::createInstance(unsigned int /*a_u4CurrLensId*/)
{
    return LensDrv::getInstance();
}


/*******************************************************************************
*
********************************************************************************/
int
MCUDrv::lensSearch( unsigned int a_u4CurrSensorDev, unsigned int a_u4CurrSensorId)
{
    Mutex::Autolock lock(gLensDevLock);

    unsigned int McuDevIdx = MCU_IDX_MAIN;
    int i = 0;

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
    }

    MCU_DRV_DBG("[%s][%s] : [CurrSensorId]0x%04x [McuDevIdx]0x%04x\n", DEV_NAME[McuDevIdx], __FUNCTION__, a_u4CurrSensorId, McuDevIdx);

    LensCustomInit(a_u4CurrSensorDev);

    LensCustomGetInitFunc(&MCUDrv::m_LensInitFunc[McuDevIdx][0]);

    MCUDrv::m_u4CurrLensIdx[McuDevIdx] = 0;

    for (i = 0; i < MAX_NUM_OF_SUPPORT_LENS; i++)
    {
        if (MCUDrv::m_LensInitFunc[McuDevIdx][i].LensId == DUMMY_LENS_ID)
        {
            MCUDrv::m_u4CurrLensIdx[McuDevIdx] = i;
        }
    }

    // force assign LensIdx if SensorId != DUMMY_SENSOR_ID (to support backup lens/new lens driver)
    for (i=0; i<MAX_NUM_OF_SUPPORT_LENS; i++)
    {
        if ((MCUDrv::m_LensInitFunc[McuDevIdx][i].SensorId == a_u4CurrSensorId) && (a_u4CurrSensorId != 0xFFFF) && (a_u4CurrSensorId != 0x0))
        {
            MCU_DRV_DBG("[%s][%s] : [%d] Lens Name %s\n", DEV_NAME[McuDevIdx], __FUNCTION__, i, MCUDrv::m_LensInitFunc[McuDevIdx][i].LensDrvName);
            int fdMCU = open(MCU_DRV_NAME[McuDevIdx], O_RDWR);
            if (fdMCU >= 0)
            {
                mcuMotorName motorName;
                memcpy(motorName.uMotorName, MCUDrv::m_LensInitFunc[McuDevIdx][i].LensDrvName, 32);
                int err = ioctl(fdMCU, mcuIOC_S_SETDRVNAME, &motorName);
                close(fdMCU);
                if (err > 0)
                {
                    MCUDrv::m_u4CurrLensIdx[McuDevIdx] = i;
                    break;
                }
            }
        }
    }
    LensCustomSetIndex(MCUDrv::m_u4CurrLensIdx[McuDevIdx]);

    MCU_DRV_DBG("[%s][%s] : [CurrLensIdx]%d\n", DEV_NAME[McuDevIdx], __FUNCTION__, MCUDrv::m_u4CurrLensIdx[McuDevIdx]);

    return MCU_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
int
MCUDrv::lensSearch( unsigned int a_u4CurrSensorDev, unsigned int a_u4CurrSensorId, unsigned int a_u4CurrModuleId)
{
    Mutex::Autolock lock(gLensDevLock);

    unsigned int McuDevIdx = MCU_IDX_MAIN;
    int i = 0;
    int DummyLensIdx = 0;

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
    }

    MCU_DRV_DBG("[%s][%s] : [CurrSensorId]0x%04x [CurrModuleId]0x%04x [McuDevIdx]0x%04x\n", DEV_NAME[McuDevIdx], __FUNCTION__, a_u4CurrSensorId, a_u4CurrModuleId, McuDevIdx);

    LensCustomInit(a_u4CurrSensorDev);

    LensCustomGetInitFunc(&MCUDrv::m_LensInitFunc[McuDevIdx][0]);

    MCUDrv::m_u4CurrLensIdx[McuDevIdx] = 0;

    for (i = 0; i < MAX_NUM_OF_SUPPORT_LENS; i++)
    {
        if (MCUDrv::m_LensInitFunc[McuDevIdx][i].LensId == DUMMY_LENS_ID)
        {
            MCUDrv::m_u4CurrLensIdx[McuDevIdx] = DummyLensIdx = i;
            break;
        }
    }


    // force assign LensIdx if SensorId != DUMMY_SENSOR_ID (to support backup lens/new lens driver)
    for (i = 0; i < MAX_NUM_OF_SUPPORT_LENS; i++)
    {
        if ((MCUDrv::m_LensInitFunc[McuDevIdx][i].SensorId == a_u4CurrSensorId) && (a_u4CurrSensorId != 0xFFFF) && (a_u4CurrSensorId != 0x0))
        {
            MCU_DRV_DBG("[%s][%s] : [%d] ListSensorID : %x - ListModuleID : %x\n", DEV_NAME[McuDevIdx], __FUNCTION__, i,
                                         MCUDrv::m_LensInitFunc[McuDevIdx][i].SensorId, MCUDrv::m_LensInitFunc[McuDevIdx][i].ModuleId);
            MCU_DRV_DBG("[%s][%s] : [%d] ListLens Name %s - SensorID : %x - ModuleID : %x\n", DEV_NAME[McuDevIdx], __FUNCTION__, i,
                                         MCUDrv::m_LensInitFunc[McuDevIdx][i].LensDrvName, a_u4CurrSensorId, a_u4CurrModuleId);

            #if 1 // use module ID directly
            if (MCUDrv::m_LensInitFunc[McuDevIdx][i].ModuleId == a_u4CurrModuleId && MCUDrv::m_LensInitFunc[McuDevIdx][i].ModuleId != DUMMY_MODULE_ID)
            {
                    MCU_DRV_DBG("[%s][%s] : [%d] Search with module id done\n", DEV_NAME[McuDevIdx], __FUNCTION__, i);
                    MCUDrv::m_u4CurrLensIdx[McuDevIdx] = i;
                    break;
            }
            #else // use kernel driver to read moduel ID
            int fdMCU = open(MCU_DRV_NAME[McuDevIdx], O_RDWR);
            if (fdMCU >= 0)
            {
                mcuMotorName motorName;
                memcpy(motorName.uMotorName, MCUDrv::m_LensInitFunc[McuDevIdx][i].LensDrvName, 32);
                int err = ioctl(fdMCU, mcuIOC_S_SETDRVNAME, &motorName);
                close(fdMCU);
                if (err > 0)
                {
                    MCUDrv::m_u4CurrLensIdx[McuDevIdx] = i;
                    break;
                }
            }
            #endif
        }
    }

    if (MCUDrv::m_u4CurrLensIdx[McuDevIdx] == DummyLensIdx)
    {
        for (i=0; i<MAX_NUM_OF_SUPPORT_LENS; i++)
        {
            if ((MCUDrv::m_LensInitFunc[McuDevIdx][i].SensorId == a_u4CurrSensorId) && (a_u4CurrSensorId != 0xFFFF) && (a_u4CurrSensorId != 0x0))
            {
                MCU_DRV_DBG("[%s][%s] : [%d] Search done\n", DEV_NAME[McuDevIdx], __FUNCTION__, i);
                MCUDrv::m_u4CurrLensIdx[McuDevIdx] = i;
                break;
            }
        }
    }

    LensCustomSetIndex(MCUDrv::m_u4CurrLensIdx[McuDevIdx]);

    MCU_DRV_DBG("[%s][%s] : [CurrLensIdx]%d\n", DEV_NAME[McuDevIdx], __FUNCTION__, MCUDrv::m_u4CurrLensIdx[McuDevIdx]);

    return MCU_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
unsigned int MCUDrv::getCurrLensID( unsigned int a_u4CurrSensorDev)
{

    if( a_u4CurrSensorDev == MCU_DEV_MAIN)
        return MCUDrv::m_LensInitFunc[MCU_IDX_MAIN  ][MCUDrv::m_u4CurrLensIdx[MCU_IDX_MAIN  ]].LensId;
    else if( a_u4CurrSensorDev == MCU_DEV_MAIN_2)
        return MCUDrv::m_LensInitFunc[MCU_IDX_MAIN_2][MCUDrv::m_u4CurrLensIdx[MCU_IDX_MAIN_2]].LensId;
    else if( a_u4CurrSensorDev == MCU_DEV_SUB)
        return MCUDrv::m_LensInitFunc[MCU_IDX_SUB   ][MCUDrv::m_u4CurrLensIdx[MCU_IDX_SUB   ]].LensId;
    else if( a_u4CurrSensorDev == MCU_DEV_SUB_2)
        return MCUDrv::m_LensInitFunc[MCU_IDX_SUB_2 ][MCUDrv::m_u4CurrLensIdx[MCU_IDX_SUB_2 ]].LensId;
    else
        return 0xFFFF;
}

/*******************************************************************************
*
********************************************************************************/
unsigned int MCUDrv::isSupportLens( unsigned int a_u4CurrSensorDev, unsigned int a_u4CurrSensorId)
{
    Mutex::Autolock lock(gLensDevLock);

    int Ret = 0;
    unsigned int McuDevIdx = MCU_IDX_MAIN;
    int i = 0;

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
	}

    MCU_DRV_DBG("[%s][%s] : [CurrSensorId]0x%04x [McuDevIdx]0x%04x\n", DEV_NAME[McuDevIdx], __FUNCTION__, a_u4CurrSensorId, McuDevIdx);

    if ((a_u4CurrSensorId == 0xFFFF) || (a_u4CurrSensorId == 0x0))
    {
        return 0;
    }

    LensCustomInit(a_u4CurrSensorDev);

    LensCustomGetInitFunc(&MCUDrv::m_LensInitFunc[McuDevIdx][0]);

    for (i=0; i<MAX_NUM_OF_SUPPORT_LENS; i++)
    {
        if ((MCUDrv::m_LensInitFunc[McuDevIdx][i].SensorId == a_u4CurrSensorId) && (MCUDrv::m_LensInitFunc[McuDevIdx][i].LensId != DUMMY_LENS_ID))
        {
            Ret = 1;
            break;
        }
    }

    return Ret;
}

/*******************************************************************************
*
********************************************************************************/
unsigned int MCUDrv::setLensNvramIdx( unsigned int a_u4CurrSensorDev)
{
    Mutex::Autolock lock(gLensDevLock);

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
	}

    LensCustomInit(a_u4CurrSensorDev);

    LensCustomGetInitFunc(&MCUDrv::m_LensInitFunc[McuDevIdx][0]);

    LensCustomSetIndex(MCUDrv::m_u4CurrLensIdx[McuDevIdx]);

    MCU_DRV_DBG("[%s][%s] : [CurrLensIdx]%d\n", DEV_NAME[McuDevIdx], __FUNCTION__, MCUDrv::m_u4CurrLensIdx[McuDevIdx]);

    return MCU_NO_ERROR;
}

