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

//! \file  lmv_drv.cpp
#include <cutils/properties.h>
#include <cutils/atomic.h>
#include <MTKGyro.h>

#include <mtkcam/drv/iopipe/CamIO/Cam_Notify.h>
#include <mtkcam/drv/IHalSensor.h>

#include "lmv_drv_imp.h"

using namespace NSCam;
using namespace NSCam::NSIoPipe::NSCamIOPipe;
using namespace NSCam::NSIoPipe;


/****************************************************************************************
* Define Value
****************************************************************************************/

#define LMV_DRV_DEBUG

#undef LOG_TAG
#define LOG_TAG "LMVDrv"
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_LMV_HAL);

#ifdef LMV_DRV_DEBUG

#undef __func__
#define __func__ __FUNCTION__

#undef LMV_LOG_LV1
#undef LMV_LOG_LV2
#undef LMV_LOG_LV3
#undef LMV_LOG
#undef LMV_S_LOG
#undef LMV_P_LOG
#undef LMV_DO
#undef MY_LOGD
#undef MY_LOGI
#undef MY_LOGW
#undef MY_LOGE
#undef MY_S_LOGD
#undef MY_S_LOGI
#undef MY_S_LOGW
#undef MY_S_LOGE
#undef MY_LOGD0
#undef MY_LOGD1
#undef MY_LOGD2
#undef MY_LOGD3
#undef MY_TRACE_API_LIFE
#undef MY_TRACE_FUNC_LIFE
#undef MY_TRACE_TAG_LIFE


#define LMV_LOG_LV1                   ( 1 <= g_debugDump )
#define LMV_LOG_LV2                   ( 2 <= g_debugDump )
#define LMV_LOG_LV3                   ( 3 <= g_debugDump )

#define LMV_LOG(lv, fmt, arg...)      CAM_ULOGM##lv("[%s]" fmt, __FUNCTION__, ##arg)
#define LMV_S_LOG(lv, fmt, arg...)    CAM_ULOGM##lv("[%s][Cam::%d]" fmt, __FUNCTION__, mSensorIdx, ##arg)

#define LMV_DO(cmd) do { cmd; } while(0)

#define MY_LOGD(fmt, arg...)          LMV_DO( LMV_LOG(D, fmt, ##arg))
#define MY_LOGI(fmt, arg...)          LMV_DO( LMV_LOG(I, fmt, ##arg))
#define MY_LOGW(fmt, arg...)          LMV_DO( LMV_LOG(W, fmt, ##arg))
#define MY_LOGE(fmt, arg...)          LMV_DO( LMV_LOG(E, fmt, ##arg))

#define MY_S_LOGD(fmt, arg...)        LMV_DO( LMV_S_LOG(D, fmt, ##arg))
#define MY_S_LOGI(fmt, arg...)        LMV_DO( LMV_S_LOG(I, fmt, ##arg))
#define MY_S_LOGW(fmt, arg...)        LMV_DO( LMV_S_LOG(W, fmt, ##arg))
#define MY_S_LOGE(fmt, arg...)        LMV_DO( LMV_S_LOG(E, fmt, ##arg))
#define MY_S_LOGD_IF(c, fmt, arg...)  LMV_DO(if(c) LMV_S_LOG(D, fmt, ##arg))

#define MY_LOGD0(fmt, arg...)         MY_S_LOGD(fmt, ##arg)
#define MY_LOGD1(fmt, arg...)         MY_S_LOGD_IF(LMV_LOG_LV1, fmt, ##arg)
#define MY_LOGD2(fmt, arg...)         MY_S_LOGD_IF(LMV_LOG_LV2, fmt, ##arg)
#define MY_LOGD3(fmt, arg...)         MY_S_LOGD_IF(LMV_LOG_LV3, fmt, ##arg)

#define MY_TRACE_API_LIFE()           CAM_ULOGM_APILIFE()
#define MY_TRACE_FUNC_LIFE()          CAM_ULOGM_FUNCLIFE()
#define MY_TRACE_TAG_LIFE(name)       CAM_ULOGM_TAGLIFE(name)


#endif

#define LMV_DRV_NAME            "LMVDrv"
#define LMV_DRV_DUMP            "vendor.debug.LMVDrv.dump"

/*******************************************************************************
* Global variable
********************************************************************************/
static MINT32 g_debugDump = 0;

