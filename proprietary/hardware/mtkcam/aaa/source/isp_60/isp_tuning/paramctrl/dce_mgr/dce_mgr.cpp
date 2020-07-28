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
#define LOG_TAG "dce_mgr"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <mtkcam/utils/std/Log.h>
#include <camera_custom_nvram.h>
#include <isp_tuning.h>
#include <isp_tuning_cam_info.h>
#include <isp_tuning_custom.h>
#include <isp_mgr.h>
#include <isp_mgr_helper.h>
#include "dce_mgr.h"
#include <isp_tuning_mgr.h>
#include <private/aaa_hal_private.h>
#include <private/aaa_utils.h>
#include <array>

#define MY_INST NS3Av3::INST_T<DceMgr>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

using namespace NSIspTuning;
namespace NSIspTuning
{


DceMgr*
DceMgr::
createInstance(MUINT32 const eSensorDev, ISP_NVRAM_DCE_TUNING_PARAM_T* pIspNvramDcePara)
{
    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(static_cast<MINT32>(eSensorDev));

    if(i4SensorIdx >= SENSOR_IDX_MAX || i4SensorIdx < 0) {
        CAM_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorIdx);
        return NULL;
    }

    MY_INST& rSingleton = gMultiton[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
		rSingleton.instance = std::make_unique<DceMgr>(eSensorDev,pIspNvramDcePara);    } );  //need check jeff

    return rSingleton.instance.get();
}

DceMgr::DceMgr(MUINT32 const eSensorDev, ISP_NVRAM_DCE_TUNING_PARAM_T* pIspNvramDcePara)
    : m_eSensorDev  (eSensorDev)
    , m_pIspNvramDcePara (pIspNvramDcePara)
    , mLogEn (0)
    , m_PrvIdx(0)
    , mAlgoEn(MTRUE)
    , m_DCES_yv_bld(0)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.dce_algo.disable", value, "0");
    mAlgoEn = !(atoi(value));

    if (mAlgoEn){
        m_pDceAlgo = MTKDce::createInstance(static_cast<eDCESensorDev_T>(m_eSensorDev));
    }
    else m_pDceAlgo = NULL;

    memset(&mEnv,    0, sizeof(MTK_DCE_ENV_INFO_STRUCT));
    memset(&mInInfo,     0, sizeof(MTK_DCE_PROC_INFO_STRUCT));
    memset(&mOutDCE,     0, sizeof(MTK_DCE_RESULT_INFO_STRUCT));
    memset(&mOutExif,    0, sizeof(MTK_DCE_EXIF_INFO_STRUCT));
    memset(&mLast_AEInfo,     0, sizeof(AE_ISP_INFO_T));
    memset(&m_FLC_Reg,   0, sizeof(ISP_NVRAM_FLC_T));

    m_FLC_Reg.gn_rb.bits.FLC_GAIN_B = 512;
    m_FLC_Reg.gn_rb.bits.FLC_GAIN_R = 512;
    m_FLC_Reg.gn_g.bits.FLC_GAIN_G  = 512;

    if(m_pDceAlgo){
        mEnv.rAutoDCEParam = *((MTK_DCE_TUNING_T*)&(m_pIspNvramDcePara[m_PrvIdx].rDceParam));
        m_pDceAlgo->DceReset();
        m_pDceAlgo->DceInit(&mEnv, NULL);
    }

    CAM_LOGD_IF(1, "[%s()] m_pDceAlgo(%p), m_eSensorDev(%d), m_pIspNvramDcePara(%p)", __FUNCTION__, m_pDceAlgo, m_eSensorDev, m_pIspNvramDcePara);

}


