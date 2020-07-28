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
#include <private/aaa_hal_private.h>
#include <private/aaa_utils.h>
#include <array>

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
#include "gma_mgr.h"
#include <isp_tuning_mgr.h>
#include <assert.h>

#define MY_INST NS3Av3::INST_T<GmaMgr>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

using namespace NSIspTuning;
namespace NSIspTuning
{

GmaMgr*
GmaMgr::
createInstance(MUINT32 const eSensorDev, ISP_NVRAM_GMA_STRUCT_T* pIspNvramGmaPara)
{
	auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(static_cast<MINT32>(eSensorDev));
	if(i4SensorIdx >= SENSOR_IDX_MAX || i4SensorIdx < 0) {
		CAM_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorIdx);
		return NULL;
       }
	MY_INST& rSingleton = gMultiton[i4SensorIdx];
	std::call_once(rSingleton.onceFlag, [&] {
	rSingleton.instance = std::make_unique<GmaMgr>(eSensorDev, pIspNvramGmaPara);
    } );

    return rSingleton.instance.get();

}

GmaMgr::GmaMgr(MUINT32 const eSensorDev, ISP_NVRAM_GMA_STRUCT_T* pIspNvramGmaPara)
    : m_eSensorDev  (eSensorDev)
    , m_pIspNvramGmaPara (pIspNvramGmaPara)
    , mLogEn (0)
    , m_PrvIdx(0)
    , mAlgoEn(MTRUE)
{
    if(sizeof(DUAL_GGM_SYNC_INFO_T) != sizeof(MTK_GMA_DUALCAM_SYNC_INFO_T)){
        CAM_LOGE("DUAL_GGM_SYNC_INFO_T size error");
        assert(0);
    }

    if(sizeof(NSIspExifDebug::IspGmaInfo) != sizeof(MTK_GMA_EXIF_INFO_STRUCT)){
        CAM_LOGE("IspGmaInfo size error");
        assert(0);
    }

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.gma_algo.disable", value, "0");
    mAlgoEn = !(atoi(value));

    m_Last_GGM = m_pIspNvramGmaPara[m_PrvIdx].GGM_Reg[1];

    if (mAlgoEn){
        m_pGmaAlgo = MTKGma::createInstance(static_cast<eGMASensorDev_T>(m_eSensorDev));
    }
    else m_pGmaAlgo = NULL;

    memset(&mEnv,        0, sizeof(MTK_GMA_ENV_INFO_STRUCT));
    memset(&mInInfo,     0, sizeof(MTK_GMA_PROC_INFO_STRUCT));
    memset(&mOutGGM,     0, sizeof(MTK_GMA_RESULT_INFO_STRUCT));
    memset(&mOutGGM_D1,  0, sizeof(MTK_GMA_RESULT_INFO_STRUCT));
    memset(&mOutIGGM_D1, 0, sizeof(MTK_GMA_RESULT_INFO_STRUCT));
    memset(&mOutExif,    0, sizeof(MTK_GMA_EXIF_INFO_STRUCT));
    memset(&mGMASyncInfo,0, sizeof(MTK_GMA_DUALCAM_SYNC_INFO_T));
    memset(&mLast_AEInfo,0, sizeof(AE_ISP_INFO_T));

    if(m_pGmaAlgo){
        for (int tb_num = 0; tb_num<GMA_TBL_NUM; tb_num++){
            for (int i = 0; i<GGM_LUT_SIZE; i++){
                mEnv.rNVRAMGmaParam.i4EncNVRAMGMA[tb_num][i] = m_pIspNvramGmaPara[m_PrvIdx].GGM_Reg[tb_num].lut[i].bits.GGM_G;
            }
            for(int i=GGM_LUT_SIZE; i<GMA_TBL_SIZE; i++){
                mEnv.rNVRAMGmaParam.i4EncNVRAMGMA[tb_num][i] = 0xFFFF;
            }
        }
        mEnv.rTuningGmaParam = *((MTK_GMA_TUNING_PARAM_T*)&(m_pIspNvramGmaPara[m_PrvIdx].rGmaParam));

        m_pGmaAlgo->GmaReset();
        m_pGmaAlgo->GmaInit(&mEnv, NULL);
    }

    CAM_LOGD_IF(1, "[%s()] m_pGmaAlgo(%p), m_eSensorDev(%d), m_pIspNvramGmaPara(%p)", __FUNCTION__, m_pGmaAlgo, m_eSensorDev, m_pIspNvramGmaPara);

}

