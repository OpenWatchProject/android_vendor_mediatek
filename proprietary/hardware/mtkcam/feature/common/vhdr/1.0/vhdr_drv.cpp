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
* @file vhdr_drv.cpp
*
* VHDR Driver Source File
*
*/

#define LOG_TAG "VHDRDrv"

#include <utils/Errors.h>
#include <utils/Mutex.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <cutils/atomic.h>
#include <cutils/properties.h>
#include <errno.h>

using namespace std;
using namespace android;

#include <mtkcam/drv/IHalSensor.h>

using namespace NSCam;

#include <mtkcam/utils/std/common.h>
using namespace NSCam::Utils;

#include "vhdr_drv_imp.h"
#include "vhdr_debug.h"
using namespace NS3Av3;

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


/*******************************************************************************
*
********************************************************************************/
static MINT32 g_debugDump = 0;

/*******************************************************************************
*
********************************************************************************/
VHdrDrv *VHdrDrv::CreateInstance(const MUINT32 &aSensorIdx)
{
    return VHdrDrvImp::GetInstance(aSensorIdx);
}

/*******************************************************************************
*
********************************************************************************/
VHdrDrv *VHdrDrvImp::GetInstance(const MUINT32 &aSensorIdx)
{
    VHDR_LOG("aSensorIdx(%u)",aSensorIdx);
    return new VHdrDrvImp(aSensorIdx);
}

/*******************************************************************************
*
********************************************************************************/
MVOID VHdrDrvImp::DestroyInstance()
{
    VHDR_LOG("+");
	delete this;
}

/*******************************************************************************
*
********************************************************************************/
VHdrDrvImp::VHdrDrvImp(const MUINT32 &aSensorIdx)
    : VHdrDrv()
    , mUsers(0)
    , mSensorTg(0)
    , mSensorIdx(aSensorIdx)
    , mSensorDev(0)
    , mIspMgr(MAKE_IspMgr())
{
}

/*******************************************************************************
*
********************************************************************************/
MINT32 VHdrDrvImp::Init()
{
    Mutex::Autolock lock(mLock);

    //====== Reference Count ======

    if(mUsers > 0)
    {
        android_atomic_inc(&mUsers);
        VHDR_LOG("mSensorIdx(%u) has %d users",mSensorIdx,mUsers);
        return VHDR_RETURN_NO_ERROR;
    }

    MINT32 err = VHDR_RETURN_NO_ERROR;

    //====== Dynamic Debug ======

    g_debugDump = ::property_get_int32(VHDR_DEBUG_PROP, 0);

    VHDR_LOG("mSensorIdx(%u) init",mSensorIdx);

    android_atomic_inc(&mUsers);    // increase reference count

    MY_LOGD_IF(g_debugDump, "-");
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 VHdrDrvImp::Uninit()
{
    Mutex::Autolock lock(mLock);

    //====== Reference Count ======

    if(mUsers <= 0) // No more users
    {
        VHDR_LOG("mSensorIdx(%u) has 0 user",mSensorIdx);
        return VHDR_RETURN_NO_ERROR;
    }

    // >= one user
    android_atomic_dec(&mUsers);

    MINT32 err = VHDR_RETURN_NO_ERROR;

    if(mUsers == 0)
    {
        VHDR_LOG("mSensorIdx(%u) uninit",mSensorIdx);

        //====== Rest Member Variable ======

        mUsers = 0;
        mSensorIdx = 0;
        mSensorDev = 0;
        mSensorTg  = CAM_TG_NONE;

    }
    else
    {
         VHDR_LOG("mSensorIdx(%u) has %d users",mSensorIdx,mUsers);
    }

    MY_LOGD_IF(g_debugDump, "X");
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MVOID VHdrDrvImp::SetSensorInfo(const MUINT32 &aSensorDev,const MUINT32 &aSensorTg)
{
    mSensorDev = aSensorDev;
    mSensorTg  = aSensorTg;
    VHDR_LOG( "(dev,tg)=(%u,%u)",mSensorDev,mSensorTg);
}

/*******************************************************************************
*
********************************************************************************/
MVOID VHdrDrvImp::ConfigRmg_ivHDR(const MUINT32 &aLeFirst)
{
    // 0 : first two rows are SE,  1 : first two rows are LE
    VHDR_INF( "(leFirst,dev,tg)=(%u,%u,%u)",aLeFirst,mSensorDev,mSensorTg);
    RMG_Config_Param rmgPara;
    rmgPara.iEnable = MTRUE;
    rmgPara.leFirst = aLeFirst;
    rmgPara.zEnable = MFALSE;
    rmgPara.zPattern = 0;

    if  ( mIspMgr ) {
        mIspMgr->configRMG_RMG2(mSensorIdx, rmgPara);
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID VHdrDrvImp::ConfigRmg_zvHDR(const MUINT32 &zPattern)
{
    // 0 : first two rows are SE,  1 : first two rows are LE
    VHDR_INF( "(zPattern,dev,tg)=(%u,%u,%u)",zPattern,mSensorDev,mSensorTg);
    RMG_Config_Param rmgPara;
    rmgPara.iEnable = MFALSE;
    rmgPara.leFirst = 0;
    rmgPara.zEnable = MTRUE;
    rmgPara.zPattern = zPattern;

    if  ( mIspMgr ) {
        mIspMgr->configRMG_RMG2(mSensorIdx, rmgPara);
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID VHdrDrvImp::EnableRmg(const MBOOL &aEn)
{
    VHDR_LOG( "en:%d mSensorIdx:%d",aEn, mSensorIdx);

    if  ( mIspMgr ) {
        mIspMgr->setRMGEnable(mSensorIdx, aEn);

        // enable debug log, we don't want to add API, so we add here.
        if  ( mIspMgr ) {
            if(aEn){
                mIspMgr->setRMGDebug(mSensorIdx, g_debugDump);
            }else{
                mIspMgr->setRMGDebug(mSensorIdx , 0);
            }
        }
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID VHdrDrvImp::EnableRmm(const MBOOL &aEn)
{
    VHDR_LOG( "en:%d mSensorIdx:%d",aEn, mSensorIdx);

    if  ( mIspMgr ) {
        mIspMgr->setRMMEnable(mSensorIdx, aEn);

        // enable debug log, we don't want to add API, so we add here.
        if(aEn){
            mIspMgr->setRMMDebug(mSensorIdx, g_debugDump);
        }else{
            mIspMgr->setRMMDebug(mSensorIdx , 0);
        }
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID VHdrDrvImp::ConfigCpnDcpn_zvHDR()
{
    MBOOL zEnable = MTRUE;

    if  ( mIspMgr ) {
        // config CPN/DCPN
        mIspMgr->configCPN_CPN2(mSensorIdx, zEnable);
        mIspMgr->configDCPN_DCPN2(mSensorIdx, zEnable);

        // set CPN/DCPN Debug
        mIspMgr->setCPNDebug(mSensorIdx, g_debugDump);
        mIspMgr->setDCPNDebug(mSensorIdx, g_debugDump);
    }
}


