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
#define LOG_TAG "gma_mgr"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include <camera_custom_nvram.h>
#include <isp_tuning.h>
#include <awb_param.h>
#include <ae_param.h>
#include <af_param.h>
#include <flash_param.h>
#include <isp_tuning_cam_info.h>
#include <isp_tuning_idx.h>
#include <isp_tuning_custom.h>
#include <isp_mgr.h>
#include <isp_mgr_helper.h>
#include "gma_mgr.h"
#include <isp_tuning_mgr.h>
#include <private/aaa_hal_private.h>
#include <private/aaa_utils.h>
#include <array>

using namespace NSIspTuning;
using namespace NSIspTuningv3;

#define MY_INST NS3Av3::INST_T<GmaMgr>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

namespace NSIspTuningv3
{
GmaMgr*
GmaMgr::
createInstance(ESensorDev_T const eSensorDev, ISP_NVRAM_TONEMAP_PARAM_T& rIspNvramReg, ISP_NVRAM_GMA_TUNING_PARAM_T* pIspNvramGmaPara)
{
    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(static_cast<MINT32>(eSensorDev));

    if(i4SensorIdx >= SENSOR_IDX_MAX || i4SensorIdx < 0) {
        CAM_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorIdx);
        return NULL;
    }

    MY_INST& rSingleton = gMultiton[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<GmaMgr>(eSensorDev, rIspNvramReg, pIspNvramGmaPara);
    } );

    return rSingleton.instance.get();
}

GmaMgr::GmaMgr(ESensorDev_T const eSensorDev, ISP_NVRAM_TONEMAP_PARAM_T& rIspNvramReg, ISP_NVRAM_GMA_TUNING_PARAM_T* pIspNvramGmaPara)
    : m_rIspNvramReg  (rIspNvramReg)
    , m_eSensorDev  (eSensorDev)
    , m_pIspNvramGmaPara (pIspNvramGmaPara)
    //, m_pGmaAlgo (MTKGma::createInstance())
    , mGmaScenario (0) //E_GMA_SCENARIO_PREVIEW
    , mbAEStable (MFALSE)
    , mLogEn (0)
    , m_ePrev_Tuning_Set(MTKGMA_TUNING_SET_Preview)
    , mfgGMA_HDR(MFALSE)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.gma_algo.enable", value, "1");
    mAlgoEn = atoi(value);

    property_get("vendor.debug.dynamic_gma.log", value, "0");
    mLogEn = atoi(value);

    memset(&mAEInfo, 0, sizeof(AE_INFO_T));
    memset(&mOutExif, 0, sizeof(MTK_GMA_EXIF_INFO_STRUCT));
    memset(&mOutGGM, 0, sizeof(MTK_GMA_RESULT_INFO_STRUCT));
    memset(&mInInfo, 0, sizeof(MTK_GMA_PROC_INFO_STRUCT));

    CAM_LOGD_IF(1, "[%s()] before MTKGma::createInstance", __FUNCTION__);

    if (mAlgoEn) m_pGmaAlgo = MTKGma::createInstance(static_cast<eGMASensorDev_T>(m_eSensorDev));
    else m_pGmaAlgo = NULL;

    if (m_pGmaAlgo)
    CAM_LOGD_IF(1, "[%s()] m_pGmaAlgo(%p), m_eSensorDev(%d), m_pIspNvramGmaPara(%p)", __FUNCTION__, m_pGmaAlgo, m_eSensorDev, m_pIspNvramGmaPara);

    if (mAlgoEn) {
        CAM_ULOG_ASSERT(Utils::ULog::MOD_ISP_MGR, m_pGmaAlgo != nullptr, "[%s]: m_pGmaAlgo is nullptr!", __FUNCTION__);
        m_pGmaAlgo->GmaReset();
    }

    CAM_LOGD_IF(1, "[%s()] after GmaReset", __FUNCTION__);

    for (int tb_num = 0; tb_num<GMA_TBL_NUM; tb_num++)
    {
        for (int i = 0; i<GGM_LUT_SIZE; i++)
        {
            mGMA_Normal_Tbl.i4EncNVRAMGMA[tb_num][i] = m_rIspNvramReg.GGM[tb_num].lut_g.lut[i].GGM_G;
            mGMA_HDR_Tbl.i4EncNVRAMGMA[tb_num][i] = m_rIspNvramReg.IHDR_GGM[tb_num].lut_g.lut[i].GGM_G;
            mEnv.rNVRAMGmaParam.i4EncNVRAMGMA[tb_num][i] = mGMA_Normal_Tbl.i4EncNVRAMGMA[tb_num][i];
            //How about R and B channels
        }
        for(int i=GGM_LUT_SIZE; i<GMA_TBL_SIZE; i++)
        {
           mGMA_Normal_Tbl.i4EncNVRAMGMA[tb_num][i] = 0xFFFF;
           mGMA_HDR_Tbl.i4EncNVRAMGMA[tb_num][i] = 0xFFFF;
           mEnv.rNVRAMGmaParam.i4EncNVRAMGMA[tb_num][i] = mGMA_Normal_Tbl.i4EncNVRAMGMA[tb_num][i];
        }
    }

    //mEnv.rTuningGmaParam = static_cast<MTK_GMA_TUNING_PARAM_T*>pIspNvramGmaPara;
    mEnv.rTuningGmaParam = *((MTK_GMA_TUNING_PARAM_T*)&(m_pIspNvramGmaPara[MTKGMA_TUNING_SET_Preview]));
    mEnv.eGMACamMode = MTKGMA_TUNING_SET_Preview;
    //FIXME!!!!!!!!!!!!!!!
    //mEnv.rTuningGmaParam.eGMAMode = eFIXED_GMA_MODE;

    CAM_LOGD_IF(1, "[%s()] before GmaInit", __FUNCTION__);

    if (mAlgoEn) m_pGmaAlgo->GmaInit(&mEnv, NULL);

    CAM_LOGD_IF(1, "[%s()] after GmaInit", __FUNCTION__);

}