LMVDrv* LMVDrv::CreateInstance(const MUINT32 &aSensorIdx)
{
    return LMVDrvImp::GetInstance(aSensorIdx);
}

LMVDrv* LMVDrvImp::GetInstance(const MUINT32 &aSensorIdx)
{
    MY_LOGI("aSensorIdx(%u)", aSensorIdx);

    return new LMVDrvImp(aSensorIdx);
}

MVOID LMVDrvImp::DestroyInstance()
{
    MY_LOGD0("-");
    delete this;
}

LMVDrvImp::LMVDrvImp(const MUINT32 &aSensorIdx) : LMVDrv()
{
    mSensorIdx = aSensorIdx;
    mName = LMV_DRV_NAME"_" + std::to_string(aSensorIdx);
}

LMVDrvImp::~LMVDrvImp()
{
}

MINT32 LMVDrvImp::Init()
{
    MY_TRACE_API_LIFE();

    MINT32 err = LMV_RETURN_NO_ERROR;

    android::Mutex::Autolock lock(mLock);

    if( mUsers > 0 )
    {
        android_atomic_inc(&mUsers);
        MY_LOGD0("has %d users", mUsers);
        return LMV_RETURN_NO_ERROR;
    }
    g_debugDump = ::property_get_int32(LMV_DRV_DUMP, 0);

    m_pNormalPipeModule = INormalPipeModule::get();
    if( ! m_pNormalPipeModule )
    {
        MY_LOGE("INormalPipeModule::get() fail");
        return LMV_RETURN_NULL_OBJ;
    }

    {
        int status = 0;

        //  Select version
        size_t count = 0;
        MUINT32 const* version = NULL;
        status = m_pNormalPipeModule->get_sub_module_api_version(&version, &count, mSensorIdx);
        if( status < 0 || ! count || ! version )
        {
            MY_LOGE(
                "[%d] INormalPipeModule::get_sub_module_api_version - err:%#x count:%zu version:%p",
                mSensorIdx, status, count, version
            );
            return LMV_RETURN_NULL_OBJ;
        }

        MUINT32 selectedVersion = *(version + count - 1); //Select max. version
        MY_LOGD0("count:%zu Selected CamIO Version:%0#x", count, selectedVersion);

        //  Create CamIO
        status = m_pNormalPipeModule->createSubModule(
            mSensorIdx, mName.c_str(), selectedVersion, (MVOID**)&m_pNormalPipe);
        if( ! m_pNormalPipe )
        {
            MY_LOGE("create INormalPipe fail");
            return LMV_RETURN_NULL_OBJ;
        }
    }

    android_atomic_inc(&mUsers);

    MY_LOGD0("-");
    return err;
}

MINT32 LMVDrvImp::Uninit()
{
    MY_TRACE_API_LIFE();

    android::Mutex::Autolock lock(mLock);

    if( mUsers <= 0 ) // No more users
    {
        MY_LOGD0("has 0 user");
        return LMV_RETURN_NO_ERROR;
    }

    MINT32 err = LMV_RETURN_NO_ERROR;

    // >= one user
    android_atomic_dec(&mUsers);

    if( mUsers == 0 )
    {
        if( m_pNormalPipe != NULL )
        {
            MY_LOGD0("m_pNormalPipe uninit");
            m_pNormalPipe->destroyInstance(mName.c_str());
            m_pNormalPipe = NULL;
        }

        memset(&mLmvRegSetting, 0, sizeof(LMV_REG_INFO));
        mIsConfig   = 0;
        mIsFirst    = 1;
        mIs2Pixel   = 0;
        mTotalMBNum = 0;
        mImgWidth   = 0;
        mImgHeight  = 0;
        mLmvDivH    = 0;
        mLmvDivV    = 0;
        mMaxGmv     = LMV_MAX_GMV_DEFAULT;
        mSensorType = LMV_NULL_SENSOR;
        mLmvoIsFirst  = 1;
        mLMVTuning = NULL;

    }
    else
    {
        MY_LOGD0("has %d users ", mUsers);
    }
    mTsForAlgoDebug = 0;

    return err;
}

