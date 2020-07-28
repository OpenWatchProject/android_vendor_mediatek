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
#ifdef MTK_CAM_VHDR_SUPPORT

//! \file vhdr_hal.cpp

#define LOG_TAG "VHDRHal"

#include <stdlib.h>
#include <stdio.h>
#include <cutils/log.h>
#include <cutils/atomic.h>
#include <cutils/properties.h>
#include <utils/Mutex.h>
#include <cutils/atomic.h>

#include <custom/aaa/ae_param.h> // for AE Target Mode
#include "vhdr_drv.h"
#include "vhdr_hal_imp.h"
#include "vhdr_debug.h"

/*******************************************************************************
*
********************************************************************************/

#undef __func__
#define __func__ __FUNCTION__

#define VHDR_LOG(fmt, arg...)    CAM_LOGD("[%s]" fmt, __func__, ##arg)
#define VHDR_INF(fmt, arg...)    CAM_LOGI("[%s]" fmt, __func__, ##arg)
#define VHDR_WRN(fmt, arg...)    CAM_LOGW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define VHDR_ERR(fmt, arg...)    CAM_LOGE("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

#define MY_LOGD_IF(cond, ...)       do { if ( (cond) >= (2) ) { VHDR_LOG(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) >= (1) ) { VHDR_INF(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) >= (1) ) { VHDR_WRN(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) >= (0) ) { VHDR_ERR(__VA_ARGS__); } }while(0)

#define VHDR_HAL_NAME "VHDRHal"


template <> VHdrHalObj<0>* VHdrHalObj<0>::spInstance = 0;
template <> VHdrHalObj<1>* VHdrHalObj<1>::spInstance = 0;
template <> VHdrHalObj<2>* VHdrHalObj<2>::spInstance = 0;
template <> VHdrHalObj<3>* VHdrHalObj<3>::spInstance = 0;

template <> Mutex VHdrHalObj<0>::s_instMutex(::Mutex::PRIVATE);
template <> Mutex VHdrHalObj<1>::s_instMutex(::Mutex::PRIVATE);
template <> Mutex VHdrHalObj<2>::s_instMutex(::Mutex::PRIVATE);
template <> Mutex VHdrHalObj<3>::s_instMutex(::Mutex::PRIVATE);

static MINT32 g_debugDump = 0;
static MINT32 g_disable = 0;

/*Sensor static info: HDR_Support
0: NO HDR, 1: iHDR, 2:mvHDR, 3:zHDR, 4:four-cell mVHDR */
typedef enum
{
    SENSOR_NO_HDR_SUPPORT,
    SENSOR_IHDR_SUPPORT,
    SENSOR_MHDR_SUPPORT,
    SENSOR_ZHDR_SUPPORT,
    SENSOR_4CELL_MHDR_SUPPORT
}eSensorHDRSupportMode;

/*******************************************************************************
*
********************************************************************************/
MBOOL
VHdrHalImp::isVhdrOn_IorZ()
{
    return (mVhdrMode == SENSOR_VHDR_MODE_IVHDR || mVhdrMode == SENSOR_VHDR_MODE_ZVHDR);
}

/*******************************************************************************
*  Get AE Target Mode
********************************************************************************/
eAETargetMODE
VHdrHalImp::getCurrentAEMode(
    MUINT32 vhdrMode
)
{
    switch(vhdrMode)
    {
        case SENSOR_VHDR_MODE_ZVHDR:
            return AE_MODE_ZVHDR_TARGET;
        case SENSOR_VHDR_MODE_IVHDR:
            return AE_MODE_IVHDR_TARGET;
        case SENSOR_VHDR_MODE_MVHDR:
            if(mSensorInfo.HDRSupportMode == SENSOR_4CELL_MHDR_SUPPORT) //4-cell mVHDR
                return AE_MODE_4CELL_MVHDR_TARGET;
            else
                return AE_MODE_MVHDR_TARGET;
    }
    return AE_MODE_NORMAL;
}