GmaMgr::~GmaMgr()
{
    if(m_pGmaAlgo){
        m_pGmaAlgo->GmaExit();
        m_pGmaAlgo->destroyInstance(m_pGmaAlgo);
    }
   CAM_LOGD_IF(1, "[%s()] destroyInstance (%p)", __FUNCTION__, m_pGmaAlgo);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
GmaMgr::
start()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.dynamic_gma.log", value, "0");
    mLogEn = atoi(value);

    m_PrvIdx = 0;

    m_Last_GGM = m_pIspNvramGmaPara[m_PrvIdx].GGM_Reg[1];

    for (int tb_num = 0; tb_num<GMA_TBL_NUM; tb_num++)
    {
        for (int i = 0; i<GGM_LUT_SIZE; i++)
        {
            mEnv.rNVRAMGmaParam.i4EncNVRAMGMA[tb_num][i] = m_pIspNvramGmaPara[m_PrvIdx].GGM_Reg[tb_num].lut[i].bits.GGM_G;
        }
        for(int i=GGM_LUT_SIZE; i<GMA_TBL_SIZE; i++)
        {
            mEnv.rNVRAMGmaParam.i4EncNVRAMGMA[tb_num][i] = 0xFFFF;
        }
    }
    mEnv.rTuningGmaParam = *((MTK_GMA_TUNING_PARAM_T*)&(m_pIspNvramGmaPara[m_PrvIdx].rGmaParam));

    setAEInfo(mLast_AEInfo);

    for (int i = 0; i<GGM_LUT_SIZE; i++)
    {
        mInInfo.i4CurrEncGMA[i] = m_pIspNvramGmaPara[m_PrvIdx].GGM_Reg[0].lut[i].bits.GGM_G;
    }

    if(m_pGmaAlgo){
        m_pGmaAlgo->GmaFeatureCtrl(MTKGMA_FEATURE_SET_ENV_INFO, &mEnv, NULL);
        m_pGmaAlgo->GmaFeatureCtrl(MTKGMA_FEATURE_SET_PROC_INFO, &mInInfo, NULL);
    }

    CAM_LOGD_IF(1, "[%s()] GmaMgr start (%p)", __FUNCTION__, m_pGmaAlgo);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
GmaMgr::
stop()
{
    CAM_LOGD_IF(1, "[%s()] GmaMgr stop (%p)", __FUNCTION__, m_pGmaAlgo);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
GmaMgr::
calculateGGM(RAWIspCamInfo const& rRawIspCamInfo, MUINT16 Idx,
                   ISP_NVRAM_GGM_T* pGGMReg_D1,ISP_NVRAM_IGGM_T* pIGGMReg_D1, ISP_NVRAM_GGM_T* pGGMReg,
                   NSIspExifDebug::IspGmaInfo* pGmaExif,
                   MBOOL bSlave, DUAL_ISP_SYNC_INFO_T& rISP_Sync_Info)
{

    if( Idx != m_PrvIdx){

        for (int tb_num = 0; tb_num<GMA_TBL_NUM; tb_num++)
        {
            for (int i = 0; i<GGM_LUT_SIZE; i++)
            {
                mEnv.rNVRAMGmaParam.i4EncNVRAMGMA[tb_num][i] = m_pIspNvramGmaPara[Idx].GGM_Reg[tb_num].lut[i].bits.GGM_G;
            }
            for(int i=GGM_LUT_SIZE; i<GMA_TBL_SIZE; i++)
            {
                mEnv.rNVRAMGmaParam.i4EncNVRAMGMA[tb_num][i] = 0xFFFF;
            }
        }
        mEnv.rTuningGmaParam = *((MTK_GMA_TUNING_PARAM_T*)&(m_pIspNvramGmaPara[Idx].rGmaParam));

        m_PrvIdx = Idx;

        if (m_pGmaAlgo){
            m_pGmaAlgo->GmaInit(&mEnv, NULL);
            m_pGmaAlgo->GmaFeatureCtrl(MTKGMA_FEATURE_SET_ENV_INFO, &mEnv, NULL);
        }

    }

    setAEInfo(rRawIspCamInfo.rAEInfo);

    mInInfo.bIsDualcamSlave = bSlave;
    if(bSlave){
        mInInfo.rGMADualcamSyncInfo = *((MTK_GMA_DUALCAM_SYNC_INFO_T*)&(rISP_Sync_Info.sGmaSyncInfo));
    }

    for (int i = 0; i<GGM_LUT_SIZE; i++)
    {
        mInInfo.i4CurrEncGMA[i] = pGGMReg->lut[i].bits.GGM_G;
    }


    if (m_pGmaAlgo)
    {
        if (mAlgoEn) {

            m_pGmaAlgo->GmaFeatureCtrl(MTKGMA_FEATURE_SET_PROC_INFO, &mInInfo, NULL);
            m_pGmaAlgo->GmaMain();
            m_pGmaAlgo->GmaFeatureCtrl(MTKGMA_FEATURE_GET_RESULT, NULL, &mOutGGM);
            m_pGmaAlgo->GmaFeatureCtrl(MTKGMA_FEATURE_GET_GGMD1_RESULT, NULL, &mOutGGM_D1);
            m_pGmaAlgo->GmaFeatureCtrl(MTKGMA_FEATURE_GET_IGGMD1_RESULT, NULL, &mOutIGGM_D1);
            m_pGmaAlgo->GmaFeatureCtrl(MTKGMA_FEATURE_GET_EXIF,   NULL, &mOutExif);
        }

        for (int i = 0; i<(GGM_LUT_SIZE); i++)
        {
            pGGMReg->lut[i].bits.GGM_R = pGGMReg->lut[i].bits.GGM_B = pGGMReg->lut[i].bits.GGM_G = mOutGGM.i4EncGMA[i];
            pGGMReg->lut[i].bits.rsv_30 = 0;

            pGGMReg_D1->lut[i].bits.GGM_R = pGGMReg_D1->lut[i].bits.GGM_B = pGGMReg_D1->lut[i].bits.GGM_G = mOutGGM_D1.i4EncGMA[i];
            pGGMReg_D1->lut[i].bits.rsv_30 = 0;

            pIGGMReg_D1->lut_rg.lut[i].IGGM_R = pIGGMReg_D1->lut_rg.lut[i].IGGM_G = pIGGMReg_D1->lut_b.lut[i].IGGM_B = mOutIGGM_D1.i4EncGMA[i];
            pIGGMReg_D1->lut_rg.lut[i].rsv_12 = pIGGMReg_D1->lut_rg.lut[i].rsv_28 = pIGGMReg_D1->lut_b.lut[i].rsv_12 = 0;
        }

        if(!bSlave){
            if (mAlgoEn) m_pGmaAlgo->GmaFeatureCtrl(MTKGMA_FEATURE_GET_DUALCAM_SYNC_INFO, NULL, &mGMASyncInfo);
            ::memcpy(((MTK_GMA_DUALCAM_SYNC_INFO_T*)&(rISP_Sync_Info.sGmaSyncInfo)), &mGMASyncInfo, sizeof(DUAL_GGM_SYNC_INFO_T));
        }

        ::memcpy(pGmaExif, &mOutExif, sizeof(NSIspExifDebug::IspGmaInfo));

        if (mLogEn)
        {
            CAM_LOGD_IF(1, "[%s()] m_eSensorDev(%d) after GmaMain()", __FUNCTION__, m_eSensorDev);
            printInfo();
        }

        updateLastGGM(*pGGMReg);
    }
}

MVOID GmaMgr::setAEInfo(AE_ISP_INFO_T const & rAEInfo)
{
    if (!mAlgoEn) return;

    mInInfo.rGMAAEInfo.u4AETarget         = rAEInfo.u4AETarget;
    mInInfo.rGMAAEInfo.u4AECurrentTarget  = rAEInfo.u4AECurrentTarget;
    mInInfo.rGMAAEInfo.i4LightValue_x10   = rAEInfo.i4LightValue_x10;
    mInInfo.rGMAAEInfo.i2FlareOffset      = rAEInfo.i2FlareOffset;
    mInInfo.rGMAAEInfo.i4GammaIdx         = rAEInfo.i4GammaIdx;
    mInInfo.rGMAAEInfo.i4LESE_Ratio       = rAEInfo.i4LESE_Ratio;
    mInInfo.rGMAAEInfo.bGammaEnable       = rAEInfo.bGammaEnable;
    mInInfo.rGMAAEInfo.bAEStable          = rAEInfo.bAEStable;
    mInInfo.rGMAAEInfo.u4EVRatio          = rAEInfo.u4EVRatio;
    mInInfo.rGMAAEInfo.i4deltaIndex       = rAEInfo.i4deltaIndex;
    mInInfo.rGMAAEInfo.u4FDProb           = rAEInfo.u4FDProb;
    mInInfo.rGMAAEInfo.u4FaceNum          = rAEInfo.u4FaceNum;
    for (int i = 0; i<GMA_AE_HISTOGRAM_BIN; i++)
    {
        mInInfo.rGMAAEInfo.u4Histogrm[i]  = rAEInfo.u4Histogrm[i];
    }

    if (mLogEn){
        printAEInfo(rAEInfo, "setAESrc");
        printAEInfo(mLast_AEInfo, "setAEDst");
    }

    mLast_AEInfo = rAEInfo;

}


MVOID
GmaMgr::
printAEInfo(AE_ISP_INFO_T const & rAEInfo, const char* username)
{
        CAM_LOGD("%s: [%s] rAEInfo: %d/%d/%d/%d/%d; %d/%d/%d/%d/%d; %d/%d"
            , __FUNCTION__
            , username
            , rAEInfo.u4AETarget
            , rAEInfo.u4AECurrentTarget
            , rAEInfo.i4LightValue_x10
            , rAEInfo.i2FlareOffset
            , rAEInfo.i4GammaIdx
            , rAEInfo.i4LESE_Ratio
            , rAEInfo.bGammaEnable
            , rAEInfo.bAEStable
            , rAEInfo.u4EVRatio
            , rAEInfo.i4deltaIndex
            , rAEInfo.u4FDProb
             ,rAEInfo.u4FaceNum
            );



        MUINT32 u4Remainder = GMA_AE_HISTOGRAM_BIN % 10;

        const MUINT32* pHist = &rAEInfo.u4Histogrm[0];
        for (int i=0; i<(GMA_AE_HISTOGRAM_BIN - u4Remainder); i+=10)
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
            int i= GMA_AE_HISTOGRAM_BIN - u4Remainder;

            CAM_LOGD( "%s: rAEInfo.u4Histogrm[%d-%d]=(%x,%x,%x,%x,%x,  %x)"
                , __FUNCTION__
                , i
                , i + ( u4Remainder - 1 )
                , pHist[i], pHist[i+1], pHist[i+2], pHist[i+3], pHist[i+4]
                , pHist[i+5]
            );
        }

}

MVOID GmaMgr::printInfo() const
{
    CAM_LOGD("%s: GMA EXIF:  %x/%x; %x/%x/%x/%x/%x; %x/%x/%x/%x/%x; %x/%x/%x/%x/%x; %x/%x/%x/%x/%x; %x/%x/%x/%x/%x; %x/%x/%x/%x/%x;"
        , __FUNCTION__

        , mOutExif.i4GMAMode
        , mOutExif.i4FaceGMAFlag
        , mOutExif.i4ChipVersion
        , mOutExif.i4MainVersion
        , mOutExif.i4SubVersion
        , mOutExif.i4SystemVersion
        , mOutExif.u4EVRatio
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

    MUINT32 u4Remainder = GGM_LUT_SIZE % 10;

    for (int i=0; i< (GGM_LUT_SIZE - u4Remainder) ; i+=10)
    {
        CAM_LOGD( "%s: mInInfo.i4CurrEncGMA[%d-%d]=(%x,%x,%x,%x,%x,  %x,%x,%x,%x,%x)"
            , __FUNCTION__
            , i
            , i+9
            , mInInfo.i4CurrEncGMA[i], mInInfo.i4CurrEncGMA[i+1], mInInfo.i4CurrEncGMA[i+2], mInInfo.i4CurrEncGMA[i+3], mInInfo.i4CurrEncGMA[i+4]
            , mInInfo.i4CurrEncGMA[i+5], mInInfo.i4CurrEncGMA[i+6], mInInfo.i4CurrEncGMA[i+7], mInInfo.i4CurrEncGMA[i+8], mInInfo.i4CurrEncGMA[i+9]);
    }
    {
        //i = 190 ~ 191
        int i= GGM_LUT_SIZE - u4Remainder;

        CAM_LOGD( "%s: mInInfo.i4CurrEncGMA[%d-%d]=(%x,%x)"
            , __FUNCTION__
            , i
            , i + ( u4Remainder - 1 )
            , mInInfo.i4CurrEncGMA[i], mInInfo.i4CurrEncGMA[i+1]);

    }


    for (int i = 0; i<(GGM_LUT_SIZE); i++){
        if (((i%10) == 0) && (i!=190)){
            ALOGE( "%s: mOutGGM.i4EncGMA[%d-%d]=(%x,%x,%x,%x,%x,  %x,%x,%x,%x,%x)"
                , __FUNCTION__
                , i
                , i+9
                , mOutGGM.i4EncGMA[i], mOutGGM.i4EncGMA[i+1], mOutGGM.i4EncGMA[i+2], mOutGGM.i4EncGMA[i+3], mOutGGM.i4EncGMA[i+4]
                , mOutGGM.i4EncGMA[i+5], mOutGGM.i4EncGMA[i+6], mOutGGM.i4EncGMA[i+7], mOutGGM.i4EncGMA[i+8], mOutGGM.i4EncGMA[i+9]);
            ALOGE( "%s: mOutGGM_D1.i4EncGMA[%d-%d]=(%x,%x,%x,%x,%x,  %x,%x,%x,%x,%x)"
                , __FUNCTION__
                , i
                , i+9
                , mOutGGM_D1.i4EncGMA[i], mOutGGM_D1.i4EncGMA[i+1], mOutGGM_D1.i4EncGMA[i+2], mOutGGM_D1.i4EncGMA[i+3], mOutGGM_D1.i4EncGMA[i+4]
                , mOutGGM_D1.i4EncGMA[i+5], mOutGGM_D1.i4EncGMA[i+6], mOutGGM_D1.i4EncGMA[i+7], mOutGGM_D1.i4EncGMA[i+8], mOutGGM_D1.i4EncGMA[i+9]);
            ALOGE( "%s: mOutIGGM_D1.i4EncGMA[%d-%d]=(%x,%x,%x,%x,%x,  %x,%x,%x,%x,%x)"
                , __FUNCTION__
                , i
                , i+9
                , mOutIGGM_D1.i4EncGMA[i], mOutIGGM_D1.i4EncGMA[i+1], mOutIGGM_D1.i4EncGMA[i+2], mOutIGGM_D1.i4EncGMA[i+3], mOutIGGM_D1.i4EncGMA[i+4]
                , mOutIGGM_D1.i4EncGMA[i+5], mOutIGGM_D1.i4EncGMA[i+6], mOutIGGM_D1.i4EncGMA[i+7], mOutIGGM_D1.i4EncGMA[i+8], mOutIGGM_D1.i4EncGMA[i+9]);
        }
        else if (i == 190){
            ALOGE( "%s: mOutGGM.i4EncGMA[%d-%d]=(%x,%x)"
                , __FUNCTION__
                , i
                , i+1
                , mOutGGM.i4EncGMA[i], mOutGGM.i4EncGMA[i+1]);
            ALOGE( "%s: mOutGGM_D1.i4EncGMA[%d-%d]=(%x,%x)"
                , __FUNCTION__
                , i
                , i+1
                , mOutGGM_D1.i4EncGMA[i], mOutGGM_D1.i4EncGMA[i+1]);
            ALOGE( "%s: mOutIGGM_D1.i4EncGMA[%d-%d]=(%x,%x)"
                , __FUNCTION__
                , i
                , i+1
                , mOutIGGM_D1.i4EncGMA[i], mOutIGGM_D1.i4EncGMA[i+1]);

        }
    }
}

}
