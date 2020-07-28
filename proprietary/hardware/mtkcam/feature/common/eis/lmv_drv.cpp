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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

//! \file  eis_drv.cpp
#include <cutils/properties.h>
#include <utils/Condition.h>
#include <cutils/atomic.h>

#include <mtkcam/feature/eis/eis_type.h>
#include <mtkcam/drv/iopipe/CamIO/Cam_Notify.h>
#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>
#include <mtkcam/drv/IHalSensor.h>

#include "lmv_drv_imp.h"

using namespace NSCam;
using namespace NSCam::NSIoPipe::NSCamIOPipe;
using namespace NSCam::NSIoPipe;


/****************************************************************************************
* Define Value
****************************************************************************************/

#define EIS_DRV_DEBUG

#undef LOG_TAG
#define LOG_TAG "LMVDrv"
#include <mtkcam/utils/std/Log.h>

#ifdef EIS_DRV_DEBUG

#undef __func__
#define __func__ __FUNCTION__

#define LMV_LOG(fmt, arg...)    CAM_LOGD("[%s]" fmt, __func__, ##arg)
#define LMV_INF(fmt, arg...)    CAM_LOGI("[%s]" fmt, __func__, ##arg)
#define LMV_WRN(fmt, arg...)    CAM_LOGW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define LMV_ERR(fmt, arg...)    CAM_LOGE("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

#else

#define LMV_LOG(a,...)
#define LMV_INF(a,...)
#define LMV_WRN(a,...)
#define LMV_ERR(a,...)

#endif

#define LMV_DRV_NAME "LMVDrv"

/*******************************************************************************
* Global variable
********************************************************************************/
static MINT32 g_debugDump = 0;
static MINT32 g_forceMaxSearchRange = LMV_MAX_GMV_DEFAULT;

struct LMV_STEP_WIN_SETTING {
    MUINT8      name[32];           // name
    MUINT32     img_w;              // width
    MUINT32     img_h;              // height
    MUINT32     eis_op_step_h;      // vertical step
    MUINT32     eis_op_step_v;      // horizontal step
    MUINT32     eis_num_h_win;      // vertical win
    MUINT32     eis_num_v_win;      // horizontal win
    MUINT32     max_range;          // max search range
};

static const LMV_STEP_WIN_SETTING lmv_step_win_setting[] = {
    {" 4K2K_REC", 3264,  2200,  4,   4,  4,  8,  LMV_MAX_GMV_64},
    {"4K2K_TWIN", 2304,  2592,  4,   4,  4,  7,  LMV_MAX_GMV_64},
    {"  FHD_REC", 2400,  1350,  4,   4,  4,  4,  LMV_MAX_GMV_64},
    {"  FHD_PRE", 1920,  1080,  4,   4,  3,  3,  LMV_MAX_GMV_64},
    {" 720P_REC", 1600,   900,  2,   2,  4,  6,  LMV_MAX_GMV_32},
    {" 720P_PRE", 1280,   720,  2,   2,  4,  4,  LMV_MAX_GMV_32},
    {"  DEFAULT",    0,     0,  1,   1,  2,  3,  LMV_MAX_GMV_DEFAULT},
};

/*******************************************************************************
*
********************************************************************************/
LMVDrv* LMVDrv::CreateInstance(const MUINT32 &aSensorIdx)
{
    return LMVDrvImp::GetInstance(aSensorIdx);
}

/*******************************************************************************
*
********************************************************************************/
LMVDrv* LMVDrvImp::GetInstance(const MUINT32 &aSensorIdx)
{
    LMV_LOG("aSensorIdx(%u)",aSensorIdx);

    return new LMVDrvImp(aSensorIdx);
}

/*******************************************************************************
*
********************************************************************************/
MVOID LMVDrvImp::DestroyInstance()
{
    LMV_LOG("+");
    delete this;
}

/*******************************************************************************
*
********************************************************************************/
LMVDrvImp::LMVDrvImp(const MUINT32 &aSensorIdx) : LMVDrv()
{
    // reference count
    mUsers = 0;

    // INormalPipe object
    m_pNormalPipeModule = NULL;
    m_pNormalPipe = NULL;

    // variable for EIS algo and EIS Hal
    mIsConfig   = 0;
    mIsFirst    = 1;
    mIs2Pixel   = 0;
    mTotalMBNum = 0;
    mImgWidth   = 0;
    mImgHeight  = 0;
    mEisDivH    = 0;
    mEisDivV    = 0;
    mMaxGmv     = LMV_MAX_GMV_DEFAULT;
    mSensorIdx  = aSensorIdx;
    mSensorTg = 0;
    mSensorType = EIS_NULL_SENSOR;
    mSensor_Width = 0;
    mSensor_Height = 0;
    mRRZ_in_Width  = 0;
    mRRZ_in_Height = 0;
    mRrz_crop_Width = 0;
    mRrz_crop_Height = 0;
    mRrz_crop_X = 0;
    mRrz_crop_Y = 0;
    mRrz_scale_Width = 0;
    mRrz_scale_Height = 0;
    mEisoIsFirst  = 1;
    mEisHwSupport = MTRUE;
    mTsForAlgoDebug = 0;
    mHasSGG2 = MTRUE;

    // Pass1 HW setting callback
    m_pLMVP1Cb = NULL;
    m_pSGG2P1Cb = NULL;

}

/*******************************************************************************
*
********************************************************************************/
LMVDrvImp::~LMVDrvImp()
{
}