MINT32 LMVDrvImp::ConfigLMVReg(const MUINT32 &aSensorTg)
{
    MY_TRACE_FUNC_LIFE();

    if( mIsConfig == 1 )
    {
        MY_LOGD1("configured");
        return LMV_RETURN_NO_ERROR;
    }
    else
    {
        if( mLMVTuning == NULL )
        {
            mLMVTuning = new LMVTuning(this, mSensorIdx);
        }
        else
        {
            MY_LOGW("LMVTuning has been created!");
        }

        if( mLMVTuning != NULL )
        {
            if( mLMVTuning->isSupportLMVCb() )
            {
                LMVP1Cb* lmvCb = mLMVTuning->getLMVCb();
                if( lmvCb != NULL )
                {
                    MY_TRACE_TAG_LIFE("LMVDrv::SetLMVCallback");
                    m_pNormalPipe->sendCommand(ENPipeCmd_SET_EIS_CBFP, (MINTPTR)lmvCb, -1, -1);
                }
                else
                {
                    MY_LOGE("Cannot get LMVP1Cb!");
                }
            }
            if( mLMVTuning->isSupportSGG2Cb() )
            {
                SGG2P1Cb* sgg2Cb = mLMVTuning->getSGG2Cb();
                if( sgg2Cb != NULL )
                {
                    MY_TRACE_TAG_LIFE("LMVDrv::SetSGG2Callback");
                    m_pNormalPipe->sendCommand(ENPipeCmd_SET_SGG2_CBFP, (MINTPTR)sgg2Cb, -1, -1);
                }
                else
                {
                    MY_LOGE("Cannot get SGG2P1Cb!");
                }
            }
        }

        mSensorTg = aSensorTg;

        mIsConfig = 1;

        MY_LOGD0("- TG(%u)", mSensorTg);
        return LMV_RETURN_NO_ERROR;
    }
}

MUINT32 LMVDrvImp::GetFirstFrameInfo()
{
    return mIsFirst ? 0 : 1;
}

MUINT32 LMVDrvImp::Get2PixelMode()
{
    return mIs2Pixel;
}

MVOID LMVDrvImp::GetLMVInputSize(MUINT32 *aWidth, MUINT32 *aHeight)
{
    *aWidth  = mImgWidth;
    *aHeight = mImgHeight;
}

MUINT32 LMVDrvImp::GetLMVDivH()
{
    MY_LOGD1("mLmvDivH(%u)", mLmvDivH);
    return mLmvDivH;
}

MUINT32 LMVDrvImp::GetLMVDivV()
{
    MY_LOGD1("mLmvDivV(%u)", mLmvDivV);
    return mLmvDivV;
}

MUINT32 LMVDrvImp::GetLMVMaxGmv()
{
    MY_LOGD1("mMaxGmv(%u)", mMaxGmv);
    return mMaxGmv;
}

MUINT32 LMVDrvImp::GetLMVMbNum()
{
    MY_LOGD1("mTotalMBNum(%u)", mTotalMBNum);
    return mTotalMBNum;
}

MBOOL LMVDrvImp::GetLMVSupportInfo(const MUINT32 &aSensorIdx)
{
    MY_TRACE_FUNC_LIFE();

    NormalPipe_EIS_Info lmvInfo;
    lmvInfo.mSupported = MFALSE;

    m_pNormalPipe->sendCommand(ENPipeCmd_GET_EIS_INFO, aSensorIdx, (MINTPTR)&lmvInfo, -1);

    MY_LOGD1("sensorIdx(%u),support(%d)", aSensorIdx, lmvInfo.mSupported);

    mLmvHwSupport = lmvInfo.mSupported;
    return lmvInfo.mSupported;
}

MINT64 LMVDrvImp::GetTsForAlgoDebug()
{
    return mTsForAlgoDebug;
}

