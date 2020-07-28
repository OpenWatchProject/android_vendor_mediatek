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


#include <stdlib.h>
#include <stdio.h>
#include <cutils/atomic.h>
#include <cutils/properties.h>
#include <utils/Mutex.h>
#include <utils/String8.h>

#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <custom/aaa/ae_param.h> // for AE Target Mode
#include "vhdr_hal_imp.h"
#include "vhdr_debug.h"

/*******************************************************************************
*
********************************************************************************/
#undef LOG_TAG
#define LOG_TAG "VHDRHal"

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>

#if (MTKCAM_HAVE_AEE_FEATURE == 1)
#include <aee.h>
#endif


CAM_ULOG_DECLARE_MODULE_ID(MOD_VHDR_HAL);


#undef MY_LOGD
#undef MY_LOGD
#undef MY_LOGI
#undef MY_LOGW
#undef MY_LOGE
#undef MY_LOGA
#undef MY_TRACE_API_LIFE
#undef MY_TRACE_FUNC_LIFE
#undef MY_TRACE_TAG_LIFE
#undef MY_LOGD_IF
#undef MY_LOGI_IF
#undef MY_LOGW_IF
#undef MY_LOGE_IF


#define MY_LOGD(fmt, arg...)       CAM_ULOGMD("[%s]" fmt, __func__, ##arg)
#define MY_LOGI(fmt, arg...)       CAM_ULOGMI("[%s]" fmt, __func__, ##arg)
#define MY_LOGW(fmt, arg...)       CAM_ULOGMW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)       CAM_ULOGME("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)
#define MY_TRACE_API_LIFE()        CAM_ULOGM_APILIFE()
#define MY_TRACE_FUNC_LIFE()       CAM_ULOGM_FUNCLIFE()
#define MY_TRACE_TAG_LIFE(name)    CAM_ULOGM_TAGLIFE(name)

#if (MTKCAM_HAVE_AEE_FEATURE == 1)
static const unsigned int VHDR_AEE_DB_FLAGS = DB_OPT_NE_JBT_TRACES | DB_OPT_PROCESS_COREDUMP | DB_OPT_PROC_MEM | DB_OPT_PID_SMAPS |
                                              DB_OPT_LOW_MEMORY_KILLER | DB_OPT_DUMPSYS_PROCSTATS | DB_OPT_FTRACE;

