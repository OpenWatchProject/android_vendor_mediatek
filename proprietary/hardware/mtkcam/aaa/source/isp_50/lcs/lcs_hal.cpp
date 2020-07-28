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

//! \file lcs_hal.cpp
#define LOG_TAG "LCSHal"
#include <stdlib.h>
#include <stdio.h>
#include <utils/threads.h>
#include <cutils/properties.h>
#include <sys/prctl.h>
#include <semaphore.h>
#include <pthread.h>

using namespace android;

#include <mtkcam/def/PriorityDefs.h>
#include <mtkcam/drv/IHalSensor.h>

using namespace NSCam;

#include <mtkcam/aaa/IHal3A.h>
using namespace NS3Av3;

#include <mtkcam/utils/std/common.h>
#include <mtkcam/utils/std/Log.h>
using namespace NSCam::Utils;

#include "ae_param.h"
#include "isp_tuning_cam_info.h"

#include "lcs_drv.h"
#include "lcs_hal_imp.h"
#include <utils/Atomic.h>
#include <array>

/*******************************************************************************
*
********************************************************************************/

#undef __func__
#define __func__ __FUNCTION__


#define LCS_LOG(fmt, arg...)    CAM_LOGD("[%s]" fmt, __func__, ##arg)
#define LCS_INF(fmt, arg...)    CAM_LOGI("[%s]" fmt, __func__, ##arg)
#define LCS_WRN(fmt, arg...)    CAM_LOGW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define LCS_ERR(fmt, arg...)    CAM_LOGE("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)


#define MY_LOGD_IF(cond, ...)       do { if ( (cond) >= (2) ) { LCS_LOG(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) >= (1) ) { LCS_INF(__VA_ARGS__); } }while(0)
//#define MY_LOGW_IF(cond, ...)       do { if ( (cond) >= (1) ) { LCS_WRN(__VA_ARGS__); } }while(0)
//#define MY_LOGE_IF(cond, ...)       do { if ( (cond) >= (0) ) { LCS_ERR(__VA_ARGS__); } }while(0)

#define LCS_HAL_NAME "LCSHal"

static MINT32 g_debugDump = 0;
/**
* Dump LCSO buffer every {g_dumpLCSO} buffers.
* Ex: g_dumpLCSO = 10,  it will dump LCSO buffer every 10 frame.
*/
static MINT32 g_dumpLCSO = 0;

#define MY_INST NS3Av3::INST_T<LcsHalImp>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

/*******************************************************************************
*
********************************************************************************/
LcsHal *LcsHal::CreateInstance(char const *userName, const MUINT32 &aSensorIdx)
{
    LCS_LOG("%s",userName);
    return LcsHalImp::GetInstance(aSensorIdx);
}

/*******************************************************************************
*
********************************************************************************/
LcsHal *LcsHalImp::GetInstance(const MUINT32 &aSensorIdx)
{
    LCS_LOG( "aSensorIdx(%u)",aSensorIdx);

    if(aSensorIdx >= SENSOR_IDX_MAX || aSensorIdx < 0) {
        CAM_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, aSensorIdx);
        return NULL;
    }

    MY_INST& rSingleton = gMultiton[aSensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<LcsHalImp>(aSensorIdx);
    } );

    return rSingleton.instance.get();
}

/*******************************************************************************
*
********************************************************************************/
MVOID LcsHalImp::DestroyInstance(char const *userName)
{
    // We don't delete instance because we have LCSO Buffers.
    // If we deallocate buffers when P2 Thread not done yet, it may cause error.
    LCS_LOG( "%s",userName);
}

/*******************************************************************************
*
********************************************************************************/
LcsHalImp::LcsHalImp(const MUINT32 &aSensorIdx) : LcsHal()
{
    mUsers = 0;

	m_pLcsDrv = NULL;
    m_pHalSensorList = NULL;
    m_pHalSensor = NULL;
    m_p3aHal = NULL;
    m_pLcsCb3A = NULL;

    mSensorIdx = aSensorIdx;
    mSensorDev = 0;
    memset(&mSensorDynamicInfo, 0, sizeof(SensorDynamicInfo));

}

/*******************************************************************************
*ILCS_NOT_READY
********************************************************************************/
LcsHalImp::~LcsHalImp()
{
    LCS_LOG( "LCS Hal destrcutor");
}