DceMgr::~DceMgr()
{
   if (m_pDceAlgo){
     m_pDceAlgo->DceExit();
     m_pDceAlgo->destroyInstance(m_pDceAlgo);
   }
   CAM_LOGD_IF(1, "[%s()] destroyInstance (%p)", __FUNCTION__, m_pDceAlgo);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
DceMgr::
start()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.dynamic_dce.log", value, "0");
    mLogEn = atoi(value);

    m_PrvIdx = 0;
    m_DCES_yv_bld = 0;
    memset(&m_FLC_Reg,   0, sizeof(ISP_NVRAM_FLC_T));

    m_FLC_Reg.gn_rb.bits.FLC_GAIN_B = 512;
    m_FLC_Reg.gn_rb.bits.FLC_GAIN_R = 512;
    m_FLC_Reg.gn_g.bits.FLC_GAIN_G  = 512;

    mEnv.rAutoDCEParam = *((MTK_DCE_TUNING_T*)&(m_pIspNvramDcePara[m_PrvIdx].rDceParam));

    setAEInfo(mLast_AEInfo);

    mInInfo.dce_lce_info.dr_idx_l = 0;
    mInInfo.dce_lce_info.dr_idx_h = 0;

    mInInfo.dces_info.dces_en = 0;

    if (m_pDceAlgo){
        m_pDceAlgo->DceFeatureCtrl(MTKDCE_FEATURE_SET_ENV_INFO, &mEnv, NULL);
        m_pDceAlgo->DceFeatureCtrl(MTKDCE_FEATURE_SET_PROC_INFO, &mInInfo, NULL);
    }

    CAM_LOGD_IF(1, "[%s()] DceMgr start (%p)", __FUNCTION__, m_pDceAlgo);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
DceMgr::
stop()
{
    CAM_LOGD_IF(1, "[%s()] DceMgr stop (%p)", __FUNCTION__, m_pDceAlgo);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
DceMgr::
calculateDCE(RAWIspCamInfo const& rRawIspCamInfo, MUINT16 Idx, MUINT32* pDcesBuffer,
                   MINT32 i4DRINDEX_L, MINT32 i4DRINDEX_H,
                   ISP_NVRAM_DCE_T* pDCEReg, NSIspExifDebug::IspDceInfo* pDceExif)
{
    if( Idx != m_PrvIdx){
        mEnv.rAutoDCEParam = *((MTK_DCE_TUNING_T*)&(m_pIspNvramDcePara[Idx].rDceParam));

        m_PrvIdx = Idx;

        if (m_pDceAlgo){
            m_pDceAlgo->DceInit(&mEnv, NULL);
            m_pDceAlgo->DceFeatureCtrl(MTKDCE_FEATURE_SET_ENV_INFO, &mEnv, NULL);
        }
    }

    if(pDcesBuffer){
        mInInfo.dces_info.dces_en = MTRUE;
        mInInfo.dces_info.u4Dces = pDcesBuffer;
        mInInfo.dces_info.u4Width = rRawIspCamInfo.rCropRzInfo.sRRZout.w; //Morgan requests IMG Size
        mInInfo.dces_info.u4Height= rRawIspCamInfo.rCropRzInfo.sRRZout.h; //Morgan requests IMG Size
    }
    else{
        mInInfo.dces_info.dces_en = MFALSE;
        mInInfo.dces_info.u4Dces = NULL;
        if (mLogEn){
            CAM_LOGE("No DCES for MTKDCE");
        }
    }

    setAEInfo(rRawIspCamInfo.rAEInfo);

    mInInfo.dce_lce_info.dr_idx_l = i4DRINDEX_L;
    mInInfo.dce_lce_info.dr_idx_h = i4DRINDEX_H;

    if(m_pDceAlgo){
        m_pDceAlgo->DceFeatureCtrl(MTKDCE_FEATURE_SET_PROC_INFO, &mInInfo, NULL);
        m_pDceAlgo->DceMain();
        m_pDceAlgo->DceFeatureCtrl(MTKDCE_FEATURE_GET_RESULT, NULL, &mOutDCE);
        m_pDceAlgo->DceFeatureCtrl(MTKDCE_FEATURE_GET_EXIF,   NULL, &mOutExif);

        pDCEReg->tc_g1.bits.DCE_TC_P1  = mOutDCE.dce_p1;
        pDCEReg->tc_g1.bits.DCE_TC_P2  = mOutDCE.dce_p2;
        pDCEReg->tc_g2.bits.DCE_TC_P3  = mOutDCE.dce_p3;
        pDCEReg->tc_g2.bits.DCE_TC_P4  = mOutDCE.dce_p4;
        pDCEReg->tc_g3.bits.DCE_TC_P5  = mOutDCE.dce_p5;
        pDCEReg->tc_g3.bits.DCE_TC_P6  = mOutDCE.dce_p6;
        pDCEReg->tc_g4.bits.DCE_TC_P7  = mOutDCE.dce_p7;
        pDCEReg->tc_g4.bits.DCE_TC_P8  = mOutDCE.dce_p8;
        pDCEReg->tc_g5.bits.DCE_TC_P9  = mOutDCE.dce_p9;
        pDCEReg->tc_g5.bits.DCE_TC_P10 = mOutDCE.dce_p10;
        pDCEReg->tc_g6.bits.DCE_TC_P11 = mOutDCE.dce_p11;
        pDCEReg->tc_g6.bits.DCE_TC_P12 = mOutDCE.dce_p12;
        pDCEReg->tc_g7.bits.DCE_TC_P13 = mOutDCE.dce_p13;
        pDCEReg->tc_g7.bits.DCE_TC_P14 = mOutDCE.dce_p14;
        pDCEReg->tc_g8.bits.DCE_TC_P15 = mOutDCE.dce_p15;
        pDCEReg->tc_g8.bits.DCE_TC_P16 = mOutDCE.dce_p16;
        pDCEReg->tc_g9.bits.DCE_TC_P17 = mOutDCE.dce_p17;
        pDCEReg->tc_g9.bits.DCE_TC_P18 = mOutDCE.dce_p18;
        pDCEReg->tc_g10.bits.DCE_TC_P19 = mOutDCE.dce_p19;
        pDCEReg->tc_g10.bits.DCE_TC_P20 = mOutDCE.dce_p20;
        pDCEReg->tc_g11.bits.DCE_TC_P21 = mOutDCE.dce_p21;
        pDCEReg->tc_g11.bits.DCE_TC_P22 = mOutDCE.dce_p22;
        pDCEReg->tc_g12.bits.DCE_TC_P23 = mOutDCE.dce_p23;
        pDCEReg->tc_g12.bits.DCE_TC_P24 = mOutDCE.dce_p24;
        pDCEReg->tc_g13.bits.DCE_TC_P25 = mOutDCE.dce_p25;
        pDCEReg->tc_g13.bits.DCE_TC_P26 = mOutDCE.dce_p26;
        pDCEReg->tc_g14.bits.DCE_TC_P27 = mOutDCE.dce_p27;
        pDCEReg->tc_g14.bits.DCE_TC_P28 = mOutDCE.dce_p28;
        pDCEReg->tc_g15.bits.DCE_TC_P29 = mOutDCE.dce_p29;
        pDCEReg->tc_g15.bits.DCE_TC_P30 = mOutDCE.dce_p30;
        pDCEReg->tc_g16.bits.DCE_TC_P31 = mOutDCE.dce_p31;
        pDCEReg->tc_g16.bits.DCE_TC_P32 = mOutDCE.dce_p32;
        pDCEReg->tc_g17.bits.DCE_TC_P33 = mOutDCE.dce_p33;
        pDCEReg->tc_g17.bits.DCE_TC_P34 = mOutDCE.dce_p34;
        pDCEReg->tc_g18.bits.DCE_TC_P35 = mOutDCE.dce_p35;
        pDCEReg->tc_g18.bits.DCE_TC_P36 = mOutDCE.dce_p36;
        pDCEReg->tc_g19.bits.DCE_TC_P37 = mOutDCE.dce_p37;
        pDCEReg->tc_g19.bits.DCE_TC_P38 = mOutDCE.dce_p38;
        pDCEReg->tc_g20.bits.DCE_TC_P39 = mOutDCE.dce_p39;
        pDCEReg->tc_g20.bits.DCE_TC_P40 = mOutDCE.dce_p40;
        pDCEReg->tc_g21.bits.DCE_TC_P41 = mOutDCE.dce_p41;
        pDCEReg->tc_g21.bits.DCE_TC_P42 = mOutDCE.dce_p42;
        pDCEReg->tc_g22.bits.DCE_TC_P43 = mOutDCE.dce_p43;
        pDCEReg->tc_g22.bits.DCE_TC_P44 = mOutDCE.dce_p44;
        pDCEReg->tc_g23.bits.DCE_TC_P45 = mOutDCE.dce_p45;
        pDCEReg->tc_g23.bits.DCE_TC_P46 = mOutDCE.dce_p46;
        pDCEReg->tc_g24.bits.DCE_TC_P47 = mOutDCE.dce_p47;
        pDCEReg->tc_g24.bits.DCE_TC_P48 = mOutDCE.dce_p48;
        pDCEReg->tc_g25.bits.DCE_TC_P49 = mOutDCE.dce_p49;
        pDCEReg->tc_g25.bits.DCE_TC_P50 = mOutDCE.dce_p50;
        pDCEReg->tc_g26.bits.DCE_TC_P51 = mOutDCE.dce_p51;
        pDCEReg->tc_g26.bits.DCE_TC_P52 = mOutDCE.dce_p52;
        pDCEReg->tc_g27.bits.DCE_TC_P53 = mOutDCE.dce_p53;
        pDCEReg->tc_g27.bits.DCE_TC_P54 = mOutDCE.dce_p54;
        pDCEReg->tc_g28.bits.DCE_TC_P55 = mOutDCE.dce_p55;
        pDCEReg->tc_g28.bits.DCE_TC_P56 = mOutDCE.dce_p56;
        pDCEReg->tc_g29.bits.DCE_TC_P57 = mOutDCE.dce_p57;
        pDCEReg->tc_g29.bits.DCE_TC_P58 = mOutDCE.dce_p58;
        pDCEReg->tc_g30.bits.DCE_TC_P59 = mOutDCE.dce_p59;
        pDCEReg->tc_g30.bits.DCE_TC_P60 = mOutDCE.dce_p60;
        pDCEReg->tc_g31.bits.DCE_TC_P61 = mOutDCE.dce_p61;
        pDCEReg->tc_g31.bits.DCE_TC_P62 = mOutDCE.dce_p62;
        pDCEReg->tc_g32.bits.DCE_TC_P63 = mOutDCE.dce_p63;
        pDCEReg->tc_g32.bits.DCE_TC_P64 = mOutDCE.dce_p64;

        m_FLC_Reg.ofst_rb.bits.FLC_OFST_B = mOutDCE.flare_offset;
        m_FLC_Reg.ofst_rb.bits.FLC_OFST_R = mOutDCE.flare_offset;
        m_FLC_Reg.ofst_g.bits.FLC_OFST_G  = mOutDCE.flare_offset;
        m_FLC_Reg.gn_rb.bits.FLC_GAIN_B = mOutDCE.flare_gain;
        m_FLC_Reg.gn_rb.bits.FLC_GAIN_R = mOutDCE.flare_gain;
        m_FLC_Reg.gn_g.bits.FLC_GAIN_G  = mOutDCE.flare_gain;

        m_DCES_yv_bld = mOutDCE.yv_bld;

        ::memcpy(pDceExif, &mOutExif, sizeof(NSIspExifDebug::IspDceInfo));
    }

    if (mLogEn)
    {
        CAM_LOGD_IF(1, "[%s()] m_eSensorDev(%d) after DceMain()", __FUNCTION__, m_eSensorDev);
        printInfo();
    }

}

MVOID DceMgr::setAEInfo(AE_ISP_INFO_T const & rAEInfo)
{
    if (!mAlgoEn) return;

    mInInfo.dce_ae_info.u4AETarget= rAEInfo.u4AETarget;
    mInInfo.dce_ae_info.u4AECurrentTarget= rAEInfo.u4AECurrentTarget;
    mInInfo.dce_ae_info.i4LightValue_x10= rAEInfo.i4LightValue_x10;
    mInInfo.dce_ae_info.i4GammaIdx= rAEInfo.i4GammaIdx;
    mInInfo.dce_ae_info.u4MaxISO= rAEInfo.u4MaxISO;
    mInInfo.dce_ae_info.u4AEStableCnt= rAEInfo.u4AEStableCnt;
    for (int i = 0; i<DCE_AE_HISTOGRAM_BIN; i++)
    {
        mInInfo.dce_ae_info.u4Histogrm[i]= rAEInfo.u4Histogrm[i];
    }
    mInInfo.dce_ae_info.bGammaEnable = rAEInfo.bGammaEnable;
    mInInfo.dce_ae_info.bAEStable = rAEInfo.bAEStable;
    mInInfo.dce_ae_info.u4EVRatio = rAEInfo.u4EVRatio;
    mInInfo.dce_ae_info.i4deltaIndex = rAEInfo.i4deltaIndex;
    mInInfo.dce_ae_info.u4AEFinerEVIdxBase = rAEInfo.u4AEFinerEVIdxBase;
    mInInfo.dce_ae_info.bAETouchEnable = rAEInfo.bAETouchEnable;
    mInInfo.dce_ae_info.u4FaceAEStable = rAEInfo.u4FaceAEStable;
    mInInfo.dce_ae_info.u4MeterFDTarget = rAEInfo.u4MeterFDTarget;
    mInInfo.dce_ae_info.u4FDProb = rAEInfo.u4FDProb;
    mInInfo.dce_ae_info.u4FaceNum = rAEInfo.u4FaceNum;
    mInInfo.dce_ae_info.i4Crnt_FDY = rAEInfo.i4Crnt_FDY;
    mInInfo.dce_ae_info.u4MaxGain = rAEInfo.u4MaxGain;
    mInInfo.dce_ae_info.u4RequestNum = rAEInfo.u4RequestNum;

    if (mLogEn){
        printAEInfo(rAEInfo, "setAESrc");
        printAEInfo(mLast_AEInfo, "setAEDst");
    }

    mLast_AEInfo = rAEInfo;

}

MVOID
DceMgr::
printAEInfo(AE_ISP_INFO_T const & rAEInfo, const char* username)
{
        CAM_LOGD("%s: [%s] rAEInfo: %d/%d/%d/%d/%d; %d/%d/%d/%d/%d; %d/%d/%d/%d; %d/%d/%d;"
            , __FUNCTION__
            , username
            , rAEInfo.u4AETarget
            , rAEInfo.u4AECurrentTarget
            , rAEInfo.i4LightValue_x10
            , rAEInfo.i4GammaIdx
            , rAEInfo.u4MaxISO

            , rAEInfo.u4AEStableCnt
            , rAEInfo.bGammaEnable
            , rAEInfo.bAEStable
            , rAEInfo.u4EVRatio
            , rAEInfo.i4deltaIndex

            , rAEInfo.u4AEFinerEVIdxBase
            , rAEInfo.bAETouchEnable
            , rAEInfo.u4FaceAEStable
            , rAEInfo.u4MeterFDTarget
            , rAEInfo.u4FDProb

            , rAEInfo.u4FaceNum
            , rAEInfo.i4Crnt_FDY
            , rAEInfo.u4MaxGain
        );


        MUINT32 u4Remainder = DCE_AE_HISTOGRAM_BIN % 10;
        const MUINT32* pHist = &rAEInfo.u4Histogrm[0];

        for (int i=0; i<(DCE_AE_HISTOGRAM_BIN - u4Remainder); i+=10)
        {
            CAM_LOGD( "%s: rAEInfo.u4Histogrm[%d-%d]=(%x,%x,%x,%x,%x,  %x,%x,%x,%x,%x)"
                , __FUNCTION__
                , i
                , i+9
                , pHist[i], pHist[i+1], pHist[i+2], pHist[i+3], pHist[i+4]
                , pHist[i+5], pHist[i+6], pHist[i+7], pHist[i+8], pHist[i+9]
            );
        }
        {
            //i = 250 ~ 255
            int i= DCE_AE_HISTOGRAM_BIN - u4Remainder;

            CAM_LOGD( "%s: rAEInfo.u4Histogrm[%d-%d]=(%x,%x,%x,%x,%x,  %x)"
                , __FUNCTION__
                , i
                , i + ( u4Remainder - 1 )
                , pHist[i], pHist[i+1], pHist[i+2], pHist[i+3], pHist[i+4]
                , pHist[i+5]
            );
        }

}

MVOID DceMgr::printInfo() const
{
    CAM_LOGD("%s: DCE Algo Result(Curve): %d/%d/%d/%d/%d/%d/%d/%d/%d/%d; %d/%d/%d/%d/%d/%d/%d/%d/%d/%d; %d/%d/%d/%d/%d/%d/%d/%d/%d/%d; %d/%d/%d/%d/%d/%d/%d/%d/%d/%d; %d/%d/%d/%d/%d/%d/%d/%d/%d/%d; %d/%d/%d/%d/%d/%d/%d/%d/%d/%d; %d/%d/%d/%d"
        , __FUNCTION__
        ,mOutDCE.dce_p1
        ,mOutDCE.dce_p2
        ,mOutDCE.dce_p3
        ,mOutDCE.dce_p4
        ,mOutDCE.dce_p5
        ,mOutDCE.dce_p6
        ,mOutDCE.dce_p7
        ,mOutDCE.dce_p8
        ,mOutDCE.dce_p9
        ,mOutDCE.dce_p10
        ,mOutDCE.dce_p11
        ,mOutDCE.dce_p12
        ,mOutDCE.dce_p13
        ,mOutDCE.dce_p14
        ,mOutDCE.dce_p15
        ,mOutDCE.dce_p16
        ,mOutDCE.dce_p17
        ,mOutDCE.dce_p18
        ,mOutDCE.dce_p19
        ,mOutDCE.dce_p20
        ,mOutDCE.dce_p21
        ,mOutDCE.dce_p22
        ,mOutDCE.dce_p23
        ,mOutDCE.dce_p24
        ,mOutDCE.dce_p25
        ,mOutDCE.dce_p26
        ,mOutDCE.dce_p27
        ,mOutDCE.dce_p28
        ,mOutDCE.dce_p29
        ,mOutDCE.dce_p30
        ,mOutDCE.dce_p31
        ,mOutDCE.dce_p32
        ,mOutDCE.dce_p33
        ,mOutDCE.dce_p34
        ,mOutDCE.dce_p35
        ,mOutDCE.dce_p36
        ,mOutDCE.dce_p37
        ,mOutDCE.dce_p38
        ,mOutDCE.dce_p39
        ,mOutDCE.dce_p40
        ,mOutDCE.dce_p41
        ,mOutDCE.dce_p42
        ,mOutDCE.dce_p43
        ,mOutDCE.dce_p44
        ,mOutDCE.dce_p45
        ,mOutDCE.dce_p46
        ,mOutDCE.dce_p47
        ,mOutDCE.dce_p48
        ,mOutDCE.dce_p49
        ,mOutDCE.dce_p50
        ,mOutDCE.dce_p51
        ,mOutDCE.dce_p52
        ,mOutDCE.dce_p53
        ,mOutDCE.dce_p54
        ,mOutDCE.dce_p55
        ,mOutDCE.dce_p56
        ,mOutDCE.dce_p57
        ,mOutDCE.dce_p58
        ,mOutDCE.dce_p59
        ,mOutDCE.dce_p60
        ,mOutDCE.dce_p61
        ,mOutDCE.dce_p62
        ,mOutDCE.dce_p63
        ,mOutDCE.dce_p64);

    CAM_LOGD("%s: DCE Algo Result(Other): Flare_Ofst(%d), Flare_Gain(%d), DCES_YV_BLD(%d)"
        , __FUNCTION__
        ,mOutDCE.flare_offset
        ,mOutDCE.flare_gain
        ,mOutDCE.yv_bld);

    CAM_LOGD("%s: DCE EXIF (Curve): %d/%d/%d/%d/%d/%d/%d/%d/%d/%d; %d/%d/%d/%d/%d/%d/%d/%d/%d/%d; %d/%d/%d/%d/%d/%d/%d/%d/%d/%d; %d/%d/%d/%d/%d/%d/%d/%d/%d/%d; %d/%d/%d/%d/%d/%d/%d/%d/%d/%d; %d/%d/%d/%d/%d/%d/%d/%d/%d/%d; %d/%d/%d/%d"
        , __FUNCTION__
        ,mOutExif.dce_p1
        ,mOutExif.dce_p2
        ,mOutExif.dce_p3
        ,mOutExif.dce_p4
        ,mOutExif.dce_p5
        ,mOutExif.dce_p6
        ,mOutExif.dce_p7
        ,mOutExif.dce_p8
        ,mOutExif.dce_p9
        ,mOutExif.dce_p10
        ,mOutExif.dce_p11
        ,mOutExif.dce_p12
        ,mOutExif.dce_p13
        ,mOutExif.dce_p14
        ,mOutExif.dce_p15
        ,mOutExif.dce_p16
        ,mOutExif.dce_p17
        ,mOutExif.dce_p18
        ,mOutExif.dce_p19
        ,mOutExif.dce_p20
        ,mOutExif.dce_p21
        ,mOutExif.dce_p22
        ,mOutExif.dce_p23
        ,mOutExif.dce_p24
        ,mOutExif.dce_p25
        ,mOutExif.dce_p26
        ,mOutExif.dce_p27
        ,mOutExif.dce_p28
        ,mOutExif.dce_p29
        ,mOutExif.dce_p30
        ,mOutExif.dce_p31
        ,mOutExif.dce_p32
        ,mOutExif.dce_p33
        ,mOutExif.dce_p34
        ,mOutExif.dce_p35
        ,mOutExif.dce_p36
        ,mOutExif.dce_p37
        ,mOutExif.dce_p38
        ,mOutExif.dce_p39
        ,mOutExif.dce_p40
        ,mOutExif.dce_p41
        ,mOutExif.dce_p42
        ,mOutExif.dce_p43
        ,mOutExif.dce_p44
        ,mOutExif.dce_p45
        ,mOutExif.dce_p46
        ,mOutExif.dce_p47
        ,mOutExif.dce_p48
        ,mOutExif.dce_p49
        ,mOutExif.dce_p50
        ,mOutExif.dce_p51
        ,mOutExif.dce_p52
        ,mOutExif.dce_p53
        ,mOutExif.dce_p54
        ,mOutExif.dce_p55
        ,mOutExif.dce_p56
        ,mOutExif.dce_p57
        ,mOutExif.dce_p58
        ,mOutExif.dce_p59
        ,mOutExif.dce_p60
        ,mOutExif.dce_p61
        ,mOutExif.dce_p62
        ,mOutExif.dce_p63
        ,mOutExif.dce_p64);

    CAM_LOGD("%s: DCE EXIF (Other):  %d/%d/%d/%d/%d/%d/%d/%d/%d/%d; %d/%d/%d/%d/%d/%d/%d/%d/%d/%d; %d/%d/%d"
        , __FUNCTION__
        , mOutExif.chip_version
        , mOutExif.main_version
        , mOutExif.sub_version
        , mOutExif.system_version
        , mOutExif.smooth_speed
        , mOutExif.lv_idx_l
        , mOutExif.lv_idx_h
        , mOutExif.dr_idx_l
        , mOutExif.dr_idx_h
        , mOutExif.dce_flat_ratio_thd_l

        , mOutExif.dce_flat_ratio_thd_h
        , mOutExif.dce_flat_prot_flag
        , mOutExif.dce_slope_max
        , mOutExif.dce_smooth_speed
        , mOutExif.flare_offset
        , mOutExif.flare_gain
        , mOutExif.dce_pt0
        , mOutExif.dce_pt1
        , mOutExif.dce_pt2
        , mOutExif.dce_pt3

        , mOutExif.dce_str1
        , mOutExif.dce_str2
        , mOutExif.dce_str3);

}

}
