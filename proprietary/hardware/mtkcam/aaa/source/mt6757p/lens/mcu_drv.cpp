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
#include "MediaTypes.h"
#include <mtkcam/aaa/drv/mcu_drv.h>
#include "lens_drv.h"
#include "lens_sensor_drv.h"
#include "gaflens_drv.h"

#define DEBUG_MCU_DRV
#ifdef DEBUG_MCU_DRV
#define MCU_DRV_DBG(fmt, arg...) ALOGD(LOG_TAG fmt, ##arg)
#define MCU_DRV_ERR(fmt, arg...) ALOGE(LOG_TAG "Err: %5d:, " fmt, __LINE__, ##arg)
#else
#define MCU_DRV_DBG(a,...)
#define MCU_DRV_ERR(a,...)
#endif


MSDK_LENS_INIT_FUNCTION_STRUCT MCUDrv::m_LensInitFunc_main[MAX_NUM_OF_SUPPORT_LENS] = {{0,0,"",NULL},{0,0,"",NULL},{0,0,"",NULL},{0,0,"",NULL}};
MSDK_LENS_INIT_FUNCTION_STRUCT MCUDrv::m_LensInitFunc_main2[MAX_NUM_OF_SUPPORT_LENS] = {{0,0,"",NULL},{0,0,"",NULL},{0,0,"",NULL},{0,0,"",NULL}};
MSDK_LENS_INIT_FUNCTION_STRUCT MCUDrv::m_LensInitFunc_sub[MAX_NUM_OF_SUPPORT_LENS] = {{0,0,"",NULL},{0,0,"",NULL},{0,0,"",NULL},{0,0,"",NULL}};
MSDK_LENS_INIT_FUNCTION_STRUCT MCUDrv::m_LensInitFunc_sub2[MAX_NUM_OF_SUPPORT_LENS] = {{0,0,"",NULL},{0,0,"",NULL},{0,0,"",NULL},{0,0,"",NULL}};


unsigned int  MCUDrv::m_u4CurrLensIdx_main = 0;
unsigned int  MCUDrv::m_u4CurrLensIdx_main2 = 0;
unsigned int  MCUDrv::m_u4CurrLensIdx_sub = 0;
unsigned int  MCUDrv::m_u4CurrLensIdx_sub2 = 0;


//extern function in camera_custom_msdk.h
//FIXME
extern UINT32 LensCustomInit(unsigned int a_u4CurrSensorDev);
extern UINT32 LensCustomSetIndex(UINT32 a_u4CurrIdx);
extern UINT32 LensCustomGetInitFunc(MSDK_LENS_INIT_FUNCTION_STRUCT *a_pLensInitFunc);

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
MCUDrv::createInstance(unsigned int a_u4CurrLensId)
{
    if (a_u4CurrLensId == SENSOR_DRIVE_LENS_ID)
    {
        return LensSensorDrv::getInstance();
    }
    else if ((a_u4CurrLensId == GAF001AF_LENS_ID)
          || (a_u4CurrLensId == GAF002AF_LENS_ID)
          || (a_u4CurrLensId == GAF008AF_LENS_ID))
    {
        return GAFLensDrv::getInstance();
    }
    else
    {
        return LensDrv::getInstance();
    }
}


