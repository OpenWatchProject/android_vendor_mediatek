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
#define LOG_TAG "lce_mgr"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <mtkcam/utils/std/Log.h>
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
#include "lce_mgr.h"
#include <isp_tuning_mgr.h>
#include "paramctrl_if.h"
#include "paramctrl.h"
#include <private/aaa_hal_private.h>
#include <private/aaa_utils.h>
#include <array>

#define MY_INST NS3Av3::INST_T<NSIspTuningv3::LceMgr>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

using namespace NSIspTuning;
using namespace NSIspTuningv3;
namespace NSIspTuningv3
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
LceMgr*
LceMgr::
createInstance(ESensorDev_T const eSensorDev, ISP_NVRAM_LCE_TUNING_PARAM_T* pIspNvramLcePara)
{
    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(static_cast<MINT32>(eSensorDev));

    if(i4SensorIdx >= SENSOR_IDX_MAX || i4SensorIdx < 0) {
        CAM_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorIdx);
        return NULL;
    }

    MY_INST& rSingleton = gMultiton[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<NSIspTuningv3::LceMgr>(eSensorDev, pIspNvramLcePara);
    } );

    return rSingleton.instance.get();
}

LceMgr::LceMgr(ESensorDev_T const eSensorDev, ISP_NVRAM_LCE_TUNING_PARAM_T* pIspNvramLcePara)
    : m_eSensorDev  (eSensorDev)
    , m_pIspNvramLcePara (pIspNvramLcePara)
    //, m_pGmaAlgo (MTKGma::createInstance())
    , mLceScenario (0) //E_LCE_SCENARIO_PREVIEW
    , mLogEn (0)
    , m_ePrev_Tuning_Set(MTKLCE_TUNING_SET_Preview)
    , m_bHLR2En(MFALSE)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.lce_algo.enable", value, "1");
    mAlgoEn = atoi(value);

    property_get("vendor.debug.dynamic_lce.log", value, "0");
    mLogEn = atoi(value);

    memset(&mAEInfo, 0, sizeof(AE_INFO_T));
    memset(&mOutExif, 0, sizeof(MTK_LCE_EXIF_INFO_STRUCT));
    memset(&mOutLCE, 0, sizeof(MTK_LCE_RESULT_INFO_STRUCT));
    memset(&mInInfo, 0, sizeof(MTK_LCE_PROC_INFO_STRUCT));

    CAM_LOGD_IF(1, "[%s()] before MTKLce::createInstance", __FUNCTION__);

    if (mAlgoEn) m_pLceAlgo = MTKLce::createInstance(static_cast<eLCESensorDev_T>(m_eSensorDev));
    else m_pLceAlgo = NULL;

    CAM_LOGD_IF(mAlgoEn, "[%s()] m_pLceAlgo(%p), m_eSensorDev(%d), m_pIspNvramLcePara(%p)", __FUNCTION__, m_pLceAlgo, m_eSensorDev, m_pIspNvramLcePara);

    if (mAlgoEn) m_pLceAlgo->LceReset();

    CAM_LOGD_IF(1, "[%s()] after LceReset", __FUNCTION__);

    mEnv.i4AutoHDREnable = m_pIspNvramLcePara->i4AutoHDREnable;
    mEnv.rAutoLCEParam[0] = *((MTK_AUTO_LCE_TUNING_T*)&(m_pIspNvramLcePara->rAutoLCEParam[MTKLCE_TUNING_SET_Preview]));
    mEnv.rAutoLCEParam[1] = *((MTK_AUTO_LCE_TUNING_T*)&(m_pIspNvramLcePara->rAutoLCEParam[MTKLCE_TUNING_SET_Preview]));
    mEnv.eLCECamMode = MTKLCE_TUNING_SET_Preview;


    CAM_LOGD_IF(1, "[%s()] before LceInit", __FUNCTION__);

    if (mAlgoEn) m_pLceAlgo->LceInit(&mEnv, NULL);

    CAM_LOGD_IF(1, "[%s()] after LceInit", __FUNCTION__);

}


