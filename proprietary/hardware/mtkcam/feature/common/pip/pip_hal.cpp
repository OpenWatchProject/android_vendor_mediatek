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
*      TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
/**
 * @file pip_hal.cpp
 * @brief PIP Hal Source File
 * @details PIP Hal Source File
 */
#define LOG_TAG "PipHal"
#define MTK_LOG_ENABLE 1

#include <utils/threads.h>
#include <mtkcam/utils/std/Log.h>
#include <android/sensor.h>

#include <cutils/atomic.h>
using namespace android;

#include <mtkcam/drv/IHalSensor.h>     // (Under mediatek\hardware\include\) For SensorStaticInfo struct.

using namespace NSCam;

#include "pip_hal_imp.h"

#include <mtkcam/middleware/v1/camutils/CamInfo.h> // For queryHalSensorDev().

#include <utility>
#include <map>
using namespace std;

/*******************************************************************************
*
********************************************************************************/
#define PIP_HAL_DEBUG

#ifdef PIP_HAL_DEBUG    // Enable debug log.

#undef __func__
#define __func__ __FUNCTION__

#define PIP_DBG(fmt, arg...)    CAM_LOGD("[%s]" fmt, __func__, ##arg)
#define PIP_INF(fmt, arg...)    CAM_LOGI("[%s]" fmt, __func__, ##arg)
#define PIP_WRN(fmt, arg...)    CAM_LOGW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define PIP_ERR(fmt, arg...)    CAM_LOGE("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

#else   // Disable debug log.
#define PIP_DBG(a,...)
#define PIP_INF(a,...)
#define PIP_WRN(a,...)
#define PIP_ERR(a,...)
#endif  // PIP_HAL_DEBUG



/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/
const MUINT32 PIP_BASE_SIZE         = (MUINT32)(1024 * 1024 * 1.05f);
const MUINT32 PIP_SENSOR_SIZE_25M   = 25 * PIP_BASE_SIZE;
const MUINT32 PIP_SENSOR_SIZE_21M   = 21 * PIP_BASE_SIZE;
const MUINT32 PIP_SENSOR_SIZE_20M   = 20 * PIP_BASE_SIZE;
const MUINT32 PIP_SENSOR_SIZE_16M   = 16 * PIP_BASE_SIZE;
const MUINT32 PIP_SENSOR_SIZE_13M   = 13 * PIP_BASE_SIZE;
const MUINT32 PIP_SENSOR_SIZE_8M    =  8 * PIP_BASE_SIZE;
const MUINT32 PIP_SENSOR_SIZE_5M    =  5 * PIP_BASE_SIZE;

struct PIP_FPS {
    const MUINT32 fpsMain;
    const MUINT32 fpsSub;

    PIP_FPS(MUINT32 main_fps=0, MUINT32 sub_fps=0)
        : fpsMain(main_fps)
        , fpsSub(sub_fps)
    {}
};

typedef PIP_FPS ZSD_OFF_FPS;
typedef PIP_FPS ZSD_ON_FPS;

//Main, Sub size(in Mega) -> non-zsd fps, zsd fps
typedef map<pair<MUINT32, MUINT32>, pair<ZSD_OFF_FPS, ZSD_ON_FPS>> FPS_SPEC_T;

const FPS_SPEC_T FPS_SPEC = {
    { {21, 13}, { ZSD_OFF_FPS(24, 24), ZSD_ON_FPS(24, 24) } }, //21M+13M, ZSD off: 24+24, ZSD on: 24+24
    { {16, 13}, { ZSD_OFF_FPS(30, 30), ZSD_ON_FPS(24, 24) } }, //16M+13M, ZSD off: 30+30, ZSD on: 24+24
    { {13, 13}, { ZSD_OFF_FPS(30, 30), ZSD_ON_FPS(26, 26) } }, //13M+13M, ZSD off: 30+30, ZSD on: 26+26
    { {21, 8},  { ZSD_OFF_FPS(30, 30), ZSD_ON_FPS(24, 28) } }, //21M+8M,  ZSD off: 30+30, ZSD on: 24+28
    { {16, 8},  { ZSD_OFF_FPS(30, 30), ZSD_ON_FPS(28, 28) } }, //16M+8M,  ZSD off: 30+30, ZSD on: 28+28
    { {13, 8},  { ZSD_OFF_FPS(30, 30), ZSD_ON_FPS(30, 30) } }, //13M+8M,  ZSD off: 30+30, ZSD on: 30+30
};

/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/

/**************************************************************************
 *       P R I V A T E    F U N C T I O N    D E C L A R A T I O N        *
 **************************************************************************/