GmaMgr::~GmaMgr()
{
   CAM_LOGD_IF(1, "[%s()] before GmaExit (%p)", __FUNCTION__, m_pGmaAlgo);
   if (mAlgoEn) m_pGmaAlgo->GmaExit();
   CAM_LOGD_IF(1, "[%s()] after GmaExit", __FUNCTION__);
   if (mAlgoEn) m_pGmaAlgo->destroyInstance(m_pGmaAlgo);
   CAM_LOGD_IF(1, "[%s()] after destroyInstance (%p)", __FUNCTION__, m_pGmaAlgo);
}

MVOID
GmaMgr::
printAEInfo(AE_INFO_T const & rAEInfo, const char* username)
{
        CAM_LOGD("%s: [%s] rAEInfo: %d/%d/%d/%d/%d; %d/%d/%d/%d/%d; %d/%d/%d/%d/%d; %d/%d/%d;"
            , __FUNCTION__
            , username

            , rAEInfo.u4AETarget
            , rAEInfo.u4AECurrentTarget
            , rAEInfo.u4Eposuretime
            , rAEInfo.u4AfeGain
            , rAEInfo.u4IspGain

            , rAEInfo.u4RealISOValue
            , rAEInfo.i4LightValue_x10
            , rAEInfo.u4AECondition
            , rAEInfo.i2FlareOffset
            , rAEInfo.i4GammaIdx

            , rAEInfo.i4LESE_Ratio
            , rAEInfo.u4SWHDR_SE
            , rAEInfo.u4MaxISO
            , rAEInfo.u4AEStableCnt
            , rAEInfo.u4OrgExposuretime

            , rAEInfo.u4OrgRealISOValue
            , rAEInfo.bGammaEnable
            , rAEInfo.u4EVRatio
            );

    /*
        typedef struct {
            MUINT32 u4AETarget;
            MUINT32 u4AECurrentTarget;
            MUINT32 u4Eposuretime;   //!<: Exposure time in ms
            MUINT32 u4AfeGain;           //!<: raw gain
            MUINT32 u4IspGain;           //!<: sensor gain
            MUINT32 u4RealISOValue;
            MINT32  i4LightValue_x10;
            MUINT32 u4AECondition;
            MINT16  i2FlareOffset;
            MINT32  i4GammaIdx;   // next gamma idx
            MINT32  i4LESE_Ratio;    // LE/SE ratio
            MUINT32 u4SWHDR_SE;      //for sw HDR SE ,  -x EV , compare with converge AE
            MUINT32 u4MaxISO;
            MUINT32 u4AEStableCnt;
            MUINT32 u4OrgExposuretime;   //!<: Exposure time in ms
            MUINT32 u4OrgRealISOValue;
            MUINT16 u2Histogrm[GMA_AE_HISTOGRAM_BIN];
            MBOOL bGammaEnable;

            MINT32 i4AEStable;
            MINT32 i4EVRatio;
        } GMA_AE_INFO_T;
    */

        const MUINT32* pHist = &rAEInfo.u4Histogrm[0];
        for (int i=0; i<120; i+=10)
        {
            CAM_LOGD( "%s: rAEInfo.u4Histogrm[%d-%d]=(%x,%x,%x,%x,%x,  %x,%x,%x,%x,%x)"
                , __FUNCTION__
                , i
                , i+9
                , pHist[i], pHist[i+1], pHist[i+2], pHist[i+3], pHist[i+4]
                , pHist[i+5], pHist[i+6], pHist[i+7], pHist[i+8], pHist[i+9]);
        }
        //(i == 120)
        {
            int i=120;
            CAM_LOGD( "%s: rAEInfo.u4Histogrm[%d-%d]=(%x,%x,%x,%x,%x,  %x,%x,%x)"
                , __FUNCTION__
                , i
                , i+7
                , pHist[i], pHist[i+1], pHist[i+2], pHist[i+3], pHist[i+4]
                , pHist[i+5], pHist[i+6], pHist[i+7]);
        }

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
GmaMgr::
stop()
{
    CAM_LOGD_IF(mLogEn, "[%s()] mGmaScenario(%d)", __FUNCTION__, mGmaScenario);
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
GmaMgr::
start()
{
#if 0
    switch (mGmaScenario)
    {
    case IspTuningMgr::E_GMA_SCENARIO_PREVIEW:
        mInInfo.eSensorMode = eGMA_PREVIEW_MODE;
        break;
    case IspTuningMgr::E_GMA_SCENARIO_CAPTURE:
        mInInfo.eSensorMode = eGMA_CAPTURE_MODE;
        break;
    case IspTuningMgr::E_GMA_SCENARIO_VIDEO:
        mInInfo.eSensorMode = eGMA_VIDEO_MODE;
        break;
    default:
        CAM_LOGE("%s: scenario not supported (%d)", __FUNCTION__, mGmaScenario);
        break;
    }
    CAM_LOGD_IF(mLogEn, "[%s()] m_eSensorDev(%d), mGmaScenario(%d)", __FUNCTION__, m_eSensorDev, mGmaScenario);
#endif
/*
    typedef struct {
        MUINT32 u4AETarget;
        MUINT32 u4AECurrentTarget;
        MUINT32 u4Eposuretime;   //!<: Exposure time in ms
        MUINT32 u4AfeGain;           //!<: raw gain
        MUINT32 u4IspGain;           //!<: sensor gain
        MUINT32 u4RealISOValue;
        MINT32  i4LightValue_x10;
        MUINT32 u4AECondition;
        MINT16  i2FlareOffset;
        MINT32  i4GammaIdx;   // next gamma idx
        MINT32  i4LESE_Ratio;    // LE/SE ratio
        MUINT32 u4SWHDR_SE;      //for sw HDR SE ,  -x EV , compare with converge AE
        MUINT32 u4MaxISO;
        MUINT32 u4AEStableCnt;
        MUINT32 u4OrgExposuretime;   //!<: Exposure time in ms
        MUINT32 u4OrgRealISOValue;
        MUINT16 u2Histogrm[GMA_AE_HISTOGRAM_BIN];
        MBOOL bGammaEnable;

        MINT32 i4AEStable;
        MINT32 i4EVRatio;
    } GMA_AE_INFO_T;
*/

    //mInInfo.eGmaProfile = (eGMA_PROFILE_T)eIspProfile;
    mInInfo.rGMAAEInfo.u4AETarget= mAEInfo.u4AETarget;
    mInInfo.rGMAAEInfo.u4AECurrentTarget= mAEInfo.u4AECurrentTarget;
    mInInfo.rGMAAEInfo.u4Eposuretime= mAEInfo.u4Eposuretime;
    mInInfo.rGMAAEInfo.u4AfeGain= mAEInfo.u4AfeGain;
    mInInfo.rGMAAEInfo.u4IspGain= mAEInfo.u4IspGain;
    mInInfo.rGMAAEInfo.u4RealISOValue= mAEInfo.u4RealISOValue;
    mInInfo.rGMAAEInfo.i4LightValue_x10= mAEInfo.i4LightValue_x10;
    mInInfo.rGMAAEInfo.u4AECondition= mAEInfo.u4AECondition;
    mInInfo.rGMAAEInfo.i2FlareOffset= mAEInfo.i2FlareOffset;
    mInInfo.rGMAAEInfo.i4GammaIdx= mAEInfo.i4GammaIdx;
    mInInfo.rGMAAEInfo.i4LESE_Ratio= mAEInfo.i4LESE_Ratio;
    mInInfo.rGMAAEInfo.u4SWHDR_SE= mAEInfo.u4SWHDR_SE;
    mInInfo.rGMAAEInfo.u4MaxISO= mAEInfo.u4MaxISO;
    mInInfo.rGMAAEInfo.u4AEStableCnt= mAEInfo.u4AEStableCnt;
    mInInfo.rGMAAEInfo.u4OrgExposuretime= mAEInfo.u4OrgExposuretime;
    mInInfo.rGMAAEInfo.u4OrgRealISOValue= mAEInfo.u4OrgRealISOValue;

    for (int i = 0; i<GMA_AE_HISTOGRAM_BIN; i++)
    {
        mInInfo.rGMAAEInfo.u4Histogrm[i]= mAEInfo.u4Histogrm[i];
    }
    for (int i = 0; i<GGM_LUT_SIZE; i++)
    {
        mInInfo.i4CurrEncGMA[i] = m_rIspNvramReg.GGM[0].lut_g.lut[i].GGM_G;
    }

    mInInfo.rGMAAEInfo.bGammaEnable = mAEInfo.bGammaEnable;

    mInInfo.rGMAAEInfo.i4AEStable = mAEInfo.bAEStable;
    mInInfo.rGMAAEInfo.i4EVRatio = mAEInfo.u4EVRatio; //1024; //FIXME: 1024

    m_ePrev_Tuning_Set = MTKGMA_TUNING_SET_Preview;
    mfgGMA_HDR = MFALSE;

    for (int tb_num = 0; tb_num<GMA_TBL_NUM; tb_num++)
    {
        for (int i = 0; i<GGM_LUT_SIZE; i++)
        {
            mGMA_Normal_Tbl.i4EncNVRAMGMA[tb_num][i] = m_rIspNvramReg.GGM[tb_num].lut_g.lut[i].GGM_G;
            mGMA_HDR_Tbl.i4EncNVRAMGMA[tb_num][i] = m_rIspNvramReg.IHDR_GGM[tb_num].lut_g.lut[i].GGM_G;
            mEnv.rNVRAMGmaParam.i4EncNVRAMGMA[tb_num][i] = mGMA_Normal_Tbl.i4EncNVRAMGMA[tb_num][i];
            //How about R and B channels
        }
        for(int i=GGM_LUT_SIZE; i<GMA_TBL_SIZE; i++)
        {
           mGMA_Normal_Tbl.i4EncNVRAMGMA[tb_num][i] = 0xFFFF;
           mGMA_HDR_Tbl.i4EncNVRAMGMA[tb_num][i] = 0xFFFF;
           mEnv.rNVRAMGmaParam.i4EncNVRAMGMA[tb_num][i] = mGMA_Normal_Tbl.i4EncNVRAMGMA[tb_num][i];
        }
    }

    mEnv.rTuningGmaParam = *((MTK_GMA_TUNING_PARAM_T*)&(m_pIspNvramGmaPara[MTKGMA_TUNING_SET_Preview]));
    mEnv.eGMACamMode = MTKGMA_TUNING_SET_Preview;

    if (mAlgoEn) m_pGmaAlgo->GmaFeatureCtrl(MTKGMA_FEATURE_SET_ENV_INFO, &mEnv, NULL);
    if (mAlgoEn) m_pGmaAlgo->GmaFeatureCtrl(MTKGMA_FEATURE_SET_PROC_INFO, &mInInfo, NULL);

    CAM_LOGD_IF(mLogEn, "[%s()] after MTKGMA_FEATURE_SET_PROC_INFO", __FUNCTION__);

}




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
GmaMgr::
calculateGGM(ISP_NVRAM_GGM_T* pGGMReg, NSIspExifDebug::IspExifDebugInfo_T::IspGmaInfo* pGmaExif, AE_INFO_T const& rAEInfo, EIspProfile_T eIspProfile)
{
    eMTKGMA_TUNING_SET_T GMA_Tuning_Set;
    MBOOL fgGMA_HDR;

    switch (eIspProfile)
    {
        case EIspProfile_Preview:
            GMA_Tuning_Set = MTKGMA_TUNING_SET_Preview;
            fgGMA_HDR = MFALSE;
        break;
        case EIspProfile_Video:
            GMA_Tuning_Set = MTKGMA_TUNING_SET_Video;
            fgGMA_HDR = MFALSE;
        break;
        case EIspProfile_Capture:
        case EIspProfile_Flash_Capture:
            GMA_Tuning_Set = MTKGMA_TUNING_SET_Capture;
            fgGMA_HDR = MFALSE;
        break;
        case EIspProfile_iHDR_Preview:
        case EIspProfile_zHDR_Preview:
        case EIspProfile_mHDR_Preview:
        case EIspProfile_iHDR_Preview_VSS:
        case EIspProfile_zHDR_Preview_VSS:
        case EIspProfile_mHDR_Preview_VSS:
        case EIspProfile_EIS_iHDR_Preview:
        case EIspProfile_EIS_zHDR_Preview:
        case EIspProfile_EIS_mHDR_Preview:
            GMA_Tuning_Set = MTKGMA_TUNING_SET_HDR_Preview;
            fgGMA_HDR = MTRUE;
        break;
        case EIspProfile_iHDR_Video:
        case EIspProfile_zHDR_Video:
        case EIspProfile_mHDR_Video:
        case EIspProfile_iHDR_Video_VSS:
        case EIspProfile_zHDR_Video_VSS:
        case EIspProfile_mHDR_Video_VSS:
        case EIspProfile_EIS_iHDR_Video:
        case EIspProfile_EIS_zHDR_Video:
        case EIspProfile_EIS_mHDR_Video:
            GMA_Tuning_Set = MTKGMA_TUNING_SET_HDR_Video;
            fgGMA_HDR = MTRUE;
        break;
    case EIspProfile_zHDR_Capture:
        case EIspProfile_mHDR_Capture:
        case EIspProfile_zHDR_Flash_Capture:
            GMA_Tuning_Set = MTKGMA_TUNING_SET_HDR_Capture;
            fgGMA_HDR = MTRUE;
            break;
        case EIspProfile_Auto_iHDR_Preview:
        case EIspProfile_Auto_zHDR_Preview:
        case EIspProfile_Auto_mHDR_Preview:
        case EIspProfile_Auto_iHDR_Preview_VSS:
        case EIspProfile_Auto_zHDR_Preview_VSS:
        case EIspProfile_Auto_mHDR_Preview_VSS:
        case EIspProfile_EIS_Auto_iHDR_Preview:
        case EIspProfile_EIS_Auto_zHDR_Preview:
        case EIspProfile_EIS_Auto_mHDR_Preview:
            GMA_Tuning_Set = MTKGMA_TUNING_SET_AutoHDR_Preview;
            fgGMA_HDR = MTRUE;
            break;
        case EIspProfile_Auto_iHDR_Video:
        case EIspProfile_Auto_zHDR_Video:
        case EIspProfile_Auto_mHDR_Video:
        case EIspProfile_Auto_iHDR_Video_VSS:
        case EIspProfile_Auto_zHDR_Video_VSS:
        case EIspProfile_Auto_mHDR_Video_VSS:
        case EIspProfile_EIS_Auto_iHDR_Video:
        case EIspProfile_EIS_Auto_zHDR_Video:
        case EIspProfile_EIS_Auto_mHDR_Video:
            GMA_Tuning_Set = MTKGMA_TUNING_SET_AutoHDR_Video;
            fgGMA_HDR = MTRUE;
        break;
        case EIspProfile_Auto_zHDR_Capture:
        case EIspProfile_Auto_mHDR_Capture:
            GMA_Tuning_Set = MTKGMA_TUNING_SET_AutoHDR_Capture;
            fgGMA_HDR = MTRUE;
        break;
    default:
            GMA_Tuning_Set = MTKGMA_TUNING_SET_Preview;
            fgGMA_HDR = MFALSE;
        break;
    }

    if( GMA_Tuning_Set != m_ePrev_Tuning_Set){

        m_ePrev_Tuning_Set = GMA_Tuning_Set;
        mEnv.rTuningGmaParam = *((MTK_GMA_TUNING_PARAM_T*)&(m_pIspNvramGmaPara[m_ePrev_Tuning_Set]));
        mEnv.eGMACamMode = m_ePrev_Tuning_Set;

        if(fgGMA_HDR != mfgGMA_HDR){
            mfgGMA_HDR = fgGMA_HDR;
            if(mfgGMA_HDR){
                mEnv.rNVRAMGmaParam = mGMA_HDR_Tbl;
            }
            else{
                mEnv.rNVRAMGmaParam = mGMA_Normal_Tbl;
            }
        }

        if (mAlgoEn){
           m_pGmaAlgo->GmaInit(&mEnv, NULL);
           m_pGmaAlgo->GmaFeatureCtrl(MTKGMA_FEATURE_SET_ENV_INFO, &mEnv, NULL);
        }
    }

/*
    typedef struct {
        MUINT32 u4AETarget;
        MUINT32 u4AECurrentTarget;
        MUINT32 u4Eposuretime;   //!<: Exposure time in ms
        MUINT32 u4AfeGain;           //!<: raw gain
        MUINT32 u4IspGain;           //!<: sensor gain
        MUINT32 u4RealISOValue;
        MINT32  i4LightValue_x10;
        MUINT32 u4AECondition;
        MINT16  i2FlareOffset;
        MINT32  i4GammaIdx;   // next gamma idx
        MINT32  i4LESE_Ratio;    // LE/SE ratio
        MUINT32 u4SWHDR_SE;      //for sw HDR SE ,  -x EV , compare with converge AE
        MUINT32 u4MaxISO;
        MUINT32 u4AEStableCnt;
        MUINT32 u4OrgExposuretime;   //!<: Exposure time in ms
        MUINT32 u4OrgRealISOValue;
        MUINT16 u2Histogrm[GMA_AE_HISTOGRAM_BIN];
        MBOOL bGammaEnable;

        MINT32 i4AEStable;
        MINT32 i4EVRatio;
    } GMA_AE_INFO_T;
*/
    MUINT32 u4_IspProfile_temp = (MUINT32)eIspProfile;
    mInInfo.eGmaProfile = static_cast<eGMA_PROFILE_T>(u4_IspProfile_temp);

    setAEInfo(rAEInfo);

    for (int i = 0; i<GGM_LUT_SIZE; i++)
    {
        mInInfo.i4CurrEncGMA[i] = pGGMReg->lut_g.lut[i].GGM_G;
    }

    if (mLogEn) printInfo();

    if (mAlgoEn) m_pGmaAlgo->GmaFeatureCtrl(MTKGMA_FEATURE_SET_PROC_INFO, &mInInfo, NULL);
    if (mAlgoEn) m_pGmaAlgo->GmaMain();
    if (mAlgoEn) m_pGmaAlgo->GmaFeatureCtrl(MTKGMA_FEATURE_GET_RESULT, NULL, &mOutGGM);
    if (mAlgoEn) m_pGmaAlgo->GmaFeatureCtrl(MTKGMA_FEATURE_GET_EXIF, NULL, &mOutExif);

    if (mLogEn)
    {
        CAM_LOGD_IF(1, "[%s()] m_eSensorDev(%d) after GmaMain()", __FUNCTION__, m_eSensorDev);
        printInfo();
        printAEInfo(mAEInfo, "mAEInfo_afterAlgo");
    }

    if (mAlgoEn)
    {
        for (int i = 0; i<(GGM_LUT_SIZE); i++)
        {
            pGGMReg->lut_rb.lut[i].GGM_R = pGGMReg->lut_rb.lut[i].GGM_B = pGGMReg->lut_g.lut[i].GGM_G = mOutGGM.i4EncGMA[i];
            pGGMReg->lut_g.lut[i].rsv_10 = 0;
            pGGMReg->lut_rb.lut[i].rsv_10 = 0;
            pGGMReg->lut_rb.lut[i].rsv_26 = 0;

            if (mLogEn)
            {
                if (((i%10) == 0) && (i!=140))
                {
                    CAM_LOGD( "%s: mOutGGM.i4EncGMA[%d-%d]=(%x,%x,%x,%x,%x,  %x,%x,%x,%x,%x)"
                        , __FUNCTION__
                        , i
                        , i+9
                        , mOutGGM.i4EncGMA[i], mOutGGM.i4EncGMA[i+1], mOutGGM.i4EncGMA[i+2], mOutGGM.i4EncGMA[i+3], mOutGGM.i4EncGMA[i+4]
                        , mOutGGM.i4EncGMA[i+5], mOutGGM.i4EncGMA[i+6], mOutGGM.i4EncGMA[i+7], mOutGGM.i4EncGMA[i+8], mOutGGM.i4EncGMA[i+9]);
                }
                else if (i == 140)
                {
                    CAM_LOGD( "%s: mOutGGM.i4EncGMA[%d-%d]=(%x,%x,%x,%x)"
                        , __FUNCTION__
                        , i
                        , i+3
                        , mOutGGM.i4EncGMA[i], mOutGGM.i4EncGMA[i+1], mOutGGM.i4EncGMA[i+2], mOutGGM.i4EncGMA[i+3]);

                }
            }
        }
        ::memcpy(pGmaExif, &mOutExif, sizeof(NSIspExifDebug::IspExifDebugInfo_T::IspGmaInfo));

        if (mLogEn)
        {
            CAM_LOGD("%s: GMA EXIF:  %x/%x/%x; %x/%x/%x/%x/%x; %x/%x/%x/%x/%x; %x/%x/%x/%x/%x; %x/%x/%x/%x/%x; %x/%x/%x/%x/%x; %x/%x/%x/%x/%x;"
                , __FUNCTION__

                , mOutExif.i4GMAMode
                , mOutExif.i4GMACamMode
                , mOutExif.i4GmaProfile
                , mOutExif.i4ChipVersion
                , mOutExif.i4MainVersion
                , mOutExif.i4SubVersion
                , mOutExif.i4SystemVersion
                , mOutExif.i4EVRatio
                , mOutExif.i4LowContrastThr
                , mOutExif.i4LowContrastRatio
                , mOutExif.i4LowContrastSeg
                , mOutExif.i4Contrast
                , mOutExif.i4Contrast_L
                , mOutExif.i4Contrast_H
                , mOutExif.i4HdrContrastWeight
                , mOutExif.i4EVContrastY
                , mOutExif.i4ContrastY_L
                , mOutExif.i4ContrastY_H
                , mOutExif.i4NightContrastWeight
                , mOutExif.i4LV
                , mOutExif.i4LV_L
                , mOutExif.i4LV_H
                , mOutExif.i4HdrLVWeight
                , mOutExif.i4NightLVWeight
                , mOutExif.i4SmoothEnable
                , mOutExif.i4SmoothSpeed
                , mOutExif.i4SmoothWaitAE
                , mOutExif.i4GMACurveEnable

                , mOutExif.i4CenterPt
                , mOutExif.i4LowCurve
                , mOutExif.i4SlopeL
                , mOutExif.i4FlareEnable
                , mOutExif.i4FlareOffset);

        /*
            typedef struct
            {
                MINT32 i4GMAMode;
                MINT32 i4GmaProfile;
                MINT32 i4ChipVersion;
                MINT32 i4MainVersion;
                MINT32 i4SubVersion;
                MINT32 i4SystemVersion;
                MINT32 i4EVRatio;
                MINT32 i4LowContrastThr;
                MINT32 i4LowContrastRatio;
                MINT32 i4LowContrastSeg;
                MINT32 i4Contrast;
                MINT32 i4Contrast_L;
                MINT32 i4Contrast_H;
                MINT32 i4HdrContrastWeight;
                MINT32 i4EVContrastY;
                MINT32 i4ContrastY_L;
                MINT32 i4ContrastY_H;
                MINT32 i4NightContrastWeight;
                MINT32 i4LV;
                MINT32 i4LV_L;
                MINT32 i4LV_H;
                MINT32 i4HdrLVWeight;
                MINT32 i4NightLVWeight;
                MINT32 i4SmoothEnable;
                MINT32 i4SmoothSpeed;
                MINT32 i4SmoothWaitAE;
                MINT32 i4GMACurveEnable;
                MINT32 i4CenterPt;
                MINT32 i4LowCurve;
                MINT32 i4SlopeL;
                MINT32 i4FlareEnable;
                MINT32 i4FlareOffset;
            } MTK_GMA_EXIF_INFO_STRUCT, *P_MTK_GMA_EXIF_INFO_STRUCT;
        */



        }
    }
}

MVOID GmaMgr::printInfo() const
{

    CAM_LOGD("%s: mInInfo(%d): %d/%d/%d/%d/%d; %d/%d/%d/%d/%d; %d/%d/%d/%d/%d; %d/%d/%d/%d;"
        , __FUNCTION__
        , mInInfo.eGmaProfile

        , mInInfo.rGMAAEInfo.u4AETarget
        , mInInfo.rGMAAEInfo.u4AECurrentTarget
        , mInInfo.rGMAAEInfo.u4Eposuretime
        , mInInfo.rGMAAEInfo.u4AfeGain
        , mInInfo.rGMAAEInfo.u4IspGain

        , mInInfo.rGMAAEInfo.u4RealISOValue
        , mInInfo.rGMAAEInfo.i4LightValue_x10
        , mInInfo.rGMAAEInfo.u4AECondition
        , mInInfo.rGMAAEInfo.i2FlareOffset
        , mInInfo.rGMAAEInfo.i4GammaIdx

        , mInInfo.rGMAAEInfo.i4LESE_Ratio
        , mInInfo.rGMAAEInfo.u4SWHDR_SE
        , mInInfo.rGMAAEInfo.u4MaxISO
        , mInInfo.rGMAAEInfo.u4AEStableCnt
        , mInInfo.rGMAAEInfo.u4OrgExposuretime

        , mInInfo.rGMAAEInfo.u4OrgRealISOValue
        , mInInfo.rGMAAEInfo.bGammaEnable
        , mInInfo.rGMAAEInfo.i4AEStable
        , mInInfo.rGMAAEInfo.i4EVRatio
        );

    /*
        typedef struct {
            MUINT32 u4AETarget;
            MUINT32 u4AECurrentTarget;
            MUINT32 u4Eposuretime;   //!<: Exposure time in ms
            MUINT32 u4AfeGain;           //!<: raw gain
            MUINT32 u4IspGain;           //!<: sensor gain
            MUINT32 u4RealISOValue;
            MINT32  i4LightValue_x10;
            MUINT32 u4AECondition;
            MINT16  i2FlareOffset;
            MINT32  i4GammaIdx;   // next gamma idx
            MINT32  i4LESE_Ratio;    // LE/SE ratio
            MUINT32 u4SWHDR_SE;      //for sw HDR SE ,  -x EV , compare with converge AE
            MUINT32 u4MaxISO;
            MUINT32 u4AEStableCnt;
            MUINT32 u4OrgExposuretime;   //!<: Exposure time in ms
            MUINT32 u4OrgRealISOValue;
            MUINT16 u2Histogrm[GMA_AE_HISTOGRAM_BIN];
            MBOOL bGammaEnable;

            MINT32 i4AEStable;
            MINT32 i4EVRatio;
        } GMA_AE_INFO_T;
    */

    const MUINT32* pHist = &mInInfo.rGMAAEInfo.u4Histogrm[0];
    for (int i=0; i<120; i+=10)
    {
        CAM_LOGD( "%s: mInInfo.rGMAAEInfo.u4Histogrm[%d-%d]=(%x,%x,%x,%x,%x,  %x,%x,%x,%x,%x)"
            , __FUNCTION__
            , i
            , i+9
            , pHist[i], pHist[i+1], pHist[i+2], pHist[i+3], pHist[i+4]
            , pHist[i+5], pHist[i+6], pHist[i+7], pHist[i+8], pHist[i+9]);
    }
    //(i == 120)
    {
        int i=120;
        CAM_LOGD( "%s: mInInfo.rGMAAEInfo.u4Histogrm[%d-%d]=(%x,%x,%x,%x,%x,  %x,%x,%x)"
            , __FUNCTION__
            , i
            , i+7
            , pHist[i], pHist[i+1], pHist[i+2], pHist[i+3], pHist[i+4]
            , pHist[i+5], pHist[i+6], pHist[i+7]);
    }



    for (int i=0; i<140; i+=10)
    {
        CAM_LOGD( "%s: mInInfo.i4CurrEncGMA[%d-%d]=(%x,%x,%x,%x,%x,  %x,%x,%x,%x,%x)"
            , __FUNCTION__
            , i
            , i+9
            , mInInfo.i4CurrEncGMA[i], mInInfo.i4CurrEncGMA[i+1], mInInfo.i4CurrEncGMA[i+2], mInInfo.i4CurrEncGMA[i+3], mInInfo.i4CurrEncGMA[i+4]
            , mInInfo.i4CurrEncGMA[i+5], mInInfo.i4CurrEncGMA[i+6], mInInfo.i4CurrEncGMA[i+7], mInInfo.i4CurrEncGMA[i+8], mInInfo.i4CurrEncGMA[i+9]);
    }
    //(i == 140)
    {
        CAM_LOGD( "%s: mInInfo.i4CurrEncGMA[%d-%d]=(%x,%x,%x,%x)"
            , __FUNCTION__
            , 140
            , 140+3
            , mInInfo.i4CurrEncGMA[140], mInInfo.i4CurrEncGMA[140+1], mInInfo.i4CurrEncGMA[140+2], mInInfo.i4CurrEncGMA[140+3]);

    }

}

MVOID GmaMgr::setAEInfo(AE_INFO_T const & rAEInfo)
{
    mAEInfo = rAEInfo;

    if (!mAlgoEn) return;

    mInInfo.rGMAAEInfo.u4AETarget         = mAEInfo.u4AETarget;
    mInInfo.rGMAAEInfo.u4AECurrentTarget  = mAEInfo.u4AECurrentTarget;
    mInInfo.rGMAAEInfo.u4Eposuretime      = mAEInfo.u4Eposuretime;
    mInInfo.rGMAAEInfo.u4AfeGain          = mAEInfo.u4AfeGain;
    mInInfo.rGMAAEInfo.u4IspGain          = mAEInfo.u4IspGain;
    mInInfo.rGMAAEInfo.u4RealISOValue     = mAEInfo.u4RealISOValue;
    mInInfo.rGMAAEInfo.i4LightValue_x10   = mAEInfo.i4LightValue_x10;
    mInInfo.rGMAAEInfo.u4AECondition      = mAEInfo.u4AECondition;
    mInInfo.rGMAAEInfo.i2FlareOffset      = mAEInfo.i2FlareOffset;
    mInInfo.rGMAAEInfo.i4GammaIdx         = mAEInfo.i4GammaIdx;
    mInInfo.rGMAAEInfo.i4LESE_Ratio       = mAEInfo.i4LESE_Ratio;
    mInInfo.rGMAAEInfo.u4SWHDR_SE         = mAEInfo.u4SWHDR_SE;
    mInInfo.rGMAAEInfo.u4MaxISO           = mAEInfo.u4MaxISO;
    mInInfo.rGMAAEInfo.u4AEStableCnt      = mAEInfo.u4AEStableCnt;
    mInInfo.rGMAAEInfo.u4OrgExposuretime  = mAEInfo.u4OrgExposuretime;
    mInInfo.rGMAAEInfo.u4OrgRealISOValue  = mAEInfo.u4OrgRealISOValue;
    mInInfo.rGMAAEInfo.bGammaEnable       = mAEInfo.bGammaEnable;
    mInInfo.rGMAAEInfo.i4AEStable         = mAEInfo.bAEStable;
    mInInfo.rGMAAEInfo.i4EVRatio          = mAEInfo.u4EVRatio;
	mInInfo.rGMAAEInfo.i4DeltaIndex       = mAEInfo.i4deltaIndex;
	mInInfo.rGMAAEInfo.u4FDProb      	  = mAEInfo.u4FDProb;
	mInInfo.rGMAAEInfo.u4FaceNum       	  = mAEInfo.u4FaceNum;

    for (int i = 0; i<GMA_AE_HISTOGRAM_BIN; i++)
    {
        mInInfo.rGMAAEInfo.u4Histogrm[i]  = mAEInfo.u4Histogrm[i];
    }

    if (mLogEn){
        printAEInfo(rAEInfo, "setAESrc");
        printAEInfo(mAEInfo, "setAEDst");
    }

}

}