#define MY_LOGA(fmt, arg...)                                                            \
        do {                                                                            \
            android::String8 const str = android::String8::format(fmt, ##arg);          \
            MY_LOGE("ASSERT(%s)", str.c_str());                                         \
            aee_system_exception(LOG_TAG, NULL, VHDR_AEE_DB_FLAGS, str.c_str());        \
            raise(SIGKILL);                                                             \
        } while(0)
#else
#define MY_LOGA(fmt, arg...)                                                            \
        do {                                                                            \
            android::String8 const str = android::String8::format(fmt, ##arg);          \
            MY_LOGE("ASSERT(%s)", str.c_str());                                         \
        } while(0)
#endif



#define MY_LOGD_IF(cond, ...)       do { if ( (cond) >= (VHDR_DEBUG_LEVEL) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) >= (VHDR_INFO_LEVEL) )  { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) >= (VHDR_WARN_LEVEL) )  { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) >= (VHDR_ERROR_LEVEL) ) { MY_LOGE(__VA_ARGS__); } }while(0)


#define VHDR_HAL_NAME "VHDRHal"


template <> VHdrHalObj<0>* VHdrHalObj<0>::spInstance = 0;
template <> VHdrHalObj<1>* VHdrHalObj<1>::spInstance = 0;
template <> VHdrHalObj<2>* VHdrHalObj<2>::spInstance = 0;
template <> VHdrHalObj<3>* VHdrHalObj<3>::spInstance = 0;
template <> VHdrHalObj<4>* VHdrHalObj<4>::spInstance = 0;
template <> VHdrHalObj<5>* VHdrHalObj<5>::spInstance = 0;
template <> VHdrHalObj<6>* VHdrHalObj<6>::spInstance = 0;
template <> VHdrHalObj<7>* VHdrHalObj<7>::spInstance = 0;
template <> VHdrHalObj<8>* VHdrHalObj<8>::spInstance = 0;
template <> VHdrHalObj<9>* VHdrHalObj<9>::spInstance = 0;

template <> Mutex VHdrHalObj<0>::s_instMutex(::Mutex::PRIVATE);
template <> Mutex VHdrHalObj<1>::s_instMutex(::Mutex::PRIVATE);
template <> Mutex VHdrHalObj<2>::s_instMutex(::Mutex::PRIVATE);
template <> Mutex VHdrHalObj<3>::s_instMutex(::Mutex::PRIVATE);
template <> Mutex VHdrHalObj<4>::s_instMutex(::Mutex::PRIVATE);
template <> Mutex VHdrHalObj<5>::s_instMutex(::Mutex::PRIVATE);
template <> Mutex VHdrHalObj<6>::s_instMutex(::Mutex::PRIVATE);
template <> Mutex VHdrHalObj<7>::s_instMutex(::Mutex::PRIVATE);
template <> Mutex VHdrHalObj<8>::s_instMutex(::Mutex::PRIVATE);
template <> Mutex VHdrHalObj<9>::s_instMutex(::Mutex::PRIVATE);

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
VHdrHalImp::isSupport()
{
    switch( mHDRHalMode )
    {
    case MTK_HDR_FEATURE_HDR_HAL_MODE_OFF:
    case MTK_HDR_FEATURE_HDR_HAL_MODE_MVHDR:
    case MTK_HDR_FEATURE_HDR_HAL_MODE_MSTREAM_CAPTURE:
    case MTK_HDR_FEATURE_HDR_HAL_MODE_MSTREAM_PREVIEW:
    case MTK_HDR_FEATURE_HDR_HAL_MODE_MSTREAM_CAPTURE_PREVIEW:
        return MTRUE;
    default:
        return MFALSE;
    }
    return MFALSE;
}

/*******************************************************************************
*  Get AE Target Mode
********************************************************************************/
eAETargetMODE
VHdrHalImp::getCurrentAEMode()
{
    if( (g_disable & VHDR_DISABLE_AE_TARGET) )
    {
        return AE_MODE_NORMAL;
    }
    switch( mHDRHalMode )
    {
    case MTK_HDR_FEATURE_HDR_HAL_MODE_MVHDR:
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
    MY_LOGD("%s",userName);
    return VHdrHalImp::GetInstance(aSensorIdx);
}

/*******************************************************************************
*
********************************************************************************/
VHdrHal *VHdrHalImp::GetInstance(const MUINT32 &aSensorIdx)
{
    MY_LOGD( "aSensorIdx(%u)",aSensorIdx);

    switch(aSensorIdx)
    {
        case 0 : return VHdrHalObj<0>::GetInstance();
        case 1 : return VHdrHalObj<1>::GetInstance();
        case 2 : return VHdrHalObj<2>::GetInstance();
        case 3 : return VHdrHalObj<3>::GetInstance();
        case 4 : return VHdrHalObj<4>::GetInstance();
        case 5 : return VHdrHalObj<5>::GetInstance();
        case 6 : return VHdrHalObj<6>::GetInstance();
        case 7 : return VHdrHalObj<7>::GetInstance();
        case 8 : return VHdrHalObj<8>::GetInstance();
        case 9 : return VHdrHalObj<9>::GetInstance();
        default :
            MY_LOGD( "current limit is 10 sensors, use 0");
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
    MY_LOGD( "%s",userName);
}

/*******************************************************************************
*
********************************************************************************/
VHdrHalImp::VHdrHalImp(const MUINT32 &aSensorIdx)
    : VHdrHal()
    , mUsers(0)
    , mHDRHalMode(0)
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
    MY_LOGD( "VHDR Hal destrcutor");

    mHDRHalMode = 0;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 VHdrHalImp::Init(const MUINT32 &aMode)
{
    MY_TRACE_FUNC_LIFE();

    Mutex::Autolock lock(mLock);


    //====== Check Reference Count ======

    if(mUsers > 0)
    {
        android_atomic_inc(&mUsers);
        MY_LOGD( "mSensorIdx(%u) has %d users",mSensorIdx,mUsers);
        return VHDR_RETURN_NO_ERROR;
    }

    MINT32 err = VHDR_RETURN_NO_ERROR;


    // ======== Debug Property Check =========

    g_debugDump = ::property_get_int32(VHDR_DEBUG_PROP, 0);

    g_disable = ::property_get_int32(VHDR_DISABLE_PROP, 0);

    // force open AE Target
    MINT32 forceAE = ::property_get_int32("vendor.debug.vhdr.force_ae_mode", 0);

    MY_LOGI( "mSensorIdx(%u), VhdrHalMode(%u) init",mSensorIdx,aMode);
    MY_LOGD("debug dump = %d, disable = %d, forceAE = %d", g_debugDump, g_disable, forceAE);

    // ===== force to set AE ====
    if(forceAE != 0){
        MY_LOGD( "force AE Target Mode %d",forceAE);
        m_p3aHal = MAKE_Hal3A(static_cast<MINT32>(mSensorIdx),VHDR_HAL_NAME);
        if( m_p3aHal != NULL ) {
            m_p3aHal->send3ACtrl(E3ACtrl_SetAETargetMode, forceAE, 0);
        }
    }

    //====== Save Mode ======

    mHDRHalMode = aMode;

    if( !isSupport() )
    {
        MY_LOGA("Error: Unknwon HDRHalMode(%u)"
                "\nCRDISPATCH_KEY:VHDR unknown mode", mHDRHalMode);
    }

    //====== Create Sensor Object ======

    m_pHalSensorList = MAKE_HalSensorList();
    if(m_pHalSensorList == NULL)
    {
        MY_LOGA("Error: Cannot get HalSensorList"
                "\nCRDISPATCH_KEY:VHDR no HalSensorList");
        return VHDR_RETURN_INVALID_DRIVER;
    }

    //====== Get Sensor Static Info ======

    err = GetSensorStaticInfo();
    if(err != VHDR_RETURN_NO_ERROR)
    {
        MY_LOGE("GetSensorStaticInfo fail(%d)", err);
        return VHDR_RETURN_API_FAIL;
    }

    android_atomic_inc(&mUsers);
    return VHDR_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 VHdrHalImp::ConfigVHdr(const VHDR_HAL_CONFIG_DATA &aConfigData)
{
    MY_TRACE_FUNC_LIFE();

    MINT32 err = VHDR_RETURN_NO_ERROR;
    (void)aConfigData; // currently this parameter is not used yet.

    //====== Get Sensor Dynamic Info ======

    err = GetSensorDynamicInfo();
    if(err != VHDR_RETURN_NO_ERROR)
    {
        MY_LOGE("GetSensorDynamicInfo fail(%d)", err);
        return VHDR_RETURN_API_FAIL;
    }

    //====== Create 3A Object ======

    m_p3aHal = MAKE_Hal3A(static_cast<MINT32>(mSensorIdx),VHDR_HAL_NAME);
    if(m_p3aHal == NULL)
    {
        MY_LOGE("m_p3aHal create fail");
        return VHDR_RETURN_NULL_OBJ;
    }

    //====== Set AE Target Mode ======

    if( ! (g_disable & VHDR_DISABLE_AE_TARGET) ){
        MY_LOGI( "setAETargetMode %d", getCurrentAEMode());
        m_p3aHal->send3ACtrl(E3ACtrl_SetAETargetMode, getCurrentAEMode(), 0);
    }else{
        MY_LOGD( "disable setAETargetMode");
    }

    return VHDR_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 VHdrHalImp::Uninit()
{
    MY_TRACE_FUNC_LIFE();

    Mutex::Autolock lock(mLock);

    //====== Check Reference Count ======

    if(mUsers <= 0)
    {
        MY_LOGD( "mSensorIdx(%u) has 0 user",mSensorIdx);
        return VHDR_RETURN_NO_ERROR;
    }

    //====== Uninitialize ======

    android_atomic_dec(&mUsers);    //decrease referebce count

    if(mUsers == 0)
    {
        MY_LOGD("mSensorIdx(%u),mode(%u) uninit",mSensorIdx,mHDRHalMode);

        //====== Destroy Sensor Object ======

        if(m_pHalSensorList != NULL)
        {
            m_pHalSensorList = NULL;
        }

        // =====  set AE Target Mode back to Normal =======
        if(m_p3aHal != NULL)
        {
            MY_LOGD( "set AE to normal");
            m_p3aHal->send3ACtrl(E3ACtrl_SetAETargetMode, AE_MODE_NORMAL, 0);
            m_p3aHal->destroyInstance(VHDR_HAL_NAME);
            m_p3aHal = NULL;
        }

        //====== Set State ======
        mHDRHalMode = 0;
    }
    else
    {
        MY_LOGD( "mSensorIdx(%u) has %d users",mSensorIdx,mUsers);
    }

    return VHDR_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 VHdrHalImp::GetSensorStaticInfo()
{
    SensorStaticInfo staticInfo;

    mSensorDev = m_pHalSensorList->querySensorDevIdx(mSensorIdx);
    m_pHalSensorList->querySensorStaticInfo(mSensorDev,&staticInfo);

    MY_LOGI( "Sensor Static Info, previewW,H = %d,%d", staticInfo.previewWidth , staticInfo.previewHeight);
    mSensorInfo.setStaticInfo(staticInfo);

    return VHDR_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 VHdrHalImp::GetSensorDynamicInfo()
{
    SensorDynamicInfo dynamicInfo;

    m_pHalSensor = m_pHalSensorList->createSensor(VHDR_HAL_NAME,1,&mSensorIdx);
    if(m_pHalSensor == NULL)
    {
        MY_LOGE("m_pHalSensorList->createSensor fail");
        return VHDR_RETURN_API_FAIL;
    }

    if(MFALSE == m_pHalSensor->querySensorDynamicInfo(mSensorDev,&dynamicInfo))
    {
        MY_LOGE("querySensorDynamicInfo fail");
        return VHDR_RETURN_API_FAIL;
    }

    mSensorInfo.setDynamicInfo(dynamicInfo);

    m_pHalSensor->destroyInstance(VHDR_HAL_NAME);
    m_pHalSensor = NULL;

    return VHDR_RETURN_NO_ERROR;
}

#endif //ifdef MTK_CAM_VHDR_SUPPORT end