/*******************************************************************************
*
********************************************************************************/
PipHal *PipHal::createInstance(const MUINT32 &aSensorIdx)
{
    PipHal *pPipHal = PipHalImp::getInstance(aSensorIdx);
    pPipHal->Init();

    return pPipHal;
}

/*******************************************************************************
*
********************************************************************************/
PipHal *PipHalImp::getInstance(const MUINT32 &aSensorIdx)
{
    PIP_DBG("sensorIdx(%u)",aSensorIdx);

    switch(aSensorIdx)
    {
        case 0 : return PipHalObj<0>::getInstance();
        case 1 : return PipHalObj<1>::getInstance();
        case 2 : return PipHalObj<2>::getInstance();
        case 3 : return PipHalObj<3>::getInstance();
        default :
            PIP_WRN("ROME limit is 4 sensors, use 0");
            return PipHalObj<0>::getInstance();
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID PipHalImp::destroyInstance(char const *userName)
{
    PIP_DBG("%s",userName);
    Uninit();
}

/*******************************************************************************
*
********************************************************************************/
PipHalImp::PipHalImp(const MUINT32 &aSensorIdx) : PipHal()
{
    mUsers = 0;

    //> member variable
    mSensorIdx = aSensorIdx;

    // sensor
    m_pHalSensorList = NULL;
    m_pHalSensor = NULL;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL PipHalImp::Init()
{
    //====== Check Reference Count ======

    Mutex::Autolock lock(mLock);

    if(mUsers > 0)
    {
        android_atomic_inc(&mUsers);
        PIP_DBG("snesorIdx(%u) has %d users", mSensorIdx, mUsers);
        return MTRUE;
    }


    PIP_DBG("mSensorIdx(%u) init", mSensorIdx);

    //====== Create Sensor Object ======
    m_pHalSensorList = MAKE_HalSensorList();
    if(m_pHalSensorList == NULL)
    {
        PIP_ERR("IHalSensorList::get fail");
        goto create_fail_exit;
    }



    //====== Increase User Count ======
    android_atomic_inc(&mUsers);

create_fail_exit:
    PIP_DBG("-");
    return MFALSE;

}

/*******************************************************************************
*
********************************************************************************/
MBOOL PipHalImp::Uninit()
{
    Mutex::Autolock lock(mLock);

    //====== Check Reference Count ======

    if(mUsers <= 0)
    {
        PIP_DBG("mSensorIdx(%u) has 0 user",mSensorIdx);
        return MTRUE;
    }

    //====== Uninitialize ======

    android_atomic_dec(&mUsers);    //decrease referebce count

    if(mUsers == 0)    // there is no user
    {
        PIP_DBG("mSensorIdx(%u) uninit", mSensorIdx);


        //====== Destroy Sensor Object ======

        if(m_pHalSensorList != NULL)
        {
            m_pHalSensorList = NULL;
        }


        //======  Reset Member Variable ======
    }
    else
    {
        PIP_DBG("mSensorIdx(%u) has %d users",mSensorIdx,mUsers);
    }

    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL PipHalImp::GetHwMaxFrameRate(MINT32 i4SensorDev, MUINT32 &ZsdHwMaxFrameRate, MUINT32 &NonZsdHwMaxFrameRate)
{
    // GetMaxFrameRate() will get both Main and Sub sensor HwMaxFrameRate, so i4CurrSensorDev is only for reference.
    MINT32 i4CurrSensorDev = (MINT32)m_pHalSensorList->querySensorDevIdx(mSensorIdx);    // Use current sensor idx to get device id. // Marked because we want to calculate specified sensor device.

    SensorStaticInfo rMainSensorStaticInfo, rSubSensorStaticInfo;
    m_pHalSensorList->querySensorStaticInfo(SENSOR_DEV_MAIN, &rMainSensorStaticInfo);
    m_pHalSensorList->querySensorStaticInfo(SENSOR_DEV_SUB,  &rSubSensorStaticInfo);

    // Get sensor full range
    MINT32 i4SensorFullWidth  = (SENSOR_DEV_MAIN == i4CurrSensorDev) ? rMainSensorStaticInfo.captureWidth : rSubSensorStaticInfo.captureWidth;
    MINT32 i4SensorFullHeight = (SENSOR_DEV_MAIN == i4CurrSensorDev) ? rMainSensorStaticInfo.captureHeight : rSubSensorStaticInfo.captureHeight;
    MINT32 i4MainSensorSize   = rMainSensorStaticInfo.captureWidth * rMainSensorStaticInfo.captureHeight;
    MINT32 i4SubSensorSize    = rSubSensorStaticInfo.captureWidth * rSubSensorStaticInfo.captureHeight;

    // Get RAW or YUV
    MINT32 i4SensorType = (SENSOR_DEV_MAIN == i4CurrSensorDev) ? rMainSensorStaticInfo.sensorType : rSubSensorStaticInfo.sensorType;
    // Check Sensor Static Info.
    if (    // If value not reasonable, return fail.
        i4SensorType < 0 ||
        i4SensorDev < 0 ||
        i4SensorFullWidth < 0 ||
        i4SensorFullHeight < 0 ||
        i4MainSensorSize < 150
    )
    {
        PIP_ERR("querySensorStaticInfo failed, use default Max NonZsd/Zsd value. Sensor Typ/InDev/CurrDev/Idx: (%d <RAW1YUV2>, %d, %d, %d), W/H: (%d, %d)", i4SensorType, i4SensorDev, i4CurrSensorDev, mSensorIdx, i4SensorFullWidth, i4SensorFullHeight);
        return MFALSE;
    }

    ZsdHwMaxFrameRate    = 0;
    NonZsdHwMaxFrameRate = 0;
    PIP_DBG("Try finding combination %d+%d", i4MainSensorSize/1000000, i4SubSensorSize/1000000);
    FPS_SPEC_T::const_iterator it = FPS_SPEC.lower_bound(make_pair(i4MainSensorSize/1000000, i4SubSensorSize/1000000));
    if(it != FPS_SPEC.end()) {
        if (i4SensorDev == SENSOR_DEV_MAIN) {
            NonZsdHwMaxFrameRate = it->second.first.fpsMain;
            ZsdHwMaxFrameRate    = it->second.second.fpsMain;
        } else {
            NonZsdHwMaxFrameRate = it->second.first.fpsSub;
            ZsdHwMaxFrameRate    = it->second.second.fpsSub;
        }
    }

    PIP_DBG("Zsd/NonZsd HW Max FrmRate(%d, %d), Sensor Typ/InDev/CurrDev/Idx: (%d <RAW1YUV2>, %d <Main1Sub2>, %d, %d), W/H: (%d, %d)", ZsdHwMaxFrameRate, NonZsdHwMaxFrameRate, i4SensorType, i4SensorDev, i4CurrSensorDev, mSensorIdx, i4SensorFullWidth, i4SensorFullHeight);

    return MTRUE;   // No error.

}


/*******************************************************************************
*
********************************************************************************/
MBOOL PipHalImp::GetSensorMaxFrameRate(MINT32 i4SensorDev, MUINT32 &ZsdSensorMaxFrameRate, MUINT32 &NonZsdSensorMaxFrameRate)
{
    // GetMaxFrameRate() will get both Main and Sub sensor SensorMaxFrameRate, so i4CurrSensorDev is only for reference.
    MINT32 i4CurrSensorDev = (MINT32)m_pHalSensorList->querySensorDevIdx(mSensorIdx);    // Use current sensor idx to get device id. // Marked because we want to calculate specified sensor device.

    #if 1   // Use static info (sensor don't need be power-on.)
    SensorStaticInfo rSensorStaticInfo;
    m_pHalSensorList->querySensorStaticInfo(i4SensorDev, &rSensorStaticInfo);
    NonZsdSensorMaxFrameRate    = rSensorStaticInfo.previewFrameRate;  // 10base. Eg: 30fps --> 300
    ZsdSensorMaxFrameRate       = rSensorStaticInfo.captureFrameRate;  // 10base. Eg: 30fps --> 300
    #else   // Use sendCommand to get sensor info. (sensor must be power-on, or won't get any info.)
    // Sensor hal init
    IHalSensor *pHalSensorObj;
    pHalSensorObj = m_pHalSensorList->createSensor("pip_hal", mSensorIdx);
    if (pHalSensorObj == NULL) {
        PIP_ERR("Can not create SensorHal obj");
    }

    MUINT32 u4SensorMode = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
    pHalSensorObj->sendCommand(
        i4SensorDev,
        SENSOR_CMD_GET_DEFAULT_FRAME_RATE_BY_SCENARIO,
        (MINT32)&u4SensorMode,
        (MINT32)&ZsdSensorMaxFrameRate, // 10base. Eg: 30fps --> 300
        0
    );

    u4SensorMode = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
    pHalSensorObj->sendCommand(
        i4SensorDev,
        SENSOR_CMD_GET_DEFAULT_FRAME_RATE_BY_SCENARIO,
        (MINT32)&u4SensorMode,
        (MINT32)&NonZsdSensorMaxFrameRate, // 10base. Eg: 30fps --> 300
        0
    );

    pHalSensorObj->destroyInstance("pip_hal");
    #endif  // Diff get sensor info method.

    // Calculate real fps. Eg: 300--->30fps.
    ZsdSensorMaxFrameRate    /= 10;
    NonZsdSensorMaxFrameRate /= 10;

    PIP_DBG("Zsd/NonZsd Sensor Max FrmRate(%d, %d), Sensor InDev/CurrDev/Idx: (%d <Main1Sub2>, %d, %d)", ZsdSensorMaxFrameRate, NonZsdSensorMaxFrameRate, i4SensorDev, i4CurrSensorDev, mSensorIdx);

    return MTRUE;   // No error.

}

/*******************************************************************************
*
********************************************************************************/
MBOOL PipHalImp::GetMaxFrameRate(MUINT32 &ZsdMaxFrameRate, MUINT32 &NonZsdMaxFrameRate)
{
    MINT32 i4CurrSensorDev = (MINT32)m_pHalSensorList->querySensorDevIdx(mSensorIdx);    // Use current sensor idx to get device id. // Marked because we want to calculate specified sensor device.


    // Get Sensor Max Frame Rate
    MUINT32 ZsdSensorMaxFrameRate_Main = 0, NonZsdSensorMaxFrameRate_Main = 0;
    MUINT32 ZsdSensorMaxFrameRate_Sub  = 0, NonZsdSensorMaxFrameRate_Sub  = 0;
    GetSensorMaxFrameRate(SENSOR_DEV_MAIN, ZsdSensorMaxFrameRate_Main, NonZsdSensorMaxFrameRate_Main);
    GetSensorMaxFrameRate(SENSOR_DEV_SUB,  ZsdSensorMaxFrameRate_Sub,  NonZsdSensorMaxFrameRate_Sub );

    // Get HW Max Frame Rate
    MUINT32 ZsdHwMaxFrameRate_Main = 0, NonZsdHwMaxFrameRate_Main = 0;
    MUINT32 ZsdHwMaxFrameRate_Sub  = 0, NonZsdHwMaxFrameRate_Sub  = 0;
    GetHwMaxFrameRate(SENSOR_DEV_MAIN, ZsdHwMaxFrameRate_Main, NonZsdHwMaxFrameRate_Main);
    GetHwMaxFrameRate(SENSOR_DEV_SUB,  ZsdHwMaxFrameRate_Sub,  NonZsdHwMaxFrameRate_Sub );

    MUINT32 ZsdMaxFrameRate_Main    = (ZsdSensorMaxFrameRate_Main    < ZsdHwMaxFrameRate_Main   ) ? (ZsdSensorMaxFrameRate_Main   ) : (ZsdHwMaxFrameRate_Main   );
    MUINT32 NonZsdMaxFrameRate_Main = (NonZsdSensorMaxFrameRate_Main < NonZsdHwMaxFrameRate_Main) ? (NonZsdSensorMaxFrameRate_Main) : (NonZsdHwMaxFrameRate_Main);

    MUINT32 ZsdMaxFrameRate_Sub    = (ZsdSensorMaxFrameRate_Sub    < ZsdHwMaxFrameRate_Sub   ) ? (ZsdSensorMaxFrameRate_Sub   ) : (ZsdHwMaxFrameRate_Sub   );
    MUINT32 NonZsdMaxFrameRate_Sub = (NonZsdSensorMaxFrameRate_Sub < NonZsdHwMaxFrameRate_Sub) ? (NonZsdSensorMaxFrameRate_Sub) : (NonZsdHwMaxFrameRate_Sub);

    #if 1   // Don't align Main/Sub frame rate (if don't want to do HW sync).
    if (i4CurrSensorDev == SENSOR_DEV_MAIN)
    {
        ZsdMaxFrameRate    = ZsdMaxFrameRate_Main;
        NonZsdMaxFrameRate = NonZsdMaxFrameRate_Main;
    }
    else if (i4CurrSensorDev == SENSOR_DEV_SUB)
    {
        ZsdMaxFrameRate    = ZsdMaxFrameRate_Sub;
        NonZsdMaxFrameRate = NonZsdMaxFrameRate_Sub;
    }
    #else   // Align Main/Sub frame rate (if want to do HW sync).
    ZsdMaxFrameRate    = (ZsdMaxFrameRate_Main    < ZsdMaxFrameRate_Sub   ) ? (ZsdMaxFrameRate_Main   ) : (ZsdMaxFrameRate_Sub   );
    NonZsdMaxFrameRate = (NonZsdMaxFrameRate_Main < NonZsdMaxFrameRate_Sub) ? (NonZsdMaxFrameRate_Main) : (NonZsdMaxFrameRate_Sub);
    #endif  // End

    PIP_DBG("Max Zsd/NonZsd FrmRate(%d, %d). CurrSensorDev(%d <Main1Sub2>)", ZsdMaxFrameRate, NonZsdMaxFrameRate, i4CurrSensorDev);

    return MTRUE;   // No error.

}