MVOID LMVDrvImp::GetRegSetting(void *data)
{
    GYRO_INIT_INFO_STRUCT *a_pGisCfgData = (GYRO_INIT_INFO_STRUCT *)data;

    a_pGisCfgData->isBinning = MFALSE; //Deprecated. No used now

    a_pGisCfgData->GyroCalInfo.PIXEL_MODE = (1 << (mIs2Pixel));
    a_pGisCfgData->GyroCalInfo.EIS_OP_H_step = mLmvDivH;
    a_pGisCfgData->GyroCalInfo.EIS_OP_V_step = mLmvDivV;

    a_pGisCfgData->GyroCalInfo.EIS_H_win_num = (mLmvRegSetting.reg_lmv_prep_me_ctrl1 >>25)&0x7;
    a_pGisCfgData->GyroCalInfo.EIS_V_win_num = (mLmvRegSetting.reg_lmv_prep_me_ctrl1 >>28)&0xF;

    a_pGisCfgData->GyroCalInfo.EIS_RP_H_num = (mLmvRegSetting.reg_lmv_prep_me_ctrl1 >>8)&0x1F;
    a_pGisCfgData->GyroCalInfo.EIS_RP_V_num = (mLmvRegSetting.reg_lmv_prep_me_ctrl1 >>21)&0xF;

    a_pGisCfgData->GyroCalInfo.EIS_H_win_size = (mLmvRegSetting.reg_lmv_mb_interval >> 16) ;
    a_pGisCfgData->GyroCalInfo.EIS_V_win_size = (mLmvRegSetting.reg_lmv_mb_interval &0xFFFF) ;

    a_pGisCfgData->GyroCalInfo.EIS_Left_offset = mLmvRegSetting.reg_lmv_mb_offset >> 16;
    a_pGisCfgData->GyroCalInfo.EIS_Top_offset = mLmvRegSetting.reg_lmv_mb_offset & 0xFFFF;

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

MINT32 LMVDrvImp::GetLmvHwStatistic(MINTPTR bufferVA, EIS_STATISTIC_STRUCT *apLmvStat)
{
    if( !bufferVA )
    {
        MY_LOGD0("bufferVA(%p) is NULL!!!", (void*)bufferVA);
        return LMV_RETURN_EISO_MISS;
    }
    MUINT32 *pLmvoAddr = (MUINT32 *)bufferVA;

    for( int i = 0; i < LMV_MAX_WIN_NUM; ++i )
    {
        if( i != 0 )
        {
            pLmvoAddr += 2;  // 64bits(8bytes)
        }

        apLmvStat->i4LMV_X2[i]   = Complement2(*pLmvoAddr & 0x1F, 5);               //[0:4]
        apLmvStat->i4LMV_Y2[i]   = Complement2(((*pLmvoAddr & 0x3E0) >> 5), 5);    //[5:9]
        apLmvStat->SAD[i]        = (*pLmvoAddr & 0x7FC00) >> 10;                    //[10:18]
        apLmvStat->NewTrust_X[i] = (*pLmvoAddr & 0x03F80000) >> 19;                 //[19:25]
        apLmvStat->NewTrust_Y[i] = ((*pLmvoAddr & 0xFC000000) >> 26) + ((*(pLmvoAddr+1) & 0x00000001) << 6);    //[26:32]
        apLmvStat->i4LMV_X[i]    = Complement2(((*(pLmvoAddr + 1) & 0x00003FFE) >> 1), 13);     //[33:45] -> [1:13]
        apLmvStat->i4LMV_Y[i]    = Complement2(((*(pLmvoAddr + 1) & 0x07FFC000) >> 14), 13);    //[46:58] -> [14:26]
        apLmvStat->SAD2[i]       = 0;
        apLmvStat->AVG_SAD[i]    = 0;

        MY_LOGD3("LMV[%d]Addr(%p)=lmv(%d,%d),lmv2(%d,%d),trust(%d,%d),sad(%d)",
                    i, pLmvoAddr,
                    apLmvStat->i4LMV_X[i], apLmvStat->i4LMV_Y[i],
                    apLmvStat->i4LMV_X2[i], apLmvStat->i4LMV_Y2[i],
                    apLmvStat->NewTrust_X[i], apLmvStat->NewTrust_Y[i],
                    apLmvStat->SAD[i]);
    }

    return LMV_RETURN_NO_ERROR;
}

MINT32 LMVDrvImp::Complement2(MUINT32 value, MUINT32 digit)
{
    MINT32 Result = 0;
    if( ((value >> (digit - 1)) & 0x1) == 1 )    // negative
    {
        Result = 0 - (MINT32)((~value + 1) & ((1 << digit) - 1));
    }
    else
    {
        Result = (MINT32)(value & ((1 << digit) - 1));
    }
    return Result;
}

MVOID LMVDrvImp::BoundaryCheck(MUINT32 &aInput,const MUINT32 &upBound,const MUINT32 &lowBound)
{
    if( aInput > upBound )
    {
        aInput = upBound;
    }
    if( aInput < lowBound )
    {
        aInput = lowBound;
    }
}

MINT64 LMVDrvImp::GetTimeStamp(const MUINT32 &aSec,const MUINT32 &aUs)
{
    return  aSec * 1000000000LL + aUs * 1000LL;
}