/*******************************************************************************
*
********************************************************************************/
MINT32 LMVDrvImp::Init()
{
    MINT32 err = EIS_RETURN_NO_ERROR;

    //====== Reference Count ======

    android::Mutex::Autolock lock(mLock);

    if(mUsers > 0)
    {
        android_atomic_inc(&mUsers);
        LMV_LOG("mSensorIdx(%u) has %d users",mSensorIdx,mUsers);
        return EIS_RETURN_NO_ERROR;
    }

    //====== Dynamic Debug ======

#if (EIS_DEBUG_FLAG)

    LMV_INF("EIS_DEBUG_FLAG on");
    g_debugDump = 1;

#else

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.LMVDrv.dump", value, "0");
    g_debugDump = atoi(value);

#endif

#if defined(MTKCAM_LMV_ISP30)
    mHasSGG2 = MFALSE;
#endif
    g_forceMaxSearchRange = ::property_get_int32("vendor.debug.LMVDrv.force.searchRange", LMV_MAX_GMV_32);

    //====== Member variable ======

    LMV_LOG("mSensorIdx(%u) init, SGG2(%d)", mSensorIdx, mHasSGG2);

    //====== Create INormalPipe Object ======
    m_pNormalPipeModule = INormalPipeModule::get();
    if  ( ! m_pNormalPipeModule ) {
        LMV_ERR("INormalPipeModule::get() fail");
        return EIS_RETURN_NULL_OBJ;
    }

    {
        int status = 0;

        //  Select version
        size_t count = 0;
        MUINT32 const* version = NULL;
        status = m_pNormalPipeModule->get_sub_module_api_version(&version, &count, mSensorIdx);
        if  ( status < 0 || ! count || ! version ) {
            LMV_ERR(
                "[%d] INormalPipeModule::get_sub_module_api_version - err:%#x count:%zu version:%p",
                mSensorIdx, status, count, version
            );
            return EIS_RETURN_NULL_OBJ;
        }

        MUINT32 selectedVersion = *(version + count - 1); //Select max. version
        LMV_LOG("[%d] count:%zu Selected CamIO Version:%0#x", mSensorIdx, count, selectedVersion);

        //  Create CamIO
        status = m_pNormalPipeModule->createSubModule(
            mSensorIdx, LMV_DRV_NAME, selectedVersion, (MVOID**)&m_pNormalPipe);
        if  ( ! m_pNormalPipe ) {
            LMV_ERR("create INormalPipe fail");
            return EIS_RETURN_NULL_OBJ;
        }
    }

    //====== Increase Reference Count ======

    android_atomic_inc(&mUsers);

    LMV_LOG("-");
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 LMVDrvImp::Uninit()
{
    //====== Reference Count ======

    android::Mutex::Autolock lock(mLock);

    if(mUsers <= 0) // No more users
    {
        LMV_LOG("mSensorIdx(%u) has 0 user",mSensorIdx);
        return EIS_RETURN_NO_ERROR;
    }

    MINT32 err = EIS_RETURN_NO_ERROR;

    // >= one user
    android_atomic_dec(&mUsers);

    if(mUsers == 0)
    {

        LMV_LOG("mSensorIdx(%u) uninit",mSensorIdx);

        //====== Destory INormalPipe ======

        if(m_pNormalPipe != NULL)
        {
            LMV_LOG("m_pNormalPipe uninit");
            m_pNormalPipe->destroyInstance(LMV_DRV_NAME);
            m_pNormalPipe = NULL;
        }

        //====== Clean Member Variable ======

        // EIS and related register setting
        memset(&mEisRegSetting,0,sizeof(EIS_REG_INFO));

        mIsConfig   = 0;
        mIsFirst    = 1;
        mIs2Pixel   = 0;
        mTotalMBNum = 0;
        mImgWidth   = 0;
        mImgHeight  = 0;
        mEisDivH    = 0;
        mEisDivV    = 0;
        mMaxGmv     = LMV_MAX_GMV_DEFAULT;
        mSensorType = EIS_NULL_SENSOR;
        mEisoIsFirst  = 1;

        // delete pass1 callback class


        delete m_pLMVP1Cb;
        m_pLMVP1Cb = NULL;

        if( mHasSGG2 && m_pSGG2P1Cb != NULL )
        {
            delete m_pSGG2P1Cb;
        }
        m_pSGG2P1Cb = NULL;

    }
    else
    {
        LMV_LOG("mSensorIdx(%u) has %d users ",mSensorIdx,mUsers);
    }
    mTsForAlgoDebug = 0;

    return err;
}


/*******************************************************************************
*
********************************************************************************/
MINT32 LMVDrvImp::ConfigLMVReg(const MUINT32 &aSensorTg)
{
    if(mIsConfig == 1)
    {
        if(g_debugDump >= 1)
        {
            LMV_LOG("configured");
        }
        return EIS_RETURN_NO_ERROR;
    }
    else
    {
        //====== Create Pass1 Callback Class ======

        m_pLMVP1Cb = new LMVP1Cb(this);
        m_pNormalPipe->sendCommand(ENPipeCmd_SET_EIS_CBFP,(MINTPTR)m_pLMVP1Cb,-1,-1);

        if( mHasSGG2 )
        {
            m_pSGG2P1Cb = new TuningNotifyImp_SGG2();
            m_pNormalPipe->sendCommand(ENPipeCmd_SET_SGG2_CBFP,(MINTPTR)m_pSGG2P1Cb,-1,-1);
        }

        //====== Sensor Info ======

        mSensorTg = aSensorTg;
        LMV_LOG("(idx,tg)=(%u,%u)",mSensorIdx,mSensorTg);
        //====== Flag Setting ======

        mIsConfig = 1;

        LMV_LOG("-");
        return EIS_RETURN_NO_ERROR;
    }
}


/*******************************************************************************
*
********************************************************************************/
MUINT32 LMVDrvImp::GetFirstFrameInfo()
{
    return mIsFirst ? 0 : 1;
}

/*******************************************************************************
*
********************************************************************************/
MUINT32 LMVDrvImp::Get2PixelMode()
{
    return mIs2Pixel;
}

/*******************************************************************************
*
********************************************************************************/
MVOID LMVDrvImp::GetLMVInputSize(MUINT32 *aWidth, MUINT32 *aHeight)
{
    *aWidth  = mImgWidth;
    *aHeight = mImgHeight;
}

/*******************************************************************************
*
********************************************************************************/
MUINT32 LMVDrvImp::GetLMVDivH()
{
    if(g_debugDump >= 1)
    {
        LMV_LOG("mEisDivH(%u)", mEisDivH);
    }
    return mEisDivH;
}

/*******************************************************************************
*
********************************************************************************/
MUINT32 LMVDrvImp::GetLMVDivV()
{
    if(g_debugDump >= 1)
    {
        LMV_LOG("mEisDivV(%u)", mEisDivV);
    }
    return mEisDivV;
}

/*******************************************************************************
*
********************************************************************************/
MUINT32 LMVDrvImp::GetLMVMaxGmv()
{
    if(g_debugDump >= 1)
    {
        LMV_LOG("mMaxGmv(%u)", mMaxGmv);
    }
    return mMaxGmv;
}

/*******************************************************************************
*
********************************************************************************/
MUINT32 LMVDrvImp::GetLMVMbNum()
{
    if(g_debugDump >= 1)
    {
        LMV_LOG("mTotalMBNum(%u)", mTotalMBNum);
    }
    return mTotalMBNum;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL LMVDrvImp::GetLMVSupportInfo(const MUINT32 &aSensorIdx)
{
    NormalPipe_EIS_Info eisInfo;
    eisInfo.mSupported = MFALSE;

    m_pNormalPipe->sendCommand(ENPipeCmd_GET_EIS_INFO,aSensorIdx,(MINTPTR)&eisInfo, -1);

    if(g_debugDump >= 1)
    {
        LMV_LOG("sensorIdx(%u),support(%d)",aSensorIdx,eisInfo.mSupported);
    }

    mEisHwSupport = eisInfo.mSupported;
    return eisInfo.mSupported;
}

/*******************************************************************************
*
********************************************************************************/
MINT64 LMVDrvImp::GetTsForAlgoDebug()
{
    return mTsForAlgoDebug;
}

MVOID LMVDrvImp::GetRegSetting(void *data)
{
    GYRO_INIT_INFO_STRUCT *a_pGisCfgData = (GYRO_INIT_INFO_STRUCT *)data;

    a_pGisCfgData->isBinning = MFALSE; //Deprecated. No used now

    a_pGisCfgData->GyroCalInfo.PIXEL_MODE = (1 << (mIs2Pixel));
    a_pGisCfgData->GyroCalInfo.EIS_OP_H_step = mEisDivH;
    a_pGisCfgData->GyroCalInfo.EIS_OP_V_step = mEisDivV;

    a_pGisCfgData->GyroCalInfo.EIS_H_win_num = (mEisRegSetting.reg_eis_prep_me_ctrl1 >>25)&0x7;
    a_pGisCfgData->GyroCalInfo.EIS_V_win_num = (mEisRegSetting.reg_eis_prep_me_ctrl1 >>28)&0xF;

    a_pGisCfgData->GyroCalInfo.EIS_RP_H_num = (mEisRegSetting.reg_eis_prep_me_ctrl1 >>8)&0x1F;
    a_pGisCfgData->GyroCalInfo.EIS_RP_V_num = (mEisRegSetting.reg_eis_prep_me_ctrl1 >>21)&0xF;

    a_pGisCfgData->GyroCalInfo.EIS_H_win_size = (mEisRegSetting.reg_eis_mb_interval >> 16) ;
    a_pGisCfgData->GyroCalInfo.EIS_V_win_size = (mEisRegSetting.reg_eis_mb_interval &0xFFFF) ;

    a_pGisCfgData->GyroCalInfo.EIS_Left_offset = mEisRegSetting.reg_eis_mb_offset >> 16;
    a_pGisCfgData->GyroCalInfo.EIS_Top_offset = mEisRegSetting.reg_eis_mb_offset & 0xFFFF;

    a_pGisCfgData->GyroCalInfo.EIS_RP_num = (a_pGisCfgData->GyroCalInfo.EIS_H_win_num * a_pGisCfgData->GyroCalInfo.EIS_V_win_num);

    a_pGisCfgData->param_Width= mSensor_Width;
    a_pGisCfgData->param_Height= mSensor_Height;

    a_pGisCfgData->sensor_Width = mRRZ_in_Width;
    a_pGisCfgData->sensor_Height = mRRZ_in_Height;

    a_pGisCfgData->rrz_crop_Width = mRrz_crop_Width;
    a_pGisCfgData->rrz_crop_Height = mRrz_crop_Height;

    a_pGisCfgData->rrz_crop_X = mRrz_crop_X;
    a_pGisCfgData->rrz_crop_Y = mRrz_crop_Y;

    a_pGisCfgData->rrz_scale_Width = mRrz_scale_Width;
    a_pGisCfgData->rrz_scale_Height = mRrz_scale_Height;

}


#if EIS_ALGO_READY

/*******************************************************************************
*
********************************************************************************/
MINT32 LMVDrvImp::GetEisHwStatistic(MINTPTR bufferVA, EIS_STATISTIC_STRUCT *apEisStat)
{
    MINT32 i;

    if (!bufferVA)
    {
        LMV_LOG("bufferVA(%p) is NULL!!!",(void*)bufferVA);
        return EIS_RETURN_EISO_MISS;
    }
    //====== Get EIS HW Statistic ======
    MUINT32 *pEisoAddr = (MUINT32 *)bufferVA;

    for(i = 0; i < EIS_MAX_WIN_NUM; ++i)
    {
        if(i != 0)
        {
            pEisoAddr += 2;  // 64bits(8bytes)

            if(g_debugDump == 3)
            {
                LMV_LOG("i(%d),pEisoAddr(%p)",i,pEisoAddr);
            }
        }

        apEisStat->i4LMV_X2[i]   = Complement2(*pEisoAddr & 0x1F, 5);               //[0:4]
        apEisStat->i4LMV_Y2[i]   = Complement2(((*pEisoAddr & 0x3E0) >> 5), 5);    //[5:9]
        apEisStat->SAD[i]        = (*pEisoAddr & 0x7FC00) >> 10;                    //[10:18]
        apEisStat->NewTrust_X[i] = (*pEisoAddr & 0x03F80000) >> 19;                 //[19:25]
        apEisStat->NewTrust_Y[i] = ((*pEisoAddr & 0xFC000000) >> 26) + ((*(pEisoAddr+1) & 0x00000001) << 6);    //[26:32]
        apEisStat->i4LMV_X[i]    = Complement2(((*(pEisoAddr + 1) & 0x00003FFE) >> 1), 13);     //[33:45] -> [1:13]
        apEisStat->i4LMV_Y[i]    = Complement2(((*(pEisoAddr + 1) & 0x07FFC000) >> 14), 13);    //[46:58] -> [14:26]
        apEisStat->SAD2[i]       = 0;
        apEisStat->AVG_SAD[i]    = 0;
    }

    if(g_debugDump == 3)
    {
        for(i = 0; i < EIS_MAX_WIN_NUM; ++i)
        {
            LMV_LOG("EIS[%d]=lmv(%d,%d),lmv2(%d,%d),trust(%d,%d),sad(%d)",i,
                                                                          apEisStat->i4LMV_X[i], apEisStat->i4LMV_Y[i],
                                                                          apEisStat->i4LMV_X2[i], apEisStat->i4LMV_Y2[i],
                                                                          apEisStat->NewTrust_X[i], apEisStat->NewTrust_Y[i],
                                                                          apEisStat->SAD[i]);
        }
        LMV_LOG("-");
    }

    return EIS_RETURN_NO_ERROR;
}

#endif

/*******************************************************************************
*
********************************************************************************/
MINT32 LMVDrvImp::Complement2(MUINT32 value, MUINT32 digit)
{
    MINT32 Result;

    if (((value >> (digit - 1)) & 0x1) == 1)    // negative
    {
        Result = 0 - (MINT32)((~value + 1) & ((1 << digit) - 1));
    }
    else
    {
        Result = (MINT32)(value & ((1 << digit) - 1));
    }

    return Result;
}

/*******************************************************************************
*
*******************************************************************************/
MVOID LMVDrvImp::BoundaryCheck(MUINT32 &aInput,const MUINT32 &upBound,const MUINT32 &lowBound)
{
    if(aInput > upBound)
    {
        aInput = upBound;
    }

    if(aInput < lowBound)
    {
        aInput = lowBound;
    }
}

/*******************************************************************************
*
*******************************************************************************/
MINT64 LMVDrvImp::GetTimeStamp(const MUINT32 &aSec,const MUINT32 &aUs)
{
    return  aSec * 1000000000LL + aUs * 1000LL;
}

/*******************************************************************************
*
********************************************************************************/
TuningNotifyImp_SGG2::TuningNotifyImp_SGG2()
{
}

/*******************************************************************************
*
********************************************************************************/
TuningNotifyImp_SGG2::~TuningNotifyImp_SGG2()
{
}


void TuningNotifyImp_SGG2::p1TuningNotify(MVOID* pIn, MVOID* pOut)
{
    LMV_SGG_CFG *sgg2_cfg = (LMV_SGG_CFG *)pOut;
    if (pIn)
    {
    }

    if (sgg2_cfg)
    {
        sgg2_cfg->bSGG2_Bypass   = 0; //Confirmed with Ethan
        sgg2_cfg->bSGG2_EN       = 1; //Confirmed with Ethan
        sgg2_cfg->PGN  = 0x10/*RSTVAL*/;
        sgg2_cfg->GMRC_1 = 0x63493527/*RSTVAL*/;
        sgg2_cfg->GMRC_2 = 0x00FFBB88/*RSTVAL*/;
    }
}



/*******************************************************************************
*
********************************************************************************/
LMVP1Cb::LMVP1Cb(MVOID *arg)
{
    m_pClassObj = arg;
}

/*******************************************************************************
*
********************************************************************************/
LMVP1Cb::~LMVP1Cb()
{
    m_pClassObj = NULL;
}

/*******************************************************************************
*
********************************************************************************/
void LMVP1Cb::p1TuningNotify(MVOID *pInput,MVOID *pOutput)
{
    LMVDrvImp *_this = reinterpret_cast<LMVDrvImp *>(m_pClassObj);
    LMV_INPUT_INFO *aEisInInfo = (LMV_INPUT_INFO *)pInput;
    LMV_CFG *a_pEisCfgData = (LMV_CFG *)pOutput;
    const MUINT32 subG_en = 0;//always be 0

    if(_this->mIsConfig == 0)
    {
        LMV_LOG("not config done");
        a_pEisCfgData->bypassLMV = MTRUE;
        return;
    }

    // ====== Dynamic Debug ======
    if (UNLIKELY(g_debugDump > 0))
    {
        LMV_LOG("TG(%u,%u),HBIN(%u,%u),RMX(%u,%u)",aEisInInfo->sTGOut.w,aEisInInfo->sTGOut.h,
                                                   aEisInInfo->sHBINOut.w,aEisInInfo->sHBINOut.h,
                                                   aEisInInfo->sRMXOut.w,aEisInInfo->sRMXOut.h);
        LMV_LOG("YUV(%d),pixMode(%d)",aEisInInfo->bYUVFmt,aEisInInfo->pixMode);
    }

#if 0
    LMV_LOG("TG(%u,%u),RRZ In(%u,%u), RRZ crop x/y(%u,%u), RRZ crop w/h(%u,%u), RMX(%u,%u), pixelMode(%d)",
                                                   aEisInInfo->sTGOut.w,aEisInInfo->sTGOut.h,
                                                   aEisInInfo->RRZ_IN_CROP.in_size_w, aEisInInfo->RRZ_IN_CROP.in_size_h,
                                                   aEisInInfo->RRZ_IN_CROP.start_x, aEisInInfo->RRZ_IN_CROP.start_y,
                                                   aEisInInfo->RRZ_IN_CROP.crop_size_w,aEisInInfo->RRZ_IN_CROP.crop_size_h,
                                                   aEisInInfo->sRMXOut.w, aEisInInfo->sRMXOut.h, aEisInInfo->pixMode);
#endif

    // ====== Image Size Dependent Register ======

    if(aEisInInfo->bYUVFmt== MFALSE)    // RAW sensor specific register
    {
        //> rrzo

        MUINT32 tempW = 0, tempH = 0;

        tempW = aEisInInfo->sRMXOut.w;
        tempH = aEisInInfo->sRMXOut.h;

        //> imgo only

        if(aEisInInfo->sRMXOut.w == 0 && aEisInInfo->sRMXOut.h == 0)
        {
            if(_this->mIsFirst == 1)
            {
                LMV_LOG("imgo only");
            }
            tempW = aEisInInfo->sHBINOut.w;
            tempH = aEisInInfo->sHBINOut.h;
        }

        //> pixel mode: 0(None) or 1(2-Pixel mode) or 2(Quad-Pixel)

        if ( (aEisInInfo->pixMode > 0) && (aEisInInfo->pixMode <3))
        {
            if (_this->mIsFirst == 1)
            {
                LMV_LOG("pixel mode: %d", aEisInInfo->pixMode);
            }

            tempW = tempW >> aEisInInfo->pixMode;
            if (tempW != (MUINT32)aEisInInfo->sHBINOut.w)
            {
                LMV_WRN("RRZ shift HDS(%d) is different to HBIN(%d)", tempW, aEisInInfo->sHBINOut.w);
                tempW = aEisInInfo->sHBINOut.w;
            }
            _this->mIs2Pixel = aEisInInfo->pixMode;
        }
        else
        {
            _this->mIs2Pixel = 0;
        }

        if(_this->mImgWidth != tempW || _this->mImgHeight != tempH)
        {
            LMV_LOG("(1)first:new(%u,%u),old(%u,%u)",tempW,tempH,_this->mImgWidth,_this->mImgHeight);
            _this->mIsFirst = 1;
            a_pEisCfgData->bypassLMV = MFALSE;
        }
        else
        {
            _this->mIsFirst = 0;
            a_pEisCfgData->bypassLMV = MTRUE;
        }

        _this->mImgWidth   = tempW;
        _this->mImgHeight  = tempH;
        _this->mSensorType = EIS_RAW_SENSOR;
    }
    else if(aEisInInfo->bYUVFmt== MTRUE)   // YUV sensor specific register
    {

        //> get TG size

        if(_this->mImgWidth != (MUINT32)aEisInInfo->sTGOut.w || _this->mImgHeight != (MUINT32)aEisInInfo->sTGOut.h)
        {
            LMV_LOG("(2)first:new(%u,%u),old(%u,%u)",aEisInInfo->sTGOut.w,aEisInInfo->sTGOut.h,_this->mImgWidth,_this->mImgHeight);
            _this->mIsFirst = 1;
            a_pEisCfgData->bypassLMV = MFALSE;
        }
        else
        {
            _this->mIsFirst = 0;
            a_pEisCfgData->bypassLMV = MTRUE;
        }

        _this->mImgWidth   = aEisInInfo->sTGOut.w;
        _this->mImgHeight  = aEisInInfo->sTGOut.h;
        _this->mSensorType = EIS_YUV_SENSOR;
    }


    _this->mSensor_Width = aEisInInfo->sTGOut.w;
    _this->mSensor_Height = aEisInInfo->sTGOut.h;
    _this->mRRZ_in_Width = aEisInInfo->RRZ_IN_CROP.in_size_w; //input size of RRZ, binning from sensor or ISP
    _this->mRRZ_in_Height= aEisInInfo->RRZ_IN_CROP.in_size_h; //input size of RRZ, binning from sensor or ISP
    _this->mRrz_crop_Width = aEisInInfo->RRZ_IN_CROP.crop_size_w;
    _this->mRrz_crop_Height = aEisInInfo->RRZ_IN_CROP.crop_size_h;
    _this->mRrz_crop_X = aEisInInfo->RRZ_IN_CROP.start_x;
    _this->mRrz_crop_Y= aEisInInfo->RRZ_IN_CROP.start_y;

    _this->mRrz_scale_Width = ( _this->mIs2Pixel > 0)? (_this->mImgWidth << (_this->mIs2Pixel)) : (_this->mImgWidth );
    _this->mRrz_scale_Height = _this->mImgHeight;

    if (UNLIKELY(g_debugDump > 0))
    {
        LMV_LOG("mImgWidth(%u),mImgHeight(%u),mSensorType(%d)",_this->mImgWidth,_this->mImgHeight,_this->mSensorType);
        LMV_LOG("Pixel Mode(%u)",_this->mIs2Pixel);
    }

    //> EIS enable bit

    a_pEisCfgData->enLMV = MTRUE;

    if(a_pEisCfgData->bypassLMV == MTRUE)
    {
        _this->mEisRegSetting.reg_eis_prep_me_ctrl2 &= 0x07FFF;

        if (UNLIKELY(g_debugDump > 0))
        {
            LMV_LOG("me_ctrl2(0x%08x)",_this->mEisRegSetting.reg_eis_prep_me_ctrl2);
        }

        a_pEisCfgData->cfg_lmv_prep_me_ctrl1   = _this->mEisRegSetting.reg_eis_prep_me_ctrl1;
        a_pEisCfgData->cfg_lmv_prep_me_ctrl2  = _this->mEisRegSetting.reg_eis_prep_me_ctrl2;
        a_pEisCfgData->cfg_lmv_lmv_th   = _this->mEisRegSetting.reg_eis_lmv_th;
        a_pEisCfgData->cfg_lmv_fl_offset   = _this->mEisRegSetting.reg_eis_fl_offset;
        a_pEisCfgData->cfg_lmv_mb_offset   = _this->mEisRegSetting.reg_eis_mb_offset;
        a_pEisCfgData->cfg_lmv_mb_interval   = _this->mEisRegSetting.reg_eis_mb_interval;
        a_pEisCfgData->cfg_lmv_gmv      = 0;  // not use
        a_pEisCfgData->cfg_lmv_err_ctrl = 0xF0000;  // HW default value, not use
        a_pEisCfgData->cfg_lmv_image_ctrl = _this->mEisRegSetting.reg_eis_image_ctrl;
        a_pEisCfgData->bypassLMV       = MFALSE;
        return;
    }

    //====== Setting Depend on Image Size ======

    MUINT32 win_numV = 3, win_numH = 2;
    MUINT32 rp_numV, rp_numH;
    MUINT32 knee_1, knee_2;
    MUINT32 proc_gain;
    MUINT32 dc_dl;
    MUINT32 vert_shr, hori_shr;
    MUINT32 rp_offsetV, rp_offsetH;
    MUINT32 win_sizeV, win_sizeH;
    MUINT32 max_range = 0;

    switch (g_forceMaxSearchRange)
    {
        case LMV_MAX_GMV_32:
            // ====== max search range 32 =======
            if(_this->mImgWidth > HD_8M_WIDTH)
            {
                _this->mEisDivH = 4;
            }
            else if(_this->mImgWidth > HD_720P_WIDTH)
            {
                _this->mEisDivH = 2;
            }
            else
            {
                _this->mEisDivH = 1;
            }

            if(_this->mImgWidth > D1_WIDTH)
            {
                win_numH = 4;
            }
            else if(_this->mImgWidth > CIF_WIDTH)
            {
                win_numH = 4;
            }
            else
            {
                win_numH = 2;
            }

            // vertical
            if(_this->mImgHeight > HD_8M_HEIGHT)
            {
                _this->mEisDivV = 4;
            }
            else if(_this->mImgHeight > HD_720P_HEIGHT)
            {
                _this->mEisDivV = 2;
            }
            else
            {
                _this->mEisDivV = 1;
            }

            if(_this->mImgHeight > D1_HEIGHT)
            {
                win_numV = 8;
            }
            else if( _this->mImgHeight > CIF_HEIGHT)
            {
                win_numV = 4;
            }
            else
            {
                win_numV = 3;
            }

            LMV_LOG("org (w,h)=(%d,%d), (DivH, DivV)=(%d,%d), (winH, winV)=(%d,%d), MaxGmv(%d)",
                _this->mImgWidth, _this->mImgHeight,
                _this->mEisDivH, _this->mEisDivV, win_numH, win_numV, max_range);

            break;
        case LMV_MAX_GMV_64:
            {
                // ====== max search range 64 =======
                MINT32 table_size = sizeof(lmv_step_win_setting)/sizeof(LMV_STEP_WIN_SETTING);
                const MUINT8  *table_name = lmv_step_win_setting[sizeof(lmv_step_win_setting)/sizeof(LMV_STEP_WIN_SETTING)-1].name;

                for(MINT32 i = 0; i < table_size; i++)
                {
                    if (_this->mImgWidth >= lmv_step_win_setting[i].img_w &&
                        _this->mImgHeight >= lmv_step_win_setting[i].img_h)
                    {
                        _this->mEisDivH = lmv_step_win_setting[i].eis_op_step_h;
                        _this->mEisDivV = lmv_step_win_setting[i].eis_op_step_v;
                        win_numH = lmv_step_win_setting[i].eis_num_h_win;
                        win_numV = lmv_step_win_setting[i].eis_num_v_win;
                        max_range = lmv_step_win_setting[i].max_range;
                        _this->mMaxGmv= max_range;
                        table_name = lmv_step_win_setting[i].name;
                        break;
                    }
                }

                LMV_LOG("[%s] (w,h)=(%d,%d), (DivH, DivV)=(%d,%d), (winH, winV)=(%d,%d), MaxGmv(%d)",
                    table_name, _this->mImgWidth, _this->mImgHeight,
                    _this->mEisDivH, _this->mEisDivV, win_numH, win_numV, max_range);
            }

            break;
        default:
            LMV_LOG("Wrong MaxSearchRange(%d). please set correct range!", g_forceMaxSearchRange);

            break;
    }

    _this->mTotalMBNum = win_numH * win_numV;

    //====== Fix Setting ======

    knee_1 = 4;
    knee_2 = 6;
    proc_gain = 0;
    _this->mEisRegSetting.reg_eis_lmv_th = 0;    // not use right now
    _this->mEisRegSetting.reg_eis_fl_offset = 0;

    //====== Setting Calculateing ======

    // decide dc_dl
    if(_this->mImgWidth > CIF_WIDTH && _this->mImgHeight > CIF_HEIGHT)
    {
        dc_dl = 32;
    }
    else
    {
        dc_dl = 16;
    }

    // decide vert_shr and hori_shr
    if(_this->mEisDivH == 1)
    {
        hori_shr = 3;
    }
    else
    {
        hori_shr = 4;
    }

    if(_this->mEisDivV == 1)
    {
        vert_shr = 3;
    }
    else
    {
        vert_shr = 4;
    }

    //Decide MB_OFFSET
    MUINT32 dead_left, dead_upper;

    dead_left  = (1 << hori_shr) * 2 + dc_dl;
    dead_upper = (1 << vert_shr) * 2;

    rp_offsetH = (dead_left  + 16 * _this->mEisDivH + 8) / _this->mEisDivH;
    rp_offsetV = (dead_upper + 16 * _this->mEisDivV + 8) / _this->mEisDivV;

    //Decide MB_INTERVAL
    MUINT32 first_win_left_corner, first_win_top_corner;
    MUINT32 active_sizeH,active_sizeV;

    first_win_left_corner = (rp_offsetH - 16) * _this->mEisDivH;
    first_win_top_corner  = (rp_offsetV - 16) * _this->mEisDivV;

    active_sizeH = _this->mImgWidth  - first_win_left_corner - 16;
    active_sizeV = _this->mImgHeight - first_win_top_corner - 8;

    win_sizeH = active_sizeH / win_numH / _this->mEisDivH;
    win_sizeV = active_sizeV / win_numV / _this->mEisDivV;

    //Decide rp_numH, rp_numV;
    rp_numH = ((win_sizeH - 1) / 16) - 1;
    rp_numV = ((win_sizeV - 1) / 16) - 1;

    if (UNLIKELY(g_debugDump > 0))
    {
        LMV_LOG("first_win_left_corner(%u),first_win_top_corner(%u)",first_win_left_corner,first_win_top_corner);
        LMV_LOG("active_sizeH(%u),active_sizeV(%u)",active_sizeH,active_sizeV);
        LMV_LOG("win_sizeH(%u),win_sizeV(%u)",win_sizeH,win_sizeV);
        LMV_LOG("rp_numH(%u),rp_numV(%u)",rp_numH,rp_numV);
    }

    if(rp_numH > 16)
    {
        rp_numH = 16;
    }

    MUINT32 tempRpV = 2048 / (win_numH * win_numV * rp_numH);
    tempRpV = std::min((MUINT32)8, tempRpV);
    tempRpV = ( win_numV > 4 ) ? std::min((MUINT32) 4, tempRpV ) : tempRpV;
    rp_numV = std::min(tempRpV, rp_numV);

    if (UNLIKELY(g_debugDump > 0))
    {
        LMV_LOG("tempRpV(%u)",tempRpV);
    }

    // MB_OFFSET value check

    MUINT32 offset_lowBound_H, offset_lowBound_V, offset_upperBound_H, offset_upperBound_V;
    MINT32 temp_fl_offset_H = (_this->mEisRegSetting.reg_eis_fl_offset & 0xFFF0000) >> 16;
    MINT32 temp_fl_offset_V = _this->mEisRegSetting.reg_eis_fl_offset & 0xFFF;

    if (UNLIKELY(g_debugDump > 0))
    {
        LMV_LOG("temp_fl_offset_H(%d)",temp_fl_offset_H);
        LMV_LOG("temp_fl_offset_V(%d)",temp_fl_offset_V);
    }

    // low bound
    if(temp_fl_offset_H < 0)
    {
        offset_lowBound_H = 11 - temp_fl_offset_H;
    }
    else
    {
        offset_lowBound_H = 11 + temp_fl_offset_H;
    }


    if(temp_fl_offset_V < 0)
    {
        offset_lowBound_V = 9 - temp_fl_offset_V;
    }
    else
    {
        offset_lowBound_V = 9 + temp_fl_offset_V;
    }

    // up bound
    if(temp_fl_offset_H > 0)
    {
        offset_upperBound_H = (_this->mImgWidth/_this->mEisDivH) - (rp_numH*16) - temp_fl_offset_H - (win_sizeH*(win_numH-1));
    }
    else
    {
        offset_upperBound_H = (_this->mImgWidth/_this->mEisDivH) - (rp_numH*16) - 1 - (win_sizeH*(win_numH-1));
    }

    if(temp_fl_offset_V > 0)
    {
        offset_upperBound_V = (_this->mImgHeight/_this->mEisDivV) - (rp_numV*16) - temp_fl_offset_V - (win_sizeV*(win_numV-1));
    }
    else
    {
        offset_upperBound_V = (_this->mImgHeight/_this->mEisDivV) - (rp_numV*16) - 1 - (win_sizeV*(win_numV-1));
    }

    if (UNLIKELY(g_debugDump > 0))
    {
        LMV_LOG("ori rp_offset (H/V)=(%d/%d)",rp_offsetH,rp_offsetV);
        LMV_LOG("bound: H(%d/%d),V(%d/%d)",offset_lowBound_H,offset_upperBound_H,offset_lowBound_V,offset_upperBound_V);
    }

    _this->BoundaryCheck(rp_offsetH,offset_upperBound_H,offset_lowBound_H);
    _this->BoundaryCheck(rp_offsetV,offset_upperBound_V,offset_lowBound_V);

    if (UNLIKELY(g_debugDump > 0))
    {
        LMV_LOG("final rp_offset (H/V)=(%d/%d)",rp_offsetH,rp_offsetV);
    }

    // MB_INTERVAL value check

    MUINT32 interval_lowBound_H, interval_lowBound_V, interval_upperBound_H, interval_upperBound_V;

    // low bound
    interval_lowBound_H = (rp_numH + 1) * 16;
    interval_lowBound_V = (rp_numV + 1) * 16 + 1;

    // up bound
    if(temp_fl_offset_H > 0)
    {
        interval_upperBound_H = ((_this->mImgWidth/_this->mEisDivH) - rp_offsetH - (rp_numH*16) - temp_fl_offset_H) / (win_numH-1);
    }
    else
    {
        interval_upperBound_H = ((_this->mImgWidth/_this->mEisDivH) - rp_offsetH - (rp_numH*16) - 1) / (win_numH-1);
    }

    if(temp_fl_offset_V > 0)
    {
        interval_upperBound_V = ((_this->mImgHeight/_this->mEisDivV) - rp_offsetV - (rp_numV*16) - temp_fl_offset_V) / (win_numV-1);
    }
    else
    {
        interval_upperBound_V = ((_this->mImgHeight/_this->mEisDivV) - rp_offsetV - (rp_numV*16) - 1) / (win_numV-1);
    }

    if (UNLIKELY(g_debugDump > 0))
    {
        LMV_LOG("ori win_size (H/V)=(%d/%d)",win_sizeH,win_sizeV);
        LMV_LOG("bound: H(%d/%d),V(%d/%d)",interval_lowBound_H,interval_upperBound_H,interval_lowBound_V,interval_upperBound_V);
    }

    _this->BoundaryCheck(win_sizeH,interval_upperBound_H,interval_lowBound_H);
    _this->BoundaryCheck(win_sizeV,interval_upperBound_V,interval_lowBound_V);

    if (UNLIKELY(g_debugDump > 0))
    {
        LMV_LOG("final win_size (H/V)=(%d/%d)",win_sizeH,win_sizeV);
    }

    //====== Intrgrate Setting ======

    _this->mEisRegSetting.reg_eis_prep_me_ctrl1 = (win_numV << 28) | (win_numH << 25) | (rp_numV << 21) |
                                           (knee_1 << 17) | (knee_2 << 13) | (rp_numH << 8) |
                                           (subG_en << 6) | (_this->mEisDivV << 3) | _this->mEisDivH;

    _this->mEisRegSetting.reg_eis_prep_me_ctrl2 = (1 << 15) | (1 << 14) | (dc_dl << 8) | (vert_shr << 5) | (hori_shr << 2) | proc_gain;

    _this->mEisRegSetting.reg_eis_mb_offset   = (rp_offsetH << 16) | rp_offsetV;
    _this->mEisRegSetting.reg_eis_mb_interval = (win_sizeH << 16) | win_sizeV;
    _this->mEisRegSetting.reg_eis_image_ctrl  = (_this->mImgWidth << 16) | _this->mImgHeight;

    //======= Set to Output Data ======

    //> EIS register

    a_pEisCfgData->cfg_lmv_prep_me_ctrl1   = _this->mEisRegSetting.reg_eis_prep_me_ctrl1;
    a_pEisCfgData->cfg_lmv_prep_me_ctrl2   = _this->mEisRegSetting.reg_eis_prep_me_ctrl2;
    a_pEisCfgData->cfg_lmv_lmv_th   = _this->mEisRegSetting.reg_eis_lmv_th;
    a_pEisCfgData->cfg_lmv_fl_offset   = _this->mEisRegSetting.reg_eis_fl_offset;
    a_pEisCfgData->cfg_lmv_mb_offset    = _this->mEisRegSetting.reg_eis_mb_offset;
    a_pEisCfgData->cfg_lmv_mb_interval   = _this->mEisRegSetting.reg_eis_mb_interval;
    a_pEisCfgData->cfg_lmv_gmv      = 0;  // not use
    a_pEisCfgData->cfg_lmv_err_ctrl = 0xF0000;  // HW default value, not use
    a_pEisCfgData->cfg_lmv_image_ctrl = _this->mEisRegSetting.reg_eis_image_ctrl;

    //====== Debug ======

    if (UNLIKELY(g_debugDump > 0))
    {
        LMV_LOG("reg_eis_prep_me_ctrl1(0x%08x)",_this->mEisRegSetting.reg_eis_prep_me_ctrl1);
        LMV_LOG("win_numV:(0x%08x,0x%08x)",win_numV,(_this->mEisRegSetting.reg_eis_prep_me_ctrl1 & 0xF0000000) >> 28);
        LMV_LOG("win_numH:(0x%08x,0x%08x)",win_numH,(_this->mEisRegSetting.reg_eis_prep_me_ctrl1 & 0xE000000) >> 25);
        LMV_LOG("rp_numV:(0x%08x,0x%08x)",rp_numV,(_this->mEisRegSetting.reg_eis_prep_me_ctrl1 & 0x1E00000) >> 21);
        LMV_LOG("knee_1:(0x%08x,0x%08x)",knee_1,(_this->mEisRegSetting.reg_eis_prep_me_ctrl1 & 0x1E0000) >> 17);
        LMV_LOG("knee_2:(0x%08x,0x%08x)",knee_2,(_this->mEisRegSetting.reg_eis_prep_me_ctrl1 & 0x1E000) >> 13);
        LMV_LOG("rp_numH:(0x%08x,0x%08x)",rp_numH,(_this->mEisRegSetting.reg_eis_prep_me_ctrl1 & 0x1F00) >> 8);
        LMV_LOG("subG_en:(0x%08x,0x%08x)",subG_en,(_this->mEisRegSetting.reg_eis_prep_me_ctrl1 & 0x40) >> 6);
        LMV_LOG("eis_op_vert:(0x%08x,0x%08x)",_this->mEisDivV,(_this->mEisRegSetting.reg_eis_prep_me_ctrl1 & 0x38) >> 3);
        LMV_LOG("eis_op_hori:(0x%08x,0x%08x)",_this->mEisDivH,_this->mEisRegSetting.reg_eis_prep_me_ctrl1 & 0x7);

        LMV_LOG("reg_eis_prep_me_ctrl2(0x%08x)",_this->mEisRegSetting.reg_eis_prep_me_ctrl2);
        LMV_LOG("first_frame:0x%08x",(_this->mEisRegSetting.reg_eis_prep_me_ctrl2 & 0x8000) >> 15);
        LMV_LOG("write_en(rp_modi):0x%08x",(_this->mEisRegSetting.reg_eis_prep_me_ctrl2 & 0x4000) >> 14);
        LMV_LOG("dc_dl:(0x%08x,0x%08x)",dc_dl,(_this->mEisRegSetting.reg_eis_prep_me_ctrl2 & 0x3F00) >> 8);
        LMV_LOG("vert_shr:(0x%08x,0x%08x)",vert_shr,(_this->mEisRegSetting.reg_eis_prep_me_ctrl2 & 0xE0) >> 5);
        LMV_LOG("hori_shr:(0x%08x,0x%08x)",hori_shr,(_this->mEisRegSetting.reg_eis_prep_me_ctrl2 & 0x1C) >> 2);
        LMV_LOG("proc_gain:(0x%08x,0x%08x)",proc_gain,_this->mEisRegSetting.reg_eis_prep_me_ctrl2 & 0x3);

        LMV_LOG("reg_eis_lmv_th(0x%08x)",_this->mEisRegSetting.reg_eis_lmv_th);
        LMV_LOG("reg_eis_fl_offset(0x%08x)",_this->mEisRegSetting.reg_eis_fl_offset);

        LMV_LOG("reg_eis_mb_offset(0x%08x)",_this->mEisRegSetting.reg_eis_mb_offset);
        LMV_LOG("rp_offsetH:(0x%08x,0x%08x)",rp_offsetH,(_this->mEisRegSetting.reg_eis_mb_offset&0xFFF0000)>>16);
        LMV_LOG("rp_offsetV:(0x%08x,0x%08x)",rp_offsetV,_this->mEisRegSetting.reg_eis_mb_offset&0xFFF);

        LMV_LOG("reg_eis_mb_interval(0x%08x)",_this->mEisRegSetting.reg_eis_mb_interval);
        LMV_LOG("win_sizeH:(0x%08x,0x%08x)",win_sizeH,(_this->mEisRegSetting.reg_eis_mb_interval&0xFFF0000)>>16);
        LMV_LOG("win_sizeV:(0x%08x,0x%08x)",win_sizeV,_this->mEisRegSetting.reg_eis_mb_interval&0xFFF);

        LMV_LOG("reg_eis_image_ctrl(0x%08x)",_this->mEisRegSetting.reg_eis_image_ctrl);
        LMV_LOG("Width:(0x%08x,0x%08x)",_this->mImgWidth,(_this->mEisRegSetting.reg_eis_image_ctrl&0x1FFF0000)>>16);
        LMV_LOG("Height:(0x%08x,0x%08x)",_this->mImgHeight,_this->mEisRegSetting.reg_eis_image_ctrl&0x1FFF);
    }
}