/*******************************************************************************
*
********************************************************************************/
MINT32 LcsHalImp::Init()
{
    Mutex::Autolock lock(mLock);

    //====== Check Reference Count ======

    if(mUsers > 0)
    {
        android_atomic_inc(&mUsers);
        LCS_LOG( "mSensorIdx(%u) has %d users",mSensorIdx,mUsers);
        return LCS_RETURN_NO_ERROR;
    }

    m_pLcsCb3A = new LcsHalCallBack(this);

    // ======== Debug Property Check =========

    g_debugDump = ::property_get_int32("vendor.debug.lcs.dump", 0);

    g_dumpLCSO = ::property_get_int32("vendor.debug.lcso.dump", 0);

    // force open AE Target
    MINT32 forceAE __unused = ::property_get_int32("vendor.debug.vhdr.force_ae_mode", 0);

    LCS_INF( "mSensorIdx(%u)  init",mSensorIdx);
    LCS_LOG("debug dump = %d", g_debugDump);

    //====== Create Sensor Object ======

    m_pHalSensorList = MAKE_HalSensorList();
    if(m_pHalSensorList == NULL)
    {
        LCS_ERR("IHalSensorList::get fail");
        goto create_fail_exit;
    }

    //====== Create LCS Driver ======
    m_pLcsDrv = LcsDrv::CreateInstance(mSensorIdx);

    if(m_pLcsDrv == NULL)
    {
        LCS_ERR("LcsDrv::createInstance fail");
        goto create_fail_exit;
    }

    android_atomic_inc(&mUsers);
    MY_LOGD_IF(g_debugDump, "-");
    return LCS_RETURN_NO_ERROR;

create_fail_exit:

    if(m_pLcsDrv != NULL)
    {
        m_pLcsDrv->Uninit();
        m_pLcsDrv->DestroyInstance();
        m_pLcsDrv = NULL;
    }

    if(m_pHalSensorList != NULL)
    {
        m_pHalSensorList = NULL;
    }

    MY_LOGD_IF(g_debugDump, "-");
    return LCS_RETURN_INVALID_DRIVER;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 LcsHalImp::Uninit()
{
    Mutex::Autolock lock(mLock);

    //====== Check Reference Count ======

    if(mUsers <= 0)
    {
        LCS_LOG( "mSensorIdx(%u) has 0 user",mSensorIdx);
        return LCS_RETURN_NO_ERROR;
    }

    //====== Uninitialize ======
    android_atomic_dec(&mUsers);    //decrease referebce count

    if(mUsers == 0)
    {
       LCS_LOG("mSensorIdx(%u), uninit",mSensorIdx);

        //====== Destroy Sensor Object ======

        if(m_pHalSensorList != NULL)
        {
            m_pHalSensorList = NULL;
        }

        //====== Delete 3A  ======

            if(m_p3aHal != NULL)
            {

                m_p3aHal->detachCb(IHal3ACb::eID_NOTIFY_LCS_ISP_PARAMS, m_pLcsCb3A);

                m_p3aHal->destroyInstance(LCS_HAL_NAME);
                m_p3aHal = NULL;

                if(m_pLcsCb3A != NULL)
                {
                    delete m_pLcsCb3A;
                    m_pLcsCb3A = NULL;
                }
            }

            //====== Destroy LCS Driver ======

            if(m_pLcsDrv != NULL)
            {

                m_pLcsDrv->Uninit();
                m_pLcsDrv->DestroyInstance();
                m_pLcsDrv = NULL;

            }

    }
    else
    {
        LCS_LOG( "mSensorIdx(%u) has %d users",mSensorIdx,mUsers);
    }

    MY_LOGD_IF(g_debugDump, "-");
    return LCS_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 LcsHalImp::GetSensorInfo()
{
    MY_LOGD_IF(g_debugDump, "mSensorIdx(%u)",mSensorIdx);

    mSensorDev = m_pHalSensorList->querySensorDevIdx(mSensorIdx);
    m_pHalSensorList->querySensorStaticInfo(mSensorDev,&mSensorStaticInfo);

    m_pHalSensor = m_pHalSensorList->createSensor(LCS_HAL_NAME,1,&mSensorIdx);
    if(m_pHalSensor == NULL)
    {
        MY_LOGE_IF(g_debugDump, "m_pHalSensorList->createSensor fail");
        return LCS_RETURN_API_FAIL;
    }

    if(MFALSE == m_pHalSensor->querySensorDynamicInfo(mSensorDev,&mSensorDynamicInfo))
    {
        MY_LOGE_IF(g_debugDump, "querySensorDynamicInfo fail");
        return LCS_RETURN_API_FAIL;
    }

    LCS_INF( "Sensor Static Info, previewW,H = %d,%d,",mSensorStaticInfo.previewWidth , mSensorStaticInfo.previewHeight);

    m_pHalSensor->destroyInstance(LCS_HAL_NAME);
    m_pHalSensor = NULL;

    return LCS_RETURN_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
MINT32 LcsHalImp::ConfigLcsHal(const LCS_HAL_CONFIG_DATA &/*aConfigData*/)
{

    MINT32 err = LCS_RETURN_NO_ERROR;

    //mLcs_outW = aConfigData.lcsOutWidth;
    //mLcs_outH = aConfigData.lcsOutHeight;
    //LCS_INF("Lcs out size (%d,%d)", mLcs_outW, mLcs_outH);

    //====== Get Sensor Info ======

    err = GetSensorInfo();
    if(err != LCS_RETURN_NO_ERROR)
    {
        LCS_ERR("GetSensorInfo fail(%d)", err);
        return LCS_RETURN_API_FAIL;
    }

    //====== Create 3A Object ======

    m_p3aHal = MAKE_Hal3A(static_cast<MINT32>(mSensorIdx),LCS_HAL_NAME);
    if(m_p3aHal == NULL)
    {
        LCS_ERR("m_p3aHal create fail");
        return LCS_RETURN_NULL_OBJ;
    }

    //> set to LCS driver

        m_pLcsDrv->SetSensorInfo(mSensorDev,mSensorDynamicInfo.TgInfo); // Jou-Feng change move it to Init()

        //====== Add 3A Callback ======

        m_p3aHal->attachCb(IHal3ACb::eID_NOTIFY_LCS_ISP_PARAMS ,m_pLcsCb3A);

    err = m_pLcsDrv->Init();
    if(err != LCS_RETURN_NO_ERROR)
    {
        LCS_ERR("LcsDrv::Init fail");
        return LCS_RETURN_API_FAIL;
    }

        err = m_pLcsDrv->ConfigLcs(); // also register P1 Tuning Notify callback for LCS setting
        if(err != LCS_RETURN_NO_ERROR)
        {
            LCS_ERR("ConfigLcs fail(%d)", err);
            return LCS_RETURN_API_FAIL;
        }

    MY_LOGD_IF(g_debugDump, "-");
    return LCS_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
IHal3A::E_VER LcsHalImp::ConvertCameraVer(const LCS_CAMERA_VER_ENUM &aVer)
{
    switch(aVer)
    {
        case LCS_CAMERA_VER_1 : return IHal3A::E_Camera_1;
        case LCS_CAMERA_VER_3 : return IHal3A::E_Camera_3;
        default : MY_LOGW_IF(g_debugDump, "unknown ver(%d) return E_Camera_1",aVer);
                  return IHal3A::E_Camera_1;
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID LcsHalImp::Set_LcsInParam(MINT32 i4FrmId, MINTPTR pLCSInfo)
{
        if(m_pLcsDrv != NULL){
            m_pLcsDrv->updateLCSList_In( i4FrmId, *(ISP_LCS_IN_INFO_T*)pLCSInfo);
        }
}

/*******************************************************************************
*
********************************************************************************/
LcsHalCallBack::LcsHalCallBack(MVOID *arg)
{
    m_pLcsHalImp = arg;
}

/*******************************************************************************
*
********************************************************************************/
LcsHalCallBack::~LcsHalCallBack()
{
    m_pLcsHalImp = NULL;
}

/*******************************************************************************
*
********************************************************************************/
void LcsHalCallBack::doNotifyCb(MINT32 _msgType, MINTPTR _ext1,MINTPTR _ext2,MINTPTR /*_ext3*/)
{
    MY_LOGD_IF(g_debugDump, "msgType(%d)",_msgType);
    LcsHalImp *_this = reinterpret_cast<LcsHalImp *>(m_pLcsHalImp);

    if(_msgType == IHal3ACb::eID_NOTIFY_LCS_ISP_PARAMS)
    {
        _this->Set_LcsInParam(_ext1, _ext2);
    }

    MY_LOGD_IF(g_debugDump, "-");
}