LceMgr::~LceMgr()
{
   CAM_LOGD_IF(1, "[%s()] before LceExit (%p)", __FUNCTION__, m_pLceAlgo);
   if (mAlgoEn) m_pLceAlgo->LceExit();
   CAM_LOGD_IF(1, "[%s()] after GmaExit", __FUNCTION__);
   if (mAlgoEn) m_pLceAlgo->destroyInstance(m_pLceAlgo);
   CAM_LOGD_IF(1, "[%s()] after destroyInstance (%p)", __FUNCTION__, m_pLceAlgo);
}

MVOID
LceMgr::
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
LceMgr::
stop()
{
    CAM_LOGD_IF(mLogEn, "[%s()] mLceScenario(%d)", __FUNCTION__, mLceScenario);
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
LceMgr::
start()
{

    m_ePrev_Tuning_Set = MTKLCE_TUNING_SET_Preview;

    mEnv.i4AutoHDREnable = m_pIspNvramLcePara->i4AutoHDREnable;
    mEnv.rAutoLCEParam[0] = *((MTK_AUTO_LCE_TUNING_T*)&(m_pIspNvramLcePara->rAutoLCEParam[MTKLCE_TUNING_SET_Preview]));
    mEnv.rAutoLCEParam[1] = *((MTK_AUTO_LCE_TUNING_T*)&(m_pIspNvramLcePara->rAutoLCEParam[MTKLCE_TUNING_SET_Preview]));
    mEnv.eLCECamMode = MTKLCE_TUNING_SET_Preview;

    if (mAlgoEn) m_pLceAlgo->LceFeatureCtrl(MTKLCE_FEATURE_SET_ENV_INFO, &mEnv, NULL);

    CAM_LOGD_IF(mLogEn, "[%s()] after MTKLCE_FEATURE_SET_PROC_INFO", __FUNCTION__);

}




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
LceMgr::
calculateLCE(ISP_LCS_OUT_INFO_T LCSO_info,
                   ISP_NVRAM_LCE_T* pLCEReg, NSIspExifDebug::IspExifDebugInfo_T::IspLceInfo* pLceExif, RAWIspCamInfo const& rRawIspCamInfo,
                   ISP_NVRAM_LCS_T const& p1Lcs, ISP_NVRAM_PGN_T const& p2Pgn, ISP_NVRAM_CCM_T const& p2CCM, MUINT16* pLcsBuffer)
{
    eMTKLCE_TUNING_SET_T LCE_Tuning_Set;

    switch (rRawIspCamInfo.eIspProfile)
    {
            case EIspProfile_Preview:
                LCE_Tuning_Set = MTKLCE_TUNING_SET_Preview;
                break;
            case EIspProfile_Video:
                LCE_Tuning_Set = MTKLCE_TUNING_SET_Video;
                break;
            case EIspProfile_Capture:
            case EIspProfile_Flash_Capture:
                LCE_Tuning_Set = MTKLCE_TUNING_SET_Capture;
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
                LCE_Tuning_Set = MTKLCE_TUNING_SET_HDR_Preview;
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
                LCE_Tuning_Set = MTKLCE_TUNING_SET_HDR_Video;
                break;
            case EIspProfile_zHDR_Capture:
            case EIspProfile_mHDR_Capture:
            case EIspProfile_zHDR_Flash_Capture:
                LCE_Tuning_Set = MTKLCE_TUNING_SET_HDR_Capture;
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
                LCE_Tuning_Set = MTKLCE_TUNING_SET_AutoHDR_Preview;
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
                LCE_Tuning_Set = MTKLCE_TUNING_SET_AutoHDR_Video;
                break;
            case EIspProfile_Auto_zHDR_Capture:
            case EIspProfile_Auto_mHDR_Capture:
                LCE_Tuning_Set = MTKLCE_TUNING_SET_AutoHDR_Capture;
                break;
            default:
                LCE_Tuning_Set = MTKLCE_TUNING_SET_Preview;
                break;
    }


    if( LCE_Tuning_Set != m_ePrev_Tuning_Set){

        m_ePrev_Tuning_Set = LCE_Tuning_Set;
        mEnv.rAutoLCEParam[0] = *((MTK_AUTO_LCE_TUNING_T*)&(m_pIspNvramLcePara->rAutoLCEParam[m_ePrev_Tuning_Set]));
        mEnv.rAutoLCEParam[1] = *((MTK_AUTO_LCE_TUNING_T*)&(m_pIspNvramLcePara->rAutoLCEParam[m_ePrev_Tuning_Set]));
        mEnv.eLCECamMode = m_ePrev_Tuning_Set;

       if (mAlgoEn){
           m_pLceAlgo->LceInit(&mEnv, NULL);
           m_pLceAlgo->LceFeatureCtrl(MTKLCE_FEATURE_SET_ENV_INFO, &mEnv, NULL);
       }
    }
    MUINT32 u4_IspProfile_temp = (MUINT32)rRawIspCamInfo.eIspProfile;
    mInInfo.eLceProfile = static_cast<eLCE_PROFILE_T>(u4_IspProfile_temp);

    mInInfo.rLCELCSInfo.u4LightMap = pLcsBuffer;
    mInInfo.rLCELCSInfo.u4Width = LCSO_info.u4OutWidth;
    mInInfo.rLCELCSInfo.u4Height= LCSO_info.u4OutHeight;
    mInInfo.rLCELCSInfo.u4FrameWidth = ISP_LCS_OUT_WD;

    setAEInfo(rRawIspCamInfo.rAEInfo);
    setPGNInfo(p1Lcs, p2Pgn);
    setCCMInfo(p1Lcs, p2CCM);

    if (mLogEn) printInfo();

    if (mAlgoEn) m_pLceAlgo->LceFeatureCtrl(MTKLCE_FEATURE_SET_PROC_INFO, &mInInfo, NULL);
    if (mAlgoEn) m_pLceAlgo->LceMain();
    if (mAlgoEn) m_pLceAlgo->LceFeatureCtrl(MTKLCE_FEATURE_GET_RESULT, NULL, &mOutLCE);
    if (mAlgoEn) m_pLceAlgo->LceFeatureCtrl(MTKLCE_FEATURE_GET_EXIF, NULL, &mOutExif);

    if (mLogEn)
    {
        CAM_LOGD_IF(1, "[%s()] m_eSensorDev(%d) after LceMain()", __FUNCTION__, m_eSensorDev);
        printInfo();
        printAEInfo(rRawIspCamInfo.rAEInfo, "mAEInfo_afterAlgo");
    }

    if (mAlgoEn)
    {
        pLCEReg->tm_para1.bits.LCE_TC_P1       = mOutLCE.LCE_A_TC_P1;
        pLCEReg->tm_para2.bits.LCE_TC_P50      = mOutLCE.LCE_A_TC_P50;
        pLCEReg->tm_para2.bits.LCE_TC_P500     = mOutLCE.LCE_A_TC_P500;
        pLCEReg->tm_para3.bits.LCE_TC_P950     = mOutLCE.LCE_A_TC_P950;
        pLCEReg->tm_para3.bits.LCE_TC_P999     = mOutLCE.LCE_A_TC_P999;
        pLCEReg->tm_para4.bits.LCE_TC_O1       = mOutLCE.LCE_A_TC_O1;
        pLCEReg->tm_para4.bits.LCE_TC_O50      = mOutLCE.LCE_A_TC_O50;
        pLCEReg->tm_para5.bits.LCE_TC_O500     = mOutLCE.LCE_A_TC_O500;
        pLCEReg->tm_para5.bits.LCE_TC_O950     = mOutLCE.LCE_A_TC_O950;
        pLCEReg->tm_para6.bits.LCE_TC_O999     = mOutLCE.LCE_A_TC_O999;
        pLCEReg->tm_para1.bits.LCE_CEN_MAX_SLP = mOutLCE.LCE_A_CEN_MAX_SLOPE;
        pLCEReg->tm_para1.bits.LCE_CEN_MIN_SLP = mOutLCE.LCE_A_CEN_MIN_SLOPE;
        pLCEReg->tm_para0.bits.LCE_GLOB_TONE = 0;
        pLCEReg->tm_para0.bits.LCE_CEN_END     = 320;
        pLCEReg->tm_para0.bits.LCE_CEN_BLD_WT  = 8;

        ::memcpy(pLceExif, &mOutExif, sizeof(NSIspExifDebug::IspExifDebugInfo_T::IspLceInfo));

        if (mLogEn)
        {
            CAM_LOGD("%s: LCE EXIF: %x/; %x/%x/%x/%x/%x; %x/%x/%x/%x/%x; %x/%x/%x/%x/%x; %x/%x/%x/%x/%x; %x/%x/%x/%x/%x; %x/%x/%x/%x/%x;\n"
                                        "%x/%x/%x/%x/%x; %x/%x/%x/%x/%x; %x/%x/%x/%x/%x; %x/%x/%x/%x/%x; %x/%x/%x/%x/%x; %x/%x/%x/%x/%x;\n"
                                        "%x/%x/%x/%x/%x; %x/%x/%x/%x/%x; %x/%x/%x/%x/%x; %x;"

                , mOutExif.i4LceProfile

                , mOutExif.i4LCECamMode
                , mOutExif.i4AutoHDREnable
                , mOutExif.i4ChipVersion
                , mOutExif.i4MainVersion
                , mOutExif.i4SubVersion

                , mOutExif.i4SystemVersion
                , mOutExif.i4LV
                , mOutExif.i4ContrastY10
                , mOutExif.i4EVRatio
                , mOutExif.i4EVContrastY10

                , mOutExif.i4SegDiv
                , mOutExif.i4ContrastIdx_L
                , mOutExif.i4ContrastIdx_H
                , mOutExif.i4LVIdx_L
                , mOutExif.i4LVIdx_H

                , mOutExif.i4DetailRatio1
                , mOutExif.i4DetailRatio50
                , mOutExif.i4DetailRatio500
                , mOutExif.i4DetailRatio950
                , mOutExif.i4DetailRatio999

                , mOutExif.i4IntpDiffRange128
                , mOutExif.i4DiffRangeIdx_L
                , mOutExif.i4DiffRangeIdx_H
                , mOutExif.i4P1
                , mOutExif.i4P50

                , mOutExif.i4P500
                , mOutExif.i4P950
                , mOutExif.i4P999
                , mOutExif.i4O1
                , mOutExif.i4O50

                , mOutExif.i4O500
                , mOutExif.i4O950
                , mOutExif.i4O999
                , mOutExif.i4CenMaxSlope
                , mOutExif.i4CenMinSlope

                , mOutExif.i4LCE_FD_Enable
                , mOutExif.i4FaceNum
                , mOutExif.i4LCSXLow
                , mOutExif.i4LCSXHi
                , mOutExif.i4LCSYLow

                , mOutExif.i4LCSYHi
                , mOutExif.i4FaceLoBound
                , mOutExif.i4FaceHiBound
                , mOutExif.i4ctrlPoint_f0
                , mOutExif.i4ctrlPoint_f1

                , mOutExif.i4FDY
                , mOutExif.i4MeterFDTarget
                , mOutExif.i4FDProb
                , mOutExif.i4AEGain
                , mOutExif.i4KeepBrightEnable

                , mOutExif.i4currBrightSlope
                , mOutExif.i4OriP1
                , mOutExif.i4OriP50
                , mOutExif.i4OriP500
                , mOutExif.i4OriP950

                , mOutExif.i4OriP999
                , mOutExif.i4OriO1
                , mOutExif.i4OriO50
                , mOutExif.i4OriO500
                , mOutExif.i4OriO950

                , mOutExif.i4OriO999
                , mOutExif.i4FaceP1
                , mOutExif.i4FaceP50
                , mOutExif.i4FaceP500
                , mOutExif.i4FaceP950

                , mOutExif.i4FaceP999
                , mOutExif.i4FaceO1
                , mOutExif.i4FaceO50
                , mOutExif.i4FaceO500
                , mOutExif.i4FaceO950

                , mOutExif.i4FaceO999
                , mOutExif.i4SmoothEnable
                , mOutExif.i4SmoothSpeed
                , mOutExif.i4SmoothWaitAE
                , mOutExif.i4FlareEnable

                , mOutExif.i4FlareOffset

                );




        }
    }
}

MVOID LceMgr::printInfo() const
{

    CAM_LOGD("%s: mInInfo(%d): %d/%d/%d/%d; %d/%d/%d/%d/%d; %d/%d/%d/%d/%d; %d/%d/%d/%d;"
        , __FUNCTION__

        , mInInfo.rLCEAEInfo.u4AETarget
        , mInInfo.rLCEAEInfo.u4AECurrentTarget
        , mInInfo.rLCEAEInfo.u4Eposuretime
        , mInInfo.rLCEAEInfo.u4AfeGain
        , mInInfo.rLCEAEInfo.u4IspGain

        , mInInfo.rLCEAEInfo.u4RealISOValue
        , mInInfo.rLCEAEInfo.i4LightValue_x10
        , mInInfo.rLCEAEInfo.u4AECondition
        , mInInfo.rLCEAEInfo.i2FlareOffset
        , mInInfo.rLCEAEInfo.i4GammaIdx

        , mInInfo.rLCEAEInfo.i4LESE_Ratio
        , mInInfo.rLCEAEInfo.u4SWHDR_SE
        , mInInfo.rLCEAEInfo.u4MaxISO
        , mInInfo.rLCEAEInfo.u4AEStableCnt
        , mInInfo.rLCEAEInfo.u4OrgExposuretime

        , mInInfo.rLCEAEInfo.u4OrgRealISOValue
        , mInInfo.rLCEAEInfo.bGammaEnable
        , mInInfo.rLCEAEInfo.i4AEStable
        , mInInfo.rLCEAEInfo.i4EVRatio

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

    const MUINT32* pHist = &mInInfo.rLCEAEInfo.u4Histogrm[0];
    for (int i=0; i<120; i+=10)
    {
        CAM_LOGD( "%s: mInInfo.rLCEAEInfo.u4Histogrm[%d-%d]=(%x,%x,%x,%x,%x,  %x,%x,%x,%x,%x)"
            , __FUNCTION__
            , i
            , i+9
            , pHist[i], pHist[i+1], pHist[i+2], pHist[i+3], pHist[i+4]
            , pHist[i+5], pHist[i+6], pHist[i+7], pHist[i+8], pHist[i+9]);
    }
    //(i == 120)
    {
        int i=120;
        CAM_LOGD( "%s: mInInfo.rLCEAEInfo.u4Histogrm[%d-%d]=(%x,%x,%x,%x,%x,  %x,%x,%x)"
            , __FUNCTION__
            , i
            , i+7
            , pHist[i], pHist[i+1], pHist[i+2], pHist[i+3], pHist[i+4]
            , pHist[i+5], pHist[i+6], pHist[i+7]);
    }


#if 0
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
#endif
}

MVOID LceMgr::setAEInfo(AE_INFO_T const & rAEInfo)
{
    mAEInfo = rAEInfo;

    if (!mAlgoEn) return;

    mInInfo.rLCEAEInfo.u4AETarget          = mAEInfo.u4AETarget;
    mInInfo.rLCEAEInfo.u4AECurrentTarget   = mAEInfo.u4AECurrentTarget;
    mInInfo.rLCEAEInfo.u4Eposuretime       = mAEInfo.u4Eposuretime;
    mInInfo.rLCEAEInfo.u4AfeGain           = mAEInfo.u4AfeGain;
    mInInfo.rLCEAEInfo.u4IspGain           = mAEInfo.u4IspGain;
    mInInfo.rLCEAEInfo.u4RealISOValue      = mAEInfo.u4RealISOValue;
    mInInfo.rLCEAEInfo.i4LightValue_x10    = mAEInfo.i4LightValue_x10;
    mInInfo.rLCEAEInfo.u4AECondition       = mAEInfo.u4AECondition;
    mInInfo.rLCEAEInfo.i2FlareOffset       = mAEInfo.i2FlareOffset;
    mInInfo.rLCEAEInfo.i4GammaIdx          = mAEInfo.i4GammaIdx;
    mInInfo.rLCEAEInfo.i4LESE_Ratio        = mAEInfo.i4LESE_Ratio;
    mInInfo.rLCEAEInfo.u4SWHDR_SE          = mAEInfo.u4SWHDR_SE;
    mInInfo.rLCEAEInfo.u4MaxISO            = mAEInfo.u4MaxISO;
    mInInfo.rLCEAEInfo.u4AEStableCnt       = mAEInfo.u4AEStableCnt;
    mInInfo.rLCEAEInfo.u4OrgExposuretime   = mAEInfo.u4OrgExposuretime;
    mInInfo.rLCEAEInfo.u4OrgRealISOValue   = mAEInfo.u4OrgRealISOValue;
    mInInfo.rLCEAEInfo.bGammaEnable        = mAEInfo.bGammaEnable;
    mInInfo.rLCEAEInfo.i4AEStable          = mAEInfo.bAEStable;
    mInInfo.rLCEAEInfo.i4EVRatio           = mAEInfo.u4EVRatio; //1024; //FIXME: 1024
    mInInfo.rLCEAEInfo.i4DeltaIdx		   = mAEInfo.i4deltaIndex;
	mInInfo.rLCEAEInfo.u4AEFinerEVIdxBase  = mAEInfo.u4AEFinerEVIdxBase;
	mInInfo.rLCEAEInfo.bAETouchEnable      = mAEInfo.bAETouchEnable;

    mInInfo.rLCEAEInfo.u4FaceAEStable      = mAEInfo.u4FaceAEStable;
    mInInfo.rLCEAEInfo.u4MeterFDTarget     = mAEInfo.u4MeterFDTarget;
    mInInfo.rLCEAEInfo.u4FDProb            = mAEInfo.u4FDProb;
    mInInfo.rLCEAEInfo.u4FaceNum           = mAEInfo.u4FaceNum;
    mInInfo.rLCEAEInfo.i4FDY_ArrayOri      = mAEInfo.i4FDY_ArrayOri[0];
    mInInfo.rLCEAEInfo.i4FDY_ArraySort     = mAEInfo.i4FDY_Array[0];
    mInInfo.rLCEAEInfo.i4Crnt_FDY          = mAEInfo.i4Crnt_FDY;
    mInInfo.rLCEAEInfo.i4FDXLow            = mAEInfo.FDArea[0].i4Left;
    mInInfo.rLCEAEInfo.i4FDXHi             = mAEInfo.FDArea[0].i4Right;
    mInInfo.rLCEAEInfo.i4FDYLow            = mAEInfo.FDArea[0].i4Top;
    mInInfo.rLCEAEInfo.i4FDYHi             = mAEInfo.FDArea[0].i4Bottom;
    mInInfo.rLCEAEInfo.i4LMXLow            = mAEInfo.LandMarkFDSmoothArea.i4Left;
    mInInfo.rLCEAEInfo.i4LMXHi             = mAEInfo.LandMarkFDSmoothArea.i4Right;
    mInInfo.rLCEAEInfo.i4LMYLow            = mAEInfo.LandMarkFDSmoothArea.i4Top;
    mInInfo.rLCEAEInfo.i4LMYHi             = mAEInfo.LandMarkFDSmoothArea.i4Bottom;
    for (int i = 0; i<LCE_Y_BIN_NUM; i++)
    {
        mInInfo.rLCEAEInfo.u4Histogrm[i]= mAEInfo.u4Histogrm[i];
    }

    if (mLogEn){
        printAEInfo(rAEInfo, "setAESrc");
        printAEInfo(mAEInfo, "setAEDst");

        CAM_LOGD("u4FaceAEStable:%d u4MeterFDTarget:%d u4FDProb:%d u4FaceNum:%d\n", mAEInfo.u4FaceAEStable, mAEInfo.u4MeterFDTarget, mAEInfo.u4FDProb, mAEInfo.u4FaceNum);
        CAM_LOGD("i4FDY_ArrayOri:%d i4FDY_ArraySort:%d i4Crnt_FDY:%d\n", mAEInfo.i4FDY_ArrayOri[0], mAEInfo.i4FDY_Array[0], mAEInfo.i4Crnt_FDY);
        CAM_LOGD("i4FDXLow:%d i4FDXHi:%d i4FDYLow:%d i4FDYHi:%d\n", mAEInfo.FDArea[0].i4Left, mAEInfo.FDArea[0].i4Right, mAEInfo.FDArea[0].i4Top, mAEInfo.FDArea[0].i4Bottom);
        CAM_LOGD("i4LMXLow:%d i4LMXHi:%d i4LMYLow:%d i4LMYHi:%d\n", mAEInfo.LandMarkFDSmoothArea.i4Left, mAEInfo.LandMarkFDSmoothArea.i4Right, mAEInfo.LandMarkFDSmoothArea.i4Top, mAEInfo.LandMarkFDSmoothArea.i4Bottom);
    }

}

MVOID LceMgr::setPGNInfo(ISP_NVRAM_LCS_T const& p1_LCS, ISP_NVRAM_PGN_T const & p2_PGN)
{
    if (!mAlgoEn) return;

    mInInfo.rLCELCSInfo.rP1AWBInfo.GAIN_R          = p1_LCS.gain_2.bits.LCS25_GAIN_R;
    mInInfo.rLCELCSInfo.rP1AWBInfo.GAIN_GR         = p1_LCS.gain_2.bits.LCS25_GAIN_GR;
    mInInfo.rLCELCSInfo.rP1AWBInfo.GAIN_B          = p1_LCS.gain_1.bits.LCS25_GAIN_B;
    mInInfo.rLCELCSInfo.rP1AWBInfo.GAIN_GB         = p1_LCS.gain_1.bits.LCS25_GAIN_GB;
    mInInfo.rLCELCSInfo.rP1AWBInfo.OFST_R          = p1_LCS.ofst_2.bits.LCS25_OFST_R;
    mInInfo.rLCELCSInfo.rP1AWBInfo.OFST_GR         = p1_LCS.ofst_2.bits.LCS25_OFST_GR;
    mInInfo.rLCELCSInfo.rP1AWBInfo.OFST_B          = p1_LCS.ofst_1.bits.LCS25_OFST_B;
    mInInfo.rLCELCSInfo.rP1AWBInfo.OFST_GB         = p1_LCS.ofst_1.bits.LCS25_OFST_GB;
    mInInfo.rLCELCSInfo.rP1AWBInfo.SATU_R          = p1_LCS.satu_2.bits.LCS25_SATU_R;
    mInInfo.rLCELCSInfo.rP1AWBInfo.SATU_GR         = p1_LCS.satu_2.bits.LCS25_SATU_GR;
    mInInfo.rLCELCSInfo.rP1AWBInfo.SATU_B          = p1_LCS.satu_1.bits.LCS25_SATU_B;
    mInInfo.rLCELCSInfo.rP1AWBInfo.SATU_GB         = p1_LCS.satu_1.bits.LCS25_SATU_GB;

    mInInfo.rLCELCSInfo.rP2AWBInfo.GAIN_R          = p2_PGN.gain_2.bits.PGN_GAIN_R;
    mInInfo.rLCELCSInfo.rP2AWBInfo.GAIN_GR         = p2_PGN.gain_2.bits.PGN_GAIN_GR;
    mInInfo.rLCELCSInfo.rP2AWBInfo.GAIN_B          = p2_PGN.gain_1.bits.PGN_GAIN_B;
    mInInfo.rLCELCSInfo.rP2AWBInfo.GAIN_GB         = p2_PGN.gain_1.bits.PGN_GAIN_GB;
    mInInfo.rLCELCSInfo.rP2AWBInfo.OFST_R          = p2_PGN.ofst_2.bits.PGN_OFST_R;
    mInInfo.rLCELCSInfo.rP2AWBInfo.OFST_GR         = p2_PGN.ofst_2.bits.PGN_OFST_GR;
    mInInfo.rLCELCSInfo.rP2AWBInfo.OFST_B          = p2_PGN.ofst_1.bits.PGN_OFST_B;
    mInInfo.rLCELCSInfo.rP2AWBInfo.OFST_GB         = p2_PGN.ofst_1.bits.PGN_OFST_GB;
    mInInfo.rLCELCSInfo.rP2AWBInfo.SATU_R          = p2_PGN.satu_2.bits.PGN_SATU_R;
    mInInfo.rLCELCSInfo.rP2AWBInfo.SATU_GR         = p2_PGN.satu_2.bits.PGN_SATU_GR;
    mInInfo.rLCELCSInfo.rP2AWBInfo.SATU_B          = p2_PGN.satu_1.bits.PGN_SATU_B;
    mInInfo.rLCELCSInfo.rP2AWBInfo.SATU_GB         = p2_PGN.satu_1.bits.PGN_SATU_GB;

}

MVOID LceMgr::setCCMInfo(ISP_NVRAM_LCS_T const& p1_LCS, ISP_NVRAM_CCM_T const & p2_CCM)
{
    if (!mAlgoEn) return;

    mInInfo.rLCELCSInfo.rP1CCMInfo.ACC          = p1_LCS.g2g_cnv_5.bits.LCS25_G2G_ACC;;
    mInInfo.rLCELCSInfo.rP1CCMInfo.CNV_00       = p1_LCS.g2g_cnv_1.bits.LCS25_G2G_CNV_00;
    mInInfo.rLCELCSInfo.rP1CCMInfo.CNV_01       = p1_LCS.g2g_cnv_1.bits.LCS25_G2G_CNV_01;
    mInInfo.rLCELCSInfo.rP1CCMInfo.CNV_02       = p1_LCS.g2g_cnv_2.bits.LCS25_G2G_CNV_02;
    mInInfo.rLCELCSInfo.rP1CCMInfo.CNV_10       = p1_LCS.g2g_cnv_2.bits.LCS25_G2G_CNV_10;
    mInInfo.rLCELCSInfo.rP1CCMInfo.CNV_11       = p1_LCS.g2g_cnv_3.bits.LCS25_G2G_CNV_11;
    mInInfo.rLCELCSInfo.rP1CCMInfo.CNV_12       = p1_LCS.g2g_cnv_3.bits.LCS25_G2G_CNV_12;
    mInInfo.rLCELCSInfo.rP1CCMInfo.CNV_20       = p1_LCS.g2g_cnv_4.bits.LCS25_G2G_CNV_20;
    mInInfo.rLCELCSInfo.rP1CCMInfo.CNV_21       = p1_LCS.g2g_cnv_4.bits.LCS25_G2G_CNV_21;
    mInInfo.rLCELCSInfo.rP1CCMInfo.CNV_22       = p1_LCS.g2g_cnv_5.bits.LCS25_G2G_CNV_22;

    mInInfo.rLCELCSInfo.rP2CCMInfo.ACC          = 9;  //fixme_Choo
    mInInfo.rLCELCSInfo.rP2CCMInfo.CNV_00       = p2_CCM.cnv_1.bits.G2G_CNV_00;
    mInInfo.rLCELCSInfo.rP2CCMInfo.CNV_01       = p2_CCM.cnv_1.bits.G2G_CNV_01;
    mInInfo.rLCELCSInfo.rP2CCMInfo.CNV_02       = p2_CCM.cnv_2.bits.G2G_CNV_02;
    mInInfo.rLCELCSInfo.rP2CCMInfo.CNV_10       = p2_CCM.cnv_3.bits.G2G_CNV_10;
    mInInfo.rLCELCSInfo.rP2CCMInfo.CNV_11       = p2_CCM.cnv_3.bits.G2G_CNV_11;
    mInInfo.rLCELCSInfo.rP2CCMInfo.CNV_12       = p2_CCM.cnv_4.bits.G2G_CNV_12;
    mInInfo.rLCELCSInfo.rP2CCMInfo.CNV_20       = p2_CCM.cnv_5.bits.G2G_CNV_20;
    mInInfo.rLCELCSInfo.rP2CCMInfo.CNV_21       = p2_CCM.cnv_5.bits.G2G_CNV_21;
    mInInfo.rLCELCSInfo.rP2CCMInfo.CNV_22       = p2_CCM.cnv_6.bits.G2G_CNV_22;

}


}