/*******************************************************************************
*
********************************************************************************/
VHdrHal *VHdrHal::CreateInstance(char const *userName, const MUINT32 &aSensorIdx)
{
    VHDR_LOG("%s",userName);
    return VHdrHalImp::GetInstance(aSensorIdx);
}

/*******************************************************************************
*
********************************************************************************/
VHdrHal *VHdrHalImp::GetInstance(const MUINT32 &aSensorIdx)
{
    VHDR_LOG( "aSensorIdx(%u)",aSensorIdx);

    switch(aSensorIdx)
    {
        case 0 : return VHdrHalObj<0>::GetInstance();
        case 1 : return VHdrHalObj<1>::GetInstance();
        case 2 : return VHdrHalObj<2>::GetInstance();
        case 3 : return VHdrHalObj<3>::GetInstance();
        default :
            VHDR_LOG( "current limit is 4 sensors, use 0");
            return VHdrHalObj<0>::GetInstance();
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID VHdrHalImp::DestroyInstance(char const *userName)
{
    // We don't delete instance because we have LCSO Buffers.
    // If we deallocate buffers when P2 Thread not done yet, it may cause error.
    VHDR_LOG( "%s",userName);
}

/*******************************************************************************
*
********************************************************************************/
VHdrHalImp::VHdrHalImp(const MUINT32 &aSensorIdx)
    : VHdrHal()
    , mUsers(0)
    , mVhdrMode(0)
    , m_pVHdrDrv(NULL)
    , m_p3aHal(NULL)
    , m_pHalSensorList(NULL)
    , m_pHalSensor(NULL)
    , mSensorDev(0)
    , mSensorIdx(aSensorIdx)
{

}

/*******************************************************************************
*IVHDR_NOT_READY
********************************************************************************/
VHdrHalImp::~VHdrHalImp()
{
    VHDR_LOG( "VHDR Hal destrcutor");

    mVhdrMode = 0;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 VHdrHalImp::Init(const MUINT32 &aMode)
{
    Mutex::Autolock lock(mLock);


    //====== Check Reference Count ======

    if(mUsers > 0)
    {
        android_atomic_inc(&mUsers);
        VHDR_LOG( "mSensorIdx(%u) has %d users",mSensorIdx,mUsers);
        return VHDR_RETURN_NO_ERROR;
    }

    MINT32 err = VHDR_RETURN_NO_ERROR;


    // ======== Debug Property Check =========

    g_debugDump = ::property_get_int32(VHDR_DEBUG_PROP, 0);

    g_disable = ::property_get_int32(VHDR_DISABLE_PROP, 0);

    // force open AE Target
    MINT32 forceAE = ::property_get_int32("vendor.debug.vhdr.force_ae_mode", 0);

    VHDR_INF( "mSensorIdx(%u), VhdrMode(%u) init",mSensorIdx,aMode);
    VHDR_LOG("debug dump = %d, disable = %d, forceAE = %d", g_debugDump, g_disable, forceAE);

    // ===== force to set AE ====
    if(forceAE != 0){
        VHDR_LOG( "force AE Target Mode %d",forceAE);
        m_p3aHal = MAKE_Hal3A(static_cast<MINT32>(mSensorIdx),VHDR_HAL_NAME);
        m_p3aHal->send3ACtrl(E3ACtrl_SetAETargetMode, forceAE, 0);
    }

    //====== Save Mode ======

    mVhdrMode = aMode;
#ifdef IVHDR_NOT_READY
    if(mVhdrMode == SENSOR_VHDR_MODE_IVHDR)
    {
        mVhdrMode = 0;
        VHDR_WRN("ivHDR not support, change mode to none.");
    }
#endif

    //====== Create Sensor Object ======

    m_pHalSensorList = MAKE_HalSensorList();
    if(m_pHalSensorList == NULL)
    {
        VHDR_ERR("IHalSensorList::get fail");
        goto create_fail_exit;
    }

    //====== Get Sensor Static Info ======

    err = GetSensorStaticInfo();
    if(err != VHDR_RETURN_NO_ERROR)
    {
        VHDR_ERR("GetSensorStaticInfo fail(%d)", err);
        return VHDR_RETURN_API_FAIL;
    }

    if(mVhdrMode == SENSOR_VHDR_MODE_IVHDR
        || mVhdrMode == SENSOR_VHDR_MODE_ZVHDR)
    {

        //====== Create VHDR Driver ======

        m_pVHdrDrv = VHdrDrv::CreateInstance(mSensorIdx);

        if(m_pVHdrDrv == NULL)
        {
            VHDR_ERR("VHdrDrv::createInstance fail");
            goto create_fail_exit;
        }

        // ===== Init will do allocate memory ==== (Jou-Feng change)
        err = m_pVHdrDrv->Init();
        if(err != VHDR_RETURN_NO_ERROR)
        {
            VHDR_ERR("VHdrDrv::Init fail");
            goto create_fail_exit;
        }

        android_atomic_inc(&mUsers);
        MY_LOGD_IF(g_debugDump, "-");
        return VHDR_RETURN_NO_ERROR;
    }
    else if(mVhdrMode == SENSOR_VHDR_MODE_MVHDR)
    {
        android_atomic_inc(&mUsers);
        MY_LOGD_IF(g_debugDump, "-");
        return VHDR_RETURN_NO_ERROR;
    }
    else
    {
        VHDR_INF("unknown VHDR mode(%u)",mVhdrMode);
    }

create_fail_exit:

    if(m_pVHdrDrv != NULL)
    {
        m_pVHdrDrv->Uninit();
        m_pVHdrDrv->DestroyInstance();
        m_pVHdrDrv = NULL;
    }

    if(m_pHalSensorList != NULL)
    {
        m_pHalSensorList = NULL;
    }

    MY_LOGD_IF(g_debugDump, "-");
    return VHDR_RETURN_INVALID_DRIVER;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 VHdrHalImp::ConfigVHdr(const VHDR_HAL_CONFIG_DATA &aConfigData)
{
    MY_LOGI_IF(g_debugDump, "VhdrMode(%u)",mVhdrMode);

    MINT32 err = VHDR_RETURN_NO_ERROR;
    (void)aConfigData; // currently this parameter is not used yet.

    //====== Get Sensor Dynamic Info ======

    err = GetSensorDynamicInfo();
    if(err != VHDR_RETURN_NO_ERROR)
    {
        VHDR_ERR("GetSensorDynamicInfo fail(%d)", err);
        return VHDR_RETURN_API_FAIL;
    }

    //====== Create 3A Object ======

    m_p3aHal = MAKE_Hal3A(static_cast<MINT32>(mSensorIdx),VHDR_HAL_NAME);
    if(m_p3aHal == NULL)
    {
        VHDR_ERR("m_p3aHal create fail");
        return VHDR_RETURN_NULL_OBJ;
    }

    //====== Set AE Target Mode ======

    if( ! (g_disable & VHDR_DISABLE_AE_TARGET) ){
        VHDR_INF( "setAETargetMode %d", getCurrentAEMode(mVhdrMode));
        m_p3aHal->send3ACtrl(E3ACtrl_SetAETargetMode, getCurrentAEMode(mVhdrMode), 0);
    }else{
        VHDR_LOG( "disable setAETargetMode");
    }


    if(mVhdrMode == SENSOR_VHDR_MODE_IVHDR
        || mVhdrMode == SENSOR_VHDR_MODE_ZVHDR)
    {
        //> set to VHDR driver

        m_pVHdrDrv->SetSensorInfo(mSensorDev,mSensorInfo.TgInfo); // Jou-Feng change move it to Init()

        //====== Config RMG  ======

        if(mVhdrMode == SENSOR_VHDR_MODE_IVHDR)
        {
            MUINT32 isLeFirst = static_cast<MUINT32>(mSensorInfo.iHDR_First_IS_LE);
            // 0 : first two rows are SE,  1 : first two rows are LE
            VHDR_INF("isLeFirst(%u)",isLeFirst);

            m_pVHdrDrv->ConfigRmg_ivHDR(isLeFirst);
        }
        else if(mVhdrMode == SENSOR_VHDR_MODE_ZVHDR)
        {
            MUINT32 zPattern = static_cast<MUINT32>(mSensorInfo.ZHDR_MODE);
            VHDR_INF("Z-Pattern(%u)",zPattern);    /*0: zhdr no support , 1~ 8 suppport pattern*/

            m_pVHdrDrv->ConfigRmg_zvHDR(zPattern);
            m_pVHdrDrv->ConfigCpnDcpn_zvHDR();

            MUINT32 forcePattern = ::property_get_int32("vendor.debug.vhdr.zpattern", 0);
            if(forcePattern != 0)
            {
                m_pVHdrDrv->ConfigRmg_zvHDR(forcePattern);
                VHDR_INF("Force Set Z-Pattern(%u)",forcePattern);
            }
        }

        //====== Enable VHDR related module ======

        if(isVhdrOn_IorZ())
        {

            if( ! (g_disable & VHDR_DISABLE_RMG) ){
                m_pVHdrDrv->EnableRmg(MTRUE);
            }else{
                VHDR_INF("Don't config RMG because vhdr prop disable RMG");
                m_pVHdrDrv->EnableRmg(MFALSE);
            }

            if( ! (g_disable & VHDR_DISABLE_RMM) ){
                m_pVHdrDrv->EnableRmm(MTRUE);
            }else{
                VHDR_INF("Don't config RMM because vhdr prop disable RMM");
                m_pVHdrDrv->EnableRmm(MFALSE);
            }
        }

    }
    else if(mVhdrMode == SENSOR_VHDR_MODE_MVHDR)
    {
        // Do nothing
    }
    else
    {
        MY_LOGI_IF(g_debugDump, "wrong mode(%u)",mVhdrMode);
    }

    MY_LOGD_IF(g_debugDump, "-");
    return VHDR_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 VHdrHalImp::Uninit()
{
    Mutex::Autolock lock(mLock);

    //====== Check Reference Count ======

    if(mUsers <= 0)
    {
        VHDR_LOG( "mSensorIdx(%u) has 0 user",mSensorIdx);
        return VHDR_RETURN_NO_ERROR;
    }

    //====== Uninitialize ======

    android_atomic_dec(&mUsers);    //decrease referebce count

    if(mUsers == 0)
    {
       VHDR_LOG("mSensorIdx(%u),mode(%u) uninit",mSensorIdx,mVhdrMode);

        MINT32 err = VHDR_RETURN_NO_ERROR;

        //====== Destroy Sensor Object ======

        if(m_pHalSensorList != NULL)
        {
            m_pHalSensorList = NULL;
        }

        // =====  set AE Target Mode back to Normal =======
        if(m_p3aHal != NULL)
        {
            VHDR_LOG( "set AE to normal");
            m_p3aHal->send3ACtrl(E3ACtrl_SetAETargetMode, AE_MODE_NORMAL, 0);
        }

        if(mVhdrMode == SENSOR_VHDR_MODE_IVHDR
            || mVhdrMode == SENSOR_VHDR_MODE_ZVHDR)
        {
            //====== Delete 3A  ======

            if(m_p3aHal != NULL)
            {
                m_p3aHal->destroyInstance(VHDR_HAL_NAME);
                m_p3aHal = NULL;
            }

            //====== Destroy VHDR Driver ======

            if(m_pVHdrDrv != NULL)
            {
                if(isVhdrOn_IorZ())
                {
                    m_pVHdrDrv->EnableRmg(MFALSE);
                    m_pVHdrDrv->EnableRmm(MFALSE);
                }

                m_pVHdrDrv->Uninit();
                m_pVHdrDrv->DestroyInstance();
                m_pVHdrDrv = NULL;
            }
        }
        else if(mVhdrMode == SENSOR_VHDR_MODE_MVHDR)
        {
            //====== Delete 3A  ======

            if(m_p3aHal != NULL)
            {
                m_p3aHal->destroyInstance(VHDR_HAL_NAME);
                m_p3aHal = NULL;
            }
        }
        else
        {
            MY_LOGW_IF(g_debugDump, "unknwon VHDR mode(%u)",mVhdrMode);
        }

        //====== Set State ======
        mVhdrMode = 0;
    }
    else
    {
        VHDR_LOG( "mSensorIdx(%u) has %d users",mSensorIdx,mUsers);
    }

    MY_LOGD_IF(g_debugDump, "-");
    return VHDR_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 VHdrHalImp::GetSensorStaticInfo()
{
    MY_LOGD_IF(g_debugDump, "mSensorIdx(%u)",mSensorIdx);

    SensorStaticInfo staticInfo;
    SensorDynamicInfo dynamicInfo;

    mSensorDev = m_pHalSensorList->querySensorDevIdx(mSensorIdx);
    m_pHalSensorList->querySensorStaticInfo(mSensorDev,&staticInfo);

    VHDR_INF( "Sensor Static Info, previewW,H = %d,%d", staticInfo.previewWidth , staticInfo.previewHeight);
    mSensorInfo.setStaticInfo(staticInfo);

    return VHDR_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 VHdrHalImp::GetSensorDynamicInfo()
{
    MY_LOGD_IF(g_debugDump, "mSensorIdx(%u)",mSensorIdx);

    SensorDynamicInfo dynamicInfo;

    m_pHalSensor = m_pHalSensorList->createSensor(VHDR_HAL_NAME,1,&mSensorIdx);
    if(m_pHalSensor == NULL)
    {
        VHDR_ERR("m_pHalSensorList->createSensor fail");
        return VHDR_RETURN_API_FAIL;
    }

    if(MFALSE == m_pHalSensor->querySensorDynamicInfo(mSensorDev,&dynamicInfo))
    {
        VHDR_ERR("querySensorDynamicInfo fail");
        return VHDR_RETURN_API_FAIL;
    }

    mSensorInfo.setDynamicInfo(dynamicInfo);

    m_pHalSensor->destroyInstance(VHDR_HAL_NAME);
    m_pHalSensor = NULL;

    return VHDR_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MVOID VHdrHalImp::SendCommand(VHDR_CMD_ENUM aCmd,MINT32 arg1, MINT32 arg2, MINT32 arg3)
{
    // To avoid compiler warning
    (void)arg1;
    (void)arg2;
    (void)arg3;

    if(mVhdrMode == SENSOR_VHDR_MODE_IVHDR || mVhdrMode == SENSOR_VHDR_MODE_ZVHDR)
    {
        switch(aCmd)
        {
            case VHDR_CMD_SET_STATE:
                break;
            case VHDR_CMD_SET_VR_EIS_ON_OFF:
                break;
            case VHDR_CMD_SET_VIDEO_SIZE:
                break;
            case VHDR_CMD_SET_ZOOM_RATIO:
                break;
            case VHDR_CMD_SET_PASS1_OUT_SIZE:
                break;
            default:
                VHDR_ERR("wrong cmd(%d)",aCmd);
        }
    }
}

/*******************************************************************************
*
********************************************************************************/
IHal3A::E_VER VHdrHalImp::ConvertCameraVer(const VHDR_CAMERA_VER_ENUM &aVer)
{
    switch(aVer)
    {
        case VHDR_CAMERA_VER_1 : return IHal3A::E_Camera_1;
        case VHDR_CAMERA_VER_3 : return IHal3A::E_Camera_3;
        default : MY_LOGW_IF(g_debugDump, "unknown ver(%d) return E_Camera_1",aVer);
                  return IHal3A::E_Camera_1;
    }
}

#endif //ifdef MTK_CAM_VHDR_SUPPORT end
