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

#include "property_utils.h"
#include <aaa_types.h>
#include <mtkcam/utils/std/Log.h>
#include <camera_custom_nvram.h>
#include <isp_tuning.h>
#include <isp_tuning_cam_info.h>
#include <isp_tuning_custom.h>
#include <isp_mgr.h>
#include <isp_mgr_helper.h>
#include "lce_mgr.h"
#include <isp_tuning_mgr.h>
#include "paramctrl_if.h"
#include "paramctrl.h"

using namespace NSIspTuning;
namespace NSIspTuning
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
template <ESensorDev_T const eSensorDev>
class LceMgrDev : public LceMgr
{
public:
    static
    LceMgr*
    getInstance(ISP_NVRAM_LCE_TUNING_PARAM_T* pIspNvramLcePara)
    {
        static LceMgrDev<eSensorDev> singleton(pIspNvramLcePara);
        return &singleton;
    }
    virtual MVOID destroyInstance() {}

    LceMgrDev(ISP_NVRAM_LCE_TUNING_PARAM_T* pIspNvramLcePara)
        : LceMgr(eSensorDev, pIspNvramLcePara)
    {}

    virtual ~LceMgrDev() {}

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define INSTANTIATE(_dev_id) \
    case _dev_id: return  LceMgrDev<_dev_id>::getInstance(pIspNvramLcePara)

LceMgr*
LceMgr::
createInstance(ESensorDev_T const eSensorDev, ISP_NVRAM_LCE_TUNING_PARAM_T* pIspNvramLcePara)
{

    switch  (eSensorDev)
    {
    INSTANTIATE(ESensorDev_Main);       //  Main Sensor
    INSTANTIATE(ESensorDev_MainSecond); //  Main Second Sensor
    INSTANTIATE(ESensorDev_MainThird);  //  Main Third Sensor
    INSTANTIATE(ESensorDev_Sub);        //  Sub Sensor
    INSTANTIATE(ESensorDev_SubSecond);  //  Sub Second Sensor
    default:
        break;
    }

    return  MNULL;
}

LceMgr::LceMgr(ESensorDev_T const eSensorDev, ISP_NVRAM_LCE_TUNING_PARAM_T* pIspNvramLcePara)
    : m_eSensorDev  (eSensorDev)
    , m_pIspNvramLcePara (pIspNvramLcePara)
    , mLogEn (0)
    , m_PrvIdx(0)
    , mAlgoEn(MTRUE)
    , mLastLCE_GainInfo(0)
{
    if(LCE_FACE_HIST_SIZE != LCE_Y_BIN_NUM)
        CAM_LOGE("LCE_FACE_HIST_SIZE != LCE_Y_BIN_NUM, Define Error!");

    getPropInt("vendor.debug.lce_algo.disable", &mAlgoEn, 0);
    mAlgoEn = !mAlgoEn;

    if (mAlgoEn){
        m_pLceAlgo = MTKLce::createInstance(static_cast<eLCESensorDev_T>(m_eSensorDev));
    }
    else m_pLceAlgo = NULL;


    memset(&mEnv,        0, sizeof(MTK_LCE_ENV_INFO_STRUCT));
    memset(&mInInfo,     0, sizeof(MTK_LCE_PROC_INFO_STRUCT));
    memset(&mOutLCE,     0, sizeof(MTK_LCE_RESULT_INFO_STRUCT));
    memset(&mOutExif,    0, sizeof(MTK_LCE_EXIF_INFO_STRUCT));
    memset(&mLastLce2DceInfo,0, sizeof(LCE2DCE_INFO_T));

    if(m_pLceAlgo){
        mEnv.rAutoLCEParam = *((MTK_AUTO_LCE_TUNING_T*)&(m_pIspNvramLcePara[m_PrvIdx].rAutoLCEParam));
        m_pLceAlgo->LceReset();
        m_pLceAlgo->LceInit(&mEnv, NULL);
    }

    CAM_LOGD_IF(1, "[%s()] m_pLceAlgo(%p), m_eSensorDev(%d), m_pIspNvramLcePara(%p)", __FUNCTION__, m_pLceAlgo, m_eSensorDev, m_pIspNvramLcePara);

}


LceMgr::~LceMgr()
{
    if (m_pLceAlgo){
        m_pLceAlgo->LceExit();
        m_pLceAlgo->destroyInstance(m_pLceAlgo);
    }
    CAM_LOGD_IF(1, "[%s()] destroyInstance (%p)", __FUNCTION__, m_pLceAlgo);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
LceMgr::
start()
{

    getPropInt("vendor.debug.dynamic_lce.log", &mLogEn, 0);

    m_PrvIdx = 0;

    mEnv.rAutoLCEParam = *((MTK_AUTO_LCE_TUNING_T*)&(m_pIspNvramLcePara[m_PrvIdx].rAutoLCEParam));

    setAEInfo(mLast_AEInfo);

    if (m_pLceAlgo){
        m_pLceAlgo->LceFeatureCtrl(MTKLCE_FEATURE_SET_ENV_INFO, &mEnv, NULL);
        m_pLceAlgo->LceFeatureCtrl(MTKLCE_FEATURE_SET_PROC_INFO, &mInInfo, NULL);
    }

    CAM_LOGD_IF(1, "[%s()] LceMgr start (%p)", __FUNCTION__, m_pLceAlgo);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
LceMgr::
stop()
{
     CAM_LOGD_IF(1, "[%s()] LceMgr stop (%p)", __FUNCTION__, m_pLceAlgo);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
LceMgr::
calculateLCE(RAWIspCamInfo const& rRawIspCamInfo, MUINT16 Idx, MUINT16* pLcesBuffer,
                       ISP_NVRAM_LCE_T* pLCEReg, NSIspExifDebug::IspLceInfo* pLceExif,
                       MUINT8* pLceshoBuffer)
{
    if( Idx != m_PrvIdx){
        mEnv.rAutoLCEParam = *((MTK_AUTO_LCE_TUNING_T*)&(m_pIspNvramLcePara[Idx].rAutoLCEParam));
        m_PrvIdx = Idx;

        if (m_pLceAlgo){
           m_pLceAlgo->LceInit(&mEnv, NULL);
           m_pLceAlgo->LceFeatureCtrl(MTKLCE_FEATURE_SET_ENV_INFO, &mEnv, NULL);
       }
    }

    mInInfo.rLCELCSInfo.puLCSHistBuf = pLceshoBuffer;
    mInInfo.rLCELCSInfo.u4LightMap = pLcesBuffer;
    mInInfo.rLCELCSInfo.u4Width = rRawIspCamInfo.rLCS_Info.u4OutWidth;
    mInInfo.rLCELCSInfo.u4Height= rRawIspCamInfo.rLCS_Info.u4OutHeight;
    mInInfo.rLCELCSInfo.u4FrameWidth = ISP_LCS_OUT_WD;

    setAEInfo(rRawIspCamInfo.rAEInfo);

//Chooo
    //setPGNInfo(p1Lcs, p2Wb);
    //setCCMInfo(p1Lcs, p2CCM);


    if (mAlgoEn){
        if (m_pLceAlgo){
            m_pLceAlgo->LceFeatureCtrl(MTKLCE_FEATURE_SET_PROC_INFO, &mInInfo, NULL);
            m_pLceAlgo->LceMain();
            m_pLceAlgo->LceFeatureCtrl(MTKLCE_FEATURE_GET_RESULT, NULL, &mOutLCE);
            m_pLceAlgo->LceFeatureCtrl(MTKLCE_FEATURE_GET_EXIF, NULL, &mOutExif);
        }

        pLCEReg->tm_para0.bits.LCE_TC_P0       = mOutLCE.LCE_TC_P[0];
        pLCEReg->tm_para0.bits.LCE_TC_P1       = mOutLCE.LCE_TC_P[1];
        pLCEReg->tm_para1.bits.LCE_TC_P50      = mOutLCE.LCE_TC_P[2];
        pLCEReg->tm_para1.bits.LCE_TC_P250     = mOutLCE.LCE_TC_P[3];
        pLCEReg->tm_para2.bits.LCE_TC_P500     = mOutLCE.LCE_TC_P[4];
        pLCEReg->tm_para2.bits.LCE_TC_P750     = mOutLCE.LCE_TC_P[5];
        pLCEReg->tm_para3.bits.LCE_TC_P950     = mOutLCE.LCE_TC_P[6];
        pLCEReg->tm_para3.bits.LCE_TC_P999     = mOutLCE.LCE_TC_P[7];
        pLCEReg->tm_para4.bits.LCE_TC_O0       = mOutLCE.LCE_TC_O[0];
        pLCEReg->tm_para4.bits.LCE_TC_O1       = mOutLCE.LCE_TC_O[1];
        pLCEReg->tm_para5.bits.LCE_TC_O50      = mOutLCE.LCE_TC_O[2];
        pLCEReg->tm_para5.bits.LCE_TC_O250     = mOutLCE.LCE_TC_O[3];
        pLCEReg->tm_para6.bits.LCE_TC_O500     = mOutLCE.LCE_TC_O[4];
        pLCEReg->tm_para6.bits.LCE_TC_O750     = mOutLCE.LCE_TC_O[5];
        pLCEReg->tm_para7.bits.LCE_TC_O950     = mOutLCE.LCE_TC_O[6];
        pLCEReg->tm_para7.bits.LCE_TC_O999     = mOutLCE.LCE_TC_O[7];

        pLCEReg->cen_para0.bits.LCE_CEN_SLOPE_DEP_EN = mOutLCE.LCE_CEN_SLOPE_DEP_EN;
        pLCEReg->cen_para0.bits.LCE_CORING_TH        = mOutLCE.LCE_CORING_TH;
        pLCEReg->cen_para1.bits.LCE_CEN_MAX_SLOPE    = mOutLCE.LCE_CEN_MAX_SLOPE;
        pLCEReg->cen_para1.bits.LCE_CEN_MIN_SLOPE    = mOutLCE.LCE_CEN_MIN_SLOPE;
        pLCEReg->cen_para2.bits.LCE_CEN_MAX_SLOPE_N  = mOutLCE.LCE_MAX_SLOPE_N;
        pLCEReg->cen_para2.bits.LCE_CEN_MIN_SLOPE_N  = mOutLCE.LCE_MIN_SLOPE_N;

        pLCEReg->tchl_para0.bits.LCE_TCHL_EN       = mOutLCE.LCE_TCHL_EN;
        pLCEReg->tchl_para0.bits.LCE_TCHL_MTH      = mOutLCE.LCE_TCHL_MTH;
        pLCEReg->tchl_para0.bits.LCE_TCHL_MGAIN    = mOutLCE.LCE_TCHL_MGAIN;
        pLCEReg->tchl_para0.bits.LCE_TCHL_DTH1     = mOutLCE.LCE_TCHL_DTH1;
        pLCEReg->tchl_para1.bits.LCE_TCHL_DTH2     = mOutLCE.LCE_TCHL_DTH2;
        pLCEReg->tchl_para1.bits.LCE_TCHL_BW1      = mOutLCE.LCE_TCHL_BW1;
        pLCEReg->tchl_para1.bits.LCE_TCHL_BW2      = mOutLCE.LCE_TCHL_BW2;

        pLCEReg->hlr_para0.bits.LCE_HLR_TC_P0    = mOutLCE.GCE_TC_P[0];
        pLCEReg->hlr_para0.bits.LCE_HLR_TC_P1    = mOutLCE.GCE_TC_P[1];
        pLCEReg->hlr_para1.bits.LCE_HLR_TC_P50   = mOutLCE.GCE_TC_P[2];
        pLCEReg->hlr_para1.bits.LCE_HLR_TC_P250  = mOutLCE.GCE_TC_P[3];
        pLCEReg->hlr_para2.bits.LCE_HLR_TC_P500  = mOutLCE.GCE_TC_P[4];
        pLCEReg->hlr_para2.bits.LCE_HLR_TC_P750  = mOutLCE.GCE_TC_P[5];
        pLCEReg->hlr_para3.bits.LCE_HLR_TC_P950  = mOutLCE.GCE_TC_P[6];
        pLCEReg->hlr_para3.bits.LCE_HLR_TC_P999  = mOutLCE.GCE_TC_P[7];
        pLCEReg->hlr_para4.bits.LCE_HLR_TC_O0    = mOutLCE.GCE_TC_O[0];
        pLCEReg->hlr_para4.bits.LCE_HLR_TC_O1    = mOutLCE.GCE_TC_O[1];
        pLCEReg->hlr_para5.bits.LCE_HLR_TC_O50   = mOutLCE.GCE_TC_O[2];
        pLCEReg->hlr_para5.bits.LCE_HLR_TC_O250  = mOutLCE.GCE_TC_O[3];
        pLCEReg->hlr_para6.bits.LCE_HLR_TC_O500  = mOutLCE.GCE_TC_O[4];
        pLCEReg->hlr_para6.bits.LCE_HLR_TC_O750  = mOutLCE.GCE_TC_O[5];
        pLCEReg->hlr_para7.bits.LCE_HLR_TC_O950  = mOutLCE.GCE_TC_O[6];
        pLCEReg->hlr_para7.bits.LCE_HLR_TC_O999  = mOutLCE.GCE_TC_O[7];

        mLastLCE_GainInfo = ((mOutLCE.LCE_MAX_GAIN_RATIO << 16) | (mOutLCE.LCE_CURRENT_MAX_GAIN & 0xFFFF));

        ::memcpy(pLceExif, &mOutExif, sizeof(NSIspExifDebug::IspLceInfo));

        mLastLce2DceInfo.DRINDEX_L = mOutLCE.LCE_DRINDEX_L;
        mLastLce2DceInfo.DRINDEX_H = mOutLCE.LCE_DRINDEX_H;
        ::memcpy(&(mLastLce2DceInfo.FACE_HIST), &(mOutLCE.face_hist), (sizeof(MINT32)*LCE_Y_BIN_NUM));
        mLastLce2DceInfo.FACEBOUND_L = mOutExif.i4FaceLoBound;
        mLastLce2DceInfo.FACEBOUND_H = mOutExif.i4FaceHiBound;

    }

    if (mLogEn)
    {
        CAM_LOGD_IF(1, "[%s()] m_eSensorDev(%d) after LceMain()", __FUNCTION__, m_eSensorDev);
        printInfo();
    }
}


MVOID LceMgr::setAEInfo(AE_ISP_INFO_T const & rAEInfo)
{
    if (!mAlgoEn) return;


//Chooo
    mInInfo.rLCEAEInfo.i4LightValue_x10      = rAEInfo.i4LightValue_x10;
    mInInfo.rLCEAEInfo.bAEStable             = rAEInfo.bAEStable;
    mInInfo.rLCEAEInfo.bAETouchEnable        = rAEInfo.bAETouchEnable;
    mInInfo.rLCEAEInfo.bIsPlineMaxIndex      = rAEInfo.bIsPlineMaxIndex;
    mInInfo.rLCEAEInfo.i4DeltaIdx            = rAEInfo.i4deltaIndex;
    mInInfo.rLCEAEInfo.u4AEFinerEVIdxBase    = rAEInfo.u4AEFinerEVIdxBase;

    mInInfo.rLCEAEInfo.u4FaceAEStable        = rAEInfo.u4FaceAEStable;
    mInInfo.rLCEAEInfo.u4MeterFDTarget       = rAEInfo.u4MeterFDTarget;
    mInInfo.rLCEAEInfo.u4MeterFDLinkTarget   = rAEInfo.u4MeterFDLinkTarget;
    mInInfo.rLCEAEInfo.u4FaceNum             = rAEInfo.u4FaceNum;
    mInInfo.rLCEAEInfo.i4FDY_ArrayOri        = rAEInfo.i4FDY_ArrayOri[0];
    mInInfo.rLCEAEInfo.i4FDY_ArraySort       = rAEInfo.i4FDY_Array[0];
    mInInfo.rLCEAEInfo.i4Crnt_FDY            = rAEInfo.i4Crnt_FDY;
    mInInfo.rLCEAEInfo.i4FDXLow              = rAEInfo.FDArea[0].i4Left;
    mInInfo.rLCEAEInfo.i4FDXHi               = rAEInfo.FDArea[0].i4Right;
    mInInfo.rLCEAEInfo.i4FDYLow              = rAEInfo.FDArea[0].i4Top;
    mInInfo.rLCEAEInfo.i4FDYHi               = rAEInfo.FDArea[0].i4Bottom;
    mInInfo.rLCEAEInfo.i4LMXLow              = rAEInfo.LandMarkFDSmoothArea.i4Left;
    mInInfo.rLCEAEInfo.i4LMXHi               = rAEInfo.LandMarkFDSmoothArea.i4Right;
    mInInfo.rLCEAEInfo.i4LMYLow              = rAEInfo.LandMarkFDSmoothArea.i4Top;
    mInInfo.rLCEAEInfo.i4LMYHi               = rAEInfo.LandMarkFDSmoothArea.i4Bottom;
    mInInfo.rLCEAEInfo.bFaceAELCELinkEnable  = rAEInfo.bFaceAELCELinkEnable;
    mInInfo.rLCEAEInfo.u4MaxGain             = rAEInfo.u4MaxGain;

    mInInfo.rLCEAEInfo.bEnableFaceAE         = rAEInfo.bEnableFaceAE;
    mInInfo.rLCEAEInfo.u4FaceState           = rAEInfo.uFaceState;
    mInInfo.rLCEAEInfo.u4FaceRobustCnt       = rAEInfo.u4FaceRobustCnt;
    mInInfo.rLCEAEInfo.u4FaceRobustTrustCnt  = rAEInfo.u4FaceRobustTrustCnt;
    mInInfo.rLCEAEInfo.u4FD_Lock_MaxCnt      = rAEInfo.u4FD_Lock_MaxCnt;
    mInInfo.rLCEAEInfo.u4FDDropTempSmooth    = rAEInfo.u4FDDropTempSmoothCnt;
    mInInfo.rLCEAEInfo.u4OTFaceCnt           = rAEInfo.u4OTFaceTrustCnt;
    mInInfo.rLCEAEInfo.bOTFaceTimeOutLockAE  = rAEInfo.bOTFaceTimeOutLockAE;

    mInInfo.rLCEAEInfo.i4AEMagicNum          = rAEInfo.u4MagicNumber;
    mInInfo.rLCEAEInfo.i4AERequestNum        = rAEInfo.u4RequestNum;

    if (mLogEn){
        printAEInfo(rAEInfo, "setAESrc");
        printAEInfo(mLast_AEInfo, "setAEDst");
    }

    mLast_AEInfo = rAEInfo;

}

MVOID LceMgr::printInfo() const
{
    CAM_LOGD("%s: mOutExif (Part1): %d/%d/%d/%d/%d; %d/%d/%d/%d/%d/%d/%d/%d/; %d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d; %d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d;"
        , __FUNCTION__

        , mOutExif.i4ChipVersion
        , mOutExif.i4MainVersion
        , mOutExif.i4SubVersion
        , mOutExif.i4SystemVersion
        , mOutExif.i4PatchVersion

        , mOutExif.i4LV
        , mOutExif.i4LVIdx_L
        , mOutExif.i4LVIdx_H
        , mOutExif.i4CurrDR
        , mOutExif.i4DRIdx_L
        , mOutExif.i4DRIdx_H
        , mOutExif.i4DiffRangeIdx_L
        , mOutExif.i4DiffRangeIdx_H

        , mOutExif.i4LumaTarget
        , mOutExif.i4FlatTarget
        , mOutExif.i4LumaProb
        , mOutExif.i4FlatProb
        , mOutExif.i4LumaFlatProb
        , mOutExif.i4LumaFlatTarget
        , mOutExif.i4LVTarget
        , mOutExif.i4BrightAvg
        , mOutExif.i4LVProb
        , mOutExif.i4DStrengthLevel
        , mOutExif.i4BStrengthLevel
        , mOutExif.i4FinalTarget
        , mOutExif.i4FinalDStrength
        , mOutExif.i4FinalBStrength

        , mOutExif.i4P0
        , mOutExif.i4P1
        , mOutExif.i4P50
        , mOutExif.i4P250
        , mOutExif.i4P500
        , mOutExif.i4P750
        , mOutExif.i4P950
        , mOutExif.i4P999
        , mOutExif.i4O0
        , mOutExif.i4O1
        , mOutExif.i4O50
        , mOutExif.i4O250
        , mOutExif.i4O500
        , mOutExif.i4O750
        , mOutExif.i4O950
        , mOutExif.i4O999
        , mOutExif.i4CenMaxSlope
        , mOutExif.i4CenMinSlope
    );

    CAM_LOGD("%s: mOutExif (Part2): %d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d; %d/%d/%d/%d/%d/%d/%d/%d/%d; %d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d; %d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d; %d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d;  %d/%d/%d/%d/%d/"
        , __FUNCTION__

        , mOutExif.i4LCE_FD_Enable
        , mOutExif.i4FaceNum
        , mOutExif.i4FDXLow
        , mOutExif.i4FDXHi
        , mOutExif.i4FDYLow
        , mOutExif.i4FDYHi
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
        , mOutExif.i4MeterFDLinkTarget
        , mOutExif.i4AEGain
        , mOutExif.i4FaceAELCELinkEnable
        , mOutExif.i4MaxAEGain
        , mOutExif.bIsFrontLight
        , mOutExif.bAETouchEnable
        , mOutExif.bFaceAEEnable
        , mOutExif.i4OriP0
        , mOutExif.i4OriP1
        , mOutExif.i4OriP50
        , mOutExif.i4OriP250
        , mOutExif.i4OriP500
        , mOutExif.i4OriP750
        , mOutExif.i4OriP950
        , mOutExif.i4OriP999
        , mOutExif.i4OriO0
        , mOutExif.i4OriO1
        , mOutExif.i4OriO50
        , mOutExif.i4OriO250
        , mOutExif.i4OriO500
        , mOutExif.i4OriO750
        , mOutExif.i4OriO950
        , mOutExif.i4OriO999
        , mOutExif.i4FaceP0
        , mOutExif.i4FaceP1
        , mOutExif.i4FaceP50
        , mOutExif.i4FaceP250
        , mOutExif.i4FaceP500
        , mOutExif.i4FaceP750
        , mOutExif.i4FaceP950
        , mOutExif.i4FaceP999
        , mOutExif.i4FaceO0
        , mOutExif.i4FaceO1
        , mOutExif.i4FaceO50
        , mOutExif.i4FaceO250
        , mOutExif.i4FaceO500
        , mOutExif.i4FaceO750
        , mOutExif.i4FaceO950
        , mOutExif.i4FaceO999
        , mOutExif.bFaceSmoothLinkEnable
        , mOutExif.u4FaceState
        , mOutExif.u4FaceRobustCnt
        , mOutExif.u4FaceRobustTrustCnt
        , mOutExif.u4NonRobustFaceCnt
        , mOutExif.bFaceLCEStable
        , mOutExif.u4FaceStableCnt
        , mOutExif.u4FaceInStableThd
        , mOutExif.u4FaceOutStableThd
        , mOutExif.u4NeedFaceProtection
        , mOutExif.u4NeedFaceGain
        , mOutExif.u4FaceProtectMode
        , mOutExif.i4SmoothEnable
        , mOutExif.i4LCEPosSpeed
        , mOutExif.i4LCENegSpeed
        , mOutExif.i4LCECrosSpeed
        , mOutExif.i4LCESpeed4AEStable
    );

    CAM_LOGD("%s: mOutExif (Part3): %d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d; %d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/; %d"
        , __FUNCTION__

        , mOutExif.i4DStrengthRatio
        , mOutExif.i4BStrengthRatio
        , mOutExif.i4CurrMaxLceGain
        , mOutExif.i4MaxFinalTarget
        , mOutExif.i4LumaMean
        , mOutExif.i4FlatMean
        , mOutExif.i4DSTDIndex
        , mOutExif.bDarkSkyProtectEnable
        , mOutExif.i4DarkFlatY
        , mOutExif.i4DarkFlatRatio
        , mOutExif.i4FlatLVRatio
        , mOutExif.bBypassLCE
        , mOutExif.bIncorrectLCSO

        , mOutExif.i4GCEP0
        , mOutExif.i4GCEP1
        , mOutExif.i4GCEP50
        , mOutExif.i4GCEP250
        , mOutExif.i4GCEP500
        , mOutExif.i4GCEP750
        , mOutExif.i4GCEP950
        , mOutExif.i4GCEP999
        , mOutExif.i4GCEO0
        , mOutExif.i4GCEO1
        , mOutExif.i4GCEO50
        , mOutExif.i4GCEO250
        , mOutExif.i4GCEO500
        , mOutExif.i4GCEO750
        , mOutExif.i4GCEO950
        , mOutExif.i4GCEO999
        , mOutExif.i4AEMagicNum
    );



}

MVOID LceMgr::setFLCInfo(ISP_NVRAM_FLC_T const & p1_FLC, ISP_NVRAM_FLC_T const & p2_FLC)
{
    if (!mAlgoEn) return;

    mInInfo.rLCELCSInfo.rP1FLCInfo.FLC_OFST_B = p1_FLC.ofst_rb.bits.FLC_OFST_B;
    mInInfo.rLCELCSInfo.rP1FLCInfo.FLC_OFST_R = p1_FLC.ofst_rb.bits.FLC_OFST_R;
    mInInfo.rLCELCSInfo.rP1FLCInfo.FLC_OFST_G = p1_FLC.ofst_g.bits.FLC_OFST_G;
    mInInfo.rLCELCSInfo.rP1FLCInfo.FLC_GAIN_B = p1_FLC.gn_rb.bits.FLC_GAIN_B;
    mInInfo.rLCELCSInfo.rP1FLCInfo.FLC_GAIN_R = p1_FLC.gn_rb.bits.FLC_GAIN_R;
    mInInfo.rLCELCSInfo.rP1FLCInfo.FLC_GAIN_G = p1_FLC.gn_g.bits.FLC_GAIN_G;

    mInInfo.rLCELCSInfo.rP2FLCInfo.FLC_OFST_B = p2_FLC.ofst_rb.bits.FLC_OFST_B;
    mInInfo.rLCELCSInfo.rP2FLCInfo.FLC_OFST_R = p2_FLC.ofst_rb.bits.FLC_OFST_R;
    mInInfo.rLCELCSInfo.rP2FLCInfo.FLC_OFST_G = p2_FLC.ofst_g.bits.FLC_OFST_G;
    mInInfo.rLCELCSInfo.rP2FLCInfo.FLC_GAIN_B = p2_FLC.gn_rb.bits.FLC_GAIN_B;
    mInInfo.rLCELCSInfo.rP2FLCInfo.FLC_GAIN_R = p2_FLC.gn_rb.bits.FLC_GAIN_R;
    mInInfo.rLCELCSInfo.rP2FLCInfo.FLC_GAIN_G = p2_FLC.gn_g.bits.FLC_GAIN_G;
}

MVOID LceMgr::setCCMInfo(ISP_NVRAM_CCM_T const & p1_CCM, ISP_NVRAM_CCM_T const & p2_CCM)
{
    if (!mAlgoEn) return;

    mInInfo.rLCELCSInfo.rP1CCMInfo.ACC          = 9;  //Chooo
    mInInfo.rLCELCSInfo.rP1CCMInfo.CNV_00       = p1_CCM.cnv_1.bits.CCM_CNV_00;
    mInInfo.rLCELCSInfo.rP1CCMInfo.CNV_01       = p1_CCM.cnv_1.bits.CCM_CNV_01;
    mInInfo.rLCELCSInfo.rP1CCMInfo.CNV_02       = p1_CCM.cnv_2.bits.CCM_CNV_02;
    mInInfo.rLCELCSInfo.rP1CCMInfo.CNV_10       = p1_CCM.cnv_3.bits.CCM_CNV_10;
    mInInfo.rLCELCSInfo.rP1CCMInfo.CNV_11       = p1_CCM.cnv_3.bits.CCM_CNV_11;
    mInInfo.rLCELCSInfo.rP1CCMInfo.CNV_12       = p1_CCM.cnv_4.bits.CCM_CNV_12;
    mInInfo.rLCELCSInfo.rP1CCMInfo.CNV_20       = p1_CCM.cnv_5.bits.CCM_CNV_20;
    mInInfo.rLCELCSInfo.rP1CCMInfo.CNV_21       = p1_CCM.cnv_5.bits.CCM_CNV_21;
    mInInfo.rLCELCSInfo.rP1CCMInfo.CNV_22       = p1_CCM.cnv_6.bits.CCM_CNV_22;

    mInInfo.rLCELCSInfo.rP2CCMInfo.ACC          = 9;  //Chooo
    mInInfo.rLCELCSInfo.rP2CCMInfo.CNV_00       = p2_CCM.cnv_1.bits.CCM_CNV_00;
    mInInfo.rLCELCSInfo.rP2CCMInfo.CNV_01       = p2_CCM.cnv_1.bits.CCM_CNV_01;
    mInInfo.rLCELCSInfo.rP2CCMInfo.CNV_02       = p2_CCM.cnv_2.bits.CCM_CNV_02;
    mInInfo.rLCELCSInfo.rP2CCMInfo.CNV_10       = p2_CCM.cnv_3.bits.CCM_CNV_10;
    mInInfo.rLCELCSInfo.rP2CCMInfo.CNV_11       = p2_CCM.cnv_3.bits.CCM_CNV_11;
    mInInfo.rLCELCSInfo.rP2CCMInfo.CNV_12       = p2_CCM.cnv_4.bits.CCM_CNV_12;
    mInInfo.rLCELCSInfo.rP2CCMInfo.CNV_20       = p2_CCM.cnv_5.bits.CCM_CNV_20;
    mInInfo.rLCELCSInfo.rP2CCMInfo.CNV_21       = p2_CCM.cnv_5.bits.CCM_CNV_21;
    mInInfo.rLCELCSInfo.rP2CCMInfo.CNV_22       = p2_CCM.cnv_6.bits.CCM_CNV_22;
}


MVOID
LceMgr::
printAEInfo(AE_ISP_INFO_T const & rAEInfo, const char* username)
{

//Chooo
#if 0
        CAM_LOGD("%s: [%s] rAEInfo: %d/%d/%d/%d/%d; %d/%d/%d/%d/%d; %d/%d/%d/%d; %d/%d/%d;"
            , __FUNCTION__
            , username

            , rAEInfo.u4AETarget
            , rAEInfo.u4AECurrentTarget
            , rAEInfo.u8P1Exposuretime_ns
            , rAEInfo.u4P1SensorGain
            , rAEInfo.u4P1DGNGain

            , rAEInfo.u4P1RealISOValue
            , rAEInfo.i4LightValue_x10
            , rAEInfo.u4AECondition
            , rAEInfo.i2FlareOffset
            , rAEInfo.i4GammaIdx

            , rAEInfo.i4LESE_Ratio
            //, rAEInfo.u4SWHDR_SE
            , rAEInfo.u4MaxISO
            , rAEInfo.u4AEStableCnt
            , rAEInfo.u4AlgoExposuretime_us

            , rAEInfo.u4AlgoRealISOValue
            , rAEInfo.bGammaEnable
            , rAEInfo.u4EVRatio
            );


#endif

}

}