/*******************************************************************************
*
********************************************************************************/
int
MCUDrv::lensSearch( unsigned int a_u4CurrSensorDev, unsigned int a_u4CurrSensorId)
{
    INT32 i;
    MCU_DRV_DBG("[CurrSensorDev]0x%04x [CurrSensorId]0x%04x\n", a_u4CurrSensorDev, a_u4CurrSensorId);

    LensCustomInit(a_u4CurrSensorDev);
    if (a_u4CurrSensorDev == MCU_DEV_MAIN )
    {
        LensCustomGetInitFunc(&MCUDrv::m_LensInitFunc_main[0]);

        MCUDrv::m_u4CurrLensIdx_main = 0;

        for (i=0; i<MAX_NUM_OF_SUPPORT_LENS; i++)
        {
            if (MCUDrv::m_LensInitFunc_main[i].LensId == DUMMY_LENS_ID)
            {
                MCUDrv::m_u4CurrLensIdx_main = i;
            }
        }

        // force assign LensIdx if SensorId != DUMMY_SENSOR_ID (to support backup lens/new lens driver)
        for (i=0; i<MAX_NUM_OF_SUPPORT_LENS; i++)
        {
            if ((MCUDrv::m_LensInitFunc_main[i].SensorId == a_u4CurrSensorId) && (a_u4CurrSensorId!=0xFFFF) && (a_u4CurrSensorId!=0x0))
            {
                //MCU_DRV_DBG("[%d] Lens Name %s", i, MCUDrv::m_LensInitFunc_main[i].LensDrvName);
                int fdMCU = open("/dev/MAINAF", O_RDWR);
                if (fdMCU >= 0)
                {
                    mcuMotorName motorName;
                    memcpy(motorName.uMotorName, MCUDrv::m_LensInitFunc_main[i].LensDrvName, 32);
                    int err = ioctl(fdMCU, mcuIOC_S_SETDRVNAME, &motorName);
                    close(fdMCU);
                    if (err > 0)
                    {
                        MCUDrv::m_u4CurrLensIdx_main = i;
                        break;
                    }
                }
            }
        }
        LensCustomSetIndex(MCUDrv::m_u4CurrLensIdx_main);
        MCU_DRV_DBG("[CurrLensIdx]%d", MCUDrv::m_u4CurrLensIdx_main);

    }
    else if( a_u4CurrSensorDev == MCU_DEV_MAIN_2)
    {
        LensCustomGetInitFunc(&MCUDrv::m_LensInitFunc_main2[0]);

        MCUDrv::m_u4CurrLensIdx_main2 = 0;

        for (i=0; i<MAX_NUM_OF_SUPPORT_LENS; i++)
        {
            if (MCUDrv::m_LensInitFunc_main2[i].LensId == DUMMY_LENS_ID)
            {
                MCUDrv::m_u4CurrLensIdx_main2 = i;
            }
        }

        // force assign LensIdx if SensorId != DUMMY_SENSOR_ID (to support backup lens/new lens driver)
        for (i=0; i<MAX_NUM_OF_SUPPORT_LENS; i++)
        {
            if ((MCUDrv::m_LensInitFunc_main2[i].SensorId == a_u4CurrSensorId) && (a_u4CurrSensorId!=0xFFFF) && (a_u4CurrSensorId!=0x0))
            {
                //MCU_DRV_DBG("[%d] Lens Name %s", i, MCUDrv::m_LensInitFunc_main2[i].LensDrvName);
                int fdMCU = open("/dev/MAIN2AF", O_RDWR);
                if (fdMCU >= 0)
                {
                    mcuMotorName motorName;
                    memcpy(motorName.uMotorName, MCUDrv::m_LensInitFunc_main2[i].LensDrvName, 32);
                    int err = ioctl(fdMCU, mcuIOC_S_SETDRVNAME, &motorName);
                    close(fdMCU);
                    if (err > 0)
                    {
                        MCUDrv::m_u4CurrLensIdx_main2 = i;
                        break;
                    }
                }
            }
        }
        LensCustomSetIndex(MCUDrv::m_u4CurrLensIdx_main2);
        MCU_DRV_DBG("[CurrLensIdx]%d", MCUDrv::m_u4CurrLensIdx_main2);

    }
    else if( a_u4CurrSensorDev == MCU_DEV_SUB)
    {
        LensCustomGetInitFunc(&MCUDrv::m_LensInitFunc_sub[0]);

        MCUDrv::m_u4CurrLensIdx_sub = 0;

        for (i=0; i<MAX_NUM_OF_SUPPORT_LENS; i++)
        {
            if (MCUDrv::m_LensInitFunc_sub[i].LensId == DUMMY_LENS_ID)
            {
                MCUDrv::m_u4CurrLensIdx_sub = i;
            }
        }

        // force assign LensIdx if SensorId != DUMMY_SENSOR_ID (to support backup lens/new lens driver)
        for (i=0; i<MAX_NUM_OF_SUPPORT_LENS; i++)
        {
            if ((MCUDrv::m_LensInitFunc_sub[i].SensorId == a_u4CurrSensorId) && (a_u4CurrSensorId!=0xFFFF) && (a_u4CurrSensorId!=0x0))
            {
                //MCU_DRV_DBG("[%d] Lens Name %s", i, MCUDrv::m_LensInitFunc_sub[i].LensDrvName);
                int fdMCU = open("/dev/SUBAF", O_RDWR);
                if (fdMCU >= 0)
                {
                    mcuMotorName motorName;
                    memcpy(motorName.uMotorName, MCUDrv::m_LensInitFunc_sub[i].LensDrvName, 32);
                    int err = ioctl(fdMCU, mcuIOC_S_SETDRVNAME, &motorName);
                    close(fdMCU);
                    if (err > 0)
                    {
                        MCUDrv::m_u4CurrLensIdx_sub = i;
                        break;
                    }
                }
            }
        }
        LensCustomSetIndex(MCUDrv::m_u4CurrLensIdx_sub);
        MCU_DRV_DBG("[CurrLensIdx]%d", MCUDrv::m_u4CurrLensIdx_sub);
    }
    else if( a_u4CurrSensorDev == MCU_DEV_SUB_2)
    {
        LensCustomGetInitFunc(&MCUDrv::m_LensInitFunc_sub2[0]);

        MCUDrv::m_u4CurrLensIdx_sub2 = 0;
        for (i=0; i<MAX_NUM_OF_SUPPORT_LENS; i++)
        {
            if (MCUDrv::m_LensInitFunc_sub2[i].LensId == DUMMY_LENS_ID)
            {
                MCUDrv::m_u4CurrLensIdx_sub2 = i;
            }
        }

        // force assign LensIdx if SensorId != DUMMY_SENSOR_ID (to support backup lens/new lens driver)
        for (i=0; i<MAX_NUM_OF_SUPPORT_LENS; i++)
        {
            if ((MCUDrv::m_LensInitFunc_sub2[i].SensorId == a_u4CurrSensorId) && (a_u4CurrSensorId!=0xFFFF) && (a_u4CurrSensorId!=0x0))
            {
                //MCU_DRV_DBG("[%d] Lens Name %s", i, MCUDrv::m_LensInitFunc_sub2[i].LensDrvName);
                int fdMCU = open("/dev/SUB2AF", O_RDWR);
                if (fdMCU >= 0)
                {
                    mcuMotorName motorName;
                    memcpy(motorName.uMotorName, MCUDrv::m_LensInitFunc_sub2[i].LensDrvName, 32);
                    int err = ioctl(fdMCU, mcuIOC_S_SETDRVNAME, &motorName);
                    close(fdMCU);
                    if (err > 0)
                    {
                        MCUDrv::m_u4CurrLensIdx_sub2 = i;
                        break;
                    }
                }
            }
        }
        LensCustomSetIndex(MCUDrv::m_u4CurrLensIdx_sub2);
        MCU_DRV_DBG("[CurrLensIdx]%d", MCUDrv::m_u4CurrLensIdx_sub2);
    }
    else
        return MCU_INVALID_DRIVER;


    return MCU_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
unsigned int MCUDrv::getCurrLensID( unsigned int a_u4CurrSensorDev)
{

    if( a_u4CurrSensorDev == MCU_DEV_MAIN)
        return MCUDrv::m_LensInitFunc_main[MCUDrv::m_u4CurrLensIdx_main].LensId;
    else if( a_u4CurrSensorDev == MCU_DEV_MAIN_2)
        return MCUDrv::m_LensInitFunc_main2[MCUDrv::m_u4CurrLensIdx_main2].LensId;
    else if( a_u4CurrSensorDev == MCU_DEV_SUB)
        return MCUDrv::m_LensInitFunc_sub[MCUDrv::m_u4CurrLensIdx_sub].LensId;
    else if( a_u4CurrSensorDev == MCU_DEV_SUB_2)
        return MCUDrv::m_LensInitFunc_sub2[MCUDrv::m_u4CurrLensIdx_sub2].LensId;
    else
        return 0xFFFF;
}

/*******************************************************************************
*
********************************************************************************/
unsigned int MCUDrv::isSupportLens( unsigned int a_u4CurrSensorDev, unsigned int a_u4CurrSensorId)
{
    int Ret = 0;
    INT32 i;
    MCU_DRV_DBG("[isSupportLens][CurrSensorDev]0x%04x [CurrSensorId]0x%04x\n", a_u4CurrSensorDev, a_u4CurrSensorId);

    if ((a_u4CurrSensorId==0xFFFF) || (a_u4CurrSensorId==0x0))
    {
        return 0;
    }

    LensCustomInit(a_u4CurrSensorDev);
    if (a_u4CurrSensorDev == MCU_DEV_MAIN )
    {
        LensCustomGetInitFunc(&MCUDrv::m_LensInitFunc_main[0]);

        for (i=0; i<MAX_NUM_OF_SUPPORT_LENS; i++)
        {
            if ((MCUDrv::m_LensInitFunc_main[i].SensorId == a_u4CurrSensorId) && (MCUDrv::m_LensInitFunc_main[i].LensId != DUMMY_LENS_ID))
            {
                Ret = 1;
                break;
            }
        }
    }
    else if( a_u4CurrSensorDev == MCU_DEV_MAIN_2)
    {
        LensCustomGetInitFunc(&MCUDrv::m_LensInitFunc_main2[0]);

        for (i=0; i<MAX_NUM_OF_SUPPORT_LENS; i++)
        {
            if ((MCUDrv::m_LensInitFunc_main2[i].SensorId == a_u4CurrSensorId) && (MCUDrv::m_LensInitFunc_main2[i].LensId != DUMMY_LENS_ID))
            {
                Ret = 1;
                break;
            }
        }
    }
    else if( a_u4CurrSensorDev == MCU_DEV_SUB)
    {
        LensCustomGetInitFunc(&MCUDrv::m_LensInitFunc_sub[0]);

        for (i=0; i<MAX_NUM_OF_SUPPORT_LENS; i++)
        {
            if ((MCUDrv::m_LensInitFunc_sub[i].SensorId == a_u4CurrSensorId) && (MCUDrv::m_LensInitFunc_sub[i].LensId != DUMMY_LENS_ID))
            {
                Ret = 1;
                break;
            }
        }
    }
    else if( a_u4CurrSensorDev == MCU_DEV_SUB_2)
    {
        LensCustomGetInitFunc(&MCUDrv::m_LensInitFunc_sub2[0]);

        for (i=0; i<MAX_NUM_OF_SUPPORT_LENS; i++)
        {
            if ((MCUDrv::m_LensInitFunc_sub2[i].SensorId == a_u4CurrSensorId) && (MCUDrv::m_LensInitFunc_sub2[i].LensId != DUMMY_LENS_ID))
            {
                Ret = 1;
                break;
            }
        }
    }

    return Ret;
}

/*******************************************************************************
*
********************************************************************************/
unsigned int MCUDrv::setLensNvramIdx( unsigned int a_u4CurrSensorDev)
{
    LensCustomInit(a_u4CurrSensorDev);

    if (a_u4CurrSensorDev == MCU_DEV_MAIN )
    {
        LensCustomGetInitFunc(&MCUDrv::m_LensInitFunc_main[0]);

        LensCustomSetIndex(MCUDrv::m_u4CurrLensIdx_main);

        MCU_DRV_DBG("[Dev]%d [CurrLensIdx]%d", a_u4CurrSensorDev, MCUDrv::m_u4CurrLensIdx_main);
    }
    else if( a_u4CurrSensorDev == MCU_DEV_MAIN_2)
    {
        LensCustomGetInitFunc(&MCUDrv::m_LensInitFunc_main2[0]);

        LensCustomSetIndex(MCUDrv::m_u4CurrLensIdx_main2);

        MCU_DRV_DBG("[Dev]%d [CurrLensIdx2Idx]%d", a_u4CurrSensorDev, MCUDrv::m_u4CurrLensIdx_main2);
    }
    else if( a_u4CurrSensorDev == MCU_DEV_SUB)
    {
        LensCustomGetInitFunc(&MCUDrv::m_LensInitFunc_sub[0]);

        LensCustomSetIndex(MCUDrv::m_u4CurrLensIdx_sub);

        MCU_DRV_DBG("[Dev]%d [CurrLensIdx]%d", a_u4CurrSensorDev, MCUDrv::m_u4CurrLensIdx_sub);
    }
    else if( a_u4CurrSensorDev == MCU_DEV_SUB_2)
    {
        LensCustomGetInitFunc(&MCUDrv::m_LensInitFunc_sub2[0]);

        LensCustomSetIndex(MCUDrv::m_u4CurrLensIdx_sub2);

        MCU_DRV_DBG("[Dev]%d [CurrLensIdx]%d", a_u4CurrSensorDev, MCUDrv::m_u4CurrLensIdx_sub2);
    }


    return MCU_NO_ERROR;
}
